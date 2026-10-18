/*=============================================================================
    Name    : mission15.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission15
=============================================================================*/

#include "MissionScriptSupport.h"

static char *m15_saveInfoName[] =
{
    "Deep Space (Gulf Sector)",
    "French version goes here",
    "German version goes here",
};

static char *m15_blockObjectiveName[] =
{
    "Block Slipgate Entrance",
    "French version goes here",
    "German version goes here",
};

static char *m15_blockObjectiveDescription[] =
{
    "Prevent further Bentusi stations from leaving by moving your Command Ship in front of the slipgate.",
    "French version goes here",
    "German version goes here",
};

static char *m15_destroyGateNodesObjectiveName[] =
{
    "Destroy Gate Nodes",
    "French version goes here",
    "German version goes here",
};

static char *m15_destroyGateNodesObjectiveDescription[] =
{
    "You must destroy all slipgate nodes in order to collapse the slipgate.  Closing the gate will keep the Bentusi from evacuating.  If we can't keep them here then we have no chance against the Beast.",
    "French version goes here",
    "German version goes here",
};

static char *m15_defendCommandShipObjectiveName[] =
{
    "Defend Command Ship",
    "French version goes here",
    "German version goes here",
};

static char *m15_defendCommandShipObjectiveDescription[] =
{
    "Defend your Command Ship from the Bentusi super fighters.",
    "French version goes here",
    "German version goes here",
};

static char *m15_hyperspaceObjectiveName[] =
{
    "Hyperspace",
    "French version goes here",
    "German version goes here",
};

static char *m15_hyperspaceObjectiveDescription[] =
{
    "Rally the fleet and prepare to hyperspace on your mark.",
    "French version goes here",
    "German version goes here",
};

static char *m15_endMissionSaveInfoName[] =
{
    "~ End of Mission",
    "French version goes here",
    "German version goes here",
};

void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void);
void Init_Mission15_MissionFlow(void);
void Watch_Mission15_MissionFlow(void);
void Watch_Mission15_Events_FIIntro1(void);
void Watch_Mission15_Events_GateNodeObjectives(void);
void Init_Mission15_Events_GateNodeCollapse(void);
void Watch_Mission15_Events_GateNodeCollapse(void);
void Init_Mission15_GateNodeDefenseTeam(void);
void Watch_Mission15_GateNodeDefenseTeam(void);
void Init_Mission15_GateNodeDefenseTeam_MoveToWaypoint(void);
void Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint(void);
void Init_Mission15_DAcolyteReinforcementTeam(void);
void Watch_Mission15_DAcolyteReinforcementTeam(void);
void Init_Mission15_DAcolyteReinforcementDelay(void);
void Watch_Mission15_DAcolyteReinforcementDelay(void);
void Begin_Mission15_DAcolyteCombatLoop(void);
void Watch_Mission15_DAcolyteCombatLoop(void);
void Begin_Mission15_GateNodeDefenseCombatLoop(void);
void Watch_Mission15_GateNodeDefenseCombatLoop(void);
void Begin_Mission15_GateNodeDefensePatrol(void);
void Watch_Mission15_GateNodeDefensePatrol(void);
void Init_Mission15_AttackMothershipTeam(void);
void Watch_Mission15_AttackMothershipTeam(void);
void Init_Mission15_Events_GateNodeCollapse_CheckGateNode(void);
void Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint_Resume(void);
void Init_Mission15_GateNodeDefenseTeam_DockStay(void);
void Watch_Mission15_GateNodeDefenseTeam_Reposition(void);
void Init_Mission16_Events_HoneyImHome(void);
void TeamHaltAndClearMoveOrders(void);
void kasSpeechEventStopTeam(void);

