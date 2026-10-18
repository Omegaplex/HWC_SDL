/*=============================================================================
    Name    : Tutorial1Scripts.c
    Purpose : Extracted Cataclysm mission-script callbacks for Tutorial1Scripts
=============================================================================*/

#include "MissionScriptSupport.h"

#define TUTORIAL_TEXT(english) { english, "French version goes here", "German version goes here" }

static char *tutorial1Lesson1ObjectiveDetailByLanguage[] =
{
    "You have clicked an objective.\nIn the singleplayer game and the tutorial, clicking an objective will give you more detailed information on how to complete that objective.  Press the OKAY button to continue the lesson.",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1EmptyObjectiveDetailByLanguage[] =
{
    "",
    "",
    "",
};

static char *tutorial1Lesson1Dummy1BriefByLanguage[] =
{
    "This is an objective",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1Lesson1Dummy2BriefByLanguage[] =
{
    "This is another objective",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1Lesson1ClickHereBriefByLanguage[] =
{
    "Click this objective to complete Lesson 1",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1Lesson2CameraHorzBriefByLanguage[] =
{
    "Rotate the camera 180 degrees horizontally.",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1Lesson2CameraVertBriefByLanguage[] =
{
    "Move camera to view the top of the Command Ship.",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1Lesson2CameraZoomBriefByLanguage[] =
{
    "Zoom the camera all of the way out.",
    "French version goes here",
    "German version goes here",
};

static char *tutorial1Lesson3FormClickBriefByLanguage[] = TUTORIAL_TEXT("Click select a group of Recons.");
static char *tutorial1Lesson3SingleClickBriefByLanguage[] = TUTORIAL_TEXT("Click select a single Recon.");
static char *tutorial1Lesson3BandBriefByLanguage[] = TUTORIAL_TEXT("Band select more than one type of ship.");
static char *tutorial1Lesson3TopRightBriefByLanguage[] = TUTORIAL_TEXT("Select ships through the ship list at the top right.");
static char *tutorial1Lesson3AddBriefByLanguage[] = TUTORIAL_TEXT("Use [Shift] to add ships to a selection.");
static char *tutorial1Lesson4FocusBriefByLanguage[] = TUTORIAL_TEXT("Focus on a selection.");
static char *tutorial1Lesson4AltFocusBriefByLanguage[] = TUTORIAL_TEXT("Hold [Alt] and bandbox to focus on some ships.");
static char *tutorial1Lesson5RightClickBriefByLanguage[] = TUTORIAL_TEXT("Bring up the Right Click Menu.");
static char *tutorial1Lesson5SensorsManagerBriefByLanguage[] = TUTORIAL_TEXT("Open Sensors Manager.");
static char *tutorial1Lesson5FocusBriefByLanguage[] = TUTORIAL_TEXT("Exit out of Sensors Manager to the main region.");
static char *tutorial1Lesson6MoveBriefByLanguage[] = TUTORIAL_TEXT("Move a ship into the cluster of asteroids.");
static char *tutorial1Lesson6PathCircularBriefByLanguage[] = TUTORIAL_TEXT("Create a patrol (circular) path.");
static char *tutorial1Lesson6PathLinearBriefByLanguage[] = TUTORIAL_TEXT("Create a normal (i.e. linear) path.");
static char *tutorial1Lesson7ClickAttackBriefByLanguage[] = TUTORIAL_TEXT("Click on a target drone to attack it.");
static char *tutorial1Lesson7BandAttackBriefByLanguage[] = TUTORIAL_TEXT("Hold [Ctrl] and bandbox to attack multiple drones.");
static char *tutorial1Lesson8BuildBriefByLanguage[] = TUTORIAL_TEXT("Build two Workers.");
static char *tutorial1Lesson9RepairBriefByLanguage[] = TUTORIAL_TEXT("Repair the Destroyer.");
static char *tutorial1Lesson9SalvageBriefByLanguage[] = TUTORIAL_TEXT("Salvage the enemy Ion Cannon Frigate.");
static char *tutorial1Lesson9HarvestBriefByLanguage[] = TUTORIAL_TEXT("Harvest resources.");
static char *tutorial1Lesson10SphereBriefByLanguage[] = TUTORIAL_TEXT("Set Acolytes into Delta formation.");
static char *tutorial1Lesson10AggressiveBriefByLanguage[] = TUTORIAL_TEXT("Set Acolytes into Aggressive tactics.");
static char *tutorial1Lesson10GuardBriefByLanguage[] = TUTORIAL_TEXT("Guard Workers with Acolytes.");
static char *tutorial1Lesson10GroupBriefByLanguage[] = TUTORIAL_TEXT("Form a selection group of all Recons and Acolytes.");
static char *tutorial1Lesson11ResearchBriefByLanguage[] = TUTORIAL_TEXT("Research Linking Technology.");
static char *tutorial1Lesson11UpgradeBriefByLanguage[] = TUTORIAL_TEXT("Upgrade Acolytes.");
static char *tutorial1Lesson11LinkBriefByLanguage[] = TUTORIAL_TEXT("Link the upgraded Acolytes.");
static char *tutorial1Lesson12TacticalBriefByLanguage[] = TUTORIAL_TEXT("Turn on the Tactical view.");
static char *tutorial1Lesson12ChaseBriefByLanguage[] = TUTORIAL_TEXT("Select one Recon and turn on chase camera view.");
static char *tutorial1Lesson12RetireBriefByLanguage[] = TUTORIAL_TEXT("Retire a ship.");
static char *tutorial1Lesson12ScuttleBriefByLanguage[] = TUTORIAL_TEXT("Scuttle a ship.");
static char *tutorial1Lesson12TimeBriefByLanguage[] = TUTORIAL_TEXT("Turn on time compression.");

static void nullsub_14(void)
{
}

void mrKeyPress(sdword ID);
sdword kasfShipsSelectAvailableUpgradeChanges(GrowSelection *result, GrowSelection *source);

void Watch_Tutorial1_HideUnhide(void);
void Watch_Tutorial1_HideUnhideRegionCleanup(void);
void Begin_Tutorial1_HideUnhideRegionCleanup(void);
void Init_Tutorial1_Tutorial03_SelectionPrompt(void);
void Watch_Tutorial1_Tutorial03_SelectionControls(void);
void Init_Tutorial1_Tutorial04_FocusPrompt(void);
void Watch_Tutorial1_Tutorial04_FocusControls(void);
void Init_Tutorial1_Tutorial05_RightClickPrompt(void);
void Watch_Tutorial1_Tutorial05_RightClickCommands(void);
void Init_Tutorial1_Tutorial06_WaypointMovementPrompt(void);
void Watch_Tutorial1_Tutorial06_WaypointMovement(void);
void Init_Tutorial1_Tutorial07_AttackCommandsPrompt(void);
void Watch_Tutorial1_Tutorial07_AttackCommands(void);
void Init_Tutorial1_Tutorial08_BuildCommandsPrompt(void);
void Watch_Tutorial1_Tutorial08_BuildCommands(void);
void Init_Tutorial1_Tutorial09_HarvestRepairSalvagePrompt(void);
void Watch_Tutorial1_Tutorial09_HarvestRepairSalvage(void);
void Init_Tutorial1_Tutorial10_GroupTacticsPrompt(void);
void Watch_Tutorial1_Tutorial10_GroupTactics(void);
void Watch_Tutorial1_Tutorial10_TaskbarDescription(void);
void Init_Tutorial1_Tutorial11_ResearchAndUpgradesPrompt(void);
void Watch_Tutorial1_Tutorial11_ResearchAndUpgrades(void);
void Init_Tutorial1_Tutorial12_CommandHotkeysPrompt(void);
void Init_Tutorial1_Tutorial12_PrimaryAttackTeam(void);
void Begin_Tutorial1_Tutorial12_PrimaryAttackTeamReentry(void);
void Watch_Tutorial1_Tutorial12_PrimaryAttackTeam(void);
void Init_Tutorial1_Tutorial12_PrimaryAttackReinforcements(void);
void Watch_Tutorial1_Tutorial12_PrimaryAttackApproach(void);
void Init_Tutorial1_Tutorial12_HiddenAttackTeam(void);
void Init_Tutorial1_Tutorial12_OtherAttackTeamHide(void);
sdword Begin_Tutorial1_EnableInterface(void);
void Begin_Tutorial1_DisplayCurrentLesson(void);
void Init_Tutorial1_Tutorial10_MoveContextMenu(void);
void Begin_Tutorial1_Tutorial10_MoveContextMenu(void);

void Watch_Tutorial1_CurrentLessonPrompt(void)
{
    sdword currentObjectives;

    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived("FIHelpTip"))
    {
        kasfEntrySetValue(156469699, objectiveGetAll());
        kasfSubtitleCleanup();
        kasfTutHideImages();
        kasfPingRemove("Ping");
        kasfTutHideImages();
        cmCloseIfOpen();
        rmCloseIfOpen();
        if (kasfSensorsIsOpen(0))
        {
            kasfCloseSensors(1);
        }
        kasfSpeechEvent(160823349, (sdword)Begin_Tutorial1_PlayCurrentLessonSpeech, (sdword)MaintainTeamContext);
        return;
    }

    currentObjectives = objectiveGetAll();
    if (kasfEntryValueGet(156469699) != currentObjectives || kasfConstructionManagerActive() || kasfResearchGUIActive())
    {
        kasfEntrySetValue(156469699, currentObjectives);
        kasTimerDestroyById(181587010);
        kasTimerSetById(181587010, 240);
        kasTimerStartById(181587010);
    }

    if (kasfEntryValueGet(124658245) || !kasTimerExpiredDestroyById(181587010))
    {
        if (kasfEntryValueGet(124658245) && (kasfConstructionManagerActive() || kasfResearchGUIActive()))
        {
            kasfSubtitleCleanup();
            kasfTutHideImages();
        }
        return;
    }

    kasfEntrySetValue(124658245, 1);
    if (objectiveCountCompleted())
    {
        kasfTutSetPointerTargetFERegion("objObjectivesButton", "ObjectivesButton");
        kasfTutSetPointerTargetXYRight("ptr2", 512, 64);
        kasfSpeechEventQueueCurrentPlayer(134222670, 0);
    }
    else
    {
        kasfTutShowImages("objObjectivesButton");
        kasfSpeechEventQueueCurrentPlayer(134222669, 0);
    }
    kasfEntrySetValue(168240451, 1);
    kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)MaintainTeamContext);
}

void Watch_Tutorial1_PostIntelCleanup(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(181587010);
    kasTimerSetById(181587010, 240);
    kasTimerStartById(181587010);
    if (!kasfIntelEventEnded() && !kasfEntryValueGet(-2054310428))
    {
        return;
    }

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
        kasfCloseSensors(1);
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
    if (kasfEntryValueGet(166041123))
    {
        kasfTutHideImages();
        kasfEntrySetValue(166041123, 0);
    }
    if (kasfEntryValueGet(124658245))
    {
        kasfEntrySetValue(124658245, 0);
    }
    if (kasfEntryValueGet(127756738))
    {
        kasfEntryIncrement(127756738);
    }
    else
    {
        kasfEntrySetValue(127756738, 1);
    }
    kasfSpeechEvent(244882162, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}

void Begin_Tutorial1_PlayCurrentLessonSpeech(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    switch (kasfEntryValueGet(-2119326914))
    {
        case 1: kasfSpeechEvent(254980721, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 2: kasfSpeechEvent(254980722, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 3: kasfSpeechEvent(254980723, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 4: kasfSpeechEvent(254980724, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 5: kasfSpeechEvent(254980725, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 6: kasfSpeechEvent(254980726, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 7: kasfSpeechEvent(254980727, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 8: kasfSpeechEvent(254980728, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 9: kasfSpeechEvent(254980729, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 10: kasfSpeechEvent(53159888, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 11: kasfSpeechEvent(53159889, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        case 12: kasfSpeechEvent(53159890, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext); break;
        default:
            kasfEntryValueGet(-2119326914);
            kasfLog("ErrorInvalidLesson");
            kasfSpeechEvent(244212684, (sdword)Init_WaitForIntelEventEnd, (sdword)MaintainTeamContext);
            break;
    }
}

void Watch_Tutorial1_Tutorial10_MoveContextMenu(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfMsgReceived((char *)48772594))
    {
        kasfUnhideShips(&team->shipList);
        kasfShipsSetRetaliation(&team->shipList);
        kasfSpeechEvent(
            139967671,
            (sdword)Init_Tutorial1_Tutorial10_MoveContextMenuAreaClear,
            (sdword)Watch_Tutorial1_Tutorial10_MoveContextMenuAreaClear);
    }
}

void Watch_Tutorial1_Tutorial10_CameraMoveUp(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(263722418))
    {
        kasfTeamSwitchPlayerOwner(TRUE);
        kasfEntrySetValue(263722418, 0);
    }
    kasfMoveTo(kasShipsVectorPtrIfExists(38093685));
}

void Watch_Tutorial1_Tutorial10_CameraMoveDown(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfEntryValueGet(263722418))
    {
        kasfTeamSwitchPlayerOwner(TRUE);
        kasfEntrySetValue(263722418, 0);
    }
    kasfDockStay(kasAITeamPtrIfExists(38093685));
}

void Watch_Tutorial1_Tutorial12_AttackTeamSignalRouter(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)0x09B3141E))
    {
        kasfSpeechEvent(188241742, (sdword)Begin_Tutorial1_Tutorial12_RevealHiddenAttackTeam, (sdword)Begin_Tutorial1_Tutorial12_RequestOtherTeamHide);
    }
    else
    {
        kasfMsgReceived((char *)0x0B315834);
    }
}

void Begin_Tutorial1_Tutorial12_RevealHiddenAttackTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfUnhideShips((GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfShipsSetPercentDamaged((GrowSelection *)(kasCurrentTeamPtr + 28), 30);
}

void Init_Tutorial1_Tutorial12_RequestOtherTeamHideFlag(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfEntryGetOrCreateSetValue(-1989121501, 1);
}

void Begin_Tutorial1_Tutorial12_RequestOtherTeamHide(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfSpeechEvent(54710165, (sdword)Init_Tutorial1_Tutorial12_RequestOtherTeamHideFlag, (sdword)Watch_Tutorial1_Tutorial12_OtherAttackTeamHide);
}

void Init_Tutorial1_Tutorial12_OtherAttackTeamHide(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfHideShips(&currentTeam->shipList);
}

void Watch_Tutorial1_Tutorial12_OtherAttackTeamHide(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfMsgReceived((char *)0x0B315834))
    {
        kasfSpeechEvent(59084052, (sdword)Init_Tutorial1_Tutorial12_OtherAttackTeamHide, (sdword)Begin_Tutorial1_Tutorial12_MaintainAttackTeamContext);
    }
    else
    {
        kasfMsgReceived((char *)0x09B3141E);
    }
}

void Begin_Tutorial1_Tutorial12_MaintainAttackTeamContext(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    kasfSpeechEvent(245013301, (sdword)MaintainTeamContext, (sdword)Watch_Tutorial1_Tutorial12_AttackTeamSignalRouter);
}

void Init_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfFormationDelta3D();
    if (kasfThisTeamIs(kasAITeamPtrIfExists(75148549)))
    {
        kasGrowSelectionPtr(211966048);
        tutDeallocateRootRegion();
    }
    else
    {
        kasfMoveTo(kasShipsVectorPtrIfExists(211966048));
    }
}

void Watch_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(75148549), (char *)4693508);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
        return;
    }

    if (!kasfThisTeamIs(kasAITeamPtrIfExists(75148549)) &&
        kasfShipsSelectNearby(
            kasGrowSelectionPtr(57245931),
            kasGrowSelectionPtr(0x4A2A29),
            kasShipsVectorPtr((char *)0x4A2A29),
            15000))
    {
        kasGrowSelectionPtr(57245931);
        tutDeallocateRootRegion();
    }
}
void Init_Tutorial1_HideUnhide(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = *(udword *)(kasCurrentTeamPtr + 88);
    if (kasfTeamDockedReadyForLaunch())
    {
        kasfMsgSend(kasAITeamPtrIfExists(90274303), (char *)238145755);
        kasfMsgSend(kasAITeamPtrIfExists(111680593), (char *)238145755);
        kasfMsgSend(kasAITeamPtrIfExists(111680594), (char *)238145755);
        kasfSpeechEvent(320211, (sdword)MaintainTeamContext, (sdword)Watch_Tutorial1_HideUnhide);
    }
}

void Init_Tutorial1_Tutorial01_ClickHerePrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)132122068);
    kasfObjectiveCreate("L1Dummy1", tutorial1Lesson1Dummy1BriefByLanguage[strCurLanguage], tutorial1Lesson1ObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L1Dummy1", 0);
    kasfObjectiveCreate("L1Dummy2", tutorial1Lesson1Dummy2BriefByLanguage[strCurLanguage], tutorial1Lesson1ObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L1Dummy2", 0);
}

void Watch_Tutorial1_Tutorial01_ClickHere(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfMsgReceived((char *)72850725))
    {
        kasfObjectiveDestroy("L1Dummy1");
        kasfObjectiveDestroy("L1Dummy2");
        kasfObjectiveCreate("L1ClickHere", tutorial1Lesson1ClickHereBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
        kasfObjectiveSet("L1ClickHere", 0);
    }
    if (kasfEntryValueGet(-2089119451))
    {
        kasfEntryScopeName(-2089119451);
        kasfObjectiveSet("L1ClickHere", 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)32966740);
    }
}

void Init_Tutorial1_Tutorial02_CameraControlsPrompt(void)
{
    sdword angleDeg;
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)148899284);
    kasfObjectiveCreate("L2CamHorz", tutorial1Lesson2CameraHorzBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L2CamHorz", 0);
    kasfObjectiveCreate("L2CamVert", tutorial1Lesson2CameraVertBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L2CamVert", 0);
    kasfObjectiveCreate("L2CamZoom", tutorial1Lesson2CameraZoomBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L2CamZoom", 0);
    angleDeg = kasfCameraGetAngleDeg();
    kasfEntryGetOrCreateSetValue(102117493, angleDeg);
    kasfEntryGetOrCreateSetValue(102278641, 0);
    kasfEntryGetOrCreate(123898464);
}

void Watch_Tutorial1_Tutorial02_CameraControls(void)
{
    sdword angleDelta;
    sdword angleDeg;
    sdword accumulatedAngle;
    AITeam *tutorialTeam;
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;

    if (kasfEntryValueGet(-1976577766))
    {
        kasfEntryScopeName(-1976577766);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)108109380);
    }
    if (kasfEntryValueGet(-1976783084))
    {
        kasfEntryScopeName(-1976783084);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)118462532);
    }
    if (kasfEntryValueGet(-1976764579))
    {
        kasfEntryScopeName(-1976764579);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)122798404);
    }

    angleDelta = kasfCameraGetAngleDeg() - kasfEntryValueGet(102117493);
    kasfEntrySetValue(123898464, angleDelta);
    if (kasfEntryValueGet(123898464) > 180)
    {
        kasfEntrySetValue(123898464, kasfEntryValueGet(123898464) - 360);
    }
    else if (kasfEntryValueGet(123898464) < -180)
    {
        kasfEntrySetValue(123898464, kasfEntryValueGet(123898464) + 360);
    }

    accumulatedAngle = kasfEntryValueGet(102278641) + kasfEntryValueGet(123898464);
    kasfEntrySetValue(102278641, accumulatedAngle);
    angleDeg = kasfCameraGetAngleDeg();
    kasfEntrySetValue(102117493, angleDeg);
    if (kasfEntryValueGet(102278641) > 180 || kasfEntryValueGet(102278641) < -180)
    {
        kasfObjectiveSet("L2CamHorz", 1);
    }
    if (kasfCameraGetDeclinationDeg() < -70)
    {
        kasfObjectiveSet("L2CamVert", 1);
    }
    if (kasfCameraGetDistance() > 12000)
    {
        kasfObjectiveSet("L2CamZoom", 1);
    }
    if (!kasfEntryValueGet(71552161) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552161, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        tutorialTeam = kasAITeamPtrIfExists(154016483);
        kasfMsgSend(tutorialTeam, (char *)33032276);
    }
}

