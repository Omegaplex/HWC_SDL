/*=============================================================================
    Name    : mission09.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission09
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m09_saveInfoName[] =
{
    "Close Orbit, Gozan IV",
    "French version goes here",
    "German version goes here",
};

void Init_Mission09_FleetIntel(void);
void Watch_Mission09_FleetIntel_CleanupAndResume(void);
void Watch_Mission09_FleetIntel_ResumeDelay(void);
void Init_Mission09_FleetIntel_RevealResourceTeam(void);
void Watch_Mission09_FleetIntel_EventFlow(void);
void Watch_Mission09_ObjectiveEventFlow(void);
void Init_Mission09_EventState(void);
void Begin_CruiseMissileStrikeWave(void);
void Watch_CruiseMissileStrikeWave(void);
void Init_Mission09_Traders_TalkingMoreCombatLoop(void);
void Watch_Mission09_Traders_TalkingMoreCombatLoop(void);
void Begin_Mission09_Traders_TalkingMoreLaunch(void);
void Watch_Mission09_Traders_Talking(void);
void Watch_Mission09_Traders_TalkingMoreLaunchPrep(void);
void Watch_Mission09_Traders_TalkingMoreLaunch(void);
void Init_Mission09_ResourceTeam(void);
void Watch_Mission09_ResourceTeam(void);
void Watch_Mission09_ResourceTeam_ReinforceFromNearbyShips(void);
void Watch_Mission09_ResourceTeam_InterceptMoveOrderTrigger(void);
void Init_Mission09_ResourceTeam_MoveToInterceptVector(void);
void Watch_Mission09_ResourceTeam_MoveToInterceptVector(void);
void Watch_Mission09_ResourceTeam_InterceptTargetTrigger(void);
void Init_Mission09_ResourceTeam_InterceptTargetGroup(void);
void Watch_Mission09_ResourceTeam_InterceptTargetGroup(void);
void Init_Mission09_AssaultEscort_Cooldown(void);
void Init_Mission09_AssaultEscort(void);
void Watch_Mission09_AssaultEscort(void);
void Init_Mission09_AssaultEscort_SetRandomWaypoint(void);
void Watch_Mission09_AssaultEscort_WaypointLoop(void);
void Begin_Mission09_AssaultEscort_AttackRun(void);
void Watch_Mission09_AssaultEscort_AttackRun(void);
void Init_Mission09_AssaultEscort_Repath(void);
void Watch_Mission09_AssaultEscort_Repath(void);
void Init_Mission09_AssaultEscort_ReinforcementDelay(void);
void Watch_Mission09_AssaultEscort_SpawnHeavyInterceptorReinforcement(void);
void Init_Mission09_AssaultEscort_DockForLaunch(void);
void Watch_Mission09_AssaultEscort_DockForLaunch(void);
void Watch_Mission09_AssaultEscort_PostLaunch(void);
void Init_Mission09_AssaultEscort_InterceptNearbyContacts(void);
void Watch_Mission09_AssaultEscort_InterceptNearbyContacts(void);
void Init_Mission09_MinelayerEscort(void);
void Watch_Mission09_MinelayerEscort(void);
void Watch_Mission09_MinelayerEscort_DeployMines(void);
void Watch_Mission09_MinelayerEscort_WaitForLaunchSignal(void);
void Init_Mission09_MinelayerEscort_SetRandomMineDropPoint(void);
void Watch_Mission09_MinelayerEscort_SetRandomMineDropPoint(void);
void Init_Mission09_MinelayerEscort_RepositionDelay(void);
void Watch_Mission09_MinelayerEscort_RepositionDelay(void);
void Init_Mission09_MinelayerEscort_Reengage(void);
void Watch_Mission09_MinelayerEscort_Reengage(void);
void Init_Mission09_IonFrigateEscort(void);
void Watch_Mission09_IonFrigateEscort(void);
void Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup(void);
void Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup(void);
void Begin_Mission09_IonFrigateEscort_AttackRun(void);
void Watch_Mission09_IonFrigateEscort_Reengage(void);
void Watch_Mission09_IonFrigateEscort_SpawnReinforcement(void);
void Init_Mission09_MultigunCorvetteEscort(void);
void Watch_Mission09_MultigunCorvetteEscort(void);
void Init_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup(void);
void Watch_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup(void);
void Begin_Mission09_MultigunCorvetteEscort_AttackRun(void);
void Watch_Mission09_MultigunCorvetteEscort_Reengage(void);
void Init_Mission09_MultigunCorvetteEscort_SelectAttackVector(void);
void Watch_Mission09_MultigunCorvetteEscort_SelectAttackVector(void);
void Watch_Mission09_MultigunCorvetteEscort_SpawnReinforcement(void);
void Init_Mission09_MultigunCorvetteEscort_ReinforcementDelay(void);
void Init_Mission09_SupportFrigateEscort(void);
void Watch_Mission09_SupportFrigateEscort(void);
void Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup(void);
void Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup(void);
sdword Begin_Mission09_SupportFrigateEscort_RepairOrder(void);
void Watch_Mission09_SupportFrigateEscort_SpawnReinforcement(void);
void Init_Mission09_SupportFrigateEscort_ReinforcementDelay(void);
void Watch_Mission09_PostIntelBranch(void);
void Watch_Mission09_BigGunSequenceEvents(void);
void Watch_Mission09_GlobalEvents(void);
void Watch_FleetIntel_WaitForIntelEventEnd(void);
void ThisAIPlayerShipCreatedCB(AIPlayer *aiplayer, Ship *ship);

void Watch_Mission09(void)
{
  int v0; // eax
  int *v1; // eax
  int *v2; // eax
  int *v3; // eax
  int *v4; // eax
  int *v5; // eax
  int v6; // eax
  signed int v7; // eax
  int *v8; // eax
  int *v9; // eax
  int v10; // eax
  int *v11; // eax
  int v12; // eax
  int v13; // eax
  int *v14; // eax
  int v15; // eax
  int v16; // eax
  int v17; // eax
  int v18; // eax
  int v19; // eax
  int v20; // eax
  int v21; // eax
  int v22; // eax
  int *v23; // eax
  int v24; // eax
  int v25; // eax
  int *v26; // eax
  int *v27; // eax
  int *v28; // eax
  int v29; // eax
  int *v30; // eax
  int *v32; // eax
  int v33; // [esp-8h] [ebp-8h]
  int v34; // [esp-8h] [ebp-8h]
  int v35; // [esp-8h] [ebp-8h]
  int v36; // [esp-8h] [ebp-8h]
  int v37; // [esp-8h] [ebp-8h]
  int v38; // [esp-8h] [ebp-8h]
  int v39; // [esp-8h] [ebp-8h]
  int v40; // [esp-8h] [ebp-8h]
  int v41; // [esp-4h] [ebp-4h]
  int v42; // [esp-4h] [ebp-4h]
  int v43; // [esp-4h] [ebp-4h]
  int *v44; // [esp-4h] [ebp-4h]
  int v45; // [esp-4h] [ebp-4h]
  int v46; // [esp-4h] [ebp-4h]
  int v47; // [esp-4h] [ebp-4h]
  int v48; // [esp-4h] [ebp-4h]
  int v49; // [esp-4h] [ebp-4h]
  int *v50; // [esp-4h] [ebp-4h]
  int v51; // [esp-4h] [ebp-4h]
  signed int v52; // [esp-4h] [ebp-4h]
  int v53; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503673;
  if ( kasTimerExpiredDestroyById(104338707) )
  {
    v41 = kasGrowSelectionPtr(18336307);
    v0 = kasVolumePtr(132933767);
    kasfFindShipsInside(v0, v41);
    v42 = kasGrowSelectionPtr(18336307);
    v1 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v1, v42);
    v43 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v2, v43);
    v3 = (int *)kasGrowSelectionPtr(130069113);
    kasfGrowSelectionClear(v3);
    v44 = (int *)kasGrowSelectionPtr(0x4A2A29);
    v4 = (int *)kasGrowSelectionPtr(130069113);
    kasfShipsAdd(v4, v44);
    if ( kasfEntryValueGet(-1909110715) && !kasfEntryValueGet(-2089378044)
      || !kasfEntryValueGet(-1909110715)
      && !kasfEntryValueGet(-2089378044)
      && (v45 = kasVolumePtr(128156707),
          v33 = kasGrowSelectionPtr(77795571),
          v5 = (int *)kasGrowSelectionPtr(82017669),
          !kasfShipsSelectInsideVolume(v5, v33, v45)) )
    {
      v46 = kasGrowSelectionPtr(77795571);
      v6 = kasGrowSelectionPtr(130069113);
      kasfShipsRemove(v6, v46);
    }
    v7 = kasfRandom(8, 15);
    kasTimerSetById(104338707, v7);
    kasTimerStartById(104338707);
  }
  if ( !kasfEntryValueGet(71552166) && kasfEntryValueGet(-2015429404) )
  {
    kasfEntrySetValue(71552166, 1);
    v47 = kasGrowSelectionPtr(18336307);
    v8 = (int *)kasGrowSelectionPtr(89157026);
    kasSelectPlayerShipsSimple(v8, v47);
    v48 = kasGrowSelectionPtr(89157026);
    v9 = (int *)kasGrowSelectionPtr(89157026);
    kasfShipsSelectNotSalvagers(v9, v48);
    v49 = kasGrowSelectionPtr(218060896);
    v10 = kasGrowSelectionPtr(82017669);
    if ( kasfShipsSelectLinkedShips(v10, v49) )
    {
      v50 = (int *)kasGrowSelectionPtr(82017669);
      v11 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsAdd(v11, v50);
    }
    v34 = kasGrowSelectionPtr(89157026);
    v12 = kasGrowSelectionPtr(0x009C105B);
    if ( kasfShipsSelectType(v12, v34, "tResourceCollector") )
    {
      v13 = kasGrowSelectionPtr(0x009C105B);
      sub_4984C0(v13, 0);
      v35 = kasGrowSelectionPtr(0x009C105B);
      v14 = (int *)kasGrowSelectionPtr(32015013);
      kasfShipsSelectFirstN(v14, v35, 3);
      v51 = kasGrowSelectionPtr(32015013);
      v15 = kasGrowSelectionPtr(0x009C105B);
      if ( kasfShipsRemove(v15, v51) )
      {
        v16 = kasGrowSelectionPtr(0x009C105B);
        kasfDeleteShips(v16);
      }
    }
    if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
    {
      v36 = kasGrowSelectionPtr(89157026);
      v17 = kasGrowSelectionPtr(96759714);
      if ( !kasfShipsSelectType(v17, v36, "sWorker") )
      {
        v37 = kasGrowSelectionPtr(218060896);
        v18 = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(v18, "sWorker", 1, v37, 0);
      }
    }
    v38 = kasGrowSelectionPtr(0x4A2A29);
    v19 = kasGrowSelectionPtr(77795571);
    if ( !kasfShipsSelectType(v19, v38, "tResourceCollector") )
    {
      v39 = kasGrowSelectionPtr(218060896);
      v20 = kasGrowSelectionPtr(77795571);
      kasfShipsCreateInside(v20, "tResourceCollector", 3, v39, 0);
    }
    switch ( GetGameDifficulty() )
    {
      case 0:
        v52 = 500;
        goto LABEL_27;
      case 1:
        v52 = 300;
        goto LABEL_27;
      case 2:
        v52 = 200;
        goto LABEL_27;
      case 3:
      case 4:
        v52 = 100;
LABEL_27:
        v21 = kasGrowSelectionPtr(77795571);
        kasfShipsSetPercentDamaged(v21, v52);
        break;
      default:
        break;
    }
    v22 = kasGrowSelectionPtr(77795571);
    kasfShipsSetAttachVelocityFlag(v22, 1);
    v53 = kasGrowSelectionPtr(18336307);
    v23 = (int *)kasGrowSelectionPtr(45901433);
    kasSelectPlayerShipsSimple(v23, v53);
    v40 = kasGrowSelectionPtr(45901433);
    v24 = kasGrowSelectionPtr(28447699);
    kasfShipsSelectType(v24, v40, "tResourceCollector");
    v25 = kasGrowSelectionPtr(28447699);
    sub_4984C0(v25, 0);
    v26 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v26, (char *)120574995);
    v27 = kasAITeamPtrIfExists(44055618);
    kasfMsgSend((struct AITeam *)(int)v27, (char *)5801588);
    spSetSinglePlayerSaveInfo(9, m09_saveInfoName[strCurLanguage]);
  }
  if ( !kasfEntryValueGet(71552167) && kasTimerExpiredById(-2036075036) )
  {
    kasfEntrySetValue(71552167, 1);
    v28 = kasAITeamPtrIfExists(77795571);
    kasfMsgSend((struct AITeam *)(int)v28, (char *)39122404);
    v29 = kasAITeamShipsPtrIfExists(77795571);
    kasfUnhideShips(v29);
    v30 = kasAITeamPtrIfExists(207674834);
    kasfMsgSend((struct AITeam *)(int)v30, (char *)5801588);
  }
  if ( !kasfEntryValueGet(71552168) )
  {
    if ( kasTimerExpiredById(91769491) )
    {
      kasfEntrySetValue(71552168, 1);
      v32 = kasAITeamPtrIfExists(91769491);
      kasfMsgSend((struct AITeam *)(int)v32, (char *)5801588);
    }
  }
}

void Watch_Mission09_MissionFlow_SelectNearbyAttackersOrQueueStandardFrigateBatch(void)
{
    GrowSelection *nearbyAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        nearbyAttackers = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(nearbyAttackers);
        kasfShipsAdd(nearbyAttackers, kasGrowSelectionPtr(211966048));
        kasfFindEnemiesNearby(nearbyAttackers, 10000);
        tutDeallocateRootRegion();
    }
    else
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_QueueStandardFrigateBatch,
            (sdword)Watch_StandardFrigateBatchComplete);
    }
}

void Watch_Mission09_MissionFlow_GhostIntro(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission09_MissionFlow_SelectNearbyAttackersOrQueueStandardFrigateBatch);
    }
}

void Watch_Mission09_Traders_TalkingMore(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_Traders_TalkingMoreCombatLoop,
            (sdword)Watch_Mission09_Traders_TalkingMoreCombatLoop);
    }
}

void Init_Mission09_AttackBomberWing(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission09_AttackBomberWing_LaunchTrigger);
}

void Watch_Mission09_AttackBomberWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission09_AttackBomberWing_SpawnDelay, (sdword)Watch_Mission09_AttackBomberWing_Spawn);
    }
}

void Watch_Mission09_AttackBomberWing_LaunchTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)178743493);
        kasfSpeechEvent(4693508, (sdword)Init_Mission09_AttackBomberWing_SpawnDelay, (sdword)Watch_Mission09_AttackBomberWing_Spawn);
    }
}

void Init_Mission09_AttackBomberWing_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerDestroyById(4693508);
    kasTimerCreateSetStartById(4693508, kasfRandom(20, 120));
}

void Watch_Mission09_AttackBomberWing_Spawn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(4693508) || kasfMsgReceived((char *)178743493))
    {
        kasTimerDestroyById(4693508);
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "tAttackBomber",
            (char *)kasfRandom(10, 20),
            (sdword)kasGrowSelectionPtr(19508),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(154639956));
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(175938357, (sdword)Init_Mission09_AttackBomberWing_Regroup, (sdword)Watch_Mission09_AttackBomberWing_Regroup);
    }
}

void Init_Mission09_AttackBomberWing_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationClaw();
    kasTimerCreateSetStartById(130659266, 20);
}

void Watch_Mission09_AttackBomberWing_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(130659266))
    {
        kasTimerCreateSetStartById(175938357, 6);
        kasfAttackMothership();
    }
    if (kasTimerExpiredById(175938357))
    {
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(218060896), kasVectorPtrIfExists(218060896), 60000);
        kasfSpeechEvent(
            316420,
            (sdword)Init_Mission09_AttackBomberWing_ReengageDelay,
            (sdword)Watch_Mission09_AttackBomberWing_ReengageDelay);
    }
}

void Init_Mission09_AttackBomberWing_ReengageDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(316420, 20);
    kasfFormationClaw();
}

void Watch_Mission09_AttackBomberWing_ReengageDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(316420))
    {
        kasfFormationClaw();
        kasfAttack(kasGrowSelectionPtr(218060896));
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_AttackBomberWing_HuntNearbyTargets,
            (sdword)Watch_Mission09_AttackBomberWing_HuntNearbyTargets);
    }
}

void Init_Mission09_AttackBomberWing_HuntNearbyTargets(void)
{
    GrowSelection *nearbyTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(
            nearbyTargets,
            kasGrowSelectionPtr(130069113),
            kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
            20000))
    {
        kasfFormationClaw();
        singlePlayerSortShipsByDistanceToPoint(
            nearbyTargets,
            kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
        tutDeallocateRootRegion();
    }
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission09_AttackBomberWing_HuntNearbyTargets(void)
{
    GrowSelection *workerTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr(57245931)) && singlePlayerShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfFormationClaw();
        if (kasfRandom(0, 1))
        {
            workerTargets = kasGrowSelectionPtr(147091442);
            kasfShipsSelectType(workerTargets, kasGrowSelectionPtr(80030249), "sWorker");
            singlePlayerSortShipsByDistanceToPoint(
                workerTargets,
                kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
            if (!kasfShipsCount(workerTargets))
            {
                kasfShipsAdd(workerTargets, kasGrowSelectionPtr(218060896));
            }
            kasfAttack(workerTargets);
        }
        else
        {
            kasfAttack(kasGrowSelectionPtr(218060896));
        }
    }
    else if (!kasTimerExpiredById(74024315))
    {
        return;
    }
    kasfSpeechEvent(
        74024315,
        (sdword)Init_Mission09_AttackBomberWing_HuntNearbyTargets,
        (sdword)Watch_Mission09_AttackBomberWing_HuntNearbyTargets);
}

void Init_Mission09_HeavyInterceptorWing(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission09_HeavyInterceptorWing_LaunchTrigger);
}

void Watch_Mission09_HeavyInterceptorWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)Init_Mission09_HeavyInterceptorWing_SpawnDelay,
            (sdword)Watch_Mission09_HeavyInterceptorWing_Spawn);
    }
}

void Watch_Mission09_HeavyInterceptorWing_LaunchTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)178743493);
        kasfSpeechEvent(
            4693508,
            (sdword)Init_Mission09_HeavyInterceptorWing_SpawnDelay,
            (sdword)Watch_Mission09_HeavyInterceptorWing_Spawn);
    }
}

void Init_Mission09_HeavyInterceptorWing_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerDestroyById(4693508);
    kasTimerCreateSetStartById(4693508, kasfRandom(20, 60));
}

void Watch_Mission09_HeavyInterceptorWing_Spawn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(4693508) || kasfMsgReceived((char *)178743493))
    {
        kasTimerDestroyById(4693508);
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "tHeavyInterceptor",
            (char *)kasfRandom(4, 6),
            (sdword)kasGrowSelectionPtr(19508),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(154639956));
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            175938357,
            (sdword)Init_Mission09_HeavyInterceptorWing_Regroup,
            (sdword)Watch_Mission09_HeavyInterceptorWing_Regroup);
    }
}

void Init_Mission09_HeavyInterceptorWing_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationDelta();
    kasTimerCreateSetStartById(130659266, 20);
}

void Watch_Mission09_HeavyInterceptorWing_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(130659266))
    {
        kasTimerCreateSetStartById(175938357, 6);
        kasfAttackMothership();
    }
    if (kasTimerExpiredById(175938357))
    {
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(218060896), kasVectorPtrIfExists(218060896), 60000);
        kasfSpeechEvent(
            316420,
            (sdword)Init_Mission09_HeavyInterceptorWing_ReengageDelay,
            (sdword)Watch_Mission09_HeavyInterceptorWing_ReengageDelay);
    }
}

void Init_Mission09_HeavyInterceptorWing_ReengageDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(316420, 20);
    kasfFormationDelta();
}

void Watch_Mission09_HeavyInterceptorWing_ReengageDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(316420))
    {
        kasfFormationDelta();
        kasfIntercept(kasGrowSelectionPtr(218060896));
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_HeavyInterceptorWing_HuntTargets,
            (sdword)Watch_Mission09_HeavyInterceptorWing_HuntTargets);
    }
}

void Init_Mission09_HeavyInterceptorWing_HuntTargets(void)
{
    GrowSelection *nearbyTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(
            nearbyTargets,
            kasGrowSelectionPtr(130069113),
            kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
            20000))
    {
        kasfFormationDelta();
        singlePlayerSortShipsByDistanceToPoint(
            nearbyTargets,
            kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
        tutDeallocateRootRegion();
    }
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission09_HeavyInterceptorWing_HuntTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if ((kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)) && kasfTeamHealthAverage() < 70) || kasfTeamCount() < 3)
    {
        kasfSpeechEvent(
            111765812,
            (sdword)Init_Mission09_HeavyInterceptorWing_RequestSupportDelay,
            (sdword)Init_Mission09_HeavyInterceptorWing_RequestSupport);
    }
    else if (kasfShipsCount(kasGrowSelectionPtr(57245931)) && singlePlayerShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfFormationDelta();
        kasfAttack(kasGrowSelectionPtr(218060896));
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_HeavyInterceptorWing_HuntTargets,
            (sdword)Watch_Mission09_HeavyInterceptorWing_HuntTargets);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_HeavyInterceptorWing_HuntTargets,
            (sdword)Watch_Mission09_HeavyInterceptorWing_HuntTargets);
    }
}

void Init_Mission09_HeavyInterceptorWing_RequestSupportDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(84331138, 20);
    kasfFormationDelta();
}

void Init_Mission09_HeavyInterceptorWing_RequestSupport(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)))
    {
        if (!kasfTeamDockedReadyForLaunch())
        {
            return;
        }
        kasfMsgSend(kasAITeamPtrIfExists(346756), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(207674834), (char *)5801588);
    }
    else
    {
        kasfKamikaze(kasGrowSelectionPtr(130069113));
    }
    kasfSpeechEvent(342540, (sdword)sub_60C010, (sdword)sub_652EF0);
}

void Init_Mission09_CapitalAssaultWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission09_CapitalAssaultWave_LaunchTrigger);
}

void Watch_Mission09_CapitalAssaultWave(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
kasfSpeechEvent(4693508, (sdword)Init_Mission09_CapitalAssaultWave_SpawnDelay, (sdword)Watch_Mission09_CapitalAssaultWave_Spawn);
    }
}

void Watch_Mission09_CapitalAssaultWave_LaunchTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
kasfMsgSend(kasAITeamPtrIfExists(248587548), (char *)5801588);
kasfMsgSend(kasAITeamPtrIfExists(238890036), (char *)5801588);
kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)178743493);
kasfSpeechEvent(4693508, (sdword)Init_Mission09_CapitalAssaultWave_SpawnDelay, (sdword)Watch_Mission09_CapitalAssaultWave_Spawn);
    }
}

void Init_Mission09_CapitalAssaultWave_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerDestroyById(4693508);
    kasTimerCreateSetStartById(4693508, kasfRandom(160, 300));
}

void Watch_Mission09_CapitalAssaultWave_Spawn(void)
{
    AITeam *team;
    sdword gameDifficulty;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(4693508) || kasfMsgReceived((char *)178743493))
    {
kasTimerDestroyById(4693508);
gameDifficulty = GetGameDifficulty();
switch (gameDifficulty)
{
    case 0:
        kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "tStandardFrigate", (char *)3, (sdword)kasGrowSelectionPtr(19508), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        break;
    case 1:
        kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "tStandardDestroyer", (char *)1, (sdword)kasGrowSelectionPtr(19508), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        break;
    case 2:
        if (kasfRandom(1, 2) == 1)
        {
            kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "tStandardDestroyer", (char *)1, (sdword)kasGrowSelectionPtr(19508), 1);
            kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
            kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        }
        else
        {
            kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "tMissileDestroyer", (char *)1, (sdword)kasGrowSelectionPtr(19508), 1);
            kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
            kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        }
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "tStandardFrigate",
            (char *)kasfRandom(3, 4),
            (sdword)kasGrowSelectionPtr(19508),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        break;
    case 3:
    case 4:
        kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "tStandardDestroyer", (char *)1, (sdword)kasGrowSelectionPtr(19508), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "tStandardFrigate",
            (char *)kasfRandom(3, 4),
            (sdword)kasGrowSelectionPtr(19508),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "tMissileDestroyer", (char *)1, (sdword)kasGrowSelectionPtr(19508), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        break;
    default:
        break;
}
kasfShipsAdd(kasGrowSelectionPtr(92594397), &team->shipList);
    }
    if (kasfTeamFinishedLaunching())
    {
kasfMsgSend(kasAITeamPtrIfExists(248587548), (char *)128121829);
kasfMsgSend(kasAITeamPtrIfExists(238890036), (char *)128121829);
kasfEntrySetValue(332667, 0);
kasfSpeechEvent(175938357, (sdword)Init_Mission09_SupportFrigateSquad_Regroup, (sdword)Watch_Mission09_CapitalAssaultWave_Regroup);
    }
}

void Init_Mission09_SupportFrigateSquad_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationWall();
    kasTimerCreateSetStartById(130659266, 20);
}

void Watch_Mission09_CapitalAssaultWave_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(130659266))
    {
kasTimerCreateSetStartById(175938357, 10);
kasfAttackMothership();
    }
    if (kasTimerExpiredById(175938357))
    {
kasfLabelledVectorSet(207674834, 0, 0, 0);
kasfVectorRandomizeInRange(kasVectorPtrIfExists(207674834), kasVectorPtrIfExists(218060896), 60000);
kasfMoveTo(kasVectorPtrIfExists(207674834));
kasfSpeechEvent(316420, (sdword)Init_Mission09_AssaultEscort_ReengageDelay, (sdword)Init_Mission09_CapitalAssaultWave_HuntTargets);
    }
}

void Init_Mission09_AssaultEscort_ReengageDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(316420, 20);
    kasfFormationWall();
}

void Init_Mission09_CapitalAssaultWave_HuntTargets(void)
{
    GrowSelection *attackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasTimerExpiredById(316420))
    {
return;
    }
    kasfFormationWall();
    if (kasfRandom(0, 1))
    {
attackTargets = kasGrowSelectionPtr(147091442);
kasfShipsSelectType(attackTargets, kasGrowSelectionPtr(80030249), "sWorker");
singlePlayerSortShipsByDistanceToPoint(
    attackTargets,
    kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
if (!kasfShipsCount(attackTargets))
{
    kasfShipsAdd(attackTargets, kasGrowSelectionPtr(218060896));
}
    }
    else
    {
attackTargets = kasGrowSelectionPtr(218060896);
    }
    kasfAttack(attackTargets);
    kasfSpeechEvent(
74024315,
(sdword)Init_Mission09_CapitalAssaultWave_HuntNearbyTargets,
(sdword)Watch_Mission09_CapitalAssaultWave_HuntNearbyTargets);
}

void Init_Mission09_CapitalAssaultWave_HuntNearbyTargets(void)
{
    GrowSelection *nearbyTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(
    nearbyTargets,
    kasGrowSelectionPtr(130069113),
    kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
    20000))
    {
kasfFormationWall();
singlePlayerSortShipsByDistanceToPoint(
    nearbyTargets,
    kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
tutDeallocateRootRegion();
    }
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission09_CapitalAssaultWave_HuntNearbyTargets(void)
{
    GrowSelection *workerTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr(57245931)) && singlePlayerShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
kasfFormationWall();
if (kasfRandom(0, 1))
{
    workerTargets = kasGrowSelectionPtr(147091442);
    kasfShipsSelectType(workerTargets, kasGrowSelectionPtr(80030249), "sWorker");
    singlePlayerSortShipsByDistanceToPoint(
        workerTargets,
        kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
    if (!kasfShipsCount(workerTargets))
    {
        kasfShipsAdd(workerTargets, kasGrowSelectionPtr(218060896));
    }
    kasfAttack(workerTargets);
}
else
{
    kasfAttack(kasGrowSelectionPtr(218060896));
}
    }
    else if (!kasTimerExpiredById(74024315))
    {
return;
    }
    kasfSpeechEvent(
74024315,
(sdword)Init_Mission09_CapitalAssaultWave_HuntNearbyTargets,
(sdword)Watch_Mission09_CapitalAssaultWave_HuntNearbyTargets);
}

void Init_Mission09_GravwellSquad(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfActivateTech("GravityWellGeneratorTech");
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (GetGameDifficulty() >= 3)
    {
        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_GravwellSquad_LaunchTrigger);
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)sub_60C010, (sdword)sub_652EF0);
    }
}

void Watch_Mission09_GravwellSquad(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission09_GravwellSquad_SpawnDelay, (sdword)Watch_Mission09_GravwellSquad_Spawn);
    }
}

void Watch_Mission09_GravwellSquad_LaunchTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)178743493);
        kasfSpeechEvent(4693508, (sdword)Init_Mission09_GravwellSquad_SpawnDelay, (sdword)Watch_Mission09_GravwellSquad_Spawn);
    }
}

void Init_Mission09_GravwellSquad_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerDestroyById(4693508);
    kasTimerCreateSetStartById(4693508, kasfRandom(120, 160));
}

void Watch_Mission09_GravwellSquad_Spawn(void)
{
    AITeam *team;
    sdword gameDifficulty;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)128121829))
    {
        kasfEntryGetOrCreateSetValue(26926501, 1);
    }
    if (kasTimerExpiredDestroyById(4693508) || kasfMsgReceived((char *)178743493))
    {
        kasTimerDestroyById(4693508);
        gameDifficulty = GetGameDifficulty();
        if (gameDifficulty >= 1 && gameDifficulty <= 4)
        {
            kasfShipsCreateInside(
                kasGrowSelectionPtr(154639956),
                "tGravWellGenerator",
                (char *)1,
                (sdword)kasGrowSelectionPtr(19508),
                1);
        }
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        singlePlayerTeamSpecialToggle();
        kasfShipsAdd(kasGrowSelectionPtr(92594397), &team->shipList);
    }
    if (kasfEntryValueGet(26926501) && kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(26926501, 0);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(175938357, (sdword)Init_Mission09_SupportFrigateSquad_Regroup, (sdword)Watch_Mission09_GravwellSquad_Regroup);
    }
}

void Watch_Mission09_GravwellSquad_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(130659266))
    {
        kasTimerCreateSetStartById(175938357, 10);
        kasfAttackMothership();
    }
    if (kasTimerExpiredById(175938357))
    {
        kasfMoveTo(kasVectorPtrIfExists(207674834));
        kasfSpeechEvent(316420, (sdword)Init_Mission09_AssaultEscort_ReengageDelay, (sdword)Watch_Mission09_GravwellSquad_Reengage);
    }
}

void Watch_Mission09_GravwellSquad_Reengage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(316420))
    {
        kasfFormationWall();
        kasfSpeechEvent(0x4C9664, (sdword)Init_Mission09_GravwellSquad_HuntDamagedCapital, (sdword)Watch_Mission09_GravwellSquad_HuntDamagedCapital);
    }
}

void Init_Mission09_GravwellSquad_HuntDamagedCapital(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationWall();
    kasfProtect(kasGrowSelectionPtr(92594397));
    if (!kasfEntryValueGet(71552165))
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfEntrySetValue(248701100, 0);
    }
    kasTimerDestroyById(0x4C9664);
    kasTimerCreateSetStartById(0x4C9664, kasfRandom(10, 20));
}

void Watch_Mission09_GravwellSquad_HuntDamagedCapital(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(207674834)))
    {
        kasfVarDestroy((char *)&((AITeam *)kasCurrentTeamPtr)->shipList);
    }
    if (kasTimerExpiredById(0x4C9664))
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission09_GravwellSquad_HuntDamagedCapital,
            (sdword)Watch_Mission09_GravwellSquad_HuntDamagedCapital);
    }
}

void Init_Mission09_SupportFrigateSquad(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    if (!GetGameDifficulty())
    {
        kasfSpeechEvent(342540, (sdword)sub_60C010, (sdword)sub_652EF0);
        return;
    }
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_SupportFrigateSquad_LaunchTrigger);
}

void Watch_Mission09_SupportFrigateSquad(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission09_GravwellSquad_SpawnDelay, (sdword)Watch_Mission09_SupportFrigateSquad_Spawn);
    }
}

void Watch_Mission09_SupportFrigateSquad_LaunchTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)178743493);
        kasfSpeechEvent(4693508, (sdword)Init_Mission09_GravwellSquad_SpawnDelay, (sdword)Watch_Mission09_SupportFrigateSquad_Spawn);
    }
}

void Watch_Mission09_SupportFrigateSquad_Spawn(void)
{
    AITeam *team;
    sdword shipsToCreate;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)128121829))
    {
        kasfEntryGetOrCreateSetValue(26926501, 1);
    }
    if (kasTimerExpiredDestroyById(4693508) || kasfMsgReceived((char *)178743493))
    {
        kasTimerDestroyById(4693508);
        shipsToCreate = 0;
        switch (GetGameDifficulty())
        {
            case 1:
            case 2:
                shipsToCreate = 1;
                break;
            case 3:
            case 4:
                shipsToCreate = 2;
                break;
        }
        if (shipsToCreate > 0)
        {
            kasfShipsCreateInside(
                kasGrowSelectionPtr(154639956),
                "tAdvanceSupportFrigate",
                (char *)shipsToCreate,
                (sdword)kasGrowSelectionPtr(19508),
                1);
        }
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        kasfShipsAdd(kasGrowSelectionPtr(92594397), &team->shipList);
    }
    if (kasfEntryValueGet(26926501) && kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(26926501, 0);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(175938357, (sdword)Init_Mission09_SupportFrigateSquad_Regroup, (sdword)Watch_Mission09_SupportFrigateSquad_Regroup);
    }
}

void Watch_Mission09_SupportFrigateSquad_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(130659266))
    {
        kasTimerCreateSetStartById(175938357, 10);
        kasfAttackMothership();
    }
    if (kasTimerExpiredById(175938357))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(207674834));
        kasfSpeechEvent(316420, (sdword)Init_Mission09_AssaultEscort_ReengageDelay, (sdword)Watch_Mission09_GravwellSquad_Reengage);
    }
}

void Init_Mission09_FleetIntel_PauseWorld(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(KH_INTEL_FLOW_ACTIVE, 1);
    kasfUniversePause();
    kasfOtherKASPause();
}

void Init_Mission09_SensorObjectiveTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfActivateTech("SensorsArrayTech");
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfSpeechEvent(174166565, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_SensorObjectiveTeam);
}

void Watch_Mission09_SensorObjectiveTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount() <= kasfTeamCountOriginal() - 1)
    {
        kasfEntryGetOrCreateSetValue(-2085181655, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)191311479);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)sub_652EF0);
    }
}

void Watch_Mission09_BigGunEscortWing_SetWaypoint(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfTeamCount())
    {
        if ((kasfThisTeamIs(kasAITeamPtrIfExists(147220289)) &&
             !kasfShipsCount(kasAITeamShipsPtrIfExists(45227969))) ||
            (kasfThisTeamIs(kasAITeamPtrIfExists(147220290)) &&
             !kasfShipsCount(kasAITeamShipsPtrIfExists(45227970))) ||
            (kasfThisTeamIs(kasAITeamPtrIfExists(147220291)) &&
             !kasfShipsCount(kasAITeamShipsPtrIfExists(45227971))) ||
            (kasfThisTeamIs(kasAITeamPtrIfExists(147220292)) &&
             !kasfShipsCount(kasAITeamShipsPtrIfExists(45227972))))
        {
            kasfFormationDelta3D();
            kasfSpeechEvent(
                74024315,
                Init_Mission09_BigGunEscortWing_ReacquireTargets,
                Watch_Mission09_BigGunEscortWing_ReacquireTargets);
        }
        return;
    }

    kasfStop();
    kasfSpeechEvent(
        110655924,
        Init_Mission09_BigGunEscortWing_SpawnReinforcements,
        Watch_Mission09_BigGunEscortWing_SpawnReinforcements);
}

void Init_Mission09_BigGunEscortWing_ReacquireTargets(void)
{
    GrowSelection *attackers = kasGrowSelectionPtr(57245931);
    GrowSelection *allTargets = kasGrowSelectionPtr(130069113);
    hvector *teamPosition;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    kasfGrowSelectionClear(attackers);
    if (!kasfShipsCount(attackers))
    {
        kasfUnderAttack(attackers);
    }
    if (!kasfShipsCount(attackers) && kasfEntryValueGet(-2085181655))
    {
        kasfShipsSelectNonCapital(attackers, allTargets);
    }
    if (!kasfShipsCount(attackers) && kasfEntryValueGet(-2085181655))
    {
        kasfShipsAdd(attackers, allTargets);
    }
    if (!kasfShipsCount(attackers))
    {
        teamPosition = kasThisTeamsVectorPtr();
        kasfShipsSelectNearby(attackers, allTargets, teamPosition, 10000.0f);
        kasfShipsSelectNonCapital(attackers, attackers);
    }
    if (!kasfShipsCount(attackers))
    {
        teamPosition = kasThisTeamsVectorPtr();
        kasfShipsSelectNearby(attackers, allTargets, teamPosition, 10000.0f);
    }

    kasfFormationDelta3D();
    singlePlayerSortShipsByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(161590357, kasfRandom(5, 20));
}

void Watch_Mission09_BigGunEscortWing_ReacquireTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) && kasTimerExpiredDestroyById(161590357))
        {
            kasTimerDestroyById(161590357);
            kasfSpeechEvent(
                74024315,
                Init_Mission09_BigGunEscortWing_ReacquireTargets,
                Watch_Mission09_BigGunEscortWing_ReacquireTargets);
        }
        return;
    }

    kasTimerDestroyById(161590357);
    kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
}

void Init_Mission09_BigGunEscortWing_SpawnReinforcements(void)
{
    GrowSelection *spawnSource = kasAITeamShipsPtrIfExists(246649714);
    GrowSelection *newShips;
    GrowSelection *escortShips;
    sdword escortCount = 0;
    char *shipType;
    sdword shipCount;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfShipsCount(spawnSource) <= 0)
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(66381540, 0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(147220289)))
    {
        escortShips = kasAITeamShipsPtrIfExists(45227969);
        escortCount = kasfShipsCount(escortShips);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220290)))
    {
        escortShips = kasAITeamShipsPtrIfExists(45227970);
        escortCount = kasfShipsCount(escortShips);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220291)))
    {
        escortShips = kasAITeamShipsPtrIfExists(45227971);
        escortCount = kasfShipsCount(escortShips);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220292)))
    {
        escortShips = kasAITeamShipsPtrIfExists(45227972);
        escortCount = kasfShipsCount(escortShips);
    }

    kasfEntrySetValue(66381540, escortCount);
    if (kasfEntryValueGet(66381540) <= 0)
    {
        return;
    }

    if (kasfRandom(1, 2) == 1)
    {
        shipType = "tHeavyInterceptor";
        shipCount = 10;
    }
    else
    {
        shipType = "tMultiGunCorvette";
        shipCount = 6;
    }

    newShips = kasGrowSelectionPtr(159882227);
    kasfShipsCreateInside(newShips, shipType, shipCount, (sdword)spawnSource, 1);
    kasAITeamShipsPtr((sdword)newShips, 2);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, newShips);
}

void Watch_Mission09_BigGunEscortWing_SpawnReinforcements(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
        return;
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            78087284,
            Init_LaunchDockedTeam,
            Watch_Mission09_BigGunEscortWing_SetWaypoint);
    }
}

void Init_Mission09_Traders_TalkingMoreCombatLoop(void)
{
    AITeam *team;
    GrowSelection *nearbyShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    nearbyShips = kasGrowSelectionPtr(57245931);
    if (!kasfShipsSelectNearby(nearbyShips, &team->shipList, kasTeamsVectorPtr(team->kasLabelHash), 15000))
    {
        kasfShipsAdd(nearbyShips, &team->shipList);
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(nearbyShips, kasGrowSelectionPtr(139967715));
    }
    kasfFormationWall();
    singlePlayerSortShipsByDistanceToPoint(nearbyShips, kasTeamsVectorPtr(team->kasLabelHash));
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission09_Traders_TalkingMoreCombatLoop(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (singlePlayerShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_Traders_TalkingMoreCombatLoop,
            (sdword)Watch_Mission09_Traders_TalkingMoreCombatLoop);
    }
}

void Init_Mission09_Traders_TalkingMoreLaunchPrep(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(-1885994875);
    kasTimerSetById(-1885994875, kasfRandom(30, 50));
    kasfEntryGetOrCreateSetValue(332667, 1);
}

void Init_Mission09_FleetIntel(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(144337492);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(232774915, 0);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfTutEnableFlags("Flags");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(
        32713509,
        (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
        (sdword)Watch_Mission09_FleetIntel_EventFlow);
}

void Watch_Mission09_FleetIntel_CleanupAndResume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
    {
        kasfEntrySetValue(-2054310428, 0);
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
            kasfCloseSensors(1);
            kasfEntrySetValue(52577235, 0);
        }
        if (kasfEntryValueGet(172907219))
        {
            kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
            kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
            kasfCameraSetDistance(kasfEntryValueGet(64272901));
            kasfTutCameraFocusCancel();
            kasfEntrySetValue(172907219, 0);
        }
        if (kasfEntryValueGet(232774915))
        {
            kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
            kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
            kasfCameraSetDistance(kasfEntryValueGet(64272901));
            kasfTutCameraFocusCancel();
            kasfTutCameraFocusCancel();
            kasfEntrySetValue(232774915, 0);
        }
        if (kasfEntryValueGet(127756738))
        {
            kasTimerStartById(144337492);
        }
        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_FleetIntel_ResumeDelay);
    }
}

void Watch_Mission09_FleetIntel_ResumeDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission09_FleetIntel_EventFlow);
    }
}

void Init_Mission09_FleetIntel_RevealResourceTeam(void)
{
    AITeam *targetTeam;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-1939595995))
    {
        kasfMusicPlay(8);
        kasfTutCameraFocus(kasGrowSelectionPtr(218060896));
        kasfCameraSetAngleDeg(205);
        kasfCameraSetDeclinationDeg(18);
        kasfCameraSetDistance(3100);
        kasfHideShips(kasAITeamShipsPtrIfExists(77795571));
        kasfShipsSwitchPlayerOwner(kasAITeamShipsPtrIfExists(77795571), 1);
        kasfUniverseUnpause();
        kasfOtherKASUnpause();
        targetTeam = kasAITeamPtrIfExists(176850956);
        kasfMsgSend(targetTeam, (char *)70838037);
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission09_FleetIntel_EventFlow);
    }
}

void Watch_Mission09_FleetIntel_EventFlow(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;
    AITeam *targetTeam;
    hvector *lookAtPoint;
    GrowSelection *focusShips;
    sdword followupMessage;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(218060896)))
    {
        kasfMissionFailed();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
        return;
    }

    queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueInfo != NULL) ? *queueInfo : -1;

    if (!kasfEntryValueGet(332667))
    {
        while (queueIndex >= 0)
        {
            message = aitQueuePopPointerByIndex(team, queueIndex);
            if ((udword)message > 0x2E80A03)
            {
                if ((udword)message > 0x85E865B)
                {
                    if (message == (void *)191311479)
                    {
                        kasfPingRemove("PingOnSensors");
                        kasfSpeechEventQueueCurrentPlayer(268444100, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(77795571), (char *)72378164);
                        followupMessage = 32453490;
                        targetTeam = kasAITeamPtrIfExists(160291907);
                        kasfMsgSend(targetTeam, (char *)followupMessage);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                        return;
                    }
                }
                else
                {
                    if (message == (void *)140412507)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444097, 0);
                        kasfPingRemove("PingOnResourcers");
                        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)152994739);
                        kasfEntrySetValue(332667, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)99116354);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                        return;
                    }
                    if (message == (void *)70838037)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444102, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)77772213);
                        kasTimerCreateSetStartById(-2036075036, 90);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                        return;
                    }
                    if (message == (void *)120574995)
                    {
                        kasTimerSetById(144337492, 3);
                        kasfEntrySetValue(127756738, 1);
                        kasfEntrySetValue(266064926, 1);
                        kasfEntrySetValue(52577235, 1);
                        kasfEntrySetValue(168240451, 1);
                        kasfOtherKASPause();
                        kasfUniversePause();
                        kasfEntrySetValue(211831571, 1);
                        kasfEntrySetValue(109086661, 1);
                        if (!kasfSensorsIsOpen(0))
                        {
                            kasfOpenSensors(150000);
                        }
                        lookAtPoint = kasVectorPtrIfExists(48187763);
                        kasfSensorsSetLookatPoint(lookAtPoint);
                        kasfPingAddPoint(kasVectorPtrIfExists(48187763), "PingOnResourcers");
                        kasfSpeechEventQueueCurrentPlayer(268444096, 0);
                        followupMessage = 120574995;
                        targetTeam = kasAITeamPtrIfExists(160291907);
                        kasfMsgSend(targetTeam, (char *)followupMessage);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                        return;
                    }
                }
            }
            else
            {
                if (message == (void *)48761347)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfSpeechEvent(
                        86933668,
                        (sdword)Init_Mission09_FleetIntel_PauseWorld,
                        (sdword)Init_Mission09_FleetIntel_RevealResourceTeam);
                    return;
                }
                if ((udword)message <= 0x1F40B84)
                {
                    if (message == (void *)32770948)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268440008, 0);
                        kasfSpeechEvent(
                            77979796,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_FleetIntel_WaitForIntelEventEnd);
                        return;
                    }
                    if (message == (void *)357797)
                    {
                        kasfEntrySetValue(332667, 1);
                        kasfSpeechEventQueueCurrentPlayer(268444106, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x575A62);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                        return;
                    }
                }
            }

            if (message != NULL)
            {
                aitQueuePointer(team, team, message);
            }
            queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
            if (queueInfo == NULL || *queueInfo == 0 || (--queueIndex < 0))
            {
                return;
            }
        }
        return;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if ((udword)message > 0x57489D4)
        {
            if (message == (void *)99116354)
            {
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)114467138);
                kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_FleetIntel_ResumeDelay);
                return;
            }
            if (message == (void *)114467138)
            {
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(127756738, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(332667, 0);
                kasfOtherKASPause();
                kasfUniversePause();
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(109086661, 1);
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(150000);
                }
                lookAtPoint = kasTeamsVectorPtr(128156707);
                kasfSensorsSetLookatPoint(lookAtPoint);
                kasfPingAddShipsIndexed(kasAITeamShipsPtrIfExists(128156707), "PingOnSensors");
                kasfSpeechEventQueueCurrentPlayer(268444098, 0);
                followupMessage = 114467138;
                targetTeam = kasAITeamPtrIfExists(160291907);
                kasfMsgSend(targetTeam, (char *)followupMessage);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                return;
            }
        }
        else
        {
            if (message == (void *)91523540)
            {
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(127756738, 1);
                kasfEntrySetValue(266064926, 1);
                kasfOtherKASPause();
                kasfUniversePause();
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(109086661, 1);
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(150000);
                }
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(172907219, 1);
                kasfPingAddShipsIndexed(kasAITeamShipsPtrIfExists(77795571), "PingOnResourcers");
                kasfEntrySetValue(332667, 0);
                kasfSpeechEventQueueCurrentPlayer(268444103, 0);
                followupMessage = 91523540;
                targetTeam = kasAITeamPtrIfExists(160291907);
                kasfMsgSend(targetTeam, (char *)followupMessage);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                return;
            }
            if (message == (void *)0x575A62)
            {
                kasfSpeechEventQueueCurrentPlayer(268440009, 0);
                kasfAllowPlayerToResearch("TECH_ENERGY");
                kasfAllowPlayerToResearch("TECH_ION");
                followupMessage = 175938357;
                targetTeam = kasAITeamPtrIfExists(160291907);
                kasfMsgSend(targetTeam, (char *)followupMessage);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                return;
            }
            if (message == (void *)48761353)
            {
                kasfMusicPlay(19);
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                focusShips = kasGrowSelectionPtr(77795571);
                kasfTutCameraFocusClose(focusShips);
                kasfSpeechEventQueueCurrentPlayer(268468686, 0);
                kasfEntryGetOrCreateSetValue(-2089378044, 1);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)91523540);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission09_FleetIntel_CleanupAndResume);
                return;
            }
        }

        if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }
        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || (--queueIndex < 0))
        {
            return;
        }
    }
}

void Watch_Mission09_ObjectiveEventFlow(void)
{
    sdword queueIndex = -1;

    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (((AITeam *)kasCurrentTeamPtr)->msgQueue != NULL)
    {
        queueIndex = ((AITeam *)kasCurrentTeamPtr)->msgQueue->head;
    }
    if (queueIndex < 0)
    {
        return;
    }

    while (1)
    {
        udword message = (udword)aitQueuePopPointerByIndex((AITeam *)kasCurrentTeamPtr, queueIndex);
        switch (message)
        {
            case 0:
                break;
            case 0x1EF3372:
                kasfObjectiveDestroy("DestroySensor");
                kasfObjectiveCreate("Wait", ((char **)0x8CB124)[strCurLanguage], ((char **)0x8CB130)[strCurLanguage]);
                break;
            case 0x34DBEE4:
                kasfMissionFailed();
                break;
            case 77772213:
                kasfObjectiveDestroy("Wait");
                kasfObjectiveCreate("Engage", ((char **)0x8CB13C)[strCurLanguage], ((char **)0x8CB148)[strCurLanguage]);
                break;
            case 91523540:
                kasfObjectiveDestroy("Engage");
                kasfObjectiveCreate("Protect", ((char **)0x8CB154)[strCurLanguage], ((char **)0x8CB160)[strCurLanguage]);
                break;
            case 175938357:
                kasfMusicPlay(8);
                kasfPingRemove("PingOnResourcers");
                objectiveDestroyAll();
                kasfObjectiveCreate("Hyperspace", ((char **)0x8CB16C)[strCurLanguage], ((char **)0x8CB178)[strCurLanguage]);
                spSetSinglePlayerSaveInfo(9, ((char **)0x8CB0E8)[strCurLanguage]);
                break;
            case 0x6D2A142:
                kasfObjectiveCreate("DestroySensor", ((char **)0x8CB10C)[strCurLanguage], ((char **)0x8CB118)[strCurLanguage]);
                break;
            case 0x72FD413:
                kasfObjectiveCreate("MoveTransports", ((char **)0x8CB0F4)[strCurLanguage], ((char **)0x8CB100)[strCurLanguage]);
                break;
            case 0x91E83B3:
                kasfObjectiveDestroy("MoveTransports");
                break;
            default:
                aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, (void *)message);
                break;
        }

        if (((AITeam *)kasCurrentTeamPtr)->msgQueue == NULL
            || ((AITeam *)kasCurrentTeamPtr)->msgQueue->head == 0
            || --queueIndex < 0)
        {
            return;
        }
    }
}

void Init_Mission09_EventState(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(144337492);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(35614828, 0);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfEntryGetOrCreateSetValue(175010388, 0);
    kasfEntryGetOrCreateSetValue(126773838, 1);
    kasfEntryGetOrCreateSetValue(77979796, 0);
    kasfTutEnableFlags("Flags");
    kasfTutEnableFlags("CancelFocus");
    kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)5801588);
    kasfMusicPlay(9);
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission09_GlobalEvents);
}

void Watch_Mission09_WaitForIntelEventEnd(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
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
        kasfEntrySetValue(172907219, 0);
    }
    if (kasfEntryValueGet(35614828))
    {
        kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
        kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
        kasfCameraSetDistance(kasfEntryValueGet(64272901));
        kasfEntrySetValue(35614828, 0);
    }

    if (kasfEntryValueGet(127756738))
    {
        kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
    }
    else if (kasfEntryValueGet(175010388))
    {
        kasfSpeechEvent(237711556, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_BigGunSequenceEvents);
    }
    else if (kasfEntryValueGet(77979796))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
    }
    else
    {
        kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission09_GlobalEvents);
    }
}

void Watch_Mission09_PostIntelBranch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)220350372))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
    }
    else if (kasTimerExpiredDestroyById(144337492))
    {
        if (kasfEntryValueGet(175010388))
        {
            kasfSpeechEvent(237711556, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_BigGunSequenceEvents);
        }
        else
        {
            kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission09_GlobalEvents);
        }
    }
}

void Watch_Mission09_BigGunSequenceEvents(void)
{
    sdword queueIndex = -1;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (((AITeam *)kasCurrentTeamPtr)->msgQueue != NULL)
    {
        queueIndex = ((AITeam *)kasCurrentTeamPtr)->msgQueue->head;
    }
    if (queueIndex < 0)
    {
        return;
    }

    while (1)
    {
        udword message = (udword)aitQueuePopPointerByIndex((AITeam *)kasCurrentTeamPtr, queueIndex);
        if (message > 0x7A62704)
        {
            if (message > 0xA4DB0B6)
            {
                if (message == 243202403)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(216270495));
                    kasfCameraSetAngleDeg(45);
                    kasfCameraSetDeclinationDeg(-20);
                }
                else
                {
                    aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, (void *)message);
                }
            }
            else if (message == 172863670)
            {
                hvector *lookAtPoint = NULL;

                kasfOpenSensors(100000);
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(126773838, 0);
                if (kasfShipsCount(kasAITeamShipsPtrIfExists(155996337)))
                {
                    kasfTutSetPointerTargetFirstShip((char *)0x8CBB10, kasAITeamShipsPtrIfExists(155996337));
                    lookAtPoint = kasTeamsVectorPtr(155996337);
                }
                else if (kasfShipsCount(kasAITeamShipsPtrIfExists(127661745)))
                {
                    kasfTutSetPointerTargetFirstShip((char *)0x8CBB10, kasAITeamShipsPtrIfExists(127661745));
                    lookAtPoint = kasTeamsVectorPtr(127661745);
                }
                if (lookAtPoint != NULL)
                {
                    kasfSensorsSetLookatPoint(lookAtPoint);
                }
                kasfSpeechEventQueueCurrentPlayer(268444138, 0);
                kasfEntrySetValue(168240451, 1);
                kasfEntrySetValue(52577235, 1);
                kasfMsgSendAll((char *)88675620);
                kasfEntryGetOrCreateSetValue(-2066633436, 1);
                kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)69919236);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            else if (message == 128717902)
            {
                kasfMsgSend(kasAITeamPtrIfExists(170794736), (char *)162731038);
                kasfEntrySetValue(127756738, 8);
                kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)45991008);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                return;
            }
            else if (message == 130015651)
            {
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(216270495));
                kasfMsgSend(kasAITeamPtrIfExists(170794736), (char *)105899523);
                kasfEntrySetValue(127756738, 10);
                kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)172863670);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                return;
            }
            else
            {
                aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, (void *)message);
            }
        }
        else if (message == 128329476)
        {
            kasfMsgSend(kasAITeamPtrIfExists(216270495), (char *)219752559);
            spEventWideScreenIn(90);
            kasfEntrySetValue(127756738, 3);
            kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)243202403);
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
            return;
        }
        else if (message > 0x2BDC460)
        {
            if (message == 69919236)
            {
                kasfEntrySetValue(175010388, 0);
                kasfTutCameraFocusCancel();
                kasfTutCameraFocusCancel();
                kasfTutCameraFocusCancel();
                kasfTutCameraFocusCancel();
                kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
                kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
                kasfCameraSetDistance(kasfEntryValueGet(64272901));
                kasfWideScreenOut(90);
                kasfMsgSend(kasAITeamPtrIfExists(170794736), (char *)219719732);
                kasfPingAddShips(kasAITeamShipsPtrIfExists(170794736), "Ping");
                kasfMsgSend(kasAITeamPtrIfExists(124217570), (char *)75188228);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                return;
            }
            else
            {
                aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, (void *)message);
            }
        }
        else if (message == 45991008)
        {
            kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(170794736));
            kasfSpeechEventQueueCurrentPlayer(268440050, 0);
            kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)130015651);
            kasfMsgSend(kasAITeamPtrIfExists(216270495), (char *)208701776);
            kasfEntrySetValue(127756738, 10);
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
            return;
        }
        else if (message == 18005407)
        {
            kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(90274303));
            kasfCameraSetAngleDeg(60);
            kasfCameraSetDeclinationDeg(20);
            kasfSpeechEventQueueCurrentPlayer(268452556, 0);
            kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)128717902);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
            return;
        }
        else if (message != 0)
        {
            aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, (void *)message);
        }

        if (((AITeam *)kasCurrentTeamPtr)->msgQueue == NULL
            || ((AITeam *)kasCurrentTeamPtr)->msgQueue->head == 0
            || --queueIndex < 0)
        {
            return;
        }
    }
}

void Watch_Mission09_GlobalEvents(void)
{
    AITeam *currentTeam;
    sdword *msgQueueHead;
    sdword queueIndex;
    udword message;
    sdword queueState;
    AITeam *targetTeam;
    sdword followupMessage;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    msgQueueHead = (sdword *)currentTeam->msgQueue;
    queueIndex = -1;
    if (msgQueueHead != NULL)
    {
        queueIndex = *msgQueueHead;
    }
    if (queueIndex < 0)
    {
        return;
    }

    while (1)
    {
        message = (udword)aitQueuePopPointerByIndex(currentTeam, queueIndex);
        if (message <= 0x5886752)
        {
            if (message == 92825426)
            {
                kasfMsgSend(currentTeam, (char *)92825427);
                spSetSinglePlayerSaveInfo(10, ((char **)0x8CB600)[strCurLanguage]);
                goto next_message;
            }
            if (message > 0x394AFF2)
            {
                if (message > 0x4765DF3)
                {
                    if (message == 75204180)
                    {
                        kasfEntrySetValue(175010388, 1);
                        kasfMsgSend(currentTeam, (char *)128329476);
                        kasfMusicPlay(22);
                        kasfSpeechEvent(237711556, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_BigGunSequenceEvents);
                        return;
                    }
                    if (message == 78338137)
                    {
                        kasfPingRemove("PingOnBigGun");
                        kasfSpeechEventQueueCurrentPlayer(268444130, 0);
                        kasfEntrySetValue(77979796, 1);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                        return;
                    }
                    if (message == 88675620)
                    {
                        kasfMsgSend(currentTeam, (char *)4679134);
                        goto next_message;
                    }
                    goto requeue_message;
                }
                if (message == 74866163)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfShipsSelectNearby(
                        kasGrowSelectionPtr(57245931),
                        kasGrowSelectionPtr(18336307),
                        kasTeamsVectorPtr(170794736),
                        8000);
                    kasfShipsSelectRace(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(57245931), 1);
                    kasfShipsRemove(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(363040));
                    kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(90274303));
                    if (kasfShipsCount(kasGrowSelectionPtr(57245931)))
                    {
                        kasfShipsMakeCrazy(kasGrowSelectionPtr(57245931), 0);
                    }
                    kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(170794736));
                    kasfSpeechEventQueueCurrentPlayer(268452331, 0);
                    kasfMsgSend(currentTeam, (char *)375908);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                    return;
                }
                if (message <= 0x43F9545)
                {
                    if (message == 71275845)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444136, 0);
                        kasfMsgSend(currentTeam, (char *)183712981);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                        return;
                    }
                    if (message == 61077973)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268452337, 0);
                        kasfMsgSend(currentTeam, (char *)175938357);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                        return;
                    }
                    if (message == 66232770)
                    {
                        if (!kasfSensorsIsOpen(0))
                        {
                            kasfOpenSensors(100000);
                            kasfEntrySetValue(126773838, 0);
                        }
                        kasfUniversePause();
                        kasfOtherKASPause();
                        kasfEntrySetValue(109086661, 1);
                        kasfEntrySetValue(211831571, 1);
                        kasfSpeechEventQueueCurrentPlayer(268440031, 0);
                        kasfTutSetPointerTargetFirstShip((char *)0x8CBB10, kasAITeamShipsPtrIfExists(230547090));
                        kasfSensorsSetLookatPoint(kasTeamsVectorPtr(230547090));
                        kasfEntrySetValue(52577235, 1);
                        kasfEntrySetValue(168240451, 1);
                        kasfEntrySetValue(266064926, 1);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                        return;
                    }
                    goto requeue_message;
                }
                if (message == 74866162)
                {
                    spEventWideScreenIn(90);
                    kasfEntrySetValue(127756738, 3);
                    kasfMsgSend(currentTeam, (char *)74866163);
                    kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                    return;
                }
                goto requeue_message;
            }
            if (message != 60076018)
            {
                if (message > 0x61C2E7)
                {
                    if (message > 0x3394AFB)
                    {
                        if (message == 54471198)
                        {
                            spEventWideScreenIn(90);
                            kasfEntrySetValue(127756738, 3);
                            kasfMsgSend(currentTeam, (char *)66232770);
                            kasfMusicPlay(21);
                            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                            return;
                        }
                    }
                    else
                    {
                        if (message == 54086395)
                        {
                            kasfEntrySetValue(127756738, 3);
                            kasfWideScreenIn(90);
                            targetTeam = kasAITeamPtrIfExists(176850956);
                            followupMessage = 164156322;
                            goto send_to_target_and_delay;
                        }
                        if (message == 0x64BE02)
                        {
                            kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                            kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                            kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                            kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(48009329));
                            kasfCameraSetAngleDeg(188);
                            kasfCameraSetDeclinationDeg(23);
                            kasfCameraSetDistance(600);
                            kasfMsgSend(kasAITeamPtrIfExists(48009329), (char *)46655475);
                            kasfMsgSend(kasAITeamPtrIfExists(48009330), (char *)46655475);
                            kasfMsgSend(kasAITeamPtrIfExists(48009331), (char *)46655475);
                            kasfMsgSend(kasAITeamPtrIfExists(48009332), (char *)46655475);
                            kasfMsgSend(kasAITeamPtrIfExists(48009333), (char *)46655475);
                            kasfMsgSend(kasAITeamPtrIfExists(48009334), (char *)46655475);
                            kasfMsgSend(currentTeam, (char *)122421411);
                            kasfEntrySetValue(127756738, 3);
                            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                            return;
                        }
                        if (message == 29410707)
                        {
                            kasfMusicPlay(9);
                            kasfSpeechEventQueueCurrentPlayer(268444132, 0);
                            kasfAllowPlayerToResearch("TECH_ADVANCED_SENSORS");
                            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                            return;
                        }
                    }
                    goto requeue_message;
                }
                if (message == 0x61C2E7)
                {
                    kasfSpeechEventQueueCurrentPlayer(268444128, 0);
                    kasfEntrySetValue(127756738, 60);
                    kasfMsgSend(currentTeam, (char *)29410707);
                    kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                    return;
                }
                if (message <= 0x4765DE)
                {
                    if (message == 0x4765DE)
                    {
                        if (kasfEntryValueGet(-2062465132))
                        {
                            kasfMsgSend(currentTeam, (char *)74866162);
                            goto next_message;
                        }
                        kasfMsgSend(currentTeam, (char *)4679134);
                        goto next_message;
                    }
                    if (message == 0)
                    {
                        goto next_message;
                    }
                    if (message == 375908)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268440044, 0);
                        kasfMsgSend(currentTeam, (char *)185906455);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                        return;
                    }
                    goto requeue_message;
                }
                if (message == 0x588674)
                {
                    kasfLocationCard(4000, ((char **)0x8CB5F4)[strCurLanguage]);
                    kasfMsgSend(currentTeam, (char *)92825426);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                    return;
                }
                goto requeue_message;
            }
            if (!kasfEntryValueGet(71552161))
            {
                kasfEntryGetOrCreateSetValue(71552161, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(230547091));
                kasfSpeechEventQueueCurrentPlayer(268440038, 0);
                kasfEntrySetValue(127756738, 45);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(35614828, 1);
                kasfMsgSend(currentTeam, (char *)173902997);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            goto next_message;
        }
        if (message <= 0xAF33CD5)
        {
            if (message == 183712981)
            {
                kasfSpeechEventQueueCurrentPlayer(268501481, 0);
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(168240451, 1);
                kasfEntrySetValue(266064926, 1);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            if (message > 0x74C00A3)
            {
                if (message == 164156322)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(127756738, 3);
                    kasfSpeechEventQueueCurrentPlayer(268440037, 0);
                    kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(124217378));
                    kasfCameraSetAngleDeg(0);
                    kasfCameraSetDeclinationDeg(-5);
                    kasfEntrySetValue(172907219, 1);
                    kasfEntrySetValue(35614828, 1);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                    return;
                }
                if (message == 173902997)
                {
                    kasfMusicPlay(9);
                    spEventWideScreenIn(90);
                    kasfEntrySetValue(127756738, 3);
                    kasfMsgSend(currentTeam, (char *)118286532);
                    targetTeam = kasAITeamPtrIfExists(90274303);
                    followupMessage = 173902997;
                    goto send_to_target_and_delay;
                }
                if (message == 175938357)
                {
                    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
                    goto next_message;
                }
            }
            else if (message == 122421411)
            {
                kasfSpeechEventQueueCurrentPlayer(268444129, 0);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(35614828, 1);
                kasfEntrySetValue(266064926, 1);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            else if (message > 0x6FC9975)
            {
                if (message == 118286532)
                {
                    if (!kasfSensorsIsOpen(0))
                    {
                        kasfOpenSensors(0);
                        kasfEntrySetValue(126773838, 0);
                    }
                    kasfUniversePause();
                    kasfOtherKASPause();
                    kasfSensorsSetLookatPoint(kasTeamsVectorPtr(90274303));
                    kasfTutSetPointerTargetFirstShip((char *)0x8CBB10, kasAITeamShipsPtrIfExists(90274303));
                    kasfSpeechEventQueueCurrentPlayer(268439854, 0);
                    kasfMsgSend(currentTeam, (char *)209347103);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                    return;
                }
            }
            else
            {
                if (message == 117217653)
                {
                    kasfSpeechEventQueueCurrentPlayer(268440047, 0);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                    return;
                }
                if (message == 92825427)
                {
                    kasfEntrySetValue(-2054310428, 0);
                    kasfSpeechEventQueueCurrentPlayer(268444126, 0);
                    kasfEntrySetValue(266064926, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)91742846);
                    kasfPingAddSingleShip(kasAITeamShipsPtrIfExists(74300574), "PingOnBigGun");
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                    return;
                }
                if (message == 115858036)
                {
                    kasfPingRemove("Ping");
                    kasfDisableMothershipBigGunLaunch();
                    kasfMsgSend(currentTeam, (char *)243115826);
                    kasfEntrySetValue(127756738, 5);
                    kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                    return;
                }
            }
            goto requeue_message;
        }
        if (message > 0xD2247A4)
        {
            if (message == 243115826)
            {
                kasfSpeechEventQueueCurrentPlayer(268444144, 0);
                kasfEntrySetValue(127756738, 2);
                kasfMsgSend(currentTeam, (char *)61077973);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            if (message == 252070894)
            {
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(currentTeam, (char *)6602242);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
                return;
            }
            if (message == 265345332)
            {
                kasfPingRemove("PingOnBigGun");
                kasfSpeechEventQueueCurrentPlayer(268444131, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)118774846);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            goto requeue_message;
        }
        if (message == 220350372)
        {
            kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
            return;
        }
        if (message > 0xC9C8D35)
        {
            if (message == 214309925)
            {
                kasfSpeechEventQueueCurrentPlayer(268444142, 0);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(35614828, 1);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)214309925);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
                return;
            }
            goto requeue_message;
        }
        if (message == 211586357)
        {
            kasfMusicPlay(21);
            spEventWideScreenIn(90);
            kasfEntrySetValue(127756738, 3);
            targetTeam = kasAITeamPtrIfExists(176850956);
            followupMessage = 164156322;
            goto send_to_target_and_delay;
        }
        if (message == 185906455)
        {
            kasfSpeechEventQueueCurrentPlayer(268444141, 0);
            kasfMsgSend(currentTeam, (char *)214309925);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
            return;
        }
        if (message == 209347103)
        {
            kasfSpeechEventQueueCurrentPlayer(268501479, 0);
            kasfMsgSend(currentTeam, (char *)71275845);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission09_WaitForIntelEventEnd);
            return;
        }
        goto requeue_message;

send_to_target_and_delay:
        kasfMsgSend(targetTeam, (char *)followupMessage);
        kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, (sdword)Watch_Mission09_PostIntelBranch);
        return;

requeue_message:
        aitQueuePointer(currentTeam, currentTeam, (void *)message);

next_message:
        currentTeam = (AITeam *)kasCurrentTeamPtr;
        queueState = (sdword)currentTeam->msgQueue;
        if (queueState == 0 || *(sdword *)queueState == 0 || --queueIndex < 0)
        {
            return;
        }
    }
}



/*=== Watch_Mission09_Traders_Talking (0x62D4B0) ===*/
void Watch_Mission09_Traders_Talking(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_CruiseMissileStrikeWave,
            (sdword)Watch_CruiseMissileStrikeWave);
    }
}

