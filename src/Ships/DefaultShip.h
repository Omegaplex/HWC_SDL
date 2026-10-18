// =============================================================================
//  DefaultShip.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___DEFAULTSHIP_H
#define ___DEFAULTSHIP_H

#include "SpaceObj.h"

extern CustShipHeader DefaultShipHeader;
extern CustShipHeader AcolyteHeader;
extern CustShipHeader MimicHeader;
extern CustShipHeader LeechHeader;
extern CustShipHeader MCVHeader;
extern CustShipHeader DreadnoughtHeader;
extern CustShipHeader RammingFrigateHeader;
extern CustShipHeader RepairDroidHeader;
extern CustShipHeader HiveFrigateHeader;
extern CustShipHeader SwarmerHeader;
extern CustShipHeader BeastSwarmerHeader;
extern CustShipHeader ProcessorHeader;
extern CustShipHeader CruiseMissileHeader;
extern CustShipHeader CargoBargeHeader;
extern CustShipHeader RKOHeader;
extern CustShipHeader NomadMoonHeader;
extern CustShipHeader NomadFieldEmitterHeader;
extern CustShipHeader DoNothingShipHeader;
extern CustShipHeader StationaryGunHeader;
extern CustShipHeader MiningBaseHeader;
extern CustShipHeader ComponentShipHeader;
extern CustShipHeader CrystalHeader;
extern CustShipHeader SlipgateHeader;

void DefaultShipAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
void DefaultShipAttackPassive(Ship *ship, Ship *target, bool32 rotate);
void DefaultShipRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);
void DefaultShipFire(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);
void shipAttackSimpleNoRotate(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);
void shipInitModuleEject(Ship *ship, bool32 deleteOnExpire, real32 startDelay, real32 activeDuration);

#endif
