/*=============================================================================
    Name    : ShipHelpers.c
    Purpose : Shared ship helper subset recovered ahead of Ship.c
=============================================================================*/

#include "types.h"
#include "Ship.h"
#include "AIShip.h"
#include "AITrack.h"
#include "AIVar.h"
#include "Collision.h"
#include "CommandDefs.h"
#include "consMgr.h"
#include "fastmath.h"
#include "Attack.h"
#include "etg.h"
#include "Globals.h"
#include "Gun.h"
#include "madLinkIn.h"
#include "MadLinkInDefs.h"
#include "nis.h"
#include "Randy.h"
#include "SalCapCorvette.h"
#include "Sensors.h"
#include "GenericInterceptor.h"
#include "ShipDefs.h"

#include "Timer.h"
#include "statscript.h"
#include "objtypes.h"
#include "SaveGame.h"
#include "singleplayer.h"
#include "universe.h"
#include "UnivUpdate.h"
#include "SalCapCorvette.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static real32 shipDisabledEffectLifeLow = 3.0f;
static real32 shipDisabledEffectLifeHigh = 5.0f;

enum
{
    SHIP_NAMES_MAX = 100,
    SHIP_NAMES_SOMTAAW_MOTHERSHIP_MIN = 8,
    SHIP_NAMES_BEAST_MOTHERSHIP_MIN = 8,
    SHIP_NAMES_SOMTAAW_CARRIER_MIN = 16,
    SHIP_NAMES_BEAST_CARRIER_MIN = 16,
};

static char shipNamesScriptFile[] = "shipnames.script";
ShipNameReservationEntry shipNameReservationStorage[SHIP_NAMES_MAX];
ShipNameReservationEntry *shipNameReservations = NULL;
sdword shipNameReservationCount = 0;
static char shipNameReservationEmpty[] = "";
sdword shipNamesCountFreeByType(ShipType shipType);

static const real32 shipWaitingForShipsTimerSeconds = 70.0f;
static const real32 shipWaitingForShipsRadiusBase = 18000.0f;
static const real32 shipWaitingForShipsRadiusRange = 50000.0f;
static const real32 shipWaitingForShipsRandomScale = 2.3283064365386963e-10f;
static const real32 shipRotateAndShootHeadingAccuracy = 0.99900001f;             /* cataclysm.exe 0x74D6EC */
static const real32 shipSharedAttackBombingRunRangeScale = 0.30000001192092896f; /* cataclysm.exe 0x74D0C8 */
static const real32 shipSharedAttackBombingRunDistance = 700.0f;                  /* cataclysm.exe 0x74D0CC */
static const real32 shipSharedAttackBombingRunHeight = 1000.0f;                   /* cataclysm.exe 0x74D0D0 */
static const real32 shipSharedAttackBreakDistance = 15000.0f;                     /* cataclysm.exe 0x74D0D4 */
static const real32 shipSharedAttackBreakVerticalThreshold = 0.75f;               /* cataclysm.exe 0x74D0D8 */
static const real32 shipSharedAttackAngleScale = 0.01745329238474369f;            /* cataclysm.exe 0x74D0DC */
static const real32 shipSharedAttackInitialOrbitAngle = 0.7853981852531433f;      /* cataclysm.exe 0x74D0E0 */

#define SHIP_SHARED_ATTACK_MAX_FLY_AWAY_OFFSET  0x0A0
#define SHIP_SHARED_ATTACK_BREAK_RANGE_OFFSET   0x13C
#define SHIP_SHARED_ATTACK_FLY_PAST_OFFSET      0x1D8
#define SHIP_SHARED_ATTACK_FACE_ACCURACY_OFFSET 0x310
#define SHIP_SHARED_ATTACK_MAX_TIME_OFFSET      0x3AC
#define SHIP_SHARED_ATTACK_FLIGHTMAN_OFFSET     0x448
#define SHIP_SHARED_ATTACK_FLIGHTMAN_STRIDE     0x148

#define SHIP_MODULE_SLOT_CHUNK_LARGE  ((TypeOfSaveChunk)0x8000B000)
#define SHIP_MODULE_SLOT_CHUNK_SMALL0 ((TypeOfSaveChunk)0x8000A000)
#define SHIP_MODULE_SLOT_CHUNK_SMALL1 ((TypeOfSaveChunk)0x80009000)

extern sdword nisIsRunning_;
extern scriptStructEntry LIStaticScriptTable[];
extern bool32 BombersUseBombingRun;
void telldir(void);

typedef struct
{
    real32 gunRange[NUM_TACTICS_TYPES];
    real32 tooCloseRange[NUM_TACTICS_TYPES];
} ShipStraightForwardAttackStatics;

typedef struct
{
    real32 state00;
    real32 state04;
    real32 state08;
    real32 state0C;
    union
    {
        real32 state10;
        udword state10Count;
    };
    uword state14;
    uword pad16;
    ubyte unk_18_1F[0x08];
    real32 state20;
} ShipSharedFlightManAttackRuntime;

static_assert(sizeof(ShipSharedFlightManAttackRuntime) == 0x24);

static GenericInterceptorStatics shipAttackStaticRecon;
static GenericInterceptorStatics shipAttackStaticSectAcolyte;
static GenericInterceptorStatics shipAttackStaticMimic;
static GenericInterceptorStatics shipAttackStaticBeastRecon;
static GenericInterceptorStatics shipAttackStaticBeastAcolyte;
static GenericInterceptorStatics shipAttackStaticBeastInterceptor;
static GenericInterceptorStatics shipAttackStaticBeastCloakedFighter;
static GenericInterceptorStatics shipAttackStaticBeastAttackBomber;
static GenericInterceptorStatics shipAttackStaticKushanAttackBomber;
static GenericInterceptorStatics shipAttackStaticKushanHeavyInterceptor;
static GenericInterceptorStatics shipAttackStaticTaiidanAttackBomber;
static GenericInterceptorStatics shipAttackStaticTaiidanHeavyInterceptor;
static GenericInterceptorStatics shipAttackStaticP1Fighter;
static GenericInterceptorStatics shipAttackStaticDerelictAcolyte;

static bool32 shipIsSectOrBeastAcolyte(Ship *ship)
{
    return ship->shiptype == sAcolyte || ship->shiptype == bAcolyte;
}

static real32 *aabbMerge(real32 *dest, real32 *aabb0, real32 *aabb1)
{
    real32 maxX = max(aabb0[0] + aabb0[3], aabb1[0] + aabb1[3]);
    real32 maxY = max(aabb0[1] + aabb0[4], aabb1[1] + aabb1[4]);
    real32 maxZ = max(aabb0[2] + aabb0[5], aabb1[2] + aabb1[5]);
    real32 minX = min(aabb0[0] - aabb0[3], aabb1[0] - aabb1[3]);
    real32 minY = min(aabb0[1] - aabb0[4], aabb1[1] - aabb1[4]);
    real32 minZ = min(aabb0[2] - aabb0[5], aabb1[2] - aabb1[5]);

    dest[3] = (maxX - minX) * 0.5f;
    dest[4] = (maxY - minY) * 0.5f;
    dest[5] = (maxZ - minZ) * 0.5f;
    dest[0] = dest[3] + minX;
    dest[1] = dest[4] + minY;
    dest[2] = dest[5] + minZ;
    return dest;
}

Ship *shipGetCompoundAABB(Ship *ship, real32 *compoundAABB)
{
    Ship *insideShip;
    StaticCollInfo *collInfo = &ship->staticinfo->staticheader.staticCollInfo;

    compoundAABB[0] = collInfo->uplength * 0.5f + collInfo->collrectoffset.x;
    compoundAABB[1] = collInfo->rightlength * 0.5f + collInfo->collrectoffset.y;
    compoundAABB[2] = collInfo->forwardlength * 0.5f + collInfo->collrectoffset.z;
    compoundAABB[3] = collInfo->uplength;
    compoundAABB[4] = collInfo->rightlength;
    compoundAABB[5] = collInfo->forwardlength;

    for (insideShip = GetFirstInsideShip(ship); insideShip != NULL; insideShip = GetNextInsideShip())
    {
        real32 insideAABB[6];

        collInfo = &insideShip->staticinfo->staticheader.staticCollInfo;
        insideAABB[0] = collInfo->uplength * 0.5f + collInfo->collrectoffset.x;
        insideAABB[1] = collInfo->rightlength * 0.5f + collInfo->collrectoffset.y;
        insideAABB[2] = collInfo->forwardlength * 0.5f + collInfo->collrectoffset.z;
        insideAABB[3] = collInfo->uplength;
        insideAABB[4] = collInfo->rightlength;
        insideAABB[5] = collInfo->forwardlength;
        aabbMerge(compoundAABB, compoundAABB, insideAABB);
    }
    return insideShip;
}

