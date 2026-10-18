/*=============================================================================
    Name    : mission01.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission01
=============================================================================*/

#include "MissionScriptSupport.h"
#include "Subtitle.h"

#undef CATACLYSM_DIAG
#define CATACLYSM_DIAG 1

// Retail mission01 tutorial flow uses opaque raw pointer tokens in this family.
// They are compared and re-queued as message IDs; they are never dereferenced.
#define M01_TUT_PESTER_TOKEN_BASE      ((char *)0x00A2BA30)
#define M01_TUT_PESTER_TOKEN_STAGE1    (M01_TUT_PESTER_TOKEN_BASE + 1)
#define M01_TUT_PESTER_TOKEN_STAGE2    (M01_TUT_PESTER_TOKEN_BASE + 2)
#define M01_TUT_PESTER_TOKEN_STAGE3    (M01_TUT_PESTER_TOKEN_BASE + 3)
#define M01_TUT_PESTER_TOKEN_STAGE4    (M01_TUT_PESTER_TOKEN_BASE + 4)

bool32 smShipVisibleToPlayer(Ship *ship);
void Init_Mission01_CarrierObjective_BeginTracking(void);
void Init_Mission01_CarrierObjective_ClearTrackedCarrier(void);
void Init_Mission01_CarrierObjective_UpdateNearbyAttackers(void);
void Init_Mission01_CarrierObjective_Start(void);
void Init_Mission01_ControllerPocketObjective_SelectPocketShips(void);
void Init_Mission01_ControllerPocketObjective_WaitForPocketReached(void);
void Init_Mission01_ControllerPocketObjective_FocusPocket(void);
void Init_Mission01_ControllerPocketObjective_WaitForSpeechDelay(void);
void Init_Mission01_ControllerPocketObjective_Start(void);
void Init_Mission01_ProximitySensorObjective_Start(void);
void Init_Mission01_ProximitySensorObjective_WaitForTrigger(void);
void Watch_Mission01_MissionFlow_NISing(void);
void Watch_Mission01_MissionFlow_CleanupFirstNISTarget(void);
void Watch_Mission01_MissionFlow_WaitForFirstNISTargetDestroyed(void);
void Watch_Mission01_MissionFlow_CleanupSecondNISTarget(void);
void Watch_Mission01_MissionFlow_WaitForSecondNISTargetDestroyed(void);
void Watch_Mission01_MissionFlow_SelectAsteroidPocketAttackers(void);
void Init_Mission01_AsteroidPocketAttackTeam_Start(void);
void Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers(void);
void Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers(void);
void Watch_Mission01_HeavyInterceptorBatchComplete(void);
void Watch_Mission01_AsteroidPocketAttackTeam_Intro(void);
void Init_Mission01_AsteroidPocketEncounter_Start(void);
void Init_Mission01_AsteroidPocketEncounter_FocusTarget(void);
void Init_Mission01_AsteroidPocketEncounter_FocusThreatWarning(void);
void Watch_Mission01_AsteroidPocketEncounter_AfterFocus(void);
void Watch_Mission01_AsteroidPocketEncounter_SelectAttackers(void);
void Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers(void);
void Watch_Mission01_AsteroidPocketEncounter_ThreatWarningAcknowledged(void);
void Watch_AsteroidPocketEncounter_BeginDefenseLoop(void);


void Init_Mission01_CarrierTeam_Activate(void);
void Watch_Mission01_MissionFlow_NISing(void);
void Watch_Mission01_MissionFlow_CleanupFirstNISTarget(void);
void Watch_Mission01_MissionFlow_WaitForFirstNISTargetDestroyed(void);
void Watch_Mission01_MissionFlow_CleanupSecondNISTarget(void);
void Watch_Mission01_MissionFlow_WaitForSecondNISTargetDestroyed(void);
void Watch_Mission01_MissionFlow_SelectAsteroidPocketAttackers(void);
void Init_Mission01_ApplyUpgradesThenDockForLaunch(void);
void Init_Mission01_DockCurrentTeamForLaunch(void);
void Watch_Mission01_MissionFlow_FirstWarning(void);
void Watch_Mission01_MissionFlow_FirstWarningDelay(void);
void Watch_Mission01_MissionFlow_AttackInClawFormation(void);
void Watch_Mission01_MissionFlow_SelectNearbyAttackersOrQueueReinforcements(void);
void Init_Mission01_QueueMultigunCorvetteOrHeavyInterceptorBatch(void);
void Watch_Mission01_MultigunCorvetteOrHeavyInterceptorBatchComplete(void);
void Watch_Mission01_MissionFlow_FleetCommandIntro(void);
void Init_Mission01_AsteroidPocketAttackTeam_Start(void);
void Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers(void);
void Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers(void);
void Init_Mission01_QueueHeavyInterceptorBatch(void);
void Watch_Mission01_HeavyInterceptorBatchComplete(void);
void Watch_Mission01_AsteroidPocketAttackTeam_Intro(void);
void Init_Mission01_AsteroidPocketEncounter_Start(void);
void Init_Mission01_AsteroidPocketEncounter_FocusTarget(void);
void Watch_Mission01_AsteroidPocketEncounter_AfterFocus(void);
void Watch_Mission01_AsteroidPocketEncounter_SelectAttackers(void);
void Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers(void);
void Init_Mission01_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor(void);
void Watch_Mission01_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady(void);
void Watch_Mission01_AsteroidPocketEncounter_ResumeAttackers(void);
void Init_Mission01_AsteroidPocketEncounter_FocusThreatWarning(void);
void Watch_Mission01_AsteroidPocketEncounter_ThreatWarningAcknowledged(void);
void Init_Mission01_ResearchShipTeam_Activate(void);
void Init_Mission01_ProximitySensorObjective_Start(void);
void Init_Mission01_ProximitySensorObjective_WaitForTrigger(void);
void Init_Mission01_GarrisonObjective_FocusTarget(void);
void Init_Mission01_GarrisonObjective_WaitForTrigger(void);
void Init_Mission01_GarrisonObjective_ClearTrackedSelection(void);
void Init_Mission01_GarrisonObjective_WaitForTrackedSelectionDestroyed(void);
void Init_Mission01_GarrisonObjective_Start(void);
void Init_Mission01_CarrierObjective_BeginTracking(void);
void Init_Mission01_CarrierObjective_ClearTrackedCarrier(void);
void Init_Mission01_CarrierObjective_UpdateNearbyAttackers(void);
void Init_Mission01_CarrierObjective_Start(void);
void Init_Mission01_ControllerPocketObjective_SelectPocketShips(void);
void Init_Mission01_ControllerPocketObjective_WaitForPocketReached(void);
void Init_Mission01_ControllerPocketObjective_FocusPocket(void);
void Init_Mission01_ControllerPocketObjective_WaitForSpeechDelay(void);
void Init_Mission01_ControllerPocketObjective_Start(void);
void Init_ApplyPlayerUpgradesToCurrentTeam(void);
void Init_Mission01_ApplyUpgradesThenProtectResourcers(void);
void Init_Mission01_HarvestController_MoveToAsteroidPocket(void);
void Init_Mission01_HarvestController_UpdateResourceFlow(void);
void Init_Mission01_HarvestController_SelectShips(void);
void Init_ApplyUpgradesThenOpenMainScreen(void);
void Watch_Mission01_OpenMainScreenLaunchOnSignal(void);

static const char *m01_destroyBombers_brief[] = { "Destroy Taiidani Bombers",         "French version goes here", "German version goes here" };
static const char *m01_destroyBombers_full[]  = { "The Taiidani ATTACK BOMBERS attacking our ION CANNON FRIGATES must be destroyed.  Build some ACOLYTES as soon as possible and send them over to help out the frigates.", "French version goes here", "German version goes here" };
static const char *m01_exploreRegion_brief[]  = { "Explore Designated Region",         "French version goes here", "German version goes here" };
static const char *m01_exploreRegion_full[]   = { "Our sensor grid has been knocked out in this area.  Send Recons in to explore and find out what enemy ships may be lurking there.", "French version goes here", "German version goes here" };
static const char *m01_destroyResourc_brief[] = { "Destroy Resourcing Convoy",          "French version goes here", "German version goes here" };
static const char *m01_destroyResourc_full[]  = { "Destroy all enemy resource collectors harvesting in the area.  Be on the look out for any ships guarding the resourcers.", "French version goes here", "German version goes here" };
static const char *m01_destroyDestroyer_brief[]= { "Destroy Crippled Heavy Cruiser",    "French version goes here", "German version goes here" };
static const char *m01_destroyDestroyer_full[] = { "This Taiidan Heavy Cruiser is fleeing and badly crippled.  Finish it off. Make sure to watch for any escort it might have.", "French version goes here", "German version goes here" };
static const char *m01_destroyProxAtt_brief[] = { "Defend proximity sensors",           "French version goes here", "German version goes here" };
static const char *m01_destroyProxAtt_full[]  = { "Proximity sensors in sector 110 are under attack.  They must not be destroyed!  Defend them immediately.", "French version goes here", "German version goes here" };
static const char *m01_hyperspace_brief[]     = { "Hyperspace",                         "French version goes here", "German version goes here" };
static const char *m01_hyperspace_full[]      = { "Rally the fleet.",                   "French version goes here", "German version goes here" };
static const char *m01_systemName[]           = { "Hiigara",                            "French version goes here", "German version goes here" };
static const char *m01_saveInfoTitle[]        = { "~ End of Mission",                   "French version goes here", "German version goes here" };

