/*=============================================================================
    Name    : LOD.h
    Purpose : Defininitions for level-of-detail control

    Created 7/20/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___LOD_H
#define ___LOD_H

#include "Mesh.h"
#include "Vector.h"

/*=============================================================================
    Switches
=============================================================================*/
#ifdef HW_BUILD_FOR_DEBUGGING

#define LOD_ERROR_CHECKING          1           //general error checking
#define LOD_VERBOSE_LEVEL           2           //control specific output code
#define LOD_PRINT_DISTANCE          1           //print distances to selected ship
#define LOD_AUTO_SAVE               1           //enable auto-save of LOD files
#define LOD_SCALE_DEBUG             1           //debug of LOD tuning

#else

#define LOD_ERROR_CHECKING          0           //general error checking
#define LOD_VERBOSE_LEVEL           0           //control specific output code
#define LOD_PRINT_DISTANCE          0           //print distances to selected ship
#define LOD_AUTO_SAVE               0           //enable auto-save of LOD files
#define LOD_SCALE_DEBUG             0           //debug of LOD tuning

#endif

/*=============================================================================
    Definitions:
=============================================================================*/
//max number of Levels of detail
#define LOD_NumberLevels            6

//level-of-detail types
#define LT_Invalid                  0x0000
#define LT_Mesh                     0x0001      //regular mesh
#define LT_TinySprite               0x0002      //tiny sprite
#define LT_SubPixel                 0x0003      //a pixel or less
#define LT_Function                 0x0004      //a custom LOD function
#define LT_NULL                     0x0005      //draw nothing
#define LM_LODType                  0x0007      //mask for all types

#define LOD_ScaleFactor             0.6f

/*=============================================================================
    Type definitions:
=============================================================================*/
//structure for each level-of-detail
typedef struct
{
    udword flags;
    real32 mOn, bOn;                            //params for on xition
    real32 mOff, bOff;                          //params for off xition
    void *pData;                                //type-specific pointer
    sdword nMatrices;                           //numnber of matrices
    //these matrices will be in the same order as they will be rendered.
//    hmatrix **startMatrix;                      //pointers to the starting hierarchy matrices
    mhbinding *hBindings;                       //hierarchy bindings, same order as matrices
    ubyte timingSampleIndex;                    // retail timing sample cursor
    ubyte timingPad[3];
    udword timingSample[4];                     // retail timing history
    udword stabilizedMetric;                    // retail cached polygon/timing metric
}
lod;
//LOD structure found in the StaticInfo for all spaceobjs
typedef struct
{
    sdword nLevels;                             //number of levels of detail for this ship
    color pointColor;                           //color to draw when it's just a point
    udword useHomeworldLODSystem;               //Cataclysm-added: non-zero selects the classic HW1 bOn/bOff distance-based LOD system instead of the selCircleRadius bucket system
    lod level[1];                               //actual LOD info for <n> levels
}
lodinfo;
//LOD structure with a maximum number of levels-of-detail
typedef struct
{
    sdword nLevels;                             //number of levels of detail for this ship
    color pointColor;                           //color to draw when it's just a point
    udword useHomeworldLODSystem;               //Cataclysm-added: non-zero selects the classic HW1 bOn/bOff distance-based LOD system instead of the selCircleRadius bucket system
    lod level[LOD_NumberLevels];                //actual LOD info
}
lodmaxinfo;

typedef struct lodmeshbinding
{
    void *objectTag;                            // retail copies polygonobject->pName here
    sdword nVertices;                           // vertex count for this object
    void *vertexLightingData;                   // shSpecularData[nVertices]
}
lodmeshbinding;

typedef struct lodmeshbindings
{
    sdword nLevels;                             // number of LOD entries in level[]
    lodmeshbinding *level[1];                  // per-LOD object binding arrays
}
lodmeshbindings;

/*=============================================================================
    Data:
=============================================================================*/
#if LOD_PRINT_DISTANCE
extern sdword rndLOD;
extern sdword lodTuningMode;
extern sdword lodDrawingMode;
#endif

//scaling factor for the LOD's
extern real32 lodScaleFactor;

#if LOD_SCALE_DEBUG
extern real32 lodDebugScaleFactor;
#endif

/*=============================================================================
    Macros:
=============================================================================*/
#define lodTableSize(n)         (sizeof(lodinfo) + (n - 1) * sizeof(lod))

static_assert(sizeof(lod) == 56);
static_assert(offsetof(lod, pData) == 20);
static_assert(offsetof(lod, timingSampleIndex) == 32);
static_assert(offsetof(lod, stabilizedMetric) == 52);
static_assert(offsetof(lodinfo, level) == 12);

/*=============================================================================
    Functions:
=============================================================================*/
lodinfo *lodTableReadScript(char *directory, char *fileName);

lod *lodLevelGet(void *spaceObj, vector *camera, vector *ship);
lod *lodLevelGetSimple(void *spaceObj, vector *camera, vector *ship);
lod *lodPanicLevelGet(void *spaceObj, vector *camera, vector *ship);
void lodAllMeshesRecolorize(lodinfo *LOD);
sdword lodHierarchySizeCompute(lodinfo *LOD);
lodmeshbindings *lodMeshBindingsAllocate(void *obj);

void lodFree(lodinfo *LOD);

#if LOD_AUTO_SAVE
sdword lodAutoSave(lodinfo *LOD, char *fileName);
#endif

#endif
