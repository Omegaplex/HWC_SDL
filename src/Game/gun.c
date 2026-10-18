/*=============================================================================
    Name    : Gun.c
    Purpose : Handles gun firing

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "glinc.h"
#include "types.h"
#include "fastmath.h"
#include "debug.h"
#include "linkedlist.h"
#include "memory.h"
#include "spaceobj.h"
#include "physics.h"
#include "vector.h"
#include "matrix.h"
#include "universe.h"
#include "soundevent.h"
#include "univupdate.h"
#include "prim3d.h"
#include "render.h"
#include "tweak.h"
#include "aiship.h"
#include "gun.h"
#include "ships.h"
#include "collision.h"
#include "tactics.h"
#include "madlinkindefs.h"
#include "madlinkin.h"
#include "randy.h"
#include "etg.h"

#include "mainrgn.h"
#include "mex.h"

#include "Ping.h"

#include "nis.h"

#define ENABLE_SHIPRECOIL       0

#define GUNBURST_STATE_READY        0
#define GUNBURST_STATE_BURST        1
#define GUNBURST_STATE_COOLOFF      2

/*=============================================================================
    Data:
=============================================================================*/
real32 gunRecoilTable[GUN_RecoilTableLength + 1];

/*=============================================================================
    Functions:
=============================================================================*/

static GunSoundType gunWeaponTypeToSoundType(GunSize gunSize, WeaponType weaponType)
{
    switch (weaponType)
    {
        case WEAPON_Projectile:
            switch (gunSize)
            {
                case GUNSIZE_Small:     return GS_SmallProjectile;
                case GUNSIZE_Medium:    return GS_MediumProjectile;
                case GUNSIZE_Large:     return GS_LargeProjectile;
                case GUNSIZE_VeryLarge: return GS_VeryLargeProjectile;
            }
            break;
        case WEAPON_Energy:
            switch (gunSize)
            {
                case GUNSIZE_Small:     return GS_SmallEnergyCannon;
                case GUNSIZE_Medium:    return GS_MediumEnergyCannon;
                case GUNSIZE_Large:     return GS_LargeEnergyCannon;
                case GUNSIZE_VeryLarge: return GS_VeryLargeEnergyCannon;
            }
            break;
        case WEAPON_Ion:
            switch (gunSize)
            {
                case GUNSIZE_Small:     return GS_SmallIonCannon;
                case GUNSIZE_Medium:    return GS_MediumIonCannon;
                case GUNSIZE_Large:     return GS_LargeIonCannon;
                case GUNSIZE_VeryLarge: return GS_VeryLargeIonCannon;
            }
            break;
        case WEAPON_PlasmaBomb:
            switch (gunSize)
            {
                case GUNSIZE_Small:     return GS_SmallPlasmaBomb;
                case GUNSIZE_Medium:    return GS_MediumPlasmaBomb;
                case GUNSIZE_Large:     return GS_LargePlasmaBomb;
                case GUNSIZE_VeryLarge: return GS_VeryLargePlasmaBomb;
            }
            break;
        case WEAPON_Laser:
            return GS_Laser;
        case WEAPON_Missile:
            return GS_MissleLauncher;
        case WEAPON_Mine:
            return GS_MineLauncher;
        case WEAPON_BigGun:
            return GS_BigGun;
        case WEAPON_BeastInfection:
            return GS_BeastWeapon;
    }
    return GS_LargeEnergyCannon;
}

static BulletType gunSoundTypeToBulletType(GunSoundType gunSoundType)
{
    switch (gunSoundType)
    {
        case GS_LargeIonCannon:
        case GS_MediumIonCannon:
        case GS_SmallIonCannon:
        case GS_VeryLargeIonCannon:
            return BULLET_Beam;
        case GS_Laser:
            return BULLET_Laser;
        case GS_LargePlasmaBomb:
        case GS_MediumPlasmaBomb:
        case GS_SmallPlasmaBomb:
        case GS_VeryLargePlasmaBomb:
            return BULLET_PlasmaBomb;
        default:
            return BULLET_Projectile;
    }
}

real32 univGetDamageMultiplier(sdword weaponClass, sdword armorClass)
{
    static const real32 damageMultiplierTable[60] =
    {
        80.0f, 75.0f, 50.0f,
        16.0f, 14.0f, 9.0f,
        100.0f, 95.0f, 65.0f,
        40.0f, 28.0f, 26.0f,
        40.0f, 38.0f, 30.0f,
        10.0f, 9.0f, 6.0f,
        70.0f, 65.0f, 45.0f,
        20.0f, 15.0f, 14.0f,
        400.0f, 300.0f, 295.0f,
        60.0f, 40.0f, 38.0f,
        22.0f, 20.0f, 14.0f,
        5.0f, 4.0f, 3.0f,
        50.0f, 47.0f, 35.0f,
        7.0f, 4.0f, 4.0f,
        125.0f, 115.0f, 90.0f,
        36.0f, 33.0f, 25.0f,
        150.0f, 140.0f, 100.0f,
        10.0f, 10.0f, 8.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };

    dbgAssert(weaponClass >= 0 && weaponClass < 20);
    dbgAssert(armorClass >= 0 && armorClass < 3);
    return damageMultiplierTable[3 * weaponClass + armorClass];
}

static sdword gunGetTargetArmorClass(SpaceObjRotImpTarg *target)
{
    if (target == NULL || target->objtype != OBJ_ShipType)
    {
        return 0;
    }

    return ((Ship *)target)->upgradeState.level;
}

static WeaponType gunGetActiveWeaponMode(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;

    if (gun->weaponMode == gunstatic->endUpgradeLevel &&
        gunstatic->endUpgradeLevel != gunstatic->startUpgradeLevel)
    {
        return gunstatic->endUpgradeLevel;
    }

    return gunstatic->startUpgradeLevel;
}

GunSoundType gunGetActiveSoundType(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;
    if (gunGetActiveWeaponMode(ship, gun) == gunstatic->endUpgradeLevel &&
        gunstatic->endUpgradeLevel != gunstatic->startUpgradeLevel)
    {
        return (GunSoundType)gunstatic->bulletType;
    }

    return gunstatic->gunsoundtype;
}

real32 gunGetActiveBulletSpeed(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;

    if (gunGetActiveWeaponMode(ship, gun) == gunstatic->endUpgradeLevel &&
        gunstatic->endUpgradeLevel != gunstatic->startUpgradeLevel &&
        (gunstatic->bulletspeed2 > 0.0f || gunstatic->bulletspeed <= 0.0f))
    {
        return gunstatic->bulletspeed2;
    }

    return gunstatic->bulletspeed;
}

static real32 gunGetActiveBulletLifetime(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;

    if (gunGetActiveWeaponMode(ship, gun) == gunstatic->endUpgradeLevel &&
        gunstatic->endUpgradeLevel != gunstatic->startUpgradeLevel &&
        (gunstatic->bulletlifetime2 > 0.0f || gunstatic->bulletlifetime <= 0.0f))
    {
        return gunstatic->bulletlifetime2;
    }

    return gunstatic->bulletlifetime;
}

static real32 gunGetActiveFireTime(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;

    if (gunGetActiveWeaponMode(ship, gun) == gunstatic->endUpgradeLevel &&
        gunstatic->endUpgradeLevel != gunstatic->startUpgradeLevel &&
        gunstatic->firetime2 > 0.0f)
    {
        return gunstatic->firetime2;
    }

    return gunstatic->firetime;
}

/*-----------------------------------------------------------------------------
    Name        : gunCanShoot
    Description : returns TRUE if gun can shoot
    Inputs      : gun
    Outputs     :
    Return      : returns TRUE if gun can shoot
----------------------------------------------------------------------------*/
bool32 gunCanShoot(Ship *ship, Gun *gun)
{
   GunStatic *gunstatic = gun->gunstatic;
   CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
   real32 firetime;
   real32 healthRatio;
   real32 sinceLastFire;
   uword attributes;

   if (gunstatic->burstFireTime == 0.0f)
   {
       firetime = gunstatic->firetime;
       if (gunstatic->gunsoundtype != GS_BeastWeapon &&
           gunstatic->gunsoundtype != GS_BigGun)
       {
           healthRatio = ship->health / ship->staticinfo->maxhealth;
           if (healthRatio < 0.10f)
           {
               firetime *= 4.0f;
           }
           else if (healthRatio < 0.25f)
           {
               firetime *= 2.5f;
           }
           else if (healthRatio < 0.50f)
           {
               firetime *= 1.75f;
           }
           else if (healthRatio < 0.75f)
           {
               firetime *= 1.25f;
           }
       }
       return universe.totaltimeelapsed - gun->lasttimefired > firetime;
   }

   switch (gun->burstState)
   {
       case GUNBURST_STATE_BURST:
           ship->cataBurstFiring = TRUE;
           if ((universe.totaltimeelapsed - gun->burstStartTime) <= gun->useBurstFireTime)
           {
               goto burst_not_ready;
           }
           gun->burstState = GUNBURST_STATE_COOLOFF;
           gun->burstStartTime = universe.totaltimeelapsed;
           soundEventShipRemove(ship);
       case GUNBURST_STATE_COOLOFF:
           ship->cataBurstFiring = FALSE;
           if ((universe.totaltimeelapsed - gun->burstStartTime) <= gun->useBurstWaitTime)
           {
               return FALSE;
           }
           gun->burstState = GUNBURST_STATE_READY;
           break;

       case GUNBURST_STATE_READY:
       default:
           break;
   }

   gun->burstStartTime = universe.totaltimeelapsed;
   gun->burstState = GUNBURST_STATE_BURST;
   gun->useBurstFireTime = gunstatic->burstFireTime;
   gun->useBurstWaitTime = gunstatic->burstWaitTime;

   healthRatio = ship->health / ship->staticinfo->maxhealth;
   if (healthRatio < 0.10f)
   {
       gun->useBurstWaitTime *= 4.0f;
   }
   else if (healthRatio < 0.25f)
   {
       gun->useBurstWaitTime *= 2.5f;
   }
   else if (healthRatio < 0.50f)
   {
       gun->useBurstWaitTime *= 1.75f;
   }
   else if (healthRatio < 0.75f)
   {
       gun->useBurstWaitTime *= 1.25f;
   }

burst_not_ready:
   if (command == NULL)
   {
       return universe.totaltimeelapsed - gun->lasttimefired > gunstatic->firetime;
   }

   attributes = command->ordertype.attributes;
   if ((attributes & COMMAND_IS_PASSIVEATTACKING) == 0 ||
       ship->staticinfo->passiveAttackPenaltyExempt ||
       ((attributes & COMMAND_IS_PROTECTING) != 0 &&
        (attributes & COMMAND_IS_FORMATION) != 0 &&
        command->formation.formationtype == SPHERE_FORMATION))
   {
       return universe.totaltimeelapsed - gun->lasttimefired > gunstatic->firetime;
   }

   firetime = tacticsInfo.PassiveRetaliationFireTimeModifierLow + gunstatic->firetime;
   sinceLastFire = universe.totaltimeelapsed - gun->lasttimefired;
   if (sinceLastFire > firetime)
   {
       if ((real32)ranRandom(10) *
               ((tacticsInfo.PassiveRetaliationFireTimeModifierHi -
                 tacticsInfo.PassiveRetaliationFireTimeModifierLow) * 0.1f) +
           tacticsInfo.PassiveRetaliationFireTimeModifierLow +
           firetime < sinceLastFire)
       {
           return TRUE;
       }
   }

   return FALSE;
}