void Watch_Mission15(void)
{
  int v0; // eax
  int *v1; // eax
  int *v2; // eax
  int v3; // eax
  int *v4; // eax
  int v5; // eax
  int *v6; // eax
  int v7; // eax
  int *v8; // eax
  int v9; // eax
  int *v10; // eax
  int v11; // eax
  int *v12; // eax
  int v13; // eax
  int *v14; // eax
  int v15; // eax
  int *v16; // eax
  int v17; // eax
  int *v18; // eax
  int v19; // eax
  int *v20; // eax
  int v21; // eax
  int *v22; // eax
  int v23; // eax
  int *v24; // eax
  int v25; // eax
  int *v26; // eax
  signed int v27; // eax
  int v28; // eax
  int *v29; // eax
  int *v30; // eax
  int *v31; // eax
  int v32; // eax
  int *v33; // eax
  int v34; // eax
  int v35; // eax
  int *v36; // eax
  int v37; // eax
  int v38; // eax
  int *v39; // eax
  int *v40; // eax
  int v41; // eax
  int *v42; // eax
  int v43; // eax
  int v44; // eax
  int v45; // eax
  int v46; // eax
  int v47; // eax
  int v48; // eax
  int *v49; // eax
  int v50; // eax
  int v51; // eax
  int v52; // eax
  int *v53; // eax
  int *v54; // eax
  int *v55; // eax
  int *v56; // eax
  int *v57; // eax
  int v58; // esi
  int v59; // eax
  int v60; // esi
  int v61; // eax
  int v62; // esi
  int v63; // eax
  int v64; // esi
  int v65; // eax
  int v66; // esi
  int *v67; // eax
  int *v68; // eax
  int v69; // eax
  int *v70; // eax
  int *v71; // eax
  int v73; // eax
  int *v74; // eax
  int v75; // [esp-Ch] [ebp-Ch]
  int v76; // [esp-Ch] [ebp-Ch]
  int v77; // [esp-Ch] [ebp-Ch]
  int v78; // [esp-8h] [ebp-8h]
  int v79; // [esp-8h] [ebp-8h]
  int v80; // [esp-8h] [ebp-8h]
  int v81; // [esp-8h] [ebp-8h]
  int v82; // [esp-8h] [ebp-8h]
  int v83; // [esp-8h] [ebp-8h]
  int v84; // [esp-8h] [ebp-8h]
  int v85; // [esp-8h] [ebp-8h]
  int v86; // [esp-8h] [ebp-8h]
  int v87; // [esp-8h] [ebp-8h]
  int v88; // [esp-8h] [ebp-8h]
  int v89; // [esp-8h] [ebp-8h]
  int v90; // [esp-8h] [ebp-8h]
  int v91; // [esp-8h] [ebp-8h]
  int v92; // [esp-8h] [ebp-8h]
  int v93; // [esp-8h] [ebp-8h]
  int v94; // [esp-8h] [ebp-8h]
  int v95; // [esp-8h] [ebp-8h]
  int v96; // [esp-8h] [ebp-8h]
  int v97; // [esp-8h] [ebp-8h]
  int v98; // [esp-8h] [ebp-8h]
  int v99; // [esp-8h] [ebp-8h]
  int v100; // [esp-8h] [ebp-8h]
  int v101; // [esp-4h] [ebp-4h]
  int v102; // [esp-4h] [ebp-4h]
  int v103; // [esp-4h] [ebp-4h]
  int *v104; // [esp-4h] [ebp-4h]
  int *v105; // [esp-4h] [ebp-4h]
  int *v106; // [esp-4h] [ebp-4h]
  int *v107; // [esp-4h] [ebp-4h]
  int *v108; // [esp-4h] [ebp-4h]
  int *v109; // [esp-4h] [ebp-4h]
  int *v110; // [esp-4h] [ebp-4h]
  int *v111; // [esp-4h] [ebp-4h]
  int *v112; // [esp-4h] [ebp-4h]
  int *v113; // [esp-4h] [ebp-4h]
  int *v114; // [esp-4h] [ebp-4h]
  int *v115; // [esp-4h] [ebp-4h]
  int *v116; // [esp-4h] [ebp-4h]
  int v117; // [esp-4h] [ebp-4h]
  int v118; // [esp-4h] [ebp-4h]
  int v119; // [esp-4h] [ebp-4h]
  int *v120; // [esp-4h] [ebp-4h]
  int v121; // [esp-4h] [ebp-4h]
  int v122; // [esp-4h] [ebp-4h]
  int v123; // [esp-4h] [ebp-4h]
  int v124; // [esp-4h] [ebp-4h]
  int v125; // [esp-4h] [ebp-4h]
  int v126; // [esp-4h] [ebp-4h]
  int v127; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503621;
  if ( kasTimerExpiredDestroyById(104338707) )
  {
    v101 = kasGrowSelectionPtr(18336307);
    v0 = kasVolumePtr(132933767);
    kasfFindShipsInside(v0, v101);
    v102 = kasGrowSelectionPtr(18336307);
    v1 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v1, v102);
    v103 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v2, v103);
    v78 = kasGrowSelectionPtr(0x4A2A29);
    v3 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v3, v78, "sMothershipSpecial");
    v104 = (int *)kasGrowSelectionPtr(5846404);
    v4 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v4, v104);
    v79 = kasGrowSelectionPtr(0x4A2A29);
    v5 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v5, v79, "sMothershipEngineering");
    v105 = (int *)kasGrowSelectionPtr(5846404);
    v6 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v6, v105);
    v80 = kasGrowSelectionPtr(0x4A2A29);
    v7 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v7, v80, "sMothershipArmour");
    v106 = (int *)kasGrowSelectionPtr(5846404);
    v8 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v8, v106);
    v81 = kasGrowSelectionPtr(0x4A2A29);
    v9 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v9, v81, "sMothershipMicro");
    v107 = (int *)kasGrowSelectionPtr(5846404);
    v10 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v10, v107);
    v82 = kasGrowSelectionPtr(0x4A2A29);
    v11 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v11, v82, "sMothershipWeapons");
    v108 = (int *)kasGrowSelectionPtr(5846404);
    v12 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v12, v108);
    v83 = kasGrowSelectionPtr(0x4A2A29);
    v13 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v13, v83, "sMothershipDockingBay");
    v109 = (int *)kasGrowSelectionPtr(5846404);
    v14 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v14, v109);
    v84 = kasGrowSelectionPtr(0x4A2A29);
    v15 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v15, v84, "sMothershipSupport");
    v110 = (int *)kasGrowSelectionPtr(5846404);
    v16 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v16, v110);
    v85 = kasGrowSelectionPtr(0x4A2A29);
    v17 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v17, v85, "sMothershipCommand");
    v111 = (int *)kasGrowSelectionPtr(5846404);
    v18 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v18, v111);
    v86 = kasGrowSelectionPtr(0x4A2A29);
    v19 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v19, v86, "sMothershipEnginesLower");
    v112 = (int *)kasGrowSelectionPtr(5846404);
    v20 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v20, v112);
    v87 = kasGrowSelectionPtr(0x4A2A29);
    v21 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v21, v87, "sMothershipEnginesUpper");
    v113 = (int *)kasGrowSelectionPtr(5846404);
    v22 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v22, v113);
    v88 = kasGrowSelectionPtr(0x4A2A29);
    v23 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v23, v88, "sMothershipLowerSpine");
    v114 = (int *)kasGrowSelectionPtr(5846404);
    v24 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v24, v114);
    v89 = kasGrowSelectionPtr(0x4A2A29);
    v25 = kasGrowSelectionPtr(5846404);
    kasfShipsSelectType(v25, v89, "sMothershipBigGun");
    v115 = (int *)kasGrowSelectionPtr(5846404);
    v26 = (int *)kasGrowSelectionPtr(218060896);
    kasfShipsAdd(v26, v115);
    v27 = kasfRandom(5, 10);
    kasTimerSetById(104338707, v27);
    kasTimerStartById(104338707);
  }
  v116 = sub_490820(218060896);
  v28 = kasGrowSelectionPtr(141092787);
  sub_49A100(v28, (float *)v116);
  v90 = kasGrowSelectionPtr(141092787);
  v29 = (int *)kasGrowSelectionPtr(4929699);
  sub_495950(v29, v90, 0);
  if ( !kasfEntryValueGet(71552164) && kasfEntryValueGet(-2015429404) )
  {
    kasfEntrySetValue(71552164, 1);
    v117 = kasGrowSelectionPtr(18336307);
    v30 = (int *)kasGrowSelectionPtr(89157026);
    kasSelectPlayerShipsSimple(v30, v117);
    v118 = kasGrowSelectionPtr(89157026);
    v31 = (int *)kasGrowSelectionPtr(89157026);
    kasfShipsSelectNotSalvagers(v31, v118);
    v119 = kasGrowSelectionPtr(218060896);
    v32 = kasGrowSelectionPtr(82017669);
    if ( kasfShipsSelectLinkedShips(v32, v119) )
    {
      v120 = (int *)kasGrowSelectionPtr(82017669);
      v33 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsAdd(v33, v120);
    }
    if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
    {
      v91 = kasGrowSelectionPtr(89157026);
      v34 = kasGrowSelectionPtr(96759714);
      if ( !kasfShipsSelectType(v34, v91, "sWorker") )
      {
        v92 = kasGrowSelectionPtr(218060896);
        v35 = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(v35, "sWorker", 1, v92, 0);
      }
    }
    v36 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v36, (char *)154206436);
    spSetSinglePlayerSaveInfo(15, m15_saveInfoName[strCurLanguage]);
  }
  if ( !kasfEntryValueGet(71552165) && !kasfEntryValueGet(-1925829575) )
  {
    v121 = kasGrowSelectionPtr(139967715);
    v37 = kasGrowSelectionPtr((int)&loc_4A2A39);
    if ( sub_498630(v37, v121) )
    {
      v93 = kasGrowSelectionPtr(114961833);
      v38 = kasGrowSelectionPtr((int)sub_58A4A2);
      if ( kasfShipsSelectType(v38, v93, "dAcolyte") )
      {
        v122 = kasGrowSelectionPtr((int)sub_58A4A2);
        v94 = kasGrowSelectionPtr(139967715);
        v39 = (int *)kasGrowSelectionPtr(139967715);
        if ( sub_495410(v39, v94, v122) )
        {
          kasfEntrySetValue(71552165, 1);
          v40 = kasAITeamPtrIfExists(176850956);
          kasfMsgSend((struct AITeam *)(int)v40, (char *)109760960);
        }
      }
    }
  }
  if ( !kasfEntryValueGet(71552166) )
  {
    v41 = kasGrowSelectionPtr(141092787);
    if ( !kasfShipsCount(v41) )
    {
      kasfEntrySetValue(71552166, 1);
      v42 = kasAITeamPtrIfExists(160291907);
      kasfMsgSend((struct AITeam *)(int)v42, (char *)55426788);
    }
  }
  if ( !kasfEntryValueGet(71552167) && kasTimerExpiredById(-2142420652) )
  {
    kasfEntrySetValue(71552167, 1);
    v95 = kasGrowSelectionPtr(0x4A2A29);
    v43 = kasGrowSelectionPtr(28678146);
    v44 = sub_495640(v43, v95, "CLASS_HeavyCruiser");
    kasfEntrySetValue(-1920416254, v44);
    v96 = kasGrowSelectionPtr(0x4A2A29);
    v45 = kasGrowSelectionPtr(120990258);
    v46 = sub_495640(v45, v96, "CLASS_Carrier");
    kasfEntrySetValue(-2065446158, v46);
    v97 = kasGrowSelectionPtr(0x4A2A29);
    v47 = kasGrowSelectionPtr(164812114);
    v48 = sub_495640(v47, v97, "CLASS_Destroyer");
    kasfEntrySetValue(-1942874494, v48);
    v123 = kasGrowSelectionPtr(0x4A2A29);
    v49 = (int *)kasGrowSelectionPtr(120958924);
    v50 = sub_4956E0(v49, v123);
    kasfEntrySetValue(-2065674484, v50);
    v51 = kasGrowSelectionPtr(218060896);
    v52 = kasfShipsHealthAverage(v51);
    kasfEntrySetValue(-2047792136, v52);
    kasfEntrySetValue(203668275, 1);
    GetGameDifficulty();
    switch ( GetGameDifficulty() )
    {
      case 0:
        kasTimerSetById(90794046, 120);
        break;
      case 1:
        kasTimerSetById(90794046, 300);
        break;
      case 2:
        kasTimerSetById(90794046, 600);
        break;
      case 3:
        kasTimerSetById(90794046, 1200);
        break;
      case 4:
        kasTimerSetById(90794046, 1500);
        break;
      default:
        break;
    }
    v53 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v53, (char *)74556980);
    v54 = kasAITeamPtrIfExists(138055537);
    kasfMsgSend((struct AITeam *)(int)v54, (char *)5801588);
    v55 = kasAITeamPtrIfExists(138055538);
    kasfMsgSend((struct AITeam *)(int)v55, (char *)5801588);
    v56 = kasAITeamPtrIfExists(138055539);
    kasfMsgSend((struct AITeam *)(int)v56, (char *)5801588);
  }
  if ( kasfEntryValueGet(203668275) )
  {
    if ( !kasfEntryValueGet(71552168) )
    {
      if ( kasTimerExpiredById(90794046) )
        goto LABEL_38;
      v98 = kasGrowSelectionPtr(0x4A2A29);
      v57 = (int *)kasGrowSelectionPtr(120958924);
      v58 = sub_4956E0(v57, v98);
      if ( v58 < kasfEntryValueGet(-2065674484) / 2 )
        goto LABEL_38;
      v75 = kasGrowSelectionPtr(0x4A2A29);
      v59 = kasGrowSelectionPtr(164812114);
      v60 = sub_495640(v59, v75, "CLASS_Destroyer");
      if ( v60 < kasfEntryValueGet(-1942874494) )
        goto LABEL_38;
      v76 = kasGrowSelectionPtr(0x4A2A29);
      v61 = kasGrowSelectionPtr(28678146);
      v62 = sub_495640(v61, v76, "CLASS_HeavyCruiser");
      if ( v62 < kasfEntryValueGet(-1920416254)
        || (v77 = kasGrowSelectionPtr(0x4A2A29),
            v63 = kasGrowSelectionPtr(120990258),
            v64 = sub_495640(v63, v77, "CLASS_Carrier"),
            v64 < kasfEntryValueGet(-2065446158))
        || (v65 = kasGrowSelectionPtr(218060896), v66 = kasfShipsHealthAverage(v65), v66 <= kasfEntryValueGet(-2047792136) / 2) )
      {
LABEL_38:
        kasfEntrySetValue(71552168, 1);
        v67 = kasAITeamPtrIfExists(176850956);
        kasfMsgSend((struct AITeam *)(int)v67, (char *)74556981);
        kasfEntryGetOrCreateSetValue(203668275, 0);
      }
    }
  }
  if ( !kasfEntryValueGet(71552169) && kasTimerExpiredById(-1925077303) )
  {
    kasfEntrySetValue(71552169, 1);
    v68 = kasAITeamPtrIfExists(176850956);
    kasfMsgSend((struct AITeam *)(int)v68, (char *)74556982);
  }
  if ( kasfEntryValueGet(-2094624789) )
  {
    if ( !kasfEntryValueGet(118718420) )
    {
      v124 = kasGrowSelectionPtr(82017669);
      v69 = kasVolumePtr((int)&loc_47137B);
      if ( kasfFindShipsInside(v69, v124) )
      {
        v125 = kasGrowSelectionPtr(218060896);
        v99 = kasGrowSelectionPtr(82017669);
        v70 = (int *)kasGrowSelectionPtr(82017669);
        if ( sub_495410(v70, v99, v125) )
        {
          if ( !kasfEntryValueGet(71552170) )
          {
            kasfEntrySetValue(71552170, 1);
            v71 = kasAITeamPtrIfExists(160291907);
            kasfMsgSend((struct AITeam *)(int)v71, (char *)118718420);
          }
          kasfEntrySetValue(118718420, 1);
          kasfMsgSendAll(217560638);
        }
      }
    }
  }
  if ( kasfEntryValueGet(-2094624789) )
  {
    if ( kasfEntryValueGet(118718420) )
    {
      v126 = kasGrowSelectionPtr(82017669);
      v73 = kasVolumePtr((int)&loc_47137B);
      if ( kasfFindShipsInside(v73, v126) )
      {
        v127 = kasGrowSelectionPtr(218060896);
        v100 = kasGrowSelectionPtr(82017669);
        v74 = (int *)kasGrowSelectionPtr(82017669);
        if ( !sub_495410(v74, v100, v127) )
        {
          kasfEntryGetOrCreateSetValue(118718420, 0);
          kasfMsgSendAll(90868245);
        }
      }
    }
  }
}

