/*=============================================================================
    Name    : SalCapCorvette.c
    Purpose : Specifics for the Salvage Capture Corvette

    Created 11/5/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "SalCapCorvette.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "select.h"
#include "shipselect.h"
#include "univupdate.h"
#include "aiship.h"
#include "AIShip.h"
#include "aitrack.h"
#include "memory.h"
#include "universe.h"
#include "fastmath.h"
#include "dock.h"
#include "vector.h"
#include "commandlayer.h"
#include "soundevent.h"
#include "physics.h"
#include "multiplayergame.h"
#include "CommandWrap.h"
#include "SaveGame.h"
#include "aivar.h"
#include "prim3d.h"
#include "render.h"
#include "etg.h"
#include "clamp.h"
#include "tweak.h"
#include "nis.h"
#include "madlinkin.h"
#include "StringsOnly.h"
#include "madlinkindefs.h"
#include "mothership.h"
#include "collision.h"
#include "carrier.h"
#include "alliance.h"
#include "tactics.h"
#include "ping.h"
#include "singleplayer.h"
#include "battle.h"
#include "BattleChat.h"
#include "commandwrap.h"
#include "objtypes.h"
#include "genericdefender.h"
#include "gravwellgenerator.h"
#include "randy.h"

#include "Timer.h"
#include "Ship.h"

#define DEBUG_SALCAP

#define SALVAGE_BEGIN               0
#define SAL_FLYTOTARGET             1
#define SAL_DOCKINGSTATE1           3
#define SAL_DOCKWITHLIGHTS1         4
#define SAL_DOCKWITHOUTLIGHTS1      5
#define SAL_WL_FLYTOWITHINCONE      6
#define SAL_CLAMPED                 8
#define SAL_STRIPTECH               9
#define SAL_TECH_UNDOCK             10
#define SAL_SALVAGE1                11

#define SAL_UNDOCK_AND_QUIT         12
#define SAL_FLYBACK_TO_DOCKING_SHIP 13
#define SAL_DOCKINGSTAGE            14
#define SAL_TECH_TAKE_TECH_HOME     15
#define SAL_WAIT                    16
#define SAL_DO_DOCK                 17

#define SAL_backoff                 35
#define SAL_DO_DOCK2                18
#define SAL_DO_DOCK3                19
#define SAL_DONE_DOCKING            20
#define WAYPOINTHACK2               21
#define SAL_DO_DOCK25               22
#define DOORDEAL                    23
#define DO_HARVEST                  24
#define SAL_DO_DAMAGE               25

#define SAL_WAYPOINTHACK            26
#define SAL_JUST_DO_DAMAGE          27

#define NO_INDEX                    999192
#define SALCAP_HOST_QUEUE_ENTRY_SIZE 20

#define SAL_GOTOPOINT_LIMITVEL      0.0f

#define SALCAP_SPECIAL_REASSIGN_INTERVAL 10.0f
#define SALCAP_SPECIAL_REASSIGN_DISTANCE_DELTA 1000.0f
#define SALCAP_SPECIAL_TARGET_DRAG_SCALE 0.3f
#define SALCAP_SPECIAL_SUPPORT_MESSAGE_SENTINEL 1649989415u
#define SALCAP_TARGET_RUNTIME_FLAGS(target) (((Ship *)(target))->specialFlags1C8)
#define SALCAP_TARGET_CLAIM_OWNER(target) ((target)->salcapTargetClaimOwner)
#define SALCAP_TARGET_LAST_SUPPORT_MESSAGE_RAW(target) ((target)->beastAttack.raw_4F0)
#define SALCAP_TARGET_LAST_REASSIGN_TIME_RAW(target) ((target)->beastAttack.raw_4F4)

enum
{
    SALCAP_SPECIAL_PICK_TARGET = 0,
    SALCAP_SPECIAL_FLY_TO_TARGET = 1,
    SALCAP_SPECIAL_RESERVE_DOCK = 2,
    SALCAP_SPECIAL_FLY_TO_CONE = 3,
    SALCAP_SPECIAL_FLY_TO_CLAMPED_CONE = 4,
    SALCAP_SPECIAL_FLY_TO_OFFSET_DOCK = 5,
    SALCAP_SPECIAL_CLAMP_TARGET = 7,
    SALCAP_SPECIAL_PICK_PROCESSING_SHIP = 8,
    SALCAP_SPECIAL_WAIT_FOR_SUPPORT = 9,
    SALCAP_SPECIAL_DISABLE_AND_TOW = 10,
    SALCAP_SPECIAL_STABILIZE_CAPTURE = 11,
    SALCAP_SPECIAL_QUEUE_FOR_PROCESSING = 12,
    SALCAP_SPECIAL_HOST_PROCESSING = 13,
    SALCAP_SPECIAL_DONE = 14
};

void clLaunchShip(CommandLayer *comlayer, SelectCommand *selectcom, ShipPtr receiverShip);
void startTractorBeam(Ship *ship, SpaceObjRotImpTargGuidanceShipDerelict *target);
void clSetAttribute(sdword ship, sdword value, sdword attribute);
Ship *univSpawnReplacementShip(sdword ship, ShipType shiptype, Player *playerowner);
sdword shipQueueBeastConversion(Ship *ship, Ship *attacker);
real32 TimerTimeRemaining(void *timer);
real64 TimerRemaining(int timer);
bool32 TimerExpired(int timer);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
static void SalCapDetachShipFromCommand(Ship *ship);
static sdword SalCapUpdateTowedTargetMotion(Ship *ship);
Ship *SalCapGetFirstAttachedShip(Ship *target);
Ship *SalCapGetNextAttachedShip(void);
void telldir(void);
real32 CrystalGetProcessingRemainingFraction(Ship *crystal);
void CrystalStartProcessing(Ship *crystal, vector *processingTarget, real32 duration);
void CrystalUpdateProcessingTarget(Ship *crystal, vector *processingTarget);
static Effect *CrystalEnsureProcessingEffect(vector *processingTarget, Ship *crystal);

typedef struct
{
    Ship *ship;
    real32 startTime;
    real32 endTime;
    sdword state;
    sdword cleanupPending;
} SalCapHostQueueEntry;

typedef struct
{
    sdword resourceValue;
} CrystalStaticInfo;

typedef struct
{
    sdword currentResourceValue;
    bool32 harvested;
    bool32 processingActive;
} CrystalShipSpecifics;

SalCapCorvetteStatics SalCapCorvetteStaticRace1;
SalCapCorvetteStatics SalCapCorvetteStaticRace2;
SalCapCorvetteStatics SalCapCorvetteStaticDawg;

/* Per-ship-class capture threshold table, indexed by ShipClass enum.
   Populated during ship static initialization. Non-zero means the class is capturable. */
real32 salCapShipClassCaptureThreshold[16];

/* Beam ETG LOD table for the salcap tractor beam effect. NULL if not loaded. */
udword salCapBeamEffectLodTable;

/* Cone constant initialization flag and precomputed tan values */
static sdword salCapBeamWidthScaleInitialized;
static real32 salCapBeamNearWidthScale;
static real32 salCapBeamFarWidthScale;

Ship *GetFirstInsideShip(Ship *ship);
Ship *GetNextInsideShip(void);
Ship *shipGetCompoundAABB(Ship *ship, real32 *compoundAABB);
real64 shipDistanceToCompoundAABB(real32 *point, Ship *ship);
void partInverselyTransform(vector *out, vector *in, matrix *mat, vector *localPos);

static Ship *salCapProcessingShip = NULL;
static real32 salCapCloakingMessageTime = 0.0f;
static real32 salCapNoTargetMessageTime = 0.0f;
static sbyte salCapNullHostQueueActiveIndex = -1;
static udword salCapNullTrackedTargetState[10];
static ubyte salCapNullHostQueue[SALCAP_HOST_QUEUE_ENTRY_SIZE * 16];
static const sdword crystalProcessingEdgePairs[12][2] =
{
    { 0, 1 },
    { 1, 2 },
    { 2, 3 },
    { 3, 0 },
    { 4, 5 },
    { 5, 6 },
    { 6, 7 },
    { 7, 4 },
    { 0, 4 },
    { 1, 5 },
    { 2, 6 },
    { 3, 7 }
};

#define CRYSTAL_PROCESSING_EDGE_BURST_COUNT 5
#define CRYSTAL_PROCESSING_CENTER_WEIGHT 0.5f
#define CRYSTAL_PROCESSING_RANDOM_MIN 0.5f
#define CRYSTAL_PROCESSING_RANDOM_MAX 5.0f
#define CRYSTAL_PROCESSING_RANDOM_SCALE (1.0f / 4294967296.0f)
#define CRYSTAL_PROCESSING_BURST_SPEED_DIVISOR 5.0f

/* -----------------------------------------------------------------------------
   ShipSpecifics overlays for salvage/capture runtime state.

   Each salvage-capable ship type stores its SalCapCorvetteSpec (or host queue)
   at a type-specific offset inside the post-Ship ShipSpecifics payload, which
   begins at Ship+0x638. The offsets below are confirmed against cataclysm.exe;
   the static_asserts pin each named field to the exact absolute Ship offset that
   the retail binary reads. ShipSpecifics is the last Ship field, so the overlays
   are cast onto ship->ShipSpecifics and a leading pad places the field at its
   absolute offset.
   ----------------------------------------------------------------------------- */
typedef struct
{
    ubyte pad[0x640 - 0x638];
    SalCapCorvetteSpec spec;
} SalCapCorvetteSpecOverlay;        /* kSalCapCorvette / tSalCapCorvette: Ship+0x640 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(SalCapCorvetteSpecOverlay, spec) == 0x640);

typedef struct
{
    ubyte pad[0x644 - 0x638];
    SalCapCorvetteSpec spec;
} WorkerSalCapSpecOverlay;          /* sWorker / bWorker: Ship+0x644 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(WorkerSalCapSpecOverlay, spec) == 0x644);

typedef struct
{
    ubyte pad[0x774 - 0x638];
    SalCapCorvetteSpec spec;
} P1StandardSalCapSpecOverlay;      /* p1StandardCorvette: Ship+0x774 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(P1StandardSalCapSpecOverlay, spec) == 0x774);

typedef struct
{
    SalCapHostQueueEntry hostQueue[16];
    sbyte activeIndex;
} ProcessorSalCapHostQueueOverlay;  /* sProcessor / bProcessor / p1Mothership: Ship+0x638 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(ProcessorSalCapHostQueueOverlay, hostQueue) == 0x638);

typedef struct
{
    ubyte pad[0x650 - 0x638];
    SalCapHostQueueEntry hostQueue[16];
    sbyte activeIndex;
} HeavyCruiserSalCapHostQueueOverlay;  /* bHeavyCruiser: Ship+0x650 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(HeavyCruiserSalCapHostQueueOverlay, hostQueue) == 0x650);

typedef struct
{
    ubyte pad[0x654 - 0x638];
    SalCapHostQueueEntry hostQueue[16];
    sbyte activeIndex;
} CarrierSalCapHostQueueOverlay;    /* sCarrier / bCarrier / kCarrier / tCarrier / dKushanCarrier: Ship+0x654 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(CarrierSalCapHostQueueOverlay, hostQueue) == 0x654);

typedef struct
{
    ubyte pad[0x6E4 - 0x638];
    SalCapHostQueueEntry hostQueue[16];
    sbyte activeIndex;
} MothershipSalCapHostQueueOverlay; /* sMothership / bMothership / trGhostship: Ship+0x6E4 */
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(MothershipSalCapHostQueueOverlay, hostQueue) == 0x6E4);

static_assert(sizeof(SalCapHostQueueEntry) == SALCAP_HOST_QUEUE_ENTRY_SIZE);
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(SalCapCorvetteSpec, salvageState) == 0x640);
static_assert(offsetof(Ship, magnitudeSquared) == 0x33C);
static_assert(offsetof(Ship, posinfo) + offsetof(PosInfo, velocity) == 0x64);
static_assert(offsetof(CommandToDo, specialtargets) == 700);
static_assert(offsetof(SalCapCorvetteStatics, HealthThreshold) == 0xC);
static_assert(offsetof(SalCapCorvetteStatics, getTechTime) == 20);

static ubyte *salCapGetProcessingState(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sWorker:
        case bWorker:
            return (ubyte *)&((WorkerSalCapSpecOverlay *)ship->ShipSpecifics)->spec;
        case kSalCapCorvette:
        case tSalCapCorvette:
            return (ubyte *)&((SalCapCorvetteSpecOverlay *)ship->ShipSpecifics)->spec;
        case p1StandardCorvette:
            return (ubyte *)&((P1StandardSalCapSpecOverlay *)ship->ShipSpecifics)->spec;
        default:
            return salCapNullHostQueue;
    }
}

static ubyte *salCapGetSpecificsState(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sWorker:
        case bWorker:
            return (ubyte *)&((WorkerSalCapSpecOverlay *)ship->ShipSpecifics)->spec;
        case kSalCapCorvette:
        case tSalCapCorvette:
            return (ubyte *)&((SalCapCorvetteSpecOverlay *)ship->ShipSpecifics)->spec;
        case p1StandardCorvette:
            return (ubyte *)&((P1StandardSalCapSpecOverlay *)ship->ShipSpecifics)->spec;
        default:
            return salCapNullHostQueue;
    }
}

static ShipCrystalProcessingOverlay *crystalGetProcessingState(Ship *crystal)
{
    return &crystal->crystalProcessingState;
}

static CrystalStaticInfo *crystalStaticInfo(Ship *crystal)
{
    return (CrystalStaticInfo *)crystal->staticinfo->custstatinfo;
}

static CrystalShipSpecifics *crystalShipSpecifics(Ship *crystal)
{
    return (CrystalShipSpecifics *)crystal->ShipSpecifics;
}

static void SalCapDetachShipFromCommand(Ship *ship)
{
    ship->cataBusyStateWindow.dockingship = NULL;
    clDetachShipFromCommand(ship);
    RemoveShipFromCommand(ship);
}

static bool32 salCapIsCrystal(Ship *ship)
{
    return ship != NULL && (ship->shiptype == dCrystal0 || ship->shiptype == dCrystal1);
}

static bool32 salCapPlayerHasUpgrade(Player *player, udword upgradeMask, ShipType excludedShipType)
{
    Node *node = universe.ShipList.head;

    while (node != NULL)
    {
        Ship *ship = (Ship *)node->structptr;
        if (ship->playerowner == player &&
            ShipHasUpgrade(ship, upgradeMask) &&
            (excludedShipType == (ShipType)-1 || ship->shiptype != excludedShipType))
        {
            return TRUE;
        }
        node = node->next;
    }
    return FALSE;
}

static bool32 salCapPlayerHasBoardingUpgrade(Player *player)
{
    return salCapPlayerHasUpgrade(player, 0x10000, bCruiseMissile);
}

static bool32 salCapTargetHasActiveSalvageTarget(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)salCapGetSpecificsState(ship);
    return spec->target != NULL;
}

static bool32 salCapTargetBlocksSpecialCapture(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sMothership:
        case bMothership:
        case trFloatingCity:
        case trCargoBarge:
        case trMiningBase:
        case trResearchStation:
        case trJunkYardDawg:
        case trJunkYardHQ:
        case trJunk_LGun:
        case trJunk_SGun:
        case dBeastMothership:
            return TRUE;

        case kGravWellGenerator:
        case tGravWellGenerator:
            return ((GravWellGeneratorSpec *)ship->ShipSpecifics)->GravFieldOn != 0;

        default:
            return FALSE;
    }
}

static sdword salCapGetTargetAttachmentCapacity(Ship *ship)
{
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;

    if (dockStaticInfo == NULL)
    {
        return 0;
    }

    /* Retail (0x4f84a3, 0x4fb2cc) sums numPointsOfType[DPT_Salvage] (byte +62)
       and numPointsOfType[DPT_SalvageLatch] (byte +63) relative to dockStaticInfo,
       not DPT_SalvageLatch + DPT_SalvageProcess. */
    return dockStaticInfo->numPointsOfType[DPT_Salvage] +
           dockStaticInfo->numPointsOfType[DPT_SalvageLatch];
}

static sdword *salCapGetPlayerTargetClaims(Ship *ship)
{
    /* Retail (SalCapPickBestTargetFromSelection, IDA 0x4f84b5) reads the per-player claims
       array as *(DWORD*)&target->cataRuntimeState[4*playerIndex+76], which resolves to the
       absolute byte offset immediately following retail's cataRuntimeState scratch region.
       Our reconstructed Ship struct's specialFlags3 field lands 4 bytes short of that true
       offset (it's still inside the scratch region); the byte position that actually matches
       retail's evidenced claims[0] slot is specialFlags4 (validated against independently
       IDA-confirmed downstream Ship offsets such as shiptype@0x2B0 and playerowner@0x32C,
       which rules out growing the scratch region itself). specialFlags3 must not be reused
       here since it retains its own confirmed SPECIAL3_* flag semantics elsewhere.
     */
    return (sdword *)&ship->specialFlags4;
}

bool32 salCapCantDoFormation(Ship *ship)
{
    switch (ship->tactics_ordertype)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
        case p1StandardCorvette:
            return TRUE;

        case trJunkYardDawg:
            telldir();
            return FALSE;

        default:
            return FALSE;
    }
}

typedef struct
{
    real32 salvageFlyToDistance;
    DockStaticPoint *centerDockPoint;
} SalCapHostQueueScriptData;

typedef struct
{
    udword state;
    SpaceObjRotImpTargGuidanceShipDerelict *trackedTarget;
    Ship *processingShip;
    udword unk_0C;
    udword unk_10;
    real32 timeoutStart;
    real32 timeoutEnd;
    udword unk_1C;
    DockStaticPoint *dockPoint;
    ubyte clearCommand;
    ubyte pad[3];
} SalCapTrackedTargetState;

static bool32 SalCapCanTargetShipWithReasonCountsImpl(Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target, uword *reasonCounts);
SpaceObjRotImpTargGuidanceShipDerelict *SalCapPickBestTargetFromSelection(Ship *ship, SelectAnyCommand *selection, sdword *pickStatus);
bool32 SalCapExecuteSpecialOrder(Ship *ship, void *custom);
sdword SalCapInitHostQueueScriptData(char *directory, char *filename, ShipStaticInfo *shipStaticInfo);
ubyte SalCapProcessHostQueueScriptData(Ship *ship);
Ship *SalCapIterAttachedShip(Ship *target);
sdword SalCapFinalizeCapturedTarget(Ship *ship);
udword *SalCapGetTrackedTargetState(Ship *ship);
SalCapTrackedTargetState *SalCapGetTrackedTargetRuntimeState(Ship *ship);
static void SalCapReleaseCrystalDockReservation(Ship *capturedShip, Ship *hostShip);
static bool32 SalCapIsTrackedTargetShipType(ShipType shiptype);

