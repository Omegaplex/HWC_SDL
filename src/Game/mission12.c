/*=============================================================================
    Name    : mission12.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission12
=============================================================================*/

#include "MissionScriptSupport.h"

void Watch_Mission12_GuardGravWells_AttackMothership(void);
void Init_Mission12_GuardGravWells_AttackMothership(void);
void Init_Mission12_MissionFlow(void);
void Watch_Mission12_MissionFlow_WaitForIntelEventEnd(void);
void Watch_Mission12_MissionFlow_Delay(void);
void Watch_Mission12_MissionFlow(void);
void Watch_Mission12_ObjectiveFlow(void);
void Watch_Mission12_FleetIntel_FIMissileTechReady(void);
void Init_Mission12_FleetIntel_FIStartResearching(void);
void Watch_Mission12_FleetIntel_FIStartResearching(void);
void Init_Mission12_FormationReleaseCoordinator(void);
void Watch_Mission12_FormationReleaseCoordinator(void);
void Begin_Mission12_FormationReleaseAbort(void);
void Watch_Mission12_FormationReleaseAbort(void);
void Init_Mission12_WorkerDockController(void);
void Begin_Mission12_WorkerDock(void);
void Watch_Mission12_WorkerDockController(void);
void Init_Mission12_A1LaunchController(void);
void Begin_Mission12_A1ProcessModuleSlots(void);
void Init_Mission12_ModuleOrderPatrolController(void);
void Watch_Mission12_ModuleOrderPatrolController(void);
void Watch_Mission12_ModuleOrderPatrolCloseRange(void);
void Init_Mission12_P1FighterReinforcementTeam(void);
void Init_Mission12_P1FighterReinforcementReset(void);
void Watch_Mission12_P1FighterReinforcementTeam(void);
void Watch_Mission12_P1FighterReinforcementReset(void);
void Init_Mission12_P1FighterReinforcementDelay(void);
void Watch_Mission12_P1FighterReinforcementDelay(void);
void Init_Mission12_SpawnP1FighterReinforcement(void);
void Watch_Mission12_P1FighterReinforcementCycle(void);
void Init_Mission12_P1FighterOrMissileCorvetteReinforcementTeam(void);
void Init_Mission12_P1FighterOrMissileCorvetteReinforcementReset(void);
void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementTeam(void);
void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementDelay(void);
void Init_Mission12_SpawnP1FighterOrMissileCorvetteReinforcement(void);
void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementCycle(void);
void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementReset(void);
void Init_Mission12_P1StandardCorvetteReinforcementTeam(void);
void Watch_Mission12_DeleteTeamOnAbortSignal(void);
void Watch_Mission12_WaitForNearbyTargetsForSpecialAttack(void);
void Init_Mission12_QueueP1FighterLaunchSlots(void);
void Watch_Mission12_QueueP1FighterLaunchSlots(void);
void Begin_Mission12_SpecialAttackNearestCapitalTarget(void);
void Watch_Mission12_P1StandardCorvetteReinforcementTeam(void);
void Init_Mission12_P1StandardCorvetteReinforcementDelay(void);
void Watch_Mission12_P1StandardCorvetteReinforcementDelay(void);
void Init_Mission12_SpawnP1StandardCorvetteReinforcement(void);
void Watch_Mission12_P1StandardCorvetteReinforcementCycle(void);
void Init_Mission12_SpecialAttackTeam(void);
void Watch_Mission12_SpecialAttackTeam(void);
void Watch_Mission12_MoveSpecialAttackTargetIntoRange(void);
void Select_Mission12_CapitalTargetsExcludingMothershipAndCarrier(void);
void Begin_Mission12_SixShipFormationTeam_AttackMothership(void);
void Init_Mission12_SixShipFormationTeam(void);
void Watch_Mission12_SixShipFormationTeam_AwaitPositionOrder(void);
void Init_Mission12_SixShipFormationTeam_MoveToAssignedPosition(void);
void Watch_Mission12_SixShipFormationTeam_AwaitAttackOrder(void);
void Init_Mission12_SixShipFormationTeam_AttackMothership(void);
void Init_Mission12_IdleResetController(void);
void Watch_Mission12_AuxTeamDeleteOnAbortSignal(void);
void Begin_Mission12_IdleReset(void);
void Watch_Mission12_IdleResetController(void);
void Watch_Mission12_StrikeTeamDispatch(void);
void Watch_Mission12_StrikeTeamDispatchRetreat(void);
void Begin_Mission12_StrikeTeamDispatchRetreat(void);
void Watch_Mission12_StrikeTeamDispatchRetreat_DamageClamp(void);
void Init_Mission12_AssaultMothershipTeam(void);
void Watch_Mission12_AssaultMothershipTeam(void);
void Init_Mission12_FrigateObjective_Start(void);
void Watch_Mission12_FrigateObjective_AttackMothershipCue(void);
void Init_Mission12_FrigateObjective_FocusTrackedTarget(void);
void Watch_Mission12_FrigateObjective_WaitForNearbyFrigate(void);
void Init_Mission12_FrigateObjective_ClearTrackedSelection(void);
void Watch_Mission12_FrigateObjective_RetargetWhenNoNearbyFrigate(void);
void Watch_Mission11_Traders(void);
void Watch_Mission11_TraderTargetLoop(void);
void Watch_Mission05_FrigateReinforcementCombatLoop(void);

void Watch_Mission12(void)
{
  int v0; // eax
  int *v1; // eax
  int *v2; // eax
  int v3; // eax
  int *v4; // eax
  int *v5; // eax
  int *v7; // eax
  int *v8; // eax
  int v9; // eax
  int *v10; // eax
  int v11; // eax
  int v12; // eax
  int v13; // [esp-8h] [ebp-8h]
  int v14; // [esp-8h] [ebp-8h]
  int v15; // [esp-8h] [ebp-8h]
  int v16; // [esp-4h] [ebp-4h]
  int v17; // [esp-4h] [ebp-4h]
  int v18; // [esp-4h] [ebp-4h]
  int v19; // [esp-4h] [ebp-4h]
  int v20; // [esp-4h] [ebp-4h]
  int v21; // [esp-4h] [ebp-4h]
  int v22; // [esp-4h] [ebp-4h]
  int v23; // [esp-4h] [ebp-4h]
  int *v24; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503618;
  if ( kasTimerExpiredDestroyById(150120898) )
  {
    v16 = kasGrowSelectionPtr(18336307);
    v0 = kasVolumePtr(132933767);
    kasfFindShipsInside(v0, v16);
    v17 = kasGrowSelectionPtr(18336307);
    v1 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v1, v17);
    v18 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v2, v18);
    if ( !kasfEntryValueGet(71552167) )
    {
      v13 = kasGrowSelectionPtr(0x4A2A29);
      v3 = kasGrowSelectionPtr(211966048);
      if ( kasfShipsSelectType(v3, v13, "sMothership") )
        kasfEntrySetValue(71552167, 1);
    }
    v19 = kasGrowSelectionPtr(114961833);
    v4 = (int *)kasGrowSelectionPtr(363040);
    if ( kasfShipsSelectRowGettingOutOfWay(v4, v19) )
    {
      v20 = kasGrowSelectionPtr(363040);
      v5 = kasAITeamPtrIfExists(48288484);
      kasfReinforceTeamWithShips((AITeam *)v5, v20);
    }
    kasTimerSetById(150120898, 10);
    kasTimerStartById(150120898);
  }
  if ( !kasfEntryValueGet(71552168) )
  {
    if ( kasfEntryValueGet(-2015429404) )
    {
      kasfEntrySetValue(71552168, 1);
      v21 = kasGrowSelectionPtr(18336307);
      v7 = (int *)kasGrowSelectionPtr(89157026);
      kasSelectPlayerShipsSimple(v7, v21);
      v22 = kasGrowSelectionPtr(89157026);
      v8 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsSelectNotSalvagers(v8, v22);
      v23 = kasGrowSelectionPtr(218060896);
      v9 = kasGrowSelectionPtr(82017669);
      if ( kasfShipsSelectLinkedShips(v9, v23) )
      {
        v24 = (int *)kasGrowSelectionPtr(82017669);
        v10 = (int *)kasGrowSelectionPtr(89157026);
        kasfShipsAdd(v10, v24);
      }
      if ( GetGameDifficulty() < 2 )
      {
        if ( kasfRUsGet(0) < 500 )
        {
          v14 = kasGrowSelectionPtr(89157026);
          v11 = kasGrowSelectionPtr(96759714);
          if ( !kasfShipsSelectType(v11, v14, "sWorker") )
          {
            v15 = kasGrowSelectionPtr(218060896);
            v12 = kasGrowSelectionPtr(154639956);
            kasfShipsCreateInside(v12, "sWorker", 1, v15, 0);
          }
        }
      }
    }
  }
}