/*=== Watch_Mission09_Traders_TalkingMoreLaunchPrep (0x5e8d30) ===*/
void Watch_Mission09_Traders_TalkingMoreLaunchPrep(void)
{
    AITeam *reserveTeam;
    GrowSelection *paradeCorvettes;
    sdword shipsToLaunch;

    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    paradeCorvettes = kasGrowSelectionPtr(0x593584);
    if (kasfShipsSelectClass(paradeCorvettes, kasGrowSelectionPtr(0x6DA2DA9), "CLASS_Corvette")
        && kasfShipsSelectSinglePlayerInParade(paradeCorvettes, paradeCorvettes))
    {
        reserveTeam = kasAITeamPtrIfExists(49029573);
        kasfReinforceTeamWithShips(reserveTeam, paradeCorvettes);
        kasfShipsAdd(paradeCorvettes, kasAITeamShipsPtrIfExists(239669557));
        kasfShipsSetFormation(paradeCorvettes, 8);
        kasfTacticsAggressive();
        kasTimerStartById(-1885994875);
    }

    reserveTeam = kasAITeamPtrIfExists(49029573);
    if (!kasfThisTeamIs(reserveTeam)
        && !kasfTeamCount()
        && kasfShipsCount(kasAITeamShipsPtrIfExists(49029573)) >= 2
        && kasfShipsSelectFirstN(
            kasGrowSelectionPtr(0x593584),
            kasAITeamShipsPtrIfExists(49029573),
            kasfShipsCount(kasAITeamShipsPtrIfExists(49029573)))
        && (kasTimerExpiredDestroyById(-1885994875) || kasfMsgReceived((char *)0xAA768C5)))
    {
        shipsToLaunch = kasfRandom(30, 50);
        kasTimerSetById(-1885994875, shipsToLaunch);
        kasfShipsRemove(kasAITeamShipsPtrIfExists(49029573), kasGrowSelectionPtr(0x593584));
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(0x593584));
        kasfTacticsNeutral();
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_Traders_TalkingMoreCombatLoop,
            (sdword)Watch_Mission09_Traders_TalkingMoreCombatLoop);
    }
    else if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfTeamHealthAverage() < 30)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_Mission09_Traders_TalkingMoreLaunch,
                    (sdword)Watch_Mission09_Traders_TalkingMoreLaunch);
            }
            else if (kasfMsgReceived((char *)0x469857B))
            {
                kasfSpeechEvent(
                    74024315,
                    (sdword)Init_Mission09_Traders_TalkingMoreCombatLoop,
                    (sdword)Watch_Mission09_Traders_TalkingMoreCombatLoop);
            }
        }
        else
        {
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        }
    }
}

