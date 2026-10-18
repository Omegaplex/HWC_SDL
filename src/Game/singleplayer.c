#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "bink.h"

#include "types.h"
#include "vector.h"
#include "linkedlist.h"
#include "objtypes.h"
#include "spaceobj.h"
#include "ACV.h"
#include "statscript.h"
#include "font.h"
#include "feflow.h"
#include "universe.h"
#include "univupdate.h"
#include "physics.h"
#include "singleplayer.h"
#include "Globals.h"
#include "GamePick.h"
#include "DDDFrigate.h"
#include "HiveFrigate.h"
#include "taskbar.h"
#include "utility.h"

#include "mainrgn.h"
#include "aitrack.h"
#include "aiship.h"
#include "levelload.h"
#include "sensors.h"
#include "nis.h"
#include "mouse.h"

#include "select.h"
#include "soundevent.h"
#include "soundstructs.h"
#include "collision.h"
#include "ResearchShip.h"
#include "InfoOverlay.h"

#include "Ship.h"
#include "GravWellGenerator.h"
#include "AIPlayer.h"
#include "KAS.h"
#include "KASHashes.h"

#include "hs.h"

#include "AIVar.h"
#include "Objectives.h"
#include "ResearchGUI.h"
#include "launchMgr.h"
#include "trademgr.h"
#include "Mothership.h"
#include "file.h"
#include "Memory.h"
#include "etg.h"
#include "horserace.h"
#include "attributes.h"
#include "dock.h"
#include "ResearchAPI.h"
#include "SaveGame.h"
#include "damage.h"
#include "TradeMgr.h"
#include "ping.h"

#include "Tutor.h"
#include "randy.h"
#include "KASFunc.h"
#include "AITeam.h"
#include "Clamp.h"

#include "..\\Generated\\mission01.h"
#include "..\\Generated\\mission02.h"
#include "..\\Generated\\mission03.h"
#include "..\\Generated\\mission04.h"
#ifdef OEM
#include "..\\Generated\\mission05_OEM.h"
#else
#include "..\\Generated\\mission05.h"
#include "..\\Generated\\mission06.h"
#include "..\\Generated\\mission07.h"
#include "..\\Generated\\mission08.h"
#include "..\\Generated\\mission09.h"
#include "..\\Generated\\mission10.h"
#include "..\\Generated\\mission11.h"
#include "..\\Generated\\mission12.h"
#include "..\\Generated\\mission13.h"
#include "..\\Generated\\mission14.h"
#include "..\\Generated\\mission15.h"
#include "..\\Generated\\mission16.h"
#endif

bool32 salCapCantDoFormation(Ship *ship);
void SalCapDropTarget(Ship *ship);
#include "..\\Generated\\tutorial1.h"

#include "strings.h"
#include "animatic.h"
#include "gravwellgenerator.h"
#include "commandwrap.h"
#include "mothership.h"
#include "alliance.h"
#include "dock.h"
#include "Clamp.h"

// Forward declarations for functions without headers
void acolyteStartLinking(Ship *ship);
void shipApplyPlayerUpgrades(Ship *ship);
void mouseForceInvisible(void);
void GetShipsClampedTo(Ship *clampedto,GrowSelection *clampedships);
void DeleteAnyMidLevelHyperspacingShips(void);
void subReset(void);
void subReloadCardTextures(void);
void ioOverlayElapsedTimeResetFromUniverse(void);
extern bool8 etgHasBeenStarted;

char *spHyperspaceCommandMessage = NULL;
sdword dword_A5B1B0 = 0;  // KAS-forced sensors level flag (set by kasfOpenSensors/kasfCloseSensors)

#define NUMBER_SINGLEPLAYER_MISSIONS 19

#define ShipCanMakeHyperspace(ship) (ShipCanHyperspace(ship) && ((ship)->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange))
#define WARP_FILENAME "Warp.script"

//stores the current mission filename
char CurrentLevelName[PATH_Max + 1] = "";
char spMissionsDir[32];
char spMissionsFile[32];

#define NUMBER_SINGLEPLAYER_NIS 11
#define NISLETS_PER_LEVEL 3

// Cataclysm NIS files: n01-n11, no race suffix (single race campaign)
char *nisR1Names[NUMBER_SINGLEPLAYER_NIS][2] =
{
    { "nis\\n01.nis",    "nis\\n01.script" },
    { "nis\\n02.nis",    "nis\\n02.script" },
    { "nis\\n03.nis",    "nis\\n03.script" },
    { "nis\\n04.nis",    "nis\\n04.script" },
    { "nis\\n05.nis",    "nis\\n05.script" },
    { "nis\\n06.nis",    "nis\\n06.script" },
    { "nis\\n07.nis",    "nis\\n07.script" },
    { "nis\\n08.nis",    "nis\\n08.script" },
    { "nis\\n09.nis",    "nis\\n09.script" },
    { "nis\\n10.nis",    "nis\\n10.script" },
    { "nis\\n11.nis",    "nis\\n11.script" },
};
char *singlePlayerNISName = NULL;

// Cataclysm has no R2 campaign, but keep the table for compatibility
char *nisR2Names[NUMBER_SINGLEPLAYER_NIS][2] =
{
    { "nis\\n01.nis",    "nis\\n01.script" },
    { "nis\\n02.nis",    "nis\\n02.script" },
    { "nis\\n03.nis",    "nis\\n03.script" },
    { "nis\\n04.nis",    "nis\\n04.script" },
    { "nis\\n05.nis",    "nis\\n05.script" },
    { "nis\\n06.nis",    "nis\\n06.script" },
    { "nis\\n07.nis",    "nis\\n07.script" },
    { "nis\\n08.nis",    "nis\\n08.script" },
    { "nis\\n09.nis",    "nis\\n09.script" },
    { "nis\\n10.nis",    "nis\\n10.script" },
    { "nis\\n11.nis",    "nis\\n11.script" },
};

char *nisletNames[NISLETS_PER_LEVEL] =
{
    "nis\\nislet01",
    "nis\\nislet02",
    "nis\\nislet03",
};

bool8 nisLoadLevelBeforeStart[NUMBER_SINGLEPLAYER_NIS] =
{
    TRUE,
    FALSE,
    FALSE,
    TRUE,
    FALSE,
    TRUE,
    TRUE,
    FALSE,
    FALSE,
    TRUE,
    FALSE
};

static sdword missionToNis[NUMBER_SINGLEPLAYER_MISSIONS + 1] =
{
    0, 1, -1, -1, 2, -1, 4, 5, -1, -1, 6, -1, 7, -1, 8, 9, 10, 11, 0, 0
};

static bool32 missionLoadsLevelAfterNIS[NUMBER_SINGLEPLAYER_MISSIONS + 1] =
{
    FALSE, TRUE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE,
    TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE
};

static bool32 missionUsesPostLoadArrivalPrepass[NUMBER_SINGLEPLAYER_MISSIONS + 1] =
{
    FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE,
    TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE
};

sdword singlePlayerNisNumber = -1;
sdword singlePlayerNisletNumber = -1;

bool32 singlePlayerHyperspacingInto = FALSE;

sdword spAutoSavePending = 0;        // autosave pending flag (0xA43FEC)
sdword spAutoSaveMissionNumber = 0;  // autosave mission number (0xA43FE8)
char   spAutoSaveMissionName[256] = {0}; // autosave mission name (0xA43F74)

bool32 spHoldHyperspaceWindow = FALSE;

bool32 hyperspaceOverride = FALSE;

real32 spFleetModifier = 0.0f;
real32 spFleetStr = 0.0f;
real32 GameDifficultyPrimaryModifier = 1.0f;
sdword GameDifficultyBuildTimeModifier = 1;
sdword GameDifficultyProductionModifier = 1;

#define WARP_TECH_STATE_COUNT 42

typedef struct WarpScriptLevelData
{
    sdword resources;
    sdword shipCounts[TOTAL_NUM_SHIPS];
    sdword techStates[WARP_TECH_STATE_COUNT];
} WarpScriptLevelData;

real32 BaseFleetStrengthForLevel[NUMBER_SINGLEPLAYER_MISSIONS+1];
real32 AdvancedFleetStrengthForLevel[NUMBER_SINGLEPLAYER_MISSIONS+1];
static WarpScriptLevelData warpScriptLevels[NUMBER_SINGLEPLAYER_MISSIONS];
static sdword warpScriptCurrentLevel = -1;
static sdword warpScriptEnabled = 0;
static sdword warpScriptLevelToWarpTo = 0;

bool32 triggerNIS = FALSE;

sdword warpLevel = 0;
sdword cmdNewMission = -1;  // set by /new N command-line flag

bool32 spBinkPlay;

extern fonthandle selGroupFont0;
extern sdword g_GameDifficulty;
extern sdword kasfMothershipBigGunDisabled;

void sub_652EF0(void);
void Init_WaitForIntelEventEnd(void);
void Begin_Tutorial1_PlayCurrentLessonSpeech(void);
void Init_Mission10_MineLayingTeam3_PostLaying9Delay(void);
void Watch_Mission10_MineLayingTeam3_PostLaying9Delay(void);
void Init_Mission07_Events_HyperdriveFailDelay(void);
void Watch_Mission07_Events_HyperdriveFail(void);

// state of kas debug overlays
#if SP_DEBUGLEVEL2
extern sdword mrKASDebugDrawStates;
extern sdword mrKASDebugDrawTimers;
extern sdword mrKASDebugDrawVars;
extern sdword mrKASDebugDrawVolumes;
extern sdword mrKASSkipFactor;
#endif


bool32 singlePlayerGameLoadNewLevelFlag;
bool32 singlePlayerSuppressHorseRaceLoadScreen = FALSE;
static bool32 singlePlayerPreserveCurrentMissionDuringLoad = FALSE;
static bool32 singlePlayerMissionStartedDuringLoad = FALSE;
static sdword spManagerScreenFlags = 0;       // mrManagerScreenBegin flags (dword_A6ADB0)
static sdword spScreenLockoutActive = 0;      // SP UI lockout active flag (dword_A5B1CC)
real32 cat_mrBlackoutEndTime = 0.0f;          // wall-clock time when blackout sequence ends (dword_A6ADB4)

static const real32 cat_mrBlackoutDelay = 5.0f; // fixed delay for widescreen hyperspace blackout (dword_72BCB4)

BOOL isSinglePlayerCampaignGame(void)
{
    return singlePlayerGame && singlePlayerGameInfo.currentMission != MISSION_ENUM_NOT_INITIALISED;
}

static sdword spHyperspaceWideScreenTable[NUMBER_SINGLEPLAYER_MISSIONS] =
{
    0,  // MISSION_ENUM_NOT_INITIALISED
    1,  // MISSION_1_KHARAK_SYSTEM
    1,  // MISSION_2_OUTSKIRTS_OF_KHARAK_SYSTEM
    0,  // MISSION_3_RETURN_TO_KHARAK
    0,  // MISSION_4_GREAT_WASTELANDS_TRADERS
    1,  // MISSION_5_GREAT_WASTELANDS_REVENGE
    0,  // MISSION_6_DIAMOND_SHOALS
    1,  // MISSION_7_THE_GARDENS_OF_KADESH
    0,  // MISSION_8_THE_CATHEDRAL_OF_KADESH
    0,  // MISSION_9_SEA_OF_LOST_SOULS
    0,  // MISSION_10_SUPER_NOVA_STATION
    0,  // MISSION_11_TENHAUSER_GATE
    0,  // MISSION_12_GALACTIC_CORE
    0,  // MISSION_13_THE_KAROS_GRAVEYARD
    0,  // MISSION_14_BRIDGE_OF_SIGHS
    1,  // MISSION_15_CHAPEL_PERILOUS
    0,  // MISSION_16_HIIGARA
    1,  // MISSION_5B_TURANIC_RAIDER_PLANETOID
    0,  // MISSION_TUTORIAL
};
bool32 spPostLoadNisBlackout = FALSE;         // Mission 1 keeps the screen black between chained post-load NIS segments
bool32 spPreservePostLoadNisMode = FALSE;     // Retail post-load NIS preserve flag (dword_A4CACC)
static bool32 spPostLoadArrivalPrepassPending = FALSE;
static sdword catMrBlackOutReset = -1;
static real32 catMrBlackOutLastTime = 0.0f;
static bool32 spPostLoadArrivalWhiteoutPrimed = FALSE;
static real32 spPostLoadArrivalWhiteoutLastTime = 0.0f;
static real32 spPostLoadArrivalWhiteoutFadeStartTime = 0.0f;
static real32 whiteoutFadeDelay = 3.0f;
SinglePlayerGameInfo singlePlayerGameInfo;

#define SINGLEPLAYERGAMEINFO_SAVE_SIZE offsetof(SinglePlayerGameInfo, ShipsInHyperspace)

real32 HYPERSPACE_SLICE_RATE = 0.075f;

bool32 HYPERSPACE_MOTHERSHIP_ALWAYS_ARRIVES_FIRST = TRUE;
real32 HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME = 5.0f;
real32 HYPERSPACE_BASE_WAITTIME = 3.0f;
real32 HYPERSPACE_BASE_ARRIVETIME = 2.0f;
real32 HYPERSPACE_BASE_MOTHERSHIPARRIVETIME = 1.0f;
real32 HYPERSPACE_RANDOM_WAITTIME = 0.2f;
real32 HYPERSPACE_WAITTIME_PER_DISTANCE = 0.0005f;
real32 HYPERSPACE_MINWAITTIME_DISTANCE = -1.0f;
real32 HYPERSPACE_MAXWAITTIME_DISTANCE = 1.0f;
real32 HYPERSPACE_ACCELERATE_TIME = 3.0f;
real32 HYPERSPACE_ACCELERATION = 1000.0f;
real32 HYPERSPACE_WHIP_VELOCITY = 50000.0f;
real32 HYPERSPACE_HEADINGACCURACY = 0.98f;
real32 HYPERSPACE_FARCLIPPLANE = 100000.0f;
real32 HYPERSPACE_WHITEOUTTIME = 0.2f;
real32 HYPERSPACE_FUNNEL_WIDTH = 5000.0f;
real32 HYPERSPACE_FUNNEL_WIDTH_TO_BACK_FACTOR = 0.5f;

real32 HYPERSPACE_FARCLIPPLANESQR;

real32 HYPERSPACE_DECELERATE_TIME = 3.0f;
real32 HYPERSPACE_DECELERATION = 10000.0f;

real32 HYPERSPACE_DECELERATE_DIST;
real32 HYPERSPACE_DECELERATE_INITIAL_VEL;
real32 HYPERSPACE_WHIPTOWARDSDIST;

real32 HYPERSPACE_TRAVELDIST_PER_RU_SPENT = 10000.0f;

sdword SINGLEPLAYER_STARTINGRUS = 200;
sdword SINGLEPLAYER_DEBUGLEVEL = 0;

real32 HS_CLIPT_NEG_THRESH = -0.4f;
real32 HS_CLIPT_NEG_SCALAR = 0.33f;
real32 HS_CLIPT_NEG_FINISH = 0.9f;
real32 HS_CLIPT_POS_THRESH = 0.5f;
real32 HS_CLIPT_POS_SCALAR = 0.5f;
real32 HS_CLIPT_POS_START  = 1.0f;

real32 HYPERSPACEGATE_HEALTH = 20000.0f;
real32 HYPERSPACEGATE_WIDTH = 1000.0f;
real32 HYPERSPACEGATE_HEIGHT = 1000.0f;
real32 HYPERSPACEGATE_WAYPOINTDIST = 800.0f;

real32 HYPERSPACE_RANGE = (3000.0f*3000.0f);
real32 spHyperspaceDelay = 0.0f;

real32 SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE = 50000.0f;

real32 SUBMESSAGE_SAFETY_TIMEOUT = 15.0f;

real32 SINGLEPLAYER_BOBBIGGESTRADIUS_LEVEL6 = 40000.0f;

void tmTechInit(void);

static void SetBaseFleetStrCB(char *directory,char *field,void *dataToFillIn);
void CleanupHyperspacingShips(bool32 FreeShips,bool32 cleanupAllPlayersShips);
void singlePlayerLaunchDockedShips(Ship *ship);
void singlePlayerKasMissionStart(sdword missionnum);
void spClearScreenLockout(void);

scriptEntry SinglePlayerTweaks[] =
{
    makeEntry(HS_CLIPT_NEG_THRESH, scriptSetReal32CB),
    makeEntry(HS_CLIPT_NEG_SCALAR, scriptSetReal32CB),
    makeEntry(HS_CLIPT_NEG_FINISH, scriptSetReal32CB),
    makeEntry(HS_CLIPT_POS_THRESH, scriptSetReal32CB),
    makeEntry(HS_CLIPT_POS_SCALAR, scriptSetReal32CB),
    makeEntry(HS_CLIPT_POS_START,  scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_HEALTH, scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_WIDTH , scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_HEIGHT, scriptSetReal32CB),
    makeEntry(HYPERSPACEGATE_WAYPOINTDIST, scriptSetReal32CB),
    makeEntry(HYPERSPACE_SLICE_RATE, scriptSetReal32CB),
    makeEntry(HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_MOTHERSHIP_ALWAYS_ARRIVES_FIRST,scriptSetBool),
    makeEntry(HYPERSPACE_BASE_WAITTIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_BASE_ARRIVETIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_BASE_MOTHERSHIPARRIVETIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_RANDOM_WAITTIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_WAITTIME_PER_DISTANCE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_MINWAITTIME_DISTANCE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_MAXWAITTIME_DISTANCE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_ACCELERATE_TIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_ACCELERATION,scriptSetReal32CB),
    makeEntry(HYPERSPACE_WHIP_VELOCITY,scriptSetReal32CB),
    makeEntry(HYPERSPACE_FARCLIPPLANE,scriptSetReal32CB),
    makeEntry(HYPERSPACE_HEADINGACCURACY,scriptSetCosAngCB),
    makeEntry(HYPERSPACE_WHITEOUTTIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_DECELERATE_TIME,scriptSetReal32CB),
    makeEntry(HYPERSPACE_DECELERATION,scriptSetReal32CB),
    makeEntry(HYPERSPACE_TRAVELDIST_PER_RU_SPENT,scriptSetReal32CB),
    makeEntry(SINGLEPLAYER_STARTINGRUS,scriptSetSdwordCB),
    makeEntry(SINGLEPLAYER_DEBUGLEVEL,scriptSetSdwordCB),
    makeEntry(HYPERSPACE_RANGE,scriptSetReal32SqrCB),
    makeEntry(SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE,scriptSetReal32CB),
    makeEntry(SUBMESSAGE_SAFETY_TIMEOUT,scriptSetReal32CB),
    makeEntry(SINGLEPLAYER_BOBBIGGESTRADIUS_LEVEL6,scriptSetReal32CB),
    { "BaseFleetStrengthForLevel",SetBaseFleetStrCB,NULL },
    endEntry
};

static void SetWarpFleetCB(char *directory,char *field,void *dataToFillIn);
static void SetWarpLevelCB(char *directory,char *field,void *dataToFillIn);
static void SetWarpResourcesCB(char *directory,char *field,void *dataToFillIn);
static void SetWarpTechCB(char *directory,char *field,void *dataToFillIn);

scriptEntry WarpScriptTable[] =
{
    { "WarpEnabled", scriptSetSdwordCB, &warpScriptEnabled },
    { "LevelToWarpTo", scriptSetSdwordCB, &warpScriptLevelToWarpTo },
    { "Level", SetWarpLevelCB, NULL },
    { "Resources", SetWarpResourcesCB, NULL},
    { "Ships", SetWarpFleetCB, NULL },
    { "Tech", SetWarpTechCB, NULL },
    endEntry
};

static sdword WarpToLevelEnabled();
static void SetMissionStartEnemyRUs(char *directory,char *field,void *dataToFillIn);
static void SetOnMissionCompleteInfo(char *directory,char *field,void *dataToFillIn);
void singlePlayerKasMissionStart(sdword missionnum);
void singlePlayerStartNis(char *nis, char *script, hvector *positionAndAngle, bool32 loadLevelBeforeStart, bool32 suppressMissionAdvance);
Node *singlePlayerCullHyperspaceShips(void);
sdword spUpdateShipHyperspace(Ship *ship, hvector *mothershipPos, bool32 instantComplete);
void tutDeallocateRootRegion(void);
void MaintainTeamContext(void);
void sub_652EF0(void);
void sub_60C010(void);
void Watch_Mission09_MissionFlow_GhostIntro(void);
void Watch_Mission09_MissionFlow_SelectNearbyAttackersOrQueueStandardFrigateBatch(void);
void Init_Mission10_AsteroidPocketAttackTeam_Start(void);
void Init_Mission10_AsteroidPocketEncounter_Start(void);
void Init_Mission10_AsteroidPocketEncounter_FocusTarget(void);
void Init_Mission10_ProximitySensorObjective_Start(void);
void Init_Mission10_ProximitySensorObjective_WaitForTrigger(void);
void Init_ApplyUpgradesThenDockForLaunch(void);
void Init_DockCurrentTeamForLaunch(void);
void Init_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor(void);
void Init_AsteroidPocketEncounter_DefenseLoop(void);
void Init_AsteroidPocketEncounter_InitialAttackerWarning(void);
void Init_AsteroidPocketEncounter_SelectInitialAttackers(void);
void Init_AsteroidPocketEncounter_ReinforcementDelay(void);
void Init_QueueHeavyInterceptorBatch(void);
void Init_AsteroidPocketEncounter_ClearCurrentSelectionHighlight(void);
void Watch_HeavyInterceptorBatchComplete(void);
void Init_SelectAndFocusProximitySensors(void);
void Watch_ProximitySensorContactLoop(void);
void Watch_Mission10_AsteroidPocketEncounter_AfterFocus(void);
void Watch_Mission10_AsteroidPocketAttackTeam_SelectAttackers(void);
void Watch_Mission10_AsteroidPocketAttackTeam_UpdateAttackers(void);
void Watch_Mission10_AsteroidPocketAttackTeam_Intro(void);
void Watch_Mission10_AsteroidPocketEncounter_SelectAttackers(void);
void Watch_Mission10_AsteroidPocketEncounter_UpdateAttackers(void);
void Watch_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady(void);
void Watch_AsteroidPocketEncounter_ResumeAttackers(void);
void Watch_AsteroidPocketEncounter_ThreatWarningAcknowledged(void);
void Watch_AsteroidPocketEncounter_DefenseLoop(void);
void Watch_AsteroidPocketEncounter_ReinforcementDelay(void);
void Watch_AsteroidPocketEncounter_ReinforcementReady(void);
void Watch_AsteroidPocketEncounter_BeginDefenseLoop(void);
void Watch_AsteroidPocketEncounter_InitialAttackerWarningAcknowledged(void);
void Watch_Mission10_MissionFlow_FirstWarning(void);
void Watch_Mission10_MissionFlow_FirstWarningDelay(void);
void Watch_Mission10_MissionFlow_AttackInClawFormation(void);
void Watch_Mission10_MissionFlow_FleetCommandIntro(void);
void Watch_Mission10_MissionFlow_SelectNearbyAttackersOrQueueReinforcements(void);
sdword Watch_Tutorial1_Tutorial10_CameraZoomIn(void);
void Watch_Mission13_MissionFlow_ComOpenSensors(void);
sdword Watch_Mission13_MissionFlow_ComOpenSensorsWait(void);
sdword Mission02ShipsSelect(GrowSelection *dest, GrowSelection *src);
void cpHueSatImageDelete(void);
void singlePlayerNISNamesGet(char **nisname, char **scriptname, bool32 *loadLevelBeforeStart, sdword nisNumber);

//static bool32 spSavedCameraYet;
//static Camera spSavedCamera;

scriptEntry SinglePlayerScriptTable[] =
{
    { "onMissionComplete", SetOnMissionCompleteInfo, &singlePlayerGameInfo },
    { "onMissionLoad", scriptSetSdwordCB, &singlePlayerGameInfo.onLoadPlayNIS},
    { "MissionStartEnemyRUs", scriptSetSdwordCB, &universe.players[1].resourceUnits },
    { "MissionStartEnemyRace", scriptSetShipRaceCB, &universe.players[1].race },
    { "Asteroid0sCanMove", scriptSetBool, &singlePlayerGameInfo.asteroid0sCanMove },
    endEntry
};

fibfileheader *spHyperspaceRollCallHandle = NULL;
regionhandle spHyperspaceRollCallScreen = NULL;

//taskbutton *hyperspacetbbutton = NULL;

void spAbortHyperspaceCB(char *string, featom *atom);
void spGoNowHyperspaceCB(char *string, featom *atom);
void spShipsRemainingDrawCB(featom *atom, regionhandle region);
void spShipsDockedDrawCB(featom *atom, regionhandle region);

void singlePlayerLoadNewLevel(bool32 loadNISLevel, bool32 suppressMissionAdvance);
SelectCommand *spBuildFleetSelection(void);
void spHyperspaceRollCallWideScreenIn(sdword frames);
void spCleanupScreenStack(void);

fecallback spHyperspaceCallback[] =
{
    {spAbortHyperspaceCB,  "AbortHyperspace"},
    {spGoNowHyperspaceCB,  "GoNowHyperspace"},
    {NULL, NULL}
};

fedrawcallback spHyperspaceDrawCallback[] =
{
    {spShipsRemainingDrawCB, "ShipsRemainingDraw"},
    {spShipsDockedDrawCB,    "ShipsDockedDraw"},
    {NULL, NULL}
};

bool32 GetPointOfName(hvector *point,char *name)
{
    hvector *vectptr = kasVectorPtrIfExists(kasHashLabel(name));

    if (vectptr != NULL)
    {
        *point = *vectptr;
        return TRUE;
    }
    else
    {
        point->x = 0.0f;   point->y = 0.0f;   point->z = 0.0f;   point->w = 0.0f;
        return FALSE;
    }
}

bool32 GetStartPointPlayer(hvector *startpoint)
{
    return GetPointOfName(startpoint,"StartPointPlayer");
}

static void spGetStartPointPlay(hvector *startpoint)
{
    hvector *playStartPoint = kasVectorPtrIfExists(kasHashLabel("StartPointPlayer"));

    if (playStartPoint != NULL)
    {
        *startpoint = *playStartPoint;
    }
    else
    {
        startpoint->x = 0.0f;
        startpoint->y = 0.0f;
        startpoint->z = 0.0f;
        startpoint->w = 0.0f;
    }
}

void SetBaseFleetStrCB(char *directory,char *field,void *dataToFillIn)
{
    sdword level;
    real32 basestrength;
    real32 advancedstrength;

    RemoveCommasFromString(field);

    sscanf(field,"%d %f %f",&level,&basestrength,&advancedstrength);

    dbgAssert(level <= NUMBER_SINGLEPLAYER_MISSIONS);

    BaseFleetStrengthForLevel[level] = basestrength;
    AdvancedFleetStrengthForLevel[level] = advancedstrength;
}

real32 GetStrengthOfShip(Ship *ship)
{
    // for now, just return RU cost of ship:
    if (ShipTypeIsMothershipType(ship->shiptype))
    {
        return 0.0f;
    }
    return (real32)ship->staticinfo->buildCost;
}

real32 GetStrengthOfShipStatic(ShipStaticInfo *shipstatic)
{
    // for now, just return RU cost of ship:
    if (ShipTypeIsMothershipType(shipstatic->shiptype))
    {
        return 0.0f;
    }
    return (real32)shipstatic->buildCost;
}

real32 GetStrengthOfRUs(sdword RUs)
{
    return (real32)RUs;
}

typedef void (*GetStrengthOfFleet)(Player *player);

/*-----------------------------------------------------------------------------
    Name        : GetFleetStrengthHyperspace
    Description : Returns fleet strength of ships in hyperspace of player
    Inputs      :
    Outputs     :
    Return      : Returns fleet strength of ships in hyperspace of player
----------------------------------------------------------------------------*/
void GetFleetStrengthHyperspace(Player *player)
{
    // find total strength of fleet in hyperspace
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    InsideShip *insideShip;
    Ship *ship;

    spFleetStr = 0.0f;
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            spFleetStr += GetStrengthOfShip(ship);
            // also check ships inside it:

            if ((ship->shipsInsideMe != NULL) && (ship->shiptype != tDFGFrigate))
            {
                Node *insidenode = ship->shipsInsideMe->insideList.head;
                InsideShip *insideship2;

                while (insidenode != NULL)
                {
                    insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
                    dbgAssert(insideship2->ship->objtype == OBJ_ShipType);

                    spFleetStr += GetStrengthOfShip(insideship2->ship);

                    insidenode = insidenode->next;
                }
            }
        }

        node = node->next;
    }

    spFleetStr += GetStrengthOfRUs(player->resourceUnits);
}


/*-----------------------------------------------------------------------------
    Name        : GetFleetStrengthInUniverseOfPlayer
    Description : Returns fleet strength of ships of player in universe
    Inputs      :
    Outputs     :
    Return      : Returns fleet strength of ships of player in universe
----------------------------------------------------------------------------*/
void GetFleetStrengthInUniverseOfPlayer(Player *player)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    spFleetStr = 0.0f;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            spFleetStr += GetStrengthOfShip(ship);

            // also check ships inside it:

            if ((ship->shipsInsideMe != NULL) && (ship->shiptype != tDFGFrigate))
            {
                Node *insidenode = ship->shipsInsideMe->insideList.head;
                InsideShip *insideship2;

                while (insidenode != NULL)
                {
                    insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
                    dbgAssert(insideship2->ship->objtype == OBJ_ShipType);

                    spFleetStr += GetStrengthOfShip(insideship2->ship);

                    insidenode = insidenode->next;
                }
            }
        }

        objnode = objnode->next;
    }

    spFleetStr += GetStrengthOfRUs(player->resourceUnits);
}

void GetFleetStrengthWarpScript(Player *player)
{
    WarpScriptLevelData *levelData;
    ShipType shiptype;
    ShipStaticInfo *shipstatic;

    spFleetStr = 0.0f;
    if (!warpLevel)
    {
        return;
    }

    levelData = &warpScriptLevels[warpLevel - 1];
    spFleetStr += GetStrengthOfRUs(levelData->resources);

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        if (levelData->shipCounts[shiptype] <= 0)
        {
            continue;
        }

        shipstatic = GetShipStaticInfo(shiptype, universe.players[0].race);
        dbgAssert(bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded));
        spFleetStr += GetStrengthOfShipStatic(shipstatic) * levelData->shipCounts[shiptype];
    }
}

void singlePlayerPreLoadCheck(void)
{
    WarpScriptLevelData *levelData;
    ShipType shiptype;

    if (warpLevel)
    {
        levelData = &warpScriptLevels[warpLevel - 1];
        for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
        {
            if (levelData->shipCounts[shiptype] > 0)
            {
                SetInfoNeededForShipAndRelatedStaticInfo(shiptype, universe.players[0].race, TRUE);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SetFleetModifier(sdword level,GetStrengthOfFleet getStrengthOfFleetFunc)
{
    real32 comparestrength,comparestrength2;

    comparestrength = BaseFleetStrengthForLevel[level];
    comparestrength2 = AdvancedFleetStrengthForLevel[level];

    if ((comparestrength == 0.0f) || (comparestrength2 <= comparestrength))
    {
        spFleetModifier = 0.0f;
        return;
    }

    getStrengthOfFleetFunc(&universe.players[0]);

    // set spFleetModifier to 0 to 1 based on spFleetStr commpared to comparestrength and comparestrength2

    if (spFleetStr <= comparestrength)
    {
        spFleetModifier = 0.0f;
        return;
    }
    else if (spFleetStr >= comparestrength2)
    {
        spFleetModifier = 1.0f;
        return;
    }
    else
    {
        spFleetModifier = (spFleetStr - comparestrength) / (comparestrength2 - comparestrength);
        if (spFleetModifier > 1.0f)
        {
            spFleetModifier = 1.0f;
        }
        else if (spFleetModifier < 0.0f)
        {
            spFleetModifier = 0.0f;
        }
    }
}

// this function doesn't need to be a function, its only called in one place
// and it changes the state variable of the case statement its called from, very confusing
// and its only 4 lines - 07/02/99 SA
//void spHyperspaceCommence()
//{
    //FocusCommand focusone;

//    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSLEAVING;
//    singlePlayerGameInfo.hyperspaceSubState = FOCUSING_ON_MOTHERSHIP;
//    dbgAssert(universe.curPlayerPtr->PlayerMothership != NULL);
//    focusone.numShips = 1;
//    focusone.ShipPtr[0] = universe.curPlayerPtr->PlayerMothership;
//    ccFocus(&universe.mainCameraCommand,&focusone);
//    singlePlayerGameInfo.hyperspaceTimeStamp = universe.totaltimeelapsed; // + HYPERSPACE_WAIT_FOR_MOTHERSHIPFOCUS_TIME;

//    speechEventFleet(STAT_F_Hyper_TakingOff, 0, universe.curPlayerIndex);
//}

void TellShipsToAbortDockForHyperspace()
{
    SelectCommand *selectAll;

    selectAll = selectAllCurrentPlayersNonHyperspacingShips();
    clHalt(&universe.mainCommandLayer,selectAll);
    memFree(selectAll);
}

void spAbortHyperspaceCB(char *string, featom *atom)
{
    if (singlePlayerGameInfo.hyperspaceState == HYPERSPACE_WAITINGROLLCALL)
    {
        SelectCommand *selection;

        // Let's abort hyperspace
        speechEventFleet(COMM_F_Hyper_Abort, 0, universe.curPlayerIndex);

        // give player refund on RU's
//        universe.curPlayerPtr->resourceUnits += singlePlayerGameInfo.RUNeededForNextHyperSpaceJump;
        clAutoLaunch(singlePlayerGameInfo.oldAutoLaunch,universe.curPlayerIndex);
        singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;

        selection = spBuildFleetSelection();
        clHalt(&universe.mainCommandLayer, selection);
        memFree(selection);

        feScreenDeleteFlags(spHyperspaceRollCallScreen, 0);
        spHyperspaceRollCallScreen = NULL;
    }
}

void spGoNowHyperspaceCB(char *string, featom *atom)
{
   if (singlePlayerGameInfo.hyperspaceState == HYPERSPACE_WAITINGROLLCALL)
   {
       SelectCommand *selection;
       SelectCommand *selectAll;
       SelectCommand selectone;
       CommandToDo *militaryGroup;
       sdword i;
       Ship *ship;
       Ship *mothership = universe.curPlayerPtr->PlayerMothership;

       dbgAssert(mothership);

       spHyperspaceRollCallWideScreenIn(90);

       selection = spBuildFleetSelection();
       clDock(&universe.mainCommandLayer, selection, DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH, NULL);
       memFree(selection);

       selectAll = selectAllCurrentPlayersHyperspacingShips();
       for (i=0;i<selectAll->numShips;i++)
       {
           ship = selectAll->ShipPtr[i];
           if (ship->staticinfo->canBuildShips)
           {
               selectone.ShipPtr[0] = ship;
               selectone.numShips = 1;
               RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selectone);
               aitrackForceSteadyShip(ship);
               GroupShipIntoMilitaryParade(&universe.mainCommandLayer, ship, mothership);
               ship->specialFlags1C8 |= 0x8080;
           }
       }
       memFree(selectAll);
       militaryGroup = GetMilitaryGroupAroundShip(&universe.mainCommandLayer,universe.curPlayerPtr->PlayerMothership);
       if (militaryGroup) setMilitaryParade(militaryGroup);

       feScreenDeleteFlags(spHyperspaceRollCallScreen, 0);
       spHyperspaceRollCallScreen = NULL;
   }
}

void spShipsRemainingDrawCB(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char numstr[10];

    sprintf(numstr,"%d",singlePlayerGameInfo.rollcallShipsRemaining);

    fhSave = fontCurrentGet();                              //save the current font
    fontMakeCurrent(selGroupFont2);             //select the appropriate font
    fontPrintCentreCentreRectangle(&region->rect,colWhite,numstr);
    fontMakeCurrent(fhSave);
}

void spShipsDockedDrawCB(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char numstr[10];

    sprintf(numstr,"%d",singlePlayerGameInfo.rollcallShipsDocked);

    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);             //select the appropriate font
    fontPrintCentreCentreRectangle(&region->rect,colWhite,numstr);
    fontMakeCurrent(fhSave);
}

void CalculateRollCall()
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    singlePlayerGameInfo.rollcallShipsDocked = 0;
    singlePlayerGameInfo.rollcallShipsRemaining = 0;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && (ship->shiptype != CryoTray) && !ShipTypeIsProximitySensorType(ship->shiptype) && !(ship->flags & SOF_Disabled))
        {
            if (ship->shipsInsideMe != NULL)
            {
                if (ship->shiptype != tDFGFrigate)
                {
                    singlePlayerGameInfo.rollcallShipsDocked += ship->shipsInsideMe->insideList.num;
                }
            }

            if (!ShipCanHyperspace(ship))
            {
                singlePlayerGameInfo.rollcallShipsRemaining++;
            }
            else if (!ShipTypeIsMothershipType(ship->shiptype))  // non-mothership hyperspacecapable ships must get in range
            {
                // is ship in range?
                if (ship->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange)
                {
                    singlePlayerGameInfo.rollcallShipsDocked++;
                }
                else
                {
                    singlePlayerGameInfo.rollcallShipsRemaining++;
                }
            }

        }
        objnode = objnode->next;
    }


}

void TellShipsToDockForHyperspace()
{
    Node *objnode;
    Ship *ship;
    SelectCommand *selectAll;

    selectAll = selectAllCurrentPlayersNonHyperspacingShips();
    if (clDock(&universe.mainCommandLayer,selectAll,DOCK_PERMANENTLY,NULL) > 0)
    {
        // moved this here cause I don't want it said if no ships are docking
        speechEventFleet(STAT_F_Hyper_Autodock, 0, universe.curPlayerIndex);
    }
    memFree(selectAll);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ship->shiptype == tDFGFrigate)
            {
                DDDFrigateMakeReadyForHyperspace(ship);
            }
        }

        objnode = objnode->next;
    }
}

bool32 AreNotInParadeConstraint(Ship *ship)
{
    if (ship->specialFlags & SPECIAL_SinglePlayerInParade)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
void MakeShipsNotIncludeLaunchingShips(SelectCommand *selection)
{
    sdword i;
    CommandToDo *shipcommand;
    for(i=0;i<selection->numShips;)
    {
        if(ShipCanHyperspace(selection->ShipPtr[i]))
        {
            //ship can hyperspace
            shipcommand = getShipAndItsCommand(&universe.mainCommandLayer,selection->ShipPtr[i]);
            if(shipcommand != NULL)
            {
                if(shipcommand->ordertype.order == COMMAND_LAUNCHSHIP)
                {
                    //launching ship can hyperspace...lets not cancel its
                    //orders
                    selection->numShips--;
                    selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
                    continue;
                }
            }
        }
        i++;
    }

}
void TellHyperspacingShipsToReady()
{
    SelectCommand *selectAll;
    sdword i;
    Ship *ship;
    ShipStaticInfo *staticinfo;
    CommandToDo *shipcommand;

    selectAll = selectAllCurrentPlayersHyperspacingShips();
    for (i=0;i<selectAll->numShips;i++)
    {
        ship = selectAll->ShipPtr[i];
        staticinfo = ship->staticinfo;

        if (staticinfo->shipclass == CLASS_Mothership)
        {
            ship->specialFlags1C8 |= 0x80;
        }
        else
        {
            if (ship->shiptype == tResourceCollector)
            {
                ship->health = staticinfo->maxhealth;
            }

            if (shipInMilitaryParade(ship))
            {
                ship->specialFlags1C8 |= 0x8080;
            }
            else
            {
                ship->specialFlags1C8 &= ~0x80;
            }
        }
    }

    for (i=0;i<selectAll->numShips;)
    {
        ship = selectAll->ShipPtr[i];
        staticinfo = ship->staticinfo;
        if (((udword)staticinfo->engineNozzleOffset[1].x != 0 || staticinfo->shipclass == CLASS_Component) &&
            ((shipcommand = getShipAndItsCommand(&universe.mainCommandLayer, ship)) != NULL) &&
            (shipcommand->ordertype.order == COMMAND_LAUNCHSHIP))
        {
            selectAll->numShips--;
            selectAll->ShipPtr[i] = selectAll->ShipPtr[selectAll->numShips];
            continue;
        }
        i++;
    }

    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,selectAll);
    clSetMilitaryParade(&universe.mainCommandLayer,selectAll);
    memFree(selectAll);
}

