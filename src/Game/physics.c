/*=============================================================================
    Name    : Physics.c
    Purpose : Implements space physics of Homeworld

    Created 6/18/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <intrin.h>
#include "types.h"
#include "debug.h"
#include "vector.h"
#include "matrix.h"
#include "fastmath.h"
#include "spaceobj.h"
#include "collision.h"
#include "physics.h"
#include "soundevent.h"
#include "Ship.h"
#include "universe.h"
#include "univupdate.h"
#include "tweak.h"
#include "tactics.h"
#include "attributes.h"
#include "gun.h"
#include "camera.h"
#include "DefenseFighter.h"
#include "multiplayergame.h"
#include "battle.h"

#include "Timer.h"
#include "Sensors.h"
#include "Globals.h"
#include "singleplayer.h"

#define UPDATE_COLLRECT_RATE    3
#define UPDATE_COLLRECT_FRAME   3

// Sensor-grid updates currently live in sensors.c with the Cataclysm fog-grid helpers.
sdword pingUpdateShipSensorGrid(Ship *ship, vector *oldPos, vector *newPos);

static sdword dfgUpdateAllFrigatesAtPosition(vector *position)
{
    Node *node = universe.ShipList.head;
    sdword hit = 0;

    while (node != NULL)
    {
        Ship *ship = listGetStructOfNode(node);

        if (ship->shiptype == bDFGFrigate || ship->shiptype == tDFGFrigate)
        {
            real32 radiusSq = ((real32 *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo)[1];
            vector delta;

            vecSub(delta, *position, ship->posinfo.position);
            if (vecMagnitudeSquared(delta) < radiusSq)
            {
                hit = 1;
            }
        }
        node = node->next;
    }

    return hit;
}

#ifdef gshaw
//#define TEST_FUEL
#endif

// Retail physics checks SpaceObj+0x14 bit 21 before suppressing ship force/rot updates.
#define PHYS_FORCE_ROT_SUPPRESSED(obj) bitTest((obj)->flags, BIT21)

/*-----------------------------------------------------------------------------
    Name        : physApplyForceToObj
    Description : applies a force in direction forcedir of magnitude force to
                  the object.  The force is translated into the mission sphere
                  coordinate system.
    Inputs      : obj,force,forcedir
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physApplyForceToObj(SpaceObj *obj,real32 force,uword transdir)
{
    vector vectorforce;

    dbgAssert((obj->flags & SOF_Rotatable) != 0);

    if (obj->objtype == OBJ_ShipType)
    {
        Ship *ship = (Ship *)obj;

        if (ship->dontapplyforceever)
        {
            //we don't want to try and fly anywheres with this ship obviously!
            return;
        }

        if (singlePlayerGame && ship->shipSinglePlayerGameInfo != NULL)
        {
            ShipSinglePlayerGameInfo *ssinfo = ship->shipSinglePlayerGameInfo;

            if (ssinfo->shipHyperspaceState == SHIPHYPERSPACE_DECELERATE)
            {
                return;
            }

            if (ssinfo->hsState != HS_INACTIVE &&
                ssinfo->hsState != HS_FINISHED)
            {
                return;
            }
        }

        if (PHYS_FORCE_ROT_SUPPRESSED(obj))
        {
            return;     // ship is crazy, so return
        }

#if 0       // new behaviour for fuel, max velocity of ship is lowered instead
        if (((ShipStaticInfo *)((Ship *)obj)->staticinfo)->maxfuel != 0.0f)
        {
            // ship burns fuel
            if ( ((Ship *)obj)->fuel <= 0.0f)
            {
                return;     // out of fuel, so return
            }
        }
#endif
    }

    switch (transdir)
    {
        case TRANS_UP:
        case TRANS_DOWN:
            matGetVectFromMatrixCol1(vectorforce,((SpaceObjRot *)obj)->rotinfo.coordsys);
            break;

        case TRANS_RIGHT:
        case TRANS_LEFT:
            matGetVectFromMatrixCol2(vectorforce,((SpaceObjRot *)obj)->rotinfo.coordsys);
            break;

        case TRANS_FORWARD:
        case TRANS_BACKWARD:
            matGetVectFromMatrixCol3(vectorforce,((SpaceObjRot *)obj)->rotinfo.coordsys);
            break;
    }

    if (transdir & 1)
    {
        force = -force;
    }

    vecMultiplyByScalar(vectorforce,force);
    vecAddTo(obj->posinfo.force,vectorforce);

}

/*-----------------------------------------------------------------------------
    Name        : physApplyRotToObj
    Description : applies a torque to an object about a rotation axis
                  and direction indicated by rotdir.
    Inputs      : torque, rotdir
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physApplyRotToObj(SpaceObjRot *obj,real32 torque,uword rotdir)
{
    dbgAssert((obj->flags & SOF_Rotatable) != 0);

    if (obj->objtype == OBJ_ShipType)
    {
        if (PHYS_FORCE_ROT_SUPPRESSED(obj))
        {
            return;     // ship is crazy, so return
        }
#if 0 // new behaviour for fuel, max velocity of ship is lowered instead
        if (((ShipStaticInfo *)((Ship *)obj)->staticinfo)->maxfuel != 0.0f)
        {
            // ship burns fuel
            if ( ((Ship *)obj)->fuel <= 0.0f)
            {
                return;     // out of fuel, so return
            }
        }
#endif
    }

    if (rotdir & 1)
    {
        torque = -torque;
    }

    switch (rotdir)
    {
        case ROT_ABOUTXCCW:
        case ROT_ABOUTXCW:
            obj->rotinfo.torque.x += torque;
            break;

        case ROT_ABOUTYCCW:
        case ROT_ABOUTYCW:
            obj->rotinfo.torque.y += torque;
            break;

        case ROT_ABOUTZCCW:
        case ROT_ABOUTZCW:
            obj->rotinfo.torque.z += torque;
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : physUpdateBulletPosVel
    Description : updates a bullets position
    Inputs      : phystimeelapsed - time elapsed since last physics frame
    Outputs     :
    Return      : true if the bullet should now be deleted
----------------------------------------------------------------------------*/
bool32 physUpdateBulletPosVel(Bullet *bullet,real32 phystimeelapsed)
{
    vector d;
    Ship *ship;
    vector gunheadingInWorldCoordSys;
    vector positionInWorldCoordSys, offset1, offset0;
    Gun *gun;
    GunStatic *gunstatic;

    if (bullet->bulletType == BULLET_Beam)
    {
        ship = (Ship *)bullet->owner;
        if (ship != NULL)
        {
            gun = bullet->gunowner;
            if (gun != NULL)
            {
                gunstatic = gun->gunstatic;

                if (bullet->target)
                {
                    switch (gunstatic->guntype)
                    {
                        case GUN_Gimble:
                        case GUN_NewGimble:
                            gunOrientGimbleGun(ship,gun,bullet->target);
                            break;
                    }
                }

                matMultiplyMatByVec(&gunheadingInWorldCoordSys,&ship->rotinfo.coordsys,&gun->gunheading);
                bullet->bulletheading = gunheadingInWorldCoordSys;
                matCreateCoordSysFromHeading(&bullet->rotinfo.coordsys,&gunheadingInWorldCoordSys);
                vecScalarMultiply(bullet->lengthvec,gunheadingInWorldCoordSys,bullet->beamtraveldist);

                matMultiplyMatByVec(&positionInWorldCoordSys,&ship->rotinfo.coordsys,&gunstatic->position);
                if (gunstatic->offset.x != 0.0f)
                {
                    matMultiplyMatByVec(&offset0, &gun->curgunorientation, &gunstatic->offset);
                    matMultiplyMatByVec(&offset1, &ship->rotinfo.coordsys, &offset0);
                    vecAdd(positionInWorldCoordSys, positionInWorldCoordSys, offset1);
                }

                vecAdd(bullet->posinfo.position,positionInWorldCoordSys,ship->posinfo.position);
            }
        }
    }
    else if (bullet->bulletType != BULLET_SpecialBurst)
    {
        if ((WeaponType)bullet->bulletType == WEAPON_BeastInfection)
        {
            bigGunUpdateQueuedBullet(bullet);
        }
        else
        {
            TreatAsReal32(bullet->cataUnknown150[2]) = bullet->posinfo.position.x;
            bullet->cataUnknown150[3] = TreatAsUdword(bullet->posinfo.position.y);
            TreatAsReal32(bullet->cataUnknown150[4]) = bullet->posinfo.position.z;

            vecScalarMultiply(d,bullet->posinfo.velocity,phystimeelapsed);
            vecAddTo(bullet->posinfo.position,d);
        }
    }

    bullet->posinfo.isMoving = TRUE;

    bullet->timelived += phystimeelapsed;
    if (bullet->timelived > bullet->totallifetime)
    {
        return TRUE;
    }
    else
    {
        if (bullet->soundType == GS_BigGun)
        {
            real32 travelledDistance;

            travelledDistance = (real32)sqrt(vecMagnitudeSquared(d)) + TreatAsReal32(bullet->cataUnknown150[1]);
            bullet->cataUnknown150[1] = TreatAsUdword(travelledDistance);
            if (travelledDistance > TreatAsReal32(bullet->cataUnknown150[0]))
            {
                return TRUE;
            }
            return dfgUpdateAllFrigatesAtPosition(&bullet->posinfo.position);
        }
        return FALSE;
    }
}