// Cataclysm-specific salvage overlay lookup shared by formation/collision helpers.
// Retail SalCapExecuteSpecialOrder (0x4faad0) inlines an equivalent switch on
// ship->shiptype (0x4faaf1) to resolve this same per-shiptype tail overlay; we reuse
// SalCapGetTrackedTargetRuntimeState here since it already resolves that overlay (via
// ship->ShipSpecifics) with the exact SalCapTrackedTargetState layout collision.c reads
// (trackedTarget at +4, unk_10 at +16) and already has a non-NULL default fallback,
// matching retail's default-case static dummy (byte_A57770) instead of NULL.
ubyte *SalvagerFunc(Ship *ship)
{
    return (ubyte *)SalCapGetTrackedTargetRuntimeState(ship);
}

static SalCapHostQueueScriptData salCapNullHostQueueScriptData;

scriptStructEntry SalCapCorvetteStaticTable[] =
{
    { "HealthThreshold", scriptSetReal32CB, (udword)&(SalCapCorvetteStaticRace1.HealthThreshold), (udword)&(SalCapCorvetteStaticRace1) },
    { "healthRemovedPerSecond", scriptSetReal32CB, (udword)&(SalCapCorvetteStaticRace1.healthRemovedPerSecond), (udword)&(SalCapCorvetteStaticRace1) },
    { "getTechTime", scriptSetReal32CB, (udword)&(SalCapCorvetteStaticRace1.getTechTime), (udword)&(SalCapCorvetteStaticRace1) },
    { "flyToDistance", scriptSetReal32CB, (udword)&(SalCapCorvetteStaticRace1.flyToDistance), (udword)&(SalCapCorvetteStaticRace1) },
    { "maxPushingVelocitySingle", scriptSetReal32CB, (udword)&(SalCapCorvetteStaticRace1.maxPushingVelocitySingle), (udword)&(SalCapCorvetteStaticRace1) },
    { "noLightClampingDistance", scriptSetReal32CB, (udword)&(SalCapCorvetteStaticRace1.noLightClampingDistance), (udword)&(SalCapCorvetteStaticRace1) },
    { NULL, NULL, 0, 0 }
};

scriptStructEntry SalCapHostQueueScriptTable[] =
{
    { "salvageFlyToDistance", scriptSetReal32CB, (udword)&salCapNullHostQueueScriptData.salvageFlyToDistance, (udword)&salCapNullHostQueueScriptData },
    { NULL, NULL, 0, 0 }
};

udword salvageNumNeeded(Ship *ship, SpaceObjRotImpTarg *target);
void salvageSetDockVector(Ship *ship, Ship *dockwith, vector *trackHeading, vector *trackUp, vector *coneheadingInWorldCoordSys);
bool32 salvageTargetTrackVector(Ship *ship, vector *trackvector);
bool32 handOffTargetToDockWith(Ship *ship);
void CleanSalCapSpec(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    spec->target = NULL;
    if (!spec->groupme)
    {
        spec->dockwith = NULL;
    }
    spec->dockindex = 0;
    spec->salvageIndex = NO_INDEX;
    spec->salvageState = SALVAGE_BEGIN;
    spec->salvageAttributes = 0;
    spec->timeCounter = 0.0f;
    spec->getTechTime = 0.0f;
    spec->tractorBeam = FALSE;
    spec->noAvoid = FALSE;
    bitClear(ship->specialFlags1C8, CATA_RUNTIME_BROKEN_FORMATION);
}

void SalCapCorvetteInit(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);
    spec->groupme = FALSE;
    CleanSalCapSpec(ship);
    spec->noDamageTarget = NULL;
    spec->noDamageTargetTime = 0.0f;
    ship->specialFlags |= SPECIAL_IsASalvager;
}

static void salCapUnBusySalvagePoint(Ship *ship, SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    SalvageInfo *salvageInfo = (target->objtype == OBJ_DerelictType) ? ((Derelict *)target)->salvageInfo : NULL;

    if (salvageInfo != NULL)
    {
        salvageInfo->salvagePoints[spec->salvageIndex].busy = 0;
        salvageInfo->numSalvagePointsFree++;
        if (ship->clampInfo != NULL)
        {
            salvageInfo->numNeededForSalvage++;
            if (salvageInfo->numNeededForSalvage > spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage)
                salvageInfo->numNeededForSalvage = spec->target->staticinfo->salvageStaticInfo->numNeededForSalvage;
        }
    }
    spec->salvageIndex = NO_INDEX;
}

void SalCapOrderChangedCleanUp(Ship *ship)
{
    bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE);
    ship->attackvars.attacktarget = NULL;
    ship->kamikazeState = 0;
    SalCapDropTargetAndRememberPreviousTarget(ship);
    bitClear(ship->specialFlags1C8, CATA_RUNTIME_BROKEN_FORMATION);
}

void salCapRemoveDerelictReferences(Ship *ship, Derelict *d)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    if ((SpaceObjRotImpTargGuidanceShipDerelict *)d == spec->target)
    {
        bitClear(ship->specialFlags, SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
        SalCapOrderChangedCleanUp(ship);
    }
    if ((SpaceObjRotImpTargGuidanceShipDerelict *)d == spec->noDamageTarget)
    {
        spec->noDamageTargetTime = 0.0f;
        spec->noDamageTarget = NULL;
    }
}

void SalCapDropTarget(Ship *ship)
{
    udword *trackedState = SalCapGetTrackedTargetState(ship);
    Ship *target;
    Ship *processingShip;
    Ship *attachedShip;
    DockStaticPoint *dockPoint;
    clSetAttribute((sdword)ship, 0, 2);

    ship->specialFlags2 &= ~0x40u;
    if (ship->clampInfo != NULL)
    {
        clDetachShipFromCommand(ship);
        trackedState[5] = TreatAsUdword(universe.totaltimeelapsed);
        trackedState[6] = trackedState[1];
    }

    if (ship->rceffect != NULL)
    {
        etgEffectDelete((Effect *)ship->rceffect);
        univRemoveObjFromRenderList((SpaceObj *)ship->rceffect);
        listDeleteNode(&ship->rceffect->objlink);
        trackedState[3] = 0;
        ship->rceffect = NULL;
    }

    dockPoint = (DockStaticPoint *)trackedState[8];
    if (dockPoint != NULL)
    {
        sdword workerOpenAnimIndex = salCapGetWorkerOpenAnimIndex(ship->shiptype);

        if (workerOpenAnimIndex != -1 &&
            workerOpenAnimIndex == *(sdword *)(ship->runtimeAnimationOverlay.madAnimationData + 12))
        {
            sdword workerCloseAnimIndex = salCapGetWorkerCloseAnimIndex(ship->shiptype);
            if (workerCloseAnimIndex != -1 &&
                *(sdword *)(ship->runtimeAnimationOverlay.madAnimationData + 12) != workerCloseAnimIndex)
            {
                madAnimationStart(ship, workerCloseAnimIndex);
            }
        }

        target = (Ship *)trackedState[1];
        if (target != NULL && target->dockvars.dockPointActivity != NULL)
        {
            target->dockvars.dockPointActivity->reserved[dockPoint->dockindex] = 0;
            target->dockvars.dockPointActivity->numDockedOfType[dockPoint->type]--;
        }
        trackedState[8] = 0;
    }

    target = (Ship *)trackedState[1];
    processingShip = (Ship *)trackedState[2];
    if (target != NULL)
    {
        if (target->shiptype == dBigGunUnit &&
            ship->specialTargetShip != NULL &&
            ship->specialTargetShip->shiptype == dBigGun)
        {
            ship->specialTargetShip = NULL;
        }

        salCapGetPlayerTargetClaims(target)[ship->playerowner->playerIndex]--;
        if (salCapGetPlayerTargetClaims(target)[ship->playerowner->playerIndex] < 0)
        {
            salCapGetPlayerTargetClaims(target)[ship->playerowner->playerIndex] = 0;
        }
        if (salCapGetPlayerTargetClaims(target)[ship->playerowner->playerIndex] == 0)
        {
            SALCAP_TARGET_CLAIM_OWNER(target) = (udword)-1;
        }

        if (processingShip != NULL)
        {
            if (SalCapIterAttachedShip(target) == NULL && target->specialTargetShip == processingShip)
            {
                target->specialTargetShip = NULL;
            }

            if (target->clampInfo == NULL && target->crystalDockSlotIndex != -1)
            {
                SalCapReleaseCrystalDockReservation(target, processingShip);
            }
        }

        if (SALCAP_TARGET_CLAIM_OWNER(target) != (udword)-1 &&
            salCapGetPlayerTargetClaims(target)[SALCAP_TARGET_CLAIM_OWNER(target)] > 0)
        {
            bool32 claimOwnerStillAttached = FALSE;

            for (attachedShip = SalCapIterAttachedShip(target);
                 attachedShip != NULL;
                 attachedShip = SalCapIterAttachedShip(NULL))
            {
                SalCapTrackedTargetState *attachedState;

                if (!SalCapIsTrackedTargetShipType(attachedShip->shiptype))
                {
                    continue;
                }

                attachedState = SalCapGetTrackedTargetRuntimeState(attachedShip);
                if (attachedShip->playerowner->playerIndex == SALCAP_TARGET_CLAIM_OWNER(target) &&
                    attachedState->trackedTarget == (SpaceObjRotImpTargGuidanceShipDerelict *)target &&
                    attachedState->state > SALCAP_SPECIAL_FLY_TO_TARGET)
                {
                    claimOwnerStillAttached = TRUE;
                    break;
                }
            }

            if (!claimOwnerStillAttached)
            {
                SALCAP_TARGET_CLAIM_OWNER(target) = (udword)-1;
            }
        }

        if (SalCapIterAttachedShip(target) == NULL)
        {
            SALCAP_TARGET_LAST_SUPPORT_MESSAGE_RAW(target) = SALCAP_SPECIAL_SUPPORT_MESSAGE_SENTINEL;
            SALCAP_TARGET_RUNTIME_FLAGS(target) &= ~0x02000000u;
        }
        if (target->clampInfo == NULL && SalCapIterAttachedShip(target) == NULL)
        {
            target->flags &= ~SOF_Slaveable;
            target->flags &= ~SOF_TargetableHW1;
        }
        if (target->clampInfo == NULL)
        {
            target->flags |= SOF_Selectable;
        }

        trackedState[1] = 0;
    }

    trackedState[0] = 0;
    trackedState[1] = 0;
    trackedState[2] = 0;
    trackedState[3] = 0;
    trackedState[4] = 0;
    trackedState[5] = 0;
    trackedState[6] = 0;
    trackedState[8] = 0;
    ship->specialFlags1C8 &= ~4u;
    ship->specialFlags1C8 &= ~CATA_RUNTIME_SALVAGE_CLAMP_APPROACH;
}

void salCapClearTechBool(void)
{
    universe.DerelictTech = FALSE;
}

void salCapCleanUpCloakingTarget(Ship *ship, Ship *shiptoremove)
{
    (void)ship;
    (void)shiptoremove;

    salCapProcessingShip = NULL;
    salCapCloakingMessageTime = 0.0f;
}

Ship *SalCapSetProcessingShip(Ship *ship)
{
    salCapProcessingShip = ship;
    return ship;
}

Ship *SalCapGetProcessingShip(void)
{
    return salCapProcessingShip;
}

Ship *SalCapNotifyProcessingShipRemoved(Ship *ship)
{
    ubyte *processingState;
    udword processingTarget;

    if (ship->flags2 != 0)
    {
        return ship;
    }

    switch (ship->shiptype)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
        case p1StandardCorvette:
            processingState = salCapGetProcessingState(ship);
            processingTarget = *(udword *)(processingState + 4);
            if (processingTarget != 0 && ship->clampInfo != NULL && *(udword *)ship->clampInfo == processingTarget)
            {
                processingState[0x24] = 1;
            }
            break;
        case JunkYardDawg:
            telldir();
            break;
        default:
            break;
    }

    return ship;
}

bool32 SalCapHasHostQueue(Ship *ship)
{
    return (ship->specialFlags2 & SPECIAL_2_CrystalDockSlot) != 0;
}

ubyte *SalCapGetHostQueue(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sCarrier:
        case bCarrier:
        case kCarrier:
        case tCarrier:
        case dKushanCarrier:
            return (ubyte *)((CarrierSalCapHostQueueOverlay *)ship->ShipSpecifics)->hostQueue;
        case sProcessor:
        case bProcessor:
        case p1Mothership:
            return (ubyte *)((ProcessorSalCapHostQueueOverlay *)ship->ShipSpecifics)->hostQueue;
        case sMothership:
        case bMothership:
        case trGhostship:
            return (ubyte *)((MothershipSalCapHostQueueOverlay *)ship->ShipSpecifics)->hostQueue;
        case bHeavyCruiser:
            return (ubyte *)((HeavyCruiserSalCapHostQueueOverlay *)ship->ShipSpecifics)->hostQueue;
        default:
            return salCapNullHostQueue;
    }
}

sbyte *SalCapGetHostQueueActiveIndexPtr(Ship *ship)
{
    ubyte *hostQueue = SalCapGetHostQueue(ship);

    if (hostQueue == salCapNullHostQueue)
    {
        return &salCapNullHostQueueActiveIndex;
    }

    return (sbyte *)(hostQueue + SALCAP_HOST_QUEUE_ENTRY_SIZE * 16);
}

udword *SalCapGetTrackedTargetState(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sWorker:
        case bWorker:
            return (udword *)&((WorkerSalCapSpecOverlay *)ship->ShipSpecifics)->spec;
        case kSalCapCorvette:
        case tSalCapCorvette:
            return (udword *)&((SalCapCorvetteSpecOverlay *)ship->ShipSpecifics)->spec;
        case p1StandardCorvette:
            return (udword *)&((P1StandardSalCapSpecOverlay *)ship->ShipSpecifics)->spec;
        default:
            return salCapNullTrackedTargetState;
    }
}

SalCapHostQueueScriptData *SalCapGetHostQueueScriptData(ShipStaticInfo *shipStaticInfo)
{
    switch (shipStaticInfo->shiptype)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
            return (SalCapHostQueueScriptData *)&((SalCapCorvetteStatics *)shipStaticInfo->custstatinfo)->HealthThreshold;
        case p1StandardCorvette:
            return (SalCapHostQueueScriptData *)shipStaticInfo->custstatinfo;
        default:
            return &salCapNullHostQueueScriptData;
    }
}

SalCapTrackedTargetState *SalCapGetTrackedTargetRuntimeState(Ship *ship)
{
    return (SalCapTrackedTargetState *)SalCapGetTrackedTargetState(ship);
}

sdword SalCapCleanupHostQueueEntry(Ship *hostShip, sdword entryIndex)
{
    SalCapHostQueueEntry *entry = (SalCapHostQueueEntry *)(SalCapGetHostQueue(hostShip) + entryIndex * SALCAP_HOST_QUEUE_ENTRY_SIZE);
    Ship *capturedShip = entry->ship;

    if (capturedShip != NULL && capturedShip->command != NULL && capturedShip->command->selection != NULL)
    {
        SelectCommand *selection = capturedShip->command->selection;
        sdword i;

        for (i = selection->numShips - 1; i >= 0; i--)
        {
            Ship *salvager = selection->ShipPtr[i];

            if (salvager->objtype != OBJ_ShipType)
            {
                continue;
            }

            switch (salvager->shiptype)
            {
                case sWorker:
                case bWorker:
                case kSalCapCorvette:
                case tSalCapCorvette:
                case p1StandardCorvette:
                {
                    CommandToDo *shipAndItsCommand;
                    sdword pickStatus = 0;
                    SelectCommand removeSelection;
                    SelectAnyCommand *specialTargets;

                    shipAndItsCommand = getShipAndItsCommand(&universe.mainCommandLayer, salvager);
                    specialTargets = (SelectAnyCommand *)shipAndItsCommand->specialtargets;

                    if (SalCapPickBestTargetFromSelection(salvager, specialTargets, &pickStatus) == NULL)
                    {
                        removeSelection.numShips = 1;
                        removeSelection.timeLastStatus = 0.0f;
                        removeSelection.ShipPtr[0] = salvager;
                        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &removeSelection);
                        GroupShip(&universe.mainCommandLayer, salvager, hostShip);
                    }
                    SalCapDropTarget(salvager);
                    break;
                }

                case trJunkYardDawg:
                    telldir();
                    break;

                default:
                    break;
            }
        }
    }

    return 0;
}

real32 CrystalGetProcessingRemainingFraction(Ship *crystal)
{
    Timer *timer = &crystalGetProcessingState(crystal)->timer;
    real32 currentTime = timer->enabled ? taskTimeElapsed : universe.totaltimeelapsed;
    real32 remainingFraction;

    if (timer->duration <= 0.0f)
    {
        telldir();
        return 0.0f;
    }

    remainingFraction = (currentTime - timer->startTime) / timer->duration;

    if (remainingFraction > 1.0f)
    {
        return 1.0f;
    }
    return remainingFraction;
}

void CrystalStartProcessing(Ship *crystal, vector *processingTarget, real32 duration)
{
    ShipCrystalProcessingOverlay *state = crystalGetProcessingState(crystal);
    ClampInfo *clampInfo;
    SelectCommand *selection;
    Ship *attachedShip;

    if (state->active != 0)
    {
        return;
    }

    for (clampInfo = crystal->clampInfo; clampInfo != NULL; clampInfo = crystal->clampInfo)
    {
        selection = (SelectCommand *)clampInfo->selection;
        if (selection == NULL || selection->numShips <= 0)
        {
            break;
        }

        attachedShip = selection->ShipPtr[0];
        attachedShip->flags &= ~SOF_ForwardDamageToClampHost;
        clDetachShipFromCommand(attachedShip);
        ApplyDamageToTarget((SpaceObjRotImpTarg *)attachedShip, 1.0e21f, GS_LargeEnergyCannon, 666, attachedShip->playerowner->playerIndex);
    }

    if (duration <= 0.0f)
    {
        duration = 10.0f;
    }

    state->active = 1;
    TimerInitTimerStruct(&state->timer, duration);
    state->target = *processingTarget;
    CrystalEnsureProcessingEffect(processingTarget, crystal);
}

void CrystalUpdateProcessingTarget(Ship *crystal, vector *processingTarget)
{
    crystalGetProcessingState(crystal)->target = *processingTarget;
    CrystalEnsureProcessingEffect(processingTarget, crystal);
}

static sdword SalCapUpdateTowedTargetMotion(Ship *ship)
{
    vector heading = ship->moveTo;

    vecNormalize(&heading);
    aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &heading, 0.99900001f, 0.99900001f, 0.0f, FALSE);
    return 0;
}

