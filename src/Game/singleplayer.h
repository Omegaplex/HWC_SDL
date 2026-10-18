/*=============================================================================
    Name    : SinglePlayer.h
    Purpose : Definitions for SinglePlayer.c

    Created 3/3/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SINGLEPLAYER_H
#define ___SINGLEPLAYER_H

#include "KAS.h"
#include "LinkedList.h"
#include "Types.h"

typedef struct tagfeatom featom;

/*=============================================================================
    Switches:
=============================================================================*/

#ifdef HW_BUILD_FOR_DEBUGGING

#define SP_ERROR_CHECKING           1           //general error checking
#define SP_VERBOSE_LEVEL            3           //control specific output code
#define SP_NISLET_TEST              1           //test NISlets
#define SP_DEBUGKEYS                1           //single player debugging keys
#define SP_DEBUGLEVEL2              1           //more debug keys

#else

#define SP_ERROR_CHECKING           0           //general error checking
#define SP_VERBOSE_LEVEL            0           //control specific output code
#define SP_NISLET_TEST              0
#define SP_DEBUGKEYS                0           //single player debugging keys
#define SP_DEBUGLEVEL2              0           //more debug keys

#endif

/*=============================================================================
    Definitions:
=============================================================================*/
typedef enum
{
    NO_HYPERSPACE = 0,
    HYPERSPACE_PREWAIT = 1,
    HYPERSPACE_WAITINGROLLCALL = 2,
    HYPERSPACE_WAITINGFORFLEETCOMMAND = 3,
    HYPERSPACE_SHIPSLEAVING = 4,
    HYPERSPACE_WHITEOUT = 5,
    HYPERSPACE_LOADDONE = 6,
    HYPERSPACE_SHIPSARRIVING = 7,
    HYPERSPACE_WAITINGFORNISARRIVAL = 8,
    HYPERSPACE_LAUNCH_DOCKED = 9,
    HYPERSPACE_FAILED = 10
} HyperspaceState;

#define HS_INACTIVE         0
#define HS_POPUP_INTO       1
#define HS_POPUP_OUTOF      2
#define HS_SLICING_INTO     3
#define HS_SLICING_OUTOF    4
#define HS_COLLAPSE_INTO    5
#define HS_COLLAPSE_OUTOF   6
#define HS_FINISHED         7

#define HSF_WAIT            1   //wait for bit to clear before changing state

#define SP_CAMERA_POINT_NAME        "StartCameraPosition"

#define NUMBER_SINGLEPLAYER_MISSIONS 19

/*=============================================================================
    Type definitions:
=============================================================================*/
// for HYPERSPACE_SHIPSLEAVING, here are some substates:
typedef enum
{
    FOCUSING_ON_MOTHERSHIP,
    LOCKED_FOCUS_FOR_HOP
} HyperspaceSubState;

typedef enum
{
    MISSION_ENUM_NOT_INITIALISED = 0,

    // these preserve the original mission numbers 
    MISSION_1_KHARAK_SYSTEM,
    MISSION_2_OUTSKIRTS_OF_KHARAK_SYSTEM,
    MISSION_3_RETURN_TO_KHARAK,
    MISSION_4_GREAT_WASTELANDS_TRADERS,
    MISSION_5_GREAT_WASTELANDS_REVENGE,
    MISSION_6_DIAMOND_SHOALS,
    MISSION_7_THE_GARDENS_OF_KADESH,
    MISSION_8_THE_CATHEDRAL_OF_KADESH,
    MISSION_9_SEA_OF_LOST_SOULS,
    MISSION_10_SUPER_NOVA_STATION,
    MISSION_11_TENHAUSER_GATE,
    MISSION_12_GALACTIC_CORE,
    MISSION_13_THE_KAROS_GRAVEYARD,
    MISSION_14_BRIDGE_OF_SIGHS,
    MISSION_15_CHAPEL_PERILOUS,
    MISSION_16_HIIGARA,
    
    // whilst these are out of sequence
    MISSION_5B_TURANIC_RAIDER_PLANETOID,
    MISSION_TUTORIAL,

} MissionEnum;

typedef struct
{
    MissionEnum currentMission;
    sdword onCompletePlayNIS;
    bool32 onCompleteHyperspace;

    sdword onLoadPlayNIS;

    HyperspaceState hyperspaceState;
    udword oldAutoLaunch;    //bits 0,1,2 = mothership, carrier1, carrier2
    real32 oldClipPlane;
    sdword rollcallShipsDocked;
    sdword rollcallShipsRemaining;
    HyperspaceSubState hyperspaceSubState;
    real32 hyperspaceTimeStamp;
    LinkedList ShipsInHyperspace;   // linked list of ships in hyperspace ether (uses InsideShip structure)
    sdword resourceUnitsCollected;

    GrowSelection ShipsToHyperspace;
    bool32 giveComputerFleetControl;
    bool32 asteroid0sCanMove;

    bool32 playerCanHyperspace;
    bool32 hyperspaceFails;

} SinglePlayerGameInfo;             // later save this data structure

