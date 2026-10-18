/*=============================================================================
    Name    : mission02.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission02
=============================================================================*/

#include "MissionScriptSupport.h"

#undef CATACLYSM_DIAG
#define CATACLYSM_DIAG 1

#define dword_A43EE8 kasCurrentTeamPtr
#define dword_C3749C kasCurrentStep
#define dword_C374A0 kasCurrentScopeHash

static const char *m02_buildHangarMod_brief[] = { "Rebuild Hangar Module", "French version goes here", "German version goes here" };
static const char *m02_buildHangarMod_full[] = { "We need a hangar module to build fighters, rebuild it immediately.", "French version goes here", "German version goes here" };
static const char *m02_findDestroyer_brief[] = { "Find the BUSHAN-RE", "French version goes here", "German version goes here" };
static const char *m02_findDestroyer_full[] = { "Locate the BUSHAN-RE so repairs can proceed immediately.  Use waypoints to send recons on routes circling out from the position of your Command Ship.", "French version goes here", "German version goes here" };
static const char *m02_buildEngineering_brief[] = { "Build Engineering Module", "French version goes here", "German version goes here" };
static const char *m02_buildEngineering_full[] = { "In order to research repair technology, you must build an engineering module.  This module can be found listed in the build manager at the bottom.", "French version goes here", "German version goes here" };
static const char *m02_repairDestroyer_brief[] = { "Repair the BUSHAN-RE", "French version goes here", "German version goes here" };
static const char *m02_repairDestroyer_full[] = { "Send a WORKER to repair the BUSHAN-RE.  The more WORKERS you send, the faster you can repair this ship. The WORKERS repairing must be upgraded after researching repair technology in the engineering module.", "French version goes here", "German version goes here" };
static const char *m02_salvagePod_brief[] = { "Salvage Derelict", "French version goes here", "German version goes here" };
static const char *m02_salvagePod_full[] = { "Send a WORKER to salvage the unknown derelict.  You should send a small contingent of fighters to support the WORKER.  The WORKER salvaging must be upgraded after researching salvaging technology in the engineering module.  To salvage, place the mouse pointer over the derelict and press the right mouse button.", "French version goes here", "German version goes here" };
static const char *m02_destroyPirates_brief[] = { "Destroy Turanic Raiders", "French version goes here", "German version goes here" };
static const char *m02_destroyPirates_full[] = { "Turanic Raiders are attempting to salvage the derelict for themselves.  Eliminate all hostile forces you encounter.  Make sure to always have a defense force near your WORKERS while they harvest.", "French version goes here", "German version goes here" };
static const char *m02_hyperspace_brief[] = { "Hyperspace", "French version goes here", "German version goes here" };
static const char *m02_hyperspace_full[] = { "Rally the fleet and prepare to hyperspace on your mark.", "French version goes here", "German version goes here" };

void Init_Mission02(void)
{
    sdword difficulty;

    kasCurrentStep = 0;
    kasCurrentScopeHash = KH_mission02;

    kasfMusicPlay(1);

    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));

    kasFSMCreate(KH_FleetIntel, Init_Mission02_SetupMissionFlowState, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission02_ObjectiveEventQueue,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(KH_M02_Pirates, Watch_NoOpTeamContext, Watch_Mission02_DispatchPirateWaves,
                 kasAITeamPtrIfExists(KH_M02_Pirates));

    kasFSMCreate(KH_M02_FSM_UNKNOWN_ASSAULT, Init_Mission02_PirateAssaultTeam, Watch_Mission02_PirateAssaultTeam,
                 kasAITeamPtrIfExists(KH_M02_pCarrier));
    kasFSMCreate(KH_M02_Kamikaze, Init_Mission02_PirateReserveAttackTeam, Watch_Mission02_PirateReserveAttackTeam,
                 kasAITeamPtrIfExists(KH_M02_Kamikaze));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_HEAVY, Init_Mission02_PirateHeavyStrikeTeam, Watch_Mission02_PirateHeavyStrikeTeam,
                 kasAITeamPtrIfExists(KH_M02_pFighter3));
    kasFSMCreate(KH_M02_FSM_Salvage, Init_Mission02_PirateSalvageCorvetteTeam, Watch_Mission02_PirateSalvageCorvetteTeam,
                 kasAITeamPtrIfExists(KH_M02_pSalvage));
    kasFSMCreate(KH_M02_FSM_Strike, Init_Mission02_PirateStrikeTeamWave, Watch_Mission02_PirateStrikeTeamWave,
                 kasAITeamPtrIfExists(KH_M02_pFighter1));
    kasFSMCreate(KH_M02_FSM_Strike, Init_Mission02_PirateStrikeTeamWave, Watch_Mission02_PirateStrikeTeamWave,
                 kasAITeamPtrIfExists(KH_M02_pFighter2));

    // Set byte857=5 on all 6 honour guard scout teams
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(KH_M02_pPatrol0), 5);
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(KH_M02_pPatrol1), 5);
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(KH_M02_pPatrol2), 5);
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(KH_M02_pPatrol3), 5);
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(KH_M02_pPatrol4), 5);
    kasAITeamShipsPtr(kasAITeamShipsPtrIfExists(KH_M02_pPatrol5), 5);

    // FSM for each scout team
    kasFSMCreate(KH_M02_FSM_UNKNOWN_PATROL, Init_Mission02_HonourGuardSwarmPatrol, Watch_Mission02_HonourGuardSwarmPatrol,
                 kasAITeamPtrIfExists(KH_M02_pPatrol0));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_PATROL, Init_Mission02_HonourGuardSwarmPatrol, Watch_Mission02_HonourGuardSwarmPatrol,
                 kasAITeamPtrIfExists(KH_M02_pPatrol1));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_PATROL, Init_Mission02_HonourGuardSwarmPatrol, Watch_Mission02_HonourGuardSwarmPatrol,
                 kasAITeamPtrIfExists(KH_M02_pPatrol2));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_PATROL, Init_Mission02_HonourGuardSwarmPatrol, Watch_Mission02_HonourGuardSwarmPatrol,
                 kasAITeamPtrIfExists(KH_M02_pPatrol3));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_PATROL, Init_Mission02_HonourGuardSwarmPatrol, Watch_Mission02_HonourGuardSwarmPatrol,
                 kasAITeamPtrIfExists(KH_M02_pPatrol4));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_PATROL, Init_Mission02_HonourGuardSwarmPatrol, Watch_Mission02_HonourGuardSwarmPatrol,
                 kasAITeamPtrIfExists(KH_M02_pPatrol5));

    // Difficulty-scaled timer and scout reduction
    difficulty = GetGameDifficulty();
    switch (difficulty)
    {
        case 0:
            kasTimerCreateSetStartById(KH_M02_DIFFICULTY_TIMER, 120);
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol1));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol2));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol3));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol4));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol5));
            break;
        case 1:
            kasTimerCreateSetStartById(KH_M02_DIFFICULTY_TIMER, 180);
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol3));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol4));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol5));
            break;
        case 2:
            kasTimerCreateSetStartById(KH_M02_DIFFICULTY_TIMER, 360);
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol4));
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol5));
            break;
        default:
            kasfVarDestroy(kasAITeamShipsPtrIfExists(KH_M02_pPatrol5));
            break;
    }

    // Post-difficulty FSMs
    kasFSMCreate(KH_M02_FSM_Destroyer, Init_Mission02_DamagedDestroyer, Watch_Mission02_DamagedDestroyer,
                 kasAITeamPtrIfExists(KH_M02_kDestroyer));
    kasFSMCreate(KH_M02_FSM_MessagePod, Init_Mission02_SalvagePodApproach, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_M02_dMessagePod));
    kasFSMCreate(0x51E1D3, Init_Mission02_PirateAdvanceSupportFrigateTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_M02_pTricky));
    kasFSMCreate(KH_M02_FSM_UNKNOWN_MISSILE, Init_Mission02_PirateMissileCorvetteTeam, Watch_Mission02_PirateMissileCorvetteTeam,
                 kasAITeamPtrIfExists(KH_M02_pGuard));
    kasFSMCreate(KH_M02_FSM_Ambush, Init_Mission02_PirateTeamReinforcementTeam, Watch_Mission02_PirateTeamReinforcementTeam,
                 kasAITeamPtrIfExists(KH_M02_Ambush1));
    kasFSMCreate(KH_M02_FSM_Ambush, Init_Mission02_PirateTeamReinforcementTeam, Watch_Mission02_PirateTeamReinforcementTeam,
                 kasAITeamPtrIfExists(KH_M02_Ambush2));
    kasFSMCreate(KH_M02_FSM_Ambush, Init_Mission02_PirateTeamReinforcementTeam, Watch_Mission02_PirateTeamReinforcementTeam,
                 kasAITeamPtrIfExists(KH_M02_Ambush3));
    kasFSMCreate(KH_M02_Decoy, Init_Mission02_AttackMothershipTeam, Watch_Mission02_AttackMothershipTeam,
                 kasAITeamPtrIfExists(KH_M02_Decoy));

    kasfRUsSet(1, 30000);

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_SALVAGE");
    kasfAllowPlayerToResearch("TECH_REPAIR");

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetCount(0, 3);

    kasfModuleSlotOrderSetShipType(1, "p1Fighter");
    kasfModuleSlotOrderSetShipType(1, "p1MissileCorvette");
    kasfModuleSlotOrderSetCount(1, 1);

    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipSpecial",    "RACE_SECT", 0);
    kasfEnableShip("sMothershipArmour",     "RACE_SECT", 0);
    kasfEnableShip("sMothershipWeapons",    "RACE_SECT", 0);
    kasfEnableShip("sMothershipMicro",      "RACE_SECT", 0);

    kasfDisableAllAIFeatures();
    kasfDisablePlayerHyperspace();

    kasTimerCreateSetStartById(KH_M02_SCAN_TIMER, 1);
    kasTimerCreateSetStartById(KH_M02_CLOAK_DETECT_TIMER, 1);

    kasfFindBeastShipsInside(kasVolumePtr(KH_M02_BEAST_AREA), kasGrowSelectionPtr(KH_M02_BEAST_AREA));

    kasfLocationCard(8000, "Outskirts of Hiigaran System");
}

