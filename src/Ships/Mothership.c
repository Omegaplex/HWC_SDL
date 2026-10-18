/*=============================================================================
    Name    : Mothership.c
    Purpose : Core mothership ship-header hooks

    Created from the original Homeworld source and trimmed to the portions
    currently restored in Cataclysm.
=============================================================================*/

#include <math.h>
#include <string.h>
#include "types.h"
#include "battlechat.h"
#include "debug.h"
#include "spaceobj.h"
#include "Mothership.h"
#include "BigGun.h"
#include "Carrier.h"
#include "CommandWrap.h"
#include "DefaultShip.h"
#include "attack.h"
#include "aitrack.h"
#include "camera.h"
#include "Clamp.h"
#include "dock.h"
#include "etg.h"
#include "launchMgr.h"
#include "linkedlist.h"
#include "memory.h"
#include "meshanim.h"
#include "pieplate.h"
#include "randy.h"
#include "SaveGame.h"
#include "select.h"
#include "sensors.h"
#include "strings.h"
#include "StringsOnly.h"
#include "SoundEvent.h"
#include "singleplayer.h"
#include "universe.h"
#include "univupdate.h"
#include "vector.h"
#include "Gun.h"
#include "BeastMothership.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "RaceDefs.h"
#include "tweak.h"

typedef struct
{
    real32 mothershipGunRange[NUM_TACTICS_TYPES];
    real32 mothershipTooCloseRange[NUM_TACTICS_TYPES];
} MothershipAttackStatics;

typedef struct MothershipBigGunEntry
{
    ubyte unknown00_6F[0x70];
    real32 progressStartTime;
    ubyte unknown74_83[0x10];
    GunStatic *gunStatic;
} MothershipBigGunEntry;

typedef struct MothershipStaticInfoBigGunOverlay
{
    ubyte unknown00_30B[0x30C];
    real32 bigGunChargeEffectNLips;
} MothershipStaticInfoBigGunOverlay;

typedef struct MothershipBigGunProjectileOverlay
{
    ubyte unknown00_14F[0x150];
    real32 maxTravelDistance;
    sdword chainDepth;
} MothershipBigGunProjectileOverlay;

typedef struct MothershipMoveCleanupOverlay
{
    ubyte unknown00_37[0x38];
    ubyte bigGunState;
    ubyte unknown39_43[0x0B];
    struct Effect *bigGunChargeEffect;
    ubyte bigGunAnimationPending;
} MothershipMoveCleanupOverlay;

static MothershipAttackStatics mothershipAttackStatics;

static_assert(offsetof(BeastMothershipSpec, modifierType) == 0x28, "BeastMothershipSpec modifierType offset mismatch");
static_assert(offsetof(BeastMothershipSpec, infectionLevel) == 0x2C, "BeastMothershipSpec infectionLevel offset mismatch");
static_assert(offsetof(BeastMothershipSpec, bigGunSpecifics) == 0x30, "BeastMothershipSpec bigGunSpecifics offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionState) == 0xAC, "BeastMothershipSpec destructionState offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionStateTime) == 0xB0, "BeastMothershipSpec destructionStateTime offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionCleanupTime) == 0xB4, "BeastMothershipSpec destructionCleanupTime offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionDamageType) == 0xB8, "BeastMothershipSpec destructionDamageType offset mismatch");
static_assert(offsetof(MothershipBigGunEntry, progressStartTime) == 0x70, "MothershipBigGunEntry progressStartTime offset mismatch");
static_assert(offsetof(MothershipBigGunEntry, gunStatic) == 0x84, "MothershipBigGunEntry gunStatic offset mismatch");
static_assert(offsetof(MothershipStaticInfoBigGunOverlay, bigGunChargeEffectNLips) == 0x30C, "MothershipStaticInfoBigGunOverlay nLips offset mismatch");
static_assert(offsetof(MothershipMoveCleanupOverlay, bigGunState) == 0x38, "Mothership move-cleanup state offset mismatch");
static_assert(offsetof(MothershipMoveCleanupOverlay, bigGunChargeEffect) == 0x44, "Mothership move-cleanup effect offset mismatch");
static_assert(offsetof(MothershipMoveCleanupOverlay, bigGunAnimationPending) == 0x48, "Mothership move-cleanup animation offset mismatch");

static const real32 mothershipBigGunProgressOffset = 11.0f;         /* cataclysm.exe 0x72BFC8 */
static const real32 mothershipBigGunChargeSpinPeriod = 3.0f;        /* cataclysm.exe 0x72BFC0 */
static const real32 mothershipBeastDamageModifierScale = 0.15f;     /* cataclysm.exe 0x72AF78 */
static const real32 mothershipBeastResearchBonusScale = 2.0f;       /* cataclysm.exe 0x72A6C0 */
static const real32 mothershipDeathStartDelayMin = 1.0f;            /* cataclysm.exe 0x8C53B0 */
static const real32 mothershipDeathStartDelayMax = 2.0f;            /* cataclysm.exe 0x8C53B4 */
static const real32 mothershipDestructionDelayMin = 5.0f;           /* cataclysm.exe 0x8C53B8 */
static const real32 mothershipDestructionDelayMax = 8.0f;           /* cataclysm.exe 0x8C53BC */
static const real32 mothershipRandomScale = 1.0f / (real32)UDWORD_Max; /* cataclysm.exe 0x72A4E0 */
static const real32 mothershipSelfDamageTick = 20.0f;               /* cataclysm.exe 0x72A4D0 */
static const real32 mothershipHealthWarningThreshold25 = 0.25f;     /* cataclysm.exe 0x72A4B0 */
static const real32 mothershipHealthWarningThreshold20 = 0.2f;      /* cataclysm.exe 0x72A798 */
static const real32 mothershipHealthWarningThreshold15 = 0.15f;     /* cataclysm.exe 0x72A7A0 */
static const real32 mothershipHealthWarningThreshold10 = 0.1f;      /* cataclysm.exe 0x72A7A8 */
static const real32 mothershipHealthWarningThreshold05 = 0.05f;     /* cataclysm.exe 0x72A778 */
static const real32 mothershipGunRefireStep = 0.1f;                 /* cataclysm.exe 0x72A4D4 */
static const real32 mothershipBigGunJitterScale = 0.1f;             /* cataclysm.exe 0x8C53C4 */
static const real32 mothershipBigGunJitterMinDistance = 20000.0f;   /* cataclysm.exe 0x8C53CC */
static const real32 mothershipBigGunFacingDot = 0.985f;             /* cataclysm.exe 0x8C53D0 */
static const real32 mothershipBigGunAlignDuration = 20.0f;          /* cataclysm.exe duration */