real64 shipDistanceToCompoundAABB(real32 *point, Ship *ship)
{
    vector localPoint;
    vector pointInShipCoords;
    real32 compoundAABB[6];
    real64 xDistance;
    real64 yDistance;
    real64 zDistance;

    localPoint.x = point[0] - ship->posinfo.position.x;
    localPoint.y = point[1] - ship->posinfo.position.y;
    localPoint.z = point[2] - ship->posinfo.position.z;
    matMultiplyVecByMat(&pointInShipCoords, &localPoint, &ship->rotinfo.coordsys);
    shipGetCompoundAABB(ship, compoundAABB);

    if (pointInShipCoords.x < compoundAABB[0] - compoundAABB[3])
    {
        xDistance = (compoundAABB[0] - compoundAABB[3]) - pointInShipCoords.x;
    }
    else if (pointInShipCoords.x > compoundAABB[0] + compoundAABB[3])
    {
        xDistance = (compoundAABB[0] + compoundAABB[3]) - pointInShipCoords.x;
    }
    else
    {
        xDistance = pointInShipCoords.x;
    }

    if (pointInShipCoords.y < compoundAABB[1] - compoundAABB[4])
    {
        yDistance = (compoundAABB[1] - compoundAABB[4]) - pointInShipCoords.y;
    }
    else if (pointInShipCoords.y > compoundAABB[1] + compoundAABB[4])
    {
        yDistance = (compoundAABB[1] + compoundAABB[4]) - pointInShipCoords.y;
    }
    else
    {
        yDistance = pointInShipCoords.y;
    }

    if (pointInShipCoords.z < compoundAABB[2] - compoundAABB[5])
    {
        zDistance = (compoundAABB[2] - compoundAABB[5]) - pointInShipCoords.z;
    }
    else if (pointInShipCoords.z > compoundAABB[2] + compoundAABB[5])
    {
        zDistance = (compoundAABB[2] + compoundAABB[5]) - pointInShipCoords.z;
    }
    else
    {
        zDistance = pointInShipCoords.z;
    }

    return fmathSqrtDouble(zDistance * zDistance + yDistance * yDistance + xDistance * xDistance);
}

void shipModuleSlotSave(void)
{
    SaveChunk *chunk;

    chunk = CreateChunk(SHIP_MODULE_SLOT_CHUNK_LARGE, 6048, smMimicPlayerTypeData());
    SaveThisChunk(chunk);
    memFree(chunk);

    chunk = CreateChunk(SHIP_MODULE_SLOT_CHUNK_SMALL0, 36, smMimicPlayerTypeExtra0());
    SaveThisChunk(chunk);
    memFree(chunk);

    chunk = CreateChunk(SHIP_MODULE_SLOT_CHUNK_SMALL1, 36, smMimicPlayerTypeExtra1());
    SaveThisChunk(chunk);
    memFree(chunk);
}

void shipModuleSlotLoad(void)
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, SHIP_MODULE_SLOT_CHUNK_LARGE);
    memcpy(smMimicPlayerTypeData(), chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, SHIP_MODULE_SLOT_CHUNK_SMALL0);
    memcpy(smMimicPlayerTypeExtra0(), chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, SHIP_MODULE_SLOT_CHUNK_SMALL1);
    memcpy(smMimicPlayerTypeExtra1(), chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);
}

static GenericInterceptorStatics *shipAttackStaticStorageForType(ShipType shipType)
{
    switch (shipType)
    {
        case sRecon:
            return &shipAttackStaticRecon;
        case sAcolyte:
            return &shipAttackStaticSectAcolyte;
        case sMimic:
            return &shipAttackStaticMimic;
        case bRecon:
            return &shipAttackStaticBeastRecon;
        case bAcolyte:
            return &shipAttackStaticBeastAcolyte;
        case bInterceptor:
            return &shipAttackStaticBeastInterceptor;
        case bCloakedFighter:
            return &shipAttackStaticBeastCloakedFighter;
        case bAttackBomber:
            return &shipAttackStaticBeastAttackBomber;
        case kAttackBomber:
            return &shipAttackStaticKushanAttackBomber;
        case kHeavyInterceptor:
            return &shipAttackStaticKushanHeavyInterceptor;
        case tAttackBomber:
            return &shipAttackStaticTaiidanAttackBomber;
        case tHeavyInterceptor:
            return &shipAttackStaticTaiidanHeavyInterceptor;
        case p1Fighter:
            return &shipAttackStaticP1Fighter;
        case dAcolyte:
            return &shipAttackStaticDerelictAcolyte;
        default:
            return NULL;
    }
}

void *shipAttackStaticInfoScript(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    GenericInterceptorStatics *attackStatic;
    TacticsType tacticsType;
    sdword classIndex;

    attackStatic = shipAttackStaticStorageForType(statinfo->shiptype);
    if (attackStatic == NULL)
    {
        dbgFatalf(DBG_Loc, "\nshipAttackStaticInfoScript: unsupported ship type %d", statinfo->shiptype);
        return NULL;
    }

    statinfo->custstatinfo = attackStatic;
    memset(attackStatic, 0, sizeof(*attackStatic));
    scriptSetStruct(directory, filename, LIStaticScriptTable, (ubyte *)attackStatic);

    for (tacticsType = Evasive; tacticsType < NUM_TACTICS_TYPES; tacticsType++)
    {
        for (classIndex = 0; classIndex < NUM_CLASSES; classIndex++)
        {
            real32 maxFlyAwayDist = attackStatic->maxFlyAwayDist[tacticsType][classIndex];
            attackStatic->maxFlyAwayDistSquared[tacticsType][classIndex] = maxFlyAwayDist * maxFlyAwayDist;
        }
    }

    for (tacticsType = Evasive; tacticsType < NUM_TACTICS_TYPES; tacticsType++)
    {
        for (classIndex = 0; classIndex < NUM_CLASSES; classIndex++)
        {
            if (attackStatic->flyPastDist[tacticsType][classIndex] == 0.0f)
            {
                attackStatic->maxFlyAwayDist[tacticsType][classIndex] =
                    attackStatic->maxFlyAwayDist[tacticsType][CLASS_Mine];
                attackStatic->breakRange[tacticsType][classIndex] =
                    attackStatic->breakRange[tacticsType][CLASS_Mine];
                attackStatic->flyPastDist[tacticsType][classIndex] =
                    attackStatic->flyPastDist[tacticsType][CLASS_Mine];
                attackStatic->triggerHappy[tacticsType][classIndex] =
                    attackStatic->triggerHappy[tacticsType][CLASS_Mine];
                attackStatic->faceTargetAccuracy[tacticsType][classIndex] =
                    attackStatic->faceTargetAccuracy[tacticsType][CLASS_Mine];
                attackStatic->maxAttackTime[tacticsType][classIndex] =
                    attackStatic->maxAttackTime[tacticsType][CLASS_Mine];
                attackStatic->flightmanProb[tacticsType][classIndex] =
                    attackStatic->flightmanProb[tacticsType][CLASS_Mine];
            }
        }
    }

    return attackStatic;
}