void Init_Mission01(void)
{
    AITeam *v0, *v1;

    kasCurrentStep = 0;
    kasCurrentScopeHash = KH_mission01;

    v0 = kasAITeamPtrIfExists(KH_FleetIntel);
    kasFSMCreate(KH_FleetIntel, Init_Mission01_SetupTutorialState, Watch_NoOpTeamContext, v0);

    v1 = kasAITeamPtrIfExists(KH_Objectives);
    kasFSMCreate(KH_Objectives, Init_Mission01_Objectives, Watch_Mission01_Objectives, v1);
}

void Init_Mission01_ProximitySensorObjective_WaitForTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)139644696))
    {
        kasfSpeechEvent(92553416, (sdword)Init_SelectAndFocusProximitySensors, (sdword)Watch_ProximitySensorContactLoop);
    }
}

void Watch_Mission01(void)
{
    sdword randomDelay;

    kasCurrentStep = 0;
    kasCurrentScopeHash = KH_mission01;
    if (kasTimerExpiredDestroyById(KH_M01_SCAN_TIMER))
    {
        kasfFindShipsInside(kasVolumePtr(KH_Everything), kasGrowSelectionPtr(KH_AllShips));
        kasSelectPlayerShips((int *)kasGrowSelectionPtr(KH_Enemy), (int)kasGrowSelectionPtr(KH_AllShips));
        kasSelectEnemyShips((int *)kasGrowSelectionPtr(KH_Friendly), (int)kasGrowSelectionPtr(KH_AllShips));
        randomDelay = kasfRandom(8, 15);
        kasTimerSetById(KH_M01_SCAN_TIMER, randomDelay);
        kasTimerStartById(KH_M01_SCAN_TIMER);
    }
    if (!kasfEntryValueGet(KH_M01_MS_FOUND))
    {
        if (kasfEntryValueGet(KH_M01_INIT_COMPLETE))
        {
            kasfEntryGetOrCreateSetValue(KH_M01_MS_FOUND, 1);
            kasfFindEnemyMothership((GrowSelection *)kasGrowSelectionPtr(KH_sMothership));
            kasfFindEnemyMothership((GrowSelection *)kasGrowSelectionPtr(KH_Mothership));
            kasfFindShipsInside(kasVolumePtr(KH_Everything), kasGrowSelectionPtr(KH_AllShips));
            kasSelectPlayerShips((int *)kasGrowSelectionPtr(KH_Enemy), (int)kasGrowSelectionPtr(KH_AllShips));
        }
    }
}

void Init_Mission01_Objectives(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(238436644, 0);
    kasfEntryGetOrCreateSetValue(84869460, 0);
    kasfEntryGetOrCreateSetValue(161120004, 0);
    kasfEntryGetOrCreateSetValue(190768900, 0);
    kasfEntryGetOrCreateSetValue(147303492, 0);
    kasfEntryGetOrCreateSetValue(223169844, 0);
    kasfEntryGetOrCreateSetValue(138385333, 0);
}

void Watch_Mission01_Objectives(void)
{
    GrowSelection *ships;

    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfEntryValueGet(223169844))
    {
        if (kasfEntryValueGet(138385333) == 1 && kasfEntryValueGet(238436644) == 1)
            kasfEntryIncrement(138385333);
        if (kasfEntryValueGet(138385333) == 2 && kasfEntryValueGet(84869460) == 1)
            kasfEntryIncrement(138385333);
        if (kasfEntryValueGet(138385333) == 3 && kasfEntryValueGet(161120004) == 1)
            kasfEntryIncrement(138385333);
        if (kasfEntryValueGet(138385333) == 4 && kasfEntryValueGet(190768900) == 1)
            kasfEntryIncrement(138385333);
        if (kasfEntryValueGet(138385333) == 5 && kasfEntryValueGet(147303492) == 1)
            kasfEntryIncrement(138385333);

        if (!kasfEntryValueGet(71552161) && kasfEntryValueGet(138385333) == 1)
        {
            kasfEntryGetOrCreateSetValue(71552161, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)5801588);
        }
        if (!kasfEntryValueGet(71552162) && kasfEntryValueGet(138385333) == 2)
        {
            kasfEntryGetOrCreateSetValue(71552162, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)154730542);
        }
        if (!kasfEntryValueGet(71552163) && kasfEntryValueGet(138385333) == 3)
        {
            kasfEntryGetOrCreateSetValue(71552163, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)81303907);
        }
        if (!kasfEntryValueGet(71552164) && kasfEntryValueGet(138385333) == 4)
        {
            kasfEntryGetOrCreateSetValue(71552164, 1);
            kasfMsgSend(kasAITeamPtrIfExists(187025617), (char *)139644696);
            kasfMsgSend(kasAITeamPtrIfExists(111204785), (char *)139644696);
            kasfMsgSend(kasAITeamPtrIfExists(247204801), (char *)139644696);
            kasfMsgSend(kasAITeamPtrIfExists(251613521), (char *)139644696);
            kasfMsgSend(kasAITeamPtrIfExists(251613522), (char *)139644696);
        }
        if (!kasfEntryValueGet(71552165) && kasfEntryValueGet(138385333) == 5)
        {
            kasfEntryGetOrCreateSetValue(71552165, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)261283218);
        }
        if (!kasfEntryValueGet(71552166) && kasfEntryValueGet(138385333) == 6)
        {
            kasfEntryGetOrCreateSetValue(71552166, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)96744581);
        }
        kasfEntrySetValue(223169844, 0);
        kasfTutRemovePointer("ObjPtr");
    }

    if (kasfMsgReceived((char *)0xDE8950A))
    {
        kasfObjectiveCreate("DestroyBombers", (char *)m01_destroyBombers_brief[strCurLanguage], (char *)m01_destroyBombers_full[strCurLanguage]);
        ships = kasAITeamShipsPtrIfExists(83684307);
        kasfTutSetPointerTargetShipSelection("ObjPtr", ships);
        goto lbl_obj_done;
    }
    if (kasfMsgReceived((char *)0x3B1A6A))
    {
        kasfObjectiveCreate("ExploreRegion", (char *)m01_exploreRegion_brief[strCurLanguage], (char *)m01_exploreRegion_full[strCurLanguage]);
        kasfTutSetPointerTargetAIVolumeWithLine("ObjPtr", kasVolumePtr(0xD56E434));
        goto lbl_obj_done;
    }
    if (kasfMsgReceived((char *)0x96DFBFA))
    {
        kasfObjectiveCreate("DestroyResourcers", (char *)m01_destroyResourc_brief[strCurLanguage], (char *)m01_destroyResourc_full[strCurLanguage]);
        ships = kasAITeamShipsPtrIfExists(213732803);
        kasfTutSetPointerTargetShipSelection("ObjPtr", ships);
        goto lbl_obj_done;
    }
    if (kasfMsgReceived((char *)0x3184CE6))
    {
        kasfAllowPlayerToResearch("TECH_LINK");
        goto lbl_obj_done;
    }
    if (kasfMsgReceived((char *)0xD99B1BA))
    {
        kasfObjectiveCreate("DestroyDestroyer", (char *)m01_destroyDestroyer_brief[strCurLanguage], (char *)m01_destroyDestroyer_full[strCurLanguage]);
        ships = kasAITeamShipsPtrIfExists(85516498);
        kasfTutSetPointerTargetShipSelection("ObjPtr", ships);
        goto lbl_obj_done;
    }
    if (kasfMsgReceived((char *)0xB0F775A))
    {
        kasfObjectiveCreate("DestroyProxAttackers", (char *)m01_destroyProxAtt_brief[strCurLanguage], (char *)m01_destroyProxAtt_full[strCurLanguage]);
        ships = kasGrowSelectionPtr(0xCB402F8);
        kasfTutSetPointerTargetShipSelection("ObjPtr", ships);
    }
    else if (kasfMsgReceived((char *)0xB315FFA))
    {
        kasfObjectiveDestroyAll();
        kasfObjectiveCreate("Hyperspace", (char *)m01_hyperspace_brief[strCurLanguage], (char *)m01_hyperspace_full[strCurLanguage]);
        spSetSinglePlayerSaveInfo(1, m01_saveInfoTitle[strCurLanguage]);
    }