static real32 mothershipLastHealthRatio[MAX_MULTIPLAYER_PLAYERS];
static bool32 mothershipLastHealthRatioInitialized = FALSE;

bool32 TimerExpired(int timer);
void telldir(void);
extern bool32 etgFireEffectsEnabled;

static BeastMothershipSpec *beastMothershipSpec(Ship *ship)
{
    return (BeastMothershipSpec *)ship->ShipSpecifics;
}

typedef struct MothershipDestructionStateOverlay
{
    ubyte state;
    ubyte pad_01_03[3];
    real32 stateTime;
    real32 cleanupTime;
    GunSoundType damageType;
} MothershipDestructionStateOverlay;

/* MothershipDestructionStateOverlay sits at ShipSpecifics+0x9C; named so the
   accessor carries no bare offset literal. */
enum { kMothershipDestructionStateOverlayOffset = 0x9C };

static MothershipDestructionStateOverlay *mothershipDestructionState(Ship *ship)
{
    return (MothershipDestructionStateOverlay *)((ubyte *)ship->ShipSpecifics + kMothershipDestructionStateOverlayOffset);
}

void crystalBeginDeath(Ship *ship, GunSoundType damageType)
{
    MothershipDestructionStateOverlay *state = mothershipDestructionState(ship);

    if (state->state != 0)
    {
        return;
    }

    state->state = 1;
    state->stateTime =
        ((real32)ranRandom(10) * ((mothershipDeathStartDelayMax - mothershipDeathStartDelayMin) * mothershipRandomScale)) +
        mothershipDeathStartDelayMin +
        universe.totaltimeelapsed;
    state->damageType = damageType;
    clSetAttribute((sdword)ship, 1, 10);
    ClearCrystalDockSlots(ship);
}

static MothershipBigGunSpecifics *mothershipBigGunSpecificsForShip(Ship *ship)
{
    if (ship == NULL)
    {
        return NULL;
    }

    if (ship->shiptype == sMothership || ship->shiptype == bMothership)
    {
        return (MothershipBigGunSpecifics *)((ubyte *)shipModuleSlotRuntime(ship) + 0x1C);
    }

    if (ship->shiptype == dBeastMothership)
    {
        return &beastMothershipSpec(ship)->bigGunSpecifics;
    }

    return NULL;
}

static MothershipMoveCleanupOverlay *mothershipMoveCleanupOverlay(Ship *ship)
{
    return (MothershipMoveCleanupOverlay *)shipModuleSlotRuntime(ship);
}

static GunSoundType mothershipBigGunExpectedSoundType(Ship *ship)
{
    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return GS_BeastWeapon;
        default:
            return GS_BigGun;
    }
}

static void mothershipInitHealthWarningCache(void)
{
    if (!mothershipLastHealthRatioInitialized)
    {
        memset(mothershipLastHealthRatio, 0xFF, sizeof(mothershipLastHealthRatio));
        mothershipLastHealthRatioInitialized = TRUE;
    }
}

static Gun *mothershipFindBigGunGun(Ship *ship)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword i;

    if (gunInfo == NULL)
    {
        return NULL;
    }

    for (i = 0; i < gunInfo->numGuns; i++)
    {
        Gun *gun = &gunInfo->guns[i];

        if (gun->gunstatic != NULL &&
            gun->gunstatic->gunsoundtype == GS_BigGun)
        {
            return gun;
        }
    }

    return NULL;
}

static void mothershipSetRandomTarget(sdword unused, char *bigGun, vector *target)
{
    (void)unused;
    (void)bigGun;
    (void)target;

    if (singlePlayerGame)
    {
        dbgFatalf(DBG_Loc, "mothershipSetRandomTarget not yet restored");
    }
}

static void GetDirectionVectorFromMat(vector *result, udword direction, matrix *coordsys)
{
    switch (direction & 3)
    {
        case 0:
            matGetVectFromMatrixCol1(*result, *coordsys);
            break;
        case 1:
            matGetVectFromMatrixCol2(*result, *coordsys);
            break;
        case 2:
        case 3:
            matGetVectFromMatrixCol3(*result, *coordsys);
            break;
    }
    if (direction & 4)
    {
        vecNegate(*result);
    }
}

static void MothershipDettachObjectFromDoor(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;
    spec->doorCargo = NULL;
}

void mothershipGetCargoPosition(Ship *ship, SpaceObjRotImpTargGuidanceShipDerelict *cargo, vector *position, matrix *coordsys, vector *heading, vector *up, vector *right)
{
    MothershipStatics *mothershipstatics = (MothershipStatics *)((ship->staticinfo))->custstatinfo;
    vector worldRelative;
    sdword headingdir, updir;

    if (madLinkInGetDoorInfo(ship, coordsys, position))
    {
        position->x += TW_R1_MOTHERSHIP_DOOR_OFFSET_MODIFIER;
        headingdir = mothershipstatics->specialDoorOrientationHeading[((Ship *)cargo)->shiprace][((Ship *)cargo)->shiptype];
        if (headingdir == -1)
            headingdir = 0;
        updir = mothershipstatics->specialDoorOrientationUp[((Ship *)cargo)->shiprace][((Ship *)cargo)->shiptype];
        if (updir == -1)
            updir = 1;
        if (cargo->objtype == OBJ_ShipType)
        {
            matMultiplyMatByVec(&worldRelative, coordsys, &mothershipstatics->specialDoorOffset[((Ship *)cargo)->shiprace][((Ship *)cargo)->shiptype]);
            GetDirectionVectorFromMat(heading, headingdir, coordsys);
            GetDirectionVectorFromMat(up, updir, coordsys);
            vecCrossProduct(*right, *heading, *up);
        }
        else
        {
            matMultiplyMatByVec(&worldRelative, coordsys, &mothershipstatics->specialDoorOffset[RACE_Sect][HeavyCruiser]);
            GetDirectionVectorFromMat(heading, 0, &ship->rotinfo.coordsys);
            GetDirectionVectorFromMat(up, 1, &ship->rotinfo.coordsys);
            vecCrossProduct(*right, *heading, *up);
        }
        vecAddTo(*position, worldRelative);
    }
    else
    {
        *position = ship->posinfo.position;
        GetDirectionVectorFromMat(heading, 0, &ship->rotinfo.coordsys);
        GetDirectionVectorFromMat(up, 1, &ship->rotinfo.coordsys);
        vecCrossProduct(*right, *heading, *up);
    }
}