void Watch_Mission02(void)
{
    sdword randomDelay;

    kasCurrentStep = 0;
    kasCurrentScopeHash = KH_mission02;

    if (kasTimerExpiredDestroyById(KH_M02_SCAN_TIMER))
    {
        kasfFindShipsInside(kasVolumePtr(KH_Everything), kasGrowSelectionPtr(KH_AllShips));
        kasSelectEnemyShips((int *)kasGrowSelectionPtr(KH_Friendly), kasGrowSelectionPtr(KH_AllShips));
        kasSelectPlayerShips((int *)kasGrowSelectionPtr(KH_Enemy), kasGrowSelectionPtr(KH_AllShips));
        kasfShipsRemove(kasGrowSelectionPtr(KH_Enemy), kasAITeamShipsPtrIfExists(KH_M02_dMessagePod));
        randomDelay = kasfRandom(5, 10);
        kasTimerSetById(KH_M02_SCAN_TIMER, randomDelay);
        kasTimerStartById(KH_M02_SCAN_TIMER);
    }
    if (kasTimerExpiredDestroyById(KH_M02_CLOAK_DETECT_TIMER) && !kasfEntryValueGet(KH_M02_CLOAKED_FOUND))
    {
        kasfFindCloakedShips(kasGrowSelectionPtr(KH_M02_CLOAKED_FOUND));
        kasTimerSetById(KH_M02_CLOAK_DETECT_TIMER, 3);
        kasTimerStartById(KH_M02_CLOAK_DETECT_TIMER);
    }
    if (!kasfEntryValueGet(KH_M02_FLAG_CLOAKED_SIGHTED))
    {
        if (kasfShipsAnyInSensorRange(kasGrowSelectionPtr(KH_M02_CLOAKED_FOUND)))
        {
            kasfEntryGetOrCreateSetValue(KH_M02_FLAG_CLOAKED_SIGHTED, 1);
            kasfEntryGetOrCreateSetValue(KH_M02_CLOAKED_FOUND, 1);
            kasfGrowSelectionClear((int *)kasGrowSelectionPtr(KH_M02_CLOAKED_FOUND));
            kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)KH_M02_CLOAKED_FOUND);
        }
    }
    if (!kasfEntryValueGet(KH_M02_CLOAKED_FOUND))
    {
        kasfGrowSelectionClear((int *)kasGrowSelectionPtr(KH_M02_CLOAKED_FOUND));
    }
    if (kasfEntryValueGet(KH_M01_INIT_COMPLETE))
    {
        if (kasfShipsCount(kasGrowSelectionPtr(KH_sMothership)))
        {
            if (kasfShipsSelectType(kasGrowSelectionPtr(KH_sMothershipDockingBay), kasGrowSelectionPtr(KH_Enemy), "sMothershipDockingBay"))
            {
                if (!kasfEntryValueGet(KH_M02_MS_DOCKED))
                {
                    if (!kasCheckShipDockFlag(kasGrowSelectionPtr(KH_sMothershipDockingBay)))
                    {
                        kasfEntryGetOrCreateSetValue(KH_M02_MS_DOCKED, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(KH_Objectives), (char *)KH_M02_MSG_DOCK_COMPLETE);
                    }
                }
            }
            else if (kasfEntryValueGet(KH_M02_MS_DOCKED))
            {
                kasfEntryGetOrCreateSetValue(KH_M02_MS_DOCKED, 0);
                kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)KH_M02_MSG_UNDOCK);
            }
        }
    }
    if (!kasfEntryValueGet(KH_M02_FLAG_INIT_COMPLETE) && kasfEntryValueGet(KH_M01_INIT_COMPLETE))
    {
        kasfEntryGetOrCreateSetValue(KH_M02_FLAG_INIT_COMPLETE, 1);
        kasSelectPlayerShipsSimple((int *)kasGrowSelectionPtr(KH_M02_SEL_PLAYER_NSAL), kasGrowSelectionPtr(KH_AllShips));
        kasfShipsSelectNotSalvagers((int *)kasGrowSelectionPtr(KH_M02_SEL_PLAYER_NSAL), kasGrowSelectionPtr(KH_M02_SEL_PLAYER_NSAL));
        if (kasfShipsSelectLinkedShips(kasGrowSelectionPtr(KH_M02_SEL_LINKED_SHIPS), kasGrowSelectionPtr(KH_sMothership)))
        {
            kasfShipsAdd((int *)kasGrowSelectionPtr(KH_M02_SEL_PLAYER_NSAL), (int *)kasGrowSelectionPtr(KH_M02_SEL_LINKED_SHIPS));
        }
        if (GetGameDifficulty() < 2 && kasfRUsGet(0) < 500)
        {
            if (!kasfShipsSelectType(kasGrowSelectionPtr(KH_M02_SEL_WORKER_CHECK), kasGrowSelectionPtr(KH_M02_SEL_PLAYER_NSAL), "sWorker"))
            {
                kasfShipsCreateInside(kasGrowSelectionPtr(KH_M02_SEL_SPAWN_WORKER), "sWorker", 1, kasGrowSelectionPtr(KH_sMothership), 0);
            }
        }
        kasfEntryGetOrCreateSetValue(KH_M02_MS_DOCKED, 1);
        spSetSinglePlayerSaveInfo(2, "Mission02");
        kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)KH_M02_MSG_FLOW_START);
    }
    if (!kasfEntryValueGet(KH_M02_FLAG_SHIPS_SCANNED))
    {
        if (kasfEntryValueGet(KH_M02_MICRO_DOCKING))
        {
            if (kasfShipsSelectType(kasGrowSelectionPtr(KH_sMothershipMicro), kasGrowSelectionPtr(KH_Enemy), "sMothershipMicro"))
            {
                if (!kasCheckShipDockFlag(kasGrowSelectionPtr(KH_sMothershipMicro)))
                {
                    kasfEntryGetOrCreateSetValue(KH_M02_FLAG_SHIPS_SCANNED, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)KH_M02_MSG_MICRO_DOCKED);
                    kasfMsgSend(kasAITeamPtrIfExists(KH_Objectives), (char *)KH_M02_MSG_MICRO_DOCKED);
                }
            }
        }
    }
    if (!kasfEntryValueGet(KH_M02_PATROL_SIGHTED))
    {
        if (kasfShipsAnyInSensorRange(kasGrowSelectionPtr(KH_M02_FSM_UNKNOWN_PATROL)))
        {
            kasfEntryGetOrCreateSetValue(KH_M02_PATROL_SIGHTED, 1);
            kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)KH_M02_MSG_PATROL_SPOTTED);
        }
    }
    if (!kasfEntryValueGet(KH_M01_MS_FOUND) && kasfBuildManagerShipTypeInBuildQueue("sMothershipMicro"))
    {
        kasfEntryGetOrCreateSetValue(KH_M01_MS_FOUND, 1);
        kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)KH_M02_MSG_MICRO_QUEUED);
    }
    if (!kasfEntryValueGet(KH_M02_FLAG_MICRO_QUEUED))
    {
        if (!kasfEntryValueGet(KH_M02_MISSION_DONE))
        {
            if (kasTimerExpiredById(KH_M02_DIFFICULTY_TIMER))
            {
                kasfEntryGetOrCreateSetValue(KH_M02_FLAG_MICRO_QUEUED, 1);
                kasAITeamPtrIfExists(KH_FleetIntel);  // result unused; matches binary
            }
        }
    }
}

void Init_Mission02_SetupMissionFlowState(void)
{
  kasCurrentStep = 1;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  kasfEntryGetOrCreateSetValue(266064926, 0);
  kasfEntryGetOrCreateSetValue(109086661, 0);
  kasfEntryGetOrCreateSetValue(211831571, 0);
  kasfEntryGetOrCreateSetValue(52577235, 0);
  kasfEntryGetOrCreateSetValue(172907219, 0);
  kasfEntryGetOrCreateSetValue(232774915, 0);
  kasfEntryGetOrCreateSetValue(127756738, 0);
  kasfEntryGetOrCreateSetValue(1182958, 0);
  kasfEntryGetOrCreateSetValue(332667, 0);
  kasfTutEnableFlags("Focus");
  kasfTutEnableFlags("CancelFocus");
  kasTimerCreateById(144337492);
  kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission02_MissionFlow); return;
}

void Watch_Mission02_PromptCleanup(void)
{
  int result; // eax
  int v1; // eax
  int v2; // eax
  int v3; // eax
  int v4; // eax
  int v5; // eax
  int v6; // eax

  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  if ( kasfIntelEventEnded() || (result = kasfEntryValueGet(-2054310428)) != 0 )
  {
#if !defined(HW_Release) && CATACLYSM_DIAG
    {
      static sdword promptCleanupDiagCount = 0;
      if ( promptCleanupDiagCount < 8 )
      {
        dbgMessagef(
          "\n" CATACLYSM_DIAG_LABEL "m02 prompt cleanup: now=%.2f widescreen=%d focus=%d focus2=%d timerRestart=%d busy=%d",
          universe.totaltimeelapsed,
          kasfEntryValueGet(266064926),
          kasfEntryValueGet(172907219),
          kasfEntryValueGet(232774915),
          kasfEntryValueGet(127756738),
          kasfEntryValueGet(332667));
      }
      promptCleanupDiagCount++;
    }
#endif
    kasfEntrySetValue(-2054310428, 0);
    kasfSubtitleDraw();
    if ( kasfEntryValueGet(266064926) )
    {
      kasfWideScreenOut(90);
      kasfEntrySetValue(266064926, 0);
    }
    if ( kasfEntryValueGet(109086661) )
    {
      kasfUniverseUnpause();
      kasfEntrySetValue(109086661, 0);
    }
    if ( kasfEntryValueGet(211831571) )
    {
      kasfOtherKASUnpause();
      kasfEntrySetValue(211831571, 0);
    }
    if ( kasfEntryValueGet(52577235) )
    {
      kasfCloseSensors(1);
      kasfEntrySetValue(52577235, 0);
    }
    if ( kasfEntryValueGet(172907219) )
    {
      kasfTutCameraFocusCancel();
      v1 = kasfEntryValueGet(260833221);
      kasfCameraSetAngleDeg(v1);
      v2 = kasfEntryValueGet(60936190);
      kasfCameraSetDeclinationDeg(v2);
      v3 = kasfEntryValueGet(64272901);
      kasfCameraSetDistance(v3);
      kasfEntrySetValue(172907219, 0);
    }
    if ( kasfEntryValueGet(232774915) )
    {
      kasfTutCameraFocusCancel();
      kasfTutCameraFocusCancel();
      v4 = kasfEntryValueGet(260833221);
      kasfCameraSetAngleDeg(v4);
      v5 = kasfEntryValueGet(60936190);
      kasfCameraSetDeclinationDeg(v5);
      v6 = kasfEntryValueGet(64272901);
      kasfCameraSetDistance(v6);
      kasfEntrySetValue(232774915, 0);
    }
    if ( kasfEntryValueGet(127756738) )
    {
      kasfEntrySetValue(127756738, 0);
      kasTimerStartById(144337492);
    }
    if ( kasfEntryValueGet(1182958) )
    {
      kasfEntrySetValue(1182958, 0);
      kasfHoldHyperspaceWindow(0);
    }
    if ( !kasfEntryValueGet(71552162) )
    {
      if ( kasfEntryValueGet(-1889385931) )
      {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfEnableShip("sMothershipMicro", "RACE_SECT", 1);
      }
    }
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_MissionFlowDelay); return;
  }
  return;
}

void Watch_Mission02_MissionFlowDelay(void)
{
  int result; // eax

  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  result = kasTimerExpiredDestroyById(144337492);
  if ( result )
  {
#if !defined(HW_Release) && CATACLYSM_DIAG
    {
      static sdword missionFlowDelayDiagCount = 0;
      if ( missionFlowDelayDiagCount < 8 )
      {
        dbgMessagef(
          "\n" CATACLYSM_DIAG_LABEL "m02 mission flow delay expired: now=%.2f",
          universe.totaltimeelapsed);
      }
      missionFlowDelayDiagCount++;
    }
#endif
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission02_MissionFlow); return;
  }
  return;
}

