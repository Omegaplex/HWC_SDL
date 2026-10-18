/*=============================================================================
    Name    : mission17.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission17
=============================================================================*/

#include "MissionScriptSupport.h"

typedef udword _DWORD;

#define sub_4907E0 kasAITeamShipsPtrIfExists
#define sub_4907A0 kasAITeamPtrIfExists
#define sub_491F90 kasTimerCreateSetStartById
#define sub_493AF0 kasfShipsSetPercentDamaged
#define sub_494A00 kasfMsgSend
#define sub_497F10 kasfModuleSlotOrderSetShipType
#define sub_497F70 kasfModuleSlotOrderSetCount
#define sub_498230 kasfDisablePlayerHyperspace
#define GetGameDifficulty_ GetGameDifficulty

void Begin_SentinelCombatRetryDelay(void);
void Watch_Mission05_OEM_MissionFlow_DontGoThere(void);
void Begin_FleetIntel_CruiseMissileWaveSetup(void);
void Init_FleetIntel_SelectInvestigationTarget(void);
void Watch_FleetIntel_InvestigateTarget(void);
void Init_FleetIntel_CruiseMissileWaveDelay(void);
void Watch_FleetIntel_CruiseMissileWaveDelay(void);
void Init_FleetIntel_CruiseMissileWaveTimer(void);
void Watch_FleetIntel_CruiseMissileWaveSpawner(void);
void Begin_BeastMultiBeamFrigateCombatLoop(void);
void Begin_BeastIonArrayFrigateCombatLoop(void);
void Begin_BeastCloakedFighterCombatLoop(void);
void Init_ReinforcementPool_ClearSelectionAndReset(void);
void Init_Mission09_AssaultEscort_Cooldown(void);
void Init_Mission05_FrigateReinforcementDelay(void);
void Init_Mission11_SentinelReinforcementBuildTimer(void);
void Init_Mission16_BeastACVReinforcementBuildTimer(void);
void Fill_Mission16_ReinforcementPoolFromParade(void);
void Init_BeastWorkerHarvestReinforcementTeam(void);
void Watch_BeastWorkerHarvestReinforcementTeam(void);
void Init_BeastProcessorReinforcementTeam(void);
void Watch_BeastProcessorReinforcementTeam(void);
void Init_BeastWorkerRepairReinforcementTeam_Preamble(void);
void Watch_BeastWorkerRepairReinforcementTeam(void);
void Init_Mission08_HiveFrigateReinforcementTeam(void);
void Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate(void);
void Init_Mission17_BeastMultiBeamFrigateReinforcementTeam(void);
void Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastMultiBeamFrigateReinforcementTeam(void);
void Watch_Mission17_BeastMultiBeamFrigateReinforcementBuildCount(void);
void Begin_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Watch_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Watch_Mission17_BeastMultiBeamFrigateCombatLoop(void);
void Init_Mission17_BeastMultiBeamFrigateCombatTeam(void);
void Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate(void);
void Watch_Mission17_BeastMultiBeamFrigateCombatTeam(void);
void Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildCount(void);
void Begin_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer(void);
void Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer(void);
void Begin_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun(void);
void Watch_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun(void);
void Init_Mission17_ReinforcementPool(void);
void Watch_Mission17_ReinforcementPool_Combat(void);
void Watch_Mission17_ReinforcementPool_Resume(void);
void Init_Mission17_BeastACVReinforcementWave(void);
void Init_Mission17_BeastACVReinforcementWave_Reevaluate(void);
void Watch_Mission17_BeastACVReinforcementWave(void);
void Watch_Mission17_BeastACVReinforcementBuildCount(void);
void Watch_Mission17_BeastACVReinforcementSpawn(void);
void Watch_Mission17_BeastACVDockAndReinforce(void);
void Init_Mission17_BeastIonArrayFrigateReinforcementTeam(void);
void Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastIonArrayFrigateReinforcementTeam(void);
void Watch_Mission17_BeastIonArrayFrigateReinforcementBuildCount(void);
void Watch_Mission17_BeastIonArrayFrigateReinforcementSpawn(void);
void Begin_Mission17_BeastIonArrayFrigateAttackRun(void);
void Watch_Mission17_BeastIonArrayFrigateCombatLoop(void);
void Watch_Mission17_BeastIonArrayFrigateAttackRun(void);
void Init_Mission17_BeastCloakedFighterReinforcementTeam(void);
void Init_Mission17_BeastCloakedFighterReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastCloakedFighterReinforcementTeam(void);
void Watch_Mission17_BeastCloakedFighterReinforcementBuildCount(void);
void Begin_Mission17_BeastCloakedFighterReinforcementBuildTimer(void);
void Watch_Mission17_BeastCloakedFighterReinforcementSpawn(void);
void Watch_Mission17_BeastCloakedFighterCombatLoop(void);
void Begin_Mission17_BeastCloakedFighterAttackRun(void);
void Watch_Mission17_BeastCloakedFighterAttackRun(void);
sdword Begin_Mission17_BeastCloakedFighterDockAndReinforce(void);
void Watch_Mission17_BeastCloakedFighterDockAndReinforce(void);
void Watch_Mission17_BeastCloakedFighterLaunchDockedReinforcements(void);
void Begin_BeastRammingFrigateCombatLoop(void);
void Begin_BeastHeavyCruiserReinforcementBuildTimer(void);
void Begin_BeastCarrierReinforcementBuildTimer(void);
void Begin_BeastACVCombatLoopDelay(void);
void Begin_Mission17_ReinforcementAttackRun(void);
void Begin_Mission17_CarrierCombatTeamLoop(void);
void Watch_Mission17_CarrierCombatTeamLoop(void);
void Watch_Mission17_CarrierCombatTeamAttackRun(void);
void Init_Mission17_CarrierCombatTeam(void);
void Init_Mission17_CarrierCombatTeam_Reevaluate(void);
void Watch_Mission17_HeavyCruiserCombatTeamBuildCount(void);
void Begin_Mission17_HeavyCruiserCombatTeamBuildTimer(void);
void Watch_Mission17_HeavyCruiserCombatTeamBuildTimer(void);
void Begin_Mission17_HeavyCruiserCombatTeamAttackStaging(void);
void Watch_Mission17_HeavyCruiserCombatTeamAttackStaging(void);
void Init_Mission17_BeastACVReinforcementTeam(void);
void Init_Mission17_BeastACVReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastACVReinforcementTeam(void);
void Init_Mission17_IonCannonFrigateReinforcementTeam(void);
void Init_Mission17_IonCannonFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission17_IonCannonFrigateReinforcementTeam(void);
void Watch_Mission17_IonCannonFrigateReinforcementBuildCount(void);
void Begin_Mission17_IonCannonFrigateReinforcementBuildTimer(void);
void Watch_Mission17_IonCannonFrigateReinforcementBuildTimer(void);
void Begin_Mission17_IonCannonFrigateCombatLoop(void);
void Watch_Mission17_IonCannonFrigateCombatRetryDelay(void);
void Init_Mission17_CarrierReinforcementTeam(void);
void Init_Mission17_CarrierReinforcementTeam_Reevaluate(void);
void Watch_Mission17_CarrierReinforcementTeam(void);
void Watch_Mission17_CarrierReinforcementBuildCount(void);
void Begin_Mission17_CarrierReinforcementBuildTimer(void);
void Watch_Mission17_CarrierReinforcementBuildTimer(void);
void Begin_Mission17_CarrierCombatLoop(void);
void Begin_Mission17_CarrierCombatAttackRun(void);
void Watch_Mission17_CarrierCombatLoop(void);
void Watch_Mission17_CarrierCombatRetryDelay(void);
void Begin_Mission17_HeavyInterceptorCombatLoop(void);
void Watch_Mission17_HeavyInterceptorCombatLoop(void);
void Init_Mission17_HeavyInterceptorCombatTeam(void);
void Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate(void);
void Watch_Mission17_HeavyInterceptorCombatTeam(void);
void Watch_Mission17_HeavyInterceptorCombatTeamBuildCount(void);
void Begin_Mission17_HeavyInterceptorCombatTeamBuildTimer(void);
void Watch_Mission17_HeavyInterceptorCombatTeamBuildTimer(void);
void Init_Mission17_HeavyInterceptorDockAndReinforce(void);
void Watch_Mission17_HeavyInterceptorDockAndReinforce(void);
void Init_Mission17_AdvanceSupportFrigateCombatTeam(void);
void Init_Mission17_AdvanceSupportFrigateCombatTeam_Reevaluate(void);
void Watch_Mission17_AdvanceSupportFrigateCombatTeam(void);
void Watch_Mission17_AdvanceSupportFrigateCombatTeamBuildCount(void);
void Watch_Mission17_AdvanceSupportFrigateCombatTeamBuildTimer(void);
void Begin_Mission17_AdvanceSupportFrigateDefenseLoop(void);
void Watch_Mission17_AdvanceSupportFrigateDefenseLoop(void);
void Begin_Mission17_AdvanceSupportFrigateRepairOrder(void);
void Watch_Mission17_AdvanceSupportFrigateRepairOrder(void);
void Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam(void);
void Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam_Reevaluate(void);
void Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeam(void);
void Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildCount(void);
void Begin_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer(void);
void Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer(void);
void Begin_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop(void);
void Watch_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop(void);
void Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder(void);
void Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder(void);
void Init_Mission17_TaiidanStandardDestroyerCombatTeam(void);
void Init_Mission17_TaiidanStandardDestroyerCombatTeam_Reevaluate(void);
void Watch_Mission17_TaiidanStandardDestroyerCombatTeam(void);
void Watch_Mission17_TaiidanStandardDestroyerCombatTeamBuildCount(void);
void Watch_Mission17_TaiidanStandardDestroyerCombatTeamBuildTimer(void);
void Begin_Mission17_TaiidanStandardDestroyerCombatLoop(void);
void Begin_Mission17_TaiidanReinforcementAttackRun(void);
void Watch_Mission17_TaiidanStandardDestroyerCombatRetryDelay(void);
void Init_Mission17_TaiidanIonCannonFrigateCombatTeam(void);
void Init_Mission17_TaiidanIonCannonFrigateCombatTeam_Reevaluate(void);
void Watch_Mission17_TaiidanIonCannonFrigateCombatTeam(void);
void Watch_Mission17_TaiidanIonCannonFrigateCombatTeamBuildCount(void);
void Watch_Mission17_TaiidanIonCannonFrigateCombatTeamBuildTimer(void);
void Begin_Mission17_TaiidanIonCannonFrigateCombatLoop(void);
void Watch_Mission17_TaiidanIonCannonFrigateCombatRetryDelay(void);
void Init_Mission17_StandardDestroyerCombatTeam(void);
void Init_Mission17_StandardDestroyerCombatTeam_Reevaluate(void);
void Watch_Mission17_StandardDestroyerCombatTeam(void);
void Watch_Mission17_StandardDestroyerCombatTeamBuildCount(void);
void Begin_Mission17_StandardDestroyerCombatTeamBuildTimer(void);
void Watch_Mission17_StandardDestroyerCombatTeamBuildTimer(void);
void Begin_Mission17_StandardDestroyerCombatLoop(void);
void Watch_Mission17_StandardDestroyerCombatRetryDelay(void);
void Init_Mission17_MultigunCorvetteReinforcementTeam(void);
void Init_Mission17_MultigunCorvetteReinforcementTeam_Reevaluate(void);
void Watch_Mission17_MultigunCorvetteReinforcementTeam(void);
void Watch_Mission17_MultigunCorvetteReinforcementBuildCount(void);
void Begin_Mission17_MultigunCorvetteReinforcementBuildTimer(void);
void Watch_Mission17_MultigunCorvetteReinforcementBuildTimer(void);
void Watch_Mission17_MultigunCorvetteCombatLoop(void);
void Watch_Mission17_MultigunCorvetteDockAndReinforce(void);
void Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam(void);
void Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate(void);
void Watch_Mission17_TaiidanMultigunCorvetteReinforcementTeam(void);
void Watch_Mission17_TaiidanMultigunCorvetteReinforcementBuildCount(void);
void Watch_Mission17_TaiidanMultigunCorvetteReinforcementBuildTimer(void);
void Begin_Mission17_TaiidanMultigunCorvetteCombatLoop(void);
void Watch_Mission17_TaiidanMultigunCorvetteCombatLoop(void);
void Begin_Mission17_TaiidanMultigunCorvetteAttackRun(void);
void Watch_Mission17_TaiidanMultigunCorvetteCombatRetryDelay(void);
void Begin_Mission17_TaiidanMultigunCorvetteDockAndReinforce(void);
void Watch_Mission17_TaiidanMultigunCorvetteDockAndReinforce(void);
void Init_Mission17_TaiidanMultigunCorvetteCombatTeam(void);
void Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate(void);
void Begin_Mission17_TaiidanMultigunCorvetteReinforcementArrival(void);
void Watch_Mission17_TaiidanMultigunCorvetteReinforcementArrival(void);
void Init_Mission17_HeavyCruiserReinforcementTeam(void);
void Init_Mission17_HeavyCruiserReinforcementTeam_Reevaluate(void);
void Watch_Mission17_HeavyCruiserReinforcementTeam(void);
void Watch_Mission17_HeavyCruiserReinforcementBuildCount(void);
void Watch_Mission17_HeavyCruiserReinforcementBuildTimer(void);
void Begin_Mission17_HeavyCruiserCombatLoop(void);
void Begin_Mission17_HeavyCruiserCombatAttackRun(void);
void Watch_Mission17_HeavyCruiserCombatRetryDelay(void);
void Begin_Mission17_BeastACVReinforcementCombatLoop(void);
void Watch_Mission17_BeastACVReinforcementCombatLoop(void);
void Watch_Mission17_BeastACVAttackRun(void);
void Watch_Mission17_WaitForMissionCompleteIntelEvent(void);
void Watch_Mission17_WaitForIntelEventEnd(void);
void Watch_Mission17_MissionFlow_Delay(void);
void Watch_Mission17_MissionFlow(void);
void Init_Mission17_BeastHeavyCruiserReinforcementTeam(void);
void Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate(void);
void Init_Mission17_BeastHeavyCruiserCombatTeam(void);
void Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate(void);
void Begin_Mission17_BeastHeavyCruiserSpecialAttack(void);
void Watch_Mission17_BeastHeavyCruiserSpecialAttack(void);
void Watch_Mission17_BeastHeavyCruiserCombatTeam(void);
void Watch_Mission17_BeastHeavyCruiserCombatTeamBuildCount(void);
void Begin_Mission17_BeastHeavyCruiserCombatTeamBuildTimer(void);
void Watch_Mission17_BeastHeavyCruiserCombatTeamBuildTimer(void);
void Begin_Mission17_BeastHeavyCruiserCombatTeam_SpecialAttack(void);
void Watch_Mission17_BeastHeavyCruiserCombatTeam_SpecialAttack(void);
void Begin_Mission17_BeastHeavyCruiserCombatTeam_AttackRun(void);
void Watch_Mission17_BeastHeavyCruiserCombatTeam_AttackRun(void);
void Begin_Mission17_BeastHeavyCruiserAttackRun(void);
void Begin_Mission17_BeastHeavyCruiserAttackRetryDelay(void);
void Watch_Mission17_BeastHeavyCruiserAttackRetryDelay(void);
void Begin_Mission17_BeastHeavyCruiserReinforcementTeam_AttackRun(void);
void Watch_Mission17_BeastHeavyCruiserAttackRun(void);
void Watch_Mission17_BeastHeavyCruiserReinforcementTeam(void);
void Init_Mission17_BeastHeavyCruiserReinforcementBuildTimer(void);
void Watch_Mission17_BeastHeavyCruiserReinforcementBuildTimer(void);
void Init_Mission17_BeastHeavyCruiserAttackMothership(void);
void Watch_Mission17_BeastHeavyCruiserCombatLoop(void);
void Begin_Mission17_FinalDFGFrigateCombatLoop(void);
void Init_Mission17_FinalDFGFrigateCombatTeam(void);
void Watch_Mission17_FinalDFGFrigateCombatLoop(void);
void Watch_Mission17_FinalDFGFrigateCombatTeamBuildTimer(void);
void Watch_Mission17_FinalDFGFrigateDockAndReinforce(void);
void sub_663CE0(void);
void sub_663E60(void);
void Watch_Mission17_ObjectiveController(void);
void Init_Mission17_BentusiAttackMothership(void);
void Watch_Mission17_BentusiAttackMothership(void);
void Move_Mission17_BentusiCloseToPlayer(void);
void Watch_Mission17_BentusiResumeAttack(void);
void Init_Mission17_NomadMoonAttackMothership(void);
void Watch_Mission17_NomadMoonAttackMothership(void);
void Init_Mission17_NaggarokBossLoop(void);
void Watch_Mission17_NaggarokBossLoop(void);
void Begin_Mission17_NaggarokFallbackMoveFar(void);
void Watch_Mission17_NaggarokFallbackMoveFar(void);
void Begin_Mission17_NaggarokFallbackMoveNearPlayer(void);
void Watch_Mission17_NaggarokFallbackMoveNearPlayer(void);
void Begin_Mission17_NaggarokWorkerBaitRun(void);
void Watch_Mission17_NaggarokWorkerBaitRun(void);
void Begin_Mission17_NaggarokCruiseMissileLaunchSfx(void);
void Watch_Mission17_NaggarokCruiseMissileLaunchSfx(void);
void Begin_Mission17_NaggarokSalvageTrap(void);
void Watch_Mission17_NaggarokSalvageTrap(void);
void Watch_Mission17_NaggarokRevealCue(void);
void Init_Mission17_CrystalPointTrigger(void);
void Watch_Mission17_CrystalPointTrigger(void);
void Init_Mission17_BeastCarrierReinforcementTeam(void);
void Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate(void);
void Begin_Mission17_BeastCarrierSpecialAttack(void);
void Watch_Mission17_BeastCarrierSpecialAttack(void);
void Begin_Mission17_BeastCarrierReinforcementTeam_AttackRun(void);
void Watch_Mission17_BeastCarrierReinforcementTeam_AttackRun(void);
void Watch_Mission17_BeastCarrierReinforcementTeam(void);
void Init_Mission17_BeastCarrierReinforcementBuildTimer(void);
void Watch_Mission17_BeastCarrierReinforcementBuildTimer(void);
void Init_Mission17_BeastCarrierAttackMothership(void);
void Begin_Mission17_BeastCarrierCombatLoop(void);
void Watch_Mission17_BeastCarrierCombatLoop(void);
void Begin_Mission17_BeastCarrierAttackRun(void);
void Watch_Mission17_BeastCarrierAttackRun(void);
void Init_Mission17_BeastCorvetteReinforcementTeam(void);
void Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastCorvetteReinforcementTeam(void);
void Init_Mission17_BeastCorvetteReinforcementBuildTimer(void);
void Watch_Mission17_BeastCorvetteReinforcementBuildTimer(void);
void Begin_Mission17_BeastCorvetteCombatLoop(void);
void Watch_Mission17_BeastCorvetteCombatLoop(void);
void Init_Mission17_BeastCorvetteCapitalStrike(void);
void Watch_Mission17_BeastCorvetteCapitalStrike(void);
void Watch_Mission17_BeastCorvetteDockAndReinforce(void);
sdword Begin_Mission17_BeastReconDockAndReinforce(void);
void Init_Mission17_BeastFighterReinforcementTeam(void);
void Init_Mission17_BeastFighterReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastFighterReinforcementTeam(void);
void Init_Mission17_BeastFighterReinforcementBuildTimer(void);
void Watch_Mission17_BeastFighterReinforcementBuildTimer(void);
void Init_Mission17_BeastFighterCombatLoop(void);
void Watch_Mission17_BeastFighterCombatLoop(void);
void Init_Mission17_BeastFighterCapitalStrike(void);
void Watch_Mission17_BeastFighterCapitalStrike(void);
void Watch_Mission17_BeastFighterDockAndReinforce(void);
void Begin_BeastReconReinforcementBuildTimer(void);
void Init_Mission17_BeastReconReinforcementTeam(void);
void Init_Mission17_BeastReconReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastReconReinforcementTeam(void);
void Init_Mission17_BeastReconReinforcementBuildTimer(void);
void Watch_Mission17_BeastReconReinforcementBuildTimer(void);
void Begin_Mission17_BeastReconCombatLoop(void);
void Watch_Mission17_BeastReconCombatLoop(void);
void Begin_Mission17_BeastReconCombatAttackRun(void);
void Watch_Mission17_BeastReconCombatAttackRun(void);
void Watch_Mission17_BeastReconDockAndReinforce(void);
void Begin_BeastCruiseMissileReinforcementBuildTimer(void);
void Init_Mission17_BeastCruiseMissileReinforcementTeam(void);
void Init_Mission17_BeastCruiseMissileReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastCruiseMissileReinforcementTeam(void);
void Init_Mission17_BeastCruiseMissileReinforcementBuildTimer(void);
void Watch_Mission17_BeastCruiseMissileReinforcementBuildTimer(void);
void Begin_Mission17_BeastCruiseMissileCombatLoop(void);
void Watch_Mission17_BeastCruiseMissileCombatLoop(void);
void Begin_Mission16_BeastDFGFrigateDockAndReinforce(void);
void Begin_Mission17_BeastDFGFrigateDockAndReinforce(void);
void Begin_Mission17_BeastDFGFrigateCombatLoop(void);
void Watch_BeastDFGFrigateCombatLoop(void);
void Begin_BeastDFGFrigateReinforcementBuildTimer(void);
void Init_Mission17_BeastDFGFrigateReinforcementTeam(void);
void Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastDFGFrigateReinforcementTeam(void);
void Init_Mission17_BeastDFGFrigateReinforcementBuildTimer(void);
void Watch_Mission17_BeastDFGFrigateReinforcementBuildTimer(void);
void Watch_Mission17_BeastDFGFrigateDockAndReinforce(void);
void Init_Mission17_BeastRammingFrigateReinforcementTeam(void);
void Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate(void);
void Watch_Mission17_BeastRammingFrigateReinforcementTeam(void);
void Init_Mission17_BeastRammingFrigateReinforcementBuildTimer(void);
void Watch_Mission17_BeastRammingFrigateReinforcementBuildTimer(void);
void Watch_Mission17_BeastRammingFrigateCombatLoop(void);
void Begin_Mission17_BeastSentinelReinforcementCombatLoop(void);
void Init_BeastSentinelReinforcementTeam(void);
void Init_BeastSentinelReinforcementTeam_Reevaluate(void);
void Watch_BeastSentinelReinforcementTeam(void);
void Watch_BeastSentinelReinforcementBuildCount(void);
void Watch_BeastSentinelReinforcementSpawn(void);
void Watch_Mission17_BeastSentinelAssault(void);
static void Begin_Mission17_BeastACVCombatAttackRun(void);

static char *m17_saveInfoName[] =
{
    "Naval Base Alpha",
    "French version goes here",
    "German version goes here",
};

static const char *m17_locationCard[] =
{
    "Republican Naval Base Alpha",
    "French version goes here",
    "German version goes here",
};

static const char *m17_researchInertialessDrive_brief[] =
{
    "Research Inertialess Drive",
    "French version goes here",
    "German version goes here",
};

static const char *m17_researchInertialessDrive_full[] =
{
    "We need to find a way to disable the NAGGAROK's inertialess drive.  Research this topic immediately.  This technology is available in the Advanced Engineering module.",
    "French version goes here",
    "German version goes here",
};

static const char *m17_reconHyperspaceSignatures_brief[] =
{
    "Recon Hyperspace Signatures",
    "French version goes here",
    "German version goes here",
};

static const char *m17_reconHyperspaceSignatures_full[] =
{
    "We need to know what those signatures mean.  Send some RECONS to investigate.",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyEmitters_brief[] =
{
    "Destroy Moon Repulse Emitters",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyEmitters_full[] =
{
    "The best ship to destroy the two emitters near the NOMAD MOON are LEECHES.  Send in as many as you can to take them out.  Make sure that no enemy PROXIMITY SENSORS, SENTINELS, or RECONS are nearby or the LEECHES will be spotted and destroyed.",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyEmittersFullHard[] =
{
    "Those two emitters allow the NOMAD MOON to fire its repulse field.  We must destroy those emitters in order to destroy the station.",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyNomadMoon_brief[] =
{
    "Destroy NOMAD MOON",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyNomadMoon_full[] =
{
    "In order to get your large ships close to the moon, you must first disable the repulse emitters.  Then you will be able to bring in your fleet to engage the moon itself.",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyNaggarok_brief[] =
{
    "Destroy the NAGGAROK",
    "French version goes here",
    "German version goes here",
};

static const char *m17_destroyNaggarok_full[] =
{
    "The full capabilities of this alien vessel are not known.  Attack with caution.",
    "French version goes here",
    "German version goes here",
};

static const char *m17_disableAndDestroyNaggarok_brief[] =
{
    "Disable and Destroy the NAGGAROK",
    "French version goes here",
    "German version goes here",
};

static const char *m17_disableAndDestroyNaggarok_full[] =
{
    "Our research suggests the NAGGAROK can be disabled with successive EMP bursts.  Use our ACVs to disrupt the its inertialess drive.  Once the drive is disabled, the NAGGAROK will not be able to move.",
    "French version goes here",
    "German version goes here",
};

void Watch_Mission17(void)
{
  int v0; // eax
  int *v1; // eax
  int *v2; // eax
  int v3; // eax
  int v4; // eax
  int *v5; // eax
  signed int v6; // eax
  int v7; // eax
  signed int v8; // eax
  int *v9; // eax
  int *v10; // eax
  int v11; // eax
  int *v12; // eax
  int v13; // eax
  int v14; // eax
  int *v15; // eax
  int v16; // eax
  int *v17; // eax
  int v18; // eax
  int v19; // eax
  int *v20; // eax
  int *v21; // eax
  int v22; // eax
  int *v23; // eax
  int v24; // eax
  int *v25; // eax
  int *v26; // eax
  int *v27; // eax
  int *v28; // eax
  int *v29; // eax
  int v31; // eax
  int *v32; // eax
  int v33; // [esp-8h] [ebp-8h]
  int v34; // [esp-8h] [ebp-8h]
  int v35; // [esp-4h] [ebp-4h]
  int v36; // [esp-4h] [ebp-4h]
  int v37; // [esp-4h] [ebp-4h]
  int v38; // [esp-4h] [ebp-4h]
  int v39; // [esp-4h] [ebp-4h]
  int *v40; // [esp-4h] [ebp-4h]
  int v41; // [esp-4h] [ebp-4h]
  int v42; // [esp-4h] [ebp-4h]
  int v43; // [esp-4h] [ebp-4h]
  int *v44; // [esp-4h] [ebp-4h]
  int v45; // [esp-4h] [ebp-4h]
  int v46; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503623;
  if ( kasTimerExpiredDestroyById(104338707) )
  {
    v35 = kasGrowSelectionPtr(18336307);
    v0 = kasVolumePtr(132933767);
    kasfFindShipsInside(v0, v35);
    v36 = kasGrowSelectionPtr(18336307);
    v1 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v1, v36);
    v37 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v2, v37);
    v38 = kasAITeamShipsPtrIfExists(96166939);
    v3 = kasGrowSelectionPtr(134620372);
    if ( kasfShipsSelectAllAttachedShips(v3, v38) )
    {
      v39 = kasGrowSelectionPtr(134620372);
      v4 = kasGrowSelectionPtr(0x4A2A29);
      kasfShipsRemove(v4, v39);
    }
    if ( kasfEntryValueGet(-1989827070) )
    {
      v40 = (int *)kasGrowSelectionPtr(140367886);
      v5 = (int *)kasGrowSelectionPtr(0x4A2A29);
      kasfShipsAdd(v5, v40);
    }
    v6 = kasfRandom(5, 10);
    kasTimerSetById(104338707, v6);
    kasTimerStartById(104338707);
  }
  if ( kasTimerExpiredDestroyById(220447043) )
  {
    if ( kasfRUsGet(1) <= 100000 )
    {
      v7 = kasfRUsGet(1);
      kasfRUsSet(1, v7 + 20000);
    }
    v8 = kasfRandom(300, 1000);
    kasTimerSetById(220447043, v8);
  }
  if ( !kasfEntryValueGet(71552176) && kasfEntryValueGet(-2015429404) && kasfIntelEventEnded() )
  {
    kasfEntrySetValue(71552176, 1);
    v41 = kasGrowSelectionPtr(18336307);
    v9 = (int *)kasGrowSelectionPtr(89157026);
    kasSelectPlayerShipsSimple(v9, v41);
    v42 = kasGrowSelectionPtr(89157026);
    v10 = (int *)kasGrowSelectionPtr(89157026);
    kasfShipsSelectNotSalvagers(v10, v42);
    v43 = kasGrowSelectionPtr(218060896);
    v11 = kasGrowSelectionPtr(82017669);
    if ( kasfShipsSelectLinkedShips(v11, v43) )
    {
      v44 = (int *)kasGrowSelectionPtr(82017669);
      v12 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsAdd(v12, v44);
    }
    if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
    {
      v33 = kasGrowSelectionPtr(89157026);
      v13 = kasGrowSelectionPtr(96759714);
      if ( !kasfShipsSelectType(v13, v33, "sWorker") )
      {
        v34 = kasGrowSelectionPtr(218060896);
        v14 = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(v14, "sWorker", 1, v34, 0);
      }
    }
    v15 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v15, (char *)119169809);
    spSetSinglePlayerSaveInfo(17, m17_saveInfoName[strCurLanguage]);
  }
  if ( !kasfEntryValueGet(71552177) )
  {
    if ( kasfEntryValueGet(-1933726299) )
    {
      v16 = kasGrowSelectionPtr(143614148);
      if ( kasfShipsAnyInSensorRange(v16) )
      {
        kasfEntrySetValue(71552177, 1);
        v17 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v17, (char *)143614148);
      }
    }
  }
  v18 = kasAITeamShipsPtrIfExists(22630782);
  if ( kasfShipsCount(v18) && (v19 = kasAITeamShipsPtrIfExists(22630782), kasfShipsHealthAverage(v19) <= 60) || kasTimerExpiredById(61565948) )
  {
    v20 = kasAITeamPtrIfExists(40027877);
    kasfMsgSend((struct AITeam *)(int)v20, (char *)5801588);
    kasTimerSetById(126381250, 5);
  }
  if ( kasTimerExpiredById(126381250) )
  {
    v21 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v21, (char *)74556980);
  }
  if ( !kasfEntryValueGet(71552178) )
  {
    v22 = kasAITeamShipsPtrIfExists(22630782);
    if ( kasfShipsAnyInSensorRange(v22) )
    {
      kasfEntrySetValue(71552178, 1);
      v23 = kasAITeamPtrIfExists(176850956);
      kasfMsgSend((struct AITeam *)(int)v23, (char *)74556979);
    }
  }
  if ( kasfEntryValueGet(-2103308245) )
  {
    v24 = kasGrowSelectionPtr(140367886);
    if ( kasfShipsCount(v24) )
    {
      if ( kasTimerExpiredById(-1989827070) )
      {
        v25 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v25, (char *)74556982);
      }
    }
  }
  if ( !kasfEntryValueGet(71552179) && kasfDoesPlayerHave("TECH_INERTIALESS_DRIVE") )
  {
    kasfEntrySetValue(71552179, 1);
    v26 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v26, (char *)157488868);
  }
  if ( !kasfEntryValueGet(71552180) )
  {
    if ( kasfEntryValueGet(-2103308245) )
    {
      v45 = kasGrowSelectionPtr(140367886);
      v27 = (int *)kasGrowSelectionPtr(124053532);
      if ( !kasfShipsSelectCapital(v27, v45) )
      {
        kasfEntrySetValue(71552180, 1);
        kasTimerSetById(0x00906500, 300);
      }
    }
  }
  if ( kasTimerExpiredById(0x00906500) )
  {
    v28 = kasAITeamPtrIfExists(252566178);
    kasfMsgSend((struct AITeam *)(int)v28, (char *)5801588);
  }
  if ( kasTimerExpiredById(-2122399739) )
  {
    v29 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v29, (char *)111517989);
  }
  if ( !kasfEntryValueGet(71552181) )
  {
    v46 = kasAITeamShipsPtrIfExists(96166939);
    v31 = kasGrowSelectionPtr(38608312);
    if ( kasfShipsSelectAllAttachedShips(v31, v46) )
    {
      kasfEntrySetValue(71552181, 1);
      v32 = kasAITeamPtrIfExists(176850956);
      kasfMsgSend((struct AITeam *)(int)v32, (char *)134620372);
    }
  }
}

void Begin_Mission17_NaggarokCruiseMissileAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x857BCE3));
    }
    singlePlayerSortShipsByDistanceToPoint(
        kasGrowSelectionPtr((char *)0x36980EB),
        kasShipsVectorPtr((char *)0x4E20));
    if (kasfShipsSelectAllAttachedShips(
            kasGrowSelectionPtr((char *)0x80624D4),
            kasAITeamShipsPtrIfExists((char *)0x5BB641B)))
    {
        kasfShipsRemove(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x80624D4));
    }
    kasfFormationDelta();
    kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x764E81C), kasGrowSelectionPtr((char *)0x36980EB));
    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(74024315, kasfRandom(20, 40));
}

void Watch_Mission17_NaggarokCruiseMissileAttackRun(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) ||
        kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_NaggarokCruiseMissileAttackRun,
            (sdword)Watch_Mission17_NaggarokCruiseMissileAttackRun);
    }
}

void Init_Mission17_MissionFlow(void)
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
    kasfEntryGetOrCreateSetValue(95916628, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(
        32713509,
        (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
        (sdword)Watch_Mission17_MissionFlow);
}

void Watch_Mission17_WaitForMissionCompleteIntelEvent(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded())
    {
        kasfMissionCompleted();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission17_WaitForIntelEventEnd(void)
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
        if (kasfEntryValueGet(95916628))
        {
            kasfMissionCompleted();
        }
        kasfSpeechEvent(
            374244,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission17_MissionFlow_Delay);
    }
}

void Watch_Mission17_MissionFlow_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasTimerSetById(144337492, 0);
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission17_MissionFlow);
    }
}

void Watch_Mission17_MissionFlow(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;
    MsgQueue *queue;
    GrowSelection *focusSelection;
    GrowSelection *doomSelection;
    AITeam *recipient;
    char *message;
    sdword nextMessage;
    sdword queueHead;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(218060896)))
    {
        kasfMissionFailed();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    if (!kasfEntryValueGet(332667))
    {
        queueHead = (team->msgQueue != NULL) ? team->msgQueue->head : -1;
        if (queueHead < 0)
        {
            return;
        }

        while (1)
        {
            message = (char *)aitQueuePopPointerByIndex(team, (udword)queueHead);
            if ((udword)message > 0x48CE874)
            {
                break;
            }
            if (message == (char *)76343412)
            {
                kasfSpeechEventQueueCurrentPlayer(268452534, 0);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission17_WaitForIntelEventEnd);
                return;
            }
            if ((udword)message > 0x28D3284)
            {
                switch ((udword)message)
                {
                    case 0x471A633u:
                        kasTimerSetById(144337492, 3);
                        kasTimerStartById(144337492);
                        kasfWideScreenIn(90);
                        kasfEntrySetValue(332667, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)43564914);
                        kasfSpeechEvent(
                            374244,
                            (sdword)MaintainTeamContext,
                            (sdword)Watch_Mission17_MissionFlow_Delay);
                        return;

                    case 0x471A634u:
                        if (kasfEntryValueGet(69879525))
                        {
                            break;
                        }
                        kasfEntryGetOrCreateSetValue(69879525, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(40027877), (char *)5801588);
                        kasTimerSetById(144337492, 3);
                        kasTimerStartById(144337492);
                        kasfEntrySetValue(332667, 1);
                        spEventWideScreenIn(90);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)119169985);
                        kasfSpeechEvent(
                            374244,
                            (sdword)MaintainTeamContext,
                            (sdword)Watch_Mission17_MissionFlow_Delay);
                        return;

                    case 0x471A635u:
                        kasfEntrySetValue(332667, 1);
                        kasfSpeechEventQueueCurrentPlayer(268444326, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)64006117);
                        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)17571950);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission17_WaitForIntelEventEnd);
                        return;

                    case 0x471A636u:
                        if (kasfEntryValueGet(44330530))
                        {
                            break;
                        }
                        kasfEntryGetOrCreateSetValue(44330530, 1);
                        kasTimerSetById(144337492, 3);
                        kasTimerStartById(144337492);
                        kasfWideScreenIn(90);
                        kasfEntrySetValue(332667, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)119170017);
                        kasfSpeechEvent(
                            374244,
                            (sdword)MaintainTeamContext,
                            (sdword)Watch_Mission17_MissionFlow_Delay);
                        return;

                    default:
                        aitQueuePointer(team, team, message);
                        break;
                }
            }
            else if (message == (char *)42807940)
            {
                kasfEntrySetValue(332667, 1);
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                spEventWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)148056178);
                kasfSpeechEvent(
                    374244,
                    (sdword)MaintainTeamContext,
                    (sdword)Watch_Mission17_MissionFlow_Delay);
                return;
            }
            else if ((udword)message > 0x489797)
            {
                if (message == (char *)29419476)
                {
                    kasfSpeechEventQueueCurrentPlayer(268452515, 0);
                    if (kasfShipsCount(kasAITeamShipsPtrIfExists(22630782)))
                    {
                        kasfEntrySetValue(332667, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)70893739);
                    }
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
                aitQueuePointer(team, team, message);
            }
            else if (message == (char *)4757399)
            {
                kasfEntrySetValue(266064926, 1);
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(127756738, 1);
                kasfLocationCard(1000, m17_saveInfoName[strCurLanguage]);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission17_WaitForIntelEventEnd);
                return;
            }
            else if (message != NULL)
            {
                if (message == (char *)300093)
                {
                    kasfSpeechEventQueueCurrentPlayer(268452529, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)4801506);
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
                aitQueuePointer(team, team, message);
            }

            queue = team->msgQueue;
            if (queue == NULL || queue->head == 0 || (--queueHead & 0x80000000) != 0)
            {
                return;
            }
        }

        if ((udword)message > 0x96316E4)
        {
            if ((udword)message > 0xEA9DDDE)
            {
                if (message == (char *)267431077)
                {
                    kasfSpeechEventQueueCurrentPlayer(268444335, 0);
                    kasfAllowPlayerToResearch("TECH_INERTIALESS_DRIVE");
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
            }
            else
            {
                switch ((udword)message)
                {
                    case 0xEA9DDDEu:
                        kasfSpeechEventQueueCurrentPlayer(268444341, 0);
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission17_WaitForIntelEventEnd);
                        return;

                    case 0xA28B655u:
                        kasfSpeechEventQueueCurrentPlayer(268452536, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)46028939);
                        kasfSpeechEvent(
                            181965865,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission17_WaitForMissionCompleteIntelEvent);
                        return;

                    case 0xE942A94u:
                        kasfEntryGetOrCreateSetValue(-2088490412, 1);
                        kasfSpeechEventQueueCurrentPlayer(268444325, 0);
                        nextMessage = 116177507;
                        recipient = kasAITeamPtrIfExists(160291907);
                        goto send_and_wait_for_intel;
                }
            }
        }
        else
        {
            if (message == (char *)157488868)
            {
                kasfEntryGetOrCreateSetValue(-2142422496, 1);
                kasfSpeechEventQueueCurrentPlayer(268444336, 0);
                nextMessage = 157488868;
                recipient = kasAITeamPtrIfExists(160291907);
                goto send_and_wait_for_intel;
            }
            if ((udword)message > 0x80624D4)
            {
                if (message == (char *)143614148)
                {
                    kasfSpeechEventQueueCurrentPlayer(268444322, 0);
                    nextMessage = 110641860;
                    recipient = kasAITeamPtrIfExists(160291907);
                    goto send_and_wait_for_intel;
                }
            }
            else
            {
                switch ((udword)message)
                {
                    case 0x80624D4u:
                        kasTimerSetById(144337492, 3);
                        kasTimerStartById(144337492);
                        kasfWideScreenIn(90);
                        kasfEntrySetValue(332667, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)6442450);
                        kasfSpeechEvent(
                            374244,
                            (sdword)MaintainTeamContext,
                            (sdword)Watch_Mission17_MissionFlow_Delay);
                        return;

                    case 0x6A5A125u:
                        kasfSpeechEventQueueCurrentPlayer(268444315, 0);
                        kasfAllowPlayerToResearch("TECH_REPULSE");
                        kasfSpeechEvent(
                            244212684,
                            (sdword)Init_WaitForIntelEventEnd,
                            (sdword)Watch_Mission17_WaitForIntelEventEnd);
                        return;

                    case 0x71A6311u:
                        kasfEntrySetValue(332667, 1);
                        kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                        kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                        kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                        kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(108434753));
                        kasfCameraSetAngleDeg(46);
                        kasfCameraSetDeclinationDeg(1);
                        kasfCameraSetDistance(1588);
                        kasfSpeechEventQueueCurrentPlayer(270533264, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)119169810);
                        nextMessage = 221012411;
                        recipient = kasAITeamPtrIfExists(160291907);
                        goto send_and_wait_for_intel;
                }
            }
        }
        aitQueuePointer(team, team, message);
    }

    queueHead = (team->msgQueue != NULL) ? team->msgQueue->head : -1;
    if (queueHead < 0)
    {
        return;
    }

    while (1)
    {
        message = (char *)aitQueuePopPointerByIndex(team, (udword)queueHead);
        if ((udword)message > 0x71A6331)
        {
            if ((udword)message <= 0x71A63D3)
            {
                if (message == (char *)119170003)
                {
                    kasTimerSetById(144337492, 7);
                    kasTimerStartById(144337492);
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    kasfCameraSetAngleDeg(201);
                    kasfCameraSetDeclinationDeg(-70);
                    kasfCameraSetDistance(2700);
                    kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(111385145));
                    kasfSpeechEventQueueCurrentPlayer(268567208, 0);
                    nextMessage = 119170004;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }

                switch ((udword)message)
                {
                    case 0x71A6332u:
                        kasTimerSetById(144337492, 3);
                        kasfEntrySetValue(127756738, 1);
                        kasfEntrySetValue(266064926, 1);
                        kasfEntrySetValue(332667, 0);
                        kasfEntrySetValue(172907219, 1);
                        kasfSpeechEventQueueCurrentPlayer(268444314, 0);
                        kasTimerCreateSetStartById(-2122399739, 60);
                        nextMessage = 159289139;
                        recipient = kasAITeamPtrIfExists(160291907);
                        goto send_and_wait_for_intel;

                    case 0x71A63C1u:
                        kasfEntryGetOrCreateSetValue(-2103308245, 1);
                        kasfUniversePause();
                        kasfOtherKASPause();
                        kasfEntrySetValue(211831571, 1);
                        kasfEntrySetValue(109086661, 1);
                        if (!kasfSensorsIsOpen(0))
                        {
                            kasfOpenSensors(150000);
                        }
                        soundStopDSound("PointerTaiidan", kasGrowSelectionPtr(140367886));
                        kasfSensorsSetLookatPoint(kasShipsVectorPtrIfExists(140367886));
                        kasfSpeechEventQueueCurrentPlayer(268444316, 0);
                        nextMessage = 119169986;
                        recipient = kasAITeamPtrIfExists(176850956);
                        goto send_and_wait_for_intel;

                    case 0x71A63C2u:
                        kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                        kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                        kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                        kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(185123377));
                        kasfCameraSetAngleDeg(325);
                        kasfCameraSetDeclinationDeg(-14);
                        kasfCameraSetDistance(1063);
                        kasfSpeechEventQueueCurrentPlayer(270533277, 0);
                        nextMessage = 119169987;
                        recipient = kasAITeamPtrIfExists(176850956);
                        goto send_and_wait_for_intel;

                    case 0x71A63C3u:
                        kasfSpeechEventQueueCurrentPlayer(268440222, 0);
                        nextMessage = 119169988;
                        recipient = kasAITeamPtrIfExists(176850956);
                        goto send_and_wait_for_intel;

                    case 0x71A63C4u:
                        kasfSpeechEventQueueCurrentPlayer(270533279, 0);
                        nextMessage = 119169989;
                        recipient = kasAITeamPtrIfExists(176850956);
                        goto send_and_wait_for_intel;

                    case 0x71A63C5u:
                        kasTimerSetById(144337492, 3);
                        kasfEntrySetValue(127756738, 1);
                        kasfEntrySetValue(52577235, 1);
                        kasfEntrySetValue(266064926, 1);
                        kasfEntrySetValue(172907219, 1);
                        kasfEntrySetValue(332667, 0);
                        kasfOtherKASPause();
                        kasfUniversePause();
                        kasfEntrySetValue(211831571, 1);
                        kasfEntrySetValue(109086661, 1);
                        if (!kasfSensorsIsOpen(0))
                        {
                            kasfOpenSensors(150000);
                        }
                        kasfSensorsSetLookatPoint(kasVectorPtrIfExists(128206740));
                        soundStopDSound("PincerLeft", kasGrowSelectionPtr(217111973));
                        kasfSpeechEventQueueCurrentPlayer(268444320, 0);
                        kasfMsgSend(kasAITeamPtrIfExists(217111973), (char *)5801588);
                        nextMessage = 0x56983E;
                        recipient = kasAITeamPtrIfExists(160291907);
                        goto send_and_wait_for_intel;

                    case 0x71A63D2u:
                        kasfOtherKASPause();
                        kasfUniversePause();
                        kasfEntrySetValue(211831571, 1);
                        kasfEntrySetValue(109086661, 1);
                        if (!kasfSensorsIsOpen(0))
                        {
                            kasfOpenSensors(150000);
                        }
                        kasfSpeechEventQueueCurrentPlayer(268444327, 0);
                        kasfSensorsSetLookatPoint(kasTeamsVectorPtr(111385145));
                        soundStopDSound("Bentusi", kasAITeamShipsPtrIfExists(111385145));
                        nextMessage = 119170003;
                        recipient = kasAITeamPtrIfExists(176850956);
                        goto send_and_wait_for_intel;
                }
            }
            else if ((udword)message > 0x71A63E3)
            {
                if (message == (char *)119170020)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(172907219, 1);
                    kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(96166939));
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEventQueueCurrentPlayer(268452525, 0);
                    kasfEntryGetOrCreateSetValue(-1989827070, 1);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
                if (message == (char *)126523733)
                {
                    kasTimerSetById(144337492, 10);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntrySetValue(266064926, 1);
                    kasfLocationCard(6000, m17_saveInfoName[strCurLanguage]);
                    kasfEntrySetValue(172907219, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(108434753), (char *)126523733);
                    kasfMsgSend(kasAITeamPtrIfExists(185123649), (char *)126523733);
                    nextMessage = 74556978;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)148056178)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntrySetValue(52577235, 1);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(332667, 0);
                    kasfOtherKASPause();
                    kasfUniversePause();
                    kasfEntrySetValue(211831571, 1);
                    kasfEntrySetValue(109086661, 1);
                    if (!kasfSensorsIsOpen(0))
                    {
                        kasfOpenSensors(150000);
                    }
                    kasfSensorsSetLookatPoint(kasTeamsVectorPtr(132678291));
                    kasfSpeechEventQueueCurrentPlayer(268444321, 0);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
            }
            else
            {
                if (message == (char *)119170019)
                {
                    kasfSpeechEventQueueCurrentPlayer(270533292, 0);
                    nextMessage = 119170020;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)119170004)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(332667, 0);
                    kasfEntrySetValue(172907219, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(111385145), (char *)175938357);
                    kasfSpeechEventQueueCurrentPlayer(268440233, 0);
                    kasfEnableShip("dAcolyte", "RACE_SECT", 1);
                    kasfMsgSend(kasAITeamPtrIfExists(96166939), (char *)5801588);
                    kasTimerCreateSetStartById(-1989827070, 90);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
                if (message == (char *)119170017)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    focusSelection = kasAITeamShipsPtrIfExists(185123377);
                    if (!kasfShipsCount(focusSelection))
                    {
                        focusSelection = kasAITeamShipsPtrIfExists(185123378);
                        if (!kasfShipsCount(focusSelection))
                        {
                            focusSelection = kasAITeamShipsPtrIfExists(108429889);
                            if (!kasfShipsCount(focusSelection))
                            {
                                focusSelection = kasAITeamShipsPtrIfExists(108429890);
                                if (!kasfShipsCount(focusSelection))
                                {
                                    focusSelection = kasGrowSelectionPtr(140367886);
                                }
                            }
                        }
                    }
                    kasfTutCameraFocusIgnoreZoom(focusSelection);
                    kasfCameraSetAngleDeg(325);
                    kasfCameraSetDeclinationDeg(-14);
                    kasfCameraSetDistance(1063);
                    kasfSpeechEventQueueCurrentPlayer(270533290, 0);
                    nextMessage = 119170018;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)119170018)
                {
                    kasfEntrySetValue(172907219, 1);
                    kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(96166939));
                    kasfSpeechEventQueueCurrentPlayer(268452523, 0);
                    nextMessage = 119170019;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
            }
        }
        else if (message == (char *)119169841)
        {
            kasfEntryGetOrCreateSetValue(-2092043915, 1);
            kasfSpeechEventQueueCurrentPlayer(268440217, 0);
            nextMessage = 119169842;
            recipient = kasAITeamPtrIfExists(176850956);
            goto send_and_wait_for_intel;
        }
        else if ((udword)message > 0x439C0AB)
        {
            if ((udword)message > 0x71A6314)
            {
                if (message == (char *)119169813)
                {
                    kasfSpeechEventQueueCurrentPlayer(270533268, 0);
                    nextMessage = 126523733;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)119169826)
                {
                    kasfSpeechEventQueueCurrentPlayer(268440214, 0);
                    nextMessage = 119169827;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)119169827)
                {
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEventQueueCurrentPlayer(268452503, 0);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission17_WaitForIntelEventEnd);
                    return;
                }
            }
            else
            {
                if (message == (char *)119169812)
                {
                    kasfSpeechEventQueueCurrentPlayer(270533267, 0);
                    nextMessage = 119169813;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)74556978)
                {
                    kasfSpeechEventQueueCurrentPlayer(268452501, 0);
                    nextMessage = 119169826;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)119169810)
                {
                    kasfEntrySetValue(172907219, 1);
                    kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(218060896));
                    kasfCameraSetAngleDeg(205);
                    kasfCameraSetDeclinationDeg(18);
                    kasfCameraSetDistance(3100);
                    kasfSpeechEventQueueCurrentPlayer(268440209, 0);
                    nextMessage = 119169811;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
                if (message == (char *)119169811)
                {
                    kasfMsgSend(kasAITeamPtrIfExists(133880113), (char *)5801588);
                    kasfShipsMakeCrazy(kasAITeamShipsPtrIfExists(108434753), 0);
                    kasfShipsMakeCrazy(kasAITeamShipsPtrIfExists(185123649), 0);
                    kasfShipsSetDelayedDeath(kasAITeamShipsPtrIfExists(108434753), 1);
                    kasfShipsSetDelayedDeath(kasAITeamShipsPtrIfExists(185123649), 1);
                    kasfSpeechEventQueueCurrentPlayer(270533266, 0);
                    nextMessage = 119169812;
                    recipient = kasAITeamPtrIfExists(176850956);
                    goto send_and_wait_for_intel;
                }
            }
        }
        else if (message == (char *)70893739)
        {
            kasfEntrySetValue(332667, 0);
            kasfSpeechEventQueueCurrentPlayer(268444324, 0);
            kasfSpeechEvent(
                244212684,
                (sdword)Init_WaitForIntelEventEnd,
                (sdword)Watch_Mission17_WaitForIntelEventEnd);
            return;
        }
        else if ((udword)message > 0x624DD2)
        {
            if (message == (char *)43564914)
            {
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(109086661, 1);
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(150000);
                }
                kasfSensorsSetLookatPoint(kasTeamsVectorPtr(22630782));
                soundStopDSound("PointerNomadMoon", kasAITeamShipsPtrIfExists(22630782));
                kasfSpeechEventQueueCurrentPlayer(268444312, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)136940542);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)43564915);
                kasfMsgSend(kasAITeamPtrIfExists(22630782), (char *)306789);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission17_WaitForIntelEventEnd);
                return;
            }
            if (message == (char *)43564915)
            {
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(22630782));
                kasfCameraSetAngleDeg(49);
                kasfCameraSetDeclinationDeg(-6);
                kasfCameraSetDistance(10456);
                kasfLocationCard(4000, m17_saveInfoName[strCurLanguage]);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)119169841);
                kasfMsgSend(kasAITeamPtrIfExists(22630782), (char *)306789);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission17_WaitForIntelEventEnd);
                return;
            }
            if (message == (char *)64006117)
            {
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)119170002);
                kasfMsgSend(kasAITeamPtrIfExists(111385145), (char *)5801588);
                kasfSpeechEvent(
                    374244,
                    (sdword)MaintainTeamContext,
                    (sdword)Watch_Mission17_MissionFlow_Delay);
                return;
            }
        }
        else if (message == (char *)6442450)
        {
            kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
            kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
            kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
            kasTimerSetById(144337492, 3);
            kasfEntrySetValue(127756738, 1);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(332667, 0);
            kasfEntrySetValue(172907219, 1);
            kasfSpeechEventQueueCurrentPlayer(268468506, 0);
            kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(38608312));
            kasfCameraSetDistance(2000);
            kasfSpeechEvent(
                244212684,
                (sdword)Init_WaitForIntelEventEnd,
                (sdword)Watch_Mission17_WaitForIntelEventEnd);
            return;
        }
        else if ((udword)message <= 4801507)
        {
            if (message == (char *)4801507)
            {
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(127756738, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(52577235, 1);
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(109086661, 1);
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(150000);
                }
                kasfSensorsSetLookatPoint(kasShipsVectorPtrIfExists(218060896));
                doomSelection = kasGrowSelectionPtr(124053532);
                kasfShipsSelectType(doomSelection, kasGrowSelectionPtr(300093), "bHeavyCruiser");
                kasfPingAddShipsIndexed(doomSelection, "Doom");
                kasfShipsSelectType(doomSelection, kasGrowSelectionPtr(4801506), "bHeavyCruiser");
                kasfPingAddShipsIndexed(doomSelection, "Doom2");
                kasfShipsSelectType(doomSelection, kasGrowSelectionPtr(4801507), "bHeavyCruiser");
                kasfPingAddShipsIndexed(doomSelection, "Doom3");
                kasfShipsSelectType(doomSelection, kasGrowSelectionPtr(4801508), "bHeavyCruiser");
                kasfPingAddShipsIndexed(doomSelection, "Doom4");
                kasfSpeechEventQueueCurrentPlayer(268440243, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)4801508);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission17_WaitForIntelEventEnd);
                return;
            }
            if (message == (char *)4801506)
            {
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)4801507);
                kasfSpeechEvent(
                    374244,
                    (sdword)MaintainTeamContext,
                    (sdword)Watch_Mission17_MissionFlow_Delay);
                return;
            }
        }
        else if (message == (char *)4801508)
        {
            kasfEntrySetValue(332667, 0);
            kasfSpeechEventQueueCurrentPlayer(268444340, 0);
            kasfSpeechEvent(
                244212684,
                (sdword)Init_WaitForIntelEventEnd,
                (sdword)Watch_Mission17_WaitForIntelEventEnd);
            return;
        }

        aitQueuePointer(team, team, message);
        queue = team->msgQueue;
        if (queue == NULL || queue->head == 0 || (--queueHead & 0x80000000) != 0)
        {
            return;
        }
    }

send_and_wait_for_intel:
    kasfMsgSend(recipient, (char *)nextMessage);
    kasfSpeechEvent(
        244212684,
        (sdword)Init_WaitForIntelEventEnd,
        (sdword)Watch_Mission17_WaitForIntelEventEnd);
}





void Init_Mission17_BeastHeavyCruiserReinforcementTeam(void)
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
        kasTimerCreateById(142843908);
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
            kasfSpeechEvent(
                74024315,
                (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
                (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementTeam);
        }
    }
}

void Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate(void)
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
            if (kasfEntryValueGet(-1989827070) &&
                kasTimerExpiredDestroyById(142843908) &&
                kasfShipsSelectNearby(
                    kasGrowSelectionPtr((char *)0x36980EB),
                    kasGrowSelectionPtr((char *)0x85DD80E),
                    kasShipsVectorPtr((char *)0x2710),
                    10000))
            {
                kasfSpeechEvent(
                    222380532,
                    (sdword)Begin_Mission17_BeastHeavyCruiserReinforcementTeam_AttackRun,
                    (sdword)Watch_Mission17_BeastHeavyCruiserAttackRun);
            }
            else if (kasfShipsAnyBigGunReady(&team->shipList) &&
                     kasfShipsSelectNearby(
                         kasGrowSelectionPtr((char *)0x36980EB),
                         kasGrowSelectionPtr((char *)0x4A2A29),
                         kasShipsVectorPtr((char *)0x4E20),
                         20000))
            {
                kasfSpeechEvent(
                    157845977,
                    (sdword)Begin_Mission17_BeastHeavyCruiserSpecialAttack,
                    (sdword)Watch_Mission17_BeastHeavyCruiserSpecialAttack);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)188751475);
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementTeam);
        }
    }
}

void Begin_Mission17_BeastHeavyCruiserSpecialAttack(void)
{
    GrowSelection *attackTargets;
    GrowSelection *underAttackSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (!kasfShipsSelectNearby(
            attackTargets,
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
        return;
    }

    kasfEntrySetValue(332667, 1);
    underAttackSelection = kasGrowSelectionPtr((char *)0x857BCE3);
    if (kasfUnderAttack(underAttackSelection) &&
        kasfShipsSelectMembersOfSelection(underAttackSelection, underAttackSelection, attackTargets))
    {
        kasfShipsAdd(attackTargets, underAttackSelection);
    }

    if (!kasfEntryValueGet((sdword)0x801620C4))
    {
        kasfShipsRemove(attackTargets, kasAITeamShipsPtrIfExists(0x6769541));
        kasfShipsRemove(attackTargets, kasAITeamShipsPtrIfExists(0xB08C341));
        if (!kasfShipsCount(attackTargets))
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
                (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
            return;
        }
    }

    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x764E81C), attackTargets))
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x764E81C));
    }
    else
    {
        kasfTeamSpecial(attackTargets);
    }
}

void Watch_Mission17_BeastHeavyCruiserSpecialAttack(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementTeam);
    }
    else if (!kasfShipsAnyBigGunReady(&team->shipList))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            157845977,
            (sdword)Begin_Mission17_BeastHeavyCruiserSpecialAttack,
            (sdword)Watch_Mission17_BeastHeavyCruiserSpecialAttack);
    }
}

void Begin_Mission17_BeastHeavyCruiserReinforcementTeam_AttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (!kasfShipsSelectNearby(
            attackTargets,
            kasGrowSelectionPtr((char *)0x85DD80E),
            kasShipsVectorPtr((char *)0x2710),
            10000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
        return;
    }

    kasfEntrySetValue(332667, 1);
    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    kasfShipsSetResourcingFlag((sdword)&team->shipList, 1);
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x764E81C), attackTargets))
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x764E81C));
    }
    else
    {
        kasfTeamSpecial(attackTargets);
    }
    kasfShipsSetResourcingFlag((sdword)&team->shipList, 0);
}

void Watch_Mission17_BeastHeavyCruiserAttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasTimerSetById(142843908, 20);
        kasTimerStartById(142843908);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementTeam);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasTimerSetById(142843908, 20);
        kasTimerStartById(142843908);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
    }
}

void Watch_Mission17_BeastHeavyCruiserReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedHeavyCruisers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedHeavyCruisers = kasGrowSelectionPtr((char *)0x58A20);
    if (kasfShipsSelectType(returnedHeavyCruisers, kasAITeamShipsPtrIfExists(0x2A98C74), "bHeavyCruiser") < 1)
    {
        if (!kasfShipsCount(kasGrowSelectionPtr((char *)0xE05243)))
        {
            kasfEntrySetValue(-1920527822, 0);
            kasfEntrySetValue(-2110057163, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        else if (kasfEntryValueGet(-1920527822))
        {
            kasfEntrySetValue(-1920527822, 0);
            kasfEntrySetValue(-2110057163, 1);
            kasfSpeechEvent(
                165710423,
                (sdword)ResetReinforcementBuildCount,
                (sdword)Init_Mission17_BeastHeavyCruiserReinforcementBuildTimer);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)185462083);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(returnedHeavyCruisers, kasAITeamShipsPtrIfExists(0x2A98C74), "bHeavyCruiser"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(0x2A98C74), returnedHeavyCruisers);
        kasfShipsStop(returnedHeavyCruisers);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedHeavyCruisers);
        kasfShipsSetSlaveableFlag(returnedHeavyCruisers, 0);
        kasfShipsApplyPlayerUpgrades(returnedHeavyCruisers);
        kasfReinforceTeamWithShips(team, returnedHeavyCruisers);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
        (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
}

void Init_Mission17_BeastHeavyCruiserReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(154639956) < 1 &&
            kasfRUsGet(1) >= kasfShipTypeBuildCost("bHeavyCruiser") / 2)
        {
            kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bHeavyCruiser") / 2);
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) > 1 &&
            kasfRUsGet(1) < kasfShipTypeBuildCost("bHeavyCruiser") / 2)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastHeavyCruiserReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementBuildTimer);
            return;
        }
    }
    else if (buildStyle == 2)
    {
        if (kasfEntryValueGet(154639956) < 1)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 1)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastHeavyCruiserReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementBuildTimer);
        }
        return;
    }
    else if (buildStyle != 0)
    {
        return;
    }
    else if (kasfEntryValueGet(154639956) < 1 &&
             kasfRUsGet(1) >= kasfShipTypeBuildCost("bHeavyCruiser"))
    {
        buildCost = kasfShipTypeBuildCost("bHeavyCruiser");
        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
        kasfEntryIncrement(154639956);
    }

    if (kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_BeastHeavyCruiserReinforcementBuildTimer,
            (sdword)Watch_Mission17_BeastHeavyCruiserReinforcementBuildTimer);
    }
}

void Watch_Mission17_BeastHeavyCruiserReinforcementBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0xE05243)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        kasfShipsCreate(
            kasGrowSelectionPtr((char *)0x9379E54),
            "bHeavyCruiser",
            kasfEntryValueGet(154639956),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)0x9379E54), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)0x9379E54));
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsSetSlaveableFlag(&team->shipList, 0);
        kasfSpeechEvent(
            175938357,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Init_Mission17_BeastHeavyCruiserAttackMothership);
    }
}

void Init_Mission17_BeastHeavyCruiserAttackMothership(void)
{
    AITeam *team;
    GrowSelection *stagingSelection;
    GrowSelection *targetSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        stagingSelection = kasGrowSelectionPtr((char *)0x716C77E);
        targetSelection = kasGrowSelectionPtr((char *)21541);
        targetIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr((char *)0xE05243)) - 1);
        if (kasfShipsSelectIndex(targetSelection, kasGrowSelectionPtr((char *)0xE05243), targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            kasTimerCreateSetStartById(23446965, 8);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, targetSelection);
            kasfShipsSetMilitaryParade(targetSelection, stagingSelection);
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
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
    }
}

void Watch_Mission17_BeastHeavyCruiserCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastMultiBeamFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatLoop);
    }
}

void Init_Mission17_BeastCarrierReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(124218850)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-2078373390, 0);
        kasfEntryGetOrCreateSetValue(-1980584283, 1);
        kasTimerCreateById(142843908);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)103051523);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfShipsSetSlaveableFlag(&team->shipList, 0);
            kasfEntrySetValue(332667, 0);
            kasTimerSetById(142843908, 20);
            kasTimerStartById(142843908);
            kasfSpeechEvent(
                337061,
                (sdword)Begin_Mission17_BeastCarrierCombatLoop,
                (sdword)Watch_Mission17_BeastCarrierCombatLoop);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_BeastCarrierReinforcementTeam);
        }
    }
}

void Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(124218850)) &&
        kasfEntryValueGet(-1980584283) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1980584283, 0);
        kasfEntrySetValue(-2078373390, 1);
    }
    if (!kasfEntryValueGet(332667))
    {
        if (!kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)50020867);
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_BeastCarrierReinforcementTeam);
            return;
        }

        if (kasfMsgReceived((char *)362601))
        {
            kasfEntryGetOrCreateSetValue(362601, 1);
            kasfEntryGetOrCreateSetValue(88, kasfVectorRoundX(kasThisTeamsVectorPtr()));
            kasfEntryGetOrCreateSetValue(89, kasfVectorRoundY(kasThisTeamsVectorPtr()));
            kasfEntryGetOrCreateSetValue(90, kasfVectorRoundZ(kasThisTeamsVectorPtr()));
        }
        if (kasfEntryValueGet(-1989827070) &&
            kasTimerExpiredDestroyById(142843908) &&
            kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x36980EB),
                kasGrowSelectionPtr((char *)0x85DD80E),
                kasShipsVectorPtr((char *)0x2710),
                10000))
        {
            kasfSpeechEvent(
                222380532,
                (sdword)Begin_Mission17_BeastCarrierReinforcementTeam_AttackRun,
                (sdword)Watch_Mission17_BeastCarrierReinforcementTeam_AttackRun);
            return;
        }
        if (!kasfEntryValueGet(71552164) &&
            !kasfShipsCount(kasAITeamShipsPtrIfExists(22630782)))
        {
            kasfEntryGetOrCreateSetValue(71552164, 1);
            kasfReinforceTeamWithShips(kasAITeamPtrIfExists(44665972), &team->shipList);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
            return;
        }
        if (kasfShipsAnyBigGunReady(&team->shipList) &&
            kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x36980EB),
                kasGrowSelectionPtr((char *)0x4A2A29),
                kasShipsVectorPtr((char *)0x4E20),
                20000))
        {
            kasfSpeechEvent(
                157845977,
                (sdword)Begin_Mission17_BeastCarrierSpecialAttack,
                (sdword)Watch_Mission17_BeastCarrierSpecialAttack);
        }
    }
}

void Begin_Mission17_BeastCarrierSpecialAttack(void)
{
    GrowSelection *attackTargets;
    GrowSelection *underAttackSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (!kasfShipsSelectNearby(
            attackTargets,
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastCarrierAttackRun,
            (sdword)Watch_Mission17_BeastCarrierAttackRun);
        return;
    }

    kasfEntrySetValue(332667, 1);
    underAttackSelection = kasGrowSelectionPtr((char *)0x857BCE3);
    if (kasfUnderAttack(underAttackSelection) &&
        kasfShipsSelectMembersOfSelection(underAttackSelection, underAttackSelection, attackTargets))
    {
        kasfShipsAdd(attackTargets, underAttackSelection);
    }
    if (!kasfEntryValueGet((sdword)0x801620C4))
    {
        kasfShipsRemove(attackTargets, kasAITeamShipsPtrIfExists(0x6769541));
        kasfShipsRemove(attackTargets, kasAITeamShipsPtrIfExists(0xB08C341));
        if (!kasfShipsCount(attackTargets))
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Begin_Mission17_BeastCarrierAttackRun,
                (sdword)Watch_Mission17_BeastCarrierAttackRun);
            return;
        }
    }

    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x764E81C), attackTargets))
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x764E81C));
    }
    else
    {
        kasfTeamSpecial(attackTargets);
    }
}

void Watch_Mission17_BeastCarrierSpecialAttack(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastCarrierReinforcementTeam);
    }
    else if (!kasfShipsAnyBigGunReady(&team->shipList) ||
             kasfShipsAnyIdle(&team->shipList))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_BeastCarrierCombatLoop,
            (sdword)Watch_Mission17_BeastCarrierCombatLoop);
    }
}

void Begin_Mission17_BeastCarrierReinforcementTeam_AttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (!kasfShipsSelectNearby(
            attackTargets,
            kasGrowSelectionPtr((char *)0x85DD80E),
            kasShipsVectorPtr((char *)0x2710),
            10000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastCarrierAttackRun,
            (sdword)Watch_Mission17_BeastCarrierAttackRun);
        return;
    }

    kasfEntrySetValue(332667, 1);
    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    kasfShipsSetResourcingFlag((sdword)&team->shipList, 1);
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x764E81C), attackTargets))
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x764E81C));
    }
    else
    {
        kasfTeamSpecial(attackTargets);
    }
    kasfShipsSetResourcingFlag((sdword)&team->shipList, 0);
}

void Watch_Mission17_BeastCarrierReinforcementTeam_AttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasTimerSetById(142843908, 20);
        kasTimerStartById(142843908);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastCarrierReinforcementTeam);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfEntrySetValue(332667, 0);
        kasTimerSetById(142843908, 20);
        kasTimerStartById(142843908);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_BeastCarrierCombatLoop,
            (sdword)Watch_Mission17_BeastCarrierCombatLoop);
    }
}

void Watch_Mission17_BeastCarrierReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedCarriers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedCarriers = kasGrowSelectionPtr((char *)0x58A20);
    if (kasfShipsSelectType(returnedCarriers, kasAITeamShipsPtrIfExists(0x2A98C74), "bCarrier") < 1)
    {
        if (!kasfShipsCount(kasGrowSelectionPtr((char *)0xE05243)))
        {
            kasfEntrySetValue(-2078373390, 0);
            kasfEntrySetValue(-1980584283, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        else if (kasfEntryValueGet(-2078373390))
        {
            kasfEntrySetValue(-2078373390, 0);
            kasfEntrySetValue(-1980584283, 1);
            kasfSpeechEvent(
                165710423,
                (sdword)ResetReinforcementBuildCount,
                (sdword)Init_Mission17_BeastCarrierReinforcementBuildTimer);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)103051523);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(returnedCarriers, kasAITeamShipsPtrIfExists(0x2A98C74), "bCarrier"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(0x2A98C74), returnedCarriers);
        kasfShipsStop(returnedCarriers);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedCarriers);
        kasfShipsSetSlaveableFlag(returnedCarriers, 0);
        kasfShipsApplyPlayerUpgrades(returnedCarriers);
        kasfReinforceTeamWithShips(team, returnedCarriers);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        337061,
        (sdword)Begin_Mission17_BeastCarrierCombatLoop,
        (sdword)Watch_Mission17_BeastCarrierCombatLoop);
}

void Init_Mission17_BeastCarrierReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(154639956) < 1 &&
            kasfRUsGet(1) >= kasfShipTypeBuildCost("bCarrier") / 2)
        {
            kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bCarrier") / 2);
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) > 1 &&
            kasfRUsGet(1) < kasfShipTypeBuildCost("bCarrier") / 2)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastCarrierReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastCarrierReinforcementBuildTimer);
            return;
        }
    }
    else if (buildStyle == 2)
    {
        if (kasfEntryValueGet(154639956) < 1)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 1)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastCarrierReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastCarrierReinforcementBuildTimer);
        }
        return;
    }
    else if (buildStyle != 0)
    {
        return;
    }
    else if (kasfEntryValueGet(154639956) < 1 &&
             kasfRUsGet(1) >= kasfShipTypeBuildCost("bCarrier"))
    {
        buildCost = kasfShipTypeBuildCost("bCarrier");
        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
        kasfEntryIncrement(154639956);
    }

    if (kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_BeastCarrierReinforcementBuildTimer,
            (sdword)Watch_Mission17_BeastCarrierReinforcementBuildTimer);
    }
}

void Watch_Mission17_BeastCarrierReinforcementBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0xE05243)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        kasfShipsCreate(kasGrowSelectionPtr((char *)0x9379E54), "bCarrier", kasfEntryValueGet(154639956), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)0x9379E54));
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsSetSlaveableFlag(&team->shipList, 0);
        kasfSpeechEvent(
            175938357,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Init_Mission17_BeastCarrierAttackMothership);
    }
}

void Init_Mission17_BeastCarrierAttackMothership(void)
{
    AITeam *team;
    GrowSelection *stagingSelection;
    GrowSelection *targetSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        stagingSelection = kasGrowSelectionPtr((char *)0x716C77E);
        targetSelection = kasGrowSelectionPtr((char *)21541);
        targetIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr((char *)0xE05243)) - 1);
        if (kasfShipsSelectIndex(targetSelection, kasGrowSelectionPtr((char *)0xE05243), targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, targetSelection);
            kasfShipsSetMilitaryParade(targetSelection, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)103051523);
        kasfEntrySetValue(332667, 0);
        kasTimerSetById(142843908, 20);
        kasTimerStartById(142843908);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_BeastCarrierCombatLoop,
            (sdword)Watch_Mission17_BeastCarrierCombatLoop);
    }
}

void Begin_Mission17_BeastCarrierCombatLoop(void)
{
    GrowSelection *candidateTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(362601))
    {
        kasfLabelledVectorSet(
            124218850,
            kasfEntryValueGet(88),
            kasfEntryValueGet(89),
            kasfEntryValueGet(90));
        kasfMoveTo(kasVectorPtrIfExists(124218850));
    }
    else if (kasfNearby(kasTeamsVectorPtr(22630782), 50000))
    {
        kasfLabelledVectorSet(191768670, 0, 0, 0);
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(191768670), kasTeamsVectorPtr(22682558), 80000);
        kasfMoveTo(kasVectorPtrIfExists(191768670));
    }
    else if (!kasfShipsSelectNearby(
                 kasGrowSelectionPtr((char *)0x36980EB),
                 kasGrowSelectionPtr((char *)0x4A2A29),
                 kasShipsVectorPtr((char *)0xEA60),
                 60000))
    {
        candidateTargets = kasGrowSelectionPtr((char *)0x524AA8F);
        if (!kasfFindShipsTargetingSelection(candidateTargets, kasGrowSelectionPtr((char *)0x469684)))
        {
            kasfShipsAdd(candidateTargets, kasAITeamShipsPtrIfExists(0x5BB641B));
        }
        kasfShipsSortByDistanceToPoint(candidateTargets, kasThisTeamsVectorPtr());
        if (kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), candidateTargets, 0))
        {
            kasfLabelledVectorSet(42190639, 0, 0, 0);
            kasfVectorRandomizeInRange(
                kasVectorPtrIfExists(42190639),
                kasShipsVectorPtrIfExists(21541),
                30000);
            kasfMoveTo(kasVectorPtrIfExists(42190639));
        }
    }
    kasTimerCreateSetStartById(337061, kasfRandom(120, 160));
}

void Watch_Mission17_BeastCarrierCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *underAttackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(362601) &&
        kasfShipsAnyIdle(&team->shipList) &&
        kasfNearby(kasTeamsVectorPtr(22630782), 50000))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_BeastCarrierCombatLoop,
            (sdword)Watch_Mission17_BeastCarrierCombatLoop);
        return;
    }

    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    underAttackTargets = kasGrowSelectionPtr((char *)0x857BCE3);
    if (kasfShipsSelectNearby(
            attackTargets,
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4E20),
            20000) &&
        kasfUnderAttack(underAttackTargets) &&
        kasfShipsSelectMembersOfSelection(underAttackTargets, underAttackTargets, attackTargets))
    {
        kasfLabelledVectorSet(
            124218850,
            kasfEntryValueGet(88),
            kasfEntryValueGet(89),
            kasfEntryValueGet(90));
        if (kasfEntryValueGet(362601) &&
            kasfFindDistance(kasVectorPtrIfExists(128425636), kasThisTeamsVectorPtr()) < 16000)
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Begin_Mission17_BeastCarrierAttackRun,
                (sdword)Watch_Mission17_BeastCarrierAttackRun);
            return;
        }
    }
    if (kasTimerExpiredById(337061) && kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_BeastCarrierCombatLoop,
            (sdword)Watch_Mission17_BeastCarrierCombatLoop);
    }
}

void Begin_Mission17_BeastCarrierAttackRun(void)
{
    GrowSelection *attackTargets;
    GrowSelection *underAttackSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (!kasfShipsSelectNearby(
            attackTargets,
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x4A2A29));
    }
    underAttackSelection = kasGrowSelectionPtr((char *)0x857BCE3);
    if (kasfUnderAttack(underAttackSelection) &&
        kasfShipsSelectMembersOfSelection(underAttackSelection, underAttackSelection, attackTargets))
    {
        kasfShipsAdd(attackTargets, underAttackSelection);
    }
    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    tutDeallocateRootRegion(attackTargets);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_BeastCarrierAttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfLabelledVectorSet(
        124218850,
        kasfEntryValueGet(88),
        kasfEntryValueGet(89),
        kasfEntryValueGet(90));
    if (kasfShipsAnyIdle(&team->shipList) ||
        (kasfEntryValueGet(362601) &&
         kasfFindDistance(kasVectorPtrIfExists(128425636), kasThisTeamsVectorPtr()) < 16000))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_BeastCarrierCombatLoop,
            (sdword)Watch_Mission17_BeastCarrierCombatLoop);
    }
    else if (kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastCarrierAttackRun,
            (sdword)Watch_Mission17_BeastCarrierAttackRun);
    }
}










void Init_Mission17_BeastCorvetteReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(75148549)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-2082149163, 0);
       kasfEntryGetOrCreateSetValue(-1931839291, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)41951459);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasfSpeechEvent(
               74024315,
               (sdword)Begin_Mission17_BeastCorvetteCombatLoop,
               (sdword)Watch_Mission17_BeastCorvetteCombatLoop);
       }
       else
       {
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastCorvetteReinforcementTeam);
       }
   }
}

void Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(75148549)) &&
       kasfEntryValueGet(-1931839291) &&
       kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1931839291, 0);
       kasfEntrySetValue(-2082149163, 1);
   }
   if (!kasfEntryValueGet(332667))
   {
       if (kasfTeamCount())
       {
           if (kasfShipsCount(kasGrowSelectionPtr(14701123)) && kasfTeamHealthAverage() < 50)
           {
               kasfSpeechEvent(
                   299899,
                   (sdword)Begin_Mission17_BeastReconDockAndReinforce,
                   (sdword)Watch_Mission17_BeastCorvetteDockAndReinforce);
           }
       }
       else
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)259199139);
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastCorvetteReinforcementTeam);
       }
   }
}

void Watch_Mission17_BeastCorvetteReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *availableShips;
   GrowSelection *readyShips;
   GrowSelection *selectedShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   availableShips = kasAITeamShipsPtrIfExists(44665972);
   readyShips = kasGrowSelectionPtr(128635107);
   selectedShips = kasGrowSelectionPtr(363040);
   kasfShipsSelectType(selectedShips, availableShips, "bMissileCorvette");
   kasfShipsAdd(readyShips, selectedShips);
   kasfShipsSelectType(selectedShips, availableShips, "bMultiGunCorvette");
   kasfShipsAdd(readyShips, selectedShips);
   kasfShipsSelectType(selectedShips, availableShips, "bHeavyCorvette");
   kasfShipsAdd(readyShips, selectedShips);
   if (kasfShipsCount(readyShips) < 3)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(14701123)))
       {
           if (kasfEntryValueGet(-2082149163))
           {
               kasfEntrySetValue(-2082149163, 0);
               kasfEntrySetValue(-1931839291, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Init_Mission17_BeastCorvetteReinforcementBuildTimer);
           }
       }
       else
       {
           kasfEntrySetValue(-2082149163, 0);
           kasfEntrySetValue(-1931839291, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)41951459);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(selectedShips, availableShips, "bMissileCorvette"))
   {
       kasfShipsRemove(availableShips, selectedShips);
       kasfShipsStop(selectedShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
       kasfShipsSetSlaveableFlag(selectedShips, 0);
       kasfShipsApplyPlayerUpgrades(selectedShips);
       kasfReinforceTeamWithShips(team, selectedShips);
       kasfTacticsNeutral();
   }
   if (kasfShipsSelectType(selectedShips, availableShips, "bMultiGunCorvette"))
   {
       kasfShipsRemove(availableShips, selectedShips);
       kasfShipsStop(selectedShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
       kasfShipsSetSlaveableFlag(selectedShips, 0);
       kasfShipsApplyPlayerUpgrades(selectedShips);
       kasfReinforceTeamWithShips(team, selectedShips);
       kasfTacticsNeutral();
   }
   if (kasfShipsSelectType(selectedShips, availableShips, "bHeavyCorvette"))
   {
       kasfShipsRemove(availableShips, selectedShips);
       kasfShipsStop(selectedShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
       kasfShipsSetSlaveableFlag(selectedShips, 0);
       kasfShipsApplyPlayerUpgrades(selectedShips);
       kasfReinforceTeamWithShips(team, selectedShips);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(
       74024315,
       (sdword)Begin_Mission17_BeastCorvetteCombatLoop,
       (sdword)Watch_Mission17_BeastCorvetteCombatLoop);
}

void Init_Mission17_BeastCorvetteReinforcementBuildTimer(void)
{
   sdword buildStyle;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildStyle = kasfEntryValueGet(-2126662828);
   if (buildStyle == 2)
   {
       if (kasfEntryValueGet(154639956) < 8)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette") / 2;
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) == 8)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_CorvetteReinforcementDelay,
               (sdword)Watch_Mission17_BeastCorvetteReinforcementBuildTimer);
       }
       return;
   }
   if (buildStyle == 1)
   {
       if (kasfEntryValueGet(154639956) < 8)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette");
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 3 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette") / 2) ||
           kasfEntryValueGet(154639956) == 8)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_CorvetteReinforcementDelay,
               (sdword)Watch_Mission17_BeastCorvetteReinforcementBuildTimer);
       }
       return;
   }
   if (buildStyle != 0)
   {
       return;
   }

   if (kasfEntryValueGet(154639956) < 8)
   {
       buildCost = kasfShipTypeBuildCost("bMissileCorvette");
       if ((sdword)kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if ((kasfEntryValueGet(154639956) > 3 &&
        (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette")) ||
       kasfEntryValueGet(154639956) == 8)
   {
       kasfSpeechEvent(
           75929221,
           (sdword)Init_Mission05_CorvetteReinforcementDelay,
           (sdword)Watch_Mission17_BeastCorvetteReinforcementBuildTimer);
   }
}

void Watch_Mission17_BeastCorvetteReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *launchTargets;
   GrowSelection *launchSelection;
   GrowSelection *createdShips;
   sdword spawnIndex;
   sdword buildCount;
   const char *shipType;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   launchTargets = kasGrowSelectionPtr(14701123);
   if (!kasfShipsCount(launchTargets))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }
   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
       launchSelection = kasGrowSelectionPtr(21541);
       kasfShipsSelectIndex(launchSelection, launchTargets, spawnIndex);
       buildCount = kasfEntryValueGet(154639956) / 2;

       switch (kasfRandom(1, 3))
       {
           case 1:
               shipType = "bMultiGunCorvette";
               break;
           case 2:
               shipType = "bHeavyCorvette";
               break;
           default:
               shipType = "bMissileCorvette";
               break;
       }
       createdShips = kasGrowSelectionPtr(154639956);
       kasfShipsCreateInside(createdShips, shipType, buildCount, launchSelection, 1);
       kasAITeamShipsPtr(createdShips, 1);
       kasfReinforceTeamWithShips(team, createdShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);

       switch (kasfRandom(1, 3))
       {
           case 1:
               shipType = "bMultiGunCorvette";
               break;
           case 2:
               shipType = "bHeavyCorvette";
               break;
           default:
               shipType = "bMissileCorvette";
               break;
       }
       createdShips = kasGrowSelectionPtr(154639956);
       kasfShipsCreateInside(createdShips, shipType, buildCount, launchSelection, 1);
       kasAITeamShipsPtr(createdShips, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_BeastCorvetteCombatLoop,
           (sdword)Watch_Mission17_BeastCorvetteCombatLoop);
   }
}

void Begin_Mission17_BeastCorvetteCombatLoop(void)
{
   AITeam *team;
   GrowSelection *attackers;
   GrowSelection *nearbyTargets;
   GrowSelection *nearbyCapitals;
   GrowSelection *priorityTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   attackers = kasGrowSelectionPtr(57245931);
   nearbyTargets = kasGrowSelectionPtr(86599289);
   if (kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 8000))
   {
       if (!kasfShipsSelectType(attackers, nearbyTargets, "sLeech") &&
           !kasfShipsSelectType(attackers, nearbyTargets, "sMCV") &&
           !kasfShipsSelectType(attackers, nearbyTargets, "sMimic"))
       {
           kasfShipsAdd(attackers, nearbyTargets);
       }
   }
   else
   {
       nearbyCapitals = kasGrowSelectionPtr(124053532);
       if (kasfShipsSelectNearby(nearbyCapitals, kasGrowSelectionPtr(team->kasFSMNameHash), kasThisTeamsVectorPtr(), 8000) &&
           kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals) &&
           kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(148487284)))
       {
           if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(team->kasFSMNameHash), "CLASS_Corvette") &&
               kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
           {
               kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
           }
           if (kasfShipsCount(nearbyCapitals))
           {
               kasfSpeechEvent(
                   0x4C9664,
                   (sdword)Init_Mission17_BeastCorvetteCapitalStrike,
                   (sdword)Watch_Mission17_BeastCorvetteCapitalStrike);
               return;
           }
       }
       kasfGrowSelectionClear(attackers);
       kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
   }
   if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
   {
       kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
   }
   kasfShipsSortByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
   if (kasfTeamCount() <= 8)
   {
       kasfFormationDelta();
   }
   else
   {
       kasfFormationDelta3D();
   }
   tutDeallocateRootRegion();
   kasTimerDestroyById(74024315);
   kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_BeastCorvetteCombatLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_BeastCorvetteCombatLoop,
           (sdword)Watch_Mission17_BeastCorvetteCombatLoop);
   }
}

void Init_Mission17_BeastCorvetteCapitalStrike(void)
{
   AITeam *team;
   GrowSelection *nearbyCapitals;
   GrowSelection *priorityTargets;
   GrowSelection *sortedTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   nearbyCapitals = kasGrowSelectionPtr(124053532);
   if (kasfShipsSelectNearby(nearbyCapitals, kasGrowSelectionPtr(team->kasFSMNameHash), kasThisTeamsVectorPtr(), 8000) &&
       kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals) &&
       kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(148487284)))
   {
       if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(team->kasFSMNameHash), "CLASS_Corvette") &&
           kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
       {
           kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
       }
       if (!kasfShipsCount(nearbyCapitals))
       {
           kasfSpeechEvent(
               74024315,
               (sdword)Begin_Mission17_BeastCorvetteCombatLoop,
               (sdword)Watch_Mission17_BeastCorvetteCombatLoop);
           return;
       }
   }
   kasfShipsSelectNearby(kasGrowSelectionPtr(86599289), nearbyCapitals, kasThisTeamsVectorPtr(), 8000);
   kasfShipsSelectType(kasGrowSelectionPtr(212896882), kasGrowSelectionPtr(86599289), "bHeavyCruiser");
   kasfShipsSelectType(kasGrowSelectionPtr(124218850), kasGrowSelectionPtr(86599289), "bCarrier");
   if (kasfShipsCount(kasGrowSelectionPtr(212896882)) || kasfShipsCount(kasGrowSelectionPtr(124218850)))
   {
       priorityTargets = kasGrowSelectionPtr(125446884);
       kasfGrowSelectionClear(priorityTargets);
       kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(212896882));
       kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(124218850));
       kasfShipsSortByDistanceToPoint(priorityTargets, kasThisTeamsVectorPtr());
       sortedTargets = priorityTargets;
   }
   else
   {
       kasfShipsSortByDistanceToPoint(nearbyCapitals, kasThisTeamsVectorPtr());
       sortedTargets = nearbyCapitals;
   }
   kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), sortedTargets, 0);
   kasfFormationSphere();
   kasfProtect(kasGrowSelectionPtr(154965556));
}

void Watch_Mission17_BeastCorvetteDockAndReinforce(void)
{
   AITeam *team;
   sdword buildStyle;
   sdword buildLimit;
   sdword buildCost;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastCorvetteReinforcementTeam);
       return;
   }

   buildStyle = kasfEntryValueGet(-2126662828);
   buildLimit = 8 - kasfTeamCount();
   if (buildStyle == 2)
   {
       if (kasfEntryValueGet(154639956) < buildLimit)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette") / 2;
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
       if (kasfEntryValueGet(154639956) == 8)
       {
           kasTimerCreateSetStartById(133682484, 0);
       }
   }
   else if (buildStyle == 1)
   {
       if (kasfEntryValueGet(154639956) < buildLimit)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette");
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 3 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette") / 2) ||
           kasfEntryValueGet(154639956) == 8)
       {
           kasTimerCreateSetStartById(
               133682484,
               kasfShipTypeBuildTime("bMissileCorvette") * kasfEntryValueGet(154639956) / 2);
       }
   }
   else if (buildStyle == 0)
   {
       if (kasfEntryValueGet(154639956) < buildLimit)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette");
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 3 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette")) ||
           kasfEntryValueGet(154639956) == 8)
       {
           kasTimerCreateSetStartById(
               133682484,
               kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bMissileCorvette"));
       }
   }

   if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
   {
       kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr(316548), kasGrowSelectionPtr(14701123));
   }
}

void Init_Mission17_BeastFighterReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(182180066)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1985856270, 0);
       kasfEntryGetOrCreateSetValue(-1929432555, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)233775571);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 1);
           kasfSpeechEvent(
               74024315,
               (sdword)Init_Mission17_BeastFighterCombatLoop,
               (sdword)Watch_Mission17_BeastFighterCombatLoop);
       }
       else
       {
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastFighterReinforcementTeam);
       }
   }
}

void Init_Mission17_BeastFighterReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(182180066)) &&
       kasfEntryValueGet(-1929432555) &&
       kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1929432555, 0);
       kasfEntrySetValue(-1985856270, 1);
   }
   if (!kasfEntryValueGet(332667))
   {
       if (!kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)154145491);
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastFighterReinforcementTeam);
           return;
       }
       if (kasfShipsCount(kasGrowSelectionPtr(14701123)) && kasfTeamHealthAverage() < 50)
       {
           kasfSpeechEvent(
               299899,
               (sdword)Begin_Mission17_BeastReconDockAndReinforce,
               (sdword)Watch_Mission17_BeastFighterDockAndReinforce);
       }
   }
}

void Watch_Mission17_BeastFighterReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *availableShips;
   GrowSelection *readyShips;
   GrowSelection *selectedShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   availableShips = kasAITeamShipsPtrIfExists(44665972);
   readyShips = kasGrowSelectionPtr(230526675);
   selectedShips = kasGrowSelectionPtr(363040);
   kasfShipsSelectType(selectedShips, availableShips, "bInterceptor");
   kasfShipsAdd(readyShips, selectedShips);
   kasfShipsSelectType(selectedShips, availableShips, "bAcolyte");
   kasfShipsAdd(readyShips, selectedShips);
   kasfShipsSelectType(selectedShips, availableShips, "bAttackBomber");
   kasfShipsAdd(readyShips, selectedShips);
   if (kasfShipsCount(readyShips) < 6)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(14701123)))
       {
           if (kasfEntryValueGet(-1985856270))
           {
               kasfEntrySetValue(-1985856270, 0);
               kasfEntrySetValue(-1929432555, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Init_Mission17_BeastFighterReinforcementBuildTimer);
           }
       }
       else
       {
           kasfEntrySetValue(-1985856270, 0);
           kasfEntrySetValue(-1929432555, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)233775571);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(selectedShips, availableShips, "bInterceptor"))
   {
       kasfShipsRemove(availableShips, selectedShips);
       kasfShipsStop(selectedShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
       kasfShipsSetSlaveableFlag(selectedShips, 0);
       kasfShipsApplyPlayerUpgrades(selectedShips);
       kasfReinforceTeamWithShips(team, selectedShips);
       kasfTacticsNeutral();
   }
   if (kasfShipsSelectType(selectedShips, availableShips, "bAcolyte"))
   {
       kasfShipsRemove(availableShips, selectedShips);
       kasfShipsStop(selectedShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
       kasfShipsSetSlaveableFlag(selectedShips, 0);
       kasfShipsApplyPlayerUpgrades(selectedShips);
       kasfReinforceTeamWithShips(team, selectedShips);
       kasfTacticsNeutral();
   }
   if (kasfShipsSelectType(selectedShips, availableShips, "bAttackBomber"))
   {
       kasfShipsRemove(availableShips, selectedShips);
       kasfShipsStop(selectedShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
       kasfShipsSetSlaveableFlag(selectedShips, 0);
       kasfShipsApplyPlayerUpgrades(selectedShips);
       kasfReinforceTeamWithShips(team, selectedShips);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(
       74024315,
       (sdword)Init_Mission17_BeastFighterCombatLoop,
       (sdword)Watch_Mission17_BeastFighterCombatLoop);
}

void Init_Mission17_BeastFighterReinforcementBuildTimer(void)
{
   sdword buildStyle;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildStyle = kasfEntryValueGet(-2126662828);
   if (buildStyle == 2)
   {
       if (kasfEntryValueGet(154639956) < 15)
       {
           kasfEntryIncrement(154639956);
       }
       if (kasfEntryValueGet(154639956) == 15)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_FighterReinforcementDelay,
               (sdword)Watch_Mission17_BeastFighterReinforcementBuildTimer);
       }
       return;
   }
   if (buildStyle == 1)
   {
       if (kasfEntryValueGet(154639956) < 15)
       {
           buildCost = kasfShipTypeBuildCost("bAcolyte") / 2;
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 6 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte") / 2) ||
           kasfEntryValueGet(154639956) == 15)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_FighterReinforcementDelay,
               (sdword)Watch_Mission17_BeastFighterReinforcementBuildTimer);
       }
       return;
   }
   if (buildStyle != 0)
   {
       return;
   }

   if (kasfEntryValueGet(154639956) < 15)
   {
       buildCost = kasfShipTypeBuildCost("bAcolyte");
       if ((sdword)kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if ((kasfEntryValueGet(154639956) > 6 &&
        (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte")) ||
       kasfEntryValueGet(154639956) == 15)
   {
       kasfSpeechEvent(
           75929221,
           (sdword)Init_Mission05_FighterReinforcementDelay,
           (sdword)Watch_Mission17_BeastFighterReinforcementBuildTimer);
   }
}

void Watch_Mission17_BeastFighterReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *launchTargets;
   GrowSelection *launchSelection;
   GrowSelection *createdShips;
   sdword spawnIndex;
   const char *shipType;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   launchTargets = kasGrowSelectionPtr(14701123);
   if (!kasfShipsCount(launchTargets))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }
   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
       launchSelection = kasGrowSelectionPtr(21541);
       kasfShipsSelectIndex(launchSelection, launchTargets, spawnIndex);
       switch (kasfRandom(1, 6))
       {
           case 1:
               shipType = "bInterceptor";
               break;
           case 2:
           case 3:
               shipType = "bAttackBomber";
               break;
           default:
               shipType = "bAcolyte";
               break;
       }
       createdShips = kasGrowSelectionPtr(154639956);
       kasfShipsCreateInside(createdShips, shipType, kasfEntryValueGet(154639956), launchSelection, 1);
       kasAITeamShipsPtr(createdShips, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)233775571);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(
           74024315,
           (sdword)Init_Mission17_BeastFighterCombatLoop,
           (sdword)Watch_Mission17_BeastFighterCombatLoop);
   }
}

void Init_Mission17_BeastFighterCombatLoop(void)
{
   AITeam *team;
   GrowSelection *attackers;
   GrowSelection *nearbyTargets;
   GrowSelection *nearbyCapitals;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   attackers = kasGrowSelectionPtr(57245931);
   nearbyTargets = kasGrowSelectionPtr(86599289);
   if (kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
   {
       if (kasfShipsSelectType(attackers, nearbyTargets, "sLeech") ||
           kasfShipsSelectType(attackers, nearbyTargets, "sMCV") ||
           kasfShipsSelectType(attackers, nearbyTargets, "sMimic") ||
           kasfShipsSelectClass(attackers, nearbyTargets, "CLASS_Fighter"))
       {
           goto sort_attackers;
       }
       kasfShipsAdd(attackers, nearbyTargets);
   }
   else
   {
       nearbyCapitals = kasGrowSelectionPtr(124053532);
       if (kasfShipsSelectNearby(nearbyCapitals, kasGrowSelectionPtr(team->kasFSMNameHash), kasThisTeamsVectorPtr(), 20000) &&
           kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals) &&
           kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(148487284)))
       {
           if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(team->kasFSMNameHash), "CLASS_Fighter") &&
               kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
           {
               kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
           }
           if (kasfShipsCount(nearbyCapitals))
           {
               kasfSpeechEvent(
                   0x4C9664,
                   (sdword)Init_Mission17_BeastFighterCapitalStrike,
                   (sdword)Watch_Mission17_BeastFighterCapitalStrike);
               return;
           }
       }
       kasfGrowSelectionClear(attackers);
       kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
   }

sort_attackers:
   if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
   {
       kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
   }
   kasfShipsSortByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
   if (kasfTeamCount() <= 8)
   {
       kasfFormationDelta();
   }
   else
   {
       kasfFormationDelta3D();
   }
   tutDeallocateRootRegion();
   kasTimerDestroyById(74024315);
   kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_BeastFighterCombatLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(
           74024315,
           (sdword)Init_Mission17_BeastFighterCombatLoop,
           (sdword)Watch_Mission17_BeastFighterCombatLoop);
   }
}

void Init_Mission17_BeastFighterCapitalStrike(void)
{
   AITeam *team;
   GrowSelection *nearbyCapitals;
   GrowSelection *priorityTargets;
   GrowSelection *sortedTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   nearbyCapitals = kasGrowSelectionPtr(124053532);
   if (kasfShipsSelectNearby(nearbyCapitals, kasGrowSelectionPtr(team->kasFSMNameHash), kasThisTeamsVectorPtr(), 20000) &&
       kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals) &&
       kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(148487284)))
   {
       if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(team->kasFSMNameHash), "CLASS_Fighter") &&
           kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
       {
           kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
       }
       if (!kasfShipsCount(nearbyCapitals))
       {
           kasfSpeechEvent(
               74024315,
               (sdword)Init_Mission17_BeastFighterCombatLoop,
               (sdword)Watch_Mission17_BeastFighterCombatLoop);
           return;
       }
   }
   kasfShipsSelectNearby(kasGrowSelectionPtr(86599289), nearbyCapitals, kasThisTeamsVectorPtr(), 20000);
   kasfShipsSelectType(kasGrowSelectionPtr(212896882), kasGrowSelectionPtr(86599289), "bHeavyCruiser");
   kasfShipsSelectType(kasGrowSelectionPtr(124218850), kasGrowSelectionPtr(86599289), "bCarrier");
   if (kasfShipsCount(kasGrowSelectionPtr(212896882)) || kasfShipsCount(kasGrowSelectionPtr(124218850)))
   {
       priorityTargets = kasGrowSelectionPtr(125446884);
       kasfGrowSelectionClear(priorityTargets);
       kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(212896882));
       kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(124218850));
       kasfShipsSortByDistanceToPoint(priorityTargets, kasThisTeamsVectorPtr());
       sortedTargets = priorityTargets;
   }
   else
   {
       kasfShipsSortByDistanceToPoint(nearbyCapitals, kasThisTeamsVectorPtr());
       sortedTargets = nearbyCapitals;
   }
   kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), sortedTargets, 0);
   kasfFormationWall();
   kasfProtect(kasGrowSelectionPtr(154965556));
}

void Watch_Mission17_BeastFighterDockAndReinforce(void)
{
   AITeam *team;
   sdword buildStyle;
   sdword buildLimit;
   sdword buildCost;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastFighterReinforcementTeam);
       return;
   }

   buildStyle = kasfEntryValueGet(-2126662828);
   buildLimit = 15 - kasfTeamCount();
   if (buildStyle == 2)
   {
       if (kasfEntryValueGet(154639956) < buildLimit)
       {
           kasfEntryIncrement(154639956);
       }
       if (kasfEntryValueGet(154639956) == 15)
       {
           kasTimerCreateSetStartById(133682484, 0);
       }
   }
   else if (buildStyle == 1)
   {
       if (kasfEntryValueGet(154639956) < buildLimit)
       {
           buildCost = kasfShipTypeBuildCost("bAcolyte") / 2;
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 6 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte") / 2) ||
           kasfEntryValueGet(154639956) == 15)
       {
           kasTimerCreateSetStartById(
               133682484,
               kasfShipTypeBuildTime("bAcolyte") * kasfEntryValueGet(154639956) / 2);
       }
   }
   else if (buildStyle == 0)
   {
       if (kasfEntryValueGet(154639956) < buildLimit)
       {
           buildCost = kasfShipTypeBuildCost("bAcolyte");
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 6 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte")) ||
           kasfEntryValueGet(154639956) == 15)
       {
           kasTimerCreateSetStartById(
               133682484,
               kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bAcolyte"));
       }
   }

   if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
   {
       kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr(316548), kasGrowSelectionPtr(14701123));
   }
}

void Init_Mission17_BeastRammingFrigateReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(223105365)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1997402011, 0);
       kasfEntryGetOrCreateSetValue(-2141093323, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)65242483);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasfSpeechEvent(
               74024315,
               (sdword)Begin_BeastRammingFrigateCombatLoop,
               (sdword)Watch_Mission17_BeastRammingFrigateCombatLoop);
       }
       else
       {
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastRammingFrigateReinforcementTeam);
       }
   }
}

void Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(223105365)) &&
       kasfEntryValueGet(-2141093323) &&
       kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-2141093323, 0);
       kasfEntrySetValue(-1997402011, 1);
   }
   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)203347251);
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastRammingFrigateReinforcementTeam);
   }
}

void Watch_Mission17_BeastRammingFrigateReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *rammingFrigates;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   rammingFrigates = kasGrowSelectionPtr(363040);
   if (kasfShipsSelectType(rammingFrigates, kasAITeamShipsPtrIfExists(44665972), "bRammingFrigate") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(14701123)))
       {
           if (kasfEntryValueGet(-1997402011))
           {
               kasfEntrySetValue(-1997402011, 0);
               kasfEntrySetValue(-2141093323, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Init_Mission17_BeastRammingFrigateReinforcementBuildTimer);
           }
       }
       else
       {
           kasfEntrySetValue(-1997402011, 0);
           kasfEntrySetValue(-2141093323, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)65242483);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(rammingFrigates, kasAITeamShipsPtrIfExists(44665972), "bRammingFrigate"))
   {
       kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), rammingFrigates);
       kasfShipsStop(rammingFrigates);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), rammingFrigates);
       kasfShipsSetSlaveableFlag(rammingFrigates, 0);
       kasfShipsApplyPlayerUpgrades(rammingFrigates);
       kasfReinforceTeamWithShips(team, rammingFrigates);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(
       74024315,
       (sdword)Begin_BeastRammingFrigateCombatLoop,
       (sdword)Watch_Mission17_BeastRammingFrigateCombatLoop);
}

void Init_Mission17_BeastRammingFrigateReinforcementBuildTimer(void)
{
   sdword buildStyle;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildStyle = kasfEntryValueGet(-2126662828);
   if (buildStyle == 2)
   {
       if (kasfEntryValueGet(154639956) < 2)
       {
           kasfEntryIncrement(154639956);
       }
       if (kasfEntryValueGet(154639956) == 2)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
               (sdword)Watch_Mission17_BeastRammingFrigateReinforcementBuildTimer);
       }
       return;
   }
   if (buildStyle == 1)
   {
       if (kasfEntryValueGet(154639956) < 2)
       {
           buildCost = kasfShipTypeBuildCost("bRammingFrigate") / 2;
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 1 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRammingFrigate") / 2) ||
           kasfEntryValueGet(154639956) == 2)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
               (sdword)Watch_Mission17_BeastRammingFrigateReinforcementBuildTimer);
       }
       return;
   }
   if (buildStyle != 0)
   {
       return;
   }

   if (kasfEntryValueGet(154639956) < 2)
   {
       buildCost = kasfShipTypeBuildCost("bRammingFrigate");
       if ((sdword)kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if ((kasfEntryValueGet(154639956) > 1 &&
        (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRammingFrigate")) ||
       kasfEntryValueGet(154639956) == 2)
   {
       kasfSpeechEvent(
           75929221,
           (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
           (sdword)Watch_Mission17_BeastRammingFrigateReinforcementBuildTimer);
   }
}

void Watch_Mission17_BeastRammingFrigateReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *launchTargets;
   GrowSelection *launchSelection;
   GrowSelection *createdShips;
   sdword spawnIndex;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   launchTargets = kasGrowSelectionPtr(14701123);
   if (!kasfShipsCount(launchTargets))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }
   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
       launchSelection = kasGrowSelectionPtr(21541);
       kasfShipsSelectIndex(launchSelection, launchTargets, spawnIndex);
       createdShips = kasGrowSelectionPtr(154639956);
       kasfShipsCreateInside(createdShips, "bRammingFrigate", kasfEntryValueGet(154639956), launchSelection, 1);
       kasAITeamShipsPtr(createdShips, 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdShips);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)65242483);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_BeastRammingFrigateCombatLoop,
           (sdword)Watch_Mission17_BeastRammingFrigateCombatLoop);
   }
}

void Watch_Mission17_BeastRammingFrigateCombatLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_BeastRammingFrigateCombatLoop,
           (sdword)Watch_Mission17_BeastRammingFrigateCombatLoop);
   }
}

void Init_Mission17_BeastReconReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(0x56983E)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-2075791394, 0);
        kasfEntryGetOrCreateSetValue(-1880696891, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)208220819);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(
                74024315,
                (sdword)Begin_Mission17_BeastReconCombatLoop,
                (sdword)Watch_Mission17_BeastReconCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastReconReinforcementTeam);
        }
    }
}

void Init_Mission17_BeastReconReinforcementTeam_Reevaluate(void)
{
    AITeam *team;
    GrowSelection *launchTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x56983E)) &&
        kasfEntryValueGet(-1880696891) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1880696891, 0);
        kasfEntrySetValue(-2075791394, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)158207907);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastReconReinforcementTeam);
        return;
    }
    launchTargets = kasGrowSelectionPtr(0xE05243);
    if (kasfShipsCount(launchTargets) && kasfTeamHealthAverage() < 30)
    {
        kasfSpeechEvent(
            299899,
            (sdword)Begin_Mission17_BeastReconDockAndReinforce,
            (sdword)Watch_Mission17_BeastReconDockAndReinforce);
    }
}

void Watch_Mission17_BeastReconReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *reconShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    reconShips = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(reconShips, kasAITeamShipsPtrIfExists(44665972), "bRecon") < 2)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
        {
            if (kasfEntryValueGet(-2075791394))
            {
                kasfEntrySetValue(-2075791394, 0);
                kasfEntrySetValue(-1880696891, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_Mission17_BeastReconReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-2075791394, 0);
            kasfEntrySetValue(-1880696891, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)208220819);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(reconShips, kasAITeamShipsPtrIfExists(44665972), "bRecon"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), reconShips);
        kasfShipsStop(reconShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), reconShips);
        kasfShipsSetSlaveableFlag(reconShips, 0);
        kasfShipsApplyPlayerUpgrades(reconShips);
        kasfReinforceTeamWithShips(team, reconShips);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_BeastReconCombatLoop,
        (sdword)Watch_Mission17_BeastReconCombatLoop);
}

void Init_Mission17_BeastReconReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < 8)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 8)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastReconReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastReconReinforcementBuildTimer);
        }
        return;
    }
    if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < 8)
        {
            buildCost = kasfShipTypeBuildCost("bRecon") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 2 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRecon") / 2) ||
            kasfEntryValueGet(154639956) == 8)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastReconReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastReconReinforcementBuildTimer);
        }
        return;
    }
    if (buildScale != 0)
    {
        return;
    }
    if (kasfEntryValueGet(154639956) < 8)
    {
        buildCost = kasfShipTypeBuildCost("bRecon");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 2 &&
         (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRecon")) ||
        kasfEntryValueGet(154639956) == 8)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_BeastReconReinforcementBuildTimer,
            (sdword)Watch_Mission17_BeastReconReinforcementBuildTimer);
    }
}

void Watch_Mission17_BeastReconReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnPointSelection;
    GrowSelection *createdShips;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasGrowSelectionPtr(0xE05243);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnPointSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnPointSelection, launchTargets, spawnIndex);
        createdShips = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(createdShips, "bRecon", kasfEntryValueGet(154639956), spawnPointSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfShipsSetSlaveableFlag(createdShips, 0);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }
    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)208220819);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastReconCombatLoop,
            (sdword)Watch_Mission17_BeastReconCombatLoop);
    }
}

void Begin_Mission17_BeastReconCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *nearbyEnemies;
    GrowSelection *underAttackShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr(0x36980EB);
    nearbyEnemies = kasGrowSelectionPtr(0x5296679);
    if (kasfShipsSelectNearby(nearbyEnemies, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        if (!kasfShipsSelectType(attackTargets, nearbyEnemies, "sLeech") &&
            !kasfShipsSelectType(attackTargets, nearbyEnemies, "sMCV") &&
            !kasfShipsSelectType(attackTargets, nearbyEnemies, "sMimic") &&
            !kasfShipsSelectClass(attackTargets, nearbyEnemies, "CLASS_Fighter"))
        {
            kasfGrowSelectionClear(attackTargets);
            kasfShipsAdd(attackTargets, nearbyEnemies);
        }
    }
    else if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission17_BeastReconCombatAttackRun,
            (sdword)Watch_Mission17_BeastReconCombatAttackRun);
        return;
    }
    else
    {
        kasfGrowSelectionClear(attackTargets);
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr(0x4A2A29));
    }

    underAttackShips = kasGrowSelectionPtr(139967715);
    if (kasfUnderAttack(underAttackShips))
    {
        kasfShipsAdd(attackTargets, underAttackShips);
    }
    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    if (kasfTeamCount() > 8)
    {
        kasfFormationDelta3D();
    }
    else
    {
        kasfFormationDelta();
    }
    tutDeallocateRootRegion(attackTargets);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_BeastReconCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastReconCombatLoop,
            (sdword)Watch_Mission17_BeastReconCombatLoop);
    }
}

void Begin_Mission17_BeastReconCombatAttackRun(void)
{
    GrowSelection *launchTargets;
    sdword targetIndex;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    launchTargets = kasGrowSelectionPtr(0xE05243);
    if (!kasfShipsCount(launchTargets))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastReconCombatLoop,
            (sdword)Watch_Mission17_BeastReconCombatLoop);
        return;
    }
    targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
    kasfShipsSelectIndex(kasGrowSelectionPtr(21541), launchTargets, targetIndex);
    kasfLabelledVectorSet(0x3100F7C, 0, 0, 0);
    kasfVectorRandomizeInRange(kasVectorPtrIfExists(0x3100F7C), kasShipsVectorPtrIfExists(21541), 6000);
    kasfMoveTo(kasVectorPtrIfExists(0x3100F7C));
}

void Watch_Mission17_BeastReconCombatAttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastReconCombatLoop,
            (sdword)Watch_Mission17_BeastReconCombatLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission17_BeastReconCombatAttackRun,
            (sdword)Watch_Mission17_BeastReconCombatAttackRun);
    }
}

void Watch_Mission17_BeastReconDockAndReinforce(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *linkedLaunchTargets;
    sdword buildScale;
    sdword maxBuildCount;
    sdword buildCost;
    sdword buildTime;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastReconReinforcementTeam);
        return;
    }

    buildScale = kasfEntryValueGet(-2126662828);
    maxBuildCount = 8 - kasfTeamCount();
    if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < maxBuildCount)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 8)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfShipTypeBuildTime("bRecon") * kasfEntryValueGet(154639956) / 2);
        }
    }
    else if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < maxBuildCount)
        {
            buildCost = kasfShipTypeBuildCost("bRecon") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 2 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRecon") / 2) ||
            kasfEntryValueGet(154639956) == 8)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfShipTypeBuildTime("bRecon") * kasfEntryValueGet(154639956) / 2);
        }
    }
    else if (buildScale == 0)
    {
        if (kasfEntryValueGet(154639956) < maxBuildCount)
        {
            buildCost = kasfShipTypeBuildCost("bRecon");
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 2 &&
             (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRecon")) ||
            kasfEntryValueGet(154639956) == 8)
        {
            buildTime = kasfShipTypeBuildTime("bRecon");
            kasTimerCreateSetStartById(133682484, kasfEntryValueGet(154639956) * buildTime);
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        launchTargets = kasGrowSelectionPtr(0xE05243);
        linkedLaunchTargets = kasGrowSelectionPtr(0x4D484);
        kasfShipsSelectLinkedWith(&team->shipList, linkedLaunchTargets, launchTargets);
    }
}

void Init_Mission17_BeastCruiseMissileReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (!kasfThisTeamIs(kasAITeamPtrIfExists(0x9919BE5)))
   {
       kasfEntryGetOrCreateSetValue(154639956, 0);
       kasfEntryGetOrCreateSetValue(111592388, 0);
       kasfEntryGetOrCreateSetValue(-1964470315, 0);
       kasfEntryGetOrCreateSetValue(-1881969579, 1);
       if (kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)155914035);
           kasAITeamShipsPtr(&team->shipList, 1);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
           kasfShipsApplyPlayerUpgrades(&team->shipList);
           kasfEntrySetValue(332667, 0);
           kasfSpeechEvent(
               74024315,
               (sdword)Begin_Mission17_BeastCruiseMissileCombatLoop,
               (sdword)Watch_Mission17_BeastCruiseMissileCombatLoop);
       }
       else
       {
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastCruiseMissileReinforcementTeam);
       }
   }
}

void Init_Mission17_BeastCruiseMissileReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(0x9919BE5)) &&
       kasfEntryValueGet(-1881969579) &&
       kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1881969579, 0);
       kasfEntrySetValue(-1964470315, 1);
   }
   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)227819059);
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastCruiseMissileReinforcementTeam);
   }
}

void Watch_Mission17_BeastCruiseMissileReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *cruiseMissiles;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   cruiseMissiles = kasGrowSelectionPtr(363040);
   if (kasfShipsSelectType(cruiseMissiles, kasAITeamShipsPtrIfExists(44665972), "bCruiseMissile") >= 1)
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)155914035);
       kasfEntrySetValue(332667, 0);
       if (kasfShipsSelectType(cruiseMissiles, kasAITeamShipsPtrIfExists(44665972), "bCruiseMissile"))
       {
           kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), cruiseMissiles);
           kasfShipsStop(cruiseMissiles);
           kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), cruiseMissiles);
           kasfShipsSetSlaveableFlag(cruiseMissiles, 0);
           kasfShipsApplyPlayerUpgrades(cruiseMissiles);
           kasfReinforceTeamWithShips(team, cruiseMissiles);
           kasfTacticsNeutral();
       }
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_BeastCruiseMissileCombatLoop,
           (sdword)Watch_Mission17_BeastCruiseMissileCombatLoop);
       return;
   }

   if (!kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
   {
       kasfEntrySetValue(-1964470315, 0);
       kasfEntrySetValue(-1881969579, 1);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }
   if (kasfEntryValueGet(-1964470315))
   {
       kasfEntrySetValue(-1964470315, 0);
       kasfEntrySetValue(-1881969579, 1);
       kasfSpeechEvent(
           165710423,
           (sdword)ResetReinforcementBuildCount,
           (sdword)Init_Mission17_BeastCruiseMissileReinforcementBuildTimer);
   }
}

void Init_Mission17_BeastCruiseMissileReinforcementBuildTimer(void)
{
   sdword buildScale;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   buildScale = kasfEntryValueGet(-2126662828);
   if (buildScale == 2)
   {
       if (kasfEntryValueGet(154639956) < 6)
       {
           kasfEntryIncrement(154639956);
       }
       if (kasfEntryValueGet(154639956) == 6)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Begin_BeastCruiseMissileReinforcementBuildTimer,
               (sdword)Watch_Mission17_BeastCruiseMissileReinforcementBuildTimer);
       }
       return;
   }
   if (buildScale == 1)
   {
       if (kasfEntryValueGet(154639956) < 6)
       {
           buildCost = kasfShipTypeBuildCost("bCruiseMissile") / 2;
           if ((sdword)kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 1 &&
            (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCruiseMissile") / 2) ||
           kasfEntryValueGet(154639956) == 6)
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Begin_BeastCruiseMissileReinforcementBuildTimer,
               (sdword)Watch_Mission17_BeastCruiseMissileReinforcementBuildTimer);
       }
       return;
   }
   if (buildScale != 0)
   {
       return;
   }
   if (kasfEntryValueGet(154639956) < 6)
   {
       buildCost = kasfShipTypeBuildCost("bCruiseMissile");
       if ((sdword)kasfRUsGet(1) >= buildCost)
       {
           kasfRUsSet(1, kasfRUsGet(1) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if ((kasfEntryValueGet(154639956) > 1 &&
        (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCruiseMissile")) ||
       kasfEntryValueGet(154639956) == 6)
   {
       kasfSpeechEvent(
           75929221,
           (sdword)Begin_BeastCruiseMissileReinforcementBuildTimer,
           (sdword)Watch_Mission17_BeastCruiseMissileReinforcementBuildTimer);
   }
}

void Watch_Mission17_BeastCruiseMissileReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *launchTargets;
   GrowSelection *spawnPointSelection;
   GrowSelection *createdMissiles;
   sdword spawnIndex;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   launchTargets = kasGrowSelectionPtr(0xE05243);
   if (!kasfShipsCount(launchTargets))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }
   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
       spawnPointSelection = kasGrowSelectionPtr(21541);
       kasfShipsSelectIndex(spawnPointSelection, launchTargets, spawnIndex);
       createdMissiles = kasGrowSelectionPtr(154639956);
       kasfShipsCreateInside(
           createdMissiles,
           "bCruiseMissile",
           kasfEntryValueGet(154639956),
           spawnPointSelection,
           1);
       kasAITeamShipsPtr(createdMissiles, 1);
       kasfShipsSetSlaveableFlag(createdMissiles, 0);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdMissiles);
       kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
   }
   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)155914035);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_BeastCruiseMissileCombatLoop,
           (sdword)Watch_Mission17_BeastCruiseMissileCombatLoop);
   }
}

void Begin_Mission17_BeastCruiseMissileCombatLoop(void)
{
   AITeam *team;
   GrowSelection *attackTargets;
   GrowSelection *nearbyEnemies;
   GrowSelection *underAttackShips;
   GrowSelection *capitalTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   attackTargets = kasGrowSelectionPtr(0x36980EB);
   nearbyEnemies = kasGrowSelectionPtr(0x5296679);
   if (kasfShipsSelectNearby(nearbyEnemies, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
   {
       if (!kasfShipsSelectCapital(attackTargets, nearbyEnemies))
       {
           kasfShipsAdd(attackTargets, nearbyEnemies);
       }
   }
   else
   {
       kasfGrowSelectionClear(attackTargets);
       kasfShipsAdd(attackTargets, kasGrowSelectionPtr(0x4A2A29));
   }

   underAttackShips = kasGrowSelectionPtr(139967715);
   if (kasfUnderAttack(underAttackShips) &&
       kasfShipsSelectMembersOfSelection(underAttackShips, underAttackShips, attackTargets))
   {
       kasfShipsAdd(attackTargets, underAttackShips);
   }
   kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
   kasfFormationWall();
   capitalTargets = kasGrowSelectionPtr(124053532);
   if (kasfShipsSelectCapital(capitalTargets, attackTargets))
   {
       tutDeallocateRootRegion(capitalTargets);
   }
   else
   {
       tutDeallocateRootRegion(attackTargets);
   }
   kasTimerDestroyById(74024315);
   kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_BeastCruiseMissileCombatLoop(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_BeastCruiseMissileCombatLoop,
           (sdword)Watch_Mission17_BeastCruiseMissileCombatLoop);
   }
}

void Init_Mission17_BeastDFGFrigateReinforcementTeam(void)
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
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastDFGFrigateReinforcementTeam);
        }
    }
}

void Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate(void)
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
            if (kasfShipsCount(kasGrowSelectionPtr(165711363)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_Mission17_BeastDFGFrigateDockAndReinforce,
                    (sdword)Watch_Mission17_BeastDFGFrigateDockAndReinforce);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)228101699);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastDFGFrigateReinforcementTeam);
        }
    }
}

void Watch_Mission17_BeastDFGFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *newFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    newFrigates = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bDFGFrigate") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(165711363)))
        {
            if (kasfEntryValueGet(-1981522985))
            {
                kasfEntrySetValue(-1981522985, 0);
                kasfEntrySetValue(-1949140875, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_Mission17_BeastDFGFrigateReinforcementBuildTimer);
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

void Init_Mission17_BeastDFGFrigateReinforcementBuildTimer(void)
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
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastDFGFrigateReinforcementBuildTimer);
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
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastDFGFrigateReinforcementBuildTimer);
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
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer,
            (sdword)Watch_Mission17_BeastDFGFrigateReinforcementBuildTimer);
    }
}

void Watch_Mission17_BeastDFGFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *spawnParents;
    GrowSelection *spawnPointSelection;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasGrowSelectionPtr(165711363);
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
        spawnPointSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnPointSelection, spawnParents, targetIndex);
        createdShips = kasGrowSelectionPtr(154639956);
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

void Watch_Mission17_BeastDFGFrigateDockAndReinforce(void)
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
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastDFGFrigateReinforcementTeam);
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
        kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr(316548), kasGrowSelectionPtr(165711363));
    }
}

void Init_Mission17_BeastACVReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfTeamSwitchPlayerOwner(0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(129010738)))
    {
        return;
    }
    kasfEntrySetValue(154639956, 0);
    kasfEntrySetValue(111592388, 0);
    kasfEntrySetValue(0x84C96822, 0);
    kasfEntrySetValue(0x88225B45, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastACVReinforcementTeam);
        return;
    }
    kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x0B0DED43);
    kasAITeamShipsPtr(&team->shipList, 3);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_ReinforcementAttackRun,
        (sdword)Watch_Mission17_HeavyCruiserCombatTeamAttackStaging);
}

void Init_Mission17_BeastACVReinforcementTeam_Reevaluate(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(129010738)) &&
        kasfEntryValueGet(0x88225B45) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x88225B45, 0);
        kasfEntrySetValue(0x84C96822, 1);
    }

    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x0B401E73);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastACVReinforcementTeam);
        return;
    }

    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)0x078A9955)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                kasfVarDestroy(&team->shipList);
            }
            else if (message == (void *)0x0E2336A5)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Watch_Mission17_BeastACVReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(0x84C96822, 0);
        kasfEntrySetValue(0x88225B45, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x84C96822))
    {
        kasfEntrySetValue(0x84C96822, 0);
        kasfEntrySetValue(0x88225B45, 1);
        kasfSpeechEvent(
            0x09E08A57,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_HeavyCruiserCombatTeamBuildCount);
    }
}

void Init_Mission17_IonCannonFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfTeamSwitchPlayerOwner(0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(205745797)))
    {
        return;
    }
    kasfEntrySetValue(154639956, 0);
    kasfEntrySetValue(111592388, 0);
    kasfEntrySetValue(0x88721F65, 0);
    kasfEntrySetValue(0x8F620235, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_IonCannonFrigateReinforcementTeam);
        return;
    }
    kasAITeamShipsPtr(&team->shipList, 3);
    kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x0676ED73);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_ReinforcementAttackRun,
        (sdword)Watch_Mission17_IonCannonFrigateCombatRetryDelay);
}

void Init_Mission17_IonCannonFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(205745797)) &&
        kasfEntryValueGet(0x8F620235) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x8F620235, 0);
        kasfEntrySetValue(0x88721F65, 1);
    }

    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x0B66A0B3);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_IonCannonFrigateReinforcementTeam);
        return;
    }

    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)0x078A9955)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                kasfVarDestroy(&team->shipList);
            }
            else if (message == (void *)0x0E2336A5)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Watch_Mission17_IonCannonFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(0x88721F65, 0);
        kasfEntrySetValue(0x8F620235, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x88721F65))
    {
        kasfEntrySetValue(0x88721F65, 0);
        kasfEntrySetValue(0x8F620235, 1);
        kasfSpeechEvent(
            0x09E08A57,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_IonCannonFrigateReinforcementBuildCount);
    }
}

void Watch_Mission17_IonCannonFrigateReinforcementBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 4)
    {
        buildCost = kasfShipTypeBuildCost("tIonCannonFrigate");
        if ((sdword)kasfRUsGet(3) >= buildCost)
        {
            kasfRUsSet(3, kasfRUsGet(3) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tIonCannonFrigate")) ||
        kasfEntryValueGet(154639956) == 4)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_IonCannonFrigateReinforcementBuildTimer,
            (sdword)Watch_Mission17_IonCannonFrigateReinforcementBuildTimer);
    }
}

void Begin_Mission17_IonCannonFrigateReinforcementBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    kasTimerCreateSetStartById(
        235522949,
        kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tIonCannonFrigate"));
}

void Watch_Mission17_IonCannonFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (!kasTimerExpiredById(235522949))
    {
        return;
    }

    kasfStop();
    createdShips = kasGrowSelectionPtr((char *)154639956);
    kasfShipsCreate(createdShips, "tIonCannonFrigate", kasfEntryValueGet(154639956), 0);
    kasAITeamShipsPtr(createdShips, 3);
    kasfShipsSetSupportFlag(createdShips, 0);
    kasfEntrySetValue(154639956, 0);
    kasfReinforceTeamWithShips(team, createdShips);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_IonCannonFrigateCombatLoop);
}

void Init_Mission17_MultigunCorvetteReinforcementTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntrySetValue(332667, 1);
   kasfTeamSwitchPlayerOwner(0);
   if (kasfThisTeamIs(kasAITeamPtrIfExists(75163141)))
   {
       return;
   }
   kasfEntrySetValue(154639956, 0);
   kasfEntrySetValue(111592388, 0);
   kasfEntrySetValue(0x8D9EF595, 0);
   kasfEntrySetValue(0x85942ED5, 1);
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_MultigunCorvetteReinforcementTeam);
       return;
   }
   kasAITeamShipsPtr(&team->shipList, 3);
   kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x028020E3);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfEntrySetValue(332667, 0);
   kasfSpeechEvent(
       74024315,
       (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
       (sdword)Watch_Mission17_MultigunCorvetteCombatLoop);
}

void Init_Mission17_MultigunCorvetteReinforcementTeam_Reevaluate(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists(75163141)) &&
       kasfEntryValueGet(0x85942ED5) &&
       kasfMsgReceived(4693508))
   {
       kasfEntrySetValue(0x85942ED5, 0);
       kasfEntrySetValue(0x8D9EF595, 1);
   }

   if (kasfEntryValueGet(332667))
   {
       return;
   }
   if (!kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x0F7310A3);
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_MultigunCorvetteReinforcementTeam);
       return;
   }
   if (kasfShipsCount(kasAITeamShipsPtrIfExists(108453441)) && kasfTeamHealthAverage() < 50)
   {
       kasfSpeechEvent(
           299899,
           (sdword)Begin_Mission17_BeastDFGFrigateDockAndReinforce,
           (sdword)Watch_Mission17_MultigunCorvetteDockAndReinforce);
   }
}

void Watch_Mission17_MultigunCorvetteReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *reinforcementShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsCount(kasAITeamShipsPtrIfExists(75163141)) >= 1)
   {
       kasfEntrySetValue(332667, 0);
       reinforcementShips = kasAITeamShipsPtrIfExists(75163141);
       kasfReinforceTeamWithShips(team, reinforcementShips);
       kasfShipsRemove(reinforcementShips, &team->shipList);
       kasfTacticsNeutral();
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
           (sdword)Watch_Mission17_MultigunCorvetteCombatLoop);
       return;
   }
   if (!kasfShipsCount(kasAITeamShipsPtrIfExists(108453441)))
   {
       kasfEntrySetValue(0x8D9EF595, 0);
       kasfEntrySetValue(0x85942ED5, 1);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
       return;
   }
   if (kasfEntryValueGet(0x8D9EF595))
   {
       kasfEntrySetValue(0x8D9EF595, 0);
       kasfEntrySetValue(0x85942ED5, 1);
       kasfSpeechEvent(
           0x09E08A57,
           (sdword)ResetReinforcementBuildCount,
           (sdword)Watch_Mission17_MultigunCorvetteReinforcementBuildCount);
   }
}

void Watch_Mission17_MultigunCorvetteReinforcementBuildCount(void)
{
   AITeam *team;
   sdword buildCost;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfEntryValueGet(154639956) < 8)
   {
       buildCost = kasfShipTypeBuildCost("tMultigunCorvette");
       if ((sdword)kasfRUsGet(3) >= buildCost)
       {
           kasfRUsSet(3, kasfRUsGet(3) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if ((kasfEntryValueGet(154639956) > 1 &&
        (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tMultigunCorvette")) ||
       kasfEntryValueGet(154639956) == 8)
   {
       kasfSpeechEvent(
           75929221,
           (sdword)Begin_Mission17_MultigunCorvetteReinforcementBuildTimer,
           (sdword)Watch_Mission17_MultigunCorvetteReinforcementBuildTimer);
   }
}

void Begin_Mission17_MultigunCorvetteReinforcementBuildTimer(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   kasTimerDestroyById(235522949);
   kasTimerCreateSetStartById(
       235522949,
       kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tMultigunCorvette"));
}

void Watch_Mission17_MultigunCorvetteReinforcementBuildTimer(void)
{
   AITeam *team;
   GrowSelection *spawnParents;
   GrowSelection *spawnTarget;
   GrowSelection *createdShips;
   sdword targetIndex;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   spawnParents = kasAITeamShipsPtrIfExists(108453441);
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
       spawnTarget = kasGrowSelectionPtr((char *)21541);
       kasfShipsSelectIndex(spawnTarget, spawnParents, targetIndex);
       createdShips = kasGrowSelectionPtr((char *)154639956);
       kasfShipsCreateInside(
           createdShips,
           "tMultigunCorvette",
           kasfEntryValueGet(154639956),
           kasGrowSelectionPtr((char *)21541),
           0);
       kasAITeamShipsPtr(createdShips, 3);
       kasfShipsSetSupportFlag(createdShips, 0);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, createdShips);
   }
   if (kasfTeamFinishedLaunching())
   {
       kasfEntrySetValue(332667, 0);
       kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x028020E3);
       kasfSpeechEvent(
           74024315,
           (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
           (sdword)Watch_Mission17_MultigunCorvetteCombatLoop);
   }
}

void Watch_Mission17_MultigunCorvetteDockAndReinforce(void)
{
   AITeam *team;
   sdword missingCount;
   sdword buildCost;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_MultigunCorvetteReinforcementTeam);
       return;
   }

   missingCount = 8 - kasfTeamCount();
   if (kasfEntryValueGet(154639956) < missingCount)
   {
       buildCost = kasfShipTypeBuildCost("tMultigunCorvette");
       if ((sdword)kasfRUsGet(3) >= buildCost)
       {
           kasfRUsSet(3, kasfRUsGet(3) - buildCost);
           kasfEntryIncrement(154639956);
       }
   }
   if ((kasfEntryValueGet(154639956) > 1 &&
        (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tMultigunCorvette")) ||
       kasfEntryValueGet(154639956) == 8)
   {
       kasTimerCreateSetStartById(
           133682484,
           kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tMultigunCorvette"));
   }
   if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
   {
       kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr((char *)316548), kasAITeamShipsPtrIfExists(108453441));
   }
}

void Init_Mission17_HeavyCruiserReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists((sdword)0x00B08C22)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x83C96832, 0);
    kasfEntryGetOrCreateSetValue(0x88355B45, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_HeavyCruiserReinforcementTeam);
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)185462083);
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfShipsSetMaxVelocity(&team->shipList, 190);
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_HeavyCruiserCombatAttackRun,
        (sdword)Watch_Mission17_HeavyCruiserCombatRetryDelay);
}

void Init_Mission17_HeavyCruiserReinforcementTeam_Reevaluate(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists((sdword)0x00B08C22)) &&
        kasfEntryValueGet(0x88355B45) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x88355B45, 0);
        kasfEntrySetValue(0x83C96832, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)188751475);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_HeavyCruiserReinforcementTeam);
        return;
    }

    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)126523733)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                kasfVarDestroy(&team->shipList);
            }
            else if (message == (void *)237188773)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Watch_Mission17_HeavyCruiserReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(0x83C96832, 0);
        kasfEntrySetValue(0x88355B45, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x83C96832))
    {
        kasfEntrySetValue(0x83C96832, 0);
        kasfEntrySetValue(0x88355B45, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_HeavyCruiserReinforcementBuildCount);
    }
}

void Watch_Mission17_HeavyCruiserReinforcementBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 1)
    {
        buildCost = kasfShipTypeBuildCost("tHeavyCruiser");
        if ((sdword)kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(2) < kasfShipTypeBuildCost("tHeavyCruiser")) ||
        kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_HeavyCruiserCombatTeamBuildTimer,
            (sdword)Watch_Mission17_HeavyCruiserReinforcementBuildTimer);
    }
}

void Watch_Mission17_HeavyCruiserReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (!kasTimerExpiredById(235522949))
    {
        return;
    }

    kasfStop();
    createdShips = kasGrowSelectionPtr((char *)154639956);
    kasfShipsCreate(createdShips, "tHeavyCruiser", kasfEntryValueGet(154639956), 1);
    kasAITeamShipsPtr(createdShips, 2);
    kasfEntrySetValue(154639956, 0);
    kasfReinforceTeamWithShips(team, createdShips);
    kasfShipsSetMaxVelocity(&team->shipList, 190);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_HeavyCruiserCombatLoop);
}

void Init_Mission17_CarrierReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *carrierTeam;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(124217378)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(0x80F92912, 0);
        kasfEntryGetOrCreateSetValue(0x89168B05, 1);
        if (kasfTeamCount())
        {
            carrierTeam = kasGrowSelectionPtr((char *)140367886);
            kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)103051523);
            kasAITeamShipsPtr(&team->shipList, 2);
            kasfShipsAdd(carrierTeam, &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfShipsSetSlaveableFlag(&team->shipList, 0);
            kasfEntrySetValue(332667, 0);
            kasfShipsSetMaxVelocity(&team->shipList, 190);
            kasfSpeechEvent(
                337061,
                (sdword)Begin_Mission17_CarrierCombatAttackRun,
                (sdword)Watch_Mission17_CarrierCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_CarrierReinforcementTeam);
        }
    }
}

void Init_Mission17_CarrierReinforcementTeam_Reevaluate(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(124217378)) &&
        kasfEntryValueGet(0x89168B05) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x89168B05, 0);
        kasfEntrySetValue(0x80F92912, 1);
    }
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            queueInfo = *((sdword **)team + 20);
            queueIndex = -1;
            if (queueInfo != NULL)
            {
                queueIndex = *queueInfo;
            }
            while (queueIndex >= 0)
            {
                message = aitQueuePopPointerByIndex(team, queueIndex);
                if (message != NULL)
                {
                    if (message == (void *)126523733)
                    {
                        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                        kasfVarDestroy(&team->shipList);
                    }
                    else if (message == (void *)237188773)
                    {
                        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
                    }
                    else
                    {
                        aitQueuePointer(team, team, (void *)message);
                    }
                }

                queueInfo = *((sdword **)team + 20);
                if (queueInfo == NULL || *queueInfo == 0)
                {
                    break;
                }
                queueIndex--;
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)50020867);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_CarrierReinforcementTeam);
        }
    }
}

void Watch_Mission17_CarrierReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        if (kasfEntryValueGet(0x80F92912))
        {
            kasfEntrySetValue(0x80F92912, 0);
            kasfEntrySetValue(0x89168B05, 1);
            kasfSpeechEvent(
                165710423,
                (sdword)ResetReinforcementBuildCount,
                (sdword)Watch_Mission17_CarrierReinforcementBuildCount);
        }
    }
    else
    {
        kasfEntrySetValue(0x80F92912, 0);
        kasfEntrySetValue(0x89168B05, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)Watch_Mission17_CarrierReinforcementTeam);
    }
}

void Watch_Mission17_CarrierReinforcementBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 1)
    {
        buildCost = kasfShipTypeBuildCost((sdword)"tCarrier");
        if (kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - kasfShipTypeBuildCost((sdword)"tCarrier"));
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         kasfRUsGet(2) < kasfShipTypeBuildCost((sdword)"tCarrier")) ||
        kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_CarrierReinforcementBuildTimer,
            (sdword)Watch_Mission17_CarrierReinforcementBuildTimer);
    }
}

void Begin_Mission17_CarrierReinforcementBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    kasTimerCreateSetStartById(
        235522949,
        kasfEntryValueGet(154639956) * kasfShipTypeBuildTime((sdword)"tCarrier"));
}

void Watch_Mission17_CarrierReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        if (kasTimerExpiredById(235522949))
        {
            kasfStop();
            createdShips = kasGrowSelectionPtr((char *)154639956);
            kasfShipsCreate(createdShips, "tCarrier", kasfEntryValueGet(154639956), 1);
            kasAITeamShipsPtr(createdShips, 2);
            kasfEntrySetValue(154639956, 0);
            kasfReinforceTeamWithShips(team, createdShips);
            kasfShipsSetMaxVelocity(&team->shipList, 190);
            kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
            kasfShipsSetSlaveableFlag(&team->shipList, 0);
            kasfSpeechEvent(
                175938357,
                (sdword)ResetReinforcementHyperspaceAttackFlag,
                (sdword)Begin_Mission17_CarrierCombatLoop);
        }
    }
    else
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)Watch_Mission17_CarrierReinforcementTeam);
    }
}

void Begin_Mission17_StandardDestroyerCombatLoop(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *attackTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108434696);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        attackTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(attackTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, attackTarget);
            kasfShipsSetMilitaryParade(attackTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)60047571);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_StandardDestroyerCombatRetryDelay);
    }
}

void Watch_Mission17_StandardDestroyerCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasfEntrySetValue(337787, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)60047571);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_StandardDestroyerCombatRetryDelay);
    }
}

void Begin_Mission17_IonCannonFrigateCombatLoop(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108434696);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)0x0676ED73);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_IonCannonFrigateCombatRetryDelay);
    }
}

void Begin_Mission17_ReinforcementAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *launchTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    launchTargets = kasGrowSelectionPtr((char *)0x6DA2DA9);
    if (!kasfShipsSelectNearby(attackTargets, launchTargets, kasShipsVectorPtr((char *)0x4E20), 20000))
    {
        kasfShipsAdd(attackTargets, launchTargets);
    }
    kasfShipsSortByDistanceToPoint(attackTargets, kasShipsVectorPtr((char *)0x4E20));
    kasfFormationWall();
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x764E81C), attackTargets))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x764E81C));
    }
    else
    {
        tutDeallocateRootRegion(attackTargets);
    }
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_IonCannonFrigateCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_IonCannonFrigateCombatRetryDelay);
    }
}

void Watch_Mission17_MultigunCorvetteCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
            (sdword)Watch_Mission17_MultigunCorvetteCombatLoop);
    }
}

void Begin_Mission17_HeavyCruiserCombatLoop(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108429832);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)185462083);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_HeavyCruiserCombatAttackRun,
            (sdword)Watch_Mission17_HeavyCruiserCombatRetryDelay);
    }
}

void Begin_Mission17_HeavyCruiserCombatAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *nearbyTargets;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x0AFF3FBE);
    kasfGrowSelectionClear(attackTargets);
    kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x004A2A29));
    kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)140367886));
    if (kasfEntryValueGet(0x8965A602))
    {
        if (kasfShipsSelectNotSalvagers(
                kasGrowSelectionPtr((char *)0x05506DA2),
                kasGrowSelectionPtr((char *)0x004A2A29)))
        {
            kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)0x05506DA2));
        }
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x469684));
    }

    nearbyTargets = kasGrowSelectionPtr((char *)0x036980EB);
    if (!kasfShipsSelectNearby(
            nearbyTargets,
            attackTargets,
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        kasfShipsAdd(nearbyTargets, attackTargets);
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x0857BCE3)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr((char *)0x0857BCE3),
            kasGrowSelectionPtr((char *)0x0857BCE3),
            nearbyTargets))
    {
        kasfShipsAdd(nearbyTargets, kasGrowSelectionPtr((char *)0x0857BCE3));
    }

    currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    kasfShipsSortByDistanceToPoint(nearbyTargets, currentPosition);
    kasfFormationWall();
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x0764E81C), nearbyTargets))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x0764E81C));
    }
    else
    {
        tutDeallocateRootRegion(nearbyTargets);
    }
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_HeavyCruiserCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_HeavyCruiserCombatAttackRun,
            (sdword)Watch_Mission17_HeavyCruiserCombatRetryDelay);
    }
}

void Begin_Mission17_CarrierCombatLoop(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108429832);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)103051523);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_CarrierCombatAttackRun,
            (sdword)Watch_Mission17_CarrierCombatLoop);
    }
}

void Begin_Mission17_CarrierCombatAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *nearbyTargets;
    hvector *movePoint;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)184500158);
    kasfGrowSelectionClear(attackTargets);
    kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x004A2A29));
    kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)140367886));
    if (kasfEntryValueGet(0x8965A602))
    {
        if (kasfShipsSelectNotSalvagers(
                kasGrowSelectionPtr((char *)0x05506DA2),
                kasGrowSelectionPtr((char *)0x004A2A29)))
        {
            kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)0x05506DA2));
        }
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x469684));
    }

    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)57245931),
            attackTargets,
            kasShipsVectorPtr((char *)0xEA60),
            60000))
    {
        nearbyTargets = kasGrowSelectionPtr((char *)86289039);
        if (!kasfFindShipsTargetingSelection(nearbyTargets, kasGrowSelectionPtr((char *)140367886)))
        {
            kasfShipsAdd(nearbyTargets, kasAITeamShipsPtrIfExists(96166939));
        }
        kasfShipsSortByDistanceToPoint(
            nearbyTargets,
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        if (kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), nearbyTargets, 0))
        {
            kasfLabelledVectorSet(42189487, 0, 0, 0);
            movePoint = kasVectorPtrIfExists((char *)42189487);
            kasfVectorRandomizeInRange(movePoint, kasVectorPtrIfExists((char *)21541), 30000);
            kasfMoveTo(kasVectorPtrIfExists((char *)42189487));
        }
    }
    kasTimerCreateSetStartById(337061, kasfRandom(120, 160));
}

void Watch_Mission17_CarrierCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)184500158);
    kasfGrowSelectionClear(attackTargets);
    kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x004A2A29));
    kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)140367886));
    if (kasfEntryValueGet(0x8965A602))
    {
        if (kasfShipsSelectNotSalvagers(
                kasGrowSelectionPtr((char *)0x05506DA2),
                kasGrowSelectionPtr((char *)0x004A2A29)))
        {
            kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)0x05506DA2));
        }
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x469684));
    }
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)57245931),
            attackTargets,
            kasShipsVectorPtr((char *)0x4E20),
            20000) &&
        kasfUnderAttack(kasGrowSelectionPtr((char *)139967715)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)57245931)))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanReinforcementAttackRun,
            (sdword)Watch_Mission17_CarrierCombatRetryDelay);
    }
    else if (kasTimerExpiredById(337061) && kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_CarrierCombatAttackRun,
            (sdword)Watch_Mission17_CarrierCombatLoop);
    }
}

void Watch_Mission17_CarrierCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_CarrierCombatAttackRun,
            (sdword)Watch_Mission17_CarrierCombatLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanReinforcementAttackRun,
            (sdword)Watch_Mission17_CarrierCombatRetryDelay);
    }
}

void Watch_Mission17_HeavyCruiserCombatTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 1)
    {
        buildCost = kasfShipTypeBuildCost("tHeavyCruiser");
        if ((sdword)kasfRUsGet(3) >= buildCost)
        {
            kasfRUsSet(3, kasfRUsGet(3) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tHeavyCruiser")) ||
        kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_HeavyCruiserCombatTeamBuildTimer,
            (sdword)Watch_Mission17_HeavyCruiserCombatTeamBuildTimer);
    }
}

void Begin_Mission17_HeavyCruiserCombatTeamBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    kasTimerCreateSetStartById(
        235522949,
        kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tHeavyCruiser"));
}

void Watch_Mission17_HeavyCruiserCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        return;
    }
    if (!kasTimerExpiredById(235522949))
    {
        return;
    }

    kasfStop();
    createdShips = kasGrowSelectionPtr((char *)154639956);
    kasfShipsCreate(createdShips, "tHeavyCruiser", kasfEntryValueGet(154639956), 0);
    kasAITeamShipsPtr(createdShips, 3);
    kasfShipsSetSupportFlag(createdShips, 0);
    kasfEntrySetValue(154639956, 0);
    kasfReinforceTeamWithShips(team, createdShips);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_HeavyCruiserCombatTeamAttackStaging);
}

void Begin_Mission17_HeavyCruiserCombatTeamAttackStaging(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108434696);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)185462083);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_HeavyCruiserCombatTeamAttackStaging);
    }
}

void Watch_Mission17_HeavyCruiserCombatTeamAttackStaging(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_HeavyCruiserCombatTeamAttackStaging);
    }
}






void Init_Mission17_HeavyInterceptorCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamSwitchPlayerOwner(0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(182180946)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x8D459D62, 0);
    kasfEntryGetOrCreateSetValue(0x8D6BD7B5, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_HeavyInterceptorCombatTeam);
        return;
    }

    kasAITeamShipsPtr(&team->shipList, 3);
    kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)233775571);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
        (sdword)Watch_Mission17_HeavyInterceptorCombatLoop);
}

void Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(182180946)) &&
        kasfEntryValueGet(0x8D6BD7B5) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x8D6BD7B5, 0);
        kasfEntrySetValue(0x8D459D62, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)154145491);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_HeavyInterceptorCombatTeam);
        return;
    }
    if (kasfShipsCount(kasGrowSelectionPtr((char *)108434696)) &&
        kasfTeamHealthAverage() < 50)
    {
        kasfSpeechEvent(
            299899,
            (sdword)Init_Mission17_HeavyInterceptorDockAndReinforce,
            (sdword)Watch_Mission17_HeavyInterceptorDockAndReinforce);
    }
}

void Watch_Mission17_HeavyInterceptorCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(0x8D459D62, 0);
        kasfEntrySetValue(0x8D6BD7B5, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x8D459D62))
    {
        kasfEntrySetValue(0x8D459D62, 0);
        kasfEntrySetValue(0x8D6BD7B5, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_HeavyInterceptorCombatTeamBuildCount);
    }
}

void Watch_Mission17_HeavyInterceptorCombatTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 12)
    {
        buildCost = kasfShipTypeBuildCost("tHeavyInterceptor");
        if ((sdword)kasfRUsGet(3) >= buildCost)
        {
            kasfRUsSet(3, kasfRUsGet(3) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tHeavyInterceptor")) ||
        kasfEntryValueGet(154639956) == 12)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_HeavyInterceptorCombatTeamBuildTimer,
            (sdword)Watch_Mission17_HeavyInterceptorCombatTeamBuildTimer);
    }
}

void Begin_Mission17_HeavyInterceptorCombatTeamBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    kasTimerCreateSetStartById(
        235522949,
        kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tHeavyInterceptor"));
}

void Watch_Mission17_HeavyInterceptorCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasGrowSelectionPtr((char *)108434696);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex);
        createdShips = kasGrowSelectionPtr((char *)154639956);
        if (kasfRandom(1, 4) == 1)
        {
            kasfShipsCreateInside(
                createdShips,
                "tAttackBomber",
                kasfEntryValueGet(154639956),
                spawnTarget,
                0);
        }
        else
        {
            kasfShipsCreateInside(
                createdShips,
                "tHeavyInterceptor",
                kasfEntryValueGet(154639956),
                spawnTarget,
                0);
        }
        kasAITeamShipsPtr(createdShips, 3);
        kasfShipsSetSupportFlag(createdShips, 0);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)233775571);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
            (sdword)Watch_Mission17_HeavyInterceptorCombatLoop);
    }
}

void Begin_Mission17_HeavyInterceptorCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *launchTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    launchTargets = kasGrowSelectionPtr((char *)0x6DA2DA9);
    if (!kasfShipsSelectNearby(attackTargets, launchTargets, kasShipsVectorPtr((char *)0x4E20), 20000))
    {
        kasfShipsAdd(attackTargets, launchTargets);
    }
    kasfShipsSortByDistanceToPoint(attackTargets, kasShipsVectorPtr((char *)0x4E20));
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    tutDeallocateRootRegion(attackTargets);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_HeavyInterceptorCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_HeavyInterceptorCombatLoop,
            (sdword)Watch_Mission17_HeavyInterceptorCombatLoop);
    }
}

void Init_Mission17_HeavyInterceptorDockAndReinforce(void)
{
    AITeam *team;
    hvector *currentPosition;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    launchTargets = kasGrowSelectionPtr((char *)108434696);
    kasfShipsSortByDistanceToPoint(launchTargets, currentPosition);
    spawnTarget = kasGrowSelectionPtr((char *)21541);
    kasfShipsSelectIndex(spawnTarget, launchTargets, 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    kasfShipsDockWithSelection(&team->shipList, spawnTarget);
}

void Watch_Mission17_HeavyInterceptorDockAndReinforce(void)
{
    AITeam *team;
    sdword missingShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_HeavyInterceptorCombatTeam);
        return;
    }

    missingShips = 12 - kasfTeamCount();
    if (kasfEntryValueGet(154639956) < missingShips &&
        (sdword)kasfRUsGet(3) >= kasfShipTypeBuildCost("tHeavyInterceptor"))
    {
        kasfRUsSet(3, kasfRUsGet(3) - kasfShipTypeBuildCost("kHeavyInterceptor"));
        kasfEntryIncrement(154639956);
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tHeavyInterceptor")) ||
        kasfEntryValueGet(154639956) == 12)
    {
        kasTimerCreateSetStartById(
            133682484,
            kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tHeavyInterceptor"));
    }
    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        kasfShipsSelectLinkedWith(
            &team->shipList,
            kasGrowSelectionPtr((char *)316548),
            kasGrowSelectionPtr((char *)108434696));
    }
}

void Init_Mission17_AdvanceSupportFrigateCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamSwitchPlayerOwner(0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(122695973)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x835D48C5, 0);
    kasfEntryGetOrCreateSetValue(0x88CAEF65, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_AdvanceSupportFrigateCombatTeam);
        return;
    }

    kasAITeamShipsPtr(&team->shipList, 3);
    kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)188946227);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        313692,
        (sdword)Begin_Mission17_AdvanceSupportFrigateRepairOrder,
        (sdword)Watch_Mission17_AdvanceSupportFrigateRepairOrder);
}

void Init_Mission17_AdvanceSupportFrigateCombatTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(122695973)) &&
        kasfEntryValueGet(0x88CAEF65) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x88CAEF65, 0);
        kasfEntrySetValue(0x835D48C5, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)135485987);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_AdvanceSupportFrigateCombatTeam);
    }
}

void Watch_Mission17_AdvanceSupportFrigateCombatTeam(void)
{
    AITeam *team;
    GrowSelection *supportTeamShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    supportTeamShips = kasAITeamShipsPtrIfExists(122695973);
    if (kasfShipsCount(supportTeamShips) >= 1)
    {
        kasfEntrySetValue(332667, 0);
        kasfReinforceTeamWithShips(team, supportTeamShips);
        kasfShipsRemove(supportTeamShips, &team->shipList);
        kasfTacticsNeutral();
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_AdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_AdvanceSupportFrigateRepairOrder);
        return;
    }
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(0x835D48C5, 0);
        kasfEntrySetValue(0x88CAEF65, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x835D48C5))
    {
        kasfEntrySetValue(0x835D48C5, 0);
        kasfEntrySetValue(0x88CAEF65, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_AdvanceSupportFrigateCombatTeamBuildCount);
    }
}

void Watch_Mission17_AdvanceSupportFrigateCombatTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 3)
    {
        buildCost = kasfShipTypeBuildCost("tAdvanceSupportFrigate");
        if ((sdword)kasfRUsGet(3) >= buildCost)
        {
            kasfRUsSet(3, kasfRUsGet(3) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tAdvanceSupportFrigate")) ||
        kasfEntryValueGet(154639956) == 3)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer,
            (sdword)Watch_Mission17_AdvanceSupportFrigateCombatTeamBuildTimer);
    }
}

void Watch_Mission17_AdvanceSupportFrigateCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasGrowSelectionPtr((char *)108434696);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex);
        createdShips = kasGrowSelectionPtr((char *)154639956);
        kasfShipsCreateInside(
            createdShips,
            "tAdvanceSupportFrigate",
            kasfEntryValueGet(154639956),
            spawnTarget,
            0);
        kasAITeamShipsPtr(createdShips, 3);
        kasfShipsSetSupportFlag(createdShips, 0);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)188946227);
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_AdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_AdvanceSupportFrigateRepairOrder);
    }
}

void Begin_Mission17_AdvanceSupportFrigateDefenseLoop(void)
{
    AITeam *team;
    GrowSelection *workingSelection;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfFindShipsTargetingSelection(
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)&loc_4A2A29)))
    {
        currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)139967715), currentPosition);
        workingSelection = kasGrowSelectionPtr((char *)139967715);
    }
    else if (kasfShipsSelectCapital(
                 kasGrowSelectionPtr((char *)124053532),
                 kasGrowSelectionPtr((char *)&loc_4A2A29)) &&
             kasfShipsRemove(kasGrowSelectionPtr((char *)124053532), &team->shipList) &&
             kasfShipsRemove(
                 kasGrowSelectionPtr((char *)124053532),
                 kasGrowSelectionPtr((char *)108434696)))
    {
        currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)124053532), currentPosition);
        workingSelection = kasGrowSelectionPtr((char *)124053532);
    }
    else
    {
        currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)&loc_4A2A29), currentPosition);
        workingSelection = kasGrowSelectionPtr((char *)&loc_4A2A29);
    }

    if (kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), workingSelection, 0))
    {
        kasfFormationBroad();
        kasfMoveTo(kasShipsVectorPtrIfExists((char *)21541));
    }
    kasTimerCreateSetStartById(337061, kasfRandom(30, 60));
}

void Watch_Mission17_AdvanceSupportFrigateDefenseLoop(void)
{
    AITeam *team;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    if ((kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)&loc_4A2A29),
             currentPosition,
             20000) &&
         kasfShipsSelectCapital(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228)) &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             90)) ||
        (kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)&loc_4A2A29),
             currentPosition,
             20000) &&
         kasfShipsSelectType(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             "tAdvanceSupportFrigate") &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             90)))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_AdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_AdvanceSupportFrigateRepairOrder);
    }
    else if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(337061))
    {
        kasTimerDestroyById(337061);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_AdvanceSupportFrigateDefenseLoop,
            (sdword)Watch_Mission17_AdvanceSupportFrigateDefenseLoop);
    }
}

void Begin_Mission17_AdvanceSupportFrigateRepairOrder(void)
{
    AITeam *team;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    if ((kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)&loc_4A2A29),
             currentPosition,
             20000) &&
         kasfShipsSelectCapital(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228)) &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             90)) ||
        (kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)&loc_4A2A29),
             currentPosition,
             20000) &&
         kasfShipsSelectType(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             "tAdvanceSupportFrigate") &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             90)))
    {
        currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)140598228), currentPosition);
        kasfFormationBroad();
        kasfTeamSpecial(kasGrowSelectionPtr((char *)140598228));
    }
    else
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_AdvanceSupportFrigateDefenseLoop,
            (sdword)Watch_Mission17_AdvanceSupportFrigateDefenseLoop);
    }
}

void Watch_Mission17_AdvanceSupportFrigateRepairOrder(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectSpecialTargets(kasGrowSelectionPtr((char *)140598228), &team->shipList) ||
        kasfShipsSelectHealthAtLeastPercent(
            kasGrowSelectionPtr((char *)140598228),
            kasGrowSelectionPtr((char *)140598228),
            100))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_AdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_AdvanceSupportFrigateRepairOrder);
    }
}

void Init_Mission17_StandardDestroyerCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamSwitchPlayerOwner(0);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(100221730)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x86D0D962, 0);
    kasfEntryGetOrCreateSetValue(0x896064F5, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    kasAITeamShipsPtr(&team->shipList, 3);
    kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)60047571);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_ReinforcementAttackRun,
        (sdword)Watch_Mission17_StandardDestroyerCombatRetryDelay);
}

void Init_Mission17_StandardDestroyerCombatTeam_Reevaluate(void)
{
    AITeam *team;
    sdword **queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(100221730)) &&
        kasfEntryValueGet(0x896064F5) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x896064F5, 0);
        kasfEntrySetValue(0x86D0D962, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)110149603);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    queueInfo = *((sdword ***)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = **queueInfo;
    }
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)126523733)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                kasfVarDestroy(&team->shipList);
            }
            else if (message == (void *)237188773)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }

        queueInfo = *((sdword ***)team + 20);
        if (queueInfo == NULL || **queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Watch_Mission17_StandardDestroyerCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(0x86D0D962, 0);
        kasfEntrySetValue(0x896064F5, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x86D0D962))
    {
        kasfEntrySetValue(0x86D0D962, 0);
        kasfEntrySetValue(0x896064F5, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_StandardDestroyerCombatTeamBuildCount);
    }
}

void Watch_Mission17_StandardDestroyerCombatTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 1)
    {
        buildCost = kasfShipTypeBuildCost("tStandardDestroyer");
        if ((sdword)kasfRUsGet(3) >= buildCost)
        {
            kasfRUsSet(3, kasfRUsGet(3) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(3) < kasfShipTypeBuildCost("tStandardDestroyer")) ||
        kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_StandardDestroyerCombatTeamBuildTimer,
            (sdword)Watch_Mission17_StandardDestroyerCombatTeamBuildTimer);
    }
}

void Begin_Mission17_StandardDestroyerCombatTeamBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    kasTimerCreateSetStartById(
        235522949,
        kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tStandardDestroyer"));
}

void Watch_Mission17_StandardDestroyerCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108434696)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        createdShips = kasGrowSelectionPtr((char *)154639956);
        kasfShipsCreate(createdShips, "tStandardDestroyer", kasfEntryValueGet(154639956), 0);
        kasAITeamShipsPtr(createdShips, 3);
        kasfShipsSetSupportFlag(createdShips, 0);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsSetSlaveableFlag(&team->shipList, 0);
        kasfSpeechEvent(
            175938357,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Begin_Mission17_StandardDestroyerCombatLoop);
    }
}

void Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(121712933)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x835448C5, 0);
    kasfEntryGetOrCreateSetValue(0x88CAE865, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeam);
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)188946227);
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfSpeechEvent(
        313692,
        (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder,
        (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder);
}

void Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(121712933)) &&
        kasfEntryValueGet(0x88CAE865) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x88CAE865, 0);
        kasfEntrySetValue(0x835448C5, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)135485987);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeam);
    }
}

void Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeam(void)
{
    AITeam *team;
    GrowSelection *supportTeamShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    supportTeamShips = kasAITeamShipsPtrIfExists(121712933);
    if (kasfShipsCount(supportTeamShips) >= 3)
    {
        kasfEntrySetValue(332667, 0);
        kasfReinforceTeamWithShips(team, supportTeamShips);
        kasfShipsRemove(supportTeamShips, &team->shipList);
        kasfTacticsNeutral();
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder);
        return;
    }
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(0x835448C5, 0);
        kasfEntrySetValue(0x88CAE865, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x835448C5))
    {
        kasfEntrySetValue(0x835448C5, 0);
        kasfEntrySetValue(0x88CAE865, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildCount);
    }
}

void Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 5)
    {
        buildCost = kasfShipTypeBuildCost("tAdvanceSupportFrigate");
        if ((sdword)kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 3 &&
         (sdword)kasfRUsGet(2) < kasfShipTypeBuildCost("tAdvanceSupportFrigate")) ||
        kasfEntryValueGet(154639956) == 5)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer);
    }
}

void Begin_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    kasTimerCreateSetStartById(
        235522949,
        kasfShipTypeBuildTime("tAdvanceSupportFrigate") * kasfEntryValueGet(154639956));
}

void Watch_Mission17_TaiidanAdvanceSupportFrigateSupportTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasGrowSelectionPtr((char *)108429832);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex);
        createdShips = kasGrowSelectionPtr((char *)154639956);
        kasfShipsCreateInside(
            createdShips,
            "tAdvanceSupportFrigate",
            kasfEntryValueGet(154639956),
            spawnTarget,
            0);
        kasAITeamShipsPtr(createdShips, 2);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), createdShips);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)188946227);
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder);
    }
}

void Begin_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop(void)
{
    AITeam *team;
    GrowSelection *targets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfFindShipsTargetingSelection(
            kasGrowSelectionPtr((char *)139967715),
            kasGrowSelectionPtr((char *)140367886)))
    {
        kasfShipsSortByDistanceToPoint(
            kasGrowSelectionPtr((char *)139967715),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        targets = kasGrowSelectionPtr((char *)139967715);
        if (kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), targets, 0))
        {
            kasfFormationBroad();
            kasfMoveTo(kasShipsVectorPtrIfExists((char *)21541));
        }
    }
    kasTimerCreateSetStartById(337061, kasfRandom(30, 60));
}

void Watch_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop(void)
{
    AITeam *team;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    currentPosition = kasShipsVectorPtr((char *)0x4E20);
    if ((kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140367886),
             currentPosition,
             20000) &&
         kasfShipsSelectCapital(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228)) &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             70)) ||
        (kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140367886),
             currentPosition,
             20000) &&
         kasfShipsSelectType(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             "tAdvanceSupportFrigate") &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             70)))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder);
    }
    else if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(337061))
    {
        kasTimerDestroyById(337061);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop);
    }
}

void Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder(void)
{
    AITeam *team;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    currentPosition = kasShipsVectorPtr((char *)0x4E20);
    if ((kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140367886),
             currentPosition,
             20000) &&
         kasfShipsSelectCapital(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228)) &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             70)) ||
        (kasfShipsSelectNearby(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140367886),
             currentPosition,
             20000) &&
         kasfShipsSelectType(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             "tAdvanceSupportFrigate") &&
         kasfShipsSelectDamaged(
             kasGrowSelectionPtr((char *)140598228),
             kasGrowSelectionPtr((char *)140598228),
             70)))
    {
        kasfShipsSortByDistanceToPoint(
            kasGrowSelectionPtr((char *)140598228),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        kasfFormationBroad();
        kasfTeamSpecial(kasGrowSelectionPtr((char *)140598228));
    }
    else
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateDefenseLoop);
    }
}

void Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectSpecialTargets(kasGrowSelectionPtr((char *)140598228), &team->shipList) ||
        kasfShipsSelectHealthAtLeastPercent(
            kasGrowSelectionPtr((char *)140598228),
            kasGrowSelectionPtr((char *)140598228),
            100))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Begin_Mission17_TaiidanAdvanceSupportFrigateRepairOrder,
            (sdword)Watch_Mission17_TaiidanAdvanceSupportFrigateRepairOrder);
    }
}

void Init_Mission17_TaiidanStandardDestroyerCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(90784546)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x8460D962, 0);
    kasfEntryGetOrCreateSetValue(0x8962F4F5, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanStandardDestroyerCombatTeam);
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)60047571);
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfShipsSetMaxVelocity(&team->shipList, 190);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_TaiidanStandardDestroyerCombatLoop);
}

void Init_Mission17_TaiidanStandardDestroyerCombatTeam_Reevaluate(void)
{
    AITeam *team;
    sdword **queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(90784546)) &&
        kasfEntryValueGet(0x8962F4F5) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x8962F4F5, 0);
        kasfEntrySetValue(0x8460D962, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)110149603);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanStandardDestroyerCombatTeam);
        return;
    }

    queueInfo = *((sdword ***)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = **queueInfo;
    }
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)126523733)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                kasfVarDestroy(&team->shipList);
            }
            else if (message == (void *)237188773)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }
        queueInfo = *((sdword ***)team + 20);
        if (queueInfo == NULL || **queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Watch_Mission17_TaiidanStandardDestroyerCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(0x8460D962, 0);
        kasfEntrySetValue(0x8962F4F5, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x8460D962))
    {
        kasfEntrySetValue(0x8460D962, 0);
        kasfEntrySetValue(0x8962F4F5, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_TaiidanStandardDestroyerCombatTeamBuildCount);
    }
}

void Watch_Mission17_TaiidanStandardDestroyerCombatTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 1)
    {
        buildCost = kasfShipTypeBuildCost("tStandardDestroyer");
        if ((sdword)kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(2) < kasfShipTypeBuildCost("tStandardDestroyer")) ||
        kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_StandardDestroyerCombatTeamBuildTimer,
            (sdword)Watch_Mission17_TaiidanStandardDestroyerCombatTeamBuildTimer);
    }
}

void Watch_Mission17_TaiidanStandardDestroyerCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (!kasTimerExpiredById(235522949))
    {
        return;
    }

    kasfStop();
    createdShips = kasGrowSelectionPtr((char *)154639956);
    kasfShipsCreate(createdShips, "tStandardDestroyer", kasfEntryValueGet(154639956), 0);
    kasAITeamShipsPtr(createdShips, 2);
    kasfEntrySetValue(154639956, 0);
    kasfReinforceTeamWithShips(team, createdShips);
    kasfShipsSetMaxVelocity(&team->shipList, 190);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), createdShips);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_TaiidanStandardDestroyerCombatLoop);
}

void Begin_Mission17_TaiidanStandardDestroyerCombatLoop(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108429832);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)60047571);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanReinforcementAttackRun,
            (sdword)Watch_Mission17_TaiidanStandardDestroyerCombatRetryDelay);
    }
}

void Begin_Mission17_TaiidanReinforcementAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *nearbyTargets;
    hvector *currentPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)184500158);
    kasfGrowSelectionClear(attackTargets);
    kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)&loc_4A2A29));
    kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)140367886));
    if (kasfEntryValueGet(0x8965A602))
    {
        if (kasfShipsSelectNotSalvagers(
                kasGrowSelectionPtr((char *)0x05506DA2),
                kasGrowSelectionPtr((char *)&loc_4A2A29)))
        {
            kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)0x05506DA2));
        }
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x469684));
    }

    nearbyTargets = kasGrowSelectionPtr((char *)0x036980EB);
    if (!kasfShipsSelectNearby(
            nearbyTargets,
            attackTargets,
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        kasfShipsAdd(nearbyTargets, attackTargets);
    }
    kasfShipsRemove(nearbyTargets, kasGrowSelectionPtr((char *)140367886));
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x0857BCE3)) &&
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr((char *)0x0857BCE3),
            kasGrowSelectionPtr((char *)0x0857BCE3),
            nearbyTargets))
    {
        kasfShipsAdd(nearbyTargets, kasGrowSelectionPtr((char *)0x0857BCE3));
    }

    currentPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    kasfShipsSortByDistanceToPoint(nearbyTargets, currentPosition);
    kasfFormationWall();
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x0764E81C), nearbyTargets))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x0764E81C));
    }
    else
    {
        tutDeallocateRootRegion(nearbyTargets);
    }
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_TaiidanStandardDestroyerCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanReinforcementAttackRun,
            (sdword)Watch_Mission17_TaiidanStandardDestroyerCombatRetryDelay);
    }
}

void Init_Mission17_TaiidanIonCannonFrigateCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(209088133)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x88471F65, 0);
    kasfEntryGetOrCreateSetValue(0x8F61D535, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanIonCannonFrigateCombatTeam);
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)108457331);
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(332667, 0);
    kasfShipsSetMaxVelocity(&team->shipList, 190);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_TaiidanIonCannonFrigateCombatLoop);
}

void Init_Mission17_TaiidanIonCannonFrigateCombatTeam_Reevaluate(void)
{
    AITeam *team;
    sdword **queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(209088133)) &&
        kasfEntryValueGet(0x8F61D535) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x8F61D535, 0);
        kasfEntrySetValue(0x88471F65, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)191275187);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanIonCannonFrigateCombatTeam);
        return;
    }

    queueInfo = *((sdword ***)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = **queueInfo;
    }
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)126523733)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
                kasfVarDestroy(&team->shipList);
            }
            else if (message == (void *)237188773)
            {
                kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
            }
            else
            {
                aitQueuePointer(team, team, message);
            }
        }
        queueInfo = *((sdword ***)team + 20);
        if (queueInfo == NULL || **queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Watch_Mission17_TaiidanIonCannonFrigateCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(0x88471F65, 0);
        kasfEntrySetValue(0x8F61D535, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x88471F65))
    {
        kasfEntrySetValue(0x88471F65, 0);
        kasfEntrySetValue(0x8F61D535, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_TaiidanIonCannonFrigateCombatTeamBuildCount);
    }
}

void Watch_Mission17_TaiidanIonCannonFrigateCombatTeamBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 1)
    {
        buildCost = kasfShipTypeBuildCost("tIonCannonFrigate");
        if ((sdword)kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(2) < kasfShipTypeBuildCost("tIonCannonFrigate")) ||
        kasfEntryValueGet(154639956) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_IonCannonFrigateReinforcementBuildTimer,
            (sdword)Watch_Mission17_TaiidanIonCannonFrigateCombatTeamBuildTimer);
    }
}

void Watch_Mission17_TaiidanIonCannonFrigateCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (!kasTimerExpiredById(235522949))
    {
        return;
    }

    kasfStop();
    createdShips = kasGrowSelectionPtr((char *)154639956);
    kasfShipsCreate(createdShips, "tIonCannonFrigate", kasfEntryValueGet(154639956), 0);
    kasAITeamShipsPtr(createdShips, 2);
    kasfEntrySetValue(154639956, 0);
    kasfReinforceTeamWithShips(team, createdShips);
    kasfShipsSetMaxVelocity(&team->shipList, 190);
    kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), createdShips);
    kasfShipsSetSlaveableFlag(&team->shipList, 0);
    kasfSpeechEvent(
        175938357,
        (sdword)ResetReinforcementHyperspaceAttackFlag,
        (sdword)Begin_Mission17_TaiidanIonCannonFrigateCombatLoop);
}

void Begin_Mission17_TaiidanIonCannonFrigateCombatLoop(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)108429832);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)118933374);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)108457331);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanReinforcementAttackRun,
            (sdword)Watch_Mission17_TaiidanIonCannonFrigateCombatRetryDelay);
    }
}

void Watch_Mission17_TaiidanIonCannonFrigateCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanReinforcementAttackRun,
            (sdword)Watch_Mission17_TaiidanIonCannonFrigateCombatRetryDelay);
    }
}

void Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(75165957)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(0x8D9EE295, 0);
        kasfEntryGetOrCreateSetValue(0x82942EC5, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)41951459);
            kasAITeamShipsPtr(&team->shipList, 2);
            kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(
                5019236,
                (sdword)Begin_Mission17_TaiidanMultigunCorvetteCombatLoop,
                (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatLoop);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_TaiidanMultigunCorvetteReinforcementTeam);
        }
    }
}

void Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(75165957)) &&
        kasfEntryValueGet(0x82942EC5) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x82942EC5, 0);
        kasfEntrySetValue(0x8D9EE295, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)259199139);
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteReinforcementTeam);
        return;
    }
    if (kasfShipsCount(kasGrowSelectionPtr((char *)108429832)) &&
        kasfTeamHealthAverage() < 50)
    {
        kasfSpeechEvent(
            299899,
            (sdword)Begin_Mission17_TaiidanMultigunCorvetteDockAndReinforce,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteDockAndReinforce);
    }
}

void Watch_Mission17_TaiidanMultigunCorvetteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)108429832)))
    {
        kasfEntrySetValue(0x8D9EE295, 0);
        kasfEntrySetValue(0x82942EC5, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x8D9EE295))
    {
        kasfEntrySetValue(0x8D9EE295, 0);
        kasfEntrySetValue(0x82942EC5, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteReinforcementBuildCount);
    }
}

void Watch_Mission17_TaiidanMultigunCorvetteReinforcementBuildCount(void)
{
    AITeam *team;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(154639956) < 10)
    {
        buildCost = kasfShipTypeBuildCost("tMultiGunCorvette");
        if ((sdword)kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(2) < kasfShipTypeBuildCost("tMultiGunCorvette")) ||
        kasfEntryValueGet(154639956) == 10)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_MultigunCorvetteReinforcementBuildTimer,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteReinforcementBuildTimer);
    }
}

void Watch_Mission17_TaiidanMultigunCorvetteReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasGrowSelectionPtr((char *)108429832);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex);
        createdShips = kasGrowSelectionPtr((char *)154639956);
        kasfShipsCreateInside(
            createdShips,
            "tMultiGunCorvette",
            kasfEntryValueGet(154639956),
            spawnTarget,
            1);
        kasAITeamShipsPtr(createdShips, 2);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr((char *)140367886), &team->shipList);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(140367886), (char *)41951459);
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_TaiidanMultigunCorvetteCombatLoop,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatLoop);
    }
}

void Begin_Mission17_TaiidanMultigunCorvetteCombatLoop(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;
    GrowSelection *nearbyEnemies;
    GrowSelection *priorityTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    capitalTargets = kasGrowSelectionPtr((char *)0x764E81C);
    if (kasfShipsSelectNearby(
            capitalTargets,
            kasGrowSelectionPtr((char *)0x85DD80E),
            kasShipsVectorPtr((char *)0x4E20),
            20000) &&
        kasfShipsSelectCapital(capitalTargets, capitalTargets))
    {
        if (kasfShipsSelectClass(
                kasGrowSelectionPtr((char *)0x0ADBD8E2),
                kasGrowSelectionPtr((char *)0x85DD80E),
                "CLASS_Corvette") &&
            kasfShipsSelectTargets(
                kasGrowSelectionPtr((char *)0x09668AE7),
                kasGrowSelectionPtr((char *)0x0ADBD8E2)))
        {
            kasfShipsRemove(capitalTargets, kasGrowSelectionPtr((char *)0x09668AE7));
        }
        if (!kasfShipsCount(capitalTargets))
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Begin_Mission17_TaiidanMultigunCorvetteAttackRun,
                (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatRetryDelay);
            return;
        }
    }

    nearbyEnemies = kasGrowSelectionPtr((char *)0x5296679);
    kasfShipsSelectNearby(
        nearbyEnemies,
        capitalTargets,
        kasShipsVectorPtr((char *)0x4E20),
        20000);
    kasfShipsSelectType(
        kasGrowSelectionPtr((char *)0x0CB08C72),
        nearbyEnemies,
        "tHeavyCruiser");
    kasfShipsSelectType(
        kasGrowSelectionPtr((char *)0x07676DE2),
        nearbyEnemies,
        "tCarrier");
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0x0CB08C72)) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)0x07676DE2)))
    {
        kasfShipsSortByDistanceToPoint(capitalTargets, ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x093C9634), capitalTargets, 0);
    }
    else
    {
        priorityTargets = kasGrowSelectionPtr((char *)0x077A2AE4);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)0x0CB08C72));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)0x07676DE2));
        kasfShipsSortByDistanceToPoint(priorityTargets, ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x093C9634), priorityTargets, 0);
    }
    kasfFormationSphere();
    kasfProtect(kasGrowSelectionPtr((char *)0x093C9634));
}

void Watch_Mission17_TaiidanMultigunCorvetteCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!(kasfShipsOrderAttributes(&team->shipList) & 2))
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_TaiidanMultigunCorvetteCombatLoop,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatLoop);
    }
}

void Begin_Mission17_TaiidanMultigunCorvetteAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *nearbyTargets;
    GrowSelection *capitalTargets;
    GrowSelection *priorityTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackTargets = kasGrowSelectionPtr((char *)0x0AFF3FBE);
    kasfGrowSelectionClear(attackTargets);
    kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x004A2A29));
    kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)0x85DD80E));
    if (kasfEntryValueGet(0x8965A602))
    {
        if (kasfShipsSelectNotSalvagers(
                kasGrowSelectionPtr((char *)0x05506DA2),
                kasGrowSelectionPtr((char *)0x004A2A29)))
        {
            kasfShipsRemove(attackTargets, kasGrowSelectionPtr((char *)0x05506DA2));
        }
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr((char *)0x469684));
    }
    nearbyTargets = kasGrowSelectionPtr((char *)0x5296679);
    if (!kasfShipsSelectNearby(
            nearbyTargets,
            attackTargets,
            kasShipsVectorPtr((char *)0x4E20),
            20000))
    {
        capitalTargets = kasGrowSelectionPtr((char *)0x764E81C);
        if (kasfShipsSelectNearby(
                capitalTargets,
                kasGrowSelectionPtr((char *)0x85DD80E),
                kasShipsVectorPtr((char *)0x4E20),
                20000) &&
            kasfShipsSelectCapital(capitalTargets, capitalTargets))
        {
            if (kasfShipsSelectClass(
                    kasGrowSelectionPtr((char *)0x0ADBD8E2),
                    kasGrowSelectionPtr((char *)0x85DD80E),
                    "CLASS_Corvette") &&
                kasfShipsSelectTargets(
                    kasGrowSelectionPtr((char *)0x09668AE7),
                    kasGrowSelectionPtr((char *)0x0ADBD8E2)))
            {
                kasfShipsRemove(capitalTargets, kasGrowSelectionPtr((char *)0x09668AE7));
            }
            if (kasfShipsCount(capitalTargets))
            {
                kasfSpeechEvent(
                    5019236,
                    (sdword)Begin_Mission17_TaiidanMultigunCorvetteCombatLoop,
                    (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatLoop);
                return;
            }
        }

        priorityTargets = kasGrowSelectionPtr((char *)0x36980EB);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)0x004A2A29));
        if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
        {
            kasfShipsAdd(priorityTargets, kasGrowSelectionPtr((char *)0x857BCE3));
        }
        nearbyTargets = priorityTargets;
    }

    kasfShipsSortByDistanceToPoint(nearbyTargets, ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    if (kasfTeamCount() > 8)
    {
        kasfFormationDelta3D();
    }
    else
    {
        kasfFormationDelta();
    }
    tutDeallocateRootRegion(nearbyTargets);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission17_TaiidanMultigunCorvetteCombatRetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_TaiidanMultigunCorvetteCombatLoop,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_TaiidanMultigunCorvetteAttackRun,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteCombatRetryDelay);
    }
}

void Begin_Mission17_TaiidanMultigunCorvetteDockAndReinforce(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    kasfShipsSortByDistanceToPoint(
        kasGrowSelectionPtr((char *)108429832),
        ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfShipsSelectIndex(
        kasGrowSelectionPtr((char *)21541),
        kasGrowSelectionPtr((char *)108429832),
        0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr((char *)21541));
}

void Watch_Mission17_TaiidanMultigunCorvetteDockAndReinforce(void)
{
    AITeam *team;
    sdword missingCount;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            4693508,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteReinforcementTeam);
        return;
    }

    missingCount = 10 - kasfTeamCount();
    if (kasfEntryValueGet(154639956) < missingCount)
    {
        buildCost = kasfShipTypeBuildCost("tMultiGunCorvette");
        if ((sdword)kasfRUsGet(2) >= buildCost)
        {
            kasfRUsSet(2, kasfRUsGet(2) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 &&
         (sdword)kasfRUsGet(2) < kasfShipTypeBuildCost("tMultiGunCorvette")) ||
        kasfEntryValueGet(154639956) == 10)
    {
        kasTimerCreateSetStartById(
            133682484,
            kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("tMultiGunCorvette"));
    }
    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        kasfShipsSelectLinkedWith(
            &team->shipList,
            kasGrowSelectionPtr((char *)316548),
            kasGrowSelectionPtr((char *)108429832));
    }
}

void Init_Mission17_BeastHeavyCruiserCombatTeam(void)
{
    AITeam *team;
    hvector *position;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x07A79EA4), &team->shipList);
    position = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    kasfEntryGetOrCreateSetValue(88, kasfVectorRoundX(position));
    kasfEntryGetOrCreateSetValue(89, kasfVectorRoundY(position));
    kasfEntryGetOrCreateSetValue(90, kasfVectorRoundZ(position));
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(0x07A79EA4)))
    {
        kasfEntryGetOrCreateSetValue(0x09379E54, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(0x83B4E744, 0);
        kasfEntryGetOrCreateSetValue(0x874B48C5, 1);
        kasTimerCreateById(0x0883A004);
        if (!kasfTeamCount())
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastHeavyCruiserCombatTeam);
            return;
        }
        kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x04CC48B4);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x00469684), &team->shipList);
        kasfShipsSetSlaveableFlag(&team->shipList, 0);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 0);
        kasTimerSetById(0x0883A004, 20);
        kasTimerStartById(0x0883A004);
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
    }
}

void Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x07A79EA4)) &&
        kasfEntryValueGet(0x874B48C5) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(0x83B4E744, 1);
        kasfEntrySetValue(0x874B48C5, 0);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }
    kasfLabelledVectorSet(
        (char *)0x07A79EA4,
        kasfEntryValueGet(88),
        kasfEntryValueGet(89),
        kasfEntryValueGet(90));
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x04CC48B4);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastHeavyCruiserCombatTeam);
        return;
    }
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0x0A79EA22)))
    {
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x83B4E744))
    {
        kasfEntrySetValue(0x83B4E744, 0);
        kasfEntrySetValue(0x874B48C5, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatTeamBuildCount);
    }
}

void Watch_Mission17_BeastHeavyCruiserCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)))
    {
        kasfEntrySetValue(0x83B4E744, 0);
        kasfEntrySetValue(0x874B48C5, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x83B4E744))
    {
        kasfEntrySetValue(0x83B4E744, 0);
        kasfEntrySetValue(0x874B48C5, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatTeamBuildCount);
    }
}

void Watch_Mission17_BeastHeavyCruiserCombatTeamBuildCount(void)
{
    sdword buildCost;
    sdword currentRUs;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(0x09379E54) < 1)
    {
        buildCost = kasfShipTypeBuildCost("bHeavyCruiser");
        currentRUs = (sdword)kasfRUsGet(1);
        if (currentRUs >= buildCost)
        {
            kasfRUsSet(1, currentRUs - buildCost);
            kasfEntryIncrement(0x09379E54);
        }
    }
    if (kasfEntryValueGet(0x09379E54) == 1 &&
        (sdword)kasfRUsGet(1) >= kasfShipTypeBuildCost("bHeavyCruiser"))
    {
        return;
    }
    if (kasfEntryValueGet(0x09379E54) == 1)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_BeastHeavyCruiserCombatTeamBuildTimer,
            (sdword)Watch_Mission17_BeastHeavyCruiserCombatTeamBuildTimer);
    }
}

void Watch_Mission17_BeastHeavyCruiserCombatTeamBuildTimer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)))
    {
        kasfEntrySetValue(0x09379E54, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasTimerExpiredById(0x0E09CB85))
    {
        kasfStop();
        kasfShipsCreate(
            kasGrowSelectionPtr((char *)0x09379E54),
            "bHeavyCruiser",
            kasfEntryValueGet(0x09379E54),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)0x09379E54), 1);
        kasfEntrySetValue(0x09379E54, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)0x09379E54));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x00469684), &team->shipList);
        kasfShipsSetSlaveableFlag(&team->shipList, 0);
        kasfSpeechEvent(
            175938357,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRun);
    }
}

void Begin_Mission17_BeastHeavyCruiserAttackRun(void)
{
    AITeam *team;
    GrowSelection *specialAttackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(0x0C9B3857))
    {
        if (!kasfShipsSelectIndex(
                kasGrowSelectionPtr((char *)0x05425),
                kasGrowSelectionPtr((char *)0x0E05243),
                randyrandombetween(
                    RAN_AIPlayer,
                    0,
                    kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)) - 1)))
        {
            goto specialAttackWatch;
        }
        kasfEntrySetValue(0x0C9B3857, 1);
        specialAttackTargets = kasGrowSelectionPtr((char *)0x0716C77E);
        kasfShipsSetHyperspaceDestinationToCurrentPosition(specialAttackTargets);
        kasfShipsAdd(specialAttackTargets, &team->shipList);
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x05425), specialAttackTargets);
        kasfShipsSetMilitaryParade(kasGrowSelectionPtr((char *)0x05425), specialAttackTargets);
        kasfTeamAttributesBitSet(0x80);
        kasfAttackMothership();
        kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
        kasTimerCreateSetStartById(0x0165C5B5, 14);
    }

specialAttackWatch:
    if (kasfEntryValueGet(0x0C9B3857) && kasTimerExpiredById(0x0165C5B5))
    {
        kasfEntrySetValue(0x05137B, 0);
        kasTimerSetById(0x0883A004, 20);
        kasTimerStartById(0x0883A004);
        kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x04CC48B4);
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
    }
}

void Begin_Mission17_BeastHeavyCruiserAttackRetryDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfLabelledVectorSet(
        (char *)0x07A79EA4,
        kasfEntryValueGet(88),
        kasfEntryValueGet(89),
        kasfEntryValueGet(90));
    if (kasfFindDistance(
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            kasVectorPtrIfExists((char *)0x07A79EA4)) >= 25000.0f)
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
        return;
    }
    kasfEntrySetValue(0x05137B, 1);
    kasfShipsSortByDistanceToPoint(
        kasGrowSelectionPtr((char *)0x036980EB),
        ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfFormationWall();
    kasfShipsSetResourcingFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    if (kasfShipsSelectCapital(
            kasGrowSelectionPtr((char *)0x0764E81C),
            kasGrowSelectionPtr((char *)0x036980EB)))
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x0764E81C));
    }
    else
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x036980EB));
    }
    kasfShipsSetResourcingFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
}

void Watch_Mission17_BeastHeavyCruiserAttackRetryDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFindShipsTargetingSelection(
        kasGrowSelectionPtr((char *)0x0A79EA22),
        kasGrowSelectionPtr((char *)0x0857BCE3));
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x057BCEC2)) &&
        kasfShipsAdd(
            kasGrowSelectionPtr((char *)0x0857BCE3),
            kasGrowSelectionPtr((char *)0x057BCEC2)) &&
        kasfShipsSelectIndex(
            kasGrowSelectionPtr((char *)0x05425),
            kasGrowSelectionPtr((char *)0x0857BCE3),
            0) &&
        kasfFindDistance(
            kasShipsVectorPtrIfExists((char *)0x05425),
            kasVectorPtrIfExists((char *)0x07A79EA4)) < 25000.0f)
    {
        kasfSpeechEvent(74024315, (sdword)sub_663CE0, (sdword)sub_663E60);
        return;
    }
    if (kasTimerExpiredById(0x04C9664))
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
    }
}

void Init_Mission17_BeastMultiBeamFrigateCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x05137B, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x03BE7685)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(0x09379E54, 0);
    kasfEntryGetOrCreateSetValue(0x06A6C3C4, 0);
    kasfEntryGetOrCreateSetValue(0x814345B4, 0);
    kasfEntryGetOrCreateSetValue(0x85B9D165, 1);
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeam);
        return;
    }

    kasfShipsAdd(kasGrowSelectionPtr((char *)0x08D9BC74), &team->shipList);
    kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x0B6BE0B4);
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfEntrySetValue(0x05137B, 0);
    kasfSpeechEvent(
        0x04C9664,
        (sdword)Begin_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun,
        (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun);
}

void Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x03BE7685)) &&
        kasfEntryValueGet(0x85B9D165) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(0x85B9D165, 0);
        kasfEntrySetValue(0x814345B4, 1);
    }
    if (!kasfEntryValueGet(0x05137B) && !kasfTeamCount())
    {
        kasfEntrySetValue(0x814345B4, 0);
        kasfEntrySetValue(0x85B9D165, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x045E9C64);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeam);
    }
}

void Watch_Mission17_BeastMultiBeamFrigateCombatTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)))
    {
        kasfEntrySetValue(0x814345B4, 0);
        kasfEntrySetValue(0x85B9D165, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }
    if (kasfEntryValueGet(0x814345B4))
    {
        kasfEntrySetValue(0x814345B4, 0);
        kasfEntrySetValue(0x85B9D165, 1);
        kasfSpeechEvent(
            165710423,
            (sdword)ResetReinforcementBuildCount,
            (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildCount);
    }
}

void Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildCount(void)
{
    sdword buildCost;
    sdword currentRUs;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(0x09379E54) < 4)
    {
        buildCost = kasfShipTypeBuildCost("bMultiBeamFrigate");
        currentRUs = (sdword)kasfRUsGet(1);
        if (currentRUs >= buildCost)
        {
            kasfRUsSet(1, currentRUs - buildCost);
            kasfEntryIncrement(0x09379E54);
        }
    }
    if (kasfEntryValueGet(0x09379E54) > 1 &&
        (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMultiBeamFrigate"))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer,
            (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer);
    }
    else if (kasfEntryValueGet(0x09379E54) == 4)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer,
            (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer);
    }
}

void Begin_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(0x0E09CB85);
    kasTimerCreateSetStartById(
        0x0E09CB85,
        kasfEntryValueGet(0x09379E54) * kasfShipTypeBuildTime("bMultiBeamFrigate"));
}

void Watch_Mission17_BeastMultiBeamFrigateCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *spawnMothership;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)))
    {
        kasfEntrySetValue(0x09379E54, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(0x0E09CB85))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)) - 1);
        spawnMothership = kasGrowSelectionPtr((char *)0x05425);
        kasfShipsSelectIndex(spawnMothership, kasGrowSelectionPtr((char *)0x0E05243), spawnIndex);
        kasfShipsCreateInside(
            kasGrowSelectionPtr((char *)0x09379E54),
            "bMultiBeamFrigate",
            kasfEntryValueGet(0x09379E54),
            spawnMothership,
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)0x09379E54), 1);
        kasfEntrySetValue(0x09379E54, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)0x09379E54));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x08D9BC74), &team->shipList);
    }

    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x0B6BE0B4);
        kasfEntrySetValue(0x05137B, 0);
        kasfSpeechEvent(
            0x04C9664,
            (sdword)Begin_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun,
            (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun);
    }
}

void Init_Mission17_BeastMultiBeamFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x03BE7685)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(0x8DFD3D15, 0);
    kasfEntryGetOrCreateSetValue(0x8D148F15, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x01776DB3);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, 0);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
    }
}

void Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x03BE7685)) &&
        kasfEntryValueGet(0x8D148F15) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(0x8D148F15, 0);
        kasfEntrySetValue(0x8DFD3D15, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0220A043);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
    }
}

void Watch_Mission17_BeastMultiBeamFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *newFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    newFrigates = kasGrowSelectionPtr((char *)0x058A20);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bMultiBeamFrigate") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)))
        {
            if (kasfEntryValueGet(0x8DFD3D15))
            {
                kasfEntrySetValue(0x8DFD3D15, 0);
                kasfEntrySetValue(0x8D148F15, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, 0);
            }
        }
        else
        {
            kasfEntrySetValue(0x8DFD3D15, 0);
            kasfEntrySetValue(0x8D148F15, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x01776DB3);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bMultiBeamFrigate"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), newFrigates);
        kasfShipsStop(newFrigates);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), newFrigates);
        kasfShipsSetSlaveableFlag(newFrigates, 0);
        kasfShipsApplyPlayerUpgrades(newFrigates);
        kasfReinforceTeamWithShips(team, newFrigates);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, 0);
}

void Watch_Mission17_BeastMultiBeamFrigateReinforcementBuildCount(void)
{
    sdword buildCount;
    sdword buildCost;
    sdword buildScale;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(0x813DB354);
    if (buildScale)
    {
        if (buildScale == 1)
        {
            if (kasfEntryValueGet(154639956) < 3)
            {
                buildCost = kasfShipTypeBuildCost("bMultiBeamFrigate") / 2;
                if ((sdword)kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 1 &&
                 (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMultiBeamFrigate") / 2) ||
                kasfEntryValueGet(154639956) == 3)
            {
                kasfSpeechEvent(75929221, (sdword)Begin_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer, 0);
            }
            return;
        }
        if (buildScale == 2)
        {
            if (kasfEntryValueGet(154639956) < 3)
            {
                kasfEntryIncrement(154639956);
            }
            if (kasfEntryValueGet(154639956) == 3)
            {
                kasfSpeechEvent(75929221, (sdword)Begin_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer, 0);
            }
            return;
        }
        return;
    }

    if (kasfEntryValueGet(154639956) < 3)
    {
        buildCost = kasfShipTypeBuildCost("bMultiBeamFrigate");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    buildCount = kasfEntryValueGet(154639956);
    if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bMultiBeamFrigate")) ||
        buildCount == 3)
    {
        kasfSpeechEvent(75929221, (sdword)Begin_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer, 0);
    }
}

void Begin_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer(void)
{
    sdword buildScale;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(0x813DB354);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bMultiBeamFrigate"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bMultiBeamFrigate") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}

void Watch_Mission17_BeastMultiBeamFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTeam;
    GrowSelection *spawnMothership;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTeam = kasGrowSelectionPtr((char *)0x0E05243);
    if (!kasfShipsCount(launchTeam))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(launchTeam) - 1);
        spawnMothership = kasGrowSelectionPtr((char *)0x05425);
        kasfShipsSelectIndex(spawnMothership, launchTeam, spawnIndex);
        kasfShipsCreateInside(
            kasGrowSelectionPtr((char *)0x09379E54),
            "bMultiBeamFrigate",
            kasfEntryValueGet(154639956),
            spawnMothership,
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)0x09379E54), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)0x09379E54));
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x01776DB3);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, 0);
    }
}

void Watch_Mission17_BeastMultiBeamFrigateCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, 0);
    }
}

void Init_Mission17_ReinforcementPool(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(
        0x0469857B,
        (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
        (sdword)Watch_Mission17_ReinforcementPool_Combat);
}

void Watch_Mission17_ReinforcementPool_Combat(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x053A0C, (sdword)sub_652EF0, (sdword)Watch_Mission17_ReinforcementPool_Resume);
    }
    else if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            0x0469857B,
            (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
            (sdword)Watch_Mission17_ReinforcementPool_Combat);
    }
}

void Watch_Mission17_ReinforcementPool_Resume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(
            0x0469857B,
            (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
            (sdword)Watch_Mission17_ReinforcementPool_Combat);
    }
}

void Init_Mission17_BeastACVReinforcementWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x05137B, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x04586)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(0x09379E54, 0);
    kasfEntryGetOrCreateSetValue(0x06A6C3C4, 0);
    kasfEntryGetOrCreateSetValue(0x89E456F6, 0);
    kasfEntryGetOrCreateSetValue(0x86F37875, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x02AB58E3);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(0x05137B, 0);
        kasfSpeechEvent(
            0x0546973C,
            (sdword)Begin_Mission17_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
    }
    else
    {
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastACVReinforcementWave);
    }
}

void Init_Mission17_BeastACVReinforcementWave_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x04586))
     && kasfEntryValueGet(0x86F37875)
     && kasfMsgReceived((char *)0x479E04))
    {
        kasfEntrySetValue(0x86F37875, 0);
        kasfEntrySetValue(0x89E456F6, 1);
    }

    if (!kasfEntryValueGet(0x05137B))
    {
        if (!kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0D985C33);
            kasfSpeechEvent(
                0x479E04,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_BeastACVReinforcementWave);
            return;
        }
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)) && kasfTeamHealthAverage() < 50)
        {
            kasfSpeechEvent(
                0x04937B,
                (sdword)Begin_Mission17_BeastCloakedFighterDockAndReinforce,
                (sdword)Watch_Mission17_BeastACVDockAndReinforce);
        }
    }
}

void Watch_Mission17_BeastACVReinforcementWave(void)
{
    AITeam *team;
    GrowSelection *reinforcementSelection;
    GrowSelection *returnedACVs;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    reinforcementSelection = kasGrowSelectionPtr((char *)0x02A98C74);
    returnedACVs = kasGrowSelectionPtr((char *)0x058A20);
    if (kasfShipsSelectType(returnedACVs, reinforcementSelection, "bACV") < 2)
    {
        if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)))
        {
            kasfEntrySetValue(0x89E456F6, 0);
            kasfEntrySetValue(0x86F37875, 1);
            kasfSpeechEvent(0x053A0C, (sdword)sub_65F6F0, (sdword)sub_652EF0);
            return;
        }

        if (kasfEntryValueGet(0x89E456F6))
        {
            kasfEntrySetValue(0x89E456F6, 0);
            kasfEntrySetValue(0x86F37875, 1);
            kasfSpeechEvent(
                0x09E08A57,
                (sdword)ResetReinforcementBuildCount,
                (sdword)Watch_Mission17_BeastACVReinforcementBuildCount);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x02AB58E3);
    kasfEntrySetValue(0x05137B, 0);
    if (kasfShipsSelectType(returnedACVs, reinforcementSelection, "bACV"))
    {
        kasfShipsRemove(reinforcementSelection, returnedACVs);
        kasfShipsStop(returnedACVs);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), returnedACVs);
        kasfShipsSetSlaveableFlag(returnedACVs, 0);
        kasfShipsApplyPlayerUpgrades(returnedACVs);
        kasfReinforceTeamWithShips(team, returnedACVs);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        0x0546973C,
        (sdword)Begin_Mission17_BeastACVReinforcementCombatLoop,
        (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
}

void Watch_Mission17_BeastACVReinforcementBuildCount(void)
{
    sdword buildScale;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(0x813DB354);
    if (buildScale != 0)
    {
        buildScale--;
        if (buildScale != 0)
        {
            if (buildScale != 1)
            {
                return;
            }
            if (kasfEntryValueGet(0x09379E54) < 10)
            {
                kasfEntryIncrement(0x09379E54);
            }
        checkAcvBuildReady:
            if (kasfEntryValueGet(0x09379E54) != 10)
            {
                return;
            }
            goto beginAcvBuildTimer;
        }

        if (kasfEntryValueGet(0x09379E54) < 10)
        {
            buildCost = kasfShipTypeBuildCost("bACV") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        if (kasfEntryValueGet(0x09379E54) <= 2)
        {
            goto checkAcvBuildReady;
        }
        buildCost = kasfShipTypeBuildCost("bACV") / 2;
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            goto checkAcvBuildReady;
        }
beginAcvBuildTimer:
        kasfSpeechEvent(
            0x04869685,
            (sdword)Init_Mission16_BeastACVReinforcementBuildTimer,
            (sdword)Watch_Mission17_BeastACVReinforcementSpawn);
        return;
    }

    if (kasfEntryValueGet(0x09379E54) < 10)
    {
        buildCost = kasfShipTypeBuildCost("bACV");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(0x09379E54);
        }
    }
    if (kasfEntryValueGet(0x09379E54) > 2)
    {
        buildCost = kasfShipTypeBuildCost("bACV");
        if ((sdword)kasfRUsGet(1) < buildCost)
        {
            goto beginAcvBuildTimer;
        }
    }
    if (kasfEntryValueGet(0x09379E54) == 10)
    {
        goto beginAcvBuildTimer;
    }
}

void Watch_Mission17_BeastACVReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *spawnSource;
    GrowSelection *spawnTarget;
    GrowSelection *createdACVs;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnSource = kasGrowSelectionPtr((char *)0x0E05243);
    if (!kasfShipsCount(spawnSource))
    {
        kasfEntrySetValue(0x09379E54, 0);
        kasfSpeechEvent(0x053A0C, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(0x0E09CB85))
    {
        kasfStop();
        spawnTarget = kasGrowSelectionPtr((char *)0x05425);
        kasfShipsSelectIndex(spawnTarget, spawnSource, randyrandombetween(RAN_AIPlayer, 0, kasfShipsCount(spawnSource) - 1));
        createdACVs = kasGrowSelectionPtr((char *)0x09379E54);
        kasfShipsCreateInside(createdACVs, "bACV", kasfEntryValueGet(0x09379E54), spawnTarget, 1);
        kasAITeamShipsPtr(createdACVs, 1);
        kasfEntrySetValue(0x09379E54, 0);
        kasfReinforceTeamWithShips(team, createdACVs);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(0x05137B, 0);
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x02AB58E3);
        kasfSpeechEvent(
            0x0546973C,
            (sdword)Begin_Mission17_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
    }
}

void Begin_Mission17_BeastACVReinforcementCombatLoop(void)
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
            (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
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

void Watch_Mission17_BeastACVReinforcementCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasThisTeamsVectorPtr(),
            20000) ||
        (kasfUnderAttack(kasGrowSelectionPtr((char *)139967715)) &&
         kasfShipsSelectMembersOfSelection(
             kasGrowSelectionPtr((char *)139967715),
             kasGrowSelectionPtr((char *)139967715),
             kasGrowSelectionPtr((char *)0x36980EB))))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastACVCombatAttackRun,
            (sdword)Watch_Mission17_BeastACVAttackRun);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission17_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
    }
}

static void Begin_Mission17_BeastACVCombatAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;
    GrowSelection *nearbyTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr((char *)338274);
    if (!kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr((char *)0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission17_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
        return;
    }

    if (!kasfShipsSelectType(kasGrowSelectionPtr((char *)0x88B9D8), nearbyTargets, "sLeech") &&
        !kasfShipsSelectType(kasGrowSelectionPtr((char *)0x88B9C8), nearbyTargets, "sMCV") &&
        !kasfShipsSelectType(kasGrowSelectionPtr((char *)0x88B9E0), nearbyTargets, "sMimic"))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), nearbyTargets);
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)139967715)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)139967715));
    }

    kasfFormationDelta3D();
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    kasfShipsSortByDistanceToPoint(attackTargets, (vector *)kasThisTeamsVectorPtr());
    if (kasfShipsAnyHyperspaceReady(&team->shipList))
    {
        if (kasfShipsCount(kasGrowSelectionPtr((char *)139967715)) &&
            kasfShipsSelectMembersOfSelection(nearbyTargets, kasGrowSelectionPtr((char *)139967715), attackTargets))
        {
            kasfIssueSpecialAttack(nearbyTargets);
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

void Watch_Mission17_BeastACVAttackRun(void)
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
                kasThisTeamsVectorPtr(),
                20000))
        {
            kasfSpeechEvent(
                88512316,
                (sdword)Begin_Mission17_BeastACVReinforcementCombatLoop,
                (sdword)Watch_Mission17_BeastACVReinforcementCombatLoop);
            return;
        }
    }
    else if (!kasTimerExpiredById(74024315))
    {
        return;
    }

    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission17_BeastACVCombatAttackRun,
        (sdword)Watch_Mission17_BeastACVAttackRun);
}

void Watch_Mission17_BeastACVDockAndReinforce(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildLimit;
    sdword buildCost;
    sdword buildTime;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastACVReinforcementWave);
        return;
    }

    buildStyle = kasfEntryValueGet(0x813DB354);
    buildLimit = 10 - kasfTeamCount();
    if (buildStyle == 2)
    {
        if (kasfEntryValueGet(0x09379E54) < buildLimit)
        {
            kasfEntryIncrement(0x09379E54);
        }
        if (kasfEntryValueGet(0x09379E54) == 10)
        {
            kasTimerCreateSetStartById(0x07F7D534, 0);
        }
    }
    else if (buildStyle == 1)
    {
        if (kasfEntryValueGet(0x09379E54) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bACV") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        buildCount = kasfEntryValueGet(0x09379E54);
        if ((buildCount > 2 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bACV") / 2)
         || buildCount == 10)
        {
            buildTime = kasfShipTypeBuildTime("bACV") * buildCount;
            kasTimerCreateSetStartById(0x07F7D534, buildTime / 2);
        }
    }
    else
    {
        if (kasfEntryValueGet(0x09379E54) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bACV");
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        buildCount = kasfEntryValueGet(0x09379E54);
        if ((buildCount > 2 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bACV"))
         || buildCount == 10)
        {
            buildTime = kasfShipTypeBuildTime("bACV") * buildCount;
            kasTimerCreateSetStartById(0x07F7D534, buildTime);
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(0x07F7D534))
    {
        kasfShipsSelectLinkedWith(
            &((AITeam *)kasCurrentTeamPtr)->shipList,
            kasGrowSelectionPtr((char *)0x04D484),
            kasGrowSelectionPtr((char *)0x0E05243));
    }
}

void Init_Mission17_BeastIonArrayFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x05137B, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x090A9E15)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(0x0135FAA7, 0);
    kasfEntryGetOrCreateSetValue(0x09379E54, 0);
    kasfEntryGetOrCreateSetValue(0x06A6C3C4, 0);
    kasfEntryGetOrCreateSetValue(0x80EE8F05, 0);
    kasfEntryGetOrCreateSetValue(0x8F097EA5, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0206E163);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(0x05137B, 0);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(0x0135FAA7, 1);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastIonArrayFrigateCombatLoop);
    }
    else
    {
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastIonArrayFrigateReinforcementTeam);
    }
}

void Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x090A9E15))
     && kasfEntryValueGet(0x8F097EA5)
     && kasfMsgReceived((char *)0x479E04))
    {
        kasfEntrySetValue(0x8F097EA5, 0);
        kasfEntrySetValue(0x80EE8F05, 1);
    }

    if (!kasfEntryValueGet(0x05137B) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0D35EDB3);
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastIonArrayFrigateReinforcementTeam);
    }
}

void Watch_Mission17_BeastIonArrayFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *reinforcementSelection;
    GrowSelection *ionArrayFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    reinforcementSelection = kasGrowSelectionPtr((char *)0x02A98C74);
    ionArrayFrigates = kasGrowSelectionPtr((char *)0x058A20);
    if (kasfShipsSelectType(ionArrayFrigates, reinforcementSelection, "bIonArrayFrigate") < 1)
    {
        if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)))
        {
            kasfEntrySetValue(0x80EE8F05, 0);
            kasfEntrySetValue(0x8F097EA5, 1);
            kasfSpeechEvent(0x053A0C, (sdword)sub_65F6F0, (sdword)sub_652EF0);
            return;
        }

        if (kasfEntryValueGet(0x80EE8F05))
        {
            kasfEntrySetValue(0x80EE8F05, 0);
            kasfEntrySetValue(0x8F097EA5, 1);
            kasfSpeechEvent(
                0x09E08A57,
                (sdword)ResetReinforcementBuildCount,
                (sdword)Watch_Mission17_BeastIonArrayFrigateReinforcementBuildCount);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0206E163);
    kasfEntrySetValue(0x05137B, 0);
    kasfTeamSpecialToggle();
    kasfEntrySetValue(0x0135FAA7, 1);
    if (kasfShipsSelectType(ionArrayFrigates, reinforcementSelection, "bIonArrayFrigate"))
    {
        kasfShipsRemove(reinforcementSelection, ionArrayFrigates);
        kasfShipsStop(ionArrayFrigates);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), ionArrayFrigates);
        kasfShipsSetSlaveableFlag(ionArrayFrigates, 0);
        kasfShipsApplyPlayerUpgrades(ionArrayFrigates);
        kasfReinforceTeamWithShips(team, ionArrayFrigates);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        0x0524A5,
        (sdword)Begin_BeastIonArrayFrigateCombatLoop,
        (sdword)Watch_Mission17_BeastIonArrayFrigateCombatLoop);
}

void Watch_Mission17_BeastIonArrayFrigateReinforcementBuildCount(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(0x813DB354);
    if (buildStyle == 2)
    {
        if (kasfEntryValueGet(0x09379E54) < 4)
        {
            kasfEntryIncrement(0x09379E54);
        }
        if (kasfEntryValueGet(0x09379E54) == 4)
        {
            kasfSpeechEvent(
                0x04869685,
                (sdword)Init_Mission05_FrigateReinforcementDelay,
                (sdword)Watch_Mission17_BeastIonArrayFrigateReinforcementSpawn);
        }
        return;
    }
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(0x09379E54) < 4)
        {
            buildCost = kasfShipTypeBuildCost("bIonArrayFrigate") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        buildCount = kasfEntryValueGet(0x09379E54);
        if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bIonArrayFrigate") / 2)
         || buildCount == 4)
        {
            kasfSpeechEvent(
                0x04869685,
                (sdword)Init_Mission05_FrigateReinforcementDelay,
                (sdword)Watch_Mission17_BeastIonArrayFrigateReinforcementSpawn);
        }
        return;
    }

    if (kasfEntryValueGet(0x09379E54) < 4)
    {
        buildCost = kasfShipTypeBuildCost("bIonArrayFrigate");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(0x09379E54);
        }
    }

    buildCount = kasfEntryValueGet(0x09379E54);
    if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bIonArrayFrigate"))
     || buildCount == 4)
    {
        kasfSpeechEvent(
            0x04869685,
            (sdword)Init_Mission05_FrigateReinforcementDelay,
            (sdword)Watch_Mission17_BeastIonArrayFrigateReinforcementSpawn);
    }
}

void Watch_Mission17_BeastIonArrayFrigateReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *newFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)))
    {
        kasfEntrySetValue(0x09379E54, 0);
        kasfSpeechEvent(0x053A0C, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(0x0E09CB85))
    {
        kasfStop();
        newFrigates = kasGrowSelectionPtr((char *)0x09379E54);
        kasfShipsCreate(newFrigates, "bIonArrayFrigate", kasfEntryValueGet(0x09379E54), 1);
        kasAITeamShipsPtr(newFrigates, 1);
        kasfEntrySetValue(0x09379E54, 0);
        kasfReinforceTeamWithShips(team, newFrigates);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
        kasfSpeechEvent(
            0x0A7C9B35,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Watch_Mission17_BeastIonArrayFrigateCombatLoop);
    }
}

void Begin_Mission17_BeastIonArrayFrigateAttackRun(void)
{
    AITeam *team;
    GrowSelection *specialAttackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(0x0C9B3857))
    {
        if (kasfShipsSelectIndex(
                kasGrowSelectionPtr((char *)0x05425),
                kasGrowSelectionPtr((char *)0x0E05243),
                randyrandombetween(
                    RAN_AIPlayer,
                    0,
                    kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)) - 1)))
        {
            kasfEntrySetValue(0x0C9B3857, 1);
            specialAttackTargets = kasGrowSelectionPtr((char *)0x0716C77E);
            kasfGrowSelectionClear(specialAttackTargets);
            kasfShipsAdd(specialAttackTargets, kasGrowSelectionPtr((char *)0x05425));
            kasfShipsAdd(specialAttackTargets, &team->shipList);
            kasfShipsSetMilitaryParade(kasGrowSelectionPtr((char *)0x05425), specialAttackTargets);
            kasfTeamAttributesBitSet(0x80);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(0x0165C5B5, 8);
        }
    }

    if (kasfEntryValueGet(0x0C9B3857) && kasTimerExpiredById(0x0165C5B5))
    {
        kasfMsgSend(kasAITeamPtrIfExists(0x0206E163), (char *)0x0206E163);
        kasfEntrySetValue(0x05137B, 0);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(0x0135FAA7, 1);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastIonArrayFrigateCombatLoop);
    }
}

void Watch_Mission17_BeastIonArrayFrigateCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            6000))
    {
        if (!kasfUnderAttack(kasGrowSelectionPtr((char *)0x0857BCE3))
         || !kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr((char *)0x0857BCE3),
                kasGrowSelectionPtr((char *)0x0857BCE3),
                kasGrowSelectionPtr((char *)0x36980EB)))
        {
            if (!kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x36980EB)))
            {
                return;
            }
        }

        kasfSpeechEvent(
            0x0469857B,
            (sdword)Begin_Mission17_BeastIonArrayFrigateAttackRun,
            (sdword)Watch_Mission17_BeastIonArrayFrigateAttackRun);
        return;
    }

    if (kasTimerExpiredDestroyById(0x0524A5) || kasfShipsAnyIdle(&team->shipList))
    {
        kasTimerDestroyById(0x0524A5);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastIonArrayFrigateCombatLoop);
    }
}

void Watch_Mission17_BeastIonArrayFrigateAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        if (!kasfEntryValueGet(0x0135FAA7))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(0x0135FAA7, 1);
        }
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission17_BeastIonArrayFrigateCombatLoop);
        return;
    }

    if (kasTimerExpiredById(0x0469857B))
    {
        kasfSpeechEvent(
            0x0469857B,
            (sdword)Begin_Mission17_BeastIonArrayFrigateAttackRun,
            (sdword)Watch_Mission17_BeastIonArrayFrigateAttackRun);
    }
}

void Init_Mission17_BeastCloakedFighterReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x05137B, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x0BEE25A2)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(0x0135FAA7, 0);
    kasfEntryGetOrCreateSetValue(0x09379E54, 0);
    kasfEntryGetOrCreateSetValue(0x06A6C3C4, 0);
    kasfEntryGetOrCreateSetValue(0x82706472, 0);
    kasfEntryGetOrCreateSetValue(0x84750445, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0BC8AAC3);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(0x05137B, 0);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(0x0135FAA7, 1);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
    }
    else
    {
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastCloakedFighterReinforcementTeam);
    }
}

void Init_Mission17_BeastCloakedFighterReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x0BEE25A2))
     && kasfEntryValueGet(0x84750445)
     && kasfMsgReceived((char *)0x479E04))
    {
        kasfEntrySetValue(0x84750445, 0);
        kasfEntrySetValue(0x82706472, 1);
    }

    if (!kasfEntryValueGet(0x05137B))
    {
        if (!kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x063B9A83);
            kasfSpeechEvent(
                0x479E04,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission17_BeastCloakedFighterReinforcementTeam);
            return;
        }
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)) && kasfTeamHealthAverage() < 50)
        {
            kasfSpeechEvent(
                0x04937B,
                (sdword)Begin_Mission17_BeastCloakedFighterDockAndReinforce,
                (sdword)Watch_Mission17_BeastCloakedFighterDockAndReinforce);
        }
    }
}

void Watch_Mission17_BeastCloakedFighterReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *reinforcementSelection;
    GrowSelection *cloakedFighters;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    reinforcementSelection = kasGrowSelectionPtr((char *)0x02A98C74);
    cloakedFighters = kasGrowSelectionPtr((char *)0x058A20);
    if (kasfShipsSelectType(cloakedFighters, reinforcementSelection, "bCloakedFighter") < 1)
    {
        if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)))
        {
            kasfEntrySetValue(0x82706472, 0);
            kasfEntrySetValue(0x84750445, 1);
            kasfSpeechEvent(0x053A0C, (sdword)sub_65F6F0, (sdword)sub_652EF0);
            return;
        }

        if (kasfEntryValueGet(0x82706472))
        {
            kasfEntrySetValue(0x82706472, 0);
            kasfEntrySetValue(0x84750445, 1);
            kasfSpeechEvent(
                0x09E08A57,
                (sdword)ResetReinforcementBuildCount,
                (sdword)Watch_Mission17_BeastCloakedFighterReinforcementBuildCount);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0x0BC8AAC3);
    kasfEntrySetValue(0x05137B, 0);
    kasfTeamSpecialToggle();
    kasfEntrySetValue(0x0135FAA7, 1);
    if (kasfShipsSelectType(cloakedFighters, reinforcementSelection, "bCloakedFighter"))
    {
        kasfShipsRemove(reinforcementSelection, cloakedFighters);
        kasfShipsStop(cloakedFighters);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), cloakedFighters);
        kasfShipsSetSlaveableFlag(cloakedFighters, 0);
        kasfShipsApplyPlayerUpgrades(cloakedFighters);
        kasfReinforceTeamWithShips(team, cloakedFighters);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        0x0524A5,
        (sdword)Begin_BeastCloakedFighterCombatLoop,
        (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
}

void Watch_Mission17_BeastCloakedFighterReinforcementBuildCount(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(0x813DB354);
    if (buildStyle == 2)
    {
        if (kasfEntryValueGet(0x09379E54) < 12)
        {
            kasfEntryIncrement(0x09379E54);
        }
        if (kasfEntryValueGet(0x09379E54) == 12)
        {
            kasfSpeechEvent(
                0x04869685,
                (sdword)Begin_Mission17_BeastCloakedFighterReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastCloakedFighterReinforcementSpawn);
        }
        return;
    }
    if (buildStyle == 1)
    {
        if (kasfEntryValueGet(0x09379E54) < 12)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        buildCount = kasfEntryValueGet(0x09379E54);
        if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter") / 2)
         || buildCount == 12)
        {
            kasfSpeechEvent(
                0x04869685,
                (sdword)Begin_Mission17_BeastCloakedFighterReinforcementBuildTimer,
                (sdword)Watch_Mission17_BeastCloakedFighterReinforcementSpawn);
        }
        return;
    }

    if (kasfEntryValueGet(0x09379E54) < 12)
    {
        buildCost = kasfShipTypeBuildCost("bCloakedFighter");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(0x09379E54);
        }
    }

    buildCount = kasfEntryValueGet(0x09379E54);
    if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
     || buildCount == 12)
    {
        kasfSpeechEvent(
            0x04869685,
            (sdword)Begin_Mission17_BeastCloakedFighterReinforcementBuildTimer,
            (sdword)Watch_Mission17_BeastCloakedFighterReinforcementSpawn);
    }
}

void Begin_Mission17_BeastCloakedFighterReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildTime;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(0x0E09CB85);
    buildStyle = kasfEntryValueGet(0x813DB354);
    if (buildStyle == 2)
    {
        kasTimerCreateSetStartById(0x0E09CB85, 0);
    }
    else
    {
        buildTime = kasfShipTypeBuildTime("bCloakedFighter") * kasfEntryValueGet(0x09379E54);
        if (buildStyle == 1)
        {
            buildTime /= 2;
        }
        kasTimerCreateSetStartById(0x0E09CB85, buildTime);
    }
}

void Watch_Mission17_BeastCloakedFighterReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *newFighters;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x0E05243)))
    {
        kasfEntrySetValue(0x09379E54, 0);
        kasfSpeechEvent(0x053A0C, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(0x0E09CB85))
    {
        kasfStop();
        kasfShipsSelectIndex(
            kasGrowSelectionPtr((char *)0x05425),
            kasGrowSelectionPtr((char *)0x0E05243),
            randyrandombetween(
                RAN_AIPlayer,
                0,
                kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)) - 1));
        newFighters = kasGrowSelectionPtr((char *)0x09379E54);
        kasfShipsCreateInside(
            newFighters,
            "bCloakedFighter",
            kasfEntryValueGet(0x09379E54),
            kasGrowSelectionPtr((char *)0x05425),
            1);
        kasAITeamShipsPtr(newFighters, 1);
        kasfEntrySetValue(0x09379E54, 0);
        kasfReinforceTeamWithShips(team, newFighters);
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(0x0BC8AAC3), (char *)0x0BC8AAC3);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(0x0135FAA7, 1);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
    }
}

void Watch_Mission17_BeastCloakedFighterCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            6000))
    {
        if ((kasfUnderAttack(kasGrowSelectionPtr((char *)0x0857BCE3))
          && kasfShipsSelectMembersOfSelection(
                kasGrowSelectionPtr((char *)0x0857BCE3),
                kasGrowSelectionPtr((char *)0x0857BCE3),
                kasGrowSelectionPtr((char *)0x36980EB)))
         || kasfShipsSelectType(
                kasGrowSelectionPtr((char *)0x36980EB),
                kasGrowSelectionPtr((char *)0x36980EB),
                "bCloakedFighter"))
        {
            kasfSpeechEvent(
                0x0469857B,
                (sdword)Begin_Mission17_BeastCloakedFighterAttackRun,
                (sdword)Watch_Mission17_BeastCloakedFighterAttackRun);
            return;
        }
    }

    if (kasTimerExpiredDestroyById(0x0524A5) || kasfShipsAnyIdle(&team->shipList))
    {
        kasTimerDestroyById(0x0524A5);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
    }
}

void Begin_Mission17_BeastCloakedFighterAttackRun(void)
{
    AITeam *team;
    GrowSelection *attackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x36980EB),
            kasGrowSelectionPtr((char *)0x4A2A29),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            6000))
    {
        if (!kasfEntryValueGet(0x0135FAA7))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(0x0135FAA7, 1);
        }
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
        return;
    }

    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x0857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x0857BCE3));
    }
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x36980EB), ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfFormationDelta3D();
    attackTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (kasfShipsCount(attackTargets))
    {
        if (kasfEntryValueGet(0x0135FAA7))
        {
            kasfEntrySetValue(0x0135FAA7, 0);
            kasfTeamSpecialToggle();
        }
    }
    else if (!kasfEntryValueGet(0x0135FAA7))
    {
        kasfEntrySetValue(0x0135FAA7, 1);
        kasfTeamSpecialToggle();
    }

    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x36980EB));
    kasTimerDestroyById(0x0469857B);
    kasTimerCreateSetStartById(0x0469857B, randyrandombetween(RAN_AIPlayer, 10, 30));
}

void Watch_Mission17_BeastCloakedFighterAttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        if (!kasfEntryValueGet(0x0135FAA7))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(0x0135FAA7, 1);
        }
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
        return;
    }

    if (kasTimerExpiredById(0x0469857B))
    {
        kasfSpeechEvent(
            0x0469857B,
            (sdword)Begin_Mission17_BeastCloakedFighterAttackRun,
            (sdword)Watch_Mission17_BeastCloakedFighterAttackRun);
    }
}

sdword Begin_Mission17_BeastCloakedFighterDockAndReinforce(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(0x05137B, 1);
    kasfEntrySetValue(0x09379E54, 0);
    kasTimerDestroyById(0x07F7D534);
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x0E05243), ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x05425), kasGrowSelectionPtr((char *)0x0E05243), 0);
    kasfFormationDelta3D();
    return kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr((char *)0x05425));
}

void Watch_Mission17_BeastCloakedFighterDockAndReinforce(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildLimit;
    sdword buildCost;
    sdword buildTime;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastCloakedFighterReinforcementTeam);
        return;
    }

    buildStyle = kasfEntryValueGet(0x813DB354);
    buildLimit = 12 - kasfTeamCount();
    if (buildStyle == 2)
    {
        if (kasfEntryValueGet(0x09379E54) < buildLimit)
        {
            kasfEntryIncrement(0x09379E54);
        }
        if (kasfEntryValueGet(0x09379E54) == 12)
        {
            kasTimerCreateSetStartById(0x07F7D534, 0);
        }
    }
    else if (buildStyle == 1)
    {
        if (kasfEntryValueGet(0x09379E54) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        buildCount = kasfEntryValueGet(0x09379E54);
        if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter") / 2)
         || buildCount == 12)
        {
            buildTime = kasfShipTypeBuildTime("bCloakedFighter") * buildCount;
            kasTimerCreateSetStartById(0x07F7D534, buildTime / 2);
        }
    }
    else
    {
        if (kasfEntryValueGet(0x09379E54) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bCloakedFighter");
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(0x09379E54);
            }
        }
        buildCount = kasfEntryValueGet(0x09379E54);
        if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
         || buildCount == 12)
        {
            buildTime = kasfShipTypeBuildTime("bCloakedFighter") * buildCount;
            kasTimerCreateSetStartById(0x07F7D534, buildTime);
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(0x07F7D534))
    {
        kasfShipsSelectLinkedWith(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr((char *)0x04D484), kasGrowSelectionPtr((char *)0x0E05243));
        kasfShipsCreateInside(
            kasGrowSelectionPtr((char *)0x09379E54),
            "bCloakedFighter",
            kasfEntryValueGet(0x09379E54),
            kasGrowSelectionPtr((char *)0x04D484),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)0x09379E54), 1);
        kasfShipsDockInstant(kasGrowSelectionPtr((char *)0x09379E54), kasGrowSelectionPtr((char *)0x04D484));
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr((char *)0x09379E54));
        kasfShipsAdd(
            kasGrowSelectionPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash),
            &((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfSpeechEvent(
            0x0506A278,
            (sdword)Init_LaunchDockedTeam,
            (sdword)Watch_Mission17_BeastCloakedFighterLaunchDockedReinforcements);
    }
}

void Watch_Mission17_BeastCloakedFighterLaunchDockedReinforcements(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_Mission17_BeastCloakedFighterReinforcementTeam);
        return;
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(0x0BC8AAC3), (char *)0x0BC8AAC3);
        kasfEntrySetValue(0x05137B, 0);
        kasfSpeechEvent(
            0x0524A5,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_Mission17_BeastCloakedFighterCombatLoop);
    }
}

void Watch_Mission17_BeastCorvetteCapitalStrike(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_BeastCorvetteCombatLoop,
            (sdword)Watch_Mission17_BeastCorvetteCombatLoop);
    }
}

void Watch_Mission17_BeastFighterCapitalStrike(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission17_BeastFighterCombatLoop,
            (sdword)Watch_Mission17_BeastFighterCombatLoop);
    }
    else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission17_BeastFighterCapitalStrike,
            (sdword)Watch_Mission17_BeastFighterCapitalStrike);
    }
}

sdword Begin_Mission17_BeastReconDockAndReinforce(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(14701123), kasThisTeamsVectorPtr());
    kasfShipsSelectIndex(kasGrowSelectionPtr(21541), kasGrowSelectionPtr(14701123), 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    return kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(21541));
}

void Begin_Mission17_BeastDFGFrigateDockAndReinforce(void)
{
    AITeam *team;
    hvector *teamPosition;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    teamPosition = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    kasfShipsSortByDistanceToPoint(kasAITeamShipsPtrIfExists(108453441), teamPosition);
    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), kasAITeamShipsPtrIfExists(108453441), 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr((char *)21541));
}

void Begin_Mission17_BeastHeavyCruiserCombatTeamBuildTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(0x0E09CB85);
    kasTimerCreateSetStartById(
        0x0E09CB85,
        kasfShipTypeBuildTime("bHeavyCruiser") * kasfEntryValueGet(0x09379E54));
}

void Init_Mission17_CarrierAttackMothership(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnTarget;
    GrowSelection *stagingSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        launchTargets = kasGrowSelectionPtr((char *)0x06769508);
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnTarget = kasGrowSelectionPtr((char *)21541);
        if (kasfShipsSelectIndex(spawnTarget, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            stagingSelection = kasGrowSelectionPtr((char *)0x0716C77E);
            kasfGrowSelectionClear(stagingSelection);
            kasfShipsAdd(stagingSelection, &team->shipList);
            kasfShipsAdd(stagingSelection, spawnTarget);
            kasfShipsSetMilitaryParade(spawnTarget, stagingSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            kasfShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 14);
        }
    }

    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfEntrySetValue(332667, 0);
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)103051523);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_CarrierCombatTeamLoop,
            (sdword)Watch_Mission17_CarrierCombatTeamLoop);
    }
}

void Begin_Mission17_CarrierCombatTeamLoop(void)
{
    GrowSelection *attackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x06DA2DA9),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            60000))
    {
        attackTargets = kasGrowSelectionPtr((char *)0x0524AA8F);
        if (!kasfFindShipsTargetingSelection(attackTargets, kasGrowSelectionPtr((char *)0x004A2A29)))
        {
            kasfShipsAdd(attackTargets, kasAITeamShipsPtrIfExists((char *)0x05BB641B));
        }
        kasfShipsSortByDistanceToPoint(attackTargets, ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        if (kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), attackTargets, 0))
        {
            kasfLabelledVectorSet(0x0283C3DF, 0, 0, 0);
            kasfVectorRandomizeInRange(
                kasVectorPtrIfExists((char *)0x0283C3DF),
                kasShipsVectorPtrIfExists((char *)21541),
                30000);
            kasfMoveTo(kasVectorPtrIfExists((char *)0x0283C3DF));
        }
    }
    kasTimerCreateSetStartById(337061, kasfRandom(120, 160));
}

void Watch_Mission17_CarrierCombatTeamLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x06DA2DA9),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            20000))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_CarrierCombatTeamAttackRun);
    }
    else if (kasTimerExpiredById(337061))
    {
        if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
        {
            kasfSpeechEvent(
                337061,
                (sdword)Begin_Mission17_CarrierCombatTeamLoop,
                (sdword)Watch_Mission17_CarrierCombatTeamLoop);
        }
    }
}

void Watch_Mission17_CarrierCombatTeamAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(
            337061,
            (sdword)Begin_Mission17_CarrierCombatTeamLoop,
            (sdword)Watch_Mission17_CarrierCombatTeamLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission17_ReinforcementAttackRun,
            (sdword)Watch_Mission17_CarrierCombatTeamAttackRun);
    }
}

void Begin_Mission17_NaggarokDelayedDeath(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
    kasfShipsSetDelayedDeath(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    if (!kasfEntryValueGet(71552163))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)246013406);
    }
    kasTimerCreateSetStartById(169152067, 30);
}

void Watch_Mission17_NaggarokDelayedDeath(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(169152067))
    {
        kasfSpeechEvent(
            169152067,
            (sdword)Begin_Mission17_NaggarokDelayedDeath,
            (sdword)Watch_Mission17_NaggarokDelayedDeath);
    }
}

void Init_Mission17_ReinforcementTriggerRouter(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(69628553, 0);
    kasfEntryGetOrCreateSetValue(26033149, 0);
    kasfEntryGetOrCreateSetValue(32013909, 0);
}

void Watch_Mission17_ReinforcementTriggerRouter(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;
    AITeam *recipient;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        recipient = NULL;
        if ((udword)message <= 0x0C1ED533)
        {
            if (message == (void *)203347251)
            {
                recipient = kasAITeamPtrIfExists(223105365);
            }
            else if ((udword)message > 0x0988E0A3)
            {
                if (message == (void *)170997699)
                {
                    recipient = kasAITeamPtrIfExists(58381106);
                }
                else if (message == (void *)190169075)
                {
                    recipient = kasAITeamPtrIfExists(171501548);
                }
                else if (message == (void *)202253283)
                {
                    recipient = kasAITeamPtrIfExists(96759714);
                }
            }
            else if (message == (void *)159965347)
            {
                kasfMsgSend(kasAITeamPtrIfExists(379047), (char *)4693508);
                recipient = kasAITeamPtrIfExists(17798);
            }
            else if ((udword)message > 0x063B9A83)
            {
                if (message == (void *)154145491)
                {
                    recipient = kasAITeamPtrIfExists(182180066);
                }
            }
            else if (message == (void *)104569475)
            {
                recipient = kasAITeamPtrIfExists(200156578);
            }
            else if (message == (void *)35692611)
            {
                if (kasfEntryValueGet(26033149))
                {
                    kasfMsgSend(team, (char *)35692611);
                }
                else
                {
                    kasfEntrySetValue(26033149, 1);
                    kasTimerCreateSetStartById(26033149, GetGameDifficulty() <= 2 ? kasfRandom(240, 320) : kasfRandom(120, 240));
                }
            }
            else if (message == (void *)73309284)
            {
                recipient = kasAITeamPtrIfExists(145191172);
            }
        }
        else if ((udword)message > 0x0D985C33)
        {
            if (message == (void *)228101699)
            {
                recipient = kasAITeamPtrIfExists(18599);
            }
            else if (message == (void *)236014723)
            {
                recipient = kasAITeamPtrIfExists(186735637);
            }
            else if (message == (void *)259199139)
            {
                recipient = kasAITeamPtrIfExists(75148549);
            }
        }
        else if (message == (void *)228088883)
        {
            recipient = kasAITeamPtrIfExists(17798);
        }
        else if (message == (void *)204436228)
        {
            recipient = kasAITeamPtrIfExists(208120516);
        }
        else if (message == (void *)221638067)
        {
            if (!kasfEntryValueGet(69628553))
            {
                kasfEntrySetValue(69628553, 1);
                kasTimerCreateSetStartById(69628553, GetGameDifficulty() <= 2 ? kasfRandom(120, 160) : kasfRandom(60, 120));
            }
            else
            {
                recipient = team;
            }
        }
        else if (message == (void *)227820083)
        {
            if (kasfEntryValueGet(32013909))
            {
                kasfMsgSend(team, (char *)227820083);
            }
            else
            {
                kasfEntrySetValue(32013909, 1);
                kasTimerCreateSetStartById(32013909, GetGameDifficulty() <= 2 ? kasfRandom(60, 120) : kasfRandom(50, 90));
            }
        }

        if (recipient != NULL)
        {
            kasfMsgSend(recipient, (char *)4693508);
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }

    if (kasfEntryValueGet(69628553) && kasTimerExpiredById(69628553))
    {
        kasfMsgSend(kasAITeamPtrIfExists(151690773), (char *)4693508);
        kasfEntrySetValue(69628553, 0);
    }
    if (kasfEntryValueGet(26033149) && kasTimerExpiredById(26033149))
    {
        kasfEntrySetValue(26033149, 0);
        kasfMsgSend(kasAITeamPtrIfExists(62813829), (char *)4693508);
    }
    if (kasfEntryValueGet(32013909) && kasTimerExpiredById(32013909))
    {
        kasfEntrySetValue(32013909, 0);
        kasfMsgSend(kasAITeamPtrIfExists(17798), (char *)4693508);
    }
}

void Init_Mission17_TaiidanMultigunCorvetteCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfTeamAttributesBitSet(128);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x85DD80E), &team->shipList);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(40027877)))
    {
        kasfSpeechEvent(
            175938357,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Begin_Mission17_TaiidanMultigunCorvetteReinforcementArrival);
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x262C6E5), &team->shipList);
    }
}

void Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(40027877)))
    {
        queueInfo = *((sdword **)team + 20);
        queueIndex = -1;
        if (queueInfo != NULL)
        {
            queueIndex = *queueInfo;
        }
        while (queueIndex >= 0)
        {
            message = aitQueuePopPointerByIndex(team, queueIndex);
            if (message == (void *)157864733)
            {
                kasfShipsSetHyperspaceDestination(
                    kasGrowSelectionPtr((char *)0x262C6E5),
                    kasVectorPtrIfExists((char *)0x0A2239A4));
                kasFSMCreate(
                    (char *)0x41312D3,
                    Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam,
                    Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam_Reevaluate,
                    kasAITeamPtrIfExists(68358673));
                kasFSMCreate(
                    (char *)0x41312D3,
                    Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam,
                    Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam_Reevaluate,
                    kasAITeamPtrIfExists(68358674));
                kasFSMCreate(
                    (char *)0x6768203,
                    Init_Mission17_CarrierReinforcementTeam,
                    Init_Mission17_CarrierReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(108429889));
                kasFSMCreate(
                    (char *)0x6768203,
                    Init_Mission17_CarrierReinforcementTeam,
                    Init_Mission17_CarrierReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(108429890));
                kasFSMCreate(
                    (char *)0xB08C273,
                    Init_Mission17_HeavyCruiserReinforcementTeam,
                    Init_Mission17_HeavyCruiserReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(185123377));
                kasFSMCreate(
                    (char *)0xB08C273,
                    Init_Mission17_HeavyCruiserReinforcementTeam,
                    Init_Mission17_HeavyCruiserReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(185123378));
                kasFSMCreate(
                    (char *)0x766E863,
                    Init_Mission17_TaiidanIonCannonFrigateCombatTeam,
                    Init_Mission17_TaiidanIonCannonFrigateCombatTeam_Reevaluate,
                    kasAITeamPtrIfExists(124184737));
                kasFSMCreate(
                    (char *)0x766E863,
                    Init_Mission17_TaiidanIonCannonFrigateCombatTeam,
                    Init_Mission17_TaiidanIonCannonFrigateCombatTeam_Reevaluate,
                    kasAITeamPtrIfExists(124184738));
                kasFSMCreate(
                    (char *)0x7AF10E3,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(267788193));
                kasFSMCreate(
                    (char *)0x7AF10E3,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(267788194));
                kasFSMCreate(
                    (char *)0x7AF10E3,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(267788195));
                kasFSMCreate(
                    (char *)0x7AF10E3,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(267788196));
                kasFSMCreate(
                    (char *)0x7AF10E3,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam,
                    Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate,
                    kasAITeamPtrIfExists(267788197));
                kasFSMCreate(
                    (char *)0x6943223,
                    Init_Mission17_TaiidanStandardDestroyerCombatTeam,
                    Init_Mission17_TaiidanStandardDestroyerCombatTeam_Reevaluate,
                    kasAITeamPtrIfExists(90784609));
                kasFSMCreate(
                    (char *)0x6943223,
                    Init_Mission17_TaiidanStandardDestroyerCombatTeam,
                    Init_Mission17_TaiidanStandardDestroyerCombatTeam_Reevaluate,
                    kasAITeamPtrIfExists(90784610));
            }
            else if (message == (void *)5801588)
            {
                if (!kasfEntryValueGet(71552165))
                {
                    kasfEntryGetOrCreateSetValue(71552165, 1);
                    kasfEntryGetOrCreateSetValue(0x8F52C6A5, 1);
                    kasTimerCreateSetStartById(0x8F52C6A5, 10);
                }
            }
            else if (message != NULL)
            {
                aitQueuePointer(team, team, message);
            }

            queueInfo = *((sdword **)team + 20);
            if (queueInfo == NULL || *queueInfo == 0)
            {
                break;
            }
            queueIndex--;
        }
    }
    else if (!kasfEntryValueGet(71552166) && kasfEntryValueGet(0x8F52C6A5))
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        kasfSpeechEvent(
            374244,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Watch_Mission17_TaiidanMultigunCorvetteReinforcementArrival);
    }
}

void Begin_Mission17_TaiidanMultigunCorvetteReinforcementArrival(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552167) && kasfShipsCount(kasGrowSelectionPtr((char *)0x262C6E5)))
    {
        kasfEntryGetOrCreateSetValue(71552167, 1);
        kasfShipsHyperspaceIn(kasGrowSelectionPtr((char *)0x262C6E5), NULL);
    }
}

void Watch_Mission17_TaiidanMultigunCorvetteReinforcementArrival(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(0x8F52C6A5))
    {
        kasfMsgSend(kasAITeamPtrIfExists(40027877), (char *)157864733);
    }
}

void Init_Mission17_FinalDFGFrigateCombatTeam_Reevaluate(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(379047)) &&
        kasfEntryValueGet(-1927751787) &&
        kasfMsgReceived(4693508))
    {
        kasfEntrySetValue(-1927751787, 0);
        kasfEntrySetValue(-1908032238, 1);
    }
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasGrowSelectionPtr(14701123)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_Mission17_BeastReconDockAndReinforce,
                    (sdword)Watch_Mission17_FinalDFGFrigateDockAndReinforce);
            }
        }
        else
        {
            kasfEntrySetValue(-1912498034, kasfEntryValueGet(-1912498034) - kasfEntryValueGet(111786462));
            kasfMsgSend(kasAITeamPtrIfExists(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), (char *)159965347);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        }
    }
}

void Watch_Mission17_FinalDFGFrigateCombatTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr(14701123)))
    {
        if (kasfEntryValueGet(-1908032238))
        {
            kasfEntrySetValue(-1908032238, 0);
            kasfEntrySetValue(-1927751787, 1);
            kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, 0);
        }
    }
    else
    {
        kasfEntrySetValue(-1908032238, 0);
        kasfEntrySetValue(-1927751787, 1);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Watch_Mission17_FinalDFGFrigateCombatTeamBuildCount(void)
{
    sdword buildCountMode;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildCountMode = kasfEntryValueGet(-2126662828);
    if (buildCountMode)
    {
        if (buildCountMode == 1)
        {
            if (kasfEntryValueGet(154639956) < 1)
            {
                sdword buildCost = kasfShipTypeBuildCost("bDFGFrigate") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 1 &&
                 kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate") / 2) ||
                kasfEntryValueGet(154639956) == 1)
            {
                kasfSpeechEvent(
                    75929221,
                    (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer,
                    (sdword)Watch_Mission17_FinalDFGFrigateCombatTeamBuildTimer);
            }
        }
        else if (buildCountMode == 2)
        {
            if (kasfEntryValueGet(154639956) < 1)
            {
                kasfEntryIncrement(154639956);
            }
            if (kasfEntryValueGet(154639956) == 1)
            {
                kasfSpeechEvent(
                    75929221,
                    (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer,
                    (sdword)Watch_Mission17_FinalDFGFrigateCombatTeamBuildTimer);
            }
        }
    }
    else
    {
        if (kasfEntryValueGet(154639956) < 1)
        {
            sdword buildCost = kasfShipTypeBuildCost("bDFGFrigate");
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1 &&
             kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate")) ||
            kasfEntryValueGet(154639956) == 1)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastDFGFrigateReinforcementBuildTimer,
                (sdword)Watch_Mission17_FinalDFGFrigateCombatTeamBuildTimer);
        }
    }
}

void Watch_Mission17_FinalDFGFrigateCombatTeamBuildTimer(void)
{
    AITeam *team;
    GrowSelection *dockTargets;
    GrowSelection *spawnTarget;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsCount(kasGrowSelectionPtr(14701123)))
    {
        if (kasTimerExpiredById(235522949))
        {
            kasfStop();
            dockTargets = kasGrowSelectionPtr(14701123);
            targetIndex = kasfRandom(0, kasfShipsCount(dockTargets) - 1);
            spawnTarget = kasGrowSelectionPtr((char *)21541);
            kasfShipsSelectIndex(spawnTarget, dockTargets, targetIndex);
            kasfShipsCreateInside(
                kasGrowSelectionPtr(154639956),
                "bDFGFrigate",
                (char *)kasfEntryValueGet(154639956),
                spawnTarget,
                1);
            kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
            kasfEntrySetValue(154639956, 0);
            kasfReinforceTeamWithShips((sdword)team, kasGrowSelectionPtr(154639956));
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        }
        if (kasfTeamFinishedLaunching() && kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)179875155);
            kasfEntrySetValue(332667, 0);
            kasfEntrySetValue(111786462, 0);
            kasfSpeechEvent(
                0x04C9664,
                (sdword)Begin_Mission17_FinalDFGFrigateCombatLoop,
                (sdword)Watch_Mission17_FinalDFGFrigateCombatLoop);
        }
    }
    else
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Begin_Mission17_FinalDFGFrigateCombatLoop(void)
{
    GrowSelection *attackTargets;
    GrowSelection *candidateTargets;
    sdword targetIndex;
    hvector *targetPoint;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(90872430, 0);
    attackTargets = kasGrowSelectionPtr(124053532);
    kasfGrowSelectionClear(attackTargets);
    switch (kasfRandom(0, 3))
    {
        case 0:
            if (!(kasfEntryValueGet(-1912498034) & 1) || kasfEntryValueGet(-1912498034) == 15)
            {
                candidateTargets = kasAITeamShipsPtrIfExists(22630782);
                if (kasfShipsCount(candidateTargets))
                {
                    kasfShipsAdd(attackTargets, candidateTargets);
                }
                if (kasfShipsCount(attackTargets))
                {
                    targetIndex = kasfRandom(0, kasfShipsCount(attackTargets) - 1);
                    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), attackTargets, targetIndex);
                    kasLabelledVectorSet((char *)0x058A20, kasShipsVectorPtrIfExists(218060896));
                    if ((sdword)kasfFindDistance(kasVectorPtrIfExists((char *)363040), kasShipsVectorPtrIfExists((char *)21541)) >= 36000)
                    {
                        if (kasfEntryValueGet(-1912498034) != 15)
                        {
                            kasfEntrySetValue(111786462, 1);
                            kasfEntrySetValue(-1912498034, kasfEntryValueGet(-1912498034) + 1);
                        }
                    }
                    else
                    {
                        kasfEntrySetValue(90872430, 1);
                    }
                }
                else
                {
                    kasfEntrySetValue(90872430, 1);
                }
            }
            break;
        case 1:
            if (!(kasfEntryValueGet(-1912498034) & 2) || kasfEntryValueGet(-1912498034) == 15)
            {
                if (kasfShipsSelectClass(attackTargets, kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "CLASS_HeavyCruiser"))
                {
                    targetIndex = kasfRandom(0, kasfShipsCount(attackTargets) - 1);
                    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), attackTargets, targetIndex);
                    kasLabelledVectorSet((char *)0x058A20, kasShipsVectorPtrIfExists(218060896));
                    if ((sdword)kasfFindDistance(kasVectorPtrIfExists((char *)363040), kasShipsVectorPtrIfExists((char *)21541)) >= 36000)
                    {
                        if (kasfEntryValueGet(-1912498034) != 15)
                        {
                            kasfEntrySetValue(111786462, 2);
                            kasfEntrySetValue(-1912498034, kasfEntryValueGet(-1912498034) + 2);
                        }
                    }
                    else
                    {
                        kasfEntrySetValue(90872430, 1);
                    }
                }
                else
                {
                    kasfEntrySetValue(90872430, 1);
                }
            }
            break;
        case 2:
            if (!(kasfEntryValueGet(-1912498034) & 4) || kasfEntryValueGet(-1912498034) == 15)
            {
                if (kasfShipsSelectClass(attackTargets, kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "CLASS_Carrier"))
                {
                    targetIndex = kasfRandom(0, kasfShipsCount(attackTargets) - 1);
                    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), attackTargets, targetIndex);
                    kasLabelledVectorSet((char *)0x058A20, kasShipsVectorPtrIfExists(218060896));
                    if ((sdword)kasfFindDistance(kasVectorPtrIfExists((char *)363040), kasShipsVectorPtrIfExists((char *)21541)) >= 36000)
                    {
                        if (kasfEntryValueGet(-1912498034) != 15)
                        {
                            kasfEntrySetValue(111786462, 4);
                            kasfEntrySetValue(-1912498034, kasfEntryValueGet(-1912498034) + 4);
                        }
                    }
                    else
                    {
                        kasfEntrySetValue(90872430, 1);
                    }
                }
                else
                {
                    kasfEntrySetValue(90872430, 1);
                }
            }
            break;
        case 3:
            if (!(kasfEntryValueGet(-1912498034) & 8) || kasfEntryValueGet(-1912498034) == 15)
            {
                if (kasfShipsSelectCapital(attackTargets, kasGrowSelectionPtr(((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash)))
                {
                    targetIndex = kasfRandom(0, kasfShipsCount(attackTargets) - 1);
                    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)21541), attackTargets, targetIndex);
                    kasLabelledVectorSet((char *)0x058A20, kasShipsVectorPtrIfExists(218060896));
                    if ((sdword)kasfFindDistance(kasVectorPtrIfExists((char *)363040), kasShipsVectorPtrIfExists((char *)21541)) >= 36000)
                    {
                        if (kasfEntryValueGet(-1912498034) != 15)
                        {
                            kasfEntrySetValue(111786462, 8);
                            kasfEntrySetValue(-1912498034, kasfEntryValueGet(-1912498034) + 8);
                        }
                    }
                    else
                    {
                        kasfEntrySetValue(90872430, 1);
                    }
                }
                else
                {
                    kasfEntrySetValue(90872430, 1);
                }
            }
            break;
    }

    if (kasfShipsCount(attackTargets))
    {
        kasfLabelledVectorSet((char *)0x0AC53AB4, 0, 0, 0);
        kasfLabelledVectorSet((char *)0x08AC2922, 0, 0, 0);
        kasLabelledVectorSet((char *)0x058A20, kasShipsVectorPtrIfExists(218060896));
        kasfVectorBlendClampToUniverse(
            kasVectorPtrIfExists((char *)180697780),
            kasVectorPtrIfExists((char *)363040),
            kasShipsVectorPtrIfExists((char *)21541),
            1,
            2);
        kasfVectorRandomizeInRange(
            kasVectorPtrIfExists((char *)145500450),
            kasVectorPtrIfExists((char *)180697780),
            8000);
        targetPoint = kasVectorPtrIfExists((char *)145500450);
        kasfMoveTo(targetPoint);
        kasTimerCreateSetStartById(0x04C9664, kasfRandom(300, 600));
    }
    else
    {
        kasfEntrySetValue(90872430, 1);
    }
}

void Watch_Mission17_FinalDFGFrigateCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(90872430))
    {
        kasfSpeechEvent(
            0x04C9664,
            (sdword)Begin_Mission17_FinalDFGFrigateCombatLoop,
            (sdword)Watch_Mission17_FinalDFGFrigateCombatLoop);
    }
    else if (kasTimerExpiredDestroyById(0x04C9664) || kasfNearby(kasShipsVectorPtrIfExists(218060896), 30000))
    {
        kasTimerDestroyById(0x04C9664);
        kasfSpeechEvent(
            0x04C9664,
            (sdword)Begin_Mission17_FinalDFGFrigateCombatLoop,
            (sdword)Watch_Mission17_FinalDFGFrigateCombatLoop);
    }
}

void Watch_Mission17_FinalDFGFrigateDockAndReinforce(void)
{
    AITeam *team;
    sdword buildCountMode;
    sdword buildCost;
    sdword desiredBuildCount;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        buildCountMode = kasfEntryValueGet(-2126662828);
        desiredBuildCount = 1 - kasfTeamCount();
        if (buildCountMode)
        {
            if (buildCountMode == 1)
            {
                if (kasfEntryValueGet(154639956) < desiredBuildCount)
                {
                    buildCost = kasfShipTypeBuildCost("bDFGFrigate") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) > 1 &&
                     kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate") / 2) ||
                    kasfEntryValueGet(154639956) == 1)
                {
                    kasTimerCreateSetStartById(
                        133682484,
                        kasfShipTypeBuildTime("bDFGFrigate") * kasfEntryValueGet(154639956) / 2);
                }
            }
            else if (buildCountMode == 2)
            {
                if (kasfEntryValueGet(154639956) < desiredBuildCount)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 1)
                {
                    kasTimerCreateSetStartById(133682484, 0);
                }
            }
        }
        else
        {
            if (kasfEntryValueGet(154639956) < desiredBuildCount)
            {
                buildCost = kasfShipTypeBuildCost("bDFGFrigate");
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 1 &&
                 kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate")) ||
                kasfEntryValueGet(154639956) == 1)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfShipTypeBuildTime("bDFGFrigate") * kasfEntryValueGet(154639956));
            }
        }

        if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
        {
            kasfShipsSelectLinkedWith(&team->shipList, kasGrowSelectionPtr((char *)316548), kasGrowSelectionPtr(14701123));
        }
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
    }
}

void Begin_Mission17_FinalAssaultHyperspaceIn(void)
{
    if (!kasfEntryValueGet(71552172) && kasfShipsCount(kasGrowSelectionPtr((char *)14527169)))
    {
        kasfEntryGetOrCreateSetValue(71552172, 1);
        kasfShipsHyperspaceIn(kasGrowSelectionPtr((char *)14527169), NULL);
    }
    if (!kasfEntryValueGet(71552173) && kasfShipsCount(kasGrowSelectionPtr((char *)14527170)))
    {
        kasfEntryGetOrCreateSetValue(71552173, 1);
        kasfShipsHyperspaceIn(kasGrowSelectionPtr((char *)14527170), NULL);
    }
    if (!kasfEntryValueGet(71552174) && kasfShipsCount(kasGrowSelectionPtr((char *)14527171)))
    {
        kasfEntryGetOrCreateSetValue(71552174, 1);
        kasfShipsHyperspaceIn(kasGrowSelectionPtr((char *)14527171), NULL);
    }
    if (!kasfEntryValueGet(71552175) && kasfShipsCount(kasGrowSelectionPtr((char *)14527172)))
    {
        kasfEntryGetOrCreateSetValue(71552175, 1);
        kasfShipsHyperspaceIn(kasGrowSelectionPtr((char *)14527172), NULL);
    }
}

void Init_Mission17_FinalAmbushTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    kasfHideShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
    kasfShipsMakeCrazy(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
}

void Watch_Mission17_FinalAmbushTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(133880113)))
        {
            kasfUnhideShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
            kasfMsgSend(kasAITeamPtrIfExists(133880114), (char *)5801588);
            kasfFormationDelta();
            tutDeallocateRootRegion((sdword)kasAITeamShipsPtrIfExists(108434753));
        }
        else
        {
            kasfFormationDelta();
            tutDeallocateRootRegion((sdword)kasAITeamShipsPtrIfExists(185123649));
        }
    }
}

void Begin_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun(void)
{
    GrowSelection *combatTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    combatTargets = kasGrowSelectionPtr((char *)0x093C9634);
    kasfGrowSelectionClear(combatTargets);
    kasfShipsAdd(combatTargets, kasGrowSelectionPtr((char *)0x09C2E0E4));
    kasfShipsAdd(combatTargets, kasGrowSelectionPtr((char *)0x0E05243));
    kasfShipsRemove(combatTargets, &((AITeam *)kasCurrentTeamPtr)->shipList);
    if (!kasfShipsCount(combatTargets))
    {
        kasFSMCreate(
            (char *)0x0BE76893,
            Init_Mission17_BeastMultiBeamFrigateReinforcementTeam,
            Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
            (AITeam *)kasCurrentTeamPtr);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    kasfShipsSortByDistanceToPoint(combatTargets, ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x05425), combatTargets, 0);
    kasfProtect(kasGrowSelectionPtr((char *)0x05425));
}

void Watch_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if ((kasfShipsOrderAttributes(&((AITeam *)kasCurrentTeamPtr)->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x04C9664,
            (sdword)Begin_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun,
            (sdword)Watch_Mission17_BeastMultiBeamFrigateCombatTeamAttackRun);
    }
}










sdword SetMission17NaggarokTargetable(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    return kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
}

void Init_FleetIntel_SelectInvestigationTarget(void)
{
    GrowSelection *candidateShips = kasGrowSelectionPtr(57245931);
    GrowSelection *teamShips = kasGrowSelectionPtr(0x4A2A29);
    hvector *investigationPoint;
    vector targetPoint;
    real32 bestDistanceSquared;
    sdword bestIndex;
    sdword i;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    kasfGrowSelectionClear(candidateShips);

    if (!kasfShipsCount(candidateShips) && kasfEntryValueGet(187416633))
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(75774793));
    }

    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectType(candidateShips, teamShips, "sProcessor");
    }

    if (!kasfShipsCount(candidateShips) && kasfRandom(1, 2) == 1)
    {
        kasfShipsSelectType(candidateShips, teamShips, "sWorker");
        if (kasfShipsCount(candidateShips) <= 1)
        {
            kasfGrowSelectionClear(candidateShips);
        }
    }

    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsSelectCapital(candidateShips, teamShips);
        kasfShipsRemove(candidateShips, kasGrowSelectionPtr(75774793));
        kasfShipsRemove(candidateShips, kasGrowSelectionPtr(211966048));
    }

    if (!kasfShipsCount(candidateShips) && kasfRandom(1, 3) != 1)
    {
        kasfShipsAdd(candidateShips, kasGrowSelectionPtr(211966048));
    }

    if (!kasfShipsCount(candidateShips))
    {
        kasfShipsAdd(candidateShips, teamShips);
    }

    investigationPoint = kasShipsVectorPtr((char *)0x4A2A29);
    if (candidateShips->selection->numShips > 1)
    {
        vecGrabVecFromHVec(targetPoint, *investigationPoint);
        bestDistanceSquared = REALlyBig;
        bestIndex = 0;
        for (i = 0; i < candidateShips->selection->numShips; i++)
        {
            real32 distanceSquared = aiuFindDistanceSquared(
                candidateShips->selection->ShipPtr[i]->posinfo.position,
                targetPoint);
            if (distanceSquared < bestDistanceSquared)
            {
                bestDistanceSquared = distanceSquared;
                bestIndex = i;
            }
        }

        if (bestIndex != 0)
        {
            ShipPtr ship = candidateShips->selection->ShipPtr[0];
            candidateShips->selection->ShipPtr[0] = candidateShips->selection->ShipPtr[bestIndex];
            candidateShips->selection->ShipPtr[bestIndex] = ship;
        }
    }
    if (candidateShips->selection->numShips > 0)
    {
        candidateShips->selection->numShips = 1;
    }
    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_FleetIntel_InvestigateTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28)) || kasTimerExpiredDestroyById(141340587))
        {
            kasTimerDestroyById(141340587);
            kasfSpeechEvent(
                74024315,
                (sdword)Init_FleetIntel_SelectInvestigationTarget,
                (sdword)Watch_FleetIntel_InvestigateTarget);
        }
    }
    else
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_FleetIntel_CruiseMissileWaveDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(128820994, 15);
}

void Watch_FleetIntel_CruiseMissileWaveDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(128820994))
    {
        kasfSpeechEvent(
            187845043,
            (sdword)Init_FleetIntel_CruiseMissileWaveTimer,
            (sdword)Watch_FleetIntel_CruiseMissileWaveSpawner);
    }
}

void Init_FleetIntel_CruiseMissileWaveTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(172555554);
    kasTimerCreateById(172555554);
    kasTimerSetById(172555554, kasfRandom(15, 35));
    kasTimerStartById(172555554);
}

void Watch_FleetIntel_CruiseMissileWaveSpawner(void)
{
    AITeam *missileTeam;
    GrowSelection *cruiseMissiles;
    GrowSelection *teamShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

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
            (sdword)Init_FleetIntel_CruiseMissileWaveTimer,
            (sdword)Watch_FleetIntel_CruiseMissileWaveSpawner);
        return;
    }

    cruiseMissiles = kasGrowSelectionPtr(248876211);
    teamShips = (GrowSelection *)(kasCurrentTeamPtr + 28);
    kasfShipsCreateInside(cruiseMissiles, "bCruiseMissile", 2, teamShips, 1);

    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208097)))
    {
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
                kasfShipsAdd(kasGrowSelectionPtr(103856424), teamShips);
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
        missileTeam = kasAITeamPtrIfExists(73208098);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) >= 1)
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
        else
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208099)))
    {
        missileTeam = kasAITeamPtrIfExists(73208099);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) >= 1)
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
        else
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208100)))
    {
        missileTeam = kasAITeamPtrIfExists(73208100);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) >= 1)
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
        else
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
    }
    else if (!kasfShipsCount(kasAITeamShipsPtrIfExists(73208101)))
    {
        missileTeam = kasAITeamPtrIfExists(73208101);
        kasfReinforceTeamWithShips(missileTeam, cruiseMissiles);
        if (kasfEntryValueGet(249494211) >= 1)
        {
            kasfMsgSend(missileTeam, (char *)70903842);
        }
        else
        {
            kasfEntryIncrement(249494211);
            kasfMsgSend(missileTeam, (char *)187416633);
        }
    }

    kasfSpeechEvent(
        187845043,
        (sdword)Init_FleetIntel_CruiseMissileWaveTimer,
        (sdword)Watch_FleetIntel_CruiseMissileWaveSpawner);
}

void Begin_Mission17_BeastSentinelReinforcementCombatLoop(void)
{
    AITeam *team;
    GrowSelection *teamShips;
    GrowSelection *priorityTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    teamShips = kasGrowSelectionPtr(team->kasFSMNameHash);
    kasfShipsSelectCapital(teamShips, teamShips);
    kasfShipsSelectRace(teamShips, teamShips, 1);
    if (kasfShipsSelectType(kasGrowSelectionPtr(0xA38A5EC), teamShips, "bSentinel"))
    {
        if (kasfShipsSelectTargets(kasGrowSelectionPtr(0x9668AE7), kasGrowSelectionPtr(0xA38A5EC)))
        {
            kasfShipsRemove(teamShips, kasGrowSelectionPtr(0x9668AE7));
        }
    }

    kasfShipsRemove(teamShips, kasGrowSelectionPtr(0x8D9BC74));
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(22630782)))
    {
        if (kasfShipsSelectNearby(kasGrowSelectionPtr(0x5296679), teamShips, kasTeamsVectorPtr(22630782), 50000))
        {
            kasfShipsRemove(teamShips, kasGrowSelectionPtr(0x5296679));
        }
    }

    if (kasfShipsSelectType(kasGrowSelectionPtr(0x90A9E15), teamShips, "bIonArrayFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(0x90A9E15));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(0xD4C5155), teamShips, "bRammingFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(0xD4C5155));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(0xB693C35), teamShips, "bDFGFrigate"))
    {
        kasfShipsRemove(teamShips, kasGrowSelectionPtr(0xB693C35));
    }

    if (!kasfShipsCount(teamShips))
    {
        if (!kasfEntryValueGet(0x506A278))
        {
            kasfSpeechEvent(
                0x05B5E4,
                (sdword)Begin_SentinelCombatRetryDelay,
                (sdword)Watch_Mission05_OEM_MissionFlow_DontGoThere);
        }
        return;
    }

    kasfTacticsNeutral();
    kasfFormationWall();
    kasfShipsSelectNearby(kasGrowSelectionPtr(0x5296679), teamShips, kasThisTeamsVectorPtr(), 20000);
    kasfShipsSelectType(kasGrowSelectionPtr(0xCB08C72), kasGrowSelectionPtr(0x5296679), "bHeavyCruiser");
    kasfShipsSelectType(kasGrowSelectionPtr(0x7676DE2), kasGrowSelectionPtr(0x5296679), "bCarrier");
    kasfShipsSelectType(kasGrowSelectionPtr(0xCE05860), kasGrowSelectionPtr(0x5296679), "bMothership");

    if (!kasfShipsCount(kasGrowSelectionPtr(0xCB08C72))
     && !kasfShipsCount(kasGrowSelectionPtr(0x7676DE2))
     && !kasfShipsCount(kasGrowSelectionPtr(0xCE05860)))
    {
        kasfShipsSortByDistanceToPoint(teamShips, (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(0x93C9634), teamShips, 0);
    }
    else
    {
        priorityTargets = kasGrowSelectionPtr(0x77A2AE4);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0xCB08C72));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0x7676DE2));
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0xCE05860));
        kasfShipsSortByDistanceToPoint(priorityTargets, (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectIndex(kasGrowSelectionPtr(0x93C9634), priorityTargets, 0);
    }

    kasfProtect(kasGrowSelectionPtr(0x93C9634));
    kasfEntryScopeName(0x6A69AD4);
    kasfEntryScopeName(0x2EFD564);
    kasfEntryScopeName(0x79E6E64);
}

void Init_BeastSentinelReinforcementTeam(void)
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
            kasfSpeechEvent(76130852, (sdword)Begin_Mission17_BeastSentinelReinforcementCombatLoop, (sdword)Watch_Mission17_BeastSentinelAssault);
        }
        else
        {
            kasfSpeechEvent(
                0x479E04,
                (sdword)sub_65F6F0,
                (sdword)Watch_BeastSentinelReinforcementTeam);
        }
    }
}

void Init_BeastSentinelReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(171501548))
     && kasfEntryValueGet(-2031143291)
     && kasfMsgReceived((char *)0x479E04))
    {
        kasfEntrySetValue(-2031143291, 0);
        kasfEntrySetValue(-1884903716, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)190169075);
        kasfSpeechEvent(
            0x479E04,
            (sdword)sub_65F6F0,
            (sdword)Watch_BeastSentinelReinforcementTeam);
    }
}

void Watch_BeastSentinelReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedSentinels;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedSentinels = kasGrowSelectionPtr(0x58A20);
    if (kasfShipsSelectType(returnedSentinels, kasGrowSelectionPtr(0x2A98C74), "bSentinel") < 3)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
        {
            if (kasfEntryValueGet(-1884903716))
            {
                kasfEntrySetValue(-1884903716, 0);
                kasfEntrySetValue(-2031143291, 1);
                kasfSpeechEvent(
                    0x09E08A57,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Watch_BeastSentinelReinforcementBuildCount);
            }
        }
        else
        {
            kasfEntrySetValue(-1884903716, 0);
            kasfEntrySetValue(-2031143291, 1);
            kasfSpeechEvent(
                0x053A0C,
                (sdword)Init_Mission09_AssaultEscort_Cooldown,
                (sdword)sub_652EF0);
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
    kasfSpeechEvent(76130852, (sdword)Begin_Mission17_BeastSentinelReinforcementCombatLoop, (sdword)Watch_Mission17_BeastSentinelAssault);
}

void Watch_BeastSentinelReinforcementBuildCount(void)
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
            kasfSpeechEvent(
                0x04869685,
                (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
                (sdword)Watch_BeastSentinelReinforcementSpawn);
            return;
        }
    }
    else if (buildStyle == 2)
    {
        if (kasfEntryValueGet(154639956) < 12)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) != 12)
        {
            return;
        }
        kasfSpeechEvent(
            0x04869685,
            (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
            (sdword)Watch_BeastSentinelReinforcementSpawn);
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
            0x04869685,
            (sdword)Init_Mission11_SentinelReinforcementBuildTimer,
            (sdword)Watch_BeastSentinelReinforcementSpawn);
    }
}

void Watch_BeastSentinelReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *spawnParents;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    spawnParents = kasGrowSelectionPtr(0xE05243);
    if (!kasfShipsCount(spawnParents))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(
            0x053A0C,
            (sdword)Init_Mission09_AssaultEscort_Cooldown,
            (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        kasfShipsSelectIndex(kasGrowSelectionPtr(0x5425), spawnParents, kasfRandom(0, kasfShipsCount(spawnParents) - 1));
        kasfShipsCreateInside(kasGrowSelectionPtr(0x9379E54), "bSentinel", kasfEntryValueGet(154639956), (sdword)kasGrowSelectionPtr(0x5425), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(0x9379E54), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(0x9379E54));
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(76130852, (sdword)Begin_Mission17_BeastSentinelReinforcementCombatLoop, (sdword)Watch_Mission17_BeastSentinelAssault);
    }
}

void Watch_Mission17_BeastSentinelAssault(void)
{
    AITeam *team;
    sdword requestedCount;
    sdword totalCost;
    sdword randomParentIndex;
    GrowSelection *spawnParents;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    if (kasfShipsSelectNearby(
            kasGrowSelectionPtr(0x5296679),
            kasAITeamShipsPtrIfExists(22630782),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            50000)
     && !kasfEntryValueGet(84320888))
    {
        kasfStop();
        kasfSpeechEvent(76130852, (sdword)Begin_Mission17_BeastSentinelReinforcementCombatLoop, (sdword)Watch_Mission17_BeastSentinelAssault);
        return;
    }

    if (kasfTeamFinishedLaunching())
    {
        if (!kasfEntryValueGet(127823460))
        {
            if ((kasfShipsOrderAttributes(&team->shipList) & 2) != 0)
            {
                kasfShipsSetFormation(&team->shipList, 10);
                kasfEntrySetValue(127823460, 1);
            }
        }
        else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0 && !kasfEntryValueGet(84320888))
        {
            kasfEntrySetValue(127823460, 0);
            kasfStop();
            kasfSpeechEvent(
                0x05B5E4,
                (sdword)Begin_SentinelCombatRetryDelay,
                (sdword)Watch_Mission05_OEM_MissionFlow_DontGoThere);
            return;
        }
    }

    if (!kasfEntryValueGet(111581908))
    {
        if (kasfTeamCount() < 12)
        {
            kasfEntrySetValue(111592388, 12 - kasfTeamCount());
            kasfEntrySetValue(111581908, 1);
        }
    }
    else if (!kasfEntryValueGet(49272164))
    {
        kasfEntrySetValue(49272164, 1);
        kasTimerCreateSetStartById(111581908, kasfEntryValueGet(111592388) * kasfShipTypeBuildTime("bSentinel"));
    }

    if (kasfEntryValueGet(-2126662828) == 0)
    {
        kasfEntryGetOrCreateSetValue(235422484, kasfShipTypeBuildCost("bSentinel"));
    }
    else if (kasfEntryValueGet(-2126662828) == 1)
    {
        kasfEntrySetValue(235422484, kasfShipTypeBuildCost("bSentinel"));
    }
    else if (kasfEntryValueGet(-2126662828) == 2)
    {
        kasfEntrySetValue(235422484, 0);
    }

    requestedCount = kasfEntryValueGet(111592388);
    totalCost = requestedCount * kasfEntryValueGet(235422484);
    spawnParents = kasGrowSelectionPtr(0xE05243);
    if (kasfRUsGet(1) >= totalCost && kasTimerExpiredById(111581908) && kasfShipsCount(spawnParents))
    {
        kasfRUsSet(1, kasfRUsGet(1) - totalCost);
        randomParentIndex = kasfRandom(0, kasfShipsCount(spawnParents) - 1);
        kasfShipsSelectIndex(kasGrowSelectionPtr(0x5425), spawnParents, randomParentIndex);
        if (kasfTeamCount() + kasfEntryValueGet(111592388) >= 12)
        {
            kasfEntrySetValue(111592388, 12 - kasfTeamCount());
        }
        kasfShipsCreateInside(kasGrowSelectionPtr(0x9379E54), "bSentinel", kasfEntryValueGet(111592388), kasGrowSelectionPtr(0x5425), 1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(0x9379E54), 1);
        kasfShipsAdd(&team->shipList, kasGrowSelectionPtr(0x9379E54));
        kasfEntrySetValue(84320888, 1);
    }

    if (kasfEntryValueGet(84320888) && kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)111734707);
        kasfEntrySetValue(84320888, 0);
        kasfEntrySetValue(111581908, 0);
        kasfEntrySetValue(49272164, 0);
        kasfShipsSelectTargets(&team->shipList, kasGrowSelectionPtr(0x9668AE7));
        kasfProtect(kasGrowSelectionPtr(0x9668AE7));
        kasfEntrySetValue(127823460, 0);
    }
}

void Begin_FleetIntel_CruiseMissileWaveSetup(void)
{
    static const sdword fleetIntelWaveTeams[] = {
        196554993,
        196554994,
        196554995,
        196554996,
        196554997,
        196554998,
        196554999,
        196555000,
        196555001,
        196554944,
        196554945,
        196554946,
    };
    static const sdword fleetIntelWaveVectors[] = {
        0x4D411,
        0x4D412,
        0x4D413,
        0x4D414,
        0x4D415,
        0x4D416,
        0x4D417,
        0x4D418,
        0x4D419,
        0x4D420,
        0x4D421,
        0x4D422,
    };
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount() <= 0)
    {
        return;
    }

    kasfShipsAdd(kasGrowSelectionPtr(0x89F03C3), &team->shipList);
    for (i = 0; i < sizeof(fleetIntelWaveTeams) / sizeof(fleetIntelWaveTeams[0]); i++)
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(fleetIntelWaveTeams[i])))
        {
            kasfLabelledVectorSet(fleetIntelWaveVectors[i], 0, 0, 0);
            kasfVectorRandomizeInRange(
                kasVectorPtrIfExists(fleetIntelWaveVectors[i]),
                kasShipsVectorPtr(0xBB8),
                0xBB8);
            kasfMoveTo(kasVectorPtrIfExists(fleetIntelWaveVectors[i]));
            break;
        }
    }

    kasfSpeechEvent(5532277, (sdword)Init_FleetIntel_CruiseMissileWaveDelay, 0);
}


















void Begin_Mission17_BeastHeavyCruiserCombatTeam_SpecialAttack(void)
{
    GrowSelection *specialAttackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(0x0C9B3857) &&
        kasfShipsSelectIndex(
            kasGrowSelectionPtr((char *)0x05425),
            kasGrowSelectionPtr((char *)0x0E05243),
            randyrandombetween(
                RAN_AIPlayer,
                0,
                kasfShipsCount(kasGrowSelectionPtr((char *)0x0E05243)) - 1)))
    {
        kasfEntrySetValue(0x0C9B3857, 1);
        specialAttackTargets = kasGrowSelectionPtr((char *)0x0716C77E);
        kasfShipsSetHyperspaceDestinationToCurrentPosition(specialAttackTargets);
        kasfShipsAdd(specialAttackTargets, &((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x05425), specialAttackTargets);
        kasfShipsSetMilitaryParade(kasGrowSelectionPtr((char *)0x05425), specialAttackTargets);
        kasfTeamAttributesBitSet(0x80);
        kasfAttackMothership();
        kasfShipsSetHyperspaceDestinationToCurrentPosition(&((AITeam *)kasCurrentTeamPtr)->shipList);
        kasTimerCreateSetStartById(0x0165C5B5, 14);
    }
}

void Watch_Mission17_BeastHeavyCruiserCombatTeam_SpecialAttack(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(0x0C9B3857) && kasTimerExpiredById(0x0165C5B5))
    {
        kasfEntrySetValue(0x05137B, 0);
        kasTimerSetById(0x0883A004, 20);
        kasTimerStartById(0x0883A004);
        kasfMsgSend(kasAITeamPtrIfExists(0x00469684), (char *)0x04CC48B4);
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
    }
}

void Begin_Mission17_BeastHeavyCruiserCombatTeam_AttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfLabelledVectorSet(
        (char *)0x07A79EA4,
        kasfEntryValueGet(88),
        kasfEntryValueGet(89),
        kasfEntryValueGet(90));
    if (kasfFindDistance(
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            kasVectorPtrIfExists((char *)0x07A79EA4)) >= 25000.0f)
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
        return;
    }
    kasfEntrySetValue(0x05137B, 1);
    kasfShipsSortByDistanceToPoint(
        kasGrowSelectionPtr((char *)0x036980EB),
        ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfFormationWall();
    kasfShipsSetResourcingFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    if (kasfShipsSelectCapital(
            kasGrowSelectionPtr((char *)0x0764E81C),
            kasGrowSelectionPtr((char *)0x036980EB)))
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x0764E81C));
    }
    else
    {
        kasfTeamSpecial(kasGrowSelectionPtr((char *)0x036980EB));
    }
    kasfShipsSetResourcingFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
}

void Watch_Mission17_BeastHeavyCruiserCombatTeam_AttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasTimerSetById(0x0883A004, 20);
        kasTimerStartById(0x0883A004);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission17_BeastHeavyCruiserCombatTeam);
        return;
    }
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x085DD80E),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            12500.0f))
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
        return;
    }
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasTimerSetById(0x0883A004, 20);
        kasTimerStartById(0x0883A004);
        kasfEntrySetValue(0x05137B, 0);
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
    }
}

void sub_663CE0(void)
{
    GrowSelection *targetSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x04A2A29),
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            25000.0f))
    {
        kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x04A2A29),
            kasGrowSelectionPtr((char *)0x036980EB));
    }
    if (kasfFindShipsTargetingSelection(
            kasGrowSelectionPtr((char *)0x0A79EA22),
            kasGrowSelectionPtr((char *)0x0857BCE3)))
    {
        kasfShipsAdd(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x0857BCE3));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x0857BCE3)))
    {
        kasfShipsAdd(
            kasGrowSelectionPtr((char *)0x036980EB),
            kasGrowSelectionPtr((char *)0x0857BCE3));
    }
    kasfShipsSortByDistanceToPoint(
        kasGrowSelectionPtr((char *)0x036980EB),
        ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfFormationWall();
    if (kasfShipsSelectCapital(
            kasGrowSelectionPtr((char *)0x0764E81C),
            kasGrowSelectionPtr((char *)0x036980EB)))
    {
        targetSelection = kasGrowSelectionPtr((char *)0x0764E81C);
    }
    else
    {
        targetSelection = kasGrowSelectionPtr((char *)0x036980EB);
    }
    tutDeallocateRootRegion(targetSelection);
    kasTimerDestroyById(0x0469857B);
    kasTimerCreateSetStartById(0x0469857B, randyrandombetween(RAN_AIPlayer, 10, 30));
}

void sub_663E60(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(0x0469857B))
    {
        kasfSpeechEvent(74024315, (sdword)sub_663CE0, (sdword)sub_663E60);
        return;
    }
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) ||
        kasfFindDistance(
            ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
            kasVectorPtrIfExists((char *)0x07A79EA4)) >= 25000.0f)
    {
        kasfSpeechEvent(
            5019236,
            (sdword)Begin_Mission17_BeastHeavyCruiserAttackRetryDelay,
            (sdword)Watch_Mission17_BeastHeavyCruiserAttackRetryDelay);
    }
}

void Init_Mission17_FinalAssaultLeadTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x088F60C4), &team->shipList);
    kasfTeamAttributesBitSet(128);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(217111973)))
    {
        kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0CF0DDA5), &team->shipList);
    }
}

void Watch_Mission17_FinalAssaultLeadTeamLaunchDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(-1933726299))
    {
        kasfMsgSend(kasAITeamPtrIfExists(217111973), (char *)157864733);
    }
}

void Watch_Mission17_FinalAssaultLeadTeam(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    if (kasfThisTeamIs(kasAITeamPtrIfExists(217111973)))
    {
        queueInfo = *((sdword **)team + 20);
        queueIndex = -1;
        if (queueInfo != NULL)
        {
            queueIndex = *queueInfo;
        }

        while (queueIndex >= 0)
        {
            message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
            if (message != NULL)
            {
                if (message == (void *)5801588)
                {
                    kasfEntryGetOrCreateSetValue(-1933726299, 1);
                    kasTimerCreateSetStartById(-1933726299, 10);
                }
                else if (message == (void *)157864733)
                {
                    kasfShipsAdd(kasGrowSelectionPtr(14701123), kasAITeamShipsPtrIfExists(108453443));
                    kasFSMCreate(
                        (char *)0x0B08C7B3,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(185124854));
                    kasFSMCreate(
                        (char *)0x0B08C7B3,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(185124855));
                    kasfShipsSetHyperspaceDestination(
                        kasGrowSelectionPtr(217111973),
                        kasVectorPtrIfExists(128206740));
                    kasFSMCreate(
                        (char *)0x0EE25AC3,
                        Init_Mission17_BeastCloakedFighterReinforcementTeam,
                        Init_Mission17_BeastCloakedFighterReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(249715329));
                    kasFSMCreate(
                        (char *)0x0BE76893,
                        Init_Mission17_BeastMultiBeamFrigateReinforcementTeam,
                        Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(199714900));
                    kasFSMCreate(
                        (char *)0x0215C113,
                        Init_BeastWorkerHarvestReinforcementTeam,
                        Watch_BeastWorkerHarvestReinforcementTeam,
                        kasAITeamPtrIfExists(34980306));
                    kasFSMCreate(
                        (char *)0x038E7FB3,
                        Init_BeastSentinelReinforcementTeam,
                        Init_BeastSentinelReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(59670134));
                    kasFSMCreate(
                        (char *)0x038E7FB3,
                        Init_BeastSentinelReinforcementTeam,
                        Init_BeastSentinelReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(59670135));
                    kasFSMCreate(
                        (char *)0x0676DE03,
                        Init_Mission17_BeastCarrierReinforcementTeam,
                        Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(108453443));
                    kasFSMCreate(
                        (char *)0x00458B3,
                        Init_Mission17_BeastACVReinforcementWave,
                        Init_Mission17_BeastACVReinforcementWave_Reevaluate,
                        kasAITeamPtrIfExists(284788));
                    kasFSMCreate(
                        (char *)0x04C51573,
                        Init_Mission17_BeastRammingFrigateReinforcementTeam,
                        Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(80025010));
                    kasFSMCreate(
                        (char *)0x04C51573,
                        Init_Mission17_BeastRammingFrigateReinforcementTeam,
                        Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(80025011));
                    kasFSMCreate(
                        (char *)0x04C51573,
                        Init_Mission17_BeastRammingFrigateReinforcementTeam,
                        Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(80025012));
                }
                else
                {
                    aitQueuePointer(team, team, message);
                }
            }

            queueInfo = *((sdword **)team + 20);
            if (queueInfo == NULL || *queueInfo == 0)
            {
                break;
            }
            queueIndex--;
        }
    }
    else if (!kasfEntryValueGet(71552169) && kasfEntryValueGet(-1933726299))
    {
        kasfEntryGetOrCreateSetValue(71552169, 1);
        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission17_FinalAssaultLeadTeamLaunchDelay);
    }
}

void Begin_Mission17_FinalAssaultLeadTeamHyperspaceIn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552170) && kasfShipsCount(kasGrowSelectionPtr((char *)0x0CF0DDA5)))
    {
        kasfEntryGetOrCreateSetValue(71552170, 1);
        ((void (__cdecl *)(GrowSelection *))kasfShipsHyperspaceIn)(kasGrowSelectionPtr((char *)0x0CF0DDA5));
    }
}

void Init_Mission17_FinalAssaultAssignWaveTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x088F60C4), &team->shipList);
    kasfTeamAttributesBitSet(128);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(252566178)))
    {
        kasfSpeechEvent(
            175938357,
            (sdword)MaintainTeamContext,
            (sdword)Begin_Mission17_FinalAssaultHyperspaceIn);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(185124852)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(59670136)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(0x0A9E1F3)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(34980307)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(128634915)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0DDAAC1), &team->shipList);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(9207697)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(34980311)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(149415701)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0DDAAC3), &team->shipList);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(9207698)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(34980312)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(149415702)))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0DDAAC4), &team->shipList);
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0DDAAC2), &team->shipList);
    }
}

sdword Begin_Mission17_FinalAssaultQueueTeamForWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(185124852)) ||
        kasfThisTeamIs(kasAITeamPtrIfExists(59670136)) ||
        kasfThisTeamIs(kasAITeamPtrIfExists(0x0A9E1F3)) ||
        kasfThisTeamIs(kasAITeamPtrIfExists(34980307)) ||
        kasfThisTeamIs(kasAITeamPtrIfExists(128634915)))
    {
        return kasfShipsAdd(kasGrowSelectionPtr((char *)0x04943D), &team->shipList);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(9207697)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(34980311)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(149415701)))
    {
        return kasfShipsAdd(kasGrowSelectionPtr((char *)0x04943E3), &team->shipList);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(9207698)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(34980312)) ||
             kasfThisTeamIs(kasAITeamPtrIfExists(149415702)))
    {
        return kasfShipsAdd(kasGrowSelectionPtr((char *)0x04943E4), &team->shipList);
    }
    else
    {
        return kasfShipsAdd(kasGrowSelectionPtr((char *)0x04943E2), &team->shipList);
    }
}

void Watch_Mission17_FinalAssaultWaveLaunchDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(-1948591454))
    {
        kasfMsgSend(kasAITeamPtrIfExists(252566178), (char *)157864733);
    }
}

void Watch_Mission17_FinalAssaultWaveController(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(252566178)))
    {
        queueInfo = *((sdword **)team + 20);
        queueIndex = -1;
        if (queueInfo != NULL)
        {
            queueIndex = *queueInfo;
        }

        while (queueIndex >= 0)
        {
            message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
            if (message != NULL)
            {
                if (message == (void *)5801588)
                {
                    kasfEntryGetOrCreateSetValue(-1948591454, 1);
                    kasfLabelledVectorSet((char *)0x04943D, 0, 0, 0);
                    kasfLabelledVectorSet((char *)0x04943E2, 0, 0, 0);
                    kasfVectorRandomizeInRange(
                        kasVectorPtrIfExists((char *)0x04943D),
                        kasShipsVectorPtrIfExists(218060896),
                        90000);
                    kasfVectorBlendClampToUniverse(
                        kasVectorPtrIfExists((char *)0x04943E2),
                        kasVectorPtrIfExists((char *)0x04943D),
                        kasShipsVectorPtrIfExists(218060896),
                        2,
                        1);
                    kasLabelledVectorSet((char *)0x04428EB0, kasVectorPtrIfExists((char *)0x04943D));
                    kasfVectorSubtractClampToUniverse(
                        kasVectorPtrIfExists((char *)0x04428EB0),
                        kasShipsVectorPtrIfExists(218060896));
                    kasfLabelledVectorSet((char *)0x0428EB52, 0, 0, 0);
                    kasfLabelledVectorSet((char *)0x0428EB53, 0, 0, 0);
                    kasfVectorCrossNormalize(
                        kasVectorPtrIfExists((char *)0x0428EB53),
                        kasVectorPtrIfExists((char *)0x04428EB0),
                        kasVectorPtrIfExists((char *)0x0428EB52));
                    kasfVectorScaleByRatio(kasVectorPtrIfExists((char *)0x0428EB53), 90000, 1);
                    kasfVectorAddClampToUniverse(
                        kasVectorPtrIfExists((char *)0x0428EB53),
                        kasShipsVectorPtrIfExists(218060896));
                    kasfLabelledVectorSet((char *)0x0428EB54, 0, 0, 0);
                    kasfVectorBlendClampToUniverse(
                        kasVectorPtrIfExists((char *)0x0428EB54),
                        kasVectorPtrIfExists((char *)0x0428EB53),
                        kasShipsVectorPtrIfExists(218060896),
                        2,
                        1);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x04943D);
                    kasTimerCreateSetStartById(-1948591454, 10);
                }
                else if (message == (void *)157864733)
                {
                    kasfShipsSetHyperspaceDestination(
                        kasGrowSelectionPtr((char *)0x04943D),
                        kasVectorPtrIfExists((char *)0x04943D));
                    kasfShipsSetHyperspaceDestination(
                        kasGrowSelectionPtr((char *)0x04943E2),
                        kasVectorPtrIfExists((char *)0x04943E2));
                    kasfShipsSetHyperspaceDestination(
                        kasGrowSelectionPtr((char *)0x04943E3),
                        kasVectorPtrIfExists((char *)0x0428EB53));
                    kasfShipsSetHyperspaceDestination(
                        kasGrowSelectionPtr((char *)0x04943E4),
                        kasVectorPtrIfExists((char *)0x0428EB54));
                    kasFSMCreate(
                        (char *)0x0B08C7B3,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(185124852));
                    kasFSMCreate(
                        (char *)0x0B08C7B3,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(185124853));
                    kasFSMCreate(
                        (char *)0x038E7FB3,
                        Init_BeastSentinelReinforcementTeam,
                        Init_BeastSentinelReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(59670136));
                    kasFSMCreate(
                        (char *)0x038E7FB3,
                        Init_BeastSentinelReinforcementTeam,
                        Init_BeastSentinelReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(59670137));
                    kasFSMCreate(
                        (char *)0x0A9E133,
                        Init_Mission17_BeastIonArrayFrigateReinforcementTeam,
                        Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(0x0A9E1F3));
                    kasFSMCreate(
                        (char *)0x0A9E133,
                        Init_Mission17_BeastIonArrayFrigateReinforcementTeam,
                        Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(0x0A9E1F4));
                    kasFSMCreate(
                        (char *)0x0215C113,
                        Init_Mission08_HiveFrigateReinforcementTeam,
                        Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(34980307));
                    kasFSMCreate(
                        (char *)0x0215C113,
                        Init_Mission08_HiveFrigateReinforcementTeam,
                        Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(34980308));
                    kasFSMCreate(
                        (char *)0x07AAD0E3,
                        Init_Mission17_BeastCorvetteReinforcementTeam,
                        Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(128634915));
                    kasFSMCreate(
                        (char *)0x07AAD0E3,
                        Init_Mission17_BeastCorvetteReinforcementTeam,
                        Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(128634916));
                    kasFSMCreate(
                        (char *)0x0B08C7B3,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(9207697));
                    kasFSMCreate(
                        (char *)0x0B08C7B3,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam,
                        Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(9207698));
                    kasFSMCreate(
                        (char *)0x0215C113,
                        Init_Mission08_HiveFrigateReinforcementTeam,
                        Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(34980311));
                    kasFSMCreate(
                        (char *)0x0215C113,
                        Init_Mission08_HiveFrigateReinforcementTeam,
                        Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(34980312));
                    kasFSMCreate(
                        (char *)0x038E7FB3,
                        Init_BeastSentinelReinforcementTeam,
                        Init_BeastSentinelReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(149415701));
                    kasFSMCreate(
                        (char *)0x038E7FB3,
                        Init_BeastSentinelReinforcementTeam,
                        Init_BeastSentinelReinforcementTeam_Reevaluate,
                        kasAITeamPtrIfExists(149415702));
                }
                else
                {
                    aitQueuePointer(team, team, message);
                }
            }

            queueInfo = *((sdword **)team + 20);
            if (queueInfo == NULL || *queueInfo == 0)
            {
                break;
            }
            queueIndex--;
        }
    }
    else if (!kasfEntryValueGet(71552171) && kasfEntryValueGet(-1948591454))
    {
        kasfEntryGetOrCreateSetValue(71552171, 1);
        kasfSpeechEvent(
            374244,
            (sdword)Begin_Mission17_FinalAssaultQueueTeamForWave,
            (sdword)Watch_Mission17_FinalAssaultWaveLaunchDelay);
    }
}

void Init_Mission17_BeastAdaptiveReinforcementController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(185124787, 0);
    kasfEntryGetOrCreateSetValue(108453379, 0);
    kasfEntryGetOrCreateSetValue(230526675, 0);
    kasfEntryGetOrCreateSetValue(22221523, 0);
    kasfEntryGetOrCreateSetValue(177484403, 0);
    kasfEntryGetOrCreateSetValue(0x0A6E863, 0);
    kasfEntryGetOrCreateSetValue(128635107, 0);
}

void Watch_Mission17_BeastAdaptiveReinforcementController(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, (udword)queueIndex);
        switch (message)
        {
            case 0x28020E3:
                kasfEntrySetValue(235370115, 0);
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfShipsSelectClass(
                    kasGrowSelectionPtr((char *)0x0484A34),
                    kasGrowSelectionPtr((char *)0x2BC6679),
                    "CLASS_Corvette");
                kasfEntrySetValue(
                    128635107,
                    kasfShipsSelectRace(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x0484A34),
                        2));
                break;
            case 0x39440D3:
            case 110149603:
                kasfEntrySetValue(3311779, 0);
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfEntrySetValue(
                    177484403,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x2BC6679),
                        "tStandardDestroyer"));
                break;
            case 0x2FB4203:
            case 103051523:
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfEntrySetValue(
                    108453379,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x2BC6679),
                        "tCarrier"));
                break;
            case 0x676ED73:
                kasfEntrySetValue(251658691, 0);
            case 191275187:
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfEntrySetValue(
                    0x0A6E863,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x2BC6679),
                        "tIonCannonFrigate"));
                break;
            case 135485987:
            case 188946227:
                kasfEntrySetValue(205094067, 0);
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfEntrySetValue(
                    22221523,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x2BC6679),
                        "tAdvanceSupportFrigate"));
                break;
            case 185462083:
            case 188751475:
                kasfEntrySetValue(23161363, 0);
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfEntrySetValue(
                    185124787,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x2BC6679),
                        "tHeavyCruiser"));
                break;
            case 154145491:
            case 233775571:
                kasfEntrySetValue(187126899, 0);
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfShipsSelectClass(
                    kasGrowSelectionPtr((char *)0x0484A34),
                    kasGrowSelectionPtr((char *)0x2BC6679),
                    "CLASS_Fighter");
                kasfEntrySetValue(
                    230526675,
                    kasfShipsSelectRace(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x0484A34),
                        2));
                break;
            case 259199139:
                kasfEntrySetValue(235370115, 0);
                kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)18336307));
                kasfShipsSelectClass(
                    kasGrowSelectionPtr((char *)0x0484A34),
                    kasGrowSelectionPtr((char *)0x2BC6679),
                    "CLASS_Corvette");
                kasfEntrySetValue(
                    128635107,
                    kasfShipsSelectRace(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)0x0484A34),
                        2));
                break;
            case 0:
                break;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }

    if (!kasfEntryValueGet(251658691) && kasfEntryValueGet(0x0A6E863) < 7 && (sdword)kasfRUsGet(3) > 3000)
    {
        kasfEntryGetOrCreateSetValue(251658691, 1);
        kasfMsgSend(kasAITeamPtrIfExists(205745797), (char *)4693508);
    }
    if (!kasfEntryValueGet(205094067) && kasfEntryValueGet(22221523) < 4 && (sdword)kasfRUsGet(3) > 1000)
    {
        kasfEntryGetOrCreateSetValue(205094067, 1);
        kasfMsgSend(kasAITeamPtrIfExists(122695973), (char *)4693508);
    }
    if (!kasfEntryValueGet(235370115) && kasfEntryValueGet(128635107) < 12 && (sdword)kasfRUsGet(3) > 2000)
    {
        kasfEntryGetOrCreateSetValue(235370115, 1);
        kasfMsgSend(kasAITeamPtrIfExists(75163141), (char *)4693508);
    }
    if (!kasfEntryValueGet(0x0B27542) && kasfEntryValueGet(230526675) < 24 && (sdword)kasfRUsGet(3) > 500)
    {
        kasfEntryGetOrCreateSetValue(187126899, 1);
        kasfMsgSend(kasAITeamPtrIfExists(182180946), (char *)4693508);
    }
}

void Init_Mission17_TaiidanAdaptiveReinforcementController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(185124787, 0);
    kasfEntryGetOrCreateSetValue(108453379, 0);
    kasfEntryGetOrCreateSetValue(22221523, 0);
    kasfEntryGetOrCreateSetValue(177484403, 0);
    kasfEntryGetOrCreateSetValue(0x0A6E863, 0);
    kasfEntryGetOrCreateSetValue(128635107, 0);
}

void Watch_Mission17_TaiidanAdaptiveReinforcementController(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, (udword)queueIndex);
        switch (message)
        {
            case 0x28020E3:
            case 259199139:
                kasfEntrySetValue(235370115, 0);
                kasfEntrySetValue(
                    128635107,
                    kasfShipsSelectClass(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)140367886),
                        "CLASS_Corvette"));
                break;
            case 0x39440D3:
            case 110149603:
                kasfEntrySetValue(3311779, 0);
                kasfEntrySetValue(
                    177484403,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)140367886),
                        "tStandardDestroyer"));
                break;
            case 0x2FB4203:
            case 103051523:
                kasfEntrySetValue(
                    108453379,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)140367886),
                        "tCarrier"));
                break;
            case 108457331:
                kasfEntrySetValue(251658691, 0);
            case 191275187:
                kasfEntrySetValue(
                    0x0A6E863,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)140367886),
                        "tIonCannonFrigate"));
                break;
            case 0x8135A23:
            case 0x0B431733:
                kasfEntrySetValue(205094067, 0);
                kasfEntrySetValue(
                    22221523,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)140367886),
                        "tAdvanceSupportFrigate"));
                break;
            case 0x0B0DED43:
            case 0x0B401E73:
                kasfEntrySetValue(23161363, 0);
                kasfEntrySetValue(
                    185124787,
                    kasfShipsSelectType(
                        kasGrowSelectionPtr((char *)0x0484A34),
                        kasGrowSelectionPtr((char *)140367886),
                        "tHeavyCruiser"));
                break;
            case 0:
                break;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }

    if (!kasfEntryValueGet(235370115) && kasfEntryValueGet(128635107) < 40 && kasfRUsGet(2) > 2000)
    {
        kasfEntryGetOrCreateSetValue(235370115, 1);
        kasfMsgSend(kasAITeamPtrIfExists(75165957), (char *)4693508);
    }
    if (kasfMsgReceived((char *)5598772))
    {
        kasfLogInteger("HVCruiser=%d", kasfEntryValueGet(185124787));
        kasfLogInteger("Carrier=%d", kasfEntryValueGet(108453379));
        kasfLogInteger("Destroyer=%d", kasfEntryValueGet(177484403));
        kasfLogInteger("Ion=%d", kasfEntryValueGet(0x0A6E863));
        kasfLogInteger("Support=%d", kasfEntryValueGet(22221523));
        kasfLogInteger("Corvette=%d", kasfEntryValueGet(128635107));
    }
}

static sdword sub_493460(GrowSelection *selection)
{
    sdword total;
    SelectCommand *shipSelection;
    sdword i;

    shipSelection = selection->selection;
    total = 0;
    if (shipSelection == NULL)
    {
        return total;
    }

    for (i = 0; i < shipSelection->numShips; i++)
    {
        total += *(sdword *)((ubyte *)shipSelection->ShipPtr[i] + 1324);
    }
    return total;
}

static void sub_493420(GrowSelection *selection, ubyte setBit)
{
    SelectCommand *shipSelection;
    sdword i;

    shipSelection = selection->selection;
    if (shipSelection == NULL)
    {
        return;
    }

    for (i = 0; i < shipSelection->numShips; i++)
    {
        udword *shipWords;

        shipWords = (udword *)shipSelection->ShipPtr[i];
        shipWords[115] = (((udword)(setBit & 1)) << 14) | (shipWords[115] & 0xFFFFBFFF);
    }
}

static void sub_499790(void)
{
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    aitDeleteAllTeamMoves(team);
    clWrapHalt(&universe.mainCommandLayer, team->shipList.selection);
    for (i = 0; i < team->shipList.selection->numShips; i++)
    {
        sdword *raw;

        raw = (sdword *)team->shipList.selection->ShipPtr[i];
        raw[0x64 / sizeof(sdword)] = 0;
        raw[0x68 / sizeof(sdword)] = 0;
        raw[0x6C / sizeof(sdword)] = 0;
        raw[0x70 / sizeof(sdword)] = 0;
        raw[0x74 / sizeof(sdword)] = 0;
        raw[0x78 / sizeof(sdword)] = 0;
        raw[0xBC / sizeof(sdword)] = 0;
        raw[0xC0 / sizeof(sdword)] = 0;
        raw[0xC4 / sizeof(sdword)] = 0;
        raw[0xC8 / sizeof(sdword)] = 0;
        raw[0xCC / sizeof(sdword)] = 0;
        raw[0xD0 / sizeof(sdword)] = 0;
    }
}

void Watch_Mission17_ObjectiveController(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        switch ((udword)message)
        {
            case 0:
                break;

            case 0x56983E:
                kasfTutSetPointerTargetAIVolumeNoLine("ObjPtr", kasVolumePtr((char *)0x479E17C));
                kasfObjectiveCreate(
                    "Recon",
                    (char *)m17_reconHyperspaceSignatures_brief[strCurLanguage],
                    (char *)m17_reconHyperspaceSignatures_full[strCurLanguage]);
                break;

            case 17571950:
                kasfTutRemovePointer("ObjPtr");
                kasfObjectiveDestroy("DestroyNomadMoon");
                break;

            case 46028939:
                kasfObjectiveDestroy("DestroyNaggarok");
                break;

            case 55426788:
                kasfMissionFailed();
                break;

            case 110641860:
                kasfTutRemovePointer("ObjPtr");
                kasfObjectiveDestroy("Recon");
                break;

            case 116177507:
                kasfObjectiveDestroy("DestroyEmitters");
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)136940542);
                break;

            case 136940542:
                kasfObjectiveCreate(
                    "DestroyNomadMoon",
                    (char *)m17_destroyNomadMoon_brief[strCurLanguage],
                    (char *)m17_destroyNomadMoon_full[strCurLanguage]);
                kasfTutSetPointerTargetShipSelection("ObjPtr", kasAITeamShipsPtrIfExists(22630782));
                break;

            case 157488868:
                kasfObjectiveDestroy("DestroyNaggarok");
                kasfObjectiveCreate(
                    "DestroyNaggarok",
                    (char *)m17_disableAndDestroyNaggarok_brief[strCurLanguage],
                    (char *)m17_disableAndDestroyNaggarok_full[strCurLanguage]);
                kasfObjectiveDestroy("Research");
                break;

            case 159289139:
                kasfPingAddShipsIndexed(kasGrowSelectionPtr((char *)0x08B809A3), "PingOnEmitters");
                kasfObjectiveDestroy("DestroyNomadMoon");
                kasfObjectiveCreate(
                    "DestroyEmitters",
                    (char *)m17_destroyEmitters_brief[strCurLanguage],
                    (char *)((GetGameDifficulty() >= 2) ? m17_destroyEmittersFullHard[strCurLanguage]
                                                        : m17_destroyEmitters_full[strCurLanguage]));
                break;

            case 175727384:
                kasfObjectiveCreate(
                    "Research",
                    (char *)m17_researchInertialessDrive_brief[strCurLanguage],
                    (char *)m17_researchInertialessDrive_full[strCurLanguage]);
                break;

            case 221012411:
                kasfObjectiveCreate(
                    "DestroyNaggarok",
                    (char *)m17_destroyNaggarok_brief[strCurLanguage],
                    (char *)m17_destroyNaggarok_full[strCurLanguage]);
                break;

            default:
                aitQueuePointer(team, team, message);
                break;
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Init_Mission17_BentusiAttackMothership(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
}

void Watch_Mission17_BentusiAttackMothership(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfSpeechEvent(374244, (sdword)Move_Mission17_BentusiCloseToPlayer, 0);
    }
}

void Move_Mission17_BentusiCloseToPlayer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTeamHyperspaceInNear(kasShipsVectorPtrIfExists(218060896), 6000);
    kasfTeamSwitchPlayerOwner(0);
}

void Watch_Mission17_BentusiResumeAttack(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0x0A7C9B35))
    {
        kasfAttackMothership();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission17_NomadMoonAttackMothership(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!GetGameDifficulty())
    {
        kasfShipsSetPercentDamaged(&((AITeam *)kasCurrentTeamPtr)->shipList, 70);
    }
    kasfTacticsAggressive();
    kasfTeamAttributesBitSet(0x10000);
    kasfShipsSetDontApplyPhysicsFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission17_NomadMoonAttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        return;
    }

    if (kasfMsgReceived((char *)306789))
    {
        kasfShipTryStartModuleSlotTimer(&team->shipList);
    }
    if (!kasfEntryValueGet(71552161) && !kasfShipGetModuleCount(&team->shipList))
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfTeamAttributesBitClear(0x10000);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)244591252);
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
        kasfSpeechEvent(76130852, (sdword)MaintainTeamContext, 0);
    }
    else if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)74556981);
        kasfMsgSend(kasAITeamPtrIfExists(96166939), (char *)71470196);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Init_Mission17_NaggarokBossLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!GetGameDifficulty())
    {
        kasfShipsSetPercentDamaged(&team->shipList, 70);
    }
    kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
    kasfTeamAttributesBitSet(0x10000);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfShipSetTargetableFlag(&team->shipList, 0);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission17_NaggarokBossLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x04428C74))
    {
        kasfTeamAttributesBitClear(0x10000);
    }
    if (!kasfEntryValueGet(71552162) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)170440277);
        spSetSinglePlayerSaveInfo(17, m17_saveInfoName[strCurLanguage]);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
        return;
    }
    if (kasfEntryValueGet(71552162) || kasfTeamCount())
    {
        if (!kasfEntryValueGet(332667))
        {
            if (sub_493460(&team->shipList) < kasfEntryValueGet(-1959681072))
            {
                if (kasfTDSalvageTrialWaitingForShipsActive(&team->shipList))
                {
                    kasfSpeechEvent(0x046B3D4, (sdword)Begin_Mission17_NaggarokSalvageTrap, 0);
                }
                else if (kasfTeamHealthLowest() < 40 &&
                         kasfShipsSelectType(
                             kasGrowSelectionPtr((char *)0x05C46FA2),
                             kasGrowSelectionPtr((char *)team->kasFSMNameHash),
                             "bWorker"))
                {
                    kasfSpeechEvent(22430, (sdword)Begin_Mission17_NaggarokWorkerBaitRun, 0);
                }
                else if (!kasfShipsSelectCapital(
                             kasGrowSelectionPtr((char *)0x0764E81C),
                             kasGrowSelectionPtr((char *)&loc_4A2A29)))
                {
                    kasfSpeechEvent(91892322, (sdword)Begin_Mission17_NaggarokFallbackMoveFar, 0);
                }
            }
            else
            {
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)76343412);
                kasfSpeechEvent(169152067, (sdword)Begin_Mission17_NaggarokDelayedDeath, 0);
            }
        }
    }
}

void Begin_Mission17_NaggarokFallbackMoveFar(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
    kasfLabelledVectorSet((char *)0x05C34E42, 0, 0, 0);
    kasfLabelledVectorSet((char *)0x0546DBDE, 0, 0, 0);
    kasfVectorRandomizeInRange(
        kasVectorPtrIfExists((char *)0x05C34E42),
        kasVectorPtrIfExists((char *)0x0546DBDE),
        100000);
    kasfMoveTo(kasVectorPtrIfExists((char *)0x05C34E42));
    kasTimerCreateSetStartById(91892322, 20);
}

void Watch_Mission17_NaggarokFallbackMoveFar(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x0764E81C), kasGrowSelectionPtr((char *)&loc_4A2A29)))
    {
        kasfSpeechEvent(18788, (sdword)SetMission17NaggarokTargetable, 0);
    }
    else if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) && kasTimerExpiredById(91892322))
    {
        kasfSpeechEvent(128133426, (sdword)Begin_Mission17_NaggarokFallbackMoveNearPlayer, 0);
    }
}

void Begin_Mission17_NaggarokFallbackMoveNearPlayer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
    kasfLabelledVectorSet((char *)0x0C356B32, 0, 0, 0);
    kasfVectorRandomizeInRange(
        kasVectorPtrIfExists((char *)0x0C356B32),
        kasShipsVectorPtrIfExists(218060896),
        10000);
    kasfMoveTo(kasVectorPtrIfExists((char *)0x0C356B32));
    kasTimerDestroyById(248969139);
    kasTimerDestroyById(91892322);
}

void Watch_Mission17_NaggarokFallbackMoveNearPlayer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)0x0764E81C), kasGrowSelectionPtr((char *)&loc_4A2A29)))
    {
        kasfSpeechEvent(18788, (sdword)SetMission17NaggarokTargetable, 0);
        return;
    }

    if (!kasfShipsSelectMoving(kasGrowSelectionPtr((char *)0x0524AE27), &((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasTimerCreateSetStartById(248969139, 30);
    }
    if (kasTimerExpiredById(248969139))
    {
        GrowSelection *createdShips;

        createdShips = kasGrowSelectionPtr((char *)0x09379E54);
        kasfShipsCreate(createdShips, "bCruiseMissile", 6, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfShipsSetPosition(createdShips, 300000.0f, 0.0f, 0.0f);
        kasfShipsDockInstant(createdShips, kasAITeamShipsPtrIfExists(96166939));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x0E87E5B3), createdShips);
        kasTimerCreateSetStartById(91892322, 20);
    }
    if (kasTimerExpiredById(91892322))
    {
        kasfSpeechEvent(91892322, (sdword)Begin_Mission17_NaggarokFallbackMoveFar, 0);
    }
}

void Watch_Mission17_NaggarokRevealCue(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfPingAddShips(&((AITeam *)kasCurrentTeamPtr)->shipList, "PingNaggarok");
        kasfEntrySetValue(332667, 0);
        kasfShipsSetDontApplyPhysicsFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
        sub_493420(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
        kasfTeamAttributesBitClear(0x10000);
        kasfSpeechEvent(18788, (sdword)SetMission17NaggarokTargetable, 0);
    }
}

void Begin_Mission17_NaggarokWorkerBaitRun(void)
{
    GrowSelection *workerSelection;
    GrowSelection *createdShips;
    hvector *playerPosition;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    workerSelection = kasGrowSelectionPtr((char *)0x05C46FA2);
    if (kasfShipsSelectType(workerSelection, kasGrowSelectionPtr((char *)((AITeam *)kasCurrentTeamPtr)->kasFSMNameHash), "bWorker") &&
        kasfTeamHealthLowest() < 80)
    {
        playerPosition = kasShipsVectorPtrIfExists((sdword)&loc_4A2A29);
        kasfShipsSortByDistanceToPoint(workerSelection, playerPosition);
        kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x5425), workerSelection, kasfShipsCount(workerSelection) - 1);
        kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
        kasfEntryGetOrCreateSetValue(206803400, kasfVectorRoundX(kasShipsVectorPtrIfExists(21541)));
        kasfEntryGetOrCreateSetValue(206803401, kasfVectorRoundY(kasShipsVectorPtrIfExists(21541)));
        kasfEntryGetOrCreateSetValue(206803402, kasfVectorRoundZ(kasShipsVectorPtrIfExists(21541)));
        kasfMoveTo(kasShipsVectorPtrIfExists(21541));
    }
    else
    {
        kasfSpeechEvent(18788, (sdword)SetMission17NaggarokTargetable, 0);
    }
}

void Watch_Mission17_NaggarokWorkerBaitRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfLabelledVectorSet(
        (char *)0x05C53914,
        kasfEntryValueGet(206803400),
        kasfEntryValueGet(206803401),
        kasfEntryValueGet(206803402));
    if (kasfNearby(kasVectorPtrIfExists((char *)0x05C53914), 5000))
    {
        sub_499790();
    }
    if (kasfTeamHealthLowest() < 80)
    {
        if (!kasfShipsSelectNearby(
                kasGrowSelectionPtr((char *)0x05296679),
                kasGrowSelectionPtr((char *)0x06DA2DA9),
                kasVectorPtrIfExists((char *)0x05C53914),
                5000) ||
            !kasfShipsSelectType(
                kasGrowSelectionPtr((char *)0x05296679),
                kasGrowSelectionPtr((char *)0x05296679),
                "bWorker"))
        {
            kasfSpeechEvent(22430, (sdword)Begin_Mission17_NaggarokWorkerBaitRun, 0);
        }
    }
    else
    {
        kasfSpeechEvent(18788, (sdword)SetMission17NaggarokTargetable, 0);
    }
}

void Begin_Mission17_NaggarokSalvageTrap(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfLabelledVectorSet((char *)0x046B3D4, 0, 0, 0);
    kasfLabelledVectorSet((char *)0x0575A5, 0, 0, 0);
    if (kasfTDSalvageTrialWaitingForShipsDone(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfSpeechEvent(161837397, (sdword)SetMission17NaggarokTargetable, 0);
    }
    else
    {
        kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
        kasfTDSalvageTrialGetWaitingForShipsPoints(
            &((AITeam *)kasCurrentTeamPtr)->shipList,
            kasVectorPtrIfExists((char *)0x046B3D4),
            kasVectorPtrIfExists((char *)0x0575A5));
        kasfMoveTo(kasVectorPtrIfExists((char *)0x0575A5));
    }
}

void Watch_Mission17_NaggarokSalvageTrap(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyInSensorRange(&((AITeam *)kasCurrentTeamPtr)->shipList) &&
        !kasfEntryValueGet(-2004742053))
    {
        kasfEntryGetOrCreateSetValue(-2004742053, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)267431077);
    }
    if (sub_493460(&((AITeam *)kasCurrentTeamPtr)->shipList) < kasfEntryValueGet(-1959681072))
    {
        if (kasfTDSalvageTrialWaitingForShipsActive(&((AITeam *)kasCurrentTeamPtr)->shipList))
        {
            if (!kasfShipsSelectMoving(kasGrowSelectionPtr((char *)0x0524AE27), &((AITeam *)kasCurrentTeamPtr)->shipList))
            {
                kasfSpeechEvent(0x046B3D4, (sdword)Begin_Mission17_NaggarokSalvageTrap, 0);
            }
        }
        else
        {
            kasfShipSetTargetableFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(18788, (sdword)SetMission17NaggarokTargetable, 0);
        }
    }
    else
    {
        kasfSpeechEvent(169152067, (sdword)Begin_Mission17_NaggarokDelayedDeath, 0);
    }
}

void Init_Mission17_NaggarokCruiseMissileStrikeTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr((char *)0x0E87E5B3));
    kasfEntryGetOrCreateSetValue(332667, 1);
}

void Watch_Mission17_NaggarokCruiseMissileStrikeTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        if (!kasfTeamCount() && kasfShipsCount(kasGrowSelectionPtr((char *)0x0E87E5B3)))
        {
            if (kasfShipsSelectResourcingOrHyperspacing(
                    kasGrowSelectionPtr((char *)0x04937F94),
                    kasGrowSelectionPtr((char *)0x0E87E5B3)) >= 4)
            {
                kasfShipsRemove(kasGrowSelectionPtr((char *)0x0E87E5B3), kasGrowSelectionPtr((char *)0x04937F94));
                kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr((char *)0x04937F94));
                kasfSpeechEvent(84320888, (sdword)Begin_Mission17_NaggarokCruiseMissileLaunchSfx, 0);
            }
        }
    }
    else if (!kasfTeamCount())
    {
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Begin_Mission17_NaggarokCruiseMissileLaunchSfx(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSoundEventShips(kasAITeamShipsPtrIfExists(96166939), 2056);
    kasfLaunch();
}

void Watch_Mission17_NaggarokCruiseMissileLaunchSfx(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_Mission17_NaggarokCruiseMissileAttackRun, 0);
    }
}

void Init_Mission17_CrystalPointTrigger(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(-1980349692, kasfTeamCount());
}

void Watch_Mission17_CrystalPointTrigger(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(134468951))
    {
        kasfShipsSelectSinglePlayerCombatShips(kasGrowSelectionPtr((char *)0x2BC6679), kasGrowSelectionPtr((char *)0x117CA33));
        if (kasfShipsSelectInsideVolume(
                kasGrowSelectionPtr((char *)0x4E37D85),
                kasGrowSelectionPtr((char *)0x2BC6679),
                kasVolumePtr((char *)0x7E88293)))
        {
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)42807940);
            kasfEntryGetOrCreateSetValue(134468951, 1);
        }
    }
}

void Init_Mission17_CarrierCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfTeamSwitchPlayerOwner(0);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(124217682)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-2131154846, 0);
        kasfEntryGetOrCreateSetValue(-2006545659, 1);
        if (kasfTeamCount())
        {
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)103051523);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfShipsSetSlaveableFlag(&team->shipList, 0);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(337061, (sdword)Begin_Mission17_CarrierCombatTeamLoop, 0);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        }
    }
}

void Init_Mission17_CarrierCombatTeam_Reevaluate(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(124217682)) && kasfEntryValueGet(-2006545659) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2006545659, 0);
        kasfEntrySetValue(-2131154846, 1);
    }
    if (kasfEntryValueGet(332667))
    {
        return;
    }

    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(84573278), (char *)50020867);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        return;
    }

    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        if (message == (void *)126523733)
        {
            kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
            kasfVarDestroy((char *)&team->shipList);
        }
        else if (message == (void *)237188773)
        {
            kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo == NULL || *queueInfo == 0)
        {
            break;
        }
        queueIndex--;
    }
}

void Init_Mission17_FinalDFGFrigateCombatTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(379047)))
    {
        kasfEntryGetOrCreateSetValue(-1912498034, 0);
    }
    else
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-1908032238, 0);
        kasfEntryGetOrCreateSetValue(-1927751787, 1);
        kasfEntryGetOrCreateSetValue(111786462, 0);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)179875155);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 1);
            kasfSpeechEvent(0x04C9664, (sdword)Begin_Mission17_FinalDFGFrigateCombatLoop, 0);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        }
    }
}

void Begin_Mission17(void)
{
int *v0; // eax
int *v1; // eax
int *v2; // eax
int *v3; // eax
int *v4; // eax
int *v5; // eax
int *v6; // eax
int *v7; // eax
int *v8; // eax
int *v9; // eax
int *v10; // eax
int *v11; // eax
int *v12; // eax
int *v13; // eax
int *v14; // eax
int *v15; // eax
int *v16; // eax
int *v17; // eax
int *v18; // eax
int *v19; // eax
int *v20; // eax
_DWORD *v21; // eax
_DWORD *v22; // eax
_DWORD *v23; // eax
_DWORD *v24; // eax
_DWORD *v25; // eax
_DWORD *v26; // eax
_DWORD *v27; // eax
_DWORD *v28; // eax
_DWORD *v29; // eax
_DWORD *v30; // eax
_DWORD *v31; // eax
_DWORD *v32; // eax
_DWORD *v33; // eax
_DWORD *v34; // eax
_DWORD *v35; // eax
_DWORD *v36; // eax
_DWORD *v37; // eax
_DWORD *v38; // eax
_DWORD *v39; // eax
_DWORD *v40; // eax
_DWORD *v41; // eax
_DWORD *v42; // eax
_DWORD *v43; // eax
_DWORD *v44; // eax
_DWORD *v45; // eax
_DWORD *v46; // eax
_DWORD *v47; // eax
_DWORD *v48; // eax
_DWORD *v49; // eax
_DWORD *v50; // eax
_DWORD *v51; // eax
_DWORD *v52; // eax
_DWORD *v53; // eax
_DWORD *v54; // eax
_DWORD *v55; // eax
_DWORD *v56; // eax
_DWORD *v57; // eax
_DWORD *v58; // eax
_DWORD *v59; // eax
struct AITeam *v60; // eax
struct AITeam *v61; // eax
struct AITeam *v62; // eax
struct AITeam *v63; // eax
struct AITeam *v64; // eax
struct AITeam *v65; // eax
struct AITeam *v66; // eax
struct AITeam *v67; // eax
struct AITeam *v68; // eax
struct AITeam *v69; // eax
struct AITeam *v70; // eax
struct AITeam *v71; // eax
struct AITeam *v72; // eax
struct AITeam *v73; // eax
struct AITeam *v74; // eax
struct AITeam *v75; // eax
struct AITeam *v76; // eax
struct AITeam *v77; // eax
struct AITeam *v78; // eax
struct AITeam *v79; // eax
struct AITeam *v80; // eax
struct AITeam *v81; // eax
struct AITeam *v82; // eax
struct AITeam *v83; // eax
struct AITeam *v84; // eax
struct AITeam *v85; // eax
struct AITeam *v86; // eax
struct AITeam *v87; // eax
struct AITeam *v88; // eax
struct AITeam *v89; // eax
struct AITeam *v90; // eax
struct AITeam *v91; // eax
struct AITeam *v92; // eax
struct AITeam *v93; // eax
struct AITeam *v94; // eax
struct AITeam *v95; // eax
struct AITeam *v96; // eax
struct AITeam *v97; // eax
struct AITeam *v98; // eax
struct AITeam *v99; // eax
struct AITeam *v100; // eax
struct AITeam *v101; // eax
struct AITeam *v102; // eax
struct AITeam *v103; // eax
struct AITeam *v104; // eax
struct AITeam *v105; // eax
struct AITeam *v106; // eax
_DWORD *v107; // eax
struct AITeam *v108; // eax
struct AITeam *v109; // eax
struct AITeam *v110; // eax
struct AITeam *v111; // eax
struct AITeam *v112; // eax
struct AITeam *v113; // eax
struct AITeam *v114; // eax
struct AITeam *v115; // eax
struct AITeam *v116; // eax
struct AITeam *v117; // eax
struct AITeam *v118; // eax
struct AITeam *v119; // eax
struct AITeam *v120; // eax
struct AITeam *v121; // eax
struct AITeam *v122; // eax
struct AITeam *v123; // eax
struct AITeam *v124; // eax
struct AITeam *v125; // eax
struct AITeam *v126; // eax
struct AITeam *v127; // eax
struct AITeam *v128; // eax
struct AITeam *v129; // eax
struct AITeam *v130; // eax
struct AITeam *v131; // eax
struct AITeam *v132; // eax
struct AITeam *v133; // eax
struct AITeam *v134; // eax
struct AITeam *v135; // eax
struct AITeam *v136; // eax
struct AITeam *v137; // eax
struct AITeam *v138; // eax
struct AITeam *v139; // eax
struct AITeam *v140; // eax
struct AITeam *v141; // eax
struct AITeam *v142; // eax
struct AITeam *v143; // eax
struct AITeam *v144; // eax
struct AITeam *v145; // eax
struct AITeam *v146; // eax
struct AITeam *v147; // eax
struct AITeam *v148; // eax
struct AITeam *v149; // eax
struct AITeam *v150; // eax
struct AITeam *v151; // eax
struct AITeam *v152; // eax
struct AITeam *v153; // eax
struct AITeam *v154; // eax
struct AITeam *v155; // eax
struct AITeam *v156; // eax
struct AITeam *v157; // eax
struct AITeam *v158; // eax
struct AITeam *v159; // eax
struct AITeam *v160; // eax
struct AITeam *v161; // eax
struct AITeam *v162; // eax
struct AITeam *v163; // eax
struct AITeam *v164; // eax
struct AITeam *v165; // eax
struct AITeam *v166; // eax
struct AITeam *v167; // eax
struct AITeam *v168; // eax
struct AITeam *v169; // eax
struct AITeam *v170; // eax
int *v171; // eax
int v172; // eax
int *v173; // eax
int v174; // eax
int *v175; // eax
int v176; // eax
int *v177; // eax
int v178; // eax
int *v179; // eax
int v180; // eax
int *v181; // eax
int v182; // eax
int *v183; // eax
int v184; // eax
struct AITeam *v185; // eax
struct AITeam *v186; // eax
struct AITeam *v187; // eax
struct AITeam *v188; // eax
struct AITeam *v189; // eax
struct AITeam *v190; // eax
struct AITeam *v191; // eax
struct AITeam *v192; // eax
struct AITeam *v193; // eax
struct AITeam *v194; // eax
struct AITeam *v195; // eax
struct AITeam *v196; // eax
struct AITeam *v197; // eax
struct AITeam *v198; // eax
struct AITeam *v199; // eax
struct AITeam *v200; // eax
struct AITeam *v201; // eax
struct AITeam *v202; // eax
struct AITeam *v203; // eax
struct AITeam *v204; // eax
_DWORD *v205; // eax
_DWORD *v206; // eax
_DWORD *v207; // eax
_DWORD *v208; // eax
_DWORD *v209; // eax
struct AITeam *v210; // eax
struct AITeam *v211; // eax
struct AITeam *v212; // eax
struct AITeam *v213; // eax
struct AITeam *v214; // eax
struct AITeam *v215; // eax
struct AITeam *v216; // eax
struct AITeam *v217; // eax
struct AITeam *v218; // eax
struct AITeam *v219; // eax
struct AITeam *v220; // eax
struct AITeam *v221; // eax
struct AITeam *v222; // eax
struct AITeam *v223; // eax
struct AITeam *v224; // eax
struct AITeam *v225; // eax
struct AITeam *v226; // eax
_DWORD *v227; // eax
struct AITeam *v228; // eax
struct AITeam *v229; // eax
_DWORD *v230; // eax
struct AITeam *v231; // eax
struct AITeam *v232; // eax
struct AITeam *v233; // eax
struct AITeam *v234; // eax
struct AITeam *v235; // eax
struct AITeam *v236; // eax
struct AITeam *v237; // eax
struct AITeam *v238; // eax
struct AITeam *v239; // eax
struct AITeam *v240; // eax
struct AITeam *v241; // eax
struct AITeam *v242; // eax
struct AITeam *v243; // eax
struct AITeam *v244; // eax
struct AITeam *v245; // eax
struct AITeam *v246; // eax
struct AITeam *v247; // eax
struct AITeam *v248; // eax
struct AITeam *v249; // eax
struct AITeam *v250; // eax
struct AITeam *v251; // eax
struct AITeam *v252; // eax
struct AITeam *v253; // eax
struct AITeam *v254; // eax
struct AITeam *v255; // eax
struct AITeam *v256; // eax
struct AITeam *v257; // eax
struct AITeam *v258; // eax
struct AITeam *v259; // eax
struct AITeam *v260; // eax
struct AITeam *v261; // eax
struct AITeam *v262; // eax
struct AITeam *v263; // eax
struct AITeam *v264; // eax
struct AITeam *v265; // eax
struct AITeam *v266; // eax
struct AITeam *v267; // eax
_DWORD *v268; // eax
_DWORD *v269; // eax
_DWORD *v270; // eax
_DWORD *v271; // eax
struct AITeam *v272; // eax
struct AITeam *v273; // eax
struct AITeam *v274; // eax
struct AITeam *v275; // eax
struct AITeam *v276; // eax
struct AITeam *v277; // eax
struct AITeam *v278; // eax
struct AITeam *v279; // eax
struct AITeam *v280; // eax
struct AITeam *v281; // eax
struct AITeam *v282; // eax
struct AITeam *v283; // eax
struct AITeam *v284; // eax
struct AITeam *v285; // eax
struct AITeam *v286; // eax
struct AITeam *v287; // eax
struct AITeam *v288; // eax
struct AITeam *v289; // eax
struct AITeam *v290; // eax
struct AITeam *v291; // eax
struct AITeam *v292; // eax
struct AITeam *v293; // eax
struct AITeam *v294; // eax
struct AITeam *v295; // eax
struct AITeam *v296; // eax
struct AITeam *v297; // eax
struct AITeam *v298; // eax
_DWORD *v299; // eax
_DWORD *v300; // eax
_DWORD *v301; // eax
_DWORD *v302; // eax
_DWORD *v303; // eax
_DWORD *v304; // eax
_DWORD *v305; // eax
_DWORD *v306; // eax
_DWORD *v307; // eax
_DWORD *v308; // eax
_DWORD *v309; // eax
_DWORD *v310; // eax
_DWORD *v311; // eax
_DWORD *v312; // eax
_DWORD *v313; // [esp-18h] [ebp-18h]
_DWORD *v314; // [esp-8h] [ebp-8h]
_DWORD *v315; // [esp-8h] [ebp-8h]
_DWORD *v316; // [esp-8h] [ebp-8h]
_DWORD *v317; // [esp-8h] [ebp-8h]
_DWORD *v318; // [esp-8h] [ebp-8h]
_DWORD *v319; // [esp-8h] [ebp-8h]
_DWORD *v320; // [esp-8h] [ebp-8h]
int *v321; // [esp-4h] [ebp-4h]
int *v322; // [esp-4h] [ebp-4h]
int *v323; // [esp-4h] [ebp-4h]
int *v324; // [esp-4h] [ebp-4h]
int *v325; // [esp-4h] [ebp-4h]
int *v326; // [esp-4h] [ebp-4h]
int *v327; // [esp-4h] [ebp-4h]
int *v328; // [esp-4h] [ebp-4h]
int *v329; // [esp-4h] [ebp-4h]
int *v330; // [esp-4h] [ebp-4h]
int *v331; // [esp-4h] [ebp-4h]
int *v332; // [esp-4h] [ebp-4h]
int *v333; // [esp-4h] [ebp-4h]
int *v334; // [esp-4h] [ebp-4h]
int *v335; // [esp-4h] [ebp-4h]
int *v336; // [esp-4h] [ebp-4h]
int *v337; // [esp-4h] [ebp-4h]
int *v338; // [esp-4h] [ebp-4h]
int *v339; // [esp-4h] [ebp-4h]
int *v340; // [esp-4h] [ebp-4h]
int *v341; // [esp-4h] [ebp-4h]

v321 = sub_4907E0(108434753);
v0 = (int *)kasGrowSelectionPtr(108434696);
kasfShipsAdd(v0, v321);
v322 = sub_4907E0(108434754);
v1 = (int *)kasGrowSelectionPtr(108434696);
kasfShipsAdd(v1, v322);
v323 = sub_4907E0(108453441);
v2 = (int *)kasGrowSelectionPtr(14701123);
kasfShipsAdd(v2, v323);
v324 = sub_4907E0(108453442);
v3 = (int *)kasGrowSelectionPtr(14701123);
kasfShipsAdd(v3, v324);
v325 = sub_4907E0(108453444);
v4 = (int *)kasGrowSelectionPtr(14701123);
kasfShipsAdd(v4, v325);
v326 = sub_4907E0(108453441);
v5 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v5, v326);
v327 = sub_4907E0(108453444);
v6 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v6, v327);
v328 = sub_4907E0(175761953);
v7 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v7, v328);
v329 = sub_4907E0(175761954);
v8 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v8, v329);
v330 = sub_4907E0(175761955);
v9 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v9, v330);
v331 = sub_4907E0(175761956);
v10 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v10, v331);
v332 = sub_4907E0(175761957);
v11 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v11, v332);
v333 = sub_4907E0(96166939);
v12 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v12, v333);
v334 = sub_4907E0(22630782);
v13 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v13, v334);
v335 = sub_4907E0(128156707);
v14 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v14, v335);
v336 = (int *)kasGrowSelectionPtr(146278819);
v15 = (int *)kasGrowSelectionPtr(175761954);
kasfShipsAdd(v15, v336);
v337 = sub_4907E0(175761953);
v16 = (int *)kasGrowSelectionPtr(163766500);
kasfShipsAdd(v16, v337);
v338 = sub_4907E0(175761954);
v17 = (int *)kasGrowSelectionPtr(163766500);
kasfShipsAdd(v17, v338);
v339 = sub_4907E0(175761955);
v18 = (int *)kasGrowSelectionPtr(163766500);
kasfShipsAdd(v18, v339);
v340 = sub_4907E0(175761956);
v19 = (int *)kasGrowSelectionPtr(163766500);
kasfShipsAdd(v19, v340);
v341 = sub_4907E0(175761957);
v20 = (int *)kasGrowSelectionPtr(163766500);
kasfShipsAdd(v20, v341);
switch ( GetGameDifficulty_() )
{
case 0:
v21 = sub_4907E0(175761957);
kasfDeleteShips((int)v21);
v22 = sub_4907E0(175575255);
kasfDeleteShips((int)v22);
v313 = sub_4907E0(179110480);
kasfDeleteShips((int)v313);
v23 = sub_4907E0(175761956);
kasfDeleteShips((int)v23);
v24 = sub_4907E0(175575254);
kasfDeleteShips((int)v24);
v25 = sub_4907E0(128634921);
kasfDeleteShips((int)v25);
v26 = sub_4907E0(175761955);
kasfDeleteShips((int)v26);
v27 = sub_4907E0(175575253);
kasfDeleteShips((int)v27);
v28 = sub_4907E0(128634920);
kasfDeleteShips((int)v28);
v29 = sub_4907E0(175761954);
kasfDeleteShips((int)v29);
v30 = sub_4907E0(175575250);
kasfDeleteShips((int)v30);
v31 = sub_4907E0(128634918);
kasfDeleteShips((int)v31);
kasfRUsSet(1, (struct Derelict *)0x4E20);
kasfRUsSet(2, (struct Derelict *)0x4E20);
kasfRUsSet(3, (struct Derelict *)0x9C40);
sub_491F90(61565948, 10000);
kasfEntryGetOrCreateSetValue(-1959681072, 20);
break;
case 1:
v32 = sub_4907E0(175761957);
kasfDeleteShips((int)v32);
v33 = sub_4907E0(175575255);
kasfDeleteShips((int)v33);
v34 = sub_4907E0(179110480);
kasfDeleteShips((int)v34);
v35 = sub_4907E0(175761956);
kasfDeleteShips((int)v35);
v36 = sub_4907E0(175575254);
kasfDeleteShips((int)v36);
v37 = sub_4907E0(128634921);
kasfDeleteShips((int)v37);
v38 = sub_4907E0(175761955);
kasfDeleteShips((int)v38);
v39 = sub_4907E0(175575253);
kasfDeleteShips((int)v39);
v40 = sub_4907E0(128634920);
kasfDeleteShips((int)v40);
kasfRUsSet(1, (struct Derelict *)0x7530);
kasfRUsSet(2, (struct Derelict *)0x7530);
kasfRUsSet(3, (struct Derelict *)0xEA60);
sub_491F90(61565948, 9600);
kasfEntryGetOrCreateSetValue(-1959681072, 40);
break;
case 2:
v41 = sub_4907E0(175761957);
kasfDeleteShips((int)v41);
v42 = sub_4907E0(175575255);
kasfDeleteShips((int)v42);
v43 = sub_4907E0(179110480);
kasfDeleteShips((int)v43);
v44 = sub_4907E0(175761956);
kasfDeleteShips((int)v44);
v45 = sub_4907E0(175575254);
kasfDeleteShips((int)v45);
v46 = sub_4907E0(128634921);
kasfDeleteShips((int)v46);
v47 = sub_4907E0(175761955);
kasfDeleteShips((int)v47);
v48 = sub_4907E0(175575253);
kasfDeleteShips((int)v48);
v49 = sub_4907E0(128634920);
kasfDeleteShips((int)v49);
kasfRUsSet(1, (struct Derelict *)0xC350);
kasfRUsSet(2, (struct Derelict *)0x9C40);
kasfRUsSet(3, (struct Derelict *)0x9C40);
sub_491F90(61565948, 8600);
kasfEntryGetOrCreateSetValue(-1959681072, 60);
break;
case 3:
v50 = sub_4907E0(175761957);
kasfDeleteShips((int)v50);
v51 = sub_4907E0(175575255);
kasfDeleteShips((int)v51);
v52 = sub_4907E0(179110480);
kasfDeleteShips((int)v52);
v53 = sub_4907E0(175761956);
kasfDeleteShips((int)v53);
v54 = sub_4907E0(175575254);
kasfDeleteShips((int)v54);
v55 = sub_4907E0(128634921);
kasfDeleteShips((int)v55);
v56 = sub_4907A0(0x56983E);
sub_494A00((int)v56, 4693508);
kasfRUsSet(1, (struct Derelict *)0x11170);
kasfRUsSet(2, (struct Derelict *)0x9C40);
kasfRUsSet(3, (struct Derelict *)0x4E20);
sub_491F90(61565948, 8000);
kasfEntryGetOrCreateSetValue(-1959681072, 80);
break;
case 4:
v57 = sub_4907A0(0x56983E);
sub_494A00((int)v57, 4693508);
v58 = sub_4907A0(0x56983E);
sub_494A00((int)v58, 4693508);
kasfRUsSet(1, (struct Derelict *)0x186A0);
kasfRUsSet(2, (struct Derelict *)0xEA60);
kasfRUsSet(3, (struct Derelict *)0x2710);
sub_491F90(61565948, 7200);
kasfEntryGetOrCreateSetValue(-1959681072, 100);
break;
default:
break;
}
kasfActivateTech("TECH_INFECT");
kasfActivateTech("TECH_FORCE_FIELD1");
kasfActivateTech("TECH_FORCE_FIELD2");
kasfActivateTech("TECH_FORCE_FIELD3");
kasfActivateTech("TECH_ENERGY");
kasfActivateTech("TECH_REPAIR");
kasfActivateTech("TECH_SALVAGE");
kasfActivateTech("TECH_CLOAKING");
kasfActivateTech("TECH_AFTERBURNER");
kasfActivateTech("TECH_INFECT");
kasfActivateTech("TECH_FORCE_FIELD1");
kasfActivateTech("TECH_FORCE_FIELD2");
kasfActivateTech("TECH_FORCE_FIELD3");
kasfActivateTech("TECH_EMP");
kasfActivateTech("TECH_ADVANCED_SENSORS");
v59 = sub_4907E0(128156707);
kasfShipsApplyPlayerUpgrades((int)v59);
v60 = (struct AITeam *)sub_4907A0(176850956);
kasFSMCreate((char *)0xA8A880C, Init_Mission17_MissionFlow, Watch_NoOpTeamContext, v60);
v61 = (struct AITeam *)sub_4907A0(160291907);
kasFSMCreate((char *)0x98DDC43, Watch_NoOpTeamContext, Watch_Mission17_ObjectiveController, v61);
v62 = (struct AITeam *)sub_4907A0(0x469684);
kasFSMCreate((char *)0x469684, Init_Mission17_ReinforcementTriggerRouter, Watch_Mission17_ReinforcementTriggerRouter, v62);
v63 = (struct AITeam *)sub_4907A0(44665972);
kasFSMCreate((char *)0x2A98C74, Init_Mission17_ReinforcementPool, Fill_Mission16_ReinforcementPoolFromParade, v63);
v64 = (struct AITeam *)sub_4907A0(217111973);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v64);
v65 = (struct AITeam *)sub_4907A0(22630782);
kasFSMCreate((char *)0x159517E, Init_Mission17_NomadMoonAttackMothership, Watch_Mission17_NomadMoonAttackMothership, v65);
v66 = (struct AITeam *)sub_4907A0(96166939);
kasFSMCreate((char *)0x5BB641B, Init_Mission17_NaggarokBossLoop, Watch_Mission17_NaggarokBossLoop, v66);
v67 = (struct AITeam *)sub_4907A0(243787187);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v67);
v68 = (struct AITeam *)sub_4907A0(142498721);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v68);
v69 = (struct AITeam *)sub_4907A0(142498722);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v69);
v70 = (struct AITeam *)sub_4907A0(142498723);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v70);
v71 = (struct AITeam *)sub_4907A0(142498724);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v71);
v72 = (struct AITeam *)sub_4907A0(142498725);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v72);
v73 = (struct AITeam *)sub_4907A0(142498726);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v73);
v74 = (struct AITeam *)sub_4907A0(142498727);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v74);
v75 = (struct AITeam *)sub_4907A0(142498728);
kasFSMCreate((char *)0xE87E5B3, Init_Mission17_NaggarokCruiseMissileStrikeTeam, Watch_Mission17_NaggarokCruiseMissileStrikeTeam, v75);
v76 = (struct AITeam *)sub_4907A0(208120516);
kasFSMCreate((char *)0xC67AAC4, Init_BeastWorkerHarvestReinforcementTeam, Watch_BeastWorkerHarvestReinforcementTeam, v76);
v77 = (struct AITeam *)sub_4907A0(108702865);
kasFSMCreate((char *)0xC67AAC4, Init_BeastWorkerHarvestReinforcementTeam, Watch_BeastWorkerHarvestReinforcementTeam, v77);
v78 = (struct AITeam *)sub_4907A0(108702866);
kasFSMCreate((char *)0xC67AAC4, Init_BeastWorkerHarvestReinforcementTeam, Watch_BeastWorkerHarvestReinforcementTeam, v78);
v79 = (struct AITeam *)sub_4907A0(108702867);
kasFSMCreate((char *)0xC67AAC4, Init_BeastWorkerHarvestReinforcementTeam, Watch_BeastWorkerHarvestReinforcementTeam, v79);
v80 = (struct AITeam *)sub_4907A0(379047);
kasFSMCreate((char *)0x48A873, Init_Mission17_FinalDFGFrigateCombatTeam, Init_Mission17_FinalDFGFrigateCombatTeam_Reevaluate, v80);
v81 = (struct AITeam *)sub_4907A0(0x5C8A81);
kasFSMCreate((char *)0x48A873, Init_Mission17_FinalDFGFrigateCombatTeam, Init_Mission17_FinalDFGFrigateCombatTeam_Reevaluate, v81);
v82 = (struct AITeam *)sub_4907A0(0x5C8A82);
kasFSMCreate((char *)0x48A873, Init_Mission17_FinalDFGFrigateCombatTeam, Init_Mission17_FinalDFGFrigateCombatTeam_Reevaluate, v82);
v83 = (struct AITeam *)sub_4907A0(0x5C8A83);
kasFSMCreate((char *)0x48A873, Init_Mission17_FinalDFGFrigateCombatTeam, Init_Mission17_FinalDFGFrigateCombatTeam_Reevaluate, v83);
v84 = (struct AITeam *)sub_4907A0(0x5C8A84);
kasFSMCreate((char *)0x48A873, Init_Mission17_FinalDFGFrigateCombatTeam, Init_Mission17_FinalDFGFrigateCombatTeam_Reevaluate, v84);
v85 = (struct AITeam *)sub_4907A0(58381106);
kasFSMCreate((char *)0x7AD3343, Init_BeastProcessorReinforcementTeam, Watch_BeastProcessorReinforcementTeam, v85);
v86 = (struct AITeam *)sub_4907A0(128791297);
kasFSMCreate((char *)0x7AD3343, Init_BeastProcessorReinforcementTeam, Watch_BeastProcessorReinforcementTeam, v86);
v87 = (struct AITeam *)sub_4907A0(128791298);
kasFSMCreate((char *)0x7AD3343, Init_BeastProcessorReinforcementTeam, Watch_BeastProcessorReinforcementTeam, v87);
v88 = (struct AITeam *)sub_4907A0(186735637);
kasFSMCreate((char *)0x215C113, Init_Mission08_HiveFrigateReinforcementTeam, Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate, v88);
v89 = (struct AITeam *)sub_4907A0(34980305);
kasFSMCreate((char *)0x215C113, Init_Mission08_HiveFrigateReinforcementTeam, Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate, v89);
v90 = (struct AITeam *)sub_4907A0(200156578);
kasFSMCreate((char *)0xEE25AC3, Init_Mission17_BeastCloakedFighterReinforcementTeam, Init_Mission17_BeastCloakedFighterReinforcementTeam_Reevaluate, v90);
v91 = (struct AITeam *)sub_4907A0(249715330);
kasFSMCreate((char *)0xEE25AC3, Init_Mission17_BeastCloakedFighterReinforcementTeam, Init_Mission17_BeastCloakedFighterReinforcementTeam_Reevaluate, v91);
v92 = (struct AITeam *)sub_4907A0(96759714);
kasFSMCreate((char *)0xC46FA23, Init_BeastWorkerRepairReinforcementTeam_Preamble, Watch_BeastWorkerRepairReinforcementTeam, v92);
v93 = (struct AITeam *)sub_4907A0(205978209);
kasFSMCreate((char *)0xC46FA23, Init_BeastWorkerRepairReinforcementTeam_Preamble, Watch_BeastWorkerRepairReinforcementTeam, v93);
v94 = (struct AITeam *)sub_4907A0(205978210);
kasFSMCreate((char *)0xC46FA23, Init_BeastWorkerRepairReinforcementTeam_Preamble, Watch_BeastWorkerRepairReinforcementTeam, v94);
v95 = (struct AITeam *)sub_4907A0(205978211);
kasFSMCreate((char *)0xC46FA23, Init_BeastWorkerRepairReinforcementTeam_Preamble, Watch_BeastWorkerRepairReinforcementTeam, v95);
v96 = (struct AITeam *)sub_4907A0(151690773);
kasFSMCreate((char *)0x0A9E133, Init_Mission17_BeastIonArrayFrigateReinforcementTeam, Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate, v96);
v97 = (struct AITeam *)sub_4907A0(0x0A9E1F1);
kasFSMCreate((char *)0x0A9E133, Init_Mission17_BeastIonArrayFrigateReinforcementTeam, Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate, v97);
v98 = (struct AITeam *)sub_4907A0(0x0A9E1F2);
kasFSMCreate((char *)0x0A9E133, Init_Mission17_BeastIonArrayFrigateReinforcementTeam, Init_Mission17_BeastIonArrayFrigateReinforcementTeam_Reevaluate, v98);
v99 = (struct AITeam *)sub_4907A0(62813829);
kasFSMCreate((char *)0xBE76893, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate, v99);
v100 = (struct AITeam *)sub_4907A0(199714897);
kasFSMCreate((char *)0xBE76893, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate, v100);
v101 = (struct AITeam *)sub_4907A0(199714898);
kasFSMCreate((char *)0xBE76893, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate, v101);
v102 = (struct AITeam *)sub_4907A0(199714899);
kasFSMCreate((char *)0xBE76893, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam, Init_Mission17_BeastMultiBeamFrigateReinforcementTeam_Reevaluate, v102);
v103 = (struct AITeam *)sub_4907A0(212896882);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v103);
v104 = (struct AITeam *)sub_4907A0(185124849);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v104);
v105 = (struct AITeam *)sub_4907A0(185124850);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v105);
v106 = (struct AITeam *)sub_4907A0(185124851);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v106);
v107 = sub_4907A0(108453441);
sub_494A00((int)v107, 362601);
v108 = (struct AITeam *)sub_4907A0(124218850);
kasFSMCreate((char *)0x676DE03, Init_Mission17_BeastCarrierReinforcementTeam, Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate, v108);
v109 = (struct AITeam *)sub_4907A0(108453441);
kasFSMCreate((char *)0x676DE03, Init_Mission17_BeastCarrierReinforcementTeam, Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate, v109);
v110 = (struct AITeam *)sub_4907A0(108453442);
kasFSMCreate((char *)0x676DE03, Init_Mission17_BeastCarrierReinforcementTeam, Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate, v110);
v111 = (struct AITeam *)sub_4907A0(75148549);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v111);
v112 = (struct AITeam *)sub_4907A0(128634913);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v112);
v113 = (struct AITeam *)sub_4907A0(128634914);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v113);
v114 = (struct AITeam *)sub_4907A0(182180066);
kasFSMCreate((char *)0xDBD8ED3, Init_Mission17_BeastFighterReinforcementTeam, Init_Mission17_BeastFighterReinforcementTeam_Reevaluate, v114);
v115 = (struct AITeam *)sub_4907A0(230526609);
kasFSMCreate((char *)0xDBD8ED3, Init_Mission17_BeastFighterReinforcementTeam, Init_Mission17_BeastFighterReinforcementTeam_Reevaluate, v115);
v116 = (struct AITeam *)sub_4907A0(230526610);
kasFSMCreate((char *)0xDBD8ED3, Init_Mission17_BeastFighterReinforcementTeam, Init_Mission17_BeastFighterReinforcementTeam_Reevaluate, v116);
v117 = (struct AITeam *)sub_4907A0(230526611);
kasFSMCreate((char *)0xDBD8ED3, Init_Mission17_BeastFighterReinforcementTeam, Init_Mission17_BeastFighterReinforcementTeam_Reevaluate, v117);
v118 = (struct AITeam *)sub_4907A0(223105365);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v118);
v119 = (struct AITeam *)sub_4907A0(80025009);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v119);
v120 = (struct AITeam *)sub_4907A0(80025013);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v120);
v121 = (struct AITeam *)sub_4907A0(80025014);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v121);
v122 = (struct AITeam *)sub_4907A0(80025015);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v122);
v123 = (struct AITeam *)sub_4907A0(80025016);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v123);
v124 = (struct AITeam *)sub_4907A0(80025017);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v124);
v125 = (struct AITeam *)sub_4907A0(206658400);
kasFSMCreate((char *)0x4C51573, Init_Mission17_BeastRammingFrigateReinforcementTeam, Init_Mission17_BeastRammingFrigateReinforcementTeam_Reevaluate, v125);
v126 = (struct AITeam *)sub_4907A0(17798);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v126);
v127 = (struct AITeam *)sub_4907A0(284785);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v127);
v128 = (struct AITeam *)sub_4907A0(284786);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v128);
v129 = (struct AITeam *)sub_4907A0(284787);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v129);
v130 = (struct AITeam *)sub_4907A0(171501548);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v130);
v131 = (struct AITeam *)sub_4907A0(59670129);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v131);
v132 = (struct AITeam *)sub_4907A0(59670130);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v132);
v133 = (struct AITeam *)sub_4907A0(59670131);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v133);
v134 = (struct AITeam *)sub_4907A0(59670132);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v134);
v135 = (struct AITeam *)sub_4907A0(59670133);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v135);
v136 = (struct AITeam *)sub_4907A0(185124854);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v136);
v137 = (struct AITeam *)sub_4907A0(185124855);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v137);
v138 = (struct AITeam *)sub_4907A0(249715329);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v138);
v139 = (struct AITeam *)sub_4907A0(199714900);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v139);
v140 = (struct AITeam *)sub_4907A0(34980306);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v140);
v141 = (struct AITeam *)sub_4907A0(108453443);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v141);
v142 = (struct AITeam *)sub_4907A0(284788);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v142);
v143 = (struct AITeam *)sub_4907A0(80025010);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v143);
v144 = (struct AITeam *)sub_4907A0(80025011);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v144);
v145 = (struct AITeam *)sub_4907A0(80025012);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v145);
v146 = (struct AITeam *)sub_4907A0(59670134);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v146);
v147 = (struct AITeam *)sub_4907A0(59670135);
kasFSMCreate((char *)0xCF0DDA5, Init_Mission17_FinalAssaultLeadTeam, Watch_Mission17_FinalAssaultLeadTeam, v147);
v148 = (struct AITeam *)sub_4907A0(252566178);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v148);
v149 = (struct AITeam *)sub_4907A0(185124852);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v149);
v150 = (struct AITeam *)sub_4907A0(185124853);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v150);
v151 = (struct AITeam *)sub_4907A0(0x0A9E1F3);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v151);
v152 = (struct AITeam *)sub_4907A0(0x0A9E1F4);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v152);
v153 = (struct AITeam *)sub_4907A0(34980307);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v153);
v154 = (struct AITeam *)sub_4907A0(34980308);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v154);
v155 = (struct AITeam *)sub_4907A0(128634915);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v155);
v156 = (struct AITeam *)sub_4907A0(128634916);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v156);
v157 = (struct AITeam *)sub_4907A0(59670136);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v157);
v158 = (struct AITeam *)sub_4907A0(59670137);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v158);
v159 = (struct AITeam *)sub_4907A0(9207697);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v159);
v160 = (struct AITeam *)sub_4907A0(9207698);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v160);
v161 = (struct AITeam *)sub_4907A0(34980311);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v161);
v162 = (struct AITeam *)sub_4907A0(34980312);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v162);
v163 = (struct AITeam *)sub_4907A0(149415701);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v163);
v164 = (struct AITeam *)sub_4907A0(149415702);
kasFSMCreate((char *)0xF0DDAA2, Init_Mission17_FinalAssaultAssignWaveTeam, Watch_Mission17_FinalAssaultWaveController, v164);
v165 = (struct AITeam *)sub_4907A0(128425636);
kasFSMCreate((char *)0xBCC1A14, Init_Mission17_BeastHeavyCruiserCombatTeam, Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate, v165);
v166 = (struct AITeam *)sub_4907A0(175761953);
kasFSMCreate((char *)0xBCC1A14, Init_Mission17_BeastHeavyCruiserCombatTeam, Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate, v166);
v167 = (struct AITeam *)sub_4907A0(175761954);
kasFSMCreate((char *)0xBCC1A14, Init_Mission17_BeastHeavyCruiserCombatTeam, Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate, v167);
v168 = (struct AITeam *)sub_4907A0(175761955);
kasFSMCreate((char *)0xBCC1A14, Init_Mission17_BeastHeavyCruiserCombatTeam, Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate, v168);
v169 = (struct AITeam *)sub_4907A0(175761956);
kasFSMCreate((char *)0xBCC1A14, Init_Mission17_BeastHeavyCruiserCombatTeam, Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate, v169);
v170 = (struct AITeam *)sub_4907A0(175761957);
kasFSMCreate((char *)0xBCC1A14, Init_Mission17_BeastHeavyCruiserCombatTeam, Init_Mission17_BeastHeavyCruiserCombatTeam_Reevaluate, v170);
if ( GetGameDifficulty_() > 2 )
{
v314 = sub_4907E0(175575249);
v171 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v171, (int)v314, 1) )
{
v172 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v172);
}
v315 = sub_4907E0(175575250);
v173 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v173, (int)v315, 1) )
{
v174 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v174);
}
v316 = sub_4907E0(175575251);
v175 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v175, (int)v316, 1) )
{
v176 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v176);
}
v317 = sub_4907E0(175575252);
v177 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v177, (int)v317, 1) )
{
v178 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v178);
}
v318 = sub_4907E0(175575253);
v179 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v179, (int)v318, 1) )
{
v180 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v180);
}
v319 = sub_4907E0(175575254);
v181 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v181, (int)v319, 1) )
{
v182 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v182);
}
v320 = sub_4907E0(175575255);
v183 = (int *)kasGrowSelectionPtr(59379477);
if ( kasfShipsSelectFirstN(v183, (int)v320, 1) )
{
v184 = kasGrowSelectionPtr(59379477);
kasfDeleteShips(v184);
}
}
v185 = (struct AITeam *)sub_4907A0(145191172);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v185);
v186 = (struct AITeam *)sub_4907A0(175575249);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v186);
v187 = (struct AITeam *)sub_4907A0(175575250);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v187);
v188 = (struct AITeam *)sub_4907A0(175575251);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v188);
v189 = (struct AITeam *)sub_4907A0(175575252);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v189);
v190 = (struct AITeam *)sub_4907A0(175575253);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v190);
v191 = (struct AITeam *)sub_4907A0(175575254);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v191);
v192 = (struct AITeam *)sub_4907A0(175575255);
kasFSMCreate((char *)0x9C2E0E4, Init_Mission17_BeastMultiBeamFrigateCombatTeam, Init_Mission17_BeastMultiBeamFrigateCombatTeam_Reevaluate, v192);
v193 = (struct AITeam *)sub_4907A0(149415696);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v193);
v194 = (struct AITeam *)sub_4907A0(149415697);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v194);
v195 = (struct AITeam *)sub_4907A0(128634917);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v195);
v196 = (struct AITeam *)sub_4907A0(128634918);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v196);
v197 = (struct AITeam *)sub_4907A0(128634919);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v197);
v198 = (struct AITeam *)sub_4907A0(128634920);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v198);
v199 = (struct AITeam *)sub_4907A0(128634921);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v199);
v200 = (struct AITeam *)sub_4907A0(179110480);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v200);
v201 = (struct AITeam *)sub_4907A0(179110481);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v201);
v202 = (struct AITeam *)sub_4907A0(179110482);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v202);
v203 = (struct AITeam *)sub_4907A0(179110483);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v203);
v204 = (struct AITeam *)sub_4907A0(179110484);
kasFSMCreate((char *)0x7AAD0E3, Init_Mission17_BeastCorvetteReinforcementTeam, Init_Mission17_BeastCorvetteReinforcementTeam_Reevaluate, v204);
v205 = sub_4907E0(90801137);
kasfDeleteShips((int)v205);
v206 = sub_4907E0(90801138);
kasfDeleteShips((int)v206);
v207 = sub_4907E0(90801139);
kasfDeleteShips((int)v207);
v208 = sub_4907E0(90801140);
kasfDeleteShips((int)v208);
v209 = sub_4907E0(90801141);
kasfDeleteShips((int)v209);
v210 = (struct AITeam *)sub_4907A0(0x56983E);
kasFSMCreate((char *)0x5698433, Init_Mission17_BeastReconReinforcementTeam, Init_Mission17_BeastReconReinforcementTeam_Reevaluate, v210);
v211 = (struct AITeam *)sub_4907A0(90801137);
kasFSMCreate((char *)0x5698433, Init_Mission17_BeastReconReinforcementTeam, Init_Mission17_BeastReconReinforcementTeam_Reevaluate, v211);
v212 = (struct AITeam *)sub_4907A0(90801138);
kasFSMCreate((char *)0x5698433, Init_Mission17_BeastReconReinforcementTeam, Init_Mission17_BeastReconReinforcementTeam_Reevaluate, v212);
v213 = (struct AITeam *)sub_4907A0(90801139);
kasFSMCreate((char *)0x5698433, Init_Mission17_BeastReconReinforcementTeam, Init_Mission17_BeastReconReinforcementTeam_Reevaluate, v213);
v214 = (struct AITeam *)sub_4907A0(90801140);
kasFSMCreate((char *)0x5698433, Init_Mission17_BeastReconReinforcementTeam, Init_Mission17_BeastReconReinforcementTeam_Reevaluate, v214);
v215 = (struct AITeam *)sub_4907A0(90801141);
kasFSMCreate((char *)0x5698433, Init_Mission17_BeastReconReinforcementTeam, Init_Mission17_BeastReconReinforcementTeam_Reevaluate, v215);
v216 = (struct AITeam *)sub_4907A0(185124856);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v216);
v217 = (struct AITeam *)sub_4907A0(185124857);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v217);
v218 = (struct AITeam *)sub_4907A0(0x8C7F90);
kasFSMCreate((char *)0xB08C7B3, Init_Mission17_BeastHeavyCruiserReinforcementTeam, Init_Mission17_BeastHeavyCruiserReinforcementTeam_Reevaluate, v218);
v219 = (struct AITeam *)sub_4907A0(34980309);
kasFSMCreate((char *)0x215C113, Init_Mission08_HiveFrigateReinforcementTeam, Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate, v219);
v220 = (struct AITeam *)sub_4907A0(34980310);
kasFSMCreate((char *)0x215C113, Init_Mission08_HiveFrigateReinforcementTeam, Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate, v220);
v221 = (struct AITeam *)sub_4907A0(149415698);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v221);
v222 = (struct AITeam *)sub_4907A0(149415699);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v222);
v223 = (struct AITeam *)sub_4907A0(149415700);
kasFSMCreate((char *)0x38E7FB3, Init_BeastSentinelReinforcementTeam, Init_BeastSentinelReinforcementTeam_Reevaluate, v223);
v224 = (struct AITeam *)sub_4907A0(284789);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v224);
v225 = (struct AITeam *)sub_4907A0(284790);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v225);
v226 = (struct AITeam *)sub_4907A0(284791);
kasFSMCreate((char *)0x458B3, Init_Mission17_BeastACVReinforcementWave, Init_Mission17_BeastACVReinforcementWave_Reevaluate, v226);
v227 = sub_4907A0(108453444);
sub_494A00((int)v227, 362601);
v228 = (struct AITeam *)sub_4907A0(108453444);
kasFSMCreate((char *)0x676DE03, Init_Mission17_BeastCarrierReinforcementTeam, Init_Mission17_BeastCarrierReinforcementTeam_Reevaluate, v228);
v229 = (struct AITeam *)sub_4907A0(160537573);
kasFSMCreate((char *)0x919BE33, Init_Mission17_BeastCruiseMissileReinforcementTeam, Init_Mission17_BeastCruiseMissileReinforcementTeam_Reevaluate, v229);
v230 = sub_4907A0(160537573);
sub_494A00((int)v230, 4693508);
v231 = (struct AITeam *)sub_4907A0(152682225);
kasFSMCreate((char *)0x919BE33, Init_Mission17_BeastCruiseMissileReinforcementTeam, Init_Mission17_BeastCruiseMissileReinforcementTeam_Reevaluate, v231);
v232 = (struct AITeam *)sub_4907A0(18599);
kasFSMCreate((char *)0x48AC3, Init_Mission17_BeastDFGFrigateReinforcementTeam, Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate, v232);
v233 = (struct AITeam *)sub_4907A0(297601);
kasFSMCreate((char *)0x48AC3, Init_Mission17_BeastDFGFrigateReinforcementTeam, Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate, v233);
v234 = (struct AITeam *)sub_4907A0(297602);
kasFSMCreate((char *)0x48AC3, Init_Mission17_BeastDFGFrigateReinforcementTeam, Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate, v234);
v235 = (struct AITeam *)sub_4907A0(297603);
kasFSMCreate((char *)0x48AC3, Init_Mission17_BeastDFGFrigateReinforcementTeam, Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate, v235);
v236 = (struct AITeam *)sub_4907A0(297604);
kasFSMCreate((char *)0x48AC3, Init_Mission17_BeastDFGFrigateReinforcementTeam, Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate, v236);
v237 = (struct AITeam *)sub_4907A0(297605);
kasFSMCreate((char *)0x48AC3, Init_Mission17_BeastDFGFrigateReinforcementTeam, Init_Mission17_BeastDFGFrigateReinforcementTeam_Reevaluate, v237);
v238 = (struct AITeam *)sub_4907A0(84573278);
kasFSMCreate((char *)0x50A7C5E, Init_Mission17_BeastAdaptiveReinforcementController, Watch_Mission17_BeastAdaptiveReinforcementController, v238);
v239 = (struct AITeam *)sub_4907A0(129010738);
kasFSMCreate((char *)0xB08C303, Init_Mission17_BeastACVReinforcementTeam, Init_Mission17_BeastACVReinforcementTeam_Reevaluate, v239);
v240 = (struct AITeam *)sub_4907A0(185123649);
kasFSMCreate((char *)0xB08C303, Init_Mission17_BeastACVReinforcementTeam, Init_Mission17_BeastACVReinforcementTeam_Reevaluate, v240);
v241 = (struct AITeam *)sub_4907A0(185123650);
kasFSMCreate((char *)0xB08C303, Init_Mission17_BeastACVReinforcementTeam, Init_Mission17_BeastACVReinforcementTeam_Reevaluate, v241);
v242 = (struct AITeam *)sub_4907A0(185123651);
kasFSMCreate((char *)0xB08C303, Init_Mission17_BeastACVReinforcementTeam, Init_Mission17_BeastACVReinforcementTeam_Reevaluate, v242);
v243 = (struct AITeam *)sub_4907A0(185123652);
kasFSMCreate((char *)0xB08C303, Init_Mission17_BeastACVReinforcementTeam, Init_Mission17_BeastACVReinforcementTeam_Reevaluate, v243);
v244 = (struct AITeam *)sub_4907A0(124217682);
kasFSMCreate((char *)0x6769503, Init_Mission17_CarrierCombatTeam, Init_Mission17_CarrierCombatTeam_Reevaluate, v244);
v245 = (struct AITeam *)sub_4907A0(108434753);
kasFSMCreate((char *)0x6769503, Init_Mission17_CarrierCombatTeam, Init_Mission17_CarrierCombatTeam_Reevaluate, v245);
v246 = (struct AITeam *)sub_4907A0(108434754);
kasFSMCreate((char *)0x6769503, Init_Mission17_CarrierCombatTeam, Init_Mission17_CarrierCombatTeam_Reevaluate, v246);
v247 = (struct AITeam *)sub_4907A0(182180946);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v247);
v248 = (struct AITeam *)sub_4907A0(230540689);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v248);
v249 = (struct AITeam *)sub_4907A0(230540690);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v249);
v250 = (struct AITeam *)sub_4907A0(230540691);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v250);
v251 = (struct AITeam *)sub_4907A0(230540692);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v251);
v252 = (struct AITeam *)sub_4907A0(230540693);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v252);
v253 = (struct AITeam *)sub_4907A0(230540694);
kasFSMCreate((char *)0xDBDC5D3, Init_Mission17_HeavyInterceptorCombatTeam, Init_Mission17_HeavyInterceptorCombatTeam_Reevaluate, v253);
v254 = (struct AITeam *)sub_4907A0(122695973);
kasFSMCreate((char *)0x50312D3, Init_Mission17_AdvanceSupportFrigateCombatTeam, Init_Mission17_AdvanceSupportFrigateCombatTeam_Reevaluate, v254);
v255 = (struct AITeam *)sub_4907A0(84087313);
kasFSMCreate((char *)0x50312D3, Init_Mission17_AdvanceSupportFrigateCombatTeam, Init_Mission17_AdvanceSupportFrigateCombatTeam_Reevaluate, v255);
v256 = (struct AITeam *)sub_4907A0(84087314);
kasFSMCreate((char *)0x50312D3, Init_Mission17_AdvanceSupportFrigateCombatTeam, Init_Mission17_AdvanceSupportFrigateCombatTeam_Reevaluate, v256);
v257 = (struct AITeam *)sub_4907A0(84087315);
kasFSMCreate((char *)0x50312D3, Init_Mission17_AdvanceSupportFrigateCombatTeam, Init_Mission17_AdvanceSupportFrigateCombatTeam_Reevaluate, v257);
v258 = (struct AITeam *)sub_4907A0(205745797);
kasFSMCreate((char *)0x436E863, Init_Mission17_IonCannonFrigateReinforcementTeam, Init_Mission17_IonCannonFrigateReinforcementTeam_Reevaluate, v258);
v259 = (struct AITeam *)sub_4907A0(70707361);
kasFSMCreate((char *)0x436E863, Init_Mission17_IonCannonFrigateReinforcementTeam, Init_Mission17_IonCannonFrigateReinforcementTeam_Reevaluate, v259);
v260 = (struct AITeam *)sub_4907A0(70707362);
kasFSMCreate((char *)0x436E863, Init_Mission17_IonCannonFrigateReinforcementTeam, Init_Mission17_IonCannonFrigateReinforcementTeam_Reevaluate, v260);
v261 = (struct AITeam *)sub_4907A0(70707363);
kasFSMCreate((char *)0x436E863, Init_Mission17_IonCannonFrigateReinforcementTeam, Init_Mission17_IonCannonFrigateReinforcementTeam_Reevaluate, v261);
v262 = (struct AITeam *)sub_4907A0(100221730);
kasFSMCreate((char *)0xF943223, Init_Mission17_StandardDestroyerCombatTeam, Init_Mission17_StandardDestroyerCombatTeam_Reevaluate, v262);
v263 = (struct AITeam *)sub_4907A0(261370465);
kasFSMCreate((char *)0xF943223, Init_Mission17_StandardDestroyerCombatTeam, Init_Mission17_StandardDestroyerCombatTeam_Reevaluate, v263);
v264 = (struct AITeam *)sub_4907A0(261370466);
kasFSMCreate((char *)0xF943223, Init_Mission17_StandardDestroyerCombatTeam, Init_Mission17_StandardDestroyerCombatTeam_Reevaluate, v264);
v265 = (struct AITeam *)sub_4907A0(75163141);
kasFSMCreate((char *)0x7AE60E3, Init_Mission17_MultigunCorvetteReinforcementTeam, Init_Mission17_MultigunCorvetteReinforcementTeam_Reevaluate, v265);
v266 = (struct AITeam *)sub_4907A0(128868385);
kasFSMCreate((char *)0x7AE60E3, Init_Mission17_MultigunCorvetteReinforcementTeam, Init_Mission17_MultigunCorvetteReinforcementTeam_Reevaluate, v266);
v267 = (struct AITeam *)sub_4907A0(128868386);
kasFSMCreate((char *)0x7AE60E3, Init_Mission17_MultigunCorvetteReinforcementTeam, Init_Mission17_MultigunCorvetteReinforcementTeam_Reevaluate, v267);
v268 = sub_4907E0(108434753);
sub_493AF0((int)v268, 40);
v269 = sub_4907E0(185123649);
sub_493AF0((int)v269, 40);
v270 = sub_4907A0(185123649);
sub_494A00((int)v270, 237188773);
v271 = sub_4907A0(108434753);
sub_494A00((int)v271, 237188773);
v272 = (struct AITeam *)sub_4907A0(111385145);
kasFSMCreate((char *)0x6A39A39, Init_Mission17_BentusiAttackMothership, Watch_Mission17_BentusiAttackMothership, v272);
v273 = (struct AITeam *)sub_4907A0(132678291);
kasFSMCreate((char *)0x7E88293, Init_Mission17_CrystalPointTrigger, Watch_Mission17_CrystalPointTrigger, v273);
v274 = (struct AITeam *)sub_4907A0(140367886);
kasFSMCreate((char *)0x85DD80E, Init_Mission17_TaiidanAdaptiveReinforcementController, Watch_Mission17_TaiidanAdaptiveReinforcementController, v274);
v275 = (struct AITeam *)sub_4907A0(40027877);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v275);
v276 = (struct AITeam *)sub_4907A0(121712933);
kasFSMCreate((char *)0x41312D3, Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam, Init_Mission17_TaiidanAdvanceSupportFrigateSupportTeam_Reevaluate, v276);
v277 = (struct AITeam *)sub_4907A0(124217378);
kasFSMCreate((char *)0x6768203, Init_Mission17_CarrierReinforcementTeam, Init_Mission17_CarrierReinforcementTeam_Reevaluate, v277);
v278 = (struct AITeam *)sub_4907A0(0x0B08C22);
kasFSMCreate((char *)0xB08C273, Init_Mission17_HeavyCruiserReinforcementTeam, Init_Mission17_HeavyCruiserReinforcementTeam_Reevaluate, v278);
v279 = (struct AITeam *)sub_4907A0(209088133);
kasFSMCreate((char *)0x766E863, Init_Mission17_TaiidanIonCannonFrigateCombatTeam, Init_Mission17_TaiidanIonCannonFrigateCombatTeam_Reevaluate, v279);
v280 = (struct AITeam *)sub_4907A0(75165957);
kasFSMCreate((char *)0x7AF10E3, Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam, Init_Mission17_TaiidanMultigunCorvetteReinforcementTeam_Reevaluate, v280);
v281 = (struct AITeam *)sub_4907A0(90784546);
kasFSMCreate((char *)0x6943223, Init_Mission17_TaiidanStandardDestroyerCombatTeam, Init_Mission17_TaiidanStandardDestroyerCombatTeam_Reevaluate, v281);
v282 = (struct AITeam *)sub_4907A0(68358673);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v282);
v283 = (struct AITeam *)sub_4907A0(68358674);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v283);
v284 = (struct AITeam *)sub_4907A0(108429889);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v284);
v285 = (struct AITeam *)sub_4907A0(108429890);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v285);
v286 = (struct AITeam *)sub_4907A0(185123377);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v286);
v287 = (struct AITeam *)sub_4907A0(185123378);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v287);
v288 = (struct AITeam *)sub_4907A0(124184737);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v288);
v289 = (struct AITeam *)sub_4907A0(124184738);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v289);
v290 = (struct AITeam *)sub_4907A0(267788193);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v290);
v291 = (struct AITeam *)sub_4907A0(267788194);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v291);
v292 = (struct AITeam *)sub_4907A0(267788195);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v292);
v293 = (struct AITeam *)sub_4907A0(267788196);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v293);
v294 = (struct AITeam *)sub_4907A0(267788197);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v294);
v295 = (struct AITeam *)sub_4907A0(110375521);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v295);
v296 = (struct AITeam *)sub_4907A0(110375522);
kasFSMCreate((char *)0x262C6E5, Init_Mission17_TaiidanMultigunCorvetteCombatTeam, Init_Mission17_TaiidanMultigunCorvetteCombatTeam_Reevaluate, v296);
v297 = (struct AITeam *)sub_4907A0(133880113);
kasFSMCreate((char *)0x57FAD95, Init_Mission17_FinalAmbushTeam, Watch_Mission17_FinalAmbushTeam, v297);
v298 = (struct AITeam *)sub_4907A0(133880114);
kasFSMCreate((char *)0x57FAD95, Init_Mission17_FinalAmbushTeam, Watch_Mission17_FinalAmbushTeam, v298);
v299 = sub_4907A0(171501548);
sub_494A00((int)v299, 4693508);
v300 = sub_4907A0(171501548);
sub_494A00((int)v300, 4693508);
v301 = sub_4907A0(182180066);
sub_494A00((int)v301, 4693508);
v302 = sub_4907A0(223105365);
sub_494A00((int)v302, 4693508);
v303 = sub_4907A0(17798);
sub_494A00((int)v303, 4693508);
v304 = sub_4907A0(17798);
sub_494A00((int)v304, 4693508);
v305 = sub_4907A0(96759714);
sub_494A00((int)v305, 4693508);
v306 = sub_4907A0(62813829);
sub_494A00((int)v306, 4693508);
v307 = sub_4907A0(200156578);
sub_494A00((int)v307, 4693508);
v308 = sub_4907A0(182180946);
sub_494A00((int)v308, 4693508);
v309 = sub_4907A0(100221730);
sub_494A00((int)v309, 4693508);
v310 = sub_4907A0(205745797);
sub_494A00((int)v310, 4693508);
v311 = sub_4907A0(122695973);
sub_494A00((int)v311, 4693508);
v312 = sub_4907A0(75163141);
sub_494A00((int)v312, 4693508);
kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
kasfAllowPlayerToResearch("TECH_ADRIVE_FIGHTER");
kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");
kasfAllowPlayerToResearch("TECH_ADRIVE_FRIGATE");
kasfAllowPlayerToResearch("TECH_DRIVE_DESTROYER");
kasfAllowPlayerToResearch("TECH_ADRIVE_DESTROYER");
kasfAllowPlayerToResearch("TECH_ARMOUR2");
kasfAllowPlayerToResearch("TECH_ARMOUR3");
kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
kasfAllowPlayerToResearch("TECH_ENERGY");
kasfAllowPlayerToResearch("TECH_ION");
kasfAllowPlayerToResearch("TECH_MISSILE");
kasfAllowPlayerToResearch("TECH_EMP");
kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
kasfAllowPlayerToResearch("TECH_LINK");
kasfAllowPlayerToResearch("TECH_MIMIC");
kasfAllowPlayerToResearch("TECH_AFTERBURNER");
kasfAllowPlayerToResearch("TECH_BIG_GUN");
kasfAllowPlayerToResearch("TECH_ADVANCED_SENSORS");
kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");
kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");
kasfAllowPlayerToResearch("TECH_FORCE_FIELD3");
kasfAllowPlayerToResearch("TECH_REPAIRBOTS");
kasfAllowPlayerToResearch("TECH_CRYSTAL_HARVEST");
sub_497F10(0, "sRecon");
sub_497F10(0, "sAcolyte");
sub_497F10(0, "sSentinel");
sub_497F10(0, "sLeech");
sub_497F10(0, "sACV");
sub_497F10(0, "sWorker");
sub_497F10(0, "sRammingFrigate");
sub_497F10(0, "sHiveFrigate");
sub_497F10(0, "sMultiBeamFrigate");
sub_497F70(0, 5);
sub_497F10(1, "bAcolyte");
sub_497F10(1, "bACV");
sub_497F10(1, "bMissileCorvette");
sub_497F10(1, "tMultigunCorvette");
sub_497F10(1, "bWorker");
sub_497F10(1, "bIonArrayFrigate");
sub_497F10(1, "bRammingFrigate");
sub_497F10(1, "tIonCannonFrigate");
sub_497F70(1, 4);
sub_497F10(2, "Asteroids");
kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
kasfEnableShip("sMothershipSpecial", "RACE_SECT", 1);
kasfEnableShip("sMothershipArmour", "RACE_SECT", 1);
kasfEnableShip("sMothershipWeapons", "RACE_SECT", 1);
kasfEnableShip("sMothershipMicro", "RACE_SECT", 1);
kasfEnableShip("dAcolyte", "RACE_SECT", 0);
sub_498230();
kasfIntelEventNotEnded();
kasfLocationCard(8000, (char *)m17_locationCard[strCurLanguage]);
sub_491F90(104338707, 1);
sub_491F90(220447043, 600);
}
