/*=============================================================================
    Name    : mission11.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission11
=============================================================================*/

#include "MissionScriptSupport.h"

void Init_Mission11_MissionFlow(void);
void Watch_Mission11_MissionFlow_WaitForIntelEventEnd(void);
void Watch_Mission11_MissionFlow_Delay(void);
void Watch_Mission11_MissionFlow(void);
void Init_Mission11_HoldAttackWaves_SetPassive(void);
void Watch_Mission11_HoldAttackWaves(void);
void Watch_Mission11_HoldAttackWaves_AttackMothership(void);
void Init_Mission11_CarrierReinforcementLoop(void);
void Watch_Mission11_CarrierReinforcementController(void);
void Init_Mission11_CarrierReinforcementDock(void);
void Watch_Mission11_CarrierReinforcementDock(void);
void Watch_Mission11_CarrierReinforcementLoop(void);
void Init_Mission11_CarrierLaunchAcolyteWave(void);
void Watch_Mission11_CarrierLaunchAcolyteWave(void);
void Init_Mission11_CarrierLaunchFrigateWaveDelay(void);
void Watch_Mission11_CarrierLaunchFrigateWaveDelay(void);
void Init_Mission11_CarrierLaunchRandomFrigateWave(void);
void Watch_Mission11_CarrierLaunchRandomFrigateWave(void);
void Watch_Mission11_CollectorDocking(void);
void Watch_Mission11_CarrierLaunchAcolyteWave_Reload(void);
void Begin_Mission12_SixShipFormationTeam_AttackMothership(void);
void Watch_Mission11_CueCruiseMissileLaunchers(void);
void Init_Mission11_CruiseMissileLauncher(void);
void Watch_Mission11_CruiseMissileLauncher(void);
void Init_Mission11_CruiseMissileLauncher_SelectTarget(void);
void Watch_Mission11_CruiseMissileLauncher_SelectTarget(void);
void Init_Mission11_CruiseMissileLauncher_ReloadDelay(void);
void Watch_Mission11_CruiseMissileLauncher_ReloadDelay(void);
void Init_Mission11_CruiseMissileLauncher_FireVolley(void);
void Watch_Mission11_CruiseMissileLauncher_FireVolley(void);
void Init_Mission11_SentinelReinforcementTeam(void);
void Init_Mission11_SentinelReinforcementTeam_Reevaluate(void);
void Watch_Mission11_SentinelReinforcementTeam(void);
void Init_Mission11_SentinelReinforcementQueue(void);
void Init_Mission11_SentinelReinforcementBuildTimer(void);
void Watch_Mission11_SentinelReinforcementBuildTimer(void);
void Init_Mission11_SentinelAssault(void);
void Watch_Mission11_SentinelAssault(void);
void Init_Mission11_SentinelAssaultReengage(void);
void Init_Mission11_HiveFrigateReinforcementTeam(void);
void Init_Mission11_HiveFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission11_HiveFrigateReinforcementTeam(void);
void Init_Mission11_HiveFrigateReinforcementBuildTimer(void);
void Watch_Mission11_HiveFrigateReinforcementBuildTimer(void);
void Watch_Mission11_HiveFrigateReinforcementCooldown(void);
void Init_Mission11_WorkerReinforcementTeam(void);
void Init_Mission11_WorkerReinforcementTeam_Reevaluate(void);
void Watch_Mission11_WorkerReinforcementTeam(void);
void Init_Mission11_WorkerReinforcementBuildTimer(void);
void Watch_Mission11_WorkerReinforcementBuildTimer(void);
void Watch_Mission11_Traders(void);
void Watch_Mission11_ReinforcementTriggerRouter(void);
void Init_Mission11_SixShipFormationTeam(void);
void Init_Mission11_HoldPositionThenAttackMothership(void);
void Init_Mission11_PocketAttackWave(void);
void Watch_Mission11_PocketAttackWave_SelectTargets(void);
void Watch_Mission11_PocketAttackWave_UpdateTargets(void);
void Init_Mission11_CarrierFocusCleanup(void);
void Init_Mission11_CarrierFocusClear(void);
void Init_Mission11_HiddenAttackRun_Start(void);
void Watch_Mission11_HiddenAttackRun_Approach(void);
void Init_Mission11_HiddenAttackRun_FocusTarget(void);
void Watch_Mission11_HiddenAttackRun_AbortIfTooClose(void);
void Init_Mission11_HiddenAttackRun_AttachToTarget(void);
void Watch_Mission11_HiddenAttackRun_CloseIn(void);
void Init_Mission11_HiddenAttackRun_DeleteTimer(void);
void Watch_Mission11_HiddenAttackRun_DeleteIfTimerExpired(void);
void Watch_Mission11_SixShipFormationTeam(void);
void Watch_Mission11_TraderTargetLoop(void);

void Begin_HiveFrigateReinforcementBuildTimer(void);
void Begin_HiveFrigateCombatLoop(void);
void Begin_SentinelCombatRetryDelay(void);
void Watch_BeastWorkerHarvestLoop(void);
void kasSpeechEventStopTeam(void);

void Watch_BeastDFGFrigateCombatLoop(void);
void Begin_Mission17_BeastDFGFrigateCombatLoop(void);
void Begin_BeastReinforcementTeamDockAndReinforce(void);
void Watch_BeastDFGFrigateDockAndReinforce(void);

void jpg_615AA0(void);
void Init_BeastDfgFrigateReinforcementTeam(void);
void Init_BeastDfgFrigateReinforcementTeam_Reevaluate(void);

void Watch_Mission11(void)
{
  int v0; // eax
  int v1; // eax
  int *v2; // eax
  int *v3; // eax
  int v4; // eax
  int *v6; // eax
  int *v7; // eax
  int v8; // eax
  int *v9; // eax
  int v10; // eax
  int v11; // eax
  int v12; // [esp-8h] [ebp-8h]
  int v13; // [esp-8h] [ebp-8h]
  int v14; // [esp-8h] [ebp-8h]
  int v15; // [esp-4h] [ebp-4h]
  int v16; // [esp-4h] [ebp-4h]
  int v17; // [esp-4h] [ebp-4h]
  int v18; // [esp-4h] [ebp-4h]
  int v19; // [esp-4h] [ebp-4h]
  int v20; // [esp-4h] [ebp-4h]
  int v21; // [esp-4h] [ebp-4h]
  int *v22; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503617;
  v15 = kasGrowSelectionPtr(18336307);
  v0 = kasVolumePtr(132933767);
  kasfFindShipsInside(v0, v15);
  v16 = kasGrowSelectionPtr(132678291);
  v1 = kasGrowSelectionPtr(18336307);
  kasfShipsRemove(v1, v16);
  v17 = kasGrowSelectionPtr(18336307);
  v2 = (int *)kasGrowSelectionPtr(114961833);
  kasSelectEnemyShips(v2, v17);
  v18 = kasGrowSelectionPtr(18336307);
  v3 = (int *)kasGrowSelectionPtr(0x4A2A29);
  kasSelectPlayerShips(v3, v18);
  if ( !kasfEntryValueGet(71552166) )
  {
    v12 = kasGrowSelectionPtr(0x4A2A29);
    v4 = kasGrowSelectionPtr(211966048);
    if ( kasfShipsSelectType(v4, v12, "sMothership") )
      kasfEntrySetValue(71552166, 1);
  }
  if ( !kasfEntryValueGet(71552167) )
  {
    if ( kasfEntryValueGet(-2015429404) )
    {
      kasfEntrySetValue(71552167, 1);
      v19 = kasGrowSelectionPtr(18336307);
      v6 = (int *)kasGrowSelectionPtr(89157026);
      kasSelectPlayerShipsSimple(v6, v19);
      v20 = kasGrowSelectionPtr(89157026);
      v7 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsSelectNotSalvagers(v7, v20);
      v21 = kasGrowSelectionPtr(218060896);
      v8 = kasGrowSelectionPtr(82017669);
      if ( kasfShipsSelectLinkedShips(v8, v21) )
      {
        v22 = (int *)kasGrowSelectionPtr(82017669);
        v9 = (int *)kasGrowSelectionPtr(89157026);
        kasfShipsAdd(v9, v22);
      }
      if ( GetGameDifficulty() < 2 )
      {
        if ( kasfRUsGet(0) < 500 )
        {
          v13 = kasGrowSelectionPtr(89157026);
          v10 = kasGrowSelectionPtr(96759714);
          if ( !kasfShipsSelectType(v10, v13, "sWorker") )
          {
            v14 = kasGrowSelectionPtr(218060896);
            v11 = kasGrowSelectionPtr(154639956);
            kasfShipsCreateInside(v11, "sWorker", 1, v14, 0);
          }
        }
      }
    }
  }
}

