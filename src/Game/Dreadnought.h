#ifndef ___DREADNOUGHT_H
#define ___DREADNOUGHT_H

#include "spaceobj.h"

bool32 dreadnoughtHasCannon(Ship *ship);
real32 dreadnoughtGetCannonTimerRemaining(Ship *ship);
void dreadnoughtStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo);
void dreadnoughtInit(Ship *ship);
void dreadnoughtHousekeep(Ship *ship);
void dreadnoughtCreateCannon(Ship *dreadnought);
void dreadnoughtReattachCannon(Ship *dreadnought);
void dreadnoughtFire(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
sdword dreadnoughtRenderCannonTimerLine(Ship *ship);
bool32 dreadnoughtSpecialActivate(Ship *ship);
bool32 dreadnoughtSpecialTargetStub(Ship *ship, void *custom);
void dreadnoughtRemoveShipReferences(Ship *ship, Ship *shiptoremove);
void dreadnoughtClose(Ship *ship);
void dreadnought_PreFix(Ship *ship);
void dreadnought_Fix(Ship *ship);

#endif
