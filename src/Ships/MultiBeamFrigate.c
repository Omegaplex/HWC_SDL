/*=============================================================================
    Name    : MultiBeamFrigate.c
    Purpose : Retail callbacks for the Sect/Beast MultiBeam Frigate
=============================================================================*/

#include <string.h>

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "MultiBeamFrigate.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "aiship.h"
#include "aitrack.h"
#include "fastmath.h"
#include "collision.h"
#include "physics.h"
#include "randy.h"
#include "universe.h"
#include "univupdate.h"
#include "CommandLayer.h"

void seekdir(void);

enum
{
    MULTIBEAM_STATE_INIT = 0,
    MULTIBEAM_STATE_APPROACH = 1,
    MULTIBEAM_STATE_FIRE = 3
};

typedef struct
{
    real32 multiBeamRangeRatio;
    AttackSideStepParameters attackSideStepParameters;
} MultiBeamFrigateStatics;

static MultiBeamFrigateStatics multiBeamFrigateStaticsSect;
static MultiBeamFrigateStatics multiBeamFrigateStaticsBeast;

static scriptStructEntry MultiBeamFrigateScriptTable[] =
{
    { "MultiBeamRangeRatio", scriptSetReal32CB, (udword)&multiBeamFrigateStaticsSect.multiBeamRangeRatio, (udword)&multiBeamFrigateStaticsSect },
    { NULL, NULL, 0, 0 }
};

static MultiBeamFrigateSpec *multiBeamFrigateShipSpecifics(Ship *ship)
{
    return (MultiBeamFrigateSpec *)ship->ShipSpecifics;
}

static MultiBeamFrigateStatics *multiBeamFrigateStatics(ShipStaticInfo *statinfo)
{
    return (MultiBeamFrigateStatics *)statinfo->custstatinfo;
}

static real32 multiBeamFrigateAttackRange(Ship *ship, MultiBeamFrigateStatics *statics)
{
    return ship->cataclysmGunInfo->guns[0].gunstatic->bulletrange * statics->multiBeamRangeRatio * 0.75f;
}

static void MultiBeamFrigateStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    MultiBeamFrigateStatics *statics = (statinfo->shiprace == RACE_Beast) ? &multiBeamFrigateStaticsBeast : &multiBeamFrigateStaticsSect;

    memset(statics, 0, sizeof(*statics));
    scriptSetStruct(directory, filename, MultiBeamFrigateScriptTable, (ubyte *)statics);
    scriptSetStruct(directory, filename, AttackSideStepParametersScriptTable, (ubyte *)&statics->attackSideStepParameters);
    statinfo->custstatinfo = statics;
}

static void MultiBeamFrigateInit(Ship *ship)
{
    ship->aistateattack = 0;
    attackSideStepInit(&multiBeamFrigateShipSpecifics(ship)->attackSideStep);
}

static void MultiBeamFrigateFire(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    GunInfo *customDockInfo = ship->cataclysmGunInfo;
    MultiBeamFrigateStatics *statics = multiBeamFrigateStatics((ShipStaticInfo *)ship->staticinfo);
    SelectAnyCommand *nearbyTargets;
    Gun *selectedGun = NULL;
    SpaceObjRotImpTarg *selectedTarget = NULL;
    vector trajectory;
    real32 dist;
    real32 range;
    real32 searchRange;
    sdword i;

    (void)rotateFlags;

    dbgAssert(customDockInfo != NULL);
    dbgAssert(customDockInfo->numGuns > 0);
    dbgAssert(statics != NULL);

    aishipGetTrajectory(ship, target, &trajectory);
    dist = fsqrt(vecMagnitudeSquared(trajectory));
    range = RangeToTargetGivenDist(ship, target, dist);

    for (i = 0; i < customDockInfo->numGuns; i++)
    {
        Gun *gun = &customDockInfo->guns[i];

        if (!gunCanShootStaggered(ship, gun))
        {
            continue;
        }
        if (gun->gunstatic->bulletrange * statics->multiBeamRangeRatio > range &&
            gunOrientGimbleGun(ship, gun, target))
        {
            selectedGun = gun;
            selectedTarget = target;
            break;
        }
    }

    if (selectedTarget == NULL)
    {
        searchRange = customDockInfo->guns[0].gunstatic->bulletrange * statics->multiBeamRangeRatio;
        nearbyTargets = univFindNearbyTargets(ship, searchRange);
        if (nearbyTargets == NULL)
        {
            return;
        }

        for (i = 0; i < customDockInfo->numGuns && selectedTarget == NULL; i++)
        {
            Gun *gun = &customDockInfo->guns[i];
            sdword j;

            if (!gunCanShootStaggered(ship, gun))
            {
                continue;
            }

            for (j = 0; j < nearbyTargets->numTargets; j++)
            {
                SpaceObjRotImpTarg *candidate = nearbyTargets->TargetPtr[j];

                if (gunOrientGimbleGun(ship, gun, candidate))
                {
                    selectedGun = gun;
                    selectedTarget = candidate;
                    break;
                }
            }
        }
    }

    if (selectedGun == NULL || selectedTarget == NULL || univIsShipCloaked((SpaceObj *)selectedTarget))
    {
        return;
    }

    gunShootGunAtTarget(ship, selectedGun, selectedTarget);

    if (ship->aistateattack != 3 && target == selectedTarget)
    {
        if (ship->command != NULL &&
            (ship->command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING) == 0)
        {
            aitrackSteadyShip(ship);
        }
        ship->aistateattack = 3;
    }
}