bool32 shipCanFireNextGunAtTarget(Ship *ship, SpaceObjRotImpTarg *target)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    real32 *attackStatic = (real32 *)ship->staticinfo->custstatinfo;
    sdword gunCount;
    uword currentGunIndex;
    sdword i;

    if ((ship->flags2 & 8) != 0 && shipHasModuleSlotState(ship))
    {
        return FALSE;
    }

    if (ship->staticinfo->madStatic != NULL &&
        ship->staticinfo->madStatic->numGunOpenIndexes > 0)
    {
        if (ship->madGunStatus != MAD_STATUS_GUNS_OPEN)
        {
            if (ship->madGunStatus != MAD_STATUS_GUNS_OPENING)
            {
                madOpenGunsShip(ship);
            }
            return FALSE;
        }
    }

    if (universe.totaltimeelapsed - gunStateSpecifics(ship)->lastFireTime < *attackStatic)
    {
        return FALSE;
    }

    gunCount = gunInfo->numGuns;
    if (shipIsSectOrBeastAcolyte(ship))
    {
        gunCount /= 2;
    }

    currentGunIndex = gunStateSpecifics(ship)->currentGunIndex;
    if (gunCount <= 0)
    {
        return FALSE;
    }

    for (i = 0; i < gunCount; i++)
    {
        Gun *gun = &gunInfo->guns[currentGunIndex];

        if (gunCanShootStaggered(ship, gun))
        {
            bool32 canFire;
            gunStateSpecifics(ship)->currentGunIndex = currentGunIndex;

            switch (gun->gunstatic->guntype)
            {
                case GUN_Fixed:
                    canFire = gunCanHitTarget(ship, gun, target, TRUE);
                    break;
                case GUN_Gimble:
                    canFire = gunOrientGimbleGun(ship, gun, target);
                    break;
                case GUN_NewGimble:
                {
                    sdword j;
                    canFire = FALSE;

                    for (j = 0; j < gunInfo->numGuns; j++)
                    {
                        if (j == currentGunIndex)
                        {
                            canFire = gunOrientGimbleGun(ship, &gunInfo->guns[j], target);
                        }
                        else
                        {
                            gunOrientGimbleGun(ship, &gunInfo->guns[j], target);
                        }
                    }
                    break;
                }
                default:
                    canFire = FALSE;
                    break;
            }
            return canFire;
        }

        currentGunIndex++;
        if (currentGunIndex >= gunCount)
        {
            currentGunIndex = 0;
        }
    }
    return FALSE;
}

void shipShootNextGunAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    GunStateSpecifics *gunState = gunStateSpecifics(ship);
    uword currentGunIndex = gunState->currentGunIndex;

    (void)maxdist;

    gunState->lastFireTime = universe.totaltimeelapsed;
    gunShootGunAtTarget(ship, &gunInfo->guns[currentGunIndex], target);

    currentGunIndex++;
    if (currentGunIndex >= (uword)gunInfo->numGuns)
    {
        currentGunIndex = 0;
    }
    if (shipIsSectOrBeastAcolyte(ship) && currentGunIndex > 1)
    {
        currentGunIndex = 0;
    }

    gunState->currentGunIndex = currentGunIndex;
}

Ship *shipResetAttackState(Ship *ship)
{
    gunStateSpecifics(ship)->currentGunIndex = 0;
    return ship;
}

ubyte *shipGetAttackRecord(Ship *ship, SpaceObjRotImpTarg *target)
{
    ShipClass targetClass = CLASS_Mine;
    TacticsType tacticstype = ship->tacticstype;

    if (target != NULL && target->objtype == OBJ_ShipType)
    {
        targetClass = ((Ship *)target)->staticinfo->shipclass;
    }

    return (ubyte *)((ubyte *)ship->staticinfo->custstatinfo +
        328 * (targetClass + 13 * tacticstype) +
        1096);
}

void shipRotateAndShootAtTarget(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    ShipStaticInfo *staticinfo = ship->staticinfo;
    vector trajectory;
    real32 dist;
    real32 range;
    real32 temp;

    if (staticinfo->gunStaticInfo != NULL && ship->cataclysmGunInfo != NULL)
    {
        real32 bulletSpeed = gunGetActiveBulletSpeed(ship, &ship->cataclysmGunInfo->guns[0]);
        aishipGetTrajectoryWithVelPrediction(ship, target, bulletSpeed, &trajectory);
    }
    else
    {
        aishipGetTrajectory(ship, target, &trajectory);
    }

    dist = fsqrt(vecMagnitudeSquared(trajectory));
    vecDivideByScalar(trajectory, dist, temp);
    range = RangeToTargetGivenDist(ship, target, dist);

    if ((rotateFlags & staticinfo->rotateToRetaliate) != 0)
    {
        aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship,
                           &trajectory,
                           shipRotateAndShootHeadingAccuracy,
                           shipRotateAndShootHeadingAccuracy,
                           0.0f,
                           0);
    }

    if (range < staticinfo->bulletRange[ship->tacticstype] &&
        shipCanFireNextGunAtTarget(ship, target))
    {
        staticinfo->custshipheader.CustShipAttack(ship, target, 0.0f);
    }
}

// Decompiled from cataclysm.exe @ 0x5BE540
void shipSharedRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    if ((ship->staticinfo->rotateToRetaliate & rotateFlags) != 0)
    {
        attackPassive(ship, (Ship *)target);
    }
    else
    {
        attackSimple(ship, target);
    }
}

// Decompiled from cataclysm.exe @ 0x5BB3E0
bool32 shipStraightForwardAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStraightForwardAttackStatics *statics = (ShipStraightForwardAttackStatics *)ship->staticinfo->custstatinfo;
    TacticsType tacticstype = ship->tacticstype;

    (void)maxdist;

    attackStraightForward(
        ship,
        target,
        statics->gunRange[tacticstype],
        statics->tooCloseRange[tacticstype]);
    return TRUE;
}

static ShipClass shipSharedAttackTargetClass(const SpaceObjRotImpTarg *target)
{
    if (target != NULL && target->objtype == OBJ_ShipType)
    {
        return ((Ship *)target)->staticinfo->shipclass;
    }
    return CLASS_Mine;
}

static real32 shipSharedAttackScriptFloat(const ShipStaticInfo *staticinfo, ShipClass targetClass, sdword attackTacticsIndex, sdword offset)
{
    sdword index = targetClass + 13 * attackTacticsIndex;
    return *(real32 *)((ubyte *)staticinfo->custstatinfo + offset + sizeof(real32) * index);
}

static FlightManProb *shipSharedAttackFlightManProb(const ShipStaticInfo *staticinfo, ShipClass targetClass, sdword attackTacticsIndex)
{
    sdword index = targetClass + 13 * attackTacticsIndex;
    return (FlightManProb *)((ubyte *)staticinfo->custstatinfo +
        SHIP_SHARED_ATTACK_FLIGHTMAN_OFFSET +
        SHIP_SHARED_ATTACK_FLIGHTMAN_STRIDE * index);
}

static real32 shipTargetAttackPointExtent(const SpaceObjRotImpTarg *target)
{
    return target->staticinfo->staticheader.staticCollInfo.approxcollspheresize;
}

static bool32 shipTargetSupportsWhipStrafe(const ShipStaticInfo *targetStaticInfo)
{
    return targetStaticInfo->shipIsCapital &&
           targetStaticInfo->shipclass != CLASS_Mothership;
}

static inline real32 shipTargetVelocityDot(const SpaceObjRotImpTarg *target, const vector *trajectory)
{
    return trajectory->x * target->posinfo.velocity.x +
           trajectory->y * target->posinfo.velocity.y +
           trajectory->z * target->posinfo.velocity.z;
}

