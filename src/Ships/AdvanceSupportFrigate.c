/*=============================================================================
    Name    : AdvanceSupportFrigate.c
    Purpose : Specifics for the AdvanceSupportFrigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "AdvanceSupportFrigate.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "Dreadnought.h"
#include "shipselect.h"
#include "aiship.h"
#include "collision.h"
#include "dock.h"
#include "universe.h"
#include "commandlayer.h"
#include "RepairDroid.h"


typedef struct
{
    real32 asfGunRange[NUM_TACTICS_TYPES];
    real32 asfTooCloseRange[NUM_TACTICS_TYPES];
    real32 repairApproachDistance;
} AdvanceSupportFrigateStatics;

AdvanceSupportFrigateStatics    AdvanceSupportFrigateStatic;

AdvanceSupportFrigateStatics AdvanceSupportFrigateStaticRace1;
AdvanceSupportFrigateStatics AdvanceSupportFrigateStaticRace2;

scriptStructEntry ASFStaticScriptTable[] =
{
    { "repairApproachDistance",    scriptSetReal32CB, (udword) &(AdvanceSupportFrigateStatic.repairApproachDistance), (udword) &(AdvanceSupportFrigateStatic) },

    { NULL,NULL,0,0 }
};


// Decompiled from cataclysm.exe @ 0x5AEF00
void AdvanceSupportFrigateStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    AdvanceSupportFrigateStatics *asfstat;

    if (statinfo->shiprace == RACE_Kushan)
    {
        asfstat = &AdvanceSupportFrigateStaticRace1;
    }
    else if (statinfo->shiprace == RACE_Taiidan)
    {
        asfstat = &AdvanceSupportFrigateStaticRace2;
    }
    else
    {
        asfstat = (AdvanceSupportFrigateStatics *)statinfo;
    }

    statinfo->custstatinfo = asfstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        asfstat->asfGunRange[i] = statinfo->bulletRange[i];
        asfstat->asfTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    scriptSetStruct(directory,filename,ASFStaticScriptTable,(ubyte *)asfstat);

}

void AdvanceSupportFrigateAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    AdvanceSupportFrigateStatics *asfstat = (AdvanceSupportFrigateStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,asfstat->asfGunRange[ship->tacticstype],asfstat->asfTooCloseRange[ship->tacticstype]);
    (void)maxdist;
}

void AdvanceSupportFrigateAttackPassive(Ship *ship,Ship *target,bool32 rotate)
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

// Decompiled from cataclysm.exe @ 0x5AEF70
bool32 AdvanceSupportFrigateSpecialTarget(Ship *ship,void *custom)
{
    AdvanceSupportFrigateStatics *asfstat = (AdvanceSupportFrigateStatics *)ship->staticinfo->custstatinfo;
    SelectAnyCommand *targets = (SelectAnyCommand *)custom;

    return repairDroidAIUpdate(ship, targets, asfstat->repairApproachDistance);
}

// Decompiled from cataclysm.exe @ 0x5AEF90
void AdvanceSupportFrigateSpecialOrderCleanup(CommandToDo *command)
{
    if (command->formation.formationSpecificInfo != NULL)
    {
        clDeleteCommandEffect(command);
    }
}

CustShipHeader AdvanceSupportFrigateHeader =
{
    sizeof(AdvanceSupportFrigateSpec),
    AdvanceSupportFrigateStaticInit,
    NULL,
    NULL,
    NULL,
    shipShootAllGunsAtTarget,
    shipSharedRotateAndShoot,
    NULL,
    AdvanceSupportFrigateSpecialTarget,
    AdvanceSupportFrigateSpecialOrderCleanup,
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