void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(-2054310428, 0);
}

void Init_Mission11_MissionFlow_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerSetById(144337492, kasfEntryValueGet(127756738));
    kasTimerStartById(144337492);
    kasfEntrySetValue(127756738, 0);
}

void Init_Mission11_ObjectiveFlow(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(
        139967671,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission11_ObjectiveFlow_DestroyAttackers);
}

void Init_Mission11_ObjectiveFlow_RepairFactorum(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfObjectiveCreate(
        "RepairFactorum",
        ((char **)0x8CBBDC)[strCurLanguage],
        ((char **)0x8CBBE8)[strCurLanguage]);
    kasfObjectiveSet("RepairFactorum", 0);
}

void Watch_Mission11_ObjectiveFlow_DestroyAttackers(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552162) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)0xA636424)))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfObjectiveDestroy("DestroyAttackers");
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)44440798);
        kasTimerCreateSetStartById(0x49D71E, 10);
    }
    if (kasfMsgReceived((char *)256926835))
    {
        kasfObjectiveCreate(
            "DestroyAttackers",
            ((char **)0x8CBBC4)[strCurLanguage],
            ((char **)0x8CBBD0)[strCurLanguage]);
        kasfObjectiveSet("DestroyAttackers", 0);
        kasfEntryGetOrCreateSetValue(290904, 1);
    }
    else if (kasTimerExpiredById(0x49D71E))
    {
        kasfMsgSendAll((char *)48864707);
        kasTimerCreateSetStartById(165518594, 60);
    }
    if (kasTimerExpiredById(165518594))
    {
        kasfSpeechEvent(
            170775959,
            (sdword)Init_Mission11_ObjectiveFlow_RepairFactorum,
            (sdword)Watch_Mission11_ObjectiveFlow_RepairFactorum);
    }
}

void Watch_Mission11_ObjectiveFlow_RepairFactorum(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)242313828))
    {
        kasfObjectiveDestroy("RepairFactorum");
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)209531860);
    }
    if (kasfMsgReceived((char *)254123566))
    {
        kasfSpeechEvent(
            20938967,
            (sdword)Init_Mission11_ObjectiveFlow_UpdateBeastObjectives,
            (sdword)Watch_Mission11_ObjectiveFlow_DestroyBeast);
    }
}

void Init_Mission11_ObjectiveFlow_UpdateBeastObjectives(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfObjectiveCreate(
        "DestroyBeast",
        ((char **)0x8CBBF4)[strCurLanguage],
        ((char **)0x8CBC00)[strCurLanguage]);
    kasfObjectiveSet("DestroyBeast", 0);
    kasfObjectiveCreate(
        "ProtectFactorum",
        ((char **)0x8CBC0C)[strCurLanguage],
        ((char **)0x8CBC18)[strCurLanguage]);
    kasfObjectiveSet("ProtectFactorum", 0);
}

void Watch_Mission11_ObjectiveFlow_DestroyBeast(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552163) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)114961833)))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfObjectiveDestroyAll();
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0xC1CF74);
    }
    if (kasfMsgReceived((char *)96744581))
    {
        kasfObjectiveCreate(
            "Hyperspace",
            ((char **)0x8CBC24)[strCurLanguage],
            ((char **)0x8CBC30)[strCurLanguage]);
        kasfObjectiveSet("Hyperspace", 0);
        spSetSinglePlayerSaveInfo(11, ((char **)0x8CBBB8)[strCurLanguage]);
    }
}

void Watch_Mission11_ObjectiveFlow_PrimeDestroyAttackers(void)
{
    GrowSelection *trackedSelection;

    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (!kasfEntryValueGet(71552161) &&
        !kasfShipsCount(kasAITeamShipsPtrIfExists(92881725)))
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x0D2247A4);
    }
    trackedSelection = kasGrowSelectionPtr((char *)0x0CA25860);
    if (!kasfShipsCount(trackedSelection) && kasfEntryValueGet(290904))
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x0D2247A4);
    }
}

void Init_Mission11_MissionFlow(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(144337492);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfEntryGetOrCreateSetValue(126773838, 1);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)5801588);
    kasfMusicPlay(22);
    kasfSpeechEvent(
        32713509,
        (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
        (sdword)Watch_Mission11_MissionFlow);
}

void Watch_Mission11_MissionFlow_WaitForIntelEventEnd(void)
{
    if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
    {
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
        if (kasfEntryValueGet(127756738))
        {
            kasfSpeechEvent(
                244882162,
                (sdword)Init_Mission11_MissionFlow_Delay,
                (sdword)Watch_Mission11_MissionFlow_Delay);
        }
        else
        {
            kasfSpeechEvent(
                32713509,
                (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
                (sdword)Watch_Mission11_MissionFlow);
        }
    }
}

void Watch_Mission11_MissionFlow_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)220350372))
    {
        kasfSpeechEvent(
            304604,
            (sdword)FailMissionIfTrackedSelectionAlive,
            (sdword)MaintainTeamContext);
    }
    else if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission11_MissionFlow);
    }
}

