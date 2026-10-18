/*=============================================================================
    Name    : mission16.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission16
=============================================================================*/

#include "MissionScriptSupport.h"

void MaintainTeamContext(void);
void Init_WaitForIntelEventEnd(void);
void Init_Mission05_CorvetteReinforcementDelay(void);
void Init_Mission05_FrigateReinforcementDelay(void);
void Init_Mission05_WorkerHarvestReinforcementDelay(void);
void Begin_BeastACVCombatLoopDelay(void);
void Begin_HiveFrigateReinforcementBuildTimer(void);
void Begin_HiveFrigateCombatLoop(void);
void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void);
void Init_Mission11_MissionFlow_Delay(void);
void Init_Mission11_SentinelReinforcementBuildTimer(void);
void Init_Mission09_AssaultEscort_Cooldown(void);
void FailMissionIfTrackedSelectionAlive(void);
void Begin_SentinelCombatRetryDelay(void);
void Begin_BeastIonArrayFrigateCombatLoop(void);
void Begin_BeastCloakedFighterCombatLoop(void);
void Begin_BeastMultiBeamFrigateCombatLoop(void);
void Init_BeastCorvetteReinforcementTeam(void);
void Init_BeastCorvetteReinforcementTeam_Reevaluate(void);
void Watch_BeastCorvetteReinforcementTeam(void);
void Init_BeastCorvetteReinforcementBuildTimer(void);
void Watch_BeastCorvetteReinforcementBuildTimer(void);
void Watch_BeastCorvetteReinforcementLaunchTimer(void);
void Init_BeastCorvetteCapitalStrike(void);
void Begin_BeastCorvetteCombatLoop(void);
void Watch_BeastCorvetteCombatLoop(void);
void Init_BeastIonArrayFrigateReinforcementTeam(void);
void Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastIonArrayFrigateReinforcementTeam(void);
void Init_BeastIonArrayFrigateReinforcementBuildTimer(void);
void Watch_BeastIonArrayFrigateReinforcementBuildTimer(void);
void Watch_BeastIonArrayFrigateHyperspaceAttack(void);
void Watch_BeastIonArrayFrigateCombatLoop(void);
void Begin_BeastIonArrayFrigateAttackRun(void);
void Watch_BeastIonArrayFrigateAttackRun(void);
void Init_BeastCloakedFighterReinforcementTeam(void);
void Init_BeastCloakedFighterReinforcementTeam_Reevaluate(void);
void Watch_BeastCloakedFighterReinforcementTeam(void);
void Init_BeastCloakedFighterReinforcementBuildTimer(void);
void Begin_BeastCloakedFighterReinforcementBuildTimer(void);
void Watch_BeastCloakedFighterReinforcementBuildTimer(void);
void Watch_BeastCloakedFighterCombatLoop(void);
void Begin_BeastCloakedFighterAttackRun(void);
void Watch_BeastCloakedFighterAttackRun(void);
void Init_BeastAcolyteReinforcementTeam(void);
void Init_BeastAcolyteReinforcementTeam_Reevaluate(void);
void Init_BeastReconReinforcementTeam(void);
void Init_BeastReconReinforcementTeam_Reevaluate(void);
void Init_BeastRammingFrigateReinforcementTeam(void);
void Init_BeastRammingFrigateReinforcementTeam_Reevaluate(void);
void Init_BeastMultiBeamFrigateReinforcementTeam(void);
void Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastMultiBeamFrigateReinforcementTeam(void);
void Init_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Begin_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Watch_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Watch_BeastMultiBeamFrigateCombatLoop(void);

void Init_Mission16_Events_HoneyImHome(void);
void Watch_Mission16_Events_HoneyImHome(void);
void Watch_Mission16_Events_HoneyImHomeRetryDelay(void);
void Watch_Mission16_WaitForIntelEventEnd(void);
void Watch_Mission16_MissionFlow_Delay(void);
void Watch_Mission16_MissionFlow(void);
void Init_Mission16_ObjectiveFlow(void);
void Watch_Mission16_ObjectiveFlow(void);
void Watch_Mission16_LocateCS(void);
void Watch_Mission16_WorkerDockAndReinforce(void);
void Begin_Mission16_DockWithCS(void);
void Watch_Mission16_DockedReadyForLaunch(void);
void Init_Mission16_DockWithCS(void);
void Init_Mission16_LaunchDockedTeam(void);
void Init_Mission16_MothershipAttackTeam(void);
void Watch_Mission16_MothershipAttackTeam(void);
void Init_Mission16_ReinforcementPool(void);
void Fill_Mission16_ReinforcementPoolFromParade(void);
void Watch_Mission16_ReinforcementPool_Combat(void);
void Watch_Mission16_ReinforcementPool_Resume(void);
void Init_Mission16_BeastDFGFrigateReinforcementTeam(void);
void Init_Mission16_BeastDFGFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission16_BeastDFGFrigateReinforcementTeam(void);
void Init_Mission16_BeastDFGFrigateReinforcementBuildTimer(void);
void Watch_Mission16_BeastDFGFrigateReinforcementBuildTimer(void);
signed int Begin_Mission16_BeastDFGFrigateDockAndReinforce(void);
void Watch_Mission16_BeastDFGFrigateDockAndReinforce(void);
void Begin_Mission17_BeastDFGFrigateCombatLoop(void);
void Watch_BeastDFGFrigateCombatLoop(void);
void Begin_BeastDFGFrigateReinforcementBuildTimer(void);
void Init_Mission16_BeastHeavyCruiserReinforcementTeam(void);
void Init_Mission16_BeastHeavyCruiserReinforcementTeam_Reevaluate(void);
void Watch_Mission16_BeastHeavyCruiserReinforcementTeam(void);
void Init_Mission16_BeastHeavyCruiserReinforcementBuildTimer(void);
void Watch_Mission16_BeastHeavyCruiserReinforcementBuildTimer(void);
void Watch_Mission16_BeastHeavyCruiserCombatLoop(void);
void Begin_Mission16_BeastHeavyCruiserAttackStaging(void);
void Begin_Mission16_BeastHeavyCruiserSpecialAttack(void);
void Watch_Mission16_BeastHeavyCruiserSpecialAttack(void);
void Init_Mission16_BeastHeavyCruiserReinforcementTeam_NoSourceShips(void);
void Watch_Mission16_BeastCloakedFighterDockAndReinforce(void);
void Watch_Mission16_BeastCloakedFighterLaunchDockedReinforcements(void);
void Watch_Mission16_BeastCorvetteCapitalStrike(void);
void Begin_BeastHeavyCruiserReinforcementBuildTimer(void);
void Init_Mission16_ReinforcementTriggerRouter(void);
void Watch_Mission16_ReinforcementTriggerRouter_Delay(void);
void Watch_Mission16_ReinforcementTriggerRouter(void);
void Init_Mission16_ReinforcementTriggerRouter_RecordJumpPosition(void);
void Watch_Mission16_ReinforcementTriggerRouter_AwaitJumpClearance(void);
void Begin_Mission16_ReinforcementTriggerRouter_ReturnToRecordedPosition(void);
void Watch_Mission16_ReinforcementTriggerRouter_AwaitReturnToRecordedPosition(void);
void Init_Mission16_CruiseMissileLauncher(void);
void Init_Mission16_CruiseMissileLauncher_SelectTarget(void);
void Watch_Mission16_CruiseMissileLauncher_SelectTarget(void);
void Init_Mission16_CruiseMissileLauncher_ReloadDelay(void);
void Watch_Mission16_CruiseMissileLauncher_ReloadDelay(void);
void Init_Mission16_CruiseMissileLauncher_FireVolley(void);
void Watch_Mission16_CruiseMissileLauncher_FireVolley(void);
void Init_Mission16_BeastACVReinforcementTeam(void);
void Init_Mission16_BeastACVReinforcementTeam_Reevaluate(void);
void Watch_Mission16_BeastACVReinforcementTeam(void);
void Watch_Mission16_BeastACVReinforcementBuildCount(void);
void Init_Mission16_BeastACVReinforcementBuildTimer(void);
void Watch_Mission16_BeastACVReinforcementSpawn(void);
void Begin_Mission16_BeastACVReinforcementCombatLoop(void);
void Watch_Mission16_BeastACVReinforcementCombatLoop(void);
void Begin_Mission16_BeastACVCombatAttackRun(void);
void Watch_Mission16_BeastACVCombatAttackRun(void);
void Watch_Mission16_BeastACVCombatRetryDelay(void);
void Watch_Mission16_BeastACVDockAndReinforce(void);
sdword Init_Mission16_DockForReinforcements(void);
void Init_Mission16_SentinelReinforcementTeam(void);
void Init_Mission16_SentinelReinforcementTeam_Reevaluate(void);
void Watch_Mission16_SentinelReinforcementTeam(void);
void Watch_Mission16_SentinelReinforcementBuildCount(void);
void Watch_Mission16_SentinelReinforcementSpawn(void);
void Begin_Mission16_SentinelReinforcementCombatLoop(void);
void Watch_Mission16_SentinelReinforcementCombatLoop(void);
void Watch_Mission16_SentinelCombatRetryDelay(void);
void Init_Mission16_HiveFrigateReinforcementTeam(void);
void Init_Mission16_HiveFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission16_HiveFrigateReinforcementTeam(void);
void Init_Mission16_HiveFrigateReinforcementBuildTimer(void);
void Watch_Mission16_HiveFrigateReinforcementBuildTimer(void);
void Watch_Mission16_HiveFrigateReinforcementCooldown(void);
void Init_Mission16_WorkerReinforcementTeam(void);
void Init_Mission16_WorkerReinforcementTeam_Reevaluate(void);
void Watch_Mission16_WorkerReinforcementTeam(void);
void Init_Mission16_WorkerReinforcementBuildTimer(void);
void Watch_Mission16_WorkerReinforcementBuildTimer(void);
void Watch_Mission16_WorkerHarvestReinforcementTeam(void);
void Init_Mission16_CrippleCS(void);
void Watch_Mission16_CrippleCSAwaitSignal(void);
void Init_Mission16_CrippleCSRefreshNearbySelection(void);
void Watch_Mission16_CrippleCSRefreshNearbySelection(void);
void Watch_Mission16_TriggerCrippleCSObjective(void);
void Init_Mission16_LocateCS(void);
void Init_Mission16_LocateCS_SetTeamPercentDamaged75(void);
void Watch_Mission16_EliteGuardCarrier_WaitForMessage(void);
void Init_ReinforcementPool_ClearSelectionAndReset(void);

static void Watch_Mission16_DestroyBeastObjective(void);
static void Watch_Mission16_DockWithCS_AwaitReady(void);
static void Watch_Mission16_CruiseMissileLauncher_AwaitSignal(void);

static char *m16_saveInfoName[] =
{
    "Sojent-Ra System (Far Reaches Sector)",
    "French version goes here",
    "German version goes here",
};

static char *m16_locateCsObjectiveName[] =
{
    "LOCATE_CS",
    "French version goes here",
    "German version goes here",
};

static char *m16_locateCsObjectiveTitle[] =
{
    "Locate CLEE-SAN's Position",
    "French version goes here",
    "German version goes here",
};

static char *m16_locateCsObjectiveDescription[] =
{
    "Locate CLEE-SAN's position by sending out some RECONS on patrols.",
    "French version goes here",
    "German version goes here",
};

static char *m16_crippleCsObjectiveName[] =
{
    "CRIPPLE_CS",
    "French version goes here",
    "German version goes here",
};

static char *m16_crippleCsObjectiveTitle[] =
{
    "Cripple the CLEE-SAN",
    "French version goes here",
    "German version goes here",
};

static char *m16_crippleCsObjectiveDescription[] =
{
    "Cripple the CLEE-SAN by firing a single shot from your siege cannon.  Target the CLEE-SAN within the 50% damage range of the weapon.",
    "French version goes here",
    "German version goes here",
};

static char *m16_dockWithCsObjectiveName[] =
{
    "DOCKWITH_CS",
    "French version goes here",
    "German version goes here",
};

static char *m16_dockWithCsObjectiveTitle[] =
{
    "Dock Worker with CLEE-SAN",
    "French version goes here",
    "German version goes here",
};

static char *m16_dockWithCsObjectiveDescription[] =
{
    "To dock a WORKER with the CLEE-SAN, simply fly the WORKER close to it.  Once within range, the WORKER will automatically dock and your team will transmit the fake distress signal from inside.  The WORKER will remain docked until you use the Launch command.",
    "French version goes here",
    "German version goes here",
};

static char *m16_destroyBeastObjectiveName[] =
{
    "DESTROY_BEAST",
    "French version goes here",
    "German version goes here",
};

static char *m16_destroyBeastObjectiveTitle[] =
{
    "Destroy Beast Command Ship Fleet",
    "French version goes here",
    "German version goes here",
};

static char *m16_destroyBeastObjectiveDescription[] =
{
    "Destroy the Beast Command Ship fleet.  Use the siege cannon as much as possible on the Beast Command Ship.",
    "French version goes here",
    "German version goes here",
};

static char *m16_harvestObjectiveName[] =
{
    "HARVEST",
    "French version goes here",
    "German version goes here",
};

static char *m16_harvestObjectiveTitle[] =
{
    "Harvest",
    "French version goes here",
    "German version goes here",
};

static char *m16_harvestObjectiveDescription[] =
{
    "Harvest remaining resources and make all necessary preparations.  The Beast fleet should be hyperspacing in at any moment.",
    "French version goes here",
    "German version goes here",
};

static char *m16_hyperspaceObjectiveName[] =
{
    "Hyperspace",
    "French version goes here",
    "German version goes here",
};

static char *m16_hyperspaceObjectiveDescription[] =
{
    "Hyperspace",
    "French version goes here",
    "German version goes here",
};

static char *m16_endMissionSaveInfoName[] =
{
    "~ End of Mission",
    "French version goes here",
    "German version goes here",
};

