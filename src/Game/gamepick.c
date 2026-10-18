/*=============================================================================
    Name    : GamePick.c
    Purpose : Functionality to choose a game

    Created 10/21/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "types.h"
#include "debug.h"
#include "file.h"
#include "memory.h"
#include "feflow.h"
#include "utility.h"
#include "font.h"
#include "fontreg.h"
#include "prim2d.h"
#include "scroller.h"
#include "SaveGame.h"
#include "GamePick.h"
#include "multiplayergame.h"
#include "universe.h"
#include "netcheck.h"
#include "globals.h"
#include "FEColour.h"
#include "StringsOnly.h"
#include "strings.h"
#include "CommandWrap.h"
#include "singleplayer.h"
#include "mainrgn.h"
#include "soundevent.h"
#include "task.h"
#include "Timer.h"
#include "researchapi.h"

/*=============================================================================
    Data:
=============================================================================*/

bool32 gpQuickSave(void);
bool32 gpQuickLoad(void);

void gpRecGameWindowInit(char *name, featom *atom);
void gpGameWindowInit(char *name, featom *atom);
void gpTextEntryWindowInit(char *name, featom *atom);
void gpGameNameDraw(featom *atom, regionhandle region);
void gpSaveGivenGame(char* gamename);
void gpLoadGivenGame(char* gamename);
void gpSaveGame(char *name, featom *atom);
void gpLoadGame(char *name, featom *atom);
void gpLoadTheRecordedGame(char *name, featom *atom);
void gpSaveTheRecordedGame(char *name, featom *atom);
void gpDeleteGame(char *name, featom *atom);
void gpBackToInGameEscapeMenu(char *name, featom *atom);
void gpStopPlayback(char *name, featom *atom);
void gpStopRecording(char *name, featom *atom);

void gpOverwriteYes(char *name, featom *atom);
void gpOverwriteNo(char *name, featom *atom);
extern sdword mouseCursorType;
void hrShutdownBackground(void);
static void gpInitSavedGamePaths(void);
static bool32 gpRecordedGameIsMultiPlayerRecording(void);

typedef struct
{
    real32 startTime;
    real32 duration;
    real32 pauseTime;
    bool32 enabled;
} GameStatsTaskTimer;

sdword gameStatsOverlayEnabled = 0;
void *gameStatsSnapshotBuffer = NULL;
fonthandle gameStatsOverlayLabelFont = 0;
fonthandle gameStatsOverlayValueFont = 0;
fonthandle gameStatsOverlayHeaderFont = 0;
GameStatsTaskTimer gameStatsOverlayTimer = { 0 };
sdword gameStatsOverlayInitialized = 0;
sdword mrRightMouseCameraMode = 0;

static const strGamesMessages gameStatsLabelStringIds[GAME_STATS_COUNT] =
{
    str_GSD_SurvivalTime,
    str_GSD_PlayersKilled,
    str_GSD_ResourcesHarvested,
    str_GSD_CrystalsHarvested,
    str_GSD_TechResearched,
    str_GSD_FightersBuilt,
    str_GSD_FightersKilled,
    str_GSD_FightersLost,
    str_GSD_FrigatesBuilt,
    str_GSD_FrigatesKilled,
    str_GSD_FrigatesLost,
    str_GSD_DestroyersBuilt,
    str_GSD_DestroyersKilled,
    str_GSD_DestroyersLost,
    str_GSD_NonCombatBuilt,
    str_GSD_NonCombatKilled,
    str_GSD_NonCombatLost
};

static const ubyte gameStatsHalfStepRows[10] = { 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 };