void Init_Tutorial1_Tutorial03_SelectionPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)165676500);
    kasfObjectiveCreate("L3FormClick", tutorial1Lesson3FormClickBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L3FormClick", 0);
    kasfObjectiveCreate("L3SingleClick", tutorial1Lesson3SingleClickBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L3SingleClick", 0);
    kasfObjectiveCreate("L3Band", tutorial1Lesson3BandBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L3Band", 0);
    kasfObjectiveCreate("L3TopRight", tutorial1Lesson3TopRightBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L3TopRight", 0);
    kasfObjectiveCreate("L3Add", tutorial1Lesson3AddBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L3Add", 0);
    kasfEntryGetOrCreateSetValue(-1899089532, 0);
    nullsub_14();
}

void Watch_Tutorial1_Tutorial03_SelectionControls(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;
    GrowSelection *selectionInsideVolume;
    GrowSelection *selectionFiltered;
    sdword shipTypeCount;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfEntrySetValue(-1899089532, 0);
    if (kasfSelectContainsShipTypes("sRecon"))
    {
        kasfEntryIncrement(-1899089532);
    }
    if (kasfSelectContainsShipTypes("sAcolyte"))
    {
        kasfEntryIncrement(-1899089532);
    }
    if (kasfSelectContainsShipTypes("sACV"))
    {
        kasfEntryIncrement(-1899089532);
    }
    if (kasfSelectContainsShipTypes("sMothership"))
    {
        kasfEntryIncrement(-1899089532);
    }
    if (kasfSelectContainsShipTypes("sWorker"))
    {
        kasfEntryIncrement(-1899089532);
    }
    if (kasfMsgReceived((char *)141932740))
    {
        kasfEntryGetOrCreateSetValue(261441908, 1);
    }
    if (kasfEntryValueGet(261441908) && kasfEntryValueGet(-1899089532) > 1)
    {
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)248947220);
        kasfEntryScopeName(261441908);
    }
    if (kasfEntryValueGet(-1995213445))
    {
        kasfEntryScopeName(-1995213445);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)160675860);
    }
    if (kasfEntryValueGet(-2061590085))
    {
        kasfEntryScopeName(-2061590085);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)258020820);
    }
    if (kasfEntryValueGet(-2141736252))
    {
        kasfEntryScopeName(-2141736252);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)266562356);
    }
    if (kasfEntryValueGet(-2135522380))
    {
        kasfEntryScopeName(-2135522380);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)206092740);
    }
    if (kasfEntryValueGet(-1912244028))
    {
        kasfEntryScopeName(-1912244028);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)261125124);
    }
    if (kasfTutGameSentMessage("Game_ClickSelectGroup"))
    {
        selectionInsideVolume = kasGrowSelectionPtr(90801203);
        kasfFindShipsInside(kasVolumePtr(132933767), selectionInsideVolume);
        selectionFiltered = kasGrowSelectionPtr(90801203);
        shipTypeCount = kasfShipsSelectType(kasGrowSelectionPtr(90801203), selectionFiltered, "sRecon");
        if (kasfSelectContainsShipTypes("sRecon") == shipTypeCount)
        {
            kasfObjectiveSet("L3FormClick", 1);
        }
        if (kasfSelectContainsShipTypes("sRecon") == 1)
        {
            kasfObjectiveSet("L3SingleClick", 1);
        }
    }
    if (kasfTutGameSentMessage("Game_BandBoxSelect") && kasfEntryValueGet(-1899089532) > 1)
    {
        kasfObjectiveSet("L3Band", 1);
    }
    if (kasfTutGameSentMessage("ShipListClick"))
    {
        kasfObjectiveSet("L3TopRight", 1);
    }
    if (kasfTutGameSentMessage("MultipleSelect"))
    {
        kasfObjectiveSet("L3Add", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552162) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552162, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33097812);
    }
}

