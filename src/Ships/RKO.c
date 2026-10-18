/*=============================================================================
    Name    : RKO.c
    Purpose : Cataclysm-only dRKO callbacks
=============================================================================*/

#include <math.h>
#include <string.h>
#include "types.h"
#include "debug.h"
#include "fastmath.h"
#include "spaceobj.h"
#include "shipselect.h"
#include "commandlayer.h"
#include "select.h"
#include "universe.h"
#include "Globals.h"
#include "Randy.h"
#include "Timer.h"
#include "Sensors.h"
#include "meshanim.h"
#include "matrix.h"
#include "vector.h"
#include "Battle.h"
#include "ACV.h"
#include "Clamp.h"
#include "dock.h"
#include "etg.h"
#include "linkedlist.h"
#include "Physics.h"
#include "RammingFrigate.h"
#include "render.h"
#include "SalCapCorvette.h"
#include "SaveGame.h"
#include "UnivUpdate.h"
#include "AIShip.h"
#include "AITrack.h"
#include "AIVar.h"
#include "ClassDefs.h"
#include "Ship.h"
#include "ShipDefs.h"
#include "gun.h"
#include "statscript.h"
#include "objtypes.h"

udword CrystalDestroyProcessingEffect(Ship *crystal);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
void telldir(void);
char repairDroidFlyToTarget(SpaceObjRotImp *ship, vector *targetPos);
BOOL repairDroidIsValidTarget(Ship *parentShip, Ship *target, sdword allowAllied);
void repairDroidAlignToTarget(SpaceObjRotImp *ship, float *a2);
extern real32 formationMoveMinimumDistance;

static const real32 rkoRandomScale = 1.0f / (real32)UDWORD_Max;
static const real32 rkoVelocityCheckSpeed = 100.0f;
static const real32 rkoFacingDotMin = 0.86602f;
static const real32 rkoVelocityDampenFactor = 0.5f;
static const real32 rkoAcquireRetryDelay = 2.0f;
static const real32 rkoTravelDurationScale = 5.0e-5f;
static const real32 rkoTravelHeadingAccuracy = 0.995f;
static const real32 rkoApproachHeadingAccuracy = 0.99900001f;
static const real32 rkoArrivalFreezeDuration = 500.0f;
static const real32 rkoPostConsumeScatterRadius = 20000.0f;
static const real32 rkoPostConsumeForwardOffset = 35000.0f;
static const real32 rkoPostConsumeShortDuration = 4.0f;
static const real32 rkoPostConsumeLongDuration = 65.0f;

typedef struct NaggarokShipSpecifics
{
    sdword   isReady;
    real32   chargeTimer;
    sdword   beamDispatchType;
    ubyte    _unk_0C[0x20];
    SpaceObj *unkRef;
    ubyte    _unk_30[4];
    /* eatBeamState/eatTarget/beamEffect/beamActive/beamApproachDist (0x34-0x47):
       these overlay offsets belong to the shared repairBeamAIUpdate state machine
       (0x5C89A0), which has exactly one caller in the retail binary: Worker's
       WorkerTargetSpecial. RKO/Naggarok never calls repairBeamAIUpdate or the
       salCap* helpers below despite RKOStaticInit pointing custstatinfo at the same
       data region, so these fields are dead weight here; the real logic lives in
       Worker.c (WorkerShipSpecifics unk_34/unk_38/unk_3C/unk_40/unk_44). Kept only
       because they cross-validate the confirmed offsets used there. */
    sdword   eatBeamState;
    Ship    *eatTarget;
    Effect  *beamEffect;
    sdword   beamActive;
    real32   beamApproachDist;
    ubyte    _unk_48[0x44];
    sdword   randomTargetSet;
    vector   watchTargetPos;
    real32   minWanderDistSq;
    vector   wanderPos;
    Timer    wanderTimer;
    ubyte    _unk_BC[0x278];
    sdword   moduleStateA;
    ubyte    _unk_338[8];
    sdword   moduleStateB;
    sdword   moduleStateC;
    ubyte    _unk_348[4];
    sdword   moduleInRange;
} NaggarokShipSpecifics;

static_assert(offsetof(NaggarokShipSpecifics, eatBeamState) == 0x34);
static_assert(offsetof(NaggarokShipSpecifics, eatTarget) == 0x38);
static_assert(offsetof(NaggarokShipSpecifics, beamEffect) == 0x3C);
static_assert(offsetof(NaggarokShipSpecifics, beamActive) == 0x40);
static_assert(offsetof(NaggarokShipSpecifics, beamApproachDist) == 0x44);
static_assert(offsetof(NaggarokShipSpecifics, randomTargetSet) == 0x8C);
static_assert(offsetof(NaggarokShipSpecifics, watchTargetPos) == 0x90);
static_assert(offsetof(NaggarokShipSpecifics, minWanderDistSq) == 0x9C);
static_assert(offsetof(NaggarokShipSpecifics, wanderPos) == 0xA0);
static_assert(offsetof(NaggarokShipSpecifics, wanderTimer) == 0xAC);
static_assert(offsetof(NaggarokShipSpecifics, moduleStateA) == 0x334);
static_assert(offsetof(NaggarokShipSpecifics, moduleStateB) == 0x340);
static_assert(offsetof(NaggarokShipSpecifics, moduleStateC) == 0x344);
static_assert(offsetof(NaggarokShipSpecifics, moduleInRange) == 0x34C);