void mothershipCleanDoorForHSInstant(Ship *ship)
{
    MothershipSpec *spec = (MothershipSpec *)ship->ShipSpecifics;

    if (spec->doorCargo != NULL)
    {
        MothershipDettachObjectFromDoor(ship);
        spec->doorCargo = NULL;
    }

    if (ship->madDoorStatus != MAD_STATUS_DOOR_CLOSED)
    {
        if (ship->madBindings->bPaused)
        {
            madAnimationPause(ship, !ship->madBindings->bPaused);
        }
        madAnimationStop(ship);

        ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorOpenIndexes[0];
        ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
        ship->madDoorStatus = MAD_STATUS_DOOR_CLOSED;
        ship->madAnimationFlags = 0;
        ship->nextAnim = 0;
    }
}

void MothershipStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    udword i;

    (void)directory;
    (void)filename;

    statinfo->custstatinfo = &mothershipAttackStatics;
    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        mothershipAttackStatics.mothershipGunRange[i] = statinfo->bulletRange[i];
        mothershipAttackStatics.mothershipTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    if (shipStaticCanHaveBigGun(statinfo))
    {
        dbgAssert(beastWeaponInfoInit());
    }
}

static bool32 MothershipSpecialActivate(Ship *ship)
{
    mothershipBeginBigGunTarget(ship);
    return TRUE;
}

bool32 MothershipSpecialOrderCleanup(Ship *ship, void *custom)
{
    (void)custom;

    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearCurrentTarget(ship);
    }
    return TRUE;
}

static void MothershipRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    if (ship->shiptype == sMothership)
    {
        if (shiptoremove->shiptype == sMothershipBigGun && shiptoremove->playerowner == ship->playerowner)
        {
            MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);
            runtime->bigGunState = 3;
            if (runtime->bigGunChargeEffect != NULL)
            {
                etgEffectDelete(runtime->bigGunChargeEffect);
                univRemoveObjFromRenderList((SpaceObj *)runtime->bigGunChargeEffect);
                listDeleteNode((Node *)runtime->bigGunChargeEffect);
                runtime->bigGunChargeEffect = NULL;
            }
        }
        if (lmActive && ship->playerowner == shiptoremove->playerowner && shiptoremove->staticinfo->shipclass == CLASS_Component)
            lmCompactModuleSlotShipsByType(shiptoremove->shiptype);
    }
    if (shipCanHaveBigGun(ship))
        shipBigGunInfoClearTargetPointers(ship, shiptoremove);
    RemoveShipFromCrystalDockSlots(ship, shiptoremove);
}

static void MothershipNotifyObjectDeleted(Ship *ship, sdword objectID)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);
    if (ship->shiptype == sMothership && runtime != NULL && runtime->bigGunChargeEffect == (Effect *)objectID)
        runtime->bigGunChargeEffect = NULL;
}

void MothershipInit(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoInit(ship);
    }
    if (ship->shiptype == bMothership)
    {
        real32 nextHyperspaceSpeechTime = universe.totaltimeelapsed + 1.0f;

        ship->statusWindow.phaseScale = -1.0f;
        ship->statusWindow.state = 1;
        ship->statusWindow.phaseStartTime = nextHyperspaceSpeechTime;
        ship->statusWindow.phaseEndTime = nextHyperspaceSpeechTime + SPEECH_HYPERSPACE_DELAY;
    }

    {
        MothershipBigGunSpecifics *bigGunOverlay = mothershipBigGunSpecificsForShip(ship);
        vecZeroVector(bigGunOverlay->bigGunTarget);
        vecZeroVector(bigGunOverlay->bigGunFirePosition);
        bigGunOverlay->bigGunProjectile = NULL;
        bigGunOverlay->bigGunState = 0;
        bigGunOverlay->bigGunStateStartTime = 0.0f;
        memset(bigGunOverlay->unknown44_47, 0, sizeof(bigGunOverlay->unknown44_47));
        bigGunOverlay->bigGunChargeEffect = NULL;
        bigGunOverlay->bigGunAnimationPending = 0;
        bigGunOverlay->bigGunTimerStarted = 0;
        TimerInitTimerStruct((Timer *)&bigGunOverlay->bigGunTimer, 0.0099999998f);
    }

    if (ship->shiptype == sMothership)
    {
        ShipModuleSlotIconState *iconState = shipModuleSlotIconState(ship);

        iconState[0].moduleType = sMothershipDockingBay;
        iconState[0].iconId = 1;
        iconState[1].moduleType = sMothershipWeapons;
        iconState[1].iconId = 1;
        iconState[2].moduleType = sMothershipArmour;
        iconState[2].iconId = 1;
        iconState[3].moduleType = sMothershipMicro;
        iconState[3].iconId = 1;
        iconState[4].moduleType = sMothershipSpecial;
        iconState[4].iconId = 1;
        iconState[5].moduleType = sMothershipSupport;
        iconState[5].iconId = 1;
        iconState[6].moduleType = sMothershipBigGun;
        iconState[6].iconId = 1;
    }

    if (ship->shiptype == bMothership)
    {
        BeastMothershipRuntimeState *beastRuntime = bMothershipRuntimeState(ship);
        beastRuntime->systemType = BS_None;
        beastRuntime->infectionLevel = 0;
    }
    else if (ship->shiptype == trGhostship)
    {
        madAnimationStartWithFlags(ship, 0, MAF_Loop);
    }

    InitCrystalDockSlots(ship);
}

BeastSystemType mothershipGetSystemType(Ship *ship)
{
    dbgAssert(ship != NULL);
    dbgAssert(ship->shiptype == bMothership);
    return bMothershipRuntimeState(ship)->systemType;
}

void mothershipSetSystemType(Ship *ship, BeastSystemType system)
{
    dbgAssert(ship != NULL);
    dbgAssert(ship->shiptype == bMothership);
    bMothershipRuntimeState(ship)->systemType = system;
}

real32 mothershipGetDamageModifier(Ship *ship)
{
    const BeastMothershipRuntimeState *runtime = bMothershipRuntimeStateConst(ship);

    if (runtime->infectionLevel != 0 && runtime->systemType == BS_DefensiveWeapons)
    {
        return mothershipBeastDamageModifierScale;
    }
    return 1.0f;
}

