/*=============================================================================
    Name    : mission05.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission05
=============================================================================*/

#include "MissionScriptSupport.h"

void Init_Mission05_FleetIntel_SetupState(void);
void Watch_Mission05_FleetIntel_CleanupAndResume(void);
void Watch_Mission05_FleetIntel_ResumeDelay(void);
void Watch_Mission05_FleetIntel_EventFlow(void);
void Watch_Mission05_Traders_FirstContact(void);
void Init_Mission05_Traders_DefendYourSelf(void);
void Watch_Mission05_Traders_DefendYourSelf(void);
void Watch_Mission05_FleetIntel_FIDestroyResourcerTrigger(void);
void Begin_Mission05_FourShipReinforcements(void);
void Begin_BeastACVCombatLoopDelay(void);
void Init_Mission05_BworkerAssaultTeam(void);
void Init_Mission05_BworkerReinforcementDelay(void);
void Watch_Mission05_BworkerReinforcementTeam(void);
void Watch_Mission05_BworkerDockAndReinforce(void);
void Watch_Mission05_BworkerReinforcementBuild(void);
void Watch_Mission05_BworkerAcquireDamagedTargets(void);
void Watch_Mission05_BworkerCloakDamagedTargets(void);
void Watch_Mission05_BworkerSpecialAttackDamagedCapital(void);
void Watch_Mission05_BworkerAssaultTeam(void);
void Watch_Mission03_EnemyFrig01_AttackFighters(void);
void Watch_Mission04_MissionBriefing_RUCrazy(void);
void Init_Tutorial1_HideUnhide(void);
void Watch_Mission05_AssaultGroup_WaitForSignal(void);
void Watch_Mission05_AssaultGroup_AttackNearbyCapital(void);
void Watch_Mission05_ReinforcementGroup_WaitForSignal(void);
void Watch_Mission05_ReinforcementGroup_AttackNearbyCapital(void);
void Watch_Mission05_FinalGroup_AttackNearbyCapital(void);
static void Begin_Mission05_Kamikaze2_BeastACVCombatAttackRun(void);
static void Watch_Mission05_Kamikaze2_BeastACVAttackRun(void);
static void Begin_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop(void);
static void Watch_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop(void);
void Watch_Mission05_Kamikaze2_Launch(void);

static char mission05TraderDialogBlank[1];

void Watch_Mission05(void)
{
  int v0; // eax
  int v1; // eax
  int *v2; // eax
  int v3; // eax
  int *v4; // eax
  int *v6; // eax
  int *v7; // eax
  int v8; // eax
  int *v9; // eax
  int v10; // eax
  int v11; // eax
  int v12; // [esp-8h] [ebp-8h]
  int v13; // [esp-8h] [ebp-8h]
  int v14; // [esp-8h] [ebp-8h]
  int v15; // [esp-4h] [ebp-4h]
  int v16; // [esp-4h] [ebp-4h]
  int v17; // [esp-4h] [ebp-4h]
  int v18; // [esp-4h] [ebp-4h]
  int v19; // [esp-4h] [ebp-4h]
  int v20; // [esp-4h] [ebp-4h]
  int *v21; // [esp-4h] [ebp-4h]

  kasCurrentStep = 0;
  kasCurrentScopeHash = 142503669;
  if ( !kasfEntryValueGet(71552175) )
  {
    v12 = kasGrowSelectionPtr(0x4A2A29);
    v0 = kasGrowSelectionPtr(211966048);
    if ( kasfShipsSelectType(v0, v12, "sMothership") )
      kasfEntrySetValue(71552175, 1);
  }
  if ( kasTimerExpiredDestroyById(50796994) )
  {
    v15 = kasGrowSelectionPtr(18336307);
    v1 = kasVolumePtr(132933767);
    kasfFindShipsInside(v1, v15);
    v16 = kasGrowSelectionPtr(18336307);
    v2 = (int *)kasGrowSelectionPtr(0x4A2A29);
    kasSelectPlayerShips(v2, v16);
    v3 = kasGrowSelectionPtr(0x4A2A29);
    kasfShipsShuffle(v3);
    v17 = kasGrowSelectionPtr(18336307);
    v4 = (int *)kasGrowSelectionPtr(114961833);
    kasSelectEnemyShips(v4, v17);
    kasTimerSetById(50796994, 10);
    kasTimerStartById(50796994);
  }
  if ( !kasfEntryValueGet(71552176) )
  {
    if ( kasfEntryValueGet(-2015429404) )
    {
      kasfEntrySetValue(71552176, 1);
      v18 = kasGrowSelectionPtr(18336307);
      v6 = (int *)kasGrowSelectionPtr(89157026);
      kasSelectPlayerShipsSimple(v6, v18);
      v19 = kasGrowSelectionPtr(89157026);
      v7 = (int *)kasGrowSelectionPtr(89157026);
      kasfShipsSelectNotSalvagers(v7, v19);
      v20 = kasGrowSelectionPtr(218060896);
      v8 = kasGrowSelectionPtr(82017669);
      if ( kasfShipsSelectLinkedShips(v8, v20) )
      {
        v21 = (int *)kasGrowSelectionPtr(82017669);
        v9 = (int *)kasGrowSelectionPtr(89157026);
        kasfShipsAdd(v9, v21);
      }
      if ( GetGameDifficulty() < 2 )
      {
        if ( kasfRUsGet(0) < 500 )
        {
          v13 = kasGrowSelectionPtr(89157026);
          v10 = kasGrowSelectionPtr(96759714);
          if ( !kasfShipsSelectType(v10, v13, "sWorker") )
          {
            v14 = kasGrowSelectionPtr(218060896);
            v11 = kasGrowSelectionPtr(154639956);
            kasfShipsCreateInside(v11, "sWorker", 1, v14, 0);
          }
        }
      }
    }
  }
}

void Init_WaitForIntelEventEnd(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfIntelEventNotEnded();
}

void Init_Mission05_FleetIntel_FIDestroyResourcer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationDelta();
    kasfTacticsNeutral();
    tutDeallocateRootRegion();
}

void Watch_Mission05_FleetIntel_FIDestroyResourcer(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)17444564))
    {
        kasfVarDestroy(kasCurrentTeamPtr + 28);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (kasfTeamCount())
    {
        if (singlePlayerShipsAnyIdle(&team->shipList))
        {
            kasfShipsSelectNearby(
                kasGrowSelectionPtr(57245931),
                &team->shipList,
                kasTeamsVectorPtr(team->kasLabelHash),
                10000);
            tutDeallocateRootRegion();
        }
    }
    else
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_FleetIntel_FIProtectBentusi,
            (sdword)Watch_Mission05_FleetIntel_FIProtectBentusi);
    }
}

void Watch_Mission05_CorvetteReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *selectedCorvettes;
   GrowSelection *shipBuffer;
   GrowSelection *reinforcementShips;
   GrowSelection *fsmShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   shipBuffer = kasGrowSelectionPtr(363040);
   selectedCorvettes = kasGrowSelectionPtr(128635107);
   reinforcementShips = kasAITeamShipsPtrIfExists(44665972);
   kasfShipsSelectType(shipBuffer, reinforcementShips, "bMissileCorvette");
   kasfShipsAdd(selectedCorvettes, shipBuffer);
   kasfShipsSelectType(shipBuffer, reinforcementShips, "bMultiGunCorvette");
   kasfShipsAdd(selectedCorvettes, shipBuffer);
   kasfShipsSelectType(shipBuffer, reinforcementShips, "bHeavyCorvette");
   kasfShipsAdd(selectedCorvettes, shipBuffer);
   if (kasfShipsCount(selectedCorvettes) < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(127797474)))
       {
           if (kasfEntryValueGet(-2082149163))
           {
               kasfEntrySetValue(-2082149163, 0);
               kasfEntrySetValue(-1931839291, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)ResetReinforcementBuildCount,
                   (sdword)Watch_Mission05_CorvetteReinforcementDock);
           }
       }
       else
       {
           kasfEntrySetValue(-2082149163, 0);
           kasfEntrySetValue(-1931839291, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       }
       return;
   }

   fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)41951459);
   kasfEntrySetValue(332667, 0);

   kasfShipsSelectType(shipBuffer, reinforcementShips, "bMissileCorvette");
   if (kasfShipsCount(shipBuffer))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(fsmShips, shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }

   kasfShipsSelectType(shipBuffer, reinforcementShips, "bMultiGunCorvette");
   if (kasfShipsCount(shipBuffer))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(fsmShips, shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }

   kasfShipsSelectType(shipBuffer, reinforcementShips, "bHeavyCorvette");
   if (kasfShipsCount(shipBuffer))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(fsmShips, shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }

   kasfSpeechEvent(74024315, (sdword)Init_Mission05_CorvetteAssault, (sdword)Watch_Mission05_CorvetteAssault);
}

void Watch_Mission05_CorvetteReinforcementDock(void)
{
   sdword buildCount;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               if (kasfEntryValueGet(154639956) < 3)
               {
                   buildCost = kasfShipTypeBuildCost("bMissileCorvette") / 2;
                   if (kasfRUsGet(1) >= buildCost)
                   {
                       kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                       kasfEntryIncrement(154639956);
                   }
               }
               if (kasfEntryValueGet(154639956) <= 1)
               {
                   return;
               }
               buildCost = kasfShipTypeBuildCost("bMissileCorvette") / 2;
               if (kasfRUsGet(1) >= buildCost)
               {
                   return;
               }
               break;
           case 1:
               if (kasfEntryValueGet(154639956) < 3)
               {
                   kasfEntryIncrement(154639956);
               }
               break;
           default:
               return;
       }
   }
   else
   {
       if (kasfEntryValueGet(154639956) < 3)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette");
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 1)
       {
           buildCost = kasfShipTypeBuildCost("bMissileCorvette");
           if (kasfRUsGet(1) < buildCost)
           {
               buildCount = 3;
           }
           else
           {
               buildCount = kasfEntryValueGet(154639956);
           }
       }
       else
       {
           buildCount = kasfEntryValueGet(154639956);
       }
       if (buildCount != 3)
       {
           return;
       }
   }

   kasfSpeechEvent(
       75929221,
       (sdword)Init_Mission05_CorvetteReinforcementDelay,
       (sdword)Watch_Mission05_CorvetteReinforcementBuild);
}

void Init_Mission05_CorvetteReinforcementDelay(void)
{
   sdword buildCount;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerDestroyById(235522949);
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               buildCount = kasfEntryValueGet(154639956);
               kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bMissileCorvette") * buildCount / 2);
               break;
           case 1:
               kasTimerCreateSetStartById(235522949, 0);
               break;
       }
   }
   else
   {
       buildCount = kasfEntryValueGet(154639956);
       kasTimerCreateSetStartById(235522949, buildCount * kasfShipTypeBuildTime("bMissileCorvette"));
   }
}

void Watch_Mission05_CorvetteReinforcementBuild(void)
{
   AITeam *team;
   GrowSelection *motherships;
   GrowSelection *buildSource;
   GrowSelection *fsmShips;
   sdword buildCount;
   char *shipType;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   motherships = kasGrowSelectionPtr(127797474);
   if (!kasfShipsCount(motherships))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       kasfShipsSelectIndex(
           kasGrowSelectionPtr(21541),
           motherships,
           kasfRandom(0, kasfShipsCount(motherships) - 1));

       buildCount = kasfEntryValueGet(154639956) / 2;
       switch (kasfRandom(1, 3) - 1)
       {
           case 1:
               shipType = "bHeavyCorvette";
               break;
           case 2:
               shipType = "bMissileCorvette";
               break;
           default:
               shipType = "bMultiGunCorvette";
               break;
       }
       kasfShipsCreateInside(kasGrowSelectionPtr(154639956), shipType, buildCount, kasGrowSelectionPtr(21541), 1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
       kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
       fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
       kasfShipsAdd(fsmShips, &team->shipList);

       buildCount = kasfEntryValueGet(154639956) / 2;
       switch (kasfRandom(1, 3) - 1)
       {
           case 1:
               shipType = "bHeavyCorvette";
               break;
           case 2:
               shipType = "bMissileCorvette";
               break;
           default:
               shipType = "bMultiGunCorvette";
               break;
       }
       kasfShipsCreateInside(kasGrowSelectionPtr(154639956), shipType, buildCount, kasGrowSelectionPtr(21541), 1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
       kasfShipsAdd(fsmShips, &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_CorvetteAssault, (sdword)Watch_Mission05_CorvetteAssault);
   }
}

void Init_Mission05_CorvetteAssault(void)
{
   AITeam *team;
   GrowSelection *attackers;
   GrowSelection *nearbyShips;
   GrowSelection *nearbyCapitals;
   GrowSelection *fsmShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   attackers = kasGrowSelectionPtr(57245931);
   nearbyShips = kasGrowSelectionPtr(86599289);
   if (kasfShipsSelectNearby(nearbyShips, &team->shipList, kasThisTeamsVectorPtr(), 20000))
   {
       if (!kasfShipsSelectType(attackers, nearbyShips, "sLeech")
        && !kasfShipsSelectType(attackers, nearbyShips, "sMCV")
        && !kasfShipsSelectType(attackers, nearbyShips, "sMimic"))
       {
           kasfShipsAdd(attackers, nearbyShips);
       }
   }
   else
   {
       fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
       nearbyCapitals = kasGrowSelectionPtr(124053532);
       if (kasfShipsSelectNearby(nearbyCapitals, fsmShips, kasShipsVectorPtr((char *)team->kasFSMNameHash), 20000)
        && kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals))
       {
           if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), fsmShips, "CLASS_Corvette")
            && kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
           {
               kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
           }
           if (kasfShipsCount(nearbyCapitals))
           {
               kasfSpeechEvent(
                   0x4C9664,
                   (sdword)Init_Mission05_CorvetteCapitalStrike,
                   (sdword)Watch_Mission05_CorvetteCapitalStrike);
               return;
           }
       }
       kasfGrowSelectionClear(attackers);
       kasfShipsAdd(attackers, &team->shipList);
   }

   if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
   {
       kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
   }
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