void Begin_Mission09_Traders_TalkingMoreLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfFormationWall();
    kasfDockStayWithTeam(kasAITeamPtrIfExists(239669557));
}

/*=== Watch_Mission09_Traders_TalkingMoreLaunch (0x5e9150) ===*/
void Watch_Mission09_Traders_TalkingMoreLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfLaunch();
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, 0);
    }
    else if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Watch_Mission09_GhostShip_Dormant(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasTimerExpiredById(148930818))
    {
        kasfDeleteShips(&teamtoreinforce->shipList);
    }
}

void Init_Mission09_AssaultEscort_Cooldown(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
}

/*=== Init_Mission09_AssaultEscort (0x607650) ===*/
void Init_Mission09_AssaultEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 0);
    if ((GetGameDifficulty() < 2 && kasfThisTeamIs(kasAITeamPtrIfExists(164953350)))
        || (GetGameDifficulty() < 3 && kasfThisTeamIs(kasAITeamPtrIfExists(164953351)))
        || (GetGameDifficulty() < 3 && kasfThisTeamIs(kasAITeamPtrIfExists(164953352))))
    {
        kasfDeleteShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)MaintainTeamContext);
    }
    else
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
}

/*=== Watch_Mission09_AssaultEscort (0x607780) ===*/
void Watch_Mission09_AssaultEscort(void)
{
    if (!kasfEntryValueGet(332667))
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)))
        {
            if (!kasfTeamCount())
            {
                kasfSpeechEvent(
                    4693508,
                    (sdword)Init_Mission09_AssaultEscort_ReinforcementDelay,
                    (sdword)Watch_Mission09_AssaultEscort_SpawnHeavyInterceptorReinforcement);
                return;
            }
            if (kasfTeamHealthAverage() < 40)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Init_Mission09_AssaultEscort_DockForLaunch,
                    (sdword)Watch_Mission09_AssaultEscort_DockForLaunch);
                return;
            }
        }
        else if (!kasfTeamCount())
        {
            kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)MaintainTeamContext);
            return;
        }

        if (!kasfEntryValueGet(-2085181655)
            && !kasfEntryValueGet(-1978936548)
            && kasfShipsSelectInsideVolume(
                kasGrowSelectionPtr(0x7D1F203),
                kasGrowSelectionPtr(0x7C0B279),
                kasVolumePtr(0x7A38423)))
        {
            kasfEntryGetOrCreateSetValue(-1978936548, 1);
            kasfSpeechEvent(
                4590196,
                (sdword)Init_Mission09_AssaultEscort_InterceptNearbyContacts,
                (sdword)Watch_Mission09_AssaultEscort_InterceptNearbyContacts);
        }
    }
}

