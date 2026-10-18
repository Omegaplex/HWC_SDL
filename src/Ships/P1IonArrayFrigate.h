// =============================================================================
//  P1IonArrayFrigate.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 5/06/1998 by ddunlop
// =============================================================================

#ifndef ___P1IONARRAYFRIGATE_H
#define ___P1IONARRAYFRIGATE_H

#include "SpaceObj.h"

typedef struct
{
    sdword cloakMode;
    real32 cloakCharge;
    real32 cloakDrainPerTick;
    real32 decloakDrainPerTick;
    real32 nextTransitionTime;
    real32 cloakBlend;
    sdword slotStatus[MAX_MULTIPLAYER_PLAYERS];
} IonArrayFrigateSpec;

typedef IonArrayFrigateSpec P1IonArrayFrigateSpec;
typedef IonArrayFrigateSpec BeastIonArrayFrigateSpec;

static_assert(sizeof(IonArrayFrigateSpec) == 0x38);
static_assert(offsetof(IonArrayFrigateSpec, cloakMode) == 0x00);
static_assert(offsetof(IonArrayFrigateSpec, cloakCharge) == 0x04);
static_assert(offsetof(IonArrayFrigateSpec, cloakDrainPerTick) == 0x08);
static_assert(offsetof(IonArrayFrigateSpec, decloakDrainPerTick) == 0x0C);
static_assert(offsetof(IonArrayFrigateSpec, nextTransitionTime) == 0x10);
static_assert(offsetof(IonArrayFrigateSpec, cloakBlend) == 0x14);
static_assert(offsetof(IonArrayFrigateSpec, slotStatus) == 0x18);

extern CustShipHeader IonArrayFrigateHeader;

#endif