void Watch_Mission05_CorvetteAssault(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_CorvetteAssault, (sdword)Watch_Mission05_CorvetteAssault);
   }
}

void Init_Mission05_CorvetteCapitalStrike(void)
{
   AITeam *team;
   GrowSelection *nearbyCapitals;
   GrowSelection *fsmShips;
   GrowSelection *sortedTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
   nearbyCapitals = kasGrowSelectionPtr(124053532);
   if (kasfShipsSelectNearby(nearbyCapitals, fsmShips, kasShipsVectorPtr((char *)team->kasFSMNameHash), 20000)
    && kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals))
   {
       if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), fsmShips, "CLASS_Corvette")
        && kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
       {
           kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
       }
       if (!kasfShipsCount(nearbyCapitals))
       {
           kasfSpeechEvent(74024315, (sdword)Init_Mission05_CorvetteAssault, (sdword)Watch_Mission05_CorvetteAssault);
           return;
       }
   }

   kasfShipsSelectNearby(
       kasGrowSelectionPtr(86599289),
       nearbyCapitals,
       kasShipsVectorPtr((char *)124053532),
       20000);
   kasfShipsSelectType(kasGrowSelectionPtr(212896882), kasGrowSelectionPtr(86599289), "bHeavyCruiser");
   kasfShipsSelectType(kasGrowSelectionPtr(124218850), kasGrowSelectionPtr(86599289), "bCarrier");
   if (kasfShipsCount(kasGrowSelectionPtr(212896882)) || kasfShipsCount(kasGrowSelectionPtr(124218850)))
   {
       kasfGrowSelectionClear(kasGrowSelectionPtr(125446884));
       kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(212896882));
       kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(124218850));
       singlePlayerSortShipsByDistanceToPoint(kasGrowSelectionPtr(125446884), kasThisTeamsVectorPtr());
       sortedTargets = kasGrowSelectionPtr(125446884);
   }
   else
   {
       singlePlayerSortShipsByDistanceToPoint(nearbyCapitals, kasThisTeamsVectorPtr());
       sortedTargets = nearbyCapitals;
   }
   kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), sortedTargets, 0);
   kasfFormationSphere();
   kasfProtect(kasGrowSelectionPtr(154965556));
}

void Watch_Mission05_CorvetteCapitalStrike(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
   {
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_CorvetteAssault, (sdword)Watch_Mission05_CorvetteAssault);
   }
}

void Init_Mission05_DockForReinforcements(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    singlePlayerSortShipsByDistanceToPoint(kasGrowSelectionPtr(127797474), kasThisTeamsVectorPtr());
    kasfShipsSelectIndex(kasGrowSelectionPtr(21541), kasGrowSelectionPtr(127797474), 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    kasfShipsDockWithSelection(&team->shipList, kasGrowSelectionPtr(21541));
}

void Watch_Mission05_CorvetteDockAndReinforce(void)
{
    sdword wantedBuilds;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_CorvetteReinforcementTeam);
        return;
    }

    wantedBuilds = 3 - kasfTeamCount();
    if (kasfEntryValueGet(-2126662828))
    {
        switch (kasfEntryValueGet(-2126662828) - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < wantedBuilds)
                {
                    buildCost = kasfShipTypeBuildCost("bMissileCorvette");
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) > 1
                  && kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette") / 2)
                 || kasfEntryValueGet(154639956) == 3)
                {
                    kasTimerCreateSetStartById(
                        133682484,
                        kasfShipTypeBuildTime("bMissileCorvette") * kasfEntryValueGet(154639956) / 2);
                }
                break;
            case 1:
                if (kasfEntryValueGet(154639956) < wantedBuilds)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - kasfShipTypeBuildCost("bMissileCorvette") / 2);
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 3)
                {
                    kasTimerCreateSetStartById(133682484, 0);
                }
                break;
        }
    }
    else
    {
        if (kasfEntryValueGet(154639956) < wantedBuilds)
        {
            buildCost = kasfShipTypeBuildCost("bMissileCorvette");
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1
          && kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette"))
         || kasfEntryValueGet(154639956) == 3)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bMissileCorvette"));
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        kasfShipsSelectLinkedWith(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr(316548), kasGrowSelectionPtr(127797474));
    }
}

void Init_Mission05_FighterTeam(void)
{
   AITeam *team;
   AITeam *otherTeam;
   GrowSelection *fsmShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   otherTeam = kasAITeamPtrIfExists((char *)182180066);
   if (kasfThisTeamIs(otherTeam))
   {
       return;
   }

   kasfEntryGetOrCreateSetValue(154639956, 0);
   kasfEntryGetOrCreateSetValue(111592388, 0);
   kasfEntryGetOrCreateSetValue(-1985856270, 0);
   kasfEntryGetOrCreateSetValue(-1929432555, 1);
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_FighterReinforcementTeam);
       return;
   }

   fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)233775571);
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfShipsAdd(fsmShips, &team->shipList);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfEntrySetValue(332667, 1);
   kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
}

void Watch_Mission05_FighterTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)182180066))
    && kasfEntryValueGet(-1929432555)
    && kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1929432555, 0);
       kasfEntrySetValue(-1985856270, 1);
   }

   if (!kasfEntryValueGet(332667))
   {
       if (!kasfTeamCount())
       {
           kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)154145491);
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_FighterReinforcementTeam);
           return;
       }
       if (kasfShipsCount(kasGrowSelectionPtr(127797474)) && kasfTeamHealthAverage() < 50)
       {
           kasfSpeechEvent(299899, (sdword)Init_Mission05_DockForReinforcements, (sdword)Watch_Mission05_FighterDockAndReinforce);
       }
   }
}

void Watch_Mission05_FighterReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *shipBuffer;
   GrowSelection *selectedFighters;
   GrowSelection *reinforcementShips;
   GrowSelection *fsmShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   shipBuffer = kasGrowSelectionPtr(363040);
   selectedFighters = kasGrowSelectionPtr(230526675);
   reinforcementShips = kasAITeamShipsPtrIfExists(44665972);
   kasfShipsSelectType(shipBuffer, reinforcementShips, "bInterceptor");
   kasfShipsAdd(selectedFighters, shipBuffer);
   kasfShipsSelectType(shipBuffer, reinforcementShips, "bAcolyte");
   kasfShipsAdd(selectedFighters, shipBuffer);
   kasfShipsSelectType(shipBuffer, reinforcementShips, "bAttackBomber");
   kasfShipsAdd(selectedFighters, shipBuffer);
   if (kasfShipsCount(selectedFighters) < 4)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(127797474)))
       {
           if (kasfEntryValueGet(-1985856270))
           {
               kasfEntrySetValue(-1985856270, 0);
               kasfEntrySetValue(-1929432555, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)Init_Mission05_FighterReinforcementDelay,
                   (sdword)Watch_Mission05_FighterReinforcementDock);
           }
       }
       else
       {
           kasfEntrySetValue(-1985856270, 0);
           kasfEntrySetValue(-1929432555, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       }
       return;
   }

   fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)233775571);
   kasfEntrySetValue(332667, 0);

   kasfShipsSelectType(shipBuffer, reinforcementShips, "bInterceptor");
   if (kasfShipsCount(shipBuffer))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(fsmShips, shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }

   kasfShipsSelectType(shipBuffer, reinforcementShips, "bAcolyte");
   if (kasfShipsCount(shipBuffer))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(fsmShips, shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }

   kasfShipsSelectType(shipBuffer, reinforcementShips, "bAttackBomber");
   if (kasfShipsCount(shipBuffer))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(fsmShips, shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }

   kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
}

void Watch_Mission05_FighterReinforcementDock(void)
{
   sdword buildCount;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               if (kasfEntryValueGet(154639956) < 6)
               {
                   buildCost = kasfShipTypeBuildCost("bAcolyte") / 2;
                   if (kasfRUsGet(1) >= buildCost)
                   {
                       kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                       kasfEntryIncrement(154639956);
                   }
               }
               if (kasfEntryValueGet(154639956) <= 4)
               {
                   return;
               }
               if (kasfRUsGet(1) >= kasfShipTypeBuildCost("bAcolyte") / 2)
               {
                   return;
               }
               break;
           case 1:
               if (kasfEntryValueGet(154639956) < 6)
               {
                   kasfEntryIncrement(154639956);
               }
               if (kasfEntryValueGet(154639956) != 6)
               {
                   return;
               }
               break;
           default:
               return;
       }
   }
   else
   {
       if (kasfEntryValueGet(154639956) < 6)
       {
           buildCost = kasfShipTypeBuildCost("bAcolyte");
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 4 && kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte"))
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_FighterReinforcementDelay,
               (sdword)Watch_Mission05_FighterReinforcementBuild);
           return;
       }
       if (kasfEntryValueGet(154639956) != 6)
       {
           return;
       }
   }

   kasfSpeechEvent(
       75929221,
       (sdword)Init_Mission05_FighterReinforcementDelay,
       (sdword)Watch_Mission05_FighterReinforcementBuild);
}

void Init_Mission05_FighterReinforcementDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerDestroyById(235522949);
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               kasTimerCreateSetStartById(
                   235522949,
                   kasfShipTypeBuildTime("bAcolyte") * kasfEntryValueGet(154639956) / 2);
               break;
           case 1:
               kasTimerCreateSetStartById(235522949, 0);
               break;
       }
   }
   else
   {
       kasTimerCreateSetStartById(
           235522949,
           kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bAcolyte"));
   }
}

void Watch_Mission05_FighterReinforcementBuild(void)
{
   AITeam *team;
   GrowSelection *motherships;
   GrowSelection *fsmShips;
   sdword spawnIndex;
   char *shipType;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   motherships = kasGrowSelectionPtr(127797474);
   if (!kasfShipsCount(motherships))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       spawnIndex = kasfRandom(0, kasfShipsCount(motherships) - 1);
       kasfShipsSelectIndex(kasGrowSelectionPtr(21541), motherships, spawnIndex);
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
       kasfShipsCreateInside(
           kasGrowSelectionPtr(154639956),
           shipType,
           kasfEntryValueGet(154639956),
           kasGrowSelectionPtr(21541),
           1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
       fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
       kasfShipsAdd(fsmShips, &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)233775571);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
   }
}

void Init_Mission05_FighterAssault(void)
{
   AITeam *team;
   GrowSelection *attackers;
   GrowSelection *nearbyShips;
   GrowSelection *nearbyCapitals;
   GrowSelection *fighters;
   GrowSelection *fsmShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   attackers = kasGrowSelectionPtr(57245931);
   nearbyShips = kasGrowSelectionPtr(86599289);
   if (kasfShipsSelectNearby(nearbyShips, &team->shipList, kasThisTeamsVectorPtr(), 20000))
   {
       if (!kasfShipsSelectType(attackers, nearbyShips, "sLeech")
        && !kasfShipsSelectType(attackers, nearbyShips, "sMCV")
        && !kasfShipsSelectType(attackers, nearbyShips, "sMimic")
        && !kasfShipsSelectClass(attackers, nearbyShips, "CLASS_Fighter"))
       {
           kasfShipsAdd(attackers, nearbyShips);
       }
   }
   else
   {
       fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
       nearbyCapitals = kasGrowSelectionPtr(124053532);
       if (kasfShipsSelectNearby(nearbyCapitals, fsmShips, kasShipsVectorPtr((char *)team->kasFSMNameHash), 20000)
        && kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals))
       {
           fighters = kasGrowSelectionPtr(182180066);
           if (kasfShipsSelectClass(fighters, fsmShips, "CLASS_Fighter")
            && kasfShipsSelectTargets(fighters, kasGrowSelectionPtr(157715175)))
           {
               kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
           }
           if (kasfShipsCount(nearbyCapitals))
           {
               kasfSpeechEvent(
                   0x4C9664,
                   (sdword)Init_Mission05_FighterCapitalStrike,
                   (sdword)Watch_Mission05_FighterCapitalStrike);
               return;
           }
       }
       kasfGrowSelectionClear(attackers);
       kasfShipsAdd(attackers, &team->shipList);
   }

   if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
   {
       kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
   }
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

void Watch_Mission05_FighterAssault(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
   {
       kasTimerDestroyById(74024315);
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
   }
}