void Watch_Mission11_MissionFlow(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueInfo != NULL) ? *queueInfo : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        switch (message)
        {
            case 0:
                break;
            case 0x00588674:
                kasfMsgSend(team, (char *)92825426);
                spSetSinglePlayerSaveInfo(11, ((char **)0x8CBBAC)[strCurLanguage]);
                break;
            case 98985824:
                kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(211966048));
                kasfCameraSetAngleDeg(25);
                kasfCameraSetDeclinationDeg(-30);
                break;
            case 14227924:
                kasfSpeechEventQueueCurrentPlayer(268468738, 0);
                kasfMsgSend(team, (char *)175977140);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 14570085:
                kasfTutCameraFocusClose(kasGrowSelectionPtr(211966048));
                kasfSpeechEventQueueCurrentPlayer(268440065, 0);
                kasfMsgSend(team, (char *)14227924);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 44440798:
                spEventWideScreenIn(90);
                kasfPingRemove("PingOnFaalCorum");
                kasfMsgSend(team, (char *)174181842);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission11_MissionFlow_Delay);
                return;
            case 48864707:
                kasfSpeechEventQueueCurrentPlayer(268468729, 0);
                kasfMsgSend(team, (char *)124149327);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 61053125:
                kasfPingAddSingleShip(kasAITeamShipsPtrIfExists(92881725), "PingOnFaalCorum");
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)256926835);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 71890693:
                kasfMsgSendAll((char *)254123566);
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMusicPlay(22);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission11_MissionFlow_Delay);
                return;
            case 83293076:
                kasfSpeechEventQueueCurrentPlayer(268444150, 0);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 92825426:
                kasfEntrySetValue(-2054310428, 0);
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(92881725));
                kasfCameraSetAngleDeg(40);
                kasfCameraSetDeclinationDeg(-40);
                kasfSpeechEventQueueCurrentPlayer(268468733, 0);
                kasfMsgSend(team, (char *)122409547);
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 103435781:
                kasfSpeechEventQueueCurrentPlayer(268468740, 0);
                kasfMsgSendAll((char *)250810117);
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(92881725));
                kasfCameraSetAngleDeg(272);
                kasfCameraSetDeclinationDeg(-4);
                kasfCameraSetDistance(2500);
                kasfEntrySetValue(127756738, 11);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission11_MissionFlow_Delay);
                return;
            case 122409547:
                kasfOpenSensors(100000);
                kasfEntrySetValue(126773838, 0);
                kasfUniversePause();
                kasfOtherKASPause();
                kasfSensorsSetLookatPoint(kasTeamsVectorPtr(92881725));
                kasfSpeechEventQueueCurrentPlayer(268440062, 0);
                kasfMsgSend(team, (char *)226631033);
                kasfUniversePause();
                kasfOtherKASPause();
                kasfTutSetPointerTargetFirstShip("PointerToFaalCorum", kasAITeamShipsPtrIfExists(92881725));
                kasfEntrySetValue(168240451, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 124149327:
                kasfSpeechEventQueueCurrentPlayer(268440058, 0);
                kasfMusicPlay(10);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(266064926, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 174181842:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(92881725));
                kasfSpeechEventQueueCurrentPlayer(268440055, 0);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 175977140:
                kasfSpeechEventQueueCurrentPlayer(268440067, 0);
                kasfMsgSend(team, (char *)103435781);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 203224914:
                kasfSpeechEventQueueCurrentPlayer(268440064, 0);
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(94015122));
                kasfCameraSetDeclinationDeg(10);
                kasfMsgSend(team, (char *)186296499);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 209531860:
                kasfSpeechEventQueueCurrentPlayer(268444155, 0);
                kasfEntrySetValue(127756738, 15);
                kasfMsgSend(team, (char *)71890693);
                kasfAllowPlayerToResearch("TECH_REPAIRBOTS");
                kasfAllowPlayerToResearch("TECH_DRIVE_DESTROYER");
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 220350372:
                kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
                return;
            case 226631033:
                kasfSpeechEventQueueCurrentPlayer(268444159, 0);
                kasfMsgSend(team, (char *)40431367);
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(172907219, 1);
                kasfMsgSend(team, (char *)61053125);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 250810117:
                kasfTutCameraFocusCancel();
                kasfWideScreenOut(90);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)96744581);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 254123566:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(51250466));
                kasfCameraSetAngleDeg(25);
                kasfCameraSetDeclinationDeg(25);
                kasfSpeechEventQueueCurrentPlayer(268444156, 0);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(266064926, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission11_MissionFlow_WaitForIntelEventEnd);
                return;
            case 12701556:
                kasfMsgSendAll((char *)76228427);
                kasfMusicPlay(10);
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(kasAITeamPtrIfExists(94015122), (char *)186296499);
                kasfMsgSend(team, (char *)203224914);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission11_MissionFlow_Delay);
                return;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission11_HoldAttackWaves(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasAITeamShipsPtr((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    kasfTeamSetPercentDamaged(50);
    kasfTeamSwitchPlayerOwner(0);
    kasfSpeechEvent(170532323, (sdword)Init_Mission11_HoldAttackWaves_SetPassive, (sdword)Watch_Mission11_HoldAttackWaves);
}

void Init_Mission11_HoldAttackWaves_SetPassive(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(19800034), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(148883458), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143436035), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143436033), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143461796), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(157843747), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(157843748), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143253431), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(156246103), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(214097495), 1);
    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(135378865), 1);
    kasfShipsSetDontApplyPhysicsFlag((GrowSelection *)(kasCurrentTeamPtr + 28), 1);
    kasfEntryGetOrCreateSetValue(237491365, 0);
    if (GetGameDifficulty() >= 2)
    {
        kasTimerCreateSetStartById(182284130, 60);
    }
}

void Watch_Mission11_HoldAttackWaves(void)
{
    GrowSelection *nearbyAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    nearbyAttackers = kasGrowSelectionPtr(69968660);
    kasfFindEnemiesNearCurrentTeam(nearbyAttackers, 5000);
    kasfShipsRemove(nearbyAttackers, kasGrowSelectionPtr(143461796));
    if (!kasfEntryValueGet(71552164) && kasfShipsCount(nearbyAttackers) > 0)
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)83293076);
        kasfEntrySetValue(237491365, 1);
    }
    if (kasTimerExpiredDestroyById(182284130) || kasfEntryValueGet(237491365))
    {
        kasTimerDestroyById(182284130);
        kasfEntryScopeName(237491365);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(19800034), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(148883458), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143436035), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143436033), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143461796), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(157843747), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(157843748), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(143253431), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(156246103), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(214097495), 0);
        kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(135378865), 0);
        kasfShipsSetDontApplyPhysicsFlag((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    }
    if (kasfMsgReceived((char *)48864707))
    {
        kasfSpeechEvent(191734483, (sdword)MaintainTeamContext, (sdword)Watch_Mission11_HoldAttackWaves_AttackMothership);
    }
}

void Watch_Mission11_HoldAttackWaves_AttackMothership(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)250810117))
    {
        kasfAttackMothership();
    }
    else if (!kasfEntryValueGet(71552165) && kasfTeamHealthAverage() >= 65)
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)242313828);
    }
}

void Init_Mission11_CarrierReinforcementController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasAITeamShipsPtr((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    kasfTeamSwitchPlayerOwner(0);
    kasfSpeechEvent(170532323, (sdword)MaintainTeamContext, (sdword)Watch_Mission11_CarrierReinforcementController);
}

void Watch_Mission11_CarrierReinforcementController(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)48864707))
    {
        kasfSpeechEvent(
            191734483,
            (sdword)Init_Mission11_CarrierReinforcementDock,
            (sdword)Watch_Mission11_CarrierReinforcementDock);
    }
    else
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(0x966A0A4));
        kasfFindShipsInside(kasVolumePtr(0x5886231), kasGrowSelectionPtr(0x966A0A4));
        kasfShipsRemove(kasGrowSelectionPtr(0x966A0A4), kasGrowSelectionPtr(0x88D0DA4));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x966A0A4));
    }
}

void Init_Mission11_CarrierReinforcementDock(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfStop();
    kasfDockSupportWith(kasAITeamPtrIfExists(92881725));
}

void Watch_Mission11_CarrierReinforcementDock(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)254123566))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(143461796)))
        {
            kasfSpeechEvent(
                67787395,
                (sdword)Init_Mission11_CarrierLaunchAcolyteWave,
                (sdword)Watch_Mission11_CarrierLaunchAcolyteWave);
        }
        else
        {
            kasfSpeechEvent(
                20938967,
                (sdword)Init_Mission11_CarrierReinforcementLoop,
                (sdword)Watch_Mission11_CarrierReinforcementLoop);
        }
    }
}

void Init_Mission11_CarrierReinforcementLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfFormationDelta3D();
}

void Watch_Mission11_CarrierReinforcementLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)76228427))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(0x58A20));
        kasfShipsAdd(kasGrowSelectionPtr(0x58A20), (GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsSelectCapital(kasGrowSelectionPtr(0x58A212), (GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsRemove(kasGrowSelectionPtr(0x58A20), kasGrowSelectionPtr(0x58A212));
        kasfShipsDockInstant(kasGrowSelectionPtr(0x58A20), kasAITeamShipsPtrIfExists(92881725));
    }
    else if (kasfMsgReceived((char *)250810117)
          && kasfShipsSelectCapital(kasGrowSelectionPtr(0x764E81C), (GrowSelection *)(kasCurrentTeamPtr + 28)))
    {
        kasfShipsHyperspaceIn(kasGrowSelectionPtr(0x764E81C), NULL);
    }
    else if (kasfShipsCount((GrowSelection *)(kasCurrentTeamPtr + 28))
          || !kasfThisTeamIs(kasAITeamPtrIfExists(148883458)))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x6DA2DA9));
    }
    else
    {
        kasfSpeechEvent(
            100141459,
            (sdword)Init_Mission11_CarrierLaunchFrigateWaveDelay,
            (sdword)Watch_Mission11_CarrierLaunchFrigateWaveDelay);
    }
}

