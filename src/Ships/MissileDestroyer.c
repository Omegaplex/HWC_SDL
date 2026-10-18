/*=============================================================================
    Name    : MissileDestroyer.c
    Purpose : Specifics for the MissileDestroyer

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "MissileDestroyer.h"
#include "Carrier.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "Dreadnought.h"
#include "universe.h"
#include "soundevent.h"
#include "battle.h"
#include "aiship.h"
#include "aitrack.h"
#include "collision.h"

typedef struct
{
    real32 missiledestroyerGunRange[NUM_TACTICS_TYPES];
    real32 missiledestroyerTooCloseRange[NUM_TACTICS_TYPES];
    real32 missileRegenerateTime;
    real32 missileVolleyTime;
    real32 missileLagVolleyTime;
} MissileDestroyerStatics;

MissileDestroyerStatics MissileDestroyerStaticRace1;
MissileDestroyerStatics MissileDestroyerStaticRace2;

scriptStructEntry MissileDestroyerScriptTable[] =
{
    { "MissileRegenerateTime",scriptSetReal32CB,(udword) &(MissileDestroyerStaticRace1.missileRegenerateTime),(udword) &(MissileDestroyerStaticRace1) },
    { "MissileVolleyTime",scriptSetReal32CB,(udword) &(MissileDestroyerStaticRace1.missileVolleyTime),(udword) &(MissileDestroyerStaticRace1) },
    { "MissileLagVolleyTime",scriptSetReal32CB,(udword) &(MissileDestroyerStaticRace1.missileLagVolleyTime),(udword) &(MissileDestroyerStaticRace1) },
    { NULL,NULL,0,0 }
};

// Decompiled from cataclysm.exe @ 0x5BB350
void MissileDestroyerStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    MissileDestroyerStatics *mdestroyerstat = (statinfo->shiprace != RACE_Taiidan) ? &MissileDestroyerStaticRace1 : &MissileDestroyerStaticRace2;

    statinfo->custstatinfo = mdestroyerstat;

    scriptSetStruct(directory,filename,MissileDestroyerScriptTable,(ubyte *)mdestroyerstat);

    for(i=0;i<NUM_TACTICS_TYPES;i++)
    {
        mdestroyerstat->missiledestroyerGunRange[i] = statinfo->bulletRange[i];
        mdestroyerstat->missiledestroyerTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

// Decompiled from cataclysm.exe @ 0x5BB3B0
void MissileDestroyerInit(Ship *ship)
{
    MissileDestroyerSpec *spec = (MissileDestroyerSpec *)ship->ShipSpecifics;

    spec->lasttimeRegeneratedMissiles = 0.0f;
    spec->lasttimeFiredVolley = 0.0f;
    spec->lasttimeDidSpecialTargeting = -100000.0f;
    spec->curTargetIndex = 0;
    spec->volleyState = VOLLEY_BEGIN;
}

void MissileDestroyerAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    MissileDestroyerStatics *mdestroyerstat = (MissileDestroyerStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship,target,mdestroyerstat->missiledestroyerGunRange[ship->tacticstype],mdestroyerstat->missiledestroyerTooCloseRange[ship->tacticstype]);
}

void MissileDestroyerAttackPassive(Ship *ship,Ship *target,bool32 rotate)
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

// Decompiled from cataclysm.exe @ 0x5BB420
void MissileDestroyerHousekeep(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    MissileDestroyerSpec *spec = (MissileDestroyerSpec *)ship->ShipSpecifics;
    MissileDestroyerStatics *mdestroyerstat = (MissileDestroyerStatics *)shipstaticinfo->custstatinfo;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    GunStatic *gunstatic;
    sdword minMissiles;
    sdword minIndex;
    dbgAssert(gunInfo != NULL);

    if ((universe.totaltimeelapsed - spec->lasttimeDidSpecialTargeting) > mdestroyerstat->missileLagVolleyTime)
    {
        if ((universe.totaltimeelapsed - spec->lasttimeRegeneratedMissiles) > mdestroyerstat->missileRegenerateTime)
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

// Decompiled from cataclysm.exe @ 0x5BB4E0
bool32 MissileDestroyerSpecialTarget(Ship *ship,void *custom)
{
    SelectAnyCommand *targets;
    sdword i;
    sdword numShipsToTarget;
    sdword numGuns;
    Gun *gun;
    real32 range;
    GunStatic *gunstatic;
    SpaceObjRotImpTarg *target;
    ShipStaticInfo *shipstaticinfo;
    MissileDestroyerSpec *spec;
    MissileDestroyerStatics *mdestroyerstat;
    GunInfo *gunInfo;
    bool32 firedSomeMissiles,triedToFire;
    vector trajectory;

    spec = (MissileDestroyerSpec *)ship->ShipSpecifics;
    shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    mdestroyerstat = (MissileDestroyerStatics *)shipstaticinfo->custstatinfo;
    gunInfo = ship->cataclysmGunInfo;
    dbgAssert(gunInfo != NULL);

    spec->lasttimeDidSpecialTargeting = universe.totaltimeelapsed;

    if ((universe.totaltimeelapsed - spec->lasttimeFiredVolley) > mdestroyerstat->missileVolleyTime)
    {
        spec->lasttimeFiredVolley = universe.totaltimeelapsed;
    }
    else
    {
        return FALSE;
    }

    targets = (SelectAnyCommand *)custom;
    numShipsToTarget = targets->numTargets;

    if (numShipsToTarget == 0)
    {
        // have destroyed targets, so we are done
        spec->curTargetIndex = 0;
        spec->volleyState = VOLLEY_BEGIN;
        return TRUE;
    }

    if(spec->volleyState == VOLLEY_BEGIN)
    {
        spec->volleyState = VOLLEY_NOT_BEGIN;
        //////////////////////
        //Volley attack speech event!
        //event num: COMM_MissleDest_VolleyAttack
        //use battle chatter
        if(ship->playerowner->playerIndex == universe.curPlayerIndex)
        {
            if (battleCanChatterAtThisTime(BCE_COMM_MissleDest_VolleyAttack, ship))
            {
                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_MissleDest_VolleyAttack, ship, SOUND_EVENT_DEFAULT);
            }
        }


        //////////////////////

    }

    if (spec->curTargetIndex >= numShipsToTarget)
    {
        spec->curTargetIndex = 0;
    }

    numGuns = gunInfo->numGuns;
    firedSomeMissiles = FALSE;
    triedToFire=FALSE;
    for (i=0;i<numGuns;i++)
    {
        gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
        if (gunstatic->guntype == GUN_MissileLauncher)
        {
            vecSub(trajectory,ship->collInfo.collPosition,targets->TargetPtr[spec->curTargetIndex]->collInfo.collPosition);
            range = RangeToTarget(ship,targets->TargetPtr[spec->curTargetIndex],&trajectory);
            gun = &gunInfo->guns[i];
            if(range < (gun->gunstatic->bulletrange*0.9f))
            {
                triedToFire=TRUE;
                if (gunHasMissiles(gun))
                {
                    firedSomeMissiles = TRUE;

                    target = targets->TargetPtr[spec->curTargetIndex];

                    gunShoot(ship,gun,target);

                    spec->curTargetIndex++;
                    if (spec->curTargetIndex >= numShipsToTarget)
                    {
                        spec->curTargetIndex = 0;
                    }
                }
            }
        }
    }

    if (firedSomeMissiles)
    {
        aitrackZeroVelocity(ship);
        return FALSE;
    }
    else if(triedToFire)
    {
        // all empty of missiles, so we are done
        spec->curTargetIndex = 0;
        spec->volleyState = VOLLEY_BEGIN;
        return TRUE;
    }
    else
    {
        return aishipFlyToShipAvoidingObjs(ship,(Ship *)targets->TargetPtr[0],AISHIP_FastAsPossible|AISHIP_PointInDirectionFlying,0.0f);
    }
}

CustShipHeader MissileDestroyerHeader =
{
    sizeof(MissileDestroyerSpec),
    MissileDestroyerStaticInit,
    NULL,
    MissileDestroyerInit,
    NULL,
    shipShootAllGunsAtTarget,
    shipSharedRotateAndShoot,
    NULL,
    MissileDestroyerSpecialTarget,
    NULL,
    MissileDestroyerHousekeep,
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
