/*=============================================================================
    Name    : mission10.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission10
=============================================================================*/

#include "MissionScriptSupport.h"

void Watch_Mission10_GlobalEvents(void);
void Init_Mission10_BigGunObjective(void);
void Watch_Mission10_BigGunObjective(void);
void Init_Mission10_BigGunAmbushTeam(void);
void Watch_Mission10_BigGunAmbushTeam_Hidden(void);
void Init_Mission10_BigGunAmbushTeam_DisguiseAsWreck(void);
void Watch_Mission10_BigGunAmbushTeam_Disguised(void);
void Init_Mission10_BigGunAmbushTeam_TargetNearestSalvager(void);
void Watch_Mission10_BigGunAmbushTeam_TargetNearestSalvager(void);
void Watch_Mission10_BigGunAmbushTeam_AttackSalvagers(void);
void Init_Mission10_BigGunAmbushTeam_AttackSpecial(void);
void Watch_Mission10_BigGunAmbushTeam_AttackSpecial(void);
void Init_Mission10_BigGunAmbushTeam_SortTargetsFromMothership(void);
void Watch_Mission10_BigGunAmbushTeam_SortTargetsFromMothership(void);
void Init_Mission10_P1StandardCorvetteWing(void);
void Init_Mission10_P1StandardCorvetteWing_Harvest(void);
void Watch_Mission10_P1StandardCorvetteWing(void);
void Watch_Mission10_P1StandardCorvetteWing_Disable(void);
void Init_Mission10_P1StandardCorvetteWing_SpawnReinforcement(void);
void Watch_Mission10_P1StandardCorvetteWing_SpawnReinforcement(void);
void Init_Mission10_StrikeGroupMember(void);
void Watch_Mission10_StrikeGroupMember_Docked(void);
void Watch_Mission10_StrikeGroupMember_Launched(void);
void Init_Mission10_StrikeGroupMember_ClearObjectiveFocus(void);
void Init_Mission10_BigGunObjective_ControllerPocket_Start(void);
void Watch_Mission10_BigGunObjective_ControllerPocket_Hidden(void);

void Watch_Mission10(void)
{
  int v0; // eax
  int v1; // eax
  int *v2; // eax
  int *v3; // eax
  int *v4; // eax
  int *v5; // eax
  int *v6; // eax
  int *v7; // eax
  int v8; // eax
  int *v9; // eax
  int v10; // eax
  int v11; // eax
  int v12; // eax
  int v13; // eax
  int v15; // [esp-8h] [ebp-8h]
  int v16; // [esp-8h] [ebp-8h]
  int v17; // [esp-8h] [ebp-8h]
  int v18; // [esp-8h] [ebp-8h]
  int v19; // [esp-8h] [ebp-8h]
  int v20; // [esp-4h] [ebp-4h]
  int v21; // [esp-4h] [ebp-4h]
  int v22; // [esp-4h] [ebp-4h]
  int v23; // [esp-4h] [ebp-4h]
  int v24; // [esp-4h] [ebp-4h]
  int v25; // [esp-4h] [ebp-4h]
  int *v26; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503616;
  if ( !kasfEntryValueGet(71552177) )
  {
    v15 = kasGrowSelectionPtr(0x4A2A29);
    v0 = kasGrowSelectionPtr(211966048);
    if ( kasfShipsSelectType(v0, v15, "sMothership") )
      kasfEntrySetValue(71552177, 1);
  }
  if ( kasTimerExpiredDestroyById(150120898) )
  {
    kasTimerSetById(150120898, 10);
    kasTimerStartById(150120898);
    v20 = kasGrowSelectionPtr(18336307);
    v1 = kasVolumePtr(132933767);
    kasfFindShipsInside(v1, v20);
    v16 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(140367886);
    kasfShipsSelectRace(v2, v16, 3);
    v17 = kasGrowSelectionPtr(18336307);
    v3 = (int *)kasGrowSelectionPtr(0x00469684);
    kasfShipsSelectRace(v3, v17, 1);
    v21 = kasGrowSelectionPtr(18336307);
    v4 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v4, v21);
    v22 = kasGrowSelectionPtr(18336307);
    v5 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v5, v22);
  }
  if ( !kasfEntryValueGet(71552178) && kasfEntryValueGet(-2015429404) )
  {
    kasfEntrySetValue(71552178, 1);
    v23 = kasGrowSelectionPtr(18336307);
    v6 = (int *)kasGrowSelectionPtr(89157026);
    kasSelectPlayerShipsSimple(v6, v23);
    v24 = kasGrowSelectionPtr(89157026);
    v7 = (int *)kasGrowSelectionPtr(89157026);
    kasfShipsSelectNotSalvagers(v7, v24);
    v25 = kasGrowSelectionPtr(218060896);
    v8 = kasGrowSelectionPtr(82017669);
    if ( kasfShipsSelectLinkedShips(v8, v25) )
    {
      v26 = (int *)kasGrowSelectionPtr(82017669);
      v9 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsAdd(v9, v26);
    }
    if ( GetGameDifficulty() < 2 && kasfRUsGet(0) < 500 )
    {
      v18 = kasGrowSelectionPtr(89157026);
      v10 = kasGrowSelectionPtr(96759714);
      if ( !kasfShipsSelectType(v10, v18, "sWorker") )
      {
        v19 = kasGrowSelectionPtr(218060896);
        v11 = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(v11, "sWorker", 1, v19, 0);
      }
    }
  }
  v12 = kasGrowSelectionPtr(125872958);
  v13 = kasfShipsCount(v12);
  kasfLogInteger("SHIPS ON BIG GUN: %d\n", v13);
}

