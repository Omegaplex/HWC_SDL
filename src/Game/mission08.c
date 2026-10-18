/*=============================================================================
    Name    : mission08.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission08
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m08_saveInfoName[] =
{
    "Deep Space (Koreth's Rift Sector)",
    "French version goes here",
    "German version goes here",
};

static char *m08_fleetIntelTriggerMsg = (char *)0x4E396F;

void Init_Mission08_Events_IndeedSpawnWorkers(void);
void Watch_Mission08_Events_IndeedSpawnWorkersLaunch(void);
void Watch_Mission08_Events_IndeedDepart(void);
void Watch_Mission07_Events_PreparingAmbassador(void);
void Init_Mission08_Events_WeCanLeaveSequence(void);
void Watch_Mission08_Events_WeCanLeaveCombat(void);
void Watch_Mission08_Events_WeCanLeave(void);
void Watch_Mission08_PreAttackPod(void);
void Watch_Mission08_HiveFrigateReinforcementTeam(void);
void Watch_Mission08_HiveFrigateReinforcementBuildCount(void);
void Watch_Mission08_HiveFrigateReinforcementSpawn(void);
void Init_Mission08_MissionFlow(void);
void Watch_Mission08_MissionFlow_PostIntelCleanup(void);
void Watch_Mission08_MissionFlow_Delay(void);
void Watch_Mission08_MissionFlow(void);
void Watch_Mission08_GuardGravWells_AttackMothership(void);
void Init_Mission08_GuardGravWells_HuntNearbyTargets(void);
void Watch_Mission08_GuardGravWells_HuntNearbyTargets(void);
void Init_Mission08_SpyTeam_AttackMothership(void);
void Watch_Mission08_SpyTeam_AttackMothership(void);
void Init_Mission08_SpyTeam_InterceptTargetGroup(void);
void Init_Mission08_FrigateGroup2_AttackMothership(void);
void Watch_Mission08_FrigateGroup2_BreakAttachmentsAndAttack(void);
void Init_Mission08_FrigateGroup3_AttackMothership(void);
void Watch_Mission08_FrigateGroup3_BreakAttachmentsAndAttack(void);
void Init_Mission08_FrigateGroup_AttackMothership(void);
void Watch_Mission08_FrigateGroup_BreakAttachmentsAndAttack(void);
void Watch_Mission08_FrigateGroup_HyperspaceInAndFormOnResourceCollectorGroup(void);
void Init_Mission08_FrigateGroup_FormOnResourceCollectorGroup(void);
void Watch_Mission08_FrigateGroup_FormOnResourceCollectorGroup(void);
void Init_Mission08_FrigateGroup_HuntNearbyTargets(void);
void Watch_Mission08_FrigateGroup_HuntNearbyTargets(void);
void Init_Mission08_ResourceCollectorGroup_AttackMothership(void);
void Watch_Mission08_ResourceCollectorGroup_BreakAttachmentsAndSignalFrigates(void);
void Init_Mission08_CrystalHarvesterTeam(void);
void Watch_Mission08_CrystalHarvesterTeam_CheckNearbyTargets(void);
void Init_CruiseMissileStrikeWave_RetryDelay(void);
void Watch_CruiseMissileStrikeWave_RetryDelay(void);
void Select_CruiseMissileStrikeTargets_Refresh(void);
void Begin_CruiseMissileStrikeWave(void);
void Watch_CruiseMissileStrikeWave(void);
void Begin_HiveFrigateReinforcementBuildTimer(void);
void Begin_HiveFrigateCombatLoop(void);
void Init_AttackMothershipDeleteDelay(void);
void Begin_BeastAcolyteWave(void);
void Watch_BeastAcolyteWave(void);
void Begin_BeastReconWave(void);
void Watch_BeastReconWave(void);
void Begin_Mission08_DockedAttackTeam_ImmediateAttack(void);
void Watch_Mission08_DockedAttackTeam_StageOrAttack(void);
void Init_Mission08_FrigateGroup3_FormOnResourceCollectorGroup(void);
void Watch_Mission08_FrigateGroup3_FormOnResourceCollectorGroup(void);
void Init_Mission08_FrigateGroup3_AttackClosestResourceCollectorThreat(void);
void Watch_Mission08_FrigateGroup3_AttackClosestResourceCollectorThreat(void);
void Watch_BeastAcolyteWave_Regroup(void);
void Watch_BeastReconWave_Regroup(void);

void Watch_Mission08(void)
{
  int v0; // eax
  int *v1; // eax
  int *v2; // eax
  int *v3; // eax
  int *v4; // eax
  int v5; // eax
  int *v6; // eax
  int *v7; // eax
  int v8; // eax
  int *v9; // eax
  int *v10; // eax
  int v11; // eax
  int *v12; // eax
  int *v13; // eax
  int v14; // eax
  int *v15; // eax
  int v16; // eax
  int v17; // eax
  int *v18; // eax
  int v19; // eax
  int *v20; // eax
  int *v21; // eax
  int *v22; // eax
  int *v23; // eax
  int v24; // eax
  int v25; // eax
  int v26; // eax
  int v27; // eax
  int v28; // esi
  int *v29; // eax
  int *v30; // eax
  int v31; // eax
  int v32; // eax
  int *v33; // eax
  int *v34; // eax
  int v35; // eax
  int *v36; // eax
  int v37; // eax
  int v38; // esi
  int v39; // eax
  int v40; // esi
  int v41; // eax
  int v42; // eax
  int *v43; // eax
  int *v44; // eax
  int *v45; // eax
  int v46; // eax
  int v47; // esi
  int v48; // eax
  int v49; // esi
  int v50; // eax
  int v51; // esi
  int *v52; // eax
  int *v53; // eax
  int v54; // eax
  int v55; // eax
  int *v56; // eax
  int *v57; // eax
  int *v58; // eax
  int v59; // eax
  int *v60; // eax
  int *v61; // eax
  int *v62; // eax
  int v63; // eax
  int *v64; // eax
  int *v65; // eax
  int v66; // eax
  int *v67; // eax
  int *v68; // eax
  int v69; // eax
  int *v70; // eax
  int v71; // eax
  int v72; // eax
  int *v73; // eax
  int *v75; // eax
  int v76; // [esp-Ch] [ebp-Ch]
  int v77; // [esp-Ch] [ebp-Ch]
  int v78; // [esp-Ch] [ebp-Ch]
  int v79; // [esp-Ch] [ebp-Ch]
  int v80; // [esp-Ch] [ebp-Ch]
  int v81; // [esp-Ch] [ebp-Ch]
  int v82; // [esp-Ch] [ebp-Ch]
  int v83; // [esp-Ch] [ebp-Ch]
  int v84; // [esp-Ch] [ebp-Ch]
  int v85; // [esp-8h] [ebp-8h]
  int v86; // [esp-8h] [ebp-8h]
  int v87; // [esp-8h] [ebp-8h]
  int v88; // [esp-8h] [ebp-8h]
  int v89; // [esp-8h] [ebp-8h]
  int v90; // [esp-8h] [ebp-8h]
  int *v91; // [esp-8h] [ebp-8h]
  int *v92; // [esp-8h] [ebp-8h]
  int *v93; // [esp-8h] [ebp-8h]
  int *v94; // [esp-8h] [ebp-8h]
  int v95; // [esp-8h] [ebp-8h]
  int v96; // [esp-8h] [ebp-8h]
  int v97; // [esp-8h] [ebp-8h]
  int v98; // [esp-8h] [ebp-8h]
  int v99; // [esp-4h] [ebp-4h]
  int v100; // [esp-4h] [ebp-4h]
  int v101; // [esp-4h] [ebp-4h]
  int *v102; // [esp-4h] [ebp-4h]
  int v103; // [esp-4h] [ebp-4h]
  int *v104; // [esp-4h] [ebp-4h]
  int v105; // [esp-4h] [ebp-4h]
  int *v106; // [esp-4h] [ebp-4h]
  int v107; // [esp-4h] [ebp-4h]
  int v108; // [esp-4h] [ebp-4h]
  int v109; // [esp-4h] [ebp-4h]
  int v110; // [esp-4h] [ebp-4h]
  int *v111; // [esp-4h] [ebp-4h]
  int v112; // [esp-4h] [ebp-4h]
  int *v113; // [esp-4h] [ebp-4h]
  int v114; // [esp-4h] [ebp-4h]
  int *v115; // [esp-4h] [ebp-4h]
  int *v116; // [esp-4h] [ebp-4h]
  int *v117; // [esp-4h] [ebp-4h]
  int v118; // [esp-4h] [ebp-4h]
  int v119; // [esp-4h] [ebp-4h]
  int *v120; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503672;
  v99 = kasGrowSelectionPtr(18336307);
  v0 = kasVolumePtr(132933767);
  kasfFindShipsInside(v0, v99);
  v100 = kasGrowSelectionPtr(18336307);
  v1 = (int *)kasGrowSelectionPtr(114961833);
  kasSelectEnemyShips(v1, v100);
  v101 = kasGrowSelectionPtr(18336307);
  v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
  kasSelectPlayerShips(v2, v101);
  v3 = (int *)kasGrowSelectionPtr(250623097);
  kasfGrowSelectionClear(v3);
  v102 = (int *)kasGrowSelectionPtr(0x4A2A29);
  v4 = (int *)kasGrowSelectionPtr(250623097);
  kasfShipsAdd(v4, v102);
  v103 = kasGrowSelectionPtr(218060896);
  v5 = kasGrowSelectionPtr(19430160);
  kasfShipsSelectLinkedShips(v5, v103);
  v85 = kasGrowSelectionPtr(19430160);
  v6 = (int *)kasGrowSelectionPtr(19430160);
  if ( kasfShipsSelectRace(v6, v85, 3) )
  {
    v104 = (int *)kasGrowSelectionPtr(19430160);
    v7 = (int *)kasGrowSelectionPtr(250623097);
    kasfShipsAdd(v7, v104);
  }
  v105 = kasGrowSelectionPtr(76514292);
  v8 = kasGrowSelectionPtr(250623097);
  kasfShipsRemove(v8, v105);
  v9 = (int *)kasGrowSelectionPtr(125683657);
  kasfGrowSelectionClear(v9);
  v106 = (int *)kasGrowSelectionPtr(18336307);
  v10 = (int *)kasGrowSelectionPtr(125683657);
  kasfShipsAdd(v10, v106);
  v107 = kasGrowSelectionPtr(250623097);
  v11 = kasGrowSelectionPtr(125683657);
  kasfShipsRemove(v11, v107);
  if ( !kasfEntryValueGet(71552166) && kasfEntryValueGet(-2015429404) && kasfIntelEventEnded() )
  {
    kasfEntrySetValue(71552166, 1);
    v108 = kasGrowSelectionPtr(18336307);
    v12 = (int *)kasGrowSelectionPtr(89157026);
    kasSelectPlayerShipsSimple(v12, v108);
    v109 = kasGrowSelectionPtr(89157026);
    v13 = (int *)kasGrowSelectionPtr(89157026);
    kasfShipsSelectNotSalvagers(v13, v109);
    v110 = kasGrowSelectionPtr(218060896);
    v14 = kasGrowSelectionPtr(82017669);
    if ( kasfShipsSelectLinkedShips(v14, v110) )
    {
      v111 = (int *)kasGrowSelectionPtr(82017669);
      v15 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsAdd(v15, v111);
    }
    if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
    {
      v86 = kasGrowSelectionPtr(89157026);
      v16 = kasGrowSelectionPtr(96759714);
      if ( !kasfShipsSelectType(v16, v86, "sWorker") )
      {
        v87 = kasGrowSelectionPtr(218060896);
        v17 = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(v17, "sWorker", 1, v87, 0);
      }
    }
    v88 = kasGrowSelectionPtr(0x4A2A29);
    v18 = (int *)kasGrowSelectionPtr(76514292);
    kasfShipsSelectRace(v18, v88, 3);
    v112 = kasGrowSelectionPtr(218060896);
    v19 = kasGrowSelectionPtr(19430160);
    kasfShipsSelectLinkedShips(v19, v112);
    v89 = kasGrowSelectionPtr(19430160);
    v20 = (int *)kasGrowSelectionPtr(19430160);
    if ( kasfShipsSelectRace(v20, v89, 3) )
    {
      v113 = (int *)kasGrowSelectionPtr(19430160);
      v21 = (int *)kasGrowSelectionPtr(76514292);
      kasfShipsAdd(v21, v113);
    }
    kasTimerSetById(22617, 180);
    v22 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)v22, m08_fleetIntelTriggerMsg);
    spSetSinglePlayerSaveInfo(8, m08_saveInfoName[strCurLanguage]);
  }
  if ( !kasfEntryValueGet(71552167) && (kasfRUsGet(0) == 4000 || kasTimerExpiredById(22617)) )
  {
    kasfEntrySetValue(71552167, 1);
    v23 = kasAITeamPtrIfExists(22617);
    kasfMsgSend((struct AITeam *)(int)v23, (char *)5801588);
  }
  if ( !kasfEntryValueGet(-2045714667) && kasfEntryValueGet(-2104338061) )
  {
    v76 = kasGrowSelectionPtr(250623097);
    v24 = kasGrowSelectionPtr(15002404);
    v25 = kasfShipsSelectType(v24, v76, "tResourceCollector");
    kasfEntrySetValue(245323827, v25);
    v26 = kasGrowSelectionPtr(15002404);
    sub_4984C0(v26, 1);
    if ( !kasfEntryValueGet(71552168) )
    {
      v77 = kasGrowSelectionPtr(125683657);
      v27 = kasGrowSelectionPtr(240038482);
      v28 = kasfShipsSelectType(v27, v77, "tResourceCollector");
      if ( kasfEntryValueGet(245323827) + v28 < 3 )
      {
        kasfEntrySetValue(71552168, 1);
        v29 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v29, (char *)15936068);
        kasfEntrySetValue((int)&unk_C8CC23, 1);
      }
    }
    if ( kasfEntryValueGet(245323827) < 1 )
    {
      kasfEntryGetOrCreateSetValue(237926452, 0);
      if ( !kasfEntryValueGet((int)&unk_C8CC23) )
      {
        if ( kasfEntryValueGet(25105763) )
        {
          v30 = kasAITeamPtrIfExists(176850956);
          kasfMsgSend((struct AITeam *)(int)v30, (char *)94060206);
        }
        kasfEntryGetOrCreateSetValue(149884979, 0);
        kasfEntryGetOrCreateSetValue(25105763, 0);
      }
    }
    else
    {
      kasfEntrySetValue(149884979, 1);
    }
    if ( kasfEntryValueGet(149884979) )
    {
      kasfEntrySetValue(25105763, 1);
      v90 = kasGrowSelectionPtr(209148451);
      v31 = kasGrowSelectionPtr(15002404);
      kasfShipsRemove(v31, v90);
      v78 = kasGrowSelectionPtr(15002404);
      v32 = kasGrowSelectionPtr(245323889);
      if ( kasfShipsSelectType(v32, v78, "tResourceCollector") )
      {
        v91 = (int *)kasGrowSelectionPtr(245323889);
        v33 = (int *)kasGrowSelectionPtr(209148451);
        kasfShipsAdd(v33, v91);
        v34 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v34, (char *)139079938);
      }
      v35 = kasGrowSelectionPtr(209148451);
      if ( kasfShipsCount(v35) >= 3 && !kasfEntryValueGet(237926452) )
      {
        kasfEntrySetValue(237926452, 1);
        v36 = kasAITeamPtrIfExists(160291907);
        kasfMsgSend((struct AITeam *)(int)v36, (char *)105466259);
      }
    }
    v79 = kasGrowSelectionPtr(250623097);
    v37 = kasGrowSelectionPtr(36255123);
    v38 = kasfShipsSelectType(v37, v79, "tAdvanceSupportFrigate");
    v80 = kasGrowSelectionPtr(250623097);
    v39 = kasGrowSelectionPtr(36255122);
    v40 = kasfShipsSelectType(v39, v80, "tIonCannonFrigate") + v38;
    v81 = kasGrowSelectionPtr(250623097);
    v41 = kasGrowSelectionPtr(36255121);
    v42 = kasfShipsSelectType(v41, v81, "tStandardFrigate");
    kasfEntrySetValue(244404307, v42 + v40);
    v92 = (int *)kasGrowSelectionPtr(36255121);
    v43 = (int *)kasGrowSelectionPtr(203592532);
    kasfShipsAdd(v43, v92);
    v93 = (int *)kasGrowSelectionPtr(36255122);
    v44 = (int *)kasGrowSelectionPtr(203592532);
    kasfShipsAdd(v44, v93);
    v94 = (int *)kasGrowSelectionPtr(36255123);
    v45 = (int *)kasGrowSelectionPtr(203592532);
    kasfShipsAdd(v45, v94);
    if ( !kasfEntryValueGet(71552169) )
    {
      v82 = kasGrowSelectionPtr(125683657);
      v46 = kasGrowSelectionPtr(36255123);
      v47 = kasfShipsSelectType(v46, v82, "tAdvanceSupportFrigate");
      v83 = kasGrowSelectionPtr(125683657);
      v48 = kasGrowSelectionPtr(36255122);
      v49 = kasfShipsSelectType(v48, v83, "tIonCannonFrigate") + v47;
      v84 = kasGrowSelectionPtr(125683657);
      v50 = kasGrowSelectionPtr(36255121);
      v51 = kasfShipsSelectType(v50, v84, "tStandardFrigate") + v49;
      if ( kasfEntryValueGet(244404307) + v51 < 1 )
      {
        kasfEntrySetValue(71552169, 1);
        v52 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v52, (char *)202461748);
        kasfEntrySetValue(183436739, 1);
      }
    }
    if ( kasfEntryValueGet(244404307) < 1 )
    {
      if ( !kasfEntryValueGet(183436739) )
      {
        if ( kasfEntryValueGet(61779987) )
        {
          v53 = kasAITeamPtrIfExists(176850956);
          kasfMsgSend((struct AITeam *)(int)v53, (char *)105313438);
        }
        kasfEntryGetOrCreateSetValue(110567443, 0);
        kasfEntryGetOrCreateSetValue(61779987, 0);
      }
    }
    else
    {
      kasfEntrySetValue(110567443, 1);
    }
    if ( kasfEntryValueGet(110567443) )
    {
      kasfEntrySetValue(61779987, 1);
      v114 = kasGrowSelectionPtr(216250451);
      v54 = kasGrowSelectionPtr(203592532);
      kasfShipsRemove(v54, v114);
      v95 = kasGrowSelectionPtr(203592532);
      v55 = kasGrowSelectionPtr(36255121);
      if ( kasfShipsSelectType(v55, v95, "tStandardFrigate") )
      {
        v115 = (int *)kasGrowSelectionPtr(36255121);
        v56 = (int *)kasGrowSelectionPtr(216250451);
        kasfShipsAdd(v56, v115);
        if ( !kasfEntryValueGet(71552170) )
        {
          kasfEntrySetValue(71552170, 1);
          v57 = kasAITeamPtrIfExists(176850956);
          kasfMsgSend((struct AITeam *)(int)v57, (char *)77768393);
        }
        v58 = kasAITeamPtrIfExists(160291907);
        kasfMsgSend((struct AITeam *)(int)v58, (char *)251107429);
      }
      v96 = kasGrowSelectionPtr(203592532);
      v59 = kasGrowSelectionPtr(36255122);
      if ( kasfShipsSelectType(v59, v96, "tIonCannonFrigate") )
      {
        v116 = (int *)kasGrowSelectionPtr(36255122);
        v60 = (int *)kasGrowSelectionPtr(216250451);
        kasfShipsAdd(v60, v116);
        if ( !kasfEntryValueGet(71552171) )
        {
          kasfEntrySetValue(71552171, 1);
          v61 = kasAITeamPtrIfExists(176850956);
          kasfMsgSend((struct AITeam *)(int)v61, (char *)20030);
        }
        v62 = kasAITeamPtrIfExists(160291907);
        kasfMsgSend((struct AITeam *)(int)v62, (char *)251107429);
      }
      v97 = kasGrowSelectionPtr(203592532);
      v63 = kasGrowSelectionPtr(36255123);
      if ( kasfShipsSelectType(v63, v97, "tAdvanceSupportFrigate") )
      {
        v117 = (int *)kasGrowSelectionPtr(36255123);
        v64 = (int *)kasGrowSelectionPtr(216250451);
        kasfShipsAdd(v64, v117);
        v65 = kasAITeamPtrIfExists(160291907);
        kasfMsgSend((struct AITeam *)(int)v65, (char *)251107429);
      }
    }
    if ( !kasfEntryValueGet(71552172) )
    {
      v66 = kasGrowSelectionPtr(114961833);
      if ( !kasfShipsCount(v66) )
      {
        kasfEntrySetValue(71552172, 1);
        v67 = kasAITeamPtrIfExists(160291907);
        kasfMsgSend((struct AITeam *)(int)v67, (char *)134085843);
      }
    }
  }
  if ( !kasfEntryValueGet(71552173)
    && kasfObjectiveGet("CaptureFrigate")
    && kasfObjectiveGet("CaptureResource")
    && kasfEntryValueGet(-1964114797) )
  {
    kasfEntrySetValue(71552173, 1);
    kasfEntrySetValue(-2045714667, 1);
    v118 = kasGrowSelectionPtr(18336307);
    v68 = (int *)kasGrowSelectionPtr(45901433);
    kasSelectPlayerShipsSimple(v68, v118);
    v119 = kasGrowSelectionPtr(218060896);
    v69 = kasGrowSelectionPtr(19430160);
    kasfShipsSelectLinkedShips(v69, v119);
    v120 = (int *)kasGrowSelectionPtr(19430160);
    v70 = (int *)kasGrowSelectionPtr(45901433);
    kasfShipsAdd(v70, v120);
    v98 = kasGrowSelectionPtr(45901433);
    v71 = kasGrowSelectionPtr(28447699);
    kasfShipsSelectType(v71, v98, "tResourceCollector");
    v72 = kasGrowSelectionPtr(28447699);
    sub_4984C0(v72, 1);
    v73 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)v73, (int)&loc_4C489A + 3);
  }
  if ( !kasfEntryValueGet(-2045714667) )
  {
    if ( kasTimerExpiredById(78083653) )
    {
      v75 = kasAITeamPtrIfExists(77795571);
      kasfMsgSend((struct AITeam *)(int)v75, (char *)78083653);
    }
  }
}

void Watch_Mission08_HarvestTeam_WaitingForSignal(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfLabelledVectorSet(77795571, 0, 0, 0);
        kasfVectorRandomizeInRange(kasVectorPtrIfExists(77795571), kasVectorPtrIfExists(48187763), 15000);
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(77795571));
        kasfShipsAdd(kasGrowSelectionPtr(114961833), &team->shipList);
        kasfMsgSend(kasAITeamPtrIfExists(115043347), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(145052708), (char *)5801588);
        kasfMsgSend(kasAITeamPtrIfExists(139033868), (char *)5801588);
        kasfSpeechEvent(208120516, (sdword)Init_Mission08_HarvestTeam_Harvest, (sdword)Watch_Mission08_HarvestTeam_Harvest);
    }
}

void Init_Mission08_HarvestTeam_Harvest(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfHarvest();
    kasTimerCreateSetStartById(208120516, kasfRandom(10, 20));
}

void Watch_Mission08_HarvestTeam_Harvest(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrder(&team->shipList) & 5) == 0 || kasTimerExpiredDestroyById(208120516))
    {
        kasTimerDestroyById(208120516);
        kasfSpeechEvent(208120516, (sdword)Init_Mission08_HarvestTeam_Harvest, (sdword)Watch_Mission08_HarvestTeam_Harvest);
    }
}

void Init_Mission08_Events_IndeedSpawnWorkers(void)
{
    AITeam *team;
    GrowSelection *createdWorkers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    createdWorkers = kasGrowSelectionPtr(164967155);
    kasfShipsCreateInside(createdWorkers, "sWorker", kasfRandom(2, 5), kasAITeamShipsPtrIfExists(101191117), 0);
    kasfReinforceTeamWithShips(team, createdWorkers);
    kasAITeamShipsPtr(createdWorkers, 0);
}

void Watch_Mission08_Events_IndeedSpawnWorkersLaunch(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            90850786,
            (sdword)Watch_Mission07_Events_PreparingAmbassador,
            (sdword)Watch_Mission08_Events_IndeedDepart);
    }
}

void Watch_Mission08_Events_IndeedDepart(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(161590357))
        {
            kasTimerDestroyById(161590357);
            kasfSpeechEvent(
                90850786,
                (sdword)Watch_Mission07_Events_PreparingAmbassador,
                (sdword)Watch_Mission08_Events_IndeedDepart);
        }
    }
    else
    {
        kasTimerDestroyById(161590357);
        if (GetGameDifficulty() == 3)
        {
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        }
        else
        {
            kasfSpeechEvent(
                110655924,
                (sdword)Init_Mission08_Events_IndeedSpawnWorkers,
                (sdword)Watch_Mission08_Events_IndeedSpawnWorkersLaunch);
        }
    }
}

void Watch_Mission08_Events_Indeed(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(27239543))
    {
        kasfSpeechEvent(
            90850786,
            (sdword)Watch_Mission07_Events_PreparingAmbassador,
            (sdword)Watch_Mission08_Events_IndeedDepart);
    }
}

void Watch_Mission08_Events_JustLikeKharToba(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)104994414))
    {
        kasfSpeechEvent(
            78199427,
            (sdword)Init_Mission08_Events_WeCanLeaveSequence,
            (sdword)Watch_Mission08_Events_WeCanLeaveCombat);
    }
}

void Watch_Mission08_Events_WeCanLeave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(51324946))
    {
        kasfDeleteShips(&team->shipList);
    }
}

void Watch_Mission08_PreAttackPod(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_HiveFrigateCombatLoop,
            (sdword)Watch_Mission08_PreAttackPod);
    }
}

void Init_Mission08_HiveFrigateReinforcementTeam(void)
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
                (sdword)Watch_Mission08_PreAttackPod);
        }
        else
        {
            kasfSpeechEvent(
                4693508,
                (sdword)sub_65F6F0,
                (sdword)Watch_Mission08_HiveFrigateReinforcementTeam);
        }
    }
}

void Init_Mission08_HiveFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(186735637)) &&
        kasfEntryValueGet(-2047426043) &&
        kasfMsgReceived((char *)0x479E04))
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
            (sdword)Watch_Mission08_HiveFrigateReinforcementTeam);
    }
}

void Watch_Mission08_HiveFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedHiveFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedHiveFrigates = kasGrowSelectionPtr(0x58A20);
    if (kasfShipsSelectType(returnedHiveFrigates, kasGrowSelectionPtr(0x2A98C74), "bHiveFrigate") < 1)
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
                    (sdword)Watch_Mission08_HiveFrigateReinforcementBuildCount);
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
        (sdword)Watch_Mission08_PreAttackPod);
}

void Watch_Mission08_HiveFrigateReinforcementBuildCount(void)
{
    AITeam *team;
    sdword buildScale;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
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
checkBuildReady:
            if (kasfEntryValueGet(154639956) != 2)
            {
                return;
            }
            goto beginBuildTimer;
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
            goto checkBuildReady;
        }

        buildCost = kasfShipTypeBuildCost("bHiveFrigate") / 2;
        if (kasfRUsGet(1) >= buildCost)
        {
            goto checkBuildReady;
        }
beginBuildTimer:
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_HiveFrigateReinforcementBuildTimer,
            (sdword)Watch_Mission08_HiveFrigateReinforcementSpawn);
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
            goto beginBuildTimer;
        }
    }
    if (kasfEntryValueGet(154639956) == 2)
    {
        goto beginBuildTimer;
    }
}

void Watch_Mission08_HiveFrigateReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *spawnSource;
    GrowSelection *spawnTarget;
    GrowSelection *createdHiveFrigates;
    sdword spawnIndex;

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
        spawnIndex = kasfRandom(0, kasfShipsCount(spawnSource) - 1);
        spawnTarget = kasGrowSelectionPtr(0x5425);
        kasfShipsSelectIndex(spawnTarget, spawnSource, spawnIndex);
        createdHiveFrigates = kasGrowSelectionPtr(0x9379E54);
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
            (sdword)Watch_Mission08_PreAttackPod);
    }
}

void Init_Mission08_MissionFlow(void)
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
    kasfTutEnableFlags("Flags");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
}

void Watch_Mission08_MissionFlow_PostIntelCleanup(void)
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
            kasTimerStartById(144337492);
            kasfEntrySetValue(127756738, 0);
        }
        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
    }
}

void Watch_Mission08_MissionFlow_Delay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, 0);
    }
}

void Watch_Mission08_MissionFlow(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;
    AITeam *targetTeam;
    GrowSelection *focusShips;
    hvector *lookAtPoint;
    sdword followupMessage;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(0xCFF5860)))
    {
        kasfMissionFailed();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
        return;
    }

    if (!kasfEntryValueGet(332667))
    {
        queueInfo = *((sdword **)team + 20);
        queueIndex = -1;
        if (queueInfo != NULL)
        {
            queueIndex = *queueInfo;
        }
        if (queueIndex < 0)
        {
            return;
        }

        while (1)
        {
            message = aitQueuePopPointerByIndex(team, queueIndex);
            if ((udword)message > 0x52E80A4)
            {
                if ((udword)message > 0x84A3102)
                {
                    if (message == (void *)140347652)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268439978, 0);
                        kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    if (message == (void *)175596756)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444083, 0);
                        kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    if (message == (void *)202461748)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444080, 0);
                        kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                }
                else
                {
                    if (message == (void *)139079938)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444078, 0);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    if (message == (void *)94060206)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444082, 0);
                        targetTeam = kasAITeamPtrIfExists(160291907);
                        kasfMsgSend((int)targetTeam, 94060206);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    if (message == (void *)105313438)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444081, 0);
                        targetTeam = kasAITeamPtrIfExists(160291907);
                        kasfMsgSend((int)targetTeam, 105313438);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    if (message == (void *)132678291)
                    {
                        kasTimerSetById(144337492, 3);
                        kasTimerStartById(144337492);
                        kasfEntrySetValue(332667, 1);
                        kasfWideScreenIn(90);
                        targetTeam = kasAITeamPtrIfExists(176850956);
                        kasfMsgSend((int)targetTeam, 243804466);
                        kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
                        return;
                    }
                }
            }
            else
            {
                if (message == (void *)86933668)
                {
                    kasfEntrySetValue(332667, 1);
                    kasTimerSetById(144337492, 3);
                    kasTimerStartById(144337492);
                    kasfWideScreenIn(90);
                    targetTeam = kasAITeamPtrIfExists(176850956);
                    kasfMsgSend((int)targetTeam, 134873953);
                    targetTeam = kasAITeamPtrIfExists(160291907);
                    kasfMsgSend((int)targetTeam, 76144100);
                    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
                    return;
                }
                if (message == (void *)m08_fleetIntelTriggerMsg)
                {
                    kasTimerSetById(144337492, 3);
                    kasTimerStartById(144337492);
                    kasfWideScreenOut(90);
                    targetTeam = kasAITeamPtrIfExists(176850956);
                    kasfMsgSend((int)targetTeam, 82024194);
                    kasfEntrySetValue(332667, 1);
                    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
                    return;
                }
                if (message != NULL)
                {
                    if (message == (void *)20030)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444076, 0);
                        kasfAllowPlayerToResearch("TECH_ION");
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    if (message == (void *)0x4C489D)
                    {
                        kasfSpeechEventQueueCurrentPlayer(268444086, 0);
                        targetTeam = kasAITeamPtrIfExists(160291907);
                        kasfMsgSend((int)targetTeam, 175938357);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                    }
                    aitQueuePointer(team, team, message);
                }
                if (message == (void *)15936068)
                {
                    kasfSpeechEventQueueCurrentPlayer(268444079, 0);
                    kasfSpeechEvent(77979796, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }
                if (message == (void *)75374526)
                {
                    kasfEntrySetValue(332667, 1);
                    kasTimerSetById(144337492, 3);
                    kasTimerStartById(144337492);
                    spEventWideScreenIn(90);
                    targetTeam = kasAITeamPtrIfExists(176850956);
                    kasfMsgSend((int)targetTeam, 132250546);
                    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, 0);
                    return;
                }
                if (message == (void *)77768393)
                {
                    kasfSpeechEventQueueCurrentPlayer(268444077, 0);
                    kasfAllowPlayerToResearch("TECH_ENERGY");
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }
            }

            queueInfo = *((sdword **)team + 20);
            if (queueInfo != NULL && *queueInfo != 0 && (--queueIndex & 0x80000000) == 0)
            {
                continue;
            }
            return;
        }
    }

    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }
    if (queueIndex < 0)
    {
        return;
    }

    while (1)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if ((udword)message > 0x80A0361)
        {
            if ((udword)message <= 0xC409F45)
            {
                if (message == (void *)205561669)
                {
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEventQueueCurrentPlayer(268439989, 0);
                    switch (GetGameDifficulty())
                    {
                        case 0:
                            kasTimerCreateSetStartById(-2086178155, 4000);
                            break;
                        case 1:
                            kasTimerCreateSetStartById(-2086178155, 3000);
                            break;
                        case 2:
                            kasTimerCreateSetStartById(-2086178155, 2400);
                            break;
                        case 3:
                            kasTimerCreateSetStartById(-2086178155, 2000);
                            break;
                        case 4:
                            kasTimerCreateSetStartById(-2086178155, 1800);
                            break;
                    }
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }
                if (message == (void *)243804466)
                {
                    kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                    kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                    kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                    focusShips = kasAITeamShipsPtrIfExists(132678291);
                    if (kasfShipsSelectIndex(kasGrowSelectionPtr(0x5425), focusShips, 0))
                    {
                        focusShips = kasGrowSelectionPtr(0x5425);
                    }
                    kasfTutCameraFocusClose(focusShips);
                    kasfSpeechEventQueueCurrentPlayer(268439871, 0);
                    targetTeam = kasAITeamPtrIfExists(176850956);
                    kasfMsgSend((int)targetTeam, 243804467);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }
                if (message == (void *)243804467)
                {
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntrySetValue(266064926, 1);
                    kasfEntrySetValue(52577235, 1);
                    kasfEntrySetValue(172907219, 1);
                    kasfEntrySetValue(168240451, 1);
                    kasfEntrySetValue(332667, 0);
                    kasfOtherKASPause();
                    kasfUniversePause();
                    kasfEntrySetValue(211831571, 1);
                    kasfEntrySetValue(109086661, 1);
                    kasfOpenSensors(150000);
                    focusShips = kasAITeamShipsPtrIfExists(132678291);
                    soundStopDSound("CrystalsPointer", focusShips);
                    kasfSpeechEventQueueCurrentPlayer(268443968, 0);
                    kasfAllowPlayerToResearch("TECH_CRYSTAL_HARVEST");
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }

                switch ((udword)message)
                {
                    case 0x80A0362u:
                        kasfSpeechEventQueueCurrentPlayer(268439992, 0);
                        followupMessage = 134873955;
                        targetTeam = kasAITeamPtrIfExists(176850956);
                        kasfMsgSend((int)targetTeam, followupMessage);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;

                    case 0x80A0363u:
                        kasfSpeechEventQueueCurrentPlayer(268468665, 0);
                        followupMessage = 134873956;
                        targetTeam = kasAITeamPtrIfExists(176850956);
                        kasfMsgSend((int)targetTeam, followupMessage);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;

                    case 0x80A0364u:
                        kasfSpeechEventQueueCurrentPlayer(268444090, 0);
                        followupMessage = 134873957;
                        targetTeam = kasAITeamPtrIfExists(176850956);
                        kasfMsgSend((int)targetTeam, followupMessage);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;

                    case 0x80A0365u:
                        kasfSpeechEventQueueCurrentPlayer(268468667, 0);
                        followupMessage = 134873958;
                        targetTeam = kasAITeamPtrIfExists(176850956);
                        kasfMsgSend((int)targetTeam, followupMessage);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;

                    case 0x80A0366u:
                        kasfSpeechEventQueueCurrentPlayer(268468668, 0);
                        kasfMsgSend((int)kasAITeamPtrIfExists(77795571), 5801588);
                        kasfMsgSend((int)kasAITeamPtrIfExists(22617), 175938357);
                        kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 4934180);
                        targetTeam = kasAITeamPtrIfExists(77795571);
                        kasfMsgSend((int)targetTeam, 5801588);
                        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                        return;
                }
            }
            else if (message == (void *)134873953)
            {
                kasfPingRemove("PointerSpies");
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                focusShips = kasAITeamShipsPtrIfExists(22617);
                kasfTutCameraFocusClose(focusShips);
                focusShips = kasAITeamShipsPtrIfExists(22617);
                kasfShipsSetDontApplyPhysicsFlag(focusShips, 1);
                kasfSpeechEventQueueCurrentPlayer(268468663, 0);
                targetTeam = kasAITeamPtrIfExists(176850956);
                kasfMsgSend((int)targetTeam, 134873954);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            }
        }
        else
        {
            if (message == (void *)134873953)
            {
                /* handled above via the ordered comparisons */
            }
            else if ((udword)message > 0x57489D4)
            {
                if (message == (void *)91523540)
                {
                    kasfEntrySetValue(232774915, 1);
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(127756738, 1);
                    kasfEntrySetValue(332667, 0);
                    kasfEntrySetValue(266064926, 1);
                    kasfSpeechEventQueueCurrentPlayer(268444072, 0);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }
                if (message == (void *)82024194)
                {
                    kasfEntrySetValue(332667, 0);
                    kasfSpeechEventQueueCurrentPlayer(268444068, 0);
                    targetTeam = kasAITeamPtrIfExists(160291907);
                    kasfMsgSend((int)targetTeam, 176464661);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }

                if (message != NULL)
                {
                    aitQueuePointer(team, team, message);
                }
            }
            else if (message == (void *)42212039)
            {
                kasfEntrySetValue(172907219, 1);
                focusShips = kasAITeamShipsPtrIfExists(130285475);
                kasfTutCameraFocusClose(focusShips);
                kasfSpeechEventQueueCurrentPlayer(268439975, 0);
                kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 91523540);
                kasfMsgSend((int)kasAITeamPtrIfExists(160291907), 91523540);
                kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                return;
            }
            else if (message != NULL)
            {
                if (message == (void *)0x4B4A24)
                {
                    kasfOtherKASPause();
                    kasfUniversePause();
                    if (!kasfSensorsIsOpen(0))
                    {
                        kasfOpenSensors(150000);
                    }
                    lookAtPoint = kasTeamsVectorPtr(77795571);
                    kasfSensorsSetLookatPoint(lookAtPoint);
                    focusShips = kasAITeamShipsPtrIfExists(77795571);
                    soundStopDSound("PointerResourcers", focusShips);
                    kasfSpeechEventQueueCurrentPlayer(268439979, 0);
                    targetTeam = kasAITeamPtrIfExists(176850956);
                    kasfMsgSend((int)targetTeam, 124099109);
                    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
                    return;
                }
                aitQueuePointer(team, team, message);
            }
        }

        if (message == (void *)110791399)
        {
            kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
            kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
            kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
            focusShips = kasAITeamShipsPtrIfExists(22617);
            kasfTutCameraFocusClose(focusShips);
            kasfSpeechEventQueueCurrentPlayer(268468646, 0);
            targetTeam = kasAITeamPtrIfExists(176850956);
            kasfMsgSend((int)targetTeam, 42212039);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
            return;
        }
        if (message == (void *)124099109)
        {
            kasfEntrySetValue(211831571, 1);
            kasfEntrySetValue(109086661, 1);
            kasTimerSetById(144337492, 3);
            kasfEntrySetValue(127756738, 1);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(52577235, 1);
            kasfEntrySetValue(168240451, 1);
            kasfEntrySetValue(172907219, 1);
            kasfSpeechEventQueueCurrentPlayer(268444084, 0);
            kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 205561669);
            kasfMsgSend((int)kasAITeamPtrIfExists(160291907), 124099109);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
            return;
        }
        if (message == (void *)132250546)
        {
            kasfOtherKASPause();
            kasfUniversePause();
            kasfEntrySetValue(211831571, 1);
            kasfEntrySetValue(109086661, 1);
            if (!kasfSensorsIsOpen(0))
            {
                kasfOpenSensors(150000);
            }
            lookAtPoint = kasTeamsVectorPtr(22617);
            kasfSensorsSetLookatPoint(lookAtPoint);
            focusShips = kasAITeamShipsPtrIfExists(22617);
            kasfPingAddShips(focusShips, "PointerSpies");
            kasfSpeechEventQueueCurrentPlayer(268439973, 0);
            targetTeam = kasAITeamPtrIfExists(176850956);
            kasfMsgSend((int)targetTeam, 110791399);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, 0);
            return;
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo != NULL && *queueInfo != 0 && (--queueIndex & 0x80000000) == 0)
        {
            continue;
        }
        return;
    }
}

