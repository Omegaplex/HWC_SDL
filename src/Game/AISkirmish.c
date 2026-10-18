/*=============================================================================
    Name    : AISkirmish
    Purpose : Cataclysm skirmish / multiplayer AI recovered from 0x42FD50..0x433D50

    The original HW1 AIFleetMan logic is primarily story-game AI. Cataclysm moved
    its skirmish / multiplayer fleet logic into a separate TU; this file stages
    that recovered block and keeps its remaining IDA references together.
=============================================================================*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "Alliance.h"
#include "Bounties.h"
#include "BigGun.h"
#include "CommandDefs.h"
#include "CommandWrap.h"
#include "CommandLayer.h"
#include "MaxMultiPlayer.h"
#include "MissionScriptSupport.h"
#include "MultiplayerGame.h"
#include "AISkirmish.h"
#include "researchapi.h"
#include "SalCapCorvette.h"
#include "ShipDefs.h"
#include "select.h"
#include "universe.h"
#include "vector.h"

AISTeamEntry aisTeams[34] = { 0 };
real32 aiLastUpdateTimes[MAX_MULTIPLAYER_PLAYERS] = { 0.0f };
sdword aiUpdateCount = 0;

#define AIS_TEAM_MERGE_DISTANCE 5000.0f
#define AIS_TEAM_SCOUT 0
#define AIS_TEAM_SCOUT_ENEMY 1
#define AIS_TEAM_FIGHTER 2
#define AIS_TEAM_FIGHTER_ENEMY 3
#define AIS_TEAM_MULTIBEAM 4
#define AIS_TEAM_MULTIBEAM_ENEMY 5
#define AIS_TEAM_SENTINEL 6
#define AIS_TEAM_SENTINEL_ENEMY 7
#define AIS_TEAM_MEDIUM_COMBAT 8
#define AIS_TEAM_MEDIUM_COMBAT_ENEMY 9
#define AIS_TEAM_EXTERNAL_BUILD 10
#define AIS_TEAM_MOTHERSHIP_MODULES 11
#define AIS_TEAM_CARRIER1_SUPPORT 12
#define AIS_TEAM_CARRIER2_SUPPORT 13
#define AIS_TEAM_MOTHERSHIP_SUPPORT 14
#define AIS_TEAM_MOTHERSHIP_BIGGUN 15
#define AIS_TEAM_MODULE_ENEMY 16
#define AIS_TEAM_CARRIER 17
#define AIS_TEAM_CARRIER_ENEMY 18
#define AIS_TEAM_MOTHERSHIP 19
#define AIS_TEAM_MOTHERSHIP_ENEMY 20
#define AIS_TEAM_PROCESSOR 21
#define AIS_TEAM_PROCESSOR_ENEMY 22
#define AIS_TEAM_MIMIC 23
#define AIS_TEAM_MIMIC_ENEMY_UNSLOTTED 24
#define AIS_TEAM_MIMIC_ENEMY_SLOTTED 25
#define AIS_TEAM_CRUISE_MISSILE 26
#define AIS_TEAM_LEECH 27
#define AIS_TEAM_LEECH_ENEMY 28
#define AIS_TEAM_DFG_FRIGATE 29
#define AIS_TEAM_DFG_FRIGATE_ENEMY 30
#define AIS_TEAM_WORKER 31
#define AIS_TEAM_WORKER_ENEMY 32
#define AIS_TEAM_CRYSTAL 33
#define AIS_PROCESSOR_MOVE_DISTANCE 4000.0f
#define AIS_PROCESSOR_MOVE_OFFSET 500.0f
#define AIS_SUPPORT_ROLE_DISTANCE 5000.0f
#define AIS_WEAKER_TARGET_RATIO_DELTA -0.4f
#define AIS_DFG_MOTHERSHIP_DISTANCE 3500.0f
#define AIS_WEAK_ENEMY_BOUNTY_RATIO 5.0f
#define AIS_MOTHERSHIP_SIEGE_DISTANCE 90000.0f
#define AIS_UNIVERSE_EDGE_MARGIN 8000.0f
#define AIS_MANAGE_TEAM_HYPERSPACE_TARGET_DISTANCE 10000.0f
#define AIS_MANAGE_TEAM_HYPERSPACE_OFFSET 500.0f
#define AIS_MANAGE_TEAM_HYPERSPACE_Z_OFFSET 100.0f
#define AIS_ATTACK_SCOUT_SHADOW_DISTANCE 20000.0f
#define AIS_ATTACK_SCOUT_SHADOW_OFFSET 16000.0f
#define AIS_FLEET_UPDATE_BEGINNER_INTERVAL 20.0f
#define AIS_FLEET_UPDATE_INTERMEDIATE_INTERVAL 10.0f
#define AIS_FLEET_UPDATE_ADVANCED_INTERVAL 5.0f
#define AIS_FLEET_UPDATE_BEGINNER_CHANCE 20.0f
#define AIS_FLEET_UPDATE_INTERMEDIATE_CHANCE 50.0f
#define AIS_FLEET_UPDATE_ADVANCED_CHANCE 100.0f
#define AIS_FLEET_DFG_HYPERSPACE_OFFSET 300.0f
#define AIS_FLEET_DFG_HYPERSPACE_MIN_DISTANCE 2000.0f
#define AIS_BEAST_RESEARCH_SWITCH_RATIO 0.7f

static bool32 aiDFGFrigateByMothership = FALSE;
static bool32 aiHasExternalConstruction = FALSE;
static bool32 aiHasBuildQueued = FALSE;
static bool32 aiHasSupportModuleQueued = FALSE;
static bool32 aiHasCarrier1ModuleQueued = FALSE;
static bool32 aiHasCarrier2ModuleQueued = FALSE;
static sdword aiSupportUnitsPending = 0;
static sdword aiBuildingShip[TOTAL_NUM_SHIPS] = { 0 };
static Ship *aiCarrier1 = NULL;
static Ship *aiCarrier2 = NULL;
static Ship *aiOwnedSiegeMothership = NULL;
static Ship *aiEnemySiegeMothership = NULL;
static Ship *aiEnemyBuildingSiegeMothership = NULL;
static Ship *aiWeakestOwnedShip = NULL;
static Ship *aiCurrentCrystalTarget = NULL;
static bool32 aisFleetCommandStartup = TRUE;

static sdword aisFighterTargetTeams[] =
{
    AIS_TEAM_MULTIBEAM_ENEMY,
    AIS_TEAM_MIMIC_ENEMY_SLOTTED,
    AIS_TEAM_MEDIUM_COMBAT_ENEMY,
    AIS_TEAM_WORKER_ENEMY,
    AIS_TEAM_SENTINEL_ENEMY,
    AIS_TEAM_FIGHTER_ENEMY,
    AIS_TEAM_CARRIER_ENEMY,
    AIS_TEAM_MOTHERSHIP_ENEMY
};

static sdword aisMultibeamTargetTeams[] =
{
    AIS_TEAM_MIMIC_ENEMY_SLOTTED,
    AIS_TEAM_FIGHTER_ENEMY,
    AIS_TEAM_SENTINEL_ENEMY,
    AIS_TEAM_MEDIUM_COMBAT_ENEMY,
    AIS_TEAM_CARRIER_ENEMY,
    AIS_TEAM_MOTHERSHIP_ENEMY
};

static sdword aisMediumCombatTargetTeams[] =
{
    AIS_TEAM_CARRIER_ENEMY,
    AIS_TEAM_MEDIUM_COMBAT_ENEMY,
    AIS_TEAM_SENTINEL_ENEMY,
    AIS_TEAM_MOTHERSHIP_ENEMY
};

static sdword aisMimicTargetTeams[] =
{
    AIS_TEAM_MEDIUM_COMBAT_ENEMY,
    AIS_TEAM_WORKER_ENEMY,
    AIS_TEAM_CARRIER_ENEMY,
    AIS_TEAM_MOTHERSHIP_ENEMY
};

static sdword aisCruiseMissileTargetTeams[] =
{
    AIS_TEAM_MULTIBEAM_ENEMY,
    AIS_TEAM_MEDIUM_COMBAT_ENEMY,
    AIS_TEAM_WORKER_ENEMY,
    AIS_TEAM_CARRIER_ENEMY,
    AIS_TEAM_MOTHERSHIP_ENEMY
};

static sdword aisDockingBayTechs[] = { 8, 10, 12, 13, 24, 9, 11 };
static sdword aisArmourTechs[] = { 14, 15, 29, 30, 31 };
static sdword aisMicroTechs[] = { 16, 7, 25, 6, 26 };
static sdword aisSpecialTechs[] = { 28, 34, 17, 33, 36 };
static sdword aisWeaponsTechs[] = { 18, 19, 20, 27, 23, 21, 22 };
static sdword aisBeastTechs[] = { 8, 16, 12, 28 };

#ifndef CATACLYSM_AISKIRMISH_PARITY_LOG && defined HW_BUILD_FOR_DEBUGGING
#define CATACLYSM_AISKIRMISH_PARITY_LOG 0
#endif

#if CATACLYSM_AISKIRMISH_PARITY_LOG
static FILE *aisParityLogFile = NULL;
static bool32 aisParityLogHeaderWritten = FALSE;

static udword aisParityCurrentLogFrame(void)
{
    if (universe.univUpdateCounter == 0)
    {
        return 0;
    }

    return universe.univUpdateCounter - 1;
}

static void aisParityBuildLogPath(char *buffer, size_t bufferSize)
{
    time_t rawTime;
    struct tm localTime;

    rawTime = time(NULL);
    if (rawTime == (time_t)-1)
    {
        snprintf(buffer, bufferSize, "ai_log.txt");
        return;
    }

    if (localtime_s(&localTime, &rawTime) != 0)
    {
        snprintf(buffer, bufferSize, "ai_log.txt");
        return;
    }

    if (strftime(buffer, bufferSize, "ai_log_%Y%m%d_%H%M%S.txt", &localTime) == 0)
    {
        snprintf(buffer, bufferSize, "ai_log.txt");
    }
}

static void aisParityLogEnsureOpen(void)
{
    char logPath[64];

    if (aisParityLogFile != NULL)
    {
        return;
    }

    aisParityBuildLogPath(logPath, sizeof(logPath));
    aisParityLogFile = fopen(logPath, "wt");
    if (aisParityLogFile == NULL)
    {
        return;
    }

    if (!aisParityLogHeaderWritten)
    {
        fprintf(aisParityLogFile, "# AI Log v1\n");
        fprintf(aisParityLogFile, "# Format: [frame=N] channel: key=value ...\n");
        fprintf(aisParityLogFile, "#\n");
        fflush(aisParityLogFile);
        aisParityLogHeaderWritten = TRUE;
    }
}

static void aisParityVLogChannel(const char *channel, const char *format, va_list args)
{
    aisParityLogEnsureOpen();
    if (aisParityLogFile == NULL)
    {
        return;
    }

    fprintf(
        aisParityLogFile,
        "[frame=%lu] %s: ",
        (unsigned long)aisParityCurrentLogFrame(),
        channel != NULL ? channel : "ai");
    vfprintf(aisParityLogFile, format, args);
    fputc('\n', aisParityLogFile);
    fflush(aisParityLogFile);
}

void aisParityLogChannel(const char *channel, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    aisParityVLogChannel(channel, format, args);
    va_end(args);
}

static void aisParityLog(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    aisParityVLogChannel("ai", format, args);
    va_end(args);
}

void aisParityLogAIRandomState(const char *source, udword x, udword y, udword z, udword c, udword n)
{
    aisParityLog(
        "event=random_state source=%s x=%u y=%u z=%u c=%u n=%u",
        source != NULL ? source : "unknown",
        (unsigned int)x,
        (unsigned int)y,
        (unsigned int)z,
        (unsigned int)c,
        (unsigned int)n);
}

void aisParityLogAISeedOverride(const char *source, udword seed, udword x, udword y, udword z, udword c, udword n)
{
    aisParityLog(
        "event=seed_override source=%s seed=%u x=%u y=%u z=%u c=%u n=%u",
        source != NULL ? source : "unknown",
        (unsigned int)seed,
        (unsigned int)x,
        (unsigned int)y,
        (unsigned int)z,
        (unsigned int)c,
        (unsigned int)n);
}

static void aisParityFormatShipSummary(char *buffer, size_t bufferSize, Ship *ship)
{
    if (ship == NULL)
    {
        snprintf(buffer, bufferSize, "type=-1 id=65535 owner=65535");
        return;
    }

    snprintf(
        buffer,
        bufferSize,
        "type=%d id=%u owner=%u",
        (int)ship->shiptype,
        (unsigned int)ship->shipID.shipNumber,
        ship->playerowner != NULL ? (unsigned int)ship->playerowner->playerIndex : (unsigned int)UWORD_Max);
}

static void aisParityFormatTargetSummary(char *buffer, size_t bufferSize, SpaceObjRotImpTarg *target)
{
    if (target == NULL)
    {
        snprintf(buffer, bufferSize, "targetObj=-1 targetType=-1 targetId=65535");
        return;
    }

    if (target->objtype == OBJ_ShipType || target->objtype == OBJ_DerelictType)
    {
        Ship *ship;

        ship = (Ship *)target;
        snprintf(
            buffer,
            bufferSize,
            "targetObj=%d targetType=%d targetId=%u",
            (int)target->objtype,
            (int)ship->shiptype,
            (unsigned int)ship->shipID.shipNumber);
        return;
    }

    snprintf(
        buffer,
        bufferSize,
        "targetObj=%d targetType=-1 targetId=65535",
        (int)target->objtype);
}

static void aisParityLogOrder(const char *kind, SelectCommand *selection, const char *detail)
{
    char shipSummary[96];
    Ship *leadShip;

    leadShip = (selection != NULL && selection->numShips > 0) ? selection->ShipPtr[0] : NULL;
    aisParityFormatShipSummary(shipSummary, sizeof(shipSummary), leadShip);
    aisParityLog(
        "event=order kind=%s ships=%d %s%s%s",
        kind,
        selection != NULL ? selection->numShips : 0,
        shipSummary,
        (detail != NULL && detail[0] != '\0') ? " " : "",
        detail != NULL ? detail : "");
}

static void aisParityLogTeams(void)
{
    char selections[512];
    char sizes[512];
    size_t selectionOffset;
    size_t sizeOffset;
    sdword index;

    selections[0] = '\0';
    sizes[0] = '\0';
    selectionOffset = 0;
    sizeOffset = 0;

    for (index = 0; index < 34; index++)
    {
        int written;

        written = snprintf(
            selections + selectionOffset,
            sizeof(selections) - selectionOffset,
            "%s%d:%u",
            index == 0 ? "" : ",",
            (int)index,
            (unsigned int)aisTeams[index].numselections);
        if (written > 0)
        {
            selectionOffset += (size_t)written;
            if (selectionOffset >= sizeof(selections))
            {
                selectionOffset = sizeof(selections) - 1;
            }
        }

        written = snprintf(
            sizes + sizeOffset,
            sizeof(sizes) - sizeOffset,
            "%s%d:%d",
            index == 0 ? "" : ",",
            (int)index,
            aisTeams[index].totalteamsize);
        if (written > 0)
        {
            sizeOffset += (size_t)written;
            if (sizeOffset >= sizeof(sizes))
            {
                sizeOffset = sizeof(sizes) - 1;
            }
        }
    }

    aisParityLog(
        "event=teams player=%u selections=%s sizes=%s",
        aiCurrentAIPlayer != NULL && aiCurrentAIPlayer->player != NULL
            ? (unsigned int)aiCurrentAIPlayer->player->playerIndex
            : (unsigned int)UWORD_Max,
        selections,
        sizes);

    {
        static bool32 loggedStartupEnemyTeams = FALSE;

        if (!loggedStartupEnemyTeams &&
            aiCurrentAIPlayer != NULL &&
            aiCurrentAIPlayer->player != NULL &&
            aiCurrentAIPlayer->player->playerIndex == 1)
        {
            sdword teamIndices[] =
            {
                AIS_TEAM_SCOUT_ENEMY,
                AIS_TEAM_MODULE_ENEMY,
                AIS_TEAM_MOTHERSHIP_ENEMY,
                AIS_TEAM_WORKER_ENEMY
            };
            udword teamListIndex;

            for (teamListIndex = 0; teamListIndex < sizeof(teamIndices) / sizeof(teamIndices[0]); teamListIndex++)
            {
                sdword teamIndex = teamIndices[teamListIndex];
                char members[512];
                size_t offset = 0;
                udword selectionIndex;

                members[0] = '\0';
                for (selectionIndex = 0; selectionIndex < aisTeams[teamIndex].numselections; selectionIndex++)
                {
                    MaxSelection *selection = &aisTeams[teamIndex].selection[selectionIndex].sel;
                    sdword shipIndex;

                    for (shipIndex = 0; shipIndex < selection->numShips; shipIndex++)
                    {
                        Ship *ship = selection->ShipPtr[shipIndex];
                        int written;

                        written = snprintf(
                            members + offset,
                            sizeof(members) - offset,
                            "%s%d:%u:%u",
                            offset == 0 ? "" : ",",
                            ship != NULL ? (int)ship->shiptype : -1,
                            ship != NULL ? (unsigned int)ship->shipID.shipNumber : (unsigned int)UWORD_Max,
                            ship != NULL && ship->playerowner != NULL ? (unsigned int)ship->playerowner->playerIndex : (unsigned int)UWORD_Max);
                        if (written > 0)
                        {
                            offset += (size_t)written;
                            if (offset >= sizeof(members))
                            {
                                offset = sizeof(members) - 1;
                                break;
                            }
                        }
                    }
                    if (offset >= sizeof(members) - 1)
                    {
                        break;
                    }
                }

                aisParityLog(
                    "event=team_members player=%u team=%d count=%d members=%s",
                    (unsigned int)aiCurrentAIPlayer->player->playerIndex,
                    (int)teamIndex,
                    aisTeams[teamIndex].totalteamsize,
                    members[0] != '\0' ? members : "-");
            }

            loggedStartupEnemyTeams = TRUE;
        }
    }
}
#else
void aisParityLogChannel(const char *channel, const char *format, ...)
{
    (void)channel;
    (void)format;
}

void aisParityLogAIRandomState(const char *source, udword x, udword y, udword z, udword c, udword n)
{
    (void)source;
    (void)x;
    (void)y;
    (void)z;
    (void)c;
    (void)n;
}

void aisParityLogAISeedOverride(const char *source, udword seed, udword x, udword y, udword z, udword c, udword n)
{
    (void)source;
    (void)seed;
    (void)x;
    (void)y;
    (void)z;
    (void)c;
    (void)n;
}

#define aisParityLog(...) ((void)0)
#define aisParityLogOrder(...) ((void)0)
#define aisParityLogTeams() ((void)0)
#define aisParityFormatShipSummary(...) ((void)0)
#define aisParityFormatTargetSummary(...) ((void)0)
#endif

void selSelectionAddSingleShipNoCheck(MaxSelection *selection, Ship *ship);

static bool32 aisShipIsCrystal(Ship *ship)
{
    return (ship->shiptype == dCrystal0 || ship->shiptype == dCrystal1);
}

static void aisAddShipToSingleSelectionTeam(AISTeamEntry *team, Ship *ship)
{
    real32 averageSize;

    if (team->numselections == 0)
    {
        team->numselections = 1;
    }
    selSelectionAddSingleShipNoCheck(&team->selection[0].sel, ship);
    team->selection[0].center = selCentrePointComputeGeneral(&team->selection[0].sel, &averageSize);
    team->totalteamsize++;
}

static void aisConsiderWeakestOwnedShip(Ship *ship, real32 *lowestHealthRatio)
{
    real32 healthRatio;

    if (ship->staticinfo->maxhealth <= 0.0f)
    {
        return;
    }

    healthRatio = ship->health / ship->staticinfo->maxhealth;
    if (healthRatio > *lowestHealthRatio)
    {
        return;
    }

    if (aiWeakestOwnedShip == NULL
        || aiWeakestOwnedShip->staticinfo->maxhealth <= ship->staticinfo->maxhealth
        || healthRatio - *lowestHealthRatio <= AIS_WEAKER_TARGET_RATIO_DELTA)
    {
        aiWeakestOwnedShip = ship;
        *lowestHealthRatio = healthRatio;
    }
}

static bool32 aisPlayerCountsAsEnemy(Player *player, sdword richestEnemyIndex, real32 richestEnemyWorth)
{
    real32 playerWorth;

    if (allianceArePlayersAllied(aiCurrentAIPlayer->player, player))
    {
        return FALSE;
    }

    if (player->playerIndex == richestEnemyIndex)
    {
        return TRUE;
    }

    playerWorth = getPlayerBountyWorth(player->playerIndex, player->aiPlayer != NULL);
    if (playerWorth <= 0.0f)
    {
        return FALSE;
    }

    return (richestEnemyWorth / playerWorth) <= AIS_WEAK_ENEMY_BOUNTY_RATIO;
}

static bool32 aisPlayerHasCrystalUpgrade(Player *player)
{
    Node *node;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship;

        ship = node->structptr;
        if (ship->playerowner == player && ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static bool32 aisManageTeamSelectionIsAvailable(AISTeamSelection *selection, sdword minimum)
{
    CommandToDo *command;

    if (selection->sel.numShips <= 0)
    {
        return FALSE;
    }

    command = selection->sel.ShipPtr[0]->command;
    if (command == NULL)
    {
        return TRUE;
    }

    if ((command->ordertype.attributes & COMMAND_IS_PROTECTING) != 0)
    {
        return FALSE;
    }

    if (command->ordertype.order != COMMAND_NULL
        && command->ordertype.order != COMMAND_MILITARYPARADE)
    {
        return FALSE;
    }

    if (command->ordertype.order == COMMAND_MILITARYPARADE
        && selection->sel.numShips < minimum)
    {
        return FALSE;
    }

    return TRUE;
}

static sdword aisCountTeamShipType(AISTeamEntry *teamEntry, ShipType shipType)
{
    sdword count;
    sdword selectionIndex;

    count = 0;
    for (selectionIndex = 0; selectionIndex < (sdword)teamEntry->numselections; selectionIndex++)
    {
        AISTeamSelection *selection;
        sdword shipIndex;

        selection = &teamEntry->selection[selectionIndex];
        for (shipIndex = 0; shipIndex < selection->sel.numShips; shipIndex++)
        {
            if (selection->sel.ShipPtr[shipIndex]->shiptype == shipType)
            {
                count++;
            }
        }
    }

    return count;
}

static bool32 aisTeamHasShipType(AISTeamEntry *teamEntry, ShipType shipType)
{
    return (aisCountTeamShipType(teamEntry, shipType) > 0);
}

static void aisAppendTeamSelection(MaxSelection *selection, AISTeamEntry *teamEntry)
{
    sdword selectionIndex;

    for (selectionIndex = 0; selectionIndex < (sdword)teamEntry->numselections; selectionIndex++)
    {
        AISTeamSelection *teamSelection;
        sdword shipIndex;

        teamSelection = &teamEntry->selection[selectionIndex];
        for (shipIndex = 0; shipIndex < teamSelection->sel.numShips; shipIndex++)
        {
            selSelectionAddSingleShipNoCheck(selection, teamSelection->sel.ShipPtr[shipIndex]);
        }
    }
}

static sdword aisDifficultyTarget(AIPlayerLevel difficulty, sdword beginner, sdword intermediate, sdword advanced)
{
    switch (difficulty)
    {
        case AI_INT:
            return intermediate;
        case AI_ADV:
            return advanced;
        default:
            return beginner;
    }
}

static sdword aisTryResearchFirstAvailableTech(Player *player, sdword *techList, sdword techCount)
{
    sdword index;

    for (index = 0; index < techCount; index++)
    {
        if (rmResearchTechForShip(player, (ShipType)techList[index]))
        {
            aisParityLog(
                "event=research player=%u tech=%d",
                player != NULL ? (unsigned int)player->playerIndex : (unsigned int)UWORD_Max,
                (int)techList[index]);
            return techList[index];
        }
    }

    return -1;
}

static void aisResearchModuleTechs(Player *player, Ship *moduleShip)
{
    sdword *techList;
    sdword techCount;
    sdword techToResearch;

    techList = NULL;
    techCount = 0;

    switch (moduleShip->shiptype)
    {
        case sMothershipSpecial:
            techList = aisSpecialTechs;
            techCount = sizeof(aisSpecialTechs) / sizeof(aisSpecialTechs[0]);
            break;
        case sMothershipArmour:
            techList = aisArmourTechs;
            techCount = sizeof(aisArmourTechs) / sizeof(aisArmourTechs[0]);
            break;
        case sMothershipMicro:
            techList = aisMicroTechs;
            techCount = sizeof(aisMicroTechs) / sizeof(aisMicroTechs[0]);
            break;
        case sMothershipWeapons:
            techList = aisWeaponsTechs;
            techCount = sizeof(aisWeaponsTechs) / sizeof(aisWeaponsTechs[0]);
            break;
        case sMothershipDockingBay:
            techList = aisDockingBayTechs;
            techCount = sizeof(aisDockingBayTechs) / sizeof(aisDockingBayTechs[0]);
            break;
        default:
            break;
    }

    if (techList == NULL)
    {
        return;
    }

    techToResearch = aisTryResearchFirstAvailableTech(player, techList, techCount);
    (void)techToResearch;
}

sdword cat_AICountShipClass(AISTeamEntry *team, Ship *ship)
{
    sdword selectionIndex;
    sdword selectedIndex;
    bool32 matchedCommand;
    real32 closestDistance;

    selectedIndex = -1;
    matchedCommand = FALSE;
    closestDistance = 1.0e21f;

    for (selectionIndex = 0; selectionIndex < (sdword)team->numselections; selectionIndex++)
    {
        AISTeamSelection *selection;

        selection = &team->selection[selectionIndex];
        if (selection->sel.numShips > 0
            && ship->command != NULL
            && ship->command == selection->sel.ShipPtr[0]->command)
        {
            matchedCommand = TRUE;
            selectedIndex = selectionIndex;
            continue;
        }

        if (!matchedCommand)
        {
            vector diff;
            real32 distance;

            vecSub(diff, selection->center, ship->posinfo.position);
            distance = getVectDistSloppy(diff);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                selectedIndex = selectionIndex;
            }
        }
    }

    if (selectedIndex == -1 || (!matchedCommand && closestDistance >= AIS_TEAM_MERGE_DISTANCE))
    {
        if (team->numselections < 10)
        {
            selectedIndex = (sdword)team->numselections;
            team->numselections++;
        }
        else
        {
            selectedIndex = 9;
        }
    }

    selSelectionAddSingleShip(&team->selection[selectedIndex].sel, ship);
    {
        real32 averageSize;

        team->selection[selectedIndex].center =
            selCentrePointComputeGeneral(&team->selection[selectedIndex].sel, &averageSize);
    }
    team->totalteamsize++;

    return team->totalteamsize;
}

Ship *aisGetTeamShipByFlatIndex(sdword flatIndex, AISTeamEntry *teamEntry)
{
    sdword selectionIndex;
    sdword shipsSeen;

    if (flatIndex < 0 || flatIndex >= teamEntry->totalteamsize)
    {
        return NULL;
    }

    shipsSeen = 0;
    for (selectionIndex = 0; selectionIndex < (sdword)teamEntry->numselections; selectionIndex++)
    {
        AISTeamSelection *selection;
        sdword shipIndex;

        selection = &teamEntry->selection[selectionIndex];
        for (shipIndex = 0; shipIndex < selection->sel.numShips; shipIndex++, shipsSeen++)
        {
            if (shipsSeen == flatIndex)
            {
                return selection->sel.ShipPtr[shipIndex];
            }
        }
    }

    return NULL;
}

AISTeamSelection *aisFindClosestTeamSelectionToPoint(sdword teamType, vector *point)
{
    AISTeamEntry *teamEntry;
    AISTeamSelection *closestSelection;
    real32 closestDistance;
    sdword selectionIndex;

    teamEntry = &aisTeams[teamType];
    closestSelection = NULL;
    closestDistance = 1.0e21f;

    for (selectionIndex = 0; selectionIndex < (sdword)teamEntry->numselections; selectionIndex++)
    {
        vector diff;
        real32 distance;

        vecSub(diff, teamEntry->selection[selectionIndex].center, *point);
        distance = getVectDistSloppy(diff);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closestSelection = &teamEntry->selection[selectionIndex];
        }
    }

    return closestSelection;
}

bool32 aisShipHasActiveCommand(Ship *ship)
{
    CommandToDo *command;

    command = ship->command;
    if (command == NULL)
    {
        return FALSE;
    }

    if ((command->ordertype.order == COMMAND_NULL
         || command->ordertype.order == COMMAND_MILITARY_PARADE)
        && !(command->ordertype.attributes & COMMAND_IS_PROTECTING))
    {
        return FALSE;
    }

    return TRUE;
}

void aisInitUpdateTimes(void)
{
    memset(aiLastUpdateTimes, 0, sizeof(aiLastUpdateTimes));
    aiUpdateCount = 0;
}

/*
    Remaining functions in this TU, per IDA locality:
        0x42fa80 cat_AICountShipClass
        0x42fd50 aisSetupTeams
        0x4310a0 aisAllianceUpdate
        0x431290 aisBuildModules
        0x431510 aisBuildShip
        0x431750 aisFleetUpdate
        0x4324d0 aisCollectResources
        0x432630 aisMoveProcessor
        0x432760 aisAttackUpdate
        0x432df0 aisManageTeam
        0x433310 aisMoveMothership
        0x433620 aisMoveHarvesterGroups
        0x433760 aisSentinelUpdate
        0x4338e0 aisLeechUpdate
        0x433d30 aisInitUpdateTimes
        0x433d50 aisFleetCommand
*/

