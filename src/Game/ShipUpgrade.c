#include "types.h"
#include "debug.h"
#include "fastmath.h"
#include "spaceobj.h"
#include "ACV.h"
#include "BeastMothership.h"
#include "CommandLayer.h"
#include "Globals.h"
#include "objtypes.h"
#include "Randy.h"
#include "ShipSelect.h"
#include "researchapi.h"
#include "soundevent.h"
#include "universe.h"
#include "UnivUpdate.h"
#include "SaveGame.h"
#include "LevelLoad.h"
#include <math.h>
#include <string.h>

static const real32 shipModuleSlotSphereScale = 1.2f;
static const real32 shipModuleSlotMorphStep = 0.05f;
static const real32 shipModuleSlotDelayScale = 9.313225746154785e-08f;
static const real32 shipModuleSlotDelayLog2 = -0.004999999888241291f;
static const real64 shipModuleSlotDelayBase = 1.0;

typedef enum
{
    /* Bit positions are authoritative; legacy HW1-style alias names in researchapi.h are not. */
    SHIPUPGRADE_Bit0 = BIT0,
    SHIPUPGRADE_Bit1 = BIT1,
    SHIPUPGRADE_Bit2 = BIT2,
    SHIPUPGRADE_Bit3 = BIT3,
    SHIPUPGRADE_Bit4 = BIT4,
    SHIPUPGRADE_Bit5 = BIT5,
    SHIPUPGRADE_Bit6 = BIT6,
    SHIPUPGRADE_Bit7 = BIT7,
    SHIPUPGRADE_Bit8 = BIT8,
    SHIPUPGRADE_Bit9 = BIT9,
    SHIPUPGRADE_Bit10 = BIT10,
    SHIPUPGRADE_Bit11 = BIT11,
    SHIPUPGRADE_Bit12 = BIT12,
    SHIPUPGRADE_Bit13 = BIT13,
    SHIPUPGRADE_Bit14 = BIT14,
    SHIPUPGRADE_Bit15 = BIT15,
    SHIPUPGRADE_Bit16 = BIT16,
    SHIPUPGRADE_Bit17 = BIT17,
    SHIPUPGRADE_Bit18 = BIT18,
    SHIPUPGRADE_Bit19 = BIT19,
    SHIPUPGRADE_Bit20 = BIT20,
    SHIPUPGRADE_Bit21 = BIT21,
    SHIPUPGRADE_Bit22 = BIT22,
    SHIPUPGRADE_Bit23 = BIT23,
    SHIPUPGRADE_Bit25 = BIT25,
    SHIPUPGRADE_Bit26 = BIT26,
} ShipUpgradeMask;

enum
{
    PLAYER_MODULE_TECH_ENERGY = 1,
    SHIP_LINK_PARTNER_OFFSET = 0x650,
    SHIP_LINK_TRANSIENT_OFFSET = 0x678,
    SHIP_ATTACK_STATE_OFFSET = 0x64E,
    SHIP_MODULE_SPECIAL_SLOT_OFFSET = 36,
    MODULE_TRANSFORM_TYPE_OFFSET = 408,
    UNIVERSE_MODULE_SLOT_BUILD_RACE_DWORD_OFFSET = 1408,
};

static void shipModuleSlotEffectDelete(Resource *effect)
{
    if (effect != NULL)
    {
        univReallyDeleteThisResourceRightNow(effect);
    }
}

void playerInitUpgradeLevels(Player *player)
{
    sdword i;

    player->shipUpgrades.level = 0;
    for (i = 0; i < (sdword)(sizeof(player->shipUpgrades.moduleTechEnabled) / sizeof(player->shipUpgrades.moduleTechEnabled[0])); i++)
    {
        player->shipUpgrades.moduleTechEnabled[i] = FALSE;
    }
    player->shipUpgrades.moduleTechEnabled[0] = TRUE;
    player->shipUpgrades.mask = 0;
}

static ShipRace shipModuleSlotGetBuildRace(sdword slotIndex)
{
    const ubyte *buildRaceBase =
        (const ubyte *)&universe.world[0] - UNIVERSE_MODULE_SLOT_BUILD_RACE_DWORD_OFFSET * sizeof(udword);
    return *(const ShipRace *)(buildRaceBase + sizeof(Player) * slotIndex);
}

static AsteroidStaticInfo *shipModuleSlotTransformStaticInfo(sdword moduleType)
{
    return &asteroidStaticInfos[moduleType];
}

lodinfo *shipGetLodInfo(SpaceObj *obj)
{
    Ship *ship;
    const ShipModuleSlotRuntime *moduleSlots;

    if (obj->objtype != OBJ_ShipType)
    {
        return obj->staticinfo->staticheader.LOD;
    }

    ship = (Ship *)obj;
    moduleSlots = shipModuleSlotRuntimeConst(ship);
    if ((ship->flags2 & BIT3) != 0 &&
        moduleSlots->state == ShipModuleSlotState_Active)
    {
        return moduleSlots->activeBuildTarget.buildTargetLodInfo;
    }

    return ship->staticinfo->staticheader.LOD;
}

real32 shipGetMinZoomDistance(SpaceObj *obj)
{
    Ship *ship;
    const ShipModuleSlotRuntime *moduleSlots;

    if (obj->objtype != OBJ_ShipType)
    {
        return ((ShipStaticInfo *)obj->staticinfo)->minimumZoomDistance;
    }

    ship = (Ship *)obj;
    moduleSlots = shipModuleSlotRuntimeConst(ship);
    if ((ship->flags2 & BIT3) == 0)
    {
        return ((ShipStaticInfo *)ship->staticinfo)->minimumZoomDistance;
    }

    switch (moduleSlots->state)
    {
        case ShipModuleSlotState_Active:
        case ShipModuleSlotState_Closing:
        case ShipModuleSlotState_Resetting:
        case ShipModuleSlotState_Destroying:
        case ShipModuleSlotState_WaitingToClose:
        case ShipModuleSlotState_WaitingToReset:
            if (moduleSlots->activeBuildTarget.buildTargetType == -1)
            {
                return moduleSlots->activeBuildTarget.transformMinimumZoomDistance;
            }
            return GetShipStaticInfo(moduleSlots->activeBuildTarget.buildTargetType,
                                     moduleSlots->activeBuildTarget.buildTargetRace)
                ->minimumZoomDistance;
        default:
            return ((ShipStaticInfo *)ship->staticinfo)->minimumZoomDistance;
    }
}

