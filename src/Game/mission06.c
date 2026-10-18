/*=============================================================================
    Name    : mission06.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission06
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m06_saveInfoName[] =
{
    "Kadiir Nebula",
    "French version goes here",
    "German version goes here",
};

static void Begin_Mission06_RepairSupportWing_AssistTarget(void);
static void Begin_Mission06_MothershipAttackTeam_CombatLoop(void);

void Init_Mission06_CarrierRouteCoordinator(void);
void Watch_Mission06_CarrierRouteCoordinator(void);
void Watch_Mission06_CarrierRouteMinefield(void);
void Begin_Mission06_CarrierRouteMineAlert(void);
void Watch_Mission06_CarrierRouteMineAlert(void);
void Begin_Mission06_CarrierRouteFinalApproach(void);
void Watch_Mission06_CarrierRouteFinalApproach(void);
void Init_Mission06_StrikeWingAlpha(void);
void Watch_Mission06_StrikeWingAlpha(void);
void Begin_Mission06_StrikeWingAlpha_Reposition(void);
void Begin_Mission06_StrikeWingAlpha_WaitPoint(void);
void Watch_Mission06_StrikeWingAlpha_EngageTrigger(void);
void Begin_Mission06_StrikeWingAlpha_CapitalShipCombatLoop(void);
void Watch_Mission06_StrikeWingAlpha_CapitalShipCombatLoop(void);
void Begin_Mission06_StrikeWingAlpha_ReinforceAndResume(void);
void Init_Mission06_RepairStrikeWingBeta(void);
void Watch_Mission06_RepairStrikeWingBeta(void);
void Watch_Mission06_RepairStrikeWingBeta_RegroupTrigger(void);
void Begin_Mission06_RepairStrikeWingBeta_RegroupMove(void);
void Begin_Mission06_RepairStrikeWingBeta_RegroupOrder(void);
void Watch_Mission06_RepairStrikeWingBeta_FullyRepaired(void);
void Init_Mission06_ReinforcementWingGamma(void);
void Watch_Mission06_ReinforcementWingGamma(void);
void Watch_Mission06_ReinforcementWingGamma_RegroupTrigger(void);
void Watch_Mission06_HeavyInterceptorWing(void);
void Watch_Mission06_HeavyInterceptorWing_Spawn(void);
void Watch_Mission06_HeavyInterceptorWing_LaunchTrigger(void);
void Init_Mission06_HeavyInterceptorWing_HuntTargets(void);
void Begin_Mission06_HeavyInterceptorWing_CombatLoop(void);
void Watch_Mission06_HeavyInterceptorWing_CombatLoop(void);
void Init_Mission06_FleetSupportWing(void);
void Watch_Mission06_FleetSupportWing(void);
void Watch_Mission06_FleetSupportWing_AttackCueDelay(void);
void Begin_Mission06_FleetSupportWing_MoveToSupportTarget(void);
void Watch_Mission06_FleetSupportWing_MoveToSupportTarget(void);
void Init_Mission06_FleetSupportWing_RepositionDelay(void);
void Init_Mission06_MothershipAttackEscort(void);
void Watch_Mission06_MothershipAttackEscort(void);
void Watch_Mission06_MothershipAttackEscort_AttackCueDelay(void);
void Begin_Mission06_MothershipAttackEscort_FormOnAttackTeam(void);
void Watch_Mission06_MothershipAttackEscort_FormOnAttackTeam(void);
void Init_Mission06_MothershipAttackEscort_CombatLoopDelay(void);
void Watch_Mission06_MothershipAttackEscort_CombatLoopDelay(void);
void Init_Mission06_RepairSupportWing(void);
void Watch_Mission06_RepairSupportWing(void);
void Begin_Mission06_RepairSupportWing_MoveToTarget(void);
void Watch_Mission06_RepairSupportWing_MoveToTarget(void);
void Watch_Mission06_RepairSupportWing_AssistTarget(void);
void Init_Mission06_MothershipAttackTeam(void);
void Watch_Mission06_MothershipAttackTeam(void);
void Begin_Mission06_MothershipAttackTeam_ApproachVector(void);
void Begin_Mission06_MothershipAttackTeam_CombatLoopMove(void);
void Watch_Mission06_MothershipAttackTeam_CombatLoopMove(void);
void Init_Mission06_MothershipAttackTeam_CombatLoopDelay(void);
void Watch_Mission06_MothershipAttackTeam_CombatLoopDelay(void);
void Watch_Mission06_MothershipAttackTeam_CombatLoop(void);

void Watch_Mission06(void)
{
  int v0; // eax
  int *v1; // eax
  int *v2; // eax
  int *v3; // eax
  signed int v4; // eax
  int *v5; // eax
  int *v6; // eax
  int v7; // eax
  int *v8; // eax
  int v9; // eax
  int v10; // eax
  int *v11; // eax
  int v12; // eax
  int *v13; // eax
  int *v14; // eax
  int *v15; // eax
  int v16; // eax
  int *v17; // eax
  int v18; // eax
  int v19; // eax
  int *v20; // eax
  int *v21; // eax
  int *v22; // eax
  int v23; // eax
  int v24; // eax
  int v25; // eax
  int *v26; // eax
  int *v27; // eax
  int *v28; // eax
  int v29; // eax
  int *v30; // eax
  int *v31; // eax
  int *v32; // eax
  int v33; // eax
  int *v34; // eax
  int *v36; // eax
  int v37; // [esp-Ch] [ebp-Ch]
  int v38; // [esp-8h] [ebp-8h]
  int v39; // [esp-8h] [ebp-8h]
  int *v40; // [esp-8h] [ebp-8h]
  int v41; // [esp-8h] [ebp-8h]
  int v42; // [esp-4h] [ebp-4h]
  int v43; // [esp-4h] [ebp-4h]
  int v44; // [esp-4h] [ebp-4h]
  int v45; // [esp-4h] [ebp-4h]
  int v46; // [esp-4h] [ebp-4h]
  int v47; // [esp-4h] [ebp-4h]
  int v48; // [esp-4h] [ebp-4h]
  int *v49; // [esp-4h] [ebp-4h]
  int v50; // [esp-4h] [ebp-4h]
  int *v51; // [esp-4h] [ebp-4h]
  int *v52; // [esp-4h] [ebp-4h]
  int v53; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503670;
  if ( kasTimerExpiredDestroyById(104338707) )
  {
    v42 = kasGrowSelectionPtr(18336307);
    v0 = kasVolumePtr(132933767);
    kasfFindShipsInside(v0, v42);
    v43 = kasGrowSelectionPtr(18336307);
    v1 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v1, v43);
    v44 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v2, v44);
    v45 = kasGrowSelectionPtr(18336307);
    v3 = (int *)kasGrowSelectionPtr(45901433);
    kasSelectPlayerShipsSimple(v3, v45);
    v4 = kasfRandom(8, 15);
    kasTimerSetById(104338707, v4);
    kasTimerStartById(104338707);
  }
  if ( !kasfEntryValueGet(71552194) && kasfEntryValueGet(-2015429404) && kasfIntelEventEnded() )
  {
    kasfEntrySetValue(71552194, 1);
    v46 = kasGrowSelectionPtr(18336307);
    v5 = (int *)kasGrowSelectionPtr(89157026);
    kasSelectPlayerShipsSimple(v5, v46);
    v47 = kasGrowSelectionPtr(89157026);
    v6 = (int *)kasGrowSelectionPtr(89157026);
    kasfShipsSelectNotSalvagers(v6, v47);
    v48 = kasGrowSelectionPtr(218060896);
    v7 = kasGrowSelectionPtr(82017669);
    if ( kasfShipsSelectLinkedShips(v7, v48) )
    {
      v49 = (int *)kasGrowSelectionPtr(82017669);
      v8 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsAdd(v8, v49);
    }
    if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
    {
      v38 = kasGrowSelectionPtr(89157026);
      v9 = kasGrowSelectionPtr(96759714);
      if ( !kasfShipsSelectType(v9, v38, "sWorker") )
      {
        v39 = kasGrowSelectionPtr(218060896);
        v10 = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(v10, "sWorker", 1, v39, 0);
      }
    }
    v11 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v11, (char *)106506883);
    spSetSinglePlayerSaveInfo(6, m06_saveInfoName[strCurLanguage]);
  }
  if ( !kasfEntryValueGet(71552195) )
  {
    v12 = kasAITeamShipsPtrIfExists(143034798);
    if ( kasfShipsAnyInSensorRange(v12) )
    {
      kasfEntrySetValue(71552195, 1);
      kasfEntrySetValue(-2070337308, 1);
      v50 = kasGrowSelectionPtr(18336307);
      v13 = (int *)kasGrowSelectionPtr(45901433);
      kasSelectPlayerShipsSimple(v13, v50);
      v40 = sub_4908C0(143034798);
      v37 = kasGrowSelectionPtr(45901433);
      v14 = (int *)kasGrowSelectionPtr(82017669);
      if ( !sub_495A50(v14, v37, v40, 40000) )
      {
        v51 = (int *)kasGrowSelectionPtr(45901433);
        v15 = (int *)kasGrowSelectionPtr(82017669);
        kasfShipsAdd(v15, v51);
      }
      v52 = sub_4908C0(143034798);
      v16 = kasGrowSelectionPtr(82017669);
      sub_49A100(v16, (float *)v52);
      v41 = kasGrowSelectionPtr(82017669);
      v17 = (int *)kasGrowSelectionPtr(237188773);
      if ( sub_495950(v17, v41, 0) )
      {
        v18 = kasGrowSelectionPtr(237188773);
        sub_4984C0(v18, 1);
        v19 = kasGrowSelectionPtr(237188773);
        sub_495EE0(v19);
      }
      v20 = kasAITeamPtrIfExists(176850956);
      kasfMsgSend((struct AITeam *)(int)v20, (char *)82017669);
      v21 = kasAITeamPtrIfExists(160291907);
      kasfMsgSend((struct AITeam *)(int)v21, (char *)223429172);
    }
  }
  if ( !kasfEntryValueGet(71552196) && kasfEntryValueGet(-2122890664) && kasfDoesPlayerHave("TECH_PHASED_TELEMETRY") )
  {
    kasfEntrySetValue(71552196, 1);
    v22 = kasAITeamPtrIfExists(160291907);
    kasfMsgSend((struct AITeam *)(int)v22, (char *)157488868);
  }
  if ( !kasfEntryValueGet(76670563) )
  {
    v53 = kasGrowSelectionPtr(5366435);
    v23 = kasVolumePtr(132933767);
    kasfFindMissilesInside(v23, v53);
    v24 = kasAITeamShipsPtrIfExists(44055618);
    if ( kasfShipsAnyInSensorRange(v24) || (v25 = kasGrowSelectionPtr(5366435), kasfShipsAnyInSensorRange(v25)) )
    {
      v26 = (int *)kasGrowSelectionPtr(5366435);
      kasfGrowSelectionClear(v26);
      v27 = kasAITeamPtrIfExists(176850956);
      kasfMsgSend((struct AITeam *)(int)v27, (char *)5366435);
      kasfEntrySetValue(76670563, 1);
    }
  }
  if ( kasTimerExpiredById(-2076128093) )
  {
    kasfEntrySetValue(-1895654316, 1);
    v28 = kasAITeamPtrIfExists(91769491);
    kasfMsgSend((struct AITeam *)(int)v28, (char *)5801588);
  }
  if ( !kasfEntryValueGet(-2098527291) )
  {
    if ( kasfEntryValueGet(-2041267004) )
    {
      v29 = kasAITeamShipsPtrIfExists(44055618);
      if ( !kasfShipsCount(v29) )
      {
        kasfEntrySetValue(-2098527291, 1);
        v30 = kasAITeamPtrIfExists(160291907);
        kasfMsgSend((struct AITeam *)(int)v30, (char *)51858660);
        v31 = kasAITeamPtrIfExists(90274303);
        kasfMsgSend((struct AITeam *)(int)v31, (char *)44561349);
        kasTimerSetById(191739589, 30);
      }
    }
  }
  if ( kasTimerExpiredById(191739589) )
  {
    v32 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v32, (char *)191739589);
  }
  if ( !kasfEntryValueGet(71552197) )
  {
    v33 = kasGrowSelectionPtr(142191106);
    if ( !kasfShipsCount(v33) )
    {
      kasfEntrySetValue(71552197, 1);
      v34 = kasAITeamPtrIfExists(160291907);
      kasfMsgSend((struct AITeam *)(int)v34, (char *)144114996);
    }
  }
  if ( kasTimerExpiredById(-2090393486) )
  {
    v36 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v36, (char *)73868450);
  }
}

void Init_Mission06_HeavyDefenderTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(0x4C9664, (sdword)Init_Mission06_HeavyDefenderAttackRun, (sdword)Watch_Mission06_HeavyDefenderAttackRun);
}

void Watch_Mission06_HeavyDefenderTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission06_HeavyDefenderBuildDelay, (sdword)Watch_Mission06_HeavyDefenderBuildDelay);
    }
}

void Init_Mission06_HeavyDefenderBuildDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, 15 * kasfShipTypeBuildTime("tHeavyDefender"));
}

void Watch_Mission06_HeavyDefenderBuildDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(4693508))
    {
        kasfSpeechEvent(75929221, (sdword)Init_Mission06_HeavyDefenderLaunch, (sdword)Watch_Mission06_HeavyDefenderLaunch);
    }
}

void Init_Mission06_HeavyDefenderLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsCreateInside(
        kasGrowSelectionPtr(154639956),
        "tHeavyDefender",
        (char *)15,
        (sdword)kasAITeamShipsPtrIfExists(143034798),
        1);
    kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
}

void Watch_Mission06_HeavyDefenderLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(0x4C9664, (sdword)Init_Mission06_HeavyDefenderAttackRun, (sdword)Watch_Mission06_HeavyDefenderAttackRun);
    }
}

void Init_Mission06_HeavyDefenderAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationDelta();
    kasfProtect(kasAITeamShipsPtrIfExists(143034798));
}

void Watch_Mission06_HeavyDefenderAttackRun(void)
{
    AITeam *team;
    sdword shipsToBuild;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(0x4C9664, (sdword)Init_Mission06_HeavyDefenderAttackRun, (sdword)Watch_Mission06_HeavyDefenderAttackRun);
        return;
    }

    if (!kasfEntryValueGet(111581908) && kasfTeamCount() < 15)
    {
        shipsToBuild = 15 - kasfTeamCount();
        kasfEntryGetOrCreateSetValue(111581908, shipsToBuild);
        kasTimerCreateSetStartById(111581908, shipsToBuild * kasfShipTypeBuildTime("tHeavyDefender"));
    }
    if (!kasfEntryValueGet(84320888) && kasfEntryValueGet(111581908) && kasTimerExpiredById(111581908))
    {
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "tHeavyDefender",
            (char *)kasfEntryValueGet(111581908),
            (sdword)kasAITeamShipsPtrIfExists(143034798),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        kasfEntryGetOrCreateSetValue(84320888, 1);
    }
    if (kasfEntryValueGet(111581908) && kasfEntryValueGet(84320888) && kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(84320888, 0);
        kasfEntrySetValue(111581908, 0);
        kasfSpeechEvent(0x4C9664, (sdword)Init_Mission06_HeavyDefenderAttackRun, (sdword)Watch_Mission06_HeavyDefenderAttackRun);
    }
}

void Init_Mission06_AttackBomberTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(
        88512316,
        (sdword)Begin_Mission06_AttackBomberPatrol,
        (sdword)Watch_Mission06_AttackBomberPatrol);
}

void Watch_Mission06_AttackBomberTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword buildCount;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        return;
    }

    if (kasfTeamCount())
    {
        if (!kasfEntryValueGet(111581908) && kasfTeamCount() < 12)
        {
            buildCount = 12 - kasfTeamCount();
            kasfEntryGetOrCreateSetValue(111581908, buildCount);
            kasTimerCreateSetStartById(
                111581908,
                buildCount * kasfShipTypeBuildTime("tAttackBomber"));
        }
        if (!kasfEntryValueGet(84320888) &&
            kasfEntryValueGet(111581908) &&
            kasTimerExpiredById(111581908))
        {
            kasfShipsCreateInside(
                kasGrowSelectionPtr(154639956),
                "tHeavyDefender",
                (char *)kasfEntryValueGet(111581908),
                (sdword)kasAITeamShipsPtrIfExists(143034798),
                1);
            kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 2);
            kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
            kasfEntryGetOrCreateSetValue(84320888, 1);
        }
        if (kasfEntryValueGet(111581908) &&
            kasfEntryValueGet(84320888) &&
            kasfTeamFinishedLaunching())
        {
            kasfEntrySetValue(84320888, 0);
            kasfEntrySetValue(111581908, 0);
            kasfSpeechEvent(
                88512316,
                (sdword)Begin_Mission06_AttackBomberPatrol,
                (sdword)Watch_Mission06_AttackBomberPatrol);
        }
    }
    else
    {
        kasfSpeechEvent(
            4693508,
            (sdword)Init_Mission06_AttackBomberBuildDelay,
            (sdword)Watch_Mission06_AttackBomberBuildDelay);
    }
}

void Init_Mission06_AttackBomberBuildDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, 12 * kasfShipTypeBuildTime("tAttackBomber"));
}

void Watch_Mission06_AttackBomberBuildDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(4693508))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission06_AttackBomberLaunch,
            (sdword)Watch_Mission06_AttackBomberLaunch);
    }
}

void Init_Mission06_AttackBomberLaunch(void)
{
    GrowSelection *attackBombers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackBombers = kasGrowSelectionPtr((char *)154639956);
    kasfShipsCreateInside(
        attackBombers,
        "tAttackBomber",
        12,
        (sdword)kasAITeamShipsPtrIfExists((char *)143034798),
        1);
    kasAITeamShipsPtr(attackBombers, 2);
}

void Watch_Mission06_AttackBomberLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_AttackBomberPatrol,
            (sdword)Watch_Mission06_AttackBomberPatrol);
    }
}

void Begin_Mission06_ReinforcementWingGamma_Assault(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(110230338)))
    {
        kasfFormationClaw();
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(271954)))
    {
        kasfFormationSphere();
    }
    else
    {
        kasfFormationWall();
    }
    kasfProtect(kasAITeamShipsPtrIfExists(142191106));
}

void Watch_Mission06_ReinforcementWingGamma_Assault(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfThisTeamIs(kasAITeamPtrIfExists(110230338)) &&
         kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)0x5866BA4),
             kasGrowSelectionPtr((char *)0x36980EB),
             kasShipsVectorPtr((char *)0x36980EB),
             10000)) ||
        (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)) &&
         kasfShipsSelectMembersOfSelection(
             kasGrowSelectionPtr((char *)0x857BCE3),
             kasGrowSelectionPtr((char *)0x857BCE3),
             kasGrowSelectionPtr((char *)0x36980EB))))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_ReinforcementWingGamma_CombatLoop,
            (sdword)Watch_Mission06_ReinforcementWingGamma_CombatLoop);
    }
    else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            (sdword)0x4C9664,
            (sdword)Begin_Mission06_ReinforcementWingGamma_Assault,
            (sdword)Watch_Mission06_ReinforcementWingGamma_Assault);
    }
}

void Begin_Mission06_ReinforcementWingGamma_CombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4A2A29),
            10000))
    {
        kasfSpeechEvent(
            (sdword)0x4C9664,
            (sdword)Begin_Mission06_ReinforcementWingGamma_Assault,
            (sdword)Watch_Mission06_ReinforcementWingGamma_Assault);
        return;
    }

    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x857BCE3));
    }
    kasfFormationClaw();
    singlePlayerSortShipsByDistanceToPoint(
        kasGrowSelectionPtr((char *)0x36980EB),
        kasShipsVectorPtr((char *)0x4A2A29));
    kasGrowSelectionPtr((char *)0x36980EB);
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}

void Watch_Mission06_ReinforcementWingGamma_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            (sdword)0x4C9664,
            (sdword)Begin_Mission06_ReinforcementWingGamma_Assault,
            (sdword)Watch_Mission06_ReinforcementWingGamma_Assault);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_ReinforcementWingGamma_CombatLoop,
            (sdword)Watch_Mission06_ReinforcementWingGamma_CombatLoop);
    }
}

void Begin_Mission06_AttackBomberPatrol(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552186))
    {
        kasfEntryGetOrCreateSetValue(71552186, 1);
        kasfLabelledVectorSet(138444994, 0, 0, 0);
    }
    kasfVectorRandomizeInRange(
        kasVectorPtrIfExists((char *)138444994),
        kasTeamsVectorPtr((char *)143034798),
        14000);
    kasfFormationClaw();
    kasfMoveTo(kasVectorPtrIfExists((char *)138444994));
}

void Watch_Mission06_AttackBomberPatrol(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectMoving(kasGrowSelectionPtr((char *)0x524AE27), &team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_AttackBomberPatrol,
            (sdword)Watch_Mission06_AttackBomberPatrol);
        return;
    }

    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4A2A29),
            14000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_AttackBomberCombatLoop,
            (sdword)Watch_Mission06_AttackBomberCombatLoop);
    }
}

void Begin_Mission06_AttackBomberCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4A2A29),
            14000))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_AttackBomberPatrol,
            (sdword)Watch_Mission06_AttackBomberPatrol);
        return;
    }

    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x857BCE3));
    }
    singlePlayerSortShipsByDistanceToPoint(
        kasGrowSelectionPtr((char *)0x36980EB),
        kasShipsVectorPtr((char *)0x4A2A29));
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
    kasfFormationClaw();
    kasGrowSelectionPtr((char *)0x36980EB);
    tutDeallocateRootRegion();
}

void Watch_Mission06_AttackBomberCombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_AttackBomberPatrol,
            (sdword)Watch_Mission06_AttackBomberPatrol);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_AttackBomberCombatLoop,
            (sdword)Watch_Mission06_AttackBomberCombatLoop);
    }
}

void Init_Mission06_RepairTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfActivateTech("TechRepair");
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        374244,
        (sdword)Begin_Mission06_RepairTeam_HomePosition,
        (sdword)Watch_Mission06_RepairTeam_HomePosition);
}

void Watch_Mission06_RepairTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552187) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552187, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Begin_Mission06_RepairTeam_HomePosition(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfProtect(kasAITeamShipsPtrIfExists(143034798));
}

void Watch_Mission06_RepairTeam_HomePosition(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x3F9DA23),
            kasGrowSelectionPtr((char *)0x6DA2DA9),
            kasShipsVectorPtr((char *)0x6DA2DA9),
            10000) &&
        kasfShipsSelectCapital(
            kasGrowSelectionPtr((char *)0x3F9DA23),
            kasGrowSelectionPtr((char *)0x3F9DA23)) &&
        kasfShipsSelectDamaged(
            kasGrowSelectionPtr((char *)0x3F9DA23),
            kasGrowSelectionPtr((char *)0x3F9DA23),
            70))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission06_RepairTeam_AssistTarget,
            (sdword)Watch_Mission06_RepairTeam_AssistTarget);
    }
}

void Begin_Mission06_RepairTeam_AssistTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamSpecial(kasGrowSelectionPtr((char *)66705955));
}

void Watch_Mission06_RepairTeam_AssistTarget(void)
{
    GrowSelection *damagedTargets;
    sdword fullyHealedCount;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    damagedTargets = kasGrowSelectionPtr((char *)0x3F9DA23);
    fullyHealedCount = kasfShipsSelectHealthAtLeastPercent(
        kasGrowSelectionPtr((char *)0x4C96094),
        damagedTargets,
        100);
    if (fullyHealedCount == kasfShipsCount(damagedTargets))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_RepairTeam_HomePosition,
            (sdword)Watch_Mission06_RepairTeam_HomePosition);
    }
}

void Begin_Mission06_ReinforcementWingGamma_RequestRepairSupport(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfDockSupportWith(kasAITeamPtrIfExists(255849604));
}

void Watch_Mission06_ReinforcementWingGamma_RequestRepairSupport(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0xF3FF484)))
    {
        kasfKamikaze(kasGrowSelectionPtr((char *)0x4A2A29));
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }

    if (kasfTeamHealthLowest() == 40)
    {
        kasfSpeechEvent(
            (sdword)0x4C9664,
            (sdword)Begin_Mission06_ReinforcementWingGamma_Assault,
            (sdword)Watch_Mission06_ReinforcementWingGamma_Assault);
    }
}

void Init_Mission06_HeavyInterceptorWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(-2115683020, 0);
    kasfEntryGetOrCreateSetValue(-1974354429, 0);
    kasfEntryGetOrCreateSetValue(332667, 1);
    switch (GetGameDifficulty())
    {
        case 0:
            kasfEntryGetOrCreateSetValue(5302947, 3);
            break;
        case 1:
            kasfEntryGetOrCreateSetValue(5302947, 4);
            break;
        case 2:
            kasfEntryGetOrCreateSetValue(5302947, 6);
            break;
        case 3:
            kasfEntryGetOrCreateSetValue(5302947, 9);
            break;
        case 4:
            kasfEntryGetOrCreateSetValue(5302947, 12);
            break;
        default:
            break;
    }
}

void Init_Mission06_HeavyInterceptorWing_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(5735588, kasfRandom(30, 180));
}

void Init_Mission06_HeavyInterceptorWing_Spawn(void)
{
    GrowSelection *spawnedShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    spawnedShips = kasGrowSelectionPtr((char *)91769491);
    kasfShipsCreateInside(
        spawnedShips,
        "tHeavyInterceptor",
        5,
        (sdword)kasAITeamShipsPtrIfExists(143034798),
        1);
    kasAITeamShipsPtr(spawnedShips, 2);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, spawnedShips);
}

void Init_Mission06_HeavyInterceptorWing_DockAndReinforce(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    kasfFormationDelta();
    kasfDockStay(kasAITeamPtrIfExists(143034798));
}

void Watch_Mission06_HeavyInterceptorWing_DockAndReinforce(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x5296679),
            kasAITeamShipsPtrIfExists(143034798),
            kasShipsVectorPtr((char *)0x5296679),
            4000))
    {
        kasfEntryGetOrCreateSetValue(-1987912172, 1);
        kasfMsgSend(kasAITeamPtrIfExists((char *)0x489688), (char *)5801588);
    }
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfPingRemove("PingOnScouts");
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
    }
}

void Init_Mission06_RepairSupportTargetWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x489688), &team->shipList);
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfFormationWall();
}

void Watch_Mission06_RepairSupportTargetWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552188))
    {
        if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)) &&
            kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x36980EB),
                kasGrowSelectionPtr((char *)0x857BCE3),
                kasThisTeamsVectorPtr(),
                20000) >= 5)
        {
            goto beginHomePosition;
        }
        if (kasfMsgReceived((char *)5801588))
        {
            goto beginHomePosition;
        }
        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x5296679),
                kasGrowSelectionPtr((char *)0x4A2A29),
                kasTeamsVectorPtr(143034798),
                20000) &&
            kasfFindShipsTargetingSelection(
                kasAITeamShipsPtrIfExists(143034798),
                kasGrowSelectionPtr((char *)0x857BCE3)) &&
            kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr((char *)0x5296679),
                kasGrowSelectionPtr((char *)0x5296679),
                kasGrowSelectionPtr((char *)0x857BCE3)))
        {
            goto beginHomePosition;
        }
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(142191106)) &&
            !kasfEntryValueGet(-1925671224))
        {
            if (!kasfEntryValueGet(71552189) && !kasfTeamCount())
            {
                kasfEntryGetOrCreateSetValue(71552189, 1);
                kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
            }
            return;
        }
    }

beginHomePosition:
    kasfEntryGetOrCreateSetValue(71552188, 1);
    kasfEntryGetOrCreateSetValue(-1925671224, 1);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x593FE0C), kasGrowSelectionPtr((char *)0xCFF5860));
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x593FE0C), kasAITeamShipsPtrIfExists(90274303));
    kasfSpeechEvent(
        337061,
        (sdword)Begin_Mission06_RepairSupportTargetWing_HomePosition,
        (sdword)Watch_Mission06_RepairSupportTargetWing_HomePosition);
}

void Begin_Mission06_RepairSupportTargetWing_HomePosition(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *moveTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x2BC6679),
            kasThisTeamsVectorPtr(),
            10000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_RepairSupportTargetWing_CombatLoop,
            (sdword)Watch_Mission06_RepairSupportTargetWing_CombatLoop);
    }
    else
    {
        if (kasfFindShipsTargetingSelection(
                kasGrowSelectionPtr((char *)0x489688),
                kasGrowSelectionPtr((char *)0x857BCE3)))
        {
            singlePlayerSortShipsByDistanceToPoint(
                kasGrowSelectionPtr((char *)0x857BCE3),
                kasThisTeamsVectorPtr());
            moveTargets = kasGrowSelectionPtr((char *)0x857BCE3);
        }
        else
        {
            singlePlayerSortShipsByDistanceToPoint(
                kasGrowSelectionPtr((char *)0x593FE0C),
                kasThisTeamsVectorPtr());
            moveTargets = kasGrowSelectionPtr((char *)0x593FE0C);
        }
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x5425), moveTargets, 0);
        kasfMoveTo(kasShipsVectorPtrIfExists(21541));
        kasTimerDestroyById(337061);
        kasTimerCreateSetStartById(337061, kasfRandom(30, 60));
    }
}

void Watch_Mission06_RepairSupportTargetWing_HomePosition(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x2BC6679),
            kasThisTeamsVectorPtr(),
            10000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_RepairSupportTargetWing_CombatLoop,
            (sdword)Watch_Mission06_RepairSupportTargetWing_CombatLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList) ||
             kasTimerExpiredDestroyById(337061))
    {
        kasTimerDestroyById(337061);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission06_RepairSupportTargetWing_HomePosition,
            (sdword)Watch_Mission06_RepairSupportTargetWing_HomePosition);
    }
}

void Begin_Mission06_RepairSupportTargetWing_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *combatTargets = kasGrowSelectionPtr((char *)57245931);

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            combatTargets,
            kasGrowSelectionPtr((char *)45901433),
            kasThisTeamsVectorPtr(),
            10000) &&
        kasfUnderAttack(kasGrowSelectionPtr((char *)139967715)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)139967715),
            combatTargets))
    {
        kasfShipsAdd(combatTargets, kasGrowSelectionPtr((char *)139967715));
        if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)124053532), combatTargets) ||
            kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr((char *)124053532),
                combatTargets,
                kasGrowSelectionPtr((char *)93584908)))
        {
            singlePlayerSortShipsByDistanceToPoint(
                kasGrowSelectionPtr((char *)124053532),
                kasThisTeamsVectorPtr());
            kasfFormationWall();
        }
        else
        {
            singlePlayerSortShipsByDistanceToPoint(combatTargets, kasThisTeamsVectorPtr());
            kasfFormationWall();
        }
    }
    else
    {
        singlePlayerSortShipsByDistanceToPoint(
            kasGrowSelectionPtr((char *)93584908),
            kasThisTeamsVectorPtr());
        kasfFormationWall();
    }
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}

void Watch_Mission06_RepairSupportTargetWing_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(-1970409083))
    {
        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x52962),
                kasGrowSelectionPtr((char *)0x2BC6679),
                kasThisTeamsVectorPtr(),
                30000))
        {
            kasfEntryGetOrCreateSetValue(-1970409083, 1);
            kasfMsgSend((sdword)kasAITeamPtrIfExists(176850956), 176786452);
            kasfMusicPlay(19);
        }
    }
    if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_RepairSupportTargetWing_CombatLoop,
            (sdword)Watch_Mission06_RepairSupportTargetWing_CombatLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission06_RepairSupportTargetWing_HomePosition,
            (sdword)Watch_Mission06_RepairSupportTargetWing_HomePosition);
    }
}

static void Watch_Mission06_CarrierRoute_WaitForMessage(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552169))
    {
        kasfEntryGetOrCreateSetValue(71552169, 1);
        kasLabelledVectorSet((char *)0x4A78474, kasVectorPtrIfExists((char *)0x52671));
    }
    if (kasfEntryValueGet(143030468))
    {
        kasfMoveTo(kasVectorPtrIfExists((char *)0x4A78474));
    }
    kasTimerCreateSetStartById(0x5DB194, kasfRandom(5, 10));
}

static void Begin_Mission06_HeavyInterceptorWing_MoveToTargetVector(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(-1963575996))
    {
        kasfEntryGetOrCreateSetValue(-1963575996, 1);
        kasTimerCreateSetStartById(-1925498653, 5);
    }
    kasfFormationDelta();
    kasfMoveTo(kasVectorPtrIfExists((char *)0x5784A93));
}

static void Watch_Mission06_FleetSupportWing_RepositionDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfTacticsNeutral();
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_FleetSupportWing_MoveToSupportTarget,
            (sdword)Watch_Mission06_FleetSupportWing_MoveToSupportTarget);
    }
}

static bool32 Mission06_CarrierRouteHasActiveBeastDockCommand(AITeam *team)
{
    SelectCommand *selection = team->shipList.selection;
    sdword i;

    for (i = 0; i < selection->numShips; i++)
    {
        CommandToDo *command = selection->ShipPtr[i]->command;

        if (command != NULL &&
            command->ordertype.order == COMMAND_DOCK &&
            dockCommandGetMode(command) == DOCK_MODE_BEAST)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void Begin_Mission06_RepairSupportWing_AssistTarget(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0xF9DA272), kasThisTeamsVectorPtr());
    if (kasfShipsSelectIndex(kasGrowSelectionPtr(0x5425), kasGrowSelectionPtr(0xF9DA272), 0))
    {
        kasfTeamSpecial(kasGrowSelectionPtr(0xF9DA272));
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

static void Begin_Mission06_MothershipAttackTeam_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (kasfFindMissilesNearTeam(kasGrowSelectionPtr(0x51E2A3), 4000) < 10)
    {
        kasfTeamSpecialToggle();
    }
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x36980EB),
            kasGrowSelectionPtr(0x4A2A29),
            kasThisTeamsVectorPtr(),
            10000) &&
        kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x857BCE3));
    }
    if (!kasfShipsCount(kasGrowSelectionPtr(0x36980EB)))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission06_MothershipAttackTeam_CombatLoopDelay,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoopDelay);
        return;
    }
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasThisTeamsVectorPtr());
    kasfFormationWall();
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

static void Begin_Mission06_AttackMothership(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
}

void Init_Mission06_CarrierRouteCoordinator(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetNonRetaliation(&team->shipList);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfEntryGetOrCreateSetValue(337587, 0);
}

void Watch_Mission06_CarrierRouteCoordinator(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(-2015429404) &&
        !kasfEntryValueGet(211332148) &&
        kasfShipsSelectType(kasGrowSelectionPtr(90274303), kasGrowSelectionPtr(0x4A2A29), "kCarrier"))
    {
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(90274303));
        kasfShipsSetSupportFlag(&team->shipList, 0);
        kasfShipsAdd(kasGrowSelectionPtr(90274303), &team->shipList);
        kasfShipsSetDelayedDeath(&team->shipList, 0);
        kasfEntrySetValue(332667, 0);
        kasfEntryGetOrCreateSetValue(211332148, 1);
        kasfShipsSetFormation(&team->shipList, 1);
        kasfShipsSetPercentFueled(&team->shipList, 0);
        kasfTeamSetPercentDamaged(40);
        kasfTeamClearMaxVelocity();
        kasfSpeechEvent(
            374244,
            (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
            (sdword)Watch_Mission06_CarrierRouteMinefield);
        return;
    }

    kasfShipsSelectType(kasGrowSelectionPtr(225018979), kasGrowSelectionPtr(0x4A2A29), "sRecon");
    if (!kasfEntryValueGet(71552161) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(68039894),
            kasGrowSelectionPtr(225018979),
            kasVectorPtrIfExists(337521),
            5000))
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfPingRemove("PingOnNav1");
        kasfEntrySetValue(158186482, 1);
        kasfEntrySetValue(337587, kasfEntryValueGet(337587) + 1);
        kasLabelledVectorSet((char *)0x4A78474, kasVectorPtrIfExists(337521));
    }
    if (!kasfEntryValueGet(71552162) && kasfEntryValueGet(-2070337308))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasTimerCreateSetStartById(86289959, 30);
    }
    if (!kasfEntryValueGet(71552163) &&
        (kasfEntryValueGet(337587) & 1) != 0 &&
        kasTimerExpiredById(86289959))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)78087284);
        kasfMoveTo(kasVectorPtrIfExists((char *)78087284));
        kasfEntryGetOrCreateSetValue(143030468, 1);
    }
    if (!kasfEntryValueGet(71552164) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(68039894),
            kasGrowSelectionPtr(225018979),
            kasVectorPtrIfExists(337522),
            5000))
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfPingRemove("PingOnNav2");
        kasfEntrySetValue(158186482, 1);
        kasfEntrySetValue(337587, kasfEntryValueGet(337587) + 2);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)112539924);
    }
    if (!kasfEntryValueGet(71552165) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(68039894),
            kasGrowSelectionPtr(225018979),
            kasVectorPtrIfExists(337523),
            5000))
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfPingRemove("PingOnNav3");
        kasfEntrySetValue(158186482, 1);
        kasfEntrySetValue(337587, kasfEntryValueGet(337587) + 4);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)112539924);
    }
    if (!kasfEntryValueGet(71552166) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(68039894),
            kasGrowSelectionPtr(225018979),
            kasVectorPtrIfExists(337524),
            5000))
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        kasfPingRemove("PingOnNav4");
        kasfEntrySetValue(158186482, 1);
        kasfEntrySetValue(337587, kasfEntryValueGet(337587) + 8);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)112539924);
    }
    if (!kasfEntryValueGet(71552167) && kasfMsgReceived((char *)0x80966D4))
    {
        kasfEntryGetOrCreateSetValue(71552167, 1);
        kasfPingRemove("PingOnNav5");
        kasfEntrySetValue(158186482, 1);
        kasfEntrySetValue(337587, kasfEntryValueGet(337587) + 16);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)112539924);
    }
    if (!kasfEntryValueGet(71552168) && kasfEntryValueGet(337587))
    {
        kasfEntryGetOrCreateSetValue(71552168, 1);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)126134468);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)297300);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission06_CarrierRouteMinefield(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(143137711),
            kasGrowSelectionPtr(218060896),
            kasThisTeamsVectorPtr(),
            12000) &&
        kasfFindMissilesNearTeam(kasGrowSelectionPtr(5366435), 16000) < 5)
    {
        if ((kasfEntryValueGet(337587) & 1) != 0 &&
            !kasfEntryValueGet(71552170) &&
            kasfNearby(kasVectorPtrIfExists(337521), 7000) &&
            kasfEntryValueGet(-2066520459))
        {
            kasfEntryGetOrCreateSetValue(71552170, 1);
            kasLabelledVectorSet((char *)0x4A78474, kasVectorPtrIfExists(337522));
            kasfSpeechEvent(
                374244,
                (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
                (sdword)Watch_Mission06_CarrierRouteMinefield);
            return;
        }
        if ((kasfEntryValueGet(337587) & 2) != 0 &&
            !kasfEntryValueGet(71552171) &&
            kasfNearby(kasVectorPtrIfExists(337522), 7000))
        {
            kasfEntryGetOrCreateSetValue(71552171, 1);
            kasLabelledVectorSet((char *)0x4A78474, kasVectorPtrIfExists(337523));
            kasfSpeechEvent(
                374244,
                (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
                (sdword)Watch_Mission06_CarrierRouteMinefield);
            return;
        }
        if ((kasfEntryValueGet(337587) & 4) != 0 &&
            !kasfEntryValueGet(71552172) &&
            kasfNearby(kasVectorPtrIfExists(337523), 7000))
        {
            kasfEntryGetOrCreateSetValue(71552172, 1);
            kasLabelledVectorSet((char *)0x4A78474, kasVectorPtrIfExists(337524));
            kasfSpeechEvent(
                374244,
                (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
                (sdword)Watch_Mission06_CarrierRouteMinefield);
            return;
        }
        if ((kasfEntryValueGet(337587) & 8) != 0 &&
            !kasfEntryValueGet(71552173) &&
            kasfNearby(kasVectorPtrIfExists(337524), 7000))
        {
            kasfEntryGetOrCreateSetValue(71552173, 1);
            kasLabelledVectorSet((char *)0x4A78474, kasVectorPtrIfExists(337525));
            kasfSpeechEvent(
                374244,
                (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
                (sdword)Watch_Mission06_CarrierRouteMinefield);
            return;
        }
        if ((kasfEntryValueGet(337587) & 0x10) != 0 &&
            !kasfEntryValueGet(71552174) &&
            kasfNearby(kasVectorPtrIfExists(337525), 7000))
        {
            kasfEntryGetOrCreateSetValue(71552174, 1);
            kasfMsgSend(kasAITeamPtrIfExists((char *)0x489688), (char *)5801588);
            kasfSpeechEvent(
                300069,
                (sdword)Begin_Mission06_CarrierRouteFinalApproach,
                (sdword)Watch_Mission06_CarrierRouteFinalApproach);
        }
        else if (kasTimerExpiredById(0x5DB194))
        {
            kasfSpeechEvent(
                374244,
                (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
                (sdword)Watch_Mission06_CarrierRouteMinefield);
        }
    }
    else
    {
        kasfPingAddShipsIndexed(kasGrowSelectionPtr(5366435), "PingOnMines");
        kasfShipsAdd(kasGrowSelectionPtr(236038147), kasGrowSelectionPtr(5366435));
        kasfSpeechEvent(
            362816,
            (sdword)Begin_Mission06_CarrierRouteMineAlert,
            (sdword)Watch_Mission06_CarrierRouteMineAlert);
    }
}

void Begin_Mission06_CarrierRouteMineAlert(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(5366435, 0);
    kasfEntryGetOrCreateSetValue(21079, 0);
    kasfStop();
    if (kasfFindMissilesNearTeam(kasGrowSelectionPtr(5366435), 16000))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(44846711));
        kasfShipsAdd(kasGrowSelectionPtr(44846711), kasGrowSelectionPtr(5366435));
        kasfShipsRemove(kasGrowSelectionPtr(44846711), kasGrowSelectionPtr(236038147));
        kasfPingAddShipsIndexed(kasGrowSelectionPtr(44846711), "PingOnMines");
        kasfShipsAdd(kasGrowSelectionPtr(236038147), kasGrowSelectionPtr(5366435));
        kasfEntrySetValue(5366435, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)44622852);
        kasTimerDestroyById(337282);
        kasTimerCreateSetStartById(337282, 160);
    }
    else
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(236038147));
        kasfPingRemove("PingOnMines");
        kasfEntrySetValue(5366435, 0);
    }
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr(143137711),
            kasGrowSelectionPtr(218060896),
            kasThisTeamsVectorPtr(),
            12000))
    {
        if (!kasfEntryValueGet(71552175))
        {
            kasfEntryGetOrCreateSetValue(71552175, 1);
            kasfEntryGetOrCreateSetValue(76825205, 1);
        }
        if (kasfEntryValueGet(76825205))
        {
            kasfEntrySetValue(76825205, 0);
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)144862692);
        }
        kasTimerDestroyById(21079);
        kasTimerCreateSetStartById(21079, 160);
        kasfEntrySetValue(21079, 1);
    }
}

void Watch_Mission06_CarrierRouteMineAlert(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfFindMissilesNearTeam(kasGrowSelectionPtr(5366435), 16000))
    {
        if (!kasfEntryValueGet(5366435))
        {
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)44622852);
            kasTimerDestroyById(337282);
            kasTimerCreateSetStartById(337282, 160);
            kasfEntrySetValue(5366435, 1);
        }
        kasfGrowSelectionClear(kasGrowSelectionPtr(44846711));
        kasfShipsAdd(kasGrowSelectionPtr(44846711), kasGrowSelectionPtr(5366435));
        kasfShipsRemove(kasGrowSelectionPtr(44846711), kasGrowSelectionPtr(236038147));
        kasfPingAddShipsIndexed(kasGrowSelectionPtr(44846711), "PingOnMines");
        kasfShipsAdd(kasGrowSelectionPtr(236038147), kasGrowSelectionPtr(5366435));
        if (kasTimerExpiredDestroyById(337282))
        {
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)44622852);
            kasTimerSetById(337282, 160);
            kasTimerStartById(337282);
        }
    }
    else
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(236038147));
        kasfPingRemove("PingOnMines");
        kasfEntrySetValue(5366435, 0);
    }

    if (kasfEntryValueGet(21079))
    {
        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr(143137711),
                kasGrowSelectionPtr(218060896),
                kasThisTeamsVectorPtr(),
                6000))
        {
            kasTimerDestroyById(21079);
            kasfEntrySetValue(21079, 0);
        }
        if (kasTimerExpiredDestroyById(21079))
        {
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)144862692);
            kasTimerSetById(21079, 160);
            kasTimerStartById(21079);
        }
    }
    if (!kasfEntryValueGet(21079) &&
        !kasfShipsSelectNearby(
            kasGrowSelectionPtr(143137711),
            kasGrowSelectionPtr(218060896),
            kasThisTeamsVectorPtr(),
            12000))
    {
        kasfEntrySetValue(21079, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)144862692);
        kasTimerDestroyById(21079);
        kasTimerCreateSetStartById(21079, 160);
    }
    if (!kasfEntryValueGet(21079) && !kasfEntryValueGet(5366435))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Watch_Mission06_CarrierRoute_WaitForMessage,
            (sdword)Watch_Mission06_CarrierRouteMinefield);
    }
}

void Begin_Mission06_CarrierRouteFinalApproach(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfDockStay(kasGrowSelectionPtr(306404));
}

void Watch_Mission06_CarrierRouteFinalApproach(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552176) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(86599289),
            &team->shipList,
            kasShipsVectorPtrIfExists(306404),
            10000))
    {
        kasfEntryGetOrCreateSetValue(71552176, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)357797);
    }
    if (!Mission06_CarrierRouteHasActiveBeastDockCommand(team))
    {
        kasfEntrySetValue(332667, 1);
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)93086467);
        kasfShipsRemove(kasGrowSelectionPtr(93584908), &team->shipList);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission06_StrikeWingAlpha(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr(142191106), &team->shipList);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(
        175938357,
        (sdword)MaintainTeamContext,
        (sdword)Begin_Mission06_StrikeWingAlpha_Reposition);
}

void Watch_Mission06_StrikeWingAlpha(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamHealthLowest() <= 50)
        {
            kasfMsgSend(kasAITeamPtrIfExists(255849604), (char *)142191106);
        }
        if (kasfMsgReceived((char *)0x48615B5))
        {
            kasfTeamSetPercentDamaged(20);
        }
    }
    if (!kasfEntryValueGet(71552177) && kasfShipsAnyInSensorRange(&team->shipList))
    {
        kasfEntryGetOrCreateSetValue(71552177, 1);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)145266441);
        kasfMsgSend(kasAITeamPtrIfExists(90274303), (char *)134833876);
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr());
        if (kasfShipsSelectIndex(kasGrowSelectionPtr(237188773), kasGrowSelectionPtr(0x4A2A29), 0))
        {
            kasfShipsSetDontApplyPhysicsFlag(kasGrowSelectionPtr(237188773), 1);
            kasfShipsStop(kasGrowSelectionPtr(237188773));
        }
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)142191106);
    }
    if (!kasfEntryValueGet(71552178) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552178, 1);
        kasfSpeechEvent(
            342540,
            (sdword)Begin_Mission06_StrikeWingAlpha_ReinforceAndResume,
            (sdword)MaintainTeamContext);
    }
}

void Begin_Mission06_StrikeWingAlpha_Reposition(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasLabelledVectorSet((char *)0x879AA02, kasVectorPtrIfExists(337525));
        kasfLabelledVectorSet((char *)0x53AB7A4, 0x1770, 0, 0);
        kasfVectorAddClampToUniverse(kasVectorPtrIfExists(142191106), kasVectorPtrIfExists(87734180));
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(142191106));
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(255849604), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(271954), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(110230338), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(217519717), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(23283093), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(44055618), (char *)5801588);
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_StrikeWingAlpha_WaitPoint,
            (sdword)Watch_Mission06_StrikeWingAlpha_EngageTrigger);
    }
}

void Begin_Mission06_StrikeWingAlpha_WaitPoint(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(54634201))
    {
        kasfIntercept(kasGrowSelectionPtr(218060896));
    }
    else
    {
        kasfMoveTo(kasVectorPtrIfExists(142191106));
    }
}

void Watch_Mission06_StrikeWingAlpha_EngageTrigger(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(142457346),
            kasGrowSelectionPtr(0x4A2A29),
            kasThisTeamsVectorPtr(),
            10000) ||
        kasfUnderAttack(kasGrowSelectionPtr(67083059)))
    {
        kasfStop();
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_StrikeWingAlpha_CapitalShipCombatLoop,
            (sdword)Watch_Mission06_StrikeWingAlpha_CapitalShipCombatLoop);
    }
}

void Begin_Mission06_StrikeWingAlpha_CapitalShipCombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *targets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr(57245931),
            kasGrowSelectionPtr(0x4A2A29),
            kasThisTeamsVectorPtr(),
            10000))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_StrikeWingAlpha_WaitPoint,
            (sdword)Watch_Mission06_StrikeWingAlpha_EngageTrigger);
        return;
    }

    if (!kasfEntryValueGet(54634201) &&
        (kasfShipsSelectType(kasGrowSelectionPtr(191073585), kasGrowSelectionPtr(57245931), "kCarrier") ||
         kasfShipsSelectType(kasGrowSelectionPtr(191073586), kasGrowSelectionPtr(57245931), "sMothership")))
    {
        kasfShipsAdd(kasGrowSelectionPtr(54634201), kasGrowSelectionPtr(218060896));
        if (!kasfEntryValueGet(71552179))
        {
            kasfEntryGetOrCreateSetValue(71552179, 1);
            kasfMsgSend(kasAITeamPtrIfExists((char *)0x489688), (char *)5801588);
        }
        if (!kasfEntryValueGet(71552180))
        {
            kasfEntryGetOrCreateSetValue(71552180, 1);
            kasfMusicPlay(19);
        }
        kasfEntryGetOrCreateSetValue(54634201, 1);
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr(139967715),
            kasGrowSelectionPtr(139967715),
            kasGrowSelectionPtr(57245931)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(139967715));
    }

    if (kasfEntryValueGet(54634201))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
        kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(218060896));
        kasfShipsAdd(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(90274303));
        targets = kasGrowSelectionPtr(57245931);
        kasfShipsSortByDistanceToPoint(targets, kasThisTeamsVectorPtr());
    }
    else
    {
        targets = kasGrowSelectionPtr(57245931);
        kasfShipsSortByDistanceToPoint(targets, kasVectorPtrIfExists(142191106));
    }
    kasfFormationWall();
    tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}

void Watch_Mission06_StrikeWingAlpha_CapitalShipCombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_StrikeWingAlpha_CapitalShipCombatLoop,
            (sdword)Watch_Mission06_StrikeWingAlpha_CapitalShipCombatLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_StrikeWingAlpha_WaitPoint,
            (sdword)Watch_Mission06_StrikeWingAlpha_EngageTrigger);
    }
}

void Begin_Mission06_StrikeWingAlpha_ReinforceAndResume(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    if (!kasfEntryValueGet(71552181))
    {
        kasfEntryGetOrCreateSetValue(71552181, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)81430868);
    }
    kasfEntrySetValue(332667, 1);
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(217519717)))
    {
        kasfMsgSend(kasAITeamPtrIfExists(217519717), (char *)90869349);
        kasfReinforceTeamWithShips(team, kasAITeamShipsPtrIfExists(217519717));
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_StrikeWingAlpha_WaitPoint,
            (sdword)Watch_Mission06_StrikeWingAlpha_EngageTrigger);
        return;
    }
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(255849604)))
    {
        kasfMsgSend(kasAITeamPtrIfExists(255849604), (char *)90869349);
        kasfReinforceTeamWithShips(team, kasAITeamShipsPtrIfExists(255849604));
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_StrikeWingAlpha_WaitPoint,
            (sdword)Watch_Mission06_StrikeWingAlpha_EngageTrigger);
        return;
    }
    kasfMsgSend(kasAITeamPtrIfExists(271954), (char *)0x4866F9);
    kasfMsgSend(kasAITeamPtrIfExists(110230338), (char *)0x4866F9);
}

void Init_Mission06_RepairStrikeWingBeta(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr(142191106), &team->shipList);
    kasfShipsAdd(kasGrowSelectionPtr(255849604), &team->shipList);
    kasfActivateTech("TechRepair");
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfGrowSelectionClear(kasGrowSelectionPtr(114048476));
    kasfSpeechEvent(
        175938357,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission06_RepairStrikeWingBeta_RegroupTrigger);
}

void Watch_Mission06_RepairStrikeWingBeta(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword *queueHead;
    sdword queueIndex;
    void *message;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == (void *)142191106)
        {
            kasfEntryGetOrCreateSetValue(293513, 1);
            kasfShipsAdd(kasGrowSelectionPtr(114048476), kasAITeamShipsPtrIfExists(142191106));
            if (!kasfEntryValueGet(332667))
            {
                kasfSpeechEvent(
                    313692,
                    (sdword)Begin_Mission06_RepairStrikeWingBeta_RegroupOrder,
                    (sdword)Watch_Mission06_RepairStrikeWingBeta_FullyRepaired);
                return;
            }
        }
        else if (message == (void *)217519717)
        {
            if (!kasfEntryValueGet(293513))
            {
                kasfShipsAdd(kasGrowSelectionPtr(114048476), kasAITeamShipsPtrIfExists(217519717));
            }
            if (!kasfEntryValueGet(332667))
            {
                kasfSpeechEvent(
                    313692,
                    (sdword)Begin_Mission06_RepairStrikeWingBeta_RegroupOrder,
                    (sdword)Watch_Mission06_RepairStrikeWingBeta_FullyRepaired);
                return;
            }
        }
        else if (message == (void *)90869349)
        {
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
            return;
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0)
        {
            break;
        }
        queueIndex--;
    }

    if (!kasfEntryValueGet(71552182) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552182, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission06_RepairStrikeWingBeta_RegroupTrigger(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntrySetValue(332667, 0);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(142191106));
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_RepairStrikeWingBeta_RegroupMove,
            (sdword)Watch_Mission06_RepairStrikeWingBeta_FullyRepaired);
    }
}

void Begin_Mission06_RepairStrikeWingBeta_RegroupMove(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfFormationWall();
    kasfProtect(kasAITeamShipsPtrIfExists(142191106));
}

void Begin_Mission06_RepairStrikeWingBeta_RegroupOrder(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfTeamSpecial(kasGrowSelectionPtr(114048476));
}

void Watch_Mission06_RepairStrikeWingBeta_FullyRepaired(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectHealthAtLeastPercent(kasGrowSelectionPtr(0x487F5D5), kasGrowSelectionPtr(0x6CC3DDC), 100) ==
        kasfShipsCount(kasGrowSelectionPtr(0x6CC3DDC)))
    {
        kasfEntrySetValue(332667, 0);
        kasfEntryGetOrCreateSetValue(293513, 0);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_RepairStrikeWingBeta_RegroupMove,
            (sdword)Watch_Mission06_RepairStrikeWingBeta_RegroupTrigger);
    }
}

void Init_Mission06_ReinforcementWingGamma(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr(0x879AA02), &team->shipList);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(
        175938357,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission06_ReinforcementWingGamma_RegroupTrigger);
}

void Watch_Mission06_ReinforcementWingGamma(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword *queueHead;
    sdword queueIndex;
    void *message;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamHealthLowest() < 40)
        {
            if (!kasfThisTeamIs(kasAITeamPtrIfExists(217519717)))
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_Mission06_ReinforcementWingGamma_RequestRepairSupport,
                    (sdword)Watch_Mission06_ReinforcementWingGamma_RequestRepairSupport);
                return;
            }
            kasfMsgSend(kasAITeamPtrIfExists(255849604), (char *)217519717);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = (queueHead != NULL) ? *queueHead : -1;
        while (queueIndex >= 0)
        {
            message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
            if (message == (void *)0x4866F9)
            {
                kasfTacticsAggressive();
                tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
                break;
            }
            if (message == (void *)90869349)
            {
                kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
                return;
            }
            if (message != NULL)
            {
                aitQueuePointer(team, team, message);
            }

            queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
            if (queueHead == NULL || *queueHead == 0)
            {
                break;
            }
            queueIndex--;
        }
    }
    if (!kasfEntryValueGet(71552183) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552183, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission06_ReinforcementWingGamma_RegroupTrigger(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntrySetValue(332667, 0);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(142191106));
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_ReinforcementWingGamma_Assault,
            (sdword)Watch_Mission06_ReinforcementWingGamma_Assault);
    }
}

void Watch_Mission06_HeavyInterceptorWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(91769491)))
        {
            kasTimerCreateSetStartById(92825426, 140);
            if (GetGameDifficulty() > 2)
            {
                kasTimerCreateSetStartById(92825427, 360);
            }
            if (GetGameDifficulty() > 3)
            {
                kasTimerCreateSetStartById(92825428, 600);
            }
        }
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission06_HeavyInterceptorWing_Spawn,
            (sdword)Watch_Mission06_HeavyInterceptorWing_Spawn);
        return;
    }
    if (kasfThisTeamIs(kasAITeamPtrIfExists(91769491)))
    {
        if (kasTimerExpiredById(92825426))
        {
            kasfMsgSend(kasAITeamPtrIfExists(126134546), (char *)5801588);
        }
        else if (kasTimerExpiredById(92825427))
        {
            kasfMsgSend(kasAITeamPtrIfExists(126134547), (char *)5801588);
        }
        else if (kasTimerExpiredById(92825428))
        {
            kasfMsgSend(kasAITeamPtrIfExists(126134548), (char *)5801588);
        }
    }
    if (!kasfEntryValueGet(332667))
    {
        if (!kasfTeamCount() && !kasfEntryValueGet(5302947))
        {
            kasfEntrySetValue(-1974354429, 0);
            kasfEntryDecrement(5302947);
            kasfSpeechEvent(
                374244,
                (sdword)Init_Mission06_HeavyInterceptorWing_SpawnDelay,
                (sdword)Watch_Mission06_HeavyInterceptorWing_Spawn);
        }
        else if (!kasfTeamCount() && kasfEntryValueGet(5302947))
        {
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
        else if (kasfEntryValueGet(-1925671224))
        {
            kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr());
            kasfKamikaze(kasGrowSelectionPtr(0x4A2A29));
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
    }
}

void Watch_Mission06_HeavyInterceptorWing_Spawn(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfShipsSetMaxVelocity(&team->shipList, 675);
        kasfStop();
        kasfSpeechEvent(
            84320888,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission06_HeavyInterceptorWing_LaunchTrigger);
    }
}

void Watch_Mission06_HeavyInterceptorWing_LaunchTrigger(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        if (kasfShipsAnyInSensorRange(kasAITeamShipsPtrIfExists(143034798)))
        {
            if (kasfEntryValueGet(-1907862091))
            {
                if (!kasfEntryValueGet(-1987912172))
                {
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)244251591);
                }
            }
            else
            {
                kasfEntryGetOrCreateSetValue(-1907862091, 1);
            }
        }
        kasLabelledVectorSet((char *)0x5784A93, kasVectorPtrIfExists(74019713));
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_HeavyInterceptorWing_MoveToTargetVector,
            (sdword)Init_Mission06_HeavyInterceptorWing_HuntTargets);
    }
}

void Init_Mission06_HeavyInterceptorWing_HuntTargets(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    vector *destination;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(-1925498653) &&
        kasfShipsAnyInSensorRange(&team->shipList) &&
        !kasfEntryValueGet(-1987912172) &&
        !kasfEntryValueGet(-1974354429) &&
        kasfEntryValueGet(-2115683020) < 2)
    {
        kasfEntryIncrement(-2115683020);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)91769491);
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)) &&
        kasfShipsAnyInSensorRange(&team->shipList) &&
        (kasfShipsSelectNearby(
             kasGrowSelectionPtr(0x46784A4),
             kasGrowSelectionPtr(0xCFF5860),
             kasThisTeamsVectorPtr(),
             1000) ||
         kasfShipsSelectNearby(
             kasGrowSelectionPtr(0x46784A4),
             kasAITeamShipsPtrIfExists(90274303),
             kasThisTeamsVectorPtr(),
             1000)))
    {
        if (!kasfEntryValueGet(-1987912172) && !kasfEntryValueGet(71552184))
        {
            kasfEntryGetOrCreateSetValue(71552184, 1);
            kasfEntrySetValue(-1974354429, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)189409763);
        }
        kasfSpeechEvent(
            299899,
            (sdword)Init_Mission06_HeavyInterceptorWing_DockAndReinforce,
            (sdword)Watch_Mission06_HeavyInterceptorWing_DockAndReinforce);
        return;
    }

    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x36980EB),
            kasGrowSelectionPtr(0x4A2A29),
            kasThisTeamsVectorPtr(),
            16000))
    {
        if (!kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)) ||
            !kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr(0x857BCE3),
                kasGrowSelectionPtr(0x857BCE3),
                kasGrowSelectionPtr(0x36980EB)))
        {
            if (!kasfShipsAnyIdle(&team->shipList))
            {
                return;
            }
            if (kasfRandom(1, 10) <= 9)
            {
                if (kasfNearby(kasVectorPtrIfExists(74019713), 8000))
                {
                    destination = kasVectorPtrIfExists(74019714);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019714), 8000))
                {
                    destination = kasVectorPtrIfExists(74019715);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019715), 8000))
                {
                    destination = kasVectorPtrIfExists(74019716);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019716), 8000))
                {
                    destination = kasVectorPtrIfExists(74019717);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019717), 8000))
                {
                    destination = kasVectorPtrIfExists(74019718);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019718), 8000))
                {
                    destination = kasVectorPtrIfExists(74019719);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019719), 8000))
                {
                    destination = kasVectorPtrIfExists(74019720);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019720), 8000))
                {
                    destination = kasVectorPtrIfExists(74019721);
                }
                else if (kasfNearby(kasVectorPtrIfExists(74019721), 8000))
                {
                    destination = kasVectorPtrIfExists(110573664);
                }
                else if (kasfNearby(kasVectorPtrIfExists(110573664), 8000))
                {
                    destination = kasVectorPtrIfExists(110573665);
                }
                else if (kasfNearby(kasVectorPtrIfExists(110573665), 8000))
                {
                    destination = kasVectorPtrIfExists(110573666);
                }
                else if (kasfNearby(kasVectorPtrIfExists(110573666), 8000))
                {
                    destination = kasVectorPtrIfExists(110573667);
                }
                else if (kasfNearby(kasVectorPtrIfExists(110573667), 8000))
                {
                    destination = kasVectorPtrIfExists(110573668);
                }
                else if (kasfNearby(kasVectorPtrIfExists(110573668), 8000))
                {
                    destination = kasVectorPtrIfExists(110573669);
                }
                else if (kasfNearby(kasVectorPtrIfExists(110573669), 8000))
                {
                    destination = kasVectorPtrIfExists(74019713);
                }
                else
                {
                    kasfSpeechEvent(
                        88512316,
                        (sdword)Begin_Mission06_HeavyInterceptorWing_MoveToTargetVector,
                        (sdword)Init_Mission06_HeavyInterceptorWing_HuntTargets);
                    return;
                }
            }
            else
            {
                destination = kasTeamsVectorPtr(90274303);
            }
            kasLabelledVectorSet((char *)0x5784A93, destination);
            kasfSpeechEvent(
                88512316,
                (sdword)Begin_Mission06_HeavyInterceptorWing_MoveToTargetVector,
                (sdword)Init_Mission06_HeavyInterceptorWing_HuntTargets);
            return;
        }
    }
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission06_HeavyInterceptorWing_CombatLoop,
        (sdword)Watch_Mission06_HeavyInterceptorWing_CombatLoop);
}

void Begin_Mission06_HeavyInterceptorWing_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x36980EB),
            kasGrowSelectionPtr(0x4A2A29),
            kasThisTeamsVectorPtr(),
            16000))
    {
        if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
        {
            kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x857BCE3));
        }
        kasfFormationDelta();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasThisTeamsVectorPtr());
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
        kasTimerDestroyById(74024315);
        kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
    }
    else
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_HeavyInterceptorWing_MoveToTargetVector,
            (sdword)Init_Mission06_HeavyInterceptorWing_HuntTargets);
    }
}

void Watch_Mission06_HeavyInterceptorWing_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)) &&
         kasfShipsSelectNearby(
             kasGrowSelectionPtr(0x46784A4),
             kasGrowSelectionPtr(0xCFF5860),
             kasThisTeamsVectorPtr(),
             10000)) ||
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x46784A4),
            kasAITeamShipsPtrIfExists(90274303),
            kasThisTeamsVectorPtr(),
            10000))
    {
        if (!kasfEntryValueGet(-1987912172) && !kasfEntryValueGet(71552185))
        {
            kasfEntryGetOrCreateSetValue(71552185, 1);
            kasfEntrySetValue(-1974354429, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)189409763);
        }
        kasfPingAddShips(&team->shipList, "PingOnScouts");
        kasfSpeechEvent(
            299899,
            (sdword)Init_Mission06_HeavyInterceptorWing_DockAndReinforce,
            (sdword)Watch_Mission06_HeavyInterceptorWing_DockAndReinforce);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission06_HeavyInterceptorWing_MoveToTargetVector,
            (sdword)Init_Mission06_HeavyInterceptorWing_HuntTargets);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_HeavyInterceptorWing_CombatLoop,
            (sdword)Watch_Mission06_HeavyInterceptorWing_CombatLoop);
    }
}

void Begin_Mission06_HeavyInterceptorWing_RegroupMove(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfFormationWall();
    kasfProtect(kasAITeamShipsPtrIfExists(143034798));
}

void Init_Mission06_FleetSupportWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    if (kasfMsgReceived((char *)0xA7C9B35))
    {
        kasfSpeechEvent(
            175938357,
            (sdword)Begin_Mission06_AttackMothership,
            (sdword)Watch_Mission06_FleetSupportWing_AttackCueDelay);
    }
    else
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_FleetSupportWing_MoveToSupportTarget,
            (sdword)Watch_Mission06_FleetSupportWing_MoveToSupportTarget);
    }
}

void Watch_Mission06_FleetSupportWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552190) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552190, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission06_FleetSupportWing_AttackCueDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasTimerCreateSetStartById(175938357, 10);
    }
    if (kasTimerExpiredById(175938357))
    {
        kasfTeamHyperspaceInNear(kasTeamsVectorPtr(44055618), 500);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_FleetSupportWing_MoveToSupportTarget,
            (sdword)Watch_Mission06_FleetSupportWing_MoveToSupportTarget);
    }
}

void Begin_Mission06_FleetSupportWing_MoveToSupportTarget(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x93C9634));
    if (kasfShipsSelectCapital(kasGrowSelectionPtr(0x764E81C), kasGrowSelectionPtr(0x6DA2DA9)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasGrowSelectionPtr(0x764E81C));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(0x2A03C42), kasGrowSelectionPtr(0x6DA2DA9), "tMinelayerCorvette"))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), kasGrowSelectionPtr(0x2A03C42));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(0x9AA2C22), kasGrowSelectionPtr(0x6DA2DA9), "tHeavyDefender") &&
        kasfShipsSelectTargets(kasGrowSelectionPtr(0x9668AE7), kasGrowSelectionPtr(0x9AA2C22)))
    {
        kasfShipsRemove(kasGrowSelectionPtr(0x93C9634), kasGrowSelectionPtr(0x9668AE7));
    }

    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x93C9634), kasThisTeamsVectorPtr());
    if (kasfShipsSelectIndex(kasGrowSelectionPtr(0x5425), kasGrowSelectionPtr(0x93C9634), 0) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x4C9664),
            kasGrowSelectionPtr(0x6DA2DA9),
            kasShipsVectorPtrIfExists(0x5425),
            2000))
    {
        kasfShipsRemove(kasGrowSelectionPtr(0x4C9664), kasGrowSelectionPtr(0x5425));
        kasfFormationWall();
        kasfProtect(kasGrowSelectionPtr(0x4C9664));
        return;
    }

    kasfSpeechEvent(
        374244,
        (sdword)Init_Mission06_FleetSupportWing_RepositionDelay,
        (sdword)Watch_Mission06_FleetSupportWing_RepositionDelay);
}

void Watch_Mission06_FleetSupportWing_MoveToSupportTarget(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_FleetSupportWing_MoveToSupportTarget,
            (sdword)Watch_Mission06_FleetSupportWing_MoveToSupportTarget);
    }
}

void Init_Mission06_FleetSupportWing_RepositionDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTacticsAggressive();
    kasfFormationWall();
    kasTimerCreateSetStartById(374244, kasfRandom(30, 60));
}

void Init_Mission06_MothershipAttackEscort(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    if (kasfMsgReceived((char *)0xA7C9B35))
    {
        kasfSpeechEvent(
            175938357,
            (sdword)Begin_Mission06_AttackMothership,
            (sdword)Watch_Mission06_MothershipAttackEscort_AttackCueDelay);
    }
    else
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_MothershipAttackEscort_FormOnAttackTeam,
            (sdword)Watch_Mission06_MothershipAttackEscort_FormOnAttackTeam);
    }
}

void Watch_Mission06_MothershipAttackEscort(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552191) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552191, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission06_MothershipAttackEscort_AttackCueDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasTimerCreateSetStartById(175938357, 10);
    }
    if (kasTimerExpiredById(175938357))
    {
        kasfTeamHyperspaceInNear(kasTeamsVectorPtr(44055618), 500);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_MothershipAttackEscort_FormOnAttackTeam,
            (sdword)Watch_Mission06_MothershipAttackEscort_FormOnAttackTeam);
    }
}

void Begin_Mission06_MothershipAttackEscort_FormOnAttackTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x93C9634));
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(44055618)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(44055618));
    }
    else
    {
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission06_MothershipAttackEscort_CombatLoopDelay,
            (sdword)Watch_Mission06_MothershipAttackEscort_CombatLoopDelay);
    }
}

void Watch_Mission06_MothershipAttackEscort_FormOnAttackTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_MothershipAttackEscort_FormOnAttackTeam,
            (sdword)Watch_Mission06_MothershipAttackEscort_FormOnAttackTeam);
    }
}

void Init_Mission06_MothershipAttackEscort_CombatLoopDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTacticsAggressive();
    kasfFormationDelta3D();
    kasTimerCreateSetStartById(374244, kasfRandom(30, 60));
}

void Watch_Mission06_MothershipAttackEscort_CombatLoopDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfTacticsNeutral();
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission06_MothershipAttackEscort_FormOnAttackTeam,
            (sdword)Watch_Mission06_MothershipAttackEscort_FormOnAttackTeam);
    }
}

void Init_Mission06_RepairSupportWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfActivateTech("TechRepair");
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        374244,
        (sdword)Begin_Mission06_RepairSupportWing_MoveToTarget,
        (sdword)Watch_Mission06_RepairSupportWing_MoveToTarget);
}

void Watch_Mission06_RepairSupportWing(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552192) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552192, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Begin_Mission06_RepairSupportWing_MoveToTarget(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x489688), kasThisTeamsVectorPtr());
    if (kasfShipsSelectIndex(kasGrowSelectionPtr(21541), kasGrowSelectionPtr(0x489688), 0))
    {
        kasfProtect(kasGrowSelectionPtr(21541));
    }
}

void Watch_Mission06_RepairSupportWing_MoveToTarget(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsSelectNearby(
             kasGrowSelectionPtr(0xF9DA272),
             kasGrowSelectionPtr(0x6DA2DA9),
             kasThisTeamsVectorPtr(),
             10000) ||
         kasfShipsAdd(kasGrowSelectionPtr(0xF9DA272), kasGrowSelectionPtr(0x489688))) &&
        kasfShipsSelectCapital(kasGrowSelectionPtr(0xF9DA272), kasGrowSelectionPtr(0xF9DA272)) &&
        kasfShipsSelectDamaged(kasGrowSelectionPtr(0xF9DA272), kasGrowSelectionPtr(0xF9DA272), 70))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission06_RepairSupportWing_AssistTarget,
            (sdword)Watch_Mission06_RepairSupportWing_AssistTarget);
    }
    else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0 &&
             kasfShipsCount(kasGrowSelectionPtr(0x489688)))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_RepairSupportWing_MoveToTarget,
            (sdword)Watch_Mission06_RepairSupportWing_MoveToTarget);
    }
}

void Watch_Mission06_RepairSupportWing_AssistTarget(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectHealthAtLeastPercent(kasGrowSelectionPtr(0x4C96094), kasGrowSelectionPtr(0xF9DA272), 100) ==
        kasfShipsCount(kasGrowSelectionPtr(0xF9DA272)))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission06_RepairSupportWing_MoveToTarget,
            (sdword)Watch_Mission06_RepairSupportWing_MoveToTarget);
    }
}

void Init_Mission06_MothershipAttackTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword difficulty;
    sdword shipsToDestroy;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    difficulty = GetGameDifficulty();
    if (difficulty == 0)
    {
        shipsToDestroy = 4;
    }
    else if (difficulty == 1)
    {
        shipsToDestroy = 3;
    }
    else if (difficulty == 2)
    {
        shipsToDestroy = 2;
    }
    else
    {
        shipsToDestroy = 0;
    }
    if (shipsToDestroy > 0)
    {
        kasfShipsSelectFirstN(kasGrowSelectionPtr(0x38A0F15), &team->shipList, shipsToDestroy);
        kasfVarDestroy(kasGrowSelectionPtr(0x38A0F15));
    }
    kasfSpeechEvent(
        175938357,
        (sdword)MaintainTeamContext,
        (sdword)Begin_Mission06_MothershipAttackTeam_ApproachVector);
}

void Watch_Mission06_MothershipAttackTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(332667) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x36980EB),
            kasGrowSelectionPtr(0x4A2A29),
            kasThisTeamsVectorPtr(),
            10000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_MothershipAttackTeam_CombatLoop,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoop);
    }
    else if (!kasfEntryValueGet(71552193) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552193, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
    }
}

void Begin_Mission06_MothershipAttackTeam_ApproachVector(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntryGetOrCreateSetValue(-2041267004, 1);
        kasfEntrySetValue(332667, 0);
        kasfLabelledVectorSet((char *)0x2A03C42, 0, 0, 0);
        kasfVectorRandomizeInRange(kasVectorPtrIfExists((char *)0x2A03C42), kasTeamsVectorPtr(142191106), 50000);
        kasLabelledVectorSet((char *)0x161C32, kasVectorPtrIfExists((char *)0x2A03C42));
        kasfTeamHyperspaceIn(kasVectorPtrIfExists((char *)0x2A03C42));
        kasfMsgSend(kasAITeamPtrIfExists(67782180), (char *)5801588);
        kasfSpeechEvent(
            5366435,
            (sdword)Begin_Mission06_MothershipAttackTeam_CombatLoopMove,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoopMove);
    }
}

void Begin_Mission06_MothershipAttackTeam_CombatLoopMove(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword blend;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryScopeName(4591038);
    kasTimerDestroyById(4591038);
    kasTimerDestroyById(31738387);
    kasfLabelledVectorSet((char *)0x526B3, 0, 0, 0);
    blend = kasfRandom(30, 60);
    kasfVectorBlendClampToUniverse(
        kasVectorPtrIfExists((char *)0x526B3),
        kasVectorPtrIfExists((char *)0x52674),
        kasVectorPtrIfExists((char *)0x52671),
        blend,
        100);
    kasfVectorRandomizeInRange(
        kasVectorPtrIfExists((char *)0x2A03C42),
        kasVectorPtrIfExists((char *)0x526B3),
        200);
    kasfMoveTo(kasVectorPtrIfExists((char *)0x2A03C42));
    kasfFormationWall();
}

void Watch_Mission06_MothershipAttackTeam_CombatLoopMove(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) && !kasfEntryValueGet(4591038))
    {
        kasfEntryGetOrCreateSetValue(4591038, 1);
        kasfFormationWall();
        kasfMoveTo(kasShipsVectorPtrIfExists(218060896));
        kasTimerCreateSetStartById(4591038, 15);
    }
    if (kasfEntryValueGet(4591038) && kasTimerExpiredById(4591038))
    {
        kasfEntryScopeName(4591038);
        kasfStop();
        kasTimerCreateSetStartById(31738387, 3);
        if (kasfFindMissilesNearTeam(kasGrowSelectionPtr((char *)0x51E2A3), 6000) < 10)
        {
            kasfTeamSpecialToggle();
        }
    }
    if (kasTimerExpiredById(31738387))
    {
        kasfStop();
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission06_MothershipAttackTeam_CombatLoopDelay,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoopDelay);
    }
}

void Init_Mission06_MothershipAttackTeam_CombatLoopDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerCreateSetStartById(374244, kasfRandom(160, 300));
}

void Watch_Mission06_MothershipAttackTeam_CombatLoopDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfSpeechEvent(
            5366435,
            (sdword)Begin_Mission06_MothershipAttackTeam_CombatLoopMove,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoopMove);
    }
}

void Watch_Mission06_MothershipAttackTeam_CombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Init_Mission06_MothershipAttackTeam_CombatLoopDelay,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoopDelay);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission06_MothershipAttackTeam_CombatLoop,
            (sdword)Watch_Mission06_MothershipAttackTeam_CombatLoop);
    }
}


extern sdword Watch_Mission05_ObjectiveEventQueue(void);

void Init_Mission06(void)
{
    kasCurrentStep = 0;
    scope = KH_mission06;

    kasfMusicPlay(5);
    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));

    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext,
                 (KASWatchFunction)Watch_Mission05_ObjectiveEventQueue,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(KH_FleetIntel, Init_Mission05_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(90274303, Init_Mission06_CarrierRouteCoordinator, Watch_Mission06_CarrierRouteCoordinator,
                 kasAITeamPtrIfExists(90274303));
    kasFSMCreate(142191106, Init_Mission06_StrikeWingAlpha, Watch_Mission06_StrikeWingAlpha,
                 kasAITeamPtrIfExists(142191106));
    kasFSMCreate(255849604, Init_Mission06_RepairStrikeWingBeta, Watch_Mission06_RepairStrikeWingBeta,
                 kasAITeamPtrIfExists(255849604));
    kasFSMCreate(217519717, Init_Mission06_ReinforcementWingGamma, Watch_Mission06_ReinforcementWingGamma,
                 kasAITeamPtrIfExists(217519717));
    kasFSMCreate(217519717, Init_Mission06_ReinforcementWingGamma, Watch_Mission06_ReinforcementWingGamma,
                 kasAITeamPtrIfExists(271954));
    kasFSMCreate(217519717, Init_Mission06_ReinforcementWingGamma, Watch_Mission06_ReinforcementWingGamma,
                 kasAITeamPtrIfExists(110230338));
    kasFSMCreate(217519717, Init_Mission06_ReinforcementWingGamma, Watch_Mission06_ReinforcementWingGamma,
                 kasAITeamPtrIfExists(23283093));
    kasFSMCreate(128253572, Begin_Mission06_HeavyInterceptorWing_RegroupMove, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(61737762));
    kasFSMCreate(138444994, Init_Mission06_AttackBomberTeam, Watch_Mission06_AttackBomberTeam,
                 kasAITeamPtrIfExists(138444994));
    kasFSMCreate(128253572, Begin_Mission06_HeavyInterceptorWing_RegroupMove, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(149733829));
    kasFSMCreate(128253572, Begin_Mission06_HeavyInterceptorWing_RegroupMove, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(157715559));
    kasFSMCreate(128253572, Begin_Mission06_HeavyInterceptorWing_RegroupMove, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(157715198));
    kasFSMCreate(128253572, Begin_Mission06_HeavyInterceptorWing_RegroupMove, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(107523938));
    kasFSMCreate(88096866, Init_Mission06_HeavyDefenderTeam, Watch_Mission06_HeavyDefenderTeam,
                 kasAITeamPtrIfExists(88096866));
    kasFSMCreate(13480900, Init_Mission06_RepairTeam, Watch_Mission06_RepairTeam,
                 kasAITeamPtrIfExists(208417508));
    kasFSMCreate(4757128, Init_Mission06_RepairSupportTargetWing, Watch_Mission06_RepairSupportTargetWing,
                 kasAITeamPtrIfExists(4757128));
    kasFSMCreate(4757128, Init_Mission06_RepairSupportTargetWing, Watch_Mission06_RepairSupportTargetWing,
                 kasAITeamPtrIfExists(76114066));
    kasFSMCreate(4757128, Init_Mission06_RepairSupportTargetWing, Watch_Mission06_RepairSupportTargetWing,
                 kasAITeamPtrIfExists(76114067));
    kasFSMCreate(204225700, Init_Mission06_RepairSupportWing, Watch_Mission06_RepairSupportWing,
                 kasAITeamPtrIfExists(204225700));
    kasFSMCreate(204225700, Init_Mission06_RepairSupportWing, Watch_Mission06_RepairSupportWing,
                 kasAITeamPtrIfExists(46385810));
    kasFSMCreate(204225700, Init_Mission06_RepairSupportWing, Watch_Mission06_RepairSupportWing,
                 kasAITeamPtrIfExists(46385811));
    kasFSMCreate(21013602, Init_Mission06_FleetSupportWing, Watch_Mission06_FleetSupportWing,
                 kasAITeamPtrIfExists(21013602));
    kasFSMCreate(21013602, Init_Mission06_FleetSupportWing, Watch_Mission06_FleetSupportWing,
                 kasAITeamPtrIfExists(67782178));
    kasFSMCreate(21013602, Init_Mission06_FleetSupportWing, Watch_Mission06_FleetSupportWing,
                 kasAITeamPtrIfExists(67782179));
    kasfMsgSend(kasAITeamPtrIfExists(67782180), (char *)0xA7C9B35);
    kasFSMCreate(71734692, Init_Mission06_MothershipAttackEscort, Watch_Mission06_MothershipAttackEscort,
                 kasAITeamPtrIfExists(67782180));
    kasFSMCreate(91769491, Init_Mission06_HeavyInterceptorWing, Watch_Mission06_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(91769491));
    kasFSMCreate(91769491, Init_Mission06_HeavyInterceptorWing, Watch_Mission06_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(126134546));
    kasFSMCreate(91769491, Init_Mission06_HeavyInterceptorWing, Watch_Mission06_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(126134547));
    kasFSMCreate(91769491, Init_Mission06_HeavyInterceptorWing, Watch_Mission06_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(126134548));
    kasFSMCreate(44055618, Init_Mission06_MothershipAttackTeam, Watch_Mission06_MothershipAttackTeam,
                 kasAITeamPtrIfExists(44055618));

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipMicro", "RACE_SECT", 1);
    kasfEnableShip("sMothershipSpecial", "RACE_SECT", 1);
    kasfEnableShip("sMothershipArmour", "RACE_SECT", 0);
    kasfEnableShip("sMothershipWeapons", "RACE_SECT", 1);

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_REPAIR");
    kasfAllowPlayerToResearch("TECH_SALVAGE");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_RESEARCH_CLEESAN_ATTACK");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sSentinel");
    kasfModuleSlotOrderSetShipType(0, "sLeech");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetShipType(0, "sHiveFrigate");
    kasfModuleSlotOrderSetCount(0, 5);
    kasfModuleSlotOrderSetShipType(1, "tHeavyInterceptor");
    kasfModuleSlotOrderSetShipType(1, "tHeavyDefender");
    kasfModuleSlotOrderSetShipType(1, "tAttackBomber");
    kasfModuleSlotOrderSetShipType(1, "tMinelayerCorvette");
    kasfModuleSlotOrderSetShipType(1, "tIonCannonFrigate");
    kasfModuleSlotOrderSetShipType(1, "tAdvanceSupportFrigate");
    kasfModuleSlotOrderSetCount(1, 4);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfDisableAllAIFeatures();
    kasfDisablePlayerHyperspace();
    kasfMusicPlay(5);

    kasfFindBeastShipsInside(kasVolumePtr(19508), kasGrowSelectionPtr(19508));
    kasfFindBeastShipsInside(kasVolumePtr(306404), kasGrowSelectionPtr(306404));

    kasTimerCreateSetStartById(104338707, 1);
    kasfLocationCard(8000, m06_saveInfoName[strCurLanguage]);
}
