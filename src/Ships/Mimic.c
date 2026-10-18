#include "types.h"
#include "spaceobj.h"
#include "Mimic.h"
#include "gun.h"
#include "universe.h"
#include "CommandLayer.h"
#include "Dreadnought.h"
#include "Ship.h"
#include "DefaultShip.h"
#include <string.h>

static void MimicStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    shipAttackStaticInfoScript(directory, filename, statinfo);
    statinfo->canSpecialBandBoxFriendlies = TRUE;
    statinfo->unk4 = TRUE;
    statinfo->unk5 = TRUE;
}

static void MimicInit(Ship *ship)
{
    MimicSpecifics *specifics = mimicShipSpecifics(ship);

    shipResetAttackState(ship);
    ship->flags2 |= BIT3;
    specifics->linkPartner = NULL;
    specifics->linkState = 0;
    specifics->linkStateUnk20 = 0;
    specifics->linkAnimationIndex = 0;

    specifics->state = MIMIC_STATE_IDLE;
    specifics->morphProgress = 0;
    specifics->activeBindings = 0;
    specifics->activeSlotIndex = -1;
    specifics->activeType = -1;
    specifics->activeRace = -1;
    specifics->activeRadius = -1;
    specifics->pendingBindings = 0;
    specifics->pendingSlotIndex = -1;
    specifics->pendingType = -1;
    specifics->pendingRace = -1;
    specifics->pendingRadius = -1;
    specifics->activeEffect = 0;
    specifics->unk_138 = 0x40000000u;
}

static void MimicAttack(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    const ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntimeConst(ship);

    if (moduleSlots->state == ShipModuleSlotState_Active &&
        ship->command != NULL &&
        (ship->command->ordertype.attributes & COMMAND_IS_FORMATION) != 0 &&
        ship->command->formation.formationtype == SPHERE_FORMATION)
    {
        ship->specialFlags1C8 |= MIMIC_LINK_ACTIVE_FLAGS;
    }
    else if (moduleSlots->state == ShipModuleSlotState_Disabled)
    {
        shipRotateAndShootAtTarget(ship, target, rotateFlags);
    }
}

static bool32 MimicRefreshCommand(Ship *ship)
{
    getShipAndItsCommand(&universe.mainCommandLayer, ship);
    return TRUE;
}

static bool32 MimicAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    shipSharedFlightManAttackCommand(ship, target, maxdist);
    ship->specialFlags1C8 |= MIMIC_LINK_ACTIVE_FLAGS;
    return TRUE;
}

CustShipHeader MimicHeader =
{
    sizeof(MimicSpecifics),
    MimicStaticInit,
    NULL,
    MimicInit,
    NULL,
    shipShootNextGunAtTarget,
    MimicAttack,
    shipModuleDisableCallback,
    shipModuleSlotProcessSpecial,
    NULL,
    shipUpdateModuleSlots,
    shipFreeModuleData,
    shipFreeModuleData,
    NULL,
    NULL,
    shipFreeModuleSlotResources,
    shipPreFixModuleSlotRegistryReferences,
    NULL,
    NULL,
    shipFixModuleSlotRegistryReferences,
    NULL,
    MimicAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    shipCleanupDormantModuleSlotResources,
    NULL,
    (void *)MimicRefreshCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