real32 shipGetCollSphereSize(SpaceObj *obj)
{
    Ship *ship;
    const ShipModuleSlotRuntime *moduleSlots;

    if (obj->objtype != OBJ_ShipType)
    {
        return obj->staticinfo->staticheader.staticCollInfo.collspheresize;
    }

    ship = (Ship *)obj;
    moduleSlots = shipModuleSlotRuntimeConst(ship);
    if ((ship->flags2 & BIT3) == 0)
    {
        return ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    }

    switch (moduleSlots->state)
    {
        case ShipModuleSlotState_Active:
        case ShipModuleSlotState_Closing:
        case ShipModuleSlotState_Resetting:
        case ShipModuleSlotState_Destroying:
        case ShipModuleSlotState_WaitingToClose:
        case ShipModuleSlotState_WaitingToReset:
            if (moduleSlots->activeBuildTarget.buildTargetType == -1)
            {
                return moduleSlots->activeBuildTarget.transformCollSphereSize;
            }
            return GetShipStaticInfo(moduleSlots->activeBuildTarget.buildTargetType,
                                     moduleSlots->activeBuildTarget.buildTargetRace)
                ->staticheader.staticCollInfo.collspheresize;
        default:
            return ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    }
}

static void shipModuleSlotRestoreBuildTarget(Ship *ship, ShipModuleSlotBuildTarget *buildTarget)
{
    if (buildTarget->buildTargetType == -1)
    {
        AsteroidStaticInfo *transformInfo = shipModuleSlotTransformStaticInfo(buildTarget->transformModuleType);

        buildTarget->buildTargetLodInfo = transformInfo->staticheader.LOD;
        buildTarget->buildTargetBindings = NULL;
    }
    else
    {
        ShipStaticInfo *targetInfo = GetShipStaticInfo(buildTarget->buildTargetType, buildTarget->buildTargetRace);

        buildTarget->buildTargetLodInfo = targetInfo->staticheader.LOD;
        buildTarget->buildTargetBindings = univMeshBindingsStaticDupe(targetInfo);
    }
}