void Init_Mission05_FighterCapitalStrike(void)
{
   AITeam *team;
   GrowSelection *nearbyCapitals;
   GrowSelection *fsmShips;
   GrowSelection *fighters;
   GrowSelection *sortedTargets;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
   nearbyCapitals = kasGrowSelectionPtr(124053532);
   if (kasfShipsSelectNearby(nearbyCapitals, fsmShips, kasShipsVectorPtr((char *)team->kasFSMNameHash), 20000)
    && kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals))
   {
       fighters = kasGrowSelectionPtr(182180066);
       if (kasfShipsSelectClass(fighters, fsmShips, "CLASS_Fighter")
        && kasfShipsSelectTargets(fighters, kasGrowSelectionPtr(157715175)))
       {
           kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
       }
       if (!kasfShipsCount(nearbyCapitals))
       {
           kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
           return;
       }
   }
   else
   {
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
       return;
   }

   kasfShipsSelectNearby(
       kasGrowSelectionPtr(86599289),
       nearbyCapitals,
       kasShipsVectorPtr((char *)124053532),
       20000);
   kasfShipsSelectType(kasGrowSelectionPtr(212896882), kasGrowSelectionPtr(86599289), "bHeavyCruiser");
   kasfShipsSelectType(kasGrowSelectionPtr(124218850), kasGrowSelectionPtr(86599289), "bCarrier");
   if (kasfShipsCount(kasGrowSelectionPtr(212896882)) || kasfShipsCount(kasGrowSelectionPtr(124218850)))
   {
       kasfGrowSelectionClear(kasGrowSelectionPtr(125446884));
       kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(212896882));
       kasfShipsAdd(kasGrowSelectionPtr(125446884), kasGrowSelectionPtr(124218850));
       singlePlayerSortShipsByDistanceToPoint(kasGrowSelectionPtr(125446884), kasThisTeamsVectorPtr());
       sortedTargets = kasGrowSelectionPtr(125446884);
   }
   else
   {
       singlePlayerSortShipsByDistanceToPoint(nearbyCapitals, kasThisTeamsVectorPtr());
       sortedTargets = nearbyCapitals;
   }
   kasfShipsSelectIndex(kasGrowSelectionPtr(154965556), sortedTargets, 0);
   kasfFormationWall();
   kasfProtect(kasGrowSelectionPtr(154965556));
}

void Watch_Mission05_FighterCapitalStrike(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), &team->shipList, kasThisTeamsVectorPtr(), 20000))
   {
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_FighterAssault, (sdword)Watch_Mission05_FighterAssault);
   }
   else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
   {
       kasfSpeechEvent(0x4C9664, (sdword)Init_Mission05_FighterCapitalStrike, (sdword)Watch_Mission05_FighterCapitalStrike);
   }
}

void Watch_Mission05_FighterDockAndReinforce(void)
{
   sdword wantedBuilds;
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_FighterReinforcementTeam);
       return;
   }

   wantedBuilds = 6 - kasfTeamCount();
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               if (kasfEntryValueGet(154639956) < wantedBuilds)
               {
                   buildCost = kasfShipTypeBuildCost("bAcolyte") / 2;
                   if (kasfRUsGet(1) >= buildCost)
                   {
                       kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                       kasfEntryIncrement(154639956);
                   }
               }
               if ((kasfEntryValueGet(154639956) > 4
                 && kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte") / 2)
                || kasfEntryValueGet(154639956) == 6)
               {
                   kasTimerCreateSetStartById(
                       133682484,
                       kasfShipTypeBuildTime("bAcolyte") * kasfEntryValueGet(154639956) / 2);
               }
               break;
           case 1:
               if (kasfEntryValueGet(154639956) < wantedBuilds)
               {
                   kasfEntryIncrement(154639956);
               }
               if (kasfEntryValueGet(154639956) == 6)
               {
                   kasTimerCreateSetStartById(133682484, 0);
               }
               break;
       }
   }
   else
   {
       if (kasfEntryValueGet(154639956) < wantedBuilds)
       {
           buildCost = kasfShipTypeBuildCost("bAcolyte");
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if ((kasfEntryValueGet(154639956) > 4
         && kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte"))
        || kasfEntryValueGet(154639956) == 6)
       {
           kasTimerCreateSetStartById(
               133682484,
               kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bAcolyte"));
       }
   }

   if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
   {
       kasfShipsSelectLinkedWith(&((AITeam *)kasCurrentTeamPtr)->shipList, kasGrowSelectionPtr(316548), kasGrowSelectionPtr(127797474));
   }
}

void Init_Mission05_RammingFrigateTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)223105365)))
   {
       return;
   }

   kasfEntryGetOrCreateSetValue(154639956, 0);
   kasfEntryGetOrCreateSetValue(111592388, 0);
   kasfEntryGetOrCreateSetValue(-1997402011, 0);
   kasfEntryGetOrCreateSetValue(-2141093323, 1);
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_RammingFrigateReinforcementTeam);
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)65242483);
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfEntrySetValue(332667, 0);
   kasfSpeechEvent(74024315, (sdword)Init_Mission05_RammingFrigateAssault, (sdword)Watch_Mission05_RammingFrigateAssault);
}

void Watch_Mission05_RammingFrigateTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)223105365))
    && kasfEntryValueGet(-2141093323)
    && kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-2141093323, 0);
       kasfEntrySetValue(-1997402011, 1);
   }
   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)203347251);
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_RammingFrigateReinforcementTeam);
   }
}

void Watch_Mission05_RammingFrigateReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *shipBuffer;
   GrowSelection *reinforcementShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   shipBuffer = kasGrowSelectionPtr(363040);
   reinforcementShips = kasAITeamShipsPtrIfExists(44665972);
   if (kasfShipsSelectType(shipBuffer, reinforcementShips, "bRammingFrigate") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(127797474)))
       {
           if (kasfEntryValueGet(-1997402011))
           {
               kasfEntrySetValue(-1997402011, 0);
               kasfEntrySetValue(-2141093323, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
                   (sdword)Watch_Mission05_RammingFrigateReinforcementDock);
           }
       }
       else
       {
           kasfEntrySetValue(-1997402011, 0);
           kasfEntrySetValue(-2141093323, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)65242483);
   kasfEntrySetValue(332667, 0);
   kasfShipsRemove(reinforcementShips, shipBuffer);
   kasfShipsStop(shipBuffer);
   kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), shipBuffer);
   kasfShipsSetSlaveableFlag(shipBuffer, 0);
   kasfShipsApplyPlayerUpgrades(shipBuffer);
   kasfReinforceTeamWithShips(team, shipBuffer);
   kasfTacticsNeutral();
   kasfSpeechEvent(74024315, (sdword)Init_Mission05_RammingFrigateAssault, (sdword)Watch_Mission05_RammingFrigateAssault);
}

void Watch_Mission05_RammingFrigateReinforcementDock(void)
{
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               if (kasfEntryValueGet(154639956) < 1)
               {
                   buildCost = kasfShipTypeBuildCost("bRammingFrigate") / 2;
                   if (kasfRUsGet(1) >= buildCost)
                   {
                       kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                       kasfEntryIncrement(154639956);
                   }
               }
               if (kasfEntryValueGet(154639956) <= 1)
               {
                   return;
               }
               if (kasfRUsGet(1) >= kasfShipTypeBuildCost("bRammingFrigate") / 2)
               {
                   return;
               }
               break;
           case 1:
               if (kasfEntryValueGet(154639956) < 1)
               {
                   kasfEntryIncrement(154639956);
               }
               if (kasfEntryValueGet(154639956) != 1)
               {
                   return;
               }
               break;
           default:
               return;
       }
   }
   else
   {
       if (kasfEntryValueGet(154639956) < 1)
       {
           buildCost = kasfShipTypeBuildCost("bRammingFrigate");
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bRammingFrigate"))
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
               (sdword)Watch_Mission05_RammingFrigateReinforcementBuild);
           return;
       }
       if (kasfEntryValueGet(154639956) != 1)
       {
           return;
       }
   }

   kasfSpeechEvent(
       75929221,
       (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
       (sdword)Watch_Mission05_RammingFrigateReinforcementBuild);
}

void Init_Mission05_RammingFrigateReinforcementDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerDestroyById(235522949);
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               kasTimerCreateSetStartById(
                   235522949,
                   kasfShipTypeBuildTime("bRammingFrigate") * kasfEntryValueGet(154639956) / 2);
               break;
           case 1:
               kasTimerCreateSetStartById(235522949, 0);
               break;
       }
   }
   else
   {
       kasTimerCreateSetStartById(
           235522949,
           kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bRammingFrigate"));
   }
}

void Watch_Mission05_RammingFrigateReinforcementBuild(void)
{
   AITeam *team;
   GrowSelection *motherships;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   motherships = kasGrowSelectionPtr(127797474);
   if (!kasfShipsCount(motherships))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       kasfShipsSelectIndex(
           kasGrowSelectionPtr(21541),
           motherships,
           kasfRandom(0, kasfShipsCount(motherships) - 1));
       kasfShipsCreateInside(
           kasGrowSelectionPtr(154639956),
           "bRammingFrigate",
           kasfEntryValueGet(154639956),
           kasGrowSelectionPtr(21541),
           1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
       kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)65242483);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(74024315, (sdword)Init_Mission05_RammingFrigateAssault, (sdword)Watch_Mission05_RammingFrigateAssault);
   }
}

void Init_Mission05_RammingFrigateAssault(void)
{
   AITeam *team;
   GrowSelection *potentialTargets;
   GrowSelection *attackers;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   potentialTargets = kasGrowSelectionPtr(93706695);
   attackers = kasGrowSelectionPtr(57245931);
   kasfShipsAdd(potentialTargets, &team->shipList);
   kasfShipsSelectType(kasGrowSelectionPtr(363040), potentialTargets, "kCarrier");
   kasfShipsRemove(potentialTargets, kasGrowSelectionPtr(363040));
   if (!kasfShipsSelectNearby(attackers, potentialTargets, kasThisTeamsVectorPtr(), 20000))
   {
       kasfShipsAdd(attackers, potentialTargets);
   }
   if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
   {
       kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
   }
   kasfFormationWall();
   if (kasfShipsSelectCapital(kasGrowSelectionPtr(124053532), attackers))
   {
       tutDeallocateRootRegion();
   }
   else
   {
       tutDeallocateRootRegion();
   }
   kasTimerDestroyById(74024315);
   kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission05_RammingFrigateAssault(void)
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
           (sdword)Init_Mission05_RammingFrigateAssault,
           (sdword)Watch_Mission05_RammingFrigateAssault);
   }
}

void Init_Mission05_BeastWaveActivation(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists((char *)208120516), (char *)4693508);
    kasfSpeechEvent(32713509, (sdword)MaintainTeamContext, (sdword)Watch_Mission05_BeastWaveCoordinator);
}

void Init_Mission05_BeastWaveResumeDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(374244, 5);
}

void Watch_Mission05_BeastWaveResumeDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfSpeechEvent(32713509, (sdword)MaintainTeamContext, (sdword)Watch_Mission05_BeastWaveCoordinator);
    }
}

void Watch_Mission05_BeastWaveCoordinator(void)
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
        switch (message)
        {
            case 203347251:
                kasfMsgSend(kasAITeamPtrIfExists((char *)223105365), (char *)4693508);
                break;
            case 154145491:
                kasfMsgSend(kasAITeamPtrIfExists((char *)182180066), (char *)4693508);
                break;
            case 0x0C2F7304:
                kasfMsgSend(kasAITeamPtrIfExists((char *)208120516), (char *)4693508);
                break;
            case 0x0D35EDB3:
                kasfMsgSend(kasAITeamPtrIfExists((char *)151690773), (char *)4693508);
                break;
            case 0x0F7310A3:
                kasfMsgSend(kasAITeamPtrIfExists((char *)75148549), (char *)4693508);
                break;
            case 0x07ECB382:
                kasfMsgSend(kasAITeamPtrIfExists((char *)75148549), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists((char *)182180066), (char *)4693508);
                kasfEntrySetValue(-2126662828, 0);
                kasfMsgSend(kasAITeamPtrIfExists((char *)182180066), (char *)4693508);
                kasfSpeechEvent(
                    374244,
                    (sdword)Init_Mission05_BeastWaveResumeDelay,
                    (sdword)Watch_Mission05_BeastWaveResumeDelay);
                return;
            case 0x010A2ED4:
                kasTimerCreateSetStartById(132952960, 30);
                break;
            case 0x07ECB381:
                kasfEntryGetOrCreateSetValue(-2126662828, 2);
                kasfMsgSend(kasAITeamPtrIfExists((char *)75148549), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists((char *)182180066), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists((char *)223105365), (char *)4693508);
                kasfMsgSend(kasAITeamPtrIfExists((char *)208120516), (char *)4693508);
                kasfMsgSend(team, (char *)132952962);
                kasfSpeechEvent(
                    374244,
                    (sdword)Init_Mission05_BeastWaveResumeDelay,
                    (sdword)Watch_Mission05_BeastWaveResumeDelay);
                return;
        }
        queueIndex--;
    }

    if (kasTimerExpiredById(132952960))
    {
        kasfMsgSend(team, (char *)132952961);
    }
}

void Init_Mission05_MissionFlow(void)
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
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfEntryGetOrCreateSetValue(237188773, 0);
    kasfTutEnableFlags("Focus");
    kasfTutEnableFlags("CancelFocus");
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission05_MissionFlow);
}

void Watch_Mission05_MissionFlow(void)
{
    if (!kasfIntelEventEnded() && !kasfEntryValueGet(-2054310428))
    {
        return;
    }

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
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
        kasfEntrySetValue(127756738, 0);
        kasTimerStartById(144337492);
    }
    if (kasfEntryValueGet(237188773))
    {
        kasfEntrySetValue(237188773, 0);
        kasfShipsSetDontApplyPhysicsFlag(kasGrowSelectionPtr(237188773), 0);
    }
    kasfSpeechEvent(374244, (sdword)MaintainTeamContext, (sdword)Watch_Mission05_MissionFlow);
}