void Init_Mission11_CarrierLaunchAcolyteWave(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfStop();
    kasfGrowSelectionClear(kasGrowSelectionPtr(21159));
    kasfShipsCreateInside(kasGrowSelectionPtr(21159), "sAcolyte", 8, kasAITeamShipsPtrIfExists(92881725), 0);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(21159));
    kasfShipsSetDontApplyPhysicsFlag((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
}

void Watch_Mission11_CarrierLaunchAcolyteWave(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            157715175,
            (sdword)Watch_Mission11_CollectorDocking,
            (sdword)Watch_Mission11_CarrierLaunchAcolyteWave_Reload);
    }
}

void Init_Mission11_CarrierLaunchFrigateWaveDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasTimerCreateSetStartById(148949186, kasfRandom(90, 120));
}

void Watch_Mission11_CarrierLaunchFrigateWaveDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(
            5105043,
            (sdword)Init_Mission11_CarrierLaunchRandomFrigateWave,
            (sdword)Watch_Mission11_CarrierLaunchRandomFrigateWave);
    }
}

void Init_Mission11_CarrierLaunchRandomFrigateWave(void)
{
    sdword count;
    char *shipType;

    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfStop();
    kasfGrowSelectionClear(kasGrowSelectionPtr(21159));

    switch (kasfRandom(1, 3))
    {
        case 1:
            shipType = "sMultiBeamFrigate";
            count = 2;
            break;
        case 2:
            shipType = "sHiveFrigate";
            count = 1;
            break;
        case 3:
            shipType = "sRammingFrigate";
            count = 2;
            break;
        default:
            shipType = NULL;
            count = 0;
            break;
    }

    if (shipType != NULL)
    {
        kasfShipsCreateInside(kasGrowSelectionPtr(21159), shipType, count, kasAITeamShipsPtrIfExists(92881725), 0);
    }
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(21159));
    kasfShipsSetDontApplyPhysicsFlag((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
}

void Watch_Mission11_CarrierLaunchRandomFrigateWave(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            20938967,
            (sdword)Init_Mission11_CarrierReinforcementLoop,
            (sdword)Watch_Mission11_CarrierReinforcementLoop);
    }
}

void Watch_Mission11_CollectorDocking(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfFormationDelta();
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(114961833), kasTeamsVectorPtr(92881725), 20000))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
    }
}

void Watch_Mission11_CarrierLaunchAcolyteWave_Reload(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)76228427))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(0x58A20));
        kasfShipsAdd(kasGrowSelectionPtr(0x58A20), (GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsSelectCapital(kasGrowSelectionPtr(0x58A212), (GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsRemove(kasGrowSelectionPtr(0x58A20), kasGrowSelectionPtr(0x58A212));
        kasfShipsDockInstant(kasGrowSelectionPtr(0x58A20), kasAITeamShipsPtrIfExists(92881725));
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            67787395,
            (sdword)Init_Mission11_CarrierLaunchAcolyteWave,
            (sdword)Watch_Mission11_CarrierLaunchAcolyteWave);
    }
    else if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28))
          && kasfShipsSelectNearby(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x6DA2DA9), kasTeamsVectorPtr(92881725), 20000))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
    }
}

void Init_Mission11_CruiseMissileLauncher(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfSpeechEvent(
        320211,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission11_CruiseMissileLauncher);
}

void Watch_Mission11_CruiseMissileLauncher(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)141560536))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission11_CruiseMissileLauncher_FireVolley,
            (sdword)Watch_Mission11_CruiseMissileLauncher_FireVolley);
    }
}

void Init_Mission11_CruiseMissileLauncher_SelectTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSelectCapital(kasGrowSelectionPtr(0x764E81C), kasGrowSelectionPtr(0x4A2A29));
    kasfShipsRemove(kasGrowSelectionPtr(0x764E81C), kasAITeamShipsPtrIfExists(0x589433D));
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x764E81C), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x764E81C));
}

void Watch_Mission11_CruiseMissileLauncher_SelectTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            142812005,
            (sdword)Init_Mission11_CruiseMissileLauncher_ReloadDelay,
            (sdword)Watch_Mission11_CruiseMissileLauncher_ReloadDelay);
    }
}

void Init_Mission11_CruiseMissileLauncher_ReloadDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(148949186, kasfRandom(60, 120));
}

void Watch_Mission11_CruiseMissileLauncher_ReloadDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission11_CruiseMissileLauncher_FireVolley,
            (sdword)Watch_Mission11_CruiseMissileLauncher_FireVolley);
    }
}

void Init_Mission11_CruiseMissileLauncher_FireVolley(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x52A7));
    kasfShipsCreateInside(
        kasGrowSelectionPtr(0x52A7),
        "bCruiseMissile",
        kasfRandom(1, 3),
        kasAITeamShipsPtrIfExists(51250466),
        1);
    kasAITeamShipsPtr(kasGrowSelectionPtr(0x52A7), 1);
    kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(0x52A7));
}

void Watch_Mission11_CruiseMissileLauncher_FireVolley(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Init_Mission11_CruiseMissileLauncher_SelectTarget,
            (sdword)Watch_Mission11_CruiseMissileLauncher_SelectTarget);
    }
}

void Init_Mission11_SentinelReinforcementTeam(void)
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
            kasfSpeechEvent(
                76130852,
                (sdword)Init_Mission11_SentinelAssault,
                (sdword)Watch_Mission11_SentinelAssault);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission11_SentinelReinforcementTeam);
        }
    }
}

void Init_Mission11_SentinelReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(171501548))
     && kasfEntryValueGet(-2031143291)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2031143291, 0);
        kasfEntrySetValue(-1884903716, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)190169075);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission11_SentinelReinforcementTeam);
    }
}

void Watch_Mission11_SentinelReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedSentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedSentinels = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(returnedSentinels, kasGrowSelectionPtr(44665972), "bSentinel") < 3)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
        {
            if (kasfEntryValueGet(-1884903716))
            {
                kasfEntrySetValue(-1884903716, 0);
                kasfEntrySetValue(-2031143291, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_Mission11_SentinelReinforcementQueue);
            }
        }
        else
        {
            kasfEntrySetValue(-1884903716, 0);
            kasfEntrySetValue(-2031143291, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
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
    kasfSpeechEvent(
        76130852,
        (sdword)Init_Mission11_SentinelAssault,
        (sdword)Watch_Mission11_SentinelAssault);
}

void Init_Mission11_SentinelReinforcementQueue(void)
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
            if (kasfEntryValueGet(154639956) < 12)
            {
                kasfEntryIncrement(154639956);
            }
checkBuildReady:
            if (kasfEntryValueGet(154639956) != 12)
            {
                return;
            }
            goto beginBuildTimer;
        }

        if (kasfEntryValueGet(154639956) < 12)
        {
            buildCost = kasfShipTypeBuildCost("bSentinel") / 2;
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if (kasfEntryValueGet(154639956) <= 3)
        {
            goto checkBuildReady;
        }

        buildCost = kasfShipTypeBuildCost("bSentinel") / 2;
        if (kasfRUsGet(1) >= buildCost)
        {
            goto checkBuildReady;
        }
beginBuildTimer:
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
            (sdword)Watch_Mission11_SentinelReinforcementBuildTimer);
        return;
    }

    if (kasfEntryValueGet(154639956) < 12)
    {
        buildCost = kasfShipTypeBuildCost("bSentinel");
        if (kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if (kasfEntryValueGet(154639956) > 3)
    {
        buildCost = kasfShipTypeBuildCost("bSentinel");
        if (kasfRUsGet(1) < buildCost)
        {
            goto beginBuildTimer;
        }
    }
    if (kasfEntryValueGet(154639956) == 12)
    {
        goto beginBuildTimer;
    }
}

void Init_Mission11_SentinelReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCount;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildCount = kasfEntryValueGet(154639956);
    buildScale = kasfEntryValueGet(-2126662828);
    switch (buildScale)
    {
        case 0:
            kasTimerCreateSetStartById(235522949, buildCount * kasfShipTypeBuildTime("bSentinel"));
            break;
        case 1:
            kasTimerCreateSetStartById(235522949, buildCount * kasfShipTypeBuildTime("bSentinel") / 2);
            break;
        case 2:
            kasTimerCreateSetStartById(235522949, 0);
            break;
    }
}