void Watch_Mission10_MissionFlow_FirstWarning(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfMsgSend(kasAITeamPtrIfExists(196001810), (char *)137343732);
        kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)Watch_Mission10_MissionFlow_FirstWarningDelay);
    }
}

void Watch_Mission10_MissionFlow_FirstWarningDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)54112162))
    {
        kasTimerCreateSetStartById(139352562, 13);
    }
    else if (kasTimerExpiredById(139352562))
    {
        kasfLaunch();
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission10_MissionFlow_FleetCommandIntro);
    }
}

void Watch_Mission10_MissionFlow_AttackInClawFormation(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationClaw();
}

void Watch_Mission10_MissionFlow_SelectNearbyAttackersOrQueueReinforcements(void)
{
    GrowSelection *nearbyAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        nearbyAttackers = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(nearbyAttackers);
        kasfShipsSelectNearby(
            nearbyAttackers,
            kasGrowSelectionPtr(21159),
            kasShipsVectorPtrIfExists(211966048),
            10000);
        tutDeallocateRootRegion();
    }
    else
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_QueueMultigunCorvetteOrHeavyInterceptorBatch,
            (sdword)Watch_MultigunCorvetteOrHeavyInterceptorBatchComplete);
    }
}

void Watch_Mission10_MissionFlow_FleetCommandIntro(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission10_MissionFlow_AttackInClawFormation,
            (sdword)Watch_Mission10_MissionFlow_SelectNearbyAttackersOrQueueReinforcements);
    }
}

void Init_Mission10_ProximitySensorObjective_WaitForTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)139644696))
    {
        kasfSpeechEvent(92553416, (sdword)Init_SelectAndFocusProximitySensors, (sdword)Watch_ProximitySensorContactLoop);
    }
}

void Watch_Mission10_AsteroidPocketAttackTeam_SelectAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackersInPocket = kasGrowSelectionPtr(22816);
    candidateAttackers = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(attackersInPocket);
    kasfGrowSelectionClear(candidateAttackers);
    kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
    kasfShipsSelectByPredicate(
        candidateAttackers,
        attackersInPocket,
        (sdword (*)(Ship *, sdword, sdword))Mission02ShipSelectPredicate,
        1,
        0);
    kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(85516498));
    kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(87999618));
    kasfCurrentTeamAttack(candidateAttackers);
    kasTimerCreateSetStartById(139352562, kasfRandom(1, 7));
}

void Watch_Mission10_AsteroidPocketAttackTeam_UpdateAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfTeamFinishedLaunching() && kasTimerExpiredDestroyById(139352562))
        {
            attackersInPocket = kasGrowSelectionPtr(22816);
            candidateAttackers = kasGrowSelectionPtr(57245931);
            kasfGrowSelectionClear(attackersInPocket);
            kasfGrowSelectionClear(candidateAttackers);
            kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
            kasfShipsSelectByPredicate(
                candidateAttackers,
                attackersInPocket,
                (sdword (*)(Ship *, sdword, sdword))Mission02ShipSelectPredicate,
                1,
                0);
            kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(85516498));
            kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(87999618));
            kasfCurrentTeamAttack(candidateAttackers);
            kasTimerSetById(139352562, 7);
            kasTimerStartById(139352562);
        }
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(178361137)))
    {
        kasfSpeechEvent(75929221, (sdword)Init_QueueHeavyInterceptorBatch, (sdword)Watch_HeavyInterceptorBatchComplete);
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission10_AsteroidPocketAttackTeam_Intro(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission10_AsteroidPocketAttackTeam_SelectAttackers,
            (sdword)Watch_Mission10_AsteroidPocketAttackTeam_UpdateAttackers);
    }
}

void Init_Mission10_AsteroidPocketEncounter_Start(void)
{
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (team->shipList.selection != NULL)
    {
        for (i = 0; i < team->shipList.selection->numShips; i++)
        {
            shipApplyPlayerUpgrades(team->shipList.selection->ShipPtr[i]);
        }
    }
    kasfShipsSetDamageFactor(&team->shipList, 10);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(87999618)))
    {
        kasfSpeechEvent(
            76775316,
            (sdword)Init_Mission10_AsteroidPocketEncounter_FocusTarget,
            (sdword)Watch_Mission10_AsteroidPocketEncounter_AfterFocus);
    }
    else
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission10_AsteroidPocketEncounter_SelectAttackers,
            (sdword)Watch_Mission10_AsteroidPocketEncounter_UpdateAttackers);
    }
}

void Init_Mission10_AsteroidPocketEncounter_FocusTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfDockInstant(kasAITeamPtrIfExists(114551810));
}

void Watch_Mission10_AsteroidPocketEncounter_AfterFocus(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)84320888))
    {
        kasfLaunch();
        if (kasfThisTeamIs(kasAITeamPtrIfExists(87999618)))
        {
            kasfSpeechEvent(
                236013954,
                (sdword)Init_AsteroidPocketEncounter_ClearCurrentSelectionHighlight,
                (sdword)cpHueSatImageDelete);
        }
        else
        {
            kasfSpeechEvent(
                139967671,
                (sdword)Watch_Mission10_AsteroidPocketEncounter_SelectAttackers,
                (sdword)Watch_Mission10_AsteroidPocketEncounter_UpdateAttackers);
        }
    }
}

void Watch_Mission10_AsteroidPocketEncounter_SelectAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackersInPocket = kasGrowSelectionPtr(22816);
    candidateAttackers = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(attackersInPocket);
    kasfGrowSelectionClear(candidateAttackers);
    kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
    Mission02ShipsSelect(candidateAttackers, attackersInPocket);
    kasfCurrentTeamAttack(candidateAttackers);
    kasTimerCreateSetStartById(139352562, kasfRandom(1, 7));
}