lbl_obj_done:
    if (!kasfEntryValueGet(71552167) && kasfEntryValueGet(-2015429404) == 1)
    {
        kasfEntryGetOrCreateSetValue(71552167, 1);
        kasfEntrySetValue(138385333, 1);
        kasfEntrySetValue(223169844, 1);
    }
    if (!kasfEntryValueGet(71552168))
    {
        GrowSelection *bombers = kasAITeamShipsPtrIfExists(83684307);
        ships = kasAITeamShipsPtrIfExists(226001251);
#if !defined(HW_Release) && CATACLYSM_DIAG
        static sdword m01DiagLastBomberCount = -1;
        static sdword m01DiagLastCarrierCount = -1;
        static real32 m01DiagLastBomberLogTime = -1000.0f;
        sdword bomberCount = kasfShipsCount(bombers);
        sdword carrierCount = kasfShipsCount(ships);

        if (bomberCount != m01DiagLastBomberCount ||
            carrierCount != m01DiagLastCarrierCount ||
            universe.totaltimeelapsed - m01DiagLastBomberLogTime >= 10.0f)
        {
            dbgMessagef(
                "\n" CATACLYSM_DIAG_LABEL "mission01 m01demo counts bombers=%d trackedCarrier=%d total=%.2f",
                bomberCount,
                carrierCount,
                universe.totaltimeelapsed);
            m01DiagLastBomberCount = bomberCount;
            m01DiagLastCarrierCount = carrierCount;
            m01DiagLastBomberLogTime = universe.totaltimeelapsed;
        }
#endif
        if (!kasfShipsCount(bombers) || !kasfShipsCount(ships))
        {
#if !defined(HW_Release) && CATACLYSM_DIAG
            dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 m01demo objective complete");
#endif
            kasfEntryGetOrCreateSetValue(71552168, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)47658068);
            kasfEntrySetValue(238436644, 1);
            kasfObjectiveDestroy("DestroyBombers");
            kasfEntrySetValue(223169844, 1);
        }
    }
    if (!kasfEntryValueGet(71552169))
    {
        ships = kasAITeamShipsPtrIfExists(213732803);
        if (kasfShipsAnyInSensorRange(ships))
        {
            kasfEntryGetOrCreateSetValue(71552169, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)112538532);
            kasfEntrySetValue(84869460, 1);
            kasfObjectiveDestroy("ExploreRegion");
            kasfEntrySetValue(223169844, 1);
        }
    }
    if (!kasfEntryValueGet(71552170))
    {
        ships = kasAITeamShipsPtrIfExists(213732803);
        if (!kasfShipsCount(ships))
        {
            kasfEntryGetOrCreateSetValue(71552170, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)141350436);
            kasfEntrySetValue(161120004, 1);
            kasfObjectiveDestroy("DestroyResourcers");
            kasfEntrySetValue(223169844, 1);
        }
    }
    if (!kasfEntryValueGet(71552171))
    {
        ships = kasAITeamShipsPtrIfExists(85516498);
        if (!kasfShipsCount(ships))
        {
            kasfEntryGetOrCreateSetValue(71552171, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)227009076);
            kasfEntrySetValue(147303492, 1);
            kasfObjectiveDestroy("DestroyDestroyer");
            kasfEntrySetValue(223169844, 1);
        }
    }
    if (!kasfEntryValueGet(71552172))
    {
        if (!kasfShipsCount(kasAITeamShipsPtrIfExists(187025617)) &&
            !kasfShipsCount(kasAITeamShipsPtrIfExists(111204785)) &&
            !kasfShipsCount(kasAITeamShipsPtrIfExists(247204801)))
        {
            kasfEntryGetOrCreateSetValue(71552172, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)19885300);
            kasfEntrySetValue(190768900, 1);
            kasfObjectiveDestroy("DestroyProxAttackers");
            kasfEntrySetValue(223169844, 1);
        }
    }
    if (!kasfEntryValueGet(71552173))
    {
        ships = kasGrowSelectionPtr(0xCA25860);
        if (!kasfShipsCount(ships) && kasfEntryValueGet(-2015429404))
        {
            kasfEntryGetOrCreateSetValue(71552173, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)220350372);
        }
    }
}

void Init_Mission01_SetupTutorialState(void)
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
  kasfEntryGetOrCreateSetValue(168240451, 0);
  kasfEntryGetOrCreateSetValue(127756738, 0);
  kasfEntryGetOrCreateSetValue(126773838, 1);
  kasfTutEnableFlags("Focus");
  kasfTutEnableFlags("CancelFocus");
  kasfEntryGetOrCreateSetValue(77979796, 0);
  kasfSpeechEvent(48761345, (sdword)Init_Mission01_ActivateTutorialFlow, (sdword)Watch_Mission01_TutorialFlow); return;
}

void Watch_Mission01_TutorialFlow(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
  AITeam *focusThreatTeam;
  AITeam *initialWarningTeam;
#endif
  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  if ( kasfEntryValueGet(KH_INTEL_FLOW_READY) == 1 )
  {
    kasfDisablePlayerHyperspace();
    kasfMusicPlay(36);
    kasFSMCreate(202030339, Init_Mission01_CarrierTeam_Activate, Watch_NoOpTeamContext, kasAITeamPtrIfExists(202030339));
    kasFSMCreate(46982421, Init_Mission01_AsteroidPocketAttackTeam_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(178361137));
    kasFSMCreate(46982421, Init_Mission01_AsteroidPocketAttackTeam_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(214760435));
    kasFSMCreate(46982421, Init_Mission01_AsteroidPocketAttackTeam_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(114532931));
    kasFSMCreate(46982421, Init_Mission01_AsteroidPocketAttackTeam_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(252164835));
    kasFSMCreate(45804546, Init_ApplyPlayerUpgradesToCurrentTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(45804546));
    kasFSMCreate(171655445, Init_Mission01_AsteroidPocketEncounter_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(98363089));
    kasFSMCreate(171655445, Init_Mission01_AsteroidPocketEncounter_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(226420817));
    kasFSMCreate(171655445, Init_Mission01_AsteroidPocketEncounter_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(82404611));
    kasFSMCreate(85516498, Init_Mission01_ControllerPocketObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(85516498));
    kasFSMCreate(171655445, Init_Mission01_AsteroidPocketEncounter_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(87999618));
    kasFSMCreate(74615275, Init_Mission01_ProximitySensorObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(27860690));
    kasFSMCreate(226001251, Watch_Mission01_MissionFlow_NISing, Watch_NoOpTeamContext, kasAITeamPtrIfExists(226001251));
    kasFSMCreate(83684307, Init_Mission01_CarrierObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(83684307));
    kasFSMCreate(82227673, Init_Mission01_ResearchShipTeam_Activate, (KASWatchFunction)airAddedResearchShip, kasAITeamPtrIfExists(82227673));
    kasFSMCreate(15725844, Init_Mission01_GarrisonObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(251613521));
    kasFSMCreate(15725844, Init_Mission01_GarrisonObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(251613522));
    kasFSMCreate(74615275, Init_Mission01_ProximitySensorObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(187025617));
    kasFSMCreate(74615275, Init_Mission01_ProximitySensorObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(111204785));
    kasFSMCreate(74615275, Init_Mission01_ProximitySensorObjective_Start, Watch_NoOpTeamContext, kasAITeamPtrIfExists(247204801));
    kasFSMCreate(53764917, Init_Mission01_ApplyUpgradesThenProtectResourcers, Watch_NoOpTeamContext, kasAITeamPtrIfExists(53764917));
    kasFSMCreate(213732803, Init_Mission01_HarvestController_MoveToAsteroidPocket, Init_Mission01_HarvestController_UpdateResourceFlow, kasAITeamPtrIfExists(213732803));
    kasFSMCreate(69725861, Init_Mission01_HarvestController_SelectShips, Watch_NoOpTeamContext, kasAITeamPtrIfExists(69725861));
    kasFSMCreate(94867715, Init_Mission01_ApplyUpgradesThenDockForLaunch, Watch_NoOpTeamContext, kasAITeamPtrIfExists(213787713));
    kasFSMCreate(94867715, Init_Mission01_ApplyUpgradesThenDockForLaunch, Watch_NoOpTeamContext, kasAITeamPtrIfExists(213787714));
    kasFSMCreate(94867715, Init_Mission01_ApplyUpgradesThenDockForLaunch, Watch_NoOpTeamContext, kasAITeamPtrIfExists(204990129));
    kasFSMCreate(196001810, Init_ApplyPlayerUpgradesToCurrentTeam, Watch_NoOpTeamContext, kasAITeamPtrIfExists(196001810));
    kasFSMCreate(233597541, Init_ApplyUpgradesThenOpenMainScreen, Watch_NoOpTeamContext, kasAITeamPtrIfExists(247899825));
    kasFSMCreate(233597541, Init_ApplyUpgradesThenOpenMainScreen, Watch_NoOpTeamContext, kasAITeamPtrIfExists(247899826));
    kasFSMCreate(233597541, Init_ApplyUpgradesThenOpenMainScreen, Watch_NoOpTeamContext, kasAITeamPtrIfExists(247899827));
#if !defined(HW_Release) && CATACLYSM_DIAG
    focusThreatTeam = kasAITeamPtrIfExists(171477969);
    initialWarningTeam = kasAITeamPtrIfExists(60517377);
    dbgMessagef(
      "\n" CATACLYSM_DIAG_LABEL "mission01 tutorial threatTeam=%p initialWarningTeam=%p total=%.2f",
      focusThreatTeam,
      initialWarningTeam,
      universe.totaltimeelapsed);
#endif
    kasFSMCreate(88770722, Init_AsteroidPocketEncounter_FocusThreatWarning, Watch_NoOpTeamContext, kasAITeamPtrIfExists(171477969));
    kasFSMCreate(87668420, Init_AsteroidPocketEncounter_InitialAttackerWarning, Watch_NoOpTeamContext, kasAITeamPtrIfExists(60517377));
    kasfGrowSelectionClear(kasGrowSelectionPtr(213123832));
    kasfShipsAdd(kasGrowSelectionPtr(213123832), kasAITeamShipsPtrIfExists(187025617));
    kasfShipsAdd(kasGrowSelectionPtr(213123832), kasAITeamShipsPtrIfExists(111204785));
    kasfShipsAdd(kasGrowSelectionPtr(213123832), kasAITeamShipsPtrIfExists(247204801));
    kasfPlayerAcquiredTechnology("TECH_DRIVE_FIGHTER");
    kasfEnableShip("sMothershipSpecial", "Race_Sect", 0);
    kasfEnableShip("sMothershipArmour", "Race_Sect", 0);
    kasfEnableShip("sMothershipMicro", "Race_Sect", 0);
    kasfEnableShip("sMothershipWeapons", "Race_Sect", 0);
    kasfRUsSet(0, 0x7D0);
    kasfRUsSet(1, 0x186A0);
    kasTimerCreateSetStartById(50796994, 1);
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission01_TutorialEventFlow); return;
  }
  return;
}