// Decompiled from cataclysm.exe @ 0x5B2350
static void shipComputeWhipStrafeAttackPoint(Ship *ship, ShipSharedFlightManAttackRuntime *runtime, SpaceObjRotImpTarg *target)
{
    real32 targetZ = target->posinfo.position.z;
    TacticsType tacticstype = ship->tacticstype;
    real32 attackPointDistance =
        shipSharedAttackBombingRunRangeScale * ship->staticinfo->bulletRange[tacticstype] +
        shipTargetAttackPointExtent(target);
    bool32 approachFromBelow;
    real32 dx;
    real32 dy;
    real32 dz;

    if (ship->posinfo.position.z <= targetZ)
    {
        targetZ -= attackPointDistance;
        approachFromBelow = FALSE;
    }
    else
    {
        targetZ += attackPointDistance;
        approachFromBelow = TRUE;
    }

    dx = target->posinfo.position.x - ship->posinfo.position.x;
    dy = target->posinfo.position.y - ship->posinfo.position.y;
    dz = targetZ - ship->posinfo.position.z;
    if ((dx * dx + dy * dy) * WHIPSTRAFE_MAXANGCOMPAREFACTOR < dz * dz)
    {
        return;
    }

    runtime->state08 = ship->posinfo.position.x + WHIPSTRAFE_FLYBYOVERSHOOT * dx;
    runtime->state0C = ship->posinfo.position.y + WHIPSTRAFE_FLYBYOVERSHOOT * dy;
    runtime->state10 = ship->posinfo.position.z + WHIPSTRAFE_FLYBYOVERSHOOT * dz;
    runtime->state14 = 1;

    flightmanInitFunc(ship, FLIGHTMAN_WHIP_STRAFE, -1);
    ship->flightmanState2 = (ubyte)approachFromBelow;
    ship->rcstate2 = STATE_WHIPSTRAFE1;
    runtime->state04 = universe.totaltimeelapsed;
}