void Init_Mission09_AssaultEscort_SetRandomWaypoint(void)
{
    hvector *randomizeSource;

    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(71552164))
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfLabelledVectorSet(228413404, 0, 0, 0);
    }

    switch (kasfRandom(0, 3))
    {
        case 0:
            randomizeSource = kasVectorPtrIfExists(80307793);
            break;
        case 1:
            randomizeSource = kasVectorPtrIfExists(80307794);
            break;
        case 2:
            randomizeSource = kasVectorPtrIfExists(80307795);
            break;
        default:
            randomizeSource = kasVectorPtrIfExists(80307796);
            break;
    }

    kasfVectorRandomizeInRange(kasVectorPtrIfExists(228413404), randomizeSource, 8000);
    kasfFormationClaw();
    kasfTacticsNeutral();
    kasfMoveTo(kasVectorPtrIfExists(228413404));
}

/*=== Watch_Mission09_AssaultEscort_WaypointLoop (0x607980) ===*/
void Watch_Mission09_AssaultEscort_WaypointLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(128156707), kasGrowSelectionPtr(0x15F6D3))
        || kasfShipsSelectNearby(
            kasGrowSelectionPtr(0xDF4CEAB),
            kasGrowSelectionPtr(0x7C0B279),
            kasShipsVectorPtr((char *)0x7530),
            0)
        || kasfEntryValueGet(-2085181655))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission09_AssaultEscort_AttackRun,
            (sdword)Watch_Mission09_AssaultEscort_AttackRun);
    }
    else if (kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(246649714), kasGrowSelectionPtr(0x16077D3)))
    {
        kasfSpeechEvent(
            76130852,
            (sdword)Init_Mission09_AssaultEscort_Repath,
            (sdword)Watch_Mission09_AssaultEscort_Repath);
    }
    else if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
}

