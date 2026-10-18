/*=============================================================================
    Name    : DefaultShip.c
    Purpose : Specifics for the DefaultShip (placeholder ship)

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "gun.h"
#include "attack.h"
#include "statscript.h"
#include "meshanim.h"
#include "universe.h"
#include "CommandLayer.h"
#include "SaveGame.h"
#include "Ship.h"
#include "dock.h"
#include "shipselect.h"
#include "nebulae.h"
#include "DefaultShip.h"
#include "Bounties.h"
#include "Clamp.h"
#include "consMgr.h"
#include "NIS.h"
#include "Randy.h"
#include "SoundEvent.h"
#include "Physics.h"
#include "Timer.h"
#include "Carrier.h"
#include "Dreadnought.h"
#include "GenericInterceptor.h"
#include "Leech.h"
#include "P1Mothership.h"
#include "RammingFrigate.h"
#include "SalCapCorvette.h"
#include "Slipgate.h"
#include "Swarmer.h"
#include "univupdate.h"

void seekdir(void);
void ResourceCollectorInit(Ship *ship);
void RKOStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo);
void RKOInit(Ship *ship);
void RKORemoveShipReferences(Ship *ship, Ship *shiptoremove);
void RKOClose(Ship *ship);
void RKOPreFix(Ship *ship);
void RKOFix(Ship *ship);
void RKOHousekeep(Ship *ship);

typedef struct
{
    udword dummy;
} DefaultShipStatics;

typedef struct
{
    udword dummy;
} DefaultShipSpec;

typedef struct
{
    bool32 dontRecurseKill;
} MiningBaseSpec;

typedef struct
{
    sdword resourceValue;
} CrystalStatics;

typedef struct
{
    sdword currentResourceValue;
    bool32 harvested;
    bool32 processingActive;
} CrystalShipSpec;

typedef struct
{
    real32 transientEffectRadius;
    real32 permanentEffectRadius;
    udword slipgateColor;
} SlipgateStatics;

typedef struct
{
    ubyte state;                  // retail: ship+0x638
    ubyte pad0[3];
    real32 expiryTime;            // retail: ship+0x63C
    vector translationDelta;      // retail: ship+0x640..0x648
    udword unk14;
    udword unk18;
    udword unk1C;
    bool32 spinActive;            // retail: ship+0x658
    real32 spinAngle;             // retail: ship+0x65C
    vector spinAxis;              // retail: ship+0x660..0x668
    bool32 deleteOnExpire;        // retail: ship+0x66C
    real32 startTime;             // retail: ship+0x670
} ComponentShipSpec;

static_assert(sizeof(ComponentShipSpec) == 0x3C);

static real32 componentDelay_sDreadnoughtCannon;
static real32 componentDelay_sCarrierSupport;
static real32 componentDelay_sMothershipSpecial;
static real32 componentDelay_sMothershipEngineering;
static real32 componentDelay_sMothershipArmour;
static real32 componentDelay_sMothershipMicro;
static real32 componentDelay_sMothershipWeapons;
static real32 componentDelay_sMothershipDockingBay;
static real32 componentDelay_sMothershipSupport;
static real32 componentDelay_sMothershipCommand;
static real32 componentDelay_sMothershipEnginesLower;
static real32 componentDelay_sMothershipEnginesUpper;
static real32 componentDelay_sMothershipLowerSpine;
static real32 componentDelay_sMothershipBigGun;
static real32 componentDelay_sMothershipOreCanArmour;
static real32 componentDelay_bMothershipDockingBay;
static real32 componentDelay_bMothershipEnginesLower;
static real32 componentDelay_bSectMoship;
static real32 componentDelay_bMothershipSupport;

static const real32 shipComponentRandomScale = 4.656612873077393e-10f; /* cataclysm.exe 0x72AFB8 */
static const real32 shipComponentSpinStep = 0.05f;                    /* cataclysm.exe 0x8C35F8 */
static const real32 shipComponentSpinWrap = 6.2831854820251465f;      /* cataclysm.exe 0x72A594 */
static CrystalStatics crystal0Statics;                                 /* cataclysm.exe 0xA762D0 */
static CrystalStatics crystal1Statics;                                 /* cataclysm.exe 0xA762D4 */
static const SlipgateStatics slipgateNodeStatics = { 0.0f, 0.0f, 0 };               /* cataclysm.exe 0xA7A5C4 */
static const SlipgateStatics slipgateSmallStatics = { 1250.0f, 5000.0f, 0xFF0000FF };  /* cataclysm.exe 0xA7A5D0 */
static const SlipgateStatics slipgateMediumStatics = { 2600.0f, 10400.0f, 0xFF00FFFF }; /* cataclysm.exe 0xA7A5C8 */
static const SlipgateStatics slipgateLargeStatics = { 11100.0f, 44400.0f, 0xFFFF0000 };  /* cataclysm.exe 0xA7A5CC */

static scriptStructEntry CrystalStaticScriptTable[] =
{
    { "resourceValue", scriptSetSdwordCB, (udword)&crystal0Statics.resourceValue, (udword)&crystal0Statics },
    { NULL, NULL, 0, 0 }
};

static_assert(sizeof(CrystalShipSpec) == 0x0C);
static_assert(sizeof(SlipgateShipSpec) == 0x30);
static_assert(offsetof(SlipgateShipSpec, queuedShips) == 0x24);

static real32 *shipGetModuleStaticData(ShipType shipType)
{
    switch (shipType)
    {
        case sDreadnoughtCannon:
            return &componentDelay_sDreadnoughtCannon;
        case sCarrierSupport:
            return &componentDelay_sCarrierSupport;
        case sMothershipSpecial:
            return &componentDelay_sMothershipSpecial;
        case sMothershipEngineering:
            return &componentDelay_sMothershipEngineering;
        case sMothershipArmour:
            return &componentDelay_sMothershipArmour;
        case sMothershipMicro:
            return &componentDelay_sMothershipMicro;
        case sMothershipWeapons:
            return &componentDelay_sMothershipWeapons;
        case sMothershipDockingBay:
            return &componentDelay_sMothershipDockingBay;
        case sMothershipSupport:
            return &componentDelay_sMothershipSupport;
        case sMothershipCommand:
            return &componentDelay_sMothershipCommand;
        case sMothershipEnginesLower:
            return &componentDelay_sMothershipEnginesLower;
        case sMothershipEnginesUpper:
            return &componentDelay_sMothershipEnginesUpper;
        case sMothershipLowerSpine:
            return &componentDelay_sMothershipLowerSpine;
        case sMothershipBigGun:
            return &componentDelay_sMothershipBigGun;
        case sMothershipOreCanArmour:
            return &componentDelay_sMothershipOreCanArmour;
        case bMothershipDockingBay:
            return &componentDelay_bMothershipDockingBay;
        case bMothershipEnginesLower:
            return &componentDelay_bMothershipEnginesLower;
        case bSectMoship:
            return &componentDelay_bSectMoship;
        case bMothershipSupport:
            return &componentDelay_bMothershipSupport;
        default:
            dbgAssert(FALSE);
            return NULL;
    }
}