void Watch_Mission01_TutorialPromptCleanup(void)
{
  sdword result;
  sdword intelEnded;
#if !defined(HW_Release) && CATACLYSM_DIAG
  static sdword m01DiagLastPromptCards = -1;
  static sdword m01DiagLastPromptEnded = -1;
  static sdword m01DiagLastPromptReason = -1;
  sdword promptCards;
#endif

  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  intelEnded = kasfIntelEventEnded();
  result = kasfEntryValueGet(-2054310428);
#if !defined(HW_Release) && CATACLYSM_DIAG
  promptCards = subRegion[STR_LetterboxBar].cardIndex;
  if (promptCards != m01DiagLastPromptCards || intelEnded != m01DiagLastPromptEnded || result != m01DiagLastPromptReason)
  {
    dbgMessagef(
      "\n" CATACLYSM_DIAG_LABEL "mission01 prompt cards=%d intelEnded=%d forceEnd=%d msgEnded=%d total=%.2f",
      promptCards,
      intelEnded,
      result,
      subMessageEnded,
      universe.totaltimeelapsed);
    m01DiagLastPromptCards = promptCards;
    m01DiagLastPromptEnded = intelEnded;
    m01DiagLastPromptReason = result;
  }
#endif
  if ( intelEnded || result != 0 )
  {
    kasfEntrySetValue(-2054310428, 0);
    if ( kasfEntryValueGet(168240451) )
    {
      kasfSubtitleDraw();
      kasfEntrySetValue(168240451, 0);
    }
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
      if ( !kasfEntryValueGet(126773838) )
      {
        kasfCloseSensors(1);
        kasfEntrySetValue(126773838, 1);
      }
      kasfEntrySetValue(52577235, 0);
    }
    if ( kasfEntryValueGet(172907219) )
    {
      kasfTutCameraFocusCancel();
      kasfEntrySetValue(172907219, 0);
    }
    if ( kasfEntryValueGet(232774915) )
    {
      kasfTutCameraFocusCancel();
      kasfTutCameraFocusCancel();
      kasfEntrySetValue(232774915, 0);
    }
    if ( kasfEntryValueGet(77979796) )
    {
      kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext); return;
    }
    kasfSpeechEvent(244882162, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_TutorialResumeDelay); return;
  }
  return;
}

void Watch_Mission01_TutorialResumeDelay(void)
{
  sdword result;
  sdword delay;

  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  if ( kasfMsgReceived(220350372) )
  {
    kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext); return;
  }
  if ( kasfEntryValueGet(127756738) )
  {
    delay = kasfEntryValueGet(127756738);
    kasTimerSetById(144337492, delay);
    kasTimerStartById(144337492);
    kasfEntrySetValue(127756738, 0);
  }
  result = kasTimerExpiredDestroyById(144337492);
  if ( result )
  {
    kasfSpeechEvent(32713509, (sdword)Init_Mission11_MissionFlow_ClearIntelAdvanceFlag, (sdword)Watch_Mission01_TutorialEventFlow); return;
  }
  return;
}