static Effect *CrystalEnsureProcessingEffect(vector *processingTarget, Ship *crystal)
{
    ShipCrystalProcessingOverlay *state = crystalGetProcessingState(crystal);
    etglod *effectLOD;
    vector heading;
    vector zeroVelocity = { 0.0f, 0.0f, 0.0f };
    matrix coordsys;
    etgeffectstatic *stat;
    real32 length;
    real32 width;
    real32 duration;
    real32 isCrystalParam;

    heading.x = crystal->posinfo.position.x - processingTarget->x;
    heading.y = crystal->posinfo.position.y - processingTarget->y;
    heading.z = crystal->posinfo.position.z - processingTarget->z;
    length = fsqrt(heading.x * heading.x + heading.y * heading.y + heading.z * heading.z);
    vecNormalize(&heading);
    matCreateCoordSysFromHeading(&coordsys, &heading);

    effectLOD = etgSpecialPurposeEffectTable[EGT_RESOURCE_SHIP];
    stat = (effectLOD != NULL && effectLOD->nLevels > 0) ? effectLOD->level[0] : NULL;
    if (stat == NULL || etgFrequencyExceeded(stat))
    {
        return state->effect;
    }

    if (state->effect != NULL)
    {
        state->effect->posinfo.position = *processingTarget;
        state->effect->rotinfo.coordsys = coordsys;
        return state->effect;
    }

    width = crystal->staticinfo->staticheader.staticCollInfo.collspheresize * 0.5f;
    duration = (real32)TimerRemaining((int)&state->timer);
    isCrystalParam = IsCrystal(crystal) ? 1.0f : 0.0f;

    state->effect = etgEffectCreate(
        stat,
        NULL,
        processingTarget,
        &zeroVelocity,
        &coordsys,
        1.0f,
        32,
        4,
        TreatAsUdword(length),
        TreatAsUdword(width),
        TreatAsUdword(duration),
        TreatAsUdword(isCrystalParam));
    return state->effect;
}

static bool32 CrystalIntersectSlicePlaneSegment(real32 *planeEquation, real32 *segmentLine, vector *intersection)
{
    real32 denom;
    real32 t;

    denom = planeEquation[0] * segmentLine[3] +
            planeEquation[1] * segmentLine[4] +
            planeEquation[2] * segmentLine[5];
    if (denom == 0.0f)
    {
        return FALSE;
    }

    t = -(planeEquation[0] * segmentLine[0] +
          planeEquation[1] * segmentLine[1] +
          planeEquation[2] * segmentLine[2] +
          planeEquation[3]) / denom;
    if (t <= 0.0f || t >= 1.0f)
    {
        return FALSE;
    }

    intersection->x = segmentLine[0] + t * segmentLine[3];
    intersection->y = segmentLine[1] + t * segmentLine[4];
    intersection->z = segmentLine[2] + t * segmentLine[5];
    return TRUE;
}

static sdword CrystalSpawnProcessingEdgeEffects(char *randomFile, sdword randomLine, Ship *crystal, sdword *points, vector *processingCenter)
{
    sdword burstIndex;
    sdword result = CRYSTAL_PROCESSING_EDGE_BURST_COUNT;
    etglod *effectLOD = etgSpecialPurposeEffectTable[EGT_SUCK_SHIP];
    etgeffectstatic *stat = (effectLOD != NULL && effectLOD->nLevels > 0) ? effectLOD->level[0] : NULL;

    for (burstIndex = 0; burstIndex < CRYSTAL_PROCESSING_EDGE_BURST_COUNT; burstIndex++)
    {
        vector weightedCenter;
        real32 weightTotal;
        real32 *pointCursor;
        sdword pointIndex;
        vector worldPoint;
        vector averagePoint;
        vector heading;
        vector processingTarget;
        matrix coordsys;

        weightedCenter.x = CRYSTAL_PROCESSING_CENTER_WEIGHT * processingCenter->x;
        weightedCenter.y = CRYSTAL_PROCESSING_CENTER_WEIGHT * processingCenter->y;
        weightedCenter.z = CRYSTAL_PROCESSING_CENTER_WEIGHT * processingCenter->z;
        weightTotal = CRYSTAL_PROCESSING_CENTER_WEIGHT;

        for (pointIndex = 0, pointCursor = (real32 *)(points + 1); pointIndex < *points; pointIndex++, pointCursor += 3)
        {
            real32 weight;

            weight =
                (real32)ranRandom(0) *
                    ((CRYSTAL_PROCESSING_RANDOM_MAX - CRYSTAL_PROCESSING_RANDOM_MIN) * CRYSTAL_PROCESSING_RANDOM_SCALE) +
                CRYSTAL_PROCESSING_RANDOM_MIN;
            weightedCenter.x += pointCursor[0] * weight;
            weightedCenter.y += pointCursor[1] * weight;
            weightedCenter.z += pointCursor[2] * weight;
            weightTotal += weight;
        }

        weightedCenter.x *= 1.0f / weightTotal;
        weightedCenter.y *= 1.0f / weightTotal;
        weightedCenter.z *= 1.0f / weightTotal;

        matMultiplyMatByVec(&worldPoint, &crystal->rotinfo.coordsys, &weightedCenter);
        averagePoint.x = worldPoint.x + crystal->posinfo.position.x;
        averagePoint.y = worldPoint.y + crystal->posinfo.position.y;
        averagePoint.z = worldPoint.z + crystal->posinfo.position.z;

        processingTarget = crystalGetProcessingState(crystal)->target;
        vecSub(heading, processingTarget, averagePoint);
        vecNormalize(&heading);
        vecMultiplyByScalar(heading, GetDistance(&processingTarget, &averagePoint) / CRYSTAL_PROCESSING_BURST_SPEED_DIVISOR);

        if (stat == NULL || etgFrequencyExceeded(stat))
        {
            result = burstIndex + 1;
            continue;
        }

        {
            vector normalizedHeading = heading;

            vecNormalize(&normalizedHeading);
            matCreateCoordSysFromHeading(&coordsys, &normalizedHeading);
            etgEffectCreate(stat, NULL, &averagePoint, &heading, &coordsys, 1.0f, 120, 0);
        }
        result = burstIndex + 1;
    }

    return result;
}

real32 *CrystalBuildProcessingSlicePlane(Ship *ship, real32 *planeEquation)
{
    vector localShipPosition;
    vector localTargetPosition;
    vector heading;
    real32 processingSpeed;
    real32 timerRemaining;
    real32 pointX;
    real32 pointY;
    real32 pointZ;
    real32 offsetScale;

    processingSpeed = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    timerRemaining = TimerTimeRemaining(&crystalGetProcessingState(ship)->timer);
    if (timerRemaining > 1.0f)
    {
        timerRemaining = 1.0f;
    }

    partInverselyTransform(&localShipPosition, &ship->posinfo.position, &ship->rotinfo.coordsys, &ship->posinfo.position);
    partInverselyTransform(&localTargetPosition, &crystalGetProcessingState(ship)->target, &ship->rotinfo.coordsys, &ship->posinfo.position);

    vecSub(heading, localTargetPosition, localShipPosition);
    vecNormalize(&heading);

    offsetScale = timerRemaining + timerRemaining;
    pointX = localShipPosition.x + heading.x * processingSpeed - offsetScale * heading.x * processingSpeed;
    pointY = localShipPosition.y + heading.y * processingSpeed - offsetScale * heading.y * processingSpeed;
    pointZ = localShipPosition.z + heading.z * processingSpeed - offsetScale * heading.z * processingSpeed;

    planeEquation[0] = -heading.x;
    planeEquation[1] = -heading.y;
    planeEquation[2] = -heading.z;
    planeEquation[3] = -(
        pointX * planeEquation[0] +
        pointY * planeEquation[1] +
        pointZ * planeEquation[2]);
    return planeEquation;
}

bool32 CrystalUpdateProcessingVisuals(Ship *crystal)
{
    ShipCrystalProcessingOverlay *state;
    real32 planeEquation[4];
    vector transformedPoints[8];
    struct
    {
        sdword count;
        vector points[12];
    } intersections;
    sdword i;

    if ((crystal->flags & SOF_Dead) != 0)
    {
        return TRUE;
    }

    state = crystalGetProcessingState(crystal);
    if (state->visualsEnabled != 1 || state->active != 1)
    {
        return TRUE;
    }

    if (TimerExpired((int)&state->timer))
    {
        return TRUE;
    }

    CrystalBuildProcessingSlicePlane(crystal, planeEquation);
    for (i = 0; i < 8; i++)
    {
        matMultiplyVecByMat(&transformedPoints[i], &crystal->collInfo.rectpos[i], &crystal->rotinfo.coordsys);
    }

    intersections.count = 0;
    for (i = 0; i < (sdword)(sizeof(crystalProcessingEdgePairs) / sizeof(crystalProcessingEdgePairs[0])); i++)
    {
        vector segmentIntersection;
        real32 segmentLine[6];
        const sdword a = crystalProcessingEdgePairs[i][0];
        const sdword b = crystalProcessingEdgePairs[i][1];

        segmentLine[0] = transformedPoints[a].x;
        segmentLine[1] = transformedPoints[a].y;
        segmentLine[2] = transformedPoints[a].z;
        segmentLine[3] = transformedPoints[b].x - transformedPoints[a].x;
        segmentLine[4] = transformedPoints[b].y - transformedPoints[a].y;
        segmentLine[5] = transformedPoints[b].z - transformedPoints[a].z;
        if (CrystalIntersectSlicePlaneSegment(planeEquation, segmentLine, &segmentIntersection))
        {
            intersections.points[intersections.count] = segmentIntersection;
            intersections.count++;
        }
    }

    if (intersections.count >= 3)
    {
        vector processingCenter = { 0.0f, 0.0f, 0.0f };

        for (i = 0; i < intersections.count; i++)
        {
            processingCenter.x += intersections.points[i].x;
            processingCenter.y += intersections.points[i].y;
            processingCenter.z += intersections.points[i].z;
        }
        processingCenter.x /= intersections.count;
        processingCenter.y /= intersections.count;
        processingCenter.z /= intersections.count;
        CrystalSpawnProcessingEdgeEffects(__FILE__, __LINE__, crystal, &intersections.count, &processingCenter);
    }

    if (state->effect == NULL)
    {
        return CrystalEnsureProcessingEffect(&state->target, crystal) != NULL;
    }
    return TRUE;
}

#if !1
sdword PreFix_SalvagerSpecifics(Ship *ship)
{
ubyte *state;
DockStaticPoint *dockPoint;

state = salCapGetSpecificsState(ship);
*(sdword *)(state + 4) = SpaceObjRegistryGetID((SpaceObj *)*(Ship **)(state + 4));
*(sdword *)(state + 8) = SpaceObjRegistryGetID((SpaceObj *)*(Ship **)(state + 8));
*(sdword *)(state + 24) = SpaceObjRegistryGetID((SpaceObj *)*(Ship **)(state + 24));

dockPoint = *(DockStaticPoint **)(state + 32);
if (dockPoint != NULL)
{
    *(sdword *)(state + 32) = dockPoint->dockindex;
    return dockPoint->dockindex;
}

*(sdword *)(state + 32) = -1;
return 0;
}

sdword Fix_SalvagerSpecifics(Ship *ship)
{
ubyte *state;
sdword dockIndex;

state = salCapGetSpecificsState(ship);
*(Ship **)(state + 4) = SpaceObjRegistryGetShip(*(sdword *)(state + 4));
*(Ship **)(state + 8) = SpaceObjRegistryGetShip(*(sdword *)(state + 8));
*(Ship **)(state + 24) = SpaceObjRegistryGetShip(*(sdword *)(state + 24));

dockIndex = *(sdword *)(state + 32);
if (dockIndex == -1)
{
    *(udword *)(state + 32) = 0;
    return 0;
}

*(DockStaticPoint **)(state + 32) = &ship->staticinfo->dockStaticInfo->dockstaticpoints[dockIndex];
return dockIndex;
}

void SalCapDropTargetCallback(Ship *ship)
{
SalCapDropTarget(ship);
}

sdword SalCapPlayerHasCrystalUpgrade(Player *player)
{
Node *node;

for (node = universe.ShipList.head; node != NULL; node = node->next)
{
    Ship *ship = node->structptr;

    if (ship->playerowner == player && ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST))
    {
        return TRUE;
    }
}
return FALSE;
}

sdword SalCapDropTargetAndRememberPreviousTarget(Ship *ship)
{
ubyte *state;
Ship *previousTarget;
sdword result;

state = salCapGetSpecificsState(ship);
previousTarget = *(Ship **)(state + 4);
result = (sdword)SalCapDropTarget(ship);
if (previousTarget != NULL)
{
    result = (sdword)LODWORD(universe.totaltimeelapsed);
    *(real32 *)(state + 20) = universe.totaltimeelapsed;
    *(Ship **)(state + 24) = previousTarget;
}
return result;
}

Effect *SalCapStartSalvageEffect(Ship *ship)
{
ubyte *state;
udword *effectTable;
etgeffectstatic *stat;
Effect *effect;

state = salCapGetSpecificsState(ship);
*(udword *)(state + 12) = 1;

effectTable = (udword *)etgTractorBeamEffectTable[ship->shiprace];
if (effectTable == NULL || ship->shiprace >= *effectTable)
{
    ship->rceffect = NULL;
    return NULL;
}

stat = (etgeffectstatic *)effectTable[ship->shiprace + 1];
if (stat == NULL || etgFrequencyExceeded(stat))
{
    ship->rceffect = NULL;
    return NULL;
}

effect = etgEffectCreate(stat, ship, NULL, NULL, NULL, ship->magnitudeSquared, 88, 0);
ship->rceffect = effect;
if (effect != NULL)
{
    effect->posinfo.velocity.x = ZERO;
    effect->posinfo.velocity.y = ZERO;
    effect->posinfo.velocity.z = ZERO;
    *(udword *)(state + 12) = 1;
    univUpdateObjRotInfo((SpaceObjRot *)effect);
    ship->soundevent.specialHandle = soundEventPlay((real32)(sdword)ship, 2070, 0);
}
return effect;
}

sdword SalCapHandleProcessingShipRemoved(Ship *ship, Ship *removedShip)
{
ubyte *state;
Ship *target;
Ship *processingShip;
Ship *nearestProcessingShip;
Ship *attachedShip;

state = salCapGetSpecificsState(ship);
target = *(Ship **)(state + 4);

if (singlePlayerGame &&
    singlePlayerGameInfo.currentMission == 2 &&
    removedShip->shiptype == dMessagePod &&
    removedShip == target &&
    (removedShip->flags & SOF_Dead) != 0)
{
    SalCapDropTarget(ship);
}

if (removedShip == target)
{
    SalCapDropTarget(ship);
}

processingShip = *(Ship **)(state + 8);
if (removedShip != processingShip)
{
    return (sdword)processingShip;
}

if (target != NULL && target->crystalDockSlotIndex != -1)
{
    RemoveShipFromCrystalDockSlots(processingShip, target);
    target->crystalDockSlotIndex = -1;
}

nearestProcessingShip = SalCapFindNearestProcessingShip((sdword)ship, (real32 *)target);
for (attachedShip = SalCapIterAttachedShip(target);
     attachedShip != NULL;
     attachedShip = SalCapIterAttachedShip(NULL))
{
    ubyte *attachedState = salCapGetSpecificsState(attachedShip);
    *(Ship **)(attachedState + 8) = nearestProcessingShip;
}

*(Ship **)(state + 8) = nearestProcessingShip;
if (nearestProcessingShip != NULL)
{
    *(sdword *)state = 9;
    return (sdword)nearestProcessingShip;
}

return SalCapFinalizeCapturedTarget(ship);
}

sdword SalCapCanHostProcessCapturedTarget(Ship *ship, Ship *target)
{
sdword canProcess;
sdword customIndex;
ubyte *customStatInfo;

canProcess = (ship->specialFlags4 & SPECIAL4_CrystalDockSlot) != 0;
if (!canProcess)
{
    return FALSE;
}

if (singlePlayerGame && ship->shiptype == kCarrier)
{
    return FALSE;
}

canProcess = (ship->specialFlags4 & SPECIAL4_ExternalBuildMember) == 0;
if (!canProcess)
{
    return FALSE;
}

if (!singlePlayerGame && !IsCrystal((sdword)target) && ship->shiprace == Race_Beast)
{
    canProcess = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT);
}
if (!canProcess)
{
    return FALSE;
}

switch (target->shiptype)
{
    case dBigGunUnit:
        customIndex = (ship->shiptype == sMothership) ? 18 : 29;
        break;
    case dMessagePod:
    case dBuoy:
        customIndex = 6;
        break;
    case dCrystal0:
    case dCrystal1:
        return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST);
    case dSlipgateSmall:
    case dSlipgateMedium:
    case dSlipgateLarge:
    case dCrate:
    case dJunk1PartA:
    case dJunk1Shell:
    case dJunk1Strut:
    case dJunk2Shipwreck:
    case dJunk3Boiler:
    case dJunk3BoilerCasing:
    case dFirstPlanet:
    case dDeadPlanet:
        customIndex = -1;
        break;
    default:
        return TRUE;
}

customStatInfo = ship->staticinfo->custstatinfo;
return customStatInfo[customIndex + 35] > 0;
}

Ship *SalCapFindNearestProcessingShip(sdword playerIndexOwnerShip, Ship *target)
{
Ship *ownerShip;
Ship *candidates[16];
Node *node;
sdword candidateCount;
Ship *bestShip;
real32 bestDistanceSquared;
sdword i;

ownerShip = (Ship *)playerIndexOwnerShip;
candidateCount = 0;
for (node = universe.ShipList.head; node != NULL; node = node->next)
{
    Ship *candidate = node->structptr;

    if (candidate->playerowner == ownerShip->playerowner &&
        (candidate->flags & SOF_Selectable) != 0 &&
        candidateCount < 16)
    {
        candidates[candidateCount++] = candidate;
    }
}

if (candidateCount == 0 || candidateCount >= 16)
{
    telldir();
}

bestShip = NULL;
bestDistanceSquared = REALlyBig;
for (i = 0; i < candidateCount; i++)
{
    Ship *candidate = candidates[i];
    real32 dx;
    real32 dy;
    real32 dz;
    real32 distanceSquared;

    if (!SalCapCanHostProcessCapturedTarget(candidate, target))
    {
        continue;
    }

    dx = target->posinfo.position.x - candidate->posinfo.position.x;
    dy = target->posinfo.position.y - candidate->posinfo.position.y;
    dz = target->posinfo.position.z - candidate->posinfo.position.z;
    distanceSquared = dx * dx + dy * dy + dz * dz;
    if (distanceSquared < bestDistanceSquared)
    {
        bestDistanceSquared = distanceSquared;
        bestShip = candidate;
    }
}

return bestShip;
}
#endif