static const sdword shipUpgradeMaxLevels[TOTAL_NUM_SHIPS] =
{
    1, 1, 1, 1, 1, 1, 2, 2, 2, 0, 2, 2, 2, 2, 2, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 2, 2, 2, 2,
    2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

static const udword shipUpgradeMasks[TOTAL_NUM_SHIPS] =
{
    0x00800001, 0x04000009, 0x000000A9, 0x00000040, 0x00000211, 0x000000B1, 0x02600100, 0x00200002,
    0x00200802, 0x00000000, 0x00200404, 0x00000000, 0x00300004, 0x00000000, 0x006C0000, 0x00000000,
    0x00340000, 0x00807000, 0x00300000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00200004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00800001, 0x04000009, 0x00000001, 0x00020001, 0x00000001, 0x00000211, 0x00000001,
    0x00000001, 0x00000002, 0x00000802, 0x00000000, 0x00020002, 0x00008002, 0x00010004, 0x00110004,
    0x02000100, 0x000C0000, 0x00000000, 0x00140000, 0x00807000, 0x00110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00010001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800000,
    0x00000000, 0x00080000, 0x00000000, 0x00000000, 0x00800000, 0x00000000, 0x00020000, 0x00000000,
    0x00800000, 0x00080020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00010000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000
};

typedef struct
{
    TechnologyType tech;
    udword primaryMask;
    udword secondaryMask;
} ShipResearchUpgradeEntry;

static const ShipResearchUpgradeEntry shipResearchUpgradeEntries[] =
{
    { TechADriveFighter,    RESEARCH_UPGRADE_MASK_ADRIVE_FIGHTER,      0 },
    { TechADriveFrigate,    RESEARCH_UPGRADE_MASK_ADRIVE_FRIGATE,      0 },
    { TechADriveDestroyer,  RESEARCH_UPGRADE_MASK_ADRIVE_DESTROYER,    0 },
    { TechLink,             RESEARCH_UPGRADE_MASK_LINK_PRIMARY,        RESEARCH_UPGRADE_MASK_LINK_SECONDARY },
    { TechMimic,            RESEARCH_UPGRADE_MASK_MIMIC,               0 },
    { TechMicroShip,        RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY,   0 },
    { TechExplosiveCharge,  RESEARCH_UPGRADE_MASK_EXPLOSIVE_CHARGE,    0 },
    { TechAfterburner,      RESEARCH_UPGRADE_MASK_AFTERBURNER,         0 },
    { TechEMP,              RESEARCH_UPGRADE_MASK_EMP,                 0 },
    { TechRepulse,          RESEARCH_UPGRADE_MASK_REPULSE,             0 },
    { TechMicroShip,        RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY, 0 },
    { TechForceField1,      RESEARCH_UPGRADE_MASK_FORCE_FIELD_1,       0 },
    { TechForceField2,      RESEARCH_UPGRADE_MASK_FORCE_FIELD_2,       0 },
    { TechForceField3,      RESEARCH_UPGRADE_MASK_FORCE_FIELD_3,       0 },
    { TechDefenseField,     RESEARCH_UPGRADE_MASK_DEFENSE_FIELD,       0 },
    { TechInfect,           RESEARCH_UPGRADE_MASK_INFECT,              0 },
    { TechCloaking,         RESEARCH_UPGRADE_MASK_CLOAKING,            0 },
    { TechRepairBots,       RESEARCH_UPGRADE_MASK_REPAIR_BOTS,         0 },
    { TechSalvage,          RESEARCH_UPGRADE_MASK_SALVAGE,             0 },
    { TechCrystalHarvest,   RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST,     0 },
    { TechRepair,           RESEARCH_UPGRADE_MASK_REPAIR,              0 },
    { TechBeastVaccine,     RESEARCH_UPGRADE_MASK_BEAST_VACCINE,       0 },
    { TechAdvancedSensors,  RESEARCH_UPGRADE_MASK_ADVANCED_SENSORS,    0 },
    { TechIon,              RESEARCH_UPGRADE_MASK_ION,                 0 },
    { TechMissile,          RESEARCH_UPGRADE_MASK_MISSILE,             0 },
};

static const real32 shipUpgradeDurationUnit = 10.0f;
static const real32 shipUpgradeLevelDurationUnits[] =
{
    10.0f, 20.0f, 30.0f, 10.0f, 10.0f
};

static const real32 shipInstalledUpgradeTypeDurationUnits[9] =
{
    10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

sdword shipTypeGetUpgradeMaxLevels(ShipType shipType)
{
    return shipUpgradeMaxLevels[shipType];
}

udword shipTypeGetUpgradeMask(ShipType shipType)
{
    return shipUpgradeMasks[shipType];
}

sdword shipTypeGetMaxUpgradeLevel(ShipType shipType)
{
    return shipUpgradeMaxLevels[shipType];
}

void shipUpdateResearchableUpgradeMask(Ship *ship)
{
    udword availableMask;
    Player *player;
    PlayerShipPrereqMask *prereqMask;
    PlayerShipPrereqMask *sourcePrereqMask;
    sdword i;

    if (ship->shiptype >= TOTAL_STD_SHIPS)
    {
        return;
    }

    availableMask = 0;
    player = ship->playerowner;
    prereqMask = &player->discoveredShipPrereqs[ship->shiptype];
    sourcePrereqMask = prereqMask;

    if (ship->shiprace == RACE_Sect &&
        player->cataPreFogUnknown2.beastTechGroupUnlockedByShipType[ship->shiptype] == 0)
    {
        sourcePrereqMask =
            &player->discoveredShipPrereqs[GetAppropriateShipTypeForRace(ship->shiptype, RACE_Kushan)];
        memcpy(prereqMask, sourcePrereqMask, sizeof(*prereqMask));
    }

    for (i = 0; i < (sdword)(sizeof(shipResearchUpgradeEntries) / sizeof(shipResearchUpgradeEntries[0])); i++)
    {
        const ShipResearchUpgradeEntry *entry = &shipResearchUpgradeEntries[i];

        if (rmTechIsResearched((Player *)sourcePrereqMask, entry->tech))
        {
            availableMask |= entry->primaryMask | entry->secondaryMask;
        }
    }

    ship->staticinfo->builtInUpgradeMask = availableMask & shipTypeGetUpgradeMask(ship->shiptype);
}

bool32 shipCanDisplayUpgradeType(Ship *ship, sdword upgradeType)
{
    switch (upgradeType)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
            return TRUE;

        case 7:
            return mothershipHasReadyBigGun(ship);

        default:
            return FALSE;
    }
}

bool32 shipHasAvailableUpgradeLevel(Ship *ship)
{
    sdword currentLevel;

    if ((ship == NULL) || (ship->playerowner == NULL))
    {
        return FALSE;
    }

    currentLevel = ship->upgradeState.level;
    return (currentLevel < shipUpgradeMaxLevels[ship->shiptype]) &&
           (currentLevel < ship->playerowner->shipUpgrades.level);
}

udword shipGetAvailablePlayerUpgradeMask(Ship *ship)
{
    udword playerMask;

    if ((ship == NULL) || (ship->playerowner == NULL))
    {
        return 0;
    }

    playerMask = ship->playerowner->shipUpgrades.mask & shipTypeGetUpgradeMask(ship->shiptype);
    if (playerMask == ship->upgradeState.mask)
    {
        return 0;
    }

    return playerMask & ~ship->upgradeState.mask;
}

real32 shipUpgradeDurationUnitsGet(udword upgradeMask)
{
    real32 duration = 0.0f;

    while (upgradeMask != 0)
    {
        duration += shipUpgradeDurationUnit;
        upgradeMask &= upgradeMask - 1;
    }

    return duration;
}

real32 shipGetUpgradeBuildTime(Ship *ship)
{
    real32 duration = 0.0f;
    udword typeFlags[9];
    sdword index;
    udword availablePlayerMask;
    udword pendingUpgradeMask;
    sdword playerUpgradeLevel;

    if ((ship == NULL) || (ship->playerowner == NULL))
    {
        return 0.0f;
    }

    playerUpgradeLevel = ship->playerowner->shipUpgrades.level;
    if (ship->upgradeState.level < shipUpgradeMaxLevels[ship->shiptype] &&
        ship->upgradeState.level < playerUpgradeLevel)
    {
        dbgAssert(playerUpgradeLevel >= 0 &&
                  playerUpgradeLevel < (sdword)(sizeof(shipUpgradeLevelDurationUnits) / sizeof(shipUpgradeLevelDurationUnits[0])));
        duration = shipUpgradeLevelDurationUnits[playerUpgradeLevel];
    }

    if (shipCollectInstalledUpgradeTypeFlags(ship, typeFlags))
    {
        for (index = 0; index < 9; index++)
        {
            if (typeFlags[index] != 0)
            {
                duration += shipInstalledUpgradeTypeDurationUnits[index];
            }
        }
    }

    availablePlayerMask = ship->playerowner->shipUpgrades.mask & shipTypeGetUpgradeMask(ship->shiptype);
    pendingUpgradeMask = (availablePlayerMask == ship->upgradeState.mask) ? 0 : (availablePlayerMask & ~ship->upgradeState.mask);
    return (shipUpgradeDurationUnitsGet(pendingUpgradeMask) + duration) * ship->staticinfo->upgradeTimeModifier;
}

udword shipCollectInstalledUpgradeTypeFlags(Ship *ship, udword *typeFlags)
{
    sdword index;
    bool32 found;
    ShipUpgradeSlotStateBlock *slotStateBlock;

    memset(typeFlags, 0, sizeof(udword) * 9);

    if ((ship == NULL) || (ship->playerowner == NULL))
    {
        return FALSE;
    }

    slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    if ((slotStateBlock == NULL) || (slotStateBlock->count <= 0))
    {
        return FALSE;
    }

    found = FALSE;
    for (index = 0; index < slotStateBlock->count; index++)
    {
        ShipUpgradeSlotStateEntry *slotState = &slotStateBlock->entries[index];
        sdword upgradeType = (sdword)slotState->state;

        if ((upgradeType == 0) &&
            (slotState->gunStatic != NULL) &&
            (slotState->gunStatic->endUpgradeLevel == 1) &&
            ship->playerowner->shipUpgrades.moduleTechEnabled[PLAYER_MODULE_TECH_ENERGY] != 0)
        {
            typeFlags[0] = TRUE;
            found = TRUE;
            continue;
        }

        if ((upgradeType < 0) || (upgradeType >= 9) ||
            ship->playerowner->shipUpgrades.moduleTechEnabled[upgradeType] == 0)
        {
            continue;
        }

        switch (upgradeType)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 8:
                break;

            case 7:
                (void)mothershipHasReadyBigGun(ship);
                break;

            default:
                typeFlags[upgradeType] = TRUE;
                found = TRUE;
                break;
        }
    }

    return found;
}