void Init_Mission15_MissionFlow(void)
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
    kasfEntryGetOrCreateSetValue(5801605, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission15_MissionFlow);
}

void Watch_Mission15_Events_FIIntro1(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr((char *)93584908));
    kasfShipsAdd(kasGrowSelectionPtr((char *)93584908), kasGrowSelectionPtr((char *)114961833));
    kasfShipsSelectType(kasGrowSelectionPtr((char *)363040), kasGrowSelectionPtr((char *)93584908), "bWorker");
    kasfShipsRemove(kasGrowSelectionPtr((char *)93584908), kasGrowSelectionPtr((char *)363040));
    kasfShipsSelectType(kasGrowSelectionPtr((char *)363040), kasGrowSelectionPtr((char *)93584908), "bProcessor");
    if (!kasfEntryValueGet(71552162) &&
        !kasfShipsCount(kasGrowSelectionPtr((char *)93584908)))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfObjectiveDestroy("DestroyBeast");
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x0C1CF74);
    }
    if (kasfMsgReceived((char *)175938357))
    {
        kasfObjectiveCreate("Hyperspace", m15_hyperspaceObjectiveName[strCurLanguage], m15_hyperspaceObjectiveDescription[strCurLanguage]);
        spSetSinglePlayerSaveInfo(16, m15_endMissionSaveInfoName[strCurLanguage]);
    }
}

