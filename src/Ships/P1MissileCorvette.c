/*=============================================================================
    Name    : P1MissileCorvette.c
    Purpose : Specifics for the P1MissileCorvette

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "P1MissileCorvette.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "P1Mothership.h"
#include "universe.h"

typedef struct
{
    real32 missilecorvetteGunRange[NUM_TACTICS_TYPES];
    real32 missilecorvetteTooCloseRange[NUM_TACTICS_TYPES];
    real32 missileRegenerateTime;
    real32 missileVolleyTime;
    real32 missileLagVolleyTime;
    AttackSideStepParameters sidestepParameters;
} P1MissileCorvetteStatics;

P1MissileCorvetteStatics P1MissileCorvetteStaticBeast;
P1MissileCorvetteStatics P1MissileCorvetteStaticP1;

scriptStructEntry P1MissileCorvetteScriptTable[] =
{
    { "MissileRegenerateTime",scriptSetReal32CB,(udword) &(P1MissileCorvetteStaticBeast.missileRegenerateTime),(udword) &(P1MissileCorvetteStaticBeast) },
    { "MissileVolleyTime",scriptSetReal32CB,(udword) &(P1MissileCorvetteStaticBeast.missileVolleyTime),(udword) &(P1MissileCorvetteStaticBeast) },
    { "MissileLagVolleyTime",scriptSetReal32CB,(udword) &(P1MissileCorvetteStaticBeast.missileLagVolleyTime),(udword) &(P1MissileCorvetteStaticBeast) },
    { NULL,NULL,0,0 }
};

void P1MissileCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    P1MissileCorvetteStatics *corvstat = (statinfo->shiprace == RACE_P1) ? &P1MissileCorvetteStaticP1 : &P1MissileCorvetteStaticBeast;

    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);
    scriptSetStruct(directory,filename,P1MissileCorvetteScriptTable,(ubyte *)corvstat);

    statinfo->custstatinfo = corvstat;

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        corvstat->missilecorvetteGunRange[i] = statinfo->bulletRange[i];
        corvstat->missilecorvetteTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

void P1MissileCorvetteInit(Ship *ship)
{
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;

    attackSideStepInit(&spec->attacksidestep);

    spec->lasttimeRegeneratedMissiles = 0.0f;
    spec->lasttimeFiredVolley = 0.0f;
    spec->lasttimeDidSpecialTargeting = -100000.0f;
    spec->curTargetIndex = 0;
}

void P1MissileCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;
    P1MissileCorvetteStatics *corvstat = (P1MissileCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void P1MissileCorvetteAttackPassive(Ship *ship,Ship *target,bool32 rotate)
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

static bool32 P1MissileCorvetteAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    P1MissileCorvetteAttack(ship, target, maxdist);
    return TRUE;
}

void P1MissileCorvetteHousekeep(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;
    P1MissileCorvetteStatics *mcorvettestat = (P1MissileCorvetteStatics *)shipstaticinfo->custstatinfo;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    GunStatic *gunstatic;
    sdword minMissiles;
    sdword minIndex;

    if ((universe.totaltimeelapsed - spec->lasttimeDidSpecialTargeting) > mcorvettestat->missileLagVolleyTime)
    {
        if ((universe.totaltimeelapsed - spec->lasttimeRegeneratedMissiles) > mcorvettestat->missileRegenerateTime)
        {
            spec->lasttimeRegeneratedMissiles = universe.totaltimeelapsed;
            numGuns = gunInfo->numGuns;

            // find missile launcher with least missiles, and give it a missile
            for (i=0,minIndex=-1,minMissiles=100000;i<numGuns;i++)
            {
                gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
                if (gunstatic->guntype == GUN_MissileLauncher)
                {
                    gun = &gunInfo->guns[i];
                    if (gun->numMissiles < gunstatic->maxMissiles)
                    {
                        if (gun->numMissiles < minMissiles)
                        {
                            minMissiles = gun->numMissiles;
                            minIndex = i;
                        }
                    }
                }
            }

            if (minIndex != -1)
            {
                gunInfo->guns[minIndex].numMissiles++;
            }
        }
    }
}

bool32 P1MissileCorvetteSpecialTarget(Ship *ship,void *custom)
{
    SelectAnyCommand *targets;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    P1MissileCorvetteSpec *spec = (P1MissileCorvetteSpec *)ship->ShipSpecifics;
    P1MissileCorvetteStatics *mcorvettestat = (P1MissileCorvetteStatics *)shipstaticinfo->custstatinfo;

    spec->lasttimeDidSpecialTargeting = universe.totaltimeelapsed;

    if ((universe.totaltimeelapsed - spec->lasttimeFiredVolley) <= mcorvettestat->missileVolleyTime)
    {
        return FALSE;
    }
    spec->lasttimeFiredVolley = universe.totaltimeelapsed;

    targets = (SelectAnyCommand *)custom;
    if (targets->numTargets != 0 && spec->curTargetIndex >= targets->numTargets)
    {
        spec->curTargetIndex = 0;
    }
    spec->curTargetIndex = 0;
    return TRUE;
}

CustShipHeader P1MissileCorvetteHeader =
{
    sizeof(P1MissileCorvetteSpec),
    P1MissileCorvetteStaticInit,
    NULL,
    P1MissileCorvetteInit,
    NULL,
    shipShootAllGunsAtTarget,
    P1MothershipRotateAndShoot,
    NULL,
    P1MissileCorvetteSpecialTarget,
    NULL,
    P1MissileCorvetteHousekeep,
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
    P1MissileCorvetteAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