static_assert(sizeof(SinglePlayerGameInfo) == 84, "SinglePlayerGameInfo save size mismatch");

typedef enum
{
    SHIPHYPERSPACE_NONE,
    SHIPHYPERSPACE_WAITING,
    SHIPHYPERSPACE_ACCELERATINGAWAY,
    SHIPHYPERSPACE_WHIPAWAY,
    SHIPHYPERSPACE_GONE,
    SHIPHYPERSPACE_WHIPTOWARDS,
    SHIPHYPERSPACE_DECELERATE,
    SHIPHYPERSPACE_DONE
} ShipHyperspaceState;

typedef struct ShipSinglePlayerGameInfo
{
    bool32 midlevelHyperspaceIn;
    bool32 midlevelHyperspaceOut;
    bool32 staticHyperspaceGate;
    hvector midlevelHyperspaceTo;
    struct Effect* hyperspaceEffect;

    ShipHyperspaceState shipHyperspaceState;
    real32 timeToHyperspace;
    real32 timeToDelayHyperspace;
    real32 forbehind;       // how far ship is forward or behind mothership
    real32 rightleft;       // how far ship is right or left of mothership
    real32 updown;          // how far ship is up or down of mothership

    real32 clipt;                   //[0..1] or -1, hyperspace clip parameter
    real32 cliptDelta;              //clipt incrementer
    udword hsState;                 //hyperspace effect state
    udword hsFlags;
} ShipSinglePlayerGameInfo;

extern SinglePlayerGameInfo singlePlayerGameInfo;
extern bool32 singlePlayerGameLoadNewLevelFlag;
extern bool32 singlePlayerSuppressHorseRaceLoadScreen;
extern bool32 hyperspaceFails;
extern bool32 singlePlayerHyperspacingInto;
extern bool32 spCollectResourcesAtEndOfMission;
extern real32 GameDifficultyPrimaryModifier;
extern sdword GameDifficultyBuildTimeModifier;
extern sdword GameDifficultyProductionModifier;
extern sdword cmdNewMission;
extern bool32 spPostLoadNisBlackout;
extern bool32 spPreservePostLoadNisMode;
extern real32 cat_mrBlackOut;

BOOL isSinglePlayerCampaignGame(void);
void gameDifficultyGetModifiers(real32 *primaryModifier, sdword *buildTimeModifier, sdword *tertiaryModifier, sdword difficulty);
void gameDifficultyUpdateModifiers(void);
void gameDifficultySettingCB(char *name, featom *atom);
bool32 spCurrentPlayerHasHyperspaceShip(void);
void singlePlayerForgetDamageEffectsRecursive(Ship *ship);
void singlePlayerSetWideScreenSpeedScale(bool32 enabled);

void spUnlockout(void);
void spHyperspaceButtonPushed(void);

void spHyperspaceSetSelectionDestination(SelectCommand *selection,hvector *destination);
void spHyperspaceSelectionIn(SelectCommand *selection);
void spHyperspaceSelectionOut(SelectCommand *selection);
void spHyperspaceSelectionInStatic(SelectCommand *selection,hvector *destination);
void spHyperspaceSelectionOutStatic(SelectCommand *selection);

void spResetMissionSequenceToBeginning(void);
void singlePlayerMissionScriptInit(char *directory, char *filename);
void singlePlayerInit(void);
void singlePlayerPostInit(bool32 loadingSaveGame);
void singlePlayerClose(void);
void singlePlayerSetMissionAttributes(char *directory,char *filename);

void singlePlayerGameCleanup(void);

void singlePlayerGameUpdate();
void smMimicInitPlayerShipTypes(void);
sdword smMimicGetCurrentPlayerIndex(void);
void smMimicSetCurrentPlayerIndex(sdword playerIndex);
bool32 smMimicIsPlayerIndexValid(sdword playerIndex);
char *smMimicGetPlayerLabel(sdword playerIndex);
sdword smMimicGetEntryCount(sdword playerIndex, sdword groupIndex);
void *smMimicGetEntryAddress(sdword playerIndex, sdword groupIndex, sdword entryIndex);
char *smMimicGetEntryLabel(sdword playerIndex, sdword groupIndex, sdword entryIndex);
sdword smMimicAddShipType(ubyte orderGroup, sdword shipType);
sdword titan_5BA150(ubyte orderGroup, sdword count);
void singlePlayerLaunchDockedShips(Ship *carrier);
void singlePlayerLoadNewLevel(bool32 loadNISLevel, bool32 suppressMissionAdvance);
sdword spPostLoadAutosaveSave(char *path);
void singlePlayerStartGame(void);
void singlePlayerStartCampaign(void);
void singlePlayerNextLevel(void);
char *singlePlayerGetHyperspaceObjectiveLabel(void);