typedef struct
{
    ShipType shipType;
    ubyte rangeIndex;
} ComponentEjectRangeMap;

typedef struct
{
    real32 col1Min;
    real32 col1Max;
    real32 col2Min;
    real32 col2Max;
    real32 col2bMin;
    real32 col2bMax;
} ComponentEjectRange;

static const ComponentEjectRange componentEjectRanges[] =
{
    { 0.4f, 0.5f, -0.3f, 0.3f, -0.02f, 0.02f },
    { -0.4f, -0.5f, -0.3f, 0.3f, -0.02f, 0.02f },
    { 0.15f, 0.2f, -0.3f, 0.3f, -0.2f, -0.4f },
    { -0.15f, -0.2f, -0.3f, 0.3f, -0.2f, -0.4f },
    { 0.1f, 0.2f, -0.3f, 0.3f, 0.1f, 0.3f },
    { -0.1f, -0.2f, -0.3f, 0.3f, 0.1f, 0.3f },
    { -0.4f, -0.5f, -0.3f, 0.3f, -0.02f, 0.02f }
};

static const ComponentEjectRangeMap componentEjectRangeMap[] =
{
    { sMothershipSpecial, 0 },
    { sMothershipEngineering, 4 },
    { sMothershipArmour, 0 },
    { sMothershipMicro, 0 },
    { sMothershipWeapons, 0 },
    { sMothershipDockingBay, 5 },
    { sMothershipSupport, 0 },
    { sMothershipEnginesLower, 3 },
    { sMothershipEnginesUpper, 2 },
    { sMothershipLowerSpine, 1 },
    { sMothershipBigGun, 6 },
    { sMothershipOreCanArmour, 0 },
    { sCarrierSupport, 1 },
    { 0, 0 }
};

static const real32 componentEjectSpeedMin = 50000.0f; /* cataclysm.exe 0x8C35FC */
static const real32 componentEjectSpeedMax = 70000.0f; /* cataclysm.exe 0x8C3600 */

static const ComponentEjectRange *shipComponentGetEjectRange(ShipType shipType)
{
    sdword i;

    for (i = 0; i < sizeof(componentEjectRangeMap) / sizeof(componentEjectRangeMap[0]); i++)
    {
        if (componentEjectRangeMap[i].shipType == shipType)
        {
            return &componentEjectRanges[componentEjectRangeMap[i].rangeIndex];
        }
    }

    return &componentEjectRanges[0];
}

// Cataclysm-specific: starts the timed module-eject motion used by detached clamped components.
void shipInitModuleEject(Ship *ship, bool32 deleteOnExpire, real32 startDelay, real32 activeDuration)
{
    ComponentShipSpec *spec = (ComponentShipSpec *)ship->ShipSpecifics;
    const ComponentEjectRange *range = shipComponentGetEjectRange(ship->shiptype);
    Ship *host = (Ship *)ship->clampInfo->host;
    vector col1Vector;
    vector col2Vector;
    real32 randomScale = shipComponentRandomScale;
    real32 translationMagnitude;

    spec->state = 1;
    spec->startTime = universe.totaltimeelapsed + startDelay;
    spec->expiryTime = spec->startTime + activeDuration;
    matGetVectFromMatrixCol1(col1Vector, host->rotinfo.coordsys);
    matGetVectFromMatrixCol2(col2Vector, host->rotinfo.coordsys);

    spec->translationDelta.x =
        (((real32)ranRandom(10) * randomScale) * (range->col1Max - range->col1Min) + range->col1Min) *
        col1Vector.x;
    spec->translationDelta.y =
        (((real32)ranRandom(10) * randomScale) * (range->col1Max - range->col1Min) + range->col1Min) *
        col1Vector.y;
    spec->translationDelta.z =
        (((real32)ranRandom(10) * randomScale) * (range->col1Max - range->col1Min) + range->col1Min) *
        col1Vector.z;

    spec->translationDelta.x +=
        (((real32)ranRandom(10) * randomScale) * (range->col2Max - range->col2Min) + range->col2Min) *
        col2Vector.x;
    spec->translationDelta.y +=
        (((real32)ranRandom(10) * randomScale) * (range->col2Max - range->col2Min) + range->col2Min) *
        col2Vector.y;
    spec->translationDelta.z +=
        (((real32)ranRandom(10) * randomScale) * (range->col2Max - range->col2Min) + range->col2Min) *
        col2Vector.z;

    spec->translationDelta.x +=
        (((real32)ranRandom(10) * randomScale) * (range->col2bMax - range->col2bMin) + range->col2bMin) *
        col2Vector.x;
    spec->translationDelta.y +=
        (((real32)ranRandom(10) * randomScale) * (range->col2bMax - range->col2bMin) + range->col2bMin) *
        col2Vector.y;
    spec->translationDelta.z +=
        (((real32)ranRandom(10) * randomScale) * (range->col2bMax - range->col2bMin) + range->col2bMin) *
        col2Vector.z;

    vecNormalize(&spec->translationDelta);
    translationMagnitude =
        ((real32)ranRandom(10) * randomScale) * (componentEjectSpeedMax - componentEjectSpeedMin) + componentEjectSpeedMin;
    spec->translationDelta.x *= translationMagnitude;
    spec->translationDelta.y *= translationMagnitude;
    spec->translationDelta.z *= translationMagnitude;

    spec->spinActive = TRUE;
    matGetVectFromMatrixCol1(spec->spinAxis, ship->rotinfo.coordsys);
    vecNormalize(&spec->spinAxis);
    spec->spinAngle = 0.0f;

    clDetachShipFromCommand(ship);
    clSetAttribute((sdword)ship, 1, 8);
    ship->flags |= SOF_AvoidanceHidden;
    spec->deleteOnExpire = deleteOnExpire;
}