void Init_Tutorial1_Tutorial04_FocusPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)182453716);
    kasfObjectiveCreate("L4Focus", tutorial1Lesson4FocusBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L4Focus", 0);
    kasfObjectiveCreate("L4AltFocus", tutorial1Lesson4AltFocusBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L4AltFocus", 0);
    nullsub_14();
}

void Watch_Tutorial1_Tutorial04_FocusControls(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfEntryValueGet(-2054654301))
    {
        kasfEntryScopeName(-2054654301);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)95845428);
    }
    if (kasfEntryValueGet(-2056560589))
    {
        kasfEntryScopeName(-2056560589);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)127679764);
    }
    if (kasfTutGameSentMessage("KB_Focus"))
    {
        kasfObjectiveSet("L4Focus", 1);
    }
    if (kasfTutGameSentMessage("AltFocus"))
    {
        kasfObjectiveSet("L4AltFocus", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552163) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552163, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33163348);
    }
}
void Init_Tutorial1_Tutorial05_RightClickPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)199230932);
    kasfObjectiveCreate("L5RightClickMenu", tutorial1Lesson5RightClickBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L5RightClickMenu", 0);
    kasfObjectiveCreate("L5SM", tutorial1Lesson5SensorsManagerBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L5SM", 0);
    kasfEntryGetOrCreateSetValue(159739150, 1);
    nullsub_14();
}
void Watch_Tutorial1_Tutorial05_RightClickCommands(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfEntryValueGet(-2078029691))
    {
        kasfEntryScopeName(-2078029691);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)254389332);
    }
    if (kasfEntryValueGet(-2065671507))
    {
        kasfEntryScopeName(-2065671507);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)246576276);
    }
    if (kasfEntryValueGet(-2053605725))
    {
        kasfEntryScopeName(-2053605725);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)95845172);
    }
    if (kasfTutGameSentMessage("Game_PopupOpen"))
    {
        kasfObjectiveSet("L5RightClickMenu", 1);
    }
    if (kasfSensorsIsOpen(0))
    {
        if (!kasfEntryValueGet(318077))
        {
            kasfObjectiveSet("L5SM", 1);
        }
        if (kasfEntryValueGet(159739150))
        {
            kasfSubtitleCleanup();
            tutHideImageStrip();
            aitQueueClear(kasAITeamPtrIfExists(154016483));
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)150414485);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)150414484);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)150414483);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)150414482);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)150414481);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)160394087);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)187124149);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)41196338);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)41196337);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)187124148);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)187124147);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)41196499);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)41196498);
            kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)0x00B1EF6E);
            kasfEntrySetValue(-2066476165, 1);
            kasfEntryScopeName(159739150);
            kasfObjectiveCreate("L5Focus", tutorial1Lesson5FocusBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
            kasfObjectiveSet("L5Focus", 0);
        }
        if (!kasfObjectiveGet("L5Focus"))
        {
            kasfEntryGetOrCreateSetValue(255474452, 1);
        }
    }
    if (kasfEntryValueGet(255474452) && !kasfSensorsIsOpen(0) && !kasfConstructionManagerActive() && !kasfResearchGUIActive())
    {
        kasfObjectiveSet("L5Focus", 1);
        kasfEntryScopeName(255474452);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552164) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552164, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33228884);
    }
}

