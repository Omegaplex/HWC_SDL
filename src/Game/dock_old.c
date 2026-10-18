/*=============================================================================
    Name    : dock_old.c
    Purpose : Uncompiled archive for dead HW1-era docking helpers that are not
              present in the Cataclysm retail binary and have no live callers
              in this tree.
=============================================================================*/
#include "ShipDefs.h"
#include "types.h"
#include "vector.h"
#include "fastmath.h"
#include "spaceobj.h"
#include "universe.h"
#include "aitrack.h"
#include "aiship.h"
#include "dock.h"
#include "mothership.h"
#include "madlinkin.h"

bool32 dockFlyToConeInsideAndAlign(Ship *ship,Ship *dockwith,real32 headingTolerance,real32 dockDistance,ShipRace dockwithRace)
{
     DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
     vector coneheadingInWorldCoordSys;
     vector conepositionInWorldCoordSys;
     vector heading;
     vector destination;
     bool32 ok=FALSE;

     matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);
     matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
     vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);


     vecScalarMultiply(destination,coneheadingInWorldCoordSys,dockDistance);
     vecAddTo(destination,conepositionInWorldCoordSys);



     aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_DontFlyToObscuredPoints|AISHIP_IgnoreDockWithObscuredPoints|AISHIP_ReturnImmedIfPointObscured, CONEINSIDE_MINFLYSPEED,&dockwith->posinfo.velocity);

     if (MoveReachedDestinationVariable(ship,&destination,100.0f))
     {
         ok = TRUE;
     }
     if(dockwithRace==RACE_Beast && ShipTypeIsMothershipType(dockwith->shiptype))
     {
        matGetVectFromMatrixCol3(heading,dockwith->rotinfo.coordsys);
     }
     else
     {
        vecScalarMultiply(heading,coneheadingInWorldCoordSys,-1.0f);
     }
     if(aitrackHeading(ship,&heading,headingTolerance))
     {
         if(ok)
         {
             return TRUE;
         }
     }

     return FALSE;
}

static bool32 dockFlyToConeOriginEx(Ship *ship,Ship *dockwith,real32 tolerance,real32 minflyspeed,bool32 pointAlongConeVector, bool32 faceWithDockwith)
{
    vector conepositionInWorldCoordSys,tmpvec;
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    sdword flags = 0;
    sdword overide;

    if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        vecAdd(tmpvec,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&tmpvec);
    }
    else
    {
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    }

    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,tolerance))
    {
        return TRUE;
    }
    else
    {
        if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
        {
            vector desiredHeading,desiredUp;
            GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,&dockwith->rotinfo.coordsys);
            GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,&dockwith->rotinfo.coordsys);
            aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
            flags = 0;
        }
        else if (pointAlongConeVector)
        {
            vector heading;
            vecSub(heading,conepositionInWorldCoordSys,ship->posinfo.position);
            vecNormalize(&heading);
            aitrackHeading(ship,&heading,FLYSHIP_HEADINGACCURACY);
            flags = 0;
        }
        else if(faceWithDockwith)
        {
            vector heading;
            matGetVectFromMatrixCol3(heading,dockwith->rotinfo.coordsys);
            aitrackHeading(ship,&heading,0.999f);
            flags = 0;
        }
        else
        {
            flags = AISHIP_FirstPointInDirectionFlying + AISHIP_PointInDirectionFlying;
        }
        aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,flags,minflyspeed,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool32 dockFlyToConeOriginLatchIfWithin(Ship *ship,Ship *dockwith,real32 withindistsqr)
{
    vector conepositionInWorldCoordSys,tmpvec;
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    vector desiredHeading;
    vector desiredUp;
    vector lefttogo;
    real32 distsqr;
    sdword overide;

    dbgAssert(dockstaticpoint->type == DPT_SalvageLatch ||
              dockstaticpoint->type == DPT_Corvette_Latch ||
              dockstaticpoint->type == DPT_Fighter_Latch);

    if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        vecAdd(tmpvec,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&tmpvec);
    }
    else
    {
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    }

    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    if (withindistsqr == 0.0f)
    {
        goto latchimmediately;
    }

    vecSub(lefttogo,ship->posinfo.position,conepositionInWorldCoordSys);
    distsqr = vecMagnitudeSquared(lefttogo);

    if (distsqr > withindistsqr)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,AISHIP_PointInDirectionFlying,-50.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
    else
    {
latchimmediately:
        if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,LATCH_TOLERANCE))
        {
            ship->posinfo.velocity = dockwith->posinfo.velocity;
            return TRUE;
        }
        else
        {
            if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
            {
                GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,&dockwith->rotinfo.coordsys);
                GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,&dockwith->rotinfo.coordsys);
            }
            else
            {
                GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,&dockwith->rotinfo.coordsys);
                GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,&dockwith->rotinfo.coordsys);
            }
            aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
            aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,0,-50.0f,&dockwith->posinfo.velocity);
            return FALSE;
        }
    }
}