void SalCapResetHostQueueScriptData(Ship *ship)
{
ubyte *state;

ship->specialFlags3 |= SPECIAL3_Unknown19;
SalCapDropTarget(ship);
state = salCapGetSpecificsState(ship);
state[36] = 0;
}

#pragma warning( 4 : 4047)

void SalCapCorvette_RegisterExtraSpaceObjs(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    (void)spec;
}

static sdword SalCapCanHostProcessCapturedTarget(Ship *ship, Ship *target)
{
    sdword canProcess;
    sdword customIndex;
    ubyte *customStatInfo;

    canProcess = (ship->specialFlags2 & SPECIAL_2_CrystalDockSlot) != 0;
    if (!canProcess)
    {
        return FALSE;
    }
    if (singlePlayerGame && ship->shiptype == kCarrier)
    {
        return FALSE;
    }
    if ((ship->specialFlags4 & SPECIAL4_ExternalBuildMember) != 0)
    {
        return FALSE;
    }
    if (!singlePlayerGame && !salCapIsCrystal(target) && ship->shiprace == RACE_Beast && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT))
    {
        return FALSE;
    }

    switch (target->shiptype)
    {
        case dBigGunUnit:
            customIndex = (ship->shiptype == sMothership) ? 18 : 29;
            break;
        case dMessagePod:
        case dBuoy:
            customIndex = 6;
            break;
        case dCrystal0:
        case dCrystal1:
            return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST);
        case dSlipgateSmall:
        case dSlipgateMedium:
        case dSlipgateLarge:
        case dCrate:
        case dJunk1PartA:
        case dJunk1Shell:
        case dJunk1Strut:
        case dJunk2Shipwreck:
        case dJunk3Boiler:
        case dJunk3BoilerCasing:
        case dFirstPlanet:
        case dDeadPlanet:
            customIndex = -1;
            break;
        default:
            return TRUE;
    }

    customStatInfo = ship->staticinfo->custstatinfo;
    return customStatInfo[customIndex + 35] > 0;
}

static Ship *SalCapFindNearestProcessingShip(Ship *ownerShip, Ship *target)
{
    Ship *candidates[16];
    Node *node;
    sdword candidateCount = 0;
    Ship *bestShip = NULL;
    real32 bestDistanceSquared = REALlyBig;
    sdword i;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *candidate = node->structptr;

        if (candidate->playerowner == ownerShip->playerowner &&
            (candidate->specialFlags2 & SPECIAL_2_CrystalDockSlot) != 0 &&
            candidateCount < 16)
        {
            candidates[candidateCount++] = candidate;
        }
    }

    if (candidateCount == 0 || candidateCount >= 16)
    {
        telldir();
    }

    for (i = 0; i < candidateCount; i++)
    {
        Ship *candidate = candidates[i];
        real32 dx;
        real32 dy;
        real32 dz;
        real32 distanceSquared;

        if (!SalCapCanHostProcessCapturedTarget(candidate, target))
        {
            continue;
        }

        dx = target->posinfo.position.x - candidate->posinfo.position.x;
        dy = target->posinfo.position.y - candidate->posinfo.position.y;
        dz = target->posinfo.position.z - candidate->posinfo.position.z;
        distanceSquared = dx * dx + dy * dy + dz * dz;
        if (distanceSquared < bestDistanceSquared)
        {
            bestDistanceSquared = distanceSquared;
            bestShip = candidate;
        }
    }

    return bestShip;
}

sdword PreFix_SalvagerSpecifics(Ship *ship)
{
    ubyte *state = salCapGetSpecificsState(ship);
    DockStaticPoint *dockPoint;

    *(sdword *)(state + 4) = SpaceObjRegistryGetID((SpaceObj *)*(Ship **)(state + 4));
    *(sdword *)(state + 8) = SpaceObjRegistryGetID((SpaceObj *)*(Ship **)(state + 8));
    *(sdword *)(state + 24) = SpaceObjRegistryGetID((SpaceObj *)*(Ship **)(state + 24));

    dockPoint = *(DockStaticPoint **)(state + 32);
    if (dockPoint != NULL)
    {
        *(sdword *)(state + 32) = dockPoint->dockindex;
        return dockPoint->dockindex;
    }

    *(sdword *)(state + 32) = -1;
    return 0;
}

sdword Fix_SalvagerSpecifics(Ship *ship)
{
    ubyte *state = salCapGetSpecificsState(ship);
    sdword dockIndex;
    Ship *target;

    *(Ship **)(state + 4) = SpaceObjRegistryGetShip(*(sdword *)(state + 4));
    *(Ship **)(state + 8) = SpaceObjRegistryGetShip(*(sdword *)(state + 8));
    *(Ship **)(state + 24) = SpaceObjRegistryGetShip(*(sdword *)(state + 24));

    target = *(Ship **)(state + 4);
    dockIndex = *(sdword *)(state + 32);
    if (dockIndex == -1)
    {
        *(udword *)(state + 32) = 0;
        return 0;
    }

    *(DockStaticPoint **)(state + 32) = &target->staticinfo->dockStaticInfo->dockstaticpoints[dockIndex];
    return dockIndex;
}

void SalCapDropTargetCallback(Ship *ship)
{
    SalCapDropTarget(ship);
}

sdword SalCapDropTargetAndRememberPreviousTarget(Ship *ship)
{
    ubyte *state = salCapGetSpecificsState(ship);
    Ship *previousTarget = *(Ship **)(state + 4);

    SalCapDropTarget(ship);
    if (previousTarget != NULL)
    {
        *(real32 *)(state + 20) = universe.totaltimeelapsed;
        *(Ship **)(state + 24) = previousTarget;
        return (sdword)TreatAsUdword(universe.totaltimeelapsed);
    }
    return 0;
}

sdword SalCapHandleProcessingShipRemoved(Ship *ship, Ship *removedShip)
{
    ubyte *state = salCapGetSpecificsState(ship);
    Ship *target = *(Ship **)(state + 4);
    Ship *processingShip;
    Ship *nearestProcessingShip;
    Ship *attachedShip;

    if (singlePlayerGame &&
        singlePlayerGameInfo.currentMission == 2 &&
        removedShip->shiptype == dMessagePod &&
        removedShip == target &&
        (removedShip->flags & SOF_Dead) != 0)
    {
        SalCapDropTarget(ship);
    }

    if (removedShip == target)
    {
        SalCapDropTarget(ship);
    }

    processingShip = *(Ship **)(state + 8);
    if (removedShip != processingShip)
    {
        return (sdword)processingShip;
    }

    if (target != NULL && target->crystalDockSlotIndex != -1)
    {
        RemoveShipFromCrystalDockSlots(processingShip, target);
        target->crystalDockSlotIndex = -1;
    }

    nearestProcessingShip = SalCapFindNearestProcessingShip(ship, target);
    for (attachedShip = SalCapIterAttachedShip(target);
         attachedShip != NULL;
         attachedShip = SalCapIterAttachedShip(NULL))
    {
        ubyte *attachedState = salCapGetSpecificsState(attachedShip);
        *(Ship **)(attachedState + 8) = nearestProcessingShip;
    }

    *(Ship **)(state + 8) = nearestProcessingShip;
    if (nearestProcessingShip != NULL)
    {
        *(sdword *)state = 9;
        return (sdword)nearestProcessingShip;
    }

    return SalCapFinalizeCapturedTarget(ship);
}

static void salcapScriptLoad(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    SalCapCorvetteStatics *corvstat;
    udword *p;

    switch (statinfo->shiprace)
    {
        case RACE_Kushan:  corvstat = &SalCapCorvetteStaticRace1; break;
        case RACE_Taiidan: corvstat = &SalCapCorvetteStaticRace2; break;
        case RACE_Traders: corvstat = &SalCapCorvetteStaticDawg;  break;
        default:           corvstat = (SalCapCorvetteStatics *)statinfo; break;
    }
    p = (udword *)corvstat;
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 0;
    p[4] = 0;
    scriptSetStruct(directory, filename, AttackSideStepParametersScriptTable, (ubyte *)corvstat);
    statinfo->custstatinfo = corvstat;
    SalCapInitHostQueueScriptData(directory, filename, statinfo);
}

static void SalCapRetailInit(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)salCapGetSpecificsState(ship);

    attackSideStepInit(&spec->attacksidestep);
    SalCapResetHostQueueScriptData(ship);
}

static bool32 SalCapRetailExecuteSpecialOrder(Ship *ship, void *custom)
{
    return SalCapExecuteSpecialOrder(ship, custom);
}

static bool32 SalCapRetailSpecialCleanup(Ship *ship, void *custom)
{
    (void)custom;
    SalCapDropTargetAndRememberPreviousTarget(ship);
    return TRUE;
}

static void SalCapRetailHouseKeep(Ship *ship)
{
    (void)SalCapProcessHostQueueScriptData(ship);
}

static void SalCapRetailRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    (void)SalCapHandleProcessingShipRemoved(ship, shipToRemove);
}

static void SalCapRetailClose(Ship *ship)
{
    SalCapDropTargetCallback(ship);
}

static void SalCapRetailPreFix(Ship *ship)
{
    (void)PreFix_SalvagerSpecifics(ship);
}

static void SalCapRetailFix(Ship *ship)
{
    (void)Fix_SalvagerSpecifics(ship);
}

void SalCapCorvette_PreFix(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    spec->target = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetID((SpaceObj *)spec->target);
    spec->dockwith = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->dockwith);
    spec->noDamageTarget = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetID((SpaceObj *)spec->noDamageTarget);
}

void SalCapCorvette_Fix(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;

    spec->target = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetTarget((sdword)spec->target);
    spec->dockwith = SpaceObjRegistryGetShip((sdword)spec->dockwith);
    spec->noDamageTarget = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetTarget((sdword)spec->noDamageTarget);
}

void CrystalResetProcessingState(Ship *crystal)
{
    ShipCrystalProcessingOverlay *state = crystalGetProcessingState(crystal);

    state->target.x = 0.0f;
    state->target.y = 0.0f;
    state->target.z = 0.0f;
    state->visualsEnabled = 1;
    state->active = 0;
    state->effect = NULL;
}

void crystalDeleteEffect(Effect *effect)
{
    if (effect != NULL)
    {
        etgEffectDelete(effect);
        univRemoveObjFromRenderList((SpaceObj *)effect);
        listDeleteNode(&effect->objlink);
    }
}

udword CrystalDestroyProcessingEffect(Ship *crystal)
{
    ShipCrystalProcessingOverlay *state;
    udword result;

    state = crystalGetProcessingState(crystal);
    result = 0;
    if (state->active == 1)
    {
        result = (udword)state->effect;
        if (result != 0)
        {
            state->visualsEnabled = 0;
            crystalDeleteEffect((Effect *)result);
            state->effect = NULL;
        }
    }
    return result;
}

#pragma warning( 2 : 4047)

CustShipHeader SalCapCorvetteHeader =
{
    sizeof(SalCapCorvetteSpec),
    salcapScriptLoad,
    NULL,
    SalCapRetailInit,
    NULL,
    NULL,
    NULL,
    NULL,
    SalCapRetailExecuteSpecialOrder,
    SalCapRetailSpecialCleanup,
    SalCapRetailHouseKeep,
    SalCapRetailRemoveShipReferences,
    SalCapRetailRemoveShipReferences,
    NULL,
    NULL,
    SalCapRetailClose,
    SalCapRetailPreFix,
    NULL,
    NULL,
    SalCapRetailFix
};