void Watch_Mission15_MissionFlow(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    if (!kasfShipsCount(kasGrowSelectionPtr((char *)218060896)))
    {
        kasfMissionFailed();
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
        return;
    }

    switch (kasfEntryValueGet(5801605))
    {
        case 1:
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
                kasfEntrySetValue(5801605, 2);
            }
            return;

        case 2:
            if (kasTimerExpiredDestroyById(144337492))
            {
                kasfEntrySetValue(5801605, 0);
            }
            return;

        default:
            break;
    }

    if (!kasfEntryValueGet(0x5137B))
    {
        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        queueIndex = (queueInfo != NULL) ? *queueInfo : -1;
        while (queueIndex >= 0)
        {
            message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
            switch (message)
            {
                case 0:
                    break;
                case 0x1A63212:
                    kasTimerSetById(144337492, 3);
                    kasTimerStartById(144337492);
                    kasfWideScreenIn(90);
                    kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A6322);
                    kasfEntrySetValue(5801605, 2);
                    break;
                case 0x1F36982:
                    kasTimerSetById(144337492, 3);
                    kasfEntrySetValue(0x79D69C2, 1);
                    kasfEntrySetValue(0x0FDBD41E, 1);
                    kasfEntrySetValue(0x5137B, 0);
                    kasfEntrySetValue(0xA4E5AD3, 1);
                    kasfSpeechEventQueueCurrentPlayer(0x1000225B, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(0x68A3735), (char *)0x588674);
                    kasfMsgSend(kasAITeamPtrIfExists(0x98DDC43), (char *)0xE1F3695);
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x471A632:
                    kasfEntrySetValue(0x5137B, 1);
                    kasfSpeechEventQueueCurrentPlayer(0x1002025C, 0);
                    if (kasfShipsCount(kasGrowSelectionPtr((char *)0x4B38A3)))
                    {
                        kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x1A63212);
                    }
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x471A633:
                    kasfEntrySetValue(0x5137B, 1);
                    kasfSpeechEventQueueCurrentPlayer(0x10020262, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A6332);
                    kasfMsgSend(kasAITeamPtrIfExists(0x98DDC43), (char *)0x8970534);
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x471A634:
                    kasfEntrySetValue(0x5137B, 1);
                    kasfSpeechEventQueueCurrentPlayer(0x10002265, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63C2);
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x471A635:
                    kasfEntrySetValue(0x5137B, 1);
                    kasfSpeechEventQueueCurrentPlayer(0x10001267, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63D2);
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x471A636:
                    kasfEntrySetValue(0x5137B, 1);
                    kasfSpeechEventQueueCurrentPlayer(0x1000226A, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E2);
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x68AD1C0:
                    kasfSpeechEventQueueCurrentPlayer(0x10002260, 0);
                    kasfMsgSend(kasAITeamPtrIfExists(0x98DDC43), (char *)0x68AD1C0);
                    if (kasfEntryValueGet(0x89F9A664) >= 2)
                    {
                        kasfEntrySetValue(0x5137B, 1);
                        kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x89AA212);
                    }
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
                    return;
                case 0x93100E4:
                    kasfEntrySetValue(0x5137B, 1);
                    kasfLocationCard(0x1F40, m15_saveInfoName[strCurLanguage]);
                    kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0xE1F3695);
                    kasfEntrySetValue(5801605, 1);
                    kasfIntelEventNotEnded();
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
        return;
    }

    queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueInfo != NULL) ? *queueInfo : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        switch (message)
        {
            case 0:
                break;
            case 0x1A63212:
                kasTimerSetById(144337492, 3);
                kasTimerStartById(144337492);
                kasfWideScreenIn(90);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A6322);
                kasfEntrySetValue(5801605, 2);
                break;
            case 0x1F36982:
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(0x79D69C2, 1);
                kasfEntrySetValue(0x0FDBD41E, 1);
                kasfEntrySetValue(0x5137B, 0);
                kasfEntrySetValue(0xA4E5AD3, 1);
                kasfSpeechEventQueueCurrentPlayer(0x1000225B, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0x68A3735), (char *)0x588674);
                kasfMsgSend(kasAITeamPtrIfExists(0x98DDC43), (char *)0xE1F3695);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A6322:
                kasTimerSetById(144337492, 3);
                kasfEntrySetValue(0x79D69C2, 1);
                kasfEntrySetValue(0x0FDBD41E, 1);
                kasfEntrySetValue(0xA4E5AD3, 1);
                kasfEntryGetOrCreateSetValue(0x0F8BFFC5, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(0x3A1CFFE, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(0x3D4BA05, kasfCameraGetDistance());
                kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr((char *)0x4B38A3));
                kasfCameraSetAngleDeg(208);
                kasfCameraSetDeclinationDeg(-22);
                kasfCameraSetDistance(3481);
                kasfSpeechEventQueueCurrentPlayer(0x1000225E, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A6323);
                kasfMsgSend(kasAITeamPtrIfExists(0xADBD8E2), (char *)0x0B1FF3D3);
                kasfMusicPlay(23);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A6323:
                kasfEntryGetOrCreateSetValue(0x83268FEB, 1);
                kasfSpeechEventQueueCurrentPlayer(0x1000125F, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0x98DDC43), (char *)0x47137B);
                kasfEntrySetValue(0x5137B, 0);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A6332:
                kasfSpeechEventQueueCurrentPlayer(0x10001263, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A6333);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A6333:
                kasfEntrySetValue(0x5137B, 0);
                kasfSpeechEventQueueCurrentPlayer(0x10020264, 0);
                kasTimerCreateSetStartById(0x804D4154, 15);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63C2:
                kasfEntrySetValue(0x5137B, 0);
                kasfSpeechEventQueueCurrentPlayer(0x10001266, 0);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63D2:
                kasfMsgSendAll((char *)0x628EC3E);
                kasfEntryGetOrCreateSetValue(0x86E54005, 1);
                kasfHaltAllNotDockedShips();
                kasfSpeechEventQueueCurrentPlayer(0x10020268, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63D3);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63D3:
                kasfEntrySetValue(0x5137B, 0);
                kasfSpeechEventQueueCurrentPlayer(0x10001269, 0);
                kasfMusicPlay(14);
                kasTimerCreateSetStartById(0x8D41A6C9, 20);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E2:
                kasfSpeechEventQueueCurrentPlayer(0x1002026B, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E3);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E3:
                kasfSpeechEventQueueCurrentPlayer(0x1000126C, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E4);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E4:
                kasfSpeechEventQueueCurrentPlayer(0x1002026D, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E5);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E5:
                kasfSpeechEventQueueCurrentPlayer(0x1000126E, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E6);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E6:
                kasfSpeechEventQueueCurrentPlayer(0x1002026F, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E7);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E7:
                kasfSpeechEventQueueCurrentPlayer(0x10001270, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x71A63E8);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x71A63E8:
                kasfSpeechEventQueueCurrentPlayer(0x10020271, 0);
                kasfMsgSend(kasAITeamPtrIfExists(0x98DDC43), (char *)0xA7C9B35);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0x89AA212:
                kasfEntrySetValue(0x5137B, 0);
                kasfSpeechEventQueueCurrentPlayer(0x10002261, 0);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
                return;
            case 0xE1F3695:
                kasfEntryGetOrCreateSetValue(0x0F8BFFC5, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(0x3A1CFFE, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(0x3D4BA05, kasfCameraGetDistance());
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(0x68A3735));
                kasfCameraSetAngleDeg(344);
                kasfCameraSetDeclinationDeg(-7);
                kasfCameraSetDistance(10150);
                kasfLocationCard(5000, " ");
                kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x1F36982);
                kasfEntrySetValue(5801605, 1);
                kasfIntelEventNotEnded();
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