void Watch_Mission12_StrikeTeamDispatch_WaitForACT2(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552173)
     && kasfThisTeamIs(kasAITeamPtrIfExists((char *)90274303))
     && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552173, 1);
        kasfMsgSend(kasAITeamPtrIfExists((char *)176850956), (char *)43324484);
    }
}

void Init_Mission12_StrikeTeamDispatch_WaitForACT2(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 4);
    kasfShipsSetMaxVelocity(&team->shipList, 0);
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}

void Watch_Mission12_A1LaunchProximity(void)
{
    GrowSelection *newShips;
    GrowSelection *originalShips;
    hvector *location;

    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    location = kasShipsVectorPtr((char *)0x1770);
    originalShips = kasGrowSelectionPtr((char *)0x4A2A29);
    newShips = kasGrowSelectionPtr((char *)0x9678C7D);
    if (kasfShipsSelectNearby(newShips, originalShips, location, 0x1770))
    {
        kasfSpeechEvent(139967671, Begin_Mission12_A1DisableForLaunch, Init_Mission10_BigGunObjective_ClearTrackedSelection);
    }
}

void Begin_Mission12_A1DisableForLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfDisableShips((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfFormationDelta3D();
}

void Begin_Mission12_ModuleOrderPatrolDisable(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfDisableShips(&team->shipList);
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x36980EB));
}

void Init_Mission12_A1LaunchController(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(0xE1D7A84, Begin_Mission12_A1ProcessModuleSlots, Watch_Mission12_A1LaunchProximity);
}

void Begin_Mission12_A1ProcessModuleSlots(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsProcessModuleSlotOrder(&team->shipList, (char *)0x8CC1FC, 0);
}

void Init_Mission12_ModuleOrderPatrolController(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfShipsProcessModuleSlotOrder(&team->shipList, (char *)0x88B9E8, 0);
    kasfSpeechEvent(0x7464CB7, MaintainTeamContext, Watch_Mission12_ModuleOrderPatrolController);
}

void Watch_Mission12_ModuleOrderPatrolController(void)
{
    AITeam *team;
    hvector *location;
    GrowSelection *movingShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    location = kasShipsVectorPtr((char *)0x61A8);
    if (kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x5296679),
                              kasGrowSelectionPtr((char *)0x4A2A29),
                              location,
                              0x61A8))
    {
        location = kasShipsVectorPtr((char *)0x1F40);
        if (kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB),
                                  kasGrowSelectionPtr((char *)0x5296679),
                                  location,
                                  0x1F40))
        {
            kasfSpeechEvent(0x857BCB7, Begin_Mission12_ModuleOrderPatrolDisable, Watch_Mission12_ModuleOrderPatrolCloseRange);
            return;
        }

        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x5296679), (vector *)kasThisTeamsVectorPtr());
        kasfShipsSelectFirst(kasGrowSelectionPtr((char *)0x5296679), kasGrowSelectionPtr((char *)0x5296679));
        kasfMoveTo(kasShipsVectorPtrIfExists((char *)0x5296679));
        return;
    }

    movingShips = kasGrowSelectionPtr((char *)0xE99E3C7);
    if (!kasfShipsSelectMoving(movingShips, &team->shipList))
    {
        switch (kasfRandom(0, 3))
        {
            case 0:
                kasfVectorRandomizeInRange(kasVectorPtrIfExists((char *)0xF671B4C), kasVectorPtrIfExists((char *)0x21DF601), 0x2710);
                break;
            case 1:
                kasfVectorRandomizeInRange(kasVectorPtrIfExists((char *)0xF671B4C), kasVectorPtrIfExists((char *)0x21DF602), 0x2710);
                break;
            case 2:
                kasfVectorRandomizeInRange(kasVectorPtrIfExists((char *)0xF671B4C), kasVectorPtrIfExists((char *)0x21DF603), 0x2710);
                break;
            case 3:
                kasfVectorRandomizeInRange(kasVectorPtrIfExists((char *)0xF671B4C), kasVectorPtrIfExists((char *)0x21DF604), 0x2710);
                break;
            default:
                break;
        }

        kasfMoveTo(kasVectorPtrIfExists((char *)0xF671B4C));
    }
}

void Watch_Mission12_ModuleOrderPatrolCloseRange(void)
{
    AITeam *team;
    hvector *location;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    location = kasShipsVectorPtr((char *)0x1F40);
    if (!kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB),
                               kasGrowSelectionPtr((char *)0x4A2A29),
                               location,
                               0x1F40))
    {
        kasfSpeechEvent(0x7464CB7, MaintainTeamContext, Watch_Mission12_ModuleOrderPatrolController);
    }
}

void Init_Mission12_P1FighterReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    if (GetGameDifficulty())
    {
        kasfSpeechEvent(0x857BCB7, Init_Mission12_P1FighterReinforcementReset, Watch_Mission12_P1FighterReinforcementTeam);
    }
    else
    {
        kasfDeleteShips(&team->shipList);
        kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
    }
}

void Init_Mission12_P1FighterReinforcementReset(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x829C5894, 0);
}

void Watch_Mission12_P1FighterReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *playerShips;
    GrowSelection *candidateShips;
    GrowSelection *typeShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x8832365, Init_Mission12_P1FighterReinforcementDelay, Watch_Mission12_P1FighterReinforcementDelay);
        return;
    }
    if (kasfMsgReceived((char *)0x4186921))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(0xD0A5821));
        return;
    }
    if (kasfMsgReceived((char *)0x4186922))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(0xD0A5822));
        return;
    }

    playerShips = kasGrowSelectionPtr((char *)0x4A2A29);
    candidateShips = kasGrowSelectionPtr((char *)0x36980EB);
    typeShips = kasGrowSelectionPtr((char *)0x5920);
    kasfGrowSelectionClear(candidateShips);
    kasfShipsSelectType(typeShips, playerShips, "sRecon");
    kasfShipsAdd(candidateShips, typeShips);
    kasfShipsSelectType(typeShips, playerShips, "sSentinel");
    kasfShipsAdd(candidateShips, typeShips);
    if (!kasfShipsCount(candidateShips))
    {
        kasfGrowSelectionClear(candidateShips);
        kasfShipsSelectType(typeShips, playerShips, "sProcessor");
        kasfShipsAdd(candidateShips, typeShips);
        kasfShipsSelectType(typeShips, playerShips, "sWorker");
        kasfShipsAdd(candidateShips, typeShips);
        if (!kasfShipsCount(candidateShips))
        {
            kasfShipsAdd(candidateShips, playerShips);
        }
    }
    kasfShipsSortByDistanceToPoint(candidateShips, (vector *)kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(candidateShips);
}