CustShipHeader JunkYardDawgHeader =
{
    sizeof(sdword),
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

void salCapExtraSpecialOrderCleanUp(SelectCommand *selection, udword ordertype, Ship *dockwith, SelectCommand *targets)
{
    sdword i;

    for (i = 0; i < selection->numShips; i++)
    {
        if (selection->ShipPtr[i]->shiptype == kSalCapCorvette)
        {
            if (((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target != NULL)
            {
                if (((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target->clampInfo != NULL &&
                    ((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target->clampInfo->host == (SpaceObjRotImpTarg *)selection->ShipPtr[i])
                {
                    switch (ordertype)
                    {
                        case COMMAND_MOVE:
                        case COMMAND_NULL:
                            break;
                        case COMMAND_SPECIAL:
                            if (ShipInSelection(targets, (Ship *)((SalCapCorvetteSpec *)selection->ShipPtr[i]->ShipSpecifics)->target))
                            {
                                break;
                            }
                            goto ditchTarget;
                        case COMMAND_DOCK:
                            if (dockwith != NULL)
                            {
                                if (dockwith->shiptype == sMothership ||
                                    dockwith->shiptype == sCarrier)
                                {
                                    break;
                                }
                            }
                            goto ditchTarget;
                        case COMMAND_ATTACK:
                        case COMMAND_LAUNCHSHIP:
                        case COMMAND_COLLECTRESOURCE:
                        case COMMAND_BUILDINGSHIP:
                        case COMMAND_HALT:
                        case COMMAND_MILITARYPARADE:
ditchTarget:
                            bitClear(selection->ShipPtr[i]->specialFlags, SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
                            SalCapOrderChangedCleanUp(selection->ShipPtr[i]);
                            break;
                    }
                }
            }
        }
    }
}

SpaceObjRotImpTargGuidanceShipDerelict *SalCapPickBestTargetFromSelection(Ship *ship, SelectAnyCommand *selection, sdword *pickStatus)
{
    sdword firstRejectedIndex = -1;
    sdword bestIndex = -1;
    real32 bestDistanceSquared = 1.0e21f;
    sdword i;

    *pickStatus = 0;

    for (i = 0; i < selection->numTargets; i++)
    {
        SpaceObjRotImpTargGuidanceShipDerelict *target = (SpaceObjRotImpTargGuidanceShipDerelict *)selection->TargetPtr[i];
        Ship *targetShip;
        sdword targetCapacity;
        sdword currentClaims;
        real32 dx;
        real32 dy;
        real32 dz;
        real32 distanceSquared;

        if (!SalCapCanTargetShipWithReasonCountsImpl(ship->playerowner, target, NULL))
        {
            continue;
        }

        targetShip = (Ship *)target;
        targetCapacity = salCapGetTargetAttachmentCapacity(targetShip);
        currentClaims = salCapGetPlayerTargetClaims(targetShip)[ship->playerowner->playerIndex];

        if (targetCapacity == currentClaims)
        {
            if (*pickStatus == 0)
            {
                firstRejectedIndex = i;
                *pickStatus = 1;
            }
            continue;
        }

        if (!SalCapTargetSupportsSalvageAttachment(target) &&
            (!singlePlayerGame || ship->playerowner->race != RACE_Beast))
        {
            if (*pickStatus == 0)
            {
                firstRejectedIndex = i;
                *pickStatus = 2;
            }
            continue;
        }

        if (currentClaims > 0 && currentClaims < targetCapacity)
        {
            bestIndex = i;
            continue;
        }

        dx = targetShip->posinfo.position.x - ship->posinfo.position.x;
        dy = targetShip->posinfo.position.y - ship->posinfo.position.y;
        dz = targetShip->posinfo.position.z - ship->posinfo.position.z;
        distanceSquared = dx * dx + dy * dy + dz * dz;
        if (distanceSquared < bestDistanceSquared)
        {
            bestDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }

    if (bestIndex == -1)
    {
        if (*pickStatus != 0 && firstRejectedIndex != -1)
        {
            return (SpaceObjRotImpTargGuidanceShipDerelict *)selection->TargetPtr[firstRejectedIndex];
        }
        return NULL;
    }

    *pickStatus = 0;
    return (SpaceObjRotImpTargGuidanceShipDerelict *)selection->TargetPtr[bestIndex];
}

bool32 SalCapReserveDockPointOnTarget(Ship *ship, Ship *target)
{
    ubyte *state = salCapGetSpecificsState(ship);
    DockStaticInfo *dockStaticInfo = target->staticinfo->dockStaticInfo;
    DockPointActivity *dockPointActivity = target->dockvars.dockPointActivity;
    sdword bestIndex = -1;
    sdword i;

    if (dockStaticInfo == NULL || dockPointActivity == NULL || dockStaticInfo->numDockPoints <= 0)
    {
        return FALSE;
    }

    for (i = 0; i < dockStaticInfo->numDockPoints; i++)
    {
        DockStaticPoint *dockStaticPoint = &dockStaticInfo->dockstaticpoints[i];

        if (dockStaticPoint->type == DPT_SalvageLatch && !dockPointActivity->reserved[i])
        {
            bestIndex = i;
            break;
        }

        if (dockStaticPoint->type == DPT_Salvage && !dockPointActivity->reserved[i])
        {
            bestIndex = i;
        }
    }

    if (bestIndex == -1)
    {
        return FALSE;
    }

    dockPointActivity->reserved[bestIndex] = 1;
    dockPointActivity->numDockedOfType[dockStaticInfo->dockstaticpoints[bestIndex].type]++;
    *(DockStaticPoint **)(state + 32) = &dockStaticInfo->dockstaticpoints[bestIndex];

    return TRUE;
}

void SalCapProcessCrystalQueueEntry(Ship *hostShip, sdword entryIndex)
{
    SalCapHostQueueEntry *entry = (SalCapHostQueueEntry *)(SalCapGetHostQueue(hostShip) + entryIndex * SALCAP_HOST_QUEUE_ENTRY_SIZE);
    DockStaticInfo *dockStaticInfo = hostShip->staticinfo->dockStaticInfo;
    vector processingTarget;
    Ship *crystal = entry->ship;
    ubyte dockPointIndex;

    matMultiplyMatByVec(
        &processingTarget,
        &hostShip->rotinfo.coordsys,
        &dockStaticInfo->dockstaticpoints[dockStaticInfo->firstPointOfType[DPT_Crystal_Effect]].position);
    processingTarget.x += hostShip->posinfo.position.x;
    processingTarget.y += hostShip->posinfo.position.y;
    processingTarget.z += hostShip->posinfo.position.z;

    switch (entry->state)
    {
        case 0:
            clampObjectSetup((SpaceObj *)crystal, (SpaceObj *)hostShip);
            CrystalStartProcessing(crystal, &processingTarget, twCrystalProcessingTime);
            crystalShipSpecifics(crystal)->processingActive = TRUE;
            entry->state = 1;
            speechEventQueue(NULL, 33558612, -1, -1, 0, hostShip->playerowner->playerIndex, -1, 5.0f, -1);
            break;

        case 1:
        {
            real32 remainingFraction = CrystalGetProcessingRemainingFraction(crystal);

            if (remainingFraction < 1.0f)
            {
                CrystalShipSpecifics *crystalSpecifics = crystalShipSpecifics(crystal);
                sdword remainingRU = (sdword)((real64)crystalStaticInfo(crystal)->resourceValue * (1.0f - remainingFraction));

                playerAddResourceUnits(hostShip->playerowner, crystalSpecifics->currentResourceValue - remainingRU);
                crystalSpecifics->currentResourceValue = remainingRU;
                CrystalUpdateProcessingTarget(crystal, &processingTarget);
            }
            else
            {
                entry->state = 2;
            }
            break;
        }

        case 2:
        {
            sdword *fsmEntry = aivarFindEntry(0, aivarFindAnyFSM("G_CrystalsHarvested"));
            CrystalShipSpecifics *crystalSpecifics = crystalShipSpecifics(crystal);

            if (fsmEntry != NULL)
            {
                fsmEntry[2]++;
            }
            universe.gameStats.playerStats[hostShip->playerowner->playerIndex].totalCrystalsRetrieved++;
            crystalSpecifics->harvested = TRUE;
            crystalSpecifics->processingActive = FALSE;

            dockPointIndex = crystal->crystalDockSlotIndex;
            if (dockPointIndex != 0xFF)
            {
                DockPointActivity *dockPointActivity = hostShip->dockvars.dockPointActivity;

                dockPointActivity->numDockedOfType[dockStaticInfo->dockstaticpoints[dockPointIndex].type]--;
                dockPointActivity->reserved[dockPointIndex] = 0;
                crystal->crystalDockSlotIndex = -1;
            }

            clDetachShipFromCommand(crystal);
            univReallyDeleteThisShipRightNow(crystal);
            memset(entry, 0, sizeof(*entry));
            speechEventQueue(NULL, 33558613, -1, -1, 0, hostShip->playerowner->playerIndex, -1, 5.0f, -1);
            break;
        }

        default:
            break;
    }
}

void SalCapProcessBigGunUnitQueueEntry(Ship *hostShip, sdword entryIndex)
{
    SalCapHostQueueEntry *entry = (SalCapHostQueueEntry *)(SalCapGetHostQueue(hostShip) + entryIndex * SALCAP_HOST_QUEUE_ENTRY_SIZE);
    sbyte *activeEntryIndex = SalCapGetHostQueueActiveIndexPtr(hostShip);
    Ship *capturedShip = entry->ship;

    switch (entry->state)
    {
        case 0:
        {
            sdword replacementShipType = CrystalGetReplacementShipType(capturedShip, capturedShip->specialTargetShip);
            ShipStaticInfo *replacementStaticInfo = GetShipStaticInfo((ShipType)replacementShipType, hostShip->shiprace);

            capturedShip->replacementStateActive = 1;
            capturedShip->replacementStateStartTime = entry->startTime;
            capturedShip->replacementStateEndTime = entry->endTime;
            capturedShip->replacementEffect = univMeshBindingsStaticDupe(replacementStaticInfo);
            capturedShip->replacementColorScheme = capturedShip->colorScheme;
            entry->state = 1;
            break;
        }
        case 1:
            if (universe.totaltimeelapsed >= entry->endTime)
            {
                entry->startTime = universe.totaltimeelapsed;
                entry->endTime = universe.totaltimeelapsed + 3.0f;
                entry->state = 2;
                *activeEntryIndex = (sbyte)entryIndex;
            }
            break;
        case 2:
            if (universe.totaltimeelapsed >= entry->endTime)
            {
                univIncrementShipTypeCount(capturedShip->shiptype, hostShip->playerowner->playerIndex);
                univReallyDeleteThisShipRightNow(capturedShip);
                *activeEntryIndex = -1;
                memset(entry, 0, sizeof(*entry));
            }
            break;
        default:
            telldir();
            break;
    }
}

void SalCapProcessMessagePodOrBuoyQueueEntry(Ship *hostShip, sdword entryIndex)
{
    SalCapHostQueueEntry *entry = (SalCapHostQueueEntry *)(SalCapGetHostQueue(hostShip) + entryIndex * SALCAP_HOST_QUEUE_ENTRY_SIZE);
    Ship *capturedShip = entry->ship;
    DockStaticInfo *dockStaticInfo = hostShip->staticinfo->dockStaticInfo;
    DockStaticPoint *dockStaticPoint = &dockStaticInfo->dockstaticpoints[capturedShip->crystalDockSlotIndex];

    switch (entry->state)
    {
        case 0:
            capturedShip->flags |= SOF_AvoidanceHidden;
            capturedShip->specialFlags2 |= 0x40u;
            capturedShip->shipidle = FALSE;
            capturedShip->autostabilizeship = FALSE;
            capturedShip->dockvars.dockPointIndex = capturedShip->crystalDockSlotIndex;
            entry->state = 1;
            break;

        case 1:
            capturedShip->shipidle = FALSE;
            capturedShip->autostabilizeship = FALSE;
            if (dockFlyToConeInside(capturedShip, hostShip, dockStaticPoint))
            {
                entry->state = 2;
            }
            break;

        case 2:
            capturedShip->shipidle = FALSE;
            capturedShip->autostabilizeship = FALSE;
            if (dockFlyToClampedConePosition(capturedShip, hostShip, dockStaticPoint))
            {
                matMultiplyMatByVec(&capturedShip->lastDockApproachHeading, &hostShip->rotinfo.coordsys, &dockStaticPoint->conenormal);
                entry->state = 3;
            }
            break;

        case 3:
            capturedShip->shipidle = FALSE;
            capturedShip->autostabilizeship = FALSE;
            if (dockFlyToFinalDockPoint(capturedShip, hostShip, 4.0f))
            {
                entry->state = 4;
            }
            break;

        case 4:
            if (capturedShip->crystalDockSlotIndex != -1)
            {
                DockPointActivity *dockPointActivity = hostShip->dockvars.dockPointActivity;
                sdword dockIndex = capturedShip->crystalDockSlotIndex;

                dockPointActivity->numDockedOfType[dockStaticInfo->dockstaticpoints[dockIndex].type]--;
                dockPointActivity->reserved[dockIndex] = 0;
                capturedShip->crystalDockSlotIndex = -1;
            }
            univIncrementShipTypeCount(capturedShip->shiptype, hostShip->playerowner->playerIndex);
            univReallyDeleteThisShipRightNow(capturedShip);
            memset(entry, 0, sizeof(*entry));
            break;

        default:
            break;
    }
}

bool32 SalCapIsAliveWorkerType(Ship *ship)
{
    return ship->flags == 0 && ((ship->shiptype == sWorker) || (ship->shiptype == bWorker));
}

void RemoveShipFromBeingSalCapTargetedByPlayer(SpaceObjRotImpTargGuidanceShipDerelict *target, uword playerIndex)
{
    (void)playerIndex;
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer, (Ship *)target, REMOVE_SALCAPTARGETING, playerIndex);
}

Effect *SalCapStartSalvageEffect(Ship *ship)
{
    SalCapTrackedTargetState *trackedTargetState = SalCapGetTrackedTargetRuntimeState(ship);
    etglod *effectTable;
    etgeffectstatic *stat;
    Effect *effect;

    trackedTargetState->unk_0C = 1;

    effectTable = etgTractorBeamEffectTable[ship->shiprace];
    if (effectTable == NULL || (sdword)ship->shiprace >= effectTable->nLevels)
    {
        ship->rceffect = NULL;
        return NULL;
    }

    stat = effectTable->level[ship->shiprace];
    if (stat == NULL || etgFrequencyExceeded(stat))
    {
        ship->rceffect = NULL;
        return NULL;
    }

    effect = etgEffectCreate(stat, ship, NULL, NULL, NULL, (real32)ship->shiprace, 88, 0);
    ship->rceffect = effect;
    if (effect != NULL)
    {
        effect->posinfo.velocity.x = 0.0f;
        effect->posinfo.velocity.y = 0.0f;
        effect->posinfo.velocity.z = 0.0f;
        trackedTargetState->unk_0C = 1;
        univUpdateObjRotInfo((SpaceObjRot *)effect);
        ship->soundevent.specialHandle = soundEventPlay(ship, 2070, NULL);
    }
    return effect;
}

Ship *SalCapIterAttachedShip(Ship *target)
{
    if (target != NULL)
    {
        return SalCapGetFirstAttachedShip(target);
    }
    return SalCapGetNextAttachedShip();
}

void SalCapQueueCapturedTargetForHostProcessing(Ship *salvager, Ship *hostShip)
{
    udword *trackedTargetState = SalCapGetTrackedTargetState(salvager);
    SalCapHostQueueEntry *hostQueue = (SalCapHostQueueEntry *)SalCapGetHostQueue(hostShip);
    Ship *capturedShip = (Ship *)trackedTargetState[1];
    real32 queueDuration = capturedShip->staticinfo->maxhealth / twSalTransSpeed;
    sdword entryIndex;
    Node *head;

    if (queueDuration >= twSalTransMaxTime)
    {
        queueDuration = twSalTransMaxTime;
    }

    for (entryIndex = 0; entryIndex < 16; entryIndex++)
    {
        if (hostQueue[entryIndex].ship == NULL)
        {
            break;
        }
    }
    if (entryIndex >= 16)
    {
        return;
    }

    hostQueue[entryIndex].ship = capturedShip;
    hostQueue[entryIndex].startTime = universe.totaltimeelapsed;
    hostQueue[entryIndex].endTime = universe.totaltimeelapsed + queueDuration;
    hostQueue[entryIndex].cleanupPending = 1;
    capturedShip->specialTargetShip = hostShip;

    for (head = universe.ShipList.head; head != NULL; head = head->next)
    {
        Ship *otherShip = head->structptr;

        switch (otherShip->shiptype)
        {
            case sWorker:
            case bWorker:
            case kSalCapCorvette:
            case tSalCapCorvette:
            case p1StandardCorvette:
            {
                CommandToDo *shipAndItsCommand = getShipAndItsCommand(&universe.mainCommandLayer, otherShip);

                if (shipAndItsCommand != NULL
                 && shipAndItsCommand->ordertype.order == COMMAND_SPECIAL
                 && shipAndItsCommand->specialtargets != NULL
                 && (!SalCapIsAliveWorkerType(otherShip) || *(ubyte *)&((SalCapCorvetteSpec *)otherShip->ShipSpecifics)->salvageState == 1))
                {
                    SpecialCommand *specialTargets = shipAndItsCommand->specialtargets;
                    sdword targetIndex = 0;

                    while (targetIndex < specialTargets->numTargets)
                    {
                        if ((Ship *)specialTargets->TargetPtr[targetIndex] == capturedShip)
                        {
                            specialTargets->numTargets--;
                            specialTargets->TargetPtr[targetIndex] = specialTargets->TargetPtr[specialTargets->numTargets];
                        }
                        else
                        {
                            targetIndex++;
                        }
                    }
                }
                break;
            }
            case 0x66:
                telldir();
                break;
            default:
                break;
        }
    }

    for (capturedShip = SalCapIterAttachedShip(capturedShip);
         capturedShip != NULL;
         capturedShip = SalCapIterAttachedShip(NULL))
    {
        clSetAttribute((sdword)capturedShip, 1, 2);
    }
}

sdword SalCapFlyCapturedShipToHost(Ship *ship, Ship *hostShip)
{
    real32 maxVelocity = (real32)(shipGetMaxSpeed(ship) * tacticsGetShipsMaxVelocity(ship));

    if (MoveReachedDestinationVariable(ship, &hostShip->posinfo.position, 6000.0f))
    {
        return TRUE;
    }

    aishipFlyToPointAvoidingObjsFunc(ship, &hostShip->posinfo.position, 0, maxVelocity, NULL);
    SalCapUpdateTowedTargetMotion(ship);
    return FALSE;
}

sdword SalCapFlyCapturedShipToHostDockPoint(Ship *ship, Ship *hostShip)
{
    DockStaticInfo *dockStaticInfo = hostShip->staticinfo->dockStaticInfo;
    DockPointActivity *dockPointActivity = hostShip->dockvars.dockPointActivity;
    real32 maxVelocity = (real32)(shipGetMaxSpeed(ship) * tacticsGetShipsMaxVelocity(ship));
    real32 bounds = 6000.0f;
    real32 dockBounds = 50.0f;

    if (!MoveReachedDestinationVariable(ship, &hostShip->posinfo.position, bounds))
    {
        aishipFlyToPointAvoidingObjsFunc(ship, &hostShip->posinfo.position, 0, maxVelocity, NULL);
        SalCapUpdateTowedTargetMotion(ship);
        return FALSE;
    }

    if (ship->crystalDockSlotIndex == -1)
    {
        sdword occupancyType;
        sdword dockPointType;
        sbyte dockPointIndex = 0;

        switch (ship->shiptype)
        {
            case dBigGunUnit:
                /* Retail (0x4fa4c9) checks hostShip->shiptype == sMothership,
                   not kCarrier -- same offset (0x2B0/688) verified via raw
                   disassembly as the Med_In_Engineering check below. */
                occupancyType = (hostShip->shiptype == sMothership) ? DPT_Destroyer_EB : DPT_SalvageProcess;
                break;

            case dMessagePod:
            case dBuoy:
                occupancyType = DPT_Med_In;
                break;

            case dCrystal0:
            case dCrystal1:
                occupancyType = DPT_Crystal_Process;
                break;

            default:
                occupancyType = -1;
                break;
        }

        dockPointType = occupancyType;
        /* Retail (0x4fa4d9/0x4fa4ec) checks hostShip->shiptype == sMothership
           (offset 0x2B0), not kCarrier -- confirmed via raw disassembly. */
        if (hostShip->shiptype == sMothership &&
            !univPlayerHasShipType(sMothershipDockingBay, hostShip->playerowner) &&
            dockPointType == DPT_Med_In)
        {
            dockPointType = DPT_Med_In_Engineering;
        }

        if (dockStaticInfo->numDockPoints > 0)
        {
            while (dockPointIndex < dockStaticInfo->numDockPoints &&
                   (dockStaticInfo->dockstaticpoints[dockPointIndex].type != dockPointType ||
                    dockPointActivity->reserved[dockPointIndex] != 0))
            {
                dockPointIndex++;
            }

            if (dockPointIndex != -1 && dockPointIndex < dockStaticInfo->numDockPoints)
            {
                dockPointActivity->numDockedOfType[occupancyType]++;
                dockPointActivity->reserved[dockPointIndex] = 1;
                ship->crystalDockSlotIndex = dockPointIndex;
            }
        }
    }

    if (ship->crystalDockSlotIndex == -1)
    {
        aitrackStabilizeGuidance((SpaceObjRotImpTargGuidance *)ship);
        aitrackZeroVelocity(ship);
        return FALSE;
    }
    else
    {
        DockStaticPoint *dockPoint = &dockStaticInfo->dockstaticpoints[ship->crystalDockSlotIndex];
        vector destination = dockPoint->position;
        vector hostToShip;
        vector hostToDestination;

        destination.x += dockPoint->mindist * dockPoint->conenormal.x;
        destination.y += dockPoint->mindist * dockPoint->conenormal.y;
        destination.z += dockPoint->mindist * dockPoint->conenormal.z;
        matMultiplyMatByVec(&destination, &hostShip->rotinfo.coordsys, &destination);
        destination.x += hostShip->posinfo.position.x;
        destination.y += hostShip->posinfo.position.y;
        destination.z += hostShip->posinfo.position.z;

        if (ship->shiptype >= dCrystal0 && ship->shiptype <= dCrystal1)
        {
            dockBounds = 150.0f;
        }

        if (MoveReachedDestinationVariable(ship, &destination, dockBounds))
        {
            return TRUE;
        }
        else
        {
            if (ship->salCapDockApproachHost != hostShip)
            {
                vecSub(hostToShip, hostShip->posinfo.position, ship->posinfo.position);
                vecSub(hostToDestination, hostShip->posinfo.position, destination);
                if (vecDotProduct(hostToShip, hostToDestination) > 0.0f)
                {
                    ship->salCapDockApproachHost = hostShip;
                }
            }
            aishipFlyToPointAvoidingObjsFunc(ship, &destination, 2, 0.0f, NULL);
            return FALSE;
        }
    }
}

sdword SalCapFinalizeCapturedTarget(Ship *ship)
{
    SalCapTrackedTargetState *trackedTargetState = SalCapGetTrackedTargetRuntimeState(ship);
    Ship *capturedShip = (Ship *)trackedTargetState->trackedTarget;
    Node *node;

    if (universe.curPlayerPtr == ship->playerowner)
    {
        if (salCapIsCrystal(capturedShip))
        {
            if (!salCapPlayerHasUpgrade(ship->playerowner, 0x100000, (ShipType)-1))
            {
                clCommandMessage(strGetString(strWorkerNeedsCrystalTech), -16776961);
            }
        }
        else if (ship->shiprace == RACE_Beast)
        {
            if (!salCapPlayerHasBoardingUpgrade(ship->playerowner))
            {
                clCommandMessage(strGetString(strDepotNeedsInfectionTech), -16776961);
            }
        }
        else
        {
            telldir();
        }
    }

    if (capturedShip->clampInfo != NULL && capturedShip->clampInfo->selection != NULL)
    {
        SelectCommand *selection = (SelectCommand *)capturedShip->clampInfo->selection;
        sdword selectionIndex;

        for (selectionIndex = selection->numShips - 1; selectionIndex >= 0; selectionIndex--)
        {
            Ship *attachedShip = selection->ShipPtr[selectionIndex];
            CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer, attachedShip);

            if (command == NULL || command->ordertype.order != COMMAND_SPECIAL)
            {
                continue;
            }

            switch (attachedShip->shiptype)
            {
                case sWorker:
                case bWorker:
                case kSalCapCorvette:
                case tSalCapCorvette:
                case p1StandardCorvette:
                    SalCapDropTarget(attachedShip);
                    break;

                case trJunkYardDawg:
                    telldir();
                    break;

                default:
                    break;
            }
        }
    }

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *otherShip = (Ship *)node->structptr;

        switch (otherShip->shiptype)
        {
            case sWorker:
            case bWorker:
            case kSalCapCorvette:
            case tSalCapCorvette:
            case p1StandardCorvette:
            {
                CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer, otherShip);

                if (command != NULL &&
                    command->ordertype.order == COMMAND_SPECIAL &&
                    command->specialtargets != NULL &&
                    (!SalCapIsAliveWorkerType(otherShip) || *(ubyte *)salCapGetSpecificsState(otherShip) == 1))
                {
                    SpecialCommand *specialTargets = command->specialtargets;
                    sdword targetIndex = 0;

                    while (targetIndex < specialTargets->numTargets)
                    {
                        if ((Ship *)specialTargets->TargetPtr[targetIndex] == capturedShip)
                        {
                            specialTargets->numTargets--;
                            specialTargets->TargetPtr[targetIndex] =
                                specialTargets->TargetPtr[specialTargets->numTargets];
                        }
                        else
                        {
                            targetIndex++;
                        }
                    }
                }
                break;
            }

            case trJunkYardDawg:
                telldir();
                break;

            default:
                break;
        }
    }

    return 0;
}

static bool32 SalCapIsTrackedTargetShipType(ShipType shiptype)
{
    switch (shiptype)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
        case p1StandardCorvette:
            return TRUE;

        case trJunkYardDawg:
            telldir();
            return FALSE;

        default:
            return FALSE;
    }
}

static void SalCapReleaseTrackedTargetRowShip(Ship *target)
{
    Ship *rowGetOutOfWay;

    if (target == NULL)
    {
        return;
    }

    rowGetOutOfWay = target->rowGetOutOfWay;
    if (rowGetOutOfWay == NULL)
    {
        return;
    }

    switch (rowGetOutOfWay->shiptype)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
        case p1StandardCorvette:
            rowRemoveShipFromGettingOutOfWay(target);
            break;

        case trJunkYardDawg:
            telldir();
            break;

        default:
            break;
    }
}

static void SalCapDampenTrackedTargetMotion(Ship *target)
{
    if (target == NULL)
    {
        return;
    }

    target->posinfo.velocity.x *= SALCAP_SPECIAL_TARGET_DRAG_SCALE;
    target->posinfo.velocity.y *= SALCAP_SPECIAL_TARGET_DRAG_SCALE;
    target->posinfo.velocity.z *= SALCAP_SPECIAL_TARGET_DRAG_SCALE;
    target->collInfo.collOffset.x *= SALCAP_SPECIAL_TARGET_DRAG_SCALE;
    target->collInfo.collOffset.y *= SALCAP_SPECIAL_TARGET_DRAG_SCALE;
    target->collInfo.collOffset.z *= SALCAP_SPECIAL_TARGET_DRAG_SCALE;
}

static Ship *SalCapFindFirstShipOfType(ShipType shiptype)
{
    Node *node;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *candidate = (Ship *)node->structptr;
        if (candidate->shiptype == shiptype)
        {
            return candidate;
        }
    }

    return NULL;
}

static bool32 SalCapIsAliveWorkerCarrierType(Ship *ship)
{
    return ship->objtype == OBJ_ShipType &&
           (ship->shiptype == sWorker || ship->shiptype == bWorker);
}

static sdword SalCapCountAttachedPlayerSalvagers(Ship *target, Player *player)
{
    /* Retail SalCapExecuteSpecialOrder state SALCAP_SPECIAL_WAIT_FOR_SUPPORT (IDA 0x4faad0,
       body at 0x4fb233-0x4fb2ad) counts attached salvagers by walking
       target->clampInfo->selection -- the SelectCommand of ships physically clamped to the
       target -- not by re-scanning the whole universe ship list via SalCapIterAttachedShip.
       Each entry is only counted if it is a real Ship (0x4fb267 tests *(ship+0x10), which is
       objtype, for OBJ_ShipType/0; our previous reconstruction instead tested flags2 at
       +0x18, an unrelated field that is never 0 for a real attached ship, so the count never
       reached capacity and the "not enough support units" message looped forever even once
       every dock point was legitimately filled), owned by the querying player (0x4fb267,
       playerowner at +0x32C), and one of the tracked salvager ship types
       (0x4fb27f switch, matches SalCapIsTrackedTargetShipType). */
    ClampInfo *clampInfo = target->clampInfo;
    SelectCommand *selection;
    sdword count = 0;
    sdword i;

    if (clampInfo == NULL)
    {
        return 0;
    }

    selection = (SelectCommand *)clampInfo->selection;
    if (selection == NULL)
    {
        return 0;
    }

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *attachedShip = selection->ShipPtr[i];

        if (attachedShip->objtype == OBJ_ShipType &&
            attachedShip->playerowner == player &&
            SalCapIsTrackedTargetShipType(attachedShip->shiptype))
        {
            count++;
        }
    }

    return count;
}

static void SalCapSetAttachedProcessingShip(Ship *target, Ship *processingShip)
{
    Ship *attachedShip;

    for (attachedShip = SalCapIterAttachedShip(target);
         attachedShip != NULL;
         attachedShip = SalCapIterAttachedShip(NULL))
    {
        SalCapTrackedTargetState *attachedState = SalCapGetTrackedTargetRuntimeState(attachedShip);
        attachedState->processingShip = processingShip;
    }
}

static void SalCapResetAttachedTrackedTargetState(Ship *target, udword state)
{
    Ship *attachedShip;

    for (attachedShip = SalCapIterAttachedShip(target);
         attachedShip != NULL;
         attachedShip = SalCapIterAttachedShip(NULL))
    {
        SalCapTrackedTargetState *attachedState = SalCapGetTrackedTargetRuntimeState(attachedShip);
        attachedState->state = state;
    }
}

static void SalCapReleaseCrystalDockReservation(Ship *capturedShip, Ship *hostShip)
{
    DockStaticInfo *dockStaticInfo;
    DockPointActivity *dockPointActivity;
    sbyte dockPointIndex;

    if (capturedShip == NULL || hostShip == NULL)
    {
        return;
    }

    dockPointIndex = capturedShip->crystalDockSlotIndex;
    if (dockPointIndex == -1)
    {
        return;
    }

    dockStaticInfo = hostShip->staticinfo->dockStaticInfo;
    dockPointActivity = hostShip->dockvars.dockPointActivity;
    if (dockStaticInfo == NULL || dockPointActivity == NULL)
    {
        capturedShip->crystalDockSlotIndex = -1;
        return;
    }

    dockPointActivity->numDockedOfType[dockStaticInfo->dockstaticpoints[dockPointIndex].type]--;
    dockPointActivity->reserved[dockPointIndex] = 0;
    capturedShip->crystalDockSlotIndex = -1;
}

bool32 SalCapExecuteSpecialOrder(Ship *ship, void *custom)
{
    SpecialCommand *specialTargets = (SpecialCommand *)custom;
    SalCapTrackedTargetState *trackedTargetState = SalCapGetTrackedTargetRuntimeState(ship);
    SalCapHostQueueScriptData *hostQueueScriptData = SalCapGetHostQueueScriptData(ship->staticinfo);
    Ship *target;
    DockStaticPoint *dockStaticPoint;
    Ship *processingShip;
    Ship *nearestProcessingShip;
    Ship *currentProcessingShip;
    SelectCommand selection;
    sdword pickStatus;
    sdword openAnimIndex;
    sdword requiredCount;
    sdword attachedCount;

    if (specialTargets == NULL || !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_SALVAGE))
    {
        return TRUE;
    }

    target = (Ship *)trackedTargetState->trackedTarget;
    if (target != NULL &&
        target->clampInfo != NULL &&
        target->clampInfo->host != NULL &&
        trackedTargetState->state >= SALCAP_SPECIAL_RESERVE_DOCK)
    {
        SalCapDropTarget(ship);
        target = (Ship *)trackedTargetState->trackedTarget;
    }

    switch (trackedTargetState->state)
    {
        case SALCAP_SPECIAL_PICK_TARGET:
            if (specialTargets->numTargets == 0 && trackedTargetState->trackedTarget == NULL)
            {
                trackedTargetState->state = SALCAP_SPECIAL_DONE;
                return FALSE;
            }

            pickStatus = 0;
            target = (Ship *)SalCapPickBestTargetFromSelection(ship, specialTargets, &pickStatus);
            if (target == NULL)
            {
                trackedTargetState->state = SALCAP_SPECIAL_DONE;
                return FALSE;
            }

            if (pickStatus == 0)
            {
                salCapGetPlayerTargetClaims(target)[ship->playerowner->playerIndex]++;
                trackedTargetState->trackedTarget = (SpaceObjRotImpTargGuidanceShipDerelict *)target;
                trackedTargetState->state = SALCAP_SPECIAL_FLY_TO_TARGET;
                if (target->shiptype == dBigGunUnit)
                {
                    ship->specialTargetShip = SalCapFindFirstShipOfType(dBigGun);
                }
                return FALSE;
            }

            if (MoveLeftToGo(ship, &target->posinfo.position, hostQueueScriptData->salvageFlyToDistance * 2.0f))
            {
                aitrackStabilizeGuidance((SpaceObjRotImpTargGuidance *)ship);
                aitrackZeroVelocity(ship);
            }
            else
            {
                aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 2, 0.0f, NULL);
            }


            if (pickStatus == 2 &&
                ship->playerowner == universe.curPlayerPtr &&
                salCapNoTargetMessageTime + twNotifySalvageTargetTooStrongRate < universeRealtimeElapsed)
            {
                clCommandMessage(strGetString(strWorkerNoSalvageableShips), -16776961);
                salCapNoTargetMessageTime = universeRealtimeElapsed;
            }
            return FALSE;

        case SALCAP_SPECIAL_FLY_TO_TARGET:
            target = (Ship *)trackedTargetState->trackedTarget;
            if (target == NULL)
            {
                return FALSE;
            }

            aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 2, 0.0f, NULL);
            /* Retail (0x4fad17) uses the per-shiptype salvageFlyToDistance as
               the MoveLeftToGo bounds here, not 0.0f -- otherwise a worker can
               never satisfy "distance to go == 0" against a large object's
               collision hull and just pushes/nudges it forever. */
            if (!MoveLeftToGo(ship, &target->posinfo.position, hostQueueScriptData->salvageFlyToDistance))
            {
                return FALSE;
            }

            if (SALCAP_TARGET_CLAIM_OWNER(target) == (udword)-1 ||
                SALCAP_TARGET_CLAIM_OWNER(target) == ship->playerowner->playerIndex)
            {
                SALCAP_TARGET_CLAIM_OWNER(target) = ship->playerowner->playerIndex;
                trackedTargetState->state = SALCAP_SPECIAL_RESERVE_DOCK;
            }
            else
            {
                aitrackStabilizeGuidance((SpaceObjRotImpTargGuidance *)ship);
                aitrackZeroVelocity(ship);
            }
            return FALSE;

        case SALCAP_SPECIAL_RESERVE_DOCK:
            target = (Ship *)trackedTargetState->trackedTarget;
            if (target == NULL)
            {
                return FALSE;
            }

            if (SalCapReserveDockPointOnTarget(ship, target))
            {
                trackedTargetState->state = SALCAP_SPECIAL_FLY_TO_CONE;
            }
            else
            {
                telldir();
                SalCapDropTarget(ship);
                trackedTargetState->state = SALCAP_SPECIAL_PICK_TARGET;
            }
            return FALSE;

        case SALCAP_SPECIAL_FLY_TO_CONE:
            target = (Ship *)trackedTargetState->trackedTarget;
            dockStaticPoint = trackedTargetState->dockPoint;
            if (target != NULL && dockFlyToConeInside(ship, target, dockStaticPoint))
            {
                ship->specialFlags2 |= 0x40u;
                trackedTargetState->unk_10 = 1;
                // Suppress ship-vs-target physical collision for the remainder of the clamp
                // approach; collCheckShipShipColl (0x43ade0) requires this bit alongside the
                // trackedTarget/unk_10 pair we already set above.
                ship->specialFlags1C8 |= CATA_RUNTIME_SALVAGE_CLAMP_APPROACH;
                trackedTargetState->state = SALCAP_SPECIAL_FLY_TO_CLAMPED_CONE;
            }

            SalCapReleaseTrackedTargetRowShip(target);
            SalCapDampenTrackedTargetMotion(target);
            return FALSE;

        case SALCAP_SPECIAL_FLY_TO_CLAMPED_CONE:
            target = (Ship *)trackedTargetState->trackedTarget;
            dockStaticPoint = trackedTargetState->dockPoint;
            if (target != NULL && dockFlyToClampedConePosition(ship, target, dockStaticPoint))
            {
                openAnimIndex = salCapGetWorkerOpenAnimIndex(ship->shiptype);
                if (openAnimIndex != -1 &&
                    *(udword *)(ship->runtimeAnimationOverlay.madAnimationData + 12) != (udword)openAnimIndex)
                {
                    madAnimationStart(ship, openAnimIndex);
                    madAnimationPause(ship, TRUE);
                }
                trackedTargetState->state = SALCAP_SPECIAL_FLY_TO_OFFSET_DOCK;
            }

            SalCapReleaseTrackedTargetRowShip(target);
            SalCapDampenTrackedTargetMotion(target);
            return FALSE;

        case SALCAP_SPECIAL_FLY_TO_OFFSET_DOCK:
            target = (Ship *)trackedTargetState->trackedTarget;
            dockStaticPoint = trackedTargetState->dockPoint;
            if (target != NULL &&
                dockFlyToOffsetDockPoint(ship, (real32 *)hostQueueScriptData->centerDockPoint, target, dockStaticPoint))
            {
                if (target->shiptype < bRecon ||
                    target->shiptype > bMissileCorvette ||
                    (ship->shiptype >= bRecon && ship->shiptype <= bMissileCorvette) ||
                    ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_BEAST_VACCINE))
                {
                    trackedTargetState->state = SALCAP_SPECIAL_CLAMP_TARGET;
                }
                else if (SalCapIsAliveWorkerCarrierType(ship))
                {
                    shipQueueBeastConversion(ship, target);
                    trackedTargetState->state = SALCAP_SPECIAL_DONE;
                }
                else
                {
                    telldir();
                }
            }

            SalCapReleaseTrackedTargetRowShip(target);
            SalCapDampenTrackedTargetMotion(target);
            return FALSE;

        case SALCAP_SPECIAL_CLAMP_TARGET:
            target = (Ship *)trackedTargetState->trackedTarget;
            if (target == NULL)
            {
                return FALSE;
            }

            clampObjectSetup((SpaceObj *)ship, (SpaceObj *)target);
            dockStaticPoint = trackedTargetState->dockPoint;
            if (dockStaticPoint != NULL && dockStaticPoint->type != DPT_SalvageProcess)
            {
                SalCapStartSalvageEffect(ship);
            }

            trackedTargetState->unk_10 = 0;
            ship->specialFlags1C8 &= ~CATA_RUNTIME_SALVAGE_CLAMP_APPROACH;
            selection.numShips = 1;
            /* Retail (0x4faf81/0x4fb029) caches p_flags2[1] (trackedTarget,
               i.e. the crystal/derelict being clamped) into v23 BEFORE
               clHalt() and halts *that* ship, not the worker itself --
               this stops the crystal's own drift/ambient order once
               clamped. Halting the worker here (as a prior version of this
               code did) triggers Worker's CustShipSpecialOrderCleanup slot
               (WorkerSpecialOrderCleanup -> SalCapDropTarget) via
               FreeLastOrder(COMMAND_SPECIAL), which immediately zeroes
               trackedTarget and calls clDetachShipFromCommand, undoing the
               clamp we just performed in the same frame. */
            selection.ShipPtr[0] = target;
            clHalt(&universe.mainCommandLayer, &selection);
            trackedTargetState->state = SALCAP_SPECIAL_PICK_PROCESSING_SHIP;
            /* Retail (0x4fb031) falls straight through into the
               PICK_PROCESSING_SHIP case in the same call via goto LABEL_77 --
               clHalt() above replaces this ship's order away from
               COMMAND_SPECIAL, so returning here instead of falling through
               would strand the state machine at state 8 forever, since the
               ship would never be dispatched through SalCapExecuteSpecialOrder
               again. */
            /* FALLTHROUGH */

        case SALCAP_SPECIAL_PICK_PROCESSING_SHIP:
            target = (Ship *)trackedTargetState->trackedTarget;
            if (singlePlayerGame && ship->playerowner->playerIndex == 1)
            {
                processingShip = SalCapGetProcessingShip();
                if (processingShip != NULL && SalCapCanHostProcessCapturedTarget(processingShip, target))
                {
                    nearestProcessingShip = processingShip;
                }
                else
                {
                    nearestProcessingShip = SalCapFindNearestProcessingShip(ship, target);
                }
            }
            else
            {
                nearestProcessingShip = SalCapFindNearestProcessingShip(ship, target);
            }

            trackedTargetState->processingShip = nearestProcessingShip;
            SalCapSetAttachedProcessingShip(target, nearestProcessingShip);

            if (nearestProcessingShip != NULL)
            {
                if (battleCanChatterAtThisTime(80, ship))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, 80, ship, ship->shiptype);
                }
                SALCAP_TARGET_LAST_REASSIGN_TIME_RAW(target) = TreatAsUdword(universe.totaltimeelapsed);
                trackedTargetState->state = SALCAP_SPECIAL_WAIT_FOR_SUPPORT;
                return FALSE;
            }

            SalCapFinalizeCapturedTarget(ship);
            return FALSE;

        case SALCAP_SPECIAL_WAIT_FOR_SUPPORT:
            target = (Ship *)trackedTargetState->trackedTarget;
            attachedCount = SalCapCountAttachedPlayerSalvagers(target, ship->playerowner);
            requiredCount = salCapGetTargetAttachmentCapacity(target);
            if (requiredCount <= 0)
            {
                requiredCount = -1;
            }

            if (attachedCount >= requiredCount)
            {
                target->flags &= ~SOF_Selectable;
                trackedTargetState->state = SALCAP_SPECIAL_DISABLE_AND_TOW;
                return FALSE;
            }

            target->flags |= SOF_Selectable;
            target->specialFlags2 &= ~SPECIAL_2_SalvageTargetAlreadyDriven;
            if (SALCAP_TARGET_LAST_SUPPORT_MESSAGE_RAW(target) == SALCAP_SPECIAL_SUPPORT_MESSAGE_SENTINEL)
            {
                SALCAP_TARGET_LAST_SUPPORT_MESSAGE_RAW(target) = TreatAsUdword(universeRealtimeElapsed);
                return FALSE;
            }
            if (TreatAsReal32(SALCAP_TARGET_LAST_SUPPORT_MESSAGE_RAW(target)) + twNotifySalvagersNeededRate >= universeRealtimeElapsed)
            {
                return FALSE;
            }

            if (ship->playerowner->playerIndex == universe.curPlayerPtr->playerIndex)
            {
                speechEventQueue(ship, 33556632, -1, -1, -1, -1, -1, 0.0f, -1);
                battleChatAddMessage(strGetString(strWorkerNeedsSupportUnits), -16776961, TRUE, (sdword)ship);
            }
            SALCAP_TARGET_LAST_SUPPORT_MESSAGE_RAW(target) = TreatAsUdword(universeRealtimeElapsed);
            return FALSE;

        case SALCAP_SPECIAL_DISABLE_AND_TOW:
            target = (Ship *)trackedTargetState->trackedTarget;
            attachedCount = SalCapCountAttachedPlayerSalvagers(target, ship->playerowner);
            requiredCount = salCapGetTargetAttachmentCapacity(target);
            if (requiredCount <= 0)
            {
                requiredCount = -1;
            }

            if (attachedCount < requiredCount)
            {
                trackedTargetState->state = SALCAP_SPECIAL_WAIT_FOR_SUPPORT;
                return FALSE;
            }

            if (TreatAsReal32(SALCAP_TARGET_LAST_REASSIGN_TIME_RAW(target)) + SALCAP_SPECIAL_REASSIGN_INTERVAL < universe.totaltimeelapsed)
            {
                nearestProcessingShip = SalCapFindNearestProcessingShip(ship, target);
                currentProcessingShip = trackedTargetState->processingShip;
                if (nearestProcessingShip != currentProcessingShip)
                {
                    real32 oldDistance;
                    real32 newDistance;
                    real32 distanceDelta;

                    oldDistance = GetDistance(&target->posinfo.position, &currentProcessingShip->posinfo.position);
                    newDistance = GetDistance(&target->posinfo.position, &nearestProcessingShip->posinfo.position);
                    distanceDelta = oldDistance - newDistance;
                    if (distanceDelta < 0.0f)
                    {
                        distanceDelta = -distanceDelta;
                    }

                    if (distanceDelta > SALCAP_SPECIAL_REASSIGN_DISTANCE_DELTA)
                    {
                        SalCapReleaseCrystalDockReservation(target, currentProcessingShip);
                        SalCapResetAttachedTrackedTargetState(target, SALCAP_SPECIAL_PICK_PROCESSING_SHIP);
                        SALCAP_TARGET_LAST_REASSIGN_TIME_RAW(target) = TreatAsUdword(universe.totaltimeelapsed);
                        return FALSE;
                    }
                }
                SALCAP_TARGET_LAST_REASSIGN_TIME_RAW(target) = TreatAsUdword(universe.totaltimeelapsed);
            }

            if ((target->flags & SOF_Slaveable) == 0)
            {
                uword curPlayerIndex;

                if ((target->flags & SOF_ForceVisible) == 0 &&
                    (universe.curPlayerPtr == target->playerowner || univIsObjFriendly((SpaceObj *)target, universe.curPlayerPtr)))
                {
                    curPlayerIndex = universe.curPlayerIndex;
                    if (curPlayerIndex == target->playerowner->playerIndex)
                    {
                        speechEventQueue(NULL, 33558593, -1, -1, 0, curPlayerIndex, -1, 5.0f, -1);
                    }
                }
                else
                {
                    curPlayerIndex = universe.curPlayerIndex;
                    if (curPlayerIndex == ship->playerowner->playerIndex &&
                        target->shiptype != dCrystal0 &&
                        target->shiptype != dCrystal1)
                    {
                        speechEventQueue(NULL, 33558594, -1, -1, 0, curPlayerIndex, -1, 5.0f, -1);
                    }
                }
            }

            selection.numShips = 1;
            selection.timeLastStatus = 0.0f;
            selection.ShipPtr[0] = target;
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selection);
            target->flags &= ~SOF_Selectable;
            target->flags |= SOF_Slaveable;
            target->flags |= SOF_TargetableHW1;
            SALCAP_TARGET_RUNTIME_FLAGS(target) &= ~0x100u;
            shipHasJustBeenDisabled(target);
            clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (SpaceObjRotImpTarg *)target);
            target->specialFlags2 |= SPECIAL_2_SalvageTargetAlreadyDriven;


            if ((SALCAP_TARGET_RUNTIME_FLAGS(target) & 0x08000000u) == 0)
            {
                bool32 reachedHost;


                switch (target->shiptype)
                {
                    case dMessagePod:
                    case dBuoy:
                    case dCrystal0:
                    case dCrystal1:
                        reachedHost = SalCapFlyCapturedShipToHostDockPoint(target, trackedTargetState->processingShip);
                        break;

                    default:
                        reachedHost = SalCapFlyCapturedShipToHost(target, trackedTargetState->processingShip);
                        break;
                }


                if (reachedHost)
                {
                    trackedTargetState->state = SALCAP_SPECIAL_STABILIZE_CAPTURE;
                }
                SALCAP_TARGET_RUNTIME_FLAGS(target) |= 0x08000000u;
            }
            return FALSE;

        case SALCAP_SPECIAL_STABILIZE_CAPTURE:
            target = (Ship *)trackedTargetState->trackedTarget;
            target->posinfo.velocity.x = 0.0f;
            target->posinfo.velocity.y = 0.0f;
            target->posinfo.velocity.z = 0.0f;
            target->rotinfo.rotspeed.x = 0.0f;
            target->rotinfo.rotspeed.y = 0.0f;
            target->rotinfo.rotspeed.z = 0.0f;
            trackedTargetState->state = SALCAP_SPECIAL_QUEUE_FOR_PROCESSING;
            return FALSE;

        case SALCAP_SPECIAL_QUEUE_FOR_PROCESSING:
            processingShip = trackedTargetState->processingShip;
            target = (Ship *)trackedTargetState->trackedTarget;
            if (!singlePlayerGame &&
                processingShip->command != NULL &&
                processingShip->command->ordertype.order == COMMAND_MP_HYPERSPACING &&
                target != NULL &&
                salCapIsCrystal(target))
            {
                SalCapDropTarget(ship);
                trackedTargetState->state = SALCAP_SPECIAL_DONE;
                return FALSE;
            }

            SalCapQueueCapturedTargetForHostProcessing(ship, processingShip);
            if (battleCanChatterAtThisTime(50, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, 50, ship, ship->shiptype);
            }
            trackedTargetState->state = SALCAP_SPECIAL_HOST_PROCESSING;
            return FALSE;

        case SALCAP_SPECIAL_HOST_PROCESSING:
            return FALSE;

        case SALCAP_SPECIAL_DONE:
            SalCapDropTarget(ship);
            return TRUE;

        default:
            return TRUE;
    }
}