void Watch_Mission08_GuardGravWells_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x7C3FFA3));
        if (kasfThisTeamIs(kasAITeamPtrIfExists(130285475)))
        {
            kasfMsgSend((int)kasAITeamPtrIfExists(139498707), 5801588);
            kasfMsgSend((int)kasAITeamPtrIfExists(91786963), 5801588);
        }
        kasfSpeechEvent(74024315, (sdword)Init_Mission08_GuardGravWells_HuntNearbyTargets, 0);
    }
    else if (!kasfEntryValueGet(71552161) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission08_FrigateGroup_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr(0x6DB6C13), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
    kasfSpeechEvent(
        175938357,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission08_FrigateGroup_HyperspaceInAndFormOnResourceCollectorGroup);
}

void Watch_Mission08_FrigateGroup_BreakAttachmentsAndAttack(void)
{
    AITeam *team;
    GrowSelection *attachedSelection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(71552165) || kasfTeamCount())
    {
        if (kasfMsgReceived((char *)0x4A77645))
        {
            kasfShipsSelectSinglePlayerCombatShips(&kasGrowSelectionPtr(0x2BC6679)->maxNumShips, (int)kasGrowSelectionPtr(0x117CA33));
            if (kasfShipsSelectAttachedToSelection(&kasGrowSelectionPtr(0x48D5)->maxNumShips, (int)kasGrowSelectionPtr(0x2BC6679), (int)&team->shipList))
            {
                attachedSelection = kasGrowSelectionPtr(0x48D5);
                kasfShipsSetPercentDamaged((int)attachedSelection, 0);
                kasfShipsDamage(kasGrowSelectionPtr(0x48D5), 1);
            }
            kasfAttackMothership();
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
        }
    }
    else
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission08_ResourceCollectorGroup_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetPercentDamaged((int)&team->shipList, 200);
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr(0x6DB6C13), &team->shipList);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
    kasfShipsSetNotBumpable(&team->shipList, 1);
    kasfSpeechEvent(
        175938357,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission08_HarvestTeam_WaitingForSignal);
}

