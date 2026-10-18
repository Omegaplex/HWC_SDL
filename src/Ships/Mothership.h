// =============================================================================
//  Mothership.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___MOTHERSHIP_H
#define ___MOTHERSHIP_H

#include "SpaceObj.h"
#include "Timer.h"
#include "Types.h"
#include "BigGun.h"

/*
 * Shared Cataclysm mothership big-gun state block. Retail stores this at
 * BeastMothershipSpec+0x30 for dBeastMothership, and at
 * ShipSpecifics+0x20 (equivalently shipModuleSlotRuntime(ship)+0x1C)
 * for sMothership/bMothership.
 */
typedef struct MothershipBigGunSpecifics
{
    vector bigGunTarget;
    vector bigGunFirePosition;
    struct SpaceObjRotImpTarg *bigGunProjectile;
    ubyte bigGunState;
    ubyte unknown3D_3F[0x03];
    real32 bigGunStateStartTime;
    ubyte unknown44_47[0x04];
    struct Effect *bigGunChargeEffect;
    ubyte bigGunAnimationPending;
    ubyte unknown4D_4F[0x03];
    sdword bigGunTimerStarted;
    TimerStorage24 bigGunTimer;
} MothershipBigGunSpecifics;

static_assert(sizeof(MothershipBigGunSpecifics) == 0x4C, "MothershipBigGunSpecifics size mismatch");
static_assert(offsetof(MothershipBigGunSpecifics, bigGunTarget) == 0x00, "MothershipBigGunSpecifics target offset mismatch");
static_assert(offsetof(MothershipBigGunSpecifics, bigGunProjectile) == 0x18, "MothershipBigGunSpecifics projectile offset mismatch");
static_assert(offsetof(MothershipBigGunSpecifics, bigGunState) == 0x1C, "MothershipBigGunSpecifics state offset mismatch");
static_assert(offsetof(MothershipBigGunSpecifics, bigGunChargeEffect) == 0x28, "MothershipBigGunSpecifics charge effect offset mismatch");
static_assert(offsetof(MothershipBigGunSpecifics, bigGunTimerStarted) == 0x30, "MothershipBigGunSpecifics timer-start flag offset mismatch");
static_assert(offsetof(MothershipBigGunSpecifics, bigGunTimer) == 0x34, "MothershipBigGunSpecifics timer offset mismatch");

typedef struct
{
    ShipBigGunState bigGunState;
    sdword doorFlags;
    SpaceObjRotImpTargGuidanceShipDerelict *doorCargo;
    SpaceObjRotImpTargGuidanceShipDerelict *droptarget[MAX_NUM_DROP];
    sdword dropstate[MAX_NUM_DROP];
    sdword dockindex[MAX_NUM_DROP];
    sdword CAP_NumInBuildCue;
    /*
     * Cataclysm mothership saves serialize a 496-byte specifics block with the
     * same 24-byte big-gun prefix used at ShipSpecifics[0x00] and a 33-slot captured
     * build queue.
     */
#define CATA_MOTHERSHIP_MAX_CAPTURABLE 33
    sdword CAPshiptype[CATA_MOTHERSHIP_MAX_CAPTURABLE];
    sdword CAPshiprace[CATA_MOTHERSHIP_MAX_CAPTURABLE];
    real32 CAPTimeToBuildShip[CATA_MOTHERSHIP_MAX_CAPTURABLE];
    ubyte freedocktimer[4];
} MothershipSpec;

static_assert(sizeof(MothershipSpec) == 496, "MothershipSpec size mismatch");

typedef struct
{
    vector specialDoorOffset[NUM_RACES][TOTAL_NUM_SHIPS];
    sdword specialDoorOrientationHeading[NUM_RACES][TOTAL_NUM_SHIPS];
    sdword specialDoorOrientationUp[NUM_RACES][TOTAL_NUM_SHIPS];
    real32 specialDoorInterpolationPerSecond;
} MothershipStatics;

extern CustShipHeader MothershipHeader;
extern CustShipHeader BMothershipHeader;

void crystalBeginDeath(Ship *ship, GunSoundType damageType);
void MothershipHouseKeep(Ship *ship);
bool32 MothershipSpecialOrderCleanup(Ship *ship, void *custom);
void Mothership_PreFix(Ship *ship);
void Mothership_Save(Ship *ship);
void Mothership_Load(Ship *ship);

void MothershipAttachObjectToDoor(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *object);
void mothershipCleanDoorForHSInstant(Ship *ship);
void MothershipDettachObjectFromDoor(Ship *ship);
void MothershipDoorUpKeep(Ship *ship);
void mothershipGetCargoPosition(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *cargo, vector *position, matrix *coordsys, vector *heading, vector *up,vector *right);
#endif