bool32 dockFlyToConeOriginLatch(Ship *ship,Ship *dockwith)
{
    vector conepositionInWorldCoordSys,tmpvec;
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    vector desiredHeading;
    vector desiredUp;
    sdword overide;

    dbgAssert(dockstaticpoint->type == DPT_SalvageLatch ||
              dockstaticpoint->type == DPT_Corvette_Latch ||
              dockstaticpoint->type == DPT_Fighter_Latch);

    if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,&dockwith->rotinfo.coordsys);
        GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,&dockwith->rotinfo.coordsys);
    }
    else
    {
        GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,&dockwith->rotinfo.coordsys);
        GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,&dockwith->rotinfo.coordsys);
    }

    if((overide = dockNeedOffsetOveride(dockwith,ship,dockstaticpoint)) != -1)
    {
        vecAdd(tmpvec,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].offset,dockstaticpoint->position);
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&tmpvec);
    }
    else
    {
        matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    }

    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    if (MoveReachedDestinationVariable(ship,&conepositionInWorldCoordSys,LATCH_TOLERANCE))
    {
        ship->posinfo.velocity = dockwith->posinfo.velocity;
        return TRUE;
    }
    else
    {
        aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
        aishipFlyToPointAvoidingObjsWithVel(ship,&conepositionInWorldCoordSys,0,-50.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool32 dockBackoutOfConeLatch(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,backupDistance);
    vecAddTo(destination,conepositionInWorldCoordSys);

    if ((MoveReachedDestinationVariable(ship,&destination,BACKOUT_TOLERANCE)))
    {
        return TRUE;
    }
    else
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,BACKOUT_MINFLYSPEED,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool32 dockBackoutOfConeLatchFacingProperly(Ship *ship,Ship *dockwith,real32 withindistsqr)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;
    vector lefttogo;
    real32 distsqr;
    vector desiredHeading;
    vector desiredUp;
    sdword overide;


    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,backupDistance);
    vecAddTo(destination,conepositionInWorldCoordSys);

    vecSub(lefttogo,ship->posinfo.position,conepositionInWorldCoordSys);
    distsqr = vecMagnitudeSquared(lefttogo);

    if (distsqr > withindistsqr)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,BACKOUT_MINFLYSPEED,&dockwith->posinfo.velocity);
        return FALSE;
    }
    else
    {
        if (MoveReachedDestinationVariable(ship,&destination,BACKOUT_TOLERANCE))
        {
            return TRUE;
        }
        else
        {
            if((overide = dockNeedRotationOveride(dockwith,ship,dockstaticpoint)) != -1)
            {
                GetDirectionVectorOfShip(&desiredHeading,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].heading,&dockwith->rotinfo.coordsys);
                GetDirectionVectorOfShip(&desiredUp,dockwith->staticinfo->dockOverideInfo->dockOverides[overide].up,&dockwith->rotinfo.coordsys);
            }
            else
            {
                GetDirectionVectorOfShip(&desiredHeading,dockstaticpoint->headingdirection,&dockwith->rotinfo.coordsys);
                GetDirectionVectorOfShip(&desiredUp,dockstaticpoint->updirection,&dockwith->rotinfo.coordsys);
            }
            aitrackHeadingAndUp(ship,&desiredHeading,&desiredUp,FLYSHIP_HEADINGACCURACY);
            aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,BACKOUT_MINFLYSPEED,&dockwith->posinfo.velocity);
            return FALSE;
        }
    }
    return FALSE;
}

