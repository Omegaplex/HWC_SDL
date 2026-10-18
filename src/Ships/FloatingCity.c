/*=============================================================================
    Name    : FloatingCity.c
    Purpose : Specifics for the FloatingCity

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "FloatingCity.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "Dreadnought.h"

void shipShootAllGunsAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
void P1MothershipRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);

typedef struct
{
    real32 floatingcityGunRange[NUM_TACTICS_TYPES];
    real32 floatingcityTooCloseRange[NUM_TACTICS_TYPES];
} FloatingCityStatics;

FloatingCityStatics FloatingCityStatic;

void FloatingCityStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    FloatingCityStatics *floatingcitystat = &FloatingCityStatic;

    statinfo->custstatinfo = floatingcitystat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        floatingcitystat->floatingcityGunRange[i] = statinfo->bulletRange[i];
        floatingcitystat->floatingcityTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

CustShipHeader FloatingCityHeader =
{
    sizeof(FloatingCitySpec),
    FloatingCityStaticInit,
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
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