void aisSetupTeams(void)
{
    Node *shipNode;
    sdword playerIndex;
    sdword richestEnemyIndex;
    real32 richestEnemyWorth;
    real32 weakestOwnedHealthRatio;

    memset(aisTeams, 0, sizeof(aisTeams));
    memset(aiBuildingShip, 0, sizeof(aiBuildingShip));

    aiCurrentCrystalTarget = NULL;
    aiCarrier1 = NULL;
    aiCarrier2 = NULL;
    aiOwnedSiegeMothership = NULL;
    aiEnemySiegeMothership = NULL;
    aiEnemyBuildingSiegeMothership = NULL;
    aiWeakestOwnedShip = NULL;
    aiHasExternalConstruction = FALSE;
    aiHasBuildQueued = FALSE;
    aiHasSupportModuleQueued = FALSE;
    aiHasCarrier1ModuleQueued = FALSE;
    aiHasCarrier2ModuleQueued = FALSE;
    aiDFGFrigateByMothership = FALSE;
    aiSupportUnitsPending = 0;
    weakestOwnedHealthRatio = 1.0f;

    richestEnemyIndex = -1;
    richestEnemyWorth = 0.0f;
    for (playerIndex = 0; playerIndex < MAX_MULTIPLAYER_PLAYERS; playerIndex++)
    {
        Player *player;
        real32 playerWorth;

        player = &universe.players[playerIndex];
        if (player->playerState == PLAYER_ALIVE
            && !allianceArePlayersAllied(aiCurrentAIPlayer->player, player))
        {
            playerWorth = getPlayerBountyWorth(playerIndex, player->aiPlayer != NULL);
            if (playerWorth > richestEnemyWorth)
            {
                richestEnemyWorth = playerWorth;
                richestEnemyIndex = playerIndex;
            }
        }
    }

    for (shipNode = universe.ShipList.head; shipNode != NULL; shipNode = shipNode->next)
    {
        Ship *ship;
        Player *shipPlayer;
        bool32 isOwned;
        bool32 isEnemy;
        bool32 isQueuedForExternalBuild;
        Ship *moduleHost;

        ship = (Ship *)listGetStructOfNode(shipNode);
        if (bitTest(ship->flags, SOF_Dead))
        {
            continue;
        }

        shipPlayer = ship->playerowner;
        isOwned = FALSE;
        isEnemy = FALSE;
        if (shipPlayer->playerIndex < MAX_MULTIPLAYER_PLAYERS)
        {
            isOwned = (shipPlayer == aiCurrentAIPlayer->player);
            isEnemy = (!isOwned && aisPlayerCountsAsEnemy(shipPlayer, richestEnemyIndex, richestEnemyWorth));
        }
        isQueuedForExternalBuild = bitTest(ship->specialFlags2, SPECIAL_2_DockFlag);
        moduleHost = ship->specialTargetShip;

        switch (ship->shiptype)
        {
            case sRecon:
            case bRecon:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_SCOUT], ship);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_SCOUT_ENEMY], ship);
                }
                break;

            case sAcolyte:
            case sACV:
            case bAcolyte:
            case bCloakedFighter:
            case bAttackBomber:
            case bACV:
            case bInterceptor:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_FIGHTER], ship);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_FIGHTER_ENEMY], ship);
                }
                break;

            case sMultiBeamFrigate:
            case bMultiGunCorvette:
            case bMultiBeamFrigate:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MULTIBEAM], ship);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MULTIBEAM_ENEMY], ship);
                }
                break;

            case sSentinel:
            case bSentinel:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_SENTINEL], ship);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_SENTINEL_ENEMY], ship);
                }
                break;

            case sMimic:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MIMIC], ship);
                }
                else if (isEnemy)
                {
                    if (shipGetSlotStatus(ship, aiCurrentAIPlayer->player->playerIndex))
                    {
                        cat_AICountShipClass(&aisTeams[AIS_TEAM_MIMIC_ENEMY_SLOTTED], ship);
                    }
                    else
                    {
                        cat_AICountShipClass(&aisTeams[AIS_TEAM_MIMIC_ENEMY_UNSLOTTED], ship);
                    }
                }
                break;

            case sLeech:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_LEECH], ship);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_LEECH_ENEMY], ship);
                }
                break;

            case sWorker:
            case bWorker:
                if (isOwned)
                {
                    SalCapCorvetteSpec *salvagerSpec;

                    cat_AICountShipClass(&aisTeams[AIS_TEAM_WORKER], ship);
                    aisConsiderWeakestOwnedShip(ship, &weakestOwnedHealthRatio);
                    salvagerSpec = NULL;
                    if (ship->command != NULL && ship->command->ordertype.order == COMMAND_SPECIAL)
                    {
                        salvagerSpec = (SalCapCorvetteSpec *)SalvagerFunc(ship);
                    }
                    if (salvagerSpec != NULL
                        && salvagerSpec->target != NULL
                        && aisShipIsCrystal((Ship *)salvagerSpec->target))
                    {
                        aiCurrentCrystalTarget = (Ship *)salvagerSpec->target;
                    }
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_WORKER_ENEMY], ship);
                }
                break;

            case sCarrier:
            case bCarrier:
                if (isOwned)
                {
                    if (isQueuedForExternalBuild)
                    {
                        aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], ship);
                        aiHasExternalConstruction = TRUE;
                    }
                    else
                    {
                        cat_AICountShipClass(&aisTeams[AIS_TEAM_CARRIER], ship);
                        if (ship->staticinfo->canBuildShips)
                        {
                            if (aiCarrier1 == NULL)
                            {
                                aiCarrier1 = ship;
                            }
                            else if (aiCarrier1 != ship && aiCarrier2 == NULL)
                            {
                                aiCarrier2 = ship;
                            }
                        }
                    }
                    aisConsiderWeakestOwnedShip(ship, &weakestOwnedHealthRatio);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_CARRIER_ENEMY], ship);
                }
                break;

            case sProcessor:
            case bProcessor:
                if (isOwned)
                {
                    if (isQueuedForExternalBuild)
                    {
                        aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], ship);
                        aiHasExternalConstruction = TRUE;
                    }
                    else
                    {
                        cat_AICountShipClass(&aisTeams[AIS_TEAM_PROCESSOR], ship);
                    }
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_PROCESSOR_ENEMY], ship);
                }
                break;

            case sMothership:
            case bMothership:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MOTHERSHIP], ship);
                    if (ship->shiptype == sMothership && mothershipCanFireBigGun(ship))
                    {
                        aiOwnedSiegeMothership = ship;
                    }
                    aisConsiderWeakestOwnedShip(ship, &weakestOwnedHealthRatio);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MOTHERSHIP_ENEMY], ship);
                    if (ship->shiptype == sMothership)
                    {
                        if (mothershipCanFireBigGun(ship))
                        {
                            aiEnemySiegeMothership = ship;
                        }
                        else if (mothershipBigGunAlmostComplete(ship))
                        {
                            aiEnemyBuildingSiegeMothership = ship;
                        }
                    }
                }
                break;

            case sRammingFrigate:
            case sHiveFrigate:
            case sDreadnought:
            case sDestroyer:
            case bHeavyCorvette:
            case bCruiseMissile:
            case bHeavyCruiser:
            case bHiveFrigate:
            case bIonArrayFrigate:
            case bRammingFrigate:
            case bMissileCorvette:
                if (isOwned)
                {
                    if (ship->shiptype == bCruiseMissile)
                    {
                        cat_AICountShipClass(&aisTeams[AIS_TEAM_CRUISE_MISSILE], ship);
                    }
                    else if (isQueuedForExternalBuild)
                    {
                        aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], ship);
                        aiHasExternalConstruction = TRUE;
                    }
                    else
                    {
                        cat_AICountShipClass(&aisTeams[AIS_TEAM_MEDIUM_COMBAT], ship);
                    }
                    switch (ship->shiptype)
                    {
                        case sRammingFrigate:
                        case sHiveFrigate:
                        case sDreadnought:
                        case sDestroyer:
                        case bHiveFrigate:
                        case bIonArrayFrigate:
                        case bHeavyCruiser:
                        case bRammingFrigate:
                            aisConsiderWeakestOwnedShip(ship, &weakestOwnedHealthRatio);
                            break;

                        default:
                            break;
                    }
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MEDIUM_COMBAT_ENEMY], ship);
                }
                break;

            case sMothershipSpecial:
            case sMothershipEngineering:
            case sMothershipArmour:
            case sMothershipMicro:
            case sMothershipWeapons:
            case sMothershipDockingBay:
            case sMothershipLowerSpine:
                if (isOwned)
                {
                    aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], ship);
                    if (isQueuedForExternalBuild)
                    {
                        aiHasBuildQueued = TRUE;
                    }
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MODULE_ENEMY], ship);
                }
                break;

            case sCarrierSupport:
                if (isOwned)
                {
                    if (moduleHost != NULL)
                    {
                        if (aiCarrier1 == NULL || moduleHost == aiCarrier1)
                        {
                            aiCarrier1 = moduleHost;
                            aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_CARRIER1_SUPPORT], ship);
                        }
                        else if (aiCarrier2 == NULL || moduleHost == aiCarrier2)
                        {
                            aiCarrier2 = moduleHost;
                            aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_CARRIER2_SUPPORT], ship);
                        }
                    }
                    if (isQueuedForExternalBuild)
                    {
                        aiHasBuildQueued = TRUE;
                        aiHasSupportModuleQueued = TRUE;
                        if (moduleHost != NULL)
                        {
                            if (moduleHost == aiCarrier1)
                            {
                                aiHasCarrier1ModuleQueued = TRUE;
                            }
                            else if (moduleHost == aiCarrier2)
                            {
                                aiHasCarrier2ModuleQueued = TRUE;
                            }
                        }
                    }

                    aisParityLogTeams();
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MODULE_ENEMY], ship);
                }
                break;

            case sMothershipSupport:
            case bMothershipSupport:
                if (isOwned)
                {
                    aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_MOTHERSHIP_SUPPORT], ship);
                    if (isQueuedForExternalBuild)
                    {
                        aiHasBuildQueued = TRUE;
                        aiHasSupportModuleQueued = TRUE;
                    }
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MODULE_ENEMY], ship);
                }
                break;

            case sMothershipBigGun:
                if (isOwned)
                {
                    Ship *bigGunHost = (ship->clampInfo != NULL) ? (Ship *)ship->clampInfo->host : NULL;

                    aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], ship);
                    if (isQueuedForExternalBuild)
                    {
                        aiHasBuildQueued = TRUE;
                    }
                    if (bigGunHost != NULL &&
                        mothershipBigGunAlmostComplete(bigGunHost))
                    {
                        aisAddShipToSingleSelectionTeam(&aisTeams[AIS_TEAM_MOTHERSHIP_BIGGUN], ship);
                    }
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_MODULE_ENEMY], ship);
                }
                break;

            case bDFGFrigate:
                if (isOwned)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_DFG_FRIGATE], ship);
                }
                else if (isEnemy)
                {
                    cat_AICountShipClass(&aisTeams[AIS_TEAM_DFG_FRIGATE_ENEMY], ship);
                    if (aiCurrentAIPlayer->player->PlayerMothership != NULL
                        && GetDistance(
                            &ship->posinfo.position,
                            &aiCurrentAIPlayer->player->PlayerMothership->posinfo.position) < AIS_DFG_MOTHERSHIP_DISTANCE)
                    {
                        aiDFGFrigateByMothership = TRUE;
                    }
                }
                break;

            case dCrystal0:
            case dCrystal1:
                cat_AICountShipClass(&aisTeams[AIS_TEAM_CRYSTAL], ship);
                break;
        }
    }

    for (shipNode = universe.mainCommandLayer.todolist.head; shipNode != NULL; shipNode = shipNode->next)
    {
        CommandToDo *command;

        command = (CommandToDo *)shipNode->structptr;
        if (command != NULL
            && command->ordertype.order == COMMAND_BUILDING_SHIP
            && command->buildingship.playerIndex == aiCurrentAIPlayer->player->playerIndex)
        {
            ShipStaticInfo *queuedStatic;

            aiBuildingShip[command->buildingship.shipType]++;
            queuedStatic = GetShipStaticInfo(
                (ShipType)command->buildingship.shipType,
                aiCurrentAIPlayer->player->race);
            aiSupportUnitsPending += ShipStaticInfoGetSupportUnits(queuedStatic);
        }
    }
}