void Watch_Mission10_AsteroidPocketEncounter_UpdateAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfTeamFinishedLaunching() && kasTimerExpiredDestroyById(139352562))
        {
            attackersInPocket = kasGrowSelectionPtr(22816);
            candidateAttackers = kasGrowSelectionPtr(57245931);
            kasfGrowSelectionClear(attackersInPocket);
            kasfGrowSelectionClear(candidateAttackers);
            kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
            Mission02ShipsSelect(candidateAttackers, attackersInPocket);
            kasfCurrentTeamAttack(candidateAttackers);
            kasTimerSetById(139352562, 7);
            kasTimerStartById(139352562);
        }
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(98363089)))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor,
            (sdword)Watch_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady);
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission10_AsteroidPocketAttackTeam_Start(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSetDamageFactor(&((AITeam *)kasCurrentTeamPtr)->shipList, 10);
    kasfTeamSwitchPlayerOwner(FALSE);
    if (kasfTeamCount() > 0)
    {
        kasfShipsEnableAttachNLips(&((AITeam *)kasCurrentTeamPtr)->shipList);
    }
    kasfSpeechEvent(
        139967671,
        (sdword)Watch_Mission10_AsteroidPocketAttackTeam_SelectAttackers,
        (sdword)Watch_Mission10_AsteroidPocketAttackTeam_UpdateAttackers);
}

void Init_Mission10_ProximitySensorObjective_Start(void)
{
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (team->shipList.selection != NULL)
    {
        for (i = 0; i < team->shipList.selection->numShips; i++)
        {
            shipApplyPlayerUpgrades(team->shipList.selection->ShipPtr[i]);
        }
    }
    kasfHideShips(&team->shipList);
    kasfSpeechEvent(55103095, (sdword)MaintainTeamContext, (sdword)Init_Mission10_ProximitySensorObjective_WaitForTrigger);
}

void Init_Mission10_BigGunAmbushTeam_Hide(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfHideShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
}

void Init_Mission10_P1StandardCorvetteWing_Disable(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfDisableShips(&((AITeam *)kasCurrentTeamPtr)->shipList);
}

void Init_Mission10_BigGunObjective_ClearTrackedSelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasGrowSelectionPtr((char *)0x4A2A29);
    tutDeallocateRootRegion();
}