bool32 dockFlyOutOfCone(Ship *ship,Ship *dockwith,sdword headingdirection,sdword updirection)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector destination;
    vector distanceawayVec;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,10000.0f);
    vecAddTo(destination,conepositionInWorldCoordSys);

    vecSub(distanceawayVec,ship->posinfo.position,conepositionInWorldCoordSys);
    if (vecMagnitudeSquared(distanceawayVec) >= (backupDistance*backupDistance))
    {
        return TRUE;
    }
    else
    {
        udword flags;
        vector heading;
        vector up;

        if ((headingdirection >= 0) && (updirection >= 0))
        {
            dbgAssert(headingdirection != updirection);
            GetDirectionVectorOfShip(&heading,headingdirection,&dockwith->rotinfo.coordsys);
            GetDirectionVectorOfShip(&up,updirection,&dockwith->rotinfo.coordsys);
            aitrackHeadingAndUp(ship,&heading,&up,FLYSHIP_HEADINGACCURACY);
            flags = 0;
        }
        else
        {
            flags = AISHIP_PointInDirectionFlying;
        }

        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,flags,0.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

bool32 dockFlyAboveMothershipDoor(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector destination;
    vector dest1;
    vector heading,up,right;
    matrix coordsysWS;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    matGetVectFromMatrixCol3(dest1,coordsysWS);
    vecScalarMultiply(dest1,dest1,-1.0f);

    vecScalarMultiply(dest1,dest1,-1800.0f);
    vecAddTo(destination,dest1);

    if(MoveReachedDestinationVariable(ship,&destination,250.0f))
    {
        return TRUE;
    }
    else
    {
        aitrackHeadingAndUp(ship,&heading,&up,1.0f);
        aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,0.0f,&dockwith->posinfo.velocity);
        return FALSE;
    }
}

#define DOOR_Stage1_Right   -3700.0f
#define DOOR_Stage1_Up  -1550.0f

bool32 dockFlyToBottomOfDoor1(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector destination;
    vector dest1;
    vector heading,up,right,doorheading,doorup;
    matrix coordsysWS;
    bool32 track = FALSE;
    bool32 dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);
    matGetVectFromMatrixCol1(doorheading,coordsysWS);
    matGetVectFromMatrixCol2(doorup,coordsysWS);

    vecScalarMultiply(dest1,doorheading,DOOR_Stage1_Right);
    vecAddTo(destination,dest1);
    vecScalarMultiply(dest1,up,DOOR_Stage1_Up);
    vecAddTo(destination,dest1);

    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,AISHIP_PointInDirectionFlying,0.0f,&dockwith->posinfo.velocity);

    if(MoveReachedDestinationVariable(ship,&destination,400.0f))
    {
        dest = TRUE;
    }
    if(dest)
        return TRUE;

    return FALSE;
}

#define DOOR_Stage2_UpOffset  -2700.0f

bool32 dockFlyToBottomOfDoor2(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector destination;
    vector dest1;
    vector heading,up,right;
    matrix coordsysWS;
    bool32 track = FALSE;
    bool32 dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);

    vecScalarMultiply(dest1,heading,DOOR_Stage2_UpOffset);
    vecAddTo(destination,dest1);

    track = aitrackHeadingAndUp(ship,&heading,&up,0.98f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);

    if(MoveReachedDestinationVariable(ship,&destination,100.0f))
    {
        dest = TRUE;
    }
    if(dest && track)
        return TRUE;

    return FALSE;
}

bool32 dockFlyToBottomOfDoor3(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector destination;
    vector dest1;
    vector heading,up,right;
    matrix coordsysWS;
    bool32 track = FALSE;
    bool32 dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);

    vecScalarMultiply(dest1,heading,-2700.0f);
    vecAddTo(destination,dest1);

    track = aitrackHeadingAndUp(ship,&heading,&up,0.98f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);

    if(MoveReachedDestinationVariable(ship,&destination,100.0f))
    {
        dest = TRUE;
    }
    if(dest && track)
        return TRUE;

    return FALSE;
}

bool32 dockFlyToDoor(Ship *ship,Ship *dockwith)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    real32 backupDistance = dockstaticpoint->flyawaydist;
    vector destination;
    vector heading,up,right;
    matrix coordsysWS;
    bool32 track = FALSE;
    bool32 dest = FALSE;

    mothershipGetCargoPosition(dockwith,(SpaceObjRotImpTargGuidanceShipDerelict *)ship,&destination,&coordsysWS,&heading,&up,&right);

    track = aitrackHeadingAndUp(ship,&heading,&up,1.0f);
    aishipFlyToPointAvoidingObjsWithVel(ship,&destination,0,0.0f,&dockwith->posinfo.velocity);

    if(MoveReachedDestinationVariable(ship,&destination,TW_R1_DOOR_DOCK_TOLERANCE))
    {
        dest = TRUE;
    }
    if(dest)
        return TRUE;

    return FALSE;
}

