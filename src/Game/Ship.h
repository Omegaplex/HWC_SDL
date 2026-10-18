#ifndef __SHIP_H
#define __SHIP_H

#include "spaceobj.h"

typedef struct ShipNameReservationEntry
{
    udword ship;
    udword unk04;
    udword shipType;
    union
    {
        udword unk0C[25];
        char name[100];
    };
} ShipNameReservationEntry;

bool32 shipIsNaggarok(Ship *ship);
bool32 shipIsCloaking(Ship *ship);
real64 shipGetMaxSpeed(Ship *ship);
extern ShipNameReservationEntry shipNameReservationStorage[100];
extern ShipNameReservationEntry *shipNameReservations;
extern sdword shipNameReservationCount;
void shipNameReserve(Ship *ship);
void shipNameRelease(Ship *ship);
sdword shipNamesStartup(void);
void shipApplyEMPShock(Ship *ship, sdword duration);
udword shipSetDisabledState(Ship *ship, sdword disabled);
void shipSetDelayedDeath(Ship *ship, real32 duration);
void shipClearDelayedDeath(Ship *ship);
void shipProcessModuleSlotOrder(Ship *ship, sdword orderType, ShipType targetType, real32 param3, real32 param4);
bool32 shipGetWaitingForShipsPoints(Ship *ship, vector *fromPoint, vector *toPoint);
void shipStartWaitingForShips(Ship *ship, vector *position);
bool32 shipWaitingForShipsDone(Ship *ship);
bool32 shipWaitingForShipsActive(Ship *ship);
sdword shipGetModuleSlotDisplayCount(Ship *ship);
sdword shipGetModuleSlotIconId(Ship *ship, sdword moduleType);
bool32 shipHasModuleSlotState(Ship *ship);
Effect *shipGetModuleSlotEffect(Ship *ship);
sdword shipGetModuleSlotTimer(Ship *ship);
void shipDecrementModuleSlotTimer(Ship *ship);
sdword shipGetModuleCount(Ship *ship);
sdword shipCanStartModuleSlotTimer(Ship *ship);
sdword shipTryStartModuleSlotTimer(Ship *ship);
lodinfo *shipGetLodInfo(SpaceObj *obj);
real32 shipGetMinZoomDistance(SpaceObj *obj);
real32 shipGetCollSphereSize(SpaceObj *obj);
real32 shipGetNLips(SpaceObj *obj);
bool32 shipCanFireNextGunAtTarget(Ship *ship, SpaceObjRotImpTarg *target);
void *shipAttackStaticInfoScript(char *directory, char *filename, ShipStaticInfo *statinfo);
Ship *shipResetAttackState(Ship *ship);
ubyte *shipGetAttackRecord(Ship *ship, SpaceObjRotImpTarg *target);
void shipShootNextGunAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
void shipRotateAndShootAtTarget(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);
void shipSharedRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);
bool32 shipStraightForwardAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
bool32 shipSharedFlightManAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
bool32 shipModuleDisableCallback(Ship *ship);
bool32 shipModuleSlotProcessSpecial(Ship *ship, void *command);
void shipResetModuleSlots(Ship *ship);
void shipFreeModuleSlotResources(Ship *ship);
Ship *shipFreeModuleData(Ship *data, Ship *ship);
void shipPreFixModuleSlotRegistryReferences(Ship *ship);
void shipFixModuleSlotRegistryReferences(Ship *ship);
bool32 shipCleanupDormantModuleSlotResources(Ship *ship);
udword shipModuleSlotResetState(Ship *ship);
bool32 shipIsAliveCarrierType(Ship *ship);
void shipCreateDisabledEffect(SpaceObj *spaceobj);
bool32 shipIsBeastRace(Ship *ship);
void *GetShipInfectionData(Ship *ship);
void workerCheckResetForHyperspace(Ship *ship);
real32 CrystalGetProcessingRemainingFraction(Ship *crystal);
void CrystalStartProcessing(Ship *crystal, vector *processingTarget, real32 duration);
void CrystalUpdateProcessingTarget(Ship *crystal, vector *processingTarget);
sdword shipGetModuleFlashLightIndex(Ship *ship);
Ship *GetFirstInsideShip(Ship *ship);
Ship *GetNextInsideShip(void);
Ship *shipGetCompoundAABB(Ship *ship, real32 *compoundAABB);
real64 shipDistanceToCompoundAABB(real32 *point, Ship *ship);
Ship *shipClearSwarmers(Ship *ship);

#endif
