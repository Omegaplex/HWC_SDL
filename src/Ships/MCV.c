#include "types.h"
#include "spaceobj.h"
#include "gun.h"
#include "attack.h"
#include "statscript.h"
#include "universe.h"
#include "CommandLayer.h"
#include "Ship.h"
#include "DefaultShip.h"
#include <string.h>

static AttackSideStepParameters mcvAttackSideStepParameters;

#define MCV_LINK_ACTIVE_FLAGS 0x6u
#define MCV_ATTACK_SIDE_STEP_OFFSET 0x40
#define MCV_RUNTIME_ZERO_OFFSET 0x684
#define MCV_RUNTIME_ZERO_SIZE 0x60
#define MCV_ACTIVE_TYPE_OFFSET 1796
#define MCV_ACTIVE_RACE_OFFSET 1800
#define MCV_ACTIVE_BINDINGS_OFFSET 1808
#define MCV_ACTIVE_SLOT_INDEX_OFFSET 1820
#define MCV_ACTIVE_RADIUS_OFFSET 1824
#define MCV_PENDING_TYPE_OFFSET 1840
#define MCV_PENDING_RACE_OFFSET 1844
#define MCV_PENDING_BINDINGS_OFFSET 1852
#define MCV_PENDING_SLOT_INDEX_OFFSET 1864
#define MCV_PENDING_RADIUS_OFFSET 1868
#define MCV_MORPH_PROGRESS_OFFSET 1884
#define MCV_STATE_OFFSET 1888
#define MCV_ACTIVE_EFFECT_OFFSET 1900
#define MCV_UNKNOWN_1904_OFFSET 1904
#define MCV_STATE_DISABLED 0
#define MCV_STATE_ACTIVE 1

// Decompiled from cataclysm.exe @ 0x5B80F0
static void MCVStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    statinfo->canSpecialBandBoxFriendlies = TRUE;
    statinfo->unk4 = TRUE;
    statinfo->unk5 = TRUE;
    statinfo->custstatinfo = &mcvAttackSideStepParameters;
    scriptSetStruct(directory, filename, AttackSideStepParametersScriptTable, (ubyte *)&mcvAttackSideStepParameters);
}

// Decompiled from cataclysm.exe @ 0x5B8130
static void MCVInit(Ship *ship)
{
    ubyte *shipBytes = (ubyte *)ship;

    memset(shipBytes + MCV_RUNTIME_ZERO_OFFSET, 0, MCV_RUNTIME_ZERO_SIZE);
    ship->flags2 |= BIT3;

    *(sdword *)(shipBytes + MCV_ACTIVE_TYPE_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_ACTIVE_RACE_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_ACTIVE_BINDINGS_OFFSET) = 0;
    *(sdword *)(shipBytes + MCV_ACTIVE_SLOT_INDEX_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_ACTIVE_RADIUS_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_PENDING_TYPE_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_PENDING_RACE_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_PENDING_BINDINGS_OFFSET) = 0;
    *(sdword *)(shipBytes + MCV_PENDING_SLOT_INDEX_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_PENDING_RADIUS_OFFSET) = -1;
    *(sdword *)(shipBytes + MCV_MORPH_PROGRESS_OFFSET) = 0;
    *(ubyte *)(shipBytes + MCV_STATE_OFFSET) = 0;
    *(sdword *)(shipBytes + MCV_ACTIVE_EFFECT_OFFSET) = 0;
    *(udword *)(shipBytes + MCV_UNKNOWN_1904_OFFSET) = 0x40000000u;
    attackSideStepInit((AttackSideStep *)(ship->ShipSpecifics + MCV_ATTACK_SIDE_STEP_OFFSET));
}

// Decompiled from cataclysm.exe @ 0x5B81D0
static bool32 MCVAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    attackStraightForward(ship, target, maxdist, 1000.0f);
    ship->specialFlags1C8 |= MCV_LINK_ACTIVE_FLAGS;
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B8210
static void MCVAttack(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    ubyte state = *(ubyte *)((ubyte *)ship + MCV_STATE_OFFSET);

    if (state == MCV_STATE_ACTIVE &&
        ship->command != NULL &&
        (ship->command->ordertype.attributes & COMMAND_IS_FORMATION) != 0 &&
        ship->command->formation.formationtype == SPHERE_FORMATION)
    {
        ship->specialFlags1C8 |= MCV_LINK_ACTIVE_FLAGS;
    }
    else if (state == MCV_STATE_DISABLED)
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

// Decompiled from cataclysm.exe @ 0x5BE7A0
static bool32 MCVDisableCallback(Ship *ship)
{
    shipModuleDisableCallback(ship);
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B82A0
static bool32 MCVProcessSpecial(Ship *ship, void *command)
{
    shipModuleSlotProcessSpecial(ship, command);
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B82C0
static void MCVHousekeep(Ship *ship)
{
    shipUpdateModuleSlots(ship);
}

// Decompiled from cataclysm.exe @ 0x5B8310
static void MCVRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    (void)shipFreeModuleData(shiptoremove, ship);
}

// Decompiled from cataclysm.exe @ 0x5B82D0
static void MCVClose(Ship *ship)
{
    shipFreeModuleSlotResources(ship);
}

// Decompiled from cataclysm.exe @ 0x5B82F0
static void MCVPreFix(Ship *ship)
{
    shipPreFixModuleSlotRegistryReferences(ship);
}

// Decompiled from cataclysm.exe @ 0x5B8300
static void MCVFix(Ship *ship)
{
    shipFixModuleSlotRegistryReferences(ship);
}

// Decompiled from cataclysm.exe @ 0x5B82E0
static bool32 MCVCanHalt(Ship *ship)
{
    return shipCleanupDormantModuleSlotResources(ship);
}

CustShipHeader MCVHeader =
{
    sizeof(MCVSpecifics),
    MCVStaticInit,
    NULL,
    MCVInit,
    NULL,
    shipShootNextGunAtTarget,
    MCVAttack,
    MCVDisableCallback,
    MCVProcessSpecial,
    NULL,
    MCVHousekeep,
    MCVRemoveShipReferences,
    MCVRemoveShipReferences,
    NULL,
    NULL,
    MCVClose,
    MCVPreFix,
    NULL,
    NULL,
    MCVFix,
    NULL,
    MCVAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MCVCanHalt
};