void Watch_Mission08_ResourceCollectorGroup_BreakAttachmentsAndSignalFrigates(void)
{
    AITeam *team;
    GrowSelection *attachedSelection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x4A77645))
    {
        kasfShipsSelectSinglePlayerCombatShips(&kasGrowSelectionPtr(0x2BC6679)->maxNumShips, (int)kasGrowSelectionPtr(0x117CA33));
        if (kasfShipsSelectAttachedToSelection(&kasGrowSelectionPtr(0x48D5)->maxNumShips, (int)kasGrowSelectionPtr(0x2BC6679), (int)&team->shipList))
        {
            attachedSelection = kasGrowSelectionPtr(0x48D5);
            kasfShipsSetPercentDamaged((int)attachedSelection, 0);
            kasfShipsDamage(kasGrowSelectionPtr(0x48D5), 1);
        }
        kasfMsgSend((int)kasAITeamPtrIfExists(115043347), 78083653);
        kasfMsgSend((int)kasAITeamPtrIfExists(145052708), 78083653);
        kasfMsgSend((int)kasAITeamPtrIfExists(139033868), 78083653);
        kasfEntryGetOrCreateSetValue(-2045714667, 1);
        kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 175596756);
        kasfAttackMothership();
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission08_GuardGravWells_HuntNearbyTargets(void)
{
    GrowSelection *attackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackTargets = kasGrowSelectionPtr(57245931);
    if (!kasfShipsSelectNearby(attackTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfShipsAdd(attackTargets, kasAITeamShipsPtrIfExists(22617));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)) &&
        kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr(139967715), kasGrowSelectionPtr(139967715), attackTargets))
    {
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr(139967715));
    }
    if (kasfThisTeamIs((int)kasAITeamPtrIfExists(130285475)))
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationWall();
    }
    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(attackTargets);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, 10);
}

