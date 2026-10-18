//
//  KASRemap.c
//
//  Save files store AITeam FSM watch function pointers as signed offsets
//  relative to the original cataclysm.exe Watch_Mission address for the
//  current mission.  A per-mission lookup table maps between the original
//  binary offsets and our build's function pointers so that save files from
//  both the original binary and our own builds can be loaded correctly.
//
//  See docs/KASRemap.md for a full explanation and instructions for adding
//  tables for new missions.
//

#include "kas.h"
#include "singleplayer.h"
#include "MissionScriptSupport.h"
#include "Debug.h"

typedef struct KASFuncOffsetEntry
{
    sdword  originalOffset;
    void   *ourFuncPtr;
} KASFuncOffsetEntry;

extern void Watch_Tutorial1(void);
extern void Watch_Mission02(void);
extern void Watch_Mission03(void);
extern void Watch_Mission04(void);
extern void Watch_Mission05(void);
extern void Watch_Mission06(void);
extern void Watch_Mission07(void);
extern void Watch_Mission08(void);
extern void Watch_Mission09(void);
extern void Watch_Mission10(void);
extern void Watch_Mission11(void);
extern void Watch_Mission12(void);
extern void Watch_Mission13(void);
extern void Watch_Mission14(void);
extern void Watch_Mission15(void);
extern void Watch_Mission16(void);
extern void Watch_Mission17(void);

static KASWatchFunction kasWatchFunctionByIndex(sdword index)
{
    switch (index)
    {
        case 0:  return Watch_Mission01;
        case 1:  return Watch_Mission02;
        case 2:  return Watch_Mission03;
        case 3:  return Watch_Mission04;
        case 4:  return Watch_Mission05;
        case 5:  return Watch_Mission06;
        case 6:  return Watch_Mission07;
        case 7:  return Watch_Mission08;
        case 8:  return Watch_Mission09;
        case 9:  return Watch_Mission10;
        case 10: return Watch_Mission11;
        case 11: return Watch_Mission12;
        case 12: return Watch_Mission13;
        case 13: return Watch_Mission14;
        case 14: return Watch_Mission15;
        case 15: return Watch_Mission16;
        case 16: return Watch_Mission17;
        case 17: return Watch_Tutorial1;
        default: return NULL;
    }
}

sdword WatchFunctionToIndex(KASWatchFunction watchFunction)
{
    sdword index;

    for (index = 0; index < 20; index++)
    {
        if (watchFunction == kasWatchFunctionByIndex(index))
        {
            return index;
        }
    }

    return -1;
}

KASWatchFunction IndexToWatchFunction(sdword index)
{
    return kasWatchFunctionByIndex(index);
}

/* ===========================================================================
   Mission 01 remap table
   All offsets are relative to Watch_Mission01 = 0x5DB150 in cataclysm.exe.
   =========================================================================== */