#define Init_Mission16_BeastCorvetteReinforcementTeam Init_BeastCorvetteReinforcementTeam
#define Init_Mission16_BeastCorvetteReinforcementTeam_Reevaluate Init_BeastCorvetteReinforcementTeam_Reevaluate
#define Watch_Mission16_BeastCorvetteReinforcementTeam Watch_BeastCorvetteReinforcementTeam
#define Init_Mission16_BeastCorvetteReinforcementBuildTimer Init_BeastCorvetteReinforcementBuildTimer
#define Watch_Mission16_BeastCorvetteReinforcementBuildTimer Watch_BeastCorvetteReinforcementBuildTimer
#define Begin_Mission16_BeastCorvetteCombatLoop Begin_BeastCorvetteCombatLoop
#define Watch_Mission16_BeastCorvetteCombatLoop Watch_BeastCorvetteCombatLoop

#define Init_Mission16_BeastIonArrayFrigateReinforcementTeam Init_BeastIonArrayFrigateReinforcementTeam
#define Init_Mission16_BeastIonArrayFrigateReinforcementTeam_Reevaluate Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate
#define Watch_Mission16_BeastIonArrayFrigateReinforcementTeam Watch_BeastIonArrayFrigateReinforcementTeam
#define Watch_Mission16_BeastIonArrayFrigateHyperspaceAttack Watch_BeastIonArrayFrigateHyperspaceAttack
#define Watch_Mission16_BeastIonArrayFrigateReinforcementCombatLoop Watch_BeastIonArrayFrigateCombatLoop
#define Begin_Mission16_BeastIonArrayFrigateCombatAttackRun Begin_BeastIonArrayFrigateAttackRun
#define Watch_Mission16_BeastIonArrayFrigateCombatAttackRun Watch_BeastIonArrayFrigateAttackRun

#define Init_Mission16_BeastCloakedFighterReinforcementTeam Init_BeastCloakedFighterReinforcementTeam
#define Init_Mission16_BeastCloakedFighterReinforcementTeam_Reevaluate Init_BeastCloakedFighterReinforcementTeam_Reevaluate
#define Watch_Mission16_BeastCloakedFighterReinforcementTeam Watch_BeastCloakedFighterReinforcementTeam
#define Watch_Mission16_BeastCloakedFighterReinforcementCombatLoop Watch_BeastCloakedFighterCombatLoop
#define Begin_Mission16_BeastCloakedFighterCombatAttackRun Begin_BeastCloakedFighterAttackRun
#define Watch_Mission16_BeastCloakedFighterCombatAttackRun Watch_BeastCloakedFighterAttackRun

#define Init_Mission16_BeastAcolyteReinforcementTeam Init_BeastAcolyteReinforcementTeam
#define Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate Init_BeastAcolyteReinforcementTeam_Reevaluate

#define Init_Mission16_BeastReconReinforcementTeam Init_BeastReconReinforcementTeam
#define Init_Mission16_BeastReconReinforcementTeam_Reevaluate Init_BeastReconReinforcementTeam_Reevaluate

#define Init_Mission16_BeastRammingFrigateReinforcementTeam Init_BeastRammingFrigateReinforcementTeam
#define Init_Mission16_BeastRammingFrigateReinforcementTeam_Reevaluate Init_BeastRammingFrigateReinforcementTeam_Reevaluate

#define Init_Mission16_BeastMultiBeamFrigateReinforcementTeam Init_BeastMultiBeamFrigateReinforcementTeam
#define Init_Mission16_BeastMultiBeamFrigateReinforcementTeam_Reevaluate Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate
#define Watch_Mission16_BeastMultiBeamFrigateReinforcementTeam Watch_BeastMultiBeamFrigateReinforcementTeam
#define Watch_Mission16_BeastMultiBeamFrigateCombatLoop Watch_BeastMultiBeamFrigateCombatLoop

void Watch_Mission16(void)
{
  int v0; // eax
  int v1; // eax
  int v2; // eax
  int v3; // eax
  int v4; // eax
  int *v5; // eax
  int *v6; // eax
  int *v8; // eax
  int *v9; // eax
  int v10; // eax
  int *v11; // eax
  int v12; // eax
  int v13; // eax
  int v14; // eax
  int v15; // [esp-8h] [ebp-8h]
  int v16; // [esp-8h] [ebp-8h]
  int v17; // [esp-8h] [ebp-8h]
  int v18; // [esp-4h] [ebp-4h]
  int v19; // [esp-4h] [ebp-4h]
  int v20; // [esp-4h] [ebp-4h]
  int v21; // [esp-4h] [ebp-4h]
  int v22; // [esp-4h] [ebp-4h]
  int v23; // [esp-4h] [ebp-4h]
  int v24; // [esp-4h] [ebp-4h]
  int v25; // [esp-4h] [ebp-4h]
  int v26; // [esp-4h] [ebp-4h]
  int v27; // [esp-4h] [ebp-4h]
  int *v28; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503622;
  if ( kasTimerExpiredDestroyById(150120898) )
  {
    v18 = kasGrowSelectionPtr(18336307);
    v0 = kasVolumePtr(132933767);
    kasfFindShipsInside(v0, v18);
    v19 = kasAITeamShipsPtrIfExists(132677969);
    v1 = kasGrowSelectionPtr(18336307);
    kasfShipsRemove(v1, v19);
    v20 = kasAITeamShipsPtrIfExists(132677970);
    v2 = kasGrowSelectionPtr(18336307);
    kasfShipsRemove(v2, v20);
    v21 = kasAITeamShipsPtrIfExists(132677971);
    v3 = kasGrowSelectionPtr(18336307);
    kasfShipsRemove(v3, v21);
    v22 = kasAITeamShipsPtrIfExists(132677972);
    v4 = kasGrowSelectionPtr(18336307);
    kasfShipsRemove(v4, v22);
    v23 = kasGrowSelectionPtr(18336307);
    v5 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v5, v23);
    v24 = kasGrowSelectionPtr(18336307);
    v6 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v6, v24);
    kasTimerSetById(150120898, 10);
    kasTimerStartById(150120898);
  }
  if ( !kasfEntryValueGet(71552167) )
  {
    if ( kasfEntryValueGet(-2015429404) )
    {
      kasfEntrySetValue(71552167, 1);
      v25 = kasGrowSelectionPtr(18336307);
      v8 = (int *)kasGrowSelectionPtr(89157026);
      kasSelectPlayerShipsSimple(v8, v25);
      v26 = kasGrowSelectionPtr(89157026);
      v9 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsSelectNotSalvagers(v9, v26);
      v27 = kasGrowSelectionPtr(218060896);
      v10 = kasGrowSelectionPtr(82017669);
      if ( kasfShipsSelectLinkedShips(v10, v27) )
      {
        v28 = (int *)kasGrowSelectionPtr(82017669);
        v11 = (int *)kasGrowSelectionPtr(89157026);
        kasfShipsAdd(v11, v28);
      }
      if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
      {
        v15 = kasGrowSelectionPtr(89157026);
        v12 = kasGrowSelectionPtr(96759714);
        if ( !kasfShipsSelectType(v12, v15, "sWorker") )
        {
          v16 = kasGrowSelectionPtr(218060896);
          v13 = kasGrowSelectionPtr(154639956);
          kasfShipsCreateInside(v13, "sWorker", 1, v16, 0);
        }
      }
      v17 = kasGrowSelectionPtr(0x4A2A29);
      v14 = kasGrowSelectionPtr(211966048);
      kasfShipsSelectType(v14, v17, "sMothership");
    }
  }
}

void Init_Mission16_Events_OverwhelmedFocusTarget(void)
{
    GrowSelection *protectTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationBroad();
    protectTargets = kasAITeamShipsPtrIfExists(77795571);
    if (!kasfShipsCount(protectTargets))
    {
        protectTargets = kasGrowSelectionPtr(115043347);
    }
    kasfProtect(protectTargets);
}

void Watch_Mission16_Events_OverwhelmedTrackDamagedCapital(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectCapital(kasGrowSelectionPtr(66705955), kasGrowSelectionPtr(114961833))
        && kasfShipsSelectDamaged(kasGrowSelectionPtr(66705955), kasGrowSelectionPtr(66705955), 70))
    {
        kasfSpeechEvent(313692, (sdword)MaintainTeamContext, (sdword)Watch_Mission16_Events_OverwhelmedCapitalRecovered);
    }
    else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission16_Events_OverwhelmedFocusTarget,
            (sdword)Watch_Mission16_Events_OverwhelmedTrackDamagedCapital);
    }
}

void Watch_Mission16_Events_OverwhelmedCapitalRecovered(void)
{
    GrowSelection *recoveredShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    recoveredShips = kasGrowSelectionPtr(66705955);
    if (kasfShipsSelectHealthAtLeastPercent(kasGrowSelectionPtr(80306324), recoveredShips, 100.0f)
        == kasfShipsCount(recoveredShips))
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission16_Events_OverwhelmedFocusTarget,
            (sdword)Watch_Mission16_Events_OverwhelmedTrackDamagedCapital);
    }
}

void Watch_Mission16_Events_Overwhelmed(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)5801588))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(77795571));
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission16_Events_OverwhelmedFocusTarget,
            (sdword)Watch_Mission16_Events_OverwhelmedTrackDamagedCapital);
    }
}

void Init_Mission16_MissionFlow(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasTimerCreateById(144337492);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(232774915, 0);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfEntryGetOrCreateSetValue(126773838, 1);
    kasfEntryGetOrCreateSetValue(77979796, 0);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)5801588);
    kasfMusicPlay(24);
    kasfSpeechEvent(32713509, (sdword)sub_652EF0, (sdword)Watch_Mission16_MissionFlow);
}

void Watch_Mission16_WaitForIntelEventEnd(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (!kasfIntelEventEnded() && !kasfEntryValueGet(-2054310428))
    {
        return;
    }
    if (kasfEntryValueGet(168240451))
    {
        kasfSubtitleDraw();
        kasfEntrySetValue(168240451, 0);
    }
    if (kasfEntryValueGet(266064926))
    {
        kasfWideScreenOut(90);
        kasfEntrySetValue(266064926, 0);
    }
    if (kasfEntryValueGet(109086661))
    {
        kasfUniverseUnpause();
        kasfEntrySetValue(109086661, 0);
    }
    if (kasfEntryValueGet(211831571))
    {
        kasfOtherKASUnpause();
        kasfEntrySetValue(211831571, 0);
    }
    if (kasfEntryValueGet(52577235))
    {
        if (!kasfEntryValueGet(126773838))
        {
            kasfCloseSensors(1);
            kasfEntrySetValue(126773838, 1);
        }
        kasfEntrySetValue(52577235, 0);
    }
    if (kasfEntryValueGet(172907219))
    {
        kasfTutCameraFocusCancel();
        kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
        kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
        kasfCameraSetDistance(kasfEntryValueGet(64272901));
        kasfEntrySetValue(172907219, 0);
    }
    if (kasfEntryValueGet(232774915))
    {
        kasfTutCameraFocusCancel();
        kasfTutCameraFocusCancel();
        kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
        kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
        kasfCameraSetDistance(kasfEntryValueGet(64272901));
        kasfEntrySetValue(232774915, 0);
    }
    if (kasfEntryValueGet(77979796))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)sub_652EF0);
    }
    else if (kasfEntryValueGet(127756738))
    {
        kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission16_MissionFlow_Delay);
    }
    else
    {
        kasfSpeechEvent(32713509, (sdword)sub_652EF0, (sdword)Watch_Mission16_MissionFlow);
    }
}

void Watch_Mission16_MissionFlow_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)220350372))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)sub_652EF0);
    }
    else if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(32713509, (sdword)sub_652EF0, (sdword)Watch_Mission16_MissionFlow);
    }
}

void Init_Mission16_BeastDFGFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(18599)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-1981522985, 0);
        kasfEntryGetOrCreateSetValue(-1949140875, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44796563);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 1);
            kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission16_BeastDFGFrigateReinforcementTeam);
        }
    }
}

void Init_Mission16_BeastDFGFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(18599)) &&
        kasfEntryValueGet(-1949140875) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1949140875, 0);
        kasfEntrySetValue(-1981522985, 1);
    }
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasGrowSelectionPtr((char *)165711363)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(299899, (sdword)Begin_Mission16_BeastDFGFrigateDockAndReinforce, (sdword)Watch_Mission16_BeastDFGFrigateDockAndReinforce);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)228101699);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission16_BeastDFGFrigateReinforcementTeam);
        }
    }
}

void Watch_Mission16_BeastDFGFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *newFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    newFrigates = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bDFGFrigate") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr((char *)165711363)))
        {
            if (kasfEntryValueGet(-1981522985))
            {
                kasfEntrySetValue(-1981522985, 0);
                kasfEntrySetValue(-1949140875, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, (sdword)Init_Mission16_BeastDFGFrigateReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-1981522985, 0);
            kasfEntrySetValue(-1949140875, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44796563);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bDFGFrigate"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), newFrigates);
        kasfShipsStop(newFrigates);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), newFrigates);
        kasfShipsSetSlaveableFlag(newFrigates, 0);
        kasfShipsApplyPlayerUpgrades(newFrigates);
        kasfReinforceTeamWithShips(team, newFrigates);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
}

