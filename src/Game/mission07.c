/*=============================================================================
    Name    : mission07.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission07
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m07_saveInfoName[] =
{
    "~ End of Mission",
    "French version goes here",
    "German version goes here",
};

static char *m07_convoyLost_subtitle[] =
{
    "The convoy has been lost.  We have failed.",
    "French version goes here",
    "German version goes here",
};

static char *m07_hyperspace_brief[] =
{
    "Hyperspace",
    "French version goes here",
    "German version goes here",
};

static char *m07_hyperspace_full[] =
{
    "Rally the fleet and prepare to hyperspace on your mark.",
    "French version goes here",
    "German version goes here",
};

static char *m07_mayday_brief[] =
{
    "Investigate Mayday Signal",
    "French versino goes here",
    "German version goes here",
};

static char *m07_mayday_full[] =
{
    "Send a RECON to investigate what is attacking the convoy.",
    "French version goes here",
    "German version goes here",
};

static char *m07_protectConvoy_brief[] =
{
    "Protect Convoy from Beast",
    "French version goes here",
    "German version goes here",
};

static char *m07_protectConvoy_full[] =
{
    "Protect convoy from multiple waves of Beast infection missiles.  The best way to do this is to send groups of RECONS and ACOLYTES to the perimeter of the convoy and immediately attack any Beast ships that show up in Sensors Manager.  Move your Command Ship closer to the route of the convoy so that you can deploy re-enforcements from a shorter distance.",
    "French version goes here",
    "German version goes here",
};

static char *m07_destroyInfectedShips_brief[] =
{
    "Destroy Infected Ship(s)",
    "French version goes here",
    "German version goes here",
};

static char *m07_destroyInfectedShips_full[] =
{
    "If a convoy ship gets hit by too many Beast missiles, it will become infected and will begin launching Beast missiles of its own.  Destroy these infected ships as quickly as possible, or the convoy's infection rate will be accelerated.",
    "French version goes here",
    "German version goes here",
};

static char *m07_fleetIntelTriggerMsg = (char *)0x4E396F;

void Watch_Mission07_MissionFlow(void);
void Watch_Mission07_WaitForIntelEventEnd(void);
void Watch_Mission07_MissionFlowResumeDelay(void);
void Watch_Mission07_MissionFlow_AdvanceToggle(void);
void Init_Mission07_ObjectiveEventQueue(void);
void Watch_Mission07_ObjectiveEventQueue(void);
void Watch_Mission07_ConvoyWing_CameraMoveDownTrigger(void);
void Watch_Mission07_ConvoyWing_CameraMoveUpTrigger(void);
void Watch_Mission07_ConvoyWing_Separation(void);
void Watch_Mission07_ConvoyWing_BreakFormation(void);
void Dispatch_Mission07_PrisonShipInfoToHoldingTeam(void);
void Watch_Mission07_DockedLaunchGroup_LaunchComplete(void);
void Init_Mission07_DockedLaunchGroup_SelectInvestigationTarget(void);
void Watch_Mission07_DockedLaunchGroup_InvestigateTarget(void);
void Init_Mission07_BeastCruiseMissileWaveTeam(void);
void Begin_Mission07_BeastCruiseMissileWaveSetup(void);
void Init_Mission07_BeastCruiseMissileWaveDelay(void);
void Watch_Mission07_BeastCruiseMissileWaveDelay(void);
void Init_Mission07_BeastCruiseMissileWaveTimer(void);
void Watch_Mission07_BeastCruiseMissileWaveSpawner(void);
void Watch_Mission07_BeastCruiseMissileWave_ReinforceFromReserve(void);
void Init_Mission07_BeastCruiseMissileWavePingDelay(void);
void Watch_Mission07_BeastCruiseMissileWavePingDelay(void);
void Init_Mission07_BeastCruiseMissileWave4Delay(void);
void Watch_Mission07_BeastCruiseMissileWave4Delay(void);
void Init_Mission07_BeastCruiseMissileWave5Delay(void);
void Watch_Mission07_BeastCruiseMissileWave5Delay(void);
void Init_Mission07_Events_HyperdriveFailDelay(void);
void Watch_Mission07_Events_HyperdriveFail(void);
void Select_Mission07_BeastCruiseMissileWaveTargets(void);
void Watch_Mission07_BeastCruiseMissileWaveTargetLoop(void);
void Init_Mission07_BeastCruiseMissileEscortReserveTeam(void);
void Watch_Mission07_BeastCruiseMissileEscortReserveTeam_AwaitLaunchSignal(void);
void Begin_Mission07_BeastCruiseMissileEscortReserveTeam_Launch(void);
void Watch_Mission07_BeastCruiseMissileEscortReserveTeam(void);
void Select_Mission07_BeastCruiseMissileEscortReserveTargets(void);
void Watch_Mission07_BeastCruiseMissileEscortReserveTargetLoop(void);
void Init_Mission07_BeastCruiseMissileWaveSequence(void);
void Watch_Mission07_BeastCruiseMissileWaveSequence(void);
void Init_Mission07_BeastCruiseMissileWave1Delay(void);
void Watch_Mission07_BeastCruiseMissileWave1Delay(void);
void Init_Mission07_BeastCruiseMissileWave2Delay(void);
void Watch_Mission07_BeastCruiseMissileWave2Delay(void);
void Init_Mission07_BeastCruiseMissileWave3Delay(void);
void Watch_Mission07_BeastCruiseMissileWave3Delay(void);
void Init_Mission07_PrisonShipInfoParadeTeam(void);
void Watch_Mission07_PrisonShipInfoParadeTeam(void);
void Watch_Mission07_PrisonShipInfoParadeZoomIn(void);
void Watch_Mission07_PrisonShipInfoParadeOpenSensors(void);
sdword Watch_Mission07_PrisonShipInfoParadeOpenSensorsWait(void);
void Watch_Mission07_BeastCruiseMissileWaveController(void);
void TeamHaltAndClearMoveOrders(void);
sdword kasTimerStopById(sdword id);
sdword kasfShipsQueueBeastConversionOrScuttle(GrowSelection *ships, GrowSelection *attacker);

static bool32 Mission07_ConvoyWingHasActiveBeastDockCommand(AITeam *team)
{
    SelectCommand *selection;
    sdword i;

    selection = team->shipList.selection;
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

void Watch_Mission07(void)
{
    GrowSelection *allShips;
    GrowSelection *enemyShips;
    GrowSelection *playerShips;
    GrowSelection *supportSelection;
    GrowSelection *linkedShips;
    GrowSelection *workerSelection;
    GrowSelection *convoySelection;
    AITeam *fleetIntelTeam;
    AITeam *currentTeam;

    kasCurrentStep = 0;
    kasCurrentScopeHash = 142503671;

    if (kasTimerExpiredDestroyById(104338707))
    {
        kasTimerSetById(104338707, 5);
        kasTimerStartById(104338707);

        allShips = kasGrowSelectionPtr(18336307);
        kasfFindShipsInside(kasVolumePtr(132933767), allShips);

        enemyShips = kasGrowSelectionPtr(114961833);
        kasSelectEnemyShips(enemyShips, allShips);
        kasfShipsRemove(enemyShips, kasGrowSelectionPtr(37784437));
        kasfShipsRemove(enemyShips, kasGrowSelectionPtr(38093685));

        playerShips = kasGrowSelectionPtr(0x4A2A29);
        kasSelectPlayerShips(playerShips, allShips);
    }

    if (!kasfEntryValueGet(71552164) && kasfEntryValueGet(-2015429404) && kasfIntelEventEnded())
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);

        allShips = kasGrowSelectionPtr(18336307);
        supportSelection = kasGrowSelectionPtr(89157026);
        kasSelectPlayerShipsSimple(supportSelection, allShips);
        kasfShipsSelectNotSalvagers(supportSelection, supportSelection);

        linkedShips = kasGrowSelectionPtr(82017669);
        if (kasfShipsSelectLinkedShips(linkedShips, kasGrowSelectionPtr(211966048)))
        {
            kasfShipsAdd(supportSelection, linkedShips);
        }

        if (GetGameDifficulty() < 2 && kasfRUsGet(0) < 500)
        {
            workerSelection = kasGrowSelectionPtr(96759714);
            if (!kasfShipsSelectType(workerSelection, supportSelection, "sWorker"))
            {
                kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "sWorker", 1, kasGrowSelectionPtr(211966048), 0);
            }
        }

        kasfMsgSendAll((char *)73407406);
        kasTimerCreateSetStartById(92906708, 3);
        spSetSinglePlayerSaveInfo(7, m07_saveInfoName[strCurLanguage]);
    }

    if (kasTimerExpiredById(92906708))
    {
        fleetIntelTeam = kasAITeamPtrIfExists(176850956);
        kasfMsgSend(fleetIntelTeam, m07_fleetIntelTriggerMsg);
    }

    if (!kasfEntryValueGet(-2071823020) && kasfEntryValueGet(-1982550828))
    {
        convoySelection = kasGrowSelectionPtr(192819920);
        if (kasfShipsCount(convoySelection) > 0)
        {
            currentTeam = (AITeam *)kasCurrentTeamPtr;
            kasfShipsAdd(convoySelection, &currentTeam->shipList);
            fleetIntelTeam = kasAITeamPtrIfExists(176850956);
            kasfMsgSend(fleetIntelTeam, (char *)74880376);
            kasfEntrySetValue(-2071823020, 1);
        }
    }

    if (!kasfEntryValueGet(-1982550828) && kasfShipsAnyInSensorRange(kasGrowSelectionPtr(75774793)))
    {
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)265695220);
        kasfMsgSend(kasAITeamPtrIfExists(70995601), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995602), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995603), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995604), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995605), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995606), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995607), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995608), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995609), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995616), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995617), (char *)139340004);
        kasfMsgSend(kasAITeamPtrIfExists(70995618), (char *)139340004);
        kasfEntrySetValue(-1982550828, 1);
        kasfPingRemove("PingOnConvoy");
    }
}

void Watch_Mission07_P2Mothership_Reappear(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552170) && !kasfShipsCount(kasAITeamShipsPtrIfExists(93748889)))
    {
        kasfEntryGetOrCreateSetValue(71552170, 1);
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
        kasfShipsSetMaxVelocity(&team->shipList, 1);
    }
    if (!kasfEntryValueGet(71552171) && kasfTeamHealthAverage() < 80)
    {
        kasfEntryGetOrCreateSetValue(71552171, 1);
        kasfMsgSend(kasAITeamPtrIfExists((char *)23816689), (char *)84320888);
    }
    if (kasfMsgReceived((char *)5281445))
    {
        kasfAttackMothership();
        kasfShipsRemove(kasGrowSelectionPtr(127797474), &team->shipList);
    }
}

void Init_Mission07_P2Mothership_Reappear(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTacticsAggressive();
    kasfShipsSetMaxVelocity(&team->shipList, 1);
}

void Init_Mission07_MissionFlow(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerCreateById(144337492);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(232774915, 0);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfEntryGetOrCreateSetValue(54409420, 0);
    kasfSpeechEvent(
        32713509,
        (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
        (sdword)Watch_Mission07_MissionFlow);
}

void Watch_Mission07_WaitForIntelEventEnd(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
    {
        kasfEntrySetValue(-2054310428, 0);
        if (kasfEntryValueGet(168240451))
        {
            kasfEntrySetValue(168240451, 0);
            kasfSubtitleDraw();
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
            kasfEntrySetValue(127756738, 0);
            kasTimerStartById(144337492);
        }
        kasfSpeechEvent(
            374244,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission07_MissionFlowResumeDelay);
    }
}

void Watch_Mission07_MissionFlowResumeDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission07_MissionFlow);
    }
}

void Watch_Mission07_MissionFlow(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;
    GrowSelection *selection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(211966048)))
    {
        kasfMissionFailed();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    if (!kasfEntryValueGet(54409420))
    {
        if (!kasfEntryValueGet(332667))
        {
            for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
            {
                message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
                if (message == (void *)103903152)
                {
                    kasfSpeechEventQueueCurrentPlayer(268439963, 0);
                    kasfAllowPlayerToResearch("Tech_EMP");
                    kasfShipsSelectAllAttachedShips(kasGrowSelectionPtr(88483763), kasGrowSelectionPtr(211966048));
                    if (kasfShipsSelectType(kasGrowSelectionPtr(227921509), kasGrowSelectionPtr(88483763), "sMothershipWeapons") > 0)
                    {
                        kasfEntrySetValue(332667, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)51837810);
                    }
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)55426788)
                {
                    if (kasfEntryValueGet(-2121240156) <= 0)
                    {
                        kasfSubtitleSimulate(0, 8000, m07_convoyLost_subtitle[strCurLanguage]);
                    }
                    else
                    {
                        kasfSpeechEventQueueCurrentPlayer(268439961, 0);
                    }
                    kasfSpeechEvent(
                        77979796,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)74880376)
                {
                    kasfEntrySetValue(332667, 1);
                    kasTimerSetById(144337492, 1);
                    kasTimerStartById(144337492);
                    kasfWideScreenIn(30);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)124344274);
                    kasfSpeechEvent(
                        374244,
                        (sdword)MaintainTeamContext,
                        (sdword)Watch_Mission07_MissionFlowResumeDelay);
                    return;
                }
                else if (message == (void *)205402837)
                {
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEventQueueCurrentPlayer(268443987, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)65220002);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)111311252)
                {
                    kasfEntrySetValue(332667, 1);
                    kasTimerSetById(144337492, 1);
                    kasTimerStartById(144337492);
                    kasfWideScreenIn(30);
                    kasfMsgSend(kasAITeamPtrIfExists(73208097), (char *)187416633);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)170367282);
                    kasfSpeechEvent(
                        374244,
                        (sdword)MaintainTeamContext,
                        (sdword)Watch_Mission07_MissionFlowResumeDelay);
                    return;
                }
                else if (message == (void *)126483525)
                {
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEventQueueCurrentPlayer(268468634, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)144688146);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)217597236)
                {
                    kasfSpeechEventQueueCurrentPlayer(268443989, 0);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)82024195)
                {
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEventQueueCurrentPlayer(268439884, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)82024194);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message != NULL)
                {
                    aitQueuePointer(team, team, message);
                }
            }
        }
        else
        {
            for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
            {
                message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
                if (message == (void *)82024195)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfEntrySetValue(172907219, 1);
                    kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(70995601));
                    kasfCameraSetAngleDeg(341);
                    kasfCameraSetDeclinationDeg(-15);
                    kasfCameraSetDistance(2286);
                    kasfMsgSend(kasAITeamPtrIfExists(122653169), (char *)84320888);
                    kasfMsgSend(kasAITeamPtrIfExists(122653170), (char *)84320888);
                    kasfSpeechEventQueueCurrentPlayer(268468558, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)82024196);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)82024194)
                {
                    kasfOtherKASPause();
                    kasfUniversePause();
                    kasfEntrySetValue(211831571, 1);
                    kasfEntrySetValue(109086661, 1);
                    if (!kasfSensorsIsOpen(0))
                    {
                        kasfOpenSensors(150000);
                    }
                    kasfPingAddShips(kasAITeamShipsPtrIfExists(70995601), "PingOnConvoy");
                    kasfSensorsSetLookatPoint(kasTeamsVectorPtr(70995601));
                    kasfSpeechEventQueueCurrentPlayer(268443981, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)82024195);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)65220002)
                {
                    kasfSpeechEventQueueCurrentPlayer(268443988, 0);
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)51837810)
                {
                    kasfSpeechEventQueueCurrentPlayer(268444060, 0);
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)124344274)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(192819920));
                    kasfCameraSetAngleDeg(12);
                    kasfCameraSetDeclinationDeg(18);
                    kasfCameraSetDistance(750);
                    kasfSpeechEventQueueCurrentPlayer(268468561, 0);
                    kasfEntrySetValue(332667, 0);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(172907219, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)145266441);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)144688146)
                {
                    kasfSpeechEventQueueCurrentPlayer(268443984, 0);
                    kasfEntrySetValue(332667, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)91523540);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)82024196)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfSpeechEventQueueCurrentPlayer(268443983, 0);
                    kasfEntrySetValue(266064926, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)85383273);
                    kasfEntrySetValue(332667, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(4621956), (char *)217445459);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message == (void *)170367282)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    selection = kasGrowSelectionPtr(103856424);
                    kasfTutCameraFocusIgnoreZoom(selection);
                    kasfCameraSetAngleDeg(266);
                    kasfCameraSetDeclinationDeg(28);
                    kasfCameraSetDistance(2236);
                    if (kasfEntryValueGet(-1888986876) <= 2)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268443986, 0);
                    }
                    else
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444496, 0);
                    }
                    kasfEntrySetValue(332667, 0);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(172907219, 1);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission07_WaitForIntelEventEnd);
                    return;
                }
                else if (message != NULL)
                {
                    aitQueuePointer(team, team, message);
                }
            }
        }
    }
}

void Watch_Mission07_Events_PreparingAmbassador(void)
{
    AITeam *team;
    GrowSelection *selection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    selection = kasGrowSelectionPtr(74072178);
    kasfGrowSelectionClear(selection);
    if (!kasfShipsCount(selection))
    {
        kasfShipsSelectDamaged(selection, kasGrowSelectionPtr(211966048), 50);
    }
    if (!kasfShipsCount(selection))
    {
        kasfShipsSelectCapital(selection, kasGrowSelectionPtr((sdword)&loc_4A2A29));
        kasfShipsSelectDamaged(selection, selection, 50);
    }
    if (!kasfShipsCount(selection))
    {
        kasfShipsSelectCapital(selection, kasGrowSelectionPtr((sdword)&loc_4A2A29));
    }
    kasfCloak(selection);
    kasTimerCreateSetStartById(161590357, kasfRandom(5, 20));
}

void Watch_Mission07_MissionFlow_AdvanceToggle(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL || message == (void *)374384 || message == (void *)73407406)
        {
            continue;
        }
        if (message == (void *)21414)
        {
            kasfEntrySetValue(54409420, 1);
        }
        else if (message == (void *)1342)
        {
            kasfEntrySetValue(54409420, 0);
        }
        else
        {
            aitQueuePointer(team, team, message);
        }
    }
}

void Init_Mission07_ObjectiveEventQueue(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(121447844, 0);
    kasfEntryGetOrCreateSetValue(33538388, 0);
}

void Watch_Mission07_ObjectiveEventQueue(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(33538388) && kasfEntryValueGet(-1947719517))
    {
        if (!kasfShipsCount(kasGrowSelectionPtr(144638915)) &&
            !kasfShipsCount(kasGrowSelectionPtr(114961833)))
        {
            kasfEntrySetValue(33538388, 1);
        }
    }
    if (!kasfEntryValueGet(121447844) && kasfEntryValueGet(-1888986876) >= 1)
    {
        kasfEntrySetValue(121447844, 1);
    }
    if (!kasfEntryValueGet(71552161) &&
        kasfEntryValueGet(33538388) &&
        kasfEntryValueGet(121447844))
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfObjectiveDestroy("Protect");
        kasfObjectiveDestroy("Destroy");
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)217597236);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
    }
    if (!kasfEntryValueGet(71552162) && kasfEntryValueGet(-2121240156) > 11)
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)55426788);
    }
    if (!kasfEntryValueGet(71552163) &&
        !kasfEntryValueGet(121447844) &&
        kasfShipsCount(kasGrowSelectionPtr(7766316)) <= 0)
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)55426788);
    }
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL || message == (void *)73407406)
        {
            continue;
        }
        if (message == (void *)91523540)
        {
            kasfEntrySetValue(-1885650956, 1);
            kasfObjectiveDestroy("Mayday");
            kasfObjectiveCreate("Protect", m07_protectConvoy_brief[strCurLanguage], m07_protectConvoy_full[strCurLanguage]);
        }
        else if (message == (void *)85383273)
        {
            kasfObjectiveCreate("Mayday", m07_mayday_brief[strCurLanguage], m07_mayday_full[strCurLanguage]);
        }
        else if (message == (void *)145266441)
        {
            kasfObjectiveCreate("Destroy", m07_destroyInfectedShips_brief[strCurLanguage], m07_destroyInfectedShips_full[strCurLanguage]);
        }
        else if (message == (void *)175938357)
        {
            objectiveDestroyAll();
            kasfObjectiveCreate("Hyperspace", m07_hyperspace_brief[strCurLanguage], m07_hyperspace_full[strCurLanguage]);
            spSetSinglePlayerSaveInfo(7, m07_saveInfoName[strCurLanguage]);
        }
        else if (message == (void *)265695220)
        {
            kasfObjectiveDestroy("Mayday");
        }
        else if (message == (void *)55426788)
        {
            kasfMissionFailed();
        }
        else
        {
            aitQueuePointer(team, team, message);
        }
    }
}

void Init_Mission07_ConvoyWing(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(263722418, 0);
    kasfShipsEnableAttachNLips(&team->shipList);
    kasAITeamShipsPtr(&team->shipList, 3);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(70995601)) ||
        kasfThisTeamIs(kasAITeamPtrIfExists(70995603)))
    {
        kasfShipsSetPercentDamaged(&team->shipList, 35);
    }
    if (kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr(0xE30AADC), kasGrowSelectionPtr(0x117CA33), &team->shipList) <= 0)
    {
        kasfDockInstant((AITeam *)kasGrowSelectionPtr(0x2408B75));
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x4843B49), &team->shipList);
    }
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}

void Watch_Mission07_ConvoyWing(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == (void *)73407406)
        {
            if (kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr(0xE30AADC), kasGrowSelectionPtr(0x117CA33), &team->shipList) <= 0)
            {
                kasfSpeechEvent(221008577, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
            }
            else
            {
                kasfSpeechEvent(
                    33693906,
                    (sdword)Watch_Tutorial1_Tutorial10_CameraMoveUp,
                    (sdword)Watch_Mission07_ConvoyWing_CameraMoveDownTrigger);
            }
            return;
        }
        if (message == (void *)374384)
        {
            kasfEntryIncrement(-1888986876);
            kasfShipsRemove(kasGrowSelectionPtr(0x4A10BEC), &team->shipList);
            kasfVarDestroy((char *)&team->shipList);
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
            return;
        }
        if (message == (void *)81963396)
        {
            kasfShipsSelectIndex(kasGrowSelectionPtr(0x889C4C2), kasAITeamShipsPtrIfExists(44666274), 0);
            kasfShipsQueueBeastConversionOrScuttle(&team->shipList, kasGrowSelectionPtr(0x889C4C2));
        }
        else if (message == (void *)139340004)
        {
            kasfEntrySetValue(263722418, 1);
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }
    }
}

void Begin_Mission07_ConvoyWing_Launch(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(263722418))
    {
        kasfTeamSwitchPlayerOwner(0);
        kasfEntrySetValue(263722418, 0);
    }
    kasfLaunch();
}

void Watch_Mission07_ConvoyWing_Launch(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfEntryDecrement(-1983884075);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }
    if (kasfTeamFinishedLaunching())
    {
        if (kasfEntryValueGet(263722418))
        {
            kasfTeamSwitchPlayerOwner(0);
            kasfEntrySetValue(263722418, 0);
        }
        kasfShipsAdd(kasGrowSelectionPtr(0x4843B49), &team->shipList);
        kasfSpeechEvent(
            249371665,
            (sdword)Watch_Tutorial1_Tutorial10_CameraMoveUp,
            (sdword)Watch_Mission07_ConvoyWing_Separation);
    }
}

void Watch_Mission07_ConvoyWing_Separation(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(263722418))
    {
        kasfTeamSwitchPlayerOwner(0);
        kasfEntrySetValue(263722418, 0);
        kasfSpeechEvent(
            249371665,
            (sdword)Watch_Tutorial1_Tutorial10_CameraMoveUp,
            (sdword)Watch_Mission07_ConvoyWing_Separation);
    }
    else if (!kasfTeamCount())
    {
        kasfEntryDecrement(-1983884075);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (!kasfNearby(kasShipsVectorPtrIfExists(37784437), 22000))
    {
        kasfEntryDecrement(-1983884075);
        kasfSpeechEvent(
            33693906,
            (sdword)Watch_Tutorial1_Tutorial10_CameraMoveUp,
            (sdword)Watch_Mission07_ConvoyWing_CameraMoveDownTrigger);
    }
}

void Init_Mission07_PrisonShipInfoDispatchController(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x7E0A0D, 1);
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x98F85F4));
    kasfSpeechEvent(
        243559672,
        (sdword)MaintainTeamContext,
        (sdword)Dispatch_Mission07_PrisonShipInfoToHoldingTeam);
}

void Watch_Mission07_PrisonShipInfoDispatchQueue(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSelectType(kasGrowSelectionPtr(0x34E5FF3), kasGrowSelectionPtr(0x6DA2DA9), "dPrisonShipInfected");
    kasfShipsSelectSinglePlayerInParade(kasGrowSelectionPtr(0x34E5FF3), kasGrowSelectionPtr(0x34E5FF3));
    kasfShipsAdd(kasGrowSelectionPtr(0x98F85F4), kasGrowSelectionPtr(0x34E5FF3));
    if (kasfShipsCount(kasGrowSelectionPtr(0x98F85F4)) <= 0)
    {
        kasTimerCreateSetStartById(226991067, 5);
    }
    else
    {
        kasfSpeechEvent(
            243559672,
            (sdword)MaintainTeamContext,
            (sdword)Dispatch_Mission07_PrisonShipInfoToHoldingTeam);
    }
}

void Watch_Mission07_PrisonShipInfoDispatchQueueDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(226991067))
    {
        kasTimerDestroyById(226991067);
        kasfSpeechEvent(
            37939474,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission07_PrisonShipInfoDispatchQueue);
    }
}

void Dispatch_Mission07_PrisonShipInfoToHoldingTeam(void)
{
    AITeam *team;
    GrowSelection *ships;
    AITeam *holdingTeam;
    sdword dispatchIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr(0x98F85F4)) <= 0)
    {
        kasfSpeechEvent(
            37939474,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission07_PrisonShipInfoDispatchQueue);
        return;
    }

    kasfShipsSelectIndex(kasGrowSelectionPtr(0xBCE7D80), kasGrowSelectionPtr(0x98F85F4), 0);
    kasfShipsRemove(kasGrowSelectionPtr(0x98F85F4), kasGrowSelectionPtr(0xBCE7D80));
    dispatchIndex = kasfEntryValueGet(0x7E0A0D);
    ships = kasGrowSelectionPtr(0xBCE7D80);
    switch (dispatchIndex)
    {
        case 1:
            holdingTeam = kasAITeamPtrIfExists(196554993);
            break;
        case 2:
            holdingTeam = kasAITeamPtrIfExists(196554994);
            break;
        case 3:
            holdingTeam = kasAITeamPtrIfExists(196554995);
            break;
        case 4:
            holdingTeam = kasAITeamPtrIfExists(196554996);
            break;
        case 5:
            holdingTeam = kasAITeamPtrIfExists(196554997);
            break;
        case 6:
            holdingTeam = kasAITeamPtrIfExists(196554998);
            break;
        case 7:
            holdingTeam = kasAITeamPtrIfExists(196554999);
            break;
        case 8:
            holdingTeam = kasAITeamPtrIfExists(196555000);
            break;
        case 9:
            holdingTeam = kasAITeamPtrIfExists(196555001);
            break;
        case 10:
            holdingTeam = kasAITeamPtrIfExists(196554944);
            break;
        case 11:
            holdingTeam = kasAITeamPtrIfExists(196554945);
            break;
        case 12:
            holdingTeam = kasAITeamPtrIfExists(196554946);
            break;
        default:
            holdingTeam = NULL;
            break;
    }
    if (holdingTeam != NULL)
    {
        kasfReinforceTeamWithShips(holdingTeam, ships);
    }
    kasfEntryIncrement(-2121240156);
    kasfShipsRemove(kasGrowSelectionPtr(0x4A10BEC), ships);
    kasfEntryIncrement(0x7E0A0D);
}

static sdword m07_prisonHoldingTeams[] =
{
    196554993, 196554994, 196554995, 196554996, 196554997, 196554998,
    196554999, 196555000, 196555001, 196554944, 196554945, 196554946,
};

static sdword m07_prisonHoldingVectors[] =
{
    0x4D411, 0x4D412, 0x4D413, 0x4D414, 0x4D415, 0x4D416,
    0x4D417, 0x4D418, 0x4D419, 0x4D420, 0x4D421, 0x4D422,
};

static sdword m07_beastWave1Teams[] = { 95856209, 95856210, 95856211 };
static sdword m07_beastWave2Teams[] = { 95856225, 95856226, 95856227, 95856228, 95856229, 95856230 };
static sdword m07_beastWave3Teams[] = { 95856241, 95856242, 95856243, 95856244, 95856245, 95856246 };
static sdword m07_beastWave4Teams[] = { 95856257, 95856258, 95856261, 95856262 };
static sdword m07_beastWave5Teams[] = { 95856263, 95856264, 95856259, 95856260 };
static sdword m07_beastWave6Teams[] = { 95856289, 95856290, 95856291 };
static sdword m07_easyDisabledCruiseTeams[] = { 95856230, 95856246, 95856289, 95856290, 95856291 };

static char *m07_beastWave1Pings[] = { "PingOn1a", "PingOn1b", "PingOn1c" };
static char *m07_beastWave2Pings[] = { "PingOn2a", "PingOn2b", "PingOn2c", "PingOn2d", "PingOn2e", "PingOn2f" };
static char *m07_beastWave3Pings[] = { "PingOn3a", "PingOn3b", "PingOn3c", "PingOn3d", "PingOn3e", "PingOn3f" };
static char *m07_beastWave4Pings[] = { "PingOn4a", "PingOn4b", "PingOn4e", "PingOn4f" };
static char *m07_beastWave5Pings[] = { "PingOn4g", "PingOn4h", "PingOn4c", "PingOn4d" };
static char *m07_beastWave6Pings[] = { "PingOn6a", "PingOn6b", "PingOn6c" };

static bool32 Mission07_CurrentTeamMatchesHash(sdword teamHash)
{
    return kasfThisTeamIs(kasAITeamPtrIfExists(teamHash));
}

static bool32 Mission07_CurrentTeamMatchesAny(sdword *teamHashes, sdword count)
{
    sdword i;

    for (i = 0; i < count; i++)
    {
        if (Mission07_CurrentTeamMatchesHash(teamHashes[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void Mission07_SendMessageToTeams(sdword *teamHashes, sdword count, sdword message)
{
    sdword i;

    for (i = 0; i < count; i++)
    {
        kasfMsgSend(kasAITeamPtrIfExists(teamHashes[i]), (char *)message);
    }
}

static void Mission07_PingCurrentTeamShips(sdword *teamHashes, char **pingNames, sdword count)
{
    sdword i;

    for (i = 0; i < count; i++)
    {
        if (Mission07_CurrentTeamMatchesHash(teamHashes[i]))
        {
            kasfPingAddShips(kasAITeamShipsPtrIfExists(teamHashes[i]), pingNames[i]);
            break;
        }
    }
}

static sdword Mission07_CurrentWaveReserveTeamHash(void)
{
    if (Mission07_CurrentTeamMatchesHash(95856241))
    {
        return 126120401;
    }
    if (Mission07_CurrentTeamMatchesHash(95856242))
    {
        return 126120402;
    }
    if (Mission07_CurrentTeamMatchesHash(95856243))
    {
        return 126120403;
    }
    if (Mission07_CurrentTeamMatchesHash(95856257))
    {
        return 126120225;
    }
    if (Mission07_CurrentTeamMatchesHash(95856258))
    {
        return 126120226;
    }
    if (Mission07_CurrentTeamMatchesHash(95856259))
    {
        return 126120227;
    }
    if (Mission07_CurrentTeamMatchesHash(95856260))
    {
        return 126120228;
    }
    return 0;
}

static sdword Mission07_CurrentReserveSourceTeamHash(void)
{
    if (Mission07_CurrentTeamMatchesHash(126120401))
    {
        return 95856241;
    }
    if (Mission07_CurrentTeamMatchesHash(126120402))
    {
        return 95856242;
    }
    if (Mission07_CurrentTeamMatchesHash(126120403))
    {
        return 95856243;
    }
    if (Mission07_CurrentTeamMatchesHash(126120225))
    {
        return 95856257;
    }
    if (Mission07_CurrentTeamMatchesHash(126120226))
    {
        return 95856258;
    }
    if (Mission07_CurrentTeamMatchesHash(126120227))
    {
        return 95856259;
    }
    if (Mission07_CurrentTeamMatchesHash(126120228))
    {
        return 95856260;
    }
    return 0;
}

static void Mission07_FindCurrentReserveTargets(GrowSelection *ships)
{
    sdword sourceTeamHash;

    sourceTeamHash = Mission07_CurrentReserveSourceTeamHash();
    if (sourceTeamHash != 0)
    {
        kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(sourceTeamHash), ships);
    }
}

static void Mission07_MoveWaveShipsToReserve(AITeam *team, sdword sourceTeamHash, sdword reserveTeamHash)
{
    GrowSelection *transferShips;
    GrowSelection *sourceShips;
    AITeam *reserveTeam;

    transferShips = kasGrowSelectionPtr(0xA7847D3);
    sourceShips = kasAITeamShipsPtrIfExists(sourceTeamHash);
    reserveTeam = kasAITeamPtrIfExists(reserveTeamHash);

    kasfGrowSelectionClear(transferShips);
    kasfShipsAdd(transferShips, sourceShips);
    kasfShipsRemove(sourceShips, sourceShips);
    kasfReinforceTeamWithShips(team, transferShips);
    kasfTeamHyperspaceInNear(kasVectorPtrIfExists(0x546DBDE), 205000);
    kasfShipsRemove(&team->shipList, transferShips);
    kasfReinforceTeamWithShips(reserveTeam, transferShips);
    kasfMsgSend(reserveTeam, (char *)126123267);
}

void Init_Mission07_DockedLaunchGroup_SelectInvestigationTarget(void)
{
    AITeam *team;
    GrowSelection *candidateShips;
    GrowSelection *playerShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;

    candidateShips = kasGrowSelectionPtr(57245931);
    playerShips = kasGrowSelectionPtr(0x4A2A29);
    kasfGrowSelectionClear(candidateShips);

    if (!kasfShipsCount(candidateShips) && kasfEntryValueGet(187416633))
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(75774793));
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectType(candidateShips, playerShips, "sProcessor");
    }
    if (!kasfShipsCount(candidateShips) && kasfRandom(1, 2) == 1)
    {
        kasfShipsSelectType(candidateShips, playerShips, "sWorker");
        if (kasfShipsCount(candidateShips) <= 1)
        {
            kasfGrowSelectionClear(candidateShips);
        }
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectCapital(candidateShips, playerShips);
        kasfShipsRemove(candidateShips, kasGrowSelectionPtr(75774793));
        kasfShipsRemove(candidateShips, kasGrowSelectionPtr(211966048));
    }
    if (!kasfShipsCount(candidateShips) && kasfRandom(1, 3) != 1)
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(211966048));
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsAdd(candidateShips, playerShips);
    }

    kasfShipsSortByDistanceToPoint(candidateShips, (vector *)kasShipsVectorPtr((char *)0x4A2A29));
    kasfShipsSelectIndex(candidateShips, candidateShips, 0);
    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission07_DockedLaunchGroup_InvestigateTarget(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(141340587))
        {
            kasTimerDestroyById(141340587);
            kasfSpeechEvent(
                74024315,
                (sdword)Init_Mission07_DockedLaunchGroup_SelectInvestigationTarget,
                (sdword)Watch_Mission07_DockedLaunchGroup_InvestigateTarget);
        }
    }
    else
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission07_DockedLaunchGroup_LaunchComplete(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission07_DockedLaunchGroup_SelectInvestigationTarget,
            (sdword)Watch_Mission07_DockedLaunchGroup_InvestigateTarget);
    }
}

void Init_Mission07_DockedLaunchGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(187416633, 1);
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}

void Watch_Mission07_DockedLaunchGroup(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL || message == (void *)73407406)
        {
            continue;
        }

        if (message == (void *)70903842)
        {
            if (kasfTeamCount() > 0)
            {
                kasfEntrySetValue(187416633, 0);
                kasfShipsAdd(kasGrowSelectionPtr(0x89F03C3), &team->shipList);
                kasfSpeechEvent(
                    84320888,
                    (sdword)Init_LaunchDockedTeam,
                    (sdword)Watch_Mission07_DockedLaunchGroup_LaunchComplete);
                return;
            }
            continue;
        }

        if (message == (void *)187416633)
        {
            if (kasfTeamCount() > 0)
            {
                kasfEntrySetValue(187416633, 1);
                kasfShipsAdd(kasGrowSelectionPtr(0x89F03C3), &team->shipList);
                kasfSpeechEvent(
                    84320888,
                    (sdword)Init_LaunchDockedTeam,
                    (sdword)Watch_Mission07_DockedLaunchGroup_LaunchComplete);
                return;
            }
            continue;
        }

        aitQueuePointer(team, team, message);
    }
}

void Init_Mission07_BeastCruiseMissileWaveDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(128820994, 15);
}

void Watch_Mission07_BeastCruiseMissileWaveDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredById(128820994))
    {
        kasfSpeechEvent(
            187845043,
            (sdword)Init_Mission07_BeastCruiseMissileWaveTimer,
            (sdword)Watch_Mission07_BeastCruiseMissileWaveSpawner);
    }
}

void Init_Mission07_BeastCruiseMissileWaveTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerStopById(172555554);
    kasTimerSetById(172555554, kasfRandom(15, 35));
    kasTimerStartById(172555554);
}

void Watch_Mission07_BeastCruiseMissileWaveSpawner(void)
{
    AITeam *team;
    GrowSelection *cruiseMissiles;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }
    if (!kasTimerExpiredDestroyById(172555554))
    {
        return;
    }

    kasfShipsCount(kasAITeamShipsPtrIfExists(73208097));
    kasfShipsCount(kasAITeamShipsPtrIfExists(73208098));
    kasfShipsCount(kasAITeamShipsPtrIfExists(73208099));
    kasfShipsCount(kasAITeamShipsPtrIfExists(73208100));
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(73208101)))
    {
        kasfSpeechEvent(
            187845043,
            (sdword)Init_Mission07_BeastCruiseMissileWaveTimer,
            (sdword)Watch_Mission07_BeastCruiseMissileWaveSpawner);
        return;
    }

    cruiseMissiles = kasGrowSelectionPtr(0xED58CB3);
    kasfShipsCreate(cruiseMissiles, "bCruiseMissile", 2, 1);
    kasfShipsDockInstant(cruiseMissiles, &team->shipList);

    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208097)))
    {
        AITeam *missileTeam;

        missileTeam = kasAITeamPtrIfExists(73208097);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) < 1)
        {
            kasfEntryIncrement(249494211);
            if (!kasfEntryValueGet(-1982550828) || kasfEntryValueGet(-1893598540))
            {
                kasfMsgSend(missileTeam, (char *)187416633);
            }
            else
            {
                kasfShipsAdd(kasGrowSelectionPtr(0x630B928), &team->shipList);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)111311252);
            }
            if (!kasfEntryValueGet(-1893598540))
            {
                kasfEntrySetValue(-1893598540, 1);
            }
        }
        else
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208098)))
    {
        AITeam *missileTeam;

        missileTeam = kasAITeamPtrIfExists(73208098);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) < 1)
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
        else
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208099)))
    {
        AITeam *missileTeam;

        missileTeam = kasAITeamPtrIfExists(73208099);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) < 1)
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
        else
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208100)))
    {
        AITeam *missileTeam;

        missileTeam = kasAITeamPtrIfExists(73208100);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) < 1)
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
        else
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208101)))
    {
        AITeam *missileTeam;

        missileTeam = kasAITeamPtrIfExists(73208101);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) < 1)
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
        else
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
    }

    kasfSpeechEvent(
        187845043,
        (sdword)Init_Mission07_BeastCruiseMissileWaveTimer,
        (sdword)Watch_Mission07_BeastCruiseMissileWaveSpawner);
}

void Init_Mission07_BeastCruiseMissileWaveTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    kasTimerCreateById(172555554);
    kasfEntryGetOrCreateSetValue(249494211, 0);
    kasfSpeechEvent(110655924, (sdword)MaintainTeamContext, (sdword)Begin_Mission07_BeastCruiseMissileWaveSetup);
}

void Begin_Mission07_BeastCruiseMissileWaveSetup(void)
{
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamCount() <= 0)
    {
        return;
    }

    kasfShipsAdd(kasGrowSelectionPtr(0x89F03C3), &team->shipList);
    for (i = 0; i < sizeof(m07_prisonHoldingTeams) / sizeof(m07_prisonHoldingTeams[0]); i++)
    {
        if (Mission07_CurrentTeamMatchesHash(m07_prisonHoldingTeams[i]))
        {
            kasfLabelledVectorSet(m07_prisonHoldingVectors[i], 0, 0, 0);
            kasfVectorRandomizeInRange(
                kasVectorPtrIfExists(m07_prisonHoldingVectors[i]),
                kasShipsVectorPtr((char *)0xBB8),
                0xBB8);
            kasfMoveTo(kasVectorPtrIfExists(m07_prisonHoldingVectors[i]));
            break;
        }
    }

    kasfSpeechEvent(
        5532277,
        (sdword)Init_Mission07_BeastCruiseMissileWaveDelay,
        (sdword)Watch_Mission07_BeastCruiseMissileWaveDelay);
}

void Init_Mission07_BeastCruiseMissileWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;

    if (GetGameDifficulty() < 2 &&
        Mission07_CurrentTeamMatchesAny(
            m07_easyDisabledCruiseTeams,
            sizeof(m07_easyDisabledCruiseTeams) / sizeof(m07_easyDisabledCruiseTeams[0])))
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    kasfEntryGetOrCreate(248808115);
    if (Mission07_CurrentTeamMatchesAny(m07_beastWave3Teams, sizeof(m07_beastWave3Teams) / sizeof(m07_beastWave3Teams[0])))
    {
        kasfEntrySetValue(248808115, 5);
    }
    else if (Mission07_CurrentTeamMatchesAny(m07_beastWave4Teams, sizeof(m07_beastWave4Teams) / sizeof(m07_beastWave4Teams[0])) ||
             Mission07_CurrentTeamMatchesAny(m07_beastWave5Teams, sizeof(m07_beastWave5Teams) / sizeof(m07_beastWave5Teams[0])))
    {
        kasfEntrySetValue(248808115, 6);
    }
    else
    {
        kasfEntrySetValue(248808115, 4);
    }

    if (kasfEntryValueGet(248808115) > 0)
    {
        GrowSelection *cruiseMissiles;

        cruiseMissiles = kasGrowSelectionPtr(0xED58CB3);
        kasfShipsCreate(cruiseMissiles, "bCruiseMissile", kasfEntryValueGet(248808115), 1);
        kasfReinforceTeamWithShips(team, cruiseMissiles);
    }

    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    if (!GetGameDifficulty())
    {
        kasfTeamSetMaxVelocity(575);
    }
    else if (GetGameDifficulty() == 1 || GetGameDifficulty() == 2)
    {
        kasfTeamSetMaxVelocity(600);
    }
    else if (GetGameDifficulty() == 3)
    {
        kasfTeamSetMaxVelocity(650);
    }
    else if (GetGameDifficulty() == 4)
    {
        kasfTeamSetMaxVelocity(675);
    }

    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission07_BeastCruiseMissileWave_ReinforceFromReserve);
}

void Watch_Mission07_BeastCruiseMissileWave(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message != NULL && message != (void *)73407406)
        {
            aitQueuePointer(team, team, message);
        }
    }
}

void Watch_Mission07_BeastCruiseMissileWave_ReinforceFromReserve(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;
    sdword reserveTeamHash;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL)
        {
            continue;
        }

        if (message != (void *)84320888)
        {
            aitQueuePointer(team, team, message);
            continue;
        }

        reserveTeamHash = Mission07_CurrentWaveReserveTeamHash();
        if (reserveTeamHash != 0)
        {
            Mission07_MoveWaveShipsToReserve(team, team->kasLabelHash, reserveTeamHash);
        }
        else
        {
            kasfTeamHyperspaceInNear(kasVectorPtrIfExists(0x546DBDE), 205000);
        }
        kasfShipsAdd(kasGrowSelectionPtr(0x89F03C3), &team->shipList);
        kasfSpeechEvent(
            262295143,
            (sdword)Init_Mission07_BeastCruiseMissileWavePingDelay,
            (sdword)Watch_Mission07_BeastCruiseMissileWavePingDelay);
        return;
    }
}

void Init_Mission07_BeastCruiseMissileWavePingDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(68769205, 7);
}

void Select_Mission07_BeastCruiseMissileWaveTargets(void)
{
    AITeam *team;
    GrowSelection *candidateShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;

    candidateShips = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(candidateShips);
    if (!kasfShipsCount(candidateShips) &&
        (Mission07_CurrentTeamMatchesHash(95856230) || Mission07_CurrentTeamMatchesHash(95856246)))
    {
        if (!kasfShipsCount(candidateShips) && kasfRandom(1, 3) == 1)
        {
            kasfShipsSelectType(candidateShips, kasGrowSelectionPtr(0x4A2A29), "sProcessor");
        }
        if (!kasfShipsCount(candidateShips))
        {
            kasfShipsSelectType(candidateShips, kasGrowSelectionPtr(0x4A2A29), "sWorker");
            if (kasfShipsCount(candidateShips) <= 1)
            {
                kasfGrowSelectionClear(candidateShips);
            }
        }
        if (!kasfShipsCount(candidateShips))
        {
            kasfShipsSelectType(candidateShips, kasGrowSelectionPtr(0x4A2A29), "sProcessor");
        }
        if (!kasfShipsCount(candidateShips))
        {
            kasfShipsAdd(candidateShips, kasGrowSelectionPtr(211966048));
        }
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(75774793));
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectCapital(candidateShips, kasGrowSelectionPtr(0x4A2A29));
        kasfShipsRemove(candidateShips, kasGrowSelectionPtr(211966048));
    }
    if (!kasfShipsCount(candidateShips) && kasfRandom(1, 3) != 1)
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(211966048));
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(0x4A2A29));
    }

    kasfShipsSortByDistanceToPoint(candidateShips, (vector *)kasThisTeamsVectorPtr());
    kasfShipsSelectIndex(candidateShips, candidateShips, 0);
    kasfFormationWall();
    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(80544517, kasfRandom(5, 15));
}

void Watch_Mission07_BeastCruiseMissileWavePingDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredById(68769205))
    {
        Mission07_PingCurrentTeamShips(m07_beastWave1Teams, m07_beastWave1Pings, sizeof(m07_beastWave1Teams) / sizeof(m07_beastWave1Teams[0]));
        Mission07_PingCurrentTeamShips(m07_beastWave2Teams, m07_beastWave2Pings, sizeof(m07_beastWave2Teams) / sizeof(m07_beastWave2Teams[0]));
        Mission07_PingCurrentTeamShips(m07_beastWave3Teams, m07_beastWave3Pings, sizeof(m07_beastWave3Teams) / sizeof(m07_beastWave3Teams[0]));
        Mission07_PingCurrentTeamShips(m07_beastWave4Teams, m07_beastWave4Pings, sizeof(m07_beastWave4Teams) / sizeof(m07_beastWave4Teams[0]));
        Mission07_PingCurrentTeamShips(m07_beastWave5Teams, m07_beastWave5Pings, sizeof(m07_beastWave5Teams) / sizeof(m07_beastWave5Teams[0]));
        Mission07_PingCurrentTeamShips(m07_beastWave6Teams, m07_beastWave6Pings, sizeof(m07_beastWave6Teams) / sizeof(m07_beastWave6Teams[0]));
        kasfSpeechEvent(
            74024315,
            (sdword)Select_Mission07_BeastCruiseMissileWaveTargets,
            (sdword)Watch_Mission07_BeastCruiseMissileWaveTargetLoop);
    }
}

void Watch_Mission07_BeastCruiseMissileWaveTargetLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(80544517))
        {
            kasTimerDestroyById(80544517);
            kasfSpeechEvent(
                74024315,
                (sdword)Select_Mission07_BeastCruiseMissileWaveTargets,
                (sdword)Watch_Mission07_BeastCruiseMissileWaveTargetLoop);
        }
    }
    else
    {
        kasTimerDestroyById(80544517);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission07_BeastCruiseMissileEscortReserveTeam(void)
{
    AITeam *team;
    GrowSelection *ships;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    if (Mission07_CurrentTeamMatchesHash(126120403) || Mission07_CurrentTeamMatchesHash(126120228))
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    kasfEntryGetOrCreate(175340931);
    if (!GetGameDifficulty() || GetGameDifficulty() == 1)
    {
        kasfEntrySetValue(175340931, 3);
    }
    else if (GetGameDifficulty() == 2)
    {
        kasfEntrySetValue(175340931, 5);
    }
    else
    {
        kasfEntrySetValue(175340931, 7);
    }

    ships = kasGrowSelectionPtr(0xA7847D3);
    kasfShipsCreate(ships, "bInterceptor", kasfEntryValueGet(175340931), 1);
    kasfReinforceTeamWithShips(team, ships);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasTimerCreateSetStartById(128210323, 1);
    kasfSpeechEvent(
        374244,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTeam_AwaitLaunchSignal);
}

void Watch_Mission07_BeastCruiseMissileEscortReserveTeam_AwaitLaunchSignal(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL)
        {
            continue;
        }
        if (message == (void *)126123267)
        {
            kasfShipsAdd(kasGrowSelectionPtr(0x89F03C3), &team->shipList);
            kasfSpeechEvent(
                0x4C9664,
                (sdword)Begin_Mission07_BeastCruiseMissileEscortReserveTeam_Launch,
                (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTeam);
            return;
        }
        aitQueuePointer(team, team, message);
    }
}

void Begin_Mission07_BeastCruiseMissileEscortReserveTeam_Launch(void)
{
    AITeam *team;
    GrowSelection *protectTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;

    protectTargets = kasGrowSelectionPtr(154965556);
    kasfGrowSelectionClear(protectTargets);
    if (Mission07_CurrentTeamMatchesHash(126120401))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856241));
    }
    else if (Mission07_CurrentTeamMatchesHash(126120402))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856242));
    }
    else if (Mission07_CurrentTeamMatchesHash(126120403))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856243));
    }
    else if (Mission07_CurrentTeamMatchesHash(126120225))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856257));
    }
    else if (Mission07_CurrentTeamMatchesHash(126120226))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856258));
    }
    else if (Mission07_CurrentTeamMatchesHash(126120227))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856259));
    }
    else if (Mission07_CurrentTeamMatchesHash(126120228))
    {
        kasfShipsAdd(protectTargets, kasAITeamShipsPtrIfExists(95856260));
    }

    kasfFormationSphere();
    kasfProtect(protectTargets);
}

void Watch_Mission07_BeastCruiseMissileEscortReserveTeam(void)
{
    AITeam *team;
    sdword sourceTeamHash;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfStop();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    if (kasTimerExpiredDestroyById(128210323))
    {
        GrowSelection *attackers;

        kasTimerSetById(128210323, kasfRandom(2, 6));
        kasTimerStartById(128210323);
        attackers = kasGrowSelectionPtr(0x857BCE3);
        if (kasfUnderAttack(attackers) > 0)
        {
            kasfStop();
            kasfSpeechEvent(
                74024315,
                (sdword)Select_Mission07_BeastCruiseMissileEscortReserveTargets,
                (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTargetLoop);
            return;
        }

        Mission07_FindCurrentReserveTargets(attackers);
        if (kasfShipsCount(attackers) > 0)
        {
            kasfStop();
            kasfSpeechEvent(
                74024315,
                (sdword)Select_Mission07_BeastCruiseMissileEscortReserveTargets,
                (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTargetLoop);
            return;
        }
    }

    sourceTeamHash = Mission07_CurrentReserveSourceTeamHash();
    if (sourceTeamHash != 0 && !kasfShipsCount(kasAITeamShipsPtrIfExists(sourceTeamHash)))
    {
        kasfStop();
        kasfSpeechEvent(
            74024315,
            (sdword)Select_Mission07_BeastCruiseMissileEscortReserveTargets,
            (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTargetLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfStop();
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission07_BeastCruiseMissileEscortReserveTeam_Launch,
            (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTeam);
    }
}

void Select_Mission07_BeastCruiseMissileEscortReserveTargets(void)
{
    AITeam *team;
    GrowSelection *candidateShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;

    candidateShips = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(candidateShips);
    if (!kasfShipsCount(candidateShips))
    {
        Mission07_FindCurrentReserveTargets(candidateShips);
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfUnderAttack(candidateShips);
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectType(kasGrowSelectionPtr(154965556), kasGrowSelectionPtr(114961833), "dPrisonShipInfected");
        kasfFindShipsTargetingSelection(kasGrowSelectionPtr(154965556), candidateShips);
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectNonCapital(candidateShips, kasGrowSelectionPtr(0x4A2A29));
    }
    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(0x4A2A29));
    }

    kasfFormationDelta();
    kasfShipsRemove(candidateShips, kasGrowSelectionPtr(75774793));
    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission07_BeastCruiseMissileEscortReserveTargetLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(141340587))
        {
            kasTimerDestroyById(141340587);
            kasfSpeechEvent(
                74024315,
                (sdword)Select_Mission07_BeastCruiseMissileEscortReserveTargets,
                (sdword)Watch_Mission07_BeastCruiseMissileEscortReserveTargetLoop);
        }
    }
    else
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission07_BeastCruiseMissileWaveSequence(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission07_BeastCruiseMissileWaveSequence(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL || message == (void *)73407406)
        {
            continue;
        }
        if (message == (void *)217445459)
        {
            kasfSpeechEvent(
                5991009,
                (sdword)Init_Mission07_BeastCruiseMissileWave1Delay,
                (sdword)Watch_Mission07_BeastCruiseMissileWave1Delay);
            return;
        }
        aitQueuePointer(team, team, message);
    }
}

void Init_Mission07_BeastCruiseMissileWave1Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(176088177, 150);
}

void Watch_Mission07_BeastCruiseMissileWave1Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(176088177))
    {
        kasTimerDestroyById(176088177);
        kasfMsgSend(kasAITeamPtrIfExists(5991009), (char *)84320888);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)126483525);
        kasfSpeechEvent(
            5991010,
            (sdword)Init_Mission07_BeastCruiseMissileWave2Delay,
            (sdword)Watch_Mission07_BeastCruiseMissileWave2Delay);
    }
}

void Init_Mission07_BeastCruiseMissileWave2Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(176088178, 170);
}

void Watch_Mission07_BeastCruiseMissileWave2Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredById(176088178))
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)103903152);
        kasfMsgSend(kasAITeamPtrIfExists(5991010), (char *)84320888);
        kasfSpeechEvent(
            5991011,
            (sdword)Init_Mission07_BeastCruiseMissileWave3Delay,
            (sdword)Watch_Mission07_BeastCruiseMissileWave3Delay);
    }
}

void Init_Mission07_BeastCruiseMissileWave3Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(176088179, 230);
}

void Init_Mission07_BeastCruiseMissileWave4Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(176088180, 330);
}

void Watch_Mission07_BeastCruiseMissileWave4Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredById(176088180))
    {
        kasfMsgSend(kasAITeamPtrIfExists(5991012), (char *)84320888);
        kasfSpeechEvent(
            5991013,
            (sdword)Init_Mission07_BeastCruiseMissileWave5Delay,
            (sdword)Watch_Mission07_BeastCruiseMissileWave5Delay);
    }
}

void Init_Mission07_BeastCruiseMissileWave5Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(176088181, 20);
}

void Watch_Mission07_BeastCruiseMissileWave3Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredById(176088179))
    {
        kasfMsgSend(kasAITeamPtrIfExists(5991011), (char *)84320888);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)205402837);
        kasfSpeechEvent(
            5991012,
            (sdword)Init_Mission07_BeastCruiseMissileWave4Delay,
            (sdword)Watch_Mission07_BeastCruiseMissileWave4Delay);
    }
}

void Watch_Mission07_BeastCruiseMissileWave5Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasTimerExpiredById(176088181))
    {
        kasfMsgSend(kasAITeamPtrIfExists(5991013), (char *)84320888);
        if (GetGameDifficulty() >= 2)
        {
            kasfSpeechEvent(
                5991014,
                (sdword)Init_Mission07_Events_HyperdriveFailDelay,
                (sdword)Watch_Mission07_Events_HyperdriveFail);
        }
        else
        {
            kasfEntrySetValue(-1947719517, 1);
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        }
    }
}

void Init_Mission07_PrisonShipInfoParadeTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    kasTimerCreateSetStartById(226991067, 5);
    kasfSpeechEvent(
        110655924,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission07_PrisonShipInfoParadeZoomIn);
}

void Watch_Mission07_PrisonShipInfoParadeTeam(void)
{
    AITeam *team;
    GrowSelection *paradeShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(226991067))
    {
        kasTimerSetById(226991067, 5);
        kasTimerStartById(226991067);

        paradeShips = kasGrowSelectionPtr(0xA3CA763);
        kasfGrowSelectionClear(paradeShips);
        kasfShipsAdd(paradeShips, kasGrowSelectionPtr(0x6DA2DA9));
        kasfShipsSelectType(kasGrowSelectionPtr(0xAEBB739), paradeShips, "dPrisonShipInfected");
        kasfShipsRemove(paradeShips, kasGrowSelectionPtr(0xAEBB739));
        kasfShipsSelectSinglePlayerInParade(paradeShips, paradeShips);
        if (kasfShipsCount(paradeShips) > 0)
        {
            kasfReinforceTeamWithShips(team, paradeShips);
        }
    }
}

void Watch_Mission07_PrisonShipInfoParadeZoomIn(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamCount() > 0)
    {
        kasfShipsAdd(kasGrowSelectionPtr(144638915), &team->shipList);
        kasfSpeechEvent(
            74024315,
            (sdword)Watch_Mission07_PrisonShipInfoParadeOpenSensors,
            (sdword)Watch_Mission07_PrisonShipInfoParadeOpenSensorsWait);
    }
}

void Watch_Mission07_PrisonShipInfoParadeOpenSensors(void)
{
    AITeam *team;
    GrowSelection *openSensorsShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;

    openSensorsShips = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(openSensorsShips);
    if (!kasfShipsCount(openSensorsShips))
    {
        kasfShipsAdd(openSensorsShips, kasGrowSelectionPtr(0x4A2A29));
        kasfShipsRemove(openSensorsShips, kasGrowSelectionPtr(75774793));
    }

    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

sdword Watch_Mission07_PrisonShipInfoParadeOpenSensorsWait(void)
{
    AITeam *team;
    sdword result;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfTeamCount())
    {
        result = kasTimerExpiredDestroyById(141340587);
        if (kasfShipsAnyIdle(&team->shipList) || result)
        {
            kasTimerDestroyById(141340587);
            kasfSpeechEvent(
                74024315,
                (sdword)Watch_Mission07_PrisonShipInfoParadeOpenSensors,
                (sdword)Watch_Mission07_PrisonShipInfoParadeOpenSensorsWait);
            return 1;
        }
        return result;
    }

    kasTimerDestroyById(141340587);
    kasfSpeechEvent(
        110655924,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission07_PrisonShipInfoParadeZoomIn);
    return 0;
}

void Watch_Mission07_BeastCruiseMissileWaveController(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL)
        {
            continue;
        }

        if (message != (void *)84320888)
        {
            aitQueuePointer(team, team, message);
            continue;
        }

        if (Mission07_CurrentTeamMatchesHash(5991009))
        {
            Mission07_SendMessageToTeams(m07_beastWave1Teams, sizeof(m07_beastWave1Teams) / sizeof(m07_beastWave1Teams[0]), 84320888);
        }
        else if (Mission07_CurrentTeamMatchesHash(5991010))
        {
            Mission07_SendMessageToTeams(m07_beastWave2Teams, sizeof(m07_beastWave2Teams) / sizeof(m07_beastWave2Teams[0]), 84320888);
        }
        else if (Mission07_CurrentTeamMatchesHash(5991011))
        {
            Mission07_SendMessageToTeams(m07_beastWave3Teams, sizeof(m07_beastWave3Teams) / sizeof(m07_beastWave3Teams[0]), 84320888);
        }
        else if (Mission07_CurrentTeamMatchesHash(5991012))
        {
            Mission07_SendMessageToTeams(m07_beastWave4Teams, sizeof(m07_beastWave4Teams) / sizeof(m07_beastWave4Teams[0]), 84320888);
        }
        else if (Mission07_CurrentTeamMatchesHash(5991013))
        {
            Mission07_SendMessageToTeams(m07_beastWave5Teams, sizeof(m07_beastWave5Teams) / sizeof(m07_beastWave5Teams[0]), 84320888);
        }
        else if (Mission07_CurrentTeamMatchesHash(5991014))
        {
            Mission07_SendMessageToTeams(m07_beastWave6Teams, sizeof(m07_beastWave6Teams) / sizeof(m07_beastWave6Teams[0]), 84320888);
            kasfEntrySetValue(-1947719517, 1);
        }
    }
}

void Watch_Mission07_ConvoyWing_CameraMoveDownTrigger(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(263722418))
    {
        kasfTeamSwitchPlayerOwner(0);
        kasfEntrySetValue(263722418, 0);
        kasfSpeechEvent(
            33693906,
            (sdword)Watch_Tutorial1_Tutorial10_CameraMoveUp,
            (sdword)Watch_Mission07_ConvoyWing_CameraMoveDownTrigger);
    }
    else if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (kasfEntryValueGet(-2140768747) <= 0)
    {
        if (kasfNearby(kasShipsVectorPtrIfExists(38093685), 4000))
        {
            kasfEntryIncrement(-2140768747);
            kasfSpeechEvent(
                230759474,
                (sdword)Watch_Tutorial1_Tutorial10_CameraMoveDown,
                (sdword)Watch_Mission07_ConvoyWing_BreakFormation);
        }
    }
    else
    {
        kasfSpeechEvent(
            71927298,
            (sdword)TeamHaltAndClearMoveOrders,
            (sdword)Watch_Mission07_ConvoyWing_CameraMoveUpTrigger);
    }
}

void Watch_Mission07_ConvoyWing_CameraMoveUpTrigger(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(263722418))
    {
        kasfTeamSwitchPlayerOwner(0);
        kasfEntrySetValue(263722418, 0);
    }
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (!kasfEntryValueGet(-2140768747))
    {
        kasfSpeechEvent(
            33693906,
            (sdword)Watch_Tutorial1_Tutorial10_CameraMoveUp,
            (sdword)Watch_Mission07_ConvoyWing_CameraMoveDownTrigger);
    }
}

void Watch_Mission07_ConvoyWing_BreakFormation(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfEntryDecrement(-2140768747);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (!Mission07_ConvoyWingHasActiveBeastDockCommand(team))
    {
        kasfEntryIncrement(-1888986876);
        kasfShipsRemove(kasGrowSelectionPtr(0x4A10BEC), &team->shipList);
        kasfEntryDecrement(-2140768747);
        kasfHideShips(&team->shipList);
        kasfTeamAttributesBitSet(128);
        kasfAttackMothership();
        kasfDeleteShips(&team->shipList);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_HideTeamShips(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfHideShips(&teamtoreinforce->shipList);
}

void Init_HiddenTeam_AwaitLaunchSignal(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfShipsMakeCrazy(&teamtoreinforce->shipList, 0);
    kasfHideShips(&teamtoreinforce->shipList);
    kasfSpeechEvent(80578718, (sdword)MaintainTeamContext, 0);
}

void Watch_HiddenTeam_ReturnToMaintainIfDestroyed(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission07_ConvoyFocus_ReleaseAfterLaunch(void)
{
    AITeam *team;

    team = teamtoreinforce;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(122653169)))
    {
        tutDeallocateRootRegion((sdword)kasAITeamShipsPtrIfExists(70995601));
    }
    if (kasfThisTeamIs(kasAITeamPtrIfExists(122653170)))
    {
        tutDeallocateRootRegion((sdword)kasAITeamShipsPtrIfExists(70995602));
    }
}

void Watch_HiddenTeam_AwaitLaunchSignal(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;

    team = teamtoreinforce;
    kasCurrentStep = 2;
    scope = team->kasLabelHash;
    for (queueIndex = aitQueueCount(team) - 1; queueIndex >= 0; queueIndex--)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == NULL)
        {
            continue;
        }
        if (message == (void *)84320888)
        {
            kasfUnhideShips(&team->shipList);
            kasfSpeechEvent(74024315, (sdword)Init_Mission07_ConvoyFocus_ReleaseAfterLaunch, 0);
            return;
        }
        aitQueuePointer(team, team, message);
    }
}


extern void Init_FleetIntel_CruiseMissileWaveTeam(void);

void Init_Mission07(void)
{
    GrowSelection *sel;
    kasCurrentStep = 0;
    scope = KH_mission07;

    sel = kasGrowSelectionPtr(77663212);
    kasfGrowSelectionClear(sel);
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995601));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995602));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995603));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995604));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995605));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995606));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995607));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995608));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995609));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995616));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995617));
    kasfShipsAdd((int *)kasGrowSelectionPtr(77663212), (int *)kasAITeamShipsPtrIfExists(70995618));

    kasfEntryGetOrCreateSetValue(-2071823020, 0);
    kasfEntryGetOrCreateSetValue(-2127936844, 0);
    kasfEntryGetOrCreateSetValue(-1893598540, 0);
    kasfEntryGetOrCreateSetValue(-1885650956, 0);
    kasfEntryGetOrCreateSetValue(-1947719517, 0);
    kasfEntryGetOrCreateSetValue(-1982550828, 0);
    kasfEntryGetOrCreateSetValue(-1983884075, 0);
    kasfEntryGetOrCreateSetValue(-2140768747, 0);
    kasfEntryGetOrCreateSetValue(-1888986876, 0);
    kasfEntryGetOrCreateSetValue(-2121240156, 0);

    kasfFindBeastShipsInside(kasVolumePtr(128789333), kasGrowSelectionPtr(37784437));
    kasfFindBeastShipsInside(kasVolumePtr(189343956), kasGrowSelectionPtr(38093685));
    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeEnemyCombatShips((int *)&kasGrowSelectionPtr(114961833)->maxNumShips, (int)kasGrowSelectionPtr(18336307));
    kasfShipsRemove(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(37784437));
    kasfShipsRemove(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(38093685));
    kasfShipsSelectMissionVolumeInactivePlayerModules((int *)&kasGrowSelectionPtr(KH_Enemy)->maxNumShips, (int)kasGrowSelectionPtr(18336307));

    kasFSMCreate(KH_FleetIntel, Init_Mission07_MissionFlow, Watch_Mission07_MissionFlow_AdvanceToggle,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Init_Mission07_ObjectiveEventQueue, Watch_Mission07_ObjectiveEventQueue,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(4627076, Init_Mission07_BeastCruiseMissileWaveSequence, Watch_Mission07_BeastCruiseMissileWaveSequence,
                 kasAITeamPtrIfExists(4627076));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995601));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995602));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995603));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995604));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995605));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995606));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995607));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995608));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995609));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995616));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995617));
    kasFSMCreate(75774793, Init_Mission07_ConvoyWing, Watch_Mission07_ConvoyWing, kasAITeamPtrIfExists(70995618));
    kasFSMCreate(87252760, Init_Mission07_PrisonShipInfoDispatchController, Watch_Mission07_BeastCruiseMissileWave,
                 kasAITeamPtrIfExists(56564114));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554993));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554994));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554995));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554996));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554997));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554998));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554999));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196555000));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196555001));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554944));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554945));
    kasFSMCreate(183220025, Init_FleetIntel_CruiseMissileWaveTeam, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(196554946));
    kasFSMCreate(189411989, Init_Mission07_DockedLaunchGroup, Watch_Mission07_DockedLaunchGroup, kasAITeamPtrIfExists(73208097));
    kasFSMCreate(189411989, Init_Mission07_DockedLaunchGroup, Watch_Mission07_DockedLaunchGroup, kasAITeamPtrIfExists(73208098));
    kasFSMCreate(189411989, Init_Mission07_DockedLaunchGroup, Watch_Mission07_DockedLaunchGroup, kasAITeamPtrIfExists(73208099));
    kasFSMCreate(189411989, Init_Mission07_DockedLaunchGroup, Watch_Mission07_DockedLaunchGroup, kasAITeamPtrIfExists(73208100));
    kasFSMCreate(189411989, Init_Mission07_DockedLaunchGroup, Watch_Mission07_DockedLaunchGroup, kasAITeamPtrIfExists(73208101));
    kasFSMCreate(44666274, Init_HideTeamShips, Watch_NoOpTeamContext, kasAITeamPtrIfExists(44666274));
    kasFSMCreate(158660757, Init_HiddenTeam_AwaitLaunchSignal, Watch_HiddenTeam_ReturnToMaintainIfDestroyed, kasAITeamPtrIfExists(122653169));
    kasFSMCreate(158660757, Init_HiddenTeam_AwaitLaunchSignal, Watch_HiddenTeam_ReturnToMaintainIfDestroyed, kasAITeamPtrIfExists(122653170));
    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission07_BeastCruiseMissileWaveController, kasAITeamPtrIfExists(5990497));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856209));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856210));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856211));
    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission07_BeastCruiseMissileWaveController, kasAITeamPtrIfExists(5990498));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856225));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856226));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856227));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856228));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856229));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856230));
    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission07_BeastCruiseMissileWaveController, kasAITeamPtrIfExists(5990499));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856241));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856242));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856243));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856244));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856245));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856246));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120401));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120402));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120403));
    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission07_BeastCruiseMissileWaveController, kasAITeamPtrIfExists(5990500));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856257));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856258));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856261));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856262));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120225));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120226));
    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission07_BeastCruiseMissileWaveController, kasAITeamPtrIfExists(5990501));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856263));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856264));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856259));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856260));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120227));
    kasFSMCreate(78087284, Init_Mission07_BeastCruiseMissileEscortReserveTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(126120228));
    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission07_BeastCruiseMissileWaveController, kasAITeamPtrIfExists(5990502));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856289));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856290));
    kasFSMCreate(374437, Init_Mission07_BeastCruiseMissileWave, Watch_Mission07_BeastCruiseMissileWave, kasAITeamPtrIfExists(95856291));
    kasFSMCreate(129848979, Init_Mission07_PrisonShipInfoParadeTeam, Watch_Mission07_PrisonShipInfoParadeTeam, kasAITeamPtrIfExists(129848979));

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
    kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");
    kasfAllowPlayerToResearch("TECH_ARMOUR2");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sSentinel");
    kasfModuleSlotOrderSetShipType(0, "sLeech");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetShipType(0, "sHiveFrigate");
    kasfModuleSlotOrderSetCount(0, 5);
    kasfModuleSlotOrderSetShipType(1, "bAcolyte");
    kasfModuleSlotOrderSetShipType(1, "bACV");
    kasfModuleSlotOrderSetShipType(1, "bRammingFrigate");
    kasfModuleSlotOrderSetShipType(1, "bHiveFrigate");
    kasfModuleSlotOrderSetShipType(1, "dPrisonShip");
    kasfModuleSlotOrderSetCount(1, 4);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfDisableAllAIFeatures();

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipEngineering", "RACE_SECT", 1);
    kasfEnableShip("sMothershipWeapons", "RACE_SECT", 1);
    kasfEnableShip("sMothershipArmour", "RACE_SECT", 1);

    kasfFindEnemyMothership(kasGrowSelectionPtr(211966048));
    kasfLocationCard(8000, "Outskirts of Kadiir Nebula");
    kasfDisablePlayerHyperspace();
    kasfMusicPlay(6);
    kasTimerCreateSetStartById(104338707, 5);
}