void Watch_Mission08_GuardGravWells_HuntNearbyTargets(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Init_Mission08_GuardGravWells_HuntNearbyTargets, 0);
    }
}

void Init_Mission08_FrigateGroup2_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr(0x6DB6C13), &team->shipList);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission08_FrigateGroup2_BreakAttachmentsAndAttack(void)
{
    AITeam *team;
    GrowSelection *attachedSelection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfEntryValueGet(71552164) || kasfTeamCount())
    {
        if (kasfMsgReceived((char *)0x4A77645))
        {
            kasfShipsSelectSinglePlayerCombatShips(&kasGrowSelectionPtr(0x2BC6679)->maxNumShips, (int)kasGrowSelectionPtr(0x117CA33));
            if (kasfShipsSelectAttachedToSelection(&kasGrowSelectionPtr(0x48D5)->maxNumShips, (int)kasGrowSelectionPtr(0x2BC6679), (int)&team->shipList))
            {
                attachedSelection = kasGrowSelectionPtr(0x48D5);
                kasfShipsSetPercentDamaged((int)attachedSelection, 0);
                kasfShipsDamage(kasGrowSelectionPtr(0x48D5), 1);
            }
            kasfAttackMothership();
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
        }
    }
    else
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission08_FrigateGroup3_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsAdd(kasGrowSelectionPtr(0x6DB6C13), &team->shipList);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
    kasfEntryGetOrCreateSetValue(332667, 0);
    kasfSpeechEvent(175938357, (sdword)MaintainTeamContext, 0);
}