void Watch_Mission05_FleetIntel_FIObjectivesComplete(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)84320888))
    {
        kasfSpeechEvent(84320888, (sdword)Init_LaunchDockedTeam, (sdword)Watch_Mission13_MissionFlow_Farewell);
    }
}

void Watch_Mission05_Interceptors_Launch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamHealthAverage() > 40)
    {
        kasfTeamSetPercentDamaged(40);
    }
}

void Init_Mission05_WorkerHarvestTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)208120516)))
   {
       return;
   }

   kasfEntryGetOrCreateSetValue(154639956, 0);
   kasfEntryGetOrCreateSetValue(111592388, 0);
   kasfEntryGetOrCreateSetValue(-1893839148, 0);
   kasfEntryGetOrCreateSetValue(-1898467739, 1);
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_WorkerHarvestReinforcementTeam);
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)207781940);
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfEntrySetValue(332667, 0);
   kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission05_WorkerHarvestResourceLoop);
}

void Watch_Mission05_WorkerHarvestTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)208120516))
    && kasfEntryValueGet(-1898467739)
    && kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1898467739, 0);
       kasfEntrySetValue(-1893839148, 1);
   }
   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)204436228);
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_WorkerHarvestReinforcementTeam);
   }
}

void Watch_Mission05_WorkerHarvestReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *shipBuffer;
   GrowSelection *reinforcementShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   shipBuffer = kasGrowSelectionPtr(363040);
   reinforcementShips = kasAITeamShipsPtrIfExists(44665972);
   if (kasfShipsSelectType(shipBuffer, reinforcementShips, "bWorker") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(127797474)))
       {
           if (kasfEntryValueGet(-1893839148))
           {
               kasfEntrySetValue(-1893839148, 0);
               kasfEntrySetValue(-1898467739, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
                   (sdword)Watch_Mission05_WorkerHarvestDockAndReinforce);
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

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)207781940);
   kasfEntrySetValue(332667, 0);
   if (kasfShipsSelectType(shipBuffer, reinforcementShips, "bWorker"))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission05_WorkerHarvestResourceLoop);
}

void Watch_Mission05_WorkerHarvestDockAndReinforce(void)
{
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               if (kasfEntryValueGet(154639956) < 4)
               {
                   buildCost = kasfShipTypeBuildCost("bWorker") / 2;
                   if (kasfRUsGet(1) >= buildCost)
                   {
                       kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                       kasfEntryIncrement(154639956);
                   }
               }
               if (kasfEntryValueGet(154639956) <= 1)
               {
                   return;
               }
               if (kasfRUsGet(1) >= kasfShipTypeBuildCost("bWorker") / 2)
               {
                   return;
               }
               break;
           case 1:
               if (kasfEntryValueGet(154639956) < 4)
               {
                   kasfEntryIncrement(154639956);
               }
               if (kasfEntryValueGet(154639956) != 4)
               {
                   return;
               }
               break;
           default:
               return;
       }
   }
   else
   {
       if (kasfEntryValueGet(154639956) < 4)
       {
           buildCost = kasfShipTypeBuildCost("bWorker");
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bWorker"))
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
               (sdword)Watch_Mission05_WorkerHarvestReinforcementBuild);
           return;
       }
       if (kasfEntryValueGet(154639956) != 4)
       {
           return;
       }
   }

   kasfSpeechEvent(
       75929221,
       (sdword)Init_Mission05_WorkerHarvestReinforcementDelay,
       (sdword)Watch_Mission05_WorkerHarvestReinforcementBuild);
}

void Init_Mission05_WorkerHarvestReinforcementDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerDestroyById(235522949);
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               kasTimerCreateSetStartById(
                   235522949,
                   kasfShipTypeBuildTime("bWorker") * kasfEntryValueGet(154639956) / 2);
               break;
           case 1:
               kasTimerCreateSetStartById(235522949, 1);
               break;
       }
   }
   else
   {
       kasTimerCreateSetStartById(
           235522949,
           kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bWorker"));
   }
}

void Watch_Mission05_WorkerHarvestReinforcementBuild(void)
{
   AITeam *team;
   GrowSelection *motherships;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   motherships = kasGrowSelectionPtr(127797474);
   if (!kasfShipsCount(motherships))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       kasfShipsSelectIndex(
           kasGrowSelectionPtr(21541),
           motherships,
           kasfRandom(0, kasfShipsCount(motherships) - 1));
       kasfShipsCreateInside(
           kasGrowSelectionPtr(154639956),
           "bWorker",
           kasfEntryValueGet(154639956),
           kasGrowSelectionPtr(21541),
           1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
       kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
   }

   if (kasfTeamFinishedLaunching())
   {
       kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)207781940);
       kasfEntrySetValue(332667, 0);
       kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission05_WorkerHarvestResourceLoop);
   }
}

void Init_Mission05_FrigateTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)151690773)))
   {
       return;
   }

   kasfEntryGetOrCreateSetValue(20314791, 0);
   kasfEntryGetOrCreateSetValue(154639956, 0);
   kasfEntryGetOrCreateSetValue(111592388, 0);
   kasfEntryGetOrCreateSetValue(-2131849467, 0);
   kasfEntryGetOrCreateSetValue(-1895203163, 1);
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_FrigateReinforcementTeam);
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)34005347);
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfEntrySetValue(332667, 0);
   singlePlayerTeamSpecialToggle();
   kasfEntrySetValue(20314791, 1);
   kasfSpeechEvent(
       337061,
       (sdword)Begin_BeastIonArrayFrigateCombatLoop,
       (sdword)Watch_Mission05_FrigateReinforcementCombatLoop);
}

void Watch_Mission05_FrigateTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)151690773))
    && kasfEntryValueGet(-1895203163)
    && kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1895203163, 0);
       kasfEntrySetValue(-2131849467, 1);
   }
   if (!kasfEntryValueGet(332667) && !kasfTeamCount())
   {
       kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)221638067);
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_FrigateReinforcementTeam);
   }
}

void Watch_Mission05_FrigateReinforcementTeam(void)
{
   AITeam *team;
   GrowSelection *shipBuffer;
   GrowSelection *reinforcementShips;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   reinforcementShips = kasAITeamShipsPtrIfExists(44665972);
   shipBuffer = kasGrowSelectionPtr(363040);
   if (kasfShipsSelectType(shipBuffer, reinforcementShips, "bIonArrayFrigate") < 1)
   {
       if (kasfShipsCount(kasGrowSelectionPtr(127797474)))
       {
           if (kasfEntryValueGet(-2131849467))
           {
               kasfEntrySetValue(-2131849467, 0);
               kasfEntrySetValue(-1895203163, 1);
               kasfSpeechEvent(
                   165710423,
                   (sdword)Init_Mission05_FrigateReinforcementDelay,
                   (sdword)Watch_Mission05_FrigateDockAndReinforce);
           }
       }
       else
       {
           kasfEntrySetValue(-2131849467, 0);
           kasfEntrySetValue(-1895203163, 1);
           kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       }
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)34005347);
   kasfEntrySetValue(332667, 0);
   singlePlayerTeamSpecialToggle();
   kasfEntrySetValue(20314791, 1);
   if (kasfShipsSelectType(shipBuffer, reinforcementShips, "bIonArrayFrigate"))
   {
       kasfShipsRemove(reinforcementShips, shipBuffer);
       kasfShipsStop(shipBuffer);
       kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), shipBuffer);
       kasfShipsSetSlaveableFlag(shipBuffer, 0);
       kasfShipsApplyPlayerUpgrades(shipBuffer);
       kasfReinforceTeamWithShips(team, shipBuffer);
       kasfTacticsNeutral();
   }
   kasfSpeechEvent(
       337061,
       (sdword)Begin_BeastIonArrayFrigateCombatLoop,
       (sdword)Watch_Mission05_FrigateReinforcementCombatLoop);
}

void Watch_Mission05_FrigateDockAndReinforce(void)
{
   sdword buildCost;

   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               if (kasfEntryValueGet(154639956) < 2)
               {
                   buildCost = kasfShipTypeBuildCost("bIonArrayFrigate") / 2;
                   if (kasfRUsGet(1) >= buildCost)
                   {
                       kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                       kasfEntryIncrement(154639956);
                   }
               }
               if (kasfEntryValueGet(154639956) <= 1)
               {
                   return;
               }
               if (kasfRUsGet(1) >= kasfShipTypeBuildCost("bIonArrayFrigate") / 2)
               {
                   return;
               }
               break;
           case 1:
               if (kasfEntryValueGet(154639956) < 2)
               {
                   kasfEntryIncrement(154639956);
               }
               if (kasfEntryValueGet(154639956) != 2)
               {
                   return;
               }
               break;
           default:
               return;
       }
   }
   else
   {
       if (kasfEntryValueGet(154639956) < 2)
       {
           buildCost = kasfShipTypeBuildCost("bIonArrayFrigate");
           if (kasfRUsGet(1) >= buildCost)
           {
               kasfRUsSet(1, kasfRUsGet(1) - buildCost);
               kasfEntryIncrement(154639956);
           }
       }
       if (kasfEntryValueGet(154639956) > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bIonArrayFrigate"))
       {
           kasfSpeechEvent(
               75929221,
               (sdword)Init_Mission05_FrigateReinforcementDelay,
               (sdword)Watch_Mission05_FrigateReinforcementBuild);
           return;
       }
       if (kasfEntryValueGet(154639956) != 2)
       {
           return;
       }
   }

   kasfSpeechEvent(
       75929221,
       (sdword)Init_Mission05_FrigateReinforcementDelay,
       (sdword)Watch_Mission05_FrigateReinforcementBuild);
}

void Init_Mission05_FrigateReinforcementDelay(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasTimerDestroyById(235522949);
   if (kasfEntryValueGet(-2126662828))
   {
       switch (kasfEntryValueGet(-2126662828) - 1)
       {
           case 0:
               kasTimerCreateSetStartById(
                   235522949,
                   kasfShipTypeBuildTime("bIonArrayFrigate") * kasfEntryValueGet(154639956) / 2);
               break;
           case 1:
               kasTimerCreateSetStartById(235522949, 0);
               break;
       }
   }
   else
   {
       kasTimerCreateSetStartById(
           235522949,
           kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bIonArrayFrigate"));
   }
}

void Watch_Mission05_FrigateReinforcementBuild(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 2;
   kasCurrentScopeHash = team->kasLabelHash;
   if (!kasfShipsCount(kasGrowSelectionPtr(127797474)))
   {
       kasfEntrySetValue(154639956, 0);
       kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
       return;
   }

   if (kasTimerExpiredById(235522949))
   {
       kasfStop();
       singlePlayerShipsCreate(kasGrowSelectionPtr(154639956), "bIonArrayFrigate", kasfEntryValueGet(154639956), 1);
       kasAITeamShipsPtr(kasGrowSelectionPtr(154639956), 1);
       kasfEntrySetValue(154639956, 0);
       kasfReinforceTeamWithShips(team, kasGrowSelectionPtr(154639956));
       kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
       kasfSpeechEvent(
           175938357,
           (sdword)ResetReinforcementHyperspaceAttackFlag,
           (sdword)Watch_Mission05_FrigateLaunchDockedReinforcements);
   }
}

void Init_Mission05_CorvetteTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   kasfEntryGetOrCreateSetValue(332667, 1);
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)75148549)))
   {
       return;
   }

   kasfEntryGetOrCreateSetValue(154639956, 0);
   kasfEntryGetOrCreateSetValue(111592388, 0);
   kasfEntryGetOrCreateSetValue(-2082149163, 0);
   kasfEntryGetOrCreateSetValue(-1931839291, 1);
   if (!kasfTeamCount())
   {
       kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_CorvetteReinforcementTeam);
       return;
   }

   kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)41951459);
   kasAITeamShipsPtr(&team->shipList, 1);
   kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
   kasfShipsApplyPlayerUpgrades(&team->shipList);
   kasfEntrySetValue(332667, 0);
   kasfSpeechEvent(74024315, (sdword)Init_Mission05_CorvetteAssault, (sdword)Watch_Mission05_CorvetteAssault);
}

void Watch_Mission05_CorvetteTeam(void)
{
   AITeam *team;

   team = (AITeam *)kasCurrentTeamPtr;
   kasCurrentStep = 1;
   kasCurrentScopeHash = team->kasLabelHash;
   if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)75148549))
    && kasfEntryValueGet(-1931839291)
    && kasfMsgReceived((char *)4693508))
   {
       kasfEntrySetValue(-1931839291, 0);
       kasfEntrySetValue(-2082149163, 1);
   }
   if (!kasfEntryValueGet(332667))
   {
       if (kasfTeamCount())
       {
           if (kasfShipsCount(kasGrowSelectionPtr(127797474)) && kasfTeamHealthAverage() < 50)
           {
               kasfSpeechEvent(
                   299899,
                   (sdword)Init_Mission05_DockForReinforcements,
                   (sdword)Watch_Mission05_CorvetteDockAndReinforce);
           }
       }
       else
       {
           kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)259199139);
           kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_CorvetteReinforcementTeam);
       }
   }
}

void Init_Mission05_FleetIntel_FIProtectBentusi(void)
{
    GrowSelection *createdShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    createdShips = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(createdShips);
    kasfShipsCreateInside(createdShips, "bAcolyte", 6, kasAITeamShipsPtrIfExists(108436545), 1);
    kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, createdShips);
}