void Init_Tutorial1_Tutorial06_WaypointMovementPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)216008148);
    kasfObjectiveCreate("L6Move", tutorial1Lesson6MoveBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L6Move", 0);
    kasfObjectiveCreate("L6PathLinear", tutorial1Lesson6PathLinearBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L6PathLinear", 0);
    kasfObjectiveCreate("L6PathCircular", tutorial1Lesson6PathCircularBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L6PathCircular", 0);
    nullsub_14();
}
void Watch_Tutorial1_Tutorial06_WaypointMovement(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfMsgReceived((char *)55181822))
    {
        kasfEntryGetOrCreateSetValue(25288126, 1);
    }
    if (kasfMsgReceived((char *)233155902))
    {
        kasfEntryGetOrCreateSetValue(93096510, 1);
    }
    if (kasfEntryValueGet(-2141466555))
    {
        kasfEntryScopeName(-2141466555);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)245493348);
    }
    if (kasfEntryValueGet(-2047144894))
    {
        kasfEntryScopeName(-2047144894);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)180602724);
    }
    if (kasfEntryValueGet(-1923630334))
    {
        kasfEntryScopeName(-1923630334);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)180602724);
    }
    if (kasfEntryValueGet(25288126) && kasfTutGameSentMessage("MoveStart"))
    {
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)68610084);
        kasfEntryScopeName(25288126);
    }
    if (kasfEntryValueGet(93096510) && kasfTutGameSentMessage("WaypointStart"))
    {
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)236395412);
        kasfEntryScopeName(93096510);
    }
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(86599289), kasGrowSelectionPtr(132933767), kasTeamsVectorPtr(4969604), 2500))
    {
        kasfObjectiveSet("L6Move", 1);
    }
    if (kasfTutGameSentMessage("WaypointCircular"))
    {
        kasfObjectiveSet("L6PathCircular", 1);
    }
    if (kasfTutGameSentMessage("WaypointNormal"))
    {
        kasfObjectiveSet("L6PathLinear", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552165) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552165, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33294420);
    }
}

