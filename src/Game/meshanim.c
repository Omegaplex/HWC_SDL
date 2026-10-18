/*=============================================================================
    Name    : MeshAnim.c
    Purpose : Code for animating meshes in ships

    Created 7/22/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "debug.h"
#include "memory.h"
#include "file.h"
#include "b-spline.h"
#include "spaceobj.h"
#include "vector.h"
#include "matrix.h"
#include "universe.h"
#include "univupdate.h"
#include "nis.h"
#include "mainrgn.h"
#include "meshanim.h"
#include "crc32.h"

/*=============================================================================
    Data:
=============================================================================*/
#if MR_TEST_HPB
sdword madTestHPBIndex = 0;
extern bool32 mrTestHPBMode;
extern real32 mrHeading, mrPitch, mrBank;
#endif
//global data for recursive-walk callbacks
static mhlocalbinding *madBindingDest;
static madheader *madDupeHeader;
static Ship *madDupeShip;
//static sdword madBindingIndex, madIncIndex;
//callback(static char *madDupeName;

static sdword madResolveAnimationIndex(const Ship *ship, sdword animIndex)
{
    const madstatic *madStat = ship->staticinfo->madStatic;

    if (madStat != NULL && madStat->animIndexRemapCount != 0)
    {
        return madStat->animIndexRemap[animIndex];
    }

    return animIndex;
}

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : madFileLoad
    Description : Load in a .MAD mesh animation file
    Inputs      : fileName - name of file to load
    Outputs     :
    Return      : newly allocated animation data
    Note        : the string block for binding information is allocated as a
                    separate block of memory so it can be freed after all
                    animation binding is complete.
----------------------------------------------------------------------------*/
madheader *madFileLoad(char *fileName)
{
    madheader header;
    madheader *newHeader;
    sdword index, j;
    filehandle file;
    sdword fileSize;

    dbgAssert(fileName != NULL);

    fileSize = fileSizeGet(fileName, 0);
    file = fileOpen(fileName, 0);
    fileBlockRead(file, &header, madHeaderSize(0));
#if MAD_ERROR_CHECKING
    if (strcmp(header.identifier, MAD_FileIdentifier) != 0)
    {
        dbgFatalf(DBG_Loc, "Invalid header in '%s'.  Expected '%s', found '%s'.", fileName, MAD_FileIdentifier, header.identifier);
    }
    if (header.version != MAD_FileVersion)
    {
        dbgFatalf(DBG_Loc, "Invalid file version in '%s'.  Expected %.2f, found %.2f", fileName, MAD_FileVersion, header.version);
    }
    if (header.nAnimations > MAD_MaxAnimations)
    {
        dbgFatalf(DBG_Loc, "Too many animations in '%s': %d", fileName, header.nAnimations);
    }
#endif

    newHeader = memAlloc(fileSize - header.stringBlockLength, "meshAnimation", NonVolatile);
    *newHeader = header;
    fileBlockRead(file, &newHeader->anim[0], fileSize - newHeader->stringBlockLength - madHeaderSize(0));

    newHeader->stringBlock = memAlloc(newHeader->stringBlockLength, "madStringBlock", NonVolatile);//!!!should this be non volatile?  I guess it depends on how soon the string block is freed, if ever.
    fileBlockRead(file, newHeader->stringBlock, newHeader->stringBlockLength);

    fileClose(file);
#if MAD_VERBOSE_LEVEL >= 2
    dbgMessagef("\nmadFileLoad: loaded %d animations for %d objects from '%s'", newHeader->nAnimations, newHeader->nObjects, fileName);
#endif

    //loop through all the structures and fix up pointers
    for (index = 0; index < newHeader->nAnimations; index++)
    {                                                       //fixup the name of all animations
        newHeader->anim[index].name += (udword)newHeader->stringBlock;
    }
    (ubyte *)newHeader->objPath += (udword)newHeader;
    for (index = 0; index < newHeader->nObjects; index++)
    {
        newHeader->objPath[index].name += (udword)newHeader->stringBlock;//fixup name
        newHeader->objPath[index].nameCRC = crc16Compute(newHeader->objPath[index].name, strlen(newHeader->objPath[index].name));
        (ubyte *)newHeader->objPath[index].times += (udword)newHeader;//fixup times pointers
        (ubyte *)newHeader->objPath[index].parameters += (udword)newHeader;//fixup times pointers
        for (j = 0; j < 6; j++)
        {                                                   //fixup the motion path array pointers
            (ubyte *)newHeader->objPath[index].path[j] += (udword)newHeader;
        }
    }

    return(newHeader);
}