sdword HyperspaceRollCallBegin(regionhandle region, sdword ID, udword event, udword data)
{
    if (!spHyperspaceRollCallHandle)
    {
        feCallbackAddMultiple(spHyperspaceCallback);      //add in the callbacks
        feDrawCallbackAddMultiple(spHyperspaceDrawCallback);
        spHyperspaceRollCallHandle = feScreensLoad("FEMan\\Hyperspace_Roll_Call.fib");   //load in the screen
    }

    CalculateRollCall();
    spHyperspaceRollCallScreen = feScreenStart(region, "HyperspaceRollCall");

    return 0;
}

ShipSinglePlayerGameInfo *spNewShipSinglePlayerGameInfo()
{
    ShipSinglePlayerGameInfo *newone = memAlloc(sizeof(ShipSinglePlayerGameInfo),"shipsingleGame",0);
    memset(newone,0,sizeof(*newone));
    newone->shipHyperspaceState = SHIPHYPERSPACE_NONE;
    newone->midlevelHyperspaceIn = FALSE;
    newone->midlevelHyperspaceOut = FALSE;
    newone->staticHyperspaceGate = FALSE;
    return newone;
}

void CalculateHyperspaceTimeForShip(Ship *ship,Ship *mothership)
{
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
    vector mothershipheading;
    vector mothershipright;
    vector mothershipup;
    vector mothershipposition;
    vector mothershipToShip;

    if (mothership != NULL)
    {
        matGetVectFromMatrixCol3(mothershipheading,mothership->rotinfo.coordsys);
        matGetVectFromMatrixCol2(mothershipright,mothership->rotinfo.coordsys);
        matGetVectFromMatrixCol1(mothershipup,mothership->rotinfo.coordsys);
        mothershipposition = mothership->posinfo.position;
    }

    if ((mothership != NULL) && (ship != mothership))
    {
        vecSub(mothershipToShip,ship->posinfo.position,mothershipposition);
        shipSinglePlayerGameInfo->forbehind = vecDotProduct(mothershipheading,mothershipToShip);
        shipSinglePlayerGameInfo->rightleft = vecDotProduct(mothershipright,mothershipToShip);
        shipSinglePlayerGameInfo->updown = vecDotProduct(mothershipup,mothershipToShip);
    }
    else
    {
        shipSinglePlayerGameInfo->forbehind = 0.0f;
        shipSinglePlayerGameInfo->rightleft = 0.0f;
        shipSinglePlayerGameInfo->updown = 0.0f;
    }

    shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_WAITING;
    shipSinglePlayerGameInfo->hsState = HS_INACTIVE;
    shipSinglePlayerGameInfo->hsFlags = 0;

    if ((mothership == NULL) || (ship == mothership))
    {
        shipSinglePlayerGameInfo->timeToDelayHyperspace = 0.0f;
    }
    else
    {
        shipSinglePlayerGameInfo->timeToDelayHyperspace =
            HYPERSPACE_BASE_WAITTIME + frandom(HYPERSPACE_RANDOM_WAITTIME);
    }
    shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + shipSinglePlayerGameInfo->timeToDelayHyperspace;

    bitSet(ship->specialFlags,SPECIAL_Hyperspacing);
    shipHasJustDisappeared(ship);
}

void CalculateHyperspaceTimesForShips(void)
{
    Node *objnode;
    Ship *ship;
    Ship *mothership = universe.curPlayerPtr->PlayerMothership;

    dbgAssert(mothership != NULL);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanMakeHyperspace(ship))
            {
                CalculateHyperspaceTimeForShip(ship,mothership);
            }
        }

        objnode = objnode->next;
    }
}

void AddShipToHyperspace(Ship *ship)
{
    InsideShip *insideShip;
    sdword trailIndex;

    insideShip = memAlloc(sizeof(InsideShip),"InsideHyper",0);
    insideShip->ship = ship;
    listAddNode(&singlePlayerGameInfo.ShipsInHyperspace,&insideShip->node,insideShip);
    bitSet(ship->flags,SOF_DontDrawTrails);
    for (trailIndex = 0; trailIndex < MAX_NUM_TRAILS; trailIndex++)
    {
        if (ship->trail[trailIndex] != NULL)
        {
            trailZeroLength(ship->trail[trailIndex]);
        }
    }

    if(singlePlayerGame)
    {
        // ships in hyperspace are instantly healed/refueled:
        //ONLY in single player game!
        ship->health = ship->staticinfo->maxhealth;
        ship->fuel = ship->staticinfo->maxfuel;
    }

    // and any ships inside as well:
    if (ship->shipsInsideMe != NULL)
    {
        Node *insidenode = ship->shipsInsideMe->insideList.head;
        InsideShip *insideship2;
        Ship *iship;

        while (insidenode != NULL)
        {
            insideship2 = (InsideShip *)listGetStructOfNode(insidenode);
            iship = insideship2->ship;
            dbgAssert(iship->objtype == OBJ_ShipType);

            iship->health = iship->staticinfo->maxhealth;
            iship->fuel = iship->staticinfo->maxfuel;

            insidenode = insidenode->next;
        }
    }
}

bool32 NoneDoing(udword state)
{
    Node* objnode;
    Ship* ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship*)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship) && !(ship->flags & SOF_Disabled))
        {
            if (ship->shipSinglePlayerGameInfo->hsState == state)
            {
                return FALSE;
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

bool32 AllDoingExceptMe(udword state, Ship* me)
{
    Node* objnode;
    Ship* ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship*)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship) && !(ship->flags & SOF_Disabled))
        {
            if (ship != me)
            {
                if (ship->shipSinglePlayerGameInfo->hsState != state)
                {
                    return FALSE;
                }
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

bool32 NoneDoingHS(udword state)
{
    InsideShip* insideship;
    Node* objnode;
    Ship* ship;

    objnode = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (objnode != NULL)
    {
        insideship = (InsideShip*)listGetStructOfNode(objnode);
        ship = insideship->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship))
        {
            if (ship->shipSinglePlayerGameInfo->hsState == state)
            {
                return FALSE;
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

bool32 AllDoingExceptMeHS(udword state, Ship* me)
{
    InsideShip* insideship;
    Node* objnode;
    Ship* ship;

    objnode = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (objnode != NULL)
    {
        insideship = (InsideShip*)listGetStructOfNode(objnode);
        ship = insideship->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
            ShipCanMakeHyperspace(ship))
        {
            if (ship != me)
            {
                if (ship->shipSinglePlayerGameInfo->hsState != state)
                {
                    return FALSE;
                }
            }
        }

        objnode = objnode->next;
    }

    return TRUE;
}

void InstantlyHyperspaceOut(Ship *ship,Ship *mothership)
{
    CalculateHyperspaceTimeForShip(ship,mothership);

    //disappear ...
    ship->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
    if ((ship->flags & SOF_DontDrawTrails) == 0)        // extra safety for hyperspacing ships already hyperspacing
    {
        univRemoveShipFromOutside(ship);
        // ... into the hyperspace ether
        AddShipToHyperspace(ship);
    }
}

bool32 UpdateHyperspacingShip(Ship *ship,bool32 midLevel)
{
    bool32 hyperspacedout = FALSE;
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
    static sdword hsSFXHandle = -1;

    ship->autostabilizeship = FALSE;        // ship is doing something, don't want to autostabilize
    ship->shipidle = FALSE;

    shipSinglePlayerGameInfo->midlevelHyperspaceOut = midLevel;

    dbgAssert(shipSinglePlayerGameInfo);

    if ((singlePlayerGameInfo.hyperspaceFails) && (!midLevel))
    {
        // do effect state machine for hyperspace fizzling here
        switch (shipSinglePlayerGameInfo->hsState)
        {
            case HS_INACTIVE:
                if (universe.totaltimeelapsed >= shipSinglePlayerGameInfo->timeToHyperspace)
                {
                    shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_ACCELERATINGAWAY;
                    shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_ACCELERATE_TIME;
                    clRemoveShipFromSelection((SelectAnyCommand *)&selSelected,ship);
                    clRemoveShipFromSelection((SelectAnyCommand *)&selSelecting,ship);
//                    hyperspacedout = TRUE;
//                    shipSinglePlayerGameInfo->hsState = HS_FINISHED;
                    hsStart(ship, HYPERSPACE_SLICE_RATE, TRUE, hsShouldDisplayEffect(ship));
                    if (ship == universe.curPlayerPtr->PlayerMothership)
                    {
                        hsSFXHandle = soundEvent(ship, Ship_SinglePlayerHyperspace);
                    }
                }
                break;
            case HS_SLICING_INTO:
                if ( midLevel || (NoneDoing(HS_POPUP_INTO) && NoneDoing(HS_INACTIVE)) )
                {
                    shipSinglePlayerGameInfo->hsState = HS_COLLAPSE_INTO;
                }
                break;
            case HS_COLLAPSE_INTO:
                if (hsSFXHandle != SOUND_NOTINITED)
                {
                    if (ship == universe.curPlayerPtr->PlayerMothership)
                    {
                        soundEventStopSound(hsSFXHandle, 1);
                        hsSFXHandle = SOUND_NOTINITED;
                    }
                }
                hsUpdate(ship);
                break;
            case HS_FINISHED:
                hyperspacedout = TRUE;
                break;
            default:
                hsUpdate(ship);
                break;
        }
    }
    else
    switch (shipSinglePlayerGameInfo->hsState)
    {
    case HS_INACTIVE:
        if (universe.totaltimeelapsed >= shipSinglePlayerGameInfo->timeToHyperspace)
        {
            shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_ACCELERATINGAWAY;
            shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_ACCELERATE_TIME;
            clRemoveShipFromSelection((SelectAnyCommand *)&selSelected,ship);
            clRemoveShipFromSelection((SelectAnyCommand *)&selSelecting,ship);
            hsStart(ship, HYPERSPACE_SLICE_RATE, TRUE, hsShouldDisplayEffect(ship));
        }
        break;
    case HS_SLICING_INTO:
        if ( midLevel || (NoneDoing(HS_POPUP_INTO) && NoneDoing(HS_INACTIVE)) )
        {
            hsUpdate(ship);
        }
        break;
    case HS_COLLAPSE_INTO:
        if (ship->soundevent.hyperspaceHandle != SOUND_NOTINITED)
        {
            soundEventStopSound(ship->soundevent.hyperspaceHandle, 1);
            ship->soundevent.hyperspaceHandle = SOUND_NOTINITED;
        }
        if ( (!midLevel) && (ship == universe.curPlayerPtr->PlayerMothership))
        {
            if (mrWhiteOut || AllDoingExceptMe(HS_FINISHED, ship))
            {
                mrWhiteOut = TRUE;
                mrWhiteOutT = 1.0f - shipSinglePlayerGameInfo->clipt;
                hsUpdate(ship);
            }
        }
        else
        {
            hsUpdate(ship);
        }
        break;
    case HS_FINISHED:
        //disappear ...
        shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
        if ((ship->flags & SOF_DontDrawTrails) == 0)        // extra safety for hyperspacing ships already hyperspacing
        {
            univRemoveShipFromOutside(ship);

            if ((!midLevel) && (ship->playerowner != universe.curPlayerPtr))
            {
                ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
                univDeleteWipeInsideShipOutOfExistence(ship);   // kill non player ships that went along for the ride
            }
            else
            {
                // ... into the hyperspace ether
                AddShipToHyperspace(ship);
            }
        }
        hyperspacedout = TRUE;
        break;
    default:
        hsUpdate(ship);
    }

    return hyperspacedout;
}

bool32 UpdateHyperspacingShips(void)
{
    Node *objnode;
    Ship *ship;
    bool32 shipsAllHyperspacedAway = TRUE;
    bool32 result;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        objnode = objnode->next;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanMakeHyperspace(ship))
            {
                result = UpdateHyperspacingShip(ship,FALSE);
                if (singlePlayerGameInfo.hyperspaceFails)
                {
                    if (!result)
                    {
                        shipsAllHyperspacedAway = FALSE;
                    }
                }
                else
                {
                    shipsAllHyperspacedAway = FALSE;
                }
            }
        }
    }

    return shipsAllHyperspacedAway;
}

bool32 OrientHyperspacingShips(void)
{
    Node *objnode;
    Ship *ship;
    vector mothershipheading;
    bool32 shipsoriented = TRUE;
    Ship *mothership = universe.curPlayerPtr->PlayerMothership;

    dbgAssert(mothership != NULL);

    matGetVectFromMatrixCol3(mothershipheading,mothership->rotinfo.coordsys);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
       ship = (Ship *)listGetStructOfNode(objnode);
       dbgAssert(ship->objtype == OBJ_ShipType);

       if (allianceIsShipAlly(ship,universe.curPlayerPtr) &&
           (ship->staticinfo->engineNozzleOffset[1].x != 0.0f ||
            ship->staticinfo->shipclass == CLASS_Component))
       {
           if (ship->staticinfo->shipclass != CLASS_Mothership)
           {
               ship->autostabilizeship = FALSE;        // ship is doing something, don't want to autostabilize
               ship->shipidle = FALSE;
           }

           if (ship->staticinfo->shipclass != CLASS_Mothership &&
               ship->staticinfo->shipclass != CLASS_Component)
           {
               if (ship->specialFlags1C8 & 0x80)
               {
                   bool32 shipready;

                   shipready = aitrackHeadingFunc(
                       (SpaceObjRotImpTargGuidance *)ship,
                       &mothershipheading,
                       HYPERSPACE_HEADINGACCURACY,
                       HYPERSPACE_HEADINGACCURACY,
                       ship->staticinfo->sinbank,
                       0);
                   if (!shipready)
                   {
                       singlePlayerGameInfo.rollcallShipsRemaining++;
                   }
                   shipsoriented &= shipready;
               }
               else if (aiuFindDistanceSquared(ship->posinfo.position,mothership->posinfo.position) < HYPERSPACE_RANGE)
               {
                   CommandToDo *shipcommand = getShipAndItsCommand(&universe.mainCommandLayer,ship);

                   if (shipcommand == NULL ||
                       shipcommand->ordertype.order != COMMAND_LAUNCH_SHIP)
                   {
                       ship->specialFlags1C8 |= 0x80;
                   }
               }
               else
               {
                   aishipFlyToPointAvoidingObjsFunc(
                       ship,
                       &mothership->posinfo.position,
                       AISHIP_FirstPointInDirectionFlying |
                           AISHIP_PointInDirectionFlying |
                           AISHIP_DontFlyToObscuredPoints |
                           AISHIP_ReturnImmedIfPointObscured,
                       0.0f,
                       NULL);
               }
           }
       }

        objnode = objnode->next;
    }

    return shipsoriented;
}

void PutShipOutsideAt(Ship *ship,vector *createat,vector *createvelocity,real32 angle)
{
    sdword i;

    collAddSpaceObjToCollBlobs((SpaceObj *)ship);
    listAddNode(&universe.SpaceObjList,&(ship->objlink),ship);
    listAddNode(&universe.ShipList,&(ship->shiplink),ship);
    listAddNode(&universe.ImpactableList,&(ship->impactablelink),ship);
    bitClear(ship->flags, SOF_DockMarker);
    ship->posinfo.position = *createat;
    univUpdateShipProximity(ship);
    ship->posinfo.velocity = *createvelocity;
    ship->rotinfo.coordsys = defaultshipmatrix;

    univAddObjToRenderList((SpaceObj *)ship);

    univRotateObjYaw((SpaceObjRot *)ship,angle);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i])
        {
            trailZeroLength(ship->trail[i]);
        }
    }
}

void CalculateArrivingTimesForShips()
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    struct ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;

    dbgAssert(node != NULL);
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ShipTypeIsMothershipType(ship->shiptype) && (HYPERSPACE_MOTHERSHIP_ALWAYS_ARRIVES_FIRST))
        {
            shipSinglePlayerGameInfo->timeToDelayHyperspace = universe.totaltimeelapsed + HYPERSPACE_BASE_MOTHERSHIPARRIVETIME;
        }
        else
        {
            shipSinglePlayerGameInfo->timeToDelayHyperspace = universe.totaltimeelapsed + HYPERSPACE_BASE_ARRIVETIME + shipSinglePlayerGameInfo->timeToDelayHyperspace;
        }

        node = node->next;
    }
}

static bool32 spMissionUsesPostLoadArrivalPrepass(void)
{
    return missionUsesPostLoadArrivalPrepass[singlePlayerGameInfo.currentMission];
}

static void spUpdatePostLoadArrivalWhiteout(void)
{
    if (cat_mrWhiteOutT <= 0.0f)
    {
        cat_mrWhiteOutT = 0.0f;
        spPostLoadArrivalWhiteoutPrimed = FALSE;
        return;
    }

    if (!spPostLoadArrivalWhiteoutPrimed)
    {
        spPostLoadArrivalWhiteoutPrimed = TRUE;
        spPostLoadArrivalWhiteoutLastTime = universe.totaltimeelapsed;
        cat_mrWhiteOutT -= 0.0001f;
        spPostLoadArrivalWhiteoutFadeStartTime = universe.totaltimeelapsed + whiteoutFadeDelay;
    }

    if (universe.totaltimeelapsed > spPostLoadArrivalWhiteoutFadeStartTime)
    {
        cat_mrWhiteOutT -= (universe.totaltimeelapsed - spPostLoadArrivalWhiteoutLastTime) * 0.5f;
    }
    spPostLoadArrivalWhiteoutLastTime = universe.totaltimeelapsed;

    if (cat_mrWhiteOutT <= 0.0f)
    {
        cat_mrWhiteOutT = 0.0f;
        spPostLoadArrivalWhiteoutPrimed = FALSE;
    }
}

static void spSignalHyperspaceArrivalComplete(void)
{
    sdword entryId;
    sdword *entry;

    entryId = kasHashLabel("G_ShipsHaveArrived");
    entry = kasfEntryFindOrCreateByScopeAndId(0, entryId);
    dbgAssert(entry != NULL);
    entry[2] = TRUE;
}

static void spGroupPlayerFleetForMissionStart(void)
{
    Node *node;
    SelectCommand selectionCommand;
    Ship *playerMothership = universe.curPlayerPtr->PlayerMothership;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = (Ship *)listGetStructOfNode(node);

        if (ship->playerowner == universe.curPlayerPtr &&
            (ship->shiptype == dEcclesiam || ship->specialFlags3 == 0))
        {
            if (ship != playerMothership)
            {
                memset(&selectionCommand, 0, sizeof(selectionCommand));
                selectionCommand.numShips = 1;
                selectionCommand.ShipPtr[0] = ship;
                RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selectionCommand);
                aitrackForceSteadyShip(ship);
                GroupShipIntoMilitaryParade(&universe.mainCommandLayer, ship, playerMothership);
                bitSet(ship->specialFlags, SPECIAL_SinglePlayerInParade);
            }
            if (ship->shipsInsideMe != NULL)
            {
                singlePlayerLaunchDockedShips(ship);
            }
        }
    }
}

static void spRunPostLoadArrivalPrepass(void)
{
    hvector startPoint;
    real32 angle;
    matrix rotation;
    vector offset;
    vector createPosition;
    vector createVelocity;
    Node *node;

    spGetStartPointPlay(&startPoint);
    angle = DEG_TO_RAD(startPoint.w);
    vecSet(createVelocity, 0.0f, 0.0f, 0.0f);
    matMakeRotAboutZ(&rotation, (real32)cos(angle), (real32)sin(angle));

    node = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (node != NULL)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
        Ship *ship = insideShip->ship;
        Node *next = node->next;
        ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;

        dbgAssert(ship->objtype == OBJ_ShipType);
        shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;

        if (allianceIsShipAlly(ship, universe.curPlayerPtr))
        {
            vecSet(offset,
                   shipSinglePlayerGameInfo->forbehind,
                   -shipSinglePlayerGameInfo->rightleft,
                   shipSinglePlayerGameInfo->updown);
            matMultiplyMatByVec(&createPosition, &rotation, &offset);
            createPosition.x += startPoint.x;
            createPosition.y += startPoint.y;
            createPosition.z += startPoint.z;

            PutShipOutsideAt(ship, &createPosition, &createVelocity, angle);
            bitClear(ship->flags, SOF_DontDrawTrails);
            bitClear(ship->specialFlags,
                     SPECIAL_SinglePlayerWithinHyperSpaceRange |
                     SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse);
            shipSinglePlayerGameInfo->hyperspaceEffect = NULL;
            listDeleteNode(node);
        }

        node = next;
    }

    spGroupPlayerFleetForMissionStart();
    clPresetShipsToPosition(&universe.mainCommandLayer);
    ccFocusOnPlayersMothership(&universe.mainCameraCommand, 0);
    if (byte_8B8844)
    {
        ccSetDirtyBit(&pilotEffect);
    }

    cat_mrBlackOut = 1.0f;
    catMrBlackOutReset = 1;
}

static void spCompleteArrivingShips(void)
{
    Node *node;
    bool32 usedPostLoadArrivalPrepass = spMissionUsesPostLoadArrivalPrepass();

    singlePlayerHyperspacingInto = FALSE;
    singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;
    spSignalHyperspaceArrivalComplete();
    CleanupHyperspacingShips(FALSE, FALSE);

    spPostLoadArrivalPrepassPending = FALSE;
    spPostLoadArrivalWhiteoutPrimed = FALSE;

    spUnlockout();
    singlePlayerSetWideScreenSpeedScale(TRUE);

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = (Ship *)listGetStructOfNode(node);
        ModuleCompleted(ship->playerowner, ship->shiptype, FALSE);
    }

    if (usedPostLoadArrivalPrepass)
    {
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *ship = (Ship *)listGetStructOfNode(node);

            if (allianceIsShipAlly(ship, universe.curPlayerPtr) &&
                (ship->staticinfo->engineNozzleOffset[1].x != 0.0f ||
                 ship->staticinfo->shipclass == CLASS_Component) &&
                (ship->specialFlags1C8 & 0x80))
            {
                ship->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_NONE;
                ship->specialFlags1C8 &= ~0x80;
            }
        }
    }

    clAutoLaunch(singlePlayerGameInfo.oldAutoLaunch, universe.curPlayerIndex);
    LaunchAllInternalShipsOfPlayerThatMustBeLaunched(universe.curPlayerPtr);

    if (singlePlayerGameInfo.onLoadPlayNIS <= 0)
    {
        if (!singlePlayerMissionStartedDuringLoad)
        {
            soundEventPlayMusic(SongNumber);
            singlePlayerKasMissionStart(singlePlayerGameInfo.currentMission);
        }
    }
    aiplayerGameStart(universe.players[1].aiPlayer);
    singlePlayerMissionStartedDuringLoad = FALSE;
}

bool32 UpdateArrivingShip(Ship *ship,hvector *topoint,bool32 midLevel)
{
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
    bool32 thisShipArrived = FALSE;
    dbgAssert(shipSinglePlayerGameInfo);

    ship->autostabilizeship = FALSE;        // ship is doing something, don't want to autostabilize
    ship->shipidle = FALSE;

    switch (shipSinglePlayerGameInfo->shipHyperspaceState)
    {
        case SHIPHYPERSPACE_GONE:
            {
                real32 forbehind;
                real32 rightleft;
                real32 updown;
                vector tempvec;
                vector createposition;
                vector createvelocity;
                real32 angle;
                matrix rotzmat;

                // calculate position based on forbehind, rightleft
                forbehind = shipSinglePlayerGameInfo->forbehind;
                rightleft = shipSinglePlayerGameInfo->rightleft;
                updown = shipSinglePlayerGameInfo->updown;

                vecSet(tempvec,forbehind,-rightleft,updown);
                vecSet(createvelocity,0.0f,0.0f,0.0f);

                angle = DEG_TO_RAD(topoint->w);

                matMakeRotAboutZ(&rotzmat,(real32)cos(angle),(real32)sin(angle));
                matMultiplyMatByVec(&createposition,&rotzmat,&tempvec);

                vecAddTo(createposition,*topoint);

                PutShipOutsideAt(ship,&createposition,&createvelocity,angle);
                bitClear(ship->flags,SOF_DontDrawTrails);

                shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_DECELERATE;
                {
                    if (midLevel)
                    {
                        shipSinglePlayerGameInfo->timeToHyperspace = 0.0f;
                    }
                    else
                    {
                        real32 waittime;
                        if (ship == universe.curPlayerPtr->PlayerMothership)
                        {
                            waittime = 2.0f * HYPERSPACE_BASE_WAITTIME;
                        }
                        else
                        {
                            waittime = 3.0f * HYPERSPACE_BASE_WAITTIME + frandom(HYPERSPACE_RANDOM_WAITTIME);
                        }
                        shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + waittime;
                    }
                }

                if ((ship == universe.curPlayerPtr->PlayerMothership) && (!midLevel))
                {
                    //FocusCommand focusone;
                    //TransitionType trans;

                    //focusone.numShips = 1;
                    //focusone.ShipPtr[0] = ship;
                    //trans = universe.mainCameraCommand.transition;
                    //universe.mainCameraCommand.transition = CAMERA_SET;

                    //dbgAssert(spSavedCameraYet);
                    //ccFocus(&universe.mainCameraCommand, &focusone);
                    //universe.mainCameraCommand.currentCameraStack->remembercam = spSavedCamera;

                    //universe.mainCameraCommand.transition = trans;
                    ccFocusOnPlayersMothership(&universe.mainCameraCommand,0);
                }
            }
            if (!midLevel)
            {
                break;
            }
            // if midLevel, deliberately fall through

        case SHIPHYPERSPACE_DECELERATE:
            if (universe.totaltimeelapsed > shipSinglePlayerGameInfo->timeToHyperspace)
            {
                vecZeroVector(ship->posinfo.velocity);
                shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_DONE;
                shipSinglePlayerGameInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_DECELERATE_TIME;
                hsStart(ship, HYPERSPACE_SLICE_RATE, FALSE, hsShouldDisplayEffect(ship));
            }
            break;

        case SHIPHYPERSPACE_DONE:
            mrWhiteOutT -= 3.0f * HYPERSPACE_SLICE_RATE;
            if (mrWhiteOutT < 0.0f)
            {
                mrWhiteOut = FALSE;
            }
            switch (shipSinglePlayerGameInfo->hsState)
            {
            case HS_SLICING_OUTOF:
                if ( midLevel || (NoneDoingHS(HS_POPUP_OUTOF) && NoneDoingHS(HS_FINISHED)) )
                {
                    if(!singlePlayerGame)
                    {
                        CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
                        if(command != NULL)
                        {
                            if(command->ordertype.order == COMMAND_MP_HYPERSPACEING)
                            {
                                //if its a capital ship (docked ships won't have this command)
                                if(universe.totaltimeelapsed - command->hyperSpaceingTime < TW_HYPERSPACE_TELEGRAPH_WAIT_TIME)
                                {
                                    //ship hasn't waited long enough for incomming hs...
                                    //so don't
                                    goto noupdateonincomminghs;
                                }
                            }
                        }
                    }
                    hsUpdate(ship);
noupdateonincomminghs:;
                }
                break;
            case HS_COLLAPSE_OUTOF:
                if (ship->soundevent.hyperspaceHandle != SOUND_NOTINITED)
                {
                    soundEventStopSound(ship->soundevent.hyperspaceHandle, 1);
                    ship->soundevent.hyperspaceHandle = SOUND_NOTINITED;
                }
                if ((!midLevel) && (ship == universe.curPlayerPtr->PlayerMothership))
                {
                    if (AllDoingExceptMe(HS_INACTIVE, ship))
                    {
                        hsUpdate(ship);
                    }
                }
                else
                {
                    if(!singlePlayerGame)
                    {
                        if(shipOwnerHasActiveGravwell(ship))
                        {
                            //ship is stuck here because of a gravwell gen somewhere
                            goto noupdateHSMP;
                        }
                    }
                    hsUpdate(ship);
noupdateHSMP:;
                }
                break;
            case HS_INACTIVE:
                if (universe.totaltimeelapsed > shipSinglePlayerGameInfo->timeToHyperspace)
                {
                    thisShipArrived = TRUE;
                }
                break;
            default:
                hsUpdate(ship);
            }
            break;
    }

    if (thisShipArrived)
    {
        bitClear(ship->specialFlags,SPECIAL_Hyperspacing);
    }

    return thisShipArrived;
}

static bool32 spUpdateArrivingShips(bool32 requireTrackedShip)
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    bool32 allShipsArrived = TRUE;
    hvector startpoint;
    bool32 shipArrived;

    if (requireTrackedShip)
    {
        GetStartPointPlayer(&startpoint);
    }
    else
    {
        spGetStartPointPlay(&startpoint);
    }

    if (requireTrackedShip)
    {
        dbgAssert(node != NULL);
    }
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            shipArrived = spUpdateShipHyperspace(ship, &startpoint, FALSE);
            allShipsArrived &= shipArrived;
        }

        node = node->next;
    }

    return allShipsArrived;
}

static bool32 spUpdateArrivalState(void)
{
    bool32 allShipsArrived;
    if (spMissionUsesPostLoadArrivalPrepass())
    {
        if (spPostLoadArrivalPrepassPending)
        {
            spRunPostLoadArrivalPrepass();
            spPostLoadArrivalPrepassPending = FALSE;
            allShipsArrived = FALSE;
        }
        else
        {
            allShipsArrived = (cat_mrBlackOut <= 0.0f);
        }
    }
    else
    {
        spUpdatePostLoadArrivalWhiteout();
        allShipsArrived = spUpdateArrivingShips(FALSE);
    }
    return allShipsArrived;
}

bool32 UpdateArrivingShips()
{
    return spUpdateArrivingShips(TRUE);
}

void GetShipsClampedToInHS(Ship *clampedto,GrowSelection *clampedships)
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;

//    dbgAssert(node != NULL);
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->clampInfo)
        {
            if ((ship->clampInfo->host == (SpaceObjRotImpTarg *)clampedto)
                && ship->shipSinglePlayerGameInfo->midlevelHyperspaceIn != TRUE)
            {
                //add ship to the ships to hs in
                growSelectAddShip(clampedships,ship);
            }
        }
        node = node->next;
    }
}

static void hyperspaceDetachClampedShips(Ship *ship)
{
    sdword i;

    if (ship->clampInfo == NULL || ship->clampInfo->selection == NULL)
    {
        return;
    }

    i = 0;
    while (ship->clampInfo != NULL && ship->clampInfo->selection != NULL)
    {
        SelectCommand *attachedShips = (SelectCommand *)ship->clampInfo->selection;
        sdword originalNumShips = attachedShips->numShips;
        Ship *containedShip;

        if (i >= attachedShips->numShips)
        {
            break;
        }

        containedShip = attachedShips->ShipPtr[i];
        workerCheckResetForHyperspace(containedShip);
        if (shipIsBeastRace(containedShip))
        {
            SelectCommand oneShipSel;

            oneShipSel.numShips = 1;
            oneShipSel.ShipPtr[0] = containedShip;
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &oneShipSel);
        }
        if (salCapCantDoFormation(containedShip))
        {
            SalCapDropTarget(containedShip);
        }
        if (containedShip->staticinfo->shipclass != CLASS_Component)
        {
            clDetachShipFromCommand(containedShip);
        }

        if (ship->clampInfo == NULL || ship->clampInfo->selection == NULL)
        {
            break;
        }

        if (originalNumShips == ((SelectCommand *)ship->clampInfo->selection)->numShips)
        {
            i++;
        }
        else
        {
            i = 0;
        }
    }
}

void spHyperspaceSetSelectionDestination(SelectCommand *selection,hvector *destination)
{
    ShipSinglePlayerGameInfo *ssinfo;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (destination == NULL || numShips < 1)
    {
        return;
    }

    for (i=0;i<numShips;i++)
    {
        growSelectInit(&clampedShips);
        GetShipsClampedToInHS(selection->ShipPtr[i],&clampedShips);
        if (clampedShips.selection->numShips > 0)
        {
            spHyperspaceSetSelectionDestination(clampedShips.selection,destination);
        }
        growSelectClose(&clampedShips);

        ssinfo = selection->ShipPtr[i]->shipSinglePlayerGameInfo;
        ssinfo->midlevelHyperspaceIn = TRUE;
        ssinfo->staticHyperspaceGate = FALSE;
        ssinfo->midlevelHyperspaceTo = *destination;
        ssinfo->timeToHyperspace += spHyperspaceDelay;
    }
}

void spHyperspaceSelectionIn(SelectCommand *selection)
{
    Ship *ship;
    Ship *defactoMothership;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (numShips < 1)
    {
        return;
    }

    defactoMothership = selection->ShipPtr[0];
    for (i = 0; i < numShips; i++)
    {
        ship = selection->ShipPtr[i];
        if (!ShipAlreadyHyperspaceOut(ship))
        {
            if (ship->attributes & ATTRIBUTES_StartInHS)
            {
                CalculateHyperspaceTimeForShip(ship, defactoMothership);
                ship->shipSinglePlayerGameInfo->hsState = HS_SLICING_OUTOF;
                if ((ship->flags & SOF_DontDrawTrails) == 0)
                {
                    univRemoveShipFromOutside(ship);
                    AddShipToHyperspace(ship);
                    ship->shipSinglePlayerGameInfo->midlevelHyperspaceOut = FALSE;
                }
                bitClear(ship->attributes, ATTRIBUTES_StartInHS);
            }
            else
            {
                hyperspaceDetachClampedShips(ship);
                CalculateHyperspaceTimeForShip(ship, defactoMothership);
                ship->shipSinglePlayerGameInfo->timeToHyperspace += spHyperspaceDelay;
                if (ship->clampInfo != NULL &&
                    ship->clampInfo->host != NULL &&
                    ship->clampInfo->host->objtype == OBJ_ShipType &&
                    (((Ship *)ship->clampInfo->host)->specialFlags & SPECIAL_Hyperspacing))
                {
                    ship->shipSinglePlayerGameInfo->timeToHyperspace =
                        ((Ship *)ship->clampInfo->host)->shipSinglePlayerGameInfo->timeToHyperspace;
                }
                growSelectAddShip(&singlePlayerGameInfo.ShipsToHyperspace, ship);
            }

            growSelectInit(&clampedShips);
            GetShipsClampedTo(ship, &clampedShips);
            if (clampedShips.selection->numShips > 0)
            {
                spHyperspaceSelectionIn(clampedShips.selection);
            }
            growSelectClose(&clampedShips);
        }
    }
}

void spHyperspaceSelectionInStatic(SelectCommand *selection,hvector *destination)
{
    ShipSinglePlayerGameInfo *ssinfo;
    sdword i;
    sdword numShips = selection->numShips;

    if (numShips < 1)
    {
        return;
    }

    for (i=0;i<numShips;i++)
    {
        ssinfo = selection->ShipPtr[i]->shipSinglePlayerGameInfo;
        ssinfo->midlevelHyperspaceIn = TRUE;
        ssinfo->staticHyperspaceGate = TRUE;
        ssinfo->midlevelHyperspaceTo = *destination;
    }
}

void GetShipsClampedTo(Ship *clampedto,GrowSelection *clampedships)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->clampInfo)
        {
            if ((ship->clampInfo->host == (SpaceObjRotImpTarg *)clampedto) && (!ShipInSelection(singlePlayerGameInfo.ShipsToHyperspace.selection,ship)))
            {
                growSelectAddShip(clampedships,ship);
            }
        }

        objnode = objnode->next;
    }
}

bool32 ShipAlreadyHyperspaceOut(Ship *ship)
{
    if ((ship->flags & SOF_Hyperspace) ||
        ShipInSelection(singlePlayerGameInfo.ShipsToHyperspace.selection,ship))
    {
        return TRUE;
    }

    return FALSE;
}

void spHyperspaceSelectionOut(SelectCommand *selection)
{
    Ship *ship;
    Ship *defactoMothership;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (numShips < 1)
    {
        return;
    }

    defactoMothership = selection->ShipPtr[0];
    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if (!ShipAlreadyHyperspaceOut(ship))
        {
            if (ship->attributes & ATTRIBUTES_StartInHS)
            {
                CalculateHyperspaceTimeForShip(ship,defactoMothership);
                ship->shipSinglePlayerGameInfo->hsState = HS_SLICING_OUTOF;
                if ((ship->flags & SOF_DontDrawTrails) == 0)
                {
                    univRemoveShipFromOutside(ship);
                    AddShipToHyperspace(ship);
                    ship->shipSinglePlayerGameInfo->midlevelHyperspaceOut = FALSE;
                }
                bitClear(ship->attributes, ATTRIBUTES_StartInHS);
            }
            else
            {
                CalculateHyperspaceTimeForShip(ship,defactoMothership);
                ship->shipSinglePlayerGameInfo->timeToHyperspace += spHyperspaceDelay;
                if(ship->clampInfo != NULL)
                {
                    //ship is clamped to another ship...
                    if(ship->clampInfo->host->objtype == OBJ_ShipType)
                    {
                        if( ((Ship *)ship->clampInfo->host)->specialFlags & SPECIAL_Hyperspacing)
                        {
                            //host has been told to hyperspace
                            //so lets set the clamped ships hyperspace time, to the same time as its hose
                            ship->shipSinglePlayerGameInfo->timeToHyperspace =
                                ((Ship *)ship->clampInfo->host)->shipSinglePlayerGameInfo->timeToHyperspace;
                        }
                    }
                }
                growSelectAddShip(&singlePlayerGameInfo.ShipsToHyperspace,ship);
            }
            growSelectInit(&clampedShips);
            GetShipsClampedTo(ship,&clampedShips);
            if (clampedShips.selection->numShips > 0)
            {
                spHyperspaceSelectionOut(clampedShips.selection);
            }
            growSelectClose(&clampedShips);
        }
#ifndef HW_Release
        else
        {
            char warningmsg[200];
            sprintf(warningmsg,"Hyperspaced %s tried to hyperspace",ShipTypeToStr(ship->shiptype));
            clCommandMessage(warningmsg);
        }
#endif

    }
}