void Watch_Mission01_TutorialEventFlow(void)
{
  sdword teamContext;
  sdword *queueHead;
  sdword result;
  sdword queueIndex;
  AISTeamEntry *message;
  sdword angleDeg;
  sdword declinationDeg;
  sdword distance;
  sdword nextMessage;
  sdword followupMessage;
#if !defined(HW_Release) && CATACLYSM_DIAG
  static sdword m01DiagLastTutorialToken = -1;
#endif

  teamContext = kasCurrentTeamPtr;
  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  queueHead = *(sdword **)(kasCurrentTeamPtr + 80);
  result = -1;
  if ( queueHead )
    result = *queueHead;
  queueIndex = result;
  if ( result < 0 )
    return;
  while ( 1 )
  {
    message = (AISTeamEntry *)aitQueuePopPointerByIndex(teamContext, queueIndex);
#if !defined(HW_Release) && CATACLYSM_DIAG
    if ((udword)message >= (udword)M01_TUT_PESTER_TOKEN_BASE &&
        (udword)message <= (udword)M01_TUT_PESTER_TOKEN_STAGE4 &&
        (sdword)(udword)message != m01DiagLastTutorialToken)
    {
      dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission01 tutorial token message=0x%08X total=%.2f",
        (udword)message,
        universe.totaltimeelapsed);
      m01DiagLastTutorialToken = (sdword)(udword)message;
    }
#endif
    if ( (udword)message <= 0x5C43485 )
    {
      if ( message == (AISTeamEntry *)96744581 )
      {
        kasfMusicPlay(0);
        kasfSpeechEventQueueCurrentPlayer(33559242, 0);
        followupMessage = 187785210;
        goto LABEL_129;
      }
      if ( (udword)message <= 0x12F6CF4 )
      {
        if ( message == (AISTeamEntry *)19885300 )
        {
          kasfPingRemove("ProxPing");
          goto LABEL_79;
        }
        if ( message > (AISTeamEntry *)M01_TUT_PESTER_TOKEN_STAGE1 )
        {
          if ( message > (AISTeamEntry *)M01_TUT_PESTER_TOKEN_STAGE4 )
          {
            if ( message == (AISTeamEntry *)17855508 )
            {
              kasfEntrySetValue(109086661, 1);
              kasfEntrySetValue(211831571, 1);
              kasfEntrySetValue(52577235, 1);
              kasfEntrySetValue(266064926, 1);
              kasfEntrySetValue(168240451, 1);
              kasfEntrySetValue(127756738, 30);
              kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)51924198);
              kasfLocationCard(11, " ");
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
            }
          }
          else
          {
            if ( message == (AISTeamEntry *)M01_TUT_PESTER_TOKEN_STAGE4 )
            {
              kasfTutCameraFocusIgnoreZoom(kasGrowSelectionPtr(211966048));
              kasfCameraSetAngleDeg(kasfEntryValueGet(260833221));
              kasfCameraSetDeclinationDeg(kasfEntryValueGet(60936190));
              kasfCameraSetDistance(kasfEntryValueGet(64272901));
              goto LABEL_79;
            }
            if ( message == (AISTeamEntry *)M01_TUT_PESTER_TOKEN_STAGE2 )
            {
              kasfSpeechEventQueueCurrentPlayer(268443998, 0);
              kasfEntrySetValue(109086661, 1);
              kasfEntrySetValue(211831571, 1);
              followupMessage = (sdword)M01_TUT_PESTER_TOKEN_STAGE3;
              goto LABEL_128;
            }
            if ( message == (AISTeamEntry *)M01_TUT_PESTER_TOKEN_STAGE3 )
            {
              kasfEntrySetValue(52577235, 1);
              kasfEntrySetValue(266064926, 1);
#if !defined(HW_Release) && CATACLYSM_DIAG
              {
                AITeam *focusTeam = kasAITeamPtrIfExists(171477969);
                GrowSelection *focusShips = kasAITeamShipsPtrIfExists(171477969);
                Ship *focusShip =
                    (focusShips != NULL && focusShips->selection != NULL && focusShips->selection->numShips > 0)
                        ? focusShips->selection->ShipPtr[0]
                        : NULL;

                dbgMessagef(
                    "\n" CATACLYSM_DIAG_LABEL
                    "mission01 pester stage3 focusTeam=%p ships=%p num=%d firstType=%d firstPos=(%.0f, %.0f, %.0f) total=%.2f",
                    focusTeam,
                    focusShips,
                    (focusShips != NULL && focusShips->selection != NULL) ? focusShips->selection->numShips : -1,
                    focusShip != NULL ? focusShip->shiptype : -1,
                    focusShip != NULL ? focusShip->posinfo.position.x : 0.0f,
                    focusShip != NULL ? focusShip->posinfo.position.y : 0.0f,
                    focusShip != NULL ? focusShip->posinfo.position.z : 0.0f,
                    universe.totaltimeelapsed);
              }
#endif
              kasfTutCameraFocusIgnoreZoom(kasAITeamShipsPtrIfExists(171477969));
              kasfCameraSetAngleDeg(38);
              kasfCameraSetDeclinationDeg(-20);
              kasfCameraSetDistance(505);
              kasfLocationCard(8000, " ");
              kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, M01_TUT_PESTER_TOKEN_STAGE4);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
            }
          }
          goto LABEL_78;
        }
        if ( message == (AISTeamEntry *)M01_TUT_PESTER_TOKEN_STAGE1 )
        {
          angleDeg = kasfCameraGetAngleDeg();
          kasfEntryGetOrCreateSetValue(260833221, angleDeg);
          declinationDeg = kasfCameraGetDeclinationDeg();
          kasfEntryGetOrCreateSetValue(60936190, declinationDeg);
          distance = kasfCameraGetDistance();
          kasfEntryGetOrCreateSetValue(64272901, distance);
          spEventWideScreenIn(90);
          kasfEntrySetValue(127756738, 3);
          kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)2772294);
          kasfSpeechEvent(244882162, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_TutorialResumeDelay); return;
        }
        if ( (udword)message <= 0x588674 )
        {
          if ( message == (AISTeamEntry *)0x588674 )
          {
            kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)92825426);
            spSetSinglePlayerSaveInfo(1, m01_systemName[strCurLanguage]);
            goto LABEL_79;
          }
          if ( !message )
            goto LABEL_79;
          if ( message == (AISTeamEntry *)2772294 )
          {
            kasfEntrySetValue(168240451, 1);
            if ( !kasfSensorsIsOpen(0) )
            {
              kasfOpenSensors(100000);
              kasfEntrySetValue(126773838, 0);
            }
            kasfUniversePause();
            kasfOtherKASPause();
            kasfSpeechEventQueueCurrentPlayer(268443997, 0);
#if !defined(HW_Release) && CATACLYSM_DIAG
            {
              AITeam *focusTeam = kasAITeamPtrIfExists(171477969);
              GrowSelection *focusShips = kasAITeamShipsPtrIfExists(171477969);
              Ship *focusShip =
                  (focusShips != NULL && focusShips->selection != NULL && focusShips->selection->numShips > 0)
                      ? focusShips->selection->ShipPtr[0]
                      : NULL;

              dbgMessagef(
                  "\n" CATACLYSM_DIAG_LABEL
                  "mission01 pester stage1 focusTeam=%p ships=%p num=%d firstType=%d firstPos=(%.0f, %.0f, %.0f) total=%.2f",
                  focusTeam,
                  focusShips,
                  (focusShips != NULL && focusShips->selection != NULL) ? focusShips->selection->numShips : -1,
                  focusShip != NULL ? focusShip->shiptype : -1,
                  focusShip != NULL ? focusShip->posinfo.position.x : 0.0f,
                  focusShip != NULL ? focusShip->posinfo.position.y : 0.0f,
                  focusShip != NULL ? focusShip->posinfo.position.z : 0.0f,
                  universe.totaltimeelapsed);
            }
#endif
            soundStopDSound("PesterPointer", (sdword)kasAITeamShipsPtrIfExists(171477969));
            kasfSensorsSetLookatPoint(kasTeamsVectorPtr(171477969));
            if ( GetGameDifficulty() >= 2 )
            {
              kasfMsgSend(kasAITeamPtrIfExists(176850956), M01_TUT_PESTER_TOKEN_STAGE3);
              kasfEntrySetValue(109086661, 1);
              kasfEntrySetValue(211831571, 1);
            }
            else
            {
              kasfMsgSend(kasAITeamPtrIfExists(176850956), M01_TUT_PESTER_TOKEN_STAGE2);
            }
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
          }
          goto LABEL_78;
        }
        if ( message != (AISTeamEntry *)M01_TUT_PESTER_TOKEN_BASE )
          goto LABEL_78;
        nextMessage = 213247342;
        kasfMsgSend(kasAITeamPtrIfExists(171477969), (char *)nextMessage);
        goto LABEL_79;
      }
      if ( (udword)message > 0x3900262 )
      {
        if ( message == (AISTeamEntry *)59769443 )
        {
          kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)3873386);
          kasfSpeechEventQueueCurrentPlayer(268468443, 0);
          if ( !kasfSensorsIsOpen(0) )
          {
            kasfOpenSensors(100000);
            kasfEntrySetValue(126773838, 0);
          }
          kasfSensorsSetLookatPoint(kasVolumeVectorPtr(223798324));
          kasfTutSetPointerTargetAIVolumeNoLine("UnexploredPoint", kasVolumePtr(93190660));
          kasfPingAddPoint(kasVolumeVectorPtr(223798324), "UnexploredPing");
          kasfUniversePause();
          kasfOtherKASPause();
          kasfEntrySetValue(109086661, 1);
          kasfEntrySetValue(211831571, 1);
          kasfEntrySetValue(52577235, 1);
          kasfEntrySetValue(266064926, 1);
          kasfEntrySetValue(168240451, 1);
          kasfEntrySetValue(127756738, 3);
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
        }
        if ( message == (AISTeamEntry *)81303907 )
        {
          spEventWideScreenIn(90);
          kasfEntrySetValue(127756738, 3);
          nextMessage = 227120642;
          goto LABEL_131;
        }
        if ( message == (AISTeamEntry *)92825426 )
        {
          kasfEntrySetValue(-2054310428, 0);
          kasfMusicPlay(36);
          kasfSpeechEventQueueCurrentPlayer(268443996, 0);
          followupMessage = 263613923;
          goto LABEL_128;
        }
        goto LABEL_78;
      }
      if ( message != (AISTeamEntry *)59769442 )
      {
        if ( (udword)message > 0x3184CE6 )
        {
          if ( message == (AISTeamEntry *)59769441 )
          {
            spEventWideScreenIn(90);
            kasfEntrySetValue(127756738, 3);
            nextMessage = 59769442;
            goto LABEL_131;
          }
          goto LABEL_78;
        }
        if ( message == (AISTeamEntry *)51924198 )
        {
          kasfSpeechEventQueueCurrentPlayer(268444004, 0);
          kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)51924198);
          goto LABEL_79;
        }
        if ( message != (AISTeamEntry *)27065970 )
        {
          if ( message == (AISTeamEntry *)47658068 )
          {
            kasfMusicPlay(0);
            kasfPingRemove("BomberPing");
            goto LABEL_79;
          }
          goto LABEL_78;
        }
        kasfSpeechEventQueueCurrentPlayer(268444005, 0);
        if ( !kasfSensorsIsOpen(0) )
        {
          kasfOpenSensors(100000);
          kasfEntrySetValue(126773838, 0);
        }
        kasfUniversePause();
        kasfOtherKASPause();
        kasfEntrySetValue(109086661, 1);
        kasfEntrySetValue(211831571, 1);
        kasfSensorsSetLookatPoint(kasShipsVectorPtrIfExists(213123832));
        soundStopDSound("ProxPointer", (sdword)kasGrowSelectionPtr(213123832));
        kasfPingAddShips(kasGrowSelectionPtr(213123832), "ProxPing");
        kasfEntrySetValue(52577235, 1);
        kasfEntrySetValue(168240451, 1);
        kasfEntrySetValue(266064926, 1);
        followupMessage = 185562970;
LABEL_129:
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)followupMessage);
        kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
      }
      if ( kasfShipsCount((sdword)kasAITeamShipsPtrIfExists(226001251)) )
        kasfSpeechEventQueueCurrentPlayer(268444000, 0);
      else
        kasfSpeechEventQueueCurrentPlayer(268444001, 0);
      followupMessage = 59769443;
LABEL_128:
      kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)followupMessage);
      goto LABEL_129;
    }
    if ( (udword)message <= 0xB66DEB5 )
    {
      if ( message != (AISTeamEntry *)191291061 )
      {
        if ( (udword)message > 0x92DD9C3 )
        {
          if ( (udword)message > 0xB66DEB3 )
          {
            kasfSpeechEventQueueCurrentPlayer(268439768, 0);
            followupMessage = 191291061;
            goto LABEL_128;
          }
          if ( message == (AISTeamEntry *)191291059 )
          {
            kasfSpeechEventQueueCurrentPlayer(268468439, 0);
            followupMessage = 191291060;
            goto LABEL_128;
          }
          if ( message == (AISTeamEntry *)154730542 )
          {
            if ( GetGameDifficulty() >= 2 )
              nextMessage = 59769441;
            else
              nextMessage = 161890388;
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)nextMessage);
            goto LABEL_79;
          }
          if ( message == (AISTeamEntry *)161890388 )
          {
            kasfSpeechEventQueueCurrentPlayer(268443999, 0);
            followupMessage = 59769441;
            goto LABEL_128;
          }
        }
        else
        {
          if ( message == (AISTeamEntry *)153999811 )
          {
            kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)228176314);
            kasfEntrySetValue(52577235, 1);
            kasfEntrySetValue(266064926, 1);
            kasfEntrySetValue(127756738, 3);
            kasfSpeechEventQueueCurrentPlayer(268443870, 0);
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
          }
          if ( (udword)message > 0x9055DE2 )
          {
            if ( message == (AISTeamEntry *)153999810 )
            {
              if ( !kasfSensorsIsOpen(0) )
              {
                kasfOpenSensors(100000);
                kasfEntrySetValue(126773838, 0);
              }
              kasfSensorsSetLookatPoint(kasTeamsVectorPtr(85516498));
              kasfPingAddShips(kasAITeamShipsPtrIfExists(85516498), "DestroyerPing");
              soundStopDSound("DestroyerPoint", (sdword)kasAITeamShipsPtrIfExists(85516498));
              kasfSpeechEventQueueCurrentPlayer(268468445, 0);
              kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)153999811);
              kasfMsgSend(kasAITeamPtrIfExists(85516498), (char *)307349);
              kasfMsgSend(kasAITeamPtrIfExists(87999618), (char *)84320888);
              kasfEntrySetValue(168240451, 1);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
            }
          }
          else
          {
            if ( message == (AISTeamEntry *)151346658 )
            {
              kasfSensorsSetLookatPoint(kasTeamsVectorPtr(45804546));
              soundStopDSound("ResourcerPoint", (sdword)kasAITeamShipsPtrIfExists(45804546));
              kasfSpeechEventQueueCurrentPlayer(268444003, 0);
              kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)17855508);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
            }
            if ( message == (AISTeamEntry *)112538532 )
            {
              kasfPingRemove("UnexploredPing");
              goto LABEL_79;
            }
            if ( message == (AISTeamEntry *)141350436 )
            {
              kasfPingRemove("ResourcerPing");
              kasfSpeechEventQueueCurrentPlayer(268439906, 0);
              kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
            }
          }
        }
        goto LABEL_78;
      }
      if ( !kasfSensorsIsOpen(0) )
      {
        kasfOpenSensors(100000);
        kasfEntrySetValue(126773838, 0);
      }
      kasfUniversePause();
      kasfOtherKASPause();
      soundStopDSound("BomberPointer", (sdword)kasAITeamShipsPtrIfExists(83684307));
      kasfSensorsSetLookatPoint(kasTeamsVectorPtr(83684307));
      kasfPingAddShips(kasAITeamShipsPtrIfExists(83684307), "BomberPing");
      kasfSpeechEventQueueCurrentPlayer(268468441, 0);
