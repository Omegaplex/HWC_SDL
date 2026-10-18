/*=============================================================================
    Name    : BigGun.h
    Purpose : Cataclysm big-gun helpers and Beast mothership targeting code
=============================================================================*/

#ifndef ___BIGGUN_H
#define ___BIGGUN_H

#include "SpaceObj.h"
#include "shipselect.h"

#define COMMAND_SIEGECANNON_TARGET_OFFSET 0x2DC

typedef struct CommandToDo CommandToDo;

typedef struct ShipBigGunState
{
    struct Ship *currentTarget;            // ShipSpecifics + 0x00
    LinkedList queuedTargets;              // ShipSpecifics + 0x04
    sdword savedState;                     // ShipSpecifics + 0x14 - serialized big-gun state dword; no proven live reader yet
    bool32 autostabilizeSuppressed;        // ShipSpecifics + 0x18 - BigGun.c uses this as a 0/1 runtime latch
} ShipBigGunState;

real32 bigGunChargeFraction(Ship *ship);
void shipBigGunInfoInit(Ship *ship);
void bigGunProcessTargetingAndFiring(Ship *ship);
bool32 shipBigGunSpecialTarget(Ship *ship, void *custom);
bool32 shipCanHaveBigGun(Ship *ship);
bool32 shipCanUseBigGun(Ship *ship);
void *shipGetBeastWeaponInfo(Ship *ship);
bool32 mothershipBigGunTimerExpired(Ship *ship);
bool32 mothershipHasReadyBigGun(Ship *ship);
real32 mothershipGetBigGunProgress(Ship *ship);
bool32 mothershipBigGunCanFire(Ship *ship);
bool32 mothershipCanFireBigGun(Ship *ship);
void *mothershipBeginBigGunTarget(Ship *ship);
bool32 mothershipBigGunAlmostComplete(Ship *ship);
bool32 MothershipSpecialActivateNoTarget(Ship *ship);
bool32 MothershipMoveOrderCleanup(Ship *ship, void *arg1, void *arg2);
bool32 MothershipAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
bool32 MothershipCanHalt(Ship *ship);
bool32 mothershipIsFacingBigGunTarget(Ship *ship);
bool32 mothershipAlignBigGun(SpaceObjRotImpTargGuidance *ship);
void mothershipBigGunUpdate(SpaceObjRotImpTargGuidance *ship);
void mothershipProcessBigGunCommand(CommandToDo *command);
void mothershipBigGunRemoveShipReferences(Ship *ship, Ship *shiptoremove);
void mothershipBigGunNotifyObjectDeleted(Ship *ship, sdword objectID);
void mothershipBigGunFix(Ship *ship);
void shipBigGunInfoClose(Ship *ship);
void shipBigGunInfoClearQueuedPointers(Ship *ship, void *objectRef, void *bulletRef);
void shipBigGunInfoClearTargetPointers(Ship *ship, void *targetRef);
Ship *bigGunFindNearestTargetInSelection(Ship *ship, SelectCommand *selection);
bool32 bigGunHasActiveOrSelectableTarget(Ship *ship, SelectCommand *selection);
bool32 bigGunChooseTargetFromSelection(Ship *ship, SelectCommand *selection);
void shipBigGunInfoClearCurrentTarget(Ship *ship);
void bigGunUpdateQueuedBullet(Bullet *bullet);
void SaveShipBigGunInfo(Ship *ship);
void LoadShipBigGunInfo(Ship *ship);
void FixShipBigGunInfo(Ship *ship);

#endif
