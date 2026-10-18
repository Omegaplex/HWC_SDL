// =============================================================================
//  RepairCorvette.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___REPAIRCORVETTE_H
#define ___REPAIRCORVETTE_H

#include "Attack.h"
#include "ShipSelect.h"

#define REPAIR_BEGIN          0
#define REPAIR_APPROACH       1
#define REPAIR_NEARING        2
#define REPAIR_STOP_ROTATION  3
#define REPAIR_DOCK_1         4
#define REPAIR_DOCK_2         5
#define REPAIR_REPAIR         6
#define REPAIR_DISENGAGE_1    7
#define REPAIR_DISENGAGE_2    8
#define REPAIR_DONE           9

typedef struct
{
    AttackSideStep attacksidestep;
    sdword repairState;
    struct Ship *target;
    sdword hyst;
    real32 disengagetime;
} RepairCorvetteSpec;

typedef struct
{
    AttackSideStepParameters sidestepParameters;
    real32 repairApproachDistance;
    real32 approachAndWaitDistance;
    real32 rotationStopDistance;
    real32 stopRotMultiplier;
    real32 sloppyRotThreshold;
    real32 dockWithRotationSpeed;
    real32 targetStartDockDistance;
    real32 startdockTolerance;
    real32 finaldockDistance;
    real32 CapitalDistanceRepairStart;
    real32 capitalShipHealthPerSecond;
    real32 AngleDotProdThreshold;
} RepairCorvetteStatics;

#define REPAIR_Begin      REPAIR_BEGIN
#define REPAIR_Approach   REPAIR_APPROACH
#define REPAIR_Nearing    REPAIR_NEARING
#define REPAIR_StopRot    REPAIR_STOP_ROTATION
#define REPAIR_Dock1      REPAIR_DOCK_1
#define REPAIR_Dock2      REPAIR_DOCK_2
#define REPAIR_Repair     REPAIR_REPAIR
#define REPAIR_Disengage1 REPAIR_DISENGAGE_1
#define REPAIR_Disengage2 REPAIR_DISENGAGE_2
#define REPAIR_Done       REPAIR_DONE
#define REPAIR_Hackwork   10

extern CustShipHeader RepairCorvetteHeader;

bool32 refuelRepairShips(Ship *ship, SelectAnyCommand *targets, real32 rangetoRefuel);
void RepairCorvetteOrderChanged(Ship *ship);
void stopRepairEffect(Ship *ship);

#endif