void aisAllianceUpdate(void)
{
    ubyte allianceMode;
    uword firstComputerPlayer;
    sdword computerCount;
    uword playerIndex;

    allianceMode = tpGameCreated.aiplayerBigotry;
    if (allianceMode == 0)
    {
        return;
    }

    firstComputerPlayer = tpGameCreated.numPlayers;
    if (allianceMode == 1)
    {
        computerCount = (sdword)universe.numPlayers - firstComputerPlayer;
        switch (computerCount)
        {
            case 3:
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 1), (unsigned int)(firstComputerPlayer + 2));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 1, firstComputerPlayer + 2);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 2), (unsigned int)(firstComputerPlayer + 1));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 2, firstComputerPlayer + 1);
                break;

            case 4:
            case 5:
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)firstComputerPlayer, (unsigned int)(firstComputerPlayer + 1));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer, firstComputerPlayer + 1);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 1), (unsigned int)firstComputerPlayer);
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 1, firstComputerPlayer);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 2), (unsigned int)(firstComputerPlayer + 3));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 2, firstComputerPlayer + 3);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 3), (unsigned int)(firstComputerPlayer + 2));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 3, firstComputerPlayer + 2);
                break;

            case 6:
            case 7:
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)firstComputerPlayer, (unsigned int)(firstComputerPlayer + 1));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer, firstComputerPlayer + 1);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 1), (unsigned int)firstComputerPlayer);
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 1, firstComputerPlayer);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 2), (unsigned int)(firstComputerPlayer + 3));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 2, firstComputerPlayer + 3);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 3), (unsigned int)(firstComputerPlayer + 2));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 3, firstComputerPlayer + 2);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 4), (unsigned int)(firstComputerPlayer + 5));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 4, firstComputerPlayer + 5);
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)(firstComputerPlayer + 5), (unsigned int)(firstComputerPlayer + 4));
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, firstComputerPlayer + 5, firstComputerPlayer + 4);
                break;

            default:
                break;
        }
    }
    else
    {
        for (playerIndex = firstComputerPlayer; playerIndex < 8; playerIndex++)
        {
            uword allyIndex;

            for (allyIndex = playerIndex + 1; allyIndex < 8; allyIndex++)
            {
                aisParityLog("event=order kind=set_alliance allianceType=%u cur=%u new=%u", (unsigned int)ALLIANCE_FORMNEWALLIANCE, (unsigned int)playerIndex, (unsigned int)allyIndex);
                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, playerIndex, allyIndex);
            }
        }
    }
}

