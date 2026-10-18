/*=============================================================================
    Name    : P1Mothership.c
    Purpose : Specifics for the P1Mothership

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "P1Mothership.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "dock.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "Dreadnought.h"
#include "HeavyCruiser.h"
#include "Mothership.h"

typedef struct
{
    real32 p1mothershipGunRange[NUM_TACTICS_TYPES];
    real32 p1mothershipTooCloseRange[NUM_TACTICS_TYPES];
} P1MothershipStatics;

P1MothershipStatics P1MothershipStatic;

void P1MothershipStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    P1MothershipStatics *mothershipstat = &P1MothershipStatic;

    statinfo->custstatinfo = mothershipstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        mothershipstat->p1mothershipGunRange[i] = statinfo->bulletRange[i];
        mothershipstat->p1mothershipTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void P1MothershipInit(Ship *ship)
{
    InitCrystalDockSlots(ship);
}

void P1MothershipRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    if ((ship->specialFlags3 & rotateFlags) != 0)
    {
        attackPassive(ship, (Ship *)target);
    }
    else
    {
        attackSimple(ship, target);
    }
}

void P1MothershipHousekeep(Ship *ship)
{
    ProcessCrystalDockSlots(ship);
}

void P1MothershipRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    RemoveShipFromCrystalDockSlots(ship, shiptoremove);
}

void P1MothershipClose(Ship *ship)
{
    ClearCrystalDockSlots(ship);
}

void P1Mothership_Fix(Ship *ship)
{
    Fix_CrystalDockSlots(ship);
}

CustShipHeader P1MothershipHeader =
{
    sizeof(P1MothershipSpec),
    P1MothershipStaticInit,
    NULL,
    P1MothershipInit,
    NULL,
    shipShootAllGunsAtTarget,
    P1MothershipRotateAndShoot,
    NULL,
    NULL,
    NULL,
    P1MothershipHousekeep,
    P1MothershipRemoveShipReferences,
    P1MothershipRemoveShipReferences,
    NULL,
    NULL,
    P1MothershipClose,
    Mothership_PreFix,
    NULL,
    NULL,
    P1Mothership_Fix,
    NULL,
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