void Init_Mission16_BeastDFGFrigateReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(154639956) < 2 && kasfRUsGet(1) >= kasfShipTypeBuildCost("bDFGFrigate") / 2)
        {
            kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bDFGFrigate") / 2);
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate") / 2)
        {
            kasfSpeechEvent(75929221, (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer, (sdword)Watch_Mission16_BeastDFGFrigateReinforcementBuildTimer);
            return;
        }
    }
    else if (buildStyle == 2)
    {
        if (kasfEntryValueGet(154639956) < 2)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 2)
        {
            kasfSpeechEvent(75929221, (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer, (sdword)Watch_Mission16_BeastDFGFrigateReinforcementBuildTimer);
        }
        return;
    }
    else if (buildStyle != 0)
    {
        return;
    }
    else if (kasfEntryValueGet(154639956) < 2 && kasfRUsGet(1) >= kasfShipTypeBuildCost("bDFGFrigate"))
    {
        buildCost = kasfShipTypeBuildCost("bDFGFrigate");
        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
        kasfEntryIncrement(154639956);
    }

    if (kasfEntryValueGet(154639956) == 2)
    {
        kasfSpeechEvent(75929221, (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer, (sdword)Watch_Mission16_BeastDFGFrigateReinforcementBuildTimer);
    }
}

void Watch_Mission16_BeastDFGFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *spawnParents;
    GrowSelection *spawnPointSelection;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasGrowSelectionPtr((char *)165711363);
    if (!kasfShipsCount(spawnParents))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        targetIndex = kasfRandom(0, kasfShipsCount(spawnParents) - 1);
        spawnPointSelection = kasGrowSelectionPtr((char *)21541);
        kasfShipsSelectIndex(spawnPointSelection, spawnParents, targetIndex);
        createdShips = kasGrowSelectionPtr((char *)154639956);
        kasfShipsCreateInside(createdShips, "bDFGFrigate", kasfEntryValueGet(154639956), spawnPointSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }
    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44796563);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
    }
}

void Init_Mission16_BeastHeavyCruiserReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(212896882)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1920527822, 0);
       kasfEntryGetOrCreateSetValue(-2110057163, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)185462083);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsSetSlaveableFlag(&team->shipList, 0);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasTimerSetById(142843908, 20);
           kasTimerStartById(142843908);
           kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_Mission16_BeastHeavyCruiserCombatLoop);
       }
       else
       {
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission16_BeastHeavyCruiserReinforcementTeam);
       }
   }
}

void Init_Mission16_BeastHeavyCruiserReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(212896882)) &&
       kasfEntryValueGet(-2110057163) &&
       kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-2110057163, 0);
       kasfEntrySetValue(-1920527822, 1);
   }

   if (!kasfEntryValueGet(332667))
   {
       if (kasfTeamCount())
       {
           if (kasfShipsAnyBigGunReady(&team->shipList) &&
               kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr((char *)0x4A2A29), kasThisTeamsVectorPtr(), 20000))
           {
               kasfSpeechEvent(
                   157845977,
                   (sdword)Begin_Mission16_BeastHeavyCruiserSpecialAttack,
                   (sdword)Watch_Mission16_BeastHeavyCruiserSpecialAttack);
           }
       }
       else
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)188751475);
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission16_BeastHeavyCruiserReinforcementTeam);
       }
   }
}

void Watch_Mission16_BeastHeavyCruiserReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *newCruisers;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   newCruisers = kasGrowSelectionPtr(363040);
   if (kasfShipsSelectType(newCruisers, kasAITeamShipsPtrIfExists(44665972), "bHeavyCruiser") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr((char *)165711363)))
       {
           if (kasfEntryValueGet(-1920527822))
           {
               kasfEntrySetValue(-1920527822, 0);
               kasfEntrySetValue(-2110057163, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Init_Mission16_BeastHeavyCruiserReinforcementBuildTimer);
           }
       }
       else
       {
           kasfEntrySetValue(-1920527822, 0);
           Init_Mission16_BeastHeavyCruiserReinforcementTeam_NoSourceShips();
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)185462083);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(newCruisers, kasAITeamShipsPtrIfExists(44665972), "bHeavyCruiser"))
   {
       kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), newCruisers);
       kasfShipsStop(newCruisers);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), newCruisers);
       kasfShipsSetSlaveableFlag(newCruisers, 0);
       kasfShipsApplyPlayerUpgrades(newCruisers);
       kasfReinforceTeamWithShips(team, newCruisers);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_Mission16_BeastHeavyCruiserCombatLoop);
}

void Init_Mission16_BeastHeavyCruiserReinforcementBuildTimer(void)
{
   sdword buildScale;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildScale = kasfEntryValueGet(-2126662828);
   if (buildScale == 2)
   {
       if (kasfEntryValueGet(154639956) < 1)
       {
           kasfEntryIncrement(154639956);
       }
   }
   else if (buildScale == 1)
   {
       if (kasfEntryValueGet(154639956) < 1)
       {
           buildCost = kasfShipTypeBuildCost("bHeavyCruiser") / 2;
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 1 &&
           (sdword)kasfRUsGet(1) >= kasfShipTypeBuildCost("bHeavyCruiser") / 2)
       {
           return;
       }
   }
   else if (buildScale == 0)
   {
       if (kasfEntryValueGet(154639956) < 1)
       {
           buildCost = kasfShipTypeBuildCost("bHeavyCruiser");
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 1 &&
           (sdword)kasfRUsGet(1) >= kasfShipTypeBuildCost("bHeavyCruiser"))
       {
           return;
       }
   }
   else
   {
       return;
   }

   if (kasfEntryValueGet(154639956) == 1)
   {
       kasfSpeechEvent(
           75929221,
           (sdword)Begin_BeastHeavyCruiserReinforcementBuildTimer,
           (sdword)Watch_Mission16_BeastHeavyCruiserReinforcementBuildTimer);
   }
}

void Watch_Mission16_BeastHeavyCruiserReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *createdShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (!kasfShipsCount(kasGrowSelectionPtr((char *)165711363)))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       createdShips = kasGrowSelectionPtr((char *)154639956);
       kasfShipsCreate(createdShips, "bHeavyCruiser", kasfEntryValueGet(154639956), 1);
       kasAITeamShipsPtr(createdShips, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
       kasfShipsSetSlaveableFlag(&team->shipList, 0);
       kasfSpeechEvent(
           175938357,
           (sdword)ResetReinforcementHyperspaceAttackFlag,
           (sdword)Begin_Mission16_BeastHeavyCruiserAttackStaging);
   }
}

void Watch_Mission16_BeastHeavyCruiserCombatLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_Mission16_BeastHeavyCruiserCombatLoop);
   }
}

void Init_Mission16_CruiseMissileLauncher(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(320211, (sdword)sub_652EF0, (sdword)Watch_Mission16_CruiseMissileLauncher_AwaitSignal);
}

static void Watch_Mission16_CruiseMissileLauncher_AwaitSignal(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x08700AD8))
    {
        kasfSpeechEvent(0x04869685, (sdword)Init_Mission16_CruiseMissileLauncher_FireVolley, (sdword)Watch_Mission16_CruiseMissileLauncher_FireVolley);
    }
}

void Init_Mission16_CruiseMissileLauncher_SelectTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(0x05821184), (char *)0x07E18873);
    kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x0764E81C), kasGrowSelectionPtr((char *)0x4A2A29));
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x0764E81C));
}

void Watch_Mission16_CruiseMissileLauncher_SelectTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x08830B65, (sdword)Init_Mission16_CruiseMissileLauncher_ReloadDelay, (sdword)Watch_Mission16_CruiseMissileLauncher_ReloadDelay);
    }
}

void Init_Mission16_CruiseMissileLauncher_ReloadDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(0x08E0C8C2, kasfRandom(15, 45));
}

void Watch_Mission16_CruiseMissileLauncher_ReloadDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(0x08E0C8C2))
    {
        kasfSpeechEvent(75929221, (sdword)Init_Mission16_CruiseMissileLauncher_FireVolley, (sdword)Watch_Mission16_CruiseMissileLauncher_FireVolley);
    }
}

void Init_Mission16_CruiseMissileLauncher_FireVolley(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfGrowSelectionClear(kasGrowSelectionPtr((char *)21159));
    kasfShipsCreateInside(
        kasGrowSelectionPtr((char *)21159),
        "bCruiseMissile",
        (char *)kasfRandom(2, 4),
        kasAITeamShipsPtrIfExists(216029280),
        1);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr((char *)21159));
}

void Watch_Mission16_CruiseMissileLauncher_FireVolley(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(139967671, (sdword)Init_Mission16_CruiseMissileLauncher_SelectTarget, (sdword)Watch_Mission16_CruiseMissileLauncher_SelectTarget);
    }
}

void Init_Mission16_BeastACVReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(17798)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1981524234, 0);
       kasfEntryGetOrCreateSetValue(-2030864267, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44783843);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasfSpeechEvent(
               88512316,
               (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
               (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
       }
       else
       {
           kasfSpeechEvent(
               4693508,
               (sdword)sub_65F6F0,
               (sdword)Watch_Mission16_BeastACVReinforcementTeam);
       }
   }
}

void Init_Mission16_BeastACVReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(17798)) &&
       kasfEntryValueGet(-2030864267) &&
       kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-2030864267, 0);
       kasfEntrySetValue(-1981524234, 1);
   }

   if (!kasfEntryValueGet(332667))
   {
       if (kasfTeamCount())
       {
           if (kasfShipsCount(kasGrowSelectionPtr(165711363)) && kasfTeamHealthAverage() < 50)
           {
               kasfSpeechEvent(
                   299899,
                   (sdword)Init_Mission16_DockForReinforcements,
                   (sdword)Watch_Mission16_BeastACVDockAndReinforce);
           }
       }
       else
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)228088883);
           kasfSpeechEvent(
               4693508,
               (sdword)sub_65F6F0,
               (sdword)Watch_Mission16_BeastACVReinforcementTeam);
       }
   }
}

void Watch_Mission16_BeastACVReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *returnedACVs;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   returnedACVs = kasGrowSelectionPtr(363040);
   if (kasfShipsSelectType(returnedACVs, kasGrowSelectionPtr(44665972), "bACV") < 2)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(165711363)))
       {
           if (kasfEntryValueGet(-1981524234))
           {
               kasfEntrySetValue(-1981524234, 0);
               kasfEntrySetValue(-2030864267, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Watch_Mission16_BeastACVReinforcementBuildCount);
           }
       }
       else
       {
           kasfEntrySetValue(-1981524234, 0);
           kasfEntrySetValue(-2030864267, 1);
           kasfSpeechEvent(
               342540,
               (sdword)Init_Mission09_AssaultEscort_Cooldown,
               (sdword)sub_652EF0);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44783843);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(returnedACVs, kasAITeamShipsPtrIfExists(44665972), "bACV"))
   {
       kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedACVs);
       kasfShipsStop(returnedACVs);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedACVs);
       kasfShipsSetSlaveableFlag(returnedACVs, 0);
       kasfShipsApplyPlayerUpgrades(returnedACVs);
       kasfReinforceTeamWithShips(team, returnedACVs);
       kasfTacticsNeutral();
   }

   kasfSpeechEvent(
       88512316,
       (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
       (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
}

void Watch_Mission16_BeastACVReinforcementBuildCount(void)
{
   sdword buildScale;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildScale = kasfEntryValueGet(-2126662828);
   if (buildScale != 0)
   {
       buildScale--;
       if (buildScale != 0)
       {
           if (buildScale != 1)
           {
               return;
           }
           if (kasfEntryValueGet(154639956) < 10)
           {
               kasfEntryIncrement(154639956);
           }
checkAcvBuildReady:
           if (kasfEntryValueGet(154639956) != 10)
           {
               return;
           }
           goto beginAcvBuildTimer;
       }

       if (kasfEntryValueGet(154639956) < 10)
       {
           buildCost = kasfShipTypeBuildCost("bACV") / 2;
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) <= 2)
       {
           goto checkAcvBuildReady;
       }
       buildCost = kasfShipTypeBuildCost("bACV") / 2;
       if (kasfRUsGet(1) >= buildCost)
       {
           goto checkAcvBuildReady;
       }
beginAcvBuildTimer:
       kasfSpeechEvent(
           75929221,
           (sdword)Init_Mission16_BeastACVReinforcementBuildTimer,
           (sdword)Watch_Mission16_BeastACVReinforcementSpawn);
       return;
   }

   if (kasfEntryValueGet(154639956) < 10)
   {
       buildCost = kasfShipTypeBuildCost("bACV");
       if (kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if (kasfEntryValueGet(154639956) > 2)
   {
       buildCost = kasfShipTypeBuildCost("bACV");
       if (kasfRUsGet(1) < buildCost)
       {
           goto beginAcvBuildTimer;
       }
   }
   if (kasfEntryValueGet(154639956) == 10)
   {
       goto beginAcvBuildTimer;
   }
}

void Init_Mission16_BeastACVReinforcementBuildTimer(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword buildTime;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    switch (kasfEntryValueGet(-2126662828))
    {
        case 2:
            kasTimerCreateSetStartById(235522949, 0);
            break;

        case 1:
            buildTime = kasfShipTypeBuildTime("bACV") * kasfEntryValueGet(154639956) / 2;
            kasTimerCreateSetStartById(235522949, buildTime);
            break;

        case 0:
            buildTime = kasfShipTypeBuildTime("bACV") * kasfEntryValueGet(154639956);
            kasTimerCreateSetStartById(235522949, buildTime);
            break;
    }
}

void Watch_Mission16_BeastACVReinforcementSpawn(void)
{
   AITeam *team;
   GrowSelection *spawnSource;
   GrowSelection *spawnTarget;
   GrowSelection *createdACVs;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   spawnSource = kasGrowSelectionPtr(0x09E08E03);
   if (!kasfShipsCount(spawnSource))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(
           342540,
           (sdword)Init_Mission09_AssaultEscort_Cooldown,
           (sdword)sub_652EF0);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnTarget = kasGrowSelectionPtr(0x5425);
       kasfShipsSelectIndex(spawnTarget, spawnSource, kasfRandom(0, kasfShipsCount(spawnSource) - 1));
       createdACVs = kasGrowSelectionPtr(0x9379E54);
       kasfShipsCreateInside(createdACVs, "bACV", kasfEntryValueGet(154639956), spawnTarget, 1);
       kasAITeamShipsPtr(createdACVs, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdACVs);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfEntrySetValue(332667, 0);
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44783843);
       kasfSpeechEvent(
           88512316,
           (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
           (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
   }
}

void Begin_Mission16_BeastACVReinforcementCombatLoop(void)
{
   GrowSelection *capitalTargets;
   GrowSelection *nearbyTargets;
   GrowSelection *underAttackTargets;
   sdword randomIndex;
   sdword randomDistance;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   capitalTargets = kasGrowSelectionPtr((char *)105808290);
   kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
   nearbyTargets = kasGrowSelectionPtr((char *)124053532);
   if (!kasfShipsSelectNearby(nearbyTargets, capitalTargets, kasThisTeamsVectorPtr(), 60000))
   {
       kasfGrowSelectionClear(nearbyTargets);
       kasfShipsAdd(nearbyTargets, capitalTargets);
   }
   if (!kasfShipsCount(nearbyTargets))
   {
       kasfSpeechEvent(
           374244,
           (sdword)Begin_BeastACVCombatLoopDelay,
           (sdword)Watch_Mission16_BeastACVCombatRetryDelay);
       return;
   }

   kasfTacticsNeutral();
   underAttackTargets = kasGrowSelectionPtr((char *)139967715);
   if (kasfFindShipsTargetingSelection(underAttackTargets, nearbyTargets))
   {
       kasfShipsSortByDistanceToPoint(underAttackTargets, (vector *)kasThisTeamsVectorPtr());
       kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), underAttackTargets, 0);
       kasfFormationDelta3D();
       kasfMoveTo(kasShipsVectorPtrIfExists((char *)21541));
       return;
   }

   if (kasfShipsSelectNearby(kasGrowSelectionPtr((char *)338274), nearbyTargets, kasShipsVectorPtr((char *)0x4E20), 20000))
   {
       kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)338274), (vector *)kasThisTeamsVectorPtr());
       kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), kasGrowSelectionPtr((char *)338274), 0);
       kasfFormationDelta3D();
       kasfMoveTo(kasShipsVectorPtrIfExists((char *)21541));
       return;
   }

   randomIndex = kasfRandom(0, kasfShipsCount(nearbyTargets) - 1);
   kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), nearbyTargets, randomIndex);
   kasfLabelledVectorSet((char *)146094692, 0, 0, 0);
   randomDistance = kasfRandom(6000, 8000);
   kasfVectorRandomizeInRange(
       kasVectorPtrIfExists((char *)146094692),
       kasShipsVectorPtrIfExists((char *)21541),
       randomDistance);
   kasfFormationDelta3D();
   kasfMoveTo(kasVectorPtrIfExists((char *)146094692));
}