void spHyperspaceSelectionOutStatic(SelectCommand *selection)
{
    Ship *ship;
    Ship *defactoMothership;
    sdword i;
    sdword numShips = selection->numShips;
    GrowSelection clampedShips;

    if (numShips < 1)
    {
        return;
    }

    defactoMothership = selection->ShipPtr[0];
    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if (!ShipAlreadyHyperspaceOut(ship))
        {
            growSelectInit(&clampedShips);
            GetShipsClampedTo(ship,&clampedShips);
            if (clampedShips.selection->numShips > 0)
            {
                spHyperspaceSelectionOutStatic(clampedShips.selection);
            }
            growSelectClose(&clampedShips);
            if (ship->attributes & ATTRIBUTES_StartInHS)
            {
                CalculateHyperspaceTimeForShip(ship,defactoMothership);
                ship->shipSinglePlayerGameInfo->hsState = HS_SLICING_OUTOF;
                if ((ship->flags & SOF_DontDrawTrails) == 0)
                {
                    univRemoveShipFromOutside(ship);
                    AddShipToHyperspace(ship);
                    ship->shipSinglePlayerGameInfo->midlevelHyperspaceOut = FALSE;
                }
                bitClear(ship->attributes, ATTRIBUTES_StartInHS);
            }
            else
            {
                ShipSinglePlayerGameInfo* ssinfo = ship->shipSinglePlayerGameInfo;
                if (ssinfo != NULL)
                {
                    ssinfo->staticHyperspaceGate = TRUE;
                }
                CalculateHyperspaceTimeForShip(ship,defactoMothership);
                growSelectAddShip(&singlePlayerGameInfo.ShipsToHyperspace,ship);
            }
        }
#ifndef HW_Release
        else
        {
            char warningmsg[200];
            sprintf(warningmsg,"Hyperspaced %s tried to hyperspace",ShipTypeToStr(ship->shiptype));
            clCommandMessage(warningmsg);
        }
#endif
    }
}

void singlePlayerInit(void)
{
    sdword levelIndex;

    spHyperspaceCommandMessage = strGetString(strSinglePlayerHyperspace);

    for (levelIndex = 0; levelIndex < NUMBER_SINGLEPLAYER_MISSIONS; levelIndex++)
    {
        warpScriptLevels[levelIndex].resources = 0;
        memset(warpScriptLevels[levelIndex].shipCounts, 0, sizeof(warpScriptLevels[levelIndex].shipCounts));
    }

    warpScriptEnabled = 0;
    warpScriptLevelToWarpTo = 0;
    scriptSet(NULL, WARP_FILENAME, WarpScriptTable);

    warpLevel = 0;
    if (warpScriptEnabled)
    {
        warpLevel = warpScriptLevelToWarpTo;
        singlePlayerGameInfo.currentMission = warpScriptLevelToWarpTo;
    }
    else if (singlePlayerGame)
    {
        singlePlayerGameInfo.currentMission = MISSION_1_KHARAK_SYSTEM;
    }

    singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;
    singlePlayerGameInfo.oldAutoLaunch = BIT0 | BIT1 | BIT2 | BIT3;
    singlePlayerGameInfo.resourceUnitsCollected = 0;
    singlePlayerGameInfo.giveComputerFleetControl = FALSE;
    singlePlayerGameInfo.asteroid0sCanMove = FALSE;
    singlePlayerGameInfo.hyperspaceFails = FALSE;
    singlePlayerGameLoadNewLevelFlag = FALSE;
    singlePlayerGameInfo.playerCanHyperspace = FALSE;
    spHyperspaceObjectiveActive = FALSE;
    smUpdateHyperspaceStatus(FALSE);

    listInit(&singlePlayerGameInfo.ShipsInHyperspace);
    growSelectInit(&singlePlayerGameInfo.ShipsToHyperspace);

    if (singlePlayerGame)
    {
        objectiveStartup();
        kasInit();
    }

    HYPERSPACE_FARCLIPPLANESQR = HYPERSPACE_FARCLIPPLANE * HYPERSPACE_FARCLIPPLANE;

    HYPERSPACE_DECELERATE_DIST = 0.5f * HYPERSPACE_DECELERATION * HYPERSPACE_DECELERATE_TIME*HYPERSPACE_DECELERATE_TIME;
    HYPERSPACE_DECELERATE_INITIAL_VEL = HYPERSPACE_DECELERATION * HYPERSPACE_DECELERATE_TIME;
    dbgAssert(HYPERSPACE_DECELERATE_DIST < HYPERSPACE_FARCLIPPLANE);
    HYPERSPACE_WHIPTOWARDSDIST = HYPERSPACE_FARCLIPPLANE - HYPERSPACE_DECELERATE_DIST;
    dbgAssert(HYPERSPACE_DECELERATE_INITIAL_VEL < HYPERSPACE_WHIP_VELOCITY);
}

static udword spLockoutFlags = 0;

void FancyFightPreLoad(sdword closeSensors)
{
    if (closeSensors && smSensorsActive)
    {
        smSensorsCloseForGood();
    }
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    spCleanupScreenStack();
}

void spMainScreen()
{
    if (smSensorsActive)
    {
        smSensorsCloseForGood();
    }
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    feAllMenusDelete();                                     //make sure no menus stay up during NIS
    tbForceTaskbar(FALSE);                                  //make sure taskbar is down during NIS
}

void spLockout(udword flags)
{
    spLockoutFlags = flags;
//comment out the rest of this function to use NIS debugging keys
#if NIS_PRINT_INFO
    if (nisNoLockout)
    {
        return;
    }
#endif
    if (flags & SPLOCKOUT_DESELECT)
    {
        selSelectNone();
    }

    smSensorsDisable = TRUE;
    tbDisable = TRUE;
    if (flags & SPLOCKOUT_MR)
    {
        mrDisable();
    }
    if (flags & SPLOCKOUT_MOUSE)
    {
        mouseDisable();
    }
}

void spMainScreenAndLockout(udword flags)
{
    if (smSensorsActive)
    {
        smSensorsCloseForGood();
    }
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    spCleanupScreenStack();
    if (spManagerScreenFlags)
    {
        spClearScreenLockout();
    }
    spManagerScreenFlags = flags;
    if (flags & SPLOCKOUT_DESELECT)
    {
        selSelected.numShips = 0;
        ioUpdateShipTotals();
    }
    spScreenLockoutActive = TRUE;
    if (flags & SPLOCKOUT_MR)
    {
        mrDisable();
        mouseCursorHide();
    }
    if (flags & SPLOCKOUT_MOUSE)
    {
        mouseForceInvisible();
    }
}

void mrManagerScreenBegin(sdword flags)
{
    if (spManagerScreenFlags)
    {
        spRestoreManagerScreen();
    }

    spManagerScreenFlags = flags;
    if (flags & SPLOCKOUT_DESELECT)
    {
        selSelected.numShips = 0;
        ioUpdateShipTotals();
    }

    spScreenLockoutActive = 1;
    if (flags & SPLOCKOUT_MR)
    {
        mrDisable();
        mouseCursorHide();
    }
    if (flags & SPLOCKOUT_MOUSE)
    {
        mouseForceInvisible();
    }
}

void spUnlockout()
{
//comment out the rest of this function to use NIS debugging keys
    smSensorsDisable = FALSE;
    tbDisable = FALSE;
    if (spLockoutFlags & SPLOCKOUT_DESELECT)
    {
        bitClear(spLockoutFlags,SPLOCKOUT_DESELECT);
    }
    if (spLockoutFlags & SPLOCKOUT_MR)
    {
        mrEnable();
        bitClear(spLockoutFlags,SPLOCKOUT_MR);
    }
    if (spLockoutFlags & SPLOCKOUT_MOUSE)
    {
        mouseEnable();
        bitClear(spLockoutFlags,SPLOCKOUT_MOUSE);
    }
}

void spRestoreManagerScreen(void)
{
    spScreenLockoutActive = 0;
    dword_A5B1B0 = 0;
    if (spManagerScreenFlags & SPLOCKOUT_DESELECT)
        spManagerScreenFlags &= ~SPLOCKOUT_DESELECT;
    if (spManagerScreenFlags & SPLOCKOUT_MR)
    {
        mrEnable();
        mouseCursorShow();
        spManagerScreenFlags &= ~SPLOCKOUT_MR;
    }
    if (spManagerScreenFlags & SPLOCKOUT_MOUSE)
        mouseEnable();
    spManagerScreenFlags = 0;
}

void spHyperspaceRollCallWideScreenIn(sdword frames);

void spHyperspaceButtonPushed(void)
{
    SelectCommand *sel;
    sdword n;
    Node *node;
    Ship *ship;
    fescreen *screen;
    featom *atom;
    const char *hyperspaceAtomName;

    if (!singlePlayerGameInfo.playerCanHyperspace && !hyperspaceOverride)
        return;

    if (overlayPauseActive)
        return;

    if (spCurrentPlayerHasHyperspaceShip())
    {
        clCommandMessage(spHyperspaceCommandMessage, (color)0xFF0000FF);
        return;
    }

    hyperspaceOverride = FALSE;
    singlePlayerGameInfo.playerCanHyperspace = FALSE;
    smUpdateHyperspaceStatus(FALSE);

    if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
        return;

    if (universe.curPlayerPtr->PlayerMothership == NULL)
        return;

    if (universePause)
        universePause = FALSE;

    ccFocusOnMyMothership(&universe.mainCameraCommand);

    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WAITINGROLLCALL;
    singlePlayerGameInfo.oldAutoLaunch = universe.curPlayerPtr->autoLaunch;
    clAutoLaunch(FALSE, universe.curPlayerIndex);

    if (spHyperspaceWideScreenTable[singlePlayerGameInfo.currentMission])
    {
        spHyperspaceRollCallWideScreenIn(90);
        cat_mrBlackoutEndTime = cat_mrBlackoutDelay + universe.realtimeelapsed;
        return;
    }

    sel = spBuildFleetSelection();
    removeShipsFromDockingWithThisShip(universe.curPlayerPtr);
    n = clDock(&universe.mainCommandLayer, sel, DOCK_PERMANENTLY, NULL);
    if (n > 0)
        speechEventQueue(0, 0x200101F, -1, -1, 0, -1, -1, 5.0f, -1);
    memFree(sel);

    TellHyperspacingShipsToReady();

    if (smSensorsActive)
        smSensorsCloseForGood();
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    spCleanupScreenStack();

    if (spManagerScreenFlags)
        spRestoreManagerScreen();
    spManagerScreenFlags = SPLOCKOUT_MR;
    spScreenLockoutActive = 1;

    mrDisable();
    mouseCursorHide();

    if (!spHyperspaceRollCallHandle)
    {
        feCallbackAddMultiple(spHyperspaceCallback);
        feDrawCallbackAddMultiple(spHyperspaceDrawCallback);
        spHyperspaceRollCallHandle = feScreensLoad("FEMan\\Hyperspace_Roll_Call.fib");
    }

    CalculateRollCall();
    spHyperspaceRollCallScreen = feScreenStart(ghMainRegion, "HyperspaceRollCall");
    screen = feScreenFind("HyperspaceRollCall");
    if (!screen)
        return;

    hyperspaceAtomName = "EmergencyHyperspace";
    node = universe.ShipList.head;
    while (node)
    {
        ship = (Ship *)node->structptr;
        if (ship->playerowner->playerIndex == 0)
        {
            if (ship->recentlyAttacked > 0 || ship->recentlyFiredUpon > 0)
            {
                hyperspaceAtomName = "GoNowHyperspace";
                break;
            }
        }
        node = node->next;
    }

    atom = feAtomFindInScreen(screen, (char *)hyperspaceAtomName);
    if (atom)
        bitSet(((region *)atom->region)->status, RSF_RegionDisabled);
}

void spTaskBarHyperspaceCB(struct taskbutton *button, ubyte *userData)
{
    spHyperspaceButtonPushed();
}

void singlePlayerPostInit(bool32 loadingSaveGame)
{
    if (!loadingSaveGame)
    {
        if (singlePlayerGameInfo.currentMission == 1)
        {
            animBinkPlay(0, 1);
        }
    }
    //hyperspacetbbutton = tbButtonCreate("Hyperspace",spTaskBarHyperspaceCB,(ubyte *)0x00,0);
    //tbButtonListRefresh();
}

void singlePlayerClose(void)
{
    kasClose();

    if (spHyperspaceRollCallHandle)
    {
        feScreensDelete(spHyperspaceRollCallHandle);
        spHyperspaceRollCallHandle = NULL;
    }
    smSensorWeirdness = FALSE;
#if SP_DEBUGLEVEL2
    mrKASDebugDrawStates = FALSE;
    mrKASDebugDrawTimers = FALSE;
    mrKASDebugDrawVars = FALSE;
    mrKASDebugDrawVolumes = FALSE;
    mrKASSkipFactor = 0;
#endif
}

void singlePlayerSetMissionAttributes(char *directory,char *filename)
{
    singlePlayerGameInfo.onCompleteHyperspace = TRUE;     // default to TRUE
    singlePlayerGameInfo.onLoadPlayNIS = 0;
    singlePlayerGameInfo.onCompletePlayNIS = 0;
    singlePlayerGameInfo.resourceUnitsCollected = 0;
    if (universe.players[0].race == RACE_Sect)
    {
        universe.players[1].race = RACE_Beast;      // default to enemy race
    }
    else
    {
        universe.players[1].race = RACE_Sect;       // default to enemy race
    }
    scriptSet(directory,filename,SinglePlayerScriptTable);

    //reset nebulae
    nebReset();
}

void singlePlayerLevelLoaded(void)
{
    if (singlePlayerGameInfo.currentMission == 10)
    {
        Node *objnode;
        Ship *ship;

        objnode = universe.ShipList.head;
        while (objnode != NULL)
        {
            ship = (Ship *)listGetStructOfNode(objnode);
            objnode = objnode->next;
            dbgAssert(ship->objtype == OBJ_ShipType);

            if ((ship->playerowner->playerIndex == 1) && ShipTypeIsCarrierType(ship->shiptype))
            {
                bitSet(ship->specialFlags2, SPECIAL_2_DontPlowThroughEnemyShips);
            }
        }
    }
}

void CleanupDamageEffects(void)
{
    InsideShip* insideShip;
    Ship* ship;
    Node* node = singlePlayerGameInfo.ShipsInHyperspace.head;

    while (node != NULL)
    {
        insideShip = (InsideShip*)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        node = node->next;

        dmgForgetEffects(ship);
    }
}

void CleanupHyperspacingShip(Ship *ship)
{
    dbgAssert(ship->shipSinglePlayerGameInfo);
    ship->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_NONE;
}

void CleanupHyperspacingShipsThatDidntHyperspace()
{
    Node *objnode;
    Ship *ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        objnode = objnode->next;
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr) && !(ship->flags & SOF_Disabled))
        {
            if (ShipCanMakeHyperspace(ship))
            {
                CleanupHyperspacingShip(ship);
            }
        }
    }
}

void CleanupHyperspacingShips(bool32 FreeShips,bool32 cleanupAllPlayersShips)
{
    InsideShip *insideShip;
    Ship *ship;
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    Node *deletenode;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        deletenode = node;
        node = node->next;

        if (cleanupAllPlayersShips || allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            listDeleteNode(deletenode);     // put before so univFreeShipContents doesn't try to delete this twice
            if ((FreeShips) && (ship->shipSinglePlayerGameInfo->shipHyperspaceState == SHIPHYPERSPACE_GONE))
            {
                if (ship->objlink.belongto != NULL)
                {
                    listRemoveNode(&ship->objlink);
                }
                univRemoveObjFromRenderList((SpaceObj *)ship);
                univFreeShipContents(ship);
                memFree(ship);
            }
            else
            {
                CleanupHyperspacingShip(ship);
            }
        }
    }

    if (singlePlayerGameInfo.hyperspaceFails)
    {
        CleanupHyperspacingShipsThatDidntHyperspace();
    }
}

void singlePlayerShipDied(Ship *ship)
{
    growSelectRemoveShip(&singlePlayerGameInfo.ShipsToHyperspace,ship);

    if (singlePlayerGameInfo.ShipsInHyperspace.num)
    {
        InsideShip *insideShip;
        Ship *checkship;
        Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;

        while (node != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            checkship = insideShip->ship;
            dbgAssert(checkship->objtype == OBJ_ShipType);

            if (checkship == ship)
            {
                listDeleteNode(node);
                break;
            }

            node = node->next;
        }
    }
}

void singlePlayerGameCleanup(void)
{
    if (!singlePlayerGame)
    {
        CleanupHyperspacingShips(TRUE,TRUE);
        return;
    }

    // delete all objectives
    objectiveShutdown();

    // remove ship lists, paths, volumes, etc.
    kasLabelledEntitiesDestroy();

    pingAllPingsDelete();

    if (singlePlayerGameLoadNewLevelFlag)
    {
        return;
    }

    spClearScreenLockout();

    CleanupHyperspacingShips(TRUE,TRUE);

    if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
    {
        if (spHyperspaceRollCallScreen != NULL)
        {
            feScreenDelete(spHyperspaceRollCallScreen);
            spHyperspaceRollCallScreen = NULL;
        }

        ccClearModeFlag(&universe.mainCameraCommand,
            CCMODE_LOCK_OUT_USER_INPUT |
            CCMODE_FOCUS_CHANGES_LOOKATPOINT_ONLY |
            CCMODE_FOCUS_CHANGES_NOTHING);

        mrWhiteOut = FALSE;
        mrWhiteOutT = 0.0f;
    }

    cat_mrBlackOut = 0.0f;
    kasfMothershipBigGunDisabled = 1;
}

void UpdateMidLevelHyperspacingShips()
{
    SelectCommand *selection = singlePlayerGameInfo.ShipsToHyperspace.selection;
    sdword i;
    Node *node;
    Node *nextnode;
    InsideShip *insideShip;
    Ship *ship;
    ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;

    // update any mid-level hyperspacing away ships
    for (i = 0; i < selection->numShips; )
    {
        if (UpdateHyperspacingShip(selection->ShipPtr[i], TRUE))
        {
            growSelectRemoveShipIndex(&singlePlayerGameInfo.ShipsToHyperspace, i);   // deliberate don't increment i
        }
        else
        {
            i++;
            selection = singlePlayerGameInfo.ShipsToHyperspace.selection;
        }
    }

    // update any mid-level hyperspacing in ships
    node = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
        dbgAssert(ship->objtype == OBJ_ShipType);
        nextnode = node->next;

        if (shipSinglePlayerGameInfo->midlevelHyperspaceIn)
        {
            if (spUpdateShipHyperspace(ship, &shipSinglePlayerGameInfo->midlevelHyperspaceTo, TRUE))
            {
                shipSinglePlayerGameInfo->midlevelHyperspaceIn = FALSE;
                shipSinglePlayerGameInfo->hyperspaceEffect = NULL;
                bitClear(ship->specialFlags, SPECIAL_Hyperspacing);
                listDeleteNode(node);
            }
        }

        node = nextnode;
    }
}

void singlePlayerGameUpdate()
{
    AIVar *tgui;
    sdword nisVariableId = kasHashLabel("G_PlayNis");
    sdword nisletVariableId = kasHashLabel("G_PlayNislet");
    sdword *nisVariable = aivarFindEntry(0, (AIVar *)nisVariableId);
    sdword *nisletVariable = aivarFindEntry(0, (AIVar *)nisletVariableId);
    sdword value;
    static real32 hyperspaceStartTime;
#ifdef gshaw
    if (triggerNIS)
    {
        hvector startpoint;
        char *nis, *script;
        bool32 centre;

        GetStartPointPlayer(&startpoint);
        singlePlayerNISNamesGet(&nis, &script, &centre, 2);
        singlePlayerStartNis(nis, script, &startpoint, centre, FALSE);
        triggerNIS = FALSE;
    }
#endif

    //play an nis when the time is right
    if (nisVariable)
    {
        value = nisVariable[2];
        if (value != 0)
        {
            hvector startpoint;
            char *nis, *script;
            bool32 centre;

            GetStartPointPlayer(&startpoint);
            singlePlayerNISNamesGet(&nis, &script, &centre, value);
            singlePlayerStartNis(nis, script, &startpoint, centre, FALSE);
        }
        kasfEntryDelete(nisVariable);
    }

    //play an nislet when the time is right
    if (nisletVariable)
    {
        value = nisletVariable[2];
        if (value != 0)
        {
            hvector startpoint;
            char *nis, *script;

            GetStartPointPlayer(&startpoint);
            if (!singlePlayerNISletNamesGet(&nis, &script, value))
            {
                dbgFatalf(DBG_Loc, "\nNISlet %d is not defined", value);
            }
            if (!fileExists(nis, 0) || !fileExists(script, 0))
            {
                dbgFatalf(DBG_Loc, "\n'%s' and/or '%s' do not exist.", nis, script);
            }
            singlePlayerStartNis(nis, script, &startpoint, FALSE, FALSE);
            if (singlePlayerGameInfo.currentMission == 1)
            {
                trNoPalShutdown();
            }
        }
        kasfEntryDelete(nisletVariable);
    }

    //pause or unpause the NIS task.  This has to be called from within a task,
    //not the loading code, in case the player alt-tabs during loading.
    //Can you think of a better spot for it?
    nisTaskPauseOrResume();

    if (cat_mrBlackOut > 0.0f)
    {
        real32 previousTime;

        if (catMrBlackOutReset == 1)
        {
            previousTime = universe.totaltimeelapsed;
            catMrBlackOutReset = 0;
        }
        else
        {
            previousTime = catMrBlackOutLastTime;
        }
        catMrBlackOutLastTime = universe.totaltimeelapsed;
        cat_mrBlackOut -= (universe.totaltimeelapsed - previousTime) * 0.5f;
        if (cat_mrBlackOut < 0.0f)
        {
            cat_mrBlackOut = 0.0f;
        }
    }
    UpdateMidLevelHyperspacingShips();

    if (universe.curPlayerPtr->PlayerMothership == NULL)
    {
        if (singlePlayerGameInfo.hyperspaceState == HYPERSPACE_WAITINGROLLCALL)
        {
            tbSensorsHook();
        }
        return;
    }


    tgui = aivarFind("Traders.PopUpGUI");
    if (tgui)
    {
        if (tgui->value)
        {
            tgui->value = 0;
            tmTradeBegin(ghMainRegion, 0, 0, 0);
        }
    }

    switch (singlePlayerGameInfo.hyperspaceState)
    {
        case HYPERSPACE_WAITINGROLLCALL:
            if (spMissionUsesPostLoadArrivalPrepass())
            {
                if (universe.totaltimeelapsed <= cat_mrBlackoutEndTime)
                {
cat_mrBlackOut = 1.0f - (cat_mrBlackoutEndTime - universe.totaltimeelapsed) / cat_mrBlackoutDelay;
                }
                else
                {
SelectCommand *selection;
SelectCommand oneShipSelection;
Node *objnode;
Ship *mothership;
Ship *hyperspaceShip;

DeleteAnyMidLevelHyperspacingShips();
selection = spBuildFleetSelection();
clDock(&universe.mainCommandLayer, selection, DOCK_PERMANENTLY, NULL);
memFree(selection);

mothership = universe.curPlayerPtr->PlayerMothership;
for (objnode = universe.ShipList.head; objnode != NULL; objnode = objnode->next)
{
    hyperspaceShip = (Ship *)listGetStructOfNode(objnode);

    bitSet(hyperspaceShip->specialFlags, SPECIAL_SinglePlayerWithinHyperSpaceRange);
    if (allianceIsShipAlly(hyperspaceShip, universe.curPlayerPtr) &&
        !(hyperspaceShip->flags & SOF_Disabled) &&
        (hyperspaceShip->staticinfo->shipIsCapital || hyperspaceShip->staticinfo->shipclass == CLASS_Component) &&
        (hyperspaceShip->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange))
    {
        oneShipSelection.numShips = 1;
        oneShipSelection.ShipPtr[0] = hyperspaceShip;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &oneShipSelection);
        aitrackForceSteadyShip(hyperspaceShip);
        if (hyperspaceShip != mothership)
        {
            GroupShipIntoMilitaryParade(&universe.mainCommandLayer, hyperspaceShip, mothership);
            bitSet(hyperspaceShip->specialFlags, SPECIAL_SinglePlayerInParade);
        }
        clPresetShipsToPosition(&universe.mainCommandLayer);
        CalculateHyperspaceTimeForShip(hyperspaceShip, mothership);
        hyperspaceShip->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
        if ((hyperspaceShip->flags & SOF_DontDrawTrails) == 0)
        {
            univRemoveShipFromOutside(hyperspaceShip);
            AddShipToHyperspace(hyperspaceShip);
            hyperspaceShip->shipSinglePlayerGameInfo->midlevelHyperspaceOut = FALSE;
        }
    }
}

singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WHITEOUT;
singlePlayerGameInfo.hyperspaceTimeStamp = universe.phystimeelapsed + SPEECH_HYPERSPACE_DELAY + HYPERSPACE_WHITEOUTTIME;
mrWhiteOut = TRUE;
mrWhiteOutT = 1.0f;
spBinkPlay = TRUE;
speechEventStopAll(0.5f);
                }
            }
            else
            {
                CalculateRollCall();
                OrientHyperspacingShips();
                if (singlePlayerGameInfo.rollcallShipsRemaining == 0)
                {
if (spHyperspaceRollCallScreen != NULL)
{
    spGoNowHyperspaceCB(NULL, NULL);
    spHyperspaceRollCallScreen = NULL;
}

singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WAITINGFORFLEETCOMMAND;
singlePlayerGameInfo.hyperspaceTimeStamp = 0.0f;
speechEventFleet(STAT_F_Hyper_TakingOff, 0, universe.curPlayerIndex);
hyperspaceStartTime = universe.phystimeelapsed + SPEECH_HYPERSPACE_DELAY;
                }
            }
            break;

        case HYPERSPACE_WAITINGFORFLEETCOMMAND:
            if (universe.phystimeelapsed >= hyperspaceStartTime)
            {
                singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSLEAVING;
            }
            break;

        case HYPERSPACE_SHIPSLEAVING:
            if (singlePlayerGameInfo.hyperspaceTimeStamp != 0.0f)
            {
                if (singlePlayerGameInfo.hyperspaceTimeStamp == 1.0f)
                {
Node *objnode;
Ship *hyperspaceShip;
bool32 allShipsHyperspacedOut = TRUE;

for (objnode = universe.ShipList.head; objnode != NULL; objnode = objnode->next)
{
    hyperspaceShip = (Ship *)listGetStructOfNode(objnode);

    if (allianceIsShipAlly(hyperspaceShip, universe.curPlayerPtr) &&
        !(hyperspaceShip->flags & SOF_Disabled) &&
        ShipCanMakeHyperspace(hyperspaceShip) &&
        !UpdateHyperspacingShip(hyperspaceShip, FALSE))
    {
        allShipsHyperspacedOut = FALSE;
    }
}

if (singlePlayerGameInfo.hyperspaceFails)
{
    if (allShipsHyperspacedOut)
    {
        singlePlayerGameInfo.hyperspaceState = HYPERSPACE_FAILED;
        goto hyperspacefailed;
    }
}
else if (allShipsHyperspacedOut)
{
    spBinkPlay = TRUE;
    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WHITEOUT;
    singlePlayerGameInfo.hyperspaceTimeStamp = universe.phystimeelapsed + HYPERSPACE_WHITEOUTTIME;
    speechEventStopAll(0.5f);
    mrWhiteOut = TRUE;
    mrWhiteOutT = 1.0f;
}
                }
            }
            else
            {
                bool32 shipsoriented = OrientHyperspacingShips();

                if ((universe.phystimeelapsed >= hyperspaceStartTime) && shipsoriented)
                {
CalculateHyperspaceTimesForShips();
DeleteAnyMidLevelHyperspacingShips();
singlePlayerGameInfo.hyperspaceTimeStamp = 1.0f;
if (!singlePlayerGameInfo.hyperspaceFails)
{
    soundEventStopTrack(musicEventCurrentTrack(), 1.5f);
}
                }
            }
            break;

        case HYPERSPACE_WHITEOUT:
            if (universe.phystimeelapsed >= singlePlayerGameInfo.hyperspaceTimeStamp)
            {
                if (spBinkPlay)
                {
#if defined(OEM)
//no animatic between OEM Missions 4 and 5 and thereafter
                    if (singlePlayerGameInfo.currentMission < 4)
#endif
                    //playback level transition animatic
                    animBinkPlay(singlePlayerGameInfo.currentMission, singlePlayerGameInfo.currentMission + 1);

#if defined(CGW)
                    if (singlePlayerGameInfo.currentMission == 2)
                    {
                        universe.quittime = universe.totaltimeelapsed;
                        utyPlugScreens = TRUE;
                    }
#elif defined(OEM)
                    if (singlePlayerGameInfo.currentMission == 5)
                    {
                        universe.quittime = universe.totaltimeelapsed;
                        utyPlugScreens = TRUE;
                    }
#else
                    if (singlePlayerGameInfo.currentMission == 16)
                    {
                        universe.quittime = universe.totaltimeelapsed;
                        utyCreditsSequence = TRUE;
                    }
#endif
                    spBinkPlay = FALSE;
                    mrWhiteOut = FALSE;
                }
                else
                {
#if BINK
                    if (!binkDonePlaying)
                    {
                        break;
                    }
#endif

                    // clear damage effects from ships
                    CleanupDamageEffects();

                    if (singlePlayerGameInfo.currentMission == 1 || singlePlayerGameInfo.onLoadPlayNIS > 0)
                    {
                        char *onLoadNis;
                        char *onLoadScript;
                        bool32 loadLevelBeforeStart;
                        sdword nisNumber = (singlePlayerGameInfo.currentMission == 1) ? 1 : singlePlayerGameInfo.onLoadPlayNIS;

                        singlePlayerNISNamesGet(&onLoadNis, &onLoadScript, &loadLevelBeforeStart, nisNumber);
                        singlePlayerStartNis(onLoadNis, onLoadScript, NULL, TRUE, FALSE);
                        singlePlayerGameInfo.hyperspaceState = HYPERSPACE_WAITINGFORNISARRIVAL;
                    }
                    else
                    {
                        // rebuild level now
                        singlePlayerGameLoadNewLevelFlag = TRUE;
                        singlePlayerHyperspacingInto = TRUE;
                        singlePlayerGameInfo.hyperspaceState = HYPERSPACE_LOADDONE;
                    }
                    cat_mrBlackOut = 1.0f;
                    catMrBlackOutReset = 1;
                }
            }
            break;

        case HYPERSPACE_LOADDONE:
            singlePlayerGameInfo.hyperspaceState = HYPERSPACE_SHIPSARRIVING;
            soundEvent(universe.curPlayerPtr->PlayerMothership, Ship_SinglePlayerHyperspace);
            singlePlayerKasMissionStart(singlePlayerGameInfo.currentMission);
            singlePlayerMissionStartedDuringLoad = TRUE;
            nisWideScreenActive = TRUE;
            if (smSensorsActive)
            {
                smSensorsCloseForGood();
            }
            cmCloseIfOpen();
            rmCloseIfOpen();
            lmCloseIfOpen();
            spCleanupScreenStack();
            if (spManagerScreenFlags)
            {
                spClearScreenLockout();
            }
            spManagerScreenFlags = SPLOCKOUT_MR;
            spScreenLockoutActive = TRUE;
            mrDisable();
            mouseCursorHide();
            if (singlePlayerGameInfo.currentMission == 1)
            {
                spPostLoadNisBlackout = TRUE;
                spPreservePostLoadNisMode = TRUE;
                cat_mrWhiteOutT = 0.0f;
            }
            spPostLoadArrivalPrepassPending = spMissionUsesPostLoadArrivalPrepass();
            if (spPostLoadArrivalPrepassPending)
            {
                spPostLoadArrivalWhiteoutFadeStartTime = universe.totaltimeelapsed + whiteoutFadeDelay;
            }
            break;

        case HYPERSPACE_WAITINGFORNISARRIVAL:
            break;

        case HYPERSPACE_SHIPSARRIVING:
            break;
hyperspacefailed:
        case HYPERSPACE_FAILED:
            {
                // all ships have arrived and are in starting position
                singlePlayerGameInfo.hyperspaceState = NO_HYPERSPACE;
                CleanupHyperspacingShips(FALSE,FALSE);

                spUnlockout();

                //launch all necessary ships first
                LaunchAllInternalShipsOfPlayerThatMustBeLaunched(universe.curPlayerPtr);

                clAutoLaunch(singlePlayerGameInfo.oldAutoLaunch,universe.curPlayerIndex);

                tutGameMessage("Game_HyperspaceFailed");
                singlePlayerGameInfo.hyperspaceFails = FALSE;
            }
            break;

        case HYPERSPACE_LAUNCH_DOCKED:
        {
            if (spUpdateArrivalState())
            {
                spCompleteArrivingShips();
            }
            break;
        }
    }
}

static void SetOnMissionCompleteInfo(char *directory,char *field,void *dataToFillIn)
{
    char str[30];
    SinglePlayerGameInfo *info = (SinglePlayerGameInfo *)dataToFillIn;

    RemoveCommasFromString(field);

    sscanf(field,"%d %s",&info->onCompletePlayNIS,str);

    if (strcmpi(str,"NOHYPERSPACE") == 0)
    {
        info->onCompleteHyperspace = FALSE;
    }
    else
    {
        info->onCompleteHyperspace = TRUE;
    }
}

void DeleteAllRemainingShips()
{
    Ship *ship;
    Node *node = universe.ShipList.head;

    while (node != NULL)
    {
        ship = listGetStructOfNode(node);
        ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
        univReallyDeleteThisShipRightNow(ship);

        node = universe.ShipList.head;  // walk the list like this because univReallyDeleteThisShipRightNow is always
                                        // modifying the list of ships as it deletes stuff
    }
}

void DeleteAnyMidLevelHyperspacingShips()
{
    Node *node = singlePlayerGameInfo.ShipsInHyperspace.head;
    Node *nextnode;
    InsideShip *insideShip;
    Ship *ship;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        ship = insideShip->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);
        nextnode = node->next;

        if (ship->shipSinglePlayerGameInfo->midlevelHyperspaceOut)
        {
            ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
            univDeleteWipeInsideShipOutOfExistence(ship);   // will wipe out of ShipsInHyperspace list
        }

        node = nextnode;
    }
}

void singlePlayerLoadNewLevel(bool32 loadNISLevel, bool32 suppressMissionAdvance)
{
    real32 savedelta = universe.unknown1;
    real32 savetottime = universe.totaltimeelapsed;
    real32 saverealtime = universe.realtimeelapsed;
    Ship *savedPlayerMothership = universe.players[0].PlayerMothership;
    SelectCommand *selection;
    sdword i;
    Node *node;
    Ship *ship;

    if (!loadNISLevel)
    {
        cat_mrBlackOut = 1.0f;
        catMrBlackOutReset = 1;
    }
    else
    {
        cat_mrBlackOut = 0.0f;
    }

    universeSwitchToPlayer(0);
    singlePlayerGameLoadNewLevelFlag = TRUE;

    if (!suppressMissionAdvance && !singlePlayerPreserveCurrentMissionDuringLoad)
    {
#ifndef HW_Release
        if (SINGLEPLAYER_DEBUGLEVEL)
        {
            singlePlayerGameInfo.currentMission = SINGLEPLAYER_DEBUGLEVEL;
            SINGLEPLAYER_DEBUGLEVEL = 0;
        }
        else
#endif
        singlePlayerGameInfo.currentMission++;                  // go to next level
    }
    singlePlayerPreserveCurrentMissionDuringLoad = FALSE;

    if (singlePlayerGameInfo.currentMission == 1 &&
        !loadNISLevel &&
        nisLoadLevelBeforeStart[0])
    {
        selection = spBuildFleetSelection();
        for (i = 0; i < selection->numShips; i++)
        {
            univRemoveShipFromOutside(selection->ShipPtr[i]);
            dockPutShipInside(selection->ShipPtr[i], universe.curPlayerPtr->PlayerMothership);
        }
        memFree(selection);
        clAutoLaunch(15, 0);

        for (node = universe.ShipList.head; node != NULL; )
        {
            ship = (Ship *)listGetStructOfNode(node);
            if (ship->playerowner == universe.curPlayerPtr &&
                allianceIsShipAlly(ship, universe.curPlayerPtr) &&
                (ship->staticinfo->shipIsCapital || ship->staticinfo->shipclass == CLASS_Component))
            {
                CalculateHyperspaceTimeForShip(ship, universe.curPlayerPtr->PlayerMothership);
                ship->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
                if ((ship->flags & SOF_DontDrawTrails) == 0)
                {
                    univRemoveShipFromOutside(ship);
                    AddShipToHyperspace(ship);
                    ship->shipSinglePlayerGameInfo->midlevelHyperspaceOut = FALSE;
                }
                node = universe.ShipList.head;
            }
            else
            {
                node = node->next;
            }
        }
    }

    DeleteAnyMidLevelHyperspacingShips();
    DeleteAllRemainingShips();

    rmResetStaticInfo();
    universeReset();
    univupdateReset();

    universe.unknown1 = savedelta;
    universe.totaltimeelapsed = savetottime;
    universe.realtimeelapsed = saverealtime;
    universe.players[0].PlayerMothership = savedPlayerMothership;

    // restart computer AI player
    dbgAssert(universe.players[1].aiPlayer == NULL);
    universe.players[1].aiPlayer = aiCurrentAIPlayer = aiplayerInit(&universe.players[1],AI_ADV);
    aiCurrentAIPlayer->primaryEnemyPlayer = universe.curPlayerPtr;

    // remove sensor weirdness if it's on
    kasfSensorsWeirdness(FALSE);
    singlePlayerCullHyperspaceShips();

    singlePlayerGameInfo.giveComputerFleetControl = FALSE;      // reset any computer control
    singlePlayerGameInfo.asteroid0sCanMove = FALSE;

    GetMissionsDirAndFile(singlePlayerGameInfo.currentMission, loadNISLevel); //what mission file to load?

//... start of crazy starting code from utility.c
    //reset the enemy player's race to the 'opposite' (R1/R2 race)
    universe.players[1].race = (universe.players[0].race == RACE_Sect) ? RACE_Beast : RACE_Sect;
    //!!! should we reset the other things like RU's and OnMissionLoad stuff?
    if (!singlePlayerSuppressHorseRaceLoadScreen)
    {
        horseRaceInit();
    }
    levelPreInit(spMissionsDir,spMissionsFile);                 //see what ships need to be loaded and what ships we can toss

    // pause sound engine
    soundEventPause(TRUE);
    //load in all the ships, register all the textures
    universeStaticInit();

    if (hrAbortLoadingGame)
    {
        goto abortload;
    }

    trailsRecolorize();
    mistrailsRecolorize();

    SetFleetModifier(singlePlayerGameInfo.currentMission, GetFleetStrengthHyperspace);
    levelStartNext(spMissionsDir,spMissionsFile);
    clPresetShipsToPosition(&universe.mainCommandLayer);
    if (etgHasBeenStarted)
    {
        etgReloadReset();
    }
    else
    {
        etgStartup();
        utySet(SSA_Effect);
        etgHasBeenStarted = TRUE;
    }
    pingReset();
    subReset();
    subReloadCardTextures();
    trRegistryRefresh();                                    //refresh registry - load in all textures
    etgFixupUV();
    utySet(SSA_RegistryRefreshed);

abortload:
    //start the sound engine playing again
    soundEventPause(FALSE);

    if (!singlePlayerSuppressHorseRaceLoadScreen)
    {
        horseRaceShutdown();
    }
    //... end of crazy starting code from utility.c

    if (!hrAbortLoadingGame)
    {
        //ccSetModeFlag(&universe.mainCameraCommand,CCMODE_LOCK_OUT_USER_INPUT);

        univUpdateRenderList();

        if (!loadNISLevel)
        {
            CalculateArrivingTimesForShips();
        }
    }

    bobInitProperties();
    levelInitResourceVolumes();
    shockwaveStartup();
    smMimicInitPlayerShipTypes();
    singlePlayerGameLoadNewLevelFlag = FALSE;
    ioOverlayElapsedTimeResetFromUniverse();
    rmUpdateBuildCapabilities(universe.curPlayerPtr);
}

