/*=============================================================================
    Name    : Collision
    Purpose : Handles object collisions

    Created 9/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "fastmath.h"
#include "vector.h"
#include "matrix.h"
#include "spaceobj.h"
#include "mesh.h"
#include "prim3d.h"
#include "render.h"
#include "collision.h"
#include "universe.h"
#include "univupdate.h"
#include "blobs.h"
#include "SalCapCorvette.h"
#include "DefenseFighter.h"
#include "DFGFrigate.h"
#include "nis.h"
#include "tactics.h"
#include "alliance.h"
#include "singleplayer.h"
#include "SaveGame.h"
#include "Battle.h"
#include "Randy.h"
#include "RammingFrigate.h"
#include "Ship.h"

#include "SoundEvent.h"

BlobProperties collBlobProperties;
static const real32 collRayPlaneParallelEpsilon = 1.0e-7f;   /* cataclysm.exe 0x72A690 / 0x72A698 */
static const real32 collBBoxInsideNegativeEpsilon = -1.0e-6f; /* cataclysm.exe 0x72A684 */
static const real32 collBBoxInsidePositiveEpsilon = 1.0e-6f;  /* cataclysm.exe 0x72A688 */
static const real32 collCruiseMissilePartialDamageScale = 0.1f; /* cataclysm.exe 0x72A4D4 */
static const real32 collMissileFriendlyDamageScale = 0.5f;      /* cataclysm.exe 0x8AC544 */

static const real32 collKamikazeSelfDamageScale = 9999.0f;      /* cataclysm.exe 0x72A6A4 */
static const real32 collMissileRockMin = -100.0f;               /* cataclysm.exe 0x8AC3F4/0x8AC3FC/0x8AC404 */
static const real32 collMissileRockMax = 100.0f;                /* cataclysm.exe 0x8AC3F8/0x8AC400/0x8AC408 */
static const real32 collRandomScale = 1.0f / (real32)UDWORD_Max; /* cataclysm.exe 0x72A4E0 */

static bitflag32 collShipSpecialFlags(const Ship *ship)
{
    return ship->specialFlags1C8;
}


real32 univGetDamageMultiplier(sdword weaponClass, sdword armorClass);
void *shipGetBeastWeaponInfo(Ship *ship);
sdword shipQueueBeastConversion(Ship *ship, Ship *attacker);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
bool32 univObjIsDying(SpaceObj *obj);
sdword collComputeKamikazeCollisionDamage(Ship *ship, SpaceObjRotImpTarg *target, real32 *selfDamage, real32 *targetDamage, sdword *killerType, sdword *killerIndex);
sdword collCheckRayShipCollPolygons(Ship *ship, vector *rayOrigin, vector *rayDir, real32 *hitDist);
real32 collCheckRayBBoxIntersection(SpaceObjRot *obj, vector *rayOrigin, vector *rayDir, real32 maxDist, sdword *hitFace);
Effect *collProcessMissileHit(sdword hitFlag, SpaceObjRotImpTarg *target, StaticHeader *targetStaticHeader, Missile *missile, real32 hitFraction, sdword armorSegment);

static real32 collShipGetKamikazeDamageMultiplier(Ship *ship)
{
    if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EXPLOSIVE_CHARGE))
    {
        return *(real32 *)((ubyte *)ship + sizeof(Ship) + offsetof(Ship, collInfo.rectpos[5].x));
    }
    return 1.0f;
}

static bool32 collRammingFrigateSameGroup(SpaceObjRotImpTarg *obj1, SpaceObjRotImpTarg *obj2)
{
    ClampInfo *clampInfo1;
    ClampInfo *clampInfo2;
    SpaceObjRotImpTarg *host1;

    if (obj1 == obj2)
    {
        return TRUE;
    }
    if (obj1 == NULL || obj2 == NULL || obj1->objtype != OBJ_ShipType || obj2->objtype != OBJ_ShipType)
    {
        return FALSE;
    }

    clampInfo1 = ((Ship *)obj1)->clampInfo;
    if (clampInfo1 == NULL)
    {
        return FALSE;
    }

    clampInfo2 = ((Ship *)obj2)->clampInfo;
    if (clampInfo2 == NULL)
    {
        return FALSE;
    }

    host1 = clampInfo1->host;
    if (host1 != NULL)
    {
        if (host1 == clampInfo2->host)
        {
            return TRUE;
        }
        if (host1 == obj2)
        {
            return TRUE;
        }
    }

    return clampInfo2->host == obj1;
}

static udword *collMissileUnknown230(Missile *missile)
{
    return (udword *)&missile->formationLink.belongto;
}

static ubyte *collShipBeastInfectionCounter(Ship *ship)
{
    return &ship->unk_614_0;
}

static hmatrix collMeshCurrentMatrix =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};
static hmatrix collMeshMatrixStack[40];
static sdword collMeshMatrixDepth = 0;

#ifdef COLLISION_CHECK_STATS

udword shipsavoidingstuff;
udword shipsavoidedwalks;
udword shipsavoidedchecks;

udword shipshipwalks;
udword shipresourcewalks;
udword shipderelictwalks;
udword shipshipchecks;
udword shipresourcechecks;
udword shipderelictchecks;

udword bulletwalks;
udword bulletchecks;

udword missilewalks;
udword missilechecks;

udword minewalks;
udword minechecks;

#endif