void Watch_Mission05_FleetIntel_FIProtectBentusi(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching() || !kasfShipsCount(kasAITeamShipsPtrIfExists(108436545)))
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(93748889)))
        {
            kasfSpeechEvent(
                66605385,
                (sdword)Init_Mission05_FleetIntel_FIDestroyResourcer,
                (sdword)Watch_Mission05_FleetIntel_FIDestroyResourcer);
        }
        else
        {
            kasfSpeechEvent(
                139967671,
                (sdword)Init_Mission05_FleetIntel_FIProtectBentusiCleanup,
                (sdword)Watch_Mission05_FleetIntel_FIProtectBentusiCleanup);
        }
    }
}

void Init_Mission05_FleetIntel_FIProtectBentusiCleanup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationDelta();
    kasfTacticsAggressive();
    tutDeallocateRootRegion();
}

void Watch_Mission05_FleetIntel_FIProtectBentusiCleanup(void)
{
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (singlePlayerShipsAnyIdle(&team->shipList))
        {
            tutDeallocateRootRegion();
        }
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission05_FrigateLaunchDockedReinforcements(void)
{
    AITeam *team;
    GrowSelection *launchSelection;
    GrowSelection *motherships;
    sdword randomIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        motherships = kasGrowSelectionPtr(127797474);
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
            singlePlayerShipsSetMilitaryParade(kasGrowSelectionPtr(21541), launchSelection);
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

void Watch_Mission05_FrigateReinforcementCombatLoop(void)
{
    AITeam *team;
    GrowSelection *nearbyTargets;
    GrowSelection *underAttackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(nearbyTargets, &team->shipList, kasThisTeamsVectorPtr(), 6000)
     && ((kasfUnderAttack(kasGrowSelectionPtr(139967715))
       && kasfShipsSelectMembersOfSelection(
           kasGrowSelectionPtr(139967715),
           kasGrowSelectionPtr(139967715),
           nearbyTargets))
      || kasfShipsSelectCapital(nearbyTargets, nearbyTargets)))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission05_FrigateCombatAttackRun,
            (sdword)Watch_Mission05_FrigateCombatAttackRun);
    }
    else if (kasTimerExpiredDestroyById(337061) || kasfShipsAnyIdle(&team->shipList))
    {
        kasTimerDestroyById(337061);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission05_FrigateReinforcementCombatLoop);
    }
}

void Begin_Mission05_FrigateCombatAttackRun(void)
{
    GrowSelection *nearbyTargets;
    GrowSelection *capitalTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    if (!kasfShipsSelectNearby(nearbyTargets, &((AITeam *)kasCurrentTeamPtr)->shipList, kasThisTeamsVectorPtr(), 6000))
    {
        if (!kasfEntryValueGet(20314791))
        {
            sub_496050();
            kasfEntrySetValue(20314791, 1);
        }
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission05_FrigateReinforcementCombatLoop);
        return;
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(nearbyTargets, kasGrowSelectionPtr(139967715));
    }
    kasfFormationWall();
    if (kasfShipsCount(nearbyTargets))
    {
        if (kasfEntryValueGet(20314791))
        {
            kasfEntrySetValue(20314791, 0);
            sub_496050();
        }
    }
    else if (!kasfEntryValueGet(20314791))
    {
        kasfEntrySetValue(20314791, 1);
        sub_496050();
    }

    capitalTargets = kasGrowSelectionPtr(124053532);
    if (!kasfShipsSelectCapital(capitalTargets, nearbyTargets))
    {
        capitalTargets = nearbyTargets;
    }
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_Mission05_FrigateCombatAttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        if (!kasfEntryValueGet(20314791))
        {
            sub_496050();
            kasfEntrySetValue(20314791, 1);
        }
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_Mission05_FrigateReinforcementCombatLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_Mission05_FrigateCombatAttackRun,
            (sdword)Watch_Mission05_FrigateCombatAttackRun);
    }
}

void Watch_Mission05_WorkerHarvestResourceLoop(void)
{
    AITeam *team;
    GrowSelection *shipBuffer;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfGetWorldResources())
    {
        if (kasfShipsAnyIdle(&team->shipList))
        {
            kasfSpeechEvent(208120516, (sdword)sub_617A10, 0);
        }
        return;
    }

    shipBuffer = kasGrowSelectionPtr(363040);
    kasfGrowSelectionClear(shipBuffer);
    kasfShipsAdd(shipBuffer, &team->shipList);
    kasfShipsRemove(&team->shipList, shipBuffer);
    kasfReinforceTeamWithShips(kasAITeamPtrIfExists(44665972), shipBuffer);
    kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
}

static void Begin_Mission05_Kamikaze2_BeastACVCombatAttackRun(void)
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
            (sdword)Begin_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop);
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

static void Watch_Mission05_Kamikaze2_BeastACVAttackRun(void)
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
                (sdword)Begin_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop,
                (sdword)Watch_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop);
            return;
        }
    }
    else if (!kasTimerExpiredById(74024315))
    {
        return;
    }

    kasfSpeechEvent(
        74024315,
        (sdword)Begin_Mission05_Kamikaze2_BeastACVCombatAttackRun,
        (sdword)Watch_Mission05_Kamikaze2_BeastACVAttackRun);
}

static void Begin_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop(void)
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
        kasfSpeechEvent(374244, (sdword)Begin_BeastACVCombatLoopDelay, (sdword)Watch_Mission05_Kamikaze2_Launch);
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

static void Watch_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop(void)
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
            (sdword)Begin_Mission05_Kamikaze2_BeastACVCombatAttackRun,
            (sdword)Watch_Mission05_Kamikaze2_BeastACVAttackRun);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop);
    }
}

void Watch_Mission05_Kamikaze2_Launch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(374244))
    {
        kasfSpeechEvent(
            88512316,
            (sdword)Begin_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop,
            (sdword)Watch_Mission05_Kamikaze2_BeastACVReinforcementCombatLoop);
    }
}

void Init_Mission05_FleetIntel_SetupState(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(144337492);
    kasfEntryGetOrCreateSetValue(266064926, 0);
    kasfEntryGetOrCreateSetValue(109086661, 0);
    kasfEntryGetOrCreateSetValue(211831571, 0);
    kasfEntryGetOrCreateSetValue(52577235, 0);
    kasfEntryGetOrCreateSetValue(172907219, 0);
    kasfEntryGetOrCreateSetValue(35614828, 0);
    kasfEntryGetOrCreateSetValue(168240451, 0);
    kasfEntryGetOrCreateSetValue(239734681, 0);
    kasfEntryGetOrCreateSetValue(127756738, 0);
    kasfEntryGetOrCreateSetValue(126773838, 1);
    kasfEntryGetOrCreateSetValue(77979796, 0);
    kasfTutEnableFlags("Flags");
    kasfTutEnableFlags("CancelFocus");
    kasfMsgSend((AITeam *)kasCurrentTeamPtr, (char *)5801588);
    kasfMusicPlay(4);
    kasfSpeechEvent(
        32713509,
        (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
        (sdword)Watch_Mission05_FleetIntel_EventFlow);
}

void Watch_Mission05_FleetIntel_CleanupAndResume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded() || kasfEntryValueGet(-2054310428))
    {
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
            kasfEntrySetValue(172907219, 0);
        }
        if (kasfEntryValueGet(35614828))
        {
            kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
            kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
            kasfCameraSetDistance(kasfEntryValueGet(64272901));
            kasfEntrySetValue(35614828, 0);
        }
        if (kasfEntryValueGet(239734681))
        {
            kasfShipsMakeCrazy(kasAITeamShipsPtrIfExists(93748889), 0);
            kasfVarDestroy(kasAITeamShipsPtrIfExists(93748889));
            kasfMsgSend(kasAITeamPtrIfExists(93748889), (char *)126523733);
            kasfSpeechEventQueueCurrentPlayer(268438226, 0);
            kasfEntrySetValue(239734681, 0);
            kasfEntrySetValue(127756738, 0);
            kasfSpeechEvent(
                244212684,
                (sdword)Init_WaitForIntelEventEnd,
                (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
        }
        else if (kasfEntryValueGet(77979796))
        {
            kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
        }
        else if (kasfEntryValueGet(127756738))
        {
            kasfSpeechEvent(
                244882162,
                (sdword)Init_Mission11_MissionFlow_Delay,
                (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
        }
        else
        {
            kasfSpeechEvent(
                32713509,
                (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
                (sdword)Watch_Mission05_FleetIntel_EventFlow);
        }
    }
}

void Watch_Mission05_FleetIntel_ResumeDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)0xD2247A4))
    {
        kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
    }
    else if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission05_FleetIntel_EventFlow);
    }
}

void Watch_Mission05_FleetIntel_EventFlow(void)
{
    AITeam *team;
    sdword queueIndex;
    char *message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;

    queueIndex = (team->msgQueue != NULL) ? team->msgQueue->head : -1;
    if (queueIndex < 0)
    {
        return;
    }

    while (queueIndex >= 0)
    {
        message = (char *)aitQueuePopPointerByIndex(team, (udword)queueIndex);
        switch ((udword)message)
        {
            case 374244:
                kasfSpeechEventQueueCurrentPlayer(268439856, 0);
                kasfMsgSend(team, (char *)232565183);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(127756738, 3);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(35614828, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 43324484:
                kasfSpeechEventQueueCurrentPlayer(268439958, 0);
                kasfEntrySetValue(77979796, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 55105476:
                kasfMsgSend(kasAITeamPtrIfExists(20572320), (char *)48772594);
                kasfMsgSend(kasAITeamPtrIfExists(20572321), (char *)48772594);
                kasfMsgSend(kasAITeamPtrIfExists(20572322), (char *)48772594);
                kasfMsgSend(team, (char *)76381282);
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 67032905:
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(75000);
                    kasfEntrySetValue(126773838, 0);
                }
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfSensorsSetLookatPoint((vector *)kasTeamsVectorPtr(93748889));
                kasfEntrySetValue(52577235, 1);
                kasfSpeechEventQueueCurrentPlayer(268443944, 0);
                kasfTutSetPointerTargetFirstShip("BentusiCityPointer", kasAITeamShipsPtrIfExists(93748889));
                kasfPingAddSingleShip(kasAITeamShipsPtrIfExists(93748889), "BentusiPing");
                kasfEntrySetValue(52577235, 1);
                kasfEntrySetValue(266064926, 1);
                kasfEntrySetValue(168240451, 1);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)0xB7F8E9);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 69366354:
                spSetSinglePlayerSaveInfo(5, ((char **)0x8C9768)[strCurLanguage]);
                break;
            case 76381282:
                kasfEntrySetValue(266064926, 1);
                kasfSpeechEventQueueCurrentPlayer(268444036, 0);
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(75000);
                    kasfEntrySetValue(126773838, 0);
                }
                kasfUniversePause();
                kasfOtherKASPause();
                kasfEntrySetValue(109086661, 1);
                kasfEntrySetValue(211831571, 1);
                kasfEntrySetValue(52577235, 1);
                kasfTutSetPointerTargetFirstShip("ThePointer", kasAITeamShipsPtrIfExists(20572321));
                kasfSensorsSetLookatPoint((vector *)kasTeamsVectorPtr(20572321));
                kasfEntrySetValue(168240451, 1);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 79033826:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfGrowSelectionClear(kasGrowSelectionPtr(0x58A20));
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(142188961));
                kasfCameraSetAngleDeg(342);
                kasfCameraSetDeclinationDeg(-24);
                kasfCameraSetDistance(975);
                kasfSpeechEventQueueCurrentPlayer(268566829, 0);
                kasfMsgSend(team, (char *)241932276);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 91170537:
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(93748889));
                kasfSpeechEventQueueCurrentPlayer(268439851, 0);
                kasfMsgSend(team, (char *)187439938);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 92825426:
                kasfEntrySetValue(-2054310428, 0);
                kasfSpeechEventQueueCurrentPlayer(268443943, 0);
                kasfMsgSend(team, (char *)67032905);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 101595527:
                kasfWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMsgSend(team, (char *)14915810);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 106615442:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(90274303));
                kasfCameraSetAngleDeg(62);
                kasfCameraSetDeclinationDeg(16);
                kasfCameraSetDistance(1200);
                kasfSpeechEventQueueCurrentPlayer(268439854, 0);
                kasfMsgSendAll((char *)227145550);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 112820146:
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(93748889));
                kasfSpeechEventQueueCurrentPlayer(268566833, 0);
                kasfEntrySetValue(239734681, 1);
                kasfMsgSend(team, (char *)126523733);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 137493174:
                kasfSpeechEventQueueCurrentPlayer(268444038, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)137493174);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 140989557:
                kasfMsgSend(team, (char *)92825426);
                spSetSinglePlayerSaveInfo(5, ((char **)0x8C975C)[strCurLanguage]);
                break;
            case 143331702:
                kasfMsgSend(team, (char *)112820146);
                kasfEntrySetValue(127756738, 4);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 14915810:
                kasfTutCameraFocusClose(kasAITeamShipsPtrIfExists(90274303));
                kasfSpeechEventQueueCurrentPlayer(268501299, 0);
                kasfMsgSend(team, (char *)4335396);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 189488980:
                kasfMsgSend(team, (char *)79033826);
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfMusicStop(3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 227145550:
                kasfSpeechEventQueueCurrentPlayer(268501295, 0);
                kasfMsgSend(team, (char *)374244);
                kasfMsgSendAll((char *)38095944);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 232565183:
                kasfSpeechEventQueueCurrentPlayer(268444037, 0);
                kasfEntrySetValue(127756738, 120);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)232565183);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 241148066:
                kasfEntryGetOrCreateSetValue(260833221, kasfCameraGetAngleDeg());
                kasfEntryGetOrCreateSetValue(60936190, kasfCameraGetDeclinationDeg());
                kasfEntryGetOrCreateSetValue(64272901, kasfCameraGetDistance());
                kasfPingRemove("BentusiPing");
                if (!kasfSensorsIsOpen(0))
                {
                    kasfOpenSensors(75000);
                    kasfEntrySetValue(126773838, 0);
                }
                kasfUniversePause();
                kasfOtherKASPause();
                kasfSpeechEventQueueCurrentPlayer(268443945, 0);
                kasfTutSetPointerTargetFirstShip("BeastAttackingPointer", kasAITeamShipsPtrIfExists(138522162));
                kasfSensorsSetLookatPoint((vector *)kasTeamsVectorPtr(138522162));
                kasfEntrySetValue(168240451, 1);
                kasfMsgSend(team, (char *)6793364);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 220350372:
                kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext);
                return;
            case 241932276:
                kasfShipsSetNonRetaliation(kasAITeamShipsPtrIfExists(93748889));
                kasfMsgSend(kasAITeamPtrIfExists(142188961), (char *)225618363);
                kasfMsgSend(team, (char *)143331702);
                kasfEntrySetValue(127756738, 2);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 258321701:
                kasfMsgSend(team, (char *)106615442);
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 266729988:
                kasfMusicPlay(23);
                kasfMsgSend(team, (char *)241148066);
                spEventWideScreenIn(90);
                kasfEntrySetValue(127756738, 3);
                kasfSpeechEvent(
                    244882162,
                    (sdword)Init_Mission11_MissionFlow_Delay,
                    (sdword)Watch_Mission05_FleetIntel_ResumeDelay);
                return;
            case 5801588:
                kasfLocationCard(4000, ((char **)0x8C975C)[strCurLanguage]);
                kasfMsgSend(team, (char *)140989557);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            case 4335396:
                kasfSpeechEventQueueCurrentPlayer(268439943, 0);
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
                kasfEntrySetValue(172907219, 1);
                kasfEntrySetValue(266064926, 1);
                kasfMsgSend(team, (char *)69366354);
                kasfSpeechEvent(
                    244212684,
                    (sdword)Init_WaitForIntelEventEnd,
                    (sdword)Watch_Mission05_FleetIntel_CleanupAndResume);
                return;
            default:
                aitQueuePointer(team, team, message);
                break;
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