extern void Init_Mission01_SetupTutorialState(void);
extern void Init_Mission01_ActivateTutorialFlow(void);
extern void Watch_Mission01_TutorialFlow(void);
extern void Watch_Mission01_TutorialPromptCleanup(void);
extern void Watch_Mission01_TutorialResumeDelay(void);
extern void Watch_Mission01_TutorialEventFlow(void);
extern void FailMissionIfTrackedSelectionAlive(void);
extern void Init_Mission01_Objectives(void);
extern void Watch_Mission01_Objectives(void);
extern void Init_Mission01_CarrierTeam_Activate(void);
extern void Watch_Mission01_MissionFlow_NISing(void);
extern void Watch_Mission01_MissionFlow_CleanupFirstNISTarget(void);
extern void Watch_Mission01_MissionFlow_WaitForFirstNISTargetDestroyed(void);
extern void Watch_Mission01_MissionFlow_CleanupSecondNISTarget(void);
extern void Watch_Mission01_MissionFlow_WaitForSecondNISTargetDestroyed(void);
extern void Watch_Mission01_MissionFlow_SelectAsteroidPocketAttackers(void);
extern void airAddedResearchShip(Ship *ship);
extern void Init_Mission01_ResearchShipTeam_Activate(void);
extern void Init_Mission01_GarrisonObjective_Start(void);
extern void Init_Mission01_GarrisonObjective_FocusTarget(void);
extern void Init_Mission01_GarrisonObjective_WaitForTrigger(void);
extern void Init_Mission01_GarrisonObjective_ClearTrackedSelection(void);
extern void Init_Mission01_GarrisonObjective_WaitForTrackedSelectionDestroyed(void);
extern void Init_Mission01_CarrierObjective_Start(void);
extern void Init_Mission01_CarrierObjective_BeginTracking(void);
extern void Init_Mission01_CarrierObjective_UpdateNearbyAttackers(void);
extern void Init_Mission01_CarrierObjective_ClearTrackedCarrier(void);
extern void Init_Mission01_AsteroidPocketAttackTeam_Start(void);
extern void Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers(void);
extern void Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers(void);
extern void Init_Mission01_AsteroidPocketEncounter_Start(void);
extern void Init_Mission01_AsteroidPocketEncounter_FocusTarget(void);
extern void Watch_Mission01_AsteroidPocketEncounter_AfterFocus(void);
extern void Watch_Mission01_AsteroidPocketEncounter_SelectAttackers(void);
extern void Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers(void);
extern void Init_Mission01_ControllerPocketObjective_Start(void);
extern void Init_Mission01_ControllerPocketObjective_SelectPocketShips(void);
extern void Init_Mission01_ControllerPocketObjective_WaitForPocketReached(void);
extern void Init_Mission01_ControllerPocketObjective_FocusPocket(void);
extern void Init_Mission01_ControllerPocketObjective_WaitForSpeechDelay(void);
extern void Init_Mission01_ProximitySensorObjective_Start(void);
extern void Init_Mission01_ProximitySensorObjective_WaitForTrigger(void);
extern void Init_ApplyPlayerUpgradesToCurrentTeam(void);
extern void Init_Mission01_ApplyUpgradesThenProtectResourcers(void);
extern void Init_Mission01_HarvestController_MoveToAsteroidPocket(void);
extern void Init_Mission01_HarvestController_UpdateResourceFlow(void);
extern void Init_Mission01_HarvestController_SelectShips(void);
extern void Init_Mission01_DockCurrentTeamForLaunch(void);
extern void Watch_Mission01_MissionFlow_FirstWarning(void);
extern void Watch_Mission01_MissionFlow_FirstWarningDelay(void);
extern void Watch_Mission01_MissionFlow_AttackInClawFormation(void);
extern void Watch_Mission01_MissionFlow_SelectNearbyAttackersOrQueueReinforcements(void);
extern void Init_Mission01_QueueMultigunCorvetteOrHeavyInterceptorBatch(void);
extern void Watch_Mission01_MultigunCorvetteOrHeavyInterceptorBatchComplete(void);
extern void Watch_Mission01_MissionFlow_FleetCommandIntro(void);
extern void Init_ApplyUpgradesThenOpenMainScreen(void);
extern void Watch_Mission01_OpenMainScreenLaunchOnSignal(void);
extern void spMainScreen(void);
extern void Watch_Mission09_MissionFlow_SelectNearbyAttackersOrQueueStandardFrigateBatch(void);
extern void Watch_Mission09_MissionFlow_GhostIntro(void);
extern void Init_Mission01_ApplyUpgradesThenDockForLaunch(void);
extern void Init_SelectAndFocusProximitySensors(void);
extern void Watch_ProximitySensorContactLoop(void);
extern void Init_Mission01_QueueHeavyInterceptorBatch(void);
extern void Watch_Mission01_HeavyInterceptorBatchComplete(void);
extern void Watch_Mission01_AsteroidPocketAttackTeam_Intro(void);
extern void Init_Mission01_AsteroidPocketEncounter_FocusThreatWarning(void);
extern void Init_AsteroidPocketEncounter_ClearCurrentSelectionHighlight(void);
extern void cpHueSatImageDelete(void);
extern void Init_Mission01_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor(void);
extern void Watch_Mission01_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady(void);
extern void Watch_Mission01_AsteroidPocketEncounter_ResumeAttackers(void);
extern void Watch_Mission01_AsteroidPocketEncounter_ThreatWarningAcknowledged(void);
extern void Init_AsteroidPocketEncounter_DefenseLoop(void);
extern void Watch_AsteroidPocketEncounter_DefenseLoop(void);
extern void Init_AsteroidPocketEncounter_ReinforcementDelay(void);
extern void Watch_AsteroidPocketEncounter_ReinforcementDelay(void);
extern void Watch_AsteroidPocketEncounter_ReinforcementReady(void);
extern void Watch_AsteroidPocketEncounter_BeginDefenseLoop(void);
extern void Watch_AsteroidPocketEncounter_InitialAttackerWarningAcknowledged(void);
extern void Init_AsteroidPocketEncounter_SelectInitialAttackers(void);
extern void Init_AsteroidPocketEncounter_InitialAttackerWarning(void);
extern void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void);
extern void Init_WaitForIntelEventEnd(void);
extern void MaintainTeamContext(void);
extern void Init_Mission01(void);
extern void Watch_Mission01(void);

