/*=============================================================================
    Name    : mission14.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission14
=============================================================================*/

#include "MissionScriptSupport.h"
void Init_BeastCorvetteReinforcementTeam(void);
void Init_BeastCorvetteReinforcementTeam_Reevaluate(void);
void Watch_BeastCorvetteReinforcementTeam(void);
void Init_BeastCorvetteReinforcementBuildTimer(void);
void Watch_BeastCorvetteReinforcementBuildTimer(void);
void Watch_BeastCorvetteReinforcementLaunchTimer(void);
void Init_BeastCorvetteCapitalStrike(void);
void Begin_BeastCorvetteCombatLoop(void);
void Watch_BeastCorvetteCombatLoop(void);

void Init_BeastAcolyteReinforcementTeam(void);
void Init_BeastAcolyteReinforcementTeam_Reevaluate(void);
void Watch_BeastAcolyteReinforcementTeam(void);
void Init_BeastAcolyteReinforcementBuildTimer(void);
void Watch_BeastAcolyteReinforcementBuildTimer(void);
void Watch_BeastAcolyteReinforcementLaunchTimer(void);
void Init_BeastAcolyteCapitalStrike(void);
void Watch_BeastAcolyteCapitalStrike(void);
void Begin_BeastAcolyteCombatLoop(void);
void Watch_BeastAcolyteCombatLoop(void);

void Init_BeastRammingFrigateReinforcementTeam(void);
void Init_BeastRammingFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastRammingFrigateReinforcementTeam(void);
void Init_BeastRammingFrigateReinforcementBuildTimer(void);
void Watch_BeastRammingFrigateReinforcementBuildTimer(void);
void Begin_BeastRammingFrigateCombatLoop(void);
void Watch_BeastRammingFrigateCombatLoop(void);

void Init_BeastIonArrayFrigateReinforcementTeam(void);
void Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastIonArrayFrigateReinforcementTeam(void);
void Watch_BeastIonArrayFrigateHyperspaceAttack(void);
void Watch_BeastIonArrayFrigateCombatLoop(void);
void Begin_BeastIonArrayFrigateAttackRun(void);
void Watch_BeastIonArrayFrigateAttackRun(void);

void Init_BeastCloakedFighterReinforcementTeam(void);
void Init_BeastCloakedFighterReinforcementTeam_Reevaluate(void);
void Watch_BeastCloakedFighterReinforcementTeam(void);
void Begin_BeastCloakedFighterCombatLoop(void);
void Watch_BeastCloakedFighterCombatLoop(void);
void Begin_BeastCloakedFighterAttackRun(void);
void Watch_BeastCloakedFighterAttackRun(void);

void Init_BeastMultiBeamFrigateReinforcementTeam(void);
void Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastMultiBeamFrigateReinforcementTeam(void);
void Begin_BeastMultiBeamFrigateCombatLoop(void);
void Watch_BeastMultiBeamFrigateCombatLoop(void);
void Begin_HiveFrigateReinforcementBuildTimer(void);
void Begin_HiveFrigateCombatLoop(void);
void Watch_Mission14_HiveFrigateReinforcementTeam(void);
void Watch_Mission14_HiveFrigateReinforcementBuildCount(void);
void Watch_Mission14_HiveFrigateReinforcementSpawn(void);
void Watch_Mission14_HiveFrigateCombatLoop(void);
void Init_Mission14_ReinforcementTriggerRouter_Delay(void);
void Watch_Mission14_ReinforcementTriggerRouter_Delay(void);
void Watch_Mission14_ReinforcementTriggerRouter(void);
void Begin_Mission14_MultigunCorvetteReinforcementSpawn(void);
void Watch_Mission14_MultigunCorvetteReinforcementSpawn(void);
void Init_Mission14_ReinforcementGroup_SelectCapitalTargets(void);
void Init_Mission14_ReinforcementGroup_AttackAssignedWing(void);
void Init_Mission14_ReinforcementBuildClosestTargetSelection(void);
void Init_Mission14_ReinforcementAttackMothershipDeleteTimer(void);
void Init_Mission14_ReinforcementFocusTrackedTarget(void);
void Begin_Mission14_FleetIntel_FISensorTechLaunch(void);
void Watch_Mission14_FleetIntel_FISensorTechLaunch(void);
void Begin_Mission14_FleetIntel_FISensorTechLaunchHold(void);
void Init_Mission14_MissionFlow(void);
void Watch_Mission14_MissionFlow_PostIntelCleanup(void);
void Watch_Mission14_MissionFlow_PostIntelDelay(void);
void Init_Mission14_TaiidanReinforcementReset(void);
void Begin_Mission14_TaiidanReinforcementSpawn(void);
void Begin_Mission14_TaiidanReinforcementAttackRun(void);
void Begin_Mission14_TaiidanReinforcementResetDelay(void);
void Init_Mission14_ReinforcementEscortTeam_Regroup(void);
void Init_AttackMothershipDeleteDelay(void);
void Begin_Mission12_SixShipFormationTeam_AttackMothership(void);
void Init_WaitForIntelEventEnd(void);
void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void);
void Init_Mission11_MissionFlow_Delay(void);
void Init_Mission11_SentinelReinforcementBuildTimer(void);
void Init_Mission16_BeastACVReinforcementBuildTimer(void);
void Init_Mission09_AssaultEscort_Cooldown(void);
void Begin_SentinelCombatRetryDelay(void);
void Begin_BeastACVCombatLoopDelay(void);
void Begin_Mission14_BeastACVDockAndReinforce(void);
void Watch_Mission14_BeastACVDockAndReinforce(void);
void Init_Mission14_BeastACVReinforcementTeam(void);
void Init_Mission14_BeastACVReinforcementTeam_Reevaluate(void);
void Watch_Mission14_BeastACVReinforcementTeam(void);
void Watch_Mission14_BeastACVReinforcementBuildCount(void);
void Watch_Mission14_BeastACVReinforcementSpawn(void);
void Begin_Mission14_BeastACVReinforcementCombatLoop(void);
void Watch_Mission14_BeastACVReinforcementCombatLoop(void);
void Begin_Mission14_BeastACVCombatAttackRun(void);
void Watch_Mission14_BeastACVCombatRetryDelay(void);
void FailMissionIfTrackedSelectionAlive(void);
void Begin_Mission14_SentinelReinforcementCombatLoop(void);
void Watch_Mission14_SentinelReinforcementCombatLoop(void);
void Watch_Mission14_SentinelReinforcementTeam(void);
void Watch_Mission14_SentinelReinforcementBuildCount(void);
void Watch_Mission14_SentinelReinforcementSpawn(void);
void Watch_Mission14_SentinelCombatRetryDelay(void);
void Init_Mission14_ReinforcementPositionAtAttackVector(void);
void Watch_Mission14_ReinforcementMaintainAttackVectorSelection(void);
void kasfIssueSpecialAttack(GrowSelection *targets);
void kasfShipsSetClamped(GrowSelection *ships, bool32 clamped);
void Init_Mission14_ReinforcementTriggerRouter_StopAndClearCapitalSelection(void);

#define Init_Mission14_BeastCorvetteReinforcementTeam Init_BeastCorvetteReinforcementTeam
#define Init_Mission14_BeastCorvetteReinforcementTeam_Reevaluate Init_BeastCorvetteReinforcementTeam_Reevaluate
#define Watch_Mission14_BeastCorvetteReinforcementTeam Watch_BeastCorvetteReinforcementTeam
#define Init_Mission14_BeastCorvetteReinforcementBuildTimer Init_BeastCorvetteReinforcementBuildTimer
#define Watch_Mission14_BeastCorvetteReinforcementBuildTimer Watch_BeastCorvetteReinforcementBuildTimer
#define Begin_Mission14_BeastCorvetteCombatLoop Begin_BeastCorvetteCombatLoop
#define Watch_Mission14_BeastCorvetteCombatLoop Watch_BeastCorvetteCombatLoop
#define Watch_Mission14_BeastCorvetteReinforcementLaunchTimer Watch_BeastCorvetteReinforcementLaunchTimer
#define Init_Mission14_BeastCorvetteCapitalStrike Init_BeastCorvetteCapitalStrike

#define Init_Mission14_BeastAcolyteReinforcementTeam Init_BeastAcolyteReinforcementTeam
#define Init_Mission14_BeastAcolyteReinforcementTeam_Reevaluate Init_BeastAcolyteReinforcementTeam_Reevaluate
#define Watch_Mission14_BeastAcolyteReinforcementTeam Watch_BeastAcolyteReinforcementTeam
#define Init_Mission14_BeastAcolyteReinforcementBuildTimer Init_BeastAcolyteReinforcementBuildTimer
#define Watch_Mission14_BeastAcolyteReinforcementBuildTimer Watch_BeastAcolyteReinforcementBuildTimer
#define Begin_Mission14_BeastAcolyteCombatLoop Begin_BeastAcolyteCombatLoop
#define Watch_Mission14_BeastAcolyteCombatLoop Watch_BeastAcolyteCombatLoop
#define Watch_Mission14_BeastAcolyteReinforcementLaunchTimer Watch_BeastAcolyteReinforcementLaunchTimer
#define Watch_Mission14_BeastAcolyteCombatAttackRun Watch_BeastAcolyteCapitalStrike
#define Init_Mission14_BeastAcolyteCapitalStrike Init_BeastAcolyteCapitalStrike

#define Init_Mission14_BeastRammingFrigateReinforcementTeam Init_BeastRammingFrigateReinforcementTeam
#define Init_Mission14_BeastRammingFrigateReinforcementTeam_Reevaluate Init_BeastRammingFrigateReinforcementTeam_Reevaluate
#define Watch_Mission14_BeastRammingFrigateReinforcementTeam Watch_BeastRammingFrigateReinforcementTeam
#define Init_Mission14_BeastRammingFrigateReinforcementBuildTimer Init_BeastRammingFrigateReinforcementBuildTimer
#define Watch_Mission14_BeastRammingFrigateReinforcementBuildTimer Watch_BeastRammingFrigateReinforcementBuildTimer
#define Watch_Mission14_BeastRammingFrigateCombatLoop Watch_BeastRammingFrigateCombatLoop

#define Init_Mission14_BeastIonArrayFrigateReinforcementTeam Init_BeastIonArrayFrigateReinforcementTeam
#define Init_Mission14_BeastIonArrayFrigateReinforcementTeam_Reevaluate Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate
#define Watch_Mission14_BeastIonArrayFrigateReinforcementTeam Watch_BeastIonArrayFrigateReinforcementTeam
#define Watch_Mission14_BeastIonArrayFrigateReinforcementCombatLoop Watch_BeastIonArrayFrigateCombatLoop
#define Begin_Mission14_BeastIonArrayFrigateCombatAttackRun Begin_BeastIonArrayFrigateAttackRun
#define Watch_Mission14_BeastIonArrayFrigateCombatAttackRun Watch_BeastIonArrayFrigateAttackRun
#define Watch_Mission14_BeastIonArrayFrigateHyperspaceAttack Watch_BeastIonArrayFrigateHyperspaceAttack

