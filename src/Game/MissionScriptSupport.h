#ifndef ___MISSION_SCRIPT_SUPPORT_H
#define ___MISSION_SCRIPT_SUPPORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "bink.h"
#include "types.h"
#include "vector.h"
#include "linkedlist.h"
#include "objtypes.h"
#include "spaceobj.h"
#include "ACV.h"
#include "statscript.h"
#include "font.h"
#include "feflow.h"
#include "universe.h"
#include "univupdate.h"
#include "physics.h"
#include "singleplayer.h"
#include "DDDFrigate.h"
#include "taskbar.h"
#include "utility.h"
#include "mainrgn.h"
#include "aitrack.h"
#include "aiship.h"
#include "levelload.h"
#include "sensors.h"
#include "nis.h"
#include "mouse.h"
#include "select.h"
#include "soundevent.h"
#include "collision.h"
#include "ResearchShip.h"
#include "InfoOverlay.h"
#include "Ship.h"
#include "AIPlayer.h"
#include "KAS.h"
#include "KASHashes.h"
#include "hs.h"
#include "AIVar.h"
#include "Objectives.h"
#include "ResearchGUI.h"
#include "launchMgr.h"
#include "trademgr.h"
#include "file.h"
#include "etg.h"
#include "horserace.h"
#include "attributes.h"
#include "SaveGame.h"
#include "damage.h"
#include "TradeMgr.h"
#include "ping.h"
#include "Tutor.h"
#include "randy.h"
#include "KASFunc.h"
#include "AITeam.h"
#include "strings.h"
#include "animatic.h"
#include "gravwellgenerator.h"
#include "commandwrap.h"
#include "mothership.h"
#include "alliance.h"
#include "dock.h"

#define scope kasCurrentScopeHash
#define teamtoreinforce ((AITeam *)kasCurrentTeamPtr)
#define loc_4A2A29 (*(char *)0x004A2A29)
#define loc_4A2A39 (*(char *)0x004A2A39)
#define loc_47137B (*(char *)0x0047137B)
#define loc_4C489A (*(char *)0x004C489A)
#define unk_C8CC23 (*(char *)0x00C8CC23)
#define sub_58A4A2 ((char *)0x0058A4A2)
#define aAngleD "Angle:      %d"
#define aDeclinationD "Declination %d"
#define aDistanceD "Distance\t%d"
#define sub_490820 kasShipsVectorPtrIfExists
#define sub_4908C0 kasTeamsVectorPtr
#define sub_495410 kasfShipsSelectMembersOfSelection
#define sub_495640 kasfShipsSelectClass
#define sub_4956E0 kasfShipsSelectCapital
#define sub_495950 kasfShipsSelectIndex
#define sub_495A50 kasfShipsSelectNearby
#define sub_495EE0 kasfShipsStop
#define sub_4984C0 kasfShipsSetDontApplyPhysicsFlag
#define sub_498630 kasfFindShipsTargetingSelection
#define sub_49A100 kasfShipsSortByDistanceToPoint

void acolyteStartLinking(Ship *ship);
void shipApplyPlayerUpgrades(Ship *ship);
void mouseForceInvisible(void);
Ship *FindPlayersMothership(Player *player);
void spUnlockout(void);
void tutDeallocateRootRegion();
void cpHueSatImageDelete(void);
sdword kasfEnableShip(char *shipType, char *race, sdword playerIndex);
sdword GetGameDifficulty(void);
sdword SetGameDifficulty(sdword difficulty);
void spEventWideScreenIn(sdword frames);
void soundStopDSound(char *name, void *handle);
void sub_65F6F0(void);
void sub_617A10(void);
void Init_ReinforcementPool_ClearSelectionAndReset(void);
void ResetReinforcementBuildCount(void);
void ResetReinforcementHyperspaceAttackFlag(void);
void *Init_LaunchDockedTeam(void);
extern sdword spStateA5B1AC;
Ship *singlePlayerShipsCreate(GrowSelection *ships, char *shipTypeName, sdword count, sdword playerIndex);
void FindShipsOfShipTypeOfPlayer(GrowSelection *growselect, ShipType shiptype, Player *player);
extern bool32 hyperspaceAtEndOfSinglePlayerNIS;
sdword shipGetSlotStatus(Ship *ship, sdword slot);
void kasfTutSetPointerTargetShipWithLine(char *label, GrowSelection *ships);
void kasfTutSetPointerTargetAIVolumeNoLine(char *label, Volume *volume);
void kasfTutSetPointerTargetAIVolumeWithLine(char *label, Volume *volume);
void kasfFindBeastShipsInside(Volume *volume, GrowSelection *result);
void airAddedResearchShip(Ship *ship);