/*=== Watch_Mission09_AssaultEscort_Repath (0x607DB0) ===*/
void Begin_Mission09_AssaultEscort_AttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(128156707), kasGrowSelectionPtr(0x15F6D3))
        && !kasfEntryValueGet(-2085181655))
    {
        kasfFormationClaw();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x15F6D3), kasShipsVectorPtr((char *)0x15F6D3));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x15F6D3));
    }
    else if (kasfShipsSelectNearby(
                 kasGrowSelectionPtr(0xDF4CEAB),
                 kasGrowSelectionPtr(0x7C0B279),
                 kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
                 30000))
    {
        kasfFormationClaw();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0xDF4CEAB), kasShipsVectorPtr((char *)0xDF4CEAB));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0xDF4CEAB));
    }
    else if (kasfEntryValueGet(-2085181655) && kasfEntryValueGet(-2081014012))
    {
        kasfFormationClaw();
        kasfGrowSelectionClear(kasGrowSelectionPtr(0x36980EB));
        kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasAITeamShipsPtrIfExists(77795571));
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasShipsVectorPtr((char *)0x36980EB));
        if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
        {
            kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x857BCE3));
        }
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasShipsVectorPtr((char *)0x36980EB));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
    }
    else
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
        return;
    }

    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

/*=== Watch_Mission09_AssaultEscort_Repath (0x607DB0) ===*/
void Watch_Mission09_AssaultEscort_Repath(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
    else if (kasTimerExpiredById(76130852))
    {
        kasfSpeechEvent(
            76130852,
            (sdword)Init_Mission09_AssaultEscort_Repath,
            (sdword)Watch_Mission09_AssaultEscort_Repath);
    }
}