void Watch_Mission11_SentinelReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *spawnParents;
    GrowSelection *spawnTarget;
    GrowSelection *createdSentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(spawnParents))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnTarget = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnTarget, spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
        createdSentinels = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(createdSentinels, "bSentinel", kasfEntryValueGet(154639956), spawnTarget, 1);
        kasAITeamShipsPtr(createdSentinels, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdSentinels);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            76130852,
            (sdword)Init_Mission11_SentinelAssault,
            (sdword)Watch_Mission11_SentinelAssault);
    }
}

void Init_Mission11_SentinelAssault(void)
{
    GrowSelection *playerCapitals;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    playerCapitals = kasGrowSelectionPtr(124053532);
    kasfShipsSelectCapital(playerCapitals, kasGrowSelectionPtr(0x4A2A29));
    kasfShipsSelectRace(playerCapitals, playerCapitals, 1);
    if (kasfShipsSelectType(kasGrowSelectionPtr(171484652), kasGrowSelectionPtr(0x4A2A29), "bSentinel")
     && kasfShipsSelectTargets(kasGrowSelectionPtr(171484652), kasGrowSelectionPtr(157715175)))
    {
        kasfShipsRemove(playerCapitals, kasGrowSelectionPtr(157715175));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(151690773), kasGrowSelectionPtr(0x4A2A29), "bIonArrayFrigate"))
    {
        kasfShipsRemove(playerCapitals, kasGrowSelectionPtr(151690773));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(223105365), kasGrowSelectionPtr(0x4A2A29), "bRammingFrigate"))
    {
        kasfShipsRemove(playerCapitals, kasGrowSelectionPtr(223105365));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(191446069), kasGrowSelectionPtr(0x4A2A29), "bDfgFrigate"))
    {
        kasfShipsRemove(playerCapitals, kasGrowSelectionPtr(191446069));
    }
    if (!kasfShipsCount(playerCapitals) && !kasfEntryValueGet(84320888))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Begin_SentinelCombatRetryDelay,
            (sdword)Init_Mission11_SentinelAssaultReengage);
        return;
    }
    kasfTacticsNeutral();
    kasfFormationWall();
    kasfShipsSelectNearby(kasGrowSelectionPtr(86599289), playerCapitals, kasThisTeamsVectorPtr(), 20000);
    kasfShipsSelectType(kasGrowSelectionPtr(212896882), kasGrowSelectionPtr(86599289), "bHeavyCruiser");
    kasfShipsSelectType(kasGrowSelectionPtr(124218850), kasGrowSelectionPtr(86599289), "bCarrier");
    kasfShipsSelectType(kasGrowSelectionPtr(216029280), kasGrowSelectionPtr(86599289), "bMothership");
    if (kasfShipsCount(kasGrowSelectionPtr(212896882))
     || kasfShipsCount(kasGrowSelectionPtr(124218850))
     || kasfShipsCount(kasGrowSelectionPtr(216029280)))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(125446884));
        kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(212896882));
        kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(124218850));
        kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(216029280));
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(125446884), kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), kasGrowSelectionPtr(125446884), 0);
    }
    else
    {
        kasfShipsSortByDistanceToPoint(playerCapitals, kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), playerCapitals, 0);
    }
    kasfProtect(kasGrowSelectionPtr(154965556));
    kasTimerDestroyById(111581908);
    kasTimerDestroyById(49272164);
    kasTimerDestroyById(127823460);
}

void Watch_Mission11_SentinelAssault(void)
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
                kasfSpeechEvent(
                    374244,
                    (sdword)Begin_SentinelCombatRetryDelay,
                    (sdword)Init_Mission11_SentinelAssaultReengage);
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
            kasTimerCreateSetStartById(
                111581908,
                kasfEntryValueGet(111592388) * kasfShipTypeBuildTime("bSentinel"));
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
        spawnParents = kasAITeamShipsPtrIfExists(51250466);
        if (kasfShipsCount(spawnParents))
        {
            kasfRUsSet(1, kasfRUsGet(1) - totalCost);
            spawnTarget = kasGrowSelectionPtr(21541);
            kasfShipsSelectIndex(spawnTarget, spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
            if (kasfTeamCount() + kasfEntryValueGet(111592388) >= 12)
            {
                kasfEntrySetValue(111592388, 12 - kasfTeamCount());
            }
            createdSentinels = kasGrowSelectionPtr(154639956);
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
        kasfShipsSelectTargets(&team->shipList, kasGrowSelectionPtr(157715175));
        kasfProtect(kasGrowSelectionPtr(157715175));
        kasfEntrySetValue(127823460, 0);
    }
}

void Init_Mission11_HiveFrigateReinforcementTeam(void)
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
                (sdword)Watch_Mission11_HiveFrigateReinforcementCooldown);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission11_HiveFrigateReinforcementTeam);
        }
    }
}

void Init_Mission11_HiveFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(186735637))
     && kasfEntryValueGet(-2047426043)
     && kasfMsgReceived((char *)4693508))
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
            (sdword)Watch_Mission11_HiveFrigateReinforcementTeam);
    }
}

void Watch_Mission11_HiveFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedHiveFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedHiveFrigates = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(returnedHiveFrigates, kasGrowSelectionPtr(44665972), "bHiveFrigate") < 1)
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
                    (sdword)Init_Mission11_HiveFrigateReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-1892538891, 0);
            kasfEntrySetValue(-2047426043, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
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
        (sdword)Watch_Mission11_HiveFrigateReinforcementCooldown);
}

void Init_Mission11_HiveFrigateReinforcementBuildTimer(void)
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
            (sdword)Watch_Mission11_HiveFrigateReinforcementBuildTimer);
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

void Watch_Mission11_HiveFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *spawnSource;
    GrowSelection *spawnTarget;
    GrowSelection *createdHiveFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnSource = kasGrowSelectionPtr(0xE05243);
    if (!kasfShipsCount(spawnSource))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnTarget = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnTarget, spawnSource, kasfRandom(0, kasfShipsCount(spawnSource) - 1));
        createdHiveFrigates = kasGrowSelectionPtr(154639956);
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
            (sdword)Watch_Mission11_HiveFrigateReinforcementCooldown);
    }
}

void Watch_Mission11_HiveFrigateReinforcementCooldown(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_HiveFrigateCombatLoop,
            (sdword)Watch_Mission11_HiveFrigateReinforcementCooldown);
    }
}

void Init_Mission11_WorkerReinforcementTeam(void)
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
                (sdword)Watch_BeastWorkerHarvestLoop);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission11_WorkerReinforcementTeam);
        }
    }
}

void Init_Mission11_WorkerReinforcementTeam_Reevaluate(void)
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
            (sdword)Watch_Mission11_WorkerReinforcementTeam);
    }
}

void Watch_Mission11_WorkerReinforcementTeam(void)
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
                    (sdword)Init_Mission11_WorkerReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-1893839148, 0);
            kasfEntrySetValue(-1898467739, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
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
    kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_BeastWorkerHarvestLoop);
}

void Init_Mission11_WorkerReinforcementBuildTimer(void)
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
            (sdword)Watch_Mission11_WorkerReinforcementBuildTimer);
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

void Watch_Mission11_WorkerReinforcementBuildTimer(void)
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
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
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
        kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_BeastWorkerHarvestLoop);
    }
}


void Init_Mission11_SentinelAssaultReengage(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfStop();
        kasfSpeechEvent(
            76130852,
            (sdword)Init_Mission11_SentinelAssault,
            (sdword)Watch_Mission11_SentinelAssault);
    }
}