static const KASFuncOffsetEntry kasOffsetTable_Mission01[] =
{
    {  -5936, Init_Mission01_SetupTutorialState     },
    {  -5728, Init_Mission01_ActivateTutorialFlow   },
    {  -5680, Watch_Mission01_TutorialFlow          },
    {  -4336, Watch_Mission01_TutorialPromptCleanup },
    {  -3904, Watch_Mission01_TutorialResumeDelay   },
    {  -3728, Watch_Mission01_TutorialEventFlow     },
    { -16752, Init_Mission01_Objectives             },
    { -16640, Watch_Mission01_Objectives            },
    { -14464, Watch_Mission01_MissionFlow_NISing    },
    { -14320, Watch_Mission01_MissionFlow_CleanupFirstNISTarget },
    { -14272, Watch_Mission01_MissionFlow_WaitForFirstNISTargetDestroyed },
    { -14144, Watch_Mission01_MissionFlow_CleanupSecondNISTarget },
    { -14096, Watch_Mission01_MissionFlow_WaitForSecondNISTargetDestroyed },
    { -14000, Watch_Mission01_MissionFlow_SelectAsteroidPocketAttackers },
    { -13776, Init_Mission01_ResearchShipTeam_Activate },
    { -13680, airAddedResearchShip                  },
    { -13616, Init_Mission01_GarrisonObjective_Start },
    { -13504, Init_Mission01_GarrisonObjective_FocusTarget },
    { -13456, Init_Mission01_GarrisonObjective_WaitForTrigger },
    { -13376, Init_Mission01_GarrisonObjective_ClearTrackedSelection },
    { -13328, Init_Mission01_GarrisonObjective_WaitForTrackedSelectionDestroyed },
    { -13248, Init_Mission01_CarrierTeam_Activate   },
    { -13152, Init_Mission01_CarrierObjective_Start },
    { -13088, Init_Mission01_CarrierObjective_BeginTracking },
    { -13008, Init_Mission01_CarrierObjective_UpdateNearbyAttackers },
    { -12784, Init_Mission01_CarrierObjective_ClearTrackedCarrier },
    { -12736, Init_Mission01_ApplyUpgradesThenProtectResourcers },
    { -12672, Init_Mission01_HarvestController_MoveToAsteroidPocket },
    { -12608, Init_Mission01_HarvestController_UpdateResourceFlow },
    { -12384, Init_Mission01_HarvestController_SelectShips },
    { -12320, Init_Mission01_ControllerPocketObjective_Start },
    { -12240, Init_Mission01_ControllerPocketObjective_SelectPocketShips },
    { -12080, Init_Mission01_ControllerPocketObjective_WaitForPocketReached },
    { -11888, Init_Mission01_ControllerPocketObjective_FocusPocket },
    { -11792, Init_Mission01_ControllerPocketObjective_WaitForSpeechDelay },
    { -11744, Init_ApplyUpgradesThenOpenMainScreen  },
    { -11680, Watch_Mission01_OpenMainScreenLaunchOnSignal },
    { -11600, Watch_Mission09_MissionFlow_SelectNearbyAttackersOrQueueStandardFrigateBatch },
    { -11440, Init_QueueStandardFrigateBatch       },
    { -11328, Watch_StandardFrigateBatchComplete   },
    { -11216, Watch_Mission09_MissionFlow_GhostIntro },
    { -11152, Init_ApplyPlayerUpgradesToCurrentTeam },
    { -11104, Init_Mission01_ApplyUpgradesThenDockForLaunch },
    { -11040, Init_Mission01_DockCurrentTeamForLaunch },
    { -10992, Watch_Mission01_MissionFlow_FirstWarning },
    { -10896, Watch_Mission01_MissionFlow_FirstWarningDelay },
    { -10784, Watch_Mission01_MissionFlow_AttackInClawFormation },
    { -10752, Watch_Mission01_MissionFlow_SelectNearbyAttackersOrQueueReinforcements },
    { -10592, Init_QueueMultigunCorvetteOrHeavyInterceptorBatch },
    { -10448, Watch_MultigunCorvetteOrHeavyInterceptorBatchComplete },
    { -10336, Watch_Mission01_MissionFlow_FleetCommandIntro },
    { -10272, Init_Mission01_ProximitySensorObjective_Start },
    { -10192, Init_Mission01_ProximitySensorObjective_WaitForTrigger },
    { -10112, Init_SelectAndFocusProximitySensors   },
    {  -9968, Watch_ProximitySensorContactLoop      },
    {  -9600, Init_Mission01_AsteroidPocketAttackTeam_Start },
    {  -9504, Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers },
    {  -9264, Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers },
    {  -8912, Init_Mission01_QueueHeavyInterceptorBatch },
    {  -8800, Watch_Mission01_HeavyInterceptorBatchComplete },
    {  -8688, Watch_Mission01_AsteroidPocketAttackTeam_Intro },
    {  -8624, Init_Mission01_AsteroidPocketEncounter_Start },
    {  -8496, Init_Mission01_AsteroidPocketEncounter_FocusTarget },
    {  -8448, Watch_Mission01_AsteroidPocketEncounter_AfterFocus },
    {  -8320, Watch_Mission01_AsteroidPocketEncounter_SelectAttackers },
    {  -8144, Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers },
    {  -7856, Init_AsteroidPocketEncounter_ClearCurrentSelectionHighlight },
    {  -7808, cpHueSatImageDelete                   },
    {  -7744, Init_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor },
    {  -7744, Init_Mission01_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor },
    {  -7632, Watch_Mission01_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady },
    {  -7520, Watch_Mission01_AsteroidPocketEncounter_ResumeAttackers },
    {  -7456, Init_AsteroidPocketEncounter_FocusThreatWarning },
    {  -7360, Watch_AsteroidPocketEncounter_ThreatWarningAcknowledged },
    {  -7280, Init_AsteroidPocketEncounter_DefenseLoop },
    {  -6992, Watch_AsteroidPocketEncounter_DefenseLoop },
    {  -6608, Init_AsteroidPocketEncounter_ReinforcementDelay },
    {  -6560, Watch_AsteroidPocketEncounter_ReinforcementDelay },
    {  -6480, Watch_AsteroidPocketEncounter_ReinforcementReady },
    {  -6368, Watch_AsteroidPocketEncounter_BeginDefenseLoop },
    {  -6240, Init_AsteroidPocketEncounter_InitialAttackerWarning },
    {  -6176, Watch_AsteroidPocketEncounter_InitialAttackerWarningAcknowledged },
    {  -6096, Init_AsteroidPocketEncounter_SelectInitialAttackers },
    {   -160, FailMissionIfTrackedSelectionAlive    },
    {    -96, Init_Mission01                        },
    {      0, Watch_Mission01                       },
    { 103904, Init_WaitForIntelEventEnd             },
    { 231408, Init_Mission11_MissionFlow_ClearIntelAdvanceFlag },
    { 334128, Watch_NoOpTeamContext                 },
    { 490912, MaintainTeamContext                   },
    {       0, NULL }
};

/* ===========================================================================
   Mission 02 remap table
   All offsets are relative to Watch_Mission02 = 0x5E0BB0 in cataclysm.exe.
   =========================================================================== */