// Decompiled from cataclysm.exe @ 0x5B2620
bool32 shipSharedFlightManAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipSharedFlightManAttackRuntime *runtime = (ShipSharedFlightManAttackRuntime *)ship->ShipSpecifics;
    ShipStaticInfo *staticinfo = ship->staticinfo;
    ShipClass targetClass = shipSharedAttackTargetClass(target);
    sdword attackTacticsIndex = ship->tacticstype;
    FlightManProb *flightManProb = shipSharedAttackFlightManProb(staticinfo, targetClass, attackTacticsIndex);
    vector trajectory;
    vector tempVector;
    vector destination;
    matrix rotationMatrix;
    real32 dist;
    real32 range;
    real32 bulletspeed;
    real32 accuracy;
    real32 temp;
    real32 rotationAngle;
    real32 sinAngle;
    real32 cosAngle;
    real32 absTrajectoryZ;
    sdword randomDegrees;
    sdword attackAxis;
    udword flightmanFlags;
    bool32 trajectoryBelowPlane;
    bool32 useStoredBreakDestination;
    vector *targetVelocity;

    (void)maxdist;

    switch (ship->rcstate2)
    {
        case STATE_INIT:
            runtime->state14 = 0;
            if ((ship->shiptype == kAttackBomber || ship->shiptype == tAttackBomber) &&
                BombersUseBombingRun &&
                targetClass <= CLASS_Frigate &&
                target->objtype == OBJ_ShipType)
            {
                ship->rcstate2 = STATE_START_BOMBING_RUN;
                runtime->state00 = ship->posinfo.position.x - target->posinfo.position.x;
                runtime->state04 = ship->posinfo.position.y - target->posinfo.position.y;
                runtime->state20 = 0.0f;
                vecNormalize((vector *)&runtime->state00);
                runtime->state00 *= shipSharedAttackBombingRunDistance;
                runtime->state04 *= shipSharedAttackBombingRunDistance;
                runtime->state20 *= shipSharedAttackBombingRunDistance;
                if (target->posinfo.position.z >= 0.0f)
                {
                    runtime->state20 -= shipSharedAttackBombingRunHeight;
                }
                else
                {
                    runtime->state20 += shipSharedAttackBombingRunHeight;
                }
                runtime->state0C = -1.0f;
                return TRUE;
            }

            ship->rcstate2 = STATE_APPROACH;
            aishipGetTrajectory(ship, target, &trajectory);
            aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 0x0A, 0.0f, NULL);
            range = RangeToTarget(ship, target, &trajectory);
            if (range < staticinfo->bulletRange[attackTacticsIndex])
            {
                ship->rcstate2 = STATE_KILL;
                runtime->state04 = universe.totaltimeelapsed;
            }
            return TRUE;

        case STATE_APPROACH:
            aishipGetTrajectory(ship, target, &trajectory);
            aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 0x0A, 0.0f, NULL);
            range = RangeToTarget(ship, target, &trajectory);
            if (range < staticinfo->bulletRange[attackTacticsIndex])
            {
                ship->rcstate2 = STATE_KILL;
                runtime->state04 = universe.totaltimeelapsed;
            }
            return TRUE;

        case STATE_KILL:
            if (staticinfo->gunStaticInfo != NULL && ship->cataclysmGunInfo != NULL)
            {
                bulletspeed = gunGetActiveBulletSpeed(ship, &ship->cataclysmGunInfo->guns[0]);
                aishipGetTrajectoryWithVelPrediction(ship, target, bulletspeed, &trajectory);
            }
            else
            {
                aishipGetTrajectory(ship, target, &trajectory);
            }

            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory, dist, temp);
            targetVelocity = (shipTargetVelocityDot(target, &trajectory) >= 0.0f) ? &target->posinfo.velocity : NULL;
            aishipFlyToPointAvoidingObjsFunc(
                ship,
                &target->posinfo.position,
                4,
                -staticinfo->staticheader.maxvelocity,
                targetVelocity);

            flightmanFlags = 0;
            if (ship->flightman == FLIGHTMAN_BARREL_ROLL ||
                ship->flightman == FLIGHTMAN_SWARMER_BARRELROLL ||
                ship->flightman == FLIGHTMAN_ROLL180)
            {
                flightmanFlags = 2;
                if (flightmanExecute(ship))
                {
                    ship->tacticsNeedToFlightMan = 0;
                }
            }
            else if (ship->flightman == FLIGHTMAN_CELEB_FLIP ||
                     ship->flightman == FLIGHTMAN_BARRELROLL_OUT)
            {
                flightmanExecute(ship);
                return TRUE;
            }

            accuracy = shipSharedAttackScriptFloat(staticinfo, targetClass, attackTacticsIndex, SHIP_SHARED_ATTACK_FACE_ACCURACY_OFFSET);
            aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &trajectory, accuracy, accuracy, staticinfo->sinbank, flightmanFlags);

            range = RangeToTargetGivenDist(ship, target, dist);
            if (range >= staticinfo->bulletRange[attackTacticsIndex])
            {
                return TRUE;
            }

            if (shipCanFireNextGunAtTarget(ship, target))
            {
                staticinfo->custshipheader.CustShipAttack(ship, target, 0.0f);
                if (ship->flightman == FLIGHTMAN_NULL && runtime->state14 == 0)
                {
                    runtime->state14 = 1;
                    if (flightmanTestRandom(flightManProb, 5, FLIGHTMAN_BARREL_ROLL))
                    {
                        ship->tacticsNeedToFlightMan |= 4u;
                        flightmanInitFunc(ship, FLIGHTMAN_BARREL_ROLL, -1);
                    }
                    else if (flightmanTestRandom(flightManProb, 5, FLIGHTMAN_CELEB_FLIP))
                    {
                        flightmanInitFunc(ship, FLIGHTMAN_CELEB_FLIP, -1);
                    }
                    else if (flightmanTestRandom(flightManProb, 5, FLIGHTMAN_SWARMER_BARRELROLL))
                    {
                        flightmanInitFunc(ship, FLIGHTMAN_SWARMER_BARRELROLL, -1);
                    }
                    else if (flightmanTestRandom(flightManProb, 5, FLIGHTMAN_ROLL180))
                    {
                        flightmanInitFunc(ship, FLIGHTMAN_ROLL180, -1);
                    }
                }
            }

            if (range >= shipSharedAttackScriptFloat(staticinfo, targetClass, attackTacticsIndex, SHIP_SHARED_ATTACK_BREAK_RANGE_OFFSET))
            {
                return TRUE;
            }

            randomDegrees = 25 + (sdword)(ranRandom(10) % (45 - 25 + 1));
            if (flightmanTestRandom(flightManProb, 5, FLIGHTMAN_BARRELROLL_OUT))
            {
                ship->rcstate2 = STATE_BARRELROLL_OUT;
                flightmanInitFunc(ship, FLIGHTMAN_BARRELROLL_OUT, -1);
                rotationAngle = (real32)randomDegrees;
            }
            else
            {
                ship->rcstate2 = STATE_BREAK;
                ship->tacticsNeedToFlightMan = 0;
                if (targetClass == CLASS_Mine)
                {
                    rotationAngle = (real32)randomDegrees;
                }
                else
                {
                    if ((randomDegrees & 1) == 0)
                    {
                        randomDegrees = -randomDegrees;
                    }
                    rotationAngle = (real32)randomDegrees;
                }
            }

            rotationAngle *= shipSharedAttackAngleScale;
            attackAxis = 7;
            if (ship->command != NULL &&
                (ship->command->ordertype.attributes & COMMAND_IS_FORMATION) != 0 &&
                attackTacticsIndex != 0)
            {
                attackAxis = ship->command->formation.formationtype;
            }

            trajectoryBelowPlane = (trajectory.z < 0.0f);
            absTrajectoryZ = trajectoryBelowPlane ? -trajectory.z : trajectory.z;

            tempVector = trajectory;
            vecMultiplyByScalar(tempVector, shipSharedAttackBreakDistance);
            sinAngle = (real32)sin(rotationAngle);
            cosAngle = (real32)cos(rotationAngle);
            if (absTrajectoryZ >= shipSharedAttackBreakVerticalThreshold)
            {
                matMakeRotAboutY(&rotationMatrix, cosAngle, sinAngle);
                matMultiplyMatByVec((vector *)&runtime->state08, &rotationMatrix, &tempVector);
            }
            else if (attackAxis >= 2)
            {
                matMakeRotAboutZ(&rotationMatrix, cosAngle, sinAngle);
                matMultiplyMatByVec((vector *)&runtime->state08, &rotationMatrix, &tempVector);
            }
            else
            {
                tempVector.z = 0.0f;
                randomDegrees = 10 + (sdword)(ranRandom(10) % (30 - 10 + 1));
                rotationAngle = (real32)randomDegrees * shipSharedAttackAngleScale;
                vecNormalizeToLength(&tempVector, 15000.0f);
                sinAngle = (real32)sin(rotationAngle);
                cosAngle = (real32)cos(rotationAngle);
                if (trajectoryBelowPlane)
                {
                    matMakeRotAboutX(&rotationMatrix, cosAngle, sinAngle);
                }
                else
                {
                    matMakeRotAboutY(&rotationMatrix, cosAngle, sinAngle);
                }
                matMultiplyMatByVec((vector *)&runtime->state08, &rotationMatrix, &tempVector);
                if (trajectoryBelowPlane)
                {
                    if (runtime->state10 < 0.0f)
                    {
                        runtime->state10 = -runtime->state10;
                    }
                }
                else if (runtime->state10 > 0.0f)
                {
                    runtime->state10 = -runtime->state10;
                }
            }

            runtime->state08 += target->posinfo.position.x;
            runtime->state0C += target->posinfo.position.y;
            runtime->state10 += target->posinfo.position.z;
            return TRUE;

        case STATE_BREAK:
        {
            aishipGetTrajectory(ship, target, &trajectory);
            useStoredBreakDestination = (target->objtype != OBJ_ShipType ||
                targetClass > CLASS_Frigate);
            real32 maxFlyAwayDistSquared =
                shipSharedAttackScriptFloat(staticinfo, targetClass, attackTacticsIndex, SHIP_SHARED_ATTACK_MAX_FLY_AWAY_OFFSET);
            bool32 reachedBreakDestination = MoveReachedDestinationVariable(
                ship,
                (vector *)&runtime->state08,
                WHIPSTRAFE_REACHDESTINATIONRANGE);
            if (vecMagnitudeSquared(trajectory) > maxFlyAwayDistSquared)
            {
                goto transition_to_turnaround;
            }
            if (reachedBreakDestination)
            {
                if (useStoredBreakDestination)
                {
                    goto transition_to_turnaround;
                }
            }
            if (!useStoredBreakDestination)
            {
                tempVector.x = runtime->state08 - target->posinfo.position.x;
                tempVector.y = runtime->state0C - target->posinfo.position.y;
                tempVector.z = runtime->state10 - target->posinfo.position.z;
                vecNormalize(&tempVector);
                tempVector.x = target->posinfo.position.x + tempVector.x * 20000.0f;
                tempVector.y = target->posinfo.position.y + tempVector.y * 20000.0f;
                tempVector.z = target->posinfo.position.z + tempVector.z * 20000.0f;
                if ((aishipFlyToPointAvoidingObjsFunc(ship, &tempVector, 0x36, WHIPSTRAFE_MINVELOCITY, NULL) & 1) == 0)
                {
                    return TRUE;
                }
                goto transition_to_turnaround;
            }
            if ((aishipFlyToPointAvoidingObjsFunc(ship, (vector *)&runtime->state08, 0x36, WHIPSTRAFE_MINVELOCITY, NULL) & 1) == 0)
            {
                return TRUE;
            }

transition_to_turnaround:
            matGetVectFromMatrixCol3(tempVector, ship->rotinfo.coordsys);
            range = vecDotProduct(trajectory, tempVector);
            if (range <= 0.0f)
            {
                udword chosenFlightman;

                ship->tacticsNeedToFlightMan |= 1u;
                chosenFlightman = flightmanGetRandom(flightManProb, 4);
                flightmanInitFunc(ship, chosenFlightman, 0);
            }
            else
            {
                flightmanInitFunc(ship, FLIGHTMAN_DONOTHING, 0);
            }
            ship->rcstate2 = STATE_TURNAROUND;
            return TRUE;
        }

        case STATE_TURNAROUND:
            if (!flightmanExecute(ship))
            {
                return TRUE;
            }

            ship->tacticsNeedToFlightMan = 0;
            ship->rcstate2 = STATE_REAPPROACH;
            runtime->state14 = 0;
            aishipResetFlyState(ship);
            if (ship->flightman != FLIGHTMAN_NULL ||
                target->objtype != OBJ_ShipType ||
                !shipTargetSupportsWhipStrafe((ShipStaticInfo *)target->staticinfo) ||
                !flightmanTestRandom(flightManProb, 5, FLIGHTMAN_WHIP_STRAFE))
            {
                return TRUE;
            }

            shipComputeWhipStrafeAttackPoint(ship, runtime, target);
            return TRUE;

        case STATE_REAPPROACH:
            aishipGetTrajectory(ship, target, &trajectory);
            aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 2, 0.0f, NULL);
            range = RangeToTarget(ship, target, &trajectory);
            if (range < staticinfo->bulletRange[attackTacticsIndex])
            {
                ship->rcstate2 = STATE_KILL;
                runtime->state04 = universe.totaltimeelapsed;
            }
            return TRUE;

        case STATE_WHIPSTRAFE1:
            if (ship->flightman != FLIGHTMAN_WHIP_STRAFE)
            {
                dbgMessagef("\nWarning: Whip strafe cancelled");
                ship->rcstate2 = STATE_INIT;
                return TRUE;
            }

            aishipGetTrajectoryWithAngleCorrection(ship, target, &trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            vecDivideByScalar(trajectory, dist, temp);
            if (RangeToTargetGivenDist(ship, target, dist) >= staticinfo->bulletRange[attackTacticsIndex] * 0.69999998807907104f)
            {
                aishipFlyToPointAvoidingObjsFunc(ship, (vector *)&runtime->state08, 6, WHIPSTRAFE_MINVELOCITY, NULL);
            }
            else
            {
                aishipFlyToPointAvoidingObjsFunc(ship, (vector *)&runtime->state08, 4, WHIPSTRAFE_MINVELOCITY, NULL);
                aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &trajectory, 0.99998999f, 0.99998999f, staticinfo->sinbank, 0);
                if (shipCanFireNextGunAtTarget(ship, target))
                {
                    staticinfo->custshipheader.CustShipAttack(ship, target, 0.0f);
                }
            }

            if (!MoveReachedDestinationVariable(ship, (vector *)&runtime->state08, WHIPSTRAFE_REACHDESTINATIONRANGE) &&
                universe.totaltimeelapsed - runtime->state04 <= shipSharedAttackScriptFloat(staticinfo, targetClass, attackTacticsIndex, SHIP_SHARED_ATTACK_MAX_TIME_OFFSET))
            {
                return TRUE;
            }

            ship->rcstate2 = STATE_WHIPSTRAFE2;
            return TRUE;

        case STATE_WHIPSTRAFE2:
            if (ship->flightman != FLIGHTMAN_WHIP_STRAFE)
            {
                dbgMessagef("\nWarning: Whip strafe cancelled");
                ship->rcstate2 = STATE_INIT;
                return TRUE;
            }

            if (!flightmanExecute(ship))
            {
                return TRUE;
            }

            ship->rcstate2 = STATE_BREAK;
            tempVector.x = ship->posinfo.position.x - target->posinfo.position.x;
            tempVector.y = ship->posinfo.position.y - target->posinfo.position.y;
            tempVector.z = 0.0f;
            vecNormalizeToLength(&tempVector,
                shipSharedAttackScriptFloat(staticinfo, targetClass, attackTacticsIndex, SHIP_SHARED_ATTACK_FLY_PAST_OFFSET));
            randomDegrees = 25 + (sdword)(ranRandom(10) % (45 - 25 + 1));
            if ((randomDegrees & 1) == 0)
            {
                randomDegrees = -randomDegrees;
            }
            rotationAngle = (real32)randomDegrees * shipSharedAttackAngleScale;
            sinAngle = (real32)sin(rotationAngle);
            cosAngle = (real32)cos(rotationAngle);
            matMakeRotAboutZ(&rotationMatrix, cosAngle, sinAngle);
            matMultiplyMatByVec((vector *)&runtime->state08, &rotationMatrix, &tempVector);
            runtime->state08 += target->posinfo.position.x;
            runtime->state0C += target->posinfo.position.y;
            runtime->state10 += target->posinfo.position.z;
            return TRUE;

        case STATE_BARRELROLL_OUT:
            if (!flightmanExecute(ship))
            {
                return TRUE;
            }
            ship->rcstate2 = STATE_BREAK;
            return TRUE;

        case STATE_START_BOMBING_RUN:
            if (runtime->state0C == -1.0f)
            {
                real32 sinOrbit = (real32)sin(shipSharedAttackInitialOrbitAngle);
                real32 cosOrbit = (real32)cos(shipSharedAttackInitialOrbitAngle);
                real32 baseX = runtime->state00;
                real32 baseY = runtime->state04;

                runtime->state08 = runtime->state20;
                runtime->state0C = shipSharedAttackInitialOrbitAngle;
                runtime->state00 = cosOrbit * baseX + sinOrbit * baseY;
                runtime->state04 = cosOrbit * baseY - sinOrbit * baseX;
            }

            destination.x = target->posinfo.position.x + runtime->state00;
            destination.y = target->posinfo.position.y + runtime->state04;
            destination.z = target->posinfo.position.z + runtime->state08;
            vecSub(tempVector, ship->posinfo.position, destination);
            if (vecMagnitudeSquared(tempVector) >= 5625.0f)
            {
                aishipFlyToPointAvoidingObjsFunc(ship, &destination, 0x16, -400.0f, &target->posinfo.velocity);
                return TRUE;
            }

            if ((ranRandom(10) & 0xF) != 0)
            {
                runtime->state0C += 0.1745329350233078f;
                if (runtime->state0C >= TWOPI)
                {
                    runtime->state0C -= TWOPI;
                }
                sinAngle = (real32)sin(runtime->state0C);
                cosAngle = (real32)cos(runtime->state0C);
                {
                    real32 baseX = runtime->state08;
                    real32 baseY = runtime->state04;
                    runtime->state00 = cosAngle * baseX + sinAngle * baseY;
                    runtime->state04 = cosAngle * baseY - sinAngle * baseX;
                }
                return TRUE;
            }

            ship->rcstate2 = STATE_BOMBING_DIVE;
            runtime->state00 = -runtime->state00;
            runtime->state04 = -runtime->state04;
            runtime->state20 = -200.0f;
            runtime->state10Count = 15;
            return TRUE;

        case STATE_BOMBING_DIVE:
            destination.x = target->posinfo.position.x + runtime->state00;
            destination.y = target->posinfo.position.y + runtime->state04;
            destination.z = target->posinfo.position.z + runtime->state20;
            vecSub(tempVector, ship->posinfo.position, destination);
            if (vecMagnitudeSquared(tempVector) >= 5625.0f)
            {
                aishipFlyToPointAvoidingObjsFunc(ship, &destination, 0x16, -400.0f, &target->posinfo.velocity);
            }
            else
            {
                ship->rcstate2 = STATE_INIT;
            }

            if (runtime->state10Count != 0)
            {
                runtime->state10Count--;
                return TRUE;
            }

            if (shipCanFireNextGunAtTarget(ship, target))
            {
                staticinfo->custshipheader.CustShipAttack(ship, target, 0.0f);
            }
            return TRUE;

        default:
            return TRUE;
    }
}

