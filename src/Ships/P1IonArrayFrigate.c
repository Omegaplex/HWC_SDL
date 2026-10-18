/*=============================================================================
    Name    : ___P1_ION_ARRAY_FRIGATE_H.c
    Purpose : Specifics for the P1IonArrayFrigate

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "P1IonArrayFrigate.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "GenericInterceptor.h"
#include "Clipper.h"
#include "universe.h"

typedef struct
{
    real32 frigateGunRange[NUM_TACTICS_TYPES];
    real32 frigateTooCloseRange[NUM_TACTICS_TYPES];
} P1IonArrayFrigateStatics;

static P1IonArrayFrigateStatics P1IonArrayFrigateStaticBeast;
static P1IonArrayFrigateStatics P1IonArrayFrigateStaticP1;
static P1IonArrayFrigateStatics P1IonArrayFrigateStaticDerelict;

static P1IonArrayFrigateStatics *p1IonArrayFrigateStaticsForRace(ShipStaticInfo *statinfo)
{
    switch (statinfo->shiprace)
    {
        case RACE_Beast:
            return &P1IonArrayFrigateStaticBeast;
        case RACE_P1:
            return &P1IonArrayFrigateStaticP1;
        case RACE_Derelict:
            return &P1IonArrayFrigateStaticDerelict;
        default:
            return (P1IonArrayFrigateStatics *)statinfo;
    }
}

void P1IonArrayFrigateStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    udword i;
    P1IonArrayFrigateStatics *frigstat = p1IonArrayFrigateStaticsForRace(statinfo);

    (void)directory;
    (void)filename;

    statinfo->custstatinfo = frigstat;

    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        frigstat->frigateGunRange[i] = statinfo->bulletRange[i];
        frigstat->frigateTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

static void P1IonArrayFrigateAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P1IonArrayFrigateStatics *frigstat = (P1IonArrayFrigateStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship, target, frigstat->frigateGunRange[ship->tacticstype], frigstat->frigateTooCloseRange[ship->tacticstype]);
    (void)maxdist;
}

static void P1IonArrayFrigateInit(Ship *ship)
{
    shipInitCloakState(ship, (real32 *)ship->ShipSpecifics, 0.1f, 0.1f);
}

static void P1IonArrayFrigateRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    if (univIsShipCloaked((SpaceObj *)ship))
    {
        if (!singlePlayerGame)
        {
            shipToggleCloak(ship);
        }
        return;
    }

    if ((((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate & rotateFlags) != 0)
    {
        attackPassive(ship, (Ship *)target);
    }
    else
    {
        attackSimple(ship, target);
    }
}

static bool32 P1IonArrayFrigateAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    if (univIsShipCloaked((SpaceObj *)ship))
    {
        if (!singlePlayerGame)
        {
            shipToggleCloak(ship);
            return FALSE;
        }
        return TRUE;
    }

    P1IonArrayFrigateAttack(ship, target, maxdist);
    return TRUE;
}

CustShipHeader IonArrayFrigateHeader =
{
    sizeof(IonArrayFrigateSpec),
    P1IonArrayFrigateStaticInit,
    NULL,
    P1IonArrayFrigateInit,
    NULL,
    shipShootAllGunsAtTarget,
    P1IonArrayFrigateRotateAndShoot,
    CloakedShipSpecialActivate,
    NULL,
    NULL,
    CloakedShipHousekeep,
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
    P1IonArrayFrigateAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