#define Init_Mission14_BeastCloakedFighterReinforcementTeam Init_BeastCloakedFighterReinforcementTeam
#define Init_Mission14_BeastCloakedFighterReinforcementTeam_Reevaluate Init_BeastCloakedFighterReinforcementTeam_Reevaluate
#define Watch_Mission14_BeastCloakedFighterReinforcementTeam Watch_BeastCloakedFighterReinforcementTeam
#define Watch_Mission14_BeastCloakedFighterReinforcementCombatLoop Watch_BeastCloakedFighterCombatLoop
#define Begin_Mission14_BeastCloakedFighterCombatAttackRun Begin_BeastCloakedFighterAttackRun
#define Watch_Mission14_BeastCloakedFighterCombatAttackRun Watch_BeastCloakedFighterAttackRun

#define Init_Mission14_BeastMultiBeamFrigateReinforcementTeam Init_BeastMultiBeamFrigateReinforcementTeam
#define Init_Mission14_BeastMultiBeamFrigateReinforcementTeam_Reevaluate Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate
#define Watch_Mission14_BeastMultiBeamFrigateReinforcementTeam Watch_BeastMultiBeamFrigateReinforcementTeam
#define Watch_Mission14_BeastMultiBeamFrigateCombatLoop Watch_BeastMultiBeamFrigateCombatLoop

void Watch_Mission14(void)
{
    GrowSelection *allShips;
    GrowSelection *playerShips;
    GrowSelection *enemyShips;
    GrowSelection *mothershipSelection;
    GrowSelection *attachedShips;
    GrowSelection *supportSelection;
    GrowSelection *linkedShips;
    GrowSelection *workerSelection;

    kasCurrentStep = 0;
    kasCurrentScopeHash = 142503620;

    if (!kasfEntryValueGet(71552166))
    {
        playerShips = kasGrowSelectionPtr(0x4A2A29);
        mothershipSelection = kasGrowSelectionPtr(211966048);
        if (kasfShipsSelectType(mothershipSelection, playerShips, "sMothership"))
        {
            kasfEntrySetValue(71552166, 1);
            attachedShips = kasGrowSelectionPtr(266741017);
            kasfShipsSelectAllAttachedShips(attachedShips, mothershipSelection);
            kasfShipsSelectType(attachedShips, attachedShips, "sMothershipDockingBay");
            if (kasfShipsCount(attachedShips) > 0)
            {
                kasfShipsSetDontApplyPhysicsFlag(attachedShips, 0);
            }
        }
    }

    if (kasTimerExpiredDestroyById(150120898))
    {
        kasTimerSetById(150120898, 10);
        kasTimerStartById(150120898);

        allShips = kasGrowSelectionPtr(18336307);
        kasfFindShipsInside(kasVolumePtr(132933767), allShips);

        playerShips = kasGrowSelectionPtr(0x4A2A29);
        kasSelectPlayerShips(playerShips, allShips);

        enemyShips = kasGrowSelectionPtr(114961833);
        kasSelectEnemyShips(enemyShips, allShips);
    }

    if (!kasfEntryValueGet(71552167) && kasfEntryValueGet(-2015429404))
    {
        kasfEntrySetValue(71552167, 1);

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
}

void Watch_Mission14_FleetIntel_FISensorTech(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission14_FleetIntel_FISensorTechCombatLoop,
            (sdword)Watch_Mission14_FleetIntel_FISensorTechCombatLoop);
    }
}

void Init_Mission14_FleetIntel_FISensorTechCombatLoop(void)
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
    if (kasfTeamCount() <= 5)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationClaw();
    }
    singlePlayerSortShipsByDistanceToPoint(nearbyShips, kasTeamsVectorPtr(team->kasLabelHash));
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission14_FleetIntel_FISensorTechCombatLoop(void)
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
            (sdword)Init_Mission14_FleetIntel_FISensorTechCombatLoop,
            (sdword)Watch_Mission14_FleetIntel_FISensorTechCombatLoop);
    }
}

void Init_Mission14_FleetIntel_FISensorTechLaunchPrep(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(-2107305438, 1);
    kasfEntryGetOrCreateSetValue(332667, 1);
}

void Init_Mission14_ReinforcementClearTrackedSelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasGrowSelectionPtr((char *)0x36980EB);
    tutDeallocateRootRegion();
}

void Init_Mission14_ReinforcementSpecialHyperspaceTeam(void)
{
  kasCurrentStep = 1;
  kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
  kasAITeamShipsPtr(kasCurrentTeamPtr + 28, 2);
  kasfTeamSpecialToggle();
  kasfPatrolPath(kasPathPtr(74019713));
}





















void Init_Mission14_BeastACVReinforcementTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    AITeam *fsmTeam;
    GrowSelection *fsmShips;

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
            fsmTeam = kasAITeamPtrIfExists(team->kasFSMNameHash);
            kasfMsgSend(fsmTeam, (char *)44783843);
            kasAITeamShipsPtr(&team->shipList, 1);
            fsmShips = kasGrowSelectionPtr(team->kasFSMNameHash);
            kasfShipsAdd(fsmShips, &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(
                88512316,
                (sdword)Begin_Mission14_BeastACVReinforcementCombatLoop,
                (sdword)Watch_Mission14_BeastACVReinforcementCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission14_BeastACVReinforcementTeam);
        }
    }
}

void Init_Mission14_BeastACVReinforcementTeam_Reevaluate(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    AITeam *fsmTeam;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(17798))
     && kasfEntryValueGet(-2030864267)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2030864267, 0);
        kasfEntrySetValue(-1981524234, 1);
    }

    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_Mission14_BeastACVDockAndReinforce,
                    (sdword)Watch_Mission14_BeastACVDockAndReinforce);
            }
        }
        else
        {
            fsmTeam = kasAITeamPtrIfExists(team->kasFSMNameHash);
            kasfMsgSend(fsmTeam, (char *)228088883);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission14_BeastACVReinforcementTeam);
        }
    }
}

void Begin_Mission14_BeastACVDockAndReinforce(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    kasfShipsSortByDistanceToPoint(kasAITeamShipsPtrIfExists(170794736), (vector *)kasThisTeamsVectorPtr());
    kasfShipsSelectIndex(kasGrowSelectionPtr(21541), kasAITeamShipsPtrIfExists(170794736), 0);
    kasfFormationDelta3D();
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(21541));
}

void Watch_Mission14_BeastACVDockAndReinforce(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword buildCount;
    sdword buildCost;
    sdword buildTime;
    sdword wantedBuilds;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission14_BeastACVReinforcementTeam);
        return;
    }

    wantedBuilds = 9 - kasfTeamCount();
    switch (kasfEntryValueGet(-2126662828))
    {
        case 2:
            buildCount = kasfEntryValueGet(154639956);
            if (buildCount < wantedBuilds)
            {
                kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bACV"));
                kasfEntryIncrement(154639956);
                buildCount++;
            }
            if (buildCount == 9)
            {
                kasTimerCreateSetStartById(133682484, 0);
            }
            break;

        case 1:
            buildCount = kasfEntryValueGet(154639956);
            if (buildCount < wantedBuilds)
            {
                buildCost = kasfShipTypeBuildCost("bACV") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                    buildCount++;
                }
            }
            if ((buildCount > 2 && kasfRUsGet(1) < kasfShipTypeBuildCost("bACV") / 2) || buildCount == 9)
            {
                buildTime = kasfShipTypeBuildTime("bACV");
                kasTimerCreateSetStartById(133682484, buildTime * buildCount / 2);
            }
            break;

        case 0:
            buildCount = kasfEntryValueGet(154639956);
            if (buildCount < wantedBuilds)
            {
                buildCost = kasfShipTypeBuildCost("bACV");
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                    buildCount++;
                }
            }
            if ((buildCount > 2 && kasfRUsGet(1) < kasfShipTypeBuildCost("bACV")) || buildCount == 9)
            {
                buildTime = kasfShipTypeBuildTime("bACV");
                kasTimerCreateSetStartById(133682484, buildTime * buildCount);
            }
            break;
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr(316548), kasAITeamShipsPtrIfExists(170794736));
    }
}

void Watch_Mission14_BeastACVReinforcementTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    AITeam *fsmTeam;
    GrowSelection *acvSelection;
    GrowSelection *fsmShips;
    GrowSelection *reinforcementPoolShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectType(kasGrowSelectionPtr(363040), kasAITeamShipsPtrIfExists(44665972), "bACV") < 2)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
        {
            if (kasfEntryValueGet(-1981524234))
            {
                kasfEntrySetValue(-1981524234, 0);
                kasfEntrySetValue(-2030864267, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Watch_Mission14_BeastACVReinforcementBuildCount);
            }
        }
        else
        {
            kasfEntrySetValue(-1884903716, 0);
            kasfEntrySetValue(-2031143291, 1);
            kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)sub_652EF0);
        }
    }
    else
    {
        fsmTeam = kasAITeamPtrIfExists(team->kasFSMNameHash);
        kasfMsgSend(fsmTeam, (char *)44783843);
        kasfEntrySetValue(332667, 0);
        if (kasfShipsSelectType(kasGrowSelectionPtr(363040), kasAITeamShipsPtrIfExists(44665972), "bACV"))
        {
            acvSelection = kasGrowSelectionPtr(363040);
            reinforcementPoolShips = kasAITeamShipsPtrIfExists(44665972);
            kasfShipsRemove(reinforcementPoolShips, acvSelection);
            kasfShipsStop(acvSelection);
            fsmShips = kasGrowSelectionPtr(team->kasFSMNameHash);
            kasfShipsAdd(fsmShips, acvSelection);
            kasfShipsSetSlaveableFlag(acvSelection, 0);
            kasfShipsApplyPlayerUpgrades(acvSelection);
            kasfReinforceTeamWithShips(team, acvSelection);
            kasfTacticsNeutral();
        }
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission14_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission14_BeastACVReinforcementCombatLoop);
    }
}

void Watch_Mission14_BeastACVReinforcementBuildCount(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    switch (kasfEntryValueGet(-2126662828))
    {
        case 2:
            if (kasfEntryValueGet(154639956) < 9)
            {
                kasfEntryIncrement(154639956);
            }
            if (kasfEntryValueGet(154639956) == 9)
            {
                kasfSpeechEvent(
                    75929221,
                    (sdword)Init_Mission16_BeastACVReinforcementBuildTimer,
                    (sdword)Watch_Mission14_BeastACVReinforcementSpawn);
            }
            break;

        case 1:
            buildCount = kasfEntryValueGet(154639956);
            if (buildCount < 9)
            {
                buildCost = kasfShipTypeBuildCost("bACV") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                    buildCount++;
                }
            }
            if ((buildCount > 2 && kasfRUsGet(1) < kasfShipTypeBuildCost("bACV") / 2) || buildCount == 9)
            {
                kasfSpeechEvent(
                    75929221,
                    (sdword)Init_Mission16_BeastACVReinforcementBuildTimer,
                    (sdword)Watch_Mission14_BeastACVReinforcementSpawn);
            }
            break;

        case 0:
            buildCount = kasfEntryValueGet(154639956);
            if (buildCount < 9)
            {
                buildCost = kasfShipTypeBuildCost("bACV");
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                    buildCount++;
                }
            }
            if ((buildCount > 2 && kasfRUsGet(1) < kasfShipTypeBuildCost("bACV")) || buildCount == 9)
            {
                kasfSpeechEvent(
                    75929221,
                    (sdword)Init_Mission16_BeastACVReinforcementBuildTimer,
                    (sdword)Watch_Mission14_BeastACVReinforcementSpawn);
            }
            break;
    }
}

