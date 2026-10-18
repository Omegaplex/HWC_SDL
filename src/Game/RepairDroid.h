#ifndef ___REPAIRDROID_H
#define ___REPAIRDROID_H

#include "types.h"
#include "ShipSelect.h"

typedef struct RepairDroidController RepairDroidController;

bool32 repairDroidAIUpdate(Ship *ship, SelectAnyCommand *targets, real32 range);
RepairDroidController *CreateRepairDroid(Ship *parentShip, sdword count);
void deleteRepairDroidData(RepairDroidController *controller);
void repairDroidUpdate(RepairDroidController *controller);
void repairDroidRenderAttachments(RepairDroidController *controller);
void repairDroidHyperspaceReset(Ship *ship);

#endif
