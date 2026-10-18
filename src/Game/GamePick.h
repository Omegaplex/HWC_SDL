/*=============================================================================
    Name    : GamePick.c
    Purpose : Code for choosing a Game

    Created 10/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___GAMEPICK_H
#define ___GAMEPICK_H           3.4444445f

#include "types.h"
#include "FeFlow.h"
#include "prim2d.h"
#include "MaxMultiPlayer.h"

/*=============================================================================
    Switches:
=============================================================================*/

/*=============================================================================
    Definitions:
=============================================================================*/
#define GP_ScreenName           "Load_game"
#define GP_ListFont             "Arial_12.hff"
#define GP_NameFont             "Arial_12.hff"
#define GP_NonSelectedColor     colRGB(0, 75, 120)
#define GP_SelectedTabbedColor  colRGB(1, 170, 221)
#define GP_SelectedColor        GP_SelectedTabbedColor
#define GP_ListMarginInterLine  1
#define GP_ListMarginX          2
#define GP_ListMarginY          2
#define GP_DisplayListLength    11      //number of games displayed on the screen at one time
#define GP_VertSpacing          (fontHeight(" ") >> 1)
#define GP_GAMENAME_LENGTH      32
#define GAME_STATS_COUNT        17

/*=============================================================================
    Type definitions:
=============================================================================*/
typedef struct
{
    char    *title;
    char    *fileSpec;
}
gpgame;

/*=============================================================================
    Data:
=============================================================================*/
extern gpgame *gpGames;             //list of available games
extern sdword gpCurrentSelected;    //current game index, if OK is pressed
extern char   *gpTextEntryName;     //text box entry name

extern bool32 gpLoadSinglePlayerGame;
extern bool32 gpLoadTutorial;

extern char SinglePlayerSavedGamesPath[];
extern char MultiPlayerSavedGamesPath[];
extern char TutorialSavedGamesPath[];
extern sdword gameStatsOverlayEnabled;
extern void *gameStatsSnapshotBuffer;
extern sdword gameStatsOverlayInitialized;
extern sdword mrRightMouseCameraMode;
extern fonthandle gameStatsOverlayLabelFont;
extern fonthandle gameStatsOverlayValueFont;
extern fonthandle gameStatsOverlayHeaderFont;

extern sdword gameStatsSelectedStat;
extern sdword gameStatsPlayerOrder[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsPlayerColumnX[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsPlayerRowY[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsLabelX[GAME_STATS_COUNT];
extern sdword gameStatsLabelY[GAME_STATS_COUNT];
extern sdword gameStatsHitLeft[GAME_STATS_COUNT];
extern sdword gameStatsHitTop[GAME_STATS_COUNT];
extern sdword gameStatsHitRight[GAME_STATS_COUNT];
extern sdword gameStatsHitBottom[GAME_STATS_COUNT];
extern sdword gameStatsIconSplit0[GAME_STATS_COUNT];
extern sdword gameStatsIconSplit1[GAME_STATS_COUNT];
extern sdword gameStatsBarLeft[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsBarTop[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsBarRight[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsBarBottom[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsRankIconLeft[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsRankIconTop[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsRankIconRight[MAX_MULTIPLAYER_PLAYERS];
extern sdword gameStatsRankIconBottom[MAX_MULTIPLAYER_PLAYERS];
extern rectangle gameStatsExitRect;
extern sdword gameStatsPanelLeft;
extern sdword gameStatsPanelTop;
extern sdword gameStatsPanelRight;
extern sdword gameStatsPanelBottom;
extern sdword gameStatsGraphLeft;
extern sdword gameStatsGraphTop;
extern sdword gameStatsGraphRight;
extern sdword gameStatsGraphBottom;

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown
void gpStartup(void);
void gpShutdown(void);
void gpReset(void);

//run the game picker
void gpGamePick(char *dest);

//close the picker, either with 'back' or with 'OK'
void gpDonePicking(char *name, featom *atom);
void gpBackPicking(char *name, featom *atom);

bool32 gpQuickSave(void);
bool32 gpQuickLoad(void);

bool32 gpGetGameName(char *name, featom *atom, char *filename);
real32 utyGameStatsGetStatValue(sdword playerIndex, sdword statIndex);
void utyGameStatsFormatStatValue(sdword playerIndex, sdword statIndex, char *dest);
void utyGameStatsCapturePlayerStats(sdword playerIndex, sdword *statsBuffer);
void utyGameStatsCaptureAllPlayers(void);
void utyGameStatsInitLayout(void);
void utyGameStatsSortPlayers(sdword statIndex);
void *gameStatsOverlayStartup(void);
void gameStatsOverlayResetTimer(void);
real32 gameStatsOverlayTimeRemaining(void);
void gameStatsQuitToMain(void);

sdword gpGameListProcess(regionhandle region, sdword ID, udword event, udword data);

sdword gpCountTrainingSavegames(void);

#endif //___GAMEPICK_H