void Watch_Mission14_BeastACVReinforcementSpawn(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    AITeam *fsmTeam;
    GrowSelection *createdShips;
    sdword dockIndex;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
    {
        if (kasTimerExpiredById(235522949))
        {
            kasfStop();
            dockIndex = kasfRandom(0, kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)) - 1);
            kasfShipsSelectIndex(kasGrowSelectionPtr(21541), kasAITeamShipsPtrIfExists(170794736), dockIndex);
            kasfShipsCreateInside(
                kasGrowSelectionPtr(154639956),
                "bACV",
                kasfEntryValueGet(154639956),
                kasGrowSelectionPtr(21541),
                1);
            createdShips = kasGrowSelectionPtr(154639956);
            kasAITeamShipsPtr(createdShips, 1);
            kasfEntrySetValue(154639956, 0);
            kasfReinforceTeamWithShips(team, createdShips);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        }
        if (kasfTeamFinishedLaunching())
        {
            kasfEntrySetValue(332667, 0);
            fsmTeam = kasAITeamPtrIfExists(team->kasFSMNameHash);
            kasfMsgSend(fsmTeam, (char *)44783843);
            kasfSpeechEvent(
                88512316,
                (sdword)Begin_Mission14_BeastACVReinforcementCombatLoop,
                (sdword)Watch_Mission14_BeastACVReinforcementCombatLoop);
        }
    }
    else
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)sub_652EF0);
    }
}

void Begin_Mission14_BeastACVReinforcementCombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *capitalShips;
    GrowSelection *nearbyCapitalShips;
    GrowSelection *attackers;
    GrowSelection *closeTargets;
    sdword targetIndex;
    sdword offsetDistance;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    capitalShips = kasGrowSelectionPtr(105808290);
    kasfShipsSelectCapital(capitalShips, kasGrowSelectionPtr(team->kasFSMNameHash));
    nearbyCapitalShips = kasGrowSelectionPtr(124053532);
    if (!kasfShipsSelectNearby(nearbyCapitalShips, capitalShips, kasThisTeamsVectorPtr(), 60000))
    {
        kasfGrowSelectionClear(nearbyCapitalShips);
        kasfShipsAdd(nearbyCapitalShips, capitalShips);
    }
    if (!kasfShipsCount(nearbyCapitalShips))
    {
        kasfSpeechEvent(374244, (sdword)Begin_BeastACVCombatLoopDelay, (sdword)Watch_Mission14_BeastACVCombatRetryDelay);
        return;
    }

    kasfTacticsNeutral();
    attackers = kasGrowSelectionPtr(139967715);
    if (kasfFindShipsTargetingSelection(nearbyCapitalShips, attackers))
    {
        kasfShipsSortByDistanceToPoint(attackers, (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(21541), attackers, 0);
        kasfFormationDelta3D();
        kasfMoveTo(kasShipsVectorPtrIfExists(21541));
        return;
    }

    closeTargets = kasGrowSelectionPtr(338274);
    if (kasfShipsSelectNearby(closeTargets, nearbyCapitalShips, kasThisTeamsVectorPtr(), 20000))
    {
        kasfShipsSortByDistanceToPoint(closeTargets, (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(21541), closeTargets, 0);
        kasfFormationDelta3D();
        kasfMoveTo(kasShipsVectorPtrIfExists(21541));
        return;
    }

    targetIndex = kasfRandom(0, kasfShipsCount(nearbyCapitalShips) - 1);
    kasfShipsSelectIndex(kasGrowSelectionPtr(21541), nearbyCapitalShips, targetIndex);
    kasfLabelledVectorSet(146094692, 0, 0, 0);
    offsetDistance = kasfRandom(6000, 8000);
    kasfVectorRandomizeInRange(kasVectorPtrIfExists(146094692), kasShipsVectorPtrIfExists(21541), offsetDistance);
    kasfFormationDelta3D();
    kasfMoveTo(kasVectorPtrIfExists(146094692));
}

void Watch_Mission14_BeastACVReinforcementCombatLoop(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *targets;
    GrowSelection *attackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    targets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(targets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000)
     || (attackers = kasGrowSelectionPtr(139967715), kasfUnderAttack(attackers))
      && kasfShipsSelectMembersOfSelection(targets, attackers, targets))
    {
        kasfSpeechEvent(74024315, (sdword)Begin_Mission14_BeastACVCombatAttackRun, 0);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission14_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission14_BeastACVReinforcementCombatLoop);
    }
}

void Begin_Mission14_BeastACVCombatAttackRun(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *nearbyShips;
    GrowSelection *targets;
    GrowSelection *attackers;
    sdword retryDelay;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    nearbyShips = kasGrowSelectionPtr(86599289);
    if (kasfShipsSelectNearby(nearbyShips, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        targets = kasGrowSelectionPtr(57245931);
        if (!kasfShipsSelectType(targets, nearbyShips, "sLeech")
         && !kasfShipsSelectType(targets, nearbyShips, "sMCV")
         && !kasfShipsSelectType(targets, nearbyShips, "sMimic"))
        {
            kasfShipsAdd(targets, nearbyShips);
        }

        attackers = kasGrowSelectionPtr(139967715);
        if (kasfUnderAttack(attackers))
        {
            kasfShipsAdd(targets, attackers);
        }

        kasfFormationDelta3D();
        kasfShipsSortByDistanceToPoint(targets, (vector *)kasThisTeamsVectorPtr());
        if (kasfShipsAnyHyperspaceReady(&team->shipList))
        {
            if (kasfShipsCount(attackers)
             && kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr(338274), attackers, targets))
            {
                kasfIssueSpecialAttack(kasGrowSelectionPtr(338274));
            }
            else
            {
                kasfIssueSpecialAttack(targets);
            }
        }
        else
        {
            tutDeallocateRootRegion(targets);
        }

        kasTimerDestroyById(74024315);
        retryDelay = kasfRandom(10, 30);
        kasTimerCreateSetStartById(74024315, retryDelay);
    }
    else
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission14_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission14_BeastACVReinforcementCombatLoop);
    }
}

void Watch_Mission14_BeastACVCombatRetryDelay(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission14_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission14_BeastACVReinforcementCombatLoop);
    }
}

void Init_Mission14_SentinelReinforcementTeam(void)
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
                (sdword)Begin_Mission14_SentinelReinforcementCombatLoop,
                (sdword)Watch_Mission14_SentinelReinforcementCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission14_SentinelReinforcementTeam);
        }
    }
}

void Init_Mission14_SentinelReinforcementTeam_Reevaluate(void)
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
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission14_SentinelReinforcementTeam);
    }
}

void Watch_Mission14_SentinelReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedSentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedSentinels = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(returnedSentinels, kasAITeamShipsPtrIfExists(44665972), "bSentinel") < 3)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
        {
            if (kasfEntryValueGet(-1884903716))
            {
                kasfEntrySetValue(-1884903716, 0);
                kasfEntrySetValue(-2031143291, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Watch_Mission14_SentinelReinforcementBuildCount);
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
    kasfSpeechEvent(
        76130852,
        (sdword)Begin_Mission14_SentinelReinforcementCombatLoop,
        (sdword)Watch_Mission14_SentinelReinforcementCombatLoop);
}

void Watch_Mission14_SentinelReinforcementBuildCount(void)
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
        if (kasfEntryValueGet(154639956) > 3 && kasfRUsGet(1) < kasfShipTypeBuildCost("bSentinel"))
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
                (sdword)Watch_Mission14_SentinelReinforcementSpawn);
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
            kasfSpeechEvent(
                75929221,
                (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
                (sdword)Watch_Mission14_SentinelReinforcementSpawn);
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
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
            (sdword)Watch_Mission14_SentinelReinforcementSpawn);
    }
}

void Watch_Mission14_SentinelReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *spawnParents;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasAITeamShipsPtrIfExists(170794736);
    if (!kasfShipsCount(spawnParents))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)Init_Mission09_AssaultEscort_Cooldown, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        kasfShipsSelectIndex(kasGrowSelectionPtr(21541), spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "bSentinel",
            kasfEntryValueGet(154639956),
            kasGrowSelectionPtr(21541),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            76130852,
            (sdword)Begin_Mission14_SentinelReinforcementCombatLoop,
            (sdword)Watch_Mission14_SentinelReinforcementCombatLoop);
    }
}

void Begin_Mission14_SentinelReinforcementCombatLoop(void)
{
    GrowSelection *teamShips;
    GrowSelection *priorityTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    teamShips = kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash);
    kasfShipsSelectCapital(teamShips, teamShips);
    kasfShipsSelectRace(teamShips, teamShips, 1);
    if (kasfShipsSelectType(kasGrowSelectionPtr(171484652), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bSentinel")
     && kasfShipsSelectTargets(kasGrowSelectionPtr(157715175), kasGrowSelectionPtr(171484652)))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(157715175));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(151690773), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bIonArrayFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(151690773));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(223105365), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bRammingFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(223105365));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(191446069), kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bDFGFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(191446069));
    }

    if (!kasfShipsCount(teamShips))
    {
        if (!kasfEntryValueGet(84320888))
        {
            kasfSpeechEvent(
                374244,
                (sdword)Begin_SentinelCombatRetryDelay,
                (sdword)Watch_Mission14_SentinelCombatRetryDelay);
        }
        return;
    }

    kasfTacticsNeutral();
    kasfFormationWall();
    kasfShipsSelectNearby(kasGrowSelectionPtr(86599289), teamShips, kasThisTeamsVectorPtr(), 20000);
    kasfShipsSelectType(kasGrowSelectionPtr(212896882), kasGrowSelectionPtr(86599289), "bHeavyCruiser");
    kasfShipsSelectType(kasGrowSelectionPtr(124218850), kasGrowSelectionPtr(86599289), "bCarrier");
    kasfShipsSelectType(kasGrowSelectionPtr(216029280), kasGrowSelectionPtr(86599289), "bMothership");
    if (!kasfShipsCount(kasGrowSelectionPtr(212896882))
     && !kasfShipsCount(kasGrowSelectionPtr(124218850))
     && !kasfShipsCount(kasGrowSelectionPtr(216029280)))
    {
        kasfShipsSortByDistanceToPoint(teamShips, (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), teamShips, 0);
    }
    else
    {
        priorityTargets = kasGrowSelectionPtr(125446884);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(212896882));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(124218850));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(216029280));
        kasfShipsSortByDistanceToPoint(priorityTargets, (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), priorityTargets, 0);
    }
    kasfProtect(kasGrowSelectionPtr(154965556));
    kasfEntryScopeName(111581908);
    kasfEntryScopeName(49272164);
    kasfEntryScopeName(127823460);
}