void Init_Mission10_BigGunObjective_CarrierObjective_Start(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasAITeamShipsPtr(kasCurrentTeamPtr + 28, 5);
    kasfShipsApplyPlayerUpgrades((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfSpeechEvent(135487444, Init_Mission10_BigGunObjective_CarrierObjective_HideAndTrack, Watch_Mission10_BigGunObjective_CarrierObjective_WaitForCue);
}

void Init_Mission10_BigGunObjective_CarrierObjective_HideAndTrack(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfShipsSetNonRetaliation((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfTeamSpecialToggle();
    kasfHideShips((GrowSelection *)(kasCurrentTeamPtr + 28));
}

void Watch_Mission10_BigGunObjective_CarrierObjective_WaitForCue(void)
{
    sdword movePointLabelHash;

    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)0x0C39AB4F))
    {
        kasTimerCreateSetStartById(139352562, 45);
    }
    if (kasTimerExpiredDestroyById(139352562))
    {
        kasTimerDestroyById(139352562);
        kasfSpeechEvent(139967671, Init_Mission10_BigGunObjective_CarrierObjective_Release, MaintainTeamContext);
    }
    else
    {
        movePointLabelHash = 0x0CA25860;
        kasfMoveTo(kasShipsVectorPtrIfExists(movePointLabelHash));
    }
}

void Init_Mission10_BigGunObjective_CarrierObjective_Release(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfShipsSetRetaliation((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfUnhideShips((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfTeamSpecialToggle();
    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x61C2E7);
    kasGrowSelectionPtr((char *)0x0CA25860);
    tutDeallocateRootRegion();
}

void Init_Mission10_BigGunObjective_GarrisonObjective_Start(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasAITeamShipsPtr(kasCurrentTeamPtr + 28, 5);
    kasfHideShips((GrowSelection *)(kasCurrentTeamPtr + 28));
}

void Watch_Mission10_BigGunObjective_GarrisonObjective_HiddenTrigger(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)0xD19286F))
    {
        kasfUnhideShips(&team->shipList);
        tutDeallocateRootRegion();
    }
    else if (kasfMsgReceived((char *)0xC708950))
    {
        tutDeallocateRootRegion();
    }
    if (!kasfEntryValueGet(0x443CCAA) &&
        kasfFindDistance(kasTeamsVectorPtr(0x56179FF), kasShipsVectorPtr((char *)0x1770)) < 6000)
    {
        kasfEntryGetOrCreateSetValue(0x443CCAA, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0x112BD9F);
    }
    if (!kasfEntryValueGet(0x443CCAB) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(0x443CCAB, 1);
        kasfMsgSend(kasAITeamPtrIfExists(0xA8A880C), (char *)0xEF5BD54);
    }
}

void Init_Mission10_BigGunObjective_ResearchShipTeam_Activate(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfTacticsAggressive();
    kasAITeamShipsPtr(kasCurrentTeamPtr + 28, 5);
    kasfSetSalCapProcessingShip((Ship *)(kasCurrentTeamPtr + 28));
    kasfSpeechEvent(170800580, MaintainTeamContext, Watch_Mission10_BigGunObjective_ResearchShipTeam_ApproachCue);
}

void Watch_Mission10_BigGunObjective_ResearchShipTeam_ApproachCue(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (!kasfEntryValueGet(71552176) && kasfTeamHealthAverage() < 70)
    {
        kasfEntryGetOrCreateSetValue(71552176, 1);
        kasfShipsSetDontApplyPhysicsFlag(kasCurrentTeamPtr + 28, 1);
    }
    if (kasfMsgReceived((char *)0x047B4804))
    {
        kasfSpeechEvent(153826340, Init_Mission10_BigGunObjective_ResearchShipTeam_ReleaseHold, MaintainTeamContext);
    }
}

void Init_Mission10_BigGunObjective_ResearchShipTeam_ReleaseHold(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfShipsSetDontApplyPhysicsFlag(kasCurrentTeamPtr + 28, 0);
}

void Watch_Mission10_GlobalEvents(void)
{
    AITeam *team;
    sdword *queueInfo;
    sdword queueIndex;
    udword message;
    GrowSelection *selection;
    hvector *teamVector;
    hvector *shipVector;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
    queueIndex = (queueInfo != NULL) ? *queueInfo : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, queueIndex);
        if (message > 0x7145C3E)
        {
            switch (message)
            {
                case 0x0A7C9B35:
                    kasfObjectiveCreate("Hyperspace", ((char **)0x8CB678)[strCurLanguage], ((char **)0x8CB684)[strCurLanguage]);
                    kasfObjectiveSet("Hyperspace", 0);
                    spSetSinglePlayerSaveInfo(10, ((char **)0x8CB60C)[strCurLanguage]);
                    break;
                case 0x0CC61C25:
                    kasfEntryGetOrCreateSetValue(-2140662891, 1);
                    kasfObjectiveCreate("FireBigGun", ((char **)0x8CB660)[strCurLanguage], ((char **)0x8CB66C)[strCurLanguage]);
                    kasfObjectiveSet("FireBigGun", 0);
                    kasfTutSetPointerTargetShipWithLine("ObjPtr", kasAITeamShipsPtrIfExists(170794736));
                    break;
                case 0x0FD0D934:
                    kasfObjectiveDestroy("SalvageBiggun");
                    kasfTutRemovePointer("ObjPtr");
                    break;
                default:
                    aitQueuePointer(team, team, (void *)message);
                    break;
            }
        }
        else
        {
            switch (message)
            {
                case 0x07145C3E:
                    kasfObjectiveCreate("ResearchBiggun", ((char **)0x8CB630)[strCurLanguage], ((char **)0x8CB63C)[strCurLanguage]);
                    kasfObjectiveSet("ResearchBiggun", 0);
                    kasfAllowPlayerToResearch("TECH_BIG_GUN");
                    break;
                case 0:
                    break;
                case 0x0577E27E:
                    kasfObjectiveCreate("SalvageBiggun", ((char **)0x8CB618)[strCurLanguage], ((char **)0x8CB624)[strCurLanguage]);
                    kasfObjectiveSet("SalvageBiggun", 0);
                    kasfTutSetPointerTargetShipWithLine("ObjPtr", kasAITeamShipsPtrIfExists(74300574));
                    kasfEntryGetOrCreateSetValue(290904, 1);
                    break;
                case 0x06E7DA74:
                    kasfObjectiveDestroy("FireBigGun");
                    kasfTutRemovePointer("ObjPtr");
                    break;
                default:
                    aitQueuePointer(team, team, (void *)message);
                    break;
            }
        }

        queueInfo = team->msgQueue ? &team->msgQueue->head : NULL;
        if (queueInfo == NULL || *queueInfo == 0 || --queueIndex < 0)
        {
            break;
        }
    }

    if (!kasfEntryValueGet(71552162) && kasfDoesPlayerHave("TECH_BIG_GUN"))
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfObjectiveDestroy("ResearchBiggun");
        kasfObjectiveCreate("BuildBiggun", ((char **)0x8CB648)[strCurLanguage], ((char **)0x8CB654)[strCurLanguage]);
        kasfObjectiveSet("BuildBiggun", 0);
    }
    if (!kasfEntryValueGet(71552163))
    {
        if (kasfShipsSelectType(kasGrowSelectionPtr(147842302), kasGrowSelectionPtr((int)&loc_4A2A29), "sMothershipBigGun"))
        {
            selection = kasGrowSelectionPtr(147842302);
            if ((selection->selection->ShipPtr[0]->specialFlags2 & SPECIAL_2_DockFlag) == 0)
            {
                kasfEntryGetOrCreateSetValue(71552163, 1);
                kasfObjectiveDestroy("BuildBiggun");
                kasfEntryGetOrCreateSetValue(-2062465132, 1);
            }
        }
    }
    if (kasfEntryValueGet(-2140662891))
    {
        teamVector = kasTeamsVectorPtr(170794736);
        shipVector = kasShipsVectorPtrIfExists(211966048);
        if (kasfFindDistance(shipVector, teamVector) < 35000)
        {
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)117217653);
            kasfEntrySetValue(-2140662891, 0);
        }
    }
    if (!kasfEntryValueGet(71552164) && !kasfShipsCount(kasGrowSelectionPtr(211966048)) && kasfEntryValueGet(290904))
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfEntryScopeName(290904);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
    }
}

void Init_Mission10_BigGunObjective(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfTeamAttributesBitSet(0x40000);
    kasfShipsSetAttachCoordsysFlag(&team->shipList, 1);
    kasfGrowSelectionClear(kasGrowSelectionPtr(107517438));
}

