/*=============================================================================
    Name    : mission04.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission04
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m04_saveInfoName[] =
{
    "Deep Space (Tel Sector)",
    "French version goes here",
    "German version goes here",
};

void Init_Mission04_EcclesiaLossMonitor(void);
void Watch_Mission04_EcclesiaLossMonitor(void);
void Watch_Mission04_EcclesiaLossMonitor_RecoverDerelict(void);
void Watch_Mission04_EcclesiaAtGeneticsDefenseLoop(void);
void Watch_Mission04_EcclesiaApproachGenetics(void);
void Watch_Mission04_FleetIntel_SetupState(void);
void Init_Mission04_AttackWing(void);
void Watch_Mission04_AttackWing(void);
void Begin_Mission04_AttackRun(void);
void Watch_Mission04_AttackRun(void);
void Watch_Mission04_InfectedEcclesiaDefenseCoordinator(void);
void Init_Mission04_InfectedEcclesiaDefenseAttackDelay(void);
void Watch_Mission04_InfectedEcclesiaDefenseAttackDelay(void);
void Init_Mission04_BeastACVReinforcementTeam(void);
void Watch_Mission04_BeastACVReinforcementTeam(void);
void Init_Mission04_BeastACVReinforcementBuildTimer(void);
void Watch_Mission04_BeastACVReinforcementSpawn(void);
void Begin_Mission04_BeastACVReinforcementCombatLoop(void);
void Watch_Mission04_BeastACVReinforcementCombatLoop(void);
void Begin_Mission04_BeastACVCombatAttackRun(void);
void Watch_Mission04_BeastACVCombatAttackRun(void);
void Watch_Mission04_BeastAttackWaveCoordinator(void);
void Init_Mission04_BeastAttackWaveBuildTimer(void);
void Watch_Mission04_BeastAttackWaveSpawn(void);
void Begin_Mission04_BeastAttackWaveCombatLoop(void);
void Watch_Mission04_BeastAttackWaveCombatLoop(void);
void Init_Mission04_BeastWorkerPool(void);
void Watch_Mission04_BeastWorkerPool(void);
void Watch_Mission04_BeastWorkerPoolCombat(void);
void Watch_Mission04_BeastWorkerPoolResume(void);
void Init_Mission04_BeastWorkerHarvestTeam(void);
void Watch_Mission04_BeastWorkerHarvestTeam(void);
void Watch_Mission04_BeastWorkerHarvestReinforcement(void);
void Watch_Mission04_BeastWorkerReinforcementBudget(void);
void Watch_Mission04_BeastWorkerBuildDelay(void);
void Watch_Mission04_BeastWorkerHarvestLoop(void);
void Watch_Mission04_BeastWorkerCoordinator(void);
void Init_Mission04_BeastAttackWaveCapitalShipLaunch(void);
void Watch_Mission04_BeastAttackWaveCapitalShipLaunch(void);
void Begin_Mission04_BeastAttackWaveCapitalShipCombatLoop(void);
void Watch_Mission04_BeastAttackWaveCapitalShipCombatLoop(void);
void Init_Mission04_EcclesiaAtGenetics(void);
void Mission04_StopInfectedEcclesia(void);
void Init_Mission04_InfectedEcclesiaEscort(void);
void Watch_Mission04_InfectedEcclesiaEscort(void);
void Mission04_PrepareInfectedEcclesiaEscort(void);
void Watch_Mission04_InfectedEcclesiaEscortDefenseLoop(void);
void Watch_Mission04_InfectedEcclesiaHeavyCruiserDefense(void);
void Mission04_ArmEcclesiaAtGeneticsSpeechTimer(void);
void Watch_BeastWorkerRepair_SpecialAttackDamagedCapital(void);

void Watch_Mission04(void)
{
    GrowSelection *allShips;
    GrowSelection *enemyShips;
    GrowSelection *playerShips;
    GrowSelection *dockingBaySelection;
    GrowSelection *supportSelection;
    GrowSelection *linkedShips;
    GrowSelection *workerSelection;
    AITeam *fleetIntelTeam;

    kasCurrentStep = 0;
    kasCurrentScopeHash = 142503668;

    if (kasTimerExpiredDestroyById(104338707))
    {
        allShips = kasGrowSelectionPtr(18336307);
        kasfFindShipsInside(kasVolumePtr(132933767), allShips);

        enemyShips = kasGrowSelectionPtr(114961833);
        kasSelectEnemyShips(enemyShips, allShips);

        playerShips = kasGrowSelectionPtr(0x4A2A29);
        kasSelectPlayerShips(playerShips, allShips);

        kasTimerSetById(104338707, kasfRandom(8, 12));
        kasTimerStartById(104338707);
    }

    if (!kasfEntryValueGet(71552180) && kasfEntryValueGet(-2031765579))
    {
        playerShips = kasGrowSelectionPtr(0x4A2A29);
        dockingBaySelection = kasGrowSelectionPtr(65385317);
        if (kasfShipsSelectType(dockingBaySelection, playerShips, "sMothershipDockingBay"))
        {
            if (!kasCheckShipDockFlag(dockingBaySelection))
            {
                kasfEntrySetValue(71552180, 1);
                fleetIntelTeam = kasAITeamPtrIfExists(176850956);
                kasfMsgSend(fleetIntelTeam, (char *)78087284);
            }
        }
    }

    if (!kasfEntryValueGet(71552181) && kasfEntryValueGet(-2015429404))
    {
        kasfEntrySetValue(71552181, 1);
        fleetIntelTeam = kasAITeamPtrIfExists(176850956);
        kasfMsgSend(fleetIntelTeam, (char *)154206436);
        spSetSinglePlayerSaveInfo(4, m04_saveInfoName[strCurLanguage]);

        allShips = kasGrowSelectionPtr(18336307);
        supportSelection = kasGrowSelectionPtr(89157026);
        kasSelectPlayerShipsSimple(supportSelection, allShips);
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

    if (!kasfEntryValueGet(71552182) && kasfDoesPlayerHave("TECH_ResearchCL"))
    {
        kasfEntrySetValue(71552182, 1);
        fleetIntelTeam = kasAITeamPtrIfExists(176850956);
        kasfMsgSend(fleetIntelTeam, (char *)29474660);
    }

    if (!kasfEntryValueGet(71552183) && kasfEntryValueGet(-2065291805))
    {
        kasfEntrySetValue(71552183, 1);
        kasTimerSetById(82209011, 30);
    }

    if (!kasfEntryValueGet(71552184) && kasTimerExpiredById(82209011))
    {
        kasfEntrySetValue(71552184, 1);
        fleetIntelTeam = kasAITeamPtrIfExists(176850956);
        kasfMsgSend(fleetIntelTeam, (char *)74556981);
    }
}

void Watch_Mission04_TuranicFighters_DefendYourSelf(void)
{
    GrowSelection *nearbyShips;
    GrowSelection *attachedShips;
    GrowSelection *sourceShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552167) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552167, 1);
        kasTimerCreateSetStartById(83004149, 1);
    }
    if (kasTimerExpiredById(83004149))
    {
        sourceShips = kasGrowSelectionPtr(18336307);
        nearbyShips = kasGrowSelectionPtr(114363417);
        kasfShipsSelectNearby(nearbyShips, sourceShips, kasShipsVectorPtr((char *)18336307), 22500);
        kasfShipsRemove(nearbyShips, kasAITeamShipsPtrIfExists(142188961));
        kasfShipsRemove(nearbyShips, kasAITeamShipsPtrIfExists(90274303));
        kasfShipsRemove(nearbyShips, kasGrowSelectionPtr(211966048));
        attachedShips = kasGrowSelectionPtr(37331187);
        kasfShipsSelectAllAttachedShips(attachedShips, kasGrowSelectionPtr(211966048));
        kasfShipsRemove(nearbyShips, attachedShips);
        kasfShipsMakeCrazy(nearbyShips, 0);
        kasfShipsDamage(nearbyShips, 100000);
        kasfShipsMakeCrazy(nearbyShips, 1);
        kasfMsgSendAll((char *)17444564);
    }
}

void Watch_Mission04_InfectedEcclesiaInParade(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSelectType(
        kasGrowSelectionPtr(213176004),
        kasGrowSelectionPtr(114961833),
        "dEcclesiamInfected");
    if (singlePlayerSelectShipsInParade(
            kasGrowSelectionPtr(213176004),
            kasGrowSelectionPtr(213176004)))
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission04_EcclesiaAtGeneticsArrival(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x58BE878))
    {
        kasfSpeechEvent(
            143246580,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission04_InfectedEcclesiaInParade);
    }
}

void Watch_Mission04_MissionBriefing_RUCrazy(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            123839973,
            (sdword)Watch_BeastWorkerRepair_SpecialAttackDamagedCapital,
            0);
    }
}

void Init_Mission04_EcclesiaLossMonitor(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(124185668, 0);
    kasfSpeechEvent(
        374244,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission04_EcclesiaLossMonitor_RecoverDerelict);
}

void Watch_Mission04_EcclesiaLossMonitor(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(124185668) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)70917812);
    }
}

void Watch_Mission04_EcclesiaApproachGenetics(void)
{
    AITeam *team;
    GrowSelection *acolytes;
    GrowSelection *acvs;
    GrowSelection *threatShips;
    GrowSelection *geneticsNearby;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    if (kasTimerExpiredDestroyById(21079))
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)144862692);
        kasTimerSetById(21079, 120);
        kasTimerStartById(21079);
    }

    acolytes = kasGrowSelectionPtr(0x0FB71A95);
    acvs = kasGrowSelectionPtr(0x05041F56);
    threatShips = kasGrowSelectionPtr(0x09AD5044);

    kasfShipsSelectType(acolytes, kasGrowSelectionPtr(0x4A2A29), "sAcolyte");
    kasfShipsSelectType(acvs, kasGrowSelectionPtr(0x4A2A29), "sACV");

    if ((kasfShipsSelectAttackingSelection(threatShips, acolytes, &team->shipList) >= 10 ||
         kasfShipsSelectAttackingSelection(threatShips, acvs, &team->shipList) >= 5) &&
        kasfShipsSelectNearby(threatShips, threatShips, kasThisTeamsVectorPtr(), 6000))
    {
        kasTimerDestroyById(21079);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)145180724);
        kasfSpeechEvent(
            337061,
            (sdword)Mission04_StopInfectedEcclesia,
            (sdword)Watch_Mission04_EcclesiaAtGeneticsDefenseLoop);
    }
    else
    {
        kasfGrowSelectionClear(threatShips);
        kasfShipsAdd(threatShips, acolytes);
        kasfShipsAdd(threatShips, acvs);
        geneticsNearby = kasGrowSelectionPtr(0x52962);
        if (kasfShipsSelectNearby(
                geneticsNearby,
                kasAITeamShipsPtrIfExists(239669557),
                kasThisTeamsVectorPtr(),
                3000))
        {
            kasfShipsSetNonRetaliation(threatShips);
            kasfStop();
            kasfSpeechEvent(
                316420,
                (sdword)Init_Mission04_EcclesiaAtGenetics,
                (sdword)Watch_Mission04_EcclesiaAtGeneticsArrival);
        }
    }
}

void Init_Mission04_EcclesiaAtGenetics(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(124185668, 0);
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)74556978);
}

void Watch_Mission04_EcclesiaLossMonitor_RecoverDerelict(void)
{
    AITeam *team;
    GrowSelection *playerShips;
    GrowSelection *recoveredEcclesia;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    if (!kasfEntryValueGet(71552164) && !kasfTeamCount())
    {
        playerShips = kasGrowSelectionPtr(0x4A2A29);
        recoveredEcclesia = kasGrowSelectionPtr(0x080A342D);
        if (kasfShipsSelectType(recoveredEcclesia, playerShips, "dEcclesiam"))
        {
            kasfEntryGetOrCreateSetValue(71552164, 1);
            kasfReinforceTeamWithShips(team, recoveredEcclesia);
            kasfShipsSetSupportFlag(&team->shipList, 0);
            kasfEntrySetValue(124185668, 1);
        }
    }

    if (kasfMsgReceived((char *)337061))
    {
        kasfSpeechEvent(
            144862692,
            (sdword)Mission04_ArmEcclesiaAtGeneticsSpeechTimer,
            (sdword)Watch_Mission04_EcclesiaApproachGenetics);
    }
}

void Mission04_StopInfectedEcclesia(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfIntercept(kasAITeamShipsPtrIfExists(239669557));
}

void Watch_Mission04_EcclesiaAtGeneticsDefenseLoop(void)
{
    AITeam *team;
    GrowSelection *acolytes;
    GrowSelection *acvs;
    GrowSelection *threatShips;
    GrowSelection *geneticsNearby;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    acolytes = kasGrowSelectionPtr(0x0FB71A95);
    acvs = kasGrowSelectionPtr(0x05041F56);
    threatShips = kasGrowSelectionPtr(0x09AD5044);

    kasfShipsSelectType(acolytes, kasGrowSelectionPtr(0x4A2A29), "sAcolyte");
    kasfShipsSelectType(acvs, kasGrowSelectionPtr(0x4A2A29), "sACV");

    if (kasfShipsSelectAttackingSelection(threatShips, acolytes, &team->shipList) < 10 &&
        kasfShipsSelectAttackingSelection(threatShips, acvs, &team->shipList) < 5 &&
        kasfShipsSelectNearby(threatShips, threatShips, kasThisTeamsVectorPtr(), 6000))
    {
        kasfStop();
        kasfSpeechEvent(
            144862692,
            (sdword)Mission04_ArmEcclesiaAtGeneticsSpeechTimer,
            (sdword)Watch_Mission04_EcclesiaApproachGenetics);
        return;
    }

    kasfGrowSelectionClear(threatShips);
    kasfShipsAdd(threatShips, acolytes);
    kasfShipsAdd(threatShips, acvs);
    geneticsNearby = kasGrowSelectionPtr(0x52962);
    if (kasfShipsSelectNearby(
            geneticsNearby,
            kasAITeamShipsPtrIfExists(239669557),
            kasThisTeamsVectorPtr(),
            3000))
    {
        kasfStop();
        kasfSpeechEvent(
            316420,
            (sdword)Init_Mission04_EcclesiaAtGenetics,
            (sdword)Watch_Mission04_EcclesiaAtGeneticsArrival);
    }
    else if (std_ctype_char_Term())
    {
        kasfSpeechEvent(
            337061,
            (sdword)Mission04_StopInfectedEcclesia,
            (sdword)Watch_Mission04_EcclesiaAtGeneticsDefenseLoop);
    }
}

void Init_Mission04_InfectedEcclesiaEscort(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 0);
    kasfSpeechEvent(
        5801588,
        (sdword)Mission04_PrepareInfectedEcclesiaEscort,
        (sdword)Watch_Mission04_InfectedEcclesiaEscortDefenseLoop);
}

void Watch_Mission04_InfectedEcclesiaEscort(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)55426788);
    }
}

void Mission04_PrepareInfectedEcclesiaEscort(void)
{
   AITeam *team;

   kasCurrentStep = 2;
   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfShipsApplyPlayerUpgrades(kasAITeamShipsPtrIfExists(239669557));
   kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
   kasfShipsSetNonRetaliation(&team->shipList);
}

void Watch_Mission04_InfectedEcclesiaEscortDefenseLoop(void)
{
    AITeam *team;
    GrowSelection *nearbyShips;
    GrowSelection *escortShips;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    nearbyShips = kasGrowSelectionPtr(0x06EDE68D);
    if (kasfShipsSelectNearby(nearbyShips, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 4000) &&
        kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr(0x857BCE3),
            kasGrowSelectionPtr(0x857BCE3),
            nearbyShips))
    {
        kasfShipsAdd(nearbyShips, kasGrowSelectionPtr(0x857BCE3));
        kasfShipsRemove(nearbyShips, kasAITeamShipsPtrIfExists(134886445));
        kasfShipsRemove(nearbyShips, kasGrowSelectionPtr(0x09AD5044));
        if (kasfShipsCount(nearbyShips))
        {
            kasLabelledVectorSet(78212681, kasShipsVectorPtrIfExists(116254349));
            kasLabelledVectorSet(86289039, kasVectorPtrIfExists(78212681));
            kasfVectorSubtractClampToUniverse(kasVectorPtrIfExists(86289039), kasThisTeamsVectorPtr());
            kasfVectorScaleByRatio(kasVectorPtrIfExists(86289039), 1, 4);
            kasfVectorAddClampToUniverse(kasVectorPtrIfExists(78212681), kasVectorPtrIfExists(86289039));
            kasfShipsMoveTo(nearbyShips, kasVectorPtrIfExists(78212681));
            if (!kasfEntryValueGet(71552165))
            {
                kasfEntryGetOrCreateSetValue(71552165, 1);
                kasfEntryGetOrCreateSetValue(70928681, 1);
            }
            if (kasfEntryValueGet(70928681))
            {
                kasTimerCreateSetStartById(70928681, 10);
                kasfEntrySetValue(70928681, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)70928681);
            }
            if (kasTimerExpiredDestroyById(70928681))
            {
                kasfEntrySetValue(70928681, 1);
            }
        }
    }

    if (kasfMsgReceived((char *)0x58BE878))
    {
        kasfShipsSetResourcingFlag((sdword)&team->shipList, 1);
        escortShips = kasGrowSelectionPtr(0x096D1ED9);
        kasfGrowSelectionClear(escortShips);
        kasfShipsAdd(escortShips, kasAITeamShipsPtrIfExists(134886445));
        kasfTeamSpecial(escortShips);
        kasfShipsSetResourcingFlag((sdword)&team->shipList, 0);
        kasfShipsSetRetaliation(&team->shipList);
        kasfMsgSend(kasAITeamPtrIfExists(143195908), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfMsgSend(kasAITeamPtrIfExists(208120516), (char *)4693508);
        kasfSpeechEvent(
            157845977,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission04_InfectedEcclesiaHeavyCruiserDefense);
    }
}

void Watch_Mission04_InfectedEcclesiaDefenseCoordinator(void)
{
    AITeam *team;
    GrowSelection *selection;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    selection = kasGrowSelectionPtr(0x0D1DB174);
    if (kasfShipsSelectType(selection, kasGrowSelectionPtr(0x06DA2DA9), "dEcclesiamInfected") &&
        kasfShipsSelectSinglePlayerInParade(selection, selection))
    {
        kasfReinforceTeamWithShips(team, selection);
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    }

    if (!kasfEntryValueGet(71552173) && kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552173, 1);
        kasfMsgSend(kasAITeamPtrIfExists(218085361), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(218085362), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(218085363), (char *)5801588);

        if (GetGameDifficulty() > 2)
        {
            kasfMsgSend(kasAITeamPtrIfExists(218085364), (char *)5801588);
        }
        else
        {
            if (GetGameDifficulty() == 3)
            {
                kasfMsgSend(kasAITeamPtrIfExists(218085365), (char *)5801588);
            }
            kasfMsgSend(kasAITeamPtrIfExists(218085366), (char *)5801588);
        }

        kasfSpeechEvent(
            175938357,
            (sdword)Init_Mission04_InfectedEcclesiaDefenseAttackDelay,
            (sdword)Watch_Mission04_InfectedEcclesiaDefenseAttackDelay);
    }
}

void Init_Mission04_InfectedEcclesiaDefenseAttackDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(4759657, 60);
}

void Watch_Mission04_InfectedEcclesiaDefenseAttackDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(4759657))
    {
        kasfAttackMothership();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission04_BeastACVReinforcementTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasAITeamShipsPtr(&team->shipList, 0);
}

void Watch_Mission04_BeastACVReinforcementTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntrySetValue(332667, 0);
    }
    if (!kasfEntryValueGet(332667) &&
        kasfUnderAttackElsewhere(kasAITeamPtrIfExists(239669557), kasGrowSelectionPtr(0x857BCE3)) &&
        !kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)Init_Mission04_BeastACVReinforcementBuildTimer,
            (sdword)Watch_Mission04_BeastACVReinforcementSpawn);
    }
}

void Init_Mission04_BeastACVReinforcementBuildTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, kasfRandom(10, 80));
}

void Watch_Mission04_BeastACVReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(4693508))
    {
        kasfStop();
        createdShips = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreateInside(
            createdShips,
            "bACV",
            kasfRandom(8, 15),
            kasAITeamShipsPtrIfExists(239669557),
            1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfReinforceTeamWithShips(team, createdShips);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission04_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission04_BeastACVReinforcementCombatLoop);
    }
}

void Begin_Mission04_BeastACVReinforcementCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationSphere();
    kasfProtect(kasAITeamShipsPtrIfExists(239669557));
    kasTimerDestroyById(0x4C9664);
    kasTimerCreateSetStartById(0x4C9664, kasfRandom(60, 120));
}

void Watch_Mission04_BeastACVReinforcementCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(0x4C9664))
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission04_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission04_BeastACVReinforcementCombatLoop);
    }
    else if (kasfShipsSelectNearby(
                 kasGrowSelectionPtr(0x36980EB),
                 kasGrowSelectionPtr(0x4A2A29),
                 kasTeamsVectorPtr(239669557),
                 15000))
    {
        kasfStop();
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission04_BeastACVCombatAttackRun,
            (sdword)Watch_Mission04_BeastACVCombatAttackRun);
    }
}

void Begin_Mission04_BeastACVCombatAttackRun(void)
{
    GrowSelection *attackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackers = kasGrowSelectionPtr(0x36980EB);
    kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasTeamsVectorPtr(239669557), 15000);
    if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x857BCE3));
    }
    kasfShipsSortByDistanceToPoint(attackers, (vector *)kasThisTeamsVectorPtr());
    kasfFormationClaw();
    tutDeallocateRootRegion(attackers);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission04_BeastACVCombatAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Begin_Mission04_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission04_BeastACVReinforcementCombatLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission04_BeastACVCombatAttackRun,
            (sdword)Watch_Mission04_BeastACVCombatAttackRun);
    }
}

void Watch_Mission04_BeastAttackWaveCoordinator(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    if (kasfMsgReceived((char *)5801588))
    {
        kasfSpeechEvent(
            4693508,
            (sdword)Init_Mission04_BeastAttackWaveBuildTimer,
            (sdword)Watch_Mission04_BeastAttackWaveSpawn);
        return;
    }

    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        if (!kasfEntryValueGet(71552174) && kasfThisTeamIs(kasAITeamPtrIfExists(218085361)))
        {
            kasfEntryGetOrCreateSetValue(71552174, 1);
            kasfMsgSend(kasAITeamPtrIfExists(218085362), (char *)5801588);
        }
        if (!kasfEntryValueGet(71552175) && kasfThisTeamIs(kasAITeamPtrIfExists(218085362)))
        {
            kasfEntryGetOrCreateSetValue(71552175, 1);
            kasfMsgSend(kasAITeamPtrIfExists(218085363), (char *)5801588);
        }
        if (GetGameDifficulty() > 2)
        {
            if (!kasfEntryValueGet(71552176) && kasfThisTeamIs(kasAITeamPtrIfExists(218085363)))
            {
                kasfEntryGetOrCreateSetValue(71552176, 1);
                kasfMsgSend(kasAITeamPtrIfExists(218085364), (char *)5801588);
            }
            if (GetGameDifficulty() > 3)
            {
                if (!kasfEntryValueGet(71552177) && kasfThisTeamIs(kasAITeamPtrIfExists(218085364)))
                {
                    kasfEntryGetOrCreateSetValue(71552177, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(218085365), (char *)5801588);
                }
                if (!kasfEntryValueGet(71552178) && kasfThisTeamIs(kasAITeamPtrIfExists(218085365)))
                {
                    kasfEntryGetOrCreateSetValue(71552178, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(218085366), (char *)5801588);
                }
            }
        }

        kasfSpeechEvent(
            4693508,
            (sdword)Init_Mission04_BeastAttackWaveBuildTimer,
            (sdword)Watch_Mission04_BeastAttackWaveSpawn);
    }
}

void Init_Mission04_BeastAttackWaveBuildTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasTimerCreateSetStartById(4693508, kasfRandom(70, 170));
}

void Watch_Mission04_BeastAttackWaveSpawn(void)
{
    AITeam *team;
    GrowSelection *createdShips;
    char *shipType;
    sdword count;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(4693508))
    {
        kasfStop();
        switch (kasfRandom(1, 6))
        {
            case 1:
            case 2:
            case 3:
                shipType = "bAcolyte";
                count = kasfRandom(8, 16);
                break;
            case 4:
            case 5:
                shipType = "bACV";
                count = kasfRandom(4, 8);
                break;
            case 6:
                shipType = "bRammingFrigate";
                count = 1;
                break;
            default:
                shipType = "bAcolyte";
                count = 8;
                break;
        }

        createdShips = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreateInside(createdShips, shipType, count, kasAITeamShipsPtrIfExists(239669557), 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfReinforceTeamWithShips(team, createdShips);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission04_BeastAttackWaveCombatLoop,
            (sdword)Watch_Mission04_BeastAttackWaveCombatLoop);
    }
}

void Begin_Mission04_BeastAttackWaveCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackers;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    attackers = kasGrowSelectionPtr(0x36980EB);
    if (!kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 30000))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x857BCE3));
    }
    kasfShipsSortByDistanceToPoint(attackers, (vector *)kasThisTeamsVectorPtr());
    if (kasfShipsSelectClass(kasGrowSelectionPtr(0x58), &team->shipList, "CLASS_Fighter"))
    {
        kasfFormationClaw();
    }
    else
    {
        kasfFormationWall();
    }
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x58));
    tutDeallocateRootRegion(attackers);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission04_BeastAttackWaveCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission04_BeastAttackWaveCombatLoop,
            (sdword)Watch_Mission04_BeastAttackWaveCombatLoop);
    }
}

void Init_Mission04_AttackWing(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 0);
    kasfShipsAdd(kasGrowSelectionPtr(0x4E668F3), &team->shipList);
    kasfTeamAttributesBitSet(128);
}

void Watch_Mission04_AttackWing(void)
{
    AITeam *team;
    GrowSelection *hyperspaceWing;
    GrowSelection *paradeFormation;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    if (!kasfEntryValueGet(71552168))
    {
        kasfEntryGetOrCreateSetValue(71552168, 1);
        paradeFormation = kasGrowSelectionPtr(0x5466585);
        kasfGrowSelectionClear(paradeFormation);
        hyperspaceWing = kasGrowSelectionPtr(0x4E668F3);
        kasfShipsAdd(paradeFormation, hyperspaceWing);
        kasfShipsAdd(paradeFormation, kasAITeamShipsPtrIfExists(230765205));
        kasfShipsSetMilitaryParade(kasAITeamShipsPtrIfExists(239669557), paradeFormation);
        kasfShipsHyperspaceIn(hyperspaceWing, kasVectorPtrIfExists(82209011));
    }

    if (kasfMsgReceived((char *)5801588))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(212896802)))
        {
            kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(212896802), 1);
            kasfMsgSend(kasAITeamPtrIfExists(239669557), (char *)82209011);
            kasfMsgSend(kasAITeamPtrIfExists(39987425), (char *)5801588);
            kasfMsgSend(kasAITeamPtrIfExists(39987426), (char *)5801588);
            kasfMsgSend(kasAITeamPtrIfExists(85887061), (char *)5801588);
        }
        kasfSpeechEvent(337061, (sdword)Begin_Mission04_AttackRun, (sdword)Watch_Mission04_AttackRun);
    }
}

void Begin_Mission04_AttackRun(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    if (!kasfEntryValueGet(71552169))
    {
        kasfEntryGetOrCreateSetValue(71552169, 1);
        kasfShipsSetHyperspaceDestination(
            kasGrowSelectionPtr(0x4E668F3),
            kasVectorPtrIfExists(82209011));
    }

    kasfShipsSetNonRetaliation(&team->shipList);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(212896802)))
    {
        kasfIntercept(kasAITeamShipsPtrIfExists(239669557));
    }
    else
    {
        kasfProtect(kasAITeamShipsPtrIfExists(212896802));
        if (kasfThisTeamIs(kasAITeamPtrIfExists(85887061)))
        {
            kasfFormationSphere();
        }
        else
        {
            kasfFormationWall();
        }
    }

    if (kasfTeamCount())
    {
        kasTimerCreateSetStartById(4759657, 8);
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission04_AttackRun(void)
{
    if (!kasfEntryValueGet(71552170) &&
        !kasfShipsCount(kasAITeamShipsPtrIfExists(212896802)))
    {
        kasfEntryGetOrCreateSetValue(71552170, 1);
        kasfIntercept(kasAITeamShipsPtrIfExists(239669557));
    }

    if (kasTimerExpiredById(4759657))
    {
        kasfEntryGetOrCreateSetValue(-1925668823, 1);
    }

    if (!kasfEntryValueGet(71552171) &&
        kasfEntryValueGet(-1925668823) &&
        kasfThisTeamIs(kasAITeamPtrIfExists(212896802)) &&
        kasfNearby((hvector *)kasTeamsVectorPtr(239669557), 6000))
    {
        kasfEntryGetOrCreateSetValue(71552171, 1);
        kasfStop();
    }

    if (!kasfEntryValueGet(71552172) && kasfEntryValueGet(-2111601197))
    {
        kasfEntryGetOrCreateSetValue(71552172, 1);
        kasTimerCreateSetStartById(4905940, 5);
    }
    if (kasTimerExpiredById(4905940))
    {
        kasfStop();
        tutDeallocateRootRegion(kasAITeamShipsPtrIfExists(239669557));
    }
}

void Init_Mission04_BeastWorkerPool(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(
        74024315,
        (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
        (sdword)Watch_Mission04_BeastWorkerPoolCombat);
}

void Watch_Mission04_BeastWorkerPool(void)
{
    AITeam *team;
    GrowSelection *poolShips;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    poolShips = kasGrowSelectionPtr(0x47C97B);
    kasfGrowSelectionClear(poolShips);
    kasfShipsSelectType(kasGrowSelectionPtr(0x7AD3343), kasGrowSelectionPtr(0x6DA2DA9), "bProcessor");
    kasfShipsSelectType(kasGrowSelectionPtr(0xC46FA23), kasGrowSelectionPtr(0x6DA2DA9), "bWorker");
    kasfShipsAdd(poolShips, kasGrowSelectionPtr(0xC46FA23));
    kasfShipsAdd(poolShips, kasGrowSelectionPtr(0x7AD3343));
    if (kasfShipsSelectSinglePlayerInParade(kasGrowSelectionPtr(0x889C4F4), poolShips))
    {
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(0x889C4F4));
    }
}

void Watch_Mission04_BeastWorkerPoolCombat(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&team->shipList))
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
                (sdword)Watch_Mission04_BeastWorkerPoolCombat);
        }
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)Watch_Mission04_BeastWorkerPoolResume);
    }
}

void Watch_Mission04_BeastWorkerPoolResume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
            (sdword)Watch_Mission04_BeastWorkerPoolCombat);
    }
}

void Init_Mission04_BeastWorkerHarvestTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(208120516)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-1893839148, 0);
    kasfEntryGetOrCreateSetValue(-1898467739, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission04_BeastWorkerHarvestReinforcement);
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        208120516,
        (sdword)sub_617A10,
        (sdword)Watch_Mission04_BeastWorkerHarvestLoop);
}

void Watch_Mission04_BeastWorkerHarvestTeam(void)
{
    AITeam *team;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(208120516)) &&
        kasfEntryValueGet(-1898467739) &&
        kasfMsgReceived((char *)4693508))
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
            (sdword)Watch_Mission04_BeastWorkerHarvestReinforcement);
    }
}

void Watch_Mission04_BeastWorkerHarvestReinforcement(void)
{
    AITeam *team;
    GrowSelection *workerShips;
    GrowSelection *supportShips;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    workerShips = kasGrowSelectionPtr(0x58A20);
    supportShips = kasAITeamShipsPtrIfExists(44665972);
    if (kasfShipsSelectType(workerShips, supportShips, "bWorker") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(239669557)))
        {
            if (kasfEntryValueGet(-1893839148))
            {
                kasfEntrySetValue(-1893839148, 0);
                kasfEntrySetValue(-1898467739, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Watch_Mission04_BeastWorkerReinforcementBudget);
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
    if (kasfShipsSelectType(workerShips, kasAITeamShipsPtrIfExists(44665972), "bWorker"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), workerShips);
        kasfShipsStop(workerShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), workerShips);
        kasfShipsSetSlaveableFlag(workerShips, 0);
        kasfShipsApplyPlayerUpgrades(workerShips);
        kasfReinforceTeamWithShips(team, workerShips);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        208120516,
        (sdword)sub_617A10,
        (sdword)Watch_Mission04_BeastWorkerHarvestLoop);
}

void Watch_Mission04_BeastWorkerReinforcementBudget(void)
{
    sdword buildStyle;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 2)
                {
                    buildCost = kasfShipTypeBuildCost("bWorker") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) > 1 &&
                     kasfRUsGet(1) < kasfShipTypeBuildCost("bWorker") / 2) ||
                    kasfEntryValueGet(154639956) == 2)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
                        (sdword)Watch_Mission04_BeastWorkerBuildDelay);
                }
                return;
            case 1:
                if (kasfEntryValueGet(154639956) < 2)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 2)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
                        (sdword)Watch_Mission04_BeastWorkerBuildDelay);
                }
                return;
            default:
                return;
        }
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
    if ((kasfEntryValueGet(154639956) > 1 &&
         kasfRUsGet(1) < kasfShipTypeBuildCost("bWorker")) ||
        kasfEntryValueGet(154639956) == 2)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
            (sdword)Watch_Mission04_BeastWorkerBuildDelay);
    }
}

void Watch_Mission04_BeastWorkerBuildDelay(void)
{
    AITeam *team;
    GrowSelection *capitalShips;
    GrowSelection *launchSelection;
    GrowSelection *newWorkers;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    capitalShips = kasAITeamShipsPtrIfExists(239669557);
    if (!kasfShipsCount(capitalShips))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        launchSelection = kasGrowSelectionPtr(0x5425);
        kasfShipsSelectIndex(
            launchSelection,
            capitalShips,
            kasfRandom(0, kasfShipsCount(capitalShips) - 1));
        newWorkers = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreateInside(
            newWorkers,
            "bWorker",
            kasfEntryValueGet(154639956),
            launchSelection,
            1);
        kasAITeamShipsPtr(newWorkers, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, newWorkers);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            208120516,
            (sdword)sub_617A10,
            (sdword)Watch_Mission04_BeastWorkerHarvestLoop);
    }
}

void Watch_Mission04_BeastWorkerHarvestLoop(void)
{
    AITeam *team;
    GrowSelection *shipBuffer;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfGetWorldResources())
    {
        if (kasfShipsAnyIdle(&team->shipList))
        {
            kasfSpeechEvent(
                208120516,
                (sdword)sub_617A10,
                (sdword)Watch_Mission04_BeastWorkerHarvestLoop);
        }
        return;
    }

    shipBuffer = kasGrowSelectionPtr(0x58A20);
    kasfGrowSelectionClear(shipBuffer);
    kasfShipsAdd(shipBuffer, &team->shipList);
    kasfShipsRemove(&team->shipList, shipBuffer);
    kasfReinforceTeamWithShips(kasAITeamPtrIfExists(44665972), shipBuffer);
    kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
}

void Watch_Mission04_BeastWorkerCoordinator(void)
{
    AITeam *team;
    sdword queueIndex;
    udword message;
    AITeam *targetTeam;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    queueIndex = (team->msgQueue != NULL) ? team->msgQueue->head : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, (udword)queueIndex);
        targetTeam = NULL;
        switch (message)
        {
            case 35692611:
                targetTeam = kasAITeamPtrIfExists(62813829);
                break;
            case 73309284:
                targetTeam = kasAITeamPtrIfExists(145191172);
                break;
            case 104569475:
                targetTeam = kasAITeamPtrIfExists(200156578);
                break;
            case 130586500:
                targetTeam = kasAITeamPtrIfExists(197925396);
                break;
            case 154145491:
                targetTeam = kasAITeamPtrIfExists(182180066);
                break;
            case 158207907:
                targetTeam = kasAITeamPtrIfExists(0x56983E);
                break;
            case 170997699:
                targetTeam = kasAITeamPtrIfExists(58381106);
                break;
            case 188751475:
                targetTeam = kasAITeamPtrIfExists(212896882);
                break;
            case 190169075:
                targetTeam = kasAITeamPtrIfExists(171501548);
                break;
            case 202253283:
                targetTeam = kasAITeamPtrIfExists(96759714);
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

        if (team->msgQueue != NULL && team->msgQueue->head)
        {
            if (--queueIndex >= 0)
            {
                continue;
            }
        }
        return;
    }
}

void Watch_Mission04_FleetIntel_SetupState(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552162) && kasTimerExpiredById(177783838))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfMsgSend(kasAITeamPtrIfExists(239669557), (char *)93055096);
    }
}

void Init_Mission04_BeastAttackWaveCapitalShipLaunch(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(-2083292155);
    kasTimerSetById(-2083292155, kasfRandom(30, 50));
    kasfEntryGetOrCreateSetValue(332667, 1);
}

void Watch_Mission04_BeastAttackWaveCapitalShipLaunch(void)
{
    AITeam *team;
    AITeam *capitalShipTeam;
    GrowSelection *teamShips;

    kasCurrentStep = 1;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    teamShips = kasGrowSelectionPtr(0x593584);
    capitalShipTeam = kasAITeamPtrIfExists(251977221);

    if (kasfShipsSelectType(teamShips, kasGrowSelectionPtr(0x6DA2DA9), "bHeavyCruiser"))
    {
        if (kasfShipsSelectSinglePlayerInParade(teamShips, teamShips))
        {
            kasfReinforceTeamWithShips(capitalShipTeam, teamShips);
            kasfShipsAdd(teamShips, kasAITeamShipsPtrIfExists(239669557));
            kasfShipsSetFormation(teamShips, 8);
            kasfTacticsAggressive();
            kasTimerSetById(-2083292155, 1);
            kasTimerStartById(-2083292155);
            kasfMsgSendAll((char *)178743493);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)143246580);
        }
    }

    if (kasfShipsSelectClass(teamShips, kasGrowSelectionPtr(0x6DA2DA9), "CLASS_Frigate"))
    {
        if (kasfShipsSelectSinglePlayerInParade(teamShips, teamShips))
        {
            kasfReinforceTeamWithShips(capitalShipTeam, teamShips);
            kasfShipsAdd(teamShips, kasAITeamShipsPtrIfExists(239669557));
            kasfShipsSetFormation(teamShips, 8);
            kasfTacticsAggressive();
            kasTimerStartById(-2083292155);
        }
    }

    if (!kasfThisTeamIs(capitalShipTeam) &&
        !kasfTeamCount() &&
        kasfShipsCount(kasAITeamShipsPtrIfExists(251977221)) >= 1 &&
        kasfShipsSelectFirstN(
            teamShips,
            kasAITeamShipsPtrIfExists(251977221),
            kasfShipsCount(kasAITeamShipsPtrIfExists(251977221))) &&
        (kasTimerExpiredDestroyById(-2083292155) || kasfMsgReceived((char *)0xAA768C5)))
    {
        kasTimerSetById(-2083292155, kasfRandom(30, 50));
        kasfShipsRemove(kasAITeamShipsPtrIfExists(251977221), teamShips);
        kasfReinforceTeamWithShips(team, teamShips);
        kasfTacticsNeutral();
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission04_BeastAttackWaveCapitalShipCombatLoop,
            (sdword)Watch_Mission04_BeastAttackWaveCapitalShipCombatLoop);
    }
    else if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfMsgReceived((char *)0x469857B))
            {
                kasfSpeechEvent(
                    74024315,
                    (sdword)Begin_Mission04_BeastAttackWaveCapitalShipCombatLoop,
                    (sdword)Watch_Mission04_BeastAttackWaveCapitalShipCombatLoop);
            }
        }
        else
        {
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
    }
}

void Begin_Mission04_BeastAttackWaveCapitalShipCombatLoop(void)
{
    GrowSelection *attackers;
    GrowSelection *nearbyTargets;
    hvector *teamVector;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    teamVector = kasThisTeamsVectorPtr();
    nearbyTargets = kasGrowSelectionPtr(0x5296679);
    attackers = kasGrowSelectionPtr(0x36980EB);
    if (kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr(0x4A2A29), teamVector, 15000))
    {
        if (!kasfShipsSelectCapital(attackers, nearbyTargets))
        {
            kasfShipsAdd(attackers, nearbyTargets);
        }
    }
    else
    {
        kasfGrowSelectionClear(attackers);
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
    }

    if (kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x857BCE3));
    }
    kasfFormationWall();
    kasfShipsSelectClass(kasGrowSelectionPtr(0xADBD8E2), attackers, "CLASS_Fighter");
    kasfShipsRemove(attackers, kasGrowSelectionPtr(0xADBD8E2));
    kasfShipsSortByDistanceToPoint(attackers, (vector *)teamVector);
    tutDeallocateRootRegion(attackers);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission04_BeastAttackWaveCapitalShipCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission04_BeastAttackWaveCapitalShipCombatLoop,
            (sdword)Watch_Mission04_BeastAttackWaveCapitalShipCombatLoop);
    }
}

void Watch_Mission04_InfectedEcclesiaHeavyCruiserDefense(void)
{
    AITeam *team;
    GrowSelection *groupShips;
    GrowSelection *heavyCruisers;
    GrowSelection *nearbyTargets;
    GrowSelection *specialTargets;
    bool32 shouldSpecial;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;

    groupShips = kasGrowSelectionPtr(82209011);
    heavyCruisers = kasGrowSelectionPtr(142191106);
    nearbyTargets = kasGrowSelectionPtr(158146265);
    specialTargets = kasGrowSelectionPtr(228170515);

    if (kasfMsgReceived((char *)207154979))
    {
        kasfShipsSetResourcingFlag((sdword)&team->shipList, 1);
        kasfShipsSelectType(heavyCruisers, groupShips, "tHeavyCruiser");
        kasfTeamSpecial(heavyCruisers);
        kasfShipsSetResourcingFlag((sdword)&team->shipList, 0);
        kasfEntryGetOrCreateSetValue(-2111601197, 1);
        kasTimerCreateSetStartById(74016921, 8);
    }
    if (kasTimerExpiredById(74016921))
    {
        kasfSpeechEventQueueCurrentPlayer(268697987, 0);
    }

    kasfShipsSelectType(heavyCruisers, groupShips, "tHeavyCruiser");
    if (!kasfEntryValueGet(71552166) &&
        kasfEntryValueGet(-2089704731) &&
        kasfShipsSelectNearby(kasGrowSelectionPtr(186676021), heavyCruisers, kasThisTeamsVectorPtr(), 6000))
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)74556982);
    }

    shouldSpecial = FALSE;
    if (kasfShipsAnyIdle(&team->shipList) &&
        kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        shouldSpecial = TRUE;
    }
    else if (kasfEntryValueGet(-2111601197) &&
             kasfShipsSelectNearby(nearbyTargets, groupShips, kasThisTeamsVectorPtr(), 20000))
    {
        shouldSpecial = TRUE;
    }

    if (shouldSpecial)
    {
        if (kasfShipsSelectMembersOfSelection(specialTargets, nearbyTargets, groupShips))
        {
            kasfShipsSetResourcingFlag((sdword)&team->shipList, 1);
        }
        kasfTeamSpecial(nearbyTargets);
        kasfShipsSetResourcingFlag((sdword)&team->shipList, 0);
    }
}

extern void Watch_Mission14_FleetIntel_FISensorTechLaunchPrep(void);
extern void Watch_Mission09_Traders_TalkingMoreLaunchPrep(void);

void Init_Mission04(void)
{
    kasCurrentStep = 0;
    scope = KH_mission04;

    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));
    kasfMusicPlay(3);
    kasfActivateTech("TECH_INFECT");

    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Init_Mission03_FleetIntel_Objectives,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(KH_FleetIntel, Init_Mission03_FleetIntel_SetupState, Watch_Mission04_FleetIntel_SetupState,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(134886445, Init_Mission04_EcclesiaLossMonitor, Watch_Mission04_EcclesiaLossMonitor,
                 kasAITeamPtrIfExists(134886445));
    kasFSMCreate(44665972, Init_Mission04_BeastWorkerPool, Watch_Mission04_BeastWorkerPool,
                 kasAITeamPtrIfExists(44665972));
    kasFSMCreate(239669557, Init_Mission04_InfectedEcclesiaEscort, Watch_Mission04_InfectedEcclesiaEscort,
                 kasAITeamPtrIfExists(239669557));
    kasFSMCreate(143195908, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastACVReinforcementTeam,
                 kasAITeamPtrIfExists(143195908));

    kasFSMCreate(164625269, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastAttackWaveCoordinator,
                 kasAITeamPtrIfExists(218085361));
    kasFSMCreate(164625269, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastAttackWaveCoordinator,
                 kasAITeamPtrIfExists(218085362));
    kasFSMCreate(164625269, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastAttackWaveCoordinator,
                 kasAITeamPtrIfExists(218085363));
    kasFSMCreate(164625269, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastAttackWaveCoordinator,
                 kasAITeamPtrIfExists(218085364));
    kasFSMCreate(164625269, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastAttackWaveCoordinator,
                 kasAITeamPtrIfExists(218085365));
    kasFSMCreate(164625269, Init_Mission04_BeastACVReinforcementTeam, Watch_Mission04_BeastAttackWaveCoordinator,
                 kasAITeamPtrIfExists(218085366));

    kasFSMCreate(246591522, Init_Mission14_FleetIntel_FISensorTechLaunchPrep, Watch_Mission14_FleetIntel_FISensorTechLaunchPrep,
                 kasAITeamPtrIfExists(246591522));
    kasFSMCreate(246591522, Init_Mission14_FleetIntel_FISensorTechLaunchPrep, Watch_Mission14_FleetIntel_FISensorTechLaunchPrep,
                 kasAITeamPtrIfExists(187368145));
    kasFSMCreate(246591522, Init_Mission14_FleetIntel_FISensorTechLaunchPrep, Watch_Mission14_FleetIntel_FISensorTechLaunchPrep,
                 kasAITeamPtrIfExists(187368146));
    kasFSMCreate(246591522, Init_Mission14_FleetIntel_FISensorTechLaunchPrep, Watch_Mission14_FleetIntel_FISensorTechLaunchPrep,
                 kasAITeamPtrIfExists(187368147));

    kasFSMCreate(49029573, Init_Mission09_Traders_TalkingMoreLaunchPrep, Watch_Mission09_Traders_TalkingMoreLaunchPrep,
                 kasAITeamPtrIfExists(49029573));
    kasFSMCreate(49029573, Init_Mission09_Traders_TalkingMoreLaunchPrep, Watch_Mission09_Traders_TalkingMoreLaunchPrep,
                 kasAITeamPtrIfExists(247602241));
    kasFSMCreate(49029573, Init_Mission09_Traders_TalkingMoreLaunchPrep, Watch_Mission09_Traders_TalkingMoreLaunchPrep,
                 kasAITeamPtrIfExists(247602242));
    kasFSMCreate(49029573, Init_Mission09_Traders_TalkingMoreLaunchPrep, Watch_Mission09_Traders_TalkingMoreLaunchPrep,
                 kasAITeamPtrIfExists(247602243));
    kasFSMCreate(49029573, Init_Mission09_Traders_TalkingMoreLaunchPrep, Watch_Mission09_Traders_TalkingMoreLaunchPrep,
                 kasAITeamPtrIfExists(247602244));

    kasFSMCreate(251977221, Init_Mission04_BeastAttackWaveCapitalShipLaunch, Watch_Mission04_BeastAttackWaveCapitalShipLaunch,
                 kasAITeamPtrIfExists(251977221));
    kasFSMCreate(251977221, Init_Mission04_BeastAttackWaveCapitalShipLaunch, Watch_Mission04_BeastAttackWaveCapitalShipLaunch,
                 kasAITeamPtrIfExists(5103761));
    kasFSMCreate(251977221, Init_Mission04_BeastAttackWaveCapitalShipLaunch, Watch_Mission04_BeastAttackWaveCapitalShipLaunch,
                 kasAITeamPtrIfExists(5103762));
    kasFSMCreate(251977221, Init_Mission04_BeastAttackWaveCapitalShipLaunch, Watch_Mission04_BeastAttackWaveCapitalShipLaunch,
                 kasAITeamPtrIfExists(5103763));
    kasFSMCreate(251977221, Init_Mission04_BeastAttackWaveCapitalShipLaunch, Watch_Mission04_BeastAttackWaveCapitalShipLaunch,
                 kasAITeamPtrIfExists(5103764));
    kasFSMCreate(251977221, Init_Mission04_BeastAttackWaveCapitalShipLaunch, Watch_Mission04_BeastAttackWaveCapitalShipLaunch,
                 kasAITeamPtrIfExists(5103765));

    kasFSMCreate(208120516, Init_Mission04_BeastWorkerHarvestTeam, Watch_Mission04_BeastWorkerHarvestTeam,
                 kasAITeamPtrIfExists(208120516));
    kasFSMCreate(208120516, Init_Mission04_BeastWorkerHarvestTeam, Watch_Mission04_BeastWorkerHarvestTeam,
                 kasAITeamPtrIfExists(108702865));
    kasFSMCreate(208120516, Init_Mission04_BeastWorkerHarvestTeam, Watch_Mission04_BeastWorkerHarvestTeam,
                 kasAITeamPtrIfExists(108702866));
    kasFSMCreate(208120516, Init_Mission04_BeastWorkerHarvestTeam, Watch_Mission04_BeastWorkerHarvestTeam,
                 kasAITeamPtrIfExists(108702867));

    kasFSMCreate(4627076, Watch_NoOpTeamContext, Watch_Mission04_BeastWorkerCoordinator,
                 kasAITeamPtrIfExists(4627076));
    kasFSMCreate(144552308, Watch_NoOpTeamContext, Watch_Mission04_InfectedEcclesiaDefenseCoordinator,
                 kasAITeamPtrIfExists(144552308));

    kasFSMCreate(82209011, Init_Mission04_AttackWing, Watch_Mission04_AttackWing,
                 kasAITeamPtrIfExists(212896802));
    kasFSMCreate(82209011, Init_Mission04_AttackWing, Watch_Mission04_AttackWing,
                 kasAITeamPtrIfExists(39987425));
    kasFSMCreate(82209011, Init_Mission04_AttackWing, Watch_Mission04_AttackWing,
                 kasAITeamPtrIfExists(39987426));
    kasFSMCreate(82209011, Init_Mission04_AttackWing, Watch_Mission04_AttackWing,
                 kasAITeamPtrIfExists(85887061));

    kasfRUsSet(1, 20000);
    kasfDisableAllAIFeatures();

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");
    kasfAllowPlayerToResearch("TECH_MISSILE");

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetCount(0, 3);
    kasfModuleSlotOrderSetShipType(1, "p1Fighter");
    kasfModuleSlotOrderSetShipType(1, "p1MissileCorvette");
    kasfModuleSlotOrderSetShipType(1, "p1IonArrayFrigate");
    kasfModuleSlotOrderSetCount(1, 2);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipSpecial",    "RACE_SECT", 0);
    kasfEnableShip("sMothershipArmour",     "RACE_SECT", 0);
    kasfEnableShip("sMothershipWeapons",    "RACE_SECT", 1);
    kasfEnableShip("sMothershipMicro",      "RACE_SECT", 1);

    kasfDisablePlayerHyperspace();
    kasTimerCreateSetStartById(104338707, 1);
}
