/*=============================================================================
    Name    : mission03.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission03
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m03_systemName[] =
{
    "Coruc-Tel System",
    "French version goes here",
    "German version goes here",
};

static char *m03_locationCard[] =
{
    "Deep Space (Tel Sector)",
    "French version goes here",
    "German version goes here",
};

static char *m03_saveInfoTitle[] =
{
    "~ End of Mission",
    "French version goes here",
    "German version goes here",
};

static char *m03_moduleMoving_subtitle[] =
{
    "The module is on the move, its on an intercept course with KUUN-LAN.  We can't let it get close to us.",
    "French version goes here",
    "German version goes here",
};

static char *m03_rebuildHangar_brief[] =
{
    "Replace HANGAR MODULE",
    "French version goes here",
    "German version goes here",
};

static char *m03_rebuildHangar_full[] =
{
    "We need to rebuild the hangar module.  The hangar module is listed in the build manager near the bottom of the list.",
    "French version goes here",
    "German version goes here",
};

static char *m03_escortCleeSan_brief[] =
{
    "Escort CLEE-SAN with 10 ACOLYTES",
    "French version goes here",
    "German version goes here",
};

static char *m03_escortCleeSan_full[] =
{
    "Send a contingent of 10 ACOLYTES or 5 ACVs to provide support for the CLEE-SAN.  Instruct them to guard the CLEE-SAN in sphere formation.",
    "French version goes here",
    "German version goes here",
};

static char *m03_researchInfection_brief[] =
{
    "Research Infection",
    "French version goes here",
    "German version goes here",
};

static char *m03_researchInfection_full[] =
{
    "Whatever happened to the research module must have spread to the other ships.  We have no choice but to deal with them as hostiles.  Get a research team studying the tapes and stay away from the research module at all costs.",
    "French version goes here",
    "German version goes here",
};

static char *m03_protectKuunLan_brief[] =
{
    "Protect KUUN-LAN",
    "French version goes here",
    "German version goes here",
};

static char *m03_protectKuunLan_full[] =
{
    "Destroy any infected ships that attack us while we review the tapes to learn more about what happened to the CLEE-SAN.  However, stay away from the research module as it will probably infect anything that gets near.",
    "French version goes here",
    "German version goes here",
};

static char *m03_hyperspace_brief[] =
{
    "Hyperspace",
    "French version goes here",
    "German version goes here",
};

static char *m03_hyperspace_full[] =
{
    "Rally the fleet and prepare to hyperspace on your mark.",
    "French version goes here",
    "German version goes here",
};

void kasSpeechEventStopTeam(void);
sdword kasfShipsSelectSinglePlayerCombatShips(sdword *a1, sdword a2);
void Init_Mission03_AssignedWaypointTeam(void);
void Watch_Mission03_AssignedWaypointTeam(void);
void Mission03_SendTeamToAssignedWaypoint(void);
void Mission03_SendTeamToAssignedTargets(void);
void Mission03_ResetAssignedWaypointTeam(void);
void Watch_Mission03_AssignedWaypointTeamIdle(void);
void Mission03_RetryAssignedWaypoint(void);
void Init_Mission03_CleeSanSupportTeam(void);
void Watch_Mission03_CleeSanSupportTeam(void);
void Mission03_SendSupportTeamToCleeSan(void);
void Mission03_ArmCleeSanThreatTimer(void);
void Watch_Mission03_CleeSanDockedReadyForLaunch(void);
void Watch_Mission03_CleeSanSensorContact(void);
void Mission03_HandleCleeSanSensorThreats(void);
void Mission03_SendResponseTeamToCleeSan(void);
void Mission03_WatchCleeSanThreatOrLaunch(void);
void Mission03_MarkAssignedWaypointHandled(void);
void Mission03_MarkCleeSanLaunchReady(void);
void Mission03_MarkCleeSanResponseActive(void);
void Init_Mission03_CleeSanResponseCoordinator(void);
void Watch_Mission03_CleeSanResponseCoordinator(void);
void Init_Mission03_CleeSanLeadAttackTeam(void);
void Watch_Mission03_CleeSanLeadAttackTeam(void);
void Mission03_SendCleeSanLeadAttackTeamToApproachVector(void);
void Init_Mission03_CleeSanWingAttackTeam(void);
void Mission03_StartCleeSanWingAttackRun(void);
void Mission03_RetargetCleeSanWingAttackRun(void);
void Mission03_SendCleeSanWingAttackTeamToApproachVector(void);
void Init_Mission03_CleeSanPrimaryResponseTeam(void);
void Watch_Mission03_CleeSanPrimaryResponseTeam(void);
void Mission03_SetCleeSanPrimaryResponseApproachVector(void);
void Mission03_StartCleeSanPrimaryResponseAttackRun(void);
void Watch_Mission03_CleeSanLeadAttackRun(void);
void Watch_Mission03_CleeSanWingAttackRun(void);
void Watch_Mission03_CleeSanWingRetarget(void);
void Watch_Mission03_CleeSanPrimaryResponseAttackRun(void);

void Watch_Mission03(void)
{
    GrowSelection *nearbyShips;
    GrowSelection *enemyShips;
    GrowSelection *playerShips;
    GrowSelection *mothershipSelection;
    GrowSelection *supportSelection;
    GrowSelection *linkedShips;
    GrowSelection *workerSelection;
    GrowSelection *missileSelection;
    AITeam *fleetIntelTeam;
    AITeam *moduleTeam;

    kasCurrentStep = 0;
    kasCurrentScopeHash = 142503667;

    if (kasTimerExpiredDestroyById(104338707))
    {
        nearbyShips = kasGrowSelectionPtr(18336307);
        kasfFindShipsInside(kasVolumePtr(132933767), nearbyShips);

        enemyShips = kasGrowSelectionPtr(114961833);
        kasSelectEnemyShips(enemyShips, nearbyShips);

        playerShips = kasGrowSelectionPtr(0x4A2A29);
        kasSelectPlayerShips(playerShips, nearbyShips);
        kasfShipsRemove(playerShips, kasAITeamShipsPtrIfExists(134850334));

        kasTimerSetById(104338707, kasfRandom(8, 15));
        kasTimerStartById(104338707);
    }

    if (kasfEntryValueGet(-2015429404) && kasfShipsCount(kasGrowSelectionPtr(218060896)))
    {
        playerShips = kasGrowSelectionPtr(0x4A2A29);
        mothershipSelection = kasGrowSelectionPtr(130693705);
        if (kasfShipsSelectType(mothershipSelection, playerShips, "sMothershipDockingBay"))
        {
            if (!kasfEntryValueGet(160113113) && !kasCheckShipDockFlag(mothershipSelection))
            {
                if (kasfEntryValueGet(-2111991596))
                {
                    moduleTeam = kasAITeamPtrIfExists(160291907);
                    kasfMsgSend(moduleTeam, (char *)175938357);
                }

                kasfEntryGetOrCreateSetValue(160113113, 1);
                moduleTeam = kasAITeamPtrIfExists(160291907);
                kasfMsgSend(moduleTeam, (char *)262155637);
            }
        }
        else if (kasfEntryValueGet(160113113))
        {
            if (kasfEntryValueGet(-2111991596))
            {
                kasfObjectiveDestroy("Hyperspace");
            }

            kasfEntryGetOrCreateSetValue(160113113, 0);
            fleetIntelTeam = kasAITeamPtrIfExists(176850956);
            kasfMsgSend(fleetIntelTeam, (char *)156249397);
        }
    }

    if (!kasfEntryValueGet(71552167) && kasfEntryValueGet(-2015429404))
    {
        kasfEntrySetValue(71552167, 1);

        fleetIntelTeam = kasAITeamPtrIfExists(176850956);
        kasfMsgSend(fleetIntelTeam, (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(44055618), (char *)5801588);

        kasfPingAddShips(kasAITeamShipsPtrIfExists(134850334), "PingOnCleeSan");
        spSetSinglePlayerSaveInfo(3, m03_systemName[strCurLanguage]);
        kasfEntrySetValue(160113113, 1);

        nearbyShips = kasGrowSelectionPtr(18336307);
        supportSelection = kasGrowSelectionPtr(89157026);
        kasSelectPlayerShipsSimple(supportSelection, nearbyShips);
        kasfShipsSelectNotSalvagers(supportSelection, supportSelection);

        linkedShips = kasGrowSelectionPtr(82017669);
        if (kasfShipsSelectLinkedShips(linkedShips, kasGrowSelectionPtr(218060896)))
        {
            kasfShipsAdd(supportSelection, linkedShips);
        }

        if (GetGameDifficulty() < 2 && kasfRUsGet(0) < 500)
        {
            workerSelection = kasGrowSelectionPtr(96759714);
            if (!kasfShipsSelectType(workerSelection, supportSelection, "sWorker"))
            {
                kasfShipsCreateInside(kasGrowSelectionPtr(154639956), "sWorker", 1, kasGrowSelectionPtr(218060896), 0);
            }
        }
    }

    if (!kasfEntryValueGet(71552168) && kasTimerExpiredById(-2066508739))
    {
        kasfEntrySetValue(71552168, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)74556978);
    }

    if (kasTimerExpiredDestroyById(148613827) && !kasfEntryValueGet(5366435))
    {
        missileSelection = kasGrowSelectionPtr(5366435);
        kasfFindMissilesInside(kasVolumePtr(132933767), missileSelection);
        kasTimerSetById(148613827, 3);
        kasTimerStartById(148613827);

        if (!kasfEntryValueGet(71552169) && kasfShipsAnyInSensorRange(missileSelection))
        {
            kasfEntrySetValue(71552169, 1);
            kasfEntrySetValue(5366435, 1);
            kasfGrowSelectionClear(missileSelection);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)176183300);
        }
    }

    if (!kasfEntryValueGet(5366435))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(5366435));
    }
}

void Init_Mission03_FleetIntel_SetupState(void)
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
    kasfEntryGetOrCreateSetValue(230354293, 0);
    kasfEntryGetOrCreateSetValue(35614828, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission03_FleetIntel_EventFlow);
}

void Watch_Mission03_FleetIntel_CleanupAndResume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
    {
        kasfEntrySetValue(-2054310428, 0);
        kasfSubtitleDraw();
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
            kasfEntrySetValue(127756738, 0);
        }
        if (kasfEntryValueGet(35614828))
        {
            kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
            kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
            kasfCameraSetDistance(kasfEntryValueGet(64272901));
            kasfEntrySetValue(35614828, 0);
        }
        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission03_FleetIntel_ResumeDelay);
    }
}

void Watch_Mission03_FleetIntel_ResumeDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission03_FleetIntel_EventFlow);
    }
}

void Watch_Mission03_FleetIntel_EventFlow(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    void *message;
    GrowSelection *selection;
    GrowSelection *selection2;
    GrowSelection *selection3;
    hvector *point;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(218060896)))
    {
        kasfMissionFailed();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    if (!kasfEntryValueGet(332667))
    {
        while (queueIndex >= 0)
        {
            message = aitQueuePopPointerByIndex(team, queueIndex);
            if (message == (void *)70917812)
            {
                kasfSpeechEventQueueCurrentPlayer(268444028, 0);
                kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_WaitForIntelEventEnd);
                return;
            }
            else if (message == (void *)29474660)
            {
                kasTimerSetById(144337492, 10);
                kasfEntrySetValue(127756738, 1);
                kasfSpeechEventQueueCurrentPlayer(268443919, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)157488868);
                kasfEntryGetOrCreateSetValue(-2065291805, 1);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                return;
            }
            else if (message == (void *)70928681)
            {
                kasfSpeechEventQueueCurrentPlayer(268439819, 0);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                return;
            }
            else if (message == (void *)74556982)
            {
                kasfEntrySetValue(332667, 1);
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119170017);
                kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission03_FleetIntel_ResumeDelay);
                return;
            }
            else if (message == (void *)74556978)
            {
                kasfEntrySetValue(332667, 1);
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenIn(90);
                kasfPingRemove("PingOnEcclesia");
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119169825);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)126123060);
                kasTimerDestroyById(78087284);
                kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission03_FleetIntel_ResumeDelay);
                return;
            }
            else if (message == (void *)74556981)
            {
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfEntrySetValue(332667, 1);
                spEventWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119170001);
                kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission03_FleetIntel_ResumeDelay);
                return;
            }
            else if (message == (void *)144862692)
            {
                if (kasTimerExpiredDestroyById(78087284))
                {
                    kasfSpeechEventQueueCurrentPlayer(268439821, 0);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                    return;
                }
            }
            else if (message == (void *)78087284)
            {
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)173036276);
                kasfPingAddShips(kasAITeamShipsPtrIfExists(134886445), "PingOnEcclesia");
                kasfPingAddShips(kasAITeamShipsPtrIfExists(239669557), "PingOnGenetics");
                kasfSpeechEventQueueCurrentPlayer(268443916, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)78087284);
                kasfMsgSend(kasAITeamPtrIfExists(134886445), (void *)337061);
                kasTimerCreateSetStartById(78087284, 30);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                return;
            }
            else if (message == (void *)86289959)
            {
                kasfSubtitleSimulate(0, 8000, m03_moduleMoving_subtitle[strCurLanguage]);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                return;
            }
            else if (message == (void *)154206436)
            {
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(127756738, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(332667, 1);
                kasfLocationCard(8000, m03_locationCard[strCurLanguage]);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119169809);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                return;
            }
            else if (message == (void *)145180724)
            {
                if (kasfEntryValueGet(71552163))
                {
                    kasfSpeechEventQueueCurrentPlayer(268444031, 0);
                }
                else
                {
                    kasfEntryGetOrCreateSetValue(71552163, 1);
                    kasTimerSetById(78087284, 1);
                    kasTimerStartById(78087284);
                }
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
                return;
            }
            else if (message != NULL)
            {
                aitQueuePointer(team, team, message);
            }

            queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
            queueIndex = (queueHead != NULL) ? *queueHead : -1;
            if (queueIndex >= 0)
            {
                queueIndex--;
            }
        }
        return;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message == (void *)27672097)
        {
            kasfMsgSend(kasAITeamPtrIfExists(239669557), (void *)207154979);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)74556983);
            kasfSpeechEvent(224772420, (sdword)MaintainTeamContext, (sdword)Watch_Mission03_FleetIntel_AdvanceAfterAcknowledge);
            return;
        }
        else if (message == (void *)27668995)
        {
            kasfSpeechEventQueueCurrentPlayer(268468506, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119169828);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x1A63221)
        {
            kasfSetPilotMode(1);
            kasfTutCameraFocusClose((sdword)kasAITeamShipsPtrIfExists(134886445));
            kasfSpeechEventQueueCurrentPlayer(268960129, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119169826);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)74556983)
        {
            selection = kasGrowSelectionPtr(114961833);
            selection2 = kasGrowSelectionPtr(69628553);
            if (kasfShipsSelectType(selection2, selection, "P1IonArrayFrigate"))
            {
                point = kasTeamsVectorPtr(239669557);
                singlePlayerSortShipsByDistanceToPoint(selection2, point);
                selection3 = kasGrowSelectionPtr(21541);
                kasfShipsSelectIndex(selection3, selection2, kasfShipsCount(selection2) - 1);
                kasfTutCameraFocusClose((sdword)selection3);
                kasfCameraSetAngleDeg(201);
                kasfCameraSetDeclinationDeg(-1);
                kasfCameraSetDistance(1767);
            }
            kasfSpeechEventQueueCurrentPlayer(268443924, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)175938357);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x71A6322)
        {
            kasfSpeechEventQueueCurrentPlayer(268960023, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)0x71A6323);
            kasTimerCreateSetStartById(177783838, 3);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x71A6321)
        {
            kasfMusicPlay(22);
            kasfTutCameraFocusClose((sdword)kasAITeamShipsPtrIfExists(134886445));
            kasfCameraSetAngleDeg(27);
            kasfCameraSetDeclinationDeg(-19);
            kasfCameraSetDistance(300);
            kasfSpeechEventQueueCurrentPlayer(268960022, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)0x1A63221);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)111478053)
        {
            kasfEntrySetValue(332667, 0);
            kasfSpeechEventQueueCurrentPlayer(268439817, 0);
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)111478053);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)119169828)
        {
            kasfSetPilotMode(0);
            kasfTutCameraFocus((sdword)kasAITeamShipsPtrIfExists(239669557));
            kasfTutCameraFocus((sdword)kasAITeamShipsPtrIfExists(239669557));
            kasfSpeechEventQueueCurrentPlayer(268452123, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119169841);
            kasfMsgSend(kasAITeamPtrIfExists(134886445), (void *)93055096);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)175938357)
        {
            kasTimerSetById(144337492, 3);
            kasfEntrySetValue(127756738, 1);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(172907219, 1);
            kasfEntrySetValue(332667, 1);
            kasfSpeechEventQueueCurrentPlayer(268443925, 0);
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)175938357);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)119170003)
        {
            kasfSpeechEventQueueCurrentPlayer(268439826, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119170004);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)5801588)
        {
            kasfOtherKASPause();
            kasfUniversePause();
            kasfEntrySetValue(211831571, 1);
            kasfEntrySetValue(109086661, 1);
            if (!kasfSensorsIsOpen(0))
            {
                kasfOpenSensors(150000);
            }
            kasfSensorsSetLookatPoint((sdword)kasShipsVectorPtrIfExists(82209011));
            soundStopDSound("PointerPirates", kasGrowSelectionPtr(82209011));
            kasfSpeechEventQueueCurrentPlayer(268439824, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119170002);
            kasfMsgSend(kasAITeamPtrIfExists(212896802), (void *)5801588);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)119170002)
        {
            selection = kasGrowSelectionPtr(82209011);
            selection2 = kasGrowSelectionPtr(142191106);
            kasfShipsSelectType(selection2, selection, "tHeavyCruiser");
            kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
            kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
            kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
            kasfTutCameraFocusClose((sdword)selection2);
            kasfCameraSetAngleDeg(341);
            kasfCameraSetDeclinationDeg(-5);
            kasfCameraSetDistance(975);
            kasfSpeechEventQueueCurrentPlayer(268697873, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)119170003);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x71A6332)
        {
            kasTimerSetById(144337492, 3);
            kasfEntrySetValue(127756738, 1);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEventQueueCurrentPlayer(268443918, 0);
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)145266441);
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (void *)175727384);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x71A6331)
        {
            kasfTutCameraFocusClose(kasGrowSelectionPtr(218060896));
            kasfCameraSetAngleDeg(243);
            kasfCameraSetDeclinationDeg(-15);
            kasfCameraSetDistance(2400);
            kasfSpeechEventQueueCurrentPlayer(268439936, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)0x71A6332);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x71A63E1)
        {
            selection = kasGrowSelectionPtr(82209011);
            selection2 = kasGrowSelectionPtr(142191106);
            kasfShipsSelectType(selection2, selection, "tHeavyCruiser");
            kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
            kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
            kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
            kasfTutCameraFocusClose((sdword)selection2);
            kasfCameraSetAngleDeg(233);
            kasfCameraSetDeclinationDeg(8);
            kasfCameraSetDistance(975);
            kasfShipsSetNonRetaliation(selection2);
            kasfSpeechEventQueueCurrentPlayer(268697884, 0);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)27672097);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message == (void *)0x71A63D4)
        {
            kasfEntrySetValue(332667, 0);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(172907219, 1);
            kasfSpeechEventQueueCurrentPlayer(268697875, 0);
            kasfEntryGetOrCreateSetValue(-2089704731, 1);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission03_FleetIntel_CleanupAndResume);
            return;
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = (queueHead != NULL) ? *queueHead : -1;
        if (queueIndex >= 0)
        {
            queueIndex--;
        }
    }
}

void Init_Mission03_FleetIntel_Objectives(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message == (void *)0x4A78474)
        {
            kasfObjectiveCreate("Escort", m03_escortCleeSan_brief[strCurLanguage], m03_escortCleeSan_full[strCurLanguage]);
        }
        else if (message == (void *)0x34DBEE4)
        {
            kasfMissionFailed();
        }
        else if (message == (void *)111478053)
        {
            kasfObjectiveCreate("Replace", m03_rebuildHangar_brief[strCurLanguage], m03_rebuildHangar_full[strCurLanguage]);
            kasfEntryGetOrCreateSetValue(-2031765579, 1);
        }
        else if (message == (void *)126123060)
        {
            kasfObjectiveDestroy("Escort");
        }
        else if (message == (void *)145266441)
        {
            kasfObjectiveCreate("Destroy", m03_protectKuunLan_brief[strCurLanguage], m03_protectKuunLan_full[strCurLanguage]);
        }
        else if (message == (void *)175938357)
        {
            kasfObjectiveDestroyAll();
            kasfObjectiveCreate("Hyperspace", m03_hyperspace_brief[strCurLanguage], m03_hyperspace_full[strCurLanguage]);
            spSetSinglePlayerSaveInfo(4, m03_saveInfoTitle[strCurLanguage]);
        }
        else if (message == (void *)0x96316E4)
        {
            kasfObjectiveSet("Research", 1);
        }
        else if (message == (void *)0xA5052F4)
        {
            kasfObjectiveDestroy("Replace");
        }
        else if (message == (void *)0xA796318)
        {
            kasfAllowPlayerToResearch("TECH_ResearchCL");
            kasfObjectiveCreate("Research", m03_researchInfection_brief[strCurLanguage], m03_researchInfection_full[strCurLanguage]);
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = (queueHead != NULL) ? *queueHead : -1;
        if (queueIndex >= 0)
        {
            queueIndex--;
        }
    }
}

void Watch_Mission03_CleeSanPrimaryResponseApproach(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) &&
        !kasfEntryValueGet(4591038))
    {
        kasfEntryGetOrCreateSetValue(4591038, 1);
        kasfFormationWall();
        kasfMoveTo(kasShipsVectorPtrIfExists(218060896));
        kasTimerCreateSetStartById(4591038, 15);
    }
    if (kasfEntryValueGet(4591038) && kasTimerExpiredById(4591038))
    {
        kasfStop();
        kasTimerCreateSetStartById(31738387, 3);
        singlePlayerTeamSpecialToggle();
    }
    if (kasTimerExpiredById(31738387))
    {
        kasfStop();
        kasfSpeechEvent(
            374244,
            (sdword)Mission03_EnterCleeSanPrimaryResponseHoldingPattern,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseHoldingPattern);
    }
}

void Watch_Mission03_CleeSanPrimaryResponseHoldingPattern(void)
{
    sdword currentDistance;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    currentDistance = kasfFindDistance(
        kasTeamsVectorPtr(134850334),
        kasShipsVectorPtrIfExists(218060896));
    if (kasfRandom(0, 9) < 5 ||
        kasfEntryValueGet(243820277) < currentDistance)
    {
        kasfSpeechEvent(
            5769632,
            (sdword)Mission03_ArmCleeSanPrimaryResponseHoldTimer,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseHoldTimer);
    }
    else
    {
        kasfEntrySetValue(243820277, currentDistance);
        kasfSpeechEvent(
            5366435,
            (sdword)Mission03_SendCleeSanPrimaryResponseToApproachVector,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseApproach);
    }
}

void Watch_Mission03_CleeSanPrimaryResponseHoldTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(5769632))
    {
        kasfSpeechEvent(
            374244,
            (sdword)Mission03_EnterCleeSanPrimaryResponseHoldingPattern,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseHoldingPattern);
    }
}

void Watch_Mission03_FleetIntel_AdvanceAfterAcknowledge(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x889C4F4))
    {
        kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission03_FleetIntel_EventFlow);
    }
}

void Watch_Mission03_CleeSanDockedReadyForLaunch(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfPingRemove("PingOnCleeSan");
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)175938357);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)5351060);
        kasfSpeechEvent(342540, (sdword)Mission03_MarkCleeSanLaunchReady, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission03_CleeSanSensorContact(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    if (kasfShipsAnyInSensorRange(&team->shipList))
    {
        if (!kasfEntryValueGet(71552161))
        {
            kasfEntryGetOrCreateSetValue(71552161, 1);
            kasfTeamSwitchPlayerOwner(0);
        }

        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)48761347);
        kasfShipsSelectSinglePlayerCombatShips(&kasGrowSelectionPtr(45992569)->maxNumShips, (sdword)kasGrowSelectionPtr(18336307));
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(89157026),
            kasGrowSelectionPtr(45992569),
            kasThisTeamsVectorPtr(),
            40000);
        kasfShipsSelectNotSalvagers(kasGrowSelectionPtr(89157026), kasGrowSelectionPtr(89157026));
        kasfShipsStop(kasGrowSelectionPtr(89157026));
        kasfShipsSetAttachVelocityFlag(&team->shipList, 0);
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);

        switch (GetGameDifficulty())
        {
            case 0:
                kasfTeamSetPercentDamaged(1000);
                break;
            case 1:
                kasfTeamSetPercentDamaged(500);
                break;
            case 2:
                kasfTeamSetPercentDamaged(100);
                break;
            case 3:
                kasfTeamSetPercentDamaged(80);
                break;
            case 4:
                kasfTeamSetPercentDamaged(50);
                break;
            default:
                break;
        }

        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(316420, (sdword)Mission03_ArmCleeSanThreatTimer, (sdword)Mission03_HandleCleeSanSensorThreats);
    }
}

void Init_Mission03_AssignedWaypointTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(88512316, (sdword)Mission03_SendTeamToAssignedWaypoint, (sdword)Mission03_SendTeamToAssignedTargets);
}

void Watch_Mission03_AssignedWaypointTeam(void)
{
    AITeam *team;
    sdword assignedTeam;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        return;
    }

    if (kasfMsgReceived((char *)362816))
    {
        kasfSpeechEvent(362816, (sdword)kasSpeechEventStopTeam, (sdword)Mission03_RetryAssignedWaypoint);
    }
    else if (!kasfTeamCount())
    {
        assignedTeam = kasfEntryValueGet(-2066494960);
        if ((assignedTeam == 1 && kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A1))) ||
            (assignedTeam == 2 && kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2))) ||
            (assignedTeam == 3 && kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A3))) ||
            (assignedTeam == 4 && kasfThisTeamIs(kasAITeamPtrIfExists(5182116))) ||
            (assignedTeam == 5 && kasfThisTeamIs(kasAITeamPtrIfExists(5182117))) ||
            (assignedTeam == 6 && kasfThisTeamIs(kasAITeamPtrIfExists(5182118))) ||
            (assignedTeam == 7 && kasfThisTeamIs(kasAITeamPtrIfExists(5182119))) ||
            (assignedTeam == 8 && kasfThisTeamIs(kasAITeamPtrIfExists(5182120))))
        {
            kasfEntrySetValue(-2066494960, 0);
        }

        kasfSpeechEvent(342540, (sdword)Mission03_MarkAssignedWaypointHandled, (sdword)MaintainTeamContext);
    }
}

void Mission03_SendTeamToAssignedWaypoint(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(74019713)))
    {
        kasfPatrolPath(kasPathPtr(74019713));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(74019714)))
    {
        kasfPatrolPath(kasPathPtr(74019714));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(74019715)))
    {
        kasfPatrolPath(kasPathPtr(74019715));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A1)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20801));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20802));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A3)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20803));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182116)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20804));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182117)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20805));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182118)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20806));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182119)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20807));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182120)))
    {
        kasfMoveTo(kasVectorPtrIfExists(20808));
    }
}

void Mission03_SendTeamToAssignedTargets(void)
{
   GrowSelection *targets;

   kasCurrentStep = 2;
   scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

   targets = NULL;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(74019713)))
   {
       targets = kasGrowSelectionPtr(98833889);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(74019713), targets);
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(74019714)))
   {
       targets = kasGrowSelectionPtr(98833890);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(74019714), targets);
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(74019715)))
   {
       targets = kasGrowSelectionPtr(98833891);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A1)) || kasfEntryValueGet(-2066494960) == 1)
   {
       targets = kasGrowSelectionPtr(73018681);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)) || kasfEntryValueGet(-2066494960) == 2)
   {
       targets = kasGrowSelectionPtr(74067257);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A3)) || kasfEntryValueGet(-2066494960) == 3)
   {
       targets = kasGrowSelectionPtr(75115833);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182116)) || kasfEntryValueGet(-2066494960) == 4)
   {
       targets = kasGrowSelectionPtr(76164409);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182117)) || kasfEntryValueGet(-2066494960) == 5)
   {
       targets = kasGrowSelectionPtr(77212985);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182118)) || kasfEntryValueGet(-2066494960) == 6)
   {
       targets = kasGrowSelectionPtr(78261561);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182119)) || kasfEntryValueGet(-2066494960) == 7)
   {
       targets = kasGrowSelectionPtr(79310137);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182120)) || kasfEntryValueGet(-2066494960) == 8)
   {
       targets = kasGrowSelectionPtr(80358713);
       kasfFindEnemyShipsInsideVolume(kasVolumePtr(140194110), targets);
       kasfShipsRemove(targets, kasAITeamShipsPtrIfExists(134850334));
   }

   if (targets == NULL || kasfShipsCount(targets) <= 0)
   {
       return;
   }

   kasfShipsSortByDistanceToPoint(targets, (vector *)kasThisTeamsVectorPtr());
   tutDeallocateRootRegion(targets);
   kasfSpeechEvent(74024315, (sdword)Mission03_ResetAssignedWaypointTeam, (sdword)Watch_Mission03_AssignedWaypointTeamIdle);
}

void Mission03_ResetAssignedWaypointTeam(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryScopeName(-2066494960);
    kasfEntryGetOrCreateSetValue(-2066494960, 0);
}

void Watch_Mission03_AssignedWaypointTeamIdle(void)
{
    AITeam *team;
    bool32 shouldReassign;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    if (!kasfEntryValueGet(-2066494960))
    {
        if (kasfShipsSelectType(kasGrowSelectionPtr(0xC04F125), &team->shipList, "P1MissileCorvette") &&
            kasfTeamHealthAverage() <= 70)
        {
            if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A1)))
            {
                kasfEntrySetValue(-2066494960, 1);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 2);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 3);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 4);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 5);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 6);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 7);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)))
            {
                kasfEntrySetValue(-2066494960, 8);
            }
        }
    }

    shouldReassign = FALSE;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(74019713)) && kasfShipsAnyIdle(&team->shipList))
    {
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(74019714)) && kasfShipsAnyIdle(&team->shipList))
    {
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(74019715)) && kasfShipsAnyIdle(&team->shipList))
    {
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A1)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 1)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A2)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 2)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x4F12A3)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 3)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182116)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 4)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182117)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 5)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182118)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 6)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182119)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 7)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(5182120)) && kasfShipsAnyIdle(&team->shipList))
    {
        if (kasfEntryValueGet(-2066494960) == 8)
        {
            kasfEntrySetValue(-2066494960, 0);
        }
        shouldReassign = TRUE;
    }

    if (shouldReassign)
    {
        kasfSpeechEvent(88512316, (sdword)Mission03_SendTeamToAssignedWaypoint, (sdword)Mission03_SendTeamToAssignedTargets);
    }
}

void Mission03_RetryAssignedWaypoint(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)1215))
    {
        kasfSpeechEvent(88512316, (sdword)Mission03_SendTeamToAssignedWaypoint, (sdword)Mission03_SendTeamToAssignedTargets);
    }
}

void Mission03_MarkAssignedWaypointHandled(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfStop();
}

void Init_Mission03_CleeSanSupportTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 0);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasTimerCreateById(21079);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission03_CleeSanSensorContact);
}

void Watch_Mission03_CleeSanSupportTeam(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    char *message;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (char *)aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (char *)5366227)
            {
                kasfEntryGetOrCreateSetValue(5366227, 1);
                kasfEntryGetOrCreateSetValue(5801588, 0);
            }
            else if (message == (char *)5801588)
            {
                kasfEntryGetOrCreateSetValue(5801588, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0)
        {
            break;
        }
        queueIndex--;
    }

    if (kasfEntryValueGet(332667) || kasfTeamCount())
    {
        if (!kasfEntryValueGet(173973911))
        {
            if (kasfNearby(kasShipsVectorPtrIfExists(218060896), 6000))
            {
                kasfSpeechEvent(
                    70808309,
                    (sdword)Mission03_SendSupportTeamToCleeSan,
                    (sdword)Watch_Mission03_CleeSanDockedReadyForLaunch);
            }
        }
    }
    else
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)297300);
        kasfSpeechEvent(342540, (sdword)Mission03_MarkCleeSanLaunchReady, (sdword)MaintainTeamContext);
    }
}

void Mission03_SendSupportTeamToCleeSan(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(173973911, 1);
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(218060896));
}

void Mission03_ArmCleeSanThreatTimer(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    if (kasfEntryValueGet(155461409))
    {
        kasTimerDestroyById(21079);
        kasTimerCreateSetStartById(21079, 60);
    }
    else
    {
        kasfEntryGetOrCreateSetValue(155461409, 1);
        kasTimerSetById(21079, 1);
    }

    if (!kasfEntryValueGet(71552162))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfEntryGetOrCreateSetValue(76825205, 0);
    }
}

void Mission03_HandleCleeSanSensorThreats(void)
{
    GrowSelection *nearbyEnemies;
    GrowSelection *mimics;
    GrowSelection *mcvs;
    GrowSelection *threatShips;
    GrowSelection *nearbyMissiles;
    GrowSelection *missilesToPing;

    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(5801588) && !kasfEntryValueGet(143938327))
    {
        nearbyEnemies = kasGrowSelectionPtr(140194110);
        kasfFindEnemiesNearCurrentTeam(nearbyEnemies, 2000);
        kasfShipsRemove(nearbyEnemies, kasAITeamShipsPtrIfExists(134850334));

        mimics = kasGrowSelectionPtr(149090881);
        kasfShipsSelectType(mimics, nearbyEnemies, "sMimic");
        mcvs = kasGrowSelectionPtr(149090882);
        kasfShipsSelectType(mcvs, nearbyEnemies, "sMCV");

        threatShips = kasGrowSelectionPtr(193867566);
        kasfGrowSelectionClear(threatShips);
        kasfShipsAdd(threatShips, mimics);
        kasfShipsAdd(threatShips, mcvs);

        if (!kasfEntryValueGet(71552163) && kasfShipsCount(threatShips))
        {
            kasfEntryGetOrCreateSetValue(71552163, 1);
            kasfShipsStop(threatShips);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)93086467);
            kasfEntryGetOrCreateSetValue(143938327, 1);
        }

        if (!kasfEntryValueGet(71552164))
        {
            if (kasfShipsCount(nearbyEnemies) > kasfShipsCount(threatShips))
            {
                kasfEntryGetOrCreateSetValue(71552164, 1);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)189009464);
            }
        }
    }

    if (kasfEntryValueGet(5366227))
    {
        nearbyMissiles = kasGrowSelectionPtr(244199198);
        if (kasfFindMissilesNearTeam(nearbyMissiles, 14000))
        {
            missilesToPing = kasGrowSelectionPtr(44846711);
            kasfGrowSelectionClear(missilesToPing);
            kasfShipsAdd(missilesToPing, nearbyMissiles);
            kasfShipsRemove(missilesToPing, kasGrowSelectionPtr(236038147));
            if (kasfShipsCount(missilesToPing))
            {
                kasfPingAddShipsIndexed(missilesToPing, "PingMines");
            }

            kasfShipsAdd(kasGrowSelectionPtr(236038147), nearbyMissiles);
            if (kasTimerExpiredDestroyById(21079))
            {
                kasTimerSetById(21079, 60);
                kasTimerStartById(21079);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)5366435);
            }
        }
        else
        {
            kasfGrowSelectionClear(kasGrowSelectionPtr(236038147));
            kasfPingRemove("PingMines");
            kasfSpeechEvent(1215, (sdword)Mission03_SendResponseTeamToCleeSan, (sdword)Mission03_WatchCleeSanThreatOrLaunch);
        }
    }
}

void Mission03_SendResponseTeamToCleeSan(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(218060896));
}

void Mission03_WatchCleeSanThreatOrLaunch(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfFindMissilesNearTeam(kasGrowSelectionPtr(244199198), 14000))
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)5366435);
        kasfSpeechEvent(316420, (sdword)Mission03_ArmCleeSanThreatTimer, (sdword)Mission03_HandleCleeSanSensorThreats);
    }
    else if (kasfTeamDockedReadyForLaunch())
    {
        kasfEntryGetOrCreateSetValue(173973911, 1);
        kasfPingRemove("PingOnCleeSan");
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)175938357);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)5351060);
        kasfSpeechEvent(342540, (sdword)Mission03_MarkCleeSanLaunchReady, (sdword)MaintainTeamContext);
    }
}

void Mission03_MarkCleeSanLaunchReady(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(173973911, 1);
    kasfEntrySetValue(332667, 1);
}

void Init_Mission03_CleeSanResponseCoordinator(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfEntryGetOrCreateSetValue(-1917543596, 0);
    kasfEntryGetOrCreateSetValue(332667, 0);
}

void Watch_Mission03_CleeSanResponseCoordinator(void)
{
    kasCurrentStep = 1;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        return;
    }

    if (kasfMsgReceived((char *)5801588))
    {
        kasfMusicPlay(20);
        kasfLabelledVectorSet(18135, 0, 0, 0);
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(18135), kasShipsVectorPtrIfExists(218060896), 70000);
        kasfMsgSend(kasAITeamPtrIfExists(74300990), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(197730432), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(197730433), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(197730434), (char *)5801588);
    }

    if (kasfEntryValueGet(-1917543596) == 4)
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)115050772);
        kasfSpeechEvent(342540, (sdword)Mission03_MarkCleeSanResponseActive, (sdword)MaintainTeamContext);
    }
}

void Init_Mission03_CleeSanLeadAttackTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsAdd(kasGrowSelectionPtr(18135), &team->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Mission03_SendCleeSanLeadAttackTeamToApproachVector);
}

void Watch_Mission03_CleeSanLeadAttackTeam(void)
{
    kasCurrentStep = 1;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfEntryIncrement(-1917543596);
        kasfSpeechEvent(342540, (sdword)Mission03_MarkCleeSanResponseActive, (sdword)MaintainTeamContext);
    }
}

void Mission03_StartCleeSanLeadAttackRun(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x4A2A29), (vector *)kasThisTeamsVectorPtr());
    kasfFormationWall();
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}

void Watch_Mission03_CleeSanLeadAttackRun(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(74024315) || kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Mission03_StartCleeSanLeadAttackRun, (sdword)Watch_Mission03_CleeSanLeadAttackRun);
    }
}

void Init_Mission03_CleeSanWingAttackTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsAdd(kasGrowSelectionPtr(18135), &team->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Mission03_SendCleeSanWingAttackTeamToApproachVector);
}

void Mission03_StartCleeSanWingAttackRun(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(74300990)))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(197730432)))
        {
            kasfFormationWall();
        }
        else
        {
            kasfFormationDelta3D();
        }

        kasfProtect(kasAITeamShipsPtrIfExists(74300990));
        kasTimerDestroyById(5019236);
        kasTimerCreateSetStartById(5019236, kasfRandom(10, 20));
    }
    else
    {
        kasfReinforceTeamWithShips(kasAITeamPtrIfExists(74300990), &team->shipList);
        kasfSpeechEvent(342540, (sdword)Mission03_MarkCleeSanResponseActive, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission03_CleeSanWingAttackRun(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(197730432)) ||
        !kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 15000))
    {
        if (kasTimerExpiredById(5019236))
        {
            kasfSpeechEvent(5019236, (sdword)Mission03_StartCleeSanWingAttackRun, (sdword)Watch_Mission03_CleeSanWingAttackRun);
        }
    }
    else
    {
        kasfSpeechEvent(74024315, (sdword)Mission03_RetargetCleeSanWingAttackRun, (sdword)Watch_Mission03_CleeSanWingRetarget);
    }
}

void Mission03_RetargetCleeSanWingAttackRun(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 15000))
    {
        kasfFormationDelta3D();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(57245931), (vector *)kasThisTeamsVectorPtr());
        tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
        kasTimerDestroyById(74024315);
        kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
    }
    else
    {
        kasfSpeechEvent(5019236, (sdword)Mission03_StartCleeSanWingAttackRun, (sdword)Watch_Mission03_CleeSanWingAttackRun);
    }
}

void Watch_Mission03_CleeSanWingRetarget(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(5019236, (sdword)Mission03_StartCleeSanWingAttackRun, (sdword)Watch_Mission03_CleeSanWingAttackRun);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(74024315, (sdword)Mission03_RetargetCleeSanWingAttackRun, (sdword)Watch_Mission03_CleeSanWingRetarget);
    }
}

void Init_Mission03_CleeSanPrimaryResponseTeam(void)
{
    AITeam *team;
    sdword difficulty;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    scope = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    difficulty = GetGameDifficulty();
    if (difficulty == 0)
    {
        kasfShipsSelectFirstN(kasGrowSelectionPtr(59379477), &team->shipList, 4);
        kasfVarDestroy((char *)kasGrowSelectionPtr(59379477));
    }
    else if (difficulty == 1)
    {
        kasfShipsSelectFirstN(kasGrowSelectionPtr(59379477), &team->shipList, 2);
        kasfVarDestroy((char *)kasGrowSelectionPtr(59379477));
    }
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Mission03_SetCleeSanPrimaryResponseApproachVector);
}

void Watch_Mission03_CleeSanPrimaryResponseTeam(void)
{
    kasCurrentStep = 1;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(332667) ||
        !kasfShipsSelectNearby(
            kasGrowSelectionPtr(57245931),
            kasGrowSelectionPtr(0x4A2A29),
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        if (!kasfEntryValueGet(71552165) && !kasfTeamCount())
        {
            kasfEntryGetOrCreateSetValue(71552165, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
    }
    else
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Mission03_StartCleeSanPrimaryResponseAttackRun,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseAttackRun);
    }
}

void Mission03_SetCleeSanPrimaryResponseApproachVector(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntrySetValue(332667, 0);
        kasfLabelledVectorSet(44055618, 0, 0, 0);
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(44055618), kasTeamsVectorPtr(134850334), 50000);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(44055618));
        kasfSpeechEvent(
            5366435,
            (sdword)Mission03_SendCleeSanPrimaryResponseToApproachVector,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseApproach);
    }
}

void Mission03_StartCleeSanPrimaryResponseAttackRun(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (kasfFindMissilesNearTeam(kasGrowSelectionPtr(5366435), 4000) < 10)
    {
        kasfTeamSpecialToggle();
    }
    kasfShipsSelectNearby(
        kasGrowSelectionPtr(57245931),
        kasGrowSelectionPtr(0x4A2A29),
        kasShipsVectorPtr((char *)0x4E20),
        20000);
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(139967715));
    }
    if (kasfShipsCount(kasGrowSelectionPtr(57245931)))
    {
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(57245931), (vector *)kasShipsVectorPtr((char *)0x4E20));
        kasfFormationWall();
        tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
        kasTimerDestroyById(74024315);
        kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
    }
    else
    {
        kasfSpeechEvent(
            374244,
            (sdword)Mission03_EnterCleeSanPrimaryResponseHoldingPattern,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseHoldingPattern);
    }
}

void Watch_Mission03_CleeSanPrimaryResponseAttackRun(void)
{
    kasCurrentStep = 2;
    scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            374244,
            (sdword)Mission03_EnterCleeSanPrimaryResponseHoldingPattern,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseHoldingPattern);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Mission03_StartCleeSanPrimaryResponseAttackRun,
            (sdword)Watch_Mission03_CleeSanPrimaryResponseAttackRun);
    }
}

void Mission03_SendCleeSanLeadAttackTeamToApproachVector(void)
{
   kasCurrentStep = 2;
   scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfMsgReceived((char *)5801588))
   {
       kasfEntrySetValue(332667, 0);
       kasfTeamHyperspaceIn(kasVectorPtrIfExists(18135));
       kasfSpeechEvent(74024315, (sdword)Mission03_StartCleeSanLeadAttackRun, (sdword)Watch_Mission03_CleeSanLeadAttackRun);
   }
}

void Mission03_SendCleeSanWingAttackTeamToApproachVector(void)
{
   kasCurrentStep = 2;
   scope = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfMsgReceived((char *)5801588))
   {
       kasfEntrySetValue(332667, 0);
       kasfTeamHyperspaceIn(kasVectorPtrIfExists(18135));
       kasfSpeechEvent(5019236, (sdword)Mission03_StartCleeSanWingAttackRun, (sdword)Watch_Mission03_CleeSanWingAttackRun);
   }
}

void Init_Mission03(void)
{
    kasCurrentStep = 0;
    scope = KH_mission03;

    kasfMusicPlay(2);
    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));

    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Init_Mission02_FleetIntel_Objectives,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(KH_FleetIntel, Init_Mission02_FleetIntel_SetupState, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(134850334, Init_Mission03_CleeSanSupportTeam, Watch_Mission03_CleeSanSupportTeam,
                 kasAITeamPtrIfExists(134850334));

    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(74019713));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(74019714));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(74019715));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182113));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182114));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182115));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182116));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182117));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182118));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182119));
    kasFSMCreate((sdword)&aisTeams[1].selection[8].sel.ShipPtr[306] + 3,
                 Init_Mission03_AssignedWaypointTeam, Watch_Mission03_AssignedWaypointTeam,
                 kasAITeamPtrIfExists(5182120));

    kasFSMCreate(44055618, Init_Mission03_CleeSanPrimaryResponseTeam, Watch_Mission03_CleeSanPrimaryResponseTeam,
                 kasAITeamPtrIfExists(44055618));
    kasFSMCreate(18135, Init_Mission03_CleeSanResponseCoordinator, Watch_Mission03_CleeSanResponseCoordinator,
                 kasAITeamPtrIfExists(18135));
    kasFSMCreate(74300990, Init_Mission03_CleeSanLeadAttackTeam, Watch_Mission03_CleeSanLeadAttackTeam,
                 kasAITeamPtrIfExists(74300990));
    kasFSMCreate(230461956, Init_Mission03_CleeSanWingAttackTeam, Watch_Mission03_CleeSanLeadAttackTeam,
                 kasAITeamPtrIfExists(197730432));
    kasFSMCreate(230461956, Init_Mission03_CleeSanWingAttackTeam, Watch_Mission03_CleeSanLeadAttackTeam,
                 kasAITeamPtrIfExists(197730433));
    kasFSMCreate(230461956, Init_Mission03_CleeSanWingAttackTeam, Watch_Mission03_CleeSanLeadAttackTeam,
                 kasAITeamPtrIfExists(197730434));

    kasfDisableAllAIFeatures();

    switch (GetGameDifficulty())
    {
        case 0: kasTimerCreateSetStartById(-2066508739, 4800); break;
        case 1: kasTimerCreateSetStartById(-2066508739, 3600); break;
        case 2: kasTimerCreateSetStartById(-2066508739, 3000); break;
        case 3: kasTimerCreateSetStartById(-2066508739, 2700); break;
        case 4: kasTimerCreateSetStartById(-2066508739, 2200); break;
        default: break;
    }

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetCount(0, 3);
    kasfModuleSlotOrderSetShipType(1, "p1Fighter");
    kasfModuleSlotOrderSetShipType(1, "p1MissileCorvette");
    kasfModuleSlotOrderSetShipType(1, "tMinelayerCorvette");
    kasfModuleSlotOrderSetShipType(1, "p1IonArrayFrigate");
    kasfModuleSlotOrderSetCount(1, 3);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipSpecial",    "RACE_SECT", 0);
    kasfEnableShip("sMothershipArmour",     "RACE_SECT", 0);
    kasfEnableShip("sMothershipWeapons",    "RACE_SECT", 0);
    kasfEnableShip("sMothershipMicro",      "RACE_SECT", 1);
    kasfEnableShip("sProcessor",            "RACE_SECT", 1);

    kasfDisableAllAIFeatures();
    kasfDisablePlayerHyperspace();
    kasTimerCreateSetStartById(104338707, 1);
    kasTimerCreateSetStartById(148613827, 1);
    kasfLocationCard(8000, m03_systemName[strCurLanguage]);
}