void Watch_Mission10_BigGunObjective(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfTeamSetPercentDamaged(100);
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(107517438),
            kasGrowSelectionPtr((int)&loc_4A2A29),
            kasThisTeamsVectorPtr(),
            18000);
        if (!kasfEntryValueGet(71552165) && kasfShipsCount(kasGrowSelectionPtr(107517438)))
        {
            kasfEntryGetOrCreateSetValue(71552165, 1);
            kasfMsgSendAll(54471198);
        }

        if (kasfShipsSelectNearby(
                kasGrowSelectionPtr(363040),
                kasGrowSelectionPtr((int)&loc_4A2A29),
                kasThisTeamsVectorPtr(),
                5000))
        {
            if (!kasfEntryValueGet(71552166)
                && kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(363040), "sWorker"))
            {
                kasfEntryGetOrCreateSetValue(71552166, 1);
                kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)252070894);
            }
        }

        if (!kasfEntryValueGet(71552167)
            && kasfShipsSelectAttachedShips(kasGrowSelectionPtr(180901854), kasAITeamShipsPtrIfExists(74300574)) >= 4
            && kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(180901854), "sWorker"))
        {
            kasfEntryGetOrCreateSetValue(71552167, 1);
            kasfMsgSendAll(205105999);
            kasfEntryGetOrCreateSetValue(182189167, 1);
        }
    }

    if (!kasfEntryValueGet(71552168) && !kasfShipsCount(kasAITeamShipsPtrIfExists(74300574)))
    {
        kasfEntryGetOrCreateSetValue(71552168, 1);
        if (kasfGetShipTypeCount("dBigGunUnit", 0))
        {
            if (!kasfEntryValueGet(182189167))
            {
                kasfMsgSendAll(205105999);
            }
            kasfMsgSendAll(265345332);
        }
        else
        {
            kasfMsgSendAll(78338137);
        }
    }
}

void Init_Mission10_BigGunAmbushTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(125709057))
        || kasfThisTeamIs(kasAITeamPtrIfExists(125709058))
        || kasfThisTeamIs(kasAITeamPtrIfExists(125709059))
        || kasfThisTeamIs(kasAITeamPtrIfExists(125709060)))
    {
        kasfSpeechEvent(
            80578718,
            (sdword)Init_Mission10_BigGunAmbushTeam_Hide,
            (sdword)Watch_Mission10_BigGunAmbushTeam_Hidden);
    }
    else
    {
        kasfSpeechEvent(
            5366227,
            (sdword)Init_Mission10_BigGunAmbushTeam_DisguiseAsWreck,
            (sdword)Watch_Mission10_BigGunAmbushTeam_Disguised);
    }
}

void Watch_Mission10_BigGunAmbushTeam_Hidden(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)54471198))
    {
        kasfUnhideShips(&team->shipList);
        kasfSpeechEvent(
            31063541,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission10_BigGunAmbushTeam_AttackSalvagers);
    }
}

void Init_Mission10_BigGunAmbushTeam_DisguiseAsWreck(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(48009332)))
    {
        kasfShipsProcessModuleSlotOrder(&team->shipList, "dJunk1PartA", 2);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(48009329)))
    {
        kasfShipsProcessModuleSlotOrder(&team->shipList, "dJunk3Boiler", 2);
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(48009330)))
    {
        kasfShipsProcessModuleSlotOrder(&team->shipList, "dJunk2Shipwreck", 2);
    }
    else
    {
        kasfShipsProcessModuleSlotOrder(&team->shipList, "dJunk1Shell", 2);
    }
    kasfShipsSetDelayedDeath(&team->shipList, 1);
    kasfShipsSetAttachCoordsysFlag(&team->shipList, 1);
}

void Watch_Mission10_BigGunAmbushTeam_Disguised(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)46655475))
    {
        kasfShipsSetDelayedDeath(&team->shipList, 0);
        kasfShipsSetAttachCoordsysFlag(&team->shipList, 0);
        kasfDisableShips(&team->shipList);
        kasfSpeechEvent(
            157929499,
            (sdword)Init_Mission10_BigGunAmbushTeam_TargetNearestSalvager,
            (sdword)Watch_Mission10_BigGunAmbushTeam_TargetNearestSalvager);
    }
}

void Init_Mission10_BigGunAmbushTeam_TargetNearestSalvager(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSelectNearby(
        kasGrowSelectionPtr(363040),
        kasGrowSelectionPtr((int)&loc_4A2A29),
        kasTeamsVectorPtr(74300574),
        10000);
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(363040), kasTeamsVectorPtr(74300574));
    tutDeallocateRootRegion(kasGrowSelectionPtr(363040));
    kasTimerCreateSetStartById(249486322, 10);
}

void Watch_Mission10_BigGunAmbushTeam_TargetNearestSalvager(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredById(249486322))
    {
        kasfSpeechEvent(
            31063541,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission10_BigGunAmbushTeam_AttackSalvagers);
    }
}

void Watch_Mission10_BigGunAmbushTeam_AttackSalvagers(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)265345332) || !kasfShipsCount(kasAITeamShipsPtrIfExists(74300574)))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission10_BigGunAmbushTeam_SortTargetsFromMothership,
            (sdword)Watch_Mission10_BigGunAmbushTeam_SortTargetsFromMothership);
        return;
    }

    if (kasfShipsSelectAttachedShips(kasGrowSelectionPtr(180901854), kasAITeamShipsPtrIfExists(74300574))
        && kasfShipsSelectType(kasGrowSelectionPtr(96759714), kasGrowSelectionPtr(180901854), "sWorker"))
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(96759714));
        return;
    }

    if (kasfShipsCount(kasGrowSelectionPtr(125872958)) < 4)
    {
        kasfShipsAdd(kasGrowSelectionPtr(125872958), &team->shipList);
        kasfSpeechEvent(
            123839973,
            (sdword)Init_Mission10_BigGunAmbushTeam_AttackSpecial,
            (sdword)Watch_Mission10_BigGunAmbushTeam_AttackSpecial);
    }
    else
    {
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(363040),
            kasGrowSelectionPtr((int)&loc_4A2A29),
            kasTeamsVectorPtr(74300574),
            10000);
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(363040), kasTeamsVectorPtr(74300574));
        tutDeallocateRootRegion(kasGrowSelectionPtr(363040));
    }
}

