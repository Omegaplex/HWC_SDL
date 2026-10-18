/*=============================================================================
    Name    : render.c
    Purpose : Render module initialization and render a specific scene.

    Created 6/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef gshaw
//#define DEBUG_COLLISIONS
#endif

#if 0  // Cat3: rinAllocZeroed
/*=== rinAllocZeroed (0x57DCC0) ===*/
void *rinAllocZeroed(size_t size)
{
    void *memory;

    memory = malloc_34(size);
    memset(memory, 0, size);
    return memory;
}
#endif

#define RND_DOCKLIGHT_TWEAK 0
#define VISIBLE_POLYS 0

#ifdef khentschel
#ifndef HW_Release
#define VERBOSE_SHIP_STATS  1
#else
#define VERBOSE_SHIP_STATS  0
#endif

#define DISPLAY_LOD_SCALE   1
#else
#define VERBOSE_SHIP_STATS  0
#define DISPLAY_LOD_SCALE   1
#endif

#ifdef ddunlop
#ifndef HW_Release
#define FONT_CHECKSPECIAL   0       // special define for testing extended characters
#else
#define FONT_CHECKSPECIAL   0       // turn off this function
#endif
#endif

#define SHOW_TRAIL_STATS    0
#define RND_WILL_PANIC      0
#define USE_RND_HINT        0
#define WILL_TWO_PASS       0

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "glinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "debug.h"
#include "switches.h"
#include "netcheck.h"
#include "spaceobj.h"
#include "Ship.h"
#include "ShipDefs.h"
#include "mouse.h"
#include "prim2d.h"
#include "region.h"
#include "task.h"
#include "key.h"
#include "universe.h"
#include "nis.h"
#include "camera.h"
#include "cameracommand.h"
#include "prim3d.h"
#include "font.h"
#include "select.h"
#include "RepairDroid.h"
#include "MissionScriptSupport.h"
#include "texreg.h"
#include "utility.h"
#include "globals.h"
#include "light.h"
#include "Randy.h"
#include "shader.h"
#include "tactical.h"
#include "mesh.h"
#include "render.h"
#include "blobs.h"
#include "collision.h"
#include "univupdate.h"
#include "soundevent.h"
#include "gun.h"
#include "dock.h"
#include "navlights.h"
#include "Dreadnought.h"
#include "memory.h"
#include "main.h"
#include "tweak.h"
#include "defensefighter.h"
#include "clouds.h"
#include "nebulae.h"
#include "fastmath.h"
#include "btg.h"
#include "demo.h"
#include "file.h"
#include "clipper.h"
#include "glcaps.h"
#include "teams.h"
#include "singleplayer.h"
#include "hs.h"
#include "screenshot.h"
#include "salcapcorvette.h"
#include "ACV.h"
#include "autolod.h"
#include "proximitysensor.h"
#include "objectives.h"
#include "options.h"
#include "subtitle.h"
#include "tutor.h"
#include "animatic.h"
#include "bink.h"
#include "StringsOnly.h"
#include "mainrgn.h"

#include "aiship.h"
#include "commandlayer.h"
#include "commandnetwork.h"
#include "beam.h"
#include "ProfileTimers.h"
#include "LagPrint.h"
#include "glcompat.h"
#include "tracking.h"
#include "launchmgr.h"
#include "devstats.h"
#include "statscript.h"

extern sdword enableRepairbots;

#define CATA_MEMGROWTH_SAVE_SIZE 156
#define CATA_DERELICT_TEXTURE_REF_OFFSET 20
#define CATA_MODULE_SLOT_SOURCE_DRAW_THRESHOLD 0.25f
#define CATA_DAMAGE_RENDER_STYLE_EXTERNAL_BUILD 1
#define CATA_DAMAGE_RENDER_STYLE_PREVIEW        2

static const sdword EMPLineSegmentsByLOD[] = { 320, 140, 80, 80 };
static const real32 EMPJitterByLOD[] = { 1.5f, 2.0f, 3.0f, 3.0f };
static const sdword EMPRenderStride = 5;
static const real32 EMPColorRed = 1.0f;
static const real32 EMPColorGreen = 1.0f;
static const real32 EMPColorBlue = 0.9f;
static const sdword CATA_TRANSPARENT_QUEUE_GROW = 16;

static sdword rndCountMeshPolygons(polygonobject *object);
static sdword rndCountTotalPolygons(lod *level);
static void rndRenderEMP(Ship *ship);
static void rndTransparentSort(sdword mode);

typedef struct
{
    bool8 active;
    ubyte style;
    bool8 unfinishedPass;
    real32 buildFraction;
} CataDamageRenderScratch;

typedef struct
{
    bool8 active;
    real32 morphProgress;
    lodinfo *buildTargetLodInfo;
    shipbindings *buildTargetBindings;
    sdword colorScheme;
    ShipRace buildTargetRace;
    real32 displaySphereRadius;
    bool32 targetStateActive;
} CataModuleSlotRenderScratch;

typedef struct
{
    bool8 active;
    real32 progress;
    lodinfo *replacementLodInfo;
    shipbindings *replacementBindings;
    ShipRace replacementRace;
    ubyte replacementColorScheme;
} CataReplacementRenderScratch;

typedef struct
{
    bool8 active;
    real32 progress;
    ShipStaticInfo *replacementStaticInfo;
} CataBeastOverlayRenderScratch;

typedef struct
{
    ubyte state;
    real32 curve;
    real32 progress;
} CataHyperspaceRenderScratch;

typedef struct
{
    Ship *ship;
    bool8 statusWindowActive;
    bool8 bigGunChargeActive;
    bool8 hyperspacePhaseActive;
    bool8 cloakStateActive;
    ubyte effectState[3];
    bool8 invulnerableStateActive;
    CataHyperspaceRenderScratch hyperspace;
    CataDamageRenderScratch damage;
    CataReplacementRenderScratch replacement;
    CataBeastOverlayRenderScratch beastOverlay;
    CataModuleSlotRenderScratch moduleSlot;
} CataRenderScratch;

static ubyte cataMemGrowthSaveState[CATA_MEMGROWTH_SAVE_SIZE];
static CataRenderScratch cataRenderScratch;
static SpaceObj **cataTransparentQueue = NULL;
static sdword cataTransparentQueueMode = 0;
static sdword cataTransparentQueueCount = 0;
static sdword cataTransparentQueueCapacity = 0;
static sdword cataTransparentQueueIndex = 0;
trhandle mainDerelictTexture = 0;
extern sdword enableLensFlare;
extern udword gDevcaps2;
void *shipGetBeastWeaponInfo(Ship *ship);
real32 bigGunChargeDuration(sdword shiptype);
extern mempool memGrowthPool[];
extern sdword rndTextureModeTable[];

// Reinterpret float bits as signed integer (IDA LODWORD pattern)
#define FLOAT_AS_SDWORD(x) (*(sdword*)&(x))

static real32 cataFloatFromUdword(udword bits)
{
    union
    {
        udword u;
        real32 f;
    } convert;

    convert.u = bits;
    return convert.f;
}

static BOOL rndCheckNaggarokEMP(SpaceObj *spaceobj)
{
    AIVar *naggarokEMP;
    sdword *entry;
    Ship *ship;

    if (spaceobj == NULL || spaceobj->objtype != OBJ_ShipType)
    {
        return FALSE;
    }

    ship = (Ship *)spaceobj;
    naggarokEMP = aivarFindAnyFSM("G_NaggarokEMP");
    entry = aivarFindEntry(0, naggarokEMP);
    return entry != NULL && (real32)entry[2] > ship->disableStateElapsed;
}

void rndComputeHyperspacePhase(SpaceObj *spaceobj)
{
    Ship *ship;
    real32 phase;
    real32 curvePhase;

    cataRenderScratch.hyperspacePhaseActive = FALSE;
    cataRenderScratch.hyperspace.state = 0;
    cataRenderScratch.hyperspace.curve = 0.0f;
    cataRenderScratch.hyperspace.progress = 0.0f;

    if (spaceobj == NULL || spaceobj->objtype != OBJ_ShipType)
    {
        return;
    }

    ship = (Ship *)spaceobj;
    if (universe.totaltimeelapsed >= (double)ship->replacementStateEndTime)
    {
        return;
    }

    phase = (universe.totaltimeelapsed - ship->replacementStateStartTime) /
        (ship->replacementStateEndTime - ship->replacementStateStartTime);
    cataRenderScratch.hyperspacePhaseActive = TRUE;
    cataRenderScratch.hyperspace.progress = phase;

    if (phase >= 0.1f)
    {
        if (phase < 0.9f)
        {
            cataRenderScratch.hyperspace.state = 2;
            cataRenderScratch.hyperspace.curve = (phase - 0.1f) * 1.25f;
            return;
        }

        cataRenderScratch.hyperspace.state = 3;
        curvePhase = (phase - 0.9f) * 9.999998f;
    }
    else
    {
        cataRenderScratch.hyperspace.state = 1;
        curvePhase = phase * 10.0f;
    }

    cataRenderScratch.hyperspace.curve =
        1.0f - (real32)(cos((curvePhase + 1.0f) * 1.570796f) + 1.0);
}

int rndCheckCloakEffect(SpaceObj *spaceobj)
{
    cataRenderScratch.moduleSlot.active = FALSE;
    if (spaceobj != NULL && (spaceobj->flags2 & BIT3) != 0)
    {
        cataRenderScratch.moduleSlot.active =
            shipModuleSlotRuntimeConst((Ship *)spaceobj)->transitionArmed != 0;
    }
    return (int)spaceobj;
}

BOOL rndCheckFocusShipState(SpaceObj *spaceobj)
{
    BOOL result;

    result = (BOOL)spaceobj;
    cataRenderScratch.cloakStateActive = FALSE;
    if (spaceobj != NULL && spaceobj->objtype == OBJ_ShipType)
    {
        result = shipIsCloaking((Ship *)spaceobj);
        if (result)
        {
            cataRenderScratch.cloakStateActive = TRUE;
        }
    }
    return result;
}

BOOL rndIsNaggarokEMPActive(SpaceObj *spaceobj)
{
    Ship *ship;

    if (spaceobj == NULL || spaceobj->objtype != OBJ_ShipType)
    {
        return FALSE;
    }

    ship = (Ship *)spaceobj;
    return ship->shiptype == dRKO &&
           ship->shiprace != RACE_Beast &&
           rndCheckNaggarokEMP(spaceobj);
}

static bool32 cataLinkedSlipgateRenderQueueApplies(Ship *ship)
{
    Ship *linkedShip = ship;

    if (ship == NULL || ship->staticinfo->shipclass != CLASS_Component)
    {
        return FALSE;
    }

    if (ship->clampInfo != NULL && ship->clampInfo->host != NULL)
    {
        linkedShip = (Ship *)ship->clampInfo->host;
    }

    return linkedShip->dockvars.dockship != NULL &&
           !linkedShip->staticinfo->externalBuild &&
           dockIsSlipgateShip(linkedShip->dockvars.dockship);
}

static void cataTransparentQueueAppend(SpaceObj *spaceobj)
{
    if (cataTransparentQueueCount >= cataTransparentQueueCapacity)
    {
        cataTransparentQueue = memRealloc(
            cataTransparentQueue,
            sizeof(*cataTransparentQueue) * (cataTransparentQueueCapacity + CATA_TRANSPARENT_QUEUE_GROW),
            0,
            0);
        cataTransparentQueueCapacity += CATA_TRANSPARENT_QUEUE_GROW;
    }

    cataTransparentQueue[cataTransparentQueueCount++] = spaceobj;
}

static bool32 cataShipQueuesForTransparentPass(Ship *ship, bool32 *renderImmediately)
{
    *renderImmediately = TRUE;

    if (cataRenderScratch.bigGunChargeActive ||
        cataRenderScratch.damage.active ||
        (cataRenderScratch.moduleSlot.active && !cataRenderScratch.moduleSlot.targetStateActive) ||
        IsCrystal(ship) ||
        cataLinkedSlipgateRenderQueueApplies(ship))
    {
        *renderImmediately = FALSE;
        return TRUE;
    }

    if (cataRenderScratch.hyperspacePhaseActive)
    {
        return TRUE;
    }

    return FALSE;
}

static void rndTransparentSort(sdword mode)
{
    sdword outerIndex;
    sdword sortBaseIndex;

    cataTransparentQueueMode = mode;
    if (mode == 1)
    {
        cataTransparentQueueCount = 0;
        return;
    }
    if (mode != 2)
    {
        return;
    }

    cataTransparentQueueIndex = 0;
    for (outerIndex = 1, sortBaseIndex = 0;
         outerIndex - 1 < cataTransparentQueueCount;
         outerIndex++, sortBaseIndex++)
    {
        sdword innerIndex;

        for (innerIndex = outerIndex; innerIndex < cataTransparentQueueCount; innerIndex++)
        {
            SpaceObj *innerSpaceObj = cataTransparentQueue[innerIndex];
            SpaceObj *outerSpaceObj = cataTransparentQueue[sortBaseIndex];

            if (outerSpaceObj->cameraDistanceSquared < innerSpaceObj->cameraDistanceSquared)
            {
                SpaceObj *swap = cataTransparentQueue[sortBaseIndex];
                cataTransparentQueue[sortBaseIndex] = innerSpaceObj;
                cataTransparentQueue[innerIndex] = swap;
            }
        }
    }
}

// Transition overlay state (memGrowthPool[7..9] used as raw scratchpad storage in retail)
static sdword rndTexture2DEnabled = 0;
static sdword rndTransitionOverlayTexcoordInitPending = 1;
static sdword rndTransitionOverlayReconfigurePending = 0;
static real32 rndTransitionOverlayLastUpdateTime = 0.0f;
static real32 rndTransitionOverlayScrollS[2];
static real32 rndTransitionOverlayScrollT[2];
static real32 rndTransitionOverlayTexcoordScale[2];
static sdword rndTransitionOverlayTexcoordSBits[2];
static sdword rndTransitionOverlayTexcoordTBits[2];

// Transition overlay data constants
#define RND_OVERLAY_COLOR_SCALE  0.00390625f
#define RND_OVERLAY_ALPHA_SCALE  3.33e-8f
#define RND_OVERLAY_ALPHA_CONV   10000000.0
#define RND_OVERLAY_SCROLL_S0    (-0.0505000018f)
#define RND_OVERLAY_SCROLL_S1    0.0500000007f
#define RND_OVERLAY_SCROLL_T0    (-0.0149999997f)
#define RND_OVERLAY_SCROLL_T1    0.0049999999f
#define RND_OVERLAY_SCALE0       0.2000000030f
#define RND_OVERLAY_SCALE1       0.25f
#define RND_OVERLAY_INIT_T       1056964608     // 0x3F400000 = 0.75f initial T texcoord

#define RND_LENSFLARE_MAX_TEXTURES    16
#define RND_LENSFLARE_MAX_ELEMENTS    32
#define RND_LENSFLARE_MAX_VISIBLE     8
#define RND_LENSFLARE_DEFAULT_FADEOUT 1.2f
#define RND_LENSFLARE_GROW_FACTOR     0.4f
#define RND_LENSFLARE_DECAY_FACTOR    0.6f
#define RND_LENSFLARE_FADE_EPSILON    1.0e-9f
#define RND_LENSFLARE_EDGE_START      1.0f
#define RND_LENSFLARE_SEGMENT_SCALE   160.0f
#define RND_LENSFLARE_SEGMENT_MIN     6
#define RND_LENSFLARE_SEGMENT_MAX     32
#define RND_LENSFLARE_DIM_CENTER      0.25f
#define RND_LENSFLARE_COLOR_SCALE     (1.0f / 256.0f)
#define RND_LENSFLARE_TEXCOORD_SCALE  (1.0f / 256.0f)
#define RND_LENSFLARE_SCREEN_BIAS     0.325f

typedef struct
{
    real32 radius;
    real32 offset;
    sbyte textureIndex;
    ubyte red;
    ubyte green;
    ubyte blue;
} rndlensflareelement;

typedef struct
{
    real32 u0;
    real32 v0;
    real32 u1;
    real32 v1;
} rndlensflaretexcoords;

typedef struct
{
    vector position;
    real32 intensity;
    ubyte enabled;
    ubyte mode;
    ubyte persist;
    ubyte unused19;
    ubyte visible;
    ubyte unused21[3];
    real32 visibility;
    real32 edgeFade;
    real32 distance;
    real32 screenX;
    real32 screenY;
    Node node;
} rndlensflaresource;

static_assert(offsetof(rndlensflaresource, enabled) == 16);
static_assert(offsetof(rndlensflaresource, mode) == 17);
static_assert(offsetof(rndlensflaresource, persist) == 18);
static_assert(offsetof(rndlensflaresource, visible) == 20);
static_assert(offsetof(rndlensflaresource, visibility) == 24);
static_assert(offsetof(rndlensflaresource, edgeFade) == 28);
static_assert(offsetof(rndlensflaresource, distance) == 32);
static_assert(offsetof(rndlensflaresource, screenX) == 36);
static_assert(offsetof(rndlensflaresource, screenY) == 40);
static_assert(offsetof(rndlensflaresource, node) == 44);

static trhandle rndLensflareTextureHandle = TR_Invalid;
static real32 rndLensflareFadeoutFrameSize = RND_LENSFLARE_DEFAULT_FADEOUT;
static sdword rndLensflareTextureCount = 0;
static sdword rndLensflareElementCount = 0;
static sdword rndLensflareVisibleCount = 0;
static rndlensflaretexcoords rndLensflareTextureCoords[RND_LENSFLARE_MAX_TEXTURES];
static rndlensflareelement rndLensflareElements[RND_LENSFLARE_MAX_ELEMENTS];
static rndlensflaresource *rndLensflareVisibleSources[RND_LENSFLARE_MAX_VISIBLE];

static void rndLensflareTextureFileSet(char *directory, char *field, void *dataToFillIn);
static void rndLensflareElementSet(char *directory, char *field, void *dataToFillIn);
static void rndLensflareTextureSet(char *directory, char *field, void *dataToFillIn);

static scriptEntry rndLensflareScriptTable[] =
{
    { "FadeoutFrameSize", scriptSetReal32CB, &rndLensflareFadeoutFrameSize },
    { "TextureFile",      rndLensflareTextureFileSet, NULL },
    { "Texture",          rndLensflareTextureSet, NULL },
    { "Element",          rndLensflareElementSet, NULL },
    END_SCRIPT_ENTRY
};

char aRaceDerelictDp[] = "race_derelict\\dplasmacloud\\rl0\\lod0\\cloud";
static void cataClearExternalBuildPassState(void);

static void cataClearModuleSlotRenderState(void)
{
    cataRenderScratch.moduleSlot.active = FALSE;
    cataRenderScratch.moduleSlot.morphProgress = 0.0f;
    cataRenderScratch.moduleSlot.buildTargetLodInfo = NULL;
    cataRenderScratch.moduleSlot.buildTargetBindings = NULL;
    cataRenderScratch.moduleSlot.colorScheme = 0;
    cataRenderScratch.moduleSlot.buildTargetRace = 0;
    cataRenderScratch.moduleSlot.displaySphereRadius = 0.0f;
    cataRenderScratch.moduleSlot.targetStateActive = FALSE;
}

static void cataClearHyperspaceRenderState(void)
{
    cataRenderScratch.hyperspacePhaseActive = FALSE;
    cataRenderScratch.hyperspace.state = 0;
    cataRenderScratch.hyperspace.curve = 0.0f;
    cataRenderScratch.hyperspace.progress = 0.0f;
}

static void cataClearDamageRenderState(void)
{
    cataClearExternalBuildPassState();
    cataRenderScratch.damage.active = FALSE;
    cataRenderScratch.damage.style = 0;
    cataRenderScratch.damage.unfinishedPass = FALSE;
    cataRenderScratch.damage.buildFraction = 0.0f;
}

static void cataClearReplacementRenderState(void)
{
    cataRenderScratch.replacement.active = FALSE;
    cataRenderScratch.replacement.progress = 0.0f;
    cataRenderScratch.replacement.replacementLodInfo = NULL;
    cataRenderScratch.replacement.replacementBindings = NULL;
    cataRenderScratch.replacement.replacementRace = 0;
    cataRenderScratch.replacement.replacementColorScheme = 0;
}


static void cataClearBeastOverlayRenderState(void)
{
    cataRenderScratch.beastOverlay.active = FALSE;
    cataRenderScratch.beastOverlay.progress = 0.0f;
    cataRenderScratch.beastOverlay.replacementStaticInfo = NULL;
}

static void cataRenderBeastOverlay(Ship *ship)
{
    ShipStaticInfo *replacementStaticInfo = cataRenderScratch.beastOverlay.replacementStaticInfo;
    real32 progress = cataRenderScratch.beastOverlay.progress;
    sdword targetLOD;
    lod *targetLevel;
    meshdata *targetMesh;
    shipbindings *overlayBindings;
    ubyte savedEffectState0;

    if (ship == NULL ||
        !cataRenderScratch.beastOverlay.active ||
        replacementStaticInfo == NULL)
    {
        return;
    }

    targetLOD = ship->currentLOD;
    if (targetLOD >= replacementStaticInfo->staticheader.LOD->nLevels)
    {
        targetLOD = replacementStaticInfo->staticheader.LOD->nLevels - 1;
    }

    targetLevel = &replacementStaticInfo->staticheader.LOD->level[targetLOD];
    while (targetLOD > 0 && targetLevel->pData == NULL)
    {
        targetLOD--;
        targetLevel = &replacementStaticInfo->staticheader.LOD->level[targetLOD];
    }

    dbgAssert((targetLevel->flags & LM_LODType) == LT_Mesh);
    targetMesh = (meshdata *)targetLevel->pData;
    overlayBindings = (shipbindings *)ship->beastAttack.effect;

    meshSetFade(3, 1.0f - progress);
    glScalef(1.01f, 1.01f, 1.01f);
    savedEffectState0 = cataRenderScratch.effectState[0];
    cataRenderScratch.effectState[0] = 1;
    if (overlayBindings != NULL)
    {
        meshRenderShipHierarchyForShip(
            ship,
            overlayBindings,
            targetLOD,
            targetMesh,
            ship->beastAttack.effectColorScheme);
    }
    else
    {
        meshRender(targetMesh, ship->beastAttack.effectColorScheme);
    }

    cataRenderScratch.effectState[0] = savedEffectState0;
    meshSetFade(3, 0.0f);
}

static void cataSetShipRenderState(Ship *ship)
{
    real32 buildFraction;
    real32 bigGunActiveUntil;

    cataRenderScratch.ship = ship;
    cataRenderScratch.statusWindowActive = FALSE;
    cataRenderScratch.bigGunChargeActive = FALSE;
    cataClearDamageRenderState();
    cataClearHyperspaceRenderState();
    cataClearReplacementRenderState();
    cataClearBeastOverlayRenderState();
    cataClearModuleSlotRenderState();
    cataRenderScratch.cloakStateActive = FALSE;
    cataRenderScratch.effectState[0] = 0;
    cataRenderScratch.effectState[1] = 0;
    cataRenderScratch.effectState[2] = 0;
    cataRenderScratch.invulnerableStateActive = FALSE;
    if (ship != NULL)
    {
        if (ship->objtype == OBJ_ShipType &&
            (ship->flags2 & BIT3) == 0 &&
            ship->statusWindow.state != 0 &&
            universe.totaltimeelapsed >= (double)ship->statusWindow.phaseStartTime &&
            universe.totaltimeelapsed < (double)ship->statusWindow.phaseEndTime)
        {
            cataRenderScratch.statusWindowActive = TRUE;
        }
    }

    if (ship != NULL && ship->objtype == OBJ_ShipType && (ship->specialFlags2 & SPECIAL_2_DockFlag) != 0)
    {
        cataRenderScratch.damage.active = TRUE;
        cataRenderScratch.damage.style = CATA_DAMAGE_RENDER_STYLE_EXTERNAL_BUILD;
        cataRenderScratch.damage.unfinishedPass = FALSE;
        cataRenderScratch.damage.buildFraction = 1.0f - ship->buildTimerElapsed;
    }

    if (ship != NULL && ship->objtype == OBJ_ShipType)
    {
        bigGunActiveUntil = ship->statusWindow.activeUntil;
        if (universe.totaltimeelapsed < (double)bigGunActiveUntil ||
            (bigGunActiveUntil != 0.0f && nisIsRunning_))
        {
            cataRenderScratch.bigGunChargeActive = TRUE;
        }
    }

    rndComputeHyperspacePhase((SpaceObj *)ship);
    rndCheckCloakEffect((SpaceObj *)ship);
    rndCheckFocusShipState((SpaceObj *)ship);
    cataRenderScratch.effectState[2] = rndIsNaggarokEMPActive((SpaceObj *)ship) ? 1 : 0;

    if (ship != NULL &&
        ship->replacementStateActive != 0 &&
        ship->replacementStateEndTime > ship->replacementStateStartTime)
    {
        ShipStaticInfo *replacementStaticInfo = NULL;

        if (ship->statusWindow.activeUntil != 0.0f &&
            ship->beastAttack.sourcePlayer != NULL &&
            ship->staticinfo->beastAttackConvertTo != (ShipType)-1)
        {
            replacementStaticInfo =
                GetShipStaticInfo(ship->staticinfo->beastAttackConvertTo, ship->beastAttack.sourcePlayer->race);
        }
        else if (ship->specialTargetShip != NULL)
        {
            sdword replacementShipType = CrystalGetReplacementShipType(ship, ship->specialTargetShip);

            if (replacementShipType != -1)
            {
                replacementStaticInfo =
                    GetShipStaticInfo((ShipType)replacementShipType, ship->specialTargetShip->shiprace);
            }
        }

        if (replacementStaticInfo != NULL)
        {
            buildFraction =
                    (universe.totaltimeelapsed - ship->replacementStateStartTime) /
                    (ship->replacementStateEndTime - ship->replacementStateStartTime);
            if (buildFraction < 0.0f)
            {
                    buildFraction = 0.0f;
            }
            else if (buildFraction > 1.0f)
            {
                buildFraction = 1.0f;
            }

            cataRenderScratch.replacement.active = TRUE;
            cataRenderScratch.replacement.progress = buildFraction;
            cataRenderScratch.replacement.replacementLodInfo = replacementStaticInfo->staticheader.LOD;
            cataRenderScratch.replacement.replacementBindings = (shipbindings *)ship->replacementEffect;
            cataRenderScratch.replacement.replacementRace = replacementStaticInfo->shiprace;
            cataRenderScratch.replacement.replacementColorScheme = ship->replacementColorScheme;

        }
    }

    if (ship != NULL &&
        ship->statusWindow.activeUntil != 0.0f &&
        cataRenderScratch.bigGunChargeActive)
    {
        udword *beastWeaponInfo = (udword *)shipGetBeastWeaponInfo(ship);
        real32 chargeDuration = bigGunChargeDuration(ship->shiptype);

        if (beastWeaponInfo != NULL &&
            chargeDuration > 0.0f &&
            ship->beastAttack.sourcePlayer != NULL)
        {
            ShipType replacementShipType = (ShipType)beastWeaponInfo[2];
            ShipStaticInfo *replacementStaticInfo;
            real32 progress;

            if (replacementShipType != (ShipType)-1)
            {
                replacementStaticInfo =
                    GetShipStaticInfo(replacementShipType, ship->beastAttack.sourcePlayer->race);
                if (replacementStaticInfo != NULL)
                {
                    progress =
                        1.0f -
                        (ship->statusWindow.activeUntil - universe.totaltimeelapsed) / chargeDuration;
                    if (progress > 1.0f)
                    {
                        progress = 1.0f;
                    }

                    cataRenderScratch.beastOverlay.active = progress > 0.0f;
                    cataRenderScratch.beastOverlay.progress = progress;
                    cataRenderScratch.beastOverlay.replacementStaticInfo = replacementStaticInfo;
                }
            }
        }
    }

    if (ship != NULL && cataRenderScratch.moduleSlot.active)
    {
        const ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntimeConst(ship);

        cataRenderScratch.moduleSlot.morphProgress = moduleSlots->morphProgress;
        cataRenderScratch.moduleSlot.buildTargetLodInfo = moduleSlots->activeBuildTarget.buildTargetLodInfo;
        cataRenderScratch.moduleSlot.buildTargetBindings = moduleSlots->activeBuildTarget.buildTargetBindings;
        cataRenderScratch.moduleSlot.colorScheme = moduleSlots->activeBuildTarget.colorScheme;
        cataRenderScratch.moduleSlot.buildTargetRace = moduleSlots->activeBuildTarget.buildTargetRace;
        cataRenderScratch.moduleSlot.displaySphereRadius = moduleSlots->activeBuildTarget.displaySphereRadius;
        cataRenderScratch.moduleSlot.targetStateActive = (moduleSlots->state == ShipModuleSlotState_Active);
    }

}

void rndSetShipRenderScratchState(Ship *ship)
{
    cataSetShipRenderState(ship);
}

void rndSetPreviewBuildRenderState(real32 buildFraction)
{
    cataSetShipRenderState(NULL);
    if (buildFraction < 1.0f)
    {
        if (buildFraction < 0.0f)
        {
            buildFraction = 0.0f;
        }

        cataRenderScratch.damage.active = TRUE;
        cataRenderScratch.damage.style = CATA_DAMAGE_RENDER_STYLE_PREVIEW;
        cataRenderScratch.damage.unfinishedPass = FALSE;
        cataRenderScratch.damage.buildFraction = buildFraction;
    }
}

bool32 rndUseExternalBuildMaterial(void)
{
    return cataRenderScratch.damage.active
        && cataRenderScratch.damage.style == CATA_DAMAGE_RENDER_STYLE_EXTERNAL_BUILD
        && cataRenderScratch.damage.unfinishedPass;
}

static bool32 cataShipUsesExternalBuildClipPlane(Ship *ship)
{
    return ship != NULL && (ship->specialFlags2 & SPECIAL_2_DockFlag) != 0;
}

static bool32 cataShipUsesCrystalProcessingClipPlane(Ship *ship)
{
    return ship != NULL &&
        ship->crystalProcessingState.active == 1 &&
        ship->crystalProcessingState.visualsEnabled == 1 &&
        (ship->flags & SOF_Dead) == 0;
}

static bool32 cataDockLightApplies(Ship *dockship)
{
    if (dockship == NULL)
    {
        return FALSE;
    }

    switch (dockship->shiptype)
    {
        case sRecon:
        case sAcolyte:
        case sMimic:
        case sLeech:
        case sRammingFrigate:
        case sMultiBeamFrigate:
        case sHiveFrigate:
        case sSwarmer:
        case sDreadnought:
        case sDreadnoughtCannon:
        case sMothershipArmour:
        case sMothershipMicro:
        case bRecon:
        case bAcolyte:
            return TRUE;

        default:
            return FALSE;
    }
}

extern bool8 g_WireframeHack;
extern bool8 g_HiddenRemoval;

static void cataSetExternalBuildPassState(Ship *ship, bool32 unfinishedPass)
{
    GLdouble equation[4];
    StaticCollInfo *collInfo;
    real32 clipOffset;

    if (!cataShipUsesExternalBuildClipPlane(ship))
    {
        return;
    }
    collInfo = &ship->staticinfo->staticheader.staticCollInfo;
    clipOffset = collInfo->collrectoffset.z + collInfo->forwardlength * ship->buildTimerElapsed;
    cataRenderScratch.damage.unfinishedPass = unfinishedPass ? TRUE : FALSE;

    equation[0] = 0.0;
    equation[1] = 0.0;
    equation[2] = unfinishedPass ? 1.0 : -1.0;
    equation[3] = unfinishedPass ? -(GLdouble)clipOffset : (GLdouble)clipOffset;
    glClipPlane(GL_CLIP_PLANE0, equation);
    if (!unfinishedPass)
    {
        glEnable(GL_CLIP_PLANE0);
    }
    else if (!RGLtype)
    {
        g_WireframeHack = TRUE;
        g_HiddenRemoval = FALSE;
    }
}

static void cataClearExternalBuildPassState(void)
{
    glDisable(GL_CLIP_PLANE0);
}

static bool32 cataSetCrystalProcessingPassState(Ship *ship, bool32 *cullFaceEnabled)
{
    GLdouble equation[4];
    real32 planeEquation[4];

    if (!cataShipUsesCrystalProcessingClipPlane(ship))
    {
        return FALSE;
    }

    CrystalBuildProcessingSlicePlane(ship, planeEquation);
    equation[0] = planeEquation[0];
    equation[1] = planeEquation[1];
    equation[2] = planeEquation[2];
    equation[3] = planeEquation[3];
    *cullFaceEnabled = glIsEnabled(GL_CULL_FACE) != 0;
    glClipPlane(GL_CLIP_PLANE0, equation);
    glEnable(GL_CLIP_PLANE0);
    glDisable(GL_CULL_FACE);
    return TRUE;
}

