// =============================================================================
//  ResourceCollector.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___RESOURCECOLLECTOR_H
#define ___RESOURCECOLLECTOR_H

#include "types.h"
#include "SpaceObj.h"

typedef struct
{
    sdword resourcesCollected;
} ResourceCollectorSpec;

extern CustShipHeader ResourceCollectorHeader;

#endif