void Watch_Mission15_Events_GateNodeObjectives(void)
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
            case 0x47137B:
                kasfObjectiveCreate("Block", m15_blockObjectiveName[strCurLanguage], m15_blockObjectiveDescription[strCurLanguage]);
                break;
            case 0x34DBEE4:
                kasfMissionFailed();
                break;
            case 0x68AD1C0:
                kasfObjectiveCreate("DefendMothership", m15_defendCommandShipObjectiveName[strCurLanguage], m15_defendCommandShipObjectiveDescription[strCurLanguage]);
                break;
            case 0x7137FD4:
                kasfObjectiveDestroy("Block");
                break;
            case 0x8970534:
                kasfObjectiveDestroy("DestroyGateNode");
                break;
            case 0xA7C9B35:
                kasfObjectiveDestroyAll();
                kasfObjectiveCreate("Hyperspace", m15_hyperspaceObjectiveName[strCurLanguage], m15_hyperspaceObjectiveDescription[strCurLanguage]);
                spSetSinglePlayerSaveInfo(15, m15_saveInfoName[strCurLanguage]);
                break;
            case 0xE1F3695:
                kasfPingAddShipsIndexed(kasAITeamShipsPtrIfExists(109721397), "PingGateNodes");
                kasfObjectiveCreate("DestroyGateNode", m15_destroyGateNodesObjectiveName[strCurLanguage], m15_destroyGateNodesObjectiveDescription[strCurLanguage]);
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

void Init_Mission15_Events_GateNodeCollapse(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(0x549575, (sdword)MaintainTeamContext, (sdword)Init_Mission15_Events_GateNodeCollapse_CheckGateNode);
}

void Init_Mission15_Events_GateNodeCollapse_CheckGateNode(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)) &&
        kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)&loc_4A2A29), kasShipsVectorPtr((char *)0x4E20), 0x4E20) &&
        kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x36980EB)))
    {
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)0x471A632);
        kasfMsgSendAll((char *)0x8608025);
        kasfSpeechEvent(0x9857BF4, (sdword)MaintainTeamContext, (sdword)Watch_Mission15_Events_GateNodeCollapse);
        return;
    }
    if (kasfTeamCount() < kasfTeamCountOriginal())
    {
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)0x471A632);
        }
        kasfSpeechEvent(0x9857BF4, (sdword)MaintainTeamContext, (sdword)Watch_Mission15_Events_GateNodeCollapse);
    }
}