static udword rkoCustStatics = 0xFFFFFFFF;     /* cataclysm.exe 0xA7A5FC */
static const real32 rkoProcessingDamageScale = 9999.0f; /* cataclysm.exe 0x72A6A4 */

typedef struct RKOCrystalProcessingAttachment
{
    ubyte  unk_00[0x58];
    vector position;
    ubyte  unk_64[0x34];
    matrix coordsys;
    ubyte  unk_BC[0x64];
    real32 *beamLength;
} RKOCrystalProcessingAttachment;

static RKOSpecifics *rkoShipSpecifics(Ship *ship)
{
    return (RKOSpecifics *)ship->ShipSpecifics;
}

static ubyte *rkoCrystalProcessedChunkCounter(Ship *crystal)
{
    /* Ship+0x518: byte counter for processed chunks (IDA: RKOHousekeep mov byte ptr [eax+518h]) */
    return (ubyte *)&crystal->unk_518;
}

static ubyte rkoCrystalProcessedChunkLimit(const Ship *crystal)
{
    /* ShipStaticInfo+0x454: per-ship-type crystal chunk limit (IDA: movsx eax, byte ptr [eax+454h]) */
    return shipStaticInfoCataclysmTail(crystal->staticinfo)->crystalProcessedChunkLimit;
}

static RKOCrystalProcessingAttachment *rkoCrystalProcessingAttachment(Ship *crystal)
{
    /* Ship+0x594 = crystalProcessingState.effect: reused as RKO processing attachment ptr */
    return (RKOCrystalProcessingAttachment *)crystal->crystalProcessingState.effect;
}

static bool32 rkoIsNaggarokEmpActive(const Ship *ship)
{
    AIVar *naggarokEmp;
    sdword *entry;

    if ((ship->specialFlags4 & SPECIAL4_NaggarokEMP) == 0)
    {
        return FALSE;
    }

    naggarokEmp = aivarFindAnyFSM("G_NaggarokEMP");
    entry = aivarFindEntry(0, naggarokEmp);
    return entry != NULL && (real32)entry[2] > ship->disableStateElapsed;
}

static real32 rkoTimerRemaining(Timer *timer)
{
    real32 currentTime = timer->enabled ? taskTimeElapsed : universe.totaltimeelapsed;
    return timer->duration - (currentTime - timer->startTime);
}

static void rkoBeginVelocityFreeze(Ship *ship, real32 duration, real32 maxDuration)
{
    if (univIsPositionInBounds((real32 *)&ship->posinfo.position) != 1)
    {
        return;
    }

    if (ship->velocityFreezeActive)
    {
        TimerSetRemaining(&ship->velocityFreezeTimer, duration);
        if (rkoTimerRemaining(&ship->velocityFreezeTimer) > maxDuration)
        {
            TimerInitTimerStruct(&ship->velocityFreezeTimer, maxDuration);
        }
    }
    else
    {
        ship->velocityFreezeInitialVelocity = ship->posinfo.velocity;
        ship->velocityFreezeActive = TRUE;
        TimerInitTimerStruct(&ship->velocityFreezeTimer, duration);
    }
}

static void rkoDampenVelocityIfReversing(Ship *ship)
{
    vector heading;
    vector velocity;
    real32 speedSquared;

    speedSquared = ship->posinfo.velocity.x * ship->posinfo.velocity.x +
                   ship->posinfo.velocity.y * ship->posinfo.velocity.y +
                   ship->posinfo.velocity.z * ship->posinfo.velocity.z;
    if (speedSquared <= rkoVelocityCheckSpeed * rkoVelocityCheckSpeed)
    {
        return;
    }

    heading.x = ship->rotinfo.coordsys.m13;
    heading.y = ship->rotinfo.coordsys.m23;
    heading.z = ship->rotinfo.coordsys.m33;
    velocity = ship->posinfo.velocity;
    vecNormalize(&velocity);
    if (velocity.x * heading.x + velocity.y * heading.y + velocity.z * heading.z > rkoFacingDotMin)
    {
        return;
    }

    ship->posinfo.velocity.x *= rkoVelocityDampenFactor;
    ship->posinfo.velocity.y *= rkoVelocityDampenFactor;
    ship->posinfo.velocity.z *= rkoVelocityDampenFactor;
}

static void rkoDamageTargetCrystal(Ship *ship)
{
    RKOSpecifics *spec;
    real32 damageTaken;

    spec = rkoShipSpecifics(ship);
    if (spec->state != 16 || spec->targetCrystal == NULL)
    {
        return;
    }

    damageTaken = spec->targetCrystal->staticinfo->maxhealth * rkoProcessingDamageScale;
    ApplyDamageToTarget(
        (SpaceObjRotImpTarg *)spec->targetCrystal,
        damageTaken,
        GS_LargeEnergyCannon,
        666,
        ship->playerowner->playerIndex);
}