void Watch_Mission08_FrigateGroup3_BreakAttachmentsAndAttack(void)
{
    AITeam *team;
    GrowSelection *attachedSelection;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasGrowSelectionPtr(0x6DB6C13)))
            {
                if (!kasfMsgReceived((char *)0x4A77645))
                {
                    return;
                }
                kasfShipsSelectSinglePlayerCombatShips(&kasGrowSelectionPtr(0x2BC6679)->maxNumShips, (int)kasGrowSelectionPtr(0x117CA33));
                if (kasfShipsSelectAttachedToSelection(&kasGrowSelectionPtr(0x48D5)->maxNumShips, (int)kasGrowSelectionPtr(0x2BC6679), (int)&team->shipList))
                {
                    attachedSelection = kasGrowSelectionPtr(0x48D5);
                    kasfShipsSetPercentDamaged((int)attachedSelection, 0);
                    kasfShipsDamage(kasGrowSelectionPtr(0x48D5), 1);
                }
            }
            kasfAttackMothership();
        }
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Init_Mission08_FrigateGroup_FormOnResourceCollectorGroup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationWall();
    if (kasfShipsCount(kasAITeamShipsPtrIfExists(77795571)))
    {
        kasfProtect((int)kasAITeamShipsPtrIfExists(77795571));
    }
    else
    {
        kasfSpeechEvent(74024315, (sdword)Init_Mission08_FrigateGroup_HuntNearbyTargets, 0);
    }
}