/*-----------------------------------------------------------------------------
    Name        : madHeaderDelete
    Description : Delete mesh amimations
    Inputs      : header - header of data to delete
    Outputs     :
    Return      :
    Note        : if header->stringBlock was freed, it should have been set to NULL
----------------------------------------------------------------------------*/
void madHeaderDelete(madheader *header)
{
    if (header->stringBlock != NULL)
    {
        memFree(header->stringBlock);
    }
    memFree(header);
}

/*-----------------------------------------------------------------------------
    Name        : madAnimBindingPause
    Description : Mesh binding function for scripted animations that are paused
    Inputs      :
    Outputs     :
    Return      : (Matrix not updated)
----------------------------------------------------------------------------*/
bool32 madAnimBindingPause(udword flags, hmatrix *startMatrix, hmatrix *matrixDest, void *data, sdword ID)
{
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : madAnimBindingUpdate
    Description : Mesh binding function for scripted mesh animations.
    Inputs      : flags - ignored
                  startMatrix - initial matrix
                  data - pointer to ship
                  ID - index of object path in the animation for this ship
    Outputs     : matrixDest - animated matrix
    Return      : TRUE = matrix updated
----------------------------------------------------------------------------*/
bool32 madAnimBindingUpdate(udword flags, hmatrix *startMatrix, hmatrix *matrixDest, void *data, sdword ID)
{
    real32 timeElapsed;
    Ship *ship = (Ship *)data;
    sdword j, startPoint;
    sdword remappedAnim;
    madheader *header;
    madanim *anim = ship->madBindings;
    madobjpath *path;
    madanimation *animation;
    splinecurve *curve;
    vector hpb, xyz;
    matrix _3x3Matrix;
    hmatrix rotMatrix;

#if MR_TEST_HPB
    if (mrTestHPBMode && ID == madTestHPBIndex)
    {
        hpb.x = mrHeading;
        hpb.y = mrPitch;
        hpb.z = mrBank;
        curve = &anim->curves[ID * 6];                      //pointer to spline curves for this object
        xyz.x = curve[0].points[0];
        xyz.y = curve[1].points[0];
        xyz.z = curve[2].points[0];
        nisObjectEulerToMatrix(&_3x3Matrix, &hpb);          //make a rotation matrix
        hmatMakeHMatFromMatAndVec(&rotMatrix, &_3x3Matrix, &xyz);//make a hmatrix
        *matrixDest = rotMatrix;
        return(TRUE);
    }
#endif
    dbgAssert(anim != NULL);
    curve = &anim->curves[ID * 6];                          //pointer to spline curves for this object
    if (curve->currentPoint == BS_NoPoint)
    {                                                       //if this object not animating
        *matrixDest = *startMatrix;
        return(FALSE);                                      //don't bother updating animation
    }
    timeElapsed = anim->timeElapsed;
    header = anim->header;                                  //pointer to animation data
    path = &header->objPath[ID];                            //pointer to motion path
    remappedAnim = madResolveAnimationIndex(ship, anim->nCurrentAnim);
    animation = &header->anim[remappedAnim];                //pointer to animation header

    if (curve[0].timeElapsed + timeElapsed > animation->endTime)
    {                                                       //if we'll go past the end
        if (bitTest(anim->flags, MAF_Loop))
        {                                                   //if this is a looping animation
            timeElapsed = (real32)fmod((double)(curve[0].timeElapsed + timeElapsed - animation->endTime), (double)(animation->endTime - animation->startTime));

            for (startPoint = 0; startPoint < curve[0].nPoints; startPoint++)
            {                                               //find a point to restart at
                if (curve[0].times[startPoint] == anim->startTime)
                {
                    goto foundTime;
                }
            }
#if MAD_ERROR_CHECKING
            dbgFatalf(DBG_Loc, "madAnimationStart: Object #%d ('%s') has no keyframe at frame %.0f",
                  anim->nCurrentAnim, header->objPath[anim->nCurrentAnim].name, anim->startTime / header->framesPerSecond);
#endif
foundTime:
            for (j = 0; j < 6; j++)
            {                                               //reset all the motion curves
                curve[j].timeElapsed = animation->startTime;
                curve[j].currentPoint = startPoint;
            }
        }
        else
        {                                                   //else it's a non-looping animation; clamp at the end
            timeElapsed = animation->endTime - curve[0].timeElapsed;//clamp to end
        }
    }
    //now update the motion curves
    xyz.x =  bsCurveUpdate(&curve[1], timeElapsed);
    xyz.y =  bsCurveUpdate(&curve[2], timeElapsed);
    xyz.z = -bsCurveUpdate(&curve[0], timeElapsed);
    hpb.x =  bsCurveUpdate(&curve[3], timeElapsed);
    hpb.y =  bsCurveUpdate(&curve[4], timeElapsed);
    hpb.z =  bsCurveUpdate(&curve[5], timeElapsed);

    dbgAssert(xyz.x != REALlyBig && xyz.y != REALlyBig && xyz.z != REALlyBig);
    dbgAssert(hpb.x != REALlyBig && hpb.y != REALlyBig && hpb.z != REALlyBig);

    nisObjectEulerToMatrix(&_3x3Matrix, &hpb);                //make a rotation matrix
    hmatMakeHMatFromMatAndVec(&rotMatrix, &_3x3Matrix, &xyz); //make a hmatrix
        *matrixDest = rotMatrix;
    //remember when this curve was last updated
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : madBindingsPost
    Description : Post-render callback for animated bindings
    Inputs      : mesh - ignored
                  bindings - binding list for the ship
                  currentLOD - ignored
    Outputs     : sets the animation's time elapsed to zero
    Return      : void
----------------------------------------------------------------------------*/
void madBindingsPost(meshdata *mesh, struct shipbindings *bindings, sdword currentLOD)
{
    mhlocalbinding *binding;
    Ship *ship;

    binding = bindings->localBinding[0];
    ship = (Ship *)binding->userData;
    dbgAssert(ship->madBindings);
    ship->madBindings->timeElapsed = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : madBindingListCallback
    Description : Recursive-walk function for creating binding lists
    Inputs      : mesh - mesh we're walking
                  object - object we're now stepping on
                  iObject - index of object
    Outputs     :
    Return      : FALSE (keep on walking)
----------------------------------------------------------------------------*/
bool32 madBindingListCallback(meshdata *mesh, polygonobject *object, sdword iObject)
{
    sdword j;

    //find the animation for this object
    for (j = 0; j < madDupeHeader->nObjects; j++)
    {
        if (object->nameCRC == madDupeHeader->objPath[j].nameCRC)
        {                                                   //if the CRC's match
            if (_strcmpi(object->pName, madDupeHeader->objPath[j].name) == 0)
            {                                               //if object name matches motion path name
                goto foundOne;
            }
        }
    }
/*
#if MAD_ERROR_CHECKING
#ifndef gshaw
    //couldn't find named object in the animation
    dbgMessagef("\nobject '%s' from '%s' not found in animation.", object->pName, mesh->fileName);
#endif
#endif
*/
    madBindingDest->userData = madDupeShip;                 //pass in pointer to ship
    madBindingDest->userID = 0;
    madBindingDest->flags = 0;
    madBindingDest->object = object;
    madBindingDest->function = NULL;                       //object not in animation; no binding
    madBindingDest++;                                       //skip to next binding struct
    return(FALSE);
foundOne:
    madBindingDest->function = madAnimBindingUpdate;        //mesh matrix update function
    madBindingDest->userData = madDupeShip;                 //pass in pointer to ship
    madBindingDest->userID = j;                             //pass in index of motion path
    madBindingDest->flags = 0;                              //flags not needed here
    madBindingDest->object = object;                        //object to animate
    madBindingDest->matrix = IdentityHMatrix;               //retail seeds animated bindings with an identity matrix

    madBindingDest++;                                       //skip to next binding struct
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : madAnimBindingsDupe
    Description : Make ship local mesh bindings for mesh animation
    Inputs      : ship - ship to make the bindings for
                  staticInfo - static info for that ship
    Outputs     : fills in the newship->madBindings->bindings structure with
                    bindings for all LOD's of the mesh.
    Return      : void
----------------------------------------------------------------------------*/
void madAnimBindingsDupe(Ship *ship, ShipStaticInfo *staticInfo,bool32 LoadingGame)
{
    shipbindings *bindings = &ship->madBindings->bindings;
    lodinfo *LOD = staticInfo->staticheader.LOD;
    sdword index, j, level;
    meshdata * mesh;
    splinecurve *curve;
    madobjpath *path;
    sdword curveIndex;

    madDupeHeader = staticInfo->madStatic->header;
    madDupeShip = ship;
    //splines occur right after the actual shipbinding structure
    curve = (splinecurve *)(((ubyte *)ship->madBindings) +
                                     madAnimSize(LOD->nLevels));
    ship->madBindings->curves = curve;
#if MESH_PRE_CALLBACK
    ship->madBindings->preCallback = NULL;                  //no precallback required
#endif
    ship->madBindings->bindings.postCallback = madBindingsPost;//clear the time elapsed to zero

    //binding structures allocated directly after the spline curves
    madBindingDest = (mhlocalbinding *)(((ubyte *)curve) + madDupeHeader->nObjects * sizeof(splinecurve) * 6);

    for (level = curveIndex = 0; level < LOD->nLevels; level++)
    {                                                       //for all LOD's of the ship
        if ((LOD->level[level].flags & LM_LODType) != LT_Mesh)
        {                                                   //if this level not a mesh
            bindings->localBinding[level] = NULL;           //no bindings for this level
            continue;
        }
        bindings->localBinding[level] = madBindingDest;        //binding list for this LOD
        mesh = (meshdata *)LOD->level[level].pData;         //get pointer to mesh
        meshHierarchyWalk(mesh, madBindingListCallback, NULL);
    }

    if (!LoadingGame)       // don't initialize curves, etc, if loading game
    {
        //'start' all the spline curves now.  They will be updated
        //later to reflect what animation they are to play.
        path = madDupeHeader->objPath;
        for (index = 0; index < madDupeHeader->nObjects; index++, path++)
        {
            for (j = 0; j < 6; j++, curve++)
            {
                bsCurveStartPrealloced(curve, path->nKeyframes, path->path[j], path->times, path->parameters);
            }
        }

        ship->madBindings->nPrevAnim = MAD_NoAnimation;     //Cataclysm: no previous animation
        ship->madBindings->nCurrentAnim = MAD_NoAnimation;      //start off by doing no animation
        ship->madBindings->nPendingAnim = -1;                //no pending activation
        ship->madBindings->bPaused = FALSE;
    }

    meshBindingsFixup(staticInfo, bindings);
}

// Cataclysm-specific: retail helper that zeros the MAD runtime state and marks
// current/previous animation slots as inactive before the live header/size are wired in.
void madAnimationInit(madanim *state)
{
    memset(state, 0, sizeof(*state));
    state->nCurrentAnim = MAD_NoAnimation;
    state->nPrevAnim = MAD_NoAnimation;
    state->nPendingAnim = 0;
}

/*-----------------------------------------------------------------------------
    Name        : madAnimationStart
    Description : Start an animation playing for a given ship
    Inputs      : ship - ship to play the animation for
                  animNumber - index of the animation
                  rate - rate of the animation 1.0 is normal animation speed.
                  -1 plays animation in reverse.
    Outputs     : sets time, rate and splinecurves up for an animation.
    Return      : void
----------------------------------------------------------------------------*/
void madAnimationStartWithFlags(Ship *ship, sdword animNumber, udword flags)
{
    madanim *anim = ship->madBindings;
    madheader *header = anim->header;
    sdword index, j, startPoint, remappedAnim;
    splinecurve *curve;
    madanimation *animation;
    udword animBit;

    if (anim->nCurrentAnim != MAD_NoAnimation)
    {
        if (anim->bPaused)
            madAnimationPause(ship, FALSE);
        anim->nPrevAnim = anim->nCurrentAnim;
        anim->nCurrentAnim = MAD_NoAnimation;
    }
    else
    {
        anim->nPrevAnim = MAD_NoAnimation;
    }

    dbgAssert(animNumber < header->nAnimations);
    anim->nCurrentAnim = animNumber;

    remappedAnim = madResolveAnimationIndex(ship, animNumber);
    animation = &header->anim[remappedAnim];
    anim->flags = flags;
    anim->startTime = anim->time = animation->startTime;
    anim->timeElapsed = 0.0f;
    anim->nPendingAnim = 1;

    animBit = (1 << animNumber);
    curve = anim->curves;
    for (index = 0; index < header->nObjects; index++)
    {
        if (!bitTest(header->objPath[index].animationBits, animBit))
        {
            curve->currentPoint = BS_NoPoint;
            curve += 6;
            continue;
        }
        for (startPoint = 0; startPoint < curve->nPoints; startPoint++)
        {
            if (curve->times[startPoint] == anim->time)
            {
                goto foundTime;
            }
        }
#if MAD_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "madAnimationStart: Object #%d ('%s') has no keyframe at frame %.0f",
              index, header->objPath[index].name, anim->time / header->framesPerSecond);
#endif
foundTime:
        for (j = 0; j < 6; j++, curve++)
        {
            curve->timeElapsed = anim->time;
            curve->currentPoint = startPoint;
        }
    }
#if MAD_VERBOSE_LEVEL >= 2
    dbgMessagef("\nmadAnimationStart: started animation #%d('%s') on ship 0x%x", animNumber, animation->name, ship);
#endif
}

void madAnimationStart(Ship *ship, sdword animNumber)
{
    madAnimationStartWithFlags(ship, animNumber, 0);
}

/*-----------------------------------------------------------------------------
    Name        : madAnimationUpdate
    Description : Update the mesh animation for an object
    Inputs      : ship - ship to update the animation for
                  timeElapsed - time elapsed since last updated
    Outputs     :
    Return      : TRUE if the animation ends
----------------------------------------------------------------------------*/
bool32 madAnimationUpdate(Ship *ship, real32 timeElapsed)
{
    madanim *anim = ship->madBindings;
    madheader *header;
    madanimation *animation;
    sdword remappedAnim;

    dbgAssert(anim != NULL);
    dbgAssert(anim->nCurrentAnim != MAD_NoAnimation);

    // Cataclysm: first-frame deferred binding activation
    if (anim->nPendingAnim != -1)
    {
        anim->nPendingAnim = -1;
        if (anim->saveBindings == NULL)
            anim->saveBindings = ship->bindings;
        ship->bindings = &anim->bindings;
    }

    header = anim->header;
    remappedAnim = madResolveAnimationIndex(ship, anim->nCurrentAnim);
    animation = &header->anim[remappedAnim];

    if (anim->bPaused)
        return FALSE;

    // TODO: sound event - requires event[] global not yet imported
    // if (anim->time == animation->startTime && anim->nCurrentAnim < 6 && shiptype < 60)
    //     soundEvent(ship, event[6 * shiptype + anim->nCurrentAnim]);

    if (anim->time + timeElapsed <= animation->endTime)
    {
        anim->time += timeElapsed;
        anim->timeElapsed += timeElapsed;
        return FALSE;
    }

    // Past end of animation
    if (bitTest(anim->flags, MAF_Loop))
    {
        do
        {
            anim->time = anim->time + timeElapsed - animation->endTime + animation->startTime;
        }
        while (anim->time + timeElapsed > animation->endTime);
        anim->timeElapsed += timeElapsed;
        return FALSE;
    }
    else if (bitTest(anim->flags, 0x2))
    {
        madAnimationStop(ship);
        return TRUE;
    }
    else
    {
        // Freeze at end
        anim->time = animation->endTime;
        madAnimationPause(ship, TRUE);
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : madAnimationStop
    Description : Stop an animation from playing.
    Inputs      : ship - ship to stop animating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madAnimationStop(Ship *ship)
{
    madanim *anim = ship->madBindings;

    dbgAssert(anim != NULL);

    if (anim->bPaused)
        madAnimationPause(ship, FALSE);

    if (anim->saveBindings != NULL)
    {
        ship->bindings = anim->saveBindings;
        anim->saveBindings = NULL;
    }

#if MAD_VERBOSE_LEVEL >= 2
    if (anim->nCurrentAnim != MAD_NoAnimation)
        dbgMessagef("\nmadAnimationStop: stopped animation #%d('%s') on ship 0x%x", anim->nCurrentAnim, anim->header->anim[anim->nCurrentAnim].name, ship);
#endif

    anim->nPrevAnim = anim->nCurrentAnim;
    anim->nCurrentAnim = MAD_NoAnimation;
}

/*-----------------------------------------------------------------------------
    Name        : madAnimIndexFindByName
    Description : Find the index of an animation by name
    Inputs      : header - header to search
                  name - name of animation to find
    Outputs     :
    Return      : index of animation, generates an error if not found
----------------------------------------------------------------------------*/
sdword madAnimIndexFindByName(madheader *header, char *name)
{
    sdword index;

    dbgAssert(header->stringBlock != NULL);

    for (index = 0; index < header->nAnimations; index++)
    {
        if (_stricmp(header->anim[index].name, name) == 0)
        {
            return(index);
        }
    }
    return(-1);
}

/*-----------------------------------------------------------------------------
    Name        : madBunBindingIndexFindByName
    Description : Find the index of a named mesh binding.
    Inputs      : info - static info to look for info in
                  name - name of binding to search for.
    Outputs     :
    Return      : index of named mesh object, if -1 if not found
----------------------------------------------------------------------------*/
sdword madGunBindingIndexFindByName(ShipStaticInfo *info, char *name)
{
    sdword index;
    mhbinding *list = info->staticheader.LOD->level[0].hBindings;
    meshdata *mesh = (meshdata *)info->staticheader.LOD->level[0].pData;

    dbgAssert(list != NULL);
    dbgAssert(mesh != NULL);

    for (index = 0; index < mesh->nPolygonObjects; index++)
    {                                                       //search rest of list
        if (list->object->pName != NULL)
        {
            if (strcmp(list->object->pName, name) == 0)
            {                                               //if names match
                return(index);                              //all done
            }
        }
        list++;                                             //update pointer
    }
    return(-1);
}


/*-----------------------------------------------------------------------------
    Name        : madBindingFindCallback
    Description : Mesh-walk callback for finding a mesh binding by name.
    Inputs      : mesh - mesh we're searching
                  object - current object we're on
                  iObject - index of object
    Outputs     :
    Return      : TRUE means stop walking, take a rest
----------------------------------------------------------------------------*/
/*
bool32 madBindingFindCallback(meshdata *mesh, polygonobject *object, sdword iObject)
{
    sdword j;

    if (!strcmp(object->pName, madDupeName))
    {
        madBindingIndex = madIncIndex;
        return(TRUE);
    }
    madIncIndex++;                                          //skip to next binding struct

    return(FALSE);
}
*/
/*-----------------------------------------------------------------------------
    Name        : madBindingIndexFindByName
    Description : Find the index of a binding by it's object's name.
    Inputs      : header - header of
    Outputs     :
    Return      : index of first binding to match the name or -1 if not found in animation.
----------------------------------------------------------------------------*/
sdword madBindingIndexFindByName(madheader *header, char *name)
{
    /*
    sdword level;
    dbgAssert(header != NULL);
    dbgAssert(name != NULL);

    madBindingIndex = 0xffffffff;
    madBindingName = name;
    for (level = madIncIndex = 0; level < LOD->nLevels; level++)
    {
        if ((LOD->level[level].flags & LM_LODType) != LT_Mesh)
        {                                                   //if this level not a mesh
            continue;
        }
        meshHierarchyWalk((meshdata *)LOD->level[level].pData, madBindingFindCallback, NULL);
    }

    dbgAssert(madBindingIndex != 0xffffffff);
    return(madBindingIndex);
    */
    sdword index;

    for (index = 0; index < header->nObjects; index++)
    {
        if (!strcmp(header->objPath[index].name, name))
        {
            return(index);                                  //found, return index of it
        }
    }
    return(-1);                                             //not found, return error code
}

/*-----------------------------------------------------------------------------
    Name        : madAnimFreeze
    Description : Pause or unpause an animation in it's current position.
    Inputs      : ship - ship to pause
                  bFreeze - TRUE to pause, false to resume
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void madAnimationPause(Ship *ship, bool32 bFreeze)
{
    madanim *anim = ship->madBindings;
    sdword index;
    mhlocalbinding *binding;

    dbgAssert(anim != NULL);

    binding = anim->bindings.localBinding[0];

    if (bFreeze)
    {                                                       //if freezing
        for (index = ship->staticinfo->hierarchySize - 1; index >= 0; index--, binding++)
        {
            if (binding->function == madAnimBindingUpdate)
            {                                               //if this binding animating
                madAnimBindingUpdate(binding->flags, &binding->object->localMatrix,//bring animation 'up to date'
                              &binding->matrix, binding->userData, binding->userID);
                binding->function = madAnimBindingPause;    //simplified update function
            }
        }
    }
    else
    {                                                       //else unpausing
        for (index = ship->staticinfo->hierarchySize - 1; index >= 0; index--, binding++)
        {
            if (binding->function == madAnimBindingPause)
            {                                               //if this binding animating
                binding->function = madAnimBindingUpdate;   //simplified update function
            }
        }
    }
    anim->bPaused = bFreeze;
}

/*-----------------------------------------------------------------------------
    Name        : madAnimBindingMatrix
    Description : Retrieve the matrix for a given ship's animation bindings.
    Inputs      : ship - ship to get matrix for
                  gunIndex - index returned by meshBindingIndexFindByName
                  madIndex - index returned by madAnimIndexFindByName
    Outputs     : destMatrix - where to put the matrix
                  destPos - where to put the position vector
    Return      : pointer to matrix
----------------------------------------------------------------------------*/
void madAnimBindingMatrix(matrix *destMatrix, vector *destPos, Ship *ship, sdword gunIndex, sdword madIndex)
{
    madanim *anim = ship->madBindings;
    splinecurve *curve;
    vector vec[2];
    real32 *value = (real32 *)&vec;
    lodinfo *lodInfo;
    mhbinding *bindingListSource;
    hmatrix *hMatrix;
    hmatrix hMatrixTemp;
    meshdata *mesh;
    splinecurve saveCurve[6];
    real32 saveTimeElapsed, saveTime;
    sdword index;

    dbgAssert(anim != NULL);
    if (anim->nCurrentAnim != MAD_NoAnimation)
    {                                                       //if there's an animation playing
        curve = &anim->curves[madIndex * 6];                //pointer to spline curves for this object
        memcpy(saveCurve, curve, sizeof(splinecurve) * 6);  //save copies of the spline curves
        saveTime = anim->time;                              //save time elapsed flags
        saveTimeElapsed = anim->timeElapsed;
        for (index = 0; index < 6; index++)
        {                                                   //rewind all the splines
            curve[index].timeElapsed = 0.0f;
            curve[index].currentPoint = 0;
        }
        anim->timeElapsed = anim->time;                     //advance all splines to deterministic animation point
        madAnimBindingUpdate(0, NULL, &hMatrixTemp, (void *)ship, madIndex);//make like we're updating the curves
        memcpy(curve, saveCurve, sizeof(splinecurve) * 6);  //restore the saved curves
        anim->time = saveTime;                              //restore animation time info
        anim->timeElapsed = saveTimeElapsed;
        matGetMatFromHMat(destMatrix, &hMatrixTemp);        //convert to 3x3/position
        hmatGetVectFromHMatrixCol4(*destPos, hMatrixTemp);
    }
    else
    {                                                       //no animation playing
        lodInfo = ship->staticinfo->staticheader.LOD;
        dbgAssert((lodInfo->level[0].flags & LM_LODType) == LT_Mesh);//make sure right type
        mesh = (meshdata *)lodInfo->level[0].pData;
        bindingListSource = lodInfo->level[0].hBindings;    //pointer to LOD0 bindings
        dbgAssert(gunIndex < mesh->nPolygonObjects);
        hMatrix = &mesh->object[gunIndex].localMatrix;      //get the matrix
        matGetMatFromHMat(destMatrix, hMatrix);             //convert to 3x3/position
        hmatGetVectFromHMatrixCol4(*destPos, *hMatrix);
    }
}

/*=============================================================================
    Save Game Stuff:
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void PreFix_madBindings(Ship *ship,Ship *fixcontents)
{
    sdword i;

    sdword madOffset = ((ubyte *)ship->madBindings) - ((ubyte *)ship);
    madanim *madBindings = (madanim *)(((ubyte *)fixcontents) + madOffset);

    sdword curvesOffset = ((ubyte *)ship->madBindings->curves) - ((ubyte *)ship->madBindings);
    splinecurve *curvesToModify = (splinecurve *)(((ubyte *)madBindings) + curvesOffset);

    sdword numCurves = 6 * madBindings->header->nObjects;

    for (i=0;i<numCurves;i++)
    {
        // store offsets from madBindings->header
        curvesToModify[i].points = ((ubyte *)curvesToModify[i].points) - ((ubyte *)madBindings->header);
        curvesToModify[i].times = ((ubyte *)curvesToModify[i].times) - ((ubyte *)madBindings->header);
        curvesToModify[i].params = ((ubyte *)curvesToModify[i].params) - ((ubyte *)madBindings->header);
    }

    // store offset from madBindings
    //madBindings->curves = ((ubyte *)&madBindings->curves) - ((ubyte *)madBindings);
}

void Save_madBindings(Ship *ship)
{
    // Cataclysm retail: no-op. The MAD body is serialized inline in the ship chunk.
    (void)ship;
}

void Load_madBindings(Ship *ship)
{
    // Cataclysm retail: no-op. Fix_madBindings reconstructs runtime pointers later.
    (void)ship;
}

void Fix_madBindings(Ship *ship)
{
    madanim *madBindings = ship->madBindings;
    splinecurve *curve;
    sdword i;
    sdword numCurves;

    madBindings->header = ship->staticinfo->madStatic->header;

    madAnimBindingsDupe(ship, ship->staticinfo, TRUE);

    numCurves = 6 * madBindings->header->nObjects;

    for (i = 0; i < numCurves; i++)
    {
        curve = &madBindings->curves[i];

        // fix, offsets from madBindings->header
        curve->points = (real32 *)(((ubyte *)madBindings->header) + ((udword)curve->points));
        curve->times = (real32 *)(((ubyte *)madBindings->header) + ((udword)curve->times));
        curve->params = (tcb *)(((ubyte *)madBindings->header) + ((udword)curve->params));
    }

    if (madBindings->nCurrentAnim == MAD_NoAnimation)
    {
        madBindings->saveBindings = NULL;
    }
    else
    {
        madBindings->saveBindings = ship->bindings;
        ship->bindings = &madBindings->bindings;
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"



#if 0  // Cat2: Binary madBindingListCallback; live implementation at meshanim.c L273 (0x4B68F0)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x4B68F0
int __cdecl madBindingListCallback(meshdata *mesh, polygonobject *object, sdword iObject)
{
int v2; // ecx
int v3; // esi
int v4; // edi
int v5; // eax

v2 = dword_A4C4DC;
v3 = 0;
if ( *(int *)(dword_A4C4DC + 28) <= 0 )
{
LABEL_7:
*(_DWORD *)(dword_A4C4E4 + 4) = dword_A4C4E0;
*(_DWORD *)dword_A4C4E4 = 0;
*(_DWORD *)(dword_A4C4E4 + 16) = a2;
dword_A4C4E4 += 88;
return 0;
}
v4 = 0;
while ( 1 )
{
v5 = v4 + *(_DWORD *)(v2 + 32);
if ( *(_WORD *)(a2 + 6) == *(_WORD *)(v5 + 4) )
break;
LABEL_6:
++v3;
v4 += 48;
if ( v3 >= *(_DWORD *)(v2 + 28) )
goto LABEL_7;
}
if ( _strcmpi(*(const char **)a2, *(const char **)v5) )
{
v2 = dword_A4C4DC;
goto LABEL_6;
}
*(_DWORD *)dword_A4C4E4 = madAnimBindingUpdate;
*(_DWORD *)(dword_A4C4E4 + 4) = dword_A4C4E0;
*(_DWORD *)(dword_A4C4E4 + 8) = v3;
*(_DWORD *)(dword_A4C4E4 + 12) = 0;
*(_DWORD *)(dword_A4C4E4 + 16) = a2;
qmemcpy((void *)(dword_A4C4E4 + 20), &unk_72AED8, 0x40u);
dword_A4C4E4 += 88;
return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4B69D0
void __cdecl madAnimationInit(madAnimState *state)
{
_DWORD *result; // eax

result = a1;
*a1 = 0;
a1[1] = 0;
a1[2] = 0;
a1[3] = -1;
a1[4] = -1;
a1[5] = 0;
a1[7] = 0;
a1[8] = 0;
a1[9] = 0;
a1[10] = 0;
a1[11] = 0;
a1[6] = 0;
a1[12] = 0;
a1[13] = 0;
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4B6FB0
void __cdecl PreFix_madBindings(Ship *ship, Ship *fixcontents)
{
int v2; // ecx
int v3; // edx
_DWORD *result; // eax

v2 = a2 + *(_DWORD *)(a1 + 1212) - a1;
v3 = 6 * *(_DWORD *)(*(_DWORD *)v2 + 28);
result = (_DWORD *)(v2 + *(_DWORD *)(*(_DWORD *)(a1 + 1212) + 40) - *(_DWORD *)(a1 + 1212));
if ( v3 > 0 )
{
result += 4;
do
{
*(result - 1) -= *(_DWORD *)v2;
*result -= *(_DWORD *)v2;
result[1] -= *(_DWORD *)v2;
result += 6;
--v3;
}
while ( v3 );
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4B7000
void __cdecl Fix_madBindings(Ship *ship)
{
_DWORD *madAnimationData; // esi
int result; // eax
int v3; // ecx
int v4; // edx
int v5; // eax
int v6; // ebp

madAnimationData = (_DWORD *)ship->runtimeAnimationOverlay.madAnimationData;
*madAnimationData = **(_DWORD **)&ship->staticinfo->unk7[4];
madAnimBindingsDupe(ship, ship->staticinfo, 1);
result = 6 * *(_DWORD *)(*madAnimationData + 28);
if ( result > 0 )
{
v3 = 0;
v4 = 6 * *(_DWORD *)(*madAnimationData + 28);
do
{
v5 = madAnimationData[10];
v6 = *(_DWORD *)(v5 + v3 + 12);
result = v3 + v5;
*(_DWORD *)(result + 12) = *madAnimationData + v6;
*(_DWORD *)(result + 16) += *madAnimationData;
v3 += 24;
--v4;
*(_DWORD *)(result + 20) += *madAnimationData;
}
while ( v4 );
}
if ( madAnimationData[3] == -1 )
{
madAnimationData[11] = 0;
}
else
{
madAnimationData[11] = ship->runtimeAnimationOverlay.madOwnedBindings;
ship->runtimeAnimationOverlay.madOwnedBindings = (udword)(madAnimationData + 12);
}
return result;
}

#endif

bool32 madAnimationCompleted(Ship *ship)
{
    madanim *animation = ship->madBindings;
    sdword currentAnim;
    sdword remappedAnim;

    dbgAssert(animation != NULL);
    currentAnim = animation->nCurrentAnim;
    if (currentAnim == MAD_NoAnimation)
    {
        return TRUE;
    }

    remappedAnim = madResolveAnimationIndex(ship, currentAnim);
    return (currentAnim == MAD_NoAnimation ||
            animation->time >= animation->header->anim[remappedAnim].endTime ||
            animation->bPaused);
}