real32 mothershipGetResearchBonus(Ship *ship)
{
    const BeastMothershipRuntimeState *runtime = bMothershipRuntimeStateConst(ship);

    if (runtime->infectionLevel != 0 && runtime->systemType == BS_Research)
    {
        return mothershipBeastResearchBonusScale;
    }
    return 1.0f;
}

sdword mothershipGetInfectionLevel(Ship *ship)
{
    return bMothershipRuntimeState(ship)->infectionLevel;
}

static MothershipBigGunEntry *mothershipFindBigGunEntry(Ship *ship)
{
    ShipUpgradeSlotStateBlock *slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    GunSoundType expectedSoundType;
    sdword i;

    if (slotStateBlock == NULL)
    {
        return NULL;
    }

    expectedSoundType = mothershipBigGunExpectedSoundType(ship);
    for (i = 0; i < slotStateBlock->count; i++)
    {
        MothershipBigGunEntry *candidate = (MothershipBigGunEntry *)&slotStateBlock->entries[i];

        if (candidate->gunStatic != NULL &&
            candidate->gunStatic->gunsoundtype == expectedSoundType)
        {
            return candidate;
        }
    }
    return NULL;
}

static Ship *mothershipFindReadyBigGun(Ship *ship)
{
    SelectCommand *selection;
    sdword i;

    if (ship->shiptype != sMothership ||
        ship->clampInfo == NULL ||
        ship->clampInfo->selection == NULL)
    {
        return NULL;
    }

    selection = (SelectCommand *)ship->clampInfo->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        Ship *module = selection->ShipPtr[i];

        if (module->shiptype == sMothershipBigGun &&
            !(module->flags & SOF_Disabled))
        {
            return module;
        }
    }
    return NULL;
}

bool32 mothershipBigGunTimerExpired(Ship *ship)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);

    if (runtime->bigGunTimerStarted != 1)
    {
        return FALSE;
    }
    return TimerExpired((int)&runtime->bigGunTimer);
}

bool32 mothershipHasReadyBigGun(Ship *ship)
{
    return mothershipFindReadyBigGun(ship) != NULL;
}

real32 mothershipGetBigGunProgress(Ship *ship)
{
    MothershipBigGunEntry *entry;
    real32 result;

    entry = mothershipFindBigGunEntry(ship);
    dbgAssert(entry != NULL);
    result = (universe.totaltimeelapsed - entry->progressStartTime) /
             (entry->gunStatic->firetime - mothershipBigGunProgressOffset);
    if (result > 1.0f)
    {
        return 1.0f;
    }
    return result;
}

bool32 mothershipBigGunAlmostComplete(Ship *ship)
{
    MothershipBigGunEntry *entry = mothershipFindBigGunEntry(ship);
    dbgAssert(entry != NULL);
    /* cataclysm.exe 0x5bd440: uses universe.totaltimeelapsed (offset +0xC0 from universe base) */
    return entry->gunStatic->firetime - mothershipBigGunProgressOffset
           < universe.totaltimeelapsed - entry->progressStartTime;
}

real32 bigGunChargeFraction(Ship *ship)
{
    ShipUpgradeSlotStateBlock *slotStateBlock;
    real32 result;
    sdword i;

    result = 0.0f;
    slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    if (slotStateBlock == NULL)
    {
        return 0.0f;
    }

    for (i = 0; i < slotStateBlock->count; i++)
    {
        MothershipBigGunEntry *entry = (MothershipBigGunEntry *)&slotStateBlock->entries[i];
        real32 elapsed;

        if (entry->gunStatic == NULL ||
            entry->gunStatic->gunsoundtype != GS_BeastWeapon)
        {
            continue;
        }

        elapsed = universe.totaltimeelapsed - entry->progressStartTime;
        if (elapsed > entry->gunStatic->firetime)
        {
            result = 1.0f;
            continue;
        }

        if (entry->gunStatic->firetime <= 0.0f)
        {
            telldir();
            return 0.0f;
        }

        result = elapsed / entry->gunStatic->firetime;
        if (result > 1.0f)
        {
            return 1.0f;
        }
        return result;
    }

    return result;
}

bool32 mothershipBigGunCanFire(Ship *ship)
{
    MothershipBigGunSpecifics *runtime;

    if (!mothershipHasReadyBigGun(ship))
    {
        return FALSE;
    }

    runtime = mothershipBigGunSpecificsForShip(ship);
    return runtime->bigGunState != 0 && runtime->bigGunState != 3;
}

static Effect *mothershipCreateBigGunChargeEffect(Ship *ship)
{
    etglod *effectTable = etgSpecialPurposeEffectTable[EGT_BIG_GUN_CHARGING];
    MothershipBigGunEntry *entry;
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);
    const MothershipStaticInfoBigGunOverlay *staticOverlay =
        (const MothershipStaticInfoBigGunOverlay *)ship->staticinfo;
    etgeffectstatic *effectStat;
    vector effectPosition;
    vector zeroVelocity = { 0.0f, 0.0f, 0.0f };
    matrix effectCoordsys;

    if (effectTable == NULL || effectTable->nLevels <= 0)
    {
        return NULL;
    }

    effectStat = effectTable->level[0];
    if (effectStat == NULL || !etgFireEffectsEnabled || etgFrequencyExceeded(effectStat))
    {
        return NULL;
    }

    entry = mothershipFindBigGunEntry(ship);
    dbgAssert(entry != NULL);
    if (entry == NULL)
    {
        return NULL;
    }

    matMultiplyMatByVec(&effectPosition, &ship->rotinfo.coordsys, &entry->gunStatic->position);
    effectPosition.x += ship->posinfo.position.x;
    effectPosition.y += ship->posinfo.position.y;
    effectPosition.z += ship->posinfo.position.z;

    if (runtime->bigGunChargeEffect != NULL)
    {
        telldir();
    }

    memcpy(&effectCoordsys, &ship->rotinfo.coordsys, sizeof(effectCoordsys));
    runtime->bigGunChargeEffect = etgEffectCreate(
        effectStat,
        NULL,
        &effectPosition,
        &zeroVelocity,
        &effectCoordsys,
        staticOverlay->bigGunChargeEffectNLips,
        120,
        0);
    return runtime->bigGunChargeEffect;
}

static void mothershipJitterBigGunTarget(Ship *ship, vector *target, vector *jitteredTarget)
{
    vector shipPosition;
    real32 jitterRange;
    real32 span;

    *jitteredTarget = *target;

    shipPosition = ship->posinfo.position;
    jitterRange = GetDistance(&shipPosition, target) * mothershipBigGunJitterScale;
    if (jitterRange <= 0.0f)
    {
        return;
    }

    span = jitterRange * 2.0f;
    jitteredTarget->x += frandom(span) - jitterRange;
    jitteredTarget->y += frandom(span) - jitterRange;
    jitteredTarget->z += frandom(span) - jitterRange;
}