extern void Init_Mission02_HonourGuardSwarmPatrol(void);
extern void Watch_Mission02_HonourGuardSwarmPatrol(void);
extern void Init_Mission02_HonourGuardSwarmMoveToRandomPoint(void);
extern void Watch_Mission02_HonourGuardSwarmDock(void);
extern void Init_Mission02_HonourGuardSwarmCollectNearbyShips(void);
extern void Watch_Mission02_HonourGuardSwarmCollectNearbyShips(void);
extern void Init_Mission02_SetupMissionFlowState(void);
extern void Watch_Mission02_PromptCleanup(void);
extern void Watch_Mission02_MissionFlowDelay(void);
extern void Watch_Mission02_MissionFlow(void);
extern void Watch_Mission02_ObjectiveEventQueue(void);
extern void Watch_Mission02_DispatchPirateWaves(void);
extern void Init_Mission02_PirateAssaultTeam(void);
extern void Watch_Mission02_PirateAssaultTeam(void);
extern void Init_Mission02_PirateMissileCorvetteAttackRun(void);
extern void Watch_Mission02_PirateMissileCorvetteAttackRun(void);
extern void Init_Mission02_PirateMissileCorvetteRetarget(void);
extern void Watch_Mission02_PirateMissileCorvetteRetarget(void);
extern void Init_Mission02_PirateMissileCorvetteTeam(void);
extern void Watch_Mission02_PirateMissileCorvetteTeam(void);
extern void Watch_Mission02_PirateMissileCorvetteReinforcements(void);
extern void Init_Mission02_PirateCarrierLaunchCycle(void);
extern void Watch_Mission02_PirateCarrierLaunchCycle(void);
extern void Init_Mission02_PirateStrikeTeamWave(void);
extern void Watch_Mission02_PirateStrikeTeamWave(void);
extern void Watch_Mission02_PirateStrikeTeamRedirect(void);
extern void Init_Mission02_PirateStrikeTeamAttackRun(void);
extern void Watch_Mission02_PirateStrikeTeamAttackRun(void);
extern void Init_Mission02_PirateStrikeTeamRetarget(void);
extern void Watch_Mission02_PirateStrikeTeamRetarget(void);
extern void Init_Mission02_PirateStrikeTeamReinforcementDelay(void);
extern void Watch_Mission02_PirateStrikeTeamReinforcements(void);
extern void Watch_Mission02_PirateStrikeTeamDockCycle(void);
extern void Watch_Mission02_PirateStrikeTeamPostLaunch(void);
extern void Init_Mission02_PirateHeavyStrikeTeam(void);
extern void Watch_Mission02_PirateHeavyStrikeTeam(void);
extern void Watch_Mission02_PirateHeavyStrikeTeamRedirect(void);
extern void Init_Mission02_PirateHeavyStrikeTeamAttackRun(void);
extern void Watch_Mission02_PirateHeavyStrikeTeamAttackRun(void);
extern void Init_Mission02_PirateHeavyStrikeTeamReinforcementDelay(void);
extern void Watch_Mission02_PirateHeavyStrikeTeamReinforcements(void);
extern void Watch_Mission02_PirateHeavyStrikeTeamDockCycle(void);
extern void Watch_Mission02_PirateHeavyStrikeTeamPostLaunch(void);
extern void Init_Mission02_ReassignSurvivingAttackTeam(void);
extern void Init_Mission02_PirateSalvageCorvetteTeam(void);
extern void Watch_Mission02_PirateSalvageCorvetteTeam(void);
extern void Watch_Mission02_PirateSalvageCorvetteIntro(void);
extern void Init_Mission02_PirateSalvageCorvetteDelay(void);
extern void Watch_Mission02_PirateSalvageCorvetteDelay(void);
extern void Init_Mission02_PirateSalvageCorvetteStateChoice(void);
extern void Init_Mission02_PirateSalvageCorvetteSpecialAttack(void);
extern void Watch_Mission02_PirateSalvageCorvetteSpecialAttack(void);
extern void Init_Mission02_PirateSalvageCorvetteRetarget(void);
extern void Watch_Mission02_PirateSalvageCorvetteRetarget(void);
extern void Init_Mission02_PirateCarrierStrikeReinforcementCycle(void);
extern void Watch_Mission02_PirateCarrierStrikeReinforcementCycle(void);
extern void Init_Mission02_PirateSalvageCorvetteCleanup(void);
extern void Init_Mission02_PirateReserveAttackTeam(void);
extern void Watch_Mission02_PirateReserveAttackTeam(void);
extern void Init_Mission02_PirateReserveAttackRun(void);
extern void Watch_Mission02_PirateReserveAttackRun(void);
extern void Init_Mission02_SalvagePodApproach(void);
extern void Init_Mission02_SalvagePodHideDelay(void);
extern void Watch_Mission02_SalvagePodHideDelay(void);
extern void Watch_Mission02_SalvagePodApproach(void);
extern void Init_Mission02_SalvagePodAmbushDelay(void);
extern void Watch_Mission02_SalvagePodAmbushDelay(void);
extern void Init_Mission02_DamagedDestroyer(void);
extern void Watch_Mission02_DamagedDestroyer(void);
extern void Watch_Mission02_DamagedDestroyerRepairStart(void);
extern void Watch_Mission02_DamagedDestroyerRepairProgress(void);
extern void Watch_Mission02_DamagedDestroyerAttackCue(void);
extern void Init_Mission02_DamagedDestroyerDespawnDelay(void);
extern void Watch_Mission02_DamagedDestroyerDespawnDelay(void);
extern void Init_Mission02_PirateAdvanceSupportFrigateTeam(void);
extern void Watch_Mission02_PirateAdvanceSupportFrigateTeam(void);
extern void Init_Mission02_PirateAdvanceSupportFrigateRepairOrder(void);
extern void Watch_Mission02_PirateAdvanceSupportFrigateRepairOrder(void);
extern void Init_Mission02_PirateAdvanceSupportFrigateDefenseLoop(void);
extern void Watch_Mission02_PirateAdvanceSupportFrigateDefenseLoop(void);
extern void Init_Mission02_PirateTeamReinforcementTeam(void);
extern void Watch_Mission02_PirateTeamReinforcementTeam(void);
extern void Watch_Mission02_PirateTeamReinforcements(void);
extern void Init_Mission02_PirateTeamAttackDelay(void);
extern void Watch_Mission02_PirateTeamAttackDelay(void);
extern void Init_Mission02_PirateTeamDefenseLoop(void);
extern void Watch_Mission02_PirateTeamDefenseLoop(void);
extern void Init_Mission02_PirateTeamReinforcementDelay(void);
extern void Watch_Mission02_PirateTeamReinforcementDelay(void);
extern void Init_Mission02_PirateTeamReinforcementGate(void);
extern void Init_Mission02_AttackMothershipTeam(void);
extern void Watch_Mission02_AttackMothershipTeam(void);
extern void Mission02_SetAttackApproachVector(void);
extern void Mission02_SendTeamToAttackTarget(void);
extern void Watch_Mission02_TeamState_5E03D0(void);
extern void Init_Mission02_TeamState_5E0440(void);
extern void Watch_Mission02_TeamState_5E0540(void);
extern void Init_Mission02(void);
extern void Watch_Mission13_MissionFlow_WaitForIntelEventEnd(void);