static bool32 dockFlyToConeInsideDistance(Ship *ship,Ship *dockwith,real32 dockDistance)
{
    DockStaticPoint *dockstaticpoint = ship->dockvars.dockstaticpoint;
    vector coneheadingInWorldCoordSys;
    vector conepositionInWorldCoordSys;
    vector coneToShip;
    real32 coneToShipMag;
    real32 dotprod;
    vector destination;

    matMultiplyMatByVec(&coneheadingInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->conenormal);

    matMultiplyMatByVec(&conepositionInWorldCoordSys,&dockwith->rotinfo.coordsys,&dockstaticpoint->position);
    vecAddTo(conepositionInWorldCoordSys,dockwith->posinfo.position);

    vecSub(coneToShip,ship->posinfo.position,conepositionInWorldCoordSys);
    coneToShipMag = fsqrt(vecMagnitudeSquared(coneToShip));
    dotprod = vecDotProduct(coneheadingInWorldCoordSys,coneToShip);

    if (dotprod >= (dockstaticpoint->coneangle*coneToShipMag))
    {
        return TRUE;
    }

    vecScalarMultiply(destination,coneheadingInWorldCoordSys,dockDistance);
    vecAddTo(destination,conepositionInWorldCoordSys);

    if ((MoveReachedDestinationVariable(ship,&destination,CONEINSIDE_TOLERANCE)))
    {
rinside:
        return TRUE;
    }
    else
    {
        if (aishipFlyToPointAvoidingObjsWithVel(ship,&destination,
            AISHIP_DontFlyToObscuredPoints|AISHIP_IgnoreDockWithObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FirstPointInDirectionFlying|AISHIP_PointInDirectionFlying, CONEINSIDE_MINFLYSPEED,&dockwith->posinfo.velocity)
            & AISHIP_FLY_OBJECT_IN_WAY)
        {
            goto rinside;
        }
        return FALSE;
    }
}

sdword ThisShipIs(ShipStaticInfo *shipstatic)
{
    if (shipstatic->shipclass == CLASS_Fighter || ShipTypeIsProximitySensorType(shipstatic->shiptype))
    {
        return THISSHIPIS_FIGHTER;
    }
    else if (shipstatic->shipclass == CLASS_Corvette)
    {
        return THISSHIPIS_CORVETTE;
    }
    else if (ShipTypeIsWorker(shipstatic->shiptype))
    {
        return THISSHIPIS_RESOURCER;
    }
    else
    {
        return THISSHIPIS_OTHERNONCAPITALSHIP;
    }
}

bool32 shipRetireNeedsMothership(Ship *ship)
{
    if(isCapitalShipStatic(ship->staticinfo))
    {
        if(ship->staticinfo->shipclass == CLASS_Mothership   ||
           ship->staticinfo->shipclass == CLASS_HeavyCruiser ||
           ship->staticinfo->shipclass == CLASS_Carrier ||
           ship->staticinfo->shipclass == CLASS_Destroyer ||
           ShipTypeIsProcessorType(ship->shiptype))
        {
            return TRUE;
        }
    }
    return FALSE;
}