void Watch_Mission05_FleetIntel_Objectives(void)
{
    AITeam *team;
    sdword queueIndex;
    void *message;
    GrowSelection *beastSelection;
    GrowSelection *engineeringSelection;
    Ship *engineeringModule;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;

    queueIndex = (team->msgQueue != NULL) ? team->msgQueue->head : -1;
    while (queueIndex >= 0)
    {
        message = aitQueuePopPointerByIndex(team, (udword)queueIndex);
        switch ((udword)message)
        {
            case 0:
                break;
            case 17444564:
                kasfObjectiveDestroy("SaveBentusi");
                break;
            case 36325177:
                kasfObjectiveCreate(
                    "SaveBentusi",
                    ((char **)0x8C978C)[strCurLanguage],
                    ((char **)0x8C9798)[strCurLanguage]);
                kasfObjectiveSet("SaveBentusi", 0);
                break;
            case 127795588:
                kasfObjectiveCreate(
                    "DestroyBeast",
                    ((char **)0x8C97BC)[strCurLanguage],
                    ((char **)0x8C97C8)[strCurLanguage]);
                kasfObjectiveSet("DestroyBeast", 0);
                break;
            case 137493174:
                kasfObjectiveCreate(
                    "BuildEngineering",
                    ((char **)0x8C97D4)[strCurLanguage],
                    ((char **)0x8C97E0)[strCurLanguage]);
                kasfObjectiveSet("BuildEngineering", 0);
                kasfEnableShip("sMothershipEngineering", "RACE_SECT", 1);
                kasfAllowPlayerToResearch("TECH_SUPPORT_SYSTEMS");
                break;
            case 175938357:
                kasfObjectiveDestroyAll();
                kasfObjectiveCreate(
                    "Hyperspace",
                    ((char **)0x8C97EC)[strCurLanguage],
                    ((char **)0x8C97F8)[strCurLanguage]);
                kasfObjectiveSet("Hyperspace", 0);
                break;
            case 232565183:
                kasfObjectiveCreate(
                    "ProtectCaalShto",
                    ((char **)0x8C97A4)[strCurLanguage],
                    ((char **)0x8C97B0)[strCurLanguage]);
                kasfObjectiveSet("ProtectCaalShto", 0);
                break;
            case 266729988:
                kasfObjectiveDestroy("InterceptBentusi");
                kasfTutRemovePointer("ObjPtr");
                break;
            case 0xB7F8E9:
                kasfObjectiveCreate(
                    "InterceptBentusi",
                    ((char **)0x8C9774)[strCurLanguage],
                    ((char **)0x8C9780)[strCurLanguage]);
                kasfObjectiveSet("InterceptBentusi", 0);
                kasfTutSetPointerTargetShipWithLine("ObjPtr", kasAITeamShipsPtrIfExists(93748889));
                kasfEntryGetOrCreateSetValue(290904, 1);
                break;
            default:
                aitQueuePointer(team, team, message);
                break;
        }

        if (team->msgQueue != NULL && team->msgQueue->head)
        {
            if (--queueIndex >= 0)
            {
                continue;
            }
        }
        break;
    }

    beastSelection = kasGrowSelectionPtr(93584908);
    kasfGrowSelectionClear(beastSelection);
    kasfShipsAdd(beastSelection, kasGrowSelectionPtr(114961833));
    if (kasfShipsSelectType(kasGrowSelectionPtr(363040), beastSelection, "bWorker"))
    {
        kasfShipsRemove(beastSelection, kasGrowSelectionPtr(363040));
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(363040), beastSelection, "bProcessor"))
    {
        kasfShipsRemove(beastSelection, kasGrowSelectionPtr(363040));
    }

    if (!kasfEntryValueGet(71552161) && !kasfShipsCount(beastSelection))
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfEntryGetOrCreateSetValue(-2024006844, 1);
        kasfObjectiveDestroy("DestroyBeast");
        kasfObjectiveDestroy("ProtectCaalShto");
    }

    if (!kasfEntryValueGet(71552162))
    {
        engineeringSelection = kasGrowSelectionPtr(86219269);
        if (kasfShipsSelectType(engineeringSelection, kasGrowSelectionPtr(0x4A2A29), "sMothershipEngineering"))
        {
            engineeringModule = engineeringSelection->selection->ShipPtr[0];
            if ((engineeringModule->specialFlags2 & 8) == 0)
            {
                kasfEntryGetOrCreateSetValue(71552162, 1);
                kasfEntryGetOrCreateSetValue(-2034155417, 1);
                kasfObjectiveDestroy("BuildEngineering");
                kasfAllowPlayerToResearch("TECH_MICRO_SHIP");
            }
        }
    }

    if (!kasfEntryValueGet(71552163) && kasfEntryValueGet(-2034155417) && kasfEntryValueGet(-2024006844))
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)0xC1CF74);
    }

    if (!kasfEntryValueGet(71552164) && !kasfShipsCount(kasGrowSelectionPtr(211966048)) && kasfEntryValueGet(290904))
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfEntryScopeName(290904);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
    }
}

void Init_Mission05_Traders_FirstContact(void)
{
    AITeam *team;
    sdword difficulty;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552165))
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfShipsEnableAttachNLips(&team->shipList);
        kasfShipsSetNonRetaliation(&team->shipList);
    }

    kasfSetTraderDialog(0, mission05TraderDialogBlank);
    kasfTeamSetMaxVelocity(0);
    difficulty = GetGameDifficulty();
    if (difficulty == 0)
    {
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    }
    else if (difficulty != 1)
    {
        kasfShipsSetPercentDamaged(&team->shipList, 80);
    }

    kasfSpeechEvent(
        170609268,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission05_Traders_FirstContact);
}

void Watch_Mission05_Traders_FirstContact(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyInSensorRange((GrowSelection *)(kasCurrentTeamPtr + 28)) || kasfMsgReceived((char *)22970740))
    {
        kasfTeamSwitchPlayerOwner(0);
        kasfMsgSendAll((char *)266729988);
        kasfSpeechEvent(
            4934180,
            (sdword)Init_Mission05_Traders_DefendYourSelf,
            (sdword)Watch_Mission05_Traders_DefendYourSelf);
    }
}

void Init_Mission05_Traders_DefendYourSelf(void)
{
    GrowSelection *attackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackers = kasGrowSelectionPtr(114364441);
    kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(115987881), kasThisTeamsVectorPtr(), 10000);
    kasfShipsRemove(attackers, kasGrowSelectionPtr(0x4A2A29));
    kasfShipsRemove(attackers, (GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfShipsRemove(attackers, kasAITeamShipsPtrIfExists(142188961));
    tutDeallocateRootRegion(attackers);
}

void Watch_Mission05_Traders_DefendYourSelf(void)
{
    GrowSelection *attackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28)))
    {
        attackers = kasGrowSelectionPtr(114364441);
        kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(115987881), kasThisTeamsVectorPtr(), 10000);
        kasfShipsRemove(attackers, kasGrowSelectionPtr(0x4A2A29));
        kasfShipsRemove(attackers, (GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsRemove(attackers, kasAITeamShipsPtrIfExists(142188961));
        tutDeallocateRootRegion(attackers);
    }

    if (kasfMsgReceived((char *)126523733))
    {
        kasfSpeechEvent(297300, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (!kasfEntryValueGet(71552166) && !kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
    }
}

void Init_Mission05_FleetIntel_FIDestroyResourcerTrigger(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsMakeCrazy((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    kasfShipsApplyPlayerUpgrades((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfSpeechEvent(
        320211,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission05_FleetIntel_FIDestroyResourcerTrigger);
}

void Watch_Mission05_FleetIntel_FIDestroyResourcerTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552168) && kasfShipsAnyInSensorRange((GrowSelection *)(kasCurrentTeamPtr + 28)))
    {
        kasfEntryGetOrCreateSetValue(71552168, 1);
        kasfMsgSend(kasAITeamPtrIfExists(93748889), (char *)22970740);
    }
    if (kasfMsgReceived((char *)266729988))
    {
        kasfSpeechEvent(
            66605385,
            (sdword)Init_Mission05_FleetIntel_FIDestroyResourcer,
            (sdword)Watch_Mission05_FleetIntel_FIDestroyResourcer);
    }
}

void Init_Mission05_FleetIntel_FIObjectivesComplete(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(
        244895509,
        (sdword)Watch_Mission05_FleetIntel_FIObjectivesComplete,
        (sdword)Watch_Mission05_FleetIntel_FIObjectivesComplete);
}

void Init_Mission05_FourShipReinforcements(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasAITeamShipsPtr(kasCurrentTeamPtr + 28, 1);
    kasfSpeechEvent(
        5597651,
        (sdword)Begin_Mission05_FourShipReinforcements,
        (sdword)Init_Tutorial1_HideUnhide);
}

void Begin_Mission05_FourShipReinforcements(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(90274303));
}

void Init_Mission05_BworkerAssaultTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists((char *)96759714)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-2070679070, 0);
        kasfEntryGetOrCreateSetValue(-2115886635, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)205770931);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(
                313692,
                (sdword)Watch_Mission05_BworkerCloakDamagedTargets,
                (sdword)Watch_Mission03_EnemyFrig01_AttackFighters);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_BworkerReinforcementTeam);
        }
    }
}

void Watch_Mission05_BworkerReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *dockTeamShips;
    GrowSelection *bworkers;
    GrowSelection *fsmShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    dockTeamShips = kasAITeamShipsPtrIfExists((char *)44665972);
    bworkers = kasGrowSelectionPtr((char *)363040);
    if (kasfShipsSelectType(bworkers, dockTeamShips, "bWorker") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr((char *)127797474)))
        {
            if (kasfEntryValueGet(-2070679070))
            {
                kasfEntrySetValue(-2070679070, 0);
                kasfEntrySetValue(-2115886635, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Watch_Mission05_BworkerDockAndReinforce);
            }
        }
        else
        {
            kasfEntrySetValue(-2070679070, 0);
            kasfEntrySetValue(-2115886635, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)205770931);
    kasfEntrySetValue(332667, 0);
    bworkers = kasGrowSelectionPtr((char *)363040);
    if (kasfShipsSelectType(bworkers, dockTeamShips, "bWorker"))
    {
        kasfShipsRemove(dockTeamShips, bworkers);
        kasfShipsStop(bworkers);
        fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
        kasfShipsAdd(fsmShips, bworkers);
        kasfShipsSetSlaveableFlag(bworkers, 0);
        kasfShipsApplyPlayerUpgrades(bworkers);
        kasfReinforceTeamWithShips(team, bworkers);
        kasfTacticsNeutral();
    }

    kasfSpeechEvent(
        313692,
        (sdword)Watch_Mission05_BworkerCloakDamagedTargets,
        (sdword)Watch_Mission03_EnemyFrig01_AttackFighters);
}