void Init_Mission10_BigGunAmbushTeam_AttackSpecial(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfAttackSpecial(kasAITeamShipsPtrIfExists(74300574));
}

void Watch_Mission10_BigGunAmbushTeam_AttackSpecial(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)265345332))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_Mission10_BigGunAmbushTeam_SortTargetsFromMothership,
            (sdword)Watch_Mission10_BigGunAmbushTeam_SortTargetsFromMothership);
        return;
    }

    if (kasfShipsSelectAttachedShips(kasGrowSelectionPtr(180901854), kasAITeamShipsPtrIfExists(74300574))
        && kasfShipsSelectType(kasGrowSelectionPtr(96759714), kasGrowSelectionPtr(180901854), "sWorker"))
    {
        kasfShipsRemove(kasGrowSelectionPtr(125872958), &team->shipList);
        kasfSpeechEvent(
            31063541,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission10_BigGunAmbushTeam_AttackSalvagers);
        return;
    }

    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfAttackSpecial(kasAITeamShipsPtrIfExists(74300574));
    }
}

void Init_Mission10_BigGunAmbushTeam_SortTargetsFromMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfGrowSelectionClear(kasGrowSelectionPtr(363040));
    kasfShipsAdd(kasGrowSelectionPtr(363040), kasGrowSelectionPtr((int)&loc_4A2A29));
    kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(363040), kasThisTeamsVectorPtr());
    tutDeallocateRootRegion(kasGrowSelectionPtr(363040));
}

void Watch_Mission10_BigGunAmbushTeam_SortTargetsFromMothership(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfGrowSelectionClear(kasGrowSelectionPtr(363040));
        kasfShipsAdd(kasGrowSelectionPtr(363040), kasGrowSelectionPtr((int)&loc_4A2A29));
        kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(363040), kasThisTeamsVectorPtr());
        tutDeallocateRootRegion(kasGrowSelectionPtr(363040));
    }
}

void Init_Mission10_P1StandardCorvetteWing(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        236812599,
        (sdword)Init_Mission10_P1StandardCorvetteWing_Harvest,
        (sdword)Watch_Mission10_P1StandardCorvetteWing);
}

void Init_Mission10_P1StandardCorvetteWing_Harvest(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsProcessModuleSlotOrder(&team->shipList, "tResourceCollector", 3);
    kasfPatrolPath(kasPathPtr(86525576));
}

void Watch_Mission10_P1StandardCorvetteWing(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount() <= 0)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission10_P1StandardCorvetteWing_SpawnReinforcement,
            (sdword)Watch_Mission10_P1StandardCorvetteWing_SpawnReinforcement);
    }
    else if (kasfFindEnemiesNearCurrentTeam(kasGrowSelectionPtr(157781117), 5000) > 0)
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Init_Mission10_P1StandardCorvetteWing_Disable,
            (sdword)Watch_Mission10_P1StandardCorvetteWing_Disable);
    }
}

void Watch_Mission10_P1StandardCorvetteWing_Disable(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount() <= 0)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission10_P1StandardCorvetteWing_SpawnReinforcement,
            (sdword)Watch_Mission10_P1StandardCorvetteWing_SpawnReinforcement);
    }
    else if (kasfFindEnemiesNearCurrentTeam(kasGrowSelectionPtr(157781117), 5000) > 0)
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(157781117));
    }
    else
    {
        kasfSpeechEvent(
            236812599,
            (sdword)Init_Mission10_P1StandardCorvetteWing_Harvest,
            (sdword)Watch_Mission10_P1StandardCorvetteWing);
    }
}

void Init_Mission10_P1StandardCorvetteWing_SpawnReinforcement(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    kasfGrowSelectionClear(kasGrowSelectionPtr(21159));
    kasfShipsCreateInside(
        kasGrowSelectionPtr(21159),
        "p1StandardCorvette",
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(124217570),
        1);
    kasAITeamShipsPtr(kasGrowSelectionPtr(21159), 5);
    kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(21159));
}

void Watch_Mission10_P1StandardCorvetteWing_SpawnReinforcement(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Init_Mission10_P1StandardCorvetteWing_Disable,
            (sdword)Watch_Mission10_P1StandardCorvetteWing_Disable);
    }
}

void Init_Mission10_StrikeGroupMember(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(124217570));
    kasfSpeechEvent(
        76775316,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission10_StrikeGroupMember_Docked);
}

void Watch_Mission10_StrikeGroupMember_Docked(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)205105999))
    {
        kasTimerCreateSetStartById(139352562, 420);
    }
    if (kasTimerExpiredById(139352562))
    {
        kasfSpeechEvent(
            70903842,
            (sdword)Init_LaunchDockedTeam,
            (sdword)Watch_Mission10_StrikeGroupMember_Launched);
    }
}

void Watch_Mission10_StrikeGroupMember_Launched(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        if (!kasfEntryValueGet(-1943004537) && kasfThisTeamIs(kasAITeamPtrIfExists(230547091)))
        {
            kasfEntryGetOrCreateSetValue(-1943004537, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0x3394AFB);
        }
        kasfFormationDelta();
        tutDeallocateRootRegion(kasGrowSelectionPtr(211966048));
    }
    if (kasfMsgReceived((char *)227317879))
    {
        kasfSpeechEvent(
            216270495,
            (sdword)MaintainTeamContext,
            (sdword)Init_Mission10_StrikeGroupMember_ClearObjectiveFocus);
    }
}