void Watch_Mission02_MissionFlow(void)
{
  int v0; // eax
  int result; // eax
  int *v12; // eax
  int *v13; // eax
  int v29; // eax
  int v30; // eax
  int v31; // eax
  int v32; // eax
  int v33; // eax
  int *v40; // eax
  signed int *v41; // eax
  int AngleDeg; // eax
  int DeclinationDeg; // eax
  int Distance; // eax
  int v52; // eax
  int *v53; // eax
  int v54; // eax
  udword *v7;
  udword *v8;
  udword *v9;
  udword *v10;
  udword *v14;
  udword *v15;
  udword *v16;
  udword *v17;
  udword *v18;
  udword *v19;
  udword *v20;
  udword *v26;
  udword *v27;
  udword *v28;
  udword *v39;
  udword *v42;
  udword *v43;
  udword *v44;
  udword *v45;
  udword *v46;
  udword *v47;
  udword *v55;
  udword *v58;
  udword *v59;
  udword *v60;
  udword *v61;
  udword *v62;
  int v63;
  int *v64;
  int *v65;
  int v66;
  int v67;
  int v68;
  int v69;
  int v70;
  unsigned int v5;
  unsigned int v6;
  unsigned int v24;
  unsigned int v25;

  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  v0 = kasGrowSelectionPtr(218060896);
  if ( !kasfShipsCount(v0) )
  {
    kasfMissionFailed();
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); return;
  }
  if ( !kasfEntryValueGet(332667) )
  {
    v5 = aitQueueCount((struct AITeam *)kasCurrentTeamPtr);
    if ( v5 >= 0 )
    {
      while ( 1 )
      {
        v6 = (udword)aitQueuePopPointerByIndex((struct AITeam *)kasCurrentTeamPtr, (udword)v5);
        if ( v6 > 0x67AE1D2 )
        {
          if ( v6 > 0x9502D35 )
          {
            switch ( v6 )
            {
              case 0xA6345B7u:
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfEntrySetValue(332667, 1);
                kasfWideScreenIn(90);
                v67 = 119169842;
                goto LABEL_63;
              case 0xA7C9B35u:
                kasfMusicPlay(1);
                kasfSpeechEventQueueCurrentPlayer(268443879, 0);
                kasfEntrySetValue(332667, 1);
                kasTimerDestroyById(144337492);
                v19 = kasAITeamPtrIfExists(160291907);
                kasfMsgSend((struct AITeam *)v19, (char *)175938357);
                v20 = kasAITeamPtrIfExists(176850956);
                kasfMsgSend((struct AITeam *)v20, (char *)130659266);
                kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); return;
              case 0xD8BD352u:
                kasfSpeechEventQueueCurrentPlayer(268444013, 0);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
            }
          }
          else
          {
            if ( v6 == 156249397 )
            {
              kasfSpeechEventQueueCurrentPlayer(134222673, 0);
              v68 = 262155621;
              v16 = kasAITeamPtrIfExists(160291907);
              goto LABEL_131;
            }
            if ( v6 > 0x927EC34 )
            {
              if ( v6 == 154206436 )
              {
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenOut(90);
                kasfEntrySetValue(332667, 1);
                v67 = 41272642;
                goto LABEL_63;
              }
            }
            else
            {
              switch ( v6 )
              {
                case 0x927EC34u:
                  kasfSpeechEventQueueCurrentPlayer(268439923, 0);
                  v17 = kasAITeamPtrIfExists(124217570);
                  kasfMsgSend((struct AITeam *)v17, (char *)130908126);
                  v18 = kasAITeamPtrIfExists(160291907);
                  kasfMsgSend((struct AITeam *)v18, (char *)153611316);
                  kasfPingRemove("PingMessagePod");
                  kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
                case 0x8970534u:
                  kasfSpeechEventQueueCurrentPlayer(268444021, 0);
                  v68 = 144114996;
                  v16 = kasAITeamPtrIfExists(160291907);
                  goto LABEL_131;
                case 0x8A89709u:
                  kasTimerSetById(144337492, 3);
                  kasTimerStartById(144337492);
                  kasfEntrySetValue(332667, 1);
                  spEventWideScreenIn(90);
                  kasfEntryGetOrCreateSetValue(-1920229822, 1);
                  v14 = kasAITeamPtrIfExists(176850956);
                  kasfMsgSend((struct AITeam *)v14, (char *)176779298);
                  v15 = kasAITeamPtrIfExists(230547091);
                  kasfMsgSend((struct AITeam *)v15, (char *)80111235);
                  v67 = 74024315;
                  v10 = kasAITeamPtrIfExists(123838693);
                  goto LABEL_64;
              }
            }
          }
          goto LABEL_37;
        }
        if ( v6 == 108716498 )
        {
          v69 = kasGrowSelectionPtr(18336307);
          v29 = kasVolumePtr(132933767);
          kasfFindShipsInside(v29, v69);
          v70 = kasGrowSelectionPtr(18336307);
          v12 = (int *)kasGrowSelectionPtr(0x4A2A29);
          Mission02ShipsSelect((GrowSelection *)v12, (GrowSelection *)v70);
          v63 = kasGrowSelectionPtr(0x4A2A29);
          v13 = (int *)kasGrowSelectionPtr(88512350);
#if !defined(HW_Release) && CATACLYSM_DIAG
          {
            static sdword oreCanScanDiagCount = 0;
            if ( oreCanScanDiagCount < 6 )
            {
              dbgMessagef(
                "\n" CATACLYSM_DIAG_LABEL "m02 micro queued scan: now=%.2f inside=%u inactive=%u",
                universe.totaltimeelapsed,
                ((GrowSelection *)v70)->selection->numShips,
                ((GrowSelection *)v63)->selection->numShips);
            }
            oreCanScanDiagCount++;
          }
#endif
          if ( kasfShipsSelectType(v13, v63, "sMothershipOreCanArmour") )
          {
#if !defined(HW_Release) && CATACLYSM_DIAG
            static sdword oreCanFoundDiagCount = 0;
            if ( oreCanFoundDiagCount < 6 )
            {
              Ship *oreCanShip;

              oreCanShip = ((GrowSelection *)v13)->selection->ShipPtr[0];
              dbgMessagef(
                "\n" CATACLYSM_DIAG_LABEL "m02 orecan found: now=%.2f ship=%x clamp=%x dockflag=%d slot1=%d active=%d",
                universe.totaltimeelapsed,
                oreCanShip,
                oreCanShip->clampInfo,
                (oreCanShip->specialFlags2 & SPECIAL_2_DockFlag) != 0,
                shipGetSlotStatus(oreCanShip, 1),
                shipIsModuleSlotActive(oreCanShip, universe.curPlayerPtr));
              oreCanFoundDiagCount++;
            }
#endif
            kasfEntrySetValue(144337492, 3);
            kasTimerStartById(144337492);
            kasfEntrySetValue(332667, 1);
            kasfWideScreenIn(90);
            v67 = 157550597;
            goto LABEL_63;
          }
#if !defined(HW_Release) && CATACLYSM_DIAG
          {
            static sdword oreCanMissingDiagCount = 0;
            if ( oreCanMissingDiagCount < 6 )
            {
              dbgMessagef("\n" CATACLYSM_DIAG_LABEL "m02 orecan missing after micro queue: now=%.2f", universe.totaltimeelapsed);
            }
            oreCanMissingDiagCount++;
          }
#endif
          kasfSpeechEventQueueCurrentPlayer(268439915, 0);
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
        }
        if ( v6 > 0x2E80A02 )
        {
          if ( v6 > 0x4A38954 )
          {
            if ( v6 == 85166420 )
            {
              kasfEntrySetValue(332667, 1);
              kasfSpeechEventQueueCurrentPlayer(268439922, 0);
              kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_WaitForIntelEventEnd); return;
            }
          }
          else
          {
            switch ( v6 )
            {
              case 0x4A38954u:
                kasfEntrySetValue(332667, 1);
                kasfSpeechEventQueueCurrentPlayer(268439924, 0);
                kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_WaitForIntelEventEnd); return;
              case 0x471A631u:
                kasfEntrySetValue(332667, 1);
                kasfSpeechEventQueueCurrentPlayer(268439777, 0);
                v68 = 119169810;
                goto LABEL_130;
              case 0x471A633u:
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfEntrySetValue(332667, 1);
                spEventWideScreenIn(90);
                v9 = kasAITeamPtrIfExists(176850956);
                kasfMsgSend((struct AITeam *)v9, (char *)119169841);
                v67 = 5801588;
                v10 = kasAITeamPtrIfExists(93771865);
                goto LABEL_64;
            }
          }
          goto LABEL_37;
        }
        if ( v6 == 48761346 )
        {
          kasTimerSetById(144337492, 3);
          kasTimerStartById(144337492);
          kasfEntrySetValue(332667, 1);
          spEventWideScreenIn(90);
          v67 = 243310657;
          goto LABEL_63;
        }
        if ( v6 > 0x479E14 )
          break;
        if ( v6 == 4693524 )
        {
          if ( !kasfDoesPlayerHave("TECH_SALVAGE") || !kasfDoesPlayerHave("TECH_REPAIR") )
            kasfSpeechEventQueueCurrentPlayer(268444012, 0);
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
        }
        if ( v6 )
        {
          if ( v6 == 314932 )
          {
            kasTimerSetById(144337492, 3);
            kasTimerStartById(144337492);
            spEventWideScreenIn(90);
            kasfEntrySetValue(332667, 1);
            v67 = 5038930;
            goto LABEL_63;
          }
LABEL_37:
          aitQueuePointer((struct AITeam *)kasCurrentTeamPtr, (struct AITeam *)kasCurrentTeamPtr, (void *)v6);
        }
        if ( aitQueueCount((struct AITeam *)kasCurrentTeamPtr) > 0 && (--v5 & 0x80000000) == 0 )
          continue;
        goto LABEL_41;
      }
      if ( v6 == 27829283 )
      {
        kasfSpeechEventQueueCurrentPlayer(268444010, 0);
        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
      }
      goto LABEL_37;
    }