static bool32 SalCapCanTargetShipWithReasonCountsImpl(Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target, uword *reasonCounts)
{
    uword localReasonCounts[9] = { 0 };
    Ship *ship;
    ShipType shipType;

    if (target->objtype != OBJ_ShipType)
    {
        if (target->objtype == OBJ_AsteroidType)
        {
            localReasonCounts[6]++;
        }
        else if (target->objtype == OBJ_DustType)
        {
            localReasonCounts[7]++;
        }
        else
        {
            localReasonCounts[0]++;
        }
        goto done;
    }

    ship = (Ship *)target;
    shipType = ship->shiptype;

    if ((ship->specialFlags & 0x40000) == 0 &&
        (player == ship->playerowner || allianceIsShipAlly(ship, player)))
    {
        localReasonCounts[3]++;
    }

    if ((ship->flags & SOF_Cloaked) != 0)
    {
        localReasonCounts[8]++;
    }

    if (salCapGetTargetAttachmentCapacity(ship) <= 0)
    {
        localReasonCounts[0]++;
    }

    if ((ship->attributes & ATTRIBUTES_KillerCollDamage) != 0 && tutorial != 1)
    {
        localReasonCounts[0]++;
    }

    if (!cmCanSupportShipType(shipType, player))
    {
        localReasonCounts[2]++;
    }

    if (salCapIsCrystal(ship))
    {
        /* Retail (0x4f7fa7) tests tpGameCreated_flag & 0x20, which is
           MG_HarvestinEnabled, not MG_CaptureCapitalShip (0x4). */
        if (singlePlayerGame || bitTest(tpGameCreated.flag, MG_HarvestinEnabled))
        {
            if (!salCapPlayerHasUpgrade(player, 0x100000, (ShipType)-1))
            {
                localReasonCounts[4]++;
            }
        }
        else
        {
            localReasonCounts[8]++;
        }
    }

    if (player->race == RACE_Beast && !salCapIsCrystal(ship) && !singlePlayerGame && !salCapPlayerHasBoardingUpgrade(player))
    {
        localReasonCounts[5]++;
    }

    switch (player->race)
    {
        case RACE_Beast:
            if (!salCapIsCrystal(ship) && ship->staticinfo->beastAttackConvertTo == (ShipType)-1)
            {
                localReasonCounts[1]++;
            }
            break;

        case RACE_Sect:
        case P1:
        case RACE_Taiidan:
        case RACE_Kushan:
        case Traders:
        case RACE_Derelict:
            ShipRaceToStr(player->race);
            telldir();
            /* fall through */
            if (!singlePlayerGame &&
                shipType >= bRecon &&
                shipType <= bMissileCorvette)
            {
                sdword salvageType;
                for (salvageType = 0; salvageType < TOTAL_NUM_SHIPS; salvageType++)
                {
                    ShipStaticInfo *salvageStaticInfo = GetShipStaticInfo((ShipType)salvageType, player->race);
                    if (salvageStaticInfo->beastAttacksRequired != (udword)-1 &&
                        salvageStaticInfo->beastAttackConvertTo == shipType &&
                        (salvageType < bRecon || salvageType > bMissileCorvette))
                    {
                        break;
                    }
                }
                if (salvageType == TOTAL_NUM_SHIPS)
                {
                    localReasonCounts[1]++;
                }
            }
            break;

        default:
            telldir();
            break;
    }

    if (singlePlayerGame &&
        player->race == RACE_Beast &&
        shipType != dMessagePod &&
        shipType != dBigGunUnit &&
        shipType != dBuoy &&
        !salCapIsCrystal(ship) &&
        ship->staticinfo->beastAttackConvertTo == (ShipType)-1)
    {
        localReasonCounts[1]++;
    }

    switch (shipType)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
        case p1StandardCorvette:
            if (salCapTargetHasActiveSalvageTarget(ship))
            {
                localReasonCounts[8]++;
            }
            break;

        case trJunkYardDawg:
            telldir();
            break;

        default:
            break;
    }

    if (salCapTargetBlocksSpecialCapture(ship))
    {
        localReasonCounts[8]++;
    }

