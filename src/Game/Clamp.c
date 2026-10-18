/*=============================================================================
    Name    : clamp.c
    Purpose : deals with latching one spaceobj to another

    Created 11/5/1997 by bpasechn
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>

#include "types.h"
#include "objtypes.h"
#include "spaceobj.h"
#include "memory.h"
#include "debug.h"
#include "universe.h"
#include "physics.h"
#include "univupdate.h"
#include "clamp.h"
#include "shipselect.h"

#define DEBUG_CLAMPING

static Ship *insideShipIteratorShip = NULL;
static sdword insideShipIteratorCount = 0;
static sdword insideShipIteratorIndex = 0;

Ship *insideShipIterator(Ship *ship)
{
    SpaceObjSelection *selection;
    Ship *insideShip;

    if (ship != NULL)
    {
        insideShipIteratorShip = ship;
        insideShipIteratorIndex = 0;
        insideShipIteratorCount = 0;

        if ((ship->clampInfo == NULL) || (ship->clampInfo->selection == NULL))
        {
            return NULL;
        }

        selection = (SpaceObjSelection *)ship->clampInfo->selection;
        insideShipIteratorCount = selection->numSpaceObjs;
    }
    else
    {
        ship = insideShipIteratorShip;
    }

    if ((ship == NULL) || (insideShipIteratorCount == 0))
    {
        return NULL;
    }

    selection = (SpaceObjSelection *)ship->clampInfo->selection;
    while (insideShipIteratorIndex < selection->numSpaceObjs)
    {
        insideShip = (Ship *)selection->SpaceObjPtr[insideShipIteratorIndex];
        if ((insideShip->objtype == OBJ_ShipType) &&
            (insideShip->staticinfo->shipclass == CLASS_Component))
        {
            insideShipIteratorIndex++;
            return insideShip;
        }
        insideShipIteratorIndex++;
    }

    return NULL;
}

Ship *GetFirstInsideShip(Ship *ship)
{
    return insideShipIterator(ship);
}

Ship *GetNextInsideShip(void)
{
    return insideShipIterator(NULL);
}

/*-----------------------------------------------------------------------------
    Name        :   clampObjToObj
    Description :   Attaches obj to dest in exact orientation obj is at with
                    respect to dest at call time (freezes position)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void clampObjToObj(SpaceObjRotImpTargGuidanceShipDerelict *obj, SpaceObjRotImpTargGuidanceShipDerelict *dest)
{
    ClampInfo *hostClampInfo;
    SelectCommand *attachedObjects;
    matrix tmpmat;
    vector tmpvec;

    obj->clampInfo = memAlloc(sizeof(ClampInfo),"ClampingInformation",NonVolatile);
    obj->clampInfo->selection = NULL;

    obj->clampInfo->host = (SpaceObjRotImpTarg *)dest;

    vecSub(tmpvec,obj->posinfo.position,dest->posinfo.position);
    matMultiplyVecByMat(&obj->clampInfo->clampOffset,&tmpvec,&dest->rotinfo.coordsys);

    //Store slaves coordsys with respect to MASTER's object space
    tmpmat = obj->rotinfo.coordsys;
    matTranspose(&tmpmat);
    matMultiplyMatByMat(&obj->clampInfo->clampCoordsys,&tmpmat,&dest->rotinfo.coordsys);
    matTranspose(&obj->clampInfo->clampCoordsys);

    bitSet(obj->flags,SOF_Clamped);

    hostClampInfo = dest->clampInfo;
    if (hostClampInfo == NULL)
    {
        hostClampInfo = memAlloc(sizeof(ClampInfo), "ClampingInformation", NonVolatile);
        memset(hostClampInfo, 0, sizeof(ClampInfo));
        dest->clampInfo = hostClampInfo;
    }

    attachedObjects = (SelectCommand *)hostClampInfo->selection;
    if (attachedObjects == NULL)
    {
        attachedObjects = memAlloc(sizeofSelectCommand(TOTAL_NUM_SHIPS), "ClampSelection", NonVolatile);
        memset(attachedObjects, 0, sizeofSelectCommand(TOTAL_NUM_SHIPS));
        hostClampInfo->selection = attachedObjects;
    }

    dbgAssert(attachedObjects->numShips < TOTAL_NUM_SHIPS);
    attachedObjects->ShipPtr[attachedObjects->numShips++] = (Ship *)obj;

    //later, allow for some looseness in clamp
    //(will allow for 1 univupdate frame of loosness as is
}

/*-----------------------------------------------------------------------------
    Name        :   unClampObj
    Description :   DeClamps a clamped object
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void unClampObj(SpaceObjRotImpTargGuidanceShipDerelict *obj)
{
    SpaceObjRotImpTargGuidanceShipDerelict *hostObject;
    ClampInfo *hostClampInfo;
    SelectCommand *attachedObjects;

//#ifdef DEBUG_CLAMPING
//    dbgAssert(obj->clampInfo != NULL);
//#endif
    if (obj->clampInfo != NULL)
    {
        if (obj->clampInfo->host != NULL)
        {
            hostObject = (SpaceObjRotImpTargGuidanceShipDerelict *)obj->clampInfo->host;
            hostClampInfo = hostObject->clampInfo;
            if (hostClampInfo != NULL && hostClampInfo->selection != NULL)
            {
                attachedObjects = (SelectCommand *)hostClampInfo->selection;
                clRemoveShipFromSelection(attachedObjects, (Ship *)obj);
                if (attachedObjects->numShips == 0)
                {
                    memFree(attachedObjects);
                    hostClampInfo->selection = NULL;
                    if (hostClampInfo->host == NULL)
                    {
                        memFree(hostClampInfo);
                        hostObject->clampInfo = NULL;
                    }
                }
            }
        }
        memFree(obj->clampInfo);
        obj->clampInfo = NULL;
        bitClear(obj->flags,SOF_Clamped);
    }
}

// Cataclysm: takes HOST (master), updates all slaves via clampInfo->selection.
// Each slave's position and orientation is set relative to its own clampInfo->host.
void updateClampedObject(SpaceObjRotImpTargGuidanceShipDerelict *host)
{
    SelectCommand *selection;
    SpaceObjRotImpTargGuidanceShipDerelict *slave;
    SpaceObjRotImpTarg *slaveHost;
    vector tmpvec;
    sdword i;
    selection = (SelectCommand *)host->clampInfo->selection;
    if (selection == NULL || selection->numShips <= 0)
        return;

    for (i = 0; i < selection->numShips; i++)
    {
        slave = (SpaceObjRotImpTargGuidanceShipDerelict *)selection->ShipPtr[i];
        if (slave == NULL)
        {
            selection = (SelectCommand *)host->clampInfo->selection;
            continue;
        }

        // Update slave physics
        if (slave->objtype == OBJ_DerelictType)
            physUpdateObjPosVelDerelicts((Derelict *)slave, universe.unknown1);
        else if (slave->objtype != OBJ_ShipType)
            physUpdateObjPosVelBasic((SpaceObj *)slave, universe.unknown1);
        else
            physUpdateObjPosVelShip((Ship *)slave, universe.unknown1);

        slaveHost = slave->clampInfo->host;

        // Decode slave's position in world space relative to its host
        matMultiplyMatByVec(&tmpvec, &slaveHost->rotinfo.coordsys, &slave->clampInfo->clampOffset);

        if (slave->objtype != OBJ_ShipType)
        {
            slave->posinfo.position.x = tmpvec.x + slaveHost->posinfo.position.x;
            slave->posinfo.position.y = tmpvec.y + slaveHost->posinfo.position.y;
            slave->posinfo.position.z = tmpvec.z + slaveHost->posinfo.position.z;
        }
        else
        {
            vector newpos;
            newpos.x = tmpvec.x + slaveHost->posinfo.position.x;
            newpos.y = tmpvec.y + slaveHost->posinfo.position.y;
            newpos.z = tmpvec.z + slaveHost->posinfo.position.z;
            posSetShipPosition((Ship *)slave, &newpos);
        }

        // Decode slave's orientation in world space relative to its host
        matMultiplyMatByMat(&slave->rotinfo.coordsys, &slaveHost->rotinfo.coordsys, &slave->clampInfo->clampCoordsys);

        slave->posinfo.velocity = slaveHost->posinfo.velocity;

        univUpdateObjRotInfo((SpaceObjRot *)slave);

        // Recursive: slave may itself be a master with sub-slaves
        if (slave->clampInfo->selection != NULL)
        {
            updateClampedObject(slave);
        }

        // Re-fetch selection in case it changed during the recursive call
        selection = (SelectCommand *)host->clampInfo->selection;
    }
}

// Cataclysm: attach a child object to its parent and register it in the
// parent's clamp selection.
void clampObjectSetup(SpaceObj *child, SpaceObj *parent)
{
    clampObjToObj((SpaceObjRotImpTargGuidanceShipDerelict *)child,
                  (SpaceObjRotImpTargGuidanceShipDerelict *)parent);
}

void clDetachShipFromCommand(Ship *ship)
{
    ClampInfo *shipClampInfo = ship->clampInfo;
    SpaceObjRotImpTarg *host;
    SpaceObjRotImpTargGuidanceShipDerelict *hostObject;
    ClampInfo *hostClampInfo;
    SelectCommand *attachedObjects;
    if (shipClampInfo == NULL)
    {
        return;
    }

    host = shipClampInfo->host;
    shipClampInfo->host = NULL;
    if (host != NULL)
    {
        hostObject = (SpaceObjRotImpTargGuidanceShipDerelict *)host;
        hostClampInfo = hostObject->clampInfo;
        if (hostClampInfo != NULL)
        {
            attachedObjects = (SelectCommand *)hostClampInfo->selection;
            if (attachedObjects != NULL)
            {
                clRemoveShipFromSelection(attachedObjects, ship);
                if (attachedObjects->numShips == 0)
                {
                    memFree(attachedObjects);
                    hostClampInfo->selection = NULL;
                    if (hostClampInfo->host == NULL)
                    {
                        memFree(hostClampInfo);
                        hostObject->clampInfo = NULL;
                        bitClear(hostObject->flags, SOF_Clamped);
                    }
                }
            }
        }
    }

    attachedObjects = (SelectCommand *)shipClampInfo->selection;
    if ((attachedObjects != NULL) && (attachedObjects->numShips == 0))
    {
        memFree(attachedObjects);
        shipClampInfo->selection = NULL;
    }

    if ((shipClampInfo->host == NULL) && (shipClampInfo->selection == NULL))
    {
        memFree(shipClampInfo);
        ship->clampInfo = NULL;
        bitClear(ship->flags, SOF_Clamped);
    }
}

#if 0  // Cat2: Binary insideShipIterator; live implementation at Clamp.c L301 (0x5AFD70)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AFD70
Ship* insideShipIterator(Ship* ship)
{
  int v1; // ecx
  int v2; // edx
  int v3; // ecx
  int result; // eax
  int *v5; // eax

  if ( a1 )
  {
    v1 = *(_DWORD *)(a1 + 444);
    if ( !v1 || !*(_DWORD *)(v1 + 52) )
    {
      dword_A762C0 = a1;
      dword_A762C4 = 0;
      dword_A762C8 = 0;
      return 0;
    }
    v2 = a1;
    dword_A762C0 = a1;
    v3 = 0;
    dword_A762C4 = **(_DWORD **)(*(_DWORD *)(a1 + 444) + 52);
    dword_A762C8 = 0;
  }
  else
  {
    v2 = dword_A762C0;
    v3 = dword_A762C8;
  }
  if ( !dword_A762C4 )
    return 0;
  v5 = *(int **)(*(_DWORD *)(v2 + 444) + 52);
  if ( v3 >= *v5 )
    return 0;
  while ( 1 )
  {
    result = v5[v3 + 2];
    if ( !*(_DWORD *)(result + 16) && *(_DWORD *)(*(_DWORD *)(result + 32) + 488) == 10 )
      break;
    dword_A762C8 = ++v3;
    v5 = *(int **)(*(_DWORD *)(v2 + 444) + 52);
    if ( v3 >= *v5 )
      return 0;
  }
  dword_A762C8 = v3 + 1;
  return result;
}
#endif