#if !defined(HW_Release) && CATACLYSM_DIAG
      dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 bomber tutorial pause");
#endif
      followupMessage = 191291062;
      goto LABEL_128;
    }
    if ( (udword)message > 0xF19CFE7 )
      break;
    if ( message != (AISTeamEntry *)253349863 )
    {
      if ( (udword)message > 0xD87E234 )
      {
        if ( message == (AISTeamEntry *)227120642 )
        {
          kasfSpeechEventQueueCurrentPlayer(268443868, 0);
          kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)158202874);
          if ( !kasfSensorsIsOpen(0) )
          {
            kasfOpenSensors(100000);
            kasfEntrySetValue(126773838, 0);
          }
          kasfUniversePause();
          kasfOtherKASPause();
          kasfSensorsSetLookatPoint(kasTeamsVectorPtr(213732803));
          soundStopDSound("ResourcerPoint", (sdword)kasAITeamShipsPtrIfExists(213732803));
          kasfPingAddShips(kasAITeamShipsPtrIfExists(213732803), "ResourcerPing");
          if ( GetGameDifficulty() >= 2 )
            kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)17855508);
          else
            kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)151346658);
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
        }
      }
      else
      {
        if ( message == (AISTeamEntry *)227009076 )
        {
          kasfSpeechEventQueueCurrentPlayer(268444007, 0);
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
        }
        if ( message == (AISTeamEntry *)191291062 )
        {
          kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)233346314);
          kasfEntrySetValue(168240451, 1);
          kasfSpeechEventQueueCurrentPlayer(268443866, 0);
          kasfEntrySetValue(109086661, 1);
          kasfEntrySetValue(211831571, 1);
          kasfEntrySetValue(266064926, 1);
          kasfEntrySetValue(52577235, 1);
          kasfEntrySetValue(127756738, 50);
          kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, M01_TUT_PESTER_TOKEN_BASE);
          kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
        }
        if ( message == (AISTeamEntry *)220350372 )
        {
          kasfSpeechEvent(304604, (sdword)FailMissionIfTrackedSelectionAlive, (sdword)MaintainTeamContext); return;
        }
      }
      goto LABEL_78;
    }
    if ( !kasfEntryValueGet(71552182) )
    {
      kasfEntryGetOrCreateSetValue(71552182, 1);
      kasfMusicPlay(19);
      spEventWideScreenIn(90);
      kasfEntrySetValue(127756738, 3);
      kasfMsgSend((struct AITeam *)kasCurrentTeamPtr, (char *)27065970);
      kasfSpeechEvent(244882162, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_TutorialResumeDelay); return;
    }
LABEL_79:
    teamContext = kasCurrentTeamPtr;
    result = *(sdword *)(kasCurrentTeamPtr + 80);
    if ( result )
    {
      if ( *(sdword *)result )
      {
        if ( --queueIndex >= 0 )
          continue;
      }
    }
    return;
  }
  if ( message != (AISTeamEntry *)261283218 )
  {
    if ( message == (AISTeamEntry *)263613923 )
    {
      kasfSpeechEventQueueCurrentPlayer(268439766, 0);
      followupMessage = 191291059;
      goto LABEL_128;
    }
    if ( message == (AISTeamEntry *)265908324 )
    {
      kasfSpeechEventQueueCurrentPlayer(268444006, 0);
      kasfEntrySetValue(77979796, 1);
      kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)Watch_Mission01_TutorialPromptCleanup); return;
    }
LABEL_78:
    aitQueuePointer(kasCurrentTeamPtr, kasCurrentTeamPtr, (sdword)message);
    goto LABEL_79;
  }
  spEventWideScreenIn(90);
  kasfEntrySetValue(127756738, 3);
  nextMessage = 153999810;
LABEL_131:
  kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)nextMessage);
  kasfSpeechEvent(244882162, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_TutorialResumeDelay); return;
}

void Init_Mission01_ActivateTutorialFlow(void)
{
  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  kasfEntryGetOrCreateSetValue(KH_INTEL_FLOW_ACTIVE, 1);
}

void Init_Mission01_CarrierTeam_Activate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552177))
    {
        kasfEntryGetOrCreateSetValue(71552177, 1);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfTeamSwitchPlayerOwner(FALSE);
        kasfShipsEnableAttachNLips(&team->shipList);
    }
}

void Watch_Mission01_MissionFlow_NISing(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552174))
    {
        kasfEntryGetOrCreateSetValue(71552174, 1);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfTeamSwitchPlayerOwner(FALSE);
        kasfShipsEnableAttachNLips(&team->shipList);
    }
    kasfTeamSetPercentDamaged(80);
    kasfShipsSetDamageFactor(&team->shipList, 50);
    kasfSpeechEvent(
        104050139,
        (sdword)Watch_Mission01_MissionFlow_CleanupFirstNISTarget,
        (sdword)Watch_Mission01_MissionFlow_WaitForFirstNISTargetDestroyed);
}

void Watch_Mission01_MissionFlow_CleanupFirstNISTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfCurrentTeamAttack(kasAITeamShipsPtrIfExists(83684307));
}

void Watch_Mission01_MissionFlow_WaitForFirstNISTargetDestroyed(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfMsgSendAll((char *)54112162);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(83684307)))
    {
        kasfSpeechEvent(
            33503762,
            (sdword)Watch_Mission01_MissionFlow_CleanupSecondNISTarget,
            (sdword)Watch_Mission01_MissionFlow_WaitForSecondNISTargetDestroyed);
    }
}

void Watch_Mission01_MissionFlow_CleanupSecondNISTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfCurrentTeamAttack(kasAITeamShipsPtrIfExists(196001810));
}

void Watch_Mission01_MissionFlow_WaitForSecondNISTargetDestroyed(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 m01DiagLastSecondTargetLogTime = -1000.0f;
#endif
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
#if !defined(HW_Release) && CATACLYSM_DIAG
    if (universe.totaltimeelapsed - m01DiagLastSecondTargetLogTime >= 10.0f)
    {
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 second target count=%d total=%.2f",
            kasfShipsCount(kasAITeamShipsPtrIfExists(196001810)),
            universe.totaltimeelapsed);
        m01DiagLastSecondTargetLogTime = universe.totaltimeelapsed;
    }
#endif
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(196001810)))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 sending initial attacker warning");
#endif
        kasfMsgSend(kasAITeamPtrIfExists(60517377), (char *)237703397);
        kasfSpeechEvent(
            57056661,
            (sdword)Watch_Mission01_MissionFlow_SelectAsteroidPocketAttackers,
            (sdword)MaintainTeamContext);
    }
}

void Watch_Mission01_MissionFlow_SelectAsteroidPocketAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsSetDamageFactor(&((AITeam *)kasCurrentTeamPtr)->shipList, 10);
    attackersInPocket = kasGrowSelectionPtr(22816);
    candidateAttackers = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(attackersInPocket);
    kasfGrowSelectionClear(candidateAttackers);
    kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
    kasfShipsSelectMissionVolumeEnemyCombatShips(
        (int *)&candidateAttackers->maxNumShips,
        (int)attackersInPocket);
    kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(85516498));
    kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(87999618));
    kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
}

void Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers(void)
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
    kasfShipsSelectMissionVolumeEnemyCombatShips(
        (int *)&candidateAttackers->maxNumShips,
        (int)attackersInPocket);
    kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(85516498));
    kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(87999618));
    kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
    kasTimerCreateSetStartById(139352562, kasfRandom(1, 7));
}

void Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 m01DiagLastPocketAttackUpdateLogTime = -1000.0f;
#endif

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
            kasfShipsSelectMissionVolumeEnemyCombatShips(
                (int *)&candidateAttackers->maxNumShips,
                (int)attackersInPocket);
            kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(85516498));
            kasfShipsRemove(candidateAttackers, kasAITeamShipsPtrIfExists(87999618));
            kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
            kasTimerSetById(139352562, 7);
            kasTimerStartById(139352562);
        }
#if !defined(HW_Release) && CATACLYSM_DIAG
        else if (universe.totaltimeelapsed - m01DiagLastPocketAttackUpdateLogTime >= 10.0f)
        {
            dbgMessagef(
                "\n" CATACLYSM_DIAG_LABEL "mission01 pocket attack update team=%p live=%d launched=%d total=%.2f",
                kasCurrentTeamPtr,
                kasfTeamCount(),
                kasfTeamFinishedLaunching(),
                universe.totaltimeelapsed);
            m01DiagLastPocketAttackUpdateLogTime = universe.totaltimeelapsed;
        }