static void rkoPickPostConsumeTarget(RKOSpecifics *spec)
{
    spec->postConsumeTarget.x = (real32)((double)ranRandom(RAN_ETG) * (smUniverseSizeX * 2.0f) * rkoRandomScale - smUniverseSizeX);
    spec->postConsumeTarget.y = (real32)((double)ranRandom(RAN_ETG) * (smUniverseSizeY * 2.0f) * rkoRandomScale - smUniverseSizeY);
    spec->postConsumeTarget.z = (real32)((double)ranRandom(RAN_ETG) * (smUniverseSizeZ * 2.0f) * rkoRandomScale - smUniverseSizeZ);
}

static void rkoBuildApproachGeometry(Ship *ship, RKOSpecifics *spec)
{
    real32 offsetDistance;

    spec->crystalPosition = spec->targetCrystal->posinfo.position;
    spec->approachPosition = spec->crystalPosition;
    spec->targetDelta.x = spec->crystalPosition.x - ship->posinfo.position.x;
    spec->targetDelta.y = spec->crystalPosition.y - ship->posinfo.position.y;
    spec->targetDelta.z = spec->crystalPosition.z - ship->posinfo.position.z;
    spec->targetDistance = fsqrt(spec->targetDelta.x * spec->targetDelta.x +
                                 spec->targetDelta.y * spec->targetDelta.y +
                                 spec->targetDelta.z * spec->targetDelta.z);
    spec->approachHeading = spec->targetDelta;
    spec->approachHeading.x /= spec->targetDistance;
    spec->approachHeading.y /= spec->targetDistance;
    spec->approachHeading.z /= spec->targetDistance;

    offsetDistance = -(ship->staticinfo->staticheader.staticCollInfo.collspheresize +
                       spec->targetCrystal->staticinfo->staticheader.staticCollInfo.collspheresize +
                       formationMoveMinimumDistance);
    spec->approachPosition.x += offsetDistance * spec->approachHeading.x;
    spec->approachPosition.y += offsetDistance * spec->approachHeading.y;
    spec->approachPosition.z += offsetDistance * spec->approachHeading.z;
}

static real32 svShutdown(void)
{
    return 50000.0f + 18000.0f;
}

void naggarokSetRandomTarget(vector *avoidPos)
{
    if (!singlePlayerGame)
        return;

    Ship *rko = NULL;
    Node *node = universe.ShipList.head;
    while (node)
    {
        Ship *s = (Ship *)node->structptr;
        if (s->shiptype == dRKO)
            rko = s;
        node = node->next;
    }
    if (!rko)
        return;

    NaggarokShipSpecifics *spec = (NaggarokShipSpecifics *)rko->ShipSpecifics;

    spec->randomTargetSet = 1;
    TimerInitTimerStruct(&spec->wanderTimer, 70.0f);

    real32 d = svShutdown();
    spec->minWanderDistSq = d * svShutdown();
    spec->watchTargetPos = *avoidPos;

    sdword found = 0;
    sdword iters = 0;
    do
    {
        real32 x = (real32)((double)ranRandom(RAN_ETG) * (smUniverseSizeX * 2.0f) * rkoRandomScale - smUniverseSizeX);
        real32 y = (real32)((double)ranRandom(RAN_ETG) * (smUniverseSizeY * 2.0f) * rkoRandomScale - smUniverseSizeY);
        real32 z = (real32)((double)ranRandom(RAN_ETG) * (smUniverseSizeZ * 2.0f) * rkoRandomScale - smUniverseSizeZ);
        real32 dx = x - avoidPos->x;
        real32 dy = y - avoidPos->y;
        real32 dz = z - avoidPos->z;
        real32 distSq = dx * dx + dy * dy + dz * dz;
        iters++;
        if (distSq > spec->minWanderDistSq)
        {
            spec->wanderPos.x = x;
            spec->wanderPos.y = y;
            spec->wanderPos.z = z;
            found = 1;
        }
    }
    while (!found);

    if (iters > 1000)
        telldir();
}

void RKOStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    (void)directory;
    (void)filename;
    statinfo->custstatinfo = &rkoCustStatics;
}

void RKOInit(Ship *ship)
{
    RKOSpecifics *spec = (RKOSpecifics *)ship->ShipSpecifics;

    memset(spec, 0, sizeof(*spec));
    spec->processingEffectDistanceLimit = 1000000.0f;
    spec->processingTargetBiasLimit = 1000000000000.0f;
}

void RKORemoveShipReferences(Ship *ship, Ship *crystal)
{
    RKOSpecifics *spec = (RKOSpecifics *)ship->ShipSpecifics;

    if (crystal == spec->targetCrystal)
    {
        CrystalDestroyProcessingEffect(crystal);
        spec->targetCrystal = NULL;
        spec->state = 0;
    }
}

void RKOClose(Ship *ship)
{
    RKOSpecifics *spec = (RKOSpecifics *)ship->ShipSpecifics;

    if (spec->state == 16 && spec->targetCrystal != NULL)
    {
        real32 damageTaken = spec->targetCrystal->staticinfo->maxhealth * rkoProcessingDamageScale;
        sdword playerIndex = ship->playerowner->playerIndex;

        ApplyDamageToTarget(
            (SpaceObjRotImpTarg *)spec->targetCrystal,
            damageTaken,
            0,
            666,
            playerIndex);
    }
}