void Watch_Mission11_Traders(void)
{
   GrowSelection *targets;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   targets = kasGrowSelectionPtr(0x36980EB);
   kasfGrowSelectionClear(targets);

   if (!kasfShipsCount(targets) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsSelectType(targets, kasGrowSelectionPtr(0x4A2A29), "kStandardDestroyer");
   }
   if (!kasfShipsCount(targets) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsSelectType(targets, kasGrowSelectionPtr(0x4A2A29), "kIonCannonFrigate");
   }
   if (!kasfShipsCount(targets) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsAdd(targets, kasGrowSelectionPtr(0x4A2A29));
       kasfShipsRemove(targets, kasGrowSelectionPtr(211966048));
   }
   if (!kasfShipsCount(targets) && GetGameDifficulty() == 0)
   {
       kasfShipsAdd(targets, kasGrowSelectionPtr(0x4A2A29));
   }
   if (!kasfShipsCount(targets))
   {
       kasfShipsAdd(targets, kasGrowSelectionPtr(0xCA25860));
   }

   kasfShipsSortByDistanceToPoint(targets, kasThisTeamsVectorPtr());
   tutDeallocateRootRegion(targets);
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission11_ReinforcementTriggerRouter(void)
{
   AITeam *team;
   sdword *queueInfo;
   sdword queueIndex;
   udword message;
   AITeam *targetTeam;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;

   queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueInfo != NULL) ? *queueInfo : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       targetTeam = NULL;
       switch (message)
       {
           case 35692611:
               targetTeam = kasAITeamPtrIfExists(62813829);
               break;
           case 104569475:
               targetTeam = kasAITeamPtrIfExists(200156578);
               break;
           case 141560536:
               kasfEntrySetValue(-2126662828, 2);
               kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(171501548), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(186735637), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(62813829), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(200156578), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(223105365), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(0x56983E), (char *)4693508);
               kasfMsgSend(kasAITeamPtrIfExists(17798), (char *)4693508);
               kasTimerCreateSetStartById(24667538, 120);
               break;
           case 154145491:
               targetTeam = kasAITeamPtrIfExists(182180066);
               break;
           case 158207907:
               targetTeam = kasAITeamPtrIfExists(0x56983E);
               break;
           case 190169075:
               targetTeam = kasAITeamPtrIfExists(171501548);
               break;
           case 203347251:
               targetTeam = kasAITeamPtrIfExists(223105365);
               break;
           case 204436228:
               targetTeam = kasAITeamPtrIfExists(208120516);
               break;
           case 221638067:
               targetTeam = kasAITeamPtrIfExists(151690773);
               break;
           case 228088883:
               targetTeam = kasAITeamPtrIfExists(17798);
               break;
           case 228101699:
               targetTeam = kasAITeamPtrIfExists(18599);
               break;
           case 236014723:
               targetTeam = kasAITeamPtrIfExists(186735637);
               break;
           case 259199139:
               targetTeam = kasAITeamPtrIfExists(75148549);
               break;
       }
       if (targetTeam != NULL)
       {
           kasfMsgSend(targetTeam, (char *)4693508);
       }

       queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
       {
           break;
       }
   }

   if (kasTimerExpiredById(24667538))
   {
       kasfEntrySetValue(-2126662828, 0);
       if (GetGameDifficulty() < 2)
       {
           kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
       }
   }
}

void Init_Mission11_HoldPositionThenAttackMothership(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfTacticsAggressive();
   kasfShipsSetMaxVelocity(&team->shipList, 0);
   kasfSpeechEvent(
       320211,
       (sdword)Begin_Mission12_SixShipFormationTeam_AttackMothership,
       (sdword)Watch_Mission11_CueCruiseMissileLaunchers);
}

void Begin_Mission12_SixShipFormationTeam_AttackMothership(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfTeamAttributesBitSet(0x80);
   kasfAttackMothership();
}

void Watch_Mission11_CueCruiseMissileLaunchers(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfMsgReceived((char *)0xF259E2E))
   {
       kasfTeamHyperspaceInNear(kasVectorPtrIfExists(0x88C0B71), 3000);
       kasTimerCreateSetStartById(139352562, 15);
   }
   else if (kasTimerExpiredById(139352562))
   {
       kasfMsgSend(kasAITeamPtrIfExists(0x469684), (char *)141560536);
       kasfMsgSend(kasAITeamPtrIfExists(0xE87A7B3), (char *)141560536);
       kasfSpeechEvent(139967671, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
   }
}

void Init_Mission11_PocketAttackWave(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfSpeechEvent(
       139967671,
       (sdword)Watch_Mission11_PocketAttackWave_SelectTargets,
       (sdword)Watch_Mission11_PocketAttackWave_UpdateTargets);
}

void Watch_Mission11_PocketAttackWave_SelectTargets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfTacticsAggressive();
   kasfFindShipsInside(kasVolumePtr(0x5886231), kasGrowSelectionPtr(0x966A0A4));
   Mission02ShipsSelect(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x966A055));
   tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
   kasTimerCreateSetStartById(139352562, 10);
}

void Watch_Mission11_PocketAttackWave_UpdateTargets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfTeamCount() && kasTimerExpiredDestroyById(139352562))
   {
       kasfFindShipsInside(kasVolumePtr(0x5886231), kasGrowSelectionPtr(0x966A0A4));
       Mission02ShipsSelect(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x966A055));
       if (kasfThisTeamIs(kasAITeamPtrIfExists(0x889DFB7)))
       {
           kasfShipsSelectCapital(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x36980EB));
       }
       tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
       kasTimerSetById(139352562, 8);
       kasTimerStartById(139352562);
   }
}

void Init_Mission11_CarrierFocusCleanup(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfSpeechEvent(
       139967671,
       (sdword)Init_Mission11_CarrierFocusClear,
       (sdword)MaintainTeamContext);
}

void Init_Mission11_CarrierFocusClear(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   tutDeallocateRootRegion(kasAITeamShipsPtrIfExists(92881725));
}

void Init_Mission11_HiddenAttackRun_Start(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfShipsSetMaxVelocity(&team->shipList, 570);
   kasfTeamSwitchPlayerOwner(0);
   kasfHideShips(&team->shipList);
   kasAITeamShipsPtr(&team->shipList, 0);
   kasfShipsSetNonRetaliation(&team->shipList);
   kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
   kasfSpeechEvent(
       19042,
       (sdword)Init_Mission11_HiddenAttackRun_FocusTarget,
       (sdword)Watch_Mission11_HiddenAttackRun_AbortIfTooClose);
}

void Watch_Mission11_HiddenAttackRun_Approach(void)
{
   hvector *targetCenter;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfMsgReceived((char *)0xB1AA8B3))
   {
       kasfUnhideShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
       kasfSpeechEvent(
           0x474640,
           (sdword)Init_Mission11_HiddenAttackRun_AttachToTarget,
           (sdword)Watch_Mission11_HiddenAttackRun_CloseIn);
   }
   else
   {
       targetCenter = kasShipsVectorPtrIfExists(0xCA25860);
       if (kasfFindDistance((hvector *)kasShipsVectorPtr((char *)targetCenter), targetCenter) >= 10000)
       {
           kasfSpeechEvent(
               19042,
               (sdword)Init_Mission11_HiddenAttackRun_FocusTarget,
               (sdword)Watch_Mission11_HiddenAttackRun_AbortIfTooClose);
       }
   }
}

void Init_Mission11_HiddenAttackRun_FocusTarget(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfMoveTo(kasShipsVectorPtrIfExists(0xCA25860));
}

void Watch_Mission11_HiddenAttackRun_AbortIfTooClose(void)
{
   hvector *targetCenter;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfMsgReceived((char *)0xB1AA8B3))
   {
       kasfUnhideShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
       kasfSpeechEvent(
           0x474640,
           (sdword)Init_Mission11_HiddenAttackRun_AttachToTarget,
           (sdword)Watch_Mission11_HiddenAttackRun_CloseIn);
   }
   else
   {
       targetCenter = kasShipsVectorPtrIfExists(0xCA25860);
       if (kasfFindDistance((hvector *)kasShipsVectorPtr((char *)targetCenter), targetCenter) < 10000)
       {
           kasfSpeechEvent(
               0x481475,
               (sdword)kasSpeechEventStopTeam,
               (sdword)Watch_Mission11_HiddenAttackRun_Approach);
       }
   }
}