void Watch_Mission15_Events_GateNodeCollapse(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(71552161) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfEntryGetOrCreateSetValue(-1925829575, 1);
        kasfEntrySetValue(-2094624789, 0);
        kasfCreateBasicShockwave(kasShipsVectorPtrIfExists((char *)0x4AD39A3));
        kasfDeleteShips(kasGrowSelectionPtr((char *)0x4AD39A3));
        kasfDeleteShips(kasGrowSelectionPtr((char *)0xAD39A02));
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)0x471A633);
        kasfMsgSend(kasAITeamPtrIfExists(159243331), (char *)0x7137FD4);
        kasfSpeechEvent(0x53A0C, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission15_GateNodeDefenseTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSetPercentDamaged(&teamtoreinforce->shipList, 200);
    kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x868E7B3), &teamtoreinforce->shipList);
    kasfShipsSetNonRetaliation(&teamtoreinforce->shipList);
    kasfEntryGetOrCreateSetValue(0x5137B, 0);
    kasfEntryGetOrCreateSetValue(0x8209731B, 0);
    kasfSpeechEvent(0x44E27E5, (sdword)Init_Mission15_GateNodeDefenseTeam_MoveToWaypoint, (sdword)Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint);
}

void Watch_Mission15_GateNodeDefenseTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(0x5137B))
    {
        if (!kasfEntryValueGet(0x5D8A374) &&
            (kasfEntryValueGet(0x8D362C39) || kasfMsgReceived((char *)0x0CF7B63E)))
        {
            kasfEntryGetOrCreateSetValue(0x5D8A374, 1);
            kasfSpeechEvent(0x5B5E4, (sdword)kasSpeechEventStopTeam, (sdword)Watch_Mission15_GateNodeDefenseTeam_Reposition);
            return;
        }
        if (!kasfTeamCount())
        {
            kasfSpeechEvent(0x53A0C, (sdword)MaintainTeamContext, (sdword)sub_65F6F0);
            return;
        }
        if (kasfMsgReceived((char *)0x628EC3E))
        {
            kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
            kasfShipsSetNonRetaliation(&teamtoreinforce->shipList);
            kasfEntryGetOrCreateSetValue(0x628EC3E, 1);
        }
        if (!kasfEntryValueGet(0x443CCA2) &&
            !kasfEntryValueGet(0x628EC3E) &&
            (kasfMsgReceived((char *)0x8608025) ||
             (kasfUnderAttack(kasGrowSelectionPtr((char *)0x857BCE3)) &&
              kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)&loc_4A2A29), kasShipsVectorPtr((char *)0x2EE0), 0x2EE0) &&
              kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x36980EB)))))
        {
            kasfEntryGetOrCreateSetValue(0x443CCA2, 1);
            kasfShipsSetRetaliation(&teamtoreinforce->shipList);
        }
    }
}

void Init_Mission15_GateNodeDefenseTeam_MoveToWaypoint(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfMoveTo(kasShipsVectorPtrIfExists((char *)0x4AD39A3));
}

void Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint_Resume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(0x8209731B))
    {
        kasfSpeechEvent(0x44E27E5, (sdword)Init_Mission15_GateNodeDefenseTeam_MoveToWaypoint, (sdword)Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint);
    }
}

void Init_Mission15_GateNodeDefenseTeam_DockStay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfShipsRemove(kasGrowSelectionPtr((char *)0x868E7B3), &teamtoreinforce->shipList);
    kasfDockStay(kasGrowSelectionPtr((char *)0x4AD39A3));
}

void Watch_Mission15_GateNodeDefenseTeam_Reposition(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(0x8D362C39) && kasfMsgReceived((char *)0x56A8A15))
    {
        kasfEntrySetValue(0x5D8A374, 0);
        kasfSpeechEvent(0x44E27E5, (sdword)Init_Mission15_GateNodeDefenseTeam_MoveToWaypoint, (sdword)Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint);
    }
}

void Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfEntryValueGet(0x8209731B) && kasfNearby(kasShipsVectorPtrIfExists((char *)0x4AD39A3), 10000))
    {
        kasfEntrySetValue(0x8209731B, 1);
        kasfSpeechEvent(0x0ADC11C5, (sdword)Init_Mission15_GateNodeDefenseTeam_DockStay, (sdword)tutHideNextButton);
        return;
    }
    if (kasfEntryValueGet(0x8209731B) && kasfNearby(kasShipsVectorPtrIfExists((char *)0x4AD39A3), 10000))
    {
        kasfSpeechEvent(0x0E6FAAB5, (sdword)TeamHaltAndClearMoveOrders, (sdword)Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint_Resume);
        return;
    }
    if (kasfShipsAnyIdle(&teamtoreinforce->shipList))
    {
        kasfSpeechEvent(0x44E27E5, (sdword)Init_Mission15_GateNodeDefenseTeam_MoveToWaypoint, (sdword)Watch_Mission15_GateNodeDefenseTeam_MoveToWaypoint);
    }
}

void Init_Mission15_DAcolyteReinforcementTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(0x5137B, 1);
}

void Watch_Mission15_DAcolyteReinforcementTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(0x5137B))
    {
        if (!kasfTeamCount())
        {
            kasfSpeechEvent(0x479E04, (sdword)Init_Mission15_DAcolyteReinforcementDelay, (sdword)Watch_Mission15_DAcolyteReinforcementDelay);
            return;
        }
        if (kasfMsgReceived((char *)0x628EC3E))
        {
            kasfEntrySetValue(0x5137B, 1);
            kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x868E7B3), kasThisTeamsVectorPtr());
            kasfShipsSelectIndex(kasGrowSelectionPtr((char *)0x0C28F6AE), kasGrowSelectionPtr((char *)0x868E7B3), 0);
            kasfShipsDockWithSelection(&teamtoreinforce->shipList, kasGrowSelectionPtr((char *)0x0C28F6AE));
            kasfShipsSetNonRetaliation(&teamtoreinforce->shipList);
            kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
            kasfSpeechEvent(0x53A0C, (sdword)MaintainTeamContext, (sdword)sub_65F6F0);
            return;
        }
    }
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0xADBD8E2)) && kasfMsgReceived((char *)0x0B1FF3D3))
    {
        kasfMsgSend(kasAITeamPtrIfExists(0xDBD8E91), (char *)0x479E04);
        kasfMsgSend(kasAITeamPtrIfExists(0xDBD8E92), (char *)0x479E04);
        kasfMsgSend(kasAITeamPtrIfExists(0xDBD8E93), (char *)0x479E04);
    }
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(0xADBD8E2)) && kasfMsgReceived((char *)0x479E04))
    {
        kasfMsgSend(teamtoreinforce, (char *)0x0AA768C5);
        kasfSpeechEvent(0x479E04, (sdword)Init_Mission15_DAcolyteReinforcementDelay, (sdword)Watch_Mission15_DAcolyteReinforcementDelay);
    }
}