void mothershipProcessBigGunCommand(CommandToDo *command)
{
    MothershipBigGunSpecifics *runtime;
    Ship *ship;
    vector *target;

    dbgAssert(command != NULL);
    dbgAssert(command->selection != NULL);

    ship = NULL;
    if (command->selection->numShips > 0)
    {
        ship = command->selection->ShipPtr[0];
    }
    if (ship == NULL)
    {
        return;
    }

    target = (vector *)((ubyte *)command + COMMAND_SIEGECANNON_TARGET_OFFSET);
    if (mothershipGetBigGunProgress(ship) < 1.0f)
    {
        return;
    }

    runtime = mothershipBigGunSpecificsForShip(ship);
    mothershipJitterBigGunTarget(ship, target, &runtime->bigGunTarget);
    runtime->bigGunState = 1;
    speechEventQueue(NULL, 67111093, -1, -1, 0, ship->playerowner->playerIndex, -1, 5.0f, -1);
    if (runtime->bigGunChargeEffect != NULL)
    {
        etgEffectDelete(runtime->bigGunChargeEffect);
        univRemoveObjFromRenderList((SpaceObj *)runtime->bigGunChargeEffect);
        listDeleteNode((Node *)runtime->bigGunChargeEffect);
        runtime->bigGunChargeEffect = NULL;
    }
}

static void mothershipBigGunCallback(vector *target)
{
    Ship *selectedMothership;
    sdword i;

    if (smSensorsActive)
    {
        smSensorsCloseForGood();
    }
    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }

    selectedMothership = NULL;
    for (i = 0; i < selSelected.numShips; i++)
    {
        Ship *ship = selSelected.ShipPtr[i];

        if (ship->shiptype == sMothership)
        {
            selectedMothership = ship;
        }
    }

    if (selectedMothership != NULL)
    {
        SelectCommand selection;

        selection.numShips = 1;
        selection.ShipPtr[0] = selectedMothership;
        clWrapSiegeCannon(&universe.mainCommandLayer, &selection, target);
    }
}

bool32 mothershipCanFireBigGun(Ship *ship)
{
    MothershipBigGunSpecifics *runtime;

    if (mothershipFindReadyBigGun(ship) == NULL)
    {
        return FALSE;
    }

    runtime = mothershipBigGunSpecificsForShip(ship);
    return runtime->bigGunState != 0;
}

void *mothershipBeginBigGunTarget(Ship *ship)
{
    if (ship == NULL ||
        ship != universe.curPlayerPtr->PlayerMothership ||
        ship->shiptype != sMothership ||
        mothershipFindReadyBigGun(ship) == NULL)
    {
        return NULL;
    }

    if (mothershipGetBigGunProgress(ship) < 1.0f)
    {
        return (void *)battleChatAddMessage(strGetString(strNotifyBigGunCharging), -16776961, 1, (sdword)ship);
    }

    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }
    piePointModeBegin(PiePointSpecTypeSiegeCannon, mothershipBigGunCallback);
    if (!smSensorsActive)
    {
        smSensorsBegin(NULL, NULL);
    }
    return NULL;
}

bool32 MothershipSpecialActivateNoTarget(Ship *ship)
{
    MothershipBigGunSpecifics *runtime;

    if (ship == NULL || ship->shiptype != sMothership)
    {
        return TRUE;
    }

    runtime = mothershipBigGunSpecificsForShip(ship);
    if (runtime != NULL &&
        mothershipFindReadyBigGun(ship) != NULL &&
        runtime->bigGunState != 0)
    {
        runtime->bigGunState = 0;
    }
    return TRUE;
}

bool32 MothershipMoveOrderCleanup(Ship *ship, void *arg1, void *arg2)
{
    (void)arg1;
    (void)arg2;

    if (ship == NULL || ship->shiptype != sMothership)
    {
        return TRUE;
    }

    {
        MothershipMoveCleanupOverlay *runtime = mothershipMoveCleanupOverlay(ship);

        if (runtime->bigGunState == 3 || runtime->bigGunState == 0)
        {
            return TRUE;
        }

        if (runtime->bigGunState == 4)
        {
            runtime->bigGunAnimationPending = 2;
            if (runtime->bigGunChargeEffect != NULL)
            {
                *(real32 *)runtime->bigGunChargeEffect->variable = 1.0f;
            }
        }

        runtime->bigGunState = 3;
        if (aitrackStabilizeGuidance((SpaceObjRotImpTargGuidance *)ship))
        {
            runtime->bigGunState = 0;
        }

        if (ship->playerowner == universe.curPlayerPtr)
        {
            clCommandMessage(strGetString(strAbortingBigGunFiring), -1);
        }
    }

    return TRUE;
}

bool32 MothershipAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    (void)maxdist;

    MothershipMoveOrderCleanup(ship, NULL, NULL);
    attackSimple(ship, target);
    return TRUE;
}

bool32 MothershipCanHalt(Ship *ship)
{
    return MothershipMoveOrderCleanup(ship, NULL, NULL);
}

bool32 mothershipIsFacingBigGunTarget(Ship *ship)
{
    MothershipBigGunSpecifics *runtime;
    vector desiredHeading;
    vector currentHeading;

    runtime = mothershipBigGunSpecificsForShip(ship);
    vecSub(desiredHeading, runtime->bigGunTarget, ship->posinfo.position);
    vecNormalize(&desiredHeading);
    matGetVectFromMatrixCol3(currentHeading, ship->rotinfo.coordsys);
    vecNormalize(&currentHeading);
    return vecDotProduct(desiredHeading, currentHeading) > mothershipBigGunFacingDot;
}

bool32 mothershipAlignBigGun(SpaceObjRotImpTargGuidance *ship)
{
    Ship *shipObj;
    MothershipBigGunSpecifics *runtime;
    vector desiredHeading;
    vector currentHeading;

    shipObj = (Ship *)ship;
    runtime = mothershipBigGunSpecificsForShip(shipObj);
    vecSub(desiredHeading, runtime->bigGunTarget, ship->posinfo.position);
    vecNormalize(&desiredHeading);
    matGetVectFromMatrixCol3(currentHeading, ship->rotinfo.coordsys);
    vecNormalize(&currentHeading);
    if (vecDotProduct(desiredHeading, currentHeading) > mothershipBigGunFacingDot &&
        runtime->bigGunTimerStarted == 0)
    {
        runtime->bigGunTimerStarted = 1;
        TimerInitTimerStruct((Timer *)&runtime->bigGunTimer, mothershipBigGunAlignDuration);
    }
    return aitrackHeadingFunc(ship, &desiredHeading, 0.99900001f, 0.2f, 0.0f, 0);
}

