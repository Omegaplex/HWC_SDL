/*=============================================================================
    Name    : StandardDestroyer.c
    Purpose : Specifics for the StandardDestroyer

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "StandardDestroyer.h"
#include "Carrier.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "Dreadnought.h"
#include "universe.h"

typedef struct
{
    real32 standarddestroyerGunRange[NUM_TACTICS_TYPES];
    real32 standarddestroyerTooCloseRange[NUM_TACTICS_TYPES];
    real32 missileRegenerateTime;
    real32 missileVolleyTime;
    real32 missileLagVolleyTime;
} StandardDestroyerStatics;

typedef struct
{
    real32 lasttimeRegeneratedMissiles;
    real32 lasttimeFiredVolley;
    real32 lasttimeDidSpecialTargeting;
    sdword curTargetIndex;
    sdword volleyState;
} StandardDestroyerRuntimeOverlay;

StandardDestroyerStatics StandardDestroyerStaticRace1;
StandardDestroyerStatics StandardDestroyerStaticRace2;
StandardDestroyerStatics StandardDestroyerStaticRace3;

scriptStructEntry StandardDestroyerScriptTable[] =
{
    { "MissileRegenerateTime", scriptSetReal32CB, (udword)&StandardDestroyerStaticRace1.missileRegenerateTime, (udword)&StandardDestroyerStaticRace1 },
    { "MissileVolleyTime", scriptSetReal32CB, (udword)&StandardDestroyerStaticRace1.missileVolleyTime, (udword)&StandardDestroyerStaticRace1 },
    { "MissileLagVolleyTime", scriptSetReal32CB, (udword)&StandardDestroyerStaticRace1.missileLagVolleyTime, (udword)&StandardDestroyerStaticRace1 },
    { NULL, NULL, 0, 0 }
};

// Decompiled from cataclysm.exe @ 0x5C7780
void StandardDestroyerStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    udword i;
    StandardDestroyerStatics *sdestroyerstat;

    switch (statinfo->shiprace)
    {
        case RACE_Sect:
            sdestroyerstat = &StandardDestroyerStaticRace1;
            scriptSetStruct(directory, filename, StandardDestroyerScriptTable, (ubyte *)sdestroyerstat);
            break;
        case RACE_Kushan:
            sdestroyerstat = &StandardDestroyerStaticRace2;
            break;
        case RACE_Taiidan:
            sdestroyerstat = &StandardDestroyerStaticRace3;
            break;
        default:
            sdestroyerstat = (StandardDestroyerStatics *)statinfo;
            break;
    }

    statinfo->custstatinfo = sdestroyerstat;

    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        sdestroyerstat->standarddestroyerGunRange[i] = statinfo->bulletRange[i];
        sdestroyerstat->standarddestroyerTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
}

// Decompiled from cataclysm.exe @ 0x5C7800
void StandardDestroyerHousekeep(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    StandardDestroyerRuntimeOverlay *runtime = (StandardDestroyerRuntimeOverlay *)ship->ShipSpecifics;
    StandardDestroyerStatics *sdestroyerstat = (StandardDestroyerStatics *)shipstaticinfo->custstatinfo;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    sdword i;
    Gun *gun;
    GunStatic *gunstatic;
    sdword minMissiles;
    sdword minIndex;

    dbgAssert(gunInfo != NULL);

    if (ship->shiptype == sDestroyer &&
        (universe.totaltimeelapsed - runtime->lasttimeDidSpecialTargeting) > sdestroyerstat->missileLagVolleyTime &&
        (universe.totaltimeelapsed - runtime->lasttimeRegeneratedMissiles) > sdestroyerstat->missileRegenerateTime)
    {
        runtime->lasttimeRegeneratedMissiles = universe.totaltimeelapsed;
        numGuns = gunInfo->numGuns;

        for (i = 0, minIndex = -1, minMissiles = 100000; i < numGuns; i++)
        {
            gunstatic = &shipstaticinfo->gunStaticInfo->gunstatics[i];
            if (gunstatic->guntype == GUN_MissileLauncher)
            {
                gun = &gunInfo->guns[i];
                if (gun->numMissiles < gunstatic->maxMissiles && gun->numMissiles < minMissiles)
                {
                    minMissiles = gun->numMissiles;
                    minIndex = i;
                }
            }
        }

        if (minIndex != -1)
        {
            gunInfo->guns[minIndex].numMissiles++;
        }
    }
}

void StandardDestroyerAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    StandardDestroyerStatics *sdestroyerstat = (StandardDestroyerStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship, target, sdestroyerstat->standarddestroyerGunRange[ship->tacticstype], sdestroyerstat->standarddestroyerTooCloseRange[ship->tacticstype]);
    (void)maxdist;
}

CustShipHeader StandardDestroyerHeader =
{
    sizeof(StandardDestroyerSpec),
    StandardDestroyerStaticInit,
    NULL,
    NULL,
    NULL,
    shipShootAllGunsAtTarget,
    shipSharedRotateAndShoot,
    NULL,
    NULL,
    NULL,
    StandardDestroyerHousekeep,
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