Ship *FindPlayersMothership(Player *player)
{
    Node *objnode;
    Ship *ship;

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if (ShipTypeIsMothershipType(ship->shiptype))
            {
                return ship;
            }
        }

        objnode = objnode->next;
    }

    return NULL;
}

SelectCommand *GetAllPlayersShipsExceptMothership(Player *player)
{
    Node *objnode;
    Ship *ship;
    SelectCommand *selection;
    sdword numShips = 0;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"ToDoSelection",0);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if (!ShipTypeIsMothershipType(ship->shiptype))
            {
                selection->ShipPtr[numShips++] = ship;
            }
        }

        objnode = objnode->next;
    }

    selection->numShips = numShips;

    return selection;
}

//nisplaying *singlePlayerNISPlaying = NULL;

bool32 hyperspaceAtEndOfSinglePlayerNIS = FALSE;

void RotatePlayersFleetAbout(Player *player,vector about,real32 deg)
{
    Node *objnode;
    Ship *ship;
    vector position;
    vector rotatedposition;
    matrix rotzmat;
    real32 rot = DEG_TO_RAD(deg);

    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            vecSub(position,ship->posinfo.position,about);

            matMakeRotAboutZ(&rotzmat,(real32)cos(rot),(real32)sin(rot));

            matMultiplyMatByVec(&rotatedposition,&rotzmat,&position);
            vecAdd(ship->posinfo.position,about,rotatedposition);

            univRotateObjYaw((SpaceObjRot *)ship,rot);
        }

        objnode = objnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : FindShipsOfShipTypeOfPlayer
    Description : finds ships of shiptype and owned by player, puts them in growselect
    Inputs      : growselect, shiptype, player
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FindShipsOfShipTypeOfPlayer(GrowSelection *growselect,ShipType shiptype,Player *player)
{
    Node *node;
    Ship *ship;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        ship = (Ship *)listGetStructOfNode(node);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if ((ship->playerowner == player) && (ship->shiptype == shiptype))
        {
            growSelectAddShip(growselect,ship);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : singlePlayerNISNamesGet
    Description : Figure out the name of the nis and script file for a given
                    nis number.
    Inputs      : nisNumber - number of NIS to run
    Outputs     : nisname - name of nis file
                  scriptname - name of nis script file
                  centreMothership - TRUE if the NIS is to be centred about player's mothership
    Return      : void
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : singlePlayerNISletNamesGet
    Description : Figure out the name of the nis and script file for a given
                    nis number.
    Inputs      : nisletNumber - number of NISlet to run
    Outputs     : nisname - name of nis file
                  scriptname - name of nis script file
    Return      : TRUE if the NIS can be played properly
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : singlePlayerStartNis
    Description : Start a single player NIS or NISlet
    Inputs      : nisname - name of NIS file
                  scriptname - name of NIS script file
                  centreMothership - TRUE if this NIS plays relative to the
                    player's mothership (does not apply to NISlets).
                  positionAndHeading - ?
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void GetMissionsDirAndFile(sdword mission, bool32 loadNISLevel)
{
#ifdef OEM
    if (!loadNISLevel && mission == 5)
    {
        singlePlayerGameInfo.onLoadPlayNIS = 0;
        sprintf(spMissionsDir, "SinglePlayer\\Mission05_OEM\\");
        sprintf(spMissionsFile, "Mission05_OEM.mission");
        return;
    }
#endif

    if (loadNISLevel)
    {
        sdword nisNumber = missionToNis[mission];

        dbgAssert(nisNumber > 0);
        singlePlayerGameInfo.onLoadPlayNIS = nisNumber;
        sprintf(spMissionsDir, "SinglePlayer\\Mission%02d_NIS%02d\\", mission, nisNumber);
        sprintf(spMissionsFile, "Mission%02d_NIS%02d.mission", mission, nisNumber);
    }
    else
    {
        singlePlayerGameInfo.onLoadPlayNIS = 0;
        sprintf(spMissionsDir, "SinglePlayer\\Mission%02d\\", mission);
        sprintf(spMissionsFile, "Mission%02d.mission", mission);
    }
}

void singlePlayerKasMissionStart(sdword missionnum)
{
    switch (missionnum)
    {
        case 1: kasMissionStart("mission01", Init_Mission01, Watch_Mission01);  break;
        case 2: kasMissionStart("mission02", Init_Mission02, Watch_Mission02);  break;
        case 3: kasMissionStart("mission03", Init_Mission03, Watch_Mission03);  break;
        case 4: kasMissionStart("mission04", Init_Mission04, Watch_Mission04);  break;
#ifdef OEM
        case 5: kasMissionStart("mission05_OEM", Init_Mission05_OEM, Watch_Mission05_OEM);  break;
#else
        case 5: kasMissionStart("mission05", Init_Mission05, Watch_Mission05);  break;
        case 6: kasMissionStart("mission06", Init_Mission06, Watch_Mission06);  break;
        case 7: kasMissionStart("mission07", Init_Mission07, Watch_Mission07);  break;
        case 8: kasMissionStart("mission08", Init_Mission08, Watch_Mission08);  break;
        case 9: kasMissionStart("mission09", Init_Mission09, Watch_Mission09);  break;
        case 10: kasMissionStart("mission10", Init_Mission10, Watch_Mission10);  break;
        case 11: kasMissionStart("mission11", Init_Mission11, Watch_Mission11);  break;
        case 12: kasMissionStart("mission12", Init_Mission12, Watch_Mission12);  break;
        case 13: kasMissionStart("mission13", Init_Mission13, Watch_Mission13);  break;
        case 14: kasMissionStart("mission14", Init_Mission14, Watch_Mission14);  break;
        case 15: kasMissionStart("mission15", Init_Mission15, Watch_Mission15);  break;
        case 16: kasMissionStart("mission16", Init_Mission16, Watch_Mission16);  break;
#endif
        default:
            dbgFatalf(DBG_Loc, "\nUnsupported mission #%d", missionnum);
            break;
    }
}

void singlePlayerMissionScriptInit(char *directory, char *filename)
{
    universe.players[0].race = RACE_Sect;
    singlePlayerSetMissionAttributes(directory, filename);
    universe.players[1].race = RACE_Beast;
}

void singlePlayerNisStoppedCB(void)
{
    bool32 savefirstlevel = FALSE;
    sdword entryId;
    sdword *entry;
    char bufferstr[30];

    switch (singlePlayerNisNumber)
    {
        case 1:
        {
            GrowSelection growselect;
            universe.curPlayerPtr->PlayerMothership = FindPlayersMothership(universe.curPlayerPtr);
            dbgAssert(universe.curPlayerPtr->PlayerMothership != NULL);

            growSelectInit(&growselect);
            FindShipsOfShipTypeOfPlayer(&growselect, sRecon, universe.curPlayerPtr);
            if (growselect.selection->numShips > 0)
            {
                clFormation(&universe.mainCommandLayer, growselect.selection, CUSTOM_FORMATION);
            }
            growSelectClose(&growselect);

            savefirstlevel = TRUE;
        }
        break;
    }

    if (singlePlayerNisletNumber != -1)
    {
        sprintf(bufferstr, "G_Nislet%dComplete", singlePlayerNisletNumber);
        entryId = kasHashLabel(bufferstr);
    }
    else
    {
        dbgAssert(toupper(singlePlayerNISName[0]) == 'N');
        sprintf(bufferstr, "G_Nis%dComplete", singlePlayerNisNumber);
        entryId = kasHashLabel(bufferstr);
    }

    entry = kasfEntryFindOrCreateByScopeAndId(0, entryId);
    spUnlockout();
    dbgAssert(entry != NULL);
    entry[2] = TRUE;
    if (singlePlayerNisletNumber != -1 && singlePlayerGameInfo.currentMission == 1)
    {
        cat_mrWhiteOutT = 1.0f;
    }
    spClearScreenLockout();
    if (spPreservePostLoadNisMode)
    {
        if (spManagerScreenFlags)
        {
            spClearScreenLockout();
        }
        spManagerScreenFlags = SPLOCKOUT_MR;
        spScreenLockoutActive = TRUE;
        mrDisable();
        mouseCursorHide();
    }

    if (hyperspaceAtEndOfSinglePlayerNIS)
    {
        hyperspaceAtEndOfSinglePlayerNIS = FALSE;
        spHyperspaceButtonPushed();
    }

    if (singlePlayerNisNumber != -1 && missionLoadsLevelAfterNIS[singlePlayerGameInfo.currentMission])
    {
        singlePlayerPreserveCurrentMissionDuringLoad = TRUE;
        singlePlayerGameLoadNewLevelFlag = TRUE;
        singlePlayerHyperspacingInto = TRUE;
        singlePlayerGameInfo.hyperspaceState = HYPERSPACE_LOADDONE;
        singlePlayerLoadNewLevel(FALSE, FALSE);
        cat_mrBlackOut = 1.0f;
    }

    if (savefirstlevel)
    {
        kasfSaveLevel(1, strGetString(strSaveSinglePlayerLevel1));
    }
    singlePlayerNISName = NULL;
    singlePlayerNisletNumber = -1;
    singlePlayerNisNumber = -1;
}

void singlePlayerNISNamesGet(char **nisname, char **scriptname, bool32 *loadLevelBeforeStart, sdword nisNumber)
{
    singlePlayerNisNumber = nisNumber;
    nisNumber--;
    dbgAssert(nisNumber >= 0);
    dbgAssert(nisNumber < NUMBER_SINGLEPLAYER_NIS);

    if (universe.players[0].race == RACE_Sect)
    {
        *nisname = nisR1Names[nisNumber][0];
        *scriptname = nisR1Names[nisNumber][1];
    }
    else
    {
        *nisname = nisR2Names[nisNumber][0];
        *scriptname = nisR2Names[nisNumber][1];
    }
    singlePlayerNISName = *nisname;
    *loadLevelBeforeStart = (bool32)nisLoadLevelBeforeStart[nisNumber];
}

bool32 singlePlayerNISletNamesGet(char **nisname, char **scriptname, sdword nisletNumber)
{
    sdword insertNumber;
    char *baseName;
    static char nisFileName[32];
    static char nisScriptName[32];

    singlePlayerNisletNumber = nisletNumber;

    insertNumber = nisletNumber - 1;
    dbgAssert(nisletNumber > 0);
    dbgAssert(insertNumber < NISLETS_PER_LEVEL);
    baseName = nisletNames[insertNumber];
    if (baseName == NULL)
    {
        dbgFatalf(DBG_Loc, "\nNISlet %d is not defined", nisletNumber);
        return(FALSE);
    }
    strcpy(nisFileName, baseName);
    strcpy(nisScriptName, nisFileName);
    strcat(nisFileName, ".nis");
    strcat(nisScriptName, ".script");
#if NIS_PRINT_INFO
    if (!fileExists(nisFileName, 0) || !fileExists(nisScriptName, 0))
    {
        dbgMessagef("\n'%s' and/or '%s' do not exist.", nisFileName, nisScriptName);
        return(0);
    }
#endif
    *nisname = nisFileName;
    *scriptname = nisScriptName;
    singlePlayerNISName = *nisname;
    return(TRUE);
}

void singlePlayerStartNis(char *nisname, char *scriptname, hvector *positionAndAngle, bool32 loadLevelBeforeStart, bool32 suppressMissionAdvance)
{
    matrix thisIdentityMatrix;
    vector position;
    real32 angle;
    matrix rotz;
    matrix nisMatrix;
    vector motherVector;
    matrix motherMatrix;
    vector tempvec;
    Ship *nisCentreShip = NULL;
    bool32 centreShipFound = FALSE;
    GrowSelection *nisSelection;
    ShipRace raceOfCentreShip = (ShipRace)-1;
    ShipType typeOfCentreShip;
    sdword instanceOfCentreShip = -1;
    sdword *nisEntry;
    Node *node;
    Derelict *derelict;

    if (loadLevelBeforeStart)
    {
        singlePlayerPreserveCurrentMissionDuringLoad = FALSE;
        singlePlayerLoadNewLevel(TRUE, suppressMissionAdvance);
    }

    if (positionAndAngle != NULL)
    {
        vecGrabVecFromHVec(position, *positionAndAngle);
        angle = DEG_TO_RAD(positionAndAngle->w);
    }
    else
    {
        positionAndAngle = kasVectorPtrIfExists(kasHashLabel("StartPointPlayer"));
        if (positionAndAngle != NULL)
        {
            vecGrabVecFromHVec(position, *positionAndAngle);
            angle = DEG_TO_RAD(positionAndAngle->w);
        }
        else
        {
            position.x = 0.0f;
            position.y = 0.0f;
            position.z = 0.0f;
            angle = 0.0f;
        }
    }

    if (smSensorsActive)
    {
        smSensorsCloseForGood();
    }
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    spCleanupScreenStack();
    if (spManagerScreenFlags)
    {
        spClearScreenLockout();
    }
    spManagerScreenFlags = SPLOCKOUT_EVERYTHING;
    selSelected.numShips = 0;
    ioUpdateShipTotals();
    spScreenLockoutActive = TRUE;
    mrDisable();
    mouseCursorHide();
    mouseForceInvisible();

    thisNisDoneCB = singlePlayerNisStoppedCB;
    thisNisHeader = nisLoad(nisname, scriptname);

    nisSelection = kasGetGrowSelectionPtrIfExists("NisCentreShip");
    if ((nisSelection) && (nisSelection->selection->numShips > 0))
    {
        nisCentreShip = nisSelection->selection->ShipPtr[0];
        kasGrowSelectionClear(nisSelection);
        typeOfCentreShip = nisCentreShip->shiptype;
    }

    nisEntry = aivarFindEntry(0, (AIVar *)kasHashLabel("G_NisCentreDerelict"));
    if (nisEntry != NULL)
    {
        raceOfCentreShip = NSR_Derelict;
        typeOfCentreShip = nisEntry[2];
        nisCentreShip = NULL;
        for (node = universe.DerelictList.head; node != NULL; node = node->next)
        {
            derelict = (Derelict *)listGetStructOfNode(node);
            if (derelict->derelicttype == typeOfCentreShip)
            {
                nisCentreShip = (Ship *)derelict;
                break;
            }
        }
    }

    nisEntry = aivarFindEntry(0, (AIVar *)kasHashLabel("G_NisCentreInstance"));
    if (nisEntry != NULL)
    {
        instanceOfCentreShip = nisEntry[2];
    }

    if (nisCentreShip)
    {
        vector heading;
        vector normal = {1.0, 0.0, 0.0};
        real32 cross;

        centreShipFound = nisShipStartPosition(&motherVector, &motherMatrix, thisNisHeader, raceOfCentreShip, typeOfCentreShip, instanceOfCentreShip);
        position = nisCentreShip->posinfo.position;
        matGetVectFromMatrixCol3(heading, nisCentreShip->rotinfo.coordsys);
        heading.z = 0;
        vecNormalize(&heading);
        cross = vecDotProduct(heading, normal);
        if (heading.y < 0.0f)
        {
            angle = TWOPI - (real32)acos((double)cross);
        }
        else
        {
            angle = (real32)acos((double)cross);
        }
    }
    if (centreShipFound)
    {
        matCopyAndTranspose(&motherMatrix, &thisIdentityMatrix);
        matMultiplyMatByMat(&nisMatrix, &nisCentreShip->rotinfo.coordsys, &thisIdentityMatrix);

        matMultiplyMatByVec(&tempvec, &nisMatrix, &motherVector);
        vecSubFrom(position, tempvec);
    }
    else
    {
        thisIdentityMatrix = IdentityMatrix;
        matMakeRotAboutZ(&rotz, (real32)cos(angle), (real32)sin(angle));
        matMultiplyMatByMat(&nisMatrix, &thisIdentityMatrix, &rotz);
    }

    thisNisPlaying = nisStart(thisNisHeader, &position, &nisMatrix);
    nisTaskResume();
}

#define MAXLINENEED 100

static sdword WarpToLevelEnabled()
{
    if (warpScriptEnabled && warpScriptLevelToWarpTo > 1)
    {
        return warpScriptLevelToWarpTo;
    }
    return 0;
}

static bool32 WarpScriptEntryApplies(void)
{
    if (warpScriptCurrentLevel < 0)
    {
        dbgFatalf(DBG_Loc, "\nLevel not specified while parsing warp script file. (JS)");
    }
    return TRUE;
}

static void SetWarpLevelCB(char *directory,char *field,void *dataToFillIn)
{
    RemoveCommasFromString(field);
    sscanf(field,"%d",&warpScriptCurrentLevel);
    warpScriptCurrentLevel--;
    if ((warpScriptCurrentLevel < 0) || (warpScriptCurrentLevel >= NUMBER_SINGLEPLAYER_MISSIONS))
    {
        dbgFatalf(DBG_Loc,"\nInvalid Level specified in warp script file: %s",field);
    }
}

static void SetWarpResourcesCB(char *directory,char *field,void *dataToFillIn)
{
    WarpScriptLevelData *levelData;

    if (!WarpScriptEntryApplies())
    {
        return;
    }

    levelData = &warpScriptLevels[warpScriptCurrentLevel];
    scriptSetSdwordCB(directory, field, &levelData->resources);
}

static void SetWarpFleetCB(char *directory,char *field,void *dataToFillIn)
{
    WarpScriptLevelData *levelData;
    ShipType shiptype;
    char shiptypestr[50];
    sdword num = 0;

    RemoveCommasFromString(field);

    if (!WarpScriptEntryApplies())
    {
        return;
    }

    sscanf(field,"%s %d",shiptypestr,&num);

    shiptype = StrToShipType(shiptypestr);
    if (!(shiptype < TOTAL_NUM_SHIPS))
    {
        dbgFatalf(DBG_Loc,"Bad shiptype in %s",field);
    }

    if (num <= 0)
    {
        return;
    }

    levelData = &warpScriptLevels[warpScriptCurrentLevel];
    levelData->shipCounts[shiptype] = num;
}

static void SetWarpTechCB(char *directory,char *field,void *dataToFillIn)
{
    WarpScriptLevelData *levelData;
    TechnologyType techtype;
    char techtypestr[256];

    if (!WarpScriptEntryApplies())
    {
        return;
    }

    RemoveCommasFromString(field);
    sscanf(field, "%255s", techtypestr);
    techtype = StrToTechType(techtypestr);
    if (techtype == -1)
    {
        dbgWarningf(DBG_Loc, "\nUnsupported warp tech in %s", field);
        return;
    }

    levelData = &warpScriptLevels[warpScriptCurrentLevel];
    levelData->techStates[techtype] = 1;
}

static Ship *WarpInFleetComponentHost(Ship *component, Ship *mothership, Ship **dockwith, sdword dockwithCount, bool32 *abandon)
{
    sdword carrierIndex;

    *abandon = FALSE;

    /* Retail (0x51baf7) only searches dockwith[] (the sCarrier ships warped in
     * so far) when the component being placed is itself sCarrierSupport. Every
     * other CLASS_Component type, including sMothershipSupport, attaches
     * directly to the mothership with no search. */
    if (component->shiptype != sCarrierSupport)
    {
        return mothership;
    }

    if (dockwithCount <= 0)
    {
        /* Retail (0x51bafe-0x51bb02) abandons creating any further copies of
         * this shiptype for the remainder of the warp-in batch when no
         * sCarrier exists yet to host it, rather than falling back to the
         * mothership. */
        *abandon = TRUE;
        return NULL;
    }

    for (carrierIndex = 0; carrierIndex < dockwithCount; carrierIndex++)
    {
        Ship *carrier = dockwith[carrierIndex];
        DockStaticInfo *dockStaticInfo = carrier->staticinfo->dockStaticInfo;
        DockPointActivity *dockPointActivity = carrier->dockvars.dockPointActivity;

        if (dockPointActivity->numDockedOfType[DPT_Support_EB] < dockStaticInfo->numPointsOfType[DPT_Support_EB])
        {
            return carrier;
        }
    }

    return dockwith[0];
}

static void WarpInFleetPositionSupportModule(Ship *component, Ship *host)
{
    ubyte supportPointIndex;
    DockStaticInfo *dockStaticInfo = host->staticinfo->dockStaticInfo;
    DockPointActivity *dockPointActivity = host->dockvars.dockPointActivity;

    supportPointIndex = dockStaticInfo->firstPointOfType[DPT_Support_EB];
    if (supportPointIndex != (ubyte)-1)
    {
        DockStaticPoint *supportPoint;

        if (dockPointActivity->numDockedOfType[DPT_Support_EB] < dockStaticInfo->numPointsOfType[DPT_Support_EB])
        {
            supportPointIndex = (ubyte)(supportPointIndex + dockPointActivity->numDockedOfType[DPT_Support_EB]);
            component->dockvars.dockship = host;
            dockReserveDockPoint(component, host, supportPointIndex);
        }

        supportPoint = &dockStaticInfo->dockstaticpoints[supportPointIndex];
        matMultiplyMatByVec(&component->posinfo.position, &host->rotinfo.coordsys, &supportPoint->position);
        component->posinfo.position.x += host->posinfo.position.x;
        component->posinfo.position.y += host->posinfo.position.y;
        component->posinfo.position.z += host->posinfo.position.z;

        if (((supportPointIndex - dockStaticInfo->firstPointOfType[DPT_Support_EB]) & 1) != 0)
        {
            matrix flipMatrix;
            matrix originalRotation;
            real32 flipAngle = DEG_TO_RAD(180.0f);

            matMakeRotAboutX(&flipMatrix, (real32)cos(flipAngle), (real32)sin(flipAngle));
            originalRotation = component->rotinfo.coordsys;
            matMultiplyMatByMat(&component->rotinfo.coordsys, &originalRotation, &flipMatrix);
        }
    }
}

void WarpInFleet(hvector *startPoint)
{
    WarpScriptLevelData *levelData;
    Node *node;
    Ship *ship;
    Ship *componentHost;
    Ship *dockwith[8];
    sdword dockwithCount = 0;
    ShipType shiptype;
    sdword shipCount;
    sdword shipIndex;
    vector zeroVector = { 0.0f, 0.0f, 0.0f };
    Player *player = &universe.players[0];
    (void)startPoint;

    dbgAssert(warpLevel > 0);
    levelData = &warpScriptLevels[warpLevel - 1];
    memset(dockwith, 0, sizeof(dockwith));

    ship = univCreateShip(sMothership, universe.players[0].race, &zeroVector, &universe.players[0], 0);
    gameStatsAddShip(ship,0);
    cat_unitCapCreateShip(ship,player);
    universe.players[0].PlayerMothership = ship;

    for (shiptype = sRecon; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        shipCount = levelData->shipCounts[shiptype];
        for (shipIndex = 0; shipIndex < shipCount; shipIndex++)
        {
            ship = univCreateShip(shiptype, universe.players[0].race, &zeroVector, &universe.players[0], 0);
            gameStatsAddShip(ship, 0);
            cat_unitCapCreateShip(ship, player);
            GroupShipIntoMilitaryParade(&universe.mainCommandLayer, ship, universe.players[0].PlayerMothership);

            if ((ship->shiptype == sCarrier) && (dockwithCount < sizeof(dockwith) / sizeof(dockwith[0])))
            {
                dockwith[dockwithCount++] = ship;
            }

            if (ship->staticinfo->shipclass == CLASS_Component)
            {
                bool32 abandon;

                componentHost = WarpInFleetComponentHost(ship, universe.players[0].PlayerMothership, dockwith, dockwithCount, &abandon);
                if (abandon)
                {
                    break;
                }

                if ((ship->shiptype == sCarrierSupport) || (ship->shiptype == sMothershipSupport))
                {
                    WarpInFleetPositionSupportModule(ship, componentHost);
                }

                clampObjectSetup((SpaceObj *)ship, (SpaceObj *)componentHost);
                ModuleCompleted(&universe.players[0], ship->shiptype, FALSE);
            }
        }
    }

    clPresetShipsToPosition(&universe.mainCommandLayer);
    universeSwitchToPlayer(0);

    player->resourceUnits = levelData->resources;
    rmOrPrereqMasks(rmPlayerCurrentPrereqMask(player), rmPlayerCurrentPrereqMask(player), levelData->techStates);
    rmUpdateBuildCapabilities(player);

    for (node = universe.ShipList.head; node != NULL; )
    {
        Ship *playerShip = (Ship *)listGetStructOfNode(node);

        if (playerShip->playerowner == universe.curPlayerPtr)
        {
            CalculateHyperspaceTimeForShip(playerShip, universe.curPlayerPtr->PlayerMothership);
            playerShip->shipSinglePlayerGameInfo->shipHyperspaceState = SHIPHYPERSPACE_GONE;
            if ((playerShip->flags & SOF_DontDrawTrails) == 0)
            {
                univRemoveShipFromOutside(playerShip);
                AddShipToHyperspace(playerShip);
                playerShip->shipSinglePlayerGameInfo->midlevelHyperspaceOut = FALSE;
                shipApplyPlayerUpgrades(playerShip);
                node = universe.ShipList.head;
                continue;
            }
            shipApplyPlayerUpgrades(playerShip);
        }

        node = node->next;
    }

    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_LOADDONE;
    singlePlayerLoadNewLevel(FALSE, TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : singleHackResearchWarp
    Description : A function that hard codes the research a player is has already
                  had based on the settings daly told me they would be in each level
                  ugly stuff... replace with an elegant solution later
    Inputs      :
    Outputs     : Allows the player to research certain topics
    Return      : void
----------------------------------------------------------------------------*/
void singleHackResearchWarp(void)
{
    switch (warpLevel)
    {
        case 16:
        case 15:
            PlayerAcquiredTechnology("HeavyGuns");
        case 14:
            AllowPlayerToResearch("HeavyGuns");
            PlayerAcquiredTechnology("CloakDefenseFighter");
        case 13:
            PlayerAcquiredTechnology("MissileWeapons");
            AllowPlayerToResearch("CloakDefenseFighter");
        case 12:
            PlayerAcquiredTechnology("GravityWellGeneratorTech");
        case 11:
            AllowPlayerToResearch("GravityWellGeneratorTech");
            PlayerAcquiredTechnology("ProximityDetector");
            PlayerAcquiredTechnology("RepairTech");
        case 10:
            AllowPlayerToResearch("RepairTech");
            AllowPlayerToResearch("MissileWeapons");
        case 9:
            PlayerAcquiredTechnology("DDDFDFGFTech");
            PlayerAcquiredTechnology("TargetingSystems");
        case 8:
            AllowPlayerToResearch("TargetingSystems");
        case 7:
            AllowPlayerToResearch("DDDFDFGFTech");
            PlayerAcquiredTechnology("MassDrive1Mt");
            PlayerAcquiredTechnology("FireControl");
        case 6:                                       // Mission 06: GIVE: Plasma Weapons
            PlayerAcquiredTechnology("PlasmaWeapons");
            AllowPlayerToResearch("MassDrive1Mt");
            AllowPlayerToResearch("FireControl");
        case 5:                                       // Mission 05: GIVE: Capital Ship Chassis, Ion Weapons, ALLOW: Plasma Weapons
            AllowPlayerToResearch("PlasmaWeapons");
            PlayerAcquiredTechnology("Chassis3");
            PlayerAcquiredTechnology("IonWeapons");
        case 4:                                       // Mission 04: GIVE: Capital Ship Drive, ALLOW: Capital Ship Chassis
            PlayerAcquiredTechnology("MassDrive100Kt");
            AllowPlayerToResearch("Chassis3");
        case 3:                                       // Mission 03: GIVE: Corvette Chassis, Heavy Corvette
            PlayerAcquiredTechnology("Chassis2");
            PlayerAcquiredTechnology("MediumGuns");
        case 2:                                       // Mission 02: GIVE: Fighter Chassis, Corvette Drive
            PlayerAcquiredTechnology("Chassis1");
            PlayerAcquiredTechnology("MassDrive10Kt");
        case 1:                                       // Mission 01, GIVE: Fighter Drive, ALLOW: Fighter Chassis
            PlayerAcquiredTechnology("MassDrive1Kt");
            AllowPlayerToResearch("Chassis1");
    }
}


void singlePlayerStartGame(void)
{
    hvector startPoint;
    char *nis, *script;
    bool32 centre;

    universe.players[1].resourceUnits = 0;

    if (warpLevel)
    {
        dbgAssert(singlePlayerGameInfo.currentMission == warpLevel);

        universeSwitchToPlayer(0);

        GetMissionsDirAndFile(singlePlayerGameInfo.currentMission, FALSE);

        SetFleetModifier(singlePlayerGameInfo.currentMission, GetFleetStrengthWarpScript);
        levelStartNext(spMissionsDir,spMissionsFile);

        GetStartPointPlayer(&startPoint);
        WarpInFleet(&startPoint);

        singlePlayerLevelLoaded();

        singleHackResearchWarp();

        ccFocusOnPlayersMothership(&universe.mainCameraCommand,0);

        soundEventPlayMusic(SongNumber);

        singlePlayerKasMissionStart(singlePlayerGameInfo.currentMission);

        if (singlePlayerGameInfo.onLoadPlayNIS > 0)
        {
            hvector startpoint;

            GetStartPointPlayer(&startpoint);
            singlePlayerNISNamesGet(&nis, &script, &centre, singlePlayerGameInfo.onLoadPlayNIS);
            singlePlayerStartNis(nis, script, &startpoint, centre, TRUE);
        }

        return;
    }

    tmTechInit();
    spFleetModifier = 0.0f;
    GetMissionsDirAndFile(1, TRUE);
    levelInit(spMissionsDir,spMissionsFile);
    singlePlayerLevelLoaded();
    singlePlayerKasMissionStart(1);
    GetStartPointPlayer(&startPoint);
    singlePlayerNISNamesGet(&nis, &script, &centre, 1);
    singlePlayerStartNis(nis, script, &startPoint, centre, FALSE);
}

void singlePlayerNextLevel(void)
{
    hvector startnisAt = { 0.0f, 0.0f, 0.0f, 0.0f };
    char *nis, *script;
    bool32 centre;

    if (singlePlayerGameInfo.currentMission == 16)
    {
        singlePlayerMissionCompleteCB();
        return;
    }

    if (singlePlayerGameInfo.onCompletePlayNIS > 0)
    {
        singlePlayerNISNamesGet(&nis, &script, &centre, singlePlayerGameInfo.onCompletePlayNIS);
        singlePlayerStartNis(nis, script, &startnisAt, centre, FALSE);
        if (singlePlayerGameInfo.onCompleteHyperspace)
        {
            hyperspaceAtEndOfSinglePlayerNIS = TRUE;
        }
        else
        {
            dbgFatal(DBG_Loc,"\nNot hyperspacing at end of level not supported yet");
        }
        return;
    }

    if (singlePlayerGameInfo.onCompleteHyperspace)
    {
        if (hyperspaceOverride)
        {
            spHyperspaceButtonPushed();
        }
    }
}

void singlePlayerMissionCompleteCB(void)
{
    selSelected.numShips = 0;
    ioUpdateShipTotals();
    mrRenderMainScreen = FALSE;
    universe.quittime = universe.realtimeelapsed;
    // this will prevent the shiplist from being displayed!
    utyCreditsSequence = TRUE;
}

void singlePlayerMissionFailedCB(void)
{
    speechEventQueue(0, 33558568, -1, -1, 0, 0, -1, 5.0f, -1);
    univKillPlayer(0, PLAYERKILLED_LOSTMISSION);
    CheckPlayerWin();
}

#if SP_DEBUGKEYS
void singlePlayerCheckDebugKeys(sdword ID)
{
#if SP_DEBUGLEVEL2
    sdword state;
#endif

    switch (ID)                 // general stuff
    {
        case RKEY:
            {
                sdword total;
                sdword numHarvestables;
                sdword numAsteroid0s;
                char totalstr[50];
                univGetResourceStatistics(&total,&numHarvestables,&numAsteroid0s);
                sprintf(totalstr,"RV=%d #R=%d #A0=%d",total,numHarvestables,numAsteroid0s);
                clCommandMessage(totalstr);
            }
            return;
    }

    if (singlePlayerGame)       // singlePlayerGame specific stuff
    switch (ID)
    {
        case CKEY:
            hyperspaceOverride = TRUE;
            singlePlayerNextLevel();
            break;

        case FKEY:
            singlePlayerMissionFailedCB();
            break;

        case EKEY:
            {
                char totalstr[50];
                GetFleetStrengthInUniverseOfPlayer(&universe.players[0]);
                sprintf(totalstr,"Fleet Str = %f",spFleetStr);
                clCommandMessage(totalstr);
            }
            break;
        case DKEY:
            kasTakeADump();
            break;

#if SP_DEBUGLEVEL2
        // debug overlays
        case SKEY: // states
            mrKASDebugDrawStates ^= TRUE;
            break;
        case TKEY: // timers
            mrKASDebugDrawTimers ^= TRUE;
            break;
        case VKEY: // vars
            mrKASDebugDrawVars ^= TRUE;
            break;
        case ZKEY: // volumes
            mrKASDebugDrawVolumes ^= TRUE;
            break;
        case AKEY: // all
            state = !(mrKASDebugDrawStates && mrKASDebugDrawTimers && mrKASDebugDrawVars && mrKASDebugDrawVolumes);
            mrKASDebugDrawStates = state;
            mrKASDebugDrawTimers = state;
            mrKASDebugDrawVars = state;
            mrKASDebugDrawVolumes = state;
            break;
        case PAGEDOWNKEY:
            mrKASSkipFactor++;
            break;
#endif //SP_DEBUGLEVEL2
    }
}
#endif //SP_DEBUGKEYS

/*-----------------------------------------------------------------------------
    Name        : spNISletTestAttempt
    Description : Attempt to test an nislet for the current level
    Inputs      : index - number of NISlet for the current level (0..2)
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
#if SP_NISLET_TEST
void spNISletTestAttempt(sdword index)
{
    hvector startpoint;
    char *nis, *script;

    if (index <= 0 || index > NISLETS_PER_LEVEL)
    {
        dbgMessage("\nindex <= 0 || index > NISLETS_PER_LEVEL");
        return;
    }
    GetStartPointPlayer(&startpoint);
    dbgAssert(singlePlayerNISletNamesGet(&nis, &script, index));
    if (!fileExists(nis, 0))
    {
        dbgMessagef("\n'%s' not found.", nis);
        return;
    }
    if (!fileExists(script, 0))
    {
        dbgMessagef("\n'%s' not found.", script);
        return;
    }
    singlePlayerStartNis(nis, script, &startpoint, FALSE, FALSE);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : spFindCameraAttitude
    Description : Find the level's default camera poistion, if any.
    Inputs      :
    Outputs     : position - position for the camera's eye point
    Return      : TRUE if the camera position could be computed, FALSE otherwise
----------------------------------------------------------------------------*/
bool32 spFindCameraAttitude(vector *position)
{
    hvector *hPosition = kasVectorPtrIfExists(SP_CAMERA_POINT_NAME);

    if (hPosition == NULL)
    {
        return(FALSE);
    }
    position->x = hPosition->x;
    position->y = hPosition->y;
    position->z = hPosition->z;
    return(TRUE);
}

/*=============================================================================
    Save Game Stuff
=============================================================================*/

// Additional NIS scissor/widescreen state (5th variable and beyond at binary offset A4CB48+)
static real32 spNisFlt_A4CB48 = 0.0f;     // 5th NIS scissor variable
static sdword spNisByte_A4CB4C = 0;       // NIS mode byte flag
static real32 spNisFlt_A4CB50 = 0.0f;    // additional NIS float
static sdword spNisDword_A4CB54 = 0;     // additional NIS dword
sdword nisWideScreenActive = 0;

void _cfltcvt_init_0(void)
{
    spNisFlt_A4CB50 = 0.0f;
    spNisFlt_A4CB48 = 0.0f;
    nisWideScreenActive = 0;
    spManagerScreenFlags = 0;
    nisScissorFade = 0.0f;
    nisScissorFadeOut = 0.0f;
    nisScissorFadeIn = 0.0f;
}

// Pre-player-data SP state
sdword spStateA5B1AC = 0;          // dword_A5B1AC

// Mothership big gun launch flag (also used by kasfDisableMothershipBigGunLaunch)
sdword kasfMothershipBigGunDisabled = 0;   // dword_894108

// ShipMimic per-player ship type data (placeholder arrays matching binary's smMimic layout)
#define SM_SHIP_TYPES_PER_PLAYER 84
#define SM_MAX_PLAYERS           (MAX_MULTIPLAYER_PLAYERS + 1)

static sdword smPlayerTypeExtra0[SM_MAX_PLAYERS];   // dword_A77AB0
static sdword smPlayerTypeExtra1[SM_MAX_PLAYERS];   // dword_A762E8
static sdword smPlayerTypeData[SM_MAX_PLAYERS * SM_SHIP_TYPES_PER_PLAYER * 2]; // dword_A76310 (stride-2 access)
static sdword smFixedData[4 * 2];  // dword_A77814..A7782C (3 serialized entries, stride-2; index 6 stays runtime-initialized)
static sdword smGlobalDataA = 0;   // dword_A77AD0
static sdword smGlobalDataB = 0;   // dword_A76308
static sdword smGlobalDataC = 0;   // dword_A77AD4 (byte-sized)
static sdword smMimicObjectEntries[4][2];

static void smMimicRefreshObjectEntries(void)
{
    memset(smMimicObjectEntries, 0, sizeof(smMimicObjectEntries));
    smMimicObjectEntries[0][1] = smFixedData[0];
    smMimicObjectEntries[1][1] = smFixedData[2];
    smMimicObjectEntries[2][1] = smFixedData[4];
    smMimicObjectEntries[3][1] = smFixedData[6];
}

void smMimicInitPlayerShipTypes(void)
{
    sdword playerIndex;
    sdword numPlayers;
    sdword race;
    sdword *playerShipTypes;

    if (singlePlayerGame)
    {
        smPlayerTypeExtra0[0] = 0;
        smPlayerTypeExtra1[0] = 0;
        smPlayerTypeExtra0[1] = 0;
        smPlayerTypeExtra1[1] = 0;
        smGlobalDataA = 0;
        smGlobalDataB = 0;
        smGlobalDataC = 0;
        smMimicRefreshObjectEntries();
        return;
    }

    numPlayers = universe.numPlayers;
    for (playerIndex = 0; playerIndex < numPlayers; playerIndex++)
    {
        playerShipTypes = &smPlayerTypeData[playerIndex * SM_SHIP_TYPES_PER_PLAYER * 2];
        race = universe.players[playerIndex].race;
        if (race == 0)
        {
            playerShipTypes[0] = 0;
            playerShipTypes[2] = 1;
            playerShipTypes[4] = 3;
            playerShipTypes[6] = 17;
            playerShipTypes[8] = 4;
            playerShipTypes[10] = 14;
            playerShipTypes[12] = 7;
            playerShipTypes[14] = 6;
            playerShipTypes[16] = 8;
            smPlayerTypeExtra0[playerIndex] = 9;
            smPlayerTypeExtra1[playerIndex] = 5;
        }
        else if (race == 1)
        {
            playerShipTypes[0] = 33;
            playerShipTypes[2] = 52;
            playerShipTypes[4] = 35;
            playerShipTypes[6] = 37;
            playerShipTypes[8] = 38;
            playerShipTypes[10] = 49;
            playerShipTypes[12] = 41;
            playerShipTypes[14] = 42;
            playerShipTypes[16] = 48;
            playerShipTypes[18] = 45;
            smPlayerTypeExtra0[playerIndex] = 10;
            smPlayerTypeExtra1[playerIndex] = 5;
        }
    }

    while (playerIndex < SM_MAX_PLAYERS)
    {
        smPlayerTypeExtra0[playerIndex++] = 0;
    }

    smFixedData[0] = 0;
    smFixedData[2] = 1;
    smFixedData[4] = 2;
    smFixedData[6] = 3;
    smGlobalDataA = 4;
    smGlobalDataB = 3;
    smGlobalDataC = 0;
    smMimicRefreshObjectEntries();
}

sdword smMimicGetCurrentPlayerIndex(void)
{
    return smGlobalDataC & 0xFF;
}

void smMimicSetCurrentPlayerIndex(sdword playerIndex)
{
    smGlobalDataC = playerIndex & 0xFF;
}

bool32 smMimicIsPlayerIndexValid(sdword playerIndex)
{
    if (singlePlayerGame)
    {
        switch (playerIndex)
        {
            case 0:
            case 1:
            case 8:
            case 9:
                return TRUE;

            default:
                return FALSE;
        }
    }

    return playerIndex == 8 || playerIndex < universe.numPlayers;
}

char *smMimicGetPlayerLabel(sdword playerIndex)
{
    if (playerIndex >= 8)
    {
        return strGetString(strMimicTypeObjects);
    }
    if (singlePlayerGame)
    {
        return strGetString((playerIndex == 1) ? strMimicTypeComputerPlayer : strMimicTypeCurrentPlayer);
    }

    return playerNames[playerIndex];
}

sdword smMimicGetEntryCount(sdword playerIndex, sdword groupIndex)
{
    if (playerIndex == 8)
    {
        return (groupIndex != 0) ? smGlobalDataA : smGlobalDataB;
    }

    return (groupIndex != 0) ? smPlayerTypeExtra0[playerIndex] : smPlayerTypeExtra1[playerIndex];
}

void *smMimicGetEntryAddress(sdword playerIndex, sdword groupIndex, sdword entryIndex)
{
    (void)groupIndex;
    if (playerIndex == 8)
    {
        return smMimicObjectEntries[entryIndex];
    }

    return &smPlayerTypeData[(entryIndex + SM_SHIP_TYPES_PER_PLAYER * playerIndex) * 2];
}

char *smMimicGetEntryLabel(sdword playerIndex, sdword groupIndex, sdword entryIndex)
{
    sdword *entryData;

    (void)groupIndex;
    if (playerIndex <= 7)
    {
        entryData = smMimicGetEntryAddress(playerIndex, groupIndex, entryIndex);
        return ShipTypeToNiceStr(entryData[0]);
    }
    if (playerIndex == 8)
    {
        entryData = smMimicGetEntryAddress(playerIndex, groupIndex, entryIndex);
        return strGetString(strMimicItemRandomAsteroid + entryData[1]);
    }

    return NULL;
}

sdword smMimicAddShipType(ubyte orderGroup, sdword shipType)
{
    sdword count;

    if (orderGroup == 8)
    {
        smFixedData[0] = 0;
        smFixedData[2] = 1;
        smFixedData[4] = 2;
        smFixedData[6] = 3;
        smGlobalDataA = 4;
        smGlobalDataB = 3;
        smMimicRefreshObjectEntries();
        return 3;
    }

    count = smPlayerTypeExtra0[orderGroup];
    smPlayerTypeData[(orderGroup * SM_SHIP_TYPES_PER_PLAYER + count) * 2] = shipType;
    smPlayerTypeExtra0[orderGroup] = count + 1;
    return orderGroup;
}

sdword titan_5BA150(ubyte orderGroup, sdword count)
{
    if (orderGroup == 8)
    {
        smGlobalDataB = count;
        return orderGroup;
    }

    smPlayerTypeExtra1[orderGroup] = count;
    return orderGroup;
}

void smMimicSetPlayerShipTypeExtra1(ubyte playerIndex, sdword value)
{
    dbgAssert(playerIndex < SM_MAX_PLAYERS);
    smPlayerTypeExtra1[playerIndex] = value;
}

sdword *smMimicPlayerTypeData(void)
{
    return smPlayerTypeData;
}

sdword *smMimicPlayerTypeExtra0(void)
{
    return smPlayerTypeExtra0;
}

sdword *smMimicPlayerTypeExtra1(void)
{
    return smPlayerTypeExtra1;
}

static void smMimicSavePlayerShipTypes(void)
{
    sdword player, i;
    sdword *v1 = smPlayerTypeData;

    for (player = 0; player < universe.numPlayers; player++)
    {
        SaveInfoNumber(smPlayerTypeExtra0[player]);
        SaveInfoNumber(smPlayerTypeExtra1[player]);
        for (i = 0; i < SM_SHIP_TYPES_PER_PLAYER; i++)
        {
            SaveInfoNumber(*v1);
            v1 += 2;
        }
    }
    for (i = 0; i < 3; i++)
    {
        SaveInfoNumber(smFixedData[i * 2]);
    }
    SaveInfoNumber(smGlobalDataA);
    SaveInfoNumber(smGlobalDataB);
    SaveInfoNumber(smGlobalDataC);
}

static void smMimicLoadPlayerShipTypes(void)
{
    sdword player, i;
    sdword *v1 = smPlayerTypeData;

    for (player = 0; player < universe.numPlayers; player++)
    {
        smPlayerTypeExtra0[player] = LoadInfoNumber();
        smPlayerTypeExtra1[player] = LoadInfoNumber();
        for (i = 0; i < SM_SHIP_TYPES_PER_PLAYER; i++)
        {
            *v1 = LoadInfoNumber();
            v1 += 2;
        }
    }
    for (i = 0; i < 3; i++)
    {
        smFixedData[i * 2] = LoadInfoNumber();
    }
    smGlobalDataA = LoadInfoNumber();
    smGlobalDataB = LoadInfoNumber();
    smGlobalDataC = LoadInfoNumber() & 0xFF;
    smMimicRefreshObjectEntries();
}

void SaveSinglePlayerGame(void)
{
    dbgAssert(singlePlayerGameLoadNewLevelFlag == FALSE);

    SaveStructureOfSize(&singlePlayerGameInfo, SINGLEPLAYERGAMEINFO_SAVE_SIZE);

    SaveLinkedListOfInsideShips(&singlePlayerGameInfo.ShipsInHyperspace);
    SaveGrowSelection(&singlePlayerGameInfo.ShipsToHyperspace);

    SaveInfoNumber(spManagerScreenFlags);
    SaveInfoNumber(TreatAsUdword(nisScissorFadeIn));
    SaveInfoNumber(TreatAsUdword(nisScissorFadeOut));
    SaveInfoNumber(TreatAsUdword(nisScissorFade));
    SaveInfoNumber(TreatAsUdword(spNisFlt_A4CB48));
    SaveInfoNumber(spNisByte_A4CB4C);
    SaveInfoNumber(TreatAsUdword(spNisFlt_A4CB50));
    SaveInfoNumber(spNisDword_A4CB54);
    SaveInfoNumber(nisWideScreenActive);
    SaveInfoNumber(spStateA5B1AC);
    smMimicSavePlayerShipTypes();
    SaveInfoNumber(kasfMothershipBigGunDisabled);
}

void LoadSinglePlayerGame(void)
{
    sdword flags;

    singlePlayerGameLoadNewLevelFlag = FALSE;

    LoadStructureOfSizeToAddress(&singlePlayerGameInfo, SINGLEPLAYERGAMEINFO_SAVE_SIZE);

    LoadLinkedListOfInsideShips(&singlePlayerGameInfo.ShipsInHyperspace);
    FixLinkedListOfInsideShips(&singlePlayerGameInfo.ShipsInHyperspace);
    LoadGrowSelectionAndFix(&singlePlayerGameInfo.ShipsToHyperspace);

    flags = LoadInfoNumber();
    spManagerScreenFlags = flags;
    if (flags & 4)
    {
        selSelected.numShips = 0;
        ioUpdateShipTotals();
        flags = spManagerScreenFlags;
    }
    if (flags)
        spScreenLockoutActive = 1;
    if (flags & 2)
    {
        mrDisable();
        mouseCursorHide();
    }
    if (flags & 1)
        mouseForceInvisible();

    TreatAsUdword(nisScissorFadeIn)  = (udword)LoadInfoNumber();
    TreatAsUdword(nisScissorFadeOut) = (udword)LoadInfoNumber();
    TreatAsUdword(nisScissorFade)    = (udword)LoadInfoNumber();
    TreatAsUdword(spNisFlt_A4CB48)  = (udword)LoadInfoNumber();
    spNisByte_A4CB4C                 = LoadInfoNumber() & 0xFF;
    TreatAsUdword(spNisFlt_A4CB50)  = (udword)LoadInfoNumber();
    spNisDword_A4CB54                = LoadInfoNumber();
    nisWideScreenActive              = LoadInfoNumber();
    nisTaskResume();

    spStateA5B1AC                    = LoadInfoNumber();
    smMimicLoadPlayerShipTypes();
    kasfMothershipBigGunDisabled     = LoadInfoNumber();
}



SelectCommand *spBuildFleetSelection(void)
{
    SelectCommand *selection;
    Ship **selectedShips;
    Node *node;
    sdword count;

    selection = (SelectCommand *)memAlloc(4 * universe.ShipList.num + 8, "spBuildFleetSelection", MBF_Volatile);
    selectedShips = selection->ShipPtr;
    count = 0;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = (Ship *)node->structptr;

        if (!allianceIsShipAlly(ship, universe.curPlayerPtr))
        {
            continue;
        }
        if (ship->staticinfo->shipIsCapital || ship->staticinfo->shipclass == CLASS_Component)
        {
            continue;
        }

        *selectedShips = ship;
        selectedShips++;
        count++;
    }

    selection->numShips = count;
    ioUpdateShipTotals();
    return selection;
}

typedef struct singleplayerfrontendstack
{
   sdword unused0;
   sdword unused4;
   sdword unused8;
   void *data12;
   void *data16;
} SinglePlayerFrontendStack;

static sdword spScreenStackDepth = 0;
static SinglePlayerFrontendStack *spCurrentScreenData = NULL;

void spCleanupScreenStack(void)
{
   SinglePlayerFrontendStack *screenData;

   while (spScreenStackDepth != 0)
   {
       spScreenStackDepth--;
       feCurrentScreenDelete();
       if (spCurrentScreenData != NULL)
       {
           break;
       }
   }

   while (spScreenStackDepth == 0 && spCurrentScreenData != NULL)
   {
       screenData = spCurrentScreenData;
       if (screenData->data16 != NULL)
       {
           memFree(screenData->data16);
       }
       if (screenData->data12 != NULL)
       {
           memFree(screenData->data12);
       }
       memFree(screenData);
       spCurrentScreenData = NULL;
   }
}

static bool32 spHyperspaceShipNeedsHold(Ship *ship)
{
   if (!allianceIsShipAlly(ship, universe.curPlayerPtr))
   {
       return FALSE;
   }
   if (!(ship->specialFlags & SPECIAL_SinglePlayerWithinHyperSpaceRange))
   {
       return FALSE;
   }

   return ship->staticinfo->engineNozzleOffset[1].x != 0.0f ||
          ship->staticinfo->shipclass == CLASS_Component;
}

static bool32 spAnyTrackedHyperspaceShipInState(ShipHyperspaceState state)
{
   Node *node;

   for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
   {
       InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
       Ship *trackedShip = insideShip->ship;

       dbgAssert(trackedShip->objtype == OBJ_ShipType);
       if (spHyperspaceShipNeedsHold(trackedShip) &&
           trackedShip->shipSinglePlayerGameInfo->shipHyperspaceState == state)
       {
           return TRUE;
       }
   }

   return FALSE;
}

static bool32 spAnyOtherTrackedHyperspaceEffectActive(Ship *ship)
{
   Node *node;

   for (node = universe.ShipList.head; node != NULL; node = node->next)
   {
       Ship *otherShip = (Ship *)listGetStructOfNode(node);

       dbgAssert(otherShip->objtype == OBJ_ShipType);
       if (otherShip != ship &&
           spHyperspaceShipNeedsHold(otherShip) &&
           otherShip->shipSinglePlayerGameInfo->hsState != HS_INACTIVE)
       {
           return TRUE;
       }
   }

   return FALSE;
}

sdword spUpdateShipHyperspace(Ship *ship, hvector *mothershipPos, bool32 instantComplete)
{
   ShipSinglePlayerGameInfo *spInfo;
   CommandToDo *command;
   Ship *commandShip;
   vector relativePosition;
   vector createPosition;
   vector createVelocity;
   matrix rotation;
   real32 angle;
   real32 waitTime;
   sdword hyperspaceFinished;

   spInfo = ship->shipSinglePlayerGameInfo;
   ship->autostabilizeship = 0;
   ship->shipidle = 0;
   hyperspaceFinished = FALSE;
   if (spInfo->shipHyperspaceState == SHIPHYPERSPACE_GONE)
   {
       vecSet(relativePosition, spInfo->forbehind, -spInfo->rightleft, spInfo->updown);
       vecZeroVector(createVelocity);

       angle = DEG_TO_RAD(mothershipPos->w);
       matMakeRotAboutZ(&rotation, (real32)cos(angle), (real32)sin(angle));
       matMultiplyMatByVec(&createPosition, &rotation, &relativePosition);
       createPosition.x += mothershipPos->x;
       createPosition.y += mothershipPos->y;
       createPosition.z += mothershipPos->z;

       if (!singlePlayerGame)
       {
           createPosition.x = min(max(createPosition.x, -smUniverseSizeX), smUniverseSizeX);
           createPosition.y = min(max(createPosition.y, -smUniverseSizeY), smUniverseSizeY);
           createPosition.z = min(max(createPosition.z, -smUniverseSizeZ), smUniverseSizeZ);
       }

       PutShipOutsideAt(ship, &createPosition, &createVelocity, angle);
       bitClear(ship->flags, SOF_DontDrawTrails);
       spInfo->shipHyperspaceState = SHIPHYPERSPACE_DECELERATE;
       if (instantComplete)
       {
           spInfo->timeToHyperspace = 0.0f;
       }
       else
       {
           if (ship == universe.curPlayerPtr->PlayerMothership ||
               (ship->clampInfo != NULL && (Ship *)ship->clampInfo->host == ship))
           {
               waitTime = 2.0f * HYPERSPACE_BASE_WAITTIME;
           }
           else
           {
               waitTime = 3.0f * HYPERSPACE_BASE_WAITTIME + frandom(HYPERSPACE_RANDOM_WAITTIME);
           }
           spInfo->timeToHyperspace = universe.totaltimeelapsed + waitTime;
       }
       if (ship == universe.curPlayerPtr->PlayerMothership && !instantComplete)
       {
           ccFocusOnPlayersMothership(&universe.mainCameraCommand, 0);
           if (byte_8B8844)
           {
               ccSetDirtyBit(&pilotEffect);
           }
       }
       ship->flags |= SOF_DockMarker;
       if (!instantComplete)
       {
           return hyperspaceFinished;
       }
   }
   else if (spInfo->shipHyperspaceState != SHIPHYPERSPACE_DECELERATE)
   {
       if (spInfo->shipHyperspaceState == SHIPHYPERSPACE_DONE)
       {
           mrWhiteOutT -= 3.0f * HYPERSPACE_SLICE_RATE;
           if (mrWhiteOutT < 0.0f)
           {
               mrWhiteOut = FALSE;
           }

           if (spInfo->hsState == HS_INACTIVE)
           {
               if (!instantComplete && spAnyTrackedHyperspaceShipInState(SHIPHYPERSPACE_ACCELERATINGAWAY))
               {
                   return hyperspaceFinished;
               }

               if (!instantComplete && spAnyTrackedHyperspaceShipInState(SHIPHYPERSPACE_DONE))
               {
                   goto updateHyperspace;
               }

               if (singlePlayerGame)
               {
                   goto updateHyperspace;
               }
               if (ship->staticinfo->shipclass == CLASS_Component &&
                   ship->cataBusyStateWindow.dockingship != NULL)
               {
                   commandShip = ship->cataBusyStateWindow.dockingship;
               }
               else
               {
                   commandShip = ship;
               }

               command = getShipAndItsCommand(&universe.mainCommandLayer, commandShip);
               if (command == NULL ||
                   command->ordertype.order != COMMAND_MP_HYPERSPACEING ||
                   universe.totaltimeelapsed - command->hyperSpaceingTime >= TW_HYPERSPACE_TELEGRAPH_WAIT_TIME)
                {
                   goto updateHyperspace;
               }
               return hyperspaceFinished;
           }
           else if (spInfo->hsState == HS_SLICING_OUTOF)
           {
               if (instantComplete)
               {
                   goto updateHyperspace;
               }

               if (spAnyTrackedHyperspaceShipInState(SHIPHYPERSPACE_ACCELERATINGAWAY))
               {
                   return hyperspaceFinished;
               }
               goto updateHyperspace;
           }
           else if (spInfo->hsState == HS_COLLAPSE_OUTOF)
           {
               if (ship->soundevent.hyperspaceHandle != SOUND_NOTINITED)
               {
                   soundEventStopSound(ship->soundevent.hyperspaceHandle, 1);
                   ship->soundevent.hyperspaceHandle = SOUND_NOTINITED;
               }
               if ((!instantComplete) && (ship == universe.curPlayerPtr->PlayerMothership))
               {
                   if (spAnyOtherTrackedHyperspaceEffectActive(ship))
                   {
                       return hyperspaceFinished;
                   }
               }
               else
               {
                   if (!singlePlayerGame && shipOwnerHasActiveGravwell(ship))
                   {
                       return hyperspaceFinished;
                   }
               }
               goto updateHyperspace;
           }

updateHyperspace:
           hsUpdate(ship);

           if (spInfo->hsState == HS_INACTIVE &&
               universe.totaltimeelapsed > spInfo->timeToHyperspace)
           {
               hyperspaceFinished = TRUE;
               bitClear(ship->specialFlags,
                        SPECIAL_SinglePlayerWithinHyperSpaceRange | SPECIAL_Hyperspacing);
           }
       }

       return hyperspaceFinished;
   }

   if (universe.totaltimeelapsed <= spInfo->timeToHyperspace)
   {
       return hyperspaceFinished;
   }

   vecZeroVector(ship->posinfo.velocity);
   spInfo->shipHyperspaceState = SHIPHYPERSPACE_DONE;
   spInfo->timeToHyperspace = universe.totaltimeelapsed + HYPERSPACE_DECELERATE_TIME;
   ship->flags &= ~SOF_DockMarker;
   hsStart(ship, HYPERSPACE_SLICE_RATE, FALSE, hsShouldDisplayEffect(ship));
   return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4972D0
void spAutoSaveGame(void)
{
    spAutoSavePending = 0;

    // Build save game path
    if (spAutoSaveMissionNumber)
        sprintf(CurrentLevelName, "%s%02d - %s", SinglePlayerSavedGamesPath, spAutoSaveMissionNumber, spAutoSaveMissionName);
    else
        sprintf(CurrentLevelName, "%s%s", TutorialSavedGamesPath, spAutoSaveMissionName);

    SaveGame(CurrentLevelName);
}

int spSetSinglePlayerSaveInfo(int a1, const char *a2)
{
    spAutoSaveMissionNumber = a1;
    spAutoSavePending = 1;
    if (a2) strcpy(spAutoSaveMissionName, a2);
    return 0;
}

// Moves all docked ships out of carrier into space and assigns them to the
// carrier's military parade. Called on level start when hyperspaceState == 9.
void singlePlayerLaunchDockedShips(Ship *carrier)
{
    LinkedList *midlevelHyperspaceIn;
    Node *node;
    InsideShip *insideShip;
    Ship *ship;
    vector createat = { 0.0f, 0.0f, 0.0f };
    GrowSelection carrierSelection;
    GrowSelection shipSelection;
    SelectCommand carrierCommand;
    SelectCommand shipCommand;
    if (carrier->shipSinglePlayerGameInfo == NULL)
    {
        return;
    }

    midlevelHyperspaceIn = (LinkedList *)carrier->shipSinglePlayerGameInfo;

    carrierSelection.maxNumShips = 1;
    carrierSelection.selection = &carrierCommand;
    carrierCommand.numShips = 1;
    carrierCommand.ShipPtr[0] = carrier;

    shipSelection.maxNumShips = 1;
    shipSelection.selection = &shipCommand;
    shipCommand.numShips = 1;

    if (carrier->shiptype != 8 && carrier->shiptype != 42)
    {
        while ((node = midlevelHyperspaceIn->head) != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            ship = insideShip->ship;
            dockRemoveShipFromInside(ship, carrier);
            dockPutShipOutside(ship, carrier, &createat, FALSE, TRUE);
            if (ShipStaticInfoUpgradeByDockingEnabled(ship->staticinfo))
                shipApplyPlayerUpgrades(ship);
            acolyteStartLinking(ship);
            acvInitSpecial(ship);
            ship->health = ship->staticinfo->maxhealth;
            bitClear(ship->specialFlags, SPECIAL_SinglePlayerWithinHyperSpaceRange);
            shipCommand.ShipPtr[0] = ship;
            kasfShipsSetMilitaryParade(&carrierSelection, &shipSelection);
        }
    }

    if (singlePlayerGame && singlePlayerGameInfo.currentMission == 4)
    {
        node = midlevelHyperspaceIn->head;
        while (node != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            ship = insideShip->ship;
            if (ship->shiptype == dEcclesiam)
            {
                dockRemoveShipFromInside(ship, carrier);
                dockPutShipOutside(ship, carrier, &createat, FALSE, TRUE);
                if (ShipStaticInfoUpgradeByDockingEnabled(ship->staticinfo))
                    shipApplyPlayerUpgrades(ship);
                acolyteStartLinking(ship);
                acvInitSpecial(ship);
                ship->health = ship->staticinfo->maxhealth;
                bitClear(ship->specialFlags, SPECIAL_SinglePlayerWithinHyperSpaceRange);
                shipCommand.ShipPtr[0] = ship;
                kasfShipsSetMilitaryParade(&carrierSelection, &shipSelection);
                node = midlevelHyperspaceIn->head;
            }
            else
            {
                node = node->next;
            }
        }
    }
}

void singlePlayerForgetDamageEffectsRecursive(Ship *ship)
{
    ShipsInsideMe *shipsInsideMe;
    Node *node;
    InsideShip *insideShip;

    shipsInsideMe = ship->shipsInsideMe;
    if (shipsInsideMe != NULL)
    {
        for (node = shipsInsideMe->insideList.head; node != NULL; node = node->next)
        {
            insideShip = (InsideShip *)listGetStructOfNode(node);
            singlePlayerForgetDamageEffectsRecursive(insideShip->ship);
        }
    }

    dmgForgetEffects(ship);
}

Node *singlePlayerCullHyperspaceShips(void)
{
   Node *node;
   ShipType clampedChildShipTypes[21];

   for (node = universe.DeleteShipList.head; node != NULL; )
   {
       DeleteShip *deleteShip;
       Ship *ship;
       sdword shipType;
       sdword i;
       sdword clampedChildCount;
       bool32 keepShip;
       Node *nextNode;

       deleteShip = (DeleteShip *)listGetStructOfNode(node);
       nextNode = node->next;
       ship = deleteShip->ship;
       shipType = ship->shiptype;
       keepShip = FALSE;

       if (ship->staticinfo->shipclass != CLASS_Component ||
           shipType == 19 || shipType == 20 || shipType == 21 || shipType == 22 ||
           shipType == 23 || shipType == 26 || shipType == 31 || shipType == 24 ||
           shipType == 13)
       {
           keepShip = (singlePlayerGameInfo.currentMission != 4 || shipType != 24);
       }
       else
       {
           clampedChildCount = univGetSectMothershipClampedChildren(clampedChildShipTypes);
           for (i = 0; i < clampedChildCount; i++)
           {
               if (clampedChildShipTypes[i] == shipType)
               {
                   keepShip = TRUE;
                   break;
               }
           }
       }

       if (!keepShip)
       {
           ship->howDidIDie = DEATH_Left_Behind_At_Hyperspace;
           univDeleteWipeInsideShipOutOfExistence(ship);
       }

       node = nextNode;
   }

   return node;
}


void gameDifficultyGetModifiers(real32 *primaryModifier, sdword *buildTimeModifier, sdword *productionModifier, sdword difficulty)
{
    if (primaryModifier != NULL)
    {
        switch (difficulty)
        {
            case 0:
                *primaryModifier = 1.2f;
                break;
            case 1:
                *primaryModifier = 1.0f;
                break;
            case 2:
                *primaryModifier = 1.0f;
                break;
            case 3:
                *primaryModifier = 0.8f;
                break;
            case 4:
                *primaryModifier = 0.7f;
                break;
            default:
                break;
        }
    }

    if (buildTimeModifier != NULL)
    {
        switch (difficulty)
        {
            case 0:
                *buildTimeModifier = 4;
                break;
            case 1:
                *buildTimeModifier = 2;
                break;
            case 2:
            case 3:
            case 4:
                *buildTimeModifier = 1;
                break;
            default:
                break;
        }
    }

    if (productionModifier != NULL)
    {
        switch (difficulty)
        {
            case 0:
                *productionModifier = 4;
                break;
            case 1:
                *productionModifier = 2;
                break;
            case 2:
            case 3:
            case 4:
                *productionModifier = 1;
                break;
            default:
                break;
        }
    }
}

void gameDifficultyUpdateModifiers(void)
{
    gameDifficultyGetModifiers(&GameDifficultyPrimaryModifier, &GameDifficultyBuildTimeModifier, &GameDifficultyProductionModifier, g_GameDifficulty);
}

void gameDifficultySettingCB(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        feRadioButtonSet(name, g_GameDifficulty);
        gameDifficultyUpdateModifiers();
        bitSet(((regionhandle)atom->region)->status, RSF_DrawThisFrame);
    }
    else
    {
        g_GameDifficulty = (sdword)atom->pData;
        gameDifficultyUpdateModifiers();
    }
}

bool32 spCurrentPlayerHasHyperspaceShip(void)
{
    Node *node;
    Ship *ship;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        ship = (Ship *)node->structptr;
        if (ship->playerowner == universe.curPlayerPtr && bitTest(ship->specialFlags, SPECIAL_Hyperspacing))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void spPrepareShipsForHyperspaceRollCall(void)
{
    SelectCommand *selectAll;
    SelectCommand oneShipSel;
    sdword i;
    Ship *ship;

    if (singlePlayerGameInfo.hyperspaceState != HYPERSPACE_WAITINGROLLCALL)
    {
        return;
    }

    selectAll = selectAllCurrentPlayersNonHyperspacingShips();
    for (i = 0; i < selectAll->numShips; i++)
    {
        ship = selectAll->ShipPtr[i];
        oneShipSel.numShips = 1;
        oneShipSel.ShipPtr[0] = ship;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &oneShipSel);
        bitSet(ship->specialFlags, SPECIAL_SinglePlayerWithinHyperSpaceRange);
    }
    memFree(selectAll);

    selectAll = selectAllCurrentPlayersHyperspacingShips();
    for (i = 0; i < selectAll->numShips; i++)
    {
        ship = selectAll->ShipPtr[i];
        if (ship->dockInfo != NULL)
        {
            oneShipSel.numShips = 1;
            oneShipSel.ShipPtr[0] = ship;
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &oneShipSel);
            aitrackForceSteadyShip(ship);
            bitSet(ship->specialFlags, SPECIAL_SinglePlayerWithinHyperSpaceRange | SPECIAL_SinglePlayerInParade);
        }
    }
    memFree(selectAll);

    feScreenDeleteFlags(spHyperspaceRollCallScreen, 0);
    spHyperspaceRollCallScreen = NULL;
}


void singlePlayerStartCampaign(void)
{
    hvector startPoint = { 0.0f, 0.0f, 0.0f, 0.0f };
    char *nis, *script;
    bool32 loadLevelBeforeStart;

    universe.players[1].resourceUnits = 0;
    tmTechInit();
    spFleetModifier = 0.0f;
    universeSwitchToPlayer(0);

    if (warpLevel)
    {
        WarpInFleet(&startPoint);
        return;
    }

    singlePlayerNISNamesGet(&nis, &script, &loadLevelBeforeStart, 1);
    singlePlayerStartNis(nis, script, &startPoint, loadLevelBeforeStart, TRUE);
}

sdword spPostLoadAutosaveSave(char *path)
{
    sdword autosaveVarId = kasHashLabel("G_AutoSave");
    sdword timesGameLoadedVarId = kasHashLabel("G_TimesGameLoaded");
    sdword *autosaveVar;
    sdword *timesGameLoadedVar;

    autosaveVar = aivarFindEntry(0, (AIVar *)autosaveVarId);
    if (autosaveVar == NULL)
    {
        return 0;
    }

    timesGameLoadedVar = aivarFindEntry(0, (AIVar *)timesGameLoadedVarId);
    if (timesGameLoadedVar != NULL)
    {
        timesGameLoadedVar[2] += 1;
    }

    SaveGame(path);
    kasfEntryDelete(autosaveVar);
    return 1;
}


Ship *singlePlayerShipsCreate(GrowSelection *ships, char *shipTypeName, sdword count, sdword playerIndex)
{
    Ship *ship;
    ShipType shipType;
    ShipRace shipRace;
    vector shipPos;

    vecZeroVector(shipPos);
    ship = NULL;
    shipType = StrToShipType(shipTypeName);
    shipRace = GetValidRaceForShipType(shipType);
    kasfGrowSelectionClear(ships);
    while (shipType != (ShipType)-1 && count-- > 0)
    {
        ship = univCreateShip(shipType, shipRace, &shipPos, &universe.players[playerIndex], 0);
        if (ship != NULL)
        {
            growSelectAddShip(ships, ship);
        }
    }
    return ship;
}


void Init_Mission01_ActivateTutorialFlow(void);
void Watch_Mission01_TutorialFlow(void);
void Watch_Mission01_TutorialPromptCleanup(void);
void Watch_Mission01_TutorialResumeDelay(void);
void Watch_Mission01_TutorialEventFlow(void);
void FailMissionIfTrackedSelectionAlive(void);
sdword GetGameDifficulty(void);
void spEventWideScreenIn(sdword frames);

void MaintainTeamContext(void);
void Init_WaitForIntelEventEnd(void);
void Init_Mission11_MissionFlow_ClearIntelAdvanceFlag(void);
void Watch_Mission13_MissionFlow_WaitForIntelEventEnd(void);
void Init_Mission02_FleetIntel_SetupState(void);
void Watch_Mission02_FleetIntel_CleanupAndResume(void);
void Watch_Mission02_FleetIntel_ResumeDelay(void);
void Init_Mission03_FleetIntel_SetupState(void);
void Watch_Mission03_FleetIntel_CleanupAndResume(void);
void Watch_Mission03_FleetIntel_ResumeDelay(void);
void Watch_Mission03_FleetIntel_EventFlow(void);
void Watch_Mission03_FleetIntel_AdvanceAfterAcknowledge(void);
void Watch_Mission02_FleetIntel_EventFlow(void);
void Watch_Mission14_FleetIntel_FISensorTech(void);
void Init_Mission14_FleetIntel_FISensorTechCombatLoop(void);
void Watch_Mission14_FleetIntel_FISensorTechCombatLoop(void);
void Watch_Mission09_Traders_TalkingMore(void);
void Init_Mission09_Traders_TalkingMoreCombatLoop(void);
void Watch_Mission09_Traders_TalkingMoreCombatLoop(void);
void Init_Mission05_FleetIntel_FIDestroyResourcer(void);
void Watch_Mission05_FleetIntel_FIDestroyResourcer(void);
void Init_Mission05_FleetIntel_FIProtectBentusi(void);
void Watch_Mission05_FleetIntel_FIProtectBentusi(void);
void Init_Mission05_FleetIntel_FIProtectBentusiCleanup(void);
void Watch_Mission05_FleetIntel_FIProtectBentusiCleanup(void);
void Init_Mission02_FleetIntel_FIDefendSalvageTeam2(void);
void Watch_Mission02_FleetIntel_FIDefendSalvageTeam2(void);
void Init_Mission07_P2Mothership_Reappear(void);
void Init_Mission12_StrikeTeamDispatch_WaitForACT2(void);
void Init_Mission14_FleetIntel_FISensorTechLaunchPrep(void);
void Init_Mission09_Traders_TalkingMoreLaunchPrep(void);
void Watch_Mission04_TuranicFighters_DefendYourSelf(void);
void Init_BeastMultiBeamFrigateReinforcementTeam(void);
void Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastMultiBeamFrigateReinforcementTeam(void);
void Init_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Watch_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Begin_BeastMultiBeamFrigateReinforcementBuildTimer(void);
void Begin_BeastMultiBeamFrigateCombatLoop(void);
void Watch_BeastMultiBeamFrigateCombatLoop(void);
void Init_BeastCorvetteReinforcementTeam(void);
void Init_BeastCorvetteReinforcementTeam_Reevaluate(void);
void Watch_BeastCorvetteReinforcementTeam(void);
void Init_BeastCorvetteReinforcementBuildTimer(void);
void Watch_BeastCorvetteReinforcementBuildTimer(void);
sdword Begin_BeastCorvetteDockAndReinforce(void);
void Watch_BeastCorvetteReinforcementLaunchTimer(void);
void Init_BeastAcolyteReinforcementTeam(void);
void Init_BeastAcolyteReinforcementTeam_Reevaluate(void);
void Watch_BeastAcolyteReinforcementTeam(void);
void Init_BeastAcolyteReinforcementBuildTimer(void);
void Watch_BeastAcolyteReinforcementBuildTimer(void);
void Watch_BeastAcolyteReinforcementLaunchTimer(void);
void Init_BeastReconReinforcementTeam(void);
void Init_BeastReconReinforcementTeam_Reevaluate(void);
void Watch_BeastReconReinforcementTeam(void);
void Init_BeastReconReinforcementBuildTimer(void);
void Watch_BeastReconReinforcementBuildTimer(void);
void Begin_BeastReconReinforcementBuildTimer(void);
void Init_BeastRammingFrigateReinforcementTeam(void);
void Init_BeastRammingFrigateReinforcementTeam_Reevaluate(void);
void Watch_BeastRammingFrigateReinforcementTeam(void);
void Init_BeastRammingFrigateReinforcementBuildTimer(void);
void Watch_BeastRammingFrigateReinforcementBuildTimer(void);
void Begin_BeastRammingFrigateCombatLoop(void);
void Watch_BeastRammingFrigateCombatLoop(void);
void Watch_BeastRammingFrigateCombatLoopDelay(void);
sdword Begin_BeastReinforcementTeamDockAndReinforce(void);
void Watch_Mission17_BeastHeavyCruiserCombatLoop(void);
void Begin_Mission17_BeastCarrierCombatLoop(void);
void Watch_Mission17_BeastCarrierCombatLoop(void);
void Begin_Mission17_BeastCarrierAttackRun(void);
void Watch_Mission17_BeastCarrierAttackRun(void);
void Watch_Mission17_BeastCarrierReinforcementTeam(void);
void sub_65F6F0(void);
void ResetReinforcementBuildCount(void);
void Init_Mission05_BworkerReinforcementDelay(void);
void Init_BeastWorkerReinforcementTeam(void);
void Init_BeastWorkerReinforcementTeam_Reevaluate(void);
void Watch_BeastWorkerReinforcementTeam(void);
void Init_BeastWorkerReinforcementBuildTimer(void);
void Watch_BeastWorkerReinforcementBuildTimer(void);
void Watch_Mission05_CorvetteReinforcementTeam(void);
void Watch_Mission05_CorvetteReinforcementDock(void);
void Init_Mission05_CorvetteReinforcementDelay(void);
void Watch_Mission05_CorvetteReinforcementBuild(void);
void Init_Mission05_CorvetteAssault(void);
void Watch_Mission05_CorvetteAssault(void);
void Init_Mission05_CorvetteCapitalStrike(void);
void Watch_Mission05_CorvetteCapitalStrike(void);
void Init_Mission05_DockForReinforcements(void);
void Watch_Mission05_CorvetteDockAndReinforce(void);
void Init_Mission05_FighterTeam(void);
void Watch_Mission05_FighterTeam(void);
void Watch_Mission05_FighterReinforcementTeam(void);
void Watch_Mission05_FighterReinforcementDock(void);
void Init_Mission05_FighterReinforcementDelay(void);
void Watch_Mission05_FighterReinforcementBuild(void);
void Init_Mission05_FighterAssault(void);
void Watch_Mission05_FighterAssault(void);
void Init_Mission05_FighterCapitalStrike(void);
void Watch_Mission05_FighterCapitalStrike(void);
void Watch_Mission05_FighterDockAndReinforce(void);
void Init_Mission05_RammingFrigateTeam(void);
void Watch_Mission05_RammingFrigateTeam(void);
void Watch_Mission05_RammingFrigateReinforcementTeam(void);
void Watch_Mission05_RammingFrigateReinforcementDock(void);
void Init_Mission05_RammingFrigateReinforcementDelay(void);
void Watch_Mission05_RammingFrigateReinforcementBuild(void);
void Init_Mission05_RammingFrigateAssault(void);
void Watch_Mission05_RammingFrigateAssault(void);
void Init_Mission05_BeastWaveActivation(void);
void Init_Mission05_BeastWaveResumeDelay(void);
void Watch_Mission05_BeastWaveResumeDelay(void);
void Watch_Mission05_BeastWaveCoordinator(void);
void Init_Mission05_WorkerHarvestTeam(void);
void Watch_Mission05_WorkerHarvestTeam(void);
void Watch_Mission05_WorkerHarvestReinforcementTeam(void);
void Watch_Mission05_WorkerHarvestDockAndReinforce(void);
void Init_Mission05_WorkerHarvestReinforcementDelay(void);
void Watch_Mission05_WorkerHarvestReinforcementBuild(void);
void sub_617A10(void);
void sub_618210(void);
void sub_6183C0(void);
void sub_6184D0(void);
void Watch_Mission05_WorkerHarvestResourceLoop(void);
void Init_Mission05_FrigateTeam(void);
void Watch_Mission05_FrigateTeam(void);
void Watch_Mission05_FrigateReinforcementTeam(void);
void Watch_Mission05_FrigateDockAndReinforce(void);
void Init_Mission05_FrigateReinforcementDelay(void);
void Watch_Mission05_FrigateReinforcementBuild(void);
void Watch_Tutorial1_Tutorial10_MoveContextMenu(void);
void Init_Mission02_ACT3Corvettes1_Launch(void);
void Watch_Mission13_MissionFlow_FILost(void);
void Watch_Mission05_FleetIntel_FIObjectivesComplete(void);
void Init_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void);
void Watch_Tutorial1_Tutorial10_MoveContextMenuAreaClear(void);
void Begin_Mission13_MissionFlow_FILostAreaClear(void);
void Watch_Mission13_MissionFlow_FILostAreaClear(void);
void Init_Mission13_MissionFlow_FILostReinforcementDelay(void);
void Begin_Mission02_ACT3Corvettes1_Launch(void);
void Watch_Mission05_FrigateLaunchDockedReinforcements(void);
void Watch_Mission05_FrigateReinforcementCombatLoop(void);
void Begin_Mission05_FrigateCombatAttackRun(void);
void Watch_Mission05_FrigateCombatAttackRun(void);
void Begin_BeastIonArrayFrigateCombatLoop(void);
void singlePlayerShipsSetHyperspaceDestinationToCurrentPosition(GrowSelection *ships);
void singlePlayerShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation);
sdword kasfShipsSelectFirstN(GrowSelection *result, GrowSelection *source, sdword count);
void ResetReinforcementHyperspaceAttackFlag(void);
void Init_Mission06_HeavyDefenderTeam(void);
void Watch_Mission06_HeavyDefenderTeam(void);
void Init_Mission06_HeavyDefenderBuildDelay(void);
void Watch_Mission06_HeavyDefenderBuildDelay(void);
void Init_Mission06_AttackBomberLaunch(void);
void Watch_Mission06_AttackBomberLaunch(void);
void Begin_Mission06_RepairTeam_HomePosition(void);
void Watch_Mission06_RepairTeam_HomePosition(void);
void Begin_Mission06_RepairTeam_AssistTarget(void);
void Watch_Mission06_RepairTeam_AssistTarget(void);
void Begin_Mission06_ReinforcementWingGamma_Assault(void);
void Watch_Mission06_ReinforcementWingGamma_Assault(void);
void Begin_Mission06_ReinforcementWingGamma_CombatLoop(void);
void Watch_Mission06_ReinforcementWingGamma_CombatLoop(void);
void Init_Mission06_HeavyDefenderLaunch(void);
void Watch_Mission06_HeavyDefenderLaunch(void);
void Init_Mission06_HeavyDefenderAttackRun(void);
void Watch_Mission06_HeavyDefenderAttackRun(void);
void Init_Mission06_AttackBomberBuildDelay(void);
void Watch_Mission06_AttackBomberBuildDelay(void);
void Begin_Mission06_AttackBomberPatrol(void);
void Watch_Mission06_AttackBomberPatrol(void);
void Begin_Mission06_AttackBomberCombatLoop(void);
void Watch_Mission06_AttackBomberCombatLoop(void);
void Begin_Mission06_RepairSupportTargetWing_HomePosition(void);
void Watch_Mission06_RepairSupportTargetWing_HomePosition(void);
void Begin_Mission06_RepairSupportTargetWing_CombatLoop(void);
static sdword kasfShipsSelectCanSpecialTargetSelection(
    GrowSelection *result,
    GrowSelection *source,
    GrowSelection *targets);