bool32 shipTypeSupportsModuleSlots(ShipType shipType)
{
    return (shipTypeGetUpgradeMask(shipType) & SHIPUPGRADE_Bit5) != 0;
}

bool32 ShipHasUpgrade(Ship *ship, udword upgradeMask)
{
    return (upgradeMask & ship->upgradeState.mask) != 0;
}

static sdword shipUpgradeSlotMapDirection(sdword slotType, sdword direction)
{
    switch (slotType)
    {
        case 1:
            switch (direction)
            {
                case 2: return 0;
                case 1: return 4;
                case 0: return 10;
                case 3: return 14;
                default: return direction;
            }
        case 2:
            switch (direction)
            {
                case 2: return 1;
                case 1: return 5;
                case 0: return 11;
                case 3: return 15;
                default: return direction;
            }
        case 3:
            switch (direction)
            {
                case 2: return 2;
                case 1: return 6;
                case 0: return 12;
                case 3: return 16;
                default: return direction;
            }
        case 0:
            switch (direction)
            {
                case 2: return 3;
                case 1: return 7;
                case 0: return 13;
                default: return direction;
            }
        case 6:
            return 8;
        case 5:
            return 9;
        case 4:
            return 17;
        case 7:
            return 18;
        case 8:
            return 19;
        default:
            return direction;
    }
}

static void shipUpgradeSlotReset(ShipUpgradeSlotStateEntry *slotState)
{
    udword *slotWords = (udword *)slotState;

    slotWords[1] = shipUpgradeSlotMapDirection((sdword)slotWords[3], (sdword)slotWords[5]);
    slotWords[2] = shipUpgradeSlotMapDirection((sdword)slotWords[4], (sdword)slotWords[5]);
}

void shipRestoreUpgradeSlots(Ship *ship)
{
    sdword i;
    ShipUpgradeSlotStateBlock *slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    GunStaticInfo *gunStaticInfo = ship->staticinfo->gunStaticInfo;

    if (slotStateBlock == NULL)
    {
        return;
    }

    dbgAssert(gunStaticInfo != NULL);
    dbgAssert(slotStateBlock->count >= 0);
    dbgAssert(slotStateBlock->count <= gunStaticInfo->numGuns);

    for (i = 0; i < slotStateBlock->count; i++)
    {
        slotStateBlock->entries[i].gunStatic = &gunStaticInfo->gunstatics[i];
    }
}

void shipInitUpgradeSlots(Ship *ship)
{
    sdword i;
    ShipUpgradeSlotStateBlock *slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;

    ship->upgradeState.level = shipUpgradeMaxLevels[ship->shiptype];
    ship->upgradeState.mask = 0;

    shipRestoreUpgradeSlots(ship);
    slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    if (slotStateBlock == NULL)
    {
        return;
    }

    for (i = 0; i < slotStateBlock->count; i++)
    {
        ShipUpgradeSlotStateEntry *slotState = &slotStateBlock->entries[i];
        udword *slotWords = (udword *)slotState;
        udword *slotStatic = (udword *)slotState->gunStatic;

        slotWords[0] = slotStatic[3];
        shipUpgradeSlotReset(slotState);
    }

}

