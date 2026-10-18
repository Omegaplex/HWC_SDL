/*=============================================================================
    Name    : LOD.c
    Purpose : Service functions for level-of-detail

    Created 7/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "statscript.h"
#include "memory.h"
#include "spaceobj.h"
#include "Ship.h"
#include "debug.h"
#include "file.h"
#include "key.h"
#include "qpcfix.h"
#include "texreg.h"
#include "LOD.h"
#include "Options.h"
#include "shader.h"

extern meshdata *defaultmesh;          // hack for now.  remove later when defaults no longer needed
extern LARGE_INTEGER utyTimerFrequency;

/*=============================================================================
    Data:
=============================================================================*/
//script-parsing function for LOD files
static void lodTypeRead(char *directory,char *field,void *dataToFillIn);
static void lodMeshFileLoad(char *directory,char *field,void *dataToFillIn);
static void lodSpriteFileRead(char *directory,char *field,void *dataToFillIn);
static void lodColorScalarRead(char *directory,char *field,void *dataToFillIn);
sdword lodBuildMeshBindingsRecursive(lodmeshbinding *bindings, sdword index, polygonobject *object);
sdword disableMeshBindings = FALSE;
static lodinfo lodStaticInfo;
scriptStructEntry lodScriptTable[] =
{
    { "pointColor",   scriptSetRGBCB,    (udword)(&lodStaticInfo.pointColor),     (udword)(&lodStaticInfo)},
    { "useHomeworldLODSystem", scriptSetUdwordCB, (udword)(&lodStaticInfo.useHomeworldLODSystem), (udword)(&lodStaticInfo)},
    { "type0",        lodTypeRead,       (udword)(&lodStaticInfo.level[0]),       (udword)(&lodStaticInfo)},
    { "type1",        lodTypeRead,       (udword)(&lodStaticInfo.level[1]),       (udword)(&lodStaticInfo)},
    { "type2",        lodTypeRead,       (udword)(&lodStaticInfo.level[2]),       (udword)(&lodStaticInfo)},
    { "type3",        lodTypeRead,       (udword)(&lodStaticInfo.level[3]),       (udword)(&lodStaticInfo)},
    { "type4",        lodTypeRead,       (udword)(&lodStaticInfo.level[4]),       (udword)(&lodStaticInfo)},
    { "type5",        lodTypeRead,       (udword)(&lodStaticInfo.level[5]),       (udword)(&lodStaticInfo)},
    { "mOn0",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].mOn),   (udword)(&lodStaticInfo)},
    { "mOn1",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].mOn),   (udword)(&lodStaticInfo)},
    { "mOn2",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].mOn),   (udword)(&lodStaticInfo)},
    { "mOn3",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].mOn),   (udword)(&lodStaticInfo)},
    { "mOn4",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].mOn),   (udword)(&lodStaticInfo)},
    { "mOn5",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].mOn),   (udword)(&lodStaticInfo)},
    { "mOff0",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].mOff),  (udword)(&lodStaticInfo)},
    { "mOff1",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].mOff),  (udword)(&lodStaticInfo)},
    { "mOff2",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].mOff),  (udword)(&lodStaticInfo)},
    { "mOff3",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].mOff),  (udword)(&lodStaticInfo)},
    { "mOff4",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].mOff),  (udword)(&lodStaticInfo)},
    { "mOff5",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].mOff),  (udword)(&lodStaticInfo)},
    { "bOn0",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].bOn),   (udword)(&lodStaticInfo)},
    { "bOn1",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].bOn),   (udword)(&lodStaticInfo)},
    { "bOn2",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].bOn),   (udword)(&lodStaticInfo)},
    { "bOn3",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].bOn),   (udword)(&lodStaticInfo)},
    { "bOn4",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].bOn),   (udword)(&lodStaticInfo)},
    { "bOn5",         scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].bOn),   (udword)(&lodStaticInfo)},
    { "bOff0",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[0].bOff),  (udword)(&lodStaticInfo)},
    { "bOff1",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[1].bOff),  (udword)(&lodStaticInfo)},
    { "bOff2",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[2].bOff),  (udword)(&lodStaticInfo)},
    { "bOff3",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[3].bOff),  (udword)(&lodStaticInfo)},
    { "bOff4",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[4].bOff),  (udword)(&lodStaticInfo)},
    { "bOff5",        scriptSetReal32CB, (udword)(&lodStaticInfo.level[5].bOff),  (udword)(&lodStaticInfo)},
    { "pMeshFile0",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[0].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile1",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[1].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile2",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[2].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile3",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[3].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile4",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[4].pData), (udword)(&lodStaticInfo)},
    { "pMeshFile5",   lodMeshFileLoad,   (udword)(&lodStaticInfo.level[5].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile0", lodSpriteFileRead, (udword)(&lodStaticInfo.level[0].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile1", lodSpriteFileRead, (udword)(&lodStaticInfo.level[1].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile2", lodSpriteFileRead, (udword)(&lodStaticInfo.level[2].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile3", lodSpriteFileRead, (udword)(&lodStaticInfo.level[3].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile4", lodSpriteFileRead, (udword)(&lodStaticInfo.level[4].pData), (udword)(&lodStaticInfo)},
    { "pSpriteFile5", lodSpriteFileRead, (udword)(&lodStaticInfo.level[5].pData), (udword)(&lodStaticInfo)},
    { "baseScalar",   lodColorScalarRead,0,            0},
    { "stripeScalar", lodColorScalarRead,2,          1},
    { NULL,NULL,0,0 }
};

//scaling factor for the LOD's
real32 lodScaleFactor = LOD_ScaleFactor;
static const real32 lodDetailBucketMax = 0.4000000059604645f;
static const real32 lodDetailBucket1 = 0.11999999731779099f;
static const real32 lodDetailBucket2 = 0.04500000178813934f;
static const real32 lodDetailBucket3 = 0.017000000923871994f;
static const real32 lodDetailBucket4 = 0.006500000134110451f;
static const ubyte lodStabilizationPassLODIndex[11] = { 99, 3, 2, 1, 0, 3, 2, 1, 3, 2, 3 };
static real32 LODStabilizationTime = 0.0f;

static sdword lodCountMeshPolygonsRecursive(polygonobject *object)
{
    if (object == NULL)
    {
        return 0;
    }
    return object->nPolygons +
        lodCountMeshPolygonsRecursive(object->pDaughter) +
        lodCountMeshPolygonsRecursive(object->pSister);
}

static sdword lodCountTotalPolygons(lod *lodLevel)
{
    meshdata *mesh;

    if (lodLevel == NULL || lodLevel->pData == NULL)
    {
        return 0;
    }
    mesh = (meshdata *)lodLevel->pData;
    return lodCountMeshPolygonsRecursive(&mesh->object[0]);
}

//fixed version of above factor for debugging reasons
#if LOD_SCALE_DEBUG
real32 lodDebugScaleFactor = 0.0f;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : lodTableReadScript
    Description : Load in and allocate a level-of-detail table from a script
                    file.
    Inputs      : fileName - name of script file to read from
    Outputs     : Allocates a LOD table and fills it in.
    Return      : newly allocated LOD table.
----------------------------------------------------------------------------*/
lodmaxinfo lodMaxInfo;
lodinfo *lodTableReadScript(char *directory, char *fileName)
{
    lodinfo *info;
    sdword index;

    lodMaxInfo.useHomeworldLODSystem = 0;
    for (index = 0; index < LOD_NumberLevels; index++)
    {                                                       //initialize the lodmaxinfo structure
        lodMaxInfo.level[index].flags = LT_Invalid;
        lodMaxInfo.level[index].pData = NULL;
        lodMaxInfo.level[index].nMatrices = 0;
        lodMaxInfo.level[index].hBindings = NULL;
        lodMaxInfo.level[index].timingSampleIndex = 0;
        memset(lodMaxInfo.level[index].timingSample, 0, sizeof(lodMaxInfo.level[index].timingSample));
        lodMaxInfo.level[index].stabilizedMetric = 0;
    }

    trBaseColorScalar = trStripeColorScalar = 0.0f;
    scriptSetStruct(directory, fileName, lodScriptTable, (ubyte *)&lodMaxInfo);//read the script file
    trBaseColorScalar = trStripeColorScalar = 0.0f;

    for (index = 0; index < LOD_NumberLevels; index++)
    {
        if ((lodMaxInfo.level[index].flags & LM_LODType) == LT_Invalid)
        {
            break;
        }
        if ((lodMaxInfo.level[index].flags & LM_LODType) == LT_Mesh)
        {                                                   //if it's a mesh
            lodMaxInfo.level[index].hBindings =                //create a bindings list
                meshBindingListCreate((meshdata *)lodMaxInfo.level[index].pData);
        }
        else
        {
            lodMaxInfo.level[index].hBindings = NULL;
        }
    }
    info = memAlloc(lodTableSize(index), "LOD Table", NonVolatile); //allocate the table
    memcpy(info, &lodMaxInfo, lodTableSize(index));            //and copy over the max table
    info->nLevels = index;                                  //store number of levels
    return(info);
}

/*=============================================================================
    Callback functions for script reading
=============================================================================*/
//set correct LOD type
static void lodColorScalarRead(char *directory,char *field,void *dataToFillIn)
{
    real32 data;

    sscanf(field, "%f", &data);
    dbgAssert(data >= 0.0f && data <= 10.0f);
    if (data == 0.0f)
    {                                                       //0 is regarded as 'no change', hence 1.0
        data = 1.0f;
    }
    if (dataToFillIn == (void *)(&lodMaxInfo))
    {
        trBaseColorScalar = data;
    }
    else
    {
        trStripeColorScalar = data;
    }
}
//set correct LOD type
static void lodTypeRead(char *directory,char *field,void *dataToFillIn)
{                                                           //set correct LOD type
    lod *dest;

    dest = (lod *)dataToFillIn;
    _strupr(field);
    if (strstr(field, "INVALID"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_Invalid);
    }
    else if (strstr(field, "MESH"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_Mesh);
    }
    else if (strstr(field, "TINYSPRITE"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_TinySprite);
    }
    else if (strstr(field, "SUBPIXEL"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_SubPixel);
    }
    else if (strstr(field, "FUNCTION"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_Function);
    }
    else if (strstr(field, "NULL"))
    {
        dest->flags = (udword)((dest->flags & (~LM_LODType)) | LT_NULL);
    }
#if LOD_ERROR_CHECKING
    else
    {
        dbgFatalf(DBG_Loc, "Invalid LOD type: '%s'", field);
    }
#endif
}
//load in a mesh
static void lodMeshFileLoad(char *directory,char *field,void *dataToFillIn)
{
    char fullfilename[80];
#if LOD_VERBOSE_LEVEL >= 1
//    dbgMessagef("\nlodMeshFileLoad: %s", field);
#endif
    if (directory != NULL)
    {
        strcpy(fullfilename,directory);
        strcat(fullfilename,field);
    }
    else
    {
        strcpy(fullfilename,field);
    }

    if (fileExists(fullfilename,0) || meshPagedVersionExists(fullfilename))
    {
        *((meshdata **)dataToFillIn) = meshLoad(fullfilename);
    }
    else
    {
        *((meshdata **)dataToFillIn) = defaultmesh;
    }
}
//load in a tiny sprite file
static void lodSpriteFileRead(char *directory,char *field,void *dataToFillIn)
{
#if LOD_VERBOSE_LEVEL >= 1
    dbgMessagef("\nlodSpriteFileRead: %s", field);
#endif
    dbgFatal(DBG_Loc, "Can't load sprite images yet!");
}

/*-----------------------------------------------------------------------------
    Name        : lodLevelGet
    Description : Get level of detail for specified ship
    Inputs      : spaceObj - space object to get
                  camera, ship - location of ship and camera, respectively
    Outputs     : updates the currentLOD of the specified space object.
    Return      : lod structure for current level of detail
----------------------------------------------------------------------------*/
#if LOD_PRINT_DISTANCE
sdword rndLOD = 0;
sdword lodTuningMode = FALSE;
sdword lodDrawingMode = FALSE;
#endif
lod *lodLevelGet(void *spaceObj, vector *camera, vector *ship)
{
    SpaceObj *obj = (SpaceObj *)spaceObj;
    lodinfo *info = shipGetLodInfo(obj);
    real32 lodMetric;
    real32 distance;

    dbgAssert(info != NULL);                                //verify the LOD table exists
    dbgAssert(info->nLevels > 0);

    vecSub(obj->cameraDistanceVector,*camera,*ship);
    lodMetric = ((SpaceObjRotImpTarg *)obj)->collInfo.selCircleRadius;
    obj->cameraDistanceSquared = distance = vecMagnitudeSquared(obj->cameraDistanceVector);

#if LOD_SCALE_DEBUG
    if (lodDebugScaleFactor != 0.0f)
    {
        lodScaleFactor = lodDebugScaleFactor;
    }
#endif

    if (!info->useHomeworldLODSystem)
    {
        if (lodMetric < 0.0f || lodMetric > lodDetailBucketMax)
        {
            obj->currentLOD = (ubyte)DetailQuality;
        }
        else if (lodMetric > lodDetailBucket1)
        {
            obj->currentLOD = (ubyte)(DetailQuality + 1);
        }
        else if (lodMetric > lodDetailBucket2)
        {
            obj->currentLOD = (ubyte)(DetailQuality + 2);
        }
        else if (lodMetric > lodDetailBucket3)
        {
            obj->currentLOD = (ubyte)(DetailQuality + 3);
        }
        else if (lodMetric > lodDetailBucket4)
        {
            obj->currentLOD = (ubyte)(DetailQuality + 4);
        }
        else
        {
            obj->currentLOD = (ubyte)(DetailQuality + 5);
        }

        if (obj->currentLOD >= info->nLevels)
        {
            obj->currentLOD = (ubyte)(info->nLevels - 1);
        }
        return &info->level[obj->currentLOD];
    }

    if (distance > info->level[obj->currentLOD].bOff * lodScaleFactor)
    {
        do
        {
            obj->currentLOD++;
            if (obj->currentLOD >= info->nLevels)
            {
                obj->currentLOD = (ubyte)(info->nLevels - 1);
                break;
            }
        }
        while (distance > info->level[obj->currentLOD].bOff * lodScaleFactor);
    }
    else
    {
        while (obj->currentLOD > 0 &&
               distance < info->level[obj->currentLOD - 1].bOn * lodScaleFactor)
        {
            obj->currentLOD--;
        }
    }

    if (obj->currentLOD < DetailQuality)
    {
        if (DetailQuality < info->nLevels)
        {
            obj->currentLOD = (ubyte)DetailQuality;
        }
        else
        {
            obj->currentLOD = (ubyte)(info->nLevels - 1);
        }
    }

    dbgAssert(obj->currentLOD >= 0);
    dbgAssert(obj->currentLOD < info->nLevels);             //verify we are within the available levels of detail
    return(&info->level[obj->currentLOD]);                  //return pointer to lod structure
}

/*-----------------------------------------------------------------------------
    Name        : lodPanicLevelGet
    Description : Get level of detail for specified ship
    Inputs      : spaceObj - space object to get
                  camera, ship - location of ship and camera, respectively
    Outputs     : updates the currentLOD of the specified space object.
    Return      : lod structure for current level of detail
----------------------------------------------------------------------------*/
lod* lodPanicLevelGet(void* spaceObj, vector* camera, vector* ship)
{
    SpaceObj* obj = (SpaceObj*)spaceObj;
    lod* LOD = lodLevelGet(spaceObj, camera, ship);
    lodinfo* info = shipGetLodInfo(obj);

    if (obj->currentLOD == 3)
    {
        obj->currentLOD++;
        if (obj->currentLOD >= info->nLevels)
        {
            obj->currentLOD--;
        }
        if ((info->level[obj->currentLOD].flags & LM_LODType) != LT_Mesh)
        {
            obj->currentLOD--;
        }
    }

    return(&info->level[obj->currentLOD]);
}

/*-----------------------------------------------------------------------------
    Name        : lodFree
    Description : frees LOD
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lodFree(lodinfo *LOD)
{
    sdword i;
    void *pData;

    for (i=0;i<LOD->nLevels;i++)
    {
        pData = LOD->level[i].pData;
        switch (LOD->level[i].flags)
        {
            case LT_Mesh:
                dbgAssert(LOD->level[i].hBindings != NULL);
                meshBindingListDelete((meshdata *)pData, LOD->level[i].hBindings);
                if ((pData != defaultmesh) && (pData != NULL))
                {
                    meshFree((meshdata *)pData);
                }
                break;

            default:        // later add other stuff you want to free such as sprites Luke.
                break;
        }
    }

    memFree(LOD);
}

/*-----------------------------------------------------------------------------
    Name        : lodAllMeshesRecolorize
    Description : Recolorizes all meshes in a particular LOD to reflect changes
                    in team color schemes.
    Inputs      : LOD - what LOD to recolorize;
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void lodAllMeshesRecolorize(lodinfo *LOD)
{
    sdword index;

    for (index = 0; index < LOD->nLevels; index++)
    {                                                       //for each level in this LOD
        if ((LOD->level[index].flags & LM_LODType) == LT_Mesh)
        {                                                   //if it's a mesh type
            meshRecolorize((meshdata *)LOD->level[index].pData);//recolorize it
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : lodHierarchySizeCompute
    Description : Compute the number of objects in all meshes in an LOD
    Inputs      : LOD - LOD to count the objects in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword lodHierarchySizeCompute(lodinfo *LOD)
{
    sdword index, size;
    lod *level;

    for (index = size = 0; index < LOD->nLevels; index++)
    {                                                       //for all levels of detail
        level = &LOD->level[index];
        if ((level->flags & LM_LODType) != LT_Mesh)
        {
            continue;
        }
        size += ((meshdata *)level->pData)->nPolygonObjects;
    }
    return(size);
}


/*-----------------------------------------------------------------------------
    Name        : lodAutoSave
    Description : Save out an LOD file after internal tuning.
    Inputs      : LOD - what LOD to save
                  fileName - full path of the .lod script file to overwrite
    Outputs     :
    Return      : 0 (success) or ERROR
    Note        : Prints a warning if the .LOD file cannot be opened for writing.
                  fileName is supplied by the caller because the runtime lodinfo
                  struct no longer tracks a source path (that slot is
                  useHomeworldLODSystem in Cataclysm - see lodLevelGet).
----------------------------------------------------------------------------*/
#if LOD_AUTO_SAVE
char *lodTypeStrings[] = {"INVALID", "MESH", "TINYSPRITE", "SUBPIXEL", "FUNCTION", "NULL"};
sdword lodAutoSave(lodinfo *LOD, char *fileName)
{
    sdword level;
    FILE *fp;
    char *filePath;

    if (fileName == NULL)
    {
        dbgWarningf(DBG_Loc, "Cannot auto-save LOD without a source filename.");
        return(ERROR);
    }

    fp = fopen(fileName, "wt");
    if (fp == NULL)
    {
        dbgWarningf(DBG_Loc, "Cannot open '%s' for writing - not checked out?", fileName);
        return(ERROR);
    }
    fprintf(fp, "[Auto-Saved LOD file]\n");
    fprintf(fp, "pointColor                  %d,%d,%d\n\n", colRed(LOD->pointColor), colGreen(LOD->pointColor), colBlue(LOD->pointColor));

    for (level = 0; level < LOD->nLevels; level++)
    {
        fprintf(fp, "\n");
        fprintf(fp, "type%d                       %s\n", level, lodTypeStrings[LOD->level[level].flags & LM_LODType]);
        if ((LOD->level[level].flags & LM_LODType) == LT_Mesh)
        {
            filePath = strchr(((meshdata *)LOD->level[level].pData)->fileName, '\\') + 1;
#if LOD_ERROR_CHECKING
            if (*filePath == 0)
            {
                dbgFatalf(DBG_Loc, "Could not find '\\' in '%s'", ((meshdata *)LOD->level[level].pData)->fileName);
            }
#endif
            fprintf(fp, "pMeshFile%d                  %s\n", level, filePath);
        }
        fprintf(fp, "mOn%d                        %.1f\n", level, LOD->level[level].mOn);
        fprintf(fp, "bOn%d                        %.0f\n", level, LOD->level[level].bOn);
        fprintf(fp, "mOff%d                       %.1f\n", level, LOD->level[level].mOff);
        fprintf(fp, "bOff%d                       %.0f\n", level, LOD->level[level].bOff);
    }
    fclose(fp);
    return(0);
}
#endif

// ============================================================================

lodmeshbindings *lodMeshBindingsAllocate(void *object)
{
    lodmeshbindings *bindings;
    SpaceObj *obj;
    Ship *ship;
    lodinfo *lodInfo;
    sdword levelIndex;

    obj = (SpaceObj *)object;
    if (disableMeshBindings || obj->objtype != OBJ_ShipType)
    {
        obj->posinfo.lodMeshBindings = NULL;
        return NULL;
    }

    ship = (Ship *)obj;
    lodInfo = ship->staticinfo->staticheader.LOD;
    bindings = memAlloc(sizeof(sdword) + sizeof(lodmeshbinding *) * lodInfo->nLevels,
                        "LodMeshBindings",
                        NonVolatile);
    obj->posinfo.lodMeshBindings = bindings;
    bindings->nLevels = lodInfo->nLevels;

    for (levelIndex = 0; levelIndex < lodInfo->nLevels; levelIndex++)
    {
        lod *lodLevel;

        bindings->level[levelIndex] = NULL;
        lodLevel = &lodInfo->level[levelIndex];
        if ((lodLevel->flags & LM_LODType) == LT_Mesh)
        {
            meshdata *mesh;
            lodmeshbinding *levelBindings;
            polygonobject *object;
            sdword objectIndex;

            mesh = (meshdata *)lodLevel->pData;
            levelBindings = memAlloc(sizeof(lodmeshbinding) * mesh->nPolygonObjects,
                                     "LodMeshBindingLevel",
                                     NonVolatile);
            objectIndex = 0;
            for (object = &mesh->object[0]; object != NULL; object = object->pSister)
            {
                objectIndex = lodBuildMeshBindingsRecursive(levelBindings, objectIndex, object);
            }
            bindings->level[levelIndex] = levelBindings;
        }
    }

    return bindings;
}

/*-----------------------------------------------------------------------------
    Name        : lodUpdateStabilize
    Description : Advance through retail stabilization passes for the current LOD.
    Inputs      : spaceobj - object to update
                  distanceIndex - render pass index
    Outputs     : may step currentLOD and cache the stabilized metric
    Return      : stabilized metric for the current LOD
-----------------------------------------------------------------------------*/
udword lodUpdateStabilize(SpaceObj *spaceobj, sdword distanceIndex)
{
    lodinfo *info;
    lod *lodLevel;
    ubyte currentLOD;

    info = shipGetLodInfo(spaceobj);
    currentLOD = spaceobj->currentLOD;
    if (lodStabilizationPassLODIndex[distanceIndex] == currentLOD)
    {
        currentLOD++;
        spaceobj->currentLOD = currentLOD;
        if (currentLOD >= info->nLevels)
        {
            spaceobj->currentLOD = (ubyte)(info->nLevels - 1);
        }
    }

    lodLevel = &info->level[spaceobj->currentLOD];
    if (LODStabilizationTime == 0.0f)
    {
        return lodLevel->stabilizedMetric;
    }

    if (lodLevel->timingSampleIndex < 4 && lodLevel->stabilizedMetric == 0)
    {
        udword stabilizedMetric;
        ubyte sampleIndex;

        stabilizedMetric = (udword)((real64)lodCountTotalPolygons(lodLevel) *
                                    (real64)utyTimerFrequency.LowPart *
                                    (real64)LODStabilizationTime + 0.5);
        sampleIndex = lodLevel->timingSampleIndex;
        lodLevel->stabilizedMetric = stabilizedMetric;
        while (sampleIndex < 4)
        {
            lodLevel->timingSample[sampleIndex] = stabilizedMetric;
            sampleIndex++;
        }
    }
    return lodLevel->stabilizedMetric;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x541B80
sdword lodBuildMeshBindingsRecursive(lodmeshbinding *bindings, sdword index, polygonobject *object)
{
    lodmeshbinding *binding;
    polygonobject *daughter;
    shSpecularData *vertexLightingData;
    sdword vertexIndex;
    sdword nextIndex;

    binding = &bindings[index];
    vertexLightingData = memAlloc(sizeof(shSpecularData) * object->nVertices,
                                  "LodMeshBindingVertices",
                                  NonVolatile);
    for (vertexIndex = 0; vertexIndex < object->nVertices; vertexIndex++)
    {
        memset(&vertexLightingData[vertexIndex], 0, sizeof(shSpecularData));
        vertexLightingData[vertexIndex].data[17] = 1;
        *((real32 *)&vertexLightingData[vertexIndex].data[20]) = -1.0f;
        if (bitTest(object->pVertexList[vertexIndex].iVertexNormal, BIT2))
        {
            *((real32 *)&vertexLightingData[vertexIndex].data[8]) = 1.0f;
        }
    }

    binding->objectTag = object->pName;
    binding->nVertices = object->nVertices;
    binding->vertexLightingData = vertexLightingData;

    nextIndex = index + 1;
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {
        nextIndex = lodBuildMeshBindingsRecursive(bindings, nextIndex, daughter);
    }
    return nextIndex;
}

static sdword lodFreeMeshBindingsRecursive(lodmeshbinding *bindings, sdword index, polygonobject *object)
{
    polygonobject *daughter;
    sdword nextIndex;

    nextIndex = index + 1;
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {
        nextIndex = lodFreeMeshBindingsRecursive(bindings, nextIndex, daughter);
    }

    if (bindings[index].vertexLightingData != NULL)
    {
        memFree(bindings[index].vertexLightingData);
        bindings[index].vertexLightingData = NULL;
    }
    return nextIndex;
}

/*-----------------------------------------------------------------------------
    Name        : lodCurrentLevelGet
    Description : Return the live retail LOD entry for the object's currentLOD.
-----------------------------------------------------------------------------*/
lod *lodCurrentLevelGet(SpaceObj *obj)
{
    lodinfo *lodInfo;

    lodInfo = shipGetLodInfo(obj);
    return &lodInfo->level[obj->currentLOD];
}

/*-----------------------------------------------------------------------------
    Name        : lodLevelGetSimple
    Description : Retail selCircleRadius-only LOD selection path.
-----------------------------------------------------------------------------*/
lod *lodLevelGetSimple(void *spaceObj, vector *camera, vector *ship)
{
    real32 lodMetric;
    lodinfo *lodInfo;
    SpaceObj *obj;

    obj = (SpaceObj *)spaceObj;
    lodMetric = ((SpaceObjRotImpTarg *)obj)->collInfo.selCircleRadius;
    lodInfo = shipGetLodInfo(obj);

    vecSub(obj->cameraDistanceVector, *camera, *ship);
    obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

    if (lodMetric <= lodDetailBucketMax)
    {
        if (lodMetric <= lodDetailBucket1)
        {
            if (lodMetric <= lodDetailBucket2)
            {
                if (lodMetric <= lodDetailBucket3)
                {
                    if (lodMetric <= lodDetailBucket4)
                    {
                        obj->currentLOD = 5;
                    }
                    else
                    {
                        obj->currentLOD = 4;
                    }
                }
                else
                {
                    obj->currentLOD = 3;
                }
            }
            else
            {
                obj->currentLOD = 2;
            }
        }
        else
        {
            obj->currentLOD = 1;
        }
    }
    else
    {
        obj->currentLOD = 0;
    }

    if (obj->currentLOD >= lodInfo->nLevels)
    {
        obj->currentLOD = (ubyte)(lodInfo->nLevels - 1);
    }
    return &lodInfo->level[obj->currentLOD];
}

/*-----------------------------------------------------------------------------
    Name        : lodTimingSampleStart
    Description : Record the retail start timestamp for the next timing sample.
-----------------------------------------------------------------------------*/
bool32 lodTimingSampleStart(lod *lodLevel)
{
    bool32 result = FALSE;
    LARGE_INTEGER performanceCount;

    if (lodLevel->timingSampleIndex < 4)
    {
        result = catQueryPerformanceCounter(&performanceCount);
        lodLevel->timingSample[lodLevel->timingSampleIndex] = performanceCount.LowPart;
    }
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : lodTimingSampleStop
    Description : Finish one retail timing sample and refresh the cached minimum.
-----------------------------------------------------------------------------*/
void lodTimingSampleStop(lod *lodLevel)
{
    sdword index;
    LARGE_INTEGER performanceCount;

    if (lodLevel->timingSampleIndex < 4)
    {
        catQueryPerformanceCounter(&performanceCount);
        lodLevel->timingSample[lodLevel->timingSampleIndex] =
            performanceCount.LowPart - lodLevel->timingSample[lodLevel->timingSampleIndex];
        lodLevel->stabilizedMetric = lodLevel->timingSample[0];
        for (index = 1; index < 4; index++)
        {
            if (lodLevel->timingSample[index] < lodLevel->stabilizedMetric)
            {
                lodLevel->stabilizedMetric = lodLevel->timingSample[index];
            }
        }
        lodLevel->timingSampleIndex++;
    }
}
