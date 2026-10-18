/*=============================================================================
    Name    : StandardFrigate.c
    Purpose : Specifics for the Standard Frigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "StandardFrigate.h"
#include "Carrier.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "Dreadnought.h"

typedef struct
{
    real32 frigateGunRange[NUM_TACTICS_TYPES];
    real32 frigateTooCloseRange[NUM_TACTICS_TYPES];
} StandardFrigateStatics;

StandardFrigateStatics StandardFrigateStaticRace1;
StandardFrigateStatics StandardFrigateStaticRace2;

// Decompiled from cataclysm.exe @ 0x5C78E0
void StandardFrigateStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    udword i;
    StandardFrigateStatics *frigstat = (statinfo->shiprace == RACE_Kushan) ? &StandardFrigateStaticRace1 : &StandardFrigateStaticRace2;

    statinfo->custstatinfo = frigstat;

    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        frigstat->frigateGunRange[i] = statinfo->bulletRange[i];
        frigstat->frigateTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void StandardFrigateAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    StandardFrigateStatics *frigstat = (StandardFrigateStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship, target, frigstat->frigateGunRange[ship->tacticstype], frigstat->frigateTooCloseRange[ship->tacticstype]);
    (void)maxdist;
}

void StandardFrigateAttackPassive(Ship *ship, Ship *target, bool32 rotate)
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

CustShipHeader StandardFrigateHeader =
{
    sizeof(StandardFrigateSpec),
    StandardFrigateStaticInit,
    NULL,
    NULL,
    NULL,
    shipShootAllGunsAtTarget,
    shipSharedRotateAndShoot,
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
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