void shipApplyPlayerUpgrades(Ship *ship)
{
    sdword i;
    sdword upgradeLevel;
    struct Player *player;
    ShipUpgradeSlotStateBlock *slotStateBlock;

    player = ship->playerowner;
    upgradeLevel = player->shipUpgrades.level;
    if (upgradeLevel > shipUpgradeMaxLevels[ship->shiptype])
    {
        upgradeLevel = shipUpgradeMaxLevels[ship->shiptype];
    }

    ship->upgradeState.level = upgradeLevel;
    ship->upgradeState.mask =
        ship->staticinfo->builtInUpgradeMask |
        (player->shipUpgrades.mask & shipTypeGetUpgradeMask(ship->shiptype));


    if (ship->soundevent.timeLastStatus != 0.0f)
    {
        soundEventShipRemove(ship);
    }

    slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    if (slotStateBlock != 0)
    {
        for (i = 0; i < slotStateBlock->count; i++)
        {
            ShipUpgradeSlotStateEntry *slotState = &slotStateBlock->entries[i];
            if (slotState->state == 0 && *((sdword *)slotState->gunStatic + 4) == 1)
            {
                if (player->shipUpgrades.moduleTechEnabled[PLAYER_MODULE_TECH_ENERGY] != 0)
                {
                    slotState->state = 1;
                }
            }
        }
    }
}

bool32 shipHasAvailableUpgradeChanges(Ship *ship)
{
    sdword currentLevel;
    udword currentMask;
    udword playerMask;
    udword typeFlags[9];

    if (ship == NULL || ship->playerowner == NULL)
    {
        return FALSE;
    }
    if (ship->upgradeActive)
    {
        return FALSE;
    }

    currentLevel = ship->upgradeState.level;
    if (currentLevel < shipUpgradeMaxLevels[ship->shiptype] &&
        currentLevel < ship->playerowner->shipUpgrades.level)
    {
        return TRUE;
    }

    currentMask = ship->upgradeState.mask;
    playerMask = ship->playerowner->shipUpgrades.mask & shipTypeGetUpgradeMask(ship->shiptype);
    if (playerMask != currentMask && (playerMask & ~currentMask) != 0)
    {
        return TRUE;
    }

    return shipCollectInstalledUpgradeTypeFlags(ship, typeFlags) != 0;
}

udword shipModuleSlotResetState(Ship *ship)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);
    udword state = moduleSlots->state;
    udword rnd;
    real64 t;
    real64 delay;

    if (ship->shiptype == bMothership)
    {
        mothershipSetSystemType(ship, BS_None);
    }

    switch (state)
    {
        case ShipModuleSlotState_Disabled:
        case ShipModuleSlotState_Destroyed:
            moduleSlots->morphProgress = 1.0f;
            rnd = ranRandom(RAN_ETG);
            t = (real64)rnd * shipModuleSlotDelayScale * shipModuleSlotDelayLog2 * 1.442695040888963387;
            delay = shipModuleSlotDelayBase - pow(2.0, t - floor(t)) + universe.totaltimeelapsed;
            moduleSlots->transitionTime = (real32)delay;
            moduleSlots->state = ShipModuleSlotState_WaitingToOpen;
            break;
        case ShipModuleSlotState_Active:
            moduleSlots->morphProgress = 0.0f;
            rnd = ranRandom(RAN_ETG);
            t = (real64)rnd * shipModuleSlotDelayScale * shipModuleSlotDelayLog2 * 1.442695040888963387;
            delay = shipModuleSlotDelayBase - pow(2.0, t - floor(t)) + universe.totaltimeelapsed;
            moduleSlots->transitionTime = (real32)delay;
            moduleSlots->state = ShipModuleSlotState_WaitingToReset;
            break;
        case ShipModuleSlotState_Opening:
        case ShipModuleSlotState_Closing:
            moduleSlots->state = ShipModuleSlotState_Resetting;
            break;
        case ShipModuleSlotState_WaitingToClose:
            moduleSlots->state = ShipModuleSlotState_WaitingToReset;
            break;
        default:
            break;
    }

    return state;
}

sdword shipDisable(Ship *ship, sdword allowDelay)
{
    udword i;
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);
    udword state = moduleSlots->state;

    switch (state)
    {
        case ShipModuleSlotState_Active:
            moduleSlots->morphProgress = 0.0f;
            if (allowDelay)
            {
                udword rnd = ranRandom(RAN_ETG);
                real64 t = (real64)rnd * shipModuleSlotDelayScale * shipModuleSlotDelayLog2 * 1.442695040888963387;
                real64 delay = shipModuleSlotDelayBase - pow(2.0, t - floor(t)) + universe.totaltimeelapsed;
                moduleSlots->transitionTime = (real32)delay;
                moduleSlots->state = ShipModuleSlotState_WaitingToClose;
                break;
            }
            moduleSlots->state = ShipModuleSlotState_Closing;
            break;
        case ShipModuleSlotState_Opening:
        case ShipModuleSlotState_Resetting:
        case ShipModuleSlotState_Destroying:
            moduleSlots->state = ShipModuleSlotState_Closing;
            break;
        case ShipModuleSlotState_Destroyed:
        case ShipModuleSlotState_WaitingToOpen:
            moduleSlots->state = ShipModuleSlotState_Disabled;
            if (moduleSlots->activeBuildTarget.buildTargetBindings != NULL)
            {
                memFree(moduleSlots->activeBuildTarget.buildTargetBindings);
                moduleSlots->activeBuildTarget.buildTargetBindings = NULL;
            }
            if (moduleSlots->pendingBuildTarget.buildTargetBindings != NULL)
            {
                memFree(moduleSlots->pendingBuildTarget.buildTargetBindings);
                moduleSlots->pendingBuildTarget.buildTargetBindings = NULL;
            }
            if (moduleSlots->activeEffect != NULL)
            {
                shipModuleSlotEffectDelete(moduleSlots->activeEffect);
                moduleSlots->activeEffect = NULL;
            }
            break;
        case ShipModuleSlotState_WaitingToReset:
            moduleSlots->state = ShipModuleSlotState_WaitingToClose;
            break;
        default:
            break;
    }

    for (i = 0; i < 8; i++)
    {
        moduleSlots->slotAvailability[i] = 1;
    }
    return 1;
}