LABEL_41:
    result = kasfEntryValueGet(71552163);
    if ( !result )
    {
      result = kasTimerExpiredById(73497208);
      if ( result )
      {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        v8 = kasAITeamPtrIfExists(93771865);
        kasfMsgSend((struct AITeam *)v8, (char *)73497208); return;
      }
    }
    return;
  }
  v24 = aitQueueCount((struct AITeam *)kasCurrentTeamPtr);
  if ( v24 < 0 )
    goto LABEL_41;
  while ( 1 )
  {
    v25 = (udword)aitQueuePopPointerByIndex((struct AITeam *)kasCurrentTeamPtr, (udword)v24);
    if ( v25 > 0x71A6331 )
    {
      if ( v25 > 0x96BED57 )
      {
        if ( v25 > 0xE80A041 )
        {
          if ( v25 == 243310658 )
          {
            kasfEntrySetValue(332667, 0);
            kasTimerSetById(144337492, 3);
            kasfEntrySetValue(127756738, 1);
            kasfEntrySetValue(266064926, 1);
            kasfLocationCard(4000, " ");
            v61 = kasAITeamShipsPtrIfExists(143897858);
            kasfTutCameraFocusClose((sdword)v61);
            kasfCameraSetAngleDeg(319);
            kasfCameraSetDeclinationDeg(11);
            kasfTutCameraSetDistanceIgnoreZoom(650);
            v62 = kasAITeamShipsPtrIfExists(143897858);
            kasfShipsSetDontApplyPhysicsFlag((sdword)v62, 0);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
          }
        }
        else
        {
          switch ( v25 )
          {
            case 0xE80A041u:
              kasfOtherKASPause();
              kasfUniversePause();
              kasfEntrySetValue(211831571, 1);
              kasfEntrySetValue(109086661, 1);
              if ( !kasfSensorsIsOpen(0) )
                kasfOpenSensors(1);
              v53 = kasTeamsVectorPtr(143897858);
              kasfSensorsSetLookatPoint((sdword)v53);
              v54 = kasAITeamShipsPtrIfExists(143897858);
              kasfPingAddShips(v54, "PingOnDestroyer");
              v55 = kasAITeamShipsPtrIfExists(143897858);
              soundStopDSound("DestroyerPointer", (sdword)v55);
              kasfSpeechEventQueueCurrentPlayer(268443872, 0);
              v58 = kasAITeamPtrIfExists(160291907);
              kasfMsgSend((struct AITeam *)v58, (char *)90850786);
              v59 = kasAITeamPtrIfExists(143897858);
              kasfMsgSend((struct AITeam *)v59, (char *)90850786);
              v68 = 243310658;
              goto LABEL_130;
            case 0xA897022u:
              kasfOtherKASPause();
              kasfUniversePause();
              kasfEntrySetValue(211831571, 1);
              kasfEntrySetValue(109086661, 1);
              if ( !kasfSensorsIsOpen(0) )
                kasfOpenSensors(1);
              v53 = kasShipsVectorPtrIfExists(82209011);
              kasfSensorsSetLookatPoint((sdword)v53);
              v54 = kasGrowSelectionPtr(82209011);
              soundStopDSound("PiratesPointer", v54);
              kasfSpeechEventQueueCurrentPlayer(268443876, 0);
              v55 = kasAITeamPtrIfExists(176850956);
              kasfMsgSend((struct AITeam *)v55, (char *)313872);
              v68 = 145266441;
              v16 = kasAITeamPtrIfExists(160291907);
LABEL_131:
              kasfMsgSend((struct AITeam *)v16, (char *)v68);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
            case 0xD829744u:
              kasfEntrySetValue(332667, 0);
              kasfSpeechEventQueueCurrentPlayer(268444017, 0);
              kasfAllowPlayerToResearch("TECH_MIMIC");
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
          }
        }
      }
      else
      {
        if ( v25 == 158068055 )
        {
          kasfSpeechEventQueueCurrentPlayer(268444022, 0);
          kasfAllowPlayerToResearch("TECH_AFTERBURNER");
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
        }
        if ( v25 > 0x7C9B3C2 )
        {
          if ( v25 == 157550597 )
          {
#if !defined(HW_Release) && CATACLYSM_DIAG
            {
              static sdword oreCanFocusBranchDiagCount = 0;
              if ( oreCanFocusBranchDiagCount < 4 )
              {
                dbgMessagef(
                  "\n" CATACLYSM_DIAG_LABEL "m02 orecan focus branch: now=%.2f",
                  universe.totaltimeelapsed);
              }
              oreCanFocusBranchDiagCount++;
            }
#endif
            cmCloseIfOpen();
            v66 = kasGrowSelectionPtr(18336307);
            v12 = (int *)kasGrowSelectionPtr(108716498);
            kasfShipsSelectType(v12, v66, "sMothershipOreCanArmour");
#if !defined(HW_Release) && CATACLYSM_DIAG
            {
              GrowSelection *oreCanSelection = (GrowSelection *)v12;
              if (oreCanSelection->selection != NULL && oreCanSelection->selection->numShips > 0)
              {
                Ship *oreCanShip = oreCanSelection->selection->ShipPtr[0];
                dbgMessagef(
                  "\n" CATACLYSM_DIAG_LABEL
                  "m02 orecan focus selection: now=%.2f count=%u ship=%x pos=(%.3f,%.3f,%.3f) flags=%x",
                  universe.totaltimeelapsed,
                  oreCanSelection->selection->numShips,
                  oreCanShip,
                  oreCanShip->posinfo.position.x,
                  oreCanShip->posinfo.position.y,
                  oreCanShip->posinfo.position.z,
                  oreCanShip->flags);
              }
              else
              {
                dbgMessagef(
                  "\n" CATACLYSM_DIAG_LABEL "m02 orecan focus selection: now=%.2f count=0",
                  universe.totaltimeelapsed);
              }
            }
#endif
            AngleDeg = kasfCameraGetAngleDeg();
            kasfEntryGetOrCreateSetValue(260833221, AngleDeg);
            DeclinationDeg = kasfCameraGetDeclinationDeg();
            kasfEntryGetOrCreateSetValue(60936190, DeclinationDeg);
            Distance = kasfCameraGetDistance();
            kasfEntryGetOrCreateSetValue(64272901, Distance);
            v52 = kasGrowSelectionPtr(108716498);
            kasfTutCameraFocusClose(v52);
            kasfSpeechEventQueueCurrentPlayer(268439915, 0);
            v68 = 104890610;
            goto LABEL_130;
          }
        }
        else
        {
          switch ( v25 )
          {
            case 0x7C9B3C2u:
              kasfSpeechEventQueueCurrentPlayer(33559242, 0);
              v68 = 158068055;
              goto LABEL_130;
            case 0x71A6332u:
              v46 = kasAITeamShipsPtrIfExists(93771865);
              kasfTutCameraFocusClose((sdword)v46);
              kasfSpeechEventQueueCurrentPlayer(268439919, 0);
              v47 = kasAITeamPtrIfExists(176850956);
              kasfMsgSend((struct AITeam *)v47, (char *)119169843);
              kasTimerCreateSetStartById(73497208, 3);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
            case 0x71A6333u:
              kasTimerSetById(144337492, 3);
              kasfEntrySetValue(127756738, 1);
              kasfEntrySetValue(266064926, 1);
              kasfEntrySetValue(332667, 0);
              v45 = kasAITeamPtrIfExists(93771865);
              kasfMsgSend((struct AITeam *)v45, (char *)73497208);
              kasfSpeechEventQueueCurrentPlayer(268444016, 0);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
          }
        }
      }
      goto LABEL_106;
    }
    if ( v25 == 119169841 )
    {
      kasTimerSetById(144337492, 3);
      kasfEntrySetValue(127756738, 1);
      kasfEntrySetValue(52577235, 1);
      kasfEntrySetValue(266064926, 1);
      kasfOtherKASPause();
      kasfUniversePause();
      kasfEntrySetValue(211831571, 1);
      kasfEntrySetValue(109086661, 1);
      kasfEntrySetValue(332667, 0);
      if ( !kasfSensorsIsOpen(0) )
        kasfOpenSensors(1);
      v40 = kasTeamsVectorPtr(93771865);
      kasfSensorsSetLookatPoint((sdword)v40);
      v42 = kasAITeamShipsPtrIfExists(93771865);
      soundStopDSound("TrickyPointer", (sdword)v42);
      kasfSpeechEventQueueCurrentPlayer(268439918, 0);
      kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
    }
    if ( v25 > 0x275C542 )
      break;
    if ( v25 == 41272642 )
    {
      kasfSpeechEventQueueCurrentPlayer(268439775, 0);
      v39 = kasAITeamPtrIfExists(160291907);
      kasfMsgSend((struct AITeam *)v39, (char *)306724);
      v68 = 4693508;
      goto LABEL_130;
    }
    if ( v25 > 0x4CA10 )
    {
      if ( v25 == 4693508 )
      {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEventQueueCurrentPlayer(268444008, 0);
        kasfEntryGetOrCreateSetValue(-1889385931, 1);
        v68 = 4693508;
        v16 = kasAITeamPtrIfExists(160291907);
        goto LABEL_131;
      }
      if ( v25 == 5038930 )
      {
        kasfOtherKASPause();
        kasfUniversePause();
        kasfEntrySetValue(211831571, 1);
        kasfEntrySetValue(109086661, 1);
        if ( !kasfSensorsIsOpen(0) )
          kasfOpenSensors(1);
        kasTimerSetById(144337492, 3);
        kasfEntrySetValue(127756738, 1);
        kasfEntrySetValue(52577235, 1);
        kasfEntrySetValue(266064926, 1);
        kasfEntrySetValue(332667, 0);
        kasfLabelledVectorSet(0x529631, 0, 0, 0);
        kasfLabelledVectorSet(5412402, 0, 0, 0);
        v64 = kasTeamsVectorPtr(143897858);
        v29 = kasVectorPtrIfExists(0x529631);
        kasfVectorRandomizeInRange(v29, v64, 20000);
        v65 = kasTeamsVectorPtr(143897858);
        v30 = kasVectorPtrIfExists(5412402);
        kasfVectorRandomizeInRange(v30, v65, 20000);
        kasfForceAttack(kasGrowSelectionPtr(82209011), kasVectorPtrIfExists(0x529631), 0);
        kasfForceAttack(kasAITeamShipsPtrIfExists(143897858), kasVectorPtrIfExists(5412402), 0);
        kasfSpeechEventQueueCurrentPlayer(268439917, 0);
        v68 = 92566517;
        goto LABEL_130;
      }
    }
    else
    {
      switch ( v25 )
      {
        case 0x4CA10u:
          v29 = kasfCameraGetAngleDeg();
          kasfEntryGetOrCreateSetValue(260833221, v29);
          v30 = kasfCameraGetDeclinationDeg();
          kasfEntryGetOrCreateSetValue(60936190, v30);
          v31 = kasfCameraGetDistance();
          kasfEntryGetOrCreateSetValue(64272901, v31);
          v32 = kasGrowSelectionPtr(218060896);
          kasfTutCameraFocusClose(v32);
          kasfCameraSetAngleDeg(345);
          kasfCameraSetDeclinationDeg(3);
          kasfTutCameraSetDistanceIgnoreZoom(2472);
          kasfSpeechEventQueueCurrentPlayer(268439781, 0);
          v68 = 293513;
          goto LABEL_130;
        case 0u:
          goto LABEL_107;
        case 0x47A89u:
          kasTimerSetById(144337492, 10);
          kasfEntrySetValue(127756738, 1);
          kasfEntrySetValue(266064926, 1);
          kasfEntrySetValue(232774915, 1);
          v27 = kasAITeamShipsPtrIfExists(143897858);
          kasfTutCameraFocusClose((sdword)v27);
          kasfCameraSetAngleDeg(30);
          kasfCameraSetDeclinationDeg(7);
          kasfTutCameraSetDistanceIgnoreZoom(650);
          kasfSpeechEventQueueCurrentPlayer(268468454, 0);
          v28 = kasAITeamPtrIfExists(143897858);
          kasfMsgSend((struct AITeam *)v28, (char *)293513);
          v68 = 226662212;
LABEL_130:
          v16 = kasAITeamPtrIfExists(176850956);
          goto LABEL_131;
      }
    }
LABEL_106:
    aitQueuePointer((struct AITeam *)kasCurrentTeamPtr, (struct AITeam *)kasCurrentTeamPtr, (void *)v25);
LABEL_107:
    if ( aitQueueCount((struct AITeam *)kasCurrentTeamPtr) > 0 && (--v24 & 0x80000000) == 0 )
      continue;
    goto LABEL_41;
  }
  if ( v25 > 0x64080F2 )
  {
    if ( v25 == 119169810 )
    {
      kasfSpeechEventQueueCurrentPlayer(268468450, 0);
      v68 = 92566517;
      goto LABEL_130;
    }
    goto LABEL_106;
  }
  if ( v25 == 104890610 )
  {
#if !defined(HW_Release) && CATACLYSM_DIAG
    {
      static sdword oreCanLocationCardDiagCount = 0;
      if ( oreCanLocationCardDiagCount < 4 )
      {
        dbgMessagef(
          "\n" CATACLYSM_DIAG_LABEL "m02 orecan location card: now=%.2f",
          universe.totaltimeelapsed);
      }
      oreCanLocationCardDiagCount++;
    }
#endif
    kasfEntrySetValue(172907219, 1);
    kasfEntrySetValue(144337492, 3);
    kasfEntrySetValue(127756738, 1);
    kasfEntrySetValue(332667, 0);
    kasfEntrySetValue(266064926, 1);
    kasfLocationCard(5000, " ");
    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_PromptCleanup); return;
  }
  if ( v25 == 74012734 )
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
    if ( !kasfSensorsIsOpen(0) )
      kasfOpenSensors(150000);
    v40 = kasTeamsVectorPtr(118669236);
    kasfSensorsSetLookatPoint((sdword)v40);
    v41 = kasAITeamShipsPtrIfExists(118669236);
    kasfPingAddShips(v41, "PingMessagePod");
    v42 = kasAITeamShipsPtrIfExists(118669236);
    soundStopDSound("TrickyPointer", (sdword)v42);
    kasfSpeechEventQueueCurrentPlayer(268443875, 0);
    v68 = 74012734;
    v16 = kasAITeamPtrIfExists(160291907);
    goto LABEL_131;
  }
  if ( v25 != 92566517 )
    goto LABEL_106;
  kasTimerSetById(144337492, 3);
  kasTimerStartById(144337492);
  kasfWideScreenIn(90);
  v67 = 74012734;
LABEL_63:
  v10 = kasAITeamPtrIfExists(176850956);
LABEL_64:
  kasfMsgSend((struct AITeam *)v10, (char *)v67);
  kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_MissionFlowDelay); return;
}

void Init_Mission02_HonourGuardSwarmPatrol(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_HonourGuardSwarmPatrol(void)
{
  MaintainTeamContext();
}

void Init_Mission02_HonourGuardSwarmMoveToRandomPoint(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_HonourGuardSwarmDock(void)
{
  MaintainTeamContext();
}

void Init_Mission02_HonourGuardSwarmCollectNearbyShips(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_HonourGuardSwarmCollectNearbyShips(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_ObjectiveEventQueue(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    void *message;
    AITeam *fleetIntel;
    team = (AITeam *)dword_A43EE8;
    dword_C3749C = 1;
    dword_C374A0 = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = -1;
    if (queueHead != NULL)
    {
        queueIndex = *queueHead;
    }

    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message == (void *)306724)
        {
            kasfObjectiveCreate("FindDestroyer", (char *)m02_findDestroyer_brief[strCurLanguage], (char *)m02_findDestroyer_full[strCurLanguage]);
        }
        else if (message == (void *)4693508)
        {
            kasfObjectiveCreate("Build", (char *)m02_buildEngineering_brief[strCurLanguage], (char *)m02_buildEngineering_full[strCurLanguage]);
        }
        else if (message == (void *)4693524)
        {
            kasfObjectiveDestroy("Build");
        }
        else if (message == (void *)334437)
        {
            kasfObjectiveCreate("Build", (char *)m02_buildEngineering_brief[strCurLanguage], (char *)m02_buildEngineering_full[strCurLanguage]);
        }
        else if (message == (void *)145266441)
        {
            kasfObjectiveCreate("DestroyPirates", (char *)m02_destroyPirates_brief[strCurLanguage], (char *)m02_destroyPirates_full[strCurLanguage]);
        }
        else if (message == (void *)74012734)
        {
            kasfObjectiveCreate("SalvagePod", (char *)m02_salvagePod_brief[strCurLanguage], (char *)m02_salvagePod_full[strCurLanguage]);
            kasfTutSetPointerTargetShipSelection("ObjPtr", kasAITeamShipsPtrIfExists(118669236));
        }
        else if (message == (void *)90850786)
        {
            kasfPingRemove("PingHint1");
            kasfPingRemove("PingHint2");
            kasfPingRemove("PingHint3");
            kasfObjectiveDestroy("FindDestroyer");
            kasfObjectiveCreate("RepairDestroyer", (char *)m02_repairDestroyer_brief[strCurLanguage], (char *)m02_repairDestroyer_full[strCurLanguage]);
            kasfTutSetPointerTargetShipSelection("ObjPtr", kasAITeamShipsPtrIfExists(143897858));
        }
        else if (message == (void *)144114996)
        {
            kasfEntryGetOrCreateSetValue(-2048692364, 1);
            kasfObjectiveDestroy("DestroyPirates");
        }
        else if (message == (void *)153611316)
        {
            kasfEntryGetOrCreateSetValue(-2084269596, 1);
            kasfTutRemovePointer("ObjPtr");
            kasfObjectiveDestroy("SalvagePod");
        }
        else if (message == (void *)172028660)
        {
            kasfTutRemovePointer("ObjPtr");
            kasfObjectiveDestroy("RepairDestroyer");
            kasfPingRemove("PingOnDestroyer");
        }
        else if (message == (void *)175938357)
        {
            kasfObjectiveDestroyAll();
            kasfObjectiveCreate("Hyperspace", (char *)m02_hyperspace_brief[strCurLanguage], (char *)m02_hyperspace_full[strCurLanguage]);
            spSetSinglePlayerSaveInfo(2, ((char **)0x8C84B0)[strCurLanguage]);
        }
        else if (message == (void *)220350372)
        {
            kasfMissionFailed();
        }
        else if (message == (void *)262155621)
        {
            kasfObjectiveCreate("BuildHangarMod", (char *)m02_buildHangarMod_brief[strCurLanguage], (char *)m02_buildHangarMod_full[strCurLanguage]);
        }
        else if (message == (void *)262155637)
        {
            kasfObjectiveDestroy("BuildHangarMod");
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = -1;
        if (queueHead != NULL)
        {
            queueIndex = *queueHead;
        }
        if (queueIndex >= 0)
        {
            queueIndex--;
        }
    }

    if (!kasfEntryValueGet(71552164) && kasfEntryValueGet(-2084269596))
    {
        if (kasfEntryValueGet(-2048692364))
        {
            kasfEntryGetOrCreateSetValue(71552164, 1);
            fleetIntel = kasAITeamPtrIfExists(176850956);
            kasfMsgSend(fleetIntel, (void *)175938357);
        }
    }
}

void Watch_Mission02_DispatchPirateWaves(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfLabelledVectorSet(109885504, 0, 0, 0);
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(109885504), kasTeamsVectorPtr(118669236), 50000);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)145266441);
        kasfMsgSend(kasAITeamPtrIfExists(123838693), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(230547089), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(230547090), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(230547091), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(88905316), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(124217570), (char *)5801588);
    }
}