void Init_Mission15_DAcolyteReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    kasfEntrySetValue(0x5137B, 1);
    kasTimerDestroyById(0x479E04);
    kasTimerCreateSetStartById(0x479E04, 60);
}

void Watch_Mission15_DAcolyteReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfMsgReceived((char *)0x0AA768C5) || kasTimerExpiredDestroyById(0x479E04))
    {
        kasTimerDestroyById(0x479E04);
        switch (GetGameDifficulty())
        {
            case 0:
                kasfShipsCreateInside(kasGrowSelectionPtr((char *)0x9379E54), "dAcolyte", kasfRandom(3, 5), kasGrowSelectionPtr((char *)0x88B194), 1);
                break;
            case 1:
                kasfShipsCreateInside(kasGrowSelectionPtr((char *)0x9379E54), "dAcolyte", kasfRandom(4, 6), kasGrowSelectionPtr((char *)0x88B194), 1);
                break;
            case 2:
                kasfShipsCreateInside(kasGrowSelectionPtr((char *)0x9379E54), "dAcolyte", kasfRandom(4, 7), kasGrowSelectionPtr((char *)0x88B194), 1);
                break;
            case 3:
                kasfShipsCreateInside(kasGrowSelectionPtr((char *)0x9379E54), "dAcolyte", kasfRandom(5, 7), kasGrowSelectionPtr((char *)0x88B194), 1);
                break;
            case 4:
                kasfShipsCreateInside(kasGrowSelectionPtr((char *)0x9379E54), "dAcolyte", kasfRandom(6, 7), kasGrowSelectionPtr((char *)0x88B194), 1);
                break;
            default:
                break;
        }
        kasfReinforceTeamWithShips(teamtoreinforce, kasGrowSelectionPtr((char *)0x9379E54));
        kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(0x5137B, 0);
        if (kasfEntryValueGet(0x8D362C39))
        {
            kasfSpeechEvent(0x6985782, (sdword)Begin_Mission15_GateNodeDefenseCombatLoop, (sdword)Watch_Mission15_GateNodeDefenseCombatLoop);
        }
        else
        {
            kasfSpeechEvent(0x469857B, (sdword)Begin_Mission15_DAcolyteCombatLoop, (sdword)Watch_Mission15_DAcolyteCombatLoop);
        }
    }
}

void Begin_Mission15_DAcolyteCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x4A2A29), kasTeamsVectorPtr(0x68A3735), 20000))
    {
        kasfSpeechEvent(0x5B5E4, (sdword)Begin_Mission15_GateNodeDefensePatrol, (sdword)Watch_Mission15_GateNodeDefensePatrol);
        return;
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr((char *)0x8C46FF2), kasGrowSelectionPtr((char *)0x4A2A29), "sMothershipBigGun"))
    {
        kasfShipsRemove(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x8C46FF2));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x57BCEC1)) ||
        kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(0x68A3735), kasGrowSelectionPtr((char *)0x57BCEC2)))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr((char *)0x857BCE3));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x57BCEC1));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x57BCEC2));
        kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x857BCE3));
    }
    kasfFormationDelta();
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x6980C4B), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x36980EB));
    kasTimerDestroyById(0x469857B);
    kasTimerCreateSetStartById(0x469857B, kasfRandom(20, 40));
}

void Watch_Mission15_DAcolyteCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasTimerExpiredDestroyById(0x469857B) || kasfShipsAnyIdle(&teamtoreinforce->shipList))
    {
        kasTimerDestroyById(0x469857B);
        if (kasfEntryValueGet(0x8D362C39))
        {
            kasfSpeechEvent(0x6985782, (sdword)Begin_Mission15_GateNodeDefenseCombatLoop, (sdword)Watch_Mission15_GateNodeDefenseCombatLoop);
        }
        else
        {
            kasfSpeechEvent(0x469857B, (sdword)Begin_Mission15_DAcolyteCombatLoop, (sdword)Watch_Mission15_DAcolyteCombatLoop);
        }
    }
}

void Begin_Mission15_GateNodeDefenseCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (!kasfShipsSelectNearby(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x4A2A29), kasShipsVectorPtr((char *)0x4E20), 0x4E20))
    {
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x57BCEC1)) ||
        kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(0x68A3735), kasGrowSelectionPtr((char *)0x57BCEC2)))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr((char *)0x857BCE3));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x57BCEC1));
        kasfShipsAdd(kasGrowSelectionPtr((char *)0x857BCE3), kasGrowSelectionPtr((char *)0x57BCEC2));
        kasfShipsSelectMembersOfSelection(kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x36980EB), kasGrowSelectionPtr((char *)0x857BCE3));
    }
    kasfFormationDelta();
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr((char *)0x6980C4B), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(kasGrowSelectionPtr((char *)0x36980EB));
    kasTimerDestroyById(0x469857B);
    kasTimerCreateSetStartById(0x469857B, kasfRandom(20, 40));
}

void Watch_Mission15_GateNodeDefenseCombatLoop(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasTimerExpiredDestroyById(0x469857B) || kasfShipsAnyIdle(&teamtoreinforce->shipList))
    {
        kasTimerDestroyById(0x469857B);
        kasfSpeechEvent(0x6985782, (sdword)Begin_Mission15_GateNodeDefenseCombatLoop, (sdword)Watch_Mission15_GateNodeDefenseCombatLoop);
    }
}

void Begin_Mission15_GateNodeDefensePatrol(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfEntryValueGet(0x8D362C39))
    {
        kasfSpeechEvent(0x6985782, (sdword)Begin_Mission15_GateNodeDefenseCombatLoop, (sdword)Watch_Mission15_GateNodeDefenseCombatLoop);
        return;
    }
    if (!kasfEntryValueGet(0x443CCA3))
    {
        kasfEntryGetOrCreateSetValue(0x443CCA3, 1);
        kasfLabelledVectorSet((char *)0xDBD8ED3, 0, 0, 0);
    }
    kasfVectorRandomizeInRange(kasVectorPtrIfExists((char *)0xDBD8ED3), kasTeamsVectorPtr(0x68A3735), 6000);
    kasfMoveTo(kasVectorPtrIfExists((char *)0xDBD8ED3));
}