void Watch_Mission16_BeastACVReinforcementCombatLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsSelectNearby(
           kasGrowSelectionPtr((char *)0x36980EB),
           kasGrowSelectionPtr((char *)0x4A2A29),
           kasShipsVectorPtr((char *)0x4E20),
           20000) ||
       (kasfUnderAttack(kasGrowSelectionPtr((char *)139967715)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)0x36980EB))))
   {
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission16_BeastACVCombatAttackRun,
           (sdword)Watch_Mission16_BeastACVCombatAttackRun);
   }
   else if (kasfShipsAnyIdle(&team->shipList))
   {
       kasfSpeechEvent(
           88512316,
           (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
           (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
   }
}

void Begin_Mission16_BeastACVCombatAttackRun(void)
{
   AITeam *team;
   GrowSelection *nearbyTargets;
   GrowSelection *attackTargets;
   GrowSelection *specialTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   nearbyTargets = kasGrowSelectionPtr((char *)0x5296679);
   if (!kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr((char *)0x4A2A29), kasShipsVectorPtr((char *)0x4E20), 20000))
   {
       kasfSpeechEvent(
           88512316,
           (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
           (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
       return;
   }

   attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
   if (!kasfShipsSelectType(attackTargets, nearbyTargets, "sLeech") &&
       !kasfShipsSelectType(attackTargets, nearbyTargets, "sMCV") &&
       !kasfShipsSelectType(attackTargets, nearbyTargets, "sMimic"))
   {
       kasfShipsAdd(attackTargets, nearbyTargets);
   }
   if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
   {
       kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x857BCE3));
   }

   kasfFormationDelta3D();
   kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
   if (kasfShipsAnyHyperspaceReady(&team->shipList))
   {
       specialTargets = kasGrowSelectionPtr((char *)0x52962);
       if (kasfShipsCount(kasGrowSelectionPtr((char *)0x857BCE3)) &&
           kasfShipsSelectMembersOfSelection(specialTargets, kasGrowSelectionPtr((char *)0x857BCE3), attackTargets))
       {
           kasfIssueSpecialAttack(specialTargets);
       }
       else
       {
           kasfIssueSpecialAttack(attackTargets);
       }
   }
   else
   {
       tutDeallocateRootRegion(attackTargets);
   }

   kasTimerDestroyById(74024315);
   kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission16_BeastACVCombatAttackRun(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList))
   {
       if (!kasfShipsSelectNearby(
               kasGrowSelectionPtr((char *)0x36980EB),
               kasGrowSelectionPtr((char *)0x4A2A29),
               kasShipsVectorPtr((char *)0x4E20),
               20000))
       {
           kasfSpeechEvent(
               88512316,
               (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
               (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
           return;
       }
   }
   else if (!kasTimerExpiredById(74024315))
   {
       return;
   }

   kasfSpeechEvent(
       74024315,
       (sdword)Begin_Mission16_BeastACVCombatAttackRun,
       (sdword)Watch_Mission16_BeastACVCombatAttackRun);
}

void Watch_Mission16_BeastACVCombatRetryDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasTimerExpiredById(374244))
   {
       kasfSpeechEvent(
           88512316,
           (sdword)Begin_Mission16_BeastACVReinforcementCombatLoop,
           (sdword)Watch_Mission16_BeastACVReinforcementCombatLoop);
   }
}

void Init_Mission16_SentinelReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(171501548)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-1884903716, 0);
        kasfEntryGetOrCreateSetValue(-2031143291, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(76130852, (sdword)Begin_Mission16_SentinelReinforcementCombatLoop, (sdword)Watch_Mission16_SentinelReinforcementCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission16_SentinelReinforcementTeam);
        }
    }
}

void Init_Mission16_SentinelReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(171501548)) &&
        kasfEntryValueGet(-2031143291) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2031143291, 0);
        kasfEntrySetValue(-1884903716, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)190169075);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission16_SentinelReinforcementTeam);
    }
}

void Watch_Mission16_SentinelReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedSentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedSentinels = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(returnedSentinels, kasAITeamShipsPtrIfExists(44665972), "bSentinel") < 3)
    {
        if (kasfShipsCount(kasGrowSelectionPtr((char *)165711363)))
        {
            if (kasfEntryValueGet(-1884903716))
            {
                kasfEntrySetValue(-1884903716, 0);
                kasfEntrySetValue(-2031143291, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, (sdword)Watch_Mission16_SentinelReinforcementBuildCount);
            }
        }
        else
        {
            kasfEntrySetValue(-1884903716, 0);
            kasfEntrySetValue(-2031143291, 1);
            kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(returnedSentinels, kasAITeamShipsPtrIfExists(44665972), "bSentinel"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedSentinels);
        kasfShipsStop(returnedSentinels);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedSentinels);
        kasfShipsSetSlaveableFlag(returnedSentinels, 0);
        kasfShipsApplyPlayerUpgrades(returnedSentinels);
        kasfReinforceTeamWithShips(team, returnedSentinels);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(76130852, (sdword)Begin_Mission16_SentinelReinforcementCombatLoop, (sdword)Watch_Mission16_SentinelReinforcementCombatLoop);
}

void Watch_Mission16_SentinelReinforcementBuildCount(void)
{
    sdword buildStyle;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(154639956) < 12 && kasfRUsGet(1) >= kasfShipTypeBuildCost("bSentinel") / 2)
        {
            kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bSentinel") / 2);
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) > 3 && kasfRUsGet(1) < kasfShipTypeBuildCost("bSentinel") / 2)
        {
            kasfSpeechEvent(75929221, (sdword)Init_Mission11_SentinelReinforcementBuildTimer, (sdword)Watch_Mission16_SentinelReinforcementSpawn);
            return;
        }
    }
    else if (buildStyle == 2)
    {
        if (kasfEntryValueGet(154639956) < 12)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 12)
        {
            kasfSpeechEvent(75929221, (sdword)Init_Mission11_SentinelReinforcementBuildTimer, (sdword)Watch_Mission16_SentinelReinforcementSpawn);
        }
        return;
    }
    else if (buildStyle != 0)
    {
        return;
    }
    else if (kasfEntryValueGet(154639956) < 12 && kasfRUsGet(1) >= kasfShipTypeBuildCost("bSentinel"))
    {
        buildCost = kasfShipTypeBuildCost("bSentinel");
        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
        kasfEntryIncrement(154639956);
    }

    if (kasfEntryValueGet(154639956) == 12)
    {
        kasfSpeechEvent(75929221, (sdword)Init_Mission11_SentinelReinforcementBuildTimer, (sdword)Watch_Mission16_SentinelReinforcementSpawn);
    }
}

void Watch_Mission16_SentinelReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *spawnParents;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasGrowSelectionPtr((char *)165711363);
    if (!kasfShipsCount(spawnParents))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
        kasfShipsCreateInside(kasGrowSelectionPtr((char *)154639956), "bSentinel", kasfEntryValueGet(154639956), kasGrowSelectionPtr((char *)21541), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)154639956), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)154639956));
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(76130852, (sdword)Begin_Mission16_SentinelReinforcementCombatLoop, (sdword)Watch_Mission16_SentinelReinforcementCombatLoop);
    }
}

void Begin_Mission16_SentinelReinforcementCombatLoop(void)
{
    GrowSelection *teamShips;
    GrowSelection *priorityTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    teamShips = kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash);
    kasfShipsSelectCapital(teamShips, kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
    kasfShipsSelectRace(teamShips, teamShips, 1);
    if (kasfShipsSelectType(kasGrowSelectionPtr((char *)171484652), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bSentinel") &&
        kasfShipsSelectTargets(kasGrowSelectionPtr((char *)157715175), kasGrowSelectionPtr((char *)171484652)))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr((char *)157715175));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr((char *)151690773), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bIonArrayFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr((char *)151690773));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr((char *)223105365), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bRammingFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr((char *)223105365));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr((char *)191446069), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bDFGFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr((char *)191446069));
    }

    if (!kasfShipsCount(teamShips) && !kasfEntryValueGet(84320888))
    {
        kasfSpeechEvent(374244, (sdword)Begin_SentinelCombatRetryDelay, (sdword)Watch_Mission16_SentinelCombatRetryDelay);
        return;
    }

    kasfTacticsNeutral();
    kasfFormationWall();
    kasfShipsSelectNearby(kasGrowSelectionPtr((char *)86599289), teamShips, kasThisTeamsVectorPtr(), 20000);
    kasfShipsSelectType(kasGrowSelectionPtr((char *)212896882), kasGrowSelectionPtr((char *)86599289), "bHeavyCruiser");
    kasfShipsSelectType(kasGrowSelectionPtr((char *)124218850), kasGrowSelectionPtr((char *)86599289), "bCarrier");
    kasfShipsSelectType(kasGrowSelectionPtr((char *)216029280), kasGrowSelectionPtr((char *)86599289), "bMothership");
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)212896882)) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)124218850)) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)216029280)))
    {
        kasfShipsSortByDistanceToPoint(teamShips, kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)154965556), teamShips, 0);
    }
    else
    {
        priorityTargets = kasGrowSelectionPtr((char *)125446884);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)212896882));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)124218850));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)216029280));
        kasfShipsSortByDistanceToPoint(priorityTargets, kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)154965556), priorityTargets, 0);
    }
    kasfProtect(kasGrowSelectionPtr((char *)154965556));
    kasTimerDestroyById(111581908);
    kasTimerDestroyById(49272164);
    kasTimerDestroyById(127823460);
}

void Watch_Mission16_SentinelReinforcementCombatLoop(void)
{
    AITeam *team;
    sdword requestedCount;
    sdword totalCost;
    GrowSelection *spawnParents;
    GrowSelection *spawnTarget;
    GrowSelection *createdSentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        if (kasfEntryValueGet(127823460) || (kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
        {
            if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0 && !kasfEntryValueGet(84320888))
            {
                kasfEntrySetValue(127823460, 0);
                kasfStop();
                kasfSpeechEvent(374244, (sdword)Begin_SentinelCombatRetryDelay, (sdword)Watch_Mission16_SentinelCombatRetryDelay);
                return;
            }
        }
        else
        {
            kasfShipsSetFormation(&team->shipList, 10);
            kasfEntryGetOrCreateSetValue(127823460, 1);
        }
    }
    if (kasfEntryValueGet(111581908))
    {
        if (!kasfEntryValueGet(49272164))
        {
            kasfEntryGetOrCreateSetValue(49272164, 1);
            kasTimerCreateSetStartById(111581908, kasfEntryValueGet(111592388) * kasfShipTypeBuildTime("bSentinel"));
        }
    }
    else if (kasfTeamCount() < 12)
    {
        kasfEntrySetValue(111592388, 12 - kasfTeamCount());
        kasfEntryGetOrCreateSetValue(111581908, 1);
    }
    if (kasfEntryValueGet(-2126662828) == 0)
    {
        kasfEntryGetOrCreateSetValue(235422484, kasfShipTypeBuildCost("bSentinel"));
    }
    else if (kasfEntryValueGet(-2126662828) == 2)
    {
        kasfEntryGetOrCreateSetValue(235422484, 0);
    }
    requestedCount = kasfEntryValueGet(111592388);
    totalCost = requestedCount * kasfEntryValueGet(235422484);
    if (kasfRUsGet(1) >= totalCost && kasTimerExpiredById(111581908))
    {
        spawnParents = kasGrowSelectionPtr((char *)165711363);
        if (kasfShipsCount(spawnParents))
        {
            kasfRUsSet(1, kasfRUsGet(1) - totalCost);
            spawnTarget = kasGrowSelectionPtr((char *)21541);
            kasfShipsSelectIndex(spawnTarget, spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
            if (kasfTeamCount() + kasfEntryValueGet(111592388) >= 12)
            {
                kasfEntrySetValue(111592388, 12 - kasfTeamCount());
            }
            createdSentinels = kasGrowSelectionPtr((char *)154639956);
            kasfShipsCreateInside(createdSentinels, "bSentinel", kasfEntryValueGet(111592388), spawnTarget, 1);
            kasAITeamShipsPtr(createdSentinels, 1);
            kasfShipsAdd(&team->shipList, createdSentinels);
            kasfEntryGetOrCreateSetValue(84320888, 1);
        }
    }
    if (kasfEntryValueGet(84320888) && kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
        kasfEntrySetValue(84320888, 0);
        kasfEntrySetValue(111581908, 0);
        kasfEntrySetValue(49272164, 0);
        kasfShipsSelectTargets(&team->shipList, kasGrowSelectionPtr((char *)157715175));
        kasfProtect(kasGrowSelectionPtr((char *)157715175));
        kasfEntrySetValue(127823460, 0);
    }
}

void Watch_Mission16_SentinelCombatRetryDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfStop();
        kasfSpeechEvent(76130852, (sdword)Begin_Mission16_SentinelReinforcementCombatLoop, (sdword)Watch_Mission16_SentinelReinforcementCombatLoop);
    }
}