#endif
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(178361137)))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 pocket attack queue heavy interceptor batch total=%.2f",
            universe.totaltimeelapsed);
#endif
        kasfSpeechEvent(75929221, (sdword)Init_Mission01_QueueHeavyInterceptorBatch, (sdword)Watch_Mission01_HeavyInterceptorBatchComplete);
    }
    else
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 pocket attack team exhausted without heavy batch total=%.2f",
            universe.totaltimeelapsed);
#endif
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission01_AsteroidPocketAttackTeam_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetDamageFactor(&team->shipList, 10);
    kasfTeamSwitchPlayerOwner(FALSE);
    if (kasfTeamCount() > 0)
    {
        kasfShipsEnableAttachNLips(&team->shipList);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission01 pocket attack start team=%p original=%d total=%.2f",
        team,
        kasfTeamCountOriginal(),
        universe.totaltimeelapsed);
#endif
    kasfSpeechEvent(
        139967671,
        (sdword)Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers,
        (sdword)Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers);
}

void Init_Mission01_QueueHeavyInterceptorBatch(void)
{
    GrowSelection *heavyInterceptorBatch;
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    heavyInterceptorBatch = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(heavyInterceptorBatch);
#if !defined(HW_Release) && CATACLYSM_DIAG
    dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission01 create heavy interceptor batch original=%d total=%.2f",
        kasfTeamCountOriginal(),
        universe.totaltimeelapsed);
#endif
    kasfShipsCreateInside(
        heavyInterceptorBatch,
        "kHeavyInterceptor",
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(202030339),
        0);
}

void Watch_Mission01_HeavyInterceptorBatchComplete(void)
{
    GrowSelection *heavyInterceptorBatch;
    AITeam *team;
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 m01DiagLastHeavyBatchLogTime = -1000.0f;
#endif

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    heavyInterceptorBatch = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(heavyInterceptorBatch) >= kasfTeamCountOriginal())
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 heavy batch ready count=%d original=%d total=%.2f",
            kasfShipsCount(heavyInterceptorBatch),
            kasfTeamCountOriginal(),
            universe.totaltimeelapsed);
#endif
        kasfReinforceTeamWithShips(team, heavyInterceptorBatch);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_AsteroidPocketAttackTeam_Intro);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    else if (universe.totaltimeelapsed - m01DiagLastHeavyBatchLogTime >= 10.0f)
    {
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 heavy batch waiting count=%d original=%d total=%.2f",
            kasfShipsCount(heavyInterceptorBatch),
            kasfTeamCountOriginal(),
            universe.totaltimeelapsed);
        m01DiagLastHeavyBatchLogTime = universe.totaltimeelapsed;
    }
#endif
}

void Watch_Mission01_AsteroidPocketAttackTeam_Intro(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 heavy batch launched total=%.2f",
            universe.totaltimeelapsed);
#endif
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission01_AsteroidPocketAttackTeam_SelectAttackers,
            (sdword)Watch_Mission01_AsteroidPocketAttackTeam_UpdateAttackers);
    }
}

void Init_Mission01_ApplyUpgradesThenDockForLaunch(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        5597651,
        (sdword)Init_Mission01_DockCurrentTeamForLaunch,
        (sdword)Watch_Mission01_MissionFlow_FirstWarning);
}

void Init_Mission01_DockCurrentTeamForLaunch(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(196001810));
}

void Watch_Mission01_MissionFlow_FirstWarning(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
    static sdword m01DiagLoggedFirstWarningReady = 0;
#endif
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamDockedReadyForLaunch())
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        if (!m01DiagLoggedFirstWarningReady)
        {
            dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 first warning ready total=%.2f", universe.totaltimeelapsed);
            m01DiagLoggedFirstWarningReady = 1;
        }
#endif
        kasfMsgSend(kasAITeamPtrIfExists(196001810), (char *)137343732);
        kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_MissionFlow_FirstWarningDelay);
    }
}

void Watch_Mission01_MissionFlow_FirstWarningDelay(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 m01DiagLastFirstWarningDelayLogTime = -1000.0f;
#endif
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)54112162))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 first warning ack total=%.2f", universe.totaltimeelapsed);
#endif
        kasTimerCreateSetStartById(139352562, 13);
    }
    else if (kasTimerExpiredById(139352562))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 first warning launch total=%.2f", universe.totaltimeelapsed);
#endif
        kasfLaunch();
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_MissionFlow_FleetCommandIntro);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    else if (universe.totaltimeelapsed - m01DiagLastFirstWarningDelayLogTime >= 10.0f)
    {
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 first warning waiting total=%.2f", universe.totaltimeelapsed);
        m01DiagLastFirstWarningDelayLogTime = universe.totaltimeelapsed;
    }
#endif
}

void Watch_Mission01_MissionFlow_AttackInClawFormation(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationClaw();
}

void Init_Mission01_QueueMultigunCorvetteOrHeavyInterceptorBatch(void)
{
    char *shipType;
    GrowSelection *shipsToCreate;
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    shipsToCreate = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(shipsToCreate);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(204990129)))
    {
        shipType = "tMultiGunCorvette";
    }
    else
    {
        shipType = "tHeavyInterceptor";
    }
    kasfShipsCreateInside(
        shipsToCreate,
        shipType,
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(196001810),
        1);
}

void Watch_Mission01_MultigunCorvetteOrHeavyInterceptorBatchComplete(void)
{
    GrowSelection *shipsToCreate;
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    shipsToCreate = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(shipsToCreate) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips(team, shipsToCreate);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_MissionFlow_FleetCommandIntro);
    }
}

void Watch_Mission01_MissionFlow_SelectNearbyAttackersOrQueueReinforcements(void)
{
    GrowSelection *nearbyAttackers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
#if !defined(HW_Release) && CATACLYSM_DIAG
    dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission01 select nearby attackers teamCount=%d total=%.2f",
        kasfTeamCount(),
        universe.totaltimeelapsed);
#endif
    if (kasfTeamCount())
    {
        nearbyAttackers = kasGrowSelectionPtr(57245931);
        kasfGrowSelectionClear(nearbyAttackers);
        kasfShipsSelectNearby(
            nearbyAttackers,
            kasGrowSelectionPtr((sdword)&loc_4A2A29),
            kasShipsVectorPtrIfExists(211966048),
            10000);
        kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
    }
    else
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission01_QueueMultigunCorvetteOrHeavyInterceptorBatch,
            (sdword)Watch_Mission01_MultigunCorvetteOrHeavyInterceptorBatchComplete);
    }
}

void Watch_Mission01_MissionFlow_FleetCommandIntro(void)
{
    AITeam *team;
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 m01DiagLastFleetIntroLogTime = -1000.0f;
#endif

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 fleet intro launch complete total=%.2f", universe.totaltimeelapsed);
#endif
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission01_MissionFlow_AttackInClawFormation,
            (sdword)Watch_Mission01_MissionFlow_SelectNearbyAttackersOrQueueReinforcements);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    else if (universe.totaltimeelapsed - m01DiagLastFleetIntroLogTime >= 10.0f)
    {
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 fleet intro waiting total=%.2f", universe.totaltimeelapsed);
        m01DiagLastFleetIntroLogTime = universe.totaltimeelapsed;
    }
#endif
}

void Init_Mission01_AsteroidPocketEncounter_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfShipsSetDamageFactor(&team->shipList, 10);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(87999618)))
    {
        kasfSpeechEvent(
            76775316,
            (sdword)Init_Mission01_AsteroidPocketEncounter_FocusTarget,
            (sdword)Watch_Mission01_AsteroidPocketEncounter_AfterFocus);
    }
    else
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission01_AsteroidPocketEncounter_SelectAttackers,
            (sdword)Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers);
    }
}

void Init_Mission01_AsteroidPocketEncounter_FocusTarget(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(114551810));
}

void Watch_Mission01_AsteroidPocketEncounter_AfterFocus(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
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
                (sdword)Watch_Mission01_AsteroidPocketEncounter_SelectAttackers,
                (sdword)Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers);
        }
    }
}

void Watch_Mission01_AsteroidPocketEncounter_SelectAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    attackersInPocket = kasGrowSelectionPtr(22816);
    candidateAttackers = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(attackersInPocket);
    kasfGrowSelectionClear(candidateAttackers);
    kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
    kasfShipsSelectMissionVolumeInactivePlayerModules(
        (int *)&candidateAttackers->maxNumShips,
        (int)attackersInPocket);
    kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
    kasTimerCreateSetStartById(139352562, kasfRandom(1, 7));
}

void Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers(void)
{
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;
    AITeam *team;

    kasCurrentStep = 2;
    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
        if (kasfTeamFinishedLaunching() && kasTimerExpiredDestroyById(139352562))
        {
            attackersInPocket = kasGrowSelectionPtr(22816);
            candidateAttackers = kasGrowSelectionPtr(57245931);
            kasfGrowSelectionClear(attackersInPocket);
            kasfGrowSelectionClear(candidateAttackers);
            kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
            kasfShipsSelectMissionVolumeInactivePlayerModules(
                (int *)&candidateAttackers->maxNumShips,
                (int)attackersInPocket);
            kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
            kasTimerSetById(139352562, 7);
            kasTimerStartById(139352562);
        }
    }
    else if (kasfThisTeamIs(kasAITeamPtrIfExists(98363089)))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission01_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor,
            (sdword)Watch_Mission01_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady);
    }
    else
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}