void Init_Mission10_StrikeGroupMember_ClearObjectiveFocus(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    tutDeallocateRootRegion(kasAITeamShipsPtrIfExists(90274303));
}

void Init_Mission10_BigGunObjective_ControllerPocket_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 5);
    kasfHideShips(&team->shipList);
    kasfSpeechEvent(
        123839973,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission10_BigGunObjective_ControllerPocket_Hidden);
}

void Watch_Mission10_BigGunObjective_ControllerPocket_Hidden(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552169) && kasfShipsCount(kasGrowSelectionPtr(0x66895FE)))
    {
        kasfEntryGetOrCreateSetValue(71552169, 1);
        kasfUnhideShips(&team->shipList);
    }
    tutDeallocateRootRegion(kasGrowSelectionPtr(0x66895FE));
    if (kasfMsgReceived((char *)265345332))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)MaintainTeamContext,
            (sdword)Init_Mission10_BigGunObjective_ClearTrackedSelection);
    }
}

/*-----------------------------------------------------------------------------
    Init_Mission10 - top-level KAS entry point (IDA: 0x612ac0)
-----------------------------------------------------------------------------*/

extern void Init_Mission09_EventState(void);
extern void Fill_Mission16_ReinforcementPoolFromParade(void);

static void jpg_610B30(void) {}
static void jpg_610C20(void) {}
static void jpg_610F40(void) {}
static void jpg_6112C0(void) {}
static void jpg_6118C0(void);
static void jpg_611A80(void) {}
static void jpg_611C90(void) {}
static void jpg_611CD0(void) {}
static void jpg_612190(void) {}
static void jpg_6121C0(void) {}
static void jpg_612900(void) {}

static void jpg_610AF0(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 5);
    kasfSpeechEvent(157715175, (sdword)jpg_610B30, (sdword)jpg_610C20);
}

static void jpg_610EF0(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 2);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)jpg_610F40);
}

static void jpg_611270(void)
{
    AITeam *dockWithTeam;

    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 2);
    dockWithTeam = kasAITeamPtrIfExists(124217378);
    kasfDockInstantWithTeam(dockWithTeam);
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)jpg_6112C0);
}

static void jpg_611880(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 2);
    kasfTeamAttributesBitSet(128);
}

static void jpg_6118C0(void)
{
    AITeam *team;
    MsgQueue *msgq;
    sdword count;
    sdword i;
    void *item;

    team = teamtoreinforce;
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    msgq = teamtoreinforce->msgQueue;
    count = -1;
    if (msgq)
        count = msgq->head;
    for (i = count; i >= 0; --i)
    {
        item = aitQueuePopPointerByIndex(team, (udword)i);
        if (item)
        {
            if (item == (void *)53402229)
                kasfAttackMothership();
            else if (item == (void *)205105999)
                kasTimerCreateSetStartById(51324946, 240);
            else
                aitQueuePointer(teamtoreinforce, teamtoreinforce, item);
        }
        team = teamtoreinforce;
        msgq = teamtoreinforce->msgQueue;
        if (!msgq || !msgq->head)
            break;
    }
    if (kasTimerExpiredById(51324946))
    {
        kasfTeamHyperspaceIn(kasVectorPtrIfExists(0xDD81A83));
        kasTimerCreateSetStartById(126405506, 10);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0xC9C8D35);
    }
    if (kasTimerExpiredById(126405506))
    {
        kasfMsgSend(kasAITeamPtrIfExists(189144017), (char *)0x506A278);
        kasfMsgSend(kasAITeamPtrIfExists(189144018), (char *)0x506A278);
        kasfMsgSend(kasAITeamPtrIfExists(189144019), (char *)0x506A278);
        kasfMsgSend(kasAITeamPtrIfExists(182076065), (char *)0x506A278);
    }
}

static void jpg_611A20(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 1);
    kasfTeamAttributesBitSet(128);
    kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
    kasfAttackMothership();
    kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)jpg_611A80);
}

static void jpg_611C10(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 1);
    kasfActivateTech("TECH_INFECT");
    kasfShipsApplyPlayerUpgrades(&teamtoreinforce->shipList);
    kasfShipsSetDontApplyPhysicsFlag(&teamtoreinforce->shipList, 1);
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(86933668, (sdword)jpg_611C90, (sdword)jpg_611CD0);
}

static void jpg_612140(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasAITeamShipsPtr(&teamtoreinforce->shipList, 1);
    kasfEntryGetOrCreateSetValue(0x816D1455, 0);
    kasfSpeechEvent(320211, (sdword)jpg_612190, (sdword)jpg_6121C0);
}

static void jpg_6128D0(void)
{
    kasCurrentStep = 1;
    scope = teamtoreinforce->kasLabelHash;
    kasfSpeechEvent(0x469857B, (sdword)Init_ReinforcementPool_ClearSelectionAndReset, (sdword)jpg_612900);
}