/*=== Init_Mission09_AssaultEscort_ReinforcementDelay (0x607E20) ===*/
void Init_Mission09_AssaultEscort_Repath(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(246649714), kasGrowSelectionPtr(0x16077D3)))
    {
        kasTimerDestroyById(76130852);
        kasTimerCreateSetStartById(76130852, kasfRandom(20, 40));
        kasfFormationClaw();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x16077D3), kasShipsVectorPtr((char *)0x16077D3));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x16077D3));
    }
    else
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
}

/*=== Init_Mission09_AssaultEscort_ReinforcementDelay (0x607E20) ===*/
void Init_Mission09_AssaultEscort_ReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, kasfRandom(30, 60));
}

/*=== Watch_Mission09_AssaultEscort_SpawnHeavyInterceptorReinforcement (0x607E70) ===*/
void Watch_Mission09_AssaultEscort_SpawnHeavyInterceptorReinforcement(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)) && kasTimerExpiredById(4693508))
    {
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "tHeavyInterceptor",
            (char *)kasfRandom(7, 12),
            (sdword)kasAITeamShipsPtrIfExists(246649714),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(154639956));
    }

    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
}

void Init_Mission09_AssaultEscort_DockForLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfDockStayWithTeam(kasAITeamPtrIfExists(246649714));
}

/*=== Watch_Mission09_AssaultEscort_DockForLaunch (0x607F90) ===*/
void Watch_Mission09_AssaultEscort_DockForLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfSpeechEvent(84320888, (sdword)Init_LaunchDockedTeam, (sdword)Watch_Mission09_AssaultEscort_PostLaunch);
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)))
    {
        kasfEntrySetValue(332667, 0);
        kasfLaunch();
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
}

/*=== Watch_Mission09_AssaultEscort_PostLaunch (0x608030) ===*/
void Watch_Mission09_AssaultEscort_PostLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(246649714)))
    {
        kasfEntrySetValue(332667, 0);
        kasfLaunch();
    }
    else
    {
        return;
    }

    kasfSpeechEvent(
        88512316,
        (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
        (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
}

/*=== Init_Mission09_IonFrigateEscort (0x6088C0) ===*/
void Init_Mission09_AssaultEscort_InterceptNearbyContacts(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectInsideVolume(
            kasGrowSelectionPtr(0x401F203),
            kasGrowSelectionPtr(0x7C0B279),
            kasVolumePtr(0x7A38423)))
    {
        kasfEntrySetValue(-1978936548, 1);
        kasTimerDestroyById(4590196);
        kasTimerCreateSetStartById(4590196, kasfRandom(20, 40));
        kasfFormationClaw();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x401F203), kasShipsVectorPtr((char *)0x401F203));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x401F203));
    }
    else
    {
        kasfEntrySetValue(-1978936548, 0);
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
}

/*=== Init_Mission09_IonFrigateEscort (0x6088C0) ===*/
void Init_Mission09_IonFrigateEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!GetGameDifficulty())
    {
        kasfShipsSelectType(kasGrowSelectionPtr(0x5425), &((AITeam *)kasCurrentTeamPtr)->shipList, "tStandardFrigate");
        kasfDeleteShips(kasGrowSelectionPtr(0x5425));
    }
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(
        374244,
        (sdword)Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup,
        (sdword)Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup);
}

/*=== Watch_Mission09_IonFrigateEscort (0x608950) ===*/
void Watch_Mission09_IonFrigateEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(44672946)))
        {
            if (!kasfTeamCount())
            {
                kasfSpeechEvent(
                    4693508,
                    (sdword)Init_Mission09_MultigunCorvetteEscort_ReinforcementDelay,
                    (sdword)Watch_Mission09_IonFrigateEscort_SpawnReinforcement);
            }
        }
        else if (!kasfTeamCount())
        {
            kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)MaintainTeamContext);
        }
    }
}

void Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup(void)
{
hvector *activeStrikePoint;

kasCurrentStep = 2;
kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
kasfFormationWall();
if (kasfThisTeamIs(kasAITeamPtrIfExists(73198753)))
{
activeStrikePoint = kasVolumeVectorPtr(93722049);
}
else if (kasfThisTeamIs(kasAITeamPtrIfExists(73198754)))
{
activeStrikePoint = kasVolumeVectorPtr(93722050);
}
else if (kasfThisTeamIs(kasAITeamPtrIfExists(73198755)))
{
activeStrikePoint = kasVolumeVectorPtr(93722051);
}
else
{
activeStrikePoint = kasVolumeVectorPtr(93722052);
}

kasLabelledVectorSet(354644, activeStrikePoint);
kasfLabelledVectorSet(363040, 40000, 0, 0);
kasfVectorAddClampToUniverse(kasVectorPtrIfExists(354644), kasVectorPtrIfExists(363040));
kasfMoveTo(kasVectorPtrIfExists(354644));
}

void Begin_Mission09_IonFrigateEscort_AttackRun(void)
{
    sdword regionLabel;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfThisTeamIs(kasAITeamPtrIfExists(73198753)))
    {
        regionLabel = 0x59615C1;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(73198754)))
    {
        regionLabel = 0x59615C2;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(73198755)))
    {
        regionLabel = 0x59615C3;
    }
    else
    {
        regionLabel = 0x59615C4;
    }

    kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0x3B41EF5), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(regionLabel));
    if (kasfShipsCount(kasGrowSelectionPtr(0x3B41EF5)) && !kasfEntryValueGet(-2085181655))
    {
        if (kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(128156707), kasGrowSelectionPtr(0x15F6D3))
            && kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr(0x15F6D3),
                kasGrowSelectionPtr(0x15F6D3),
                kasGrowSelectionPtr(0x3B41EF5)))
        {
            kasfFormationWall();
            kasfShipsSortByDistanceToPoint(
                kasGrowSelectionPtr(0x15F6D3),
                kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
            regionLabel = 0x15F6D3;
        }
        else if (kasfShipsSelectNearby(
                     kasGrowSelectionPtr(0xEFB9D72),
                     kasGrowSelectionPtr(0x3B41EF5),
                     kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
                     20000))
        {
            kasfFormationWall();
            kasfShipsSortByDistanceToPoint(
                kasGrowSelectionPtr(0xEFB9D72),
                kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
            regionLabel = 0xEFB9D72;
        }
        else
        {
            kasfFormationWall();
            kasfShipsSortByDistanceToPoint(
                kasGrowSelectionPtr(0x3B41EF5),
                kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
            regionLabel = 0x3B41EF5;
        }
    }
    else if (kasfEntryValueGet(-2085181655))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(0x36980EB));
        kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x7C0B279));
        kasfShipsSortByDistanceToPoint(
            kasGrowSelectionPtr(0x36980EB),
            kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
        kasfFormationWall();
        if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
        {
            kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x857BCE3));
        }
        regionLabel = 0x36980EB;
    }
    else
    {
        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr(0x36980EB),
                kasGrowSelectionPtr(0x7C0B279),
                kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
                20000)
            || kasfUnderAttack(kasGrowSelectionPtr(0x36980EB)))
        {
            kasfFormationWall();
            kasfShipsSortByDistanceToPoint(
                kasGrowSelectionPtr(0x36980EB),
                kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
            regionLabel = 0x36980EB;
        }
        else
        {
            kasfSpeechEvent(
                374244,
                (sdword)Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup,
                (sdword)Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup);
            return;
        }
    }

    tutDeallocateRootRegion(kasGrowSelectionPtr(regionLabel));
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}

/*=== Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup (0x608AC0) ===*/
void Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if ((kasfThisTeamIs(kasAITeamPtrIfExists(73198753))
            && kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0xD5B86B4), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(0x59615C1)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(73198754))
            && kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0xD5B86B4), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(0x59615C2)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(73198755))
            && kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0xD5B86B4), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(0x59615C3)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(73198756))
            && kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0xD5B86B4), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(0x59615C4))))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission09_IonFrigateEscort_AttackRun,
            (sdword)Watch_Mission09_IonFrigateEscort_Reengage);
    }
}

/*=== Watch_Mission09_IonFrigateEscort_Reengage (0x608F70) ===*/
void Watch_Mission09_IonFrigateEscort_Reengage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission09_IonFrigateEscort_AttackRun,
            (sdword)Watch_Mission09_IonFrigateEscort_Reengage);
    }
}

/*=== Watch_Mission09_IonFrigateEscort_SpawnReinforcement (0x608FE0) ===*/
void Watch_Mission09_IonFrigateEscort_SpawnReinforcement(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(44672946)))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup);
        return;
    }

    if (kasTimerExpiredById(4693508))
    {
        kasfShipsCreateInside(
            kasGrowSelectionPtr(0x945CEC2),
            "tIonCannonFrigate",
            (char *)kasfRandom(3, 5),
            kasAITeamShipsPtrIfExists(44672946),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(0x945CEC2), 2);
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(0x945CEC2));
        kasTimerSetById(0xCB88434, 60);
        kasTimerStartById(0xCB88434);
    }

    if (kasfTeamCount() && (kasfTeamFinishedLaunching() || kasTimerExpiredDestroyById(0xCB88434)))
    {
        kasTimerDestroyById(0xB847D64);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_IonFrigateEscort_FormOnActiveStrikeGroup);
    }
}

void Init_Mission09_MultigunCorvetteEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(
        0x4C9664,
        (sdword)Init_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup,
        (sdword)Watch_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup);
}

void Watch_Mission09_MultigunCorvetteEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(0x2A9A7B2)))
        {
            if (!kasfTeamCount())
            {
                kasfSpeechEvent(
                    0x479E04,
                    (sdword)Init_Mission09_MultigunCorvetteEscort_ReinforcementDelay,
                    (sdword)Watch_Mission09_MultigunCorvetteEscort_SpawnReinforcement);
            }
        }
        else if (!kasfTeamCount())
        {
            kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)MaintainTeamContext);
        }
    }
}

void Init_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE1)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(0x45CECA1));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE2)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(0x45CECA2));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE3)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(0x45CECA3));
    }
    else
    {
        kasfProtect(kasAITeamShipsPtrIfExists(0x45CECA4));
    }
}

void Watch_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if ((kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE1)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA1)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE2))
            && !kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA2)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE3))
            && !kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA3)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE4))
            && !kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA4))))
    {
        kasfSpeechEvent(
            0x469857B,
            (sdword)Begin_Mission09_MultigunCorvetteEscort_AttackRun,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_Reengage);
    }
}

void Begin_Mission09_MultigunCorvetteEscort_AttackRun(void)
{
    sdword regionLabel;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE1)))
    {
        regionLabel = 0x59615C1;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE2)))
    {
        regionLabel = 0x59615C2;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE3)))
    {
        regionLabel = 0x59615C3;
    }
    else
    {
        regionLabel = 0x59615C4;
    }

    kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0x3B41EF5), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(regionLabel));
    if (kasfShipsCount(kasGrowSelectionPtr(0x3B41EF5)) && !kasfEntryValueGet(-2085181655))
    {
        if (kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(0x7A38423), kasGrowSelectionPtr(0x15F6D3))
            && kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr(0x15F6D3),
                kasGrowSelectionPtr(0x15F6D3),
                kasGrowSelectionPtr(0x3B41EF5)))
        {
            kasfFormationWall();
            kasfShipsSortByDistanceToPoint(
                kasGrowSelectionPtr(0x15F6D3),
                kasShipsVectorPtr((char *)0x15F6D3));
            regionLabel = 0x15F6D3;
        }
        else
        {
            kasfFormationWall();
            if (kasfShipsSelectNearby(
                    kasGrowSelectionPtr(0xEFB9D72),
                    kasGrowSelectionPtr(0x3B41EF5),
                    kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
                    20000))
            {
                kasfShipsSortByDistanceToPoint(
                    kasGrowSelectionPtr(0xEFB9D72),
                    kasShipsVectorPtr((char *)0xEFB9D72));
                regionLabel = 0xEFB9D72;
            }
            else
            {
                kasfShipsSortByDistanceToPoint(
                    kasGrowSelectionPtr(0x3B41EF5),
                    kasShipsVectorPtr((char *)0x3B41EF5));
                regionLabel = 0x3B41EF5;
            }
        }
    }
    else if (kasfShipsSelectNearby(
                 kasGrowSelectionPtr(0x36980EB),
                 kasGrowSelectionPtr(0x7C0B279),
                 kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
                 20000)
             || kasfFindShipsTargetingSelection(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr(0x36980EB)))
    {
        kasfFormationWall();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasShipsVectorPtr((char *)0x36980EB));
        regionLabel = 0x36980EB;
    }
    else if (kasfEntryValueGet(-2085181655))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(0x36980EB));
        kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x7C0B279));
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasShipsVectorPtr((char *)0x36980EB));
        kasfFormationWall();
        regionLabel = 0x36980EB;
    }
    else
    {
        kasfSpeechEvent(
            0x5B5E4,
            (sdword)Init_Mission09_MultigunCorvetteEscort_SelectAttackVector,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_SelectAttackVector);
        return;
    }

    tutDeallocateRootRegion(kasGrowSelectionPtr(regionLabel));
    kasTimerDestroyById(0x469857B);
    kasTimerCreateSetStartById(0x469857B, kasfRandom(30, 60));
}

void Watch_Mission09_MultigunCorvetteEscort_Reengage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            0x5B5E4,
            (sdword)Init_Mission09_MultigunCorvetteEscort_SelectAttackVector,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_SelectAttackVector);
    }
    else if (kasTimerExpiredById(0x469857B))
    {
        kasfSpeechEvent(
            0x469857B,
            (sdword)Begin_Mission09_MultigunCorvetteEscort_AttackRun,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_Reengage);
    }
}