void Init_Mission02_PirateAssaultTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)dword_A43EE8)->shipList, 5);
    kasfShipsAdd(kasGrowSelectionPtr(82209011), &((AITeam *)dword_A43EE8)->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
}

void Watch_Mission02_PirateAssaultTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfEntryValueGet(71552165) || kasfTeamCount())
    {
        if (!kasfEntryValueGet(71552166) &&
            (kasfEntryValueGet(-2084269596) || kasfMsgReceived((char *)130908126)))
        {
            kasfEntryGetOrCreateSetValue(71552166, 1);
            kasfEntryGetOrCreateSetValue(-2035735516, 1);
            kasfAttackMothership();
            kasfSpeechEvent(126523733, (sdword)Init_Mission02_PirateAssaultTeamDespawnDelay, (sdword)Watch_Mission02_PirateAssaultTeamDespawnDelay);
        }
        else if (kasfMsgReceived((char *)5801588))
        {
            kasfTeamHyperspaceIn(kasVectorPtrIfExists(109885504));
            kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_PirateAssaultTeamFollowup);
        }
    }
    else
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfEntryGetOrCreateSetValue(-2035735516, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_RequestPirateStrikeGroups);
    }
}

void Init_Mission02_PirateMissileCorvetteAttackRun(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateMissileCorvetteAttackRun(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateMissileCorvetteRetarget(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateMissileCorvetteRetarget(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateMissileCorvetteReinforcements(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateCarrierLaunchCycle(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateCarrierLaunchCycle(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateMissileCorvetteTeam(void)
{
    kasAITeamShipsPtr(&((AITeam *)dword_A43EE8)->shipList, 5);
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasfShipsAdd(kasGrowSelectionPtr(82209011), &((AITeam *)dword_A43EE8)->shipList);
    kasfEntryGetOrCreateSetValue(332667, 1);
}

void Watch_Mission02_PirateMissileCorvetteTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588)
        || (!kasfEntryValueGet(332667)
            && kasfUnderAttackElsewhere(kasAITeamPtrIfExists(124217570), kasGrowSelectionPtr(-3936141))
            && !kasfTeamCount()))
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }
    if (kasfEntryValueGet(-2035735516))
    {
        kasfSpeechEvent(102625813, (sdword)Init_Mission02_ReassignSurvivingAttackTeam, (sdword)MaintainTeamContext);
    }
}

void Init_Mission02_PirateStrikeTeamWave(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasAITeamShipsPtr(&((AITeam *)dword_A43EE8)->shipList, 5);
    kasfShipsAdd(kasGrowSelectionPtr(82209011), &((AITeam *)dword_A43EE8)->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_PirateStrikeTeamRedirect);
}

void Watch_Mission02_PirateStrikeTeamWave(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        if (!kasfEntryValueGet(71552168) && kasfEntryValueGet(-2035735516))
        {
            kasfEntryGetOrCreateSetValue(71552168, 1);
            kasfSpeechEvent(102625813, (sdword)Init_Mission02_ReassignSurvivingAttackTeam, (sdword)MaintainTeamContext);
        }
    }
    else if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission02_PirateStrikeTeamReinforcementDelay, (sdword)Watch_Mission02_PirateStrikeTeamReinforcements);
        return;
    }
    else if (kasfTeamHealthAverage() >= 15)
    {
        if (!kasfEntryValueGet(71552168) && kasfEntryValueGet(-2035735516))
        {
            kasfEntryGetOrCreateSetValue(71552168, 1);
            kasfSpeechEvent(102625813, (sdword)Init_Mission02_ReassignSurvivingAttackTeam, (sdword)MaintainTeamContext);
        }
    }
    else
    {
        kasfSpeechEvent(299899, (sdword)Init_Mission02_PirateStrikeTeamDockCycle, (sdword)Watch_Mission02_PirateStrikeTeamDockCycle);
    }
}

void Watch_Mission02_PirateStrikeTeamRedirect(void)
{
    dword_C3749C = 2;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntrySetValue(332667, 0);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(109885504));
        kasfSpeechEvent(374244, (sdword)Init_Mission02_PirateStrikeTeamAttackRun, (sdword)Watch_Mission02_PirateStrikeTeamAttackRun);
    }
}

void Init_Mission02_PirateStrikeTeamAttackRun(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateStrikeTeamAttackRun(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateStrikeTeamRetarget(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateStrikeTeamRetarget(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateStrikeTeamReinforcementDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateStrikeTeamReinforcements(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateStrikeTeamDockCycle(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateStrikeTeamPostLaunch(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateHeavyStrikeTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasAITeamShipsPtr(&((AITeam *)dword_A43EE8)->shipList, 5);
    kasfShipsAdd(kasGrowSelectionPtr(82209011), &((AITeam *)dword_A43EE8)->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_PirateHeavyStrikeTeamRedirect);
}

void Watch_Mission02_PirateHeavyStrikeTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        if (!kasfEntryValueGet(71552169) && kasfEntryValueGet(-2035735516))
        {
            kasfEntryGetOrCreateSetValue(71552169, 1);
            kasfSpeechEvent(102625813, (sdword)Init_Mission02_ReassignSurvivingAttackTeam, (sdword)MaintainTeamContext);
        }
    }
    else if (kasfMsgReceived((char *)80111235))
    {
        kasfSpeechEvent(74024315, (sdword)Init_Mission02_PirateHeavyStrikeTeamAttackRun, (sdword)Watch_Mission02_PirateHeavyStrikeTeamAttackRun);
        return;
    }
    else if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission02_PirateHeavyStrikeTeamReinforcementDelay, (sdword)Watch_Mission02_PirateHeavyStrikeTeamReinforcements);
        return;
    }
    else if (kasfTeamHealthAverage() >= 25)
    {
        if (!kasfEntryValueGet(71552169) && kasfEntryValueGet(-2035735516))
        {
            kasfEntryGetOrCreateSetValue(71552169, 1);
            kasfSpeechEvent(102625813, (sdword)Init_Mission02_ReassignSurvivingAttackTeam, (sdword)MaintainTeamContext);
        }
    }
    else
    {
        kasfSpeechEvent(299899, (sdword)Init_Mission02_PirateStrikeTeamDockCycle, (sdword)Watch_Mission02_PirateHeavyStrikeTeamDockCycle);
    }
}

void Watch_Mission02_PirateHeavyStrikeTeamRedirect(void)
{
    dword_C3749C = 2;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(109885504));
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Init_Mission02_PirateHeavyStrikeTeamAttackRun, (sdword)Watch_Mission02_PirateHeavyStrikeTeamAttackRun);
    }
}

void Init_Mission02_PirateHeavyStrikeTeamAttackRun(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateHeavyStrikeTeamAttackRun(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateHeavyStrikeTeamReinforcementDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateHeavyStrikeTeamReinforcements(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateHeavyStrikeTeamDockCycle(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateHeavyStrikeTeamPostLaunch(void)
{
  MaintainTeamContext();
}

void Init_Mission02_ReassignSurvivingAttackTeam(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateSalvageCorvetteTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)dword_A43EE8)->shipList, 5);
    kasfEntryGetOrCreateSetValue(332667, 1);
    kasfActivateTech("TECH_SALVAGE");
    kasfShipsAdd(kasGrowSelectionPtr(82209011), &((AITeam *)dword_A43EE8)->shipList);
    kasfShipsApplyPlayerUpgrades(&((AITeam *)dword_A43EE8)->shipList);
    kasfSetSalCapProcessingShip(kasAITeamShipsPtrIfExists(124217570));
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_PirateSalvageCorvetteIntro);
}

void Watch_Mission02_PirateSalvageCorvetteTeam(void)
{
    dword_C3749C = 1;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfEntryValueGet(332667))
    {
        if (!kasfEntryValueGet(71552170) && kasfEntryValueGet(-2035735516))
        {
            kasfEntryGetOrCreateSetValue(71552170, 1);
            kasfSpeechEvent(102625813, (sdword)Init_Mission02_PirateSalvageCorvetteCleanup, (sdword)MaintainTeamContext);
        }
    }
    else if (kasfMsgReceived((char *)74024315))
    {
        kasfSpeechEvent(123839973, (sdword)Init_Mission02_PirateSalvageCorvetteSpecialAttack, (sdword)Watch_Mission02_PirateSalvageCorvetteSpecialAttack);
        return;
    }
    else if (kasfTeamCount())
    {
        if (!kasfEntryValueGet(71552170) && kasfEntryValueGet(-2035735516))
        {
            kasfEntryGetOrCreateSetValue(71552170, 1);
            kasfSpeechEvent(102625813, (sdword)Init_Mission02_PirateSalvageCorvetteCleanup, (sdword)MaintainTeamContext);
        }
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)Init_Mission02_PirateCarrierStrikeReinforcementCycle, (sdword)Watch_Mission02_PirateCarrierStrikeReinforcementCycle);
    }
}

void Watch_Mission02_PirateSalvageCorvetteIntro(void)
{
    dword_C3749C = 2;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfEntrySetValue(332667, 0);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(109885504));
        kasfSpeechEvent(374244, (sdword)Init_Mission02_PirateSalvageCorvetteDelay, (sdword)Watch_Mission02_PirateSalvageCorvetteDelay);
    }
}