void singlePlayerShipDied(Ship *ship);

void singlePlayerMissionCompleteCB(void);
void singlePlayerMissionFailedCB(void);

#if SP_DEBUGKEYS
void singlePlayerCheckDebugKeys(sdword ID);
#endif

void spSetCurrentMission(MissionEnum mission);

MissionEnum spGetPreviousMission(void);
MissionEnum spGetCurrentMission (void);
MissionEnum spGetNextMission    (void);

void GetMissionsDirAndFile(MissionEnum mission, bool32 loadNISLevel);
void singlePlayerKasMissionStart(sdword missionnum);

void spMainScreen();

bool32 GetStartPointPlayer(hvector *startpoint);
bool32 GetPointOfName(hvector *point,char *name);

ShipSinglePlayerGameInfo *spNewShipSinglePlayerGameInfo();

// flags for spLockout
#define SPLOCKOUT_MOUSE     1
#define SPLOCKOUT_MR        2
#define SPLOCKOUT_DESELECT  4
#define SPLOCKOUT_EVERYTHING    (SPLOCKOUT_MOUSE+SPLOCKOUT_MR+SPLOCKOUT_DESELECT)

void spMainScreen();
void spLockout(udword flags);
void spMainScreenAndLockout(udword flags);
void spMainScreenCleanupAndLockout(sdword flags);
void spClearScreenLockout(void);
bool32 spScreenLockoutActiveGet(void);
void mrManagerScreenBegin(sdword flags);
void spCleanupScreenStack(void);
void spAutoSaveGame(void);
int spSetSinglePlayerSaveInfo(int missionNumber, const char *missionName);
extern sdword spAutoSavePending;

void singlePlayerRegisterShockwaveProjectile(SpaceObjRotImpTarg *projectile);
void singlePlayerUnregisterShockwaveProjectile(SpaceObjRotImpTarg *projectile);
SpaceObjRotImpTarg *spaceObjEnableAttachNLips(SpaceObjRotImpTarg *spaceObj);
void singlePlayerCollectHyperspaceShipTypesForNIS(sdword *count, ShipType *shipTypes);
char *singlePlayerGetHyperspaceObjectiveLabel(void);

void LoadSinglePlayerGame(void);
void SaveSinglePlayerGame(void);

MissionEnum      WatchFunctionToMissionEnum(KASWatchFunction watchFunction);
KASWatchFunction MissionEnumToWatchFunction(MissionEnum mission);

udword FunctionListSize(MissionEnum mission);
const void** MissionEnumToFunctionList(MissionEnum mission);

#if SP_NISLET_TEST
void spNISletTestAttempt(sdword index);
#endif

void spRestoreManagerScreen(void);

void singlePlayerNisStoppedCB(void);
void singlePlayerNISNamesGet(char **nisname, char **scriptname, bool32 *loadLevelBeforeStart, sdword nisNumber);
bool32 singlePlayerNISletNamesGet(char **nisname, char **scriptname, sdword nisletNumber);
void singlePlayerStartNis(char *nisname, char *scriptname, hvector *positionAndAngle, bool32 loadLevelBeforeStart, bool32 suppressMissionAdvance);
bool32 spFindCameraAttitude(vector *position);

void UpdateMidLevelHyperspacingShips();

void singlePlayerPreLoadCheck(void);
sdword *smMimicPlayerTypeData(void);
sdword *smMimicPlayerTypeExtra0(void);
sdword *smMimicPlayerTypeExtra1(void);

//stores the current mission filename
extern char CurrentLevelName[];

/*=============================================================================
    Data:
=============================================================================*/

extern sdword SINGLEPLAYER_STARTINGRUS;
extern char spMissionsDir[];
extern char spMissionsFile[];

extern real32 spFleetModifier;

extern real32 spHyperspaceDelay;
extern bool32   spHoldHyperspaceWindow;

extern real32 HYPERSPACEGATE_HEALTH;
extern real32 HYPERSPACEGATE_WIDTH;
extern real32 HYPERSPACEGATE_HEIGHT;
extern real32 HYPERSPACEGATE_WAYPOINTDIST;

extern real32 cat_mrBlackoutEndTime;

extern real32 SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE;

void smMimicSetPlayerShipTypeExtra1(ubyte playerIndex, sdword value);

#endif