void Watch_Mission14_SentinelReinforcementCombatLoop(void)
{
    AITeam *team;
    sdword requestedCount;
    sdword totalCost;
    sdword randomParentIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        if (!kasfEntryValueGet(127823460) && (kasfShipsOrderAttributes(&team->shipList) & 2) != 0)
        {
            kasfFormationWall();
            kasfEntryGetOrCreateSetValue(127823460, 1);
        }
        else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0 && !kasfEntryValueGet(84320888))
        {
            kasfEntrySetValue(127823460, 0);
            kasfStop();
            kasfSpeechEvent(
                374244,
                (sdword)Begin_SentinelCombatRetryDelay,
                (sdword)Watch_Mission14_SentinelCombatRetryDelay);
            return;
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
    if (kasfRUsGet(1) >= totalCost
     && kasTimerExpiredById(111581908)
     && kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
    {
        kasfRUsSet(1, kasfRUsGet(1) - totalCost);
        randomParentIndex = kasfRandom(0, kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)) - 1);
        kasfShipsSelectIndex(kasGrowSelectionPtr(21541), kasAITeamShipsPtrIfExists(170794736), randomParentIndex);
        if (kasfTeamCount() + kasfEntryValueGet(111592388) >= 12)
        {
            kasfEntrySetValue(111592388, 12 - kasfTeamCount());
        }
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "bSentinel",
            kasfEntryValueGet(111592388),
            kasGrowSelectionPtr(21541),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
        kasfShipsAdd(&team->shipList, kasGrowSelectionPtr(154639956));
        kasfEntryGetOrCreateSetValue(84320888, 1);
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

void Watch_Mission14_SentinelCombatRetryDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfStop();
        kasfSpeechEvent(
76130852,
(sdword)Begin_Mission14_SentinelReinforcementCombatLoop,
(int)Watch_Mission14_SentinelReinforcementCombatLoop);
    }
}

void Init_Mission14_HiveFrigateReinforcementTeam(void)
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
                (sdword)Watch_Mission14_HiveFrigateCombatLoop);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission14_HiveFrigateReinforcementTeam);
        }
    }
}

void Init_Mission14_HiveFrigateReinforcementTeam_Reevaluate(void)
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
            (sdword)Watch_Mission14_HiveFrigateReinforcementTeam);
    }
}

void Watch_Mission14_HiveFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedHiveFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedHiveFrigates = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(returnedHiveFrigates, kasGrowSelectionPtr(44665972), "bHiveFrigate") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
        {
            if (kasfEntryValueGet(-1892538891))
            {
                kasfEntrySetValue(-1892538891, 0);
                kasfEntrySetValue(-2047426043, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Watch_Mission14_HiveFrigateReinforcementBuildCount);
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
        (sdword)Watch_Mission14_HiveFrigateCombatLoop);
}

void Watch_Mission14_HiveFrigateReinforcementBuildCount(void)
{
    sdword buildStyle;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle != 0)
    {
        buildStyle--;
        if (buildStyle != 0)
        {
            if (buildStyle != 1)
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
        }
        else
        {
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
        }
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_HiveFrigateReinforcementBuildTimer,
            (sdword)Watch_Mission14_HiveFrigateReinforcementSpawn);
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
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_HiveFrigateReinforcementBuildTimer,
                (sdword)Watch_Mission14_HiveFrigateReinforcementSpawn);
            return;
        }
    }
    if (kasfEntryValueGet(154639956) == 2)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_HiveFrigateReinforcementBuildTimer,
            (sdword)Watch_Mission14_HiveFrigateReinforcementSpawn);
    }
}

void Watch_Mission14_HiveFrigateReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *spawnParents;
    GrowSelection *spawnTarget;
    GrowSelection *createdHiveFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasAITeamShipsPtrIfExists(170794736);
    if (!kasfShipsCount(spawnParents))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnTarget = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnTarget, spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
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
            (sdword)Watch_Mission14_HiveFrigateCombatLoop);
    }
}

void Watch_Mission14_HiveFrigateCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_HiveFrigateCombatLoop,
            (sdword)Watch_Mission14_HiveFrigateCombatLoop);
    }
}
























void Watch_Mission14_FleetIntel_FISensorTechLaunchPrep(void)
{
    AITeam *team;
    GrowSelection *fighters;
    AITeam *reserveTeam;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    fighters = kasGrowSelectionPtr(0x593584);
    if (kasfShipsSelectClass(fighters, kasGrowSelectionPtr(0x6DA2DA9), "CLASS_Fighter")
     && kasfShipsSelectSinglePlayerInParade(fighters, fighters))
    {
        reserveTeam = kasAITeamPtrIfExists(246591522);
        kasfReinforceTeamWithShips(reserveTeam, fighters);
        kasfShipsAdd(fighters, kasAITeamShipsPtrIfExists(239669557));
        kasfShipsSetFormation(fighters, DELTA_FORMATION);
        kasfTacticsAggressive();
        kasTimerStartById(-2107305438);
    }
    if (kasTimerExpiredDestroyById(-2107305438)
     && !kasfThisTeamIs(kasAITeamPtrIfExists(246591522))
     && !kasfTeamCount()
     && kasfShipsCount(kasAITeamShipsPtrIfExists(246591522)) >= 3
     && kasfShipsSelectFirstN(fighters, kasAITeamShipsPtrIfExists(246591522), kasfShipsCount(kasAITeamShipsPtrIfExists(246591522))))
    {
        kasTimerSetById(-2107305438, kasfRandom(30, 50));
        kasfShipsRemove(kasAITeamShipsPtrIfExists(246591522), fighters);
        kasfReinforceTeamWithShips(team, fighters);
        kasfTacticsNeutral();
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Init_Mission14_FleetIntel_FISensorTechCombatLoop, 0);
    }
    else if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfTeamHealthAverage() < 30)
            {
                kasfSpeechEvent(299899, (sdword)Begin_Mission14_FleetIntel_FISensorTechLaunch, 0);
            }
            else if (kasfMsgReceived((char *)0x0469857B))
            {
                kasfSpeechEvent(74024315, (sdword)Init_Mission14_FleetIntel_FISensorTechCombatLoop, 0);
            }
        }
        else
        {
            kasfSpeechEvent(342540, (sdword)Begin_Mission14_FleetIntel_FISensorTechLaunchHold, 0);
        }
    }
}

void Begin_Mission14_FleetIntel_FISensorTechLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (kasfTeamCount() <= 5)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationClaw();
    }
    kasfDockStayWithTeam(kasAITeamPtrIfExists(239669557));
}

void Watch_Mission14_FleetIntel_FISensorTechLaunch(void)
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
        kasfSpeechEvent(342540, (sdword)Begin_Mission14_FleetIntel_FISensorTechLaunchHold, 0);
    }
}

void Begin_Mission14_FleetIntel_FISensorTechLaunchHold(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (!kasfEntryValueGet(71552179))
    {
        kasfEntryGetOrCreateSetValue(71552179, 1);
        kasfMsgSend(kasAITeamPtrIfExists(218085361), (char *)5801588);
    }
}

void Init_Mission14_MissionFlow(void)
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
    kasfEntryGetOrCreateSetValue(70074560, 0);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfTutEnableFlags("Flags");
    kasfTutEnableFlags("CancelFocus");
    kasfEntryGetOrCreateSetValue(109580580, 0);
    kasfMusicPlay(13);
    kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)5801588);
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
}

void Watch_Mission14_MissionFlow_PostIntelCleanup(void)
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
        if (kasfEntryValueGet(70074560))
        {
            kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(0xCA25860));
            kasfEntrySetValue(70074560, 0);
        }
        if (kasfEntryValueGet(52577235))
        {
            kasfCloseSensors(1);
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
        if (kasfEntryValueGet(127756738))
        {
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
        }
        else if (kasfEntryValueGet(109580580))
        {
            kasfSpeechEvent(109580580, (sdword)MaintainTeamContext, 0);
        }
        else
        {
            kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
        }
    }
}

void Watch_Mission14_MissionFlow_PostIntelDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0D2247A4))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, 0);
    }
    else if (kasTimerExpiredDestroyById(144337492))
    {
        if (kasfEntryValueGet(109580580))
        {
            kasfSpeechEvent(109580580, (sdword)MaintainTeamContext, 0);
        }
        else
        {
            kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
        }
    }
}

void Watch_Mission14_MissionFlow(void)
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
            case 0x00588674:
                kasfLocationCard(4000, ((char **)0x8CC474)[strCurLanguage]);
                kasfMsgSend(team, (char *)92825426);
                break;
            case 290904:
                kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(0xCA25860));
                kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
                kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
                kasfCameraSetDistance(kasfEntryValueGet(64272901));
                kasfWideScreenOut(90);
                kasfSubtitleDraw();
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)24416660);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
                return;
            case 74109998:
                kasfSpeechEventQueueCurrentPlayer(268452426, 0);
                kasfMsgSend(team, (char *)161461235);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 74620837:
                kasfOpenSensors(100000);
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfPingAddShips(kasAITeamShipsPtrIfExists(96166939), "PingOnRKO");
                kasfSensorsSetLookatPoint(kasTeamsVectorPtr(96166939));
                soundStopDSound("PointToRKO", kasAITeamShipsPtrIfExists(96166939));
                kasfSpeechEventQueueCurrentPlayer(268444239, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)74620837);
                kasfMsgSend(team, (char *)290904);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 82233758:
                kasfSpeechEventQueueCurrentPlayer(268469071, 0);
                kasfEntrySetValue(127756738, 3);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(232774915, 1);
                kasfMsgSend(team, (char *)241998258);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 92825426:
                kasfMsgSend(team, (char *)92825427);
                spSetSinglePlayerSaveInfo(14, ((char **)0x8CC474)[strCurLanguage]);
                break;
            case 92825427:
                kasfEntrySetValue(-2054310428, 0);
                kasfSpeechEventQueueCurrentPlayer(268440135, 0);
                kasfMsgSend(team, (char *)183745415);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 104994414:
                kasfMsgSend(team, (char *)69297810);
                kasfEntrySetValue(109580580, 1);
                kasfSpeechEvent(109580580, (sdword)MaintainTeamContext, 0);
                return;
            case 133313269:
                kasfPingAddShipsIndexed(kasAITeamShipsPtrIfExists(226320642), "PingOnRKO");
                kasfSpeechEventQueueCurrentPlayer(268444242, 0);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 161461235:
                kasfSpeechEventQueueCurrentPlayer(268440139, 0);
                kasfMsgSend(team, (char *)168706900);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 168706900:
                kasfSpeechEventQueueCurrentPlayer(268452428, 0);
                kasfMsgSend(team, (char *)172978339);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 172978339:
                kasfSpeechEventQueueCurrentPlayer(268440141, 0);
                kasfMsgSend(team, (char *)195477364);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 178863038:
                kasfSpeechEventQueueCurrentPlayer(268452425, 0);
                kasfMsgSend(team, (char *)74109998);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 183745415:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(96166939));
                kasfCameraSetAngleDeg(1);
                kasfCameraSetDeclinationDeg(-10);
                kasfCameraSetDistance(3955);
                kasfSpeechEventQueueCurrentPlayer(268452424, 0);
                kasfMsgSend(team, (char *)178863038);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 195477364:
                kasfSpeechEventQueueCurrentPlayer(268452430, 0);
                kasfMsgSend(team, (char *)74620837);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 196355317:
                kasfPingRemove("PingOnRKO");
                kasfMsgSend(team, (char *)188895186);
                kasfEntrySetValue(109580580, 1);
                kasfSpeechEvent(109580580, (sdword)MaintainTeamContext, 0);
                return;
            case 220350372:
                kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, 0);
                return;
            case 241998258:
                kasfEntrySetValue(127756738, 120);
                kasfMsgSend(team, (char *)196355317);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
                return;
            case 24416660:
                kasfMusicPlay(22);
                kasfMsgSendAll((char *)74024315);
                kasfSpeechEventQueueCurrentPlayer(268444240, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)24416660);
                kasfMsgSend(team, (char *)104994414);
                kasfEntrySetValue(127756738, 120);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            case 250361986:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(0x801A8E3));
                kasfCameraSetDeclinationDeg(0);
                kasfCameraSetAngleDeg(0);
                kasfCameraSetDistance(500);
                break;
            case 250528645:
                kasfPingRemove("PingOnRKO");
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)250361986);
                kasfMsgReceived((char *)0x07F232F5);
                kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
                return;
            case 260320331:
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(96166939));
                kasfCameraSetAngleDeg(0);
                kasfCameraSetDistance(5000);
                break;
            default:
                if (message != 0)
                {
                    aitQueuePointer(team, team, (void *)message);
                }
                break;
        }

        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
        {
            return;
        }
    }
}

