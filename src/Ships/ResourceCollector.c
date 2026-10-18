/*=============================================================================
    Name    : ResourceCollector.c
    Purpose : Specifics for the ResourceCollector ship

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "ResourceCollector.h"
#include "statscript.h"
#include "RepairDroid.h"

typedef struct
{
    real32 repairApproachDistance;
} ResourceCollectorStatics;

ResourceCollectorStatics ResourceCollectorStatic;
ResourceCollectorStatics ResourceCollectorStaticRace1;
ResourceCollectorStatics ResourceCollectorStaticRace2;

scriptStructEntry ResourceCollectorStaticScriptTable[] =
{
    { "repairApproachDistance", scriptSetReal32CB, (udword)&(ResourceCollectorStatic.repairApproachDistance), (udword)&(ResourceCollectorStatic) },
    { NULL, NULL, 0, 0 }
};

void ResourceCollectorStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    ResourceCollectorStatics *resourcestat = (statinfo->shiprace == RACE_Sect) ? &ResourceCollectorStaticRace1 : &ResourceCollectorStaticRace2;

    statinfo->custstatinfo = resourcestat;
    scriptSetStruct(directory, filename, ResourceCollectorStaticScriptTable, (ubyte *)resourcestat);
}

void ResourceCollectorInit(Ship *ship)
{
    ResourceCollectorSpec *spec = (ResourceCollectorSpec *)ship->ShipSpecifics;

    spec->resourcesCollected = 0;
}

bool32 ResourceCollectorSpecialTarget(Ship *ship, void *custom)
{
    SelectAnyCommand *targets = (SelectAnyCommand *)custom;
    return repairDroidAIUpdate(ship, targets, ship->staticinfo->bulletRange[1]);
}

CustShipHeader ResourceCollectorHeader =
{
    sizeof(ResourceCollectorSpec),
    ResourceCollectorStaticInit,
    NULL,
    ResourceCollectorInit,
    NULL,
    NULL,
    NULL,
    NULL,
    ResourceCollectorSpecialTarget,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