void Init_Mission09_MultigunCorvetteEscort_SelectAttackVector(void)
{
hvector *attackVector;

kasCurrentStep = 2;
kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
kasfFormationWall();
if (kasfThisTeamIs(kasAITeamPtrIfExists(111094753)))
{
attackVector = kasVolumeVectorPtr(93722049);
}
else if (kasfThisTeamIs(kasAITeamPtrIfExists(111094754)))
{
attackVector = kasVolumeVectorPtr(93722050);
}
else if (kasfThisTeamIs(kasAITeamPtrIfExists(111094755)))
{
attackVector = kasVolumeVectorPtr(93722051);
}
else
{
attackVector = kasVolumeVectorPtr(93722052);
}
kasfMoveTo(attackVector);
}

void Watch_Mission09_MultigunCorvetteEscort_SelectAttackVector(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if ((kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE1)) && kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA1)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE2))
            && kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA2)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE3))
            && kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA3)))
        || (kasfThisTeamIs(kasAITeamPtrIfExists(0x69F2BE4))
            && kasfShipsCount(kasAITeamShipsPtrIfExists(0x45CECA4))))
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup);
    }
    else if (kasfShipsSelectInsideVolume(kasGrowSelectionPtr(0xD5B86B4), kasGrowSelectionPtr(0x7C0B279), kasVolumePtr(0x59615C1)))
    {
        kasfSpeechEvent(
            0x469857B,
            (sdword)Begin_Mission09_MultigunCorvetteEscort_AttackRun,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_Reengage);
    }
}

void Init_Mission09_MultigunCorvetteEscort_ReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, kasfRandom(60, 300));
    kasTimerDestroyById(213419060);
    kasTimerCreateById(213419060);
}

void Watch_Mission09_MultigunCorvetteEscort_SpawnReinforcement(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x2A9A7B2)))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup);
        return;
    }

    if (kasTimerExpiredById(0x479E04))
    {
        kasfShipsCreateInside(
            kasGrowSelectionPtr(0x945CEC2),
            "tMultiGunCorvette",
            (char *)kasfRandom(5, 8),
            (sdword)kasAITeamShipsPtrIfExists(0x2A9A7B2),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(0x945CEC2), 2);
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(0x945CEC2));
        kasTimerSetById(0xCB88434, 60);
        kasTimerStartById(0xCB88434);
    }

    if (kasfTeamCount() && (kasfTeamFinishedLaunching() || kasTimerExpiredDestroyById(0xCB88434)))
    {
        kasTimerDestroyById(0xB847D64);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_MultigunCorvetteEscort_FormOnActiveStrikeGroup);
    }
}

void Init_Mission09_SupportFrigateEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(
        0x4C9664,
        (sdword)Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup,
        (sdword)Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup);
}

void Watch_Mission09_SupportFrigateEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(0x2A9A7B2)))
        {
            if (!kasfTeamCount())
            {
                kasfSpeechEvent(
                    0x479E04,
                    (sdword)Init_Mission09_SupportFrigateEscort_ReinforcementDelay,
                    (sdword)Watch_Mission09_SupportFrigateEscort_SpawnReinforcement);
            }
        }
        else if (!kasfTeamCount())
        {
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
    }
}

void Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup(void)
{
    sdword attackLabel;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x93C9634));
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x6C32E91)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasAITeamShipsPtrIfExists(0x45CECA1));
        attackLabel = 0x69F2BE1;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x6C32E92)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasAITeamShipsPtrIfExists(0x45CECA2));
        attackLabel = 0x69F2BE2;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x6C32E93)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasAITeamShipsPtrIfExists(0x45CECA3));
        attackLabel = 0x69F2BE3;
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasAITeamShipsPtrIfExists(0x45CECA4));
        attackLabel = 0x69F2BE4;
    }
    kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasAITeamShipsPtrIfExists(attackLabel));
    kasfProtect(kasGrowSelectionPtr(0x93C9634));
    kasTimerCreateSetStartById(0x4C9664, 30);
}

void Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x3F9DA23),
            kasGrowSelectionPtr(0x6DA2DA9),
            kasShipsVectorPtr((char *)0x4E20),
            20000)
        && kasfShipsSelectCapital(kasGrowSelectionPtr(0x3F9DA23), kasGrowSelectionPtr(0x3F9DA23))
        && kasfShipsSelectDamaged(kasGrowSelectionPtr(0x3F9DA23), kasGrowSelectionPtr(0x3F9DA23), 70))
    {
        kasfSpeechEvent(0x4C95C, (sdword)Begin_Mission09_SupportFrigateEscort_RepairOrder, (sdword)ThisAIPlayerShipCreatedCB);
        return;
    }

    if (kasTimerExpiredById(0x4C9664) && kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x2A9A7B2)))
        {
            kasfAttackMothership();
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
        else
        {
            kasfSpeechEvent(
                0x4C9664,
                (sdword)Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup,
                (sdword)Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup);
        }
    }
}

void Init_Mission09_SupportFrigateEscort_ReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, kasfRandom(300, 400));
    kasTimerDestroyById(213419060);
    kasTimerCreateById(213419060);
}

sdword Begin_Mission09_SupportFrigateEscort_RepairOrder(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x3F9DA23),
            kasGrowSelectionPtr(0x6DA2DA9),
            kasShipsVectorPtr((char *)0x4E20),
            20000)
        && kasfShipsSelectCapital(kasGrowSelectionPtr(0x3F9DA23), kasGrowSelectionPtr(0x3F9DA23))
        && kasfShipsSelectDamaged(kasGrowSelectionPtr(0x3F9DA23), kasGrowSelectionPtr(0x3F9DA23), 70))
    {
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x3F9DA23), kasShipsVectorPtr((char *)0x3F9DA23));
        return kasfTeamSpecial(kasGrowSelectionPtr(0x3F9DA23));
    }

    kasfSpeechEvent(
        0x4C9664,
        (sdword)Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup,
        (sdword)Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup);
    return 0;
}

void Watch_Mission09_SupportFrigateEscort_SpawnReinforcement(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x2A9A7B2)))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup);
        return;
    }

    if (kasTimerExpiredById(0x479E04))
    {
        kasfShipsCreateInside(
            kasGrowSelectionPtr(0x945CEC2),
            "tAdvanceSupportFrigate",
            (char *)1,
            (sdword)kasAITeamShipsPtrIfExists(0x2A9A7B2),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(0x945CEC2), 2);
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(0x945CEC2));
        kasTimerSetById(0xCB88434, 60);
        kasTimerStartById(0xCB88434);
    }

    if (kasfTeamCount() && (kasfTeamFinishedLaunching() || kasTimerExpiredDestroyById(0xCB88434)))
    {
        kasTimerDestroyById(0xB847D64);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup,
            (sdword)Watch_Mission09_SupportFrigateEscort_FormOnActiveStrikeGroup);
    }
}

/*=== Watch_Mission09_AssaultEscort_AttackRun (0X607C90) ===*/
void Watch_Mission09_AssaultEscort_AttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission09_AssaultEscort_AttackRun,
            (sdword)Watch_Mission09_AssaultEscort_AttackRun);
    }
}

/*=== Init_Mission09_MinelayerEscort (0x608210) ===*/
void Init_Mission09_MinelayerEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfActivateTech("MineLayerTech");
    kasfShipsApplyPlayerUpgrades(&((AITeam *)kasCurrentTeamPtr)->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    if (kasfThisTeamIs(kasAITeamPtrIfExists(44055618)))
    {
        kasfSpeechEvent(158897298, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_MinelayerEscort);
    }
    else
    {
        kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_MinelayerEscort);
    }
}

/*=== Watch_Mission09_MinelayerEscort (0x6082B0) ===*/
void Watch_Mission09_MinelayerEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr(0x36980EB),
                kasGrowSelectionPtr(0x4A2A29),
                kasShipsVectorPtr((char *)0x4E20),
                0))
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Init_Mission09_MinelayerEscort_Reengage,
                (sdword)Watch_Mission09_MinelayerEscort_Reengage);
        }
        else if (!kasfTeamCount())
        {
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
    }
}

/*=== Watch_Mission09_MinelayerEscort_DeployMines (0x6084F0) ===*/
void Watch_Mission09_MinelayerEscort_DeployMines(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfTeamSpecialToggle();
        kasTimerCreateSetStartById(31738387, 3);
    }
    if (kasTimerExpiredById(31738387))
    {
        kasfStop();
        kasfSpeechEvent(
            5366435,
            (sdword)Init_Mission09_MinelayerEscort_SetRandomMineDropPoint,
            (sdword)Watch_Mission09_MinelayerEscort_DeployMines);
    }
}

void Init_Mission09_MinelayerEscort_SetRandomApproachVector(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfFormationWall();
    kasfVectorRandomizeInRange(kasVectorPtrIfExists(44055618), kasShipsVectorPtrIfExists(128156707), 30000);
    kasfMoveTo(kasVectorPtrIfExists(44055618));
}

/*=== Init_Mission09_MinelayerEscort_RepositionDelay (0x608630) ===*/
void Init_Mission09_MinelayerEscort_RepositionDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(374244);
    kasTimerCreateSetStartById(374244, 20);
}

/*=== Watch_Mission09_MinelayerEscort_RepositionDelay (0x608670) ===*/
void Watch_Mission09_MinelayerEscort_RepositionDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        if (kasfFindMissilesInside(kasVolumePtr(0x7EC6887), kasGrowSelectionPtr(0x51E2A3)) <= 1000)
        {
            kasfSpeechEvent(
                5366435,
                (sdword)Init_Mission09_MinelayerEscort_SetRandomMineDropPoint,
                (sdword)Watch_Mission09_MinelayerEscort_DeployMines);
        }
        else
        {
            kasfSpeechEvent(
                5769632,
                (sdword)Init_Mission09_MinelayerEscort_SetRandomApproachVector,
                (sdword)Watch_Mission09_MinelayerEscort_SetRandomMineDropPoint);
        }
    }
}

/*=== Watch_Mission09_MinelayerEscort_Reengage (0x608840) ===*/
void Watch_Mission09_MinelayerEscort_Reengage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission09_MinelayerEscort_RepositionDelay,
            (sdword)Watch_Mission09_MinelayerEscort_RepositionDelay);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission09_MinelayerEscort_Reengage,
            (sdword)Watch_Mission09_MinelayerEscort_Reengage);
    }
}

void Watch_Mission09_MinelayerEscort_LaunchTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfMsgSend(kasAITeamPtrIfExists(168018961), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(168018962), (char *)5801588);
    }
}

/*=== Init_Mission09_MinelayerEscort_SetRandomMineDropPoint (0x608420) ===*/
void Init_Mission09_MinelayerEscort_SetRandomMineDropPoint(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(31738387);
    if (kasfFindMissilesInside(kasVolumePtr(0x7EC6887), kasGrowSelectionPtr(0x51E2A3)) > 1000)
    {
        kasfSpeechEvent(
            5769632,
            (sdword)Init_Mission09_MinelayerEscort_SetRandomApproachVector,
            (sdword)Watch_Mission09_MinelayerEscort_SetRandomMineDropPoint);
        return;
    }

    kasfLabelledVectorSet(
        0x2A03C42,
        kasfRandom(-23000, 23000),
        kasfRandom(-73000, 73000),
        kasfRandom(-100000, -55000));
    kasfFormationWall();
    kasfMoveTo(kasVectorPtrIfExists(0x2A03C42));
}

/*=== Watch_Mission09_MinelayerEscort_SetRandomMineDropPoint (0x6085C0) ===*/
void Watch_Mission09_MinelayerEscort_SetRandomMineDropPoint(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList)
        && kasfFindMissilesInside(kasVolumePtr(0x7EC6887), kasGrowSelectionPtr(0x51E2A3)) <= 1000)
    {
        kasfSpeechEvent(
            5366435,
            (sdword)Init_Mission09_MinelayerEscort_SetRandomMineDropPoint,
            (sdword)Watch_Mission09_MinelayerEscort_DeployMines);
    }
}

/*=== Init_Mission09_ResourceTeam (0x607110) ===*/
void Init_Mission09_ResourceTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(55103095, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_ResourceTeam);
}

/*=== Watch_Mission09_ResourceTeam (0x607150) ===*/
void Watch_Mission09_ResourceTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552161) && kasfEntryValueGet(-2015429404))
    {
        if (!kasfShipsCount(kasGrowSelectionPtr(77795571)) && !kasfEntryValueGet(-2121668332))
        {
            kasfEntryGetOrCreateSetValue(71552161, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)32770948);
        }
    }
}

/*=== Watch_Mission09_ResourceTeam_ReinforceFromNearbyShips (0x6071E0) ===*/
void Watch_Mission09_ResourceTeam_ReinforceFromNearbyShips(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if ((kasfShipsSelectNearby(
             kasGrowSelectionPtr(0xCDB1974),
             kasGrowSelectionPtr(77795571),
             kasVectorPtrIfExists(48187763),
             12000) >= 3
         && kasfShipsCount(kasGrowSelectionPtr(77795571)))
        || kasfMsgReceived((char *)5801588))
    {
        kasfEntryGetOrCreateSetValue(-1909110715, 1);
        kasfEntrySetValue(332667, 0);
        kasfTeamSwitchPlayerOwner(0);
        kasfStop();
        kasfShipsSelectFirstN(kasGrowSelectionPtr(77795571), kasGrowSelectionPtr(77795571), 3);
        kasfShipsCount(kasGrowSelectionPtr(77795571));
        seekdir();
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(77795571));
        kasfShipsSetMaxVelocity(&((AITeam *)kasCurrentTeamPtr)->shipList, 500);
        kasfTacticsNeutral();
        kasfShipsSetSupportFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
        kasfShipsSetAttachVelocityFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)140412507);
        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
    }
}

/*=== Watch_Mission09_ResourceTeam_InterceptMoveOrderTrigger (0x607350) ===*/
void Watch_Mission09_ResourceTeam_InterceptMoveOrderTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x4506734))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Init_Mission09_ResourceTeam_MoveToInterceptVector,
            (sdword)Watch_Mission09_ResourceTeam_MoveToInterceptVector);
    }
}

void Init_Mission09_ResourceTeam_MoveToInterceptVector(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationDelta();
    kasfMoveTo(kasVectorPtrIfExists(44955244));
}

/*=== Watch_Mission09_ResourceTeam_MoveToInterceptVector (0x6073E0) ===*/
void Watch_Mission09_ResourceTeam_MoveToInterceptVector(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfNearby(kasVectorPtrIfExists(44955244), 8000))
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)48761347);
        kasfSpeechEvent(143009833, (sdword)MaintainTeamContext, 0);
    }
}

/*=== Watch_Mission09_ResourceTeam_InterceptTargetTrigger (0x607450) ===*/
void Watch_Mission09_ResourceTeam_InterceptTargetTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x254F5E4))
    {
        kasfSpeechEvent(
            299899,
            (sdword)Init_Mission09_ResourceTeam_InterceptTargetGroup,
            (sdword)Watch_Mission09_ResourceTeam_InterceptTargetGroup);
    }
}

void Init_Mission09_ResourceTeam_InterceptTargetGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamSwitchPlayerOwner(0);
    kasfUnhideShips(&team->shipList);
    kasfFormationDelta();
    kasfIntercept(kasGrowSelectionPtr(218060896));
}