// Decompiled from cataclysm.exe @ 0x5AF5E0
static void shipSetupModuleSlot(char *directory, char *filename, ShipStaticInfo *staticInfo)
{
    real32 *moduleDelay = shipGetModuleStaticData(staticInfo->shiptype);

    (void)directory;
    (void)filename;

    dbgAssert(moduleDelay != NULL);

    switch (staticInfo->shiptype)
    {
        case sDreadnoughtCannon:
        case sCarrierSupport:
            *moduleDelay = 1.0f;
            break;
        case sMothershipSpecial:
            *moduleDelay = 4.0f;
            break;
        case sMothershipEngineering:
            *moduleDelay = 4.5f;
            break;
        case sMothershipArmour:
        case bMothershipDockingBay:
        case bSectMoship:
            *moduleDelay = 2.0f;
            break;
        case sMothershipMicro:
            *moduleDelay = 3.5f;
            break;
        case sMothershipWeapons:
            *moduleDelay = 2.25f;
            break;
        case sMothershipDockingBay:
            *moduleDelay = 1.6f;
            break;
        case sMothershipSupport:
            *moduleDelay = 1.25f;
            break;
        case sMothershipCommand:
            *moduleDelay = 1.75f;
            break;
        case sMothershipEnginesLower:
            *moduleDelay = 5.25f;
            break;
        case sMothershipEnginesUpper:
            *moduleDelay = 5.0f;
            break;
        case sMothershipLowerSpine:
            *moduleDelay = 3.25f;
            break;
        case sMothershipBigGun:
        case bMothershipSupport:
            *moduleDelay = 2.75f;
            break;
        case sMothershipOreCanArmour:
            *moduleDelay = 2.5f;
            break;
        case bMothershipEnginesLower:
            *moduleDelay = 3.0f;
            break;
        default:
            dbgAssert(FALSE);
            *moduleDelay = 0.0f;
            break;
    }

    staticInfo->custstatinfo = moduleDelay;
    staticInfo->canSpecialBandBoxFriendlies = TRUE;
    staticInfo->unk4 = TRUE;
    staticInfo->unk5 = TRUE;
}

// Decompiled from cataclysm.exe @ 0x5AF720
static void shipComponentInit(Ship *ship)
{
    ComponentShipSpec *spec = (ComponentShipSpec *)ship->ShipSpecifics;

    spec->state = 0;
    spec->expiryTime = 0.0f;
    spec->spinActive = FALSE;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothershipDockingBay:
        case bMothershipEnginesLower:
            if (ship->shiprace == RACE_Beast)
            {
                real32 nextTime = universe.totaltimeelapsed + 1.0f;
                ship->statusWindow.phaseScale = -1.0f;
                ship->statusWindow.state = 1;
                ship->statusWindow.phaseStartTime = nextTime;
                ship->statusWindow.phaseEndTime = nextTime + SPEECH_HYPERSPACE_DELAY;
            }
            break;
        default:
            break;
    }

    if (ship->shiptype == sMothershipCommand)
    {
        madAnimationStartWithFlags(ship, 0, MAF_Loop);
    }
}

// Decompiled from cataclysm.exe @ 0x5AF7C0
static void shipComponentRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    if (shiptoremove == ship->dockvars.dockship)
    {
        ship->flags &= ~SOF_AvoidanceHidden;
        ship->specialFlags2 &= ~0x40u;
        ship->dockvars.dockship = NULL;
        clDetachShipFromCommand(ship);
    }
}

// Decompiled from cataclysm.exe @ 0x5AF800
static void shipComponentHousekeep(Ship *ship)
{
    ComponentShipSpec *spec = (ComponentShipSpec *)ship->ShipSpecifics;

    if (spec->state == 1)
    {
        if (spec->expiryTime >= universe.totaltimeelapsed)
        {
            ship->posinfo.force.x += spec->translationDelta.x;
            ship->posinfo.force.y += spec->translationDelta.y;
            ship->posinfo.force.z += spec->translationDelta.z;
            spec->spinAngle += shipComponentSpinStep;
            if (spec->spinAngle >= shipComponentSpinWrap)
            {
                spec->spinAngle = 0.0f;
            }
        }
        else if (spec->deleteOnExpire)
        {
            ship->howDidIDie = 777;
            ship->whoKilledMe = 99;
            bountyShipWasKilled(ship);
            ship->playerowner->totalships--;
            AddTargetToDeleteList((SpaceObjRotImpTarg *)ship, 5);
        }
        else
        {
            ship->flags |= SOF_DontApplyPhysics;
        }
    }

    if (!nisIsRunning_ && ship->clampInfo == NULL && spec->state != 1)
    {
        real32 *moduleDelay = (real32 *)ship->staticinfo->custstatinfo;

        dbgAssert(moduleDelay != NULL);
        spec->expiryTime = ((real32)ranRandom(1) * shipComponentRandomScale) + *moduleDelay + universe.totaltimeelapsed;
        spec->state = 1;
        spec->startTime = universe.totaltimeelapsed;
        spec->deleteOnExpire = TRUE;
    }
}

// Decompiled from cataclysm.exe @ 0x5AFE30
static void shipComponentClose(Ship *ship)
{
    SelectCommand *moduleSelection;
    Ship *host;
    sdword i;

    if (ship->shiptype != sMothershipDockingBay ||
        ship->clampInfo == NULL ||
        ship->clampInfo->host == NULL)
    {
        return;
    }

    host = (Ship *)ship->clampInfo->host;
    if (host->objtype != OBJ_ShipType || host->shiptype != sMothership)
    {
        return;
    }

    cmCancelFactoryExternalBuilds(host);

    if (host->clampInfo == NULL || host->clampInfo->selection == NULL)
    {
        return;
    }

    moduleSelection = (SelectCommand *)host->clampInfo->selection;
    for (i = 0; i < moduleSelection->numShips; i++)
    {
        Ship *module = moduleSelection->ShipPtr[i];

        if (module != NULL && (module->specialFlags2 & SPECIAL_2_DisabledState) != 0)
        {
            AddTargetToDeleteList((SpaceObjRotImpTarg *)module, 5);
        }
    }
}