static sdword singlePlayerShipsAnyIdle(GrowSelection *ships);
static void singlePlayerSortShipsByDistanceToPoint(GrowSelection *ships, hvector *point);
void Watch_Mission06_RepairSupportTargetWing_CombatLoop(void);
void Mission03_EnterCleeSanPrimaryResponseHoldingPattern(void);
void Mission03_ArmCleeSanPrimaryResponseHoldTimer(void);
void Watch_Mission03_CleeSanPrimaryResponseHoldTimer(void);
void Watch_Mission03_CleeSanPrimaryResponseHoldingPattern(void);
void Watch_Mission04_InfectedEcclesiaInParade(void);
void Init_Mission11_ObjectiveFlow_RepairFactorum(void);
void Watch_Mission11_ObjectiveFlow_DestroyAttackers(void);
void Watch_Mission11_ObjectiveFlow_RepairFactorum(void);
void Init_Mission11_ObjectiveFlow_UpdateBeastObjectives(void);
void Watch_Mission11_ObjectiveFlow_DestroyBeast(void);
void Begin_Mission17_NaggarokCruiseMissileAttackRun(void);
void Init_Mission09_AttackBomberWing(void);
void Watch_Mission09_AttackBomberWing(void);
void Watch_Mission09_AttackBomberWing_LaunchTrigger(void);
void Init_Mission09_AttackBomberWing_SpawnDelay(void);
void Watch_Mission09_AttackBomberWing_Spawn(void);
void Init_Mission09_AttackBomberWing_Regroup(void);
void Watch_Mission09_AttackBomberWing_Regroup(void);
void Init_Mission09_AttackBomberWing_ReengageDelay(void);
void Watch_Mission09_AttackBomberWing_ReengageDelay(void);
void Init_Mission09_AttackBomberWing_HuntNearbyTargets(void);
void Watch_Mission09_AttackBomberWing_HuntNearbyTargets(void);
void Init_Mission09_HeavyInterceptorWing(void);
void Watch_Mission09_HeavyInterceptorWing(void);
void Watch_Mission09_HeavyInterceptorWing_LaunchTrigger(void);
void Init_Mission09_HeavyInterceptorWing_SpawnDelay(void);
void Watch_Mission09_HeavyInterceptorWing_Spawn(void);
void Init_Mission09_HeavyInterceptorWing_Regroup(void);
void Watch_Mission09_HeavyInterceptorWing_Regroup(void);
void Init_Mission09_HeavyInterceptorWing_ReengageDelay(void);
void Watch_Mission09_HeavyInterceptorWing_ReengageDelay(void);
void Init_Mission09_HeavyInterceptorWing_HuntTargets(void);
void Watch_Mission09_HeavyInterceptorWing_HuntTargets(void);
void Init_Mission09_HeavyInterceptorWing_RequestSupportDelay(void);
void Init_Mission09_HeavyInterceptorWing_RequestSupport(void);
void Init_Mission09_CapitalAssaultWave(void);
void Watch_Mission09_CapitalAssaultWave(void);
void Watch_Mission09_CapitalAssaultWave_LaunchTrigger(void);
void Init_Mission09_CapitalAssaultWave_SpawnDelay(void);
void Watch_Mission09_CapitalAssaultWave_Spawn(void);
void Init_Mission09_SupportFrigateSquad_Regroup(void);
void Watch_Mission09_CapitalAssaultWave_Regroup(void);
void Watch_Mission09_BigGunEscortWing_SetWaypoint(void);
void Init_Mission09_BigGunEscortWing_ReacquireTargets(void);
void Watch_Mission09_BigGunEscortWing_ReacquireTargets(void);
void Init_Mission09_BigGunEscortWing_SpawnReinforcements(void);
void Watch_Mission09_BigGunEscortWing_SpawnReinforcements(void);
void Init_Mission09_AssaultEscort_ReengageDelay(void);
void Init_Mission09_CapitalAssaultWave_HuntTargets(void);
void Init_Mission09_CapitalAssaultWave_HuntNearbyTargets(void);
void Watch_Mission09_CapitalAssaultWave_HuntNearbyTargets(void);
void Init_Mission09_SupportFrigateSquad(void);
void Watch_Mission09_SupportFrigateSquad(void);
void Watch_Mission09_SupportFrigateSquad_LaunchTrigger(void);
void Watch_Mission09_SupportFrigateSquad_Spawn(void);
void Watch_Mission09_SupportFrigateSquad_Regroup(void);
void Init_Mission08_HarvestTeam_Harvest(void);
void Watch_Mission08_HarvestTeam_Harvest(void);
void Init_Mission16_Events_OverwhelmedFocusTarget(void);
void Watch_Mission16_Events_OverwhelmedCapitalRecovered(void);
void Watch_Mission16_Events_OverwhelmedTrackDamagedCapital(void);
void Watch_Mission09_FleetIntel_EventFlow(void);
void Watch_Mission09_FleetIntel_ResumeDelay(void);
void Watch_Mission09_SensorObjectiveTeam(void);
void Init_Mission09_GravwellSquad(void);
void Watch_Mission09_GravwellSquad(void);
void Watch_Mission09_GravwellSquad_LaunchTrigger(void);
void Init_Mission09_GravwellSquad_SpawnDelay(void);
void Watch_Mission09_GravwellSquad_Spawn(void);
void Watch_Mission09_GravwellSquad_Regroup(void);
void Watch_Mission09_GravwellSquad_Reengage(void);
void Init_Mission09_GravwellSquad_HuntDamagedCapital(void);
void Watch_Mission09_GravwellSquad_HuntDamagedCapital(void);
void *Init_LaunchDockedTeam(void);
void Watch_Mission13_MissionFlow_Farewell(void);