typedef struct AISTeamSelection
{
    MaxSelection sel;
    vector center;
} AISTeamSelection;

typedef struct AISTeamEntry
{
    AISTeamSelection selection[10];
    udword numselections;
    sdword totalteamsize;
} AISTeamEntry;

extern AISTeamEntry aisTeams[];

void Init_Mission10_CarrierTeam_Activate(void);
void Watch_Mission10_MissionFlow_NISing(void);
void Init_Mission10_ResearchShipTeam_Activate(void);
void Init_Mission10_GarrisonObjective_Start(void);
void Init_Mission10_CarrierObjective_Start(void);
void Init_Mission10_ControllerPocketObjective_Start(void);
void Init_ApplyPlayerUpgradesToCurrentTeam(void);
void Init_Mission10_ApplyUpgradesThenProtectResourcers(void);
void Init_Mission10_HarvestController_MoveToAsteroidPocket(void);
void Init_Mission10_HarvestController_UpdateResourceFlow(void);
void Init_Mission10_HarvestController_SelectShips(void);
void Init_ApplyUpgradesThenOpenMainScreen(void);
void Mission03_EnterCleeSanPrimaryResponseHoldingPattern(void);
void Mission03_ArmCleeSanPrimaryResponseHoldTimer(void);
void Mission03_SendCleeSanPrimaryResponseToApproachVector(void);