CustShipHeader ComponentShipHeader =
{
    sizeof(ComponentShipSpec),
    shipSetupModuleSlot,
    NULL,
    shipComponentInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    shipComponentHousekeep,
    shipComponentRemoveShipReferences,
    NULL,
    NULL,
    NULL,
    shipComponentClose,
    NULL,
    NULL
};

// Decompiled from cataclysm.exe @ 0x5AFFD0
static void CrystalStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    CrystalStatics *crystalStatics = NULL;

    switch (statinfo->shiptype)
    {
        case dCrystal0:
            crystalStatics = &crystal0Statics;
            break;
        case dCrystal1:
            crystalStatics = &crystal1Statics;
            break;
        default:
            dbgAssert(FALSE);
            return;
    }

    crystalStatics->resourceValue = 0;
    scriptSetStruct(directory, filename, CrystalStaticScriptTable, (ubyte *)crystalStatics);
    if (crystalStatics->resourceValue <= 0)
    {
        ShipTypeToStr(statinfo->shiptype);
        telldir();
    }
    statinfo->custstatinfo = crystalStatics;
    statinfo->custstatinfo = crystalStatics;
}

// Decompiled from cataclysm.exe @ 0x5B0040
static void CrystalInit(Ship *ship)
{
    CrystalShipSpec *spec = (CrystalShipSpec *)ship->ShipSpecifics;
    CrystalStatics *statics = (CrystalStatics *)ship->staticinfo->custstatinfo;

    clSetAttribute((sdword)ship, TRUE, 12);
    ship->flags &= ~SOF_Selectable;
    ship->specialFlags1C8 |= 0x00040000;
    spec->currentResourceValue = statics->resourceValue;
    spec->harvested = FALSE;
    spec->processingActive = FALSE;
}

// Decompiled from cataclysm.exe @ 0x5B0090
static void CrystalClose(Ship *ship)
{
    CrystalShipSpec *spec = (CrystalShipSpec *)ship->ShipSpecifics;
    udword x = *((udword *)&ship->posinfo.position.x);
    udword y = *((udword *)&ship->posinfo.position.y);
    udword z = *((udword *)&ship->posinfo.position.z);

    if (!spec->harvested)
    {
        shockwaveCreateTyped(
            3,
            x,
            y,
            z,
            ship->playerowner->playerIndex);
    }
}

CustShipHeader CrystalHeader =
{
    sizeof(CrystalShipSpec),
    CrystalStaticInit,                    // 0x5AFFD0
    NULL,
    CrystalInit,                         // 0x5B0040
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    CrystalClose                         // 0x5B0090
};

// Decompiled from cataclysm.exe @ 0x5C71C0
static void SlipgateStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    (void)directory;
    (void)filename;

    statinfo->shipFlags |= BIT14 | BIT15;

    switch (statinfo->shiptype)
    {
        case dSlipgateSmall:
            statinfo->custstatinfo = (void *)&slipgateSmallStatics;
            return;
        case dSlipgateMedium:
            statinfo->custstatinfo = (void *)&slipgateMediumStatics;
            return;
        case dSlipgateLarge:
            statinfo->custstatinfo = (void *)&slipgateLargeStatics;
            return;
        case dSlipgateNode:
            statinfo->custstatinfo = (void *)&slipgateNodeStatics;
            return;
        default:
            statinfo->custstatinfo = statinfo;
            return;
    }
}

// Decompiled from cataclysm.exe @ 0x5C7250
static void SlipgateInit(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;
    const SlipgateStatics *statics = (const SlipgateStatics *)ship->staticinfo->custstatinfo;

    spec->state = 0;
    spec->approachStartTime = 0.0f;
    spec->partner = NULL;
    spec->effect = NULL;
    memset(spec->playerTransitState, 0, sizeof(spec->playerTransitState));
    spec->stateExpiryTime = 0.0f;
    ship->specialFlags1C8 |= 0x00040000;
    ship->flags2 |= SOF2_Unknown4;
    ship->flags &= ~SOF_Selectable;
    ship->specialFlags2 |= SPECIAL_2_FriendlyDockingOverride;
    spec->transientEffectRadius = statics->transientEffectRadius;
    spec->permanentEffectRadius = statics->permanentEffectRadius;
    spec->slipgateColor = statics->slipgateColor;
    growSelectInit(&spec->queuedShips);
}

// Decompiled from cataclysm.exe @ 0x5C74D0
static void SlipgateHousekeep(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;
    SpaceObj *effect;
    sdword i;

    if (spec->queuedShips.selection != NULL && spec->queuedShips.selection->numShips > 0)
    {
        for (i = 0; i < spec->queuedShips.selection->numShips; i++)
        {
            slipgateRequestDock(ship, spec->queuedShips.selection->ShipPtr[i]);
        }
        growSelectReset(&spec->queuedShips);
    }

    if (spec->effect != NULL &&
        spec->approachStartTime != 0.0f &&
        spec->approachStartTime + SPEECH_HYPERSPACE_DELAY < universe.totaltimeelapsed)
    {
        slipgateSetState(ship, 3);
        if (spec->partner != NULL && (ship->flags & SOF_AvoidanceHidden) == 0)
        {
            slipgateSetState(spec->partner, 3);
        }
        spec->approachStartTime = 0.0f;
    }

    if ((spec->state == 1 || spec->state == 3) &&
        spec->stateExpiryTime != 0.0f &&
        universe.totaltimeelapsed > spec->stateExpiryTime)
    {
        if (spec->state == 1)
        {
            spec->state = 2;
        }
        else if (spec->state == 3)
        {
            spec->state = 0;
            effect = (SpaceObj *)spec->effect;
            if (effect != NULL)
            {
                etgEffectDelete(spec->effect);
                univRemoveObjFromRenderList(effect);
                listDeleteNode(&effect->objlink);
                spec->effect = NULL;
            }
        }
        spec->stateExpiryTime = 0.0f;
    }

    if (spec->effect == NULL)
    {
        slipgateSetState(ship, spec->state);
    }
}