sdword gameStatsSelectedStat = 0;
sdword gameStatsPlayerOrder[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsPlayerColumnX[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsPlayerRowY[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsLabelX[GAME_STATS_COUNT] = { 0 };
sdword gameStatsLabelY[GAME_STATS_COUNT] = { 0 };
sdword gameStatsHitLeft[GAME_STATS_COUNT] = { 0 };
sdword gameStatsHitTop[GAME_STATS_COUNT] = { 0 };
sdword gameStatsHitRight[GAME_STATS_COUNT] = { 0 };
sdword gameStatsHitBottom[GAME_STATS_COUNT] = { 0 };
sdword gameStatsIconSplit0[GAME_STATS_COUNT] = { 0 };
sdword gameStatsIconSplit1[GAME_STATS_COUNT] = { 0 };
sdword gameStatsBarLeft[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsBarTop[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsBarRight[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsBarBottom[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsRankIconLeft[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsRankIconTop[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsRankIconRight[MAX_MULTIPLAYER_PLAYERS] = { 0 };
sdword gameStatsRankIconBottom[MAX_MULTIPLAYER_PLAYERS] = { 0 };
rectangle gameStatsExitRect = { 0, 0, 0, 0 };
sdword gameStatsPanelLeft = 0;
sdword gameStatsPanelTop = 0;
sdword gameStatsPanelRight = 0;
sdword gameStatsPanelBottom = 0;
sdword gameStatsGraphLeft = 0;
sdword gameStatsGraphTop = 0;
sdword gameStatsGraphRight = 0;
sdword gameStatsGraphBottom = 0;

#define GAME_STATS_PANEL_WIDTH              640
#define GAME_STATS_PANEL_HEIGHT             480
#define GAME_STATS_EXIT_WIDTH               200
#define GAME_STATS_EXIT_TOP_OFFSET          30
#define GAME_STATS_EXIT_HEIGHT              40
#define GAME_STATS_LABEL_LEFT_MARGIN        10
#define GAME_STATS_VALUE_INSET              10
#define GAME_STATS_PLAYER_X_OFFSET          28
#define GAME_STATS_PLAYER_Y_OFFSET          20
#define GAME_STATS_ROW_SPACING              1
#define GAME_STATS_ROW_GROUP_GAP            30
#define GAME_STATS_PLAYER_BAR_GAP           35
#define GAME_STATS_LABEL_EXTRA_GAP          10
#define GAME_STATS_GRAPH_PADDING            10

static sdword utyGameStatsCountResearchedTechs(Player *player)
{
    sdword techIndex;
    sdword researchedCount = 0;

    for (techIndex = 0; techIndex < 41; techIndex++)
    {
        if (rmTechIsResearched(player, techIndex))
        {
            researchedCount++;
        }
    }

    return researchedCount;
}

void utyGameStatsCaptureAllPlayers(void)
{
    sdword playerIndex;

    if (gameStatsSnapshotBuffer != NULL)
    {
        memFree((ubyte *)gameStatsSnapshotBuffer);
        gameStatsSnapshotBuffer = NULL;
    }

    if (universe.numPlayers <= 0)
    {
        return;
    }

    gameStatsSnapshotBuffer = memAlloc(sizeof(sdword) * GAME_STATS_COUNT * universe.numPlayers, "gameStatsBuffer", NonVolatile);
    for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++)
    {
        utyGameStatsCapturePlayerStats(playerIndex, (sdword *)((ubyte *)gameStatsSnapshotBuffer + sizeof(sdword) * GAME_STATS_COUNT * playerIndex));
    }
}

void utyGameStatsSortPlayers(sdword statToSort)
{
    sdword i;

    gameStatsSelectedStat = statToSort;
    for (i = 0; i < universe.numPlayers; i++)
    {
        gameStatsPlayerOrder[i] = i;
    }

    i = 0;
    while (i < universe.numPlayers - 1)
    {
        if (utyGameStatsGetStatValue(gameStatsPlayerOrder[i], statToSort) >=
            utyGameStatsGetStatValue(gameStatsPlayerOrder[i + 1], statToSort))
        {
            i++;
        }
        else
        {
            sdword temp = gameStatsPlayerOrder[i];
            gameStatsPlayerOrder[i] = gameStatsPlayerOrder[i + 1];
            gameStatsPlayerOrder[i + 1] = temp;
            i = 0;
        }
    }
}

bool32 gpLoadSinglePlayerGame = FALSE;
bool32 gpLoadTutorial = FALSE;

//SinglePlayerSavedGamesPath is non-static because it is used in KASFunc.c
char SinglePlayerSavedGamesPath[PATH_Max + 1] = "";
char MultiPlayerSavedGamesPath[PATH_Max + 1] = "";
static char RecordedGamesPath[PATH_Max + 1] = "";
//TutorialSavedGamesPath is non-static because it is used in Tutor.c
char TutorialSavedGamesPath[PATH_Max + 1] = "";
static char QuickSaveName[] = "Quick Save";

char *SavedGamesPath = NULL;

fecallback gpCallbacks[] =
{
    {gpGameWindowInit,         "FE_RecordedGameWindowInit"},
    {gpGameWindowInit,         "FE_GameWindowInit"},
    {gpGameWindowInit,         "FE_TutorialGameWindowInit"},
    {gpTextEntryWindowInit,    "IG_TextEntryWindowInit"},
    {gpStopPlayback,           "IG_StopPlayback"},
    {gpStopRecording,          "IG_StopRecording"},
    {gpLoadGame,               "IG_LoadGame"},
    {gpLoadTheRecordedGame,    "LoadRecordedGame"},
    {gpSaveTheRecordedGame,    "IG_SaveRecordedGame"},
    {gpSaveGame,               "IG_SaveGame"},
    {gpDeleteGame,             "IG_DeleteGame"},
    {gpBackToInGameEscapeMenu, "IG_StartEscapeMenu"},
    //{gpAcceptBackToInGameEscapeMenu, "IG_StartEscapeMenu_AcceptOptions"},
    {gpOverwriteYes,           "OverwriteGameYes" },
    {gpOverwriteNo,            "OverwriteGameNo" },
    {NULL, NULL}
};

fedrawcallback gpDrawCallbacks[] =
{
    {gpGameNameDraw, "FE_GameName"},
    {NULL, NULL}
};

listwindowhandle gpGameListWindow = NULL;
textentryhandle  gpNameEntryBox   = NULL;

enum {
    ESCAPE_DEFAULT = 0,
    ESCAPE_SINGLE,
    ESCAPE_RECORDING,
    ESCAPE_PLAYING,
    ESCAPE_TUTORIAL,
    ESCAPE_DEFAULT_NOSAVE,
    ESCAPE_RECORDING_NOSAVE
};

/*=============================================================================*/

gpgame *gpGames;                                //list of available games
sdword gpCurrentGame = 0;                       //current game index, 0 is always default
sdword gpCurrentSelected = 0;                   //current game index, if OK is pressed
sdword gpNumberGames = 0;                       //number of available games
char   *gpTextEntryName;

fonthandle gpListFont, gpNameFont;              //fonts used for font printing

color gpSelectedColor = GP_SelectedColor;       //color to draw selected one at

uword gpTopIndex = 0;                          //index of what is at the top of the list
uword gpDisplayLength = GP_DisplayListLength;  //number of games displayed at one time

sdword escapemenu;                              //remember what scren to return to

/*=============================================================================
    Private functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : gpGameItemDraw
    Description : Draw the list of available games
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpGameItemDraw(rectangle *rect, listitemhandle data)
{
    char            temp[64];
    sdword          x, y;
    color           c;
    fonthandle      oldfont;
    gpgame         *gpGame = (gpgame *)data->data;

    oldfont = fontMakeCurrent(gpListFont);

    if (data->flags&UICLI_Selected)
        c = FEC_ListItemSelected;
    else
        c = FEC_ListItemStandard;

    x = rect->x0;
    y = rect->y0;

    sprintf(temp,"%s",gpGame->title);
    fontPrint(x,y,c,temp);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : gpGameNameDraw
    Description : Draw the currently selected game name in screens other
                    than the game picker.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpGameNameDraw(featom *atom, regionhandle region)
{
    fonthandle fhSave;
    char gameName[GP_GAMENAME_LENGTH];

    rectangle *r = &region->rect;

    fhSave = fontMakeCurrent(gpNameFont); //select the appropriate font
    memStrncpy(gameName, gpGames[gpCurrentSelected].title, GP_GAMENAME_LENGTH - 1);
    _strupr(gameName);
    fontPrintf(r->x0, r->y0, GP_SelectedColor, gameName);
    fontMakeCurrent(fhSave);
}

/*-----------------------------------------------------------------------------
    Name        : compareGamesCB
    Description : compare's the games alphabetically
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int compareGamesCB(const void *arg1,const void *arg2)
{
    int result;

    gpgame *game1 = (gpgame *)arg1;
    gpgame *game2 = (gpgame *)arg2;

    if (!_stricmp(game1->title, QuickSaveName))
    {
        return -1;
    }
    else if (!_stricmp(game2->title, QuickSaveName))
    {
        return 1;
    }

    result = _stricmp(game1->title,game2->title);
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : gpCountTrainingSavegames
    Description : count the number of training savegames
    Inputs      :
    Outputs     :
    Return      : >= 0
----------------------------------------------------------------------------*/
sdword gpCountTrainingSavegames(void)
{
    struct _finddata_t find;
    char fileSearch[PATH_Max + 1];
    sdword handle, startHandle;
    sdword hits;

    hits = 0;

    strcpy(fileSearch, TutorialSavedGamesPath);
    strcat(fileSearch, "*.*");

    startHandle = handle = _findfirst(filePathPrepend(fileSearch, 0), &find);

    while (handle != -1)
    {
        if (find.name[0] != '.')
        {
            hits++;
        }

        handle = _findnext(startHandle, &find);
    }

    return hits;
}

/*-----------------------------------------------------------------------------
    Name        : gpTitleListLoad
    Description : Scans the games directory and loads in the titles of all
                    the game files available.
    Inputs      : void
    Outputs     : Fills in gpGames and gpNumberGames
    Return      : void
----------------------------------------------------------------------------*/
void gpTitleListLoad(void)
{
    struct _finddata_t find;
    sdword handle, startHandle;
    sdword index;
    char fileName[_MAX_PATH];
    char fileSearch[PATH_Max + 1];
//    char *pString;
//    char *title;

    for (index = 0; index < gpNumberGames; index++)
    {
        memFree(gpGames[index].fileSpec);
        memFree(gpGames[index].title);
    }
    if (gpNumberGames > 0)
    {
        memFree(gpGames);
        gpGames = NULL;
        gpNumberGames = 0;
    }

    strcpy(fileSearch,SavedGamesPath);
    strcat(fileSearch,"*.*");

    startHandle = handle = _findfirst(filePathPrepend(fileSearch, 0), &find);

    while (handle != -1)
    {
        if (find.name[0] == '.')
        {
            goto alreadyLoaded;
        }
        fileName[0] = 0;

        strcpy(fileName, find.name);

        if (fileName[0] == 0)
        {
            goto alreadyLoaded;
        }

        if (strstr(fileName,PKTS_EXTENSION))
        {
            goto alreadyLoaded;
        }

        for (index = 0; index < gpNumberGames; index++)
        {
            if (!_stricmp(gpGames[index].fileSpec, fileName))
            {                                               //if matching file specs,
                goto alreadyLoaded;                         //break-continue
            }
        }

        gpGames = memRealloc(gpGames, (gpNumberGames+1) * sizeof(gpgame), "gpGames", NonVolatile);

        gpGames[gpNumberGames].fileSpec = memStringDupe(fileName);
        //gpGames[gpNumberGames].title = title;
        gpGames[gpNumberGames].title = memStringDupe(fileName);

        gpNumberGames++;
alreadyLoaded:;
        handle = _findnext(startHandle, &find);
    }

    if (gpNumberGames > 1)
    {
        //alphabetically sort the game list
        qsort(&gpGames[0],gpNumberGames,sizeof(gpgame),compareGamesCB);
    }
    gpCurrentSelected = gpCurrentGame = 0;      //set default game
}
/*-----------------------------------------------------------------------------
    Name        : gpTextEntryWindowInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpTextEntryWindowInit(char *name, featom *atom)
{
    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        gpNameEntryBox = (textentryhandle)atom->pData;
//        uicTextEntrySet(gpNameEntryBox,gpTextEntryName,strlen(gpTextEntryName)+1);
        uicTextBufferResize(gpNameEntryBox,GP_GAMENAME_LENGTH-2);
        bitSet(gpNameEntryBox->textflags, UICTE_FileName);
        return;
    }
    else if ((FELASTCALL(atom)))
    {
        gpNameEntryBox = NULL;
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_LoseFocus :
        case CM_AcceptText :
            strcpy(gpTextEntryName,gpNameEntryBox->textBuffer);
        break;
        case CM_GainFocus :
        break;
    }
}

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : gpStartup
    Description :
    Inputs      :
    Outputs     : Registers some callbacks for buttons and user regions.
    Return      :
----------------------------------------------------------------------------*/
void gpStartup(void)
{
    feDrawCallbackAddMultiple(gpDrawCallbacks);
    feCallbackAddMultiple(gpCallbacks);

    gpInitSavedGamePaths();
    gpListFont = frFontRegister(GP_ListFont);
    gpNameFont = frFontRegister(GP_NameFont);
    gpTextEntryName = memAlloc(GP_GAMENAME_LENGTH, "gpTextEntryName", NonVolatile);
    gpTextEntryName[0] = 0;
    gpGames = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : gpReset
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpReset(void)
{
    SavedGamesPath = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : gpShutdown
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpShutdown(void)
{
    sdword index;

    for (index = 0; index < gpNumberGames; index++)
    {
        memFree(gpGames[index].fileSpec);
        memFree(gpGames[index].title);
    }
    if (gpGames!=NULL)
        memFree(gpGames);
    if (gpTextEntryName!=NULL)
        memFree(gpTextEntryName);
//    frFontUnregister(gpListFont);
//    frFontUnregister(gpNameFont);
}

char* gpQuickSetup(void)
{
    if (gameIsRunning)
    {
        if (singlePlayerGame)
        {
            gpLoadSinglePlayerGame = TRUE;
        }
        else
        {
            gpLoadSinglePlayerGame = FALSE;
        }

        gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
    }
    else
    {
        if (mgRunning)
        {
            gpLoadSinglePlayerGame = FALSE;
        }
        else
        {
            gpLoadSinglePlayerGame = TRUE;
        }

        gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
    }

    if (gpLoadSinglePlayerGame)
    {
        return(gpLoadTutorial ? TutorialSavedGamesPath : SinglePlayerSavedGamesPath);
    }
    else
    {
        return(MultiPlayerSavedGamesPath);
    }
}

bool32 gpQuickSave(void)
{
    char* path;
    char  filename[PATH_Max + 1];

    path = gpQuickSetup();
    if (path == NULL)
    {
        dbgMessage("\ngpQuickSave couldn't find a dir");
        return FALSE;
    }

    strcpy(filename, path);
    strcat(filename, QuickSaveName);
    gpSaveGivenGame(filename);

    return TRUE;
}

bool32 gpQuickLoad(void)
{
    char* path;
    char  filename[PATH_Max + 1];

    path = gpQuickSetup();
    if (path == NULL)
    {
        dbgMessage("\ngpQuickLoad couldn't find a dir");
        return FALSE;
    }

    strcpy(filename, path);
    strcat(filename, QuickSaveName);
    gpLoadGivenGame(filename);

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : gpGameWindowInit
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void gpGameWindowInit(char *name, featom *atom)
{
    fonthandle  oldfont;
    sdword      index;

    if (FEFIRSTCALL(atom))
    {
        if (_stricmp(name,"FE_RecordedGameWindowInit") == 0)
        {
            SavedGamesPath = RecordedGamesPath;
        }
        else if (_stricmp(name,"FE_TutorialGameWindowInit") == 0)
        {
            SavedGamesPath = TutorialSavedGamesPath;
            tutorial = TUTORIAL_ONLY;
            gpLoadTutorial = TRUE;
            gpLoadSinglePlayerGame = TRUE;
        }
        else
        {
            if (gameIsRunning)
            {
                if (singlePlayerGame)
                {
                    gpLoadSinglePlayerGame = TRUE;
                }
                else
                {
                    gpLoadSinglePlayerGame = FALSE;
                }

                gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
            }
            else
            {
                if (mgRunning)
                {
                    gpLoadSinglePlayerGame = FALSE;
                }
                else
                {
                    gpLoadSinglePlayerGame = TRUE;
                }

                gpLoadTutorial = (tutorial==TUTORIAL_ONLY) ? TRUE : FALSE;
            }

            if (gpLoadSinglePlayerGame)
            {
                SavedGamesPath = gpLoadTutorial ? TutorialSavedGamesPath : SinglePlayerSavedGamesPath;
            }
            else
            {
                SavedGamesPath = MultiPlayerSavedGamesPath;
            }
        }

        gpTitleListLoad();

        oldfont = fontMakeCurrent(gpListFont);

        gpGameListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(gpGameListWindow,
                          NULL,                             // title draw, no title
                          NULL,                             // title click process, no title
                          0,                                // title height, no title
                          gpGameItemDraw,                   // item draw function
                          fontHeight(" ")+GP_VertSpacing,   // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);

        for (index = 0; index < gpNumberGames; index++)
        {
            if (index == 0)
            {
                uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect|UICLI_Selected, UICLW_AddToTail);
                gpCurrentSelected = 0;
            }
            else
            {
                uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect, UICLW_AddToTail);
            }
        }

        fontMakeCurrent(oldfont);
        return;
    } else if (FELASTCALL(atom))
    {
        gpGameListWindow = NULL;
        return;
    }
    else if (gpGameListWindow->message == CM_NewItemSelected)
    {
        if (gpNameEntryBox != NULL)
        {
            uicTextEntrySet(gpNameEntryBox, ((gpgame *)gpGameListWindow->CurLineSelected->data)->title,strlen(((gpgame *)gpGameListWindow->CurLineSelected->data)->title));
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : gpGamePick
    Description : Starts the game picker
    Inputs      : dest - where to store file name of destiation file.
                    Will be "" if nothing (yet) chosen.
    Outputs     : Starts the game picker screen.  When chosen, the name
                    will be stored in dest.
    Return      : void
----------------------------------------------------------------------------*/
void gpGamePick(char *dest)
{
    if (dest[0] == 'S')
    {
        gpLoadSinglePlayerGame = TRUE;
    }
    else
    {
        gpLoadSinglePlayerGame = FALSE;
    }

    SavedGamesPath = (gpLoadSinglePlayerGame) ? SinglePlayerSavedGamesPath : MultiPlayerSavedGamesPath;

    feScreenStart(ghMainRegion, GP_ScreenName);
}

/*-----------------------------------------------------------------------------
    Name        : gpDonePicking/gpBackPicking
    // if the region is the game picker
    Description : Close the color picker and optionally specify the file path
                    of the chosen file.
    Inputs      : standard FE callback
    Outputs     : DonePicking stores chosen file name in parameter to gpGamePick
    Return      : void
----------------------------------------------------------------------------*/
void gpDonePicking(char *name, featom *atom)
{
    sdword index;

    gpCurrentSelected = gpCurrentGame;

    if (gpGameListWindow->CurLineSelected!=NULL)
    {
        for (index = 0; index < gpNumberGames; index++)
        {
            if (gpGameListWindow->CurLineSelected->data == (ubyte *)&gpGames[index])
            {
                gpCurrentSelected = index;
                break;
            }
        }
    }
}

static char overwritefilename[PATH_Max + 1] = "";
static bool32 overwriteRecGame = FALSE;

static void gpInitSavedGamePaths(void)
{
    char savedGamesRoot[PATH_Max + 1];

    strcpy(savedGamesRoot, utyGetSavedGamesPath());

    strcpy(SinglePlayerSavedGamesPath, savedGamesRoot);
    strcat(SinglePlayerSavedGamesPath, "SinglePlayer\\");
    utyEnsureDirectoryTree(SinglePlayerSavedGamesPath);

    strcpy(MultiPlayerSavedGamesPath, savedGamesRoot);
    strcat(MultiPlayerSavedGamesPath, "MultiPlayer\\");
    utyEnsureDirectoryTree(MultiPlayerSavedGamesPath);

    strcpy(RecordedGamesPath, savedGamesRoot);
    strcat(RecordedGamesPath, "RecordedGames\\");
    utyEnsureDirectoryTree(RecordedGamesPath);

    strcpy(TutorialSavedGamesPath, savedGamesRoot);
    strcat(TutorialSavedGamesPath, "Training\\");
    utyEnsureDirectoryTree(TutorialSavedGamesPath);
}

void gpOverwriteYes(char *name, featom *atom)
{
    dbgAssert(overwritefilename[0] != 0);

    if (overwriteRecGame)
    {
        overwriteRecGame = FALSE;

        recPackInGameStartCB(overwritefilename);
        feAllScreensDelete();
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
    }
    else
    {
        if (SaveGame(overwritefilename))
            clCommandMessage(strGetString(strSavedGame), -1);
        else
            clCommandMessage(strGetString(strPatchUnableWriteFile));
        feAllScreensDelete();
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
    }
}

void gpOverwriteNo(char *name, featom *atom)
{
    feScreenDisappear(NULL, NULL);
}

void gpSaveGame(char *name, featom *atom)
{
    char filename[PATH_Max + 1];

    if ((gpTextEntryName != NULL) && (strlen(gpTextEntryName) > 0))
    {
        if (strlen(gpTextEntryName) < 2)
        {
            GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
            return;
        }

        strcpy(filename,SavedGamesPath);
        strcat(filename,gpTextEntryName);

        if (fileExists(filename,0))
        {
            overwriteRecGame = FALSE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        if (!SaveGame(filename))
        {
            GeneralMessageBox(strGetString(strPatchUnableWriteFile),NULL);
            return;
        }

        clCommandMessage(strGetString(strSavedGame), -1);
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);
        dbgAssert(gpCurrentSelected < gpNumberGames);
        strcpy(filename,SavedGamesPath);
        strcat(filename,gpGames[gpCurrentSelected].title);

        if (fileExists(filename,0))
        {
            overwriteRecGame = FALSE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        if (!SaveGame(filename))
        {
            GeneralMessageBox(strGetString(strPatchUnableWriteFile),NULL);
            return;
        }

        clCommandMessage(strGetString(strSavedGame), -1);
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
}

void gpLoadTheRecordedGame(char *name, featom *atom)
{
    sdword verifysavename;

    if (!gpGetGameName(name,atom,recordPacketSaveFileName))
    {
        return;
    }

    strcpy(recordPacketFileName,recordPacketSaveFileName);
    strcat(recordPacketFileName,PKTS_EXTENSION);

    if ((verifysavename = VerifySaveFile(recordPacketSaveFileName)) != VERIFYSAVEFILE_OK)
    {
        if (verifysavename == VERIFYSAVEFILE_BADVERSION)
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
        else
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
        return;
    }

    feScreenDisappear(NULL, NULL);

    recPackPlayInGameInit();

    utyLoadMultiPlayerGameGivenFilename(recordPacketSaveFileName);
    recordingActive = TRUE;
    recordSaveGameInProgress = TRUE;
    multiPlayerRecording = gpRecordedGameIsMultiPlayerRecording();
}

void gpSaveTheRecordedGame(char *name, featom *atom)
{
    char filename[PATH_Max + 1];

    if ((gpTextEntryName != NULL) && (strlen(gpTextEntryName) > 0))
    {
        if (strlen(gpTextEntryName) < 2)
        {
            GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
            return;
        }

        strcpy(filename,SavedGamesPath);
        strcat(filename,gpTextEntryName);

        if (fileExists(filename,0))
        {
            overwriteRecGame = TRUE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        recPackInGameStartCB(filename);
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);
        dbgAssert(gpCurrentSelected < gpNumberGames);
        strcpy(filename,SavedGamesPath);
        strcat(filename,gpGames[gpCurrentSelected].title);

        if (fileExists(filename,0))
        {
            overwriteRecGame = TRUE;
            strcpy(overwritefilename,filename);
            feScreenStart(ghMainRegion, "OverwriteGamePopup");
            return;
        }

        recPackInGameStartCB(filename);
        feScreenDisappear(NULL, NULL);
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        return;
    }

    GeneralMessageBox(strGetString(strErrorInSaveGameName),strGetString(strAtLeast2Chars));
}

void gpStopPlayback(char *name, featom *atom)
{
    (void)name;
    (void)atom;
}

void gpStopRecording(char *name, featom *atom)
{
    recPackInGameStopCB();
    feScreenDisappear(NULL, NULL);
    if (!multiPlayerGame)
    {
        universePause = FALSE;      // unpause game
    }
}

void gpSaveGivenGame(char* gamename)
{
    if (SaveGame(gamename))
        clCommandMessage(strGetString(strQuickSave), -1);
    else
        clCommandMessage(strGetString(strPatchUnableWriteFile), 0xFF0000FF);

    if (!multiPlayerGame)
    {
        universePause = FALSE;
    }
}

void gpLoadGivenGame(char* gamename)
{
    sdword verifysavename;

    if (!fileExists(gamename, 0))
    {
        return;
    }

    if ((verifysavename = VerifySaveFile(gamename)) != VERIFYSAVEFILE_OK)
    {
        if (verifysavename == VERIFYSAVEFILE_BADVERSION)
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
        else
            GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
        return;
    }

    gameEnd();

    if (gpLoadSinglePlayerGame)
    {
        if (gpLoadTutorial)
        {
            tutorial = 1;
        }
        else
        {
            tutorial = 2;
        }
        utyLoadSinglePlayerGameGivenFilename(gamename);
    }
    else
    {
        utyLoadMultiPlayerGameGivenFilename(gamename);
    }
    if (!multiPlayerGame)
    {
        universePause = FALSE;
    }
}

void spRestartLevel(char *name, featom *atom)
{
    if (strlen(CurrentLevelName) > 0)
    {
        gpLoadSinglePlayerGame = TRUE;
        gpLoadTutorial = FALSE;
        gpLoadGivenGame(CurrentLevelName);
    }
}

void gpLoadGame(char *name, featom *atom)
{
    char filename[PATH_Max + 1];
    sdword verifysavename;

    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);

        dbgAssert(gpCurrentSelected < gpNumberGames);
        if (gpLoadTutorial)
        {
            strcpy(filename,TutorialSavedGamesPath);
        }
        else
        {
            strcpy(filename,SavedGamesPath);
        }
        strcat(filename,gpGames[gpCurrentSelected].title);
        if ((verifysavename = VerifySaveFile(filename)) != VERIFYSAVEFILE_OK)
        {
            if (verifysavename == VERIFYSAVEFILE_BADVERSION)
                GeneralMessageBox(strGetString(strErrorInvalidSaveGameFileVersion),NULL);
            else
                GeneralMessageBox(strGetString(strErrorInvalidSaveGameFile),NULL);
            return;
        }

        feScreenDisappear(NULL, NULL);

        soundEventStopTrack(SOUND_EVENT_DEFAULT, 1.0f);
        soundEventPause(TRUE);
        gameEnd();
        if (gpLoadSinglePlayerGame)
        {
            if (gpLoadTutorial)
            {
                tutorial = 1;
            }
            else
            {
                //for single player games, tutorial == 2 to use certain tutorial functions
                tutorial = 2;
            }
            utyLoadSinglePlayerGameGivenFilename(filename);
        }
        else
        {
            utyLoadMultiPlayerGameGivenFilename(filename);
        }
        if (!multiPlayerGame)
        {
            universePause = FALSE;      // unpause game
        }
        soundEventPause(FALSE);
    }
}

static bool32 gpRecordedGameIsMultiPlayerRecording(void)
{
    sdword i;
    char defaultName[MAX_PERSONAL_NAME_LEN];

    for (i = 1; i < universe.numPlayers; i++)
    {
        sprintf(defaultName, "%s %i", strGetString(strComputerName), i);
        if (strcmp(playerNames[i], defaultName))
        {
            return TRUE;
        }
    }

    return FALSE;
}


void gpDeleteGame(char *name, featom *atom)
{
    char filename[PATH_Max + 1];
    sdword i;
    sdword index;
    fonthandle  oldfont;

    if (!gpGetGameName(name,atom,filename))
    {
        return;
    }

    // gpCurrentSelected will be set by gpGetGameName

    feScreenDisappear(NULL, NULL);

    fileDelete(filename);
    if (SavedGamesPath == RecordedGamesPath)
    {
        char tmpfile[PATH_Max + 1];
        strcpy(tmpfile,filename);
        strcat(tmpfile,PKTS_EXTENSION);
        fileDelete(tmpfile);
    }

    for (i=gpCurrentSelected;i<gpNumberGames-1;i++)
    {
        gpGames[i] = gpGames[i+1];
    }
    gpNumberGames--;
    if (gpNumberGames == 0)
    {
        gpCurrentSelected = 0;
    }
    else if (gpCurrentSelected >= gpNumberGames)
    {
        gpCurrentSelected--;
        dbgAssert(gpCurrentSelected >= 0);
        dbgAssert(gpCurrentSelected < gpNumberGames);
    }

    dbgAssert(gpNumberGames >= 0);

    oldfont = fontMakeCurrent(gpListFont);      // fontHeight called later on in this function

    uicListCleanUp(gpGameListWindow);

    uicListWindowInit(gpGameListWindow,
                      NULL,                             // title draw, no title
                      NULL,                             // title click process, no title
                      0,                                // title height, no title
                      gpGameItemDraw,                   // item draw function
                      fontHeight(" ")+GP_VertSpacing,   // item height
                      UICLW_CanSelect);

    for (index = 0; index < gpNumberGames; index++)
    {
        if (index==gpCurrentSelected)
            uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect|UICLI_Selected, UICLW_AddToTail);
        else
            uicListAddItem(gpGameListWindow, (ubyte *)&gpGames[index], UICLI_CanSelect, UICLW_AddToTail);
    }

    fontMakeCurrent(oldfont);
}

bool32 gpGetGameName(char *name, featom *atom, char *filename)
{
    if (gpNumberGames > 0)
    {
        gpDonePicking(name, atom);
        strcpy(filename,SavedGamesPath);
        strcat(filename,gpGames[gpCurrentSelected].title);
        return TRUE;
    }
    return FALSE;
}

void gpBackPicking(char *name, featom *atom)
{
    feScreenDisappear(NULL, NULL);
}

#if 0
void gpAcceptBackToInGameEscapeMenu(char* name, featom* atom)
{
    void opInGameOptionsAccept(char* name, featom* atom);

    opInGameOptionsAccept(name, atom);

    feScreenStart(ghMainRegion, "In_game_esc_menu");
}
#endif

void gpBackToInGameEscapeMenu(char *name, featom *atom)
{
    feScreenDisappear(NULL, NULL);

#if defined(Downloadable)
    if ((singlePlayerGame) && (!(utyCreditsSequence|utyPlugScreens)) && (universe.quittime > 0.0f) && (universe.totaltimeelapsed >= universe.quittime))
#else
    if ((singlePlayerGame) && (!(utyCreditsSequence)) && (universe.quittime > 0.0f) && (universe.realtimeelapsed >= universe.quittime))
#endif
    {
        feScreenStart(ghMainRegion, "SP_Game_Over");
        return;
    }

    switch (escapemenu)
    {
        case ESCAPE_DEFAULT:
            feScreenStart(ghMainRegion, "In_game_esc_menu");
            break;

        case ESCAPE_SINGLE:
            feScreenStart(ghMainRegion, "In_game_esc_menu2");
            break;

        case ESCAPE_RECORDING:
            feScreenStart(ghMainRegion, "In_game_esc_menu_recording");
            break;

        case ESCAPE_PLAYING:
            feScreenStart(ghMainRegion, "In_game_esc_menu_playing");
            break;

        case ESCAPE_TUTORIAL:
            feScreenStart(ghMainRegion, "In_game_esc_menu_tutorial");
            break;

        case ESCAPE_DEFAULT_NOSAVE:
            feScreenStart(ghMainRegion, "In_game_esc_menu_nosave");
            break;

        case ESCAPE_RECORDING_NOSAVE:
            feScreenStart(ghMainRegion, "In_game_esc_menu_nosave_recording");
            break;
    }
}

void gpStartInGameEscapeMenu(void)
{
    if (tutorial==TUTORIAL_ONLY)
    {
        escapemenu = ESCAPE_TUTORIAL;
    }
    else if (singlePlayerGame)
    {
        escapemenu = ESCAPE_SINGLE;
    }
    else if (playPackets)
    {
        escapemenu = ESCAPE_PLAYING;
    }
    else if (recordPackets)
    {
        if (debugPacketRecord)
        {
            if (multiPlayerGame)
                escapemenu = ESCAPE_DEFAULT_NOSAVE;
            else
                escapemenu = ESCAPE_DEFAULT;
        }
        else
        {
            if (multiPlayerGame)
                escapemenu = ESCAPE_RECORDING_NOSAVE;
            else
                escapemenu = ESCAPE_RECORDING;
        }
    }
    else
    {
        if (multiPlayerGame)
            escapemenu = ESCAPE_DEFAULT_NOSAVE;
        else
            escapemenu = ESCAPE_DEFAULT;
    }

    mrReset();
    gpBackToInGameEscapeMenu(NULL, NULL);
}






real32 __cdecl utyGameStatsGetStatValue(sdword playerIndex, sdword statIndex)
{
    sdword *stats = (sdword *)gameStatsSnapshotBuffer + GAME_STATS_COUNT * playerIndex;

    switch (statIndex)
    {
        case 0:  return (real32)stats[0];
        case 1:  return (real32)stats[1];
        case 2:  return (real32)stats[2];
        case 3:  return (real32)stats[3];
        case 4:  return (real32)stats[4];
        case 5:  return (real32)stats[13];
        case 6:  return (real32)stats[9];
        case 7:  return (real32)stats[5];
        case 8:  return (real32)stats[14];
        case 9:  return (real32)stats[10];
        case 10: return (real32)stats[6];
        case 11: return (real32)stats[15];
        case 12: return (real32)stats[11];
        case 13: return (real32)stats[7];
        case 14: return (real32)stats[16];
        case 15: return (real32)stats[12];
        case 16: return (real32)stats[8];
        default: return 0.0f;
    }
}


void __cdecl utyGameStatsFormatStatValue(sdword playerIndex, sdword statIndex, char *dest)
{
    sdword *stats = (sdword *)gameStatsSnapshotBuffer + GAME_STATS_COUNT * playerIndex;

    switch (statIndex)
    {
        case 0:
            sprintf(dest, "%d:%02d:%02d", stats[0] / 60 / 60, stats[0] / 60 % 60, stats[0] % 60);
            break;
        case 1:  sprintf(dest, "%d", stats[1]); break;
        case 2:  sprintf(dest, "%d", stats[2]); break;
        case 3:  sprintf(dest, "%d", stats[3]); break;
        case 4:  sprintf(dest, "%d", stats[4]); break;
        case 5:  sprintf(dest, "%d", stats[13]); break;
        case 6:  sprintf(dest, "%d", stats[9]); break;
        case 7:  sprintf(dest, "%d", stats[5]); break;
        case 8:  sprintf(dest, "%d", stats[14]); break;
        case 9:  sprintf(dest, "%d", stats[10]); break;
        case 10: sprintf(dest, "%d", stats[6]); break;
        case 11: sprintf(dest, "%d", stats[15]); break;
        case 12: sprintf(dest, "%d", stats[11]); break;
        case 13: sprintf(dest, "%d", stats[7]); break;
        case 14: sprintf(dest, "%d", stats[16]); break;
        case 15: sprintf(dest, "%d", stats[12]); break;
        case 16: sprintf(dest, "%d", stats[8]); break;
        default: break;
    }
}


void __cdecl utyGameStatsCapturePlayerStats(sdword playerIndex, sdword *statsBuffer)
{
    Player *player = &universe.players[playerIndex];
    PlayerStats *playerStats = &universe.gameStats.playerStats[playerIndex];
    sdword survivalTime = (sdword)playerStats->timeOfDeath;

    statsBuffer[0] = survivalTime;
    if (survivalTime == 0)
    {
        statsBuffer[0] = (sdword)universe.totaltimeelapsed;
    }

    statsBuffer[1] = playerStats->totalPlayersKilled;
    statsBuffer[2] = playerStats->totalResourceUnitsCollected;
    statsBuffer[3] = playerStats->totalCrystalsRetrieved;
    statsBuffer[4] = utyGameStatsCountResearchedTechs(player);

    statsBuffer[13] = playerStats->totalKillsForEachShip[3];
    statsBuffer[13] += playerStats->totalKillsForEachShip[2];
    statsBuffer[13] += playerStats->totalKillsForEachShip[8];

    statsBuffer[5] = playerStats->totalLossesForEachShip[2];
    statsBuffer[5] += playerStats->totalLossesForEachShip[1];
    statsBuffer[5] += playerStats->totalLossesForEachShip[7];

    statsBuffer[9] = (sdword)playerStats->pointsOfDamageDoneToWho[3];
    statsBuffer[9] += (sdword)playerStats->pointsOfDamageDoneToWho[2];
    statsBuffer[9] += playerStats->totalPlayersKilled;

    statsBuffer[14] = playerStats->totalKillsForEachShip[1];
    statsBuffer[6] = playerStats->totalLossesForEachShip[0];
    statsBuffer[10] = (sdword)playerStats->pointsOfDamageDoneToWho[1];

    statsBuffer[15] = playerStats->totalKillsForEachClass[12];
    statsBuffer[15] += playerStats->totalRUsLost;

    statsBuffer[7] = playerStats->totalKillsForEachClass[0];
    statsBuffer[7] += playerStats->totalLosses;

    statsBuffer[11] = playerStats->totalLossesForEachClass[11];
    statsBuffer[11] += (sdword)playerStats->pointsOfDamageDoneToWho[0];

    statsBuffer[16] = playerStats->totalRUsKilled;
    statsBuffer[16] += playerStats->totalKillsForEachShip[4];
    statsBuffer[16] += playerStats->totalKillsForEachShip[5];
    statsBuffer[16] += playerStats->totalKillsForEachShip[7];

    statsBuffer[8] = playerStats->totalLossesForEachClass[0];
    statsBuffer[8] += playerStats->totalLossesForEachShip[3];
    statsBuffer[8] += playerStats->totalLossesForEachShip[4];
    statsBuffer[8] += playerStats->totalLossesForEachShip[6];

    statsBuffer[12] = playerStats->totalLossesForEachClass[12];
    statsBuffer[12] += (sdword)playerStats->pointsOfDamageDoneToWho[4];
    statsBuffer[12] += (sdword)playerStats->pointsOfDamageDoneToWho[5];
    statsBuffer[12] += (sdword)playerStats->pointsOfDamageDoneToWho[7];
}


void utyGameStatsInitLayout(void)
{
    sdword i;
    sdword labelMaxWidth = 0;
    sdword currentFontHeight;
    sdword titleBlockHeight;
    sdword originalPanelLeft;
    sdword originalPanelTop;
    sdword columnWidth;
    sdword rowY;
    sdword playerAreaBottom;
    sdword valueColumnTop;
    sdword valueColumnBottom;
    sdword valueColumnLeft;

    fontMakeCurrent(gameStatsOverlayLabelFont);
    currentFontHeight = fontHeight(" ");
    titleBlockHeight = 7 * GAME_STATS_ROW_SPACING + 8 * currentFontHeight;

    for (i = 0; i < GAME_STATS_COUNT; i++)
    {
        sdword width = fontWidth(strGetString(gameStatsLabelStringIds[i]));
        if (width > labelMaxWidth)
        {
            labelMaxWidth = width;
        }
    }

    originalPanelLeft = (MAIN_WindowWidth - GAME_STATS_PANEL_WIDTH) / 2;
    originalPanelTop = (MAIN_WindowHeight - GAME_STATS_PANEL_HEIGHT) / 2;
    columnWidth = (GAME_STATS_PANEL_WIDTH - 2 * (currentFontHeight + GAME_STATS_VALUE_INSET) - GAME_STATS_LABEL_EXTRA_GAP -
                   GAME_STATS_LABEL_LEFT_MARGIN - labelMaxWidth) / 8;

    gameStatsExitRect.x0 = (MAIN_WindowWidth - GAME_STATS_EXIT_WIDTH) / 2;
    gameStatsExitRect.x1 = gameStatsExitRect.x0 + GAME_STATS_EXIT_WIDTH;
    gameStatsExitRect.y0 = MAIN_WindowHeight - GAME_STATS_EXIT_HEIGHT;
    gameStatsExitRect.y1 = gameStatsExitRect.y0 + GAME_STATS_EXIT_TOP_OFFSET;

    rowY = originalPanelTop + GAME_STATS_PLAYER_Y_OFFSET + currentFontHeight * (8 - universe.numPlayers);
    for (i = 0; i < universe.numPlayers; i++)
    {
        gameStatsPlayerColumnX[i] = originalPanelLeft + GAME_STATS_PLAYER_X_OFFSET;
        gameStatsPlayerRowY[i] = rowY;
        rowY += currentFontHeight + GAME_STATS_ROW_SPACING;
    }

    for (i = 0; i < GAME_STATS_COUNT; i++)
    {
        sdword labelWidth = fontWidth(strGetString(gameStatsLabelStringIds[i]));

        gameStatsLabelX[i] = labelMaxWidth + originalPanelLeft + GAME_STATS_LABEL_LEFT_MARGIN - labelWidth;
        if (i == 0)
        {
            gameStatsLabelY[i] = currentFontHeight + titleBlockHeight + originalPanelTop + GAME_STATS_PLAYER_Y_OFFSET +
                                 GAME_STATS_ROW_GROUP_GAP + GAME_STATS_PLAYER_BAR_GAP;
        }
        else
        {
            gameStatsLabelY[i] = gameStatsLabelY[i - 1] + currentFontHeight;
            if (i >= 5 && i <= 14 && gameStatsHalfStepRows[i - 5] == 0)
            {
                gameStatsLabelY[i] += currentFontHeight / 2;
            }
        }

        gameStatsHitLeft[i] = gameStatsLabelX[i] + GAME_STATS_VALUE_INSET + labelWidth;
        gameStatsIconSplit0[i] = gameStatsHitLeft[i] + currentFontHeight;
        gameStatsHitTop[i] = gameStatsLabelY[i];
        gameStatsHitBottom[i] = gameStatsLabelY[i] + currentFontHeight;
        gameStatsIconSplit1[i] = gameStatsIconSplit0[i] + columnWidth * universe.numPlayers + GAME_STATS_VALUE_INSET;
        gameStatsHitRight[i] = gameStatsIconSplit1[i] + currentFontHeight;
    }

    valueColumnTop = currentFontHeight + titleBlockHeight + originalPanelTop + GAME_STATS_PLAYER_Y_OFFSET +
                     GAME_STATS_PLAYER_BAR_GAP + GAME_STATS_ROW_GROUP_GAP;
    playerAreaBottom = gameStatsHitBottom[GAME_STATS_COUNT - 1];
    valueColumnBottom = playerAreaBottom;
    valueColumnLeft = gameStatsIconSplit0[0];

    for (i = 0; i < MAX_MULTIPLAYER_PLAYERS; i++)
    {
        gameStatsBarLeft[i] = valueColumnLeft;
        gameStatsBarTop[i] = valueColumnTop;
        gameStatsBarRight[i] = gameStatsBarLeft[i] + columnWidth;
        gameStatsBarBottom[i] = valueColumnBottom;
        gameStatsRankIconLeft[i] = (columnWidth - GAME_STATS_PLAYER_BAR_GAP) / 2 + gameStatsBarLeft[i];
        gameStatsRankIconTop[i] = gameStatsBarTop[i] - GAME_STATS_PLAYER_BAR_GAP - currentFontHeight;
        gameStatsRankIconRight[i] = gameStatsRankIconLeft[i] + GAME_STATS_PLAYER_BAR_GAP;
        gameStatsRankIconBottom[i] = gameStatsRankIconTop[i] + GAME_STATS_PLAYER_BAR_GAP;
        valueColumnLeft += columnWidth;
    }

    gameStatsPanelLeft = originalPanelLeft - 10;
    gameStatsPanelTop = (universe.numPlayers > 0) ? gameStatsPlayerRowY[0] - 10 : originalPanelTop;
    gameStatsPanelRight = originalPanelLeft + GAME_STATS_PANEL_WIDTH + 10;
    gameStatsPanelBottom = valueColumnBottom + 2 * currentFontHeight;
    gameStatsGraphLeft = (universe.numPlayers > 0) ? gameStatsPlayerColumnX[0] : originalPanelLeft + GAME_STATS_PLAYER_X_OFFSET;
    gameStatsGraphTop = (universe.numPlayers > 0) ? gameStatsPlayerRowY[0] : valueColumnTop;
    gameStatsGraphRight = gameStatsBarRight[MAX_MULTIPLAYER_PLAYERS - 1];
    gameStatsGraphBottom = (universe.numPlayers > 0) ? gameStatsPlayerRowY[universe.numPlayers - 1] + currentFontHeight : valueColumnTop + currentFontHeight;

}
void *gameStatsOverlayStartup(void)
{
    gameStatsOverlayLabelFont = frFontRegister("default.hff");
    gameStatsOverlayValueFont = frFontRegister("default.hff");
    gameStatsOverlayHeaderFont = frFontRegister("Arial_b17.hff");
    return TimerInitTaskTimerStruct(&gameStatsOverlayTimer, 2.0f);
}

void gameStatsOverlayResetTimer(void)
{
    TimerInitTaskTimerStruct(&gameStatsOverlayTimer, 2.0f);
}

real32 gameStatsOverlayTimeRemaining(void)
{
    return TimerTimeRemaining(&gameStatsOverlayTimer);
}

void gameStatsQuitToMain(void)
{
    universePause = FALSE;
    mrRenderMainScreen = TRUE;
    if (gameStatsSnapshotBuffer != NULL)
    {
        memFree((ubyte *)gameStatsSnapshotBuffer);
        gameStatsSnapshotBuffer = NULL;
    }
    mrEnable();
    gameStatsOverlayEnabled = 0;
    hrShutdownBackground();
    utyGameQuitToMain_0();
}