static bool32 MultiBeamFrigateAttackState(Ship *ship, SpaceObjRotImpTarg *target)
{
    MultiBeamFrigateStatics *statics = multiBeamFrigateStatics((ShipStaticInfo *)ship->staticinfo);
    MultiBeamFrigateSpec *spec = multiBeamFrigateShipSpecifics(ship);
    real32 rangeThreshold;
    vector trajectory;

    dbgAssert(statics != NULL);
    dbgAssert(ship->cataclysmGunInfo != NULL);

    rangeThreshold = multiBeamFrigateAttackRange(ship, statics);

    switch (ship->aistateattack)
    {
        case MULTIBEAM_STATE_INIT:
            ship->aistateattack = MULTIBEAM_STATE_APPROACH;
        case MULTIBEAM_STATE_APPROACH:
            aishipGetTrajectory(ship, target, &trajectory);
            if (RangeToTarget(ship, target, &trajectory) <= rangeThreshold ||
                aishipFlyToPointAvoidingObjsFunc(
                    ship,
                    &target->posinfo.position,
                    AISHIP_PointInDirectionFlying | AISHIP_CarTurn,
                    -10.0f,
                    &target->posinfo.velocity) != AISHIP_FLY_OK)
            {
                aitrackSteadyShip(ship);
                ship->aistateattack = MULTIBEAM_STATE_FIRE;
                spec->attackSideStep.aidirection = ((ranRandom(10) & 3) == 0) ? 3 : 2;
            }
            MultiBeamFrigateFire(ship, target, 0);
            return TRUE;

        case MULTIBEAM_STATE_FIRE:
            aishipGetTrajectory(ship, target, &trajectory);
            physApplyForceToObj(
                (SpaceObj *)ship,
                ship->nonstatvars.thruststrength[spec->attackSideStep.aidirection],
                (uword)spec->attackSideStep.aidirection);
            aishipFlyToPointAvoidingObjsFunc(ship, NULL, 0, 0.0f, &ship->posinfo.velocity);
            MultiBeamFrigateFire(ship, target, 0);
            if (RangeToTarget(ship, target, &trajectory) > rangeThreshold)
            {
                ship->aistateattack = MULTIBEAM_STATE_APPROACH;
            }
            return TRUE;

        default:
            return TRUE;
    }
}

static void MultiBeamFrigateRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    (void)rotateFlags;
    MultiBeamFrigateFire(ship, target, 0);
}

static void MultiBeamFrigateHousekeep(Ship *ship)
{
    (void)ship;
    seekdir();
}

CustShipHeader MultiBeamFrigateHeader =
{
    sizeof(MultiBeamFrigateSpec),
    MultiBeamFrigateStaticInit,
    NULL,
    MultiBeamFrigateInit,
    NULL,
    MultiBeamFrigateFire,
    MultiBeamFrigateRotateAndShoot,
    NULL,
    NULL,
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
    MultiBeamFrigateAttackState
};