void Init_Mission02_PirateSalvageCorvetteDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateSalvageCorvetteDelay(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateSalvageCorvetteStateChoice(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateSalvageCorvetteSpecialAttack(void)
{
    dword_C3749C = 2;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasfFormationBroad();
    kasfAttackSpecial(kasAITeamShipsPtrIfExists(118669236));
}

void Watch_Mission02_PirateSalvageCorvetteSpecialAttack(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateSalvageCorvetteRetarget(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateSalvageCorvetteRetarget(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateCarrierStrikeReinforcementCycle(void)
{
    dword_C3749C = 2;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (!kasfEntryValueGet(71552171))
    {
        kasfEntryGetOrCreateSetValue(71552171, 1);
        kasfEntryGetOrCreateSetValue(5302947, 5);
    }
    if (kasfEntryValueGet(5302947))
    {
        kasTimerCreateSetStartById(4693508, kasfRandom(40, 60));
    }
    else
    {
        kasfMsgSend(kasAITeamPtrIfExists(124217570), (char *)32963987);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission02_PirateCarrierStrikeReinforcementCycle(void)
{
    dword_C3749C = 2;
    dword_C374A0 = ((AITeam *)dword_A43EE8)->kasLabelHash;
    if (kasTimerExpiredById(4693508) && kasfShipsCount(kasAITeamShipsPtrIfExists(124217570)))
    {
        kasfShipsCreateInside(
            kasGrowSelectionPtr(154639956),
            "P1StandardCorvette",
            kasfEntryValueGet(154639956),
            kasAITeamShipsPtrIfExists(124217570),
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 5);
        kasfReinforceTeamWithShips((AITeam *)dword_A43EE8, kasGrowSelectionPtr(154639956));
    }
    if (kasfTeamFinishedLaunching())
    {
        AITeam *targetTeam;

        if (kasfShipsCount(kasAITeamShipsPtrIfExists(230547089)))
        {
            if (kasfShipsCount(kasAITeamShipsPtrIfExists(230547090)))
            {
                kasfEntryDecrement(5302947);
                kasfEntrySetValue(332667, 0);
                kasfSpeechEvent(374244, (sdword)Init_Mission02_PirateSalvageCorvetteDelay, (sdword)Watch_Mission02_PirateSalvageCorvetteDelay);
                return;
            }
            targetTeam = kasAITeamPtrIfExists(230547090);
        }
        else
        {
            targetTeam = kasAITeamPtrIfExists(230547089);
        }
        kasfMsgSend(targetTeam, (char *)178751685);
        kasfEntryDecrement(5302947);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(374244, (sdword)Init_Mission02_PirateSalvageCorvetteDelay, (sdword)Watch_Mission02_PirateSalvageCorvetteDelay);
    }
}

void Init_Mission02_PirateSalvageCorvetteCleanup(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateReserveAttackTeam(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateReserveAttackTeam(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateReserveAttackRun(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateReserveAttackRun(void)
{
  MaintainTeamContext();
}

void Init_Mission02_SalvagePodApproach(void)
{
  MaintainTeamContext();
}

void Init_Mission02_SalvagePodHideDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_SalvagePodHideDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_SalvagePodApproach(void)
{
  MaintainTeamContext();
}

void Init_Mission02_SalvagePodAmbushDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_SalvagePodAmbushDelay(void)
{
  MaintainTeamContext();
}

void Init_Mission02_DamagedDestroyer(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_DamagedDestroyer(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_DamagedDestroyerRepairStart(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_DamagedDestroyerRepairProgress(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_DamagedDestroyerAttackCue(void)
{
  MaintainTeamContext();
}

void Init_Mission02_DamagedDestroyerDespawnDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_DamagedDestroyerDespawnDelay(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateAdvanceSupportFrigateTeam(void)
{
    dword_C3749C = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
    kasfActivateTech("TECH_MIMIC");
    kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
    kasfShipsProcessModuleSlotOrder(&teamtoreinforce->shipList, "kAdvanceSupportFrigate", 0);
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 5);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_PirateAdvanceSupportFrigateTeam);
}

void Watch_Mission02_PirateAdvanceSupportFrigateTeam(void)
{
    hvector *hyperspacePoint;

    dword_C3749C = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfLabelledVectorSet(0x051E1D3, 0, 0, 0);
        hyperspacePoint = kasVectorPtrIfExists(0x051E1D3);
        kasfVectorRandomizeInRange(hyperspacePoint, kasTeamsVectorPtr(KH_M02_kDestroyer), 45000);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x051E1D3));
        kasfMsgSend(kasAITeamPtrIfExists(KH_M02_Decoy), (char *)5801588);
        kasfSpeechEvent(0x04617A78, (sdword)Init_Mission02_PirateAdvanceSupportFrigateRepairOrder, (sdword)Watch_Mission02_PirateAdvanceSupportFrigateRepairOrder);
    }
}

void Init_Mission02_PirateAdvanceSupportFrigateRepairOrder(void)
{
    dword_C3749C = 2;
    scope = teamtoreinforce->kasLabelHash;
    kasfFormationDelta();
    kasfIntercept(kasAITeamShipsPtrIfExists(KH_M02_kDestroyer));
}

void Watch_Mission02_PirateAdvanceSupportFrigateRepairOrder(void)
{
    dword_C3749C = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(71552177) && std_ctype_char_Term())
    {
        kasfEntryGetOrCreateSetValue(71552177, 1);
        kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)0x0A6345B7);
    }
    if (kasfMsgReceived((char *)KH_M02_FSM_Ambush))
    {
        kasfMusicPlay(20);
        kasfMsgSend(kasAITeamPtrIfExists(KH_M02_Ambush1), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(KH_M02_Decoy), (char *)KH_M02_FSM_Ambush);
        kasfEntryGetOrCreateSetValue(0x83617EA8, 1);
        kasfDisableShips(&teamtoreinforce->shipList);
        kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 0);
        kasfSpeechEvent(74024315, (sdword)Init_Mission02_PirateAdvanceSupportFrigateDefenseLoop, (sdword)Watch_Mission02_PirateAdvanceSupportFrigateDefenseLoop);
    }
}

void Init_Mission02_PirateAdvanceSupportFrigateDefenseLoop(void)
{
    GrowSelection *attackTargets;

    dword_C3749C = 2;
    scope = teamtoreinforce->kasLabelHash;
    kasfShipsSelectNearby(
        kasGrowSelectionPtr(0x036980EB),
        kasGrowSelectionPtr(KH_Enemy),
        ((hvector *(__cdecl *)(void))kasShipsVectorPtr)(),
        10000);
    kasfShipsAdd(kasGrowSelectionPtr(0x036980EB), kasAITeamShipsPtrIfExists(KH_M02_kDestroyer));
    if (kasfUnderAttack(kasGrowSelectionPtr(0x0857BCE3)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x036980EB), kasGrowSelectionPtr(0x0857BCE3));
    }
    if (!kasfShipsCount(kasGrowSelectionPtr(0x036980EB)))
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x036980EB), kasGrowSelectionPtr(KH_Enemy));
    }
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x036980EB), ((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
    kasfFormationDelta();
    attackTargets = kasGrowSelectionPtr(0x036980EB);
    tutDeallocateRootRegion((sdword)attackTargets);
    kasTimerCreateSetStartById(0x0469857B, kasfRandom(10, 30));
}

void Watch_Mission02_PirateAdvanceSupportFrigateDefenseLoop(void)
{
    dword_C3749C = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&teamtoreinforce->shipList) || kasTimerExpiredById(74024315))
        {
            kasfSpeechEvent(74024315, (sdword)Init_Mission02_PirateAdvanceSupportFrigateDefenseLoop, (sdword)Watch_Mission02_PirateAdvanceSupportFrigateDefenseLoop);
        }
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission02_PirateTeamReinforcementTeam(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateTeamReinforcementTeam(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateTeamReinforcements(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateTeamAttackDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateTeamAttackDelay(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateTeamDefenseLoop(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateTeamDefenseLoop(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateTeamReinforcementDelay(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_PirateTeamReinforcementDelay(void)
{
  MaintainTeamContext();
}

void Init_Mission02_PirateTeamReinforcementGate(void)
{
  MaintainTeamContext();
}

void Init_Mission02_AttackMothershipTeam(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_AttackMothershipTeam(void)
{
  MaintainTeamContext();
}

void Mission02_SetAttackApproachVector(void)
{
  MaintainTeamContext();
}

void Mission02_SendTeamToAttackTarget(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_TeamState_5E03D0(void)
{
  MaintainTeamContext();
}

void Init_Mission02_TeamState_5E0440(void)
{
  MaintainTeamContext();
}

void Watch_Mission02_TeamState_5E0540(void)
{
  MaintainTeamContext();
}

sdword Watch_Mission02_PirateAssaultTeamFollowup(void)
{
    sdword result;
    GrowSelection *pirates;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)32963859))
    {
        kasfEntryGetOrCreateSetValue(-2035735516, 1);
        kasfAttackMothership();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_RequestPirateStrikeGroups);
        return 1;
    }

    result = kasfTeamCount();
    if (!result)
    {
        kasfEntryGetOrCreateSetValue(-2035735516, 1);
        pirates = kasGrowSelectionPtr(KH_M02_Pirates);
        result = kasfShipsCount(pirates);
        if (!result)
        {
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_RequestPirateStrikeGroups);
            return 1;
        }
    }
    return result;
}

void Init_Mission02_PirateAssaultTeamDespawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(300093, 10);
}

sdword Watch_Mission02_PirateAssaultTeamDespawnDelay(void)
{
    sdword result;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    result = kasTimerExpiredById(300093);
    if (result)
    {
        kasfDeleteShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_RequestPirateStrikeGroups);
    }
    return result;
}

sdword Watch_Mission02_RequestPirateStrikeGroups(void)
{
    sdword result;
    GrowSelection *strikeTargets;
    GrowSelection *enemyShips;
    GrowSelection *pirates;
    AITeam *fleetIntel;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    result = kasfEntryValueGet(71552167);
    if (!result)
    {
        strikeTargets = kasGrowSelectionPtr(139967715);
        enemyShips = kasGrowSelectionPtr(KH_Enemy);
        result = kasfFindShipsTargetingSelection(enemyShips, strikeTargets);
        if (!result)
        {
            pirates = kasGrowSelectionPtr(KH_M02_Pirates);
            result = kasfShipsCount(pirates);
            if (!result)
            {
                kasfEntryGetOrCreateSetValue(71552167, 1);
                fleetIntel = kasAITeamPtrIfExists(176850956);
                kasfMsgSend(fleetIntel, (char *)144114996);
                return 1;
            }
        }
    }
    return result;
}

void Init_Mission02_PirateStrikeTeamDockCycle(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfDockStay(kasAITeamPtrIfExists(KH_M02_pCarrier));
}

void Init_Mission02_FleetIntel_SetupState(void)
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
  kasfEntryGetOrCreateSetValue(127756738, 0);
  kasfTutEnableFlags("Focus");
  kasfTutEnableFlags("CancelFocus");
  kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission02_FleetIntel_EventFlow); return;
}

void Watch_Mission02_FleetIntel_CleanupAndResume(void)
{
  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  if ( kasfIntelEventEnded() || kasfEntryValueGet(-2054310428) )
  {
    kasfEntrySetValue(-2054310428, 0);
    kasfSubtitleDraw();
    if ( kasfEntryValueGet(266064926) )
    {
      kasfWideScreenOut(90);
      kasfEntrySetValue(266064926, 0);
    }
    if ( kasfEntryValueGet(109086661) )
    {
      kasfUniverseUnpause();
      kasfEntrySetValue(109086661, 0);
    }
    if ( kasfEntryValueGet(211831571) )
    {
      kasfOtherKASUnpause();
      kasfEntrySetValue(211831571, 0);
    }
    if ( kasfEntryValueGet(52577235) )
    {
      kasfCloseSensors(1);
      kasfEntrySetValue(52577235, 0);
    }
    if ( kasfEntryValueGet(172907219) )
    {
      kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
      kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
      kasfCameraSetDistance(kasfEntryValueGet(64272901));
      kasfTutCameraFocusCancel();
      kasfEntrySetValue(172907219, 0);
    }
    if ( kasfEntryValueGet(232774915) )
    {
      kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
      kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
      kasfCameraSetDistance(kasfEntryValueGet(64272901));
      kasfTutCameraFocusCancel();
      kasfTutCameraFocusCancel();
      kasfEntrySetValue(232774915, 0);
    }
    if ( kasfEntryValueGet(127756738) )
    {
      kasTimerStartById(144337492);
      kasfEntrySetValue(127756738, 0);
    }
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_FleetIntel_ResumeDelay); return;
  }
}

void Watch_Mission02_FleetIntel_ResumeDelay(void)
{
  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  if ( kasTimerExpiredDestroyById(144337492) )
  {
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission02_FleetIntel_EventFlow); return;
  }
}

void Watch_Mission02_FleetIntel_EventFlow(void)
{
int v0; // eax
int result; // eax
int v2; // edx
int *v3; // ecx
int v4; // esi
char *v5; // eax
void *v6; // eax
void *v7; // eax
void *v8; // eax
void *v9; // eax
void *v10; // eax
void *v11; // eax
void *v12; // eax
void *v13; // eax
void *v14; // eax
void *v15; // eax
void *v16; // eax
void *v17; // eax
void *v18; // eax
void *v19; // eax
int v20; // edx
int *v21; // ecx
int v22; // esi
unsigned int v23; // eax
int *v24; // eax
int *v25; // eax
int v26; // eax
void *v27; // eax
void *v28; // eax
void *v29; // eax
void *v30; // eax
void *v31; // eax
void *v32; // eax
void *v33; // eax
void *v34; // eax
void *v35; // eax
void *v36; // eax
void *v37; // eax
void *v38; // eax
void *v39; // eax
void *v40; // eax
void *v41; // eax
void *v42; // eax
void *v43; // eax
int *v44; // eax
int v45; // eax
int AngleDeg; // eax
int DeclinationDeg; // eax
int Distance; // eax
int v49; // eax
int v50; // eax
int v51; // eax
int v52; // eax
void *v53; // eax
int v54; // eax
int v55; // eax
int v56; // eax
int v57; // eax
void *v58; // eax
int *v59; // eax
int v60; // eax
signed int *v61; // eax
int v62; // eax
int v63; // eax
int v64; // eax
void *v65; // eax
void *v66; // eax
int v67; // [esp-8h] [ebp-Ch]
int v68; // [esp-8h] [ebp-Ch]
int v69; // [esp-4h] [ebp-8h]
int v70; // [esp-4h] [ebp-8h]
int v71; // [esp-4h] [ebp-8h]
int v72; // [esp-4h] [ebp-8h]
int *v73; // [esp-4h] [ebp-8h]

kasCurrentStep = 2;
kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
v0 = kasGrowSelectionPtr(218060896);
if ( !kasfShipsCount(v0) )
{
kasfMissionFailed();
kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
return;
}
if ( !kasfEntryValueGet(332667) )
{
v2 = kasCurrentTeamPtr;
v3 = *(int **)(kasCurrentTeamPtr + 80);
result = -1;
if ( v3 )
result = *v3;
v4 = result;
if ( result < 0 )
return;
while ( 1 )
{
v5 = (char *)aitQueuePopPointerByIndex((AITeam *)v2, v4);
if ( (unsigned int)v5 > 0x58C6303 )
{
if ( (unsigned int)v5 > 0x9502D35 )
{
if ( v5 == (char *)175938357 )
{
kasfSpeechEventQueueCurrentPlayer(268443887, 0);
v69 = 175938357;
v19 = kasAITeamPtrIfExists(160291907);
goto LABEL_104;
}
if ( v5 == (char *)176183300 )
{
kasTimerSetById(144337492, 3);
kasTimerStartById(144337492);
kasfEntrySetValue(332667, 1);
kasfWideScreenIn(90);
v70 = 134578418;
goto LABEL_50;
}
if ( v5 == (char *)189009464 )
{
kasfSpeechEventQueueCurrentPlayer(268444024, 0);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
}
else
{
if ( v5 == (char *)156249397 )
{
kasfSpeechEventQueueCurrentPlayer(134222673, 0);
v69 = 262155621;
v19 = kasAITeamPtrIfExists(160291907);
goto LABEL_104;
}
if ( v5 == (char *)115050772 )
{
kasfSpeechEventQueueCurrentPlayer(268444029, 0);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
if ( v5 == (char *)119169809 )
{
kasfEntrySetValue(332667, 1);
kasfSpeechEventQueueCurrentPlayer(268443880, 0);
v69 = 119169810;
goto LABEL_103;
}
if ( v5 == (char *)123938021 )
{
kasfSpeechEventQueueCurrentPlayer(268960122, 0);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
}
goto LABEL_31;
}
if ( v5 == (char *)93086467 )
{
kasTimerSetById(144337492, 3);
kasTimerStartById(144337492);
kasfEntrySetValue(332667, 1);
kasfWideScreenIn(90);
v70 = 147206162;
goto LABEL_50;
}
if ( (unsigned int)v5 > 0x588674 )
{
if ( v5 == (char *)32013909 )
{
kasfEntrySetValue(332667, 1);
kasfSpeechEventQueueCurrentPlayer(134222674, 0);
kasfAllowPlayerToResearch("TECH_MISSILE");
v69 = 243787122;
goto LABEL_103;
}
if ( v5 == (char *)48761347 )
{
kasTimerSetById(144337492, 3);
kasTimerStartById(144337492);
kasfEntrySetValue(332667, 1);
kasfWideScreenIn(90);
v7 = kasAITeamPtrIfExists(176850956);
kasfMsgSend((AITeam *)v7, (void *)243310673);
v8 = kasAITeamPtrIfExists(74019713);
kasfMsgSend((AITeam *)v8, (void *)362816);
v9 = kasAITeamPtrIfExists(74019714);
kasfMsgSend((AITeam *)v9, (void *)362816);
v10 = kasAITeamPtrIfExists(74019715);
kasfMsgSend((AITeam *)v10, (void *)362816);
v11 = kasAITeamPtrIfExists(5182113);
kasfMsgSend((AITeam *)v11, (void *)362816);
v12 = kasAITeamPtrIfExists(5182114);
kasfMsgSend((AITeam *)v12, (void *)362816);
v13 = kasAITeamPtrIfExists(5182115);
kasfMsgSend((AITeam *)v13, (void *)362816);
v14 = kasAITeamPtrIfExists(5182116);
kasfMsgSend((AITeam *)v14, (void *)362816);
v15 = kasAITeamPtrIfExists(5182117);
kasfMsgSend((AITeam *)v15, (void *)362816);
v16 = kasAITeamPtrIfExists(5182118);
kasfMsgSend((AITeam *)v16, (void *)362816);
v17 = kasAITeamPtrIfExists(5182119);
kasfMsgSend((AITeam *)v17, (void *)362816);
v70 = 362816;
v18 = kasAITeamPtrIfExists(5182120);
LABEL_51:
kasfMsgSend((AITeam *)v18, (void *)v70);
kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_FleetIntel_ResumeDelay);
return;
}
if ( v5 == (char *)74556978 )
{
kasTimerSetById(144337492, 3);
kasTimerStartById(144337492);
kasfEntrySetValue(332667, 1);
spEventWideScreenIn(90);
v6 = kasAITeamPtrIfExists(18135);
kasfMsgSend((AITeam *)v6, (void *)5801588);
v70 = 119169825;
goto LABEL_50;
}
goto LABEL_31;
}
if ( v5 == (char *)5801588 )
{
kasTimerSetById(144337492, 3);
kasTimerStartById(144337492);
kasfWideScreenOut(90);
v70 = 119169809;
LABEL_50:
v18 = kasAITeamPtrIfExists(176850956);
goto LABEL_51;
}
if ( v5 > (char *)4942178 )
break;
if ( v5 == (char *)4942178 )
{
kasfSpeechEventQueueCurrentPlayer(268444027, 0);
v69 = 32013909;
goto LABEL_103;
}
if ( v5 )
{
if ( v5 == (char *)297300 )
{
kasfEntrySetValue(332667, 1);
kasfSpeechEventQueueCurrentPlayer(268444028, 0);
kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_WaitForIntelEventEnd);
return;
}
LABEL_31:
aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, v5);
}
v2 = kasCurrentTeamPtr;
v3 = *(int **)(kasCurrentTeamPtr + 80);
result = -1;
if ( v3 )
result = *v3;
if ( result >= 0 && --v4 >= 0 )
continue;
return;
}
if ( v5 == (char *)5366435 )
{
kasfSpeechEventQueueCurrentPlayer(268439929, 0);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
goto LABEL_31;
}
v20 = kasCurrentTeamPtr;
v21 = *(int **)(kasCurrentTeamPtr + 80);
result = -1;
if ( v21 )
result = *v21;
v22 = result;
if ( result >= 0 )
{
while ( 1 )
{
v23 = (unsigned int)aitQueuePopPointerByIndex((AITeam *)v20, v22);
if ( v23 > 0x71A6322 )
break;
if ( v23 == 119169826 )
{
AngleDeg = kasfCameraGetAngleDeg();
kasfEntryGetOrCreateSetValue(260833221, AngleDeg);
DeclinationDeg = kasfCameraGetDeclinationDeg();
kasfEntryGetOrCreateSetValue(60936190, DeclinationDeg);
Distance = kasfCameraGetDistance();
kasfEntryGetOrCreateSetValue(64272901, Distance);
v49 = kasGrowSelectionPtr(18135);
kasfTutCameraFocusClose(v49);
kasfSpeechEventQueueCurrentPlayer(268697841, 0);
v69 = 119169827;
goto LABEL_103;
}
if ( v23 > 0x4862BA2 )
{
switch ( v23 )
{
case 0x71A6312u:
kasfSpeechEventQueueCurrentPlayer(268439785, 0);
v69 = 119169811;
goto LABEL_103;
case 0x71A6313u:
kasfSpeechEventQueueCurrentPlayer(268959978, 0);
v69 = 75901858;
goto LABEL_103;
case 0x71A6321u:
kasfOtherKASPause();
kasfUniversePause();
kasfEntrySetValue(211831571, 1);
kasfEntrySetValue(109086661, 1);
if ( !kasfSensorsIsOpen(0) )
kasfOpenSensors(1);
v44 = kasTeamsVectorPtr(18135);
kasfSensorsSetLookatPoint((int)v44);
kasfSpeechEventQueueCurrentPlayer(268443888, 0);
v45 = kasGrowSelectionPtr(18135);
soundStopDSound("PointerBig", v45);
v69 = 119169826;
LABEL_103:
v19 = kasAITeamPtrIfExists(176850956);
LABEL_104:
kasfMsgSend((AITeam *)v19, (void *)v69);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
goto LABEL_77;
}
if ( v23 == 75901858 )
{
kasfEntrySetValue(332667, 0);
kasfSpeechEventQueueCurrentPlayer(268443883, 0);
v69 = 306724;
v19 = kasAITeamPtrIfExists(160291907);
goto LABEL_104;
}
if ( v23 )
{
if ( v23 == 20061 )
{
kasfEntrySetValue(332667, 0);
kasfSpeechEventQueueCurrentPlayer(268443884, 0);
v30 = kasAITeamPtrIfExists(160291907);
kasfMsgSend((AITeam *)v30, (void *)5366227);
v31 = kasAITeamPtrIfExists(134850334);
kasfMsgSend((AITeam *)v31, (void *)5801588);
kasfEntrySetValue(211831571, 1);
v32 = kasAITeamPtrIfExists(74019713);
kasfMsgSend((AITeam *)v32, (void *)1215);
v33 = kasAITeamPtrIfExists(74019714);
kasfMsgSend((AITeam *)v33, (void *)1215);
v34 = kasAITeamPtrIfExists(74019715);
kasfMsgSend((AITeam *)v34, (void *)1215);
v35 = kasAITeamPtrIfExists(5182113);
kasfMsgSend((AITeam *)v35, (void *)1215);
v36 = kasAITeamPtrIfExists(5182114);
kasfMsgSend((AITeam *)v36, (void *)1215);
v37 = kasAITeamPtrIfExists(5182115);
kasfMsgSend((AITeam *)v37, (void *)1215);
v38 = kasAITeamPtrIfExists(5182116);
kasfMsgSend((AITeam *)v38, (void *)1215);
v39 = kasAITeamPtrIfExists(5182117);
kasfMsgSend((AITeam *)v39, (void *)1215);
v40 = kasAITeamPtrIfExists(5182118);
kasfMsgSend((AITeam *)v40, (void *)1215);
v41 = kasAITeamPtrIfExists(5182119);
kasfMsgSend((AITeam *)v41, (void *)1215);
v42 = kasAITeamPtrIfExists(5182120);
kasfMsgSend((AITeam *)v42, (void *)1215);
v43 = kasAITeamShipsPtrIfExists(134850334);
kasfShipsAttributesBitSet((GrowSelection *)v43, SOF_AttachVelocity);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
if ( v23 == 308885 )
{
kasTimerSetById(144337492, 3);
kasfEntrySetValue(127756738, 1);
kasfEntrySetValue(332667, 0);
kasfEntrySetValue(266064926, 1);
kasfEntrySetValue(172907219, 1);
kasfEntrySetValue(52577235, 1);
kasfOtherKASPause();
kasfUniversePause();
kasfEntrySetValue(211831571, 1);
kasfEntrySetValue(109086661, 1);
if ( !kasfSensorsIsOpen(0) )
kasfOpenSensors(150000);
v67 = kasGrowSelectionPtr(114961833);
v24 = (int *)kasGrowSelectionPtr(5182051);
kasfShipsSelectType(v24, v67, "P1MissileCorvette");
v25 = kasShipsVectorPtrIfExists(5182051);
kasfSensorsSetLookatPoint((int)v25);
v26 = kasGrowSelectionPtr(5182051);
soundStopDSound("PointerPirates", v26);
kasfSpeechEventQueueCurrentPlayer(268443886, 0);
v27 = kasAITeamPtrIfExists(160291907);
kasfMsgSend((AITeam *)v27, (void *)334437);
v28 = kasAITeamPtrIfExists(134850334);
kasfMsgSend((AITeam *)v28, (void *)5366227);
if ( GetGameDifficulty() >= 2 )
v71 = 32013909;
else
v71 = 4942178;
v29 = kasAITeamPtrIfExists(176850956);
kasfMsgSend((AITeam *)v29, (void *)v71);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
goto LABEL_77;
}
LABEL_78:
v20 = kasCurrentTeamPtr;
v21 = *(int **)(kasCurrentTeamPtr + 80);
result = -1;
if ( v21 )
result = *v21;
if ( result >= 0 && --v22 >= 0 )
continue;
return;
}
if ( v23 > 0x8C63013 )
{
switch ( v23 )
{
case 0xE80A051u:
v62 = kasfCameraGetAngleDeg();
kasfEntryGetOrCreateSetValue(260833221, v62);
v63 = kasfCameraGetDeclinationDeg();
kasfEntryGetOrCreateSetValue(60936190, v63);
v64 = kasfCameraGetDistance();
kasfEntryGetOrCreateSetValue(64272901, v64);
v65 = kasAITeamShipsPtrIfExists(134850334);
kasfTutCameraFocus((int)v65);
v66 = kasAITeamShipsPtrIfExists(134850334);
kasfTutCameraFocus((int)v66);
kasfCameraSetAngleDeg(315);
kasfCameraSetDeclinationDeg(28);
kasfTutCameraSetDistanceIgnoreZoom(1086);
kasfSpeechEventQueueCurrentPlayer(268959995, 0);
v69 = 243310674;
goto LABEL_103;
case 0xE80A052u:
kasTimerSetById(144337492, 10);
kasfEntrySetValue(127756738, 1);
kasfEntrySetValue(266064926, 1);
kasfEntrySetValue(232774915, 1);
kasfSpeechEventQueueCurrentPlayer(268444030, 0);
v69 = 20061;
goto LABEL_103;
case 0xE87E572u:
kasfEntrySetValue(332667, 0);
kasfSpeechEventQueueCurrentPlayer(134226771, 0);
kasfEnableShip("sMothershipWeapons", "Race_Sect", 1);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
}
}
else
{
switch ( v23 )
{
case 0x8C63013u:
kasfSpeechEventQueueCurrentPlayer(268960444, 0);
v69 = 308885;
goto LABEL_103;
case 0x71A6323u:
kasfEntrySetValue(172907219, 1);
kasTimerSetById(144337492, 3);
kasfEntrySetValue(127756738, 1);
kasfEntrySetValue(332667, 0);
kasfEntrySetValue(266064926, 1);
kasfSpeechEventQueueCurrentPlayer(268439794, 0);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
case 0x80580F2u:
v54 = kasfCameraGetAngleDeg();
kasfEntryGetOrCreateSetValue(260833221, v54);
v55 = kasfCameraGetDeclinationDeg();
kasfEntryGetOrCreateSetValue(60936190, v55);
v56 = kasfCameraGetDistance();
kasfEntryGetOrCreateSetValue(64272901, v56);
kasTimerSetById(144337492, 3);
kasfEntrySetValue(127756738, 1);
kasfEntrySetValue(266064926, 1);
kasfEntrySetValue(172907219, 1);
kasfEntrySetValue(332667, 0);
v72 = kasGrowSelectionPtr(5366435);
v57 = kasVolumePtr(132933767);
singlePlayerFindMissilesInside((Volume *)v57, (GrowSelection *)v72);
v73 = kasShipsVectorPtrIfExists(218060896);
v58 = (void *)kasGrowSelectionPtr(5366435);
singlePlayerSortShipsByDistanceToPoint((GrowSelection *)v58, (hvector *)v73);
v68 = kasGrowSelectionPtr(5366435);
v59 = (int *)kasGrowSelectionPtr(21541);
kasfShipsSelectIndex(v59, v68, 0);
v60 = kasGrowSelectionPtr(21541);
kasfTutCameraFocusClose(v60);
v61 = (signed int *)kasGrowSelectionPtr(21541);
kasfPingAddShips(v61, "PingOnMine");
kasfSpeechEventQueueCurrentPlayer(268444023, 0);
kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission02_FleetIntel_CleanupAndResume);
return;
case 0x8C63012u:
v50 = kasfCameraGetAngleDeg();
kasfEntryGetOrCreateSetValue(260833221, v50);
v51 = kasfCameraGetDeclinationDeg();
kasfEntryGetOrCreateSetValue(60936190, v51);
v52 = kasfCameraGetDistance();
kasfEntryGetOrCreateSetValue(64272901, v52);
spStateA5B1AC = 1;
v53 = kasAITeamShipsPtrIfExists(134850334);
kasfTutCameraFocusClose((int)v53);
kasfCameraSetAngleDeg(329);
kasfCameraSetDeclinationDeg(-12);
kasfTutCameraSetDistanceIgnoreZoom(339);
kasfSpeechEventQueueCurrentPlayer(268959981, 0);
v69 = 147206163;
goto LABEL_103;
}
}
LABEL_77:
aitQueuePointer((AITeam *)kasCurrentTeamPtr, (AITeam *)kasCurrentTeamPtr, (void *)v23);
goto LABEL_78;
}
return;
}

void Init_Mission02_FleetIntel_Objectives(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    void *message;
    GrowSelection *selection;
    GrowSelection *dockSelection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = -1;
    if (queueHead != NULL)
    {
        queueIndex = *queueHead;
    }
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message == (void *)306724)
        {
            kasfObjectiveCreate("Find", ((char **)0x8C8BA4)[strCurLanguage], ((char **)0x8C8BB0)[strCurLanguage]);
        }
        else if (message == (void *)334437)
        {
            kasfObjectiveDestroy("Mimic");
            kasfObjectiveCreate("Lure", ((char **)0x8C8BEC)[strCurLanguage], ((char **)0x8C8BF8)[strCurLanguage]);
        }
        else if (message == (void *)5351060)
        {
            kasfObjectiveDestroy("Lure");
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (void *)118858197);
        }
        else if (message == (void *)5366227)
        {
            kasfObjectiveDestroy("Find");
            kasfObjectiveCreate("Mimic", ((char **)0x8C8BBC)[strCurLanguage], ((char **)0x8C8BC8)[strCurLanguage]);
        }
        else if (message == (void *)262155621)
        {
            kasfObjectiveCreate("BuildHangarMod", ((char **)0x8C8B80)[strCurLanguage], ((char **)0x8C8B8C)[strCurLanguage]);
        }
        else if (message == (void *)55426788)
        {
            kasfMissionFailed();
        }
        else if (message == (void *)175938357)
        {
            kasfEntryGetOrCreateSetValue(-2111991596, 1);
            selection = kasGrowSelectionPtr(4860457);
            dockSelection = kasGrowSelectionPtr(130693705);
            if (kasfShipsSelectType(dockSelection, selection, "sMothershipDockingBay"))
            {
                kasfObjectiveDestroyAll();
                kasfObjectiveCreate("Hyperspace", ((char **)0x8C8C04)[strCurLanguage], ((char **)0x8C8C10)[strCurLanguage]);
                spSetSinglePlayerSaveInfo(3, ((char **)0x8C8B74)[strCurLanguage]);
            }
        }
        else if (message == (void *)262155637)
        {
            kasfObjectiveDestroy("BuildHangarMod");
        }
        else if (message != NULL)
        {
            aitQueuePointer(team, team, message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = -1;
        if (queueHead != NULL)
        {
            queueIndex = *queueHead;
        }
        if (queueIndex >= 0)
        {
            queueIndex--;
        }
    }
}

void Init_Mission02_FleetIntel_FIDefendSalvageTeam2(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfProtect(kasAITeamShipsPtrIfExists(142188961));
}

void Watch_Mission02_FleetIntel_FIDefendSalvageTeam2(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)17444564))
    {
        kasfVarDestroy(kasCurrentTeamPtr + 28);
       kasfSpeechEvent(139967671, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
   }
}

void Init_Mission02_ACT3Corvettes1_Launch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_Mission02_ACT3Corvettes1_Launch,
            (sdword)Watch_Mission13_MissionFlow_FILost);
    }
}