/*-----------------------------------------------------------------------------
    Name        : RangeToTarget
    Description : Returns range from ship to target, taking into account collision
                  information of target.
    Inputs      : ship, target, trajectory (which is target->posinfo.position - ship->posinfo.position)
    Outputs     :
    Return      : range (in metres) to target
----------------------------------------------------------------------------*/
real32 RangeToTarget(Ship *ship,SpaceObjRotImpTarg *target,vector *trajectory)
{
    real32 dist;

    dist = fsqrt(vecMagnitudeSquared(*trajectory));
    dist -= target->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    if (dist > 0.0f)
    {
        return dist;
    }
    else
    {
        return 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RangeToTargetGivenDist
    Description : Returns range from ship to target, taking into account collision
                  information of target.
                  Takes the additional parameter dist if it has been pre-calculated
    Inputs      : ship, target, trajectory (which is target->posinfo.position - ship->posinfo.position), dist
    Outputs     :
    Return      : range (in metres) to target
----------------------------------------------------------------------------*/
real32 RangeToTargetGivenDist(Ship *ship,SpaceObjRotImpTarg *target,real32 dist)
{
    dist -= target->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    if (dist > 0.0f)
    {
        return dist;
    }
    else
    {
        return 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collUpdateCollRectangle
    Description : updates collision rectangle information
    Inputs      : irobj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collUpdateCollRectangle(SpaceObjRotImp *irobj)
{
    StaticCollInfo *staticCollInfo;
    CollInfo *collInfo = &irobj->collInfo;

    staticCollInfo = &irobj->staticinfo->staticheader.staticCollInfo;
    vector upvector;
    vector rightvector;
    vector forwardvector;

    if (staticCollInfo->uplength == 0)
    {
        return;         // don't bother for all-zero rectangles
    }

    matMultiplyMatByVec(&collInfo->rectpos[0],&(irobj->rotinfo.coordsys),&staticCollInfo->collrectoffset);

    matGetVectFromMatrixCol1(upvector,irobj->rotinfo.coordsys);
    matGetVectFromMatrixCol2(rightvector,irobj->rotinfo.coordsys);
    matGetVectFromMatrixCol3(forwardvector,irobj->rotinfo.coordsys);

    vecMultiplyByScalar(upvector,staticCollInfo->uplength);
    vecMultiplyByScalar(rightvector,staticCollInfo->rightlength);
    vecMultiplyByScalar(forwardvector,staticCollInfo->forwardlength);

    vecAdd(collInfo->rectpos[1],collInfo->rectpos[0],rightvector);
    vecAdd(collInfo->rectpos[2],collInfo->rectpos[1],forwardvector);
    vecAdd(collInfo->rectpos[3],collInfo->rectpos[0],forwardvector);

    vecAdd(collInfo->rectpos[4],collInfo->rectpos[0],upvector);
    vecAdd(collInfo->rectpos[5],collInfo->rectpos[1],upvector);
    vecAdd(collInfo->rectpos[6],collInfo->rectpos[2],upvector);
    vecAdd(collInfo->rectpos[7],collInfo->rectpos[3],upvector);
}

#define LEFT    -1
#define MIDDLE  0
#define RIGHT   1

/*-----------------------------------------------------------------------------
    Name        : collCheckRectLine
    Description : checks for a collision between a rectangle and a line of length linelength
    Inputs      : obj1 (object to check collision with), univpoint (origin of line in universe coordinates),
                  univdir (unit vector direction of line in universe coordinates), linelength
    Outputs     : collSide
                  if hit on a side, up == 0 (TRANS_UP), down == 1, right == 2, left == 3, forward == 4, back == 5
                  otherwise collSide == -1
    Return      : -1.0 if line misses, 0.0 if line origin is inside rectangle, else +x if line collides where x is the distance
                  from the line origin to the collision point
----------------------------------------------------------------------------*/
real32 collCheckRectLine(SpaceObjRotImp *obj1,vector *univpoint,vector *univdir,real32 linelength,sdword *collSide)
{
    StaticCollInfo *staticCollInfo = &obj1->staticinfo->staticheader.staticCollInfo;
    vector pointInWorld;
    real32 minB[3];
    real32 maxB[3];
    real32 origin[3];
    real32 dir[3];
    sdword quadrant[3];
    real32 maxT[3];
    real32 candidatePlane[3];
    real32 coord;
    real32 largestMaxT;
    sdword i;
    sdword whichPlane;
    bool32 inside;

    *collSide = -1;

    if (staticCollInfo->uplength == 0)
    {
        return 0.0f;            // if no rectangle information present, assume collision
    }

    // axis aligned bounding box in object co-ordinates
    minB[0] = staticCollInfo->collrectoffset.x;
    minB[1] = staticCollInfo->collrectoffset.y;
    minB[2] = staticCollInfo->collrectoffset.z;
    maxB[0] = minB[0] + staticCollInfo->uplength;
    maxB[1] = minB[1] + staticCollInfo->rightlength;
    maxB[2] = minB[2] + staticCollInfo->forwardlength;

    // convert univpoint,univdir from universe to object co-ordinates

    vecSub(pointInWorld,*univpoint,obj1->posinfo.position);
    matMultiplyVecByMat((vector *)origin,&pointInWorld,&obj1->rotinfo.coordsys);

    // now we have minB, maxB, origin, dir in object co-ordinates
    // use a ray-box intersection algorithm as found in Graphics Gems I

    inside = TRUE;

    for (i=0;i<3;i++)
    {
        if (origin[i] < minB[i])
        {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = FALSE;
        }
        else if (origin[i] > maxB[i])
        {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = FALSE;
        }
        else
        {
            quadrant[i] = MIDDLE;
        }
    }

    if (inside)
    {
        return 0.0f;
    }

    if (vecDotProduct(pointInWorld,*univdir) > 0.0f)
    {
        return -1.0f;
    }

    matMultiplyVecByMat((vector *)dir,univdir,&obj1->rotinfo.coordsys);

    // calculate distances to candidate planes
    for (i=0;i<3;i++)
    {
        if ((quadrant[i] != MIDDLE) && (dir[i] != 0.0f))
        {
            maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
        }
        else
        {
            maxT[i] = -1.0f;
        }
    }

    // find largest maxT for final choice of intersection
    whichPlane = 0;
    for (i=1;i<3;i++)
    {
        if (maxT[i] > maxT[whichPlane])
        {
            whichPlane = i;
        }
    }

    largestMaxT = maxT[whichPlane];
    if (largestMaxT < 0.0f)
    {
        return -1.0f;
    }

    if (largestMaxT > linelength)
    {
        return -1.0f;
    }

    // Check final candidate if it is actually inside box
    for (i=0;i<3;i++)
    {
        if (whichPlane == i)
        {
            continue;
        }

        coord = origin[i] + largestMaxT * dir[i];

        if ((coord <= minB[i]) || (coord >= maxB[i]))
        {
            return -1.0f;
        }
    }

    // Set collSide baesd on where the plane intersected the box:
    // up == 0 (TRANS_UP), down == 1, right == 2, left == 3, forward == 4, back == 5
    *collSide = (whichPlane<<1) + ((quadrant[whichPlane] == LEFT) ? 1 : 0);
    dbgAssert((*collSide >= 0) && (*collSide < NUM_TRANS_DEGOFFREEDOM));
    return largestMaxT;
}

/*-----------------------------------------------------------------------------
    Name        : collCheckRectPoint
    Description : checks if collision between rectangle of obj1 and point (if point is inside obj1 rectangle)
    Inputs      : obj1, point
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 collCheckRectPoint(SpaceObjRotImp *obj1,vector *point)
{
    vector rect0;
    vector rect6;

    vector upvector;
    vector rightvector;
    vector forwardvector;

    vector rect0topoint;
    vector rect6topoint;

    if (obj1->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return TRUE;            // if no rectangle information present, assume collision
    }

    vecAdd(rect0,obj1->collInfo.rectpos[0],obj1->posinfo.position);
    vecAdd(rect6,obj1->collInfo.rectpos[6],obj1->posinfo.position);

    matGetVectFromMatrixCol1(upvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol2(rightvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol3(forwardvector,obj1->rotinfo.coordsys);

    vecSub(rect0topoint,*point,rect0);
    vecSub(rect6topoint,*point,rect6);

    // reject if point is forward of forward face
    if (vecDotProduct(forwardvector,rect6topoint) > 0.0f)
    {
        return FALSE;
    }

    // reject if point is behind behind face
    if (vecDotProduct(forwardvector,rect0topoint) < 0.0f)
    {
        return FALSE;
    }

    // reject if point is right of right face
    if (vecDotProduct(rightvector,rect6topoint) > 0.0f)
    {
        return FALSE;
    }

    // reject if point is left of left face
    if (vecDotProduct(rightvector,rect0topoint) < 0.0f)
    {
        return FALSE;
    }

    // reject if point is above up face
    if (vecDotProduct(upvector,rect6topoint) > 0.0f)
    {
        return FALSE;
    }

    // reject if point is below bottom face
    if (vecDotProduct(upvector,rect0topoint) < 0.0f)
    {
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : collCheckRectInRect
    Description : checks if rectangle obj1 is in obj2
    Inputs      : obj1, obj2
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 collCheckRectInRect(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2)
{
    vector rect0;
    vector rect6;

    vector upvector;
    vector rightvector;
    vector forwardvector;

    vector rect0topoint;
    vector rect6topoint;

    vector obj2point;

    udword i;
    bool32 pointinside = FALSE;

    matGetVectFromMatrixCol1(upvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol2(rightvector,obj1->rotinfo.coordsys);
    matGetVectFromMatrixCol3(forwardvector,obj1->rotinfo.coordsys);

    vecAdd(rect0,obj1->collInfo.rectpos[0],obj1->posinfo.position);
    vecAdd(rect6,obj1->collInfo.rectpos[6],obj1->posinfo.position);

    for (i=0;i<8;i++)
    {
        vecAdd(obj2point,obj2->collInfo.rectpos[i],obj2->posinfo.position);

        vecSub(rect0topoint,obj2point,rect0);
        vecSub(rect6topoint,obj2point,rect6);

        // reject if point is forward of forward face
        if (vecDotProduct(forwardvector,rect6topoint) > (real32)1e-6)
        {
            continue;
        }

        // reject if point is behind behind face
        if (vecDotProduct(forwardvector,rect0topoint) < (real32)-1e-6)
        {
            continue;
        }

        // reject if point is right of right face
        if (vecDotProduct(rightvector,rect6topoint) > (real32)1e-6)
        {
            continue;
        }

        // reject if point is left of left face
        if (vecDotProduct(rightvector,rect0topoint) < (real32)-1e-6)
        {
            continue;
        }

        // reject if point is above up face
        if (vecDotProduct(upvector,rect6topoint) > (real32)1e-6)
        {
            continue;
        }

        // reject if point is below bottom face
        if (vecDotProduct(upvector,rect0topoint) < (real32)-1e-6)
        {
            continue;
        }

        pointinside = TRUE;
        break;
    }

    return pointinside;
}

/*-----------------------------------------------------------------------------
    Name        : collCheckRectBumpCollision
    Description : checks if two rectangles are colliding
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 collCheckRectBumpCollision(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2)
{
    sdword collSide;
    vector sourcePosition;
    vector univdir;
    real32 linelength;
    real32 temp;

    if (obj1->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return TRUE;            // if no rectangle information present, assume collision
    }

    if (obj2->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return TRUE;            // if no rectangle information present, assume collision
    }

    if (collCheckRectInRect(obj1,obj2) || collCheckRectInRect(obj2,obj1))
    {
        return TRUE;
    }

    // now check case if both rectangles are colliding, but points are not inside each other
    vecAdd(sourcePosition,obj1->collInfo.rectpos[0],obj1->posinfo.position);

    vecSub(univdir,obj1->collInfo.rectpos[6],obj1->collInfo.rectpos[0]);

    linelength = obj1->staticinfo->staticheader.staticCollInfo.diagonallength;
    vecDivideByScalar(univdir,linelength,temp);

    if (collCheckRectLine(obj2,&sourcePosition,&univdir,linelength,&collSide) >= 0.0f)
    {
        return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : collZeroRectInfo
    Description : zero's out all rectangular information so it's not used
    Inputs      : irobj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collZeroRectInfo(StaticCollInfo *staticCollInfo)
{
    staticCollInfo->collrectoffset.x = 0.0f;
    staticCollInfo->collrectoffset.y = 0.0f;
    staticCollInfo->collrectoffset.z = 0.0f;

    staticCollInfo->uplength = 0.0f;
    staticCollInfo->rightlength = 0.0f;
    staticCollInfo->forwardlength = 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : collDrawCollisionInfo
    Description : draws all collision info for debugging purposes
    Inputs      : irobj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collDrawCollisionInfo(SpaceObjRotImp *irobj)
{
    vector position;
    vector univrectpos[8];
    udword i;

    if (irobj->staticinfo->staticheader.staticCollInfo.uplength == 0)
    {
        return;         // don't bother for all-zero rectangles
    }

    position = irobj->posinfo.position;

    for (i=0;i<8;i++)
    {
        vecAdd(univrectpos[i],position,irobj->collInfo.rectpos[i]);
    }

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    primLine3(&univrectpos[0],&univrectpos[1],colRGB(255,255,255));
    primLine3(&univrectpos[1],&univrectpos[2],colRGB(255,0,0));
    primLine3(&univrectpos[2],&univrectpos[3],colRGB(0,255,0));
    primLine3(&univrectpos[3],&univrectpos[0],colRGB(0,0,255));

    primLine3(&univrectpos[4],&univrectpos[5],colRGB(200,200,200));
    primLine3(&univrectpos[5],&univrectpos[6],colRGB(255,255,0));
    primLine3(&univrectpos[6],&univrectpos[7],colRGB(0,255,255));
    primLine3(&univrectpos[7],&univrectpos[4],colRGB(255,0,255));

    primLine3(&univrectpos[0],&univrectpos[4],colRGB(50,50,50));
    primLine3(&univrectpos[1],&univrectpos[5],colRGB(50,50,50));
    primLine3(&univrectpos[2],&univrectpos[6],colRGB(50,50,50));
    primLine3(&univrectpos[3],&univrectpos[7],colRGB(50,50,50));

    rndLightingEnable(TRUE);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collAddSpaceObjToCollBlobs(SpaceObj *obj)
{
    bobAddObjToNearestBlob(&universe.collBlobList,obj);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : collUpdateCollBlobs
    Description : Updates collision blobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collUpdateCollBlobs(void)
{
    if (collUpdateAllBlobs)
    {
        collUpdateAllBlobs = FALSE;
        bobListDelete(&universe.collBlobList);
    }
    bobListCreate(&collBlobProperties, &universe.collBlobList, 0);

}


/*-----------------------------------------------------------------------------
    Name        : collUpdateObjsInCollBlobs
    Description : Updates objects in collision blobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collUpdateObjsInCollBlobs(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;

    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);

        bobUpdateObjsInBlobCollInfo(thisBlob);

        blobnode = blobnode->next;
    }
}

//function "fixes" a ships velocity and forces such that
//a collision looks good, as opposed to physically correct...

void kamikazeVelocityFix(SpaceObjRotImpTarg *obj1, SpaceObjRotImpTarg *obj2)
{

}

/*-----------------------------------------------------------------------------
    Name        : collCheckShipShipColl
    Description : checks collisions between ships and ships, and impacts a force
                  on each one (equal and opposite forces)
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckShipShipColl(blob *thisBlob,bool32 checkSmallShips)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Ship *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 checkdist;

    SelectCommand *selection;
    sdword numShips;

    real32 maxShipCollSphereSize;

    if (checkSmallShips)
    {
        selection = thisBlob->blobSmallShips;
        maxShipCollSphereSize = thisBlob->blobMaxSmallShipCollSphereSize;
    }
    else
    {
        selection = thisBlob->blobBigShips;
        maxShipCollSphereSize = thisBlob->blobMaxBigShipCollSphereSize;
    }
    numShips = selection->numShips;

    while (obj1index < numShips)
    {
        obj1 = selection->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxShipCollSphereSize;

        obj2index = obj1index + 1;
        while (obj2index < numShips)
        {
            obj2 = selection->ShipPtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipshipwalks++;
#endif

            checkdist = obj2->collOptimizeDist - obj1->collOptimizeDist;

            if (checkdist > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (checkdist > colldist)
            {
                goto nocollision;
            }

            //old docking rejection code

            if ((shipDockingCollisionTarget(obj1) == obj2) || (shipDockingCollisionTarget(obj2) == obj1))
            {
                goto nocollision;       // docking ships shouldn't collide
            }

            if (shipDockingCollisionTarget(obj1) != NULL && shipDockingCollisionTarget(obj2) != NULL)
            {   //special for slaved ships in DOCKING process...silly I know
                goto nocollision;
            }

            if (bitTest(obj1->flags, SOF_Clamped) &&
                obj1->clampInfo != NULL &&
                (SpaceObjRotImpTarg *)obj2 == obj1->clampInfo->host)
            {
                goto nocollision;
            }
            if (bitTest(obj2->flags, SOF_Clamped) &&
                obj2->clampInfo != NULL &&
                (SpaceObjRotImpTarg *)obj1 == obj2->clampInfo->host)
            {
                goto nocollision;
            }

            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipshipchecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            // Retail collCheckShipShipColl (0x43ade0) tests specialFlags1C8 bit 0x80000
            // (BIT19), not SPECIAL_IsASalvager (BIT20 of the unrelated legacy specialFlags
            // word), and reads the target/noAvoid fields through SalvagerFunc() rather than
            // a direct SalCapCorvetteSpec cast, since SalvagerFunc() resolves to different
            // ship-type-specific offsets (sWorker/bWorker/p1StandardCorvette all route
            // through here too, not just SalCap corvettes).
            if (bitTest(collShipSpecialFlags(obj1), BIT19))
            {
                if (*(Ship **)(SalvagerFunc(obj1) + 4) == obj2)
                {
                    if (*(sdword *)(SalvagerFunc(obj1) + 16))
                    {
                        goto nocollision;
                    }
                }
            }
            if (bitTest(collShipSpecialFlags(obj2), BIT19))
            {
                if (*(Ship **)(SalvagerFunc(obj2) + 4) == obj1)
                {
                    if (*(sdword *)(SalvagerFunc(obj2) + 16))
                    {
                        goto nocollision;
                    }
                }
            }

            if ((obj1->flags|obj2->flags) & SOF_HideForAvoidance)
            {
                goto nocollision;
            }

            if(obj1->clampInfo != NULL && obj1->clampInfo->host != NULL)
            {
                //fix later...maybe can assume obj ios a ship?
                //no a big speed concern...not A LOT of clamping going on
                if(obj1->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(ShipTypeIsSalCapCorvetteType(((Ship *)obj1->clampInfo->host)->shiptype))
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj1->clampInfo->host)->ShipSpecifics)->dockwith == obj2)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                }
                if(obj2->clampInfo != NULL && obj2->clampInfo->host != NULL)
                {
                    if(shipDockingCollisionTarget((Ship *)obj2->clampInfo->host) != NULL)
                    {
                        if(shipDockingCollisionTarget((Ship *)obj2->clampInfo->host) == shipDockingCollisionTarget(obj1))
                        {
                            //docking with same ships...
                            goto nocollision;
                        }
                    }
                    if(obj2->clampInfo->host == obj1->clampInfo->host)
                    {
                        //same host, so don't do collision detection
                        goto nocollision;
                    }
                }
                else if(shipDockingCollisionTarget((Ship *)obj1->clampInfo->host) != NULL)
                {
                    if (shipDockingCollisionTarget((Ship *)obj1->clampInfo->host) == obj2)
                    {
                        goto nocollision;
                    }
                    if (shipDockingCollisionTarget((Ship *)obj1->clampInfo->host) == shipDockingCollisionTarget(obj2))
                    {
                        goto nocollision;
                    }
                }
            }
            if(obj2->clampInfo != NULL && obj2->clampInfo->host != NULL)
            {
                if(obj2->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(ShipTypeIsSalCapCorvetteType(((Ship *)obj2->clampInfo->host)->shiptype))
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj2->clampInfo->host)->ShipSpecifics)->dockwith == obj1)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                    if(obj1->clampInfo != NULL && obj1->clampInfo->host != NULL)
                    {
                        if(((Ship *)obj1->clampInfo->host)->cataBusyStateWindow.dockingship != NULL)
                        {
                            if(shipSharesDockingTarget((Ship *)obj1->clampInfo->host, obj2))
                            {
                                //docking with same ships...
                                goto nocollision;
                            }
                        }
                    }
                    else if(shipDockingTarget((Ship *)obj2->clampInfo->host) != NULL)
                    {
                        if (shipDockingTarget((Ship *)obj2->clampInfo->host) == obj1)
                        {
                            goto nocollision;
                        }
                        if (shipSharesDockingTarget((Ship *)obj2->clampInfo->host, obj1))
                        {
                            goto nocollision;
                        }
                    }
                }
            }
            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if (collCheckBBoxBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                        dist = 10.0f;
                    }

                    if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                    {
                        if(!collApplyKamikazeCollisionDamage((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))
                        {
                            rammingFrigateCollisionCallback((SpaceObjRotImp *)obj1, (SpaceObjRotImp *)obj2);

                            if (obj1->playerowner == obj2->playerowner)
                            {
                                goto dontapplydamage;      // don't apply bonk damage for own ships
                            }

                            // Retail collCheckShipShipColl (0x43ade0, 0x43b382-392) tests
                            // SOF_Slaveable (bit 0x800000) here, not SOF_Disabled -- confirmed
                            // via disasm "test [esi+14h],eax" against the literal 0x800000
                            // (Hex-Rays mis-renders this immediate as "&unk_800000" since a
                            // coincidental data symbol exists at that address). This matches
                            // the same bit reuse already documented for
                            // univSetupShipForControl in SpaceObj.h.
                            if ((obj1->flags & SOF_Slaveable) || (obj2->flags & SOF_Slaveable))
                            {
                                goto dontapplydamage;
                            }

                            if(ShipTypeIsSalCapCorvetteType(obj1->shiptype) || obj1->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj1->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj2))
                                {
                                    goto dontapplydamage;
                                }
                            }
                            else if(ShipTypeIsSalCapCorvetteType(obj2->shiptype) || obj2->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj2->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj1))
                                {
                                    goto dontapplydamage;
                                }
                            }

                            // Retail collCheckShipShipColl (0x43b3d9-0x43b414): the Beast-infection
                            // exemption is asymmetric, not a symmetric && of both ships. If obj1 is
                            // Beast-race, obj2's infection status is never even checked: obj1 alone
                            // gates the skip (infected -> skip univHandleShipCollision entirely,
                            // not-infected -> always proceed regardless of obj2). Only when obj1 is
                            // NOT Beast-race does obj2 get its own independent infection check.
                            if (shipIsBeastRace(obj1))
                            {
                                if (!(*((sdword *)GetShipInfectionData(obj1) + 18)))
                                {
                                    if (!shipIsNaggarok(obj1) && !shipIsNaggarok(obj2))
                                    {
                                        univHandleShipCollision((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2);
                                    }
                                }
                            }
                            else if (!shipIsBeastRace(obj2) || !(*((sdword *)GetShipInfectionData(obj2) + 18)))
                            {
                                if (!shipIsNaggarok(obj1) && !shipIsNaggarok(obj2))
                                {
                                    univHandleShipCollision((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2);
                                }
                            }

dontapplydamage:
                            univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist);
                        }
                        else
                        {
                            //fix collided ships velocity....
                            univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist);
                        }

                    }

                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckBigShipSmallShipColl
    Description : checks collisions between bigships and smallships, and impacts a force
                  on each one (equal and opposite forces)
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckBigShipSmallShipColl(blob *thisBlob)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Ship *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 checkdist;

    SelectCommand *selection1 = thisBlob->blobBigShips;
    sdword numShips1 = selection1->numShips;

    SelectCommand *selection2 = thisBlob->blobSmallShips;
    sdword numShips2 = selection2->numShips;

    real32 maxShipCollSphereSize = thisBlob->blobMaxSmallShipCollSphereSize;

    sdword bottom,top;

    if (numShips2 == 0)
    {
        return;
    }

    while (obj1index < numShips1)
    {
        obj1 = selection1->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxShipCollSphereSize;

        // binary search targetindex such that
        // (obj1->collOptimizeDist - obj2->collOptimizeDist) <= maxdistCollPossible
        bottom = 0;
        top = numShips2-1;
        while (top >= bottom)
        {
            obj2index = (bottom+top)>>1;
            if ((obj1->collOptimizeDist - selection2->ShipPtr[obj2index]->collOptimizeDist) > maxdistCollPossible)
            {
                // targetindex is too small
                bottom = obj2index+1;
            }
            else
            {
                top = obj2index-1;
            }
        }

        dbgAssert(obj2index >= 0);
        dbgAssert(obj2index < numShips2);

        while (obj2index < numShips2)
        {
            obj2 = selection2->ShipPtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipshipwalks++;
#endif

            checkdist = obj2->collOptimizeDist - obj1->collOptimizeDist;

            if (checkdist > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (checkdist > colldist)
            {
                goto nocollision;
            }

            //old docking rejection code

            if (bitTest(obj1->flags, SOF_Clamped) &&
                obj1->clampInfo != NULL &&
                (SpaceObjRotImpTarg *)obj2 == obj1->clampInfo->host)
            {
                goto nocollision;
            }
            if (bitTest(obj2->flags, SOF_Clamped) &&
                obj2->clampInfo != NULL &&
                (SpaceObjRotImpTarg *)obj1 == obj2->clampInfo->host)
            {
                goto nocollision;
            }

            if (obj1->clampInfo != NULL && obj1->clampInfo->host != NULL)
            {
                if (obj2->clampInfo != NULL && obj2->clampInfo->host != NULL)
                {
                    if (shipDockingCollisionTarget((Ship *)obj2->clampInfo->host) == shipDockingCollisionTarget(obj1) ||
                        obj2->clampInfo->host == obj1->clampInfo->host)
                    {
                        goto nocollision;
                    }
                }
                else
                {
                    Ship *overlayHost = shipDockingCollisionTarget((Ship *)obj1->clampInfo->host);

                    if (overlayHost != NULL &&
                        (overlayHost == obj2 || overlayHost == shipDockingCollisionTarget(obj2)))
                    {
                        goto nocollision;
                    }
                }
            }

            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipshipchecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            // Same specialFlags1C8 BIT19 / SalvagerFunc() pattern as collCheckShipShipColl
            // above; retail collCheckBigShipSmallShipColl (0x43b460) tests the identical bit.
            if (bitTest(collShipSpecialFlags(obj1), BIT19))
            {
                if (*(Ship **)(SalvagerFunc(obj1) + 4) == obj2)
                {
                    if (*(sdword *)(SalvagerFunc(obj1) + 16))
                    {
                        goto nocollision;
                    }
                }
            }
            if (bitTest(collShipSpecialFlags(obj2), BIT19))
            {
                if (*(Ship **)(SalvagerFunc(obj2) + 4) == obj1)
                {
                    if (*(sdword *)(SalvagerFunc(obj2) + 16))
                    {
                        goto nocollision;
                    }
                }
            }

            if ((obj1->flags|obj2->flags) & SOF_HideForAvoidance)
            {
                goto nocollision;
            }

            if(obj1->clampInfo != NULL && obj1->clampInfo->host != NULL)
            {
                if(obj1->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(ShipTypeIsSalCapCorvetteType(((Ship *)obj1->clampInfo->host)->shiptype))
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj1->clampInfo->host)->ShipSpecifics)->dockwith == obj2)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                }
                if(obj2->clampInfo != NULL && obj2->clampInfo->host != NULL)
                {
                    if(((Ship *)obj2->clampInfo->host)->cataBusyStateWindow.dockingship != NULL)
                    {
                        if(shipSharesDockingTarget((Ship *)obj2->clampInfo->host, obj1))
                        {
                            //docking with same ships...
                            goto nocollision;
                        }
                    }
                    if(obj2->clampInfo->host == obj1->clampInfo->host)
                    {
                        //same host, so don't do collision detection
                        goto nocollision;
                    }
                }
                else if(shipDockingTarget((Ship *)obj1->clampInfo->host) != NULL)
                {
                    if (shipDockingTarget((Ship *)obj1->clampInfo->host) == obj2)
                    {
                        goto nocollision;
                    }
                    if (shipSharesDockingTarget((Ship *)obj1->clampInfo->host, obj2))
                    {
                        goto nocollision;
                    }
                }
            }
            if(obj2->clampInfo != NULL && obj2->clampInfo->host != NULL)
            {
                if(obj2->clampInfo->host->objtype == OBJ_ShipType)
                {
                    if(ShipTypeIsSalCapCorvetteType(((Ship *)obj2->clampInfo->host)->shiptype))
                    {
                        if(((SalCapCorvetteSpec *)((Ship *)obj2->clampInfo->host)->ShipSpecifics)->dockwith == obj1)
                        {
                            //ship clamped to salcap is colliding with the ship the salcap is docking with,
                            //don't really collide
                            goto nocollision;
                        }
                    }
                }
                if(obj1->clampInfo != NULL && obj1->clampInfo->host != NULL)
                {
                    if(shipDockingCollisionTarget((Ship *)obj1->clampInfo->host) != NULL)
                    {
                        if(shipDockingCollisionTarget((Ship *)obj1->clampInfo->host) == shipDockingCollisionTarget(obj2))
                        {
                            //docking with same ships...
                            goto nocollision;
                        }
                    }
                }
                else if(shipDockingCollisionTarget((Ship *)obj2->clampInfo->host) != NULL)
                {
                    if (shipDockingCollisionTarget((Ship *)obj2->clampInfo->host) == obj1)
                    {
                        goto nocollision;
                    }

                    if (shipDockingCollisionTarget((Ship *)obj2->clampInfo->host) == shipDockingCollisionTarget(obj1))
                    {
                        goto nocollision;
                    }
                }
            }

            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if (collCheckBBoxBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                        dist = 10.0f;
                    }

                    if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                    {
                        if(!collApplyKamikazeCollisionDamage((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))
                        {
                            rammingFrigateCollisionCallback((SpaceObjRotImp *)obj1, (SpaceObjRotImp *)obj2);

                            if (obj1->playerowner == obj2->playerowner)
                            {
                                goto dontapplydamage;
                            }

                            // Retail collCheckBigShipSmallShipColl (0x43b460, 0x43ba82) tests
                            // SOF_Slaveable (bit 0x800000) here too, not SOF_Disabled -- same
                            // disasm-confirmed bit reuse as collCheckShipShipColl above.
                            if ((obj1->flags & SOF_Slaveable) || (obj2->flags & SOF_Slaveable))
                            {
                                goto dontapplydamage;
                            }

                            if(ShipTypeIsSalCapCorvetteType(obj1->shiptype) || obj1->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj1->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj2))
                                {
                                    goto dontapplydamage;
                                }
                            }
                            else if(ShipTypeIsSalCapCorvetteType(obj2->shiptype) || obj2->shiptype == JunkYardDawg)
                            {
                                SalCapCorvetteSpec *spec = (SalCapCorvetteSpec *)obj2->ShipSpecifics;
                                if(((SpaceObjRotImpTarg *)spec->noDamageTarget) == ((SpaceObjRotImpTarg *)obj1))
                                {
                                    goto dontapplydamage;
                                }
                            }

                            // Retail collCheckBigShipSmallShipColl (0x43ba82-0x43ba94): the
                            // Beast-infection exemption here is a clean symmetric && of both
                            // ships (unlike the asymmetric form in collCheckShipShipColl), and
                            // this function has NO Naggarok exemption at all.
                            if ((!shipIsBeastRace(obj1) || !(*((sdword *)GetShipInfectionData(obj1) + 18))) &&
                                (!shipIsBeastRace(obj2) || !(*((sdword *)GetShipInfectionData(obj2) + 18))))
                            {
                                univHandleShipCollision((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2);
                            }

dontapplydamage:
                            univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist);
                        }
                        else
                        {
                            //fix collided ships velocity....
                            univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2,colldist,&distvector,dist);
                        }
                    }

                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckShipResourceColl
    Description : checks collisions between ships and resources
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckShipResourceColl(blob *thisBlob)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Resource *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 distcheck;
    real32 absdistcheck;

    SelectCommand *selection = thisBlob->blobShips;
    sdword numShips = selection->numShips;

    ResourceSelection *resselection = thisBlob->blobResources;
    sdword numResources = resselection->numResources;

    real32 maxResourceCollSphereSize;

    sdword bottom,top;

    if (numResources == 0)
    {
        return;
    }

    maxResourceCollSphereSize = thisBlob->blobMaxResourceCollSphereSize;

    while (obj1index < numShips)
    {
        obj1 = selection->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxResourceCollSphereSize;

        // binary search targetindex such that
        // (obj1->collOptimizeDist - obj2->collOptimizeDist) <= maxdistCollPossible
        bottom = 0;
        top = numResources-1;
        while (top >= bottom)
        {
            obj2index = (bottom+top)>>1;
            if ((obj1->collOptimizeDist - resselection->ResourcePtr[obj2index]->collOptimizeDist) > maxdistCollPossible)
            {
                // targetindex is too small
                bottom = obj2index+1;
            }
            else
            {
                top = obj2index-1;
            }
        }

        dbgAssert(obj2index >= 0);
        dbgAssert(obj2index < numResources);

        while (obj2index < numResources)
        {
            obj2 = resselection->ResourcePtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipresourcewalks++;
#endif
            if ((distcheck = obj2->collOptimizeDist - obj1->collOptimizeDist) > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((absdistcheck = abs(distcheck)) > maxdistCollPossible)
            {
                goto nocollision;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (absdistcheck > colldist)
            {
                goto nocollision;
            }

            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipresourcechecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            distsquared = vecMagnitudeSquared(distvector);
            dist = (real32)fsqrt(distsquared);

            if (dist < colldist)
            {
                if ((bitTest(obj1->flags, 0x1000000) == 0) ||
                    (obj1->clampInfo == NULL) ||
                    ((SpaceObjRotImpTarg *)obj2 != obj1->clampInfo->host))
                {
                    if (collCheckBBoxBumpCollision((SpaceObjRotImp *)obj1,(SpaceObjRotImp *)obj2))
                    {
                        if (distsquared < 1.0f)
                        {
                            distvector.x = 10.0f;   // special case, if objects are too close together
                            distsquared = 100.0f;
                            dist = 10.0f;
                        }

                        if (!bitTest(obj1->flags, SOF_NotBumpable) && !bitTest(obj2->flags, SOF_NotBumpable))
                        {
                            if(!collApplyKamikazeCollisionDamage((SpaceObjRotImpTarg *)obj1,(SpaceObjRotImpTarg *)obj2))
                            {
                                // Retail collCheckShipResourceColl (0x43bac0, 0x43be3e-0x43be48)
                                // calls univHandleShipCollision here, not a bespoke ship/resource
                                // damage function -- confirmed via decompile.
                                if ((*(udword *)&obj1->staticinfo->canBuildShips != 7) ||
                                    ((obj1->attributesParam | obj2->attributesParam) & 0x200))
                                {
                                    univHandleShipCollision((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2);
                                }
                                if (((obj1->attributesParam | obj2->attributesParam) & 0x200) == 0)
                                {
                                    univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2, colldist, &distvector, dist);
                                }
                            }
                            else
                            {
                                // Retail (0x43be17) always applies the collision impulse after a
                                // kamikaze hit here too, unlike the two branches above which gate
                                // it on the attributesParam bit.
                                univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2, colldist, &distvector, dist);
                            }
                        }
                    }
                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckShipDerelictColl
    Description : checks collisions between ships and derelicts
    Inputs      : thisBlob
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckShipDerelictColl(blob *thisBlob)
{
    sdword obj1index = 0;
    Ship *obj1;

    sdword obj2index;
    Derelict *obj2;

    real32 dist;
    real32 distsquared;
    real32 colldist;
    vector distvector;
    real32 selfDamage;
    real32 targetDamage;
    sdword damageHow;
    sdword playerIndex;
    sdword killerType;
    sdword killerIndex;
    bool32 hit1;
    bool32 hit2;

    real32 obj1collspheresize;
    real32 maxdistCollPossible;

    real32 distcheck;
    real32 absdistcheck;

    SelectCommand *selection = thisBlob->blobShips;
    sdword numShips = selection->numShips;

    DerelictSelection *derselection = thisBlob->blobDerelicts;
    sdword numDerelicts = derselection->numDerelicts;

    real32 maxDerelictCollSphereSize;

    sdword bottom,top;

    if (numDerelicts == 0)
    {
        return;
    }

    maxDerelictCollSphereSize = thisBlob->blobMaxDerelictCollSphereSize;

    while (obj1index < numShips)
    {
        obj1 = selection->ShipPtr[obj1index];
        obj1collspheresize = obj1->staticinfo->staticheader.staticCollInfo.collspheresize;
        maxdistCollPossible = obj1collspheresize + maxDerelictCollSphereSize;

        // binary search targetindex such that
        // (obj1->collOptimizeDist - obj2->collOptimizeDist) <= maxdistCollPossible
        bottom = 0;
        top = numDerelicts-1;
        while (top >= bottom)
        {
            obj2index = (bottom+top)>>1;
            if ((obj1->collOptimizeDist - derselection->DerelictPtr[obj2index]->collOptimizeDist) > maxdistCollPossible)
            {
                // targetindex is too small
                bottom = obj2index+1;
            }
            else
            {
                top = obj2index-1;
            }
        }

        dbgAssert(obj2index >= 0);
        dbgAssert(obj2index < numDerelicts);

        while (obj2index < numDerelicts)
        {
            obj2 = derselection->DerelictPtr[obj2index];

#ifdef COLLISION_CHECK_STATS
            shipderelictwalks++;
#endif
            if ((distcheck = obj2->collOptimizeDist - obj1->collOptimizeDist) > maxdistCollPossible)
            {
                goto nextobj1;
            }

            if ((absdistcheck = abs(distcheck)) > maxdistCollPossible)
            {
                goto nocollision;
            }

            if ((obj1->posinfo.isMoving | obj2->posinfo.isMoving) == FALSE)     // assume objects not moving can't collide
            {
                goto nocollision;
            }

            colldist = obj1collspheresize + obj2->staticinfo->staticheader.staticCollInfo.collspheresize;

            if (absdistcheck > colldist)
            {
                goto nocollision;
            }

            if ((bitTest(obj1->flags, SOF_Clamped) && (SpaceObjRotImpTarg *)obj2 == obj1->clampInfo->host) ||
                (bitTest(obj2->flags, SOF_Clamped) && (SpaceObjRotImpTarg *)obj1 == obj2->clampInfo->host))
            {
                goto nocollision;
            }

            if (obj2->dockingship == obj1)
            {
                goto nocollision;
            }
            // Check for collisions between obj1 and obj2 here:

#ifdef COLLISION_CHECK_STATS
            shipderelictchecks++;
#endif

            vecSub(distvector,obj2->collInfo.collPosition,obj1->collInfo.collPosition);

            if (!isBetweenInclusive(distvector.x,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.y,-colldist,colldist))
            {
                goto nocollision;
            }

            if (!isBetweenInclusive(distvector.z,-colldist,colldist))
            {
                goto nocollision;
            }

            if (bitTest(collShipSpecialFlags(obj1), BIT19))
            {
                if (*(SpaceObjRotImpTarg **)(SalvagerFunc(obj1) + 4) == (SpaceObjRotImpTarg *)(SpaceObj *)obj2)
                {
                    /* retail calls a no-op logging stub here */
                }
            }

            if (bitTest(obj1->flags, SOF_Hide) || bitTest(obj2->flags, SOF_Hide))
            {
                goto nocollision;
            }

            distsquared = vecMagnitudeSquared(distvector);
            if (distsquared < colldist * colldist)
            {
                if (collCheckBBoxBumpCollision((SpaceObjRotImp *)obj1, (SpaceObjRotImp *)obj2))
                {
                    if (distsquared < 1.0f)
                    {
                        distvector.x = 10.0f;   // special case, if objects are too close together
                        distsquared = 100.0f;
                    }

                    if (!bitTest(obj1->flags, BIT19) && !bitTest(obj2->flags, BIT19))
                    {
                        dist = (real32)fsqrt(distsquared);
                        damageHow = 777;
                        killerType = 777;
                        playerIndex = 99;
                        killerIndex = 99;
                        selfDamage = 0.0f;
                        targetDamage = 0.0f;

                        hit1 = collComputeKamikazeCollisionDamage(obj1, (SpaceObjRotImpTarg *)obj2, &selfDamage, &targetDamage, &killerType, &killerIndex);
                        hit2 = collComputeKamikazeCollisionDamage((Ship *)obj2, (SpaceObjRotImpTarg *)obj1, &targetDamage, &selfDamage, &damageHow, &playerIndex);
                        if (hit1 || hit2)
                        {
                            ApplyDamageToTarget((SpaceObjRotImpTarg *)obj1, selfDamage, GS_LargeEnergyCannon, damageHow, playerIndex);
                            ApplyDamageToTarget((SpaceObjRotImpTarg *)obj2, targetDamage, GS_LargeEnergyCannon, killerType, killerIndex);
                        }

                        univApplyCollisionImpulse((SpaceObjRotImpTarg *)obj1, (SpaceObjRotImpTarg *)obj2, colldist, &distvector, dist);
                    }
                }
            }
nocollision:
            obj2index++;
        }
nextobj1:
        obj1index++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckLineOfSight
    Description :
    Inputs      : source - ship that's firing
                  target - potential destination (target)
                  sourcePosition - origin of the los test in worldspace
                  direction - worldspace unit direction
    Outputs     :
    Return      : TRUE if path is clear, FALSE if obscured
    todo: must check if target is closer than a "hit"
----------------------------------------------------------------------------*/
bool32 collCheckLineOfSight(Ship* source, Ship* target, vector* sourcePosition, vector* direction)
{
    sdword i, collSide;
    real32 length;
    blob* thisBlob;
    Ship* ship;
    vector distvec;
    Ship *host;
    Ship *targetHost;

    if (target == NULL)
    {
        length = REALlyBig;
    }
    else
    {
        vecSub(distvec, target->posinfo.position, *sourcePosition);
        length = fsqrt(vecMagnitudeSquared(distvec));
    }

    if (target == NULL || target->collMyBlob == source->collMyBlob)
    {
        //los in current blob
        thisBlob = source->collMyBlob;
        for (i = 0; i < thisBlob->blobShips->numShips; i++)
        {
            ship = thisBlob->blobShips->ShipPtr[i];
            if (ship == source || ship == target)
            {
                //ignore source or target
                continue;
            }

            if (ship->flags2 == 0 && ship->clampInfo != NULL)
            {
                host = (Ship *)ship->clampInfo->host;
                targetHost = (target != NULL && target->clampInfo != NULL) ? (Ship *)target->clampInfo->host : NULL;
                if (host == source ||
                    host == target ||
                    host == targetHost ||
                    ship == targetHost)
                {
                    continue;
                }
            }

            if (!univIsObjFriendly((SpaceObj *)ship, source->playerowner))
            {
                //only test with allied ships, self included
                continue;
            }

            if (collCheckRayBBoxIntersection((SpaceObjRot*)ship, sourcePosition, direction, length, &collSide) != -1.0f)
            {
                //hit something of ours
                return FALSE;
            }
        }

        //path is clear
        return TRUE;
    }
    else
    {
        //los in universe
        Node* blobnode = universe.collBlobList.head;
        vector distvector;
        real32 distsquared, v, dsqr;

        while (blobnode != NULL)
        {
            thisBlob = (blob*)listGetStructOfNode(blobnode);
            blobnode = blobnode->next;

            vecSub(distvector, thisBlob->centre, *sourcePosition);
            distsquared = vecMagnitudeSquared(distvector);
            v = vecDotProduct(distvector, *direction);
			if (v > 0)
			{
				dsqr = (thisBlob->radius * thisBlob->radius) - (distsquared - (v*v));
				if (dsqr > 0)
				{
					//line intersects blob, check all ships
					for (i = 0; i < thisBlob->blobShips->numShips; i++)
					{
						ship = thisBlob->blobShips->ShipPtr[i];
						if (ship == source || ship == target)
						{
							//ignore source or target
							continue;
						}

						if (ship->flags2 == 0 && ship->clampInfo != NULL)
						{
							host = (Ship *)ship->clampInfo->host;
							targetHost = (target != NULL && target->clampInfo != NULL) ? (Ship *)target->clampInfo->host : NULL;
							if (host == source ||
							    host == target ||
							    host == targetHost ||
							    ship == targetHost)
							{
							    continue;
							}
						}

						if (!univIsObjFriendly((SpaceObj *)ship, source->playerowner))
						{
							//only test with allied ships, self included
							continue;
						}

						if (collCheckRayBBoxIntersection((SpaceObjRot*)ship, sourcePosition, direction, length, &collSide) != -1.0f)
						{
							//hit something of ours
							return FALSE;
						}
					}
				}
			}
        }

        //path is clear
        return TRUE;
    }
}

void collCheckBeamCollWithTargetsInBlob(Bullet *bullet,blob *thisBlob,real32 *minbeamCollideLineDist,SpaceObjRotImpTarg **minbeamTarget,sdword *minbeamCollSide, sdword *minbeamPolyHit)
{
    SelectAnyCommand *targetselection;
    sdword numTargets;

    SpaceObjRotImpTarg *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 collideLineDist;

    sdword targetindex;

    sdword pass = 0;

passagain:

    if (pass == 0)
    {
        targetselection = thisBlob->blobSmallTargets;
    }
    else
    {
        dbgAssert(pass == 1);
        targetselection = thisBlob->blobBigTargets;
    }

    targetindex = 0;
    numTargets = targetselection->numTargets;

    while (targetindex < numTargets)
    {
        target = targetselection->TargetPtr[targetindex];
        targetstaticheader = &target->staticinfo->staticheader;

#ifdef COLLISION_CHECK_STATS
bulletwalks++;
#endif

        if (bullet->owner == (Ship *)target)
        {
            goto beamnexttarget;
        }

        if (target->objtype == OBJ_ShipType)
        {
            Ship *shipTarget = (Ship *)target;
            real32 hitDist;
            sdword polygonHit;

            if ((shipTarget->shiptype == sSentinel || shipTarget->shiptype == bSentinel) &&
                univObjIsDying((SpaceObj *)shipTarget) &&
                bullet->playerowner != shipTarget->playerowner)
            {
                polygonHit = collCheckRayShipCollPolygons(shipTarget,
                                                         &bullet->posinfo.position,
                                                         &bullet->bulletheading,
                                                         &hitDist);
                if (polygonHit != -1 && hitDist < *minbeamCollideLineDist)
                {
                    *minbeamCollideLineDist = hitDist;
                    *minbeamTarget = target;
                    *minbeamCollSide = 0;
                    *minbeamPolyHit = polygonHit;
                    goto beamnexttarget;
                }
            }
        }

        // Check for collisions between bullet and target here:

#ifdef COLLISION_CHECK_STATS
bulletchecks++;
#endif

        vecSub(distvector,target->collInfo.collPosition,bullet->posinfo.position);
        distsquared = vecMagnitudeSquared(distvector);
        v = vecDotProduct(distvector,bullet->bulletheading);
		if (v > 0)
        {
			dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
			if (dsqr > 0)
			{
				d = fsqrt(dsqr);
				if ((v-d) < bullet->traveldist)
				{
					if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
					{
						if (collideLineDist == 0.0f)
						{
							collideLineDist = v-d;
							if (collideLineDist < 0.0f)
							{
								collideLineDist = 0.0f;
							}
						}
						if (collideLineDist < *minbeamCollideLineDist)
						{
							*minbeamCollideLineDist = collideLineDist;
							*minbeamTarget = target;
							*minbeamCollSide = collSide;
							*minbeamPolyHit = -1;
						}
					}
				}
			}
		}

beamnexttarget:
        targetindex++;
    }

    pass++;
    if (pass < 2)
    {
        goto passagain;
    }
}

void collCheckBeamColl(blob *thisBlob,Bullet *bullet)
{
    real32 minbeamCollideLineDist = REALlyBig;
    SpaceObjRotImpTarg *minbeamTarget = NULL;
    sdword minbeamCollSide = -1;
    sdword minbeamPolyHit = -1;
    Node *blobnode = universe.collBlobList.head;
    blob *checkBlob;

    real32 collideLineDist;
    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    if(bullet->target != NULL)
    {
        switch(bullet->target->objtype)
        {
        case OBJ_MissileType:
            //do bullet/missle target collision with its target only!
            vecSub(distvector,bullet->target->collInfo.collPosition,bullet->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            v = vecDotProduct(distvector,bullet->bulletheading);
			//if (v > 0)
			{
				dsqr = bullet->target->staticinfo->staticheader.staticCollInfo.collspheresizeSqr - (distsquared - (v*v));
				if (dsqr > 0)
				{
					d = fsqrt(dsqr);
					if ((v-d) < bullet->traveldist)
					{
						if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)bullet->target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
						{
							if (collideLineDist == 0.0f)
							{
								collideLineDist = v-d;
								if (collideLineDist < 0.0f)
								{
									collideLineDist = 0.0f;
								}
							}
							if (collideLineDist < minbeamCollideLineDist)
							{
								minbeamCollideLineDist = collideLineDist;
								minbeamTarget = bullet->target;
								minbeamCollSide = collSide;
							}
						}
					}
				}
			}
            break;
        }
    }

    collCheckBeamCollWithTargetsInBlob(bullet,thisBlob,&minbeamCollideLineDist,&minbeamTarget,&minbeamCollSide, &minbeamPolyHit);

    // do line-blob collision test for all other blobs in universe:
    while (blobnode != NULL)
    {
        checkBlob = (blob *)listGetStructOfNode(blobnode);
        blobnode = blobnode->next;

        if (checkBlob == thisBlob)
        {
            continue;
        }

        vecSub(distvector,checkBlob->centre,bullet->posinfo.position);
        distsquared = vecMagnitudeSquared(distvector);
        v = vecDotProduct(distvector,bullet->bulletheading);
		if (v > 0)
		{
			dsqr = (checkBlob->radius*checkBlob->radius) - (distsquared - (v*v));
			if (dsqr > 0)
			{
				d = fsqrt(dsqr);
				if ((v-d) < bullet->traveldist)
				{
					collCheckBeamCollWithTargetsInBlob(bullet,checkBlob,&minbeamCollideLineDist,&minbeamTarget,&minbeamCollSide, &minbeamPolyHit);
				}
			}
		}
    }

    if (minbeamTarget != NULL)        // beam only collides with closest target
    {
        dbgAssert(minbeamCollideLineDist >= 0.0f);
        univBulletCollidedWithTarget(minbeamTarget,&minbeamTarget->staticinfo->staticheader,bullet,minbeamCollideLineDist,minbeamCollSide);
        bullet->beamtraveldist = minbeamCollideLineDist;
    }
    else
    {
        bullet->beamtraveldist = bullet->lengthmag;
    }
    //adjust length of effect associated with bullet, if applicable
    if (bullet->effect != NULL)
    {
        ((real32 *)bullet->effect->variable)[ETG_LengthVariable] = bullet->beamtraveldist;
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckBulletTargetColl
    Description : checks collisions between bullets and targets.  When a bullet
                  collides with a target, it should disappear and do damage to
                  the enemy ship.
    Inputs      :
    Outputs     :
    Return      :
    Notes       : This routine uses line-sphere intersection for collision detection
----------------------------------------------------------------------------*/
void collCheckBulletTargetColl(blob *thisBlob)
{
    sdword bulletindex = 0;
    Bullet *bullet;

    sdword targetindex;
    SpaceObjRotImpTarg *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,d,dsqr;
    sdword collSide;

    real32 distcheck;

    bool32 deletebulletflag;

    real32 collideLineDist;

    real32 testcollsizesqr;

    BulletSelection *bulletselection = thisBlob->blobBullets;

    SelectAnyCommand *targetselection;
    sdword numTargets;

    sdword bottom,top;

    real32 maxTargetCollSphereSizePlusBulletTravelDist;

    sdword pass;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (bulletindex < bulletselection->numBullets)
    {
        bullet = bulletselection->BulletPtr[bulletindex];
        dbgAssert(bullet->objtype == OBJ_BulletType);

        deletebulletflag = FALSE;

        switch ((WeaponType)bullet->bulletType)
        {
            case WEAPON_Ion:
                collCheckBeamColl(thisBlob,bullet);
                break;
            case WEAPON_BeastInfection:
            case WEAPON_Laser:
                break;              //don't do collision detection for lasers
            default:
                pass = 0;


                if(bullet->target != NULL)
                {
                    switch(bullet->target->objtype)
                    {
                    case OBJ_MissileType:
                        //do bullet/missle target collision with its target only!
                         distcheck = bullet->target->collOptimizeDist - bullet->collOptimizeDist;

                        if (abs(distcheck) > (bullet->target->staticinfo->staticheader.staticCollInfo.collspheresize+bullet->traveldist))
                        {
                            //trivially reject...
                            goto nominecollision;
                        }
                        vecSub(distvector,bullet->target->collInfo.collPosition,bullet->posinfo.position);
                        distsquared = vecMagnitudeSquared(distvector);
                        v = vecDotProduct(distvector,bullet->bulletheading);
						//if (v > 0)
						{

							testcollsizesqr = bullet->target->staticinfo->staticheader.staticCollInfo.collspheresizeSqr;

							dsqr = testcollsizesqr - (distsquared - (v*v));
							if (dsqr > 0)
							{
								d = fsqrt(dsqr);
								if ((v-d) < bullet->traveldist)
								{
									if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)bullet->target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
									{
										univBulletCollidedWithTarget(bullet->target,&bullet->target->staticinfo->staticheader,bullet,collideLineDist,collSide);
										deletebulletflag = TRUE;
										goto nextbullet;
									}
								}
							}
						}

                        break;
                    }
                }
nominecollision:
passagain:
                if (pass == 0)
                {
                    targetselection = thisBlob->blobSmallTargets;
                    numTargets = targetselection->numTargets;

                    if (numTargets == 0)
                    {
                        goto nextpass;         // go directly to next pass
                    }

                    maxTargetCollSphereSizePlusBulletTravelDist = thisBlob->blobMaxSTargetCollSphereSize + bullet->traveldist;

                    // binary search targetindex such that
                    // (bullet->collOptimizeDist - target->collOptimizeDist) <= maxTargetCollSphereSize
                    bottom = 0;
                    top = numTargets-1;
                    while (top >= bottom)
                    {
                        targetindex = (bottom+top)>>1;
                        if ((bullet->collOptimizeDist - targetselection->TargetPtr[targetindex]->collOptimizeDist) > maxTargetCollSphereSizePlusBulletTravelDist)
                        {
                            // targetindex is too small
                            bottom = targetindex+1;
                        }
                        else
                        {
                            top = targetindex-1;
                        }
                    }

                    dbgAssert(targetindex >= 0);
                    dbgAssert(targetindex < numTargets);
                }
                else
                {
                    dbgAssert(pass == 1);
                    targetselection = thisBlob->blobBigTargets;
                    numTargets = targetselection->numTargets;
                    targetindex = 0;
                    maxTargetCollSphereSizePlusBulletTravelDist = thisBlob->blobMaxBTargetCollSphereSize + bullet->traveldist;
                }

                while (targetindex < numTargets)
                {
                    target = targetselection->TargetPtr[targetindex];
                    targetstaticheader = &target->staticinfo->staticheader;

#ifdef COLLISION_CHECK_STATS
            bulletwalks++;
#endif

                    distcheck = target->collOptimizeDist - bullet->collOptimizeDist;

                    if (distcheck > maxTargetCollSphereSizePlusBulletTravelDist)
                    {
                        goto nextpass;
                    }

                    if ((target->objtype != OBJ_ShipType) || (!ShipTypeIsDefenseFieldFrigateType(((Ship *)target)->shiptype) && (((Ship *)target)->shiptype != sSentinel) && (((Ship *)target)->shiptype != bSentinel)))
                    {
                        if (abs(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+bullet->traveldist))
                        {
                            goto nexttarget;
                        }
                    }

                    if (bullet->owner == (Ship *)target)
                    {
                        goto nexttarget;
                    }


                    if ((target->objtype == OBJ_DustType) || (target->objtype == OBJ_NebulaType))
                    {
                        goto nexttarget;
                    }

                    // Check for collisions between bullet and target here:

#ifdef COLLISION_CHECK_STATS
            bulletchecks++;
#endif

                    vecSub(distvector,target->collInfo.collPosition,bullet->posinfo.position);
                    distsquared = vecMagnitudeSquared(distvector);
                    v = vecDotProduct(distvector,bullet->bulletheading);

                    if (target->objtype == OBJ_ShipType)
                    {
                        if(ShipTypeIsDefenseFieldFrigateType(((Ship *)target)->shiptype))
                        {       //if target is a dfgf,
                            DFGFrigateStatics *DFGFstatics;

                            DFGFstatics = (DFGFrigateStatics *) ((ShipStaticInfo *)(((Ship *)target)->staticinfo))->custstatinfo;
                            if(distsquared < DFGFstatics->DFGFrigateFieldRadiusSqr)
                            {   //and bullet is within its field radius...affect the bullet
                                univDFGFieldEffect((Ship *)target,bullet,universe.totaltimeelapsed);
                            }
                        }
                        if((((Ship *)target)->shiptype == sSentinel) || (((Ship *)target)->shiptype == bSentinel))
                        {    //if target is a DefenseFighter
                            DefenseFighterStatics *defensestatics;
                            defensestatics = (DefenseFighterStatics *) ((ShipStaticInfo *)(((Ship *)target)->staticinfo))->custstatinfo;
                            if(distsquared < ((Ship *)target)->staticinfo->bulletRangeSquared[((Ship *)target)->tacticstype])
                            {
                                DefenseFighterReportBullet((Ship *)target, bullet);
                            }
                        }
                    }

					//if (v > 0)
					{

						testcollsizesqr = targetstaticheader->staticCollInfo.collspheresizeSqr;

						dsqr = testcollsizesqr - (distsquared - (v*v));
						if (dsqr > 0)
						{
							d = fsqrt(dsqr);
							if ((v-d) < bullet->traveldist)
							{
								if ((collideLineDist = collCheckRectLine((SpaceObjRotImp *)target,&bullet->posinfo.position,&bullet->bulletheading,bullet->traveldist,&collSide)) >= 0.0f)
								{
									univBulletCollidedWithTarget(target,targetstaticheader,bullet,collideLineDist,collSide);
									deletebulletflag = TRUE;
									goto nextbullet;
								}
							}
						}
					}

        nexttarget:
                    targetindex++;
                }

nextpass:
                pass++;
                if (pass < 2)
                {
                    goto passagain;
                }
                break;
        }

nextbullet:
        if (deletebulletflag)
        {
            // we are deleting this bullet's index, so don't update bulletindex

            if(bitTest(bullet->SpecialEffectFlag, 0x0002))
            {    //bullet is possibly being targetted by a defense fighter...remove that reference
                DefenseFighterBulletRemoval(bullet);
            }

            bobObjectDied((SpaceObj *)bullet,&universe.collBlobList);
            listRemoveNode(&bullet->bulletlink);
            univRemoveObjFromRenderList((SpaceObj *)bullet);
            listDeleteNode(&bullet->objlink);
#if 0
            univDeleteBullet(bullet);
#endif
        }
        else
        {
            bulletindex++;
        }
    }
#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : collCheckMissileShipColl
    Description : checks collisions between missiles and ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileShipColl(blob *thisBlob,MissileType missileType)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Ship *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection;
    sdword numMissiles;

    SelectCommand *targetselection;
    sdword numTargets;

    sdword bottom,top;

    real32 maxCollSphereSizePlusMissileTravelDist;

    sdword pass;

    if (missileType == MISSILE_Mine)
    {
        missileselection = thisBlob->blobMissileMines;
    }
    else
    {
        missileselection = thisBlob->blobMissileMissiles;
    }
    numMissiles = missileselection->numMissiles;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        pass = 0;

passagain:

        if (pass == 0)
        {
            targetselection = thisBlob->blobSmallShips;
            numTargets = targetselection->numShips;

            if (numTargets == 0)
            {
                goto nextpass;
            }

            maxCollSphereSizePlusMissileTravelDist = thisBlob->blobMaxSmallShipCollSphereSize+missile->maxtraveldist;

            // binary search targetindex such that
            // (missile->collOptimizeDist - target->collOptimizeDist) <= maxCollSphereSize
            bottom = 0;
            top = numTargets-1;
            while (top >= bottom)
            {
                targetindex = (bottom+top)>>1;
                if ((missile->collOptimizeDist - targetselection->ShipPtr[targetindex]->collOptimizeDist) > maxCollSphereSizePlusMissileTravelDist)
                {
                    // targetindex is too small
                    bottom = targetindex+1;
                }
                else
                {
                    top = targetindex-1;
                }
            }

            dbgAssert(targetindex >= 0);
            dbgAssert(targetindex < numTargets);
        }
        else
        {
            dbgAssert(pass == 1);
            targetselection = thisBlob->blobBigShips;
            numTargets = targetselection->numShips;
            maxCollSphereSizePlusMissileTravelDist = thisBlob->blobMaxBigShipCollSphereSize+missile->maxtraveldist;
            targetindex = 0;
        }

        while (targetindex < numTargets)
        {
            target = targetselection->ShipPtr[targetindex];

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxCollSphereSizePlusMissileTravelDist)
            {
                goto nextpass;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (abs(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            if (missile->owner == (Ship *)target)
            {
                goto nexttarget;
            }

            if ((target->shiptype == sSentinel || target->shiptype == bSentinel) &&
                univObjIsDying((SpaceObj *)target) &&
                missile->playerowner != target->playerowner)
            {
                real32 hitDist;

                if (collCheckRayShipCollPolygons(target,
                                                 &missile->posinfo.position,
                                                 &missile->posinfo.velocity,
                                                 &hitDist) != -1)
                {
                    univMissileCollidedWithTarget((SpaceObjRotImpTarg *)target, targetstaticheader, missile, hitDist, -1);
                    AddMissileToDeleteMissileList(missile, -1);
                    goto nextmissile;
                }
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
            dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v * v));
            if (dsqr > 0.0f && missile->maxtraveldist * missile->maxtraveldist > (v * v) - dsqr)
            {
                if ((collideLineDist = collCheckRayBBoxIntersection((SpaceObjRot *)target, &missile->posinfo.position, &missileheading, missile->maxtraveldist, &collSide)) >= 0.0f)
                {
                    collProcessMissileHit(TRUE, (SpaceObjRotImpTarg *)target, targetstaticheader, missile, collideLineDist, collSide);
                    AddMissileToDeleteMissileList(missile, -1);
                    goto nextmissile;
                }
            }

nexttarget:
            targetindex++;
        }

nextpass:
        pass++;
        if (pass < 2)
        {
            goto passagain;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}

/*-----------------------------------------------------------------------------
    Name        : collCheckMissileResourceColl
    Description : checks collisions between missiles and resources
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileResourceColl(blob *thisBlob,MissileType missileType)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Resource *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection;
    sdword numMissiles;

    ResourceSelection *targetselection = thisBlob->blobResources;
    sdword numTargets = targetselection->numResources;

    sdword bottom,top;

    real32 maxCollSphereSize = thisBlob->blobMaxResourceCollSphereSize;
    real32 maxCollSphereSizePlusMissileTravelDist;

    if (numTargets == 0)
    {
        return;
    }

    if (missileType == MISSILE_Mine)
    {
        missileselection = thisBlob->blobMissileMines;
    }
    else
    {
        missileselection = thisBlob->blobMissileMissiles;
    }
    numMissiles = missileselection->numMissiles;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        maxCollSphereSizePlusMissileTravelDist = maxCollSphereSize + missile->maxtraveldist;

        // binary search targetindex such that
        // (missile->collOptimizeDist - target->collOptimizeDist) <= maxCollSphereSizePlusMissileTravelDist
        bottom = 0;
        top = numTargets-1;
        while (top >= bottom)
        {
            targetindex = (bottom+top)>>1;
            if ((missile->collOptimizeDist - targetselection->ResourcePtr[targetindex]->collOptimizeDist) > maxCollSphereSizePlusMissileTravelDist)
            {
                // targetindex is too small
                bottom = targetindex+1;
            }
            else
            {
                top = targetindex-1;
            }
        }

        dbgAssert(targetindex >= 0);
        dbgAssert(targetindex < numTargets);

        while (targetindex < numTargets)
        {
            target = targetselection->ResourcePtr[targetindex];

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxCollSphereSizePlusMissileTravelDist)
            {
                goto nextmissile;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (abs(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            if (target->flags & SOF_NotBumpable)
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
            dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v * v));
            if (dsqr > 0.0f && missile->maxtraveldist * missile->maxtraveldist > (v * v) - dsqr)
            {
                if ((collideLineDist = collCheckRayBBoxIntersection((SpaceObjRot *)target, &missile->posinfo.position, &missileheading, missile->maxtraveldist, &collSide)) >= 0.0f)
                {
                    collProcessMissileHit(TRUE, (SpaceObjRotImpTarg *)target, targetstaticheader, missile, collideLineDist, collSide);
                    AddMissileToDeleteMissileList(missile, -1);
                    goto nextmissile;
                }
            }

nexttarget:
            targetindex++;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}


/*-----------------------------------------------------------------------------
    Name        : collCheckMissileDerelictColl
    Description : checks collisions between missiles and derelicts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileDerelictColl(blob *thisBlob,MissileType missileType)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Derelict *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection;
    sdword numMissiles;

    DerelictSelection *targetselection = thisBlob->blobDerelicts;
    sdword numTargets = targetselection->numDerelicts;

    real32 maxCollSphereSize = thisBlob->blobMaxDerelictCollSphereSize;
    real32 maxCollSphereSizePlusMissileTravelDist;

    sdword bottom,top;

    if (numTargets == 0)
    {
        return;
    }

    if (missileType == MISSILE_Mine)
    {
        missileselection = thisBlob->blobMissileMines;
    }
    else
    {
        missileselection = thisBlob->blobMissileMissiles;
    }
    numMissiles = missileselection->numMissiles;

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        maxCollSphereSizePlusMissileTravelDist = maxCollSphereSize + missile->maxtraveldist;

        // binary search targetindex such that
        // (missile->collOptimizeDist - target->collOptimizeDist) <= maxCollSphereSizePlusMissileTravelDist
        bottom = 0;
        top = numTargets-1;
        while (top >= bottom)
        {
            targetindex = (bottom+top)>>1;
            if ((missile->collOptimizeDist - targetselection->DerelictPtr[targetindex]->collOptimizeDist) > maxCollSphereSizePlusMissileTravelDist)
            {
                // targetindex is too small
                bottom = targetindex+1;
            }
            else
            {
                top = targetindex-1;
            }
        }

        dbgAssert(targetindex >= 0);
        dbgAssert(targetindex < numTargets);

        while (targetindex < numTargets)
        {
            target = targetselection->DerelictPtr[targetindex];

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxCollSphereSizePlusMissileTravelDist)
            {
                goto nextmissile;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (abs(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
            dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v * v));
            if (dsqr > 0.0f && missile->maxtraveldist * missile->maxtraveldist > (v * v) - dsqr)
            {
                if ((collideLineDist = collCheckRayBBoxIntersection((SpaceObjRot *)target, &missile->posinfo.position, &missileheading, missile->maxtraveldist, &collSide)) >= 0.0f)
                {
                    collProcessMissileHit(TRUE, (SpaceObjRotImpTarg *)target, targetstaticheader, missile, collideLineDist, collSide);
                    AddMissileToDeleteMissileList(missile, -1);
                    goto nextmissile;
                }
            }

nexttarget:
            targetindex++;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}

/*-----------------------------------------------------------------------------
    Name        : collCheckMissileMineColl
    Description : checks collisions between missiles and mines
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckMissileMineColl(blob *thisBlob)
{
    sdword missileindex = 0;
    Missile *missile;

    sdword targetindex;
    Missile *target;
    StaticHeader *targetstaticheader;

    real32 distsquared;
    vector distvector;
    real32 v,dsqr;
    sdword collSide;

    real32 distcheck;

    real32 collideLineDist;

    vector missileheading;

    MissileSelection *missileselection = thisBlob->blobMissileMissiles;
    sdword numMissiles = missileselection->numMissiles;

    MissileSelection *targetselection = thisBlob->blobMissileMines;
    sdword numTargets = targetselection->numMissiles;

    sdword bottom,top;

    real32 maxMineCollSphereSizePlusMissileTravelDist;

    if (numTargets == 0)
    {
        return;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 1;
#endif

    while (missileindex < numMissiles)
    {
        missile = missileselection->MissilePtr[missileindex];
        dbgAssert(missile->objtype == OBJ_MissileType);
        dbgAssert(missile->missileType == MISSILE_Regular);

        if (missile->flags & SOF_Dead)
        {
            goto nextmissile;
        }

        maxMineCollSphereSizePlusMissileTravelDist = maxMineCollSphereSize + missile->maxtraveldist;

        // binary search targetindex such that
        // (missile->collOptimizeDist - target->collOptimizeDist) <= maxMineCollSphereSizePlusMissileTravelDist
        bottom = 0;
        top = numTargets-1;
        while (top >= bottom)
        {
            targetindex = (bottom+top)>>1;
            if ((missile->collOptimizeDist - targetselection->MissilePtr[targetindex]->collOptimizeDist) > maxMineCollSphereSizePlusMissileTravelDist)
            {
                // targetindex is too small
                bottom = targetindex+1;
            }
            else
            {
                top = targetindex-1;
            }
        }

        dbgAssert(targetindex >= 0);
        dbgAssert(targetindex < numTargets);

        while (targetindex < numTargets)
        {
            target = targetselection->MissilePtr[targetindex];
            dbgAssert(target->objtype == OBJ_MissileType);
            dbgAssert(target->missileType == MISSILE_Mine);

#ifdef COLLISION_CHECK_STATS
    missilewalks++;
#endif

            distcheck = target->collOptimizeDist - missile->collOptimizeDist;

            if (distcheck > maxMineCollSphereSizePlusMissileTravelDist)
            {
                goto nextmissile;
            }

            targetstaticheader = &target->staticinfo->staticheader;

            if (abs(distcheck) > (targetstaticheader->staticCollInfo.collspheresize+missile->maxtraveldist))
            {
                goto nexttarget;
            }

            // Check for collisions between missile and target here:

#ifdef COLLISION_CHECK_STATS
    missilechecks++;
#endif
            vecSub(distvector,target->collInfo.collPosition,missile->posinfo.position);
            distsquared = vecMagnitudeSquared(distvector);
            matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
            v = vecDotProduct(distvector,missileheading);
            dsqr = targetstaticheader->staticCollInfo.collspheresizeSqr - (distsquared - (v * v));
            if (dsqr > 0.0f && missile->maxtraveldist * missile->maxtraveldist > (v * v) - dsqr)
            {
                if ((collideLineDist = collCheckRayBBoxIntersection((SpaceObjRot *)target, &missile->posinfo.position, &missileheading, missile->maxtraveldist, &collSide)) >= 0.0f)
                {
                    collProcessMissileHit(TRUE, (SpaceObjRotImpTarg *)target, targetstaticheader, missile, collideLineDist, collSide);
                    AddMissileToDeleteMissileList(missile, -1);
                    goto nextmissile;
                }
            }

nexttarget:
            targetindex++;
        }

nextmissile:
        missileindex++;
    }

#ifndef HW_Release
//    thisBlob->debugFlag = 0;
#endif

}

/*-----------------------------------------------------------------------------
    Name        : collCheckAllBumpCollisions
    Description : checks all bump collisions in universe (ship-ship, ship-resource, etc)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckAllBumpCollisions(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;

#ifdef COLLISION_CHECK_STATS
    shipshipwalks = 0;
    shipshipchecks = 0;
    shipresourcewalks = 0;
    shipresourcechecks = 0;
    shipderelictwalks = 0;
    shipderelictchecks = 0;
#endif

    if (!nisIsRunning)
    {
        // for each blob, check all collisions within that blob
        while (blobnode != NULL)
        {
            thisBlob = (blob *)listGetStructOfNode(blobnode);
            blobnode = blobnode->next;

            collCheckShipShipColl(thisBlob,TRUE);       // check smallship-smallship collisions
            collCheckShipShipColl(thisBlob,FALSE);      // check bigship-bigship collisions
            collCheckBigShipSmallShipColl(thisBlob);    // check bigship-smallship collisions
            collCheckShipResourceColl(thisBlob);
            collCheckShipDerelictColl(thisBlob);
        }
    }
    else if ((singlePlayerGame) && (singlePlayerGameInfo.currentMission == 4))
    {
        // for each blob, check all collisions within that blob
        while (blobnode != NULL)
        {
            thisBlob = (blob *)listGetStructOfNode(blobnode);
            blobnode = blobnode->next;

            // in case Bentusi traders run into some ships...
            collCheckShipShipColl(thisBlob,FALSE);      // check bigship-bigship collisions
            collCheckBigShipSmallShipColl(thisBlob);    // check bigship-smallship collisions
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : collCheckAllBulletMissileCollisions
    Description : checks all bullet, missile, and mine collisions in universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void collCheckAllBulletMissileCollisions(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;
    udword savemissilewalks;
    udword savemissilechecks;

#ifdef COLLISION_CHECK_STATS
    bulletwalks = 0;
    bulletchecks = 0;
    missilewalks = 0;
    missilechecks = 0;
    minewalks = 0;
    minechecks = 0;
#endif

    // for each blob, check all collisions within that blob
    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);
        blobnode = blobnode->next;

        collCheckBulletTargetColl(thisBlob);
        //collCheckBulletShipColl(thisBlob);
        //collCheckBulletResourceColl(thisBlob);
        //collCheckBulletDerelictColl(thisBlob);
        //collCheckBulletMissileColl(thisBlob);

        //collCheckMissileTargetColl(thisBlob);

        collCheckMissileShipColl(thisBlob,MISSILE_Regular);
        collCheckMissileResourceColl(thisBlob,MISSILE_Regular);
        collCheckMissileDerelictColl(thisBlob,MISSILE_Regular);

#ifdef COLLISION_CHECK_STATS
        savemissilewalks = missilewalks;
        savemissilechecks = missilechecks;
        missilewalks = 0;
        missilechecks = 0;
#endif

        collCheckMissileShipColl(thisBlob,MISSILE_Mine);
        //collCheckMissileResourceColl(thisBlob,MISSILE_Mine);
        collCheckMissileDerelictColl(thisBlob,MISSILE_Mine);

#ifdef COLLISION_CHECK_STATS
        minewalks += missilewalks;
        minechecks += missilechecks;
        missilewalks = savemissilewalks;
        missilechecks = savemissilechecks;
#endif

        collCheckMissileMineColl(thisBlob);
    }
}




sdword collCheckRayTriangleIntersection(vector *rayOrigin, vector *rayEnd, vector *v0, vector *v1, vector *v2, vector *normal, real32 *hitDist)
{
    vector computedNormal;
    vector *planeNormal = normal;
    real32 dotRayPlane;
    real32 t;
    vector intersectPoint;
    vector edge;
    vector edgeCross;
    vector *edgeStart;
    sdword edgeIndex;

    if (planeNormal == NULL)
    {
        vecComputeTriangleNormal(v2, v1, v0, &computedNormal);
        planeNormal = &computedNormal;
    }

    dotRayPlane = vecDotProduct(*rayOrigin, *planeNormal) - vecDotProduct(*rayEnd, *planeNormal);
    if (dotRayPlane > -collRayPlaneParallelEpsilon && dotRayPlane < collRayPlaneParallelEpsilon)
    {
        return FALSE;
    }

    t = (vecDotProduct(*rayOrigin, *planeNormal) - vecDotProduct(*v0, *planeNormal)) / dotRayPlane;
    *hitDist = t;
    if (t < 0.0f)
    {
        return FALSE;
    }

    intersectPoint.x = ((rayEnd->x - rayOrigin->x) * t) + rayOrigin->x;
    intersectPoint.y = ((rayEnd->y - rayOrigin->y) * t) + rayOrigin->y;
    intersectPoint.z = ((rayEnd->z - rayOrigin->z) * t) + rayOrigin->z;

    for (edgeIndex = 0; edgeIndex < 3; edgeIndex++)
    {
        if (edgeIndex == 0)
        {
            edgeStart = v0;
            vecSub(edge, *v1, *v0);
        }
        else if (edgeIndex == 1)
        {
            edgeStart = v1;
            vecSub(edge, *v2, *v1);
        }
        else
        {
            edgeStart = v2;
            vecSub(edge, *v0, *v2);
        }

        vecCrossProduct(edgeCross, edge, *planeNormal);
        vecNormalize(&edgeCross);
        if (vecDotProduct(edgeCross, *edgeStart) - vecDotProduct(edgeCross, intersectPoint) < 0.0f)
        {
            return FALSE;
        }
    }

    return TRUE;
}

sdword collCheckRayShipCollPolygons(Ship *ship, vector *rayOrigin, vector *rayDir, real32 *hitDist)
{
    ShipCollisionPolyOverlay *polyOverlay = shipCollisionPolyOverlay(ship);
    sdword bestPolyIdx = 0;
    sdword curIdx = -1;
    real32 bestDist = 1.0e21f;
    vector rayEnd;
    sdword polyCount;
    udword *polyEntry;

    rayEnd.x = rayOrigin->x + rayDir->x;
    rayEnd.y = rayOrigin->y + rayDir->y;
    rayEnd.z = rayOrigin->z + rayDir->z;
    polyCount = polyOverlay->polyCount;
    if (polyCount <= 0)
    {
        return -1;
    }

    polyEntry = polyOverlay->polyEntries;
    while (bestPolyIdx < polyCount)
    {
        real32 testDist;

        if (collCheckRayTriangleIntersection(rayOrigin,
                                             &rayEnd,
                                             &ship->posinfo.position,
                                             &((SpaceObjRotImpTarg *)polyEntry[0])->posinfo.position,
                                             &((SpaceObjRotImpTarg *)polyEntry[1])->posinfo.position,
                                             NULL,
                                             &testDist) &&
            testDist < bestDist)
        {
            curIdx = bestPolyIdx;
            bestDist = testDist;
        }
        bestPolyIdx++;
        polyEntry += 2;
    }

    if (curIdx != -1)
    {
        *hitDist = bestDist;
    }
    return curIdx;
}

real32 collCheckRayBBoxIntersection(SpaceObjRot *obj, vector *rayOrigin, vector *rayDir, real32 maxDist, sdword *hitFace)
{
    StaticCollInfo *staticCollInfo = &obj->staticinfo->staticheader.staticCollInfo;
    real32 minBounds[3];
    real32 maxBounds[3];
    real32 intersectionParam[3];
    real32 clampCoord[3];
    vector localOrigin;
    vector localDir;
    sdword clampSide[3];
    sdword inside = TRUE;
    sdword i;
    sdword bestAxis = 0;
    real32 hitT;

    *hitFace = -1;
    if (staticCollInfo->uplength == 0.0f)
    {
        return -1.0f;
    }

    minBounds[0] = staticCollInfo->collrectoffset.x;
    minBounds[1] = staticCollInfo->collrectoffset.y;
    minBounds[2] = staticCollInfo->collrectoffset.z;
    maxBounds[0] = staticCollInfo->collrectoffset.x + staticCollInfo->uplength;
    maxBounds[1] = staticCollInfo->collrectoffset.y + staticCollInfo->rightlength;
    maxBounds[2] = staticCollInfo->collrectoffset.z + staticCollInfo->forwardlength;

    vecSub(localOrigin, *rayOrigin, obj->posinfo.position);
    matMultiplyVecByMat(&localOrigin, &localOrigin, &obj->rotinfo.coordsys);

    for (i = 0; i < 3; i++)
    {
        if (((real32 *)&localOrigin)[i] < minBounds[i])
        {
            clampSide[i] = -1;
            clampCoord[i] = minBounds[i];
            inside = FALSE;
        }
        else if (((real32 *)&localOrigin)[i] > maxBounds[i])
        {
            clampSide[i] = 1;
            clampCoord[i] = maxBounds[i];
            inside = FALSE;
        }
        else
        {
            clampSide[i] = 0;
        }
    }

    if (inside)
    {
        return 0.0f;
    }

    matMultiplyVecByMat(&localDir, rayDir, &obj->rotinfo.coordsys);
    for (i = 0; i < 3; i++)
    {
        if (clampSide[i] == 0 || ((real32 *)&localDir)[i] == 0.0f)
        {
            intersectionParam[i] = -1.0f;
        }
        else
        {
            intersectionParam[i] = (clampCoord[i] - ((real32 *)&localOrigin)[i]) / ((real32 *)&localDir)[i];
        }
    }

    for (i = 1; i < 3; i++)
    {
        if (intersectionParam[i] > intersectionParam[bestAxis])
        {
            bestAxis = i;
        }
    }

    hitT = intersectionParam[bestAxis];
    if (hitT < 0.0f || hitT > maxDist)
    {
        return -1.0f;
    }

    for (i = 0; i < 3; i++)
    {
        real32 hitCoord;

        if (bestAxis == i)
        {
            continue;
        }

        hitCoord = hitT * ((real32 *)&localDir)[i] + ((real32 *)&localOrigin)[i];
        if (hitCoord <= minBounds[i] || hitCoord >= maxBounds[i])
        {
            return -1.0f;
        }
    }

    *hitFace = (clampSide[bestAxis] == -1) + 2 * bestAxis;
    return hitT;
}

void collApplyDamageToShip(Ship *target, sdword unused, Bullet *bullet)
{
    ShipBeamDamageState *dmg = shipBeamDamageState(target);
    real32 scaledDamage;
    sdword roundedDamage;

    (void)unused;

    dmg->unk_968 = universe.totaltimeelapsed;
    scaledDamage = univGetDamageMultiplier(bullet->soundType, target->upgradeState.level) * bullet->damage;
    roundedDamage = (sdword)(scaledDamage + 0.5f);
    dmg->unk_974 -= (real32)roundedDamage;
    if (dmg->unk_974 < 0.0f)
    {
        dmg->unk_978 = TRUE;
    }
}

// Cataclysm-specific: applies missile impact force, damage, and hit effects to one target.
Effect *collProcessMissileHit(sdword hitFlag, SpaceObjRotImpTarg *target, StaticHeader *targetStaticHeader, Missile *missile, real32 hitFraction, sdword armorSegment)
{
    Ship *owner = missile->owner;
    MissileStaticInfo *missileStaticInfo = (MissileStaticInfo *)missile->staticinfo;
    real32 damageTaken = missile->damage;
    etgeffectstatic *stat;
    vector hitLocation;
    vector missileHeading;
    real32 targetScale;

    if (hitFlag)
    {
        if (target->objtype == OBJ_ShipType &&
            ((Ship *)target)->playerowner->playerIndex == universe.curPlayerIndex &&
            *collMissileUnknown230(missile) != 0)
        {
            sdword groupIndex = (((Ship *)target)->hotKeyGroup - 1) & 0xF;

            if (groupIndex == 15)
            {
                battleChatterFleetAttempt(-1, 52, -1, NULL);
            }
            else
            {
                battleChatterFleetAttempt(-1, 53, groupIndex, NULL);
            }
        }

        if (!targetStaticHeader->immobile && !(target->flags & SOF_NotBumpable))
        {
            real32 massOverTime = missileStaticInfo->staticheader.mass / universe.unknown1;

            target->rotinfo.torque.x +=
                ((real32)ranRandom(10) * ((collMissileRockMax - collMissileRockMin) * collRandomScale) + collMissileRockMin) *
                missileStaticInfo->staticheader.mass;
            target->rotinfo.torque.y +=
                ((real32)ranRandom(10) * ((collMissileRockMax - collMissileRockMin) * collRandomScale) + collMissileRockMin) *
                missileStaticInfo->staticheader.mass;
            target->rotinfo.torque.z +=
                ((real32)ranRandom(10) * ((collMissileRockMax - collMissileRockMin) * collRandomScale) + collMissileRockMin) *
                missileStaticInfo->staticheader.mass;
            target->posinfo.force.x += missile->posinfo.velocity.x * massOverTime;
            target->posinfo.force.y += missile->posinfo.velocity.y * massOverTime;
            target->posinfo.force.z += missile->posinfo.velocity.z * massOverTime;
        }

        if (target->objtype == OBJ_ShipType)
        {
            Ship *targetShip = (Ship *)target;

            if (armorSegment >= 0)
            {
                damageTaken *= targetShip->staticinfo->collSideModifiers[armorSegment];
            }

            if (owner == NULL)
            {
                speechEventUnderAttack(targetShip);
            }
            else
            {
                if (owner->playerowner == targetShip->playerowner)
                {
                    damageTaken *= collMissileFriendlyDamageScale;
                    bitSet(targetShip->chatterBits, BCB_FriendlyFire);
                }
                else if (univIsObjFriendly((SpaceObj *)targetShip, owner->playerowner))
                {
                    bitSet(targetShip->chatterBits, BCB_AlliedFire);
                }
                else
                {
                    speechEventUnderAttack(targetShip);
                }

                targetShip->gettingrocked = owner;
                targetShip->recentAttacker = owner;
                targetShip->recentlyAttacked = RECENT_ATTACK_DURATION;
            }

            if (owner != NULL && owner->playerowner != NULL)
            {
                ApplyDamageToTarget(target, damageTaken, missile->soundType, DEATH_Killed_By_Player, owner->playerowner->playerIndex);
            }
            else
            {
                ApplyDamageToTarget(target, damageTaken, missile->soundType, DEATH_Killed_By_Dead_Player, 99);
            }
        }
    }

    stat = NULL;
    if (missile->hitEffect != NULL && target->currentLOD < missile->hitEffect->nLevels)
    {
        stat = missile->hitEffect->level[target->currentLOD];
    }

    if (!univSpaceObjInRenderList((SpaceObj *)target) || stat == NULL || !etgHitEffectsEnabled || etgFrequencyExceeded(stat))
    {
        return NULL;
    }

    if (hitFraction == 0.0f)
    {
        hitLocation = missile->posinfo.position;
    }
    else
    {
        matGetVectFromMatrixCol3(missileHeading, missile->rotinfo.coordsys);
        vecScalarMultiply(hitLocation, missileHeading, hitFraction);
        vecAdd(hitLocation, missile->posinfo.position, hitLocation);
    }

    if (target->objtype == OBJ_ShipType)
    {
        targetScale = ((Ship *)target)->magnitudeSquared;
        vecSubFrom(hitLocation, target->posinfo.position);
        vecMultiplyByScalar(hitLocation, targetScale);
        vecAddTo(hitLocation, target->posinfo.position);
    }
    else
    {
        targetScale = 1.0f;
    }

    return etgEffectCreate(stat,
                           NULL,
                           &hitLocation,
                           &target->posinfo.velocity,
                           &missile->rotinfo.coordsys,
                           targetScale,
                           0,
                           2);
}

sdword collCheckBBoxPoint(SpaceObjRotImp *obj, vector *point)
{
    vector minCorner;
    vector maxCorner;
    vector deltaMin;
    vector deltaMax;

    if (obj->staticinfo->staticheader.staticCollInfo.uplength == 0.0f)
    {
        return TRUE;
    }

    vecAdd(minCorner, obj->collInfo.rectpos[0], obj->posinfo.position);
    vecAdd(maxCorner, obj->collInfo.rectpos[6], obj->posinfo.position);
    vecSub(deltaMin, *point, minCorner);
    vecSub(deltaMax, *point, maxCorner);

    if (deltaMax.x * obj->rotinfo.coordsys.m31 + deltaMax.y * obj->rotinfo.coordsys.m32 + deltaMax.z * obj->rotinfo.coordsys.m33 > 0.0f)
    {
        return FALSE;
    }
    if (deltaMin.x * obj->rotinfo.coordsys.m31 + deltaMin.y * obj->rotinfo.coordsys.m32 + deltaMin.z * obj->rotinfo.coordsys.m33 < 0.0f)
    {
        return FALSE;
    }
    if (deltaMax.x * obj->rotinfo.coordsys.m21 + deltaMax.y * obj->rotinfo.coordsys.m22 + deltaMax.z * obj->rotinfo.coordsys.m23 > 0.0f)
    {
        return FALSE;
    }
    if (deltaMin.x * obj->rotinfo.coordsys.m21 + deltaMin.y * obj->rotinfo.coordsys.m22 + deltaMin.z * obj->rotinfo.coordsys.m23 < 0.0f)
    {
        return FALSE;
    }
    if (deltaMax.x * obj->rotinfo.coordsys.m11 + deltaMax.y * obj->rotinfo.coordsys.m12 + deltaMax.z * obj->rotinfo.coordsys.m13 > 0.0f)
    {
        return FALSE;
    }

    return deltaMin.x * obj->rotinfo.coordsys.m11 + deltaMin.y * obj->rotinfo.coordsys.m12 + deltaMin.z * obj->rotinfo.coordsys.m13 >= 0.0f;
}

sdword collCheckBBoxInBBox(SpaceObjRotImp *obj1, SpaceObjRotImp *obj2)
{
    vector minCorner;
    vector maxCorner;
    sdword i;

    vecAdd(minCorner, obj1->collInfo.rectpos[0], obj1->posinfo.position);
    vecAdd(maxCorner, obj1->collInfo.rectpos[6], obj1->posinfo.position);

    for (i = 0; i < 8; i++)
    {
        vector corner;
        vector deltaMin;
        vector deltaMax;

        vecAdd(corner, obj2->collInfo.rectpos[i], obj2->posinfo.position);
        vecSub(deltaMin, corner, minCorner);
        vecSub(deltaMax, corner, maxCorner);

        if (deltaMax.x * obj1->rotinfo.coordsys.m13 + deltaMax.y * obj1->rotinfo.coordsys.m23 + deltaMax.z * obj1->rotinfo.coordsys.m33 <= collBBoxInsidePositiveEpsilon &&
            deltaMin.x * obj1->rotinfo.coordsys.m13 + deltaMin.y * obj1->rotinfo.coordsys.m23 + deltaMin.z * obj1->rotinfo.coordsys.m33 >= collBBoxInsideNegativeEpsilon &&
            deltaMax.x * obj1->rotinfo.coordsys.m12 + deltaMax.y * obj1->rotinfo.coordsys.m22 + deltaMax.z * obj1->rotinfo.coordsys.m32 <= collBBoxInsidePositiveEpsilon &&
            deltaMin.x * obj1->rotinfo.coordsys.m12 + deltaMin.y * obj1->rotinfo.coordsys.m22 + deltaMin.z * obj1->rotinfo.coordsys.m32 >= collBBoxInsideNegativeEpsilon &&
            deltaMax.x * obj1->rotinfo.coordsys.m11 + deltaMax.y * obj1->rotinfo.coordsys.m21 + deltaMax.z * obj1->rotinfo.coordsys.m31 <= collBBoxInsidePositiveEpsilon &&
            deltaMin.x * obj1->rotinfo.coordsys.m11 + deltaMin.y * obj1->rotinfo.coordsys.m21 + deltaMin.z * obj1->rotinfo.coordsys.m31 >= collBBoxInsideNegativeEpsilon)
        {
            return TRUE;
        }
    }

    return FALSE;
}

sdword collCheckBBoxBumpCollision(SpaceObjRotImp *obj1, SpaceObjRotImp *obj2)
{
    StaticCollInfo *staticCollInfo;
    vector rayOrigin;
    vector rayDir;
    real32 diagonalLength;
    sdword hitFace;

    if (obj1->staticinfo->staticheader.staticCollInfo.uplength == 0.0f)
    {
        return TRUE;
    }
    if (obj2->staticinfo->staticheader.staticCollInfo.uplength == 0.0f)
    {
        return TRUE;
    }
    if (collCheckBBoxInBBox(obj1, obj2))
    {
        return TRUE;
    }
    if (collCheckBBoxInBBox(obj2, obj1))
    {
        return TRUE;
    }

    staticCollInfo = &obj1->staticinfo->staticheader.staticCollInfo;
    vecAdd(rayOrigin, obj1->collInfo.rectpos[0], obj1->posinfo.position);
    vecSub(rayDir, obj1->collInfo.rectpos[6], obj1->collInfo.rectpos[0]);
    diagonalLength = staticCollInfo->diagonallength;
    rayDir.x /= diagonalLength;
    rayDir.y /= diagonalLength;
    rayDir.z /= diagonalLength;

    return collCheckRayBBoxIntersection((SpaceObjRot *)obj2, &rayOrigin, &rayDir, diagonalLength, &hitFace) >= 0.0f;
}

sdword collComputeKamikazeCollisionDamage(
    Ship *ship,
    SpaceObjRotImpTarg *target,
    real32 *selfDamage,
    real32 *targetDamage,
    sdword *killerType,
    sdword *killerIndex)
{
    Ship *targetShip;
    void *beastWeaponInfo;

    if (ship->objtype != OBJ_ShipType || !bitTest(collShipSpecialFlags(ship), SPECIAL_Kamikaze))
    {
        return FALSE;
    }

    if (!collRammingFrigateSameGroup(ship->attackvars.attacktarget, target))
    {
        if (target->objtype == OBJ_ShipType)
        {
            targetShip = (Ship *)target;
            if ((target->flags & SOF_AttachPosition) != 0 || ship->playerowner == targetShip->playerowner)
            {
                return FALSE;
            }
        }
    }

    if (target->objtype == OBJ_ShipType)
    {
        targetShip = (Ship *)target;
        if (ship->shiptype == bCruiseMissile &&
            (beastWeaponInfo = shipGetBeastWeaponInfo(targetShip)) != NULL &&
            (singlePlayerGame ||
             (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT) &&
              (singlePlayerGame || ship->playerowner != targetShip->playerowner))))
        {
            ++*collShipBeastInfectionCounter(targetShip);
            if (singlePlayerGame &&
                targetShip->shiptype == dPrisonShip &&
                *collShipBeastInfectionCounter(targetShip) < ((sdword *)beastWeaponInfo)[1])
            {
                *targetDamage = targetShip->health * collCruiseMissilePartialDamageScale;
            }
            else
            {
                shipQueueBeastConversion(targetShip, ship);
            }
        }
        else
        {
            *targetDamage = kamikazeDamage[ship->shiptype];
        }
    }
    else
    {
        *targetDamage = kamikazeDamage[ship->shiptype];
    }

    if ((ship->flags & SOF_AttachPosition) != 0)
    {
        *targetDamage *= collShipGetKamikazeDamageMultiplier(ship);
    }

    *selfDamage = ship->staticinfo->maxhealth * collKamikazeSelfDamageScale;
    *killerType = DEATH_Killed_By_Kamikaze;
    *killerIndex = ship->playerowner->playerIndex;
    ship->posinfo.velocity.x = 0.0f;
    ship->posinfo.velocity.y = 0.0f;
    ship->posinfo.velocity.z = 0.0f;
    return TRUE;
}

bool32 collApplyKamikazeCollisionDamage(SpaceObjRotImpTarg *obj1, SpaceObjRotImpTarg *obj2)
{
    real32 selfDamage = 0.0f;
    real32 targetDamage = 0.0f;
    sdword killerType = 777;
    sdword killerIndex = 99;
    sdword otherKillerType = 777;
    sdword otherKillerIndex = 99;
    sdword hit1;
    sdword hit2;

    hit1 = collComputeKamikazeCollisionDamage((Ship *)obj1, obj2, &selfDamage, &targetDamage, &killerType, &killerIndex);
    hit2 = collComputeKamikazeCollisionDamage((Ship *)obj2, obj1, &targetDamage, &selfDamage, &otherKillerType, &otherKillerIndex);
    if (!hit1 && !hit2)
    {
        return FALSE;
    }

    ApplyDamageToTarget(obj1, selfDamage, 0, otherKillerType, otherKillerIndex);
    ApplyDamageToTarget(obj2, targetDamage, 0, killerType, killerIndex);
    return TRUE;
}





void collIgnoreSentinelShipCollision(Ship *obj, Ship *otherObj, bool32 *ignoreCollision)
{
    Ship *sentinel = NULL;
    Ship *otherSentinel = NULL;
    Ship *dyingSentinel = NULL;
    Ship *otherDyingSentinel = NULL;
    CommandToDo *command;

    if (obj->shiptype == sSentinel || obj->shiptype == bSentinel)
    {
        sentinel = obj;
        if (univObjIsDying((SpaceObj *)obj))
        {
            dyingSentinel = obj;
        }
    }

    if (otherObj->shiptype == sSentinel || otherObj->shiptype == bSentinel)
    {
        otherSentinel = otherObj;
        if (univObjIsDying((SpaceObj *)otherObj))
        {
            otherDyingSentinel = otherObj;
        }
    }

    if (dyingSentinel != NULL && !(otherObj->flags & SOF_Impactable))
    {
        *ignoreCollision = TRUE;
        return;
    }
    if (otherDyingSentinel != NULL && !(obj->flags & SOF_Impactable))
    {
        *ignoreCollision = TRUE;
        return;
    }

    if (sentinel != NULL)
    {
        if (otherSentinel == NULL)
        {
            command = sentinel->command;
            if (command != NULL && (command->ordertype.attributes & COMMAND_MASK_PROTECTING) != 0)
            {
                if (command->specialtargets->numTargets > 0 && command->specialtargets->TargetPtr[0] == (TargetPtr)otherObj)
                {
                    *ignoreCollision = TRUE;
                }
            }
        }
        return;
    }

    if (otherSentinel != NULL)
    {
        command = otherSentinel->command;
        if (command != NULL && (command->ordertype.attributes & COMMAND_MASK_PROTECTING) != 0)
        {
            if (command->specialtargets->numTargets > 0 && command->specialtargets->TargetPtr[0] == (TargetPtr)obj)
            {
                *ignoreCollision = TRUE;
            }
        }
        return;
    }

    *ignoreCollision = FALSE;
}

hmatrix *collMeshMultiplyCurrentMatrix(hmatrix *localMatrix)
{
    hmatrix current = collMeshCurrentMatrix;

    collMeshCurrentMatrix.m11 = current.m11 * localMatrix->m11 + current.m12 * localMatrix->m21 + current.m13 * localMatrix->m31 + current.m14 * localMatrix->m41;
    collMeshCurrentMatrix.m21 = current.m21 * localMatrix->m11 + current.m22 * localMatrix->m21 + current.m23 * localMatrix->m31 + current.m24 * localMatrix->m41;
    collMeshCurrentMatrix.m31 = current.m31 * localMatrix->m11 + current.m32 * localMatrix->m21 + current.m33 * localMatrix->m31 + current.m34 * localMatrix->m41;
    collMeshCurrentMatrix.m41 = current.m41 * localMatrix->m11 + current.m42 * localMatrix->m21 + current.m43 * localMatrix->m31 + current.m44 * localMatrix->m41;

    collMeshCurrentMatrix.m12 = current.m11 * localMatrix->m12 + current.m12 * localMatrix->m22 + current.m13 * localMatrix->m32 + current.m14 * localMatrix->m42;
    collMeshCurrentMatrix.m22 = current.m21 * localMatrix->m12 + current.m22 * localMatrix->m22 + current.m23 * localMatrix->m32 + current.m24 * localMatrix->m42;
    collMeshCurrentMatrix.m32 = current.m31 * localMatrix->m12 + current.m32 * localMatrix->m22 + current.m33 * localMatrix->m32 + current.m34 * localMatrix->m42;
    collMeshCurrentMatrix.m42 = current.m41 * localMatrix->m12 + current.m42 * localMatrix->m22 + current.m43 * localMatrix->m32 + current.m44 * localMatrix->m42;

    collMeshCurrentMatrix.m13 = current.m11 * localMatrix->m13 + current.m12 * localMatrix->m23 + current.m13 * localMatrix->m33 + current.m14 * localMatrix->m43;
    collMeshCurrentMatrix.m23 = current.m21 * localMatrix->m13 + current.m22 * localMatrix->m23 + current.m23 * localMatrix->m33 + current.m24 * localMatrix->m43;
    collMeshCurrentMatrix.m33 = current.m31 * localMatrix->m13 + current.m32 * localMatrix->m23 + current.m33 * localMatrix->m33 + current.m34 * localMatrix->m43;
    collMeshCurrentMatrix.m43 = current.m41 * localMatrix->m13 + current.m42 * localMatrix->m23 + current.m43 * localMatrix->m33 + current.m44 * localMatrix->m43;

    collMeshCurrentMatrix.m14 = current.m11 * localMatrix->m14 + current.m12 * localMatrix->m24 + current.m13 * localMatrix->m34 + current.m14 * localMatrix->m44;
    collMeshCurrentMatrix.m24 = current.m21 * localMatrix->m14 + current.m22 * localMatrix->m24 + current.m23 * localMatrix->m34 + current.m24 * localMatrix->m44;
    collMeshCurrentMatrix.m34 = current.m31 * localMatrix->m14 + current.m32 * localMatrix->m24 + current.m33 * localMatrix->m34 + current.m34 * localMatrix->m44;
    collMeshCurrentMatrix.m44 = current.m41 * localMatrix->m14 + current.m42 * localMatrix->m24 + current.m43 * localMatrix->m34 + current.m44 * localMatrix->m44;

    return localMatrix;
}

sdword collCheckLineMeshObject(vector *rayOrigin, vector *rayEnd, polygonobject *object, vector *hitPoint, real32 *hitDist, sdword *hitFound)
{
    matrix rotation;
    vector translation;
    vector localRayOrigin;
    vector localRayEnd;
    vector localHitPoint;
    vector transformedOrigin;
    vector transformedEnd;
    sdword polygonIndex;

    rotation.m11 = collMeshCurrentMatrix.m11;
    rotation.m12 = collMeshCurrentMatrix.m12;
    rotation.m13 = collMeshCurrentMatrix.m13;
    rotation.m21 = collMeshCurrentMatrix.m21;
    rotation.m22 = collMeshCurrentMatrix.m22;
    rotation.m23 = collMeshCurrentMatrix.m23;
    rotation.m31 = collMeshCurrentMatrix.m31;
    rotation.m32 = collMeshCurrentMatrix.m32;
    rotation.m33 = collMeshCurrentMatrix.m33;
    translation.x = collMeshCurrentMatrix.m41;
    translation.y = collMeshCurrentMatrix.m42;
    translation.z = collMeshCurrentMatrix.m43;

    vecSub(transformedOrigin, *rayOrigin, translation);
    matMultiplyVecByMat(&localRayOrigin, &transformedOrigin, &rotation);
    vecSub(transformedEnd, *rayEnd, translation);
    matMultiplyVecByMat(&localRayEnd, &transformedEnd, &rotation);

    for (polygonIndex = 0; polygonIndex < object->nPolygons; polygonIndex++)
    {
        polyentry *polygon = &object->pPolygonList[polygonIndex];
        real32 triangleHitDist;

        if (collCheckRayTriangleIntersection(&localRayOrigin,
                                             &localRayEnd,
                                             (vector *)&object->pVertexList[polygon->iV0],
                                             (vector *)&object->pVertexList[polygon->iV1],
                                             (vector *)&object->pVertexList[polygon->iV2],
                                             (vector *)&object->pNormalList[polygon->iFaceNormal],
                                             &triangleHitDist) &&
            (triangleHitDist < *hitDist || !*hitFound))
        {
            vector rayDelta;

            vecSub(rayDelta, localRayEnd, localRayOrigin);
            localHitPoint.x = rayDelta.x * triangleHitDist + localRayOrigin.x;
            localHitPoint.y = rayDelta.y * triangleHitDist + localRayOrigin.y;
            localHitPoint.z = rayDelta.z * triangleHitDist + localRayOrigin.z;
            matMultiplyMatByVec(hitPoint, &rotation, &localHitPoint);
            vecAddTo(*hitPoint, translation);
            *hitDist = triangleHitDist;
            *hitFound = TRUE;
        }
    }

    return object->nPolygons;
}

sdword collCheckLineMeshHierarchy(vector *rayOrigin, vector *rayEnd, polygonobject *object, vector *hitPoint, real32 *hitDist, sdword *hitFound)
{
    polygonobject *child;
    sdword result;

    if (collMeshMatrixDepth < 40)
    {
        collMeshMatrixStack[collMeshMatrixDepth++] = collMeshCurrentMatrix;
    }

    collMeshMultiplyCurrentMatrix(&object->localMatrix);
    for (child = object->pDaughter; child != NULL; child = child->pSister)
    {
        collCheckLineMeshHierarchy(rayOrigin, rayEnd, child, hitPoint, hitDist, hitFound);
    }
    result = collCheckLineMeshObject(rayOrigin, rayEnd, object, hitPoint, hitDist, hitFound);

    if (collMeshMatrixDepth > 0)
    {
        collMeshCurrentMatrix = collMeshMatrixStack[--collMeshMatrixDepth];
    }

    return result;
}