void Watch_Mission12_P1FighterReinforcementReset(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfSpeechEvent(0x857BCB7, Init_Mission12_P1FighterReinforcementReset, Watch_Mission12_P1FighterReinforcementTeam);
}

void Init_Mission12_P1FighterReinforcementDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerCreateSetStartById(148949186, 60);
}

void Watch_Mission12_P1FighterReinforcementDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(0x4869685, Init_Mission12_SpawnP1FighterReinforcement, Watch_Mission12_P1FighterReinforcementCycle);
    }
}

void Init_Mission12_SpawnP1FighterReinforcement(void)
{
    AITeam *team;
    GrowSelection *reinforcementShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    reinforcementShips = kasGrowSelectionPtr((char *)0x52A7);
    kasfGrowSelectionClear(reinforcementShips);
    kasfShipsCreateInside(reinforcementShips, "p1Fighter", kasfTeamCountOriginal(), kasAITeamShipsPtrIfExists(0xCF25860), 1);
    kasAITeamShipsPtr(reinforcementShips, 5);
    kasfReinforceTeamWithShips(team, reinforcementShips);
    kasfEntryIncrement(0x829C5894);
}

void Watch_Mission12_P1FighterReinforcementCycle(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(0xCF25860)))
    {
        kasfTacticsAggressive();
        kasfFormationDelta();
        if (kasfEntryValueGet(0x829C5894) == 3
         || kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x58A20),
                                  kasGrowSelectionPtr((char *)0x4A2A29),
                                  kasShipsVectorPtr((char *)0x2710),
                                  0x2710))
        {
            kasfSpeechEvent(0x86C309B, MaintainTeamContext, Watch_Mission12_P1FighterReinforcementReset);
        }
    }
}

void Init_Mission12_P1FighterOrMissileCorvetteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfSpeechEvent(0x857BCB7, Init_Mission12_P1FighterOrMissileCorvetteReinforcementReset, Watch_Mission12_P1FighterOrMissileCorvetteReinforcementTeam);
}

void Init_Mission12_P1FighterOrMissileCorvetteReinforcementReset(void)
{
    AITeam *team;
    GrowSelection *candidateShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x815404D4, 0);
    candidateShips = kasGrowSelectionPtr((char *)0x36980EB);
    kasfGrowSelectionClear(candidateShips);
    kasfShipsAdd(candidateShips, kasGrowSelectionPtr((char *)0x4A2A29));
    kasfShipsSortByDistanceToPoint(candidateShips, (vector *)kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(candidateShips);
}

void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *candidateShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x8832365, Init_Mission12_P1FighterReinforcementDelay, Watch_Mission12_P1FighterOrMissileCorvetteReinforcementDelay);
        return;
    }
    if (!kasfShipsAnyIdle(&team->shipList))
    {
        return;
    }

    candidateShips = kasGrowSelectionPtr((char *)0x36980EB);
    kasfGrowSelectionClear(candidateShips);
    kasfShipsAdd(candidateShips, kasGrowSelectionPtr((char *)0x4A2A29));
    kasfShipsSortByDistanceToPoint(candidateShips, (vector *)kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(candidateShips);
}

void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(0x4869685, Init_Mission12_SpawnP1FighterOrMissileCorvetteReinforcement, Watch_Mission12_P1FighterOrMissileCorvetteReinforcementCycle);
    }
}

void Init_Mission12_SpawnP1FighterOrMissileCorvetteReinforcement(void)
{
    AITeam *team;
    GrowSelection *reinforcementShips;
    char *shipType;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    reinforcementShips = kasGrowSelectionPtr((char *)0x52A7);
    kasfGrowSelectionClear(reinforcementShips);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x5184721)))
    {
        shipType = "p1MissileCorvette";
    }
    else
    {
        shipType = "p1Fighter";
    }
    kasfShipsCreateInside(reinforcementShips, shipType, kasfTeamCountOriginal(), kasAITeamShipsPtrIfExists(0xCF25860), 1);
    kasAITeamShipsPtr(reinforcementShips, 5);
    kasfReinforceTeamWithShips(team, reinforcementShips);
    kasfEntryIncrement(0x815404D4);
}

void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementCycle(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfTacticsAggressive();
        kasfFormationDelta();
        if (kasfEntryValueGet(0x815404D4) == 3
         || kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x58A20),
                                  kasGrowSelectionPtr((char *)0x4A2A29),
                                  kasShipsVectorPtr((char *)0x2710),
                                  0x2710))
        {
            kasfSpeechEvent(0x86C309B, MaintainTeamContext, Watch_Mission12_P1FighterOrMissileCorvetteReinforcementReset);
        }
    }
}

void Watch_Mission12_P1FighterOrMissileCorvetteReinforcementReset(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfSpeechEvent(0x857BCB7, Init_Mission12_P1FighterOrMissileCorvetteReinforcementReset, Watch_Mission12_P1FighterOrMissileCorvetteReinforcementTeam);
}

void Init_Mission12_P1StandardCorvetteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (GetGameDifficulty())
    {
        kasAITeamShipsPtr(&team->shipList, 5);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfSpeechEvent(0x7B65285, MaintainTeamContext, Watch_Mission12_WaitForNearbyTargetsForSpecialAttack);
    }
    else
    {
        kasfDeleteShips(&team->shipList);
        kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
    }
}

void Watch_Mission12_DeleteTeamOnAbortSignal(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x76A7002))
    {
        kasfDeleteShips(&team->shipList);
        kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
    }
}

void Watch_Mission12_WaitForNearbyTargetsForSpecialAttack(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x53A0C),
                              kasGrowSelectionPtr((char *)0x4A2A29),
                              kasShipsVectorPtr((char *)0xEA60),
                              0xEA60))
    {
        kasfSpeechEvent(0x1A5EB47, Begin_Mission12_SpecialAttackNearestCapitalTarget, Watch_Mission12_P1StandardCorvetteReinforcementTeam);
    }
}

void Init_Mission12_QueueP1FighterLaunchSlots(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTacticsAggressive();
    kasfShipsProcessModuleSlotOrder(&team->shipList, "p1Fighter", 5);
    kasfEntryGetOrCreateSetValue(0x1E1D86E, 1);
}