bool32 dockShipRefuelsAtShip(Ship *ship,Ship *dockwith)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    ShipStaticInfo *dockwithstaticinfo = (ShipStaticInfo *)dockwith->staticinfo;
    bool32 alldone = TRUE;

    dbgAssert(dockwithstaticinfo->canReceiveShips | dockwithstaticinfo->canReceiveResources);

    if ((dockwithstaticinfo->canReceiveResources) && (ship->resources > 0))
    {
#ifdef DEBUG_DOCKING
        dbgMessagef("\nDepositing %d RU",ship->resources);
#endif
        ship->playerowner->resourceUnits += ship->resources;
        universe.gameStats.playerStats[ship->playerowner->playerIndex].totalResourceUnitsCollected += ship->resources;
        universe.gameStats.playerStats[ship->playerowner->playerIndex].totalResourceUnits += ship->resources;
        universe.gameStats.totalResourceUnitsCollected+=ship->resources;
        if ((singlePlayerGame) && (ship->playerowner->playerIndex == 0))
        {
            singlePlayerGameInfo.resourceUnitsCollected += ship->resources;
        }
        ship->resources = 0;
        soundEvent(ship, Ship_ResourceTransfer);
        speechEvent(ship, STAT_ResCol_ResourcesTransferred, 0);
    }

    if (dockwithstaticinfo->repairOtherShipRate != 0.0f)
    {
        if (ship->health < shipstaticinfo->maxhealth)
        {
            if(ShipTypeIsWorker(ship->shiptype) &&
               dockwithstaticinfo->repairResourceCollectorRate != 0.0f)
            {
                ship->health += dockwithstaticinfo->repairResourceCollectorRate * shipstaticinfo->maxhealth * 0.01f;
            }
            else
            {
                ship->health += dockwithstaticinfo->repairOtherShipRate * shipstaticinfo->maxhealth * 0.01f;
            }
            if (ship->health > shipstaticinfo->maxhealth)
            {
                ship->health = shipstaticinfo->maxhealth;
            }
            else
            {
                alldone = FALSE;
            }
        }
    }

    if (dockwithstaticinfo->pumpFuelRate != 0.0f)
    {
        if (ship->fuel < shipstaticinfo->maxfuel)
        {
            ship->fuel += dockwithstaticinfo->pumpFuelRate * universe.unknown1;
            if (ship->fuel > shipstaticinfo->maxfuel)
            {
                ship->fuel = shipstaticinfo->maxfuel;
            }
            else
            {
                alldone = FALSE;
            }
        }
    }
    if(ship->forceCancelDock)
    {
        alldone = TRUE;
    }
    return alldone;
}

sdword dockFindLaunchPoint(sdword *dockpointindices[],Ship *dockwith)
{
    sdword *curlaunchpoint = dockpointindices[0];
    sdword curlaunchindex;
    DockInfo *dockInfo = dockwith->dockInfo;

    while (curlaunchpoint != NULL)
    {
        curlaunchindex = *curlaunchpoint;
        dbgAssert(curlaunchindex < dockInfo->numDockPoints);

        if (!dockInfo->dockpoints[curlaunchindex].thisDockBusy)
        {
            return curlaunchindex;
        }

        dockpointindices++;
        curlaunchpoint = *dockpointindices;
    }

    return -1;
}

sdword dockFindLaunchPointRandom(sdword *dockpointindices[],Ship *dockwith,Ship *ship)
{
    sdword *curlaunchpoint = dockpointindices[0];
    sdword curlaunchindex;
    DockInfo *dockInfo = dockwith->dockInfo;
    sdword numDockPoints = dockInfo->numDockPoints;
    sdword *PotentialIndices;
    sdword foundNumberIndices = 0;
    sdword retval;

    PotentialIndices = memAlloc(numDockPoints<<2,"pi(potentialindxs)",Pyrophoric);

    while (curlaunchpoint != NULL)
    {
        curlaunchindex = *curlaunchpoint;
        dbgAssert(curlaunchindex < numDockPoints);

        if (!dockInfo->dockpoints[curlaunchindex].thisDockBusy)
        {
            if(dockwith->shiprace == RACE_Sect)
            {
                if (ShipTypeIsMothershipType(dockwith->shiptype))
                {
                    if(stricmp(dockInfo->dockpoints[curlaunchindex].dockstaticpoint->name,"Big") == 0)
                    {
                        if(dockwith->madDoorStatus != MAD_STATUS_DOOR_CLOSED)
                        {
                            goto busyDockPoint;
                        }
                        else
                        {
                            ship->specialFlags |= SPECIAL_EXITING_DOOR;
                        }
                    }
                }
            }

            PotentialIndices[foundNumberIndices] = curlaunchindex;
            foundNumberIndices++;
        }

    busyDockPoint:

        dockpointindices++;
        curlaunchpoint = *dockpointindices;
    }

    if (foundNumberIndices == 0)
    {
        retval = -1;
    }
    else if (foundNumberIndices == 1)
    {
        retval = PotentialIndices[0];
    }
    else
    {
        retval = PotentialIndices[random(foundNumberIndices)];
    }

    memFree(PotentialIndices);

    return retval;
}