void Init_Mission10(void)
{
    kasCurrentStep = 0;
    scope = KH_mission10;
    kasfDisableAllAIFeatures();
    kasfActivateTech("TECH_SALVAGE");
    kasfActivateTech("TECH_MIMIC");
    kasfActivateTech("TECH_CLOAKING");
    kasfActivateTech("TECH_INFECT");
    kasfDisablePlayerHyperspace();

    kasFSMCreate(KH_FleetIntel, Init_Mission09_EventState, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission10_GlobalEvents,
                 kasAITeamPtrIfExists(KH_Objectives));
    kasFSMCreate(124217570, Init_Mission10_BigGunObjective_ResearchShipTeam_Activate, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(124217570));
    kasFSMCreate(74300574, Init_Mission10_BigGunObjective, Watch_Mission10_BigGunObjective,
                 kasAITeamPtrIfExists(74300574));

    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(125709057));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(125709058));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(125709059));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(125709060));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(48009329));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(48009330));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(48009332));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(48009333));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(48009334));
    kasFSMCreate(123839973, Init_Mission10_BigGunAmbushTeam, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(48009331));

    kasFSMCreate(216274622, Init_Mission10_BigGunObjective_ControllerPocket_Start, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(134184993));
    kasFSMCreate(216274622, Init_Mission10_BigGunObjective_ControllerPocket_Start, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(230547090));

    kasFSMCreate(42066307, Init_Mission10_P1StandardCorvetteWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(136202337));
    kasFSMCreate(42066307, Init_Mission10_P1StandardCorvetteWing, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(136202338));

    kasFSMCreate(265332103, Init_Mission10_BigGunObjective_CarrierObjective_Start, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(81489361));

    kasFSMCreate(204451636, jpg_610EF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(37429281));
    kasFSMCreate(204451636, jpg_610EF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(155996337));
    kasFSMCreate(204451636, jpg_610EF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(127661745));
    kasFSMCreate(204451636, jpg_610EF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(148598225));
    kasFSMCreate(204451636, jpg_610EF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(182076065));
    kasFSMCreate(204451636, jpg_610EF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(113126577));

    kasFSMCreate(136391650, jpg_611270, Watch_NoOpTeamContext, kasAITeamPtrIfExists(189144017));
    kasFSMCreate(136391650, jpg_611270, Watch_NoOpTeamContext, kasAITeamPtrIfExists(189144018));
    kasFSMCreate(136391650, jpg_611270, Watch_NoOpTeamContext, kasAITeamPtrIfExists(189144019));

    kasFSMCreate(63041922, jpg_611880, jpg_6118C0, kasAITeamPtrIfExists(124217378));

    kasFSMCreate(150965250, Init_Mission10_StrikeGroupMember, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(230547091));
    kasFSMCreate(150965250, Init_Mission10_StrikeGroupMember, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(230547092));
    kasFSMCreate(150965250, Init_Mission10_StrikeGroupMember, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(134184994));

    kasFSMCreate(239696434, jpg_610AF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(77046737));
    kasFSMCreate(239696434, jpg_610AF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(77046738));
    kasFSMCreate(239696434, jpg_610AF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(77046739));
    kasFSMCreate(239696434, jpg_610AF0, Watch_NoOpTeamContext, kasAITeamPtrIfExists(77046740));

    kasFSMCreate(216270495, Init_Mission10_BigGunObjective_GarrisonObjective_Start,
                 Watch_Mission10_BigGunObjective_GarrisonObjective_HiddenTrigger,
                 kasAITeamPtrIfExists(216270495));

    kasFSMCreate(90274303, jpg_611A20, Watch_NoOpTeamContext, kasAITeamPtrIfExists(90274303));
    kasFSMCreate(170794736, jpg_611C10, Watch_NoOpTeamContext, kasAITeamPtrIfExists(170794736));

    kasFSMCreate(150139171, jpg_612140, Watch_NoOpTeamContext, kasAITeamPtrIfExists(4610161));
    kasFSMCreate(150139171, jpg_612140, Watch_NoOpTeamContext, kasAITeamPtrIfExists(138522161));
    kasFSMCreate(150139171, jpg_612140, Watch_NoOpTeamContext, kasAITeamPtrIfExists(23816689));
    kasFSMCreate(150139171, jpg_612140, Watch_NoOpTeamContext, kasAITeamPtrIfExists(106381681));

    kasFSMCreate(44665972, jpg_6128D0, Fill_Mission16_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(44665972));

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeEnemyCombatShips(kasGrowSelectionPtr(114961833), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeInactivePlayerModules(kasGrowSelectionPtr(KH_Enemy), kasGrowSelectionPtr(18336307));

    kasfShipsSetDelayedDeath(kasAITeamShipsPtrIfExists(326187), 1);
    kasfShipsSetAttachCoordsysFlag(kasAITeamShipsPtrIfExists(326187), 1);
    kasfShipsSetAttachCoordsysFlag(kasAITeamShipsPtrIfExists(176035815), 1);

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
    kasfAllowPlayerToResearch("TECH_FORCE_FIELD2");

    kasfEnableShip("sMothershipDockingBay", "RACE_SECT", 1);
    kasfEnableShip("sMothershipEngineering", "RACE_SECT", 1);
    kasfEnableShip("sMothershipWeapons", "RACE_SECT", 1);
    kasfEnableShip("sMothershipArmour", "RACE_SECT", 1);

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

    kasfModuleSlotOrderSetShipType(1, "bAcolyte");
    kasfModuleSlotOrderSetShipType(1, "bHeavyCorvette");
    kasfModuleSlotOrderSetShipType(1, "p1Fighter");
    kasfModuleSlotOrderSetShipType(1, "p1MissileCorvette");
    kasfModuleSlotOrderSetShipType(1, "tHeavyInterceptor");
    kasfModuleSlotOrderSetShipType(1, "tHeavyDefender");
    kasfModuleSlotOrderSetShipType(1, "tMultiGunCorvette");
    kasfModuleSlotOrderSetShipType(1, "bWorker");
    kasfModuleSlotOrderSetShipType(1, "p1IonArrayFrigate");
    kasfModuleSlotOrderSetShipType(1, "tIonCannonFrigate");
    kasfModuleSlotOrderSetCount(1, 7);

    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasTimerCreateSetStartById(150120898, 1);
}