sdword Watch_Mission02_RequestPirateStrikeGroups(void);


void sub_65F6F0(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfEntrySetValue(332667, 1);
}

void ResetReinforcementBuildCount(void)
{
   kasCurrentStep = 2;
   kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
   kasfEntrySetValue(154639956, 0);
}




void Init_Mission10_MineLayingTeam3_Laying9Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(176088180, 330);
}

void Watch_Mission10_MineLayingTeam3_Laying9(void)
{
    AITeam *mineLayer;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasTimerExpiredById(176088180))
    {
        return;
    }

    mineLayer = kasAITeamPtrIfExists(5991012);
    kasfMsgSend(mineLayer, (char *)84320888);
    kasfSpeechEvent(
        5991013,
        (sdword)Init_Mission10_MineLayingTeam3_PostLaying9Delay,
        (sdword)Watch_Mission10_MineLayingTeam3_PostLaying9Delay);
}

void Init_Mission10_MineLayingTeam3_PostLaying9Delay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(176088181, 20);
}

void Watch_Mission10_MineLayingTeam3_PostLaying9Delay(void)
{
    AITeam *mineLayer;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasTimerExpiredById(176088181))
    {
        return;
    }

    mineLayer = kasAITeamPtrIfExists(5991013);
    kasfMsgSend(mineLayer, (char *)84320888);
    if (GetGameDifficulty() >= 2)
    {
        kasfSpeechEvent(
            5991014,
            (sdword)Init_Mission07_Events_HyperdriveFailDelay,
            (sdword)Watch_Mission07_Events_HyperdriveFail);
    }
    else
    {
        kasfEntrySetValue(-1947719517, 1);
        kasfSpeechEvent(342540, (sdword)sub_652EF0, (sdword)sub_652EF0);
    }
}



void Init_Mission07_Events_HyperdriveFailDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(176088182, 20);
}

void Watch_Mission07_Events_HyperdriveFail(void)
{
    AITeam *hyperdriveFailTeam;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasTimerExpiredById(176088182))
    {
        return;
    }

    hyperdriveFailTeam = kasAITeamPtrIfExists(5991014);
    kasfMsgSend(hyperdriveFailTeam, (char *)84320888);
    kasfEntrySetValue(-1947719517, 1);
    kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
}



sdword Watch_Tutorial1_Tutorial10_CameraZoomIn(void)
{
    GrowSelection *cameraTargets;
    sdword result;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    result = kasfTeamCount();
    if (result <= 0)
    {
        return result;
    }

    cameraTargets = (GrowSelection *)kasGrowSelectionPtr(144638915);
    kasfShipsAdd(cameraTargets, (GrowSelection *)(kasCurrentTeamPtr + 28));
    kasfSpeechEvent(
        74024315,
        (sdword)Watch_Mission13_MissionFlow_ComOpenSensors,
        (sdword)Watch_Mission13_MissionFlow_ComOpenSensorsWait);
    return result;
}



void Watch_Mission13_MissionFlow_ComOpenSensors(void)
{
    GrowSelection *comOpenSensorsShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    comOpenSensorsShips = (GrowSelection *)kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(comOpenSensorsShips);
    if (!kasfShipsCount(comOpenSensorsShips))
    {
        Mission02ShipsSelect(
            comOpenSensorsShips,
            (GrowSelection *)kasGrowSelectionPtr(0x4A2A29));
        kasfShipsRemove(
            comOpenSensorsShips,
            (GrowSelection *)kasGrowSelectionPtr(75774793));
    }

    tutDeallocateRootRegion();
    kasTimerCreateSetStartById(141340587, kasfRandom(5, 20));
}



sdword Watch_Mission13_MissionFlow_ComOpenSensorsWait(void)
{
    sdword result;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    if (kasfTeamCount())
    {
        result = kasTimerExpiredDestroyById(141340587);
        if (kasfShipsAnyIdle((GrowSelection *)(kasCurrentTeamPtr + 28)) || result != 0)
        {
            kasTimerDestroyById(141340587);
            kasfSpeechEvent(
                74024315,
                (sdword)Watch_Mission13_MissionFlow_ComOpenSensors,
                (sdword)Watch_Mission13_MissionFlow_ComOpenSensorsWait);
            return 1;
        }
        return result;
    }

    kasTimerDestroyById(141340587);
    kasfSpeechEvent(
        110655924,
        (sdword)sub_652EF0,
        (sdword)Watch_Tutorial1_Tutorial10_CameraZoomIn);
    return 1;
}


void Watch_FleetIntel_WaitForIntelEventEnd(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfIntelEventEnded())
    {
        kasfMsgSend(kasAITeamPtrIfExists(160291907), (char *)55426788);
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
}


void *Init_LaunchDockedTeam(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfLaunch();
    return NULL;
}


void Init_BeastWorkerReinforcementTeam(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(96759714)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-2070679070, 0);
    kasfEntryGetOrCreateSetValue(-2115886635, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)205770931);
        kasAITeamShipsPtr(&currentTeam->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
        kasfShipsApplyPlayerUpgrades(&currentTeam->shipList);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(313692, (sdword)sub_6183C0, (sdword)sub_6184D0);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastWorkerReinforcementTeam);
    }
}

void Init_BeastWorkerReinforcementTeam_Reevaluate(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(96759714))
     && kasfEntryValueGet(-2115886635)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2115886635, 0);
        kasfEntrySetValue(-2070679070, 1);
    }

    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)202253283);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastWorkerReinforcementTeam);
    }
}

void Watch_BeastWorkerReinforcementTeam(void)
{
    AITeam *currentTeam;
    GrowSelection *workerTeam;
    GrowSelection *workerShips;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    workerTeam = kasGrowSelectionPtr(44665972);
    workerShips = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(workerShips, workerTeam, "bWorker") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(127797474)))
        {
            if (kasfEntryValueGet(-2070679070))
            {
                kasfEntrySetValue(-2070679070, 0);
                kasfEntrySetValue(-2115886635, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_BeastWorkerReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-2070679070, 0);
            kasfEntrySetValue(-2115886635, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)205770931);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(workerShips, kasAITeamShipsPtrIfExists(44665972), "bWorker"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), workerShips);
        kasfShipsStop(workerShips);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), workerShips);
        kasfShipsSetSlaveableFlag(workerShips, 0);
        kasfShipsApplyPlayerUpgrades(workerShips);
        kasfReinforceTeamWithShips(currentTeam, workerShips);
        kasfTacticsNeutral();
    }

    kasfSpeechEvent(313692, (sdword)sub_6183C0, (sdword)sub_6184D0);
}

void Init_BeastWorkerReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle)
    {
        switch (buildStyle - 1)
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
                if ((kasfEntryValueGet(154639956) > 1
                  && kasfRUsGet(1) < kasfShipTypeBuildCost("bWorker") / 2)
                 || kasfEntryValueGet(154639956) == 4)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_BworkerReinforcementDelay,
                        (sdword)Watch_BeastWorkerReinforcementBuildTimer);
                }
                return;
            case 1:
                if (kasfEntryValueGet(154639956) < 4)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 4)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_BworkerReinforcementDelay,
                        (sdword)Watch_BeastWorkerReinforcementBuildTimer);
                }
                return;
            default:
                return;
        }
    }

    if (kasfEntryValueGet(154639956) < 4)
    {
        buildCost = kasfShipTypeBuildCost("bWorker");
        if (kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }

    buildCount = kasfEntryValueGet(154639956);
    if ((buildCount > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bWorker"))
     || buildCount == 4)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_BworkerReinforcementDelay,
            (sdword)Watch_BeastWorkerReinforcementBuildTimer);
    }
}

void Watch_BeastWorkerReinforcementBuildTimer(void)
{
    AITeam *currentTeam;
    GrowSelection *motherships;
    GrowSelection *launchSelection;
    GrowSelection *newWorkers;
    sdword randomIndex;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    motherships = kasGrowSelectionPtr(127797474);
    if (!kasfShipsCount(motherships))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        randomIndex = kasfRandom(0, kasfShipsCount(motherships) - 1);
        launchSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(launchSelection, motherships, randomIndex);

        newWorkers = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(
            newWorkers,
            "bWorker",
            kasfEntryValueGet(154639956),
            launchSelection,
            1);
        kasAITeamShipsPtr(newWorkers, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(currentTeam, newWorkers);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)205770931);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(313692, (sdword)sub_6183C0, (sdword)sub_6184D0);
    }
}

static void singlePlayerCollectUniverseShips(GrowSelection *ships)
{
    Node *node;

    growSelectInit(ships);
    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        growSelectAddShip(ships, (Ship *)node->structptr);
    }
}

static sdword kasfShipsSelectCanSpecialTargetSelection(
    GrowSelection *result,
    GrowSelection *source,
    GrowSelection *targets)
{
    GrowSelection copiedSource;
    GrowSelection *scanSource;
    sdword i;
    sdword j;
    sdword selected;

    if (!result || !source || !targets || !source->selection || !targets->selection)
    {
        return 0;
    }

    scanSource = source;
    if (result == source)
    {
        growSelectInit(&copiedSource);
        kasfShipsAdd(&copiedSource, source);
        scanSource = &copiedSource;
    }

    kasfGrowSelectionClear(result);
    selected = 0;
    for (i = 0; i < scanSource->selection->numShips; i++)
    {
        Ship *ship;

        ship = scanSource->selection->ShipPtr[i];
        for (j = 0; j < targets->selection->numShips; j++)
        {
            Ship *targetShip;

            targetShip = targets->selection->ShipPtr[j];
            if (shipCanSpecialTarget(targetShip) && shipCanBeSpecialTargeted(ship))
            {
                growSelectAddShip(result, ship);
                selected++;
                break;
            }
        }
    }

    if (scanSource == &copiedSource)
    {
        growSelectClose(&copiedSource);
    }
    return selected;
}

static sdword singlePlayerShipsAnyIdle(GrowSelection *ships)
{
    return kasfShipsAnyIdle(ships);
}

static void singlePlayerSortShipsByDistanceToPoint(GrowSelection *ships, hvector *point)
{
    vector targetPoint;
    sdword i;
    sdword j;

    if (!ships || !ships->selection || ships->selection->numShips <= 1 || !point)
    {
        return;
    }

    vecGrabVecFromHVec(targetPoint, *point);
    for (i = 0; i < ships->selection->numShips - 1; i++)
    {
        real32 bestDistanceSquared;
        sdword bestIndex;

        bestDistanceSquared = aiuFindDistanceSquared(
            ships->selection->ShipPtr[i]->posinfo.position,
            targetPoint);
        bestIndex = i;
        for (j = i + 1; j < ships->selection->numShips; j++)
        {
            real32 distanceSquared = aiuFindDistanceSquared(
                ships->selection->ShipPtr[j]->posinfo.position,
                targetPoint);
            if (distanceSquared < bestDistanceSquared)
            {
                bestDistanceSquared = distanceSquared;
                bestIndex = j;
            }
        }

        if (bestIndex != i)
        {
            ShipPtr ship = ships->selection->ShipPtr[i];
            ships->selection->ShipPtr[i] = ships->selection->ShipPtr[bestIndex];
            ships->selection->ShipPtr[bestIndex] = ship;
        }
    }
}

void sub_618210(void)
{
    GrowSelection allShips;
    GrowSelection *teamShips;
    GrowSelection *workerTargets;
    GrowSelection *damagedWorkers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    teamShips = kasAITeamShipsPtrIfExists(44665972);
    if (teamShips->selection->numShips > 0)
    {
        kasfSpeechEvent(313692, (sdword)sub_6183C0, (sdword)sub_6184D0);
        return;
    }

    workerTargets = kasGrowSelectionPtr(214830220);
    damagedWorkers = kasGrowSelectionPtr(85495135);
    singlePlayerCollectUniverseShips(&allShips);
    kasfShipsSelectType(workerTargets, &allShips, "bWorker");
    kasfShipsSelectCapital(workerTargets, workerTargets);
    kasfShipsSelectCanSpecialTargetSelection(damagedWorkers, workerTargets, teamShips);
    if (!singlePlayerShipsAnyIdle(damagedWorkers))
    {
        kasfShipsSelectDamaged(damagedWorkers, workerTargets, 30);
    }

    if (damagedWorkers->selection->numShips > 0)
    {
        singlePlayerSortShipsByDistanceToPoint(damagedWorkers, kasThisTeamsVectorPtr());
        kasfSpeechEvent(208120516, (sdword)sub_617A10, (sdword)sub_618210);
    }
    else
    {
        kasfSpeechEvent(
            208120516,
            (sdword)Init_Mission05_BworkerReinforcementDelay,
            (sdword)Watch_BeastWorkerReinforcementBuildTimer);
    }

    growSelectClose(&allShips);
}

void sub_6183C0(void)
{
    GrowSelection allShips;
    GrowSelection *teamShips;
    GrowSelection *attackTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    teamShips = kasAITeamShipsPtrIfExists(44665972);
    attackTargets = kasGrowSelectionPtr(214830220);
    singlePlayerCollectUniverseShips(&allShips);
    kasfShipsSelectCapital(attackTargets, &allShips);
    kasfShipsSelectCanSpecialTargetSelection(attackTargets, attackTargets, teamShips);
    if (attackTargets->selection->numShips > 1)
    {
        singlePlayerSortShipsByDistanceToPoint(attackTargets, kasThisTeamsVectorPtr());
    }
    kasfAttack(attackTargets);
    growSelectClose(&allShips);
}

void sub_6184D0(void)
{
    GrowSelection allShips;
    GrowSelection *teamShips;
    GrowSelection *workerTargets;
    GrowSelection *damagedWorkers;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    teamShips = kasAITeamShipsPtrIfExists(44665972);
    workerTargets = kasGrowSelectionPtr(214830220);
    damagedWorkers = kasGrowSelectionPtr(85495135);

    singlePlayerCollectUniverseShips(&allShips);
    kasfShipsSelectType(workerTargets, &allShips, "bWorker");
    kasfShipsSelectCapital(workerTargets, workerTargets);
    kasfShipsSelectCanSpecialTargetSelection(damagedWorkers, workerTargets, teamShips);
    if (!singlePlayerShipsAnyIdle(damagedWorkers))
    {
        kasfShipsSelectDamaged(damagedWorkers, workerTargets, 30);
    }

    if (damagedWorkers->selection->numShips <= 0)
    {
        kasfMsgSendAll((char *)4693508);
        return;
    }

    if (damagedWorkers->selection->numShips > 1)
    {
        singlePlayerSortShipsByDistanceToPoint(damagedWorkers, kasThisTeamsVectorPtr());
    }
    kasfShipsSetPercentDamaged(damagedWorkers, 55);
    kasfAttack(damagedWorkers);
    growSelectClose(&allShips);
}

void Watch_BeastIonArrayFrigateReinforcementTeam(void);
void Init_BeastIonArrayFrigateReinforcementBuildTimer(void);
void Watch_BeastIonArrayFrigateReinforcementBuildTimer(void);
void Watch_BeastIonArrayFrigateHyperspaceAttack(void);
void Watch_BeastIonArrayFrigateCombatLoop(void);
void Begin_BeastIonArrayFrigateAttackRun(void);
void Watch_BeastIonArrayFrigateAttackRun(void);
void Watch_BeastCloakedFighterReinforcementTeam(void);
void Init_BeastCloakedFighterReinforcementBuildTimer(void);
void Watch_BeastCloakedFighterReinforcementBuildTimer(void);
sdword Begin_BeastCloakedFighterDockForReinforcements(void);
void Watch_BeastCloakedFighterDockAndReinforce(void);
void Begin_BeastCloakedFighterCombatLoop(void);
void Watch_BeastCloakedFighterCombatLoop(void);
void Begin_BeastCloakedFighterAttackRun(void);
void Watch_BeastCloakedFighterAttackRun(void);
void Begin_BeastCloakedFighterReinforcementBuildTimer(void);
void Init_BeastCorvetteCapitalStrike(void);
void Watch_BeastCorvetteCapitalStrike(void);
void Begin_BeastCorvetteCombatLoop(void);
void Watch_BeastCorvetteCombatLoop(void);
void Init_BeastAcolyteCapitalStrike(void);
void Watch_BeastAcolyteCapitalStrike(void);
void Begin_BeastAcolyteCombatLoop(void);
void Watch_BeastAcolyteCombatLoop(void);
void Begin_BeastReconCapitalStrike(void);
void Watch_BeastReconCapitalStrike(void);
void Begin_BeastReconCombatLoop(void);
void Watch_BeastReconCombatLoop(void);

void Init_BeastIonArrayFrigateReinforcementTeam(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(151690773)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(20314791, 0);
    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-2131849467, 0);
    kasfEntryGetOrCreateSetValue(-1895203163, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)34005347);
        kasAITeamShipsPtr(&currentTeam->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
        kasfShipsApplyPlayerUpgrades(&currentTeam->shipList);
        kasfEntrySetValue(332667, 0);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(20314791, 1);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_BeastIonArrayFrigateCombatLoop);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastIonArrayFrigateReinforcementTeam);
    }
}

void Init_BeastIonArrayFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(151690773))
     && kasfEntryValueGet(-1895203163)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1895203163, 0);
        kasfEntrySetValue(-2131849467, 1);
    }

    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)221638067);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastIonArrayFrigateReinforcementTeam);
    }
}

void Watch_BeastIonArrayFrigateReinforcementTeam(void)
{
    AITeam *currentTeam;
    GrowSelection *reinforcementShips;
    GrowSelection *frigateShips;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    reinforcementShips = kasGrowSelectionPtr(44665972);
    frigateShips = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(frigateShips, reinforcementShips, "bIonArrayFrigate") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            if (kasfEntryValueGet(-2131849467))
            {
                kasfEntrySetValue(-2131849467, 0);
                kasfEntrySetValue(-1895203163, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_BeastIonArrayFrigateReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-2131849467, 0);
            kasfEntrySetValue(-1895203163, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)34005347);
    kasfEntrySetValue(332667, 0);
    kasfTeamSpecialToggle();
    kasfEntrySetValue(20314791, 1);
    if (kasfShipsSelectType(frigateShips, kasAITeamShipsPtrIfExists(44665972), "bIonArrayFrigate"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), frigateShips);
        kasfShipsStop(frigateShips);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), frigateShips);
        kasfShipsSetSlaveableFlag(frigateShips, 0);
        kasfShipsApplyPlayerUpgrades(frigateShips);
        kasfReinforceTeamWithShips(currentTeam, frigateShips);
        kasfTacticsNeutral();
    }

    kasfSpeechEvent(
        337061,
        (sdword)Begin_BeastIonArrayFrigateCombatLoop,
        (sdword)Watch_BeastIonArrayFrigateCombatLoop);
}

void Init_BeastIonArrayFrigateReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 2)
                {
                    buildCost = kasfShipTypeBuildCost("bIonArrayFrigate") / 2;
                    if ((sdword)kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) > 1
                  && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bIonArrayFrigate") / 2)
                 || kasfEntryValueGet(154639956) == 2)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_FrigateReinforcementDelay,
                        (sdword)Watch_BeastIonArrayFrigateReinforcementBuildTimer);
                }
                return;
            case 1:
                if (kasfEntryValueGet(154639956) < 2)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 2)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_FrigateReinforcementDelay,
                        (sdword)Watch_BeastIonArrayFrigateReinforcementBuildTimer);
                }
                return;
            default:
                return;
        }
    }

    if (kasfEntryValueGet(154639956) < 2)
    {
        buildCost = kasfShipTypeBuildCost("bIonArrayFrigate");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }

    buildCount = kasfEntryValueGet(154639956);
    if ((buildCount > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bIonArrayFrigate"))
     || buildCount == 2)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_FrigateReinforcementDelay,
            (sdword)Watch_BeastIonArrayFrigateReinforcementBuildTimer);
    }
}

void Watch_BeastIonArrayFrigateReinforcementBuildTimer(void)
{
    AITeam *currentTeam;
    GrowSelection *motherships;
    GrowSelection *newFrigates;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    motherships = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(motherships))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        newFrigates = kasGrowSelectionPtr(154639956);
        kasfShipsCreate(newFrigates, "bIonArrayFrigate", kasfEntryValueGet(154639956), 1);
        kasAITeamShipsPtr(newFrigates, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(currentTeam, newFrigates);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
        kasfSpeechEvent(
            175938357,
            (sdword)ResetReinforcementHyperspaceAttackFlag,
            (sdword)Watch_BeastIonArrayFrigateHyperspaceAttack);
    }
}

void Init_BeastCloakedFighterReinforcementTeam(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(200156578)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(20314791, 0);
    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-2106563470, 0);
    kasfEntryGetOrCreateSetValue(-2072705979, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)197700291);
        kasAITeamShipsPtr(&currentTeam->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
        kasfShipsApplyPlayerUpgrades(&currentTeam->shipList);
        kasfEntrySetValue(332667, 0);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(20314791, 1);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_BeastCloakedFighterCombatLoop);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastCloakedFighterReinforcementTeam);
    }
}

void Init_BeastCloakedFighterReinforcementTeam_Reevaluate(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(200156578))
     && kasfEntryValueGet(-2072705979)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2072705979, 0);
        kasfEntrySetValue(-2106563470, 1);
    }

    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466))
             && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_BeastCloakedFighterDockForReinforcements,
                    (sdword)Watch_BeastCloakedFighterDockAndReinforce);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)104569475);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastCloakedFighterReinforcementTeam);
        }
    }
}

void Watch_BeastCloakedFighterReinforcementTeam(void)
{
    AITeam *currentTeam;
    GrowSelection *reinforcementShips;
    GrowSelection *fighterShips;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    reinforcementShips = kasGrowSelectionPtr(44665972);
    fighterShips = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(fighterShips, reinforcementShips, "bCloakedFighter") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            if (kasfEntryValueGet(-2106563470))
            {
                kasfEntrySetValue(-2106563470, 0);
                kasfEntrySetValue(-2072705979, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_BeastCloakedFighterReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-2106563470, 0);
            kasfEntrySetValue(-2072705979, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)197700291);
    kasfEntrySetValue(332667, 0);
    kasfTeamSpecialToggle();
    kasfEntrySetValue(20314791, 1);
    if (kasfShipsSelectType(fighterShips, kasAITeamShipsPtrIfExists(44665972), "bCloakedFighter"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), fighterShips);
        kasfShipsStop(fighterShips);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), fighterShips);
        kasfShipsSetSlaveableFlag(fighterShips, 0);
        kasfShipsApplyPlayerUpgrades(fighterShips);
        kasfReinforceTeamWithShips(currentTeam, fighterShips);
        kasfTacticsNeutral();
    }

    kasfSpeechEvent(
        337061,
        (sdword)Begin_BeastCloakedFighterCombatLoop,
        (sdword)Watch_BeastCloakedFighterCombatLoop);
}

void Init_BeastCloakedFighterReinforcementBuildTimer(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 6)
                {
                    buildCost = kasfShipTypeBuildCost("bCloakedFighter") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) > 1
                  && kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter") / 2)
                 || kasfEntryValueGet(154639956) == 6)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Begin_BeastCloakedFighterReinforcementBuildTimer,
                        (sdword)Watch_BeastCloakedFighterReinforcementBuildTimer);
                }
                return;
            case 1:
                if (kasfEntryValueGet(154639956) < 6)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 6)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Begin_BeastCloakedFighterReinforcementBuildTimer,
                        (sdword)Watch_BeastCloakedFighterReinforcementBuildTimer);
                }
                return;
            default:
                return;
        }
    }

    if (kasfEntryValueGet(154639956) < 6)
    {
        buildCost = kasfShipTypeBuildCost("bCloakedFighter");
        if (kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }

    buildCount = kasfEntryValueGet(154639956);
    if ((buildCount > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
     || buildCount == 6)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_BeastCloakedFighterReinforcementBuildTimer,
            (sdword)Watch_BeastCloakedFighterReinforcementBuildTimer);
    }
}

void Watch_BeastCloakedFighterReinforcementBuildTimer(void)
{
    AITeam *currentTeam;
    GrowSelection *motherships;
    GrowSelection *newFighters;
    sdword randomIndex;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    motherships = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(motherships))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        randomIndex = kasfRandom(0, kasfShipsCount(motherships) - 1);
        kasfShipsSelectIndex(kasGrowSelectionPtr(21541), motherships, randomIndex);
        newFighters = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(
            newFighters,
            "bCloakedFighter",
            kasfEntryValueGet(154639956),
            kasGrowSelectionPtr(21541),
            1);
        kasAITeamShipsPtr(newFighters, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(currentTeam, newFighters);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(currentTeam->kasFSMNameHash), (char *)197700291);
        kasfTeamSpecialToggle();
        kasfEntrySetValue(20314791, 1);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_BeastCloakedFighterCombatLoop);
    }
}

void Init_BeastMultiBeamFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(62813829)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-1912783595, 0);
    kasfEntryGetOrCreateSetValue(-1928032491, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)24604083);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_BeastMultiBeamFrigateCombatLoop);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastMultiBeamFrigateReinforcementTeam);
    }
}

void Init_BeastMultiBeamFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(62813829))
     && kasfEntryValueGet(-1928032491)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1928032491, 0);
        kasfEntrySetValue(-1912783595, 1);
    }

    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)35692611);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastMultiBeamFrigateReinforcementTeam);
    }
}

void Watch_BeastMultiBeamFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *newFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    newFrigates = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bMultiBeamFrigate") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            if (kasfEntryValueGet(-1912783595))
            {
                kasfEntrySetValue(-1912783595, 0);
                kasfEntrySetValue(-1928032491, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, (sdword)Init_BeastMultiBeamFrigateReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-1912783595, 0);
            kasfEntrySetValue(-1928032491, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)24604083);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(newFrigates, kasAITeamShipsPtrIfExists(44665972), "bMultiBeamFrigate"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), newFrigates);
        kasfShipsStop(newFrigates);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), newFrigates);
        kasfShipsSetSlaveableFlag(newFrigates, 0);
        kasfShipsApplyPlayerUpgrades(newFrigates);
        kasfReinforceTeamWithShips(team, newFrigates);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_BeastMultiBeamFrigateCombatLoop);
}

void Init_BeastMultiBeamFrigateReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale)
    {
        switch (buildScale - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 4)
                {
                    buildCost = kasfShipTypeBuildCost("bMultiBeamFrigate") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) <= 1)
                 || (kasfRUsGet(1) >= kasfShipTypeBuildCost("bMultiBeamFrigate") / 2))
                {
                    if (kasfEntryValueGet(154639956) == 4)
                    {
                        kasfSpeechEvent(75929221, (sdword)Begin_BeastMultiBeamFrigateReinforcementBuildTimer, (sdword)Watch_BeastMultiBeamFrigateReinforcementBuildTimer);
                    }
                }
                else
                {
                    kasfSpeechEvent(75929221, (sdword)Begin_BeastMultiBeamFrigateReinforcementBuildTimer, (sdword)Watch_BeastMultiBeamFrigateReinforcementBuildTimer);
                }
                return;

            case 1:
                if (kasfEntryValueGet(154639956) < 4)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 4)
                {
                    kasfSpeechEvent(75929221, (sdword)Begin_BeastMultiBeamFrigateReinforcementBuildTimer, (sdword)Watch_BeastMultiBeamFrigateReinforcementBuildTimer);
                }
                return;

            default:
                return;
        }
    }

    if (kasfEntryValueGet(154639956) < 4)
    {
        buildCost = kasfShipTypeBuildCost("bMultiBeamFrigate");
        if (kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    buildCount = kasfEntryValueGet(154639956);
    if ((buildCount > 1 && kasfRUsGet(1) < kasfShipTypeBuildCost("bMultiBeamFrigate"))
     || buildCount == 4)
    {
        kasfSpeechEvent(75929221, (sdword)Begin_BeastMultiBeamFrigateReinforcementBuildTimer, (sdword)Watch_BeastMultiBeamFrigateReinforcementBuildTimer);
    }
}

void Watch_BeastMultiBeamFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTeam;
    GrowSelection *spawnMothership;
    GrowSelection *newFrigates;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTeam = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(launchTeam))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(launchTeam) - 1);
        spawnMothership = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnMothership, launchTeam, spawnIndex);
        newFrigates = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(newFrigates, "bMultiBeamFrigate", kasfEntryValueGet(154639956), spawnMothership, 1);
        kasAITeamShipsPtr(newFrigates, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, newFrigates);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)24604083);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_BeastMultiBeamFrigateCombatLoop);
    }
}

void Init_BeastCorvetteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(75148549)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-2082149163, 0);
    kasfEntryGetOrCreateSetValue(-1931839291, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)41951459);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastCorvetteReinforcementTeam);
    }
}

void Init_BeastCorvetteReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(75148549))
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
            if (kasfShipsCount(kasGrowSelectionPtr(14701123)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_BeastCorvetteDockAndReinforce,
                    (sdword)Watch_BeastCorvetteReinforcementLaunchTimer);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)259199139);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastCorvetteReinforcementTeam);
        }
    }
}

void Watch_BeastCorvetteReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *availableShips;
    GrowSelection *readyShips;
    GrowSelection *selectedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    availableShips = kasAITeamShipsPtrIfExists(44665972);
    readyShips = kasGrowSelectionPtr(128635107);
    kasfShipsSelectType(readyShips, availableShips, "bMissileCorvette");
    selectedShips = kasGrowSelectionPtr(363040);
    kasfShipsSelectType(selectedShips, availableShips, "bMultiGunCorvette");
    kasfShipsAdd(readyShips, selectedShips);
    kasfShipsSelectType(selectedShips, availableShips, "bHeavyCorvette");
    kasfShipsAdd(readyShips, selectedShips);
    if (kasfShipsCount(readyShips) < 3)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(14701123)))
        {
            if (kasfEntryValueGet(-2082149163))
            {
                kasfEntrySetValue(-2082149163, 0);
                kasfEntrySetValue(-1931839291, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, (sdword)Init_BeastCorvetteReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-2082149163, 0);
            kasfEntrySetValue(-1931839291, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)41951459);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(selectedShips, availableShips, "bMissileCorvette"))
    {
        kasfShipsRemove(availableShips, selectedShips);
        kasfShipsStop(selectedShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
        kasfShipsSetSlaveableFlag(selectedShips, 0);
        kasfShipsApplyPlayerUpgrades(selectedShips);
        kasfReinforceTeamWithShips(team, selectedShips);
        kasfTacticsNeutral();
    }
    if (kasfShipsSelectType(selectedShips, availableShips, "bMultiGunCorvette"))
    {
        kasfShipsRemove(availableShips, selectedShips);
        kasfShipsStop(selectedShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
        kasfShipsSetSlaveableFlag(selectedShips, 0);
        kasfShipsApplyPlayerUpgrades(selectedShips);
        kasfReinforceTeamWithShips(team, selectedShips);
        kasfTacticsNeutral();
    }
    if (kasfShipsSelectType(selectedShips, availableShips, "bHeavyCorvette"))
    {
        kasfShipsRemove(availableShips, selectedShips);
        kasfShipsStop(selectedShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
        kasfShipsSetSlaveableFlag(selectedShips, 0);
        kasfShipsApplyPlayerUpgrades(selectedShips);
        kasfReinforceTeamWithShips(team, selectedShips);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
}

void Init_BeastCorvetteReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale)
    {
        switch (buildScale - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 5)
                {
                    buildCost = kasfShipTypeBuildCost("bMissileCorvette") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) <= 3)
                 || (kasfRUsGet(1) >= kasfShipTypeBuildCost("bMissileCorvette") / 2))
                {
                    if (kasfEntryValueGet(154639956) == 5)
                    {
                        kasfSpeechEvent(
                            75929221,
                            (sdword)Init_Mission05_CorvetteReinforcementDelay,
                            (sdword)Watch_BeastCorvetteReinforcementBuildTimer);
                    }
                }
                else
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_CorvetteReinforcementDelay,
                        (sdword)Watch_BeastCorvetteReinforcementBuildTimer);
                }
                return;

            case 1:
                if (kasfEntryValueGet(154639956) < 5)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 5)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_CorvetteReinforcementDelay,
                        (sdword)Watch_BeastCorvetteReinforcementBuildTimer);
                }
                return;

            default:
                return;
        }
    }

    if (kasfEntryValueGet(154639956) < 5)
    {
        buildCost = kasfShipTypeBuildCost("bMissileCorvette");
        if (kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 3 && kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette"))
     || kasfEntryValueGet(154639956) == 5)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_CorvetteReinforcementDelay,
            (sdword)Watch_BeastCorvetteReinforcementBuildTimer);
    }
}