void Watch_Mission12_QueueP1FighterLaunchSlots(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;
    GrowSelection *nearbyTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x8832365, Init_Mission12_P1StandardCorvetteReinforcementDelay, Watch_Mission12_P1StandardCorvetteReinforcementDelay);
        return;
    }

    capitalTargets = kasGrowSelectionPtr((char *)0x761FCF5);
    nearbyTargets = kasGrowSelectionPtr((char *)0x43811E5);
    kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr((char *)0x4A2A29));
    kasfShipsSelectNearby(nearbyTargets, capitalTargets, kasTeamsVectorPtr(0xCF25860), 0x61A8);
    kasfShipsRemove(capitalTargets, nearbyTargets);
    if (kasfShipsCount(capitalTargets))
    {
        kasfSpeechEvent(0x1A5EB47, Begin_Mission12_SpecialAttackNearestCapitalTarget, Watch_Mission12_P1StandardCorvetteReinforcementTeam);
    }
    else
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x4A2A29));
    }
}

void Begin_Mission12_SpecialAttackNearestCapitalTarget(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;
    GrowSelection *nearbyTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTacticsNeutral();
    capitalTargets = kasGrowSelectionPtr((char *)0x761FCF5);
    nearbyTargets = kasGrowSelectionPtr((char *)0x43811E5);
    kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr((char *)0x4A2A29));
    kasfShipsSelectNearby(nearbyTargets, capitalTargets, kasTeamsVectorPtr(0xCF25860), 0x61A8);
    kasfShipsRemove(capitalTargets, nearbyTargets);
    kasfShipsSortByDistanceToPoint(capitalTargets, (vector *)kasThisTeamsVectorPtr());
    kasfAttackSpecial(capitalTargets);
}

void Watch_Mission12_P1StandardCorvetteReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(0x8832365, Init_Mission12_P1StandardCorvetteReinforcementDelay, Watch_Mission12_P1StandardCorvetteReinforcementDelay);
        return;
    }
    if (kasfEntryValueGet(0x1E1D86E))
    {
        capitalTargets = kasGrowSelectionPtr((char *)0x5296679);
        if (kasfShipsSelectNearby(capitalTargets,
                                  kasGrowSelectionPtr((char *)0x4A2A29),
                                  kasShipsVectorPtr((char *)0xFA0),
                                  0xFA0))
        {
            if (kasfShipsSelectCapital(capitalTargets, capitalTargets))
            {
                kasfDisableShips(&team->shipList);
                kasfEntrySetValue(0x1E1D86E, 0);
            }
        }
    }
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(0x71AFB25, Init_Mission12_QueueP1FighterLaunchSlots, Watch_Mission12_QueueP1FighterLaunchSlots);
    }
}

void Init_Mission12_P1StandardCorvetteReinforcementDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerCreateSetStartById(148949186, 180);
}

void Watch_Mission12_P1StandardCorvetteReinforcementDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(0x4869685, Init_Mission12_SpawnP1StandardCorvetteReinforcement, Watch_Mission12_P1StandardCorvetteReinforcementCycle);
    }
}

void Init_Mission12_SpawnP1StandardCorvetteReinforcement(void)
{
    AITeam *team;
    GrowSelection *reinforcementShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    reinforcementShips = kasGrowSelectionPtr((char *)0x52A7);
    kasfGrowSelectionClear(reinforcementShips);
    kasfShipsCreateInside(reinforcementShips, "p1StandardCorvette", kasfTeamCountOriginal(), kasAITeamShipsPtrIfExists(0xCF25860), 1);
    kasAITeamShipsPtr(reinforcementShips, 5);
    kasfReinforceTeamWithShips(team, reinforcementShips);
}

void Watch_Mission12_P1StandardCorvetteReinforcementCycle(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(0xCF25860)))
    {
        kasfSpeechEvent(0x71AFB25, Init_Mission12_QueueP1FighterLaunchSlots, Watch_Mission12_QueueP1FighterLaunchSlots);
    }
}

void Init_Mission12_SpecialAttackTeam(void)
{
    AITeam *team;
    sdword difficulty;
    sdword limit;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    difficulty = GetGameDifficulty();
    if (difficulty == 0)
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(0xD0A5822)))
        {
            kasfDeleteShips(&team->shipList);
            kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
            return;
        }
        limit = 3;
    }
    else if (difficulty == 1)
    {
        limit = 2;
    }
    else if (difficulty == 2)
    {
        limit = 1;
    }
    else
    {
        limit = -1;
    }

    if (limit >= 0)
    {
        kasfShipsSelectFirstN(kasGrowSelectionPtr((char *)0x38A0F15), &team->shipList, limit);
        kasfVarDestroy((char *)kasGrowSelectionPtr((char *)0x38A0F15));
    }

    kasfTeamSpecialToggle();
    kasfEntryGetOrCreateSetValue(135486452, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0xD0A5821)))
    {
        kasfSpeechEvent(0x9DBF1E0, MaintainTeamContext, Watch_Mission12_MoveSpecialAttackTargetIntoRange);
    }
    else
    {
        kasfSpeechEvent(0xE98CF15, MaintainTeamContext, Select_Mission12_CapitalTargetsExcludingMothershipAndCarrier);
    }
}

void Watch_Mission12_SpecialAttackTeam(void)
{
    AITeam *team;
    sdword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0xA7C9B35))
    {
        kasfAttackMothership();
        kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
    }
    else if (!kasfEntryValueGet(71552166) && kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)))
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        if (kasfThisTeamIs(kasAITeamPtrIfExists(0xD0A5821)))
        {
            message = 68708641;
        }
        else
        {
            message = 68708642;
        }
        kasfMsgSend(kasAITeamPtrIfExists(0xACA8E41), (char *)message);
        kasfMsgSend(kasAITeamPtrIfExists(0xACA8E42), (char *)message);
        kasfMsgSend(kasAITeamPtrIfExists(0xACA8E43), (char *)message);
    }
}

void Watch_Mission12_MoveSpecialAttackTargetIntoRange(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(135486452))
    {
        if (kasfFindDistance(kasThisTeamsVectorPtr(), kasShipsVectorPtrIfExists(0xCA25860)) < 0x1964)
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(135486452, 0);
            tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0xCA25860));
        }
        else
        {
            kasfMoveTo(kasShipsVectorPtrIfExists(0xCA25860));
        }
    }
}

void Select_Mission12_CapitalTargetsExcludingMothershipAndCarrier(void)
{
    AITeam *team;
    GrowSelection *nearbyShips;
    GrowSelection *capitalTargets;
    GrowSelection *preferredTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsAnyIdle(&team->shipList))
    {
        return;
    }
    if (!kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x5296679),
                               kasGrowSelectionPtr((char *)0x4A2A29),
                               kasShipsVectorPtr((char *)0x3A98),
                               0x3A98))
    {
        return;
    }

    capitalTargets = kasGrowSelectionPtr((char *)0x764E81C);
    if (!kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr((char *)0x5296679)))
    {
        return;
    }
    if (kasfEntryValueGet(135486452))
    {
        kasfTeamSpecialToggle();
        kasfEntrySetValue(135486452, 0);
    }

    kasfShipsSortByDistanceToPoint(capitalTargets, (vector *)kasThisTeamsVectorPtr());
    preferredTargets = kasGrowSelectionPtr((char *)0x36980EB);
    if (!kasfShipsSelectType(preferredTargets, capitalTargets, "sMothership")
     && !kasfShipsSelectType(preferredTargets, capitalTargets, "sCarrier"))
    {
        kasfShipsAdd(preferredTargets, capitalTargets);
    }
    nearbyShips = preferredTargets;
    tutDeallocateRootRegion(nearbyShips);
}