static sdword shipGetAvailableColorScheme(ShipStaticInfo *staticInfo, sdword preferredSlot)
{
    sdword result;

    if (singlePlayerGame)
    {
        switch (GetValidRaceForShipType(staticInfo->shiptype))
        {
            case 0: result = 0; goto check;
            case 1: result = 1; goto check;
            case 2: result = 4; goto check;
            case 3: result = 2; goto check;
            case 4:
            case 5:
            case 6: result = 5; goto check;
check:
                if (staticInfo->teamColor[result] == 0)
                {
                    goto scan;
                }
                return result;
            default:
scan:
                result = 0;
                break;
        }

        while (staticInfo->teamColor[result] == 0)
        {
            if (++result >= 8)
            {
                return 0;
            }
        }
    }
    else
    {
        result = preferredSlot;
        if (staticInfo->teamColor[preferredSlot] == 0)
        {
            result = 0;
            while (staticInfo->teamColor[result] == 0)
            {
                if (++result >= 8)
                {
                    return 0;
                }
            }
        }
    }

    return result;
}

static sdword shipModuleSlotSetBuildTarget(Ship *ship, ShipType targetType, real32 slotIndex)
{
    ShipModuleSlotRuntime *moduleSlots;
    ShipModuleSlotBuildTarget *buildTarget;
    ShipStaticInfo *targetInfo;
    ShipStaticInfo *myInfo;
    real32 sphere;
    ShipType result;

    moduleSlots = shipModuleSlotRuntime(ship);
    targetInfo = GetShipStaticInfo(targetType, GetValidRaceForShipType(targetType));
    result = ship->shiptype;

    if (result != p1StandardCorvette)
    {
        switch (targetInfo->shipclass)
        {
            case CLASS_Mothership:
            case CLASS_HeavyCruiser:
            case CLASS_Carrier:
            case CLASS_Destroyer:
            case CLASS_MonsterShip:
            case CLASS_Component:
                return result;
            case CLASS_Frigate:
                if (result == sMimic)
                {
                    return result;
                }
                break;
            case CLASS_Fighter:
            case CLASS_Micro:
                if (result == sMCV)
                {
                    result = ShipHasUpgrade(ship, SHIPUPGRADE_Bit4);
                    if (!result)
                    {
                        return result;
                    }
                    moduleSlots->state = (moduleSlots->state != ShipModuleSlotState_Disabled) + 6;
                }
                break;
            case CLASS_Resource:
                if (result == sMimic || targetType == sProcessor || targetType == bProcessor)
                {
                    return result;
                }
                break;
            case CLASS_NonCombat:
                if (result == sMimic && targetType != dCrate)
                {
                    return result;
                }
                break;
            default:
                break;
        }
    }

    if (moduleSlots->state == ShipModuleSlotState_Disabled)
    {
        buildTarget = &moduleSlots->activeBuildTarget;
    }
    else
    {
        buildTarget = &moduleSlots->pendingBuildTarget;
    }

    buildTarget->buildTargetType = targetType;
    buildTarget->buildTargetRace = shipModuleSlotGetBuildRace((sdword)slotIndex);
    buildTarget->buildTargetLodInfo = targetInfo->staticheader.LOD;
    myInfo = ship->staticinfo;
    sphere = (myInfo->staticheader.staticCollInfo.collspheresize >= targetInfo->staticheader.staticCollInfo.collspheresize)
        ? myInfo->staticheader.staticCollInfo.collspheresize
        : targetInfo->staticheader.staticCollInfo.collspheresize;
    if (buildTarget->buildTargetBindings != NULL)
    {
        memFree(buildTarget->buildTargetBindings);
    }
    buildTarget->buildTargetBindings = univMeshBindingsStaticDupe(targetInfo);
    buildTarget->colorScheme = shipGetAvailableColorScheme(targetInfo, (sdword)slotIndex);
    buildTarget->displaySphereRadius = sphere * shipModuleSlotSphereScale;
    buildTarget->slotIndex = (sdword)slotIndex;
    buildTarget->transformModuleType = -1;

    return shipModuleSlotResetState(ship);
}

static void shipModuleSlotSetupTransform(Ship *ship, real32 transformType)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);
    sdword moduleType = 2;
    AsteroidStaticInfo *transformInfo;
    ShipModuleSlotBuildTarget *buildTarget;
    real32 halfX;
    real32 halfY;
    real32 halfZ;
    real32 boundingSphere;
    real32 sphere;

    if (transformType == 0.0f)
    {
        if (ship->shiptype == sMimic)
        {
            moduleType = ((ranRandom(RAN_ETG) & 1) != 0) ? 2 : 1;
        }
        else
        {
            moduleType = ((ranRandom(RAN_ETG) & 1) == 0) ? 2 : 3;
        }
    }
    else
    {
        switch ((sdword)transformType)
        {
            case 1:
                moduleType = 1;
                break;
            case 2:
                moduleType = 2;
                break;
            case 3:
                moduleType = 3;
                break;
            default:
                moduleType = (sdword)ship;
                break;
        }
    }

    switch (moduleType)
    {
        case 1:
            if (ship->shiptype == sMCV)
            {
                if (!ShipHasUpgrade(ship, SHIPUPGRADE_Bit4))
                {
                    return;
                }
                moduleSlots->state = (moduleSlots->state != 0) + 6;
            }
            transformType = 130.0f;
            break;
        case 2:
            transformType = 500.0f;
            break;
        case 3:
            if (ship->shiptype == sMimic)
            {
                return;
            }
            transformType = 800.0f;
            break;
        default:
            break;
    }

    transformInfo = shipModuleSlotTransformStaticInfo(moduleType);
    halfX = transformInfo->staticheader.staticCollInfo.uplength * 0.5f;
    halfY = transformInfo->staticheader.staticCollInfo.rightlength * 0.5f;
    halfZ = transformInfo->staticheader.staticCollInfo.forwardlength * 0.5f;
    boundingSphere = (real32)fmathSqrtDouble(halfZ * halfZ + halfY * halfY + halfX * halfX) * shipModuleSlotSphereScale;

    buildTarget = (moduleSlots->state != 0) ? &moduleSlots->pendingBuildTarget : &moduleSlots->activeBuildTarget;
    buildTarget->buildTargetType = -1;
    buildTarget->buildTargetRace = -1;
    buildTarget->buildTargetLodInfo = transformInfo->staticheader.LOD;
    buildTarget->colorScheme = 0;
    sphere = (ship->staticinfo->staticheader.staticCollInfo.collspheresize >= boundingSphere)
        ? ship->staticinfo->staticheader.staticCollInfo.collspheresize
        : boundingSphere;
    buildTarget->displaySphereRadius = sphere * shipModuleSlotSphereScale;
    buildTarget->slotIndex = -1;
    buildTarget->transformModuleType = moduleType;
    buildTarget->transformCollSphereSize = boundingSphere;
    buildTarget->transformMinimumZoomDistance = transformType;
    buildTarget->displayCount = 10 * transformInfo->resourcevalue;
    if (moduleSlots->state != 0 && buildTarget->buildTargetBindings != NULL)
    {
        memFree(buildTarget->buildTargetBindings);
    }
    buildTarget->buildTargetBindings = NULL;

    shipModuleSlotResetState(ship);
}