void Watch_BeastCorvetteReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *launchSelection;
    GrowSelection *createdShips;
    sdword spawnIndex;
    sdword buildCount;
    const char *shipType;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(14701123)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr(14701123)) - 1);
        launchTargets = kasGrowSelectionPtr(14701123);
        launchSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(launchSelection, launchTargets, spawnIndex);
        buildCount = kasfEntryValueGet(154639956) / 2;

        switch (kasfRandom(1, 3))
        {
            case 1:
                shipType = "bMultiGunCorvette";
                break;
            case 2:
                shipType = "bHeavyCorvette";
                break;
            default:
                shipType = "bMissileCorvette";
                break;
        }
        createdShips = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(createdShips, shipType, buildCount, launchSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);

        switch (kasfRandom(1, 3))
        {
            case 1:
                shipType = "bMultiGunCorvette";
                break;
            case 2:
                shipType = "bHeavyCorvette";
                break;
            default:
                shipType = "bMissileCorvette";
                break;
        }
        createdShips = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(createdShips, shipType, buildCount, launchSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
    }
}

sdword Begin_BeastCorvetteDockAndReinforce(void)
{
    AITeam *team;
    GrowSelection *dockSelection;
    GrowSelection *selectedTarget;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    dockSelection = kasGrowSelectionPtr(14701123);
    kasfShipsSortByDistanceToPoint(dockSelection, (vector *)kasThisTeamsVectorPtr());
    selectedTarget = kasGrowSelectionPtr(21541);
    kasfShipsSelectIndex(selectedTarget, dockSelection, 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    return kasfShipsDockWithSelection(&team->shipList, selectedTarget);
}

void Watch_BeastCorvetteReinforcementLaunchTimer(void)
{
    AITeam *team;
    GrowSelection *linkedMotherships;
    sdword buildLimit;
    sdword buildScale;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        return;
    }

    buildLimit = 5 - kasfTeamCount();
    buildScale = kasfEntryValueGet(-2126662828);
    switch (buildScale)
    {
        case 1:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                buildCost = kasfShipTypeBuildCost("bMissileCorvette") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 3 && kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette") / 2)
             || kasfEntryValueGet(154639956) == 5)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfShipTypeBuildTime("bMissileCorvette") * kasfEntryValueGet(154639956) / 2);
            }
            break;

        case 2:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                kasfEntryIncrement(154639956);
            }
            if (kasfEntryValueGet(154639956) == 5)
            {
                kasTimerCreateSetStartById(133682484, 0);
            }
            break;

        default:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                buildCost = kasfShipTypeBuildCost("bMissileCorvette");
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 3 && kasfRUsGet(1) < kasfShipTypeBuildCost("bMissileCorvette"))
             || kasfEntryValueGet(154639956) == 5)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bMissileCorvette"));
            }
            break;
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        linkedMotherships = kasGrowSelectionPtr(316548);
        kasfShipsSelectLinkedWith(&team->shipList, linkedMotherships, kasGrowSelectionPtr(0xE05243));
    }
}

void Init_BeastAcolyteReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(182180066)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-1985856270, 0);
    kasfEntryGetOrCreateSetValue(-1929432555, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)233775571);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 1);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastAcolyteReinforcementTeam);
    }
}

void Init_BeastAcolyteReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(182180066))
     && kasfEntryValueGet(-1929432555)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1929432555, 0);
        kasfEntrySetValue(-1985856270, 1);
    }

    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_BeastReinforcementTeamDockAndReinforce,
                    (sdword)Watch_BeastAcolyteReinforcementLaunchTimer);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)154145491);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastAcolyteReinforcementTeam);
        }
    }
}

void Watch_BeastAcolyteReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *availableShips;
    GrowSelection *readyShips;
    GrowSelection *selectedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    availableShips = kasAITeamShipsPtrIfExists(44665972);
    readyShips = kasGrowSelectionPtr(230526675);
    kasfShipsSelectType(readyShips, availableShips, "bInterceptor");
    selectedShips = kasGrowSelectionPtr(363040);
    kasfShipsSelectType(selectedShips, availableShips, "bAcolyte");
    kasfShipsAdd(readyShips, selectedShips);
    kasfShipsSelectType(selectedShips, availableShips, "bAttackBomber");
    kasfShipsAdd(readyShips, selectedShips);
    if (kasfShipsCount(readyShips) < 6)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            if (kasfEntryValueGet(-1985856270))
            {
                kasfEntrySetValue(-1985856270, 0);
                kasfEntrySetValue(-1929432555, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, (sdword)Init_BeastAcolyteReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-1985856270, 0);
            kasfEntrySetValue(-1929432555, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)233775571);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(selectedShips, availableShips, "bInterceptor"))
    {
        kasfShipsRemove(availableShips, selectedShips);
        kasfShipsStop(selectedShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
        kasfShipsSetSlaveableFlag(selectedShips, 0);
        kasfShipsApplyPlayerUpgrades(selectedShips);
        kasfReinforceTeamWithShips(team, selectedShips);
        kasfTacticsNeutral();
    }
    if (kasfShipsSelectType(selectedShips, availableShips, "bAcolyte"))
    {
        kasfShipsRemove(availableShips, selectedShips);
        kasfShipsStop(selectedShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
        kasfShipsSetSlaveableFlag(selectedShips, 0);
        kasfShipsApplyPlayerUpgrades(selectedShips);
        kasfReinforceTeamWithShips(team, selectedShips);
        kasfTacticsNeutral();
    }
    if (kasfShipsSelectType(selectedShips, availableShips, "bAttackBomber"))
    {
        kasfShipsRemove(availableShips, selectedShips);
        kasfShipsStop(selectedShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), selectedShips);
        kasfShipsSetSlaveableFlag(selectedShips, 0);
        kasfShipsApplyPlayerUpgrades(selectedShips);
        kasfReinforceTeamWithShips(team, selectedShips);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
}

void Init_BeastAcolyteReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale)
    {
        switch (buildScale - 1)
        {
            case 0:
                if (kasfEntryValueGet(154639956) < 8)
                {
                    buildCost = kasfShipTypeBuildCost("bAcolyte") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if ((kasfEntryValueGet(154639956) <= 6)
                 || (kasfRUsGet(1) >= kasfShipTypeBuildCost("bAcolyte") / 2))
                {
                    if (kasfEntryValueGet(154639956) == 8)
                    {
                        kasfSpeechEvent(
                            75929221,
                            (sdword)Init_Mission05_FighterReinforcementDelay,
                            (sdword)Watch_BeastAcolyteReinforcementBuildTimer);
                    }
                }
                else
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_FighterReinforcementDelay,
                        (sdword)Watch_BeastAcolyteReinforcementBuildTimer);
                }
                return;

            case 1:
                if (kasfEntryValueGet(154639956) < 8)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) == 8)
                {
                    kasfSpeechEvent(
                        75929221,
                        (sdword)Init_Mission05_FighterReinforcementDelay,
                        (sdword)Watch_BeastAcolyteReinforcementBuildTimer);
                }
                return;

            default:
                return;
        }
    }

    if (kasfEntryValueGet(154639956) < 8)
    {
        buildCost = kasfShipTypeBuildCost("bAcolyte");
        if (kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 6 && kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte"))
     || kasfEntryValueGet(154639956) == 8)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_FighterReinforcementDelay,
            (sdword)Watch_BeastAcolyteReinforcementBuildTimer);
    }
}

void Watch_BeastAcolyteReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTeam;
    GrowSelection *launchSelection;
    GrowSelection *createdShips;
    sdword spawnIndex;
    const char *shipType;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTeam = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(launchTeam))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)sub_652EF0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(launchTeam) - 1);
        launchSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(launchSelection, launchTeam, spawnIndex);
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
        createdShips = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(createdShips, shipType, kasfEntryValueGet(154639956), launchSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)233775571);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
    }
}

void Watch_BeastAcolyteReinforcementLaunchTimer(void)
{
    AITeam *team;
    GrowSelection *linkedMotherships;
    sdword buildLimit;
    sdword buildScale;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        return;
    }

    buildLimit = 8 - kasfTeamCount();
    buildScale = kasfEntryValueGet(-2126662828);
    switch (buildScale)
    {
        case 1:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                buildCost = kasfShipTypeBuildCost("bAcolyte") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 6 && kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte") / 2)
             || kasfEntryValueGet(154639956) == 8)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfShipTypeBuildTime("bAcolyte") * kasfEntryValueGet(154639956) / 2);
            }
            break;

        case 2:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                kasfEntryIncrement(154639956);
            }
            if (kasfEntryValueGet(154639956) == 8)
            {
                kasTimerCreateSetStartById(133682484, 0);
            }
            break;

        default:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                buildCost = kasfShipTypeBuildCost("bAcolyte");
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 6 && kasfRUsGet(1) < kasfShipTypeBuildCost("bAcolyte"))
             || kasfEntryValueGet(154639956) == 8)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bAcolyte"));
            }
            break;
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        linkedMotherships = kasGrowSelectionPtr(316548);
        kasfShipsSelectLinkedWith(&team->shipList, linkedMotherships, kasAITeamShipsPtrIfExists(51250466));
    }
}

void Init_BeastReconReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x56983E)))
    {
        return;
    }
    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-2075791394, 0);
    kasfEntryGetOrCreateSetValue(-1880696891, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)208220819);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 1);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastReconCombatLoop, (sdword)Watch_BeastReconCombatLoop);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastReconReinforcementTeam);
    }
}

void Init_BeastReconReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(0x56983E))
     && kasfEntryValueGet(-1880696891)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-1880696891, 0);
        kasfEntrySetValue(-2075791394, 1);
    }
    if (!kasfEntryValueGet(332667))
    {
        if (kasfTeamCount())
        {
            if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)) && kasfTeamHealthAverage() < 50)
            {
                kasfSpeechEvent(
                    299899,
                    (sdword)Begin_BeastReinforcementTeamDockAndReinforce,
                    (sdword)Watch_BeastReconReinforcementBuildTimer);
            }
        }
        else
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)158207907);
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastReconReinforcementTeam);
        }
    }
}

void Watch_BeastReconReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *reinforcementShips;
    GrowSelection *reconShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    reinforcementShips = kasGrowSelectionPtr(44665972);
    reconShips = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(reconShips, reinforcementShips, "bRecon") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            if (kasfEntryValueGet(-2075791394))
            {
                kasfEntrySetValue(-2075791394, 0);
                kasfEntrySetValue(-1880696891, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_BeastReconReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-2075791394, 0);
            kasfEntrySetValue(-1880696891, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        }
        return;
    }
    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)208220819);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(reconShips, kasAITeamShipsPtrIfExists(44665972), "bRecon"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), reconShips);
        kasfShipsStop(reconShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), reconShips);
        kasfShipsSetSlaveableFlag(reconShips, 0);
        kasfShipsApplyPlayerUpgrades(reconShips);
        kasfReinforceTeamWithShips(team, reconShips);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(74024315, (sdword)Begin_BeastReconCombatLoop, (sdword)Watch_BeastReconCombatLoop);
}

void Init_BeastReconReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < 7)
        {
            buildCost = kasfShipTypeBuildCost("bRecon") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if (kasfEntryValueGet(154639956) <= 1
         || (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRecon") / 2
         || kasfEntryValueGet(154639956) == 7)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastReconReinforcementBuildTimer,
                (sdword)Watch_BeastReconReinforcementBuildTimer);
        }
        return;
    }
    if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < 7)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 7)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Begin_BeastReconReinforcementBuildTimer,
                (sdword)Watch_BeastReconReinforcementBuildTimer);
        }
        return;
    }
    if (buildScale != 0)
    {
        return;
    }
    if (kasfEntryValueGet(154639956) < 7)
    {
        buildCost = kasfShipTypeBuildCost("bRecon");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRecon"))
     || kasfEntryValueGet(154639956) == 7)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Begin_BeastReconReinforcementBuildTimer,
            (sdword)Watch_BeastReconReinforcementBuildTimer);
    }
}

void Watch_BeastReconReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnPointSelection;
    GrowSelection *createdShips;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnPointSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnPointSelection, launchTargets, spawnIndex);
        createdShips = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(createdShips, "bRecon", kasfEntryValueGet(154639956), spawnPointSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }
    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)208220819);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastReconCombatLoop, (sdword)Watch_BeastReconCombatLoop);
    }
}

void Init_BeastRammingFrigateReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(223105365)))
    {
        return;
    }
    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-1997402011, 0);
    kasfEntryGetOrCreateSetValue(-2141093323, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)65242483);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastRammingFrigateCombatLoop,
            (sdword)Watch_BeastRammingFrigateCombatLoopDelay);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastRammingFrigateReinforcementTeam);
    }
}

void Init_BeastRammingFrigateReinforcementTeam_Reevaluate(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfThisTeamIs(kasAITeamPtrIfExists(223105365))
     && kasfEntryValueGet(-2141093323)
     && kasfMsgReceived((char *)4693508))
    {
        kasfEntrySetValue(-2141093323, 0);
        kasfEntrySetValue(-1997402011, 1);
    }
    if (!kasfEntryValueGet(332667) && !kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)203347251);
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)Watch_BeastRammingFrigateReinforcementTeam);
    }
}

void Watch_BeastRammingFrigateReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *reinforcementShips;
    GrowSelection *rammingFrigates;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    reinforcementShips = kasGrowSelectionPtr(44665972);
    rammingFrigates = kasGrowSelectionPtr(363040);
    if (kasfShipsSelectType(rammingFrigates, reinforcementShips, "bRammingFrigate") < 1)
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            if (kasfEntryValueGet(-1997402011))
            {
                kasfEntrySetValue(-1997402011, 0);
                kasfEntrySetValue(-2141093323, 1);
                kasfSpeechEvent(
                    165710423,
                    (sdword)ResetReinforcementBuildCount,
                    (sdword)Init_BeastRammingFrigateReinforcementBuildTimer);
            }
        }
        else
        {
            kasfEntrySetValue(-1997402011, 0);
            kasfEntrySetValue(-2141093323, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)Watch_BeastRammingFrigateReinforcementTeam);
        }
        return;
    }
    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)65242483);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(rammingFrigates, kasAITeamShipsPtrIfExists(44665972), "bRammingFrigate"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), rammingFrigates);
        kasfShipsStop(rammingFrigates);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), rammingFrigates);
        kasfShipsSetSlaveableFlag(rammingFrigates, 0);
        kasfShipsApplyPlayerUpgrades(rammingFrigates);
        kasfReinforceTeamWithShips(team, rammingFrigates);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(
        74024315,
        (sdword)Begin_BeastRammingFrigateCombatLoop,
        (sdword)Watch_BeastRammingFrigateCombatLoopDelay);
}

void Init_BeastRammingFrigateReinforcementBuildTimer(void)
{
    sdword buildScale;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < 2)
        {
            buildCost = kasfShipTypeBuildCost("bRammingFrigate") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if (kasfEntryValueGet(154639956) <= 1
         || (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRammingFrigate") / 2
         || kasfEntryValueGet(154639956) == 2)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
                (sdword)Watch_BeastRammingFrigateReinforcementBuildTimer);
        }
        return;
    }
    if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < 2)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 2)
        {
            kasfSpeechEvent(
                75929221,
                (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
                (sdword)Watch_BeastRammingFrigateReinforcementBuildTimer);
        }
        return;
    }
    if (buildScale != 0)
    {
        return;
    }
    if (kasfEntryValueGet(154639956) < 2)
    {
        buildCost = kasfShipTypeBuildCost("bRammingFrigate");
        if ((sdword)kasfRUsGet(1) >= buildCost)
        {
            kasfRUsSet(1, kasfRUsGet(1) - buildCost);
            kasfEntryIncrement(154639956);
        }
    }
    if ((kasfEntryValueGet(154639956) > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bRammingFrigate"))
     || kasfEntryValueGet(154639956) == 2)
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_Mission05_RammingFrigateReinforcementDelay,
            (sdword)Watch_BeastRammingFrigateReinforcementBuildTimer);
    }
}

void Watch_BeastRammingFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnPointSelection;
    GrowSelection *createdShips;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)Watch_BeastRammingFrigateReinforcementBuildTimer);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnPointSelection = kasGrowSelectionPtr(21541);
        kasfShipsSelectIndex(spawnPointSelection, launchTargets, spawnIndex);
        createdShips = kasGrowSelectionPtr(154639956);
        kasfShipsCreateInside(
            createdShips,
            "bRammingFrigate",
            kasfEntryValueGet(154639956),
            spawnPointSelection,
            1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }
    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)65242483);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastRammingFrigateCombatLoop,
            (sdword)Watch_BeastRammingFrigateCombatLoopDelay);
    }
}

void Begin_BeastMultiBeamFrigateCombatLoop(void)
{
    GrowSelection *combatTargets;
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    combatTargets = kasGrowSelectionPtr(57245931);
    if (!kasfShipsSelectNearby(combatTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfShipsAdd(combatTargets, kasGrowSelectionPtr(0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715))
     && kasfShipsSelectMembersOfSelection(
            kasGrowSelectionPtr(139967715),
            kasGrowSelectionPtr(139967715),
            combatTargets))
    {
        kasfShipsAdd(combatTargets, kasGrowSelectionPtr(139967715));
    }
    kasfShipsSortByDistanceToPoint(combatTargets, (vector *)kasThisTeamsVectorPtr());
    kasfFormationWall();
    kasfShipsSelectCapital(kasGrowSelectionPtr(124053532), combatTargets);
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_BeastMultiBeamFrigateCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastMultiBeamFrigateCombatLoop, (sdword)Watch_BeastMultiBeamFrigateCombatLoop);
    }
}

void Init_BeastCorvetteCapitalStrike(void)
{
    AITeam *team;
    GrowSelection *nearbyCapitals;
    GrowSelection *fsmShips;
    GrowSelection *sortedTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    fsmShips = &team->shipList;
    nearbyCapitals = kasGrowSelectionPtr(124053532);
    if (kasfShipsSelectNearby(nearbyCapitals, fsmShips, kasThisTeamsVectorPtr(), 20000)
     && kasfShipsSelectCapital(nearbyCapitals, nearbyCapitals))
    {
        if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), fsmShips, "CLASS_Corvette")
         && kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
        {
            kasfShipsRemove(nearbyCapitals, kasGrowSelectionPtr(157715175));
        }
        if (!kasfShipsCount(nearbyCapitals))
        {
            kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
            return;
        }
    }
    else
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
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
    kasfFormationSphere();
    kasfProtect(kasGrowSelectionPtr(154965556));
}

void Begin_BeastCorvetteCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackers = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        if (!kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sLeech")
         && !kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sMCV")
         && !kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sMimic"))
        {
            kasfGrowSelectionClear(attackers);
            kasfShipsAdd(attackers, kasGrowSelectionPtr(86599289));
        }
    }
    else
    {
        if (kasfShipsSelectNearby(kasGrowSelectionPtr(124053532), &team->shipList, kasThisTeamsVectorPtr(), 20000)
         && kasfShipsSelectCapital(kasGrowSelectionPtr(124053532), kasGrowSelectionPtr(124053532)))
        {
            if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), &team->shipList, "CLASS_Corvette")
             && kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
            {
                kasfShipsRemove(kasGrowSelectionPtr(124053532), kasGrowSelectionPtr(157715175));
            }
            if (kasfShipsCount(kasGrowSelectionPtr(124053532)))
            {
                kasfSpeechEvent(
                    0x4C9664,
                    (sdword)Init_BeastCorvetteCapitalStrike,
                    (sdword)Watch_BeastCorvetteCapitalStrike);
                return;
            }
        }
        kasfGrowSelectionClear(attackers);
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
    }
    singlePlayerSortShipsByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
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

void Watch_BeastCorvetteCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
    }
}

void Init_BeastAcolyteCapitalStrike(void)
{
    AITeam *team;
    GrowSelection *nearbyCapitals;
    GrowSelection *fsmShips;
    GrowSelection *fighters;
    GrowSelection *sortedTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    fsmShips = &team->shipList;
    nearbyCapitals = kasGrowSelectionPtr(124053532);
    if (kasfShipsSelectNearby(nearbyCapitals, fsmShips, kasThisTeamsVectorPtr(), 20000)
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
            kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
            return;
        }
    }
    else
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
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

void Begin_BeastAcolyteCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackers = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        if (!kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sLeech")
         && !kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sMCV")
         && !kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sMimic")
         && !kasfShipsSelectClass(attackers, kasGrowSelectionPtr(86599289), "CLASS_Fighter"))
        {
            kasfGrowSelectionClear(attackers);
            kasfShipsAdd(attackers, kasGrowSelectionPtr(86599289));
        }
    }
    else
    {
        if (kasfShipsSelectNearby(kasGrowSelectionPtr(124053532), &team->shipList, kasThisTeamsVectorPtr(), 20000)
         && kasfShipsSelectCapital(kasGrowSelectionPtr(124053532), kasGrowSelectionPtr(124053532)))
        {
            if (kasfShipsSelectClass(kasGrowSelectionPtr(182180066), &team->shipList, "CLASS_Fighter")
             && kasfShipsSelectTargets(kasGrowSelectionPtr(182180066), kasGrowSelectionPtr(157715175)))
            {
                kasfShipsRemove(kasGrowSelectionPtr(124053532), kasGrowSelectionPtr(157715175));
            }
            if (kasfShipsCount(kasGrowSelectionPtr(124053532)))
            {
                kasfSpeechEvent(
                    0x4C9664,
                    (sdword)Init_BeastAcolyteCapitalStrike,
                    (sdword)Watch_BeastAcolyteCapitalStrike);
                return;
            }
        }
        kasfGrowSelectionClear(attackers);
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
    }
    singlePlayerSortShipsByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
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

void Watch_BeastAcolyteCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
    }
}

void Begin_BeastReconCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackers = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(attackers, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 10000))
    {
        if (!kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sLeech")
         && !kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sMCV")
         && !kasfShipsSelectType(attackers, kasGrowSelectionPtr(86599289), "sMimic")
         && !kasfShipsSelectClass(attackers, kasGrowSelectionPtr(86599289), "CLASS_Fighter"))
        {
            kasfGrowSelectionClear(attackers);
            kasfShipsAdd(attackers, kasGrowSelectionPtr(86599289));
        }
    }
    else
    {
        if (kasfShipsCount(kasAITeamShipsPtrIfExists(51250466)))
        {
            kasfSpeechEvent(88512316, (sdword)Begin_BeastReconCapitalStrike, (sdword)Watch_BeastReconCapitalStrike);
            return;
        }
        kasfGrowSelectionClear(attackers);
        kasfShipsAdd(attackers, kasGrowSelectionPtr(0x4A2A29));
    }
    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(attackers, kasGrowSelectionPtr(139967715));
    }
    singlePlayerSortShipsByDistanceToPoint(attackers, kasThisTeamsVectorPtr());
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

void Watch_BeastReconCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList) || kasTimerExpiredDestroyById(74024315))
    {
        kasTimerDestroyById(74024315);
        kasfSpeechEvent(74024315, (sdword)Begin_BeastReconCombatLoop, (sdword)Watch_BeastReconCombatLoop);
    }
}


void Begin_BeastRammingFrigateCombatLoop(void)
{
    AITeam *team;
    GrowSelection *attackers;
    GrowSelection *scratchShips;
    GrowSelection *underAttackTargets;
    GrowSelection *capitalTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    attackers = kasGrowSelectionPtr(57245931);
    scratchShips = kasGrowSelectionPtr(0x4A2A29);
    kasfShipsAdd(attackers, scratchShips);
    kasfShipsSelectType(kasGrowSelectionPtr(0x58A20), attackers, "kCarrier");
    kasfShipsRemove(attackers, kasGrowSelectionPtr(0x58A20));
    if (!kasfShipsSelectNearby(kasGrowSelectionPtr(86599289), attackers, kasThisTeamsVectorPtr(), 20000))
    {
        kasfShipsAdd(kasGrowSelectionPtr(86599289), attackers);
    }
    underAttackTargets = kasGrowSelectionPtr(139967715);
    if (kasfUnderAttack(underAttackTargets))
    {
        kasfShipsAdd(kasGrowSelectionPtr(86599289), underAttackTargets);
    }
    singlePlayerSortShipsByDistanceToPoint(kasGrowSelectionPtr(86599289), kasThisTeamsVectorPtr());
    kasfFormationWall();
    capitalTargets = kasGrowSelectionPtr(124053532);
    if (!kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr(86599289)))
    {
        capitalTargets = kasGrowSelectionPtr(86599289);
    }
    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_BeastRammingFrigateCombatLoop(void)
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
            (sdword)Begin_BeastRammingFrigateCombatLoop,
            (sdword)Watch_BeastRammingFrigateCombatLoop);
    }
}


void Begin_SentinelCombatRetryDelay(void)
{
    AITeam *team;
    GrowSelection *moveTargets;
    GrowSelection *friendlyCapitals;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    moveTargets = kasGrowSelectionPtr(team->kasFSMNameHash);
    friendlyCapitals = kasGrowSelectionPtr(65634268);
    if (kasfShipsSelectCapital(friendlyCapitals, moveTargets))
    {
        if (kasfShipsSelectFriendly(kasGrowSelectionPtr(65633788), friendlyCapitals))
        {
            kasfShipsSortByDistanceToPoint(kasGrowSelectionPtr(65633788), (vector *)kasThisTeamsVectorPtr());
            moveTargets = kasGrowSelectionPtr(65633788);
        }
        else
        {
            kasfShipsSortByDistanceToPoint(friendlyCapitals, (vector *)kasThisTeamsVectorPtr());
            moveTargets = friendlyCapitals;
        }

        kasfShipsSelectIndex(kasGrowSelectionPtr(0x5425), moveTargets, 0);
        kasfMoveTo(kasShipsVectorPtrIfExists(21541));
    }

    kasfFormationWall();
    kasfTacticsAggressive();
    kasTimerCreateSetStartById(374244, kasfRandom(10, 20));
}

static bool32 singlePlayerShipHasReadySwarmers(Ship *ship)
{
    return (hiveFrigateSpec(ship)->currentTarget != NULL) && (hiveFrigateSpec(ship)->launchState != 0);
}

static bool32 singlePlayerShipsAnySwarmersReady(GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection;

    selection = ships->selection;
    if (selection == NULL || selection->numShips <= 0)
    {
        return FALSE;
    }

    for (i = 0; i < selection->numShips; i++)
    {
        if (!singlePlayerShipHasReadySwarmers(selection->ShipPtr[i]))
        {
            return TRUE;
        }
    }

    return FALSE;
}


void Begin_HiveFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bHiveFrigate"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bHiveFrigate") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}

void Begin_HiveFrigateCombatLoop(void)
{
    AITeam *team;
    GrowSelection *combatTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    combatTargets = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(combatTargets, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfEntryGetOrCreateSetValue(5813869, 1);
    }
    else
    {
        kasfEntryGetOrCreateSetValue(5813869, 0);
        kasfShipsAdd(combatTargets, kasGrowSelectionPtr(0x4A2A29));
    }

    if (kasfUnderAttack(kasGrowSelectionPtr(139967715)))
    {
        kasfShipsAdd(combatTargets, kasGrowSelectionPtr(139967715));
    }

    kasfShipsSortByDistanceToPoint(combatTargets, (vector *)kasThisTeamsVectorPtr());
    if (kasfShipsOrder(&team->shipList) != 7)
    {
        if (kasfEntryValueGet(5813869) && singlePlayerShipsAnySwarmersReady(&team->shipList))
        {
            kasfTeamSpecial(combatTargets);
        }
        else
        {
            tutDeallocateRootRegion();
        }
    }

    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(30, 60));
}


void Begin_BeastMultiBeamFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bMultiBeamFrigate"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bMultiBeamFrigate") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}



void Begin_BeastHeavyCruiserReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bHeavyCruiser"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bHeavyCruiser") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}


void Begin_BeastCarrierReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bCarrier"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bCarrier") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}


void Begin_BeastReconReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bRecon"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bRecon") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}


void Begin_BeastCruiseMissileReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bCruiseMissile"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(
            235522949,
            kasfShipTypeBuildTime("bCruiseMissile") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}


void Begin_BeastDFGFrigateReinforcementBuildTimer(void)
{
    AITeam *team;
    sdword buildScale;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildScale = kasfEntryValueGet(-2126662828);
    if (buildScale == 0)
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bDFGFrigate"));
    }
    else if (buildScale == 1)
    {
        kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bDFGFrigate") * kasfEntryValueGet(154639956) / 2);
    }
    else if (buildScale == 2)
    {
        kasTimerCreateSetStartById(235522949, 0);
    }
}


void ResetReinforcementHyperspaceAttackFlag(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(211499095, 0);
}


void Begin_Tutorial1_Tutorial12_RevealHiddenAttackTeam(void);
void Begin_Tutorial1_Tutorial12_MaintainAttackTeamContext(void);
void Begin_Tutorial1_Tutorial12_RequestOtherTeamHide(void);
void Watch_Tutorial1_Tutorial12_OtherAttackTeamHide(void);


sdword SetGameDifficulty(sdword difficulty)
{
    g_GameDifficulty = difficulty;
    return difficulty;
}

sdword GetGameDifficulty(void)
{
    return g_GameDifficulty;
}

extern sdword spHyperspaceObjectiveActive;

void singlePlayerRegisterShockwaveProjectile(SpaceObjRotImpTarg *projectile)
{
    shockwaveRegisterProjectile(projectile);
}

void singlePlayerUnregisterShockwaveProjectile(SpaceObjRotImpTarg *projectile)
{
    shockwaveUnregisterProjectile(projectile);
}

void spMainScreenCleanupAndLockout(sdword flags)
{
    cmCloseIfOpen();
    rmCloseIfOpen();
    lmCloseIfOpen();
    spCleanupScreenStack();
    spManagerScreenFlags = flags;
    if (flags & SPLOCKOUT_DESELECT)
    {
        selSelected.numShips = 0;
        ioUpdateShipTotals();
    }
    spScreenLockoutActive = 1;
    if (flags & SPLOCKOUT_MR)
    {
        mrDisable();
        mouseCursorHide();
    }
    if (flags & SPLOCKOUT_MOUSE)
    {
        mouseForceInvisible();
    }
}

bool32 spScreenLockoutActiveGet(void)
{
    return spScreenLockoutActive;
}

void spClearScreenLockout(void)
{
    sdword flags = spManagerScreenFlags;

    spScreenLockoutActive = 0;
    dword_A5B1B0 = 0;
    if (flags & SPLOCKOUT_DESELECT)
    {
        spManagerScreenFlags &= ~SPLOCKOUT_DESELECT;
        flags = spManagerScreenFlags;
    }
    if (flags & SPLOCKOUT_MR)
    {
        mrEnable();
        mouseCursorShow();
        spManagerScreenFlags &= ~SPLOCKOUT_MR;
        flags = spManagerScreenFlags;
    }
    if (flags & SPLOCKOUT_MOUSE)
    {
        mouseEnable();
    }
    spManagerScreenFlags = 0;
}

bool32 singlePlayerHyperspaceObjectiveActive(void)
{
    return singlePlayerGame && spHyperspaceObjectiveActive;
}

SpaceObjRotImpTarg *spaceObjEnableAttachNLips(SpaceObjRotImpTarg *spaceObj)
{
    bitSet(spaceObj->flags, SOF_AttachNLips);
    return spaceObj;
}

void singlePlayerCollectHyperspaceShipTypesForNIS(sdword *count, ShipType *shipTypes)
{
    Node *node;

    *count = 0;
    for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
    {
        InsideShip *insideShip = listGetStructOfNode(node);
        Ship *ship = insideShip->ship;
        SelectCommand *selection = ship->clampInfo != NULL ? (SelectCommand *)ship->clampInfo->selection : NULL;

        if (selection != NULL && selection->numShips > 0)
        {
            Ship *clampedShip = selection->ShipPtr[0];

            if (clampedShip != NULL &&
                clampedShip->objtype == OBJ_ShipType &&
                clampedShip->shiptype == sMothership)
            {
                shipTypes[(*count)++] = ship->shiptype;
            }
        }
    }
}

char *singlePlayerGetHyperspaceObjectiveLabel(void)
{
    if (singlePlayerGameInfo.onCompleteHyperspace)
    {
        return strGetString(strSinglePlayerHyperspace);
    }

    return strGetString(strSinglePlayerNextLevel);
}


void kasSpeechEventStopTeam(void)
{
    AITeam *team = (AITeam *)kasCurrentTeamPtr;

    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
}


void Mission03_SendCleeSanPrimaryResponseToApproachVector(void)
{
    hvector *result;
    hvector *sourceA;
    hvector *sourceB;
    real32 blendWeight;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(4591038);
    kasTimerDestroyById(31738387);
    result = kasVectorPtrIfExists((char *)44055618);
    sourceA = kasTeamsVectorPtr(134850334);
    sourceB = kasShipsVectorPtrIfExists(218060896);
    blendWeight = (real32)kasfRandom(30, 70) / 100.0f;
    result->x = sourceA->x * (1.0f - blendWeight) + sourceB->x * blendWeight;
    result->y = sourceA->y * (1.0f - blendWeight) + sourceB->y * blendWeight;
    result->z = sourceA->z * (1.0f - blendWeight) + sourceB->z * blendWeight;
    if (result->x > smUniverseSizeX) result->x = smUniverseSizeX;
    if (result->x < -smUniverseSizeX) result->x = -smUniverseSizeX;
    if (result->y > smUniverseSizeY) result->y = smUniverseSizeY;
    if (result->y < -smUniverseSizeY) result->y = -smUniverseSizeY;
    if (result->z > smUniverseSizeZ) result->z = smUniverseSizeZ;
    if (result->z < -smUniverseSizeZ) result->z = -smUniverseSizeZ;
    kasfMoveTo(result);
    kasfFormationWall();
}



void Mission03_EnterCleeSanPrimaryResponseHoldingPattern(void)
{
    sdword currentDistance;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfVectorRandomizeInRange(
        kasVectorPtrIfExists((char *)44055618),
        kasTeamsVectorPtr(134850334),
        50000);
    kasfMoveTo(kasVectorPtrIfExists((char *)44055618));
    kasfFormationWall();
    if (!kasfEntryValueGet(71552166))
    {
        kasfEntryGetOrCreateSetValue(71552166, 1);
        currentDistance = kasfFindDistance(
            kasTeamsVectorPtr(134850334),
            kasShipsVectorPtrIfExists(218060896));
        kasfEntryGetOrCreateSetValue(243820277, currentDistance);
    }
}


void Mission03_MarkCleeSanResponseActive(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
}

void Mission03_ArmCleeSanPrimaryResponseHoldTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(5769632, 40);
}


void Mission04_ArmEcclesiaAtGeneticsSpeechTimer(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(21079, 60);
}


extern real32 volSFX;

static bool32 spWideScreenSpeedScaleEnabled = -1;
static real32 spWideScreenSavedSFXVolume = 10.0f;

void singlePlayerSetWideScreenSpeedScale(bool32 enabled)
{
    spWideScreenSpeedScaleEnabled = enabled;
    if (!singlePlayerGame)
    {
        return;
    }

    if (enabled)
    {
        spWideScreenSavedSFXVolume = volSFX;
        if (singlePlayerGameInfo.currentMission == 5)
        {
            volSFX *= 0.3f;
        }
        else
        {
            volSFX *= 0.6f;
        }
    }
    else if (spWideScreenSavedSFXVolume < 9.0f)
    {
        volSFX = spWideScreenSavedSFXVolume;
        spWideScreenSavedSFXVolume = 10.0f;
    }
}


void sub_617A10(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfHarvest();
}


/*=== Begin_BeastCloakedFighterReinforcementBuildTimer (0x651d10) ===*/
void Begin_BeastCloakedFighterReinforcementBuildTimer(void)
{
    sdword buildStyle;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(235522949);
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                kasTimerCreateSetStartById(
                    235522949,
                    kasfShipTypeBuildTime("bCloakedFighter") * kasfEntryValueGet(154639956) / 2);
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
            kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bCloakedFighter"));
    }
}
void Begin_BeastACVCombatLoopDelay(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    kasfFormationDelta3D();
    kasfTacticsAggressive();
    kasTimerCreateSetStartById(374244, kasfRandom(10, 20));
}


void Init_BeastWorkerHarvestReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(208120516)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-1893839148, 0);
        kasfEntryGetOrCreateSetValue(-1898467739, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(208120516, (sdword)sub_617A10, 0);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        }
    }
}

void Watch_BeastWorkerHarvestReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedWorkers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedWorkers = kasGrowSelectionPtr(0x58A20);
    if (kasfShipsSelectType(returnedWorkers, kasGrowSelectionPtr(0x2A98C74), "bWorker") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
        {
            if (kasfEntryValueGet(-1893839148))
            {
                kasfEntrySetValue(-1893839148, 0);
                kasfEntrySetValue(-1898467739, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, 0);
            }
        }
        else
        {
            kasfEntrySetValue(-1893839148, 0);
            kasfEntrySetValue(-1898467739, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        }
        return;
    }

    kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
    kasfEntrySetValue(332667, 0);
    if (kasfShipsSelectType(returnedWorkers, kasAITeamShipsPtrIfExists(44665972), "bWorker"))
    {
        kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedWorkers);
        kasfShipsStop(returnedWorkers);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedWorkers);
        kasfShipsSetSlaveableFlag(returnedWorkers, 0);
        kasfShipsApplyPlayerUpgrades(returnedWorkers);
        kasfReinforceTeamWithShips(team, returnedWorkers);
        kasfTacticsNeutral();
    }
    kasfSpeechEvent(208120516, (sdword)sub_617A10, 0);
}

