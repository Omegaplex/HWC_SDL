/*=============================================================================
    Name    : GenericDefender.c
    Purpose : Specifics for the Generic Defender

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "GenericDefender.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "shipselect.h"
#include "aiship.h"
#include "fastmath.h"
#include "gun.h"
#include "collision.h"
#include "aitrack.h"
#include "battle.h"
#include "Carrier.h"
#include "minelayercorvette.h"
#include "P1Mothership.h"
#include "RammingFrigate.h"
#include "soundevent.h"
#include "physics.h"
#include "GenericInterceptor.h"
#include "Tactics.h"
#include "Ship.h"
#include "Universe.h"
#include "UnivUpdate.h"

#ifdef gshaw
#define DEBUG_DEFENDER
#endif

typedef struct
{
    real32 gunRange[NUM_TACTICS_TYPES];
    real32 tooCloseRange[NUM_TACTICS_TYPES];
    real32 CIRCLE_RIGHT_VELOCITY;
    real32 CIRCLE_RIGHT_THRUST;
} GenericDefenderStatics;

GenericDefenderStatics LightDefenderStaticRace1;
GenericDefenderStatics LightDefenderStaticRace2;

GenericDefenderStatics HeavyDefenderStaticRace1;
GenericDefenderStatics HeavyDefenderStaticRace2;

scriptStructEntry DefenderStaticScriptTable[] =
{
    { "CIRCLE_RIGHT_VELOCITY",    scriptSetReal32CB, (udword)&(HeavyDefenderStaticRace1.CIRCLE_RIGHT_VELOCITY), (udword)&(HeavyDefenderStaticRace1) },
    { "CIRCLE_RIGHT_THRUST",      scriptSetReal32CB, (udword)&(HeavyDefenderStaticRace1.CIRCLE_RIGHT_THRUST), (udword)&(HeavyDefenderStaticRace1) },
    { NULL, NULL, 0, 0 }
};

void GenericDefenderStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    udword i;
    GenericDefenderStatics *defenderstat;

    switch (statinfo->shiptype)
    {
        case kHeavyDefender:
            defenderstat = &HeavyDefenderStaticRace1;
            break;
        case tHeavyDefender:
            defenderstat = &HeavyDefenderStaticRace2;
            break;
        default:
            dbgAssert(FALSE);
            defenderstat = &HeavyDefenderStaticRace1;
    }

    statinfo->custstatinfo = defenderstat;

    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        defenderstat->gunRange[i] = statinfo->bulletRange[i];
        defenderstat->tooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }

    defenderstat->CIRCLE_RIGHT_VELOCITY = 20.0f;
    defenderstat->CIRCLE_RIGHT_THRUST = 0.1f;

    scriptSetStruct(directory, filename, DefenderStaticScriptTable, (ubyte *)defenderstat);
}

#define DEFENDER_APPROACH        1
#define DEFENDER_CIRCLE          2

static bool32 GenericDefenderAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    GenericDefenderStatics *defenderstat = (GenericDefenderStatics *)shipstaticinfo->custstatinfo;
    real32 newrange;

    vector trajectory;
    real32 dist;
    real32 range;
    real32 temp;
    real32 rightvelocity;
    vector right;

    newrange = (defenderstat->gunRange[ship->tacticstype] * 2) - maxdist;
    if (newrange < 0.0f)
    {
        newrange = defenderstat->tooCloseRange[ship->tacticstype] + 100.0f;
    }
    else if (newrange > defenderstat->gunRange[ship->tacticstype])
    {
        newrange = defenderstat->gunRange[ship->tacticstype];
    }

    switch (ship->aistateattack)
    {
        case 0:
            ship->aistateattack = DEFENDER_APPROACH;

        case DEFENDER_APPROACH:
            aishipGetTrajectory(ship, target, &trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship, target, dist);
            if (range > newrange)
            {
                aishipFlyToShipAvoidingObjsWithVel(ship, target, AISHIP_FastAsPossible | AISHIP_PointInDirectionFlying, 0.0f, &target->posinfo.velocity);
                break;
            }
#ifdef DEBUG_DEFENDER
            dbgMessagef("\n%x Changing to circle", ship);
#endif
            ship->aistateattack = DEFENDER_CIRCLE;
            goto circleAlreadyCalculatedTrajectory;

        case DEFENDER_CIRCLE:
            aishipGetTrajectory(ship, target, &trajectory);
            dist = fsqrt(vecMagnitudeSquared(trajectory));
            range = RangeToTargetGivenDist(ship, target, dist);
            if (range > newrange)
            {
#ifdef DEBUG_DEFENDER
                dbgMessagef("\n%x Changing back to approach", ship);
#endif
                ship->aistateattack = DEFENDER_APPROACH;
                break;
            }
circleAlreadyCalculatedTrajectory:

            vecDivideByScalar(trajectory, dist, temp);

            aitrackHeading(ship, &trajectory, FLYSHIP_ATTACKACCURACY);

            matGetVectFromMatrixCol2(right, ship->rotinfo.coordsys);
            rightvelocity = vecDotProduct(right, ship->posinfo.velocity);

            if (rightvelocity < defenderstat->CIRCLE_RIGHT_VELOCITY)
            {
                physApplyForceToObj((SpaceObj *)ship, ship->nonstatvars.thruststrength[TRANS_RIGHT] * defenderstat->CIRCLE_RIGHT_THRUST, TRANS_RIGHT);
            }

            if (range < defenderstat->tooCloseRange[ship->tacticstype])
            {
                aitrackZeroForwardVelocity(ship);
            }

            if (ship->attackvars.multipleAttackTargets)
            {
                gunShootGunsAtMultipleTargets(ship);
            }
            else
            {
                gunShootGunsAtTarget(ship, target, range, &trajectory);
            }

            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    return TRUE;
}

void GenericDefenderAttackPassive(Ship *ship, Ship *target, bool32 rotate)
{
    if ((rotate) & ((bool32)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship, target);
    }
    else
    {
        attackPassive(ship, target);
    }
}

#define K_Start         0
#define K_Ram           1
#define K_Ram2          2
#define K_TurnAround    3
#define K_TurnAround2   4
#define K_Yell          5

#define kamikazeFlyByDist 1200.0f
static const real32 kamikazeImpactVelocityMinRange = 200.0f;
static const real32 kamikazeImpactVelocityMinSpeed = 100.0f;
static const real32 kamikazeImpactVelocityBoost = 1.3f;
static const real32 kamikazeImpactVelocityTargetSpeedCutoff = 50.0f;

static void shipUpdateKamikazeImpactVelocity(Ship *ship, Ship *target, real32 range)
{
    real32 shipMaxVelocity;
    real32 targetMaxVelocity;
    real32 shipSpeed;
    vector targetPosition;
    vector heading;
    vector impactVelocity;
    real32 desiredSpeed;

    if (target->objtype != OBJ_ShipType || (sdword)target->shiptype < 0 || (target->flags2 & SOF2_Unknown4) != 0)
    {
        return;
    }

    shipMaxVelocity = (real32)(shipGetMaxSpeed(ship) * tacticsGetShipsMaxVelocity(ship));
    targetMaxVelocity = (real32)(shipGetMaxSpeed(target) * tacticsGetShipsMaxVelocity(target));
    if (targetMaxVelocity > shipMaxVelocity)
    {
        return;
    }

    targetPosition = target->collInfo.collPosition;
    vecSub(heading, targetPosition, ship->posinfo.position);
    vecNormalize(&heading);

    shipSpeed = fsqrt(vecMagnitudeSquared(ship->posinfo.velocity));

    if (range < kamikazeImpactVelocityMinRange &&
        range > 0.0f &&
        target->staticinfo->staticheader.staticCollInfo.collspheresize < kamikazeImpactVelocityTargetSpeedCutoff)
    {
        impactVelocity = heading;
        desiredSpeed = kamikazeImpactVelocityMinSpeed;

        if (universe.phystimeelapsed > 0.0f)
        {
            desiredSpeed = GetDistance(&ship->posinfo.position, &targetPosition) / universe.phystimeelapsed;
            if (desiredSpeed < kamikazeImpactVelocityMinSpeed)
            {
                desiredSpeed = kamikazeImpactVelocityMinSpeed;
            }
        }

        vecScalarMultiply(impactVelocity, impactVelocity, desiredSpeed);
        vecAddTo(impactVelocity, target->posinfo.velocity);
        ship->posinfo.velocity = impactVelocity;
    }
    else
    {
        impactVelocity = heading;
        desiredSpeed = shipSpeed;

        if (desiredSpeed < 200.0f)
        {
            desiredSpeed = 200.0f;
        }
        if (shipMaxVelocity > desiredSpeed)
        {
            desiredSpeed *= kamikazeImpactVelocityBoost;
        }

        vecScalarMultiply(impactVelocity, impactVelocity, desiredSpeed);
        ship->posinfo.velocity = impactVelocity;
    }

    rammingFrigateBlendHeading((SpaceObjRotImp *)ship, &heading);
}

static void shipApplyKamikazeCollisionImpulse(Ship *ship, Ship *target)
{
    vector delta;
    real32 totalMass;
    real32 distance;

    if (!collApplyKamikazeCollisionDamage(ship, target))
    {
        return;
    }

    totalMass = ship->staticinfo->staticheader.mass + target->staticinfo->staticheader.mass;
    vecSub(delta, target->posinfo.position, ship->posinfo.position);
    distance = fsqrt(vecMagnitudeSquared(delta));
    univApplyCollisionImpulse((SpaceObjRotImpTarg *)ship, (SpaceObjRotImpTarg *)target, totalMass, &delta, distance);
}

void doKamikazeAttack(Ship *ship, SpaceObjRotImpTarg *target)
{
    real32 mag, dist, range, shipvel;
    vector destination, trajectory, heading, shipheading, EinsteinVelocity;
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;

    vecSub(trajectory, target->collInfo.collPosition, ship->posinfo.position);
    mag = vecMagnitudeSquared(trajectory);
    dist = fsqrt(mag);

    destination = trajectory;
    vecNormalize(&destination);
    heading = destination;

    mag = dist + 500.0f;
    vecScalarMultiply(destination, destination, mag);
    vecAddTo(destination, target->collInfo.collPosition);

    range = RangeToTargetGivenDist(ship, target, dist);

    switch (ship->kamikazeState)
    {
        case K_Start:
            bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE_CRAZY_FAST);
            if (range < 500.0f)
            {
                matGetVectFromMatrixCol3(shipheading, ship->rotinfo.coordsys);
                vecScalarMultiply(shipheading, shipheading, kamikazeFlyByDist);
                vecAdd(ship->kamikazeVector, shipheading, target->collInfo.collPosition);
                ship->kamikazeState = K_TurnAround;
            }
            ship->kamikazeState = K_Ram;

            break;

        case K_Ram:
            if (range < 400.0f)
            {
                ship->kamikazeState = K_Yell;
            }
        case K_Ram2:
            vecSub(EinsteinVelocity, ship->posinfo.velocity, target->posinfo.velocity);
            shipvel = fsqrt(vecMagnitudeSquared(EinsteinVelocity));
            if (ship->shiptype == tMinelayerCorvette)
            {
                ((MinelayerCorvetteSpec *)(ship->ShipSpecifics))->mineaistate = MINE_DROP_ATTACK;
            }

            if (isShipStaticInterceptor(shipstaticinfo))
            {
                if (range < shipstaticinfo->bulletRange[ship->tacticstype])
                {
                    GenericInterceptorStatics *interceptorstat = (GenericInterceptorStatics *)shipstaticinfo->custstatinfo;
                    uword targetIndex;

                    if (target->objtype == OBJ_ShipType)
                    {
                        targetIndex = (uword)((ShipStaticInfo *)target->staticinfo)->shipclass;
                    }
                    else
                    {
                        targetIndex = (uword)NUM_CLASSES;
                    }

                    if (GenericInterceptorCanFire(ship, target, &trajectory, interceptorstat->triggerHappy[ship->tacticstype][targetIndex]))
                    {
                        ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipRotateAndShoot(ship, target, TRUE);
                    }
                }
            }
            else
            {
                gunShootGunsAtTarget(ship, target, range, &trajectory);
            }
            aishipFlyToPointAvoidingObjs(ship, &destination, AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying | AISHIP_FastAsPossible, 0.0f);
            if (range < 1500.0f)
            {
                bitSet(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE_CRAZY_FAST);
            }
            else
            {
                bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE_CRAZY_FAST);
            }
            if (range < kamikazeImpactVelocityMinRange)
            {
                shipUpdateKamikazeImpactVelocity(ship, (Ship *)target, range);
            }
            if (range < 1.0f)
            {
                Ship *targetShip = (Ship *)target;

                if (ship->collMyBlob != targetShip->collMyBlob ||
                    ship->collMyBlob == NULL ||
                    targetShip->collMyBlob == NULL)
                {
                    if (collCheckBBoxBumpCollision((SpaceObjRotImp *)ship, (SpaceObjRotImp *)targetShip))
                    {
                        shipApplyKamikazeCollisionImpulse(ship, targetShip);
                    }
                }
            }
            break;
        case K_TurnAround:
            bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE_CRAZY_FAST);
            aishipFlyToPointAvoidingObjs(ship, &ship->kamikazeVector, AISHIP_PointInDirectionFlying | AISHIP_FirstPointInDirectionFlying | AISHIP_FastAsPossible, 0.0f);
            if (MoveReachedDestinationVariable(ship, &ship->kamikazeVector, 300.0f))
            {
                ship->kamikazeState = K_TurnAround2;
            }
            break;
        case K_TurnAround2:
            bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE_CRAZY_FAST);
            if (aitrackHeadingWithFlags(ship, &heading, 0.90f, AITRACKHEADING_IGNOREUPVEC))
            {
                ship->kamikazeState = K_Ram;
            }
            break;
        case K_Yell:
            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Kamikaze, ship, SOUND_EVENT_DEFAULT);
            ship->kamikazeState = K_Ram2;
            break;
    }
}

CustShipHeader GenericDefenderHeader =
{
    sizeof(GenericDefenderSpec),
    GenericDefenderStaticInit,
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
    GenericDefenderAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