static void shipNamesReadScriptEntry(char *directory, char *field, void *dataToFillIn)
{
    ShipType shipType;
    char shipTypeName[256];
    char shipName[256];
    ShipNameReservationEntry *entry;
    (void)directory;
    (void)dataToFillIn;

    sscanf(field, "%255s %255s", shipTypeName, shipName);
    shipType = StrToShipType(shipTypeName);
    switch (shipType)
    {
        case 12:
        case 18:
        case 47:
        case 53:
        case 137:
            break;
        default:
            dbgFatalf(DBG_Loc,
                      "\nShip Type not valid for nameing:  %s.  Please remove from '%s'.",
                      shipTypeName, shipNamesScriptFile);
            break;
    }

    if (shipNameReservationCount >= SHIP_NAMES_MAX)
    {
        dbgFatalf(DBG_Loc,
                  "\nTo many ship names in '%s'!  Make MAX_SHIP_NAMES bigger (ShipNames.c).",
                  shipNamesScriptFile);
        return;
    }

    entry = &shipNameReservationStorage[shipNameReservationCount];
    entry->ship = 0;
    entry->unk04 = 0;
    entry->shipType = shipType;
    strcpy(entry->name, shipName);
    shipNameReservationCount++;
}

static scriptEntry shipNamesScriptTable[] =
{
    { "ShipName", shipNamesReadScriptEntry, NULL },
    endEntry
};

static bool32 shipNameReservationApplies(Ship *ship)
{
    if (ship == NULL || ship->playerowner == NULL)
    {
        return FALSE;
    }

    if (multiPlayerGame)
    {
        if (ship->playerowner->playerIndex != sigsPlayerIndex)
        {
            return FALSE;
        }
    }
    else if (ship->playerowner->playerIndex != 0)
    {
        return FALSE;
    }

    switch (ship->shiptype)
    {
        case 12:
        case 18:
        case 47:
        case 53:
        case 137:
            return TRUE;
        default:
            return FALSE;
    }
}

static void shipNameReservationClear(Ship *ship)
{
    sdword i;

    if (shipNameReservations == NULL)
    {
        return;
    }

    for (i = 0; i < shipNameReservationCount; i++)
    {
        if (shipNameReservations[i].ship == (udword)ship)
        {
            shipNameReservations[i].ship = 0;
            return;
        }
    }
}

sdword shipNamesStartup(void)
{
    shipNameReservations = shipNameReservationStorage;
    shipNameReservationCount = 0;
    memset(shipNameReservationStorage, 0, sizeof(shipNameReservationStorage));

    scriptSet(NULL, shipNamesScriptFile, shipNamesScriptTable);
    if (shipNamesCountFreeByType(18) < SHIP_NAMES_SOMTAAW_MOTHERSHIP_MIN)
    {
        dbgFatalf(DBG_Loc,
                  "\nNot enough names for sMothership in '%s'!  Need %d names, only %d found.",
                  shipNamesScriptFile, SHIP_NAMES_SOMTAAW_MOTHERSHIP_MIN, shipNamesCountFreeByType(18));
    }
    if (shipNamesCountFreeByType(53) < SHIP_NAMES_BEAST_MOTHERSHIP_MIN)
    {
        dbgFatalf(DBG_Loc,
                  "\nNot enough names for bMothership in '%s'!  Need %d names, only %d found.",
                  shipNamesScriptFile, SHIP_NAMES_BEAST_MOTHERSHIP_MIN, shipNamesCountFreeByType(53));
    }
    if (shipNamesCountFreeByType(12) < SHIP_NAMES_SOMTAAW_CARRIER_MIN)
    {
        dbgFatalf(DBG_Loc,
                  "\nNot enough names for sCarrier in '%s'!  Need %d names, only %d found.",
                  shipNamesScriptFile, SHIP_NAMES_SOMTAAW_CARRIER_MIN, shipNamesCountFreeByType(12));
    }
    if (shipNamesCountFreeByType(47) < SHIP_NAMES_BEAST_CARRIER_MIN)
    {
        dbgFatalf(DBG_Loc,
                  "\nNot enough names for bCarrier in '%s'!  Need %d names, only %d found.",
                  shipNamesScriptFile, SHIP_NAMES_BEAST_CARRIER_MIN, shipNamesCountFreeByType(47));
    }

    return shipNameReservationCount;
}