static void cataClearCrystalProcessingPassState(bool32 cullFaceEnabled)
{
    glDisable(GL_CLIP_PLANE0);
    if (cullFaceEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
}

void *mainDerelictTextureSaveData(void)
{
    return cataMemGrowthSaveState;
}

trhandle mainLoadDerelictTexture(void)
{
    if (!mainDerelictTexture)
    {
        mainDerelictTexture = trTextureRegister(aRaceDerelictDp, NULL, cataMemGrowthSaveState + CATA_DERELICT_TEXTURE_REF_OFFSET);
    }
    return mainDerelictTexture;
}

bool8 rndFogOn = FALSE;

#if RND_VISUALIZATION
extern bool32 dockLines;
extern bool32 gunLines;
extern bool8 RENDER_BOXES;
extern bool8 RENDER_LIGHTLINES;
#endif

extern bool8 rndDamageFlashBiasActive;

extern color HorseRaceDropoutColor;

vector g_RndPosition;

bool32 rndTakeScreenshot = FALSE;

/*=============================================================================
    Private Types:
=============================================================================*/

/*=============================================================================
    Data:
=============================================================================*/
//this function pointer is what to call to render the main view
renderfunction rndMainViewRender = rndMainViewRenderFunction;

static sdword rndHint = 0;

HGLRC hGLRenderContext;
HDC hGLDeviceContext;
HWND hGLWindow;

udword meshRenders;

real32 rndAspectRatio;                                      //aspect ratio of rendering context

static sdword rndFillCounter = 0;
static color rndFillColour = 0;

//callback functions optionally called during rendering a mission sphere
rendercallback rndPreObjectCallback = NULL, rndPostObjectCallback = NULL;

sdword rndPerspectiveCorrect = FALSE;
sdword rndTextureEnabled = FALSE;
sdword rndTextureEnviron = RTE_Modulate;
sdword rndNormalization = FALSE;
sdword rndAdditiveBlending = FALSE;
sdword rndLightingEnabled = TRUE;
sdword rndScissorEnabled = FALSE;

int __cdecl rndSetDepthTesting(int enable)
{
    int wasEnabled;

    wasEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (enable)
    {
        glEnable(GL_DEPTH_TEST);
        if (glCapFeatureExists(0x9003))
        {
            glDepthMask(TRUE);
        }
    }
    else if (RGLtype && glCapFeatureExists(0x9003))
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(FALSE);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    return wasEnabled;
}

void primGLRectOutline2(rectangle *rect, sdword thickness, color c)
{
    float x0ndc, y0ndc, x1ndc, y1ndc;

    if (glcActive())
    {
        primRectOutline2(rect, thickness, c);
        return;
    }

    glColor3ub(colRed(c), colGreen(c), colBlue(c));
    glPushAttrib(GL_LINE_BIT);
    glLineWidth((float)thickness);
    glBegin(GL_LINE_LOOP);
    x0ndc = ((float)rect->x0 + 0.375f) / (float)MAIN_WindowWidth * 2.0f - 1.0f;
    y0ndc = 1.0f - ((float)rect->y0 + 0.375f) / (float)MAIN_WindowHeight * 2.0f;
    glVertex2f(x0ndc, y0ndc);
    x1ndc = ((float)rect->x1 + 0.375f) / (float)MAIN_WindowWidth * 2.0f - 1.0f;
    glVertex2f(x1ndc, y0ndc);
    y1ndc = 1.0f - ((float)rect->y1 + 0.375f) / (float)MAIN_WindowHeight * 2.0f;
    glVertex2f(x1ndc, y1ndc);
    glVertex2f(x0ndc, y1ndc);
    glEnd();
    glPopAttrib();
}

//data for billboard geometry
hmatrix rndCameraMatrix;
hmatrix rndProjectionMatrix;
hvector rndBillboardCentre;
#if RND_ERROR_CHECKING
sdword rndBillboardEnabled = FALSE;
#endif


typedef struct c4ub_v3f_s
{
    ubyte  c[4];
    real32 v[3];
} c4ub_v3f;
c4ub_v3f* stararray = NULL;
c4ub_v3f* bigstararray = NULL;


//asteroid0 stuff
typedef struct asteroid0data
{
    color c;
    vector* position;
} asteroid0data;

#define ASTEROID0DATA_SIZE 80
asteroid0data asteroid0Data[ASTEROID0DATA_SIZE];


//frame counter
udword rndFrameCount;

//frame rate data and definitions
#if RND_FRAME_RATE
#define RND_FrameRatePeriod         2.0f
#define RND_FrameRateSeconds        2.0f
#define RND_FrameRateMaxRate        10
#define RND_FrameRateStack          1024
#define RND_FrameRateKey            NUMPAD7
udword rndFrameRateStart;
udword rndFrameRate;
udword rndFrameCountMax, rndFrameCountMin;
udword rndPrintCount;
sdword rndDisplayFrameRate = 0;
taskhandle rndFrameRateTaskHandle;
extern regionhandle ghMainRegion;
#endif //RND_FRAME_RATE
#if defined(COLLISION_CHECK_STATS) || defined(PROFILE_TIMERS)
sdword rndDisplayCollStats = 0;
#define RND_CollStatsKey            NUMPADSLASH
#endif

//polygon statistics measurments
#if RND_POLY_STATS
#define RND_PolyStatsPeriod         2.0f
#define RND_PolyStatsSeconds        2.0f
#define RND_PolyStatsStack          1024
#define RND_PolyStatsKey            NUMPAD8
sdword rndDisplayPolyStats = 0;
sdword rndNumberPolys;
sdword rndNumberTextured;
sdword rndNumberSmoothed;
sdword rndNumberDots;
sdword rndNumberLines;
taskhandle rndPolyStatsTaskHandle;
char rndPolyStatsString[256];
color rndPolyStatsColor = colReddish;
sdword rndPolyStatFrameCounter = 0;
#endif //RND_POLY_STATS

#if RND_XYZ
#define RND_XYZKey                  NUMPAD9
bool32 rndXYZPrint = FALSE;
#endif

//scaling minimum cap crap
static sdword RND_CAPSCALECAP_STATS = 0;
char rndCapScaleCapStatsString[256];
#if RND_SCALECAP_TWEAK
#define scaleCapSlopeDelta  0.000001f                       //adjust the scaling cap
static char scaleCapString[256] = "";
#endif //RND_SCALECAP_TWEAK

#define RND_FOV_TWEAK 0

real32 btgFieldOfView = 50.0f;

#if RND_FOV_TWEAK
#define btgFieldOfViewDelta 1.0f
static char fovString[256] = "";
#endif

static sdword shipsAtLOD[5];
static sdword polysAtLOD[5];

//data for debugging the GL state by dumping it's contents
#if RND_GL_STATE_DEBUG
typedef struct
{
    char *name;
    GLenum enumeration;
    bool32 bDefault;
}
enumentry;
#define enumEntry(string, n)            {string, n, FALSE}
#define enumDefaultEntry(string, n)     {string, n, TRUE}
#define enumEnd                         {NULL, 0, FALSE}
#define enumError                       {"ERROR", 0xffffff8, TRUE}
enumentry rndBoolEnums[] =
{
    enumEntry("GL_TRUE", GL_TRUE),
    enumDefaultEntry("GL_FALSE", GL_FALSE),
    enumEnd
};
enumentry rndAlphaTestFuncEnum[] =
{
    enumEntry("GL_NEVER", GL_NEVER),
    enumEntry("GL_LESS", GL_LESS),
    enumEntry("GL_EQUAL", GL_EQUAL),
    enumEntry("GL_LEQUAL", GL_LEQUAL),
    enumEntry("GL_GREATER", GL_GREATER),
    enumEntry("GL_NOTEQUAL", GL_NOTEQUAL),
    enumEntry("GL_GEQUAL", GL_GEQUAL),
    enumEntry("GL_ALWAYS", GL_ALWAYS),
    enumError,
    enumEnd
};
enumentry rndBlendFuncEnums[] =
{
    enumEntry("GL_ZERO", GL_ZERO),
    enumEntry("GL_ONE", GL_ONE),
    enumEntry("GL_DST_COLOR", GL_DST_COLOR),
    enumEntry("GL_ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR),
    enumEntry("GL_SRC_ALPHA", GL_SRC_ALPHA),
    enumEntry("GL_ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA),
    enumEntry("GL_DST_ALPHA", GL_DST_ALPHA),
    enumEntry("GL_ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA),
    enumEntry("GL_SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE),
    enumError,
    enumEnd
};
enumentry rndFaceEnums[] =
{
    enumEntry("GL_FRONT", GL_FRONT),
    enumEntry("GL_BACK", GL_BACK),
    enumEntry("GL_FRONT_AND_BACK", GL_FRONT_AND_BACK),
    enumError,
    enumEnd
};
enumentry rndMatrixEnums[] =
{
    enumEntry("GL_MODELVIEW", GL_MODELVIEW),
    enumEntry("GL_PROJECTION", GL_PROJECTION),
    enumEntry("GL_TEXTURE", GL_TEXTURE),
    enumError,
    enumEnd
};
enumentry rndHintEnums[] =
{
    enumEntry("GL_FASTEST", GL_FASTEST),
    enumEntry("GL_NICEST", GL_NICEST),
    enumEntry("GL_DONT_CARE", GL_DONT_CARE),
    enumError,
    enumEnd
};
enumentry rndShadeModelEnums[] =
{
    enumEntry("GL_FLAT", GL_FLAT),
    enumEntry("GL_SMOOTH", GL_SMOOTH),
    enumError,
    enumEnd
};

enumentry rndTextureEnvEnums[] =
{
    enumEntry("GL_MODULATE", GL_MODULATE),
    enumEntry("GL_DECAL", GL_DECAL),
    enumEntry("GL_BLEND", GL_BLEND),
    enumEntry("GL_REPLACE", GL_REPLACE),
    enumError,
    enumEnd
};

typedef struct
{
    char *heading;
    GLenum enumeration;
    sdword type;
    sdword nValues;
    enumentry *enumTable;
}
glstateentry;

#define stateEntry(string, enumb, type, nValues, table)  {string, enumb, type, nValues, table}
#define G_Bool                  0
#define G_GetBool               1
#define G_Integer               2
#define G_Float                 3
#define G_FloatByte             4
#define G_IntFunc               5

//definitions for special-case functions
#define G_TextureEnv            0

glstateentry rndStateSaveTable[] =
{
    stateEntry("GL_ALPHA_TEST", GL_ALPHA_TEST,                                  G_Bool, 1, rndBoolEnums),
    stateEntry("GL_BLEND", GL_BLEND,                                            G_Bool, 1, rndBoolEnums),
    stateEntry("GL_CULL_FACE", GL_CULL_FACE,                                    G_Bool, 1, rndBoolEnums),
    stateEntry("GL_DEPTH_TEST", GL_DEPTH_TEST,                                  G_Bool, 1, rndBoolEnums),
    stateEntry("GL_FOG", GL_FOG,                                                G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LIGHT0", GL_LIGHT0,                                          G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LIGHT1", GL_LIGHT1,                                          G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LIGHTING", GL_LIGHTING,                                      G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LINE_SMOOTH", GL_LINE_SMOOTH,                                G_Bool, 1, rndBoolEnums),
    stateEntry("GL_LINE_STIPPLE", GL_LINE_STIPPLE,                              G_Bool, 1, rndBoolEnums),
    stateEntry("GL_NORMALIZE", GL_NORMALIZE,                                    G_Bool, 1, rndBoolEnums),
    stateEntry("GL_POINT_SMOOTH", GL_POINT_SMOOTH,                              G_Bool, 1, rndBoolEnums),
    stateEntry("GL_POLYGON_SMOOTH", GL_POLYGON_SMOOTH,                          G_Bool, 1, rndBoolEnums),
    stateEntry("GL_POLYGON_STIPPLE", GL_POLYGON_STIPPLE,                        G_Bool, 1, rndBoolEnums),
    stateEntry("GL_SCISSOR_TEST", GL_SCISSOR_TEST,                              G_Bool, 1, rndBoolEnums),
    stateEntry("GL_TEXTURE_2D", GL_TEXTURE_2D,                                  G_Bool, 1, rndBoolEnums),

    //stateEntry("GL_TEXTURE_ENV", G_TextureEnv,                                  G_IntFunc,   1, rndTextureEnvEnums),
    //stateEntry("GL_TEXTURE_2D_BINDING", GL_TEXTURE_2D_BINDING,                  G_Integer,   1, NULL),
    stateEntry("GL_ALPHA_TEST_FUNC", GL_ALPHA_TEST_FUNC,                        G_Integer,   1, rndAlphaTestFuncEnum),
    stateEntry("GL_ALPHA_TEST_REF", GL_ALPHA_TEST_REF,                          G_Float,     1, NULL),
    stateEntry("GL_BLEND_DST", GL_BLEND_DST,                                    G_Integer,   1, rndBlendFuncEnums),
    stateEntry("GL_BLEND_SRC", GL_BLEND_SRC,                                    G_Integer,   1, rndBlendFuncEnums),
    stateEntry("GL_BLUE_BIAS", GL_BLUE_BIAS,                                    G_FloatByte, 1, NULL),
    stateEntry("GL_COLOR_CLEAR_VALUE", GL_COLOR_CLEAR_VALUE,                    G_Float,     4, NULL),
    stateEntry("GL_COLOR_MATERIAL_FACE", GL_COLOR_MATERIAL_FACE,                G_Integer,   1, rndFaceEnums),
    stateEntry("GL_CULL_FACE_MODE", GL_CULL_FACE_MODE,                          G_Integer,   1, rndFaceEnums),
    stateEntry("GL_CURRENT_COLOR", GL_CURRENT_COLOR,                            G_FloatByte, 4, NULL),
    stateEntry("GL_CURRENT_RASTER_COLOR", GL_CURRENT_RASTER_COLOR,              G_FloatByte, 4, NULL),
    stateEntry("GL_CURRENT_RASTER_POSITION", GL_CURRENT_RASTER_POSITION,        G_Float,     4, NULL),
    stateEntry("GL_CURRENT_TEXTURE_COORDS", GL_CURRENT_TEXTURE_COORDS,          G_Float,     4, NULL),
    stateEntry("GL_DEPTH_BITS", GL_DEPTH_BITS,                                  G_Integer,   1, NULL),
    stateEntry("GL_DEPTH_WRITEMASK", GL_DEPTH_WRITEMASK,                        G_GetBool,   1, rndBoolEnums),
    stateEntry("GL_FOG_COLOR", GL_FOG_COLOR,                                    G_FloatByte, 4, NULL),
    stateEntry("GL_FOG_DENSITY", GL_FOG_DENSITY,                                G_Float,     1, NULL),
    stateEntry("GL_GREEN_BIAS", GL_GREEN_BIAS,                                  G_FloatByte, 1, NULL),
    stateEntry("GL_LIGHT_MODEL_AMBIENT", GL_LIGHT_MODEL_AMBIENT,                G_FloatByte, 4, NULL),
    stateEntry("GL_LIGHT_MODEL_TWO_SIDE", GL_LIGHT_MODEL_TWO_SIDE,              G_GetBool,   1, rndBoolEnums),
    stateEntry("GL_LINE_WIDTH", GL_LINE_WIDTH,                                  G_Float,     1, NULL),
    stateEntry("GL_LINE_WIDTH_GRANULARITY", GL_LINE_WIDTH_GRANULARITY,          G_Float,     1, NULL),
    stateEntry("GL_MATRIX_MODE", GL_MATRIX_MODE,                                G_Integer,   1, rndMatrixEnums),
    stateEntry("GL_MAX_TEXTURE_SIZE", GL_MAX_TEXTURE_SIZE,                      G_Integer,   1, NULL),
    stateEntry("GL_MAX_VIEWPORT_DIMS", GL_MAX_VIEWPORT_DIMS,                    G_Integer,   2, NULL),
    stateEntry("GL_MODELVIEW_MATRIX", GL_MODELVIEW_MATRIX,                      G_Float,     16, NULL),
    stateEntry("GL_PERSPECTIVE_CORRECTION_HINT", GL_PERSPECTIVE_CORRECTION_HINT,G_Integer,   1, rndHintEnums),
    stateEntry("GL_POINT_SIZE", GL_POINT_SIZE,                                  G_Float,     1, NULL),
    stateEntry("GL_POINT_SIZE_GRANULARITY", GL_POINT_SIZE_GRANULARITY,          G_Float,     1, NULL),
    stateEntry("GL_POLYGON_MODE", GL_POLYGON_MODE,                              G_Integer,   1, rndFaceEnums),
    stateEntry("GL_PROJECTION_MATRIX", GL_PROJECTION_MATRIX,                    G_Float,     16, NULL),
    stateEntry("GL_RED_BIAS", GL_RED_BIAS,                                      G_FloatByte, 1, NULL),
    stateEntry("GL_SCISSOR_BOX", GL_SCISSOR_BOX,                                G_Integer,   4, NULL),
    stateEntry("GL_SHADE_MODEL", GL_SHADE_MODEL,                                G_Integer,   1, rndShadeModelEnums),
    stateEntry("GL_SUBPIXEL_BITS", GL_SUBPIXEL_BITS,                            G_Integer,   1, NULL),
    stateEntry("GL_VIEWPORT", GL_VIEWPORT,                                      G_Integer,   1, NULL),
    {NULL, 0, 0, 0, NULL}
};
bool32 rndGLStateSaving = FALSE;
char rndGLStateLogFileName[128];
sdword rndGLStateLogIndex = 0;
#endif //RND_GL_STATE_DEBUG

#if RND_PLUG_DISABLEABLE
bool8 rndShamelessPlugEnabled = TRUE;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
#if RND_GL_STATE_DEBUG

/*-----------------------------------------------------------------------------
    Name        : rndIntToString
    Description : Convert a GL enumeration to a string
    Inputs      : enumb - enumeration to convert
                  table - table of enumerations/strings
    Outputs     :
    Return      : name of enumeration
----------------------------------------------------------------------------*/
char *rndIntToString(GLenum enumb, enumentry *table)
{
    sdword index;

    for (index = 0; table[index].name != NULL; index++)
    {
        if (enumb == table[index].enumeration || table[index].bDefault)
        {
            return(table[index].name);
        }
    }
    dbgAssert(FALSE);
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : rndGLStateLog
    Description : Log the state of the GL machine.
    Inputs      : location - user-specified name for where this function is called from
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#define MAX_FLOATS          16
#define MAX_INTS            4
#define MAX_BOOLS           1
void rndGLStateLogFunction(char *location)
{
    sdword index, j;
    GLfloat floats[MAX_FLOATS];
    GLint ints[MAX_INTS];
    GLboolean bools[MAX_BOOLS];
    char totalString[256];
    char valueString[128];
    FILE *f;

    f = fopen(rndGLStateLogFileName, "at");
    if (f == NULL)
    {
        dbgMessagef("\nError opening '%s' for GL state logging.", rndGLStateLogFileName);
        return;
    }

#if RND_GL_STATE_WINDOW
    dbgMessagef("\n******************** %s", location);
#endif
    fprintf(f, "******************** %s\n", location);
    for (index = 0; rndStateSaveTable[index].heading != NULL; index++)
    {
        sprintf(totalString, "%32s:", rndStateSaveTable[index].heading);
        switch (rndStateSaveTable[index].type)
        {
            case G_Bool:
                bools[0] = glIsEnabled(rndStateSaveTable[index].enumeration);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(bools[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", bools[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_GetBool:
                glGetBooleanv(rndStateSaveTable[index].enumeration, bools);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(bools[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", bools[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_Integer:
                glGetIntegerv(rndStateSaveTable[index].enumeration, ints);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(ints[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", ints[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_Float:
                glGetFloatv(rndStateSaveTable[index].enumeration, floats);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    sprintf(valueString, "%.2f", floats[j]);
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_FloatByte:
                glGetFloatv(rndStateSaveTable[index].enumeration, floats);
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    sprintf(valueString, "%d", colRealToUbyte(floats[j]));
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            case G_IntFunc:
                switch (rndStateSaveTable[index].enumeration)
                {
                    case G_TextureEnv:
//                        glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, ints);
                        break;
                    default:
                        dbgAssert(FALSE);
                }
                for (j = 0; j < rndStateSaveTable[index].nValues; j++)
                {
                    if (rndStateSaveTable[index].enumTable != NULL)
                    {
                        sprintf(valueString, "%s", rndIntToString(ints[j], rndStateSaveTable[index].enumTable));
                    }
                    else
                    {
                        sprintf(valueString, "%d", ints[j]);
                    }
                    strcat(totalString, valueString);
                    if (j + 1 < rndStateSaveTable[index].nValues)
                    {
                        strcat(totalString, ", ");
                    }
                }
                break;
            default:
                dbgAssert(FALSE);
        }
#if RND_GL_STATE_WINDOW
        dbgMessagef("\n%s", totalString);
#endif
        fprintf(f, "%s\n", totalString);
    }
    fclose(f);
}

#endif //RND_GL_STATE_DEBUG
#if RND_FRAME_RATE
/*-----------------------------------------------------------------------------
    Name        : rndFrameRateTaskFunction
    Description : Task function for the frame rate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void rndFrameRateTaskFunction(void)
{
    rndFrameRateStart = rndFrameRate = rndPrintCount = 0;   //it counters
    rndFrameCountMax = 0;                                   //set min/max counters
    rndFrameCountMin = SDWORD_Max;                          //to rediculous values

    taskYield(0);

    while (1)
    {
        taskStackSaveCond(0);
        rndFrameRate = rndFrameCount - rndFrameRateStart;   //number of frames since last printed
        rndFrameRateStart = rndFrameCount;                  //reset start frame
        rndFrameCountMax = max(rndFrameCountMax, rndFrameRate);//update min/max
        rndFrameCountMin = min(rndFrameCountMin, rndFrameRate);

        if (rndDisplayFrameRate)
        {                                                   //display frame rate
            dbgMessagef("\nFrame rate: %.2f <= %.2f <= %.2f",
                       (real32)(rndFrameCountMin) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameCountMax) / RND_FrameRateSeconds);
        }
        rndPrintCount++;
        if (rndPrintCount >= RND_FrameRateMaxRate)
        {                                                   //if time to reset min/max
            rndPrintCount = 0;
            rndFrameCountMax = 0;                           //set min/max counters
            rndFrameCountMin = SDWORD_Max;                  //to rediculous values
        }
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)
//toggle frame rate on/off
sdword rndFrameRateToggle(regionhandle region, sdword ID, udword event, udword data)
{
    rndDisplayFrameRate ^= TRUE;
    dbgMessagef("\nFrame rate printing %s", rndDisplayFrameRate ? "ON" : "OFF");
    return(0);
}

#endif //RND_FRAME_RATE

#if defined(COLLISION_CHECK_STATS) || defined(PROFILE_TIMERS)
udword rndCollStatsToggle(regionhandle region, sdword ID, udword event, udword data)
{
    rndDisplayCollStats ^= TRUE;
    dbgMessagef("\nCollision stats printing %s", rndDisplayCollStats ? "ON" : "OFF");
    return(0);
}
#endif //COLLISION_CHECK_STATS

#if RND_POLY_STATS
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void rndPolyStatsTaskFunction(void)
{
    rndNumberPolys    = 0;
    rndNumberTextured = 0;
    rndNumberSmoothed = 0;
    rndNumberDots     = 0;
    rndNumberLines    = 0;

    taskYield(0);

    while (1)
    {
        taskStackSaveCond(0);

        if (rndDisplayPolyStats && (rndPolyStatFrameCounter != 0 && rndNumberPolys != 0))
        {                                                   //display frame rate
            sprintf(rndPolyStatsString, "\n%d, (%.2f, %.2f), %d, %d",
                    rndNumberPolys / rndPolyStatFrameCounter,
                    100.0f * rndNumberTextured / rndNumberPolys,
                    100.0f * rndNumberSmoothed / rndNumberPolys, 10, 11);
            //print the poly count of the current LOD
            if (selSelected.numShips == 1)
            {
                static char polyCountString[256];
                static Ship *ship;
                static lod *lodInfo;
                static sdword index, polyCount, vertexCount, lodIndex;
                static meshdata *mesh;

                ship = selSelected.ShipPtr[0];
                if (lodTuningMode)
                {
                    lodIndex = rndLOD;
                }
                else
                {
                    lodIndex = ship->currentLOD;
                }
                if (lodIndex < ship->staticinfo->staticheader.LOD->nLevels)
                {
                    lodInfo = &ship->staticinfo->staticheader.LOD->level[lodIndex];
                    if ((lodInfo->flags & LM_LODType) == LT_Mesh)
                    {                                       //if it's a mesh type lod
                        mesh = (meshdata *)lodInfo->pData;  //get the mesh pointer
                        polyCount = rndCountTotalPolygons(lodInfo);
                        for (index = vertexCount = 0; index < mesh->nPolygonObjects; index++)
                        {                                   //count vertices and polygons
                            vertexCount += mesh->object[index].nVertices;
                        }
                        sprintf(polyCountString, ", %d, %d, %d", lodIndex, vertexCount, polyCount);
                        strcat(rndPolyStatsString, polyCountString);
                    }
                }
            }
            dbgMessage(rndPolyStatsString);
#if MESH_MATERIAL_STATS
            sprintf(meshMaterialStatsString, "\n%.2f / %.2f / %.2f",
                    (real32)meshMaterialChanges / (real32)rndPolyStatFrameCounter,
                    (real32)meshTotalMaterials / (real32)rndPolyStatFrameCounter,
                    (real32)meshMaterialChanges / (real32)meshTotalMaterials);
            dbgMessage(meshMaterialStatsString);
#endif //MESH_MATERIAL_STATS
        }
        rndPrintCount++;
        rndPolyStatsColor = colReddish;
        rndNumberPolys    = 0;
        rndNumberTextured = 0;
        rndNumberSmoothed = 0;
        rndNumberDots     = 0;
        rndNumberLines    = 0;
        rndPolyStatFrameCounter = 0;
#if MESH_MATERIAL_STATS
        meshMaterialChanges = 0;
        meshTotalMaterials = 0;
#endif //MESH_MATERIAL_STATS
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)
//toggle frame rate on/off
sdword rndPolyStatsToggle(regionhandle region, sdword ID, udword event, udword data)
{
    rndDisplayPolyStats ^= TRUE;
    dbgMessagef("\nFrame rate printing %s", rndDisplayPolyStats ? "ON" : "OFF");
    if (rndDisplayPolyStats)
    {
        strcpy(rndPolyStatsString, "\nnPolys (%textured, %smoothed), nDots, nLines, shipLod, nVertices, nPolys");
        dbgMessage(rndPolyStatsString);
        rndPolyStatsColor = colWhite;
#if MESH_MATERIAL_STATS
        strcpy(meshMaterialStatsString, "\nnMaterialChanges / nMaterials / factor");
        dbgMessage(meshMaterialStatsString);
#endif //MESH_MATERIAL_STATS
    }
    return(0);
}

#endif //RND_POLY_STATS

static real32 scalar;
void rndCapScaleCapStatsTaskFunction(void)
{
    if (selSelected.numShips == 1)
    {
        Ship* ship = selSelected.ShipPtr[0];
        ShipStaticInfo* shipstaticinfo = (ShipStaticInfo*)ship->staticinfo;

        if (ship->trail[0] == NULL)
        {
            scalar = 0.0f;
        }
        else
        {
            scalar = ship->trail[0]->scalecap;
            if (scalar == -1.0f)
            {
                scalar = 0.0f;
            }
        }
    }
    else
    {
        scalar = 0.0f;
    }

    sprintf(rndCapScaleCapStatsString, "\n trailScaleCap %f", scalar);
}

bool32 setupPixelFormat(HDC hDC)
{
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  /* size */
        1,              /* version */
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,       /* support double-buffering */
        PFD_TYPE_RGBA,          /* color type */
//        16,             /* prefered color depth */
        MAIN_WindowDepth,
        0, 0, 0, 0, 0, 0,       /* color bits (ignored) */
        0,              /* no alpha buffer */
        0,              /* alpha bits (ignored) */
        0,              /* no accumulation buffer */
        0, 0, 0, 0,         /* accum bits (ignored) */
        16,             /* depth buffer */
        0,              /* no stencil buffer */
        0,              /* no auxiliary buffers */
        PFD_MAIN_PLANE,         /* main layer */
        0,              /* reserved */
        0, 0, 0,            /* no layer, visible, damage masks */
    };

    if (rwglChoosePixelFormat == NULL || glNT)
    {
        pixelFormat = ChoosePixelFormat(hDC, &pfd);
    }
    else
    {
        pixelFormat = rwglChoosePixelFormat(hDC, &pfd);
    }
    if (pixelFormat == 0)
    {
        dbgMessagef("\nChoosePixelFormat failed (%d).", GetLastError());
        return FALSE;
    }

    if (rwglSetPixelFormat == NULL || glNT)
    {
        if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
        {
            dbgMessagef("\nSetPixelFormat failed (%d).", GetLastError());
            return FALSE;
        }
    }
    else
    {
        if (rwglSetPixelFormat(hDC, pixelFormat, &pfd) != TRUE)
        {
            dbgMessagef("\nSetPixelFormat failed (%d).", GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

bool32 setupPalette(HDC hDC)
{
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;

    if (rwglGetPixelFormat == NULL || glNT)
    {
        pixelFormat = GetPixelFormat(hDC);
    }
    else
    {
        pixelFormat = rwglGetPixelFormat(hDC);
    }

    if (rwglDescribePixelFormat == NULL || glNT)
    {
        DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    }
    else
    {
        rwglDescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    }

    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        dbgMessage("rndInit: needs paletted display");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

typedef int (*AUXINITPOSITIONPROC)(GLuint, GLuint, GLuint, GLuint, GLuint);

sdword rndSmallInit(rndinitdata* initData, bool32 GL)
{
    if (GL)
    {
        hGLWindow = initData->hWnd;
        hGLDeviceContext = GetDC(initData->hWnd);
        if (!setupPixelFormat(hGLDeviceContext))
        {
            return FALSE;
        }
        if (!setupPalette(hGLDeviceContext))
        {
            return FALSE;
        }
        hGLRenderContext = (HGLRC)rwglCreateContext((unsigned int)hGLDeviceContext);
        rwglMakeCurrent((int)hGLDeviceContext, (int)hGLRenderContext);
    }
    else
    {
        AUXINITPOSITIONPROC initPositionProc;

        initPositionProc = (AUXINITPOSITIONPROC)rwglGetProcAddress("rauxInitPosition");
        dbgMessagef("rndSmallInit: rauxInitPosition=%p", initPositionProc);
        dbgAssert(initPositionProc != NULL);

        hGLWindow = initData->hWnd;
        dbgMessagef("rndSmallInit: calling rglCreateWindow(hwnd=%p, w=%d, depth=%d)", hGLWindow, MAIN_WindowWidth, MAIN_WindowDepth);
        rglCreateWindow((GLint)hGLWindow, (GLint)MAIN_WindowWidth, (GLint)MAIN_WindowDepth);

        dbgMessagef("rndSmallInit: calling rauxInitPosition(0, 0, %d, %d, %d)", initData->width, initData->height, MAIN_WindowDepth);
        if (!initPositionProc(0, 0, initData->width, initData->height, MAIN_WindowDepth))
        {
            dbgMessage("rndSmallInit: rauxInitPosition FAILED");
            return FALSE;
        }
    }

    glLockArraysEXT = (LOCKARRAYSEXTproc)rwglGetProcAddress("glLockArraysEXT");
    glUnlockArraysEXT = (UNLOCKARRAYSEXTproc)rwglGetProcAddress("glUnlockArraysEXT");

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : rndInit
    Description : Initialize the rendering system.
    Inputs      : initData - pointer to a structure containing initialization
                    info.
    Outputs     : ??
    Return      : ERROR if an error occurred.
----------------------------------------------------------------------------*/
static GLfloat lightPosition0[] = {10000.0f, 1.0f, 1.0f, 0.1f};

sdword rndInit(rndinitdata *initData)
{
    static GLfloat  ambientProperties[] = {0.9f, 0.5f, 0.5f, 1.0f};
    static GLfloat  diffuseProperties[] = {0.8f, 0.8f, 0.8f, 1.0f};
    static GLfloat  specularProperties[] = {1.0f, 1.0f, 1.0f, 1.0f};

    if (!RGL)
    {
        hGLWindow = initData->hWnd;
        hGLDeviceContext = GetDC(initData->hWnd);
        if (!setupPixelFormat(hGLDeviceContext))
        {
            dbgMessage("\nrndInit: GL couldn't setupPixelFormat");
            return !OKAY;
        }
        if (!setupPalette(hGLDeviceContext))
        {
            dbgMessage("\nrndInit: GL couldn't setupPalette");
            return !OKAY;
        }
        hGLRenderContext = (HGLRC)rwglCreateContext((unsigned int)hGLDeviceContext);  //greate GL render context and select into window
        rwglMakeCurrent((int)hGLDeviceContext, (int)hGLRenderContext);

//        auxInitPosition(0, 0, initData->width, initData->height);   //create the viewport for rendering
//        auxInitDisplayMode(AUX_RGB | AUX_DEPTH | AUX_DOUBLE);       //set mode (direct color, Z-buffered, double buffered)
    }
    else
    {
        AUXINITPOSITIONPROC initPositionProc;

        initPositionProc = (AUXINITPOSITIONPROC)rwglGetProcAddress("rauxInitPosition");
        dbgAssert(initPositionProc != NULL);

        hGLWindow = initData->hWnd;
        rglCreateWindow((GLint)hGLWindow, (GLint)MAIN_WindowWidth, (GLint)MAIN_WindowDepth);

        if (!initPositionProc(0, 0, initData->width, initData->height, MAIN_WindowDepth))
        {
            dbgMessage("\nrndInit: RGL couldn't initPositionProc");
            return !OKAY;
        }
    }

    glCapStartup();
    if (!glCapFeatureExists(GL_SHARED_TEXTURE_PALETTE_EXT))
    {
        trNoPalettes = TRUE;
    }

#if RND_VERBOSE_LEVEL >= 1
    dbgMessagef("\nrndInit: OpenGL Vendor     :%s", glGetString(GL_VENDOR));
    dbgMessagef("\nrndInit: OpenGL Renderer   :%s", glGetString(GL_RENDERER));
    dbgMessagef("\nrndInit: OpenGL Version    :%s", glGetString(GL_VERSION));
    dbgMessagef("\nrndInit: OpenGL Extensions :%s", glGetString(GL_EXTENSIONS));
#endif
    rndSetClearColor(colRGBA(0,0,0,255));
    glClearDepth( 1.0 );

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    rndAdditiveBlending = FALSE;

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);

    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientProperties);
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseProperties);
    glLightfv( GL_LIGHT0, GL_SPECULAR, specularProperties);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0f);
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientProperties);

    glEnable( GL_LIGHT0 );

//    glShadeModel(GL_FLAT);

    glCullFace(GL_BACK);                                    //enable culling of back-facing polys
    glEnable(GL_CULL_FACE);

    rndPreObjectCallback = rndPostObjectCallback = NULL;    //no render callbacks yet

    rndAspectRatio = (GLfloat)initData->width / (GLfloat)initData->height;

    rndFrameCount = 0;

#if RND_FRAME_RATE
    rndFrameRateTaskHandle = taskStart(rndFrameRateTaskFunction, RND_FrameRatePeriod, 0);//start frame rate task
    regKeyChildAlloc(ghMainRegion, RND_FrameRateKey, RPE_KeyDown, rndFrameRateToggle, 1, RND_FrameRateKey);
#endif//RND_FRAME_RATE
#if defined(COLLISION_CHECK_STATS) || defined(PROFILE_TIMERS)
    regKeyChildAlloc(ghMainRegion, RND_CollStatsKey, RPE_KeyDown, rndCollStatsToggle, 1, RND_CollStatsKey);
#endif
#if RND_POLY_STATS
    rndPolyStatsTaskHandle = taskStart(rndPolyStatsTaskFunction, RND_PolyStatsPeriod, 0);//start frame rate task
    regKeyChildAlloc(ghMainRegion, RND_PolyStatsKey, RPE_KeyDown, rndPolyStatsToggle, 1, RND_PolyStatsKey);
#endif //RND_POLY_STATS

#if TEST_SPAN_PERFORMANCE
    testHook();
#endif

    glDepthFunc(glCapDepthFunc);

    glLockArraysEXT = (LOCKARRAYSEXTproc)rwglGetProcAddress("glLockArraysEXT");
    glUnlockArraysEXT = (UNLOCKARRAYSEXTproc)rwglGetProcAddress("glUnlockArraysEXT");

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : rndClose
    Description : Closes the rendering module.
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void rndClose(void)
{
    if (RGL)
    {
        rglDeleteWindow((GLint)hGLWindow);
    }
    else
    {
        if (hGLRenderContext)
        {
            rwglMakeCurrent(0, 0);                              //release GL render context
            rwglDeleteContext((int)hGLRenderContext);
        }
        ReleaseDC(hGLWindow, hGLDeviceContext);                 //release the Device context
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndBillboardEnable
    Description : Enable billboard mode
    Inputs      : centre - location of the centre or 'pivot' point of the
                    billboard geometry.
    Outputs     : Loads identity for modelview matrix and translates by
                    (cameramatrix * centre) which is stored in rndBillboardCentre.
    Return      : void
    Note        : Billboard geometry should be drawn on the z = 0 plane
                    about x = y = 0.
----------------------------------------------------------------------------*/
void rndBillboardEnable(vector *centre)
{
    hvector hCentre;

#if RND_ERROR_CHECKING
    if (rndBillboardEnabled)
    {
        dbgFatal(DBG_Loc, "Billboarding already enabled");
    }
    rndBillboardEnabled = TRUE;
#endif
    glLoadIdentity();
    hCentre.x = centre->x;
    hCentre.y = centre->y;
    hCentre.z = centre->z;
    hCentre.w = 1.0f;
    hmatMultiplyHMatByHVec(&rndBillboardCentre, &rndCameraMatrix, &hCentre);
    glTranslatef(rndBillboardCentre.x, rndBillboardCentre.y, rndBillboardCentre.z);
}

/*-----------------------------------------------------------------------------
    Name        : rndBillboardDisable
    Description : Disable billboard mode.
    Inputs      : void
    Outputs     : Loads rndCameraMatrix into modelview (should only be 1
                    level on the model-view matrix stack).
    Return      : void
----------------------------------------------------------------------------*/
void rndBillboardDisable(void)
{
#if RND_ERROR_CHECKING
    if (rndBillboardEnabled == FALSE)
    {
        dbgFatal(DBG_Loc, "Billboarding already disabled.");
    }
    rndBillboardEnabled = FALSE;
#endif
    glLoadMatrixf((GLfloat *)(&rndCameraMatrix));    //restore previous camera matrix
}

void rndFilter(bool32 on)
{
    if (on)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

static real64 rndNear(real64 in)
{
    return (RGLtype == D3Dtype) ? 0.5 * in : in;
}

static sdword rndCountMeshPolygons(polygonobject *object)
{
    sdword polygonCount;
    polygonobject *childObject;

    polygonCount = object->nPolygons;
    for (childObject = object->pDaughter; childObject != NULL; childObject = childObject->pSister)
    {
        polygonCount += rndCountMeshPolygons(childObject);
    }

    return polygonCount;
}

static sdword rndCountTotalPolygons(lod *level)
{
    meshdata *mesh;
    sdword objectIndex;
    sdword polygonCount;

    if (level == NULL || (level->flags & LM_LODType) != LT_Mesh || level->pData == NULL)
    {
        return 0;
    }

    mesh = (meshdata *)level->pData;
    polygonCount = 0;
    for (objectIndex = 0; objectIndex < mesh->nPolygonObjects; objectIndex++)
    {
        if (mesh->object[objectIndex].pMother == NULL)
        {
            polygonCount += rndCountMeshPolygons(&mesh->object[objectIndex]);
        }
    }

    return polygonCount;
}

typedef struct
{
    ubyte state;
    ubyte pad0[3];
    real32 expiryTime;
    vector translationDelta;
    udword unk14;
    udword unk18;
    udword unk1C;
    bool32 spinActive;      // retail Ship+0x658
    real32 spinAngle;       // retail Ship+0x65C
    vector spinAxis;        // retail Ship+0x660
    bool32 deleteOnExpire;
    real32 startTime;
} rndComponentShipSpec;

static_assert(offsetof(rndComponentShipSpec, spinActive) == 0x20, "rndComponentShipSpec::spinActive offset mismatch");
static_assert(offsetof(rndComponentShipSpec, spinAngle) == 0x24, "rndComponentShipSpec::spinAngle offset mismatch");
static_assert(offsetof(rndComponentShipSpec, spinAxis) == 0x28, "rndComponentShipSpec::spinAxis offset mismatch");
static_assert(sizeof(rndComponentShipSpec) == 0x3C, "rndComponentShipSpec size mismatch");

static void rndApplyComponentSpinTransform(SpaceObj *spaceobj)
{
    Ship *ship;
    rndComponentShipSpec *componentSpec;
    vector *pivot;

    if (spaceobj->objtype != OBJ_ShipType)
    {
        return;
    }

    ship = (Ship *)spaceobj;
    if (ship->staticinfo->shipclass != CLASS_Component)
    {
        return;
    }

    componentSpec = (rndComponentShipSpec *)ship->ShipSpecifics;
    if (!componentSpec->spinActive)
    {
        return;
    }

    pivot = &ship->staticinfo->staticheader.staticCollInfo.collsphereoffset;
    glTranslatef(pivot->x, pivot->y, pivot->z);
    glRotatef(RAD_TO_DEG(componentSpec->spinAngle), componentSpec->spinAxis.x, componentSpec->spinAxis.y, componentSpec->spinAxis.z);
    glTranslatef(-pivot->x, -pivot->y, -pivot->z);
}

/*-----------------------------------------------------------------------------
    Name        : rndBackgroundRender
    Description : Render the background of a mission sphere
    Inputs      : radius - radius of mission sphere
                  galaxy - pointer to galaxyinfo structure describing background
    Outputs     : renders each segment as a quadrangle about the mission sphere
    Return      : void
----------------------------------------------------------------------------*/
void rndBackgroundRender(real32 radius, Camera* camera, bool32 bDrawStars)
{
    real32 projection[16];
    sdword index;

    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(btgFieldOfView, rndAspectRatio,     //set projection matrix
                    rndNear(camera->clipPlaneNear), camera->clipPlaneFar);
    glMatrixMode(GL_MODELVIEW);

    if (showBackgrounds && gameIsRunning)
    {
        btgRender();
    }

    //draw the stars in the mission sphere
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, 0);
    rndTextureEnable(FALSE);
    if (bDrawStars && gameIsRunning)
    {
        bool32 blends, pointSize;

        blends = glCapFeatureExists(GL_POINT_SMOOTH);
        pointSize = glCapFeatureExists(GL_POINT_SIZE);

        glPointSize(1.0f);

        if (blends)
        {
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_BLEND);
            //additively blend only if background are enabled
            rndAdditiveBlends(showBackgrounds);
            if (pointSize && (MAIN_WindowWidth > 1024))
            {
                glPointSize(1.2f);
            }
        }

        //need to reset the projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        rgluPerspective(btgFieldOfView, rndAspectRatio,
                        rndNear(camera->clipPlaneNear), camera->clipPlaneFar);

        //draw small stars
        if (glCapFeatureExists(GL_VERTEX_ARRAY))
        {
            glInterleavedArrays(GL_C4UB_V3F, 0, (GLvoid*)stararray);
            glDrawArrays(GL_POINTS, 0, universe.star3dinfo->Num3dStars - NUM_BIG_STARS);
        }
        else
        {
            glBegin(GL_POINTS);
            for (index = 0; index < (universe.star3dinfo->Num3dStars - NUM_BIG_STARS); index++)
            {
                glColor4ub(stararray[index].c[0], stararray[index].c[1],
                           stararray[index].c[2], stararray[index].c[3]);
                glVertex3fv((GLfloat*)&stararray[index].v);
            }
            glEnd();
        }

        if (blends && pointSize)
        {
            switch (MAIN_WindowWidth)
            {
            case 640:
            case 800:
                glPointSize(2.1f);
                break;
            case 1024:
                glPointSize(2.5f);
                break;
            case 1280:
                glPointSize(2.65f);
                break;
            default:
                glPointSize(2.8f);
            }
        }

        //draw big stars
        if (glCapFeatureExists(GL_VERTEX_ARRAY))
        {
            glInterleavedArrays(GL_C4UB_V3F, 0, (GLvoid*)bigstararray);
            glDrawArrays(GL_POINTS, 0, NUM_BIG_STARS);
        }
        else
        {
            glBegin(GL_POINTS);
            for (index = 0; index < NUM_BIG_STARS; index++)
            {
                glColor4ub(bigstararray[index].c[0], bigstararray[index].c[1],
                           bigstararray[index].c[2], bigstararray[index].c[3]);
                glVertex3fv((GLfloat*)&bigstararray[index].v);
            }
            glEnd();
        }

        if (blends)
        {
            glDisable(GL_BLEND);
            glDisable(GL_POINT_SMOOTH);
            if (pointSize)
            {
                glPointSize(1.0f);
            }
        }
    }

    rndAdditiveBlends(FALSE);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);
    glMatrixMode(GL_MODELVIEW);
}

/*-----------------------------------------------------------------------------
    Name        : rndCameraInside
    Description : decides whether the camera is inside a ship or not
    Inputs      : spaceobj - the object to test
                  camera - the current camera object
    Outputs     :
    Return      : TRUE if camera is inside, FALSE otherwise
----------------------------------------------------------------------------*/
bool32 rndCameraInside(SpaceObj* spaceobj, Camera* camera)
{
    if ((spaceobj->flags & SOF_Rotatable) && (spaceobj->flags & SOF_Impactable))
    {
        vector dist;
        real32 collspheresize;

        collspheresize = spaceobj->staticinfo->staticheader.staticCollInfo.collspheresize;
        collspheresize *= collspheresize;

        //sphere test
        vecSub(dist, spaceobj->posinfo.position, camera->eyeposition);
        if (vecMagnitudeSquared(dist) > collspheresize)
        {
            //further away than the bounding sphere, trivially reject
            return FALSE;
        }

        //bounding-box test (final)
        return collCheckBBoxPoint((SpaceObjRotImp*)spaceobj, &camera->eyeposition);
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndInsideShip
    Description : determines whether the camera is inside a ship
    Inputs      : spaceobj - the object to consider
                  camera - current camera object
    Outputs     :
    Return      : TRUE if camera inside, else FALSE
----------------------------------------------------------------------------*/
bool32 rndInsideShip(SpaceObj* spaceobj, Camera* camera)
{
    if (nisIsRunning)
    {
        return FALSE;
    }
    if (bitTest(universe.mainCameraCommand.ccMode,CCMODE_PILOT_SHIP))
    {
        return FALSE;
    }
    return rndCameraInside(spaceobj, camera);
}

/*-----------------------------------------------------------------------------
    Name        : rndShipVisible
    Description : determines if a SpaceObj is within the viewing frustum via
                  a few different means
    Inputs      : spaceobj - the spaceobject to consider
                  camera - current camera object
    Outputs     :
    Return      : TRUE if ship is visible, else FALSE
----------------------------------------------------------------------------*/
bool32 rndShipVisible(SpaceObj* spaceobj, Camera* camera)
{
    vector veye, vobj;

    vecSub(veye, camera->eyeposition, camera->lookatpoint);
    vecSub(vobj, camera->eyeposition, spaceobj->posinfo.position);

    if ((spaceobj->flags & SOF_Rotatable) && (spaceobj->flags & SOF_Impactable))
    {
        //bbox frustum culling
        StaticCollInfo* sinfo = &((SpaceObjRotImp*)spaceobj)->staticinfo->staticheader.staticCollInfo;

#if 0 //GLcompat
        return !rglIsClipped((GLfloat*)&sinfo->collrectoffset,
                             sinfo->uplength, sinfo->rightlength,
                             sinfo->forwardlength);
#else
        return !clipBBoxIsClipped((real32*)&sinfo->collrectoffset,
                                  sinfo->uplength, sinfo->rightlength,
                                  sinfo->forwardlength);
#endif
    }
    else
    {
        //sphere test

        if (spaceobj->objtype == OBJ_ShipType)
        {
            //account for ship's radius
            real32 radius = ((Ship*)spaceobj)->staticinfo->staticheader.staticCollInfo.approxcollspheresize;
            return (vecDotProduct(veye, vobj) > -radius);
        }
        else
        {
            return (vecDotProduct(veye, vobj) > 0.0f);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndShipVisibleUsingCoordSys
    Description : calls rndShipVisible after setting up appropriate matrices in the GL
    Inputs      : spaceobj - the spaceobject to consider
                  camera - current camera object
    Outputs     :
    Return      : TRUE if visible, FALSE otherwise
----------------------------------------------------------------------------*/
/* Retail uses unresolved runtime/static slots in this visibility prepass; keep the raw offsets local here. */
enum
{
    RND_MISSILE_VISIBILITY_SCALE_OFFSET = 0x110,
    RND_SHIP_VISIBILITY_SCALE_OFFSET = 0x30C,
    RND_MISSILE_VISIBILITY_FADE_OFFSET = 0x254,
    RND_SHIP_VISIBILITY_FADE_OFFSET = 0x33C,
    RND_SHIP_REPLACEMENT_TYPE_OFFSET = 0x704,
    RND_SHIP_STATIC_STATE_OFFSET = 0x760,
    RND_SHIP_TEMPORARY_STATICINFO_BIT = 0x00000008,
};

static real32 rndVisibilityStaticScale(const void *staticinfo, size_t offset)
{
    return *(const real32 *)((const ubyte *)staticinfo + offset);
}

static real32 *rndVisibilityFadePtr(void *spaceobj, size_t offset)
{
    return (real32 *)((ubyte *)spaceobj + offset);
}

static real32 rndSelectionCircleCameraSpaceZ(const SpaceObjRotImpTarg *spaceobj)
{
    return -(spaceobj->collInfo.selCircleDepth / SEL_DepthSelectMultiplier) * CAMERA_MAX_ZOOMOUT_DISTANCE;
}

static real32 rndShipVisibilityScale(Ship *ship)
{
    ShipType replacementType;
    ubyte staticState;

    if (ship->objtype != OBJ_ShipType || !bitTest(ship->flags2, RND_SHIP_TEMPORARY_STATICINFO_BIT))
    {
        return 0.0f;
    }

    staticState = *(ubyte *)((ubyte *)ship + RND_SHIP_STATIC_STATE_OFFSET);
    switch (staticState)
    {
        case 0:
        case 6:
        case 8:
        case 9:
            return rndVisibilityStaticScale(ship->staticinfo, RND_SHIP_VISIBILITY_SCALE_OFFSET);

        default:
            replacementType = *(ShipType *)((ubyte *)ship + RND_SHIP_REPLACEMENT_TYPE_OFFSET);
            if (replacementType == (ShipType)-1)
            {
                return 0.0f;
            }
            return rndVisibilityStaticScale(
                GetShipStaticInfo(replacementType, GetValidRaceForShipType(replacementType)),
                RND_SHIP_VISIBILITY_SCALE_OFFSET);
    }
}

bool32 rndShipVisibleUsingCoordSys(SpaceObj* spaceobj, Camera* camera)
{
    GLfloat projection[16], modelview[16];
    hmatrix coordMatrixForGL;
    bool32    result;
    real32 visibilityScale;
    real32 visibilityFade;
    real32 cameraSpaceZ;
    Ship *ship;
    Missile *missile;
    SelectCommand *selection;
    sdword i;

    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_MODELVIEW_MATRIX,  modelview);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(camera->fieldofview, rndAspectRatio,
                    rndNear(camera->clipPlaneNear), camera->clipPlaneFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    rgluLookAt(camera->eyeposition.x, camera->eyeposition.y, camera->eyeposition.z,
               camera->lookatpoint.x, camera->lookatpoint.y, camera->lookatpoint.z,
               camera->upvector.x, camera->upvector.y, camera->upvector.z);

    hmatMakeHMatFromMat(&coordMatrixForGL, &((SpaceObjRot*)spaceobj)->rotinfo.coordsys);
    hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position, coordMatrixForGL);
    glMultMatrixf((GLfloat*)&coordMatrixForGL);

    selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);
    cameraSpaceZ = rndSelectionCircleCameraSpaceZ((SpaceObjRotImpTarg *)spaceobj);

    if (spaceobj->objtype == OBJ_MissileType)
    {
        missile = (Missile *)spaceobj;
        if (cameraSpaceZ >= 0.0f ||
            rndVisibilityStaticScale(missile->staticinfo, RND_MISSILE_VISIBILITY_SCALE_OFFSET) == 0.0f)
        {
            *rndVisibilityFadePtr(missile, RND_MISSILE_VISIBILITY_FADE_OFFSET) = 1.0f;
        }
        else
        {
            visibilityFade = 1.0f -
                (1000.0f + cameraSpaceZ) *
                rndVisibilityStaticScale(missile->staticinfo, RND_MISSILE_VISIBILITY_SCALE_OFFSET);
            if (visibilityFade < 1.0f)
            {
                visibilityFade = 1.0f;
            }
            *rndVisibilityFadePtr(missile, RND_MISSILE_VISIBILITY_FADE_OFFSET) = visibilityFade;
        }
    }
    else if (spaceobj->objtype == OBJ_ShipType)
    {
        ship = (Ship *)spaceobj;
        if (ship->clampInfo == NULL || ship->clampInfo->host == NULL)
        {
            if (cameraSpaceZ >= 0.0f ||
                rndVisibilityStaticScale(ship->staticinfo, RND_SHIP_VISIBILITY_SCALE_OFFSET) == 0.0f)
            {
                *rndVisibilityFadePtr(ship, RND_SHIP_VISIBILITY_FADE_OFFSET) = 1.0f;
            }
            else
            {
                visibilityScale = 1000.0f + cameraSpaceZ;
                if (bitTest(ship->flags2, RND_SHIP_TEMPORARY_STATICINFO_BIT))
                {
                    visibilityScale *= rndShipVisibilityScale(ship);
                }
                else
                {
                    visibilityScale *= rndVisibilityStaticScale(ship->staticinfo, RND_SHIP_VISIBILITY_SCALE_OFFSET);
                }
                visibilityFade = 1.0f - visibilityScale;
                if (visibilityFade < 1.0f)
                {
                    visibilityFade = 1.0f;
                }
                *rndVisibilityFadePtr(ship, RND_SHIP_VISIBILITY_FADE_OFFSET) = visibilityFade;
            }

            ship->collInfo.selCircleRadius *= *rndVisibilityFadePtr(ship, RND_SHIP_VISIBILITY_FADE_OFFSET);
            if (ship->clampInfo != NULL)
            {
                selection = ship->clampInfo->selection;
                if (selection != NULL)
                {
                    for (i = 0; i < selection->numShips; i++)
                    {
                        *rndVisibilityFadePtr(selection->ShipPtr[i], RND_SHIP_VISIBILITY_FADE_OFFSET) =
                            *rndVisibilityFadePtr(ship, RND_SHIP_VISIBILITY_FADE_OFFSET);
                        selection->ShipPtr[i]->collInfo.selCircleRadius *=
                            *rndVisibilityFadePtr(ship, RND_SHIP_VISIBILITY_FADE_OFFSET);
                    }
                }
            }
        }
    }

    result = rndShipVisible(spaceobj, camera);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelview);

    return result;
}

/*-----------------------------------------------------------------------------
    Name        : rndFade
    Description : configures the rendering pipeline for fading ships out of existence
                  at the edge of the renderlist
    Inputs      : spaceobj - the object
                  camera - the camera
    Outputs     :
    Return      : TRUE if the ship is not visible (totally faded), FALSE otherwise
----------------------------------------------------------------------------*/
bool32 rndFade(SpaceObj* spaceobj, Camera* camera)
{
    vector distvec;
    real32 distsqr0, distsqr1, distsqr;
    real32 fadedist, maxdist, mindist;

    real32 mult = 1.3f;

    vecSub(distvec, camera->lookatpoint, spaceobj->posinfo.position);
    distsqr0 = vecMagnitudeSquared(distvec);

    vecSub(distvec, camera->eyeposition, spaceobj->posinfo.position);
    distsqr1 = vecMagnitudeSquared(distvec);

    distsqr = (distsqr0 < distsqr1) ? distsqr0 : distsqr1;

    if (spaceobj->objtype == OBJ_ShipType)
    {
        switch (((ShipStaticInfo*)spaceobj->staticinfo)->shipclass)
        {
        case CLASS_Mothership:
            fadedist = RENDER_FADE_MOTHERSHIP;
            maxdist  = RENDER_LIMIT_MOTHERSHIP;
            break;
        case CLASS_HeavyCruiser:
            fadedist = RENDER_FADE_HEAVYCRUISER;
            maxdist  = RENDER_LIMIT_HEAVYCRUISER;
            break;
        case CLASS_Carrier:
            fadedist = RENDER_FADE_CARRIER;
            maxdist  = RENDER_LIMIT_CARRIER;
            break;
        case CLASS_Destroyer:
            fadedist = RENDER_FADE_DESTROYER;
            maxdist  = RENDER_LIMIT_DESTROYER;
            break;
        case CLASS_Frigate:
            fadedist = RENDER_FADE_FRIGATE;
            maxdist  = RENDER_LIMIT_FRIGATE;
            break;
        case CLASS_Corvette:
            fadedist = RENDER_FADE_CORVETTE;
            maxdist  = RENDER_LIMIT_CORVETTE;
            break;
        case CLASS_Fighter:
            fadedist = RENDER_FADE_FIGHTER;
            maxdist  = RENDER_LIMIT_FIGHTER;
            break;
        case CLASS_Resource:
            fadedist = RENDER_FADE_RESOURCE;
            maxdist  = RENDER_LIMIT_RESOURCE;
            break;
        case CLASS_NonCombat:
            fadedist = RENDER_FADE_NONCOMBAT;
            maxdist  = RENDER_LIMIT_NONCOMBAT;
            break;
        default:
            fadedist = RENDER_FADE_FRIGATE;     //happy medium
            maxdist  = RENDER_LIMIT_FRIGATE;
        }

        if (((ShipStaticInfo*)spaceobj->staticinfo)->renderlistFade != 0.0f)
        {
            fadedist = ((ShipStaticInfo*)spaceobj->staticinfo)->renderlistFade;
            maxdist  = ((ShipStaticInfo*)spaceobj->staticinfo)->renderlistLimitSqr;
        }

        fadedist *= mult;
        maxdist = mult * fsqrt(maxdist);
    }
    else if (spaceobj->objtype == OBJ_DerelictType)
    {
        if (((DerelictStaticInfo*)spaceobj->staticinfo)->renderlistFade != 0.0f)
        {
            fadedist = ((DerelictStaticInfo*)spaceobj->staticinfo)->renderlistFade;
            maxdist  = ((DerelictStaticInfo*)spaceobj->staticinfo)->renderlistLimitSqr;
        }
        else if (spaceobj->flags & SOF_BigObject)
        {
            fadedist = RENDER_MAXVIEWABLE_FADE;
            maxdist  = RENDER_MAXVIEWABLE_DISTANCE_SQR;
        }
        else
        {
            fadedist = RENDER_VIEWABLE_FADE;
            maxdist  = RENDER_VIEWABLE_DISTANCE_SQR;
        }

        fadedist *= mult;
        maxdist = mult * fsqrt(maxdist);
    }
    else
    {
        if (spaceobj->objtype == OBJ_AsteroidType &&
            bitTest(spaceobj->attributesParam, ATTRIBUTES_HeadShotVelToMothership))
        {
            fadedist = mult * RENDER_FADE_FRIGATE;
            maxdist  = mult * fsqrt(RENDER_LIMIT_FRIGATE);
        }
        else if (spaceobj->flags & SOF_BigObject)
        {
            fadedist = mult * RENDER_MAXVIEWABLE_FADE;
            maxdist  = mult * fsqrt(RENDER_MAXVIEWABLE_DISTANCE_SQR);
        }
        else
        {
            fadedist = mult * RENDER_VIEWABLE_FADE;
            maxdist  = mult * fsqrt(RENDER_VIEWABLE_DISTANCE_SQR);
        }
    }

    mindist = maxdist - fadedist;
    fadedist *= fadedist;
    mindist *= mindist;

    if (distsqr < mindist || rndDamageFlashBiasActive)
    {
        if (RGL && !usingShader)
            rglLightingAdjust(0.0f);
        else
            meshSetFade(0, 0.0f);
    }
    else if (distsqr < mindist+fadedist)
    {
        real32 amount = (distsqr - mindist) / fadedist;
        if (RGL && !usingShader)
            rglLightingAdjust(amount);
        else
            meshSetFade(0, amount);
        rndAdditiveBlends(FALSE);
    }
    else
    {
        if (RGL && !usingShader)
            rglLightingAdjust(1.0f);
        else
            meshSetFade(0, 1.0f);
        return TRUE;
    }

    return FALSE;
}

void rndUnFade()
{
    if (RGL && !usingShader)
        rglLightingAdjust(0.0f);
    else
        meshSetFade(0, 0.0f);
}

/*-----------------------------------------------------------------------------
    Name        : rndRenderAHomeworld
    Description : Special 'world-render' code for drawing planets
    Inputs      : camera - what camera we're rendering from
                  world - what planet we're drawing
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define camera ((Camera *)voidCamera)
#define world  ((Derelict *)voidWorld)
void rndRenderAHomeworld(void* voidCamera, void *voidWorld)
{
    real32 planetScale;
    vector *position;
    meshdata *worldMesh;
    hmatrix coordMatrixForGL;
    sdword colorScheme = world->colorScheme;
    sdword depthTesting;

    planetScale = world->staticinfo->scaleFactor;
    position = &world->posinfo.position;
    worldMesh = (meshdata *)world->staticinfo->staticheader.LOD->level[0].pData;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(btgFieldOfView, rndAspectRatio, rndNear(camera->clipPlaneNear), CAMERA_CLIP_FAR_PLANET);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    hmatMakeHMatFromMat(&coordMatrixForGL,&world->rotinfo.coordsys);
    hmatPutVectIntoHMatrixCol4(*position, coordMatrixForGL);
    glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
    glScalef(planetScale, planetScale, planetScale);

    rndNormalizeEnable(TRUE);
    depthTesting = rndSetDepthTesting(FALSE);
    meshObjectRender(&worldMesh->object[0], worldMesh->localMaterial, colorScheme);
    rndSetDepthTesting(depthTesting);
    rndNormalizeEnable(FALSE);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)&rndProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
}
#undef world
#undef camera

/*-----------------------------------------------------------------------------
    Name        : rndRenderAWorldEffect
    Description : Render an effect in the manner of a homeworld.
    Inputs      : camera - camera we're looking at it from
                  effect - effect we are to render
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rndRenderAWorldEffect(Camera *camera, Effect *effect)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(btgFieldOfView, rndAspectRatio, rndNear(camera->clipPlaneNear), CAMERA_CLIP_FAR_PLANET);

    glMatrixMode(GL_MODELVIEW);

    rndNormalizeEnable(TRUE);
    glDisable(GL_DEPTH_TEST);

    etgEffectDraw(effect);

    glEnable(GL_DEPTH_TEST);
    rndNormalizeEnable(FALSE);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)&rndProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
}

static real32 lastTime0 = 0.0f;
static real32 lastTime1 = 0.0f;

void rndResetFloatTime(void)
{
    lastTime0 = taskTimeElapsed;
    lastTime1 = taskTimeElapsed;
}

/*-----------------------------------------------------------------------------
    Name        : rndCameraOffset0
    Description : camera offset fn 0
    Inputs      :
    Outputs     :
    Return      : a value to be added to a camera coordinate
----------------------------------------------------------------------------*/
real32 rndCameraOffset0(void)
{
    real32 et;
    static real32 t = 0.0f;

    t += taskTimeElapsed - lastTime0;
    lastTime0 = taskTimeElapsed;

    et = t * CAMERA_FLOAT_SCALAR0;

    return (real32)(sin((real64)et) - cos(0.5f * (real64)et));
}

/*-----------------------------------------------------------------------------
    Name        : rndCameraOffset1
    Description : camera offset fn 1
    Inputs      :
    Outputs     :
    Return      : a value to be added to a camera coordinate
----------------------------------------------------------------------------*/
real32 rndCameraOffset1(void)
{
    real32 et;
    static real32 t = 0.0f;

    t += taskTimeElapsed - lastTime1;
    lastTime1 = taskTimeElapsed;

    et = t * CAMERA_FLOAT_SCALAR1;

    return (real32)(sin((real64)et) + cos(0.5f * (real64)et));
}

/*-----------------------------------------------------------------------------
    Name        : rndScaleCameraOffsets
    Description : apply scaling to the camera float offsets to fudge a slightly
                  more linear falloff with distance
    Inputs      : camera - the camera
                  offset - offsets, 2 of 'em
    Outputs     : scaledOffset - scaled versions of the offsets
    Return      :
----------------------------------------------------------------------------*/
void rndScaleCameraOffsets(real32 *scaledOffset, Camera* camera, real32* offset)
{
    real32 dist;

    dist = camera->distance * CAMERA_FLOAT_DIST_SCALAR;
    if (dist > 1.0f)
    {
        scaledOffset[0] = offset[0] * dist;
        scaledOffset[1] = offset[1] * dist;
        scaledOffset[2] = offset[2] * dist;
    }
    else
    {
        scaledOffset[0] = offset[0];
        scaledOffset[1] = offset[1];
        scaledOffset[2] = offset[2];
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndDockScalar
    Description : returns a value [0..1] indicating the amount of docking light
                  being received
    Inputs      : ship - ship to light
                  dockship - the ship that 'ship' is docking with
                  nearVal, farVal - range of light
    Outputs     :
    Return      : scalar [0..1] indicating amount of light being received
----------------------------------------------------------------------------*/
real32 rndDockScalar(Ship* ship, Ship* dockship, real32 nearVal, real32 farVal)
{
    vector veclen;
    real32 dist;

    vecSub(veclen, dockship->posinfo.position, ship->posinfo.position);
    dist = fsqrt(vecMagnitudeSquared(veclen));
    if (dist < nearVal) dist = nearVal;
    if (dist > farVal)  dist = farVal;
    dist -= nearVal;
    dist /= (farVal - nearVal);
    return 1.0f - dist;
}

/*-----------------------------------------------------------------------------
    Name        : rndDrawAsteroid0
    Description : renders asteroid0's (small asteroids that provide visible
                  structure to a level)
    Inputs      : n - number of points to render
    Outputs     : displays the asteroid0Data list
    Return      :
----------------------------------------------------------------------------*/
void rndDrawAsteroid0(sdword n)
{
    sdword  index;
    color   c;
    vector* v;

    glPointSize(glCapFeatureExists(GL_POINT_SIZE) ? 2.0f : 1.0f);
    glBegin(GL_POINTS);
    for (index = 0; index < n; index++)
    {
        c = asteroid0Data[index].c;
        v = asteroid0Data[index].position;
        glColor3ub(colRed(c), colGreen(c), colBlue(c));
        glVertex3fv((GLfloat*)v);
    }
    glEnd();
    glPointSize(1.0f);
}

/*-----------------------------------------------------------------------------
    Name        : rndPreRenderDebugStuff
    Description : A bunch of debug stuff from the beginning of the render
                    function.
    Inputs      : camera - same as parameter to rndMainViewRender
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rndPreRenderDebugStuff(Camera *camera)
{
#if GUN_TUNE_MODE
    if (gunTuningMode && (selSelected.numShips == 1))
    {
        if (keyIsStuck(INSERTKEY))
        {
            tuningGun++;

            keyClearSticky(INSERTKEY);
        }
    }
#endif

#if LOD_PRINT_DISTANCE
    if (keyIsStuck(INSERTKEY))
    {
        if (rndLOD < 10)
        {
            rndLOD++;
            dbgMessagef("\nLevel of detail = %d", rndLOD);
        }
        keyClearSticky(INSERTKEY);
    }
    if (keyIsStuck(HOMEKEY))
    {
        if (rndLOD > 0)
        {
            rndLOD--;
            dbgMessagef("\nLevel of detail = %d", rndLOD);
        }
        keyClearSticky(HOMEKEY);
    }
    if (keyIsStuck(DELETEKEY))
    {
        lodDrawingMode ^= TRUE;
        if (selSelected.numShips == 1)
        {
            vector length;

            length.x = camera->eyeposition.x - selSelected.ShipPtr[0]->posinfo.position.x;
            length.y = camera->eyeposition.y - selSelected.ShipPtr[0]->posinfo.position.y;
            length.z = camera->eyeposition.z - selSelected.ShipPtr[0]->posinfo.position.z;
            dbgMessagef("\nCamera distance = %f", vecMagnitudeSquared(length));
        }
        else
        {
            dbgMessagef("\nPlease select one and only one object for LOD tuning.");
        }
        keyClearSticky(DELETEKEY);
    }
    if (keyIsStuck(ENDKEY))
    {
        lodTuningMode ^= TRUE;
        dbgMessagef("\nLOD tuning %s", lodTuningMode ? "ON" : "OFF");
        keyClearSticky(ENDKEY);
    }
#endif
#if RND_XYZ
    if (keyIsStuck(RND_XYZKey))
    {
        keyClearSticky(RND_XYZKey);
        rndXYZPrint ^= TRUE;
    }
#endif //RND_XYZ
}

/*-----------------------------------------------------------------------------
    Name        : rndPostRender3DStuff
    Description : Post-object rendering of 3D debugging stuff
    Inputs      : camera - same as parameter to rndMainViewRender
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndPostRenderDebug3DStuff(Camera *camera)
{
#if NIS_DRAW_CAMERA
    if (nisIsRunning && nisCamera != NULL && nisCamera != camera)
    {                                                       //if we should draw the NIS camera
        vector upVector = nisCamera->eyeposition;
        vector cameraVector;
        static bool32 flashFlag;

        rndLightingEnable(FALSE);
        vecSub(cameraVector, nisCamera->lookatpoint, nisCamera->eyeposition);
        vecMultiplyByScalar(cameraVector, 200);
        vecAdd(cameraVector, cameraVector, nisCamera->lookatpoint);
        primLine3(&nisCamera->eyeposition, &cameraVector, flashFlag ? colWhite : colFuscia);
        vecScalarMultiply(upVector, nisCamera->upvector, 200);
        vecAdd(upVector, nisCamera->eyeposition, upVector);
//        upVector.z -= abs(nisCamera->eyeposition.z - cameraVector.z) +
//            abs(nisCamera->eyeposition.y - cameraVector.y);
        primLine3(&nisCamera->eyeposition, &upVector, colWhite);
//      if (selSelected.numShips > 0)
//      {
//          primLine3(&nisCamera->eyeposition, &selSelected.ShipPtr[0]->posinfo.position, flashFlag ? colWhite : colFuscia);
//      }
        flashFlag ^= TRUE;
    }
#endif
}


#if FONT_CHECKSPECIAL
fonthandle testing=1;
extern fontregistry frFontRegistry[FR_NumberFonts];
#endif

/*-----------------------------------------------------------------------------
    Name        : rndPostRenderDebug2DStuff
    Description : Post-object rendering of 2D debug stuff
    Inputs      : camera - same as parameter to rndMainViewRender
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndPostRenderDebug2DStuff(Camera *camera)
{
#if LOD_PRINT_DISTANCE
    if (lodDrawingMode && selSelected.numShips == 1)
    {
        vector length;
        vector shippos = selSelected.ShipPtr[0]->collInfo.collPosition;
        sdword nCurrentLOD;

        if (!lodTuningMode)
        {
            nCurrentLOD = selSelected.ShipPtr[0]->currentLOD;
        }
        else
        {
            nCurrentLOD = rndLOD;
        }

        vecSub(length,camera->eyeposition,shippos);

        fontPrintf(0, MAIN_WindowHeight - fontHeight(" ") - 1, colWhite,
                   "Current LOD = %d, distance = %f, x = %.1f y = %.1f z = %.1f",
                   nCurrentLOD, vecMagnitudeSquared(length),shippos.x,shippos.y,shippos.z);
    }
#endif
#if GUN_TUNE_MODE
    if (gunTuningMode && (selSelected.numShips == 1))
    {
        gunTuneGun(selSelected.ShipPtr[0]);
    }
#endif

#if RND_SCALECAP_TWEAK
#ifndef khentschel
    fontPrint(0, 0, colWhite, scaleCapString);
#endif
#endif

#if SHOW_TRAIL_STATS
    {
        char buf[256];
        static lastUpdated;
        static lastNotUpdated;

        if (trailsUpdated == 0)
        {
            sprintf(buf, "ren %3d up %3d not %3d",
                    trailsRendered, lastUpdated, lastNotUpdated);
        }
        else
        {
            sprintf(buf, "ren %3d up %3d not %3d",
                    trailsRendered, trailsUpdated, trailsNotUpdated);
            lastUpdated = trailsUpdated;
            lastNotUpdated = trailsNotUpdated;
        }

        fontPrint(0, 32, colWhite, buf);
        trailsUpdated = trailsNotUpdated = 0;

        {
            extern sdword bTrailRender, bTrailDraw;

            sprintf(buf, "render %d draw %d", bTrailRender, bTrailDraw);
            fontPrint(0, 48, colWhite, buf);

            if (keyIsHit(SHIFTKEY))
            {
                if (keyIsHit(NINEKEY)) bTrailRender = 1;
                if (keyIsHit(ZEROKEY)) bTrailDraw = 1;
            }
            else
            {
                if (keyIsHit(NINEKEY)) bTrailRender = 0;
                if (keyIsHit(ZEROKEY)) bTrailDraw = 0;
            }
        }
    }
#endif

#if USE_RND_HINT
    {
        char buf[32];
        switch (rndHint)
        {
        case 0:
            sprintf(buf, "hint [default]");
            break;
        case 1:
            sprintf(buf, "hint [on]");
            break;
        case 2:
            sprintf(buf, "hint [off]");
            break;
        default:
            sprintf(buf, "hint [?]");
        }
        fontPrint(MAIN_WindowWidth - fontWidth(buf), fontHeight(buf), colWhite, buf);
    }
#endif

#if RND_FOV_TWEAK
    if (keyIsHit(NINEKEY))
    {
        btgFieldOfView -= btgFieldOfViewDelta;
        sprintf(fovString, "fov = %f", btgFieldOfView);
    }
    if (keyIsHit(ZEROKEY))
    {
        btgFieldOfView += btgFieldOfViewDelta;
        sprintf(fovString, "fov = %f", btgFieldOfView);
    }
    fontPrint(MAIN_WindowWidth - fontWidth(fovString), 0, colWhite, fovString);
#endif //RND_FOV_TWEAK

#if RND_SCALECAP_TWEAK
    if (keyIsHit(RND_CapScaleCapKey))
    {
        if (keyIsHit(CONTROLKEY))
        {
            RND_CAPSCALECAP_STATS = TRUE;
        }
        else
        {
            RND_CAPSCALECAP_STATS = FALSE;
        }
    }
    if (RND_CAPSCALECAP_STATS)
    {
        rndCapScaleCapStatsTaskFunction();
    }
#endif //RND_SCALECAP_TWEAK
#if RND_DOCKLIGHT_TWEAK
    if (selSelected.numShips == 1)
    {
        Ship* ship = selSelected.ShipPtr[0];
        if (ship->dockvars.reserveddocking != -1 &&
            ship->dockvars.dockship != NULL)
        {
            vector veclen;
            real32 dist;
            Ship* dockship;
            char string[64];

            dockship = ship->dockvars.dockship;
            vecSub(veclen, ship->posinfo.position, dockship->posinfo.position);
            dist = fsqrt(vecMagnitudeSquared(veclen));
            sprintf(string, "%0.2f m", dist);
            fontPrint(0, 0, colWhite, string);
        }
    }
#endif

#if RND_FRAME_RATE
    if (rndDisplayFrameRate)
    {
        char   string[256];
#if VERBOSE_SHIP_STATS
        sdword i;
#endif
        extern sdword trTextureChanges, trAvoidedChanges;

        if (RGL)
        {
            sprintf(string, "(%d . %d) (%u) %u . %u . %u",
                    trTextureChanges, trAvoidedChanges,
                    alodGetPolys(), rglNumPolys(),
                    rglCulledPolys(), meshRenders);
        }
        else
        {
            sprintf(string, "(%d . %d) (%u) . %u",
                    trTextureChanges, trAvoidedChanges,
                    alodGetPolys(), meshRenders);
        }

        trTextureChanges = 0;
        trAvoidedChanges = 0;

        meshRenders = 0;
        fontPrint(MAIN_WindowWidth - fontWidth(string), 0, colWhite, string);
#if VERBOSE_SHIP_STATS
        for (i = 0; i < 5; i++)
        {
            sprintf(string, "%d: %d / %d", i, shipsAtLOD[i], polysAtLOD[i]);
            shipsAtLOD[i] = 0;
            polysAtLOD[i] = 0;
            fontPrint(MAIN_WindowWidth - fontWidth(string), (i+1)*fontHeight(string), colWhite, string);
        }
#endif
#if DISPLAY_LOD_SCALE
        sprintf(string, "scale %f", lodScaleFactor);
        fontPrintf(0, fontHeight(string), colWhite, string);
#endif
        if (alodGetPanic())
        {
            sprintf(string, "PANIC!");
            fontPrint((MAIN_WindowWidth - fontWidth(string)) / 2, 0, colRGB(255,0,0), string);
        }

        if (universe.mainCameraCommand.currentCameraStack->focus.numShips == 1)
        {
            Ship* ship = universe.mainCameraCommand.currentCameraStack->focus.ShipPtr[0];
            vector veclen;
            real32 dist;

//            vecSub(veclen, ship->posinfo.position, camera->eyeposition);
            vecSub(veclen, ship->collInfo.collPosition, camera->eyeposition);
            dist = fsqrt(vecMagnitudeSquared(veclen));
            sprintf(string, "dist %0.2f m", dist);
            fontPrint((MAIN_WindowWidth - fontWidth(string)) / 2,
                      fontHeight(" "),
                      colWhite, string);
        }
    }
#endif
#if MEM_STATISTICS
        if (memStatsLogging)
        {
            sdword y = 0, x = 0, maxWidth = 0, width, index;
            fontPrint(0, y, colWhite, memStatString);
            for (index = 0; index < MS_NumberCookieNames; index++)
            {
                if (memStatsCookieNames[index].outputString[0] == 0)
                {
                    continue;
                }
                y += fontHeight(" ") + 1;
                width = fontWidth(memStatsCookieNames[index].outputString);
                maxWidth = max(maxWidth, width);
                if (y >= MAIN_WindowHeight)
                {
                    y = fontHeight(" ") + 1;
                    x += maxWidth + 1;
                    maxWidth = 0;
                    if (x > MAIN_WindowWidth)
                    {
                        break;
                    }
                }
                fontPrint(x, y, colWhite, memStatsCookieNames[index].outputString);
            }
        }
#endif
#if DEM_CHECKSUM
    if (demChecksumError)
    {
        fontPrint(2, MAIN_WindowHeight - 60, colWhite, demChecksumString);
    }
#endif

#if FONT_CHECKSPECIAL
    fontMakeCurrent(testing);
    fontPrint(10,50,colWhite,"����������������������������������������������������������������");

    if ((keyIsStuck(CONTROLKEY)) && (keyIsHit(SHIFTKEY)))
    {
        testing--;
        if (testing < 1) testing = 1;
        keyClearSticky(CONTROLKEY);
    }
    else if (keyIsStuck(CONTROLKEY))
    {
        testing++;
        if (frFontRegistry[testing].name == NULL)
        {
            testing--;
        }
        keyClearSticky(CONTROLKEY);
    }

#endif

    trkTrackValuesDisplay();
}

/*-----------------------------------------------------------------------------
    Name        : rndMainViewRenderNothingFunction
    Description : Don't do anything; just return.  For when something else is
                    already rendering the screen.
    Inputs      : camera - it's not even used
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rndMainViewRenderNothingFunction(Camera *camera)
{
    return;
}

/*-----------------------------------------------------------------------------
    Name        : rndMainViewAllButRenderFunction
    Description : Compute selection info for space obj's but don't render them
    Inputs      :
    Outputs     :
    Return      :
    Note        : Because this function mirrors much of the functionality of
                    rndMainViewRenderFunction, any drastic changes to
                    rndMainViewRenderFunction will have to be made this function.
----------------------------------------------------------------------------*/
void rndMainViewAllButRenderFunction(Camera *camera)
{
    ;
}

static void rndRenderSpaceObj(Camera *camera, SpaceObj *spaceobj, bool32 twopass)
{
    udword i;
    vector to;
    lod *level;
    ShipStaticInfo *shipStaticInfo;
    sdword playerIndex;
    hmatrix coordMatrixForGL;
    Effect *effect;
    sdword colorScheme;
    bool32 displayEffect;

    switch (spaceobj->objtype)
    {
        case OBJ_BulletType:                            //type bullet
            if (((Bullet *)spaceobj)->effect == NULL)
            {                                           //if there is no effect for this bullet
                Bullet *bullet = (Bullet *)spaceobj;

                rndLightingEnable(FALSE);               //draw default line
                rndTextureEnable(FALSE);
                if((WeaponType)bullet->bulletType == WEAPON_Laser)
                {   //bullet update 'feature' to draw laser to bullet
                    defenseFighterAdjustLaser(bullet);
                }
                else if ((WeaponType)bullet->bulletType == WEAPON_Ion)
                {
                    if(bullet->timelived <= UNIVERSE_UPDATE_PERIOD)
                    {
                        goto dontdraw;
                    }
                }
                else if ((WeaponType)bullet->bulletType == WEAPON_BeastInfection)
                {
                    goto dontdraw;
                }
                vecAdd(to, spaceobj->posinfo.position, bullet->lengthvec);
                primLine3(&spaceobj->posinfo.position, &to, bullet->bulletColor);
dontdraw:
                rndLightingEnable(TRUE);
            }
            break;
        case OBJ_EffectType:                            //if type effect
            if (cataTransparentQueueMode != 2)
            {
                cataTransparentQueueAppend(spaceobj);
            }
            else
            {
                effect = (Effect *)spaceobj;                    //get effect pointer
                if(effect->owner != NULL)
                {
                   SpaceObj *effectowner = (SpaceObj *)effect->owner;
                   if(effectowner->objtype == OBJ_BulletType)
                   {
                     //effect is owned by a bullet
                     if((WeaponType)((Bullet *) effectowner)->bulletType == WEAPON_Laser)
                     {
                        //effect is owned by a defense fighter laser beam
                        defenseFighterAdjustLaser(((Bullet *) effectowner));
                        etgEffectUpdate(effect, 0.0f);                        //luke suggested fix to laser length problem
                     }
                     else if((WeaponType)((Bullet *) effectowner)->bulletType == WEAPON_BeastInfection)
                     {
                        etgEffectUpdate(effect, 0.0f);
                     }
                     else if((WeaponType)((Bullet *) effectowner)->bulletType == WEAPON_Ion)
                     {
                         if(((Bullet *)effectowner)->timelived <= UNIVERSE_UPDATE_PERIOD)
                         {
                            goto dontdraw2;
                         }
                     }
                   }
                }
                etgEffectDraw(effect);
dontdraw2:;
            }
            break;
        case OBJ_ShipType:
            cataSetShipRenderState((Ship *)spaceobj);
            if (cataTransparentQueueMode != 2)
            {
                bool32 renderImmediately;

                if (cataShipQueuesForTransparentPass((Ship *)spaceobj, &renderImmediately))
                {
                    cataTransparentQueueAppend(spaceobj);
                    if (!renderImmediately)
                    {
                        break;
                    }
                }
            }
            if (spaceobj->staticinfo->staticheader.LOD != NULL)
            {
                if (!univIsShipCloaked(spaceobj) ||
                    ((Ship *)spaceobj)->playerowner == universe.curPlayerPtr ||
                    shipGetSlotStatus((Ship *)spaceobj, universe.curPlayerPtr->playerIndex))
                {
                    //put the ship into the mission map array
//                        mmAddShipToMapArray((ShipPtr)spaceobj);
                    shipStaticInfo = (ShipStaticInfo *)spaceobj->staticinfo;
                    playerIndex = (sdword)((Ship *)spaceobj)->playerowner->playerIndex;
                    dbgAssert((spaceobj->flags & SOF_Rotatable) != 0);
                    selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);//compute selection circle

                    if (((Ship *)spaceobj)->shiptype == sACV || ((Ship *)spaceobj)->shiptype == bACV)
                    {
                        rndRenderEMP((Ship *)spaceobj);
                    }
                    if (ShipTypeIsProcessorType(((Ship *)spaceobj)->shiptype))
                    {
                        nebRenderShipTendrils((Ship *)spaceobj);
                    }
                    dreadnoughtRenderCannonTimerLine((Ship *)spaceobj);

#if RND_VISUALIZATION
    #ifdef DEBUG_COLLISIONS
                    collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
    #else
                    if (RENDER_BOXES)
                    {
                        collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
                    }
    #endif
#endif

                    glPushMatrix();
                    level = lodLevelGet((void *)spaceobj, &camera->eyeposition, &((SpaceObjRotImp *)spaceobj)->collInfo.collPosition);

                    if (taskTimeElapsed-((Ship *)spaceobj)->flashtimer < FLASH_TIMER)
                    {
                        rndDamageFlashBiasActive = TRUE;
                        if (RGL)
                        {
                            glPixelTransferf(GL_RED_BIAS, 0.3f);
                            glPixelTransferf(GL_GREEN_BIAS, 0.3f);
                            glPixelTransferf(GL_BLUE_BIAS, 0.3f);
                        }
                    }

                    switch (level->flags & LM_LODType)
                    {
                        case LT_Invalid:
                            break;
                        case LT_Mesh:
                            displayEffect = hsShouldDisplayEffect((Ship*)spaceobj);
#if LIGHT_PLAYER_COLORS
                            lightPlayerColorLightSet(playerIndex);
#endif //LIGHT_PLAYER_COLORS
                            hmatMakeHMatFromMat(&coordMatrixForGL,&((SpaceObjRot *)spaceobj)->rotinfo.coordsys);
                            hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position,coordMatrixForGL);
                            glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
                            rndApplyComponentSpinTransform(spaceobj);
                            shPushLightMatrix(&coordMatrixForGL);
                            if (selCameraSpace.z < 0.0f)
                            {
                                //((Ship *)spaceobj)->magnitudeSquared = 1.0f - selCameraSpace.z * shipStaticInfo->NLIPS;
#if RND_SCALECAP_TWEAK
                                if (selSelected.numShips == 1 &&
                                    selSelected.ShipPtr[0]->staticinfo == shipStaticInfo)
                                {
                                    if (keyIsHit(ONEKEY))
                                    {
                                        shipStaticInfo->scaleCap -= scaleCapSlopeDelta;
                                        dbgMessagef(scaleCapString, "\nN-LIPS = %f", shipStaticInfo->scaleCap);
                                        sprintf(scaleCapString, "N-LIPS = %f", shipStaticInfo->scaleCap);
                                    }
                                    if (keyIsHit(TWOKEY))
                                    {
                                        shipStaticInfo->scaleCap += scaleCapSlopeDelta;
                                        dbgMessagef(scaleCapString, "\nN-LIPS = %f", shipStaticInfo->scaleCap);
                                        sprintf(scaleCapString, "N-LIPS = %f", shipStaticInfo->scaleCap);
                                    }
                                }
#endif //RND_SCALECAP_TWEAK

                                if (shipStaticInfo->scaleCap != 0.0f)
                                {                   //if there's a scaling cap
                                    real32 scaleFactor;
                                    if (glCapFeatureExists(GL_RESCALE_NORMAL))
                                    {
                                        glEnable(GL_RESCALE_NORMAL);
                                        rndNormalization = TRUE;
                                    }
                                    else
                                    {
                                        rndNormalizeEnable(TRUE);
                                    }
                                    scaleFactor = 1.0f - selCameraSpace.z * shipStaticInfo->scaleCap;
#if RND_VERBOSE_LEVEL >= 1
                                    if (_isnan((double)scaleFactor))
                                    {
                                        dbgMessage("\n-- scaleFactor is NaN --");
                                    }
#endif
                                    ((Ship *)spaceobj)->magnitudeSquared = scaleFactor;
                                    glScalef(scaleFactor, scaleFactor, scaleFactor);
                                    ((Ship *)spaceobj)->collInfo.selCircleRadius *= scaleFactor;
                                }
                                else
                                {
                                    ((Ship *)spaceobj)->magnitudeSquared = 1.0f;
                                }
                            }
                            else                        //don't even bother rendering if behind camera
                            {
                                ((Ship *)spaceobj)->magnitudeSquared = 1.0f;
//!!!                                    break;
                            }
#if SO_CLOOGE_SCALE
                            if (shipStaticInfo->scaleFactor != 1.0f)
                            {
                                glScalef(shipStaticInfo->scaleFactor,
                                         shipStaticInfo->scaleFactor,
                                         shipStaticInfo->scaleFactor);
                            }
#endif
#if RND_VISUALIZATION
                            if (dockLines) dockDrawDockInfo((Ship *)spaceobj);
                            if (gunLines) gunDrawGunInfo((Ship *)spaceobj);
                            if (dockLines) dockDrawSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)spaceobj);
#endif

 /*
                            switch (shipStaticInfo->shiprace)
                            {
                                case R1:
                                case R2:
                                    i = ((Ship *)spaceobj)->playerowner->playerIndex;
                                    dbgAssert(i >= 0 && i < 8);//!!!
                                    break;
                                default:
                                    i = 0;
                                    break;
                            }
*/
                            i = ((Ship *)spaceobj)->colorScheme;
                            dbgAssert(i >= 0 && i < TE_NumberPlayers);

                            if (rndShipVisible(spaceobj, camera))
                            {
                                bool32 result = rndFade(spaceobj, camera);
                                Ship* ship = (Ship*)spaceobj;
                                ShipSinglePlayerGameInfo* ssinfo = ship->shipSinglePlayerGameInfo;
                                real32 cloakTransitionBlend = 0.0f;
                                bool32 shipCloakTransitionActive = shipGetCloakTransitionBlend(ship, &cloakTransitionBlend);
                                bool32 cloakWireframeFadePass = shipCloakTransitionActive && cloakTransitionBlend > 0.1f;
                                bool32 crystalProcessingCullFaceEnabled = FALSE;
                                bool32 crystalProcessingClipActive = FALSE;

                                if (ssinfo != NULL)
                                {
                                    if (ssinfo->shipHyperspaceState == SHIPHYPERSPACE_NONE)
                                    {
                                        ssinfo = NULL;
                                    }
                                }

                                if (!result)
                                {
                                    if (spaceobj->currentLOD == 0)
                                    {
                                        rndPerspectiveCorrection(TRUE);
                                    }

                                    if (shipCloakTransitionActive)
                                    {
                                        meshSetFade(5, cloakTransitionBlend);
                                    }
                                    else
                                    {
                                        meshSetFade(5, 0.0f);
                                        if (univIsShipCloaked(spaceobj) &&
                                            ship->playerowner == universe.curPlayerPtr)
                                        {
                                            g_WireframeHack = TRUE;
                                        }
                                    }

                                    meshRenders++;

                                    if (rndInsideShip(spaceobj, camera))
                                    {
                                        g_WireframeHack = TRUE;
                                        g_HiddenRemoval = FALSE;
                                    }

                                    if (ssinfo != NULL &&
                                        ssinfo->hsState != HS_INACTIVE &&
                                        ssinfo->hsState != HS_FINISHED)
                                    {
                                        if (ssinfo->staticHyperspaceGate)
                                        {
                                            hsNoGate(TRUE);
                                        }
                                        hsContinue((Ship*)spaceobj, displayEffect);
                                        if (ssinfo->staticHyperspaceGate)
                                        {
                                            hsNoGate(FALSE);
                                        }
                                    }

                                    crystalProcessingClipActive =
                                        cataSetCrystalProcessingPassState(ship, &crystalProcessingCullFaceEnabled);

                                    //
                                    // lighting while docking
                                    //
                                    if (((Ship *)spaceobj)->dockvars.dockPointIndex != -1 &&
                                        ((Ship*)spaceobj)->dockvars.dockship != NULL)
                                    {
                                        Ship* dockship;
                                        real32 t, nearVal, farVal;

                                        dockship = ((Ship*)spaceobj)->dockvars.dockship;
                                        if (cataDockLightApplies(dockship))
                                        {
                                            nearVal = dockship->staticinfo->dockLightNear;
                                            farVal  = dockship->staticinfo->dockLightFar;
                                            t = rndDockScalar((Ship*)spaceobj,
                                                              ((Ship*)spaceobj)->dockvars.dockship,
                                                              nearVal, farVal);
                                            shDockLightColor(dockship->staticinfo->dockLightColor);
                                            shDockLight(t);
                                        }
                                    }

                                    if ((ssinfo != NULL &&
                                         (ssinfo->hsState != HS_FINISHED || singlePlayerGameInfo.hyperspaceFails)) ||
                                        ssinfo == NULL)
                                    {
                                        extern sdword visiblePoly;
                                        extern bool32 g_SpecificPoly;
                                        extern bool32 g_Points;
                                        sdword renderPass;
                                        bool32 beastOverlayActive = cataRenderScratch.beastOverlay.active;
                                        sdword renderPasses =
                                           (cataShipUsesExternalBuildClipPlane((Ship *)spaceobj) || beastOverlayActive) ? 2 : 1;

                                        for (renderPass = 0; renderPass < renderPasses; renderPass++)
                                        {
                                           sdword cloakRenderPass;
                                           sdword cloakRenderPasses = cloakWireframeFadePass ? 2 : 1;
                                           bool32 renderReplacementState = cataRenderScratch.replacement.active &&
                                                                          cataRenderScratch.replacement.replacementLodInfo != NULL;
                                           bool32 renderModuleSlotState = cataRenderScratch.moduleSlot.active &&
                                                                       cataRenderScratch.moduleSlot.buildTargetLodInfo != NULL;
                                           bool32 renderBeastOverlayPass = beastOverlayActive && renderPass == 1;

                                           cataSetExternalBuildPassState((Ship *)spaceobj, renderPass != 0);
                                           for (cloakRenderPass = 0; cloakRenderPass < cloakRenderPasses; cloakRenderPass++)
                                           {
                                               if (shipCloakTransitionActive && cloakRenderPass == 0)
                                               {
                                                   meshSetFade(5, cloakTransitionBlend);
                                               }
                                               else
                                               {
                                                   meshSetFade(5, 0.0f);
                                                   if (cloakRenderPass != 0)
                                                   {
                                                       g_WireframeHack = TRUE;
                                                   }
                                                   else if (univIsShipCloaked(spaceobj) &&
                                                            ship->playerowner == universe.curPlayerPtr)
                                                   {
                                                       g_WireframeHack = TRUE;
                                                   }
                                               }

                                               if (renderBeastOverlayPass)
                                               {
                                                   cataRenderBeastOverlay((Ship *)spaceobj);
                                               }
                                               else if (renderReplacementState)
                                               {
                                                   Ship *ship = (Ship *)spaceobj;
                                                   sdword targetLOD = ship->currentLOD;
                                                   lodinfo *targetLodInfo = cataRenderScratch.replacement.replacementLodInfo;
                                                   shipbindings *targetBindings = cataRenderScratch.replacement.replacementBindings;
                                                   ShipRace targetRace = cataRenderScratch.replacement.replacementRace;
                                                   real32 morphProgress = cataRenderScratch.replacement.progress;
                                                   ubyte replacementColorScheme = cataRenderScratch.replacement.replacementColorScheme;
                                                   lod *targetLevel;
                                                   meshdata *targetMesh;
                                                   ubyte *effectState = cataRenderScratch.effectState;

                                                   dbgAssert(targetLodInfo->nLevels > 0);
                                                   if (targetLOD >= targetLodInfo->nLevels)
                                                   {
                                                       targetLOD = targetLodInfo->nLevels - 1;
                                                   }

                                                   targetLevel = &targetLodInfo->level[targetLOD];
                                                   if (targetLevel->pData == NULL)
                                                   {
                                                       dbgAssert(targetLOD > 0);
                                                       targetLOD--;
                                                       targetLevel = &targetLodInfo->level[targetLOD];
                                                   }

                                                   dbgAssert((targetLevel->flags & LM_LODType) == LT_Mesh);
                                                   targetMesh = (meshdata *)targetLevel->pData;

                                                   if (morphProgress < 1.0f)
                                                   {
                                                       meshSetFade(0, morphProgress);
                                                       if (ship->bindings != NULL)
                                                       {
                                                           meshRenderShipHierarchyForShip(
                                                               ship,
                                                               ship->bindings,
                                                               ship->currentLOD,
                                                               (meshdata *)level->pData,
                                                               i);
                                                       }
                                                       else
                                                       {
                                                           meshRender((meshdata *)level->pData, i);
                                                       }
                                                   }

                                                   meshSetFade(0, 1.0f - morphProgress);
                                                   if (targetRace == RACE_Beast)
                                                   {
                                                       effectState[0] = 1;
                                                   }

                                                   if (targetBindings != NULL)
                                                   {
                                                       meshRenderShipHierarchyForShip(
                                                           ship,
                                                           targetBindings,
                                                           targetLOD,
                                                           targetMesh,
                                                           replacementColorScheme);
                                                   }
                                                   else
                                                   {
                                                       meshRender(targetMesh, replacementColorScheme);
                                                   }

                                                   meshSetFade(0, 0.0f);
                                                   effectState[0] = 0;
                                               }
                                               else if (renderModuleSlotState)
                                               {
                                                   Ship *ship = (Ship *)spaceobj;
                                                   sdword targetLOD = ship->currentLOD;
                                                   lodinfo *targetLodInfo = cataRenderScratch.moduleSlot.buildTargetLodInfo;
                                                   shipbindings *targetBindings = cataRenderScratch.moduleSlot.buildTargetBindings;
                                                   sdword targetRace = cataRenderScratch.moduleSlot.buildTargetRace;
                                                   sdword targetColorScheme = cataRenderScratch.moduleSlot.colorScheme;
                                                   real32 morphProgress = cataRenderScratch.moduleSlot.morphProgress;
                                                   real32 displaySphereRadius = cataRenderScratch.moduleSlot.displaySphereRadius;
                                                   bool32 activeModuleSlotState = cataRenderScratch.moduleSlot.targetStateActive;
                                                   lod *targetLevel;
                                                   meshdata *targetMesh;
                                                   ubyte *effectState = cataRenderScratch.effectState;

                                                   dbgAssert(targetLodInfo->nLevels > 0);
                                                   if (targetLOD >= targetLodInfo->nLevels)
                                                   {
                                                       targetLOD = targetLodInfo->nLevels - 1;
                                                   }

                                                   targetLevel = &targetLodInfo->level[targetLOD];
                                                   if (targetLevel->pData == NULL)
                                                   {
                                                       dbgAssert(targetLOD > 0);
                                                       targetLOD--;
                                                       targetLevel = &targetLodInfo->level[targetLOD];
                                                   }

                                                   dbgAssert((targetLevel->flags & LM_LODType) == LT_Mesh);
                                                   targetMesh = (meshdata *)targetLevel->pData;

                                                   if (activeModuleSlotState)
                                                   {
                                                       if (targetRace == 1)
                                                       {
                                                           effectState[0] = 1;
                                                       }

                                                       if (targetBindings != NULL)
                                                       {
                                                           meshRenderShipHierarchyForShip(
                                                               ship,
                                                               targetBindings,
                                                               targetLOD,
                                                               targetMesh,
                                                               targetColorScheme);
                                                       }
                                                       else
                                                       {
                                                           meshRender(targetMesh, targetColorScheme);
                                                       }

                                                       effectState[0] = 0;
                                                   }
                                                   else
                                                   {
                                                       if (morphProgress > CATA_MODULE_SLOT_SOURCE_DRAW_THRESHOLD)
                                                       {
                                                           meshSetFade(0, 1.0f - morphProgress);
                                                           if (ship->bindings != NULL)
                                                           {
                                                               meshRenderShipHierarchyForShip(
                                                                   ship,
                                                                   ship->bindings,
                                                                   ship->currentLOD,
                                                                   (meshdata *)level->pData,
                                                                   i);
                                                           }
                                                           else
                                                           {
                                                               meshRender((meshdata *)level->pData, i);
                                                           }
                                                       }

                                                       meshSetFade(0, morphProgress);
                                                       if (targetRace == 1)
                                                       {
                                                           effectState[0] = 1;
                                                       }

                                                       if (targetBindings != NULL)
                                                       {
                                                           meshRenderShipHierarchyCoreForShip(
                                                               ship,
                                                               targetBindings,
                                                               targetLOD,
                                                               targetMesh,
                                                               &ship->staticinfo->staticheader.staticCollInfo.collsphereoffset,
                                                               morphProgress,
                                                               displaySphereRadius,
                                                               targetColorScheme);
                                                       }
                                                       else
                                                       {
                                                           meshMorphedPolygonObjectRenderForShip(
                                                               ship,
                                                               NULL,
                                                               &targetMesh->object[0],
                                                               targetMesh->object[0].pPolygonList,
                                                               targetMesh->localMaterial,
                                                               &ship->staticinfo->staticheader.staticCollInfo.collsphereoffset,
                                                               morphProgress,
                                                               displaySphereRadius,
                                                               targetColorScheme);
                                                       }

                                                       meshSetFade(0, 0.0f);
                                                       effectState[0] = 0;
                                                   }
                                               }
                                               else
                                               {
                                                   meshRenderShipHierarchyForShip(
                                                       (Ship *)spaceobj,
                                                       ((Ship *)spaceobj)->bindings,
                                                       ((Ship *)spaceobj)->currentLOD,
                                                       (meshdata *)level->pData,
                                                       i);
#if VISIBLE_POLYS
                                                    if (visiblePoly >= 0)
                                                    {
                                                        g_SpecificPoly = TRUE;
                                                        g_WireframeHack = TRUE;
                                                        glScalef(1.02f, 1.02f, 1.02f);
                                                        meshRenderShipHierarchyForShip((Ship *)spaceobj,
                                                                               ((Ship*)spaceobj)->bindings,
                                                                               ((Ship*)spaceobj)->currentLOD,
                                                                               (meshdata*)level->pData, i);
                                                        g_WireframeHack = FALSE;
                                                        glScalef(1.01f, 1.01f, 1.01f);
                                                        g_Points = TRUE;
                                                        meshRenderShipHierarchyForShip((Ship *)spaceobj,
                                                                               ((Ship*)spaceobj)->bindings,
                                                                               ((Ship*)spaceobj)->currentLOD,
                                                                               (meshdata*)level->pData, i);
                                                        g_Points = FALSE;
                                                        g_SpecificPoly = FALSE;
                                                    }
#endif
                                               }

                                               if (cloakRenderPass != 0)
                                               {
                                                   g_WireframeHack = FALSE;
                                               }
                                           }
                                        }

                                        cataRenderScratch.damage.unfinishedPass = FALSE;
                                        if (cataShipUsesExternalBuildClipPlane((Ship *)spaceobj))
                                        {
                                            cataClearExternalBuildPassState();
                                        }
                                        if (crystalProcessingClipActive)
                                        {
                                            cataClearCrystalProcessingPassState(crystalProcessingCullFaceEnabled);
                                        }
                                        if (ShipHasUpgrade((Ship *)spaceobj, RESEARCH_UPGRADE_MASK_REPAIR_BOTS) &&
                                            enableRepairbots &&
                                            ((Ship *)spaceobj)->staticinfo->shiprace == RACE_Sect &&
                                            ((Ship *)spaceobj)->runtimeAnimationOverlay.runtimeController != 0 &&
                                            ((Ship *)spaceobj)->currentLOD < 3u)
                                        {
                                            repairDroidRenderAttachments(
                                                (RepairDroidController *)((Ship *)spaceobj)->runtimeAnimationOverlay.runtimeController);
                                        }
                                        spaceobj->renderedLODs |= (1 << spaceobj->currentLOD);

                                        rndPerspectiveCorrection(FALSE);

                                        //navlights
                                        if (!bitTest(spaceobj->flags, SOF_Cloaked))
                                        {
                                            RenderNAVLights((Ship*)spaceobj);
                                        }
                                    }

                                    g_WireframeHack = FALSE;
                                    g_HiddenRemoval = TRUE;

                                    shDockLight(0.0f);

#if VERBOSE_SHIP_STATS
                                    if (rndDisplayFrameRate)
                                    {
                                        shipsAtLOD[spaceobj->currentLOD]++;
                                        {
                                            meshdata* mesh = (meshdata*)level->pData;
                                            polysAtLOD[spaceobj->currentLOD] += mesh->object[0].nPolygons;
                                        }
                                    }
#endif
                                }

                                rndUnFade();
                            }
                            shPopLightMatrix();

                            if (glCapFeatureExists(GL_RESCALE_NORMAL))
                            {
                                glDisable(GL_RESCALE_NORMAL);
                                rndNormalization = FALSE;
                            }
                            else
                            {
                                rndNormalizeEnable(FALSE);
                            }
                            break;
                        case LT_TinySprite:
                            dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                            break;
                        case LT_SubPixel:
                            rndTextureEnable(FALSE);
                            rndLightingEnable(FALSE);
                            primPointSize3(&spaceobj->posinfo.position,
                                ((Ship *)spaceobj)->collInfo.selCircleRadius,
                                toPlayerColor[playerIndex]);
                            rndLightingEnable(TRUE);
                            break;
                        case LT_Function:
                            dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                            break;
                        case LT_NULL:
                            break;
                        default:
                            dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                    }

                    if (rndDamageFlashBiasActive)
                    {
                        rndDamageFlashBiasActive = FALSE;
                        if (RGL)
                        {
                            glPixelTransferf(GL_RED_BIAS, 0.0f);
                            glPixelTransferf(GL_GREEN_BIAS, 0.0f);
                            glPixelTransferf(GL_BLUE_BIAS, 0.0f);
                        }
                    }

                    {
                        udword index;
                        for (index = 0; index < 2; index++)
                        {
                            if (((Ship*)spaceobj)->lightning[index] != NULL)
                            {
                                lightning* l = (lightning*)((Ship*)spaceobj)->lightning[index];
                                cloudRenderAndUpdateLightning(l, spaceobj->currentLOD);

                                l->countdown--;
                                if (l->countdown == 0)
                                {
                                    cloudKillLightning(l);
                                    ((Ship*)spaceobj)->lightning[index] = NULL;
                                }
                            }
                        }
                    }

                    if (cataRenderScratch.statusWindowActive)
                    {
                        if (((Ship *)spaceobj)->bindings != NULL)
                        {
                            meshRenderLightningHierarchy((Ship *)spaceobj, (meshdata *)level->pData);
                        }
                        else
                        {
                            meshRenderLightning((meshdata *)level->pData);
                        }
                    }

                    glPopMatrix();

                    {
                        Ship* ship = (Ship*)spaceobj;
                        ShipSinglePlayerGameInfo* ssinfo = ship->shipSinglePlayerGameInfo;

                        if (ssinfo != NULL &&
                            ssinfo->hsState != HS_INACTIVE &&
                            ssinfo->hsState != HS_FINISHED)
                        {
                            if (ssinfo->staticHyperspaceGate)
                            {
                                hsNoGate(TRUE);
                            }
                            hsEnd((Ship*)spaceobj, displayEffect);
                            if (ssinfo->staticHyperspaceGate)
                            {
                                hsNoGate(FALSE);
                            }
                        }
                    }

                    rndPerspectiveCorrection(FALSE);

                    rndFade(spaceobj, camera);
                    if (!bitTest(spaceobj->flags, SOF_Cloaked) || (((Ship*)spaceobj)->playerowner == universe.curPlayerPtr) || proximityCanPlayerSeeShip(universe.curPlayerPtr,(Ship*)spaceobj))

                    {
                        udword idx;
                        for (idx = 0; idx < MAX_NUM_TRAILS; idx++)
                        {
                            if (((Ship*)spaceobj)->trail[idx] != NULL)
                            {
                                trailDraw(&((Ship*)spaceobj)->enginePosition,
                                          ((Ship*)spaceobj)->trail[idx],
                                          spaceobj->currentLOD);
                            }
                        }
                    }

                    if ((((Ship *)spaceobj)->shiptype == sSentinel || ((Ship *)spaceobj)->shiptype == bSentinel) &&
                        ((Ship *)spaceobj)->command != NULL &&
                        ((Ship *)spaceobj)->command->selection != NULL &&
                        ((Ship *)spaceobj)->command->selection->ShipPtr[0] == (Ship *)spaceobj)
                    {
                        beamRenderBeams(&camera->eyeposition, ((Ship *)spaceobj)->command->selection);
                    }
                    rndUnFade();
                }
            }
            else
            {                                           //else behind camera
                ((Ship *)spaceobj)->collInfo.selCircleRadius = -1.0f;
            }
            break;
        case OBJ_DerelictType:
            if (((Derelict *)spaceobj)->staticinfo->worldRender)
            {                                           //if it's a world derelict
                break;                                  //render as normal
            }

            if (((Derelict *)spaceobj)->derelicttype == HyperspaceGate)
            {
                selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);//compute selection circle

#if RND_VISUALIZATION
                if (RENDER_BOXES)
                {
                    collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
                }
#endif
                break;
            }

            //else fall through
        case OBJ_AsteroidType:
        case OBJ_DustType:
        case OBJ_GasType:
        case OBJ_MissileType:
            //cloaked effect hack...change later when implement 'stages maybe'
            if(bitTest(spaceobj->flags,SOF_Cloaked))
                break;   //if it isn't cloaked, or it is and it is the local players field draw it

            if (spaceobj->staticinfo->staticheader.LOD != NULL)
            {
                dbgAssert((spaceobj->flags & (SOF_Rotatable+SOF_Impactable+SOF_Targetable)) != 0);

                //add this space object to the mission map array
//                    mmAddObjToMapArray(spaceobj);

                selCircleCompute(&rndCameraMatrix, &rndProjectionMatrix, (SpaceObjRotImpTarg *)spaceobj);//compute selection circle

#if RND_VISUALIZATION
#ifdef DEBUG_COLLISIONS
                collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
#else
                if (RENDER_BOXES)
                {
                    collDrawCollisionInfo((SpaceObjRotImp *)spaceobj);
                }
#endif
#endif

                glPushMatrix();
                level = lodLevelGet((void *)spaceobj, &camera->eyeposition, &((SpaceObjRotImp *)spaceobj)->collInfo.collPosition);

                if (taskTimeElapsed-((Ship *)spaceobj)->flashtimer < FLASH_TIMER)
                {
                    rndDamageFlashBiasActive = TRUE;
                    if (RGL)
                    {
                        glPixelTransferf(GL_RED_BIAS, 0.3f);
                        glPixelTransferf(GL_GREEN_BIAS, 0.3f);
                        glPixelTransferf(GL_BLUE_BIAS, 0.3f);
                    }
                }

                switch (level->flags & LM_LODType)
                {
                    case LT_Invalid:
                        break;
                    case LT_Mesh:
#if LIGHT_PLAYER_COLORS
//                            lightDefaultLightSet();
//!!!                            lightSetLighting();
#endif
                        g_RndPosition = spaceobj->posinfo.position;
                        hmatMakeHMatFromMat(&coordMatrixForGL,&((SpaceObjRot *)spaceobj)->rotinfo.coordsys);
                        hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position,coordMatrixForGL);
                        glMultMatrixf((float *)&coordMatrixForGL);//ship's rotation matrix
                        rndApplyComponentSpinTransform(spaceobj);
                        shPushLightMatrix(&coordMatrixForGL);

                        if (spaceobj->objtype == OBJ_AsteroidType)
                        {
                            real32 scaling = ((Asteroid *)spaceobj)->scaling;
                            if (scaling != 1.0f)
                            {
                                glScalef(scaling,scaling,scaling);
                            }
                        }

#ifndef HW_Release
                        if(spaceobj->objtype == OBJ_DerelictType)
                        if(((Derelict *)spaceobj)->derelicttype < NUM_DERELICTTYPES)
                        if (dockLines) dockDrawSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)spaceobj);
#endif
                        colorScheme = 0;
                        switch (spaceobj->objtype)
                        {
                        case OBJ_GasType:
                        case OBJ_DustType:
                            rndFade(spaceobj, camera);
                            meshRenders++;
                            cloudRenderSystem((void*)level->pData, ((DustCloud*)spaceobj)->stub, spaceobj->currentLOD);
//                                shipsAtLOD[spaceobj->currentLOD]++;
                            rndUnFade();
                            break;
                        case OBJ_MissileType:
                            colorScheme = ((Missile *)spaceobj)->colorScheme;
                            goto renderDefault;
                        case OBJ_DerelictType:
                            colorScheme = ((Derelict *)spaceobj)->colorScheme;
                            //fall through
renderDefault:
                        default:
                            if (rndShipVisible(spaceobj, camera))
                            {
                                rndFade(spaceobj, camera);

                                if (spaceobj->currentLOD == 0)
                                {
                                    rndPerspectiveCorrection(TRUE);
                                }
                                rndGLStateLog("Before Derelict");
                                meshRenders++;
                                meshRender((meshdata *)level->pData,colorScheme);
                                spaceobj->renderedLODs |= (1 << spaceobj->currentLOD);
                                rndGLStateLog("After Derelict");
                                //navlights
                                if (spaceobj->objtype == OBJ_DerelictType && !bitTest(spaceobj->flags, SOF_Cloaked))
                                {
                                    RenderNAVLights((Ship*)spaceobj);
                                }

                                rndPerspectiveCorrection(FALSE);
#if VERBOSE_SHIP_STATS
                                if (rndDisplayFrameRate)
                                {
                                    shipsAtLOD[spaceobj->currentLOD]++;
                                    {
                                        meshdata* mesh = (meshdata*)level->pData;
                                        polysAtLOD[spaceobj->currentLOD] += mesh->object[0].nPolygons;
                                    }
                                }
#endif
                                rndUnFade();
                            }
                        }
                        shPopLightMatrix();
                        break;

                    case LT_TinySprite:
                        dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                        break;
                    case LT_SubPixel:
                        rndTextureEnable(FALSE);
                        rndLightingEnable(FALSE);
                        primPointSize3(&spaceobj->posinfo.position,
                                       ((Ship *)spaceobj)->collInfo.selCircleRadius,
                                       spaceobj->staticinfo->staticheader.LOD->pointColor);
                        rndLightingEnable(TRUE);
                        break;
                    case LT_Function:
                        dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                        break;
                    case LT_NULL:
                        break;
                    default:
                        dbgFatalf(DBG_Loc, "Unsupported LOD type 0x%x", level->flags);
                }

                if (rndDamageFlashBiasActive)
                {
                    rndDamageFlashBiasActive = FALSE;
                    if (RGL)
                    {
                        glPixelTransferf(GL_RED_BIAS, 0.0f);
                        glPixelTransferf(GL_GREEN_BIAS, 0.0f);
                        glPixelTransferf(GL_BLUE_BIAS, 0.0f);
                    }
                }

                glPopMatrix();

                if (spaceobj->objtype == OBJ_MissileType)
                {
                    Missile* mis = (Missile*)spaceobj;

                    if (mis->trail != NULL)
                    {
                        playerIndex = (sdword)mis->playerowner->playerIndex;
                        mistrailDraw(&mis->enginePosition, mis->trail,
                                     spaceobj->currentLOD, mis->colorScheme);
                    }
                }
            }
            break;
        case OBJ_NebulaType:
            //do nothing
            break;
        default:
            dbgFatalf(DBG_Loc, "Undefined object type %d", spaceobj->objtype);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndMainViewRenderFunction
    Description : Render a mission sphere as referenced by a specific camera.
    Inputs      : camera - pointer to a camera structure which includes a
                    mission sphere.
    Outputs     : ??
    Return      : void
    Note        : Because much of the functionality of this function is mirrored
                    in rndMainViewAllButRenderFunction, any drastic changes to
                    rndMainViewRenderFunction will have to be made to
                    rndMainViewAllButRenderFunction.
                  Retail splits per-space-object render dispatch into a helper
                    at 0x5776F0 (`rndRenderSpaceObj`). This source tree now
                    mirrors that split with a local helper call.
----------------------------------------------------------------------------*/
void rndMainViewRenderFunction(Camera *camera)
{
    Node *objnode;
    bool32 twopass;
    SpaceObj *spaceobj;
    udword i,numstars;
    Star3d *star;
    static sdword init = FALSE;
//    hvector cameraSpace, eyeSpace;
//    matrix scaledMatrix;
//    real32 scaleFactor;
//    meshdata *worldMesh;
    Effect *effect;
    static sdword shipTrails;
    extern sdword trailsUpdated, trailsNotUpdated, trailsRendered;

    sdword asteroid0Count;

    real32 scaledOffset[3];
    static real32 cameraOffset[3] = {0.0f, 0.0f, 0.0f};
    static bool32   cameraFloating = FALSE;

    mouseCursorObjPtr  = NULL;               //Falko: got an obscure crash where mouseCursorObjPtr was mangled, will this work?

    dbgAssert(camera != NULL); //verify parameters

#if 0
    if (!binkDonePlaying)
    {                               //!!! can be cleaned up using the render function pointer
        return;
    }
#endif

    if (RGL)
    {
        rglFeature(RGL_LOCK);                               //exclusive lock on the framebuffer
    }

    primModeClear2();                                       //go to 3D rendering mode

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluPerspective(camera->fieldofview, rndAspectRatio,    //set projection matrix
                    rndNear(camera->clipPlaneNear), camera->clipPlaneFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (gameIsRunning && !nisIsRunning && piePointSpecMode == PSM_Idle && !universePause)
    {
        if (!cameraFloating)
        {
            rndResetFloatTime();
        }
        cameraFloating = TRUE;
    }
    else
    {
        cameraFloating = FALSE;
        if (nisIsRunning)
        {
            cameraOffset[0] = cameraOffset[1] = cameraOffset[2] = 0.0f;
        }
    }

    if (bitTest(universe.mainCameraCommand.ccMode,CCMODE_PILOT_SHIP))
    {
        cameraFloating = FALSE;
        cameraOffset[0] = cameraOffset[1] = cameraOffset[2] = 0.0f;
    }

    if (cameraFloating)
    {
        cameraOffset[0] = CAMERA_FLOAT_OFFSET_SCALAR0 * rndCameraOffset0();
        cameraOffset[1] = CAMERA_FLOAT_OFFSET_SCALAR1 * rndCameraOffset1();
        cameraOffset[2] = (0.5f * cameraOffset[0]) + (0.5f * cameraOffset[1]);
    }
    rndScaleCameraOffsets(scaledOffset, camera, cameraOffset);
    rgluLookAt(camera->eyeposition.x + scaledOffset[2],
               camera->eyeposition.y + scaledOffset[0],
               camera->eyeposition.z + scaledOffset[1],
               camera->lookatpoint.x, camera->lookatpoint.y, camera->lookatpoint.z,
               camera->upvector.x, camera->upvector.y, camera->upvector.z);
    //get local copy of matrices
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)(&rndCameraMatrix));
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)(&rndProjectionMatrix));

    //position light(s) in world
    lightPositionSet();

    rndPreRenderDebugStuff(camera);

    //!!! should this be here?  It does not sound like it.
    if (feRenderEverything)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        feRenderEverything = FALSE;
    }

    rndUpdateTransitionOverlay();

//draw the texture-mapped background
#if RND_BACKGROUND_STATIC
    glPushMatrix();
        glTranslatef(camera->eyeposition.x, camera->eyeposition.y, camera->eyeposition.z);
#endif
        primErrorMessagePrint();

        numstars=universe.star3dinfo->Num3dStars;
        if (stararray == NULL)
        {
            c4ub_v3f* ptr;

            stararray = (c4ub_v3f*)memAlloc((numstars - NUM_BIG_STARS) * sizeof(c4ub_v3f), "stararray", NonVolatile);
            bigstararray = (c4ub_v3f*)memAlloc(NUM_BIG_STARS * sizeof(c4ub_v3f), "bigstararray", NonVolatile);

            ptr = stararray;
            for (i = 0, star = universe.star3dinfo->Stars; i < (numstars - NUM_BIG_STARS); i++, star++, ptr++)
            {
                ptr->c[0] = star->r;
                ptr->c[1] = star->g;
                ptr->c[2] = star->b;
                ptr->c[3] = 200;
                ptr->v[0] = star->position.x;
                ptr->v[1] = star->position.y;
                ptr->v[2] = star->position.z;
            }

            for (ptr = bigstararray; i < numstars; i++, star++, ptr++)
            {
                ptr->c[0] = star->r;
                ptr->c[1] = star->g;
                ptr->c[2] = star->b;
                ptr->c[3] = 200;
                ptr->v[0] = star->position.x;
                ptr->v[1] = star->position.y;
                ptr->v[2] = star->position.z;
            }
        }

        rndLightingEnable(FALSE);                           //stars are self-illuminated
        primErrorMessagePrint();
        glDisable(GL_DEPTH_TEST);                           //and infinitely far away
        primErrorMessagePrint();
        if (singlePlayerGame && singlePlayerGameInfo.currentMission == 13)
        {
            rndBackgroundRender(BG_RADIUS, camera, !showBackgrounds);
        }
        else
        {
            rndBackgroundRender(BG_RADIUS, camera, TRUE);
        }

        rndTextureEnvironment(RTE_Modulate);
        glEnable(GL_DEPTH_TEST);                            //re-enable depth test
        primErrorMessagePrint();
        rndLightingEnable(TRUE);                            //and lighting
        primErrorMessagePrint();
#if RND_BACKGROUND_STATIC
    glPopMatrix();
#endif
#if !LIGHT_PLAYER_COLORS
//    lightDefaultLightSet();
    lightSetLighting();
#endif

    //render all worlds in the universe
    for (i = 0; i < UNIV_NUMBER_WORLDS; i++)
    {
        if (universe.world[i] != NULL)
        {                                                   //if this one defined
            if (universe.world[i]->objtype == OBJ_DerelictType)
            {                                               //if it's a derelict
                rndRenderAHomeworld(camera, universe.world[i]);
            }
            else
            {                                               //else it must be a world effect
                dbgAssert(universe.world[i]->objtype == OBJ_EffectType);
                rndRenderAWorldEffect(camera, (Effect *)universe.world[i]);
            }
        }
        else
        {                                                   //stop processing worlds
            break;
        }
    }
    if (rndPreObjectCallback != NULL)
    {                                                       //render the pre-object callback if needed
        rendercallback rc = rndPreObjectCallback;
        rndPreObjectCallback = NULL;
        rc();
    }

    if (RGL && rndFogOn)
    {
        glEnable(GL_FOG);
    }

#if PIE_MOVE_NEARTO
    //clear out the 'move near to' info
    selClosestTarget = NULL;
    selClosestDistance = SDWORD_Max;
#endif //PIE_MOVE_NEARTO

    trailsRendered = shipTrails = 0;
    alodSetPolys(0);

#if WILL_TWO_PASS
    if (RGL && (RGLtype == SWtype))
    {
        twopass = TRUE;
    }
    else
    {
        twopass = FALSE;
    }
#else
    twopass = FALSE;
#endif
    if (twopass)
    {
        objnode = universe.RenderList.tail;
    }
    else
    {
        objnode = universe.RenderList.head;
    }

    rndTransparentSort(1);
    while (objnode != NULL)
    {
        spaceobj = (SpaceObj *)listGetStructOfNode(objnode);
        beamUpdateEffects(&camera->eyeposition, spaceobj);

        g_WireframeHack = FALSE;
        rndPerspectiveCorrection(FALSE);
        cataRenderScratch.ship = NULL;
        cataClearDamageRenderState();

       rndRenderSpaceObj(camera, spaceobj, twopass);
       if (twopass)
       {
           objnode = objnode->prev;
        }
        else
        {
            objnode = objnode->next;
        }
    }

    if (!twopass)
    {
        rndTransparentSort(2);
        while (cataTransparentQueueIndex < cataTransparentQueueCount)
        {
            spaceobj = cataTransparentQueue[cataTransparentQueueIndex++];
            beamUpdateEffects(&camera->eyeposition, spaceobj);

            g_WireframeHack = FALSE;
            rndPerspectiveCorrection(FALSE);
            cataRenderScratch.ship = NULL;
            cataClearDamageRenderState();

            rndRenderSpaceObj(camera, spaceobj, FALSE);
        }
    }

    if (twopass)
    {
        objnode = universe.RenderList.head;
        while (objnode != NULL)
        {
            spaceobj = (SpaceObj*)listGetStructOfNode(objnode);
            beamUpdateEffects(&camera->eyeposition, spaceobj);
            g_WireframeHack = FALSE;
            rndPerspectiveCorrection(FALSE);
            switch (spaceobj->objtype)
            {
            case OBJ_EffectType:
                effect = (Effect *)spaceobj;                    //get effect pointer
                if(effect->owner != NULL)
                {
                   SpaceObj *effectowner = (SpaceObj *)effect->owner;
                   if(effectowner->objtype == OBJ_BulletType)
                   {
                     //effect is owned by a bullet
                     if((WeaponType)((Bullet *) effectowner)->bulletType == WEAPON_Laser)
                     {
                        //effect is owned by a defense fighter laser beam
                        defenseFighterAdjustLaser(((Bullet *) effectowner));
                        etgEffectUpdate(effect, 0.0f);                        //luke suggested fix to laser length problem
                     }
                     else if((WeaponType)((Bullet *) effectowner)->bulletType == WEAPON_BeastInfection)
                     {
                        etgEffectUpdate(effect, 0.0f);
                     }
                     else if((WeaponType)((Bullet *) effectowner)->bulletType == WEAPON_Ion)
                     {
                         if(((Bullet *)effectowner)->timelived <= UNIVERSE_UPDATE_PERIOD)
                         {
                            goto dontdraw3;
                         }
                     }
                   }
                }
                etgEffectDraw(effect);
dontdraw3:;
            }
            objnode = objnode->next;
        }
    }

    beamFlushEffects();
    rndTransparentSort(0);

    //
    // minor renderlist (asteroid0 list)
    //
    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);
    g_WireframeHack = FALSE;
    glPointSize(1.0f);

    asteroid0Count = 0;

    objnode = universe.MinorRenderList.head;
    while (objnode != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(objnode);

        switch (spaceobj->objtype)
        {
        case OBJ_AsteroidType:
            asteroid0Data[asteroid0Count].c = spaceobj->staticinfo->staticheader.LOD->pointColor;
            asteroid0Data[asteroid0Count].position = &spaceobj->posinfo.position;
            asteroid0Count++;
            if (asteroid0Count == ASTEROID0DATA_SIZE)
            {
                rndDrawAsteroid0(asteroid0Count);
                asteroid0Count = 0;
            }
            break;

        default:
            dbgFatalf(DBG_Loc, "MinorRenderList contains invalid object type %d", spaceobj->objtype);
        }

        objnode = objnode->next;
    }
    if (asteroid0Count != 0)
    {
        rndDrawAsteroid0(asteroid0Count);
    }
    rndLightingEnable(TRUE);

    if (byte_8B8844)
    {
        ccPilotViewRenderEffect(camera, &pilotEffect);
    }
    shockwaveRenderAll();

    //hyperspace
    hsStaticRender();

    alodAdjustScaleFactor();

    rndPerspectiveCorrection(FALSE);

    if (RGL)
    {
        glDisable(GL_FOG);
    }

    nebRender();
    rndRenderLensflares();

    if (rndPostObjectCallback != NULL)
    {                                                       //render the post-object callback if needed
        rndPostObjectCallback();
        rndPostObjectCallback = NULL;
    }
    rndPostRenderDebug3DStuff(camera);
    primModeSet2();
    rndPostRenderDebug2DStuff(camera);
    if (RGL)
    {
        rglFeature(RGL_UNLOCK);         //remove our exclusive lock
    }
}

GLuint plug_handle = 0;
sdword plug_width, plug_height;

typedef struct tagTGAHeader
{
    unsigned char idLength;
    unsigned char colorMapType;
    unsigned char imageType;
    // Color Map information.
    unsigned short colorMapStartIndex;
    unsigned short colorMapNumEntries;
    unsigned char colorMapBitsPerEntry;
    // Image Map information.
    signed short imageOffsetX;
    signed short imageOffsetY;
    unsigned short imageWidth;
    unsigned short imageHeight;
    unsigned char pixelDepth;
    unsigned char imageDescriptor;
} TGAHeader;

//rndLoadTarga
udword rndLoadTarga(char* filename, sdword* width, sdword* height)
{
    TGAHeader head;
    ubyte* data;
    ubyte* pdata;
    ubyte* bp;
    uword* psdata;
    sdword i;
    GLuint handle;

    fileLoadAlloc(filename, (void**)&data, 0);

    pdata = data;
    head.idLength = *pdata++;
    head.colorMapType = *pdata++;
    head.imageType = *pdata++;
    psdata = (uword*)pdata;
    head.colorMapStartIndex = *psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.colorMapNumEntries = *psdata;
    pdata += 2;
    head.colorMapBitsPerEntry = *pdata++;
    psdata = (uword*)pdata;
    head.imageOffsetX = (sword)*psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.imageOffsetY = (sword)*psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.imageWidth = *psdata;
    pdata += 2;
    psdata = (uword*)pdata;
    head.imageHeight = *psdata;
    pdata += 2;
    head.pixelDepth = *pdata++;
    head.imageDescriptor = *pdata++;

    dbgAssert(head.pixelDepth == 32);

    pdata += head.idLength;

    *width  = (sdword)head.imageWidth;
    *height = (sdword)head.imageHeight;

    for (i = 0, bp = pdata; i < 4*(*width)*(*height); i += 4, bp += 4)
    {
        ubyte r, b;
        r = bp[0];
        b = bp[2];
        bp[0] = b;
        bp[2] = r;
        bp[3] = (ubyte)(255 - (int)bp[3]);
    }

    glGenTextures(1, &handle);
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pdata);

    memFree(data);

    return handle;
}

/*-----------------------------------------------------------------------------
    Name        : rndLoadShamelessPlug
    Description : loads or frees the Relic shameless plug texture
    Inputs      : on - TRUE or FALSE (load or free)
    Outputs     : plug_handle is either valid or invalid
    Return      :
----------------------------------------------------------------------------*/
void rndLoadShamelessPlug(bool32 on)
{
    if (on)
    {
        plug_handle = rndLoadTarga("plug.tga", &plug_width, &plug_height);
    }
    else
    {
        if (plug_handle != 0)
        {
            glDeleteTextures(1, &plug_handle);
            plug_handle = 0;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndShamelessPlug
    Description : displays the Relic shameless plug in bottom right corner of screen
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndShamelessPlug()
{
    GLboolean blendOn, depthOn;
    GLfloat   fwidth, fheight;
    GLint     matrixMode;
    sdword    lightOn;

    GLfloat   projection[16];
    GLfloat   winWidth, winHeight;

#ifdef fpoiker
    return;
#endif

    if (plug_handle == 0)
    {
        rndLoadShamelessPlug(TRUE);
    }

    fwidth  = (GLfloat)plug_width;
    fheight = (GLfloat)plug_height;
    winWidth  = (GLfloat)MAIN_WindowWidth;
    winHeight = (GLfloat)MAIN_WindowHeight;

    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rgluOrtho2D(0.0f, winWidth, 0.0f, winHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    blendOn = glIsEnabled(GL_BLEND);
    lightOn = rndLightingEnable(FALSE);
    depthOn = glIsEnabled(GL_DEPTH_TEST);
    if (!blendOn) glEnable(GL_BLEND);
    if (depthOn)  glDisable(GL_DEPTH_TEST);

    winWidth  -= 1.0f;
    winHeight -= 1.0f;

    if (RGLtype == SWtype)
    {
        glTexCoord2f(winWidth - fwidth, 0);
        trClearCurrent();
        glBindTexture(GL_TEXTURE_2D, plug_handle);
        glDrawPixels(plug_width, plug_height, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    else
    {
        rndTextureEnable(TRUE);
        rndAdditiveBlends(FALSE);
        trClearCurrent();
        glBindTexture(GL_TEXTURE_2D, plug_handle);

        glColor4ub(255,255,255,255);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(winWidth - fwidth, fheight);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(winWidth - fwidth, 0);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(winWidth, 0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(winWidth, fheight);
        glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);
    if (matrixMode == GL_MODELVIEW)
    {
        glMatrixMode(GL_MODELVIEW);
    }

    rndLightingEnable(lightOn);
    if (!blendOn) glDisable(GL_BLEND);
    if (depthOn)  glEnable(GL_DEPTH_TEST);
}

extern udword receivedPacketNumber;

static udword printLagMinFrames = 0;

extern udword PRINTLAG_IFGREATERTHAN;
extern udword PRINTLAG_MINFRAMES;

/*-----------------------------------------------------------------------------
    Name        : rndDrawOnScreenSyncStatus
    Description : Prints sync status to screen
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndDrawOnScreenSyncStatus(void)
{
#if SYNC_CHECK
    if ((multiPlayerGame|playPackets) && (gameIsRunning))
    {
        sdword y;
        udword rsyncpkts = queueNumberEntries(ProcessSyncPktQ);

        if (netcheckRandSyncErr)
        {
            fontPrintf(400,10,colWhite,"RAND SYNC ERR: Frame %d",netcheckRandSyncErrFrame);
        }
        if (netcheckUnivSyncErr)
        {
            fontPrintf(400,20,colWhite,"UNIV SYNC ERR: Frame %d",netcheckUnivSyncErrFrame);
        }
        if ((netcheckBlobSyncErr) && (multiPlayerGame)) // only print cheat detect in multiPlayerGame
        {
            fontPrintf(400,30,colWhite,strGetString(strCheatDetect),netcheckBlobSyncErrFrame);
        }
        if (netcheckPktSyncErr)
        {
            fontPrint(400,40,colWhite,"PKT SYNC ERR");
        }

        y = 50;

        if ((printLagMinFrames) || (rsyncpkts > PRINTLAG_IFGREATERTHAN))
        {
            if (rsyncpkts > PRINTLAG_IFGREATERTHAN)
                printLagMinFrames = PRINTLAG_MINFRAMES;
            else if (printLagMinFrames)
                printLagMinFrames--;

            //fontPrintf(400,y,colWhite,"Lag >= %3.3f",(real32)(rsyncpkts * CAPTAINSERVER_PERIOD));
            lagSlowComputerIcon();
        }

        lagSlowInternetIcon();

        if ((playPackets) && (!recordplayPacketsInGame))
        {
            fontPrintf(400,y,colWhite,"%d %f",receivedPacketNumber,universe.totaltimeelapsed);
            y += 10;
        }

/*        if (multiPlayerGame)
        {
            sdword i;

            for (i=0;i<sigsNumPlayers;i++)
            {
                if (playersReadyToGo[i] == PLAYER_DROPPED_OUT)
                {
                    fontPrintf(400,y,HorseRaceDropoutColor,"%s %s",playerNames[i],strGetString(strDroppedOut));
                    y += 10;
                }
            }
        }*/
    }
#endif
}

void rndPostObjectCallbackExecute(void)
{
    regFunctionsDraw();
}

/*-----------------------------------------------------------------------------
    Name        : rndDrawOnScreenDebugInfo
    Description : Handle render-time debugging info to print on the screen
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VERBOSE_LEVEL >= 1
extern sdword numchans[];
extern sdword channelsinuse;

void rndDrawOnScreenDebugInfo(void)
{
    static sdword counter = 0;
    static color c;
    sdword y = 0;
#if MR_TEST_HPB
    if (mrTestHPBMode)
    {
        if (selSelected.numShips == 1)
        {
            sdword width = fontWidth("H:  -9999.00");
            fontPrintf(MAIN_WindowWidth - width, 0, colWhite, "H:  %.2f", mrHeading * 180.0f / PI);
            fontPrintf(MAIN_WindowWidth - width, fontHeight(" ") + 1, colWhite, "P:  %.2f", mrPitch * 180.0f / PI);
            fontPrintf(MAIN_WindowWidth - width, (fontHeight(" ") + 1) * 2, colWhite, "B:  %.2f", mrBank * 180.0f / PI);
            fontPrintf(MAIN_WindowWidth - width, (fontHeight(" ") + 1) * 3, colWhite, "I: %d", madTestHPBIndex);
            if (keyIsHit(ALTKEY))
            {
                mrHeading = mrPitch = mrBank = 0.0f;
            }
        }
        mouseCursorHide();
    }
    mrTestHPBMode = FALSE;
#endif
#if PIE_VISUALIZE_VERTICAL
    if (pieOnScreen)
    {
        static bool32 pieFlashFlag = FALSE;
        pieFlashFlag ^= TRUE;
        if (pieFlashFlag)
        {
            primLine2((sdword)pieScreen0.x, 479 - (sdword)pieScreen0.y, (sdword)pieScreen1.x, 479 - (sdword)pieScreen1.y, colWhite);
        }
    }
#endif //PIE_VISUALIZE_VERTICAL

    y = 0;

#if VISIBLE_POLYS
    if (selSelected.numShips == 1)
    {
        extern sdword visiblePoly, visibleSegment, visibleDirection, visibleWhich;
        extern real32 visibleUV[3][2];
        if (counter > 0)
        {
            counter--;
        }
        else
        {
            counter = 4;
            if (keyIsHit(NINEKEY))
            {
                //dec
                if (visiblePoly >= 0)
                {
                    if (keyIsHit(SHIFTKEY))
                    {
                        visiblePoly -= 10;
                        if (visiblePoly < -1)
                        {
                            visiblePoly = -1;
                        }
                    }
                    else
                    {
                        visiblePoly--;
                    }
                }
            }
            else if (keyIsHit(ZEROKEY))
            {
                //inc
                if (keyIsHit(SHIFTKEY))
                {
                    visiblePoly += 10;
                }
                else
                {
                    visiblePoly++;
                }
            }
            else if (keyIsHit(ONEKEY))
            {
                visibleDirection = -1;
                visibleWhich = 0;
            }
            else if (keyIsHit(TWOKEY))
            {
                visibleDirection = +1;
                visibleWhich = 0;
            }
            else if (keyIsHit(THREEKEY))
            {
                visibleDirection = -1;
                visibleWhich = 1;
            }
            else if (keyIsHit(FOURKEY))
            {
                visibleDirection = +1;
                visibleWhich = 1;
            }
            else if (keyIsHit(FIVEKEY))
            {
                visibleSegment++;
                if (visibleSegment > 2)
                {
                    visibleSegment = 0;
                }
            }
            else if (keyIsHit(SEVENKEY))
            {
                extern bool32 g_Output;
                g_Output = TRUE;
            }
        }
        fontPrintf(0, 0, colWhite, "%d poly %d", visibleSegment, visiblePoly);
        c = (visibleSegment == 0) ? colRGB(255,0,0) : colWhite;
        fontPrintf(0, 16, c, "%0.4f %0.4f", visibleUV[0][0], visibleUV[0][1]);
        c = (visibleSegment == 1) ? colRGB(0,255,0) : colWhite;
        fontPrintf(0, 32, c, "%0.4f %0.4f", visibleUV[1][0], visibleUV[1][1]);
        c = (visibleSegment == 2) ? colRGB(0,0,255) : colWhite;
        fontPrintf(0, 48, c, "%0.4f %0.4f", visibleUV[2][0], visibleUV[2][1]);
    }
#endif

#if RND_FRAME_RATE
    if (rndDisplayFrameRate)
    {                                                   //display frame rate
        cmDeterministicBuildDisplay();
        if (rndFrameCountMin == SDWORD_Max)
        {
            fontPrintf(0, 0, colReddish, "%.2f <= %.2f <= %.2f",
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds);
        }
        else
        {
            fontPrintf(0, 0, colWhite, "%.2f <= %.2f <= %.2f",
                       (real32)(rndFrameCountMin) / RND_FrameRateSeconds,
                       (real32)(rndFrameRate) / RND_FrameRateSeconds,
                       (real32)(rndFrameCountMax) / RND_FrameRateSeconds);
        }
        y += 10;

        fontPrintf(0,y += 20,colRGB(255,255,0),"Channels in use:%d Guns:%d Ships:%d SFX:%d UI:%d",channelsinuse,numchans[0],numchans[1],numchans[2],numchans[3]);

    }
#endif//RND_FRAME_RATE

#ifdef COLLISION_CHECK_STATS
    if (rndDisplayCollStats)
    {
        fontPrintf(0,y += 20,colWhite, "Ship:%d  %d  Resources:%d  %d  Derelicts:%d %d  Total:%d  %d",shipshipwalks,shipshipchecks,
                   shipresourcewalks,shipresourcechecks,shipderelictwalks,shipderelictchecks,
                   shipshipwalks+shipresourcewalks+shipderelictwalks,shipshipchecks+shipresourcechecks+shipderelictchecks);

        fontPrintf(0,y += 20,colWhite, "Bullet:%d  %d  Missile:%d  %d  Mine:%d  %d  Total:%d  %d",
                   bulletwalks,bulletchecks,missilewalks,missilechecks,minewalks,minechecks,
                   bulletwalks+missilewalks+minewalks,bulletchecks+missilechecks+minechecks);

        fontPrintf(0,y += 20,colWhite, "Ships Avoiding Stuff:%d   Avoided Walks:%d   Avoided Checks:%d",
                   shipsavoidingstuff,shipsavoidedwalks,shipsavoidedchecks);
#ifdef BOB_STATS
        if (bobStats.statsValid)
        {
            sdword timeDuration = bobStats.timeDuration / (1000L);
            fontPrintf(0,y += 20,colWhite, "Blobtype %d Time: %d numPasses:%d numWalks: %d",
                       bobStats.subBlobs, timeDuration, bobStats.numPasses, bobStats.numWalks);
            fontPrintf(0,y += 20,colWhite,"numChecks:%d trivialRejects:%d InitialBlobs: %d FinalBlobs: %d",
                       bobStats.numChecks, bobStats.trivialRejects, bobStats.initialBlobs, bobStats.finalBlobs);
        }
#endif
#ifdef AISHIP_STATS
        aishipStatsPrint(&y);
#endif
    }
#endif
#if RND_POLY_STATS
    if (rndDisplayPolyStats)
    {                                                   //display frame rate
        fontPrint(0, MAIN_WindowHeight - fontHeight(rndPolyStatsString),
                  rndPolyStatsColor, rndPolyStatsString);
        rndPolyStatFrameCounter++;
#if MESH_MATERIAL_STATS
        fontPrint(MAIN_WindowWidth - fontWidth(meshMaterialStatsString),
                  MAIN_WindowHeight - fontHeight(rndPolyStatsString),
                  rndPolyStatsColor, meshMaterialStatsString);
#endif //MESH_MATERIAL_STATS
    }
#endif//RND_POLY_STATS
#if RND_XYZ
    if (rndXYZPrint && selSelected.numShips == 1)
    {
        static char string[256];
        if (selSelecting.numTargets == 1)
        {
            sprintf(string, "deltaXYZ = (%.2f, %.2f, %.2f)", selSelecting.TargetPtr[0]->posinfo.position.x - selSelected.ShipPtr[0]->posinfo.position.x, selSelecting.TargetPtr[0]->posinfo.position.y - selSelected.ShipPtr[0]->posinfo.position.y, selSelecting.TargetPtr[0]->posinfo.position.z - selSelected.ShipPtr[0]->posinfo.position.z);
        }
        else
        {
            sprintf(string, "XYZ = (%.2f, %.2f, %.2f)", selSelected.ShipPtr[0]->posinfo.position.x, selSelected.ShipPtr[0]->posinfo.position.y, selSelected.ShipPtr[0]->posinfo.position.z);
        }
        fontPrint(MAIN_WindowWidth - fontWidth(string), 0, colWhite, string);
    }
#endif
#if NIS_PRINT_INFO
    if (nisPrintInfo)
    {
        fontPrintf(MAIN_WindowWidth - fontWidth(nisInfoString) - 10,
                   MAIN_WindowHeight - fontHeight(nisInfoString) - 1,
                   colWhite, nisInfoString);
    }
#endif

#if 0
    if (selSelected.numShips == 1)
    {
        real32 overlap = rndComputeOverlap(selSelected.ShipPtr[0], 2.0f);
        fontPrintf(0, 0, colWhite, "overlap %0.3f     ", overlap);
    }
#endif
#if RND_SCALECAP_TWEAK
    if (RND_CAPSCALECAP_STATS)
    {
        if (selSelected.numShips == 1)
        {
            static Ship* ship;
            static shiptrail* trail;

            ship = selSelected.ShipPtr[0];
            trail = ship->trail[0];

            if (trail != NULL)
            {
                if (keyIsHit(RND_CapScaleCapKeyDec))
                {
                    //decrement
                    if (trail->scalecap == -1.0f)
                    {
                        trail->scalecap = 0.0f;
                    }
                    trail->scalecap -= rndCapScaleCapSlopeDelta;
                    if (trail->scalecap < 0.0f)
                    {
                        trail->scalecap = 0.0f;
                    }
                }
                if (keyIsHit(RND_CapScaleCapKeyInc))
                {
                    //increment
                    if (trail->scalecap == -1.0f)
                    {
                        trail->scalecap = 0.0f;
                    }
                    trail->scalecap += rndCapScaleCapSlopeDelta;
                }
            }
            fontPrint(0, MAIN_WindowHeight - fontHeight(rndCapScaleCapStatsString),
                      colWhite, rndCapScaleCapStatsString);
        }
    }
#endif
}
#endif

/*-----------------------------------------------------------------------------
    Name        : rndDrawScissorBars
    Description : Draw the scissor window bars top and bottom for when
                    scissoring is only partial or not supported.
    Inputs      : scissorEnabled - does the renderer support scissor windows?
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndDrawScissorBars(bool32 scissorEnabled)
{
    sdword y;
    real32 GLy, oneGLy;
    sdword wasAdditive;

    if (nisScissorFade == 0.0f && !scissorEnabled)
    {
        y = NIS_LetterHeight;
        GLy = primScreenToGLY(y);
        oneGLy = primScreenToGLY(MAIN_WindowHeight - y);
        glColor3f(0.0f, 0.0f, 0.0f);
        wasAdditive = rndAdditiveBlending;
    }
    else
    {
        y = (sdword)((real32)NIS_LetterHeight * nisScissorFade);
        GLy = primScreenToGLY(y);
        oneGLy = primScreenToGLY(MAIN_WindowHeight - y);
        dbgAssert(!nisFullyScissored || !scissorEnabled);
        dbgAssert((nisScissorFade > 0.0f && nisScissorFade <= 1.0f) || !scissorEnabled);
        glColor3f(MR_LetterboxGrey);
        glBegin(GL_QUADS);
        //top grey part
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(0));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(0));
        glVertex2f(primScreenToGLX(-1), GLy);
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), GLy);
        //bottom scissor part
        glVertex2f(primScreenToGLX(-1), oneGLy);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(MAIN_WindowHeight));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), oneGLy);
        glEnd();

        wasAdditive = rndAdditiveBlends(FALSE);
        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, nisScissorFade);
    }
    glBegin(GL_QUADS);
    //top scissor part
    glVertex2f(primScreenToGLX(-1), primScreenToGLY(0));
    glVertex2f(primScreenToGLX(-1), GLy);
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), GLy);
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(0));
    //bottom scissor part
    glVertex2f(primScreenToGLX(-1), oneGLy);
    glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight));
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(MAIN_WindowHeight));
    glVertex2f(primScreenToGLX(MAIN_WindowWidth), oneGLy);
    glEnd();
    glDisable(GL_BLEND);
    rndAdditiveBlends(wasAdditive);
}

void rndDisableTransitionOverlay(void)
{
    *(ubyte*)&memGrowthPool[7].lastSize[0] = 0;
    btgSetColourMultiplier(1, 1.0f);
}

int rndConfigureTransitionOverlay(int a1, char *a2)
{
    int result;
    float v3;
    void *v4;
    int v5, v6, v7, v8, v9, v10;

    *(ubyte*)&memGrowthPool[7].lastSize[0] = 0;
    btgSetColourMultiplier(1, 1.0f);

    if (*a2)
    {
        result = sscanf(a2, "%d,%d,%d,%d,%d,%d,%f,%f", &v7, &v6, &v5, &v10, &v9, &v8, &v4, &v3);
        v3 = (float)((double)v3 * RND_OVERLAY_ALPHA_CONV);
        if (result == 8)
        {
            memGrowthPool[8].wholePool = v4;
            *(float*)&memGrowthPool[8].pool = v3;
            memGrowthPool[8].heapLength = (ubyte)v7 | (((ubyte)v6 | ((v5 | 0xFFFFFF00) << 8)) << 8);
            result = (int)mainDerelictTexture;
            memGrowthPool[9].poolLength = 0;
            memGrowthPool[8].poolLength = (ubyte)v10 | (((ubyte)v9 | ((v8 | 0xFFFFFF00) << 8)) << 8);
            if (v3 <= 0.0f)
            {
                if (mainDerelictTexture)
                {
                    result = trTextureUnregister(mainDerelictTexture);
                    mainDerelictTexture = 0;
                }
                *(ubyte*)&memGrowthPool[7].lastSize[0] = 0;
            }
            else
            {
                if (!mainDerelictTexture)
                {
                    result = (int)trTextureRegister(aRaceDerelictDp, NULL, memGrowthPool[7].lastSize);
                    mainDerelictTexture = (trhandle)result;
                }
                *(ubyte*)&memGrowthPool[7].lastSize[0] = 1;
            }
        }
    }
    else
    {
        result = (int)mainDerelictTexture;
        if (mainDerelictTexture)
        {
            result = trTextureUnregister(mainDerelictTexture);
            mainDerelictTexture = 0;
        }
        memGrowthPool[9].poolLength = 0;
        memGrowthPool[8].wholePool = 0;
        memGrowthPool[8].pool = 0;
        memGrowthPool[8].heapLength = 0;
        memGrowthPool[8].poolLength = 0;
    }

    if (rndTransitionOverlayReconfigurePending)
        rndTransitionOverlayReconfigurePending = 0;

    return result;
}

void rndTransitionOverlayFadeTo(float a1, void *a2, void *a3, void *a4, void *a5)
{
    if (a1 <= 0.0f)
    {
        memGrowthPool[8].wholePool = a4;
        memGrowthPool[9].poolLength = 0;
        memGrowthPool[8].pool = (ubyte*)a5;
        memGrowthPool[8].heapLength = (sdword)(intptr_t)a2;
        memGrowthPool[8].poolLength = (sdword)(intptr_t)a3;
    }
    else
    {
        memGrowthPool[8].biggestBlock = (memcookie*)(intptr_t)memGrowthPool[8].heapLength;
        *(float*)&memGrowthPool[9].poolLength = universe.phystimeelapsed + a1;
        memGrowthPool[8].biggestBlockSize = (memcookie*)(intptr_t)memGrowthPool[8].poolLength;
        memGrowthPool[8].firstSize[0] = (memcookie*)memGrowthPool[8].wholePool;
        memGrowthPool[8].lastSize[0] = (memcookie*)memGrowthPool[8].pool;
        memGrowthPool[9].heapLength = FLOAT_AS_SDWORD(universe.phystimeelapsed);
        memGrowthPool[8].lastFree = (memcookie*)a4;
        memGrowthPool[8].last = (memcookie*)a5;
        memGrowthPool[8].first = (memcookie*)a2;
        memGrowthPool[8].firstFree = (memcookie*)a3;
    }
}

static real32 rndInterpolateChannel(double from, double to, double t)
{
    double v = (to - from) * t + from;
    if (v < 0.0) return 0.0f;
    if (v > 255.0) return 255.0f;
    return (real32)v;
}

void rndUpdateTransitionOverlay(void)
{
    double t;
    real32 r0, g0, b0, r1, g1, b1;
    real32 fromRate, fromAlpha, targetRate, targetAlpha, curRate, curAlpha;
    real32 btgMult;

    if (*(ubyte*)&memGrowthPool[7].lastSize[0])
    {
        if (*(float*)&memGrowthPool[9].poolLength != 0.0f)
        {
            if (universe.phystimeelapsed >= (double)*(float*)&memGrowthPool[9].poolLength)
            {
                memGrowthPool[8].wholePool = memGrowthPool[8].lastFree;
                memGrowthPool[8].heapLength = (sdword)(intptr_t)memGrowthPool[8].first;
                memGrowthPool[8].poolLength = (sdword)(intptr_t)memGrowthPool[8].firstFree;
                memGrowthPool[8].pool = (ubyte*)memGrowthPool[8].last;
                memGrowthPool[9].poolLength = 0;
                btgMult = (*(float*)&memGrowthPool[8].last == 0.0f) ? 1.0f : 0.0f;
                btgSetColourMultiplier(1, btgMult);
            }
            else
            {
                t = (universe.phystimeelapsed - *(float*)&memGrowthPool[9].heapLength)
                  / (*(float*)&memGrowthPool[9].poolLength - *(float*)&memGrowthPool[9].heapLength);

                curRate  = (*(float*)&memGrowthPool[8].lastFree - *(float*)&memGrowthPool[8].firstSize[0]) * t
                         + *(float*)&memGrowthPool[8].firstSize[0];
                curAlpha = (*(float*)&memGrowthPool[8].last - *(float*)&memGrowthPool[8].lastSize[0]) * t
                         + *(float*)&memGrowthPool[8].lastSize[0];
                *(float*)&memGrowthPool[8].wholePool = curRate;
                *(float*)&memGrowthPool[8].pool      = curAlpha;

                b0 = rndInterpolateChannel((double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].biggestBlock) >> 16),
                                           (double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].first) >> 16), t);
                g0 = rndInterpolateChannel((double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].biggestBlock) >> 8),
                                           (double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].first) >> 8), t);
                r0 = rndInterpolateChannel((double)(ubyte)(sdword)(intptr_t)memGrowthPool[8].biggestBlock,
                                           (double)(ubyte)(sdword)(intptr_t)memGrowthPool[8].first, t);
                memGrowthPool[8].heapLength = (ubyte)(sdword)r0 | ((ubyte)(sdword)g0 << 8) | ((ubyte)(sdword)b0 << 16);

                b1 = rndInterpolateChannel((double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].biggestBlockSize) >> 16),
                                           (double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].firstFree) >> 16), t);
                g1 = rndInterpolateChannel((double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].biggestBlockSize) >> 8),
                                           (double)(ubyte)(((sdword)(intptr_t)memGrowthPool[8].firstFree) >> 8), t);
                r1 = rndInterpolateChannel((double)(ubyte)(sdword)(intptr_t)memGrowthPool[8].biggestBlockSize,
                                           (double)(ubyte)(sdword)(intptr_t)memGrowthPool[8].firstFree, t);
                memGrowthPool[8].poolLength = (ubyte)(sdword)r1 | ((ubyte)(sdword)g1 << 8) | ((ubyte)(sdword)b1 << 16);

                targetAlpha = *(float*)&memGrowthPool[8].last;
                fromAlpha   = *(float*)&memGrowthPool[8].lastSize[0];
                real32 fromBtg = (fromAlpha == 0.0f) ? 1.0f : 0.0f;
                real32 toBtg   = (targetAlpha == 0.0f) ? 1.0f : 0.0f;
                btgSetColourMultiplier(1, (real32)((toBtg - fromBtg) * t + fromBtg));
            }
        }
    }
    else
    {
        btgSetColourMultiplier(1, 1.0f);
    }
}

void rndRenderTransitionOverlay(void)
{
    real32 alpha;
    real32 red, green, blue;
    real32 expansion, dt;
    sdword savedLighting, savedTexture2D;
    sdword savedTexEnv;
    BOOL savedCulling;
    int tile;

    if (!*(ubyte*)&memGrowthPool[7].lastSize[0])
        return;

    if (!RGLtype)
        return;

    if (!mainDerelictTexture || (gDevcaps2 & 0x200) != 0)
        return;

    alpha = *(float*)&memGrowthPool[8].pool * RND_OVERLAY_ALPHA_SCALE;
    if (alpha > 1.0f)
        alpha = 1.0f;
    if (alpha == 0.0f)
        return;

    if (rndTransitionOverlayTexcoordInitPending)
    {
        rndTransitionOverlayTexcoordTBits[0] = RND_OVERLAY_INIT_T;
        rndTransitionOverlayTexcoordSBits[0] = 0;
        rndTransitionOverlayTexcoordTBits[1] = RND_OVERLAY_INIT_T;
        rndTransitionOverlayTexcoordSBits[1] = 0;
        for (tile = 0; tile < 2; tile++)
        {
            rndTransitionOverlayScrollS[tile] = (tile & 1) ? RND_OVERLAY_SCROLL_S1 : RND_OVERLAY_SCROLL_S0;
            rndTransitionOverlayScrollT[tile] = (tile & 1) ? RND_OVERLAY_SCROLL_T1 : RND_OVERLAY_SCROLL_T0;
            rndTransitionOverlayTexcoordScale[tile] = (tile & 1) ? RND_OVERLAY_SCALE1 : RND_OVERLAY_SCALE0;
        }
        rndTransitionOverlayTexcoordInitPending = 0;
    }
    else
    {
        dt = universe.phystimeelapsed - rndTransitionOverlayLastUpdateTime;
        rndTransitionOverlayLastUpdateTime = universe.phystimeelapsed;
        expansion = *(float*)&memGrowthPool[8].wholePool * dt;
        for (tile = 0; tile < 2; tile++)
        {
            real32 newS = expansion * rndTransitionOverlayScrollS[tile] + *(real32*)&rndTransitionOverlayTexcoordSBits[tile];
            *(real32*)&rndTransitionOverlayTexcoordSBits[tile] = newS;
            real32 newT = expansion * rndTransitionOverlayScrollT[tile] + *(real32*)&rndTransitionOverlayTexcoordTBits[tile];
            *(real32*)&rndTransitionOverlayTexcoordTBits[tile] = newT;
            if (newS > 100.0f)
            {
                *(real32*)&rndTransitionOverlayTexcoordSBits[tile] = newS - 100.0f;
                *(real32*)&rndTransitionOverlayTexcoordTBits[tile] = newT - 100.0f;
            }
        }
    }

    if (!primModeEnabled)
        primModeSetFunction2();

    trMakeCurrent(mainDerelictTexture);
    trStructureGet((unsigned short)mainDerelictTexture);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    if (rndAdditiveBlending != 1)
    {
        rndAdditiveBlending = 1;
        if ((gDevcaps2 & 2) != 0)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    glDepthMask(0);

    savedTexEnv = rndTextureEnviron;
    if (rndTextureEnviron)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, rndTextureModeTable[0]);
        rndTextureEnviron = 0;
    }

    savedCulling = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    red   = (real32)((ubyte)memGrowthPool[8].heapLength)         * alpha;
    green = (real32)((ubyte)(memGrowthPool[8].heapLength >> 8))  * alpha;
    blue  = (real32)((ubyte)(memGrowthPool[8].heapLength >> 16)) * alpha;
    red   = max(0.0f, min(255.0f, red));
    green = max(0.0f, min(255.0f, green));
    blue  = max(0.0f, min(255.0f, blue));

    savedLighting = rndLightingEnabled;
    if (rndLightingEnabled)
    {
        glDisable(GL_LIGHTING);
        rndLightingEnabled = 0;
    }

    savedTexture2D = rndTexture2DEnabled;
    if (!rndTexture2DEnabled)
    {
        glEnable(GL_TEXTURE_2D);
        rndTexture2DEnabled = 1;
    }

    glColor3f(red * RND_OVERLAY_COLOR_SCALE, green * RND_OVERLAY_COLOR_SCALE, blue * RND_OVERLAY_COLOR_SCALE);

    for (tile = 0; tile < 2; tile++)
    {
        real32 S      = *(real32*)&rndTransitionOverlayTexcoordSBits[tile];
        real32 T      = *(real32*)&rndTransitionOverlayTexcoordTBits[tile];
        real32 scale  = rndTransitionOverlayTexcoordScale[tile];
        real32 top    = (real32)(1.0 - (RND_LENSFLARE_SCREEN_BIAS / (double)MAIN_WindowHeight + RND_LENSFLARE_SCREEN_BIAS / (double)MAIN_WindowHeight));
        real32 bottom = (real32)(1.0 - (((double)MAIN_WindowHeight + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowHeight
                                      + ((double)MAIN_WindowHeight + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowHeight));
        real32 left   = (real32)(RND_LENSFLARE_SCREEN_BIAS / (double)MAIN_WindowWidth + RND_LENSFLARE_SCREEN_BIAS / (double)MAIN_WindowWidth - 1.0);
        real32 right  = (real32)(((double)MAIN_WindowWidth + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowWidth
                                + ((double)MAIN_WindowWidth + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowWidth - 1.0);

        glBegin(GL_QUADS);
        glTexCoord2f(S, T);
        glVertex2f(left, top);

        glTexCoord2f(S, T + scale);
        glVertex2f(left, bottom);

        glTexCoord2f(S + scale, T + scale);
        glVertex2f(right, bottom);

        glTexCoord2f(S + scale, T);
        glVertex2f(right, top);
        glEnd();
    }

    glDisable(GL_BLEND);
    if (rndAdditiveBlending)
    {
        rndAdditiveBlending = 0;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (rndTextureEnviron != savedTexEnv)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, rndTextureModeTable[savedTexEnv]);
        rndTextureEnviron = savedTexEnv;
    }

    if (savedCulling)
        glEnable(GL_CULL_FACE);

    glDepthMask(1);

    if (primModeEnabled)
        primModeClearFunction2();

    if (savedLighting)
    {
        if (!rndLightingEnabled)
        {
            glEnable(GL_LIGHTING);
            rndLightingEnabled = 1;
        }
    }
    else if (rndLightingEnabled)
    {
        glDisable(GL_LIGHTING);
        rndLightingEnabled = 0;
    }

    if (savedTexture2D)
    {
        if (!rndTexture2DEnabled)
        {
            glEnable(GL_TEXTURE_2D);
            rndTexture2DEnabled = 1;
        }
    }
    else if (rndTexture2DEnabled)
    {
        glDisable(GL_TEXTURE_2D);
        rndTexture2DEnabled = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndRenderTask
    Description : Main render task
    Inputs      : void
    Outputs     : Clears frame buffer, renders regions, draws mouse, swaps buffers.
    Return      : void
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void rndRenderTask(void)
{
    static bool32 shouldSwap;
    static sdword index;
#ifdef PROFILE_TIMERS
    static sdword y;
#endif
    taskYield(0);

    while (1)
    {
        taskStackSaveCond(0);
        primErrorMessagePrint();

        speechEventUpdate();
        subTitlesUpdate();

#if RND_GL_STATE_DEBUG
        if (keyIsHit(GKEY) && keyIsStuck(LKEY))
        {                                                   //if starting a new round of state saving
            static FILE *f;
            keyClearSticky(LKEY);
            rndGLStateSaving = TRUE;
            if (rndGLStateLogIndex < 10)
            {
                sprintf(rndGLStateLogFileName, "gl0%d.state", rndGLStateLogIndex);
            }
            else
            {
                sprintf(rndGLStateLogFileName, "gl%d.state", rndGLStateLogIndex);
            }
            rndGLStateLogIndex++;
            f = fopen(rndGLStateLogFileName, "wt");         //open the file to clean it out
            if (f != NULL)
            {
                fclose(f);                                  //close the file
            }
        }
#endif
        glColor3ub(colRed(RND_StarColor), colGreen(RND_StarColor), colBlue(RND_StarColor));
        shouldSwap = feShouldSaveMouseCursor();
        if (shouldSwap)
        {
            if (RGL)
            {
                rglFeature(RGL_SAVEBUFFER_ON);
                if (mainRasterSkip)
                {
                    rglFeature(RGL_NOSKIP_RASTER);
                }
            }
        }
        else
        {
            if (RGL)
            {
                rglFeature(RGL_SAVEBUFFER_OFF);
            }
#if BINK
            if (binkDonePlaying)
#endif
            {
                if (RGL)
                {
                    if (mainRasterSkip && gameIsRunning)
                    {
                        rglFeature(RGL_SKIP_RASTER);
                    }
                    else
                    {
                        rglFeature(RGL_NOSKIP_RASTER);
                    }
                }
                if (lmActive)
                {
                    rndClearToBlack();
                    glClear(GL_DEPTH_BUFFER_BIT);
                }
                else
                {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the buffers
                }
            }
        }

        rndScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);   //can we do scissoring?
        primErrorMessagePrint();
        //default rendering scheme is primitives on. any
        //functions which want it off should set it back on when done
        rndPostObjectCallbackExecute();
        primErrorMessagePrint();
        rndFrameCount++;                                    //update frame count
        //draw partial scissor window, if applicable
        if (nisScissorFade != 0.0f || (nisFullyScissored && !rndScissorEnabled))
        {
            rndDrawScissorBars(rndScissorEnabled);
        }
        glDisable(GL_SCISSOR_TEST);                         //in case the scene was rendered with scissoring, turn it off properlu.


        /* need to update audio event layer */
        soundEventUpdate();

        //draw the subtitle text.  It's done here because it has to draw over everything
        if (singlePlayerGame)
        {
            for (index = 0; index < SUB_NumberRegions; index++)
            {
                if ((!tutPointersDrawnThisFrame) && index == STR_LetterboxBar)
                {                                       //if this is the subtitle region
                    tutDrawTextPointers(&subRegion[STR_LetterboxBar].rect);//draw any active pointers there may be
                }
                if (subRegion[index].bEnabled && subRegion[index].cardIndex > 0)
                {
                    if (index == STR_NIS)
                    {
                        subTimeElapsed = &thisNisPlaying->timeElapsed;
                    }
                    else
                    {
                        subTimeElapsed = &universe.totaltimeelapsed;
                    }
                    subTitlesDraw(&subRegion[index]);
                }
            }
        }

        rndDrawOnScreenSyncStatus();        // always print sync status, even in HW_Release mode

#if RND_VERBOSE_LEVEL >= 1
        rndDrawOnScreenDebugInfo();                         //draw a bunch of debugging crapola
#endif
#ifdef PROFILE_TIMERS
        y = 300;
        if (rndDisplayCollStats) profTimerStatsPrint(&y);        // compiled out automatically if PROFILE_TIMERS not defined
#endif
        // set the cursor type, then draw the mouse cursor
        mouseSetCursorSetting();
        shouldSwap = feShouldSaveMouseCursor();
        if (shouldSwap)
        {
            mouseStoreCursorUnder();
        }
        if (!glcActive())
        {
            mouseDraw();                                        //draw mouse atop everything
        }

        if (universePause
#if RND_PLUG_DISABLEABLE
            && rndShamelessPlugEnabled
#endif
           )
        {
#if 0
            if (singlePlayerGame)
            {
                static sdword intelliCount;
                static Node*  intelliNode;

                //count the number of popped up popups
                intelliCount = 0;
                intelliNode  = poFleetIntelligence.head;
                while (intelliNode != NULL)
                {
                    FleetIntelligence* inode = (FleetIntelligence*)listGetStructOfNode(intelliNode);
                    if (inode->showOnce)
                    {
                        intelliCount++;
                    }
                    intelliNode = intelliNode->next;
                }
                //don't display plug if a popup has paused the game
                if (intelliCount == 0)
                {
                    rndShamelessPlug();
                }
            }
            else
#endif
            {
                rndShamelessPlug();
            }
        }

        //take a screenshot or sequence thereof
#if SS_SCREENSHOTS
        if (keyIsStuck(SCROLLKEY))
        {
            keyClearSticky(SCROLLKEY);
#if MAIN_Password
            if (mainScreenShotsEnabled)
#endif //MAIN_Password
            {
                rndTakeScreenshot = TRUE;
            }
        }
        else if (keyIsStuck(PAUSEKEY))
        {
            keyClearSticky(PAUSEKEY);
            if (RGL)
            {
#if SS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nMulti-shot end.");
#endif
                rglFeature(RGL_MULTISHOT_END);
            }
        }
        if (rndTakeScreenshot)
        {
            static ubyte* buf;

            rndTakeScreenshot = FALSE;
            //buf = (ubyte*)malloc(3*MAIN_WindowWidth*MAIN_WindowHeight);
            buf = (void *)VirtualAlloc(NULL, 3*MAIN_WindowWidth*MAIN_WindowHeight, MEM_COMMIT, PAGE_READWRITE);
            if (buf != NULL)
            {
                bool32 result;
                glReadPixels(0, 0, MAIN_WindowWidth, MAIN_WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, buf);
                ssSaveScreenshot(buf);
                //free(buf);
                result = VirtualFree(buf, 0, MEM_RELEASE);
                dbgAssert(result);
            }
        }
#endif //SS_SCREENSHOTS

        if (rndFillCounter)
        {
            static rectangle r;
            r.x0 = 0;
            r.y0 = 0;
            r.x1 = MAIN_WindowWidth - 1;
            r.y1 = MAIN_WindowHeight - 1;
            primRectSolid2(&r, rndFillColour);
            rndFillCounter--;
        }
#if 0
#if defined (Downloadable) || defined(DLPublicBeta)
        ;
#else
        {
            extern bool32 mainPlayAVIs;

            if (mainPlayAVIs)
            {
                mainPlayAVIs = FALSE;
                animSmackPlay(-1, (sdword)"Movies\\sierra.smk");
                animSmackPlay(-1, (sdword)"Movies\\relicintro.smk");
            }
        }
#endif
#endif
        if (mainReinitRenderer > 0)
        {
            mainReinitRenderer--;
            if (mainReinitRenderer == 0)
            {
                mainReinitRGL();
                trNoPalResizePool(opNoPalMB);
            }
        }
        else
        {
            if (animaticJustPlayed > 0)
            {
                animaticJustPlayed--;
            }
            else if (!feDontFlush)
            {
                if (glCapFeatureExists(GL_SWAPFRIENDLY))
                {
                    if (glcActive())
                    {
                        if (RGLtype == SWtype)
                        {
                            (void)glcActivate(FALSE);
                        }
                        else
                        {
                            glcPageFlip(FALSE);
                        }
                    }
                }
                rndFlush();
            }
            feDontFlush = FALSE;
        }
        primErrorMessagePrint();

        if (shouldSwap)
        {
            mouseRestoreCursorUnder();
        }
        primErrorMessagePrint();

#if RND_GL_STATE_DEBUG
        rndGLStateSaving = FALSE;                           //done saving the file for now
#endif //RND_GL_STATE_DEBUG
        tutPointersDrawnThisFrame = FALSE;
        taskStackRestoreCond();
        taskYield(0);                                       //hold off to next frame
    }
    taskExit();
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : rndLightingEnable
    Description : Enable or disable back-face culling
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : just calls glEnable/glDisable
    Return      : void
----------------------------------------------------------------------------*/
sdword rndLightingEnable(sdword bEnable)
{
    sdword oldStatus = rndLightingEnabled;

    if (bEnable)
    {
        if (!rndLightingEnabled)
        {
            glEnable(GL_LIGHTING);
            rndLightingEnabled = TRUE;
        }
    }
    else
    {
        if (rndLightingEnabled)
        {
            glDisable(GL_LIGHTING);
            rndLightingEnabled = FALSE;
        }
    }

    return oldStatus;
}

/*-----------------------------------------------------------------------------
    Name        : rndBackFaceCullEnable
    Description : Enable or disable back-face culling
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : just calls glEnable/glDisable
    Return      : void
----------------------------------------------------------------------------*/
void rndBackFaceCullEnable(sdword bEnable)
{
    if (bEnable)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndTextureEnable
    Description : Enable or disable texturing
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : toggles rndTexture2DEnabled
    Return      : previous enabled status
----------------------------------------------------------------------------*/
sdword rndTextureEnable(sdword bEnable)
{
    sdword enabled = rndTexture2DEnabled;
    if (bEnable)
    {
        if (!rndTexture2DEnabled)
        {
            glEnable(GL_TEXTURE_2D);
            rndTexture2DEnabled = TRUE;
        }
    }
    else
    {
        if (rndTexture2DEnabled)
        {
            glDisable(GL_TEXTURE_2D);
            rndTexture2DEnabled = FALSE;
        }
    }
    return(enabled);
}

/*-----------------------------------------------------------------------------
    Name        : rndTextureEnvironment
    Description : changes the current texture environment
    Inputs      : mode - one of RTE_Modulate, RTE_Replace, RTE_Decal, RTE_Blend
    Outputs     : changes the current texture environment mode
    Return      : old mode
----------------------------------------------------------------------------*/
sdword rndTextureModeTable[] =
{
    GL_MODULATE, GL_REPLACE, GL_DECAL, GL_BLEND
};
sdword rndTextureEnvironment(sdword mode)
{
    sdword oldMode = rndTextureEnviron;
    if (rndTextureEnviron != mode)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, rndTextureModeTable[mode]);
        rndTextureEnviron = mode;
    }
    return(oldMode);
}

/*-----------------------------------------------------------------------------
    Name        : rndPerspectiveCorrection
    Description : provides a hint to the GL as to whether to perspectively correct textures
    Inputs      : bEnable - TRUE enables (NICEST), FALSE disables (FASTEST)
    Outputs     : may toggle rndPerspectiveCorrect
    Return      : previous status
----------------------------------------------------------------------------*/
sdword rndPerspectiveCorrection(sdword bEnable)
{
    sdword oldStatus = rndPerspectiveCorrect;
#if USE_RND_HINT
    if (rndHint > 0)
    {
        if (rndHint == 1)
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
            rndPerspectiveCorrect = TRUE;
        }
        else
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
            rndPerspectiveCorrect = FALSE;
        }
        return(oldStatus);
    }
#endif
    if (mainNoPerspective)
    {
        if (RGL) glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        return(oldStatus);
    }
    if (bEnable)
    {
        if (!rndPerspectiveCorrect)
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
            rndPerspectiveCorrect = TRUE;
        }
    }
    else
    {
        if (rndPerspectiveCorrect)
        {
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
            rndPerspectiveCorrect = FALSE;
        }
    }
    return(oldStatus);
}

/*-----------------------------------------------------------------------------
    Name        : rndNormalizeEnable
    Description : enables or disables normalization
    Inputs      : bEnable - TRUE enables, FALSE disables
    Outputs     : toggles rndNormalization
    Return      : previous status
----------------------------------------------------------------------------*/
sdword rndNormalizeEnable(sdword bEnable)
{
    sdword oldStatus = rndNormalization;
    if (bEnable)
    {
        glEnable(GL_NORMALIZE);
        rndNormalization = TRUE;
    }
    else
    {
        glDisable(GL_NORMALIZE);
        rndNormalization = FALSE;
    }
    return(oldStatus);
}

/*-----------------------------------------------------------------------------
    Name        : rndHintInc
    Description : changes the glHint mode of operation
    Inputs      :
    Outputs     : alters rndHint
    Return      :
----------------------------------------------------------------------------*/
void rndHintInc()
{
#if USE_RND_HINT
    rndHint++;
    if (rndHint >= 3) rndHint = 0;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : rndAdditiveBlends
    Description : enables or disables additive blending mode
    Inputs      : bAdditive - TRUE or FALSE
    Outputs     :
    Return      : previous status
----------------------------------------------------------------------------*/
sdword rndAdditiveBlends(sdword bAdditive)
{
    sdword oldStatus;
    extern udword gDevcaps2;

    oldStatus = rndAdditiveBlending;
    if (bAdditive != rndAdditiveBlending)
    {
        rndAdditiveBlending = bAdditive;
        if (bAdditive)
        {
            if (bitTest(gDevcaps2, DEVSTAT2_NO_ADDITIVE))
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
        }
        else
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    return(oldStatus);
}

/*-----------------------------------------------------------------------------
    Name        : rndMaterialfv
    Description : set GL material params
    Inputs      : face - GL_FRONT or GL_FRONT_AND_BACK
                  pname - GL_AMBIENT or GL_DIFFUSE
                  params - RGBA [0..1]
    Outputs     :
    Return      : TRUE if material was updated, FALSE if same as last
----------------------------------------------------------------------------*/
#define V4_EQUAL(A, B) (A[0] != B[0] || A[1] != B[1] || A[2] != B[2] || A[3] != B[3]) ? FALSE : TRUE
#define V4_SET(D, S) \
    { \
        D[0] = S[0]; \
        D[1] = S[1]; \
        D[2] = S[2]; \
        D[3] = S[3]; \
    }
sdword rndMaterialfv(sdword face, sdword pname, real32* params)
{
    static real32 ambient[2][4] = {{-1.0f, -1.0f, -1.0f, -1.0f},{-1.0f, -1.0f, -1.0f, -1.0f}};
    static real32 diffuse[2][4] = {{-1.0f, -1.0f, -1.0f, -1.0f},{-1.0f, -1.0f, -1.0f, -1.0f}};

    if (face == -1)
    {
        real32 reset[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
        V4_SET(ambient[0], reset);
        V4_SET(ambient[1], reset);
        V4_SET(diffuse[0], reset);
        V4_SET(diffuse[1], reset);
        return TRUE;
    }

    if (face == GL_FRONT)
    {
        if (pname == GL_AMBIENT)
        {
            if (!V4_EQUAL(ambient[0], params))
            {
                V4_SET(ambient[0], params);
                glMaterialfv(face, pname, params);
                return TRUE;
            }
        }
        else
        {
            if (!V4_EQUAL(diffuse[0], params))
            {
                V4_SET(diffuse[0], params);
                glMaterialfv(face, pname, params);
                return TRUE;
            }
        }
    }

    return FALSE;
}


#define vcSub(a,b,c) \
    (a)##->x = (b)##->x - (c)##->x; \
    (a)##->y = (b)##->y - (c)##->y; \
    (a)##->z = (b)##->z - (c)##->z

#define vcCrossProduct(result,a,b) \
    (result)##->x = ((a)##->y * (b)##->z) - ((a)##->z * (b)##->y); \
    (result)##->y = ((a)##->z * (b)##->x) - ((a)##->x * (b)##->z); \
    (result)##->z = ((a)##->x * (b)##->y) - ((a)##->y * (b)##->x)

#define vcAddToScalarMultiply(dstvec,vec,k) \
    (dstvec)##->x += ((vec)##->x * (k));       \
    (dstvec)##->y += ((vec)##->y * (k));       \
    (dstvec)##->z += ((vec)##->z * (k))

#define vcAddToScalarMultiply(dstvec,vec,k) \
    (dstvec)##->x += ((vec)##->x * (k));       \
    (dstvec)##->y += ((vec)##->y * (k));       \
    (dstvec)##->z += ((vec)##->z * (k))

#define vcScalarMultiply(dstvec,vec,k) \
    (dstvec)##->x = (vec)##->x * (k);       \
    (dstvec)##->y = (vec)##->y * (k);       \
    (dstvec)##->z = (vec)##->z * (k)



#define vcDotProduct(a,b) \
    ( ((a)##->x * (b)##->x) + ((a)##->y * (b)##->y) + ((a)##->z * (b)##->z) )


/*-----------------------------------------------------------------------------
    Name        : rndEnvironmentMap
    Description : calculates parameters for rendering a reflective poly
    Inputs      : vector camera : location of camera
                  vectors A,B,C : vertices of poly


    Outputs     : vectors U,V,W : corners on texture map (.x and .y only!)
    Return      :
----------------------------------------------------------------------------*/

void rndEnvironmentMap(vector* camera, vector* A, vector* B, vector* C,
                       vector* U, vector* V, vector* W)
{
    vector v1,v2,normal;
    real32 length;

    //get normal of triangle
    vcSub(&v1,B,A);
    vcSub(&v2,C,A);
    vcCrossProduct(&normal,&v1,&v2);
    vecNormalizeToLength(&normal, (real32)1.0f);

    //vertex A:

    vcSub(&v1,A,camera);        //v1 is incoming ray
    length = vcDotProduct(&v1,&normal);
    vcAddToScalarMultiply(&v1,&normal,2*length);  //possibly "-2*length"

    //v1 is now reflected ray
    U->x = atan(v1.y/v1.x);                             //azimuth
    U->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex B:

    vcSub(&v1,B,camera);
    length = vcDotProduct(&v1,&normal);
    vcAddToScalarMultiply(&v1,&normal,2*length);  //possibly "-2*length"

    V->x = atan(v1.y/v1.x);                             //azimuth
    V->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex C:

    vcSub(&v1,C,camera);
    length = vcDotProduct(&v1,&normal);
    vcAddToScalarMultiply(&v1,&normal,2*length);  //possibly "-2*length"

    W->x = atan(v1.y/v1.x);                             //azimuth
    W->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;

}

/*-----------------------------------------------------------------------------
    Name        : rndEnvironmentMapConvex
    Description : calculates parameters for rendering a reflective poly with
                    a convex or concave surface
    Inputs      : vector camera : location of camera
                  vectors A,B,C : vertices of poly
                  convex: curvature of surface
                        convex < 1 : concave
                        convex = 1 : flat
                        convex > 1 : convex


    Outputs     : vectors U,V,W : corners on texture map (.x and .y only!)
    Return      :
----------------------------------------------------------------------------*/

void rndEnvironmentMapConvex(vector* camera, vector* A, vector* B, vector* C,
                       real32 convex, vector* U, vector* V, vector* W)
{
    vector v1,v2,normal;
    real32 length;

    //get normal of triangle
    vcSub(&v1,B,A);
    vcSub(&v2,C,A);
    vcCrossProduct(&normal,&v1,&v2);
    vecNormalizeToLength(&normal, (real32)1.0f);

    //vertex A:

    vcSub(&v1,A,camera);        //v1 is incoming ray
    length = vcDotProduct(&v1,&normal);
    vcScalarMultiply(&v2,&normal,length);
    vcAddToScalarMultiply(&v1,&v2,1+convex);

    //v1 is now reflected ray
    U->x = atan(v1.y/v1.x);                             //azimuth
    U->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex B:

    vcSub(&v1,B,camera);
    length = vcDotProduct(&v1,&normal);
    vcScalarMultiply(&v2,&normal,2*length);
    vcAddToScalarMultiply(&v1,&v2,1+convex);

    V->x = atan(v1.y/v1.x);                             //azimuth
    V->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;


    //vertex C:

    vcSub(&v1,C,camera);
    length = vcDotProduct(&v1,&normal);
    vcScalarMultiply(&v2,&normal,2*length);
    vcAddToScalarMultiply(&v1,&v2,1+convex);

    W->x = atan(v1.y/v1.x);                             //azimuth
    W->y = atan(v1.z/(sqrt(v1.x*v1.x + v1.y*v1.y)));    //elevation;



}

/*-----------------------------------------------------------------------------
    Name        : rndSetScreenFill
    Description : set a flag to fill the screen entirely with a rectangle of
                  specified colour immediately before the next Flush
    Inputs      : count - number of frames for fill to occur
                  c - color to fill screen with
    Outputs     : globals are set
    Return      :
-----------------------------------------------------------------------------*/
void rndSetScreenFill(sdword count, color c)
{
    rndFillCounter = count;
    rndFillColour = c;
}

/*-----------------------------------------------------------------------------
    Name        : rndSetClearColor
    Description : set the background clear color, if such a thing is supported by
                  underlying GL
    Inputs      : c - the color to set
    Outputs     : GL_COLOR_CLEAR_VALUE is perhaps modified
    Return      :
----------------------------------------------------------------------------*/
void rndSetClearColor(color c)
{
    if (glCapFeatureExists(GL_COLOR_CLEAR_VALUE))
    {
        glClearColor(colReal32(colRed(c)),
                     colReal32(colGreen(c)),
                     colReal32(colBlue(c)),
                     1.0f);
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndResetGLState
    Description : reset the GL's state after switching renderers
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndResetGLState(void)
{
    trClearCurrent();

    rndMaterialfv(-1, 0, NULL);

    if (rndAdditiveBlending)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    else
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    rndAdditiveBlends(rndAdditiveBlending);

    if (rndNormalization)
    {
        glEnable(GL_NORMALIZE);
    }
    else
    {
        glDisable(GL_NORMALIZE);
    }
    rndNormalizeEnable(rndNormalization);

    if (rndPerspectiveCorrect)
    {
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }
    else
    {
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    }
    rndPerspectiveCorrection(rndPerspectiveCorrect);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, RTE_Modulate);
    rndTextureEnviron = RTE_Modulate;

    if (rndTexture2DEnabled)
    {
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
    rndTextureEnable(rndTexture2DEnabled);

    rndBackFaceCullEnable(TRUE);

    if (rndLightingEnabled)
    {
        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }
    rndLightingEnable(rndLightingEnabled);

    rndSetClearColor(colBlack);
}

/*-----------------------------------------------------------------------------
    Name        : rndComputeOverlap
    Description : compute amount of overlap of collision spheres wrt ships in renderlist
    Inputs      : ship - ship to test
                  scalar - multiplier for strike craft collision radii
    Outputs     :
    Return      : [0..1] amount of obscuring that's going on
----------------------------------------------------------------------------*/
real32 rndComputeOverlap(Ship* ship, real32 scalar)
{
    Node* objnode;
    SpaceObj* spaceobj;
    ShipStaticInfo* shipStaticInfo;
    Ship* testship;
    vector vobj;
    real32 overlap;
    CollInfo* acoll;
    CollInfo* bcoll;
    rectangle arect, brect, crect;
    real32 aarea, carea;
    real32 ratio;
    real32 radius;

    //looking for our ship
    objnode = universe.RenderList.head;
    while (objnode != NULL)
    {
        testship = (Ship*)listGetStructOfNode(objnode);
        if (testship == ship)
        {
            break;
        }
        objnode = objnode->next;
    }

    if (objnode == NULL)
    {
        //couldn't find it
        return 0.0f;
    }

    //move backwards computing overlap
//    objnode = objnode->prev;
    objnode = objnode->next;
    overlap = 0.0f;
    //make screenspace rectangle
    acoll = &ship->collInfo;
    radius = scalar * acoll->selCircleRadius;
    arect.x0 = primGLToScreenX(acoll->selCircleX - radius);
    arect.x1 = primGLToScreenX(acoll->selCircleX + radius);
    arect.y0 = primGLToScreenY(acoll->selCircleY + radius);
    arect.y1 = primGLToScreenY(acoll->selCircleY - radius);
    aarea = (real32)(2 * (arect.x1 - arect.x0) +
                     2 * (arect.y1 - arect.y0));
    while (objnode != NULL)
    {
        spaceobj = (SpaceObj*)listGetStructOfNode(objnode);
        if (spaceobj->objtype == OBJ_ShipType)
        {
            //useful stuff
            testship = (Ship*)spaceobj;
            shipStaticInfo = (ShipStaticInfo*)spaceobj->staticinfo;

            if (ship->staticinfo->shipclass <= CLASS_Frigate)
            {
                if (testship->staticinfo->shipclass > CLASS_Frigate)
                {
                    // don't calculate strikecraft overlapping capital ships
                    goto nextobj;
                }
            }
#if 0
            //not sure what to do about this yet
            if (ship->collInfo.precise != NULL)
            {
                __asm int 3 ;
            }
#endif

            //position in cameraspace
            vecSub(vobj, mrCamera->eyeposition, spaceobj->posinfo.position);

            //make screenspace rectangle
            bcoll = &testship->collInfo;
            radius = scalar * bcoll->selCircleRadius;
            brect.x0 = primGLToScreenX(bcoll->selCircleX - radius);
            brect.x1 = primGLToScreenX(bcoll->selCircleX + radius);
            brect.y0 = primGLToScreenY(bcoll->selCircleY + radius);
            brect.y1 = primGLToScreenY(bcoll->selCircleY - radius);

            //find union of ab
            primRectUnion2(&crect, &arect, &brect);
            if (crect.x0 == crect.x1 || crect.y0 == crect.y1)
            {
                carea = 0.0f;
            }
            else
            {
                carea = (real32)(2 * (crect.x1 - crect.x0) +
                                 2 * (crect.y1 - crect.y0));
            }

            //compute overlap
            ratio = carea / aarea;
            overlap += ratio;
            if (overlap >= 1.0f)
            {
                overlap = 1.0f;
                break;
            }
        }

//        objnode = objnode->prev;
nextobj:
        objnode = objnode->next;
    }

    return overlap;
}

/*-----------------------------------------------------------------------------
    Name        : rndClearToBlack
    Description : clear the screen to black, guaranteed
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndClearToBlack(void)
{
    real32 bgcolor[4];

    if (glCapFeatureExists(GL_COLOR_CLEAR_VALUE))
    {
        //get current clearcolor
        glGetFloatv(GL_COLOR_CLEAR_VALUE, (GLfloat*)bgcolor);
        //set clearcolor to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        //reset clearcolor
        glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
    }
    else
    {
        //clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndAllClearToBlack
    Description : clear all buffers to black
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndAllClearToBlack(void)
{
    real32 bgcolor[4];

    if (glCapFeatureExists(GL_COLOR_CLEAR_VALUE))
    {
        //get current clearcolor
        glGetFloatv(GL_COLOR_CLEAR_VALUE, (GLfloat*)bgcolor);
        //set clearcolor to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //clear the screen
        rndClear();
        //reset clearcolor
        glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
    }
    else
    {
        //clear the screen
        rndClear();
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndClear
    Description : clear front and back buffers
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndClear(void)
{
    sdword i;

    for (i = 0; i < 3; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        rndFlush();
    }

    if (RGLtype == SWtype)
    {
        rglFeature(RGL_SPEEDY);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rndFlush
    Description : flush the GL and swap buffers if necessary
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rndFlush(void)
{
    glFlush();
    primErrorMessagePrint();
    if (!RGL)
    {
        if (rwglSwapBuffers == NULL || glNT)
        {
            SwapBuffers(hGLDeviceContext);
        }
        else
        {
            rwglSwapBuffers(hGLDeviceContext);
        }
    }
    Sleep(1);
}

#if 0  // Cat3: rndUpdateLensflareSource
/*=== rndUpdateLensflareSource (0x57BAD0) ===*/
void __cdecl rndUpdateLensflareSource(vector *location)
{
int v2; // ecx
int v3; // ebp
int v4; // edx
char v5; // al
double v6; // st7
Node *i; // edi
struct SpaceObj *structptr; // ecx
int v9; // eax
_DWORD *v10; // eax
Node *head; // edi
double v12; // st7
double x; // st7
char v15; // c0
double y; // st7
char v18; // c0
long double v19; // st7
double v20; // st7
int v21; // [esp+10h] [ebp-28h]
int v22; // [esp+10h] [ebp-28h]
int hitFace; // [esp+14h] [ebp-24h] BYREF
real32 destRadius; // [esp+18h] [ebp-20h] BYREF
vector destX; // [esp+1Ch] [ebp-1Ch] BYREF
vector a; // [esp+2Ch] [ebp-Ch] BYREF
float locationa; // [esp+3Ch] [ebp+4h]
if ( !LOBYTE(location[1].y) )
{
LOBYTE(location[1].z) = 0;
return;
}
LOBYTE(location[1].z) = 1;
destX.x = mrCamera_->eyeposition.x - location->x;
destX.y = mrCamera_->eyeposition.y - location->y;
destX.z = mrCamera_->eyeposition.z - location->z;
locationa = getVectDistSloppy(destX);
if ( dword_A75DC8 >= 8 )
{
v2 = 1;
v3 = 0;
if ( dword_A75DC8 > 1 )
{
v22 = 0;
while ( 1 )
{
v4 = dword_B37820[v2];
if ( !v4 || !*(_BYTE *)(v4 + 20) )
break;
if ( *(float *)(v4 + 32) > (double)*(float *)(dword_B37820[v22] + 32) && locationa < (double)*(float *)(v4 + 32) )
{
v3 = v2;
v22 = v2;
}
if ( ++v2 >= dword_A75DC8 )
goto LABEL_15;
}
v3 = v2;
}
LABEL_15:
v21 = v3;
}
else
{
v21 = dword_A75DC8++;
}
if ( v21 < 0 )
return;
v5 = BYTE1(location[1].y);
if ( v5 == 2 )
{
a = destX;
vecNormalize(&a);
for ( i = universe.ShipList.head; i; i = i->next )
{
structptr = (struct SpaceObj *)i->structptr;
if ( !*((_DWORD *)structptr + 4) )
{
v9 = *((_DWORD *)structptr + 172);
if ( v9 == 112 || v9 == 113 || v9 == 114 )
continue;
}
if ( collCheckRayBBoxIntersection(structptr, &location->x, &a.x, locationa, &hitFace) != MINUS_ONE )
{
v10 = i->structptr;
if ( v10[4] || v10[172] != 144 )
goto LABEL_19;
}
}
head = universe.ResourceList.head;
if ( universe.ResourceList.head )
{
while ( collCheckRayBBoxIntersection((struct SpaceObj *)head->structptr, &location->x, &a.x, locationa, &hitFace) == MINUS_ONE )
{
head = head->next;
if ( !head )
goto LABEL_34;
}
goto LABEL_19;
}
LABEL_34:
if ( location[2].x < (double)ONE )
{
v12 = (ONE - location[2].x) * flt_72A674 + location[2].x;
location[2].x = v12;
if ( v12 > ONE )
location[2].x = 1.0;
}
goto LABEL_37;
}
if ( v5 != 1 )
goto LABEL_34;
LABEL_19:
if ( location[2].x > (double)ZERO )
{
v6 = location[2].x * flt_72AF48;
location[2].x = v6;
if ( v6 < dbl_72BEB8 )
LOBYTE(location[1].z) = 0;
}
LABEL_37:
if ( LOBYTE(location[1].z) )
{
selCircleComputeGeneral(&second, &first, location, 1.0, &destX.x, &destX.y, &destRadius);
if ( destRadius > (double)ZERO )
{
x = destX.x;
if ( v15 )
x = -x;
if ( x > dbl_72A548 )
goto LABEL_46;
y = destX.y;
if ( v18 )
y = -y;
if ( y > dbl_72A548 )
{
LABEL_46:
v19 = flt_8BABA8 - fabs(destX.x);
*(float *)&hitFace = flt_8BABA8 - fabs(destX.y);
if ( v19 >= *(float *)&hitFace )
v19 = *(float *)&hitFace;
if ( v19 <= ZERO )
location[2].y = 0.0;
else
location[2].y = v19 / (flt_8BABA8 - ONE);
}
else
{
location[2].y = 1.0;
}
if ( location[2].y > (double)ZERO )
{
*(double *)&vol = (double)MAIN_WindowWidth * destX.x * ZERO_POINT_FIVE + dbl_72A458;
LODWORD(destRadius) = *(_DWORD *)&vol + MAIN_WindowWidth / 2;
location[3].x = (float)SLODWORD(destRadius);
*(double *)&vol = (double)MAIN_WindowHeight * destX.y * ZERO_POINT_FIVE + dbl_72A458;
LODWORD(destRadius) = MAIN_WindowHeight / 2 - *(_DWORD *)&vol;
v20 = (double)SLODWORD(destRadius);
location[2].z = locationa;
location[3].y = v20;
dword_B37820[v21] = (int)location;
return;
}
}
LOBYTE(location[1].z) = 0;
}
dword_B37820[v21] = (int)location;
}
#endif

#if 0  // Cat3: rndRenderLensflares
/*=== rndRenderLensflares (0x57BEC0) ===*/
void __cdecl rndRenderLensflares()
{
Node *head; // esi
int v1; // ecx
int v2; // esi
double v3; // st7
int v4; // ebp
int v5; // edi
int v6; // edi
int v7; // edi
bool v8; // bl
int v9; // edi
double v10; // st7
int v11; // ecx
double v12; // st7
double v13; // st7
double v14; // st7
double v15; // st7
int v16; // esi
long double v17; // st7
long double v18; // st7
int v19; // edx
int v20; // ecx
int v21; // eax
GLfloat red; // [esp+88h] [ebp-84h]
GLfloat reda; // [esp+88h] [ebp-84h]
GLfloat redb; // [esp+88h] [ebp-84h]
GLfloat green; // [esp+8Ch] [ebp-80h]
GLfloat greena; // [esp+8Ch] [ebp-80h]
GLfloat greenb; // [esp+8Ch] [ebp-80h]
GLfloat greenc; // [esp+8Ch] [ebp-80h]
GLfloat greend; // [esp+8Ch] [ebp-80h]
GLfloat greene; // [esp+8Ch] [ebp-80h]
GLfloat greenf; // [esp+8Ch] [ebp-80h]
GLfloat greeng; // [esp+8Ch] [ebp-80h]
GLfloat greenh; // [esp+8Ch] [ebp-80h]
GLfloat greeni; // [esp+8Ch] [ebp-80h]
GLfloat greenj; // [esp+8Ch] [ebp-80h]
GLfloat bluea; // [esp+90h] [ebp-7Ch]
GLfloat blueb; // [esp+90h] [ebp-7Ch]
GLfloat bluec; // [esp+90h] [ebp-7Ch]
GLfloat blued; // [esp+90h] [ebp-7Ch]
GLfloat bluee; // [esp+90h] [ebp-7Ch]
GLfloat bluef; // [esp+90h] [ebp-7Ch]
float blue; // [esp+90h] [ebp-7Ch]
GLfloat blueg; // [esp+90h] [ebp-7Ch]
GLfloat blueh; // [esp+90h] [ebp-7Ch]
GLfloat bluei; // [esp+90h] [ebp-7Ch]
GLfloat bluej; // [esp+90h] [ebp-7Ch]
char IsEnabled; // [esp+A6h] [ebp-66h]
char v48; // [esp+A7h] [ebp-65h]
int v49; // [esp+A8h] [ebp-64h]
float v50; // [esp+ACh] [ebp-60h]
float v51; // [esp+B0h] [ebp-5Ch]
float v52; // [esp+B4h] [ebp-58h]
float v53; // [esp+B8h] [ebp-54h]
float v54; // [esp+BCh] [ebp-50h]
float v55; // [esp+BCh] [ebp-50h]
float v56; // [esp+C0h] [ebp-4Ch]
float v57; // [esp+C4h] [ebp-48h]
float v58; // [esp+C8h] [ebp-44h]
float v59; // [esp+CCh] [ebp-40h]
int v60; // [esp+D0h] [ebp-3Ch]
float v61; // [esp+D4h] [ebp-38h]
int v62; // [esp+D8h] [ebp-34h]
int v63; // [esp+DCh] [ebp-30h]
int i; // [esp+E0h] [ebp-2Ch]
float v65; // [esp+E4h] [ebp-28h]
float v66; // [esp+E4h] [ebp-28h]
double v67; // [esp+E4h] [ebp-28h]
float v68; // [esp+F0h] [ebp-1Ch]
float v69; // [esp+F0h] [ebp-1Ch]
float v70; // [esp+F4h] [ebp-18h]
float v71; // [esp+F8h] [ebp-14h]
int v72; // [esp+FCh] [ebp-10h]
int v73; // [esp+100h] [ebp-Ch]
double v74; // [esp+104h] [ebp-8h]
v72 = rndLightingEnabled;
if ( rndLightingEnabled )
{
dword_B3A4B0(2896);
rndLightingEnabled = 0;
}
v73 = rndTexture2DEnabled;
if ( !rndTexture2DEnabled )
{
glEnable(3553);
rndTexture2DEnabled = 1;
}
if ( *(_DWORD *)&dword_A75D4C && enableLensFlare && !bitTest(gDevcaps2, DEVSTAT2_NO_FLARES) )
{
head = rndLensflareSourceList.head;
for ( dword_A75DC8 = 0; head; head = head->next )
rndUpdateLensflareSource((vector *)head->structptr);
if ( !primModeEnabled )
primModeSetFunction2();
glDepthMask(0);
v60 = 0;
if ( dword_A75DC8 > 0 )
{
v1 = rndTexture2DEnabled;
do
{
v2 = dword_B37820[v60];
if ( v2 && *(_BYTE *)(v2 + 20) )
{
v70 = *(float *)(v2 + 36) - (double)(MAIN_WindowWidth / 2);
v71 = *(float *)(v2 + 40) - (double)(MAIN_WindowHeight / 2);
v3 = *(float *)(v2 + 28) * *(float *)(v2 + 24) * *(float *)(v2 + 12);
v61 = v3;
if ( v3 != ZERO )
{
for ( i = 0; i < dword_A75DD0; ++i )
{
v4 = byte_A75AD8[16 * i];
v53 = (double)MAIN_WindowHeight * *(float *)&dword_A75AD0[4 * i];
v5 = v1;
v54 = v70 * *(float *)&dword_A75AD4[4 * i] + (double)(MAIN_WindowWidth / 2);
v59 = v71 * *(float *)&dword_A75AD4[4 * i] + (double)(MAIN_WindowHeight / 2);
if ( v4 < 0 )
{
if ( v1 )
{
dword_B3A4B0(3553);
rndTexture2DEnabled = 0;
}
v62 = v5;
v9 = rndLightingEnabled;
if ( rndLightingEnabled )
{
dword_B3A4B0(2896);
rndLightingEnabled = 0;
}
v63 = v9;
glEnable(3042);
dword_B3A4B0(3008);
if ( dword_A75D80 != 1 )
{
dword_A75D80 = 1;
if ( (gDevcaps2 & 2) != 0 )
glBlendFunc(770, 771);
else
glBlendFunc(770, 1);
}
v7 = rndTextureEnviron;
if ( rndTextureEnviron )
{
glTexEnvi(8960, 8704, rndTextureModeTable[0]);
rndTextureEnviron = 0;
}
v8 = (unsigned __int8)glIsEnabled(2884) != 0;
IsEnabled = glIsEnabled(3042);
v48 = glIsEnabled(2882);
if ( !IsEnabled )
glEnable(3042);
if ( v48 )
dword_B3A4B0(2882);
}
else
{
if ( !v1 )
{
glEnable(3553);
rndTexture2DEnabled = 1;
}
v62 = v5;
v6 = rndLightingEnabled;
if ( rndLightingEnabled )
{
dword_B3A4B0(2896);
rndLightingEnabled = 0;
}
v63 = v6;
trMakeCurrent(*(unsigned int *)&dword_A75D4C);
if ( (*(_BYTE *)(trStructureGet(dword_A75D4C) + 12) & 8) != 0 )
{
glEnable(3042);
dword_B3A4B0(3008);
if ( dword_A75D80 != 1 )
{
dword_A75D80 = 1;
if ( (gDevcaps2 & 2) != 0 )
glBlendFunc(770, 771);
else
glBlendFunc(770, 1);
}
}
v7 = rndTextureEnviron;
if ( rndTextureEnviron )
{
glTexEnvi(8960, 8704, rndTextureModeTable[0]);
rndTextureEnviron = 0;
}
v8 = (unsigned __int8)glIsEnabled(2884) != 0;
}
dword_B3A4B0(2884);
v10 = (double)dword_A75ADC[16 * i] * v61;
if ( v10 >= ZERO )
{
if ( v10 <= FLT_255 )
v52 = v10;
else
v52 = 255.0;
}
else
{
v52 = 0.0;
}
v11 = *(_DWORD *)&dword_A75ADC[16 * i];
v12 = (double)BYTE1(v11) * v61;
if ( v12 >= ZERO )
{
if ( v12 <= FLT_255 )
v51 = v12;
else
v51 = 255.0;
}
else
{
v51 = 0.0;
}
v13 = (double)BYTE2(v11) * v61;
if ( v13 >= ZERO )
{
if ( v13 <= FLT_255 )
v50 = v13;
else
v50 = 255.0;
}
else
{
v50 = 0.0;
}
if ( v4 < 0 )
{
switch ( v4 )
{
case -4:
v49 = (__int64)(*(float *)&dword_A75AD0[4 * i] * flt_72BEC0 * flt_72A530);
if ( v49 >= 6 )
{
if ( v49 > 32 )
v49 = 32;
}
else
{
v49 = 6;
}
v58 = v52 * RND_LENSFLARE_DIM_CENTER;
v57 = v51 * RND_LENSFLARE_DIM_CENTER;
v56 = v50 * RND_LENSFLARE_DIM_CENTER;
break;
case -3:
v49 = (__int64)(*(float *)&dword_A75AD0[4 * i] * flt_72BEC0 * flt_72A530);
if ( v49 >= 6 )
{
if ( v49 > 32 )
v49 = 32;
}
else
{
v49 = 6;
}
v58 = v52;
v52 = v52 * RND_LENSFLARE_DIM_CENTER;
v57 = v51;
v56 = v50;
v51 = v51 * RND_LENSFLARE_DIM_CENTER;
v50 = v50 * RND_LENSFLARE_DIM_CENTER;
break;
case -2:
v49 = (__int64)(*(float *)&dword_A75AD0[4 * i] * flt_72BEC0 * flt_72A530);
if ( v49 >= 6 )
{
if ( v49 > 32 )
v49 = 32;
}
else
{
v49 = 6;
}
v58 = v52;
v57 = v51;
v56 = v50;
break;
case -1:
v49 = 6;
v58 = v52;
v57 = v51;
v56 = v50;
break;
default:
break;
}
blueg = v56 * RND_LENSFLARE_COLOR_SCALE;
greenf = v57 * RND_LENSFLARE_COLOR_SCALE;
reda = v58 * RND_LENSFLARE_COLOR_SCALE;
glColor3f(reda, greenf, blueg);
glBegin(6);
v69 = v59 + RND_LENSFLARE_SCREEN_BIAS;
blueh = ONE - (v69 / (double)MAIN_WindowHeight + v69 / (double)MAIN_WindowHeight);
v15 = (v54 + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowWidth;
greeng = v15 + v15 - ONE;
glVertex2f(LODWORD(greeng), LODWORD(blueh));
bluei = v50 * RND_LENSFLARE_COLOR_SCALE;
greenh = v51 * RND_LENSFLARE_COLOR_SCALE;
redb = v52 * RND_LENSFLARE_COLOR_SCALE;
glColor3f(redb, greenh, bluei);
v74 = 0.0;
if ( v49 > 0 )
{
v16 = v49;
v67 = dbl_72A500 / (double)v49;
do
{
v17 = (sin(v74) * v53 + v59 + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowHeight;
bluej = ONE - (v17 + v17);
v18 = (cos(v74) * v53 + v54 + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowWidth;
greeni = v18 + v18 - ONE;
glVertex2f(LODWORD(greeni), LODWORD(bluej));
--v16;
v74 = v67 + v74;
}
while ( v16 );
}
blue = ONE - (v69 / (double)MAIN_WindowHeight + v69 / (double)MAIN_WindowHeight);
v14 = (v54 + v53 + RND_LENSFLARE_SCREEN_BIAS) / (double)MAIN_WindowWidth;
}
else
{
bluea = v50 * RND_LENSFLARE_COLOR_SCALE;
green = v51 * RND_LENSFLARE_COLOR_SCALE;
red = v52 * RND_LENSFLARE_COLOR_SCALE;
glColor3f(red, green, bluea);
glBegin(7);
glTexCoord2f(dword_A759B8[4 * v4], LODWORD(dword_A759BC[4 * v4]));
v65 = v59 - v53 + RND_LENSFLARE_SCREEN_BIAS;
v68 = v54 - v53 + RND_LENSFLARE_SCREEN_BIAS;
blueb = ONE - (v65 / (double)MAIN_WindowHeight + v65 / (double)MAIN_WindowHeight);
greena = v68 / (double)MAIN_WindowWidth + v68 / (double)MAIN_WindowWidth - ONE;
glVertex2f(LODWORD(greena), LODWORD(blueb));
greenb = flt_A759C0[4 * v4] + *(float *)&dword_A759B8[4 * v4];
glTexCoord2f(LODWORD(greenb), LODWORD(dword_A759BC[4 * v4]));
v55 = v54 + v53 + RND_LENSFLARE_SCREEN_BIAS;
bluec = ONE - (v65 / (double)MAIN_WindowHeight + v65 / (double)MAIN_WindowHeight);
greenc = v55 / (double)MAIN_WindowWidth + v55 / (double)MAIN_WindowWidth - ONE;
glVertex2f(LODWORD(greenc), LODWORD(bluec));
blued = dword_A759BC[4 * v4] + flt_A759C0[4 * v4 + 1];
greend = flt_A759C0[4 * v4] + *(float *)&dword_A759B8[4 * v4];
glTexCoord2f(LODWORD(greend), LODWORD(blued));
v66 = v59 + v53 + RND_LENSFLARE_SCREEN_BIAS;
bluee = ONE - (v66 / (double)MAIN_WindowHeight + v66 / (double)MAIN_WindowHeight);
greene = v55 / (double)MAIN_WindowWidth + v55 / (double)MAIN_WindowWidth - ONE;
glVertex2f(LODWORD(greene), LODWORD(bluee));
bluef = dword_A759BC[4 * v4] + flt_A759C0[4 * v4 + 1];
glTexCoord2f(dword_A759B8[4 * v4], LODWORD(bluef));
blue = ONE - (v66 / (double)MAIN_WindowHeight + v66 / (double)MAIN_WindowHeight);
v14 = v68 / (double)MAIN_WindowWidth;
}
greenj = v14 + v14 - ONE;
glVertex2f(LODWORD(greenj), LODWORD(blue));
glEnd(v20, v19);
if ( v4 < 0 )
{
dword_B3A4B0(3042);
if ( rndTextureEnviron != v7 )
{
glTexEnvi(8960, 8704, rndTextureModeTable[v7]);
rndTextureEnviron = v7;
}
if ( dword_A75D80 )
{
dword_A75D80 = 0;
glBlendFunc(770, 771);
}
v1 = rndTexture2DEnabled;
if ( v62 )
{
if ( !rndTexture2DEnabled )
{
glEnable(3553);
v1 = 1;
goto LABEL_111;
}
}
else if ( rndTexture2DEnabled )
{
dword_B3A4B0(3553);
v1 = 0;
LABEL_111:
rndTexture2DEnabled = v1;
}
if ( v63 )
{
if ( !v1 )
{
glEnable(3553);
v1 = 1;
goto LABEL_117;
}
}
else if ( v1 )
{
dword_B3A4B0(3553);
v1 = 0;
LABEL_117:
rndTexture2DEnabled = v1;
}
if ( v8 )
{
glEnable(2884);
v1 = rndTexture2DEnabled;
}
if ( !IsEnabled )
{
dword_B3A4B0(3042);
v1 = rndTexture2DEnabled;
}
if ( !v48 )
continue;
glEnable(2882);
goto LABEL_124;
}
dword_B3A4B0(3042);
if ( rndTextureEnviron != v7 )
{
glTexEnvi(8960, 8704, rndTextureModeTable[v7]);
rndTextureEnviron = v7;
}
if ( dword_A75D80 )
{
dword_A75D80 = 0;
glBlendFunc(770, 771);
}
v1 = rndTexture2DEnabled;
if ( v62 )
{
if ( rndTexture2DEnabled )
goto LABEL_94;
glEnable(3553);
v1 = 1;
}
else
{
if ( !rndTexture2DEnabled )
goto LABEL_94;
dword_B3A4B0(3553);
v1 = 0;
}
rndTexture2DEnabled = v1;
LABEL_94:
if ( v63 )
{
if ( v1 )
goto LABEL_100;
glEnable(3553);
v1 = 1;
}
else
{
if ( !v1 )
goto LABEL_100;
dword_B3A4B0(3553);
v1 = 0;
}
rndTexture2DEnabled = v1;
LABEL_100:
if ( v8 )
{
glEnable(2884);
LABEL_124:
v1 = rndTexture2DEnabled;
}
}
}
v21 = dword_B37820[v60];
if ( *(_BYTE *)(v21 + 18) )
{
*(_BYTE *)(v21 + 17) = 1;
v1 = rndTexture2DEnabled;
}
}
++v60;
}
while ( v60 < dword_A75DC8 );
}
glDepthMask(1);
if ( primModeEnabled )
primModeClearFunction2();
if ( v72 )
{
if ( !rndLightingEnabled )
{
glEnable(2896);
rndLightingEnabled = 1;
}
}
else if ( rndLightingEnabled )
{
dword_B3A4B0(2896);
rndLightingEnabled = 0;
}
if ( v73 )
{
if ( !rndTexture2DEnabled )
{
glEnable(3553);
rndTexture2DEnabled = 1;
}
}
else if ( rndTexture2DEnabled )
{
dword_B3A4B0(3553);
rndTexture2DEnabled = 0;
}
}
}
#endif

static LinkedList rndLensflareSourceList = { 0 };

/* Anonymous 0x40-byte lensflare-source blob allocated in particle.c
   (partInitParticle, PART_LENSFLARES); only the intrusive render-list node at
   offset 0x2C has a determinable type.  The leading bytes are opaque source
   parameters initialised by the particle code. */
typedef struct LensflareSource
{
    ubyte opaqueParams[0x2C]; /* 0x00..0x2B */
    Node  renderLink;         /* 0x2C: node in rndLensflareSourceList */
    ubyte opaqueTail[0x04];   /* 0x3C..0x3F */
} LensflareSource;
static_assert(sizeof(LensflareSource) == 0x40, "LensflareSource size mismatch");
static_assert(offsetof(LensflareSource, renderLink) == 44, "LensflareSource renderLink offset mismatch");

void __cdecl rndLensflareSourceRemove(void *source)
{
    listRemoveNode(&((LensflareSource *)source)->renderLink);
}

void __cdecl rndLensflareSourceAdd(void *source)
{
    listAddNode(&rndLensflareSourceList, &((LensflareSource *)source)->renderLink, source);
    *((udword *)source + 6) = 0;
    *((ubyte *)source + 17) = 2;
}

static void rndLensflareReset(void)
{
    if (rndLensflareTextureHandle != TR_Invalid)
    {
        trRegisterRemoval(rndLensflareTextureHandle);
        rndLensflareTextureHandle = TR_Invalid;
    }
    rndLensflareFadeoutFrameSize = RND_LENSFLARE_DEFAULT_FADEOUT;
    rndLensflareTextureCount = 0;
    rndLensflareElementCount = 0;
    rndLensflareVisibleCount = 0;
    memClearDword(rndLensflareTextureCoords, 0, sizeof(rndLensflareTextureCoords) / sizeof(udword));
    memClearDword(rndLensflareElements, 0, sizeof(rndLensflareElements) / sizeof(udword));
    memClearDword(rndLensflareVisibleSources, 0, sizeof(rndLensflareVisibleSources) / sizeof(udword));
}

static real32 rndLensflareScreenToGLX(real32 x)
{
    real32 biased = x + RND_LENSFLARE_SCREEN_BIAS;
    return biased / (real32)MAIN_WindowWidth * 2.0f - 1.0f;
}

static real32 rndLensflareScreenToGLY(real32 y)
{
    real32 biased = y + RND_LENSFLARE_SCREEN_BIAS;
    return 1.0f - biased / (real32)MAIN_WindowHeight * 2.0f;
}

static sdword rndLensflareVisibleSlotSelect(real32 distance)
{
    sdword index;
    sdword farthestIndex;
    real32 farthestDistance;

    for (index = 0; index < RND_LENSFLARE_MAX_VISIBLE; index++)
    {
        if (rndLensflareVisibleSources[index] == NULL ||
            !rndLensflareVisibleSources[index]->visible)
        {
            return index;
        }
    }

    farthestIndex = 0;
    farthestDistance = rndLensflareVisibleSources[0]->distance;
    for (index = 1; index < RND_LENSFLARE_MAX_VISIBLE; index++)
    {
        if (rndLensflareVisibleSources[index]->distance > farthestDistance)
        {
            farthestDistance = rndLensflareVisibleSources[index]->distance;
            farthestIndex = index;
        }
    }

    if (distance < farthestDistance)
    {
        return farthestIndex;
    }
    return -1;
}

static bool32 rndLensflareSourceOccluded(rndlensflaresource *source, vector *direction, real32 distance)
{
    Node *node;
    sdword hitFace;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = (Ship *)node->structptr;

        if (ship->playerowner == NULL)
        {
            if (ship->shiptype == dSlipgateSmall ||
                ship->shiptype == dSlipgateMedium ||
                ship->shiptype == dSlipgateLarge)
            {
                continue;
            }
        }
        if (collCheckRayBBoxIntersection((SpaceObjRot *)ship, &source->position, direction, distance, &hitFace) != -1)
        {
            if (ship->playerowner != NULL || ship->shiptype != nisLensflareSource1)
            {
                return TRUE;
            }
        }
    }

    for (node = universe.ResourceList.head; node != NULL; node = node->next)
    {
        if (collCheckRayBBoxIntersection((SpaceObjRot *)node->structptr, &source->position, direction, distance, &hitFace) != -1)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void __cdecl rndUpdateLensflareSource(vector *location)
{
    rndlensflaresource *source = (rndlensflaresource *)location;
    vector eyeToSource;
    vector ray;
    real32 distance;
    real32 projectedX;
    real32 projectedY;
    real32 projectedRadius;
    real32 edgeDistance;
    sdword visibleSlot;

    if (!source->enabled)
    {
        source->visible = FALSE;
        return;
    }

    eyeToSource.x = mrCamera->eyeposition.x - source->position.x;
    eyeToSource.y = mrCamera->eyeposition.y - source->position.y;
    eyeToSource.z = mrCamera->eyeposition.z - source->position.z;
    distance = getVectDistSloppy(eyeToSource);
    source->distance = distance;

    visibleSlot = rndLensflareVisibleSlotSelect(distance);
    if (visibleSlot < 0)
    {
        return;
    }

    if (distance > 0.0f)
    {
        ray = eyeToSource;
        vecNormalize(&ray);
    }
    else
    {
        ray.x = ray.y = 0.0f;
        ray.z = 1.0f;
    }

    if (source->mode == 2)
    {
        if (rndLensflareSourceOccluded(source, &ray, distance))
        {
            source->visibility *= RND_LENSFLARE_DECAY_FACTOR;
            if (source->visibility < RND_LENSFLARE_FADE_EPSILON)
            {
                source->visible = FALSE;
            }
        }
        else if (source->visibility < 1.0f)
        {
            source->visibility = (1.0f - source->visibility) * RND_LENSFLARE_GROW_FACTOR + source->visibility;
            if (source->visibility > 1.0f)
            {
                source->visibility = 1.0f;
            }
        }
    }
    else if (source->mode == 1)
    {
        if (source->visibility > 0.0f)
        {
            source->visibility *= RND_LENSFLARE_DECAY_FACTOR;
            if (source->visibility < RND_LENSFLARE_FADE_EPSILON)
            {
                source->visible = FALSE;
            }
        }
    }
    else if (source->visibility < 1.0f)
    {
        source->visibility = (1.0f - source->visibility) * RND_LENSFLARE_GROW_FACTOR + source->visibility;
        if (source->visibility > 1.0f)
        {
            source->visibility = 1.0f;
        }
    }

    source->visible = TRUE;

    selCircleComputeGeneral(&rndCameraMatrix, &rndProjectionMatrix, &source->position, 1.0f,
                            &projectedX, &projectedY, &projectedRadius);
    if (projectedRadius <= 0.0f)
    {
        source->visible = FALSE;
    }
    else
    {
        if (fabs(projectedX) > RND_LENSFLARE_EDGE_START || fabs(projectedY) > RND_LENSFLARE_EDGE_START)
        {
            edgeDistance = rndLensflareFadeoutFrameSize - fabs(projectedX);
            if (rndLensflareFadeoutFrameSize - fabs(projectedY) < edgeDistance)
            {
                edgeDistance = rndLensflareFadeoutFrameSize - fabs(projectedY);
            }
            if (edgeDistance <= 0.0f)
            {
                source->edgeFade = 0.0f;
            }
            else
            {
                source->edgeFade = edgeDistance / (rndLensflareFadeoutFrameSize - 1.0f);
            }
        }
        else
        {
            source->edgeFade = 1.0f;
        }

        if (source->edgeFade > 0.0f)
        {
            source->screenX = (real32)((sdword)((real32)MAIN_WindowWidth * projectedX * 0.5f +
                                                (real32)(MAIN_WindowWidth / 2) + 0.5f));
            source->screenY = (real32)((sdword)((real32)(MAIN_WindowHeight / 2) -
                                                (real32)MAIN_WindowHeight * projectedY * 0.5f + 0.5f));
            rndLensflareVisibleSources[visibleSlot] = source;
            if (visibleSlot >= rndLensflareVisibleCount)
            {
                rndLensflareVisibleCount = visibleSlot + 1;
            }
            return;
        }
        source->visible = FALSE;
    }

    rndLensflareVisibleSources[visibleSlot] = source;
    if (visibleSlot >= rndLensflareVisibleCount)
    {
        rndLensflareVisibleCount = visibleSlot + 1;
    }
}

static void rndLensflareDrawCircle(real32 centerX, real32 centerY, real32 radius, sdword segments,
                                   real32 centerRed, real32 centerGreen, real32 centerBlue,
                                   real32 outerRed, real32 outerGreen, real32 outerBlue)
{
    sdword index;
    real32 step;
    real32 angle;

    glColor3f(centerRed, centerGreen, centerBlue);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(rndLensflareScreenToGLX(centerX), rndLensflareScreenToGLY(centerY));

    glColor3f(outerRed, outerGreen, outerBlue);
    step = (real32)(2.0 * PI / (real32)segments);
    angle = 0.0f;
    for (index = 0; index < segments; index++)
    {
        real32 x = centerX + radius * (real32)cos(angle);
        real32 y = centerY + radius * (real32)sin(angle);

        glVertex2f(rndLensflareScreenToGLX(x), rndLensflareScreenToGLY(y));
        angle += step;
    }
    glVertex2f(rndLensflareScreenToGLX(centerX + radius), rndLensflareScreenToGLY(centerY));
    glEnd();
}

static void rndLensflareDrawTextured(real32 centerX, real32 centerY, real32 radius,
                                     real32 red, real32 green, real32 blue,
                                     rndlensflaretexcoords *coords)
{
    real32 left = centerX - radius;
    real32 right = centerX + radius;
    real32 top = centerY - radius;
    real32 bottom = centerY + radius;

    glColor3f(red, green, blue);
    glBegin(GL_QUADS);
    glTexCoord2f(coords->u0, coords->v0);
    glVertex2f(rndLensflareScreenToGLX(left), rndLensflareScreenToGLY(top));
    glTexCoord2f(coords->u1, coords->v0);
    glVertex2f(rndLensflareScreenToGLX(right), rndLensflareScreenToGLY(top));
    glTexCoord2f(coords->u1, coords->v1);
    glVertex2f(rndLensflareScreenToGLX(right), rndLensflareScreenToGLY(bottom));
    glTexCoord2f(coords->u0, coords->v1);
    glVertex2f(rndLensflareScreenToGLX(left), rndLensflareScreenToGLY(bottom));
    glEnd();
}

void __cdecl rndRenderLensflares(void)
{
    Node *node;
    texreg *texture;
    sdword sourceIndex;
    sdword oldLighting;
    sdword oldTexture;
    sdword oldTextureEnv;

    if (rndLensflareTextureHandle == TR_Invalid ||
        !enableLensFlare ||
        bitTest(gDevcaps2, DEVSTAT2_NO_FLARES))
    {
        return;
    }

    texture = trStructureGet(rndLensflareTextureHandle);
    if (texture == NULL)
    {
        return;
    }

    memClearDword(rndLensflareVisibleSources, 0, sizeof(rndLensflareVisibleSources) / sizeof(udword));
    rndLensflareVisibleCount = 0;
    for (node = rndLensflareSourceList.head; node != NULL; node = node->next)
    {
        rndUpdateLensflareSource((vector *)node->structptr);
    }
    if (rndLensflareVisibleCount == 0 || rndLensflareElementCount == 0)
    {
        return;
    }

    oldLighting = rndLightingEnable(FALSE);
    oldTexture = rndTextureEnable(TRUE);
    oldTextureEnv = rndTextureEnvironment(0);
    if (!primModeEnabled)
    {
        primModeSetFunction2();
    }

    glDepthMask(GL_FALSE);

    for (sourceIndex = 0; sourceIndex < rndLensflareVisibleCount; sourceIndex++)
    {
        rndlensflaresource *source = rndLensflareVisibleSources[sourceIndex];
        sdword elementIndex;

        if (source == NULL || !source->visible)
        {
            continue;
        }

        for (elementIndex = 0; elementIndex < rndLensflareElementCount; elementIndex++)
        {
            rndlensflareelement *element = &rndLensflareElements[elementIndex];
            GLboolean oldBlendEnabled;
            GLboolean oldCullEnabled;
            sdword elementTextureEnabled = rndTexture2DEnabled;
            sdword elementLightingEnabled = rndLightingEnabled;
            real32 radius = (real32)MAIN_WindowHeight * element->radius;
            real32 centerX = (source->screenX - (real32)(MAIN_WindowWidth / 2)) * element->offset +
                             (real32)(MAIN_WindowWidth / 2);
            real32 centerY = (source->screenY - (real32)(MAIN_WindowHeight / 2)) * element->offset +
                             (real32)(MAIN_WindowHeight / 2);
            real32 scale = source->visibility * source->edgeFade * source->intensity;
            real32 red = (real32)element->red * scale;
            real32 green = (real32)element->green * scale;
            real32 blue = (real32)element->blue * scale;

            if (red < 0.0f) red = 0.0f; else if (red > 255.0f) red = 255.0f;
            if (green < 0.0f) green = 0.0f; else if (green > 255.0f) green = 255.0f;
            if (blue < 0.0f) blue = 0.0f; else if (blue > 255.0f) blue = 255.0f;

            if (element->textureIndex < 0)
            {
                sdword segments = (sdword)(element->radius * RND_LENSFLARE_SEGMENT_SCALE);
                real32 centerRed = red * RND_LENSFLARE_COLOR_SCALE;
                real32 centerGreen = green * RND_LENSFLARE_COLOR_SCALE;
                real32 centerBlue = blue * RND_LENSFLARE_COLOR_SCALE;
                real32 outerRed = centerRed;
                real32 outerGreen = centerGreen;
                real32 outerBlue = centerBlue;

                oldBlendEnabled = glIsEnabled(GL_BLEND);
                oldCullEnabled = glIsEnabled(GL_CULL_FACE);
                if (segments < RND_LENSFLARE_SEGMENT_MIN)
                {
                    segments = RND_LENSFLARE_SEGMENT_MIN;
                }
                else if (segments > RND_LENSFLARE_SEGMENT_MAX)
                {
                    segments = RND_LENSFLARE_SEGMENT_MAX;
                }

                switch (element->textureIndex)
                {
                    case -4:
                        segments = max(segments, RND_LENSFLARE_SEGMENT_MIN);
                        centerRed *= RND_LENSFLARE_DIM_CENTER;
                        centerGreen *= RND_LENSFLARE_DIM_CENTER;
                        centerBlue *= RND_LENSFLARE_DIM_CENTER;
                        break;
                    case -3:
                        segments = max(segments, RND_LENSFLARE_SEGMENT_MIN);
                        outerRed *= RND_LENSFLARE_DIM_CENTER;
                        outerGreen *= RND_LENSFLARE_DIM_CENTER;
                        outerBlue *= RND_LENSFLARE_DIM_CENTER;
                        break;
                    case -2:
                        segments = max(segments, RND_LENSFLARE_SEGMENT_MIN);
                        break;
                    case -1:
                        segments = 6;
                        break;
                    default:
                        continue;
                }

                rndTextureEnable(FALSE);
                rndLightingEnable(FALSE);
                glEnable(GL_BLEND);
                glDisable(GL_ALPHA_TEST);
                rndAdditiveBlends(TRUE);
                glDisable(GL_CULL_FACE);
                rndLensflareDrawCircle(centerX, centerY, radius, segments,
                                       centerRed, centerGreen, centerBlue,
                                       outerRed, outerGreen, outerBlue);

                glDisable(GL_BLEND);
                if (rndTextureEnviron != oldTextureEnv)
                {
                    rndTextureEnvironment(oldTextureEnv);
                }
                if (rndAdditiveBlending)
                {
                    rndAdditiveBlends(FALSE);
                }
                if (elementTextureEnabled)
                {
                    rndTextureEnable(TRUE);
                }
                else
                {
                    rndTextureEnable(FALSE);
                }
                if (elementLightingEnabled)
                {
                    rndLightingEnable(TRUE);
                }
                else
                {
                    rndLightingEnable(FALSE);
                }
                if (oldCullEnabled)
                {
                    glEnable(GL_CULL_FACE);
                }
                if (oldBlendEnabled)
                {
                    glEnable(GL_BLEND);
                }
            }
            else if (element->textureIndex < rndLensflareTextureCount)
            {
                oldCullEnabled = glIsEnabled(GL_CULL_FACE);
                rndTextureEnable(TRUE);
                rndLightingEnable(FALSE);
                trMakeCurrent(rndLensflareTextureHandle);
                if (bitTest(texture->flags, TRF_Alpha))
                {
                    glEnable(GL_BLEND);
                    glDisable(GL_ALPHA_TEST);
                    rndAdditiveBlends(TRUE);
                }
                glDisable(GL_CULL_FACE);
                rndLensflareDrawTextured(centerX, centerY, radius,
                                         red * RND_LENSFLARE_COLOR_SCALE,
                                         green * RND_LENSFLARE_COLOR_SCALE,
                                         blue * RND_LENSFLARE_COLOR_SCALE,
                                         &rndLensflareTextureCoords[element->textureIndex]);

                glDisable(GL_BLEND);
                if (rndTextureEnviron != oldTextureEnv)
                {
                    rndTextureEnvironment(oldTextureEnv);
                }
                if (rndAdditiveBlending)
                {
                    rndAdditiveBlends(FALSE);
                }
                if (elementTextureEnabled)
                {
                    rndTextureEnable(TRUE);
                }
                else
                {
                    rndTextureEnable(FALSE);
                }
                if (elementLightingEnabled)
                {
                    rndLightingEnable(TRUE);
                }
                else
                {
                    rndLightingEnable(FALSE);
                }
                if (oldCullEnabled)
                {
                    glEnable(GL_CULL_FACE);
                }
            }
        }

        if (source->persist)
        {
            source->mode = 1;
        }
    }

    glDepthMask(GL_TRUE);
    if (primModeEnabled)
    {
        primModeClearFunction2();
    }
    rndTextureEnvironment(oldTextureEnv);
    rndTextureEnable(oldTexture);
    rndLightingEnable(oldLighting);
}

static void rndLensflareElementSet(char *directory, char *field, void *dataToFillIn)
{
    rndlensflareelement *element;
    real32 radius;
    real32 offset;
    sdword textureIndex;
    sdword red;
    sdword green;
    sdword blue;

    if (sscanf(field, "%f %f %d %d %d %d", &radius, &offset, &textureIndex, &red, &green, &blue) != 6)
    {
        return;
    }

    element = &rndLensflareElements[rndLensflareElementCount++];
    element->radius = radius;
    element->offset = offset;
    element->textureIndex = (sbyte)textureIndex;
    element->red = (ubyte)red;
    element->green = (ubyte)green;
    element->blue = (ubyte)blue;
}

static void rndLensflareTextureSet(char *directory, char *field, void *dataToFillIn)
{
    rndlensflaretexcoords *coords;
    sdword x;
    sdword y;
    sdword width;
    sdword height;

    if (sscanf(field, "%d %d %d %d", &x, &y, &width, &height) != 4)
    {
        return;
    }

    coords = &rndLensflareTextureCoords[rndLensflareTextureCount++];
    coords->u0 = (real32)x * RND_LENSFLARE_TEXCOORD_SCALE;
    coords->v0 = (real32)y * RND_LENSFLARE_TEXCOORD_SCALE;
    coords->u1 = (real32)(x + width) * RND_LENSFLARE_TEXCOORD_SCALE;
    coords->v1 = (real32)(y + height) * RND_LENSFLARE_TEXCOORD_SCALE;
}

static void rndLensflareTextureFileSet(char *directory, char *field, void *dataToFillIn)
{
    char texturePath[_MAX_PATH];
    char *extension;

    dbgAssert(strlen(directory) + strlen(field) < sizeof(texturePath));
    strcpy(texturePath, directory);
    strcat(texturePath, field);
    extension = strrchr(texturePath, '.');
    if (extension != NULL)
    {
        *extension = '\0';
    }
    if (rndLensflareTextureHandle != TR_Invalid)
    {
        trRegisterRemoval(rndLensflareTextureHandle);
    }
    rndLensflareTextureHandle = trTextureRegister(texturePath, NULL, NULL);
}

void rndLensflareLoad(char *presetName)
{
    char scriptName[_MAX_PATH];
    char *extension;

    rndLensflareReset();
    if (presetName == NULL || presetName[0] == '\0')
    {
        return;
    }

    dbgAssert(strlen(presetName) < sizeof(scriptName) - 8);
    strcpy(scriptName, presetName);
    StripTrailingSpaces(scriptName);
    extension = strrchr(scriptName, '.');
    if (extension == NULL)
    {
        strcat(scriptName, ".script");
    }

    scriptSet("LensFlares\\", scriptName, rndLensflareScriptTable);
    trRegistryRefresh();
}

#if 0  // Cat2: rndCameraInside live code differs from binary or raw IDA offsets
/*=== rndCameraInside (0x574F80) ===*/
BOOL __cdecl rndCameraInside(int a1, int a2)
{
int v2; // eax
unsigned __int8 v4; // c0
unsigned __int8 v5; // c3

v2 = *(_DWORD *)(a1 + 20);
return (v2 & 1) != 0 && (v2 & 2) != 0 && v4 | v5 && collCheckBBoxPoint(a1, (float *)(a2 + 16));
}
#endif

// Decompiled from cataclysm.exe @ 0x576010
void rndRenderJaggedLine3(
    real32 x0,
    real32 y0,
    real32 z0,
    real32 x1,
    real32 y1,
    real32 z1,
    sdword segments,
    real32 jitterRadius)
{
    vector segmentDelta;
    vector evenPosition;
    vector jitteredPosition;
    real32 segmentLength;
    real32 jitterLimit;
    sdword segmentIndex;
    sdword correctionThreshold;
    sdword remainingSegments;

    if (segments <= 0)
    {
        return;
    }

    segmentDelta.x = x1 - x0;
    segmentDelta.y = y1 - y0;
    segmentDelta.z = z1 - z0;
    segmentLength = fsqrt(
        segmentDelta.x * segmentDelta.x +
        segmentDelta.y * segmentDelta.y +
        segmentDelta.z * segmentDelta.z);
    vecNormalize(&segmentDelta);
    if (segmentDelta.x == 0.0f && segmentDelta.y == 0.0f && segmentDelta.z == 0.0f)
    {
        segmentDelta.z = 1.0f;
    }

    segmentDelta.x *= segmentLength / (real32)segments;
    segmentDelta.y *= segmentLength / (real32)segments;
    segmentDelta.z *= segmentLength / (real32)segments;
    jitterLimit = fsqrt(
        segmentDelta.x * jitterRadius * (segmentDelta.x * jitterRadius) +
        segmentDelta.y * jitterRadius * (segmentDelta.y * jitterRadius) +
        segmentDelta.z * jitterRadius * (segmentDelta.z * jitterRadius)) * 0.5f;

    evenPosition.x = x0;
    evenPosition.y = y0;
    evenPosition.z = z0;
    jitteredPosition = evenPosition;
    correctionThreshold = segments / 5;

    glBegin(GL_LINE_STRIP);
    glVertex3f(x0, y0, z0);
    for (segmentIndex = 0, remainingSegments = segments; segmentIndex < segments; segmentIndex++, remainingSegments--)
    {
        evenPosition.x += segmentDelta.x;
        evenPosition.y += segmentDelta.y;
        evenPosition.z += segmentDelta.z;

        jitteredPosition.x += segmentDelta.x;
        jitteredPosition.y += segmentDelta.y;
        jitteredPosition.z += segmentDelta.z;

        if (segmentIndex < segments - 1)
        {
            real32 jitterRange = jitterLimit * 2.0f;
            real32 jitterBase = -jitterLimit;

            jitteredPosition.x += (real32)ranRandom(RAN_ParticleStream) * jitterRange * (1.0f / (real32)UDWORD_Max) + jitterBase;
            jitteredPosition.y += (real32)ranRandom(RAN_ParticleStream) * jitterRange * (1.0f / (real32)UDWORD_Max) + jitterBase;
            jitteredPosition.z += (real32)ranRandom(RAN_ParticleStream) * jitterRange * (1.0f / (real32)UDWORD_Max) + jitterBase;

            if (remainingSegments < correctionThreshold)
            {
                jitteredPosition.x += (evenPosition.x - jitteredPosition.x) / (real32)remainingSegments;
                jitteredPosition.y += (evenPosition.y - jitteredPosition.y) / (real32)remainingSegments;
                jitteredPosition.z += (evenPosition.z - jitteredPosition.z) / (real32)remainingSegments;
            }
        }

        glVertex3f(jitteredPosition.x, jitteredPosition.y, jitteredPosition.z);
    }
    glEnd();
}

static void rndRenderEMP(Ship *ship)
{
    GrowSelection *empSelection;
    sdword i;
    udword currentLOD;
    real32 jitterRadius;
    sdword segmentCount;
    Ship *target;
    sdword lightingWasEnabled;
    sdword textureWasEnabled;

    if (acvIsEMPActive(ship))
    {
        i = 0;
        lightingWasEnabled = rndLightingEnabled;
        if (rndLightingEnabled)
        {
            glDisable(2896);
            rndLightingEnabled = 0;
        }
        textureWasEnabled = rndTexture2DEnabled;
        if (rndTexture2DEnabled)
        {
            glDisable(3553);
            rndTexture2DEnabled = 0;
        }
        empSelection = acvGetEMPSelection(ship);
        glColor3f(EMPColorRed, EMPColorGreen, EMPColorBlue);
        currentLOD = ship->currentLOD;
        if (currentLOD >= 3)
        {
            currentLOD = 3;
        }
        jitterRadius = EMPJitterByLOD[currentLOD];
        segmentCount = EMPLineSegmentsByLOD[currentLOD];
        if (empSelection->selection != NULL && empSelection->selection->numShips > 0)
        {
            do
            {
                if (EMPRenderStride <= 0 || !(ranRandom(RAN_ParticleStream) % EMPRenderStride))
                {
                    target = empSelection->selection->ShipPtr[i];
                    if (target != NULL)
                    {
                        rndRenderJaggedLine3(
                            ship->posinfo.position.x,
                            ship->posinfo.position.y,
                            ship->posinfo.position.z,
                            target->posinfo.position.x,
                            target->posinfo.position.y,
                            target->posinfo.position.z,
                            segmentCount,
                            jitterRadius);
                    }
                }
                ++i;
            }
            while (i < empSelection->selection->numShips);
        }
        if (lightingWasEnabled)
        {
            if (!rndLightingEnabled)
            {
                glEnable(2896);
                rndLightingEnabled = 1;
            }
        }
        else if (rndLightingEnabled)
        {
            glDisable(2896);
            rndLightingEnabled = 0;
        }
        if (textureWasEnabled)
        {
            if (!rndTexture2DEnabled)
            {
                glEnable(3553);
                rndTexture2DEnabled = 1;
            }
        }
        else if (rndTexture2DEnabled)
        {
            glDisable(3553);
            rndTexture2DEnabled = 0;
        }
    }
}