#define robj ((SpaceObjRot *)obj)
#define irobj ((SpaceObjRotImp *)obj)
#define shipobj ((Ship *)obj)

/*-----------------------------------------------------------------------------
    Name        : physUpdateObjPosVel
    Description : !!!!! THIS IS NO LONGER USED!!!! USE THE CUSTOM FUNCTIONS
                  BELOW!!!
                  updates a space objects position and orientation based
                  on any external forces/torques, as well as its current
                  velocity/spin.
    Inputs      : phystimeelapsed - time elapsed since last physics frame
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physUpdateObjPosVel(SpaceObj *obj,real32 phystimeelapsed)
{
    real32 maxX = smUniverseSizeX - 300.0f;
    real32 maxY = smUniverseSizeY - 300.0f;
    real32 maxZ = smUniverseSizeZ - 300.0f;
    (void)phystimeelapsed;

    if (((maxX < obj->posinfo.position.x) && (obj->posinfo.velocity.x > 0.0f)) ||
        ((-maxX > obj->posinfo.position.x) && (obj->posinfo.velocity.x < 0.0f)))
    {
        obj->posinfo.velocity.x = 0.0f;
    }

    if (((maxY < obj->posinfo.position.y) && (obj->posinfo.velocity.y > 0.0f)) ||
        ((-maxY > obj->posinfo.position.y) && (obj->posinfo.velocity.y < 0.0f)))
    {
        obj->posinfo.velocity.y = 0.0f;
    }

    if (((maxZ < obj->posinfo.position.z) && (obj->posinfo.velocity.z > 0.0f)) ||
        ((-maxZ > obj->posinfo.position.z) && (obj->posinfo.velocity.z < 0.0f)))
    {
        obj->posinfo.velocity.z = 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : physUpdateObjPosVelShip
    Description : custom update function for SHIPS only
    Inputs      : phystimeelapsed - time elapsed since last physics frame
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physUpdateObjPosVelShip(Ship *obj,real32 phystimeelapsed)
{
    bool32 *cataVelocityCapActive;
    Timer *cataVelocityCapTimer;
    vector a;
    vector d;
    vector oldPos;
    bool32 coordsysChanged = FALSE;
    real32 speedmult;
    matrix rotxmatrix;
    matrix rotymatrix;
    matrix rotzmatrix;
    matrix tempmat;
    real32 angxrotate;
    real32 angyrotate;
    real32 angzrotate;
    real32 maxMagnitude;
    StaticHeader *staticheader;
    ShipStaticInfo *shipstaticinfo;

    staticheader = &obj->staticinfo->staticheader;
    cataVelocityCapActive = &obj->velocityFreezeActive;
    cataVelocityCapTimer = &obj->velocityFreezeTimer;

/*-----------------------------------------------------------------------------
    Update object position
-----------------------------------------------------------------------------*/

    // a = F/m
    vecScalarMultiply(a,obj->posinfo.force,staticheader->oneOverMass);

    // a = at
    vecMultiplyByScalar(a,phystimeelapsed);

    // v = v + at
    if (!*cataVelocityCapActive)
    {
        vecAddTo(obj->posinfo.velocity,a);
    }

    if (TreatAsUdword(obj->posinfo.velocity.x) | TreatAsUdword(obj->posinfo.velocity.y) | TreatAsUdword(obj->posinfo.velocity.z))
    {
        obj->posinfo.isMoving = TRUE;

        speedmult = (real32)(shipGetMaxSpeed(obj) * tacticsGetShipsMaxVelocity(obj));
        if (!*cataVelocityCapActive)
        {
            vecCapVectorSloppy(&obj->posinfo.velocity, speedmult);
        }
        else
        {
            maxMagnitude = speedmult;
            if (maxMagnitude < 1400.0f)
            {
                maxMagnitude = 1400.0f;
            }
            if (TimerTimeRemaining(cataVelocityCapTimer) < 0.5f &&
                1400.0f > speedmult)
            {
                maxMagnitude = (1.0f - TimerTimeRemaining(cataVelocityCapTimer)) * (1400.0f - speedmult) + speedmult;
            }
            vecCapVectorSloppy(&obj->posinfo.velocity, maxMagnitude);
        }

        if (!singlePlayerGame)
        {
            physUpdateObjPosVel((SpaceObj *)obj, phystimeelapsed);
        }

        // d = vt
        vecScalarMultiply(d,obj->posinfo.velocity,phystimeelapsed);

        oldPos = obj->posinfo.position;

        // x = x + d
        vecAddTo(obj->posinfo.position,d);

        if (physShipIsMovable(obj))
        {
            pingUpdateShipSensorGrid(obj, &oldPos, &obj->posinfo.position);
        }

    }
    else
    {
        obj->posinfo.isMoving = FALSE;
        vecZeroVector(d);
    }

        shipstaticinfo = (ShipStaticInfo *)((Ship *)obj)->staticinfo;
        if ((obj->flags & SOF_Dead) == 0)
        {
            if (shipobj->flags & (SOF_Crazy|SOF_Disabled))
            {
                if (obj->posinfo.isMoving)
                {
                    if (!(TreatAsUdword(a.x) | TreatAsUdword(a.y) | TreatAsUdword(a.z)))   // if object is moving, but has no acceleration
                    {
                        if ((isBetweenExclusive(obj->posinfo.velocity.x,CONSIDERED_STILL_LO,CONSIDERED_STILL_HI)) &&
                            (isBetweenExclusive(obj->posinfo.velocity.y,CONSIDERED_STILL_LO,CONSIDERED_STILL_HI)) &&
                            (isBetweenExclusive(obj->posinfo.velocity.z,CONSIDERED_STILL_LO,CONSIDERED_STILL_HI)) )
                        {
                            obj->posinfo.isMoving = FALSE;
                            vecZeroVector(d);
                            vecZeroVector(obj->posinfo.velocity);
                        }
                        else
                        {
                            obj->posinfo.velocity.x *= SHIP_CRAZY_FRICTION;
                            obj->posinfo.velocity.y *= SHIP_CRAZY_FRICTION;
                            obj->posinfo.velocity.z *= SHIP_CRAZY_FRICTION;
                        }
                    }
                }
            }
        }

    dbgAssert(bitTest(obj->flags,SOF_Rotatable));
    {

    /*-----------------------------------------------------------------------------
        Update object orientation
    -----------------------------------------------------------------------------*/

        if ((robj->rotinfo.torque.x != 0) || (robj->rotinfo.torque.y != 0) || (robj->rotinfo.torque.z != 0))
        {

            // alpha = Torque/momentOfInertia (about each axis)
            a.x = robj->rotinfo.torque.x * staticheader->oneOverMomentOfInertiaX;
            a.y = robj->rotinfo.torque.y * staticheader->oneOverMomentOfInertiaY;
            a.z = robj->rotinfo.torque.z * staticheader->oneOverMomentOfInertiaZ;

            // alpha = alpha*t
            vecMultiplyByScalar(a,phystimeelapsed);

            // w = w + alpha*t
            vecAddTo(robj->rotinfo.rotspeed,a);

            // cap rotation speed
            vecCapVectorSloppy(&robj->rotinfo.rotspeed,staticheader->maxrot);

        }

        // We now have w.x, w.y, and w.z which indicate the
        // rotational velocities (rad/s) around the x,y, and z axes of the ship.

    /*-----------------------------------------------------------------------------
        Update coordsys based on changes in rotation
    -----------------------------------------------------------------------------*/

        if (robj->rotinfo.rotspeed.x)
        {
            angxrotate = robj->rotinfo.rotspeed.x * phystimeelapsed;

            matMakeRotAboutX(&rotxmatrix,(real32)cos(angxrotate),(real32)sin(angxrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotxmatrix);
            coordsysChanged = TRUE;
            bitSet(obj->posinfo.isMoving,ISMOVING_ROTATING);
        }

        if (robj->rotinfo.rotspeed.y)
        {
            angyrotate = robj->rotinfo.rotspeed.y * phystimeelapsed;

            matMakeRotAboutY(&rotymatrix,(real32)cos(angyrotate),(real32)sin(angyrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotymatrix);
            coordsysChanged = TRUE;
            bitSet(obj->posinfo.isMoving,ISMOVING_ROTATING);
        }

        if (robj->rotinfo.rotspeed.z)
        {
            angzrotate = robj->rotinfo.rotspeed.z * phystimeelapsed;

            matMakeRotAboutZ(&rotzmatrix,(real32)cos(angzrotate),(real32)sin(angzrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotzmatrix);
            coordsysChanged = TRUE;
            bitSet(obj->posinfo.isMoving,ISMOVING_ROTATING);
        }

        if (bitTest(obj->flags,SOF_Impactable))
        {
            if (coordsysChanged)
            {
                matMultiplyMatByVec(&irobj->collInfo.collOffset,&(irobj->rotinfo.coordsys),&staticheader->staticCollInfo.collsphereoffset);
                if ((universe.univUpdateCounter & UPDATE_COLLRECT_RATE) == UPDATE_COLLRECT_FRAME)
                    collUpdateCollRectangle(irobj);
            }

            vecAdd(irobj->collInfo.collPosition,obj->posinfo.position,irobj->collInfo.collOffset);

            if (coordsysChanged)
            {
                if ((shipstaticinfo->engineNozzleOffset[0].x != 0) || (shipstaticinfo->engineNozzleOffset[0].y != 0) || (shipstaticinfo->engineNozzleOffset[0].z != 0))
                {
                    matMultiplyMatByVec(&obj->engineOffset,&(robj->rotinfo.coordsys),&shipstaticinfo->engineNozzleOffset[0]);
                }
            }
            vecAdd(shipobj->enginePosition,obj->posinfo.position,shipobj->engineOffset);
        }
        vecZeroVector(robj->rotinfo.torque);
    }
    vecZeroVector(obj->posinfo.force);
}

/*-----------------------------------------------------------------------------
    Name        : physUpdateObjPosVelDerelicts
    Description : updates a derelicts position and orientation based
                  on any external forces/torques, as well as its current
                  velocity/spin.
    Inputs      : phystimeelapsed - time elapsed since last physics frame
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physUpdateObjPosVelDerelicts(Derelict *obj,real32 phystimeelapsed)
{
    vector a;
    vector d;
    StaticHeader *staticheader = &obj->staticinfo->staticheader;
    bool32 coordsysChanged = FALSE;
    //real32 burnmult;
    matrix rotxmatrix;
    matrix rotymatrix;
    matrix rotzmatrix;
    matrix tempmat;
    real32 angxrotate;
    real32 angyrotate;
    real32 angzrotate;

/*-----------------------------------------------------------------------------
    Update object position
-----------------------------------------------------------------------------*/

    // a = F/m
    vecScalarMultiply(a,obj->posinfo.force,staticheader->oneOverMass);

    // a = at
    vecMultiplyByScalar(a,phystimeelapsed);

    // v = v + at
    vecAddTo(obj->posinfo.velocity,a);

    if (TreatAsUdword(obj->posinfo.velocity.x) | TreatAsUdword(obj->posinfo.velocity.y) | TreatAsUdword(obj->posinfo.velocity.z))
    {
        obj->posinfo.isMoving = TRUE;
        vecCapVectorSloppy(&obj->posinfo.velocity,staticheader->maxvelocity);
        // d = vt
        vecScalarMultiply(d,obj->posinfo.velocity,phystimeelapsed);
        // x = x + d
        vecAddTo(obj->posinfo.position,d);
    }
    else
    {
        obj->posinfo.isMoving = FALSE;
        vecZeroVector(d);
    }

    // space friction for derelicts
    if (obj->posinfo.isMoving)
    {
        if (!(TreatAsUdword(a.x) | TreatAsUdword(a.y) | TreatAsUdword(a.z)))   // if object is moving, but has no acceleration
        {
            if ((isBetweenExclusive(obj->posinfo.velocity.x,CONSIDERED_STILL_LO,CONSIDERED_STILL_HI)) &&
                (isBetweenExclusive(obj->posinfo.velocity.y,CONSIDERED_STILL_LO,CONSIDERED_STILL_HI)) &&
                (isBetweenExclusive(obj->posinfo.velocity.z,CONSIDERED_STILL_LO,CONSIDERED_STILL_HI)) )
            {
                obj->posinfo.isMoving = FALSE;
                vecZeroVector(d);
                vecZeroVector(obj->posinfo.velocity);
            }
            else
            {
                obj->posinfo.velocity.x *= DONT_DRIFT_FRICTION;
                obj->posinfo.velocity.y *= DONT_DRIFT_FRICTION;
                obj->posinfo.velocity.z *= DONT_DRIFT_FRICTION;
            }
        }
    }

    dbgAssert(bitTest(obj->flags,SOF_Rotatable));
    {

    /*-----------------------------------------------------------------------------
        Update object orientation
    -----------------------------------------------------------------------------*/

        if ((robj->rotinfo.torque.x != 0) || (robj->rotinfo.torque.y != 0) || (robj->rotinfo.torque.z != 0))
        {

            // alpha = Torque/momentOfInertia (about each axis)
            a.x = robj->rotinfo.torque.x * staticheader->oneOverMomentOfInertiaX;
            a.y = robj->rotinfo.torque.y * staticheader->oneOverMomentOfInertiaY;
            a.z = robj->rotinfo.torque.z * staticheader->oneOverMomentOfInertiaZ;

            // alpha = alpha*t
            vecMultiplyByScalar(a,phystimeelapsed);

            // w = w + alpha*t
            vecAddTo(robj->rotinfo.rotspeed,a);

            // cap rotation speed
            vecCapVectorSloppy(&robj->rotinfo.rotspeed,staticheader->maxrot);

        }

        // We now have w.x, w.y, and w.z which indicate the
        // rotational velocities (rad/s) around the x,y, and z axes of the ship.

    /*-----------------------------------------------------------------------------
        Update coordsys based on changes in rotation
    -----------------------------------------------------------------------------*/

        if (robj->rotinfo.rotspeed.x)
        {
            angxrotate = robj->rotinfo.rotspeed.x * phystimeelapsed;

            matMakeRotAboutX(&rotxmatrix,(real32)cos(angxrotate),(real32)sin(angxrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotxmatrix);
            coordsysChanged = TRUE;
            bitSet(obj->posinfo.isMoving,ISMOVING_ROTATING);
        }

        if (robj->rotinfo.rotspeed.y)
        {
            angyrotate = robj->rotinfo.rotspeed.y * phystimeelapsed;

            matMakeRotAboutY(&rotymatrix,(real32)cos(angyrotate),(real32)sin(angyrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotymatrix);
            coordsysChanged = TRUE;
            bitSet(obj->posinfo.isMoving,ISMOVING_ROTATING);
        }

        if (robj->rotinfo.rotspeed.z)
        {
            angzrotate = robj->rotinfo.rotspeed.z * phystimeelapsed;

            matMakeRotAboutZ(&rotzmatrix,(real32)cos(angzrotate),(real32)sin(angzrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotzmatrix);
            coordsysChanged = TRUE;
            bitSet(obj->posinfo.isMoving,ISMOVING_ROTATING);
        }

        if (bitTest(obj->flags,SOF_Impactable))
        {
            if (coordsysChanged)
            {
                matMultiplyMatByVec(&irobj->collInfo.collOffset,&(irobj->rotinfo.coordsys),&staticheader->staticCollInfo.collsphereoffset);
                if ((universe.univUpdateCounter & UPDATE_COLLRECT_RATE) == UPDATE_COLLRECT_FRAME)
                    collUpdateCollRectangle(irobj);
            }

            vecAdd(irobj->collInfo.collPosition,obj->posinfo.position,irobj->collInfo.collOffset);
        }
        vecZeroVector(robj->rotinfo.torque);
    }
    vecZeroVector(obj->posinfo.force);
}

/*-----------------------------------------------------------------------------
    Name        : physUpdateObjPosVelMissile
    Description : updates a missiles position and orientation based
                  on any external forces/torques, as well as its current
                  velocity/spin.
    Inputs      : phystimeelapsed - time elapsed since last physics frame
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physUpdateObjPosVelMissile(Missile *obj,real32 phystimeelapsed)
{
    vector a;
    vector d;
    StaticHeader *staticheader = &obj->staticinfo->staticheader;
    bool32 coordsysChanged = FALSE;
    matrix rotxmatrix;
    matrix rotymatrix;
    matrix rotzmatrix;
    matrix tempmat;
    real32 angxrotate;
    real32 angyrotate;
    real32 angzrotate;

/*-----------------------------------------------------------------------------
    Update object position
-----------------------------------------------------------------------------*/

    // a = F/m
    vecScalarMultiply(a,obj->posinfo.force,staticheader->oneOverMass);

    // a = at
    vecMultiplyByScalar(a,phystimeelapsed);

    // v = v + at
    vecAddTo(obj->posinfo.velocity,a);

    if (TreatAsUdword(obj->posinfo.velocity.x) | TreatAsUdword(obj->posinfo.velocity.y) | TreatAsUdword(obj->posinfo.velocity.z))
    {
        obj->posinfo.isMoving = TRUE;

        if(((Missile *)obj)->FORCE_DROPPED == TRUE)
        {
            // cap velocity vector
            vecCapVectorSloppy(&obj->posinfo.velocity,((MissileStaticInfo *)((Missile *)obj)->staticinfo)->maxvelocity_FORCED);
        }
        else
        {
            // cap velocity vector
            vecCapVectorSloppy(&obj->posinfo.velocity,staticheader->maxvelocity);
        }
        // d = vt
        vecScalarMultiply(d,obj->posinfo.velocity,phystimeelapsed);

        // x = x + d
        vecAddTo(obj->posinfo.position,d);

    }
    else
    {
        obj->posinfo.isMoving = FALSE;
        vecZeroVector(d);
    }

    dbgAssert(bitTest(obj->flags,SOF_Rotatable));
    {

    /*-----------------------------------------------------------------------------
        Update object orientation
    -----------------------------------------------------------------------------*/

        if ((robj->rotinfo.torque.x != 0) || (robj->rotinfo.torque.y != 0) || (robj->rotinfo.torque.z != 0))
        {

            // alpha = Torque/momentOfInertia (about each axis)
            a.x = robj->rotinfo.torque.x * staticheader->oneOverMomentOfInertiaX;
            a.y = robj->rotinfo.torque.y * staticheader->oneOverMomentOfInertiaY;
            a.z = robj->rotinfo.torque.z * staticheader->oneOverMomentOfInertiaZ;

            // alpha = alpha*t
            vecMultiplyByScalar(a,phystimeelapsed);

            // w = w + alpha*t
            vecAddTo(robj->rotinfo.rotspeed,a);

            // cap rotation speed
            vecCapVectorSloppy(&robj->rotinfo.rotspeed,staticheader->maxrot);

        }

        // We now have w.x, w.y, and w.z which indicate the
        // rotational velocities (rad/s) around the x,y, and z axes of the ship.

    /*-----------------------------------------------------------------------------
        Update coordsys based on changes in rotation
    -----------------------------------------------------------------------------*/

        if (robj->rotinfo.rotspeed.x)
        {
            angxrotate = robj->rotinfo.rotspeed.x * phystimeelapsed;

            matMakeRotAboutX(&rotxmatrix,(real32)cos(angxrotate),(real32)sin(angxrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotxmatrix);
            coordsysChanged = TRUE;
        }

        if (robj->rotinfo.rotspeed.y)
        {
            angyrotate = robj->rotinfo.rotspeed.y * phystimeelapsed;

            matMakeRotAboutY(&rotymatrix,(real32)cos(angyrotate),(real32)sin(angyrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotymatrix);
            coordsysChanged = TRUE;
        }

        if (robj->rotinfo.rotspeed.z)
        {
            angzrotate = robj->rotinfo.rotspeed.z * phystimeelapsed;

            matMakeRotAboutZ(&rotzmatrix,(real32)cos(angzrotate),(real32)sin(angzrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotzmatrix);
            coordsysChanged = TRUE;
        }

        if (bitTest(obj->flags,SOF_Impactable))
        {
            if (coordsysChanged)
            {
                matMultiplyMatByVec(&irobj->collInfo.collOffset,&(irobj->rotinfo.coordsys),&staticheader->staticCollInfo.collsphereoffset);
                if ((universe.univUpdateCounter & UPDATE_COLLRECT_RATE) == UPDATE_COLLRECT_FRAME)
                    collUpdateCollRectangle(irobj);
            }

            vecAdd(irobj->collInfo.collPosition,obj->posinfo.position,irobj->collInfo.collOffset);

            obj->enginePosition = obj->posinfo.position;
            if (obj->trail != NULL)
            {
                mistrailUpdate(obj->trail, &obj->enginePosition);
            }
        }
        vecZeroVector(robj->rotinfo.torque);
    }
    vecZeroVector(obj->posinfo.force);
}

/*-----------------------------------------------------------------------------
    Name        : physUpdateObjPosVelBasic
    Description : Used for basic objects
                  updates a space objects position and orientation based
                  on any external forces/torques, as well as its current
                  velocity/spin.
    Inputs      : phystimeelapsed - time elapsed since last physics frame
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void physUpdateObjPosVelBasic(SpaceObj *obj,real32 phystimeelapsed)
{
    vector a;
    vector d;
    bool32 coordsysChanged = FALSE;
    matrix rotxmatrix;
    matrix rotymatrix;
    matrix rotzmatrix;
    matrix tempmat;
    real32 angxrotate;
    real32 angyrotate;
    real32 angzrotate;
    StaticHeader *staticheader;

    staticheader = &obj->staticinfo->staticheader;

/*-----------------------------------------------------------------------------
    Update object position
-----------------------------------------------------------------------------*/

    // a = F/m
    vecScalarMultiply(a,obj->posinfo.force,staticheader->oneOverMass);

    // a = at
    vecMultiplyByScalar(a,phystimeelapsed);

    // v = v + at
    vecAddTo(obj->posinfo.velocity,a);

    if (TreatAsUdword(obj->posinfo.velocity.x) | TreatAsUdword(obj->posinfo.velocity.y) | TreatAsUdword(obj->posinfo.velocity.z))
    {
        obj->posinfo.isMoving = TRUE;

        if (obj->objtype == OBJ_ShipType)
        {
            vecCapVectorSloppy(&obj->posinfo.velocity, (real32)(shipGetMaxSpeed((Ship *)obj) * staticheader->maxvelocity));
        }
        else
        {
            vecCapVectorSloppy(&obj->posinfo.velocity,staticheader->maxvelocity);
        }

        // d = vt
        vecScalarMultiply(d,obj->posinfo.velocity,phystimeelapsed);

        // x = x + d
        vecAddTo(obj->posinfo.position,d);

    }
    else
    {
        obj->posinfo.isMoving = FALSE;
        vecZeroVector(d);
    }

    if (bitTest(obj->flags,SOF_Rotatable))
    {

    /*-----------------------------------------------------------------------------
        Update object orientation
    -----------------------------------------------------------------------------*/

        if ((robj->rotinfo.torque.x != 0) || (robj->rotinfo.torque.y != 0) || (robj->rotinfo.torque.z != 0))
        {

            // alpha = Torque/momentOfInertia (about each axis)
            a.x = robj->rotinfo.torque.x * staticheader->oneOverMomentOfInertiaX;
            a.y = robj->rotinfo.torque.y * staticheader->oneOverMomentOfInertiaY;
            a.z = robj->rotinfo.torque.z * staticheader->oneOverMomentOfInertiaZ;

            // alpha = alpha*t
            vecMultiplyByScalar(a,phystimeelapsed);

            // w = w + alpha*t
            vecAddTo(robj->rotinfo.rotspeed,a);

            // cap rotation speed
            vecCapVectorSloppy(&robj->rotinfo.rotspeed,staticheader->maxrot);

        }

        // We now have w.x, w.y, and w.z which indicate the
        // rotational velocities (rad/s) around the x,y, and z axes of the ship.

    /*-----------------------------------------------------------------------------
        Update coordsys based on changes in rotation
    -----------------------------------------------------------------------------*/

        if (robj->rotinfo.rotspeed.x)
        {
            angxrotate = robj->rotinfo.rotspeed.x * phystimeelapsed;

            matMakeRotAboutX(&rotxmatrix,(real32)cos(angxrotate),(real32)sin(angxrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotxmatrix);
            coordsysChanged = TRUE;
        }

        if (robj->rotinfo.rotspeed.y)
        {
            angyrotate = robj->rotinfo.rotspeed.y * phystimeelapsed;

            matMakeRotAboutY(&rotymatrix,(real32)cos(angyrotate),(real32)sin(angyrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotymatrix);
            coordsysChanged = TRUE;
        }

        if (robj->rotinfo.rotspeed.z)
        {
            angzrotate = robj->rotinfo.rotspeed.z * phystimeelapsed;

            matMakeRotAboutZ(&rotzmatrix,(real32)cos(angzrotate),(real32)sin(angzrotate));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&(robj->rotinfo.coordsys),&tempmat,&rotzmatrix);
            coordsysChanged = TRUE;
        }

        if (bitTest(obj->flags,SOF_Impactable))
        {
            if (coordsysChanged)
            {
                matMultiplyMatByVec(&irobj->collInfo.collOffset,&(irobj->rotinfo.coordsys),&staticheader->staticCollInfo.collsphereoffset);
                if ((universe.univUpdateCounter & UPDATE_COLLRECT_RATE) == UPDATE_COLLRECT_FRAME)
                    collUpdateCollRectangle(irobj);
            }

            vecAdd(irobj->collInfo.collPosition,obj->posinfo.position,irobj->collInfo.collOffset);

        }

        vecZeroVector(robj->rotinfo.torque);
    }

    vecZeroVector(obj->posinfo.force);
}
// Cataclysm (0x480CE0): returns TRUE if ship should update the ping sensor grid when moved
bool32 physShipIsMovable(Ship *ship)
{
    if (ship->staticinfo->shipclass == CLASS_Component)
        return FALSE;

    if (ship->shiptype == 9 || ship->shiptype == 43)
        return FALSE;

    return TRUE;
}

// Cataclysm (0x54D180): set ship position and notify ping sensor grid if movable
void posSetShipPosition(Ship *ship, vector *newPosition)
{
    if (physShipIsMovable(ship))
        pingUpdateShipSensorGrid(ship, &ship->posinfo.position, newPosition);
    ship->posinfo.position = *newPosition;
}

// Cataclysm (0x4E4390): integrate force into velocity for one physics timestep
void physIntegrateForceToVelocity(Ship *ship)
{
    real32 oneOverMass = ship->staticinfo->staticheader.oneOverMass;
    ship->posinfo.velocity.x += oneOverMass * ship->posinfo.force.x * universe.unknown1;
    ship->posinfo.velocity.y += oneOverMass * ship->posinfo.force.y * universe.unknown1;
    ship->posinfo.velocity.z += oneOverMass * ship->posinfo.force.z * universe.unknown1;
}