void Init_Mission14_ReinforcementPool(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(74024315, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, 0);
}

void Fill_Mission14_ReinforcementPoolFromParade(void)
{
    AITeam *team;
    GrowSelection *paradeShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    paradeShips = kasGrowSelectionPtr(0x889C4F4);
    if (kasfShipsSelectSinglePlayerInParade(paradeShips, kasGrowSelectionPtr(0x6DA2DA9)))
    {
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsApplyPlayerUpgrades(paradeShips);
        kasfReinforceTeamWithShips(team, paradeShips);
    }
}

void Watch_Mission14_ReinforcementPool_Combat(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&team->shipList))
        {
            kasfSpeechEvent(74024315, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, 0);
        }
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Watch_Mission14_ReinforcementPool_Resume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(74024315, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, 0);
    }
}

void Watch_Mission14_BeastCloakedFighterReinforcementLaunchTimer(void)
{
    AITeam *team;
    sdword buildStyle;
    sdword buildLimit;
    sdword buildCost;
    GrowSelection *linkedMotherships;
    GrowSelection *newFighters;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        return;
    }

    buildStyle = kasfEntryValueGet(-2126662828);
    buildLimit = 6 - kasfTeamCount();
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter") / 2;
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1
          && kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
         || kasfEntryValueGet(154639956) == 6)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfShipTypeBuildTime("bCloakedFighter") * kasfEntryValueGet(154639956) / 2);
        }
    }
    else if (buildStyle == 2)
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
    else
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter");
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1
          && kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
         || kasfEntryValueGet(154639956) == 6)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bCloakedFighter"));
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        linkedMotherships = kasGrowSelectionPtr(0x4D484);
        kasfShipsSelectLinkedWith(&team->shipList, linkedMotherships, kasAITeamShipsPtrIfExists(170794736));
        newFighters = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreate(newFighters, "bCloakedFighter", kasfEntryValueGet(154639956), 1);
        kasAITeamShipsPtr(newFighters, 1);
        kasfShipsDockInstant(newFighters, linkedMotherships);
        kasfReinforceTeamWithShips(team, newFighters);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfSpeechEvent(84320888, (sdword)Init_LaunchDockedTeam, 0);
    }
}




void Init_Mission14_ReinforcementTriggerRouter(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(32713509, (sdword)MaintainTeamContext, 0);
}

void Init_Mission14_ReinforcementTriggerRouter_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(374244, 90);
}

void Watch_Mission14_ReinforcementTriggerRouter_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfSpeechEvent(32713509, (sdword)MaintainTeamContext, 0);
    }
}

void Watch_Mission14_ReinforcementTriggerRouter(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;
    AITeam *targetTeam;
    GrowSelection *capitalShips;
    GrowSelection *sentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
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
            case 132952962:
                kasfMsgSend(kasAITeamPtrIfExists(171501548), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(62813829), (char *)4693508);
                kasfMsgSend(team, (char *)132952963);
                kasfSpeechEvent(
                    374244,
                    (sdword)Init_Mission14_ReinforcementTriggerRouter_Delay,
                    (sdword)Watch_Mission14_ReinforcementTriggerRouter_Delay);
                return;
            case 132952963:
                kasfEntryGetOrCreateSetValue(-2126662828, 0);
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                kasfSpeechEvent(
                    374244,
                    (sdword)Init_Mission14_ReinforcementTriggerRouter_Delay,
                    (sdword)Watch_Mission14_ReinforcementTriggerRouter_Delay);
                return;
            case 141560536:
                kasfEntryGetOrCreateSetValue(-2126662828, 2);
                kasfMsgSend(kasAITeamPtrIfExists(17798), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(171501548), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(62813829), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(200156578), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists(223105365), (char *)4693508);
                if (GetGameDifficulty())
                {
                    kasfMsgSend(kasAITeamPtrIfExists(186735637), (char *)4693508);
                    kasfMsgSend(team, (char *)132952962);
                }
                kasfSpeechEvent(
                    374244,
                    (sdword)Init_Mission14_ReinforcementTriggerRouter_Delay,
                    (sdword)Watch_Mission14_ReinforcementTriggerRouter_Delay);
                return;
            case 154145491:
                targetTeam = kasAITeamPtrIfExists(182180066);
                break;
            case 158207907:
                targetTeam = kasAITeamPtrIfExists(0x56983E);
                break;
            case 173761573:
                capitalShips = kasGrowSelectionPtr(0x764E81C);
                if (kasfShipsSelectCapital(capitalShips, kasGrowSelectionPtr(team->kasFSMNameHash)))
                {
                    kasfShipsHyperspaceIn(capitalShips, NULL);
                    kasTimerCreateSetStartById(51324946, 20);
                }
                sentinels = kasGrowSelectionPtr(0x58A20);
                if (kasfShipsSelectType(sentinels, kasGrowSelectionPtr(team->kasFSMNameHash), "bSentinel"))
                {
                    kasfReinforceTeamWithShips(kasAITeamPtrIfExists(171814892), sentinels);
                }
                kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
                return;
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

    if (kasTimerExpiredById(51324946))
    {
        capitalShips = kasGrowSelectionPtr(0x764E81C);
        kasfShipsSelectCapital(capitalShips, kasGrowSelectionPtr(team->kasFSMNameHash));
        kasfDeleteShips(capitalShips);
    }
    if (kasTimerExpiredById(132952960))
    {
        kasfMsgSend(team, (char *)132952961);
    }
}

void Watch_Mission14_BeastCloakedFighterLaunchDockedReinforcements(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfTeamFinishedLaunching())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)197700291);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(337061, (sdword)Begin_BeastCloakedFighterCombatLoop, 0);
        }
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
    }
}

void Init_Mission14_DockForReinforcements(void)
{
    AITeam *team;
    GrowSelection *dockTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    kasfShipsSortByDistanceToPoint(kasAITeamShipsPtrIfExists(170794736), (vector *)kasThisTeamsVectorPtr());
    dockTargets = kasGrowSelectionPtr(21541);
    kasfShipsSelectIndex(dockTargets, kasAITeamShipsPtrIfExists(170794736), 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    kasfShipsDockWithSelection(&team->shipList, dockTargets);
}

void Watch_Mission14_MissionFlow_AdvanceQueue(void)
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
        if (message == 69297810)
        {
            kasfMsgSend(kasAITeamPtrIfExists(170794736), (char *)104994414);
            kasfMsgSend(kasAITeamPtrIfExists(142188961), (char *)104994414);
        }
        else if (message == 18980)
        {
            kasfEntrySetValue(127756738, kasfEntryValueGet(256401159));
            kasfEntryScopeName(256401159);
            kasfEntrySetValue(109580580, 0);
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
            return;
        }
        else if (message == 142510786)
        {
            kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
            kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
            kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
            kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(170794736));
            kasfCameraSetAngleDeg(30);
            kasfCameraSetDeclinationDeg(-13);
            kasfCameraSetDistance(2800);
            kasfSpeechEventQueueCurrentPlayer(268444241, 0);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(127756738, 3);
            kasfMsgSend(team, (char *)18980);
            kasfEntrySetValue(172907219, 1);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
            return;
        }
        else if (message == 173761573)
        {
            kasfMusicPlay(13);
            if (kasfEntryValueGet(-1915569842))
            {
                kasfSpeechEventQueueCurrentPlayer(268444244, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)231208574);
                kasfEntrySetValue(266064926, 1);
                kasfMsgSend(kasAITeamPtrIfExists(144390657), (char *)145024709);
                kasfMsgSend(kasAITeamPtrIfExists(144390658), (char *)145024709);
                kasfMsgSend(team, (char *)18980);
                kasfEntrySetValue(172907219, 1);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
            }
            else
            {
                kasfWideScreenOut(90);
                kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, 0);
            }
            return;
        }
        else if (message == 159901860)
        {
            spEventWideScreenIn(90);
            kasfEntrySetValue(127756738, 3);
            kasfMsgSend(team, (char *)142510786);
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
            return;
        }
        else if (message == 188895186)
        {
            kasfMsgSend(kasAITeamPtrIfExists(96166939), (char *)196355317);
            spEventWideScreenIn(90);
            kasfMsgSend(team, (char *)69533137);
            kasfEntrySetValue(127756738, 3);
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
            return;
        }
        else if (message == 241998259)
        {
            kasfMsgSendAll((char *)173761573);
            kasfEntrySetValue(127756738, 12);
            kasfSpeechEvent(244882162, (sdword)Init_Mission11_MissionFlow_Delay, 0);
            return;
        }
        else
        {
            aitQueuePointer(team, team, (void *)message);
        }

        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
        {
            return;
        }
    }
}

