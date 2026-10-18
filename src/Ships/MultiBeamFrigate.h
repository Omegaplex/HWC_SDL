// =============================================================================
//  MultiBeamFrigate.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 5/07/1998 by ddunlop
// =============================================================================

#ifndef ___MULTIBEAMFRIGATE_H
#define ___MULTIBEAMFRIGATE_H

#include "Attack.h"
#include "SpaceObj.h"

typedef struct
{
    ubyte unk_00_1F[0x20];
    AttackSideStep attackSideStep;
} MultiBeamFrigateSpec;

static_assert(offsetof(MultiBeamFrigateSpec, attackSideStep) == 0x20, "MultiBeamFrigateSpec layout mismatch");
static_assert(sizeof(MultiBeamFrigateSpec) == 0x28, "MultiBeamFrigateSpec size mismatch");

extern CustShipHeader MultiBeamFrigateHeader;

#endif