void Init_Tutorial1_Tutorial07_AttackCommandsPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)232785364);
    kasfObjectiveCreate("L7ClickAttack", tutorial1Lesson7ClickAttackBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L7ClickAttack", 0);
    kasfObjectiveCreate("L7BandAttack", tutorial1Lesson7BandAttackBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L7BandAttack", 0);
    nullsub_14();
}

void Watch_Tutorial1_Tutorial07_AttackCommands(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfMsgReceived((char *)220868811))
    {
        kasfEntryGetOrCreateSetValue(135715995, 1);
    }
    else if (kasfMsgReceived((char *)72202971))
    {
        kasfEntryGetOrCreateSetValue(18773019, 1);
    }
    if (kasfEntryValueGet(135715995) && kasfSelectNumSelected())
    {
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)126577179);
        kasfEntryScopeName(135715995);
    }
    else if (kasfEntryValueGet(135715995) && kasfSelectNumSelected())
    {
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)166168955);
        kasfEntryScopeName(18773019);
    }
    if (kasfEntryValueGet(-1965898229))
    {
        kasfEntryScopeName(-1965898229);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)252103780);
    }
    if (kasfEntryValueGet(-2009974853))
    {
        kasfEntryScopeName(-2009974853);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)140905492);
    }
    if (kasfTutGameSentMessage("ClickAttack"))
    {
        kasfObjectiveSet("L7ClickAttack", 1);
    }
    if (kasfTutGameSentMessage("BandAttack"))
    {
        kasfObjectiveSet("L7BandAttack", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552166) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33359956);
    }
}