void RKOPreFix(Ship *ship)
{
    RKOSpecifics *spec = (RKOSpecifics *)ship->ShipSpecifics;

    spec->targetCrystal = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->targetCrystal);
}

void RKOFix(Ship *ship)
{
    RKOSpecifics *spec = (RKOSpecifics *)ship->ShipSpecifics;

    spec->targetCrystal = SpaceObjRegistryGetShip((sdword)spec->targetCrystal);
}

// Decompiled from cataclysm.exe @ 0x5C4260
void RKOHousekeep(Ship *ship)
{
    RKOSpecifics *spec;
    CommandToDo *command;
    Node *node;
    sdword allowAllied;
    Ship *candidate;
    Ship *currentTarget;
    real32 bestScore;
    real32 score;
    vector offset;
    vector toApproach;
    vector processingTarget;
    DockStaticPoint *weaponDock;
    SelectCommand selection;
    real32 processingDuration;
    real32 progress;
    ClampInfo *clampInfo;
    Ship **clampedShips;
    sdword i;
    RKOCrystalProcessingAttachment *attachment;
    real32 distanceToAttachment;

    spec = rkoShipSpecifics(ship);

    rkoDampenVelocityIfReversing(ship);

    if (spec->cachedNaggarokEmpDuration == 0)
    {
        AIVar *naggarokEmpFSM;
        sdword *entry;

        naggarokEmpFSM = aivarFindAnyFSM("G_NaggarokEMP");
        entry = aivarFindEntry(0, naggarokEmpFSM);
        if (entry != NULL)
        {
            spec->cachedNaggarokEmpDuration = entry[2];
        }
    }

    if ((ship->specialFlags2 & 0x400000) != 0 &&
        ship->disableStateElapsed >= spec->cachedNaggarokEmpDuration)
    {
        rkoDamageTargetCrystal(ship);
        rkoDampenVelocityIfReversing(ship);
        return;
    }

    if (spec->state == 16 &&
        spec->arrivalDamageActive == 1 &&
        vecDistanceLessThanRadiusSq(&ship->posinfo.position, &spec->arrivalDamageTarget, spec->arrivalDamageRadiusSq))
    {
        rkoDamageTargetCrystal(ship);
    }

    if (spec->state != 4)
    {
        spec->headingTimerArmed = 0;
    }

    command = ship->command;
    if (command != NULL && command->ordertype.order != 0)
    {
        if (command->ordertype.order == 1)
        {
            rkoDampenVelocityIfReversing(ship);
            for (node = universe.ShipList.head; node != NULL; node = node->next)
            {
                (void)shipIsACV((Ship *)node->structptr);
            }
        }

        if (spec->state != 0)
        {
            rkoDamageTargetCrystal(ship);
            spec->state = 0;
            ship->autostabilizeship = 1;
        }
    }
    else if (!spec->postConsumeActive)
    {
        switch (spec->state)
        {
            case 0:
                spec->state = 1;
                /* fall through */

            case 1:
                if (universe.totaltimeelapsed < spec->reacquireTime)
                {
                    break;
                }

                spec->targetCrystal = NULL;
                spec->reacquireTime = universe.totaltimeelapsed + rkoAcquireRetryDelay;
                allowAllied = 0;
                {
                    AIVar *traitorFSM;
                    sdword *entry;

                    traitorFSM = aivarFindAnyFSM("G_Traitor");
                    entry = aivarFindEntry(0, traitorFSM);
                    if (entry != NULL)
                    {
                        allowAllied = (entry[2] == 1);
                    }
                }

                bestScore = -2.0f;
                for (node = universe.SpaceObjList.head; node != NULL; node = node->next)
                {
                    SpaceObj *obj;

                    obj = (SpaceObj *)node->structptr;
                    if (obj->objtype != OBJ_ShipType)
                    {
                        continue;
                    }

                    candidate = (Ship *)obj;
                    if (candidate == ship || !repairDroidIsValidTarget(ship, candidate, allowAllied))
                    {
                        continue;
                    }

                    offset.x = ship->posinfo.position.x - candidate->posinfo.position.x;
                    offset.y = ship->posinfo.position.y - candidate->posinfo.position.y;
                    offset.z = ship->posinfo.position.z - candidate->posinfo.position.z;
                    score = ship->posinfo.velocity.x * offset.x +
                            ship->posinfo.velocity.y * offset.y +
                            ship->posinfo.velocity.z * offset.z;
                    currentTarget = spec->targetCrystal;

                    if (currentTarget != NULL &&
                        ((candidate->tacticstype == 3 && currentTarget->tacticstype != 3) ||
                         currentTarget->shiprace == 12))
                    {
                        bestScore = score;
                        spec->targetCrystal = candidate;
                    }
                    else if (score > bestScore &&
                             (currentTarget == NULL ||
                              ((candidate->shiprace != 12 || currentTarget->shiprace == 12) &&
                               (candidate->tacticstype == 3 || currentTarget->tacticstype != 3))))
                    {
                        bestScore = score;
                        spec->targetCrystal = candidate;
                    }
                }

                if (spec->targetCrystal == NULL)
                {
                    break;
                }

                spec->state = 2;
                /* fall through */

            case 2:
                spec->state = 3;
                /* fall through */

            case 3:
                rkoBuildApproachGeometry(ship, spec);
                aitrackZeroVelocity(ship);
                spec->state = 4;
                /* fall through */

            case 4:
                ship->autostabilizeship = 0;
                ship->posinfo.velocity.x = 0.0f;
                ship->posinfo.velocity.y = 0.0f;
                ship->posinfo.velocity.z = 0.0f;
                if (spec->targetCrystal != NULL && !univIsPositionInBounds(&spec->targetCrystal->posinfo.position))
                {
                    spec->targetCrystal = NULL;
                    spec->state = 0;
                    break;
                }

                if (aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &spec->approachHeading, rkoTravelHeadingAccuracy, 0.0f, 0.0f, 0))
                {
                    spec->state = 5;
                }
                else
                {
                    if (spec->headingTimerArmed == 0)
                    {
                        spec->headingTimerArmed = 1;
                        TimerInitTimerStruct(&spec->headingTimer, 10.0f);
                    }
                    if (spec->headingTimerArmed == 1 && TimerExpired((int)&spec->headingTimer))
                    {
                        offset = spec->approachHeading;
                        vecNormalize(&offset);
                        rammingFrigateSetHeading((SpaceObjRotImp *)ship, &offset);
                        spec->state = 5;
                    }
                    break;
                }
                /* fall through */

            case 5:
                ship->autostabilizeship = 1;
                spec->state = 6;
                /* fall through */

            case 6:
                spec->phaseStartTime = universe.totaltimeelapsed;
                spec->phaseEndTime = spec->phaseStartTime + spec->targetDistance * rkoTravelDurationScale;
                spec->state = 7;
                /* fall through */

            case 7:
                ship->posinfo.velocity.x = 0.0f;
                ship->posinfo.velocity.y = 0.0f;
                ship->posinfo.velocity.z = 0.0f;
                ship->autostabilizeship = 0;
                if ((universe.totaltimeelapsed - spec->phaseStartTime) / (spec->phaseEndTime - spec->phaseStartTime) >= 1.0f)
                {
                    spec->state = 8;
                }
                else
                {
                    break;
                }
                /* fall through */

            case 8:
                ship->autostabilizeship = 1;
                spec->state = 9;
                /* fall through */

            case 9:
                ship->autostabilizeship = 0;
                spec->state = 10;
                /* fall through */

            case 10:
                ship->autostabilizeship = 0;
                for (node = universe.ShipList.head; node != NULL; node = node->next)
                {
                    (void)shipIsACV((Ship *)node->structptr);
                }

                if (spec->targetCrystal == NULL || !univIsPositionInBounds(&spec->targetCrystal->posinfo.position))
                {
                    spec->targetCrystal = NULL;
                    spec->state = 0;
                    break;
                }

                aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &spec->approachHeading, rkoApproachHeadingAccuracy, 0.0f, 0.0f, 0);
                toApproach.x = spec->approachPosition.x - ship->posinfo.position.x;
                toApproach.y = spec->approachPosition.y - ship->posinfo.position.y;
                toApproach.z = spec->approachPosition.z - ship->posinfo.position.z;
                if (spec->targetCrystal != NULL && rkoIsNaggarokEmpActive(spec->targetCrystal))
                {
                    spec->targetCrystal->posinfo.velocity.x *= rkoVelocityDampenFactor;
                    spec->targetCrystal->posinfo.velocity.y *= rkoVelocityDampenFactor;
                    spec->targetCrystal->posinfo.velocity.z *= rkoVelocityDampenFactor;
                    rkoBeginVelocityFreeze(spec->targetCrystal, rkoArrivalFreezeDuration, rkoArrivalFreezeDuration);
                }

                if (rkoArrivalFreezeDuration * rkoArrivalFreezeDuration <
                    toApproach.x * toApproach.x + toApproach.y * toApproach.y + toApproach.z * toApproach.z)
                {
                    vecCopyAndNormalize(&toApproach, &ship->posinfo.velocity);
                    ship->posinfo.velocity.x *= ship->staticinfo->staticheader.maxvelocity;
                    ship->posinfo.velocity.y *= ship->staticinfo->staticheader.maxvelocity;
                    ship->posinfo.velocity.z *= ship->staticinfo->staticheader.maxvelocity;
                }
                else
                {
                    if (toApproach.x * spec->targetDelta.x +
                        toApproach.y * spec->targetDelta.y +
                        toApproach.z * spec->targetDelta.z <= 0.0f)
                    {
                        posSetShipPosition(ship, &spec->approachPosition);
                    }
                    ship->posinfo.velocity.x = 0.0f;
                    ship->posinfo.velocity.y = 0.0f;
                    ship->posinfo.velocity.z = 0.0f;
                    spec->state = 11;
                }
                break;

            case 11:
                ship->autostabilizeship = 1;
                spec->state = 12;
                /* fall through */

            case 12:
                spec->state = 13;
                /* fall through */

            case 13:
                if (spec->targetCrystal == NULL || !univIsPositionInBounds(&spec->targetCrystal->posinfo.position))
                {
                    spec->targetCrystal = NULL;
                    spec->state = 0;
                    break;
                }
                if (!rkoIsNaggarokEmpActive(spec->targetCrystal))
                {
                    spec->state = 0;
                    break;
                }
                spec->state = 14;
                /* fall through */

            case 14:
                spec->state = 15;
                /* fall through */

            case 15:
                weaponDock = dockFindDockStaticPoint("Weapon", ship->staticinfo->dockStaticInfo);
                ship->autostabilizeship = 0;
                ship->rotinfo.rotspeed.x = 0.0f;
                ship->rotinfo.rotspeed.y = 0.0f;
                ship->rotinfo.rotspeed.z = 0.0f;
                clSetAttribute((sdword)spec->targetCrystal, 1, 11);
                selection.numShips = 1;
                selection.ShipPtr[0] = spec->targetCrystal;
                RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selection);
                matMultiplyMatByVec(&offset, &ship->rotinfo.coordsys, &weaponDock->headingdirection);
                offset.x += ship->posinfo.position.x;
                offset.y += ship->posinfo.position.y;
                offset.z += ship->posinfo.position.z;
                processingDuration = 30.0f;
                if (spec->targetCrystal->staticinfo->RKOEatMeTime > 0.0f)
                {
                    processingDuration = spec->targetCrystal->staticinfo->RKOEatMeTime;
                }
                CrystalStartProcessing(spec->targetCrystal, &offset, processingDuration);
                *rkoCrystalProcessedChunkCounter(spec->targetCrystal) = 0;
                ship->autostabilizeship = 0;
                spec->state = 16;
                clampInfo = spec->targetCrystal->clampInfo;
                if (clampInfo != NULL && clampInfo->selection != NULL)
                {
                    clampedShips = ((SelectCommand *)clampInfo->selection)->ShipPtr;
                    for (i = ((SelectCommand *)clampInfo->selection)->numShips - 1; i >= 0; i--)
                    {
                        workerCheckResetForHyperspace(clampedShips[i]);
                        if (shipIsBeastRace(clampedShips[i]))
                        {
                            *(ubyte *)clampedShips[i]->ShipSpecifics = 1;
                        }
                        SalCapNotifyProcessingShipRemoved(clampedShips[i]);
                    }
                }
                workerCheckResetForHyperspace(spec->targetCrystal);
                repairDroidAlignToTarget((SpaceObjRotImp *)ship, (float *)spec->targetCrystal);
                if (spec->targetCrystal->clampInfo == NULL || spec->targetCrystal->clampInfo->host == NULL)
                {
                    clampObjectSetup((SpaceObj *)spec->targetCrystal, (SpaceObj *)ship);
                }
                spec->postConsumeShortScatter = (spec->targetCrystal->tacticstype == 3);
                break;

            case 16:
                progress = CrystalGetProcessingRemainingFraction(spec->targetCrystal);
                if (progress > 1.0f)
                {
                    progress = 1.0f;
                }

                while ((real32)rkoCrystalProcessedChunkCounter(spec->targetCrystal)[0] <
                       (real32)rkoCrystalProcessedChunkLimit(spec->targetCrystal) * progress)
                {
                    rkoCrystalProcessedChunkCounter(spec->targetCrystal)[0]++;
                    spec->processedCrystalCount++;
                }

                if (CrystalGetProcessingRemainingFraction(spec->targetCrystal) >= 1.0f)
                {
                    spec->state = 17;
                    /* fall through */
                }
                else
                {
                    break;
                }

            case 17:
                univReallyDeleteThisShipRightNow(spec->targetCrystal);
                ship->autostabilizeship = 1;
                spec->state = 0;
                spec->postConsumeActive = 1;
                spec->postConsumeTarget.x = ship->posinfo.position.x + ship->rotinfo.coordsys.m13 * rkoPostConsumeForwardOffset;
                spec->postConsumeTarget.y = ship->posinfo.position.y + ship->rotinfo.coordsys.m23 * rkoPostConsumeForwardOffset;
                spec->postConsumeTarget.z = ship->posinfo.position.z + ship->rotinfo.coordsys.m33 * rkoPostConsumeForwardOffset;
                TimerInitTimerStruct(
                    &spec->postConsumeTimer,
                    spec->postConsumeShortScatter == 1 ? rkoPostConsumeShortDuration : rkoPostConsumeLongDuration);
                break;
        }
    }

    if (spec->targetCrystal != NULL)
    {
        attachment = rkoCrystalProcessingAttachment(spec->targetCrystal);
        if (attachment != NULL)
        {
            weaponDock = dockFindDockStaticPoint("Weapon", ship->staticinfo->dockStaticInfo);
            matMultiplyMatByVec(&processingTarget, &ship->rotinfo.coordsys, &weaponDock->headingdirection);
            processingTarget.x += ship->posinfo.position.x;
            processingTarget.y += ship->posinfo.position.y;
            processingTarget.z += ship->posinfo.position.z;

            offset.x = spec->targetCrystal->posinfo.position.x - processingTarget.x;
            offset.y = spec->targetCrystal->posinfo.position.y - processingTarget.y;
            offset.z = spec->targetCrystal->posinfo.position.z - processingTarget.z;
            vecNormalize(&offset);
            matCreateCoordSysFromHeading(&attachment->coordsys, &offset);
            attachment->position = processingTarget;
            distanceToAttachment = fsqrt(offset.x * offset.x + offset.y * offset.y + offset.z * offset.z);
            *attachment->beamLength = distanceToAttachment;
            CrystalUpdateProcessingTarget(spec->targetCrystal, &processingTarget);
        }
    }

    if (spec->postConsumeActive == 1)
    {
        if (TimerExpired((int)&spec->postConsumeTimer))
        {
            spec->postConsumeActive = 0;
        }
        else if (vecDistanceLessThanRadiusSq(
                     &ship->posinfo.position,
                     &spec->postConsumeTarget,
                     rkoPostConsumeScatterRadius * rkoPostConsumeScatterRadius))
        {
            rkoPickPostConsumeTarget(spec);
        }
        else
        {
            repairDroidFlyToTarget((SpaceObjRotImp *)ship, &spec->postConsumeTarget);
        }
    }

    if (spec->arrivalDamageActive == 1 && TimerExpired((int)&spec->arrivalDamageTimer))
    {
        spec->arrivalDamageActive = 0;
    }
}

