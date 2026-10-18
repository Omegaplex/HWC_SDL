/*=============================================================================
    Name    : univupdate
    Purpose : updates the universe, main game loop

    Created 10/4/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "qpcfix.h"
#include "glinc.h"

#include "types.h"
#include "debug.h"
#include "matrix.h"
#include "fastmath.h"
#include "linkedlist.h"
#include "objtypes.h"
#include "spaceobj.h"
#include "memory.h"
#include "collision.h"
#include "physics.h"

#include "aitrack.h"
#include "aiship.h"
#include "commandlayer.h"
#include "universe.h"
#include "select.h"
#include "statscript.h"
#include "soundevent.h"
#include "CrashLog.h"
#include "color.h"

#include "globals.h"
#include "flightman.h"
#include "univupdate.h"
#include "clamp.h"
#include "DFGFrigate.h"
#include "Randy.h"
#include "RammingFrigate.h"

#include "ships.h"
#include "dock.h"
#include "Mothership.h"
#include "ProximitySensor.h"
#include "Ship.h"
#include "LOD.h"
#include "mainrgn.h"
#include "pieplate.h"
#include "ScenPick.h"
#include "rescollect.h"

extern sdword cataShipTypeCounts[MAX_MULTIPLAYER_PLAYERS * TOTAL_NUM_SHIPS];
extern sdword cataReplacementShipTypeCounts[MAX_MULTIPLAYER_PLAYERS * TOTAL_NUM_SHIPS];
extern sdword enableRepairbots;
sdword univForceShipUpgradeRefresh;

sdword shipGetSlotStatus(Ship *ship, sdword slotIndex);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
void *shipGetBeastWeaponInfo(Ship *ship);
bool32 shipHasReadySwarmers(Ship *ship);
bool32 shipIsBeastShip(Ship *ship);
void CrystalResetProcessingState(Ship *crystal);
void aisParityLogChannel(const char *channel, const char *format, ...);
void soundEventShipRemoveHandles(Ship *ship);
static void univCreateClampedChildren(Ship *ship, ShipRace shiprace, vector *shippos, Player *playerowner, sdword built);

#include "sensors.h"
#include "file.h"

#include "netcheck.h"
#include "commandwrap.h"
#include "SalCapCorvette.h"

void cmUntrackExternalBuildShip(Ship *ship);
void dockWorkerClampFunc(Ship *ship);

static bool32 univShipUsesBigDeathEffect(ShipType shiptype, sdword *bigDeathLength, real32 *bigDeathPercent)
{
    switch (shiptype)
    {
        case sMothershipWeapons:
        case sMothershipDockingBay:
        case sMothershipEnginesUpper:
        case bMothershipDockingBay:
        case bMothershipEnginesLower:
        case trFloatingCity:
        case trGhostship:
            *bigDeathLength = 3;
            *bigDeathPercent = 0.75f;
            return TRUE;

        case sDreadnoughtCannon:
        case sMothership:
        case sMothershipOreCanArmour:
        case bMothership:
        case bSectMoship:
            *bigDeathLength = 2;
            *bigDeathPercent = 0.5f;
            return TRUE;

        case sCarrierSupport:
        case sMothershipSpecial:
        case sMothershipEngineering:
        case sMothershipArmour:
        case sMothershipMicro:
        case sMothershipSupport:
        case sMothershipCommand:
        case sMothershipEnginesLower:
        case sMothershipLowerSpine:
        case sMothershipBigGun:
        case bMothershipSupport:
            *bigDeathLength = 1;
            *bigDeathPercent = 0.4f;
            return TRUE;

        default:
            return FALSE;
    }
}

static bool32 univBulletUsesSiegeShockwave(const Bullet *bullet)
{
    return bullet != NULL && bullet->soundType == GS_BigGun;
}

#include "singleplayer.h"
#include "utility.h"
#include "singleplayer.h"
#include "nis.h"
#include "mex.h"
#include "mouse.h"
#include "aiplayer.h"
#include "clouds.h"
#include "nebulae.h"
#include "blobs.h"
#include "strings.h"
#include "researchapi.h"
#include "tactics.h"
#include "infooverlay.h"
#include "multiplayergame.h"
#include "meshanim.h"
#include "alliance.h"
#include "damage.h"
#include "madLinkIn.h"
#include "madLinkInDefs.h"
#include "clamp.h"
#include "trails.h"

#include "levelload.h"

#include "attributes.h"
#include "ping.h"
#include "teams.h"
#include "bounties.h"
#include "tutor.h"
#include "trademgr.h"
#include "Timer.h"
#include "crates.h"
#include "SaveGame.h"
#include "battle.h"
#include "BattleChat.h"
#include "randy.h"
#include "hs.h"

#include "launchmgr.h"
#include "ResearchGUI.h"
#include "ProfileTimers.h"

#include "consMgr.h"
#include "RepairDroid.h"
#include "star3d.h"
#include "KAS.h"
#include "AITeam.h"
#include "GamePick.h"
#include "Options.h"

bool32 CrystalUpdateProcessingVisuals(Ship *crystal);
sdword GetRaceForShipTypeRange(udword shipType);
void telldir(void);
BOOL IsShipInfected(Ship *ship);
sdword aiSelectRandomShipType(sdword shipType, sdword shipRace);
real32 TimerTimeRemaining(void *timer);
sdword playerGetSensorCoverage(Player *player);
real32 univGetMaxHealth(Ship *ship, bool32 isEnemy);
void univUpdateShipAITick(Ship *ship);
void fogRemoveSphereFromGrid(real32 x, real32 y, real32 z, real32 radius);
sdword mrClearDeadShipReference(Ship *ship);
bool32 smOverlayRemoveTargetFromSelections(TargetPtr removeTargetPtr);
void univActivateAllShips(void);

#define PLAYER_SENSOR_COVERAGE_OFFSET 25856

static real32 univAutosaveTimer = 0.0f;
static sdword cataOverlayPauseCountdown = -1;
static real32 cataMultiplayerAutosaveTime = 0.0f;
static sdword cataMultiplayerAutosaveSlot = 0;
static sdword univWipeShipDirectFreeOverride = 0;


#define CATA_MULTIPLAYER_AUTOSAVE_INTERVAL_SECONDS 200.0f

void univUpdateFormationSpeeds(void)
{
    Node *node;

    for (node = universe.mainCommandLayer.todolist.head; node != NULL; node = node->next)
    {
        CommandToDo *todo = (CommandToDo *)listGetStructOfNode(node);
        real32 minSpeed;
        SelectCommand *selection;
        sdword i;

        if ((todo->ordertype.attributes & COMMAND_IS_PROTECTING) == 0)
        {
            continue;
        }

        minSpeed = GetShipsTravelVel(todo->selection, 1.0f);
        selection = todo->selection;
        for (i = 0; i < selection->numShips; i++)
        {
            Ship *ship = selection->ShipPtr[i];

            if (ship->dockvars.dockship != NULL)
            {
                continue;
            }
            if (singlePlayerGame)
            {
                if (singlePlayerGameInfo.currentMission == MISSION_8_THE_CATHEDRAL_OF_KADESH &&
                    ship->shiptype == tHeavyInterceptor)
                {
                    continue;
                }
                if (singlePlayerGameInfo.currentMission == MISSION_9_SEA_OF_LOST_SOULS &&
                    ship->shiptype == tResourceCollector)
                {
                    continue;
                }
            }

            if (salCapCantDoFormation(ship))
            {
                Ship *salvTarget = *(Ship **)(SalvagerFunc(ship) + 4);

                if (salvTarget != NULL &&
                    GetDistance(&ship->posinfo.position, &salvTarget->posinfo.position) < 750.0f)
                {
                    continue;
                }
            }

            if (ship->speedThrottle < 0.0f)
            {
                ship->speedThrottle = REALlyBig;
            }
            if (minSpeed < ship->speedThrottle)
            {
                ship->speedThrottle = minSpeed;
            }
        }
    }
}

sdword univPingShipDied(sdword shipAddress)
{
    Ship *ship = (Ship *)shipAddress;
    sdword sensorCoverage;
    sdword savedSensorCoverage;

    if (!multiPlayerGame)
    {
        sensorCoverage = *(sdword *)((ubyte *)ship->playerowner + PLAYER_SENSOR_COVERAGE_OFFSET);
    }
    else if (allianceArePlayersAllied(universe.curPlayerPtr, ship->playerowner))
    {
        sensorCoverage = *(sdword *)((ubyte *)universe.curPlayerPtr + PLAYER_SENSOR_COVERAGE_OFFSET);
    }
    else
    {
        sensorCoverage = *(sdword *)((ubyte *)ship->playerowner + PLAYER_SENSOR_COVERAGE_OFFSET);
    }

    if (sensorCoverage == 0)
    {
        return 0;
    }

    savedSensorCoverage = univSetCurrentSensorCoverage(sensorCoverage);
    if (physShipIsMovable(ship))
    {
        real32 sensorRadius = univGetMaxHealth(ship, FALSE);

        fogRemoveSphereFromGrid(
            ship->posinfo.position.x,
            ship->posinfo.position.y,
            ship->posinfo.position.z,
            sensorRadius);
    }
    return univSetCurrentSensorCoverage(savedSensorCoverage);
}

extern LONGLONG utyTimerDivisor;
extern real32 taskFrequency;

static LONGLONG realtimeremainder = -1;

#define UPDATE_AI_RUNTIME_FLAGS(ship)            ((ship)->specialFlags2)
#define UPDATE_AI_BUILD_PROGRESS_FLAG            BIT19
#define UPDATE_AI_UPGRADE_CHECK_MASK             0xFF
#define UPDATE_AI_SPECIAL_CHECK_MASK             0x1F

static SpecialCommand *univUpdateShipAITickCopySpecialTargets(SelectAnyCommand *targets)
{
    SpecialCommand *copy;

    copy = memAlloc(sizeofSelectCommand(targets->numTargets), "AITickSpecialTargets", Volatile);
    copy->numTargets = targets->numTargets;
    copy->timeLastStatus = 0.0f;
    memcpy(copy->TargetPtr, targets->TargetPtr, sizeof(TargetPtr) * targets->numTargets);
    return copy;
}

static AttackCommand *univUpdateShipAITickMergeAttackTargets(
    Ship *ship,
    CommandToDo *command,
    SelectAnyCommand *nearbyTargets,
    bool32 restrictToPriorityPlayers)
{
    AttackCommand *mergedTargets;
    sdword existingTargetCount = 0;
    sdword mergedCount = 0;
    sdword i;

    if (command != NULL && command->ordertype.order == COMMAND_ATTACK && command->attack != NULL)
    {
        existingTargetCount = command->attack->numTargets;
    }

    mergedTargets = memAlloc(
        sizeofSelectCommand(existingTargetCount + nearbyTargets->numTargets),
        "AITickAttack",
        Volatile);
    mergedTargets->numTargets = 0;
    mergedTargets->timeLastStatus = 0.0f;

    for (i = 0; i < existingTargetCount; i++)
    {
        mergedTargets->TargetPtr[mergedCount++] = command->attack->TargetPtr[i];
    }

    for (i = 0; i < nearbyTargets->numTargets; i++)
    {
        Ship *target = (Ship *)nearbyTargets->TargetPtr[i];
        bool32 allowTarget;
        sdword j;

        if (restrictToPriorityPlayers)
        {
            Player *targetPlayer = target->playerowner;
            allowTarget =
                targetPlayer == ship->playerowner->aiPlayer->primaryEnemyPlayer ||
                (ship->gettingrocked != NULL && targetPlayer == ship->gettingrocked->playerowner);
        }
        else
        {
            allowTarget = TRUE;
        }

        if (!allowTarget)
        {
            continue;
        }

        for (j = 0; j < existingTargetCount; j++)
        {
            if ((Ship *)command->attack->TargetPtr[j] == target)
            {
                break;
            }
        }
        if (j == existingTargetCount)
        {
            mergedTargets->TargetPtr[mergedCount++] = nearbyTargets->TargetPtr[i];
        }
    }

    mergedTargets->numTargets = mergedCount;
    return mergedTargets;
}

void univUpdateAttachedObjRotInfo(SpaceObjRot *attachedObjectRot);
void univUpdateShipHousekeeping(Ship *ship);
void clSetAttribute(sdword ship, sdword value, sdword attribute);
void univShipHouseKeep(Ship *ship);
void seekdir(void);
void shipUpdateFreezeState(Ship *ship);
void univResetMultiplayerAutosaveState(void);

real64 cat_TimerFunc2(void)
{
    LONGLONG difference;
    LARGE_INTEGER performanceCount;

    catQueryPerformanceCounter(&performanceCount);
    if (realtimeremainder == -1)
    {
        realtimeremainder = performanceCount.QuadPart - performanceCount.QuadPart % utyTimerDivisor;
        return 0.0;
    }
    difference = performanceCount.QuadPart - realtimeremainder;
    realtimeremainder = performanceCount.QuadPart - difference % utyTimerDivisor;
    return (real64)(udword)(difference / utyTimerDivisor) / taskFrequency;
}

static bool32 cataReplacementUnlocksNewTech(Player *sourcePlayer, ShipType sourceShipType, Player *playerowner, ShipType replacementShipType)
{
    PlayerShipPrereqMask *sourceDiscoveredTable = sourcePlayer->discoveredShipPrereqs;
    PlayerShipPrereqMask *sourceReferenceTable = sourcePlayer->referenceShipPrereqs;
    sdword *currentPrereqMask = rmPlayerCurrentPrereqMask(playerowner);
    sdword mergedPrereqs[42];

    if (sourceDiscoveredTable == NULL)
    {
        return FALSE;
    }

    rmPreRequisitesAnd(
        mergedPrereqs,
        sourceDiscoveredTable[sourceShipType].value,
        currentPrereqMask);
    if (!rmTechDependenciesEqual(mergedPrereqs, sourceDiscoveredTable[sourceShipType].value))
    {
        return TRUE;
    }

    rmPreRequisitesAnd(
        mergedPrereqs,
        sourceReferenceTable[sourceShipType].value,
        currentPrereqMask);
    return !rmTechDependenciesEqual(mergedPrereqs, sourceReferenceTable[sourceShipType].value);
}

sdword univProcessDelayedShipDeath(sdword shipAddress)
{
    Ship *ship = (Ship *)shipAddress;
    Player *replacementOwner;
    Ship *replacementShip;
    ShipType replacementType = bWorker;
    void *beastWeaponInfo;

    if (ship->statusWindow.activeUntil == 0.0f || nisIsRunning_)
    {
        return FALSE;
    }

    if (ship->command != NULL)
    {
        SelectCommand selection;

        memset(&selection, 0, sizeof(selection));
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selection);
    }

    if (universe.totaltimeelapsed < ship->statusWindow.activeUntil)
    {
        return FALSE;
    }

    beastWeaponInfo = shipGetBeastWeaponInfo(ship);
    if (beastWeaponInfo != NULL)
    {
        replacementType = ship->staticinfo->beastAttackConvertTo;
    }

    replacementOwner = ship->beastAttack.sourcePlayer;
    dbgAssert(replacementOwner != NULL);

    ship->howDidIDie = DEATH_Killed_By_Player;
    ship->whoKilledMe = replacementOwner->playerIndex;
    replacementShip = univSpawnReplacementShip(shipAddress, replacementType, replacementOwner);
    if (replacementShip != NULL)
    {
        replacementShip->specialFlags4 |= SPECIAL4_Unknown17;
    }
    return TRUE;
}

// Cataclysm-specific: convert a captured ship into its replacement hull while preserving runtime state.
Ship *univSpawnReplacementShip(sdword shipAddress, ShipType shiptype, Player *playerowner)
{
    Ship *sourceShip = (Ship *)shipAddress;
    Ship *newShip = NULL;
    Player *sourcePlayer = sourceShip->playerowner;
    ShipType sourceShipType = sourceShip->shiptype;
    udword savedSourceShipNumber = sourceShip->shipID.shipNumber;
    vector savedPosition = sourceShip->posinfo.position;
    vector savedVelocity = sourceShip->posinfo.velocity;
    vector savedForce = sourceShip->posinfo.force;
    matrix savedCoordsys = sourceShip->rotinfo.coordsys;
    real32 savedHealth = sourceShip->health;
    bool16 savedIsMoving = sourceShip->posinfo.isMoving;
    sdword savedResources = sourceShip->resources;
    sdword savedUpgradeLevel = sourceShip->upgradeState.level;
    udword savedUpgradeMask = sourceShip->upgradeState.mask;
    Ship *savedSpecialTarget = sourceShip->specialTargetShip;
    ShipRace shipRace;
    ShipUpgradeSlotStateBlock *sourceSlotStateBlock;
    ShipUpgradeSlotStateBlock *newSlotStateBlock;

    sourceShip->statusWindow.activeUntil = 0.0f;
    if (sourceShip->beastAttack.effect != NULL)
    {
        memFree(sourceShip->beastAttack.effect);
        sourceShip->beastAttack.effect = NULL;
    }
    if (sourceShip->replacementEffect != NULL)
    {
        memFree(sourceShip->replacementEffect);
        sourceShip->replacementEffect = NULL;
    }
    sourceShip->replacementStateActive = FALSE;

    if (!cmCanSupportShipType(shiptype, playerowner) && sourceShip->specialTargetShip == NULL)
    {
        if (playerowner == universe.curPlayerPtr)
        {
            clCommandMessage(strGetString(strNotEnoughSupportUnitsToConvert), -16776961);
        }
        ApplyDamageToTarget((SpaceObjRotImpTarg *)sourceShip, 5000.0f, 19, 666, playerowner->playerIndex);
        sourceShip->statusWindow.activeUntil = 0.0f;
        sourceShip->flags &= 0xFF5FFFFF;
        return NULL;
    }

    shipRace = (ShipRace)GetRaceForShipTypeRange(shiptype);
    newShip = univCreateShip(shiptype, shipRace, &savedPosition, playerowner, 0);

    sourceSlotStateBlock = (ShipUpgradeSlotStateBlock *)sourceShip->unk_490;
    newSlotStateBlock = (ShipUpgradeSlotStateBlock *)newShip->unk_490;
    if (sourceSlotStateBlock != NULL && newSlotStateBlock != NULL)
    {
        sdword i;
        sdword count = newSlotStateBlock->count;

        if (count > sourceSlotStateBlock->count)
        {
            count = sourceSlotStateBlock->count;
        }
        for (i = 0; i < count; i++)
        {
            newSlotStateBlock->entries[i].state = sourceSlotStateBlock->entries[i].state;
        }
    }

    if (sourceShip->renderlink.belongto != NULL)
    {
        udword flags = newShip->flags;
        if ((flags & SOF_DockMarker) == 0)
        {
            if ((flags & 0x4000000) != 0)
            {
                univAddObjToRenderList((SpaceObj *)newShip);
            }
            else
            {
                newShip->cameraDistanceVector = sourceShip->cameraDistanceVector;
                newShip->cameraDistanceSquared = sourceShip->cameraDistanceSquared;
                listAddNodeAfter(&sourceShip->renderlink, &newShip->renderlink, newShip);
            }
        }
    }

    cat_unitCapCreateShip(newShip, newShip->playerowner);
    if (newShip->playerowner->discoveredShipPrereqs != NULL &&
        sourcePlayer->discoveredShipPrereqs != NULL)
    {
        memcpy(
            &newShip->playerowner->discoveredShipPrereqs[shiptype],
            &sourcePlayer->discoveredShipPrereqs[sourceShipType],
            sizeof(PlayerShipPrereqMask));
    }

    ShipTypeToStr(sourceShipType);
    telldir();
    univDeleteDeadShip(sourceShip, -1);
    if (sourceShip->renderlink.belongto != NULL)
    {
        listRemoveNode(&sourceShip->renderlink);
    }
    univWipeShipOutOfExistence(sourceShip);

    newShip->posinfo.velocity = savedVelocity;
    newShip->posinfo.force = savedForce;
    newShip->rotinfo.coordsys = savedCoordsys;
    newShip->health = savedHealth;
    newShip->posinfo.isMoving = savedIsMoving;
    if (newShip->health > newShip->staticinfo->maxhealth)
    {
        newShip->health = newShip->staticinfo->maxhealth;
    }
    newShip->lastTimeRepaired = 0.0f;
    newShip->resources = savedResources;
    if (newShip->resources > newShip->staticinfo->maxresources)
    {
        newShip->resources = newShip->staticinfo->maxresources;
    }
    newShip->upgradeState.level = savedUpgradeLevel;
    newShip->upgradeState.mask = savedUpgradeMask;
    univUpdateObjRotInfo((SpaceObjRot *)newShip);

    if (playerowner->race != RACE_Beast)
    {
        return newShip;
    }

    if (savedSpecialTarget == NULL || savedSpecialTarget->shiprace == RACE_Beast)
    {
        newShip->statusWindow.state = 2;
        newShip->statusWindow.phaseStartTime = universe.totaltimeelapsed + 0.5f;
        newShip->statusWindow.phaseEndTime = newShip->statusWindow.phaseStartTime + 0.5f;
    }

    if (singlePlayerGame)
    {
        return newShip;
    }

    if (playerowner->aiPlayer != NULL && newShip->shiptype != bProcessor)
    {
        SelectCommand selectcom;

        memset(&selectcom, 0, sizeof(selectcom));
        selectcom.ShipPtr[0] = newShip;
        selectcom.numShips = 1;
        universe.aiplayerProcessing = 1;
        if ((ranRandom(6) % 100) + 1 >= 10)
        {
            clWrapDock(&universe.mainCommandLayer, &selectcom, 0, 0);
        }
        else
        {
            clWrapDock(&universe.mainCommandLayer, &selectcom, 7, 0);
        }
        universe.aiplayerProcessing = 0;
        return newShip;
    }

    if (universe.curPlayerPtr == playerowner)
    {
        if (newShip->shiptype == bProcessor)
        {
            rmRecomputeShipPrereqMask(newShip);
            if (cataReplacementUnlocksNewTech(sourcePlayer, sourceShipType, playerowner, newShip->shiptype))
            {
                battleChatAddMessage(strGetString(strNewTechFound), -16711936, 1, (sdword)newShip);
            }
        }
        else if (cataReplacementUnlocksNewTech(sourcePlayer, sourceShipType, playerowner, newShip->shiptype))
        {
            battleChatAddMessage(strGetString(strNewTechFoundRetire), -16711936, 1, (sdword)newShip);
        }
    }

    return newShip;
}

void univNotifyShipDied(Ship *ship)
{
    Node *node;

    bitSet(ship->flags2, SOF2_NotifyShipDiedPending);

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *otherShip = (Ship *)listGetStructOfNode(node);
        void (*shipDiedFn)(Ship *, Ship *) = otherShip->staticinfo->custshipheader.CustShipNotifyShipDied;

        if (shipDiedFn != NULL)
        {
            shipDiedFn(otherShip, ship);
        }
    }

    for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
        Ship *otherShip = insideShip->ship;
        void (*shipDiedFn)(Ship *, Ship *) = otherShip->staticinfo->custshipheader.CustShipNotifyShipDied;

        if (shipDiedFn != NULL)
        {
            shipDiedFn(otherShip, ship);
        }
    }
}

vector defaultshipupvector = { 0.0f, 0.0f, 1.0f };
vector defaultshiprightvector = { 0.0f, -1.0f, 0.0f };
vector defaultshipheadingvector = { 1.0f, 0.0f, 0.0f };
matrix defaultshipmatrix = { 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

IDToPtrTable ShipIDToPtr;
IDToPtrTable ResourceIDToPtr;
IDToPtrTable DerelictIDToPtr;
IDToPtrTable MissileIDToPtr;

/*=============================================================================
    Private functions:
=============================================================================*/

void setSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *object);


bool32 ApplyDamageToTarget(SpaceObjRotImpTarg *target,real32 damagetaken,GunSoundType soundType,sdword damageHow,sdword playerIndex);
void MakeExplosionRockBumpables(Ship *ship,bool32 scuttle);
void pointExplosionInSpace(vector *position,real32 blastRadius,real32 maxDamage,sdword playerIndex);

void univCheckShipState(Ship *ship);
void univSetupShipForControl(Ship *ship);
bool32 shipIsNotDocked(Ship *ship, bool32 requireNotDocked);

static void trailSegAddDamage(DustCloud *dustcloud, sdword damagetaken, bool32 targetWasAlive)
{
    cloudSystem *system = (cloudSystem *)dustcloud->stub;

    if (targetWasAlive && system != NULL)
    {
        system->charge += ((real32)damagetaken / (real32)system->maxhealth) * 0.5f;
        if (system->charge > 1.0f)
        {
            bitSet(system->flags, LIGHTNING_DISCHARGING);
            system->charge = 1.0f;
        }
    }
}

static bool32 univShipCanRetreatForRepairs(Ship *ship, bool32 considerSingleShip)
{
    ShipStaticInfo *staticInfo = ship->staticinfo;

    if (staticInfo->shipclass == CLASS_NonCombat ||
        staticInfo->shipclass == CLASS_Mothership ||
        staticInfo->externalBuild)
    {
        return FALSE;
    }

    return !considerSingleShip || ship->cataBusyStateWindow.dockingship == NULL;
}

static bool32 univShipCanDockForRepairs(Ship *ship, bool32 considerSingleShip)
{
    if (ship->shiptype == sSwarmer)
    {
        return FALSE;
    }

    return shipIsNotDocked(ship, considerSingleShip);
}

static void univRetreatShipForRepairs(Ship *ship)
{
    SelectCommand selectcom;

    if (ship->flags2 != 0 ||
        !univShipCanDockForRepairs(ship, TRUE) ||
        !univShipCanRetreatForRepairs(ship, TRUE) ||
        ship->staticinfo->oneOverMaxHealth * ship->health >= 0.333f)
    {
        return;
    }

    if (ship->command != NULL &&
        ship->command->ordertype.order == COMMAND_DOCK &&
        univShipCanRetreatForRepairs(ship, TRUE) &&
        ship->staticinfo->oneOverMaxHealth * ship->health < 0.333f)
    {
        return;
    }

    memset(&selectcom, 0, sizeof(selectcom));
    selectcom.numShips = 1;
    selectcom.ShipPtr[0] = ship;
    clDock(&universe.mainCommandLayer, &selectcom, 0, 0);
}

/******************************************************************************
Private data
******************************************************************************/

bool32 gameIsEnding=FALSE;
GrowSelection ClampedShipList = {0};

/*=============================================================================
    Tweakables
=============================================================================*/

real32 FLY_INTO_WORLD_PERCENT_DIST = 0.7f;

udword NUM_STARS            = 800;
udword NUM_BIG_STARS        = 175;

color backgroundColor = colRGB(0, 25, 0);

real32 MAX_VELOCITY_TANGENT_FACTOR = 1.0f;
sdword MIN_COLLISION_DAMAGE = 5;

scriptEntry StarTweaks[] =
{
    { "NumStars",               scriptSetUdwordCB, &NUM_STARS },
    { "NumBigStars",            scriptSetUdwordCB, &NUM_BIG_STARS },
    { "starMaxColor",           scriptSetUdwordCB, &starMaxColor },
    { "starMinColor",           scriptSetUdwordCB, &starMinColor },
    { "starRedVariance",        scriptSetUdwordCB, &starRedVariance },
    { "starGreenVariance",        scriptSetUdwordCB, &starGreenVariance },
    { "starBlueVariance",        scriptSetUdwordCB, &starBlueVariance },
    { NULL, NULL, 0 }
};
scriptEntry GalaxyTweaks[] =
{
    { "BackgroundColor",        scriptSetRGBCB, &backgroundColor },
    { NULL, NULL, 0 }
};

void univUpdateObjRotInfo(SpaceObjRot *robj)
{
#define irobj ((SpaceObjRotImp *)robj)
#define shipobj ((Ship *)robj)

    if (bitTest(robj->flags,SOF_Impactable))
    {
        matMultiplyMatByVec(&irobj->collInfo.collOffset,&irobj->rotinfo.coordsys,&irobj->staticinfo->staticheader.staticCollInfo.collsphereoffset);
        vecAdd(irobj->collInfo.collPosition,irobj->posinfo.position,irobj->collInfo.collOffset);

        collUpdateCollRectangle(irobj);

        if (irobj->objtype == OBJ_ShipType)
        {
            matMultiplyMatByVec(&shipobj->engineOffset,&shipobj->rotinfo.coordsys,&((ShipStaticInfo *)shipobj->staticinfo)->engineNozzleOffset[0]);

            vecAdd(shipobj->enginePosition,shipobj->posinfo.position,shipobj->engineOffset);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRotateObjPitch
    Description : rotates an obj about its right vector (producing pitch), correctly
                  rotating collision information, etc.
    Inputs      : obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRotateObjPitch(SpaceObjRot *robj,real32 rot)
{
    matrix rotymat;
    matrix tempmat;

    matMakeRotAboutY(&rotymat,(real32)cos(rot),(real32)sin(rot));
    tempmat = robj->rotinfo.coordsys;
    matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotymat);

    univUpdateObjRotInfo(robj);
}

/*-----------------------------------------------------------------------------
    Name        : univRotateObjYaw
    Description : rotates an obj about its up vector (producing yaw), correctly
                  rotating collision information, etc.
    Inputs      : obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRotateObjYaw(SpaceObjRot *robj,real32 rot)
{
    matrix rotxmat;
    matrix tempmat;

    matMakeRotAboutX(&rotxmat,(real32)cos(rot),(real32)sin(rot));
    tempmat = robj->rotinfo.coordsys;
    matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotxmat);

    univUpdateObjRotInfo(robj);
}

/*-----------------------------------------------------------------------------
    Name        : univRotateObjRoll
    Description : rotates an obj about its heading vector (producing Roll), correctly
                  rotating collision information, etc.
    Inputs      : obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRotateObjRoll(SpaceObjRot *robj,real32 rot)
{
    matrix rotzmat;
    matrix tempmat;

    matMakeRotAboutZ(&rotzmat,(real32)cos(rot),(real32)sin(rot));
    tempmat = robj->rotinfo.coordsys;
    matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotzmat);

    univUpdateObjRotInfo(robj);
}

/*-----------------------------------------------------------------------------
    Name        : univInitSpaceObjPosRot
    Description : initializes the position and rotation info of a space object,
                  including orientation (heading), position
    Inputs      : position, randomOrientation flag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univInitSpaceObjPosRot(SpaceObj *obj,vector *position,bool32 randomOrientation)
{
    memset(&(obj->posinfo),0,sizeof(obj->posinfo));
    obj->posinfo.position = *position;
    obj->posinfo.isMoving = FALSE;
    obj->posinfo.haventCalculatedDist = TRUE;

    if (bitTest(obj->flags,SOF_Rotatable))
    {
#define robj ((SpaceObjRot *)obj)
        memset(&robj->rotinfo,0,sizeof(robj->rotinfo));
        robj->rotinfo.coordsys = defaultshipmatrix;

        if (randomOrientation)
        {
            real32 rotaboutx = frandom(2*PI);
            real32 rotabouty = frandom(2*PI);
            real32 rotaboutz = frandom(2*PI);
            matrix rotxmat;
            matrix rotymat;
            matrix rotzmat;
            matrix tempmat;

            matMakeRotAboutX(&rotxmat,(real32)cos(rotaboutx),(real32)sin(rotaboutx));
            matMakeRotAboutY(&rotymat,(real32)cos(rotabouty),(real32)sin(rotabouty));
            matMakeRotAboutZ(&rotzmat,(real32)cos(rotaboutz),(real32)sin(rotaboutz));

            tempmat = robj->rotinfo.coordsys;
            matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotxmat);
            matMultiplyMatByMat(&tempmat,&robj->rotinfo.coordsys,&rotymat);
            matMultiplyMatByMat(&robj->rotinfo.coordsys,&tempmat,&rotzmat);
        }

        univUpdateObjRotInfo((SpaceObjRot *)obj);
    }
}

#define IDTOPTR_GROWBATCH       250

void IDToPtrTableInit(IDToPtrTable *table)
{
    table->numEntries = 0;
    table->objptrs = NULL;
}

void IDToPtrTableClose(IDToPtrTable *table)
{
    if (table->objptrs)
    {
        memFree(table->objptrs);
        table->objptrs = NULL;
    }
    table->numEntries = 0;
}

void IDToPtrTableReset(IDToPtrTable *table)
{
    IDToPtrTableClose(table);
    IDToPtrTableInit(table);
}

void IDToPtrTableObjDied(IDToPtrTable *table,uword ID)
{
    if (ID >= table->numEntries)
    {
        return;
    }

    table->objptrs[ID] = NULL;      // remove reference
}

SpaceObjPtr IDToPtrTableIDToObj(IDToPtrTable *table,uword ID)
{
    if (ID >= table->numEntries)
    {
        return NULL;
    }

    return table->objptrs[ID];
}

void IDToPtrTableAdd(IDToPtrTable *table,uword ID,SpaceObj *obj)
{
    dbgAssert(obj);

    if (ID >= table->numEntries)
    {
        sdword newnumber = table->numEntries+IDTOPTR_GROWBATCH;
#ifdef gshaw
        if (ID > (table->numEntries+1))
        {
            _asm int 3
        }
#endif

        if (table->objptrs)
        {
            table->objptrs = memRealloc(table->objptrs,sizeof(SpaceObjPtr) * newnumber,"idtoptrs",0);
        }
        else
        {
            table->objptrs = memAlloc(sizeof(SpaceObjPtr) * newnumber,"idtoptrs",0);
        }
        memset(&table->objptrs[table->numEntries],0,sizeof(SpaceObjPtr)*IDTOPTR_GROWBATCH);
        table->numEntries += IDTOPTR_GROWBATCH;
    }

    dbgAssert(ID < table->numEntries);

    if (table->objptrs[ID])
    {
        dbgFatalf(DBG_Loc,"Obj with this ID %d already exists",ID);
    }
    else
    {
        table->objptrs[ID] = obj;
    }
    return;
}

/*=============================================================================
    Save Game Stuff
=============================================================================*/
#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

typedef struct SaveIDToPtrTableStruct {
    sdword num;
    sdword ID[1];
} SaveIDToPtrTableStruct;

#define sizeofSaveIDToPtrTableStruct(n) (sizeof(SaveIDToPtrTableStruct) + (n-1)*sizeof(sdword))

void SaveIDToPtrTable(IDToPtrTable *table)
{
    sdword num = table->numEntries;
    SaveChunk *chunk;
    struct SaveIDToPtrTableStruct *savecontents;
    sdword i;

    chunk = CreateChunk(IDTOPTRTABLE,sizeofSaveIDToPtrTableStruct(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    for (i=0;i<num;i++)
    {
        savecontents->ID[i] = SpaceObjRegistryGetID(table->objptrs[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadIDToPtrTable(IDToPtrTable *table)
{
    SaveChunk *chunk;
    SaveIDToPtrTableStruct *loadcontents;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,IDTOPTRTABLE);

    loadcontents = (SaveIDToPtrTableStruct *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveIDToPtrTableStruct(num) == chunk->contentsSize);

    table->numEntries = num;
    if (num == 0)
    {
        table->objptrs = NULL;
    }
    else
    {
        table->objptrs = memAlloc(sizeof(SpaceObjPtr) * num,"idtoptrs",0);
    }

    for (i=0;i<num;i++)
    {
        table->objptrs[i] = SpaceObjRegistryGetObj(loadcontents->ID[i]);
}

    memFree(chunk);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"
/*=============================================================================
    Save Game Stuff End
=============================================================================*/

void univInitFastNetworkIDLookups(void)
{
    IDToPtrTableInit(&ShipIDToPtr);
    IDToPtrTableInit(&ResourceIDToPtr);
    IDToPtrTableInit(&DerelictIDToPtr);
    IDToPtrTableInit(&MissileIDToPtr);
}

void univCloseFastNetworkIDLookups(void)
{
    IDToPtrTableClose(&ShipIDToPtr);
    IDToPtrTableClose(&ResourceIDToPtr);
    IDToPtrTableClose(&DerelictIDToPtr);
    IDToPtrTableClose(&MissileIDToPtr);
}

void univResetFastNetworkIDLookups(void)
{
    IDToPtrTableReset(&ShipIDToPtr);
    IDToPtrTableReset(&ResourceIDToPtr);
    IDToPtrTableReset(&DerelictIDToPtr);
    IDToPtrTableReset(&MissileIDToPtr);
}

/*-----------------------------------------------------------------------------
    Name        : ShipIDtoShip
    Description : Converts a ShipID to a pointer to the actual ship
    Inputs      : shipID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
Ship *ShipIDtoShip(ShipID shipID,bool32 considerInsideShips)
{
    Ship *ship = (Ship *)IDToPtrTableIDToObj(&ShipIDToPtr,shipID.shipNumber);

    if (ship != NULL)
    {
        dbgAssert(ship->objtype == OBJ_ShipType);
        if (!considerInsideShips)
        {
            if (ship->flags & SOF_Hide)
            {
                ship = NULL;
            }
        }
    }

    return ship;
}

/*-----------------------------------------------------------------------------
    Name        : univFindShipIAmInside
    Description : returns the ship I am inside if I'm inside one, otherwise NULL
    Inputs      : me
    Outputs     :
    Return      : returns the ship I am inside if I'm inside one, otherwise NULL
----------------------------------------------------------------------------*/
Ship *univFindShipIAmInside(Ship *me)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->shipsInsideMe != NULL)
        {
            Node *insidenode = ship->shipsInsideMe->insideList.head;
            InsideShip *insideship;

            while (insidenode != NULL)
            {
                insideship = (InsideShip *)listGetStructOfNode(insidenode);
                dbgAssert(insideship->ship->objtype == OBJ_ShipType);

                if (insideship->ship == me)
                {
                    return ship;
                }

                insidenode = insidenode->next;
            }
        }

        objnode = objnode->next;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : univAmIInsideThisShip
    Description : returns TRUE if me ship is inside ship
    Inputs      : me,ship
    Outputs     :
    Return      : returns TRUE if me ship is inside ship
----------------------------------------------------------------------------*/
bool32 univAmIInsideThisShip(Ship *me,Ship *ship)
{
    if (ship->shipsInsideMe != NULL)
    {
        Node *insidenode = ship->shipsInsideMe->insideList.head;
        InsideShip *insideship;

        while (insidenode != NULL)
        {
            insideship = (InsideShip *)listGetStructOfNode(insidenode);
            dbgAssert(insideship->ship->objtype == OBJ_ShipType);

            if (insideship->ship == me)
            {
                return TRUE;
            }

            insidenode = insidenode->next;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : univFindShipContainingShipID
    Description : returns the ship containing shipID, or NULL if not docked inside one
    Inputs      : shipID
    Outputs     :
    Return      : containing ship or NULL
----------------------------------------------------------------------------*/
Ship *univFindShipContainingShipID(ShipID shipID)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (univShipContainsShipID(shipID, ship))
        {
            return ship;
        }

        objnode = objnode->next;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : univShipContainsShipID
    Description : returns TRUE if ship contains shipID
    Inputs      : shipID, ship
    Outputs     :
    Return      : TRUE if shipID is docked inside ship
----------------------------------------------------------------------------*/
bool32 univShipContainsShipID(ShipID shipID, Ship *ship)
{
    ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;

    if (shipsInsideMe != NULL)
    {
        Node *insidenode = shipsInsideMe->insideList.head;
        InsideShip *insideship;

        while (insidenode != NULL)
        {
            insideship = (InsideShip *)listGetStructOfNode(insidenode);
            dbgAssert(insideship->ship->objtype == OBJ_ShipType);

            if (insideship->ship->shipID.shipNumber == shipID.shipNumber)
            {
                return TRUE;
            }

            insidenode = insidenode->next;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : ResourceIDtoResourcePtr
    Description : Converts a resourceID to a pointer to the actual resource
    Inputs      : resourceID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
ResourcePtr ResourceIDtoResourcePtr(ResourceID resourceID)
{
    return (Resource *)IDToPtrTableIDToObj(&ResourceIDToPtr,resourceID.resourceNumber);
}

/*-----------------------------------------------------------------------------
    Name        : DerelictIDtoDerelictPtr
    Description : Converts a derelictID to a pointer to the actual derelict
    Inputs      : derelictID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
DerelictPtr DerelictIDToDerelictPtr(DerelictID derelictID)
{
    return (Derelict *)IDToPtrTableIDToObj(&DerelictIDToPtr,derelictID.derelictNumber);
}

/*-----------------------------------------------------------------------------
    Name        : MissileIDtoMissilePtr
    Description : Converts a missileID to a pointer to the actual missile
    Inputs      : missileID
    Outputs     :
    Return      :
    Notes       : Later optimize this routine (perhaps use a table look-up)
----------------------------------------------------------------------------*/
MissilePtr MissileIDtoMissilePtr(MissileID missileID)
{
    return (Missile *)IDToPtrTableIDToObj(&MissileIDToPtr,missileID.missileNumber);
}

/*-----------------------------------------------------------------------------
    Name        : univAddAsteroid
    Description : adds an asteroid to the mission sphere at position astpos
    Inputs      : astpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Asteroid *univAddAsteroid(AsteroidType asteroidtype,vector *astpos)
{
    Asteroid *newAsteroid;

    dbgAssert(asteroidtype < NUM_ASTEROIDTYPES);

    newAsteroid = memAlloc(sizeof(Asteroid),"Asteroid",NonVolatile);

    newAsteroid->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newAsteroid);
    universe.resourceNumber++;

    newAsteroid->objtype = OBJ_AsteroidType;
    newAsteroid->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource;
    if (asteroidtype == Asteroid0) newAsteroid->flags |= SOF_DontApplyPhysics;      // optimization for Asteroid0
    newAsteroid->staticinfo = (ResourceStaticInfo *)&asteroidStaticInfos[asteroidtype];
    ClearNode(newAsteroid->renderlink);
    newAsteroid->collMyBlob = NULL;
    newAsteroid->currentLOD = 0;
    newAsteroid->cameraDistanceSquared = REALlyBig;
    newAsteroid->flashtimer = 0;
    newAsteroid->lasttimecollided = 0;
    newAsteroid->attributes = 0;
    newAsteroid->attributesParam = 0;
    newAsteroid->resourcevalue = asteroidStaticInfos[asteroidtype].resourcevalue;
    newAsteroid->resourceNotAccessible = 0;
    newAsteroid->resourceVolume = NULL;
    newAsteroid->asteroidtype = asteroidtype;
    newAsteroid->scaling = 1.0f;
    newAsteroid->health = newAsteroid->staticinfo->maxhealth;
    newAsteroid->collInfo.precise = NULL;
    newAsteroid->cataRuntimeState2.linkedDerelict = NULL;

    vecZeroVector(newAsteroid->posinfo.force);
    vecZeroVector(newAsteroid->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj *)newAsteroid,astpos,TRUE);

    if (asteroidtype == Asteroid0)
    {
        newAsteroid->collMyBlob = NULL;
        listAddNode(&universe.MinorSpaceObjList,&(newAsteroid->objlink),newAsteroid);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)newAsteroid);
        listAddNode(&universe.SpaceObjList,&(newAsteroid->objlink),newAsteroid);
        listAddNode(&universe.ResourceList,&(newAsteroid->resourcelink),newAsteroid);
        listAddNode(&universe.ImpactableList,&(newAsteroid->impactablelink),newAsteroid);
        lodMeshBindingsAllocate((SpaceObj *)newAsteroid);
    }

    newAsteroid->posinfo.unk_2C = NULL;
    newAsteroid->posinfo.lodMeshBindings = NULL;

    return newAsteroid;
}

/*-----------------------------------------------------------------------------
    Name        : univAddDerelict
    Description : adds a derelict to the mission sphere at position pos
    Inputs      : pos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Derelict *univAddDerelict(DerelictType derelicttype,vector *pos)
{
    dbgAssert(derelicttype < NUM_DERELICTTYPES);
    return(univAddDerelictByStatInfo(derelicttype, (DerelictStaticInfo *)&derelictStaticInfos[derelicttype], pos));
}

Derelict *univAddHyperspaceGateAsDerelict(hvector *posAndRotation)
{
    Derelict *derelict = univAddDerelict(HyperspaceGate,(vector *)posAndRotation);
    univRotateObjYaw((SpaceObjRot *)derelict,posAndRotation->w);
    bitSet(derelict->flags,SOF_NotBumpable|SOF_DontApplyPhysics);
#ifdef gshaw
    //bitSet(derelict->flags,SOF_ForceVisible);
#endif
    return derelict;
}

/*-----------------------------------------------------------------------------
    Name        : univAddMissile
    Description : Adds a missile to the world.  This missile will have no
                    owner, no target and it's velocity, position and coordsys
                    must be set up by the owner.  Some missile this is!
    Inputs      : race - race of missile to create
    Outputs     :
    Return      : pointer to newly created missile
----------------------------------------------------------------------------*/
Missile *univAddMissile(ShipRace race)
{
    Missile *missile;
    sdword i;

    missile = memAlloc(sizeof(Missile),"Missile",0);
    memset(missile,0,sizeof(Missile));

    missile->objtype = OBJ_MissileType;
    missile->flags = SOF_Rotatable + SOF_Impactable + SOF_NotBumpable + SOF_Targetable;

    missile->enginePosition.x = 0.0f;
    missile->enginePosition.y = 0.0f;
    missile->enginePosition.z = 0.0f;
    missile->trail = NULL;

    missile->staticinfo = &missileStaticInfos[race];
    missile->maxtraveldist = missile->staticinfo->staticheader.maxvelocity * universe.unknown1;
    missile->missileType = MISSILE_Regular;
    missile->FORCE_DROPPED = FALSE;
    missile->formationinfo = NULL;
    missile->trail = mistrailNew(missile->staticinfo->trailStatic, missile);

    missile->health = ((MissileStaticInfo *)missile->staticinfo)->maxhealth;

    ClearNode(missile->renderlink);
    missile->currentLOD = 0;
    missile->cameraDistanceSquared = 1.0f;  //!!!
    missile->flashtimer = 0;
    missile->lasttimecollided = 0;
    missile->attributes = 0;
    missile->attributesParam = 0;

    vecZeroVector(missile->posinfo.force);
    vecZeroVector(missile->rotinfo.torque);

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.thruststrength[i] = missile->staticinfo->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
       missile->nonstatvars.rotstrength[i] = missile->staticinfo->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
       missile->nonstatvars.turnspeed[i] = missile->staticinfo->turnspeedstat[i];
    }

    missile->soundType = GS_MissleLauncher;
    missile->race = race;
    missile->timelived = 0.0f;
    missile->totallifetime = 12.26f;                        //!!! from the missile destroyer .ship file
    missile->playerowner = &universe.players[((race == universe.players[0].race)) ? 0 : 1];
    missile->owner = NULL;
    missile->target = NULL;

    missile->damage = 48.0f;                                //from the.shp file

    missile->missileID.missileNumber = universe.missileNumber;
    if (multiPlayerGame || (multiPlayerGameUnderWay && playPackets))
    {
        IDToPtrTableAdd(&MissileIDToPtr, universe.missileNumber, (SpaceObj *)missile);
    }
    universe.missileNumber++;

    //position, velocity and coordsys left blank
    missile->posinfo.isMoving = TRUE;
    missile->posinfo.haventCalculatedDist = TRUE;

    collAddSpaceObjToCollBlobs((SpaceObj *)missile);

    //proper hit effect
    missile->hitEffect = etgGunEventTable[race][GS_MissleLauncher][EGT_GunHit];//get pointer to hit effect

    listAddNode(&universe.SpaceObjList,&(missile->objlink),missile);
    listAddNode(&universe.ImpactableList,&(missile->impactablelink),missile);
    listAddNode(&universe.MissileList,&(missile->missilelink),missile);

    return(missile);
}

/*-----------------------------------------------------------------------------
    Name        : univAddToWorldList
    Description : Add a derelict to the universe's world list
    Inputs      : world - what object to add to the world list
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univAddToWorldList(Derelict *world)
{
    sdword index;

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        if (universe.world[index] == NULL)
        {
            universe.world[index] = world;
            return;
        }
    }
    dbgAssert(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveFromWorldList
    Description : Remove a derelict reference from the universe's world list
    Inputs      : world - what world we are to remove
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univRemoveFromWorldList(Derelict *world)
{
    sdword index, j;

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        if (universe.world[index] == world)
        {
            if (index < UNIV_NUMBER_WORLDS - 1)
            {
                for (j = UNIV_NUMBER_WORLDS - 1; j > index; j--)
                {
                    if (universe.world[j] != NULL)
                    {
                        universe.world[index] = universe.world[j];
                        universe.world[j] = NULL;
                        return;
                    }
                }
            }
            universe.world[index] = NULL;
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univAddDerelictByStatInfo
    Description : Add a derelict by direct staticinfo pointer rather than just
                    by type.  Mostly used for generic objects in NIS's.
    Inputs      : derelictType - type of derelict.  Can be outside range of 0..NUM_DERELICTTYPES
                  stat - static info for this derelict.
                  pos - where to create it
    Outputs     :
    Return      : Newly allocated derelict.
----------------------------------------------------------------------------*/
Derelict *univAddDerelictByStatInfo(DerelictType derelictType, DerelictStaticInfo *stat, vector *pos)
{
    Derelict *newDerelict;
    sdword i;

    newDerelict = memAlloc(sizeof(Derelict),"Derelict",NonVolatile);

    newDerelict->derelictID.derelictNumber = universe.derelictNumber;
    newDerelict->derelictID.unused = 0;
    IDToPtrTableAdd(&DerelictIDToPtr,universe.derelictNumber,(SpaceObj *)newDerelict);
    universe.derelictNumber++;

    newDerelict->objtype = OBJ_DerelictType;
    newDerelict->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable;
    newDerelict->flags2 = 0;
    newDerelict->unk_1C = 0;
    newDerelict->staticinfo = stat;
    ClearNode(newDerelict->renderlink);
    newDerelict->collMyBlob = NULL;
    newDerelict->renderedLODs = 0;
    newDerelict->currentLOD = 0;
    newDerelict->cameraDistanceSquared = REALlyBig;
    newDerelict->flashtimer = 0;
    newDerelict->lasttimecollided = 0;
    newDerelict->attributes = 0;
    newDerelict->attributesParam = 0;
    memClearDword(newDerelict->unk_3C, 0, sizeof(newDerelict->unk_3C) / sizeof(udword));
    newDerelict->derelicttype = derelictType;
    newDerelict->health = newDerelict->staticinfo->maxhealth;
    newDerelict->collInfo.precise = NULL;
    newDerelict->dockingship = NULL;
    newDerelict->putOnDoor=FALSE;
    newDerelict->TractorTrue = FALSE;
    newDerelict->tractorbeaminfo = 0;
    newDerelict->tractorbeam_playerowner = NULL;
    newDerelict->tractorPosinfo[0] = FALSE;
    newDerelict->tractorPosinfo[1] = FALSE;
    newDerelict->tractorPosinfo[2] = FALSE;
    newDerelict->tractorPosinfo[3] = FALSE;
    newDerelict->aistate = 0;
    newDerelict->specialFlags = 0;
    newDerelict->specialFlags2 = 0;
    memset(newDerelict->cataRuntimeState, 0, sizeof(newDerelict->cataRuntimeState));
    newDerelict->clampInfo = NULL;
    newDerelict->creationTime = universe.totaltimeelapsed;
    newDerelict->ambientSoundHandle = SOUND_NOTINITED;
    newDerelict->randomSoundHandle = SOUND_NOTINITED;
    newDerelict->nextRandomTime = universe.totaltimeelapsed;

    vecZeroVector(newDerelict->posinfo.force);
    vecZeroVector(newDerelict->rotinfo.torque);

    memset(&newDerelict->specialFlags3, 0, sizeof(sdword) * MAX_MULTIPLAYER_PLAYERS);


    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        newDerelict->nonstatvars.thruststrength[i] = stat->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
        newDerelict->nonstatvars.rotstrength[i] = stat->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
        newDerelict->nonstatvars.turnspeed[i] = stat->turnspeedstat[i];
    }

    newDerelict->newDockWithTransfer = NULL;
    univInitSpaceObjPosRot((SpaceObj *)newDerelict,pos,FALSE);

    InitializeDerelictNavLights(newDerelict);

    collAddSpaceObjToCollBlobs((SpaceObj *)newDerelict);
    listAddNode(&universe.SpaceObjList,&(newDerelict->objlink),newDerelict);
    listAddNode(&universe.DerelictList,&(newDerelict->derelictlink),newDerelict);
    listAddNode(&universe.ImpactableList,&(newDerelict->impactablelink),newDerelict);
    newDerelict->posinfo.unk_2C = NULL;
    lodMeshBindingsAllocate((SpaceObj *)newDerelict);

    if (stat->worldRender)
    {                                                       //if planet-style rendering
//        bitSet(newDerelict->flags, SOF_ForceVisible);
        univAddToWorldList(newDerelict);
    }

    //find a color scheme
    for (i = 0; i < MAX_MULTIPLAYER_PLAYERS; i++)
    {
        if (stat->teamColor[i])
        {
            newDerelict->colorScheme = (sbyte)i;
            break;
        }
    }

    //if a crate, report the placement!
    if (!singlePlayerGame)
    {
        if(derelictType == Crate)
        {
            cratesReportCratePlacement(newDerelict);
        }
    }

    return newDerelict;
}

/*-----------------------------------------------------------------------------
    Name        : univAddDustCloud
    Description : adds a dustcloud to the mission sphere at position cloudpos
    Inputs      : cloudpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
DustCloud *univAddDustCloud(DustCloudType dustcloudtype,vector *cloudpos)
{
    DustCloud* newCloud;

    dbgAssert(dustcloudtype < NUM_DUSTCLOUDTYPES);

    newCloud = memAlloc(sizeof(DustCloud),"DustCloud",NonVolatile);

    newCloud->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newCloud);
    universe.resourceNumber++;

    newCloud->objtype = OBJ_DustType;
    newCloud->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource + SOF_NotBumpable;
    newCloud->staticinfo = (ResourceStaticInfo *)&dustcloudStaticInfos[dustcloudtype];
    ClearNode(newCloud->renderlink);
    newCloud->collMyBlob = NULL;
    newCloud->currentLOD = 0;
    newCloud->cameraDistanceSquared = REALlyBig;
    newCloud->flashtimer = 0;
    newCloud->lasttimecollided = 0;
    newCloud->attributes = 0;
    newCloud->attributesParam = 0;
    newCloud->resourcevalue = dustcloudStaticInfos[dustcloudtype].resourcevalue;
    newCloud->resourceNotAccessible = 0;
    newCloud->resourceVolume = NULL;
    newCloud->dustcloudtype = dustcloudtype;
    newCloud->scaling = 1.0f;
    newCloud->health = newCloud->staticinfo->maxhealth;
    newCloud->collInfo.precise = NULL;
    newCloud->stub = (void*)cloudCreateSystem(newCloud->staticinfo->staticheader.staticCollInfo.collspheresize, 0.1f);
    {
        cloudSystem* csys = (cloudSystem*)newCloud->stub;
        csys->position = &newCloud->posinfo.position;
        csys->rotation = &newCloud->rotinfo.coordsys;
        csys->maxhealth = (sdword) newCloud->health;
    }

    vecZeroVector(newCloud->posinfo.force);
    vecZeroVector(newCloud->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj *)newCloud,cloudpos,TRUE);

    collAddSpaceObjToCollBlobs((SpaceObj *)newCloud);
    listAddNode(&universe.SpaceObjList,&(newCloud->objlink),newCloud);
    listAddNode(&universe.ResourceList,&(newCloud->resourcelink),newCloud);
    listAddNode(&universe.ImpactableList,&(newCloud->impactablelink),newCloud);

    return newCloud;
}

/*-----------------------------------------------------------------------------
    Name        : univAddGasCloud
    Description : adds a Gascloud to the mission sphere at position cloudpos
    Inputs      : cloudpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
GasCloud *univAddGasCloud(GasCloudType gascloudtype,vector *cloudpos)
{
    GasCloud *newCloud;

    dbgAssert(gascloudtype < NUM_GASCLOUDTYPES);

    newCloud = memAlloc(sizeof(GasCloud),"GasCloud",NonVolatile);

    newCloud->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newCloud);
    universe.resourceNumber++;

    newCloud->objtype = OBJ_GasType;
    newCloud->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource + SOF_NotBumpable;
    newCloud->flags2 = 0;
    newCloud->staticinfo = (ResourceStaticInfo *)&gascloudStaticInfos[gascloudtype];
    ClearNode(newCloud->renderlink);
    newCloud->collMyBlob = NULL;
    newCloud->currentLOD = 0;
    newCloud->cameraDistanceSquared = REALlyBig;
    newCloud->flashtimer = 0;
    newCloud->lasttimecollided = 0;
    newCloud->attributes = 0;
    newCloud->attributesParam = 0;
    newCloud->resourcevalue = gascloudStaticInfos[gascloudtype].resourcevalue;
    newCloud->resourceNotAccessible = 0;
    newCloud->resourceVolume = NULL;
    newCloud->gascloudtype = gascloudtype;
    newCloud->scaling = 1.0f;
    newCloud->health = newCloud->staticinfo->maxhealth;
    newCloud->collInfo.precise = NULL;

    vecZeroVector(newCloud->posinfo.force);
    vecZeroVector(newCloud->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj *)newCloud,cloudpos,TRUE);

    collAddSpaceObjToCollBlobs((SpaceObj *)newCloud);
    listAddNode(&universe.SpaceObjList,&(newCloud->objlink),newCloud);
    listAddNode(&universe.ResourceList,&(newCloud->resourcelink),newCloud);
    listAddNode(&universe.ImpactableList,&(newCloud->impactablelink),newCloud);
    newCloud->posinfo.unk_2C = NULL;
    newCloud->posinfo.lodMeshBindings = NULL;

    return newCloud;
}

/*-----------------------------------------------------------------------------
    Name        : univAddNebula
    Description : adds a nebula to the mission sphere at position nebpos
    Inputs      : nebpos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Nebula* univAddNebula(NebulaType nebulatype,vector* nebpos, void* stub)
{
    Nebula* newNeb;

    dbgAssert(nebulatype < NUM_NEBULATYPES);

    newNeb = memAlloc(sizeof(Nebula), "Nebula", NonVolatile);

    newNeb->resourceID.resourceNumber = universe.resourceNumber;
    IDToPtrTableAdd(&ResourceIDToPtr,universe.resourceNumber,(SpaceObj *)newNeb);
    universe.resourceNumber++;

    newNeb->objtype = OBJ_NebulaType;
    newNeb->flags = SOF_Rotatable + SOF_Impactable + SOF_Targetable + SOF_Resource + SOF_NotBumpable;
    newNeb->staticinfo = (ResourceStaticInfo *)&nebulaStaticInfos[nebulatype];
    ClearNode(newNeb->renderlink);
    newNeb->collMyBlob = NULL;
    newNeb->currentLOD = 0;
    newNeb->cameraDistanceSquared = REALlyBig;
    newNeb->flashtimer = 0;
    newNeb->lasttimecollided = 0;
    if (nebAttributes != 0)
    {
        newNeb->attributes = (uword)nebAttributes;
    }
    else
    {
        newNeb->attributes = 0;
    }
    newNeb->attributesParam = 0;
    newNeb->resourcevalue = nebulaStaticInfos[nebulatype].resourcevalue;
    newNeb->resourceNotAccessible = 0;
    newNeb->resourceVolume = NULL;
    newNeb->nebulatype = nebulatype;
    newNeb->scaling = 1.0f;
    newNeb->health = newNeb->staticinfo->maxhealth;
    newNeb->collInfo.precise = NULL;

    newNeb->stub = stub;
    nebNewNeb(newNeb);

    vecZeroVector(newNeb->posinfo.force);
    vecZeroVector(newNeb->rotinfo.torque);

    univInitSpaceObjPosRot((SpaceObj*)newNeb, nebpos, TRUE);

    collAddSpaceObjToCollBlobs((SpaceObj *)newNeb);
    listAddNode(&universe.SpaceObjList, &(newNeb->objlink), newNeb);
    listAddNode(&universe.ResourceList, &(newNeb->resourcelink), newNeb);
    listAddNode(&universe.ImpactableList, &(newNeb->impactablelink), newNeb);
    newNeb->posinfo.unk_2C = NULL;
    newNeb->posinfo.lodMeshBindings = NULL;
 
    return newNeb;
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteEffect
    Description : Deletes an effect
    Inputs      : effect - pointer to effect to delete
    Outputs     : frees the effect and it's effector list (actually a single memFree operation)
    Return      :
----------------------------------------------------------------------------*/
void univDeleteEffect(Effect *effect)
{
    univRemoveObjFromRenderList((SpaceObj *)effect);
    listDeleteNode(&effect->objlink);
}


/*-----------------------------------------------------------------------------
    Name        : univMeshBindingsDupe
    Description : Duplicate the local static bindings for a ship and fix up gun
                    pointers local to this ship instance.
    Inputs      : shipstaticinfo - staticinfo for this ship
                  newShip - newly allocated ship
    Outputs     : allocates new RAM and copies/fixes up the binding info
    Return      : address of newly allocated RAM or NULL
----------------------------------------------------------------------------*/
shipbindings *univMeshBindingsDupe(ShipStaticInfo *shipstaticinfo, Ship *newShip)
{
    shipbindings *newBindings;
    mhbinding *bindingListSource;
    mhlocalbinding *bindingListDest;
    sdword nLevels, index, nObjects, j;
    lodinfo *lodInfo = shipstaticinfo->staticheader.LOD;
    ShipUpgradeSlotStateBlock *slotStateBlock = (ShipUpgradeSlotStateBlock *)newShip->runtimeData490;

    if (shipstaticinfo->hierarchySize == 0)
    {                                                       //do nothing if there is no bindings to dupe
        return(NULL);
    }
    nLevels = lodInfo->nLevels;                             //record number of LOD's
    newBindings = memAlloc(shipBindingsLength(nLevels) +    //allocate bindings
                           shipstaticinfo->hierarchySize * sizeof(mhlocalbinding),
                           "ShipLocalBindings", NonVolatile);
#if MESH_PRE_CALLBACK
    newBindings->preCallback = NULL;
#endif
    newBindings->postCallback = NULL;                       //no callbacks needed for guns
    bindingListDest = (mhlocalbinding *)((ubyte *)newBindings +
                    shipBindingsLength(nLevels));           //start of first binding list
    for (index = 0; index < nLevels; index++)
    {                                                       //for each LOD
        if ((lodInfo->level[index].flags & LM_LODType) != LT_Mesh)
        {
            newBindings->localBinding[index] = NULL;
            continue;
        }
        newBindings->localBinding[index] = bindingListDest; //store list pointer for this LOD
        nObjects = ((meshdata *)lodInfo->level[index].pData)->nPolygonObjects;
        bindingListSource = lodInfo->level[index].hBindings;
//        memcpy(bindingList, lodInfo->level[index].hBindings,//copy the bindings
//                nObjects * sizeof(mhbinding));
        for (j = 0; j < nObjects; j++)
        {
            bindingListDest->function = bindingListSource->function;
            bindingListDest->userData = bindingListSource->userData;
            bindingListDest->userID = bindingListSource->userID;
            bindingListDest->flags = bindingListSource->flags;
            bindingListDest->object = bindingListSource->object;

            if (bindingListSource->function != NULL)
            {
                dbgAssert(slotStateBlock != NULL);
                dbgAssert(bindingListSource->userID >= 0);
                dbgAssert(bindingListSource->userID < slotStateBlock->count);
                bindingListDest->userData = &slotStateBlock->entries[bindingListSource->userID];
                bindingListDest->userID = (sdword)newShip;    //also save ship pointer
            }
            bindingListDest++;
            bindingListSource++;
        }
//        bindingListDest += nObjects;                        //update location of list
    }
    newBindings->frequencyCounter = 0;                      //counter expires right now
    meshBindingsFixup(shipstaticinfo, newBindings);
    dbgAssert((udword)((ubyte *)bindingListDest - (ubyte *)newBindings) == shipBindingsLength(nLevels) + shipstaticinfo->hierarchySize * sizeof(mhlocalbinding));
#if MESH_VERBOSE_LEVEL >= 2
    dbgMessagef("\nunivMeshBindingsDupe: localizing bindings 0x%x for ship 0x%x lengh %d",
                newBindings, newShip, shipBindingsLength(nLevels) + shipstaticinfo->hierarchySize * sizeof(mhlocalbinding));
#endif
    return(newBindings);
}

shipbindings *univMeshBindingsStaticDupe(ShipStaticInfo *shipstaticinfo)
{
    shipbindings *newBindings;
    mhbinding *bindingListSource;
    mhlocalbinding *bindingListDest;
    sdword nLevels, level, index;
    lodinfo *lodInfo = shipstaticinfo->staticheader.LOD;
    udword bindingsLength;

    if (shipstaticinfo->hierarchySize == 0)
    {
        return NULL;
    }

    nLevels = lodInfo->nLevels;
    bindingsLength = shipBindingsLength(nLevels) + shipstaticinfo->hierarchySize * sizeof(mhlocalbinding);
    newBindings = memAlloc(bindingsLength, "ShipLocalBindings", NonVolatile);
    memset(newBindings, 0, bindingsLength);
#if MESH_PRE_CALLBACK
    newBindings->preCallback = NULL;
#endif
    newBindings->postCallback = NULL;
    bindingListDest = (mhlocalbinding *)((ubyte *)newBindings + shipBindingsLength(nLevels));

    for (level = 0; level < nLevels; level++)
    {
        lod *lodLevel = &lodInfo->level[level];
        meshdata *mesh;

        if ((lodLevel->flags & LM_LODType) != LT_Mesh)
        {
            newBindings->localBinding[level] = NULL;
            continue;
        }

        newBindings->localBinding[level] = bindingListDest;
        mesh = (meshdata *)lodLevel->pData;
        bindingListSource = lodLevel->hBindings;
        for (index = 0; index < mesh->nPolygonObjects; index++, bindingListDest++, bindingListSource++)
        {
            bindingListDest->object = bindingListSource->object;
        }
    }

    newBindings->frequencyCounter = 0;
    meshBindingsFixup(shipstaticinfo, newBindings);
    return newBindings;
}

void meshBindingsFixup(ShipStaticInfo *shipstaticinfo, shipbindings *bindings)
{
    lodinfo *LOD = shipstaticinfo->staticheader.LOD;
    sdword level, index;

    for (level = 0; level < LOD->nLevels; level++)
    {
        lod *lodLevel = &LOD->level[level];
        meshdata *mesh;
        mhbinding *sourceBinding;
        mhlocalbinding *destBinding;

        if ((lodLevel->flags & LM_LODType) != LT_Mesh)
        {
            continue;
        }

        destBinding = bindings->localBinding[level];
        if (destBinding == NULL)
        {
            continue;
        }

        mesh = (meshdata *)lodLevel->pData;
        sourceBinding = lodLevel->hBindings;
        for (index = 0; index < mesh->nPolygonObjects; index++, destBinding++, sourceBinding++)
        {
            if (sourceBinding->parentIndex == 0xff)
            {
                destBinding->parentBinding = NULL;
            }
            else
            {
                dbgAssert(sourceBinding->parentIndex < mesh->nPolygonObjects);
                destBinding->parentBinding = bindings->localBinding[level] + sourceBinding->parentIndex;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univMeshBindingsDelete
    Description : Delete the mesh bindings for a ship about to be deleted.
    Inputs      : ship - ship being deleted
    Outputs     : frees the memory
    Return      :
----------------------------------------------------------------------------*/
void univMeshBindingsDelete(Ship *ship)
{
    if (ship->madBindings && ship->madBindings->nCurrentAnim != MAD_NoAnimation)
    {                                                       //if there's a mesh animation going on
        ship->bindings = ship->madBindings->saveBindings;   //restore old bindings
    }
    if (ship->bindings == NULL)
    {
        return;
    }
#if MESH_VERBOSE_LEVEL >= 2
    dbgMessagef("\nunivMeshBindingsDelete: freeing bindings for ship 0x%x", ship);
#endif
    dbgAssert(ship->bindings != NULL);
    memFree(ship->bindings);
}

void InitializeEngineTrails(Ship *newship)
{
    ShipStaticInfo *shipstaticinfo = newship->staticinfo;
    char engine[5];
    sdword idx;
    sdword i;
    sdword numTrails;

    numTrails = 0;
    strcpy(engine, "EngX");
    for (idx = 0; idx < MAX_NUM_TRAILS; idx++)
    {
        engine[3] = (char) ('0' + (char)idx);
        if (mexGetChunk((void*)shipstaticinfo->staticheader.pMexData, "Eng", engine) != 0)
        {
            numTrails = idx + 1;
        }
    }

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        newship->trail[i] = NULL;
    }
    if (shipstaticinfo->trailStatic[0] != NULL)
    {
        for (i = 0; i < numTrails; i++)
        {
            bool8 second = (bool8) ((i == 0) ? 0 : 1);
            newship->trail[i] = trailNew(shipstaticinfo->trailStatic[i], (void*)newship, second, (ubyte)i);
        }
    }
    else
    {
        //capital ship engine glow
        for (i = 0; i < numTrails; i++)
        {
            if (newship->shiprace == RACE_Beast)
            {
                newship->trail[i] = trailNew(shipstaticinfo->trailStatic[i], (void*)newship, 2, (ubyte)i);
            }
            else
            {
                newship->trail[i] = trailNew(shipstaticinfo->trailStatic[i], (void*)newship, 3, (ubyte)i);
            }
        }
    }
}

void univResetNewGimbleGun(Gun *gun)
{
    GunStatic *gunstatic = gun->gunstatic;
    dbgAssert(gunstatic->guntype == GUN_NewGimble);

    gun->angle = 0.0f;
    gun->declination = 0.0f;
    gun->anglespeed = 0.0f;
    gun->declinationspeed = 0.0f;
    gun->curgunorientation = gunstatic->defaultgunorientation;
    if (bitTest(gunstatic->flags, GF_MultiLevelMatrix))
    {
        gun->curGunOrientationNonConcat = gunstatic->defaultGunOrientationNonConcat;
    }
    matGetVectFromMatrixCol3(gun->gunheading,gun->curgunorientation);
}

static void InitializeNavLightsFromStaticInfo(NAVLightStaticInfo *navlightstaticinfo, NAVLightInfo **navLightInfo)
{
    if (navlightstaticinfo != NULL)
    {
        sdword numNAVLights = navlightstaticinfo->numNAVLights;
        NAVLightStatic *curnavlightstatic;
        NAVLight *curnavlight;
        sdword i;

        dbgAssert(numNAVLights > 0);

        *navLightInfo = memAlloc(sizeofNavLightInfo(numNAVLights),"navlightInfo",NonVolatile);
        (*navLightInfo)->numLights = numNAVLights;

        for (i=0,curnavlight=&(*navLightInfo)->navLights[0],curnavlightstatic=&navlightstaticinfo->navlightstatics[0];i<numNAVLights;i++,curnavlight++,curnavlightstatic++)
        {
            curnavlight->navlightstatic = curnavlightstatic;
            curnavlight->lightstate = 0;
            curnavlight->lastTimeFlashed = curnavlightstatic->startdelay;
        }
    }
    else
    {
        *navLightInfo = NULL;
    }
}

void InitializeNavLights(Ship *newship)
{
    InitializeNavLightsFromStaticInfo(newship->staticinfo->navlightStaticInfo, &newship->navLightInfo);
}

void InitializeDerelictNavLights(Derelict *derelict)
{
    InitializeNavLightsFromStaticInfo(derelict->staticinfo->navlightStaticInfo, &derelict->navLightInfo);
}

void InitializeShowingDamage(Ship* ship)
{
    sdword i, j;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            ship->showingDamage[i][j] = NULL;
        }
    }

    ship->nDamageLights = 0;
    ship->damageLights = NULL;

    dmgGetLights(ship);
}

/*-----------------------------------------------------------------------------
    Name        : univCreateShip
    Description : creates a ship and adds it to the universe unless suppressed
    Inputs      : shiptype, shiprace, shippos, playerowner,
                  built - flag indicating if ship was built or placed in world
    Outputs     :
    Return      : address of ship created
----------------------------------------------------------------------------*/
static Ship *univCreateShipInternal(
    ShipType shiptype,
    ShipRace shiprace,
    vector *shippos,
    struct Player *playerowner,
    sdword built,
    void *createLink,
    sdword createContextFlags)
{
    Ship *newship;
    udword sizeship, totalsize, madAssedSize;
    ShipStaticInfo *shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
    sdword i;
    ubyte *extraPointer;

    dbgAssert(playerowner != NULL);
/*
#ifndef HW_Release
    if (playerowner->race != shiprace)
    {
        dbgFatal(DBG_Loc,"Illegal race of ship for player");
    }
#endif
*/
#ifndef HW_Release
    if (!bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
    {
        // Try on-demand loading (NIS objects may not be in mission preload list)
        bitSet(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded);
        InitStatShipInfo(shipstaticinfo, shiptype, shiprace);
        if (!bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            dbgMessagef("\n%s\\%s not found - skipping ship creation.", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
            return NULL;
        }
    }
#endif
    sizeship = offsetof(Ship, ShipSpecifics) + sizeof(udword);
    if (shipstaticinfo->custshipheader.sizeofShipSpecifics != 0)
    {
        sizeship += shipstaticinfo->custshipheader.sizeofShipSpecifics - sizeof(udword);
    }
    totalsize = sizeship;
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {                                                       //account for the local precise selection
        totalsize += sizeof(PreciseSelection);
    }
    if (shipstaticinfo->madStatic != NULL)
    {                                                       //account for the local animation bindings
        madAssedSize = madAnimSize(shipstaticinfo->staticheader.LOD->nLevels) + //size of the binding info
                     shipstaticinfo->hierarchySize * sizeof(mhlocalbinding) + //size of the actual binding structures
                     shipstaticinfo->madStatic->header->nObjects * sizeof(splinecurve) * 6; //size of spline curves, enough for all objects

        totalsize += madAssedSize;
    }
    newship = memAlloc(totalsize,"Ship",NonVolatile);
    memset(newship,0,sizeship);                             // set everything to 0 by default
    extraPointer = (ubyte *)newship + sizeship;             //point to end of ship where extra RAM was allocated

    /* Retail (0x5423d2) explicitly sets this byte to 0xFF (-1) right after the
       memset -- without it, crystalDockSlotIndex defaults to 0 (a valid-looking
       dock point index) instead of the "no slot assigned" sentinel, causing
       SalCapFlyCapturedShipToHostDockPoint to skip dock point selection entirely
       and lock onto whatever point is at index 0. */
    newship->crystalDockSlotIndex = -1;

    newship->shipDeCloakTime = -20.0f;                  //initialize to a negative number so if a battle occurs at time 0.0f or anytime near by, then it will not screw up.
    newship->objtype = OBJ_ShipType;
    newship->flags = SOF_Rotatable + SOF_Impactable + SOF_Selectable + SOF_Targetable;
    if (shipstaticinfo->staticheader.staticCollInfo.collspheresize > VERYBIGSHIP_SIZE)
    {
        newship->flags |= SOF_BigObject;
    }
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
//        newship->collInfo.precise = (PreciseSelection *)(((ubyte *)newship) + sizeship);
        newship->collInfo.precise = (PreciseSelection *)extraPointer;
        extraPointer += sizeof(PreciseSelection);
    }
    else
    {
        newship->collInfo.precise = NULL;
    }
    newship->staticinfo = shipstaticinfo;
    bitClear(newship->specialFlags,SPECIAL_ParadeNeedTomoveCloser);
//    ClearNode(newship->renderlink);

    //need to copy over static info to non static structures so we
    //can beging fiddling with them...

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        newship->nonstatvars.thruststrength[i] = shipstaticinfo->thruststrengthstat[i];
    }
    for(i=0;i< NUM_ROT_DEGOFFREEDOM;i++)
    {
        newship->nonstatvars.rotstrength[i] = shipstaticinfo->rotstrengthstat[i];
    }
    for(i=0;i< NUM_TURN_TYPES;i++)
    {
        newship->nonstatvars.turnspeed[i] = shipstaticinfo->turnspeedstat[i];
    }

    newship->shipID.shipNumber = universe.shipNumber;
    IDToPtrTableAdd(&ShipIDToPtr,universe.shipNumber,(SpaceObj *)newship);
    universe.shipNumber++;

    newship->shiptype = shiptype;
    newship->shiprace = shiprace;
    newship->tacticstype = Neutral;         //default to neutral tactical setting
    newship->tactics_ordertype = NO_ORDER_NO_ENEMY;  //default to no bonuses...
    newship->tacticsTalk = 0;
    newship->tacticsNeedToFlightMan = FALSE;
    newship->dockwaitforNorm=FALSE;
    newship->putOnDoor=FALSE;
    newship->tacticsFormationVar1 = FALSE;
    newship->DodgeTime = 0.0f;
    newship->isDodging = FALSE;
    newship->timeCreated = universe.totaltimeelapsed;
//    newship->currentLOD = 0;                                //start at highest LOD by default
//    newship->collMyBlob = NULL;
    newship->cameraDistanceSquared = REALlyBig;
    newship->playerowner = playerowner;
    newship->cataBusyStateWindow.dockingship = NULL;
    newship->specialFlags = 0;
    newship->specialFlags = 0;
    newship->kamikazeState = 0;
    newship->visibleToWho = 0;      //initialize cloaked visible flag to invisible to everyone!
    newship->visibleToWhoPreviousFrame = 0;
    newship->damageModifier = 1.0f;
    newship->forceCancelDock=FALSE;
    newship->hyperspacePing=FALSE;
    newship->gravwellTimeEffect=0.0f;
    newship->passiveAttackCancelTimer=0.0f;
    // Alex's Repair Droid stuff...
//    newship->pRepairDroids = memAlloc(sizeof(RepairDroid), "RDroid", NonVolatile);

    univInitSpaceObjPosRot((SpaceObj *)newship,shippos,FALSE);

    newship->tractorbeaminfo = 0;
    newship->upgradeState.level = 0;
    newship->tractorPosinfo[0] = FALSE;
    newship->tractorPosinfo[1] = FALSE;
    newship->tractorPosinfo[2] = FALSE;
    newship->tractorPosinfo[3] = FALSE;
    newship->tractorstate = 0;            //info variable for tractorbeaming state
    newship->needNewLeader = FALSE;

    memset(&newship->specialFlags3, 0, sizeof(sdword) * MAX_MULTIPLAYER_PLAYERS);

    newship->clampInfo = NULL;

    newship->dontrotateever = FALSE;
    newship->dontapplyforceever = FALSE;
    newship->health = shipstaticinfo->maxhealth;
    newship->fuel = shipstaticinfo->maxfuel;

    if (shipstaticinfo->nRepairDroids != 0 && shipstaticinfo->shiprace == RACE_Sect)
    {
        newship->runtimeAnimationOverlay.runtimeController = (udword)CreateRepairDroid(newship, shipstaticinfo->nRepairDroids);
    }
//    newship->resources = 0;
//    newship->aistate = 0;
//    newship->aistateattack = 0;
//    newship->hotKeyGroup = 0;

    newship->shipSinglePlayerGameInfo = spNewShipSinglePlayerGameInfo();

    InitializeEngineTrails(newship);

    InitializeShowingDamage(newship);

    if (shipstaticinfo->gunStaticInfo != NULL)
    {
        sdword numSlots = shipstaticinfo->gunStaticInfo->numGuns;

        if (numSlots > 0)
        {
            GunInfo *gunInfo = memAlloc(sizeofGunInfo(numSlots), "gunInfo", NonVolatile);
            GunStatic *gunStatic = shipstaticinfo->gunStaticInfo->gunstatics;

            memset(gunInfo, 0, sizeofGunInfo(numSlots));
            newship->cataclysmGunInfo = gunInfo;
            gunInfo->numGuns = numSlots;

            for (i = 0; i < numSlots; i++, gunStatic++)
            {
                Gun *gun = &gunInfo->guns[i];

                gun->gunstatic = gunStatic;
                switch (gunStatic->guntype)
                {
                    case GUN_Gimble:
                    case GUN_Fixed:
                        gun->gunheading = gunStatic->gunnormal;
                        break;
                    case GUN_NewGimble:
                        gun->curgunorientation = gunStatic->defaultgunorientation;
                        if ((gunStatic->flags & 1) != 0)
                        {
                            gun->curGunOrientationNonConcat = gunStatic->defaultGunOrientationNonConcat;
                        }
                        gun->gunheading.x = gun->curgunorientation.m13;
                        gun->gunheading.y = gun->curgunorientation.m23;
                        gun->gunheading.z = gun->curgunorientation.m33;
                        break;
                    case GUN_MissileLauncher:
                    case GUN_MineLauncher:
                        gun->numMissiles = gunStatic->maxMissiles;
                        gun->gunheading = gunStatic->gunnormal;
                        break;
                    default:
                        break;
                }

                gun->gimblehandle = (sdword)-1;
            }
        }
        else
        {
            newship->cataclysmGunInfo = NULL;
        }
    }
    else
    {
        newship->cataclysmGunInfo = NULL;
    }

    shipInitUpgradeSlots(newship);
    if ((built & UNIV_CREATE_SHIP_SKIP_PLAYER_UPGRADES) == 0)
    {
        shipApplyPlayerUpgrades(newship);
    }

    setSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)newship);

    if (shipstaticinfo->dockStaticInfo != NULL)
    {
        DockStaticInfo *dockstaticinfo = shipstaticinfo->dockStaticInfo;
        sdword numDockPoints = dockstaticinfo->numDockPoints;
        DockStaticPoint *curdockstaticpoint;
        DockPoint *curdockpoint;

        dbgAssert(numDockPoints > 0);

        newship->dockInfo = memAlloc(sizeofDockInfo(numDockPoints),"dockInfo",NonVolatile);
        newship->dockInfo->busyness = 0;
        newship->dockInfo->numDockPoints = numDockPoints;

        for (i=0,curdockpoint=&newship->dockInfo->dockpoints[0],curdockstaticpoint=&dockstaticinfo->dockstaticpoints[0];i<numDockPoints;i++,curdockpoint++,curdockstaticpoint++)
        {
            curdockpoint->dockstaticpoint = curdockstaticpoint;
            curdockpoint->thisDockBusy = 0;
        }

        dockAllocDockPointActivity(newship);
    }
    else
    {
        newship->dockInfo = NULL;
    }

    InitializeNavLights(newship);

    if (shipstaticinfo->canBuildShips ||
        shipstaticinfo->canReceiveShipsPermanently ||
        bitTest(shipstaticinfo->shipFlags, BIT15))
    {
        ShipsInsideMe *shipsInsideMe = memAlloc(sizeof(ShipsInsideMe),"ShipsInsideMe",NonVolatile);
        memset(shipsInsideMe, 0, sizeof(ShipsInsideMe));
        listInit(&shipsInsideMe->insideList);
        shipsInsideMe->FightersInsideme = 0;
        shipsInsideMe->CorvettesInsideme = 0;
        newship->shipsInsideMe = shipsInsideMe;
    }
    else
    {
        newship->shipsInsideMe = NULL;
    }

    if (createLink == NULL &&
        (!singlePlayerGame || playerowner->playerIndex == 0) &&
        shipstaticinfo->specialActivateIsContinuous &&
        (shiptype == sCarrier ||
         shiptype == sMothership ||
         shiptype == bCarrier ||
         shiptype == bMothership))
    {
        cmAddFactory(newship, shipstaticinfo->canBuildBigShips);
    }

    /* init the sound variables for this ship */
    soundEventInitStruct(&newship->soundevent);
    newship->burstGunIndex = 0;

    //lightning effect stuff
    newship->lightning[0] = NULL;
    newship->lightning[1] = NULL;

    //create the instance version of the hierarchy bindings
    newship->bindings = univMeshBindingsDupe(shipstaticinfo, newship);
    lodMeshBindingsAllocate((SpaceObj *)newship);

    if (shipstaticinfo->madStatic != NULL)
    {
        newship->madBindings = (madanim *)extraPointer;
        extraPointer += sizeof(madanim);
        madAnimationInit(newship->madBindings);
        newship->madBindings->header = shipstaticinfo->madStatic->header;
        newship->madBindings->totalSize = madAssedSize;
        madAnimBindingsDupe(newship, shipstaticinfo, FALSE);

        //ship has mesh animations associated with it
        //so lets set its inital state.
        newship->madAnimationFlags = 0;
        newship->madGunStatus = 0;            //gun status //invalid states!
        newship->madWingStatus = 0;           //wing animation status
        newship->madSpecialStatus = 0;
        newship->madDoorStatus = MAD_STATUS_DOOR_CLOSED;
        newship->cuedAnimationIndex = 0;
        newship->cuedAnimationType = 0;

        if ((built & UNIV_CREATE_SHIP_BUILT) != 0)
        {
            madLinkInSetUpInitialBuiltMadState(newship);
        }
        else
        {
            madLinkInSetUpInitialPlacedMadState(newship);
        }

    }

    newship->statusWindow.state = 0;
    newship->statusWindow.chainHitCount = 0;
    newship->statusWindow.activeUntil = 0.0f;
    newship->beastAttack.sourceShip = NULL;
    newship->beastAttack.sourcePlayer = NULL;
    newship->beastAttack.raw_4F0 = 0;
    newship->beastAttack.nextChainTime = 0.0f;
    newship->beastAttack.effect = NULL;
    newship->replacementStateActive = FALSE;
    newship->replacementEffect = NULL;

    if (((ShipStaticInfo *)newship->staticinfo)->custshipheader.CustShipInit)
    {
        ((ShipStaticInfo *)newship->staticinfo)->custshipheader.CustShipInit(newship);
    }

    if ((shipstaticinfo->externalBuild != 0) && ((built & UNIV_CREATE_SHIP_BUILT) != 0))
    {
        dbgAssert(shiptype != bCruiseMissile);
        newship->health = 0.0f;
        newship->buildTimerElapsed = 0.0f;
        newship->specialFlags2 |= SPECIAL_2_DockFlag;
        newship->specialFlags4 |= SPECIAL4_ExternalBuildMember;
    }
    else
    {
        newship->buildTimerElapsed = 1.0f;
        newship->specialFlags2 &= ~SPECIAL_2_DockFlag;
        newship->specialFlags4 &= ~SPECIAL4_ExternalBuildMember;
    }

    shipAddSupportUsage(newship, playerowner);

    //determine what color scheme to use
    if ((playerowner->playerIndex < MAX_MULTIPLAYER_PLAYERS) && (shipstaticinfo->teamColor[playerowner->playerIndex]))
    {                                                       //if this player's default scheme woudl be a good color
        newship->colorScheme = (sbyte)playerowner->playerIndex;
    }
    else
    {                                                       //else let's find any old color scheme
        for (i = 0; i < MAX_MULTIPLAYER_PLAYERS; i++)
        {
            if (shipstaticinfo->teamColor[i])
            {
                newship->colorScheme = (sbyte)i;
                break;
            }
        }
    }

    if ((built & UNIV_CREATE_SHIP_NO_UNIVERSE_ADD) == 0)
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)newship);
        listAddNode(&universe.SpaceObjList, &(newship->objlink), newship);
        listAddNode(&universe.ShipList, &(newship->shiplink), newship);
        listAddNode(&universe.ImpactableList, &(newship->impactablelink), newship);
    }

    if (shiptype != sMothership ||
        (createContextFlags & UNIV_CREATE_SHIP_CONTEXT_SKIP_AUTO_CHILDREN) == 0 ||
        singlePlayerGameInfo.currentMission == 1)
    {
        univCreateClampedChildren(newship, shiprace, shippos, playerowner, built);
    }

    // Retail unconditionally stores the creator-link argument into dockstaticpoint here,
    // NOT customdockinfo. Raw disassembly of univCreateShip (0x542c0f) shows
    // `mov [ebp+4ACh], edx` — offset 0x4AC is dockstaticpoint (confirmed via
    // static_assert(offsetof(ShipDockRuntime, dockstaticpoint) == 0x18) => Ship+0x4AC),
    // not customdockinfo (Ship+0x4B0). An earlier read of this same instruction via
    // Hex-Rays decompile mislabeled the destination as customdockinfo; that decompile
    // name is wrong for this overlapping-union region (same trap seen in BigGun.c's
    // beast-cascade fields). edx here is the 6th stack argument to univCreateShip and is
    // confirmed to still hold `creator` at that point in clCreateShip's call site
    // (IDA 0x44a925/0x44a934).
    newship->dockvars.dockstaticpoint = (DockStaticPoint *)createLink;

    shipResetSlotStatusesForAllPlayers(newship);
    CrystalResetProcessingState(newship);
    shipNameReserve(newship);

    if ((multiPlayerGame || (multiPlayerGameUnderWay && playPackets)) &&
        GetRaceForShipTypeRange(newship->shiptype) == RACE_Sect &&
        !univIsCrystalTarget(newship) &&
        !shipIsBeastShip(newship))
    {
        newship->specialFlags2 |= 0x10;
        shipSetDelayedDeath(newship, 1.0e21f);
    }

    return newship;
}

Ship *univCreateShip(ShipType shiptype,ShipRace shiprace,vector *shippos,struct Player *playerowner,sdword built)
{
    return univCreateShipInternal(shiptype, shiprace, shippos, playerowner, built, NULL, 0);
}

Ship *univCreateShipWithContext(ShipType shiptype, ShipRace shiprace, vector *shippos, struct Player *playerowner, sdword built, void *createLink, sdword createContextFlags)
{
    dbgAssert((createContextFlags & ~UNIV_CREATE_SHIP_CONTEXT_SKIP_AUTO_CHILDREN) == 0);

    return univCreateShipInternal(shiptype, shiprace, shippos, playerowner, built, createLink, createContextFlags);
}

/*-----------------------------------------------------------------------------
    Name        : univAddShip
    Description : adds a ship to the universe
    Inputs      : shiptype, shiprace, shippos, playerowner
    Outputs     :
    Return      : address of ship added
----------------------------------------------------------------------------*/
Ship *univAddShip(ShipType shiptype,ShipRace shiprace,vector *shippos,struct Player *playerowner,sdword built)
{
    Ship *newship = univCreateShip(shiptype,shiprace,shippos,playerowner,built);

    //special case - don't want cryotrays to be selectable
    if (newship != NULL && shiptype == CryoTray)
    {
        bitClear(newship->flags, SOF_Selectable);
    }

    return newship;
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelMinorObjs
    Description : Exclusive physics updating for what would seem to
                  be Asteroid ZERO's in the single player game?
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void univUpdateAllPosVelMinorObjs()
{
    Node *objnode;
    SpaceObj *spaceobj;

    if ((singlePlayerGame) && (singlePlayerGameInfo.asteroid0sCanMove))
    {
        objnode = universe.MinorSpaceObjList.head;

        while (objnode != NULL)
        {
            spaceobj = (SpaceObj *)listGetStructOfNode(objnode);
            physUpdateObjPosVelBasic(spaceobj, universe.unknown1);
            objnode = objnode->next;
        }
    }
}

#define ENERGY_BULLET_RETARGET_DELAY          0.5f
#define ENERGY_BULLET_GUIDANCE_DISTANCE_SQR   40000.0f
#define ENERGY_BULLET_CLOSE_DISTANCE_SQR      10000.0f
#define ENERGY_BULLET_DEFAULT_SPEED           1600.0f
#define ENERGY_BULLET_SPEED_EPSILON_SQR       10000.0f
#define ENERGY_BULLET_MIN_TRACK_DOT          -0.86602497f
#define ENERGY_BULLET_CLOSE_SPEED_CAP         150.0f
#define ENERGY_BULLET_CLOSE_SPEED_DIVISOR     2.0f

// Cataclysm-specific: retail energy bullets retarget in flight after launch.
static void univUpdateEnergyBulletVelocity(Bullet *bullet)
{
    SpaceObjRotImpTarg *target = bullet->target;
    vector aimPoint;
    vector targetOffset;
    vector targetDirection;
    vector initialTargetDirection;
    real32 targetDistanceSquared;
    real32 speedSquared;
    real32 speed;

    if (target == NULL || bullet->timelived <= ENERGY_BULLET_RETARGET_DELAY)
    {
        return;
    }

    if (target->objtype == OBJ_ShipType)
    {
        ShipType shipType = ((Ship *)target)->shiptype;

        if (shipType == sSwarmer || shipType == bSwarmer)
        {
            return;
        }
    }

    aimPoint = target->posinfo.position;
    aishipGetTargetAimPoint(target, &aimPoint);
    vecSub(targetOffset, aimPoint, bullet->posinfo.position);
    targetDistanceSquared = vecMagnitudeSquared(targetOffset);
    targetDirection = targetOffset;
    vecNormalize(&targetDirection);
    initialTargetDirection = targetDirection;

    if (targetDistanceSquared >= ENERGY_BULLET_GUIDANCE_DISTANCE_SQR &&
        vecDotProduct(targetOffset, bullet->posinfo.velocity) < ENERGY_BULLET_MIN_TRACK_DOT)
    {
        return;
    }

    speedSquared = vecMagnitudeSquared(bullet->posinfo.velocity);
    if (speedSquared >= ENERGY_BULLET_DEFAULT_SPEED + ENERGY_BULLET_SPEED_EPSILON_SQR ||
        speedSquared <= ENERGY_BULLET_DEFAULT_SPEED - ENERGY_BULLET_SPEED_EPSILON_SQR)
    {
        speed = fsqrt(speedSquared);
    }
    else
    {
        speed = ENERGY_BULLET_DEFAULT_SPEED;
    }

    if (targetDistanceSquared < ENERGY_BULLET_GUIDANCE_DISTANCE_SQR && speed > 0.0f)
    {
        real32 leadTime = fsqrt(targetDistanceSquared) / speed;
        vector leadOffset;

        vecScalarMultiply(leadOffset, target->posinfo.velocity, leadTime);
        vecAddTo(targetOffset, leadOffset);
        targetDirection = targetOffset;
        vecNormalize(&targetDirection);
    }

    vecScalarMultiply(bullet->posinfo.velocity, targetDirection, speed);
    bullet->bulletheading = targetDirection;

    if (targetDistanceSquared < ENERGY_BULLET_CLOSE_DISTANCE_SQR)
    {
        if (speed > ENERGY_BULLET_CLOSE_SPEED_CAP)
        {
            speed /= ENERGY_BULLET_CLOSE_SPEED_DIVISOR;
        }

        vecScalarMultiply(bullet->posinfo.velocity, initialTargetDirection, speed);
        vecAddTo(bullet->posinfo.velocity, target->posinfo.velocity);

        targetDirection = bullet->posinfo.velocity;
        vecNormalize(&targetDirection);
        bullet->bulletheading = targetDirection;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelBullets
    Description : Exclusive physics updating for bulletes
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelBullets()
{
    Node *bulletnode = universe.BulletList.head;
    Node *deletenode;
    Bullet *bullet;
    bool32 addEffect = FALSE;
    etglod *etgLOD;
    sdword LOD;
    etgeffectstatic *stat;
    Effect *effect;

    while (bulletnode != NULL)
    {
        bullet = (Bullet *)listGetStructOfNode(bulletnode);

        if ((bullet->flags & SOF_DontApplyPhysics) == 0)
        {
            if ((WeaponType)bullet->bulletType == WEAPON_Energy)
            {
                univUpdateEnergyBulletVelocity(bullet);
            }

            if (physUpdateBulletPosVel(bullet, universe.unknown1))
            {
                //bullet has died so clean it up

                //turn off sound of beam
                if ((WeaponType)bullet->bulletType == WEAPON_Ion)
                {
                    if ((bullet->owner != NULL) && (bullet->gunowner != NULL))
                    {
                        soundEventBurstStop((Ship *)bullet->owner, (Gun *)bullet->gunowner);
                    }
                }

                //delete effect associated with bullet
                effect = bullet->effect;
#if ETG_DISABLEABLE
                if (effect != NULL && etgEffectsEnabled)
#else
                if (effect != NULL)
#endif
                {
                    if (bitTest(((etgeffectstatic*)effect->staticinfo)->specialOps, ESO_SelfDeleting))
                    {                               //if the effect can delete itself
                        ((real32 *)effect->variable)[ETG_BulletDurationParam] =
                            effect->timeElapsed;    //time it out
                        effect->owner = NULL;
                        bitClear(effect->flags, SOF_AttachPosition);
                        bitClear(effect->flags, SOF_AttachCoordsys);
                        bitClear(effect->flags, SOF_AttachVelocity);
                    }
                    else
                    {
                        etgEffectDelete(effect);
                        univRemoveObjFromRenderList((SpaceObj *)(effect));
                        listDeleteNode(&effect->objlink);
                        addEffect = TRUE;
                    }
                }
                deletenode = bulletnode;
                bulletnode = bulletnode->next;

                if (univBulletUsesSiegeShockwave(bullet))
                {
                    shockwaveProjectileHitStationary(bullet);
                }

                if (bullet->cataUnknown10C)
                {
                    etgLOD = etgSpecialPurposeEffectTable[
                        bullet->cataUnknown10C ? EGT_BIG_GUN_BULLET_EXPLOSION : EGT_BURST_EXPLOSION_EFFECT];

                    if (etgLOD != NULL)
                    {
                        LOD = bullet->currentLOD;

                        if (LOD >= etgLOD->nLevels)
                        {
                            stat = NULL;
                        }
                        else
                        {
                            stat = etgLOD->level[LOD];
                        }
                    }
                    else
                    {
                        stat = NULL;
                    }

                    if (addEffect)
                    {
#if ETG_DISABLEABLE
                        if (stat != NULL && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
                        if (stat != NULL && !etgFrequencyExceeded(stat))
#endif
                        {
                            vector velocity;
                            matrix coordsys = bullet->rotinfo.coordsys;
                            velocity.x = 0.0f;
                            velocity.y = 0.0f;
                            velocity.z = 0.0f;

                            etgEffectCreate(stat, NULL, &bullet->posinfo.position, &velocity, &coordsys, 1.0f, EAF_AllButNLips, 0);
                        }
                    }

                    pointExplosionInSpace(&bullet->posinfo.position,burstRadius,bullet->damage,bullet->playerowner->playerIndex);
                }

                //Defense Fighter 'hack' to remove its reference
                if(bitTest(bullet->SpecialEffectFlag, 0x0002))
                {    //bullet is possibly being targetted by a defense fighter...remove that reference
                    DefenseFighterBulletRemoval(bullet);
                }

                bobObjectDied(((SpaceObj *)bullet),&universe.collBlobList);
                listRemoveNode(&bullet->bulletlink);
                univRemoveObjFromRenderList(((SpaceObj *)bullet));
                listDeleteNode(&bullet->objlink);
                continue;
            }
        }
        bulletnode = bulletnode->next;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelMissiles
    Description : Exclusive physics updating for misiles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelMissiles()
{
    Node *misnode = universe.MissileList.head;
    Missile *missile;
    bool32 deleteflag;

    while (misnode != NULL)
    {
        missile = (Missile *)listGetStructOfNode(misnode);

        if ((missile->flags & SOF_DontApplyPhysics) == 0)
        {
            if(missile->missileType == MISSILE_Regular)
            {   //if missileType is a normal missile
                deleteflag = aishipGuideMissile(missile);
            }
            else if(missile->missileType == MISSILE_Mine)
            {   //if missiletype is a mine
                deleteflag = aishipGuideMine(missile);
            }
            else
            {
                deleteflag = FALSE;
                dbgAssert(FALSE);
            }

            physUpdateObjPosVelMissile(missile, universe.unknown1);
            if ((deleteflag) && ((((SpaceObj *)missile)->flags & SOF_Dead) == 0))
            {
                AddMissileToDeleteMissileList(missile, -1);
            }
        }
        misnode = misnode->next;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelDerelicts
    Description : Exclusive physics updating for derelicts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelDerelicts()
{
    Node *objnode = universe.DerelictList.head;
    Derelict *derelict;

    while (objnode != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(objnode);

        if ((derelict->flags & SOF_DontApplyPhysics) == 0)
        {
            if ((derelict->flags & SOF_NISShip) == 0)
            {
                physUpdateObjPosVelDerelicts(derelict, universe.unknown1);
            }
        }
        objnode = objnode->next;
    }
}
/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelResources
    Description : Exclusive physics updating for resources
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelResources()
{
    Node *objnode = universe.ResourceList.head;
    Resource *resource;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);

        if ((resource->flags & (SOF_DontApplyPhysics|SOF_NISShip)) == 0)
        {
            physUpdateObjPosVelBasic((SpaceObj *)resource, universe.unknown1);
        }

        if (resource->resourceNotAccessible > 0)
        {
            resource->resourceNotAccessible--;
        }

        objnode = objnode->next;
    }
}

// Next time to call univUpdateFormationSpeeds (global shared with univShipHouseKeep).
// Corresponds to runtime data section address 0xA6ED6C in the Cataclysm binary.
real32 univFormationSpeedNextUpdate = 0.0f;

void univActivateAllShips(void)
{
    Node *node;
    sdword i;

    growSelectReset(&universe.HousekeepShipList);
    growSelectReset(&universe.unk1);

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        univUpdateShipHousekeeping((Ship *)listGetStructOfNode(node));
    }

    {
        SelectCommand *sel = universe.unk1.selection;
        for (i = 0; i < sel->numShips; i++)
        {
            Ship *ship = sel->ShipPtr[i];
            if (ship != NULL)
            {
                univShipHouseKeep(ship);
            }
        }
    }

    {
        SelectCommand *sel = universe.HousekeepShipList.selection;
        for (i = 0; i < sel->numShips; i++)
        {
            Ship *ship = sel->ShipPtr[i];
            if (ship != NULL && ship->staticinfo->custshipheader.CustShipHousekeep != NULL)
            {
                ship->staticinfo->custshipheader.CustShipHousekeep(ship);
            }
        }
    }

    growSelectReset(&universe.HousekeepShipList);
    growSelectReset(&universe.unk1);
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelShips
    Description : Exclusive physics updating for ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelShips()
{
    Node *objnode;
    Ship *ship;
    sdword i;
    ShipStaticInfo *staticinfo;

    growSelectReset(&universe.HousekeepShipList);
    growSelectReset(&universe.unk1);
    if (ClampedShipList.selection == NULL)
        growSelectInit(&ClampedShipList);
    growSelectReset(&ClampedShipList);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if ((ship->flags & SOF_DontApplyPhysics) == 0)
        {
            // Physics path: setup control first, then physics or clamp, then housekeeping
            univSetupShipForControl(ship);

            // Add master host to ClampedShipList when this ship is the first slave, to avoid
            // processing the same master multiple times (one master may have many slaves)
            {
                ClampInfo *clampInfo = ship->clampInfo;
                SpaceObj *slaveHost;
                ClampInfo *hostClampInfo;
                if (clampInfo != NULL
                    && (slaveHost = (SpaceObj *)clampInfo->host) != NULL
                    && slaveHost->objtype == OBJ_ShipType
                    && (hostClampInfo = ((Ship *)slaveHost)->clampInfo)->host == NULL
                    && ((SelectCommand *)hostClampInfo->selection)->ShipPtr[0] == ship)
                {
                    growSelectAddShip(&ClampedShipList, (Ship *)slaveHost);
                }
                else
                {
                    physUpdateObjPosVelShip(ship, universe.unknown1);
                }
            }
            univUpdateShipHousekeeping(ship);
        }
        else
        {
            // No-physics path (NIS ships, etc.): manual dmg/trail update
            ship->gettingrocked = NULL;
            dmgShipThink(ship);
            for (i = 0; i < MAX_NUM_TRAILS; i++)
            {
                if (ship->trail[i] != NULL)
                    trailUpdate(ship->trail[i], &ship->enginePosition);
            }
            if (ship->clampInfo != NULL && ship->clampInfo->host != NULL)
                univUpdateAttachedObjRotInfo((SpaceObjRot *)ship);

            staticinfo = (ShipStaticInfo *)ship->staticinfo;
            if (staticinfo->custshipheader.CustShipHousekeep != NULL)
                growSelectAddShip(&universe.HousekeepShipList, ship);
            growSelectAddShip(&universe.unk1, ship);
        }

        // Update mesh animations
        // Cataclysm: live animation state is in runtimeAnimationOverlay.madAnimationData
        {
            madanim *madData = (madanim *)ship->runtimeAnimationOverlay.madAnimationData;
            if (madData != NULL)
            {
                if (ship->staticinfo->madStatic->animIndexRemapCount != 0)
                {
                    if (madData->nCurrentAnim != -1)
                    {
                        madAnimationUpdate(ship, universe.unknown1);
                    }
                }
                else if (madData->nCurrentAnim != -1)
                {
                    if (ship->madAnimationFlags & MAD_ANIMATION_NEED_PROC)
                    {
                        madLinkInUpdateMeshAnimations(ship);
                    }
                    else
                    {
                        madAnimationUpdate(ship, universe.unknown1);
                    }
                }
            }
        }

        objnode = objnode->next;
    }

    // Process clamped ship physics (update slave positions relative to each master)
    {
        SelectCommand *sel = ClampedShipList.selection;
        for (i = 0; i < sel->numShips; i++)
        {
            Ship *masterShip = sel->ShipPtr[i];
            if (masterShip != NULL)
            {
                ClampInfo *masterClamp = masterShip->clampInfo;
                if (masterClamp != NULL && masterClamp->host == NULL)
                {
                    updateClampedObject((SpaceObjRotImpTargGuidanceShipDerelict *)masterShip);
                }
            }
        }
    }

    // Retail base ship housekeeping runs for every active ship in universe.unk1.
    {
        SelectCommand *sel = universe.unk1.selection;
        for (i = 0; i < sel->numShips; i++)
        {
            if (sel->ShipPtr[i])
                univShipHouseKeep(sel->ShipPtr[i]);
        }
    }

    // Custom ship per-frame housekeeping callbacks
    {
        SelectCommand *sel = universe.HousekeepShipList.selection;
        for (i = 0; i < sel->numShips; i++)
        {
            Ship *obj = sel->ShipPtr[i];
            if (obj && obj->staticinfo->custshipheader.CustShipHousekeep != NULL)
                obj->staticinfo->custshipheader.CustShipHousekeep(obj);
        }
    }

    // Update formation speeds at a fixed interval
    if (universe.totaltimeelapsed > (double)univFormationSpeedNextUpdate)
    {
        univUpdateFormationSpeeds();
        univFormationSpeedNextUpdate = universe.totaltimeelapsed + 0.5f;
    }

    growSelectReset(&universe.HousekeepShipList);
    growSelectReset(&universe.unk1);
    growSelectReset(&ClampedShipList);
}

/*-----------------------------------------------------------------------------
    Name        : univPausedUpdateAllPosVelShips
    Description : A minor version of the above function that only does update
                    operations appropriate for when the universe is paused
                    by nisUniversePaused.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univPausedUpdateAllPosVelShips(void)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    sdword i;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        ship->gettingrocked = NULL;
        dmgShipThink(ship);
        for (i = 0; i < MAX_NUM_TRAILS; i++)
        {
            if (ship->trail[i] != NULL)
                trailUpdate(ship->trail[i], &ship->enginePosition);
        }
        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateAllPosVelEffects
    Description : Exclusive physics updating for Effects -
                  must update effects AFTER anything that can be an
                  effect owner (be on safe side and do it last)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateAllPosVelEffects()
{
    Node *objnode = universe.effectList.head;
    Node *deletenode;
    Effect *effect;

    while (objnode != NULL)
    {
        effect = (Effect *)listGetStructOfNode(objnode);

        if ((effect->flags & SOF_DontApplyPhysics) == 0)
        {
            if (etgEffectUpdate(effect, universe.unknown1))
            {
                deletenode = objnode;
                objnode = objnode->next;

                etgEffectDelete(effect);
                univRemoveObjFromRenderList((SpaceObj *)effect);
                listDeleteNode(&effect->objlink);
                continue;
            }
        }
        objnode = objnode->next;
    }

}

/*-----------------------------------------------------------------------------
    Name        : MakeNewAsteroidStaticInfo
    Description : makes a new asteroid staticinfo based on its scaling
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeNewAsteroidStaticInfo(Asteroid *asteroid)
{
    AsteroidStaticInfo *originalAsteroidStaticInfo = &asteroidStaticInfos[asteroid->asteroidtype];
    AsteroidStaticInfo *newAsteroidStaticInfo;
    StaticHeader *newStaticHeader;
    StaticHeader *originalStaticHeader;
    StaticCollInfo *newStaticCollInfo;
    StaticCollInfo *originalStaticCollInfo;
    real32 scaling = asteroid->scaling;

    dbgAssert(scaling > 0.0f);

    if ((asteroid->flags & SOF_StaticInfoIsDynamicCata) == 0)
    {
        asteroid->staticinfo = memAlloc(sizeof(AsteroidStaticInfo),"aststatic",NonVolatile);
        *((AsteroidStaticInfo *)(asteroid->staticinfo)) = asteroidStaticInfos[asteroid->asteroidtype];  // structure copy
        asteroid->flags |= SOF_StaticInfoIsDynamicCata;
    }

    newAsteroidStaticInfo = (AsteroidStaticInfo *)asteroid->staticinfo;

    newAsteroidStaticInfo->harvestableByMultipleShips = (udword)(((real32)originalAsteroidStaticInfo->harvestableByMultipleShips) * scaling);
    if (newAsteroidStaticInfo->harvestableByMultipleShips == 0)
    {
        newAsteroidStaticInfo->harvestableByMultipleShips = 1;
    }

    newStaticHeader = &newAsteroidStaticInfo->staticheader;
    originalStaticHeader = &originalAsteroidStaticInfo->staticheader;

    newStaticHeader->mass = originalStaticHeader->mass * scaling;
    newStaticHeader->momentOfInertiaX = originalStaticHeader->momentOfInertiaX * scaling;
    newStaticHeader->momentOfInertiaY = originalStaticHeader->momentOfInertiaY * scaling;
    newStaticHeader->momentOfInertiaZ = originalStaticHeader->momentOfInertiaZ * scaling;

    newStaticHeader->oneOverMass =             1.0f /      newStaticHeader->mass;
    newStaticHeader->oneOverMomentOfInertiaX = 1.0f /      newStaticHeader->momentOfInertiaX;
    newStaticHeader->oneOverMomentOfInertiaY = 1.0f /      newStaticHeader->momentOfInertiaY;
    newStaticHeader->oneOverMomentOfInertiaZ = 1.0f /      newStaticHeader->momentOfInertiaZ;

    newStaticCollInfo = &newStaticHeader->staticCollInfo;
    originalStaticCollInfo = &originalStaticHeader->staticCollInfo;

    newStaticCollInfo->originalcollspheresize = originalStaticCollInfo->originalcollspheresize;  // DO NOT CHANGE INTENTIONALLY
    newStaticCollInfo->approxcollspheresize = originalStaticCollInfo->approxcollspheresize * scaling;
    newStaticCollInfo->avoidcollspheresize = originalStaticCollInfo->avoidcollspheresize * scaling;
    newStaticCollInfo->avoidcollspherepad = originalStaticCollInfo->avoidcollspherepad * scaling;
    newStaticCollInfo->collspheresize = originalStaticCollInfo->collspheresize * scaling;
    newStaticCollInfo->collspheresizeSqr = newStaticCollInfo->collspheresize * newStaticCollInfo->collspheresize;

    vecScalarMultiply(newStaticCollInfo->collsphereoffset,originalStaticCollInfo->collsphereoffset,scaling);
    vecScalarMultiply(newStaticCollInfo->collrectoffset,originalStaticCollInfo->collrectoffset,scaling);

    newStaticCollInfo->forwardlength = originalStaticCollInfo->forwardlength * scaling;
    newStaticCollInfo->rightlength = originalStaticCollInfo->rightlength * scaling;
    newStaticCollInfo->uplength = originalStaticCollInfo->uplength * scaling;

    newStaticCollInfo->diagonallength = originalStaticCollInfo->diagonallength * scaling;

    collUpdateCollRectangle((SpaceObjRotImp *)asteroid);
}

/*-----------------------------------------------------------------------------
    Name        : MakeNewDustCloudStaticInfo
    Description : makes a new dustcloud staticinfo based on its scaling
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeNewDustCloudStaticInfo(DustCloud *dustcloud)
{
    DustCloudStaticInfo *originalDustCloudStaticInfo = &dustcloudStaticInfos[dustcloud->dustcloudtype];
    DustCloudStaticInfo *newDustCloudStaticInfo;
    StaticHeader *newStaticHeader;
    StaticHeader *originalStaticHeader;
    StaticCollInfo *newStaticCollInfo;
    StaticCollInfo *originalStaticCollInfo;
    real32 scaling = dustcloud->scaling;

    dbgAssert(scaling > 0.0f);

    if ((dustcloud->flags & SOF_StaticInfoIsDynamicCata) == 0)
    {
        dustcloud->staticinfo = memAlloc(sizeof(DustCloudStaticInfo),"dcloudstatic",NonVolatile);
        *((DustCloudStaticInfo *)(dustcloud->staticinfo)) = dustcloudStaticInfos[dustcloud->dustcloudtype];  // structure copy
        dustcloud->flags |= SOF_StaticInfoIsDynamicCata;
    }

    newDustCloudStaticInfo = (DustCloudStaticInfo *)dustcloud->staticinfo;

    newDustCloudStaticInfo->harvestableByMultipleShips = (udword)(((real32)originalDustCloudStaticInfo->harvestableByMultipleShips) * scaling);
    if (newDustCloudStaticInfo->harvestableByMultipleShips == 0)
    {
        newDustCloudStaticInfo->harvestableByMultipleShips = 1;
    }

    newStaticHeader = &newDustCloudStaticInfo->staticheader;
    originalStaticHeader = &originalDustCloudStaticInfo->staticheader;

    newStaticHeader->mass = originalStaticHeader->mass * scaling;
    newStaticHeader->momentOfInertiaX = originalStaticHeader->momentOfInertiaX * scaling;
    newStaticHeader->momentOfInertiaY = originalStaticHeader->momentOfInertiaY * scaling;
    newStaticHeader->momentOfInertiaZ = originalStaticHeader->momentOfInertiaZ * scaling;

    newStaticHeader->oneOverMass =             1.0f /      newStaticHeader->mass;
    newStaticHeader->oneOverMomentOfInertiaX = 1.0f /      newStaticHeader->momentOfInertiaX;
    newStaticHeader->oneOverMomentOfInertiaY = 1.0f /      newStaticHeader->momentOfInertiaY;
    newStaticHeader->oneOverMomentOfInertiaZ = 1.0f /      newStaticHeader->momentOfInertiaZ;

    newStaticCollInfo = &newStaticHeader->staticCollInfo;
    originalStaticCollInfo = &originalStaticHeader->staticCollInfo;

    newStaticCollInfo->originalcollspheresize = originalStaticCollInfo->originalcollspheresize;  // DO NOT CHANGE INTENTIONALLY
    newStaticCollInfo->approxcollspheresize = originalStaticCollInfo->approxcollspheresize * scaling;
    newStaticCollInfo->avoidcollspheresize = originalStaticCollInfo->avoidcollspheresize * scaling;
    newStaticCollInfo->avoidcollspherepad = originalStaticCollInfo->avoidcollspherepad * scaling;
    newStaticCollInfo->collspheresize = originalStaticCollInfo->collspheresize * scaling;
    newStaticCollInfo->collspheresizeSqr = newStaticCollInfo->collspheresize * newStaticCollInfo->collspheresize;

    vecScalarMultiply(newStaticCollInfo->collsphereoffset,originalStaticCollInfo->collsphereoffset,scaling);
    vecScalarMultiply(newStaticCollInfo->collrectoffset,originalStaticCollInfo->collrectoffset,scaling);

    newStaticCollInfo->forwardlength = originalStaticCollInfo->forwardlength * scaling;
    newStaticCollInfo->rightlength = originalStaticCollInfo->rightlength * scaling;
    newStaticCollInfo->uplength = originalStaticCollInfo->uplength * scaling;

    newStaticCollInfo->diagonallength = originalStaticCollInfo->diagonallength * scaling;

    collUpdateCollRectangle((SpaceObjRotImp *)dustcloud);
}

/*-----------------------------------------------------------------------------
    Name        : MakeNewGasCloudStaticInfo
    Description : makes a new gascloud staticinfo based on its scaling
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeNewGasCloudStaticInfo(GasCloud *gascloud)
{
    GasCloudStaticInfo *originalGasCloudStaticInfo = &gascloudStaticInfos[gascloud->gascloudtype];
    GasCloudStaticInfo *newGasCloudStaticInfo;
    StaticHeader *newStaticHeader;
    StaticHeader *originalStaticHeader;
    StaticCollInfo *newStaticCollInfo;
    StaticCollInfo *originalStaticCollInfo;
    real32 scaling = gascloud->scaling;

    dbgAssert(scaling > 0.0f);

    if ((gascloud->flags & SOF_StaticInfoIsDynamicCata) == 0)
    {
        gascloud->staticinfo = memAlloc(sizeof(GasCloudStaticInfo),"gcloudstatic",NonVolatile);
        *((GasCloudStaticInfo *)(gascloud->staticinfo)) = gascloudStaticInfos[gascloud->gascloudtype];  // structure copy
        gascloud->flags |= SOF_StaticInfoIsDynamicCata;
    }

    newGasCloudStaticInfo = (GasCloudStaticInfo *)gascloud->staticinfo;

    newGasCloudStaticInfo->harvestableByMultipleShips = (udword)(((real32)originalGasCloudStaticInfo->harvestableByMultipleShips) * scaling);
    if (newGasCloudStaticInfo->harvestableByMultipleShips == 0)
    {
        newGasCloudStaticInfo->harvestableByMultipleShips = 1;
    }

    newStaticHeader = &newGasCloudStaticInfo->staticheader;
    originalStaticHeader = &originalGasCloudStaticInfo->staticheader;

    newStaticHeader->mass = originalStaticHeader->mass * scaling;
    newStaticHeader->momentOfInertiaX = originalStaticHeader->momentOfInertiaX * scaling;
    newStaticHeader->momentOfInertiaY = originalStaticHeader->momentOfInertiaY * scaling;
    newStaticHeader->momentOfInertiaZ = originalStaticHeader->momentOfInertiaZ * scaling;

    newStaticHeader->oneOverMass =             1.0f /      newStaticHeader->mass;
    newStaticHeader->oneOverMomentOfInertiaX = 1.0f /      newStaticHeader->momentOfInertiaX;
    newStaticHeader->oneOverMomentOfInertiaY = 1.0f /      newStaticHeader->momentOfInertiaY;
    newStaticHeader->oneOverMomentOfInertiaZ = 1.0f /      newStaticHeader->momentOfInertiaZ;

    newStaticCollInfo = &newStaticHeader->staticCollInfo;
    originalStaticCollInfo = &originalStaticHeader->staticCollInfo;

    newStaticCollInfo->originalcollspheresize = originalStaticCollInfo->originalcollspheresize;  // DO NOT CHANGE INTENTIONALLY
    newStaticCollInfo->approxcollspheresize = originalStaticCollInfo->approxcollspheresize * scaling;
    newStaticCollInfo->avoidcollspheresize = originalStaticCollInfo->avoidcollspheresize * scaling;
    newStaticCollInfo->avoidcollspherepad = originalStaticCollInfo->avoidcollspherepad * scaling;
    newStaticCollInfo->collspheresize = originalStaticCollInfo->collspheresize * scaling;
    newStaticCollInfo->collspheresizeSqr = newStaticCollInfo->collspheresize * newStaticCollInfo->collspheresize;

    vecScalarMultiply(newStaticCollInfo->collsphereoffset,originalStaticCollInfo->collsphereoffset,scaling);
    vecScalarMultiply(newStaticCollInfo->collrectoffset,originalStaticCollInfo->collrectoffset,scaling);

    newStaticCollInfo->forwardlength = originalStaticCollInfo->forwardlength * scaling;
    newStaticCollInfo->rightlength = originalStaticCollInfo->rightlength * scaling;
    newStaticCollInfo->uplength = originalStaticCollInfo->uplength * scaling;

    newStaticCollInfo->diagonallength = originalStaticCollInfo->diagonallength * scaling;

    collUpdateCollRectangle((SpaceObjRotImp *)gascloud);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : ObjectsAreMovingCloserTogether
    Description : determines if objects are moving closer together based on
                  current positions and velocity
    Inputs      : ship1, ship2, curdistsquared (between ship1,ship2)
    Outputs     :
    Return      : TRUE if objects are moving closer together, FALSE otherwise
----------------------------------------------------------------------------*/
bool32 ObjectsAreMovingCloserTogether(SpaceObjRotImp *obj1,SpaceObjRotImp *obj2,real32 curdistsquared)
{
    vector newpos1,newpos2;
    vector d1,d2;
    vector newdist;

    vecScalarMultiply(d1, obj1->posinfo.velocity, universe.unknown1);
    vecScalarMultiply(d2, obj2->posinfo.velocity, universe.unknown1);
    vecAdd(newpos1,obj1->collInfo.collPosition,d1);
    vecAdd(newpos2,obj2->collInfo.collPosition,d2);
    vecSub(newdist,newpos2,newpos1);

    if (vecMagnitudeSquared(newdist) < curdistsquared)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : ApplyDamageToCollidingObjects
    Description : Applies collision damage to two colliding objects
    Inputs      : obj1, obj2, distvector, dist
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define HYPERING_IN_DAMAGE    twHyperingInDamage
#define HYPERED_INTO_DAMAGE   twHyperedIntoDamage

void ApplyDamageToCollidingObjects(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2,vector *distvector,real32 dist)
{
    real32 damage;
    real32 damage2;
    real32 mindamage;
    real32 maxdamage;
    StaticInfoHealth *staticobj1;
    StaticInfoHealth *staticobj2;
    real32 veltangentfactor;
    vector obj2VelRelToObj1;

    if ((obj1->attributes^obj2->attributes) & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
    {
        // if one or the other but not both objects have KillerCollDamage
        if (obj1->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
        {
            ApplyDamageToTarget(obj1,min(obj2->health,obj1->health),0,DEATH_Killed_By_Collision,99);
            ApplyDamageToTarget(obj2,(obj1->attributes & ATTRIBUTES_HeadShotKillerCollDamage) ? attrHEADSHOTKillerCollDamage : attrKillerCollDamage,0,DEATH_Killed_By_Collision,99); // and do lots of damage to thing it hit
            return;
        }
        else if (obj2->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))
        {
            ApplyDamageToTarget(obj2,min(obj2->health,obj1->health),0,DEATH_Killed_By_Collision,99);
            ApplyDamageToTarget(obj1,(obj2->attributes & ATTRIBUTES_HeadShotKillerCollDamage) ? attrHEADSHOTKillerCollDamage : attrKillerCollDamage,0,DEATH_Killed_By_Collision,99); // and do lots of damage to thing it hit
            return;
        }
    }

    vecSub(obj2VelRelToObj1,obj2->posinfo.velocity,obj1->posinfo.velocity);
    veltangentfactor = fabsf(vecDotProduct(*distvector, obj2VelRelToObj1));
    veltangentfactor /= (COLLDAMAGE_VELOCITY_TANGENT_SCALE * dist);
    // now veltangentfactor is equal to k * |vel| cos(theta) where theta is angle between distvector and obj2VelRelToObj1

    damage = min(obj1->health,obj2->health);
    damage = (damage * veltangentfactor);

    staticobj1 = (StaticInfoHealth *)obj1->staticinfo;
    staticobj2 = (StaticInfoHealth *)obj2->staticinfo;
    mindamage = min(staticobj1->minCollDamage,staticobj2->minCollDamage);
    maxdamage = min(staticobj1->maxCollDamage,staticobj2->maxCollDamage);

    if (damage < mindamage)
    {
        damage = mindamage;
    }
    else if (damage > maxdamage)
    {
        damage = maxdamage;
    }
    damage2 = damage;

    if(obj1->objtype == OBJ_ShipType)
    {
       if (((Ship *)obj1)->shiptype == FloatingCity)
       {
          return;       // Floating city's are peaceful and magical - don't do any collision damage
       }
       else
       {
          CommandToDo *obj1command=getShipAndItsCommand(&universe.mainCommandLayer,((Ship *)obj1));
          if(obj1command != NULL)
          {
             if(obj1command->ordertype.order == COMMAND_MP_HYPERSPACEING)
             {
                //if ship is hyperspaceing IN, modify its damage
                if(obj1command->hyperspaceState == COM_HYPERSPACE_IN)
                {
                   damage *= HYPERING_IN_DAMAGE;
                   damage2 *= HYPERED_INTO_DAMAGE;
                }
             }
          }
       }
    }

    if(obj2->objtype == OBJ_ShipType)
    {
       if (((Ship *)obj2)->shiptype == FloatingCity)
       {
          return;       // Floating city's are peaceful and magical - don't do any collision damage
       }
       else
       {
          CommandToDo *obj2command=getShipAndItsCommand(&universe.mainCommandLayer,((Ship *)obj2));
          if(obj2command != NULL)
          {
             if(obj2command->ordertype.order == COMMAND_MP_HYPERSPACEING)
             {
                //if ship is hyperspaceing IN, modify its damage
                if(obj2command->hyperspaceState == COM_HYPERSPACE_IN)
                {
                   damage2 *= HYPERING_IN_DAMAGE;
                   damage *= HYPERED_INTO_DAMAGE;
                }
             }
          }
       }
    }

    ApplyDamageToTarget(obj1,damage,0,DEATH_Killed_By_Collision,99);
    ApplyDamageToTarget(obj2,damage2,0,DEATH_Killed_By_Collision,99);
}

bool32 ObjectsHaveCollidedLately(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2)
{
    if ( ((universe.totaltimeelapsed - obj1->lasttimecollided) < COLLISION_CLEAR_TIME) &&
         ((universe.totaltimeelapsed - obj2->lasttimecollided) < COLLISION_CLEAR_TIME) )
    {
        return TRUE;
    }
    obj1->lasttimecollided =  universe.totaltimeelapsed;
    obj2->lasttimecollided =  universe.totaltimeelapsed;
    return FALSE;
}

bool32 ShouldDoGlancingCollision(SpaceObjRotImpTarg *fastobj,real32 fastobjvelmag2,SpaceObjRotImpTarg *slowobj,real32 slowobjvelmag2)
{
    // don't do glancing collision for kamikaze stuff
    if(fastobj->objtype == OBJ_ShipType)
        if(bitTest(((Ship *)fastobj)->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            return FALSE;

    if(slowobj->objtype == OBJ_ShipType)
        if(bitTest(((Ship *)slowobj)->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            return FALSE;

    if ((fastobjvelmag2/slowobjvelmag2) >= GLANCE_COLLISION_VEL_RATIO)
    {
        return TRUE;
    }

    if (vecDotProduct(fastobj->posinfo.velocity,slowobj->posinfo.velocity) < 0)
    {
        return TRUE;
    }

    return FALSE;
}

ubyte univApplyCollisionImpulse(
    SpaceObjRotImpTarg *obj1,
    SpaceObjRotImpTarg *obj2,
    real32 collisionRadius,
    vector *distVector,
    real32 dist)
{
    vector collisionForce;
    vector momentum1;
    vector momentum2;
    vector tangentialForce;
    vector fastestToSlowest;
    vector up = { 0.0f, 0.0f, 1.0f };
    real32 repulse;
    real32 obj1velmag2;
    real32 obj2velmag2;
    real32 fastobjvelmag2;
    real32 slowobjvelmag2;
    SpaceObjRotImpTarg *fastobj;
    SpaceObjRotImpTarg *slowobj;

    repulse = (collisionRadius - dist) * COLLISION_REPULSE_FACTOR / dist;
    if ((universe.totaltimeelapsed - obj1->lasttimecollided) < COLLISION_CLEAR_TIME &&
        (universe.totaltimeelapsed - obj2->lasttimecollided) < COLLISION_CLEAR_TIME)
    {
        vector repulseOverlap;

        vecScalarMultiply(repulseOverlap, *distVector, repulse);
        if (!obj2->staticinfo->staticheader.immobile &&
            !(obj2->attributes & ATTRIBUTES_KillerCollDamage))
        {
            vecAddTo(obj2->posinfo.force, repulseOverlap);
        }
        if (!obj1->staticinfo->staticheader.immobile &&
            !(obj1->attributes & ATTRIBUTES_KillerCollDamage))
        {
            vecSubFrom(obj1->posinfo.force, repulseOverlap);
        }
        return 16;
    }

    obj1->lasttimecollided = universe.totaltimeelapsed;
    obj2->lasttimecollided = universe.totaltimeelapsed;

    obj1velmag2 = vecMagnitudeSquared(obj1->posinfo.velocity);
    obj2velmag2 = vecMagnitudeSquared(obj2->posinfo.velocity);
    if (obj1velmag2 <= obj2velmag2)
    {
        fastobj = obj2;
        fastobjvelmag2 = obj2velmag2;
        slowobj = obj1;
        slowobjvelmag2 = obj1velmag2;
    }
    else
    {
        fastobj = obj1;
        fastobjvelmag2 = obj1velmag2;
        slowobj = obj2;
        slowobjvelmag2 = obj2velmag2;
    }

    vecScalarMultiply(momentum1, obj2->posinfo.velocity, obj2->staticinfo->staticheader.mass);
    vecScalarMultiply(momentum2, obj1->posinfo.velocity, obj1->staticinfo->staticheader.mass);
    vecSub(collisionForce, momentum1, momentum2);
    vecDivideByScalar(collisionForce, universe.unknown1, dist);

    if ((fastobj->objtype != OBJ_ShipType || ((((Ship *)fastobj)->specialFlags2) & BIT1) == 0) &&
        (slowobj->objtype != OBJ_ShipType || ((((Ship *)slowobj)->specialFlags2) & BIT1) == 0) &&
        ShouldDoGlancingCollision(fastobj, fastobjvelmag2, slowobj, slowobjvelmag2))
    {
        if (collisionForce.x == 0.0f && collisionForce.y == 0.0f)
        {
            collisionForce.x = 0.1f;
        }

        vecCrossProduct(tangentialForce, collisionForce, up);
        vecNormalizeToLength(&tangentialForce, fsqrt(vecMagnitudeSquared(collisionForce)));

        vecSub(fastestToSlowest, slowobj->collInfo.collPosition, fastobj->collInfo.collPosition);
        if (fastestToSlowest.x * fastobj->posinfo.velocity.y -
            fastestToSlowest.y * fastobj->posinfo.velocity.x <= 0.0f)
        {
            collisionForce = tangentialForce;
        }
        else
        {
            vecCopyAndNegate(collisionForce, tangentialForce);
        }
    }

    if (!fastobj->staticinfo->staticheader.immobile &&
        !(fastobj->attributes & ATTRIBUTES_KillerCollDamage))
    {
        vecAddTo(fastobj->posinfo.force, collisionForce);
    }
    if (!slowobj->staticinfo->staticheader.immobile &&
        !(slowobj->attributes & ATTRIBUTES_KillerCollDamage))
    {
        vecSubFrom(slowobj->posinfo.force, collisionForce);
    }

    return 16;
}

/*-----------------------------------------------------------------------------
    Name        : ObjectsCollided
    Description : Handles momentum transferral of objects colliding
    Inputs      : obj1, obj2, distvector, dist, distsquared
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ObjectsCollided(SpaceObjRotImpTarg *obj1,SpaceObjRotImpTarg *obj2,real32 colldist,vector *distvector,real32 dist,real32 distsquared)
{
    vector collforce;
    vector cforce1;
    vector cforce2;
    vector repulseoverlap;
    real32 temp;
    real32 repulse = (colldist - dist) * COLLISION_REPULSE_FACTOR / dist;
    vector up = { 0.0f, 0.0f, 1.0f };
    real32 collforcemag;
    vector tangforce;

    real32 obj1velmag2;
    real32 obj2velmag2;

    real32 slowobjvelmag2;
    real32 fastobjvelmag2;

    SpaceObjRotImpTarg *slowobj;
    SpaceObjRotImpTarg *fastobj;

    vector fastesttoslowest;

    vector leftrightdecidevec;

#ifdef DEBUG_COLLISIONS
    dbgMessagef("\n%x and %x collided, Repulsion %f",(udword)obj1,(udword)obj2,repulse);
#endif

    if (!ObjectsHaveCollidedLately(obj1,obj2))
    {
        // decide which ship is fastest ship in collision

        obj1velmag2 = vecMagnitudeSquared(obj1->posinfo.velocity);
        obj2velmag2 = vecMagnitudeSquared(obj2->posinfo.velocity);

        if (obj1velmag2 > obj2velmag2)
        {
            fastobj = obj1; fastobjvelmag2 = obj1velmag2;
            slowobj = obj2; slowobjvelmag2 = obj2velmag2;
        }
        else
        {
            fastobj = obj2; fastobjvelmag2 = obj2velmag2;
            slowobj = obj1; slowobjvelmag2 = obj1velmag2;
        }

        // collforce = (m2v2 - m1v1)/t
        vecScalarMultiply(cforce1,obj2->posinfo.velocity,obj2->staticinfo->staticheader.mass);
        vecScalarMultiply(cforce2,obj1->posinfo.velocity,obj1->staticinfo->staticheader.mass);
        vecSub(collforce,cforce1,cforce2);
        vecDivideByScalar(collforce, universe.unknown1, temp);

        if (ShouldDoGlancingCollision(fastobj,fastobjvelmag2,slowobj,slowobjvelmag2))
        {
#ifdef DEBUG_COLLISIONS
        dbgMessage("\nGlancing Collision");
#endif
            if ((collforce.x == 0.0f) && (collforce.y == 0.0f))
            {
                collforce.x = 0.1f;     // so cross product with up doesn't blow up
            }

            collforcemag = fsqrt(vecMagnitudeSquared(collforce));

            // make force tangential:

            vecCrossProduct(tangforce,collforce,up);
            vecNormalizeToLength(&tangforce,collforcemag);

            // decide which way to apply tangential force based on if "velocity" vector of fastest obj is left or right
            // of vector from fastestobj to slowestobj

            vecSub(fastesttoslowest,slowobj->collInfo.collPosition,fastobj->collInfo.collPosition);

            vecCrossProduct(leftrightdecidevec,fastesttoslowest,fastobj->posinfo.velocity);

            if (leftrightdecidevec.z > 0)
            {
                vecCopyAndNegate(collforce,tangforce);
            }
            else
            {
                collforce = tangforce;
            }
        }
        else
        {
#ifdef DEBUG_COLLISIONS
        dbgMessage("\nXchanging Velocities");
#endif
        }

        if (!fastobj->staticinfo->staticheader.immobile)
        {
            if(!(fastobj->attributes & ATTRIBUTES_KillerCollDamage))
            {
                vecAddTo(fastobj->posinfo.force,collforce);
            }
        }
        if (!slowobj->staticinfo->staticheader.immobile)
        {
            if(!(slowobj->attributes & ATTRIBUTES_KillerCollDamage))
            {
                vecSubFrom(slowobj->posinfo.force,collforce);
            }
        }
    }
    else
    {
        // add a repulsion factor if objects overlap, so they don't stick together
        vecScalarMultiply(repulseoverlap,*distvector,repulse);
        if (!obj2->staticinfo->staticheader.immobile)
        {
            if(!(obj2->attributes & ATTRIBUTES_KillerCollDamage))
            {
                vecAddTo(obj2->posinfo.force,repulseoverlap);
            }
        }
        if (!obj1->staticinfo->staticheader.immobile)
        {
            if(!(obj1->attributes & ATTRIBUTES_KillerCollDamage))
            {
               vecSubFrom(obj1->posinfo.force,repulseoverlap);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : ApplyCareen
    Description : Applies careen (random spin) to a ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ApplyCareen(Ship *ship)
{
    // apply three random torques, to ship,

    ship->rotinfo.torque.x += frandombetween(YAWCAREENLO,YAWCAREENHI);
    ship->rotinfo.torque.y += frandombetween(PITCHCAREENLO,PITCHCAREENHI);
    ship->rotinfo.torque.z += frandombetween(ROLLCAREENLO,ROLLCAREENHI);
}

void ApplyCareenRotationDirectly(Ship *ship)
{
    StaticHeader *staticheader = &ship->staticinfo->staticheader;
    vector a;

    // apply three random torques, to ship,

    a.x = frandombetween(YAWCRAZYLO, YAWCRAZYHI) * universe.unknown1; // * staticheader->oneOverMomentOfInertiaX;
    a.y = frandombetween(PITCHCRAZYLO, PITCHCRAZYHI) * universe.unknown1; // * staticheader->oneOverMomentOfInertiaY;
    a.z = frandombetween(ROLLCRAZYLO, ROLLCRAZYHI) * universe.unknown1; // * staticheader->oneOverMomentOfInertiaZ;

    // w = w + alpha*t
    vecAddTo(ship->rotinfo.rotspeed,a);

    // cap rotation speed
    vecCapVectorSloppy(&ship->rotinfo.rotspeed,staticheader->maxrot);
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToDeleteShipList
    Description : adds ship to the DeleteShip list (for deletion at end of main loop)
    Inputs      : ship, damageType
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToDeleteShipList(Ship *ship,GunSoundType damageType)
{
    DeleteShip *deleteShip;
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

    if (shipstaticinfo->shiptype == sMothership)
    {
        crystalBeginDeath(ship, damageType);
        return;
    }

    ship->flags |= SOF_Dead;

    if (!shipstaticinfo->staticheader.immobile)
    {
        ship->rotinfo.torque.x += frandombetween(YAWCRAZYLO, YAWCRAZYHI) * universe.unknown1;
        ship->rotinfo.torque.y += frandombetween(PITCHCRAZYLO, PITCHCRAZYHI) * universe.unknown1;
        ship->rotinfo.torque.z += frandombetween(ROLLCRAZYLO, ROLLCRAZYHI) * universe.unknown1;
    }

    //if (shipstaticinfo->shipclass != CLASS_Fighter)
        MakeExplosionRockBumpables(ship,(damageType == -1));

    // add to delete ship list
    deleteShip = memAlloc(sizeof(DeleteShip),"DeleteShip",Pyrophoric);
    deleteShip->ship = ship;
    if (ship->health <= etgBigDeathFactor[shipstaticinfo->shiprace][shipstaticinfo->shipclass])
    {                                   //ship took a really big bong hit
        deleteShip->deathBy = (udword)etgDeathModeByGunType[damageType];
    }
    else
    {                                   //if it just barely died, use the small explosion
        deleteShip->deathBy = EDT_AccumDamage;
    }
    listAddNode(&universe.DeleteShipList,&(deleteShip->objlink),deleteShip);
}

/*-----------------------------------------------------------------------------
    Name        : AddResourceToDeleteResourceList
    Description : adds resource to the DeleteResource list (for deletion at end of main loop)
    Inputs      : resource
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddResourceToDeleteResourceList(Resource *resource)
{
    // add this resource to the delete resource list
    DeleteResource *deleteResource = memAlloc(sizeof(DeleteResource),"DeleteResource",Pyrophoric);

    resource->flags |= SOF_Dead;

    deleteResource->resource = resource;
    listAddNode(&universe.DeleteResourceList,&(deleteResource->objlink),deleteResource);
}

/*-----------------------------------------------------------------------------
    Name        : AddDerelictToDeleteDerelictList
    Description : adds derelict to the DeleteDerelict list (for deletion at end of main loop)
    Inputs      : derelict
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddDerelictToDeleteDerelictList(Derelict *derelict,GunSoundType damageType)
{
    // add this derelict to the delete derelict list
    DeleteDerelict *deleteDerelict = memAlloc(sizeof(DeleteDerelict),"DeleteDerelict",Pyrophoric);

    bobObjectDied((SpaceObj *)derelict,&universe.collBlobList);//can't have dead derelicts in the blobs
    derelict->flags |= SOF_Dead;

    deleteDerelict->derelict = derelict;
    if (derelict->health <= etgBigDeathFactorDerelict[derelict->derelicttype])
    {                                   //derelict took a really big bong hit
        deleteDerelict->deathBy = (udword)etgDeathModeByGunType[damageType];
    }
    else
    {                                   //if it just barely died, use the small explosion
        deleteDerelict->deathBy = EDT_AccumDamage;
    }
    listAddNode(&universe.DeleteDerelictList,&(deleteDerelict->objlink),deleteDerelict);
    //univRemoveFromWorldList(derelict);                      //it may be a planet; remove from the planet list if so.
}

/*-----------------------------------------------------------------------------
    Name        : AddMissileToDeleteMissileList
    Description : adds missile to the DeleteMissile list (for deletion at end of main loop)
    Inputs      : missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddMissileToDeleteMissileList(Missile *missile,GunSoundType damageType)
{
    // add this missile to the delete missile list
    DeleteMissile *deleteMissile = memAlloc(sizeof(DeleteMissile),"DeleteMissile",Pyrophoric);

    missile->flags |= SOF_Dead;

    deleteMissile->missile = missile;
    deleteMissile->deathBy = damageType;
    listAddNode(&universe.DeleteMissileList,&(deleteMissile->objlink),deleteMissile);
}

/*-----------------------------------------------------------------------------
    Name        : AddTargetToDeleteList
    Description : adds target (ship, asteroid, missile) to appropriate delete list
    Inputs      : target, soundType (indicating type of death)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddTargetToDeleteList(SpaceObjRotImpTarg *target,GunSoundType soundType)
{
    switch (target->objtype)
    {
        case OBJ_ShipType:
            AddShipToDeleteShipList((Ship *)target,soundType);
            break;

        case OBJ_AsteroidType:
        case OBJ_NebulaType:
        case OBJ_GasType:
        case OBJ_DustType:
            AddResourceToDeleteResourceList((Resource *)target);
            break;

        case OBJ_MissileType:
            AddMissileToDeleteMissileList((Missile *)target, soundType);
            break;

        case OBJ_DerelictType:
            AddDerelictToDeleteDerelictList((Derelict *)target, soundType);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

void ApplyExplosionRockToTarget(SpaceObjRotImp *obj,real32 blaststrength)
{
    obj->rotinfo.torque.x += frandombetween(YAWEXPLOSIONROCKLO,YAWEXPLOSIONROCKHI) * blaststrength;
    obj->rotinfo.torque.y += frandombetween(PITCHEXPLOSIONROCKLO,PITCHEXPLOSIONROCKHI) * blaststrength;
    obj->rotinfo.torque.z += frandombetween(ROLLEXPLOSIONROCKLO,ROLLEXPLOSIONROCKHI) * blaststrength;
}

void ApplyRockToTarget(SpaceObjRotImpTarg *target,real32 projectilemass)
{
    target->rotinfo.torque.x += frandombetween(YAWROCKLO,YAWROCKHI) * projectilemass;
    target->rotinfo.torque.y += frandombetween(PITCHROCKLO,PITCHROCKHI) * projectilemass;
    target->rotinfo.torque.z += frandombetween(ROLLROCKLO,ROLLROCKHI) * projectilemass;
}

/*-----------------------------------------------------------------------------
    Name        : MakeExplosionRockBumpables
    Description : Make all impactable objects within blast radius get rocked
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeExplosionRockBumpables(Ship *ship,bool32 scuttle)
{
    Node *objnode = universe.ImpactableList.head;
    ShipStaticInfo *shipstatic = ship->staticinfo;
    SpaceObjRotImp *obj;
    vector distvector;
    real32 distsquared;
    real32 dist;
    real32 blastradius = shipstatic->staticheader.staticCollInfo.collspheresize * shipstatic->blastRadiusShockWave *
                         (scuttle ? BLAST_RADIUS_SCUTTLE_BONUS : 1.0f);
    real32 blastradiussqr = blastradius * blastradius;
    real32 blaststrength;
    real32 damage;
    real32 damageperc;

    while (objnode != NULL)
    {
        obj = (SpaceObjRotImp *) listGetStructOfNode(objnode);

        if ((obj->flags & (SOF_Impactable|SOF_NotBumpable)) != (SOF_Impactable))
        {
            goto nextnode;      // object must be impactable, bumpable
        }

        vecSub(distvector,obj->collInfo.collPosition,ship->collInfo.collPosition);

        if (!isBetweenInclusive(distvector.x,-blastradius,blastradius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.y,-blastradius,blastradius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.z,-blastradius,blastradius))
        {
            goto nextnode;
        }

        distsquared = vecMagnitudeSquared(distvector);
        if (distsquared > blastradiussqr)
        {
            goto nextnode;
        }

        if (obj == (SpaceObjRotImp *)ship)
        {
            goto nextnode;
        }

        if (obj->staticinfo->staticheader.immobile)
        {
            goto nextnode;
        }

        // obj is within blast radius, so it should get rocked

        if (distsquared < 1.0f)
        {
            distsquared = 1.0f;
        }
        dist = fsqrt(distsquared);
        blaststrength = 1.0f - (dist / blastradius);

        vecNormalizeToLength(&distvector,(scuttle ? BLAST_CONSTANT_SCUTTLE : BLAST_CONSTANT) * blaststrength);

        ApplyExplosionRockToTarget(obj,blaststrength);
        physApplyForceVectorToObj(obj,distvector);

        if ((obj->flags & (SOF_Targetable | SOF_ForwardDamageToClampHost)) == SOF_Targetable)
        {
            // calculate damage based on ship blowing up
            real32 health = (shipstatic->blastRadiusDamage != 0.0f) ? shipstatic->blastRadiusDamage : shipstatic->maxhealth;

            damageperc = (scuttle ? BLAST_DAMAGE_MAX_PERCENT_SCUTTLE : BLAST_DAMAGE_MAX_PERCENT) * blaststrength;
            damage =  health * damageperc;

            if (damage > 0)
            {
                if(ship->playerowner != NULL)
                    ApplyDamageToTarget((SpaceObjRotImpTarg *)obj,damage,0,DEATH_Killed_By_Player_Explosion,ship->playerowner->playerIndex);
                else
                    ApplyDamageToTarget((SpaceObjRotImpTarg *)obj,damage,0,DEATH_Killed_By_Dead_Player,99);
            }
        }

nextnode:
        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ApplyDamageToTarget
    Description : applies damage to target, possible resulting in target being added
                  to the delete list.
    Inputs      : damageHow - Defines how the damaged was taken:
                              DEATH_Killed_By_Collision    - collision
                              DEATH_Killed_By_Player - player ship attacking
                              (anything else is assumed to be something else and essentially nothing
                              to worry about)
                  playerIndex = playerIndex of playerdoing damage
    Outputs     :
    Return      : returns TRUE if damage was fatal damage
----------------------------------------------------------------------------*/
bool32 ApplyDamageToTarget(SpaceObjRotImpTarg *target,real32 damagetaken,GunSoundType soundType,sdword damageHow,sdword playerIndex)
{
    bool32 targetWasAlive = ((target->flags & SOF_Dead) == 0);
    real32 previousHealth;
    real32 currentHealth;
    real32 maxHealth;

    previousHealth = (target->objtype == OBJ_ShipType) ? ((Ship *)target)->health : target->health;

    if (target->objtype == OBJ_ShipType)
    {
        Ship *targetShip = (Ship *)target;

        if (targetShip->staticinfo->shipclass == CLASS_Component &&
            bitTest(targetShip->flags, SOF_ForwardDamageToClampHost) &&
            targetShip->clampInfo != NULL &&
            targetShip->clampInfo->host != NULL)
        {
            target = targetShip->clampInfo->host;
        }

        univRetreatShipForRepairs((Ship *)target);
    }

    if (target->objtype == OBJ_ShipType)
    {
        if (nisIsRunning || bitTest(((Ship *)target)->flags2, SOF2_Unknown4))
        {
            return FALSE;
        }
        ((Ship *)target)->health -= damagetaken;
    }
    else
    {
        target->health -= damagetaken;
    }
    currentHealth = (target->objtype == OBJ_ShipType) ? ((Ship *)target)->health : target->health;
    switch (target->objtype)
    {
        case OBJ_AsteroidType:
            if (AsteroidTakesDamage((Asteroid *)target,(sdword)damagetaken,targetWasAlive))
            {
                AddTargetToDeleteList(target,soundType);
                return TRUE;                    //used by effect
            }
            else
            {
                return FALSE;
            }
            break;

        case OBJ_DustType:
            switch (soundType)
            {
            case GS_LargeIonCannon:
            case GS_MediumIonCannon:
            case GS_SmallIonCannon:
            case GS_VeryLargeIonCannon:
                trailSegAddDamage((DustCloud *)target, (sdword)damagetaken, targetWasAlive);
                return FALSE;
            }

#if 0
            if (DustCloudTakesDamage((DustCloud*)target, (sdword)damagetaken, targetWasAlive))
            {
                AddTargetToDeleteList(target, soundType);
                return TRUE;
            }
            else
#endif
            {
                return FALSE;
            }
            break;

        case OBJ_DerelictType:
            if (((Derelict *)target)->derelicttype == HyperspaceGate)
            {
                hsDerelictTakesDamage((Derelict *)target);
            }
            goto fallthrough;

        case OBJ_ShipType:
            switch(damageHow)
            {
                case DEATH_Killed_By_Player:
                case DEATH_Killed_By_Kamikaze:
                case DEATH_Killed_By_Player_Explosion:
                    universe.gameStats.playerStats[playerIndex].pointsOfDamageDoneToWho[((Ship *)target)->playerowner->playerIndex] += damagetaken;
                    break;
            }
            maxHealth = ((ShipStaticInfo *)target->staticinfo)->maxhealth;
            if (previousHealth /  maxHealth >= BAT_InTheRedFactor)
            {                                               //it wasn't in the red
                if (currentHealth / maxHealth < BAT_InTheRedFactor)
                {                                           //if it's just now in the red
                    bitSet(((Ship *)target)->chatterBits, BCB_DamageInTheRed);//flag as such
                }
            }
        // purposefully fall through
fallthrough:;
        default:
            if ((currentHealth <= 0) && (targetWasAlive))      // target just died, so add to DeleteShipList
            {
                if(target->objtype == OBJ_ShipType)
                {
                    ((Ship *)target)->howDidIDie = damageHow;
                    ((Ship *)target)->whoKilledMe = playerIndex; //if was collision death, who cares
                    bountyShipWasKilled((Ship *)target);
                }
                AddTargetToDeleteList(target,soundType);
                return TRUE;                    //used by effect
            }
            else
            {
                return FALSE;
            }
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pointExplosionInSpace(vector *position,real32 blastRadius)
    Description : Make all impactable objects within blast radius get rocked
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pointExplosionInSpace(vector *position,real32 blastRadius,real32 maxDamage,sdword playerIndex)
{
    Node *objnode = universe.ImpactableList.head;
    SpaceObjRotImp *obj;
    vector distvector;
    real32 distsquared;
    real32 dist;
    //real32 blastradius = shipstatic->staticheader.staticCollInfo.collspheresize * shipstatic->blastRadiusShockWave;
    real32 blastradiussqr = blastRadius * blastRadius;

    real32 blaststrength;

    while (objnode != NULL)
    {
        obj = (SpaceObjRotImp *) listGetStructOfNode(objnode);

        if ((obj->flags & (SOF_Impactable|SOF_NotBumpable)) != (SOF_Impactable))
        {
            goto nextnode;      // object must be impactable, bumpable
        }

        vecSub(distvector,obj->collInfo.collPosition,*position);

        if (!isBetweenInclusive(distvector.x,-blastRadius,blastRadius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.y,-blastRadius,blastRadius))
        {
            goto nextnode;
        }

        if (!isBetweenInclusive(distvector.z,-blastRadius,blastRadius))
        {
            goto nextnode;
        }

        distsquared = vecMagnitudeSquared(distvector);
        if (distsquared > blastradiussqr)
        {
            goto nextnode;
        }

        if (obj->staticinfo->staticheader.immobile)
        {
            goto nextnode;
        }

        // obj is within blast radius, so it should get rocked

        if (distsquared < 1.0f)
        {
            distsquared = 1.0f;
        }
        dist = fsqrt(distsquared);
        blaststrength = 1.0f - (dist / blastRadius);

        vecNormalizeToLength(&distvector,BLAST_CONSTANT * blaststrength);

        ApplyExplosionRockToTarget(obj,blaststrength);
        vecScalarMultiply(distvector,distvector,TW_BURST_ATTACK_FORCE_MODIFIER);
        physApplyForceVectorToObj(obj,distvector);

        if (obj->flags & SOF_Targetable)
        {
            // calculate damage based on blast as a % of maxhealth
            ApplyDamageToTarget((SpaceObjRotImpTarg *)obj,maxDamage,0,DEATH_Killed_By_Player_Explosion,playerIndex);
        }

nextnode:
        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univBulletCollidedWithTarget
    Description : Call when bullet collides with a target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univBulletCollidedWithTarget(SpaceObjRotImpTarg *target,StaticHeader *targetstaticheader,Bullet *bullet,real32 collideLineDist,sdword collSide)
{
    real32 damagetaken = bullet->damage;
    Ship *bulletowner = (Ship *)bullet->owner;
    real32 massovertime;
    vector recoil;
    etgeffectstatic *stat;
    sdword LOD;
    vector LOF;
    bool32 fatalHit;
    vector hitLocation;
    matrix hitCoordsys;
    real32 floatDamage;
    real32 damageMult = 1.0f;
    udword intDamage;
    real32 targetScale;
    // if target isn't immobile, apply impact force from bullet
    if (!targetstaticheader->immobile && !bitTest(target->flags, SOF_NotBumpable))
    {
        if(target->objtype == OBJ_ShipType )
        {
            if((((Ship *)target)->specialFlags1C8 & BIT28) != 0)
            {
                goto nobulletmasstransfer;
            }
        }
        massovertime = bullet->bulletmass / universe.unknown1;
        recoil = bullet->posinfo.velocity;
        vecMultiplyByScalar(recoil,massovertime);

        ApplyRockToTarget(target,bullet->bulletmass);
        physApplyForceVectorToObj(target,recoil);
    }

nobulletmasstransfer:
    if (univBulletUsesSiegeShockwave(bullet))
    {
        shockwaveProjectileBulletHit(&bullet->posinfo.position, bullet);
    }

    if (target->objtype == OBJ_ShipType)
    {
        damageMult = ((Ship *)target)->magnitudeSquared;

        // apply different damage modifier based on collision side:
        if (collSide >= 0)
        {
            dbgAssert(collSide < NUM_TRANS_DEGOFFREEDOM);
            damagetaken *= ((Ship *)target)->staticinfo->collSideModifiers[collSide];
        }

        if (bulletowner != NULL)
        {
            if ((bulletowner->shiptype == kHeavyDefender) && (((ShipStaticInfo *)target->staticinfo)->shipclass == CLASS_Fighter))
            {
                // armour piercing bullets vs fighters
                damagetaken *= armourPiercingModifier;
            }

            if (bulletowner->playerowner == ((Ship *)target)->playerowner)
            {                                               //if it's friendly fire
        //!!! why only in non-debug???
#ifndef HW_Debug
                damagetaken *= friendlyFireModifier;
#endif
                bitSet(((Ship *)target)->chatterBits, BCB_FriendlyFire);
            }
            else if (allianceIsShipAlly((Ship *)target, bulletowner->playerowner))
            {
                bitSet(((Ship *)target)->chatterBits, BCB_AlliedFire);
            }
            else
            {
                speechEventUnderAttack((Ship *)target);
            }
            ((Ship *)target)->gettingrocked = bulletowner;

            ((Ship *)target)->recentAttacker = bulletowner;
            ((Ship *)target)->recentlyAttacked = RECENT_ATTACK_DURATION;  // start counting down
        }
        else
        {
            speechEventUnderAttack((Ship *)target);
        }
    }

    if(bulletowner != NULL && bulletowner->playerowner != NULL)
        fatalHit = ApplyDamageToTarget(target,damagetaken,bullet->soundType,DEATH_Killed_By_Player,bulletowner->playerowner->playerIndex);
    else
        fatalHit = ApplyDamageToTarget(target,damagetaken,bullet->soundType,DEATH_Killed_By_Dead_Player,99);
    //delete the bullet effect
#if ETG_DISABLEABLE
    if (bullet->effect != NULL &&
        etgEffectsEnabled &&
        (WeaponType)bullet->bulletType != WEAPON_Ion &&
        (WeaponType)bullet->bulletType != WEAPON_BeastInfection)
#else
    if (bullet->effect != NULL &&
        (WeaponType)bullet->bulletType != WEAPON_Ion &&
        (WeaponType)bullet->bulletType != WEAPON_BeastInfection)
#endif
    {
        etgEffectDelete(bullet->effect);
        univRemoveObjFromRenderList((SpaceObj *)bullet->effect);
        listDeleteNode(&bullet->effect->objlink);
        bullet->effect = NULL;
    }

    if ((WeaponType)bullet->bulletType != WEAPON_Ion)
    {
        if (collideLineDist == 0.0f)
        {                                                   //if it's a beam weapon and there is no collision vector
            return;                                         //don't play any hit effects
        }
    }
    //play the imact effect
    if (bullet->hitEffect != NULL)
    {
        LOD = target->currentLOD;
        if (LOD < bullet->hitEffect->nLevels)
        {
            stat = bullet->hitEffect->level[LOD];
        }
        else
        {
            stat = NULL;
        }
    }
    else
    {
        stat = NULL;
    }
    if (univSpaceObjInRenderList((SpaceObj *)target))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && (!etgFrequencyExceeded(stat)))
#else
        if (stat != NULL && etgHitEffectsEnabled && (!etgFrequencyExceeded(stat)))
#endif
        {
            if (collideLineDist == 0.0f)
            {
                hitLocation = bullet->posinfo.position;
            }
            else
            {
                dbgAssert(collideLineDist > 0.0f);
                vecScalarMultiply(hitLocation,bullet->bulletheading,collideLineDist);
                vecAdd(hitLocation,bullet->posinfo.position,hitLocation);
            }
            //N-Lip the impact position
            if (target->objtype == OBJ_ShipType)
            {
                targetScale = ((Ship *)target)->magnitudeSquared;
                vecSubFrom(hitLocation, target->posinfo.position);
                vecMultiplyByScalar(hitLocation, targetScale);
                vecAddTo(hitLocation, target->posinfo.position);
            }
            else
            {
                targetScale = 1.0f;
            }

            vecCopyAndNegate(LOF,bullet->bulletheading);
            matCreateCoordSysFromHeading(&hitCoordsys,&LOF);
            floatDamage = (real32)damagetaken;
            if (RGLtype == SWtype)
            {                                               //smaller hit effects in software
                floatDamage *= etgSoftwareScalarHit;
            }
            floatDamage *= damageMult;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, NULL, &hitLocation, &target->posinfo.velocity, &hitCoordsys, targetScale, 0, 2, intDamage, fatalHit);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univMissileCollidedWithTarget
    Description : Call when a missile collides with a target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univMissileCollidedWithTarget(SpaceObjRotImpTarg *target,StaticHeader *targetstaticheader,Missile *missile,real32 collideLineDist,sdword collSide)
{
    real32 damagetaken = missile->damage;
    Ship *missileowner = missile->owner;
    MissileStaticInfo *missilestaticinfo = (MissileStaticInfo *)missile->staticinfo;
    real32 massovertime;
    vector recoil;
    etgeffectstatic *stat;
    sdword LOD;
    bool32 fatalHit;
    real32 floatDamage;
    udword intDamage;
    vector missileheading;
    vector hitLocation;
    real32 targetScale;
    real32 damageMult = 1.0f;

    // if target isn't immobile, apply impact force from missile
    if (!targetstaticheader->immobile && !bitTest(target->flags, SOF_NotBumpable))
    {
        massovertime = missilestaticinfo->staticheader.mass / universe.unknown1;
        recoil = missile->posinfo.velocity;
        vecMultiplyByScalar(recoil,massovertime);

        ApplyRockToTarget(target,missilestaticinfo->staticheader.mass);
        physApplyForceVectorToObj(target,recoil);
    }

    if (target->objtype == OBJ_ShipType)
    {
        damageMult = ((Ship *)target)->magnitudeSquared;
        // apply different damage modifier based on collision side:
        if (collSide >= 0)
        {
            dbgAssert(collSide < NUM_TRANS_DEGOFFREEDOM);
            damagetaken *= ((Ship *)target)->staticinfo->collSideModifiers[collSide];
        }

        if (missileowner != NULL)
        {
            if (missileowner->playerowner == ((Ship *)target)->playerowner)
            {                                               //if it's friendly fire
        //!!! why only in non-debug???
#ifndef HW_Debug
                damagetaken *= friendlyFireModifier;
#endif
                bitSet(((Ship *)target)->chatterBits, BCB_FriendlyFire);
            }
            else if (allianceIsShipAlly((Ship *)target, missileowner->playerowner))
            {
                bitSet(((Ship *)target)->chatterBits, BCB_AlliedFire);
            }
            else
            {
                speechEventUnderAttack((Ship *)target);
            }
            ((Ship *)target)->gettingrocked = missileowner;

            ((Ship *)target)->recentAttacker = missileowner;
            ((Ship *)target)->recentlyAttacked = RECENT_ATTACK_DURATION;  // start counting down
        }
        else
        {
            speechEventUnderAttack((Ship *)target);
        }
    }

    if(missileowner != NULL && missileowner->playerowner != NULL)
        fatalHit = ApplyDamageToTarget(target,damagetaken,missile->soundType,DEATH_Killed_By_Player,missileowner->playerowner->playerIndex);
    else
        fatalHit = ApplyDamageToTarget(target,damagetaken,missile->soundType,DEATH_Killed_By_Dead_Player,99);

    if (missile->hitEffect != NULL)
    {
        LOD = target->currentLOD;
        if (LOD < missile->hitEffect->nLevels)
        {
            stat = missile->hitEffect->level[LOD];
        }
        else
        {
            stat = NULL;
        }
    }
    else
    {
        stat = NULL;
    }
    if (univSpaceObjInRenderList((SpaceObj *)target))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && (!etgFrequencyExceeded(stat)))
#else
        if (stat != NULL && etgHitEffectsEnabled && (!etgFrequencyExceeded(stat)))
#endif
        {
            if (collideLineDist == 0.0f)
            {
                hitLocation = missile->posinfo.position;
            }
            else
            {
                dbgAssert(collideLineDist > 0.0f);
                matGetVectFromMatrixCol3(missileheading,missile->rotinfo.coordsys);
                vecScalarMultiply(hitLocation,missileheading,collideLineDist);
                vecAdd(hitLocation,missile->posinfo.position,hitLocation);
            }
            //N-Lip the impact position
            if (target->objtype == OBJ_ShipType)
            {
                targetScale = ((Ship *)target)->magnitudeSquared;
                vecSubFrom(hitLocation, target->posinfo.position);
                vecMultiplyByScalar(hitLocation, targetScale);
                vecAddTo(hitLocation, target->posinfo.position);
            }
            else
            {
                targetScale = 1.0f;
            }

            floatDamage = (real32)damagetaken;
            if (RGLtype == SWtype)
            {                                               //smaller hit effects in software
                floatDamage *= etgSoftwareScalarHit;
            }
            floatDamage *= damageMult;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, NULL, &hitLocation, &target->posinfo.velocity, &missile->rotinfo.coordsys, targetScale, 0, 2, intDamage, fatalHit);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllTargetReferencesFromBullets
    Description : removes all ship references from any bullets
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllTargetReferencesFromBullets(SpaceObjRotImpTarg *target)
{
    Node *objnodebullet = universe.BulletList.head;
    Bullet *bullet;

    // Update bullet->owner for all bullets whose owner is now dead
    while (objnodebullet != NULL)
    {
        bullet = (Bullet *)listGetStructOfNode(objnodebullet);
        dbgAssert(bullet->objtype == OBJ_BulletType);

        if (bullet->owner == (Ship *)target)
        {
            bullet->owner = NULL;       // bullet has no owner now that parent ship died
            bullet->gunowner = NULL;
            if ((WeaponType)bullet->bulletType == WEAPON_Ion)
            {
                bullet->timelived += 10000.0f;  // delete bullet if it is a beam weapon and target wielding it died
            }
        }

        if (bullet->target == target)
        {
            bullet->target = NULL;
        }

        objnodebullet = objnodebullet->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllTargetReferencesFromMissiles
    Description : removes all target references from any missiles
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllTargetReferencesFromMissiles(SpaceObjRotImpTarg *target)
{
    Node *objnodemissile = universe.MissileList.head;
    Missile *missile;

    // Update missile->owner for all missiles whose owner is now dead
    while (objnodemissile != NULL)
    {
        missile = (Missile *)listGetStructOfNode(objnodemissile);
        dbgAssert(missile->objtype == OBJ_MissileType);

        if (missile->owner == (Ship *)target)
        {
            missile->owner = NULL;       // missile has no owner now that parent ship died
        }

        if (missile->target == target)
        {
            missile->target = NULL;
            if (missile->maxtraveldist - missile->timelived > 5.0f)
            {
                missile->maxtraveldist = missile->timelived + 5.0f;
            }
        }

        objnodemissile = objnodemissile->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllDerelictReferencesFromSpecialShips
    Description : removes all derelict references from special ships
    Inputs      : d - derelictto remove
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllDerelictReferencesFromSpecialShips(Derelict *d)
{
    Node *node = universe.ShipList.head;
    Ship *ship;
    InsideShip *insideShip;

    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if(ship->specialFlags & SPECIAL_IsASalvager)
            salCapRemoveDerelictReferences(ship,d);

        node = node->next;
    }

    // walk through ships in hyperspace too!

    node = singlePlayerGameInfo.ShipsInHyperspace.head;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if(ship->specialFlags & SPECIAL_IsASalvager)
            salCapRemoveDerelictReferences(ship,d);

        node = node->next;
    }
}

void univRemoveAllShipReferencesFromSpecialShip(Ship *shiptoremove,Ship *ship)
{
    if (ship->recentAttacker == shiptoremove)
    {
        ship->recentlyAttacked = 0;
        ship->recentAttacker = NULL;
    }
    if (ship->firingAtUs == shiptoremove)
    {
        ship->recentlyFiredUpon = 0;
        ship->firingAtUs = NULL;
    }

    if (ship->gettingrocked == shiptoremove)
    {
        ship->gettingrocked = NULL;
    }

    if (ship->rowGetOutOfWay == shiptoremove)
    {
        rowGetOutOfWayShipDiedCB(ship);
        ship->rowGetOutOfWay = NULL;
    }

    if (ship->specialTargetShip == shiptoremove)
    {
        ship->specialTargetShip = NULL;
    }

    if (ship->beastAttack.sourceShip == shiptoremove)
    {
        ship->beastAttack.sourceShip = NULL;
    }

    if (((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipRemoveShipReferences != NULL)
    {
        ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipRemoveShipReferences(ship, shiptoremove);
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveAllShipReferencesFromSpecialShips
    Description : removes all ship references from special ships
    Inputs      : shiptoremove
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveAllShipReferencesFromSpecialShips(Ship *shiptoremove)
{
    Node *node = universe.ShipList.head;
    Node *insideNode;
    Ship *ship;
    InsideShip *insideShip;
    ShipsInsideMe *shipsInsideMe;

    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        univRemoveAllShipReferencesFromSpecialShip(shiptoremove,ship);

        if (shiptoremove == ship->dockvars.dockship)
        {
            ship->dockvars.dockship = NULL;
        }

        if ((void *)shiptoremove == (void *)ship->dockvars.launchpoints)
        {
            ship->dockvars.launchpoints = NULL;
        }

        shipsInsideMe = ship->shipsInsideMe;
        if (shipsInsideMe != NULL)
        {
            insideNode = shipsInsideMe->insideList.head;
            while (insideNode != NULL)
            {
                insideShip = (InsideShip *)listGetStructOfNode(insideNode);
                if (insideShip->ship == ship)
                {
                    listDeleteNode(insideNode);
                    lmUpdateShipsInside();

                    if (isShipOfClass(shiptoremove, CLASS_Fighter) || isShipOfClass(shiptoremove, CLASS_Micro))
                    {
                        shipsInsideMe->FightersInsideme--;
                    }
                    else if (isShipOfClass(shiptoremove, CLASS_Corvette) || isShipOfClass(shiptoremove, CLASS_Resource))
                    {
                        shipsInsideMe->CorvettesInsideme--;
                    }
                    break;
                }
                insideNode = insideNode->next;
            }
        }

        node = node->next;
    }

    // walk through ships in hyperspace too!

    node = singlePlayerGameInfo.ShipsInHyperspace.head;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        univRemoveAllShipReferencesFromSpecialShip(shiptoremove,ship);

        if (shiptoremove == ship->dockvars.dockship)
        {
            ship->dockvars.dockship = NULL;
        }

        node = node->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : DeleteShipsInsideMe
    Description : deletes ships that are inside me
    Inputs      : ship which contains other ships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeleteShipsInsideMe(Ship *ship)
{
    InsideShip *insideShip;
    ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;
    Node *node = shipsInsideMe->insideList.head;

    while (node != NULL)
    {
        insideShip = listGetStructOfNode(node);

        dbgAssert(insideShip->ship);
        insideShip->ship->howDidIDie = ship->howDidIDie;
        insideShip->ship->whoKilledMe = ship->whoKilledMe;
        univDeleteWipeInsideShipOutOfExistence(insideShip->ship);

        node = node->next;
    }

    listDeleteAll(&shipsInsideMe->insideList);
    memFree(shipsInsideMe);
    ship->shipsInsideMe = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : FreeShipsInsideMe
    Description : frees ships contents that are inside me
    Inputs      : ship which contains other ships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FreeShipsInsideMe(Ship *ship)
{
    InsideShip *insideShip;
    ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;
    Node *node = shipsInsideMe->insideList.head;

    while (node != NULL)
    {
        insideShip = listGetStructOfNode(node);

        dbgAssert(insideShip->ship);
        insideShip->ship->howDidIDie = ship->howDidIDie;
        insideShip->ship->whoKilledMe = ship->whoKilledMe;
        if (insideShip->ship->objlink.belongto != NULL)
        {
            listRemoveNode(&insideShip->ship->objlink);
        }
        univRemoveObjFromRenderList((SpaceObj *)insideShip->ship);
        univFreeShipContents(insideShip->ship);
        memFree(insideShip->ship);

        node = node->next;
    }

    listDeleteAll(&shipsInsideMe->insideList);
    memFree(shipsInsideMe);
    ship->shipsInsideMe = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : univWipeShipOutOfExistence
    Description : totally destroys and wipes this ship out of memory, and
                  removes any remaining references.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univWipeShipOutOfExistence(Ship *ship)
{
    sdword savedSensorCoverage;

    dbgAssert(ship != NULL);

    if (playerGetSensorCoverage(ship->playerowner))
    {
        savedSensorCoverage = univSetCurrentSensorCoverage(playerGetSensorCoverage(ship->playerowner));
        if (physShipIsMovable(ship))
        {
            real32 radius = univGetMaxHealth(ship, FALSE);
            fogRemoveSphereFromGrid(
                ship->posinfo.position.x,
                ship->posinfo.position.y,
                ship->posinfo.position.z,
                radius);
        }
        univSetCurrentSensorCoverage(savedSensorCoverage);
    }

    if (ship->beastAttack.effect != NULL)
    {
        memFree(ship->beastAttack.effect);
        ship->beastAttack.effect = NULL;
    }
    if (ship->replacementEffect != NULL)
    {
        memFree(ship->replacementEffect);
        ship->replacementEffect = NULL;
    }

    if (ship->shiplink.belongto != NULL)
    {
        listRemoveNode(&ship->shiplink);
    }
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    nisObjectDied((SpaceObj *)ship);
    etgShipDied(ship);
    if (ship->renderlink.belongto != NULL)
    {
        listRemoveNode(&ship->renderlink);
    }
    univFreeShipContents(ship);
    if (univWipeShipDirectFreeOverride)
    {
        univWipeShipDirectFreeOverride = 0;
        memFree(ship);
    }
    else if (ship->objlink.belongto != NULL)
    {
        listDeleteNode(&ship->objlink);
    }
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteWipeInsideShipOutOfExistence
    Description : totally destroys and wipes a "ship that was inside another ship"
                  out of memory.
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteWipeInsideShipOutOfExistence(Ship *ship)
{
    if (ship->shiplink.belongto != NULL)
    {
        listRemoveNode(&ship->shiplink);
    }
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    if (ship->objlink.belongto != NULL)
    {
        listRemoveNode(&ship->objlink);
    }

    etgShipDied(ship);
    soundEventShipRemoveHandles(ship);
    pieShipDied(ship);
    mouseClickShipDied(ship);
    clShipDied(&universe.mainCommandLayer, ship);
    univRemoveAllShipReferencesFromSpecialShips(ship);
    univFreeShipContents(ship);
    memFree(ship);
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveShipFromOutside
    Description : removes a ship from the universe and from mission spheres so that it
                  can be placed inside a ship
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveShipFromOutside(Ship *ship)
{
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)ship);

    shipHasJustDisappeared(ship);

    ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)ship);

    clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)ship);
    clRemoveTargetFromSelection((SelectAnyCommand *)&attackcom, (TargetPtr)ship);
    ioUpdateShipTotals();

    smFocusRemoveObj((SpaceObj *)ship);

    bobObjectDied((SpaceObj *)ship,&universe.collBlobList);
    pingObjectDied((SpaceObj *)ship);
    ship->collMyBlob = NULL;

    pieShipDied(ship);
    mouseClickShipDied(ship);
    if (!singlePlayerGame)
    {
        aiplayerShipDied(ship);
    }

    if (ship->shiplink.belongto != NULL)
    {
        listRemoveNode(&ship->shiplink);
    }
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    if (ship->objlink.belongto != NULL)
    {
        listRemoveNode(&ship->objlink);
    }
    if (ship->renderlink.belongto != NULL)
    {
        listRemoveNode(&ship->renderlink);
    }

    soundEventShipRemoveHandles(ship);

    ship->flags |= SOF_DockMarker;
    if (ship->shipsInsideMe != NULL)
    {
        Node *insideNode = ship->shipsInsideMe->insideList.head;
        InsideShip *insideShip;

        while (insideNode != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(insideNode);
            if (insideShip->ship->renderlink.belongto != NULL)
            {
                listRemoveNode(&insideShip->ship->renderlink);
            }
            insideShip->ship->flags |= SOF_DockMarker;
            insideNode = insideNode->next;
        }
    }

    ship->recentAttacker = NULL;
    ship->recentlyAttacked = 0;
    ship->firingAtUs = NULL;
    ship->recentlyFiredUpon = 0;
    ship->gettingrocked = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveShipFromHotkeyGroup
    Description : Removes ship from hotkey group
    Inputs      : ship
    Outputs     : if ship was removed or not.
    Return      : bool8
----------------------------------------------------------------------------*/
bool8 univRemoveShipFromHotkeyGroup(Ship *ship, bool8 PlayDefeat)
{
    udword i;

    if (selAnyHotKeyTest(ship))
    {                                                       //if ship in any hot key groups
        // Remove ships that are destroyed from any hot key groups
        for (i=0;i<SEL_NumberHotKeyGroups;i++)
        {
            if (selHotKeyTest(ship, i))
            {                                               //if it's in this hot-key group
                if (clRemoveShipFromSelection((SelectCommand *)&selHotKeyGroup[i],ship))
                {
                    if (selHotKeyGroup[i].numShips == 0)
                    {
                        selHotKeyGroup[i].timeLastStatus = universe.totaltimeelapsed;
                    }
                }
            }
        }

        ship->hotKeyGroup = 0;

        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : univHideShipFromSpheres
    Description : Removes a ship from all mission spheres but not from the universe
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univHideShipFromSpheres(Ship *ship)
{
    univRemoveObjFromRenderList((SpaceObj *)ship);
    ship->flags |= SOF_Hide;
}

/*-----------------------------------------------------------------------------
    Name        : univHideJustAboutEverything
    Description : Sets the SOF_Hide bit on evertything that is not a resource
                    and does not have the SOF_NISShip bit set.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univHideJustAboutEverything(void)
{
    Node *node;
    SpaceObj *obj;

    for (node = universe.SpaceObjList.head; node != NULL; node = node->next)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        if (!bitTest(obj->flags, SOF_NISShip))
        {
            switch (obj->objtype)
            {
                case OBJ_ShipType:
                case OBJ_BulletType:
                case OBJ_DerelictType:
                case OBJ_EffectType:
                case OBJ_MissileType:
                    if (bitTest(obj->flags, SOF_Hide))
                    {
                        bitSet(obj->flags, SOF_PreviouslyHiddenByHideAll);
                    }
                    else
                    {
                        univRemoveObjFromRenderList(obj);
                        bitSet(obj->flags, SOF_Hide);
                    }
                    break;
                case OBJ_AsteroidType:
                case OBJ_NebulaType:
                case OBJ_GasType:
                case OBJ_DustType:
                    break;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univUnhideJustAboutEverything
    Description : Unhide everything that was hidden in a previous call
                    to univHideJustAboutEverything.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univUnhideJustAboutEverything(void)
{
    Node *node;
    SpaceObj *obj;

    for (node = universe.SpaceObjList.head; node != NULL; node = node->next)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        if (bitTest(obj->flags, SOF_PreviouslyHiddenByHideAll))
        {                                                   //if it was already hidden last univHideJustAboutEverything
            bitClear(obj->flags, SOF_PreviouslyHiddenByHideAll);
        }
        else
        {
            bitClear(obj->flags, SOF_Hide);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadShip
    Description : Removes all references to the ship, except for a few ones
                  to let the ship hang around after the explosion.
    Inputs      : ship
                  deathby - mode of death, or -1 for no death effects
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadShip(Ship *ship, sdword deathBy)
{
    Effect *effect;
    etgeffectstatic *explosion;
    etglod *etgLOD;
    sdword LOD;
    sdword bigDeathLength;
    real32 colSize;
    real32 bigDeathPercent;
    real32 bigDeathMaxX, bigDeathMaxY, bigDeathMaxZ;
    udword colSizeDword, velMagnitudeDword;
    real32 velMagnitude, maxVelocity;
    bool32 useBigDeathEffect;
    ShipStaticInfo *staticinfo;
    sdword canBuildShipsRaw;

    vecZeroVector(ship->posinfo.force);
    vecZeroVector(ship->rotinfo.torque);

    ship->flags = (ship->flags & ~(SOF_Selectable | 0x800)) | SOF_Dead;

    if (singlePlayerGame && ship == SalCapGetProcessingShip())
        SalCapSetProcessingShip(NULL);

    if (((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipClose != NULL)
        ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipClose(ship);

    staticinfo = ship->staticinfo;
    if (staticinfo->canBuildShips && (!singlePlayerGame || !ship->playerowner->playerIndex))
        cmRemoveFactory(ship);

    // Raw disasm of univDeleteDeadShip (0x547d92: `mov al, [ecx+1F0h]`) tests
    // ShipStaticInfo+0x1F0, which is externalBuild (confirmed via
    // static_assert(offsetof(ShipStaticInfo, externalBuild) == 0x1F0)), not
    // rotateToRetaliate (0x204) - Hex-Rays mislabeled this field in the original
    // decompile. sCarrierSupport/sMothershipSpecial/sMothershipBigGun all set
    // "externalBuild TRUE" in their .shp scripts and rely on this gate to call
    // cmUntrackExternalBuildShip when they die mid-construction.
    if (staticinfo->externalBuild)
    {
        cmUntrackExternalBuildShip(ship);
        ship->specialFlags2 &= ~SPECIAL_2_DockFlag;
    }

    if (staticinfo->externalBuild && ship->dockvars.dockship)
        dockWorkerClampFunc(ship);

    if (ship->rceffect != NULL)
        clDeleteCommandEffect((CommandToDo *)ship);

    if (lmActive)
    {
        canBuildShipsRaw = *(sdword *)&staticinfo->canBuildShips;
        if (canBuildShipsRaw == 2 || !canBuildShipsRaw)
            lmLaunchCapableShipDied(ship);
    }

    tacticsRemoveShipFromRetreatList(ship);

    if (ship->shipSinglePlayerGameInfo != NULL)
    {
        LinkedList *midlevelHyperspaceIn = (LinkedList *)ship->shipSinglePlayerGameInfo;
        Node *node = midlevelHyperspaceIn->head;

        while (node != NULL)
        {
            Node *next = node->next;
            InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
            Ship *childShip = insideShip->ship;

            childShip->howDidIDie = ship->howDidIDie;
            childShip->whoKilledMe = ship->whoKilledMe;

            if (childShip->shiplink.belongto != NULL)
            {
                listRemoveNode(&childShip->shiplink);
            }
            if (childShip->impactablelink.belongto != NULL)
            {
                listRemoveNode(&childShip->impactablelink);
            }
            if (childShip->objlink.belongto != NULL)
            {
                listRemoveNode(&childShip->objlink);
            }

            etgShipDied(childShip);
            soundEventShipRemoveHandles(childShip);
            pieShipDied(childShip);
            mouseClickShipDied(childShip);
            clShipDied(&universe.mainCommandLayer, childShip);
            univRemoveAllShipReferencesFromSpecialShips(childShip);
            univFreeShipContents(childShip);
            memFree(childShip);

            node = next;
        }

        listDeleteAll(midlevelHyperspaceIn);
        memFree(ship->shipSinglePlayerGameInfo);
        ship->shipSinglePlayerGameInfo = NULL;
    }
    else if (ship->shipsInsideMe != NULL)
    {
        DeleteShipsInsideMe(ship);
    }

    deleteRepairDroidData((RepairDroidController *)ship->runtimeAnimationOverlay.runtimeController);
    ship->runtimeAnimationOverlay.runtimeController = 0;

#if ETG_DISABLEABLE
    if (deathBy >= 0 && etgEffectsEnabled)
#else
    if (deathBy >= 0)
#endif
    {
        if (deathBy == EDT_AccumDamage)
        {
            battleShipDyingWithTimeToScream(ship);
        }
//        if (univSpaceObjInRenderList((SpaceObj *)ship))
//        {
            useBigDeathEffect = univShipUsesBigDeathEffect(ship->shiptype, &bigDeathLength, &bigDeathPercent);
            if (useBigDeathEffect)
            {
                etgLOD = etgDeathEventTable[((ShipStaticInfo *)ship->staticinfo)->shiprace]
                    [CLASS_Mothership][deathBy];
            }
            else
            {
                etgLOD = etgDeathEventTable[((ShipStaticInfo *)ship->staticinfo)->shiprace]
                    [((ShipStaticInfo *)ship->staticinfo)->shipclass][deathBy];
            }
            if (etgLOD != NULL)
            {
                LOD = ship->currentLOD;
                explosion = etgLOD->level[min(etgLOD->nLevels - 1, LOD)];
            }
            else
            {
                explosion = NULL;
            }
            if (explosion)
            {
                colSize = ((ShipStaticInfo *)(ship->staticinfo))->staticheader.staticCollInfo.collspheresize;
//                if (univSpaceObjInRenderList((SpaceObj *)ship))
//                {
                    colSize *= ship->magnitudeSquared;
//                }
                colSizeDword = TreatAsUdword(colSize);

                maxVelocity = (real32)(shipGetMaxSpeed(ship) * ship->staticinfo->staticheader.maxvelocity);
                if (maxVelocity != 0.0f)
                {
                    velMagnitude = fsqrt(vecMagnitudeSquared(ship->posinfo.velocity));
                    velMagnitude /= maxVelocity;
                }
                else
                {
                    velMagnitude = 0;
                }
                velMagnitudeDword = TreatAsUdword(velMagnitude);

                if (useBigDeathEffect)
                {
                    bigDeathMaxX = ship->staticinfo->staticheader.staticCollInfo.collrectoffset.x + ship->staticinfo->staticheader.staticCollInfo.uplength;
                    bigDeathMaxY = ship->staticinfo->staticheader.staticCollInfo.collrectoffset.y + ship->staticinfo->staticheader.staticCollInfo.rightlength;
                    bigDeathMaxZ = ship->staticinfo->staticheader.staticCollInfo.collrectoffset.z + ship->staticinfo->staticheader.staticCollInfo.forwardlength;

                    if (RGLtype == SWtype)
                    {
                        if (bigDeathLength == 1)
                        {
                            bigDeathLength = 0;
                        }
                        else if (bigDeathLength <= 3)
                        {
                            bigDeathLength = 1;
                        }
                        colSize *= LOD_ScaleFactor;
                        bigDeathPercent *= LOD_ScaleFactor;
                        colSizeDword = TreatAsUdword(colSize);
                    }

                    effect = etgEffectCreate(
                        explosion,
                        ship,
                        NULL,
                        NULL,
                        NULL,
                        1.0f,
                        0,
                        10,
                        TreatAsUdword(bigDeathPercent),
                        bigDeathLength,
                        colSizeDword,
                        velMagnitudeDword,
                        TreatAsUdword(ship->staticinfo->staticheader.staticCollInfo.collrectoffset.x),
                        TreatAsUdword(ship->staticinfo->staticheader.staticCollInfo.collrectoffset.y),
                        TreatAsUdword(ship->staticinfo->staticheader.staticCollInfo.collrectoffset.z),
                        TreatAsUdword(bigDeathMaxX),
                        TreatAsUdword(bigDeathMaxY),
                        TreatAsUdword(bigDeathMaxZ));
                }
                else
                {
                    //!!!??? should there be some EAF_ here???
                    effect = etgEffectCreate(explosion, ship, NULL, NULL, NULL, 1.0f, 0, 2, colSizeDword, velMagnitudeDword);
                }
                /*
                if (!univSpaceObjInRenderList((SpaceObj *)effect))
                {
                    univAddObjToRenderList((SpaceObj *)effect);//make sure it's in render list
                }
                */
                bitSet(effect->flags, SOF_ForceVisible);
            }
//        }
    }

    //notify leader of a lost wingman
    if (ship->tacticstype == Evasive)
    {                                               //if in evasive tactics
        if (ship->attackvars.myLeaderIs != NULL)
        {                                           //if ship has a wingman
            bitSet(ship->attackvars.myLeaderIs->chatterBits, BCB_WingmanLost);
        }
    }

    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)ship);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)ship);

    smFocusRemoveObj((SpaceObj *)ship);

    bobObjectDied((SpaceObj *)ship, &universe.collBlobList);
    pingObjectDied((SpaceObj *)ship);
    ship->collMyBlob = NULL;

    clShipDied(&universe.mainCommandLayer, ship);

    univRemoveAllShipReferencesFromSpecialShips(ship);
    univDetachShipsInsideFromCommands(ship);
    clDetachShipFromCommand(ship);

    aiplayerShipDied(ship);

    ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)ship);

    singlePlayerShipDied(ship);

    if (tutorial)
        tutPlayerShipDied(ship);

    if (deathBy != -1)
        soundEventShipDied(ship);

    IDToPtrTableObjDied(&ShipIDToPtr, ship->shipID.shipNumber);

    growSelectRemoveShipBySettingNULL(&universe.HousekeepShipList, ship);
    growSelectRemoveShipBySettingNULL((GrowSelection *)&selSelected, ship);
    clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)ship);
    clRemoveTargetFromSelection((SelectAnyCommand *)&attackcom, (TargetPtr)ship);
    ioUpdateShipTotals();

    pieShipDied(ship);
    mouseClickShipDied(ship);

    // Remove ships that are destroyed from any hot key groups and play group death sound if all dead
    if (ship->hotKeyGroup & 0x7FE0)
    {
        sdword i;
        for (i = 0; i < SEL_NumberHotKeyGroups; i++)
        {
            if (((udword)1 << (i + 5)) & ship->hotKeyGroup)
                if (clRemoveShipFromSelection((SelectCommand *)&selHotKeyGroup[i], ship))
                    if (selHotKeyGroup[i].numShips == 0)
                    {
                        selHotKeyGroup[i].timeLastStatus = universe.phystimeelapsed;
                    }
        }
        ship->hotKeyGroup = 0;
    }

//if no effects, we should delete it properly
    /*
#if ETG_DISABLEABLE
    if (!etgEffectsEnabled)
    {
        univWipeShipOutOfExistence(ship);
    }
#endif
*/
}

void DisableAllPlayersShips(sdword playerIndex)
{
    Node *objnode;
    Ship *ship;
    Player *player;
    sdword i;

    if (playerIndex >= 8)
    {
        return;
    }

    objnode = universe.ShipList.head;
    player = &universe.players[playerIndex];

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if (ship->playerowner == player)
        {
            if ((ship->flags & 0x00800000) == 0)
            {
                ship->flags |= SOF_Crazy;
                ship->deathtime = universe.totaltimeelapsed + frandombetween(PLAYERLOSE_SHIPDEATHTIMEMIN,PLAYERLOSE_SHIPDEATHTIMEMAX);
                if (ship->flags2 & BIT3)
                {
                    shipDisable(ship, FALSE);
                }
                if (!singlePlayerGame && ship->shiptype != sMothership && ship->shiptype != bMothership)
                {
                    ApplyCareenRotationDirectly(ship);
                }
            }

            bitClear(ship->flags, SOF_Selectable);
            clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)ship);
            if (ship->hotKeyGroup & 0x7FE0)
            {
                for (i = 0; i < SEL_NumberHotKeyGroups; i++)
                {
                    if (((udword)1 << (i + 5)) & ship->hotKeyGroup)
                    {
                        if (clRemoveTargetFromSelection((SelectAnyCommand *)&selHotKeyGroup[i], (TargetPtr)ship) &&
                            !selHotKeyGroup[i].numShips)
                        {
                            selHotKeyGroup[i].timeLastStatus = universe.totaltimeelapsed;
                        }
                    }
                }
                ship->hotKeyGroup = 0;
            }
        }

        objnode = objnode->next;
    }
    ioUpdateShipTotals();
    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }
}

void DisplayPlayerHasBeenDefeatedMessage(sdword playerIndex)
{
    char defeatmsg[100];

    strcpy(defeatmsg,playerNames[playerIndex]);
    strcat(defeatmsg," ");
    strcat(defeatmsg,strGetString(strPlayerHasDied));
    clCommandMessage(defeatmsg, (color)0xFF0000FF);
}

void CheckPlayerWin(void)
{
    udword numPlayersAlive = 0;
    udword i;

    if (gatherStats|showStatsFight|showStatsFancyFight)
        return;     // just gathering stats, don't want to do the player win thing

    if (!singlePlayerGame)
    {
        // check how many players are still alive
        for (i=0;i<universe.numPlayers;i++)
        {
            if (universe.players[i].playerState == PLAYER_ALIVE)
            {
                numPlayersAlive++;
            }
        }

        if (numPlayersAlive <= 1)
        {
            // end of game
            universe.wintime = universe.totaltimeelapsed + WAIT_TIME_TO_WIN;
            universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
        }

        //check for allied victory
        if((!singlePlayerGame) && (tpGameCreated.flag & MG_AlliedVictory))
        {
            sdword ally = -1;
            bool32 alliedvictory = TRUE;
            //all living players on same team?
            //find allies of first player, make sure all other living players
            // are allies
            universe.players[sigsPlayerIndex].Allies;
            for (i=0; ((i<universe.numPlayers) && alliedvictory); i++)
            {
                if (universe.players[i].playerState == PLAYER_ALIVE)
                {
                    if (ally == -1)
                    {
                        //first player
                        ally = i;

                    }
                    else
                    {
                        if ( !((1<<i) & universe.players[ally].Allies) )
                        {

                            alliedvictory = FALSE;
                        }
                    }
                }
            }
            if (alliedvictory)
            {
                // end of game
                universe.wintime = universe.totaltimeelapsed + WAIT_TIME_TO_WIN;
                universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
            }
        }
    }
    else
    {
        if (universe.players[0].playerState == PLAYER_DEAD)
        {
            // end of game
            universe.wintime = universe.totaltimeelapsed + WAIT_TIME_TO_WIN;
            universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
        }
    }
}

void DeclarePlayerWin(void)
{
    udword numPlayersAlive = 0;
    udword i, count=0;
    char winmsg[200];

    if (singlePlayerGame)
    {
        return;
    }

    // check how many players are still alive
    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].playerState == PLAYER_ALIVE)
        {
            numPlayersAlive++;
        }
    }

    if (numPlayersAlive == 0)
    {
        bigmessageDisplay(strGetString(strAllPlayersDead),1);
    }
    else
    {
        sdword otherplayersdied = 0;
        sdword otherplayersdroppedout = 0;

        strcpy(winmsg,"");
        for (i=0;i<universe.numPlayers;i++)
        {
            if (universe.players[i].playerState == PLAYER_ALIVE)
            {
                strcat(winmsg,playerNames[i]);
                strcat(winmsg," ");
                count++;
                if ((numPlayersAlive-count)>1)
                {
                    strcat(winmsg, ", ");
                } else if ((numPlayersAlive-count)==1)
                {
                    strcat(winmsg, strGetString(strAnd));
                    strcat(winmsg, " ");
                }
            }
            else
            {
                if (i != universe.curPlayerIndex)
                {
                    otherplayersdied++;
                    if (universe.gameStats.playerStats[i].typeOfPlayerDeath == PLAYERKILLED_DROPPEDOUT)
                    {
                        otherplayersdroppedout++;
                    }
                }
            }
        }

        if ((otherplayersdroppedout > 0) && (otherplayersdroppedout == otherplayersdied))
        {
            bigmessageDisplay(strGetString(strAllOtherPlayersDroppedOut),1);
        }
        else
        {
            strcat(winmsg," ");
            if (numPlayersAlive > 1)
                strcat(winmsg,strGetString(strPlayersHaveWon));
            else
                strcat(winmsg,strGetString(strPlayerHasWon));
            bigmessageDisplay(winmsg,1);
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : univFindBackupMothership
    Description : Looks through the universeshiplist to find out if the
                  player who's mothership died has a backup mothership
    Inputs      : player - the player who's mothership just died
    Outputs     :
    Return      : TRUE if a backup was found
----------------------------------------------------------------------------*/
bool32 univFindBackupMothership(struct Player *player)
{
    Node *node;
    Ship *ship;
    Ship *currentMothership = player->PlayerMothership;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        ship = (Ship *)listGetStructOfNode(node);
        if (((ship->flags & SOF_Dead) == 0) &&
            (ship->playerowner == player) &&
            (ship != currentMothership) &&
            (ShipTypeIsCarrierType(ship->shiptype) || (ship->staticinfo->shipclass == CLASS_Mothership)))
        {
            if ((currentMothership != NULL) &&
                gameIsRunning &&
                ShipTypeIsMothershipType(currentMothership->shiptype))
            {
                speechEventFleet(STAT_F_ReassignedToCarrier, 0, player->playerIndex);
            }

            player->PlayerMothership = ship;
            return TRUE;
        }
    }

    if (singlePlayerGame || !bitTest(tpGameCreated.flag, MG_CaptureCapitalShip))
    {
        return FALSE;
    }

    for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        if (((ship->flags & SOF_Dead) == 0) &&
            (ship->playerowner == player) &&
            (ship != currentMothership) &&
            (ShipTypeIsCarrierType(ship->shiptype) || (ship->staticinfo->shipclass == CLASS_Mothership)))
        {
            player->PlayerMothership = ship;
            return TRUE;
        }
    }

    return FALSE;
}

void univKillOtherPlayersIfDead(Ship *ship)
{
    Player *player;
    sdword i;

    i = ship->playerowner->playerIndex;
    if ((uword)i >= MAX_MULTIPLAYER_PLAYERS)
    {
        return;
    }

    player = &universe.players[i];

    if (ship == player->PlayerMothership)
    {
        if (singlePlayerGame)
        {
            if (i != 0)
            {
                player->PlayerMothership = NULL;
            }
            else
            {
                univKillPlayer(i, PLAYERKILLED_STANDARDDEATH);
                CheckPlayerWin();
            }
            return;
        }

        if (univFindBackupMothership(player))
        {
            return;
        }

        if (!bitTest(tpGameCreated.flag, MG_LastMotherShip) && player->totalships != 0)
        {
            if (player == universe.curPlayerPtr)
            {
                rmCloseIfOpen();
                cmCloseIfOpen();
                lmCloseIfOpen();
            }
            player->PlayerMothership = NULL;
            return;
        }

        univKillPlayer(i, PLAYERKILLED_STANDARDDEATH);
        CheckPlayerWin();
        return;
    }

    if ((player->totalships == 0) && !singlePlayerGame)
    {
        Node *node;
        bool32 allDead = TRUE;

        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *otherShip = (Ship *)listGetStructOfNode(node);
            if (((otherShip->flags & SOF_Dead) == 0) && (otherShip->playerowner == player))
            {
                allDead = FALSE;
                break;
            }
        }

        if (allDead && bitTest(tpGameCreated.flag, MG_CaptureCapitalShip))
        {
            for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
            {
                InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
                Ship *otherShip = insideShip->ship;
                if (((otherShip->flags & SOF_Dead) == 0) && (otherShip->playerowner == player))
                {
                    allDead = FALSE;
                    break;
                }
            }
        }

        if (allDead)
        {
            univKillPlayer(i, PLAYERKILLED_STANDARDDEATH);
            CheckPlayerWin();
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadShips
    Description : deletes all dead ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadShips()
{
    Node *objnode;
    DeleteShip *deleteShip;
    Ship *ship;

    for (objnode = universe.DeleteShipList.head; universe.DeleteShipList.head != NULL; objnode = universe.DeleteShipList.head)
    {
        deleteShip = (DeleteShip *)listGetStructOfNode(objnode);
        ship = deleteShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        univDeleteDeadShip(ship,deleteShip->deathBy);
        if (ship->impactablelink.belongto != NULL)
        {
            listRemoveNode(&ship->impactablelink);
        }

        if (ship->staticinfo->shipclass == CLASS_Resource)
        {
            if (ship->fuel > 0)
            {
                DustCloud *dustCloud = univAddDustCloud(DustCloud0,&ship->posinfo.position);
                dustCloud->resourcevalue = (sdword)(ship->fuel * DUSTCLOUD_RCOLLECTOR_BLOWUP_SCALE);
                HandleDustCloudScaling(dustCloud);

                if (ship->renderlink.belongto != NULL)
                {
                    if ((dustCloud->flags & SOF_Hide) == 0)
                    {
                        if (bitTest(dustCloud->flags, SOF_AlwaysSortFront))
                        {
                            univAddObjToRenderList((SpaceObj *)dustCloud);
                        }
                        else
                        {
                            dustCloud->cameraDistanceVector = ship->cameraDistanceVector;
                            dustCloud->cameraDistanceSquared = ship->cameraDistanceSquared;
                            listAddNodeAfter(&ship->renderlink, &dustCloud->renderlink, dustCloud);
                        }
                    }
                }
            }
        }

        univKillOtherPlayersIfDead(ship);

        // don't totally delete ship, until after explosion
#if ETG_DISABLEABLE
        if (!etgEffectsEnabled)
        {
            univWipeShipOutOfExistence(ship);
        }
#endif
        listDeleteNode(universe.DeleteShipList.head);
    }
}

/*-----------------------------------------------------------------------------
    Name        : univReallyDeleteThisShipRightNow
    Description : Like you need to ask?  It kills it.  No effects, no melodrama,
                    no waiting around to delete it later.  Just BOOM!
    Inputs      : ship - ship to delete
    Outputs     :
    Return      : void
    Note        : do not call this while you are walking any ship-related
                    linked lists.
----------------------------------------------------------------------------*/
void univReallyDeleteThisShipRightNow(Ship *ship)
{
    univDeleteDeadShip(ship, -1);
    if (ship->impactablelink.belongto != NULL)
    {
        listRemoveNode(&ship->impactablelink);
    }
    univWipeShipOutOfExistence(ship);
}

void univNotifyResourceDiedToHarvestShips(Resource *resource)
{
    Node *node;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = (Ship *)listGetStructOfNode(node);

        if ((ship->shiptype == sRammingFrigate) || (ship->shiptype == bRammingFrigate))
        {
            rammingFrigateClearTarget(ship, (Ship *)resource);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : DeleteDeadResource
    Description : Deletes an individual resource, and removes all references
                  to it.  Also frees its contents.
    Inputs      : resource
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeleteDeadResource(Resource *resource)
{
    Node *node;
    sdword resourceNumber;

    smFocusRemoveObj((SpaceObj *)resource);
    ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)resource);

    bobObjectDied((SpaceObj *)resource, &universe.collBlobList);
    pingObjectDied((SpaceObj *)resource);

    node = universe.BulletList.head;
    while (node != NULL)
    {
        Bullet *bullet = (Bullet *)listGetStructOfNode(node);

        if (bullet->owner == (Ship *)resource)
        {
            bool32 isBeam = (bullet->bulletType == BULLET_Beam);
            bullet->owner = NULL;
            bullet->gunowner = NULL;
            if (isBeam)
            {
                bullet->timelived += 10000.0f;
            }
        }

        if (bullet->target == (SpaceObjRotImpTarg *)resource)
        {
            bullet->target = NULL;
        }

        node = node->next;
    }

    node = universe.MissileList.head;
    while (node != NULL)
    {
        Missile *missile = (Missile *)listGetStructOfNode(node);

        if (missile->owner == (Ship *)resource)
        {
            missile->owner = NULL;
        }

        if (missile->target == (SpaceObjRotImpTarg *)resource)
        {
            missile->target = NULL;
            if (missile->totallifetime - missile->timelived > 5.0f)
            {
                missile->totallifetime = 5.0f + missile->timelived;
            }
        }

        node = node->next;
    }

    clResourceDied(&universe.mainCommandLayer, resource);
    aiplayerResourceDied(resource);

    resourceNumber = resource->resourceID.resourceNumber;
    if (resourceNumber < ResourceIDToPtr.numEntries)
    {
        ResourceIDToPtr.objptrs[resourceNumber] = NULL;
    }

    univNotifyResourceDiedToHarvestShips(resource);
    kasRemoveTargetFromSelections((SpaceObjRotImpTarg *)resource);
    univFreeSpaceObjContents((SpaceObj *)resource);

    if (resource->objtype != OBJ_AsteroidType || ((Asteroid *)resource)->asteroidtype != Asteroid0)
    {
        listRemoveNode(&resource->resourcelink);
        listRemoveNode(&resource->impactablelink);
        if (resource->renderlink.belongto != NULL)
        {
            listRemoveNode(&resource->renderlink);
        }
    }
    else if (resource->renderlink.belongto != NULL)
    {
        listRemoveNode(&resource->renderlink);
    }

    listDeleteNode(&resource->objlink);
}

/*-----------------------------------------------------------------------------
    Name        : univReallyDeleteThisResourceRightNow
    Description : Like you need to ask?  It kills it.  No effects, no melodrama,
                  no waiting around to delete it later.  Just BOOM!
    Inputs      : resource - resource to delete
    Outputs     :
    Return      : do not call this while you are walking any resource-related
                  linked lists.
----------------------------------------------------------------------------*/
void univReallyDeleteThisResourceRightNow(Resource *resource)
{
    DeleteDeadResource(resource);
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadResources
    Description : deletes all dead resources
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadResources()
{
    Node *objnode = universe.DeleteResourceList.head;
    DeleteResource *deleteResource;
    Resource *resource;

    if (objnode == NULL)
    {
        return;
    }

    do
    {
        deleteResource = (DeleteResource *)listGetStructOfNode(objnode);
        resource = deleteResource->resource;

        if (resource->resourcevalue == 0)
        {
            // resource was harvested away, so give opportunity for more to grow:
            if (resource->resourceVolume)
            {
                if (resource->resourceVolume->actualnumber > 0)
                {
                    resource->resourceVolume->actualnumber--;
                }
            }
        }

        if (resource->objtype == OBJ_AsteroidType)
        {
            if (((Asteroid *)resource)->resourcevalue > 0)
            {
                BreakAsteroidUp((Asteroid *)resource);
            }
        }

        univReallyDeleteThisResourceRightNow(resource);

        objnode = objnode->next;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteResourceList);
}

/*-----------------------------------------------------------------------------
    Name        : DeleteDeadDerelict
    Description : Deletes an individual derelict, and removes all references
                  to it.  Also frees its contents.
    Inputs      : derelict - derelict to delete
                  deathBy - how it died.  Used to create an effect.  -1 means no effect.
    Outputs     :
    Return      : TRUE if an effect for deleting the derelict was played
----------------------------------------------------------------------------*/
bool32 DeleteDeadDerelict(Derelict *derelict, sdword deathBy)
{
    Node *node;
    Effect *effect;
    etgeffectstatic *explosion = NULL;
    etglod *etgLOD;
    sdword LOD;

    if (derelict->derelicttype == HyperspaceGate)
    {
        hsDerelictDied(derelict);
    }

    bobObjectDied((SpaceObj *)derelict,&universe.collBlobList);
    smFocusRemoveObj((SpaceObj *)derelict);
    soundEventDerelictRemove(derelict);
    pingObjectDied((SpaceObj *)derelict);
    ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)derelict);

    node = universe.BulletList.head;
    while (node != NULL)
    {
        Bullet *bullet = (Bullet *)listGetStructOfNode(node);

        if (bullet->owner == (Ship *)derelict)
        {
            bool32 isBeam = (bullet->bulletType == BULLET_Beam);
            bullet->owner = NULL;
            bullet->gunowner = NULL;
            if (isBeam)
            {
                bullet->timelived += 10000.0f;
            }
        }

        if (bullet->target == (SpaceObjRotImpTarg *)derelict)
        {
            bullet->target = NULL;
        }

        node = node->next;
    }

    node = universe.MissileList.head;
    while (node != NULL)
    {
        Missile *missile = (Missile *)listGetStructOfNode(node);

        if (missile->owner == (Ship *)derelict)
        {
            missile->owner = NULL;
        }

        if (missile->target == (SpaceObjRotImpTarg *)derelict)
        {
            missile->target = NULL;
            if (missile->totallifetime - missile->timelived > 5.0f)
            {
                missile->totallifetime = 5.0f + missile->timelived;
            }
        }

        node = node->next;
    }

    clDerelictDied(&universe.mainCommandLayer,derelict);

    if (derelict->derelictID.derelictNumber < DerelictIDToPtr.numEntries)
    {
        DerelictIDToPtr.objptrs[derelict->derelictID.derelictNumber] = NULL;
    }

#if ETG_DISABLEABLE
    if (deathBy >= 0 && etgEffectsEnabled)
#else
    if (deathBy >= 0)
#endif
    {                                                       //if we should create an effect
        etgLOD = etgDeathEventTableDerelict[derelict->derelicttype][deathBy];
        if (etgLOD != NULL)
        {
            LOD = derelict->currentLOD;
            explosion = etgLOD->level[min(etgLOD->nLevels - 1, LOD)];
        }
        else
        {
            explosion = NULL;
        }
        if (explosion)
        {
            effect = etgEffectCreate(explosion, (Ship *)derelict, NULL, NULL, NULL, 1.0f, 0, 2);
            bitSet(effect->flags, SOF_ForceVisible);
        }
    }

    univFreeAttackTargets((Ship *)derelict);
    if (derelict->posinfo.unk_2C != NULL)
    {
        memFree(derelict->posinfo.unk_2C);
        derelict->posinfo.unk_2C = NULL;
    }
    if (derelict->navLightInfo != NULL)
    {
        memFree(derelict->navLightInfo);
        derelict->navLightInfo = NULL;
    }
    if (derelict->clampInfo != NULL)
    {
        memFree(derelict->clampInfo);
        derelict->clampInfo = NULL;
        bitClear(derelict->flags, SOF_Clamped);
    }

    listRemoveNode(&derelict->derelictlink);
    listRemoveNode(&derelict->impactablelink);
    derelict->derelictlink.belongto = NULL;
    derelict->impactablelink.belongto = NULL;
    univRemoveFromWorldList(derelict);                      //if it's a planet, remove it from the planet list
    return(explosion != NULL);
}

/*-----------------------------------------------------------------------------
    Name        : univReallyDeleteThisDerelictRightNow
    Description : Like you need to ask?  It kills it.  No effects, no melodrama,
                  no waiting around to delete it later.  Just BOOM!
    Inputs      : derelict - derelict to delete
    Outputs     :
    Return      : do not call this while you are walking any derelict-related
                  linked lists.
----------------------------------------------------------------------------*/
void univReallyDeleteThisDerelictRightNow(Derelict *derelict)
{
    bitSet(derelict->flags, SOF_Dead);
    DeleteDeadDerelict(derelict, -1);
//    listRemoveNode(&derelict->derelictlink);
//    listRemoveNode(&derelict->impactablelink);
    univRemoveObjFromRenderList((SpaceObj *)derelict);
//    univRemoveFromWorldList(derelict);                      //if it's a planet, remove it from the planet list
    listDeleteNode(&derelict->objlink);
}

/*-----------------------------------------------------------------------------
    Name        : univWipeDerelictOutOfExistance
    Description : This function deletes a derelict upon command of the death effect.
    Inputs      : derelict - derelict to go bye-bye
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void univWipeDerelictOutOfExistance(Derelict *derelict)
{
    bobObjectDied((SpaceObj *)derelict,&universe.collBlobList);
    nisObjectDied((SpaceObj *)derelict);
    if (derelict->derelictlink.belongto != NULL)
    {
        listRemoveNode(&derelict->derelictlink);
    }
    if (derelict->impactablelink.belongto != NULL)
    {
        listRemoveNode(&derelict->impactablelink);
    }
    univRemoveObjFromRenderList((SpaceObj *)derelict);
    etgShipDied((Ship *)derelict);
    univRemoveFromWorldList(derelict);                      //if it's a planet, remove it from the planet list
    listDeleteNode(&derelict->objlink);
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadDerelicts
    Description : deletes all dead derelicts
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadDerelicts()
{
    Node *objnode = universe.DeleteDerelictList.head;
    DeleteDerelict *deleteDerelict;
    Derelict *derelict;

    if (objnode == NULL)
    {
        return;
    }

    do
    {
        deleteDerelict = (DeleteDerelict *)listGetStructOfNode(objnode);
        derelict = deleteDerelict->derelict;
        dbgAssert(derelict->objtype == OBJ_DerelictType);

        // don't totally delete ship, until after explosion
#if ETG_DISABLEABLE
        if (!etgEffectsEnabled)
        {
            univReallyDeleteThisDerelictRightNow(derelict);
        }
        else
#endif
        {
            if (!DeleteDeadDerelict(derelict, deleteDerelict->deathBy))
            {                                               //if there was no effect
                univRemoveObjFromRenderList((SpaceObj *)derelict);
                listDeleteNode(&derelict->objlink);         //finish deleting it properly
            }
        }

        objnode = objnode->next;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteDerelictList);
}

/*-----------------------------------------------------------------------------
    Name        : DeleteDeadMissile
    Description : Deletes a dead missile, and removes all references to it.
                  Also frees it contents.
    Inputs      : deathBy - how the missile/mine died
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DeleteDeadMissile(Missile *missile, sdword deathBy)
{
    real32 damagetaken = missile->damage;
    MissileStaticInfo *missilestaticinfo = (MissileStaticInfo *)missile->staticinfo;
    etgeffectstatic *stat;
    sdword LOD;
    bool32 fatalHit;
    real32 floatDamage;
    udword intDamage;
    vector hitLocation;


    if (deathBy >= 0)
    {                                                       //if it was destroyed by forced attack
        if (missile->hitEffect != NULL)
        {                                                   //use the regular hit effect
            LOD = missile->currentLOD;
            if (LOD < missile->hitEffect->nLevels)
            {
                stat = missile->hitEffect->level[LOD];
            }
            else
            {
                stat = NULL;
            }
        }
        else
        {
            stat = NULL;
        }
        if (univSpaceObjInRenderList((SpaceObj *)missile))
        {
#if ETG_DISABLEABLE
            if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && (!etgFrequencyExceeded(stat)))
#else
            if (stat != NULL && etgHitEffectsEnabled && (!etgFrequencyExceeded(stat)))
#endif
            {
                hitLocation = missile->posinfo.position;

                floatDamage = missile->damage;
                if (RGLtype == SWtype)
                {                                               //smaller hit effects in software
                    floatDamage *= etgSoftwareScalarHit;
                }
                intDamage = TreatAsUdword(floatDamage);
                fatalHit = FALSE;
                etgEffectCreate(stat, NULL, &hitLocation, &missile->posinfo.velocity, &missile->rotinfo.coordsys, 1.0, 0, 2, intDamage, fatalHit);
            }
        }
    }
    smFocusRemoveObj((SpaceObj *)missile);
    ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)missile);
    bobObjectDied((SpaceObj *)missile,&universe.collBlobList);
    pingObjectDied((SpaceObj *)missile);

    univRemoveAllTargetReferencesFromBullets((SpaceObjRotImpTarg *)missile);
    univRemoveAllTargetReferencesFromMissiles((SpaceObjRotImpTarg *)missile);
    nisRemoveMissileReference(missile);
    clMissileDied(&universe.mainCommandLayer,missile);
    kasRemoveTargetFromSelections((SpaceObjRotImpTarg *)missile);

    IDToPtrTableObjDied(&MissileIDToPtr,missile->missileID.missileNumber);

    univFreeMissileContents(missile);
}

/*-----------------------------------------------------------------------------
    Name        : univDeleteDeadMissiles
    Description : deletes all dead missiles
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univDeleteDeadMissiles()
{
    Node *objnode = universe.DeleteMissileList.head;
    DeleteMissile *deleteMissile;
    Missile *missile;

    if (objnode == NULL)
    {
        return;
    }


    do
    {
        deleteMissile = (DeleteMissile *)listGetStructOfNode(objnode);
        missile = deleteMissile->missile;


        DeleteDeadMissile(missile, deleteMissile->deathBy);
        listRemoveNode(&missile->missilelink);
        listRemoveNode(&missile->impactablelink);
        if (missile->renderlink.belongto != NULL)
        {
            listRemoveNode(&missile->renderlink);
        }
        listDeleteNode(&missile->objlink);

        objnode = objnode->next;

    } while (objnode != NULL);

    listDeleteAll(&universe.DeleteMissileList);
}

/*-----------------------------------------------------------------------------
    Name        : univGetResourceStatistics
    Description : outputs the total resourceValue, numHarvestableResources, and numAsteroid0s in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univGetResourceStatistics(sdword *resourceValue,sdword *numHarvestableResources,sdword *numAsteroid0s)
{
    Node *objnode = universe.ResourceList.head;
    Resource *resource;
    sdword total = 0;
    sdword numharvestable = 0;
    sdword numasteroid0s = 0;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->objtype >= OBJ_AsteroidType && resource->objtype <= OBJ_DustType);
        if (nisIsRunning && (resource->health >= REALlyBig))
        {
            goto nextnode;       // don't pick NIS resources
        }
#if 0       // Asteroid0's not in Resource list anymore so this isn't needed
        if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
        {
            goto nextnode;      // don't harvest pebbles
        }
#endif
        total += resource->resourcevalue;
        numharvestable++;

nextnode:
        objnode = objnode->next;
    }

    for (objnode = universe.MinorSpaceObjList.head;objnode != NULL;objnode = objnode->next)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
        {
            numasteroid0s++;
        }
    }

    *resourceValue = total;
    *numHarvestableResources = numharvestable;
    *numAsteroid0s = numasteroid0s;
}

bool32 univObjectOutsideWorld(SpaceObj *obj)
{
    vector src = obj->posinfo.position;
    vector dst = obj->posinfo.position;

    univClipPointToWorldBox(&src, &dst);
    return (src.x != dst.x || src.y != dst.y || src.z != dst.z);
}

/*-----------------------------------------------------------------------------
    Name        : univFindNearestResource
    Description : finds the nearest resource to ship in optional volume specified by volumePosition and volumeRadius.
                  If VolumeRadius == 0, then volume is ignored
    Inputs      :
    Outputs     :
    Return      : returns the nearest resource (or NULL if none found)
----------------------------------------------------------------------------*/
Resource *univFindNearestResource(Ship *ship,real32 volumeRadius,vector *volumePosition)
{
    Node *objnode = universe.ResourceList.head;
    vector diff;
    real32 dist;
    real32 mindist = (real32)1e20;
    Resource *closestResource = NULL;
    Resource *resource;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->objtype >= OBJ_AsteroidType && resource->objtype <= OBJ_DustType);
        if (nisIsRunning && (resource->health >= REALlyBig))
        {
            goto nextnode;       // don't pick NIS resources
        }
#if 0       // Asteroid0's not in Resource list anymore so this isn't needed
        if ((resource->objtype == OBJ_AsteroidType) && (((Asteroid *)resource)->asteroidtype == Asteroid0))
        {
            goto nextnode;      // don't harvest pebbles
        }
#endif
        if (ResourceMovingTooFast(resource))
        {
            goto nextnode;
        }

        if (ResourceAlreadyBeingHarvested(&universe.mainCommandLayer,NULL,resource))
        {
            goto nextnode;
        }

        if (resource->resourceNotAccessible)
        {
            goto nextnode;
        }

        if (volumeRadius > 0.0f)
        {
            vecSub(diff,resource->posinfo.position,*volumePosition);

            if (!isBetweenInclusive(diff.x,-volumeRadius,volumeRadius))
            {
                goto nextnode;
            }

            if (!isBetweenInclusive(diff.y,-volumeRadius,volumeRadius))
            {
                goto nextnode;
            }

            if (!isBetweenInclusive(diff.z,-volumeRadius,volumeRadius))
            {
                goto nextnode;
            }
        }

        if (univObjectOutsideWorld((SpaceObj *)resource))
        {
            goto nextnode;
        }

        vecSub(diff,resource->posinfo.position,ship->posinfo.position);
        dist = vecMagnitudeSquared(diff);

        if (dist < mindist)
        {
            mindist = dist;
            closestResource = resource;
        }

nextnode:
        objnode = objnode->next;
    }

    return closestResource;
}

/*-----------------------------------------------------------------------------
    Name        : univGetChecksum
    Description : returns a checksum representing the status of the universe
    Inputs      :
    Outputs     : numShipsInChecksum
    Return      : checksum of mission sphere at this point in time
----------------------------------------------------------------------------*/
real32 univGetChecksum(sdword *numShipsInChecksum)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    Derelict *derelict;
    Resource *resource;
#if BINNETLOG
    Bullet   *bullet;
#endif
    sdword countShips = 0;

    real32 x=0.0f;
    real32 y=0.0f;
    real32 z=0.0f;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if ((ship->flags & SOF_Dead) == 0)
        {
            x += ship->posinfo.position.x;
            y += ship->posinfo.position.y;
            z += ship->posinfo.position.z;

            if ((netlogfile) && (logEnable == LOG_VERBOSE))
            {
#if BINNETLOG
                binnetlogShipInfo bns;

                bns.header = makenetcheckHeader('S','S','S','S');
                bns.shipID = ship->shipID.shipNumber;
                bns.playerIndex = (ubyte)ship->playerowner->playerIndex;
                bns.shiprace = (ubyte)ship->shiprace;
                bns.shiptype = (ubyte)ship->shiptype;

                if (ship->command)
                {
                    bns.shiporder = (sbyte)ship->command->ordertype.order;
                    bns.shipattributes = (sbyte)ship->command->ordertype.attributes;
                }
                else
                {
                    bns.shiporder = -1;
                    bns.shipattributes = -1;
                }

                bns.tacticstype = (ubyte)ship->tacticstype;
                bns.isDodging = (ubyte)ship->isDodging;
                bns.DodgeDir = (ubyte)ship->DodgeDir;

                bns.health = ship->health;
                bns.x = ship->posinfo.position.x;    bns.y = ship->posinfo.position.y;    bns.z = ship->posinfo.position.z;
                bns.vx = ship->posinfo.velocity.x;  bns.vy = ship->posinfo.velocity.y;    bns.vz = ship->posinfo.velocity.z;
                bns.fuel = ship->fuel;

                fwrite(&bns, sizeof(bns), 1 ,netlogfile);
#else
                fprintf(netlogfile,"S:%d %d %d %d (%f %f %f) %f %d %d %d %d (%d %d)\n",ship->playerowner->playerIndex,ship->shiprace,ship->shiptype,
                        ship->shipID.shipNumber,ship->posinfo.position.x,ship->posinfo.position.y,ship->posinfo.position.z,ship->fuel,ship->tacticstype,ship->isDodging,ship->DodgeDir,ship->tactics_ordertype,
                        (ship->command == NULL) ? -1 : ship->command->ordertype.order,
                        (ship->command == NULL) ? -1 : ship->command->ordertype.attributes);
#endif

                if ((ship->command) && (ship->command->ordertype.order == COMMAND_COLLECTRESOURCE))
                {
                    CollectResourceCommand *collect = &ship->command->collect;
#if BINNETLOG
                    binnetlogShipResourceInfo bnr;
                    bnr.header = makenetcheckHeader('S','R','S','R');
                    bnr.resourceID = (collect->resource != NULL) ? collect->resource->resourceID.resourceNumber : -1;
                    bnr.volume = collect->resourceVolumeSize;
                    bnr.x = collect->resourceVolumePosition.x;
                    bnr.y = collect->resourceVolumePosition.y;
                    bnr.z = collect->resourceVolumePosition.z;
                    fwrite(&bnr, sizeof(bnr), 1 ,netlogfile);
#else
                    fprintf(netlogfile,"R:%d ",
                            (collect->resource != NULL) ? collect->resource->resourceID.resourceNumber : -1);
#endif
                }

                if (ship->dockvars.dockPointActivity != NULL)
                {
                    DockPointActivity *dockPointActivity = ship->dockvars.dockPointActivity;
                    sdword numDockPoints = ship->staticinfo->dockStaticInfo->numDockPoints;
                    sdword i;
#if BINNETLOG
                    binnetlogShipDockInfo bnd;
                    udword orflag = 1;
                    bnd.header = makenetcheckHeader('S','D','S','D');
                    bnd.busyness = 0;
                    bnd.numDockPoints = (uword)numDockPoints;
                    bnd.thisDockBusy = 0;
                    for (i=0;i<numDockPoints;i++)
                    {
                        if (dockPointActivity->reserved[i] != 0)
                        {
                            bnd.thisDockBusy |= orflag;
                        }
                        orflag <<= 1;
                    }
                    fwrite(&bnd, sizeof(bnd), 1 ,netlogfile);
#else
                    fprintf(netlogfile,"B:");
                    for (i=0;i<NUM_DOCK_POINT_TYPES;i++)
                    {
                        fprintf(netlogfile," %d",dockPointActivity->numDockedOfType[i]);
                    }
                    fprintf(netlogfile,"\n");
                    fprintf(netlogfile,"P:");
                    for (i=0;i<numDockPoints;i++)
                    {
                        fprintf(netlogfile," %d",dockPointActivity->reserved[i] != 0);
                    }
                    fprintf(netlogfile,"\n");
#endif
                }
                if (ship->madBindings != NULL)
                {
#if BINNETLOG
                    binnetlogShipMadInfo bnm;
                    bnm.header = makenetcheckHeader('S','M','S','M');
                    bnm.info[0] = (ubyte)ship->madAnimationFlags;
                    bnm.info[1] = (ubyte)ship->madGunStatus;
                    bnm.info[2] = (ubyte)ship->madWingStatus;
                    bnm.info[3] = (ubyte)ship->madDoorStatus;
                    bnm.info[4] = (ubyte)ship->madSpecialStatus;
                    bnm.info[5] = (ubyte)ship->cuedAnimationIndex;
                    bnm.info[6] = (ubyte)ship->cuedAnimationType;
                    bnm.info[7] = 0;
                    bnm.info[8] = 0;
                    fwrite(&bnm, sizeof(bnm), 1 ,netlogfile);
#else
                    //                  flags,guns,wings,dock,special,index,type
                    fprintf(netlogfile,"M: %df %dg %dw %dd %ds %di %dt\n", ship->madAnimationFlags,ship->madGunStatus,ship->madWingStatus, ship->madDoorStatus, ship->madSpecialStatus,ship->cuedAnimationIndex,ship->cuedAnimationType);
#endif
                }
            }

            countShips++;
        }
        objnode = objnode->next;
    }

#ifndef HW_Release

#if BINNETLOG
    if ((netlogfile) && (logEnable == LOG_VERBOSE))
    {
        objnode = universe.BulletList.head;
        while (objnode != NULL)
        {
            bullet = (Bullet *)listGetStructOfNode(objnode);

            if ((!bitTest(bullet->flags, SOF_Dead)))
            {
#if BINNETLOG
                binnetlogBulletInfo bi;

                bi.header = makenetcheckHeader('B','B','B','B');
                bi.bullettype = bullet->bulletType;
                bi.bulletplayerowner = (bullet->playerowner!= NULL) ? bullet->playerowner->playerIndex : -1;
                bi.bulletowner = (bullet->owner != NULL) ? bullet->owner->shipID.shipNumber : -1;
                bi.x = bullet->posinfo.position.x; bi.y = bullet->posinfo.position.y; bi.z = bullet->posinfo.position.z;
                bi.vx = bullet->posinfo.velocity.x; bi.vy = bullet->posinfo.velocity.y; bi.vz = bullet->posinfo.velocity.z;
                bi.timelived = bullet->timelived;
                bi.totallifetime = bullet->totallifetime;
                bi.traveldist = bullet->traveldist;
                bi.damage = bullet->damage;
                bi.damageFull = bullet->damageFull;
                bi.DFGFieldEntryTime = bullet->DFGFieldEntryTime;
                bi.BulletSpeed = 0.0f;
                bi.collBlobSortDist = (bullet->collMyBlob) ? bullet->collMyBlob->sortDistance : 0.0f;
                fwrite(&bi,sizeof(bi),1,netlogfile);
#else
                if (bullet->playerowner!= NULL)
                    fprintf(netlogfile,"B: %d %d ", universe.univUpdateCounter, bullet->playerowner->playerIndex);
                else
                    fprintf(netlogfile,"B: %d N ", universe.univUpdateCounter);

                fprintf(netlogfile,"(%f %f %f) [%f %f] %f <%f %f> %f %f ",
                        bullet->posinfo.position.x,
                        bullet->posinfo.position.y,
                        bullet->posinfo.position.z,
                        bullet->timelived,
                        bullet->totallifetime,
                        bullet->traveldist,
                        bullet->damage,
                        bullet->damageFull,
                        bullet->DFGFieldEntryTime,
                        bullet->BulletSpeed);

                if (bullet->collMyBlob)
                    fprintf(netlogfile, "%f\n", bullet->collMyBlob->sortDistance);
                else
                    fprintf(netlogfile, "N\n");
#endif
            }

            objnode = objnode->next;
        }
    }
#endif

#if BINNETLOG
    if ((netlogfile) && (logEnable == LOG_VERBOSE))
    {
        clChecksum();
    }
#endif

    objnode = universe.DerelictList.head;
    while (objnode != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(objnode);
        if ((derelict->flags & SOF_Dead) == 0)
        {
            x += derelict->posinfo.position.x;
            y += derelict->posinfo.position.y;
            z += derelict->posinfo.position.z;

            if ((netlogfile) && (logEnable == LOG_VERBOSE))
            {
#if BINNETLOG
                binnetDerelictInfo di;
                di.header = makenetcheckHeader('D','D','D','D');
                di.derelictid = derelict->derelictID.derelictNumber;
                di.derelicttype = derelict->derelicttype;
                di.health = derelict->health;
                di.x = derelict->posinfo.position.x; di.y = derelict->posinfo.position.y; di.z = derelict->posinfo.position.z;
                di.vx = derelict->posinfo.velocity.x; di.vy = derelict->posinfo.velocity.y; di.vz = derelict->posinfo.velocity.z;
                fwrite(&di,sizeof(di),1,netlogfile);
#else
                fprintf(netlogfile,"  Derelict:%d %s (%f %f %f)\n",derelict->derelictID.derelictNumber,DerelictTypeToStr(derelict->derelicttype),
                        derelict->posinfo.position.x,derelict->posinfo.position.y,derelict->posinfo.position.z);
#endif
            }

            countShips++;
        }
        objnode = objnode->next;
    }

    objnode = universe.ResourceList.head;
    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);

        x += resource->posinfo.position.x;
        y += resource->posinfo.position.y;
        z += resource->posinfo.position.z;

#if BINNETLOG
        if ((netlogfile) && (logEnable == LOG_VERBOSE))
        {
        binnetResourceInfo ri;
        ri.header = makenetcheckHeader('R','R','R','R');
        ri.resourceid = resource->resourceID.resourceNumber;
        ri.resourcetype = resource->objtype;
        ri.resourceValue = resource->resourcevalue;
        ri.health = resource->health;
        ri.x = resource->posinfo.position.x; ri.y = resource->posinfo.position.y; ri.z = resource->posinfo.position.z;
        ri.vx = resource->posinfo.velocity.x; ri.vy = resource->posinfo.velocity.y; ri.vz = resource->posinfo.velocity.z;
        fwrite(&ri,sizeof(ri),1,netlogfile);
        }
#endif

        countShips++;

        objnode = objnode->next;
    }

#endif

    *numShipsInChecksum = countShips;

    return (x+y+z);
}

/*-----------------------------------------------------------------------------
    Name        : univCalcShipChecksum
    Description : returns a checksum representing the status of the ships in the universe
    Inputs      :
    Outputs     : numShipsInChecksum
    Return      : checksum of mission sphere at this point in time
----------------------------------------------------------------------------*/
udword univCalcShipChecksum(void)
{
    udword  shipcheck=0;
    Node   *search;

    Bullet *bullet;

    /*
    Ship   *ship;
    udword  i;

    search = universe.ShipList.head;

    while (search!=NULL)
    {
        ship = listGetStructOfNode(search);
        if(ship->flags & SOF_Dead)
        {
            //don't consider dead ships...they're bad news
            //they still get written to the syncdump.txt, but
            //they won't generate a sync error.
            search = search->next;
            continue;
        }
        shipcheck += TreatAsUdword(ship->collOptimizeDist);

        shipcheck += TreatAsUdword(ship->posinfo.position.x);
        shipcheck += TreatAsUdword(ship->posinfo.position.y);
        shipcheck += TreatAsUdword(ship->posinfo.position.z);
        shipcheck += TreatAsUdword(ship->posinfo.velocity.x);
        shipcheck += TreatAsUdword(ship->posinfo.velocity.y);
        shipcheck += TreatAsUdword(ship->posinfo.velocity.z);
        shipcheck += TreatAsUdword(ship->posinfo.force.x);
        shipcheck += TreatAsUdword(ship->posinfo.force.y);
        shipcheck += TreatAsUdword(ship->posinfo.force.z);
        shipcheck += (udword)ship->posinfo.isMoving;
        shipcheck += (udword)ship->posinfo.haventCalculatedDist;

        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m11);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m21);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m31);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m12);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m22);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m32);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m13);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m23);
        shipcheck += TreatAsUdword(ship->rotinfo.coordsys.m33);
        shipcheck += TreatAsUdword(ship->rotinfo.rotspeed.x);
        shipcheck += TreatAsUdword(ship->rotinfo.rotspeed.y);
        shipcheck += TreatAsUdword(ship->rotinfo.rotspeed.z);
        shipcheck += TreatAsUdword(ship->rotinfo.torque.x);
        shipcheck += TreatAsUdword(ship->rotinfo.torque.y);
        shipcheck += TreatAsUdword(ship->rotinfo.torque.z);

        shipcheck += TreatAsUdword(ship->collInfo.collOffset.x);
        shipcheck += TreatAsUdword(ship->collInfo.collOffset.y);
        shipcheck += TreatAsUdword(ship->collInfo.collOffset.z);
        shipcheck += TreatAsUdword(ship->collInfo.collPosition.x);
        shipcheck += TreatAsUdword(ship->collInfo.collPosition.y);
        shipcheck += TreatAsUdword(ship->collInfo.collPosition.z);

        for (i=0;i<8;i++)
        {
            shipcheck += TreatAsUdword(ship->collInfo.rectpos[i].x);
            shipcheck += TreatAsUdword(ship->collInfo.rectpos[i].y);
            shipcheck += TreatAsUdword(ship->collInfo.rectpos[i].z);
        }

        shipcheck += TreatAsUdword(ship->health);
        shipcheck += TreatAsUdword(ship->lasttimecollided);

        for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
            shipcheck += TreatAsUdword(ship->nonstatvars.thruststrength[i]);
        for (i=0;i<NUM_ROT_DEGOFFREEDOM;i++)
            shipcheck += TreatAsUdword(ship->nonstatvars.rotstrength[i]);
        for (i=0;i<NUM_TURN_TYPES;i++)
            shipcheck += TreatAsUdword(ship->nonstatvars.turnspeed[i]);

        shipcheck += (udword)ship->aistate;
        shipcheck += ship->specialFlags&(~(SPECIAL_Resourcing|SPECIAL_Finished_Resource));
        shipcheck += (udword)ship->dockingCollisionTarget;

        shipcheck += ship->specialFlags3;
        shipcheck += ship->specialFlags4;
        shipcheck += ship->specialFlagsUnknown2;
        shipcheck += ship->specialFlagsUnknown3;
        shipcheck += ship->specialFlagsUnknown4;
        shipcheck += ship->specialFlagsUnknown5;
        shipcheck += TreatAsUdword(ship->selection22C);
        shipcheck += ship->specialFlagsUnknown7;

        shipcheck += TreatAsUdword(ship->moveTo.x);
        shipcheck += TreatAsUdword(ship->moveTo.y);
        shipcheck += TreatAsUdword(ship->moveTo.z);
        shipcheck += TreatAsUdword(ship->moveFrom.x);
        shipcheck += TreatAsUdword(ship->moveFrom.y);
        shipcheck += TreatAsUdword(ship->moveFrom.z);
        shipcheck += TreatAsUdword(ship->formationOffset.x);
        shipcheck += TreatAsUdword(ship->formationOffset.y);
        shipcheck += TreatAsUdword(ship->formationOffset.z);
        shipcheck += TreatAsUdword(ship->formationHeading.x);
        shipcheck += TreatAsUdword(ship->formationHeading.y);
        shipcheck += TreatAsUdword(ship->formationHeading.z);
        shipcheck += TreatAsUdword(ship->engineOffset.x);
        shipcheck += TreatAsUdword(ship->engineOffset.y);
        shipcheck += TreatAsUdword(ship->engineOffset.z);
        shipcheck += TreatAsUdword(ship->enginePosition.x);
        shipcheck += TreatAsUdword(ship->enginePosition.y);
        shipcheck += TreatAsUdword(ship->enginePosition.z);

        shipcheck += TreatAsUdword(ship->rowOriginalPoint.x);
        shipcheck += TreatAsUdword(ship->rowOriginalPoint.y);
        shipcheck += TreatAsUdword(ship->rowOriginalPoint.z);
        shipcheck += TreatAsUdword(ship->rowOutOfWayPoint.x);
        shipcheck += TreatAsUdword(ship->rowOutOfWayPoint.y);
        shipcheck += TreatAsUdword(ship->rowOutOfWayPoint.z);

        shipcheck += (udword)ship->shiptype;
        shipcheck += (udword)ship->shiprace;
        shipcheck += (udword)ship->tacticstype;

        shipcheck += (udword)ship->isDodging;
        shipcheck += TreatAsUdword(ship->DodgeTime);
        shipcheck += (udword)ship->DodgeDir;
        shipcheck += (udword)ship->tacticsFormationVar1;
        shipcheck += ship->DodgeFlag;
        shipcheck += (udword)ship->tactics_ordertype;
        shipcheck += ship->tacticsTalk;
        shipcheck += (udword)ship->tacticsNeedToFlightMan;

        shipcheck += TreatAsUdword(ship->timeCreated);
        shipcheck += (udword)ship->kamikazeState;

        shipcheck += TreatAsUdword(ship->kamikazeVector.x);
        shipcheck += TreatAsUdword(ship->kamikazeVector.y);
        shipcheck += TreatAsUdword(ship->kamikazeVector.z);

        shipcheck += TreatAsUdword(ship->speedBurstTime);
        shipcheck += TreatAsUdword(ship->singlePlayerSpeedLimiter);
        shipcheck += TreatAsUdword(ship->deathtime);
        shipcheck += ship->madAnimationFlags;
        shipcheck += ship->madGunStatus;
        shipcheck += ship->madWingStatus;
        shipcheck += ship->madDoorStatus;
        shipcheck += ship->madSpecialStatus;
        shipcheck += ship->cuedAnimationIndex;
        shipcheck += ship->cuedAnimationType;
        shipcheck += ship->nextAnim;
        shipcheck += (udword)ship->forceCancelDock;
        shipcheck += (udword)ship->resources;
        shipcheck += TreatAsUdword(ship->fuel);
        shipcheck += TreatAsUdword(ship->lastTimeRepaired);

        shipcheck += (udword)ship->shipidle;
        shipcheck += (udword)ship->autostabilizeship;
        shipcheck += (udword)ship->autostabilizestate;
        shipcheck += (udword)ship->autostabilizewait;
        shipcheck += (udword)ship->aistateattack;
        shipcheck += (udword)ship->flightmanState1;
        shipcheck += (udword)ship->flightmanState2;
        shipcheck += (udword)ship->rcstate1;
        shipcheck += (udword)ship->rcstate2;
        shipcheck += (udword)ship->aistatecommand;
        shipcheck += (udword)ship->dontrotateever;
        shipcheck += (udword)ship->dontapplyforceever;
        shipcheck += (udword)ship->shipisattacking;
        shipcheck += (udword)ship->colorScheme;
        shipcheck += (udword)ship->recentlyAttacked;
        shipcheck += (udword)ship->recentlyFiredUpon;
        shipcheck += (udword)ship->rowState;
        shipcheck += TreatAsUdword(ship->aidescend);
        shipcheck += ship->flightman;

        shipcheck += ship->attackvars.flightmansLeft;
        shipcheck += ship->attackvars.attacksituation;
        shipcheck += ship->attackvars.attackevents;

        shipcheck += TreatAsUdword(ship->dockvars.dockLaunchState);
        shipcheck += TreatAsUdword(ship->dockvars.dockPointIndex);
        shipcheck += TreatAsUdword(ship->dockvars.dockStateDeadline);

        shipcheck += (udword)ship->dockvars.dockPointActivity;

        shipcheck += (udword)ship->visibleToWho;
        shipcheck += (udword)ship->visibleToWhoPreviousFrame;
        shipcheck += (udword)ship->howDidIDie;
        shipcheck += (udword)ship->whoKilledMe;
        shipcheck += (udword)ship->nDamageLights;

        shipcheck += TreatAsUdword(ship->holdingPatternPos.x);
        shipcheck += TreatAsUdword(ship->holdingPatternPos.y);
        shipcheck += TreatAsUdword(ship->holdingPatternPos.z);
        shipcheck += TreatAsUdword(ship->damageModifier);

        shipcheck += ship->tractorbeaminfo;
        shipcheck += ship->tractorvector;

        shipcheck += (udword)ship->tractorstate;

        for (i=0;i<4;i++)
            shipcheck += (udword)ship->tractorPosinfo[i];

        shipcheck += (udword)ship->needNewLeader;
        shipcheck += (udword)ship->newDockIndexTransfer;

        shipcheck += TreatAsUdword(ship->shipDeCloakTime);
        shipcheck += (udword)ship->shipID.shipNumber;

        search = search->next;
    }
    */

    search = universe.BulletList.head;

    while (search!=NULL)
    {
        bullet = (Bullet *)listGetStructOfNode(search);
        if(bullet->flags & SOF_Dead)
        {
            search = search->next;
            continue;
        }

        //shipcheck += TreatAsUdword(bullet->posinfo.position.x);
        //shipcheck += TreatAsUdword(bullet->posinfo.position.y);
        //shipcheck += TreatAsUdword(bullet->posinfo.position.z);

        //shipcheck += TreatAsUdword(bullet->posinfo.velocity.x);
        //shipcheck += TreatAsUdword(bullet->posinfo.velocity.y);
        //shipcheck += TreatAsUdword(bullet->posinfo.velocity.z);
        //shipcheck += (udword)bullet->posinfo.isMoving;
        //shipcheck += (udword)bullet->posinfo.haventCalculatedDist;

        //shipcheck += TreatAsUdword(bullet->lengthmag);
        //shipcheck += TreatAsUdword(bullet->traveldist);
        //shipcheck += TreatAsUdword(bullet->timelived);
        //shipcheck += TreatAsUdword(bullet->totallifetime);
        //shipcheck += TreatAsUdword(bullet->damage);
        //shipcheck += TreatAsUdword(bullet->damageFull);
        //shipcheck += (udword)bullet->SpecialEffectFlag;
        //shipcheck += TreatAsUdword(bullet->DFGFieldEntryTime);
        //shipcheck += TreatAsUdword(bullet->BulletSpeed);
        if(bullet->owner != NULL)
        {
            shipcheck += 128;
        }
        //if(bullet->gunowner != NULL)
        //{
        //    shipcheck += 32;
        //}
        search = search->next;
    }

    return (shipcheck);
}

void univSetupShipForControl(Ship *ship)
{
    univCheckShipState(ship);
}

void univResetMultiplayerAutosaveState(void)
{
    cataMultiplayerAutosaveTime = CATA_MULTIPLAYER_AUTOSAVE_INTERVAL_SECONDS;
    cataMultiplayerAutosaveSlot = 0;
}

/*-----------------------------------------------------------------------------
    Name        : univSetupShipsForControl
    Description : initializes the shipidle and gettingrocked flags for this pass
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univSetupShipsForControl()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    SelectCommand *housekeepSelection;
    sdword i;
    sdword numShips;

    growSelectReset(&universe.HousekeepShipList);

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        univSetupShipForControl(ship);

        objnode = objnode->next;
    }

    housekeepSelection = universe.HousekeepShipList.selection;
    numShips = housekeepSelection->numShips;

    for (i=0;i<numShips;i++)
    {
        ship = housekeepSelection->ShipPtr[i];
        if (ship)
        {
            ship->staticinfo->custshipheader.CustShipHousekeep(ship);
        }
    }

    growSelectReset(&universe.HousekeepShipList);       // don't need to keep track of them anymore
}

//same as above but for derelicts
void univSetupDerelictsForControl()
{

}
SelectCommand *getEnemiesWithinProximity(Ship *thisship,real32 retaliateZone)
{
    Player *playerowner = thisship->playerowner;
    blob *thisblob = thisship->collMyBlob;
    udword objindex = 0;
    SelectCommand *blobships = thisblob->blobShips;
    udword num = blobships->numShips;
    SelectCommand *select;
    udword shipIndex = 0;
    Ship *ship;
    vector diff;
    real32 timesincecloak;
    real32 negRetaliateZone = -retaliateZone;

    if ((thisship->shiptype == FloatingCity) || (thisship->shiptype == TargetDrone) ||
        (bitTest(thisship->specialFlags, SPECIAL_FriendlyStatus)))
    {
        return NULL;      // traders get special status, not considered enemy to anyone
    }

    select = memAlloc(sizeofSelectCommand(num),"se(selectenemies)",Pyrophoric);    // worst case scenario

    while (objindex < num)
    {
        ship = blobships->ShipPtr[objindex];

        if (allianceIsShipAlly(ship,playerowner))
        {
            goto nextnode;      // must be enemy ship
        }
        if ((ship->shiptype == FloatingCity) || (ship->shiptype == TargetDrone) ||
            (ship->shiptype == dCrystal0) || (ship->shiptype == dCrystal1) ||
            (bitTest(ship->specialFlags, SPECIAL_FriendlyStatus|SPECIAL_Hyperspacing)))
        {
            goto nextnode;      // passive retaliation should not target neutral/special-case objects
        }
        if (bitTest(ship->flags,SOF_Cloaked))
        {
            if(!proximityCanPlayerSeeShip(thisship->playerowner,ship))
            {
                goto nextnode;      // if ship is cloaked, and can't be seen by that ships player..
            }
        }
        timesincecloak = universe.totaltimeelapsed - ship->shipDeCloakTime;
        if(timesincecloak <= TW_SECOND_SEE_TIME)
        {
            if(timesincecloak < TW_NOONE_SEE_TIME)
            {
                //ship being considered has decloaked so recently that 'thisship' couldn't
                //possibly see it yet
                goto nextnode;
            }
            else if(timesincecloak < TW_FIRST_SEE_TIME)
            {
                //ship has decloaked so long ago that only a small percentage could see it now
                if(randombetween(0,100) > TW_FIRST_SEE_PERCENTAGE)
                {
                    goto nextnode;
                }
            }
            else if(timesincecloak < TW_SECOND_SEE_TIME)
            {
                //ship has decloaked a fair ammount of time ago, and some more ships are likely to see it now
                if(randombetween(0,100) > TW_SECOND_SEE_PERCENTAGE)
                {
                    goto nextnode;
                }
            }
        }
        if(bitTest(ship->flags,SOF_Disabled|SOF_Hide))
        {
            //don't attack disabled ships.
            goto nextnode;
        }
        vecSub(diff,ship->posinfo.position,thisship->posinfo.position);

        if (isBetweenExclusive(diff.x,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.y,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.z,negRetaliateZone,retaliateZone) )
        {
            select->ShipPtr[shipIndex++] = ship;
        }

nextnode:
        objindex++;
    }

    if (shipIndex == 0)
    {
        memFree(select);
        return NULL;
    }

    select->numShips = shipIndex;
    return select;
}

void univCheckShipState(Ship *ship)
{
    SelectCommand selectone;
    AttackCommand *attack;
    CommandToDo *command;

    if (ship->deathtime != 0.0f)
    {
        if ((ship->flags & SOF_Dead) == 0)
        {
            if (universe.totaltimeelapsed >= ship->deathtime)
            {
                // Self-destruct ship
                if(!(ship->flags & SOF_Disabled))
                {
                    //if ship is disabled..it didn't receive the
                    //self destruct order
                    ship->health = 0;
                    AddShipToDeleteShipList(ship,-1);
                }
                else if (ShipTypeIsGravWellGeneratorType(ship->shiptype))
                {
                    if( ((GravWellGeneratorSpec *)ship->ShipSpecifics)->GravDerelict)
                    {
                        //if ship is disabled..it didn't receive the
                        //self destruct order
                        ship->health = 0;
                        AddShipToDeleteShipList(ship,-1);
                    }
                }
            }
        }
    }

    if (ship->flags & SOF_Dead)
    {
        return;
    }

    if (ship->specialFlags1C8 & CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY)
    {
        rowFlyShipOutOfWay(ship);
        //ship->autostabilizeship = FALSE;
    }

    // IDA univSetupShipForControl (0x54a650, 0x54a709-0x54a731): the autostabilize gate tests
    // "(flags & 0x2A08000) || (flags & SOF_NISShip) || (attributes & ATTRIBUTES_DontApplyFriction)",
    // where 0x2A08000 = SOF_DontDrawTrails|SOF_Slaveable|SOF_Crazy|SOF_Hide. Retail does NOT test
    // SOF_Disabled/SOF_Hyperspace here (our prior mask), and DOES test SOF_Slaveable -- the bit our
    // SalCap tow logic sets on a captured target instead of SOF_Disabled. Without SOF_Slaveable in
    // this mask, a towed target still passes as a normal ship needing autostabilize, so
    // aitrackSteadyShip/aitrackForceSteadyShip zero its towed velocity a few ticks after the one-shot
    // SalCap fly-to-host call, freezing it in place. Retail also has no TargetDrone-type exclusion here.
    if (ship->autostabilizeship &&
        !bitTest(ship->flags, SOF_DontDrawTrails|SOF_Slaveable|SOF_Crazy|SOF_Hide|SOF_NISShip) &&
        !bitTest(ship->attributes,ATTRIBUTES_DontApplyFriction))
    {
        bool32 isShipSteady;

        if (ship->shiptype == MiningBase)
            isShipSteady = aitrackIsShipSteadyAnywhere(ship);       // don't care about orientation up
        else
            isShipSteady = aitrackIsShipSteady(ship);

        if (isShipSteady)
        {
            aitrackForceSteadyShip(ship);
            ship->autostabilizestate = 0;
        }
        else
        {
            switch (ship->autostabilizestate)
            {
                case 0:
                    // we just detected ship isn't stable
                    ship->autostabilizewait = AUTOSTABILIZE_FRAME_KICK_IN;
                    ship->autostabilizestate = 1;

                case 1:
                    if (ship->autostabilizewait > 0)
                    {
                        ship->autostabilizewait--;
                        break;
                    }
                    else
                    {
                        ship->autostabilizestate = 2;
                    }

                case 2:
                    if (ship->shiptype == MiningBase)
                        aitrackSteadyShipAnywhere(ship);
                    else
                        aitrackSteadyShip(ship);
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
    }

#ifndef HW_Release
    if (!shipsRetaliate)
    {
        return;
    }
#endif

    if (nisIsRunning)
    {
        return;      // don't autoretaliate if NIS running
    }

    if(!bitTest(ship->flags, SOF_Slaveable))
    {
        // Retail (univSetupShipForControl @ 0x54a650) does not call tacticsUpdate(ship) here --
        // the entire fresh decompile of this function has zero calls into it. Cataclysm's new
        // clAutoAttackResolvedShipTargets subsystem replaced the legacy HW1 retreat/tactics
        // machinery, so calling it here (as our HW1-derived code originally did) leaves
        // universe.RetreatList populated with RetreatAtoms that nothing ever drains (see
        // tacticsGlobalUpdate below, also confirmed absent from retail's univUpdate), producing
        // dangling CommandToDo references once the referenced attack command is freed.
        if ((universe.univUpdateCounter & CHECK_SHIP_OUTOFWORLD_RATE) == (ship->shipID.shipNumber & CHECK_SHIP_OUTOFWORLD_RATE))
        {
            if (!(singlePlayerGame && (ship->playerowner->playerIndex == 1)))        // don't apply to KAS ships in single player game
            {
                if (univObjectOutsideWorld((SpaceObj *)ship))
                {
                    vector newdest;
                    SelectCommand *selectmove = getShipAndItsFormation(&universe.mainCommandLayer,ship);
                    CommandToDo *commandofship = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                    dbgAssert(selectmove);

                    if(commandofship != NULL)
                    {
                        if(commandofship->ordertype.order == COMMAND_MP_HYPERSPACEING)
                        {
                            //if ship is hyperspaceing, don't check if its in the world...once it is done, then we can move it!
                            goto DONT_MOVE_ME_YET;
                        }

                        if(commandofship->ordertype.order == COMMAND_ATTACK)
                        {
                            // let's see if we can find a target that is inside the world and attack that:
                            SpaceObjRotImpTarg *currenttarget = ship->attackvars.attacktarget;
                            SpaceObjRotImpTarg *testtarget;
                            AttackCommand *attack;
                            sdword i;

                            if (currenttarget)
                            {
                                if (!univObjectOutsideWorld((SpaceObj *)currenttarget))
                                {
                                    // our target is still in world, so we don't have to move into world
                                    goto DONT_MOVE_ME_YET;
                                }
                            }

                            attack = commandofship->attack;
                            for (i=0;i<attack->numTargets;i++)
                            {
                                testtarget = attack->TargetPtr[i];
                                if (testtarget == currenttarget)
                                {
                                    continue;
                                }

                                if (!univObjectOutsideWorld((SpaceObj *)testtarget))
                                {
                                    ship->attackvars.attacktarget = testtarget;
                                    goto DONT_MOVE_ME_YET;
                                }
                            }
                        }
                    }
                    newdest = ship->posinfo.position;
                    vecMultiplyByScalar(newdest,FLY_INTO_WORLD_PERCENT_DIST);
                    clMove(&universe.mainCommandLayer,selectmove,clBuildMovePointCommand(ship->posinfo.position,&newdest));
                    memFree(selectmove);
                    return;
                }
            }
        }
    }
DONT_MOVE_ME_YET:
    if (ship->shipidle)
    {
        if(!bitTest(ship->flags, SOF_Slaveable))
        {
            if (!shipIsCollectingResource(ship))
            {
                // ship isn't actively harvesting resources -> full auto-attack path
                if ((ship->gettingrocked == NULL) || univIsObjFriendly((SpaceObj *)ship,ship->gettingrocked->playerowner))
                {
                    // no attacker, or the ship "getting rocked" turns out to be friendly
                    // (e.g. friendly-fire/formation splash) -> fall through to a periodic
                    // passive-retaliate scan for real nearby hostiles instead of attacking it
                    if (((universe.univUpdateCounter & CHECK_PASSIVE_ATTACK_RATE) == (ship->shipID.shipNumber & CHECK_PASSIVE_ATTACK_RATE))
                        && (ship->staticinfo->collSideModifiers[TRANS_FORWARD] != 0.0f)
                        && !univIsShipCloaked((SpaceObj *)ship)
                        && ((ship->flags2 & SOF2_Unknown8) == 0 || !shipHasModuleSlotState(ship))
                        && bitTest(ship->flags, SOF_Selectable))
                    {
                        command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                        attack = (AttackCommand *)univFindNearbyTargets(ship,ship->staticinfo->collSideModifiers[TRANS_FORWARD]);
                        clAutoAttackResolvedShipTargets(ship,command,attack);
                    }
                }
                else
                {
                    // real hostile attacker -> attack it and its formation
                    attack = (AttackCommand *)getShipAndItsFormation(&universe.mainCommandLayer,ship->gettingrocked);
                    command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                    clAutoAttackResolvedShipTargets(ship,command,attack);
                }
            }
            else if (ship->gettingrocked != NULL)
            {
                // ship IS actively harvesting resources and is under attack -> flee/dock
                // unless the ship is in Aggressive tactics mode
                if (ship->tacticstype != Aggressive)
                {
                    selectone.numShips = 1;
                    selectone.ShipPtr[0] = ship;

                    clDock(&universe.mainCommandLayer,&selectone,DOCK_FOR_BOTHREFUELANDREPAIR | DOCK_FOR_REFUEL,NULL);
                }
            }
        }
    }
    else
    {
        //ship isn't idle
        //so lets do a check every so often to see if
        //ship should attempt passive attacking
        if(!bitTest(ship->flags, SOF_Slaveable))
        {
            if (((universe.univUpdateCounter & CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE) == (ship->shipID.shipNumber & CHECK_PASSIVE_ATTACK_WHILEMOVING_RATE))
                && (ship->staticinfo->collSideModifiers[TRANS_FORWARD] != 0.0f)
                && (TreatAsUdword(ship->staticinfo->engineNozzleOffset[1].x) != 0))
            {
                command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                if (command != NULL)
                {
                    if(command->ordertype.order == COMMAND_MOVE
                    ||command->ordertype.order == COMMAND_DOCK)
                    {
                        if (!univIsShipCloaked((SpaceObj *)ship)
                            && ((ship->flags2 & SOF2_Unknown8) == 0 || !shipHasModuleSlotState(ship)))
                        {
                            attack = (AttackCommand *)univFindNearbyTargets(ship,ship->staticinfo->collSideModifiers[TRANS_FORWARD]);
                            clAutoAttackResolvedShipTargets(ship,command,attack);
                        }
                    }
                }
            }
        }
    }
}

#if 0           // no longer needed - univCheckShipState called in univUpdateAllObjPosVel
/*-----------------------------------------------------------------------------
    Name        : univCheckShipStates
    Description : checks if shipidle and gettingrocked, and if so, tells ship
                  to retaliate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univCheckShipStates()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
//    bool32 attackpassive;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        univCheckShipState(ship);

nextnode:
        objnode = objnode->next;
    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : univupdateReset
    Description : resets univupdate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateReset()
{
    universe.phystimeelapsed = 0.0f;
    universe.totaltimeelapsed = 0.0f;
    universe.realtimeelapsed = 0.0f;
    universeRealtimeElapsed = 0.0f;
    smEnemySpottedNotificationTime = 0.0f;
    universe.radius = 20000.0f;
    universe.univUpdateCounter = 0;
    universe.resourceNumber = 0;
    universe.derelictNumber = 0;
    universe.shipNumber = 0;
    universe.missileNumber = 0;
    univResetFastNetworkIDLookups();
    universe.quittime = 0.0f;
    universe.wintime = 0.0f;
    universe.aiplayerProcessing = FALSE;
    cataOverlayPauseCountdown = -1;
    univResetMultiplayerAutosaveState();

    universe.CapitalShipCaptured = 0;
    universe.PlayerWhoWon = 0;

    universe.lasttimeadded = 0;
    universe.DerelictTech=FALSE;

    universe.crateTimer = 0.0f;
    universe.numCratesInWorld = 0;
    universe.bounties = FALSE;

    collUpdateAllBlobs = FALSE;

    listInit(&universe.RenderList);
    listInit(&universe.MinorRenderList);
    universe.dontUpdateRenderList = FALSE;

    listInit(&universe.SpaceObjList);
    listInit(&universe.MinorSpaceObjList);
    listInit(&universe.effectList);
    listInit(&universe.ShipList);
    listInit(&universe.BulletList);
    listInit(&universe.ResourceList);
    listInit(&universe.DerelictList);
    listInit(&universe.ImpactableList);
    listInit(&universe.MissileList);
    listInit(&universe.MineFormationList);

    listInit(&universe.DeleteMissileList);
    listInit(&universe.DeleteResourceList);
    listInit(&universe.DeleteDerelictList);
    listInit(&universe.DeleteShipList);

    listInit(&universe.collBlobList);

    // tactics stuff
    listInit(&universe.RetreatList);
    listInit(&tacticsAttackMemory);

    // regrow resources stuff
    listInit(&universe.ResourceVolumeList);

    growSelectClose(&universe.HousekeepShipList);
    growSelectClose(&universe.unk1);
    growSelectInit(&universe.HousekeepShipList);
    growSelectInit(&universe.unk1);
    growSelectClose(&ClampedShipList);
    growSelectInit(&ClampedShipList);

    univAutosaveTimer = (opAutoSaveInterval > 0) ? (real32)opAutoSaveInterval * 60.0f : 0.0f;
}

/*-----------------------------------------------------------------------------
    Name        : univupdateInit
    Description : intializes univupdate
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateInit()
{
    univInitFastNetworkIDLookups();
    growSelectInit(&universe.HousekeepShipList);
    growSelectInit(&universe.unk1);
    growSelectInit(&ClampedShipList);

    universe.bountySize = 0;
    univupdateReset();
    smEnemySpottedNotificationTime = 0.0f;
    universe.star3dinfo = star3dInit(NUM_STARS, universe.radius - 1.0f, universe.radius);
    scriptSet(NULL,"Galaxy.script",GalaxyTweaks);
    universe.backgroundColor = backgroundColor;
}

/*-----------------------------------------------------------------------------
    Name        : univFreeEffectContents
    Description : frees the effect (for exiting the program)
    Inputs      : effect
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeEffectContents(Effect *effect)
{
    etgEffectDelete(effect);
}

/*-----------------------------------------------------------------------------
    Name        : univFreeDerelictContents
    Description : Frees the contents of a derelict without freeing the derelict itself.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeDerelictContents(Derelict *derelict)
{
    if (derelict->navLightInfo)
    {
        memFree(derelict->navLightInfo);
        derelict->navLightInfo = NULL;
    }
    if(derelict->salvageInfo != NULL)
    {
        memFree(derelict->salvageInfo);
        derelict->salvageInfo = NULL;
    }
    if(derelict->clampInfo != NULL)
    {
        memFree(derelict->clampInfo);
        derelict->clampInfo = NULL;
    }

}

/*-----------------------------------------------------------------------------
    Name        : univFreeAsteroidContents
    Description : frees the asteroid contents
    Inputs      : asteroid
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeAsteroidContents(Asteroid *asteroid)
{
    if (asteroid->flags & SOF_StaticInfoIsDynamicCata)
    {
        memFree(asteroid->staticinfo);
    }
}

void univFreeNebulaContents(Nebula *nebula)
{
    if (nebula->stub != NULL)
    {
        nebDeleteChunk((nebChunk*)nebula->stub);
        nebula->stub = NULL;
    }

    if (nebula->flags & SOF_StaticInfoIsDynamicCata)
    {
        memFree(nebula->staticinfo);
    }
}

void univFreeGasCloudContents(GasCloud *gascloud)
{
    if (gascloud->flags & SOF_StaticInfoIsDynamicCata)
    {
        memFree(gascloud->staticinfo);
    }
}

void univFreeDustCloudContents(DustCloud *dustcloud)
{
    if (dustcloud->stub != NULL)
    {
        cloudDeleteSystem((cloudSystem*)dustcloud->stub);
        dustcloud->stub = NULL;
    }

    if (dustcloud->flags & SOF_StaticInfoIsDynamicCata)
    {
        memFree(dustcloud->staticinfo);
    }
}

void univFreeResourceContents(Resource *resource)
{
    switch (resource->objtype)
    {
        case OBJ_AsteroidType:
            univFreeAsteroidContents((Asteroid *)resource);
            break;

        case OBJ_NebulaType:
            univFreeNebulaContents((Nebula *)resource);
            break;

        case OBJ_GasType:
            univFreeGasCloudContents((GasCloud *)resource);
            break;

        case OBJ_DustType:
            univFreeDustCloudContents((DustCloud *)resource);
            break;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univFreeBulletContents
    Description : frees the bullet (for exiting program)
    Inputs      : bullet
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeBulletContents(Bullet *bullet)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : univFreeMissileContents
    Description : frees the missile (for exiting program)
    Inputs      : missile
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeMissileContents(Missile *missile)
{
    if(missile->formationinfo != NULL)
    {
        listRemoveNode(&missile->formationLink);     //take out of formation info list
    }
    univFreeAttackTargets((Ship *)missile);
    if (missile->posinfo.unk_2C != NULL)
    {
        memFree(missile->posinfo.unk_2C);
        missile->posinfo.unk_2C = NULL;
    }
    if(missile->trail != NULL)
    {
        mistrailDelete(missile->trail);
        missile->trail = NULL;
    }
    if(missile->clampInfo != NULL)
    {
        memFree(missile->clampInfo);
        missile->clampInfo = NULL;
        bitClear(missile->flags, SOF_Clamped);
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveShipReferences
    Description : Calls functions to remove references to ships.  This will
                    always be called no matter how a ship dies.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveShipReferences(Ship *ship)
{
    Node *node;

    ship->flags = (ship->flags & ~(SOF_Selectable | SOF_Dead)) | SOF_Dead;

    gameStatsShipDied(ship);

    if (ship->staticinfo->canBuildShips &&
        (!singlePlayerGame || !ship->playerowner->playerIndex))
    {
        cmRemoveFactory(ship);
    }

    if (ship->rceffect != NULL)
    {
        clDeleteCommandEffect((CommandToDo *)ship);
    }

    tacticsRemoveShipFromRetreatList(ship);

    if (ship->shipSinglePlayerGameInfo != NULL)
    {
        LinkedList *midlevelHyperspaceIn = (LinkedList *)ship->shipSinglePlayerGameInfo;
        node = midlevelHyperspaceIn->head;

        while (node != NULL)
        {
            Node *next = node->next;
            InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
            Ship *childShip = insideShip->ship;

            childShip->howDidIDie = ship->howDidIDie;
            childShip->whoKilledMe = ship->whoKilledMe;

            if (childShip->shiplink.belongto != NULL)
            {
                listRemoveNode(&childShip->shiplink);
            }
            if (childShip->impactablelink.belongto != NULL)
            {
                listRemoveNode(&childShip->impactablelink);
            }
            if (childShip->objlink.belongto != NULL)
            {
                listRemoveNode(&childShip->objlink);
            }

            etgShipDied(childShip);
            soundEventShipRemoveHandles(childShip);
            pieShipDied(childShip);
            mouseClickShipDied(childShip);
            clShipDied(&universe.mainCommandLayer, childShip);
            univRemoveAllShipReferencesFromSpecialShips(childShip);
            univFreeShipContents(childShip);
            memFree(childShip);

            node = next;
        }

        listDeleteAll(midlevelHyperspaceIn);
        memFree(ship->shipSinglePlayerGameInfo);
        ship->shipSinglePlayerGameInfo = NULL;
    }

    node = universe.BulletList.head;
    while (node != NULL)
    {
        Bullet *bullet = (Bullet *)listGetStructOfNode(node);

        if (bullet->owner == ship)
        {
            bool32 isBeam = (bullet->bulletType == BULLET_Beam);
            bullet->owner = NULL;
            bullet->gunowner = NULL;
            if (isBeam)
            {
                bullet->timelived += 10000.0f;
            }
        }

        if (bullet->target == (SpaceObjRotImpTarg *)ship)
        {
            bullet->target = NULL;
        }

        node = node->next;
    }

    node = universe.MissileList.head;
    while (node != NULL)
    {
        Missile *missile = (Missile *)listGetStructOfNode(node);

        if (missile->owner == ship)
        {
            missile->owner = NULL;
        }

        if (missile->target == (SpaceObjRotImpTarg *)ship)
        {
            missile->target = NULL;
            if (missile->totallifetime - missile->timelived > 5.0f)
            {
                missile->totallifetime = 5.0f + missile->timelived;
            }
        }

        node = node->next;
    }

    clShipDied(&universe.mainCommandLayer,ship);
    univRemoveAllShipReferencesFromSpecialShips(ship);
    univDetachShipsInsideFromCommands(ship);
    clDetachShipFromCommand(ship);
    smFocusRemoveObj((SpaceObj *)ship);
    ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)ship);
    bobObjectDied((SpaceObj *)ship,&universe.collBlobList);
    pingObjectDied((SpaceObj *)ship);
    aiplayerShipDied(ship);
    singlePlayerShipDied(ship);
    if (tutorial)
    {
        tutPlayerShipDied(ship);
    }
    soundEventShipRemoveHandles(ship);

    if (ship->shipID.shipNumber < ShipIDToPtr.numEntries)
    {
        ShipIDToPtr.objptrs[ship->shipID.shipNumber] = NULL;
    }

    growSelectRemoveShipBySettingNULL(&universe.unk1,ship);
    growSelectRemoveShipBySettingNULL(&universe.HousekeepShipList,ship);

    if (ship->hotKeyGroup & 0x7FE0)
    {
        sdword i;

        for (i = 0; i < SEL_NumberHotKeyGroups; i++)
        {
            SelectAnyCommand *hotkeyGroup = (SelectAnyCommand *)&selHotKeyGroup[i];

            if (((udword)1 << (i + 5)) & ship->hotKeyGroup)
            {
                if (clRemoveTargetFromSelection(hotkeyGroup, (TargetPtr)ship) &&
                    hotkeyGroup->numTargets == 0)
                {
                    hotkeyGroup->timeLastStatus = universe.phystimeelapsed;
                }
            }
        }
        ship->hotKeyGroup = 0;
    }

    shipNameRelease(ship);
    mrClearDeadShipReference(ship);
    smOverlayRemoveTargetFromSelections((TargetPtr)ship);
    battleChatClearShipMessages(ship);
}

/*-----------------------------------------------------------------------------
    Name        : univFreeShipContents
    Description : frees the ship (for exiting program)
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univFreeShipContents(Ship *ship)
{
    lightning **p_lightning;
    sdword count;
    shiptrail **trail;
    ShipsInsideMe *shipsInsideMe;
    Node *node;
    InsideShip *insideShip;

    shipRemoveSupportUsage(ship);
    univFreeAttackTargets(ship);

    if (ship->posinfo.unk_2C != NULL)
    {
        memFree(ship->posinfo.unk_2C);
        ship->posinfo.unk_2C = NULL;
    }

    univRemoveShipReferences(ship);

    if (ship->attackvars.multipleAttackTargets)
    {
        memFree(ship->attackvars.multipleAttackTargets);
        ship->attackvars.multipleAttackTargets = NULL;
    }

    if (ship->shipSinglePlayerGameInfo)
    {
        memFree(ship->shipSinglePlayerGameInfo);
        ship->shipSinglePlayerGameInfo = NULL;
    }

    deleteRepairDroidData((RepairDroidController *)ship->runtimeAnimationOverlay.runtimeController);
    ship->runtimeAnimationOverlay.runtimeController = 0;

    if (ship->navLightInfo)
    {
        memFree(ship->navLightInfo);
        ship->navLightInfo = NULL;
    }

    if (ship->clampInfo)
    {
        memFree(ship->clampInfo);
        ship->clampInfo = NULL;
        ship->flags &= ~SOF_Crazy;
    }

    // Free lightning effect handles
    p_lightning = ship->lightning;
    count = 2;
    do
    {
        if (*p_lightning)
        {
            cloudKillLightning(*p_lightning);
            *p_lightning = NULL;
        }
        ++p_lightning;
        --count;
    } while (count);

    dmgStopEffect(ship, DMG_All);
    dmgClearLights(ship);

    // Free engine trails using star3dClose (Cataclysm replaces trailDelete)
    trail = ship->trail;
    count = MAX_NUM_TRAILS;
    do
    {
        if (*trail)
        {
            star3dClose((Star3dInfo*)*trail);
            *trail = NULL;
        }
        ++trail;
        --count;
    } while (count);

    if (ship->cataclysmGunInfo != NULL)
    {
        memFree(ship->cataclysmGunInfo);
        ship->cataclysmGunInfo = NULL;
    }

    if (ship->dockvars.dockPointActivity)
    {
        memFree(ship->dockvars.dockPointActivity);
        ship->dockvars.dockPointActivity = NULL;
    }

    shipsInsideMe = ship->shipsInsideMe;
    if (shipsInsideMe)
    {
        for (node = shipsInsideMe->insideList.head; node; node = node->next)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            insideShip->ship->howDidIDie = ship->howDidIDie;
            insideShip->ship->whoKilledMe = ship->whoKilledMe;
            /* NON-PARITY: retail's univFreeShipContents (IDA 0x54b6a7-0x54b70e)
               recurses over this same list (walks [ship+4B4h], our confirmed
               dockvars.launchpoints/shipsInsideMe offset) and frees each
               contained ship via a direct univFreeShipContents+memFree call,
               with no equivalent of etgShipDied() for the contained ship. Any
               live Effect still attached to a contained ship (owner == this
               ship) is left with a dangling owner pointer once it's freed here
               -- confirmed to match retail exactly, so this is not a
               reconstruction bug. Under ASan this trips a heap-use-after-free
               in etgEffectUpdate (etg.c) the next time such an effect updates;
               retail simply reads stale-but-still-mapped heap memory instead
               of crashing. We proactively call etgShipDied() here (as the
               sibling cleanup path in this file already does for the "full"
               ship-death sequence) purely to keep ASan/debug runs stable --
               this deliberately diverges from retail's raw disassembly. */
            etgShipDied(insideShip->ship);
            univFreeShipContents(insideShip->ship);
            memFree(insideShip->ship);
        }
        listDeleteAll(&shipsInsideMe->insideList);
        memFree(shipsInsideMe);
        ship->shipsInsideMe = NULL;
    }

    if (((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipFreeShipContents != NULL)
    {
        ((ShipStaticInfo *)ship->staticinfo)->custshipheader.CustShipFreeShipContents(ship);
    }

    // Release MAD owned bindings
    if (ship->runtimeAnimationOverlay.madAnimationData &&
        *(udword*)(ship->runtimeAnimationOverlay.madAnimationData + 12) != (udword)-1)
    {
        ship->runtimeAnimationOverlay.madOwnedBindings =
            *(udword*)(ship->runtimeAnimationOverlay.madAnimationData + 44);
    }
    if (ship->runtimeAnimationOverlay.madOwnedBindings)
    {
        memFree((void*)ship->runtimeAnimationOverlay.madOwnedBindings);
        ship->runtimeAnimationOverlay.madOwnedBindings = 0;
    }

    if (ship->flags & SOF_StaticInfoIsDynamicCata)
    {
        memFree(ship->staticinfo);
        ship->staticinfo = NULL;
    }
}


/*-----------------------------------------------------------------------------
    Name        : univupdateCloseAllObjectsAndMissionSpheres
    Description : closes all objects in the universe and all mission spheres in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateCloseAllObjectsAndMissionSpheres()
{
    Node *objnode = universe.SpaceObjList.head;
    SpaceObj *obj;
    sdword i;

    univCloseFastNetworkIDLookups();
    growSelectClose(&universe.HousekeepShipList);
    growSelectClose(&universe.unk1);
    growSelectClose(&ClampedShipList);

    gameIsEnding = TRUE;    //set global flag to indicate to other functions that the game is ending

    singlePlayerGameCleanup();

    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

        switch (obj->objtype)
        {
            case OBJ_ShipType:
                ((Ship *)obj)->howDidIDie = DEATH_BY_GAME_END;
                univFreeShipContents((Ship *)obj);
                break;

            case OBJ_BulletType:
                univFreeBulletContents((Bullet *)obj);
                break;

            case OBJ_AsteroidType:
                univFreeAsteroidContents((Asteroid *)obj);
                break;

            case OBJ_NebulaType:
                univFreeNebulaContents((Nebula *)obj);
                break;

            case OBJ_GasType:
                univFreeGasCloudContents((GasCloud *)obj);
                break;

            case OBJ_DustType:
                univFreeDustCloudContents((DustCloud *)obj);
                break;

            case OBJ_DerelictType:
                univFreeDerelictContents((Derelict *)obj);
                break;

            case OBJ_EffectType:
                univFreeEffectContents((Effect *)obj);
                break;

            case OBJ_MissileType:
                univFreeMissileContents((Missile *)obj);
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
        univRemoveObjFromRenderList(obj);

        objnode = objnode->next;
    }

    objnode = universe.MinorSpaceObjList.head;
    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

        switch (obj->objtype)
        {
            case OBJ_AsteroidType:
                univFreeAsteroidContents((Asteroid *)obj);
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
        univRemoveObjFromMinorRenderList(obj);

        objnode = objnode->next;
    }

    listDeleteAll(&universe.SpaceObjList);
    listDeleteAll(&universe.MinorSpaceObjList);

    listInit(&universe.effectList);
    listInit(&universe.ShipList);
    listInit(&universe.BulletList);
    listInit(&universe.ResourceList);
    listInit(&universe.DerelictList);
    listInit(&universe.ImpactableList);
    listInit(&universe.MissileList);

    bobListDelete(&universe.collBlobList);

    listDeleteAll(&universe.MineFormationList);
    listDeleteAll(&universe.ResourceVolumeList);

    // deselect everything as well
    selSelected.numShips = 0;
    selSelecting.numTargets = 0;

    if (!singlePlayerGameLoadNewLevelFlag)
    {
        for (i=0;i<SEL_NumberHotKeyGroups;i++)      // clear hotkey groups if not loading a new level
        {
            selHotKeyGroup[i].numShips = 0;
        }

        growSelectClose(&singlePlayerGameInfo.ShipsToHyperspace);
    }
    gameIsEnding=FALSE;
}

void univResetSpaceObjs(void)
{
    univupdateCloseAllObjectsAndMissionSpheres();
}

/*-----------------------------------------------------------------------------
    Name        : univupdateClose
    Description : closes a mission
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univupdateClose()
{
    univupdateCloseAllObjectsAndMissionSpheres();

    star3dClose(universe.star3dinfo);
    universe.star3dinfo = NULL;
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void setSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *object)
{
    StaticInfoHealthGuidanceShipDerelict *statinfo = (StaticInfoHealthGuidanceShipDerelict *)object->staticinfo;
    sdword sizeofSalvageInfo;

    switch(object->objtype)
    {
    case OBJ_ShipType:
        // Ships do not have salvageStaticInfo in Cataclysm
        ((Ship *)object)->cataBusyStateWindow.busyStatePtr1EC = NULL;
        return;
    case OBJ_DerelictType:
        if(((Derelict *)object)->staticinfo->salvageStaticInfo == NULL)
        {
            ((Derelict *)object)->salvageInfo = NULL;
            return;
        }
        break;
    default:
        return;
    }
    {
        Derelict *derelict = (Derelict *)object;
        SalvagePoint *cursalvagepoint;
        SalvageStaticPoint *cursalvagestaticpoint;
        sdword i;
        sizeofSalvageInfo = sizeofSalvageInfo(statinfo->salvageStaticInfo->numSalvagePoints-1);
        derelict->salvageInfo = memAlloc(sizeofSalvageInfo,"SalvageInfo",NonVolatile);
        derelict->salvageInfo->numSalvagePoints = statinfo->salvageStaticInfo->numSalvagePoints;
        //list free ones...account for heading and up ones!
        derelict->salvageInfo->numSalvagePointsFree = derelict->salvageInfo->numSalvagePoints;

        for(i=0,cursalvagepoint=&derelict->salvageInfo->salvagePoints[0],cursalvagestaticpoint=&statinfo->salvageStaticInfo->salvageStaticPoints[0];i<statinfo->salvageStaticInfo->numSalvagePoints;i++,cursalvagepoint++,cursalvagestaticpoint++)
        {
            cursalvagepoint->salvageStaticPoint = cursalvagestaticpoint;
            cursalvagepoint->busy = 0;
        }
        derelict->salvageInfo->numNeededForSalvage = statinfo->salvageStaticInfo->numNeededForSalvage;

    }
}


/*-----------------------------------------------------------------------------
    Name        : univRegrowAsteroids
    Description : regrows an asteroid
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRegrowAsteroids()
{
    ;       // complete later, regrowing asteroids properly
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : ObjDistCompareCB
    Description : callback function to compare camera distances of obj1,obj2
    Inputs      : obj1,obj2
    Outputs     :
    Return      : returns TRUE if obj1 camera distance > obj2 camera distance
----------------------------------------------------------------------------*/
bool32 ObjDistCompareCB(SpaceObj *obj1,SpaceObj *obj2)
{
    return (obj1->collOptimizeDist > obj2->collOptimizeDist);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : univSpaceObjInRenderList
    Description : returns TRUE if the SpaceObj is in the render list
    Inputs      :
    Outputs     :
    Return      : returns TRUE if the SpaceObj is in the render list
----------------------------------------------------------------------------*/
bool32 univSpaceObjInRenderList(SpaceObj *spaceobj)
{
    if (spaceobj->renderlink.belongto != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : univRemoveObjFromRenderList
    Description : removes the spaceobj from the render list (if it is there)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univRemoveObjFromRenderList(SpaceObj *spaceobj)
{
    listVerify(&universe.RenderList);
    if (univSpaceObjInRenderList(spaceobj))
    {
        listRemoveNode(&spaceobj->renderlink);
    }
    listVerify(&universe.RenderList);
}

/*-----------------------------------------------------------------------------
    Name        : univAddObjToRenderListIf
    Description : adds newobj to renderlist if ifobj is in render list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univAddObjToRenderListIf(SpaceObj *newobj,SpaceObj *ifobj)
{
    listVerify(&universe.RenderList);
    if (univSpaceObjInRenderList(ifobj))
    {
        if ((newobj->flags & SOF_DockMarker) == 0 &&
            (newobj->flags & SOF_Hide) == 0)
        {
            if (bitTest(newobj->flags, SOF_AlwaysSortFront))
            {
                univAddObjToRenderList(newobj);
                return;
            }
            newobj->cameraDistanceVector = ifobj->cameraDistanceVector;
            newobj->cameraDistanceSquared = ifobj->cameraDistanceSquared;

            listAddNodeAfter(&ifobj->renderlink,&newobj->renderlink,newobj);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : univAddObjToRenderList
    Description : adds newobj to renderlist
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univAddObjToRenderList(SpaceObj *newobj)
{
    listVerify(&universe.RenderList);
    if ((newobj->flags & SOF_DockMarker) == 0 &&
        (newobj->flags & SOF_Hide) == 0)
    {
        vector eyeposition = universe.mainCameraCommand.actualcamera.eyeposition;
        Node *objnode;
        SpaceObj *obj;

        vecSub(newobj->cameraDistanceVector,eyeposition,newobj->posinfo.position);
        newobj->cameraDistanceSquared = vecMagnitudeSquared(newobj->cameraDistanceVector);

        // now find where to put object in list to maintain sort order

        // render list is sorted from back to front, so find first node which has cameraDistanceSquared less than ours

        if (bitTest(newobj->flags, SOF_AlwaysSortFront))
        {                                                   //if this object has to go in front
            objnode = universe.RenderList.tail;
            while (objnode != NULL)
            {
                obj = (SpaceObj *)listGetStructOfNode(objnode);

                if (obj->cameraDistanceSquared > newobj->cameraDistanceSquared)
                {
                    listAddNodeAfter(objnode, &newobj->renderlink,newobj);
                    listVerify(&universe.RenderList);
                    goto doneForward;
                }
                objnode = objnode->prev;
            }
            listAddNode(&universe.RenderList,&newobj->renderlink,newobj);
            listVerify(&universe.RenderList);
doneForward:;
        }
        else
        {
            objnode = universe.RenderList.head;
            while (objnode != NULL)
            {
                obj = (SpaceObj *)listGetStructOfNode(objnode);

                if (obj->cameraDistanceSquared < newobj->cameraDistanceSquared)
                {
                    listAddNodeBefore(objnode,&newobj->renderlink,newobj);
                    goto doneRegular;
                }

                objnode = objnode->next;
            }

            listAddNode(&universe.RenderList,&newobj->renderlink,newobj);
doneRegular:
            ;
        }
    }
}

void univUpdateMinorRenderList()
{
    Node *objnode = universe.MinorSpaceObjList.head;
    SpaceObj *obj;
    vector distvec;
    vector lookatposition;
    vector eyeposition;
    real32 distsqr;

    if (mrCamera == NULL)
    {
        return;     // don't update render list if no mrCamera
    }

    lookatposition = mrCamera->lookatpoint;
    eyeposition = mrCamera->eyeposition;

    listVerify(&universe.RenderList);
    listRemoveAll(&universe.MinorRenderList);

    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

#ifdef gshaw
        dbgAssert((obj->objtype == OBJ_AsteroidType) && (((Asteroid *)obj)->asteroidtype == Asteroid0));
#endif

        vecSub(distvec,lookatposition,obj->posinfo.position);
        distsqr = vecMagnitudeSquared(distvec);

        if (distsqr < RENDER_VIEWABLE_DISTANCE_SQR)
        {
            if ((obj->flags & SOF_Hide) == 0)
            {
                vecSub(obj->cameraDistanceVector,eyeposition,obj->posinfo.position);
                obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                listAddNode(&universe.MinorRenderList,&obj->renderlink,obj);
            }
        }

        objnode = objnode->next;
    }
    listVerify(&universe.RenderList);
}

/*-----------------------------------------------------------------------------
    Name        : univUpdateRenderList
    Description : updates the render list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univUpdateRenderList()
{
    Node *objnode = universe.SpaceObjList.head;
    Node *nextnode;
    SpaceObj *obj;
    vector distvec;
    vector lookatposition;
    vector eyeposition;
    real32 distsqr;
    static bool32 limited = FALSE;
    static real32 limits[NUM_CLASSES];
    LinkedList sortFrontList;

    listVerify(&universe.RenderList);
    listInit(&sortFrontList);

    if (mrCamera == NULL)
    {
        return;     // don't update render list if no mrCamera
    }

    if (!limited)
    {
        limits[CLASS_Mothership] = RENDER_LIMIT_MOTHERSHIP;
        limits[CLASS_HeavyCruiser] = RENDER_LIMIT_HEAVYCRUISER;
        limits[CLASS_Carrier] = RENDER_LIMIT_CARRIER;
        limits[CLASS_Destroyer] = RENDER_LIMIT_DESTROYER;
        limits[CLASS_Frigate] = RENDER_LIMIT_FRIGATE;
        limits[CLASS_Corvette] = RENDER_LIMIT_CORVETTE;
        limits[CLASS_Fighter] = RENDER_LIMIT_FIGHTER;
        limits[CLASS_Resource] = RENDER_LIMIT_RESOURCE;
        limits[CLASS_NonCombat] = RENDER_LIMIT_NONCOMBAT;
        limits[CLASS_MonsterShip] = RENDER_LIMIT_MOTHERSHIP;
        limits[CLASS_Component] = RENDER_LIMIT_MOTHERSHIP;
        limits[CLASS_Micro] = RENDER_LIMIT_FIGHTER;
        limited = TRUE;
    }

    lookatposition = mrCamera->lookatpoint;
    eyeposition = mrCamera->eyeposition;

    listRemoveAll(&universe.RenderList);

    while (objnode != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(objnode);

#ifdef gshaw
#ifndef HW_Release
        if ((obj->objtype == OBJ_AsteroidType) && (((Asteroid *)obj)->asteroidtype == Asteroid0))
        {
            dbgAssert(FALSE);
        }
#endif
#endif

        vecSub(distvec,lookatposition,obj->posinfo.position);
        distsqr = vecMagnitudeSquared(distvec);

        if (obj->flags & SOF_ForceVisible)
        {                                                   //if object is to always be in render list
            vecSub(obj->cameraDistanceVector, eyeposition, obj->posinfo.position);
            obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

            listAddNode(&universe.RenderList, &obj->renderlink, obj);
        }
        else
        {                                                   //else check if object should be in render list
            if (obj->objtype == OBJ_ShipType)
            {
                ShipClass shipclass;
                real32 limit;

                shipclass = ((ShipStaticInfo*)obj->staticinfo)->shipclass;
                dbgAssert((udword)shipclass < NUM_CLASSES);

                if (((ShipStaticInfo*)obj->staticinfo)->renderlistLimitSqr != 0.0f)
                {
                    limit = ((ShipStaticInfo*)obj->staticinfo)->renderlistLimitSqr;
                }
                else
                {
                    limit = limits[(sdword)shipclass];
                }
                if ((distsqr < limit) &&
                    (obj->flags & (SOF_DockMarker | SOF_Hide)) == 0)
                {
                    vecSub(obj->cameraDistanceVector, eyeposition, obj->posinfo.position);
                    obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                    listAddNode(&universe.RenderList, &obj->renderlink, obj);
                }
            }
            else if (obj->objtype == OBJ_DerelictType)
            {
                real32 limit;

                if (((DerelictStaticInfo*)obj->staticinfo)->renderlistLimitSqr != 0.0f)
                {
                    limit = ((DerelictStaticInfo*)obj->staticinfo)->renderlistLimitSqr;
                }
                else
                {
                    limit = (obj->flags & SOF_BigObject) ? RENDER_MAXVIEWABLE_DISTANCE_SQR : RENDER_VIEWABLE_DISTANCE_SQR;
                }

                if (distsqr < limit)
                {
                    if ((obj->flags & (SOF_DockMarker | SOF_Hide)) == 0)
                    {
                        vecSub(obj->cameraDistanceVector, eyeposition, obj->posinfo.position);
                        obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                        if (bitTest(obj->flags, SOF_AlwaysSortFront))
                        {
                            listAddNode(&sortFrontList, &obj->renderlink, obj);
                        }
                        else
                        {
                            listAddNode(&universe.RenderList, &obj->renderlink, obj);
                        }
                    }
                }
            }
            else
            {
                if (distsqr < RENDER_MAXVIEWABLE_DISTANCE_SQR)
                {
                    if ((distsqr < RENDER_VIEWABLE_DISTANCE_SQR) || (obj->flags & SOF_BigObject))
                    {
                        if ((obj->flags & (SOF_DockMarker | SOF_Hide)) == 0)
                        {
                            vecSub(obj->cameraDistanceVector,eyeposition,obj->posinfo.position);
                            obj->cameraDistanceSquared = vecMagnitudeSquared(obj->cameraDistanceVector);

                            if (bitTest(obj->flags, SOF_AlwaysSortFront))
                            {
                                listAddNode(&sortFrontList, &obj->renderlink, obj);
                            }
                            else
                            {
                                listAddNode(&universe.RenderList,&obj->renderlink,obj);
                            }
                        }
                    }
                }
            }
        }

        objnode = objnode->next;
    }

    listVerify(&universe.RenderList);
    listMergeSort2(&universe.RenderList);

    listVerify(&universe.RenderList);
    if (sortFrontList.num > 0)
    {                                                       //if there are force-front-sort objects
        listMergeSort2(&sortFrontList);                     //sort them like regular rederlist objects
        objnode = sortFrontList.head;
        while (objnode != NULL)
        {
            obj = (SpaceObj *)listGetStructOfNode(objnode);
            nextnode = objnode->next;
            dbgAssert(&obj->renderlink == objnode);
            listAddNode(&universe.RenderList, &obj->renderlink, obj);
            objnode = nextnode;
        }
    }
    listVerify(&universe.RenderList);
}

/*-----------------------------------------------------------------------------
    Name        : univCheckRegrowResources
    Description : checks if any resources should be regrown
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univCheckRegrowResources(void)
{
    Node *node = universe.ResourceVolumeList.head;
    ResourceVolume *resvol;

    while (node != NULL)
    {
        resvol = (ResourceVolume *)listGetStructOfNode(node);

        if (resvol->actualnumber < resvol->number)
        {
            // we are short some resources, so there is a chance we might regrow them

            real32 shortperc = 1.0f - ( (real32) resvol->actualnumber / (real32) resvol->number );
            sdword regrowrate = resvol->attributesParam;

            if ((regrowrate < 0) || (regrowrate > REGROWRATE_Max))
            {
                regrowrate = 0;
            }

            shortperc *= REGROW_RATE_MODIFIER[regrowrate];

            if (frandom(1) < shortperc)
            {
                sdword tr;

                // let's regrow the resource ... or at least try 3 times (make sure it doesn't overlap ship)
                switch (resvol->resourceVolumeType)
                {
                    case ResourceSphereType:
                        for (tr=0;tr<3;tr++)
                        {
                            if (AddResourceToSphere(resvol,TRUE))
                            {
                                resvol->actualnumber++;
                                break;
                            }
                        }
                        break;

                    case ResourceCylinderType:
                        for (tr=0;tr<3;tr++)
                        {
                            if (AddResourceToCylinder(resvol,TRUE))
                            {
                                resvol->actualnumber++;
                                break;
                            }
                        }
                        break;

                    case ResourceRectangleType:
                        for (tr=0;tr<3;tr++)
                        {
                            if (AddResourceToRectangle(resvol,TRUE))
                            {
                                resvol->actualnumber++;
                                break;
                            }
                        }
                        break;

                    default:
                        break;
                }
            }
        }

        node = node->next;
    }
}




void wkSmoothShips(void);

/*-----------------------------------------------------------------------------
    Name        : univUpdate
    Description : Updates the mission sphere ships and objects
    Inputs      : phystimeelapsed, time since last time it was updated
    Outputs     :
    Return      : TRUE if done (game over)
----------------------------------------------------------------------------*/
bool32 univUpdate(real32 phystimeelapsed)
{
    if (overlayPauseActive)
    {
        if (cataOverlayPauseCountdown <= 0)
        {
            return FALSE;
        }
        cataOverlayPauseCountdown--;
    }

    if (!singlePlayerGame)
    {
        cataMultiplayerAutosaveTime -= phystimeelapsed;
        if (cataMultiplayerAutosaveTime <= 0.0f)
        {
            char savegamename[PATH_Max + 1];

            strcpy(savegamename, MultiPlayerSavedGamesPath);
            strcat(savegamename, "Autosave ");
            sprintf(savegamename + strlen(savegamename), "%d", cataMultiplayerAutosaveSlot);
            SaveGame(savegamename);
            clCommandMessage(strGetString(strSavedGame), -1);
            cataMultiplayerAutosaveTime = CATA_MULTIPLAYER_AUTOSAVE_INTERVAL_SECONDS;
            cataMultiplayerAutosaveSlot++;
            if (cataMultiplayerAutosaveSlot > 5)
            {
                cataMultiplayerAutosaveSlot = 0;
            }
        }
    }

    if ((autoSaveDebug) && ((universe.univUpdateCounter & 31) == 0))    // every 2s
    {
        char savegamename[PATH_Max + 1];
        static sdword savenumber = 0;

        if (multiPlayerGame)
        {
            strcpy(savegamename, utyGetSavedGamesPath());
            sprintf(savegamename + strlen(savegamename), "AutoSave%f", universe.totaltimeelapsed);
            SaveGame(savegamename);
            clCommandMessage(strGetString(strSavedGame), -1);
        }
        else
        {
            if ((universe.univUpdateCounter & 255) == 0)        // only save every 16s
            {
                if (((singlePlayerGame) && (singlePlayerGameInfo.hyperspaceState)) ||
                    (nisIsRunning))
                {
                    ; // don't save game
                }
                else
                {
                    strcpy(savegamename, utyGetSavedGamesPath());
                    sprintf(savegamename + strlen(savegamename), "AutoSaveDebug%d", savenumber);
                    savenumber = (savenumber+1) & 7;
                    SaveGame(savegamename);
                    clCommandMessage(strGetString(strSavedGame), -1);
                }
            }
        }
    }

    // Non-parity auto-save feature, do not revert:
    if (opAutoSaveInterval > 0 && (singlePlayerGame || multiPlayerGame))
    {
        univAutosaveTimer -= phystimeelapsed;
        if (univAutosaveTimer <= 0.0f)
        {
            char autosavePath[PATH_Max + 1];
            bool32 saved = FALSE;

            if (singlePlayerGame && !singlePlayerGameInfo.hyperspaceState && !nisIsRunning)
            {
                strcpy(autosavePath, SinglePlayerSavedGamesPath);
                strcat(autosavePath, "Autosave");
                saved = SaveGame(autosavePath);
            }
            else if (multiPlayerGame)
            {
                strcpy(autosavePath, MultiPlayerSavedGamesPath);
                strcat(autosavePath, "Autosave");
                saved = SaveGame(autosavePath);
            }

            if (saved)
                clCommandMessage(strGetString(strSavedGame), -1);

            univAutosaveTimer = (real32)opAutoSaveInterval * 60.0f;
        }
    }

#ifdef GOD_LIKE_SYNC_CHECKING
    if(multiPlayerGame)
    {
        if(syncDumpOn)
        {
            industrialStrengthSyncDebugging(universe.univUpdateCounter);
            netcheckIndustrialChecksum();
        }
    }
#endif

    universe.phystimeelapsed = phystimeelapsed;
    universe.totaltimeelapsed += phystimeelapsed;
    universe.realtimeelapsed += (real32)cat_TimerFunc2();
    universeRealtimeElapsed = universe.realtimeelapsed;
    universe.univUpdateCounter++;

#define firstTime (universe.univUpdateCounter == 1)

    if (universe.quittime != 0.0f)
    {
        real32 usequittime = universe.quittime;

        if (universe.wintime != 0.0f)
        {
            if (universe.totaltimeelapsed >= universe.wintime)
            {
                DeclarePlayerWin();
                universe.wintime = 0.0f;
            }
        }

        if ((multiPlayerGame) && (IAmCaptain))
        {
            usequittime += 5.0f;    // give a extra 5s so we transmit some more sync pkts other people so they quit too...
        }

        if (universe.totaltimeelapsed >= usequittime)
        {
            // quit the game here
            spMainScreen();

#if (defined(Downloadable) || defined(OEM))
            if ((singlePlayerGame) && (!(utyCreditsSequence|utyPlugScreens)))
#else
            if ((singlePlayerGame) && (!(utyCreditsSequence)))
#endif
            {
                gameIsRunning = FALSE;
                mrReset();
                feScreenStart(ghMainRegion, "SP_Game_Over");
                return TRUE;
            }
            else
            {
                utyGameQuitToMain(NULL,NULL);
                return TRUE;
            }
        }
    }
    else if (multiPlayerGame)
    {
        if (queueNumberOverruns(ProcessSyncPktQ) > 0)
        {
            bigmessageDisplay(strGetString(strSyncPktQOverrun),1);
            universe.quittime = universe.totaltimeelapsed + WAIT_TIME_TO_QUIT;
        }
    }

    PTSLAB(0,"univupdate");
#ifdef OPTIMIZE_VERBOSE
    vecNormalizeCounter=0;
#endif
    if ((!singlePlayerGame) && ((universe.univUpdateCounter & regenerateRUrate) == 0))
    {
        sdword i;
        Player *player;

        for (i=0;i<universe.numPlayers;i++)
        {
            player = &universe.players[i];

            if ((player->playerState != PLAYER_DEAD) && (player->resourceUnits < regenerateRUsIfUnder))
            {
                sdword before = player->resourceUnits;
                player->resourceUnits += regenerateRUs;
                if (player->resourceUnits > regenerateRUsIfUnder)
                {
                    player->resourceUnits = regenerateRUsIfUnder;

                }
                universe.gameStats.playerStats[player->playerIndex].totalRegeneratedResourceUnits += (player->resourceUnits - before);
            }
        }
    }

    if(tutorialdone)
    {
        // quit the game here
        spMainScreen();
        utyGameQuitToMain(NULL,NULL);
        return TRUE;
    }

    PTSLAB(1,"collblobs");

    if (firstTime || ((universe.univUpdateCounter & REFRESH_COLLBLOB_RATE) == REFRESH_COLLBLOB_FRAME))
    {
        collUpdateCollBlobs();
    }
    else
    {
        if ((universe.univUpdateCounter & REFRESH_COLLBLOB_RATE) == REFRESH_COLLBLOB_BATTLEPING_FRAME)
        {
            pingBattlePingsCreate(&universe.collBlobList);
        }
        collUpdateObjsInCollBlobs();
    }
    PTEND(1);

    cloudSetFog();      //dust/gas cloud think
    nebSetFog();        //nebula think

    //construction manager deterministic think
    cmDeterministicBuildProcess();
    // crappy fucking construction manager, no longer a task
    cmBuildTaskFunction();

    if ((universe.univUpdateCounter & REFRESH_RESEARCH_RATE) == REFRESH_RESEARCH_FRAME)
    {
        rmUpdateResearch();
    }

    if (!singlePlayerGame)
    {
        if ( (bitTest(tpGameCreated.flag,MG_ResourceInjections)) &&
             ((universe.univUpdateCounter-universe.lasttimeadded) == tpGameCreated.resourceInjectionInterval) )
        {
            universe.lasttimeadded = universe.univUpdateCounter;
            resourceInjectionTask();
            if (universe.players[universe.curPlayerIndex].playerState != PLAYER_DEAD)
                speechEventQueue(0, STAT_F_ResourceInjectionRecieved, -1, -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
        }

        if ( (bitTest(tpGameCreated.flag,MG_ResourceLumpSum)) &&
             (universe.univUpdateCounter == tpGameCreated.resourceLumpSumTime))
        {
            resourceLumpSum();
            if (universe.players[universe.curPlayerIndex].playerState != PLAYER_DEAD)
                speechEventQueue(0, STAT_F_ResourceInjectionRecieved, -1, -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
        }

        if (universe.univUpdateCounter == universe.players[sigsPlayerIndex].AllianceProposalsTimeout)
        {
            universe.players[sigsPlayerIndex].AllianceProposals = 0;
        }
    }

#ifdef COLLISION_CHECK_STATS
    shipsavoidingstuff = 0;
    shipsavoidedwalks = 0;
    shipsavoidedchecks = 0;
#endif

    //univZeroForcesOnResources();      // no longer needed - force, torque cleared after physUpdateObjPosVel acts on it
    //univZeroForcesOnDerelicts();
    //univZeroForcesOnMissiles();
    if (firstTime)
    {
        univSetupShipsForControl();     // only do it first time, since its done at end of univUpdateAllPosVel
        univActivateAllShips();
        gameStatsCalcShipCostTotals();      // calculate the total ship cost at game start for the game stats
    }
    //univSetupDerelictsForControl();


    if (wkTradeStuffActive)
    {
        wkTradeUpdate();
    }

    if (singlePlayerGame)
    {
        singlePlayerGameUpdate();
    }
    else if (bitTest(tpGameCreated.flag,MG_Hyperspace))
    {
        UpdateMidLevelHyperspacingShips();
    }

    PTSLAB(3,"collbulmis");
    collCheckAllBulletMissileCollisions();
    PTEND(3);

    PTSLAB(4,"compplayer");

    cratesUpdate();

    aiplayerUpdateAll();
    PTEND(4);

    PTSLAB(5,"cmdlayer");

    PTSLABLITTLE(2,"aiship");


    if (!nisUniversePause)
    {
        clProcess(&universe.mainCommandLayer);
        clPostProcess(&universe.mainCommandLayer);
    }


    PTENDLITTLE(2);

    // Retail (univUpdate @ 0x54c3f0) does not call tacticsGlobalUpdate() here: the fully-named
    // decompile goes directly from clPostProcess to univUpdateMineFormations with no call in
    // between. Cataclysm dropped the legacy HW1 retreat/tactics-global-update subsystem in favor
    // of the new clAutoAttackResolvedShipTargets auto-attack path (see univCheckShipState above,
    // which also no longer calls tacticsUpdate(ship) to populate universe.RetreatList).
#if 0                           // no longer needed - univCheckShipState called in univUpdateAllPosVel now
    if (!nisUniversePause)
    {
        univCheckShipStates();
    }
#endif

    univUpdateMineWallFormations();
    PTEND(5);

    PTSLAB(7,"collbump");
    collCheckAllBumpCollisions();
    PTEND(7);

    PTSLAB(6,"updateobjpos");

    //old school function replaced with new functions below
    //univUpdateAllPosVel();       // also does a univCheckShipStates
    univUpdateAllPosVelMinorObjs();
    univUpdateAllPosVelBullets();
    univUpdateAllPosVelMissiles();
    if (!nisUniversePause)
    {

        univUpdateAllPosVelDerelicts();
        univUpdateAllPosVelResources();
        univUpdateAllPosVelShips();     // also does a univCheckShipStates

    }
    else
    {
        univPausedUpdateAllPosVelShips();   //certain operations only for when universe is paused
    }
    shockwaveUpdateAll();
    univUpdateAllPosVelEffects();   // MUST do effects sometime after ships+bullets and possibly other things that they may be attached to

    PTEND(6);

    univDeleteDeadShips();
    univDeleteDeadResources();
    univDeleteDeadDerelicts();
    univDeleteDeadMissiles();


    if ((universe.univUpdateCounter & REGROW_RESOURCES_CHECK_RATE) == REGROW_RESOURCES_CHECK_FRAME)
    {
        univCheckRegrowResources();
    }

    PTEND(0);

    if (singlePlayerGameLoadNewLevelFlag)
    {
        singlePlayerLoadNewLevel(FALSE, FALSE);
        if (hrAbortLoadingGame)
        {
            utyGameQuitToMain(NULL,NULL);
            hrAbortLoadingGame = FALSE;
            return TRUE;
        }
    }

    if(!universe.bounties || (universe.univUpdateCounter & CHECK_BOUNTIES_RATE) == CHECK_BOUNTIES_FRAME)
    {
        universe.bounties = TRUE;
        calculatePlayerBounties();
    }

    if (spAutoSavePending)
    {
        spAutoSaveGame();
    }

    return FALSE;
}

void univKillPlayer(sdword i,sdword playerdeathtype)
{
    if (gatherStats|showStatsFight|showStatsFancyFight)
        return;     // just gathering stats, don't want to do the player win thing

    if (universe.players[i].playerState == PLAYER_DEAD)
    {
        return;     // player already dead
    }

    if (i == MAX_MULTIPLAYER_PLAYERS)
    {
        return;     // special player for autoguns, just exit
    }

    universe.players[i].PlayerMothership = NULL;
    universe.players[i].playerState = PLAYER_DEAD;
    universe.players[i].sensorLevel = 2;
    universe.gameStats.playerStats[i].timeOfDeath = universe.totaltimeelapsed;
    universe.gameStats.playerStats[i].typeOfPlayerDeath = playerdeathtype;
    aiplayerPlayerDied(&universe.players[i]);
    if (i == universe.curPlayerIndex)
    {
        smGhostMode = TRUE;
    }

    /* This will only be played if its a Relic player that died */
    speechEventQueue(0, 33558573, i, -1, 0, i, -1, 5.0f, -1);

    // player i has lost the game
    if (!singlePlayerGame) DisplayPlayerHasBeenDefeatedMessage(i);
    DisableAllPlayersShips(i);
    if (universe.curPlayerIndex == i)
    {
        if (singlePlayerGame)
        {
            bigmessageDisplay(strGetString(strQuestIsLost),0);
        }
        else
        {
            if (playerdeathtype == PLAYERKILLED_DROPPEDOUT)
            {
                bigmessageDisplay(strGetString(strHaveBeenDroppedOut),0);
            }
            else
            {
                bigmessageDisplay(strGetString(strHaveBeenDefeated),0);
            }
        }
    }
}



#if 0  // Cat2: Binary univApplyShipDamageEffects; live implementation at univupdate.c L8850 (0x545B80)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x545B80
void univApplyShipDamageEffects(Ship *ship, int damageLevel)
{ /* decompiled */ }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x541AE0
int * __cdecl meshBindingsFixup(int **meshData, int bindings)
{
    int *result;
    int nObjects;
    int *objPtr;
    int bindingBase;
    _DWORD *destBinding;
    _BYTE *srcIndex;
    int count;

    result = *a1;
    nObjects = **a1;
    if (nObjects > 0)
    {
        objPtr = result + 10;
        int *bindingList = (int *)(a2 + 8);
        int remaining = nObjects;
        do
        {
            bindingBase = *bindingList;
            if ((*(_BYTE *)(objPtr - 7) & 7) == 1 && *(int *)(*(objPtr - 2) + 16) > 0)
            {
                destBinding = (_DWORD *)(bindingBase + 84);
                srcIndex = (_BYTE *)(*objPtr + 20);
                count = *(_DWORD *)(*(objPtr - 2) + 16);
                do
                {
                    if (*srcIndex == 0xFF)
                        *destBinding = 0;
                    else
                        *destBinding = bindingBase + 88 * (unsigned __int8)*srcIndex;
                    destBinding += 22;
                    srcIndex += 24;
                    --count;
                } while (count);
            }
            objPtr += 14;
            ++bindingList;
            --remaining;
        } while (remaining);
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5147D0
void univUpdateShipDamageLevel(Ship *ship)
{
    float *weights;
    int *healthComponents;
    float totalDamage;
    int i;
    char statusMsg[256];

    totalDamage = 0.0f;
    if (ship->damageLevel == 4)
        return;

    weights = damageLevelWeights;
    healthComponents = (int *)&ship->healthComponents;
    do
    {
        totalDamage = (float)(*healthComponents) * (*weights) + totalDamage;
        ++weights;
        ++healthComponents;
    }
    while (weights < damageLevelWeightsEnd);

    i = 0;
    ship->damageLevel = 0;
    do
    {
        if (totalDamage > (double)damageLevelThresholds[4 * ship->staticinfo->shipclass + i])
        {
            ship->damageLevel = i + 1;
            if (enableShipDamageMessages)
            {
                if (tutorial != 1
                    && ship->playerowner == universe.curPlayerPtr
                    && !ship->flags_528)
                {
                    sprintf(statusMsg, "%s %s %s",
                        ShipTypeToNiceStr(ship->shiptype),
                        damageLevelVerb,
                        damageLevelStrings[i]);
                    clCommandMessage(statusMsg, -16711936, 1, (int)ship);
                    ship->damageLevelTimer = universeRealtimeElapsed + damageLevelDisplayTime;
                }
            }
        }
        ++i;
    }
    while (i < NUM_DAMAGE_LEVELS);
}

#endif

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5475E0
void univNotifyEffectDeletion(sdword deadObject)
{
    Node *node;
    Ship *ship;
    void (__cdecl *callback)(Ship *, sdword);
    InsideShip *insideShip;
    void (__cdecl *callback2)(Ship *, sdword);

    for (node = universe.ShipList.head; node; node = node->next)
    {
        ship = (Ship *)listGetStructOfNode(node);
        callback = ship->staticinfo->custshipheader.CustShipNotifyObjectDeleted;
        if (callback)
            callback(ship, deadObject);
    }

    for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        callback2 = ship->staticinfo->custshipheader.CustShipNotifyObjectDeleted;
        if (callback2)
            callback2(ship, deadObject);
    }
}

#if 0  // Cat2: Binary univApplyShipDamageEffects; live implementation at univupdate.c L8850 (0x545B80)
void univUpdateFormationSpeeds(void)
{
    Node *node;

    for (node = universe.mainCommandLayer.todolist.head; node != NULL; node = node->next)
    {
        CommandToDo *todo = (CommandToDo *)listGetStructOfNode(node);

        if (bitTest(todo->ordertype.attributes, COMMAND_IS_PROTECTING))
        {
            real32 minSpeed = GetShipsTravelVel(todo->selection, 1.0f);
            SelectCommand *selection = todo->selection;
            sdword i;

            for (i = 0; i < selection->numShips; i++)
            {
                Ship *ship = selection->ShipPtr[i];

                if (ship->dockvars.dockship != NULL)
                {
                    continue;
                }
                if (singlePlayerGame)
                {
                    if (singlePlayerGameInfo.currentMission == MISSION_8_THE_CATHEDRAL_OF_KADESH &&
                        ship->shiptype == 0x52)
                    {
                        continue;
                    }
                    if (singlePlayerGameInfo.currentMission == MISSION_9_SEA_OF_LOST_SOULS &&
                        ship->shiptype == 0x58)
                    {
                        continue;
                    }
                }

                if (salCapCantDoFormation(ship))
                {
                    Ship *salvTarget = *(Ship **)(SalvagerFunc(ship) + 4);

                    if (salvTarget != NULL &&
                        GetDistance(&ship->posinfo.position, &salvTarget->posinfo.position) < 750.0f)
                    {
                        continue;
                    }
                }

                if (ship->speedThrottle < 0.0f)
                {
                    ship->speedThrottle = REALlyBig;
                }
                if (minSpeed < ship->speedThrottle)
                {
                    ship->speedThrottle = minSpeed;
                }
            }
        }
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x54D110
unsigned int __cdecl univBulletDie(int *bullet)
{
    int parentShip;

    // Free bullet trail/effect data
    if (bullet[33])
    {
        memFree((ubyte *)bullet[33]);
        bullet[33] = 0;
    }
    // Clear back-reference in parent ship
    parentShip = bullet[77];
    if (parentShip)
        *(_DWORD *)(parentShip + 232) = 0;
    // Remove from collision blob system
    bobObjectDied(bullet, &universe.collBlobList.head);
    // Remove from bullet linked list
    listRemoveNode(bullet + 53);
    // Remove from render list if flagged
    if (bullet[11])
        listRemoveNode(bullet + 9);
    // Delete the node
    return listDeleteNode(bullet);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x545DD0
char __cdecl AddTargetToDeleteList(SpaceObjRotImpTarg *target, sdword deathBy)
{
int v2; // eax
_DWORD *v3; // eax
_DWORD *v4; // eax
_DWORD *v5; // edi

v2 = *(_DWORD *)(a1 + 16);
switch ( v2 )
{
case 0:
LOBYTE(v2) = AddShipToDeleteShipList(a1, a2);
break;
case 2:
case 3:
case 4:
case 5:
v3 = memAllocFunction(20, MBF_SmallBlockHeap);
*(_DWORD *)(a1 + 20) |= 0x800u;
v3[4] = a1;
LOBYTE(v2) = (unsigned __int8)listAddNode(&universe.DeleteResourceList.head, v3, (int)v3);
break;
case 6:
v5 = memAllocFunction(24, MBF_SmallBlockHeap);
bobObjectDied((_DWORD *)a1, &universe.collBlobList.head);
*(_DWORD *)(a1 + 20) |= 0x800u;
v5[4] = a1;
if ( (double)dword_C428A0[*(_DWORD *)(a1 + 656)] < *(float *)(a1 + 368) )
v5[5] = 0;
else
v5[5] = (unsigned __int8)byte_87AC64[a2];
LOBYTE(v2) = (unsigned __int8)listAddNode(&universe.DeleteDerelictList.head, v5, (int)v5);
break;
case 8:
v4 = memAllocFunction(24, MBF_SmallBlockHeap);
*(_DWORD *)(a1 + 20) |= 0x800u;
v4[4] = a1;
v4[5] = a2;
LOBYTE(v2) = (unsigned __int8)listAddNode(&universe.DeleteMissileList.head, v4, (int)v4);
break;
default:
return v2;
}
return v2;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5432D0
void univUpdateAllPosVelResources(void)
{
Node *i; // edi
int structptr; // esi
double v2; // st7
double v3; // st7
double v4; // st7
__int16 v5; // ax

for ( i = universe.ResourceList.head; i; i = i->next )
{
structptr = (int)i->structptr;
if ( (*(_BYTE *)(structptr + 20) & 0x84) == 0 )
{
physUpdateObjPosVelBasic(structptr, universe.unknown1);
if ( (*(_DWORD *)(structptr + 56) & 0x200) != 0 )
{
v2 = -*(float *)(structptr + 424);
if ( v2 <= *(float *)(structptr + 88) )
{
if ( *(float *)(structptr + 88) > (double)*(float *)(structptr + 424) )
*(float *)(structptr + 88) = v2;
}
else
{
*(_DWORD *)(structptr + 88) = *(_DWORD *)(structptr + 424);
}
v3 = -*(float *)(structptr + 428);
if ( v3 <= *(float *)(structptr + 92) )
{
if ( *(float *)(structptr + 92) > (double)*(float *)(structptr + 428) )
*(float *)(structptr + 92) = v3;
}
else
{
*(_DWORD *)(structptr + 92) = *(_DWORD *)(structptr + 428);
}
v4 = -*(float *)(structptr + 432);
if ( v4 <= *(float *)(structptr + 96) )
{
if ( *(float *)(structptr + 96) > (double)*(float *)(structptr + 432) )
*(float *)(structptr + 96) = v4;
}
else
{
*(_DWORD *)(structptr + 96) = *(_DWORD *)(structptr + 432);
}
}
}
v5 = *(_WORD *)(structptr + 398);
if ( v5 )
*(_WORD *)(structptr + 398) = v5 - 1;
}
}

#endif

/*=== univFreeAttackTargets (0x541df0) ===*/
static sdword lodFreeMeshBindingsRecursive(lodmeshbinding *bindings, sdword index, polygonobject *object)
{
    polygonobject *daughter;
    sdword nextIndex;

    nextIndex = index + 1;
    for (daughter = object->pDaughter; daughter != NULL; daughter = daughter->pSister)
    {
        nextIndex = lodFreeMeshBindingsRecursive(bindings, nextIndex, daughter);
    }

    if (bindings[index].vertexLightingData != NULL)
    {
        memFree(bindings[index].vertexLightingData);
        bindings[index].vertexLightingData = NULL;
    }
    return nextIndex;
}

void univFreeAttackTargets(Ship *ship)
{
    lodmeshbindings *bindings;
    lodinfo *lodInfo;
    sdword levelIndex;

    bindings = ship->posinfo.lodMeshBindings;
    if (bindings == NULL)
    {
        return;
    }

    lodInfo = ship->staticinfo->staticheader.LOD;
    for (levelIndex = 0; levelIndex < lodInfo->nLevels; levelIndex++)
    {
        lodmeshbinding *levelBindings;

        levelBindings = bindings->level[levelIndex];
        if (levelBindings != NULL)
        {
            lod *lodLevel;
            polygonobject *object;
            sdword objectIndex;

            lodLevel = &lodInfo->level[levelIndex];
            objectIndex = 0;
            for (object = &((meshdata *)lodLevel->pData)->object[0]; object != NULL; object = object->pSister)
            {
                objectIndex = lodFreeMeshBindingsRecursive(levelBindings, objectIndex, object);
            }
            memFree(levelBindings);
            bindings->level[levelIndex] = NULL;
        }
    }

    memFree(bindings);
    ship->posinfo.lodMeshBindings = NULL;
}

#if 0  // Cat2: univCreateClampedChildren live code differs from binary
/*=== univCreateClampedChildren (0x5420b0) ===*/
/*
_DWORD *__cdecl univCreateClampedChildren(_DWORD *a1, ShipRaceEnum shiprace, vector *shippos, struct Player *ship, sdword built)
{
  _DWORD *result; // eax
  ShipTypeEnum *v7; // edi
  Ship *v8; // eax
  Ship *v9; // esi
  ShipTypeEnum *v10; // edi
  Ship *v11; // esi
  _DWORD v12[20]; // [esp+10h] [ebp-54h] BYREF
  int v13; // [esp+60h] [ebp-4h]
  int v14; // [esp+68h] [ebp+4h]

  result = (_DWORD *)a1[172];
  if ( result == (_DWORD *)18 )
  {
    sub_53F680(v12);
  }
  else
  {
    if ( result != (_DWORD *)53 )
      return result;
    sub_53F840(v12);
  }
  v14 = 0;
  if ( v13 > 0 )
  {
    v7 = (ShipTypeEnum *)v12;
    do
    {
      v8 = univCreateShip(*v7, shiprace, shippos, ship, built);
      v9 = v8;
      if ( shiprace == Race_Sect )
        ModuleCompleted(ship, v8->shiptype, 0);
      v9->posinfo.position.x = (real32)a1[22];
      v9->posinfo.position.y = (real32)a1[23];
      v9->posinfo.position.z = (real32)a1[24];
      v9->posinfo.velocity.x = (real32)a1[25];
      v9->posinfo.velocity.y = (real32)a1[26];
      v9->posinfo.velocity.z = (real32)a1[27];
      clampObjectSetup((int)v9, (int)a1);
      v9->flags |= 0x40000000u;
      ++v7;
      ++v14;
    }
    while ( v14 < v13 );
  }
  result = (_DWORD *)singlePlayerGame;
  if ( !singlePlayerGame && (tpGameCreated_flag & 8) == 0 && a1[172] == 18 )
  {
    v10 = (ShipTypeEnum *)&unk_8B5C28;
    do
    {
      v11 = univCreateShip(*v10, shiprace, shippos, ship, built);
      cat_unitCapCreateShip_(v11, (int)v11->playerowner);
      v11->posinfo.position.x = (real32)a1[22];
      v11->posinfo.position.y = (real32)a1[23];
      v11->posinfo.position.z = (real32)a1[24];
      v11->posinfo.velocity = *(vector *)(a1 + 25);
      result = clampObjectSetup((int)v11, (int)a1);
      ++v10;
    }
    while ( (int)v10 < (int)&flt_8B5C44 );
  }
  return result;
}
*/
/*=== univPingShipDied (0x5422b0) ===*/
/*
sdword __cdecl univPingShipDied(int a1)
{
  sdword result; // eax
  int v2; // edi
  float MaxHealth; // [esp+0h] [ebp-10h]

  result = playerGetSensorCoverage(*(_DWORD *)(a1 + 812));
  if ( result )
  {
    v2 = univSetCurrentSensorCoverage(result);
    if ( physShipIsMovable(a1) )
    {
      MaxHealth = univGetMaxHealth((_DWORD *)a1, 0);
      sub_47F1D0(*(float *)(a1 + 88), *(float *)(a1 + 92), *(float *)(a1 + 96), MaxHealth);
    }
    return univSetCurrentSensorCoverage(v2);
  }
  return result;
}
*/
/*=== univUpdateClampVelocity (0x542cb0) ===*/
/*
void __cdecl univUpdateClampVelocity(int a1)
{
  float *v2; // ecx
  int v3; // eax
  real32 v4; // edx
  real32 x; // edi
  real32 y; // ebx
  double v7; // st7
  real32 z; // ebp
  double v9; // st4
  double v10; // st7
  float *v11; // eax
  double v12; // st7
  real32 v13; // eax
  float *v14; // ecx
  double v15; // st7
  int v16; // eax
  int v17; // edx
  real32 *v18; // eax
  real32 v19; // edx
  real32 v20; // edx
  real32 v21; // eax
  real32 v22; // ecx
  real32 v23; // ecx
  real32 v24; // edx
  float v25; // [esp+1Ch] [ebp-4Ch]
  vector a; // [esp+20h] [ebp-48h] BYREF
  vector v27; // [esp+2Ch] [ebp-3Ch]
  vector v28; // [esp+38h] [ebp-30h] BYREF
  float v29; // [esp+44h] [ebp-24h] BYREF
  float v30; // [esp+48h] [ebp-20h]
  float v31; // [esp+4Ch] [ebp-1Ch]
  vector v32; // [esp+50h] [ebp-18h]
  float v33; // [esp+5Ch] [ebp-Ch]
  float v34; // [esp+60h] [ebp-8h]
  float v35; // [esp+64h] [ebp-4h]
  float v36; // [esp+6Ch] [ebp+4h]

  v2 = *(float **)(a1 + 292);
  if ( v2 )
  {
    if ( *(float *)(a1 + 276) > (double)flt_8B5C54 )
    {
      if ( *((_DWORD *)v2 + 4) || (v3 = *((_DWORD *)v2 + 172), v3 != 9) && v3 != 43 )
      {
        v29 = v2[56];
        v30 = v2[57];
        v31 = v2[58];
        v28.x = *(real32 *)(a1 + 88);
        v4 = *(float *)(a1 + 96);
        v28.y = *(real32 *)(a1 + 92);
        v28.z = v4;
        sub_415AF0((int)v2, &v29);
        v27.x = v29 - v28.x;
        x = v27.x;
        v27.y = v30 - v28.y;
        y = v27.y;
        v7 = v31 - v28.z;
        v27.z = v7;
        z = v27.z;
        a = v27;
        v25 = v7 * v27.z + v27.y * v27.y + v27.x * v27.x;
        vecNormalize(&a);
        v32 = a;
        if ( v25 < (double)flt_8B5C58
          || v27.z * *(float *)(a1 + 108) + v27.y * *(float *)(a1 + 104) + v27.x * *(float *)(a1 + 100) >= flt_8B5C50 )
        {
          v9 = *(float *)(a1 + 100) * *(float *)(a1 + 100)
             + *(float *)(a1 + 104) * *(float *)(a1 + 104)
             + *(float *)(a1 + 108) * *(float *)(a1 + 108);
          if ( v9 >= flt_8B5C48 + flt_8B5C44 || v9 <= flt_8B5C44 - flt_8B5C48 )
            v36 = fmathSqrtDouble(v9);
          else
            v36 = *(float *)&dword_8B5C4C;
          if ( v25 < (double)flt_8B5C58 && v36 > (double)ZERO )
          {
            v28.x = x;
            v28.y = y;
            v28.z = z;
            v10 = fmathSqrtDouble(v25) / v36;
            v11 = *(float **)(a1 + 292);
            v33 = v10 * v11[25];
            v34 = v10 * v11[26];
            v12 = v10 * v11[27];
            v28.x = v33 + v27.x;
            v28.y = v34 + v28.y;
            v28.z = v12 + v28.z;
            a = v28;
            vecNormalize(&a);
          }
          v13 = a.x;
          v14 = (float *)(a1 + 100);
          *(float *)(a1 + 100) = a.x * v36;
          *(real32 *)(a1 + 228) = v13;
          v15 = a.y * v36;
          *(real32 *)(a1 + 232) = a.y;
          *(real32 *)(a1 + 236) = a.z;
          *(float *)(a1 + 104) = v15;
          *(float *)(a1 + 108) = a.z * v36;
          if ( v25 < (double)flt_8B5C5C )
          {
            v16 = *(_DWORD *)(a1 + 292);
            if ( *(_DWORD *)(v16 + 16) || (v17 = *(_DWORD *)(v16 + 688), v17 != 9) && v17 != 43 )
            {
              v18 = (real32 *)(v16 + 100);
              v28.x = *v18;
              v19 = v18[1];
              v28.z = v18[2];
              v28.y = v19;
              if ( v36 > (double)flt_8B5C60 )
                v36 = v36 / flt_8B5C64;
              v34 = v32.y * v36;
              v35 = v32.z * v36;
              *v14 = v28.x + v32.x * v36;
              *(float *)(a1 + 104) = v28.y + v34;
              *(float *)(a1 + 108) = v28.z + v35;
            }
            v20 = *v14;
            v21 = *(float *)(a1 + 104);
            v22 = *(float *)(a1 + 108);
            v28.x = v20;
            v28.y = v21;
            v28.z = v22;
            vecNormalize(&v28);
            v23 = v28.y;
            v24 = v28.z;
            *(real32 *)(a1 + 228) = v28.x;
            *(real32 *)(a1 + 232) = v23;
            *(real32 *)(a1 + 236) = v24;
          }
        }
      }
    }
  }
}
*/
/*=== univProcessDelayedShipDeath (0x543860) ===*/
/*
signed int __cdecl univProcessDelayedShipDeath(int a1)
{
  int v1; // eax
  unsigned int v2; // ecx
  int v3; // eax
  _DWORD *v4; // eax
  int v6[3]; // [esp+4h] [ebp-Ch] BYREF

  if ( *(float *)(a1 + 1252) == ZERO || nisIsRunning_ )
    return 0;
  if ( *(_DWORD *)(a1 + 880) )
  {
    v6[0] = 1;
    v6[1] = 0;
    v6[2] = a1;
    RemoveShipsFromDoingStuff((int **)&universe, v6);
  }
  if ( universe.totaltimeelapsed < (double)*(float *)(a1 + 1252) )
    return 0;
  v1 = sub_429B70(a1);
  if ( v1 )
    v2 = *(_DWORD *)(v1 + 8);
  else
    v2 = 49;
  ++dword_A6D9A8[8 * *(unsigned __int16 *)(*(_DWORD *)(a1 + 1260) + 36) + v2];
  v3 = *(_DWORD *)(a1 + 1260);
  *(_DWORD *)(a1 + 924) = 666;
  *(_DWORD *)(a1 + 928) = *(unsigned __int16 *)(v3 + 36);
  v4 = univSpawnReplacementShip(a1, v2, v3);
  if ( v4 )
    v4[115] |= 0x10000u;
  return 1;
}
*/
/*=== univSpawnReplacementShip (0x543950) ===*/
/*
Ship *__cdecl univSpawnReplacementShip(int a1, ShipTypeEnum shiptype, struct Player *playerowner)
{
  Ship *Ship; // ebp
  ubyte *v4; // eax
  unsigned int v5; // eax
  udword v7; // eax
  sdword v8; // edx
  int v9; // eax
  int *v10; // eax
  int *unk_490; // ecx
  int v12; // ecx
  int v13; // eax
  int v14; // eax
  udword flags; // eax
  bool16 v16; // dx
  ShipStaticInfo2 *staticinfo; // ecx
  sdword maxresources; // ecx
  double totaltimeelapsed; // st7
  double v20; // st7
  _DWORD *v21; // esi
  _DWORD *v22; // esi
  float v23; // [esp+10h] [ebp-10D0h]
  int v24; // [esp+14h] [ebp-10CCh]
  int v25; // [esp+18h] [ebp-10C8h]
  udword v26; // [esp+1Ch] [ebp-10C4h]
  sdword v27; // [esp+20h] [ebp-10C0h]
  udword v28; // [esp+24h] [ebp-10BCh]
  int v29; // [esp+28h] [ebp-10B8h]
  _BYTE shippos[40]; // [esp+2Ch] [ebp-10B4h] BYREF
  _DWORD v31[42]; // [esp+54h] [ebp-108Ch] BYREF
  char v32[60]; // [esp+FCh] [ebp-FE4h] BYREF
  SelectCommand v33[334]; // [esp+138h] [ebp-FA8h] BYREF

  Ship = 0;
  v25 = *(_DWORD *)(a1 + 812);
  v4 = *(ubyte **)(a1 + 1276);
  v24 = *(_DWORD *)(a1 + 688);
  *(_DWORD *)(a1 + 1252) = 0;
  if ( v4 )
  {
    memFree(v4);
    *(_DWORD *)(a1 + 1276) = 0;
  }
  if ( *(_DWORD *)(a1 + 1296) )
  {
    memFree(*(ubyte **)(a1 + 1296));
    *(_DWORD *)(a1 + 1296) = 0;
  }
  if ( !sub_459B40(shiptype, (int)playerowner) && !*(_DWORD *)(a1 + 1004) )
  {
    if ( playerowner == universe.curPlayerPtr )
      clCommandMessage((const char *)dword_BB5008, -16776961);
    ApplyDamageToTarget(a1, 5000.0, 19, 666, playerowner->playerIndex, 0);
    v5 = *(_DWORD *)(a1 + 20) & 0xFF5FFFFF;
    *(_DWORD *)(a1 + 1252) = 0;
    *(_DWORD *)(a1 + 20) = v5;
    return Ship;
  }
  v7 = *(_DWORD *)(a1 + 1008);
  v8 = *(_DWORD *)(a1 + 816);
  qmemcpy(v32, (const void *)(a1 + 152), sizeof(v32));
  qmemcpy(shippos, (const void *)(a1 + 88), sizeof(shippos));
  v23 = *(float *)(a1 + 368);
  v28 = v7;
  v26 = *(_DWORD *)(a1 + 1012);
  v27 = v8;
  v29 = *(_DWORD *)(a1 + 1004);
  v9 = sub_4D7170(shiptype);
  Ship = univCreateShip(shiptype, (ShipRaceEnum)v9, (vector *)shippos, playerowner, 0);
  v10 = *(int **)(a1 + 1168);
  if ( v10 )
  {
    unk_490 = (int *)Ship->unk_490;
    if ( unk_490 )
    {
      v12 = *unk_490;
      v13 = *v10;
      if ( v12 > v13 )
        v12 = v13;
      if ( v12 > 0 )
      {
        v14 = 0;
        do
        {
          *(_DWORD *)(v14 + Ship->unk_490 + 4) = *(_DWORD *)(v14 + *(_DWORD *)(a1 + 1168) + 4);
          v14 += 136;
          --v12;
        }
        while ( v12 );
      }
    }
  }
  if ( *(_DWORD *)(a1 + 44) )
  {
    flags = Ship->flags;
    if ( (flags & SOF_DockMarker) == 0 )
    {
      if ( (flags & 0x4000000) != 0 )
      {
        univAddObjToRenderList((int)Ship);
      }
      else
      {
        Ship->cameraDistanceVector.x = *(real32 *)(a1 + 68);
        Ship->cameraDistanceVector.y = *(real32 *)(a1 + 72);
        Ship->cameraDistanceVector.z = *(real32 *)(a1 + 76);
        Ship->cameraDistanceSquared = *(real32 *)(a1 + 64);
        listAddNodeAfter((int *)(a1 + 36), (int *)&Ship->renderlink, (int)Ship);
      }
    }
  }
  cat_unitCapCreateShip_(Ship, (int)Ship->playerowner);
  qmemcpy(
    (void *)(Ship->playerowner[1].shiptotals[43] + 168 * shiptype),
    (const void *)(*(_DWORD *)(*(_DWORD *)(a1 + 812) + 848) + 168 * *(_DWORD *)(a1 + 688)),
    0xA8u);
  ShipTypeToStr(*(_DWORD *)(a1 + 688));
  telldir();
  sub_547D00(a1, -1);
  if ( *(_DWORD *)(a1 + 360) )
    listRemoveNode((_DWORD *)(a1 + 352));
  sub_5477D0(a1);
  Ship->posinfo.velocity.x = *(real32 *)&shippos[12];
  Ship->posinfo.velocity.y = *(real32 *)&shippos[16];
  Ship->posinfo.velocity.z = *(real32 *)&shippos[20];
  Ship->posinfo.force.x = *(real32 *)&shippos[24];
  Ship->posinfo.force.y = *(real32 *)&shippos[28];
  Ship->posinfo.force.z = *(real32 *)&shippos[32];
  v16 = *(_WORD *)&shippos[36];
  qmemcpy(&Ship->rotinfo.coordsys.m13, v32, 0x3Cu);
  staticinfo = Ship->staticinfo;
  Ship->health = v23;
  Ship->posinfo.isMoving = v16;
  if ( v23 > (double)staticinfo->maxhealth )
    Ship->health = staticinfo->maxhealth;
  Ship->lastTimeRepaired = 0.0;
  Ship->resources = v27;
  maxresources = staticinfo->maxresources;
  if ( v27 > maxresources )
    Ship->resources = maxresources;
  Ship->upgradeState.level = v28;
  Ship->upgradeState.mask = v26;
  univUpdateObjRotInfo((int)Ship);
  if ( playerowner->race != Race_Beast )
    return Ship;
  if ( !v29 || *(_DWORD *)(v29 + 692) == 1 )
  {
    totaltimeelapsed = universe.totaltimeelapsed;
    Ship->statusWindow.state = 2;
    v20 = totaltimeelapsed + ZERO_POINT_FIVE;
    Ship->statusWindow.phaseStartTime = v20;
    Ship->statusWindow.phaseEndTime = v20 + ZERO_POINT_FIVE;
  }
  if ( singlePlayerGame )
    return Ship;
  if ( !playerowner->aiPlayer || Ship->shiptype == bProcessor )
  {
    if ( Ship->shiptype == bProcessor )
    {
      sub_4F4870(Ship);
      if ( universe.curPlayerPtr != playerowner )
        return Ship;
      rmPreRequisitesAnd(v31, *(_DWORD *)(v25 + 848) + 168 * v24, playerowner[1].shiptotals);
      if ( rmTechDependenciesEqual((int)v31, (_DWORD *)(*(_DWORD *)(v25 + 848) + 168 * v24)) )
      {
        v21 = (_DWORD *)(168 * v24 + v25 + 904);
        rmPreRequisitesAnd(v31, (int)v21, playerowner[1].shiptotals);
        if ( rmTechDependenciesEqual((int)v31, v21) )
          return Ship;
      }
      if ( universe.curPlayerPtr != playerowner )
        return Ship;
      battleChatAddMessage((const char *)dword_BB5000, -16711936, 1, (int)Ship);
      return Ship;
    }
    else
    {
      if ( universe.curPlayerPtr != playerowner )
        return Ship;
      rmPreRequisitesAnd(v31, *(_DWORD *)(v25 + 848) + 168 * v24, playerowner[1].shiptotals);
      if ( rmTechDependenciesEqual((int)v31, (_DWORD *)(*(_DWORD *)(v25 + 848) + 168 * v24)) )
      {
        v22 = (_DWORD *)(168 * v24 + v25 + 904);
        rmPreRequisitesAnd(v31, (int)v22, playerowner[1].shiptotals);
        if ( rmTechDependenciesEqual((int)v31, v22) )
          return Ship;
      }
      if ( universe.curPlayerPtr != playerowner )
        return Ship;
      battleChatAddMessage((const char *)dword_BB5004, -16711936, 1, (int)Ship);
      return Ship;
    }
  }
  else
  {
    v33[0].ShipPtr[0] = Ship;
    v33[0].numShips = 1;
    universe.aiplayerProcessing = 1;
    if ( ranRandom(6) % 0x64 + 1 >= 0xA )
      clWrapDock(&universe.mainCommandLayer, v33, 0, 0);
    else
      clWrapDock(&universe.mainCommandLayer, v33, 7u, 0);
    universe.aiplayerProcessing = 0;
    return Ship;
  }
}
*/
/*=== univBroadcastGameStats (0x548750) ===*/
/*
void univBroadcastGameStats()
{
  int *v0; // ebx
  sdword *p_totalResourceUnitsSpent; // edi
  char *v2; // esi
  PlayerState *p_playerState; // ebp
  __int64 v4; // rax
  int v5; // ecx
  int numPlayers; // [esp+0h] [ebp-68h]
  uword v7[4]; // [esp+4h] [ebp-64h] BYREF
  char v8; // [esp+Ch] [ebp-5Ch] BYREF

  v7[0] = universe.numPlayers;
  if ( universe.numPlayers )
  {
    v0 = dword_88611C;
    p_totalResourceUnitsSpent = &universe.gameStats.playerStats[1].totalResourceUnitsSpent;
    v2 = &v8;
    p_playerState = &universe.players[0].playerState;
    numPlayers = universe.numPlayers;
    do
    {
      if ( *p_playerState )
        *((_WORD *)v2 + 3) = 0;
      else
        *((_WORD *)v2 + 3) = 1;
      v4 = (__int64)*((float *)p_totalResourceUnitsSpent - 500);
      *(_DWORD *)v2 = v4;
      if ( !(_DWORD)v4 )
        *(_DWORD *)v2 = (__int64)universe.totaltimeelapsed;
      v5 = *v0;
      *((_WORD *)v2 + 2) = *(_WORD *)p_totalResourceUnitsSpent;
      *((_DWORD *)v2 - 1) = v5;
      p_playerState += 6476;
      p_totalResourceUnitsSpent += 510;
      v0 += 15;
      v2 += 12;
      --numPlayers;
    }
    while ( numPlayers );
  }
  sub_58B420(18, v7, 0x64u);
}
*/
/*=== univFindReplacementMothership (0x548ad0) ===*/
/*
signed int __cdecl univFindReplacementMothership(int a1)
{
  Node *head; // ecx
  _DWORD *structptr; // esi
  int v3; // eax
  _DWORD *v4; // edx
  _DWORD *v5; // eax
  int v6; // ecx

  head = universe.ShipList.head;
  if ( !universe.ShipList.head )
  {
LABEL_13:
    if ( singlePlayerGame )
      return 0;
    if ( (tpGameCreated_flag & 0x4000) == 0 )
      return 0;
    v4 = (_DWORD *)dword_C0A5EC;
    if ( !dword_C0A5EC )
      return 0;
    while ( 1 )
    {
      v5 = *(_DWORD **)(v4[3] + 16);
      if ( (v5[5] & 0x800) == 0 && v5[203] == a1 && v5 != *(_DWORD **)a1 )
      {
        v6 = *(_DWORD *)(a1 + 20);
        if ( !v6 && (v5[172] == 12 || !*(_DWORD *)(v5[8] + 488)) )
          break;
        if ( v6 == 1 && (v5[172] == 47 || !*(_DWORD *)(v5[8] + 488)) )
          break;
      }
      v4 = (_DWORD *)*v4;
      if ( !v4 )
        return 0;
    }
    *(_DWORD *)a1 = v5;
    return 1;
  }
  while ( 1 )
  {
    structptr = head->structptr;
    if ( (structptr[5] & 0x800) != 0 || structptr[203] != a1 || structptr == *(_DWORD **)a1 )
      goto LABEL_12;
    v3 = *(_DWORD *)(a1 + 20);
    if ( !v3 )
      break;
    if ( v3 == 1 && (structptr[172] == 47 || !*(_DWORD *)(structptr[8] + 488)) )
    {
      if ( *(_DWORD *)a1 && gameIsRunning && *(_DWORD *)(*(_DWORD *)a1 + 688) == 53 )
        goto LABEL_34;
      goto LABEL_35;
    }
LABEL_12:
    head = head->next;
    if ( !head )
      goto LABEL_13;
  }
  if ( structptr[172] != 12 && *(_DWORD *)(structptr[8] + 488) )
    goto LABEL_12;
  if ( *(_DWORD *)a1 && gameIsRunning && *(_DWORD *)(*(_DWORD *)a1 + 688) == 18 )
LABEL_34:
    speechEventQueue(0, 33558557, -1, -1, 0, *(unsigned __int16 *)(a1 + 36), -1, 5.0, -1);
LABEL_35:
  *(_DWORD *)a1 = structptr;
  return 1;
}
*/
#endif
void univUpdateShipAITick(Ship *ship)
{
    udword runtimeFlags = UPDATE_AI_RUNTIME_FLAGS(ship);
    SelectCommand selectcom;
    SelectAnyCommand *nearbyTargets = NULL;
    bool32 fetchedNearbyTargets = FALSE;
    sdword specialChance;
    ShipType shiptype;
    if ((runtimeFlags & UPDATE_AI_BUILD_PROGRESS_FLAG) != 0)
    {
        if (ship->buildTimerElapsed > 0.0f)
        {
            ship->buildTimerElapsed = TimerTimeRemaining(&ship->buildTimer);
            if (ship->buildTimerElapsed > 1.0f)
            {
                ship->buildTimerElapsed = 1.0f;
            }

            ship->health = ship->staticinfo->maxhealth * ship->buildTimerElapsed;
            if (ship->health >= ship->staticinfo->maxhealth)
            {
                ship->health = ship->staticinfo->maxhealth;
                UPDATE_AI_RUNTIME_FLAGS(ship) &= ~BIT3;
                if (ship->shiprace == RACE_Sect && ship->staticinfo->shipclass == CLASS_Component)
                {
                    ModuleCompleted(ship->playerowner, ship->shiptype, FALSE);
                }
                UPDATE_AI_RUNTIME_FLAGS(ship) &= ~UPDATE_AI_BUILD_PROGRESS_FLAG;
            }
        }
        else
        {
            real32 duration = (real32)(ship->staticinfo->buildTime / 16);

            TimerInitTimerStruct(&ship->buildTimer, duration);
            ship->buildTimerElapsed = 1.0e-6f;
            ship->playerowner->resourceUnits -= ship->staticinfo->buildCost;
        }
        return;
    }

    if ((runtimeFlags & BIT3) == 0 && ship->playerowner->aiPlayer != NULL)
    {
        memset(&selectcom, 0, sizeof(selectcom));
        selectcom.ShipPtr[0] = ship;
        selectcom.numShips = 1;

        if ((universe.univUpdateCounter & UPDATE_AI_UPGRADE_CHECK_MASK) ==
            (UPDATE_AI_UPGRADE_CHECK_MASK & ship->shipID.shipNumber))
        {
            if (shipHasAvailableUpgradeChanges(ship))
            {
                if (ShipStaticInfoUpgradeByDockingEnabled(ship->staticinfo))
                {
                    CommandToDo *command = ship->command;

                    if (command == NULL ||
                        command->ordertype.order == COMMAND_NULL ||
                        command->ordertype.order == COMMAND_MILITARYPARADE)
                    {
                        clWrapDock(&universe.mainCommandLayer, &selectcom, 0, 0);
                    }
                }
                else
                {
                    clWrapUpgrade(&universe.mainCommandLayer, &selectcom, 0);
                }
            }
        }

        if ((universe.univUpdateCounter & UPDATE_AI_SPECIAL_CHECK_MASK) ==
            (UPDATE_AI_SPECIAL_CHECK_MASK & ship->shipID.shipNumber))
        {
            udword specialRoll;

            if (singlePlayerGame)
            {
                switch (g_GameDifficulty)
                {
                    case 1:
                        specialChance = 2;
                        break;
                    case 2:
                        specialChance = 5;
                        break;
                    case 3:
                        specialChance = 10;
                        break;
                    case 4:
                        specialChance = 15;
                        break;
                    default:
                        specialChance = 1;
                        break;
                }
            }
            else
            {
                switch (ship->playerowner->aiPlayer->aiplayerDifficultyLevel)
                {
                    case AI_INTERMEDIATE:
                        specialChance = 5;
                        break;
                    case AI_ADVANCED:
                        specialChance = 10;
                        break;
                    default:
                        specialChance = 1;
                        break;
                }
            }

            {
                udword specialRollValue = ranRandom(RAN_AIPlayer);
                specialRoll = specialRollValue % 0xC9;
                aisParityLogChannel(
                    "ai",
                    "event=external_build_roll site=special_roll type=%d id=%u owner=%u slot=%u raw=%u mod=%u chance=%d",
                    (int)ship->shiptype,
                    (unsigned int)ship->shipID.shipNumber,
                    ship->playerowner != NULL ? (unsigned int)ship->playerowner->playerIndex : (unsigned int)UWORD_Max,
                    (unsigned int)(ship->shipID.shipNumber & UPDATE_AI_SPECIAL_CHECK_MASK),
                    (unsigned int)specialRollValue,
                    (unsigned int)specialRoll,
                    (int)specialChance);
            }
            if ((sdword)specialRoll < specialChance)
            {
                nearbyTargets = univFindNearbyTargets(ship, univGetMaxHealth(ship, FALSE));
                fetchedNearbyTargets = TRUE;
                if (nearbyTargets != NULL && nearbyTargets->numTargets > 0)
                {
                    SpecialCommand *specialTargets = NULL;

                    switch (ship->shiptype)
                    {
                        case sMimic:
                        case sMCV:
                            if (!singlePlayerGame && !shipHasModuleSlotState(ship))
                            {
                                clWrapExternalConstructionSelection(
                                    &universe.mainCommandLayer,
                                    &selectcom,
                                    1,
                                    aiSelectRandomShipType(ship->shiptype, ship->playerowner->race),
                                    ship->playerowner->playerIndex,
                                    -1);
                            }
                            break;

                        case sLeech:
                            if (!IsShipInfected(ship))
                            {
                                specialTargets = univUpdateShipAITickCopySpecialTargets(nearbyTargets);
                            }
                            break;

                        case sACV:
                        case bACV:
                            if (!singlePlayerGame)
                            {
                                specialTargets = univUpdateShipAITickCopySpecialTargets(nearbyTargets);
                            }
                            break;

                        case sRammingFrigate:
                        case bRammingFrigate:
                            if (!singlePlayerGame && !shipIsRammingFrigateType(ship))
                            {
                                specialTargets = univUpdateShipAITickCopySpecialTargets(nearbyTargets);
                            }
                            break;

                        case sHiveFrigate:
                        case bHiveFrigate:
                            if (!singlePlayerGame && !shipHasReadySwarmers(ship))
                            {
                                specialTargets = univUpdateShipAITickCopySpecialTargets(nearbyTargets);
                            }
                            break;

                        case sDreadnought:
                            if (!singlePlayerGame)
                            {
                                clWrapSpecial(&universe.mainCommandLayer, &selectcom, NULL);
                            }
                            break;

                        case bHeavyCruiser:
                        case bCarrier:
                        case bMothership:
                            if ((runtimeFlags & 0x800000u) == 0)
                            {
                                specialTargets = univUpdateShipAITickCopySpecialTargets(nearbyTargets);
                            }
                            break;

                        default:
                            break;
                    }

                    if (specialTargets != NULL)
                    {
                        clWrapSpecial(&universe.mainCommandLayer, &selectcom, specialTargets);
                        memFree(specialTargets);
                    }
                }
            }

            shiptype = ship->shiptype;
            if ((shiptype == bCloakedFighter || shiptype == bIonArrayFrigate) &&
                !singlePlayerGame &&
                univIsShipCloaked((SpaceObj *)ship))
            {
                clWrapSpecial(&universe.mainCommandLayer, &selectcom, NULL);
            }

            shiptype = ship->shiptype;
            if (shiptype != sLeech &&
                shiptype != sWorker &&
                shiptype != bWorker &&
                !singlePlayerGame)
            {
                udword attackRollValue = ranRandom(RAN_AIPlayer);
                udword attackRoll = attackRollValue % 0xC9;

                aisParityLogChannel(
                    "ai",
                    "event=external_build_roll site=attack_roll type=%d id=%u owner=%u slot=%u raw=%u mod=%u chance=%d",
                    (int)ship->shiptype,
                    (unsigned int)ship->shipID.shipNumber,
                    ship->playerowner != NULL ? (unsigned int)ship->playerowner->playerIndex : (unsigned int)UWORD_Max,
                    (unsigned int)(ship->shipID.shipNumber & UPDATE_AI_SPECIAL_CHECK_MASK),
                    (unsigned int)attackRollValue,
                    (unsigned int)attackRoll,
                    (int)specialChance);

                if ((sdword)attackRoll < specialChance)
                {
                AttackCommand *attackTargets;
                udword priorityRollValue = ranRandom(RAN_AIPlayer);
                udword priorityRoll = priorityRollValue % 0x65;
                bool32 restrictToPriorityPlayers = priorityRoll < 0x64;

                aisParityLogChannel(
                    "ai",
                    "event=external_build_roll site=priority_roll type=%d id=%u owner=%u slot=%u raw=%u mod=%u",
                    (int)ship->shiptype,
                    (unsigned int)ship->shipID.shipNumber,
                    ship->playerowner != NULL ? (unsigned int)ship->playerowner->playerIndex : (unsigned int)UWORD_Max,
                    (unsigned int)(ship->shipID.shipNumber & UPDATE_AI_SPECIAL_CHECK_MASK),
                    (unsigned int)priorityRollValue,
                    (unsigned int)priorityRoll);

                    if (!fetchedNearbyTargets)
                    {
                        nearbyTargets = univFindNearbyTargets(ship, univGetMaxHealth(ship, FALSE));
                    }

                    if (nearbyTargets != NULL && nearbyTargets->numTargets > 0)
                    {
                        memset(&selectcom, 0, sizeof(selectcom));
                        selectcom.ShipPtr[0] = ship;
                        selectcom.numShips = 1;

                        attackTargets = univUpdateShipAITickMergeAttackTargets(
                            ship,
                            ship->command,
                            nearbyTargets,
                            restrictToPriorityPlayers);
                        if (attackTargets->numTargets > 0)
                        {
                            clWrapAttack(&universe.mainCommandLayer, &selectcom, attackTargets);
                        }
                        memFree(attackTargets);
                    }
                }
            }
        }
    }
}

/*=== univUpdateShipHousekeeping (0x54a230) ===*/
void univUpdateShipHousekeeping(Ship *ship)
{
    udword flags = ship->flags;
    sdword i;
    ShipStaticInfo *staticinfo;

    ship->gettingrocked = NULL;

    if ((flags & SOF_Dead) == 0)
    {
        staticinfo = (ShipStaticInfo *)ship->staticinfo;

        // Retail clears specialFlags1C8's BIT27 latch every housekeeping tick
        // (0x54a26d: and edi,0F7FFFFFFh on Ship+0x1C8). This is what lets
        // SalCapExecuteSpecialOrder's DISABLE_AND_TOW case (SalCapCorvette.c)
        // re-fire its tow-steering call every frame instead of once.
        ship->specialFlags1C8 &= ~0x08000000u;

        ship->unk_600 = 0;

        ship->shipidle = TRUE;
        ship->autostabilizeship = TRUE;

        // Count down recentlyAttacked; clear recentAttacker when expired
        if (ship->recentlyAttacked)
        {
            if (ship->recentAttacker)
            {
                ship->recentlyAttacked--;
                if (!ship->recentlyAttacked)
                    ship->recentAttacker = NULL;
            }
            else
            {
                ship->recentlyAttacked = 0;
            }
        }

        // Count down recentlyFiredUpon; clear firingAtUs when expired
        if (ship->recentlyFiredUpon)
        {
            if (ship->firingAtUs)
            {
                ship->recentlyFiredUpon--;
                if (!ship->recentlyFiredUpon)
                    ship->firingAtUs = NULL;
            }
            else
            {
                ship->recentlyFiredUpon = 0;
            }
        }

        // Clear SOF_TargetableHW1 each frame (re-set by AI/weapons as needed)
        ship->flags = flags & ~SOF_TargetableHW1;

        dmgShipThink(ship);

        for (i = 0; i < MAX_NUM_TRAILS; i++)
        {
            if (ship->trail[i] != NULL)
                trailUpdate(ship->trail[i], &ship->enginePosition);
        }

        if (staticinfo->custshipheader.CustShipHousekeep != NULL)
            growSelectAddShip(&universe.HousekeepShipList, ship);

        growSelectAddShip(&universe.unk1, ship);
    }
}

void cat_RegenerateShipHealth(Ship *ship)
{
    static const real32 catMothershipModuleRepairScale = 7.0f;
    ShipStaticInfo *staticinfo = ship->staticinfo;
    Ship *repairTargets[7] = { 0 };
    sdword repairCounts[3] = { 0 };
    bool32 inCombat;
    real32 repairDelay;
    real32 repairDamage;
    sdword i;

    inCombat = ship->shipisattacking
        || ship->recentlyAttacked
        || bitTest(ship->specialFlags1C8, BIT22);

    if (inCombat)
    {
        repairDelay = staticinfo->repairCombatTime;
        repairDamage = staticinfo->repairCombatDamage;
    }
    else
    {
        repairDelay = staticinfo->repairTime;
        repairDamage = staticinfo->repairDamage;
    }

    if ((!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPAIR_BOTS) && ship->shiprace != RACE_Beast)
        || repairDamage <= 0.0f
        || bitTest(ship->specialFlags2, SPECIAL_2_DockFlag))
    {
        return;
    }

    if (enableRepairbots && staticinfo->shiprace == RACE_Sect && ship->runtimeAnimationOverlay.runtimeController != 0)
    {
        repairDroidUpdate((RepairDroidController *)ship->runtimeAnimationOverlay.runtimeController);
    }

    if (staticinfo->buildTime == 10
        || (ship->health >= staticinfo->maxhealth && ship->shiptype != sMothership))
    {
        ship->lastTimeRepaired = universe.totaltimeelapsed;
        return;
    }

    if (universe.totaltimeelapsed - ship->lastTimeRepaired < repairDelay)
    {
        return;
    }

    ship->lastTimeRepaired = universe.totaltimeelapsed;

    if (ship->shiptype == sMothership && ship->clampInfo != NULL && ship->clampInfo->selection != NULL)
    {
        ShipModuleSlotIconState *moduleGroups = shipModuleSlotIconState(ship);
        SelectCommand *selection = ship->clampInfo->selection;

        for (i = 0; i < selection->numShips; i++)
        {
            Ship *module = selection->ShipPtr[i];
            sdword groupIndex;

            switch (module->shiptype)
            {
                case sMothershipDockingBay:
                    repairTargets[0] = module;
                    if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[0].iconId;
                        repairCounts[groupIndex]++;
                    }
                    else
                    {
                        repairTargets[0] = NULL;
                    }
                    break;

                case sMothershipWeapons:
                    repairTargets[1] = module;
                    if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[1].iconId;
                        repairCounts[groupIndex]++;
                    }
                    else
                    {
                        repairTargets[1] = NULL;
                    }
                    break;

                case sMothershipArmour:
                    repairTargets[2] = module;
                    if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[2].iconId;
                        repairCounts[groupIndex]++;
                    }
                    else
                    {
                        repairTargets[2] = NULL;
                    }
                    break;

                case sMothershipMicro:
                    repairTargets[3] = module;
                    if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[3].iconId;
                        repairCounts[groupIndex]++;
                    }
                    else
                    {
                        repairTargets[3] = NULL;
                    }
                    break;

                case sMothershipSpecial:
                    repairTargets[4] = module;
                    if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[4].iconId;
                        repairCounts[groupIndex]++;
                    }
                    else
                    {
                        repairTargets[4] = NULL;
                    }
                    break;

                case sMothershipSupport:
                    if (repairTargets[5] != NULL)
                    {
                        if (module->health < repairTargets[5]->health)
                        {
                            repairTargets[5] = module;
                        }
                    }
                    else if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[5].iconId;
                        repairCounts[groupIndex]++;
                        repairTargets[5] = module;
                    }
                    break;

                case sMothershipBigGun:
                    repairTargets[6] = module;
                    if (module->health < module->staticinfo->maxhealth)
                    {
                        groupIndex = moduleGroups[6].iconId;
                        repairCounts[groupIndex]++;
                    }
                    else
                    {
                        repairTargets[6] = NULL;
                    }
                    break;

                default:
                    break;
            }
        }

        for (i = 0; i < 7; i++)
        {
            Ship *module = repairTargets[i];
            sdword groupIndex;
            real32 moduleRepairDamage;

            if (module == NULL)
            {
                continue;
            }

            moduleRepairDamage = inCombat ? module->staticinfo->repairCombatDamage : module->staticinfo->repairDamage;
            if (moduleRepairDamage <= 0.0f)
            {
                continue;
            }

            groupIndex = moduleGroups[i].iconId;
            if (repairCounts[groupIndex] <= 0)
            {
                continue;
            }

            module->health += catMothershipModuleRepairScale
                / (real32)repairCounts[groupIndex]
                * module->damageModifier
                * moduleRepairDamage;
            if (module->health > module->staticinfo->maxhealth)
            {
                module->health = module->staticinfo->maxhealth;
            }
        }
    }

    if ((ship->shiptype != bMothership || !mothershipGetInfectionLevel(ship))
        && ship->health < staticinfo->maxhealth)
    {
        ship->health += repairDamage * ship->damageModifier;
        if (ship->health > staticinfo->maxhealth)
        {
            ship->health = staticinfo->maxhealth;
        }
    }
}

static void univShipHouseKeepImpl(Ship *ship)
{
    const real32 cataStatusWindowStartRandomScale = 1.862645e-10f;
    const real32 cataStatusWindowStartRandomBase = 0.4f;
    const real32 cataStatusWindowEndRandomScale = 3.49246e-10f;
    const real32 cataStatusWindowEndRandomBase = 1.5f;
    const real64 cataBurstSoundEventDelay = 0.1;
    const sdword cataSlotStatusMask = 0xF;
    typedef struct
    {
        udword unknown00;
        sdword state;
        ubyte unknown08_77[112];
        real32 startTime;
        real32 duration;
        udword unknown80;
        sdword phase;
    } CataBurstSoundEventEntry;

    typedef struct
    {
        sdword count;
        CataBurstSoundEventEntry entries[1];
    } CataBurstSoundEventArray;
    sdword i;

    if (bitTest(ship->flags2, SOF2_NotifyShipDiedPending))
    {
        Node *node;

        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *otherShip = (Ship *)listGetStructOfNode(node);
            void (*shipDiedFn)(Ship *, Ship *) = otherShip->staticinfo->custshipheader.CustShipNotifyShipDied;

            if (shipDiedFn != NULL)
            {
                shipDiedFn(otherShip, ship);
            }
        }

        for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
        {
            InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
            Ship *otherShip = insideShip->ship;
            void (*shipDiedFn)(Ship *, Ship *) = otherShip->staticinfo->custshipheader.CustShipNotifyShipDied;

            if (shipDiedFn != NULL)
            {
                shipDiedFn(otherShip, ship);
            }
        }

        bitClear(ship->flags2, SOF2_NotifyShipDiedPending);
    }

    if (univProcessDelayedShipDeath((sdword)ship))
    {
        return;
    }

    if (ship->dockvars.customdockinfo != NULL)
    {
        AttackCommand attackcom;
        SelectCommand selectcom;

        memset(&attackcom, 0, sizeof(attackcom));
        memset(&selectcom, 0, sizeof(selectcom));
        attackcom.TargetPtr[0] = (TargetPtr)ship->dockvars.customdockinfo;
        selectcom.numShips = 1;
        selectcom.ShipPtr[0] = ship;
        attackcom.numTargets = 1;
        clAttack(&universe.mainCommandLayer, &selectcom, &attackcom);
        ship->dockvars.customdockinfo = NULL;
    }

    if ((universe.univUpdateCounter & 0x1F) != 0)
    {
        ShipType shiptype = ship->shiptype;

        if ((shiptype == sSentinel || shiptype == bSentinel)
            && universe.totaltimeelapsed > sentinelShipSpecifics(ship)->unk348)
        {
            sentinelShipSpecifics(ship)->unk344 = 0;
        }
    }

    if (universe.totaltimeelapsed > univFormationSpeedNextUpdate)
    {
        ship->speedThrottle = -1.0f;
    }

    CrystalUpdateProcessingVisuals(ship);

    if (ship->statusWindow.state != 0 &&
        universe.totaltimeelapsed > ship->statusWindow.phaseStartTime &&
        universe.totaltimeelapsed > ship->statusWindow.phaseEndTime)
    {
        if (ship->statusWindow.state == 1)
        {
            if (ship->statusWindow.phaseScale < 0.0f)
            {
                ship->statusWindow.phaseStartTime =
                    (real32)ranRandom(10) * cataStatusWindowStartRandomScale +
                    cataStatusWindowStartRandomBase + universe.totaltimeelapsed;
            }
            else
            {
                ship->statusWindow.phaseStartTime = ship->statusWindow.phaseScale;
            }
            ship->statusWindow.phaseEndTime =
                (real32)ranRandom(10) * cataStatusWindowEndRandomScale +
                cataStatusWindowEndRandomBase + ship->statusWindow.phaseStartTime;
        }
        else
        {
            ship->statusWindow.state = 0;
        }
    }

    cat_RegenerateShipHealth(ship);

    if (ship->cataBurstFiring)
    {
        CataBurstSoundEventArray *eventArray = (CataBurstSoundEventArray *)ship->runtimeData490;

        if (eventArray->count > 0)
        {
            for (i = 0; i < eventArray->count; i++)
            {
                CataBurstSoundEventEntry *entry = &eventArray->entries[i];

                if (entry->state != 2 &&
                    entry->phase == 1 &&
                    entry->duration + cataBurstSoundEventDelay <
                        universe.totaltimeelapsed - entry->startTime)
                {
                    entry->phase = 2;
                    ship->cataBurstFiring = FALSE;
                    entry->startTime = universe.totaltimeelapsed;
                    soundEventShipRemove(ship);
                }
            }
        }
    }

    if (ship->upgradeActive &&
        universe.totaltimeelapsed > ship->upgradeEndTime)
    {
        ship->upgradeActive = FALSE;
        clSetAttribute((sdword)ship, 0, 0);
        shipApplyPlayerUpgrades(ship);
        speechEventQueue(0, 33556629, -1, -1, 0,
                         ship->playerowner->playerIndex, -1, 5.0f, -1);
    }

    if (univForceShipUpgradeRefresh)
    {
        shipApplyPlayerUpgrades(ship);
    }
    seekdir();
    shipUpdateFreezeState(ship);

    if ((ship->staticinfo->shipFlags & 0x20000) != 0 &&
        ((universe.univUpdateCounter & cataSlotStatusMask) ==
         (unsigned short)(cataSlotStatusMask & ship->shipID.shipNumber)))
    {
        shipUpdateSlotStatusesForAllPlayers(ship);
    }

    if ((ship->flags & SOF_Dead) == 0)
    {
        universe.aiplayerProcessing = 1;
        univUpdateShipAITick(ship);
        universe.aiplayerProcessing = 0;
    }

}

void univShipHouseKeep(Ship *ship)
{
    univShipHouseKeepImpl(ship);
}

static bool32 univIsCrystalTarget(Ship *ship)
{
    return ship != NULL && (ship->shiptype == dCrystal0 || ship->shiptype == dCrystal1);
}

bool32 clIsAttackTargetEligible(Player *player, Ship *targetShip)
{
    bool32 slotVisible;
    ShipType shipType;

    slotVisible = TRUE;
    if (player != NULL)
    {
        slotVisible = shipGetSlotStatus(targetShip, player->playerIndex) != 0;
    }

    if (univIsCrystalTarget(targetShip))
    {
        return FALSE;
    }
    if (bitTest(targetShip->staticinfo->shipFlags, SSTATF_BeastShip))
    {
        return FALSE;
    }
    if (targetShip->specialTargetShip != NULL)
    {
        return FALSE;
    }
    if (targetShip->statusWindow.activeUntil != 0.0f)
    {
        return FALSE;
    }
    if (bitTest(targetShip->specialFlags2, 0x02000000))
    {
        return FALSE;
    }
    if (targetShip->objtype != OBJ_ShipType)
    {
        return FALSE;
    }

    shipType = targetShip->shiptype;
    if (shipType == dBigGun ||
        shipType == dBigGunUnit ||
        shipType == dCrate ||
        shipType == dJunk1PartA ||
        shipType == dJunk1Shell ||
        shipType == dJunk1Strut ||
        shipType == dJunk2Shipwreck ||
        shipType == dJunk3Boiler ||
        shipType == dJunk3BoilerCasing ||
        shipType == dMessagePod ||
        shipType == dBuoy ||
        shipType == dEcclesiamInfected ||
        targetShip->posinfo.position.x < -smUniverseSizeX ||
        targetShip->posinfo.position.x > smUniverseSizeX ||
        targetShip->posinfo.position.y < -smUniverseSizeY ||
        targetShip->posinfo.position.y > smUniverseSizeY ||
        targetShip->posinfo.position.z < -smUniverseSizeZ ||
        targetShip->posinfo.position.z > smUniverseSizeZ)
    {
        return FALSE;
    }

    return slotVisible;
}

SelectAnyCommand *univFindNearbyTargets(Ship *ship, real32 range)
{
    static SelectAnyCommandMax6 nearbyTargets;
    SelectCommand *blobShips;
    Player *player;
    real32 negativeRange;
    real32 timeSinceDecloak;
    sdword count;
    udword index;

    if (ship == NULL || ship->collMyBlob == NULL || ship->playerowner == NULL)
    {
        return NULL;
    }
    if (ship->shiptype == trFloatingCity)
    {
        return NULL;
    }
    if ((UPDATE_AI_RUNTIME_FLAGS(ship) & 0x40000u) != 0)
    {
        return NULL;
    }

    blobShips = ship->collMyBlob->blobShips;
    if (blobShips == NULL || blobShips->numShips <= 0)
    {
        return NULL;
    }

    player = ship->playerowner;
    negativeRange = -range;
    count = 0;
    nearbyTargets.numTargets = 0;
    nearbyTargets.timeLastStatus = 0.0f;

    for (index = 0; index < (udword)blobShips->numShips && count < 6; index++)
    {
        Ship *targetShip;
        real32 deltaX;
        real32 deltaY;
        real32 deltaZ;

        targetShip = blobShips->ShipPtr[index];
        if (targetShip == NULL)
        {
            continue;
        }
        if (!clIsAttackTargetEligible(player, targetShip))
        {
            continue;
        }
        if (univIsObjFriendly((SpaceObj *)targetShip, player))
        {
            continue;
        }
        if ((UPDATE_AI_RUNTIME_FLAGS(targetShip) & 0x240000u) != 0)
        {
            continue;
        }
        if (!shipGetSlotStatus(targetShip, player->playerIndex))
        {
            continue;
        }

        timeSinceDecloak = universe.totaltimeelapsed - targetShip->shipDeCloakTime;
        if (timeSinceDecloak <= TW_SECOND_SEE_TIME)
        {
            if (timeSinceDecloak < TW_NOONE_SEE_TIME)
            {
                continue;
            }
            else if (timeSinceDecloak < TW_FIRST_SEE_TIME)
            {
                if ((ranRandom(10) % 101) > TW_FIRST_SEE_PERCENTAGE)
                {
                    continue;
                }
            }
            else if (timeSinceDecloak < TW_SECOND_SEE_TIME)
            {
                if ((ranRandom(10) % 101) > TW_SECOND_SEE_PERCENTAGE)
                {
                    continue;
                }
            }
        }
        if (bitTest(targetShip->flags, SOF_DockMarker | SOF_Slaveable))
        {
            continue;
        }
        if (targetShip->shiptype == sMothership || targetShip->shiptype == bMothership)
        {
            continue;
        }

        deltaX = targetShip->posinfo.position.x - ship->posinfo.position.x;
        deltaY = targetShip->posinfo.position.y - ship->posinfo.position.y;
        deltaZ = targetShip->posinfo.position.z - ship->posinfo.position.z;
        if (deltaX <= negativeRange || deltaX >= range ||
            deltaY <= negativeRange || deltaY >= range ||
            deltaZ <= negativeRange || deltaZ >= range)
        {
            continue;
        }
        nearbyTargets.TargetPtr[count++] = (TargetPtr)targetShip;
    }

    if (count == 0)
    {
        return NULL;
    }

    nearbyTargets.numTargets = count;
    return (SelectAnyCommand *)&nearbyTargets;
}
/*=== univScaleCollisionInfo (0x545390) ===*/
/*
void __cdecl univScaleCollisionInfo(int a1)
{
  int v2; // ebx
  bool v3; // zf
  void *v4; // eax
  int v5; // edx
  int v6; // esi
  __int64 v7; // rax
  double v8; // st7
  float v9; // [esp+18h] [ebp+4h]

  v2 = 200 * *(_DWORD *)(a1 + 408) + 12195712;
  v3 = (*(_DWORD *)(a1 + 20) & 0x40000) == 0;
  v9 = *(float *)(a1 + 412);
  if ( v3 )
  {
    v4 = memAllocFunction(200, MBF_NonVolatile);
    v5 = 25 * *(_DWORD *)(a1 + 408);
    *(_DWORD *)(a1 + 32) = v4;
    qmemcpy(v4, (const void *)(8 * v5 + 12195712), 0xC8u);
    *(_DWORD *)(a1 + 20) |= 0x40000u;
  }
  v6 = *(_DWORD *)(a1 + 32);
  v7 = (__int64)((double)*(unsigned int *)(v2 + 184) * v9);
  *(_DWORD *)(v6 + 184) = v7;
  if ( !(_DWORD)v7 )
    *(_DWORD *)(v6 + 184) = 1;
  *(float *)(v6 + 8) = v9 * *(float *)(v2 + 8);
  *(float *)(v6 + 12) = v9 * *(float *)(v2 + 12);
  *(float *)(v6 + 16) = v9 * *(float *)(v2 + 16);
  *(float *)(v6 + 20) = v9 * *(float *)(v2 + 20);
  *(float *)(v6 + 24) = ONE / *(float *)(v6 + 8);
  *(float *)(v6 + 28) = ONE / *(float *)(v6 + 12);
  *(float *)(v6 + 32) = ONE / *(float *)(v6 + 16);
  *(float *)(v6 + 36) = ONE / *(float *)(v6 + 20);
  *(_DWORD *)(v6 + 72) = *(_DWORD *)(v2 + 72);
  *(float *)(v6 + 52) = v9 * *(float *)(v2 + 52);
  *(float *)(v6 + 56) = v9 * *(float *)(v2 + 56);
  *(float *)(v6 + 60) = v9 * *(float *)(v2 + 60);
  v8 = v9 * *(float *)(v2 + 76);
  *(float *)(v6 + 76) = v8;
  *(float *)(v6 + 80) = v8 * v8;
  *(float *)(v6 + 84) = v9 * *(float *)(v2 + 84);
  *(float *)(v6 + 88) = v9 * *(float *)(v2 + 88);
  *(float *)(v6 + 92) = v9 * *(float *)(v2 + 92);
  *(float *)(v6 + 96) = v9 * *(float *)(v2 + 96);
  *(float *)(v6 + 100) = v9 * *(float *)(v2 + 100);
  *(float *)(v6 + 104) = v9 * *(float *)(v2 + 104);
  *(float *)(v6 + 128) = v9 * *(float *)(v2 + 128);
  *(float *)(v6 + 124) = v9 * *(float *)(v2 + 124);
  *(float *)(v6 + 120) = v9 * *(float *)(v2 + 120);
  *(float *)(v6 + 132) = v9 * *(float *)(v2 + 132);
  collUpdateCollRectangle(a1);
}
*/
// LOD table of ship-ship bump-collision ETG effects (retail dword_C412C4). Retail
// has zero write-xrefs to this global anywhere in the binary, so it is always NULL
// at runtime and the effect-spawn branch below never actually fires; kept for exact
// structural parity with univHandleShipCollision (0x545540).
static etglod *shipCollisionEffectTable = NULL;

/*-----------------------------------------------------------------------------
    Name        : univHandleShipCollision
    Description : Cataclysm-specific ship-ship bump-collision handler. Called by
                  collCheckShipShipColl/collCheckBigShipSmallShipColl in place of
                  HW1's ApplyDamageToCollidingObjects. When the two ships'
                  ATTRIBUTES_HeadShotVelToMothership attributesParam bits differ,
                  applies symmetric min-health collision damage to both ships and
                  spawns a collision ETG effect keyed off whichever ship had the
                  bit set. Otherwise, just wakes each ship's AI command.
    Inputs      : ship, target - the two colliding ships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void univHandleShipCollision(SpaceObjRotImpTarg *ship, SpaceObjRotImpTarg *target)
{
    if (((ship->attributesParam ^ target->attributesParam) & ATTRIBUTES_HeadShotVelToMothership) != 0)
    {
        real32 damagetaken = (ship->health <= target->health) ? ship->health : target->health;
        SpaceObjRotImpTarg *effectShip;
        etgeffectstatic *stat;

        ApplyDamageToTarget(ship, damagetaken, 0, DEATH_Killed_By_Collision, 99);
        ApplyDamageToTarget(target, damagetaken, 0, DEATH_Killed_By_Collision, 99);

        effectShip = ((ship->attributesParam & ATTRIBUTES_HeadShotVelToMothership) != 0) ? ship : target;

        stat = NULL;
        if (shipCollisionEffectTable != NULL && effectShip->currentLOD < shipCollisionEffectTable->nLevels)
        {
            stat = shipCollisionEffectTable->level[effectShip->currentLOD];
        }

        if (univSpaceObjInRenderList((SpaceObj *)effectShip) && stat != NULL && !etgFrequencyExceeded(stat))
        {
            vector heading;
            matrix coordsys;
            vector position;

            heading.x = -effectShip->posinfo.velocity.x;
            heading.y = -effectShip->posinfo.velocity.y;
            heading.z = -effectShip->posinfo.velocity.z;
            vecNormalize(&heading);
            matCreateCoordSysFromHeading(&coordsys, &heading);

            vecAdd(position, effectShip->posinfo.position, effectShip->posinfo.velocity);

            etgEffectCreate(stat, NULL, &position, &effectShip->posinfo.velocity, &coordsys, 0.0f, 0, 2);
        }
    }
    else
    {
        if (ship->objtype == OBJ_ShipType && ((Ship *)ship)->shiptype != trFloatingCity)
        {
            getShipAndItsCommand(&universe.mainCommandLayer, (Ship *)ship);
        }
        if (target->objtype == OBJ_ShipType && ((Ship *)target)->shiptype != trFloatingCity)
        {
            getShipAndItsCommand(&universe.mainCommandLayer, (Ship *)target);
        }
    }
}
/*=== univApplyCollisionImpulse (0x5456d0) ===*/
/*
char __cdecl univApplyCollisionImpulse(int a1, int a2, float a3, float *a4, float a5)
{
  double v5; // st7
  int v6; // esi
  int v7; // edi
  int v8; // eax
  double v9; // st6
  double v10; // st5
  int v11; // eax
  int v12; // ecx
  double v13; // st5
  double v14; // st6
  double v15; // st5
  char result; // al
  int v17; // esi
  float v18; // [esp+10h] [ebp-24h]
  float v19; // [esp+14h] [ebp-20h]
  float v20; // [esp+14h] [ebp-20h]
  float v21; // [esp+18h] [ebp-1Ch]
  float v22; // [esp+18h] [ebp-1Ch]
  int v23; // [esp+1Ch] [ebp-18h] BYREF
  float v24; // [esp+20h] [ebp-14h]
  float v25; // [esp+24h] [ebp-10h]
  float v26; // [esp+2Ch] [ebp-8h]
  float v27; // [esp+30h] [ebp-4h]
  float v28; // [esp+40h] [ebp+Ch]
  float v29; // [esp+48h] [ebp+14h]
  float v30; // [esp+48h] [ebp+14h]
  float v31; // [esp+48h] [ebp+14h]
  float v32; // [esp+48h] [ebp+14h]

  v29 = (a3 - a5) * collisionImpulseRecentScale / a5;
  if ( universe.totaltimeelapsed - *(float *)(a1 + 376) < collisionImpulseRecentWindow
    && universe.totaltimeelapsed - *(float *)(a2 + 376) < collisionImpulseRecentWindow )
  {
    v17 = *(_DWORD *)(a2 + 32);
    *(float *)&v23 = v29 * *a4;
    v24 = v29 * a4[1];
    result = 16;
    v25 = v29 * a4[2];
    if ( !*(_BYTE *)(v17 + 49) && (*(_BYTE *)(a2 + 56) & 0x10) == 0 )
    {
      *(float *)(a2 + 112) = *(float *)&v23 + *(float *)(a2 + 112);
      *(float *)(a2 + 116) = v24 + *(float *)(a2 + 116);
      *(float *)(a2 + 120) = v25 + *(float *)(a2 + 120);
    }
    if ( !*(_BYTE *)(*(_DWORD *)(a1 + 32) + 49) && (*(_BYTE *)(a1 + 56) & 0x10) == 0 )
    {
      *(float *)(a1 + 112) = *(float *)(a1 + 112) - *(float *)&v23;
      *(float *)(a1 + 116) = *(float *)(a1 + 116) - v24;
      *(float *)(a1 + 120) = *(float *)(a1 + 120) - v25;
    }
  }
  else
  {
    *(real32 *)(a1 + 376) = universe.totaltimeelapsed;
    *(real32 *)(a2 + 376) = universe.totaltimeelapsed;
    v5 = *(float *)(a1 + 100) * *(float *)(a1 + 100)
       + *(float *)(a1 + 104) * *(float *)(a1 + 104)
       + *(float *)(a1 + 108) * *(float *)(a1 + 108);
    v30 = *(float *)(a2 + 100) * *(float *)(a2 + 100)
        + *(float *)(a2 + 104) * *(float *)(a2 + 104)
        + *(float *)(a2 + 108) * *(float *)(a2 + 108);
    if ( v5 <= v30 )
    {
      v6 = a2;
      v28 = *(float *)(a2 + 100) * *(float *)(a2 + 100)
          + *(float *)(a2 + 104) * *(float *)(a2 + 104)
          + *(float *)(a2 + 108) * *(float *)(a2 + 108);
      v7 = a1;
    }
    else
    {
      v28 = v5;
      v6 = a1;
      v5 = v30;
      v7 = a2;
    }
    v8 = *(_DWORD *)(a2 + 32);
    v9 = *(float *)(v8 + 8) * *(float *)(a2 + 100);
    v24 = *(float *)(v8 + 8) * *(float *)(a2 + 104);
    v10 = *(float *)(v8 + 8);
    v11 = *(_DWORD *)(a1 + 32);
    v12 = *(_DWORD *)(v6 + 16);
    v25 = v10 * *(float *)(a2 + 108);
    v13 = *(float *)(v11 + 8) * *(float *)(a1 + 100);
    v26 = *(float *)(v11 + 8) * *(float *)(a1 + 104);
    v27 = *(float *)(v11 + 8) * *(float *)(a1 + 108);
    v14 = v9 - v13;
    v19 = v24 - v26;
    v21 = v25 - v27;
    v15 = ONE / universe.unknown1;
    v31 = v15;
    v18 = v15 * v14;
    v20 = v31 * v19;
    v22 = v31 * v21;
    if ( (v12 || (*(_BYTE *)(v6 + 456) & 2) == 0)
      && (*(_DWORD *)(v7 + 16) || (*(_BYTE *)(v7 + 456) & 2) == 0)
      && (v28 / v5 >= collisionImpulseSpeedRatioThreshold
       || *(float *)(v6 + 108) * *(float *)(v7 + 108)
        + *(float *)(v6 + 104) * *(float *)(v7 + 104)
        + *(float *)(v6 + 100) * *(float *)(v7 + 100) < ZERO) )
    {
      if ( v18 == ZERO && v20 == ZERO )
        v18 = 0.1;
      v32 = fmathSqrtDouble(v22 * v22 + v20 * v20 + v18 * v18);
      *(float *)&v23 = v20 - ZERO;
      v25 = 0.0;
      v24 = ZERO - v18;
      vecNormalizeToLength((float *)&v23, v32);
      if ( (*(float *)(v7 + 224) - *(float *)(v6 + 224)) * *(float *)(v6 + 104)
         - (*(float *)(v7 + 228) - *(float *)(v6 + 228)) * *(float *)(v6 + 100) <= ZERO )
      {
        v18 = *(float *)&v23;
        v20 = v24;
        v22 = v25;
      }
      else
      {
        v18 = -*(float *)&v23;
        v20 = -v24;
        v22 = -v25;
      }
    }
    result = 16;
    if ( !*(_BYTE *)(*(_DWORD *)(v6 + 32) + 49) && (*(_BYTE *)(v6 + 56) & 0x10) == 0 )
    {
      *(float *)(v6 + 112) = v18 + *(float *)(v6 + 112);
      *(float *)(v6 + 116) = v20 + *(float *)(v6 + 116);
      *(float *)(v6 + 120) = v22 + *(float *)(v6 + 120);
    }
    if ( !*(_BYTE *)(*(_DWORD *)(v7 + 32) + 49) && (*(_BYTE *)(v7 + 56) & 0x10) == 0 )
    {
      *(float *)(v7 + 112) = *(float *)(v7 + 112) - v18;
      *(float *)(v7 + 116) = *(float *)(v7 + 116) - v20;
      *(float *)(v7 + 120) = *(float *)(v7 + 120) - v22;
    }
  }
  return result;
}
*/
void cat_unitCapDeleteShip_(Ship *ship, Player *player)
{
   if (cdEnabled && (ship->specialFlags1C8 & CATA_RUNTIME_IN_UNIT_CAPS))
   {
       player->totalships--;
       if (player->totalships < 0)
       {
           player->totalships = 0;
           dbgMessagef("\nWarning: total shipcount in Cataclysm unit caps went below 0");
       }

       player->shiptotals[ship->shiptype]--;
       if (player->shiptotals[ship->shiptype] < 0)
       {
           player->shiptotals[ship->shiptype] = 0;
           dbgMessagef("\nWarning: Cataclysm shipcount of shiptype %d went below 0", ship->shiptype);
       }

       player->classtotals[ship->staticinfo->shipclass]--;
       if (player->classtotals[ship->staticinfo->shipclass] < 0)
       {
           player->classtotals[ship->staticinfo->shipclass] = 0;
           dbgMessagef("\nWarning: Cataclysm classtotals of class %d went below 0", ship->staticinfo->shipclass);
       }

       ship->specialFlags1C8 &= ~CATA_RUNTIME_IN_UNIT_CAPS;
   }
}
















// Returns the clamped-child ship types for the Sect mothership and the count.
sdword univGetSectMothershipClampedChildren(ShipType *shipTypes)
{
    const char *mapName;

    if (tutorial == 1)
    {
        shipTypes[0] = sMothershipEnginesLower; shipTypes[1] = sMothershipEnginesUpper; shipTypes[2] = sMothershipCommand;
        shipTypes[3] = sMothershipLowerSpine; shipTypes[4] = sMothershipOreCanArmour; shipTypes[5] = sMothershipEngineering;
        shipTypes[6] = sMothershipDockingBay;
        return 7;
    }
    if (singlePlayerGame == 1)
    {
        if (singlePlayerGameInfo.currentMission == 1 || singlePlayerGameInfo.currentMission == 2)
        {
            shipTypes[0] = sMothershipEnginesLower; shipTypes[1] = sMothershipEnginesUpper; shipTypes[2] = sMothershipCommand;
            shipTypes[3] = sMothershipLowerSpine; shipTypes[4] = sMothershipOreCanArmour; shipTypes[5] = sMothershipEngineering;
            return 6;
        }
        if (singlePlayerGameInfo.currentMission == 3)
        {
            shipTypes[0] = sMothershipEnginesLower; shipTypes[1] = sMothershipEnginesUpper; shipTypes[2] = sMothershipCommand;
            shipTypes[3] = sMothershipLowerSpine; shipTypes[4] = sMothershipEngineering;
            return 5;
        }
    }
    else if (spScenarios != NULL)
    {
        mapName = spScenarios[spCurrentSelected].fileSpec;
        if (mapName != NULL && !_strcmpi(mapName, "demobiggun2"))
        {
            shipTypes[0] = sMothershipEnginesLower; shipTypes[1] = sMothershipEnginesUpper; shipTypes[2] = sMothershipCommand;
            shipTypes[3] = sMothershipEngineering; shipTypes[4] = sMothershipSpecial; shipTypes[5] = sMothershipBigGun;
            shipTypes[6] = sMothershipLowerSpine;
            return 7;
        }
        if (mapName != NULL && _strcmpi(mapName, "demosect2") == 0)
        {
            shipTypes[0] = sMothershipEnginesLower; shipTypes[1] = sMothershipEnginesUpper; shipTypes[2] = sMothershipCommand;
            shipTypes[3] = sMothershipEngineering; shipTypes[4] = sMothershipSpecial; shipTypes[5] = sMothershipBigGun;
            shipTypes[6] = sMothershipLowerSpine; shipTypes[7] = sMothershipArmour; shipTypes[8] = sMothershipMicro;
            shipTypes[9] = sMothershipWeapons; shipTypes[10] = sMothershipDockingBay;
            return 12;
        }
    }
    shipTypes[0] = sMothershipEnginesUpper; shipTypes[1] = sMothershipCommand; shipTypes[2] = sMothershipEngineering;
    return 3;
}

// Returns the clamped-child ship types for the Beast mothership and the count.
static sdword univGetBeastMothershipClampedChildren(ShipType *shipTypes)
{
    if (singlePlayerGame == 1)
    {
        if (singlePlayerGameInfo.currentMission == 1 || singlePlayerGameInfo.currentMission == 2 || singlePlayerGameInfo.currentMission == 3)
        {
            return 0;
        }
    }
    shipTypes[0] = bMothershipDockingBay; shipTypes[1] = bMothershipEnginesLower;
    return 2;
}

static const ShipType univSectMothershipMultiplayerExtraChildren[] =
{
    sMothershipSpecial,
    sMothershipEngineering,
    sMothershipArmour,
    sMothershipMicro,
    sMothershipWeapons,
    sMothershipDockingBay,
    sMothershipBigGun
};

static void univCreateClampedChildren(Ship *ship, ShipRace shiprace, vector *shippos, Player *playerowner, sdword built)
{
    ShipType childShipTypes[21];
    sdword count;
    sdword i;
    Ship *child;

    if (ship->shiptype == sMothership)
    {
        count = univGetSectMothershipClampedChildren(childShipTypes);
    }
    else if (ship->shiptype == bMothership)
    {
        count = univGetBeastMothershipClampedChildren(childShipTypes);
    }
    else
    {
        return;
    }

    for (i = 0; i < count; i++)
    {
        child = univCreateShip(childShipTypes[i], shiprace, shippos, playerowner, built);
        if (shiprace == RACE_Sect)
        {
            ModuleCompleted(playerowner, child->shiptype, FALSE);
        }
        child->posinfo.position = ship->posinfo.position;
        child->posinfo.velocity = ship->posinfo.velocity;
        clampObjectSetup((SpaceObj *)child, (SpaceObj *)ship);
        child->flags |= SOF_ForwardDamageToClampHost;
    }

    if (!singlePlayerGame &&
        !bitTest(tpGameCreated.flag, MG_ResearchEnabled) &&
        ship->shiptype == sMothership)
    {
        for (i = 0; i < sizeof(univSectMothershipMultiplayerExtraChildren) / sizeof(univSectMothershipMultiplayerExtraChildren[0]); i++)
        {
            child = univCreateShip(
                univSectMothershipMultiplayerExtraChildren[i],
                shiprace,
                shippos,
                playerowner,
                built);
            cat_unitCapCreateShip(child, child->playerowner);
            child->posinfo.position = ship->posinfo.position;
            child->posinfo.velocity = ship->posinfo.velocity;
            clampObjectSetup((SpaceObj *)child, (SpaceObj *)ship);
        }
    }

}

// Cataclysm-specific unit cap tracking. Retail uses Ship+0x1C8 BIT29 and
// accounts for mothership clamped children.
void cat_unitCapCreateShip(Ship *ship, Player *player)
{
    ShipType childShipTypes[21];
    sdword i, count;

    if (cdEnabled && !(ship->specialFlags1C8 & CATA_RUNTIME_IN_UNIT_CAPS))
    {
        player->totalships++;
        player->shiptotals[ship->shiptype]++;
        player->classtotals[ship->staticinfo->shipclass]++;
        ship->specialFlags1C8 |= CATA_RUNTIME_IN_UNIT_CAPS;

        if (ship->shiptype == sMothership)
        {
            count = univGetSectMothershipClampedChildren(childShipTypes);
            for (i = 0; i < count; i++)
            {
                player->totalships++;
                player->shiptotals[childShipTypes[i]]++;
                player->classtotals[CLASS_Resource]++;
            }
        }
        else if (ship->shiptype == bMothership)
        {
            count = univGetBeastMothershipClampedChildren(childShipTypes);
            for (i = 0; i < count; i++)
            {
                player->totalships++;
                player->shiptotals[childShipTypes[i]]++;
                player->classtotals[CLASS_Resource]++;
            }
        }
    }
}


sdword univGetShipTypeCount(sdword shipType, sdword playerIndex)
{
    sdword result = 0;
    sdword i;

    if (shipType != -1)
    {
        return cataShipTypeCounts[(TOTAL_NUM_SHIPS * playerIndex) + shipType];
    }

    for (i = 0; i < TOTAL_NUM_SHIPS; ++i)
    {
        result += cataShipTypeCounts[(TOTAL_NUM_SHIPS * playerIndex) + i];
    }
    return result;
}

sdword univGetReplacementShipTypeCount(sdword shipType, sdword playerIndex)
{
    sdword result = 0;
    sdword i;

    if (shipType != -1)
    {
        return cataReplacementShipTypeCounts[(TOTAL_NUM_SHIPS * playerIndex) + shipType];
    }

    for (i = 0; i < TOTAL_NUM_SHIPS; ++i)
    {
        result += cataReplacementShipTypeCounts[(TOTAL_NUM_SHIPS * playerIndex) + i];
    }
    return result;
}

sdword univIncrementShipTypeCount(sdword shipType, sdword playerIndex)
{
    return ++universe.players[playerIndex].shiptotals[shipType];
}



sdword univDetachShipsInsideFromCommands(Ship *ship)
{
    ClampInfo *clampInfo = ship->clampInfo;

    if (clampInfo != NULL && clampInfo->selection != NULL)
    {
        do
        {
            clDetachShipFromCommand(((SelectCommand *)clampInfo->selection)->ShipPtr[0]);
            clampInfo = ship->clampInfo;
        }
        while (clampInfo != NULL);
    }

    return (sdword)clampInfo;
}

void univUpdateAttachedObjRotInfo(SpaceObjRot *attachedObjectRot)
{
    SpaceObjRotImpTargGuidanceShipDerelict *attachedObject = (SpaceObjRotImpTargGuidanceShipDerelict *)attachedObjectRot;
    ClampInfo *clampInfo = attachedObject->clampInfo;
    SpaceObjRotImpTargGuidanceShipDerelict *host;
    vector rotatedOffset;

    dbgAssert(clampInfo != NULL);
    dbgAssert(clampInfo->host != NULL);

    host = (SpaceObjRotImpTargGuidanceShipDerelict *)clampInfo->host;

    matMultiplyMatByVec(&rotatedOffset, &host->rotinfo.coordsys, &clampInfo->clampOffset);
    vecAdd(attachedObjectRot->posinfo.position, host->posinfo.position, rotatedOffset);
    matMultiplyMatByMat(&attachedObjectRot->rotinfo.coordsys, &host->rotinfo.coordsys, &clampInfo->clampCoordsys);
    attachedObjectRot->posinfo.velocity = host->posinfo.velocity;
    univUpdateObjRotInfo(attachedObjectRot);
}