void Init_Tutorial1_Tutorial08_BuildCommandsPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)249562580);
    kasfObjectiveCreate("L8Build", tutorial1Lesson8BuildBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L8Build", 0);
    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(132933767));
    kasfShipsSelectType(kasGrowSelectionPtr(90801203), kasGrowSelectionPtr(132933767), "sWorker");
    kasfEntryGetOrCreateSetValue(205979459, kasfShipsCount(kasGrowSelectionPtr(90801203)));
    nullsub_14();
}

void Watch_Tutorial1_Tutorial08_BuildCommands(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;
    sdword workerCount;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfMsgReceived((char *)68935342))
    {
        kasfEntryGetOrCreateSetValue(141913358, 1);
    }
    if (kasfEntryValueGet(141913358) && kasfConstructionManagerActive())
    {
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)244856750);
        kasfEntryScopeName(141913358);
    }
    if (kasfEntryValueGet(-2050305020))
    {
        kasfEntryScopeName(-2050305020);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)152229684);
    }
    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(132933767));
    workerCount = kasfShipsSelectType(kasGrowSelectionPtr(90801203), kasGrowSelectionPtr(132933767), "sWorker");
    if (workerCount - kasfEntryValueGet(205979459) >= 2)
    {
        kasfObjectiveSet("L8Build", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552167) && objectiveGetAll() && !kasfConstructionManagerActive())
    {
        kasfEntryGetOrCreateSetValue(71552167, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33425492);
    }
}

void Init_Tutorial1_Tutorial09_HarvestRepairSalvagePrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)266339796);
    kasfObjectiveCreate("L9Repair", tutorial1Lesson9RepairBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L9Repair", 0);
    kasfObjectiveCreate("L9Salvage", tutorial1Lesson9SalvageBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L9Salvage", 0);
    kasfObjectiveCreate("L9Harvest", tutorial1Lesson9HarvestBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L9Harvest", 0);
    kasfEntryGetOrCreateSetValue(-1937265244, 1);
    nullsub_14();
}

void Watch_Tutorial1_Tutorial09_HarvestRepairSalvage(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfEntryValueGet(-1900108366))
    {
        kasfEntryScopeName(-1900108366);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)108160324);
    }
    if (kasfEntryValueGet(-1976590251))
    {
        kasfEntryScopeName(-1976590251);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)101132612);
    }
    if (kasfEntryValueGet(-2127974572))
    {
        kasfEntryScopeName(-2127974572);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)67658052);
    }
    if (kasfEntryValueGet(-1989121501) && kasfShipsHealthAverage(kasAITeamShipsPtrIfExists(74072178)) >= 90)
    {
        kasfObjectiveSet("L9Repair", 1);
    }
    if (kasfGetShipTypeCount("tIonCannonFrigate", 0) > kasfEntryValueGet(244811028))
    {
        kasfObjectiveSet("L9Salvage", 1);
    }
    if (kasfTutGameSentMessage("KB_Harvest,Game_ClickHarvest"))
    {
        kasfObjectiveSet("L9Harvest", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552168) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552168, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)33491028);
    }
}

void Init_Tutorial1_Tutorial10_GroupTacticsPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)0x00C0BC44);
    kasfObjectiveCreate("L10Sphere", tutorial1Lesson10SphereBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L10Sphere", 0);
    kasfObjectiveCreate("L10Aggressive", tutorial1Lesson10AggressiveBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L10Aggressive", 0);
    kasfObjectiveCreate("L10Guard", tutorial1Lesson10GuardBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L10Guard", 0);
    kasfObjectiveCreate("L10Group", tutorial1Lesson10GroupBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L10Group", 0);
    kasfEntryGetOrCreateSetValue(244811028, kasfGetShipTypeCount("dBuoy", 0));
    kasfShipsSetPercentDamaged(kasGrowSelectionPtr(211966048), 75);
    nullsub_14();
}

void Watch_Tutorial1_HideUnhide(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfMsgReceived((char *)38030408))
    {
        kasfTeamSwitchPlayerOwner(TRUE);
        kasfShipsEnableAttachNLips(&currentTeam->shipList);
        kasfLaunch();
        kasfSpeechEvent(139967671, (sdword)MaintainTeamContext, (sdword)Watch_Tutorial1_HideUnhideRegionCleanup);
    }
}

void Watch_Tutorial1_HideUnhideRegionCleanup(void)
{
    GrowSelection *selection;
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (!kasfEntryValueGet(71552174) && kasfTeamFinishedLaunching())
    {
        kasfEntryGetOrCreateSetValue(71552174, 1);
        selection = kasGrowSelectionPtr(114959785);
        tutDeallocateRootRegion((int)selection);
    }
    if (kasfMsgReceived((char *)17444564))
    {
        kasfVarDestroy((char *)&currentTeam->shipList);
        kasfSpeechEvent(141957152, (sdword)Begin_Tutorial1_HideUnhideRegionCleanup, (sdword)MaintainTeamContext);
    }
}

void Begin_Tutorial1_HideUnhideRegionCleanup(void)
{
    GrowSelection *selection;
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    selection = kasGrowSelectionPtr(114959785);
    tutDeallocateRootRegion((int)selection);
}

void Watch_Tutorial1_Tutorial10_GroupTactics(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (kasfEntryValueGet(-1999974587))
    {
        kasfEntryScopeName(-1999974587);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)23337124);
    }
    if (kasfEntryValueGet(-2088670363))
    {
        kasfEntryScopeName(-2088670363);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)95137540);
    }
    if (kasfEntryValueGet(-2103937484))
    {
        kasfEntryScopeName(-2103937484);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)77669652);
    }
    if (kasfEntryValueGet(-2103929616))
    {
        kasfEntryScopeName(-2103929616);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)73656596);
    }
    if (kasfTutGameSentMessage("Delta_Formation") && kasfSelectContainsShipTypes("sAcolyte") > 1)
    {
        kasfObjectiveSet("L10Sphere", 1);
    }
    if (kasfTutGameSentMessage("KB_TacticsAggressive") && kasfSelectContainsShipTypes("sAcolyte"))
    {
        kasfObjectiveSet("L10Aggressive", 1);
    }
    if (kasfTutGameSentMessage("Game_BandBoxGuard") && kasfSelectContainsShipTypes("sAcolyte"))
    {
        kasfObjectiveSet("L10Guard", 1);
    }
    if (kasfTutGameSentMessage("KB_GroupAssign") && kasfSelectContainsShipTypes("sRecon") && kasfSelectContainsShipTypes("sAcolyte"))
    {
        kasfObjectiveSet("L10Group", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552169) && objectiveGetAll())
    {
        kasfEntryGetOrCreateSetValue(71552169, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)142633092);
    }
}