// Decompiled from cataclysm.exe @ 0x5C7620
static void SlipgateClose(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;
    SpaceObj *effect = (SpaceObj *)spec->effect;

    if (effect != NULL)
    {
        etgEffectDelete(spec->effect);
        univRemoveObjFromRenderList(effect);
        listDeleteNode(&effect->objlink);
    }
    if (spec->queuedShips.selection != NULL)
    {
        growSelectClose(&spec->queuedShips);
    }
}

// Decompiled from cataclysm.exe @ 0x5C7670
static void SlipgateRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;

    if (spec->partner == shiptoremove)
    {
        spec->partner = NULL;
    }
    if (spec->queuedShips.selection != NULL && spec->queuedShips.selection->numShips > 0)
    {
        growSelectRemoveShip(&spec->queuedShips, shiptoremove);
    }
}

// Decompiled from cataclysm.exe @ 0x5C76B0
static void SlipgateNotifyObjectDeleted(Ship *ship, sdword objectID)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;

    if ((sdword)spec->effect == objectID)
    {
        spec->effect = NULL;
        if (spec->state == 1)
        {
            spec->state = 2;
        }
        else if (spec->state == 3)
        {
            spec->state = 0;
        }
    }
}

// Decompiled from cataclysm.exe @ 0x5C76F0
static void Slipgate_PreFix(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;

    spec->effect = NULL;
    spec->partner = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->partner);
}

// Decompiled from cataclysm.exe @ 0x5C7720
static void Slipgate_Save(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;

    SaveGrowSelection(&spec->queuedShips);
}

// Decompiled from cataclysm.exe @ 0x5C7740
static void Slipgate_Load(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;

    LoadGrowSelectionAndFix(&spec->queuedShips);
}

// Decompiled from cataclysm.exe @ 0x5C7760
static void Slipgate_Fix(Ship *ship)
{
    SlipgateShipSpec *spec = (SlipgateShipSpec *)ship->ShipSpecifics;

    spec->partner = SpaceObjRegistryGetShip((sdword)spec->partner);
}

CustShipHeader SlipgateHeader =
{
    sizeof(SlipgateShipSpec),
    SlipgateStaticInit,                  // 0x5C71C0
    NULL,
    SlipgateInit,                        // 0x5C7250
    SlipgateClose,                       // 0x5C7620
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SlipgateHousekeep,                   // 0x5C74D0
    SlipgateRemoveShipReferences,        // 0x5C7670
    SlipgateRemoveShipReferences,        // 0x5C7670
    SlipgateNotifyObjectDeleted,         // 0x5C76B0
    NULL,
    SlipgateClose,                       // 0x5C7620
    Slipgate_PreFix,                     // 0x5C76F0
    Slipgate_Save,                       // 0x5C7720
    Slipgate_Load,                       // 0x5C7740
    Slipgate_Fix                         // 0x5C7760
};

typedef struct
{
    udword active;
    udword visualsEnabled;
    vector target;
    ubyte timer[0x10];
    Effect *effect;
} ProcessorCrystalProcessingState;

#define PROCESSOR_ATTACK_COMMAND_TOO_CLOSE_RANGE 3000.0f
#define CRYSTAL_PROCESSING_STATE_OFFSET 0x570

static ProcessorSpecifics *processorShipSpecifics(Ship *ship)
{
    return (ProcessorSpecifics *)ship->ShipSpecifics;
}

static ProcessorCrystalProcessingState *processorCrystalProcessingState(Ship *crystal)
{
    return (ProcessorCrystalProcessingState *)((ubyte *)crystal + CRYSTAL_PROCESSING_STATE_OFFSET);
}

static void ProcessorDeleteCrystalEffect(Ship *crystal)
{
    ProcessorCrystalProcessingState *processingState = processorCrystalProcessingState(crystal);

    if (processingState->active != 1 || processingState->effect == NULL)
    {
        return;
    }

    processingState->visualsEnabled = 0;
    etgEffectDelete(processingState->effect);
    univRemoveObjFromRenderList((SpaceObj *)processingState->effect);
    listDeleteNode(&processingState->effect->objlink);
    processingState->effect = NULL;
}

static void ProcessorDeleteTendrilEffect(ProcessorTendrilEntry *tendril)
{
    if (tendril->effect != NULL)
    {
        etgEffectDelete(tendril->effect);
        univRemoveObjFromRenderList((SpaceObj *)tendril->effect);
        listDeleteNode(&tendril->effect->objlink);
        tendril->effect = NULL;
    }
}

static void ProcessorClearTendrilTarget(ProcessorTendrilEntry *tendril)
{
    tendril->targetShip = NULL;
    ProcessorDeleteTendrilEffect(tendril);
}

static void ProcessorInit(Ship *ship)
{
    nebInitShipTendrils(ship);
    InitCrystalDockSlots(ship);
}

void shipAttackSimpleNoRotate(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    (void)rotateFlags;
    attackSimple(ship, target);
}

static void ProcessorHouseKeep(Ship *ship)
{
    nebUpdateShipTendrils(ship);
    ProcessCrystalDockSlots(ship);
}

static void ProcessorRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    ProcessorSpecifics *specifics = processorShipSpecifics(ship);
    sdword i;

    for (i = 0; i < specifics->tendrilCount; i++)
    {
        ProcessorTendrilEntry *tendril = &specifics->tendrils[i];
        if (tendril->targetShip == shiptoremove)
        {
            ProcessorClearTendrilTarget(tendril);
        }
    }

    if (shiptoremove->shiptype == dBigGunUnit)
    {
        ProcessorDeleteCrystalEffect(shiptoremove);
    }

    RemoveShipFromCrystalDockSlots(ship, shiptoremove);
}

static void ProcessorNotifyObjectDeleted(Ship *ship, sdword objectID)
{
    ProcessorSpecifics *specifics = processorShipSpecifics(ship);
    CrystalDockState *dockState = dockGetCrystalDockState(ship);
    sdword i;

    for (i = 0; i < specifics->tendrilCount; i++)
    {
        ProcessorTendrilEntry *tendril = &specifics->tendrils[i];
        if (tendril->effect != NULL &&
            SpaceObjRegistryGetID((SpaceObj *)tendril->effect) == objectID)
        {
            tendril->effect = NULL;
        }
    }

    if (dockState == NULL)
    {
        return;
    }

    for (i = 0; i < 16; i++)
    {
        Ship *slotShip = dockState->slots[i].ship;
        ProcessorCrystalProcessingState *processingState;

        if (slotShip == NULL || slotShip->shiptype != dBigGunUnit)
        {
            continue;
        }

        processingState = processorCrystalProcessingState(slotShip);
        if (processingState->effect != NULL &&
            SpaceObjRegistryGetID((SpaceObj *)processingState->effect) == objectID)
        {
            processingState->effect = NULL;
        }
    }
}