void Watch_Mission14_PrimaryObjectiveController(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;
    GrowSelection *queuedShips;
    GrowSelection *capitalShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueInfo != NULL) ? *queueInfo : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message == 74620837)
        {
            kasfObjectiveCreate(
                "PROTECT_MOTHERSHIP",
                ((char **)0x8CC48C)[strCurLanguage],
                ((char **)0x8CC498)[strCurLanguage]);
            kasfObjectiveSet("PROTECT_MOTHERSHIP", 0);
            kasfObjectiveCreate("GET_SAMPLE", ((char **)0x8CC4A4)[strCurLanguage], ((char **)0x8CC4B0)[strCurLanguage]);
            kasfObjectiveSet("GET_SAMPLE", 0);
            kasfEntryGetOrCreateSetValue(290904, 1);
        }
        else if (message == 24416660)
        {
            kasfPlayerAcquiredTechnology("TECH_ADRIVE_DESTROYER");
            kasfPlayerAcquiredTechnology("TECH_MISSILE");
            kasfPlayerAcquiredTechnology("TECH_ENERGY");
            kasfPlayerAcquiredTechnology("TECH_ION");
            kasfEnableShip("sDreadnought", "RACE_SECT", 1);
        }
        else if (message == 231208574)
        {
            kasfObjectiveCreate(
                "CAPTURE_TAIIDAN",
                ((char **)0x8CC4BC)[strCurLanguage],
                ((char **)0x8CC4C8)[strCurLanguage]);
            kasfObjectiveSet("CAPTURE_TAIIDAN", 0);
            queuedShips = kasGrowSelectionPtr(0x61A06E4);
            kasfShipsSelectRowGettingOutOfWay(queuedShips, kasGrowSelectionPtr(0x4A2A29));
            kasfShipsAdd(kasGrowSelectionPtr(0x48F83F4), queuedShips);
            kasfEntryGetOrCreateSetValue(0x05ED1D40, 1);
            kasfEntryGetOrCreateSetValue(0x0EF66ED3, kasfGetShipTypeCount(" ", 0));
        }
        else if (message == 82233758)
        {
            kasfObjectiveDestroy("GET_SAMPLE");
            kasfEntryGetOrCreateSetValue(-1915569842, 1);
        }
        else
        {
            if (message != 0)
            {
                aitQueuePointer(team, team, (void *)message);
            }
        }

        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
        {
            break;
        }
    }

    if (kasfEntryValueGet(0x05ED1D40))
    {
        queuedShips = kasGrowSelectionPtr(0x61A06E4);
        if (kasfShipsSelectRowGettingOutOfWay(queuedShips, kasGrowSelectionPtr(0x4A2A29)))
        {
            kasfEntrySetValue(0x05ED1D40, 0);
            kasfEntryGetOrCreateSetValue(0x0B81BCB4, 1);
            objectiveDestroyAll();
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)211499095);
            kasfObjectiveCreate("Hyperspace", ((char **)0x8CC4EC)[strCurLanguage], ((char **)0x8CC4F8)[strCurLanguage]);
            kasfObjectiveSet("Hyperspace", 0);
            spSetSinglePlayerSaveInfo(14, ((char **)0x8CC480)[strCurLanguage]);
        }
        else
        {
            kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
            capitalShips = kasGrowSelectionPtr(0x85DD80E);
            kasfShipsSelectRace(capitalShips, kasGrowSelectionPtr(18336307), 3);
            kasfShipsRemove(capitalShips, kasGrowSelectionPtr(0x48F83F4));
            kasfShipsSelectCapital(capitalShips, capitalShips);
            kasfShipsRemove(capitalShips, kasAITeamShipsPtrIfExists(226320642));
            kasfShipsAdd(capitalShips, kasAITeamShipsPtrIfExists(144390657));
            kasfShipsAdd(capitalShips, kasAITeamShipsPtrIfExists(144390658));
            if (!kasfShipsCount(capitalShips))
            {
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
                kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
                return;
            }
        }
    }

    if (!kasfShipsCount(kasGrowSelectionPtr(0xCA25860)) && kasfEntryValueGet(290904))
    {
        kasfEntryScopeName(290904);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission14_MultigunCorvetteReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(148949186, 6 * kasfShipTypeBuildTime("tMultiGunCorvette") / 2);
}

void Watch_Mission14_MultigunCorvetteReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(75929221, (sdword)Begin_Mission14_MultigunCorvetteReinforcementSpawn, 0);
    }
}

void Begin_Mission14_MultigunCorvetteReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *createdCorvettes;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    createdCorvettes = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(createdCorvettes);
    kasfShipsCreateInside(createdCorvettes, "tMultiGunCorvette", 6, kasAITeamShipsPtrIfExists(124217378), 1);
    kasfReinforceTeamWithShips(team, createdCorvettes);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasAITeamShipsPtr(&team->shipList, 2);
}

void Watch_Mission14_MultigunCorvetteReinforcementSpawn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(124217378)))
    {
        kasfFormationDelta();
        kasfSpeechEvent(139967671, (sdword)Init_Mission14_ReinforcementGroup_SelectCapitalTargets, 0);
    }
}

void Init_Mission14_TaiidanReinforcementTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    kasfSpeechEvent(117831748, (sdword)Init_Mission14_TaiidanReinforcementReset, 0);
}

void Init_Mission14_TaiidanReinforcementReset(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(-2102956012, 0);
}

void Watch_Mission14_TaiidanReinforcementTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(104399076))
         || kasfThisTeamIs(kasAITeamPtrIfExists(104399077)))
        {
            kasfSpeechEvent(139967671, (sdword)Init_Mission14_ReinforcementBuildClosestTargetSelection, 0);
        }
        else
        {
            kasfAttackMothership();
            kasfSpeechEvent(342540, (sdword)Init_AttackMothershipDeleteDelay, 0);
        }
    }
    else if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
        {
            if (kasfThisTeamIs(kasAITeamPtrIfExists(268338043)))
            {
                kasfTeamSpecial(kasAITeamShipsPtrIfExists(96166939));
            }
            else
            {
                Init_Mission14_ReinforcementBuildClosestTargetSelection();
            }
        }
    }
    else
    {
        kasfSpeechEvent(75929221, (sdword)Begin_Mission14_TaiidanReinforcementSpawn, 0);
    }
}

void Begin_Mission14_TaiidanReinforcementSpawn(void)
{
   AITeam *team;
   GrowSelection *spawnTarget;
   char *shipType;
   sdword shipCount;
   GrowSelection *createdShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   spawnTarget = NULL;
   shipType = NULL;
   shipCount = 0;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(104399073)))
   {
       spawnTarget = kasGrowSelectionPtr(239808609);
       shipType = "tHeavyCruiser";
       shipCount = kasfRandom(3, 5);
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(104399074))
         || kasfThisTeamIs(kasAITeamPtrIfExists(104399075)))
   {
       spawnTarget = kasGrowSelectionPtr(239808609);
       if (kasfRandom(1, 2) - 1 == 0)
       {
           shipType = "tStandardDestroyer";
           shipCount = kasfRandom(3, 5);
       }
       else
       {
           shipType = "tMissileDestroyer";
           shipCount = kasfRandom(2, 4);
       }
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(104399076))
         || kasfThisTeamIs(kasAITeamPtrIfExists(104399077)))
   {
       spawnTarget = kasGrowSelectionPtr(239808610);
       if (kasfRandom(1, 2) - 1 == 0)
       {
           shipType = "tIonCannonFrigate";
           shipCount = kasfRandom(2, 6);
       }
       else
       {
           shipType = "tStandardFrigate";
           shipCount = kasfRandom(2, 7);
       }
   }
   else if (kasfThisTeamIs(kasAITeamPtrIfExists(268338043)))
   {
       spawnTarget = kasGrowSelectionPtr(239808611);
       shipType = "tAdvanceSupportFrigate";
       shipCount = kasfRandom(3, 8);
   }
   if (spawnTarget != NULL)
   {
       createdShips = kasGrowSelectionPtr(21159);
       kasfShipsCreateInside(createdShips, shipType, shipCount, spawnTarget, 1);
       kasfReinforceTeamWithShips(team, createdShips);
   }
   kasAITeamShipsPtr(&team->shipList, 2);
}

void Begin_Mission14_TaiidanReinforcementFormWall(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationWall();
}

void Watch_Mission14_TaiidanReinforcementSpawn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
    {
        kasfSpeechEvent(74188670, (sdword)Begin_Mission14_TaiidanReinforcementFormWall, 0);
    }
}

void Watch_Mission14_TaiidanReinforcement_AwaitStop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectMoving(kasGrowSelectionPtr(0x53A0C), &team->shipList))
    {
        kasfSpeechEvent(261136005, (sdword)Begin_Mission14_TaiidanReinforcementAttackRun, 0);
    }
}

void Begin_Mission14_TaiidanReinforcementAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfEntryIncrement(-2102956012);
}

void Watch_Mission14_TaiidanReinforcementBuildCap(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-2102956012) == 6)
    {
        kasfSpeechEvent(175938357, (sdword)Begin_Mission14_TaiidanReinforcementResetDelay, 0);
    }
}

void Begin_Mission14_TaiidanReinforcementResetDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamHyperspaceInNear(kasVectorPtrIfExists(232266371), 4000);
    kasTimerCreateSetStartById(51324946, 15);
}

void Watch_Mission14_TaiidanReinforcementResetDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(51324946))
    {
        kasfSpeechEvent(117831748, (sdword)Init_Mission14_TaiidanReinforcementReset, 0);
    }
}

void Init_Mission14_MultigunCorvetteReinforcementTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
}

void Watch_Mission14_MultigunCorvetteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        kasfAttackMothership();
        kasTimerCreateSetStartById(51324946, 20);
    }
    if (kasTimerExpiredById(51324946))
    {
        kasfDeleteShips(&team->shipList);
    }
}

void Init_Mission14_IntroTargetDormant(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipSetTargetableFlag(&team->shipList, 0);
    kasfShipsSetMaxVelocity(&team->shipList, 0);
    kasfShipsSetNonRetaliation(&team->shipList);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfSpeechEvent(90805035, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission14_IntroTarget_ClampRadius50(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamHealthAverage() > 50)
    {
        kasfTeamSetPercentDamaged(50);
    }
}

void Watch_Mission14_MimicMCVTransferTrigger(void)
{
    GrowSelection *nearbyShips;
    GrowSelection *capturableShips;
    GrowSelection *playerOwnedShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0BB424F5))
    {
        kasfSpeechEvent(76060203, (sdword)MaintainTeamContext, 0);
    }
    else
    {
        if (!kasfEntryValueGet(157031925) && kasfShipsCount(kasAITeamShipsPtrIfExists(226320642)))
        {
            nearbyShips = kasGrowSelectionPtr(0x5296679);
            if (kasfShipsSelectNearby(nearbyShips, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 40000)
             && (kasfShipsSelectType(kasGrowSelectionPtr(0x801A8E3), nearbyShips, "sMimic")
              || kasfShipsSelectType(kasGrowSelectionPtr(0x801A8E3), nearbyShips, "sMCV")))
            {
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)133313269);
                kasfEntryGetOrCreateSetValue(157031925, 1);
            }
        }

        nearbyShips = kasGrowSelectionPtr(0x5296679);
        if (kasfFindEnemiesNearCurrentTeam(nearbyShips, 10000))
        {
            capturableShips = kasGrowSelectionPtr(0x801A8E3);
            playerOwnedShips = kasGrowSelectionPtr(0x80428E3);
            if (kasfShipsSelectType(capturableShips, nearbyShips, "sMimic")
             && kasfShipsSelectMembersOfSelection(playerOwnedShips, capturableShips, kasGrowSelectionPtr(0x4A2A29))
                < kasfShipsCount(capturableShips))
            {
                goto transfer_ship;
            }
            if (kasfShipsSelectType(capturableShips, nearbyShips, "sMCV")
             && kasfShipsSelectMembersOfSelection(playerOwnedShips, capturableShips, kasGrowSelectionPtr(0x4A2A29))
                < kasfShipsCount(capturableShips))
            {
transfer_ship:
                kasfEntryGetOrCreateSetValue(157031925, 1);
                kasfShipsRemove(capturableShips, playerOwnedShips);
                kasfShipsSelectFirst(capturableShips, capturableShips);
                kasfShipsSwitchPlayerOwner(capturableShips, 1);
                kasfReinforceTeamWithShips(kasAITeamPtrIfExists(134326499), capturableShips);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)250528645);
                kasfShipsSetDontApplyPhysicsFlag(capturableShips, 1);
                kasfSpeechEvent(76060203, (sdword)MaintainTeamContext, 0);
            }
        }
    }
}