static real32 shipWaitingForShipsRadius(void)
{
    return shipWaitingForShipsRadiusBase + shipWaitingForShipsRadiusRange;
}

static real32 shipWaitingForShipsRandomCoord(real32 halfExtent)
{
    return ((real32)ranRandom(RAN_ETG) * (2.0f * halfExtent) * shipWaitingForShipsRandomScale) - halfExtent;
}

static void shipApplyNaggarokEMPShock(Ship *ship, real32 cappedDuration)
{
    AIVar *anyFSM;
    sdword *entry;
    real32 phase;

    if ((ship->specialFlags4 & SPECIAL4_NaggarokEMP) == 0)
        return;

    ship->disableStateElapsed += cappedDuration;
    anyFSM = aivarFindAnyFSM("G_NaggarokEMP");
    entry = aivarFindEntry(0, anyFSM);
    if (entry != NULL)
        phase = ship->disableStateElapsed / (real32)entry[2];
    else
        phase = cappedDuration;

    if (phase > 1.0f)
        phase = 1.0f;
    else if (phase < 0.0f)
        phase = 0.0f;

    phase = 1.0f - phase;
    {
        real32 v13;
        real32 v14;
        ship->statusWindow.phaseScale = phase;
        v13 = phase + universe.phystimeelapsed;
        ship->statusWindow.chainHitCount = 1;
        ship->statusWindow.activeUntil = v13;
        v14 = v13 + 1.0f;
        ship->beastAttack.raw_4EC = TreatAsUdword(v14);
    }
}