void Watch_Tutorial1_Tutorial10_TaskbarDescription(void)
{
    MaxSelection selection;

    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    MakeShipsOnlyHaveUpgrade(&selection.numShips, 64);
    MakeShipsSpecialActivateCapable((SelectCommand *)&selection, TRUE);
    if (selection.numShips > 0)
        smOverlayOpenView28();
}

void Init_Tutorial1_Tutorial11_ResearchAndUpgradesPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)0x0DC0BC54);
    kasfObjectiveCreate("L11Research", tutorial1Lesson11ResearchBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L11Research", kasfDoesPlayerHave("TECH_LINK") != 0);
    kasfEntryGetOrCreateSetValue(172537829, 0);
    kasfEntryGetOrCreateSetValue(220597867, 0);
    nullsub_14();
}

void Watch_Tutorial1_Tutorial11_ResearchAndUpgrades(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;
    sdword availableUpgradeChanges;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (!kasfEntryValueGet(172537829) && kasfObjectiveGet("L11Research"))
    {
        kasfEntrySetValue(172537829, 1);
        kasfObjectiveCreate("L11Upgrade", tutorial1Lesson11UpgradeBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
        kasfObjectiveSet("L11Upgrade", 0);
    }
    if (!kasfEntryValueGet(220597867) && kasfObjectiveGet("L11Upgrade"))
    {
        availableUpgradeChanges = kasfShipsSelectAvailableUpgradeChanges(kasGrowSelectionPtr(342540), kasGrowSelectionPtr(138538227));
        if (kasfShipsCount(kasGrowSelectionPtr(138538227)) - availableUpgradeChanges >= 2)
        {
            kasfEntryGetOrCreateSetValue(220597867, 1);
            kasfObjectiveCreate("L11Link", tutorial1Lesson11LinkBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
            kasfObjectiveSet("L11Link", 0);
        }
    }
    if (kasfMsgReceived((char *)68935390))
    {
        kasfEntryGetOrCreateSetValue(158690574, 1);
    }
    if (kasfEntryValueGet(158690574) && kasfResearchGUIActive())
    {
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)244791214);
        kasfEntryScopeName(158690574);
    }
    if (kasfEntryValueGet(-2072228520))
    {
        kasfEntryScopeName(-2072228520);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)230156916);
    }
    if (kasfEntryValueGet(-1901316299))
    {
        kasfEntryScopeName(-1901316299);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)38090452);
    }
    if (kasfEntryValueGet(-2060893141))
    {
        kasfEntryScopeName(-2060893141);
        aitQueueClear(kasAITeamPtrIfExists(154016483));
        kasfEntrySetValue(-2066476165, 0);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)209906804);
    }
    kasfFindShipsInside(kasVolumePtr(132933767), kasGrowSelectionPtr(132933767));
    if (!kasfEntryValueGet(71552170) && kasfDoesPlayerHave("TECH_LINK"))
    {
        kasfEntryGetOrCreateSetValue(71552170, 1);
        kasfObjectiveSet("L11Research", 1);
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)160322645);
    }
    kasfShipsSelectType(kasGrowSelectionPtr(138538227), kasGrowSelectionPtr(132933767), "sAcolyte");
    if (kasfDoesPlayerHave("TECH_LINK"))
    {
        availableUpgradeChanges = kasfShipsSelectAvailableUpgradeChanges(kasGrowSelectionPtr(342540), kasGrowSelectionPtr(138538227));
        if (availableUpgradeChanges < kasfShipsCount(kasGrowSelectionPtr(138538227)) ||
            kasfShipsSelectType(kasGrowSelectionPtr(342540), kasGrowSelectionPtr(132933767), "sACV"))
        {
            kasfObjectiveSet("L11Upgrade", 1);
        }
    }
    if (kasfShipsSelectType(kasGrowSelectionPtr(342540), kasGrowSelectionPtr(132933767), "sACV"))
    {
        kasfObjectiveSet("L11Link", 1);
    }
    nullsub_14();
    if (!kasfEntryValueGet(71552171) && kasfObjectiveGet("L11Link"))
    {
        kasfEntryGetOrCreateSetValue(71552171, 1);
        kasfSubtitleCleanup();
        tutHideImageStrip();
        kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)142436484);
    }
}