void shipProcessModuleSlotOrder(Ship *ship, sdword orderType, ShipType targetType, real32 param3, real32 param4)
{
    if (ShipHasUpgrade(ship, SHIPUPGRADE_Bit5) &&
        (ship->shiptype != bMothership || mothershipGetSystemType(ship) == BS_None))
    {
        if (orderType == 0)
        {
            shipDisable(ship, 1);
        }
        else if (orderType == 1)
        {
            shipModuleSlotSetBuildTarget(ship, targetType, param3);
        }
        else if (orderType == 2)
        {
            shipModuleSlotSetupTransform(ship, param4);
        }
    }
}

bool32 shipModuleDisableCallback(Ship *ship)
{
    shipDisable(ship, TRUE);
    return TRUE;
}

bool32 shipModuleSlotProcessSpecial(Ship *ship, void *command)
{
    SpaceObjRotImpTarg *sourceTarget;
    Ship *sourceShip;
    sdword moduleType;

    if (!ShipHasUpgrade(ship, SHIPUPGRADE_Bit5) ||
        (ship->shiptype == bMothership && mothershipGetSystemType(ship) != BS_None))
    {
        return TRUE;
    }

    sourceTarget = ((SpecialCommand *)command)->TargetPtr[0];
    if (sourceTarget->objtype == OBJ_ShipType)
    {
        sourceShip = (Ship *)sourceTarget;
        shipModuleSlotSetBuildTarget(
            ship,
            sourceShip->shiptype,
            (real32)*(uword *)((ubyte *)sourceShip->staticinfo + SHIP_MODULE_SPECIAL_SLOT_OFFSET));
        return TRUE;
    }

    if (sourceTarget->objtype != OBJ_AsteroidType)
    {
        return TRUE;
    }

    switch (*(sdword *)((ubyte *)sourceTarget + MODULE_TRANSFORM_TYPE_OFFSET) - 1)
    {
        case 0:
            moduleType = 1;
            break;
        case 2:
            moduleType = 3;
            break;
        default:
            moduleType = 2;
            break;
    }

    if ((ship->shiptype == sMimic && moduleType == 3) ||
        (ship->shiptype == sMCV && moduleType == 1 && !ShipHasUpgrade(ship, SHIPUPGRADE_Bit4)))
    {
        moduleType = 2;
    }

    shipModuleSlotSetupTransform(ship, (real32)moduleType);
    return TRUE;
}

void shipFreeModuleSlotResources(Ship *ship)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);

    if (moduleSlots->activeBuildTarget.buildTargetBindings != NULL)
    {
        memFree(moduleSlots->activeBuildTarget.buildTargetBindings);
        moduleSlots->activeBuildTarget.buildTargetBindings = NULL;
    }

    if (moduleSlots->pendingBuildTarget.buildTargetBindings != NULL)
    {
        memFree(moduleSlots->pendingBuildTarget.buildTargetBindings);
        moduleSlots->pendingBuildTarget.buildTargetBindings = NULL;
    }

    if (moduleSlots->activeEffect != NULL)
    {
        shipModuleSlotEffectDelete((Resource *)moduleSlots->activeEffect);
        moduleSlots->activeEffect = NULL;
    }
}

Ship *shipFreeModuleData(Ship *data, Ship *ship)
{
    if (data == ship)
    {
        shipFreeModuleSlotResources(ship);
    }
    return data;
}

void shipPreFixModuleSlotRegistryReferences(Ship *ship)
{
    *(sdword *)((ubyte *)ship + SHIP_LINK_PARTNER_OFFSET) =
        SpaceObjRegistryGetID(*(SpaceObj **)((ubyte *)ship + SHIP_LINK_PARTNER_OFFSET));
    *(sdword *)&shipModuleSlotRuntime(ship)->activeEffect =
        SpaceObjRegistryGetID((SpaceObj *)shipModuleSlotRuntime(ship)->activeEffect);
    *(sdword *)((ubyte *)ship + SHIP_LINK_TRANSIENT_OFFSET) = 0;
}

