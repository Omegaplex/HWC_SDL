/*=============================================================================
    Name    : mission13.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission13
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m13_saveInfoName[] =
{
    "Location Unknown",
    "French version goes here",
    "German version goes here",
};

#define sub_491EC0 kasTimerCreateById
#define sub_491F90 kasTimerCreateSetStartById
#define sub_4941A0 kasfMoveTo
#define sub_496380 kasfTutSetPointerTargetFirstShip
#define sub_496530 kasfSubtitleDraw
#define sub_497420 kasfWideScreenIn
#define sub_497630 kasfWideScreenOut
#define GetGameDifficulty_ GetGameDifficulty

void Init_Mission13_MissionFlow_BriefingSequence(void);
void Watch_Mission13_MissionFlow(void);
void Watch_Mission13_MissionFlow_PostIntelDelay(void);
void Watch_Mission13_MissionFlow_DestroyEnemyBriefing(void);
void Watch_Mission13_MissionFlow_ProtectFaalcorBriefing(void);
void Init_Mission13_AttackBomberWing_SpawnDelay(void);
void Watch_Mission13_AttackBomberWing_SpawnDelay(void);
void Init_Mission13_AttackBomberWing_Spawn(void);
void Watch_Mission13_AttackBomberWing_Spawn(void);
void Watch_Mission13_AttackBomberWing_AwaitAttackSignal(void);
void Watch_Mission13_HeavyInterceptorWing_AwaitAttackSignal(void);
void Init_Mission13_HeavyInterceptorWing_Spawn(void);
void Watch_Mission13_HeavyInterceptorWing_Spawn(void);
void Watch_Mission13_EnemyCarrierController_AwaitEscortLoss(void);
void Init_Mission13_EnemyCarrier_FinalApproach(void);
void Watch_Mission13_EnemyCarrier_FinalApproach(void);
void Init_Mission13_FinalAttackTimeout(void);
void Watch_Mission13_FinalAttackTimeout(void);
void Watch_Mission13_KamikazeWave(void);
static void Watch_Mission13_KamikazeWave_AttackRun(void);
static void Begin_Mission13_KamikazeWave_ReissueAttackRun(void);
void Begin_Mission13_MinelayerEscortReserve_CombatLoop(void);
void Watch_Mission13_MinelayerEscortReserve_CombatLoop(void);
void Watch_Mission13_MinelayerEscortReserve(void);
void Init_Mission13_MinelayerCorvetteReinforcement(void);
void Init_Mission13_MinelayerCorvette_FinalApproach(void);
static void Watch_Mission13_MinelayerCorvetteTeam_AwaitLoss(void);
void Watch_Mission13_MinelayerCorvetteReinforcement(void);
void Watch_Mission13_MinelayerCorvette_FinalApproach(void);
void Watch_Mission13_HeavyDefenderTeam_ReplenishTrigger(void);
void Init_Mission13_HeavyDefenderTeam_Replenish(void);
void Watch_Mission13_HeavyDefenderTeam_Replenish(void);
void Init_Mission13_HeavyDefenderPatrol_AttackRun(void);
void Watch_Mission13_HeavyDefenderPatrol(void);
void Init_Mission13_HeavyDefenderPatrol_DespawnDelay(void);
void Watch_Mission13_HeavyDefenderPatrol_DespawnDelay(void);
void Watch_Mission13_TraderWing_AwaitAttackSignal(void);
void Watch_Mission13_TraderWing_TargetLoop(void);
void Watch_Mission13_TraderEscort_AwaitAttackSignal(void);
void Init_Mission13_TraderEscort_MoveToLinkedTeam(void);
void Watch_Mission13_TraderEscort_AwaitLinkedTeamLoss(void);
void Select_Mission13_TraderTargets(void);
void Watch_Mission13_TraderTargetLoop(void);
void Init_Mission13_ReinforcementParadeHyperspaceIn(void);
void Watch_Mission13_ReinforcementParade_AwaitDepartSignal(void);
void Init_Mission13_ReinforcementParade_Depart(void);
void Watch_Mission13_ReinforcementParade_Depart(void);
void Watch_Mission13_ParadeFollower_AwaitDepartSignal(void);
void Init_Mission13_ParadeFollower_Depart(void);
void Watch_Mission13_ParadeFollower_BroadcastAdvanceSignal(void);
void Init_Mission13_ParadeFollower_DockOrPassive(void);
void Watch_Mission13_ParadeFollower_HandleDepartOrAdvanceSignal(void);
void Init_Mission13_ParadeFollower_DockToFaalcor(void);
void Watch_Mission13_ParadeFollower_AwaitDockComplete(void);
void Watch_Mission13_ParadeFollower_AwaitSupportSignal(void);
void Init_Mission13_ParadeFollower(void);
void Watch_Mission13_ParadeWorkerWing_AwaitDepartSignal(void);
void Init_Mission13_ParadeWorkerWing_Depart(void);
void Watch_Mission13_ParadeWorkerWing_Depart(void);
void Init_Mission13_ParadeWorkerWing_SpawnWorkers(void);
void Init_Mission13_ParadeWorkerWing_DockToFaalcor(void);
void Watch_Mission13_ParadeWorkerWing_AwaitAdvanceSignal(void);
void Watch_Mission13_ParadeWorkerWing_AwaitCombatDone(void);
void Select_Mission13_ParadeWorkerWing_Targets(void);
void Watch_Mission13_ParadeWorkerWing_TargetLoop(void);
void Init_Mission13_ParadeWorkerWing(void);
void Watch_Mission13_ParadeReserveWing_AwaitDepartSignal(void);
void Init_Mission13_ParadeReserveWing(void);
void Select_Mission13_ParadeReserveWing_Targets(void);
void Watch_Mission13_ParadeReserveWing_TargetLoop(void);
static void Init_Mission13_ParadeWorkerWing_TargetLoop(void);
static void Init_Mission13_ParadeWorkerWing_SpawnAcolytes(void);
void Watch_Mission13_StrikeWing_AwaitAttackSignal(void);
void Init_Mission13_StrikeWing_ReacquireDelay(void);
void Watch_Mission13_StrikeWing_ReacquireDelay(void);
void Watch_Mission13_StrikeWing2_AwaitAttackSignal(void);
void Init_Mission13_StrikeWing2_RegroupToLinkedTeam(void);
void Watch_Mission13_StrikeWing2_AwaitLinkedTeamLoss(void);
void Select_Mission13_StrikeWing2_Targets(void);
void Watch_Mission13_StrikeWing2_TargetLoop(void);
void Watch_Mission13_StrikeWing3_AwaitAttackSignal(void);
void Select_Mission13_StrikeWing3_Targets(void);
void Watch_Mission13_StrikeWing3_TargetLoop(void);
static void Select_Mission13_TraderWing_Targets(void);
static void Watch_Mission13_TraderWing_TargetLoop_Reacquire(void);
void Watch_Mission13_StrikeWingShared_AwaitAttackSignal(void);
void Init_Mission13_StrikeWingShared_RegroupToLinkedTeam(void);
void Watch_Mission13_StrikeWingShared_AwaitLinkedTeamLoss(void);
void Watch_Mission13_StrikeWingShared_TargetLoop(void);
static void sub_499790(void);

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

void Watch_Mission13(void)
{
  int v0; // eax
  int v1; // eax
  int v2; // eax
  int v3; // eax
  int *v4; // eax
  int *v5; // eax
  int v6; // eax
  int v7; // eax
  int v8; // eax
  int v9; // eax
  int v10; // eax
  int v11; // eax
  int *v12; // eax
  signed int v13; // eax
  int *v14; // eax
  int *v15; // eax
  int *v16; // eax
  int *v17; // eax
  int *v18; // eax
  int *v19; // eax
  int *v20; // eax
  int *v21; // eax
  int v22; // eax
  int *v23; // eax
  int v24; // eax
  int v25; // eax
  int v26; // eax
  int v27; // eax
  int v28; // eax
  int *v29; // eax
  int v30; // eax
  int v31; // eax
  int v32; // eax
  int v33; // eax
  int v34; // eax
  int *v35; // eax
  int v36; // eax
  int *v37; // eax
  int v38; // eax
  int v39; // eax
  int v40; // eax
  int v41; // eax
  int *v42; // eax
  int v44; // [esp-8h] [ebp-8h]
  int v45; // [esp-8h] [ebp-8h]
  int v46; // [esp-4h] [ebp-4h]
  int v47; // [esp-4h] [ebp-4h]
  int v48; // [esp-4h] [ebp-4h]
  int v49; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503619;
  v0 = kasfCameraGetAngleDeg();
  kasfLogInteger(aAngleD, v0);
  v1 = kasfCameraGetDeclinationDeg();
  kasfLogInteger(aDeclinationD, v1);
  v2 = kasfCameraGetDistance();
  kasfLogInteger(aDistanceD, v2);
  if ( kasTimerExpiredDestroyById(50796994) )
  {
    v46 = kasGrowSelectionPtr(18336307);
    v3 = kasVolumePtr(132933767);
    kasfFindShipsInside(v3, v46);
    v47 = kasGrowSelectionPtr(18336307);
    v4 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v4, v47);
    v48 = kasGrowSelectionPtr(18336307);
    v5 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v5, v48);
    if ( GetGameDifficulty() < 2 && !kasfEntryValueGet(71552169) )
    {
      v6 = kasAITeamShipsPtrIfExists(228456241);
      if ( kasfShipsAnyInSensorRange(v6)
        || (v7 = kasAITeamShipsPtrIfExists(92316977), kasfShipsAnyInSensorRange(v7))
        || (v8 = kasAITeamShipsPtrIfExists(228456242), kasfShipsAnyInSensorRange(v8))
        || (v9 = kasAITeamShipsPtrIfExists(92316978), kasfShipsAnyInSensorRange(v9))
        || (v10 = kasAITeamShipsPtrIfExists(228456243), kasfShipsAnyInSensorRange(v10))
        || (v11 = kasAITeamShipsPtrIfExists(92316979), kasfShipsAnyInSensorRange(v11)) )
      {
        kasfEntrySetValue(71552169, 1);
        v12 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v12, (char *)58599404);
      }
    }
    v13 = kasfRandom(8, 15);
    kasTimerSetById(50796994, v13);
    kasTimerStartById(50796994);
  }
  if ( !kasfEntryValueGet(71552170) && kasfEntryValueGet(-2015429404) && kasfIntelEventEnded() )
  {
    kasfEntrySetValue(71552170, 1);
    v14 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v14, (char *)5801588);
    spSetSinglePlayerSaveInfo(13, m13_saveInfoName[strCurLanguage]);
  }
  if ( !kasfEntryValueGet(71552171) && kasTimerExpiredById(-1919585614) )
  {
    kasfEntrySetValue(71552171, 1);
    v15 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v15, (char *)51823726);
  }
  if ( !kasfEntryValueGet(71552172) && kasTimerExpiredById(-2146342830) )
  {
    kasfEntrySetValue(71552172, 1);
    v16 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v16, (char *)93785508);
  }
  if ( !kasfEntryValueGet(71552173) && kasTimerExpiredById(-2002086123) )
  {
    kasfEntrySetValue(71552173, 1);
    v17 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v17, (char *)101498677);
  }
  if ( kasfEntryValueGet(-2027227947) )
  {
    if ( !kasfEntryValueGet(71552174) )
    {
      if ( kasTimerExpiredDestroyById(203302989)
        || (v49 = kasfEntryValueGet(-1999086536),
            v44 = kasGrowSelectionPtr(211966048),
            v18 = (int *)kasGrowSelectionPtr(125237552),
            kasfShipsSelectDamaged(v18, v44, v49) > 0) )
      {
        kasfEntrySetValue(71552174, 1);
        kasTimerDestroyById(203302989);
        v45 = kasGrowSelectionPtr(211966048);
        v19 = (int *)kasGrowSelectionPtr(125237552);
        if ( !kasfShipsSelectDamaged(v19, v45, 0) )
        {
          v20 = kasAITeamPtrIfExists(176850956);
          kasfMsgSend((struct AITeam *)(int)v20, (char *)87236965);
        }
        kasfEntryGetOrCreateSetValue(-1914453392, 1);
      }
    }
  }
  if ( !kasfEntryValueGet(71552175) && kasTimerExpiredById(-2121429067) )
  {
    kasfEntrySetValue(71552175, 1);
    v21 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v21, (char *)74001333);
    v22 = kasGrowSelectionPtr(211966048);
    kasfShipsAttributesBitClear((GrowSelection *)v22, 1024);
  }
  if ( !kasfEntryValueGet(71552176) )
  {
    if ( kasTimerExpiredById(-2020468171) )
    {
      kasfEntrySetValue(71552176, 1);
      kasfBuilderRestrictNone();
      v23 = kasAITeamPtrIfExists(176850956);
      kasfMsgSend((struct AITeam *)(int)v23, (char *)140308101);
      v24 = kasGrowSelectionPtr(211966048);
      kasfShipsAttributesBitClear((GrowSelection *)v24, 0x80000);
      v25 = kasGrowSelectionPtr(266741017);
      if ( kasfShipsCount(v25) > 0 )
      {
        v26 = kasGrowSelectionPtr(266741017);
        kasfShipsSetDontApplyPhysicsFlag((GrowSelection *)v26, 0);
      }
    }
  }
  if ( !kasfEntryValueGet(71552177) )
  {
    v27 = kasAITeamShipsPtrIfExists(123922546);
    if ( kasfShipsCount(v27) )
    {
      v28 = kasAITeamShipsPtrIfExists(123922546);
      if ( kasfShipsAnyInSensorRange(v28) )
      {
        kasfEntrySetValue(71552177, 1);
        v29 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v29, (char *)16403059);
      }
    }
  }
  if ( !kasfEntryValueGet(71552178) )
  {
    if ( kasTimerExpiredById(-1958016235) )
    {
      kasfEntrySetValue(71552178, 1);
      if ( GetGameDifficulty() > 1 )
      {
        v30 = kasGrowSelectionPtr(29806117);
        if ( kasfShipsCount(v30) > 0 )
        {
          v31 = kasGrowSelectionPtr(29806117);
          kasfShipsSetDontApplyPhysicsFlag((GrowSelection *)v31, 0);
          v32 = kasGrowSelectionPtr(29806117);
          kasfShipsSetPercentDamaged((GrowSelection *)v32, 0);
          v33 = kasGrowSelectionPtr(29806117);
          kasfShipsDamage((GrowSelection *)v33, 1);
        }
      }
    }
  }
  if ( !kasfEntryValueGet(71552179) )
  {
    v34 = kasGrowSelectionPtr(126483525);
    if ( !kasfShipsCount(v34) )
    {
      kasfEntrySetValue(71552179, 1);
      v35 = kasAITeamPtrIfExists(187875832);
      kasfMsgSend((struct AITeam *)(int)v35, (char *)115287035);
      kasTimerSetById(203302989, 200);
      kasTimerSetById(-2121429067, 1);
      kasTimerSetById(-2020468171, 270);
    }
  }
  if ( !kasfEntryValueGet(71552180) )
  {
    v36 = kasGrowSelectionPtr(70045477);
    if ( !kasfShipsCount(v36) )
    {
      kasfEntrySetValue(71552180, 1);
      v37 = kasAITeamPtrIfExists(187875832);
      kasfMsgSend((struct AITeam *)(int)v37, (char *)115287019);
    }
  }
  if ( !kasfEntryValueGet(71552181) )
  {
    v38 = kasGrowSelectionPtr(114961833);
    if ( !kasfShipsCount(v38) )
    {
      v39 = kasGrowSelectionPtr(126483525);
      if ( !kasfShipsCount(v39) )
      {
        v40 = kasGrowSelectionPtr(70045477);
        if ( !kasfShipsCount(v40) )
        {
          v41 = kasGrowSelectionPtr(241823333);
          if ( !kasfShipsCount(v41) )
          {
            kasfEntrySetValue(71552181, 1);
            kasfEntryGetOrCreateSetValue(-1948554199, 1);
          }
        }
      }
    }
  }
  if ( !kasfEntryValueGet(-2092529271) && kasfEntryValueGet(-1948554199) )
  {
    kasfEntrySetValue(-2092529271, 1);
    kasfObjectiveDestroy("DESTROY_ENEMY");
  }
  if ( !kasfEntryValueGet(-2118074508) && kasfEntryValueGet(-1948554199) )
  {
    kasfEntrySetValue(-2118074508, 1);
    kasfObjectiveDestroy("PROTECT_FAALCORUM");
  }
  if ( !kasfEntryValueGet(71552182) && kasfEntryValueGet(-2092529271) && kasfEntryValueGet(-2118074508) )
  {
    kasfEntrySetValue(71552182, 1);
    v42 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v42, (char *)185311156);
  }
  if ( kasfEntryValueGet(-1914453392) )
  {
    if ( !kasfEntryValueGet(71552183) )
    {
      kasfEntrySetValue(71552183, 1);
      switch ( GetGameDifficulty() )
      {
        case 0:
          kasTimerSetById(126238450, 200);
          break;
        case 1:
          kasTimerSetById(126238450, 120);
          break;
        case 2:
          kasTimerSetById(126238450, 90);
          break;
        case 3:
          kasTimerSetById(126238450, 60);
          break;
        case 4:
          kasTimerSetById(126238450, 45);
          break;
        default:
          break;
      }
    }
    if ( !kasfEntryValueGet(71552184) )
    {
      if ( kasTimerExpiredById(126238450) )
      {
        kasfEntrySetValue(71552184, 1);
        kasfEntryGetOrCreateSetValue(-1914453392, 0);
      }
    }
  }
}

void Watch_Mission13_MissionFlow_WaitForIntelEventEnd(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded())
    {
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)220350372);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission13_MissionFlow_FILost(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists((char *)108436545)))
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Begin_Mission13_MissionFlow_FILostAreaClear,
            (sdword)Watch_Mission13_MissionFlow_FILostAreaClear);
    }
}

void Watch_Mission13_MissionFlow_Farewell(void)
{
    AITeam *team;
    GrowSelection *attackers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552172) && kasfTeamFinishedLaunching())
    {
        kasfEntryGetOrCreateSetValue(71552172, 1);
        kasfTacticsAggressive();
        attackers = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(attackers);
        kasfShipsAdd(attackers, &team->shipList);
        singlePlayerSortShipsByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
        tutDeallocateRootRegion();
        kasfMsgSend(kasAITeamPtrIfExists((char *)108436545), (char *)5281445);
    }
    if (kasfShipsAnyIdle(&team->shipList))
    {
        attackers = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(attackers);
        kasfShipsAdd(attackers, &team->shipList);
        singlePlayerSortShipsByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
        tutDeallocateRootRegion();
    }
}

void Init_Mission13_PrimaryObjectiveController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfBuilderRestrictAll();
}

void Begin_Mission13_MissionFlow_FILostAreaClear(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTacticsAggressive();
}

void Watch_Mission13_MissionFlow_FILostAreaClear(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x10A2ED4))
    {
        kasfVarDestroy((char *)&team->shipList);
        kasfSpeechEvent(
            142812005,
            (sdword)Init_Mission13_MissionFlow_FILostReinforcementDelay,
            (sdword)Init_Mission02_ACT3Corvettes1_Launch);
        return;
    }

    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            142812005,
            (sdword)Init_Mission13_MissionFlow_FILostReinforcementDelay,
            (sdword)Init_Mission02_ACT3Corvettes1_Launch);
        return;
    }

    if (kasfShipsSelectCapital(kasGrowSelectionPtr((char *)342540), &team->shipList))
    {
        kasAITeamShipsPtrIfExists((char *)90274303);
        tutDeallocateRootRegion();
    }
    else
    {
        kasfShipsSelectNearby(
            kasGrowSelectionPtr((char *)86599289),
            kasGrowSelectionPtr((char *)0x4A2A29),
            kasTeamsVectorPtr((char *)90274303),
            10000);
    }
    kasGrowSelectionPtr((char *)86599289);
    tutDeallocateRootRegion();
}

void Init_Mission13_MissionFlow_FILostReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(148949186, kasfRandom(45, 90));
}

void Init_Mission13_StrikeWingShared_AttackMothership(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(80578718, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}

void Watch_Mission13_LGuns_Waiting(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(-2082781531) && !kasfShipsCount(kasGrowSelectionPtr(130285475)))
    {
        kasfEntryGetOrCreateSetValue(-2082781531, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)86933668);
        kasfMusicPlay(7);
    }
}

void Init_Mission13_MissionFlow(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    sub_491EC0(144337492);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(232774915, 0);
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfEntryGetOrCreateSetValue(126773838, 1);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(32713509, Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission13_MissionFlow);
}

void Watch_Mission13_MissionFlow_PostIntelCleanup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
    {
        kasfEntrySetValue(-2054310428, 0);
        if (kasfEntryValueGet(168240451))
        {
            sub_496530();
            kasfEntrySetValue(168240451, 0);
        }
        if (kasfEntryValueGet(266064926))
        {
            sub_497630(90);
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
            kasfEntrySetValue(172907219, 0);
        }
        if (kasfEntryValueGet(232774915))
        {
            kasfTutCameraFocusCancel();
            kasfTutCameraFocusCancel();
            kasfEntrySetValue(232774915, 0);
        }
        if (kasfEntryValueGet(127756738))
        {
            kasfEntrySetValue(127756738, 0);
            kasTimerStartById(144337492);
        }
        kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_MissionFlow_PostIntelDelay);
    }
}

void Watch_Mission13_MissionFlow_PostIntelDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(32713509, Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission13_MissionFlow);
    }
}

void Watch_Mission13_MissionFlow(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    if (!kasfEntryValueGet(332667))
    {
        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = (queueHead != NULL) ? *queueHead : -1;
        while (queueIndex >= 0)
        {
            message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
            switch (message)
            {
                case 0:
                    break;
                case 5801588:
                    kasTimerSetById(144337492, 3);
                    kasTimerStartById(144337492);
                    sub_497630(90);
                    kasfMsgSend(team, (char *)92825426);
                    kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_MissionFlow_PostIntelDelay);
                    return;
                case 16403059:
                    kasTimerSetById(144337492, 3);
                    kasTimerStartById(144337492);
                    kasfEntrySetValue(332667, 1);
                    sub_497420(90);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)262448962);
                    kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_MissionFlow_PostIntelDelay);
                    return;
                case 51823726:
                    kasfSpeechEventQueueCurrentPlayer(268440111, 0);
                    sub_491F90(-2146342830, 30);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 87236965:
                    kasfSpeechEvent(101191117, Init_Mission13_MissionFlow_BriefingSequence, (sdword)Watch_Mission13_MissionFlow_ProtectFaalcorBriefing);
                    return;
                case 58599404:
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEventQueueCurrentPlayer(268444205, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)132284130);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 74001333:
                    kasfSpeechEventQueueCurrentPlayer(268440114, 0);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 89548626:
                    kasfSpeechEvent(191248531, Init_Mission13_MissionFlow_BriefingSequence, (sdword)Watch_Mission13_MissionFlow_DestroyEnemyBriefing);
                    return;
                case 93785508:
                    kasfSpeechEventQueueCurrentPlayer(268444208, 0);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 101498677:
                    kasfSpeechEventQueueCurrentPlayer(268444216, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)101498677);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 140308101:
                    kasfSpeechEventQueueCurrentPlayer(268440115, 0);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 185311156:
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEventQueueCurrentPlayer(268444214, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)103880325);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 220350372:
                    kasfSpeechEvent(77979796, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_WaitForIntelEventEnd);
                    return;
                default:
                    aitQueuePointer(team, team, (void *)message);
                    break;
            }

            queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
            if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
            {
                break;
            }
        }
    }
    else
    {
        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = (queueHead != NULL) ? *queueHead : -1;
        while (queueIndex >= 0)
        {
            message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
            switch (message)
            {
                case 0:
                    break;
                case 103880325:
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEventQueueCurrentPlayer(268440119, 0);
                    sub_491F90(-2002086123, 3);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 132284130:
                    kasfSpeechEventQueueCurrentPlayer(268444206, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)132284131);
                    kasfEntrySetValue(332667, 0);
                    sub_491F90(-1919585614, 30);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                case 262448962:
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfOtherKASPause();
                    kasfUniversePause();
                    kasfEntrySetValue(211831571, 1);
                    kasfEntrySetValue(109086661, 1);
                    if (!kasfSensorsIsOpen(0))
                    {
                        kasfOpenSensors(100000);
                        kasfEntrySetValue(126773838, 0);
                    }
                    sub_496380("Carrierpointer", (int)kasAITeamShipsPtrIfExists(123922546));
                    kasfSensorsSetLookatPoint((int)kasShipsVectorPtrIfExists(123922546));
                    kasfEntrySetValue(168240451, 1);
                    kasfEntrySetValue(52577235, 1);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEventQueueCurrentPlayer(268444212, 0);
                    kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
                    return;
                default:
                    if (message != 0)
                    {
                        aitQueuePointer(team, team, (void *)message);
                    }
                    break;
            }

            queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
            if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
            {
                break;
            }
        }
    }
}

void Watch_Mission13_PrimaryObjectiveController(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        switch (message)
        {
            case 0:
                break;
            case 24290736:
                kasfObjectiveCreate("DESTROY_ENEMY", ((char **)0x8CC274)[strCurLanguage], ((char **)0x8CC280)[strCurLanguage]);
                kasfObjectiveSet("DESTROY_ENEMY", 0);
                break;
            case 101498677:
                objectiveDestroyAll();
                kasfObjectiveCreate("Hyperspace", ((char **)0x8CC2A4)[strCurLanguage], ((char **)0x8CC2B0)[strCurLanguage]);
                kasfObjectiveSet("Hyperspace", 0);
                spSetSinglePlayerSaveInfo(13, m13_saveInfoName[strCurLanguage]);
                break;
            case 220350372:
                kasfMissionFailed();
                break;
            case 232458573:
                kasfObjectiveCreate("PROTECT_FAALCORUM", ((char **)0x8CC28C)[strCurLanguage], ((char **)0x8CC298)[strCurLanguage]);
                kasfObjectiveSet("PROTECT_FAALCORUM", 0);
                break;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission13_AttackBomberWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!GetGameDifficulty_())
    {
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfSpeechEvent(5532277, (sdword)Init_Mission13_AttackBomberWing_SpawnDelay, (sdword)Watch_Mission13_AttackBomberWing_SpawnDelay);
}

void Watch_Mission13_AttackBomberWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552163) && !kasfTeamCount() && kasfEntryValueGet(-2027278702))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Init_Mission13_AttackBomberWing_HuntNearbyTargets(void)
{
    GrowSelection *targets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    targets = kasGrowSelectionPtr(0x36980EB);
    kasfGrowSelectionClear(targets);
    if (!kasfShipsCount(targets) && kasfRandom(1, 3) != 1)
    {
        kasfUnderAttack(kasGrowSelectionPtr(0x857BCE3));
    }
    if (!kasfShipsCount(targets) && GetGameDifficulty() > 1 && (GetGameDifficulty() > 2 || kasfRandom(1, 3) != 1))
    {
        kasfShipsSelectType(targets, kasGrowSelectionPtr(0x4A2A29), "sWorker");
    }
    if (!kasfShipsCount(targets) && GetGameDifficulty() > 0 && (GetGameDifficulty() > 2 || kasfRandom(1, 3) != 1))
    {
        kasfShipsSelectType(targets, kasGrowSelectionPtr(0x4A2A29), "sRammingFrigate");
    }
    if (!kasfShipsCount(targets))
    {
        kasfShipsSelectType(targets, kasGrowSelectionPtr(0x4A2A29), "sCarrier");
    }
    if (!kasfShipsCount(targets))
    {
        kasfFindShipsTargetingSelection(kasGrowSelectionPtr(0x6DA2DA9), targets);
    }
    if (!kasfShipsCount(targets))
    {
        kasfShipsSelectCapital(targets, kasGrowSelectionPtr(0x4A2A29));
    }
    kasfFormationDelta();
    tutDeallocateRootRegion(targets);
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_AttackBomberWing_HuntNearbyTargets(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
        {
            kasTimerDestroyById(141340587);
            kasfSpeechEvent(74024315, (sdword)Init_Mission13_AttackBomberWing_HuntNearbyTargets, (sdword)Watch_Mission13_AttackBomberWing_HuntNearbyTargets);
        }
    }
    else
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(5532277, (sdword)Init_Mission13_AttackBomberWing_SpawnDelay, (sdword)Watch_Mission13_AttackBomberWing_SpawnDelay);
    }
}

void Init_Mission13_AttackBomberWing_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(201190885, 15 * kasfRandom(1, 5));
}

void Watch_Mission13_AttackBomberWing_SpawnDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(201190885))
    {
        if (kasfEntryValueGet(-2051875389) <= kasfEntryValueGet(-2144628077))
        {
            kasfEntryIncrement(-2051875389);
            kasfSpeechEvent(110655924, Init_Mission13_AttackBomberWing_Spawn, (sdword)Watch_Mission13_AttackBomberWing_Spawn);
        }
        else
        {
            kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        }
    }
}

void Init_Mission13_AttackBomberWing_Spawn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-2046393961))
    {
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }
    kasfShipsCreateInside(
        kasGrowSelectionPtr(164967155),
        "tAttackBomber",
        kasfRandom(4, 8),
        kasAITeamShipsPtrIfExists(123922546),
        1);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(164967155));
    kasAITeamShipsPtr(kasGrowSelectionPtr(164967155), 2);
    kasfEntryIncrement(-2126415161);
}

void Watch_Mission13_AttackBomberWing_Spawn(void)
{
    AITeam *targetTeam;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching() || kasfEntryValueGet(-2027278702))
    {
        if (kasfTeamCount() > 0)
        {
            kasfEntryDecrement(-2126415161);
            if (kasfThisTeamIs(kasAITeamPtrIfExists(68592433)))
            {
                targetTeam = kasAITeamPtrIfExists(126123489);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(68592434)))
            {
                targetTeam = kasAITeamPtrIfExists(126123490);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(68592435)))
            {
                targetTeam = kasAITeamPtrIfExists(126123491);
            }
            else
            {
                kasfSpeechEvent(190746231, (sdword)sub_652EF0, (sdword)Watch_Mission13_AttackBomberWing_AwaitAttackSignal);
                return;
            }
            kasfMsgSend(targetTeam, (char *)126123267);
            kasfSpeechEvent(190746231, (sdword)sub_652EF0, (sdword)Watch_Mission13_AttackBomberWing_AwaitAttackSignal);
        }
    }
}

void Watch_Mission13_AttackBomberWing_AwaitAttackSignal(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-2046393961))
    {
        kasfSpeechEvent(74024315, (sdword)Init_Mission13_AttackBomberWing_HuntNearbyTargets, (sdword)Watch_Mission13_AttackBomberWing_HuntNearbyTargets);
        return;
    }

    team = (AITeam *)kasCurrentTeamPtr;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message == 70883199)
        {
            kasfSpeechEvent(74024315, (sdword)Init_Mission13_AttackBomberWing_HuntNearbyTargets, (sdword)Watch_Mission13_AttackBomberWing_HuntNearbyTargets);
            return;
        }
        if (message != 0)
        {
            aitQueuePointer(team, team, (void *)message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission13_HeavyInterceptorWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!GetGameDifficulty_())
    {
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfSpeechEvent(143009833, (sdword)sub_652EF0, (sdword)Watch_Mission13_HeavyInterceptorWing_AwaitAttackSignal);
}

void Watch_Mission13_HeavyInterceptorWing(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552164) && !kasfTeamCount() && kasfEntryValueGet(-2027278702))
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Init_Mission13_HeavyInterceptorWing_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(126123489)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(68592433));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(126123490)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(68592434));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(126123491)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(68592435));
    }
    kasfFormationDelta();
}

void Watch_Mission13_HeavyInterceptorWing_Regroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (((!kasfThisTeamIs(kasAITeamPtrIfExists(126123489))) || kasfShipsCount(kasAITeamShipsPtrIfExists(68592433)))
     && ((!kasfThisTeamIs(kasAITeamPtrIfExists(126123490))) || kasfShipsCount(kasAITeamShipsPtrIfExists(68592434)))
     && ((!kasfThisTeamIs(kasAITeamPtrIfExists(126123491))) || kasfShipsCount(kasAITeamShipsPtrIfExists(68592435))))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
            {
                kasfStop();
                kasfSpeechEvent(91523540, Init_Mission13_HeavyInterceptorWing_Regroup, (sdword)Watch_Mission13_HeavyInterceptorWing_Regroup);
            }
        }
        else
        {
            kasfStop();
            kasfSpeechEvent(143009833, (sdword)sub_652EF0, (sdword)Watch_Mission13_HeavyInterceptorWing_AwaitAttackSignal);
        }
    }
    else
    {
        kasfStop();
        kasfReinforceTeamWithShips(kasAITeamPtrIfExists(239784178), &((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfShipsRemove(&((AITeam *)kasCurrentTeamPtr)->shipList, &((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfSpeechEvent(143009833, (sdword)sub_652EF0, (sdword)Watch_Mission13_HeavyInterceptorWing_AwaitAttackSignal);
    }
}

void Watch_Mission13_HeavyInterceptorWing_AwaitAttackSignal(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    team = (AITeam *)kasCurrentTeamPtr;
    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message == 126123267)
        {
            kasfSpeechEvent(110655924, Init_Mission13_HeavyInterceptorWing_Spawn, (sdword)Watch_Mission13_HeavyInterceptorWing_Spawn);
            return;
        }
        if (message != 0)
        {
            aitQueuePointer(team, team, (void *)message);
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission13_HeavyInterceptorWing_Spawn(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-2046393961))
    {
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }
    kasfShipsCreateInside(
        kasGrowSelectionPtr(164967155),
        "tHeavyInterceptor",
        kasfRandom(4, 8),
        kasAITeamShipsPtrIfExists(123922546),
        1);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(164967155));
    kasAITeamShipsPtr(kasGrowSelectionPtr(164967155), 2);
    kasfEntryIncrement(-2126415161);
}

void Watch_Mission13_HeavyInterceptorWing_Spawn(void)
{
    AITeam *targetTeam;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching() || kasfEntryValueGet(-2027278702))
    {
        if (kasfTeamCount() > 0)
        {
            kasfEntryDecrement(-2126415161);
            if (kasfThisTeamIs(kasAITeamPtrIfExists(126123489)))
            {
                targetTeam = kasAITeamPtrIfExists(68592433);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(126123490)))
            {
                targetTeam = kasAITeamPtrIfExists(68592434);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(126123491)))
            {
                targetTeam = kasAITeamPtrIfExists(68592435);
            }
            else
            {
                kasfSpeechEvent(91523540, Init_Mission13_HeavyInterceptorWing_Regroup, (sdword)Watch_Mission13_HeavyInterceptorWing_Regroup);
                return;
            }
            kasfMsgSend(targetTeam, (char *)70883199);
            kasfSpeechEvent(91523540, Init_Mission13_HeavyInterceptorWing_Regroup, (sdword)Watch_Mission13_HeavyInterceptorWing_Regroup);
        }
    }
}

void Init_Mission13_KamikazeWave(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(317701, (sdword)sub_652EF0, (sdword)Watch_Mission13_KamikazeWave);
}

void Init_Mission13_KamikazeWave_AttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfKamikaze(kasGrowSelectionPtr(0x4A2A29));
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_KamikazeWave(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount() > 0)
    {
        kasfSpeechEvent(74024315, Init_Mission13_KamikazeWave_AttackRun, (sdword)Watch_Mission13_KamikazeWave_AttackRun);
    }
}

static void Watch_Mission13_KamikazeWave_AttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(317701, (sdword)Begin_Mission13_KamikazeWave_ReissueAttackRun, (sdword)sub_652EF0);
        return;
    }
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(74024315, Init_Mission13_KamikazeWave_AttackRun, (sdword)Watch_Mission13_KamikazeWave_AttackRun);
    }
}

static void Begin_Mission13_KamikazeWave_ReissueAttackRun(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount() > 0)
    {
        kasfSpeechEvent(74024315, Init_Mission13_KamikazeWave_AttackRun, (sdword)Watch_Mission13_KamikazeWave_AttackRun);
    }
}

void Init_Mission13_HeavyDefenderTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreate(147349827);
    kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_HeavyDefenderTeam_ReplenishTrigger);
}

void Watch_Mission13_HeavyDefenderTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-2027278702))
    {
        if (kasfShipsAnyInSensorRange(&((AITeam *)kasCurrentTeamPtr)->shipList))
        {
            tutDeallocateRootRegion(kasGrowSelectionPtr(211966048));
        }
        else
        {
            kasfDeleteShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
        }
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Watch_Mission13_HeavyDefenderTeam_ReplenishTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(254235363)) < 12)
    {
        kasfSpeechEvent(237703397, Init_Mission13_HeavyDefenderTeam_Replenish, (sdword)Watch_Mission13_HeavyDefenderTeam_Replenish);
    }
}

void Init_Mission13_HeavyDefenderTeam_Replenish(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(147349827, 20 - kasfShipsCount(kasAITeamShipsPtrIfExists(254235363)));
    if (kasfEntryValueGet(147349827) > 0)
    {
        if (kasfEntryValueGet(-2046393961))
        {
            kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        }
        else
        {
            kasfShipsCreateInside(
                kasGrowSelectionPtr(264778467),
                "tHeavyDefender",
                kasfEntryValueGet(147349827),
                kasAITeamShipsPtrIfExists(123922546),
                1);
            kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(264778467));
            kasAITeamShipsPtr(kasGrowSelectionPtr(264778467), 2);
            kasfGrowSelectionClear(kasGrowSelectionPtr(264778467));
            kasfTacticsAggressive();
            kasfEntryIncrement(-2126415161);
        }
    }
}

void Watch_Mission13_HeavyDefenderTeam_Replenish(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfTeamFinishedLaunching())
        {
            kasfEntryDecrement(-2126415161);
            kasfTacticsEvasive();
            kasfGrowSelectionClear(kasGrowSelectionPtr(264778467));
            kasfShipsAdd(kasGrowSelectionPtr(264778467), &((AITeam *)kasCurrentTeamPtr)->shipList);
            kasfShipsRemove(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr(264778467));
            kasfReinforceTeamWithShips(kasAITeamPtrIfExists(254235363), kasGrowSelectionPtr(264778467));
            kasfGrowSelectionClear(kasGrowSelectionPtr(264778467));
            kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_HeavyDefenderTeam_ReplenishTrigger);
        }
    }
    else
    {
        kasfEntryDecrement(-2126415161);
        kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_HeavyDefenderTeam_ReplenishTrigger);
    }
}

void Watch_Mission13_EventRelay(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;
    AITeam *targetTeam;
    udword relayMessage;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        targetTeam = NULL;
        relayMessage = 0;
        switch (message)
        {
            case 0:
                break;
            case 115286795:
                kasfMsgSend(kasAITeamPtrIfExists(228456241), (char *)115286795);
                kasfMsgSend(kasAITeamPtrIfExists(92316977), (char *)115286795);
                kasfMsgSend(kasAITeamPtrIfExists(228456242), (char *)115286795);
                kasfMsgSend(kasAITeamPtrIfExists(92316978), (char *)115286795);
                kasfMsgSend(kasAITeamPtrIfExists(228456243), (char *)115286795);
                kasfMsgSend(kasAITeamPtrIfExists(92316979), (char *)115286795);
                break;
            case 115287019:
                kasfMsgSend(kasAITeamPtrIfExists(117532788), (char *)115287019);
                kasfMsgSend(kasAITeamPtrIfExists(118271680), (char *)115287019);
                kasfMsgSend(kasAITeamPtrIfExists(212827794), (char *)115287019);
                break;
            case 115287035:
                kasfMsgSend(kasAITeamPtrIfExists(232664177), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(18084417), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(232664178), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(18084418), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(232664179), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(18084419), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(232664180), (char *)115287035);
                kasfMsgSend(kasAITeamPtrIfExists(18084420), (char *)115287035);
                break;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission13_ReinforcementParadeHyperspaceIn(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr(175835620));
    kasfShipsAdd(kasGrowSelectionPtr(175835620), kasAITeamShipsPtrIfExists(226174961));
    kasfShipsAdd(kasGrowSelectionPtr(175835620), kasAITeamShipsPtrIfExists(82596931));
    kasfShipsAdd(kasGrowSelectionPtr(175835620), kasAITeamShipsPtrIfExists(184278179));
    kasfShipsAdd(kasGrowSelectionPtr(175835620), kasAITeamShipsPtrIfExists(219993395));
    kasfShipsAdd(kasGrowSelectionPtr(175835620), kasAITeamShipsPtrIfExists(214312755));
    kasfShipsSetMilitaryParade(kasAITeamShipsPtrIfExists(101191117), kasGrowSelectionPtr(175835620));
    kasfShipsAdd(kasGrowSelectionPtr(175835620), kasAITeamShipsPtrIfExists(101191117));
    kasfShipsHyperspaceIn(kasGrowSelectionPtr(175835620), NULL);
}

void Watch_Mission13_ReinforcementParade_AwaitDepartSignal(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message == 87236965)
        {
            kasfSpeechEvent(81263136, Init_Mission13_ReinforcementParade_Depart, (sdword)Watch_Mission13_ReinforcementParade_Depart);
            return;
        }
        if (message != 0)
        {
            aitQueuePointer(team, team, (void *)message);
        }
        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission13_ReinforcementParade_Depart(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSetHyperspaceDestination(kasGrowSelectionPtr(175835620), kasVectorPtrIfExists(78117408));
    kasTimerCreateSetStartById(27239543, 5);
}

void Watch_Mission13_ReinforcementParade_Depart(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(27239543))
    {
        kasfMsgSend(kasAITeamPtrIfExists(101191117), (char *)223562421);
        kasfMsgSend(kasAITeamPtrIfExists(214312755), (char *)223562421);
        kasfMsgSend(kasAITeamPtrIfExists(226174961), (char *)223562421);
        kasfMsgSend(kasAITeamPtrIfExists(82596931), (char *)223562421);
        kasfMsgSend(kasAITeamPtrIfExists(184278179), (char *)223562421);
        kasfMsgSend(kasAITeamPtrIfExists(219993395), (char *)223562421);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Watch_Mission13_MissionFlow_FailIfFaalcorDestroyed(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552169) && !kasfShipsCount(kasAITeamShipsPtrIfExists(101191117)))
    {
        kasfEntryGetOrCreateSetValue(71552169, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
    }
}

void Init_Mission13_MissionFlow_BriefingSequence(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(5801605, 0);
}

void Watch_Mission13_MissionFlow_DestroyEnemyBriefing(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    switch (kasfEntryValueGet(5801605))
    {
        case 0:
            kasfSpeechEventQueueCurrentPlayer(268440101, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 1:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 2:
            kasfShipsSetPercentDamaged(kasGrowSelectionPtr(211966048), 80);
            kasfEntrySetValue(-2027227947, 1);
            kasfSpeechEventQueueCurrentPlayer(268444198, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 3:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 4:
            kasTimerCreateSetStartById(-1960672491, 11);
            kasfSpeechEventQueueCurrentPlayer(268440103, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 5:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 6:
            kasfSpeechEventQueueCurrentPlayer(268444200, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 7:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 8:
            spEventWideScreenIn(90);
            kasTimerCreateSetStartById(-582129490, 3);
            kasfEntryIncrement(5801605);
            return;
        case 9:
            if (kasTimerExpiredById(-582129490))
            {
                kasfEntryIncrement(5801605);
            }
            break;
        case 10:
            if (!kasfSensorsIsOpen(0))
            {
                kasfOpenSensors(150000);
                kasfEntrySetValue(126773838, 0);
            }
            kasfShipsSelectCapital(kasGrowSelectionPtr(268174972), kasGrowSelectionPtr(126483525));
            kasfPingAddShipsIndexed(kasGrowSelectionPtr(268174972), 0x8CC3E0);
            kasfSpeechEventQueueCurrentPlayer(268440105, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 11:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 12:
            kasfSpeechEventQueueCurrentPlayer(268444202, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 13:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 14:
            kasfWideScreenOut(90);
            kasTimerCreateSetStartById(-734882956, 3);
            kasfEntryIncrement(5801605);
            return;
        case 15:
            if (kasTimerExpiredById(-734882956))
            {
                kasfEntryIncrement(5801605);
            }
            break;
        case 16:
            if (!kasfEntryValueGet(126773838))
            {
                kasfCloseSensors(1);
                kasfEntrySetValue(126773838, 1);
            }
            kasfPingRemove(0x8CC3E0);
            kasfSpeechEventQueueCurrentPlayer(268444203, 0);
            kasfMsgSend(kasAITeamPtrIfExists(187875832), (char *)115287035);
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)24290736);
            kasfEntryIncrement(5801605);
            kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
            return;
        default:
            break;
    }
}

void Watch_Mission13_MissionFlow_ProtectFaalcorBriefing(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    switch (kasfEntryValueGet(5801605))
    {
        case 0:
            kasTimerCreateSetStartById(-582129490, 3);
            kasfWideScreenIn(90);
            kasfEntryIncrement(5801605);
            return;
        case 1:
            if (kasTimerExpiredById(-582129490))
            {
                kasfEntryIncrement(5801605);
            }
            break;
        case 2:
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)87236965);
            kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(101191117));
            kasfCameraSetAngleDeg(36);
            kasfCameraSetDeclinationDeg(2);
            kasfCameraSetDistance(5276);
            kasfSpeechEventQueueCurrentPlayer(268468685, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 3:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 4:
            kasfWideScreenOut(90);
            kasTimerCreateSetStartById(-734882956, 3);
            kasfEntryIncrement(5801605);
            return;
        case 5:
            if (kasTimerExpiredById(-734882956))
            {
                kasfEntryIncrement(5801605);
            }
            break;
        case 6:
            kasfSpeechEventQueueCurrentPlayer(268440122, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 7:
            if (kasfIntelEventEnded())
            {
                kasfEntryIncrement(5801605);
            }
            break;
        case 8:
            kasfSpeechEventQueueCurrentPlayer(268468283, 0);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 9:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 10:
            kasfSpeechEventQueueCurrentPlayer(268440124, 0);
            kasfMsgSendAll((char *)187654963);
            kasfIntelEventNotEnded();
            kasfEntryIncrement(5801605);
            return;
        case 11:
            if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
            {
                kasfEntrySetValue(-2054310428, 0);
                kasfEntryIncrement(5801605);
            }
            break;
        case 12:
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)232458573);
            kasfSpeechEventQueueCurrentPlayer(268444209, 0);
            kasfEntryIncrement(5801605);
            kasfSpeechEvent(244212684, Init_WaitForIntelEventEnd, (sdword)Watch_Mission13_MissionFlow_PostIntelCleanup);
            return;
        default:
            break;
    }
}

void Init_Mission13_EnemyCarrierController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
    kasfSpeechEvent(566284, (sdword)sub_652EF0, (sdword)Watch_Mission13_EnemyCarrierController_AwaitEscortLoss);
}

void Watch_Mission13_EnemyCarrierController(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552170) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552170, 1);
        kasfEntrySetValue(-2027278702, 1);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Watch_Mission13_EnemyCarrierController_AwaitEscortLoss(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(126483525))
     && !kasfShipsCount(kasGrowSelectionPtr(70045477))
     && !kasfShipsCount(kasGrowSelectionPtr(241823333)))
    {
        kasfSpeechEvent(111764788, Init_Mission13_EnemyCarrier_FinalApproach, (sdword)Watch_Mission13_EnemyCarrier_FinalApproach);
    }
}

void Init_Mission13_EnemyCarrier_FinalApproach(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(-2046393961, 1);
    sub_4941A0(kasVectorPtrIfExists(143796334));
}

void Watch_Mission13_EnemyCarrier_FinalApproach(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfNearby(kasVectorPtrIfExists(143796334), 3000))
    {
        sub_499790();
        if (kasfEntryValueGet(-2126415161) <= 0)
        {
            if (!kasfShipsSelectDamaged(kasGrowSelectionPtr(115692564), &((AITeam *)kasCurrentTeamPtr)->shipList, 25))
            {
                kasfAttackMothership();
                kasfEntrySetValue(-2027278702, 1);
                kasfSpeechEvent(187746676, Init_Mission13_FinalAttackTimeout, (sdword)Watch_Mission13_FinalAttackTimeout);
                return;
            }
        }
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Watch_Mission13_FinalAttackTimeout(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(187746676))
    {
        kasfDeleteShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

void Init_Mission13_MinelayerEscortReserve(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfSpeechEvent(317701, (sdword)sub_652EF0, (sdword)Watch_Mission13_MinelayerEscortReserve);
}

void Begin_Mission13_MinelayerEscortReserve_CombatLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfGrowSelectionClear(kasGrowSelectionPtr(92661347));
       kasfShipsAdd(kasGrowSelectionPtr(92661347), kasAITeamShipsPtrIfExists(92661281));
       kasfShipsAdd(kasGrowSelectionPtr(92661347), kasAITeamShipsPtrIfExists(92661282));
       kasfFindShipsTargetingSelection(kasGrowSelectionPtr(92661347), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29));
   }
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(161590357, kasfRandom(5, 20));
}

void Watch_Mission13_MinelayerEscortReserve_CombatLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(161590357);
       kasfSpeechEvent(317701, (sdword)sub_652EF0, (sdword)Watch_Mission13_MinelayerEscortReserve);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(161590357))
   {
       kasTimerDestroyById(161590357);
       kasfSpeechEvent(317701, (sdword)sub_652EF0, (sdword)Watch_Mission13_MinelayerEscortReserve);
   }
}

void Watch_Mission13_MinelayerEscortReserve(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfTeamCount() > 0)
   {
       kasfSpeechEvent(74024315, Begin_Mission13_MinelayerEscortReserve_CombatLoop, (sdword)Watch_Mission13_MinelayerEscortReserve_CombatLoop);
   }
}

void Init_Mission13_MinelayerCorvetteTeam(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfSpeechEvent(208122564, (sdword)sub_617A10, (sdword)Watch_Mission13_MinelayerCorvetteTeam_AwaitLoss);
}

void Watch_Mission13_MinelayerCorvetteTeam(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2027278702))
   {
       kasfSpeechEvent(111764788, Init_Mission13_MinelayerCorvette_FinalApproach, (sdword)Watch_Mission13_MinelayerCorvette_FinalApproach);
   }
}

static void Watch_Mission13_MinelayerCorvetteTeam_AwaitLoss(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(110655924, Init_Mission13_MinelayerCorvetteReinforcement, (sdword)Watch_Mission13_MinelayerCorvetteReinforcement);
   }
}

void Init_Mission13_MinelayerCorvetteReinforcement(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfEntryValueGet(-2044292713))
   {
       kasfShipsCreateInside(
           kasGrowSelectionPtr(164967155),
           "tResourceCollector",
           kasfRandom(3, 5),
           kasAITeamShipsPtrIfExists(123922546),
           1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(164967155), 2);
       kasfEntryIncrement(-2126415161);
       return;
   }
   kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
}

void Watch_Mission13_MinelayerCorvetteReinforcement(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfTeamFinishedLaunching() || kasfEntryValueGet(-2027278702))
   {
       if (kasfTeamCount() > 0)
       {
           kasfEntryDecrement(-2126415161);
           kasfSpeechEvent(208122564, (sdword)sub_617A10, (sdword)Watch_Mission13_MinelayerCorvetteTeam_AwaitLoss);
       }
   }
}

void Init_Mission13_MinelayerCorvette_FinalApproach(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfMoveTo(kasVectorPtrIfExists(143796334));
}

void Watch_Mission13_MinelayerCorvette_FinalApproach(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfNearby(kasVectorPtrIfExists(143796334), 3000))
   {
       sub_499790();
       if (!kasfShipsSelectDamaged(kasGrowSelectionPtr(115692564), &((AITeam *)kasCurrentTeamPtr)->shipList, 25))
       {
           kasfAttackMothership();
           kasfSpeechEvent(187746676, Init_Mission13_FinalAttackTimeout, (sdword)Watch_Mission13_FinalAttackTimeout);
           return;
       }
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
   }
}

void Init_Mission13_FinalAttackTimeout(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerCreateSetStartById(187746676, 8);
}

void Init_Mission13_HeavyDefenderPatrol(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfSpeechEvent(5034340, Init_Mission13_HeavyDefenderPatrol_AttackRun, (sdword)Watch_Mission13_HeavyDefenderPatrol);
}

void Init_Mission13_HeavyDefenderPatrol_AttackRun(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfFormationSphere();
   kasfProtect(kasAITeamShipsPtrIfExists(123922546));
}

void Watch_Mission13_HeavyDefenderPatrol(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2027278702))
   {
       kasfStop();
       kasfSpeechEvent(5520083, Init_Mission13_HeavyDefenderPatrol_DespawnDelay, (sdword)Watch_Mission13_HeavyDefenderPatrol_DespawnDelay);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList))
   {
       kasfStop();
       kasfSpeechEvent(5034340, Init_Mission13_HeavyDefenderPatrol_AttackRun, (sdword)Watch_Mission13_HeavyDefenderPatrol);
   }
}

void Init_Mission13_HeavyDefenderPatrol_DespawnDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerCreateSetStartById(228415362, kasfRandom(5, 15));
}

void Watch_Mission13_HeavyDefenderPatrol_DespawnDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasTimerExpiredById(228415362))
   {
       if (kasfShipsAnyInSensorRange(&((AITeam *)kasCurrentTeamPtr)->shipList))
       {
           tutDeallocateRootRegion(kasGrowSelectionPtr(211966048));
       }
       else
       {
           kasfDeleteShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
       }
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
   }
}

void Init_Mission13_TraderWing(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_TraderWing_AwaitAttackSignal);
}

void Watch_Mission13_TraderWing_AwaitAttackSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 114238219)
       {
           kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderWing_Targets, (sdword)Watch_Mission13_TraderWing_TargetLoop_Reacquire);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Watch_Mission13_TraderWing_TargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfEntryValueGet(71552165) && kasfEntryValueGet(-1914453392))
   {
       kasfEntryGetOrCreateSetValue(71552165, 1);
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderWing_Targets, (sdword)Watch_Mission13_TraderWing_TargetLoop_Reacquire);
       return;
   }
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderWing_Targets, (sdword)Watch_Mission13_TraderWing_TargetLoop_Reacquire);
   }
}

void Init_Mission13_TraderEscort(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfSpeechEvent(374244, (sdword)sub_652EF0, (sdword)Watch_Mission13_TraderEscort_AwaitAttackSignal);
}

void Watch_Mission13_TraderEscort_AwaitAttackSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 114238219)
       {
           kasfSpeechEvent(91523540, Init_Mission13_TraderEscort_MoveToLinkedTeam, (sdword)Watch_Mission13_TraderEscort_AwaitLinkedTeamLoss);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_TraderEscort_MoveToLinkedTeam(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfFormationSphere();
   if (kasfThisTeamIs(kasAITeamPtrIfExists(92316977)))
   {
       kasfProtect(kasAITeamShipsPtrIfExists(228456241));
       return;
   }
   if (kasfThisTeamIs(kasAITeamPtrIfExists(92316978)))
   {
       kasfProtect(kasAITeamShipsPtrIfExists(228456242));
       return;
   }
   if (kasfThisTeamIs(kasAITeamPtrIfExists(92316979)))
   {
       kasfProtect(kasAITeamShipsPtrIfExists(228456243));
   }
}

void Watch_Mission13_TraderEscort_AwaitLinkedTeamLoss(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfStop();
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if ((kasfThisTeamIs(kasAITeamPtrIfExists(92316977)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(228456241))) ||
       (kasfThisTeamIs(kasAITeamPtrIfExists(92316978)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(228456242))) ||
       (kasfThisTeamIs(kasAITeamPtrIfExists(92316979)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(228456243))))
   {
       kasfStop();
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderTargets, (sdword)Watch_Mission13_TraderTargetLoop);
   }
}

void Select_Mission13_TraderTargets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsSelectType(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), "kStandardDestroyer");
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsSelectType(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), "kIonCannonFrigate");
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29));
       kasfShipsRemove(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(211966048));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(211966048));
   }
   kasfFormationBroad();
   kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(57245931), kasThisTeamsVectorPtr());
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_TraderTargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfEntryValueGet(71552166) && kasfEntryValueGet(-1914453392))
   {
       kasfEntryGetOrCreateSetValue(71552166, 1);
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderTargets, (sdword)Watch_Mission13_TraderTargetLoop);
       return;
   }
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderTargets, (sdword)Watch_Mission13_TraderTargetLoop);
   }
}

void Init_Mission13_StrikeWing_AttackMothership(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfTeamAttributesBitSet(128);
   kasfAttackMothership();
   kasfSpeechEvent(80578718, (sdword)sub_652EF0, (sdword)Watch_Mission13_StrikeWing_AwaitAttackSignal);
}

void Watch_Mission13_StrikeWing_AwaitAttackSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 114238443)
       {
           kasfTeamHyperspaceIn(kasVectorPtrIfExists(166619059));
           kasfSpeechEvent(74024315, Init_Mission13_StrikeWing_ReacquireDelay, (sdword)Watch_Mission13_StrikeWing_ReacquireDelay);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_StrikeWing_ReacquireDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   tutDeallocateRootRegion(kasGrowSelectionPtr(211966048));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_StrikeWing_ReacquireDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, Init_Mission13_StrikeWing_ReacquireDelay, (sdword)Watch_Mission13_StrikeWing_ReacquireDelay);
   }
}

void Init_Mission13_StrikeWing2_AttackMothership(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfTeamAttributesBitSet(128);
   kasfAttackMothership();
   kasfSpeechEvent(80578718, (sdword)sub_652EF0, (sdword)Watch_Mission13_StrikeWing2_AwaitAttackSignal);
}

void Watch_Mission13_StrikeWing2_AwaitAttackSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 114238443)
       {
           kasfTeamHyperspaceIn(kasVectorPtrIfExists(166619059));
           kasfSpeechEvent(91523540, Init_Mission13_StrikeWing2_RegroupToLinkedTeam, (sdword)Watch_Mission13_StrikeWing2_AwaitLinkedTeamLoss);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_StrikeWing2_RegroupToLinkedTeam(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfFormationSphere();
   kasfProtect(kasAITeamShipsPtrIfExists(117532788));
}

void Watch_Mission13_StrikeWing2_AwaitLinkedTeamLoss(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfStop();
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (!kasfShipsCount(kasAITeamShipsPtrIfExists(117532788)))
   {
       kasfStop();
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_StrikeWing2_Targets, (sdword)Watch_Mission13_StrikeWing2_TargetLoop);
   }
}

void Select_Mission13_StrikeWing2_Targets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfUnderAttack(kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(211966048));
   }
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_StrikeWing2_TargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_StrikeWing2_Targets, (sdword)Watch_Mission13_StrikeWing2_TargetLoop);
   }
}

void Init_Mission13_StrikeWing3_AttackMothership(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
   kasfTeamAttributesBitSet(128);
   kasfAttackMothership();
   kasfSpeechEvent(80578718, (sdword)sub_652EF0, (sdword)Watch_Mission13_StrikeWing3_AwaitAttackSignal);
}

void Watch_Mission13_StrikeWing3_AwaitAttackSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 114238443)
       {
           kasfTeamHyperspaceIn(kasVectorPtrIfExists(166619059));
           kasfSpeechEvent(74024315, (sdword)Select_Mission13_StrikeWing3_Targets, (sdword)Watch_Mission13_StrikeWing3_TargetLoop);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Select_Mission13_StrikeWing3_Targets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfUnderAttack(kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsSelectNonCapital(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(117532788), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(118271680), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(211966048));
   }
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_StrikeWing3_TargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_StrikeWing3_Targets, (sdword)Watch_Mission13_StrikeWing3_TargetLoop);
   }
}

static void Select_Mission13_TraderWing_Targets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsSelectType(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), "kStandardDestroyer");
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsSelectType(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), "kIonCannonFrigate");
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfEntryValueGet(-1914453392))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29));
       kasfShipsRemove(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(211966048));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && GetGameDifficulty() == 0)
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(211966048));
   }
   kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(57245931), kasThisTeamsVectorPtr());
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

static void Watch_Mission13_TraderWing_TargetLoop_Reacquire(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfEntryValueGet(71552167) && kasfEntryValueGet(-1914453392))
   {
       kasfEntryGetOrCreateSetValue(71552167, 1);
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderWing_Targets, (sdword)Watch_Mission13_TraderWing_TargetLoop_Reacquire);
       return;
   }
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderWing_Targets, (sdword)Watch_Mission13_TraderWing_TargetLoop_Reacquire);
   }
}

void Watch_Mission13_ParadeFollower_AwaitDepartSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 223562421)
       {
           kasfSpeechEvent(244211987, Init_Mission13_ParadeFollower_Depart, (sdword)Watch_Mission13_ParadeFollower_BroadcastAdvanceSignal);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_ParadeFollower_Depart(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfTeamSwitchPlayerOwner(0);
   kasfShipsApplyPlayerUpgrades(&((AITeam *)kasCurrentTeamPtr)->shipList);
   kasfTeamHyperspaceInNear(kasVectorPtrIfExists(78117408), 3000);
   kasTimerCreateSetStartById(139352562, 10);
}

void Watch_Mission13_ParadeFollower_BroadcastAdvanceSignal(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasTimerExpiredById(139352562))
   {
       kasfMsgSend(kasAITeamPtrIfExists(4627185), (char *)84255352);
       kasfMsgSend(kasAITeamPtrIfExists(4627186), (char *)84255352);
       kasfMsgSend(kasAITeamPtrIfExists(28839409), (char *)84255352);
       kasfMsgSend(kasAITeamPtrIfExists(230185987), (char *)84255352);
       kasfSpeechEvent(140229815, (sdword)sub_652EF0, (sdword)sub_652EF0);
   }
}

void Init_Mission13_ParadeFollower_DockOrPassive(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
   if (kasfThisTeamIs(kasAITeamPtrIfExists(230185987)))
   {
       kasfShipsDockInstant(&((AITeam *)kasCurrentTeamPtr)->shipList, kasAITeamShipsPtrIfExists(101191117));
   }
   else
   {
       kasfTeamAttributesBitSet(128);
   }
   kasfSpeechEvent(320211, (sdword)sub_652EF0, (sdword)Watch_Mission13_ParadeFollower_HandleDepartOrAdvanceSignal);
}

void Watch_Mission13_ParadeFollower_HandleDepartOrAdvanceSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 223562421)
       {
           kasfTeamSwitchPlayerOwner(0);
           kasfShipsApplyPlayerUpgrades(&((AITeam *)kasCurrentTeamPtr)->shipList);
           kasfTacticsAggressive();
           if (kasfThisTeamIs(kasAITeamPtrIfExists(82628675)))
           {
               kasfTeamHyperspaceIn(kasVectorPtrIfExists(230523536));
           }
           else if (kasfThisTeamIs(kasAITeamPtrIfExists(184278179)))
           {
               kasfTeamHyperspaceIn(kasVectorPtrIfExists(230436752));
           }
           else if (kasfThisTeamIs(kasAITeamPtrIfExists(219993395)))
           {
               kasfTeamHyperspaceIn(kasVectorPtrIfExists(230527120));
           }
           else
           {
               kasfTeamHyperspaceInNear(kasVectorPtrIfExists(78117408), 3000);
           }
           kasfSpeechEvent(63695746, (sdword)sub_652EF0, (sdword)Watch_Mission13_ParadeFollower_AwaitSupportSignal);
           return;
       }
       if (message == 84255352)
       {
           kasfSpeechEvent(84255352, Init_Mission13_ParadeFollower_DockToFaalcor, (sdword)Watch_Mission13_ParadeFollower_AwaitDockComplete);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_ParadeFollower_DockToFaalcor(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfTeamSwitchPlayerOwner(0);
   kasfShipsApplyPlayerUpgrades(&((AITeam *)kasCurrentTeamPtr)->shipList);
   kasfShipsDockStay(&((AITeam *)kasCurrentTeamPtr)->shipList, kasAITeamShipsPtrIfExists(101191117));
}

void Watch_Mission13_ParadeFollower_AwaitDockComplete(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfTeamFinishedLaunching())
   {
       kasfSpeechEvent(63695746, (sdword)sub_652EF0, (sdword)Watch_Mission13_ParadeFollower_AwaitSupportSignal);
   }
}

void Watch_Mission13_ParadeFollower_AwaitSupportSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 188703027)
       {
           kasfShipsSetSupportFlag(&((AITeam *)kasCurrentTeamPtr)->shipList, 1);
           kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_ParadeFollower(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
   kasfTeamAttributesBitSet(128);
   kasfSpeechEvent(320211, (sdword)sub_652EF0, (sdword)Watch_Mission13_ParadeFollower_AwaitDepartSignal);
}

void Init_Mission13_ParadeWorkerWing(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 0);
    kasfTeamAttributesBitSet(128);
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)Watch_Mission13_ParadeWorkerWing_AwaitDepartSignal);
}

void Watch_Mission13_ParadeWorkerWing_AwaitDepartSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 223562421)
       {
           kasfSpeechEvent(84255352, Init_Mission13_ParadeWorkerWing_Depart, (sdword)Watch_Mission13_ParadeWorkerWing_Depart);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_ParadeWorkerWing_Depart(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfTeamSwitchPlayerOwner(0);
   kasfShipsApplyPlayerUpgrades(&((AITeam *)kasCurrentTeamPtr)->shipList);
   kasfTeamHyperspaceInNear(kasVectorPtrIfExists(78117408), 3000);
   kasTimerCreateSetStartById(27239543, 5);
}

static void Init_Mission13_ParadeWorkerWing_TargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(74072178));
   if (!kasfShipsCount(kasGrowSelectionPtr(74072178)))
   {
       kasfShipsSelectDamaged(kasGrowSelectionPtr(74072178), kasGrowSelectionPtr(211966048), 50);
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(74072178)))
   {
       kasfShipsSelectCapital(kasGrowSelectionPtr(74072178), kasGrowSelectionPtr(0x4A2A29));
       kasfShipsSelectDamaged(kasGrowSelectionPtr(74072178), kasGrowSelectionPtr(74072178), 50);
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(74072178)))
   {
       kasfShipsSelectCapital(kasGrowSelectionPtr(74072178), kasGrowSelectionPtr(0x4A2A29));
   }
   kasfCloak(kasGrowSelectionPtr(74072178));
   kasTimerCreateSetStartById(161590357, kasfRandom(5, 20));
}

void Watch_Mission13_ParadeWorkerWing_Depart(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(161590357);
       if (GetGameDifficulty() == 3)
       {
           kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       }
       else
       {
           kasfSpeechEvent(110655924, Init_Mission13_ParadeWorkerWing_SpawnWorkers, (sdword)Watch_Mission13_ParadeWorkerWing_AwaitCombatDone);
       }
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(161590357))
   {
       kasTimerDestroyById(161590357);
       kasfSpeechEvent(90850786, (sdword)Init_Mission13_ParadeWorkerWing_TargetLoop, (sdword)Watch_Mission13_ParadeWorkerWing_Depart);
   }
}

void Init_Mission13_ParadeWorkerWing_SpawnWorkers(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfShipsCreateInside(
       kasGrowSelectionPtr(164967155),
       "sWorker",
       kasfRandom(2, 5),
       kasAITeamShipsPtrIfExists(101191117),
       0);
   kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(164967155));
   kasAITeamShipsPtr(kasGrowSelectionPtr(164967155), 0);
}

static void Init_Mission13_ParadeWorkerWing_SpawnAcolytes(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfShipsCreateInside(
       kasGrowSelectionPtr(164967155),
       "sAcolyte",
       kasfRandom(6, 10),
       kasAITeamShipsPtrIfExists(101191117),
       0);
   kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, kasGrowSelectionPtr(164967155));
   kasAITeamShipsPtr(kasGrowSelectionPtr(164967155), 0);
}

void Init_Mission13_ParadeWorkerWing_DockToFaalcor(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
   kasfEntryGetOrCreateSetValue(103894148, 0);
   kasfShipsDockInstant(&((AITeam *)kasCurrentTeamPtr)->shipList, kasAITeamShipsPtrIfExists(101191117));
   kasfSpeechEvent(320211, (sdword)sub_652EF0, (sdword)Watch_Mission13_ParadeWorkerWing_AwaitAdvanceSignal);
}

void Watch_Mission13_ParadeWorkerWing_AwaitAdvanceSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 84255352)
       {
           kasfSpeechEvent(84255352, Init_Mission13_ParadeFollower_DockToFaalcor, (sdword)Watch_Mission13_ParadeWorkerWing_AwaitCombatDone);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Watch_Mission13_ParadeWorkerWing_AwaitCombatDone(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfTeamFinishedLaunching())
   {
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_ParadeWorkerWing_Targets, (sdword)Watch_Mission13_ParadeWorkerWing_TargetLoop);
   }
}

void Select_Mission13_ParadeWorkerWing_Targets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfRandom(1, 3) == 1)
   {
       kasfUnderAttack(kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(101191117), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfFindShipsTargetingSelection(kasGrowSelectionPtr(211966048), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfRandom(1, 3) == 1)
   {
       kasfShipsSelectNonCapital(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(114961833));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsSelectCapital(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(114961833));
       kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(92661313));
       kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(92661313));
       if (!kasfShipsAnyInSensorRange(kasAITeamShipsPtrIfExists(123922546)))
       {
           kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(123922546));
       }
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(114961833));
   }
   kasfFormationDelta();
   kasfShipsRemove(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(175835620));
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_ParadeWorkerWing_TargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       if (kasfEntryValueGet(103894148) < 3)
       {
           kasfEntryIncrement(103894148);
           kasfSpeechEvent(110655924, (sdword)Init_Mission13_ParadeWorkerWing_SpawnAcolytes, (sdword)Watch_Mission13_ParadeWorkerWing_AwaitCombatDone);
       }
       else
       {
           kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       }
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_ParadeWorkerWing_Targets, (sdword)Watch_Mission13_ParadeWorkerWing_TargetLoop);
   }
}

void Watch_Mission13_ParadeReserveWing_AwaitDepartSignal(void)
{
   AITeam *team;
   sdword *queueHead;
   sdword queueIndex;
   udword message;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;

   queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
   queueIndex = (queueHead != NULL) ? *queueHead : -1;
   while (queueIndex >= 0)
   {
       message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
       if (message == 223562421)
       {
           kasfTeamSwitchPlayerOwner(0);
           kasfShipsApplyPlayerUpgrades(&((AITeam *)kasCurrentTeamPtr)->shipList);
           kasfSpeechEvent(74024315, (sdword)Select_Mission13_ParadeReserveWing_Targets, (sdword)Watch_Mission13_ParadeReserveWing_TargetLoop);
           return;
       }
       if (message != 0)
       {
           aitQueuePointer(team, team, (void *)message);
       }
       queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
       if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
       {
           break;
       }
   }
}

void Init_Mission13_ParadeReserveWing(void)
{
   kasCurrentStep = 1;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 0);
   kasfTeamAttributesBitSet(128);
   kasfSpeechEvent(320211, (sdword)sub_652EF0, (sdword)Watch_Mission13_ParadeReserveWing_AwaitDepartSignal);
}

void Select_Mission13_ParadeReserveWing_Targets(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfGrowSelectionClear(kasGrowSelectionPtr(57245931));
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)) && kasfRandom(1, 3) == 1)
   {
       kasfUnderAttack(kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(101191117), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfFindShipsTargetingSelection(kasGrowSelectionPtr(211966048), kasGrowSelectionPtr(57245931));
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsSelectCapital(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(114961833));
       kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(92661313));
       kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(92661313));
       if (!kasfShipsAnyInSensorRange(kasAITeamShipsPtrIfExists(123922546)))
       {
           kasfShipsRemove(kasGrowSelectionPtr(57245931), kasAITeamShipsPtrIfExists(123922546));
       }
   }
   if (!kasfShipsCount(kasGrowSelectionPtr(57245931)))
   {
       kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(114961833));
   }
   kasfShipsRemove(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(175835620));
   tutDeallocateRootRegion(kasGrowSelectionPtr(57245931));
   kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}

void Watch_Mission13_ParadeReserveWing_TargetLoop(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
       return;
   }
   if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
   {
       kasTimerDestroyById(141340587);
       kasfSpeechEvent(74024315, (sdword)Select_Mission13_ParadeReserveWing_Targets, (sdword)Watch_Mission13_ParadeReserveWing_TargetLoop);
   }
}

void Watch_Mission13_StrikeWingShared_AwaitAttackSignal(void)
{
    AITeam *team;
    sdword *queueHead;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueHead != NULL) ? *queueHead : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message == 114238459)
        {
            if (kasfThisTeamIs(kasAITeamPtrIfExists(18084417)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217196307), 1000);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(18084418)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217195539), 1000);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(18084419)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217195795), 1000);
            }
            else if (kasfThisTeamIs(kasAITeamPtrIfExists(18084420)))
            {
                kasfTeamHyperspaceInNear(kasVectorPtrIfExists(217197075), 1000);
            }
            kasfSpeechEvent(91523540, Init_Mission13_StrikeWingShared_RegroupToLinkedTeam, (sdword)Watch_Mission13_StrikeWingShared_AwaitLinkedTeamLoss);
            return;
        }
        if (message != 0)
        {
            aitQueuePointer(team, team, (void *)message);
        }
        queueHead = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueHead == NULL || *queueHead == 0 || --queueIndex < 0)
        {
            break;
        }
    }
}

void Init_Mission13_StrikeWingShared_RegroupToLinkedTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationSphere();
    if (kasfThisTeamIs(kasAITeamPtrIfExists(18084417)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(232664177));
        return;
    }
    if (kasfThisTeamIs(kasAITeamPtrIfExists(18084418)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(232664178));
        return;
    }
    if (kasfThisTeamIs(kasAITeamPtrIfExists(18084419)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(232664179));
        return;
    }
    if (kasfThisTeamIs(kasAITeamPtrIfExists(18084420)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(232664180));
    }
}

void Watch_Mission13_StrikeWingShared_AwaitLinkedTeamLoss(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfStop();
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }
    if ((kasfThisTeamIs(kasAITeamPtrIfExists(18084417)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(232664177))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(18084418)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(232664178))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(18084419)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(232664179))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(18084420)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(232664180))))
    {
        kasfStop();
        kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderTargets, (sdword)Watch_Mission13_StrikeWingShared_TargetLoop);
    }
}

void Watch_Mission13_StrikeWingShared_TargetLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552168) && kasfEntryValueGet(-1914453392))
    {
        kasfEntryGetOrCreateSetValue(71552168, 1);
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderTargets, (sdword)Watch_Mission13_StrikeWingShared_TargetLoop);
        return;
    }
    if (!kasfTeamCount())
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }
    if (kasfShipsAnyIdle(&((AITeam *)kasCurrentTeamPtr)->shipList) || kasTimerExpiredDestroyById(141340587))
    {
        kasTimerDestroyById(141340587);
        kasfSpeechEvent(74024315, (sdword)Select_Mission13_TraderTargets, (sdword)Watch_Mission13_StrikeWingShared_TargetLoop);
    }
}

void Init_Mission13_StrikeWingShared_PruneAndAttackMothership(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    seekdir();
    if (GetGameDifficulty() == 0)
    {
        kasfDeleteShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
    }
    else if (GetGameDifficulty() == 1)
    {
        kasfShipsSelectFirstN(kasGrowSelectionPtr(155457129), &((AITeam *)kasCurrentTeamPtr)->shipList, 2);
        kasfDeleteShips(kasGrowSelectionPtr(155457129));
    }
    if (kasfTeamCount() > 0)
    {
        kasAITeamShipsPtr(&((AITeam *)kasCurrentTeamPtr)->shipList, 2);
        kasfTeamAttributesBitSet(128);
        kasfAttackMothership();
        kasfSpeechEvent(80578718, (sdword)sub_652EF0, (sdword)Watch_Mission13_StrikeWingShared_AwaitAttackSignal);
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}

/*-----------------------------------------------------------------------------
    Init_Mission13 - top-level KAS entry point (IDA: 0x629a90)
-----------------------------------------------------------------------------*/

