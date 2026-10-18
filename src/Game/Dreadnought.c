/*=============================================================================
    Name    : Dreadnought.c
    Purpose : Decompiled functions from cataclysm.exe (Dreadnought module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "Dreadnought.h"
#include "glinc.h"
#include "spaceobj.h"
#include "AIShip.h"
#include "AITrack.h"
#include "Attack.h"
#include "Clamp.h"
#include "Collision.h"
#include "CommandLayer.h"
#include "FastMath.h"
#include "FlightMan.h"
#include "GenericInterceptor.h"
#include "Gun.h"
#include "etg.h"
#include "Randy.h"
#include "RepairDroid.h"
#include "render.h"
#include "SaveGame.h"
#include "Ship.h"
#include "SoundEvent.h"
#include "statscript.h"
#include "Timer.h"
#include "universe.h"
#include "UnivUpdate.h"
#include <math.h>

#define DREADNOUGHT_CLAMP_TARGET_OFFSET 0x1BC
#define DREADNOUGHT_CANNON_HUD_ACTIVE_OFFSET 0x1398
#define DREADNOUGHT_CANNON_RECHARGE_TIMER_OFFSET 0x1368
#define DREADNOUGHT_CANNON_SYNC_TIMER_OFFSET     0x1378
#define DREADNOUGHT_CANNON_SPEECH_TIMER_OFFSET   0x1388
#define DREADNOUGHT_CANNON_SHIP_OFFSET  0x139C

#define DREADNOUGHT_CANNON_RECHARGE_DURATION 70.0f
#define DREADNOUGHT_CANNON_SPEECH_DELAY      1.0f
#define DREADNOUGHT_CANNON_SYNC_DELAY        0.01f
#define DREADNOUGHT_SPECIAL_ACTIVATE_EVENT   0x40008AC
#define DREADNOUGHT_CANNON_TIMER_LINE_RADIUS       1000.0f
#define DREADNOUGHT_CANNON_TIMER_LINE_SEGMENTS     80
#define DREADNOUGHT_CANNON_TIMER_LINE_JITTER       3.0f
#define DREADNOUGHT_CANNON_TIMER_LINE_ANGLE_SCALE  6.2831854820251465f
#define DREADNOUGHT_CANNON_TIMER_LINE_POLAR_ANGLE  1.5707963705062866

void rndRenderJaggedLine3(
    real32 x0,
    real32 y0,
    real32 z0,
    real32 x1,
    real32 y1,
    real32 z1,
    sdword segments,
    real32 jitterRadius);

typedef struct
{
    real32 gunRange[NUM_TACTICS_TYPES];
    real32 tooCloseRange[NUM_TACTICS_TYPES];
    real32 missileRegenerateTime;
    real32 missileVolleyTime;
    real32 missileLagVolleyTime;
} DreadnoughtStatics;

typedef struct
{
    real32 lasttimeRegeneratedMissiles;
    real32 lasttimeFiredVolley;
    real32 lasttimeDidSpecialTargeting;
    udword volleyState;
    ubyte unknown10_5F[0x50];
    udword unknownStateSlots[832]; // 0x60: 40 strided state slots (stride 0x54 = 21 udwords), per dreadnoughtInit @0x5B0EB0
    udword cannonArmed;
    Ship *cannonShip;
} DreadnoughtRuntimeOverlay;

static_assert(sizeof(DreadnoughtRuntimeOverlay) == sizeof(DreadnoughtSpecifics));
static_assert(offsetof(DreadnoughtRuntimeOverlay, unknownStateSlots) == 0x60);
static_assert(offsetof(DreadnoughtRuntimeOverlay, cannonArmed) == 0xD60);
static_assert(offsetof(DreadnoughtRuntimeOverlay, cannonShip) == 0xD64);

static DreadnoughtStatics dreadnoughtStatics;

static scriptStructEntry DreadnoughtScriptTable[] =
{
    { "MissileRegenerateTime", scriptSetReal32CB, (udword)&dreadnoughtStatics.missileRegenerateTime, (udword)&dreadnoughtStatics },
    { "MissileVolleyTime", scriptSetReal32CB, (udword)&dreadnoughtStatics.missileVolleyTime, (udword)&dreadnoughtStatics },
    { "MissileLagVolleyTime", scriptSetReal32CB, (udword)&dreadnoughtStatics.missileLagVolleyTime, (udword)&dreadnoughtStatics },
    END_SCRIPT_STRUCT_ENTRY
};

static DreadnoughtRuntimeOverlay *dreadnoughtRuntime(Ship *ship)
{
    return (DreadnoughtRuntimeOverlay *)ship->ShipSpecifics;
}

static udword *dreadnoughtUnknownStateSlot(DreadnoughtRuntimeOverlay *runtime, sdword index)
{
    return &runtime->unknownStateSlots[21 * index];
}

static void dreadnoughtRegenerateMissile(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    GunStatic *gunstatic;
    sdword minMissiles;
    sdword minIndex;

    dbgAssert(gunInfo != NULL);

    numGuns = gunInfo->numGuns;
    for (i = 0, minIndex = -1, minMissiles = 100000; i < numGuns; i++)
    {
        gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
        if (gunstatic->guntype == GUN_MissileLauncher)
        {
            gun = &gunInfo->guns[i];
            if (gun->numMissiles < gunstatic->maxMissiles && gun->numMissiles < minMissiles)
            {
                minMissiles = gun->numMissiles;
                minIndex = i;
            }
        }
    }

    if (minIndex != -1)
    {
        gunInfo->guns[minIndex].numMissiles++;
    }
}

static etgeffectstatic *dreadnoughtGetLinkingEffectStat(void)
{
    etglod *effectLOD = etgSpecialPurposeEffectTable[EGT_LINKING_EFFECT];

    if (effectLOD == NULL || effectLOD->nLevels <= 0)
    {
        return NULL;
    }
    return effectLOD->level[0];
}


// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B0E50
void dreadnoughtStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    sdword i;

    statinfo->custstatinfo = &dreadnoughtStatics;
    scriptSetStruct(directory, filename, DreadnoughtScriptTable, &dreadnoughtStatics);

    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        dreadnoughtStatics.gunRange[i] = statinfo->bulletRange[i];
        dreadnoughtStatics.tooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B0EB0
void dreadnoughtInit(Ship *ship)
{
    DreadnoughtRuntimeOverlay *runtime = dreadnoughtRuntime(ship);
    sdword i;

    runtime->lasttimeRegeneratedMissiles = 0.0f;
    runtime->lasttimeFiredVolley = 0.0f;
    runtime->lasttimeDidSpecialTargeting = -100000.0f;
    runtime->volleyState = 0;
    TimerInitTimerStruct((Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_RECHARGE_TIMER_OFFSET), DREADNOUGHT_CANNON_RECHARGE_DURATION);

    for (i = 0; i < 40; i++)
    {
        *dreadnoughtUnknownStateSlot(runtime, i) = 1;
    }

    runtime->cannonArmed = 0;
    runtime->cannonShip = NULL;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B3870
void dreadnoughtFire(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    GunInfo *customDockInfo = ship->cataclysmGunInfo;
    Gun *gun;
    sdword i;

    (void)maxdist;

    if (customDockInfo == NULL || customDockInfo->numGuns <= 0)
    {
        return;
    }

    gun = &customDockInfo->guns[0];
    for (i = 0; i < customDockInfo->numGuns; i++, gun++)
    {
        if (!gunCanShootStaggered(ship, gun))
        {
            continue;
        }

        switch (gun->gunstatic->guntype)
        {
            case GUN_Fixed:
            case GUN_Gimble:
            case GUN_NewGimble:
                gunShootGunAtTarget(ship, gun, target);
                break;

            case GUN_MissileLauncher:
            case GUN_MineLauncher:
                if (gun->numMissiles > 0)
                {
                    missileShoot(ship, gun, target);
                }
                break;

            default:
                break;
        }
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B0F10
void dreadnoughtHousekeep(Ship *ship)
{
    DreadnoughtRuntimeOverlay *runtime = dreadnoughtRuntime(ship);
    DreadnoughtStatics *statics = (DreadnoughtStatics *)ship->staticinfo->custstatinfo;
    Timer *rechargeTimer = (Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_RECHARGE_TIMER_OFFSET);
    Timer *syncTimer = (Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_SYNC_TIMER_OFFSET);
    Timer *speechTimer = (Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_SPEECH_TIMER_OFFSET);

    if (universe.totaltimeelapsed - runtime->lasttimeDidSpecialTargeting > statics->missileLagVolleyTime &&
        universe.totaltimeelapsed - runtime->lasttimeRegeneratedMissiles > statics->missileRegenerateTime)
    {
        runtime->lasttimeRegeneratedMissiles = universe.totaltimeelapsed;
        dreadnoughtRegenerateMissile(ship);
    }

    if (runtime->cannonShip != NULL && runtime->cannonArmed && TimerExpired((sdword)speechTimer))
    {
        repairDroidHyperspaceReset(ship);
        runtime->cannonArmed = 0;

        if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPULSE) && TimerExpired((sdword)rechargeTimer))
        {
            shockwaveCreateTyped(
                4,
                *(udword *)&ship->posinfo.position.x,
                *(udword *)&ship->posinfo.position.y,
                *(udword *)&ship->posinfo.position.z,
                ship->playerowner->playerIndex);
            TimerInitTimerStruct(rechargeTimer, DREADNOUGHT_CANNON_RECHARGE_DURATION);
        }
    }

    if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPULSE) && runtime->cannonShip == NULL)
    {
        if (!ship->unk_590)
        {
            dreadnoughtCreateCannon(ship);
            TimerInitTimerStruct(syncTimer, DREADNOUGHT_CANNON_SYNC_DELAY);
        }
    }

    if (runtime->cannonShip != NULL && !TimerExpired((sdword)syncTimer))
    {
        runtime->cannonShip->buildTimerElapsed = TimerTimeRemaining(syncTimer);
    }
}

// Decompiled from cataclysm.exe @ 0x5B1160
bool32 dreadnoughtSpecialActivate(Ship *ship)
{
    DreadnoughtRuntimeOverlay *runtime = dreadnoughtRuntime(ship);

    if (runtime->cannonShip != NULL &&
        ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPULSE) &&
        runtime->cannonArmed == 0 &&
        TimerExpired((sdword)((ubyte *)ship + DREADNOUGHT_CANNON_RECHARGE_TIMER_OFFSET)))
    {
        speechEventQueue(ship, DREADNOUGHT_SPECIAL_ACTIVATE_EVENT, -1, -1, -1, -1, -1, 0.0f, -1);
        runtime->cannonArmed = 1;
        TimerInitTimerStruct((Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_SPEECH_TIMER_OFFSET), DREADNOUGHT_CANNON_SPEECH_DELAY);
    }

    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x6FE9A0
bool32 dreadnoughtSpecialTargetStub(Ship *ship, void *custom)
{
    (void)ship;
    (void)custom;
    return FALSE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B1270
void dreadnoughtRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    if (*(Ship **)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET) == shiptoremove)
    {
        *(udword *)((ubyte *)ship + DREADNOUGHT_CANNON_HUD_ACTIVE_OFFSET) = 0;
        *(Ship **)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET) = NULL;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B11E0
void dreadnoughtClose(Ship *ship)
{
    Ship *cannon = *(Ship **)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET);

    if (cannon != NULL)
    {
        if (cannon->clampInfo != NULL &&
            cannon->clampInfo->host != NULL &&
            ((Ship *)cannon->clampInfo->host)->shiptype == sMothership)
        {
            clDetachShipFromCommand(cannon);
            cannon->flags &= ~SOF_ForwardDamageToClampHost;
        }
        univReallyDeleteThisShipRightNow(cannon);
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B1230
void dreadnought_PreFix(Ship *ship)
{
    *(sdword *)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET) =
        SpaceObjRegistryGetID((SpaceObj *)*(Ship **)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET));
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B1250
void dreadnought_Fix(Ship *ship)
{
    *(Ship **)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET) =
        SpaceObjRegistryGetShip(*(sdword *)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET));
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B1290
bool32 dreadnoughtHasCannon(Ship *ship)
{
    return ship->shiptype == sDreadnought && ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPULSE);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x514080
real32 shipUpgradeDurationGet(Ship *ship, udword upgradeMask)
{
    return shipUpgradeDurationUnitsGet(upgradeMask) * ship->staticinfo->upgradeTimeModifier;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B12C0
real32 dreadnoughtGetCannonTimerRemaining(Ship *ship)
{
    real32 result;

    result = TimerTimeRemaining((Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_RECHARGE_TIMER_OFFSET));
    if (result > 1.0f)
    {
        return 1.0f;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B12F0
void dreadnoughtCreateCannon(Ship *dreadnought)
{
    Ship *cannon;
    Player *playerowner;
    SpaceObj *clampTarget;
    real32 duration;

    duration = shipUpgradeDurationGet(dreadnought, 0x400);
    TimerInitTimerStruct((Timer *)((ubyte *)dreadnought + DREADNOUGHT_CANNON_SYNC_TIMER_OFFSET), duration);
    cannon = univCreateShip(
                 sDreadnoughtCannon,
                 dreadnought->shiprace,
                 &dreadnought->posinfo.position,
                 dreadnought->playerowner,
                 0);
    playerowner = cannon->playerowner;
    cannon->rotinfo.coordsys = dreadnought->rotinfo.coordsys;
    gameStatsAddShip(cannon, playerowner->playerIndex);
    cat_unitCapCreateShip(cannon, cannon->playerowner);
    clampTarget = NULL;
    if (dreadnought->clampInfo != NULL)
    {
        clampTarget = (SpaceObj *)dreadnought->clampInfo->host;
    }
    if (clampTarget != NULL)
        clampObjectSetup((SpaceObj *)cannon, clampTarget);
    else
        clampObjectSetup((SpaceObj *)cannon, (SpaceObj *)dreadnought);
    cannon->flags |= SOF_ForwardDamageToClampHost;
    *(Ship **)((ubyte *)dreadnought + DREADNOUGHT_CANNON_SHIP_OFFSET) = cannon;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B13B0
void dreadnoughtReattachCannon(Ship *dreadnought)
{
    Ship *cannon;

    cannon = *(Ship **)((ubyte *)dreadnought + DREADNOUGHT_CANNON_SHIP_OFFSET);
    if (cannon)
    {
        clDetachShipFromCommand(cannon);
        clampObjectSetup((SpaceObj *)cannon, (SpaceObj *)dreadnought);
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x516A40
static sdword dreadnoughtRenderCannonTimerLineEffect(vector *lineStart, real32 angle)
{
    real32 theta = angle * DREADNOUGHT_CANNON_TIMER_LINE_ANGLE_SCALE;
    real32 sinPolarAngle = (real32)sin(DREADNOUGHT_CANNON_TIMER_LINE_POLAR_ANGLE);
    vector effectPosition;
    vector zeroVelocity = { 0.0f, 0.0f, 0.0f };
    etgeffectstatic *effectStat = dreadnoughtGetLinkingEffectStat();
    sdword textureEnabled;
    sdword lightingEnabled;

    effectPosition.x = (real32)(cos(theta) * DREADNOUGHT_CANNON_TIMER_LINE_RADIUS * sinPolarAngle + lineStart->x);
    effectPosition.y = (real32)(sin(theta) * DREADNOUGHT_CANNON_TIMER_LINE_RADIUS * sinPolarAngle + lineStart->y);
    effectPosition.z = (real32)(cos(DREADNOUGHT_CANNON_TIMER_LINE_POLAR_ANGLE) * DREADNOUGHT_CANNON_TIMER_LINE_RADIUS + lineStart->z);

    if (!universePause && effectStat != NULL && !etgFrequencyExceeded(effectStat))
    {
        matrix effectCoordsys = defaultshipmatrix;

        etgEffectCreate(effectStat, NULL, &effectPosition, &zeroVelocity, &effectCoordsys, 1.0f, 120, 0);
    }

    textureEnabled = rndTextureEnable(FALSE);
    lightingEnabled = rndLightingEnable(FALSE);
    glColor3f(1.0f, 1.0f, 1.0f);
    rndRenderJaggedLine3(
        lineStart->x,
        lineStart->y,
        lineStart->z,
        effectPosition.x,
        effectPosition.y,
        effectPosition.z,
        DREADNOUGHT_CANNON_TIMER_LINE_SEGMENTS,
        DREADNOUGHT_CANNON_TIMER_LINE_JITTER);
    rndTextureEnable(textureEnabled);
    return rndLightingEnable(lightingEnabled);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B1110
sdword dreadnoughtRenderCannonTimerLine(Ship *ship)
{
    Ship *cannon;

    if (ship->shiptype == sDreadnought &&
        *(udword *)((ubyte *)ship + DREADNOUGHT_CANNON_HUD_ACTIVE_OFFSET) != 0)
    {
        cannon = *(Ship **)((ubyte *)ship + DREADNOUGHT_CANNON_SHIP_OFFSET);
        if (cannon != NULL)
        {
            real32 timerRemaining = TimerTimeRemaining((Timer *)((ubyte *)ship + DREADNOUGHT_CANNON_SYNC_TIMER_OFFSET));

            return dreadnoughtRenderCannonTimerLineEffect(&cannon->collInfo.collPosition, timerRemaining);
        }
    }
    return FALSE;
}