static const KASFuncOffsetEntry kasOffsetTable_Mission02[] =
{
    { -22800, Init_Mission02_HonourGuardSwarmPatrol              },
    { -22512, Watch_Mission02_HonourGuardSwarmPatrol             },
    { -22320, Init_Mission02_HonourGuardSwarmMoveToRandomPoint   },
    { -22048, Watch_Mission02_HonourGuardSwarmDock               },
    { -21984, Init_Mission02_HonourGuardSwarmCollectNearbyShips  },
    { -21696, Watch_Mission02_HonourGuardSwarmCollectNearbyShips },
    { -21568, Init_Mission02_SetupMissionFlowState               },
    { -21376, Watch_Mission02_PromptCleanup                      },
    { -20800, Watch_Mission02_MissionFlowDelay                   },
    { -20720, Watch_Mission02_MissionFlow                        },
    { -16352, Watch_Mission02_ObjectiveEventQueue                },
    { -15520, Watch_Mission02_DispatchPirateWaves                },
    { -15216, Init_Mission02_PirateAssaultTeam                   },
    { -15120, Watch_Mission02_PirateAssaultTeam                  },
    { -14848, Init_Mission02_PirateMissileCorvetteAttackRun      },
    { -14688, Watch_Mission02_PirateMissileCorvetteAttackRun     },
    { -14640, Init_Mission02_PirateMissileCorvetteRetarget       },
    { -14560, Watch_Mission02_PirateMissileCorvetteRetarget      },
    { -14400, Init_Mission02_PirateMissileCorvetteTeam           },
    { -14304, Watch_Mission02_PirateMissileCorvetteTeam          },
    { -14096, Watch_Mission02_PirateMissileCorvetteReinforcements},
    { -13888, Init_Mission02_PirateCarrierLaunchCycle            },
    { -13824, Watch_Mission02_PirateCarrierLaunchCycle           },
    { -13760, Init_Mission02_PirateStrikeTeamWave                },
    { -13632, Watch_Mission02_PirateStrikeTeamWave               },
    { -13440, Watch_Mission02_PirateStrikeTeamRedirect           },
    { -13344, Init_Mission02_PirateStrikeTeamAttackRun           },
    { -13232, Watch_Mission02_PirateStrikeTeamAttackRun          },
    { -13120, Init_Mission02_PirateStrikeTeamRetarget            },
    { -12624, Watch_Mission02_PirateStrikeTeamRetarget           },
    { -12512, Init_Mission02_PirateStrikeTeamReinforcementDelay  },
    { -12448, Watch_Mission02_PirateStrikeTeamReinforcements     },
    { -12048, Watch_Mission02_PirateStrikeTeamDockCycle          },
    { -11984, Watch_Mission02_PirateStrikeTeamPostLaunch         },
    { -11904, Init_Mission02_PirateHeavyStrikeTeam               },
    { -11776, Watch_Mission02_PirateHeavyStrikeTeam              },
    { -11552, Watch_Mission02_PirateHeavyStrikeTeamRedirect      },
    { -11456, Init_Mission02_PirateHeavyStrikeTeamAttackRun      },
    { -11200, Watch_Mission02_PirateHeavyStrikeTeamAttackRun     },
    { -11104, Init_Mission02_PirateHeavyStrikeTeamReinforcementDelay },
    { -11040, Watch_Mission02_PirateHeavyStrikeTeamReinforcements},
    { -10704, Watch_Mission02_PirateHeavyStrikeTeamDockCycle     },
    { -10640, Watch_Mission02_PirateHeavyStrikeTeamPostLaunch    },
    { -10560, Init_Mission02_ReassignSurvivingAttackTeam         },
    { -10448, Init_Mission02_PirateSalvageCorvetteTeam           },
    { -10288, Watch_Mission02_PirateSalvageCorvetteTeam          },
    { -10096, Watch_Mission02_PirateSalvageCorvetteIntro         },
    { -10000, Init_Mission02_PirateSalvageCorvetteDelay          },
    {  -9936, Watch_Mission02_PirateSalvageCorvetteDelay         },
    {  -9856, Init_Mission02_PirateSalvageCorvetteStateChoice    },
    {  -9744, Init_Mission02_PirateSalvageCorvetteSpecialAttack  },
    {  -9680, Watch_Mission02_PirateSalvageCorvetteSpecialAttack },
    {  -9568, Init_Mission02_PirateSalvageCorvetteRetarget       },
    {  -9120, Watch_Mission02_PirateSalvageCorvetteRetarget      },
    {  -9024, Init_Mission02_PirateCarrierStrikeReinforcementCycle },
    {  -8848, Watch_Mission02_PirateCarrierStrikeReinforcementCycle },
    {  -8528, Init_Mission02_PirateSalvageCorvetteCleanup        },
    {  -8464, Init_Mission02_PirateReserveAttackTeam             },
    {  -8400, Watch_Mission02_PirateReserveAttackTeam            },
    {  -8320, Init_Mission02_PirateReserveAttackRun              },
    {  -8208, Watch_Mission02_PirateReserveAttackRun             },
    {  -8080, Init_Mission02_SalvagePodApproach                  },
    {  -7904, Init_Mission02_SalvagePodHideDelay                 },
    {  -7856, Watch_Mission02_SalvagePodHideDelay                },
    {  -7712, Watch_Mission02_SalvagePodApproach                 },
    {  -7584, Init_Mission02_SalvagePodAmbushDelay               },
    {  -7536, Watch_Mission02_SalvagePodAmbushDelay              },
    {  -7344, Init_Mission02_DamagedDestroyer                    },
    {  -6896, Watch_Mission02_DamagedDestroyer                   },
    {  -6784, Watch_Mission02_DamagedDestroyerRepairStart        },
    {  -6592, Watch_Mission02_DamagedDestroyerRepairProgress     },
    {  -6288, Watch_Mission02_DamagedDestroyerAttackCue          },
    {  -6144, Init_Mission02_DamagedDestroyerDespawnDelay        },
    {  -6096, Watch_Mission02_DamagedDestroyerDespawnDelay       },
    {  -6032, Init_Mission02_PirateAdvanceSupportFrigateTeam     },
    {  -5888, Watch_Mission02_PirateAdvanceSupportFrigateTeam    },
    {  -5712, Init_Mission02_PirateAdvanceSupportFrigateRepairOrder },
    {  -5648, Watch_Mission02_PirateAdvanceSupportFrigateRepairOrder },
    {  -5408, Init_Mission02_PirateAdvanceSupportFrigateDefenseLoop },
    {  -5104, Watch_Mission02_PirateAdvanceSupportFrigateDefenseLoop },
    {  -4976, Init_Mission02_PirateTeamReinforcementTeam         },
    {  -4784, Watch_Mission02_PirateTeamReinforcementTeam        },
    {  -4704, Watch_Mission02_PirateTeamReinforcements           },
    {  -3488, Init_Mission02_PirateTeamAttackDelay               },
    {  -3440, Watch_Mission02_PirateTeamAttackDelay              },
    {  -3312, Init_Mission02_PirateTeamDefenseLoop               },
    {  -2800, Watch_Mission02_PirateTeamDefenseLoop              },
    {  -2704, Init_Mission02_PirateTeamReinforcementDelay        },
    {  -2656, Watch_Mission02_PirateTeamReinforcementDelay       },
    {  -2560, Init_Mission02_PirateTeamReinforcementGate         },
    {  -2448, Init_Mission02_AttackMothershipTeam               },
    {  -2336, Watch_Mission02_AttackMothershipTeam              },
    {  -2256, Mission02_SetAttackApproachVector                 },
    {  -2080, Mission02_SendTeamToAttackTarget                  },
    {  -2016, Watch_Mission02_TeamState_5E03D0                  },
    {  -1904, Init_Mission02_TeamState_5E0440                    },
    {  -1648, Watch_Mission02_TeamState_5E0540                   },
    {  -1552, Init_Mission02                                    },
    {      0, Watch_Mission02                                   },
    {  80768, Init_WaitForIntelEventEnd                         },
    { 208272, Init_Mission11_MissionFlow_ClearIntelAdvanceFlag  },
    { 278256, Watch_Mission13_MissionFlow_WaitForIntelEventEnd  },
    {  310992, Watch_NoOpTeamContext                             },
    {  467776, MaintainTeamContext                               },
    {       0, NULL }
};