void Begin_Mission02_ACT3Corvettes1_Launch(void)
{
   AITeam *team = (AITeam *)kasCurrentTeamPtr;
   GrowSelection *shipsToCreate;
   GrowSelection *launchSource;
   char *shipType;
   sdword shipCount;

   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfStop();
   shipsToCreate = kasGrowSelectionPtr((char *)21159);
   kasfGrowSelectionClear(shipsToCreate);

   switch (kasfRandom(1, 3))
   {
       case 1:
           shipType = "bHeavyCorvette";
           shipCount = kasfRandom(2, 3);
           break;
       case 2:
           shipType = "bAttackBomber";
           shipCount = kasfRandom(2, 3);
           break;
       default:
           shipType = "bAcolyte";
           shipCount = kasfRandom(3, 5);
           break;
   }

   launchSource = kasAITeamShipsPtrIfExists((char *)108436545);
   kasfShipsCreateInside(shipsToCreate, shipType, shipCount, (sdword)launchSource, 1);
   kasfReinforceTeamWithShips(team, shipsToCreate);
   kasfShipsCreateInside(shipsToCreate, "bAcolyte", kasfRandom(1, 2), (sdword)launchSource, 1);
   kasfReinforceTeamWithShips(team, shipsToCreate);
}

void Watch_Mission02_FleetIntel_FIInvestigateP1Mothership(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfTeamFinishedLaunching())
        kasfSpeechEvent(74024315, (sdword)Init_FleetIntel_SelectInvestigationTarget, (sdword)Watch_FleetIntel_InvestigateTarget);
}