done:
    if (reasonCounts != NULL)
    {
        sdword i;
        for (i = 0; i < 9; i++)
        {
            reasonCounts[i] += localReasonCounts[i];
        }
    }

    return memcmp(localReasonCounts, salCapNullHostQueue, sizeof(localReasonCounts)) == 0;
}

bool32 SalCapCanTargetShipWithReasonCounts(Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target, uword *reasonCounts)
{
    return SalCapCanTargetShipWithReasonCountsImpl(player, target, reasonCounts);
}

bool32 SalCapCanTargetShip(Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    return SalCapCanTargetShipWithReasonCountsImpl(player, target, NULL);
}

void SalCapProcessCapturedShipQueueEntryR2(Ship *hostShip, sdword entryIndex)
{
    SalCapHostQueueEntry *entry = (SalCapHostQueueEntry *)(SalCapGetHostQueue(hostShip) + entryIndex * SALCAP_HOST_QUEUE_ENTRY_SIZE);
    Ship *capturedShip = entry->ship;

    switch (entry->state)
    {
        case 0:
        {
            sdword replacementShipType = CrystalGetReplacementShipType(capturedShip, capturedShip->specialTargetShip);
            ShipStaticInfo *replacementStaticInfo = GetShipStaticInfo((ShipType)replacementShipType, hostShip->shiprace);
            uword playerIndex = hostShip->playerowner->playerIndex;
            ubyte colorScheme = 0;

            capturedShip->replacementStateActive = 1;
            capturedShip->replacementStateStartTime = entry->startTime;
            capturedShip->replacementStateEndTime = entry->endTime;
            capturedShip->replacementEffect = univMeshBindingsStaticDupe(replacementStaticInfo);

            if (playerIndex < MAX_MULTIPLAYER_PLAYERS && capturedShip->staticinfo->teamColor[playerIndex])
            {
                colorScheme = (ubyte)playerIndex;
            }
            else
            {
                while (colorScheme < MAX_MULTIPLAYER_PLAYERS && !capturedShip->staticinfo->teamColor[colorScheme])
                {
                    colorScheme++;
                }
                if (colorScheme >= MAX_MULTIPLAYER_PLAYERS)
                {
                    colorScheme = 0;
                }
            }

            capturedShip->replacementColorScheme = colorScheme;
            entry->state = 1;
            break;
        }

        case 1:
            if (universe.totaltimeelapsed >= entry->endTime)
            {
                ubyte colorScheme = capturedShip->replacementColorScheme;
                Player *player = hostShip->playerowner;
                sdword replacementShipType = CrystalGetReplacementShipType(capturedShip, hostShip);
                Ship *replacementShip = univSpawnReplacementShip((sdword)capturedShip, (ShipType)replacementShipType, player);

                if (replacementShip != NULL)
                {
                    udword specialFlags = replacementShip->specialFlags4 | SPECIAL4_Unknown17;

                    replacementShip->colorScheme = colorScheme;
                    replacementShip->specialFlags4 = specialFlags;
                    replacementShip->specialTargetShip = NULL;
                    replacementShip->specialFlags4 = (specialFlags & ~SPECIAL4_Unknown18) |
                        ((player->playerIndex == 0) ? SPECIAL4_Unknown18 : 0);
                    univIncrementShipTypeCount(replacementShip->shiptype, player->playerIndex);
                }

                memset(entry, 0, sizeof(*entry));
            }
            break;

        default:
            telldir();
            break;
    }
}

void SalCapProcessCapturedShipQueueEntryR1(Ship *hostShip, sdword entryIndex)
{
    SalCapHostQueueEntry *entry = (SalCapHostQueueEntry *)(SalCapGetHostQueue(hostShip) + entryIndex * SALCAP_HOST_QUEUE_ENTRY_SIZE);

    switch (entry->state)
    {
        case 0:
            entry->startTime = universe.totaltimeelapsed;
            entry->endTime = universe.totaltimeelapsed + 2.0f;
            entry->state = 1;
            break;

        case 1:
            if (universe.totaltimeelapsed > entry->endTime)
            {
                shipQueueBeastConversion(entry->ship, hostShip);
                entry->state = 2;
            }
            break;

        case 2:
            break;

        default:
            telldir();
            break;
    }
}

bool32 SalCapTargetSupportsSalvageAttachment(SpaceObjRotImpTargGuidanceShipDerelict *target)
{
    real32 salvageDamageLimit;
    Ship *ship = (Ship *)target;
    ShipClass shipClass = ship->staticinfo->shipclass;

    if (singlePlayerGame && singlePlayerGameInfo.currentMission == 8 && g_GameDifficulty < 2)
    {
        return TRUE;
    }

    if (salCapIsCrystal(ship))
    {
        return TRUE;
    }

    switch (ship->shiptype)
    {
        case dMessagePod:
        case dBuoy:
        case dBigGunUnit:
            return TRUE;

        default:
            break;
    }

    if (singlePlayerGame && singlePlayerGameInfo.currentMission == 8 && g_GameDifficulty == 2)
    {
        salvageDamageLimit = 0.75f;
    }
    else
    {
        salvageDamageLimit = twSalvageDamageLimit[shipClass];
        if (salvageDamageLimit < 0.0f || salvageDamageLimit > 1.0f)
        {
            ShipClassToStr(shipClass);
            telldir();
        }
    }

    return salvageDamageLimit * ship->staticinfo->maxhealth >= ship->health;
}

sdword SalCapInitHostQueueScriptData(char *directory, char *filename, ShipStaticInfo *shipStaticInfo)
{
    SalCapHostQueueScriptData *scriptData = SalCapGetHostQueueScriptData(shipStaticInfo);

    scriptData->salvageFlyToDistance = 0.0f;
    scriptData->centerDockPoint = NULL;
    scriptSetStruct(directory, filename, SalCapHostQueueScriptTable, scriptData);
    scriptData->centerDockPoint = dockFindDockStaticPoint("Center", shipStaticInfo->dockStaticInfo);
    return (sdword)scriptData->centerDockPoint;
}

ubyte SalCapProcessHostQueueScriptData(Ship *ship)
{
    SalCapTrackedTargetState *trackedTargetState = SalCapGetTrackedTargetRuntimeState(ship);
    ubyte result;

    if (universe.totaltimeelapsed - trackedTargetState->timeoutStart > 5.0f)
    {
        trackedTargetState->timeoutStart = 0.0f;
        trackedTargetState->timeoutEnd = 0.0f;
    }
    if (ship->rceffect == NULL
     && trackedTargetState->trackedTarget != NULL
     && ship->cataBusyStateWindow.dockingship != NULL
     && ship->cataBusyStateWindow.dockingship == (Ship *)trackedTargetState->trackedTarget
     && trackedTargetState->dockPoint != NULL
     && trackedTargetState->dockPoint->type == DPT_SalvageProcess)
    {
        if (trackedTargetState->unk_0C == 0)
        {
            telldir();
        }
        SalCapStartSalvageEffect(ship);
    }

    if ((universe.univUpdateCounter & 0x1F) != 0)
    {
        CommandToDo *command = ship->command;

        if (command != NULL
         && command->ordertype.order == COMMAND_SPECIAL
         && command->specialtargets != NULL
         && command->selection->ShipPtr[0] == ship)
        {
            SpecialCommand *specialTargets = command->specialtargets;
            sdword targetIndex;

            if (trackedTargetState->trackedTarget != NULL
             && SalCapTargetSupportsSalvageAttachment(trackedTargetState->trackedTarget))
            {
                RemoveShipFromBeingSalCapTargetedByPlayer(trackedTargetState->trackedTarget, ship->playerowner->playerIndex);
            }

            for (targetIndex = 0;
                 targetIndex < specialTargets->numTargets && targetIndex < 10;
                 targetIndex++)
            {
                SpaceObjRotImpTargGuidanceShipDerelict *target =
                    (SpaceObjRotImpTargGuidanceShipDerelict *)specialTargets->TargetPtr[targetIndex];

                if (target->flags == 0
                 && SalCapCanTargetShipWithReasonCountsImpl(ship->playerowner, target, NULL)
                 && SalCapTargetSupportsSalvageAttachment(target))
                {
                    RemoveShipFromBeingSalCapTargetedByPlayer(trackedTargetState->trackedTarget, ship->playerowner->playerIndex);
                }
            }
        }
    }

    result = trackedTargetState->clearCommand;
    if (result)
    {
        SelectCommand selectcom;

        trackedTargetState->clearCommand = 0;
        selectcom.numShips = 1;
        selectcom.ShipPtr[0] = ship;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selectcom);
    }
    return result;
}

BOOL shipIsSalCapFrigate(SpaceObj *obj)
{
    ShipType v2;

    if (obj->objtype == OBJ_ShipType)
    {
        v2 = ((Ship *)obj)->shiptype;
        if (v2 == kSalCapCorvette || v2 == tSalCapCorvette)
            return TRUE;
    }
    return FALSE;
}