/* ===========================================================================
   Tutorial 01 remap table
   All offsets are relative to Watch_Tutorial1 = 0x66D380 in cataclysm.exe.
   =========================================================================== */

extern void Init_Tutorial1_Tutorial10_MoveContextMenu(void);
extern void Begin_Tutorial1_Tutorial10_MoveContextMenu(void);
extern void Watch_Tutorial1_Tutorial10_MoveContextMenu(void);
extern void Init_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void);
extern void Watch_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void);
extern void Init_Tutorial1_HideUnhide(void);
extern void Watch_Tutorial1_HideUnhide(void);
extern void Watch_Tutorial1_HideUnhideRegionCleanup(void);
extern void Begin_Tutorial1_HideUnhideRegionCleanup(void);
extern void Init_WaitForIntelEventEnd(void);
extern void Watch_Tutorial1_Tutorial10_CameraMoveUp(void);
extern void Watch_Tutorial1_Tutorial10_CameraMoveDown(void);
extern void Init_Tutorial1_Tutorial12_OtherAttackTeamHide(void);
extern void Watch_NoOpTeamContext(void);
extern void MaintainTeamContext(void);
extern void Watch_Tutorial1_CurrentLessonPrompt(void);
extern void Watch_Tutorial1_PostIntelCleanup(void);
extern void Begin_Tutorial1_PlayCurrentLessonSpeech(void);
extern void Init_Tutorial1(void);
extern sdword Begin_Tutorial1_EnableInterface(void);
extern void Begin_Tutorial1_DisplayCurrentLesson(void);
extern void Init_Tutorial1_Tutorial01_ClickHerePrompt(void);
extern void Watch_Tutorial1_Tutorial01_ClickHere(void);
extern void Init_Tutorial1_Tutorial02_CameraControlsPrompt(void);
extern void Watch_Tutorial1_Tutorial02_CameraControls(void);
extern void Init_Tutorial1_Tutorial03_SelectionPrompt(void);
extern void Watch_Tutorial1_Tutorial03_SelectionControls(void);
extern void Init_Tutorial1_Tutorial04_FocusPrompt(void);
extern void Watch_Tutorial1_Tutorial04_FocusControls(void);
extern void Init_Tutorial1_Tutorial05_RightClickPrompt(void);
extern void Watch_Tutorial1_Tutorial05_RightClickCommands(void);
extern void Init_Tutorial1_Tutorial06_WaypointMovementPrompt(void);
extern void Watch_Tutorial1_Tutorial06_WaypointMovement(void);
extern void Init_Tutorial1_Tutorial07_AttackCommandsPrompt(void);
extern void Watch_Tutorial1_Tutorial07_AttackCommands(void);
extern void Init_Tutorial1_Tutorial08_BuildCommandsPrompt(void);
extern void Watch_Tutorial1_Tutorial08_BuildCommands(void);
extern void Init_Tutorial1_Tutorial09_HarvestRepairSalvagePrompt(void);
extern void Watch_Tutorial1_Tutorial09_HarvestRepairSalvage(void);
extern void Init_Tutorial1_Tutorial10_GroupTacticsPrompt(void);
extern void Watch_Tutorial1_Tutorial10_GroupTactics(void);
extern void Watch_Tutorial1_Tutorial10_TaskbarDescription(void);
extern void Init_Tutorial1_Tutorial11_ResearchAndUpgradesPrompt(void);
extern void Watch_Tutorial1_Tutorial11_ResearchAndUpgrades(void);
extern void Init_Tutorial1_Tutorial12_CommandHotkeysPrompt(void);
extern void mrKeyPress(sdword ID);
extern void Init_Tutorial1_Tutorial12_PrimaryAttackTeam(void);
extern void Begin_Tutorial1_Tutorial12_PrimaryAttackTeamReentry(void);
extern void Watch_Tutorial1_Tutorial12_PrimaryAttackTeam(void);
extern void Init_Tutorial1_Tutorial12_PrimaryAttackReinforcements(void);
extern void Watch_Tutorial1_Tutorial12_PrimaryAttackApproach(void);
extern void Init_Tutorial1_Tutorial12_HiddenAttackTeam(void);
extern void Watch_Tutorial1_Tutorial12_AttackTeamSignalRouter(void);
extern void Begin_Tutorial1_Tutorial12_RevealHiddenAttackTeam(void);
extern void Begin_Tutorial1_Tutorial12_RequestOtherTeamHide(void);
extern void Init_Tutorial1_Tutorial12_RequestOtherTeamHideFlag(void);
extern void Watch_Tutorial1_Tutorial12_OtherAttackTeamHide(void);
extern void Begin_Tutorial1_Tutorial12_MaintainAttackTeamContext(void);