void Watch_Mission02_FleetIntel_FISeenP1Mothership(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfShipsAnyIdle(&teamtoreinforce->shipList))
        kasfSpeechEvent(123839973, (sdword)Watch_BeastWorkerRepair_SpecialAttackDamagedCapital, (sdword)Watch_Mission02_FleetIntel_FISeenP1Mothership);
}

// Halt the current team's ships and clear their per-ship state fields (offsets 25-30 and 47-52).
static void HaltTeamAndClearShipState(void)
{
    SelectCommand *sel = teamtoreinforce->shipList.selection;
    sdword i;
    udword *shipDwords;

    aitDeleteAllTeamMoves(teamtoreinforce);
    clWrapHalt(&universe.mainCommandLayer, sel);

    for (i = 0; i < sel->numShips; i++)
    {
        shipDwords = (udword *)sel->ShipPtr[i];
        shipDwords[25] = 0;
        shipDwords[26] = 0;
        shipDwords[27] = 0;
        shipDwords[28] = 0;
        shipDwords[29] = 0;
        shipDwords[30] = 0;
        shipDwords[47] = 0;
        shipDwords[48] = 0;
        shipDwords[49] = 0;
        shipDwords[50] = 0;
        shipDwords[51] = 0;
        shipDwords[52] = 0;
    }
}

void Init_Mission02_DefendSalvageTeam2Approach(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
    kasfShipsSetNonRetaliation(&teamtoreinforce->shipList);
    kasfShipsSetSlaveableFlag(&teamtoreinforce->shipList, 0);
    kasTimerCreateById(139352562);
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)Watch_Mission02_DefendSalvageTeam2ApproachSignal);
}

void Watch_Mission02_DefendSalvageTeam2ApproachSignal(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfMsgReceived((char *)0xFE5FA04))
        kasfSpeechEvent(86289959, (sdword)Init_Mission02_DefendSalvageTeam2MarkTarget, (sdword)Watch_Mission02_DefendSalvageTeam2Approach);
}

void Init_Mission02_DefendSalvageTeam2MarkTarget(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    kasfMoveTo(kasTeamsVectorPtr(93748889));
}

void Watch_Mission02_DefendSalvageTeam2Approach(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfTeamHealthAverage() < 70)
        kasfTeamSetPercentDamaged(70);
    if (!kasfEntryValueGet(71552169))
    {
        if ((sdword)kasfFindDistance((float *)kasShipsVectorPtr(NULL), (float *)kasTeamsVectorPtr(93748889)) < 38000)
        {
            kasfEntryGetOrCreateSetValue(71552169, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)258321701);
        }
    }
    if ((sdword)kasfFindDistance((float *)kasShipsVectorPtr(NULL), (float *)kasTeamsVectorPtr(93748889)) < 6000)
        kasfSpeechEvent(177783815, (sdword)Init_Mission02_DefendSalvageTeam2Intercept, (sdword)Watch_Mission02_DefendSalvageTeam2Intercept);
}

void Init_Mission02_DefendSalvageTeam2Intercept(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    HaltTeamAndClearShipState();
    tutDeallocateRootRegion((sdword)kasAITeamShipsPtrIfExists(93748889));
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)189488980);
}

void Watch_Mission02_DefendSalvageTeam2Intercept(void)
{
    GrowSelection *target;

    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfTeamHealthAverage() < 70)
        kasfTeamSetPercentDamaged(70);
    if (kasfMsgReceived((char *)0xD72A9BB))
    {
        kasfShipsSetNonRetaliation(&teamtoreinforce->shipList);
        kasfTacticsNeutral();
        kasfStop();
        target = (GrowSelection *)kasAITeamShipsPtrIfExists(93748889);
        kasfTeamSpecial(target);
    }
    if (kasfMsgReceived((char *)0x10A2ED4))
    {
        kasfTeamSetPercentDamaged(20);
        kasfShipsSetPercentFueled(&teamtoreinforce->shipList, 0.0f);
        kasfShipsSetRetaliation(&teamtoreinforce->shipList);
        kasfSpeechEvent(70903842, (sdword)Init_Mission02_DefendSalvageTeam2ResumeCapture, (sdword)Watch_Mission02_DefendSalvageTeam2ResumeCapture);
    }
    else if (kasfShipsAnyIdle(&teamtoreinforce->shipList))
    {
        tutDeallocateRootRegion((sdword)kasAITeamShipsPtrIfExists(93748889));
    }
}

void Init_Mission02_DefendSalvageTeam2ResumeCapture(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    kasfTacticsAggressive();
    kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 0);
    kasfShipsSetSlaveableFlag(&teamtoreinforce->shipList, 1);
    kasfMoveTo(kasShipsVectorPtrIfExists(211966048));
}

void Watch_Mission02_DefendSalvageTeam2ResumeCapture(void)
{
    kasCurrentStep = 2;
    scope = teamtoreinforce->kasLabelHash;
    if (kasfShipsAnyIdle(&teamtoreinforce->shipList))
        kasfMoveTo(kasShipsVectorPtrIfExists(211966048));
    if ((sdword)kasfFindDistance((float *)kasShipsVectorPtr(NULL), (float *)kasShipsVectorPtrIfExists(211966048)) < 10000)
    {
        kasfShipsSetRetaliation(&teamtoreinforce->shipList);
        tutDeallocateRootRegion((sdword)kasGrowSelectionPtr((sdword)(char *)0x4A2A29));
    }
}