void Init_Tutorial1_Tutorial12_CommandHotkeysPrompt(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)247512148);
    kasfObjectiveCreate("L12Tactical", tutorial1Lesson12TacticalBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L12Tactical", 0);
    kasfObjectiveCreate("L12Chase", tutorial1Lesson12ChaseBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L12Chase", 0);
    kasfObjectiveCreate("L12Retire", tutorial1Lesson12RetireBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L12Retire", 0);
    kasfObjectiveCreate("L12Scuttle", tutorial1Lesson12ScuttleBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L12Scuttle", 0);
    kasfObjectiveCreate("L12Time", tutorial1Lesson12TimeBriefByLanguage[strCurLanguage], tutorial1EmptyObjectiveDetailByLanguage[strCurLanguage]);
    kasfObjectiveSet("L12Time", 0);
    nullsub_14();
}

void Init_Tutorial1_Tutorial12_PrimaryAttackTeam(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    scope = currentTeam->kasLabelHash;
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfSpeechEvent(129389189, (sdword)Begin_Tutorial1_Tutorial12_PrimaryAttackTeamReentry, (sdword)Watch_Tutorial1_Tutorial12_PrimaryAttackTeam);
}

void Begin_Tutorial1_Tutorial12_PrimaryAttackTeamReentry(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfTeamAttributesBitSet(0x40000);
    kasfTeamHyperspaceIn(kasVectorPtrIfExists(75103795));
}

void Watch_Tutorial1_Tutorial12_PrimaryAttackTeam(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(75929221, (sdword)Init_Tutorial1_Tutorial12_PrimaryAttackReinforcements, (sdword)Watch_Tutorial1_Tutorial12_PrimaryAttackApproach);
    }
}

void Init_Tutorial1_Tutorial12_PrimaryAttackReinforcements(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfStop();
    kasfShipsCreate(kasGrowSelectionPtr(21159), "dBuoy", 9, 1);
    kasfReinforceTeamWithShips(currentTeam, kasGrowSelectionPtr(21159));
    kasfTeamAttributesBitSet(128);
    kasfAttackMothership();
    kasfFormationWall();
}

void Watch_Tutorial1_Tutorial12_PrimaryAttackApproach(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    if (!kasfShipsSelectMoving(kasGrowSelectionPtr(342540), &currentTeam->shipList))
    {
        kasfSpeechEvent(129389189, (sdword)Begin_Tutorial1_Tutorial12_PrimaryAttackTeamReentry, (sdword)Watch_Tutorial1_Tutorial12_PrimaryAttackTeam);
    }
}

void Init_Tutorial1_Tutorial12_HiddenAttackTeam(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    scope = currentTeam->kasLabelHash;
    kasfShipsSetPercentDamaged(&currentTeam->shipList, 30);
    kasfShipsSetDontApplyPhysicsFlag(&currentTeam->shipList, 1);
    kasfShipsSetNonRetaliation(&currentTeam->shipList);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(74072178)))
    {
        kasfShipsSwitchPlayerOwner(&currentTeam->shipList, TRUE);
    }
    kasfHideShips(&currentTeam->shipList);
    kasfSpeechEvent(245013301, (sdword)MaintainTeamContext, (sdword)Watch_Tutorial1_Tutorial12_AttackTeamSignalRouter);
}

sdword Begin_Tutorial1_EnableInterface(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    kasfSubtitleDraw();
    kasfTutRemovePointer("ObjectivePtr");
    tutEnableEverything();
    return 0;
}

void Begin_Tutorial1_DisplayCurrentLesson(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    scope = currentTeam->kasLabelHash;
    objectiveDestroyAll();
    aitQueueClear(kasAITeamPtrIfExists(154016483));
    kasfEntrySetValue(-2066476165, 0);
    kasfMsgSend(kasAITeamPtrIfExists(154016483), (char *)64284357);
    kasfMsgSend(kasAITeamPtrIfExists(123862261), (char *)187783220);
    kasfMsgSend(kasAITeamPtrIfExists(74072178), (char *)187783220);
    kasTimerSetById(97472066, 4);
    kasTimerStartById(97472066);
    switch (kasfEntryValueGet(-2119326914))
    {
        case 1:
            kasfSpeechEvent((sdword)0x0A883A1, (sdword)Init_Tutorial1_Tutorial01_ClickHerePrompt, (sdword)Watch_Tutorial1_Tutorial01_ClickHere);
            break;
        case 2:
            kasfSpeechEvent((sdword)0x0A883A2, (sdword)Init_Tutorial1_Tutorial02_CameraControlsPrompt, (sdword)Watch_Tutorial1_Tutorial02_CameraControls);
            break;
        case 3:
            kasfSpeechEvent((sdword)0x0A883A3, (sdword)Init_Tutorial1_Tutorial03_SelectionPrompt, (sdword)Watch_Tutorial1_Tutorial03_SelectionControls);
            break;
        case 4:
            kasfSpeechEvent((sdword)0x0A883A4, (sdword)Init_Tutorial1_Tutorial04_FocusPrompt, (sdword)Watch_Tutorial1_Tutorial04_FocusControls);
            break;
        case 5:
            kasfSpeechEvent((sdword)0x0A883A5, (sdword)Init_Tutorial1_Tutorial05_RightClickPrompt, (sdword)Watch_Tutorial1_Tutorial05_RightClickCommands);
            break;
        case 6:
            kasfSpeechEvent((sdword)0x0A883A6, (sdword)Init_Tutorial1_Tutorial06_WaypointMovementPrompt, (sdword)Watch_Tutorial1_Tutorial06_WaypointMovement);
            break;
        case 7:
            kasfSpeechEvent((sdword)0x0A883A7, (sdword)Init_Tutorial1_Tutorial07_AttackCommandsPrompt, (sdword)Watch_Tutorial1_Tutorial07_AttackCommands);
            break;
        case 8:
            kasfSpeechEvent((sdword)0x0A883A8, (sdword)Init_Tutorial1_Tutorial08_BuildCommandsPrompt, (sdword)Watch_Tutorial1_Tutorial08_BuildCommands);
            break;
        case 9:
            kasfSpeechEvent((sdword)0x0A883A9, (sdword)Init_Tutorial1_Tutorial09_HarvestRepairSalvagePrompt, (sdword)Watch_Tutorial1_Tutorial09_HarvestRepairSalvage);
            break;
        case 10:
            kasfSpeechEvent(176699936, (sdword)Init_Tutorial1_Tutorial10_GroupTacticsPrompt, (sdword)Watch_Tutorial1_Tutorial10_GroupTactics);
            break;
        case 11:
            tutShowNextButton();
            kasfSpeechEvent(176699937, (sdword)Init_Tutorial1_Tutorial11_ResearchAndUpgradesPrompt, (sdword)Watch_Tutorial1_Tutorial11_ResearchAndUpgrades);
            break;
        case 12:
            tutHideNextButtonRegion();
            kasfSpeechEvent(176699938, (sdword)Init_Tutorial1_Tutorial12_CommandHotkeysPrompt, (sdword)mrKeyPress);
            break;
        default:
            kasfLogInteger("ERROR:  Invalid lesson (%d).", kasfEntryValueGet(-2119326914));
            kasfSpeechEvent(39356997, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
            break;
    }
}

void Init_Tutorial1_Tutorial10_MoveContextMenu(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 1;
    scope = currentTeam->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&currentTeam->shipList);
    kasfSpeechEvent(129389189, (sdword)Begin_Tutorial1_Tutorial10_MoveContextMenu, (sdword)MaintainTeamContext);
}

void Begin_Tutorial1_Tutorial10_MoveContextMenu(void)
{
    AITeam *currentTeam = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    kasfHideShips(&currentTeam->shipList);
    kasfShipsSetNonRetaliation(&currentTeam->shipList);
}

#undef TUTORIAL_TEXT
