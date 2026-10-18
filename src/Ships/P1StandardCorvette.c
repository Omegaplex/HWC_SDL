/*=============================================================================
    Name    : P1StandardCorvette.c
    Purpose : Specifics for the P1StandardCorvette

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "P1StandardCorvette.h"
#include "statscript.h"
#include "gun.h"
#include "AIShip.h"
#include "attack.h"
#include "CommandLayer.h"
#include "matrix.h"
#include "Ship.h"
#include "universe.h"
#include "DefaultShip.h"
#include "SalCapCorvette.h"

void SalCapResetHostQueueScriptData(Ship *ship);
void shipShootAllGunsAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);

typedef struct
{
    AttackSideStepParameters sidestepParameters;
} P1StandardCorvetteStatics;

P1StandardCorvetteStatics P1StandardCorvetteStatic;

#define P1_CORVETTE_ACTIVE_TYPE_OFFSET 1796
#define P1_CORVETTE_ACTIVE_RACE_OFFSET 1800
#define P1_CORVETTE_ACTIVE_BINDINGS_OFFSET 1808
#define P1_CORVETTE_ACTIVE_SLOT_INDEX_OFFSET 1820
#define P1_CORVETTE_ACTIVE_RADIUS_OFFSET 1824
#define P1_CORVETTE_PENDING_TYPE_OFFSET 1840
#define P1_CORVETTE_PENDING_RACE_OFFSET 1844
#define P1_CORVETTE_PENDING_BINDINGS_OFFSET 1852
#define P1_CORVETTE_PENDING_SLOT_INDEX_OFFSET 1864
#define P1_CORVETTE_PENDING_RADIUS_OFFSET 1868
#define P1_CORVETTE_MORPH_PROGRESS_OFFSET 1884
#define P1_CORVETTE_STATE_OFFSET 1888
#define P1_CORVETTE_ACTIVE_EFFECT_OFFSET 1900
#define P1_CORVETTE_SPECIAL_STATE_OFFSET 1948

#define P1_STANDARD_CORVETTE_LINK_ACTIVE_FLAGS 0x6u

void P1StandardCorvetteStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    P1StandardCorvetteStatics *corvstat = &P1StandardCorvetteStatic;

    memset(corvstat,sizeof(*corvstat),0);
    scriptSetStruct(directory,filename,AttackSideStepParametersScriptTable,(ubyte *)&corvstat->sidestepParameters);

    statinfo->custstatinfo = corvstat;
}

void P1StandardCorvetteInit(Ship *ship)
{
    P1StandardCorvetteSpec *spec = (P1StandardCorvetteSpec *)ship->ShipSpecifics;
    ubyte *shipBytes = (ubyte *)ship;

    attackSideStepInit(&spec->attacksidestep);
    ship->flags2 |= BIT3;
    *(ubyte *)(shipBytes + P1_CORVETTE_STATE_OFFSET) = 0;
    *(sdword *)(shipBytes + P1_CORVETTE_MORPH_PROGRESS_OFFSET) = 0;
    *(sdword *)(shipBytes + P1_CORVETTE_ACTIVE_BINDINGS_OFFSET) = 0;
    *(sdword *)(shipBytes + P1_CORVETTE_ACTIVE_SLOT_INDEX_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_ACTIVE_TYPE_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_ACTIVE_RACE_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_ACTIVE_RADIUS_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_PENDING_BINDINGS_OFFSET) = 0;
    *(sdword *)(shipBytes + P1_CORVETTE_PENDING_SLOT_INDEX_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_PENDING_TYPE_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_PENDING_RACE_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_PENDING_RADIUS_OFFSET) = -1;
    *(sdword *)(shipBytes + P1_CORVETTE_ACTIVE_EFFECT_OFFSET) = 0;
    *(ubyte *)(shipBytes + P1_CORVETTE_SPECIAL_STATE_OFFSET) = 0;
    SalCapResetHostQueueScriptData(ship);
}

void P1StandardCorvetteAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    P1StandardCorvetteSpec *spec = (P1StandardCorvetteSpec *)ship->ShipSpecifics;
    P1StandardCorvetteStatics *corvstat = (P1StandardCorvetteStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackSideStep(ship,target,&spec->attacksidestep,&corvstat->sidestepParameters);
}

void P1StandardCorvetteAttackPassive(Ship *ship,Ship *target,bool32 rotate)
{
    if ((rotate) & ((bool32)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship,target);
    }
    else
    {
        attackPassive(ship,target);
    }
}

static void P1StandardCorvetteRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    ubyte specialState = *(ubyte *)((ubyte *)ship + P1_CORVETTE_SPECIAL_STATE_OFFSET);

    if (specialState == 1 &&
        ship->command != NULL &&
        (ship->command->ordertype.attributes & COMMAND_IS_FORMATION) != 0 &&
        ship->command->formation.formationtype == SPHERE_FORMATION)
    {
        ship->specialFlags1C8 |= P1_STANDARD_CORVETTE_LINK_ACTIVE_FLAGS;
    }
    else if (specialState == 0)
    {
        if ((ship->staticinfo->rotateToRetaliate & rotateFlags) != 0)
        {
            attackPassive(ship, (Ship *)target);
        }
        else
        {
            attackSimple(ship, target);
        }
    }
}

static bool32 P1StandardCorvetteAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 gunRange)
{
    ubyte specialState = *(ubyte *)((ubyte *)ship + P1_CORVETTE_SPECIAL_STATE_OFFSET);

    if (specialState == 1)
    {
        vector shipHeading;
        vector trajectory;

        attackStraightForward(ship, target, gunRange, 1000.0f);
        aishipGetTrajectory(ship, target, &trajectory);
        matGetVectFromMatrixCol3(shipHeading, ship->rotinfo.coordsys);

        if (vecDotProduct(trajectory, shipHeading) > 0.5f)
        {
            ship->specialFlags1C8 |= P1_STANDARD_CORVETTE_LINK_ACTIVE_FLAGS;
        }
    }
    else
    {
        P1StandardCorvetteAttack(ship, target, gunRange);
    }

    return TRUE;
}

static bool32 P1StandardCorvetteDisableCallback(Ship *ship)
{
    shipModuleDisableCallback(ship);
    return TRUE;
}

static bool32 P1StandardCorvetteSpecialTarget(Ship *ship, void *custom)
{
    ubyte *specialState = (ubyte *)ship + P1_CORVETTE_SPECIAL_STATE_OFFSET;
    sdword result;

    if (*specialState == 0)
    {
        *specialState = 0;
        return TRUE;
    }

    if (*specialState == 1)
    {
        result = shipModuleSlotProcessSpecial(ship, custom);
    }
    else if (*specialState == 2)
    {
        result = SalCapExecuteSpecialOrder(ship, custom);
    }
    else
    {
        *specialState = 0;
        return TRUE;
    }

    if (result)
    {
        *specialState = 0;
    }

    return result;
}

static bool32 P1StandardCorvetteSpecialCleanup(Ship *ship, void *custom)
{
    (void)custom;
    (void)SalCapDropTargetAndRememberPreviousTarget(ship);
    *(ubyte *)((ubyte *)ship + P1_CORVETTE_SPECIAL_STATE_OFFSET) = 0;
    return TRUE;
}

static void P1StandardCorvetteHousekeep(Ship *ship)
{
    shipUpdateModuleSlots(ship);
    (void)SalCapProcessHostQueueScriptData(ship);
}

static void P1StandardCorvetteRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    (void)shipFreeModuleData(shiptoremove, ship);
    (void)SalCapHandleProcessingShipRemoved(ship, shiptoremove);
}

static void P1StandardCorvetteClose(Ship *ship)
{
    shipFreeModuleSlotResources(ship);
    SalCapDropTargetCallback(ship);
}

static void P1StandardCorvettePreFix(Ship *ship)
{
    shipPreFixModuleSlotRegistryReferences(ship);
    (void)PreFix_SalvagerSpecifics(ship);
}

static void P1StandardCorvetteFix(Ship *ship)
{
    shipFixModuleSlotRegistryReferences(ship);
    (void)Fix_SalvagerSpecifics(ship);
}

static bool32 P1StandardCorvetteCanHalt(Ship *ship)
{
    return shipCleanupDormantModuleSlotResources(ship);
}

CustShipHeader P1StandardCorvetteHeader =
{
    sizeof(P1StandardCorvetteSpec),
    P1StandardCorvetteStaticInit,
    NULL,
    P1StandardCorvetteInit,
    NULL,
    shipShootAllGunsAtTarget,
    P1StandardCorvetteRotateAndShoot,
    P1StandardCorvetteDisableCallback,
    P1StandardCorvetteSpecialTarget,
    P1StandardCorvetteSpecialCleanup,
    P1StandardCorvetteHousekeep,
    P1StandardCorvetteRemoveShipReferences,
    P1StandardCorvetteRemoveShipReferences,
    NULL,
    NULL,
    P1StandardCorvetteClose,
    P1StandardCorvettePreFix,
    NULL,
    NULL,
    P1StandardCorvetteFix,
    NULL,
    P1StandardCorvetteAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    P1StandardCorvetteCanHalt
};