// Cataclysm-specific: keeps the processor near the active crystal target or escorting workers.
void aisMoveProcessor(void)
{
    Ship *processor;
    SelectCommand selectCommand;

    if (aisTeams[AIS_TEAM_PROCESSOR].totalteamsize != 1
        || aisTeams[AIS_TEAM_PROCESSOR].numselections == 0
        || aisTeams[AIS_TEAM_PROCESSOR].selection[0].sel.numShips == 0)
    {
        return;
    }

    processor = aisTeams[AIS_TEAM_PROCESSOR].selection[0].sel.ShipPtr[0];
    memset(&selectCommand, 0, sizeof(selectCommand));
    selectCommand.numShips = 1;
    selectCommand.ShipPtr[0] = processor;

    if (aiCurrentCrystalTarget != NULL)
    {
        vector offset;

        vecSub(offset, aiCurrentCrystalTarget->posinfo.position, processor->posinfo.position);
        if (getVectDistSloppy(offset) > AIS_PROCESSOR_MOVE_DISTANCE)
        {
            vector destination;

            destination = aiCurrentCrystalTarget->posinfo.position;
            destination.x += AIS_PROCESSOR_MOVE_OFFSET;
            destination.y += AIS_PROCESSOR_MOVE_OFFSET;
            {
                char detail[160];

                snprintf(detail, sizeof(detail), "from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f", processor->posinfo.position.x, processor->posinfo.position.y, processor->posinfo.position.z, destination.x, destination.y, destination.z);
                aisParityLogOrder("move", &selectCommand, detail);
            }
            clWrapMovePoint(&universe.mainCommandLayer, &selectCommand, processor->posinfo.position, destination);
        }
    }
    else if (aisTeams[AIS_TEAM_WORKER].totalteamsize > 0
             && aisTeams[AIS_TEAM_WORKER].numselections > 0
             && aisTeams[AIS_TEAM_WORKER].selection[0].sel.numShips > 0)
    {
        ProtectCommand protectCommand;

        memset(&protectCommand, 0, sizeof(protectCommand));
        protectCommand.numShips = 1;
        protectCommand.ShipPtr[0] = aisTeams[AIS_TEAM_WORKER].selection[0].sel.ShipPtr[0];
        {
            char detail[128];
            char targetSummary[96];

            aisParityFormatShipSummary(targetSummary, sizeof(targetSummary), protectCommand.ShipPtr[0]);
            snprintf(detail, sizeof(detail), "protectCount=%d target_%s", protectCommand.numShips, targetSummary);
            aisParityLogOrder("protect", &selectCommand, detail);
        }
        clWrapProtect(&universe.mainCommandLayer, &selectCommand, &protectCommand);
    }
}

// Cataclysm-specific: repositions the lead harvester carrier around the active crystal target.
void aisMoveHarvesterGroups(void)
{
    Ship *carrier;
    SelectCommand selectCommand;

    if (aisTeams[AIS_TEAM_PROCESSOR].totalteamsize != 0
        || aisTeams[AIS_TEAM_CARRIER].totalteamsize <= 0
        || aisTeams[AIS_TEAM_CARRIER].numselections == 0
        || aisTeams[AIS_TEAM_CARRIER].selection[0].sel.numShips == 0)
    {
        return;
    }

    carrier = aisTeams[AIS_TEAM_CARRIER].selection[0].sel.ShipPtr[0];
    memset(&selectCommand, 0, sizeof(selectCommand));
    selectCommand.numShips = 1;
    selectCommand.ShipPtr[0] = carrier;

    if (aiCurrentCrystalTarget != NULL)
    {
        vector offset;

        vecSub(offset, aiCurrentCrystalTarget->posinfo.position, carrier->posinfo.position);
        if (getVectDistSloppy(offset) > AIS_PROCESSOR_MOVE_DISTANCE)
        {
            vector destination;

            destination = aiCurrentCrystalTarget->posinfo.position;
            destination.x += AIS_PROCESSOR_MOVE_OFFSET;
            destination.y += AIS_PROCESSOR_MOVE_OFFSET;
            {
                char detail[160];

                snprintf(detail, sizeof(detail), "from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f", carrier->posinfo.position.x, carrier->posinfo.position.y, carrier->posinfo.position.z, destination.x, destination.y, destination.z);
                aisParityLogOrder("move", &selectCommand, detail);
            }
            clWrapMovePoint(&universe.mainCommandLayer, &selectCommand, carrier->posinfo.position, destination);
        }
    }
    else if (aisTeams[AIS_TEAM_WORKER].totalteamsize > 0
             && aisTeams[AIS_TEAM_WORKER].numselections > 0
             && aisTeams[AIS_TEAM_WORKER].selection[0].sel.numShips > 0)
    {
        ProtectCommand protectCommand;

        memset(&protectCommand, 0, sizeof(protectCommand));
        protectCommand.numShips = 1;
        protectCommand.ShipPtr[0] = aisTeams[AIS_TEAM_WORKER].selection[0].sel.ShipPtr[0];
        {
            char detail[128];
            char targetSummary[96];

            aisParityFormatShipSummary(targetSummary, sizeof(targetSummary), protectCommand.ShipPtr[0]);
            snprintf(detail, sizeof(detail), "protectCount=%d target_%s", protectCommand.numShips, targetSummary);
            aisParityLogOrder("protect", &selectCommand, detail);
        }
        clWrapProtect(&universe.mainCommandLayer, &selectCommand, &protectCommand);
    }
}

// Cataclysm-specific: keeps the mothership displaced from the nearest enemy mothership and opportunistically fires the big gun.
void aisMoveMothership(void)
{
    Ship *mothership;
    AISTeamSelection *enemySelection;
    Ship *enemyMothership;

    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_INT)
    {
        /* keep original move distances from the binary */
    }
    else if (aiCurrentAIPlayer->aiplayerDifficultyLevel != AI_ADV)
    {
        return;
    }

    if (aisTeams[AIS_TEAM_MOTHERSHIP].totalteamsize <= 0
        || aisTeams[AIS_TEAM_MOTHERSHIP].numselections == 0
        || aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.numShips == 0)
    {
        return;
    }

    mothership = aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0];
    enemySelection = aisFindClosestTeamSelectionToPoint(AIS_TEAM_MOTHERSHIP_ENEMY, &mothership->posinfo.position);
    if (enemySelection == NULL || enemySelection->sel.numShips <= 0)
    {
        return;
    }

    enemyMothership = enemySelection->sel.ShipPtr[0];
    if (aiOwnedSiegeMothership == NULL)
    {
        if (aisTeams[AIS_TEAM_MOTHERSHIP_BIGGUN].totalteamsize > 0
            && !aiDFGFrigateByMothership
            && GetDistance(&mothership->posinfo.position, &enemyMothership->posinfo.position) < AIS_MOTHERSHIP_SIEGE_DISTANCE)
        {
            char detail[96];

            snprintf(detail, sizeof(detail), "target=%.1f,%.1f,%.1f", enemyMothership->posinfo.position.x, enemyMothership->posinfo.position.y, enemyMothership->posinfo.position.z);
            aisParityLogOrder("siege_cannon", (SelectCommand *)&aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel, detail);
            clWrapSiegeCannon(
                &universe.mainCommandLayer,
                (SelectCommand *)&aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel,
                &enemyMothership->posinfo.position);
            return;
        }

        if (!aisShipHasActiveCommand(mothership))
        {
            real32 moveDistance;
            vector direction;
            vector destination;

            moveDistance = (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV) ? 20000.0f : 40000.0f;
            vecSub(direction, mothership->posinfo.position, enemyMothership->posinfo.position);
            vecNormalize(&direction);
            vecScalarMultiply(direction, direction, moveDistance);
            vecAdd(destination, enemyMothership->posinfo.position, direction);

            if (destination.x < AIS_UNIVERSE_EDGE_MARGIN - smUniverseSizeX)
            {
                destination.x = AIS_UNIVERSE_EDGE_MARGIN - smUniverseSizeX;
            }
            else if (destination.x > smUniverseSizeX - AIS_UNIVERSE_EDGE_MARGIN)
            {
                destination.x = smUniverseSizeX - AIS_UNIVERSE_EDGE_MARGIN;
            }

            if (destination.y < AIS_UNIVERSE_EDGE_MARGIN - smUniverseSizeY)
            {
                destination.y = AIS_UNIVERSE_EDGE_MARGIN - smUniverseSizeY;
            }
            else if (destination.y > smUniverseSizeY - AIS_UNIVERSE_EDGE_MARGIN)
            {
                destination.y = smUniverseSizeY - AIS_UNIVERSE_EDGE_MARGIN;
            }

            if (destination.z < AIS_UNIVERSE_EDGE_MARGIN - smUniverseSizeZ)
            {
                destination.z = AIS_UNIVERSE_EDGE_MARGIN - smUniverseSizeZ;
            }
            else if (destination.z > smUniverseSizeZ - AIS_UNIVERSE_EDGE_MARGIN)
            {
                destination.z = smUniverseSizeZ - AIS_UNIVERSE_EDGE_MARGIN;
            }

            {
                char detail[160];

                snprintf(detail, sizeof(detail), "from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f", mothership->posinfo.position.x, mothership->posinfo.position.y, mothership->posinfo.position.z, destination.x, destination.y, destination.z);
                aisParityLogOrder("move", (SelectCommand *)&aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel, detail);
            }
            clWrapMovePoint(
                &universe.mainCommandLayer,
                (SelectCommand *)&aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel,
                mothership->posinfo.position,
                destination);
        }
    }
}