void Init_Mission12_SixShipFormationTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfSpeechEvent(0x4E2D3, Begin_Mission12_SixShipFormationTeam_AttackMothership, Watch_Mission12_SixShipFormationTeam_AwaitPositionOrder);
}

void Watch_Mission12_SixShipFormationTeam_AwaitPositionOrder(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x9B3141E))
    {
        kasfSpeechEvent(0x4A93A83, Init_Mission12_SixShipFormationTeam_MoveToAssignedPosition, Watch_Mission12_SixShipFormationTeam_AwaitAttackOrder);
    }
}

void Init_Mission12_SixShipFormationTeam_MoveToAssignedPosition(void)
{
    static const sdword teamLabels[] = {
        0x8B09A01,
        0x8B09A02,
        0x8B09A03,
        0x8B09A04,
        0x8B09A05,
        0x8B09A06,
    };
    static const sdword vectorLabels[] = {
        0x1AA5F91,
        0x1AA5F92,
        0x1AA5F93,
        0x1AA5F94,
        0x1AA5F95,
        0x1AA5F96,
    };
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    for (i = 0; i < sizeof(teamLabels) / sizeof(teamLabels[0]); i++)
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(teamLabels[i])))
        {
            kasfTeamHyperspaceIn(kasVectorPtrIfExists(vectorLabels[i]));
            return;
        }
    }
}

void Watch_Mission12_SixShipFormationTeam_AwaitAttackOrder(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0xB315834))
    {
        kasfSpeechEvent(0x4C425, Init_Mission12_SixShipFormationTeam_AttackMothership, MaintainTeamContext);
    }
}

void Init_Mission12_SixShipFormationTeam_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfAttackMothership();
}

void Init_Mission12_IdleResetController(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfSpeechEvent(0x469857B, Begin_Mission12_IdleReset, Watch_Mission12_IdleResetController);
}

void Watch_Mission12_AuxTeamDeleteOnAbortSignal(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x76A7002))
    {
        kasfDeleteShips(&team->shipList);
        kasfSpeechEvent(342540, sub_65F6F0, MaintainTeamContext);
    }
}

void Begin_Mission12_IdleReset(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfTacticsAggressive();
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x4A2A29));
}

void Watch_Mission12_IdleResetController(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(0x469857B, Begin_Mission12_IdleReset, Watch_Mission12_IdleResetController);
    }
}

void Watch_Mission12_StrikeTeamDispatch(void)
{
    AITeam *team;
    hvector *destination;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0xE31D0DB))
    {
        kasfAttackMothership();
        return;
    }
    if (!kasfMsgReceived((char *)0xD89F74E))
    {
        return;
    }

    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x6A81C51)))
    {
        destination = kasVectorPtrIfExists((char *)0x7A06B51);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(0x6A81C52)))
    {
        destination = kasVectorPtrIfExists((char *)0x7A06B52);
    }
    else
    {
        destination = kasVectorPtrIfExists((char *)0x17A06B3);
    }
    kasfTeamHyperspaceIn(destination);
    kasfTeamSwitchPlayerOwner(0);
    kasfSpeechEvent(0x857BCB7, MaintainTeamContext, Watch_Mission12_StrikeTeamDispatchRetreat);
}

void Watch_Mission12_StrikeTeamDispatchRetreat(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x10A2ED4))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(0x56179FF)))
        {
            kasfSpeechEvent(0x8761820, Begin_Mission12_StrikeTeamDispatchRetreat, Watch_Mission12_StrikeTeamDispatchRetreat_DamageClamp);
        }
        else
        {
            kasfVarDestroy((char *)&team->shipList);
        }
    }
}

void Begin_Mission12_StrikeTeamDispatchRetreat(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetPercentFueled(&team->shipList, 0);
    kasfShipsSetNonRetaliation(&team->shipList);
    kasfTeamSetPercentDamaged(40);
    kasfShipsSetDelayedDeath(&team->shipList, TRUE);
}

void Watch_Mission12_StrikeTeamDispatchRetreat_DamageClamp(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamHealthAverage() > 40)
    {
        kasfTeamSetPercentDamaged(40);
    }
}

void Init_Mission12_AssaultMothershipTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasAITeamShipsPtr(&team->shipList, 5);
    if (GetGameDifficulty() >= 2)
    {
        kasfTacticsAggressive();
    }
    if (GetGameDifficulty() == 0)
    {
        kasfShipsSetNonRetaliation(&team->shipList);
    }
}

void Watch_Mission12_AssaultMothershipTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x76A7002))
    {
        kasfTacticsNeutral();
        kasfShipsSetNonRetaliation(&team->shipList);
        kasfStop();
    }
    if (kasfMsgReceived((char *)0xA7C9B35))
    {
        kasfAttackMothership();
    }
}

void Init_Mission12_FrigateObjective_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfSpeechEvent(0x7464CB7, Init_Mission12_FrigateObjective_FocusTrackedTarget, Watch_Mission12_FrigateObjective_WaitForNearbyFrigate);
}

void Watch_Mission12_FrigateObjective_AttackMothershipCue(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0xA7C9B35))
    {
        kasfAttackMothership();
        kasfSpeechEvent(342540, MaintainTeamContext, MaintainTeamContext);
    }
}

void Init_Mission12_FrigateObjective_FocusTrackedTarget(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamSpecialToggle();
    kasfPatrolPath(kasPathPtr((char *)0x4697381));
}

void Watch_Mission12_FrigateObjective_WaitForNearbyFrigate(void)
{
    AITeam *team;
    GrowSelection *trackedFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    trackedFrigates = kasGrowSelectionPtr((char *)0x36980EB);
    kasfShipsSelectNearby(trackedFrigates,
                          kasGrowSelectionPtr((char *)0x4A2A29),
                          kasShipsVectorPtr((char *)0x3A98),
                          0x3A98);
    if (kasfShipsSelectClass(trackedFrigates, trackedFrigates, "CLASS_Frigate"))
    {
        kasfSpeechEvent(0x857BCB7, Init_Mission12_FrigateObjective_ClearTrackedSelection, Watch_Mission12_FrigateObjective_RetargetWhenNoNearbyFrigate);
    }
}

void Init_Mission12_FrigateObjective_ClearTrackedSelection(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamSpecialToggle();
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x36980EB));
}

void Watch_Mission12_FrigateObjective_RetargetWhenNoNearbyFrigate(void)
{
    AITeam *team;
    GrowSelection *trackedFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    trackedFrigates = kasGrowSelectionPtr((char *)0x36980EB);
    kasfShipsSelectNearby(trackedFrigates,
                          kasGrowSelectionPtr((char *)0x4A2A29),
                          kasShipsVectorPtr((char *)0x3A98),
                          0x3A98);
    if (!kasfShipsSelectClass(trackedFrigates, trackedFrigates, "CLASS_Frigate"))
    {
        kasfSpeechEvent(0x7464CB7, Init_Mission12_FrigateObjective_FocusTrackedTarget, Watch_Mission12_FrigateObjective_WaitForNearbyFrigate);
    }
}