/*=== Watch_Mission09_ResourceTeam_InterceptTargetGroup (0x6074F0) ===*/
void Watch_Mission09_ResourceTeam_InterceptTargetGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552162) && kasfNearby(kasShipsVectorPtrIfExists(218060896), 8000))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfStop();
        kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(218060896));
    }
    if (!kasfEntryValueGet(71552163))
    {
        if (kasfFindDistance(
                kasShipsVectorPtrIfExists(218060896),
                kasShipsVectorPtr((char *)team->kasFSMNameHash))
                <= kasfEntryValueGet(-2083929931)
            || kasfShipsSelectInsideVolume(
                kasGrowSelectionPtr(0x4E37D85),
                &team->shipList,
                kasVolumePtr(0x46A9F5E)))
        {
            kasfEntryGetOrCreateSetValue(71552163, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)39122404);
        }
    }
    if (kasfEntryValueGet(-2121668332))
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)357797);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

/*=== Watch_Mission09_AssaultEscort_InterceptNearbyContacts (0x608180) ===*/
void Watch_Mission09_AssaultEscort_InterceptNearbyContacts(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount() || kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfEntrySetValue(-1978936548, 0);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            88512316,
            (sdword)Init_Mission09_AssaultEscort_SetRandomWaypoint,
            (sdword)Watch_Mission09_AssaultEscort_WaypointLoop);
    }
    else if (kasTimerExpiredDestroyById(4590196))
    {
        kasfSpeechEvent(
            4590196,
            (sdword)Init_Mission09_AssaultEscort_InterceptNearbyContacts,
            (sdword)Watch_Mission09_AssaultEscort_InterceptNearbyContacts);
    }
}

/*=== Watch_Mission09_MinelayerEscort_WaitForLaunchSignal (0x6083B0) ===*/
void Watch_Mission09_MinelayerEscort_WaitForLaunchSignal(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfTeamHyperspaceInNear(kasTeamsVectorPtr(246649714), 20000);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            5366435,
            (sdword)Init_Mission09_MinelayerEscort_SetRandomMineDropPoint,
            (sdword)Watch_Mission09_MinelayerEscort_DeployMines);
    }
}


/*=== Init_Mission09_MinelayerEscort_Reengage (0x608700) ===*/
void Init_Mission09_MinelayerEscort_Reengage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (kasfFindMissilesNearTeam(kasGrowSelectionPtr(0x51E2A3), 1000) < 20)
    {
        kasfTeamSpecialToggle();
    }

    kasfShipsSelectNearby(
        kasGrowSelectionPtr(0x36980EB),
        kasGrowSelectionPtr(0x4A2A29),
        kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
        20000);
    if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x857BCE3));
    }

    if (!kasfShipsCount(kasGrowSelectionPtr(0x36980EB)))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission09_MinelayerEscort_RepositionDelay,
            (sdword)Watch_Mission09_MinelayerEscort_RepositionDelay);
        return;
    }

    kasfShipsSortByDistanceToPoint(
        kasGrowSelectionPtr(0x36980EB),
        kasShipsVectorPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash));
    kasfFormationWall();
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(5, 10));
}


static void Init_Mission09_BigGunAssaultFleet_SetWaypoint(void) {}
static void Watch_Mission09_BigGunAssaultFleet_SetWaypoint(void) {}
static void Init_Mission09_BigGunEscortWing_SetWaypoint(void) {}

void Init_Mission09_BigGunAssaultFleet(void)
{
    sdword count;

    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfEntryGetOrCreateSetValue(23649204, 0);
    kasfEntryGetOrCreateSetValue(87694910, 0);
    switch (GetGameDifficulty())
    {
        case 0:
            kasfEntrySetValue(23649204, 1);
            kasfEntrySetValue(87694910, 0);
            break;
        case 1:
            kasfEntrySetValue(23649204, 2);
            kasfEntrySetValue(87694910, 0);
            break;
        case 2:
        case 3:
            kasfEntrySetValue(23649204, 2);
            kasfEntrySetValue(87694910, 1);
            break;
        case 4:
            kasfEntrySetValue(23649204, 3);
            kasfEntrySetValue(87694910, 2);
            break;
        default:
            break;
    }
    if (kasfEntryValueGet(23649204) > 0)
    {
        count = kasfEntryValueGet(23649204);
        kasfShipsCreate(kasGrowSelectionPtr(0x9879BF3), "tStandardFrigate", count, 1);
        kasfReinforceTeamWithShips(teamtoreinforce, kasGrowSelectionPtr(0x9879BF3));
    }
    if (kasfEntryValueGet(87694910) > 0)
    {
        count = kasfEntryValueGet(87694910);
        kasfShipsCreate(kasGrowSelectionPtr(0x9879BF3), "tIonCannonFrigate", count, 1);
        kasfReinforceTeamWithShips(teamtoreinforce, kasGrowSelectionPtr(0x9879BF3));
    }
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 2);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfEntryGetOrCreateSetValue(260114404, 1);
    kasfEntryGetOrCreateSetValue(218000372, 0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(45227969)))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A61));
        kasfEntrySetValue(260114404, 1);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(45227970)))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A63));
        kasfEntrySetValue(260114404, 3);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(45227971)))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A65));
        kasfEntrySetValue(260114404, 5);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(45227972)))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A67));
        kasfEntrySetValue(260114404, 7);
    }
    kasfSpeechEvent(0x546973C, (sdword)Init_Mission09_BigGunAssaultFleet_SetWaypoint, (sdword)Watch_Mission09_BigGunAssaultFleet_SetWaypoint);
}

void Init_Mission09_BigGunEscortWing(void)
{
    GrowSelection *ships;
    sdword count;
    char *shipType;
    sdword shipCount;

    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfEntryGetOrCreateSetValue(66381540, 0);

    if (kasfThisTeamIs(kasAITeamPtrIfExists(147220289)))
        ships = kasAITeamShipsPtrIfExists(45227969);
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220290)))
        ships = kasAITeamShipsPtrIfExists(45227970);
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220291)))
        ships = kasAITeamShipsPtrIfExists(45227971);
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220292)))
        ships = kasAITeamShipsPtrIfExists(45227972);
    else
        goto done;

    count = kasfShipsCount(ships);
    kasfEntrySetValue(66381540, count);

done:
    if (GetGameDifficulty() >= 2 && kasfEntryValueGet(66381540) > 0)
    {
        if (kasfRandom(1, 2) == 1)
        {
            shipCount = 10;
            shipType = "tHeavyInterceptor";
        }
        else
        {
            shipCount = 6;
            shipType = "tMultigunCorvette";
        }
        kasfShipsCreate(kasGrowSelectionPtr(0x9879BF3), shipType, shipCount, 1);
        kasfReinforceTeamWithShips(teamtoreinforce, kasGrowSelectionPtr(0x9879BF3));
        kasAITeamShipsPtr(&teamtoreinforce->shipList, 2);
        kasfTeamAttributesBitSet(128);
        kasfAttackMothership();
        if (kasfThisTeamIs(kasAITeamPtrIfExists(147220289)))
            kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A61));
        else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220290)))
            kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A63));
        else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220291)))
            kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A65));
        else if (kasfThisTeamIs(kasAITeamPtrIfExists(147220292)))
            kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x5F8A67));
        kasfSpeechEvent(0x4A78474, (sdword)Init_Mission09_BigGunEscortWing_SetWaypoint, (sdword)Watch_Mission09_BigGunEscortWing_SetWaypoint);
        return;
    }
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}

void Init_Mission09(void)
{
    kasCurrentStep = 0;
    scope = KH_mission09;
    switch (GetGameDifficulty())
    {
        case 0:  kasfEntryGetOrCreateSetValue(-2083929931, 30000); break;
        case 1:  kasfEntryGetOrCreateSetValue(-2083929931, 40000); break;
        case 2:
        case 3:
        case 4:  kasfEntryGetOrCreateSetValue(-2083929931, 50000); break;
        default: break;
    }

    kasFSMCreate(KH_FleetIntel, Init_Mission09_FleetIntel, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission09_ObjectiveEventFlow,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(77795571, Init_Mission09_ResourceTeam, Watch_Mission09_ResourceTeam,
                 kasAITeamPtrIfExists(77795571));
    kasFSMCreate(128156707, Init_Mission09_SensorObjectiveTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(128156707));
    kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953345));
    kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953346));
    kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953347));
    kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953348));
    kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953349));

    if (GetGameDifficulty())
    {
        kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953350));
        kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953351));
        kasFSMCreate(228413404, Init_Mission09_AssaultEscort, Watch_Mission09_AssaultEscort, kasAITeamPtrIfExists(164953352));
    }
    else
    {
        kasfDeleteShips(kasAITeamShipsPtrIfExists(164953350));
        kasfDeleteShips(kasAITeamShipsPtrIfExists(164953351));
        kasfDeleteShips(kasAITeamShipsPtrIfExists(164953352));
    }

    kasFSMCreate(44055618, Init_Mission09_MinelayerEscort, Watch_Mission09_MinelayerEscort, kasAITeamPtrIfExists(44055618));
    kasFSMCreate(44055618, Init_Mission09_MinelayerEscort, Watch_Mission09_MinelayerEscort, kasAITeamPtrIfExists(168018961));
    kasFSMCreate(44055618, Init_Mission09_MinelayerEscort, Watch_Mission09_MinelayerEscort, kasAITeamPtrIfExists(168018962));
    kasFSMCreate(44055618, Init_Mission09_MinelayerEscort, Watch_Mission09_MinelayerEscort, kasAITeamPtrIfExists(168018963));
    kasFSMCreate(44055618, Init_Mission09_MinelayerEscort, Watch_Mission09_MinelayerEscort, kasAITeamPtrIfExists(168018964));
    kasFSMCreate(44055618, Init_Mission09_MinelayerEscort, Watch_Mission09_MinelayerEscort, kasAITeamPtrIfExists(168018965));

    kasFSMCreate(155569858, Init_Mission09_IonFrigateEscort, Watch_Mission09_IonFrigateEscort, kasAITeamPtrIfExists(73198753));
    kasFSMCreate(155569858, Init_Mission09_IonFrigateEscort, Watch_Mission09_IonFrigateEscort, kasAITeamPtrIfExists(73198754));
    kasFSMCreate(155569858, Init_Mission09_IonFrigateEscort, Watch_Mission09_IonFrigateEscort, kasAITeamPtrIfExists(73198755));
    kasFSMCreate(155569858, Init_Mission09_IonFrigateEscort, Watch_Mission09_IonFrigateEscort, kasAITeamPtrIfExists(73198756));

    kasFSMCreate(107606711, Init_Mission09_MultigunCorvetteEscort, Watch_Mission09_MultigunCorvetteEscort, kasAITeamPtrIfExists(111094753));
    kasFSMCreate(107606711, Init_Mission09_MultigunCorvetteEscort, Watch_Mission09_MultigunCorvetteEscort, kasAITeamPtrIfExists(111094754));
    kasFSMCreate(107606711, Init_Mission09_MultigunCorvetteEscort, Watch_Mission09_MultigunCorvetteEscort, kasAITeamPtrIfExists(111094755));
    kasFSMCreate(107606711, Init_Mission09_MultigunCorvetteEscort, Watch_Mission09_MultigunCorvetteEscort, kasAITeamPtrIfExists(111094756));

    kasFSMCreate(208417508, Init_Mission09_SupportFrigateEscort, Watch_Mission09_SupportFrigateEscort, kasAITeamPtrIfExists(113454737));
    kasFSMCreate(208417508, Init_Mission09_SupportFrigateEscort, Watch_Mission09_SupportFrigateEscort, kasAITeamPtrIfExists(113454738));
    kasFSMCreate(208417508, Init_Mission09_SupportFrigateEscort, Watch_Mission09_SupportFrigateEscort, kasAITeamPtrIfExists(113454739));
    kasFSMCreate(208417508, Init_Mission09_SupportFrigateEscort, Watch_Mission09_SupportFrigateEscort, kasAITeamPtrIfExists(113454740));

    kasFSMCreate(346756, Init_Mission09_AttackBomberWing, Watch_Mission09_AttackBomberWing, kasAITeamPtrIfExists(346756));
    kasFSMCreate(91769491, Init_Mission09_HeavyInterceptorWing, Watch_Mission09_HeavyInterceptorWing, kasAITeamPtrIfExists(91769491));
    kasFSMCreate(207674834, Init_Mission09_CapitalAssaultWave, Watch_Mission09_CapitalAssaultWave, kasAITeamPtrIfExists(207674834));
    kasFSMCreate(248587548, Init_Mission09_GravwellSquad, Watch_Mission09_GravwellSquad, kasAITeamPtrIfExists(248587548));
    kasFSMCreate(238890036, Init_Mission09_SupportFrigateSquad, Watch_Mission09_SupportFrigateSquad, kasAITeamPtrIfExists(238890036));

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
    kasfModuleSlotOrderSetShipType(1, "tAttackBomber");
    kasfModuleSlotOrderSetShipType(1, "tHeavyDefender");
    kasfModuleSlotOrderSetShipType(1, "tMultiGunCorvette");
    kasfModuleSlotOrderSetShipType(1, "tIonCannonFrigate");
    kasfModuleSlotOrderSetCount(1, 3);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasFSMCreate(19603964, Init_Mission09_BigGunAssaultFleet, Watch_NoOpTeamContext, kasAITeamPtrIfExists(45227969));
    kasFSMCreate(25978484, Init_Mission09_BigGunEscortWing, Watch_NoOpTeamContext, kasAITeamPtrIfExists(147220289));
    if (GetGameDifficulty() > 0)
    {
        kasFSMCreate(19603964, Init_Mission09_BigGunAssaultFleet, Watch_NoOpTeamContext, kasAITeamPtrIfExists(45227970));
        kasFSMCreate(25978484, Init_Mission09_BigGunEscortWing, Watch_NoOpTeamContext, kasAITeamPtrIfExists(147220290));
    }
    if (GetGameDifficulty() > 2)
    {
        kasFSMCreate(19603964, Init_Mission09_BigGunAssaultFleet, Watch_NoOpTeamContext, kasAITeamPtrIfExists(45227971));
        kasFSMCreate(25978484, Init_Mission09_BigGunEscortWing, Watch_NoOpTeamContext, kasAITeamPtrIfExists(147220291));
    }
    if (GetGameDifficulty() > 4)
    {
        kasFSMCreate(19603964, Init_Mission09_BigGunAssaultFleet, Watch_NoOpTeamContext, kasAITeamPtrIfExists(45227972));
        kasFSMCreate(25978484, Init_Mission09_BigGunEscortWing, Watch_NoOpTeamContext, kasAITeamPtrIfExists(147220292));
    }

    kasfShipsAdd((int *)kasGrowSelectionPtr(128156707), (int *)kasAITeamShipsPtrIfExists(128156707));

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
    kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");
    kasfAllowPlayerToResearch("TECH_ARMOUR2");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");
    kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
    kasfAllowPlayerToResearch("TECH_ADVANCED_SYSTEMS");
    kasfAllowPlayerToResearch("TECH_CRYSTAL_HARVEST");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipEngineering", "RACE_SECT", 1);
    kasfEnableShip("sMothershipWeapons", "RACE_SECT", 1);
    kasfEnableShip("sMothershipArmour", "RACE_SECT", 1);
    kasfEnableShip("sMothershipSpecial", "RACE_SECT", 1);

    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));
    kasfDisablePlayerHyperspace();
    kasfLocationCard(8000, "Close Orbit, Gozan IV (Taiidan Research Planet)");
    kasfMusicPlay(8);
    kasTimerCreateSetStartById(104338707, 1);
    kasfFindBeastShipsInside(kasVolumePtr(14988037), kasGrowSelectionPtr(19508));
    kasTimerCreateSetStartById(91769491, kasfRandom(60, 120));
}