void Watch_Mission08_FrigateGroup_FormOnResourceCollectorGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (((kasfShipsOrderAttributes((int)&team->shipList) == 0) & 2) != 0)
    {
        kasfSpeechEvent(0x4C9664, (sdword)Init_Mission08_FrigateGroup_FormOnResourceCollectorGroup, 0);
    }
}

void Init_Mission08_FrigateGroup_HuntNearbyTargets(void)
{
    GrowSelection *attackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackTargets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(attackTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr(0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)) &&
        kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr(139967715), kasGrowSelectionPtr(139967715), attackTargets))
    {
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr(139967715));
    }
    kasfShipsSortByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    tutDeallocateRootRegion(attackTargets);
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}

void Watch_Mission08_FrigateGroup_HuntNearbyTargets(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Init_Mission08_FrigateGroup_HuntNearbyTargets, 0);
    }
}

void Init_Mission08_SpyTeam_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
}

void Watch_Mission08_SpyTeam_AttackMothership(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    void *message;
    GrowSelection *focusShips;
    sdword damageAmount;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueInfo = *((sdword **)team + 20);
    queueIndex = -1;
    if (queueInfo != NULL)
    {
        queueIndex = *queueInfo;
    }
    if (queueIndex < 0)
    {
        goto attack_complete;
    }

    while (1)
    {
        message = aitQueuePopPointerByIndex(team, queueIndex);
        if (message != NULL)
        {
            if (message == (void *)5801588)
            {
                kasfTeamSwitchPlayerOwner(0);
                kasfShipsEnableAttachNLips(&team->shipList);
                kasfEntryGetOrCreateSetValue(1089, kasfRandom(-1, 1));
                kasfEntryGetOrCreateSetValue(1090, kasfRandom(-1, 1));
                if (!kasfEntryValueGet(1089))
                {
                    if (kasfRandom(1, 2) == 1)
                    {
                        kasfEntryIncrement(1089);
                    }
                    else
                    {
                        kasfEntryDecrement(1089);
                    }
                }
                if (!kasfEntryValueGet(1090))
                {
                    if (kasfRandom(1, 2) == 1)
                    {
                        kasfEntryIncrement(1090);
                    }
                    else
                    {
                        kasfEntryDecrement(1090);
                    }
                }
                kasfLabelledVectorSet(
                    0x5859,
                    85000 * kasfEntryValueGet(1089),
                    85000 * kasfEntryValueGet(1090),
                    kasfRandom(-6000, 6000));
                kasfEntryScopeName(1089);
                kasfEntryScopeName(1090);
                kasfVectorAddClampToUniverse(kasVectorPtrIfExists(0x5859), kasShipsVectorPtrIfExists(218060896));
                kasfLabelledVectorSet(0x7C3FFA3, 0, 0, 0);
                switch (GetGameDifficulty())
                {
                    case 0:
                        kasfShipsSetPercentDamaged((int)&team->shipList, 800);
                        damageAmount = 800;
                        break;
                    case 1:
                        kasfShipsSetPercentDamaged((int)&team->shipList, 200);
                        damageAmount = 700;
                        break;
                    case 2:
                        kasfShipsSetPercentDamaged((int)&team->shipList, 100);
                        damageAmount = 600;
                        break;
                    case 3:
                        damageAmount = 550;
                        break;
                    case 4:
                        damageAmount = 500;
                        break;
                    default:
                        damageAmount = 0;
                        break;
                }
                if (GetGameDifficulty() <= 4)
                {
                    kasfVectorBlendClampToUniverse(
                        kasVectorPtrIfExists(0x7C3FFA3),
                        kasShipsVectorPtrIfExists(218060896),
                        kasVectorPtrIfExists(0x5859),
                        damageAmount,
                        100);
                }
                kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x7C3FFA3));
                kasfMsgSend((int)kasAITeamPtrIfExists(130285475), 5801588);
                kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 75374526);
                kasfMusicPlay(19);
                kasfSpeechEvent(78083653, (sdword)Init_Mission08_SpyTeam_InterceptTargetGroup, 0);
                return;
            }
            if (message == (void *)175938357)
            {
                kasfSpeechEvent(374244, (sdword)Init_Mission08_SpyTeam_InterceptTargetGroup, 0);
                return;
            }
            aitQueuePointer(team, team, message);
        }

        queueInfo = *((sdword **)team + 20);
        if (queueInfo != NULL && *queueInfo != 0 && (--queueIndex & 0x80000000) == 0)
        {
            continue;
        }
        break;
    }

attack_complete:
    if (!kasfEntryValueGet(71552162) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 140347652);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, 0);
    }
}

void Init_Mission08_CrystalHarvesterTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfSpeechEvent(
        0x47C97B,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission08_CrystalHarvesterTeam_CheckNearbyTargets);
}

void Watch_Mission08_FrigateGroup_HyperspaceInAndFormOnResourceCollectorGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)5801588))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(77795571));
        kasfShipsAdd(kasGrowSelectionPtr(0x6DA2DA9), &team->shipList);
        kasfEntryGetOrCreateSetValue(-2104338061, 1);
        kasfSpeechEvent(0x4C9664, (sdword)Init_Mission08_FrigateGroup_FormOnResourceCollectorGroup, 0);
    }
}

void Watch_Mission08_CrystalHarvesterTeam_CheckNearbyTargets(void)
{
    AITeam *team;
    GrowSelection *nearbyTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSelectSinglePlayerCombatShips(&kasGrowSelectionPtr(0x2BC6679)->maxNumShips, (int)kasGrowSelectionPtr(0x117CA33));
    nearbyTargets = kasGrowSelectionPtr(0x5296679);
    if (kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr(0x2BC6679), kasThisTeamsVectorPtr(), 20000) &&
        kasfShipsRemove(nearbyTargets, kasAITeamShipsPtrIfExists(0x5859)))
    {
        kasfMsgSend((int)kasAITeamPtrIfExists(176850956), 132678291);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission08_SpyTeam_InterceptTargetGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfIntercept(kasGrowSelectionPtr(0xCFF5860));
}

