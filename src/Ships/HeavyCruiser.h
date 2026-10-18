// =============================================================================
//  HeavyCruiser.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___HEAVYCRUISER_H
#define ___HEAVYCRUISER_H

#include "SpaceObj.h"
#include "BigGun.h"

typedef struct HeavyCruiserSpec
{
    ShipBigGunState bigGunState;
    ubyte unk_18[0x144];
} HeavyCruiserSpec;

static_assert(sizeof(HeavyCruiserSpec) == 0x15C, "HeavyCruiserSpec size mismatch");

extern CustShipHeader HeavyCruiserHeader;
extern CustShipHeader BeastHeavyCruiserHeader;
bool32 HeavyCruiserAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);

#endif
