// =============================================================================
//  MissileDestroyer.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___MISSILEDESTROYER_H
#define ___MISSILEDESTROYER_H

#include "SpaceObj.h"

typedef struct
{
    real32 lasttimeRegeneratedMissiles;
    real32 lasttimeFiredVolley;
    real32 lasttimeDidSpecialTargeting;
    sdword curTargetIndex;
    sdword volleyState;
} MissileDestroyerSpec;

#define VOLLEY_BEGIN        10
#define VOLLEY_NOT_BEGIN    20

extern CustShipHeader MissileDestroyerHeader;

#endif