void shipFixModuleSlotRegistryReferences(Ship *ship)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);

    *(Ship **)((ubyte *)ship + SHIP_LINK_PARTNER_OFFSET) =
        SpaceObjRegistryGetShip((sdword)*(Ship **)((ubyte *)ship + SHIP_LINK_PARTNER_OFFSET));

    switch (moduleSlots->state)
    {
        case 1:
        case 2:
        case 3:
        case 5:
        case 7:
        case 9:
        case 10:
        case 11:
            shipModuleSlotRestoreBuildTarget(ship, &moduleSlots->activeBuildTarget);
            break;
        default:
            break;
    }

    switch (moduleSlots->state)
    {
        case 4:
        case 6:
        case 7:
        case 8:
        case 11:
            shipModuleSlotRestoreBuildTarget(ship, &moduleSlots->pendingBuildTarget);
            break;
        default:
            break;
    }

    moduleSlots->activeEffect =
        (struct Resource *)SpaceObjRegistryGetShip((sdword)moduleSlots->activeEffect);
}

bool32 shipCleanupDormantModuleSlotResources(Ship *ship)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);

    if (moduleSlots->state == 8)
    {
        moduleSlots->state = 0;
        shipFreeModuleSlotResources(ship);
    }
    return TRUE;
}

void shipResetModuleSlots(Ship *ship)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);
    udword i;

    if ((ship->specialFlags2 & BIT27) != 0)
    {
        return;
    }

    moduleSlots->state = ShipModuleSlotState_Disabled;
    moduleSlots->transitionArmed = FALSE;

    if (moduleSlots->activeBuildTarget.buildTargetBindings != NULL)
    {
        memFree(moduleSlots->activeBuildTarget.buildTargetBindings);
        moduleSlots->activeBuildTarget.buildTargetBindings = NULL;
    }

    if (moduleSlots->pendingBuildTarget.buildTargetBindings != NULL)
    {
        memFree(moduleSlots->pendingBuildTarget.buildTargetBindings);
        moduleSlots->pendingBuildTarget.buildTargetBindings = NULL;
    }

    if (moduleSlots->activeEffect != NULL)
    {
        shipModuleSlotEffectDelete(moduleSlots->activeEffect);
        moduleSlots->activeEffect = NULL;
    }

    for (i = 0; i < 8; i++)
    {
        moduleSlots->slotAvailability[i] = 1;
    }
}

void shipUpdateModuleSlots(Ship *ship)
{
    ShipModuleSlotRuntime *moduleSlots = shipModuleSlotRuntime(ship);

    switch (moduleSlots->state)
    {
        case ShipModuleSlotState_Opening:
        {
            real32 progress = moduleSlots->morphProgress - shipModuleSlotMorphStep;
            moduleSlots->morphProgress = progress;
            if (progress <= 0.0f)
            {
                sdword slotIndex = moduleSlots->activeBuildTarget.slotIndex;

                moduleSlots->state = ShipModuleSlotState_Active;
                if (slotIndex < 0 || moduleSlots->activeBuildTarget.buildTargetType > 0x3B)
                {
                    memset(moduleSlots->slotAvailability, 0, sizeof(moduleSlots->slotAvailability));
                }
                else
                {
                    moduleSlots->slotAvailability[slotIndex] = 0;
                }
            }
            break;
        }

        case ShipModuleSlotState_Closing:
        {
            real32 progress;

            if (moduleSlots->morphProgress <= 0.0f)
            {
                soundEventPlay(ship, 2072, 0);
                speechEventQueue(ship, 67111078, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);
            }

            progress = moduleSlots->morphProgress + shipModuleSlotMorphStep;
            moduleSlots->morphProgress = progress;
            if (progress >= 1.0f)
            {
                moduleSlots->state = ShipModuleSlotState_Disabled;
                moduleSlots->transitionArmed = FALSE;
                if (moduleSlots->activeBuildTarget.buildTargetBindings != NULL)
                {
                    memFree(moduleSlots->activeBuildTarget.buildTargetBindings);
                    moduleSlots->activeBuildTarget.buildTargetBindings = NULL;
                }
                if (moduleSlots->pendingBuildTarget.buildTargetBindings != NULL)
                {
                    memFree(moduleSlots->pendingBuildTarget.buildTargetBindings);
                    moduleSlots->pendingBuildTarget.buildTargetBindings = NULL;
                }
                goto cleanup_effect;
            }
            break;
        }

        case ShipModuleSlotState_Resetting:
        {
            real32 progress;

            if (moduleSlots->morphProgress <= 0.0f)
            {
                soundEventPlay(ship, 2072, 0);
            }

            progress = moduleSlots->morphProgress + shipModuleSlotMorphStep;
            moduleSlots->morphProgress = progress;
            if (progress >= 1.0f)
            {
                if (moduleSlots->activeBuildTarget.buildTargetBindings != NULL)
                {
                    memFree(moduleSlots->activeBuildTarget.buildTargetBindings);
                }
                moduleSlots->activeBuildTarget = moduleSlots->pendingBuildTarget;
                moduleSlots->pendingBuildTarget.buildTargetBindings = NULL;
                moduleSlots->state = ShipModuleSlotState_Opening;
            }
            break;
        }

        case ShipModuleSlotState_WaitingToOpen:
            if (universe.totaltimeelapsed > moduleSlots->transitionTime)
            {
                moduleSlots->state = ShipModuleSlotState_Opening;
                moduleSlots->transitionArmed = TRUE;
            }
            break;

        case ShipModuleSlotState_WaitingToClose:
            if (universe.totaltimeelapsed > moduleSlots->transitionTime)
            {
                moduleSlots->state = ShipModuleSlotState_Closing;
            }
            break;

        case ShipModuleSlotState_WaitingToReset:
            if (universe.totaltimeelapsed > moduleSlots->transitionTime)
            {
                moduleSlots->state = ShipModuleSlotState_Resetting;
            }
            break;

        default:
            break;
    }

cleanup_effect:
    if (moduleSlots->state != ShipModuleSlotState_Active &&
        moduleSlots->activeEffect != NULL)
    {
        shipModuleSlotEffectDelete(moduleSlots->activeEffect);
        moduleSlots->activeEffect = NULL;
    }
}