void Init_Mission11_HiddenAttackRun_AttachToTarget(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfShipsDockWithSelection(kasAITeamShipsPtrIfExists(0x59A8E92), kasGrowSelectionPtr(0xCA25860));
}

void Watch_Mission11_HiddenAttackRun_CloseIn(void)
{
   hvector *targetCenter;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(0x3BADC22));
   targetCenter = kasShipsVectorPtrIfExists(0xCA25860);
   if (kasfFindDistance((hvector *)kasShipsVectorPtr((char *)targetCenter), targetCenter) < 1400)
   {
       kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)14570085);
       kasfSpeechEvent(
           342540,
           (sdword)Init_Mission11_HiddenAttackRun_DeleteTimer,
           (sdword)Watch_Mission11_HiddenAttackRun_DeleteIfTimerExpired);
   }
}

void Init_Mission11_HiddenAttackRun_DeleteTimer(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerCreateSetStartById(148930818, 4);
}

void Watch_Mission11_HiddenAttackRun_DeleteIfTimerExpired(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasTimerExpiredById(148930818) && kasfShipsCount(&team->shipList))
   {
       kasfDeleteShips(&team->shipList);
   }
}

void Init_Mission11_SixShipFormationTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfSpeechEvent(
       320211,
       (sdword)Begin_Mission12_SixShipFormationTeam_AttackMothership,
       (sdword)Watch_Mission11_SixShipFormationTeam);
}

void Watch_Mission11_SixShipFormationTeam(void)
{
   AITeam *team;
   AITeam *targetTeam;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfMsgReceived((char *)0xF259E2E))
   {
       kasfTeamHyperspaceInNear(kasVectorPtrIfExists(0x88C0B71), 3000);
       kasTimerCreateSetStartById(139352562, 15);
   }
   else if (kasTimerExpiredById(139352562))
   {
       if (kasfThisTeamIs(kasAITeamPtrIfExists(5793959)))
       {
           targetTeam = kasAITeamPtrIfExists(297601);
       }
       else
       {
           targetTeam = kasAITeamPtrIfExists(0xA9E1F1);
       }
       kasfReinforceTeamWithShips(targetTeam, &team->shipList);
       kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
   }
}

void Watch_Mission11_TraderTargetLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (!kasfEntryValueGet(71552167) && kasfEntryValueGet(-1914453392))
   {
       kasfEntryGetOrCreateSetValue(71552167, 1);
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(
           74024315,
           (sdword)Watch_Mission11_Traders,
           (sdword)Watch_Mission11_TraderTargetLoop);
   }
   else if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
   }
   else if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(
           74024315,
           (sdword)Watch_Mission11_Traders,
           (sdword)Watch_Mission11_TraderTargetLoop);
   }
}

/*-----------------------------------------------------------------------------
    Init_Mission11 - top-level KAS entry point (IDA: 0x61f690)
-----------------------------------------------------------------------------*/

extern void Fill_Mission16_ReinforcementPoolFromParade(void);
extern void Init_BeastWorkerReinforcementTeam(void);
extern void Init_BeastWorkerReinforcementTeam_Reevaluate(void);
extern void Init_BeastIonArrayFrigateReinforcementTeam(void);
extern void Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate(void);
extern void Init_BeastMultiBeamFrigateReinforcementTeam(void);
extern void Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void);
extern void Init_BeastCloakedFighterReinforcementTeam(void);
extern void Init_BeastCloakedFighterReinforcementTeam_Reevaluate(void);
extern void Init_BeastCorvetteReinforcementTeam(void);
extern void Init_BeastCorvetteReinforcementTeam_Reevaluate(void);
extern void Init_BeastAcolyteReinforcementTeam(void);
extern void Init_BeastAcolyteReinforcementTeam_Reevaluate(void);
extern void Init_BeastRammingFrigateReinforcementTeam(void);
extern void Init_BeastRammingFrigateReinforcementTeam_Reevaluate(void);
extern void Init_BeastReconReinforcementTeam(void);
extern void Init_BeastReconReinforcementTeam_Reevaluate(void);