void Init_Mission13(void)
{
    kasCurrentStep = 0;
    scope = KH_mission13;
    kasfDisableMothershipBigGunLaunch();

    kasfEntryGetOrCreateSetValue(-2027278702, 0);
    kasfEntryGetOrCreateSetValue(-2027227947, 0);
    kasfEntryGetOrCreateSetValue(-2144628077, 9999);
    kasfEntryGetOrCreateSetValue(-2051875389, 0);
    kasfEntryGetOrCreateSetValue(-1914453392, 0);
    kasfEntryGetOrCreateSetValue(-1999086536, 30);
    kasfEntryGetOrCreateSetValue(-1948554199, 0);
    kasfEntryGetOrCreateSetValue(-2046393961, 0);
    kasfEntryGetOrCreateSetValue(-2126415161, 0);

    kasfCameraSetAngleDeg(49);
    kasfCameraSetDeclinationDeg(-18);
    kasfCameraSetDistance(3451);
    kasfLocationCard(8000, "Location Unknown");

    switch (GetGameDifficulty())
    {
        case 0: kasfEntrySetValue(-2144628077, 1); break;
        case 1: kasfEntrySetValue(-2144628077, 9); break;
        case 2:
        case 3:
        case 4: kasfEntrySetValue(-2144628077, 9999); break;
        default: break;
    }
    switch (GetGameDifficulty())
    {
        case 0: kasfEntrySetValue(-1999086536, 60); break;
        case 1: kasfEntrySetValue(-1999086536, 40); break;
        case 2: kasfEntrySetValue(-1999086536, 30); break;
        case 3: kasfEntrySetValue(-1999086536, 25); break;
        case 4: kasfEntrySetValue(-1999086536, 15); break;
        default: break;
    }

    kasFSMCreate(KH_FleetIntel, Init_Mission13_MissionFlow, Watch_Mission13_MissionFlow_FailIfFaalcorDestroyed,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Init_Mission13_PrimaryObjectiveController, Watch_Mission13_PrimaryObjectiveController,
                 kasAITeamPtrIfExists(KH_Objectives));

    kasFSMCreate(95444435, Init_Mission13_ParadeWorkerWing_DockToFaalcor, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(4594417));
    kasFSMCreate(95444435, Init_Mission13_ParadeWorkerWing_DockToFaalcor, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(4594418));
    kasFSMCreate(95444435, Init_Mission13_ParadeWorkerWing_DockToFaalcor, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(28839409));

    kasFSMCreate(248954643, Init_Mission13_ParadeReserveWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(226174961));

    kasFSMCreate(226188099, Init_Mission13_ParadeFollower_DockOrPassive, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(82596931));
    kasFSMCreate(226188099, Init_Mission13_ParadeFollower_DockOrPassive, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(184278179));
    kasFSMCreate(226188099, Init_Mission13_ParadeFollower_DockOrPassive, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(230185987));
    kasFSMCreate(226188099, Init_Mission13_ParadeFollower_DockOrPassive, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(219993395));

    kasFSMCreate(214312755, Init_Mission13_ParadeWorkerWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(214312755));
    kasFSMCreate(101191117, Init_Mission13_ParadeFollower, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(101191117));

    kasFSMCreate(175835620, Init_Mission13_ReinforcementParadeHyperspaceIn, Watch_Mission13_ReinforcementParade_AwaitDepartSignal,
                 kasAITeamPtrIfExists(175835620));

    kasFSMCreate(124218850, Init_Mission13_EnemyCarrierController, Watch_Mission13_EnemyCarrierController,
                 kasAITeamPtrIfExists(123922546));

    kasFSMCreate(77795571, Init_Mission13_MinelayerCorvetteTeam, Watch_Mission13_MinelayerCorvetteTeam,
                 kasAITeamPtrIfExists(92661313));
    kasFSMCreate(77795571, Init_Mission13_MinelayerCorvetteTeam, Watch_Mission13_MinelayerCorvetteTeam,
                 kasAITeamPtrIfExists(92661314));

    kasFSMCreate(178438883, Init_Mission13_HeavyDefenderPatrol, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(254235363));
    kasFSMCreate(51117410, Init_Mission13_HeavyDefenderTeam, Watch_Mission13_HeavyDefenderTeam,
                 kasAITeamPtrIfExists(51280114));

    kasFSMCreate(121727539, Init_Mission13_AttackBomberWing, Watch_Mission13_AttackBomberWing,
                 kasAITeamPtrIfExists(68592433));
    kasFSMCreate(121727539, Init_Mission13_AttackBomberWing, Watch_Mission13_AttackBomberWing,
                 kasAITeamPtrIfExists(68592434));
    kasFSMCreate(121727539, Init_Mission13_AttackBomberWing, Watch_Mission13_AttackBomberWing,
                 kasAITeamPtrIfExists(68592435));

    kasFSMCreate(221719987, Init_Mission13_HeavyInterceptorWing, Watch_Mission13_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(126123489));
    kasFSMCreate(221719987, Init_Mission13_HeavyInterceptorWing, Watch_Mission13_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(126123490));
    kasFSMCreate(221719987, Init_Mission13_HeavyInterceptorWing, Watch_Mission13_HeavyInterceptorWing,
                 kasAITeamPtrIfExists(126123491));

    kasFSMCreate(239784178, Init_Mission13_MinelayerEscortReserve, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(239784178));
    kasFSMCreate(105229077, Init_Mission13_KamikazeWave, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(105229077));

    kasFSMCreate(187875832, Watch_NoOpTeamContext, Watch_Mission13_EventRelay,
                 kasAITeamPtrIfExists(187875832));

    kasFSMCreate(206543918, Init_Mission13_TraderWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(228456241));
    kasFSMCreate(81096334, Init_Mission13_TraderEscort, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(92316977));
    kasFSMCreate(206543918, Init_Mission13_TraderWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(228456242));
    kasFSMCreate(81096334, Init_Mission13_TraderEscort, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(92316978));
    kasFSMCreate(206543918, Init_Mission13_TraderWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(228456243));
    kasFSMCreate(81096334, Init_Mission13_TraderEscort, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(92316979));

    kasfGrowSelectionClear(kasGrowSelectionPtr(126483525));
    kasfShipsAdd(kasGrowSelectionPtr(126483525), kasAITeamShipsPtrIfExists(228456241));
    kasfShipsAdd(kasGrowSelectionPtr(126483525), kasAITeamShipsPtrIfExists(92316977));
    kasfShipsAdd(kasGrowSelectionPtr(126483525), kasAITeamShipsPtrIfExists(228456242));
    kasfShipsAdd(kasGrowSelectionPtr(126483525), kasAITeamShipsPtrIfExists(92316978));
    kasfShipsAdd(kasGrowSelectionPtr(126483525), kasAITeamShipsPtrIfExists(228456243));
    kasfShipsAdd(kasGrowSelectionPtr(126483525), kasAITeamShipsPtrIfExists(92316979));

    kasFSMCreate(31318722, Init_Mission13_StrikeWingShared_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(232664177));
    kasFSMCreate(34684706, Init_Mission13_StrikeWingShared_PruneAndAttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(18084417));
    kasFSMCreate(31318722, Init_Mission13_StrikeWingShared_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(232664178));
    kasFSMCreate(34684706, Init_Mission13_StrikeWingShared_PruneAndAttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(18084418));
    kasFSMCreate(31318722, Init_Mission13_StrikeWingShared_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(232664179));
    kasFSMCreate(34684706, Init_Mission13_StrikeWingShared_PruneAndAttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(18084419));
    kasFSMCreate(31318722, Init_Mission13_StrikeWingShared_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(232664180));
    kasFSMCreate(34684706, Init_Mission13_StrikeWingShared_PruneAndAttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(18084420));

    kasfGrowSelectionClear(kasGrowSelectionPtr(70045477));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(232664177));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(18084417));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(232664178));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(18084418));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(232664179));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(18084419));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(232664180));
    kasfShipsAdd(kasGrowSelectionPtr(70045477), kasAITeamShipsPtrIfExists(18084420));

    kasFSMCreate(117532788, Init_Mission13_StrikeWing_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(117532788));
    kasFSMCreate(118271680, Init_Mission13_StrikeWing2_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(118271680));
    kasFSMCreate(212827794, Init_Mission13_StrikeWing3_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(212827794));

    kasfGrowSelectionClear(kasGrowSelectionPtr(241823333));
    kasfShipsAdd(kasGrowSelectionPtr(241823333), kasAITeamShipsPtrIfExists(117532788));
    kasfShipsAdd(kasGrowSelectionPtr(241823333), kasAITeamShipsPtrIfExists(118271680));
    kasfShipsAdd(kasGrowSelectionPtr(241823333), kasAITeamShipsPtrIfExists(212827794));

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
    kasfModuleSlotOrderSetShipType(1, "tHeavyDefender");
    kasfModuleSlotOrderSetShipType(1, "tAttackBomber");
    kasfModuleSlotOrderSetShipType(1, "tResourceCollector");
    kasfModuleSlotOrderSetShipType(1, "tIonCannonFrigate");
    kasfModuleSlotOrderSetCount(1, 3);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeEnemyCombatShips(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeInactivePlayerModules(kasGrowSelectionPtr(KH_Enemy), kasGrowSelectionPtr(18336307));

    kasfFindEnemyMothership(kasGrowSelectionPtr(211966048));
    kasfShipsSelectAllAttachedShips(kasGrowSelectionPtr(88483763), kasGrowSelectionPtr(211966048));

    if (kasfShipsSelectType(kasGrowSelectionPtr(266741017), kasGrowSelectionPtr(88483763), "sMothershipDockingBay"))
    {
        kasfShipsSetPercentDamaged(kasGrowSelectionPtr(266741017), 5);
        kasfShipsSetDontApplyPhysicsFlag(kasGrowSelectionPtr(266741017), 1);
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(59117605), kasGrowSelectionPtr(88483763), "sMothershipMicro"))
    {
        kasfDeleteShips(kasGrowSelectionPtr(59117605));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(59117605), kasGrowSelectionPtr(88483763), "sMothershipArmour"))
    {
        kasfShipsSetPercentDamaged(kasGrowSelectionPtr(59117605), kasfRandom(5, 20));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(59117605), kasGrowSelectionPtr(88483763), "sMothershipSpecial"))
    {
        kasfShipsSetPercentDamaged(kasGrowSelectionPtr(59117605), kasfRandom(5, 20));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(29806117), kasGrowSelectionPtr(88483763), "sMothershipSupport"))
    {
        kasfShipsSelectIndex(kasGrowSelectionPtr(29806117), kasGrowSelectionPtr(29806117), 0);
        kasfShipsSetPercentDamaged(kasGrowSelectionPtr(29806117), 5);
    }

    kasfShipsAttributesBitSet(kasGrowSelectionPtr(211966048), 1024);
    kasfShipsAttributesBitSet(kasGrowSelectionPtr(211966048), 0x80000);
    kasfShipsSetPercentDamaged(kasGrowSelectionPtr(211966048), 20);

    kasfDisablePlayerHyperspace();
    kasfMusicPlay(12);
    kasTimerCreateSetStartById(50796994, 1);
}