void Watch_BeastWorkerHarvestReinforcementBuildCount(void)
{
    AITeam *team;
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    buildCount = kasfEntryValueGet(154639956);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                if (buildCount < 2)
                {
                    buildCost = kasfShipTypeBuildCost("bWorker") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                        buildCount++;
                    }
                }
                if (buildCount > 1 && kasfRUsGet(1) >= kasfShipTypeBuildCost("bWorker") / 2)
                {
                    return;
                }
                break;
            case 1:
                if (buildCount < 2)
                {
                    kasfEntryIncrement(154639956);
                    buildCount++;
                }
                break;
            default:
                return;
        }
    }
    else
    {
        if (buildCount < 2)
        {
            buildCost = kasfShipTypeBuildCost("bWorker");
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
                buildCount++;
            }
        }
        if (buildCount > 1 && kasfRUsGet(1) >= kasfShipTypeBuildCost("bWorker"))
        {
            return;
        }
    }

    if (kasfEntryValueGet(154639956) == 2)
    {
        kasfSpeechEvent(75929221, (sdword)Init_Mission05_WorkerHarvestReinforcementDelay, 0);
    }
}

void Watch_BeastWorkerRepair_CloakDamagedTargets(void);

void Watch_BeastWorkerHarvestReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *launchPoints;
    GrowSelection *createdShips;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr(0xE05243)) - 1);
        launchPoints = kasGrowSelectionPtr(0x5425);
        kasfShipsSelectIndex(launchPoints, kasGrowSelectionPtr(0xE05243), spawnIndex);
        createdShips = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreateInside(createdShips, "bWorker", kasfEntryValueGet(154639956), launchPoints, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)207781940);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(208120516, (sdword)sub_617A10, 0);
    }
}

void Begin_BeastProcessorCombatLoop(void);
void Watch_BeastProcessorCombatLoop(void);
void Begin_BeastProcessorReinforcementBuildTimer(void);
void Reset_BeastProcessorCombatLoopTimer(void);

void Init_BeastProcessorReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (!kasfThisTeamIs(kasAITeamPtrIfExists(58381106)))
    {
        kasfEntryGetOrCreateSetValue(154639956, 0);
        kasfEntryGetOrCreateSetValue(111592388, 0);
        kasfEntryGetOrCreateSetValue(-1969307038, 0);
        kasfEntryGetOrCreateSetValue(-2106839323, 1);
        if (kasfTeamCount())
        {
            kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)84161299);
            kasAITeamShipsPtr(&team->shipList, 1);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
            kasfShipsApplyPlayerUpgrades(&team->shipList);
            kasfEntrySetValue(332667, 0);
            kasfSpeechEvent(0x4C9664, (sdword)Begin_BeastProcessorCombatLoop, (sdword)Watch_BeastProcessorCombatLoop);
        }
        else
        {
            kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        }
    }
}

void Watch_BeastProcessorReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedProcessors;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedProcessors = kasGrowSelectionPtr(0x58A20);
    if (kasfShipsSelectType(returnedProcessors, kasGrowSelectionPtr(0x2A98C74), "bProcessor") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
        {
            if (kasfEntryValueGet(-1969307038))
            {
                kasfEntrySetValue(-1969307038, 0);
                kasfEntrySetValue(-2106839323, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, 0);
            }
        }
        else
        {
            kasfEntrySetValue(-1969307038, 0);
            kasfEntrySetValue(-2106839323, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        }
    }
    else
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)84161299);
        kasfEntrySetValue(332667, 0);
        if (kasfShipsSelectType(returnedProcessors, kasAITeamShipsPtrIfExists(44665972), "bProcessor"))
        {
            kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedProcessors);
            kasfShipsStop(returnedProcessors);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedProcessors);
            kasfShipsSetSlaveableFlag(returnedProcessors, 0);
            kasfShipsApplyPlayerUpgrades(returnedProcessors);
            kasfReinforceTeamWithShips(team, returnedProcessors);
            kasfTacticsNeutral();
        }
        kasfSpeechEvent(0x4C9664, (sdword)Begin_BeastProcessorCombatLoop, (sdword)Watch_BeastProcessorCombatLoop);
    }
}

void Watch_BeastProcessorReinforcementBuildCount(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    buildCount = kasfEntryValueGet(154639956);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                if (buildCount < 1)
                {
                    buildCost = kasfShipTypeBuildCost("bProcessor") / 2;
                    if (kasfRUsGet(1) >= buildCost)
                    {
                        kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                        kasfEntryIncrement(154639956);
                    }
                }
                if (kasfEntryValueGet(154639956) != 1)
                {
                    return;
                }
                if (kasfRUsGet(1) >= kasfShipTypeBuildCost("bProcessor") / 2)
                {
                    return;
                }
                break;

            case 1:
                if (buildCount < 1)
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
        if (buildCount < 1)
        {
            buildCost = kasfShipTypeBuildCost("bProcessor");
            if (kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if (kasfEntryValueGet(154639956) > 1 &&
            kasfRUsGet(1) >= kasfShipTypeBuildCost("bProcessor"))
        {
            return;
        }
        if (kasfEntryValueGet(154639956) != 1)
        {
            return;
        }
    }

    kasfSpeechEvent(75929221, (sdword)Begin_BeastProcessorReinforcementBuildTimer, 0);
}

void Begin_BeastProcessorReinforcementBuildTimer(void)
{
    sdword buildStyle;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntryGetOrCreateSetValue(118933374, 0);
    kasTimerDestroyById(235522949);
    buildStyle = kasfEntryValueGet(-2126662828);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                kasTimerCreateSetStartById(235522949, kasfShipTypeBuildTime("bProcessor") * kasfEntryValueGet(154639956) / 2);
                break;

            case 1:
                kasTimerCreateSetStartById(235522949, 0);
                break;
        }
    }
    else
    {
        kasTimerCreateSetStartById(235522949, kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bProcessor"));
    }
}

void Watch_BeastProcessorReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *createdShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        createdShips = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreate(createdShips, "bProcessor", kasfEntryValueGet(154639956), 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfSpeechEvent(175938357, (sdword)ResetReinforcementHyperspaceAttackFlag, 0);
    }
}

void Begin_BeastProcessorAttackStaging(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *launchSelection;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfEntryValueGet(211499095))
    {
        targetIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr(0xE05243)) - 1);
        launchTargets = kasGrowSelectionPtr(0xE05243);
        launchSelection = kasGrowSelectionPtr(0x5425);
        if (kasfShipsSelectIndex(launchSelection, launchTargets, targetIndex))
        {
            kasfEntrySetValue(211499095, 1);
            kasfGrowSelectionClear(kasGrowSelectionPtr(0x716C77E));
            kasfShipsAdd(kasGrowSelectionPtr(0x716C77E), &team->shipList);
            kasfShipsAdd(kasGrowSelectionPtr(0x716C77E), launchSelection);
            kasfGrowSelectionClear(kasGrowSelectionPtr(0x716C77E));
            kasfShipsAdd(kasGrowSelectionPtr(0x716C77E), &team->shipList);
            kasfShipsAdd(kasGrowSelectionPtr(0x716C77E), launchSelection);
            singlePlayerShipsSetMilitaryParade(launchSelection, kasGrowSelectionPtr(0x716C77E));
            kasfTeamAttributesBitSet(128);
            kasfAttackMothership();
            singlePlayerShipsSetHyperspaceDestinationToCurrentPosition(&team->shipList);
            kasTimerCreateSetStartById(23446965, 10);
        }
    }
    if (kasfEntryValueGet(211499095) && kasTimerExpiredById(23446965))
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)84161299);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(0x4C9664, (sdword)Begin_BeastProcessorCombatLoop, (sdword)Watch_BeastProcessorCombatLoop);
    }
}

void Begin_BeastProcessorCombatLoop(void)
{
    GrowSelection *processorTargets;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    processorTargets = kasGrowSelectionPtr(0xC67AAC4);
    if (!kasfShipsCount(processorTargets))
    {
        kasfSpeechEvent(0x5B5E4, (sdword)Reset_BeastProcessorCombatLoopTimer, (sdword)Watch_BeastProcessorCombatLoop);
        return;
    }

    kasfShipsSelectType(kasGrowSelectionPtr(0x37EF332), processorTargets, "bProcessor");
    kasfShipsSelectTargets(kasGrowSelectionPtr(0x37EF332), kasGrowSelectionPtr(0x9668AE7));
    kasfGrowSelectionClear(kasGrowSelectionPtr(0x93C9634));
    kasfShipsAdd(kasGrowSelectionPtr(0x93C9634), processorTargets);
    kasfShipsRemove(kasGrowSelectionPtr(0x93C9634), kasGrowSelectionPtr(0x9668AE7));
    if (!kasfShipsCount(kasGrowSelectionPtr(0x93C9634)))
    {
        kasfSpeechEvent(0x5B5E4, (sdword)Reset_BeastProcessorCombatLoopTimer, (sdword)Watch_BeastProcessorCombatLoop);
        return;
    }

    kasfTacticsNeutral();
    kasfShipsSortByDistanceToPoint(processorTargets, (vector *)kasShipsVectorPtr((char *)0xC67AAC4));
    kasfShipsSelectFirstN(kasGrowSelectionPtr(0x93C9634), processorTargets, kasfShipsCount(processorTargets) / 2);
    kasfProtect(kasGrowSelectionPtr(0x93C9634));
}

void Watch_BeastProcessorCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(0x4C9664, (sdword)Begin_BeastProcessorCombatLoop, (sdword)Watch_BeastProcessorCombatLoop);
    }
}

void Watch_BeastWorkerRepair_CloakDamagedTargets(void);

void Init_BeastWorkerRepairReinforcementTeam(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(-2115886635, 1);
    if (kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)205770931);
        kasAITeamShipsPtr(&team->shipList, 1);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
        kasfShipsApplyPlayerUpgrades(&team->shipList);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(313692, (sdword)Watch_BeastWorkerRepair_CloakDamagedTargets, 0);
    }
    else
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
    }
}

void Watch_BeastWorkerRepairReinforcementTeam(void)
{
    AITeam *team;
    GrowSelection *returnedWorkers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    returnedWorkers = kasGrowSelectionPtr(0x58A20);
    if (kasfShipsSelectType(returnedWorkers, kasGrowSelectionPtr(0x2A98C74), "bWorker") < 1)
    {
        if (kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
        {
            if (kasfEntryValueGet(-2070679070))
            {
                kasfEntrySetValue(-2070679070, 0);
                kasfEntrySetValue(-2115886635, 1);
                kasfSpeechEvent(165710423, (sdword)ResetReinforcementBuildCount, 0);
            }
        }
        else
        {
            kasfEntrySetValue(-2070679070, 0);
            kasfEntrySetValue(-2115886635, 1);
            kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        }
    }
    else
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)205770931);
        kasfEntrySetValue(332667, 0);
        if (kasfShipsSelectType(returnedWorkers, kasAITeamShipsPtrIfExists(44665972), "bWorker"))
        {
            kasfShipsRemove(kasAITeamShipsPtrIfExists(44665972), returnedWorkers);
            kasfShipsStop(returnedWorkers);
            kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), returnedWorkers);
            kasfShipsSetSlaveableFlag(returnedWorkers, 0);
            kasfShipsApplyPlayerUpgrades(returnedWorkers);
            kasfReinforceTeamWithShips(team, returnedWorkers);
            kasfTacticsNeutral();
        }
        kasfSpeechEvent(313692, (sdword)Watch_BeastWorkerRepair_CloakDamagedTargets, 0);
    }
}

void Watch_BeastWorkerRepairReinforcementBuildCount(void)
{
    sdword buildStyle;
    sdword buildCount;
    sdword buildCost;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    buildStyle = kasfEntryValueGet(-2126662828);
    buildCount = kasfEntryValueGet(154639956);
    if (buildStyle)
    {
        switch (buildStyle - 1)
        {
            case 0:
                if (buildCount < 3)
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
                if (buildCount < 3)
                {
                    kasfEntryIncrement(154639956);
                }
                if (kasfEntryValueGet(154639956) != 3)
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
        if (buildCount < 3)
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
        if (kasfEntryValueGet(154639956) != 3)
        {
            return;
        }
    }

    kasfSpeechEvent(75929221, (sdword)Init_Mission05_BworkerReinforcementDelay, 0);
}

void Watch_BeastWorkerRepairReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *launchPoints;
    GrowSelection *createdShips;
    sdword spawnIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsCount(kasGrowSelectionPtr(0xE05243)))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
        return;
    }

    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        spawnIndex = kasfRandom(0, kasfShipsCount(kasGrowSelectionPtr(0xE05243)) - 1);
        launchPoints = kasGrowSelectionPtr(0x5425);
        kasfShipsSelectIndex(launchPoints, kasGrowSelectionPtr(0xE05243), spawnIndex);
        createdShips = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreateInside(createdShips, "bWorker", kasfEntryValueGet(154639956), launchPoints, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }

    if (kasfTeamFinishedLaunching())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)205770931);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(313692, (sdword)Watch_BeastWorkerRepair_CloakDamagedTargets, 0);
    }
}

void Watch_BeastWorkerRepair_CloakDamagedTargets(void);
void Watch_BeastWorkerRepair_SpecialAttackDamagedCapital(void);

void Watch_BeastWorkerRepairCombatLoop(void)
{
    AITeam *team;
    GrowSelection *damagedShips;
    GrowSelection *sourceShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    sourceShips = kasGrowSelectionPtr(team->kasFSMNameHash);
    damagedShips = kasGrowSelectionPtr(0x8615BD4);
    if ((kasfShipsSelectCapital(damagedShips, sourceShips) &&
         kasfShipsSelectDamaged(damagedShips, damagedShips, 90)) ||
        (kasfShipsSelectType(damagedShips, sourceShips, "bWorker") &&
         kasfShipsSelectDamaged(damagedShips, damagedShips, 90)))
    {
        kasfSpeechEvent(313692, (sdword)Watch_BeastWorkerRepair_CloakDamagedTargets, 0);
        return;
    }

    if (kasfShipsSelectDamaged(damagedShips, kasGrowSelectionPtr(0x4A2A29), 45) &&
        kasfShipsSelectCapital(damagedShips, damagedShips) &&
        kasfShipsSelectCanSpecialTargetSelection(damagedShips, damagedShips, &team->shipList))
    {
        kasfSpeechEvent(123839973, (sdword)Watch_BeastWorkerRepair_SpecialAttackDamagedCapital, 0);
        return;
    }

    if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(208120516, (sdword)sub_617A10, 0);
    }
}


void Watch_BeastWorkerHarvestLoop(void)
{
    AITeam *team;
    GrowSelection *returnedWorkers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfGetWorldResources())
    {
        if (kasfShipsAnyIdle(&team->shipList))
        {
            kasfSpeechEvent(208120516, (sdword)sub_617A10, 0);
        }
    }
    else
    {
        returnedWorkers = kasGrowSelectionPtr(0x58A20);
        kasfGrowSelectionClear(returnedWorkers);
        kasfShipsAdd(returnedWorkers, &team->shipList);
        kasfShipsRemove(&team->shipList, returnedWorkers);
        kasfReinforceTeamWithShips(kasAITeamPtrIfExists(44665972), returnedWorkers);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
    }
}

void Watch_BeastIonArrayFrigateHyperspaceAttack(void)
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
        motherships = kasAITeamShipsPtrIfExists(51250466);
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
        kasfTeamSpecialToggle();
        kasfEntrySetValue(20314791, 1);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_BeastIonArrayFrigateCombatLoop);
    }
}

void Watch_BeastIonArrayFrigateCombatLoop(void)
{
    AITeam *team;
    GrowSelection *nearbyTargets;
    GrowSelection *underAttackTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    underAttackTargets = kasGrowSelectionPtr(139967715);
    if (kasfShipsSelectNearby(nearbyTargets, &team->shipList, kasThisTeamsVectorPtr(), 6000)
     && ((kasfUnderAttack(underAttackTargets)
       && kasfShipsSelectMembersOfSelection(underAttackTargets, underAttackTargets, nearbyTargets))
      || kasfShipsSelectCapital(nearbyTargets, nearbyTargets)))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastIonArrayFrigateAttackRun,
            (sdword)Watch_BeastIonArrayFrigateAttackRun);
    }
    else if (kasTimerExpiredDestroyById(337061) || kasfShipsAnyIdle(&team->shipList))
    {
        kasTimerDestroyById(337061);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_BeastIonArrayFrigateCombatLoop);
    }
}

void Begin_BeastIonArrayFrigateAttackRun(void)
{
    AITeam *team;
    GrowSelection *nearbyTargets;
    GrowSelection *underAttackTargets;
    GrowSelection *capitalTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    nearbyTargets = kasGrowSelectionPtr(57245931);
    underAttackTargets = kasGrowSelectionPtr(139967715);
    if (!kasfShipsSelectNearby(nearbyTargets, &team->shipList, kasThisTeamsVectorPtr(), 6000))
    {
        if (!kasfEntryValueGet(20314791))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(20314791, 1);
        }
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_BeastIonArrayFrigateCombatLoop);
        return;
    }

    if (kasfUnderAttack(underAttackTargets))
    {
        kasfShipsAdd(nearbyTargets, underAttackTargets);
    }
    kasfFormationWall();
    if (kasfShipsCount(nearbyTargets))
    {
        if (kasfEntryValueGet(20314791))
        {
            kasfEntrySetValue(20314791, 0);
            kasfTeamSpecialToggle();
        }
    }
    else if (!kasfEntryValueGet(20314791))
    {
        kasfEntrySetValue(20314791, 1);
        kasfTeamSpecialToggle();
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

void Watch_BeastIonArrayFrigateAttackRun(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsAnyIdle(&team->shipList))
    {
        if (!kasfEntryValueGet(20314791))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(20314791, 1);
        }
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastIonArrayFrigateCombatLoop,
            (sdword)Watch_BeastIonArrayFrigateCombatLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastIonArrayFrigateAttackRun,
            (sdword)Watch_BeastIonArrayFrigateAttackRun);
    }
}

void Begin_BeastCloakedFighterCombatLoop(void)
{
    GrowSelection *workerTargets;
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    workerTargets = kasGrowSelectionPtr(124053532);
    kasfShipsSelectType(workerTargets, kasGrowSelectionPtr(0x4A2A29), "sWorker");
    singlePlayerSortShipsByDistanceToPoint(workerTargets, kasThisTeamsVectorPtr());
    kasfFormationDelta3D();
    if (kasfShipsSelectIndex(kasGrowSelectionPtr(21541), workerTargets, 0))
    {
        kasfMoveTo(kasShipsVectorPtrIfExists(21541));
    }
    else
    {
        kasfMoveTo(kasShipsVectorPtrIfExists(218060896));
    }
    kasTimerDestroyById(337061);
    kasTimerCreateSetStartById(337061, kasfRandom(10, 20));
}

void Watch_BeastCloakedFighterCombatLoop(void)
{
    AITeam *currentTeam;
    GrowSelection *nearbyShips;
    GrowSelection *attackers;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;

    nearbyShips = kasGrowSelectionPtr(57245931);
    if (kasfShipsSelectNearby(nearbyShips, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 6000)
     && ((attackers = kasGrowSelectionPtr(140098787),
          kasfUnderAttack(attackers)
          && kasfShipsSelectMembersOfSelection(attackers, attackers, nearbyShips))
      || kasfShipsSelectType(nearbyShips, nearbyShips, "sWorker")))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastCloakedFighterAttackRun,
            (sdword)Watch_BeastCloakedFighterAttackRun);
    }
    else if (kasTimerExpiredDestroyById(337061) || kasfShipsAnyIdle(&currentTeam->shipList))
    {
        kasTimerDestroyById(337061);
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_BeastCloakedFighterCombatLoop);
    }
}

void Begin_BeastCloakedFighterAttackRun(void)
{
    GrowSelection *nearbyShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;

    nearbyShips = kasGrowSelectionPtr(57245931);
    if (!kasfShipsSelectNearby(nearbyShips, kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 6000))
    {
        if (!kasfEntryValueGet(20314791))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(20314791, 1);
        }
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_BeastCloakedFighterCombatLoop);
        return;
    }

    if (kasfUnderAttack(kasGrowSelectionPtr(140098787)))
    {
        kasfShipsAdd(nearbyShips, kasGrowSelectionPtr(140098787));
    }

    singlePlayerSortShipsByDistanceToPoint(nearbyShips, kasThisTeamsVectorPtr());
    if (kasfShipsCount(nearbyShips))
    {
        if (kasfEntryValueGet(20314791))
        {
            kasfEntrySetValue(20314791, 0);
        }
        else
        {
            kasfEntrySetValue(20314791, 1);
        }
        kasfTeamSpecialToggle();
    }

    tutDeallocateRootRegion();
    kasTimerDestroyById(74024315);
    kasTimerCreateSetStartById(74024315, kasfRandom(10, 30));
}

void Watch_BeastCloakedFighterAttackRun(void)
{
    AITeam *currentTeam;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    if (kasfShipsAnyIdle(&currentTeam->shipList))
    {
        if (!kasfEntryValueGet(20314791))
        {
            kasfTeamSpecialToggle();
            kasfEntrySetValue(20314791, 1);
        }
        kasfSpeechEvent(
            337061,
            (sdword)Begin_BeastCloakedFighterCombatLoop,
            (sdword)Watch_BeastCloakedFighterCombatLoop);
    }
    else if (kasTimerExpiredById(74024315))
    {
        kasfSpeechEvent(
            74024315,
            (sdword)Begin_BeastCloakedFighterAttackRun,
            (sdword)Watch_BeastCloakedFighterAttackRun);
    }
}

sdword Begin_BeastCloakedFighterDockForReinforcements(void)
{
    GrowSelection *launchMothership;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);

    singlePlayerSortShipsByDistanceToPoint(kasAITeamShipsPtrIfExists(51250466), kasThisTeamsVectorPtr());
    launchMothership = kasGrowSelectionPtr(21541);
    kasfShipsSelectIndex(launchMothership, kasAITeamShipsPtrIfExists(51250466), 0);
    kasfFormationDelta3D();
    return kasfShipsDockWithSelection(&((AITeam *)kasCurrentTeamPtr)->shipList, launchMothership);
}

void Watch_BeastCloakedFighterDockAndReinforce(void)
{
    AITeam *currentTeam;
    sdword buildLimit;
    sdword buildCost;
    GrowSelection *linkedMotherships;
    GrowSelection *newFighters;

    currentTeam = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = currentTeam->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, 0);
        return;
    }

    buildLimit = 6 - kasfTeamCount();
    switch (kasfEntryValueGet(-2126662828))
    {
        case 1:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                buildCost = kasfShipTypeBuildCost("bCloakedFighter") / 2;
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 1
              && kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
             || kasfEntryValueGet(154639956) == 6)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfShipTypeBuildTime("bCloakedFighter") * kasfEntryValueGet(154639956) / 2);
            }
            break;
        case 2:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                kasfEntryIncrement(154639956);
            }
            if (kasfEntryValueGet(154639956) == 6)
            {
                kasTimerCreateSetStartById(133682484, 0);
            }
            break;
        default:
            if (kasfEntryValueGet(154639956) < buildLimit)
            {
                buildCost = kasfShipTypeBuildCost("bCloakedFighter");
                if (kasfRUsGet(1) >= buildCost)
                {
                    kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                    kasfEntryIncrement(154639956);
                }
            }
            if ((kasfEntryValueGet(154639956) > 1
              && kasfRUsGet(1) < kasfShipTypeBuildCost("bCloakedFighter"))
             || kasfEntryValueGet(154639956) == 6)
            {
                kasTimerCreateSetStartById(
                    133682484,
                    kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bCloakedFighter"));
            }
            break;
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        linkedMotherships = kasGrowSelectionPtr(316548);
        kasfShipsSelectLinkedWith(&currentTeam->shipList, linkedMotherships, kasAITeamShipsPtrIfExists(51250466));
        newFighters = kasGrowSelectionPtr(154639956);
        kasfShipsCreate(newFighters, "bCloakedFighter", kasfEntryValueGet(154639956), 1);
        kasAITeamShipsPtr(newFighters, 1);
        kasfShipsDockInstant(newFighters, linkedMotherships);
        kasfReinforceTeamWithShips(currentTeam, newFighters);
        kasfShipsAdd(kasGrowSelectionPtr(currentTeam->kasFSMNameHash), &currentTeam->shipList);
        kasfSpeechEvent(84320888, (sdword)Init_LaunchDockedTeam, 0);
    }
}

void Watch_BeastCorvetteCapitalStrike(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastCorvetteCombatLoop, (sdword)Watch_BeastCorvetteCombatLoop);
    }
}

void Watch_BeastAcolyteCapitalStrike(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 20000))
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastAcolyteCombatLoop, (sdword)Watch_BeastAcolyteCombatLoop);
    }
    else if ((kasfShipsOrderAttributes(&team->shipList) & 2) == 0)
    {
        kasfSpeechEvent(0x4C9664, (sdword)Init_BeastAcolyteCapitalStrike, (sdword)Watch_BeastAcolyteCapitalStrike);
    }
}

void Begin_BeastReconCapitalStrike(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(launchTargets))
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastReconCombatLoop, (sdword)Watch_BeastReconCombatLoop);
        return;
    }

    targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
    kasfShipsSelectIndex(kasGrowSelectionPtr(21541), launchTargets, targetIndex);
    kasfLabelledVectorSet(0x3100F7C, 0, 0, 0);
    kasfVectorRandomizeInRange(kasVectorPtrIfExists(0x3100F7C), kasShipsVectorPtrIfExists(21541), 6000);
    kasfMoveTo(kasVectorPtrIfExists(0x3100F7C));
}

void Watch_BeastReconCapitalStrike(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfShipsSelectNearby(kasGrowSelectionPtr(57245931), kasGrowSelectionPtr(0x4A2A29), kasThisTeamsVectorPtr(), 10000))
    {
        kasfSpeechEvent(74024315, (sdword)Begin_BeastReconCombatLoop, (sdword)Watch_BeastReconCombatLoop);
    }
    else if (kasfShipsAnyIdle(&team->shipList))
    {
        kasfSpeechEvent(88512316, (sdword)Begin_BeastReconCapitalStrike, (sdword)Watch_BeastReconCapitalStrike);
    }
}

void Begin_Mission17_BeastDFGFrigateCombatLoop(void);
void Watch_BeastDFGFrigateCombatLoop(void);

void Begin_Mission17_BeastDFGFrigateCombatLoop(void)
{
    AITeam *team;
    GrowSelection *capitalTargets;
    GrowSelection *dfgTargets;
    GrowSelection *attackTargets;
    GrowSelection *ionTargets;
    GrowSelection *protectionTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    capitalTargets = kasGrowSelectionPtr(124053532);
    kasfShipsSelectCapital(capitalTargets, kasGrowSelectionPtr(team->kasFSMNameHash));
    dfgTargets = kasGrowSelectionPtr(288935);
    kasfShipsSelectType(dfgTargets, kasGrowSelectionPtr(team->kasFSMNameHash), "bDFGFrigate");
    attackTargets = kasGrowSelectionPtr(157715175);
    if (kasfShipsSelectTargets(dfgTargets, attackTargets))
    {
        kasfShipsRemove(capitalTargets, attackTargets);
    }
    kasfShipsRemove(capitalTargets, dfgTargets);
    ionTargets = attackTargets;
    if (kasfShipsSelectType(ionTargets, capitalTargets, "bIonArrayFrigate"))
    {
        kasfShipsRemove(capitalTargets, ionTargets);
    }
    if (!kasfShipsCount(capitalTargets))
    {
        return;
    }

    protectionTargets = kasGrowSelectionPtr(154965556);
    if (!kasfShipsSelectType(protectionTargets, capitalTargets, "bCarrier") &&
        !kasfShipsSelectType(protectionTargets, capitalTargets, "bHeavyCruiser"))
    {
        kasfShipsSelectIndex(protectionTargets, capitalTargets, kasfRandom(0, kasfShipsCount(capitalTargets) - 1));
    }
    kasfProtect(protectionTargets);
}

void Watch_BeastDFGFrigateReinforcementSpawn(void)
{
    AITeam *team;
    GrowSelection *launchTargets;
    GrowSelection *spawnPointSelection;
    GrowSelection *createdShips;
    sdword targetIndex;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    launchTargets = kasAITeamShipsPtrIfExists(51250466);
    if (!kasfShipsCount(launchTargets))
    {
        kasfEntrySetValue(154639956, 0);
        kasfSpeechEvent(342540, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }
    if (kasTimerExpiredById(235522949))
    {
        kasfStop();
        targetIndex = kasfRandom(0, kasfShipsCount(launchTargets) - 1);
        spawnPointSelection = kasGrowSelectionPtr(0x5425);
        kasfShipsSelectIndex(spawnPointSelection, launchTargets, targetIndex);
        createdShips = kasGrowSelectionPtr(0x9379E54);
        kasfShipsCreateInside(createdShips, "bDFGFrigate", kasfEntryValueGet(154639956), spawnPointSelection, 1);
        kasAITeamShipsPtr(createdShips, 1);
        kasfEntrySetValue(154639956, 0);
        kasfReinforceTeamWithShips(team, createdShips);
        kasfShipsAdd(kasGrowSelectionPtr(team->kasFSMNameHash), &team->shipList);
    }
    if (kasfTeamFinishedLaunching() && kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)44796563);
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
    }
}

void Watch_BeastDFGFrigateCombatLoop(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfShipsSelectTargets(&team->shipList, kasGrowSelectionPtr(0x9668AE7)))
    {
        kasfSpeechEvent(0x4C9664, (sdword)Begin_Mission17_BeastDFGFrigateCombatLoop, (sdword)Watch_BeastDFGFrigateCombatLoop);
    }
}

void Watch_BeastDFGFrigateDockAndReinforce(void)
{
    AITeam *team;
    sdword buildScale;
    sdword buildLimit;
    sdword buildCost;
    GrowSelection *linkedMotherships;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfSpeechEvent(4693508, (sdword)sub_65F6F0, (sdword)MaintainTeamContext);
        return;
    }

    buildScale = kasfEntryValueGet(-2126662828);
    buildLimit = 2 - kasfTeamCount();
    if (buildScale == 1)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bDFGFrigate") / 2;
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1
          && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate") / 2)
         || kasfEntryValueGet(154639956) == 2)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfShipTypeBuildTime("bDFGFrigate") * kasfEntryValueGet(154639956) / 2);
        }
    }
    else if (buildScale == 2)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            kasfEntryIncrement(154639956);
        }
        if (kasfEntryValueGet(154639956) == 2)
        {
            kasTimerCreateSetStartById(133682484, 0);
        }
    }
    else if (buildScale == 0)
    {
        if (kasfEntryValueGet(154639956) < buildLimit)
        {
            buildCost = kasfShipTypeBuildCost("bDFGFrigate");
            if ((sdword)kasfRUsGet(1) >= buildCost)
            {
                kasfRUsSet(1, kasfRUsGet(1) - buildCost);
                kasfEntryIncrement(154639956);
            }
        }
        if ((kasfEntryValueGet(154639956) > 1 && (sdword)kasfRUsGet(1) < kasfShipTypeBuildCost("bDFGFrigate"))
         || kasfEntryValueGet(154639956) == 2)
        {
            kasTimerCreateSetStartById(
                133682484,
                kasfEntryValueGet(154639956) * kasfShipTypeBuildTime("bDFGFrigate"));
        }
    }

    if (kasfTeamDockedReadyForLaunch() && kasTimerExpiredById(133682484))
    {
        linkedMotherships = kasGrowSelectionPtr(316548);
        kasfShipsSelectLinkedWith(&team->shipList, linkedMotherships, kasAITeamShipsPtrIfExists(51250466));
    }
}


sdword Begin_BeastReinforcementTeamDockAndReinforce(void)
{
    AITeam *team;
    GrowSelection *dockSelection;
    GrowSelection *selectedTarget;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    kasfEntrySetValue(154639956, 0);
    kasTimerDestroyById(133682484);
    dockSelection = kasAITeamShipsPtrIfExists(51250466);
    kasfShipsSortByDistanceToPoint(dockSelection, kasShipsVectorPtr((char *)51250466));
    selectedTarget = kasGrowSelectionPtr(21541);
    kasfShipsSelectIndex(selectedTarget, dockSelection, 0);
    if (kasfTeamCount() <= 8)
    {
        kasfFormationDelta();
    }
    else
    {
        kasfFormationDelta3D();
    }
    return kasfShipsDockWithSelection(&team->shipList, selectedTarget);
}

void Watch_BeastRammingFrigateCombatLoopDelay(void)
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
            (sdword)Begin_BeastRammingFrigateCombatLoop,
            (sdword)Watch_BeastRammingFrigateCombatLoop);
    }
}


void Watch_BeastWorkerHarvestNoWorldResources(void)
{
    AITeam *team;
    GrowSelection *returnedWorkers;

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

    returnedWorkers = kasGrowSelectionPtr(0x58A20);
    kasfGrowSelectionClear(returnedWorkers);
    kasfShipsAdd(returnedWorkers, &team->shipList);
    kasfShipsRemove(&team->shipList, returnedWorkers);
    kasfReinforceTeamWithShips(kasAITeamPtrIfExists(44665972), returnedWorkers);
    kasfSpeechEvent(342540, (sdword)sub_65F6F0, 0);
}

void Begin_BeastWorkerHarvestCloak(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntrySetValue(332667, 1);
    if (team->shipList.selection != NULL && team->shipList.selection->numShips > 0)
    {
        clWrapSpecial(&universe.mainCommandLayer, team->shipList.selection, NULL);
    }
}

void Reset_BeastProcessorCombatLoopTimer()
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerDestroyById(374244);
    kasTimerCreateSetStartById(374244, kasfRandom(10, 20));
}


void Init_FleetIntel_CruiseMissileWaveTeam(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateById(172555554);
    kasfEntryGetOrCreateSetValue(249494211, 0);
    kasfSpeechEvent(110655924, (sdword)MaintainTeamContext, 0);
}

void spHyperspaceRollCallWideScreenIn(sdword frames)
{
    kasfWideScreenIn(frames);
}

void spEventWideScreenIn(sdword frames)
{
    kasfWideScreenIn(frames);
}


void TeamHaltAndClearMoveOrders(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
}
void Watch_BeastWorkerRepair_CloakDamagedTargets(void)
{
    AITeam *team;
    GrowSelection *sourceShips;
    GrowSelection *damagedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    sourceShips = kasGrowSelectionPtr(team->kasFSMNameHash);
    damagedShips = kasGrowSelectionPtr(0x8615BD4);
    if (kasfShipsSelectCapital(damagedShips, sourceShips) &&
        kasfShipsSelectDamaged(damagedShips, damagedShips, 90))
    {
        kasfShipsSortByDistanceToPoint(damagedShips, (vector *)kasThisTeamsVectorPtr());
        kasfCloak(damagedShips);
        return;
    }

    if (kasfShipsSelectType(damagedShips, sourceShips, "bWorker") &&
        kasfShipsSelectDamaged(damagedShips, damagedShips, 90))
    {
        kasfShipsSortByDistanceToPoint(damagedShips, (vector *)kasThisTeamsVectorPtr());
        kasfCloak(damagedShips);
        return;
    }

    kasfSpeechEvent(0xC67AAC4, (sdword)sub_617A10, (sdword)Watch_BeastWorkerRepairCombatLoop);
}

void Watch_BeastWorkerRepair_SpecialAttackDamagedCapital(void)
{
    AITeam *team;
    GrowSelection *damagedShips;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    damagedShips = kasGrowSelectionPtr(0x8615BD4);
    if (!kasfShipsSelectDamaged(damagedShips, kasGrowSelectionPtr(0x4A2A29), 50) ||
        !kasfShipsSelectCapital(damagedShips, damagedShips) ||
        !kasfShipsSelectCanSpecialTargetSelection(damagedShips, damagedShips, &team->shipList))
    {
        kasfSpeechEvent(0xC67AAC4, (sdword)sub_617A10, (sdword)Watch_BeastWorkerRepairCombatLoop);
        return;
    }

    kasfShipsSortByDistanceToPoint(damagedShips, (vector *)kasThisTeamsVectorPtr());
    kasfAttackSpecial(damagedShips);
}

void Init_Mission10_BigGunObjective_CarrierObjective_HideAndTrack(void);
void Watch_Mission10_BigGunObjective_CarrierObjective_WaitForCue(void);
void Init_Mission10_BigGunObjective_CarrierObjective_Release(void);


void Watch_Mission10_BigGunObjective_ResearchShipTeam_ApproachCue(void);
void Init_Mission10_BigGunObjective_ResearchShipTeam_ReleaseHold(void);


void Init_ReinforcementPool_ClearSelectionAndReset(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfTacticsAggressive();
    tutDeallocateRootRegion();
}
void Begin_Mission12_A1DisableForLaunch(void);
void Init_Mission10_BigGunObjective_ClearTrackedSelection(void);


void Init_AttackMothershipDeleteDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(51324946, 20);
}
void Watch_BeastWorkerHarvest_SpecialTargetsResolved(void)
{
    AITeam *team;
    GrowSelection *specialTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(team->kasFSMNameHash), (char *)0xC2F7304);
        kasfSpeechEvent(0x479E04, (sdword)sub_65F6F0, (sdword)Watch_BeastWorkerHarvestReinforcementTeam);
        return;
    }

    specialTargets = kasGrowSelectionPtr(0x8615BD4);
    if (!kasfShipsSelectSpecialTargets(specialTargets, &team->shipList) &&
        !kasfShipsSelectHealthAtLeastPercent(specialTargets, specialTargets, 100))
    {
        kasfEntrySetValue(332667, 0);
        kasfSpeechEvent(0xC67AAC4, (sdword)sub_617A10, (sdword)Watch_BeastWorkerHarvestNoWorldResources);
        return;
    }

    if (!kasfShipsCount(kasAITeamShipsPtrIfExists(0x5BB641B)))
    {
        kasfSpeechEvent(0xC67AAC4, (sdword)sub_617A10, (sdword)Watch_BeastWorkerHarvestNoWorldResources);
    }
}
void Init_BeastWorkerRepairReinforcementTeam_Preamble(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfEntryGetOrCreateSetValue(332667, 1);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(96759714)))
    {
        return;
    }

    kasfEntryGetOrCreateSetValue(154639956, 0);
    kasfEntryGetOrCreateSetValue(111592388, 0);
    kasfEntryGetOrCreateSetValue(-2070679070, 0);
    kasfEntryGetOrCreateSetValue(-2115886635, 1);
}
void Watch_BeastWorkerRepair_SpecialTargetsFullyHealed(void)
{
    AITeam *team;
    GrowSelection *specialTargets;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    specialTargets = kasGrowSelectionPtr(0x8615BD4);
    if (kasfShipsSelectSpecialTargets(specialTargets, &team->shipList) &&
        !kasfShipsSelectHealthAtLeastPercent(specialTargets, specialTargets, 100))
    {
        return;
    }

    kasfSpeechEvent(
        0x4C95C,
        (sdword)Watch_BeastWorkerRepair_CloakDamagedTargets,
        (sdword)Watch_BeastWorkerRepair_SpecialTargetsFullyHealed);
}