void Init_Mission16_HiveFrigateReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(186735637)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1892538891, 0);
       kasfEntryGetOrCreateSetValue(-2047426043, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)219517875);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasfSpeechEvent(
               74024315,
               (sdword)Begin_HiveFrigateCombatLoop,
               (sdword)Watch_Mission16_HiveFrigateReinforcementCooldown);
       }
       else
       {
           kasfSpeechEvent(
               4693508,
               (sdword)sub_65F6F0,
               (sdword)Watch_Mission16_HiveFrigateReinforcementTeam);
       }
   }
}

void Init_Mission16_HiveFrigateReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(186735637)) &&
       kasfEntryValueGet(-2047426043) &&
       kasfMsgReceived((char *)0x479E04))
   {
       kasfEntrySetValue(-2047426043, 0);
       kasfEntrySetValue(-1892538891, 1);
   }

   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)236014723);
       kasfSpeechEvent(
           4693508,
           (sdword)sub_65F6F0,
           (sdword)Watch_Mission16_HiveFrigateReinforcementTeam);
   }
}

void Watch_Mission16_HiveFrigateReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *returnedHiveFrigates;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   returnedHiveFrigates = kasGrowSelectionPtr(0x58A20);
   if (kasfShipsSelectType(returnedHiveFrigates, kasGrowSelectionPtr(0x2A98C74), "bHiveFrigate") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
       {
           if (kasfEntryValueGet(-1892538891))
           {
               kasfEntrySetValue(-1892538891, 0);
               kasfEntrySetValue(-2047426043, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Init_Mission16_HiveFrigateReinforcementBuildTimer);
           }
       }
       else
       {
           kasfEntrySetValue(-1892538891, 0);
           kasfEntrySetValue(-2047426043, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)219517875);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(returnedHiveFrigates, kasAITeamShipsPtrIfExists(44665972), "bHiveFrigate"))
   {
       kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedHiveFrigates);
       kasfShipsStop(returnedHiveFrigates);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedHiveFrigates);
       kasfShipsSetSlaveableFlag(returnedHiveFrigates, 0);
       kasfShipsApplyPlayerUpgrades(returnedHiveFrigates);
       kasfReinforceTeamWithShips(team, returnedHiveFrigates);
       kasfTacticsNeutral();
   }

   kasfSpeechEvent(
       74024315,
       (sdword)Begin_HiveFrigateCombatLoop,
       (sdword)Watch_Mission16_HiveFrigateReinforcementCooldown);
}

void Init_Mission16_HiveFrigateReinforcementBuildTimer(void)
{
   AITeam *team;
   sdword buildScale;
   sdword buildCost;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   buildScale = kasfEntryValueGet(-2126662828);
   if (buildScale != 0)
   {
       buildScale--;
       if (buildScale != 0)
       {
           if (buildScale != 1)
           {
               return;
           }
           if (kasfEntryValueGet(154639956) < 2)
           {
               kasfEntryIncrement(154639956);
           }
checkHiveBuildReady:
           if (kasfEntryValueGet(154639956) != 2)
           {
               return;
           }
           goto beginHiveBuildTimer;
       }

       if (kasfEntryValueGet(154639956) < 2)
       {
           buildCost = kasfShipTypeBuildCost("bHiveFrigate") / 2;
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) <= 1)
       {
           goto checkHiveBuildReady;
       }

       buildCost = kasfShipTypeBuildCost("bHiveFrigate") / 2;
       if (kasfRUsGet(1) >= buildCost)
       {
           goto checkHiveBuildReady;
       }
beginHiveBuildTimer:
       kasfSpeechEvent(
           75929221,
           (sdword)Begin_HiveFrigateReinforcementBuildTimer,
           (sdword)Watch_Mission16_HiveFrigateReinforcementBuildTimer);
       return;
   }

   if (kasfEntryValueGet(154639956) < 2)
   {
       buildCost = kasfShipTypeBuildCost("bHiveFrigate");
       if (kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if (kasfEntryValueGet(154639956) > 1)
   {
       buildCost = kasfShipTypeBuildCost("bHiveFrigate");
       if (kasfRUsGet(1) < buildCost)
       {
           goto beginHiveBuildTimer;
       }
   }
   if (kasfEntryValueGet(154639956) == 2)
   {
       goto beginHiveBuildTimer;
   }
}

void Watch_Mission16_HiveFrigateReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *spawnSource;
   GrowSelection *spawnTarget;
   GrowSelection *createdHiveFrigates;
   sdword spawnIndex;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   spawnSource = kasGrowSelectionPtr(0xE05243);
   if (!kasfShipsCount(spawnSource))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnIndex = kasfRandom(0, kasfShipsCount(spawnSource) - 1);
       spawnTarget = kasGrowSelectionPtr(0x5425);
       kasfShipsSelectIndex(spawnTarget, spawnSource, spawnIndex);
       createdHiveFrigates = kasGrowSelectionPtr(0x9379E54);
       kasfShipsCreateInside(createdHiveFrigates, "bHiveFrigate", kasfEntryValueGet(154639956), spawnTarget, 1);
       kasAITeamShipsPtr(createdHiveFrigates, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdHiveFrigates);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)219517875);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_HiveFrigateCombatLoop,
           (sdword)Watch_Mission16_HiveFrigateReinforcementCooldown);
   }
}

void Watch_Mission16_HiveFrigateReinforcementCooldown(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_HiveFrigateCombatLoop,
           (sdword)Watch_Mission16_HiveFrigateReinforcementCooldown);
   }
}

void Init_Mission16_WorkerReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(208120516)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1893839148, 0);
       kasfEntryGetOrCreateSetValue(-1898467739, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasfSpeechEvent(
               208120516,
               (sdword)sub_617A10,
               (sdword)Watch_Mission16_WorkerHarvestReinforcementTeam);
       }
       else
       {
           kasfSpeechEvent(
               4693508,
               (sdword)sub_65F6F0,
               (sdword)Watch_Mission16_WorkerReinforcementTeam);
       }
   }
}

void Init_Mission16_WorkerReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(208120516))
    && kasfEntryValueGet(-1898467739)
    && kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1898467739, 0);
       kasfEntrySetValue(-1893839148, 1);
   }
   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)204436228);
       kasfSpeechEvent(
           4693508,
           (sdword)sub_65F6F0,
           (sdword)Watch_Mission16_WorkerReinforcementTeam);
   }
}

void Watch_Mission16_WorkerReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *returnedWorkers;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   returnedWorkers = kasGrowSelectionPtr(363040);
   if (kasfShipsSelectType(returnedWorkers, kasGrowSelectionPtr(44665972), "bWorker") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
       {
           if (kasfEntryValueGet(-1893839148))
           {
               kasfEntrySetValue(-1893839148, 0);
               kasfEntrySetValue(-1898467739, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Init_Mission16_WorkerReinforcementBuildTimer);
           }
       }
       else
       {
           kasfEntrySetValue(-1893839148, 0);
           kasfEntrySetValue(-1898467739, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(returnedWorkers, kasAITeamShipsPtrIfExists(44665972), "bWorker"))
   {
       kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedWorkers);
       kasfShipsStop(returnedWorkers);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedWorkers);
       kasfShipsSetSlaveableFlag(returnedWorkers, 0);
       kasfShipsApplyPlayerUpgrades(returnedWorkers);
       kasfReinforceTeamWithShips(team, returnedWorkers);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission16_WorkerHarvestReinforcementTeam);
}

void Init_Mission16_WorkerReinforcementBuildTimer(void)
{
   sdword buildScale;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildScale = kasfEntryValueGet(-2126662828);
   if (buildScale != 0)
   {
       buildScale--;
       if (buildScale != 0)
       {
           if (buildScale != 1)
           {
               return;
           }
           if (kasfEntryValueGet(154639956) < 2)
           {
               kasfEntryIncrement(154639956);
           }
checkWorkerBuildReady:
           if (kasfEntryValueGet(154639956) != 2)
           {
               return;
           }
           goto beginWorkerBuildTimer;
       }

       if (kasfEntryValueGet(154639956) < 2)
       {
           buildCost = kasfShipTypeBuildCost("bWorker") / 2;
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) <= 1)
       {
           goto checkWorkerBuildReady;
       }
       buildCost = kasfShipTypeBuildCost("bWorker") / 2;
       if (kasfRUsGet(1) >= buildCost)
       {
           goto checkWorkerBuildReady;
       }
beginWorkerBuildTimer:
       kasfSpeechEvent(
           75929221,
           (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
           (sdword)Watch_Mission16_WorkerReinforcementBuildTimer);
       return;
   }

   if (kasfEntryValueGet(154639956) < 2)
   {
       buildCost = kasfShipTypeBuildCost("bWorker");
       if (kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if (kasfEntryValueGet(154639956) > 1)
   {
       buildCost = kasfShipTypeBuildCost("bWorker");
       if (kasfRUsGet(1) < buildCost)
       {
           goto beginWorkerBuildTimer;
       }
   }
   if (kasfEntryValueGet(154639956) == 2)
   {
       goto beginWorkerBuildTimer;
   }
}

void Watch_Mission16_WorkerReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *spawnSource;
   GrowSelection *spawnTarget;
   GrowSelection *createdWorkers;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   spawnSource = kasGrowSelectionPtr(0xE05243);
   if (!kasfShipsCount(spawnSource))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnTarget = kasGrowSelectionPtr(21541);
       kasfShipsSelectIndex(spawnTarget, spawnSource, kasfRandom(0, kasfShipsCount(spawnSource) - 1));
       createdWorkers = kasGrowSelectionPtr(154639956);
       kasfShipsCreateInside(createdWorkers, "bWorker", kasfEntryValueGet(154639956), spawnTarget, 1);
       kasAITeamShipsPtr(createdWorkers, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdWorkers);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission16_WorkerHarvestReinforcementTeam);
   }
}

void Watch_Mission16_WorkerHarvestReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *returnedWorkers;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfGetWorldResources())
   {
       if (kasfShipsAnyIdle(&team->shipList))
       {
           kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission16_WorkerHarvestReinforcementTeam);
       }
   }
   else
   {
       returnedWorkers = kasGrowSelectionPtr(0x58A20);
       kasfGrowSelectionClear(returnedWorkers);
       kasfShipsAdd(returnedWorkers, &team->shipList);
       kasfShipsRemove(&team->shipList, returnedWorkers);
       kasfReinforceTeamWithShips(kasAITeamPtrIfExists(44665972), returnedWorkers);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
   }
}



void Watch_Mission16_BeastACVDockAndReinforce(void)
{
   sdword buildStyle;
   sdword buildLimit;
   sdword buildTime;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(
           4693508,
           (sdword)sub_65F6F0,
           (sdword)Watch_Mission16_BeastACVReinforcementTeam);
       return;
   }

   buildStyle = kasfEntryValueGet(-2126662828);
   switch (buildStyle)
   {
       case 2:
           buildLimit = 10 - kasfTeamCount();
           if (kasfEntryValueGet(154639956) < buildLimit)
           {
               kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bACV"));
               kasfEntryIncrement(154639956);
           }
           if (kasfEntryValueGet(154639956) == 10)
           {
               kasTimerCreateSetStartById(133682484, 0);
           }
           break;

       case 1:
           buildLimit = 10 - kasfTeamCount();
           if (kasfEntryValueGet(154639956) < buildLimit)
           {
               buildCost = kasfShipTypeBuildCost("bACV") / 2;
               if (kasfRUsGet(1) >= buildCost)
               {
                   kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                   kasfEntryIncrement(154639956);
               }
           }
           if ((kasfEntryValueGet(154639956) <= 2 || kasfRUsGet(1) >= kasfShipTypeBuildCost("bACV") / 2) &&
               kasfEntryValueGet(154639956) != 10)
           {
               break;
           }
           buildTime = kasfShipTypeBuildTime("bACV") / 2;
           kasTimerCreateSetStartById(133682484, kasfEntryValueGet(154639956) * buildTime);
           break;

       case 0:
           buildLimit = 10 - kasfTeamCount();
           if (kasfEntryValueGet(154639956) < buildLimit)
           {
               buildCost = kasfShipTypeBuildCost("bACV");
               if (kasfRUsGet(1) >= buildCost)
               {
                   kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                   kasfEntryIncrement(154639956);
               }
           }
           if ((kasfEntryValueGet(154639956) <= 2 || kasfRUsGet(1) >= kasfShipTypeBuildCost("bACV")) &&
               kasfEntryValueGet(154639956) != 10)
           {
               break;
           }
           buildTime = kasfShipTypeBuildTime("bACV");
           kasTimerCreateSetStartById(133682484, kasfEntryValueGet(154639956) * buildTime);
           break;
   }

   if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
   {
       kasfShipsSelectLinkedWith(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr((char *)316548), kasGrowSelectionPtr((char *)165711363));
   }
}

sdword Init_Mission16_DockForReinforcements(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfEntrySetValue(332667, 1);
   kasfEntrySetValue(154639956, 0);
   kasTimerDestroyById(133682484);
   kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)165711363), kasThisTeamsVectorPtr());
   kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), kasGrowSelectionPtr((char *)165711363), 0);
   kasfFormationDelta3D();
   return kasfShipsDockWithSelection(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr((char *)21541));
}

