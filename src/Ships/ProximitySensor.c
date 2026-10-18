/*=============================================================================
    Name    : ProximitySensor.c
    Purpose : Specifics for the ProximitySensor

    Created 01/06/1998 by bpasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "ProximitySensor.h"
#include "Battle.h"
#include "MadLinkIn.h"
#include "MadLinkInDefs.h"
#include "nebulae.h"
#include "ping.h"
#include "prim2d.h"
#include "SoundEvent.h"
#include "statscript.h"
#include "tweak.h"
#include "universe.h"

color ProximitySensorBlipColor = colWhite;

ProximitySensorStatics ProximitySensorStatic;
ProximitySensorStatics ProximitySensorStaticRace1;
ProximitySensorStatics ProximitySensorStaticRace2;

scriptStructEntry ProximitySensorStaticScriptTable[] =
{
    { "SearchRate", scriptSetUdwordCB, (udword)&(ProximitySensorStatic.SearchRate), (udword)&(ProximitySensorStatic) },
    { "SearchRadius", scriptSetReal32CB, (udword)&(ProximitySensorStatic.SearchRadius), (udword)&(ProximitySensorStatic) },
    { "SensorCircleRadius", scriptSetSdwordCB, (udword)&(ProximitySensorStatic.SensorCircleRadius), (udword)&(ProximitySensorStatic) },
    { "SensorBlinkRate", scriptSetReal32CB, (udword)&(ProximitySensorStatic.SensorBlinkRate), (udword)&(ProximitySensorStatic) },
    { "SearchRateAfterFound", scriptSetUdwordCB, (udword)&(ProximitySensorStatic.SearchRateAfterFound), (udword)&(ProximitySensorStatic) },
    { "TriggerSpeed", scriptSetReal32CB, (udword)&(ProximitySensorStatic.TriggerSpeed), (udword)&(ProximitySensorStatic) },
    { "blipColor", scriptSetRGBACB, (udword)&(ProximitySensorStatic.blipColor), (udword)&(ProximitySensorStatic) },
    { "blipThickness", scriptSetSdwordCB, (udword)&(ProximitySensorStatic.blipThickness), (udword)&(ProximitySensorStatic) },
    { NULL, NULL, 0, 0 }
};

sdword nebTendrilRevealEnemies(Ship *ship);

void ProximitySensorStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    ProximitySensorStatics *proximitySensorStat = (statinfo->shiprace == RACE_Kushan) ? &ProximitySensorStaticRace1 : &ProximitySensorStaticRace2;

    statinfo->custstatinfo = proximitySensorStat;
    scriptSetStruct(directory, filename, ProximitySensorStaticScriptTable, (ubyte *)proximitySensorStat);

    proximitySensorStat->SearchRadiusSqr = proximitySensorStat->SearchRadius * proximitySensorStat->SearchRadius;
    proximitySensorStat->TriggerSpeedSqr = proximitySensorStat->TriggerSpeed * proximitySensorStat->TriggerSpeed;
}

void ProximitySensorInit(Ship *ship)
{
    ProximitySensorSpec *spec = (ProximitySensorSpec *)ship->ShipSpecifics;

    spec->sensorState = SENSOR_BEGIN;
    spec->blipRadius = 0.0f;
    spec->TAGGED = FALSE;
}

void proxInFocusSelection(SelectCommand *focus)
{
    sdword count;

    for (count = 0; count < focus->numShips; count++)
    {
        if (ShipTypeIsProximitySensorType(focus->ShipPtr[count]->shiptype))
        {
            ((ProximitySensorSpec *)focus->ShipPtr[count]->ShipSpecifics)->TAGGED = TRUE;
        }
    }
}

bool32 proxShouldDrawOverlay(Ship *ship)
{
    ProximitySensorSpec *spec = (ProximitySensorSpec *)ship->ShipSpecifics;

    return (ship->playerowner == universe.curPlayerPtr) &&
           (spec->sensorState == SENSOR_SENSED || spec->sensorState == SENSOR_SENSED2);
}

udword proxBlipColor(Ship *ship)
{
    ProximitySensorStatics *proximitySensorStatics = (ProximitySensorStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    ProximitySensorBlipColor = proximitySensorStatics->blipColor;
    return ProximitySensorBlipColor;
}

sdword proxBlipThickness(Ship *ship)
{
    ProximitySensorStatics *proximitySensorStatics = (ProximitySensorStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    return proximitySensorStatics->blipThickness;
}

sdword proxGetBlipRadius(Ship *ship)
{
    return (sdword)((ProximitySensorSpec *)ship->ShipSpecifics)->blipRadius;
}

bool32 ProximitySensorPingTimeout(ping *hellaPing, udword userID, char *userData, bool32 bRemoveReferences)
{
    ProximitySensorSpec *spec = (ProximitySensorSpec *)((Ship *)userID)->ShipSpecifics;

    (void)hellaPing;
    (void)userData;

    if (bRemoveReferences)
    {
        return ((SpaceObj *)userID == pingDyingObject) ? TRUE : FALSE;
    }

    return (spec->sensorState != SENSOR_SENSED && spec->sensorState != SENSOR_SENSED2) ? TRUE : FALSE;
}

void ProximitySensorHouseKeep(Ship *ship)
{
    ping *newPing;
    ProximitySensorStatics *proximitySensorStatics = (ProximitySensorStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;
    ProximitySensorSpec *spec = (ProximitySensorSpec *)ship->ShipSpecifics;

    if (ship->posinfo.isMoving & ISMOVING_MOVING)
    {
        if (vecMagnitudeSquared(ship->posinfo.velocity) > 40000.0f)
        {
            if (ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPEN)
            {
                madLinkInCloseSpecialShip(ship);
            }
        }
        else if (ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPEN &&
                 ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPENING &&
                 ship->madSpecialStatus != MAD_STATUS_SPECIAL_CLOSING)
        {
            madLinkInOpenSpecialShip(ship);
        }
    }
    else if (ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPEN &&
             ship->madSpecialStatus != MAD_STATUS_SPECIAL_OPENING)
    {
        madLinkInOpenSpecialShip(ship);
    }

    switch (spec->sensorState)
    {
        case SENSOR_BEGIN:
            spec->sensorState = SENSOR_SENSE;
            break;

        case SENSOR_SENSE:
            if ((universe.univUpdateCounter & TW_ProximitySensorSearchRate) == TW_ProximitySensorSearchFrame)
            {
                if (nebTendrilRevealEnemies(ship))
                {
                    spec->sensorState = SENSOR_SENSED;
                    spec->blipRadius = 0.0f;
                    spec->TAGGED = FALSE;

                    if (ship->playerowner->playerIndex == universe.curPlayerIndex)
                    {
                        newPing = pingCreate(NULL, (SpaceObj *)ship, ProximitySensorPingTimeout, NULL, 0, (udword)ship);
                        newPing->c = proxBlipColor(ship);
                        newPing->size = proximitySensorStatics->SearchRadius;
                        newPing->minScreenSize = primScreenToGLScaleX(2);
                        newPing->pingDuration = 1.0f / proximitySensorStatics->SensorBlinkRate;
                        newPing->interPingPause = 0.25f;
                        newPing->TOMask = PTOM_Proximity;
                    }
                }
            }
            break;

        case SENSOR_SENSED:
            if (!spec->TAGGED)
            {
                spec->sensorState = SENSOR_SENSED2;
            }
            /* fall through */

        case SENSOR_SENSED2:
            spec->blipRadius += proximitySensorStatics->SensorBlinkRate;
            if (spec->blipRadius >= (real32)proximitySensorStatics->SensorCircleRadius)
            {
                spec->blipRadius = 0.0f;
            }

            if ((universe.univUpdateCounter & TW_ProximitySensorSearchRate) == TW_ProximitySensorSearchFrame)
            {
                if (!nebTendrilRevealEnemies(ship))
                {
                    spec->sensorState = SENSOR_SENSE;
                    spec->TAGGED = FALSE;
                }
            }
            break;
    }
}

#ifndef STATVIEWER_PROGRAM
bool32 proximityCanPlayerSeeShip(Player *player, Ship *shipInQuestion)
{
    sdword mask = 0x01;

    if (player == NULL)
    {
        return FALSE;
    }

    mask <<= player->playerIndex;
    return bitTest(shipInQuestion->visibleToWho, mask) ? TRUE : FALSE;
}
#endif

CustShipHeader ProximitySensorHeader =
{
    sizeof(ProximitySensorSpec),
    ProximitySensorStaticInit,
    NULL,
    ProximitySensorInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ProximitySensorHouseKeep,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
