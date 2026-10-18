// =============================================================================
//  Carrier.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___CARRIER_H
#define ___CARRIER_H

#include "SpaceObj.h"
#include "Types.h"
#include "BigGun.h"

typedef struct
{
    ShipBigGunState bigGunState;
    SpaceObjRotImpTargGuidanceShipDerelict *droptarget[MAX_NUM_DROP];
    sdword dropstate[MAX_NUM_DROP];
    sdword dockindex[MAX_NUM_DROP];
    sdword CAP_NumInBuildCue;
    /*
     * Cataclysm overlays ShipSpecifics[0x00..0x17] with big-gun runtime/save state.
     * The remaining 336 bytes preserve the retail 360-byte carrier specifics
     * block used by old saves.
     */
#define CATA_CARRIER_MAX_CAPTURABLE 22
    sdword CAPshiptype[CATA_CARRIER_MAX_CAPTURABLE];
    sdword CAPshiprace[CATA_CARRIER_MAX_CAPTURABLE];
    real32 CAPTimeToBuildShip[CATA_CARRIER_MAX_CAPTURABLE];
    sdword dockAnimationState;
    real32 dockAnimationTime;
} CarrierSpec;

static_assert(offsetof(CarrierSpec, dockAnimationState) == 0x160, "CarrierSpec dockAnimationState offset mismatch");
static_assert(offsetof(CarrierSpec, dockAnimationTime) == 0x164, "CarrierSpec dockAnimationTime offset mismatch");
static_assert(sizeof(CarrierSpec) == 360, "CarrierSpec size mismatch");

extern CustShipHeader CarrierHeader;
void shipShootAllGunsAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
void shipBigGunNotifyObjectDeleted(Ship *ship, void *deadObject);

#endif
