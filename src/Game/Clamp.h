/*=============================================================================
    Name    : clamp.h
    Purpose : defines, prototypes etc...

    Created 5/8/1998 by bpasechn
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "spaceobj.h"
#include "shipselect.h"

//to expand clamping to others than just ships, must modify line
//in univupdate to check for more than just ships with
//clamp status

void clampObjToObj(SpaceObjRotImpTargGuidanceShipDerelict *obj, SpaceObjRotImpTargGuidanceShipDerelict *dest);
void clampObjectSetup(SpaceObj *child, SpaceObj *parent);
void clDetachShipFromCommand(Ship *ship);
void unClampObj(SpaceObjRotImpTargGuidanceShipDerelict *obj);
// Cataclysm: takes HOST (master), iterates slaves via clampInfo->selection
void updateClampedObject(SpaceObjRotImpTargGuidanceShipDerelict *host);
