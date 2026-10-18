/*=============================================================================
    Name    : Mex.c
    Purpose : Mesh extensions

    Created 7/18/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <math.h>
#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "memory.h"
#include "debug.h"
#include "file.h"
#include "mex.h"
#include "color.h"

static sdword mexFindMeshBindingIndexByName(mhbinding *list, meshdata *mesh, char *name);

bool32 mexVerify(void *mex)
{
    ubyte *curmexptr = (ubyte *)mex;

    if (_stricmp(curmexptr,"mannngo") == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void *mexLoad(char *filename)
{
    void *address;
    fileLoadAlloc(filename,&address,NonVolatile);

    if (!mexVerify(address))
    {
        dbgFatal(DBG_Loc,"Invalid MEX file");
    }
    return address;
}

void mexFree(void *mex)
{
    memFree(mex);
}

void *mexGetChunk(void *mex,char *type,char *name)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (_stricmp(curmexchunk->type,type) == 0)
        {
            if (_stricmp(curmexchunk->name,name) == 0)
            {
                return (void *)curmexchunk;
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}

MEXDockingChunk *mexGetDockChunk(void *mex,char *dockname)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (_stricmp(curmexchunk->type,"Dok") == 0)
        {
            if (_stricmp(curmexchunk->name,"Dock") == 0)
            {
                if (_stricmp(((MEXDockingChunk *)curmexchunk)->dockName,dockname) == 0)
                {
                    return (MEXDockingChunk *)curmexchunk;
                }
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}

MEXSalvageChunk *mexGetSalChunk(void *mex,char *salvagename)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if ((_stricmp(curmexchunk->type,"Dok") == 0 &&
             _stricmp(curmexchunk->name,"Dock") == 0) ||
            (_stricmp(curmexchunk->type,"Sal") == 0 &&
             _stricmp(curmexchunk->name,"SALVAGE") == 0))
        {
            if (_stricmp(((MEXSalvageChunk *)curmexchunk)->Name,salvagename) == 0)
            {
                return (MEXSalvageChunk *)curmexchunk;
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}


MEXNAVLightChunk *mexGetNAVLightChunk(void *mex,char *navLightName)
{
    MEXFileHeader *header = (MEXFileHeader *)mex;
    MEXChunk *curmexchunk = (MEXChunk *) ( ((ubyte *)mex) + sizeof(MEXFileHeader) );
    uword numChunks = header->nChunks;
    uword i;

    for (i=0;i<numChunks;i++)
    {
        if (_stricmp(curmexchunk->type,"Nav") == 0)
        {
            if (_stricmp(curmexchunk->name,"Nav") == 0)
            {
                if (_stricmp(((MEXNAVLightChunk *)curmexchunk)->NAVLightName,navLightName) == 0)
                {
                    return (MEXNAVLightChunk *)curmexchunk;
                }
            }
        }

        curmexchunk = (MEXChunk *)(((ubyte *)curmexchunk) + sizeof(MEXChunk) + curmexchunk->chunkSize);
    }

    return NULL;
}

ResNozzleStatic *mexGetResNozzleStatic(void *mex)
{
    MEXGunChunk *gunchunk;
    ResNozzleStatic *resnozzle = NULL;

    if (mex == NULL)
    {
        return NULL;
    }

    gunchunk = mexGetChunk(mex,"Gun","Gun0");       // just use gun chunk for now
    if (gunchunk != NULL)
    {
        resnozzle = memAlloc(sizeof(ResNozzleStatic),"resnozzle",NonVolatile);

        resnozzle->position = gunchunk->position;

        mexVecToVec(&resnozzle->nozzlenormal,&gunchunk->normal);

        return resnozzle;
    }

    return NULL;
}

RepairNozzleStatic *mexGetRepairNozzleStatic(void *mex)
{
    MEXGunChunk *gunchunk;
    RepairNozzleStatic *resnozzle = NULL;

    if (mex == NULL)
    {
        return NULL;
    }

    gunchunk = mexGetChunk(mex,"Gun","Rep0");       // just use gun chunk for now
    if (gunchunk != NULL)
    {
        resnozzle = memAlloc(sizeof(RepairNozzleStatic),"repairnozzle",NonVolatile);

        resnozzle->position = gunchunk->position;

        return resnozzle;
    }

    return NULL;
}

TractorBeamStatic *mexGetTractorBeamStatic(void *mex)
{
    MEXGunChunk *gunchunk;
    TractorBeamStatic *tractoremitter = NULL;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: ResNozzle info not retrieved.  Mesh extensions not loaded");
        return NULL;
    }

    gunchunk = mexGetChunk(mex,"Gun","Beam0");       // just use gun chunk for now
    if (gunchunk != NULL)
    {
        tractoremitter = memAlloc(sizeof(TractorBeamStatic),"tractoremitter",NonVolatile);

        tractoremitter->position = gunchunk->position;

        mexVecToVec(&tractoremitter->nozzlenormal,&gunchunk->normal);

        return tractoremitter;
    }

    return NULL;
}

void mexGetGunStaticInfo(GunStaticInfo *gunstaticinfo,void *mex)
{
    sdword i;
    MEXGunChunk *gunchunk;
    char gunname[8];
    sdword numGuns = gunstaticinfo->numGuns;
    GunStatic *gunstatic;

    if (mex == NULL)
    {
//        dbgFatal(DBG_Loc,"Could not get gun info because mesh extensions were not loaded");
        dbgMessage("\nWarning: Gun info not retrieved.  Mesh extensions not loaded");
        return;
    }

    for (i=0;i<numGuns;i++)
    {
        sprintf(gunname,"Gun%d",i);
        gunchunk = mexGetChunk(mex,"Gun",gunname);
        if (gunchunk != NULL)
        {
            gunstatic = &gunstaticinfo->gunstatics[i];

            gunstatic->position = gunchunk->position;

            mexVecToVec(&gunstatic->gunnormal,&gunchunk->normal);
            if ((gunstatic->gunnormal.x == 0.0f) &&
                (gunstatic->gunnormal.y == 0.0f) &&
                (gunstatic->gunnormal.z == 0.0f))
            {
                gunstatic->gunnormal.x = 1.0f;
            }
            else
            {
                vecNormalize(&gunstatic->gunnormal);
            }

            if ((gunstatic->cosminAngleFromNorm == 0.0f) && (gunstatic->cosmaxAngleFromNorm == 0.0f))
            {
                gunstatic->cosminAngleFromNorm = (real32)cos(DEG_TO_RAD(gunchunk->edgeAngle));
                gunstatic->cosmaxAngleFromNorm = (real32)cos(DEG_TO_RAD(gunchunk->coneAngle + gunchunk->edgeAngle));
            }
            else
            {
                dbgMessagef("\nWarning: Could not find gun in mex at %x",mex);
            }
        }
    }
}

void mexGetDockStaticInfo(DockStaticInfo *dockstaticinfo,void *mex)
{
    sdword i;
    MEXSalvageChunk *dockchunk;
    sdword numDockPoints = dockstaticinfo->numDockPoints;
    DockStaticPoint *dockstaticpoint;

    if (mex == NULL)
    {
        dbgMessage("\nWarning: Dock info not retrieved.  Mesh extensions not loaded");
        return;
    }

    for (i=0;i<numDockPoints;i++)
    {
        dockstaticpoint = &dockstaticinfo->dockstaticpoints[i];
        dockstaticpoint->dockindex = i;
        // Retail (0x4b75a0) looks up dock points via mexGetSalChunk, which matches
        // both "Dok"/"Dock" and "Sal"/"SALVAGE" mesh chunks by name, not mexGetDockChunk
        // (which only matches "Dok"/"Dock"). Cataclysm's DPT_Salvage dock points (e.g.
        // the crystal's Point1/Point2) are authored as "Sal" chunks in the mesh, so a
        // Dok-only search always fails to find them and leaves conenormal at (0,0,0).
        dockchunk = mexGetSalChunk(mex,dockstaticpoint->name);
        if (dockchunk != NULL)
        {
            dockstaticpoint->position = dockchunk->position;

            mexVecToVec(&dockstaticpoint->conenormal,&dockchunk->normal);

            if ((dockstaticpoint->conenormal.x == 0.0f) &&
                (dockstaticpoint->conenormal.y == 0.0f) &&
                (dockstaticpoint->conenormal.z == 0.0f))
            {
                dockstaticpoint->conenormal.x = 1.0f;
            }
            else
            {
                vecNormalize(&dockstaticpoint->conenormal);
            }

            dockstaticpoint->coneangle = (real32)cos(DEG_TO_RAD(dockchunk->coneAngle + dockchunk->edgeAngle));
        }
        else
        {
            // Retail (0x4b7634-0x4b7664) fallback on lookup failure: zero position,
            // conenormal defaults to (1,0,0), and coneangle defaults to cos(90 deg).
            vecZeroVector(dockstaticpoint->position);
            dockstaticpoint->conenormal.x = 1.0f;
            dockstaticpoint->conenormal.y = 0.0f;
            dockstaticpoint->conenormal.z = 0.0f;
            dockstaticpoint->coneangle = (real32)cos(DEG_TO_RAD(90.0f));
        }
    }
}

void mexGetSalvageStaticInfo(SalvageStaticInfo *salvagestaticinfo,void *mex)
{
    sdword i;
    MEXSalvageChunk *salchunk;
    sdword numSalvagePoints = salvagestaticinfo->numSalvagePoints;
    SalvageStaticPoint *salvagestaticpoint;

    if (mex == NULL)
    {
        return;
    }

    for (i=0;i<numSalvagePoints;i++)
    {
        salvagestaticpoint = &salvagestaticinfo->salvageStaticPoints[i];
        salvagestaticpoint->number = i;
        salchunk = mexGetSalChunk(mex,salvagestaticpoint->name);
        if (salchunk != NULL)
        {
            salvagestaticpoint->position = salchunk->position;

            mexVecToVec(&salvagestaticpoint->conenormal,&salchunk->normal);

            if ((salvagestaticpoint->conenormal.x == 0.0f) &&
                (salvagestaticpoint->conenormal.y == 0.0f) &&
                (salvagestaticpoint->conenormal.z == 0.0f))
            {
                salvagestaticpoint->conenormal.x = 1.0f;
            }
            else
            {
                vecNormalize(&salvagestaticpoint->conenormal);
            }

            salvagestaticpoint->coneangle = (real32)cos(DEG_TO_RAD(salchunk->coneAngle + salchunk->edgeAngle));
        }
        else
        {
            vecZeroVector(salvagestaticpoint->position);
            salvagestaticpoint->conenormal.x = 1.0f;
            salvagestaticpoint->conenormal.y = 0.0f;
            salvagestaticpoint->conenormal.z = 0.0f;
            salvagestaticpoint->coneangle = (real32)cos(DEG_TO_RAD(90.0f));
        }
    }
}

void mexGetNAVLightStaticInfo(NAVLightStaticInfo *navlightstaticinfo,void *mex)
{
    sdword i;
    sdword level;
    sdword bindingIndex;
    lodinfo *lodInfo;
    void *rawMexData;
    MEXNAVLightChunk *navlightchunk;
    sdword numNAVLights = navlightstaticinfo->numNAVLights;
    NAVLightStatic *navlightstatic;
    MEXNAVLightContext *navLightContext = (MEXNAVLightContext *)mex;
    char *bindingName;

    dbgAssert(navLightContext != NULL);

    if (navLightContext->rawMexData == NULL)
    {
        return;
    }

    lodInfo = navLightContext->lodInfo;
    rawMexData = navLightContext->rawMexData;

    for (i=0;i<numNAVLights;i++)
    {
        navlightstatic = &navlightstaticinfo->navlightstatics[i];
        navlightchunk = mexGetNAVLightChunk(rawMexData, navlightstatic->name);
        if (navlightchunk == NULL)
        {
            continue;
        }

        navlightstatic->position = navlightchunk->position;
        navlightstatic->color = colRGB(navlightchunk->red,
                                       navlightchunk->green,
                                       navlightchunk->blue);

        if (lodInfo != NULL)
        {
            dbgAssert(lodInfo->nLevels <= sizeof(navlightstatic->lodBindingIndices));
            bindingName = (char *)navlightchunk + sizeof(MEXNAVLightChunk);
            for (level = 0; level < lodInfo->nLevels; level++)
            {
                navlightstatic->lodBindingIndices[level] = UBYTE_Max;
                if (*bindingName == '\0' ||
                    lodInfo->level[level].pData == NULL ||
                    lodInfo->level[level].hBindings == NULL)
                {
                    continue;
                }

                bindingIndex = mexFindMeshBindingIndexByName(
                    lodInfo->level[level].hBindings,
                    lodInfo->level[level].pData,
                    bindingName);
                if (bindingIndex >= 0)
                {
                    navlightstatic->lodBindingIndices[level] = (ubyte)bindingIndex;
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mexVecToVec
    Description : Convert a mex vector (rotation in x, y, z) to a real vector
    Inputs      : dest - where to store new vector
                  source - rotation of x, y, z
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mexVecToVec(vector *dest, vector *source)
{
    *dest = *source;  //!!! pass straight through !!!
    if (dest->x == 0.0f && dest->y == 0.0f && dest->z == 0.0f)
    {
        dest->x = 1.0f;
    }
    else
    {
        vecNormalize(dest);     //!!! normalize just for now
    }
}

void mexSetCollisionInfo(StaticCollInfo *staticCollInfo,void *mex,real32 forwardScale,real32 upScale,real32 rightScale, real32 xslide, real32 yslide, real32 zslide)
{
    MEXCollisionSphereChunk *spherechunk;
    MEXCollisionRectangleChunk *rectchunk;
    real32 maxScale;

    if (mex == NULL)
    {
        return;
    }

    //set uninitialized scale factors to 1.0f;
    if(forwardScale == 0.0f)
        forwardScale = 1.0f;
    if(upScale == 0.0f)
        upScale = 1.0f;
    if(rightScale == 0.0f)
        rightScale = 1.0f;

    maxScale = forwardScale;
    if (maxScale < upScale)
    {
        maxScale = upScale;
    }
    if (maxScale < rightScale)
    {
        maxScale = rightScale;
    }

    spherechunk = mexGetChunk(mex,"Col","Col0");
    if (spherechunk != NULL)
    {
        real32 sphereSize = maxScale * spherechunk->r;

        staticCollInfo->originalcollspheresize = sphereSize;
        staticCollInfo->collspheresize = sphereSize;
        staticCollInfo->approxcollspheresize = sphereSize;
        staticCollInfo->avoidcollspheresize = sphereSize + 3.0f;
        staticCollInfo->collsphereoffset = spherechunk->offset;
        staticCollInfo->collsphereoffset.x += xslide;
        staticCollInfo->collsphereoffset.y += yslide;
        staticCollInfo->collsphereoffset.z += zslide;
        staticCollInfo->collspheresizeSqr = staticCollInfo->collspheresize * staticCollInfo->collspheresize;
    }

    rectchunk = mexGetChunk(mex,"Rct","Rct0");
    if (rectchunk != NULL)
    {
        staticCollInfo->collrectoffset.x = rectchunk->ox+xslide;
        staticCollInfo->collrectoffset.y = rectchunk->oy+yslide;
        staticCollInfo->collrectoffset.z = rectchunk->oz+zslide;

        staticCollInfo->uplength = rectchunk->dx;
        staticCollInfo->rightlength = rectchunk->dy;
        staticCollInfo->forwardlength = rectchunk->dz;

        if (upScale != 1.0f)
        {
            real32 olduplength = staticCollInfo->uplength;
            staticCollInfo->uplength *= upScale;
            staticCollInfo->collrectoffset.x -= (staticCollInfo->uplength - olduplength) * 0.5f;
        }

        if (rightScale != 1.0f)
        {
            real32 oldrightlength = staticCollInfo->rightlength;
            staticCollInfo->rightlength *= rightScale;
            staticCollInfo->collrectoffset.y -= (staticCollInfo->rightlength - oldrightlength) * 0.5f;
        }

        if (forwardScale != 1.0f)
        {
            real32 oldforwardlength = staticCollInfo->forwardlength;
            staticCollInfo->forwardlength *= forwardScale;
            staticCollInfo->collrectoffset.z -= (staticCollInfo->forwardlength - oldforwardlength) * 0.5f;
        }
        // scale the sphere up to guarantee it surrounds the rectangle if necessary
        {
            real32 up = staticCollInfo->uplength * 0.5f;
            real32 right = staticCollInfo->rightlength * 0.5f;
            real32 forward = staticCollInfo->forwardlength * 0.5f;

            real32 minr = (real32)(sqrt(up*up+right*right+forward*forward) * 1.1f);

            if (minr > staticCollInfo->collspheresize)
            {
                staticCollInfo->originalcollspheresize = minr;
                staticCollInfo->collspheresize = minr;
                staticCollInfo->approxcollspheresize = minr;
                staticCollInfo->avoidcollspheresize = minr;
                staticCollInfo->avoidcollspherepad = minr;
                staticCollInfo->collspheresizeSqr = staticCollInfo->collspheresize * staticCollInfo->collspheresize;
            }
        }

        staticCollInfo->diagonallength = (real32)sqrt(staticCollInfo->uplength*staticCollInfo->uplength +
                                                      staticCollInfo->forwardlength*staticCollInfo->forwardlength +
                                                      staticCollInfo->rightlength*staticCollInfo->rightlength);
    }
    else
    {
        // approximate rectangle using collsphere info
        real32 temp = staticCollInfo->collspheresize * 0.5f;

        staticCollInfo->collrectoffset.x = staticCollInfo->collsphereoffset.x - temp;
        staticCollInfo->collrectoffset.y = staticCollInfo->collsphereoffset.y - temp;
        staticCollInfo->collrectoffset.z = staticCollInfo->collsphereoffset.z - temp;

        staticCollInfo->uplength = staticCollInfo->collspheresize*upScale;
        staticCollInfo->rightlength = staticCollInfo->collspheresize*rightScale;
        staticCollInfo->forwardlength = staticCollInfo->collspheresize*forwardScale;

        staticCollInfo->diagonallength = (real32)sqrt(staticCollInfo->uplength*staticCollInfo->uplength +
                                                      staticCollInfo->forwardlength*staticCollInfo->forwardlength +
                                                      staticCollInfo->rightlength*staticCollInfo->rightlength);
    }

    staticCollInfo->collRectFarHalf[0] = (staticCollInfo->uplength + staticCollInfo->collrectoffset.x) * 0.5f;
    staticCollInfo->collRectFarHalf[1] = (staticCollInfo->collrectoffset.y + staticCollInfo->rightlength) * 0.5f;
    staticCollInfo->collRectFarHalf[2] = (staticCollInfo->collrectoffset.z + staticCollInfo->forwardlength) * 0.5f;
}

static sdword mexFindMeshBindingIndexByName(mhbinding *list, meshdata *mesh, char *name)
{
    mhbinding *binding;

    binding = meshBindingFindByName(NULL, list, mesh, name);
    if (binding == NULL)
    {
        return -1;
    }

    return binding - list;
}

void mexSetEngineNozzle(ShipStaticInfo *shipstaticinfo, void* mex, sdword index)
{
    MEXEngineChunk* enginechunk;
    lodinfo *lodInfo;
    char engine[5];
    char *thrusterName;
    sdword i;
    sdword bindingIndex;

    strcpy(engine, "Eng0");
    engine[3] = (char)('0' + index);

    lodInfo = shipstaticinfo->staticheader.LOD;
    if (lodInfo != NULL)
    {
        for (i = 0; i < lodInfo->nLevels; i++)
        {
            shipstaticinfo->unk6[i * MAX_NUM_TRAILS + index] = UBYTE_Max;
        }
    }

    if (mex == NULL)
    {
        return;
    }

    enginechunk = mexGetChunk(mex,"Eng",engine);
    if (enginechunk != NULL)
    {
        shipstaticinfo->engineNozzleOffset[index] = enginechunk->position;

        if (lodInfo == NULL)
        {
            return;
        }

        thrusterName = (char *)enginechunk + sizeof(MEXEngineChunk);
        for (i = 0; i < lodInfo->nLevels; i++)
        {
            if (*thrusterName == '\0' ||
                lodInfo->level[i].pData == NULL ||
                lodInfo->level[i].hBindings == NULL)
            {
                continue;
            }

            bindingIndex = mexFindMeshBindingIndexByName(
                lodInfo->level[i].hBindings,
                lodInfo->level[i].pData,
                thrusterName);
            if (bindingIndex >= 0 && bindingIndex < UBYTE_Max)
            {
                shipstaticinfo->unk6[i * MAX_NUM_TRAILS + index] = (ubyte)bindingIndex;
            }
        }
    }
}



#if 0  // Cat2: mexSetCollisionInfo live code differs from binary
/*=== mexSetCollisionInfo (0x4B7880) ===*/
void __cdecl mexSetCollisionInfo(
StaticCollInfo *staticCollInfo,
void *mex,
float forwardScale,
float upScale,
float rightScale,
float xslide,
float yslide,
float zslide)
{
_WORD *v8; // ecx
double v9; // st7
double v10; // st6
float v11; // edx
unsigned __int16 v12; // bx
int v13; // ebp
char *v14; // edi
double v15; // st7
vector *v16; // edi
unsigned __int16 v17; // bp
int v18; // ebx
char *v19; // edi
double v20; // st7
double v21; // st7
double v22; // st6
double v23; // st5
double v24; // st4
double v25; // st3
bool v26; // c3
double diagonallength; // st7
double v28; // st6
double v29; // st7
double v30; // st6
double collBoxScaleForward; // st7
double v32; // st6
double v33; // st6
double v34; // st5
double v35; // st3
long double v36; // st4
float v37; // [esp+0h] [ebp-4h]

v8 = mex;
if ( !mex )
return;
if ( forwardScale == ZERO )
forwardScale = 1.0;
if ( upScale == ZERO )
upScale = 1.0;
if ( rightScale == ZERO )
rightScale = 1.0;
if ( forwardScale <= (double)upScale )
v9 = upScale;
else
v9 = forwardScale;
if ( upScale <= (double)rightScale )
v10 = rightScale;
else
v10 = upScale;
if ( v9 <= v10 )
{
if ( upScale > (double)rightScale )
{
v37 = upScale;
goto LABEL_22;
}
v11 = rightScale;
LABEL_21:
v37 = v11;
goto LABEL_22;
}
if ( forwardScale <= (double)upScale )
{
v11 = upScale;
goto LABEL_21;
}
v37 = forwardScale;
LABEL_22:
v12 = *((_WORD *)mex + 5);
v13 = 0;
v14 = (char *)mex + 12;
if ( v12 )
{
while ( _strcmpi(v14, off_886004) || _strcmpi(v14 + 4, aCol0) )
{
++v13;
v14 += *((_DWORD *)v14 + 3) + 16;
if ( (unsigned __int16)v13 >= v12 )
goto LABEL_29;
}
if ( v14 )
{
v15 = v37 * *((float *)v14 + 8);
v16 = (vector *)(v14 + 20);
staticCollInfo->originalcollspheresize = v15;
staticCollInfo->collspheresize = v37 * v16[1].x;
staticCollInfo->approxcollspheresize = v37 * v16[1].x;
staticCollInfo->avoidcollspheresize = v37 * v16[1].x + flt_72A534;
staticCollInfo->collsphereoffset = *v16;
staticCollInfo->collsphereoffset.x = xslide + staticCollInfo->collsphereoffset.x;
staticCollInfo->collsphereoffset.y = yslide + staticCollInfo->collsphereoffset.y;
staticCollInfo->collsphereoffset.z = zslide + staticCollInfo->collsphereoffset.z;
staticCollInfo->collspheresizeSqr = staticCollInfo->collspheresize * staticCollInfo->collspheresize;
}
LABEL_29:
v8 = mex;
}
v17 = v8[5];
v18 = 0;
v19 = (char *)(v8 + 6);
if ( !v17 )
goto LABEL_34;
while ( _strcmpi(v19, off_885FF8) || _strcmpi(v19 + 4, aRct0) )
{
++v18;
v19 += *((_DWORD *)v19 + 3) + 16;
if ( (unsigned __int16)v18 >= v17 )
goto LABEL_34;
}
if ( v19 )
{
v26 = upScale == ONE;
staticCollInfo->collrectoffset = *(vector *)(v19 + 20);
staticCollInfo->diagonallength = *((real32 *)v19 + 8);
staticCollInfo->preciseSelection = *((_DWORD *)v19 + 9);
staticCollInfo->collBoxScaleForward = *((real32 *)v19 + 10);
if ( !v26 )
{
diagonallength = staticCollInfo->diagonallength;
v28 = upScale * diagonallength;
staticCollInfo->diagonallength = v28;
staticCollInfo->collrectoffset.x = staticCollInfo->collrectoffset.x - (v28 - diagonallength) * ZERO_POINT_FIVE;
}
if ( rightScale != ONE )
{
v29 = *(float *)&staticCollInfo->preciseSelection;
v30 = rightScale * v29;
*(float *)&staticCollInfo->preciseSelection = v30;
staticCollInfo->collrectoffset.y = staticCollInfo->collrectoffset.y - (v30 - v29) * ZERO_POINT_FIVE;
}
if ( forwardScale != ONE )
{
collBoxScaleForward = staticCollInfo->collBoxScaleForward;
v32 = forwardScale * collBoxScaleForward;
staticCollInfo->collBoxScaleForward = v32;
staticCollInfo->collrectoffset.z = staticCollInfo->collrectoffset.z
- (v32 - collBoxScaleForward) * ZERO_POINT_FIVE;
}
v33 = *(float *)&staticCollInfo->preciseSelection * ZERO_POINT_FIVE;
v34 = staticCollInfo->collBoxScaleForward * ZERO_POINT_FIVE;
v35 = staticCollInfo->diagonallength * ZERO_POINT_FIVE;
v36 = sqrt(v34 * v34 + v33 * v33 + v35 * v35) * dbl_72AF60;
if ( v36 > staticCollInfo->collspheresize )
{
staticCollInfo->originalcollspheresize = v36;
staticCollInfo->collspheresize = v36;
staticCollInfo->approxcollspheresize = v36;
staticCollInfo->avoidcollspheresize = v36;
staticCollInfo->avoidcollspherepad = v36;
staticCollInfo->collspheresizeSqr = v36 * v36;
}
v24 = staticCollInfo->collBoxScaleForward * staticCollInfo->collBoxScaleForward
+ *(float *)&staticCollInfo->preciseSelection * *(float *)&staticCollInfo->preciseSelection;
v25 = staticCollInfo->diagonallength * staticCollInfo->diagonallength;
}
else
{
LABEL_34:
v20 = staticCollInfo->collspheresize * ZERO_POINT_FIVE;
staticCollInfo->collrectoffset.x = staticCollInfo->collsphereoffset.x - v20;
staticCollInfo->collrectoffset.y = staticCollInfo->collsphereoffset.y - v20;
staticCollInfo->collrectoffset.z = staticCollInfo->collsphereoffset.z - v20;
v21 = upScale * staticCollInfo->collspheresize;
staticCollInfo->diagonallength = v21;
v22 = rightScale * staticCollInfo->collspheresize;
*(float *)&staticCollInfo->preciseSelection = v22;
v23 = forwardScale * staticCollInfo->collspheresize;
staticCollInfo->collBoxScaleForward = v23;
v24 = v23 * v23 + v22 * v22;
v25 = v21 * v21;
}
staticCollInfo->collBoxScaleUp = sqrt(v24 + v25);
staticCollInfo->uplength = (staticCollInfo->diagonallength + staticCollInfo->collrectoffset.x) * ZERO_POINT_FIVE;
staticCollInfo->rightlength = (staticCollInfo->collrectoffset.y + *(float *)&staticCollInfo->preciseSelection)
* ZERO_POINT_FIVE;
staticCollInfo->forwardlength = (staticCollInfo->collrectoffset.z + staticCollInfo->collBoxScaleForward)
* ZERO_POINT_FIVE;
}
#endif