static void ProcessorClose(Ship *ship)
{
    ProcessorSpecifics *specifics = processorShipSpecifics(ship);
    sdword i;

    for (i = 0; i < specifics->tendrilCount; i++)
    {
        ProcessorClearTendrilTarget(&specifics->tendrils[i]);
    }

    ClearCrystalDockSlots(ship);
}

static void Processor_PreFix(Ship *ship)
{
    ProcessorSpecifics *specifics = processorShipSpecifics(ship);
    ShipStaticInfo *shipStaticInfo = ship->staticinfo;
    DockStaticInfo *dockStaticInfo;
    sdword i;

    if (shipStaticInfo == NULL)
    {
        shipStaticInfo = GetShipStaticInfo(ship->shiptype, ship->shiprace);
    }
    dockStaticInfo = (shipStaticInfo != NULL) ? shipStaticInfo->dockStaticInfo : NULL;

    for (i = 0; i < specifics->tendrilCount; i++)
    {
        ProcessorTendrilEntry *tendril = &specifics->tendrils[i];

        tendril->sourceShip = (Ship *)SpaceObjRegistryGetID((SpaceObj *)tendril->sourceShip);
        if (dockStaticInfo != NULL && tendril->dockStaticPoint != NULL)
        {
            tendril->dockStaticPoint = (DockStaticPoint *)(tendril->dockStaticPoint - dockStaticInfo->dockstaticpoints);
        }
        else
        {
            tendril->dockStaticPoint = NULL;
        }
        tendril->targetShip = (Ship *)SpaceObjRegistryGetID((SpaceObj *)tendril->targetShip);
        tendril->effect = NULL;
    }

    PreFix_CrystalDockSlots(ship);
}

static void Processor_Fix(Ship *ship)
{
    ProcessorSpecifics *specifics = processorShipSpecifics(ship);
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    sdword i;

    for (i = 0; i < specifics->tendrilCount; i++)
    {
        ProcessorTendrilEntry *tendril = &specifics->tendrils[i];
        sdword dockIndex = (sdword)tendril->dockStaticPoint;

        tendril->sourceShip = SpaceObjRegistryGetShip((sdword)tendril->sourceShip);
        if (dockStaticInfo != NULL && dockIndex >= 0 && dockIndex < dockStaticInfo->numDockPoints)
        {
            tendril->dockStaticPoint = &dockStaticInfo->dockstaticpoints[dockIndex];
        }
        else
        {
            tendril->dockStaticPoint = NULL;
        }
        tendril->targetShip = SpaceObjRegistryGetShip((sdword)tendril->targetShip);
        tendril->effect = NULL;
    }

    Fix_CrystalDockSlots(ship);
}

static bool32 ProcessorAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    attackStraightForward(ship, target, ship->staticinfo->bulletRange[ship->tacticstype], PROCESSOR_ATTACK_COMMAND_TOO_CLOSE_RANGE);
    (void)maxdist;
    return TRUE;
}

void DefaultShipFire(Ship *ship,SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    GunInfo *guninfo = ship->cataclysmGunInfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    (void)rotateFlags;

    if (guninfo == NULL)
    {
        return;
    }

    numGuns = guninfo->numGuns;

    for (i=0;i<numGuns;i++)
    {
        gun = &guninfo->guns[i];
        if (gunCanShootStaggered(ship, gun))
        {
            switch (gun->gunstatic->guntype)
            {
                case GUN_MissileLauncher:
                case GUN_MineLauncher:
                    if (gunHasMissiles(gun))
                    {
                        missileShoot(ship,gun,target);
                    }
                    break;

                case GUN_NewGimble:
                case GUN_Gimble:
                case GUN_Fixed:
                    gunShoot(ship,gun,target);
                    break;

                default:
                    dbgAssert(FALSE);

            }
        }
    }
}

void DefaultShipAttack(Ship *ship,SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    if (ship->cataclysmGunInfo == NULL)
    {
        dbgMessagef("\nWARNING: %s tried to attack but has no guns",ShipTypeToStr(ship->shiptype));
        return;
    }

    attackStraightForward(ship,target,shipstaticinfo->bulletRange[ship->tacticstype],shipstaticinfo->minBulletRange[ship->tacticstype]*0.9f);
}