void Watch_Mission16_BeastCloakedFighterDockAndReinforce(void)
{
    AITeam *team;
    sdword buildScale;
    sdword buildLimit;
    sdword buildCost;
    sdword buildTime;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        return;
    }

    buildScale = kasfEntryValueGet(-2126662828);
    buildLimit = 6 - kasfTeamCount();
    if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 6)
        {
            kasTimerCreateSetStartById(133682484, 0);
        }
    }
    else if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter")) ||
            kasfEntryValueGet(154639956) == 6)
        {
            buildTime = kasfShipTypeBuildTime("bCloakedFighter");
            kasTimerCreateSetStartById(133682484, buildTime * kasfEntryValueGet(154639956) / 2);
        }
    }
    else if (buildScale == 0)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter");
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter")) ||
            kasfEntryValueGet(154639956) == 6)
        {
            buildTime = kasfShipTypeBuildTime("bCloakedFighter");
            kasTimerCreateSetStartById(133682484, kasfEntryValueGet(154639956) * buildTime);
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr((char *)316548), kasGrowSelectionPtr((char *)165711363));
        createdShips = kasGrowSelectionPtr((char *)154639956);
        kasfShipsCreate(createdShips, "bCloakedFighter", kasfEntryValueGet(154639956), 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfShipsDockInstant(createdShips, kasGrowSelectionPtr((char *)316548));
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfSpeechEvent(84320888, (sdword)Init_LaunchDockedTeam, 0);
    }
}

void Watch_Mission16_BeastCloakedFighterLaunchDockedReinforcements(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        return;
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0BC8AAC3);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(337061, (sdword)Begin_BeastCloakedFighterCombatLoop, 0);
    }
}

void Watch_Mission16_EliteGuardCarrier_WaitForMessage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(0x443CCA9))
    {
        kasfEntryGetOrCreateSetValue(0x443CCA9, 1);
        kasLabelledVectorSet((char *)0x04A78474, kasVectorPtrIfExists((char *)0x052671));
    }
    if (kasfEntryValueGet(0x88678C4))
    {
        kasfMoveTo(kasVectorPtrIfExists((char *)0x04A78474));
    }
    kasTimerCreateSetStartById(0x05DB194, kasfRandom(5, 10));
}

void Watch_Mission16_BeastDFGFrigateReinforcementSpawn(GrowSelection *createdShips)
{
    kasfReinforceTeamWithShips(teamtoreinforce, createdShips);
    kasfShipsAdd(kasGrowSelectionPtr((char *)teamtoreinforce->kasFSMNameHash), &teamtoreinforce->shipList);
    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists((char *)teamtoreinforce->kasFSMNameHash), (char *)44796563);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
    }
}

void Watch_Mission16_BeastDFGFrigateDockAndReinforce(void)
{
    AITeam *team;
    sdword buildScale;
    sdword buildLimit;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    buildScale = kasfEntryValueGet(-2126662828);
    buildLimit = 2 - kasfTeamCount();
    if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bDFGFrigate") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate") / 2) ||
            kasfEntryValueGet(154639956) == 2)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfShipTypeBuildTime("bDFGFrigate") * kasfEntryValueGet(154639956) / 2);
        }
    }
    else if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 2)
        {
            kasTimerCreateSetStartById(133682484, 0);
        }
    }
    else if (buildScale == 0)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bDFGFrigate");
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate")) ||
            kasfEntryValueGet(154639956) == 2)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bDFGFrigate"));
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr((char *)316548), kasGrowSelectionPtr((char *)165711363));
    }
}

void Begin_Mission16_BeastHeavyCruiserAttackStaging(void)
{
    AITeam *team;
    GrowSelection *spawnParents;
    GrowSelection *spawnPointSelection;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        spawnParents = kasGrowSelectionPtr((char *)165711363);
        targetIndex = kasfRandom(0, kasfShipsCount(spawnParents) - 1);
        spawnPointSelection = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnPointSelection, spawnParents, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            kasTimerCreateSetStartById(23446965, 8);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnPointSelection);
            kasfShipsSetMilitaryParade(spawnPointSelection, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)185462083);
        kasfEntrySetValue(332667, 0);
        kasTimerSetById(142843908, 20);
        kasTimerStartById(142843908);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, 0);
    }
}

void Init_Mission16_ReinforcementTriggerRouter(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
}

void Watch_Mission16_ReinforcementTriggerRouter_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
    }
}

void Watch_Mission16_ReinforcementTriggerRouter(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        switch (message)
        {
            case 35692611:
                kasfMsgSend(kasAITeamPtrIfExists(62813829), (char *)4693508);
                break;
            case 104569475:
                kasfMsgSend(kasAITeamPtrIfExists(200156578), (char *)4693508);
                break;
            case 132952961:
                kasfEntryGetOrCreateSetValue(-2126662828, 2);
                kasfMsgSend(kasAITeamPtrIfExists(17798), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(171501548), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(186735637), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(62813829), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(200156578), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(223105365), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(5675070), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(18599), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(212896882), (char *)4693508);
                if (GetGameDifficulty())
                {
                    kasfMsgSend(team, (char *)132952962);
                }
                kasfSpeechEvent(374244, (sdword)Init_Mission05_BeastWaveResumeDelay, 0);
                return;
            case 132952962:
                kasfMsgSend(kasAITeamPtrIfExists(171501548), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                kasfMsgSend(team, (char *)132952963);
                kasfSpeechEvent(374244, (sdword)Init_Mission05_BeastWaveResumeDelay, 0);
                return;
            case 132952963:
                kasfEntryGetOrCreateSetValue(-2126662828, 0);
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                kasfSpeechEvent(374244, (sdword)Init_Mission05_BeastWaveResumeDelay, 0);
                return;
            case 154145491:
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                break;
            case 158207907:
                kasfMsgSend(kasAITeamPtrIfExists(5675070), (char *)4693508);
                break;
            case 188751475:
                kasfMsgSend(kasAITeamPtrIfExists(212896882), (char *)4693508);
                break;
            case 190169075:
                kasfMsgSend(kasAITeamPtrIfExists(171501548), (char *)4693508);
                break;
            case 203347251:
                kasfMsgSend(kasAITeamPtrIfExists(223105365), (char *)4693508);
                break;
            case 204436228:
                kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
                break;
            case 217984405:
                kasTimerCreateSetStartById(10270786, 12);
                if (kasfRUsGet(1) < 8000)
                {
                    kasfRUsSet(1, 8000);
                }
                kasFSMCreate(
                    (char *)0x0C67AAC4,
                    Init_Mission16_WorkerReinforcementTeam,
                    Init_Mission16_WorkerReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(108702867));
                kasFSMCreate(
                    (char *)0x0458B3,
                    Init_Mission16_BeastACVReinforcementTeam,
                    Init_Mission16_BeastACVReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(284786));
                kasFSMCreate(
                    (char *)0x038E7FB3,
                    Init_Mission16_SentinelReinforcementTeam,
                    Init_Mission16_SentinelReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(59670131));
                kasFSMCreate(
                    (char *)11133235,
                    Init_Mission16_BeastIonArrayFrigateReinforcementTeam,
                    Init_Mission16_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(11133426));
                kasFSMCreate(
                    (char *)0x0215C113,
                    Init_Mission16_HiveFrigateReinforcementTeam,
                    Init_Mission16_HiveFrigateReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(34980306));
                kasFSMCreate(
                    (char *)0x0BE76893,
                    Init_Mission16_BeastMultiBeamFrigateReinforcementTeam,
                    Init_Mission16_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(199714898));
                kasFSMCreate(
                    (char *)0x07AAD0E3,
                    Init_Mission16_BeastCorvetteReinforcementTeam,
                    Init_Mission16_BeastCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(128634915));
                kasFSMCreate(
                    (char *)0x07AAD0E3,
                    Init_Mission16_BeastCorvetteReinforcementTeam,
                    Init_Mission16_BeastCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(128634916));
                kasFSMCreate(
                    (char *)0x0DBD8ED3,
                    Init_Mission16_BeastAcolyteReinforcementTeam,
                    Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(230526612));
                kasFSMCreate(
                    (char *)0x0DBD8ED3,
                    Init_Mission16_BeastAcolyteReinforcementTeam,
                    Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(230526613));
                kasFSMCreate(
                    (char *)0x04C51573,
                    Init_Mission16_BeastRammingFrigateReinforcementTeam,
                    Init_Mission16_BeastRammingFrigateReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(80025009));
                kasFSMCreate(
                    (char *)0x05698433,
                    Init_Mission16_BeastReconReinforcementTeam,
                    Init_Mission16_BeastReconReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(90801139));
                kasFSMCreate(
                    (char *)0x0B08C7B3,
                    Init_Mission16_BeastHeavyCruiserReinforcementTeam,
                    Init_Mission16_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(185124850));
                break;
            case 221638067:
                kasfMsgSend(kasAITeamPtrIfExists(151690773), (char *)4693508);
                break;
            case 222530862:
                kasFSMCreate(
                    (char *)0x0458B3,
                    Init_Mission16_BeastACVReinforcementTeam,
                    Init_Mission16_BeastACVReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(284785));
                kasFSMCreate(
                    (char *)0x038E7FB3,
                    Init_Mission16_SentinelReinforcementTeam,
                    Init_Mission16_SentinelReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(59670129));
                kasFSMCreate(
                    (char *)0x0215C113,
                    Init_Mission16_HiveFrigateReinforcementTeam,
                    Init_Mission16_HiveFrigateReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(34980305));
                kasFSMCreate(
                    (char *)0x0BE76893,
                    Init_Mission16_BeastMultiBeamFrigateReinforcementTeam,
                    Init_Mission16_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(199714897));
                kasFSMCreate(
                    (char *)0x07AAD0E3,
                    Init_Mission16_BeastCorvetteReinforcementTeam,
                    Init_Mission16_BeastCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(128634913));
                kasFSMCreate(
                    (char *)0x0DBD8ED3,
                    Init_Mission16_BeastAcolyteReinforcementTeam,
                    Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(230526609));
                kasFSMCreate(
                    (char *)0x0DBD8ED3,
                    Init_Mission16_BeastAcolyteReinforcementTeam,
                    Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(230526610));
                kasTimerCreateSetStartById(10270786, 120);
                break;
            case 228088883:
                kasfMsgSend(kasAITeamPtrIfExists(17798), (char *)4693508);
                break;
            case 228101699:
                kasfMsgSend(kasAITeamPtrIfExists(18599), (char *)4693508);
                break;
            case 236014723:
                kasfMsgSend(kasAITeamPtrIfExists(186735637), (char *)4693508);
                break;
            case 259199139:
                kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
                break;
            default:
                break;
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }

    if (kasTimerExpiredById(10270786))
    {
        kasfMsgSend(team, (char *)132952961);
    }
}

void Watch_Mission16_Events_HoneyImHome(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x4A2A29), kasShipsVectorPtr((char *)0x4E20), 0x4E20))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x4A2A29));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr((char *)0x8C46FF2), kasGrowSelectionPtr((char *)0x4A2A29), "sMothershipBigGun"))
    {
        kasfShipsRemove(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x8C46FF2));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x857BCE3));
    }
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x36980EB), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x36980EB));
    kasTimerDestroyById(0x469857B);
    kasTimerCreateSetStartById(0x469857B, kasfRandom(30, 50));
}

void Init_Mission16_Events_HoneyImHome(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfMsgReceived((char *)0x588674))
    {
        kasfEntrySetValue(0x5137B, 0);
        kasfTeamHyperspaceInNear(kasShipsVectorPtrIfExists((char *)0x0CFF5860), 30000);
        kasfSpeechEvent(0x469857B, (sdword)Watch_Mission16_Events_HoneyImHome, (sdword)Watch_Mission16_Events_HoneyImHomeRetryDelay);
    }
}

void Watch_Mission16_Events_HoneyImHomeRetryDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfShipsAnyIdle(&teamtoreinforce->shipList) || kasTimerExpiredDestroyById(0x469857B))
    {
        kasTimerDestroyById(0x469857B);
        kasfSpeechEvent(0x469857B, (sdword)Watch_Mission16_Events_HoneyImHome, (sdword)Watch_Mission16_Events_HoneyImHomeRetryDelay);
    }
}