// Cataclysm-specific: assigns sentinel groups to protect the strongest repair-capable medium combat ship.
void aisSentinelUpdate(void)
{
    AISTeamEntry *sentinelTeam;
    AISTeamEntry *mediumCombatTeam;
    Ship *bestRepairShip;
    sdword selectionIndex;

    sentinelTeam = &aisTeams[AIS_TEAM_SENTINEL];
    if (sentinelTeam->numselections <= 0)
    {
        return;
    }

    bestRepairShip = NULL;
    mediumCombatTeam = &aisTeams[AIS_TEAM_MEDIUM_COMBAT];
    for (selectionIndex = 0; selectionIndex < (sdword)mediumCombatTeam->numselections; selectionIndex++)
    {
        AISTeamSelection *mediumSelection;
        sdword shipIndex;

        mediumSelection = &mediumCombatTeam->selection[selectionIndex];
        for (shipIndex = 0; shipIndex < mediumSelection->sel.numShips; shipIndex++)
        {
            Ship *candidate;

            candidate = mediumSelection->sel.ShipPtr[shipIndex];
            if (bestRepairShip == NULL
                || candidate->staticinfo->repairOtherShipRate > bestRepairShip->staticinfo->repairOtherShipRate)
            {
                bestRepairShip = candidate;
            }
        }
    }

    if (bestRepairShip == NULL)
    {
        return;
    }

    for (selectionIndex = 0; selectionIndex < (sdword)sentinelTeam->numselections; selectionIndex++)
    {
        AISTeamSelection *sentinelSelection;
        SelectCommand *selection;
        bool32 alreadyProtecting;
        sdword shipIndex;

        sentinelSelection = &sentinelTeam->selection[selectionIndex];
        selection = (SelectCommand *)&sentinelSelection->sel;
        if (selection->numShips <= 3 || selection->numShips > 12)
        {
            continue;
        }

        alreadyProtecting = FALSE;
        for (shipIndex = 0; shipIndex < selection->numShips; shipIndex++)
        {
            Ship *sentinel;
            CommandToDo *command;

            sentinel = selection->ShipPtr[shipIndex];
            command = sentinel->command;
            if (command != NULL
                && (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                && command->protect != NULL
                && ShipInSelection((SelectCommand *)command->protect, bestRepairShip))
            {
                alreadyProtecting = TRUE;
                break;
            }
        }

        if (!alreadyProtecting)
        {
            ProtectCommand protectCommand;
            char detail[128];
            char targetSummary[96];

            memset(&protectCommand, 0, sizeof(protectCommand));
            protectCommand.numShips = 1;
            protectCommand.ShipPtr[0] = bestRepairShip;
            aisParityFormatShipSummary(targetSummary, sizeof(targetSummary), bestRepairShip);
            snprintf(detail, sizeof(detail), "protectCount=%d target_%s", protectCommand.numShips, targetSummary);
            aisParityLogOrder("protect", selection, detail);
            clWrapProtect(&universe.mainCommandLayer, selection, &protectCommand);
            aisParityLogOrder("formation", selection, "formation=6");
            clWrapFormation(&universe.mainCommandLayer, selection, FORCEFIELD_SPHERE_FORMATION);
        }
    }
}

// Cataclysm-specific: apportions idle leeches across worker, carrier, and mothership targets near the AI mothership.
void aisLeechUpdate(void)
{
    Ship *mothership;
    vector *mothershipPosition;
    sdword flatIndex;
    MaxSelection leechSelection;
    AISTeamSelection *targetSelection;

    if (aiCurrentAIPlayer->player->PlayerMothership == NULL)
    {
        return;
    }

    mothership = aiCurrentAIPlayer->player->PlayerMothership;
    mothershipPosition = &mothership->posinfo.position;
    flatIndex = 0;

    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
    {
        targetSelection = aisFindClosestTeamSelectionToPoint(AIS_TEAM_WORKER_ENEMY, mothershipPosition);
        if (targetSelection != NULL)
        {
            memset(&leechSelection, 0, sizeof(leechSelection));
            while (flatIndex < aisTeams[AIS_TEAM_LEECH].totalteamsize
                   && leechSelection.numShips < 2 * targetSelection->sel.numShips)
            {
                Ship *leech;

                leech = aisGetTeamShipByFlatIndex(flatIndex, &aisTeams[AIS_TEAM_LEECH]);
                if (leech == NULL)
                {
                    break;
                }
                if (!aisShipHasActiveCommand(leech))
                {
                    selSelectionAddSingleShip(&leechSelection, leech);
                }
                flatIndex++;
            }

            if (leechSelection.numShips > 0)
            {
                char detail[160];
                char targetSummary[96];

                aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)targetSelection->sel.ShipPtr[0]);
                snprintf(detail, sizeof(detail), "specialCount=%d %s", targetSelection->sel.numShips, targetSummary);
                aisParityLogOrder("special", (SelectCommand *)&leechSelection, detail);
                clWrapSpecial(
                    &universe.mainCommandLayer,
                    (SelectCommand *)&leechSelection,
                    (SpecialCommand *)&targetSelection->sel);
            }
        }

        targetSelection = aisFindClosestTeamSelectionToPoint(AIS_TEAM_CARRIER_ENEMY, mothershipPosition);
        if (targetSelection != NULL)
        {
            memset(&leechSelection, 0, sizeof(leechSelection));
            while (flatIndex < aisTeams[AIS_TEAM_LEECH].totalteamsize
                   && leechSelection.numShips < 10 * targetSelection->sel.numShips)
            {
                Ship *leech;

                leech = aisGetTeamShipByFlatIndex(flatIndex, &aisTeams[AIS_TEAM_LEECH]);
                if (leech == NULL)
                {
                    break;
                }
                if (!aisShipHasActiveCommand(leech))
                {
                    selSelectionAddSingleShip(&leechSelection, leech);
                }
                flatIndex++;
            }

            if (leechSelection.numShips > 0)
            {
                char detail[160];
                char targetSummary[96];

                aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)targetSelection->sel.ShipPtr[0]);
                snprintf(detail, sizeof(detail), "specialCount=%d %s", targetSelection->sel.numShips, targetSummary);
                aisParityLogOrder("special", (SelectCommand *)&leechSelection, detail);
                clWrapSpecial(
                    &universe.mainCommandLayer,
                    (SelectCommand *)&leechSelection,
                    (SpecialCommand *)&targetSelection->sel);
            }
        }
    }

    targetSelection = aisFindClosestTeamSelectionToPoint(AIS_TEAM_MOTHERSHIP_ENEMY, mothershipPosition);
    if (targetSelection != NULL)
    {
        memset(&leechSelection, 0, sizeof(leechSelection));
        while (flatIndex < aisTeams[AIS_TEAM_LEECH].totalteamsize)
        {
            Ship *leech;
            bool32 hasActiveCommand;

            leech = aisGetTeamShipByFlatIndex(flatIndex, &aisTeams[AIS_TEAM_LEECH]);
            if (leech == NULL)
            {
                break;
            }
            hasActiveCommand = aisShipHasActiveCommand(leech);
            if (!hasActiveCommand)
            {
                selSelectionAddSingleShip(&leechSelection, leech);
            }
            flatIndex++;
        }

        if (leechSelection.numShips > 0)
        {
            char detail[160];
            char targetSummary[96];

            aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)targetSelection->sel.ShipPtr[0]);
            snprintf(detail, sizeof(detail), "specialCount=%d %s", targetSelection->sel.numShips, targetSummary);
            aisParityLogOrder("special", (SelectCommand *)&leechSelection, detail);
            clWrapSpecial(
                &universe.mainCommandLayer,
                (SelectCommand *)&leechSelection,
                (SpecialCommand *)&targetSelection->sel);
        }
    }
}

// Cataclysm-specific: assigns scouts to shadow enemy motherships, picket home, and harass scout-class targets.
void aisAttackUpdate(void)
{
    AISTeamEntry *scoutTeam;
    AISTeamEntry *enemyMothershipTeam;
    sdword scoutIndex;
    sdword scoutShadowAssignments;

    scoutTeam = &aisTeams[AIS_TEAM_SCOUT];
    enemyMothershipTeam = &aisTeams[AIS_TEAM_MOTHERSHIP_ENEMY];
    scoutIndex = 0;

    switch (aiCurrentAIPlayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            scoutShadowAssignments = 5;
            break;

        case AI_INT:
            scoutShadowAssignments = 3;
            break;

        default:
            scoutShadowAssignments = 1;
            break;
    }

    while (scoutShadowAssignments > 0
           && scoutIndex < scoutTeam->totalteamsize
           && scoutIndex < enemyMothershipTeam->totalteamsize)
    {
        Ship *scout;
        Ship *enemyMothership;
        vector offset;

        scout = aisGetTeamShipByFlatIndex(scoutIndex, scoutTeam);
        enemyMothership = aisGetTeamShipByFlatIndex(scoutIndex, enemyMothershipTeam);
        if (scout == NULL || enemyMothership == NULL)
        {
            break;
        }

        vecSub(offset, scout->posinfo.position, enemyMothership->posinfo.position);
        if (getVectDistSloppy(offset) > AIS_ATTACK_SCOUT_SHADOW_DISTANCE)
        {
            SelectCommand scoutSelection;
            vector destination;

            vecNormalize(&offset);
            vecScalarMultiply(offset, offset, AIS_ATTACK_SCOUT_SHADOW_OFFSET);
            vecAdd(destination, enemyMothership->posinfo.position, offset);

            memset(&scoutSelection, 0, sizeof(scoutSelection));
            scoutSelection.numShips = 1;
            scoutSelection.ShipPtr[0] = scout;
            {
                char detail[160];

                snprintf(detail, sizeof(detail), "from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f", scout->posinfo.position.x, scout->posinfo.position.y, scout->posinfo.position.z, destination.x, destination.y, destination.z);
                aisParityLogOrder("move", &scoutSelection, detail);
            }
            clWrapMovePoint(
                &universe.mainCommandLayer,
                &scoutSelection,
                scout->posinfo.position,
                destination);
        }

        scoutIndex++;
        scoutShadowAssignments--;
    }

    if (aiCurrentAIPlayer->aiplayerDifficultyLevel > AI_BEG
        && aisTeams[AIS_TEAM_MOTHERSHIP].totalteamsize > 0
        && aisTeams[AIS_TEAM_MOTHERSHIP].numselections > 0
        && aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.numShips > 0)
    {
        Ship *scout;

        scout = aisGetTeamShipByFlatIndex(scoutIndex, scoutTeam);
        if (scout != NULL && !aisShipHasActiveCommand(scout))
        {
            SelectCommand scoutSelection;
            ProtectCommand mothershipProtect;

            memset(&scoutSelection, 0, sizeof(scoutSelection));
            memset(&mothershipProtect, 0, sizeof(mothershipProtect));
            scoutSelection.numShips = 1;
            scoutSelection.ShipPtr[0] = scout;
            mothershipProtect.numShips = 1;
            mothershipProtect.ShipPtr[0] = aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0];
            {
                char detail[128];
                char targetSummary[96];

                aisParityFormatShipSummary(targetSummary, sizeof(targetSummary), mothershipProtect.ShipPtr[0]);
                snprintf(detail, sizeof(detail), "protectCount=%d target_%s", mothershipProtect.numShips, targetSummary);
                aisParityLogOrder("protect", &scoutSelection, detail);
            }
            clWrapProtect(&universe.mainCommandLayer, &scoutSelection, &mothershipProtect);
            scoutIndex++;
        }
    }

    if (aisTeams[AIS_TEAM_SCOUT_ENEMY].totalteamsize > 0)
    {
        MaxSelection scoutAttackers;
        sdword attackScoutCount;

        switch (aiCurrentAIPlayer->aiplayerDifficultyLevel)
        {
            case AI_ADV:
                attackScoutCount = 6;
                break;

            case AI_INT:
                attackScoutCount = 3;
                break;

            default:
                attackScoutCount = 0;
                break;
        }

        memset(&scoutAttackers, 0, sizeof(scoutAttackers));
        while (scoutIndex < scoutTeam->totalteamsize && attackScoutCount > 0)
        {
            Ship *scout;

            scout = aisGetTeamShipByFlatIndex(scoutIndex, scoutTeam);
            if (scout == NULL)
            {
                break;
            }

            if (!aisShipHasActiveCommand(scout))
            {
                selSelectionAddSingleShipNoCheck(&scoutAttackers, scout);
                attackScoutCount--;
            }
            scoutIndex++;
        }

        if (scoutAttackers.numShips > 0
            && aisTeams[AIS_TEAM_SCOUT_ENEMY].numselections > 0
            && aisTeams[AIS_TEAM_SCOUT_ENEMY].selection[0].sel.numShips > 0)
        {
            AttackCommand scoutTarget;

            memset(&scoutTarget, 0, sizeof(scoutTarget));
            scoutTarget.numTargets = 1;
            scoutTarget.TargetPtr[0] = (TargetPtr)aisTeams[AIS_TEAM_SCOUT_ENEMY].selection[0].sel.ShipPtr[0];
            {
                char detail[160];
                char targetSummary[96];

                aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)scoutTarget.TargetPtr[0]);
                snprintf(detail, sizeof(detail), "targets=%d %s", scoutTarget.numTargets, targetSummary);
                aisParityLogOrder("attack", (SelectCommand *)&scoutAttackers, detail);
            }
            clWrapAttack(
                &universe.mainCommandLayer,
                (SelectCommand *)&scoutAttackers,
                &scoutTarget);
        }
    }

    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
    {
        Node *node;
        Ship *derelictRecon;

        derelictRecon = NULL;
        for (node = universe.DerelictList.head; node != NULL; node = node->next)
        {
            Ship *derelict;

            derelict = (Ship *)listGetStructOfNode(node);
            if (derelict->shiptype == sRecon)
            {
                derelictRecon = derelict;
                break;
            }
        }

        if (derelictRecon != NULL)
        {
            Ship *scout;

            scout = aisGetTeamShipByFlatIndex(scoutIndex, scoutTeam);
            if (scout != NULL)
            {
                SelectCommand scoutSelection;
                AttackCommand derelictTarget;

                memset(&scoutSelection, 0, sizeof(scoutSelection));
                memset(&derelictTarget, 0, sizeof(derelictTarget));
                scoutSelection.numShips = 1;
                scoutSelection.ShipPtr[0] = scout;
                derelictTarget.numTargets = 1;
                derelictTarget.TargetPtr[0] = (TargetPtr)derelictRecon;
                {
                    char detail[160];
                    char targetSummary[96];

                    aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)derelictTarget.TargetPtr[0]);
                    snprintf(detail, sizeof(detail), "targets=%d %s", derelictTarget.numTargets, targetSummary);
                    aisParityLogOrder("attack", &scoutSelection, detail);
                }
                clWrapAttack(&universe.mainCommandLayer, &scoutSelection, &derelictTarget);
            }
        }
    }
}