void Watch_Mission12_GuardGravWells_AttackMothership(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
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
            case 54954452:
                kasfObjectiveDestroy("Resource");
                kasfObjectiveCreate("Protect", ((char **)0x8CAAAC)[strCurLanguage], ((char **)0x8CAAB8)[strCurLanguage]);
                break;
            case 76455461:
                kasfObjectiveCreate("Resource", ((char **)0x8CAA94)[strCurLanguage], ((char **)0x8CAAA0)[strCurLanguage]);
                break;
            case 94060206:
                kasfPingAddShipsIndexed(kasAITeamShipsPtrIfExists(77795571), "PingOnResource");
                kasfObjectiveSet("CaptureFrigate", 0);
                break;
            case 105313438:
                kasfPingAddShips(kasAITeamShipsPtrIfExists(115043347), "PingOnFrigates");
                kasfObjectiveSet("CaptureFrigate", 0);
                break;
            case 105466259:
                kasfObjectiveSet("CaptureResource", 1);
                kasfPingRemove("PingOnResource");
                break;
            case 176464661:
                kasfObjectiveCreate("Resource", ((char **)0x8CAA94)[strCurLanguage], ((char **)0x8CAAA0)[strCurLanguage]);
                break;
            case 251107429:
                kasfObjectiveSet("CaptureFrigate", 1);
                kasfPingRemove("PingOnFrigates");
                break;
            case 54981148:
                kasfMissionFailed();
                break;
            case 76159573:
                kasfObjectiveCreate("CaptureResource", ((char **)0x8CAADC)[strCurLanguage], ((char **)0x8CAAE8)[strCurLanguage]);
                kasfObjectiveCreate("CaptureFrigate", ((char **)0x8CAAF4)[strCurLanguage], ((char **)0x8CAB00)[strCurLanguage]);
                kasfObjectiveCreate("NoSurvivors", ((char **)0x8CAB0C)[strCurLanguage], ((char **)0x8CAB18)[strCurLanguage]);
                kasfPingAddShipsIndexed(kasAITeamShipsPtrIfExists(77795571), "PingOnResource");
                break;
            case 134864083:
                kasfObjectiveDestroy("NoSurvivors");
                kasfEntryGetOrCreateSetValue(-1964114797, 1);
                break;
            case 175938357:
                kasfEntryGetOrCreateSetValue(-2045714667, 1);
                kasfObjectiveDestroyAll();
                kasfObjectiveCreate("Hyperspace", ((char **)0x8CAAC4)[strCurLanguage], ((char **)0x8CAAD0)[strCurLanguage]);
                spSetSinglePlayerSaveInfo(8, ((char **)0x8CAA88)[strCurLanguage]);
                break;
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

void Init_Mission12_GuardGravWells_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 2);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfShipsAdd(kasGrowSelectionPtr(130285475), &team->shipList);
}

void Init_Mission12_MissionFlow(void)
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
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfEntryGetOrCreateSetValue(126773838, 1);
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)5801588);
    kasfSpeechEvent(
        32713509,
        (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
        (sdword)Watch_Mission12_MissionFlow);
}

void Watch_Mission12_MissionFlow_WaitForIntelEventEnd(void)
{
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
            kasfSpeechEvent(
                244882162,
                (sdword)Init_Mission11_MissionFlow_Delay,
                (sdword)Watch_Mission12_MissionFlow_Delay);
        }
        else
        {
            kasfSpeechEvent(
                32713509,
                (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
                (sdword)Watch_Mission12_MissionFlow);
        }
    }
}

void Watch_Mission12_MissionFlow_Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)220350372))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)sub_652EF0);
    }
    else if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission12_MissionFlow);
    }
}

void Watch_Mission12_MissionFlow(void)
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
            case 5801588:
                kasfLocationCard(4000, ((char **)0x8CBE8C)[strCurLanguage]);
                kasfMsgSend(team, (char *)92825426);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 92825426:
                kasfMsgSend(team, (char *)92825427);
                spSetSinglePlayerSaveInfo(12, ((char **)0x8CBE8C)[strCurLanguage]);
                break;
            case 92825427:
                kasfEntrySetValue(-2054310428, 0);
                kasfUniversePause();
                kasfOtherKASPause();
                kasfSpeechEventQueueCurrentPlayer(268440083, 0);
                kasfMsgSend(team, (char *)38822801);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 36986663:
                spEventWideScreenIn(90);
                kasfMsgSendAll((char *)124416002);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)156562014);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 38822801:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(143034734));
                kasfCameraSetAngleDeg(349);
                kasfCameraSetDeclinationDeg(-6);
                kasfCameraSetDistance(2700);
                kasfEntrySetValue(127756738, 5);
                kasfMsgSend(team, (char *)53411379);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 38822802:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(143034734));
                kasfCameraSetAngleDeg(349);
                kasfCameraSetDeclinationDeg(-6);
                kasfCameraSetDistance(2700);
                kasfEntrySetValue(127756738, 5);
                kasfMsgSend(team, (char *)114787147);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 53411379:
                kasfTutCameraFocusFar(kasGrowSelectionPtr(211966048));
                kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
                kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
                kasfCameraSetDistance(kasfEntryValueGet(64272901));
                kasfSpeechEventQueueCurrentPlayer(268440084, 0);
                kasfMsgSend(team, (char *)38822802);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 54915746:
                kasfMusicPlay(11);
                kasfMsgSend(kasAITeamPtrIfExists(145791489), (char *)187783220);
                kasfMsgSend(kasAITeamPtrIfExists(145791490), (char *)187783220);
                kasfMsgSend(kasAITeamPtrIfExists(145791491), (char *)187783220);
                kasfMsgSend(kasAITeamPtrIfExists(145791492), (char *)187783220);
                kasfMsgSend(kasAITeamPtrIfExists(145791493), (char *)187783220);
                kasfMsgSend(kasAITeamPtrIfExists(145791494), (char *)187783220);
                kasfMsgSend(team, (char *)54915747);
                kasfEntrySetValue(127756738, 4);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 54915747:
                kasfSpeechEventQueueCurrentPlayer(268444185, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)262333985);
                kasfEntrySetValue(127756738, 4);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 79494820:
                spEventWideScreenIn(90);
                kasfMsgSend(team, (char *)198175250);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 81371987:
                if (!kasfEntryValueGet(71552161))
                {
                    kasfEntryGetOrCreateSetValue(71552161, 1);
                    kasfSpeechEventQueueCurrentPlayer(268444183, 0);
                    kasfSpeechEvent(
                        244212684,
                        (sdword)Init_WaitForIntelEventEnd,
                        (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                    return;
                }
                break;
            case 83086594:
                kasfSpeechEventQueueCurrentPlayer(268444191, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)0x8D9FD3);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 81007716:
                kasfPingRemove("Ping");
                kasfWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)222397970);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 101101799:
                kasfSpeechEventQueueCurrentPlayer(268468768, 0);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 114787147:
                kasfMusicPlay(21);
                kasfSpeechEventQueueCurrentPlayer(268444181, 0);
                kasfEntrySetValue(266064926, 1);
                kasfUniverseUnpause();
                kasfEntrySetValue(211831571, 1);
                kasfMsgSend(team, (char *)225981602);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 156305422:
                kasfSpeechEventQueueCurrentPlayer(268444193, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 156562014:
                kasfShipsStop(kasGrowSelectionPtr(18336307));
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(145791489));
                kasfCameraSetAngleDeg(207);
                kasfCameraSetDeclinationDeg(-22);
                kasfCameraSetDistance(1500);
                kasfSpeechEventQueueCurrentPlayer(268438222, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)54915746);
                kasfEntrySetValue(127756738, 5);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 198175250:
                kasfSpeechEventQueueCurrentPlayer(268444182, 0);
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(100000);
                    kasfEntrySetValue(126773838, 0);
                }
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfTutSetPointerTargetFirstShip((char *)0x8CBB10, kasAITeamShipsPtrIfExists(143034734));
                kasfSensorsSetLookatPoint(kasTeamsVectorPtr(143034734));
                kasfEntrySetValue(168240451, 1);
                kasfEntrySetValue(266064926, 1);
                kasfPingAddShips(kasAITeamShipsPtrIfExists(143034734), "Ping");
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)31797178);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)81371987);
                kasfEntrySetValue(127756738, 180);
                kasfEntrySetValue(52577235, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 220350372:
                kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)sub_652EF0);
                return;
            case 222397970:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(145791489));
                kasfCameraSetAngleDeg(342);
                kasfCameraSetDeclinationDeg(-10);
                kasfCameraSetDistance(1600);
                kasfMsgSend(team, (char *)222397971);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 222397971:
                kasfSpeechEventQueueCurrentPlayer(268444184, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)36986663);
                kasfEntrySetValue(127756738, 30);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(172907219, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 225981602:
                kasfTutCameraFocusFar(kasGrowSelectionPtr(211966048));
                kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
                kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
                kasfCameraSetDistance(kasfEntryValueGet(64272901));
                kasfMsgSend(team, (char *)79494820);
                kasfEntrySetValue(127756738, 90);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission12_MissionFlow_Delay);
                return;
            case 262333985:
                kasfSpeechEventQueueCurrentPlayer(268698138, 0);
                kasfTutCameraFocusFar(kasAITeamShipsPtrIfExists(143034734));
                kasfCameraSetAngleDeg(349);
                kasfCameraSetDeclinationDeg(-6);
                kasfCameraSetDistance(2700);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)262333986);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 262333986:
                kasfSpeechEventQueueCurrentPlayer(268440091, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)262333987);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 262333987:
                kasfSpeechEventQueueCurrentPlayer(268698140, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)262333988);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 262333988:
                kasfSpeechEventQueueCurrentPlayer(268440093, 0);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)262333989);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
                return;
            case 262333989:
                kasfSpeechEventQueueCurrentPlayer(268698142, 0);
                kasfEntrySetValue(232774915, 1);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
                kasfMsgSend(kasAITeamPtrIfExists(217208928), (char *)175938357);
                kasfMsgSend(kasAITeamPtrIfExists(81489361), (char *)175938357);
                kasfMsgSend(kasAITeamPtrIfExists(81489362), (char *)175938357);
                kasfMsgSend(kasAITeamPtrIfExists(218781729), (char *)175938357);
                kasfMsgSend(kasAITeamPtrIfExists(218781730), (char *)175938357);
                kasfEntrySetValue(266064926, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission12_MissionFlow_WaitForIntelEventEnd);
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