static const KASFuncOffsetEntry kasOffsetTable_Tutorial01[] =
{
    { -1172368, Watch_Tutorial1_Tutorial10_TaskbarDescription },
    { -523680, Init_Tutorial1_Tutorial10_MoveContextMenu },
    { -523616, Begin_Tutorial1_Tutorial10_MoveContextMenu },
    { -523552, Watch_Tutorial1_Tutorial10_MoveContextMenu },
    { -523456, Init_Tutorial1_Tutorial10_MoveContextMenuAreaClear },
    { -523360, Watch_Tutorial1_Tutorial10_MoveContextMenuAreaClear },
    { -520528, Init_Tutorial1_HideUnhide },
    { -520384, Watch_Tutorial1_HideUnhide },
    { -520288, Watch_Tutorial1_HideUnhideRegionCleanup },
    { -520144, Begin_Tutorial1_HideUnhideRegionCleanup },
    { -494672, Init_WaitForIntelEventEnd },
    { -459376, Watch_Tutorial1_Tutorial10_CameraMoveUp },
    { -458880, Watch_Tutorial1_Tutorial10_CameraMoveDown },
    { -382448, Init_Tutorial1_Tutorial12_OtherAttackTeamHide },
    { -264448, Watch_NoOpTeamContext },
    { -107664, MaintainTeamContext },
    {  -15696, Watch_Tutorial1_CurrentLessonPrompt },
    {  -15200, Watch_Tutorial1_PostIntelCleanup },
    {  -14560, Begin_Tutorial1_PlayCurrentLessonSpeech },
    {     -80, Init_Tutorial1 },
    {       0, Watch_Tutorial1 },
    {     176, Begin_Tutorial1_EnableInterface },
    {     224, Begin_Tutorial1_DisplayCurrentLesson },
    {     816, Init_Tutorial1_Tutorial01_ClickHerePrompt },
    {     960, Watch_Tutorial1_Tutorial01_ClickHere },
    {    1152, Init_Tutorial1_Tutorial02_CameraControlsPrompt },
    {    1376, Watch_Tutorial1_Tutorial02_CameraControls },
    {    2048, Init_Tutorial1_Tutorial03_SelectionPrompt },
    {    2336, Watch_Tutorial1_Tutorial03_SelectionControls },
    {    3504, Init_Tutorial1_Tutorial04_FocusPrompt },
    {    3648, Watch_Tutorial1_Tutorial04_FocusControls },
    {    4016, Init_Tutorial1_Tutorial05_RightClickPrompt },
    {    4176, Watch_Tutorial1_Tutorial05_RightClickCommands },
    {    5264, Init_Tutorial1_Tutorial06_WaypointMovementPrompt },
    {    5456, Watch_Tutorial1_Tutorial06_WaypointMovement },
    {    6208, Init_Tutorial1_Tutorial07_AttackCommandsPrompt },
    {    6352, Watch_Tutorial1_Tutorial07_AttackCommands },
    {    6960, Init_Tutorial1_Tutorial08_BuildCommandsPrompt },
    {    7168, Watch_Tutorial1_Tutorial08_BuildCommands },
    {    7632, Init_Tutorial1_Tutorial09_HarvestRepairSalvagePrompt },
    {    7840, Watch_Tutorial1_Tutorial09_HarvestRepairSalvage },
    {    8384, Init_Tutorial1_Tutorial10_GroupTacticsPrompt },
    {    8672, Watch_Tutorial1_Tutorial10_GroupTactics },
    {    9376, Init_Tutorial1_Tutorial11_ResearchAndUpgradesPrompt },
    {    9536, Watch_Tutorial1_Tutorial11_ResearchAndUpgrades },
    {   10672, Init_Tutorial1_Tutorial12_CommandHotkeysPrompt },
    {   10960, mrKeyPress },
    {   11712, Init_Tutorial1_Tutorial12_PrimaryAttackTeam },
    {   11776, Begin_Tutorial1_Tutorial12_PrimaryAttackTeamReentry },
    {   11840, Watch_Tutorial1_Tutorial12_PrimaryAttackTeam },
    {   11904, Init_Tutorial1_Tutorial12_PrimaryAttackReinforcements },
    {   12016, Watch_Tutorial1_Tutorial12_PrimaryAttackApproach },
    {   12096, Init_Tutorial1_Tutorial12_HiddenAttackTeam },
    {   12256, Watch_Tutorial1_Tutorial12_AttackTeamSignalRouter },
    {   12336, Begin_Tutorial1_Tutorial12_RevealHiddenAttackTeam },
    {   12400, Begin_Tutorial1_Tutorial12_RequestOtherTeamHide },
    {   12448, Init_Tutorial1_Tutorial12_RequestOtherTeamHideFlag },
    {   12496, Watch_Tutorial1_Tutorial12_OtherAttackTeamHide },
    {   12592, Begin_Tutorial1_Tutorial12_MaintainAttackTeamContext },
    {       0, NULL }
};

