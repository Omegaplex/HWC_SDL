// =============================================================================
//  StandardDestroyer.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 01/06/1998 by bpasechnik
// =============================================================================

#ifndef ___STANDARDDESTROYER_H
#define ___STANDARDDESTROYER_H

#include "types.h"
#include "SpaceObj.h"

typedef struct
{
    ubyte unk[0xD30];
} StandardDestroyerSpec;

static_assert(sizeof(StandardDestroyerSpec) == 0xD30, "StandardDestroyerSpec size mismatch");

extern CustShipHeader StandardDestroyerHeader;

#endif