void Watch_Mission12_ObjectiveFlow(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
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
            case 9281491:
                kasfObjectiveCreate("WorkerDock", ((char **)0x8CBED4)[strCurLanguage], ((char **)0x8CBEE0)[strCurLanguage]);
                break;
            case 31797178:
                kasfObjectiveCreate(
                    "DefendMothership",
                    ((char **)0x8CBEA4)[strCurLanguage],
                    ((char **)0x8CBEB0)[strCurLanguage]);
                kasfObjectiveCreate(
                    "AttackOutpost",
                    ((char **)0x8CBEBC)[strCurLanguage],
                    ((char **)0x8CBEC8)[strCurLanguage]);
                break;
            case 124416002:
                kasfObjectiveDestroy("DefendMothership");
                kasfObjectiveDestroy("AttackOutpost");
                break;
            case 175938357:
                kasfObjectiveDestroy("WorkerReturn");
                kasfObjectiveCreate("Hyperspace", ((char **)0x8CBF04)[strCurLanguage], ((char **)0x8CBF10)[strCurLanguage]);
                spSetSinglePlayerSaveInfo(12, ((char **)0x8CBE98)[strCurLanguage]);
                break;
            case 218149011:
                kasfObjectiveDestroy("WorkerDock");
                kasfObjectiveCreate("WorkerReturn", ((char **)0x8CBEEC)[strCurLanguage], ((char **)0x8CBEF8)[strCurLanguage]);
                break;
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

    if (!kasfEntryValueGet(71552162) &&
        !kasfShipsCount(kasGrowSelectionPtr(211966048)) &&
        kasfEntryValueGet(-2015429404))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
    }
}

void Watch_Mission12_FleetIntel_FIMissileTechReady(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            237709643,
            (sdword)Init_Mission12_FleetIntel_FIStartResearching,
            (sdword)Watch_Mission12_FleetIntel_FIStartResearching);
    }
}

void Init_Mission12_FleetIntel_FIStartResearching(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)101101799);
    kasfShipsDockWithSelection(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr(211966048));
}

void Watch_Mission12_FleetIntel_FIStartResearching(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)156305422);
        kasfShipsSetSupportFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
        kasfLaunch();
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Watch_Mission12_STRCG_OnTheMoveToMothership(void)
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
        if (message == 115287035)
        {
            if (kasfThisTeamIs(kasAITeamPtrIfExists(232664177)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217196307), 1000);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(232664178)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217195539), 1000);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(232664179)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217195795), 1000);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(232664180)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217197075), 1000);
            }
            kasfSpeechEvent(
                74024315,
                (sdword)Watch_Mission11_Traders,
                (sdword)Watch_Mission11_TraderTargetLoop);
            return;
        }
        if (message != 0)
        {
            aitQueuePointer(team, team, (void *)message);
        }

        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Watch_Mission12_Cruiser(void)
{
    AITeam *team;
    GrowSelection *motherships;
    GrowSelection *launchSelection;
    sdword randomIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        motherships = kasGrowSelectionPtr(14701123);
        randomIndex = kasfRandom(0, kasfShipsCount(motherships) - 1);
        if (kasfShipsSelectIndex(kasGrowSelectionPtr(21541), motherships, randomIndex))
        {
            launchSelection = kasGrowSelectionPtr(118933374);
            kasfEntrySetValue(211499095, 1);
            kasfGrowSelectionClear(launchSelection);
            kasfShipsAdd(launchSelection, kasGrowSelectionPtr(21541));
            kasfShipsAdd(launchSelection, &team->shipList);
            kasfGrowSelectionClear(launchSelection);
            kasfShipsAdd(launchSelection, &team->shipList);
            kasfShipsAdd(launchSelection, kasGrowSelectionPtr(21541));
            kasfShipsSetMilitaryParade(kasGrowSelectionPtr(21541), launchSelection);
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            singlePlayerShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 8);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)34005347);
        kasfEntrySetValue(332667, 0);
        sub_496050();
        kasfEntrySetValue(20314791, 1);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission05_FrigateReinforcementCombatLoop);
    }
}

