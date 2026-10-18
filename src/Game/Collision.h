/*=============================================================================
    Name    : collision.h
    Purpose : Definitions for collision.c

    Created 9/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COLLISION_H
#define ___COLLISION_H

#include "types.h"
#include "spaceobj.h"
#include "blobs.h"

real32 RangeToTarget(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory);
real32 RangeToTargetGivenDist(Ship *ship,SpaceObjRotImpTarg *target,real32 dist);
void collUpdateCollRectangle(SpaceObjRotImp *irobj);
void collDrawCollisionInfo(SpaceObjRotImp *irobj);
void collZeroRectInfo(StaticCollInfo *staticCollInfo);
real32 collCheckRayBBoxIntersection(SpaceObjRot *obj, vector *rayOrigin, vector *rayDir, real32 maxDist, sdword *hitFace);
sdword collCheckBBoxInBBox(SpaceObjRotImp *obj1, SpaceObjRotImp *obj2);
sdword collCheckBBoxBumpCollision(SpaceObjRotImp *obj1, SpaceObjRotImp *obj2);
bool32 collApplyKamikazeCollisionDamage(SpaceObjRotImpTarg *obj1, SpaceObjRotImpTarg *obj2);
real32 collCheckRectLine(SpaceObjRotImp *obj1,vector *univpoint,vector *univdir,real32 linelength,sdword *collSide);
sdword collCheckBBoxPoint(SpaceObjRotImp *obj, vector *point);
bool32 collCheckRectPoint(SpaceObjRotImp *obj1,vector *point);
bool32 collCheckRectInRect(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2);
bool32 collCheckLineOfSight(Ship* source, Ship* target, vector* sourcePosition, vector* direction);
sdword collCheckRayTriangleIntersection(vector *rayOrigin, vector *rayEnd, vector *v0, vector *v1, vector *v2, vector *normal, real32 *hitDist);
sdword collCheckRayShipCollPolygons(Ship *ship, vector *rayOrigin, vector *rayDir, real32 *hitDist);

#define collAddSpaceObjToCollBlobs(obj) bobAddObjToNearestBlob(&universe.collBlobList,obj)
#define collAddSpaceObjToSpecificBlob(tblob,tobj) bobAddObjToSpecificBlob(tblob,tobj)
//void collAddSpaceObjToCollBlobs(SpaceObj *obj);

void collUpdateCollBlobs(void);
void collUpdateObjsInCollBlobs(void);

void collCheckAllBumpCollisions(void);
void collCheckAllBulletMissileCollisions(void);

#define COLLISION_CHECK_STATS

/*=============================================================================
    Data:
=============================================================================*/
#ifdef COLLISION_CHECK_STATS

extern udword shipsavoidingstuff;
extern udword shipsavoidedwalks;
extern udword shipsavoidedchecks;

extern udword shipshipwalks;
extern udword shipresourcewalks;
extern udword shipderelictwalks;
extern udword shipshipchecks;
extern udword shipresourcechecks;
extern udword shipderelictchecks;

extern udword bulletwalks;
extern udword bulletchecks;

extern udword missilewalks;
extern udword missilechecks;

extern udword minewalks;
extern udword minechecks;

#endif

#endif