void Init_Mission02_SetupMissionFlowState(void);
void Watch_Mission02_PromptCleanup(void);
void Watch_Mission02_MissionFlowDelay(void);
void Watch_Mission02_MissionFlow(void);
void Init_Mission02_HonourGuardSwarmPatrol(void);
void Watch_Mission02_HonourGuardSwarmPatrol(void);
void Init_Mission02_HonourGuardSwarmMoveToRandomPoint(void);
void Watch_Mission02_HonourGuardSwarmDock(void);
void Init_Mission02_HonourGuardSwarmCollectNearbyShips(void);
void Watch_Mission02_HonourGuardSwarmCollectNearbyShips(void);
void Watch_Mission02_ObjectiveEventQueue(void);
void Watch_Mission02_DispatchPirateWaves(void);
void Init_Mission02_PirateAssaultTeam(void);
void Watch_Mission02_PirateAssaultTeam(void);
void Init_Mission02_PirateMissileCorvetteAttackRun(void);
void Watch_Mission02_PirateMissileCorvetteAttackRun(void);
void Init_Mission02_PirateMissileCorvetteRetarget(void);
void Watch_Mission02_PirateMissileCorvetteRetarget(void);
void Watch_Mission02_PirateMissileCorvetteReinforcements(void);
void Init_Mission02_PirateCarrierLaunchCycle(void);
void Watch_Mission02_PirateCarrierLaunchCycle(void);
void Init_Mission02_PirateMissileCorvetteTeam(void);
void Watch_Mission02_PirateMissileCorvetteTeam(void);
void Init_Mission02_PirateStrikeTeamWave(void);
void Watch_Mission02_PirateStrikeTeamWave(void);
void Watch_Mission02_PirateStrikeTeamRedirect(void);
void Init_Mission02_PirateStrikeTeamAttackRun(void);
void Watch_Mission02_PirateStrikeTeamAttackRun(void);
void Init_Mission02_PirateStrikeTeamRetarget(void);
void Watch_Mission02_PirateStrikeTeamRetarget(void);
void Init_Mission02_PirateStrikeTeamReinforcementDelay(void);
void Watch_Mission02_PirateStrikeTeamReinforcements(void);
void Watch_Mission02_PirateStrikeTeamDockCycle(void);
void Watch_Mission02_PirateStrikeTeamPostLaunch(void);
void Init_Mission02_PirateHeavyStrikeTeam(void);
void Watch_Mission02_PirateHeavyStrikeTeam(void);
void Watch_Mission02_PirateHeavyStrikeTeamRedirect(void);
void Init_Mission02_PirateHeavyStrikeTeamAttackRun(void);
void Watch_Mission02_PirateHeavyStrikeTeamAttackRun(void);
void Init_Mission02_PirateHeavyStrikeTeamReinforcementDelay(void);
void Watch_Mission02_PirateHeavyStrikeTeamReinforcements(void);
void Watch_Mission02_PirateHeavyStrikeTeamDockCycle(void);
void Watch_Mission02_PirateHeavyStrikeTeamPostLaunch(void);
void Init_Mission02_ReassignSurvivingAttackTeam(void);
void Init_Mission02_PirateSalvageCorvetteTeam(void);
void Watch_Mission02_PirateSalvageCorvetteTeam(void);
void Watch_Mission02_PirateSalvageCorvetteIntro(void);
void Init_Mission02_PirateSalvageCorvetteDelay(void);
void Watch_Mission02_PirateSalvageCorvetteDelay(void);
void Init_Mission02_PirateSalvageCorvetteStateChoice(void);
void Init_Mission02_PirateSalvageCorvetteSpecialAttack(void);
void Watch_Mission02_PirateSalvageCorvetteSpecialAttack(void);
void Init_Mission02_PirateSalvageCorvetteRetarget(void);
void Watch_Mission02_PirateSalvageCorvetteRetarget(void);
void Init_Mission02_PirateCarrierStrikeReinforcementCycle(void);
void Watch_Mission02_PirateCarrierStrikeReinforcementCycle(void);
void Init_Mission02_PirateSalvageCorvetteCleanup(void);
void Init_Mission02_PirateReserveAttackTeam(void);
void Watch_Mission02_PirateReserveAttackTeam(void);
void Init_Mission02_PirateReserveAttackRun(void);
void Watch_Mission02_PirateReserveAttackRun(void);
void Init_Mission02_SalvagePodApproach(void);
void Init_Mission02_SalvagePodHideDelay(void);
void Watch_Mission02_SalvagePodHideDelay(void);
void Watch_Mission02_SalvagePodApproach(void);
void Init_Mission02_SalvagePodAmbushDelay(void);
void Watch_Mission02_SalvagePodAmbushDelay(void);
void Init_Mission02_DamagedDestroyer(void);
void Watch_Mission02_DamagedDestroyer(void);
void Watch_Mission02_DamagedDestroyerRepairStart(void);
void Watch_Mission02_DamagedDestroyerRepairProgress(void);
void Watch_Mission02_DamagedDestroyerAttackCue(void);
void Init_Mission02_DamagedDestroyerDespawnDelay(void);
void Watch_Mission02_DamagedDestroyerDespawnDelay(void);
void Init_Mission02_PirateAdvanceSupportFrigateTeam(void);
void Watch_Mission02_PirateAdvanceSupportFrigateTeam(void);
void Init_Mission02_PirateAdvanceSupportFrigateRepairOrder(void);
void Watch_Mission02_PirateAdvanceSupportFrigateRepairOrder(void);
void Init_Mission02_PirateAdvanceSupportFrigateDefenseLoop(void);
void Watch_Mission02_PirateAdvanceSupportFrigateDefenseLoop(void);
void Init_Mission02_PirateTeamReinforcementTeam(void);
void Watch_Mission02_PirateTeamReinforcementTeam(void);
void Watch_Mission02_PirateTeamReinforcements(void);
void Init_Mission02_PirateTeamAttackDelay(void);
void Watch_Mission02_PirateTeamAttackDelay(void);
void Init_Mission02_PirateTeamDefenseLoop(void);
void Watch_Mission02_PirateTeamDefenseLoop(void);
void Init_Mission02_PirateTeamReinforcementDelay(void);
void Watch_Mission02_PirateTeamReinforcementDelay(void);
void Init_Mission02_PirateTeamReinforcementGate(void);
void Init_Mission02_AttackMothershipTeam(void);
void Watch_Mission02_AttackMothershipTeam(void);
void Mission02_SetAttackApproachVector(void);
void Mission02_SendTeamToAttackTarget(void);
void Watch_Mission02_TeamState_5E03D0(void);
void Init_Mission02_TeamState_5E0440(void);
void Watch_Mission02_TeamState_5E0540(void);
sdword Watch_Mission02_PirateAssaultTeamFollowup(void);
void Init_Mission02_PirateAssaultTeamDespawnDelay(void);
sdword Watch_Mission02_PirateAssaultTeamDespawnDelay(void);
sdword Watch_Mission02_RequestPirateStrikeGroups(void);
void Init_Mission02_PirateStrikeTeamDockCycle(void);
void Init_Mission02_FleetIntel_SetupState(void);
void Watch_Mission02_FleetIntel_CleanupAndResume(void);
void Watch_Mission02_FleetIntel_ResumeDelay(void);
void Watch_Mission02_FleetIntel_EventFlow(void);
void Init_Mission02_FleetIntel_Objectives(void);
void Init_Mission02_FleetIntel_FIDefendSalvageTeam2(void);
void Watch_Mission02_FleetIntel_FIDefendSalvageTeam2(void);
void Watch_Mission02_FleetIntel_FIInvestigateP1Mothership(void);
void Watch_Mission02_FleetIntel_FISeenP1Mothership(void);
void Init_Mission02_DefendSalvageTeam2Approach(void);
void Watch_Mission02_DefendSalvageTeam2ApproachSignal(void);
void Init_Mission02_DefendSalvageTeam2MarkTarget(void);
void Watch_Mission02_DefendSalvageTeam2Approach(void);
void Init_Mission02_DefendSalvageTeam2Intercept(void);
void Watch_Mission02_DefendSalvageTeam2Intercept(void);
void Init_Mission02_DefendSalvageTeam2ResumeCapture(void);
void Watch_Mission02_DefendSalvageTeam2ResumeCapture(void);
void Init_Mission02_ACT3Corvettes1_Launch(void);
void Begin_Mission02_ACT3Corvettes1_Launch(void);
void Watch_Mission01(void);
void Init_Mission01_Objectives(void);
void Watch_Mission01_Objectives(void);
void Init_Mission01_SetupTutorialState(void);
void Watch_Mission01_TutorialFlow(void);
void Watch_Mission01_TutorialPromptCleanup(void);
void Watch_Mission01_TutorialResumeDelay(void);
void Watch_Mission01_TutorialEventFlow(void);
void Init_Mission01_ActivateTutorialFlow(void);
int kasCheckShipDockFlag(int a1);
sdword kasShipSelectPredicate(Ship *ship, sdword arg1, sdword arg2);
sdword kasSimpleShipSelectPredicate(Ship *ship, sdword arg1, sdword arg2);
int kasSelectEnemyShips(int *result, int source);
int kasSelectPlayerShips(int *result, int source);
int kasSelectPlayerShipsSimple(int *result, int source);
void MaintainTeamContext(void);
void Init_FleetIntel_SelectInvestigationTarget(void);
void Watch_FleetIntel_InvestigateTarget(void);
void Watch_BeastWorkerRepair_SpecialAttackDamagedCapital(void);
sdword Mission02ShipSelectPredicate(Ship *ship, sdword arg1);
sdword Mission02ShipsSelect(GrowSelection *dest, GrowSelection *src);
sdword kasfShipsSelectMissionVolumeEnemyCombatShips(int *result, int source);
sdword kasfShipsSelectMissionVolumeInactivePlayerModules(int *result, int source);
void Init_QueueStandardFrigateBatch(void);
void Watch_StandardFrigateBatchComplete(void);
void Init_QueueMultigunCorvetteOrHeavyInterceptorBatch(void);
void Watch_MultigunCorvetteOrHeavyInterceptorBatchComplete(void);
void Init_SelectAndFocusProximitySensors(void);
void Watch_ProximitySensorContactLoop(void);
void Init_QueueHeavyInterceptorBatch(void);
void Watch_HeavyInterceptorBatchComplete(void);
void Init_ApplyUpgradesThenDockForLaunch(void);
void Init_DockCurrentTeamForLaunch(void);
void Init_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor(void);
void Watch_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady(void);
void Watch_AsteroidPocketEncounter_ResumeAttackers(void);
void Init_AsteroidPocketEncounter_FocusThreatWarning(void);
void Watch_AsteroidPocketEncounter_ThreatWarningAcknowledged(void);
sdword singlePlayerShipsAnyIdle(GrowSelection *ships);
void singlePlayerSortShipsByDistanceToPoint(GrowSelection *ships, hvector *point);
sdword singlePlayerShipIsMemberOfSelection(Ship *ship, sdword arg1, sdword arg2);
sdword singlePlayerShipIsMarkedInParade(Ship *ship, sdword arg1, sdword arg2);
sdword singlePlayerSelectShipsInParade(GrowSelection *result, GrowSelection *source);
void singlePlayerTeamSpecialToggle(void);
void sub_496050(void);
void sub_652EF0(void);
void sub_60C010(void);
void singlePlayerShipsSetHyperspaceDestinationToCurrentPosition(GrowSelection *ships);
void singlePlayerShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation);
sdword kasfFindShipsTargetingSelection(GrowSelection *targets, GrowSelection *ships);
sdword singlePlayerFindMissilesInside(Volume *vol, GrowSelection *ships);
void Init_AsteroidPocketEncounter_DefenseLoop(void);
void Watch_AsteroidPocketEncounter_DefenseLoop(void);
void Init_AsteroidPocketEncounter_ReinforcementDelay(void);
void Watch_AsteroidPocketEncounter_ReinforcementDelay(void);
void Watch_AsteroidPocketEncounter_ReinforcementReady(void);
void Watch_AsteroidPocketEncounter_BeginDefenseLoop(void);
void Init_AsteroidPocketEncounter_InitialAttackerWarning(void);
void Watch_AsteroidPocketEncounter_InitialAttackerWarningAcknowledged(void);
void Init_AsteroidPocketEncounter_SelectInitialAttackers(void);
void Init_AsteroidPocketEncounter_ClearCurrentSelectionHighlight(void);
void FailMissionIfTrackedSelectionAlive(void);
void Begin_BeastIonArrayFrigateCombatLoop(void);
void Init_Mission03_FleetIntel_SetupState(void);
void Watch_Mission03_FleetIntel_CleanupAndResume(void);
void Watch_Mission03_FleetIntel_ResumeDelay(void);
void Watch_Mission03_FleetIntel_EventFlow(void);
void Init_Mission03_FleetIntel_Objectives(void);
void Watch_Mission03_CleeSanPrimaryResponseApproach(void);
void Watch_Mission03_CleeSanPrimaryResponseHoldingPattern(void);
void Watch_Mission03_CleeSanPrimaryResponseHoldTimer(void);
void Watch_Mission03_FleetIntel_AdvanceAfterAcknowledge(void);
void Watch_Mission04_TuranicFighters_DefendYourSelf(void);
void Watch_Mission04_InfectedEcclesiaInParade(void);
void Watch_Mission04_EcclesiaAtGeneticsArrival(void);
void Init_WaitForIntelEventEnd(void);
void Init_Mission05_FleetIntel_FIDestroyResourcer(void);
void Watch_Mission05_FleetIntel_FIDestroyResourcer(void);
void Watch_Mission05_CorvetteReinforcementTeam(void);
void Watch_Mission05_CorvetteReinforcementDock(void);
void Init_Mission05_CorvetteReinforcementDelay(void);
void Watch_Mission05_CorvetteReinforcementBuild(void);
void Init_Mission05_CorvetteAssault(void);
void Watch_Mission05_CorvetteAssault(void);
void Init_Mission05_CorvetteCapitalStrike(void);
void Watch_Mission05_CorvetteCapitalStrike(void);
void Init_Mission05_DockForReinforcements(void);
void Watch_Mission05_CorvetteDockAndReinforce(void);
void Init_Mission05_FighterTeam(void);
void Watch_Mission05_FighterTeam(void);
void Watch_Mission05_FighterReinforcementTeam(void);
void Watch_Mission05_FighterReinforcementDock(void);
void Init_Mission05_FighterReinforcementDelay(void);
void Watch_Mission05_FighterReinforcementBuild(void);
void Init_Mission05_FighterAssault(void);
void Watch_Mission05_FighterAssault(void);
void Init_Mission05_FighterCapitalStrike(void);
void Watch_Mission05_FighterCapitalStrike(void);
void Watch_Mission05_FighterDockAndReinforce(void);
void Init_Mission05_RammingFrigateTeam(void);
void Watch_Mission05_RammingFrigateTeam(void);
void Watch_Mission05_RammingFrigateReinforcementTeam(void);
void Watch_Mission05_RammingFrigateReinforcementDock(void);
void Init_Mission05_RammingFrigateReinforcementDelay(void);
void Watch_Mission05_RammingFrigateReinforcementBuild(void);
void Init_Mission05_RammingFrigateAssault(void);
void Watch_Mission05_RammingFrigateAssault(void);
void Init_Mission05_BeastWaveActivation(void);
void Init_Mission05_BeastWaveResumeDelay(void);
void Watch_Mission05_BeastWaveResumeDelay(void);
void Watch_Mission05_BeastWaveCoordinator(void);
void Init_Mission05_MissionFlow(void);
void Watch_Mission05_MissionFlow(void);
void Watch_Mission05_FleetIntel_FIObjectivesComplete(void);
void Watch_Mission05_Interceptors_Launch(void);
void Init_Mission05_WorkerHarvestTeam(void);
void Watch_Mission05_WorkerHarvestTeam(void);
void Watch_Mission05_WorkerHarvestReinforcementTeam(void);
void Watch_Mission05_WorkerHarvestDockAndReinforce(void);
void Init_Mission05_WorkerHarvestReinforcementDelay(void);
void Watch_Mission05_WorkerHarvestReinforcementBuild(void);
void Init_Mission05_FrigateTeam(void);
void Watch_Mission05_FrigateTeam(void);
void Watch_Mission05_FrigateReinforcementTeam(void);
void Watch_Mission05_FrigateDockAndReinforce(void);
void Init_Mission05_FrigateReinforcementDelay(void);
void Watch_Mission05_FrigateReinforcementBuild(void);
void Init_Mission05_CorvetteTeam(void);
void Watch_Mission05_CorvetteTeam(void);
void Init_Mission05_FleetIntel_FIProtectBentusi(void);
void Watch_Mission05_FleetIntel_FIProtectBentusi(void);
void Init_Mission05_FleetIntel_FIProtectBentusiCleanup(void);
void Watch_Mission05_FleetIntel_FIProtectBentusiCleanup(void);
void Watch_Mission05_FrigateLaunchDockedReinforcements(void);
void Watch_Mission05_FrigateReinforcementCombatLoop(void);
void Begin_Mission05_FrigateCombatAttackRun(void);
void Watch_Mission05_FrigateCombatAttackRun(void);
void Watch_Mission05_WorkerHarvestResourceLoop(void);
void Init_Mission06_HeavyDefenderTeam(void);
void Watch_Mission06_HeavyDefenderTeam(void);
void Init_Mission06_HeavyDefenderBuildDelay(void);
void Watch_Mission06_HeavyDefenderBuildDelay(void);
void Init_Mission06_HeavyDefenderLaunch(void);
void Watch_Mission06_HeavyDefenderLaunch(void);
void Init_Mission06_HeavyDefenderAttackRun(void);
void Watch_Mission06_HeavyDefenderAttackRun(void);
void Init_Mission06_AttackBomberTeam(void);
void Watch_Mission06_AttackBomberTeam(void);
void Init_Mission06_AttackBomberBuildDelay(void);
void Watch_Mission06_AttackBomberBuildDelay(void);
void Init_Mission06_AttackBomberLaunch(void);
void Watch_Mission06_AttackBomberLaunch(void);
void Begin_Mission06_ReinforcementWingGamma_Assault(void);
void Watch_Mission06_ReinforcementWingGamma_Assault(void);
void Begin_Mission06_ReinforcementWingGamma_CombatLoop(void);
void Watch_Mission06_ReinforcementWingGamma_CombatLoop(void);
void Begin_Mission06_AttackBomberPatrol(void);
void Watch_Mission06_AttackBomberPatrol(void);
void Begin_Mission06_AttackBomberCombatLoop(void);
void Watch_Mission06_AttackBomberCombatLoop(void);
void Init_Mission06_RepairTeam(void);
void Watch_Mission06_RepairTeam(void);
void Begin_Mission06_RepairTeam_HomePosition(void);
void Watch_Mission06_RepairTeam_HomePosition(void);
void Begin_Mission06_RepairTeam_AssistTarget(void);
void Watch_Mission06_RepairTeam_AssistTarget(void);
void Begin_Mission06_ReinforcementWingGamma_RequestRepairSupport(void);
void Watch_Mission06_ReinforcementWingGamma_RequestRepairSupport(void);
void Init_Mission06_HeavyInterceptorWing(void);
void Init_Mission06_HeavyInterceptorWing_SpawnDelay(void);
void Init_Mission06_HeavyInterceptorWing_Spawn(void);
void Init_Mission06_HeavyInterceptorWing_DockAndReinforce(void);
void Watch_Mission06_HeavyInterceptorWing_DockAndReinforce(void);
void Init_Mission06_RepairSupportTargetWing(void);
void Watch_Mission06_RepairSupportTargetWing(void);
void Begin_Mission06_RepairSupportTargetWing_HomePosition(void);
void Watch_Mission06_RepairSupportTargetWing_HomePosition(void);
void Begin_Mission06_RepairSupportTargetWing_CombatLoop(void);
void Watch_Mission06_RepairSupportTargetWing_CombatLoop(void);
void Watch_Mission07_P2Mothership_Reappear(void);
void Init_Mission07_P2Mothership_Reappear(void);
void Watch_Mission08_HarvestTeam_WaitingForSignal(void);
void Init_Mission08_HarvestTeam_Harvest(void);
void Watch_Mission08_HarvestTeam_Harvest(void);
void Watch_Mission09_MissionFlow_SelectNearbyAttackersOrQueueStandardFrigateBatch(void);
void Watch_Mission09_MissionFlow_GhostIntro(void);
void Watch_Mission09_Traders_TalkingMore(void);
void Init_Mission09_AttackBomberWing(void);
void Watch_Mission09_AttackBomberWing(void);
void Watch_Mission09_AttackBomberWing_LaunchTrigger(void);
void Init_Mission09_AttackBomberWing_SpawnDelay(void);
void Watch_Mission09_AttackBomberWing_Spawn(void);
void Init_Mission09_AttackBomberWing_Regroup(void);
void Watch_Mission09_AttackBomberWing_Regroup(void);
void Init_Mission09_AttackBomberWing_ReengageDelay(void);
void Watch_Mission09_AttackBomberWing_ReengageDelay(void);
void Init_Mission09_AttackBomberWing_HuntNearbyTargets(void);
void Watch_Mission09_AttackBomberWing_HuntNearbyTargets(void);
void Init_Mission09_HeavyInterceptorWing(void);
void Watch_Mission09_HeavyInterceptorWing(void);
void Watch_Mission09_HeavyInterceptorWing_LaunchTrigger(void);
void Init_Mission09_HeavyInterceptorWing_SpawnDelay(void);
void Watch_Mission09_HeavyInterceptorWing_Spawn(void);
void Init_Mission09_HeavyInterceptorWing_Regroup(void);
void Watch_Mission09_HeavyInterceptorWing_Regroup(void);
void Init_Mission09_HeavyInterceptorWing_ReengageDelay(void);
void Watch_Mission09_HeavyInterceptorWing_ReengageDelay(void);
void Init_Mission09_HeavyInterceptorWing_HuntTargets(void);
void Watch_Mission09_HeavyInterceptorWing_HuntTargets(void);
void Init_Mission09_HeavyInterceptorWing_RequestSupportDelay(void);
void Init_Mission09_HeavyInterceptorWing_RequestSupport(void);
void Init_Mission09_CapitalAssaultWave(void);
void Watch_Mission09_CapitalAssaultWave(void);
void Watch_Mission09_CapitalAssaultWave_LaunchTrigger(void);
void Init_Mission09_CapitalAssaultWave_SpawnDelay(void);
void Watch_Mission09_CapitalAssaultWave_Spawn(void);
void Init_Mission09_SupportFrigateSquad_Regroup(void);
void Watch_Mission09_CapitalAssaultWave_Regroup(void);
void Init_Mission09_AssaultEscort_ReengageDelay(void);
void Init_Mission09_CapitalAssaultWave_HuntTargets(void);
void Init_Mission09_CapitalAssaultWave_HuntNearbyTargets(void);
void Watch_Mission09_CapitalAssaultWave_HuntNearbyTargets(void);
void Init_Mission09_GravwellSquad(void);
void Watch_Mission09_GravwellSquad(void);
void Watch_Mission09_GravwellSquad_LaunchTrigger(void);
void Init_Mission09_GravwellSquad_SpawnDelay(void);
void Watch_Mission09_GravwellSquad_Spawn(void);
void Watch_Mission09_GravwellSquad_Regroup(void);
void Watch_Mission09_GravwellSquad_Reengage(void);
void Init_Mission09_GravwellSquad_HuntDamagedCapital(void);
void Watch_Mission09_GravwellSquad_HuntDamagedCapital(void);
void Init_Mission09_SupportFrigateSquad(void);
void Watch_Mission09_SupportFrigateSquad(void);
void Watch_Mission09_SupportFrigateSquad_LaunchTrigger(void);
void Watch_Mission09_SupportFrigateSquad_Spawn(void);
void Watch_Mission09_SupportFrigateSquad_Regroup(void);
void Init_Mission09_FleetIntel_PauseWorld(void);
void Init_Mission09_SensorObjectiveTeam(void);
void Watch_Mission09_SensorObjectiveTeam(void);
void Watch_Mission09_BigGunEscortWing_SetWaypoint(void);
void Init_Mission09_BigGunEscortWing_ReacquireTargets(void);
void Watch_Mission09_BigGunEscortWing_ReacquireTargets(void);
void Init_Mission09_BigGunEscortWing_SpawnReinforcements(void);
void Watch_Mission09_BigGunEscortWing_SpawnReinforcements(void);
void Init_Mission09_Traders_TalkingMoreCombatLoop(void);
void Watch_Mission09_Traders_TalkingMoreCombatLoop(void);
void Init_Mission09_Traders_TalkingMoreLaunchPrep(void);
void Watch_Mission10_MissionFlow_FirstWarning(void);
void Watch_Mission10_MissionFlow_FirstWarningDelay(void);
void Watch_Mission10_MissionFlow_AttackInClawFormation(void);
void Watch_Mission10_MissionFlow_SelectNearbyAttackersOrQueueReinforcements(void);
void Watch_Mission10_MissionFlow_FleetCommandIntro(void);
void Init_Mission10_ProximitySensorObjective_WaitForTrigger(void);
void Watch_Mission10_AsteroidPocketAttackTeam_SelectAttackers(void);
void Watch_Mission10_AsteroidPocketAttackTeam_UpdateAttackers(void);
void Watch_Mission10_AsteroidPocketAttackTeam_Intro(void);
void Init_Mission10_AsteroidPocketEncounter_Start(void);
void Init_Mission10_AsteroidPocketEncounter_FocusTarget(void);
void Watch_Mission10_AsteroidPocketEncounter_AfterFocus(void);
void Watch_Mission10_AsteroidPocketEncounter_SelectAttackers(void);
void Watch_Mission10_AsteroidPocketEncounter_UpdateAttackers(void);
void Init_Mission10_AsteroidPocketAttackTeam_Start(void);
void Init_Mission10_ProximitySensorObjective_Start(void);
void Init_Mission10_BigGunAmbushTeam_Hide(void);
void Init_Mission10_P1StandardCorvetteWing_Disable(void);
void Init_Mission10_BigGunObjective_ClearTrackedSelection(void);
void Init_Mission10_BigGunObjective_CarrierObjective_Start(void);
void Init_Mission10_BigGunObjective_CarrierObjective_HideAndTrack(void);
void Watch_Mission10_BigGunObjective_CarrierObjective_WaitForCue(void);
void Init_Mission10_BigGunObjective_CarrierObjective_Release(void);
void Init_Mission10_BigGunObjective_GarrisonObjective_Start(void);
void Watch_Mission10_BigGunObjective_GarrisonObjective_HiddenTrigger(void);
void Init_Mission10_BigGunObjective_ResearchShipTeam_Activate(void);
void Watch_Mission10_BigGunObjective_ResearchShipTeam_ApproachCue(void);
void Init_Mission10_BigGunObjective_ResearchShipTeam_ReleaseHold(void);
void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void);
void Init_Mission11_MissionFlow_Delay(void);
void Init_Mission11_ObjectiveFlow(void);
void Init_Mission11_ObjectiveFlow_RepairFactorum(void);
void Watch_Mission11_ObjectiveFlow_DestroyAttackers(void);
void Watch_Mission11_ObjectiveFlow_RepairFactorum(void);
void Init_Mission11_ObjectiveFlow_UpdateBeastObjectives(void);
void Watch_Mission11_ObjectiveFlow_DestroyBeast(void);
void Watch_Mission11_ObjectiveFlow_PrimeDestroyAttackers(void);
void Watch_Mission12_StrikeTeamDispatch_WaitForACT2(void);
void Init_Mission12_StrikeTeamDispatch_WaitForACT2(void);
void Watch_Mission12_A1LaunchProximity(void);
void Begin_Mission12_A1DisableForLaunch(void);
void Begin_Mission12_ModuleOrderPatrolDisable(void);
void Watch_Mission13_MissionFlow_WaitForIntelEventEnd(void);
void Watch_Mission13_MissionFlow_FILost(void);
void Watch_Mission13_MissionFlow_Farewell(void);
void Init_Mission13_PrimaryObjectiveController(void);
void Begin_Mission13_MissionFlow_FILostAreaClear(void);
void Watch_Mission13_MissionFlow_FILostAreaClear(void);
void Init_Mission13_MissionFlow_FILostReinforcementDelay(void);
void Init_Mission13_StrikeWingShared_AttackMothership(void);
void Watch_Mission14_FleetIntel_FISensorTech(void);
void Init_Mission14_FleetIntel_FISensorTechCombatLoop(void);
void Watch_Mission14_FleetIntel_FISensorTechCombatLoop(void);
void Init_Mission14_FleetIntel_FISensorTechLaunchPrep(void);
void Init_Mission14_ReinforcementClearTrackedSelection(void);
void Init_Mission14_ReinforcementSpecialHyperspaceTeam(void);
void Init_Mission16_Events_OverwhelmedFocusTarget(void);
void Watch_Mission16_Events_OverwhelmedTrackDamagedCapital(void);
void Watch_Mission16_Events_OverwhelmedCapitalRecovered(void);
void Begin_Mission17_NaggarokCruiseMissileAttackRun(void);
void Watch_Mission17_NaggarokCruiseMissileAttackRun(void);
void Watch_Tutorial1_CurrentLessonPrompt(void);
void Watch_Tutorial1_PostIntelCleanup(void);
void Begin_Tutorial1_PlayCurrentLessonSpeech(void);
void Watch_Tutorial1_Tutorial10_MoveContextMenu(void);
void Watch_Tutorial1_Tutorial10_CameraMoveUp(void);
void Watch_Tutorial1_Tutorial10_CameraMoveDown(void);
void Watch_Tutorial1_Tutorial12_AttackTeamSignalRouter(void);
void Begin_Tutorial1_Tutorial12_RevealHiddenAttackTeam(void);
void Init_Tutorial1_Tutorial12_RequestOtherTeamHideFlag(void);
void Begin_Tutorial1_Tutorial12_RequestOtherTeamHide(void);
void Watch_Tutorial1_Tutorial12_OtherAttackTeamHide(void);
void Begin_Tutorial1_Tutorial12_MaintainAttackTeamContext(void);
void Init_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void);
void Watch_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void);

#endif