// Cataclysm-specific: assigns a source team against the nearest viable target team buckets.
void aisManageTeam(
    AISTeamEntry *teams,
    sdword minimum,
    sdword maximum,
    ubyte randomizeFormation,
    ubyte startExternalConstruction,
    ubyte allowHyperspace,
    ubyte enableKamikaze,
    sdword targetTeamCount,
    sdword *targetTeams)
{
    sdword selectionIndex;
    sdword runningSize;
    sdword currentTargetBucket;

    if (teams->numselections <= 0 || targetTeamCount <= 0)
    {
        return;
    }

    runningSize = 0;
    currentTargetBucket = 0;

    for (selectionIndex = 0; selectionIndex < (sdword)teams->numselections; selectionIndex++)
    {
        AISTeamSelection *selection;
        AISTeamSelection *targetSelection;
        sdword fallbackBucket;

        selection = &teams->selection[selectionIndex];
        if (!aisManageTeamSelectionIsAvailable(selection, minimum))
        {
            continue;
        }

        runningSize += selection->sel.numShips;
        if (runningSize > maximum && selectionIndex > 0)
        {
            currentTargetBucket++;
            runningSize = 0;
            if (currentTargetBucket >= targetTeamCount)
            {
                currentTargetBucket = targetTeamCount - 1;
            }
        }

        targetSelection = aisFindClosestTeamSelectionToPoint(targetTeams[currentTargetBucket], &selection->center);
        if (targetSelection == NULL)
        {
            for (fallbackBucket = currentTargetBucket + 1; fallbackBucket < targetTeamCount; fallbackBucket++)
            {
                currentTargetBucket = fallbackBucket;
                runningSize = 0;
                targetSelection = aisFindClosestTeamSelectionToPoint(targetTeams[currentTargetBucket], &selection->center);
                if (targetSelection != NULL)
                {
                    break;
                }
            }
        }

        if (targetSelection == NULL || targetSelection->sel.numShips <= 0)
        {
            continue;
        }

        if (bitTest(tpGameCreated.flag, MG_Hyperspace)
            && allowHyperspace
            && aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
        {
            Ship *leadShip;
            Ship *targetShip;
            vector shipDelta;
            real32 distance;

            leadShip = selection->sel.ShipPtr[0];
            targetShip = targetSelection->sel.ShipPtr[0];
            vecSub(shipDelta, leadShip->posinfo.position, targetShip->posinfo.position);
            distance = getVectDistSloppy(shipDelta);
            if (distance > AIS_MANAGE_TEAM_HYPERSPACE_TARGET_DISTANCE
                && hyperspaceCost(distance, (SelectCommand *)&selection->sel) + AIS_MANAGE_TEAM_HYPERSPACE_TARGET_DISTANCE
                    < aiCurrentAIPlayer->player->resourceUnits)
            {
                vector destination;

                destination = targetShip->posinfo.position;
                destination.x += AIS_MANAGE_TEAM_HYPERSPACE_OFFSET;
                destination.y += AIS_MANAGE_TEAM_HYPERSPACE_OFFSET;
                destination.z += AIS_MANAGE_TEAM_HYPERSPACE_Z_OFFSET;
                {
                    char detail[160];

                    snprintf(detail, sizeof(detail), "from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f", selection->center.x, selection->center.y, selection->center.z, destination.x, destination.y, destination.z);
                    aisParityLogOrder("hyperspace", (SelectCommand *)&selection->sel, detail);
                }
                clWrapMpHyperspace(
                    &universe.mainCommandLayer,
                    (SelectCommand *)&selection->sel,
                    selection->center,
                    destination);
                continue;
            }
        }

        if (randomizeFormation)
        {
            sdword formation;
            udword formationRoll;

            formationRoll = ranRandom(RAN_AIPlayer);
            aisParityLog("event=random site=manage_team_formation value=%u", (unsigned int)formationRoll);
            formation = formationRoll % 5;
            if (currentTargetBucket == 9 || currentTargetBucket == 18)
            {
                formation = 5;
            }
            {
                char detail[64];

                snprintf(detail, sizeof(detail), "formation=%d", (int)formation);
                aisParityLogOrder("formation", (SelectCommand *)&selection->sel, detail);
            }
            clWrapFormation(&universe.mainCommandLayer, (SelectCommand *)&selection->sel, formation);
        }

        if (startExternalConstruction)
        {
            aisParityLogOrder("external_construction", (SelectCommand *)&selection->sel, "operation=2 arg1=0 arg2=0 arg3=0");
            clWrapExternalConstructionSelection(
                &universe.mainCommandLayer,
                (SelectCommand *)&selection->sel,
                2,
                0,
                0,
                0);
        }

        if (enableKamikaze && currentTargetBucket == 5)
        {
            aisParityLogOrder("set_kamikaze", (SelectCommand *)&selection->sel, "");
            clWrapSetKamikaze(&universe.mainCommandLayer, (SelectCommand *)&selection->sel);
        }

        aisParityLogOrder("set_tactics", (SelectCommand *)&selection->sel, "tactics=2");
        clWrapSetTactics(&universe.mainCommandLayer, (SelectCommand *)&selection->sel, Aggressive);
        {
            char detail[160];
            char targetSummary[96];

            aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)targetSelection->sel.ShipPtr[0]);
            snprintf(detail, sizeof(detail), "targets=%d %s", targetSelection->sel.numShips, targetSummary);
            aisParityLogOrder("attack", (SelectCommand *)&selection->sel, detail);
        }
        clWrapAttack(
            &universe.mainCommandLayer,
            (SelectCommand *)&selection->sel,
            (AttackCommand *)&targetSelection->sel);
    }
}

// Cataclysm-specific: drives the periodic skirmish fleet update loop for multiplayer AI players.
void aisFleetCommand(void)
{
    real32 fleetUpdateInterval;
    sdword fighterMinimum;
    sdword fighterMaximum;
    sdword multibeamMinimum;
    sdword mediumCombatMinimum;
    sdword mediumCombatMaximum;

    fleetUpdateInterval = AIS_FLEET_UPDATE_BEGINNER_INTERVAL;
    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_INT)
    {
        fleetUpdateInterval = AIS_FLEET_UPDATE_INTERMEDIATE_INTERVAL;
    }
    else if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
    {
        fleetUpdateInterval = AIS_FLEET_UPDATE_ADVANCED_INTERVAL;
    }

    aisParityLog(
        "event=fleet_command_begin player=%u difficulty=%d ru=%d",
        aiCurrentAIPlayer != NULL && aiCurrentAIPlayer->player != NULL
            ? (unsigned int)aiCurrentAIPlayer->player->playerIndex
            : (unsigned int)UWORD_Max,
        aiCurrentAIPlayer != NULL ? (int)aiCurrentAIPlayer->aiplayerDifficultyLevel : -1,
        aiCurrentAIPlayer != NULL && aiCurrentAIPlayer->player != NULL
            ? aiCurrentAIPlayer->player->resourceUnits
            : -1);

    if (aiCurrentAIPlayer->player->PlayerMothership == NULL
        || universe.totaltimeelapsed - aiLastUpdateTimes[aiCurrentAIPlayer->player->playerIndex] <= fleetUpdateInterval)
    {
        return;
    }

    if (aisFleetCommandStartup)
    {
        aisFleetCommandStartup = FALSE;
    }

    aiLastUpdateTimes[aiCurrentAIPlayer->player->playerIndex] = universe.totaltimeelapsed;
    if (aiUpdateCount == 0)
    {
        aisAllianceUpdate();
        aiUpdateCount++;
    }

    aisSetupTeams();
    aisBuildModules();
    aisParityLogTeams();
    {
        udword fleetUpdateRoll;

        fleetUpdateRoll = ranRandom(RAN_AIPlayer);
        aisParityLog("event=random site=fleet_command_update_roll value=%u", (unsigned int)fleetUpdateRoll);
        if (((fleetUpdateRoll % 100) + 1)
        <= aisDifficultyTarget(
            aiCurrentAIPlayer->aiplayerDifficultyLevel,
            (sdword)AIS_FLEET_UPDATE_BEGINNER_CHANCE,
            (sdword)AIS_FLEET_UPDATE_INTERMEDIATE_CHANCE,
            (sdword)AIS_FLEET_UPDATE_ADVANCED_CHANCE))
        {
            aisFleetUpdate();
        }
    }
    aisCollectResources();
    aisMoveProcessor();
    aisAttackUpdate();

    if (bitTest(tpGameCreated.flag, MG_Hyperspace)
        && aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV
        && aiEnemyBuildingSiegeMothership != NULL
        && aisTeams[AIS_TEAM_DFG_FRIGATE].totalteamsize > 0
        && aiCurrentAIPlayer->player->resourceUnits > 20000
        )
    {
        vector destination;
        udword dfgHyperspaceRoll;

        dfgHyperspaceRoll = ranRandom(RAN_AIPlayer);
        aisParityLog("event=random site=fleet_command_dfg_hyperspace_roll value=%u", (unsigned int)dfgHyperspaceRoll);
        if (((dfgHyperspaceRoll % 100) + 1) >= 10)
        {
            goto skip_dfg_hyperspace;
        }

        destination = aiEnemyBuildingSiegeMothership->posinfo.position;
        destination.x += AIS_FLEET_DFG_HYPERSPACE_OFFSET;
        destination.y += AIS_FLEET_DFG_HYPERSPACE_OFFSET;
        destination.z += AIS_FLEET_DFG_HYPERSPACE_OFFSET;
        if (GetDistance(&destination, &aisTeams[AIS_TEAM_DFG_FRIGATE].selection[0].center) > AIS_FLEET_DFG_HYPERSPACE_MIN_DISTANCE)
        {
            SelectCommand dfgSelection;

            memset(&dfgSelection, 0, sizeof(dfgSelection));
            dfgSelection.numShips = 1;
            dfgSelection.ShipPtr[0] = aisTeams[AIS_TEAM_DFG_FRIGATE].selection[0].sel.ShipPtr[0];
            clWrapMpHyperspace(
                &universe.mainCommandLayer,
                &dfgSelection,
                aisTeams[AIS_TEAM_DFG_FRIGATE].selection[0].center,
                destination);
        }
    }
skip_dfg_hyperspace:

    aisLeechUpdate();

    fighterMinimum = 10;
    fighterMaximum = 10;
    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_INT)
    {
        fighterMaximum = 20;
    }
    else if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
    {
        fighterMaximum = 30;
    }
    if (aiCurrentAIPlayer->player->PlayerMothership != NULL)
    {
        ShipType mothershipType;

        mothershipType = aiCurrentAIPlayer->player->PlayerMothership->shiptype;
        if (mothershipType != sMothership && mothershipType != bMothership)
        {
            fighterMinimum = 5;
        }
    }

    aisManageTeam(
        &aisTeams[AIS_TEAM_FIGHTER],
        fighterMinimum,
        fighterMaximum,
        1,
        0,
        0,
        aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV,
        sizeof(aisFighterTargetTeams) / sizeof(aisFighterTargetTeams[0]),
        aisFighterTargetTeams);

    multibeamMinimum = 10;
    if (aiCurrentAIPlayer->player->PlayerMothership != NULL)
    {
        ShipType mothershipType;

        mothershipType = aiCurrentAIPlayer->player->PlayerMothership->shiptype;
        if (mothershipType != sMothership && mothershipType != bMothership)
        {
            multibeamMinimum = 4;
        }
    }

    aisManageTeam(
        &aisTeams[AIS_TEAM_MIMIC],
        multibeamMinimum,
        10,
        1,
        1,
        0,
        0,
        sizeof(aisMimicTargetTeams) / sizeof(aisMimicTargetTeams[0]),
        aisMimicTargetTeams);

    aisManageTeam(
        &aisTeams[AIS_TEAM_CRUISE_MISSILE],
        3,
        5,
        0,
        0,
        0,
        0,
        sizeof(aisCruiseMissileTargetTeams) / sizeof(aisCruiseMissileTargetTeams[0]),
        aisCruiseMissileTargetTeams);

    mediumCombatMinimum = 3;
    mediumCombatMaximum = 1;
    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_INT)
    {
        mediumCombatMaximum = 5;
    }
    else if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
    {
        mediumCombatMaximum = 10;
    }

    aisManageTeam(
        &aisTeams[AIS_TEAM_MULTIBEAM],
        mediumCombatMinimum,
        mediumCombatMaximum,
        0,
        0,
        0,
        0,
        sizeof(aisMultibeamTargetTeams) / sizeof(aisMultibeamTargetTeams[0]),
        aisMultibeamTargetTeams);

    mediumCombatMinimum = 1;
    mediumCombatMaximum = 1;
    if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_INT)
    {
        mediumCombatMaximum = 3;
    }
    else if (aiCurrentAIPlayer->aiplayerDifficultyLevel == AI_ADV)
    {
        mediumCombatMaximum = 10;
    }

    aisManageTeam(
        &aisTeams[AIS_TEAM_MEDIUM_COMBAT],
        mediumCombatMinimum,
        mediumCombatMaximum,
        0,
        0,
        1,
        0,
        sizeof(aisMediumCombatTargetTeams) / sizeof(aisMediumCombatTargetTeams[0]),
        aisMediumCombatTargetTeams);

    aisMoveMothership();
    aisMoveHarvesterGroups();
    aisSentinelUpdate();
    aisParityLog(
        "event=fleet_command_end player=%u difficulty=%d ru=%d",
        aiCurrentAIPlayer != NULL && aiCurrentAIPlayer->player != NULL
            ? (unsigned int)aiCurrentAIPlayer->player->playerIndex
            : (unsigned int)UWORD_Max,
        aiCurrentAIPlayer != NULL ? (int)aiCurrentAIPlayer->aiplayerDifficultyLevel : -1,
        aiCurrentAIPlayer != NULL && aiCurrentAIPlayer->player != NULL
            ? aiCurrentAIPlayer->player->resourceUnits
            : -1);
}