void Watch_Mission15_GateNodeDefensePatrol(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = teamtoreinforce->kasLabelHash;
    if (kasfUnderAttack(kasGrowSelectionPtr((char *)0x57BCEC1)) ||
        kasfFindShipsTargetingSelection(kasAITeamShipsPtrIfExists(0x68A3735), kasGrowSelectionPtr((char *)0x57BCEC2)))
    {
        kasfSpeechEvent(0x469857B, (sdword)Begin_Mission15_DAcolyteCombatLoop, (sdword)Watch_Mission15_DAcolyteCombatLoop);
        return;
    }
    if (kasfShipsAnyIdle(&teamtoreinforce->shipList))
    {
        kasfSpeechEvent(0x5B5E4, (sdword)Begin_Mission15_GateNodeDefensePatrol, (sdword)Watch_Mission15_GateNodeDefensePatrol);
    }
}

void Init_Mission15_AttackMothershipTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSetPercentDamaged(&teamtoreinforce->shipList, 200);
    kasfEntryGetOrCreateSetValue(0x5137B, 1);
    kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
    kasfShipsAdd(kasGrowSelectionPtr((char *)0x83A90B3), &teamtoreinforce->shipList);
    kasfTeamAttributesBitSet(0x80);
    kasfAttackMothership();
    kasfSpeechEvent(0x0A7C9B35, (sdword)MaintainTeamContext, (sdword)Init_Mission16_Events_HoneyImHome);
}

void Watch_Mission15_AttackMothershipTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(0x5137B))
    {
        if (!kasfTeamCount())
        {
            kasfSpeechEvent(0x53A0C, (sdword)MaintainTeamContext, (sdword)sub_65F6F0);
            return;
        }
        if (kasfMsgReceived((char *)0x628EC3E))
        {
            kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
            kasfStop();
            kasfShipsSetNonRetaliation(&teamtoreinforce->shipList);
            kasfSpeechEvent(0x53A0C, (sdword)MaintainTeamContext, (sdword)sub_65F6F0);
        }
    }
}

/*-----------------------------------------------------------------------------
    Init_Mission15 - top-level KAS entry point (IDA: 0x639ce0)
-----------------------------------------------------------------------------*/

void Init_Mission15(void)
{
    kasCurrentStep = 0;
    scope = KH_mission15;

    kasfMusicPlay(14);
    kasfEnableShip("dAcolyte", "RACE_SECT", 0);
    kasfDisableMothershipBigGunLaunch();
    kasfFindEnemyMothership(kasGrowSelectionPtr(KH_sMothership));
    kasfFindBeastShipsInside(kasVolumePtr(78461347), kasGrowSelectionPtr(78461347));
    kasfFindBeastShipsInside(kasVolumePtr(181639682), kasGrowSelectionPtr(181639682));

    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission15_Events_GateNodeObjectives,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(KH_FleetIntel, Init_Mission15_MissionFlow, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));

    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(141092465));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(141092466));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(141092467));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(141092471));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(109995937));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(109995941));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(109995945));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(109995955));
    kasFSMCreate(141092787, Init_Mission15_GateNodeDefenseTeam, Watch_Mission15_GateNodeDefenseTeam,
                 kasAITeamPtrIfExists(109995959));

    kasFSMCreate(138055859, Init_Mission15_AttackMothershipTeam, Watch_Mission15_AttackMothershipTeam,
                 kasAITeamPtrIfExists(138055537));
    kasFSMCreate(138055859, Init_Mission15_AttackMothershipTeam, Watch_Mission15_AttackMothershipTeam,
                 kasAITeamPtrIfExists(138055538));
    kasFSMCreate(138055859, Init_Mission15_AttackMothershipTeam, Watch_Mission15_AttackMothershipTeam,
                 kasAITeamPtrIfExists(138055539));

    kasFSMCreate(230526675, Init_Mission15_DAcolyteReinforcementTeam, Watch_Mission15_DAcolyteReinforcementTeam,
                 kasAITeamPtrIfExists(182180066));
    kasFSMCreate(230526675, Init_Mission15_DAcolyteReinforcementTeam, Watch_Mission15_DAcolyteReinforcementTeam,
                 kasAITeamPtrIfExists(230526609));
    kasFSMCreate(230526675, Init_Mission15_DAcolyteReinforcementTeam, Watch_Mission15_DAcolyteReinforcementTeam,
                 kasAITeamPtrIfExists(230526610));
    kasFSMCreate(230526675, Init_Mission15_DAcolyteReinforcementTeam, Watch_Mission15_DAcolyteReinforcementTeam,
                 kasAITeamPtrIfExists(230526611));

    kasFSMCreate(109721397, Init_Mission15_Events_GateNodeCollapse, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(109721397));

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_EXPLOSIVE_CHARGE");
    kasfAllowPlayerToResearch("TECH_DRIVE_FRIGATE");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD1");
    kasfAllowPlayerToResearch("TECH_ARMOUR2");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");
    kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
    kasfAllowPlayerToResearch("TECH_ADVANCED_SENSORS");
    kasfAllowPlayerToResearch("TECH_ENERGY");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");
    kasfAllowPlayerToResearch("TECH_EMP");
    kasfAllowPlayerToResearch("TECH_DRIVE_DESTROYER");
    kasfAllowPlayerToResearch("TECH_MISSILE");
    kasfAllowPlayerToResearch("TECH_ADRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_ARMOUR3");
    kasfAllowPlayerToResearch("TECH_ADRIVE_FRIGATE");
    kasfAllowPlayerToResearch("TECH_ADRIVE_DESTROYER");
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD3");
    kasfActivateTech("TECH_ADVANCED_SENSORS");

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
    kasfModuleSlotOrderSetShipType(1, "bRecon");
    kasfModuleSlotOrderSetShipType(1, "bAcolyte");
    kasfModuleSlotOrderSetShipType(1, "bAttackBomber");
    kasfModuleSlotOrderSetShipType(1, "bMultiGunCorvette");
    kasfModuleSlotOrderSetShipType(1, "bIonArrayFrigate");
    kasfModuleSlotOrderSetShipType(1, "bHiveFrigate");
    kasfModuleSlotOrderSetShipType(1, "bDFGFrigate");
    kasfModuleSlotOrderSetCount(1, 4);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasfDisableAllAIFeatures();
    kasfDisablePlayerHyperspace();
    kasTimerCreateSetStartById(104338707, 1);
}