void Watch_Mission08_SpyTeam_InterceptTargetGroup(void)
{
    AITeam *team;
    GrowSelection *dockFormation;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552163))
    {
        if (kasfNearby(kasShipsVectorPtrIfExists(218060896), 5000))
        {
            kasfEntryGetOrCreateSetValue(71552163, 1);
            kasfStop();
            dockFormation = kasGrowSelectionPtr(0x716C77E);
            kasfGrowSelectionClear(dockFormation);
            kasfShipsAdd(dockFormation, &team->shipList);
            kasfShipsAdd(dockFormation, kasGrowSelectionPtr(0xCFF5860));
            kasfShipsSetFormation(dockFormation, 8);
            kasTimerCreateSetStartById(374244, 60);
        }
    }
    if (kasTimerExpiredById(374244))
    {
        kasfMsgSend(kasAITeamPtrIfExists(123922546), (char *)5801588);
        kasTimerCreateSetStartById(299899, 8);
    }
    if (kasTimerExpiredById(299899))
    {
        kasfDockStayWithTeam(kasAITeamPtrIfExists(123922546));
    }
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfMsgSend(kasAITeamPtrIfExists(123922546), (char *)175938357);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Begin_Mission08_DockedAttackTeam_ImmediateAttack(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 3);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
}

void Init_Mission08_DockedAttackTeam_AttackMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfSpeechEvent(
        175938357,
        (sdword)Begin_Mission08_DockedAttackTeam_ImmediateAttack,
        (sdword)Watch_Mission08_DockedAttackTeam_StageOrAttack);
}

void Watch_Mission08_DockedAttackTeam_StageOrAttack(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x588674))
    {
        kasfTeamSwitchPlayerOwner(0);
        kasfTeamHyperspaceInNear(kasShipsVectorPtrIfExists(218060896), 6000);
    }
    if (kasfMsgReceived((char *)0xA7C9B35))
    {
        kasfAttackMothership();
    }
}

void Begin_Mission08_FrigateGroup3_ProtectResourceCollectorGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x588674))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0x4A310F3));
        kasfTeamSpecialToggle();
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission08_FrigateGroup3_FormOnResourceCollectorGroup,
            (sdword)Watch_Mission08_FrigateGroup3_FormOnResourceCollectorGroup);
    }
}

void Init_Mission08_FrigateGroup3_FormOnResourceCollectorGroup(void)
{
    AITeam *team;
    GrowSelection *protectTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfFormationWall();
    protectTargets = kasAITeamShipsPtrIfExists(77795571);
    if (kasfShipsCount(protectTargets))
    {
        kasfProtect(protectTargets);
    }
    else
    {
        protectTargets = kasAITeamShipsPtrIfExists(115043347);
        if (!kasfShipsCount(protectTargets))
        {
            protectTargets = kasGrowSelectionPtr(0x6DB6C13);
        }
        kasfProtect(protectTargets);
    }
}

void Watch_Mission08_FrigateGroup3_FormOnResourceCollectorGroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfFindShipsTargetingSelection(kasGrowSelectionPtr(0x6DB6C13), kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfSpeechEvent(
            25690548,
            (sdword)Init_Mission08_FrigateGroup3_AttackClosestResourceCollectorThreat,
            (sdword)Watch_Mission08_FrigateGroup3_AttackClosestResourceCollectorThreat);
    }
    else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission08_FrigateGroup3_FormOnResourceCollectorGroup,
            (sdword)Watch_Mission08_FrigateGroup3_FormOnResourceCollectorGroup);
    }
}

void Init_Mission08_FrigateGroup3_AttackClosestResourceCollectorThreat(void)
{
    AITeam *team;
    GrowSelection *resourceCollectorThreats;
    GrowSelection *attackTarget;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    resourceCollectorThreats = kasGrowSelectionPtr(0x6DB6C13);
    if (!kasfFindShipsTargetingSelection(resourceCollectorThreats, kasGrowSelectionPtr(0x857BCE3)))
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission08_FrigateGroup3_FormOnResourceCollectorGroup,
            (sdword)Watch_Mission08_FrigateGroup3_FormOnResourceCollectorGroup);
        return;
    }

    kasfShipsSortByDistanceToPoint(resourceCollectorThreats, (vector *)kasShipsVectorPtrIfExists(139967715));
    attackTarget = kasGrowSelectionPtr(0x4C9664);
    kasfShipsSelectIndex(attackTarget, resourceCollectorThreats, 0);
    kasfProtect(attackTarget);
}

void Watch_Mission08_FrigateGroup3_AttackClosestResourceCollectorThreat(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(
            0x4C9664,
            (sdword)Init_Mission08_FrigateGroup3_FormOnResourceCollectorGroup,
            (sdword)Watch_Mission08_FrigateGroup3_FormOnResourceCollectorGroup);
    }
}

void Init_Mission08_Events_WeCanLeaveSequence(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamHyperspaceIn(kasVectorPtrIfExists(0xE099B35));
    kasTimerCreateSetStartById(150120898, 15);
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)159901860);
    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)141560536);
}

void Init_CruiseMissileStrikeWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, 0);
}

void Begin_CruiseMissileStrikeWave(void)
{
    AITeam *team;
    GrowSelection *createdMissiles;
    GrowSelection *launchSource;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    createdMissiles = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(createdMissiles);
    launchSource = kasAITeamShipsPtrIfExists(170794736);
    kasfShipsCreateInside(createdMissiles, "bCruiseMissile", kasfRandom(2, 4), launchSource, 1);
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfReinforceTeamWithShips(team, createdMissiles);
}

void Select_CruiseMissileStrikeTargets(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;
    GrowSelection *priorityTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfFormationDelta();
    kasfMsgSend(kasAITeamPtrIfExists(136386561), (char *)132221043);
    capitalTargets = kasGrowSelectionPtr(124053532);
    if (kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr(0x4A2A29)))
    {
        priorityTargets = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsSelectType(kasGrowSelectionPtr(363040), capitalTargets, "sRammingFrigate");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(363040));
        kasfShipsSelectType(kasGrowSelectionPtr(363040), capitalTargets, "sMultiBeamFrigate");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(363040));
        kasfShipsSelectType(kasGrowSelectionPtr(363040), capitalTargets, "sHiveFrigate");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(363040));
        kasfShipsSelectType(kasGrowSelectionPtr(363040), capitalTargets, "sProcessor");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(363040));
        kasfShipsSelectType(kasGrowSelectionPtr(363040), capitalTargets, "sWorker");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(363040));
        if (!kasfShipsCount(priorityTargets))
        {
            kasfShipsAdd(priorityTargets, capitalTargets);
        }
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29));
    }
    singlePlayerSortShipsByDistanceToPoint(kasGrowSelectionPtr(57245931), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion();
}

void Watch_CruiseMissileStrikeWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
    {
        kasfSpeechEvent(139967671, (sdword)Select_CruiseMissileStrikeTargets, 0);
    }
}

void Watch_Mission08_Events_WeCanLeaveCombat(void)
{
    AITeam *team;
    GrowSelection *nearbyTargets;
    GrowSelection *excludedTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0xA5B6425))
    {
        kasfAttackMothership();
        kasfSpeechEvent(
            342540,
            (sdword)Init_AttackMothershipDeleteDelay,
            (sdword)Watch_Mission08_Events_WeCanLeave);
        return;
    }

    if (!kasTimerExpiredDestroyById(150120898))
    {
        return;
    }

    if (!kasfEntryValueGet(71552163))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfMsgSend(kasAITeamPtrIfExists(243787121), (char *)141560536);
    }

    nearbyTargets = kasGrowSelectionPtr(0x98CB5A5);
    kasfShipsSelectNearby(nearbyTargets, kasGrowSelectionPtr(0x4A2A29), kasShipsVectorPtr((char *)0x1D4C), 0x1D4C);
    excludedTargets = kasGrowSelectionPtr(0x58A20);
    kasfShipsSelectType(excludedTargets, nearbyTargets, "sMimic");
    kasfShipsRemove(nearbyTargets, excludedTargets);
    kasfShipsSelectType(excludedTargets, nearbyTargets, "sMcv");
    kasfShipsRemove(nearbyTargets, excludedTargets);
    kasfShipsSelectType(excludedTargets, nearbyTargets, "sCarrier");
    kasfShipsRemove(nearbyTargets, excludedTargets);
    kasfShipsSelectType(excludedTargets, nearbyTargets, "sDreadnought");
    kasfShipsRemove(nearbyTargets, excludedTargets);
    kasfShipsSelectType(excludedTargets, nearbyTargets, "sDestroyer");
    kasfShipsRemove(nearbyTargets, excludedTargets);
    kasfShipsSelectType(excludedTargets, nearbyTargets, "sLeech");
    kasfShipsRemove(nearbyTargets, excludedTargets);
    if (kasfShipsCount(nearbyTargets))
    {
        kasfTeamSpecial(nearbyTargets);
    }
    else
    {
        kasfMoveTo(kasShipsVectorPtrIfExists(211966048));
    }

    kasTimerSetById(150120898, 1);
    kasTimerStartById(150120898);
}