void aisCollectResources(void)
{
    MaxSelection collectSelection;
    MaxSelection cloakSelection;
    AttackCommand cloakTargets;
    sdword selectionIndex;

    memset(&collectSelection, 0, sizeof(collectSelection));
    memset(&cloakSelection, 0, sizeof(cloakSelection));
    memset(&cloakTargets, 0, sizeof(cloakTargets));

    for (selectionIndex = 0; selectionIndex < (sdword)aisTeams[AIS_TEAM_WORKER].numselections; selectionIndex++)
    {
        AISTeamSelection *selection;
        sdword shipIndex;

        selection = &aisTeams[AIS_TEAM_WORKER].selection[selectionIndex];
        for (shipIndex = 0; shipIndex < selection->sel.numShips; shipIndex++)
        {
            Ship *worker;

            worker = selection->sel.ShipPtr[shipIndex];
            if (worker->playerowner->resourceUnits >= 50000)
            {
                if (worker != aiWeakestOwnedShip)
                {
                    selSelectionAddSingleShipNoCheck(&cloakSelection, worker);
                }
                continue;
            }

            if (!bitTest(tpGameCreated.flag, MG_HarvestinEnabled))
            {
                continue;
            }
            if (aisShipHasActiveCommand(worker))
            {
                continue;
            }
            // Retail calls ShipHasUpgrade/SalCapPlayerHasCrystalUpgrade here but discards the
            // result on both branches (0x432554-0x432594: both paths jump to the same
            // selSelectionAddSingleShipNoCheck call with identical arguments) - the worker is
            // always added to the collect selection regardless of the crystal-upgrade check.
            if (ShipHasUpgrade(worker, RESEARCH_UPGRADE_MASK_SALVAGE))
            {
                aisPlayerHasCrystalUpgrade(worker->playerowner);
            }

            selSelectionAddSingleShipNoCheck(&collectSelection, worker);
        }
    }

    if (collectSelection.numShips > 0)
    {
        if (aiCurrentCrystalTarget != NULL)
        {
            char detail[160];
            char targetSummary[96];

            aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)aiCurrentCrystalTarget);
            snprintf(detail, sizeof(detail), "%s target=%.1f,%.1f,%.1f", targetSummary, aiCurrentCrystalTarget->posinfo.position.x, aiCurrentCrystalTarget->posinfo.position.y, aiCurrentCrystalTarget->posinfo.position.z);
            aisParityLogOrder("collect", (SelectCommand *)&collectSelection, detail);
        }
        else
        {
            aisParityLogOrder("collect", (SelectCommand *)&collectSelection, "");
        }
        clWrapCollectResource(
            &universe.mainCommandLayer,
            (SelectCommand *)&collectSelection,
            NULL);
    }

    if (cloakSelection.numShips > 0 && aiWeakestOwnedShip != NULL)
    {
        char detail[160];
        char targetSummary[96];

        cloakTargets.numTargets = 1;
        cloakTargets.TargetPtr[0] = (SpaceObjRotImpTarg *)aiWeakestOwnedShip;
        aisParityFormatTargetSummary(targetSummary, sizeof(targetSummary), (SpaceObjRotImpTarg *)aiWeakestOwnedShip);
        snprintf(detail, sizeof(detail), "cloak=%d targets=%d %s", FALSE, cloakTargets.numTargets, targetSummary);
        aisParityLogOrder("special_cloak", (SelectCommand *)&cloakSelection, detail);
        clWrapSpecialCloak(
            &universe.mainCommandLayer,
            (SelectCommand *)&cloakSelection,
            &cloakTargets,
            FALSE);
    }
}

void aisBuildModules(void)
{
    Player *player;
    Ship *playerMothership;

    player = aiCurrentAIPlayer->player;
    playerMothership = player->PlayerMothership;
    if (playerMothership == NULL || playerMothership->staticinfo->shipclass != CLASS_Mothership)
    {
        return;
    }

    if (player->race == RACE_Beast)
    {
        if (!rmResearchingAnything(player))
        {
            aisTryResearchFirstAvailableTech(
                player,
                aisBeastTechs,
                sizeof(aisBeastTechs) / sizeof(aisBeastTechs[0]));
        }

        if ((playerMothership->health / playerMothership->staticinfo->maxhealth) > AIS_BEAST_RESEARCH_SWITCH_RATIO &&
            aiCurrentAIPlayer->aiplayerDifficultyLevel > AI_BEG)
        {
            char shipSummary[96];

            (void)shipSummary;
            aisParityFormatShipSummary(shipSummary, sizeof(shipSummary), playerMothership);
            aisParityLog("event=order kind=set_beast_system %s system=%u", shipSummary, (unsigned int)BS_Research);
            clWrapSetBeastSystem(&universe.mainCommandLayer, playerMothership, BS_Research);
        }
        return;
    }

    if (aisTeams[AIS_TEAM_MOTHERSHIP_MODULES].numselections == 1)
    {
        MaxSelection *moduleSelection;
        sdword shipIndex;

        moduleSelection = &aisTeams[AIS_TEAM_MOTHERSHIP_MODULES].selection[0].sel;
        for (shipIndex = 0; shipIndex < moduleSelection->numShips; shipIndex++)
        {
            aisResearchModuleTechs(player, moduleSelection->ShipPtr[shipIndex]);
        }
    }
}

void aisBuildShip(Player *player, sdword shipType, sdword buildCost)
{
    ShipStaticInfo *shipStatic;
    Ship *mothership;
    sdword supportUnitsRequired;
    sdword supportUnitsUsed;
    sdword resourceUnitsRemaining;
    sdword carrierIndex;

    aisParityLog(
        "event=request_ship player=%u shipType=%d buildCost=%d ru=%d supportUsed=%d supportMax=%d",
        player != NULL ? (unsigned int)player->playerIndex : (unsigned int)UWORD_Max,
        (int)shipType,
        buildCost,
        player != NULL ? player->resourceUnits : -1,
        player != NULL ? player->supportUnitsUsed : -1,
        player != NULL ? player->supportUnitsMax : -1);

    shipStatic = GetShipStaticInfo((ShipType)shipType, player->race);
    supportUnitsRequired = aiSupportUnitsPending + player->supportUnitsUsed + ShipStaticInfoGetSupportUnits(shipStatic);
    supportUnitsUsed = supportUnitsRequired;
    resourceUnitsRemaining = player->resourceUnits;
    if (supportUnitsRequired >= player->supportUnitsMax || player->resourceUnits <= buildCost)
    {
        return;
    }

    mothership = NULL;
    if (aisTeams[AIS_TEAM_MOTHERSHIP].totalteamsize > 0)
    {
        mothership = aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0];
    }

    if (shipStatic->externalBuild)
    {
        if (mothership != NULL && rmCanBuildShip(player, (ShipType)shipType))
        {
            char creatorSummary[96];

            (void)creatorSummary;
            aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), mothership);
            aisParityLog(
                "event=order kind=create_ship shipType=%d race=%d player=%u creator_%s",
                (int)shipType,
                (int)player->race,
                (unsigned int)player->playerIndex,
                creatorSummary);
            clWrapCreateShip(
                &universe.mainCommandLayer,
                (ShipType)shipType,
                player->race,
                player->playerIndex,
                mothership);
        }
        return;
    }

    if (mothership != NULL && rmCanBuildShip(player, (ShipType)shipType))
    {
        char creatorSummary[96];

        (void)creatorSummary;
        aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), mothership);
        aisParityLog(
            "event=order kind=build_ship shipType=%d race=%d player=%u creator_%s",
            (int)shipType,
            (int)player->race,
            (unsigned int)player->playerIndex,
            creatorSummary);
        clWrapBuildShip(
            &universe.mainCommandLayer,
            (ShipType)shipType,
            player->race,
            player->playerIndex,
            mothership);
        supportUnitsUsed += ShipStaticInfoGetSupportUnits(shipStatic);
        resourceUnitsRemaining -= buildCost;
    }

    if (!rmCanBuildShip(player, (ShipType)shipType))
    {
        return;
    }

    for (carrierIndex = 0; carrierIndex < aisTeams[AIS_TEAM_CARRIER].totalteamsize; carrierIndex++)
    {
        Ship *carrier;

        if (supportUnitsUsed >= player->supportUnitsMax || resourceUnitsRemaining <= buildCost)
        {
            break;
        }

        carrier = aisGetTeamShipByFlatIndex(carrierIndex, &aisTeams[AIS_TEAM_CARRIER]);
        if (carrier == NULL)
        {
            break;
        }

        {
            char creatorSummary[96];

            (void)creatorSummary;
            aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), carrier);
            aisParityLog(
                "event=order kind=build_ship shipType=%d race=%d player=%u creator_%s",
                (int)shipType,
                (int)player->race,
                (unsigned int)player->playerIndex,
                creatorSummary);
        }
        clWrapBuildShip(
            &universe.mainCommandLayer,
            (ShipType)shipType,
            player->race,
            player->playerIndex,
            carrier);
        supportUnitsUsed += ShipStaticInfoGetSupportUnits(shipStatic);
        resourceUnitsRemaining -= buildCost;
    }
}