void Watch_Mission14_TransferTeam_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        kasfAttackMothership();
        kasTimerCreateSetStartById(51324946, 20);
    }
    if (kasTimerExpiredById(51324946))
    {
        kasfDeleteShips(&team->shipList);
    }
}

void Init_Mission14_TransferTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission14_TransferTeam_FormOnIntroTarget(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetClamped(&team->shipList, 1);
    kasfShipsDockWithSelection(&team->shipList, kasAITeamShipsPtrIfExists(96166939));
}

void Init_Mission14_IntroActivateMeLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsDockStay(&((AITeam *)kasCurrentTeamPtr)->shipList, kasAITeamShipsPtrIfExists(96166939));
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)82233758);
    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)82233758);
}

void Watch_Mission14_IntroActivateMeLaunch(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552162) && kasfTeamFinishedLaunching())
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfShipsSetClamped(&team->shipList, 0);
        kasfShipsSwitchPlayerOwner(&team->shipList, 0);
        kasfShipsDockSupportWith(&team->shipList, kasGrowSelectionPtr(0xCFF5860));
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
        kasfShipsSetSupportFlag(&team->shipList, 1);
    }
}

void Init_Mission14_ResourcingTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsSetResourcingFlag((sdword)&team->shipList, 1);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfShipsSetMaxVelocity(&team->shipList, 20);
    kasfSpeechEvent(320211, (sdword)Begin_Mission12_SixShipFormationTeam_AttackMothership, 0);
}

void Init_Mission14_ReinforcementEscortTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(320211, (sdword)Begin_Mission12_SixShipFormationTeam_AttackMothership, 0);
}

void Watch_Mission14_ReinforcementEscortTeam_AwaitSignal(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0642166E))
    {
        kasfTeamHyperspaceInNear(kasVectorPtrIfExists(0xE099B35), 5000);
        kasTimerCreateSetStartById(227074530, 12);
    }
    if (kasTimerExpiredById(227074530))
    {
        kasfSpeechEvent(78199427, (sdword)Init_Mission14_ReinforcementEscortTeam_Regroup, 0);
    }
}

void Init_Mission14_ReinforcementEscortTeam_Regroup(void)
{
    GrowSelection *regroupTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTacticsAggressive();
    kasfFormationSphere();
    regroupTargets = kasGrowSelectionPtr(0x93C9634);
    kasfGrowSelectionClear(regroupTargets);
    kasfShipsAdd(regroupTargets, kasAITeamShipsPtrIfExists(170794736));
    kasfShipsAdd(regroupTargets, kasAITeamShipsPtrIfExists(142188961));
    kasfProtect(regroupTargets);
}

void Watch_Mission14_ReinforcementEscortTeam_Combat(void)
{
    GrowSelection *escortTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        kasfAttackMothership();
        kasfSpeechEvent(342540, (sdword)Init_AttackMothershipDeleteDelay, 0);
    }
    else if (!kasfShipsSelectTargets(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr(0x53A0C)))
    {
        escortTargets = kasGrowSelectionPtr(0x93C9634);
        kasfGrowSelectionClear(escortTargets);
        kasfShipsAdd(escortTargets, kasAITeamShipsPtrIfExists(170794736));
        kasfShipsAdd(escortTargets, kasAITeamShipsPtrIfExists(142188961));
        kasfProtect(escortTargets);
    }
}

void Init_Mission14_SelfDeleteTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamSwitchPlayerOwner(0);
}

void Watch_Mission14_SelfDeleteTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552164))
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfDeleteShips(&team->shipList);
    }
}

void Init_Mission14_ReinforcementGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    if (!GetGameDifficulty()
     && (kasfThisTeamIs(kasAITeamPtrIfExists(144353233))
      || kasfThisTeamIs(kasAITeamPtrIfExists(144353234))
      || kasfThisTeamIs(kasAITeamPtrIfExists(159083985))))
    {
        kasfDeleteShips(&team->shipList);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
    else
    {
        kasfSpeechEvent(190746231, (sdword)MaintainTeamContext, 0);
    }
}

void Watch_Mission14_ReinforcementGroup_AwaitSignal(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0469857B))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(144353233))
         || kasfThisTeamIs(kasAITeamPtrIfExists(144353234)))
        {
            kasfSpeechEvent(157715175, (sdword)Init_Mission14_ReinforcementGroup_AttackAssignedWing, 0);
        }
        else
        {
            kasfSpeechEvent(139967671, (sdword)Init_Mission14_ReinforcementGroup_SelectCapitalTargets, 0);
        }
    }
}

void Init_Mission14_ReinforcementGroup_SelectCapitalTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTacticsAggressive();
    kasfShipsSelectCapital(kasGrowSelectionPtr(0x764E81C), kasGrowSelectionPtr(0x4A2A29));
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x764E81C));
}

void Watch_Mission14_ReinforcementGroup_AttackRun(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *capitalShips;
    GrowSelection *fallbackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount() && kasfThisTeamIs(kasAITeamPtrIfExists(0x868E301)))
    {
        kasfSpeechEvent(
            0x8832365,
            (sdword)Init_Mission14_MultigunCorvetteReinforcementDelay,
            (sdword)Watch_Mission14_MultigunCorvetteReinforcementDelay);
        return;
    }

    if (kasfShipsAnyIdle(&team->shipList))
    {
        capitalShips = kasGrowSelectionPtr(0x764E81C);
        if (kasfShipsSelectCapital(capitalShips, kasGrowSelectionPtr(0x4A2A29)))
        {
            kasfShipsSortByDistanceToPoint(capitalShips, (vector *)kasThisTeamsVectorPtr());
            tutDeallocateRootRegion(capitalShips);
        }
        else
        {
            fallbackTargets = kasGrowSelectionPtr(0x36980EB);
            kasfGrowSelectionClear(fallbackTargets);
            kasfShipsAdd(fallbackTargets, kasGrowSelectionPtr(0x4A2A29));
            kasfShipsSortByDistanceToPoint(fallbackTargets, (vector *)kasThisTeamsVectorPtr());
            tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
        }
    }
}

void Init_Mission14_ReinforcementGroup_AttackAssignedWing(void)
{
    GrowSelection *assignedWing;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(144353233)))
    {
        assignedWing = kasAITeamShipsPtrIfExists(159083985);
    }
    else
    {
        assignedWing = kasAITeamShipsPtrIfExists(148598225);
    }
    kasfProtect(assignedWing);
}

void Watch_Mission14_ReinforcementGroup_AwaitWingClear(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552165))
    {
        if ((kasfThisTeamIs(kasAITeamPtrIfExists(144353233)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(159083985)))
         || (kasfThisTeamIs(kasAITeamPtrIfExists(144353234)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(148598225))))
        {
            kasfEntryGetOrCreateSetValue(71552165, 1);
            kasfSpeechEvent(139967671, (sdword)Init_Mission14_ReinforcementGroup_SelectCapitalTargets, 0);
        }
    }
}

void Init_Mission14_ReinforcementTrackedTargetStart(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(122047671, (sdword)Init_Mission14_ReinforcementFocusTrackedTarget, 0);
}

void Init_Mission14_ReinforcementFocusTrackedTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(106090545)))
    {
        kasfMoveTo(kasVectorPtrIfExists(0x691F554));
    }
    else
    {
        kasfPatrolPath(kasPathPtr(74019713));
    }
}

void Watch_Mission14_ReinforcementWaitForNearbyTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(72141345))
         || kasfThisTeamIs(kasAITeamPtrIfExists(72141346)))
        {
            kasfSpeechEvent(70903842, (sdword)Init_Mission14_ReinforcementBuildClosestTargetSelection, 0);
        }
        else
        {
            kasfSpeechEvent(175938357, (sdword)Init_Mission14_ReinforcementAttackMothershipDeleteTimer, 0);
        }
    }
    else if (kasfShipsSelectNearby(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x4A2A29), kasTeamsVectorPtr(96166939), 40000))
    {
        kasfSpeechEvent(139967671, (sdword)Init_Mission14_ReinforcementClearTrackedSelection, 0);
    }
}

void Watch_Mission14_ReinforcementRetargetWhenNoNearbyTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(72141345))
         || kasfThisTeamIs(kasAITeamPtrIfExists(72141346)))
        {
            kasfSpeechEvent(70903842, (sdword)Init_Mission14_ReinforcementBuildClosestTargetSelection, 0);
        }
        else
        {
            kasfSpeechEvent(175938357, (sdword)Init_Mission14_ReinforcementAttackMothershipDeleteTimer, 0);
        }
    }
    else if (!kasfShipsSelectNearby(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x4A2A29), kasTeamsVectorPtr(96166939), 40000))
    {
        kasfSpeechEvent(122047671, (sdword)Init_Mission14_ReinforcementFocusTrackedTarget, 0);
    }
}

void Init_Mission14_ReinforcementBuildClosestTargetSelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x4A2A29), kasTeamsVectorPtr(96166939), 25000))
    {
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasTeamsVectorPtr(96166939));
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
    }
}

void Watch_Mission14_ReinforcementAwaitIdleToBuildClosestTargetSelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(70903842, (sdword)Init_Mission14_ReinforcementBuildClosestTargetSelection, 0);
    }
}

void Init_Mission14_ReinforcementAttackMothershipDeleteTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfAttackMothership();
    kasTimerCreateSetStartById(148930818, 20);
}

void Watch_Mission14_ReinforcementSpecialHyperspaceTeam_AwaitLaunchSignal(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A5B6425))
    {
        kasfShipsHyperspaceIn(&team->shipList, NULL);
    }
}

void Init_Mission14_ReinforcementAttackCueTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    kasfSpeechEvent(21861, (sdword)Begin_Mission12_SixShipFormationTeam_AttackMothership, 0);
}

void Watch_Mission14_ReinforcementAttackCueTeam_AwaitPositionSignal(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x08A4E6C5))
    {
        kasfSpeechEvent(
            349316,
            (sdword)Init_Mission14_ReinforcementPositionAtAttackVector,
            (sdword)Watch_Mission14_ReinforcementMaintainAttackVectorSelection);
    }
}

void Init_Mission14_ReinforcementPositionAtAttackVector(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    hvector *attackVector;
    GrowSelection *targetSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x89B3A01)))
    {
        attackVector = kasVectorPtrIfExists(0x0B3AF5D1);
    }
    else
    {
        attackVector = kasVectorPtrIfExists(0x0B3AF5D2);
    }
    kasfTeamHyperspaceIn(attackVector);
    targetSelection = kasGrowSelectionPtr(0x36980EB);
    kasfGrowSelectionClear(targetSelection);
    kasfShipsAdd(targetSelection, kasGrowSelectionPtr(0x4A2A29));
    kasfShipsSortByDistanceToPoint(targetSelection, (vector *)kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(targetSelection);
}

void Watch_Mission14_ReinforcementMaintainAttackVectorSelection(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *targetSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        targetSelection = kasGrowSelectionPtr(0x36980EB);
        kasfGrowSelectionClear(targetSelection);
        kasfShipsAdd(targetSelection, kasGrowSelectionPtr(0x4A2A29));
        kasfShipsSortByDistanceToPoint(targetSelection, (vector *)kasThisTeamsVectorPtr());
        tutDeallocateRootRegion(targetSelection);
    }
}