void Watch_Mission05_BworkerDockAndReinforce(void)
{
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(-2126662828))
    {
        switch (kasfEntryValueGet(-2126662828) - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 4)
                {
                    buildCost = kasfShipTypeBuildCost("bWorker") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if (kasfEntryValueGet(154639956) <= 1)
                {
                    return;
                }
                buildCost = kasfShipTypeBuildCost("bWorker") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    return;
                }
                break;
            case 1:
                if (kasfEntryValueGet(154639956) < 4)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) != 4)
                {
                    return;
                }
                break;
            default:
                return;
        }
    }
    else
    {
        if (kasfEntryValueGet(154639956) < 4)
        {
            buildCost = kasfShipTypeBuildCost("bWorker");
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if (kasfEntryValueGet(154639956) > 1)
        {
            buildCost = kasfShipTypeBuildCost("bWorker");
            if (kasfRUsGet(1) >= buildCost)
            {
                return;
            }
        }
        if (kasfEntryValueGet(154639956) != 4)
        {
            return;
        }
    }

    kasfSpeechEvent(
        75929221,
        (sdword)Init_Mission05_BworkerReinforcementDelay,
        (sdword)Watch_Mission05_BworkerReinforcementBuild);
}

void Init_Mission05_BworkerReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(235522949);
    if (kasfEntryValueGet(-2126662828))
    {
        switch (kasfEntryValueGet(-2126662828) - 1)
        {
            case 0:
                kasTimerCreateSetStartById(
                    235522949,
                    kasfShipTypeBuildTime("bWorker") * kasfEntryValueGet(154639956) / 2);
                break;
            case 1:
                kasTimerCreateSetStartById(235522949, 0);
                break;
        }
    }
    else
    {
        kasTimerCreateSetStartById(
            235522949,
            kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bWorker"));
    }
}

void Watch_Mission05_BworkerReinforcementBuild(void)
{
    AITeam *team;
    GrowSelection *dockTargets;
    GrowSelection *spawnSelection;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    dockTargets = kasGrowSelectionPtr((char *)127797474);
    if (!kasfShipsCount(dockTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(dockTargets) - 1);
        spawnSelection = kasGrowSelectionPtr((char *)21541);
        kasfShipsSelectIndex(spawnSelection, dockTargets, spawnIndex);
        kasfShipsCreateInside(
            kasGrowSelectionPtr((char *)154639956),
            "bWorker",
            kasfEntryValueGet(154639956),
            spawnSelection,
            1);
        kasAITeamShipsPtr(kasGrowSelectionPtr((char *)154639956), 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, kasGrowSelectionPtr((char *)154639956));
        kasfShipsAdd(kasGrowSelectionPtr((char *)team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)205770931);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            313692,
            (sdword)Watch_Mission05_BworkerCloakDamagedTargets,
            (sdword)Watch_Mission03_EnemyFrig01_AttackFighters);
    }
}

void Watch_Mission05_BworkerAcquireDamagedTargets(void)
{
    AITeam *team;
    GrowSelection *fsmShips;
    GrowSelection *damagedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
    damagedShips = kasGrowSelectionPtr((char *)140598228);
    if ((kasfShipsSelectCapital(damagedShips, fsmShips) && kasfShipsSelectDamaged(damagedShips, damagedShips, 70)) ||
        (kasfShipsSelectType(damagedShips, fsmShips, "bWorker") && kasfShipsSelectDamaged(damagedShips, damagedShips, 70)))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Watch_Mission05_BworkerCloakDamagedTargets,
            (sdword)Watch_Mission03_EnemyFrig01_AttackFighters);
    }
    else if (kasfShipsSelectDamaged(damagedShips, kasGrowSelectionPtr((char *)0x4A2A29), 45) &&
             kasfShipsSelectCapital(damagedShips, damagedShips) &&
             kasfShipsSelectCanSpecialTargetSelection(damagedShips, damagedShips, &team->shipList))
    {
        kasfSpeechEvent(
            123839973,
            (sdword)Watch_Mission05_BworkerSpecialAttackDamagedCapital,
            (sdword)Watch_Mission04_MissionBriefing_RUCrazy);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission05_BworkerAcquireDamagedTargets);
    }
}

void Watch_Mission05_BworkerCloakDamagedTargets(void)
{
    AITeam *team;
    GrowSelection *fsmShips;
    GrowSelection *damagedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    fsmShips = kasGrowSelectionPtr((char *)team->kasFSMNameHash);
    damagedShips = kasGrowSelectionPtr((char *)140598228);
    if ((kasfShipsSelectCapital(damagedShips, fsmShips) && kasfShipsSelectDamaged(damagedShips, damagedShips, 70)) ||
        (kasfShipsSelectType(damagedShips, fsmShips, "bWorker") && kasfShipsSelectDamaged(damagedShips, damagedShips, 70)))
    {
        kasfShipsSortByDistanceToPoint(damagedShips, (vector *)kasThisTeamsVectorPtr());
        kasfCloak(damagedShips);
    }
    else
    {
        kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission05_BworkerAcquireDamagedTargets);
    }
}

void Watch_Mission03_EnemyFrig01_AttackFighters(void)
{
    AITeam *team;
    GrowSelection *specialTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    specialTargets = kasGrowSelectionPtr((char *)140598228);
    if (!kasfShipsSelectSpecialTargets(specialTargets, &team->shipList) ||
        kasfShipsSelectHealthAtLeastPercent(specialTargets, specialTargets, 100))
    {
        kasfSpeechEvent(
            313692,
            (sdword)Watch_Mission05_BworkerCloakDamagedTargets,
            (sdword)Watch_Mission03_EnemyFrig01_AttackFighters);
    }
}

void Watch_Mission05_BworkerSpecialAttackDamagedCapital(void)
{
    AITeam *team;
    GrowSelection *damagedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    damagedShips = kasGrowSelectionPtr((char *)140598228);
    if (kasfShipsSelectDamaged(damagedShips, kasGrowSelectionPtr((char *)0x4A2A29), 50) &&
        kasfShipsSelectCapital(damagedShips, damagedShips) &&
        kasfShipsSelectCanSpecialTargetSelection(damagedShips, damagedShips, &team->shipList))
    {
        kasfShipsSortByDistanceToPoint(damagedShips, (vector *)kasThisTeamsVectorPtr());
        kasfAttackSpecial(damagedShips);
    }
    else
    {
        kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)Watch_Mission05_BworkerAcquireDamagedTargets);
    }
}

sdword Watch_Mission05_ResumeMissionFlowAfterIntel(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(144337492))
    {
        kasfSpeechEvent(
            32713509,
            (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag,
            (sdword)Watch_Mission05_MissionFlow);
        return TRUE;
    }
    return FALSE;
}

sdword Watch_Mission05_ObjectiveEventQueue(void)
{
    AITeam *team;
    sdword queueIndex;
    udword message;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    queueIndex = (team->msgQueue != NULL) ? team->msgQueue->head : -1;
    while (queueIndex >= 0)
    {
        message = (udword)aitQueuePopPointerByIndex(team, (udword)queueIndex);
        switch (message)
        {
            case 0:
                break;
            case 5735588:
                kasfObjectiveCreate("Scout", ((char **)0x8C9BE0)[strCurLanguage], ((char **)0x8C9BEC)[strCurLanguage]);
                break;
            case 51858660:
                kasfObjectiveDestroy("Mines");
                break;
            case 0x4FE0C:
                kasfObjectiveCreate("Destroy", ((char **)0x8C9C10)[strCurLanguage], ((char **)0x8C9C1C)[strCurLanguage]);
                break;
            case 0x51E2A3:
                kasfObjectiveCreate("Mines", ((char **)0x8C9C40)[strCurLanguage], ((char **)0x8C9C4C)[strCurLanguage]);
                break;
            case 126123060:
                kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)175938357);
                kasfObjectiveDestroy("Escort");
                kasfPingRemove("PingOnCaalShto");
                break;
            case 0x34DBEE4:
                kasfMissionFailed();
                break;
            case 0x4A78474:
                kasfObjectiveCreate("Escort", ((char **)0x8C9BF8)[strCurLanguage], ((char **)0x8C9C04)[strCurLanguage]);
                kasfPingAddShips(kasGrowSelectionPtr(90274303), "PingOnCaalShto");
                break;
            case 0x4FE1094:
                kasfObjectiveDestroy("Destroy");
                break;
            case 0x784A8C4:
                kasfObjectiveDestroy("Scout");
                break;
            case 0x8970534:
                kasfObjectiveDestroy("HvCruiser");
                break;
            case 0x8A89709:
                kasfObjectiveCreate("HvCruiser", ((char **)0x8C9BC8)[strCurLanguage], ((char **)0x8C9BD4)[strCurLanguage]);
                break;
            case 157488868:
                kasfObjectiveDestroy("Leech");
                break;
            case 175727384:
                kasfEntryGetOrCreateSetValue(-2122890664, 1);
                kasfObjectiveCreate("Leech", ((char **)0x8C9C58)[strCurLanguage], ((char **)0x8C9C64)[strCurLanguage]);
                break;
            case 0xA7C9B35:
                kasfMusicPlay(5);
                kasfObjectiveDestroyAll();
                kasfObjectiveCreate("Hyperspace", ((char **)0x8C9C70)[strCurLanguage], ((char **)0x8C9C7C)[strCurLanguage]);
                spSetSinglePlayerSaveInfo(6, ((char **)0x8C9BB0)[strCurLanguage]);
                break;
            case 0xAD51425:
                kasfObjectiveCreate("Explore", ((char **)0x8C9C28)[strCurLanguage], ((char **)0x8C9C34)[strCurLanguage]);
                kasfTutSetPointerTargetShipSelection("ObjPtr", kasAITeamShipsPtrIfExists(143034798));
                kasfPingAddShips(kasAITeamShipsPtrIfExists(143034798), "PingOnStation");
                break;
            case 0xD514234:
                kasfTutRemovePointer("ObjPtr");
                kasfEntryGetOrCreateSetValue(-2066520459, 1);
                kasfObjectiveDestroy("Explore");
                kasfPingRemove("PingOnStation");
                break;
            default:
                aitQueuePointer(team, team, (void *)message);
                break;
        }

        if (team->msgQueue != NULL && team->msgQueue->head)
        {
            if (--queueIndex >= 0)
            {
                continue;
            }
        }
        return queueIndex;
    }
    return queueIndex;
}

void Watch_Mission05_BworkerAssaultTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists((char *)96759714)) &&
        kasfEntryValueGet(-2115886635) &&
        kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2115886635, 0);
        kasfEntrySetValue(-2070679070, 1);
    }

    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists((char *)team->kasFSMNameHash), (char *)202253283);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_Mission05_BworkerReinforcementTeam);
    }
}

void Init_Mission05_BeastWorkerPool(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfSpeechEvent(
        74024315,
        (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
        (sdword)Init_ReinforcementPool_ClearSelectionAndReset);
}

void Watch_Mission05_BeastWorkerPoolCombat(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28)))
        {
            kasfSpeechEvent(
                74024315,
                (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
                (sdword)Init_ReinforcementPool_ClearSelectionAndReset);
        }
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Watch_Mission05_BeastWorkerPoolResume(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Init_ReinforcementPool_ClearSelectionAndReset,
            (sdword)Init_ReinforcementPool_ClearSelectionAndReset);
    }
}

static void Begin_Mission05_AssaultGroup_AttackNearbyCapital(void)
{
    GrowSelection *attackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    attackers = kasGrowSelectionPtr(57245931);
    kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasTeamsVectorPtr(93748889), 15000);
    kasfShipsSelectCapital(kasGrowSelectionPtr(363040), attackers);
    kasfShipsAdd(attackers, kasAITeamShipsPtrIfExists(93748889));
    tutDeallocateRootRegion(attackers);
}

void Init_Mission05_AssaultGroup_WaitForSignal(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsMakeCrazy((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    kasfShipsApplyPlayerUpgrades((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfSpeechEvent(
        320211,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission05_AssaultGroup_WaitForSignal);
}

void Watch_Mission05_AssaultGroup_WaitForSignal(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)266729988))
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(226191841)))
        {
            kasfSpeechEvent(
                157715175,
                (sdword)Init_Mission02_FleetIntel_FIDefendSalvageTeam2,
                (sdword)Watch_Mission02_FleetIntel_FIDefendSalvageTeam2);
        }
        else
        {
            kasfSpeechEvent(
                66605385,
                (sdword)Begin_Mission05_AssaultGroup_AttackNearbyCapital,
                (sdword)Watch_Mission05_AssaultGroup_AttackNearbyCapital);
        }
    }
}

void Watch_Mission05_AssaultGroup_AttackNearbyCapital(void)
{
    GrowSelection *attackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)17444564))
    {
        kasfVarDestroy((char *)(kasCurrentTeamPtr + 28));
        kasTimerDestroyById(139352562);
        kasfSpeechEvent(139967671, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28)))
    {
        attackers = kasGrowSelectionPtr(57245931);
        kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasTeamsVectorPtr(93748889), 15000);
        kasfShipsSelectCapital(kasGrowSelectionPtr(363040), attackers);
        kasfShipsAdd(attackers, kasAITeamShipsPtrIfExists(93748889));
        tutDeallocateRootRegion(attackers);
    }
}