static void mothershipBeginBigGunCharge(Ship *ship)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);

    mothershipCreateBigGunChargeEffect(ship);
    runtime->bigGunState = 4;
    runtime->bigGunAnimationPending = 1;
    runtime->bigGunStateStartTime = universe.totaltimeelapsed;
    ship->autostabilizeship = FALSE;
}

static void mothershipBigGunStartTimer(Ship *ship)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);

    if (runtime->bigGunTimerStarted == 0)
    {
        runtime->bigGunTimerStarted = 1;
        TimerInitTimerStruct((Timer *)&runtime->bigGunTimer, mothershipBigGunAlignDuration);
    }
}

static void mothershipUpdateBigGunFormation(Ship *ship)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);
    MothershipBigGunEntry *entry;
    Ship *module;
    Node *node;
    SpaceObjRotImp *chargeEffect;
    vector effectOffset;
    matrix baseCoordsys;
    matrix rotation;
    real32 angle;

    if (runtime->bigGunState == 4 &&
        universe.totaltimeelapsed - runtime->bigGunStateStartTime > mothershipBigGunProgressOffset)
    {
        speechEventQueue(NULL, 33556662, -1, -1, 0, ship->playerowner->playerIndex, -1, 5.0f, -1);
        if (ship->playerowner != universe.curPlayerPtr)
        {
            clCommandMessage(strGetString(strWarnBigGunHasFired), -16776961);
            soundEventPlay(NULL, 317, 0);
        }

        runtime->bigGunState = 2;
        module = NULL;
        if (ship->playerowner->PlayerMothership != NULL)
        {
            for (node = universe.ShipList.head; node != NULL && module == NULL; node = node->next)
            {
                Ship *candidate = (Ship *)node->structptr;

                if (candidate->shiptype == sMothershipBigGun &&
                    candidate->playerowner == ship->playerowner &&
                    (candidate->specialFlags2 & SPECIAL_2_DockFlag) == 0)
                {
                    module = candidate;
                }
            }
        }

        if (module != NULL && runtime->bigGunAnimationPending == 1)
        {
            madAnimationStart(module, 0);
        }
        runtime->bigGunAnimationPending = 0;
        runtime->bigGunChargeEffect = NULL;
    }

    if (runtime->bigGunChargeEffect == NULL)
    {
        return;
    }

    entry = mothershipFindBigGunEntry(ship);
    dbgAssert(entry != NULL);
    if (entry == NULL)
    {
        return;
    }

    chargeEffect = (SpaceObjRotImp *)runtime->bigGunChargeEffect;
    matMultiplyMatByVec(&effectOffset, &ship->rotinfo.coordsys, &entry->gunStatic->position);
    vecAdd(chargeEffect->posinfo.position, effectOffset, ship->posinfo.position);
    memcpy(&chargeEffect->rotinfo.coordsys, &ship->rotinfo.coordsys, sizeof(matrix));

    angle = (universe.totaltimeelapsed - runtime->bigGunStateStartTime) / mothershipBigGunChargeSpinPeriod;
    angle *= angle;
    matMakeRotAboutZ(&rotation, (real32)cos(angle), (real32)sin(angle));
    memcpy(&baseCoordsys, &chargeEffect->rotinfo.coordsys, sizeof(baseCoordsys));
    matMultiplyMatByMat(&chargeEffect->rotinfo.coordsys, &baseCoordsys, &rotation);
}