void DefaultShipAttackPassive(Ship *ship, Ship *target, bool32 rotate)
{
    if (rotate && ((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
    {
        attackPassiveRotate(ship, target);
    }
    else
    {
        attackPassive(ship, target);
    }
}

void DefaultShipRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    if ((((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate & rotateFlags) != 0)
    {
        attackPassiveRotate(ship, (Ship *)target);
    }
    else
    {
        attackPassive(ship, (Ship *)target);
    }
}

void DefaultShipAttackStationary(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    DefaultShipAttackPassive(ship, (Ship *)target, TRUE);
    (void)maxdist;
}

static bool32 CargoBargeAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    if (ship->cataclysmGunInfo == NULL)
    {
        (void)ShipTypeToStr(ship->shiptype);
        return FALSE;
    }

    attackStraightForward(ship, target, shipstaticinfo->bulletRange[ship->tacticstype], shipstaticinfo->minBulletRange[ship->tacticstype] * 0.9f);
    (void)maxdist;
    return TRUE;
}

void MiningBaseInit(Ship *ship)
{
    MiningBaseSpec *spec = (MiningBaseSpec *)ship->ShipSpecifics;

    spec->dontRecurseKill = FALSE;
}

void MiningBaseDied(Ship *ship)
{
    Ship *deleteme;
    Node *node;
    MiningBaseSpec *spec = (MiningBaseSpec *)ship->ShipSpecifics;

    dbgAssert(ship->shiptype == MiningBase);
    if (ship->flags & SOF_NISShip)
    {
        return;
    }

    if (spec->dontRecurseKill)
    {
        return;
    }

    // first pass: Set all other MiningBases to dontRecurseKill so we don't get weird recursion

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        deleteme = (Ship *)listGetStructOfNode(node);
        dbgAssert(deleteme->objtype == OBJ_ShipType);

        if ((deleteme->shiptype == MiningBase) && (deleteme != ship) && ((deleteme->flags & (SOF_Dead|SOF_NISShip)) == 0))
        {
            spec = (MiningBaseSpec *)deleteme->ShipSpecifics;
            spec->dontRecurseKill = TRUE;
        }
    }

    // delete all other mining bases

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        deleteme = (Ship *)listGetStructOfNode(node);
        dbgAssert(deleteme->objtype == OBJ_ShipType);

        if ((deleteme->shiptype == MiningBase) && (deleteme != ship) && ((deleteme->flags & (SOF_Dead|SOF_NISShip)) == 0))
        {
            deleteme->howDidIDie = ship->howDidIDie;
            deleteme->whoKilledMe = ship->whoKilledMe;
            univDeleteDeadShip(deleteme,EDT_AccumDamage);
        }
    }
}

CustShipHeader DefaultShipHeader =
{
    sizeof(DefaultShipSpec),
    NULL,
    NULL,
    NULL,
    NULL,
    DefaultShipAttack,
    DefaultShipFire,
    DefaultShipAttackPassive,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader LeechHeader =
{
    sizeof(LeechSpecifics),
    leechStaticInit,
    NULL,
    leechInit,
    leechFreeShipContents,
    NULL,
    NULL,
    NULL,
    leechSpecialActivate,
    leechSpecialCleanup,
    leechHousekeep,
    leechRemoveShipReferences,
    leechRemoveShipReferences,
    NULL,
    NULL,
    leechClose,
    leech_PreFix,
    leech_Save,
    leech_Load,
    leech_Fix
};

CustShipHeader DreadnoughtHeader =
{
    sizeof(DreadnoughtSpecifics),
    dreadnoughtStaticInit,
    NULL,
    dreadnoughtInit,
    NULL,
    dreadnoughtFire,
    shipSharedRotateAndShoot,
    dreadnoughtSpecialActivate,
    dreadnoughtSpecialTargetStub,
    NULL,
    dreadnoughtHousekeep,
    dreadnoughtRemoveShipReferences,
    dreadnoughtRemoveShipReferences,
    NULL,
    NULL,
    dreadnoughtClose,
    dreadnought_PreFix,
    NULL,
    NULL,
    dreadnought_Fix,
    NULL,
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader RammingFrigateHeader =
{
    sizeof(RammingFrigateSpecifics),
    rammingFrigateSelectParams,
    NULL,
    rammingFrigateInit,
    rammingFrigateResetTarget,
    NULL,
    seekdir,
    rammingFrigateReset,
    rammingFrigateSelectTarget,
    NULL,
    rammingFrigateAttack,
    rammingFrigateClearTarget,
    rammingFrigateClearTarget,
    rammingFrigateClearDeadRef,
    NULL,
    rammingFrigateDeleteEffect,
    rammingFrigateSave,
    NULL,
    NULL,
    rammingFrigateFixupPointers,
    rammingFrigateResetState,
    rammingFrigateSetTarget,
    rammingFrigateResetState,
    NULL,
    NULL,
    NULL,
    NULL,
    rammingFrigateResetState
};

CustShipHeader RepairDroidHeader =
{
    /* Retail 0x8C6170: size + clBeamStaticScriptLoad only; remaining callback slots are null. */
    sizeof(RepairDroidSpecifics),
    clBeamStaticScriptLoad,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader SwarmerHeader =
{
    SWARMER_RETAIL_SPECIFICS_SIZE,
    SharedSwarmerStaticInit,
    NULL,
    SharedSwarmerInit,
    NULL,
    shipShootAllGunsAtTarget,
    NULL,
    NULL,
    NULL,
    NULL,
    seekdir,
    SharedSwarmerRemoveShipReferences,
    NULL,
    NULL,
    NULL,
    NULL,
    SharedSwarmer_PreFix,
    NULL,
    NULL,
    SharedSwarmer_Fix,
    NULL,
    SharedSideStepAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader BeastSwarmerHeader =
{
    sizeof(BeastSwarmerSpecifics),
    SharedSwarmerStaticInit,
    NULL,
    SharedSwarmerInit,
    NULL,
    shipShootAllGunsAtTarget,
    NULL,
    NULL,
    NULL,
    NULL,
    seekdir,
    SharedSwarmerRemoveShipReferences,
    NULL,
    NULL,
    NULL,
    NULL,
    SharedSwarmer_PreFix,
    NULL,
    NULL,
    SharedSwarmer_Fix,
    NULL,
    SharedSideStepAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader ProcessorHeader =
{
    sizeof(ProcessorSpecifics),
    NULL,
    NULL,
    ProcessorInit,
    NULL,
    shipShootAllGunsAtTarget,
    shipAttackSimpleNoRotate,
    NULL,
    NULL,
    NULL,
    ProcessorHouseKeep,
    ProcessorRemoveShipReferences,
    ProcessorRemoveShipReferences,
    ProcessorNotifyObjectDeleted,
    NULL,
    ProcessorClose,
    Processor_PreFix,
    NULL,
    NULL,
    Processor_Fix,
    NULL,
    ProcessorAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader CargoBargeHeader =
{
    sizeof(CargoBargeSpecifics),
    NULL,
    NULL,
    NULL,
    NULL,
    shipShootAllGunsAtTarget,
    P1MothershipRotateAndShoot,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    CargoBargeAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader RKOHeader =
{
    sizeof(RKOSpecifics),
    RKOStaticInit,
    NULL,
    RKOInit,
    NULL,
    shipShootAllGunsAtTarget,
    shipAttackSimpleNoRotate,
    NULL,
    NULL,
    NULL,
    RKOHousekeep,
    RKORemoveShipReferences,
    RKORemoveShipReferences,
    NULL,
    NULL,
    RKOClose,
    RKOPreFix,
    NULL,
    NULL,
    RKOFix,
    NULL
};

// Decompiled from cataclysm.exe @ 0x5BE050
static bool32 NomadMoonAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    (void)maxdist;
    attackSimple(ship, target);
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5BDBB0
static void NomadMoonInit(Ship *ship)
{
    Ship **emitters = (Ship **)((NomadMoonSpecifics *)ship->ShipSpecifics)->unk;
    vector shipPos = ship->posinfo.position;

    emitters[0] = NULL;
    emitters[1] = NULL;

    shipPos.z += 150.0f;
    emitters[0] = univCreateShip(dNomadFieldEmitter, ship->shiprace, &shipPos, ship->playerowner, 0);

    shipPos = ship->posinfo.position;
    shipPos.z -= 150.0f;
    emitters[1] = univCreateShip(dNomadFieldEmitter, ship->shiprace, &shipPos, ship->playerowner, 0);

    ((NomadMoonSpecifics *)ship->ShipSpecifics)->unk[8] = 0;
}

// Decompiled from cataclysm.exe @ 0x5BDDB0
static void NomadMoonRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    NomadMoonSpecifics *spec = (NomadMoonSpecifics *)ship->ShipSpecifics;
    Ship **emitters = (Ship **)spec->unk;
    sdword emitterCount;

    if (emitters[0] == shipToRemove)
    {
        emitters[0] = NULL;
    }
    if (emitters[1] == shipToRemove)
    {
        emitters[1] = NULL;
    }

    emitterCount = (emitters[0] != NULL);
    if (emitters[1] != NULL)
    {
        emitterCount++;
    }
    if (emitterCount == 0)
    {
        spec->unk[8] = 0;
    }
}

// Decompiled from cataclysm.exe @ 0x5BDE40
static void NomadMoonPreFix(Ship *ship)
{
    Ship **emitters = (Ship **)((NomadMoonSpecifics *)ship->ShipSpecifics)->unk;

    emitters[0] = (Ship *)SpaceObjRegistryGetID((SpaceObj *)emitters[0]);
    emitters[1] = (Ship *)SpaceObjRegistryGetID((SpaceObj *)emitters[1]);
}

// Decompiled from cataclysm.exe @ 0x5BDE10
static void NomadMoonFix(Ship *ship)
{
    Ship **emitters = (Ship **)((NomadMoonSpecifics *)ship->ShipSpecifics)->unk;

    emitters[0] = SpaceObjRegistryGetShip((sdword)emitters[0]);
    emitters[1] = SpaceObjRegistryGetShip((sdword)emitters[1]);
}

// Decompiled from cataclysm.exe @ 0x5BE070
static void NomadMoonAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    NomadMoonSpecifics *spec = (NomadMoonSpecifics *)ship->ShipSpecifics;
    Ship **emitters = (Ship **)spec->unk;
    sdword emitterCount;

    (void)target;
    (void)maxdist;
    if (spec->unk[8] != 0)
    {
        return;
    }

    emitterCount = (emitters[0] != NULL);
    if (emitters[1] != NULL)
    {
        emitterCount++;
    }
    if (emitterCount > 0)
    {
        TimerInitTimerStruct((Timer *)&spec->unk[12], (nisIsRunning_ == 1) ? 2.0f : 0.25f);
        spec->unk[8] = 1;
    }
}

// Decompiled from cataclysm.exe @ 0x5BDE70
static void NomadMoonHousekeep(Ship *ship)
{
    NomadMoonSpecifics *spec = (NomadMoonSpecifics *)ship->ShipSpecifics;
    Ship **emitters = (Ship **)spec->unk;
    vector newPosition;
    sdword emitterCount;

    if (spec->unk[8] == 1)
    {
        if (TimerExpired((int)&spec->unk[12]))
        {
            TimerInitTimerStruct((Timer *)&spec->unk[12], 0.5f);
            spec->unk[8] = 2;
            shockwaveCreateTyped(
                5,
                TreatAsUdword(ship->posinfo.position.x),
                TreatAsUdword(ship->posinfo.position.y),
                TreatAsUdword(ship->posinfo.position.z),
                ship->playerowner->playerIndex);
            soundEventPlay(ship, 2069, 0);
        }
    }
    else if (spec->unk[8] == 2 && TimerExpired((int)&spec->unk[12]))
    {
        spec->unk[8] = 0;
    }

    if (nisIsRunning_ == 1)
    {
        if (emitters[0] == NULL || emitters[1] == NULL)
        {
            return;
        }

        newPosition = ship->posinfo.position;
        newPosition.z += 150.0f;
        posSetShipPosition(emitters[0], &newPosition);

        newPosition = ship->posinfo.position;
        newPosition.z -= 150.0f;
        posSetShipPosition(emitters[1], &newPosition);
    }

    if (nisIsRunning_ || !shockwaveHasActiveShockwaves())
    {
        return;
    }

    emitterCount = (emitters[0] != NULL);
    if (emitters[1] != NULL)
    {
        emitterCount++;
    }
    if (emitterCount > 0 && spec->unk[8] == 0)
    {
        TimerInitTimerStruct((Timer *)&spec->unk[12], (nisIsRunning_ == 1) ? 2.0f : 0.25f);
        spec->unk[8] = 1;
    }
}

CustShipHeader NomadMoonHeader =
{
    sizeof(NomadMoonSpecifics),
    seekdir,
    NULL,
    NomadMoonInit,
    NULL,
    NomadMoonAttack,
    NULL,
    NULL,
    NULL,
    NULL,
    NomadMoonHousekeep,
    NomadMoonRemoveShipReferences,
    NomadMoonRemoveShipReferences,
    NULL,
    NULL,
    NULL,
    NomadMoonPreFix,
    NULL,
    NULL,
    NomadMoonFix,
    NULL,
    NomadMoonAttackCommand
};

CustShipHeader NomadFieldEmitterHeader =
{
    sizeof(NomadFieldEmitterSpecifics),
    seekdir,
    NULL,
    seekdir,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader DoNothingShipHeader =
{
    sizeof(DefaultShipSpec),
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader MiningBaseHeader =
{
    sizeof(MiningBaseSpec),
    NULL,
    NULL,
    ResourceCollectorInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MiningBaseDied,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader StationaryGunHeader =
{
    sizeof(DefaultShipSpec),
    NULL,
    NULL,
    NULL,
    NULL,
    shipShootAllGunsAtTarget,
    P1MothershipRotateAndShoot,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