void Watch_Mission16_MissionFlow(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = teamtoreinforce;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        switch (message)
        {
            case 0:
                break;
            case 0x010C95FE:
                kasfSpeechEventQueueCurrentPlayer(268444290, 0);
                kasfEntrySetValue(77979796, 1);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x024522E2:
                kasfSpeechEventQueueCurrentPlayer(268444296, 0);
                kasfEntrySetValue(127756738, 300);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x005B6A62);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(172907219, 1);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x035D878E:
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)70832674);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission16_MissionFlow_Delay);
                return;
            case 0x056C4702:
                kasfSpeechEventQueueCurrentPlayer(268440193, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)242875788);
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(134850334));
                kasfCameraSetAngleDeg(224);
                kasfCameraSetDeclinationDeg(-32);
                kasfCameraSetDistance(325);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x057F377E:
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)98072770);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission16_MissionFlow_Delay);
                return;
            case 0x0588674:
                kasfMsgSend(team, (char *)92825426);
                spSetSinglePlayerSaveInfo(16, m16_saveInfoName[strCurLanguage]);
                break;
            case 0x05A4EA36:
                if (!kasfEntryValueGet(-2141371705))
                {
                    kasfMsgSend(kasAITeamPtrIfExists(127538865), (char *)90982146);
                }
                kasfSpeechEventQueueCurrentPlayer(268444287, 0);
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(134850334));
                kasfCameraSetAngleDeg(220);
                kasfCameraSetDeclinationDeg(14);
                kasfCameraSetDistance(500);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(266064926, 1);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)173859854);
                kasfEntrySetValue(127756738, 10);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x05A4EA37:
                kasfSpeechEventQueueCurrentPlayer(268444291, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)145996014);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(266064926, 1);
                kasfMsgSend(team, (char *)83293698);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x060DF8CE:
                kasfSpeechEventQueueCurrentPlayer(268440199, 0);
                kasfMsgSend(team, (char *)181736389);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x072AD9B4:
                kasfSpeechEventQueueCurrentPlayer(268440202, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)169481154);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x0A5CE40E:
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(0);
                    kasfEntrySetValue(126773838, 0);
                }
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfTutSetPointerTargetFirstShip((char *)0x8CBB10, kasAITeamShipsPtrIfExists(216029280));
                kasfPingAddShips(kasAITeamShipsPtrIfExists(216029280), "Ping");
                kasfSensorsSetLookatPoint(kasTeamsVectorPtr(216029280));
                kasfSpeechEventQueueCurrentPlayer(268444297, 0);
                kasfMsgSendAll((char *)217984405);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(168240451, 1);
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x0AD513C5:
                kasfEntrySetValue(-2054310428, 0);
                kasfOpenSensors(100000);
                kasfPingAddShips(kasAITeamShipsPtrIfExists(134850334), "Ping");
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(126773838, 0);
                kasfSpeechEventQueueCurrentPlayer(268444286, 0);
                soundStopDSound("CleeSanPointer", kasAITeamShipsPtrIfExists(134850334));
                kasfSensorsSetLookatPoint(kasTeamsVectorPtr(134850334));
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(168240451, 1);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)92223358);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x0B2454FE:
                kasfSpeechEventQueueCurrentPlayer(268444299, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x0C1CF74:
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)191522898);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission16_MissionFlow_Delay);
                return;
            case 0x0CFE2D95:
                kasfEntrySetValue(127756738, 2);
                kasfMsgSend(team, (char *)223894178);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission16_MissionFlow_Delay);
                return;
            case 0x0D438D2E:
                kasfSpeechEventQueueCurrentPlayer(270533252, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)169481154);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 0x0D585AA2:
                kasfMsgSend(kasAITeamPtrIfExists(134326499), (char *)84320888);
                kasfMsgSend(kasAITeamPtrIfExists(134850334), (char *)181736389);
                kasfMsgSend(team, (char *)115162707);
                kasfEntrySetValue(127756738, 5);
                break;
            case 169481154:
                kasfSpeechEventQueueCurrentPlayer(268440197, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)59045045);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 59045045:
                kasfSpeechEventQueueCurrentPlayer(270533254, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)128009460);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission16_WaitForIntelEventEnd);
                return;
            case 83293698:
                kasfMsgSend(kasAITeamPtrIfExists(134850334), (char *)83293698);
                break;
            case 38096706:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(134326499));
                kasfCameraSetAngleDeg(326);
                kasfCameraSetDeclinationDeg(15);
                kasfCameraSetDistance(200);
                break;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        if (team->msgQueue == NULL || team->msgQueue->head == 0)
        {
            return;
        }
        queueIndex--;
    }
}

void Init_Mission16_ObjectiveFlow(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(0x01C441B2, 900);
    kasfSpeechEvent(0x005B6A61, (sdword)sub_652EF0, (sdword)Watch_Mission16_ObjectiveFlow);
}

static void Watch_Mission16_DestroyBeastObjective(void)
{
    GrowSelection *remainingParadeShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    remainingParadeShips = kasGrowSelectionPtr((char *)0x593FE0C);
    kasfGrowSelectionClear(remainingParadeShips);
    kasfShipsAdd(remainingParadeShips, kasGrowSelectionPtr((char *)0x6DA2DA9));
    kasfShipsSelectType(kasGrowSelectionPtr((char *)0x58A20), remainingParadeShips, "bWorker");
    kasfShipsRemove(remainingParadeShips, kasGrowSelectionPtr((char *)0x58A20));
    kasfShipsSelectType(kasGrowSelectionPtr((char *)0x58A20), remainingParadeShips, "bProcessor");
    if (!kasfEntryValueGet(0x443CCA2) && !kasfShipsCount(remainingParadeShips))
    {
        kasfEntryGetOrCreateSetValue(0x443CCA2, 1);
        kasfObjectiveDestroy("DESTROY_BEAST");
        kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x00C1CF74);
    }
    if (kasfMsgReceived((char *)0x0A7C9B35))
    {
        kasfObjectiveCreate("Hyperspace", m16_hyperspaceObjectiveName[strCurLanguage], m16_hyperspaceObjectiveDescription[strCurLanguage]);
        spSetSinglePlayerSaveInfo(16, m16_endMissionSaveInfoName[strCurLanguage]);
    }
}

void Watch_Mission16_ObjectiveFlow(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        switch (message)
        {
            case 0x035D878E:
                kasfObjectiveDestroy("CRIPPLE_CS");
                kasfObjectiveDestroy("LOCATE_CS");
                break;
            case 0x057F377E:
                kasfObjectiveCreate("LOCATE_CS", m16_locateCsObjectiveTitle[strCurLanguage], m16_locateCsObjectiveDescription[strCurLanguage]);
                kasfObjectiveSet("LOCATE_CS", 0);
                break;
            case 0x05B6A62:
                kasfObjectiveDestroy("HARVEST");
                kasfObjectiveCreate("DESTROY_BEAST", m16_destroyBeastObjectiveTitle[strCurLanguage], m16_destroyBeastObjectiveDescription[strCurLanguage]);
                kasfObjectiveSet("DESTROY_BEAST", 0);
                kasTimerCreateSetStartById(0x02E44642, 15);
                break;
            case 0x08B3B8EE:
                kasfObjectiveCreate("DOCKWITH_CS", m16_dockWithCsObjectiveTitle[strCurLanguage], m16_dockWithCsObjectiveDescription[strCurLanguage]);
                kasfObjectiveSet("DOCKWITH_CS", 0);
                break;
            case 0x0A5CE40E:
                kasfObjectiveCreate("CRIPPLE_CS", m16_crippleCsObjectiveTitle[strCurLanguage], m16_crippleCsObjectiveDescription[strCurLanguage]);
                kasfObjectiveSet("CRIPPLE_CS", 0);
                break;
            case 0x0B2454FE:
                kasfObjectiveDestroy("DOCKWITH_CS");
                kasfObjectiveCreate("HARVEST", m16_harvestObjectiveTitle[strCurLanguage], m16_harvestObjectiveDescription[strCurLanguage]);
                break;
            case 0x0D438D2E:
                kasfObjectiveDestroy("LOCATE_CS");
                break;
            default:
                break;
        }

        if (team->msgQueue == NULL || team->msgQueue->head == 0)
        {
            break;
        }
        queueIndex--;
    }

    if (kasTimerExpiredById(0x01C441B2) && !kasfEntryValueGet(0x805D42C7))
    {
        kasfEntryGetOrCreateSetValue(0x805D42C7, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0x0469684), (char *)0x0D438D2E);
        kasfMsgSend(kasAITeamPtrIfExists(0x079A16B1), (char *)0x056C4702);
    }
    if (kasTimerExpiredById(0x02E44642))
    {
        kasfSpeechEvent(0x05B6A62, (sdword)sub_652EF0, (sdword)Watch_Mission16_DestroyBeastObjective);
    }
}

void Watch_Mission16_LocateCS(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    if (!kasfEntryValueGet(0x443CCA3) &&
        kasfShipsAnyInSensorRange(&team->shipList) &&
        !kasfEntryValueGet(0x0BCA7444))
    {
        kasfEntryGetOrCreateSetValue(0x443CCA3, 1);
        kasfEntryGetOrCreateSetValue(0x04B4A24, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x0D438D2E);
        kasfMsgSend(kasAITeamPtrIfExists(0x098DDC43), (char *)0x0D438D2E);
        if (!kasfMsgReceived((char *)0x805D42C7))
        {
            kasfEntrySetValue(0x805D42C7, 1);
            kasfMsgSend(kasAITeamPtrIfExists(0x0469684), (char *)0x0D438D2E);
        }
    }

    if (kasfTeamAttributesAnySet(0x20000))
    {
        kasfEntryGetOrCreateSetValue(0x0BCA7444, 1);
        kasfTeamAttributesBitClear(0x10000);
        if (kasfFindDistance(kasShipsVectorPtrIfExists((char *)0x0CA25860), &team->shipList) <= 35000.0f)
        {
            kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x010C95FE);
            kasfVarDestroy(&team->shipList);
            kasfSpeechEvent(0x048954, (sdword)sub_652EF0, (sdword)sub_652EF0);
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x035D878E);
            kasfMsgSend(kasAITeamPtrIfExists(0x098DDC43), (char *)0x035D878E);
            kasfSpeechEvent(0x08615BD4, (sdword)Init_Mission16_LocateCS_SetTeamPercentDamaged75, (sdword)Watch_Mission16_WorkerDockAndReinforce);
        }
    }
}

void Watch_Mission16_WorkerDockAndReinforce(void)
{
    AITeam *team;
    GrowSelection *nearbyEnemies;
    GrowSelection *workerSelection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    if (kasfTeamHealthAverage() >= 22)
    {
        kasfTeamSetPercentDamaged(kasfTeamHealthAverage() - 1);
    }

    if (!kasfEntryValueGet(0x443CCA4))
    {
        nearbyEnemies = kasGrowSelectionPtr((char *)0x9678C7D);
        if (kasfFindEnemiesNearCurrentTeam(nearbyEnemies, 1500) &&
            kasfShipsSelectType(kasGrowSelectionPtr((char *)0x77A0A6D), nearbyEnemies, "sWorker"))
        {
            kasfEntryGetOrCreateSetValue(0x443CCA4, 1);
            workerSelection = kasGrowSelectionPtr((char *)0x77A0A6D);
            kasfShipsSelectFirst(workerSelection, workerSelection);
            kasfShipsSwitchPlayerOwner(workerSelection, 0);
            kasfShipsSetDontApplyPhysicsFlag(workerSelection, 1);
            kasfReinforceTeamWithShips(kasAITeamPtrIfExists(0x0801A8E3), workerSelection);
            kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x0B2454FE);
            kasfMsgSend(kasAITeamPtrIfExists(0x098DDC43), (char *)0x0B2454FE);
        }
    }

    if (kasfMsgReceived((char *)0x0AD513C5))
    {
        kasTimerCreateSetStartById(0x0C059092, 3);
    }
    if (kasTimerExpiredById(0x0C059092))
    {
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
        kasfVarDestroy(&team->shipList);
        kasfSpeechEvent(0x048954, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Begin_Mission16_DockWithCS(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfShipsSetRotatableFlag(kasAITeamShipsPtrIfExists(134326499), 1);
    kasfShipsDockSupportWith(kasAITeamShipsPtrIfExists(134326499), kasAITeamShipsPtrIfExists(134850334));
}

void Watch_Mission16_DockedReadyForLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(0x443CCA5) && kasfTeamDockedReadyForLaunch())
    {
        kasfEntryGetOrCreateSetValue(0x443CCA5, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x0FD585A4);
    }
    if (kasfMsgReceived((char *)0x0506A278))
    {
        kasfSpeechEvent(0x0A279E47, (sdword)Init_Mission16_LaunchDockedTeam, (sdword)sub_652EF0);
    }
}

void Init_Mission16_LaunchDockedTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfShipsSetRotatableFlag(kasAITeamShipsPtrIfExists(134326499), 0);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(134326499), 0);
    kasfShipsSwitchPlayerOwner(kasAITeamShipsPtrIfExists(134326499), 1);
    kasfShipsDockSupportWith(kasAITeamShipsPtrIfExists(134326499), kasGrowSelectionPtr((char *)211966048));
}

void Init_Mission16_MothershipAttackTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamAttributesBitSet(0x80);
    kasfShipsSetMaxVelocity(&((AITeam *)kasCurrentTeamPtr)->shipList, 10);
    kasfAttackMothership();
}

void Watch_Mission16_MothershipAttackTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0CFE2D95))
    {
        kasfTeamHyperspaceInNear(kasVectorPtrIfExists((char *)0x6968893), 4000);
        kasTimerCreateSetStartById(0x0788CB82, 20);
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0469684), kasAITeamShipsPtrIfExists(0x0CE05860));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x09E08E03), kasAITeamShipsPtrIfExists(0x0CE05860));
    }
    if (kasTimerExpiredById(0x0788CB82))
    {
        kasfTacticsAggressive();
        kasfMsgSend(kasAITeamPtrIfExists(0x0E87E571), (char *)0x08700AD8);
    }
    if (!kasfEntryValueGet(0x443CCA6) && kasfTeamAttributesAnySet(0x20000))
    {
        kasfEntryGetOrCreateSetValue(0x443CCA6, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0x0A8A880C), (char *)0x086A9EB4);
    }
}

void Init_Mission16_ReinforcementPool(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfSpeechEvent(0x0469857B, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, (sdword)Watch_Mission16_ReinforcementPool_Combat);
}

void Fill_Mission16_ReinforcementPoolFromParade(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfShipsSelectSinglePlayerInParade(kasGrowSelectionPtr((char *)143246580), kasGrowSelectionPtr((char *)114961833)))
    {
        kasfReinforceTeamWithShips(teamtoreinforce, kasGrowSelectionPtr((char *)143246580));
    }
}

void Watch_Mission16_ReinforcementPool_Combat(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x053A0C, (sdword)sub_652EF0, (sdword)Watch_Mission16_ReinforcementPool_Resume);
    }
    else if (kasfShipsAnyIdle(&teamtoreinforce->shipList))
    {
        kasfSpeechEvent(0x0469857B, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, (sdword)Watch_Mission16_ReinforcementPool_Combat);
    }
}

void Watch_Mission16_ReinforcementPool_Resume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(0x0469857B, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, (sdword)Watch_Mission16_ReinforcementPool_Combat);
    }
}

signed int Begin_Mission16_BeastDFGFrigateDockAndReinforce(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)165711363), kasShipsVectorPtr((char *)teamtoreinforce->kasFSMNameHash));
    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), kasGrowSelectionPtr((char *)165711363), 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    return kasfShipsDockWithSelection(&teamtoreinforce->shipList, kasGrowSelectionPtr((char *)21541));
}

