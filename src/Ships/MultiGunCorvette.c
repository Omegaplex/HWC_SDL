/*=============================================================================
    Name    : MultiGunCorvette.c
    Purpose : Specifics for the MultiGunCorvette

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "MultiGunCorvette.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "aiship.h"
#include "collision.h"
#include "RaceDefs.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "P1Mothership.h"
#include "Swarmer.h"

typedef struct
{
    AttackSideStepParameters sidestepParameters;
} MultiGunCorvetteStatics;

void MultiGunCorvetteAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
void MultiGunCorvetteAttackPassive(Ship *ship, Ship *target, bool32 rotate);

MultiGunCorvetteStatics MultiGunCorvetteStaticRace1;
MultiGunCorvetteStatics MultiGunCorvetteStaticRace2;
MultiGunCorvetteStatics MultiGunCorvetteStaticRace3;

void MultiGunCorvetteStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    MultiGunCorvetteStatics *corvstat;

    switch (statinfo->shiprace)
    {
        case RACE_Beast:
            corvstat = &MultiGunCorvetteStaticRace1;
            break;
        case RACE_Kushan:
            corvstat = &MultiGunCorvetteStaticRace2;
            break;
        case RACE_Taiidan:
            corvstat = &MultiGunCorvetteStaticRace3;
            break;
        default:
            dbgAssert(FALSE);
            corvstat = (MultiGunCorvetteStatics *)statinfo;
            break;
    }

    scriptSetStruct(directory, filename, AttackSideStepParametersScriptTable, (ubyte *)&corvstat->sidestepParameters);

    statinfo->custstatinfo = corvstat;
}

void MultiGunCorvetteInit(Ship *ship)
{
    MultiGunCorvetteSpec *spec = (MultiGunCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);
}

void MultiGunCorvetteAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    MultiGunCorvetteSpec *spec = (MultiGunCorvetteSpec *)ship->ShipSpecifics;
    MultiGunCorvetteStatics *corvstat = (MultiGunCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship, target, &spec->attacksidestep, &corvstat->sidestepParameters);
    (void)maxdist;
}

void MultiGunCorvetteAttackPassive(Ship *ship, Ship *target, bool32 rotate)
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

CustShipHeader MultiGunCorvetteHeader =
{
    sizeof(MultiGunCorvetteSpec),
    MultiGunCorvetteStaticInit,
    NULL,
    MultiGunCorvetteInit,
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
    SharedSideStepAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