bool32 gunCanShootStaggered(Ship *ship, Gun *gun)
{
    GunStatic *gunstatic;
    GunInfo *gunInfo;
    Gun *otherGun;
    GunStatic *otherStatic;
    real32 sinceLastFire;
    real32 mostRecentLastTimeFired;
    sdword staggeredGunCount;
    sdword i;

    if (ship != NULL && ship->unk_590 == 1)
    {
        return FALSE;
    }

    gunstatic = gun->gunstatic;
    if (gun->weaponMode != gunstatic->endUpgradeLevel ||
        gunstatic->startUpgradeLevel == gunstatic->endUpgradeLevel)
    {
        return gunCanShoot(ship, gun);
    }

    sinceLastFire = universe.totaltimeelapsed - gun->lasttimefired;
    if (gunstatic->firetime2 > 0.0f)
    {
        if (sinceLastFire > gunstatic->firetime2)
        {
            gunInfo = ship->cataclysmGunInfo;
            mostRecentLastTimeFired = gun->lasttimefired;
            staggeredGunCount = 0;

            if (gunInfo == NULL || gunInfo->numGuns <= 0)
            {
                return TRUE;
            }

            otherGun = gunInfo->guns;
            for (i = 0; i < gunInfo->numGuns; i++, otherGun++)
            {
                otherStatic = otherGun->gunstatic;
                if (otherGun->weaponMode == otherStatic->endUpgradeLevel &&
                    otherStatic->startUpgradeLevel != otherStatic->endUpgradeLevel)
                {
                    staggeredGunCount++;
                    if (mostRecentLastTimeFired < otherGun->lasttimefired)
                    {
                        mostRecentLastTimeFired = otherGun->lasttimefired;
                    }
                }
            }

            if (staggeredGunCount <= 1)
            {
                return TRUE;
            }

            if (mostRecentLastTimeFired < universe.totaltimeelapsed - gunstatic->firetime2 / (real32)staggeredGunCount)
            {
                return TRUE;
            }
        }
    }
    else if (sinceLastFire > gunstatic->firetime)
    {
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : gunNewGimbleUpdateCoordSys
    Description : update a New_Gimble gun's coordinate system to reflect gun->angle, gun->declination
    Inputs      : gun, gunstatic
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunNewGimbleUpdateCoordSys(Gun *gun,GunStatic *gunstatic)
{
    matrix newmat,rotxmatrix,rotymatrix;

    matMakeRotAboutY(&rotymatrix,(real32)cos(gun->declination),(real32)sin(gun->declination));
    matMakeRotAboutX(&rotxmatrix,(real32)cos(gun->angle),(real32)sin(gun->angle));

    matMultiplyMatByMat(&newmat,&gunstatic->defaultgunorientation,&rotxmatrix);
    matMultiplyMatByMat(&gun->curgunorientation,&newmat,&rotymatrix);
    if (bitTest(gunstatic->flags, GF_MultiLevelMatrix))
    {
        matMultiplyMatByMat(&newmat,&gunstatic->defaultGunOrientationNonConcat,&rotymatrix);
        matMultiplyMatByMat(&gun->curGunOrientationNonConcat,&newmat,&rotxmatrix);
    }

    matGetVectFromMatrixCol3(gun->gunheading,gun->curgunorientation);
}

/*-----------------------------------------------------------------------------
    Name        : gunUpdateSlave
    Description : Update a slave gun's status from it's slave driver
    Inputs      : gun - slave gun
                  guns - list of gunst
                  iSlaveDriver - index of slaveDriver gun
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunUpdateSlave(Gun *gun, GunInfo *guns, sdword iSlaveDriver)
{
    Gun *slaveDriver;

    dbgAssert(iSlaveDriver >= 0 && iSlaveDriver < guns->numGuns);
    slaveDriver = &guns->guns[iSlaveDriver];
    gun->gunheading = slaveDriver->gunheading;              //copy the essentials from the slave driver
    gun->curgunorientation = slaveDriver->curgunorientation;
    gun->declination = slaveDriver->declination;
    gun->angle = slaveDriver->angle;
}

/*-----------------------------------------------------------------------------
    Name        : gunOrientGimbleGun
    Description : if possible, orients the gimble gun to point at target
    Inputs      : gun,target
    Outputs     :
    Return      : TRUE if gun oriented as desired
----------------------------------------------------------------------------*/
bool32 gunOrientGimbleGun(Ship *ship,Gun *gun,SpaceObjRotImpTarg *target)
{
    GunStatic *gunstatic = gun->gunstatic;
    vector targetInWorldCoordSys;
    vector targetInShipCoordSys;
    vector targetInGunCoordSys;
    real32 dotprod,bulletspeed;
    real32 triggerHappy, maxAngle, maxAngleSpeed;

    real32 tryangle;
    real32 turndeclination;
    real32 desireddeclinationspeed;
    real32 turnangle;
    real32 desiredanglespeed;

    bool32 updateguncoordsys;

    if (bitTest(ship->flags, SOF_NISShip))
    {
        triggerHappy = NIS_TriggerHappyAngle;
        maxAngle = NIS_TriggerHappyAngle;
        maxAngleSpeed = gunstatic->maxanglespeed;
    }
    else
    {
        triggerHappy = gunstatic->triggerHappy;
        maxAngle = gunstatic->cosmaxAngleFromNorm;
        maxAngleSpeed = gunstatic->maxanglespeed;
    }
    bulletspeed = gunGetActiveBulletSpeed(ship, gun);


    if (gunstatic->guntype == GUN_Gimble)
    {
        aishipGetTrajectoryWithVelPrediction(ship,target,bulletspeed,&targetInWorldCoordSys);
//        vecSub(targetInWorldCoordSys,target->posinfo.position,ship->posinfo.position);

        matMultiplyVecByMat(&targetInShipCoordSys,&targetInWorldCoordSys,&ship->rotinfo.coordsys);

        vecSubFrom(targetInShipCoordSys,gunstatic->position);

        vecNormalize(&targetInShipCoordSys);

        dotprod = vecDotProduct(targetInShipCoordSys,gunstatic->gunnormal);
        if (dotprod > 1.0f)
        {
            dotprod = 1.0f;     // do this because vecNormalize does not normalize perfectly
        }
        if (isBetweenInclusive(dotprod,maxAngle,gunstatic->cosminAngleFromNorm))
        {
            gun->gunheading = targetInShipCoordSys;

            soundEventStop(gun->gimblehandle);
            gun->gimblehandle = -1;

            return TRUE;
        }

        if (gun->gimblehandle < 0)
        {
            gun->gimblehandle = soundEventPlay(ship, Gun_WeaponMove, gun);
        }

        return FALSE;
    }
    else if (gunstatic->guntype == GUN_NewGimble)
    {
        if (gunstatic->slaveDriver >= 0)
        {                                                   //if this gun has a slavedriver
            gunUpdateSlave(gun, ship->cataclysmGunInfo, gunstatic->slaveDriver);
        }

        aishipGetTrajectoryWithVelPrediction(ship,target,bulletspeed,&targetInWorldCoordSys);
//        vecSub(targetInWorldCoordSys,target->posinfo.position,ship->posinfo.position);

        matMultiplyVecByMat(&targetInShipCoordSys,&targetInWorldCoordSys,&ship->rotinfo.coordsys);

        // we have targetInShipCoordSys.  Now change to gun's coordinate system

        vecSubFrom(targetInShipCoordSys,gunstatic->position);

        vecNormalize(&targetInShipCoordSys);

        matMultiplyVecByMat(&targetInGunCoordSys,&targetInShipCoordSys,&gun->curgunorientation);

        dotprod = targetInGunCoordSys.z;        // (0,0,1) dot (x,y,z) == z

        gun->anglespeed = 0.0f;
        gun->declinationspeed = 0.0f;

        if (dotprod < triggerHappy)
        {
            WeaponType startUpgradeLevel = gunstatic->startUpgradeLevel;

            if (startUpgradeLevel != WEAPON_Ion && startUpgradeLevel != WEAPON_Laser)
            {
                goto onTargetExitEarly;
            }
        }

        updateguncoordsys = FALSE;

        if (gunstatic->slaveDriver < 0)
        {                                                   //only do updates for free guns
            // always want target to be on positive z axis in gun coordinate system (i.e. gun barrel on z should point to target)
            if (targetInGunCoordSys.y != 0)
            {
                desiredanglespeed = -targetInGunCoordSys.y * gunstatic->angletracking;
                if (desiredanglespeed > maxAngleSpeed)
                {
                    desiredanglespeed = maxAngleSpeed;
                }
                else if (desiredanglespeed < -maxAngleSpeed)
                {
                    desiredanglespeed = -maxAngleSpeed;
                }

                turnangle = desiredanglespeed * universe.unknown1;
                tryangle = turnangle + gun->angle;
                if ((tryangle < gunstatic->maxturnangle) && (tryangle > gunstatic->minturnangle))
                {
                    gun->angle = tryangle;
                    gun->anglespeed = desiredanglespeed;
                    updateguncoordsys = TRUE;
                }
            }

            if (targetInGunCoordSys.x != 0)
            {
                desireddeclinationspeed = targetInGunCoordSys.x * gunstatic->declinationtracking;
                if (desireddeclinationspeed > maxAngleSpeed)
                {
                    desireddeclinationspeed = maxAngleSpeed;
                }
                else if (desireddeclinationspeed < -maxAngleSpeed)
                {
                    desireddeclinationspeed = -maxAngleSpeed;
                }

                turndeclination = desireddeclinationspeed * universe.unknown1;
                tryangle = turndeclination + gun->declination;
                if ((tryangle < gunstatic->maxdeclination) && (tryangle > gunstatic->mindeclination))
                {
                    gun->declination = tryangle;
                    gun->declinationspeed = desireddeclinationspeed;
                    updateguncoordsys = TRUE;
                }
            }

            if (updateguncoordsys)
            {
                gunNewGimbleUpdateCoordSys(gun,gunstatic);
            }
        }

        if (dotprod >= triggerHappy)
        {
onTargetExitEarly:
            gun->gunheading = targetInShipCoordSys;

            soundEventStop(gun->gimblehandle);
            gun->gimblehandle = -1;

            return TRUE;
        }
        else
        {
            if (gun->gimblehandle < 0)
            {
                gun->gimblehandle = soundEventPlay(ship, Gun_WeaponMove, gun);
            }

            return FALSE;
        }
    }
    else
    {
        dbgAssert(FALSE);
        return FALSE;
    }
    return(FALSE);
}

bool32 gunCanHitTarget(Ship *ship, Gun *gun, SpaceObjRotImpTarg *target, bool32 useGunHeading)
{
    GunStatic *gunstatic = gun->gunstatic;
    real32 bulletSpeed;
    vector predictedPos;
    vector gunPosition;
    vector gunDirection;
    vector delta;
    real32 dist;
    real32 dotProduct;

    bulletSpeed = gunGetActiveBulletSpeed(ship, gun);
    aishipGetTrajectoryWithVelPrediction(ship, target, bulletSpeed, &predictedPos);

    matMultiplyMatByVec(&gunPosition, &ship->rotinfo.coordsys, &gunstatic->position);
    vecAddTo(gunPosition, ship->posinfo.position);

    vecSub(delta, predictedPos, gunPosition);

    if (useGunHeading)
    {
        matMultiplyMatByVec(&gunDirection, &ship->rotinfo.coordsys, &gun->gunheading);
    }
    else
    {
        matGetVectFromMatrixCol3(gunDirection, ship->rotinfo.coordsys);
    }

    dist = fsqrt(vecMagnitudeSquared(delta));
    vecDivideByScalar(delta, dist, dist);

    dotProduct = vecDotProduct(delta, gunDirection);
    if (dotProduct <= 0.0f)
    {
        return FALSE;
    }
    if (dotProduct > 1.0f)
    {
        dotProduct = 1.0f;
    }

    return fsqrt(1.0f - dotProduct * dotProduct) * dist <
           target->staticinfo->staticheader.staticCollInfo.collspheresize * 0.75f;
}

void gunGetGunPositionInWorld(vector *positionInWorldCoordSys,matrix *coordsys,Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;
    matMultiplyMatByVec(positionInWorldCoordSys,coordsys,&gunstatic->position);
}

/*-----------------------------------------------------------------------------
    Name        : missileShoot
    Description : shoots a missile
    Inputs      : ship which owns gun, gun to fire missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void missileShoot(Ship *ship,Gun *gun,SpaceObjRotImpTarg *target)
{
    Missile *missile;
    GunStatic *gunstatic = gun->gunstatic;
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    etgeffectstatic *stat;
    etglod *etgLOD;
    sdword LOD,i;
    real32 floatDamage;
    udword intDamage;
    ShipRace shiprace = ship->shiprace;
    matrix newCoordsys;

    MinelayerCorvetteSpec *spec;

    vector gunheadingInWorldCoordSys;
    vector positionInWorldCoordSys,mineup,mineright,minehead;

#if ENABLE_SHIPRECOIL
    vector recoil;
    real32 massovertime;
#endif

    dbgAssert(gunstatic->guntype == GUN_MissileLauncher || gunstatic->guntype == GUN_MineLauncher);
    dbgAssert(gunHasMissiles(gun));

    if (!ship->shipisattacking)
    {
        ship->shipisattacking = TRUE;
    }
    if ((ship->attackvars.myWingmanIs != NULL) && !ship->attackvars.myWingmanIs->shipisattacking)
    {
        ship->attackvars.myWingmanIs->shipisattacking = TRUE;
    }
    if (bitTest(ship->specialFlags1C8, CATA_RUNTIME_GRAVWELL_CAPTURED))
    {
        return;
    }
    if (target != NULL && target->objtype == OBJ_ShipType)
    {
        Ship *targetShip = (Ship *)target;
        if (bitTest(targetShip->flags, SOF_Cloaked))
        {
            return;
        }
    }

    //line-of-sight check
    if (target != NULL && gunstatic->guntype == GUN_MissileLauncher)
    {
        vector gunDirection, gunPosition;

        matMultiplyMatByVec(&gunDirection, &ship->rotinfo.coordsys, &gun->gunheading);
        matMultiplyMatByVec(&gunPosition, &ship->rotinfo.coordsys, &gunstatic->position);
        vecAddTo(gunPosition, ship->posinfo.position);
        if (!collCheckLineOfSight(ship, (Ship*)target, &gunPosition, &gunDirection))
        {
#if GUN_VERBOSE_LEVEL > 1
            dbgMessage("\nline-of-sight obscured, not shooting");
#endif
            return;
        }
    }

    gun->numMissiles--;
    gun->lasttimefired = universe.totaltimeelapsed;

    missile = memAlloc(sizeof(Missile),"Missile",0);
    memset(missile,0,sizeof(Missile));

    missile->objtype = OBJ_MissileType;
    missile->flags = SOF_Rotatable + SOF_Impactable + SOF_NotBumpable + SOF_Targetable;

    missile->enginePosition.x = 0.0f;
    missile->enginePosition.y = 0.0f;
    missile->enginePosition.z = 0.0f;
    missile->trail = NULL;

    if(gunstatic->guntype == GUN_MissileLauncher)
    {   //specific definitions for MISSILES
        missile->staticinfo = &missileStaticInfos[shiprace];
        missile->maxtraveldist = missile->staticinfo->staticheader.maxvelocity * universe.unknown1;
        missile->missileType = MISSILE_Regular;
        missile->FORCE_DROPPED = FALSE;
        missile->formationinfo = NULL;
        missile->trail = mistrailNew(missile->staticinfo->trailStatic, missile);
    }   //specific definitions for MINES
    else
    {   //must be a mine launcher
        missile->staticinfo = &mineStaticInfos[shiprace];
        dbgAssert(ship->shiptype == tMinelayerCorvette);
        spec = (MinelayerCorvetteSpec *)ship->ShipSpecifics;
        missile->missileType = MISSILE_Mine;
        missile->mineAIState = spec->mineaistate;       //initialize the Mines AI state
        if(spec->mineaistate == MINE_DROP_FORMATION)
        {
            listAddNode(&spec->mineforminfo->MineList,&(missile->formationLink),missile);
            missile->formationinfo = spec->mineforminfo;
            missile->formation_position = spec->formation_position;         //only way to communicate this info
            missile->formation_number_X = spec->formation_number_X;
            missile->formation_number_Y = spec->formation_number_Y;
            missile->maxtraveldist = ((MissileStaticInfo *)missile->staticinfo)->maxvelocity_FORCED * universe.unknown1;
            missile->FORCE_DROPPED = TRUE;                                  //set flag
        }
        else
        {
            missile->FORCE_DROPPED = FALSE;                                  //set flag
            missile->formationinfo = NULL;
            missile->maxtraveldist = missile->staticinfo->staticheader.maxvelocity * universe.unknown1;
        }
    }
    missile->haveCheckedForLoss = 0;    //init target loss variable

    missile->colorScheme = ship->colorScheme;
    missile->health = ((MissileStaticInfo *)missile->staticinfo)->maxhealth;

    ClearNode(missile->renderlink);
    missile->currentLOD = ship->currentLOD;
    missile->collMyBlob = NULL;
    missile->cameraDistanceSquared = ship->cameraDistanceSquared;
    missile->flashtimer = 0;
    missile->lasttimecollided = 0;
    missile->attributes = 0;
    missile->attributesParam = 0;

    vecZeroVector(missile->posinfo.force);
    vecZeroVector(missile->rotinfo.torque);

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.thruststrength[i] = missile->staticinfo->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.rotstrength[i] = missile->staticinfo->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
       missile->nonstatvars.turnspeed[i] = missile->staticinfo->turnspeedstat[i];
    }


    missile->soundType = gunstatic->gunsoundtype;
    missile->race = shiprace;
    missile->timelived = 0.0f;
    missile->playerowner = ship->playerowner;
    missile->owner = ship;
    missile->target = target;

    if(missile->FORCE_DROPPED == TRUE)
    {   //force dropped mine only!
        MinelayerCorvetteStatics *staticinfo = (MinelayerCorvetteStatics *) ship->staticinfo->custstatinfo;
        missile->damage = (frandombetween(staticinfo->forced_drop_damage_lo,staticinfo->forced_drop_damage_hi));
        missile->totallifetime = staticinfo->forced_drop_lifetime;
    }
    else
    {   //normal
        missile->damage = (frandombetween(gunstatic->gunDamageLo[ship->tacticstype],gunstatic->gunDamageHi[ship->tacticstype]));
        missile->totallifetime = gunstatic->bulletlifetime;
    }

    //test to see if needs damage modifier
    if(bitTest(ship->specialFlags,SPECIAL_KasSpecialDamageModifier))
    {
        //ship is being 'beefed' up by KAS...add value to damagemultiplier
        missile->damage*=ship->damageModifier;
    }

    missile->missileID.missileNumber = universe.missileNumber;
    if (multiPlayerGame) IDToPtrTableAdd(&MissileIDToPtr,universe.missileNumber,(SpaceObj *)missile);
    universe.missileNumber++;

    // now fill in missile->posinfo

    // heading
    matMultiplyMatByVec(&gunheadingInWorldCoordSys,&ship->rotinfo.coordsys,&gun->gunheading);

    // position
    matMultiplyMatByVec(&positionInWorldCoordSys,&ship->rotinfo.coordsys,&gunstatic->position);

    vecAdd(missile->posinfo.position,positionInWorldCoordSys,ship->posinfo.position);
    missile->posinfo.position.x += gunheadingInWorldCoordSys.x * gunstatic->barrelLength;
    missile->posinfo.position.y += gunheadingInWorldCoordSys.y * gunstatic->barrelLength;
    missile->posinfo.position.z += gunheadingInWorldCoordSys.z * gunstatic->barrelLength;

    missile->posinfo.isMoving = TRUE;
    missile->posinfo.haventCalculatedDist = TRUE;

    vecScalarMultiply(missile->posinfo.velocity,gunheadingInWorldCoordSys,gunstatic->bulletspeed);

    // now fill in missile->rotinfo
    if (gunstatic->guntype == GUN_MineLauncher)
    {   //mine launcher so fix mine rotation depending on gun launched from...and race?
        //hard coded fix to a special exception :)

        if(ship->cataclysmGunInfo->numGuns > 1)
        {    //race 1 ship..do rotation..later make global LOD on this..since its a waste of time..really
            //build mine coordinate system from the ships...BIG savings...
            if(&ship->cataclysmGunInfo->guns[0] == gun)
            {
                matGetVectFromMatrixCol1(mineright,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol2(mineup,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol3(minehead,ship->rotinfo.coordsys);
                vecScalarMultiply(mineup,mineup,-1.0f);
            }
            else
            {
                matGetVectFromMatrixCol1(mineright,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol2(mineup,ship->rotinfo.coordsys);
                matGetVectFromMatrixCol3(minehead,ship->rotinfo.coordsys);
                vecScalarMultiply(mineright,mineright, -1.0f);
            }
            matPutVectIntoMatrixCol1(mineup,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol2(mineright,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol3(minehead,missile->rotinfo.coordsys);
        }
        else
        {
            // LM: what is this stranded code doing?
            matGetVectFromMatrixCol1(mineup,ship->rotinfo.coordsys);
            matGetVectFromMatrixCol2(mineright,ship->rotinfo.coordsys);
            matGetVectFromMatrixCol3(minehead,ship->rotinfo.coordsys);
            matPutVectIntoMatrixCol1(mineup,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol2(mineright,missile->rotinfo.coordsys);
            matPutVectIntoMatrixCol3(minehead,missile->rotinfo.coordsys);
            //            matCreateCoordSysFromHeading(&missile->rotinfo.coordsys,&gunheadingInWorldCoordSys);
        }
    }
    else
    {
        matCreateCoordSysFromHeading(&missile->rotinfo.coordsys,&gunheadingInWorldCoordSys);
    }

    univUpdateObjRotInfo((SpaceObjRot *)missile);

    if ((target != NULL) && (target->collMyBlob != NULL))
    {
        collAddSpaceObjToSpecificBlob(target->collMyBlob,(SpaceObj *)missile);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)missile);
    }
    listAddNode(&universe.SpaceObjList,&(missile->objlink),missile);
    listAddNode(&universe.ImpactableList,&(missile->impactablelink),missile);
    listAddNode(&universe.MissileList,&(missile->missilelink),missile);

    univAddObjToRenderListIf((SpaceObj *)missile,(SpaceObj *)ship);     // add missile to render list if parent ship is in render list

//    if (gunstatic->burstFireTime == 0.0f)
    {
        soundEventPlay(ship, Gun_WeaponShot, gun);
    }

    //muzle flash effect trigger and set collision event
    missile->hitEffect = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunHit];//get pointer to bullet effect
//    missile->hitEffect = etgGunEventTable[shiprace][gunstatic->gunsoundtype][EGT_GunHit];//just get pointer from parent gun
    etgLOD = etgGunEventTable[shipstatic->shiprace][gunstatic->gunsoundtype][EGT_GunFire];//get pointer to bullet effect
    if (etgLOD != NULL)
    {
        LOD = ship->currentLOD;
        if (LOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[LOD];
        }
    }
    else
    {
        stat = NULL;
    }
//    stat = etgGunEventTable[shiprace][gunstatic->gunsoundtype][EGT_GunFire];//get gun effect

    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgFireEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgFireEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {                                                       //if there is a gun fire effect
            matCreateCoordSysFromHeading(&newCoordsys,&gunheadingInWorldCoordSys);
            floatDamage = (real32)missile->damage;
            if (RGLtype == SWtype)
            {                                                   //smaller muzzle flashes in software
                floatDamage *= etgSoftwareScalarFire;
            }
            floatDamage *= ship->magnitudeSquared;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, ship, &missile->posinfo.position, NULL, &newCoordsys, 1.0f, EAF_Velocity | EAF_NLips, 1, intDamage);
        }
    }
    if(bitTest(ship->flags,SOF_CloakGenField))
    {   //dang decloakage...
        bitSet(ship->flags,SOF_DeCloaking);
        if(bitTest(ship->flags,SOF_Cloaked))
        {
            bitClear(ship->flags,SOF_Cloaked);
            ship->shipDeCloakTime = universe.totaltimeelapsed;
        }
        bitClear(ship->flags,SOF_Cloaking);
    }
    if (target != NULL && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
}


/*-----------------------------------------------------------------------------
    Name        : gunRecoilTableCompute
    Description : Computes and fills in the gun recoil motion table
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunRecoilTableCompute(void)
{
    sdword index;
    real32 timeIndex, value;

    for (index = 0, timeIndex = 0.0f; timeIndex < GUN_RecoilTime;
         index++, timeIndex += 1.0f / (real32)GUN_RecoilTableLength)
    {
        //y = 1 - ((x - .05) * 20)^2
        value = (timeIndex - GUN_RecoilTime) * GUN_OneOverTime;
        gunRecoilTable[index] = 1.0f - (value * value);
    }
    for (; index < GUN_RecoilTableLength; index++, timeIndex += 1.0f / (real32)GUN_RecoilTableLength)
    {
        //y = ((-atan((x-.4)*8)/PI+.5-.06538)/.825433
        gunRecoilTable[index] = (((real32)-atan((timeIndex-0.4f)*8.0f)/PI+0.5f)-0.06538f)/0.825433f;
    }
    gunRecoilTable[GUN_RecoilTableLength] = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : gunRecoilVectorCompute
    Description : Compute a vector representing a gun's recoil as a function
                    of time between shots.
    Inputs      : dest - heading of gun mesh
                  recoilLength - length of the gun's recoil, in meters
                  lastFired - absolute time of the last gun shot
                  rateOfFire - time between shots
    Outputs     : dest - recoil vector to be added to position
    Return      :
----------------------------------------------------------------------------*/
void gunRecoilVectorCompute(vector *dest, real32 recoilLength, real32 lastFired, real32 rateOfFire)
{
    real32 timeElapsed, offset;
    sdword index;

    timeElapsed = universe.totaltimeelapsed - lastFired;
    if (timeElapsed > rateOfFire)
    {                                                       //if the gun is sitting idle
        dest->x = dest->y = dest->z = 0.0f;                 //no recoil at all
        return;
    }
    timeElapsed /= rateOfFire;                              //normalize time elapsed from 0 to 1

    index = (sdword)(timeElapsed * (real32)GUN_RecoilTableLength);
    offset = gunRecoilTable[index];                         //table-lookup for gun recoil curve
    offset *= -recoilLength;
    dest->x *= offset;
    dest->y *= offset;
    dest->z *= offset;
}

/*-----------------------------------------------------------------------------
    Name        : gunShoot
    Description : shoots a given gun of the given ship
    Inputs      : ship  - that which owns gun
                  gun - gun to fire
                  target - object being shot at or NULL
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunShoot(Ship *ship, Gun *gun, SpaceObjRotImpTarg *target)
{
    GunStatic *gunstatic = gun->gunstatic;
    switch (gunstatic->guntype)
    {
        case GUN_MissileLauncher:
        case GUN_MineLauncher:
            missileShoot(ship, gun, target);
            break;
        default:
            gunShootGunAtTarget(ship, gun, target);
            break;
    }
}

Bullet *gunShootGunAtTarget(Ship *ship, Gun *gun, SpaceObjRotImpTarg *target)
{
    GunStatic *gunstatic = gun->gunstatic;
    real32 damage;
    Ship *targetShip = NULL;
    vector gunDirection;
    vector gunPosition;

    if (ship->shiptype == dRKO &&
        target != NULL &&
        target->objtype == OBJ_ShipType &&
        ship->beastAttack.sourceShip == (Ship *)target)
    {
        return NULL;
    }

    if (target != NULL && target->objtype == OBJ_ShipType)
    {
        targetShip = (Ship *)target;
        if (ship->staticinfo->shipclass <= CLASS_Frigate &&
            gunGetActiveSoundType(ship, gun) != GS_BigGun)
        {
            matMultiplyMatByVec(&gunDirection, &ship->rotinfo.coordsys, &gun->gunheading);
            matMultiplyMatByVec(&gunPosition, &ship->rotinfo.coordsys, &gunstatic->position);
            vecAddTo(gunPosition, ship->posinfo.position);

            if (!collCheckLineOfSight(ship, targetShip, &gunPosition, &gunDirection))
            {
                return NULL;
            }
        }
    }

    if (!ship->shipisattacking)
    {
        ship->shipisattacking = TRUE;
    }
    if (ship->attackvars.myWingmanIs != NULL && !ship->attackvars.myWingmanIs->shipisattacking)
    {
        ship->attackvars.myWingmanIs->shipisattacking = TRUE;
    }
    if (bitTest(ship->specialFlags1C8, CATA_RUNTIME_GRAVWELL_CAPTURED))
    {
        return NULL;
    }

    damage = gunstatic->damageMult * tacticsGetShipDamageModifier(ship);
    if (bitTest(ship->specialFlags, SPECIAL_KasSpecialDamageModifier))
    {
        damage *= ship->damageModifier;
    }
    if (target != NULL)
    {
        damage *= univGetDamageMultiplier(gunGetActiveSoundType(ship, gun), gunGetTargetArmorClass(target));
    }

    if (targetShip != NULL && targetShip->shiptype == sLeech)
    {
        ApplyDamageToTarget((SpaceObjRotImpTarg *)targetShip, 10.0f, gunGetActiveSoundType(ship, gun), 666, ship->playerowner->playerIndex);
    }

    return gunCreateBullet(ship, gun, NULL, target, damage);
}

Bullet *gunCreateBullet(Ship *ship, Gun *gun, SpaceObjRotImpTarg *sourceShip, SpaceObjRotImpTarg *target, real32 damage)
{
    Bullet *bullet;
    GunStatic *gunstatic = gun->gunstatic;
    GunSoundType activeSoundType = gunGetActiveSoundType(ship, gun);
    WeaponType activeWeaponMode = gunGetActiveWeaponMode(ship, gun);
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    etgeffectstatic *stat;
    etglod *etgLOD;
    sdword LOD;
    real32 floatDamage;
    real32 bulletspeed;
    udword intDamage;
    udword intVelocity;
    udword intLength;
    vector offset0;
    vector offset1;
    vector gunheadingInWorldCoordSys;
    vector positionInWorldCoordSys;

    gun->lasttimefired = universe.totaltimeelapsed;
    bullet = memAlloc(sizeof(Bullet), "Bu(Bullet)", Pyrophoric);
    bullet->objtype = OBJ_BulletType;
    bullet->flags = 0;
    bullet->flags2 = 0;
    bullet->unk_1C = 0;
    bullet->staticinfo = NULL;
    ClearNode(bullet->renderlink);
    memClearDword(bullet->unk_3C, 0, sizeof(bullet->unk_3C) / sizeof(udword));
    bullet->currentLOD = ship->currentLOD;
    bullet->collMyBlob = NULL;
    bullet->cameraDistanceSquared = ship->cameraDistanceSquared;
    bullet->damageFull = bullet->damage = damage;
    bullet->bulletType = (BulletType)activeWeaponMode;
    bullet->soundType = activeSoundType;
    bullet->cataUnknown10C = bitTest(ship->specialFlags1C8, BIT6);
    bullet->owner = ship;
    bullet->playerowner = ship->playerowner;
    bullet->gunowner = gun;
    bullet->target = bullet->cataUnknown10C ? NULL : target;
    bullet->bulletColor = etgBulletColor[shipstatic->shiprace][activeSoundType];
    bullet->bulletmass = gunstatic->bulletmass;
    bullet->lengthmag = gunstatic->bulletlength;
    bullet->timelived = 0.0f;
    bullet->SpecialEffectFlag = 0;
    bullet->DFGFieldEntryTime = 0.0f;
    bullet->BulletSpeed = 0.0f;

    bullet->totallifetime = gunGetActiveBulletLifetime(ship, gun);

    memset(&(bullet->posinfo), 0, sizeof(bullet->posinfo));
    if (sourceShip != NULL && target != NULL)
    {
        vecSub(gunheadingInWorldCoordSys, target->posinfo.position, sourceShip->posinfo.position);
        if (!vecIsZero(gunheadingInWorldCoordSys))
        {
            vecNormalize(&gunheadingInWorldCoordSys);
        }
    }
    else
    {
        matMultiplyMatByVec(&gunheadingInWorldCoordSys, &ship->rotinfo.coordsys, &gun->gunheading);
    }

    bullet->bulletheading = gunheadingInWorldCoordSys;
    vecScalarMultiply(bullet->lengthvec, gunheadingInWorldCoordSys, gunstatic->bulletlength);
    matCreateCoordSysFromHeading(&bullet->rotinfo.coordsys, &gunheadingInWorldCoordSys);

    switch (activeWeaponMode)
    {
        case WEAPON_Ion:
            bulletspeed = 0.0f;
            vecZeroVector(bullet->posinfo.velocity);
            bullet->traveldist = gunstatic->bulletlength;
            bullet->beamtraveldist = gunstatic->bulletlength;
            break;

        case WEAPON_Laser:
        case WEAPON_BeastInfection:
            bulletspeed = 0.0f;
            vecZeroVector(bullet->posinfo.velocity);
            bullet->traveldist = gunstatic->bulletlength;
            bullet->beamtraveldist = gunstatic->bulletlength;
            break;

        default:
            bulletspeed = gunGetActiveBulletSpeed(ship, gun);
            vecScalarMultiply(bullet->posinfo.velocity, gunheadingInWorldCoordSys, bulletspeed);
            bullet->traveldist = bulletspeed * universe.unknown1;
            break;
    }
    if (sourceShip != NULL)
    {
        bullet->posinfo.position = ship->posinfo.position;
    }
    else
    {
        matMultiplyMatByVec(&positionInWorldCoordSys, &ship->rotinfo.coordsys, &gunstatic->position);
        if (gunstatic->offset.x != 0.0f)
        {
            matMultiplyMatByVec(&offset0, &gun->curgunorientation, &gunstatic->offset);
            matMultiplyMatByVec(&offset1, &ship->rotinfo.coordsys, &offset0);
            vecAdd(positionInWorldCoordSys, positionInWorldCoordSys, offset1);
        }
        vecAdd(bullet->posinfo.position, positionInWorldCoordSys, ship->posinfo.position);
        bullet->posinfo.position.x += gunheadingInWorldCoordSys.x * gunstatic->barrelLength;
        bullet->posinfo.position.y += gunheadingInWorldCoordSys.y * gunstatic->barrelLength;
        bullet->posinfo.position.z += gunheadingInWorldCoordSys.z * gunstatic->barrelLength;
    }

    bullet->posinfo.isMoving = TRUE;
    bullet->posinfo.haventCalculatedDist = TRUE;

    if (target != NULL && target->collMyBlob != NULL)
    {
        collAddSpaceObjToSpecificBlob(target->collMyBlob, (SpaceObj *)bullet);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)bullet);
    }
    listAddNode(&universe.SpaceObjList, &(bullet->objlink), bullet);
    listAddNode(&universe.BulletList, &(bullet->bulletlink), bullet);
    univAddObjToRenderListIf((SpaceObj *)bullet, (SpaceObj *)ship);

    if (activeSoundType != GS_BigGun && activeSoundType != GS_BeastWeapon)
    {
        if (activeWeaponMode == WEAPON_Ion)
        {
            soundEventPlay(ship, Gun_WeaponFireLooped, gun);
        }
        else
        {
            soundEventPlay(ship, Gun_WeaponShot, gun);
        }
    }

    floatDamage = (real32)bullet->damage;
    floatDamage *= ship->magnitudeSquared;
    intDamage = TreatAsUdword(floatDamage);
    intVelocity = TreatAsUdword(bulletspeed);
    intLength = TreatAsUdword(gunstatic->bulletlength);
    etgLOD = etgGunEventTable[shipstatic->shiprace][activeSoundType][EGT_GunBullet];
    if (etgLOD != NULL)
    {
        if (target != NULL)
        {
            LOD = min(ship->currentLOD, target->currentLOD);
        }
        else
        {
            LOD = ship->currentLOD;
        }
        stat = (LOD >= etgLOD->nLevels) ? NULL : etgLOD->level[LOD];
    }
    else
    {
        stat = NULL;
    }
    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgBulletEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgBulletEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {
            if (activeWeaponMode == WEAPON_Ion)
            {
                bullet->effect = etgEffectCreate(stat,
                                                 (Ship *)bullet,
                                                 NULL,
                                                 &ship->posinfo.velocity,
                                                 NULL,
                                                 1.0f,
                                                 EAF_AllButNLips,
                                                 5,
                                                 (udword)sourceShip,
                                                 (udword)ship,
                                                 (udword)gun,
                                                 (udword)ship->shiptype,
                                                 TreatAsUdword(gunstatic->barrelLength));
            }
            else
            {
                bullet->effect = etgEffectCreate(stat,
                                                 (Ship *)bullet,
                                                 NULL,
                                                 &ship->posinfo.velocity,
                                                 NULL,
                                                 1.0f,
                                                 EAF_AllButNLips,
                                                 3,
                                                 intDamage,
                                                 intVelocity,
                                                 intLength);
            }
        }
        else
        {
            bullet->effect = NULL;
        }
    }
    else
    {
        bullet->effect = NULL;
    }

    bullet->hitEffect = etgGunEventTable[shipstatic->shiprace][activeSoundType][EGT_GunHit];
    etgLOD = etgGunEventTable[shipstatic->shiprace][activeSoundType][EGT_GunFire];
    if (etgLOD != NULL)
    {
        LOD = ship->currentLOD;
        stat = (LOD >= etgLOD->nLevels) ? NULL : etgLOD->level[LOD];
    }
    else
    {
        stat = NULL;
    }

    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgFireEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgFireEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {
            if (RGLtype == SWtype)
            {
                floatDamage *= etgSoftwareScalarFire;
            }
            etgEffectCreate(stat, ship, &bullet->posinfo.position, &ship->posinfo.velocity, &bullet->rotinfo.coordsys, 1.0f, EAF_Velocity, 1, intDamage);
        }
    }
    if (bitTest(ship->flags, SOF_CloakGenField))
    {
        bitSet(ship->flags, SOF_DeCloaking);
        if (bitTest(ship->flags, SOF_Cloaked))
        {
            bitClear(ship->flags, SOF_Cloaked);
            ship->shipDeCloakTime = universe.totaltimeelapsed;
        }
        bitClear(ship->flags, SOF_Cloaking);
    }
    if (target != NULL && target->objtype == OBJ_ShipType)
    {
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
    if (activeSoundType == GS_BigGun)
    {
        pingAnomalyObjectPingAdd("BulletBigGun", (SpaceObj *)bullet);
    }

    TreatAsReal32(bullet->cataUnknown150[2]) = bullet->posinfo.position.x;
    bullet->cataUnknown150[3] = TreatAsUdword(bullet->posinfo.position.y);
    TreatAsReal32(bullet->cataUnknown150[4]) = bullet->posinfo.position.z;

    return bullet;
}
/*-----------------------------------------------------------------------------
    Name        : gunShootGunsAtTarget
    Description : shoots a ship's guns at target
    Inputs      : ship, target, range, [trajectory] (trajectory required for ships with fixed guns)
    Outputs     :
    Return      :
    Note        : trajectory can be passed in as NULL for ships that don't have
                  any fixed guns.  trajectory is required only for ships that have
                  fixed guns.
----------------------------------------------------------------------------*/
bool32 gunShootGunsAtTarget(Ship *ship,SpaceObjRotImpTarg *target,real32 range,vector *trajectory)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    Gun *gun;
    sdword i;
    vector shipheading;
    bool32 shotguns = FALSE;
    GunStatic *gunstatic;
    real32 dotprod;
    real32 triggerHappy;

    if (gunInfo == NULL)
    {
//        dbgMessagef("\nWARNING: no guns on ship %s %s, tried to shoot",ShipRaceToStr(ship->shiprace),ShipTypeToStr(ship->shiptype));
        return shotguns;
    }

    numGuns = gunInfo->numGuns;

    for (i=0;i<numGuns;i++)
    {
        gun = &gunInfo->guns[i];
        gunstatic = gun->gunstatic;
        if (bitTest(ship->flags, SOF_NISShip))
        {
            triggerHappy = NIS_TriggerHappyAngle;
        }
        else
        {
            triggerHappy = gunstatic->triggerHappy;
        }

        if(ship->staticinfo->madStatic != NULL)
        {
            //ship has meshanimations
            if(ship->staticinfo->madStatic->numGunOpenIndexes > 0)
            {
                //has gun opening animations
                if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN &&
                    ship->madGunStatus != MAD_STATUS_GUNS_OPENING)
                {
                    madOpenGunsShip(ship);
                    return shotguns;
                }
                else if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN)
                {
                    return shotguns;
                }
                //otherwise, we're open..try to fire away!
            }
        }

        if (range < gunstatic->bulletrange)
        {
            if (ship->crystalProcessingState.active == 1 ||
                gunstatic->gunsoundtype == GS_BigGun ||
                gunstatic->gunsoundtype == GS_BeastWeapon)
            {
                continue;
            }

            bool32 canShoot = gunCanShootStaggered(ship, gun);
            if (canShoot)
            {
                switch (gunstatic->guntype)
                {
                    case GUN_MissileLauncher:
                        if (gunHasMissiles(gun))
                        {
                            matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);
                            dotprod = vecDotProduct(*trajectory,shipheading);

                            if (dotprod >= triggerHappy &&
                                ship->shiptype != sAcolyte &&
                                ship->shiptype != bAcolyte &&
                                ship->shiptype != dAcolyte)
                            {
                                missileShoot(ship,gun,target);
                                shotguns = TRUE;
                            }
                        }
                        break;
                    case GUN_MineLauncher:
                        if (gunHasMissiles(gun))
                        {
                            matGetVectFromMatrixCol3(shipheading,ship->rotinfo.coordsys);
                            dotprod = vecDotProduct(*trajectory,shipheading);

                            if (dotprod >= triggerHappy)
                            {
                                missileShoot(ship,gun,target);
                                shotguns = TRUE;
                            }
                        }
                    break;
                    case GUN_Fixed:
                        if (gunCanHitTarget(ship, gun, target, TRUE))
                        {
                            gunShootGunAtTarget(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    case GUN_Gimble:
                    case GUN_NewGimble:
                    {
                        bool32 oriented = gunOrientGimbleGun(ship,gun,target);
                        if (oriented)
                        {
                            gunShootGunAtTarget(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }
/*
    if (shotguns && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
    */
    return shotguns;
}

/*-----------------------------------------------------------------------------
    Name        : gunShootGunsAtMultipleTargets
    Description : shoots a ship's guns at multiple targets
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 gunShootGunsAtMultipleTargets(Ship *ship)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns = gunInfo->numGuns;
    Gun *gun;
    sdword i;
    bool32 shotguns = FALSE;
    GunStatic *gunstatic;
    AttackTargets *attackTargets = ship->attackvars.multipleAttackTargets;
    SpaceObjRotImpTarg *target;

    dbgAssert(attackTargets->numAttackTargets == numGuns);      // one attack target for each gun

    for (i=0;i<numGuns;i++)
    {
        gun = &gunInfo->guns[i];
        gunstatic = gun->gunstatic;

        if (gunstatic->gunsoundtype == GS_BigGun ||
            gunstatic->gunsoundtype == GS_BeastWeapon)
        {
            continue;
        }

        if(ship->staticinfo->madStatic != NULL)
        {
            //ship has meshanimations
            if(ship->staticinfo->madStatic->numGunOpenIndexes > 0)
            {
                //has gun opening animations
                if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN &&
                    ship->madGunStatus != MAD_STATUS_GUNS_OPENING)
                {
                    madOpenGunsShip(ship);
                    return shotguns;
                }
                else if(ship->madGunStatus != MAD_STATUS_GUNS_OPEN)
                {
                    return shotguns;
                }
                //otherwise, we're open..try to fire away!
            }
        }

        if (gunCanShootStaggered(ship, gun))
        {
            target = attackTargets->TargetPtr[i];

            if (target != NULL)
            {
                if (ship->crystalProcessingState.active == 1)
                {
                    continue;
                }

                switch (gunstatic->guntype)
                {
                    case GUN_MissileLauncher:
                        if (gunHasMissiles(gun))
                        {
                            missileShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;
                    case GUN_MineLauncher:
                        if (gunHasMissiles(gun))
                        {
                            missileShoot(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;
                    case GUN_Fixed:
                        if (gunCanHitTarget(ship, gun, target, TRUE))
                        {
                            gunShootGunAtTarget(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    case GUN_Gimble:
                    case GUN_NewGimble:
                        if (gunOrientGimbleGun(ship,gun,target))
                        {
                            gunShootGunAtTarget(ship,gun,target);
                            shotguns = TRUE;
                        }
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }
            }
        }
        else if (gunstatic->slaveDriver >= 0)
        {
            gunUpdateSlave(gun, gunInfo, gunstatic->slaveDriver);
        }
    }
/*
    if (shotguns && target->objtype == OBJ_ShipType)
    {                                                       //if we succeeded in shooting at enemy
        ((Ship *)target)->firingAtUs = ship;
        ((Ship *)target)->recentlyFiredUpon = RECENT_ATTACK_DURATION;
    }
*/
    return shotguns;
}

/*-----------------------------------------------------------------------------
    Name        : gunMatrixUpdate
    Description : Binding matrix callback for updating gun mesh positions.
    Inputs      : flags - flags indicating certain attributes (see mesh.h)
                  startMatrix - starting matrix (from .geo file)
                  matrix - matrix to store result in.
                  data - pointer to Gun structure
                  ID - pointer to ship structure
    Outputs     : computes and fills in the gun matrix.
    Return      : true = matrix updated
----------------------------------------------------------------------------*/
bool32 gunMatrixUpdate(udword flags, hmatrix *startMatrix, hmatrix *matrix, void *data, sdword ID)
{
    Ship *ship;
    Gun *gun;
    GunStatic *gunstatic;
    bool32 bRecoil;
    vector recoilVector;

    ship = (Ship *)ID;
    dbgAssert(ship->objtype == OBJ_ShipType);
    gun = (Gun *)data;
    gunstatic = gun->gunstatic;

    bRecoil = bitTest(flags, HBF_Recoil);
    flags &= (HBF_Declination|HBF_Azimuth);

    switch (gunstatic->guntype)
    {
        case GUN_NewGimble:
            if (flags == (HBF_Declination|HBF_Azimuth))
            {
                if (bitTest(gunstatic->flags, GF_MultiLevelMatrix))
                {
                    hmatMakeHMatFromMat(matrix,&gun->curGunOrientationNonConcat);
                }
                else
                {
                    hmatMakeHMatFromMat(matrix,&gun->curgunorientation);
                }
                matrix->m14 = startMatrix->m14;
                matrix->m24 = startMatrix->m24;
                matrix->m34 = startMatrix->m34;
                break;
            }
            else if (flags == HBF_Azimuth)
            {
                hmatrix rotmatrix;
                real32 angle = gun->angle;
                hmatMakeRotAboutX(&rotmatrix,(real32)cos(angle),(real32)sin(angle));
                hmatMultiplyHMatByHMat(matrix, startMatrix, &rotmatrix);
                break;
            }
            else if (flags == HBF_Declination)
            {
                hmatrix rotmatrix;
                real32 declination = gun->declination;
                hmatMakeRotAboutY(&rotmatrix,(real32)cos(declination),(real32)sin(declination));
                hmatMultiplyHMatByHMat(matrix, startMatrix, &rotmatrix);
                break;
            }
            else
            {
//                dbgAssert(FALSE);
//                return FALSE;
                *matrix = IdentityHMatrix;
                matrix->m14 = startMatrix->m14;
                matrix->m24 = startMatrix->m24;
                matrix->m34 = startMatrix->m34;
                break;
            }

        default:
            dbgAssert(FALSE);
            return FALSE;
    }
    //add recoil, if any
    if (bRecoil)
    {
        hmatGetVectFromHMatrixCol3(recoilVector, *matrix);
        gunRecoilVectorCompute(&recoilVector, gunstatic->recoilLength, gun->lasttimefired, gunstatic->firetime);
//        vecAddTo(bullet->posinfo.position, offset0);
        matrix->m14 += recoilVector.x;
        matrix->m24 += recoilVector.y;
        matrix->m34 += recoilVector.z;
    }


    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : gunDrawGunInfo
    Description : draws gun debug information
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VISUALIZATION
void gunDrawGunInfo(Ship *ship)
{
    sdword i;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    Gun *gun;
    GunStatic *gunstatic;
    vector to, from, offset0;

    if (gunInfo == NULL)
    {
        return;
    }

    numGuns = gunInfo->numGuns;

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    for (i=0,gun=&gunInfo->guns[0];i<numGuns;i++,gun++)
    {
        gunstatic = gun->gunstatic;
        from = gun->gunstatic->position;
        vecScalarMultiply(to, gun->gunheading,500.0f);
        vecAddTo(to, from);
        // heading

        //gun offset
        if (gunstatic->offset.x != 0.0f)
        {                                                       //if there is an offset vector
            matMultiplyMatByVec(&offset0, &gun->curgunorientation, &gunstatic->offset);//offset in gun space
            vecAddTo(from, offset0);                            //add to position
            vecAddTo(to, offset0);                            //add to position
        }
        //add barrel length
        from.x += gun->gunheading.x * gunstatic->barrelLength;
        from.y += gun->gunheading.y * gunstatic->barrelLength;
        from.z += gun->gunheading.z * gunstatic->barrelLength;
        to.x += gun->gunheading.x * gunstatic->barrelLength;
        to.y += gun->gunheading.y * gunstatic->barrelLength;
        to.z += gun->gunheading.z * gunstatic->barrelLength;
        primLine3(&from, &to, colFuscia/*colRGB(10, 255, 100)*/);
    }

    rndLightingEnable(TRUE);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : gunFirePower
    Description : Compute the firepower of a gun for a given tactics setting.
    Inputs      : gunStatic - static info of the gun
                  tactic - tactics setting to determine damage for
    Outputs     : fireTime - the computed rate of fire.  Not set for guns of type BULLET_Beam.
    Return      :
----------------------------------------------------------------------------*/
real32 gunFirePower(GunStatic *gunStatic, TacticsType tactics, real32 *fireTime)
{
    dbgAssert(tactics == Evasive || tactics == Neutral || tactics == Aggressive);

    if (gunStatic->firetime == 0.0f)
    {
        *fireTime = UNIVERSE_UPDATE_PERIOD;
    }
    else
    {
        *fireTime = gunStatic->firetime;
    }
    if (gunSoundTypeToBulletType(gunStatic->gunsoundtype) == BULLET_Beam)
    {
        return((gunStatic->gunDamageLo[tactics] + gunStatic->gunDamageHi[tactics]) / 2.0f * (real32)UNIVERSE_UPDATE_RATE * gunStatic->bulletlifetime / *fireTime);
    }
    else if(gunStatic->burstFireTime != 0.0f)
    {
        //burst fire gun..add aditional factor of:
        //burstfiretime/(burstfiretime_burstwaittime)
        return(((gunStatic->gunDamageLo[tactics] + gunStatic->gunDamageHi[tactics]) / 2.0f / *fireTime)*(gunStatic->burstFireTime/(gunStatic->burstFireTime+gunStatic->burstWaitTime)));
    }
    return((gunStatic->gunDamageLo[tactics] + gunStatic->gunDamageHi[tactics]) / 2.0f / *fireTime);
}

/*-----------------------------------------------------------------------------
    Name        : gunShipFirePower
    Description : Compute the firepower for a given ship at some tactics setting
    Inputs      : info - staticinfo of ship
                  tactics - what tactic to compute damage for
    Outputs     :
    Return      : total damage of the ship
----------------------------------------------------------------------------*/
#define GUN_NumberInterceptorTypes      7
ubyte gunInterceptorTypes[GUN_NumberInterceptorTypes] = {sRecon, kHeavyInterceptor, bCloakedFighter, p1Fighter, sSwarmer, sAcolyte, TargetDrone};
real32 gunShipFirePower(ShipStaticInfo *info, TacticsType tactics)
{
    sdword index;
    real32 power = 0.0f, powerThisGun, totalFireRate, fireRate;
    GenericInterceptorStatics *interceptorstat;

    if (memchr(gunInterceptorTypes, info->shiptype, GUN_NumberInterceptorTypes))
    {                                                       //interceptors have a different rate of fire computation
        interceptorstat = (GenericInterceptorStatics *)info->custstatinfo;
        totalFireRate = interceptorstat->firetime;
        if (info->gunStaticInfo != NULL)
        {
            for (index = 0; index < info->gunStaticInfo->numGuns; index++)
            {
                powerThisGun = gunFirePower(&info->gunStaticInfo->gunstatics[index], tactics, &fireRate);
                powerThisGun *= fireRate / (totalFireRate * (real32)info->gunStaticInfo->numGuns);
                power += powerThisGun;
            }
        }
    }
    else
    {
        if (info->gunStaticInfo != NULL)
        {
            for (index = 0; index < info->gunStaticInfo->numGuns; index++)
            {
                power += gunFirePower(&info->gunStaticInfo->gunstatics[index], tactics, &fireRate);
            }
        }
    }
    return(power);
}

/*-----------------------------------------------------------------------------
    Name        : gunStartup
    Description : Start the gun module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunStartup(void)
{
    gunRecoilTableCompute();
}

/*-----------------------------------------------------------------------------
    Name        : gunShutdown
    Description : Shut down the gun module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gunShutdown(void)
{
    ;
}

#if GUN_TUNE_MODE

bool32 gunTuningMode = FALSE;
sdword tuningGun = 0;

void gunTuneGun(Ship *ship)
{
    ShipStaticInfo *shipstatic = ship->staticinfo;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    Gun *gun;
    GunStatic *gunstatic;
    bool32 updateguncoordsys = FALSE;

    if (gunInfo == NULL)
    {
        return;
    }

    if (gunInfo->numGuns == 0)
    {
        return;
    }

    if (tuningGun >= gunInfo->numGuns)
    {
        tuningGun = 0;
    }

    gun = &gunInfo->guns[tuningGun];
    gunstatic = gun->gunstatic;

    switch (gunstatic->guntype)
    {
        case GUN_NewGimble:
            if (keyIsHit(ARRLEFT))
            {
                gun->angle -= GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }

            if (keyIsHit(ARRRIGHT))
            {
                gun->angle += GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }

            if (keyIsHit(ARRUP))
            {
                gun->declination += GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }

            if (keyIsHit(ARRDOWN))
            {
                gun->declination -= GUNTUNE_SPEED;
                updateguncoordsys = TRUE;
            }
            if (keyIsHit(NUMPAD0))
            {
                gun->angle = 0.0f;
                gun->declination = 0.0f;
                updateguncoordsys = TRUE;
            }

            if (updateguncoordsys)
            {
                if (gun->angle < gunstatic->minturnangle)
                {
                    gun->angle = gunstatic->minturnangle;
                }

                if (gun->angle > gunstatic->maxturnangle)
                {
                    gun->angle = gunstatic->maxturnangle;
                }

                if (gun->declination < gunstatic->mindeclination)
                {
                    gun->declination = gunstatic->mindeclination;
                }

                if (gun->declination > gunstatic->maxdeclination)
                {
                    gun->declination = gunstatic->maxdeclination;
                }

                gunNewGimbleUpdateCoordSys(gun,gunstatic);
            }

            {
                char buffer[300];
                sprintf(buffer,"Gun %d %s Angle:%4.2f Declin:%4.2f Heading:%2.3f %2.3f %2.3f %d",tuningGun,GunTypeToStr(gunstatic->guntype),
                           RAD_TO_DEG(gun->angle),RAD_TO_DEG(gun->declination),gun->gunheading.x,gun->gunheading.y,gun->gunheading.z,universe.univUpdateCounter);

                strcat(buffer,"\n");
                fontPrint(0, 400, colWhite, buffer);
                //dbgMessage(buffer);
            }
            break;

        default:
            fontPrintf(0, 400, colWhite,
                       "Gun %d %s",tuningGun,GunTypeToStr(gunstatic->guntype));
            break;
    }
}
#endif



#if 0  // Cat2: Binary gunStartup; live implementation at gun.c L2007 (0x48CCA0)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x48CCA0
void gunStartup(void)
{
double v0; // st7
int v1; // edx
double v2; // st6
float *v3; // ecx
int v4; // eax

v0 = ZERO;
v1 = 0;
v2 = ZERO;
v3 = flt_C38F20;
do
{
++v1;
*v3++ = ONE - (v2 - flt_72A654) * BeastSelfDamageTick * ((v2 - flt_72A654) * BeastSelfDamageTick);
v2 = v2 + flt_72AB34;
}
while ( v2 < flt_72A654 );
if ( v1 < 255 )
{
v4 = 4 * v1 + 12816160;
do
{
v4 += 4;
*(float *)(v4 - 4) = (ZERO_POINT_FIVE - atan2((v2 - flt_72A674) * flt_72A558, 1.0) * flt_72AB30 - flt_72AB2C)
* flt_72AB28;
v2 = v2 + flt_72AB34;
}
while ( v4 < (int)&flt_C3931C );
}
flt_C3931C = v0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48AAC0
void gunShoot(Ship *ship, Gun *gun, SpaceObjRotImpTarg *target)
{
  bool v4; // zf
  float *result; // eax
  char *v7; // esi
  int v8; // eax
  _DWORD *v9; // ecx
  int v10; // eax
  _DWORD *v11; // ecx
  int v12; // eax
  _DWORD *v13; // ecx
  _DWORD *v14; // eax
  int v15; // ecx
  int v16; // eax
  int v17; // edi
  int v18; // edx
  float *v19; // edi
  int v20; // eax
  _DWORD *v21; // ecx
  int v22; // edi
  int v23; // ecx
  double v24; // st7
  int v25; // ecx
  int v26; // eax
  double v27; // st7
  int v28; // ecx
  int v29; // eax
  double v30; // st7
  int v31; // ecx
  int v32; // eax
  double v33; // st7
  int *v34; // eax
  double v35; // st7
  float v36; // eax
  float v37; // ecx
  int v38; // edx
  float v39; // eax
  double v40; // st6
  int v41; // eax
  float v42; // edx
  int v43; // eax
  int v44; // ecx
  int v45; // edx
  float v46; // eax
  float v47; // ecx
  int v48; // edx
  int v49; // eax
  int v50; // ecx
  double v51; // st7
  float v52; // eax
  int v53; // ecx
  int v54; // edx
  int v55; // ecx
  int v56; // edx
  int v57; // eax
  float v58; // edx
  int v59; // eax
  int v60; // ecx
  int v61; // edx
  float *v62; // eax
  _DWORD *v63; // eax
  int v64; // ecx
  int v65; // ecx
  _DWORD *v66; // eax
  int v67; // ecx
  int v68; // eax
  _DWORD *v69; // eax
  int v70; // ecx
  _DWORD *v71; // ebp
  int v72; // [esp+10h] [ebp-64h]
  float v73; // [esp+14h] [ebp-60h]
  int v74; // [esp+14h] [ebp-60h]
  float v75; // [esp+18h] [ebp-5Ch]
  float v76; // [esp+18h] [ebp-5Ch]
  int v77; // [esp+18h] [ebp-5Ch]
  float v78; // [esp+1Ch] [ebp-58h]
  float v79; // [esp+1Ch] [ebp-58h]
  int v80; // [esp+1Ch] [ebp-58h]
  int v81; // [esp+20h] [ebp-54h] BYREF
  int v82; // [esp+24h] [ebp-50h]
  int v83; // [esp+28h] [ebp-4Ch]
  float v84; // [esp+2Ch] [ebp-48h] BYREF
  float v85; // [esp+30h] [ebp-44h]
  float v86; // [esp+34h] [ebp-40h]
  float v87; // [esp+38h] [ebp-3Ch] BYREF
  float v88; // [esp+3Ch] [ebp-38h]
  float v89; // [esp+40h] [ebp-34h]
  float v90[3]; // [esp+44h] [ebp-30h] BYREF
  int v91[9]; // [esp+50h] [ebp-24h] BYREF
  int v92; // [esp+78h] [ebp+4h]
  float DamageMultiplier; // [esp+78h] [ebp+4h]
  float *v94; // [esp+7Ch] [ebp+8h]
  int v95; // [esp+7Ch] [ebp+8h]
  int v96; // [esp+7Ch] [ebp+8h]
  int v97; // [esp+7Ch] [ebp+8h]
  v4 = a1 == 0;
  result = *(float **)(a2 + 132);
  v94 = result;
  v72 = a1[8];
  v92 = a1[173];
  if ( v4 || a1[4] || !a1[251] )
  {
    if ( !a3
      || (*(_DWORD *)(a3 + 16) || !*(_DWORD *)(a3 + 1004))
      && (*(_DWORD *)result != 3
       || (matMultiplyMatByVec((float *)&v81, (float *)a1 + 38, (float *)(a2 + 4)),
           matMultiplyMatByVec(&v84, (float *)a1 + 38, v94 + 9),
           v84 = v84 + *((float *)a1 + 22),
           v85 = v85 + *((float *)a1 + 23),
           v86 = v86 + *((float *)a1 + 24),
           (result = (float *)sub_43C2E0(a1, a3, &v84, (float *)&v81)) != 0)) )
    {
      --*(_DWORD *)(a2 + 104);
      *(real32 *)(a2 + 112) = universe.totaltimeelapsed;
      v7 = (char *)memAllocFunction(600, MBF_Volatile);
      memset(v7, 0, 0x258u);
      *((_DWORD *)v7 + 4) = 8;
      *((_DWORD *)v7 + 5) = 0;
      *((_DWORD *)v7 + 6) = 0;
      *((_DWORD *)v7 + 5) |= 0x180003u;
      *((_DWORD *)v7 + 113) = 0;
      *((_DWORD *)v7 + 114) = 0;
      *((_DWORD *)v7 + 115) = 0;
      *((_DWORD *)v7 + 112) = 0;
      if ( *(_DWORD *)v94 == 3 )
      {
        *((_DWORD *)v7 + 8) = 300 * v92 + 12193600;
        *((float *)v7 + 123) = universe.unknown1 * *((float *)&unk_BA0F68 + 75 * v92);
        *((_DWORD *)v7 + 120) = 0;
        *((_DWORD *)v7 + 140) = 0;
        *((_DWORD *)v7 + 145) = 0;
        *((_DWORD *)v7 + 112) = sub_5A4AF0(*(_DWORD **)(*((_DWORD *)v7 + 8) + 288), (int)v7);
      }
      else
      {
        *((_DWORD *)v7 + 8) = 300 * v92 + 11774944;
        *((_DWORD *)v7 + 120) = 1;
        *((_DWORD *)v7 + 136) = a1[406];
        if ( a1[406] == 1 )
        {
          listAddNode((_DWORD *)(a1[428] + 16), (_DWORD *)v7 + 141, (int)v7);
          *((_DWORD *)v7 + 145) = a1[428];
          *((_DWORD *)v7 + 137) = a1[407];
          *((_DWORD *)v7 + 138) = a1[408];
          *((_DWORD *)v7 + 139) = a1[409];
          *((_DWORD *)v7 + 146) = a1[429];
          *((_DWORD *)v7 + 147) = a1[430];
          *((float *)v7 + 123) = universe.unknown1 * *(float *)(*((_DWORD *)v7 + 8) + 268);
          *((_DWORD *)v7 + 140) = 1;
        }
        else
        {
          *((_DWORD *)v7 + 140) = 0;
          *((_DWORD *)v7 + 145) = 0;
          *((float *)v7 + 123) = universe.unknown1 * *(float *)(*((_DWORD *)v7 + 8) + 40);
        }
      }
      *((_DWORD *)v7 + 131) = 0;
      v7[592] = *((_BYTE *)a1 + 857);
      *((_DWORD *)v7 + 92) = *(_DWORD *)(*((_DWORD *)v7 + 8) + 164);
      *((_DWORD *)v7 + 9) = 0;
      *((_DWORD *)v7 + 10) = 0;
      *((_DWORD *)v7 + 11) = 0;
      *((_DWORD *)v7 + 12) = 0;
      v7[52] = *((_BYTE *)a1 + 52);
      *((_DWORD *)v7 + 21) = 0;
      *((_DWORD *)v7 + 16) = a1[16];
      *((_DWORD *)v7 + 93) = 0;
      *((_DWORD *)v7 + 94) = 0;
      *((_DWORD *)v7 + 14) = 0;
      *((_WORD *)v7 + 30) = 0;
      *((_DWORD *)v7 + 28) = 0;
      *((_DWORD *)v7 + 29) = 0;
      *((_DWORD *)v7 + 30) = 0;
      *((_DWORD *)v7 + 50) = 0;
      *((_DWORD *)v7 + 51) = 0;
      *((_DWORD *)v7 + 52) = 0;
      v8 = 180;
      v9 = v7 + 380;
      do
      {
        *v9 = *(_DWORD *)(v8 + *((_DWORD *)v7 + 8));
        v8 += 4;
        ++v9;
      }
      while ( v8 < 204 );
      v10 = 204;
      v11 = v7 + 404;
      do
      {
        *v11 = *(_DWORD *)(v10 + *((_DWORD *)v7 + 8));
        v10 += 4;
        ++v11;
      }
      while ( v10 < 228 );
      v12 = 228;
      v13 = v7 + 428;
      do
      {
        *v13 = *(_DWORD *)(v12 + *((_DWORD *)v7 + 8));
        v12 += 4;
        ++v13;
      }
      while ( v12 < 240 );
      *((_DWORD *)v7 + 110) = *(_DWORD *)(*((_DWORD *)v7 + 8) + 40);
      v14 = *(_DWORD **)(a2 + 132);
      v15 = v14[4];
      if ( *(_DWORD *)a2 != v15 || v14[3] == v15 )
        v16 = v14[1];
      else
        v16 = v14[2];
      *((_DWORD *)v7 + 121) = v16;
      *((_DWORD *)v7 + 122) = v92;
      v4 = *((_DWORD *)v7 + 140) == 1;
      *((_DWORD *)v7 + 124) = 0;
      *((_DWORD *)v7 + 126) = a1[203];
      *((float *)v7 + 127) = *(float *)&a1;
      *((_DWORD *)v7 + 128) = a3;
      if ( v4 )
      {
        v17 = *(_DWORD *)(a1[8] + 404);
        *((float *)v7 + 130) = (double)ranRandomFn(10) * ((*(float *)(v17 + 112) - *(float *)(v17 + 116)) * flt_72A4E0)
                             + *(float *)(v17 + 116);
        v18 = *(_DWORD *)(v17 + 124);
        v19 = v94;
        *((_DWORD *)v7 + 125) = v18;
      }
      else
      {
        if ( !a3 || *(_DWORD *)(a3 + 16) )
          v20 = 0;
        else
          v20 = *(_DWORD *)(a3 + 1008);
        v21 = *(_DWORD **)(a2 + 132);
        v22 = v21[4];
        if ( *(_DWORD *)a2 != v22 || v21[3] == v22 )
          v23 = v21[1];
        else
          v23 = v21[2];
        DamageMultiplier = univGetDamageMultiplier(v23, v20);
        v19 = v94;
        *((float *)v7 + 130) = sub_5148F0((int)a1) * DamageMultiplier * v94[6];
        *((float *)v7 + 125) = v94[54];
      }
      if ( (a1[114] & 0x4000) != 0 )
        *((float *)v7 + 130) = *((float *)a1 + 248) * *((float *)v7 + 130);
      *((_WORD *)v7 + 264) = universe.missileNumber;
      if ( multiPlayerGame || dword_A7519C && dword_A75190 )
        IDToPtrTableAdd(&MissileIDToPtr, universe.missileNumber, (int)v7);
      ++universe.missileNumber;
      matMultiplyMatByVec(&v87, (float *)a1 + 38, (float *)(a2 + 4));
      matMultiplyMatByVec(v90, (float *)a1 + 38, v19 + 9);
      *((float *)v7 + 22) = v90[0] + *((float *)a1 + 22);
      *((float *)v7 + 23) = v90[1] + *((float *)a1 + 23);
      *((float *)v7 + 24) = v90[2] + *((float *)a1 + 24);
      *((float *)v7 + 22) = v87 * v19[65] + *((float *)v7 + 22);
      *((float *)v7 + 23) = v88 * v19[65] + *((float *)v7 + 23);
      v24 = v89 * v19[65];
      *((_WORD *)v7 + 62) = 1;
      *((_WORD *)v7 + 63) = 1;
      *((float *)v7 + 24) = v24 + *((float *)v7 + 24);
      v25 = *(_DWORD *)(a2 + 132);
      v26 = *(_DWORD *)(v25 + 16);
      if ( *(_DWORD *)a2 != v26 || *(_DWORD *)(v25 + 12) == v26 )
      {
        v27 = *(float *)(v25 + 220);
      }
      else if ( *(float *)(v25 + 248) > (double)ZERO || *(float *)(v25 + 220) <= (double)ZERO )
      {
        v27 = *(float *)(v25 + 248);
      }
      else
      {
        v95 = *(int *)(v25 + 220);
        telldir();
        v27 = *(float *)&v95;
      }
      *((float *)v7 + 25) = v27 * v87;
      v28 = *(_DWORD *)(a2 + 132);
      v29 = *(_DWORD *)(v28 + 16);
      if ( *(_DWORD *)a2 != v29 || *(_DWORD *)(v28 + 12) == v29 )
      {
        v30 = *(float *)(v28 + 220);
      }
      else if ( *(float *)(v28 + 248) > (double)ZERO || *(float *)(v28 + 220) <= (double)ZERO )
      {
        v30 = *(float *)(v28 + 248);
      }
      else
      {
        v96 = *(int *)(v28 + 220);
        telldir();
        v30 = *(float *)&v96;
      }
      *((float *)v7 + 26) = v30 * v88;
      v31 = *(_DWORD *)(a2 + 132);
      v32 = *(_DWORD *)(v31 + 16);
      if ( *(_DWORD *)a2 != v32 || *(_DWORD *)(v31 + 12) == v32 )
      {
        v33 = *(float *)(v31 + 220);
      }
      else if ( *(float *)(v31 + 248) > (double)ZERO || *(float *)(v31 + 220) <= (double)ZERO )
      {
        v33 = *(float *)(v31 + 248);
      }
      else
      {
        v97 = *(int *)(v31 + 220);
        telldir();
        v33 = *(float *)&v97;
      }
      *((float *)v7 + 27) = v33 * v89;
      *((float *)v7 + 25) = *((float *)a1 + 25) + *((float *)v7 + 25);
      *((float *)v7 + 26) = *((float *)a1 + 26) + *((float *)v7 + 26);
      *((float *)v7 + 27) = *((float *)a1 + 27) + *((float *)v7 + 27);
      if ( *(_DWORD *)v19 == 4 )
      {
        v34 = (int *)a1[292];
        if ( *v34 <= 1 )
        {
          v51 = *((float *)a1 + 38);
          v52 = *((float *)a1 + 40);
          v53 = a1[41];
          v85 = *((float *)a1 + 39);
          v54 = a1[42];
          v86 = v52;
          v74 = v53;
          v55 = a1[44];
          v77 = v54;
          v56 = a1[45];
          v80 = a1[43];
          v57 = a1[46];
          *((float *)v7 + 38) = v51;
          v81 = v55;
          v82 = v56;
          v58 = v86;
          v83 = v57;
          *((float *)v7 + 39) = v85;
          *((float *)v7 + 40) = v58;
          *((_DWORD *)v7 + 41) = v74;
          v59 = v81;
          *((_DWORD *)v7 + 42) = v77;
          v60 = v82;
          *((_DWORD *)v7 + 43) = v80;
          v61 = v83;
          *((_DWORD *)v7 + 44) = v59;
          *((_DWORD *)v7 + 45) = v60;
          *((_DWORD *)v7 + 46) = v61;
        }
        else
        {
          if ( v34 + 1 == (int *)a2 )
          {
            v35 = *((float *)a1 + 41) * MINUS_ONE;
            v36 = *((float *)a1 + 39);
            v37 = *((float *)a1 + 40);
            v73 = *((float *)a1 + 38);
            v85 = *((float *)a1 + 42);
            v38 = a1[45];
            v75 = v36;
            v39 = *((float *)a1 + 43);
            v85 = v85 * MINUS_ONE;
            v86 = v39;
            v40 = v39;
            v41 = a1[46];
            v78 = v37;
            v81 = a1[44];
            v86 = v40 * MINUS_ONE;
            v82 = v38;
            v83 = v41;
          }
          else
          {
            v35 = *((float *)a1 + 41);
            v76 = *((float *)a1 + 39);
            v42 = *((float *)a1 + 43);
            v79 = *((float *)a1 + 40);
            v43 = a1[44];
            v73 = *((float *)a1 + 38) * MINUS_ONE;
            v85 = *((float *)a1 + 42);
            v44 = a1[45];
            v86 = v42;
            v45 = a1[46];
            v75 = v76 * MINUS_ONE;
            v81 = v43;
            v82 = v44;
            v83 = v45;
            v78 = v79 * MINUS_ONE;
          }
          v46 = v85;
          *((float *)v7 + 38) = v35;
          v47 = v86;
          *((float *)v7 + 39) = v46;
          *((float *)v7 + 40) = v47;
          *((float *)v7 + 41) = v73;
          v48 = v81;
          *((float *)v7 + 42) = v75;
          v49 = v82;
          *((float *)v7 + 43) = v78;
          v50 = v83;
          *((_DWORD *)v7 + 44) = v48;
          *((_DWORD *)v7 + 45) = v49;
          *((_DWORD *)v7 + 46) = v50;
        }
      }
      else
      {
        matCreateCoordSysFromHeading((float *)v7 + 38, &v87);
      }
      univUpdateObjRotInfo((int)v7);
      if ( a3 && (v62 = *(float **)(a3 + 84)) != 0 )
        bobCalcObjDistAndAdd(v62, (int)v7);
      else
        collAddSpaceObjToCollBlobs(&universe.collBlobList.head, (int)v7);
      listAddNode(&universe.SpaceObjList.head, v7, (int)v7);
      listAddNode(&universe.ImpactableList.head, (_DWORD *)v7 + 88, (int)v7);
      listAddNode(&universe.MissileList.head, (_DWORD *)v7 + 116, (int)v7);
      lodMeshBindingsAllocate((int)v7);
      univAddObjToRenderListIf((int *)v7, a1);
      soundEventPlay(*(float *)&a1, 513, (_DWORD *)a2);
      v63 = *(_DWORD **)(a2 + 132);
      v64 = v63[4];
      if ( *(_DWORD *)a2 != v64 || v63[3] == v64 )
        v65 = v63[1];
      else
        v65 = v63[2];
      *((_DWORD *)v7 + 129) = dword_C41184[100 * *(_DWORD *)(v72 + 484) + 5 * v65];
      v66 = *(_DWORD **)(a2 + 132);
      v67 = v66[4];
      if ( *(_DWORD *)a2 != v67 || v66[3] == v67 )
        v68 = v66[1];
      else
        v68 = v66[2];
      v69 = (_DWORD *)dword_C41180[100 * *(_DWORD *)(v72 + 484) + 5 * v68];
      if ( v69 && (v70 = *((unsigned __int8 *)a1 + 52), v70 < *v69) )
        v71 = (_DWORD *)v69[v70 + 1];
      else
        v71 = 0;
      result = (float *)univSpaceObjInRenderList((int)a1);
      if ( result )
      {
        if ( v71 )
        {
          result = (float *)etgFireEffectsEnabled;
          if ( etgFireEffectsEnabled )
          {
            result = (float *)etgFrequencyExceeded(v71);
            if ( !result )
            {
              matCreateCoordSysFromHeading((float *)v91, &v87);
              result = (float *)etgEffectCreate((int)v71, (int)a1, (_DWORD *)v7 + 22, 0, v91, 1.0, 96, 2);
            }
          }
        }
      }
      if ( a3 )
      {
        result = *(float **)(a3 + 16);
        if ( !result )
        {
          *(float *)(a3 + 840) = *(float *)&a1;
          *(_BYTE *)(a3 + 859) = 80;
        }
      }
    }
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48C2E0
void gunShootGunsAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 range, vector *trajectory)
{
  int *v5; // edi
  int AnyFSM; // eax
  int v8; // eax
  float *v9; // ecx
  float v10; // edi
  float *v11; // ebx
  int v12; // eax
  int v13; // eax
  int v14; // eax
  int v15; // eax
  signed int v16; // eax
  int *v17; // eax
  double v18; // st7
  int v19; // ebp
  float *v20; // ecx
  int v21; // edi
  int v22; // eax
  int v23; // edx
  float v25; // [esp+10h] [ebp-18h]
  float v26; // [esp+14h] [ebp-14h]
  float *i; // [esp+18h] [ebp-10h]
  float v28; // [esp+1Ch] [ebp-Ch]
  int v29; // [esp+20h] [ebp-8h]
  int v30; // [esp+24h] [ebp-4h]
  int v31; // [esp+2Ch] [ebp+4h]
  v5 = (int *)*((_DWORD *)a1 + 292);
  v31 = 0;
  if ( !v5 )
    return 0;
  if ( singlePlayerGame )
  {
    if ( singlePlayerGameInfo == 15 )
    {
      AnyFSM = aivarFindAnyFSM(aGStopshipsfiri);
      v8 = sub_491B20(0, AnyFSM);
      if ( v8 )
      {
        if ( *(_DWORD *)(v8 + 8) )
          return 0;
      }
    }
  }
  v30 = *v5;
  v29 = 0;
  if ( *v5 <= 0 )
    return v31;
  v9 = (float *)(v5 + 34);
  for ( i = (float *)(v5 + 34); ; i += 34 )
  {
    v10 = *v9;
    v11 = v9 - 33;
    v25 = *v9;
    v26 = a1[20] >= 0 ? *(float *)(LODWORD(v10) + 176) : 0.70710677;
    v12 = *(_DWORD *)(*((_DWORD *)a1 + 8) + 788);
    if ( v12 )
    {
      if ( *(int *)(v12 + 20) > 0 )
      {
        v13 = *((_DWORD *)a1 + 193);
        if ( v13 != 1 )
          break;
      }
    }
    if ( a3 < (double)*(float *)(LODWORD(v10) + 224) )
    {
      v14 = *(_DWORD *)(LODWORD(v10) + 4);
      if ( v14 != 19 && v14 != 18 && (*(float *)&a1 == 0.0 || *((_DWORD *)a1 + 348) != 1) )
      {
        v15 = *(_DWORD *)(LODWORD(v10) + 16);
        if ( *(_DWORD *)v11 != v15 || *(_DWORD *)(LODWORD(v10) + 12) == v15 )
        {
          v16 = sub_48A2F0((int)a1, (int)(v9 - 33));
        }
        else
        {
          v28 = universe.totaltimeelapsed - *(v9 - 5);
          if ( *(float *)(LODWORD(v10) + 256) > (double)ZERO )
          {
            if ( v28 <= (double)*(float *)(LODWORD(v10) + 256) )
              goto LABEL_42;
            v17 = (int *)*((_DWORD *)a1 + 292);
            v18 = *(i - 5);
            v19 = 0;
            if ( *v17 <= 0 )
            {
              v16 = 1;
            }
            else
            {
              v20 = (float *)(v17 + 29);
              v21 = *v17;
              do
              {
                v22 = *((_DWORD *)v20 + 5);
                v23 = *(_DWORD *)(v22 + 16);
                if ( *((_DWORD *)v20 - 28) == v23 && *(_DWORD *)(v22 + 12) != v23 )
                {
                  ++v19;
                  if ( v18 < *v20 )
                    v18 = *v20;
                }
                v20 += 34;
                --v21;
              }
              while ( v21 );
              if ( v19 <= 1 )
              {
                v10 = v25;
                v16 = 1;
              }
              else
              {
                if ( v18 >= universe.totaltimeelapsed - *(float *)(*((_DWORD *)v11 + 33) + 256) / (double)v19 )
                {
                  v10 = v25;
                  goto LABEL_42;
                }
                v10 = v25;
                v16 = 1;
              }
            }
          }
          else
          {
            telldir();
            if ( v28 > (double)*(float *)(LODWORD(v10) + 232) )
            {
              v16 = 1;
              goto LABEL_44;
            }
LABEL_42:
            v16 = 0;
          }
        }
LABEL_44:
        if ( v16 )
        {
          switch ( *(_DWORD *)LODWORD(v10) )
          {
            case 0:
              if ( sub_48CD40((int)a1, (int)v11, (int)a2, COERCE_FLOAT(1)) )
              {
                sub_48B5B0(*(float *)&a1, (int)v11, a2);
                v31 = 1;
              }
              break;
            case 1:
            case 2:
              if ( sub_48A5E0(*(float *)&a1, (int)v11, (int)a2) )
              {
                sub_48B5B0(*(float *)&a1, (int)v11, a2);
                goto LABEL_57;
              }
              break;
            case 3:
              if ( *((int *)v11 + 26) > 0
                && *((float *)a1 + 46) * a4[2] + *((float *)a1 + 45) * a4[1] + *((float *)a1 + 44) * *a4 >= v26
                && !sub_5AD360((int)a1) )
              {
                goto LABEL_49;
              }
              break;
            case 4:
              if ( *((int *)v11 + 26) > 0
                && *((float *)a1 + 46) * a4[2] + *((float *)a1 + 45) * a4[1] + *((float *)a1 + 44) * *a4 >= v26 )
              {
LABEL_49:
                sub_48AAC0((int *)a1, (int)v11, (int)a2);
LABEL_57:
                v31 = 1;
              }
              break;
            default:
              break;
          }
        }
      }
    }
    v9 = i + 34;
    if ( ++v29 >= v30 )
      return v31;
  }
  if ( v13 != 3 )
    sub_4A5560((int)a1);
  return v31;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48C690
void gunShootGunsAtMultipleTargets(Ship *ship)
{
  int *v2; // edx
  int v3; // eax
  float *v4; // ecx
  int v5; // edi
  int v6; // eax
  float *v7; // ebx
  int v8; // eax
  int v9; // eax
  int v10; // eax
  signed int v11; // eax
  int *v12; // eax
  double v13; // st7
  int v14; // edi
  int *v15; // ecx
  int v16; // esi
  int v17; // eax
  int v18; // edx
  _DWORD *v19; // esi
  bool v20; // zf
  int v21; // eax
  int *v22; // eax
  bool v23; // cc
  int *v25; // [esp+10h] [ebp-1Ch]
  float *i; // [esp+14h] [ebp-18h]
  float v27; // [esp+18h] [ebp-14h]
  int v28; // [esp+1Ch] [ebp-10h]
  int v29; // [esp+20h] [ebp-Ch]
  int *v30; // [esp+24h] [ebp-8h]
  int v31; // [esp+28h] [ebp-4h]
  int v32; // [esp+30h] [ebp+4h]
  v2 = (int *)a1[292];
  v3 = a1[221];
  v30 = v2;
  v31 = *v2;
  v32 = 0;
  v29 = 0;
  if ( *v2 <= 0 )
    return v32;
  v4 = (float *)(v2 + 34);
  v25 = (int *)(v3 + 4);
  for ( i = (float *)(v2 + 34); ; i += 34 )
  {
    v5 = *(_DWORD *)v4;
    v6 = *(_DWORD *)(*(_DWORD *)v4 + 4);
    v7 = v4 - 33;
    v28 = *(_DWORD *)v4;
    if ( v6 == 19 || v6 == 18 )
      goto LABEL_43;
    v8 = *(_DWORD *)(a1[8] + 788);
    if ( v8 )
    {
      if ( *(int *)(v8 + 20) > 0 )
      {
        v9 = a1[193];
        if ( v9 != 1 )
          break;
      }
    }
    if ( *(float *)&a1 != 0.0 && a1[348] == 1 )
    {
LABEL_41:
      v21 = *(_DWORD *)(v5 + 280);
      if ( v21 >= 0 )
      {
        v22 = &v30[34 * v21 + 1];
        *((_DWORD *)v7 + 1) = v22[1];
        *((_DWORD *)v7 + 2) = v22[2];
        *((_DWORD *)v7 + 3) = v22[3];
        qmemcpy(v7 + 4, v22 + 4, 0x24u);
        *((_DWORD *)v7 + 23) = v22[23];
        *((_DWORD *)v7 + 22) = v22[22];
      }
      goto LABEL_43;
    }
    v10 = *(_DWORD *)(v5 + 16);
    if ( *(_DWORD *)v7 != v10 || *(_DWORD *)(v5 + 12) == v10 )
    {
      v11 = sub_48A2F0((int)a1, (int)(v4 - 33));
      goto LABEL_32;
    }
    v27 = universe.totaltimeelapsed - *(v4 - 5);
    if ( *(float *)(v5 + 256) > (double)ZERO )
    {
      if ( v27 <= (double)*(float *)(v5 + 256) )
        goto LABEL_30;
      v12 = (int *)a1[292];
      v13 = *(i - 5);
      v14 = 0;
      if ( *v12 <= 0 )
        goto LABEL_28;
      v15 = v12 + 29;
      v16 = *v12;
      do
      {
        v17 = v15[5];
        v18 = *(_DWORD *)(v17 + 16);
        if ( *(v15 - 28) == v18 && *(_DWORD *)(v17 + 12) != v18 )
        {
          ++v14;
          if ( v13 < *(float *)v15 )
            v13 = *(float *)v15;
        }
        v15 += 34;
        --v16;
      }
      while ( v16 );
      if ( v14 <= 1 )
      {
LABEL_28:
        v5 = v28;
        v11 = 1;
      }
      else
      {
        if ( v13 >= universe.totaltimeelapsed - *(float *)(*((_DWORD *)v7 + 33) + 256) / (double)v14 )
        {
          v5 = v28;
LABEL_30:
          v11 = 0;
          goto LABEL_32;
        }
        v5 = v28;
        v11 = 1;
      }
    }
    else
    {
      telldir();
      if ( v27 <= (double)*(float *)(v5 + 232) )
        goto LABEL_30;
      v11 = 1;
    }
LABEL_32:
    if ( !v11 )
      goto LABEL_41;
    v19 = (_DWORD *)*v25;
    if ( *v25 )
    {
      switch ( *(_DWORD *)v5 )
      {
        case 0:
          v20 = sub_48CD40((int)a1, (int)v7, (int)v19, COERCE_FLOAT(1)) == 0;
          goto LABEL_38;
        case 1:
        case 2:
          v20 = sub_48A5E0(*(float *)&a1, (int)v7, *v25) == 0;
LABEL_38:
          if ( !v20 )
          {
            sub_48B5B0(*(float *)&a1, (int)v7, v19);
            v32 = 1;
          }
          break;
        case 3:
        case 4:
          if ( *((int *)v7 + 26) > 0 )
          {
            sub_48AAC0(a1, (int)v7, *v25);
            v32 = 1;
          }
          break;
        default:
          break;
      }
    }
LABEL_43:
    v4 = i + 34;
    v23 = ++v29 < v31;
    ++v25;
    if ( !v23 )
      return v32;
  }
  if ( v9 != 3 )
    sub_4A5560((int)a1);
  return v32;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48A2F0
BOOL __cdecl gunCanShoot(int ship, int gun)
{
    int gunstatic;
    int ShipAndItsCommand;
    double fireTime;
    int bulletType;
    double healthRatio;
    float burstFireTime;

    gunstatic = *(int *)(gun + 132);
    ShipAndItsCommand = getShipAndItsCommand((int)&universe, ship);

    if (*(float *)(gunstatic + 236) == ZERO)
    {
        fireTime = *(float *)(gunstatic + 232);
        bulletType = *(int *)(gunstatic + 4);

        if (bulletType != 19 && bulletType != 18)
        {
            healthRatio = *(float *)(ship + 368) / *(float *)(*(int *)(ship + 32) + 164);

            if (healthRatio >= flt_72A4D4)
            {
                if (healthRatio >= (double)flt_72A450)
                {
                    if (healthRatio >= (double)ZERO_POINT_FIVE)
                    {
                        if (healthRatio < (double)flt_72A670)
                            fireTime = fireTime * dbl_72AB08;
                    }
                    else
                        fireTime = fireTime * dbl_72AB10;
                }
                else
                    fireTime = fireTime * dbl_72AB18;
            }
            else
                fireTime = fireTime * dbl_72AB20;
        }

        return universe.totaltimeelapsed - *(float *)(gun + 112) > fireTime;
    }

    /* Burst fire state machine */
    int burstState = *(int *)(gun + 128);
    if (burstState)
    {
        if (burstState == 1)
        {
            ship->cataBurstFiring = TRUE;
            if (universe.totaltimeelapsed - *(float *)(gun + 116) <= *(float *)(gun + 120))
                goto not_ready;
            *(int *)(gun + 128) = 2;
            *(float *)(gun + 116) = universe.totaltimeelapsed;
            soundEventShipRemove((int *)ship);
        }

        ship->cataBurstFiring = FALSE;
        if (universe.totaltimeelapsed - *(float *)(gun + 116) <= *(float *)(gun + 124))
            return 0;
        *(int *)(gun + 128) = 0;
    }

    *(float *)(gun + 116) = universe.totaltimeelapsed;
    *(int *)(gun + 128) = 1;
    *(int *)(gun + 120) = *(int *)(gunstatic + 236);
    *(float *)(gun + 124) = *(float *)(gunstatic + 240);

    /* Adjust cooloff time by health ratio */
    burstFireTime = *(float *)(gunstatic + 240);
    healthRatio = *(float *)(ship + 368) / *(float *)(*(int *)(ship + 32) + 164);

    if (healthRatio >= flt_72A4D4)
    {
        if (healthRatio >= (double)flt_72A450)
        {
            if (healthRatio >= (double)ZERO_POINT_FIVE)
            {
                if (healthRatio < (double)flt_72A670)
                    *(float *)(gun + 124) = burstFireTime * dbl_72AB08;
            }
            else
                *(float *)(gun + 124) = burstFireTime * dbl_72AB10;
        }
        else
            *(float *)(gun + 124) = burstFireTime * dbl_72AB18;
    }
    else
        *(float *)(gun + 124) = burstFireTime * dbl_72AB20;

not_ready:
    /* Passive attack fire rate penalty */
    if (!ShipAndItsCommand
        || (*(short *)(ShipAndItsCommand + 22) & 4) == 0
        || *(int *)(*(int *)(ship + 32) + 524)
        || (*(short *)(ShipAndItsCommand + 22) & 2) != 0
           && (*(short *)(ShipAndItsCommand + 22) & 1) != 0
           && *(int *)(ShipAndItsCommand + 560) == 5)
    {
        if (universe.totaltimeelapsed - *(float *)(gun + 112) > *(float *)(gunstatic + 232))
            return 1;
    }
    else
    {
        float passiveFireTime = flt_C523A8 + *(float *)(gunstatic + 232);
        float elapsed = universe.totaltimeelapsed - *(float *)(gun + 112);
        if (elapsed > passiveFireTime)
        {
            if ((double)ranRandomFn(10) * ((flt_C523AC - flt_C523A8) * flt_72A4E0)
                + flt_C523A8 + passiveFireTime < elapsed)
                return 1;
        }
    }
    return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48CD40
bool __cdecl gunCanHitTarget(int ship, int gun, int target, float useGunHeading)
{
    int gunstatic;
    float bulletSpeed;
    float targetInWorldCoordSys[3];
    int predictedPos[3];
    float gunPosWorld[3];
    int gunMountPos[3];
    float dx, dy, dz, dist;
    double invDist, dotProduct;

    gunstatic = *(int *)(gun + 132);
    int currentTarget = *(int *)(gunstatic + 16);

    if (*(int *)gun != currentTarget || *(int *)(gunstatic + 12) == currentTarget)
        bulletSpeed = *(float *)(gunstatic + 220);
    else if (*(float *)(gunstatic + 248) > (double)ZERO || *(float *)(gunstatic + 220) <= (double)ZERO)
        bulletSpeed = *(float *)(gunstatic + 248);
    else
    {
        bulletSpeed = *(float *)(gunstatic + 220);
        telldir();
    }

    aishipGetTrajectoryWithVelPrediction(ship, target, bulletSpeed, (int)predictedPos);
    matMultiplyMatByVec(gunPosWorld, (float *)(ship + 152), (float *)(gunstatic + 36));

    dx = *(float *)predictedPos - gunPosWorld[0];
    dy = *(float *)&predictedPos[1] - gunPosWorld[1];
    dz = *(float *)&predictedPos[2] - gunPosWorld[2];

    if (LOBYTE(useGunHeading))
        matMultiplyMatByVec(targetInWorldCoordSys, (float *)(ship + 152), (float *)(gun + 4));
    else
    {
        gunMountPos[0] = *(int *)(ship + 176);
        gunMountPos[1] = *(int *)(ship + 180);
        gunMountPos[2] = *(int *)(ship + 184);
        matMultiplyMatByVec(targetInWorldCoordSys, (float *)(ship + 152), (float *)gunMountPos);
    }

    dist = fmathSqrtDouble(dz * dz + dy * dy + dx * dx);
    invDist = ONE / dist;
    dx = invDist * dx;
    dy = invDist * dy;

    dotProduct = invDist * dz * targetInWorldCoordSys[2]
               + targetInWorldCoordSys[1] * dy
               + targetInWorldCoordSys[0] * dx;

    if (dotProduct <= ZERO)
        return 0;
    if (dotProduct > ONE)
        dotProduct = ONE;

    return fmathSqrtDouble(ONE - dotProduct * dotProduct) * dist
         < *(float *)(*(int *)(target + 32) + 76) * flt_72A670;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48B5B0
int *__cdecl gunShootGunAtTarget(float shipAsFloat, int gun, int *target)
{
    int gunstatic;
    float gunDirection[3];
    float gunPosition[3];
    int ship = LODWORD(shipAsFloat);

    gunstatic = *(int *)(gun + 132);

    /* Skip if dRKO (type 109) targeting its own turret */
    if (*(int *)(ship + 688) == 109 && target && !target[4] && *(int **)(ship + 1636) == target)
        return 0;

    /* Skip if ship is dead */
    if (shipAsFloat != 0.0 && !*(int *)(ship + 16) && *(int *)(ship + 1004))
        return 0;

    if (target)
    {
        /* Skip if target is cloaked */
        if (!target[4] && target[251])
            return 0;

        /* Line of sight check for small ships */
        if (*(int *)(*(int *)(ship + 32) + 488) <= 4u && *(int *)(gunstatic + 4) != 18)
        {
            matMultiplyMatByVec(gunDirection, (float *)(ship + 152), (float *)(gun + 4));
            matMultiplyMatByVec(gunPosition, (float *)(ship + 152), (float *)(gunstatic + 36));
            gunPosition[0] += *(float *)(ship + 88);
            gunPosition[1] += *(float *)(ship + 92);
            gunPosition[2] += *(float *)(ship + 96);

            if (!collCheckLineOfSight((int *)ship, (int)target, gunPosition, gunDirection))
                return 0;
        }
    }

    /* Set ship as firing */
    if (!*(unsigned char *)(ship + 856))
    {
        *(unsigned char *)(ship + 856) = 1;
        *(unsigned char *)(ship + 1558) = 1;
    }

    /* Set slave ship as firing too */
    int slaveShip = *(int *)(ship + 896);
    if (slaveShip && !*(unsigned char *)(slaveShip + 856))
    {
        *(unsigned char *)(slaveShip + 856) = 1;
        *(unsigned char *)(ship + 1558) = 1;
    }

    if ((*(int *)(ship + 456) & 0x10000000) != 0)
        return 0;

    /* Record fire time and compute damage */
    *(float *)(gun + 112) = universe.totaltimeelapsed;
    float damageMult = tacticsGetShipDamageModifier(ship);
    float damage = damageMult * *(float *)(gunstatic + 24);

    if ((*(int *)(ship + 456) & 0x4000) != 0)
        damage = damage * *(float *)(ship + 992);

    /* Beast infection damage */
    if (target && !target[4] && target[172] == 3)
        ApplyDamageToTarget(
            (int)target, 10.0f,
            *(int *)(gunstatic + 4), 666,
            *(unsigned short *)(*(int *)(ship + 812) + 36),
            (int *)ship);

    return sub_48BB00(shipAsFloat, gun, 0, (int)target, damage);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48A5E0
int __cdecl gunOrientGimbleGun(int ship, float *gun, int target)
{
    int gunstatic;
    float triggerHappy, maxAngle, maxAngleSpeed, bulletSpeed;
    vector targetInWorldCoordSys;
    float targetInGunCoordSys[3];
    int predictedPos[3];
    float rotY[9], rotX[9], tempMat[9];
    float dotprod, desiredSpeed;

    gunstatic = *((int *)gun + 33);

    if (*(char *)(ship + 20) >= 0)
    {
        maxAngle = *(float *)(gunstatic + 64);
        triggerHappy = *(float *)(gunstatic + 176);
    }
    else
    {
        triggerHappy = 0.70710677f;
        maxAngle = 0.70710677f;
    }

    maxAngleSpeed = *(float *)(gunstatic + 156);

    int currentTarget = *(int *)(gunstatic + 16);
    if (*(int *)gun != currentTarget || *(int *)(gunstatic + 12) == currentTarget)
        bulletSpeed = *(float *)(gunstatic + 220);
    else if (*(float *)(gunstatic + 248) > (double)ZERO || *(float *)(gunstatic + 220) <= (double)ZERO)
        bulletSpeed = *(float *)(gunstatic + 248);
    else
    {
        bulletSpeed = *(float *)(gunstatic + 220);
        telldir();
    }

    if (*(int *)gunstatic == 1)  /* GUN_Gimble */
    {
        aishipGetTrajectoryWithVelPrediction(ship, target, bulletSpeed, (int)predictedPos);
        matMultiplyVecByMat(&targetInWorldCoordSys, (float *)predictedPos, (float *)(ship + 152));
        targetInWorldCoordSys.x -= *(float *)(gunstatic + 36);
        targetInWorldCoordSys.y -= *(float *)(gunstatic + 40);
        targetInWorldCoordSys.z -= *(float *)(gunstatic + 44);
        vecNormalize(&targetInWorldCoordSys);

        dotprod = targetInWorldCoordSys.z * *(float *)(gunstatic + 56)
                + targetInWorldCoordSys.y * *(float *)(gunstatic + 52)
                + targetInWorldCoordSys.x * *(float *)(gunstatic + 48);

        if (dotprod > ONE)
            dotprod = ONE;

        if (dotprod >= maxAngle && dotprod <= *(float *)(gunstatic + 60))
            goto AIM_AT_TARGET;

        if (*((int *)gun + 27) < 0)
            *((int *)gun + 27) = soundEventPlay(ship, 512, gun);
        return 0;
    }

    /* GUN_NewGimble path */
    int linkedGunIndex = *(int *)(gunstatic + 280);
    if (linkedGunIndex >= 0)
    {
        float *linkedGun = (float *)(*(int *)(ship + 1168) + 136 * linkedGunIndex + 4);
        gun[1] = linkedGun[1]; gun[2] = linkedGun[2]; gun[3] = linkedGun[3];
        qmemcpy(gun + 4, linkedGun + 4, 0x24u);
        gun[23] = linkedGun[23]; gun[22] = linkedGun[22];
    }

    aishipGetTrajectoryWithVelPrediction(ship, target, bulletSpeed, (int)predictedPos);
    matMultiplyVecByMat(&targetInWorldCoordSys, (float *)predictedPos, (float *)(ship + 152));
    targetInWorldCoordSys.x -= *(float *)(gunstatic + 36);
    targetInWorldCoordSys.y -= *(float *)(gunstatic + 40);
    targetInWorldCoordSys.z -= *(float *)(gunstatic + 44);
    vecNormalize(&targetInWorldCoordSys);

    matMultiplyVecByMat(targetInGunCoordSys, &targetInWorldCoordSys, gun + 4);
    dotprod = targetInGunCoordSys[2];

    gun[24] = 0.0f;
    gun[25] = 0.0f;

    if (dotprod < (double)triggerHappy)
    {
        int subtype = *(int *)(gunstatic + 12);
        if (subtype != 2 && subtype != 4)
            goto AIM_AT_TARGET;
    }

    int turningY = 0;
    if (*(int *)(gunstatic + 280) >= 0)
        goto CHECK_AIM;

    /* Y-axis (declination) tracking */
    if (targetInGunCoordSys[1] != ZERO)
    {
        desiredSpeed = -(targetInGunCoordSys[1] * *(float *)(gunstatic + 164));
        if (desiredSpeed > maxAngleSpeed)
            desiredSpeed = maxAngleSpeed;
        else if (desiredSpeed < -maxAngleSpeed)
            desiredSpeed = -maxAngleSpeed;

        float newDeclination = universe.unknown1 * desiredSpeed + gun[22];
        if (newDeclination < *(float *)(gunstatic + 144) && newDeclination > *(float *)(gunstatic + 140))
        {
            gun[22] = newDeclination;
            turningY = 1;
            gun[24] = desiredSpeed;
        }
    }

    /* X-axis (angle) tracking */
    if (targetInGunCoordSys[0] == ZERO)
        goto CHECK_TURNING;

    desiredSpeed = targetInGunCoordSys[0] * *(float *)(gunstatic + 168);
    if (desiredSpeed > maxAngleSpeed)
        desiredSpeed = maxAngleSpeed;
    else if (desiredSpeed < -maxAngleSpeed)
        desiredSpeed = -maxAngleSpeed;

    float newAngle = universe.unknown1 * desiredSpeed + gun[23];
    if (newAngle >= *(float *)(gunstatic + 152) || newAngle <= *(float *)(gunstatic + 148))
    {
CHECK_TURNING:
        if (!turningY)
            goto CHECK_AIM;
    }
    else
    {
        gun[23] = newAngle;
        gun[25] = desiredSpeed;
    }

    /* Rebuild coordinate system from angles */
    matMakeRotAboutY((int)rotY, cos(gun[23]), sin(gun[23]));
    matMakeRotAboutX((int)rotX, cos(gun[22]), sin(gun[22]));
    matMultiplyMatByMat((int)tempMat, gunstatic + 68, (float *)rotX);
    matMultiplyMatByMat((int)(gun + 4), (int)tempMat, (float *)rotY);

    if ((*(unsigned char *)(gunstatic + 28) & 1) != 0)
    {
        matMultiplyMatByMat((int)tempMat, gunstatic + 104, (float *)rotY);
        matMultiplyMatByMat((int)(gun + 13), (int)tempMat, (float *)rotX);
    }

    gun[1] = gun[10]; gun[2] = gun[11]; gun[3] = gun[12];

CHECK_AIM:
    if (dotprod >= (double)triggerHappy)
    {
AIM_AT_TARGET:
        gun[1] = targetInWorldCoordSys.x;
        gun[2] = targetInWorldCoordSys.y;
        gun[3] = targetInWorldCoordSys.z;
        soundEventStopSound(*((int *)gun + 27), 0);
        gun[27] = NAN;
        return 1;
    }

    if (*((int *)gun + 27) < 0)
        *((int *)gun + 27) = soundEventPlay(ship, 512, gun);
    return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48BB00
char *__cdecl gunCreateBullet(int *ship, int gun, float *sourceShip, int target, int damage)
{
    int gunstatic;
    int staticinfo;
    char *bullet;
    vector heading;
    float gunPosWorld[3];
    float muzzleOffset[3];
    float tempVec[3];
    float bulletSpeed;
    int bulletType, lodLevel;

    gunstatic = *(int *)(gun + 132);
    staticinfo = ship[8];

    /* Allocate bullet object */
    bullet = (char *)memAllocFunction(356, MBF_SmallBlockHeap);
    *((int *)bullet + 33) = 0;
    *((int *)bullet + 4) = 1;
    *((int *)bullet + 5) = 0;
    *((int *)bullet + 6) = 0;
    *((int *)bullet + 8) = 0;
    *((int *)bullet + 9) = 0;
    *((int *)bullet + 10) = 0;
    *((int *)bullet + 11) = 0;
    *((int *)bullet + 12) = 0;
    bullet[52] = ((Ship *)ship)->currentLOD;
    *((int *)bullet + 21) = 0;
    *((int *)bullet + 16) = ship[16];
    lodMeshBindingsAllocate((int)bullet);

    *((int *)bullet + 67) = (ship[114] & 0x40) != 0;
    *((int *)bullet + 66) = *(int *)gun;
    *((int *)bullet + 78) = damage;

    /* Get bullet type based on current vs default target */
    int currentTarget = *(int *)(gunstatic + 16);
    int bType;
    if (*(int *)gun != currentTarget || *(int *)(gunstatic + 12) == currentTarget)
        bType = *(int *)(gunstatic + 4);
    else
        bType = *(int *)(gunstatic + 8);
    *((int *)bullet + 68) = bType;

    *((float *)bullet + 71) = *(float *)&ship;
    *((int *)bullet + 83) = ship[203];
    *((int *)bullet + 72) = gun;

    /* Set target (or 0 if friendly fire disabled) */
    if (!target || (ship[114] & 0x40) != 0)
        *((int *)bullet + 73) = 0;
    else
        *((int *)bullet + 73) = target;

    *((int *)bullet + 74) = dword_C42920[20 * *(int *)(staticinfo + 484) + bType];
    *((int *)bullet + 75) = *(int *)(gunstatic + 228);
    *((int *)bullet + 63) = *(int *)(gunstatic + 212);
    *((int *)bullet + 69) = 0;

    /* Bullet speed */
    int curTarget2 = *(int *)(gunstatic + 16);
    if (*(int *)gun != curTarget2 || *(int *)(gunstatic + 12) == curTarget2)
        bulletSpeed = *(float *)(gunstatic + 216);
    else
        bulletSpeed = *(float *)(gunstatic + 244);
    *((float *)bullet + 70) = bulletSpeed;

    *((int *)bullet + 80) = 0;
    *((int *)bullet + 81) = 0;
    memset(bullet + 88, 0, 0x28u);

    /* Compute heading direction */
    if (sourceShip)
    {
        heading.x = *(float *)(target + 88) - sourceShip[22];
        heading.y = *(float *)(target + 92) - sourceShip[23];
        heading.z = *(float *)(target + 96) - sourceShip[24];
        vecNormalize(&heading);
    }
    else
    {
        matMultiplyMatByVec(&heading.x, (float *)ship + 38, (float *)(gun + 4));
    }

    *((vector *)bullet + 19) = heading;
    *((float *)bullet + 60) = *(float *)(gunstatic + 212) * heading.x;
    *((float *)bullet + 61) = *(float *)(gunstatic + 212) * heading.y;
    *((float *)bullet + 62) = *(float *)(gunstatic + 212) * heading.z;

    /* Set bullet velocity based on gun type */
    int gunType = *(int *)gun;
    switch (gunType)
    {
        case 0: case 1: case 3: case 7:  /* projectile types */
        {
            float speed;
            if ((ship[114] & 0x40) != 0)
                speed = flt_BB32A0;
            else
            {
                int gs = *(int *)(gun + 132);
                int ct = *(int *)(gs + 16);
                if (*(int *)gun != ct || *(int *)(gs + 12) == ct)
                    speed = *(float *)(gs + 220);
                else if (*(float *)(gs + 248) > (double)ZERO || *(float *)(gs + 220) <= (double)ZERO)
                    speed = *(float *)(gs + 248);
                else
                {
                    speed = *(float *)(gs + 220);
                    telldir();
                }
            }
            *((float *)bullet + 25) = speed * heading.x;
            *((float *)bullet + 26) = speed * heading.y;
            *((float *)bullet + 27) = speed * heading.z;
            *((float *)bullet + 64) = universe.unknown1 * speed;
            *((int *)bullet + 65) = 0;
            break;
        }
        case 2: case 4: case 8:  /* beam/instant types */
            *((int *)bullet + 25) = 0;
            *((int *)bullet + 26) = 0;
            *((int *)bullet + 27) = 0;
            *((int *)bullet + 64) = *(int *)(gunstatic + 212);
            *((int *)bullet + 65) = *(int *)(gunstatic + 212);
            break;
    }

    /* Set bullet position */
    if (sourceShip)
    {
        *((int *)bullet + 22) = ship[22];
        *((int *)bullet + 23) = ship[23];
        *((int *)bullet + 24) = ship[24];
    }
    else
    {
        matMultiplyMatByVec(gunPosWorld, (float *)ship + 38, (float *)(gunstatic + 36));
        if (*(float *)(gunstatic + 268) != ZERO)
        {
            matMultiplyMatByVec(tempVec, (float *)(gun + 16), (float *)(gunstatic + 268));
            matMultiplyMatByVec(muzzleOffset, (float *)ship + 38, tempVec);
            gunPosWorld[0] += muzzleOffset[0];
            gunPosWorld[1] += muzzleOffset[1];
            gunPosWorld[2] += muzzleOffset[2];
        }
        *((float *)bullet + 22) = gunPosWorld[0] + *((float *)ship + 22);
        *((float *)bullet + 23) = gunPosWorld[1] + *((float *)ship + 23);
        *((float *)bullet + 24) = gunPosWorld[2] + *((float *)ship + 24);
        *((float *)bullet + 22) += heading.x * *(float *)(gunstatic + 260);
        *((float *)bullet + 23) += heading.y * *(float *)(gunstatic + 260);
        *((float *)bullet + 24) += heading.z * *(float *)(gunstatic + 260);
    }

    /* Add to universe */
    *((short *)bullet + 62) = 1;
    *((short *)bullet + 63) = 1;

    if (target && *(float **)(target + 84))
        bobCalcObjDistAndAdd(*(float **)(target + 84), (int)bullet);
    else
        collAddSpaceObjToCollBlobs(&universe.collBlobList.head, (int)bullet);

    listAddNode(&universe.SpaceObjList.head, bullet, (int)bullet);
    listAddNode(&universe.BulletList.head, (int *)bullet + 53, (int)bullet);
    univAddObjToRenderListIf((int *)bullet, ship);

    /* Play firing sounds */
    if (*(float *)(gunstatic + 236) != ZERO)
    {
        if (ship[289])
            soundEventPlay(*(float *)&ship, 514, (int *)gun);
    }
    else if (*((int *)bullet + 66) == 2)
        soundEventPlay(*(float *)&ship, 514, (int *)gun);
    else if (bType != 19)
        soundEventPlay(*(float *)&ship, 513, (int *)gun);

    /* Create visual effects */
    univGetDamageMultiplier(bType, 0);

    int *etgLOD = NULL;
    if ((ship[114] & 0x40) != 0)
        etgLOD = (int *)dword_C41CCC;
    else
        etgLOD = (int *)dword_C41188[100 * *(int *)(staticinfo + 484) + 5 * bType];

    if (etgLOD)
    {
        lodLevel = *((unsigned char *)ship + 52);
        if (target && lodLevel >= *(unsigned char *)(target + 52))
            lodLevel = *(unsigned char *)(target + 52);
        if (bType == 18)
            lodLevel = 0;

        if (lodLevel < *etgLOD)
        {
            int *etgStat = (int *)etgLOD[lodLevel + 1];
            if (univSpaceObjInRenderList((int)ship) && etgStat && etgBulletEffectsEnabled && !etgFrequencyExceeded(etgStat))
            {
                if (*((int *)bullet + 66) == 2)
                    *((int *)bullet + 77) = etgEffectCreate((int)etgStat, (int)bullet, 0, ship + 25, 0, 1.0, 56, 5);
                else
                    *((int *)bullet + 77) = etgEffectCreate((int)etgStat, (int)bullet, 0, ship + 25, 0, 1.0, 56, 3);
            }
            else
                *((int *)bullet + 77) = 0;
        }
    }

    /* Muzzle flash effect */
    int *muzzleLOD = (int *)dword_C41184[100 * *(int *)(staticinfo + 484) + 5 * bType];
    int *muzzleFlash = (int *)dword_C41180[100 * *(int *)(staticinfo + 484) + 5 * bType];

    *((int *)bullet + 76) = (int)muzzleLOD;

    if (muzzleFlash)
    {
        int shipLOD = *((unsigned char *)ship + 52);
        if (shipLOD < *muzzleFlash)
        {
            int *flashStat = (int *)muzzleFlash[shipLOD + 1];
            if (univSpaceObjInRenderList((int)ship) && flashStat && etgFireEffectsEnabled && !etgFrequencyExceeded(flashStat))
                etgEffectCreate((int)flashStat, (int)ship, (int *)bullet + 22, ship + 25, bullet + 152, 1.0, 32, 1);
        }
    }

    /* Ping for big gun bullets */
    if (*((int *)bullet + 68) == 18)
        pingAnomalyObjectPingAdd((int)aBulletbiggun, (float *)bullet);

    /* Save initial position */
    *((int *)bullet + 86) = *((int *)bullet + 22);
    *((int *)bullet + 87) = *((int *)bullet + 23);
    *((int *)bullet + 88) = *((int *)bullet + 24);
    *((int *)bullet + 79) = 0;

    /* Mark target as being fired upon */
    if (target && !*(int *)(target + 16))
    {
        *(float *)(target + 840) = *(float *)&ship;
        *(char *)(target + 859) = 80;
    }

    return bullet;
}
#endif