BOOL salCapIsCapturing(Ship *ship)
{
    ShipType v1;
    CommandToDo *v2;

    v1 = ship->shiptype;
    if (v1 == kSalCapCorvette || v1 == tSalCapCorvette)
    {
        v2 = ship->command;
        if (v2)
        {
            if ((v2->ordertype.attributes & COMMAND_IS_FORMATION) != 0 &&
                v2->formation.formationtype == SAL_TECH_UNDOCK)
                return TRUE;
        }
    }
    return FALSE;
}

BOOL salCapIsActiveOrBeaming(Ship *ship)
{
    ShipType v1;
    CommandToDo *v2;
    TypeOfFormation v3;

    v1 = ship->shiptype;
    if (v1 == kSalCapCorvette || v1 == tSalCapCorvette)
    {
        v2 = ship->command;
        if (v2)
        {
            if ((v2->ordertype.attributes & COMMAND_IS_FORMATION) != 0)
            {
                v3 = v2->formation.formationtype;
                if (v3 == SAL_TECH_UNDOCK || v3 == SAL_STRIPTECH)
                    return TRUE;
            }
        }
    }
    return FALSE;
}

static Ship *salCapSelectClosestPriorityShip(Ship *ship, sdword *list)
{
    sdword count = list[0];
    sdword minPriority;
    sdword bestIdx;
    sdword fallbackIdx;
    sdword i;
    Ship *target;
    real32 bestDistSq;
    real32 distSq;
    real32 dx, dy, dz;

    bestIdx = -1;
    fallbackIdx = -1;
    bestDistSq = (real32)1e21f;
    minPriority = 0x7FFFFFFF;

    if (count > 0)
    {
        for (i = 0; i < count; i++)
        {
            target = (Ship *)list[i + 2];
            if (minPriority >= (sdword)target->unk_600)
                minPriority = (sdword)target->unk_600;
        }
    }

    if (count <= 0)
        return NULL;

    for (i = 0; i < count; i++)
    {
        target = (Ship *)list[i + 2];
        dx = target->posinfo.position.x - ship->posinfo.position.x;
        dy = target->posinfo.position.y - ship->posinfo.position.y;
        dz = target->posinfo.position.z - ship->posinfo.position.z;
        distSq = dy * dy + dx * dx + dz * dz;
        if (distSq < bestDistSq)
        {
            if ((sdword)target->unk_600 == minPriority)
            {
                bestIdx = i;
                bestDistSq = distSq;
            }
            else if (bestIdx == -1 && fallbackIdx == -1)
            {
                fallbackIdx = i;
            }
        }
    }

    if (bestIdx != -1)
        return (Ship *)list[bestIdx + 2];
    if (fallbackIdx == -1)
        return NULL;
    return (Ship *)list[fallbackIdx + 2];
}

static Ship *salCapSelectTarget(Ship *ship, SelectAnyCommand *selection, sdword *techFlag)
{
    sdword v3;
    sdword v4;
    Ship *v5;
    ShipStaticInfo *v6;
    Ship *v7;
    Ship *i;
    real32 v9;
    real32 v10, v11, v12, v13;
    sdword v14;
    sdword v16, v17;
    real32 v18;
    sdword v19;
    real32 v20;
    sdword v21[1002];
    sdword *a2 = (sdword *)selection;

    v16 = -1;
    v17 = -1;
    v3 = 0;
    memset(v21, 0, sizeof(v21));
    *techFlag = 0;
    v20 = 1.0e21f;
    if (*a2 <= 0)
        goto LABEL_26;
    v4 = 8;
    v19 = 8;
    do
    {
        v5 = *(Ship **)((ubyte *)a2 + v4);
        v6 = v5->staticinfo;
        if (v5->objtype != OBJ_ShipType
            || !univIsObjFriendly((SpaceObj *)v5, ship->playerowner)
            || salCapShipClassCaptureThreshold[v6->shipclass] <= 0.0f)
        {
            v14 = *a2 - 1;
            *a2 = v14;
            *(sdword *)((ubyte *)a2 + v4) = a2[v14 + 2];
        }
        else
        {
            if ((v5->specialFlags2 & 8) == 0)
            {
                v7 = v5;
                v18 = v5->health / v6->maxhealth;
                for (i = GetFirstInsideShip(v5); i; i = GetNextInsideShip())
                {
                    v9 = i->health / i->staticinfo->maxhealth;
                    if (v9 < v18)
                    {
                        v18 = v9;
                        v7 = i;
                    }
                }
                if (v7->health >= v7->staticinfo->maxhealth)
                {
                    v21[v21[0]++ + 2] = (sdword)v5;
                }
                else
                {
                    v10 = v5->posinfo.position.y - ship->posinfo.position.y;
                    v11 = v5->posinfo.position.z - ship->posinfo.position.z;
                    v12 = v5->posinfo.position.x - ship->posinfo.position.x;
                    v13 = v11 * v11 + v10 * v10 + v12 * v12;
                    if (v13 < v20)
                    {
                        if (v5->unk_604)
                        {
                            if (v16 == -1 && v17 == -1)
                                v17 = v3;
                        }
                        else
                        {
                            v20 = v13;
                            v16 = v3;
                        }
                    }
                }
            }
            ++v3;
            v4 = v19 + 4;
            v19 += 4;
        }
    }
    while (v3 < *a2);
    if (v16 != -1)
        return (Ship *)a2[v16 + 2];
    if (v17 != -1)
        return (Ship *)a2[v17 + 2];
LABEL_26:
    if (v21[0] <= 0)
        return NULL;
    *techFlag = 1;
    return salCapSelectClosestPriorityShip(ship, v21);
}



static real64 dbl_72C028 = 0.17453292519943295;   /* 10 degrees in radians */
static real64 dbl_72C020 = 0.45378560551852566;   /* 26 degrees in radians */

static Effect *salCapCreateBeamEffect(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)salCapGetSpecificsState(ship);
    SpaceObjRotImpTarg *beamTarget = *(SpaceObjRotImpTarg **)&spec->noDamageTargetTime;
    Effect *result;
    ubyte *v3;
    sdword currentLOD;
    sdword v5;
    real64 v6;
    real64 v7, v8;
    vector pos;
    vector trajectory;
    matrix coordsys;

    v3 = (ubyte *)&((SalCapCorvetteStatics *)ship->staticinfo->custstatinfo)->getTechTime;
    if (salCapBeamEffectLodTable
        && (currentLOD = ship->currentLOD, currentLOD < (sdword)(*(udword *)salCapBeamEffectLodTable))
        && (v5 = (sdword)((udword *)salCapBeamEffectLodTable)[currentLOD + 1]) != 0
        && etgFrequencyExceeded((etgeffectstatic *)((udword *)salCapBeamEffectLodTable)[currentLOD + 1]) == 0)
    {
        aishipGetTrajectory(ship, beamTarget, &trajectory);
        matCreateCoordSysFromHeading(&coordsys, &trajectory);
        matMultiplyMatByVec(&pos, &ship->rotinfo.coordsys, (vector *)(v3 + 16));
        pos.x += ship->posinfo.position.x;
        pos.y += ship->posinfo.position.y;
        pos.z += ship->posinfo.position.z;
        v6 = shipDistanceToCompoundAABB(&pos.x, (Ship *)beamTarget);
        if (!salCapBeamWidthScaleInitialized)
        {
            salCapBeamWidthScaleInitialized = 1;
            v7 = tan(dbl_72C028);
            salCapBeamNearWidthScale = (real32)(v7 + v7);
            v8 = tan(dbl_72C020);
            salCapBeamFarWidthScale = (real32)(v8 + v8);
        }
        spec->noDamageTarget = 1;
        spec->beamRange = (real32)v6;
        result = etgEffectCreate((etgeffectstatic *)v5, ship, &pos, &ship->posinfo.velocity, &coordsys, 1.0f, 112, 2);
        spec->groupme = (sdword)result;
    }
    else
    {
        spec->groupme = 0;
        result = NULL;
    }
    return result;
}

static void salCapDeleteBeamEffect(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)salCapGetSpecificsState(ship);
    Effect *beamEffect = (Effect *)spec->groupme;

    if (beamEffect)
    {
        etgEffectDelete(beamEffect);
        univRemoveObjFromRenderList((SpaceObj *)beamEffect);
        listDeleteNode((Node *)beamEffect);
        spec->noDamageTarget = 0;
        spec->groupme = 0;
        spec->beamRange = 0.0f;
    }
}

static BOOL salCapIsInBeamRange(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)salCapGetSpecificsState(ship);
    Ship *beamTarget = *(Ship **)&spec->noDamageTargetTime;
    real32 range;
    vector diff;
    vector localPoint;
    real32 aabb[6];

    range = beamTarget->staticinfo->staticheader.staticCollInfo.collspheresize * 5.0f;
    if (range >= 1500.0f)
        range = 1500.0f;
    else if (800.0f > range)
        range = 800.0f;

    diff.x = ship->posinfo.position.x - beamTarget->posinfo.position.x;
    diff.y = ship->posinfo.position.y - beamTarget->posinfo.position.y;
    diff.z = ship->posinfo.position.z - beamTarget->posinfo.position.z;
    matMultiplyVecByMat(&localPoint, &diff, &beamTarget->rotinfo.coordsys);
    shipGetCompoundAABB(beamTarget, aabb);
    return aabb[0] - (range + aabb[3]) < localPoint.x
        && localPoint.x < aabb[0] + (range + aabb[3])
        && aabb[1] - (range + aabb[4]) < localPoint.y
        && aabb[1] + (range + aabb[4]) > localPoint.y
        && aabb[2] - (range + aabb[5]) < localPoint.z
        && aabb[2] + (range + aabb[5]) > localPoint.z;
}

static BOOL salCapFlyToTarget(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)salCapGetSpecificsState(ship);
    Ship *beamTarget = *(Ship **)&spec->noDamageTargetTime;
    ubyte *v3 = (ubyte *)&((SalCapCorvetteStatics *)ship->staticinfo->custstatinfo)->getTechTime;
    BOOL inRange;
    vector dir;
    BOOL v4;
    vector a;
    real64 dist;

    inRange = salCapIsInBeamRange(ship);
    dir.x = beamTarget->posinfo.position.x - ship->posinfo.position.x;
    dir.y = beamTarget->posinfo.position.y - ship->posinfo.position.y;
    dir.z = beamTarget->posinfo.position.z - ship->posinfo.position.z;
    vecNormalize(&dir);
    v4 = (ship->rotinfo.torque.z * dir.z
          + ship->rotinfo.torque.y * dir.y
          + ship->rotinfo.torque.x * dir.x)
         >= *(real32 *)(v3 + 12);
    if (inRange)
    {
        if (spec->noDamageTarget
            && (a.x = ship->posinfo.position.x - beamTarget->posinfo.position.x,
                a.y = ship->posinfo.position.y - beamTarget->posinfo.position.y,
                dist = (real64)(a.z = ship->posinfo.position.z - beamTarget->posinfo.position.z),
                dist = fmathSqrtDouble(dist * a.z + a.y * a.y + a.x * a.x),
                dist < (real64)spec->beamRange))
        {
            real32 ratio = (real32)((spec->beamRange - dist) * (1.0 / dist));
            a.x = ratio * a.x + ship->posinfo.position.x;
            a.y = ratio * a.y + ship->posinfo.position.y;
            a.z = ratio * a.z + ship->posinfo.position.z;
            aishipFlyToPointAvoidingObjsFunc(ship, &a, 0, 0.0f, 0);
        }
        else
        {
            aitrackForceGuidanceZeroRotation((SpaceObjRotImpTargGuidance *)ship);
            aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)ship,
                                          &beamTarget->posinfo.velocity);
        }
    }
    else
    {
        aishipFlyToPointAvoidingObjsFunc(ship, &beamTarget->posinfo.position, 0, 0.0f, 0);
    }
    dir.x = beamTarget->posinfo.position.x - ship->posinfo.position.x;
    dir.y = beamTarget->posinfo.position.y - ship->posinfo.position.y;
    dir.z = beamTarget->posinfo.position.z - ship->posinfo.position.z;
    vecNormalize(&dir);
    if (aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &dir, 0.99000001f, 0.99000001f, 0.0f, 0))
        aitrackZeroRotationAnywhere(ship);
    return inRange && v4;
}

/*-----------------------------------------------------------------------------
    Name        : docapture
    Description : Handles the actual capture of a ship by queuing it for
                  reconstruction in the capturing ship's build queue.
----------------------------------------------------------------------------*/
static void docapture(Ship *ship, Ship *target)
{
    if (target->shiptype == CryoTray)
    {
        return;
    }

    if (universe.curPlayerIndex == ship->playerowner->playerIndex)
    {
        speechEventFleetSpec(ship, STAT_F_EnemyShipCaptured, target->shiptype, ship->playerowner->playerIndex);
    }
    if (ship->shiptype == Mothership)
    {
        MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
        if (spec->CAP_NumInBuildCue < CATA_MOTHERSHIP_MAX_CAPTURABLE)
        {
            spec->CAPTimeToBuildShip[spec->CAP_NumInBuildCue] = max(SALVAGE_MIN_RETROFIT_TIME, SALVAGE_MAX_RETROFIT_TIME * (1.0f - target->health / target->staticinfo->maxhealth));
            spec->CAPshiptype[spec->CAP_NumInBuildCue] = target->shiptype;
            spec->CAPshiprace[spec->CAP_NumInBuildCue] = target->staticinfo->shiprace;
            spec->CAP_NumInBuildCue++;
        }
    }
    else if (ship->shiptype == Carrier)
    {
        CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
        if (spec->CAP_NumInBuildCue < CATA_CARRIER_MAX_CAPTURABLE)
        {
            spec->CAPTimeToBuildShip[spec->CAP_NumInBuildCue] = max(SALVAGE_MIN_RETROFIT_TIME, SALVAGE_MAX_RETROFIT_TIME * (1.0f - target->health / target->staticinfo->maxhealth));
            spec->CAPshiptype[spec->CAP_NumInBuildCue] = target->shiptype;
            spec->CAPshiprace[spec->CAP_NumInBuildCue] = target->staticinfo->shiprace;
            spec->CAP_NumInBuildCue++;
        }
    }
    else
    {
        dbgFatalf(DBG_Loc, "\nUnknown type of ship receiving captured ship.  NOT a carrier or mothership");
    }
}

/*-----------------------------------------------------------------------------
    Name        : salCapHarvestTarget
    Description : Processes the final capture/harvest of a salvage target.
----------------------------------------------------------------------------*/
void salCapHarvestTarget(SpaceObjRotImpTargGuidanceShipDerelict *target, Ship *dockwith)
{
    bool32 spm3AlreadySalvaged = FALSE;

    switch (target->objtype)
    {
    case OBJ_ShipType:
        if (!singlePlayerGame)
        {
            if (bitTest(tpGameCreated.flag, MG_CaptureCapitalShip))
            {
                if ((isCapitalShip((Ship *)target)) &&
                    (((Ship *)target)->playerowner->playerIndex != dockwith->playerowner->playerIndex) &&
                    (!allianceIsShipAlly((Ship *)target, dockwith->playerowner)))
                {
                    universe.CapitalShipCaptured |= (1 << ((Ship *)target)->playerowner->playerIndex);
                    universe.PlayerWhoWon |= (1 << dockwith->playerowner->playerIndex);

                    if (((Ship *)target)->playerowner->playerState == PLAYER_ALIVE)
                    {
                        univKillPlayer(((Ship *)target)->playerowner->playerIndex, PLAYERKILLED_CAPTUREDSHIP);
                        CheckPlayerWin();
                    }
                }
            }
        }
        else
        {
            AIVar *var;

            if (((Ship *)target)->shiptype == CryoTray)
            {
                if ((var = aivarFind("SalvageCryoTray")) == NULL)
                {
                    var = aivarCreate("SalvageCryoTray");
                }
                aivarValueSet(var, TRUE);
            }
            else if (((Ship *)target)->shiptype == StandardFrigate)
            {
                if ((var = aivarFind("SalvageFrigate")) == NULL)
                {
                    var = aivarCreate("SalvageFrigate");
                }
                else
                {
                    spm3AlreadySalvaged = TRUE;
                }
                aivarValueSet(var, TRUE);
            }
            else
            {
                if ((var = aivarFind("SalvageTS")) == NULL)
                {
                    var = aivarCreate("SalvageTS");
                }
                aivarValueSet(var, TRUE);
            }
        }

        if ((((Ship *)target)->shiptype == ResearchShip) ||
            (((Ship *)target)->shiptype == TargetDrone) ||
            (singlePlayerGame &&
             (((Ship *)target)->shiptype == StandardFrigate) &&
             (singlePlayerGameInfo.currentMission == 3) &&
             (!spm3AlreadySalvaged)))
        {
            dockwith->playerowner->resourceUnits = (sdword)(dockwith->playerowner->resourceUnits + ((Ship *)target)->staticinfo->buildCost * 0.7f);
            universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnitsCollected = (sdword)(universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnitsCollected + ((Ship *)target)->staticinfo->buildCost * 0.7f);
        }
        else
        {
            docapture(dockwith, (Ship *)target);
        }

        univKillOtherPlayersIfDead((Ship *)target);

        bitSet(target->specialFlags, SPECIAL_Harvested);
        univRemoveShipFromOutside((Ship *)target);
        ((Ship *)target)->howDidIDie = DEATH_I_DIDNT_DIE_I_WAS_SALAVAGED;
        ((Ship *)target)->whoKilledMe = dockwith->playerowner->playerIndex;
        univDeleteWipeInsideShipOutOfExistence((Ship *)target);
        break;
    case OBJ_DerelictType:
        dockwith->playerowner->resourceUnits += 250;
        universe.gameStats.playerStats[dockwith->playerowner->playerIndex].totalResourceUnitsCollected += 250;
        bitSet(target->specialFlags, SPECIAL_Harvested);
        AddTargetToDeleteList((SpaceObjRotImpTarg *)target, -1);
        break;
#ifdef DEBUG_SALCAP
    default:
        dbgFatalf(DBG_Loc, "\nSal/Cap: Trying to convert an unknown object type to RU's.");
        break;
#endif
    }
}

void SalCapCancelOrder(Ship *ship)
{
    SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)ship->ShipSpecifics;
    if (spec->target != NULL)
    {
        if (bitTest(spec->target->specialFlags2, SPECIAL_2_BIT6_UNAUDITED))
        {
            bitClear(spec->target->specialFlags2, SPECIAL_2_BIT6_UNAUDITED);
            if (spec->dockwith != NULL)
            {
                if (spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy)
                    spec->dockwith->dockInfo->dockpoints[spec->dockindex].thisDockBusy--;
            }
        }
    }
}