void Init_CruiseMissileStrikeWave_RetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    switch (GetGameDifficulty())
    {
        case 0:
            kasTimerCreateSetStartById(0x8E0C8C2, kasfRandom(300, 600));
            break;
        case 1:
            kasTimerCreateSetStartById(0x8E0C8C2, kasfRandom(200, 400));
            break;
        case 2:
            kasTimerCreateSetStartById(0x8E0C8C2, kasfRandom(120, 300));
            break;
        case 3:
        case 4:
            kasTimerCreateSetStartById(0x8E0C8C2, kasfRandom(90, 120));
            break;
    }
}

void Watch_CruiseMissileStrikeWave_RetryDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(0x8E0C8C2))
    {
        kasfSpeechEvent(
            0x4869685,
            (sdword)Begin_CruiseMissileStrikeWave,
            (sdword)Watch_CruiseMissileStrikeWave);
    }
}

void Select_CruiseMissileStrikeTargets_Refresh(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;
    GrowSelection *priorityTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(
            0x8832365,
            (sdword)Init_CruiseMissileStrikeWave_RetryDelay,
            (sdword)Watch_CruiseMissileStrikeWave_RetryDelay);
        return;
    }
    if (!kasfShipsAnyIdle(&team->shipList))
    {
        return;
    }

    capitalTargets = kasGrowSelectionPtr(124053532);
    if (kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr(0x4A2A29)))
    {
        priorityTargets = kasGrowSelectionPtr(0x36980EB);
        kasfGrowSelectionClear(priorityTargets);
        kasfShipsSelectType(kasGrowSelectionPtr(0x58A20), capitalTargets, "sRammingFrigate");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0x58A20));
        kasfShipsSelectType(kasGrowSelectionPtr(0x58A20), capitalTargets, "sMultiBeamFrigate");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0x58A20));
        kasfShipsSelectType(kasGrowSelectionPtr(0x58A20), capitalTargets, "sHiveFrigate");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0x58A20));
        kasfShipsSelectType(kasGrowSelectionPtr(0x58A20), capitalTargets, "sProcessor");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0x58A20));
        kasfShipsSelectType(kasGrowSelectionPtr(0x58A20), capitalTargets, "sWorker");
        kasfShipsAdd(priorityTargets, kasGrowSelectionPtr(0x58A20));
        if (!kasfShipsCount(priorityTargets))
        {
            kasfShipsAdd(priorityTargets, capitalTargets);
        }
    }
    else
    {
        kasfShipsAdd(kasGrowSelectionPtr(0x36980EB), kasGrowSelectionPtr(0x4A2A29));
    }
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(0x36980EB), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x36980EB));
}

void Watch_BeastAcolyteWave_AwaitSignal(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x7E18873))
    {
        kasfSpeechEvent(75929221, (sdword)Begin_BeastAcolyteWave, (sdword)Watch_BeastAcolyteWave);
    }
}

void Regroup_BeastAcolyteWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfFormationDelta();
    kasfProtect(kasAITeamShipsPtrIfExists(243787121));
}

void Watch_BeastAcolyteWave_Regroup(void)
{
    AITeam *team;
    GrowSelection *attackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectTargets(&team->shipList, kasGrowSelectionPtr(342540)))
    {
        attackTargets = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(attackTargets);
        kasfShipsAdd(attackTargets, kasGrowSelectionPtr(0x4A2A29));
        kasfShipsSortByDistanceToPoint(attackTargets, (vector *)kasThisTeamsVectorPtr());
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
    }
}

void Begin_BeastAcolyteWave(void)
{
    AITeam *team;
    GrowSelection *createdAcolytes;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    createdAcolytes = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(createdAcolytes);
    kasfShipsCreateInside(createdAcolytes, "bAcolyte", 7, kasAITeamShipsPtrIfExists(170794736), 1);
    kasAITeamShipsPtr(&team->shipList, 1);
    kasfReinforceTeamWithShips(team, createdAcolytes);
}

void Watch_BeastAcolyteWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(170794736)))
    {
        kasfSpeechEvent(139967671, (sdword)Regroup_BeastAcolyteWave, (sdword)Watch_BeastAcolyteWave_Regroup);
    }
}

void Watch_BeastReconWave_AwaitSignal(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0x7E18873))
    {
        kasfSpeechEvent(75929221, (sdword)Begin_BeastReconWave, (sdword)Watch_BeastReconWave);
    }
}

void Regroup_BeastReconWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfProtect(kasAITeamShipsPtrIfExists(243787121));
}

void Watch_BeastReconWave_Regroup(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
    }
}

void Begin_BeastReconWave(void)
{
    AITeam *team;
    GrowSelection *createdRecons;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    createdRecons = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(createdRecons);
    kasfShipsCreateInside(createdRecons, "bRecon", 10, kasAITeamShipsPtrIfExists(216029280), 1);
    kasfReinforceTeamWithShips(team, createdRecons);
}

void Watch_BeastReconWave(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(139967671, (sdword)Regroup_BeastReconWave, (sdword)Watch_BeastReconWave_Regroup);
    }
}


extern void Init_Mission12_GuardGravWells_AttackMothership(void);
extern void Watch_Mission12_GuardGravWells_AttackMothership(void);
extern void Init_Mission08_DockedAttackTeam_AttackMothership(void);

void Init_Mission08(void)
{
    kasCurrentStep = 0;
    scope = KH_mission08;

    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));
    kasfMusicPlay(7);

    kasFSMCreate(KH_FleetIntel, Init_Mission08_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission12_GuardGravWells_AttackMothership,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(22617, Init_Mission08_SpyTeam_AttackMothership, Watch_Mission08_SpyTeam_AttackMothership,
                 kasAITeamPtrIfExists(22617));
    kasFSMCreate(123922546, Init_Mission08_DockedAttackTeam_AttackMothership, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(123922546));
    kasFSMCreate(130285475, Init_Mission12_GuardGravWells_AttackMothership, Watch_Mission08_GuardGravWells_AttackMothership,
                 kasAITeamPtrIfExists(130285475));
    kasFSMCreate(130285475, Init_Mission12_GuardGravWells_AttackMothership, Watch_Mission08_GuardGravWells_AttackMothership,
                 kasAITeamPtrIfExists(139498707));
    kasFSMCreate(130285475, Init_Mission12_GuardGravWells_AttackMothership, Watch_Mission08_GuardGravWells_AttackMothership,
                 kasAITeamPtrIfExists(91786963));
    kasFSMCreate(77795571, Init_Mission08_ResourceCollectorGroup_AttackMothership, Watch_Mission08_ResourceCollectorGroup_BreakAttachmentsAndSignalFrigates,
                 kasAITeamPtrIfExists(77795571));
    kasFSMCreate(115043347, Init_Mission08_FrigateGroup_AttackMothership, Watch_Mission08_FrigateGroup_BreakAttachmentsAndAttack,
                 kasAITeamPtrIfExists(115043347));
    kasFSMCreate(145052708, Init_Mission08_FrigateGroup2_AttackMothership, Watch_Mission08_FrigateGroup2_BreakAttachmentsAndAttack,
                 kasAITeamPtrIfExists(145052708));
    kasFSMCreate(139033868, Init_Mission08_FrigateGroup3_AttackMothership, Watch_Mission08_FrigateGroup3_BreakAttachmentsAndAttack,
                 kasAITeamPtrIfExists(139033868));
    kasFSMCreate(132678291, Init_Mission08_CrystalHarvesterTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(132678291));

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
    kasfModuleSlotOrderSetShipType(1, "tResourceCollector");
    kasfModuleSlotOrderSetShipType(1, "tIonCannonFrigate");
    kasfModuleSlotOrderSetShipType(1, "tDFGFrigate");
    kasfModuleSlotOrderSetShipType(1, "tGravWellGenerator");
    kasfModuleSlotOrderSetCount(1, 1);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_REPAIR");
    kasfAllowPlayerToResearch("TECH_SALVAGE");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_RESEARCH_CLEESAN_ATTACK");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
    kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");
    kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
    kasfAllowPlayerToResearch("TECH_EMP");
    kasfAllowPlayerToResearch("TECH_ARMOUR2");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");
    kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");

    kasfActivateTech("TECH_ENERGY");
    kasfActivateTech("TECH_REPAIR");

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipMicro", "RACE_SECT", 1);
    kasfEnableShip("sMothershipSpecial", "RACE_SECT", 1);
    kasfEnableShip("sMothershipArmour", "RACE_SECT", 1);
    kasfEnableShip("sMothershipWeapons", "RACE_SECT", 1);

    kasfDisableAllAIFeatures();
    kasfDisablePlayerHyperspace();
    kasfMusicPlay(7);
    kasfLocationCard(8000, "Deep Space (Koreth's Rift Sector)");
}