static bool32 shipCataBusyStateWindowHasEntries(const Ship *ship)
{
    sdword i;

    for (i = 0; i < SHIP_CATA_BUSY_STATE_WORD_COUNT; i++)
    {
        if (ship->cataBusyStateWindow.words[i] != 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

bool32 shipIsNaggarok(Ship *ship)
{
    return ship->objtype == OBJ_ShipType && ship->shiptype == dRKO;
}

void shipApplyEMPShock(Ship *ship, sdword duration)
{
    real32 cappedDuration;
    real32 disableDuration;
    udword flags;

    cappedDuration = (duration > 10) ? 10.0f : (real32)duration;
    if (TreatAsReal32(ship->beastAttack.raw_4EC) != 0.0f)
        return;

    if (!singlePlayerGame &&
        ship->command != NULL &&
        ship->command->ordertype.order == COMMAND_MP_HYPERSPACING)
    {
        return;
    }

    if (ship->staticinfo->TimeShockedByEMP <= 0.0f)
    {
        shipApplyNaggarokEMPShock(ship, cappedDuration);
        return;
    }

    if (!ship->disableStateStarted)
    {
        ship->disableStateStarted = TRUE;

        if (shipIsBeastRace(ship))
        {
            *(ubyte *)ship->ShipSpecifics = 1;
        }

        workerCheckResetForHyperspace(ship);
        SalCapNotifyProcessingShipRemoved(ship);
    }

    disableDuration = cappedDuration * ship->staticinfo->TimeShockedByEMP;
    if (ship->disableStateActive)
    {
        TimerSetRemaining(&ship->disableStateTimer, disableDuration);
        ship->disableStateElapsed += cappedDuration;
        return;
    }

    ship->disableStateActive = TRUE;
    ship->cataBusyStateWindow.words[SHIP_CATA_BUSY_STATE_DISABLE_LATCH_INDEX] = TRUE;

    flags = ship->flags;
    ship->flags = flags | 0x00A00000;
    ship->disableStateSavedFlag23 = (flags >> 23) & 1;
    ship->disableStateSavedFlag21 = (flags >> 21) & 1;
    ship->disableStateSavedAutostabilize = ship->autostabilizeship;
    TimerInitTimerStruct(&ship->disableStateTimer, disableDuration);
    ApplyCareenRotationDirectly(ship);
    ship->disableStateElapsed += cappedDuration;
}

void shipSetDelayedDeath(Ship *ship, real32 duration)
{
    udword flags;

    if (ship->disableStateActive)
    {
        TimerSetRemaining(&ship->disableStateTimer, duration);
        return;
    }

    ship->disableStateActive = TRUE;
    ship->cataBusyStateWindow.words[SHIP_CATA_BUSY_STATE_DISABLE_LATCH_INDEX] = TRUE;

    flags = ship->flags;
    ship->flags = flags | 0x00A00000;
    ship->disableStateSavedFlag23 = (flags >> 23) & 1;
    ship->disableStateSavedFlag21 = (flags >> 21) & 1;
    ship->disableStateSavedAutostabilize = ship->autostabilizeship;
    TimerInitTimerStruct(&ship->disableStateTimer, duration);
    ApplyCareenRotationDirectly(ship);
}

void shipClearDelayedDeath(Ship *ship)
{
    sdword restoredFlags;
    bool32 zeroVelocityX;

    if (!ship->disableStateActive)
    {
        return;
    }

    ship->cataBusyStateWindow.words[SHIP_CATA_BUSY_STATE_DISABLE_LATCH_INDEX] = FALSE;

    zeroVelocityX = (ship->posinfo.velocity.x == 0.0f);
    restoredFlags = (ship->disableStateSavedFlag21 & 1) | (4 * (ship->disableStateSavedFlag23 & 1));
    ship->flags = (ship->flags & 0xFF5FFFFF) | (restoredFlags << 21);
    ship->autostabilizeship = (bool8)ship->disableStateSavedAutostabilize;
    if (!zeroVelocityX || ship->posinfo.velocity.y != 0.0f || ship->posinfo.velocity.z != 0.0f)
    {
        ship->autostabilizeship = TRUE;
    }

    ship->disableStateActive = FALSE;
    if (ship->disableStateStarted == TRUE)
    {
        ship->disableStateStarted = FALSE;
    }
}

void shipCreateDisabledEffect(SpaceObj *spaceobj)
{
    Ship *ship = (Ship *)spaceobj;
    etglod *lod;
    etgeffectstatic *stat;
    real32 nextTime;
    udword nextTimeBits;

    lod = etgSpecialPurposeEffectTable[EGT_CAUGHT_GRAVWELL];
    if (lod && ship->currentLOD < lod->nLevels)
        stat = lod->level[ship->currentLOD];
    else
        stat = NULL;

    if (univSpaceObjInRenderList(spaceobj) && stat)
    {
        if (!etgFrequencyExceeded(stat))
        {
            etgEffectCreate(stat,
                            ship,
                            &ship->posinfo.position,
                            &ship->posinfo.velocity,
                            (matrix *)&ship->rotinfo,
                            ship->magnitudeSquared,
                            120,
                            1);
            nextTimeBits = etgFRandom(shipDisabledEffectLifeLow, shipDisabledEffectLifeHigh);
            nextTime = TreatAsReal32(nextTimeBits);
            ship->gravwellTimeEffect = nextTime + universe.phystimeelapsed;
        }
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B7FD0
bool32 shipIsBeastRace(Ship *ship)
{
    return ship->shiprace == RACE_Beast;
}

void shipNameReserve(Ship *ship)
{
    sdword i;
    sdword freeIndex;

    if (!shipNameReservationApplies(ship) || shipNameReservations == NULL || shipNameReservationCount <= 0)
    {
        return;
    }

    freeIndex = -1;
    for (i = 0; i < shipNameReservationCount; i++)
    {
        if (shipNameReservations[i].shipType == (udword)ship->shiptype &&
            shipNameReservations[i].ship == 0)
        {
            freeIndex = i;
            break;
        }
    }

    if (freeIndex < 0)
    {
        return;
    }

    shipNameReservationClear(ship);
    shipNameReservations[freeIndex].ship = (udword)ship;
}

void shipNameRelease(Ship *ship)
{
    if (!shipNameReservationApplies(ship))
    {
        return;
    }

    shipNameReservationClear(ship);
}

sdword shipNamesCountFreeByType(ShipType shipType)
{
    sdword i;
    sdword freeCount = 0;

    if (shipNameReservations == NULL)
    {
        return 0;
    }

    for (i = 0; i < shipNameReservationCount; i++)
    {
        if (shipNameReservations[i].shipType == (udword)shipType &&
            shipNameReservations[i].ship == 0)
        {
            freeCount++;
        }
    }
    return freeCount;
}

sdword shipNameReserveByIndex(Ship *ship, sdword freeIndex)
{
    sdword i;
    sdword currentFreeIndex = -1;

    if (ship == NULL || shipNameReservations == NULL || shipNameReservationCount <= 0)
    {
        return 0;
    }

    for (i = 0; i < shipNameReservationCount; i++)
    {
        if (shipNameReservations[i].shipType == (udword)ship->shiptype &&
            shipNameReservations[i].ship == 0)
        {
            currentFreeIndex++;
            if (currentFreeIndex == freeIndex)
            {
                shipNameReservationClear(ship);
                shipNameReservations[i].ship = (udword)ship;
                return sizeof(ShipNameReservationEntry) * i;
            }
        }
    }

    return sizeof(ShipNameReservationEntry) * i;
}

char *shipNamesGetFreeByTypeIndex(ShipType shipType, sdword freeIndex)
{
    sdword i;
    sdword currentFreeIndex = -1;

    if (shipNameReservations == NULL)
    {
        return NULL;
    }

    for (i = 0; i < shipNameReservationCount; i++)
    {
        if (shipNameReservations[i].shipType == (udword)shipType &&
            shipNameReservations[i].ship == 0)
        {
            currentFreeIndex++;
            if (currentFreeIndex == freeIndex)
            {
                return shipNameReservations[i].name;
            }
        }
    }

    return NULL;
}

char *shipNameGet(Ship *ship)
{
    sdword i;

    if (playPackets)
    {
        return shipNameReservationEmpty;
    }

    if (shipNameReservations != NULL)
    {
        for (i = 0; i < shipNameReservationCount; i++)
        {
            if (shipNameReservations[i].ship == (udword)ship)
            {
                return shipNameReservations[i].name;
            }
        }
    }

    return shipNameReservationEmpty;
}

bool32 shipCanHaveName(Ship *ship)
{
    return shipNameReservationApplies(ship);
}

udword shipSetDisabledState(Ship *ship, sdword disabled)
{
    udword result;

    if (disabled == TRUE)
    {
        if ((ship->specialFlags2 & SPECIAL_2_DisabledState) == 0)
        {
            shipRemoveSupportUsage(ship);
        }
    }
    else if ((ship->specialFlags2 & SPECIAL_2_DisabledState) != 0)
    {
        ship->specialFlags2 &= ~SPECIAL_2_DisabledState;
        shipAddSupportUsage(ship, ship->playerowner);
    }

    result = ship->specialFlags2 & ~SPECIAL_2_DisabledState;
    ship->specialFlags2 = result | (disabled ? SPECIAL_2_DisabledState : 0);
    return result;
}

bool32 shipGetWaitingForShipsPoints(Ship *ship, vector *fromPoint, vector *toPoint)
{
    /* Retail (IDA 0x5c56b0) reads the waiting-flag/points from a fixed scratch
       block at Ship+0x6C4, not from ship[1] pointer arithmetic. */
    ShipWaitingForShipsOverlay *overlay = shipWaitingForShipsOverlay(ship);

    if (overlay->waitingFlag != 1)
    {
        return FALSE;
    }

    *fromPoint = overlay->fromPoint;
    *toPoint = overlay->toPoint;
    return TRUE;
}

void shipStartWaitingForShips(Ship *ship, vector *position)
{
    /* Retail (IDA 0x5c5500) writes the waiting-flag/timer/points to a fixed
       scratch block at Ship+0x6C4, not to ship[1] pointer arithmetic. */
    ShipWaitingForShipsOverlay *overlay;
    sdword attempts;
    real32 safeDistanceSquared;
    vector randomPoint;

    if (ship == NULL)
    {
        return;
    }

    overlay = shipWaitingForShipsOverlay(ship);
    overlay->waitingFlag = 1;
    TimerInitTimerStruct(&overlay->timer, shipWaitingForShipsTimerSeconds);

    safeDistanceSquared = shipWaitingForShipsRadius();
    safeDistanceSquared *= safeDistanceSquared;
    overlay->safeDistanceSquared = safeDistanceSquared;
    overlay->fromPoint = *position;

    attempts = 0;
    do
    {
        randomPoint.x = shipWaitingForShipsRandomCoord(smUniverseSizeX);
        randomPoint.y = shipWaitingForShipsRandomCoord(smUniverseSizeY);
        randomPoint.z = shipWaitingForShipsRandomCoord(smUniverseSizeZ);
        attempts++;
    }
    while ((randomPoint.x - position->x) * (randomPoint.x - position->x) +
           (randomPoint.y - position->y) * (randomPoint.y - position->y) +
           (randomPoint.z - position->z) * (randomPoint.z - position->z) < safeDistanceSquared);

    overlay->toPoint = randomPoint;

    if (attempts > 1000)
    {
        telldir();
    }
}

bool32 shipWaitingForShipsDone(Ship *ship)
{
    /* Retail (IDA 0x5c5700) compares ship->posinfo.position against the
       fromPoint/safeDistanceSquared scratch block at Ship+0x6C4. */
    ShipWaitingForShipsOverlay *overlay = shipWaitingForShipsOverlay(ship);

    if (overlay->waitingFlag != 1)
    {
        return TRUE;
    }

    return (ship->posinfo.position.x - overlay->fromPoint.x) * (ship->posinfo.position.x - overlay->fromPoint.x) +
           (ship->posinfo.position.y - overlay->fromPoint.y) * (ship->posinfo.position.y - overlay->fromPoint.y) +
           (ship->posinfo.position.z - overlay->fromPoint.z) * (ship->posinfo.position.z - overlay->fromPoint.z) >
           overlay->safeDistanceSquared;
}

bool32 shipWaitingForShipsActive(Ship *ship)
{
    /* Retail (IDA disasm) reads the waiting-flag from Ship+0x6C4. */
    return shipWaitingForShipsOverlay(ship)->waitingFlag == 1;
}

sdword shipGetModuleSlotTimer(Ship *ship)
{
    return (ship != NULL) ? (sdword)ship->ShipSpecifics[3] : 0;
}

void shipDecrementModuleSlotTimer(Ship *ship)
{
    if (ship != NULL)
    {
        --ship->ShipSpecifics[3];
    }
}

sdword shipGetModuleCount(Ship *ship)
{
    sdword count = 0;

    if (ship != NULL)
    {
        count = (ship->ShipSpecifics[0] != 0);
        if (ship->ShipSpecifics[1] != 0)
        {
            ++count;
        }
    }
    return count;
}

sdword shipCanStartModuleSlotTimer(Ship *ship)
{
    return (ship != NULL &&
            shipGetModuleCount(ship) > 0 &&
            *((ubyte *)&ship->ShipSpecifics[2]) == 0);
}

sdword shipTryStartModuleSlotTimer(Ship *ship)
{
    if (!shipCanStartModuleSlotTimer(ship))
    {
        return FALSE;
    }

    TimerInitTimerStruct((Timer *)&ship->ShipSpecifics[3], nisIsRunning_ == 1 ? 5.0f : 6.0f);
    *((ubyte *)&ship->ShipSpecifics[2]) = TRUE;
    return TRUE;
}