void mothershipBigGunUpdate(SpaceObjRotImpTargGuidance *ship)
{
    Ship *shipObj = (Ship *)ship;
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(shipObj);
    MothershipBigGunEntry *entry;
    Gun *bigGun;
    MothershipBigGunProjectileOverlay *projectile;
    vector desiredHeading;
    real32 maxTravelDistance;

    if (runtime->bigGunState == 0 || runtime->bigGunState == 3)
    {
        runtime->bigGunTimerStarted = 0;
        TimerInitTimerStruct((Timer *)&runtime->bigGunTimer, 0.01f);
    }

    switch (runtime->bigGunState)
    {
        case 1:
            vecSub(desiredHeading, runtime->bigGunTarget, shipObj->posinfo.position);
            vecNormalize(&desiredHeading);
            shipObj->autostabilizeship = FALSE;

            if (mothershipIsFacingBigGunTarget(shipObj) && runtime->bigGunTimerStarted == 0)
            {
                runtime->bigGunTimerStarted = 1;
                TimerInitTimerStruct((Timer *)&runtime->bigGunTimer, mothershipBigGunAlignDuration);
            }

            if ((runtime->bigGunTimerStarted == 1 && TimerExpired((int)&runtime->bigGunTimer)) ||
                aitrackHeadingFunc(ship, &desiredHeading, 0.99900001f, 0.2f, 0.0f, 0))
            {
                runtime->bigGunFirePosition = shipObj->posinfo.position;
                if (mothershipHasReadyBigGun(shipObj))
                {
                    mothershipBeginBigGunCharge(shipObj);
                    return;
                }
                runtime->bigGunState = 3;
            }
            break;

        case 2:
            bigGun = mothershipFindBigGunGun(shipObj);
            dbgAssert(bigGun != NULL);
            if (bigGun == NULL)
            {
                runtime->bigGunState = 3;
                break;
            }

            vecSub(desiredHeading, runtime->bigGunTarget, shipObj->posinfo.position);
            vecNormalize(&desiredHeading);
            shipObj->autostabilizeship = FALSE;

            if (mothershipIsFacingBigGunTarget(shipObj) && runtime->bigGunTimerStarted == 0)
            {
                runtime->bigGunTimerStarted = 1;
                TimerInitTimerStruct((Timer *)&runtime->bigGunTimer, mothershipBigGunAlignDuration);
            }
            if (!mothershipBigGunTimerExpired(shipObj))
            {
                aitrackHeadingFunc(ship, &desiredHeading, 0.99900001f, 0.2f, 0.0f, 0);
            }

            runtime->bigGunProjectile = (SpaceObjRotImpTarg *)gunShootGunAtTarget(shipObj, bigGun, NULL);
            singlePlayerRegisterShockwaveProjectile(runtime->bigGunProjectile);
            mothershipSetRandomTarget(0, (char *)bigGun, &runtime->bigGunTarget);

            if (runtime->bigGunProjectile != NULL)
            {
                vecSub(desiredHeading, runtime->bigGunTarget, runtime->bigGunProjectile->posinfo.position);
                maxTravelDistance = (real32)sqrt(vecMagnitudeSquared(desiredHeading));
                if (!singlePlayerGame && maxTravelDistance > mothershipBigGunJitterMinDistance)
                {
                    real32 jitterRange = mothershipBigGunJitterScale * maxTravelDistance;

                    maxTravelDistance = (((real32)ranRandom(1) * (1.0f / (real32)UDWORD_Max)) *
                                         (jitterRange - -jitterRange)) +
                                        -jitterRange +
                                        maxTravelDistance;
                }

                projectile = (MothershipBigGunProjectileOverlay *)runtime->bigGunProjectile;
                projectile->chainDepth = 0;
                projectile->maxTravelDistance = maxTravelDistance;
                runtime->bigGunState = 3;
            }
            break;

        case 5:
            vecSub(desiredHeading, runtime->bigGunTarget, shipObj->posinfo.position);
            vecNormalize(&desiredHeading);
            shipObj->autostabilizeship = FALSE;

            if (mothershipIsFacingBigGunTarget(shipObj) && runtime->bigGunTimerStarted == 0)
            {
                mothershipBigGunStartTimer(shipObj);
            }
            if (!mothershipBigGunTimerExpired(shipObj))
            {
                aitrackHeadingFunc(ship, &desiredHeading, 0.99900001f, 0.2f, 0.0f, 0);
            }

            entry = mothershipFindBigGunEntry(shipObj);
            dbgAssert(entry != NULL);
            if (entry != NULL &&
                entry->gunStatic->firetime - mothershipBigGunProgressOffset <
                    universe.totaltimeelapsed - entry->progressStartTime)
            {
                if (mothershipHasReadyBigGun(shipObj))
                {
                    mothershipBeginBigGunCharge(shipObj);
                    return;
                }
                runtime->bigGunState = 3;
            }
            break;

        case 4:
            mothershipAlignBigGun(ship);
            mothershipUpdateBigGunFormation(shipObj);
            return;

        case 3:
            mothershipUpdateBigGunFormation(shipObj);
            if (aitrackStabilizeGuidance((SpaceObjRotImpTargGuidance *)ship))
            {
                runtime->bigGunState = 0;
            }
            break;

        case 0:
            mothershipUpdateBigGunFormation(shipObj);
            return;

        default:
            telldir();
            runtime->bigGunState = 3;
            break;
    }
}

void mothershipBigGunRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);

    if ((ship->shiptype == sMothership || ship->shiptype == bMothership) &&
        runtime != NULL &&
        shiptoremove != NULL &&
        shiptoremove->shiptype == sMothershipBigGun &&
        shiptoremove->playerowner == ship->playerowner)
    {
        runtime->bigGunState = 3;
        if (runtime->bigGunChargeEffect != NULL)
        {
            etgEffectDelete(runtime->bigGunChargeEffect);
            univRemoveObjFromRenderList((SpaceObj *)runtime->bigGunChargeEffect);
            listDeleteNode((Node *)runtime->bigGunChargeEffect);
            runtime->bigGunChargeEffect = NULL;
        }
    }
}

void mothershipBigGunNotifyObjectDeleted(Ship *ship, sdword objectID)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);

    if (runtime != NULL &&
        runtime->bigGunProjectile != NULL &&
        runtime->bigGunProjectile->objtype == (ObjType)objectID)
    {
        runtime->bigGunProjectile = NULL;
    }
}

void mothershipBigGunFix(Ship *ship)
{
    MothershipBigGunSpecifics *runtime = mothershipBigGunSpecificsForShip(ship);

    if (runtime != NULL)
    {
        runtime->bigGunChargeEffect = NULL;
        runtime->bigGunProjectile = (SpaceObjRotImpTarg *)SpaceObjRegistryGetID((SpaceObj *)runtime->bigGunProjectile);
    }
}

static void mothershipProcessBeastSelfDamage(Ship *ship)
{
    BeastMothershipRuntimeState *runtime = bMothershipRuntimeState(ship);
    real32 minHealth;
    real32 nextHealth;
    GunInfo *gunInfo;
    sdword i;

    if (shipCanHaveBigGun(ship))
    {
        bigGunProcessTargetingAndFiring(ship);
    }

    if (runtime->systemType == BS_None)
    {
        runtime->infectionLevel = 0;
        return;
    }

    minHealth = ship->staticinfo->maxhealth * 0.5f;
    if (minHealth >= ship->health)
    {
        runtime->infectionLevel = 0;
        lmBeastSystemClear(ship);
        return;
    }

    runtime->infectionLevel = 1;
    nextHealth = ship->health - mothershipSelfDamageTick;
    ship->health = nextHealth;
    if (nextHealth < minHealth)
    {
        ship->health = minHealth;
    }

    if (runtime->systemType != BS_Infection)
    {
        return;
    }

    gunInfo = ship->cataclysmGunInfo;
    for (i = 0; i < gunInfo->numGuns; i++)
    {
        Gun *gun = &gunInfo->guns[i];

        if (gun->gunstatic->gunsoundtype == GS_BeastWeapon &&
            universe.totaltimeelapsed - gun->lasttimefired < gun->gunstatic->firetime)
        {
            gun->lasttimefired -= mothershipGunRefireStep;
            if (gun->lasttimefired < 0.0f)
            {
                gun->lasttimefired = 0.0f;
            }
        }
    }
}

