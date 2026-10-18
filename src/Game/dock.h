/*=============================================================================
    Name    : Dock.h
    Purpose : Definitions for dock.c

    Created 11/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DOCK_H
#define ___DOCK_H

#include "render.h"
#include "ShipSelect.h"
#include "SpaceObj.h"

#define DOCK_FOR_BOTHREFUELANDREPAIR    1
#define DOCK_FOR_REFUEL                 2
#define DOCK_FOR_REPAIR                 4
#define DOCK_TO_DEPOSIT_RESOURCES       8
#define DOCK_PERMANENTLY                16
#define DOCK_FOR_RESEARCH               32
#define DOCK_AT_SPECIFIC_SHIP           64
#define DOCK_FOR_REFUELREPAIR           (DOCK_FOR_REFUEL | DOCK_FOR_REPAIR)
#define DOCK_STAY_TILL_EXPLICITLAUNCH   128
#define DOCK_INSTANTANEOUSLY            256
#define DOCK_FOR_RETIRE                 512
typedef udword DockType;

#define DOCK_MODE_REFUELREPAIR                  0
#define DOCK_MODE_STAY_TILL_EXPLICITLAUNCH      1
#define DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP       2
#define DOCK_MODE_DEPOSIT_RESOURCES             3
#define DOCK_MODE_LEECH                         4
#define DOCK_MODE_TOW                           5
#define DOCK_MODE_RETIRE                        7
#define DOCK_MODE_INSTANT_REFUELREPAIR          8
#define DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH 9
#define DOCK_MODE_BEAST                         10

typedef struct
{
    DockType dockType;
    udword formationToGoIntoAfterDockingID;
    bool8 wasHarvesting;
    bool8 allDockingWithSameShip;
    bool8 allNearShip;
    bool8 haveFlyedFarEnoughAway;
} DockCommand;

typedef struct
{
    Ship *ship;
    real32 startTime;
    real32 endTime;
    sdword state;
    sdword cleanupPending;
} CrystalDockSlot;

typedef struct CrystalDockState
{
    CrystalDockSlot slots[16];
    sbyte assignedSlot;
    ubyte pad[3];
} CrystalDockState;

#if RND_VISUALIZATION
void dockDrawDockInfo(Ship *ship);
#endif

bool32 processSingleLaunchShipToDo(Ship *ship, Ship *receiverShip, sdword launchType);
sdword cmGetBuildState(unsigned int shipclass, unsigned int shiptype, int isAcolyte, udword shipFlags, int checkMode, Ship *factoryShip);
sdword dockFindDockIndex(char *name,DockStaticInfo *dockstaticinfo);
DockStaticPoint *dockFindDockStaticPoint(char *name,DockStaticInfo *dockstaticinfo);
void dockReserveDockPoint(Ship *ship, Ship *dockwith, sdword dockpointindex);
void dockPutShipInside(Ship *ship,Ship *dockwith);
void dockInitShipForLaunch(Ship *ship);
sdword dockDisableShipForLaunch(Ship *ship, sdword allowDelay);
void dockSetDockingTarget(Ship *ship, Ship *target);
void dockClearDockingTarget(Ship *ship);
void dockFreeDockPoint(Ship *ship);
void dockComputeOverideOrientation(Ship *ship, real32 *dockOffset, Ship *dockwith, DockStaticPoint *dockpoint, matrix *result);
bool32 dockFlyToDockPoint(Ship *ship, Ship *dockwith);
void playerAddResourceUnits(struct Player *player, sdword amount);
bool32 ThisShipCanDockWith(Ship *ship, Ship *dockwith, DockType dockType, bool32 checkCapacity);
bool32 dockFlyToConeOrigin(Ship *ship, Ship *dockwith);
bool32 dockFlyToConeInside(Ship *ship, Ship *dockwith, DockStaticPoint *dockstaticpoint);
sdword dockFindNearestDockPoint(Ship *ship, Ship *dockwith, sdword dockType);
bool32 dockFindAndReserveDockPoint(Ship *ship, Ship *dockwith);
sdword dockAllocDockPointActivity(Ship *ship);
bool32 dockNeedsSpecialDocking(Ship *ship, sdword dockState);
bool32 dockFlyToFinalDockPoint(Ship *ship, Ship *dockwith, real32 tolerance);
bool32 dockFlyToClampedConePosition(Ship *ship, Ship *dockwith, DockStaticPoint *dockstaticpoint);
bool32 dockFlyToOffsetDockPoint(Ship *ship, real32 *dockOffset, Ship *dockwith, DockStaticPoint *dockstaticpoint);
void dockRemoveShipFromInside(Ship *ship,Ship *dockwith);
void dockPutShipOutside(Ship *ship,Ship *creator,vector *createat,udword headingdirection,udword updirection);

void RemoveShipFromLaunching(Ship *ship);
void RemoveShipFromDocking(Ship *ship);

sdword dockPrepareSingleShipForLaunch(Ship *ship, Ship *dockship, DockType dockType);
sdword dockChangeSingleShipToDock(struct CommandToDo *command, Ship *ship, Ship *dock, bool32 wasHarvesting, DockType dockType);
void DockCleanup(struct CommandToDo *docktodo);
void LaunchCleanup(struct CommandToDo *launchtodo);
bool32 processDockToDo(struct CommandToDo *docktodo);
bool32 processLinkedLaunchToDo(struct CommandToDo *docktodo);

bool32 ShipWithinDockRange(Ship *ship,Ship *target);
Ship *FindNearestShipToDockAt(Ship *ship,DockType dockType);

sdword PreFix_CrystalDockSlots(Ship *ship);
Ship *Fix_CrystalDockSlots(Ship *ship);
sdword salCapGetWorkerOpenAnimIndex(sdword shiptype);
sdword salCapGetWorkerCloseAnimIndex(sdword shiptype);
sdword InitCrystalDockSlots(Ship *ship);
bool32 IsCrystal(Ship *ship);
bool32 dockIsSlipgateShip(Ship *ship);
sdword CrystalGetReplacementShipType(Ship *crystalShip, Ship *hostShip);
void ProcessCrystalDockSlots(Ship *ship);
void ClearCrystalDockSlots(Ship *ship);
sdword RemoveShipFromCrystalDockSlots(Ship *hostShip, Ship *crystalShip);
bool32 CrystalHasAssignedDockSlot(Ship *crystalShip);
real32 CrystalGetDockSlotProgress(Ship *crystalShip);
CrystalDockState *dockGetCrystalDockState(Ship *ship);
void dockCompleteSingleShip(Ship *ship, Ship *dockwith, struct CommandToDo *docktodo);

// specific docking for certain ships

bool32 ShipIsRefuelingAtCarrierMother(struct Ship *ship);
bool32 ShipIsWaitingForSoftLaunch(struct Ship *ship);

sdword **GetLaunchPoints(ShipStaticInfo *shipstatic,ShipStaticInfo *dockwithstatic);

DockType dockGetAppropriateTypeOfDocking(SelectCommand *selectcom);

typedef struct
{
    sdword size;
    real32 dockmisc;
    sdword docktoindex;
    sdword dockingindex;
} AtResearchShipDockInfo;

#endif