void aisFleetUpdate(void)
{
    Player *player;
    Ship *playerMothership;
    bool32 aiHasLaunchingShipsLocal;
    sdword workerType;
    sdword scoutType;
    sdword difficultyLevel;
    sdword workerCount;
    sdword workerTargetCount;
    sdword scoutCount;
    sdword scoutTargetCount;
    sdword fighterTargetCount;
    sdword fighterCount;
    sdword mediumShipToBuild;
    bool32 couldNotBuildMedium;

    player = aiCurrentAIPlayer->player;
    playerMothership = player->PlayerMothership;
    difficultyLevel = aiCurrentAIPlayer->aiplayerDifficultyLevel;
    workerType = (player->race == RACE_Beast) ? bWorker : sWorker;
    scoutType = (player->race == RACE_Beast) ? bRecon : sRecon;

    if (playerMothership != NULL && playerMothership->staticinfo->shipclass == CLASS_Mothership)
    {
        if (player->race == RACE_Beast)
        {
            if (aisTeams[AIS_TEAM_MOTHERSHIP].totalteamsize > 0
                && aisTeams[AIS_TEAM_MOTHERSHIP_SUPPORT].totalteamsize < 12
                && !aiHasSupportModuleQueued
                && rmCanBuildShip(player, bMothershipSupport))
            {
                char creatorSummary[96];

                (void)creatorSummary;
                aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0]);
                aisParityLog("event=order kind=create_ship shipType=%d race=%d player=%u creator_%s", (int)bMothershipSupport, (int)player->race, (unsigned int)player->playerIndex, creatorSummary);
                clWrapCreateShip(
                    &universe.mainCommandLayer,
                    bMothershipSupport,
                    player->race,
                    player->playerIndex,
                    aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0]);
            }
        }
        else if (player->resourceUnits > 0)
        {
            if (!aisTeamHasShipType(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], sMothershipDockingBay))
            {
                aisBuildShip(player, sMothershipDockingBay, 0);
            }
            else if (!aisTeamHasShipType(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], sMothershipMicro))
            {
                aisBuildShip(player, sMothershipMicro, 0);
            }
            else if (!aisTeamHasShipType(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], sMothershipWeapons))
            {
                aisBuildShip(player, sMothershipWeapons, 0);
            }
            else if (!aisTeamHasShipType(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], sMothershipSpecial))
            {
                aisBuildShip(player, sMothershipSpecial, 0);
            }
            else if (!aisTeamHasShipType(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], sMothershipArmour))
            {
                aisBuildShip(player, sMothershipArmour, 0);
            }
            else if (!aisTeamHasShipType(&aisTeams[AIS_TEAM_MOTHERSHIP_MODULES], sMothershipBigGun))
            {
                aisBuildShip(player, sMothershipBigGun, 5000);
            }

            if (aisTeams[AIS_TEAM_MOTHERSHIP].totalteamsize > 0
                && aisTeams[AIS_TEAM_MOTHERSHIP_SUPPORT].totalteamsize < 12
                && !aiHasSupportModuleQueued
                && rmCanBuildShip(player, sMothershipSupport))
            {
                char creatorSummary[96];

                (void)creatorSummary;
                aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0]);
                aisParityLog("event=order kind=create_ship shipType=%d race=%d player=%u creator_%s", (int)sMothershipSupport, (int)player->race, (unsigned int)player->playerIndex, creatorSummary);
                clWrapCreateShip(
                    &universe.mainCommandLayer,
                    sMothershipSupport,
                    player->race,
                    player->playerIndex,
                    aisTeams[AIS_TEAM_MOTHERSHIP].selection[0].sel.ShipPtr[0]);
            }
        }

        aiHasLaunchingShipsLocal = aiHasExternalConstruction;
        if (!aiHasExternalConstruction)
        {
            Node *todoNode;

            for (todoNode = universe.mainCommandLayer.todolist.head; todoNode != NULL; todoNode = todoNode->next)
            {
                CommandToDo *todoCommand;
                Ship *launchShip;

                todoCommand = (CommandToDo *)listGetStructOfNode(todoNode);
                if (todoCommand->ordertype.order != COMMAND_LAUNCHSHIP
                    || todoCommand->selection == NULL
                    || todoCommand->selection->numShips <= 0)
                {
                    continue;
                }

                launchShip = todoCommand->selection->ShipPtr[0];
                if (launchShip->playerowner == player
                    && launchShip->staticinfo->externalBuild
                    && launchShip->staticinfo->shipclass != CLASS_Component)
                {
                    aiHasLaunchingShipsLocal = TRUE;
                    aiHasExternalConstruction = TRUE;
                }
            }
        }

        if (!aiHasLaunchingShipsLocal)
        {
            if (player->race == RACE_Beast)
            {
                if (rmCanBuildShip(player, bCarrier) && aisTeams[AIS_TEAM_CARRIER].totalteamsize < 2)
                {
                    aisBuildShip(player, bCarrier, 2000);
                }
                else if (rmCanBuildShip(player, bHeavyCruiser) && player->resourceUnits > 2000)
                {
                    aisBuildShip(player, bHeavyCruiser, 3000);
                }
                else if (rmCanBuildShip(player, bProcessor)
                         && player->resourceUnits > 1000
                         && !aisTeams[AIS_TEAM_PROCESSOR].totalteamsize)
                {
                    aisBuildShip(player, bProcessor, 1000);
                }
            }
            else
            {
                if (rmCanBuildShip(player, sCarrier) && aisTeams[AIS_TEAM_CARRIER].totalteamsize < 2)
                {
                    aisBuildShip(player, sCarrier, 2000);
                }
                else if (rmCanBuildShip(player, sDreadnought) && player->resourceUnits > 2000)
                {
                    aisBuildShip(player, sDreadnought, 3000);
                }
                else if (rmCanBuildShip(player, sDestroyer) && player->resourceUnits > 1000)
                {
                    aisBuildShip(player, sDestroyer, 1500);
                }
                else if (rmCanBuildShip(player, sProcessor)
                         && player->resourceUnits > 1000
                         && !aisTeams[AIS_TEAM_PROCESSOR].totalteamsize)
                {
                    aisBuildShip(player, sProcessor, 1000);
                }
            }
        }
    }

    if (aiCarrier1 != NULL
        && aiCarrier1->playerowner->race == RACE_Sect
        && aisTeams[AIS_TEAM_CARRIER1_SUPPORT].totalteamsize < 6
        && !aiHasCarrier1ModuleQueued
        && rmCanBuildShip(player, sCarrierSupport))
    {
        char creatorSummary[96];

        (void)creatorSummary;
        aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), aiCarrier1);
        aisParityLog("event=order kind=create_ship shipType=%d race=%d player=%u creator_%s", (int)sCarrierSupport, (int)player->race, (unsigned int)player->playerIndex, creatorSummary);
        clWrapCreateShip(
            &universe.mainCommandLayer,
            sCarrierSupport,
            player->race,
            player->playerIndex,
            aiCarrier1);
    }

    if (aiCarrier2 != NULL
        && aiCarrier2->playerowner->race == RACE_Sect
        && aisTeams[AIS_TEAM_CARRIER2_SUPPORT].totalteamsize < 6
        && !aiHasCarrier2ModuleQueued
        && rmCanBuildShip(player, sCarrierSupport))
    {
        char creatorSummary[96];

        (void)creatorSummary;
        aisParityFormatShipSummary(creatorSummary, sizeof(creatorSummary), aiCarrier2);
        aisParityLog("event=order kind=create_ship shipType=%d race=%d player=%u creator_%s", (int)sCarrierSupport, (int)player->race, (unsigned int)player->playerIndex, creatorSummary);
        clWrapCreateShip(
            &universe.mainCommandLayer,
            sCarrierSupport,
            player->race,
            player->playerIndex,
            aiCarrier2);
    }

    workerCount = aisTeams[AIS_TEAM_WORKER].totalteamsize
        + aiBuildingShip[workerType]
        + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], (ShipType)workerType);
    workerTargetCount = aisDifficultyTarget(difficultyLevel, 5, 8, 12);
    if (player->resourceUnits > 50000)
    {
        workerTargetCount = aisDifficultyTarget(difficultyLevel, 2, 4, 6);
    }
    if (workerCount < workerTargetCount)
    {
        aisBuildShip(player, workerType, 0);
    }

    scoutTargetCount = aisDifficultyTarget(difficultyLevel, 5, 8, 12);
    if (aisTeams[AIS_TEAM_SCOUT_ENEMY].totalteamsize < scoutTargetCount)
    {
        scoutTargetCount = aisTeams[AIS_TEAM_SCOUT_ENEMY].totalteamsize;
    }
    scoutCount = aisTeams[AIS_TEAM_SCOUT].totalteamsize
        + aiBuildingShip[scoutType]
        + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], (ShipType)scoutType);
    if (scoutCount < scoutTargetCount)
    {
        aisBuildShip(player, scoutType, 50);
    }

    if (player->race == RACE_Beast)
    {
        sdword dfgFrigateCount;
        sdword dfgFrigateTargetCount;

        dfgFrigateTargetCount = aisDifficultyTarget(difficultyLevel, 0, 1, 2);
        dfgFrigateCount = aisTeams[AIS_TEAM_DFG_FRIGATE].totalteamsize
            + aiBuildingShip[bDFGFrigate]
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bDFGFrigate);
        if (dfgFrigateCount < dfgFrigateTargetCount)
        {
            aisBuildShip(player, bDFGFrigate, 2000);
        }
    }

    if (player->race == RACE_Sect)
    {
        sdword leechCount;
        sdword leechTargetCount;
        sdword mimicCount;
        sdword mimicTargetCount;

        leechTargetCount = aisDifficultyTarget(difficultyLevel, 0, 10, 20);
        leechCount = aisTeams[AIS_TEAM_LEECH].totalteamsize
            + aiBuildingShip[sLeech]
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], sLeech);
        if (leechCount < leechTargetCount)
        {
            aisBuildShip(player, sLeech, 100);
        }

        mimicTargetCount = aisDifficultyTarget(difficultyLevel, 0, 0, 20);
        mimicCount = aisTeams[AIS_TEAM_MIMIC].totalteamsize
            + aiBuildingShip[sMimic]
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], sMimic);
        if (mimicCount < mimicTargetCount)
        {
            aisBuildShip(player, sMimic, 150);
            aisBuildShip(player, sMCV, 300);
        }
    }

    if (player->race == RACE_Beast)
    {
        sdword cruiseMissileCount;
        sdword cruiseMissileTargetCount;

        cruiseMissileTargetCount = aisDifficultyTarget(difficultyLevel, 0, 0, 10);
        cruiseMissileCount = aisTeams[AIS_TEAM_CRUISE_MISSILE].totalteamsize
            + aiBuildingShip[bCruiseMissile]
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bCruiseMissile);
        if (cruiseMissileCount < cruiseMissileTargetCount)
        {
            aisBuildShip(player, bCruiseMissile, 400);
        }
    }

    if (aisTeams[AIS_TEAM_FIGHTER_ENEMY].totalteamsize > 15)
    {
        if (player->race == RACE_Beast)
        {
            aisBuildShip(player, bMultiBeamFrigate, 800);
            aisBuildShip(player, bMultiGunCorvette, 400);
        }
        else
        {
            aisBuildShip(player, sMultiBeamFrigate, 800);
        }
        goto build_fighters;
    }

    couldNotBuildMedium = FALSE;
    if (player->race == RACE_Beast)
    {
        udword mediumRoll;

        if (!rmCanBuildShip(player, bHeavyCruiser) || aiHasExternalConstruction)
        {
            mediumRoll = ranRandom(RAN_AIPlayer);
            aisParityLog("event=random site=fleet_update_beast_medium_roll value=%u", (unsigned int)mediumRoll);
            switch (mediumRoll % 7)
            {
                case 0:
                    mediumShipToBuild = bHeavyCorvette;
                    break;
                case 1:
                    mediumShipToBuild = bHiveFrigate;
                    break;
                case 2:
                case 3:
                    mediumShipToBuild = bIonArrayFrigate;
                    break;
                case 4:
                    mediumShipToBuild = bRammingFrigate;
                    break;
                case 5:
                    mediumShipToBuild = bCruiseMissile;
                    break;
                default:
                    mediumShipToBuild = bMissileCorvette;
                    break;
            }
            aisBuildShip(
                player,
                mediumShipToBuild,
                GetShipStaticInfo((ShipType)mediumShipToBuild, player->race)->buildCost);
        }
        else
        {
            couldNotBuildMedium = TRUE;
        }
    }
    else if ((!rmCanBuildShip(player, sDreadnought) && !rmCanBuildShip(player, sDestroyer))
             || aiHasExternalConstruction)
    {
        udword mediumRoll;

        mediumRoll = ranRandom(RAN_AIPlayer);
        aisParityLog("event=random site=fleet_update_sect_medium_roll value=%u", (unsigned int)mediumRoll);
        if ((mediumRoll % 6) == 0)
        {
            mediumShipToBuild = sRammingFrigate;
        }
        else
        {
            mediumShipToBuild = sHiveFrigate;
        }
        aisBuildShip(
            player,
            mediumShipToBuild,
            GetShipStaticInfo((ShipType)mediumShipToBuild, player->race)->buildCost);
    }
    else
    {
        couldNotBuildMedium = TRUE;
    }

    if ((player->supportUnitsUsed + aiSupportUnitsPending + 50) <= player->supportUnitsMax && !couldNotBuildMedium)
    {
        goto build_fighters;
    }

    if (aisTeams[AIS_TEAM_FIGHTER].numselections > 0)
    {
        MaxSelection fighterSelection;

        memset(&fighterSelection, 0, sizeof(fighterSelection));
        aisAppendTeamSelection(&fighterSelection, &aisTeams[AIS_TEAM_FIGHTER]);
        if (fighterSelection.numShips > 0)
        {
            if (difficultyLevel >= AI_ADV)
            {
                aisParityLogOrder("set_kamikaze", (SelectCommand *)&fighterSelection, "");
                clWrapSetKamikaze(&universe.mainCommandLayer, (SelectCommand *)&fighterSelection);
            }
            else
            {
                aisParityLogOrder("scuttle", (SelectCommand *)&fighterSelection, "");
                clWrapScuttle(&universe.mainCommandLayer, (SelectCommand *)&fighterSelection);
                aisParityLogOrder("scuttle", (SelectCommand *)&fighterSelection, "");
                clWrapScuttle(&universe.mainCommandLayer, (SelectCommand *)&fighterSelection);
            }
        }
    }
    return;

build_fighters:
    fighterTargetCount = aisDifficultyTarget(difficultyLevel, 25, 35, 60);
    if (aisTeams[AIS_TEAM_FIGHTER].totalteamsize >= fighterTargetCount)
    {
        return;
    }

    if (player->race == RACE_Beast)
    {
        fighterCount = aisTeams[AIS_TEAM_FIGHTER].totalteamsize
            + aiBuildingShip[bAcolyte]
            + aiBuildingShip[bInterceptor]
            + aiBuildingShip[bCloakedFighter]
            + aiBuildingShip[bAttackBomber]
            + aiBuildingShip[bACV]
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bAcolyte)
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bInterceptor)
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bCloakedFighter)
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bAttackBomber)
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], bACV);
        if (fighterCount < fighterTargetCount)
        {
            aisBuildShip(player, bInterceptor, 100);
            if (difficultyLevel > AI_BEG)
            {
                aisBuildShip(player, bAttackBomber, 150);
                aisBuildShip(player, bCloakedFighter, 150);
            }
            if (difficultyLevel > AI_INT)
            {
                aisBuildShip(player, bAcolyte, 150);
                aisBuildShip(player, bACV, 300);
            }
        }
    }
    else
    {
        fighterCount = aisTeams[AIS_TEAM_FIGHTER].totalteamsize
            + aiBuildingShip[sAcolyte]
            + aiBuildingShip[sACV]
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], sAcolyte)
            + aisCountTeamShipType(&aisTeams[AIS_TEAM_EXTERNAL_BUILD], sACV);
        if (fighterCount < fighterTargetCount)
        {
            aisBuildShip(player, sAcolyte, 150);
            if (difficultyLevel > AI_BEG)
            {
                aisBuildShip(player, sACV, 300);
            }
        }
    }
}