static void mothershipWarnLowHealth(Ship *ship)
{
    real32 previousHealthRatio;
    real32 currentHealthRatio;
    bool32 crossedCriticalThreshold;
    uword playerIndex = ship->playerowner->playerIndex;

    if (playerIndex != universe.curPlayerIndex || !(ship->flags & SOF_Targetable))
    {
        return;
    }

    mothershipInitHealthWarningCache();
    previousHealthRatio = mothershipLastHealthRatio[playerIndex];
    currentHealthRatio = ship->health / ship->staticinfo->maxhealth;
    crossedCriticalThreshold =
        (previousHealthRatio > mothershipHealthWarningThreshold25 && currentHealthRatio < mothershipHealthWarningThreshold25) ||
        (previousHealthRatio > mothershipHealthWarningThreshold20 && currentHealthRatio < mothershipHealthWarningThreshold20);

    if (previousHealthRatio > mothershipHealthWarningThreshold15 && currentHealthRatio < mothershipHealthWarningThreshold15)
    {
        crossedCriticalThreshold = TRUE;
    }
    if (previousHealthRatio > mothershipHealthWarningThreshold10 && currentHealthRatio < mothershipHealthWarningThreshold10)
    {
        crossedCriticalThreshold = TRUE;
    }
    if ((previousHealthRatio > mothershipHealthWarningThreshold05 && currentHealthRatio < mothershipHealthWarningThreshold05) ||
        crossedCriticalThreshold)
    {
        battleChatAddMessage(strGetString(strMothershipHealthLow), -16776961, 1, (sdword)ship);
        speechEventQueue(NULL, 33558615, -1, -1, 0, playerIndex, -1, 5.0f, -1);
    }

    mothershipLastHealthRatio[playerIndex] = currentHealthRatio;
}

void MothershipHouseKeep(Ship *ship)
{
    MothershipDestructionStateOverlay *state = mothershipDestructionState(ship);

    if (state->state == 1)
    {
        if (universe.totaltimeelapsed > state->stateTime)
        {
            SelectCommand *selection = ship->clampInfo != NULL ? (SelectCommand *)ship->clampInfo->selection : NULL;
            sdword i;

            if (selection != NULL)
            {
                for (i = 0; i < selection->numShips; i++)
                {
                    Ship *clampedShip = selection->ShipPtr[i];

                    if (clampedShip->staticinfo->shipclass == CLASS_Component)
                    {
                        bitSet(clampedShip->flags2, SOF2_Unknown4);
                    }
                }
            }

            state->state = 2;
            state->cleanupTime =
                ((real32)ranRandom(10) * ((mothershipDestructionDelayMax - mothershipDestructionDelayMin) * mothershipRandomScale)) +
                mothershipDestructionDelayMin +
                universe.totaltimeelapsed;
        }
    }
    else if (state->state == 2)
    {
        DeleteShip *deleteShip;

        MakeExplosionRockBumpables(ship, FALSE);
        deleteShip = memAlloc(sizeof(DeleteShip), "DeleteShip", MBF_SmallBlockHeap);
        deleteShip->ship = ship;
        deleteShip->deathBy = (sdword)etgDeathModeByGunType[state->damageType];
        listAddNode(&universe.DeleteShipList, &deleteShip->objlink, deleteShip);
        ship->flags |= SOF_Dead;
    }
    else if (state->state != 3)
    {
        if (ship->shiptype == sMothership)
        {
            mothershipBigGunUpdate((SpaceObjRotImpTargGuidance *)ship);
        }
        else if (ship->shiptype == bMothership)
        {
            mothershipProcessBeastSelfDamage(ship);
        }

        ProcessCrystalDockSlots(ship);
        if ((universe.univUpdateCounter & 0x1F) != 0)
        {
            mothershipWarnLowHealth(ship);
        }
    }
}

#pragma warning( 4 : 4047)
void Mothership_PreFix(Ship *ship)
{
    PreFix_CrystalDockSlots(ship);
}

void Mothership_Save(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        SaveShipBigGunInfo(ship);
    }
}

void Mothership_Load(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        LoadShipBigGunInfo(ship);
    }
}

void Mothership_Fix(Ship *ship)
{
    MothershipBigGunSpecifics *runtime;
    if (shipCanHaveBigGun(ship))
        FixShipBigGunInfo(ship);
    if (ship->shiptype == sMothership)
    {
        runtime = mothershipBigGunSpecificsForShip(ship);
        runtime->bigGunProjectile = NULL;
        runtime->bigGunChargeEffect = NULL;
    }
    Fix_CrystalDockSlots(ship);
}
#pragma warning( 2 : 4047)

void MothershipDied(Ship *ship)
{
    sdword i;
    int *rawSelection;

    shockwaveCreateTyped(2,
        TreatAsUdword(ship->posinfo.position.x),
        TreatAsUdword(ship->posinfo.position.y),
        TreatAsUdword(ship->posinfo.position.z),
        ship->playerowner->playerIndex);

    if (shipCanHaveBigGun(ship))
        shipBigGunInfoClose(ship);
    ClearCrystalDockSlots(ship);

    if (ship->clampInfo != NULL)
    {
        rawSelection = (int *)ship->clampInfo->selection;
        if (rawSelection != NULL && rawSelection[0] > 0)
        {
            i = 0;
            while (i < rawSelection[0])
            {
                Ship *module = (Ship *)rawSelection[i + 2];
                if (bitTest(module->specialFlags2, SPECIAL_2_DockFlag))
                    module->deathtime = universe.totaltimeelapsed + 1.0f;
                rawSelection = (int *)ship->clampInfo->selection;
                i++;
            }
        }
    }
}

CustShipHeader MothershipHeader =
{
    sizeof(MothershipSpec),
    MothershipStaticInit,
    NULL,
    MothershipInit,
    NULL,
    shipShootAllGunsAtTarget,
    shipAttackSimpleNoRotate,
    MothershipSpecialActivateNoTarget,
    shipBigGunSpecialTarget,
    MothershipSpecialOrderCleanup,
    MothershipHouseKeep,
    MothershipRemoveShipReferences,
    MothershipRemoveShipReferences,
    MothershipNotifyObjectDeleted,
    shipBigGunNotifyObjectDeleted,
    MothershipDied,
    Mothership_PreFix,
    Mothership_Save,
    Mothership_Load,
    Mothership_Fix,
    MothershipMoveOrderCleanup,
    MothershipAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MothershipCanHalt
};

CustShipHeader BMothershipHeader =
{
    sizeof(MothershipSpec),
    MothershipStaticInit,
    NULL,
    MothershipInit,
    NULL,
    shipShootAllGunsAtTarget,
    shipAttackSimpleNoRotate,
    MothershipSpecialActivateNoTarget,
    shipBigGunSpecialTarget,
    MothershipSpecialOrderCleanup,
    MothershipHouseKeep,
    MothershipRemoveShipReferences,
    MothershipRemoveShipReferences,
    MothershipNotifyObjectDeleted,
    shipBigGunNotifyObjectDeleted,
    MothershipDied,
    Mothership_PreFix,
    Mothership_Save,
    Mothership_Load,
    Mothership_Fix,
    MothershipMoveOrderCleanup,
    MothershipAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MothershipCanHalt
};