void Init_Mission01_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor(void)
{
    GrowSelection *shipsToCreate;
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    shipsToCreate = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(shipsToCreate);
    kasfShipsCreateInside(
        shipsToCreate,
        "tHeavyInterceptor",
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(114551810),
        1);
}

void Watch_Mission01_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady(void)
{
    GrowSelection *shipsToCreate;
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    shipsToCreate = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(shipsToCreate) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, shipsToCreate);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_AsteroidPocketEncounter_ResumeAttackers);
    }
}

void Watch_Mission01_AsteroidPocketEncounter_ResumeAttackers(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission01_AsteroidPocketEncounter_SelectAttackers,
            (sdword)Watch_Mission01_AsteroidPocketEncounter_UpdateAttackers);
    }
}

void Init_Mission01_AsteroidPocketEncounter_FocusThreatWarning(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(114551810));
    kasfEntryGetOrCreateSetValue(-1970110140, 1);
#if !defined(HW_Release) && CATACLYSM_DIAG
    dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission01 focus threat warning team=%p total=%.2f",
        team,
        universe.totaltimeelapsed);
#endif
    kasfSpeechEvent(204400306, (sdword)MaintainTeamContext, (sdword)Watch_Mission01_AsteroidPocketEncounter_ThreatWarningAcknowledged);
}

void Watch_Mission01_AsteroidPocketEncounter_ThreatWarningAcknowledged(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 m01DiagLastThreatAckLogTime = -1000.0f;
#endif

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)213247342))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 threat warning acknowledged total=%.2f",
            universe.totaltimeelapsed);
#endif
        kasfLaunch();
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_AsteroidPocketEncounter_BeginDefenseLoop);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    else if (universe.totaltimeelapsed - m01DiagLastThreatAckLogTime >= 10.0f)
    {
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission01 threat warning waiting total=%.2f",
            universe.totaltimeelapsed);
        m01DiagLastThreatAckLogTime = universe.totaltimeelapsed;
    }
#endif
}

void Init_Mission01_ResearchShipTeam_Activate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasAITeamShipsPtr(&team->shipList, 6);
    if (!kasfEntryValueGet(71552175))
    {
        kasfEntryGetOrCreateSetValue(71552175, 1);
        kasfTeamSwitchPlayerOwner(FALSE);
        kasfShipsEnableAttachNLips(&team->shipList);
    }
}

void Init_Mission01_ProximitySensorObjective_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfHideShips(&team->shipList);
    kasfSpeechEvent(55103095, (sdword)MaintainTeamContext, (sdword)Init_Mission01_ProximitySensorObjective_WaitForTrigger);
}

void Init_Mission01_GarrisonObjective_FocusTarget(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfPatrolPath(kasPathPtr(222544508));
}

void Init_Mission01_GarrisonObjective_WaitForTrigger(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived(139644696))
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Init_Mission01_GarrisonObjective_ClearTrackedSelection,
            (sdword)Init_Mission01_GarrisonObjective_WaitForTrackedSelectionDestroyed);
    }
}

void Init_Mission01_GarrisonObjective_ClearTrackedSelection(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfCurrentTeamAttack(kasGrowSelectionPtr(213123832));
}

void Init_Mission01_GarrisonObjective_WaitForTrackedSelectionDestroyed(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(213123832)))
    {
        kasfSpeechEvent(
            122047671,
            (sdword)Init_Mission01_GarrisonObjective_FocusTarget,
            (sdword)Init_Mission01_GarrisonObjective_WaitForTrigger);
    }
}

void Init_Mission01_GarrisonObjective_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(71552176))
    {
        kasfEntryGetOrCreateSetValue(71552176, 1);
        kasfTeamSwitchPlayerOwner(FALSE);
        kasfShipsEnableAttachNLips(&team->shipList);
    }
    kasfSpeechEvent(
        122047671,
        (sdword)Init_Mission01_GarrisonObjective_FocusTarget,
        (sdword)Init_Mission01_GarrisonObjective_WaitForTrigger);
}

void Init_Mission01_CarrierObjective_BeginTracking(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerCreateSetStartById(139352562, 7);
    kasfShipsSetDamageFactor(&team->shipList, 50);
    kasfCurrentTeamAttack(kasAITeamShipsPtrIfExists(226001251));
}

void Init_Mission01_CarrierObjective_ClearTrackedCarrier(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfCurrentTeamAttack(kasGrowSelectionPtr(211966048));
}

void Init_Mission01_CarrierObjective_UpdateNearbyAttackers(void)
{
    AITeam *team;
    GrowSelection *nearbyAttackers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasTimerExpiredDestroyById(139352562))
    {
        kasTimerSetById(139352562, 7);
        kasTimerStartById(139352562);
        nearbyAttackers = kasGrowSelectionPtr(57245931);
        kasfShipsSelectNearby(
            nearbyAttackers,
            kasGrowSelectionPtr(4860457),
            kasTeamsVectorPtr(226001251),
            10000);
        kasfShipsSortByDistanceToPoint(nearbyAttackers, (vector *)((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
        kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
    }
    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(226001251)))
    {
        kasfSpeechEvent(
            165409248,
            (sdword)Init_Mission01_CarrierObjective_ClearTrackedCarrier,
            (sdword)MaintainTeamContext);
    }
}

void Init_Mission01_CarrierObjective_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        129867387,
        (sdword)Init_Mission01_CarrierObjective_BeginTracking,
        (sdword)Init_Mission01_CarrierObjective_UpdateNearbyAttackers);
}

void Init_Mission01_ControllerPocketObjective_SelectPocketShips(void)
{
    AITeam *team;
    GrowSelection *shipsInPocket;
    GrowSelection *pocketPlayers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetDamageFactor(&team->shipList, 10);
    shipsInPocket = kasGrowSelectionPtr(22816);
    pocketPlayers = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(shipsInPocket);
    kasfGrowSelectionClear(pocketPlayers);
    kasfFindShipsInside(kasVolumePtr(151951029), shipsInPocket);
    kasfShipsSelectMissionVolumeInactivePlayerModules(
        (int *)&pocketPlayers->maxNumShips,
        (int)shipsInPocket);
    kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
    kasfTeamSetPercentDamaged(40);
}

void Init_Mission01_ControllerPocketObjective_WaitForPocketReached(void)
{
    AITeam *team;
    GrowSelection *shipsInPocket;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived(307349))
    {
        kasfSpeechEvent(
            307349,
            (sdword)Init_Mission01_ControllerPocketObjective_FocusPocket,
            (sdword)Init_Mission01_ControllerPocketObjective_WaitForSpeechDelay);
        return;
    }

    shipsInPocket = kasGrowSelectionPtr(22816);
    if (kasfFindEnemiesNearSelection(shipsInPocket, 5000) &&
        kasfShipsSelectRace(shipsInPocket, shipsInPocket, 0))
    {
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
    }
    else
    {
        kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    }
}

void Init_Mission01_ControllerPocketObjective_FocusPocket(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetDamageFactor(&team->shipList, 50);
    kasfShipsSetPercentFueled(&team->shipList, 0.0f);
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 0);
    kasfPatrolPath(kasPathPtr(84922760));
}

void Init_Mission01_ControllerPocketObjective_WaitForSpeechDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamHealthAverage() > 20)
    {
        kasfTeamSetPercentDamaged(20);
    }
}

void Init_Mission01_ControllerPocketObjective_Start(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetDontApplyPhysicsFlag(&team->shipList, 1);
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        90812613,
        (sdword)Init_Mission01_ControllerPocketObjective_SelectPocketShips,
        (sdword)Init_Mission01_ControllerPocketObjective_WaitForPocketReached);
}

void Init_ApplyPlayerUpgradesToCurrentTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
}

void Init_Mission01_ApplyUpgradesThenProtectResourcers(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfProtect(kasAITeamShipsPtrIfExists(45804546));
}

void Init_Mission01_HarvestController_MoveToAsteroidPocket(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfMoveTo(kasVolumeVectorPtr(223798324));
}

void Init_Mission01_HarvestController_UpdateResourceFlow(void)
{

    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfEntryValueGet(71552178) &&
        kasfShipsAnyInSensorRange(&((AITeam *)kasCurrentTeamPtr)->shipList))
    {
        kasfEntryGetOrCreateSetValue(71552178, 1);
        kasfHarvest();
    }
    if (!kasfEntryValueGet(71552179) &&
        !kasfGetWorldResources() &&
        kasfTeamCount())
    {
        kasfEntryGetOrCreateSetValue(71552179, 1);
        kasfMsgSend(kasAITeamPtrIfExists(247899825), (char *)88401603);
        kasfMsgSend(kasAITeamPtrIfExists(247899826), (char *)88401603);
        kasfMsgSend(kasAITeamPtrIfExists(247899827), (char *)88401603);
    }
}

void Init_Mission01_HarvestController_SelectShips(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfProtect(kasAITeamShipsPtrIfExists(213732803));
}

void Init_ApplyUpgradesThenOpenMainScreen(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(
        77123413,
        (sdword)MaintainTeamContext,
        (sdword)Watch_Mission01_OpenMainScreenLaunchOnSignal);
}

void Watch_Mission01_OpenMainScreenLaunchOnSignal(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)88401603))
    {
        kasfLaunch();
        kasfSpeechEvent(
            84320888,
            (sdword)MaintainTeamContext,
            (sdword)Watch_Mission09_MissionFlow_GhostIntro);
    }
}