void Init_Mission05_ReinforcementGroup_WaitForSignal(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsMakeCrazy((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    kasfShipsApplyPlayerUpgrades((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfSpeechEvent(
        157715175,
        (sdword)Watch_Mission05_ReinforcementGroup_WaitForSignal,
        (sdword)Watch_Mission05_ReinforcementGroup_AttackNearbyCapital);
}

void Watch_Mission05_ReinforcementGroup_WaitForSignal(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(138522164)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(226191842));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(138522165)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(226191843));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(138522166)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(226191841));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(138522167)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(226191844));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(138522168)))
    {
        kasfProtect(kasAITeamShipsPtrIfExists(226191845));
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(20572322)))
    {
        kasfHideShips((GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsSetNonRetaliation((GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfProtect(kasAITeamShipsPtrIfExists(10114145));
    }
}

void Watch_Mission05_ReinforcementGroup_AttackNearbyCapital(void)
{
    GrowSelection *targets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        if (kasfThisTeamIs(kasAITeamPtrIfExists(20572322)))
        {
            kasfMsgSend(kasAITeamPtrIfExists(182180066), (char *)4693508);
        }
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28)))
    {
        if (!kasfShipsSelectTargets(kasGrowSelectionPtr(342540), (GrowSelection *)(kasCurrentTeamPtr + 28)))
        {
            targets = kasGrowSelectionPtr(57245931);
            kasfGrowSelectionClear(targets);
            kasfShipsAdd(targets, kasGrowSelectionPtr(0x4A2A29));
            kasfShipsSortByDistanceToPoint(targets, kasThisTeamsVectorPtr());
            tutDeallocateRootRegion(targets);
        }
    }

    if (kasfMsgReceived((char *)17444564))
    {
        if (!kasfThisTeamIs(kasAITeamPtrIfExists(20572322)))
        {
            kasfVarDestroy((char *)(kasCurrentTeamPtr + 28));
            kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
            return;
        }
    }

    if (kasfMsgReceived((char *)48772594))
    {
        kasfUnhideShips((GrowSelection *)(kasCurrentTeamPtr + 28));
        kasfShipsSetRetaliation((GrowSelection *)(kasCurrentTeamPtr + 28));
    }

    if ((kasfThisTeamIs(kasAITeamPtrIfExists(138522164)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(226191842))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(138522165)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(226191843))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(138522166)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(226191841))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(138522167)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(226191844))) ||
        (kasfThisTeamIs(kasAITeamPtrIfExists(138522168)) && !kasfShipsCount(kasAITeamShipsPtrIfExists(226191845))))
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(93748889)))
        {
            tutDeallocateRootRegion(kasAITeamShipsPtrIfExists(93748889));
        }
        else
        {
            tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
        }
        return;
    }

    if (kasfThisTeamIs(kasAITeamPtrIfExists(93748889)))
    {
        tutDeallocateRootRegion(kasAITeamShipsPtrIfExists(93748889));
    }
    else
    {
        tutDeallocateRootRegion(kasGrowSelectionPtr(0x4A2A29));
    }
}

void Init_Mission05_FinalGroup_WaitForSignal(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsMakeCrazy((GrowSelection *)(kasCurrentTeamPtr + 28), 0);
    kasfSpeechEvent(
        129389189,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission05_FinalGroup_AttackNearbyCapital);
}

void Watch_Mission05_FinalGroup_AttackNearbyCapital(void)
{
    GrowSelection *targets;
    GrowSelection *capitalTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamCount())
    {
        targets = kasGrowSelectionPtr(86599289);
        if (kasfShipsSelectNearby(targets, kasGrowSelectionPtr(0x4A2A29), kasShipsVectorPtr((char *)0x2710), 10000))
        {
            capitalTargets = kasGrowSelectionPtr(57245931);
            if (kasfShipsSelectCapital(capitalTargets, targets))
            {
                tutDeallocateRootRegion(capitalTargets);
            }
            else
            {
                tutDeallocateRootRegion(targets);
            }
        }
        if (kasfMsgReceived((char *)227145550))
        {
            kasfSpeechEvent(
                139967671,
                (sdword)Begin_Mission13_MissionFlow_FILostAreaClear,
                (sdword)Watch_Mission13_MissionFlow_FILostAreaClear);
        }
    }
    else
    {
        kasfSpeechEvent(
            142812005,
            (sdword)Init_Mission13_MissionFlow_FILostReinforcementDelay,
            (sdword)Init_Mission02_ACT3Corvettes1_Launch);
    }
}

extern void Init_Tutorial1_Tutorial10_MoveContextMenu(void);
extern void Fill_Mission16_ReinforcementPoolFromParade(void);
void Init_Mission05(void)
{
    kasCurrentStep = 0;
    scope = KH_mission05;

    kasfDisableAllAIFeatures();
    kasfActivateTech("TECH_INFECT");
    kasfDisablePlayerHyperspace();

    kasFSMCreate(KH_FleetIntel, Init_Mission05_FleetIntel_SetupState, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(KH_FleetIntel));
    kasFSMCreate(KH_Objectives, Watch_NoOpTeamContext, Watch_Mission05_FleetIntel_Objectives,
                 kasAITeamPtrIfExists(KH_Objectives));

    kasFSMCreate(51250466, Init_Mission07_P2Mothership_Reappear, Watch_Mission07_P2Mothership_Reappear,
                 kasAITeamPtrIfExists(108436545));
    kasfShipsAdd((int *)kasGrowSelectionPtr(127797474), (int *)kasAITeamShipsPtrIfExists(108436545));

    kasFSMCreate(5103699, Init_Mission05_AssaultGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(226191841));
    kasFSMCreate(5103699, Init_Mission05_AssaultGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(226191842));
    kasFSMCreate(5103699, Init_Mission05_AssaultGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(226191843));
    kasFSMCreate(5103699, Init_Mission05_AssaultGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(226191844));

    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(113788721));
    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(126533841));
    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522161));
    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522162));
    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522163));
    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522169));
    kasFSMCreate(187368083, Init_Mission05_FleetIntel_FIDestroyResourcerTrigger, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(68871072));

    kasFSMCreate(161204435, Init_Mission05_ReinforcementGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522164));
    kasFSMCreate(161204435, Init_Mission05_ReinforcementGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522165));
    kasFSMCreate(161204435, Init_Mission05_ReinforcementGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522166));
    kasFSMCreate(161204435, Init_Mission05_ReinforcementGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522167));
    kasFSMCreate(161204435, Init_Mission05_ReinforcementGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(138522168));

    kasFSMCreate(142571938, Init_Tutorial1_Tutorial10_MoveContextMenu, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(20572321));
    kasFSMCreate(142571938, Init_Tutorial1_Tutorial10_MoveContextMenu, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists((sdword)&aisTeams[20].selection[1].sel.ShipPtr[129] + 1));
    kasFSMCreate(161204435, Init_Mission05_ReinforcementGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(20572322));

    kasFSMCreate(216033913, Init_Mission02_DefendSalvageTeam2Approach, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(142188961));

    kasFSMCreate(173181663, Init_Mission05_FinalGroup_WaitForSignal, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(226191845));

    kasFSMCreate(214404275, Init_Mission05_FourShipReinforcements, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(127951697));
    kasFSMCreate(214404275, Init_Mission05_FourShipReinforcements, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(127951698));
    kasFSMCreate(214404275, Init_Mission05_FourShipReinforcements, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(127951699));
    kasFSMCreate(214404275, Init_Mission05_FourShipReinforcements, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(168179969));

    kasFSMCreate(90274303, Init_Mission12_StrikeTeamDispatch_WaitForACT2, Watch_Mission12_StrikeTeamDispatch_WaitForACT2,
                 kasAITeamPtrIfExists(90274303));
    kasFSMCreate(90274303, Init_Mission12_StrikeTeamDispatch_WaitForACT2, Watch_Mission12_StrikeTeamDispatch_WaitForACT2,
                 kasAITeamPtrIfExists(111680593));
    kasFSMCreate(90274303, Init_Mission12_StrikeTeamDispatch_WaitForACT2, Watch_Mission12_StrikeTeamDispatch_WaitForACT2,
                 kasAITeamPtrIfExists(111680594));

    kasFSMCreate(111385145, Init_Mission05_Traders_FirstContact, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(93748889));
    kasFSMCreate(109687717, Init_Mission05_FleetIntel_FIObjectivesComplete, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(23816689));

    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeEnemyCombatShips((int *)&kasGrowSelectionPtr(114961833)->maxNumShips, (int)kasGrowSelectionPtr(18336307));
    kasfShipsSelectMissionVolumeInactivePlayerModules((int *)&kasGrowSelectionPtr(KH_Enemy)->maxNumShips, (int)kasGrowSelectionPtr(18336307));

    kasfRUsSet(1, 10000);

    kasFSMCreate(4627076, Init_Mission05_BeastWaveActivation, Watch_NoOpTeamContext,
                 kasAITeamPtrIfExists(4627076));

    kasFSMCreate(208120516, Init_Mission05_WorkerHarvestTeam, Watch_Mission05_WorkerHarvestTeam,
                 kasAITeamPtrIfExists(208120516));
    kasFSMCreate(208120516, Init_Mission05_WorkerHarvestTeam, Watch_Mission05_WorkerHarvestTeam,
                 kasAITeamPtrIfExists(108702865));

    kasFSMCreate(205978147, Init_Mission05_BworkerAssaultTeam, Watch_Mission05_BworkerAssaultTeam,
                 kasAITeamPtrIfExists(96759714));
    kasFSMCreate(205978147, Init_Mission05_BworkerAssaultTeam, Watch_Mission05_BworkerAssaultTeam,
                 kasAITeamPtrIfExists(205978209));

    kasFSMCreate(11133235, Init_Mission05_FrigateTeam, Watch_Mission05_FrigateTeam,
                 kasAITeamPtrIfExists(151690773));
    kasFSMCreate(11133235, Init_Mission05_FrigateTeam, Watch_Mission05_FrigateTeam,
                 kasAITeamPtrIfExists(11133425));

    kasFSMCreate(128635107, Init_Mission05_CorvetteTeam, Watch_Mission05_CorvetteTeam,
                 kasAITeamPtrIfExists(75148549));
    kasFSMCreate(128635107, Init_Mission05_CorvetteTeam, Watch_Mission05_CorvetteTeam,
                 kasAITeamPtrIfExists(128634913));
    kasFSMCreate(128635107, Init_Mission05_CorvetteTeam, Watch_Mission05_CorvetteTeam,
                 kasAITeamPtrIfExists(128634914));

    kasFSMCreate(230526675, Init_Mission05_FighterTeam, Watch_Mission05_FighterTeam,
                 kasAITeamPtrIfExists(182180066));
    kasFSMCreate(230526675, Init_Mission05_FighterTeam, Watch_Mission05_FighterTeam,
                 kasAITeamPtrIfExists(230526609));
    kasFSMCreate(230526675, Init_Mission05_FighterTeam, Watch_Mission05_FighterTeam,
                 kasAITeamPtrIfExists(230526610));

    kasFSMCreate(80024947, Init_Mission05_RammingFrigateTeam, Watch_Mission05_RammingFrigateTeam,
                 kasAITeamPtrIfExists(223105365));
    kasFSMCreate(80024947, Init_Mission05_RammingFrigateTeam, Watch_Mission05_RammingFrigateTeam,
                 kasAITeamPtrIfExists(80025009));

    kasFSMCreate(44665972, Init_Mission05_BeastWorkerPool, Fill_Mission16_ReinforcementPoolFromParade,
                 kasAITeamPtrIfExists(44665972));

    kasfAllowPlayerToResearch("TECH_DRIVE_FIGHTER");
    kasfAllowPlayerToResearch("TECH_MIMIC");
    kasfAllowPlayerToResearch("TECH_LINK");
    kasfAllowPlayerToResearch("TECH_REPAIR");
    kasfAllowPlayerToResearch("TECH_SALVAGE");
    kasfAllowPlayerToResearch("TECH_RESEARCH_CLEESAN_ATTACK");
    kasfAllowPlayerToResearch("TECH_AFTERBURNER");

    kasfEnableShip("sMothershipSpecial",      "RACE_SECT", 0);
    kasfEnableShip("sMothershipArmour",       "RACE_SECT", 0);
    kasfEnableShip("sMothershipMicro",        "RACE_SECT", 1);
    kasfEnableShip("sMothershipWeapons",      "RACE_SECT", 1);
    kasfEnableShip("sMothershipDockingBay",   "RACE_SECT", 1);
    kasfEnableShip("sMothershipEngineering",  "RACE_SECT", 0);

    kasfModuleSlotOrderSetShipType(0, "sRecon");
    kasfModuleSlotOrderSetShipType(0, "sAcolyte");
    kasfModuleSlotOrderSetShipType(0, "sSentinel");
    kasfModuleSlotOrderSetShipType(0, "sACV");
    kasfModuleSlotOrderSetShipType(0, "sWorker");
    kasfModuleSlotOrderSetShipType(0, "sRammingFrigate");
    kasfModuleSlotOrderSetCount(0, 4);
    kasfModuleSlotOrderSetShipType(1, "bAcolyte");
    kasfModuleSlotOrderSetShipType(1, "bHeavyCorvette");
    kasfModuleSlotOrderSetShipType(1, "bIonArrayFrigate");
    kasfModuleSlotOrderSetCount(1, 2);
    kasfModuleSlotOrderSetShipType(2, "Asteroids");

    kasTimerCreateSetStartById(50796994, 1);

    kasfShipsSelectType(kasGrowSelectionPtr(363040), kasGrowSelectionPtr(114961833), "bIonArrayFrigate");
    kasfShipsSetMaxVelocity(kasGrowSelectionPtr(363040), 225);
    kasfShipsSetMaxVelocity(kasAITeamShipsPtrIfExists(142188961), 175);
}