void Init_Mission14_ReinforcementTriggerRouter_Resume(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
}

void Init_Mission14_ReinforcementTriggerRouter_StopAndClearCapitalSelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationWall();
    kasfTacticsAggressive();
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
}

void Watch_Mission14_ReinforcementTriggerRouter_AbortIfNoTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Watch_Mission14_ReinforcementTriggerRouter_ResumeAfterDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(74024315, (sdword)Init_Mission14_ReinforcementTriggerRouter_StopAndClearCapitalSelection, 0);
    }
}

/*-----------------------------------------------------------------------------
    Init_Mission14 - top-level KAS entry point (IDA: 0x636d30)
-----------------------------------------------------------------------------*/

extern void Init_CruiseMissileStrikeWave(void);
extern void Watch_BeastAcolyteWave_AwaitSignal(void);

void Init_Mission14(void)
{
    kasCurrentStep = 0;
    scope = KH_mission14;

    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));
    kasfDisableMothershipBigGunLaunch();
    kasfDisableAllAIFeatures();
    kasfActivateTech("TECH_INFECT");
    kasfActivateTech("TECH_FORCE_FIELD1");
    kasfActivateTech("TECH_FORCE_FIELD2");
    kasfActivateTech("TECH_ENERGY");
    kasfActivateTech("TECH_REPAIR");
    kasfActivateTech("TECH_SALVAGE");
    kasfActivateTech("TECH_CLOAKING");
    kasfActivateTech("TECH_AFTERBURNER");
    kasfActivateTech("TECH_EMP");
    kasfActivateTech("TECH_ADVANCED_SENSORS");
    kasfDisablePlayerHyperspace();

    kasFSMCreate(KH_FleetIntel, Init_Mission14_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission14_PrimaryObjectiveController,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(96166939, Init_Mission14_IntroTargetDormant, Watch_Mission14_IntroTarget_ClampRadius50,
                 kasAITeamPtrIfExists(96166939));
    kasFSMCreate(170794736, Init_Mission14_ResourcingTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(170794736));

    if (GetGameDifficulty() > 1)
    {
        kasFSMCreate(135358963, Init_CruiseMissileStrikeWave, Watch_NoOpTeamContext,
                     kasAITeamPtrIfExists(243787121));
        kasFSMCreate(108585267, Watch_NoOpTeamContext, Watch_BeastAcolyteWave_AwaitSignal,
                     kasAITeamPtrIfExists(136386561));
    }

    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(141091585));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(141091586));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(5792881));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(5792882));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(159083985));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(148598225));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(144353233));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(144353234));
    kasFSMCreate(5990497, Init_Mission14_ReinforcementGroup, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(85477153));

    kasFSMCreate(88512316, Init_Mission14_ReinforcementTrackedTargetStart, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(72141345));
    kasFSMCreate(88512316, Init_Mission14_ReinforcementTrackedTargetStart, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(72141346));
    kasFSMCreate(88512316, Init_Mission14_ReinforcementTrackedTargetStart, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(72141347));
    kasFSMCreate(88512316, Init_Mission14_ReinforcementTrackedTargetStart, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(106090545));

    kasFSMCreate(80123022, Init_Mission14_SelfDeleteTeam, Watch_Mission14_SelfDeleteTeam,
                 kasAITeamPtrIfExists(80123022));
    kasFSMCreate(134326499, Init_Mission14_TransferTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(134326499));
    kasFSMCreate(61264016, Init_Mission14_ReinforcementEscortTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(142188961));

    kasFSMCreate(103506811, Init_Mission14_TaiidanReinforcementTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(104399073));
    kasFSMCreate(103506811, Init_Mission14_TaiidanReinforcementTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(104399074));
    kasFSMCreate(103506811, Init_Mission14_TaiidanReinforcementTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(104399075));
    kasFSMCreate(103506811, Init_Mission14_TaiidanReinforcementTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(104399076));
    kasFSMCreate(103506811, Init_Mission14_TaiidanReinforcementTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(104399077));
    kasFSMCreate(103506811, Init_Mission14_TaiidanReinforcementTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(268338043));

    kasFSMCreate(109687717, Init_Mission14_ReinforcementAttackCueTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(144390657));
    kasFSMCreate(109687717, Init_Mission14_ReinforcementAttackCueTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(144390658));

    kasFSMCreate(112979315, Init_Mission14_ReinforcementSpecialHyperspaceTeam, Watch_Mission14_ReinforcementSpecialHyperspaceTeam_AwaitLaunchSignal,
                 kasAITeamPtrIfExists(93087345));
    kasFSMCreate(112979315, Init_Mission14_ReinforcementSpecialHyperspaceTeam, Watch_Mission14_ReinforcementSpecialHyperspaceTeam_AwaitLaunchSignal,
                 kasAITeamPtrIfExists(93087346));
    kasFSMCreate(112979315, Init_Mission14_ReinforcementSpecialHyperspaceTeam, Watch_Mission14_ReinforcementSpecialHyperspaceTeam_AwaitLaunchSignal,
                 kasAITeamPtrIfExists(93087347));

    kasFSMCreate(124218850, Init_Mission14_MultigunCorvetteReinforcementTeam, Watch_Mission14_MultigunCorvetteReinforcementTeam,
                 kasAITeamPtrIfExists(124217378));

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeEnemyCombatShips(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeInactivePlayerModules(kasGrowSelectionPtr(KH_Enemy), kasGrowSelectionPtr(18336307));
    kasfFindEnemyMothership(kasGrowSelectionPtr(211966048));

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
    kasfModuleSlotOrderSetShipType(1, "tHeavyInterceptor");
    kasfModuleSlotOrderSetShipType(1, "tAttackBomber");
    kasfModuleSlotOrderSetShipType(1, "tMultiGunCorvette");
    kasfModuleSlotOrderSetShipType(1, "tStandardFrigate");
    kasfModuleSlotOrderSetShipType(1, "tIonCannonFrigate");
    kasfModuleSlotOrderSetShipType(1, "tAdvanceSupportFrigate");
    kasfModuleSlotOrderSetCount(1, 3);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfFindBeastShipsInside(kasVolumePtr(51009377), kasGrowSelectionPtr(239808609));
    kasfFindBeastShipsInside(kasVolumePtr(51009378), kasGrowSelectionPtr(239808610));
    kasfFindBeastShipsInside(kasVolumePtr(51009379), kasGrowSelectionPtr(239808611));

    kasTimerCreateSetStartById(150120898, 1);
    kasfRUsSet(1, 100000);

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
    kasfAllowPlayerToResearch("TECH_REPAIRBOTS");
    kasfAllowPlayerToResearch("TECH_DRIVE_DESTROYER");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");

    kasFSMCreate(4627076, Init_Mission14_ReinforcementTriggerRouter, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(4627076));

    kasFSMCreate(284851, Init_Mission14_BeastACVReinforcementTeam, Init_Mission14_BeastACVReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(17798));
    kasFSMCreate(284851, Init_Mission14_BeastACVReinforcementTeam, Init_Mission14_BeastACVReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(284785));
    kasFSMCreate(284851, Init_Mission14_BeastACVReinforcementTeam, Init_Mission14_BeastACVReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(284786));

    kasFSMCreate(59670451, Init_Mission14_SentinelReinforcementTeam, Init_Mission14_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(171501548));
    kasFSMCreate(59670451, Init_Mission14_SentinelReinforcementTeam, Init_Mission14_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(59670129));
    kasFSMCreate(59670451, Init_Mission14_SentinelReinforcementTeam, Init_Mission14_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(59670130));
    kasFSMCreate(59670451, Init_Mission14_SentinelReinforcementTeam, Init_Mission14_SentinelReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(59670131));

    kasFSMCreate(11133235, Init_Mission14_BeastIonArrayFrigateReinforcementTeam, Init_Mission14_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(151690773));
    kasFSMCreate(11133235, Init_Mission14_BeastIonArrayFrigateReinforcementTeam, Init_Mission14_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(11133425));

    kasFSMCreate(34980115, Init_Mission14_HiveFrigateReinforcementTeam, Init_Mission14_HiveFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(186735637));
    kasFSMCreate(34980115, Init_Mission14_HiveFrigateReinforcementTeam, Init_Mission14_HiveFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(34980305));
    kasFSMCreate(34980115, Init_Mission14_HiveFrigateReinforcementTeam, Init_Mission14_HiveFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(34980306));

    kasFSMCreate(199714963, Init_Mission14_BeastMultiBeamFrigateReinforcementTeam, Init_Mission14_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(62813829));
    kasFSMCreate(199714963, Init_Mission14_BeastMultiBeamFrigateReinforcementTeam, Init_Mission14_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(199714897));
    kasFSMCreate(199714963, Init_Mission14_BeastMultiBeamFrigateReinforcementTeam, Init_Mission14_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(199714898));

    kasFSMCreate(249715395, Init_Mission14_BeastCloakedFighterReinforcementTeam, Init_Mission14_BeastCloakedFighterReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(200156578));
    kasFSMCreate(249715395, Init_Mission14_BeastCloakedFighterReinforcementTeam, Init_Mission14_BeastCloakedFighterReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(249715329));

    kasFSMCreate(128635107, Init_Mission14_BeastCorvetteReinforcementTeam, Init_Mission14_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(75148549));
    kasFSMCreate(128635107, Init_Mission14_BeastCorvetteReinforcementTeam, Init_Mission14_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(128634913));
    kasFSMCreate(128635107, Init_Mission14_BeastCorvetteReinforcementTeam, Init_Mission14_BeastCorvetteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(128634914));

    kasFSMCreate(230526675, Init_Mission14_BeastAcolyteReinforcementTeam, Init_Mission14_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(182180066));
    kasFSMCreate(230526675, Init_Mission14_BeastAcolyteReinforcementTeam, Init_Mission14_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526609));
    kasFSMCreate(230526675, Init_Mission14_BeastAcolyteReinforcementTeam, Init_Mission14_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526610));
    kasFSMCreate(230526675, Init_Mission14_BeastAcolyteReinforcementTeam, Init_Mission14_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526611));
    kasFSMCreate(230526675, Init_Mission14_BeastAcolyteReinforcementTeam, Init_Mission14_BeastAcolyteReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(230526612));

    kasFSMCreate(80024947, Init_Mission14_BeastRammingFrigateReinforcementTeam, Init_Mission14_BeastRammingFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(223105365));
    kasFSMCreate(80024947, Init_Mission14_BeastRammingFrigateReinforcementTeam, Init_Mission14_BeastRammingFrigateReinforcementTeam_Reevaluate,
                 kasAITeamPtrIfExists(80025009));

    kasFSMCreate(44665972, Init_Mission14_ReinforcementPool, Fill_Mission14_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(44665972));

    kasfShipsAdd(kasGrowSelectionPtr(4627076), kasAITeamShipsPtrIfExists(170794736));

    kasFSMCreate(174875315, Init_Mission14_ReinforcementTriggerRouter_Resume, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(171814892));

    kasfShipsApplyPlayerUpgrades(kasAITeamShipsPtrIfExists(226320642));
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(226320642), 2);
}
