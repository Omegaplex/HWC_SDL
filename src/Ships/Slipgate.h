// =============================================================================
//  Slipgate.h
// =============================================================================

#ifndef ___SLIPGATE_H
#define ___SLIPGATE_H

#include "ShipSelect.h"
#include "render.h"

typedef struct
{
    Ship *partner;
    ubyte state;
    ubyte pad0[3];
    real32 approachStartTime;
    Effect *effect;
    real32 permanentEffectRadius;
    real32 transientEffectRadius;
    udword slipgateColor;
    ubyte playerTransitState[8];
    GrowSelection queuedShips;
    real32 stateExpiryTime;
} SlipgateShipSpec;

Effect *slipgateSetState(Ship *ship, ubyte state);
bool32 slipgateActivateApproach(Ship *ship);
bool32 slipgateActivateBeam(Ship *ship);
Ship *slipgateSetPartner(Ship *ship, Ship *partner);
Ship *slipgateGetPartner(Ship *ship);
ubyte slipgateGetState(Ship *ship);
void slipgateMarkPlayerTransit(Ship *ship, sdword playerIndex);
void slipgateAddCaptureTarget(Ship *ship, Ship *captureTarget);
void slipgateRequestDock(Ship *dockwith, Ship *ship);

#endif
