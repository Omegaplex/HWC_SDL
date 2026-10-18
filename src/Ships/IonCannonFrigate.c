/*=============================================================================
    Name    : IonCannonFrigate.c
    Purpose : Specifics for the Standard Frigate

    Created 11/5/1997 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "IonCannonFrigate.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "Dreadnought.h"

typedef struct
{
    real32 frigateGunRange[NUM_TACTICS_TYPES];
    real32 frigateTooCloseRange[NUM_TACTICS_TYPES];
} IonCannonFrigateStatics;

IonCannonFrigateStatics IonCannonFrigateStaticRace1;
IonCannonFrigateStatics IonCannonFrigateStaticRace2;
IonCannonFrigateStatics IonCannonFrigateStaticRace3;

// Decompiled from cataclysm.exe @ 0x5B6820
void IonCannonFrigateStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    IonCannonFrigateStatics *frigstat;

    (void)directory;
    (void)filename;

    switch (statinfo->shiprace)
    {
        case RACE_Kushan:
            frigstat = &IonCannonFrigateStaticRace1;
            break;
        case RACE_Taiidan:
            frigstat = &IonCannonFrigateStaticRace2;
            break;
        case RACE_Derelict:
            frigstat = &IonCannonFrigateStaticRace3;
            break;
        default:
            frigstat = (IonCannonFrigateStatics *)statinfo;
            break;
    }

    statinfo->custstatinfo = frigstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        frigstat->frigateGunRange[i] = statinfo->bulletRange[i]*0.725f;
        frigstat->frigateTooCloseRange[i] = statinfo->minBulletRange[i] * 0.5f;
    }
}

void IonCannonFrigateAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    IonCannonFrigateStatics *frigstat = (IonCannonFrigateStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,frigstat->frigateGunRange[ship->tacticstype],frigstat->frigateTooCloseRange[ship->tacticstype]);
    (void)maxdist;
}

void IonCannonFrigateAttackPassive(Ship *ship,Ship *target,bool32 rotate)
{
    if ((rotate) & ((bool32)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship,target);
    }
    else
    {
        attackPassive(ship,target);
    }
}

CustShipHeader IonCannonFrigateHeader =
{
    sizeof(IonCannonFrigateSpec),
    IonCannonFrigateStaticInit,
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