void Init_Mission12_FormationReleaseCoordinator(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasAITeamShipsPtr(&team->shipList, 0);
    kasfSpeechEvent(104050139, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission12_FormationReleaseCoordinator(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552163) && kasfShipsAnyInSensorRange(&team->shipList))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)81371987);
    }
    if (!kasfEntryValueGet(71552164) && kasfTeamHealthAverage() <= 25)
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfMsgSend(kasAITeamPtrIfExists(145791489), (char *)162731038);
        kasfMsgSend(kasAITeamPtrIfExists(145791490), (char *)162731038);
        kasfMsgSend(kasAITeamPtrIfExists(145791491), (char *)162731038);
        kasfMsgSend(kasAITeamPtrIfExists(145791492), (char *)162731038);
        kasfMsgSend(kasAITeamPtrIfExists(145791493), (char *)162731038);
        kasfMsgSend(kasAITeamPtrIfExists(145791494), (char *)162731038);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)81008740);
    }
    if (!kasfEntryValueGet(71552165) && kasfTeamHealthAverage() <= 15)
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    }
    if (kasfMsgReceived((char *)124416002))
    {
        kasfSpeechEvent(186510843, (sdword)Begin_Mission12_FormationReleaseAbort, 0);
    }
}

void Begin_Mission12_FormationReleaseAbort(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
    kasfStop();
    kasfShipsSetNonRetaliation(&team->shipList);
    kasfShipsEnableAttachNLips(&team->shipList);
    kasfGrowSelectionClear(kasGrowSelectionPtr(108973396));
}

void Watch_Mission12_FormationReleaseAbort(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
}

void Init_Mission12_WorkerDockController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(21861, (sdword)MaintainTeamContext, 0);
}

void Begin_Mission12_WorkerDock(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(157511906));
}

void Watch_Mission12_WorkerDockController(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
        kasTimerCreateSetStartById(126405506, 30);
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)218149011);
    }
    if (kasTimerExpiredById(126405506))
    {
        kasfSpeechEvent(170368583, (sdword)Init_LaunchDockedTeam, 0);
    }
}










































































/*-----------------------------------------------------------------------------
    Init_Mission12 - top-level KAS entry point (IDA: 0x623690)
-----------------------------------------------------------------------------*/

void Init_Mission12(void)
{
    kasCurrentStep = 0;
    scope = KH_mission12;
    kasfDisableMothershipBigGunLaunch();
    kasfDisableAllAIFeatures();
    kasfActivateTech("TECH_SALVAGE");
    kasfActivateTech("TECH_ADVANCED_SENSORS");
    kasfActivateTech("TECH_MIMIC");
    kasfActivateTech("CloakDefenseFighter");
    kasfActivateTech("A1");

    kasFSMCreate(KH_FleetIntel, Init_Mission12_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission12_ObjectiveFlow,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(5193300, Init_Mission12_FormationReleaseCoordinator, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(143034734));
    kasFSMCreate(170530304, Init_Mission12_AssaultMothershipTeam, Watch_Mission12_AssaultMothershipTeam,
                 kasAITeamPtrIfExists(217208928));

    kasFSMCreate(75234260, Init_Mission12_A1LaunchController, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(30554545));
    kasFSMCreate(75234260, Init_Mission12_A1LaunchController, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(30554546));
    kasFSMCreate(75234260, Init_Mission12_A1LaunchController, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(30554547));
    kasFSMCreate(75234260, Init_Mission12_A1LaunchController, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(30554548));

    kasfLabelledVectorSet(258415436, 0, 0, 0);

    kasFSMCreate(10824722, Init_Mission12_ModuleOrderPatrolController, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(35178465));
    kasFSMCreate(10824722, Init_Mission12_ModuleOrderPatrolController, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(35178466));

    kasFSMCreate(237676307, Init_Mission12_FrigateObjective_Start, Watch_Mission12_FrigateObjective_AttackMothershipCue,
                 kasAITeamPtrIfExists(81489361));
    kasFSMCreate(237676307, Init_Mission12_FrigateObjective_Start, Watch_Mission12_FrigateObjective_AttackMothershipCue,
                 kasAITeamPtrIfExists(81489362));

    kasFSMCreate(168854210, Init_Mission12_P1FighterReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(181046849));
    kasFSMCreate(168854210, Init_Mission12_P1FighterReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(181046850));
    kasFSMCreate(168854210, Init_Mission12_P1FighterReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(181046851));

    kasFSMCreate(209160610, Init_Mission12_P1FighterOrMissileCorvetteReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(230547089));
    kasFSMCreate(209160610, Init_Mission12_P1FighterOrMissileCorvetteReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(230547090));
    kasFSMCreate(209160610, Init_Mission12_P1FighterOrMissileCorvetteReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(85477153));

    kasFSMCreate(96747187, Init_Mission12_SpecialAttackTeam, Watch_Mission12_SpecialAttackTeam,
                 kasAITeamPtrIfExists(218781729));
    kasFSMCreate(96747187, Init_Mission12_SpecialAttackTeam, Watch_Mission12_SpecialAttackTeam,
                 kasAITeamPtrIfExists(218781730));

    kasFSMCreate(63425861, Init_Mission12_P1StandardCorvetteReinforcementTeam, Watch_Mission12_DeleteTeamOnAbortSignal,
                 kasAITeamPtrIfExists(102370833));

    kasFSMCreate(140367886, Init_Mission12_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(145791489));
    kasFSMCreate(140367886, Init_Mission12_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(145791490));
    kasFSMCreate(140367886, Init_Mission12_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(145791491));
    kasFSMCreate(140367886, Init_Mission12_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(145791492));
    kasFSMCreate(140367886, Init_Mission12_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(145791493));
    kasFSMCreate(140367886, Init_Mission12_SixShipFormationTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(145791494));

    kasfShipsSetDontApplyPhysicsFlag(kasAITeamShipsPtrIfExists(145791489), 1);

    kasFSMCreate(102391524, Init_Mission12_IdleResetController, Watch_Mission12_AuxTeamDeleteOnAbortSignal,
                 kasAITeamPtrIfExists(48288484));
    kasFSMCreate(199689021, Init_Mission12_WorkerDockController, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(199689021));

    kasfDisablePlayerHyperspace();

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

    kasfModuleSlotOrderSetShipType(1, "p1Fighter");
    kasfModuleSlotOrderSetShipType(1, "p1MissileCorvette");
    kasfModuleSlotOrderSetShipType(1, "p1IonArrayFrigate");
    kasfModuleSlotOrderSetCount(1, 2);

    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfGrowSelectionClear(kasGrowSelectionPtr(326187));

    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(18336307), "dJunk1PartA");
    kasfShipsAdd(kasGrowSelectionPtr(326187), kasGrowSelectionPtr(363040));
    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(18336307), "dJunk1Shell");
    kasfShipsAdd(kasGrowSelectionPtr(326187), kasGrowSelectionPtr(363040));
    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(18336307), "dJunk1Strut");
    kasfShipsAdd(kasGrowSelectionPtr(326187), kasGrowSelectionPtr(363040));
    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(18336307), "dJunk2Shipwreck");
    kasfShipsAdd(kasGrowSelectionPtr(326187), kasGrowSelectionPtr(363040));
    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(18336307), "dJunk3Boiler");
    kasfShipsAdd(kasGrowSelectionPtr(326187), kasGrowSelectionPtr(363040));
    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(18336307), "dJunk3BoilerCasing");
    kasfShipsAdd(kasGrowSelectionPtr(326187), kasGrowSelectionPtr(363040));

    kasfShipsSetAttachCoordsysFlag(kasGrowSelectionPtr(326187), 1);
    kasfShipsSetDelayedDeath(kasGrowSelectionPtr(326187), 1);

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

    kasTimerCreateSetStartById(150120898, 1);

    kasfSetSalCapProcessingShip(kasAITeamShipsPtrIfExists(217208928));
}