void Watch_Mission16_BeastCorvetteCapitalStrike(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, 0);
    }
}

void Watch_Mission16_BeastHeavyCruiserSpecialAttack(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyBigGunReady(&team->shipList))
        {
            if (kasfShipsAnyIdle(&team->shipList))
            {
                kasfSpeechEvent(157845977, (sdword)Begin_Mission16_BeastHeavyCruiserSpecialAttack, 0);
            }
        }
        else
        {
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, 0);
        }
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
    }
}

void Watch_Mission16_TriggerCrippleCSObjective(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552161) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)0x0CA25860)) &&
        kasfEntryValueGet(-2015429404))
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x0D2247A4);
    }
}

void Init_Mission16_LocateCS(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamAttributesBitSet(0x10000);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfSpeechEvent(87322972, (sdword)MaintainTeamContext, 0);
}

void Init_Mission16_LocateCS_SetTeamPercentDamaged75(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamSetPercentDamaged(75);
}

void Begin_Mission16_BeastHeavyCruiserSpecialAttack(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamSpecial(kasGrowSelectionPtr(57245931));
}

void Init_Mission16_BeastHeavyCruiserReinforcementTeam_NoSourceShips(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(-2110057163, 1);
    kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
}

void Init_Mission16_ReinforcementTriggerRouter_RecordJumpPosition(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x05137B, 0);
    kasLabelledVectorSet((char *)0x0E451D4, kasShipsVectorPtr((char *)0x05137B));
    kasfSpeechEvent(0x05B5E4, (sdword)sub_652EF0, (sdword)Watch_Mission16_ReinforcementTriggerRouter_AwaitJumpClearance);
}

void Watch_Mission16_ReinforcementTriggerRouter_AwaitJumpClearance(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfNearby(kasVectorPtrIfExists((char *)0x0E451D4), 5000))
    {
        kasfSpeechEvent(0x0524A5, (sdword)Begin_Mission16_ReinforcementTriggerRouter_ReturnToRecordedPosition, (sdword)Watch_Mission16_ReinforcementTriggerRouter_AwaitReturnToRecordedPosition);
    }
}

void Begin_Mission16_ReinforcementTriggerRouter_ReturnToRecordedPosition(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfMoveTo(kasVectorPtrIfExists((char *)0x0E451D4));
}

void Watch_Mission16_ReinforcementTriggerRouter_AwaitReturnToRecordedPosition(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfNearby(kasVectorPtrIfExists((char *)0x0E451D4), 5000))
    {
        kasfSpeechEvent(0x05B5E4, (sdword)sub_652EF0, (sdword)Watch_Mission16_ReinforcementTriggerRouter_AwaitJumpClearance);
        return;
    }
    if ((kasfShipsOrder(&((AITeam *)kasCurrentTeamPtr)->shipList) & COMMAND_MOVE) == 0)
    {
        kasfSpeechEvent(0x0524A5, (sdword)Begin_Mission16_ReinforcementTriggerRouter_ReturnToRecordedPosition, (sdword)Watch_Mission16_ReinforcementTriggerRouter_AwaitReturnToRecordedPosition);
    }
}

static void Watch_Mission16_DockWithCS_AwaitReady(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(0x0937FE67, (sdword)Begin_Mission16_DockWithCS, (sdword)Watch_Mission16_DockedReadyForLaunch);
    }
}

void Init_Mission16_DockWithCS(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(0x05B5E4, (sdword)sub_652EF0, (sdword)Watch_Mission16_DockWithCS_AwaitReady);
}

void Init_Mission16_CrippleCS(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(0x091C5F69, (sdword)sub_652EF0, (sdword)Watch_Mission16_CrippleCSAwaitSignal);
}

void Watch_Mission16_CrippleCSAwaitSignal(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x5296679),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x1F40),
            8000))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x5296679));
    }

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message == 0x056C4702)
        {
            kasfSpeechEvent(0x0427D112, (sdword)Init_Mission16_CrippleCSRefreshNearbySelection, (sdword)Watch_Mission16_CrippleCSRefreshNearbySelection);
            return;
        }
        if (message != 0)
        {
            aitQueuePointer(team, team, (void *)message);
        }
        if (team->msgQueue == NULL || team->msgQueue->head == 0)
        {
            return;
        }
        queueIndex--;
    }
}

void Init_Mission16_CrippleCSRefreshNearbySelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTacticsAggressive();
    kasfFormationDelta3D();
    kasTimerCreateSetStartById(0x084E59F2, 1);
}

void Watch_Mission16_CrippleCSRefreshNearbySelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(0x084E59F2))
    {
        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x5296679),
                kasGrowSelectionPtr((char *)0x4A2A29),
                kasShipsVectorPtrIfExists((char *)0x0CA25860),
                15000))
        {
            tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x5296679));
        }
        kasTimerSetById(0x084E59F2, 8);
        kasTimerStartById(0x084E59F2);
    }
}

/*-----------------------------------------------------------------------------
    Init_Mission16 - top-level KAS entry point (IDA: 0x647c90)
-----------------------------------------------------------------------------*/

extern void Watch_BeastReconWave_AwaitSignal(void);
extern void Watch_Mission02_AttackMothershipTeam(void);

void Init_Mission16(void)
{
    kasCurrentStep = 0;
    scope = KH_mission16;

    kasfDisableAllAIFeatures();
    kasfActivateTech("TECH_INFECT");
    kasfActivateTech("TECH_ADVANCED_SENSORS");
    kasfActivateTech("TECH_FORCE_FIELD1");
    kasfActivateTech("TECH_FORCE_FIELD2");
    kasfActivateTech("TECH_FORCE_FIELD3");
    kasfActivateTech("TECH_ENERGY");
    kasfActivateTech("TECH_CRYSTAL_HARVEST");
    kasfActivateTech("TECH_EMP");
    kasfDisablePlayerHyperspace();
    kasfLocationCard(8000, "Beast Command Ship Fleet");

    kasFSMCreate(KH_FleetIntel, Init_Mission16_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Init_Mission16_ObjectiveFlow, Watch_Mission16_TriggerCrippleCSObjective,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(134850334, Init_Mission16_LocateCS, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(134850334));
    kasFSMCreate(179233620, Init_Mission16_CrippleCS, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(127538865));
    kasFSMCreate(113255168, Init_Mission16_MothershipAttackTeam, Watch_Mission16_MothershipAttackTeam,
                 kasAITeamPtrIfExists(216029280));
    kasFSMCreate(135358963, Init_Mission16_CruiseMissileLauncher, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(243787121));
    kasFSMCreate(108585267, Watch_NoOpTeamContext, Watch_BeastReconWave_AwaitSignal,
                 kasAITeamPtrIfExists(92410244));
    kasFSMCreate(44665972, Init_Mission16_ReinforcementPool, Fill_Mission16_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(44665972));

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsRemove(kasGrowSelectionPtr(18336307), kasAITeamShipsPtrIfExists(132677969));
    kasfShipsRemove(kasGrowSelectionPtr(18336307), kasAITeamShipsPtrIfExists(132677970));
    kasfShipsRemove(kasGrowSelectionPtr(18336307), kasAITeamShipsPtrIfExists(132677971));
    kasfShipsRemove(kasGrowSelectionPtr(18336307), kasAITeamShipsPtrIfExists(132677972));
    kasfShipsSelectMissionVolumeEnemyCombatShips(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeInactivePlayerModules(kasGrowSelectionPtr(KH_Enemy), kasGrowSelectionPtr(18336307));
    kasfShipsSelectType(kasGrowSelectionPtr(134850334), kasGrowSelectionPtr(114961833), "dEcclesiamInfected");

    kasfRUsSet(1, 10000);
    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_SALVAGE");
    kasfAllowPlayerToResearch("TECH_REPAIR");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_RESEARCH_CLEESAN_ATTACK");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
    kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");
    kasfAllowPlayerToResearch("TECH_CRYSTAL_HARVEST");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");
    kasfAllowPlayerToResearch("TECH_ARMOUR2");
    kasfAllowPlayerToResearch("TECH_EMP");
    kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
    kasfAllowPlayerToResearch("TECH_ENERGY");
    kasfAllowPlayerToResearch("TECH_ION");
    kasfAllowPlayerToResearch("TECH_MISSILE");
    kasfAllowPlayerToResearch("TECH_PHASED_TELEMETRY");
    kasfAllowPlayerToResearch("TECH_ADVANCED_SENSORS");
    kasfAllowPlayerToResearch("TECH_BIG_GUN");
    kasfAllowPlayerToResearch("TECH_REPAIRBOTS");
    kasfAllowPlayerToResearch("TECH_DRIVE_DESTROYER");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD3");

    kasFSMCreate(4627076, Init_Mission16_ReinforcementTriggerRouter, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(4627076));

    kasFSMCreate(208120516, Init_Mission16_WorkerReinforcementTeam, Init_Mission16_WorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(17798));
    kasFSMCreate(208120516, Init_Mission16_WorkerReinforcementTeam, Init_Mission16_WorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(108702865));
    kasFSMCreate(208120516, Init_Mission16_WorkerReinforcementTeam, Init_Mission16_WorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(108702866));

    kasFSMCreate(284851, Init_Mission16_BeastACVReinforcementTeam, Init_Mission16_BeastACVReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(17798));

    kasFSMCreate(59670451, Init_Mission16_SentinelReinforcementTeam, Init_Mission16_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(171501548));
    kasFSMCreate(59670451, Init_Mission16_SentinelReinforcementTeam, Init_Mission16_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(59670130));

    kasFSMCreate(11133235, Init_Mission16_BeastIonArrayFrigateReinforcementTeam, Init_Mission16_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(151690773));
    kasFSMCreate(11133235, Init_Mission16_BeastIonArrayFrigateReinforcementTeam, Init_Mission16_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(11133425));

    kasFSMCreate(34980115, Init_Mission16_HiveFrigateReinforcementTeam, Init_Mission16_HiveFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(186735637));

    kasFSMCreate(199714963, Init_Mission16_BeastMultiBeamFrigateReinforcementTeam, Init_Mission16_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(62813829));

    kasFSMCreate(249715395, Init_Mission16_BeastCloakedFighterReinforcementTeam, Init_Mission16_BeastCloakedFighterReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(200156578));
    kasFSMCreate(249715395, Init_Mission16_BeastCloakedFighterReinforcementTeam, Init_Mission16_BeastCloakedFighterReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(249715329));

    kasFSMCreate(128635107, Init_Mission16_BeastCorvetteReinforcementTeam, Init_Mission16_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(75148549));
    kasFSMCreate(128635107, Init_Mission16_BeastCorvetteReinforcementTeam, Init_Mission16_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(128634914));

    kasFSMCreate(230526675, Init_Mission16_BeastAcolyteReinforcementTeam, Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(182180066));
    kasFSMCreate(230526675, Init_Mission16_BeastAcolyteReinforcementTeam, Init_Mission16_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526611));

    kasFSMCreate(80024947, Init_Mission16_BeastRammingFrigateReinforcementTeam, Init_Mission16_BeastRammingFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(223105365));
    kasFSMCreate(80024947, Init_Mission16_BeastRammingFrigateReinforcementTeam, Init_Mission16_BeastRammingFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(80025009));

    kasFSMCreate(90801203, Init_Mission16_BeastReconReinforcementTeam, Init_Mission16_BeastReconReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(5675070));
    kasFSMCreate(90801203, Init_Mission16_BeastReconReinforcementTeam, Init_Mission16_BeastReconReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(90801137));
    kasFSMCreate(90801203, Init_Mission16_BeastReconReinforcementTeam, Init_Mission16_BeastReconReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(90801138));

    kasFSMCreate(297667, Init_Mission16_BeastDFGFrigateReinforcementTeam, Init_Mission16_BeastDFGFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(18599));
    kasFSMCreate(297667, Init_Mission16_BeastDFGFrigateReinforcementTeam, Init_Mission16_BeastDFGFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(297601));

    kasFSMCreate(185124787, Init_Mission16_BeastHeavyCruiserReinforcementTeam, Init_Mission16_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(212896882));
    kasFSMCreate(185124787, Init_Mission16_BeastHeavyCruiserReinforcementTeam, Init_Mission16_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(185124849));

    kasFSMCreate(124218850, Init_Mission16_ReinforcementTriggerRouter_RecordJumpPosition, Watch_Mission02_AttackMothershipTeam,
                 kasAITeamPtrIfExists(108436545));

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sSentinel");
    kasfModuleSlotOrderSetShipType(0, "sLeech");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetShipType(0, "sHiveFrigate");
    kasfModuleSlotOrderSetShipType(0, "sMultiBeamFrigate");
    kasfModuleSlotOrderSetCount(0, 5);
    kasfModuleSlotOrderSetShipType(1, "bAcolyte");
    kasfModuleSlotOrderSetShipType(1, "bHeavyCorvette");
    kasfModuleSlotOrderSetShipType(1, "bMissileCorvette");
    kasfModuleSlotOrderSetShipType(1, "bWorker");
    kasfModuleSlotOrderSetShipType(1, "bIonArrayFrigate");
    kasfModuleSlotOrderSetShipType(1, "bDFGFrigate");
    kasfModuleSlotOrderSetCount(1, 3);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfShipsAdd(kasGrowSelectionPtr(4627076), kasAITeamShipsPtrIfExists(108436545));
    kasfShipsAdd(kasGrowSelectionPtr(4627076), kasAITeamShipsPtrIfExists(127538865));
    kasfShipsAdd(kasGrowSelectionPtr(165711363), kasAITeamShipsPtrIfExists(108436545));

    kasFSMCreate(44665972, Init_Mission16_ReinforcementPool, Fill_Mission16_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(44665972));
    kasFSMCreate(134326499, Init_Mission16_DockWithCS, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(134326499));

    kasfShipsApplyPlayerUpgrades(kasGrowSelectionPtr(114961833));
    kasTimerCreateSetStartById(150120898, 10);
}