#if 0  // Cat2: Live naggarokClearProjectileRef is static in shockwave.c; uses proper typed Ship* access instead of raw offsets (0x5C5780)
Node* naggarokClearProjectileRef()
{
    Node* result; // eax
    struct Node* structptr; // edx

    result = (Node*)singlePlayerGame;
    if (singlePlayerGame)
    {
        result = universe.ShipList.head;
        structptr = 0;
        if (universe.ShipList.head)
        {
            do
            {
                if (result[1].prev[86].next == (struct Node*)109)
                    structptr = result[1].prev;
                result = result->next;
            } while (result);
            if (structptr)
                structptr[216].prev = 0;
        }
    }
    return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== naggarokUpdate (0x5c6830) ===*/
void __cdecl naggarokUpdate(Ship* ship)
{
    Ship* v1; // ebp
    struct CommandToDo* command; // eax
    sdword v3; // edi
    uword attributes; // dx
    struct AttackCommand* attack; // ecx
    Ship* v6; // ebx
    struct SelectCommand* selection; // eax
    sdword v8; // ecx
    int v9; // edx
    struct CommandToDo* v10; // eax
    int numShips; // edx
    struct SelectCommand** p_selection; // eax
    int v13; // ebp
    int v14; // esi
    struct CommandToDo* v15; // eax
    sdword v16; // edx
    struct SelectCommand* v17; // eax
    int v18; // esi
    int v19; // ecx
    int v20; // ebx
    int v21; // edx
    int v22; // eax
    int v23; // esi
    bool v24; // cc
    int* v25; // eax
    int v26; // edx
    struct CommandToDo* v27; // eax
    struct SelectCommand* v28; // eax
    sdword v29; // ebx
    int v30; // esi
    Ship* v31; // eax
    struct Ship* dockingship; // ecx
    double v33; // st7
    double v34; // st7
    double v35; // st7
    int v36; // [esp+18h] [ebp-1Ch]
    float v37; // [esp+1Ch] [ebp-18h]
    float v38; // [esp+20h] [ebp-14h]
    float v39; // [esp+24h] [ebp-10h]
    float v40; // [esp+28h] [ebp-Ch] BYREF
    float v41; // [esp+2Ch] [ebp-8h]
    float v42; // [esp+30h] [ebp-4h]
    int shipa; // [esp+38h] [ebp+4h]

    v1 = ship;
    command = (struct CommandToDo*)ship->unk_390;
    v3 = 0;
    if (!command)
    {
    LABEL_65:
        ship->sizeofShipSpecifics = 0;
        clDetachShipFromCommand(ship);
        return;
    }
    attributes = HIWORD(command->move.heading.y);
    if ((attributes & 1) == 0
        || (attack = (struct AttackCommand*)LODWORD(command[2].formation.coordsys.m12), attack != (struct AttackCommand*)9)
        && attack != (struct AttackCommand*)10
        || *(Ship**)(LODWORD(command->move.heading.x) + 8) != ship)
    {
        if (LODWORD(command[2].formation.coordsys.m12) == 10)
            return;
        goto LABEL_65;
    }
    if (attack == (struct AttackCommand*)10)
    {
        if ((attributes & 2) != 0)
        {
            v6 = (Ship*)command[2].buildingship.creator->objtype;
        }
        else
        {
            LODWORD(command[2].formation.coordsys.m12) = 9;
            formationContentHasChanged((struct CommandToDo*)ship->unk_390);
            v6 = ship;
        }
        if (LODWORD(v6->formationOffset.x))
            goto LABEL_13;
        *(_DWORD*)(ship->unk_390 + 560) = 9;
        formationContentHasChanged((struct CommandToDo*)ship->unk_390);
    }
    v6 = ship;
LABEL_13:
    selection = *(struct SelectCommand**)(ship->unk_390 + 16);
    v8 = 0;
    if (selection->numShips > 0)
    {
        v9 = 8;
        do
        {
            *(_DWORD*)(*(sdword*)((char*)&selection->numShips + v9) + 1592) = 0;
            selection = *(struct SelectCommand**)(ship->unk_390 + 16);
            ++v8;
            v9 += 4;
        } while (v8 < selection->numShips);
    }
    v10 = (struct CommandToDo*)ship->unk_390;
    numShips = *(_DWORD*)LODWORD(v10->move.heading.x);
    p_selection = (struct SelectCommand**)&v10->move;
    if (numShips > 0)
    {
        v13 = 8;
        do
        {
            v14 = *(sdword*)((char*)&(*p_selection)->numShips + v13);
            v37 = *(float*)(v14 + 88) - v6->collInfo.rectpos[2].y;
            v38 = *(float*)(v14 + 92) - v6->collInfo.rectpos[2].z;
            v39 = *(float*)(v14 + 96) - v6->collInfo.rectpos[3].x;
            matMultiplyMatByVec((vector*)&v40, (matrix*)&v6->rotinfo.rotspeed.y, (vector*)(v14 + 612));
            *(_DWORD*)(v14 + 2436) = fabs(fmathSqrtDouble((v39 - v42) * (v39 - v42) + (v38 - v41) * (v38 - v41) + (v37 - v40) * (v37 - v40))) <= flt_8C67B8;
            *(_DWORD*)(v14 + 1592) = 0;
            v15 = (struct CommandToDo*)ship->unk_390;
            v16 = *(_DWORD*)LODWORD(v15->move.heading.x);
            p_selection = (struct SelectCommand**)&v15->move;
            ++v3;
            v13 += 4;
        } while (v3 < v16);
        v1 = ship;
    }
    v17 = *(struct SelectCommand**)(v1->unk_390 + 16);
    v18 = v17->numShips;
    v36 = 0;
    if (v17->numShips > 0)
    {
        shipa = 8;
        while (1)
        {
            v19 = *(sdword*)((char*)&v17->numShips + shipa);
            v20 = 1;
            if (*(_DWORD*)(v19 + 2436))
            {
                v21 = *(_DWORD*)(v19 + 1600);
                if (v21 > 0)
                {
                    v22 = v19 + 1608;
                    do
                    {
                        if (!*(_DWORD*)(*(_DWORD*)(v22 - 4) + 2436) || !*(_DWORD*)(*(_DWORD*)v22 + 2436))
                            v20 = 0;
                        v22 += 8;
                        --v21;
                    } while (v21);
                }
            }
            else
            {
                v20 = 0;
            }
            if (v18 <= 2)
                break;
            v23 = 0;
            if (*(_DWORD*)(v19 + 2424) || *(_BYTE*)(v19 + 1320) || *(_DWORD*)(v19 + 2428))
                goto LABEL_35;
            if (v20)
            {
                if (*(float*)(v19 + 1596) == ZERO)
                    *(_DWORD*)(v19 + 2412) = 1065353216;
                v33 = universe.unknown1 * flt_8C67C4 + *(float*)(v19 + 1596);
                *(float*)(v19 + 1596) = v33;
                if (v33 > dbl_72A548)
                {
                    v34 = *(float*)(v19 + 2412);
                    *(_DWORD*)(v19 + 1596) = 1065353216;
                    if (v34 > ZERO)
                    {
                        v35 = *(float*)(v19 + 2412) - flt_8C67C8 * universe.unknown1;
                        *(float*)(v19 + 2412) = v35;
                        if (v35 < ZERO)
                            *(_DWORD*)(v19 + 2412) = 0;
                    }
                }
                goto LABEL_37;
            }
        LABEL_36:
            *(_DWORD*)(v19 + 1596) = 0;
        LABEL_37:
            v24 = *(_DWORD*)(v19 + 1600) <= 0;
            *(_DWORD*)(v19 + 1592) = v20;
            if (!v24)
            {
                v25 = (int*)(v19 + 1608);
                do
                {
                    v26 = *(v25 - 1);
                    if (!*(_DWORD*)(v26 + 1592) || (v26 = *v25, !*(_DWORD*)(*v25 + 1592)))
                        *(_DWORD*)(v26 + 1592) = v20;
                    ++v23;
                    v25 += 2;
                } while (v23 < *(_DWORD*)(v19 + 1600));
            }
            v17 = *(struct SelectCommand**)(v1->unk_390 + 16);
            shipa += 4;
            v18 = v17->numShips;
            if (++v36 >= v17->numShips)
                goto LABEL_44;
        }
        v23 = 0;
    LABEL_35:
        v20 = 0;
        goto LABEL_36;
    }
LABEL_44:
    v27 = (struct CommandToDo*)v1->unk_390;
    if (LODWORD(v27[2].formation.coordsys.m12) == 10)
    {
        v28 = (struct SelectCommand*)LODWORD(v27->move.heading.x);
        v29 = 0;
        if (v28->numShips > 0)
        {
            v30 = 8;
            do
            {
                v31 = *(Ship**)((char*)&v28->numShips + v30);
                if (v31->sizeofShipSpecifics)
                {
                    dockingship = (struct Ship*)v31->salvageNumTagged[3];
                    if (!dockingship)
                        clampObjectSetup((struct SpaceObj*)v31, *(struct SpaceObj**)(*(_DWORD*)(v31->unk_390 + 692) + 8));
                }
                else
                {
                    clDetachShipFromCommand(v31);
                }
                v28 = *(struct SelectCommand**)(v1->unk_390 + 16);
                ++v29;
                v30 += 4;
            } while (v29 < v28->numShips);
        }
    }
}
#endif
