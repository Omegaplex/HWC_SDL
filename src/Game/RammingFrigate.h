#ifndef ___RAMMINGFRIGATE_H
#define ___RAMMINGFRIGATE_H

#include "spaceobj.h"

static inline RammingFrigateSpecifics *rammingFrigateSpecifics(Ship *ship)
{
    return (RammingFrigateSpecifics *)ship->ShipSpecifics;
}

static inline const RammingFrigateSpecifics *rammingFrigateSpecificsConst(const Ship *ship)
{
    return (const RammingFrigateSpecifics *)ship->ShipSpecifics;
}

bool32 shipIsLargeDerelict(Ship *ship);
void rammingFrigateSelectParams(char *directory, char *filename, ShipStaticInfo *statinfo);
void rammingFrigateInit(Ship *ship);
void rammingFrigateAttack(Ship *ship);
bool32 rammingFrigateReset(Ship *ship);
bool32 rammingFrigateSelectTarget(Ship *ship, void *custom);
bool32 rammingFrigateSetTarget(Ship *ship, void *custom);
bool32 rammingFrigateResetState(Ship *ship);
bool32 rammingFrigateMoveConstraint(Ship *ship, vector initialPoint, sdword pointCount, vector *points);
bool32 rammingFrigateDockStart(Ship *ship, sdword dockType, Ship *dockwith);
bool32 rammingFrigateCanHalt(Ship *ship);
void rammingFrigateResetTarget(Ship *ship);
void rammingFrigateClearTarget(Ship *ship, Ship *shipToRemove);
void rammingFrigateClearDeadRef(Ship *ship, sdword objectID);
void rammingFrigateDeleteEffect(Ship *ship);
void rammingFrigateSave(Ship *ship);
void rammingFrigateFixupPointers(Ship *ship);
bool32 shipIsRammingFrigateActive(Ship *ship);
sdword rammingFrigateCollisionCallback(SpaceObjRotImp *obj1, SpaceObjRotImp *obj2);

void rammingFrigateBlendHeadingWeighted(SpaceObjRotImp *irobj, vector *heading, real32 weight, real32 currentWeight);
void rammingFrigateSetHeading(SpaceObjRotImp *irobj, vector *heading);
void rammingFrigateBlendHeading(SpaceObjRotImp *irobj, vector *heading);
bool32 univIsPositionInBounds(vector *position);

#endif