static const KASFuncOffsetEntry *kasGetOffsetTable(void)
{
    switch (singlePlayerGameInfo.currentMission)
    {
        case 1:  return kasOffsetTable_Mission01;
        case 2:  return kasOffsetTable_Mission02;
        case 17: return kasOffsetTable_Tutorial01;
        default: return NULL;
    }
}

static KASWatchFunction kasGetMissionAnchor(void)
{
    return IndexToWatchFunction(singlePlayerGameInfo.currentMission - 1);
}

sdword kasConvertFuncPtrToOffset(void *func)
{
    const KASFuncOffsetEntry *table;

    if (func == NULL)
    {
        return -1;
    }

    table = kasGetOffsetTable();
    if (table != NULL)
    {
        while (table->ourFuncPtr != NULL)
        {
            if (table->ourFuncPtr == func)
            {
                return table->originalOffset;
            }
            table++;
        }

        dbgFatalf(DBG_Loc,
                  "\nkasConvertFuncPtrToOffset: function 0x%x not in table for mission %d",
                  func,
                  singlePlayerGameInfo.currentMission);
        return -1;
    }

    return ((ubyte *)func) - ((ubyte *)kasGetMissionAnchor());
}

void *kasConvertOffsetToFuncPtr(sdword offset)
{
    const KASFuncOffsetEntry *table;

    if (offset == -1)
    {
        return NULL;
    }

    table = kasGetOffsetTable();
    if (table != NULL)
    {
        while (table->ourFuncPtr != NULL)
        {
            if (table->originalOffset == offset)
            {
                return table->ourFuncPtr;
            }
            table++;
        }

        dbgFatalf(DBG_Loc,
                  "\nkasConvertOffsetToFuncPtr: offset %d not in table for mission %d",
                  offset,
                  singlePlayerGameInfo.currentMission);
        return NULL;
    }

    return (void *)(((ubyte *)kasGetMissionAnchor()) + offset);
}
