// =============================================================================
//  P1Mothership.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 5/06/1998 by ddunlop
// =============================================================================

#ifndef ___P1MOTHERSHIP_H
#define ___P1MOTHERSHIP_H

#include "SpaceObj.h"

typedef struct
{
    /*
     * Retail SaveShip uses 1592 + sizeofShipSpecifics for the main ship chunk body.
     * P1 mothership retail saves carry 1916-byte ship bodies with no precise
     * selection or mad bindings, so the serialized specifics payload is 0x144 bytes.
     */
    ubyte unk[0x144];
} P1MothershipSpec;

static_assert(sizeof(P1MothershipSpec) == 0x144, "P1MothershipSpec size mismatch");

extern CustShipHeader P1MothershipHeader;
void P1MothershipRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);

#endif