void Init_Mission11(void)
{
    kasCurrentStep = 0;
    scope = KH_mission11;
    kasfDisableMothershipBigGunLaunch();
    kasfActivateTech("TECH_INFECT");
    kasfActivateTech("TECH_FORCE_FIELD1");
    kasfActivateTech("TECH_ENERGY");
    kasfActivateTech("TECH_REPAIR");
    kasfActivateTech("TECH_SALVAGE");
    kasfActivateTech("TECH_CLOAKING");
    kasfActivateTech("TECH_AFTERBURNER");
    kasfActivateTech("TECH_EMP");
    kasfActivateTech("TECH_ADVANCED_SENSORS");
    kasfDisablePlayerHyperspace();
    kasfLocationCard(8000, "System AZ-23769");

    kasFSMCreate(KH_FleetIntel, Init_Mission11_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Init_Mission11_ObjectiveFlow, Watch_Mission11_ObjectiveFlow_PrimeDestroyAttackers,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(92881725, Init_Mission11_HoldAttackWaves, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(92881725));

    kasFSMCreate(168024436, Init_Mission11_CarrierReinforcementController, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(19800034));
    kasFSMCreate(168024436, Init_Mission11_CarrierReinforcementController, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(148883458));
    kasFSMCreate(168024436, Init_Mission11_CarrierReinforcementController, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(143436035));
    kasFSMCreate(168024436, Init_Mission11_CarrierReinforcementController, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(143436033));
    kasFSMCreate(168024436, Init_Mission11_CarrierReinforcementController, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(143461796));

    kasFSMCreate(109618292, Init_Mission11_PocketAttackWave, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(157843747));
    kasFSMCreate(109618292, Init_Mission11_PocketAttackWave, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(157843748));

    kasFSMCreate(16860324, Init_Mission11_CarrierFocusCleanup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(143253431));

    kasFSMCreate(109618292, Init_Mission11_PocketAttackWave, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(156246103));

    kasFSMCreate(44665972, jpg_615AA0, Fill_Mission16_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(214097495));

    kasFSMCreate(16860324, Init_Mission11_CarrierFocusCleanup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(135378865));
    kasFSMCreate(51250466, Init_Mission11_HoldPositionThenAttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(51250466));
    kasFSMCreate(135358963, Init_Mission11_CruiseMissileLauncher, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(243771315));
    kasFSMCreate(94015122, Init_Mission11_HiddenAttackRun_Start, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(94015122));

    kasfGrowSelectionClear(kasGrowSelectionPtr(132678291));
    kasfShipsAdd(kasGrowSelectionPtr(132678291), kasAITeamShipsPtrIfExists(132677969));
    kasfShipsAdd(kasGrowSelectionPtr(132678291), kasAITeamShipsPtrIfExists(132677970));
    kasfShipsAdd(kasGrowSelectionPtr(132678291), kasAITeamShipsPtrIfExists(132677971));

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsRemove(kasGrowSelectionPtr(18336307), kasGrowSelectionPtr(132678291));
    kasfShipsApplyPlayerUpgrades(kasGrowSelectionPtr(18336307));

    kasfShipsSelectMissionVolumeEnemyCombatShips(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeInactivePlayerModules(kasGrowSelectionPtr(KH_Enemy), kasGrowSelectionPtr(18336307));

    kasfGrowSelectionClear(kasGrowSelectionPtr(143461796));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasGrowSelectionPtr(111034899));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasAITeamShipsPtrIfExists(92881725));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasAITeamShipsPtrIfExists(19800034));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasAITeamShipsPtrIfExists(148883458));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasAITeamShipsPtrIfExists(143436035));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasAITeamShipsPtrIfExists(143436033));
    kasfShipsAdd(kasGrowSelectionPtr(143461796), kasAITeamShipsPtrIfExists(94015122));

    kasfGrowSelectionClear(kasGrowSelectionPtr(174285860));
    kasfShipsAdd(kasGrowSelectionPtr(174285860), kasAITeamShipsPtrIfExists(157843747));
    kasfShipsAdd(kasGrowSelectionPtr(174285860), kasAITeamShipsPtrIfExists(157843748));
    kasfShipsAdd(kasGrowSelectionPtr(174285860), kasAITeamShipsPtrIfExists(143253431));
    kasfShipsAdd(kasGrowSelectionPtr(174285860), kasAITeamShipsPtrIfExists(156246103));
    kasfShipsAdd(kasGrowSelectionPtr(174285860), kasAITeamShipsPtrIfExists(214097495));
    kasfShipsAdd(kasGrowSelectionPtr(174285860), kasAITeamShipsPtrIfExists(135378865));

    kasfRUsSet(1, 20000);

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
    kasfAllowPlayerToResearch("TECH_ARMOUR2");
    kasfAllowPlayerToResearch("TECH_EMP");
    kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
    kasfAllowPlayerToResearch("TECH_ENERGY");
    kasfAllowPlayerToResearch("TECH_ION");
    kasfAllowPlayerToResearch("TECH_MISSILE");
    kasfAllowPlayerToResearch("TECH_PHASED_TELEMETRY");
    kasfAllowPlayerToResearch("TECH_ADVANCED_SENSORS");
    kasfAllowPlayerToResearch("TECH_BIG_GUN");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");

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
    kasfModuleSlotOrderSetShipType(1, "bMultiGunCorvette");
    kasfModuleSlotOrderSetShipType(1, "bCruiseMissile");
    kasfModuleSlotOrderSetShipType(1, "bWorker");
    kasfModuleSlotOrderSetShipType(1, "bIonArrayFrigate");
    kasfModuleSlotOrderSetShipType(1, "bRammingFrigate");
    kasfModuleSlotOrderSetCount(1, 4);

    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasFSMCreate(4627076, Watch_NoOpTeamContext, Watch_Mission11_ReinforcementTriggerRouter,
                 kasAITeamPtrIfExists(4627076));

    kasFSMCreate(208120516, Init_Mission11_WorkerReinforcementTeam, Init_Mission11_WorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(208120516));
    kasFSMCreate(208120516, Init_Mission11_WorkerReinforcementTeam, Init_Mission11_WorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(108702865));
    kasFSMCreate(208120516, Init_Mission11_WorkerReinforcementTeam, Init_Mission11_WorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(108702866));

    kasFSMCreate(205978147, Init_BeastWorkerReinforcementTeam, Init_BeastWorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(96759714));
    kasFSMCreate(205978147, Init_BeastWorkerReinforcementTeam, Init_BeastWorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(205978209));
    kasFSMCreate(205978147, Init_BeastWorkerReinforcementTeam, Init_BeastWorkerReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(205978210));

    kasFSMCreate(59670451, Init_Mission11_SentinelReinforcementTeam, Init_Mission11_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(171501548));
    kasFSMCreate(59670451, Init_Mission11_SentinelReinforcementTeam, Init_Mission11_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(59670129));
    kasFSMCreate(59670451, Init_Mission11_SentinelReinforcementTeam, Init_Mission11_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(59670130));

    kasFSMCreate(11133235, Init_BeastIonArrayFrigateReinforcementTeam, Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(151690773));
    kasFSMCreate(11133235, Init_BeastIonArrayFrigateReinforcementTeam, Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(11133425));

    kasFSMCreate(34980115, Init_Mission11_HiveFrigateReinforcementTeam, Init_Mission11_HiveFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(186735637));
    kasFSMCreate(34980115, Init_Mission11_HiveFrigateReinforcementTeam, Init_Mission11_HiveFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(34980305));

    kasFSMCreate(199714963, Init_BeastMultiBeamFrigateReinforcementTeam, Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(62813829));
    kasFSMCreate(199714963, Init_BeastMultiBeamFrigateReinforcementTeam, Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(199714897));

    kasFSMCreate(249715395, Init_BeastCloakedFighterReinforcementTeam, Init_BeastCloakedFighterReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(200156578));
    kasFSMCreate(249715395, Init_BeastCloakedFighterReinforcementTeam, Init_BeastCloakedFighterReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(249715329));

    kasFSMCreate(128635107, Init_BeastCorvetteReinforcementTeam, Init_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(75148549));
    kasFSMCreate(128635107, Init_BeastCorvetteReinforcementTeam, Init_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(128634913));
    kasFSMCreate(128635107, Init_BeastCorvetteReinforcementTeam, Init_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(128634914));

    kasFSMCreate(230526675, Init_BeastAcolyteReinforcementTeam, Init_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(182180066));
    kasFSMCreate(230526675, Init_BeastAcolyteReinforcementTeam, Init_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526609));
    kasFSMCreate(230526675, Init_BeastAcolyteReinforcementTeam, Init_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526610));
    kasFSMCreate(230526675, Init_BeastAcolyteReinforcementTeam, Init_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526611));

    kasFSMCreate(80024947, Init_BeastRammingFrigateReinforcementTeam, Init_BeastRammingFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(223105365));
    kasFSMCreate(80024947, Init_BeastRammingFrigateReinforcementTeam, Init_BeastRammingFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(80025009));

    kasFSMCreate(90801203, Init_BeastReconReinforcementTeam, Init_BeastReconReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(5675070));
    kasFSMCreate(90801203, Init_BeastReconReinforcementTeam, Init_BeastReconReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(90801137));

    kasFSMCreate(297667, Init_BeastDfgFrigateReinforcementTeam, Init_BeastDfgFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(18599));
    kasFSMCreate(297667, Init_BeastDfgFrigateReinforcementTeam, Init_BeastDfgFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(297601));

    kasfShipsAdd(kasGrowSelectionPtr(4627076), kasAITeamShipsPtrIfExists(51250466));

    kasFSMCreate(5990498, Init_Mission11_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(115042305));
    kasFSMCreate(5990498, Init_Mission11_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(92703361));

    kasFSMCreate(44665972, jpg_615AA0, Fill_Mission16_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(44665972));
}

void Watch_BeastDfgFrigateReinforcementTeam(void) {}

void jpg_615B10(void) {}

void jpg_615AA0(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfSpeechEvent(0x469857B, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, (sdword)jpg_615B10);
}

void Init_BeastDfgFrigateReinforcementTeam(void)
{
    AITeam *v0;

    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    v0 = kasAITeamPtrIfExists(18599);
    if (!kasfThisTeamIs(v0))
    {
        kasfEntryGetOrCreateSetValue(0x9379E54, 0);
        kasfEntryGetOrCreateSetValue(0x6A6C3C4, 0);
        kasfEntryGetOrCreateSetValue(0x89E45BD7, 0);
        kasfEntryGetOrCreateSetValue(0x8BD27875, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(0x469684), (char *)0x2AB8A93);
            kasAITeamShipsPtr(&teamtoreinforce->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr(0x469684), &teamtoreinforce->shipList);
            kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
            kasfEntrySetValue(332667, 1);
            kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
        }
        else
        {
            kasfSpeechEvent(0x479E04, (sdword)sub_65F6F0, (sdword)Watch_BeastDfgFrigateReinforcementTeam);
        }
    }
}

void Init_BeastDfgFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *v0;

    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    v0 = kasAITeamPtrIfExists(18599);
    if (kasfThisTeamIs(v0) && kasfEntryValueGet(0x8BD27875) && kasfMsgReceived((char *)0x479E04))
    {
        kasfEntrySetValue(0x8BD27875, 0);
        kasfEntrySetValue(0x89E45BD7, 1);
    }
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            GrowSelection *ships = kasAITeamShipsPtrIfExists(0x30E0522);
            if (kasfShipsCount(ships))
            {
                if (kasfTeamHealthAverage() < 50)
                    kasfSpeechEvent(0x4937B, (sdword)Begin_BeastReinforcementTeamDockAndReinforce, (sdword)Watch_BeastDFGFrigateDockAndReinforce);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(0x469684), (char *)0xD988E43);
            kasfSpeechEvent(0x479E04, (sdword)sub_65F6F0, (sdword)Watch_BeastDfgFrigateReinforcementTeam);
        }
    }
}
