/*=============================================================================
    Name    : InfoOverlay.c
    Purpose : This file contains the logic for the information overlay.

    Created 6/8/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "infooverlay.h"
#include "key.h"
#include "options.h"
#include "fontreg.h"
#include "utility.h"
#include "select.h"
#include "region.h"
#include "task.h"
#include "statscript.h"
#include "Objectives.h"
#include "consMgr.h"
#include "BattleChat.h"
#include "mouse.h"
#include "Globals.h"
#include "researchapi.h"
#include "strings.h"
#include "sensors.h"
#include "singleplayer.h"
#include "SoundEvent.h"
#include "Teams.h"
#include "Alliance.h"
#include "Bounties.h"
#include "dock.h"
#include "MultiplayerGame.h"
#include "tutor.h"
#include "Ship.h"
#include "universe.h"
#include "SaveGame.h"
#include "glinc.h"
#include "glcompat.h"
#include "main.h"
#include "mainrgn.h"
#include "KeyBindings.h"
#include "NIS.h"

#include <stdio.h>
#include <string.h>

extern sdword mrRightMouseCameraMode;
extern sdword mouseCursorOverShipType;
extern ubyte byte_A2DCD4;
extern sdword tutNextVisible;
extern sdword tutPrevButtonVisible;
extern sdword tutQuitVisible;
int lgUpdateChatRegions(void);
void tutHideQuitButton(void);

/*=============================================================================
    Defines:
=============================================================================*/

#define numOverlays             (TOTAL_NUM_SHIPS)
#define numButtons              TOTAL_NUM_SHIPS
#define IO_VertSpacing          (fontHeight(" ") >> 1)
#define IO_HorzSpacing          (fontWidth(" "))
#define IO_ListWidth            150

#define IO_DefaultFont          "HW_EuroseCond_11.hff"
#define IO_BoundedLabelPadding  5
#define IO_MaxObjectiveEntries  64
#define IO_ProgressPanelX       10
#define IO_ProgressPanelTopInset 15
#define IO_ProgressPanelGap     17
#define IO_ProgressPanelArrowX  30
#define IO_ProgressPanelArrowWidth 25
#define IO_ProgressPanelBarWidth 150
#define IO_ProgressPanelArrowPad 5
#define IO_ProgressPanelBottomPadding 10
#define IO_ProgressPanelBackdropColor 0x64323232
#define IO_ProgressPanelFillColor 0x4B000000
#define IO_ObjectivesX          10
#define IO_ObjectivesPadding    5
#define IO_ObjectivesGap        4
#define IO_ObjectivesBackdropColor colRGBA(50, 50, 50, 100)
#define IO_ObjectivesHeaderColor colRGB(255, 255, 255)
#define IO_ObjectivesInactiveColor colRGB(255, 0, 0)
#define IO_ObjectivesCompleteColor colRGB(0, 255, 0)
#define IO_ObjectivesSelectedColor colRGB(255, 255, 255)
#define IO_ObjectivesBackdropPadTop 5
#define IO_ObjectivesBackdropPadRight 5
#define IO_EventFeedTextX 17
#define IO_EventFeedArrowLeft 3
#define IO_EventFeedArrowSize 17
#define IO_EventFeedArrowHeight (IO_EventFeedArrowSize - 3)
#define IO_EventFeedBottomPadding 10
#define IO_EventFeedBottomArrowYOffset 24.0f
#define IO_EventFeedBottomArrowScale 1.5f
#define IO_EventFeedNewestColor 0xFF00FF00
#define IO_EventFeedFadeToColor 0x00000000
#define IO_EventFeedFlashDelay 8.0f
#define IO_EventFeedFlashDuration 5.0f
#define IO_TooltipCursorXOffset  (-5)
#define IO_TooltipCursorYOffset  30
#define IO_TooltipTextInset      5
#define IO_TooltipTextColorResource 0xFF3AC1E9
#define IO_TooltipBackdropColorResource 0x645FC855
#define IO_TooltipOutlineColorResource 0xFFA0A0A0
#define IO_TooltipTextColorMissile 0xFF0000FF
#define IO_TooltipBackdropColorMissile 0x64646464
#define IO_TooltipOutlineColorMissile 0xFF0000FF
#define IO_TooltipTextColorModule 0xFF0000FF
#define IO_TooltipBackdropColorModule 0x644B6496
#define IO_TooltipOutlineColorModule 0xFFA0A0A0
#define IO_TooltipTextColorForeign 0xFFFFC000
#define IO_TooltipBackdropColorForeign 0x64000000
#define IO_TooltipOutlineColorForeign 0xFFA0A0A0
#define IO_TooltipTextColorFriendly 0xFF00FF00
#define IO_TooltipBackdropColorFriendly 0x644B644B
#define IO_TooltipOutlineColorFriendly 0xFFA0A0A0
#define IO_TooltipOutlineThickness 2.0f
#define IO_TooltipOutlineCorner 2
#define IO_BuildProgressTextColor 0xFF25AF5B
#define IO_ResearchProgressTextColor 0xFFBFAF6A
#define IO_ProgressArrowFillColor 0xFF00FF00
#define IO_ProgressArrowOutlineColor 0xFFFFFFFF
#define IO_PlayerStatusRightPadding 10
#define IO_PlayerStatusBackdropPadX 5
#define IO_PlayerStatusBackdropPadY 5
#define IO_PlayerStatusTimeGap 10
#define IO_PlayerStatusBackdropColor 0x64323232
#define IO_PlayerStatusLabelColor 0xFFC8C8C8
#define IO_EventFeedMaxMessages 25
#define IO_EventFeedVisibleRows 5
#define IO_EventFeedTextLength  512
#define IO_BandboxPreviewGap 10
#define IO_BandboxPreviewFriendlyColor 0xFF00FF00
#define IO_BandboxPreviewAlliedColor 0xFFFFC000
#define IO_BandboxPreviewEnemyColor 0xFF0000FF
#define IO_BandboxPreviewBackdropColor 0x64323232

typedef struct InfoOverlayProgressPanelState InfoOverlayProgressPanelState;
typedef struct InfoOverlayResearchProgressEntry InfoOverlayResearchProgressEntry;
typedef struct InfoOverlayShipCountEntry InfoOverlayShipCountEntry;

void ioDrawObjectivesList(void);
udword ioDrawShipCountLists(void);
static udword ioDrawBandboxShipCountPreview(void);
void ioDrawMouseoverObjectTooltip(void);
void ioDrawEventFeed(void);
sdword __cdecl ioDrawPlayerResourceStats(int y);
int __cdecl ioDrawPathMarkerLegend(int y);
sdword __cdecl ioDrawBuildInProgressPanel(sdword topY, sdword bottomY);
sdword __cdecl ioDrawResearchProgressPanel(sdword topY, sdword bottomY);
static void ioResetProgressPanelState(struct InfoOverlayProgressPanelState *state);
static void ioUnionRect(rectangle *dest, rectangle *src, bool32 *hasRect);
static void ioScrollArrowTrianglePointsGet(sdword direction, rectangle *rect, triangle *tri);
static sdword ioBuildCurrentResearchProgressList(Player *player, InfoOverlayResearchProgressEntry *outList);
static const char *ioShipCountMissionOverrideName(sdword shiptype);
static void ioFormatShipCountLabel(char *buffer, sdword count, sdword shiptype, bool32 applyMissionOverride);
udword ioDrawOverlaySectionStatus(void);
void ioPrintBoundedLabel(rectangle *bounds, sdword x, sdword y, color c, char *string);
static bool32 ioPointInRect(sdword x, sdword y, rectangle *rect);
static void ioShipCountEntryReset(InfoOverlayShipCountEntry *entries);
static void ioShipCountEntryAccumulate(InfoOverlayShipCountEntry *entries, sdword shiptype);
static void ioDrawShipCountEntryRow(InfoOverlayShipCountEntry *entry, sdword shiptype, sdword x, sdword *y, sdword rowHeight,
                                    bool32 showBackdrop, bool32 applyMissionOverride, color defaultColor, rectangle *backdropRect);

/*=============================================================================
    Data:
=============================================================================*/


fonthandle   ioShipListFont=0;
char         ioShipListFontName[64] = IO_DefaultFont;

ShipListInfo    listinfo[numOverlays + 1]=
{
    {0      , 0     , sSentinel             , NULL },
    {0      , 0     , kHeavyDefender        , NULL },
    {0      , 0     , sRecon                , NULL },
    {0      , 0     , kHeavyInterceptor     , NULL },
    {0      , 0     , bCloakedFighter       , NULL },
    {0      , 0     , sSentinel             , NULL },
    {0      , 0     , kAttackBomber         , NULL },
    {0      , 0     , sACV                  , NULL },
    {0      , 0     , kHeavyCorvette        , NULL },
    {0      , 0     , sRepairDroid          , NULL },
    {0      , 0     , kSalCapCorvette       , NULL },
    {0      , 0     , tMinelayerCorvette    , NULL },
    {0      , 0     , kMultiGunCorvette     , NULL },
    {0      , 0     , sWorker               , NULL },
    {0      , 0     , sProcessor            , NULL },
    {0      , 0     , tStandardFrigate      , NULL },
    {0      , 0     , kAdvanceSupportFrigate, NULL },
    {0      , 0     , tDFGFrigate           , NULL },
    {0      , 0     , tDFGFrigate           , NULL },
    {0      , 0     , kIonCannonFrigate     , NULL },
    {0      , 0     , kStandardDestroyer    , NULL },
    {0      , 0     , tMissileDestroyer     , NULL },
    {0      , 0     , kHeavyCruiser         , NULL },
    {0      , 0     , kCarrier              , NULL },
    {0      , 0     , kProximitySensor      , NULL },
    {0      , 0     , kProximitySensor      , NULL },
    {0      , 0     , kProximitySensor      , NULL },
    {0      , 0     , sSentinel             , NULL },
    {0      , 0     , kGravWellGenerator    , NULL },
    {0      , 0     , kCloakGenerator       , NULL },
    {0      , 0     , sMothership           , NULL },   //mothership
    {0      , 0     , DefaultShip           , NULL },   //target drone
    {0      , 0     , DefaultShip           , NULL },   //drone
    {0      , 0     , DefaultShip           , NULL },    //headshot asteroid

    {0      , 0     , CryoTray           , NULL },    //headshot asteroid
    {0      , 0     , P1Fighter           , NULL },    //headshot asteroid
    {0      , 0     , P1IonArrayFrigate           , NULL },    //headshot asteroid
    {0      , 0     , P1MissileCorvette           , NULL },    //headshot asteroid
    {0      , 0     , P1Mothership           , NULL },    //headshot asteroid
    {0      , 0     , P1StandardCorvette           , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , DefaultShip          , NULL },    //headshot asteroid
    {0      , 0     , FloatingCity           , NULL },    //headshot asteroid
    {0      , 0     , CargoBarge           , NULL },    //headshot asteroid
    {0      , 0     , MiningBase           , NULL },    //headshot asteroid
    {0      , 0     , ResearchStation           , NULL },    //headshot asteroid
    {0      , 0     , JunkYardDawg           , NULL },    //headshot asteroid
    {0      , 0     , JunkYardHQ           , NULL },    //headshot asteroid
    {0      , 0     , Ghostship           , NULL },    //headshot asteroid
    {0      , 0     , Junk_LGun           , NULL },    //headshot asteroid
    {0      , 0     , Junk_SGun           , NULL },    //headshot asteroid
    {0      , 0     , ResearchStationBridge           , NULL },    //headshot asteroid
    {-1      , 0     , ResearchStationTower           , NULL }    //headshot asteroid
};

ShipOverlayInfo overlayinfo[numButtons];

sdword          lookupforlist[TOTAL_NUM_SHIPS + 1];

color ioListTextColor     = IO_ListTextColor;
color ioSelectedTextColor = IO_SelectedTextColor;

static real32     ioOverlayElapsedTime = 0.0f;
static udword     ioPendingOverlayClick = 0;
sdword            ioSavedNextVisible = FALSE;
sdword            ioSavedPrevButtonVisible = FALSE;
sdword            ioSavedQuitVisible = FALSE;

typedef struct
{
    bool32      enabled;
    bool32      selected;
    rectangle   rect;
    Objective  *objective;
} InfoOverlayObjectiveEntry;

typedef struct InfoOverlayProgressPanelState
{
    rectangle textBounds;
    rectangle panelRect;
    rectangle topArrowRect;
    rectangle bottomArrowRect;
    sdword page;
    bool32 textBoundsActive;
    bool32 panelRectActive;
    bool32 topArrowVisible;
    bool32 bottomArrowVisible;
    bool32 topArrowHighlighted;
    bool32 bottomArrowHighlighted;
} InfoOverlayProgressPanelState;

typedef struct InfoOverlayResearchProgressEntry
{
    TechnologyType tech;
    real32 totalTime;
    real32 timeLeft;
} InfoOverlayResearchProgressEntry;

typedef struct InfoOverlayShipCountEntry
{
    bool32 enabled;
    bool32 selected;
    rectangle rect;
    sdword count;
    bool32 pressed;
} InfoOverlayShipCountEntry;

static sdword                    ioObjectiveCount = 0;
static InfoOverlayObjectiveEntry ioObjectiveEntries[IO_MaxObjectiveEntries];
static sdword                    ioObjectivesPanelRight = 0;
static sdword                    ioObjectivesPointerY = 0;
static rectangle                 ioEventFeedNewerRect = {0, 0, 0, 0};
static rectangle                 ioEventFeedOlderRect = {0, 0, 0, 0};
static bool32                    ioEventFeedNewerVisible = FALSE;
static bool32                    ioEventFeedOlderVisible = FALSE;
typedef struct
{
    char   message[IO_EventFeedTextLength];
    sdword priority;
    real32 highlightStart;
    sdword sourceShip;
    color  messageColor;
} IOEventFeedMessage;

static sdword             ioEventFeedCount = 0;
static IOEventFeedMessage ioEventFeedMessages[IO_EventFeedMaxMessages];

static void ioEventFeedReset(void)
{
    ioEventFeedCount = 0;
    memset(ioEventFeedMessages, 0, sizeof(ioEventFeedMessages));
}

static void ioEventFeedDiscardOldest(void)
{
    if (ioEventFeedCount <= 0)
    {
        return;
    }

    memmove(&ioEventFeedMessages[0], &ioEventFeedMessages[1],
            sizeof(ioEventFeedMessages[0]) * (ioEventFeedCount - 1));
    ioEventFeedCount--;
    memset(&ioEventFeedMessages[ioEventFeedCount], 0, sizeof(ioEventFeedMessages[0]));
}

bool32 ioEventFeedAddMessage(char *message)
{
    IOEventFeedMessage *entry;

    dbgAssert(message != NULL);

    if (message[0] == '\0')
    {
        return FALSE;
    }

    if (ioEventFeedCount >= IO_EventFeedMaxMessages)
    {
        ioEventFeedDiscardOldest();
    }

    entry = &ioEventFeedMessages[ioEventFeedCount];
    strncpy(entry->message, message, sizeof(entry->message) - 1);
    entry->message[sizeof(entry->message) - 1] = '\0';
    entry->priority = 0;
    entry->highlightStart = 0.0f;
    entry->sourceShip = 0;
    entry->messageColor = colWhite;
    ioEventFeedCount++;
    return TRUE;
}

extern Objective **objectives;
extern sdword objectivesUsed;
extern sdword screenX, screenY;

enum
{
    IO_PageSelected = 0,
    IO_PageSelectedBackdrop,
    IO_PageSelecting,
    IO_PageSelectingBackdrop,
    IO_PageObjectives,
    IO_PageObjectivesBackdrop,
    IO_PageTooltips,
    IO_PageTooltipsBackdrop,
    IO_PageViewOptions,
    IO_PageViewOptionsBackdrop,
    IO_PagePlayerInfo,
    IO_PagePlayerInfoBackdrop,
    IO_PageGameMessage,
    IO_PageGameMessageBackdrop,
    IO_PagePlayerNames,
    IO_PagePlayerNamesBackdrop,
    IO_PageConstruction,
    IO_PageConstructionBackdrop,
    IO_PageResearch,
    IO_PageResearchBackdrop,
    IO_PageFlagCount
};

#define IO_SectionToggleCount 9

static fonthandle ioOverlayStatusFont = 0;
static fonthandle ioOverlayShipCountFont = 0;
static fonthandle ioOverlayObjectivesFont = 0;
static fonthandle ioOverlayDebugFont = 0;

static bool32 ioOverlayShowingBackdropOptions = FALSE;
static sdword ioOverlayPageIndex = 0;
static sdword ioOverlayAutoShowTargetHeight = 0;
static sdword ioOverlayAutoShowVelocity = 0;
static real32 ioOverlayAutoShowStartTime = 0.0f;
static sdword ioOverlayPanelY = 0;
static sdword ioOverlayPanelTargetY = 0;
static bool32 ioMainOverlayShowing = FALSE;

static bool32 ioSensorLeftButtonPressed = FALSE;
static bool32 ioSensorRightButtonPressed = FALSE;
static bool32 ioSensorBottomButtonPressed = FALSE;
static rectangle ioSensorLeftButtonRect = {0, 0, 0, 0};
static rectangle ioSensorRightButtonRect = {0, 0, 0, 0};
static rectangle ioSensorTopButtonRect = {0, 0, 0, 0};
static rectangle ioSensorBottomButtonRect = {0, 0, 0, 0};
static rectangle ioShipCountListRect = {0, 0, 0, 0};
static rectangle ioShipCountListBackdropRect = {0, 0, 0, 0};
static rectangle ioShipCountPreviewRect = {0, 0, 0, 0};
static rectangle ioShipCountPreviewBackdropRect = {0, 0, 0, 0};
static rectangle ioOverlaySectionStatusRect = {0, 0, 0, 0};
static rectangle ioPlayerStatusPanelRect = {0, 0, 0, 0};
static bool32 ioShipCountListRectActive = FALSE;
static bool32 ioShipCountListBackdropRectActive = FALSE;
static bool32 ioShipCountPreviewRectActive = FALSE;
static bool32 ioShipCountPreviewBackdropRectActive = FALSE;
static sdword ioShipCountListRowCount = 0;
static InfoOverlayShipCountEntry ioShipCountPrimary[TOTAL_NUM_SHIPS];
static InfoOverlayShipCountEntry ioShipCountSecondary[TOTAL_NUM_SHIPS];
static InfoOverlayShipCountEntry ioShipCountTertiary[TOTAL_NUM_SHIPS];

typedef struct
{
    sdword hover;
    sdword x0;
    sdword y0;
    sdword x1;
    sdword y1;
    sdword labelId;
    sdword commandId;
} InfoOverlayHotkeyArea;
#define IO_NUM_HOTKEY_AREAS 6
static InfoOverlayHotkeyArea ioHotkeyAreas[IO_NUM_HOTKEY_AREAS] = {
    { 0, 0, 0, 0, 0, 1430, 79 },
    { 0, 0, 0, 0, 0, 1429, 79 },
    { 0, 0, 0, 0, 0, 1428, 81 },
    { 0, 0, 0, 0, 0, 1427, 80 },
    { 0, 0, 0, 0, 0, 1426, 84 },
    { 0, 0, 0, 0, 0, 1425, 83 },
};

/* Build queue and construction panel scroll buttons */
static bool32 ioBuildScrollLeftEnabled = FALSE;
static rectangle ioBuildScrollLeftRect = {0, 0, 0, 0};
static bool32 ioBuildScrollLeftPressed = FALSE;
static sdword ioBuildScrollPage = 0;
static bool32 ioBuildScrollRightEnabled = FALSE;
static rectangle ioBuildScrollRightRect = {0, 0, 0, 0};
static bool32 ioBuildScrollRightPressed = FALSE;
static bool32 ioConstructScrollUpEnabled = FALSE;
static rectangle ioConstructScrollUpRect = {0, 0, 0, 0};
static bool32 ioConstructScrollUpPressed = FALSE;
static sdword ioConstructScrollOffset = 0;
static bool32 ioConstructScrollDownEnabled = FALSE;
static rectangle ioConstructScrollDownRect = {0, 0, 0, 0};
static bool32 ioConstructScrollDownPressed = FALSE;


/* Build queue paged scroll */
static sdword ioBuildQueuePageIndex = 0;
static sdword ioBuildQueueTotal = 0;
static sdword ioBuildQueuePageSize = 0;
static sdword ioBuildQueueScrollIndex = 0;
static bool32 ioBuildQueueUpArrowEnabled = FALSE;
static sdword ioBuildQueueUpArrowX0 = 0;
static sdword ioBuildQueueUpArrowY0 = 0;
static sdword ioBuildQueueUpArrowX1 = 0;
static sdword ioBuildQueueUpArrowY1 = 0;
static bool32 ioBuildQueueDownArrowEnabled = FALSE;
static sdword ioBuildQueueDownArrowX0 = 0;
static sdword ioBuildQueueDownArrowY0 = 0;
static sdword ioBuildQueueDownArrowX1 = 0;
static sdword ioBuildQueueDownArrowY1 = 0;

/* Objective hover mode flag */
static bool32 ioObjectiveHoverActive = FALSE;

/* Construction panel item list (548 bytes per entry, up to ioBuildQueueTotal entries) */
#define IO_CONSTRUCT_ITEM_STRIDE 137  /* 548 bytes = 137 DWORDs */
#define IO_MAX_CONSTRUCT_ITEMS   32
static sdword ioConstructItems[IO_MAX_CONSTRUCT_ITEMS][IO_CONSTRUCT_ITEM_STRIDE];

/* Build queue panel configuration (80 bytes per page, multiplayer + singleplayer) */
#define IO_MAX_BUILD_PAGES 16
#define IO_BUILD_PAGE_STRIDE 20  /* 80 bytes = 20 DWORDs */
static sdword ioBuildQueueConfig[IO_MAX_BUILD_PAGES][IO_BUILD_PAGE_STRIDE];
static sdword ioBuildQueueConfigSP[IO_MAX_BUILD_PAGES][IO_BUILD_PAGE_STRIDE];

static const char *ioShipCountMissionOverrideName(sdword shiptype)
{
    if (!singlePlayerGame)
    {
        return NULL;
    }

    if (singlePlayerGameInfo.currentMission == 2 && shiptype == kStandardDestroyer)
    {
        return "kDestroyer";
    }

    if ((singlePlayerGameInfo.currentMission == 5 || singlePlayerGameInfo.currentMission == 6) &&
        shiptype == kCarrier)
    {
        return "CaalShto";
    }

    return NULL;
}

static void ioFormatShipCountLabel(char *buffer, sdword count, sdword shiptype, bool32 applyMissionOverride)
{
    const char *overrideName = NULL;

    if (applyMissionOverride)
    {
        overrideName = ioShipCountMissionOverrideName(shiptype);
    }

    if (overrideName != NULL)
    {
        sprintf(buffer, "1 x %s", overrideName);
    }
    else
    {
        sprintf(buffer, "%i x %s", count, ShipTypeToNiceStr(shiptype));
    }
}

static void ioShipCountEntryReset(InfoOverlayShipCountEntry *entries)
{
    sdword shiptype;

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        entries[shiptype].enabled = FALSE;
        entries[shiptype].selected = FALSE;
        entries[shiptype].count = 0;
    }
}

static void ioShipCountEntryAccumulate(InfoOverlayShipCountEntry *entries, sdword shiptype)
{
    if (shiptype < 0 || shiptype >= TOTAL_NUM_SHIPS)
    {
        return;
    }

    entries[shiptype].enabled = TRUE;
    entries[shiptype].count++;
}

static void ioDrawShipCountEntryRow(InfoOverlayShipCountEntry *entry, sdword shiptype, sdword x, sdword *y, sdword rowHeight,
                                    bool32 showBackdrop, bool32 applyMissionOverride, color defaultColor, rectangle *backdropRect)
{
    color textColor;
    char string[128];

    if (!entry->enabled || entry->count <= 0)
    {
        return;
    }

    ioFormatShipCountLabel(string, entry->count, shiptype, applyMissionOverride);
    entry->rect.x0 = x;
    entry->rect.y0 = *y;
    entry->rect.x1 = x + fontWidth(string);
    entry->rect.y1 = *y + rowHeight;

    if (ioPointInRect(mouseCursorX(), mouseCursorY(), &entry->rect))
    {
        textColor = colRGB(255, 100, 100);
    }
    else if (entry->selected)
    {
        textColor = colRGB(0, 255, 0);
    }
    else
    {
        textColor = defaultColor;
    }

    if (showBackdrop)
    {
        ioPrintBoundedLabel(backdropRect, x, *y, textColor, string);
        ioShipCountListBackdropRectActive = TRUE;
    }
    else
    {
        fontPrint(x, *y, textColor, string);
    }

    ioUnionRect(&ioShipCountListRect, &entry->rect, &ioShipCountListRectActive);
    *y += rowHeight;
    ioShipCountListRowCount++;
}
static InfoOverlayProgressPanelState ioBuildProgressPanelState = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
static InfoOverlayProgressPanelState ioResearchProgressPanelState = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
static rectangle ioMainOverlayButtonRects[4] =
{
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

bool32 ioSensorLeftButtonPressedGet(void)
{
    return ioSensorLeftButtonPressed;
}

bool32 ioSensorRightButtonPressedGet(void)
{
    return ioSensorRightButtonPressed;
}

bool32 ioSensorBottomButtonPressedGet(void)
{
    return ioSensorBottomButtonPressed;
}

static sdword ioSinglePlayerPageFlags[3][IO_PageFlagCount] =
{
    {1,0, 1,0, 1,1, 1,1, 0,1, 1,1, 1,0, 0,0, 1,0, 1,0},
    {1,0, 1,0, 1,1, 1,1, 0,1, 1,1, 0,0, 0,0, 0,0, 0,0},
    {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0}
};

static sdword ioMultiPlayerPageFlags[3][IO_PageFlagCount] =
{
    {1,0, 1,0, 0,1, 1,1, 0,1, 1,1, 1,0, 1,0, 1,0, 1,0},
    {1,0, 1,0, 0,1, 1,1, 0,1, 1,1, 0,0, 1,0, 0,0, 0,0},
    {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0}
};

static sdword ioOverlayStatusFlagIndices[IO_SectionToggleCount] =
{
    IO_PageSelected,
    IO_PageSelecting,
    IO_PageObjectives,
    IO_PageTooltips,
    IO_PagePlayerInfo,
    IO_PageGameMessage,
    IO_PagePlayerNames,
    IO_PageConstruction,
    IO_PageResearch
};

static sdword ioOverlayStatusLabelStrings[IO_SectionToggleCount] =
{
    strMO_Option_ShowSelectedShips,
    strMO_Option_ShowShipsWithinBandbox,
    strMO_Option_ShowObjectives,
    strMO_Option_ShowTooltips,
    strMO_Option_ShowPlayerInfo,
    strMO_Option_ShowLastCommand,
    strMO_Option_ShowPlayerNames,
    strMO_Option_ShowConstruction,
    strMO_Option_ShowResearch
};

static sdword ioOverlayBackdropStatusLabelStrings[IO_SectionToggleCount] =
{
    strMO_Option_ShowSelectedShipsBackdrop,
    strMO_Option_ShowShipsWithinBandboxBackdrop,
    strMO_Option_ShowObjectivesBackdrop,
    strMO_Option_ShowTooltipsBackdrop,
    strMO_Option_ShowPlayerInfoBackdrop,
    strMO_Option_ShowLastCommandBackdrop,
    strMO_Option_ShowPlayerNamesBackdrop,
    strMO_Option_ShowConstructionBackdrop,
    strMO_Option_ShowResearchBackdrop
};

static sdword *ioCurrentOverlayPageFlags(void)
{
    if (singlePlayerGame == 1)
    {
        return ioSinglePlayerPageFlags[ioOverlayPageIndex];
    }

    return ioMultiPlayerPageFlags[ioOverlayPageIndex];
}

bool32 ioOverlayPageIsNonPrimary(void)
{
    return ioOverlayPageIndex != 0;
}

/*=============================================================================
    Logic:
=============================================================================*/

// Function callback for maks of region
udword ioListClick(regionhandle reg, sdword ID, udword event, udword data);

// Draw callback for list of ships selected
void ioShipListDraw(regionhandle region);
void ioMainOverlayStartAutoShow(sdword targetHeight);
bool32 ioMouseOverObjectivesList(void);
static bool32 ioPointInRect(sdword x, sdword y, rectangle *rect);
static bool32 ioMouseOverOverlayUIAt(sdword x, sdword y);
static sdword ioTaskbarButtonHeight(void);
static sdword ioBuildIncompleteListHeight(void);
sdword smDrawSensorEdgeButtons(void);
void smDrawPlayerAndBorderIcons(void);
extern sdword mrRenderMainScreen;
extern sdword nisWideScreenActive;
extern bool32 mrWhiteOut;
char *__cdecl TechTypeToNiceString(TechnologyType tech);
sdword __cdecl kbGetCommandCategoryName(sdword category);
void mrBuildShips(char *string, featom *atom);
void mrLaunch(char *string, featom *atom);
void mrResearch(char *string, featom *atom);
char *shipNameGet(Ship *ship);
bool32 shipCanHaveName(Ship *ship);

bool32 crapthing(udword num, void *data, struct BabyCallBack *baby)
{
    ioUpdateShipTotals();

    return(TRUE);
}

udword ioListClick(regionhandle reg, sdword ID, udword event, udword data)
{
    if (keyIsHit(SHIFTKEY))
    {
        overlayinfo[ID].listinfo->bSelected = (bool16)!overlayinfo[ID].listinfo->bSelected;
    }
    else
    {
        overlayinfo[ID].listinfo->bSelected = TRUE;
        ioSetSelection(FALSE);
    }
    tutGameMessage("Game_InfoOverlayClick");

    return(0);
}

void ioShipListDraw(regionhandle region)
{
    (void)region;
}

void ioSetSelection(bool32 shiftrelease)
{
    MaxSelection temp;
    sdword       index;
    bool32         ships=FALSE;

    temp.numShips = 0;

    for (index=0;index<selSelected.numShips;index++)
    {
        if (selSelected.ShipPtr[index]->shiptype < TOTAL_NUM_SHIPS)
        {
            if (listinfo[lookupforlist[selSelected.ShipPtr[index]->shiptype]].bSelected)
            {
//                if (selSelected.ShipPtr[index]->shiptype!=Mothership)
//                {
                    selSelectionAddSingleShip(&temp,selSelected.ShipPtr[index]);
                    ships=TRUE;
//                }
            }
        }
    }

    if (shiftrelease)
    {
        if (ships)
        {
            selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)&temp);

            taskCallBackRegister(crapthing, 0, NULL, (real32)0.25);
            tutGameMessage("Game_InfoOverlayShiftSelect");
        }
    }
    else
    {
        selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)&temp);

        taskCallBackRegister(crapthing, 0, NULL, (real32)0.25);
        tutGameMessage("Game_InfoOverlaySelect");
    }
}

void ioUpdateShipTotals(void)
{
    sdword index;

    ioShipCountEntryReset(ioShipCountPrimary);
    ioShipCountEntryReset(ioShipCountSecondary);
    ioShipCountEntryReset(ioShipCountTertiary);

    for (index=0;index<selSelected.numShips;index++)
    {
        Ship *ship;

        ship = selSelected.ShipPtr[index];
        if (ship->playerowner == universe.curPlayerPtr)
        {
            if (!shipHasCataBusyState(ship))
            {
                ioShipCountEntryAccumulate(ioShipCountPrimary, ship->shiptype);
            }
            else if ((ship->specialFlags2 & BIT20) != 0)
            {
                ioShipCountEntryAccumulate(ioShipCountTertiary, ship->shiptype);
            }
            else
            {
                ioShipCountEntryAccumulate(ioShipCountSecondary, ship->shiptype);
            }
        }
        else if ((ship->flags2 & BIT3) == 0)
        {
            ioShipCountEntryAccumulate(ioShipCountPrimary, ship->shiptype);
        }
        else if (shipIsModuleSlotActive(ship, universe.curPlayerPtr))
        {
            ioShipCountEntryAccumulate(ioShipCountPrimary, univGetShipRace((SpaceObj *)ship));
        }
        else if (shipHasModuleSlotState(ship) && univGetShipRace((SpaceObj *)ship) >= 0)
        {
            ioShipCountEntryAccumulate(ioShipCountPrimary, univGetShipRace((SpaceObj *)ship));
        }
        else if (!shipModuleSlotIsEligible(ship, universe.curPlayerPtr))
        {
            ioShipCountEntryAccumulate(ioShipCountPrimary, ship->shiptype);
        }
    }

    ioUpdateOverlayStates();
}

bool32 ioDisable(void)
{
    bool32 save;

    save = tutNextVisible;
    ioSavedNextVisible = tutNextVisible;
    ioSavedPrevButtonVisible = tutPrevButtonVisible;
    ioSavedQuitVisible = tutQuitVisible;

    if (tutNextVisible)
    {
        tutHideNextButtonRegion();
    }
    if (tutPrevButtonVisible)
    {
        tutHidePrevButtonRegion();
    }
    if (tutQuitVisible)
    {
        tutHideQuitButton();
    }

    return save;
}

void ioEnable(void)
{
    lgUpdateChatRegions();
}

void ioToggle(void)
{
    if (tutNextVisible || tutPrevButtonVisible || tutQuitVisible)
    {
        ioDisable();
    }
    else if (ioSavedNextVisible || ioSavedPrevButtonVisible || ioSavedQuitVisible)
    {
        ioEnable();
    }
}

void ioToggleBackdropOptions(void)
{
    ioOverlayShowingBackdropOptions = (bool32)!ioOverlayShowingBackdropOptions;
}

bool32 ioToggleOverlaySectionByHotkey(sdword key)
{
    sdword *pageFlags;
    sdword index;
    sdword flagIndex;

    if (!kbCommandIsHitByName("Toggle_Overlay_Options"))
    {
        return FALSE;
    }
    if (key < ONEKEY || key > NINEKEY)
    {
        return FALSE;
    }

    index = key - ONEKEY;
    if (index < 0 || index >= IO_SectionToggleCount)
    {
        return FALSE;
    }

    pageFlags = ioCurrentOverlayPageFlags();
    flagIndex = ioOverlayStatusFlagIndices[index] + (ioOverlayShowingBackdropOptions ? 1 : 0);
    pageFlags[flagIndex] ^= TRUE;
    return TRUE;
}

void ioMainOverlayStateReset(void)
{
    ioOverlayAutoShowTargetHeight = 0;
    ioOverlayAutoShowVelocity = 0;
    ioOverlayAutoShowStartTime = taskTimeElapsed;
    ioOverlayPanelY = MAIN_WindowHeight;
    ioPendingOverlayClick = FALSE;
    ioShipCountListRect.x0 = ioShipCountListRect.y0 = 0;
    ioShipCountListRect.x1 = ioShipCountListRect.y1 = 0;
    ioShipCountListBackdropRect.x0 = ioShipCountListBackdropRect.y0 = 0;
    ioShipCountListBackdropRect.x1 = ioShipCountListBackdropRect.y1 = 0;
    ioShipCountListRectActive = FALSE;
    ioShipCountListBackdropRectActive = FALSE;
    ioOverlaySectionStatusRect.x0 = ioOverlaySectionStatusRect.y0 = 0;
    ioOverlaySectionStatusRect.x1 = ioOverlaySectionStatusRect.y1 = 0;
    ioPlayerStatusPanelRect.x0 = ioPlayerStatusPanelRect.y0 = 0;
    ioPlayerStatusPanelRect.x1 = ioPlayerStatusPanelRect.y1 = 0;
    ioObjectivesPanelRight = 0;
    ioObjectivesPointerY = 0;
    ioEventFeedNewerRect.x0 = ioEventFeedNewerRect.y0 = 0;
    ioEventFeedNewerRect.x1 = ioEventFeedNewerRect.y1 = 0;
    ioEventFeedOlderRect.x0 = ioEventFeedOlderRect.y0 = 0;
    ioEventFeedOlderRect.x1 = ioEventFeedOlderRect.y1 = 0;
    ioEventFeedNewerVisible = FALSE;
    ioEventFeedOlderVisible = FALSE;
}

void ioStartup(void)
{
    sdword index, x, y;
    fonthandle oldfont;

    ioShipListFont = frFontRegister(ioShipListFontName);
    oldfont        = fontMakeCurrent(ioShipListFont);

    y = ghMainRegion->rect.y0+IO_VertSpacing;
    x = ghMainRegion->rect.x1-IO_ListWidth;

    for (index=0;index<numButtons;index++)
    {
        overlayinfo[index].region = regChildAlloc(ghMainRegion, index,
                                                  x, y, IO_ListWidth, fontHeight(" "),
                                                  0, RPE_PressLeft);
        regDrawFunctionSet(overlayinfo[index].region, ioShipListDraw);
        regFunctionSet(overlayinfo[index].region, ioListClick);
        regLinkRemove(overlayinfo[index].region);
        overlayinfo[index].inlist = FALSE;
        y+=fontHeight(" ")-2;
    }

    for (index=0;index<TOTAL_NUM_SHIPS;index++)
    {
        lookupforlist[index] = numOverlays;
    }

    for (index=0;listinfo[index].nShips!=-1;index++)
    {
        lookupforlist[listinfo[index].shipnum] = index;
    }

    ioOverlayStatusFont = frFontRegister("default.hff");
    ioOverlayShipCountFont = frFontRegister("SimplixSSK_13.hff");
    ioOverlayObjectivesFont = frFontRegister("default.hff");
    ioOverlayDebugFont = frFontRegister("Arial_12.hff");
    ioOverlayPageIndex = 0;
    ioOverlayShowingBackdropOptions = FALSE;
    ioMainOverlayStateReset();
    ioSensorLeftButtonPressed = FALSE;
    ioSensorRightButtonPressed = FALSE;
    ioSensorBottomButtonPressed = FALSE;
    ioEventFeedReset();

    fontMakeCurrent(oldfont);
}

void ioShutdown(void)
{
    sdword index;

    for (index=0;index<numButtons;index++)
    {
        regRegionDelete(overlayinfo[index].region);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ioResolutionChange
    Description : Called when there is a change in screen resolution.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ioResolutionChange(void)
{
    sdword index, x, diff;

    x = ghMainRegion->rect.x1-IO_ListWidth;
    diff = x - overlayinfo[0].region->rect.x0;

    for (index=0;index<numButtons;index++)
    {
        overlayinfo[index].region->rect.x0 += diff;
        overlayinfo[index].region->rect.x1 += diff;
    }

}



void ioUpdateOverlayStates(void)
{
    sdword n;
    sdword x, y;

    ioPendingOverlayClick = FALSE;

    if (!piePointSpecMode &&
        !(IsTabStatsEnabled() && kbKeyIsHit(98)) &&
        !byte_A2DCD4 &&
        !smSensorsActiveGet() &&
        !mrDisabled &&
        smBandboxEnabled != 1)
    {
        y = screenY;

        if (homeworldTaskbar && tutorial != 1)
        {
            if (ioMainOverlayShowing)
                ioMainOverlayShowing = (y >= ioOverlayPanelY - 5);
            else if (y >= MAIN_WindowHeight - 2)
            {
                ioMainOverlayShowing = TRUE;
                ioOverlayPanelY = 0;
            }
        }

        x = screenX;

        /* Ship-type list hover */
        for (n = 0; n < TOTAL_NUM_SHIPS; n++)
        {
            if (ioShipCountPrimary[n].enabled == TRUE)
            {
                if (x >= ioShipCountPrimary[n].rect.x0 && y >= ioShipCountPrimary[n].rect.y0 &&
                    x < ioShipCountPrimary[n].rect.x1 && y < ioShipCountPrimary[n].rect.y1)
                {
                    ioShipCountPrimary[n].selected = TRUE;
                    ioPendingOverlayClick = TRUE;
                }
                else
                    ioShipCountPrimary[n].selected = FALSE;
            }
            if (ioShipCountSecondary[n].enabled == TRUE)
            {
                if (x >= ioShipCountSecondary[n].rect.x0 && y >= ioShipCountSecondary[n].rect.y0 &&
                    x < ioShipCountSecondary[n].rect.x1 && y < ioShipCountSecondary[n].rect.y1)
                {
                    ioShipCountSecondary[n].selected = TRUE;
                    ioPendingOverlayClick = TRUE;
                }
                else
                    ioShipCountSecondary[n].selected = FALSE;
            }
            if (ioShipCountTertiary[n].enabled == TRUE)
            {
                if (x >= ioShipCountTertiary[n].rect.x0 && y >= ioShipCountTertiary[n].rect.y0 &&
                    x < ioShipCountTertiary[n].rect.x1 && y < ioShipCountTertiary[n].rect.y1)
                {
                    ioShipCountTertiary[n].selected = TRUE;
                    ioPendingOverlayClick = TRUE;
                }
                else
                    ioShipCountTertiary[n].selected = FALSE;
            }
        }

        /* Objective hover */
        ioObjectiveHoverActive = FALSE;
        for (n = 0; n < ioObjectiveCount; n++)
        {
            if (x >= ioObjectiveEntries[n].rect.x0 && y >= ioObjectiveEntries[n].rect.y0 &&
                x < ioObjectiveEntries[n].rect.x1 && y < ioObjectiveEntries[n].rect.y1)
            {
                ioObjectiveEntries[n].selected = TRUE;
                ioPendingOverlayClick = TRUE;
                if (_strcmpi(ioObjectiveEntries[n].objective->label, "Hyperspace") != 0 &&
                    _strcmpi(ioObjectiveEntries[n].objective->label, "EndMission") != 0)
                    ioObjectiveHoverActive = TRUE;
            }
            else
                ioObjectiveEntries[n].selected = FALSE;
        }

        /* Hotkey areas hover */
        for (n = 0; n < IO_NUM_HOTKEY_AREAS; n++)
        {
            if (x >= ioHotkeyAreas[n].x0 && y >= ioHotkeyAreas[n].y0 &&
                x < ioHotkeyAreas[n].x1 && y < ioHotkeyAreas[n].y1)
            {
                ioHotkeyAreas[n].hover = TRUE;
                ioPendingOverlayClick = TRUE;
            }
            else
                ioHotkeyAreas[n].hover = FALSE;
        }

        /* Build queue left-scroll hover */
        if (ioBuildScrollLeftEnabled)
        {
            if (x >= ioBuildScrollLeftRect.x0 && y >= ioBuildScrollLeftRect.y0 &&
                x < ioBuildScrollLeftRect.x1 && y < ioBuildScrollLeftRect.y1)
            {
                ioBuildScrollLeftPressed = TRUE;
                ioPendingOverlayClick = TRUE;
                return;
            }
            ioBuildScrollLeftPressed = FALSE;
        }
        /* Build queue right-scroll hover */
        if (ioBuildScrollRightEnabled)
        {
            if (x >= ioBuildScrollRightRect.x0 && y >= ioBuildScrollRightRect.y0 &&
                x < ioBuildScrollRightRect.x1 && y < ioBuildScrollRightRect.y1)
            {
                ioBuildScrollRightPressed = TRUE;
                ioPendingOverlayClick = TRUE;
                return;
            }
            ioBuildScrollRightPressed = FALSE;
        }
        /* Construction scroll up hover */
        if (ioConstructScrollUpEnabled)
        {
            if (x >= ioConstructScrollUpRect.x0 && y >= ioConstructScrollUpRect.y0 &&
                x < ioConstructScrollUpRect.x1 && y < ioConstructScrollUpRect.y1)
            {
                ioConstructScrollUpPressed = TRUE;
                ioPendingOverlayClick = TRUE;
                return;
            }
            ioConstructScrollUpPressed = FALSE;
        }
        /* Construction scroll down hover */
        if (ioConstructScrollDownEnabled)
        {
            if (x >= ioConstructScrollDownRect.x0 && y >= ioConstructScrollDownRect.y0 &&
                x < ioConstructScrollDownRect.x1 && y < ioConstructScrollDownRect.y1)
            {
                ioConstructScrollDownPressed = TRUE;
                ioPendingOverlayClick = TRUE;
                return;
            }
            ioConstructScrollDownPressed = FALSE;
        }

        /* Construction panel items hover */
        if (ioBuildQueueScrollIndex > ioBuildQueueScrollIndex - ioBuildQueuePageSize)
        {
            sdword *item = ioConstructItems[ioBuildQueueScrollIndex] + IO_CONSTRUCT_ITEM_STRIDE - IO_CONSTRUCT_ITEM_STRIDE;
            sdword count = ioBuildQueueScrollIndex - (ioBuildQueueScrollIndex - ioBuildQueuePageSize);
            sdword idx = ioBuildQueueScrollIndex;
            while (count)
            {
                item = ioConstructItems[idx];
                item[4] = 0;
                if (item[6] && x >= item[0] && y >= item[1] && x < item[2] && y < item[3])
                {
                    item[4] = 1;
                    ioPendingOverlayClick = TRUE;
                }
                idx--;
                if (idx < 0) idx = 0;
                count--;
            }
        }

        /* Build queue up-arrow hover */
        if (x >= ioBuildQueueUpArrowX0 && y >= ioBuildQueueUpArrowY0 &&
            x < ioBuildQueueUpArrowX1 && y < ioBuildQueueUpArrowY1)
        {
            ioPendingOverlayClick = TRUE;
            return;
        }
        /* Build queue down-arrow hover */
        if (x >= ioBuildQueueDownArrowX0 && y >= ioBuildQueueDownArrowY0 &&
            x < ioBuildQueueDownArrowX1 && y < ioBuildQueueDownArrowY1)
            ioPendingOverlayClick = TRUE;
        return;
    }

    /* Disabled: clear all hover states */
    for (n = 0; n < TOTAL_NUM_SHIPS; n++)
    {
        ioShipCountPrimary[n].selected = FALSE;
        ioShipCountSecondary[n].selected = FALSE;
        ioShipCountTertiary[n].selected = FALSE;
    }
    for (n = 0; n < ioObjectiveCount; n++)
        ioObjectiveEntries[n].selected = FALSE;
    for (n = 0; n < IO_NUM_HOTKEY_AREAS; n++)
        ioHotkeyAreas[n].hover = FALSE;
    ioBuildScrollLeftPressed = FALSE;
    ioBuildScrollRightPressed = FALSE;
    ioConstructScrollUpPressed = FALSE;
    ioConstructScrollDownPressed = FALSE;
    if (ioBuildQueueTotal > 0)
    {
        sdword i;
        for (i = 0; i < ioBuildQueueTotal; i++)
            ioConstructItems[i][4] = 0;
    }
    ioPendingOverlayClick = FALSE;
}

static sdword ioTaskbarButtonHeight(void)
{
    udword oldFont;
    sdword buttonHeight;

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    buttonHeight = fontHeight(" ") + 8;
    fontMakeCurrent(oldFont);
    return buttonHeight;
}

static sdword ioBuildIncompleteListHeight(void)
{
    udword oldFont;
    sdword height;

    if (!singlePlayerGame || !ioCurrentOverlayPageFlags()[IO_PageConstruction])
    {
        return 0;
    }

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    height = ioObjectiveCount * fontHeight(" ");
    if (ioObjectiveCount > 0)
    {
        height += 15 + fontHeight(" ");
    }
    fontMakeCurrent(oldFont);
    return height;
}

sdword smDrawSensorEdgeButtons(void)
{
    sdword leftX;
    sdword leftIconRightX;
    sdword rightIconLeftX;
    sdword centerY;
    sdword topY;
    sdword buttonTop;
    sdword buttonHeight;
    sdword forceVisible;

    if (!smSensorsActive)
    {
        return FALSE;
    }

    leftX = MAIN_WindowWidth / 2 - 64;
    leftIconRightX = leftX + 32;
    rightIconLeftX = leftX + 96;
    forceVisible = (ioBuildIncompleteListHeight() > leftX);

    ioSensorLeftButtonPressed = FALSE;
    ioSensorRightButtonPressed = FALSE;
    ioSensorBottomButtonPressed = FALSE;

    if ((showTaskbarButtons && tutorial != 1) || forceVisible)
    {
        buttonTop = ioOverlayPanelY;
    }
    else
    {
        buttonTop = MAIN_WindowHeight;
    }

    buttonHeight = ioTaskbarButtonHeight();
    topY = buttonTop - buttonHeight - 32;
    centerY = topY + 16;

    ioSensorLeftButtonRect.x0 = leftX;
    ioSensorLeftButtonRect.y0 = topY;
    ioSensorLeftButtonRect.x1 = leftIconRightX;
    ioSensorLeftButtonRect.y1 = topY + 32;

    ioSensorRightButtonRect.x0 = rightIconLeftX;
    ioSensorRightButtonRect.y0 = topY;
    ioSensorRightButtonRect.x1 = leftX + 128;
    ioSensorRightButtonRect.y1 = topY + 32;

    ioSensorTopButtonRect.x0 = leftIconRightX;
    ioSensorTopButtonRect.y0 = topY;
    ioSensorTopButtonRect.x1 = rightIconLeftX;
    ioSensorTopButtonRect.y1 = centerY;

    ioSensorBottomButtonRect.x0 = leftX + 32;
    ioSensorBottomButtonRect.y0 = centerY;
    ioSensorBottomButtonRect.x1 = leftX + 96;
    ioSensorBottomButtonRect.y1 = topY + 32;

    if (ioPointInRect(mouseCursorX(), mouseCursorY(), &ioSensorLeftButtonRect))
    {
        opOverlayIconDraw(OPI_SensorsScroll_Left_Mouse, topY, leftX, 32, 32, colWhite);
    }
    else
    {
        opOverlayIconDraw(OPI_SensorsScroll_Left, topY, leftX, 32, 32, colWhite);
    }

    if (ioPointInRect(mouseCursorX(), mouseCursorY(), &ioSensorRightButtonRect))
    {
        opOverlayIconDraw(OPI_SensorsScroll_Right_Mouse, topY, rightIconLeftX, 32, 32, colWhite);
    }
    else
    {
        opOverlayIconDraw(OPI_SensorsScroll_Right, topY, rightIconLeftX, 32, 32, colWhite);
    }

    if (ioPointInRect(mouseCursorX(), mouseCursorY(), &ioSensorTopButtonRect))
    {
        opOverlayIconDraw(OPI_SensorsScroll_Up_Mouse, topY, leftIconRightX, 64, 16, colWhite);
    }
    else
    {
        opOverlayIconDraw(OPI_SensorsScroll_Up, topY, leftIconRightX, 64, 16, colWhite);
    }

    if (ioPointInRect(mouseCursorX(), mouseCursorY(), &ioSensorBottomButtonRect))
    {
        opOverlayIconDraw(OPI_SensorsScroll_Down_Mouse, centerY, leftX + 32, 64, 16, colWhite);
    }
    else
    {
        opOverlayIconDraw(OPI_SensorsScroll_Down, centerY, leftX + 32, 64, 16, colWhite);
    }

    return TRUE;
}

void smDrawPlayerAndBorderIcons(void)
{
    sdword lightingEnabled;
    sdword textureEnabled;
    sdword depthTestingEnabled;
    sdword listHeight;
    sdword leftColumnY;
    sdword progressBottomY;
    sdword progressMidY;
    udword oldFont;
    rectangle backdropRect;

    if (nisWideScreenActive || nisIsRunning || !(mrRenderMainScreen || smSensorsActive) || !gameIsRunning || mrWhiteOut)
    {
        return;
    }

    if (homeworldTaskbar && tutorial != 1)
    {
        if (ioMainOverlayShowing)
        {
            ioOverlayPanelY = ioOverlayPanelTargetY;
        }
        else if (ioOverlayPanelY < MAIN_WindowHeight + 20)
        {
            ioOverlayPanelY += 20;
        }
    }
    else
    {
        ioOverlayPanelY = ioOverlayPanelTargetY;
    }

    fontShadowSet(FS_SE, colBlack);
    lightingEnabled = rndLightingEnable(FALSE);
    textureEnabled = rndTextureEnable(FALSE);
    depthTestingEnabled = rndSetDepthTesting(FALSE);
    rndAdditiveBlends(TRUE);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    listHeight = ioBuildIncompleteListHeight();
    ioOverlayPanelTargetY = MAIN_WindowHeight - listHeight;

    if (MAIN_WindowHeight - ioOverlayPanelTargetY <= 50)
    {
        ioOverlayPanelTargetY = MAIN_WindowHeight - 50;
    }

    smDrawSensorEdgeButtons();

    if (showTaskbarBackdrop && tutorial != 1)
    {
        backdropRect.x0 = 0;
        backdropRect.y0 = ioOverlayPanelY;
        backdropRect.x1 = MAIN_WindowWidth;
        backdropRect.y1 = MAIN_WindowHeight;
        primRectTranslucent2(&backdropRect, colRGBA(0, 0, 0, 96));
    }

    ioDrawShipCountLists();
    ioDrawBandboxShipCountPreview();
    ioDrawObjectivesList();
    ioDrawMouseoverObjectTooltip();
    ioDrawEventFeed();

    leftColumnY = ioDrawPlayerResourceStats(ioOverlayAutoShowTargetHeight);
    leftColumnY = ioDrawPathMarkerLegend(leftColumnY);

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    progressBottomY = ioOverlayPanelTargetY + IO_ProgressPanelBottomPadding + 5 * fontHeight(" ");
    fontMakeCurrent(oldFont);
    progressMidY = leftColumnY + (progressBottomY - leftColumnY) / 2;
    ioDrawBuildInProgressPanel(leftColumnY, progressMidY);
    ioDrawResearchProgressPanel(progressMidY, progressBottomY);
    ioDrawPlayerStatusPanel();
    ioDrawMainOverlayButtons();

    if (kbCommandIsHitByName("Toggle_Overlay_Options"))
    {
        ioDrawOverlaySectionStatus();
    }

    fontShadowSet(FS_NONE, colBlack);
    glDisable(GL_BLEND);
    rndLightingEnable(lightingEnabled);
    rndTextureEnable(textureEnabled);
    rndSetDepthTesting(depthTestingEnabled);
}

// Cataclysm-specific: draw a skinned overlay button and optionally highlight one hotkey character.
void ioDrawBoundedLabel(
    rectangle *bounds,
    char *text,
    sdword highlightedIndex,
    fonthandle font,
    sdword capWidth,
    sdword leftIcon,
    sdword leftHoverIcon,
    sdword centerIcon,
    sdword centerHoverIcon,
    sdword rightIcon,
    sdword rightHoverIcon,
    bool32 hoverWholeButton)
{
    sdword activeLeftIcon = leftIcon;
    sdword activeCenterIcon = centerIcon;
    sdword activeRightIcon = rightIcon;
    sdword textX;
    sdword textY;
    sdword textWidth;
    sdword textHeight;
    char prefix[20];
    char highlightedCharacter[2];
    fonthandle oldFont;

    if (hoverWholeButton)
    {
        if (primPointInRectXY2(bounds, mouseCursorX(), mouseCursorY()))
        {
            activeLeftIcon = leftHoverIcon;
            activeCenterIcon = centerHoverIcon;
            activeRightIcon = rightHoverIcon;
        }
    }
    else if (mouseCursorX() > bounds->x0 &&
             mouseCursorY() > bounds->y0 &&
             mouseCursorX() < bounds->x0 + capWidth &&
             mouseCursorY() < bounds->y1)
    {
        activeLeftIcon = leftHoverIcon;
    }
    else if (mouseCursorX() > bounds->x1 - capWidth &&
             mouseCursorY() > bounds->y0 &&
             mouseCursorX() < bounds->x1 &&
             mouseCursorY() < bounds->y1)
    {
        activeRightIcon = rightHoverIcon;
    }

    opOverlayIconDraw(activeLeftIcon, bounds->y0, bounds->x0, capWidth, bounds->y1 - bounds->y0, colRGB(255, 255, 255));
    opOverlayIconDraw(activeRightIcon, bounds->y0, bounds->x1 - capWidth, capWidth, bounds->y1 - bounds->y0, colRGB(255, 255, 255));
    opOverlayIconDraw(activeCenterIcon, bounds->y0, bounds->x0 + capWidth, bounds->x1 - bounds->x0 - 2 * capWidth, bounds->y1 - bounds->y0, colRGB(255, 255, 255));

    oldFont = fontMakeCurrent(font);
    textWidth = fontWidth(text);
    textHeight = fontHeight(text);
    textX = bounds->x0 + capWidth + (bounds->x1 - bounds->x0 - 2 * capWidth - textWidth) / 2;
    textY = bounds->y0 + (bounds->y1 - bounds->y0 - textHeight) / 2;

    if (highlightedIndex == -1)
    {
        fontPrintf(textX, textY, colRGB(255, 255, 255), text);
    }
    else
    {
        strcpy(prefix, text);
        prefix[highlightedIndex] = '\0';
        highlightedCharacter[0] = text[highlightedIndex];
        highlightedCharacter[1] = '\0';

        fontPrintf(textX, textY, 0xFFC8C8C8, prefix);
        textX += fontWidth(prefix);
        fontPrintf(textX, textY, colRGB(255, 255, 255), highlightedCharacter);
        textX += fontWidth(highlightedCharacter);
        fontPrintf(textX, textY, 0xFFC8C8C8, &text[highlightedIndex + 1]);
    }

    fontMakeCurrent(oldFont);
}

// Cataclysm-specific: grow a translucent bounds box around overlay text as it is printed.
void ioPrintBoundedLabel(rectangle *bounds, sdword x, sdword y, color c, char *string)
{
    if (x - IO_BoundedLabelPadding < bounds->x0)
    {
        bounds->x0 = x - IO_BoundedLabelPadding;
    }
    if (x + IO_BoundedLabelPadding + fontWidth(string) > bounds->x1)
    {
        bounds->x1 = x + IO_BoundedLabelPadding + fontWidth(string);
    }
    if (y - IO_BoundedLabelPadding < bounds->y0)
    {
        bounds->y0 = y - IO_BoundedLabelPadding;
    }
    if (y + IO_BoundedLabelPadding + fontHeight(string) > bounds->y1)
    {
        bounds->y1 = y + IO_BoundedLabelPadding + fontHeight(string);
    }

    glDisable(3042);
    fontPrint(x, y, c, string);
    glEnable(3042);
}

static bool32 ioPointInRect(sdword x, sdword y, rectangle *rect)
{
    return (x >= rect->x0 && x < rect->x1 && y >= rect->y0 && y < rect->y1);
}

static void ioResetProgressPanelState(InfoOverlayProgressPanelState *state)
{
    state->textBounds.x0 = 0;
    state->textBounds.y0 = 0;
    state->textBounds.x1 = 0;
    state->textBounds.y1 = 0;
    state->panelRect.x0 = 0;
    state->panelRect.y0 = 0;
    state->panelRect.x1 = 0;
    state->panelRect.y1 = 0;
    state->topArrowRect.x0 = 0;
    state->topArrowRect.y0 = 0;
    state->topArrowRect.x1 = 0;
    state->topArrowRect.y1 = 0;
    state->bottomArrowRect.x0 = 0;
    state->bottomArrowRect.y0 = 0;
    state->bottomArrowRect.x1 = 0;
    state->bottomArrowRect.y1 = 0;
    state->textBoundsActive = FALSE;
    state->panelRectActive = FALSE;
    state->topArrowVisible = FALSE;
    state->bottomArrowVisible = FALSE;
    state->topArrowHighlighted = FALSE;
    state->bottomArrowHighlighted = FALSE;
}

static void ioUnionRect(rectangle *dest, rectangle *src, bool32 *hasRect)
{
    if (!*hasRect)
    {
        *dest = *src;
        *hasRect = TRUE;
        return;
    }

    if (src->x0 < dest->x0)
    {
        dest->x0 = src->x0;
    }
    if (src->y0 < dest->y0)
    {
        dest->y0 = src->y0;
    }
    if (src->x1 > dest->x1)
    {
        dest->x1 = src->x1;
    }
    if (src->y1 > dest->y1)
    {
        dest->y1 = src->y1;
    }
}

static void ioScrollArrowTrianglePointsGet(sdword direction, rectangle *rect, triangle *tri)
{
    sdword centerX;
    sdword centerY;

    centerX = (rect->x0 + rect->x1) / 2;
    centerY = (rect->y0 + rect->y1) / 2;

    tri->x0 = centerX;
    tri->x1 = rect->x0 + 3;
    tri->x2 = rect->x1 - 3;

    if (direction != 0)
    {
        tri->y0 = rect->y1 - 3;
        tri->y1 = rect->y0 + 3;
        tri->y2 = rect->y0 + 3;
    }
    else
    {
        tri->y0 = rect->y0 + 3;
        tri->y1 = rect->y1 - 3;
        tri->y2 = rect->y1 - 3;
    }
}

static sdword ioBuildCurrentResearchProgressList(Player *player, InfoOverlayResearchProgressEntry *outList)
{
    Node *walk;
    ResearchTopic *topic;
    sdword *timeTable;
    sdword count;

    if (player == NULL || outList == NULL)
    {
        return 0;
    }

    timeTable = rmPlayerResearchTimeTable(player);
    if (timeTable == NULL)
    {
        return 0;
    }

    count = 0;
    for (walk = rmPlayerResearchTopicList(player)->head; walk != NULL; walk = walk->next)
    {
        topic = (ResearchTopic *)listGetStructOfNode(walk);
        if (topic == NULL)
        {
            continue;
        }
        if (topic->techresearch < 0 || topic->techresearch >= NumTechnologies)
        {
            continue;
        }

        outList[count].tech = topic->techresearch;
        outList[count].totalTime = (real32)timeTable[topic->techresearch];
        outList[count].timeLeft = topic->timeleft;
        count++;
    }

    return count;
}

static bool32 ioMouseOverOverlayUIAt(sdword x, sdword y)
{
    sdword index;

    if (ioPointInRect(x, y, &ioSensorLeftButtonRect) ||
        ioPointInRect(x, y, &ioSensorRightButtonRect) ||
        ioPointInRect(x, y, &ioSensorTopButtonRect) ||
        ioPointInRect(x, y, &ioSensorBottomButtonRect))
    {
        return TRUE;
    }

    if (ioShipCountListRectActive &&
        ioPointInRect(x, y, &ioShipCountListRect))
    {
        return TRUE;
    }

    if (showTaskbarButtons && tutorial != 1)
    {
        for (index = 0; index < 4; index++)
        {
            if (ioPointInRect(x, y, &ioMainOverlayButtonRects[index]))
            {
                return TRUE;
            }
        }
    }

    if (ioPointInRect(x, y, &ioOverlaySectionStatusRect) ||
        ioPointInRect(x, y, &ioPlayerStatusPanelRect) ||
        ioPointInRect(x, y, &ioBuildProgressPanelState.panelRect) ||
        ioPointInRect(x, y, &ioResearchProgressPanelState.panelRect) ||
        ioPointInRect(x, y, &ioBuildProgressPanelState.topArrowRect) ||
        ioPointInRect(x, y, &ioBuildProgressPanelState.bottomArrowRect) ||
        ioPointInRect(x, y, &ioResearchProgressPanelState.topArrowRect) ||
        ioPointInRect(x, y, &ioResearchProgressPanelState.bottomArrowRect))
    {
        return TRUE;
    }

    for (index = 0; index < ioObjectiveCount; index++)
    {
        if (ioObjectiveEntries[index].enabled &&
            ioPointInRect(x, y, &ioObjectiveEntries[index].rect))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void ioDrawPanelBackground(rectangle *rect)
{
    primRectSolid2(rect, colRGB(0, 0, 0));
    primRoundRectOutline(rect, 3, colRGB(96, 160, 224), 4, 4);
}

static char ioMouseoverLabel[256];

static bool32 ioShipIsCrystal(Ship *ship)
{
    return IsCrystal(ship);
}

static sdword ioAsteroidTooltipResourceValue(const Asteroid *asteroid)
{
    sdword resourceValue = asteroid->resourcevalue;

    switch (asteroid->asteroidtype)
    {
        case Asteroid2:
            resourceValue += 320;
            break;
        case Asteroid3:
            resourceValue += 1600;
            break;
        case Asteroid4:
            resourceValue += 5920;
            break;
        default:
            break;
    }

    return resourceValue;
}

static char *ioFormatResourceTooltipLabel(const char *label, sdword resourceValue)
{
    sprintf(ioMouseoverLabel, "%s (%d)", label, resourceValue);
    return ioMouseoverLabel;
}

static void ioSetTooltipColors(color *textColor, color *backdropColor, color *outlineColor,
                               color text, color backdrop, color outline)
{
    *textColor = text;
    *backdropColor = backdrop;
    *outlineColor = outline;
}

static char *ioFormatShipTypeLabel(Ship *ship)
{
    sprintf(ioMouseoverLabel, "%s", ShipTypeToNiceStr(ship->shiptype));
    return ioMouseoverLabel;
}

static char *ioFormatShipTypeOwnerLabel(Ship *ship)
{
    if (!singlePlayerGame && ship->playerowner != NULL)
    {
        sprintf(ioMouseoverLabel, "%s (%s)",
                ShipTypeToNiceStr(ship->shiptype),
                playerNames[ship->playerowner->playerIndex]);
    }
    else
    {
        sprintf(ioMouseoverLabel, "%s", ShipTypeToNiceStr(ship->shiptype));
    }
    return ioMouseoverLabel;
}

static char *ioMouseoverLabelGet(SpaceObj *spaceObj, color *textColor, color *backdropColor, color *outlineColor)
{
    Ship *ship;
    sdword displayType;
    char *shipName;
    bool32 moduleSlotActive;
    bool32 moduleSlotPending;
    bool32 moduleSlotCountLabel;

    if (spaceObj == NULL)
    {
        return NULL;
    }

    switch (spaceObj->objtype)
    {
        case OBJ_ShipType:
            ship = (Ship *)spaceObj;
            displayType = univGetShipRace(spaceObj);
            moduleSlotActive = shipIsModuleSlotActive(ship, universe.curPlayerPtr);
            moduleSlotPending = univShipModuleSlotIsEligibleUpgrade(ship, universe.curPlayerPtr);
            moduleSlotCountLabel = shipModuleSlotIsEligible(ship, universe.curPlayerPtr);

            switch (mouseCursorOverShipType)
            {
                case MC_ENEMY_SHIP:
                    if ((ship->flags & SOF_DockMarker) == 0)
                    {
                        if ((ship->flags2 & BIT3) != 0)
                        {
                            if (moduleSlotActive && displayType >= 0)
                            {
                                sprintf(ioMouseoverLabel, "%s", ShipTypeToNiceStr((ShipType)displayType));
                                ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                                   IO_TooltipTextColorFriendly,
                                                   IO_TooltipBackdropColorFriendly,
                                                   IO_TooltipOutlineColorFriendly);
                                return ioMouseoverLabel;
                            }

                            if (moduleSlotPending ||
                                (shipHasModuleSlotState(ship) && displayType >= 0))
                            {
                                if (displayType >= 0)
                                {
                                    sprintf(ioMouseoverLabel, "%s", ShipTypeToNiceStr((ShipType)displayType));
                                    ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                                       IO_TooltipTextColorModule,
                                                       IO_TooltipBackdropColorModule,
                                                       IO_TooltipOutlineColorModule);
                                    return ioMouseoverLabel;
                                }
                            }

                            if (moduleSlotCountLabel)
                            {
                                sprintf(ioMouseoverLabel, "%s (%d)",
                                        ShipTypeToNiceStr(ship->shiptype),
                                        shipModuleSlotRuntime(ship)->activeBuildTarget.displayCount);
                                ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                                   IO_TooltipTextColorResource,
                                                   IO_TooltipBackdropColorResource,
                                                   IO_TooltipOutlineColorResource);
                                return ioMouseoverLabel;
                            }
                        }

                        ioFormatShipTypeOwnerLabel(ship);
                        ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                           IO_TooltipTextColorModule,
                                           IO_TooltipBackdropColorModule,
                                           IO_TooltipOutlineColorModule);
                        return ioMouseoverLabel;
                    }
                    break;

                case MC_ALLIED_SHIP:
                    ioFormatShipTypeOwnerLabel(ship);
                    ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                       IO_TooltipTextColorForeign,
                                       IO_TooltipBackdropColorForeign,
                                       IO_TooltipOutlineColorForeign);
                    return ioMouseoverLabel;
            }

            if (ioShipIsCrystal(ship))
            {
                sprintf(ioMouseoverLabel, "%s (%d)", ShipTypeToNiceStr(ship->shiptype), ship->ShipSpecifics[0]);
                ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                   IO_TooltipTextColorResource,
                                   IO_TooltipBackdropColorResource,
                                   IO_TooltipOutlineColorResource);
                return ioMouseoverLabel;
            }

            if (shipCanHaveName(ship))
            {
                shipName = shipNameGet(ship);
                if (shipName != NULL && shipName[0] != '\0')
                {
                    sprintf(ioMouseoverLabel, "%s (%s)", ShipTypeToNiceStr(ship->shiptype), shipName);
                }
                else
                {
                    sprintf(ioMouseoverLabel, "%s", ShipTypeToNiceStr(ship->shiptype));
                }
            }
            else
            {
                sprintf(ioMouseoverLabel, "%s", ShipTypeToNiceStr(ship->shiptype));
            }

            if ((ship->specialFlags & SPECIAL_FriendlyStatus) != 0)
            {
                ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                   IO_TooltipTextColorForeign,
                                   IO_TooltipBackdropColorForeign,
                                   IO_TooltipOutlineColorForeign);
            }
            else
            {
                ioSetTooltipColors(textColor, backdropColor, outlineColor,
                                   IO_TooltipTextColorFriendly,
                                   IO_TooltipBackdropColorFriendly,
                                   IO_TooltipOutlineColorFriendly);
            }
            return ioMouseoverLabel;
        case OBJ_AsteroidType:
            ioSetTooltipColors(textColor, backdropColor, outlineColor,
                               IO_TooltipTextColorResource,
                               IO_TooltipBackdropColorResource,
                               IO_TooltipOutlineColorResource);
            return ioFormatResourceTooltipLabel(strGetString(strMO_Tooltip_Resource),
                                                ioAsteroidTooltipResourceValue((Asteroid *)spaceObj));
        case OBJ_NebulaType:
            ioSetTooltipColors(textColor, backdropColor, outlineColor,
                               IO_TooltipTextColorResource,
                               IO_TooltipBackdropColorResource,
                               IO_TooltipOutlineColorResource);
            return ioFormatResourceTooltipLabel(strGetString(strMO_Tooltip_Nebula),
                                                ((Nebula *)spaceObj)->resourcevalue);
        case OBJ_GasType:
            ioSetTooltipColors(textColor, backdropColor, outlineColor,
                               IO_TooltipTextColorResource,
                               IO_TooltipBackdropColorResource,
                               IO_TooltipOutlineColorResource);
            return ioFormatResourceTooltipLabel(strGetString(strMO_Tooltip_GasCloud),
                                                ((GasCloud *)spaceObj)->resourcevalue);
        case OBJ_DustType:
            ioSetTooltipColors(textColor, backdropColor, outlineColor,
                               IO_TooltipTextColorResource,
                               IO_TooltipBackdropColorResource,
                               IO_TooltipOutlineColorResource);
            return ioFormatResourceTooltipLabel(strGetString(strMO_Tooltip_DustCloud),
                                                ((DustCloud *)spaceObj)->resourcevalue);
        case OBJ_DerelictType:
            *textColor = IO_TooltipTextColorResource;
            *backdropColor = IO_TooltipBackdropColorResource;
            *outlineColor = IO_TooltipOutlineColorResource;
            return "Derelict";
        case OBJ_MissileType:
            *textColor = IO_TooltipTextColorMissile;
            *backdropColor = IO_TooltipBackdropColorMissile;
            *outlineColor = IO_TooltipOutlineColorMissile;
            return "Missile";
        default:
            return NULL;
    }
}

udword ioConsumePendingOverlayClick(void)
{
    udword pendingClick = ioPendingOverlayClick;
    ioPendingOverlayClick = FALSE;
    return pendingClick;
}

bool32 ioMouseOverObjectivesList(void)
{
    sdword index;

    for (index = 0; index < ioObjectiveCount; index++)
    {
        if (!ioObjectiveEntries[index].enabled)
        {
            continue;
        }
        if (ioPointInRect(screenX, screenY, &ioObjectiveEntries[index].rect))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void ioMainOverlayStartAutoShow(sdword targetHeight)
{
    ioOverlayAutoShowTargetHeight = targetHeight;
    ioOverlayAutoShowStartTime = taskTimeElapsed;
    ioOverlayPanelY = MAIN_WindowHeight + targetHeight;
}

sdword ioOverlayPageCycle(sdword forward)
{
    ioOverlayPageIndex += (forward != 0) ? 1 : -1;
    if (ioOverlayPageIndex >= 3)
    {
        ioOverlayPageIndex = 0;
    }
    else if (ioOverlayPageIndex < 0)
    {
        ioOverlayPageIndex = 2;
    }

    return ioOverlayPageIndex;
}

void ioOverlayElapsedTimeResetFromUniverse(void)
{
    ioOverlayElapsedTime = universe.totaltimeelapsed;
}

void ioOverlayElapsedTimeSave(void)
{
    SaveInfoNumber(TreatAsUdword(ioOverlayElapsedTime));
}

void ioOverlayElapsedTimeLoad(void)
{
    sdword elapsedTime = LoadInfoNumber();
    ioOverlayElapsedTime = TreatAsReal32(elapsedTime);
}

void ioDrawObjectivesList(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    fonthandle oldFont;
    sdword panelTop;
    sdword rowHeight;
    sdword index;
    sdword textX;
    sdword y;
    sdword maxRight;
    rectangle backdropRect;
    char string[256];
    char *headerString;
    char *statusString;
    color textColors[IO_MaxObjectiveEntries];

    ioObjectiveCount = 0;
    if (!pageFlags[IO_PageObjectives])
    {
        return;
    }

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    rowHeight = fontHeight(" ");
    headerString = strGetString(strMO_PrimaryObjectives);
    textX = IO_ObjectivesX;
    panelTop = ioOverlayPanelY;
    y = panelTop;
    maxRight = 0;
    for (index = 0; index < objectivesUsed && ioObjectiveCount < IO_MaxObjectiveEntries; index++)
    {
        Objective *objective = objectives[index];
        char *description;
        color textColor;

        if (objective == NULL)
        {
            continue;
        }
        if (objective->primary != TRUE)
        {
            continue;
        }

        if (ioObjectiveCount == 0)
        {
            maxRight = textX + fontWidth(headerString);
            y += rowHeight + IO_ObjectivesGap;
        }

        if (_stricmp(objective->label, "Hyperspace") == 0)
        {
            sprintf(string, "%s", singlePlayerGetHyperspaceObjectiveLabel());
        }
        else
        {
            description = (objective->description != NULL) ? objective->description : objective->label;
            statusString = objective->status ? strGetString(strMO_ObjectiveStatus_Complete) : strGetString(strMO_ObjectiveStatus_Incomplete);
            sprintf(string, "%s - %s", statusString, description);
        }

        ioObjectiveEntries[ioObjectiveCount].enabled = TRUE;
        ioObjectiveEntries[ioObjectiveCount].objective = objective;
        ioObjectiveEntries[ioObjectiveCount].rect.x0 = textX;
        ioObjectiveEntries[ioObjectiveCount].rect.y0 = y;
        ioObjectiveEntries[ioObjectiveCount].rect.x1 = textX + fontWidth(string);
        ioObjectiveEntries[ioObjectiveCount].rect.y1 = y + rowHeight;
        ioObjectiveEntries[ioObjectiveCount].selected = ioPointInRect(screenX, screenY, &ioObjectiveEntries[ioObjectiveCount].rect);

        if (ioObjectiveEntries[ioObjectiveCount].rect.x1 > maxRight)
        {
            maxRight = ioObjectiveEntries[ioObjectiveCount].rect.x1;
        }

        if (objective->status == TRUE)
        {
            textColor = IO_ObjectivesCompleteColor;
        }
        else if (ioObjectiveEntries[ioObjectiveCount].selected)
        {
            textColor = IO_ObjectivesSelectedColor;
        }
        else
        {
            textColor = IO_ObjectivesInactiveColor;
        }

        textColors[ioObjectiveCount] = textColor;
        ioObjectiveCount++;
        y += rowHeight;
    }

    if (ioObjectiveCount > 0)
    {
        if (pageFlags[IO_PageObjectivesBackdrop] && (!showTaskbarBackdrop || tutorial == 1))
        {
            backdropRect.x0 = 0;
            backdropRect.y0 = panelTop - IO_ObjectivesBackdropPadTop;
            backdropRect.x1 = maxRight + IO_ObjectivesBackdropPadRight;
            backdropRect.y1 = MAIN_WindowHeight;
            primRectTranslucent2(&backdropRect, IO_ObjectivesBackdropColor);
        }

        fontPrint(textX, panelTop, IO_ObjectivesHeaderColor, headerString);
        for (index = 0; index < ioObjectiveCount; index++)
        {
            Objective *objective = ioObjectiveEntries[index].objective;
            char *description;

            if (_stricmp(objective->label, "Hyperspace") == 0)
            {
                sprintf(string, "%s", singlePlayerGetHyperspaceObjectiveLabel());
            }
            else
            {
                description = (objective->description != NULL) ? objective->description : objective->label;
                statusString = objective->status ? strGetString(strMO_ObjectiveStatus_Complete) : strGetString(strMO_ObjectiveStatus_Incomplete);
                sprintf(string, "%s - %s", statusString, description);
            }

            fontPrint(ioObjectiveEntries[index].rect.x0,
                      ioObjectiveEntries[index].rect.y0,
                      textColors[index],
                      string);
        }
        ioObjectivesPanelRight = maxRight + IO_ObjectivesBackdropPadRight;
        ioObjectivesPointerY = panelTop - IO_ObjectivesBackdropPadTop;
        tutSetPointerTargetCursorPosition(ioObjectivesPanelRight, ioObjectivesPointerY);
    }
    else
    {
        ioObjectivesPanelRight = 0;
        ioObjectivesPointerY = 0;
    }

    fontMakeCurrent(oldFont);
}

udword ioDrawOverlaySectionStatus(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    rectangle rect;
    fonthandle oldFont;
    sdword rowHeight;
    sdword index;
    char string[256];

    oldFont = fontMakeCurrent(ioOverlayStatusFont);
    rowHeight = fontHeight(" ");

    rect.x0 = 10;
    rect.y0 = 10 + ioOverlayAutoShowTargetHeight;
    rect.x1 = rect.x0 + fontWidth(" (9) AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ") + 30;
    rect.y1 = rect.y0 + IO_SectionToggleCount * rowHeight + 20;
    ioOverlaySectionStatusRect = rect;

    ioDrawPanelBackground(&rect);

    for (index = 0; index < IO_SectionToggleCount; index++)
    {
        sdword flagIndex = ioOverlayStatusFlagIndices[index];
        sdword labelString = ioOverlayShowingBackdropOptions ? ioOverlayBackdropStatusLabelStrings[index]
                                                             : ioOverlayStatusLabelStrings[index];
        bool32 enabled = pageFlags[flagIndex + (ioOverlayShowingBackdropOptions ? 1 : 0)];
        color textColor = enabled ? colRGB(255, 255, 255) : colRGB(160, 160, 160);
        char *statusText = enabled ? "Enabled" : "Disabled";

        sprintf(string,
                "(%c) %s %s",
                (char)('1' + index),
                strGetString(labelString),
                statusText);
        fontPrint(rect.x0 + 10, rect.y0 + 10 + index * rowHeight, textColor, string);
    }

    fontMakeCurrent(oldFont);
    return TRUE;
}

udword ioDrawShipCountLists(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    fonthandle oldFont;
    sdword shiptype;
    sdword rowHeight;
    sdword x;
    sdword y;
    rectangle backdropRect;
    bool32 showBackdrop;

    ioShipCountListRect.x0 = ioShipCountListRect.y0 = 0;
    ioShipCountListRect.x1 = ioShipCountListRect.y1 = 0;
    ioShipCountListRectActive = FALSE;
    ioShipCountListRowCount = 0;

    if (!pageFlags[IO_PageSelected])
    {
        ioShipCountListBackdropRect.x0 = ioShipCountListBackdropRect.y0 = 0;
        ioShipCountListBackdropRect.x1 = ioShipCountListBackdropRect.y1 = 0;
        ioShipCountListBackdropRectActive = FALSE;
        return FALSE;
    }

    oldFont = fontMakeCurrent(ioOverlayShipCountFont);
    rowHeight = fontHeight(" ");
    x = MAIN_WindowWidth - IO_ListWidth;
    y = ghMainRegion->rect.y0 + IO_VertSpacing;
    showBackdrop = pageFlags[IO_PageSelectedBackdrop];

    if (showBackdrop && ioShipCountListBackdropRectActive)
    {
        primRectTranslucent2(&ioShipCountListBackdropRect, IO_ProgressPanelBackdropColor);
    }
    backdropRect.x0 = x;
    backdropRect.y0 = y;
    backdropRect.x1 = x;
    backdropRect.y1 = y;
    ioShipCountListBackdropRectActive = FALSE;

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        ioDrawShipCountEntryRow(&ioShipCountPrimary[shiptype], shiptype, x, &y, rowHeight,
                                showBackdrop, FALSE, colWhite, &backdropRect);
        ioDrawShipCountEntryRow(&ioShipCountSecondary[shiptype], shiptype, x, &y, rowHeight,
                                showBackdrop, FALSE, colRGB(120, 120, 120), &backdropRect);
        ioDrawShipCountEntryRow(&ioShipCountTertiary[shiptype], shiptype, x, &y, rowHeight,
                                showBackdrop, TRUE, colRGB(0, 192, 255), &backdropRect);
    }

    if (showBackdrop && ioShipCountListBackdropRectActive)
    {
        ioShipCountListBackdropRect = backdropRect;
        ioUnionRect(&ioShipCountListRect, &backdropRect, &ioShipCountListRectActive);
    }
    else
    {
        ioShipCountListBackdropRect.x0 = ioShipCountListBackdropRect.y0 = 0;
        ioShipCountListBackdropRect.x1 = ioShipCountListBackdropRect.y1 = 0;
    }

    fontMakeCurrent(oldFont);
    return ioShipCountListRectActive;
}

static udword ioDrawBandboxShipCountPreview(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    fonthandle oldFont;
    sdword rowHeight;
    sdword x;
    sdword y;
    sdword index;
    sdword shiptype;
    sdword friendlyCounts[TOTAL_NUM_SHIPS];
    sdword alliedCounts[TOTAL_NUM_SHIPS];
    sdword enemyCounts[TOTAL_NUM_SHIPS];
    sdword mineCount;
    rectangle backdropRect;
    rectangle lineRect;
    bool32 showBackdrop;
    bool32 drewAnything;
    bool32 drewBackdrop = FALSE;
    char string[128];

    ioShipCountPreviewRect.x0 = ioShipCountPreviewRect.y0 = 0;
    ioShipCountPreviewRect.x1 = ioShipCountPreviewRect.y1 = 0;
    ioShipCountPreviewRectActive = FALSE;

    if (!pageFlags[IO_PageSelecting] || attackcom.numTargets <= 0)
    {
        ioShipCountPreviewBackdropRect.x0 = ioShipCountPreviewBackdropRect.y0 = 0;
        ioShipCountPreviewBackdropRect.x1 = ioShipCountPreviewBackdropRect.y1 = 0;
        ioShipCountPreviewBackdropRectActive = FALSE;
        return FALSE;
    }

    memset(friendlyCounts, 0, sizeof(friendlyCounts));
    memset(alliedCounts, 0, sizeof(alliedCounts));
    memset(enemyCounts, 0, sizeof(enemyCounts));
    mineCount = 0;
    drewAnything = FALSE;

    for (index = 0; index < attackcom.numTargets; index++)
    {
        Ship *ship;
        sdword previewShipType;

        if (attackcom.TargetPtr[index]->objtype != OBJ_ShipType)
        {
            continue;
        }

        ship = (Ship *)attackcom.TargetPtr[index];
        if (ship->playerowner != universe.curPlayerPtr)
        {
            if ((ship->flags2 & BIT3) != 0 && shipIsModuleSlotActive(ship, universe.curPlayerPtr))
            {
                previewShipType = univGetShipRace((SpaceObj *)ship);
                if (previewShipType >= 0 && previewShipType < TOTAL_NUM_SHIPS)
                {
                    friendlyCounts[previewShipType]++;
                }
            }
        }
        else if ((ship->specialFlags2 & BIT20) == 0)
        {
            previewShipType = ship->shiptype;
            if (previewShipType >= 0 && previewShipType < TOTAL_NUM_SHIPS)
            {
                friendlyCounts[previewShipType]++;
            }
        }
        else
        {
            previewShipType = ship->shiptype;
            if (previewShipType >= 0 && previewShipType < TOTAL_NUM_SHIPS)
            {
                alliedCounts[previewShipType]++;
            }
        }
    }

    for (index = 0; index < attackcom.numTargets; index++)
    {
        Ship *ship;

        if (attackcom.TargetPtr[index]->objtype != OBJ_ShipType)
        {
            continue;
        }

        ship = (Ship *)attackcom.TargetPtr[index];
        if (ship->playerowner != universe.curPlayerPtr &&
            allianceArePlayersAllied(ship->playerowner, universe.curPlayerPtr))
        {
            shiptype = ship->shiptype;
            if (shiptype >= 0 && shiptype < TOTAL_NUM_SHIPS)
            {
                alliedCounts[shiptype]++;
            }
        }
    }

    for (index = 0; index < attackcom.numTargets; index++)
    {
        Ship *ship;

        if (attackcom.TargetPtr[index]->objtype != OBJ_ShipType)
        {
            continue;
        }

        ship = (Ship *)attackcom.TargetPtr[index];
        if (ship->playerowner == universe.curPlayerPtr ||
            allianceArePlayersAllied(ship->playerowner, universe.curPlayerPtr))
        {
            continue;
        }
        if ((ship->flags2 & BIT3) != 0)
        {
            if (shipIsModuleSlotActive(ship, universe.curPlayerPtr) ||
                shipModuleSlotIsEligible(ship, universe.curPlayerPtr) ||
                univShipModuleSlotIsEligibleUpgrade(ship, universe.curPlayerPtr))
            {
                continue;
            }
            if (shipHasModuleSlotState(ship))
            {
                shiptype = univGetShipRace((SpaceObj *)ship);
                if (shiptype >= 0 && shiptype < TOTAL_NUM_SHIPS)
                {
                    enemyCounts[shiptype]++;
                    continue;
                }
            }
        }
        shiptype = ship->shiptype;
        if (shiptype >= 0 && shiptype < TOTAL_NUM_SHIPS)
        {
            enemyCounts[shiptype]++;
        }
    }

    for (index = 0; index < attackcom.numTargets; index++)
    {
        if (attackcom.TargetPtr[index]->objtype == OBJ_MissileType &&
            ((Missile *)attackcom.TargetPtr[index])->missileType == MISSILE_Mine)
        {
            mineCount++;
        }
    }

    oldFont = fontMakeCurrent(ioOverlayShipCountFont);
    rowHeight = fontHeight(" ");
    x = MAIN_WindowWidth - IO_ListWidth;
    y = ghMainRegion->rect.y0 + IO_BandboxPreviewGap + ioShipCountListRowCount * rowHeight;
    showBackdrop = pageFlags[IO_PageSelectingBackdrop];

    if (showBackdrop)
    {
        if (ioShipCountPreviewBackdropRectActive)
        {
            primRectTranslucent2(&ioShipCountPreviewBackdropRect, IO_BandboxPreviewBackdropColor);
        }
        backdropRect.x0 = x;
        backdropRect.y0 = y;
        backdropRect.x1 = x;
        backdropRect.y1 = y;
    }

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        if (friendlyCounts[shiptype] <= 0)
        {
            continue;
        }

        ioFormatShipCountLabel(string, friendlyCounts[shiptype], shiptype, FALSE);
        lineRect.x0 = x;
        lineRect.y0 = y;
        lineRect.x1 = x + fontWidth(string);
        lineRect.y1 = y + rowHeight;
        if (showBackdrop)
        {
            ioPrintBoundedLabel(&backdropRect, x, y, IO_BandboxPreviewFriendlyColor, string);
            drewBackdrop = TRUE;
        }
        else
        {
            fontPrint(x, y, IO_BandboxPreviewFriendlyColor, string);
        }
        ioUnionRect(&ioShipCountPreviewRect, &lineRect, &ioShipCountPreviewRectActive);
        y += rowHeight;
        drewAnything = TRUE;
    }

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        color textColor = IO_BandboxPreviewAlliedColor;

        if (alliedCounts[shiptype] <= 0)
        {
            continue;
        }

        ioFormatShipCountLabel(string, alliedCounts[shiptype], shiptype, TRUE);
        lineRect.x0 = x;
        lineRect.y0 = y;
        lineRect.x1 = x + fontWidth(string);
        lineRect.y1 = y + rowHeight;
        if (showBackdrop)
        {
            ioPrintBoundedLabel(&backdropRect, x, y, textColor, string);
            drewBackdrop = TRUE;
        }
        else
        {
            fontPrint(x, y, textColor, string);
        }
        ioUnionRect(&ioShipCountPreviewRect, &lineRect, &ioShipCountPreviewRectActive);
        y += rowHeight;
        drewAnything = TRUE;
    }

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        if (enemyCounts[shiptype] <= 0)
        {
            continue;
        }

        sprintf(string, "%i x %s", enemyCounts[shiptype], ShipTypeToNiceStr(shiptype));
        lineRect.x0 = x;
        lineRect.y0 = y;
        lineRect.x1 = x + fontWidth(string);
        lineRect.y1 = y + rowHeight;
        if (showBackdrop)
        {
            ioPrintBoundedLabel(&backdropRect, x, y, IO_BandboxPreviewEnemyColor, string);
            drewBackdrop = TRUE;
        }
        else
        {
            fontPrint(x, y, IO_BandboxPreviewEnemyColor, string);
        }
        ioUnionRect(&ioShipCountPreviewRect, &lineRect, &ioShipCountPreviewRectActive);
        y += rowHeight;
        drewAnything = TRUE;
    }

    if (mineCount > 0)
    {
        sprintf(string, "%i x %s", mineCount, strGetString(strMO_Tooltip_Mine));
        lineRect.x0 = x;
        lineRect.y0 = y;
        lineRect.x1 = x + fontWidth(string);
        lineRect.y1 = y + rowHeight;
        if (showBackdrop)
        {
            ioPrintBoundedLabel(&backdropRect, x, y, colWhite, string);
            drewBackdrop = TRUE;
        }
        else
        {
            fontPrint(x, y, colWhite, string);
        }
        ioUnionRect(&ioShipCountPreviewRect, &lineRect, &ioShipCountPreviewRectActive);
        y += rowHeight;
        drewAnything = TRUE;
    }

    if (showBackdrop && drewBackdrop)
    {
        ioShipCountPreviewBackdropRect = backdropRect;
        ioShipCountPreviewBackdropRectActive = TRUE;
        ioUnionRect(&ioShipCountPreviewRect, &backdropRect, &ioShipCountPreviewRectActive);
    }
    else
    {
        ioShipCountPreviewBackdropRect.x0 = ioShipCountPreviewBackdropRect.y0 = 0;
        ioShipCountPreviewBackdropRect.x1 = ioShipCountPreviewBackdropRect.y1 = 0;
        ioShipCountPreviewBackdropRectActive = FALSE;
    }

    fontMakeCurrent(oldFont);
    return drewAnything;
}

void ioDrawMouseoverObjectTooltip(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    char *label;
    rectangle rect;
    color textColor;
    color backdropColor;
    color outlineColor;

    if (!pageFlags[IO_PageTooltips])
    {
        return;
    }

    if (mrRightMouseCameraMode || mrDisabled || smSensorsActiveGet())
    {
        return;
    }

    label = ioMouseoverLabelGet(mouseCursorObjPtr, &textColor, &backdropColor, &outlineColor);
    if (label == NULL)
    {
        return;
    }

    rect.x0 = mouseCursorX() + IO_TooltipCursorXOffset;
    rect.y0 = mouseCursorY() + IO_TooltipCursorYOffset;
    rect.x1 = rect.x0 + fontWidth(label) + 2 * IO_TooltipTextInset;
    rect.y1 = rect.y0 + fontHeight(label) + 2 * IO_TooltipTextInset;
    if (pageFlags[IO_PageTooltipsBackdrop])
    {
        primRectTranslucent2(&rect, backdropColor);
        primRoundRectOutline(&rect, IO_TooltipOutlineThickness, outlineColor, IO_TooltipOutlineCorner, IO_TooltipOutlineCorner);
    }
    fontPrint(rect.x0 + IO_TooltipTextInset, rect.y0 + IO_TooltipTextInset, textColor, label);
}

udword ioDrawPlayerStatusPanel(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    Player *player;
    fonthandle oldFont;
    rectangle rect;
    sdword rowHeight;
    sdword left;
    sdword valueLeft;
    sdword y;
    color supportColor;
    real32 supportUsageRatio;
    char string[128];

    if (!pageFlags[IO_PagePlayerInfo])
    {
        ioPlayerStatusPanelRect.x0 = ioPlayerStatusPanelRect.y0 = 0;
        ioPlayerStatusPanelRect.x1 = ioPlayerStatusPanelRect.y1 = 0;
        return FALSE;
    }

    player = universe.curPlayerPtr;
    if (player == NULL)
    {
        ioPlayerStatusPanelRect.x0 = ioPlayerStatusPanelRect.y0 = 0;
        ioPlayerStatusPanelRect.x1 = ioPlayerStatusPanelRect.y1 = 0;
        return FALSE;
    }

    oldFont = fontMakeCurrent(ioOverlayStatusFont);
    rowHeight = fontHeight(" ");
    sprintf(string, "%s 000 / 000", strGetString(strMO_SupportUnits));
    left = MAIN_WindowWidth - IO_PlayerStatusRightPadding - fontWidth(string);
    y = ioOverlayPanelY;

    rect.x0 = left - IO_PlayerStatusBackdropPadX;
    rect.x1 = MAIN_WindowWidth;
    rect.y0 = y - IO_PlayerStatusBackdropPadY;
    rect.y1 = MAIN_WindowHeight;
    ioPlayerStatusPanelRect = rect;

    if (pageFlags[IO_PagePlayerInfoBackdrop] && (!showTaskbarBackdrop || tutorial == 1))
    {
        primRectTranslucent2(&rect, IO_PlayerStatusBackdropColor);
    }

    sprintf(string, "%s ", strGetString(strMO_SupportUnits));
    fontPrint(left, y, IO_PlayerStatusLabelColor, string);
    valueLeft = left + fontWidth(string);

    supportUsageRatio = ((real32)player->supportUnitsUsed / (real32)player->supportUnitsMax) * 100.0f;
    if (supportUsageRatio > 100.0f)
    {
        supportColor = colRGB(255, 0, 0);
    }
    else if (supportUsageRatio > 80.0f)
    {
        supportColor = colRGB(255, 255, 0);
    }
    else
    {
        supportColor = colRGB(0, 255, 0);
    }
    sprintf(string, "%d", player->supportUnitsUsed);
    fontPrint(valueLeft, y, supportColor, string);
    valueLeft += fontWidth(string);

    sprintf(string, " / %d", player->supportUnitsMax);
    fontPrint(valueLeft, y, colRGB(255, 255, 255), string);

    y += rowHeight;
    sprintf(string, "%s ", strGetString(strMO_ResourceUnits));
    fontPrint(left, y, IO_PlayerStatusLabelColor, string);
    valueLeft = left + fontWidth(string);

    sprintf(string, "%d", player->resourceUnits);
    fontPrint(valueLeft, y, colRGB(255, 255, 255), string);

    y += rowHeight + IO_PlayerStatusTimeGap;
    sprintf(string,
            "%02d:%02d:%02d",
            (sdword)(universe.totaltimeelapsed - ioOverlayElapsedTime) / 3600,
            ((sdword)(universe.totaltimeelapsed - ioOverlayElapsedTime) / 60) % 60,
            (sdword)(universe.totaltimeelapsed - ioOverlayElapsedTime) % 60);
    fontPrint(left, y, colRGB(255, 255, 255), string);

    fontMakeCurrent(oldFont);
    return TRUE;
}

udword ioDrawMainOverlayButtons(void)
{
    fonthandle oldFont;
    char *buttonLabels[4];
    sdword leftX;
    sdword topY;
    rectangle rect;

    if (!showTaskbarButtons || tutorial == 1)
    {
        return FALSE;
    }

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    buttonLabels[0] = strGetString(smSensorsActive ? str_MOMainViewButton : str_MOSensorsButton);
    buttonLabels[1] = strGetString(str_MOBuildButton);
    buttonLabels[2] = strGetString(str_MOResearchButton);
    buttonLabels[3] = strGetString(str_MOSystemsButton);

    leftX = MAIN_WindowWidth / 2 - (6 + 2 * 90) / 2;
    topY = ioOverlayPanelY + 5;

    rect.x0 = leftX;
    rect.y0 = topY;
    rect.x1 = rect.x0 + 90;
    rect.y1 = rect.y0 + 20;
    ioMainOverlayButtonRects[0] = rect;
    ioDrawBoundedLabel(&rect, buttonLabels[0], -1, ioOverlayObjectivesFont, 15, 19, 22, 21, 24, 20, 23, TRUE);

    rect.y0 = rect.y1 + 3;
    rect.y1 = rect.y0 + 20;
    ioMainOverlayButtonRects[1] = rect;
    ioDrawBoundedLabel(&rect, buttonLabels[1], -1, ioOverlayObjectivesFont, 15, 19, 22, 21, 24, 20, 23, TRUE);

    rect.x0 = ioMainOverlayButtonRects[0].x1 + 6;
    rect.x1 = rect.x0 + 90;
    rect.y0 = ioMainOverlayButtonRects[0].y0;
    rect.y1 = ioMainOverlayButtonRects[0].y1;
    ioMainOverlayButtonRects[2] = rect;
    ioDrawBoundedLabel(&rect, buttonLabels[2], -1, ioOverlayObjectivesFont, 15, 19, 22, 21, 24, 20, 23, TRUE);

    rect.x0 = ioMainOverlayButtonRects[1].x1 + 6;
    rect.x1 = rect.x0 + 90;
    rect.y0 = ioMainOverlayButtonRects[1].y0;
    rect.y1 = ioMainOverlayButtonRects[1].y1;
    ioMainOverlayButtonRects[3] = rect;
    ioDrawBoundedLabel(&rect, buttonLabels[3], -1, ioOverlayObjectivesFont, 15, 19, 22, 21, 24, 20, 23, TRUE);

    fontMakeCurrent(oldFont);
    return TRUE;
}

void ioDrawEventFeed(void)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    fonthandle oldFont;
    rectangle rect;
    sdword rowHeight;
    sdword bottomY;
    sdword lineBottom;
    sdword visibleCount;
    sdword firstVisible;
    sdword index;
    sdword fadeStep;
    sdword y;
    sdword maxWidth;
    bool32 gameMessagesEnabled;

    ioEventFeedNewerVisible = FALSE;
    ioEventFeedOlderVisible = FALSE;
    if (!pageFlags[IO_PageGameMessage])
    {
        return;
    }

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    rowHeight = fontHeight(" ");
    bottomY = ioOverlayPanelTargetY;
    if (bottomY > MAIN_WindowHeight)
    {
        bottomY = MAIN_WindowHeight;
    }
    lineBottom = bottomY - rowHeight;
    gameMessagesEnabled = pageFlags[IO_PageGameMessage];

    if (battleChatMessageCount > 0)
    {
        if (battleChatCurrentIndex != battleChatMessageCount - 1 &&
            gameMessagesEnabled)
        {
            ioEventFeedNewerVisible = TRUE;
            ioEventFeedNewerRect.x0 = IO_EventFeedArrowLeft;
            ioEventFeedNewerRect.x1 = IO_EventFeedArrowSize - 3;
            ioEventFeedNewerRect.y0 = lineBottom - rowHeight;
            ioEventFeedNewerRect.y1 = ioEventFeedNewerRect.y0 + IO_EventFeedArrowHeight;
            if (mouseCursorX() < ioEventFeedNewerRect.x0 ||
                mouseCursorY() < ioEventFeedNewerRect.y0 ||
                mouseCursorX() >= ioEventFeedNewerRect.x1 ||
                mouseCursorY() >= ioEventFeedNewerRect.y1)
            {
                opOverlayIconDraw(OPI_Buffer_Down,
                                  ioEventFeedNewerRect.y0,
                                  ioEventFeedNewerRect.x0,
                                  ioEventFeedNewerRect.x1 - ioEventFeedNewerRect.x0,
                                  ioEventFeedNewerRect.y1 - ioEventFeedNewerRect.y0,
                                  colWhite);
            }
            else
            {
                opOverlayIconDraw(OPI_Buffer_Down_Mouse,
                                  ioEventFeedNewerRect.y0,
                                  ioEventFeedNewerRect.x0,
                                  ioEventFeedNewerRect.x1 - ioEventFeedNewerRect.x0,
                                  ioEventFeedNewerRect.y1 - ioEventFeedNewerRect.y0,
                                  colWhite);
            }
        }

        if (battleChatCurrentIndex - battleChatVisibleCount >= 0 &&
            gameMessagesEnabled)
        {
            ioEventFeedOlderVisible = TRUE;
            ioEventFeedOlderRect.x0 = IO_EventFeedArrowLeft;
            ioEventFeedOlderRect.x1 = IO_EventFeedArrowSize - 3;
            ioEventFeedOlderRect.y0 = lineBottom + (sdword)(rowHeight * IO_EventFeedBottomArrowScale + IO_EventFeedBottomArrowYOffset) -
                                      (IO_EventFeedBottomPadding + BATTLE_CHAT_MAX_VISIBLE * rowHeight);
            ioEventFeedOlderRect.y1 = ioEventFeedOlderRect.y0 + IO_EventFeedArrowHeight;
            if (mouseCursorX() < ioEventFeedOlderRect.x0 ||
                mouseCursorY() < ioEventFeedOlderRect.y0 ||
                mouseCursorX() >= ioEventFeedOlderRect.x1 ||
                mouseCursorY() >= ioEventFeedOlderRect.y1)
            {
                opOverlayIconDraw(OPI_Buffer_Up,
                                  ioEventFeedOlderRect.y0,
                                  ioEventFeedOlderRect.x0,
                                  ioEventFeedOlderRect.x1 - ioEventFeedOlderRect.x0,
                                  ioEventFeedOlderRect.y1 - ioEventFeedOlderRect.y0,
                                  colWhite);
            }
            else
            {
                opOverlayIconDraw(OPI_Buffer_Up_Mouse,
                                  ioEventFeedOlderRect.y0,
                                  ioEventFeedOlderRect.x0,
                                  ioEventFeedOlderRect.x1 - ioEventFeedOlderRect.x0,
                                  ioEventFeedOlderRect.y1 - ioEventFeedOlderRect.y0,
                                  colWhite);
            }
        }

        battleChatLock();
        fadeStep = 0;
        index = battleChatCurrentIndex;
        while (index > battleChatCurrentIndex - battleChatVisibleCount)
        {
            BattleChatEntry *entry = &battleChatEntries[index];
            color drawColor;

            entry->rect.x0 = IO_EventFeedTextX;
            entry->rect.y0 = lineBottom - rowHeight;
            entry->rect.x1 = entry->rect.x0 + fontWidth(entry->message);
            entry->rect.y1 = lineBottom;

            if (gameMessagesEnabled)
            {
                if (entry->highlighted)
                {
                    drawColor = IO_EventFeedNewestColor;
                    if (entry->color == IO_EventFeedNewestColor)
                    {
                        drawColor = 0xFFFFFFFF;
                    }
                }
                else
                {
                    real32 factor;

                    switch (fadeStep)
                    {
                        case 0: factor = 1.0f; break;
                        case 1: factor = 0.8f; break;
                        case 2: factor = 0.7f; break;
                        case 3: factor = 0.6f; break;
                        case 4: factor = 0.5f; break;
                        default: factor = 0.0f; break;
                    }
                    drawColor = colBlend(entry->color, 0xFF000000, factor);
                }

                if (battleChatCurrentIndex == battleChatMessageCount - 1 &&
                    index == battleChatCurrentIndex - battleChatVisibleCount + 1)
                {
                    real32 elapsed;
                    real32 flashFactor = 1.0f;

                    if (entry->flashStartTime == 0.0f)
                    {
                        entry->flashStartTime = taskTimeElapsed;
                    }

                    elapsed = taskTimeElapsed - entry->flashStartTime;
                    if (elapsed > IO_EventFeedFlashDelay)
                    {
                        flashFactor = 1.0f - ((elapsed - IO_EventFeedFlashDelay) / IO_EventFeedFlashDuration);
                        if (flashFactor < 0.0f)
                        {
                            flashFactor = 0.0f;
                        }
                    }
                    drawColor = colBlendRGBA(drawColor, IO_EventFeedFadeToColor, flashFactor);
                    if (elapsed > IO_EventFeedFlashDelay + IO_EventFeedFlashDuration)
                    {
                        battleChatVisibleCount--;
                    }
                    else
                    {
                        fontPrint(entry->rect.x0, entry->rect.y0, drawColor, entry->message);
                    }
                }
                else
                {
                    fontPrint(entry->rect.x0, entry->rect.y0, drawColor, entry->message);
                }
            }

            lineBottom -= rowHeight;
            index--;
            fadeStep++;
        }
        battleChatUnlock();
        fontMakeCurrent(oldFont);
        return;
    }

    if (ioEventFeedCount > 0)
    {
        visibleCount = min(ioEventFeedCount, IO_EventFeedVisibleRows);
        firstVisible = ioEventFeedCount - visibleCount;
        y = MAIN_WindowHeight - 48 - rowHeight * visibleCount;
        maxWidth = 0;

        for (index = firstVisible; index < ioEventFeedCount; index++)
        {
            maxWidth = max(maxWidth, fontWidth(ioEventFeedMessages[index].message));
        }

        rect.x0 = 12;
        rect.y0 = y - 6;
        rect.x1 = rect.x0 + maxWidth + 16;
        rect.y1 = MAIN_WindowHeight - 48 + 6;

        if (pageFlags[IO_PageGameMessageBackdrop])
        {
            ioDrawPanelBackground(&rect);
        }

        for (index = firstVisible; index < ioEventFeedCount; index++)
        {
            fontPrint(rect.x0 + 8, y, ioEventFeedMessages[index].messageColor, ioEventFeedMessages[index].message);
            y += rowHeight;
        }
    }

    fontMakeCurrent(oldFont);
}

sdword __cdecl ioDrawPlayerResourceStats(int y)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    fonthandle oldFont;
    sdword rowHeight;
    sdword swatchWidth;
    sdword baseX;
    sdword textX;
    sdword topY;
    sdword playerIndex;
    Player *player;
    color textColor;
    rectangle swatchRect;
    rectangle detailRect;
    rectangle backdropRect;
    sdword maxRight;
    sdword bountyValue;
    char bountyString[32];
    char string[128];

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    rowHeight = fontHeight(" ");
    swatchWidth = (2 * rowHeight) / 3;
    baseX = 10;
    textX = baseX + rowHeight + 4;
    topY = y + 20;
    y = topY;

    if (!pageFlags[IO_PagePlayerNames])
    {
        fontMakeCurrent(oldFont);
        return y;
    }

    backdropRect.x0 = baseX - 5;
    backdropRect.y0 = topY - 5;
    backdropRect.x1 = baseX - 5;
    backdropRect.y1 = topY - 5;
    maxRight = baseX + rowHeight;
    rndAdditiveBlends(FALSE);

    for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++)
    {
        player = &universe.players[playerIndex];
        if (player->playerState == PLAYER_DEAD)
        {
            textColor = colRGB(100, 100, 100);
        }
        else if (playerIndex == sigsPlayerIndex)
        {
            textColor = teFriendlyColor;
        }
        else if (allianceArePlayersAllied(&universe.players[sigsPlayerIndex], player))
        {
            textColor = teAlliedColor;
        }
        else
        {
            textColor = teHostileColor;
        }

        sprintf(string, "%s..%d", playerNames[playerIndex],
                universe.gameStats.playerStats[playerIndex].totalResourceUnitsSpent);

        swatchRect.x0 = baseX;
        swatchRect.x1 = baseX + swatchWidth;
        swatchRect.y0 = y;
        swatchRect.y1 = y + rowHeight;
        primRectSolid2(&swatchRect, teColorSchemes[playerIndex].textureColor.base);

        detailRect.x0 = swatchRect.x1;
        detailRect.x1 = baseX + rowHeight;
        detailRect.y0 = y;
        detailRect.y1 = y + rowHeight;
        primRectSolid2(&detailRect, teColorSchemes[playerIndex].textureColor.detail);

        fontPrint(textX, y, textColor, string);
        if (pageFlags[IO_PagePlayerNamesBackdrop])
        {
            if (textX + fontWidth(string) + 5 > backdropRect.x1)
            {
                backdropRect.x1 = textX + fontWidth(string) + 5;
            }
            if (y + rowHeight + 5 > backdropRect.y1)
            {
                backdropRect.y1 = y + rowHeight + 5;
            }
        }
        if (!singlePlayerGame && tpGameCreated.bountySize != MG_BountiesOff)
        {
            bountyValue = (sdword)getPlayerBountyWorth(playerIndex, player->aiPlayer != NULL) / 1000;
            sprintf(bountyString, " ( %d )", bountyValue);
            fontPrint(textX + fontWidth(string), y, colWhite, bountyString);
            if (pageFlags[IO_PagePlayerNamesBackdrop] &&
                textX + fontWidth(string) + fontWidth(bountyString) + 5 > backdropRect.x1)
            {
                backdropRect.x1 = textX + fontWidth(string) + fontWidth(bountyString) + 5;
            }
        }
        if (baseX + rowHeight > maxRight)
        {
            maxRight = baseX + rowHeight;
        }

        y += rowHeight + 2;
    }

    if (pageFlags[IO_PagePlayerNamesBackdrop])
    {
        if (backdropRect.x1 < maxRight + 5)
        {
            backdropRect.x1 = maxRight + 5;
        }
        if (backdropRect.y1 < topY + rowHeight + 5)
        {
            backdropRect.y1 = topY + rowHeight + 5;
        }
        primRectTranslucent2(&backdropRect, 0x64323232);

        y = topY;
        for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++)
        {
            player = &universe.players[playerIndex];
            if (player->playerState == PLAYER_DEAD)
            {
                textColor = colRGB(100, 100, 100);
            }
            else if (playerIndex == sigsPlayerIndex)
            {
                textColor = teFriendlyColor;
            }
            else if (allianceArePlayersAllied(&universe.players[sigsPlayerIndex], player))
            {
                textColor = teAlliedColor;
            }
            else
            {
                textColor = teHostileColor;
            }

            sprintf(string, "%s..%d", playerNames[playerIndex],
                    universe.gameStats.playerStats[playerIndex].totalResourceUnitsSpent);
            swatchRect.x0 = baseX;
            swatchRect.x1 = baseX + swatchWidth;
            swatchRect.y0 = y;
            swatchRect.y1 = y + rowHeight;
            primRectSolid2(&swatchRect, teColorSchemes[playerIndex].textureColor.base);
            detailRect.x0 = swatchRect.x1;
            detailRect.x1 = baseX + rowHeight;
            detailRect.y0 = y;
            detailRect.y1 = y + rowHeight;
            primRectSolid2(&detailRect, teColorSchemes[playerIndex].textureColor.detail);
            fontPrint(textX, y, textColor, string);
            if (!singlePlayerGame && tpGameCreated.bountySize != MG_BountiesOff)
            {
                bountyValue = (sdword)getPlayerBountyWorth(playerIndex, player->aiPlayer != NULL) / 1000;
                sprintf(bountyString, " ( %d )", bountyValue);
                fontPrint(textX + fontWidth(string), y, colWhite, bountyString);
            }
            y += rowHeight + 2;
        }
    }

    fontMakeCurrent(oldFont);
    return y;
}

int __cdecl ioDrawPathMarkerLegend(int y)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    fonthandle oldFont;

    if (!pageFlags[IO_PageViewOptions])
    {
        return y;
    }

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    fontPrint(MAIN_WindowWidth - 220, y, colRGB(255, 255, 255), "Path markers follow current orders.");
    y += fontHeight(" ");
    fontMakeCurrent(oldFont);
    return y;
}

sdword __cdecl ioDrawBuildInProgressPanel(sdword topY, sdword bottomY)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    sdword buildSummary[(TOTAL_NUM_SHIPS + 1) * 10];
    InfoOverlayProgressPanelState *state;
    fonthandle oldFont;
    sdword rowHeight;
    sdword drawTopY;
    sdword drawBottomY;
    sdword buildCount;
    sdword firstVisible;
    sdword lastVisible;
    sdword pageSize;
    sdword pageCount;
    sdword index;
    sdword x;
    sdword y;
    sdword primarySlot;
    sdword buildTime;
    sdword timeLeft;
    sdword activeSlots[4];
    sdword activeSlotCount;
    sdword slotIndex;
    sdword rowBottomY;
    sdword progressLeft;
    rectangle fillRect;
    rectangle rowRect;
    rectangle labelRect;
    triangle tri;
    bool32 showBackdrop;
    char string[256];
    Ship *playerMothership;
    state = &ioBuildProgressPanelState;
    if (!pageFlags[IO_PageConstruction] || universe.curPlayerPtr == NULL)
    {
        ioResetProgressPanelState(state);
        return topY;
    }

    state->panelRect.x0 = 0;
    state->panelRect.y0 = 0;
    state->panelRect.x1 = 0;
    state->panelRect.y1 = 0;
    state->panelRectActive = FALSE;
    state->topArrowRect.x0 = 0;
    state->topArrowRect.y0 = 0;
    state->topArrowRect.x1 = 0;
    state->topArrowRect.y1 = 0;
    state->bottomArrowRect.x0 = 0;
    state->bottomArrowRect.y0 = 0;
    state->bottomArrowRect.x1 = 0;
    state->bottomArrowRect.y1 = 0;
    state->topArrowVisible = FALSE;
    state->bottomArrowVisible = FALSE;
    state->topArrowHighlighted = FALSE;
    state->bottomArrowHighlighted = FALSE;

    oldFont = fontMakeCurrent(ioOverlayShipCountFont);
    rowHeight = fontHeight(" ");
    x = IO_ProgressPanelX;
    drawTopY = topY + IO_ProgressPanelGap + IO_ProgressPanelTopInset;
    drawBottomY = bottomY - IO_ProgressPanelGap;
    y = drawTopY;
    showBackdrop = pageFlags[IO_PageConstructionBackdrop];

    if (showBackdrop && state->textBoundsActive)
    {
        primRectTranslucent2(&state->textBounds, IO_ProgressPanelBackdropColor);
    }
    state->textBounds.x0 = x;
    state->textBounds.y0 = drawTopY;
    state->textBounds.x1 = x;
    state->textBounds.y1 = drawTopY;
    state->textBoundsActive = FALSE;

    memset(buildSummary, 0, sizeof(buildSummary));
    buildCount = cmCollectPlayerBuildTypesInProgress(universe.curPlayerPtr, buildSummary);
    if (buildCount <= 0 || drawBottomY <= drawTopY)
    {
        fontMakeCurrent(oldFont);
        return topY;
    }

    if (y + buildCount * rowHeight <= drawBottomY)
    {
        firstVisible = 0;
        lastVisible = buildCount;
    }
    else
    {
        pageSize = (drawBottomY - y) / rowHeight;
        if (pageSize < 1)
        {
            pageSize = 1;
        }
        pageCount = (buildCount + pageSize - 1) / pageSize;
        if (state->page >= pageCount)
        {
            state->page = pageCount - 1;
        }
        if (state->page < 0)
        {
            state->page = 0;
        }
        firstVisible = pageSize * state->page;
        lastVisible = firstVisible + pageSize;
        if (lastVisible > buildCount)
        {
            lastVisible = buildCount;
        }
        state->topArrowVisible = (state->page > 0);
        state->bottomArrowVisible = (state->page + 1 < pageCount);
        state->topArrowRect.x0 = IO_ProgressPanelArrowX;
        state->topArrowRect.y0 = drawTopY - IO_ProgressPanelGap;
        state->topArrowRect.x1 = IO_ProgressPanelArrowX + IO_ProgressPanelArrowWidth;
        state->topArrowRect.y1 = drawTopY;
        state->bottomArrowRect.x0 = IO_ProgressPanelArrowX;
        state->bottomArrowRect.y0 = bottomY - IO_ProgressPanelGap;
        state->bottomArrowRect.x1 = IO_ProgressPanelArrowX + IO_ProgressPanelArrowWidth;
        state->bottomArrowRect.y1 = bottomY;
    }

    playerMothership = universe.curPlayerPtr->PlayerMothership;
    primarySlot = 0;
    if (playerMothership != NULL && playerMothership->staticinfo->shipclass != CLASS_Mothership)
    {
        primarySlot = 1;
    }

    for (index = firstVisible; index < lastVisible; index++)
    {
        sdword *summary = &buildSummary[index * 10];

        if (summary[1 + primarySlot] <= 0)
        {
            continue;
        }

        buildTime = summary[5];
        timeLeft = summary[6 + primarySlot];
        rowBottomY = y + rowHeight - 1;
        progressLeft = x;
        if (buildTime > 0)
        {
            progressLeft = x + (sdword)(((real32)(buildTime - timeLeft) / (real32)buildTime) * (real32)IO_ProgressPanelBarWidth);
        }

        fillRect.x0 = progressLeft;
        fillRect.y0 = y - 1;
        fillRect.x1 = x + IO_ProgressPanelBarWidth;
        fillRect.y1 = rowBottomY;
        glDisable(GL_BLEND);
        opOverlayIconDraw(OPI_Build_ProgressBar, y - 1, x, IO_ProgressPanelBarWidth, rowBottomY - (y - 1), colWhite);
        glEnable(GL_BLEND);
        rndAdditiveBlends(FALSE);
        if (!RGL || RGLtype)
        {
            primRectTranslucent2(&fillRect, IO_ProgressPanelFillColor);
        }
        else
        {
            primRectSolid2(&fillRect, colBlack);
        }

        rowRect.x0 = x;
        rowRect.y0 = y - 1;
        rowRect.x1 = x + IO_ProgressPanelBarWidth;
        rowRect.y1 = rowBottomY;
        ioUnionRect(&state->panelRect, &rowRect, &state->panelRectActive);

        activeSlotCount = 0;
        for (slotIndex = 0; slotIndex < 4; slotIndex++)
        {
            if (summary[1 + slotIndex] > 0)
            {
                activeSlots[activeSlotCount] = slotIndex;
                activeSlotCount++;
            }
        }

        if (activeSlotCount == 1)
        {
            sprintf(string, "%s (%d)", ShipTypeToNiceStr(summary[0]), summary[1 + activeSlots[0]]);
        }
        else if (activeSlotCount == 2)
        {
            sprintf(string, "%s (%d/%d)", ShipTypeToNiceStr(summary[0]),
                    summary[1 + activeSlots[0]], summary[1 + activeSlots[1]]);
        }
        else if (activeSlotCount == 3)
        {
            sprintf(string, "%s (%d/%d/%d)", ShipTypeToNiceStr(summary[0]),
                    summary[1 + activeSlots[0]], summary[1 + activeSlots[1]], summary[1 + activeSlots[2]]);
        }
        else if (activeSlotCount >= 4)
        {
            sprintf(string, "%s (%d/%d/%d/%d)", ShipTypeToNiceStr(summary[0]),
                    summary[1 + activeSlots[0]], summary[1 + activeSlots[1]],
                    summary[1 + activeSlots[2]], summary[1 + activeSlots[3]]);
        }
        else
        {
            string[0] = '\0';
        }

        if (string[0] != '\0')
        {
            if (showBackdrop)
            {
                ioPrintBoundedLabel(&state->textBounds, x, y, IO_BuildProgressTextColor, string);
                state->textBoundsActive = TRUE;
            }
            else
            {
                fontPrint(x, y, IO_BuildProgressTextColor, string);
            }

            labelRect.x0 = x;
            labelRect.y0 = y;
            labelRect.x1 = x + fontWidth(string);
            labelRect.y1 = y + rowHeight;
            ioUnionRect(&state->panelRect, &labelRect, &state->panelRectActive);
        }

        y += rowHeight;
    }

    if (state->textBoundsActive)
    {
        ioUnionRect(&state->panelRect, &state->textBounds, &state->panelRectActive);
    }

    glEnable(GL_BLEND);
    rndAdditiveBlends(TRUE);
    if (state->topArrowVisible)
    {
        state->topArrowHighlighted = ioPointInRect(mouseCursorX(), mouseCursorY(), &state->topArrowRect);
        ioScrollArrowTrianglePointsGet(FALSE, &state->topArrowRect, &tri);
        if (state->topArrowHighlighted)
        {
            primTriSolid2(&tri, IO_ProgressArrowFillColor);
        }
        primTriOutline2(&tri, 1, IO_ProgressArrowOutlineColor);
        ioUnionRect(&state->panelRect, &state->topArrowRect, &state->panelRectActive);
    }
    if (state->bottomArrowVisible)
    {
        state->bottomArrowHighlighted = ioPointInRect(mouseCursorX(), mouseCursorY(), &state->bottomArrowRect);
        ioScrollArrowTrianglePointsGet(TRUE, &state->bottomArrowRect, &tri);
        if (state->bottomArrowHighlighted)
        {
            primTriSolid2(&tri, IO_ProgressArrowFillColor);
        }
        primTriOutline2(&tri, 1, IO_ProgressArrowOutlineColor);
        ioUnionRect(&state->panelRect, &state->bottomArrowRect, &state->panelRectActive);
    }

    fontMakeCurrent(oldFont);
    return y;
}

sdword __cdecl ioDrawResearchProgressPanel(sdword topY, sdword bottomY)
{
    sdword *pageFlags = ioCurrentOverlayPageFlags();
    InfoOverlayResearchProgressEntry researchList[NumTechnologies];
    InfoOverlayProgressPanelState *state;
    fonthandle oldFont;
    sdword rowHeight;
    sdword drawTopY;
    sdword drawBottomY;
    sdword researchCount;
    sdword pageSize;
    sdword pageCount;
    sdword firstVisible;
    sdword lastVisible;
    sdword index;
    sdword x;
    sdword y;
    sdword rowBottomY;
    sdword progressLeft;
    rectangle fillRect;
    rectangle rowRect;
    rectangle labelRect;
    triangle tri;
    bool32 showBackdrop;
    char string[256];

    state = &ioResearchProgressPanelState;
    if (!pageFlags[IO_PageResearch] || universe.curPlayerPtr == NULL)
    {
        ioResetProgressPanelState(state);
        return topY;
    }

    state->panelRect.x0 = 0;
    state->panelRect.y0 = 0;
    state->panelRect.x1 = 0;
    state->panelRect.y1 = 0;
    state->panelRectActive = FALSE;
    state->topArrowRect.x0 = 0;
    state->topArrowRect.y0 = 0;
    state->topArrowRect.x1 = 0;
    state->topArrowRect.y1 = 0;
    state->bottomArrowRect.x0 = 0;
    state->bottomArrowRect.y0 = 0;
    state->bottomArrowRect.x1 = 0;
    state->bottomArrowRect.y1 = 0;
    state->topArrowVisible = FALSE;
    state->bottomArrowVisible = FALSE;
    state->topArrowHighlighted = FALSE;
    state->bottomArrowHighlighted = FALSE;

    oldFont = fontMakeCurrent(ioOverlayObjectivesFont);
    rowHeight = fontHeight(" ");
    x = IO_ProgressPanelX;
    drawTopY = topY + IO_ProgressPanelGap + IO_ProgressPanelTopInset;
    drawBottomY = bottomY - IO_ProgressPanelGap;
    y = drawTopY;
    showBackdrop = pageFlags[IO_PageResearchBackdrop];

    if (showBackdrop && state->textBoundsActive)
    {
        primRectTranslucent2(&state->textBounds, IO_ProgressPanelBackdropColor);
    }
    state->textBounds.x0 = x;
    state->textBounds.y0 = drawTopY;
    state->textBounds.x1 = x;
    state->textBounds.y1 = drawTopY;
    state->textBoundsActive = FALSE;

    memset(researchList, 0, sizeof(researchList));
    researchCount = ioBuildCurrentResearchProgressList(universe.curPlayerPtr, researchList);
    if (researchCount <= 0 || drawBottomY <= drawTopY)
    {
        fontMakeCurrent(oldFont);
        return topY;
    }

    if (y + researchCount * rowHeight <= drawBottomY)
    {
        firstVisible = 0;
        lastVisible = researchCount;
    }
    else
    {
        pageSize = (drawBottomY - y) / rowHeight;
        if (pageSize < 1)
        {
            pageSize = 1;
        }
        pageCount = (researchCount + pageSize - 1) / pageSize;
        if (state->page >= pageCount)
        {
            state->page = pageCount - 1;
        }
        if (state->page < 0)
        {
            state->page = 0;
        }
        firstVisible = pageSize * state->page;
        lastVisible = firstVisible + pageSize;
        if (lastVisible > researchCount)
        {
            lastVisible = researchCount;
        }
        state->topArrowVisible = (state->page > 0);
        state->bottomArrowVisible = (state->page + 1 < pageCount);
        state->topArrowRect.x0 = IO_ProgressPanelArrowX;
        state->topArrowRect.y0 = drawTopY - IO_ProgressPanelGap;
        state->topArrowRect.x1 = IO_ProgressPanelArrowX + IO_ProgressPanelArrowWidth;
        state->topArrowRect.y1 = drawTopY;
        state->bottomArrowRect.x0 = IO_ProgressPanelArrowX;
        state->bottomArrowRect.y0 = bottomY - IO_ProgressPanelGap;
        state->bottomArrowRect.x1 = IO_ProgressPanelArrowX + IO_ProgressPanelArrowWidth;
        state->bottomArrowRect.y1 = bottomY;
    }

    for (index = firstVisible; index < lastVisible; index++)
    {
        rowBottomY = y + rowHeight - 3;
        progressLeft = x;
        if (researchList[index].totalTime > 0.0f)
        {
            progressLeft = x + (sdword)(((researchList[index].totalTime - researchList[index].timeLeft) /
                                         researchList[index].totalTime) * (real32)IO_ProgressPanelBarWidth);
        }

        fillRect.x0 = progressLeft;
        fillRect.y0 = y - 1;
        fillRect.x1 = x + IO_ProgressPanelBarWidth;
        fillRect.y1 = rowBottomY;
        glDisable(GL_BLEND);
        opOverlayIconDraw(OPI_Research_ProgressBar, y - 1, x, IO_ProgressPanelBarWidth, rowBottomY - (y - 1), colWhite);
        glEnable(GL_BLEND);
        rndAdditiveBlends(FALSE);
        if (!RGL || RGLtype)
        {
            primRectTranslucent2(&fillRect, IO_ProgressPanelFillColor);
        }
        else
        {
            primRectSolid2(&fillRect, colBlack);
        }

        rowRect.x0 = x;
        rowRect.y0 = y - 1;
        rowRect.x1 = x + IO_ProgressPanelBarWidth;
        rowRect.y1 = rowBottomY;
        ioUnionRect(&state->panelRect, &rowRect, &state->panelRectActive);

        sprintf(string, "%s", TechTypeToNiceString(researchList[index].tech));
        if (showBackdrop)
        {
            ioPrintBoundedLabel(&state->textBounds, x, y, IO_ResearchProgressTextColor, string);
            state->textBoundsActive = TRUE;
        }
        else
        {
            fontPrint(x, y, IO_ResearchProgressTextColor, string);
        }

        labelRect.x0 = x;
        labelRect.y0 = y;
        labelRect.x1 = x + fontWidth(string);
        labelRect.y1 = y + rowHeight;
        ioUnionRect(&state->panelRect, &labelRect, &state->panelRectActive);
        y += rowHeight;
    }

    if (state->textBoundsActive)
    {
        ioUnionRect(&state->panelRect, &state->textBounds, &state->panelRectActive);
    }

    glEnable(GL_BLEND);
    rndAdditiveBlends(TRUE);
    if (state->topArrowVisible)
    {
        state->topArrowHighlighted = ioPointInRect(mouseCursorX(), mouseCursorY(), &state->topArrowRect);
        ioScrollArrowTrianglePointsGet(FALSE, &state->topArrowRect, &tri);
        if (state->topArrowHighlighted)
        {
            primTriSolid2(&tri, IO_ProgressArrowFillColor);
        }
        primTriOutline2(&tri, 1, IO_ProgressArrowOutlineColor);
        ioUnionRect(&state->panelRect, &state->topArrowRect, &state->panelRectActive);
    }
    if (state->bottomArrowVisible)
    {
        state->bottomArrowHighlighted = ioPointInRect(mouseCursorX(), mouseCursorY(), &state->bottomArrowRect);
        ioScrollArrowTrianglePointsGet(TRUE, &state->bottomArrowRect, &tri);
        if (state->bottomArrowHighlighted)
        {
            primTriSolid2(&tri, IO_ProgressArrowFillColor);
        }
        primTriOutline2(&tri, 1, IO_ProgressArrowOutlineColor);
        ioUnionRect(&state->panelRect, &state->bottomArrowRect, &state->panelRectActive);
    }

    fontMakeCurrent(oldFont);
    return y;
}


void __cdecl ioDrawKeyBindingsColumn(int *categories, int x)
{
    fonthandle oldFont;
    sdword y;
    sdword index;

    oldFont = fontMakeCurrent(ioOverlayDebugFont);
    y = 54;

    for (index = 0; categories[index] >= 0; index++)
    {
        fontPrint(x, y, colRGB(255, 255, 255),
                  (categories[index] == 0) ? "Movement" :
                  (categories[index] == 1) ? "Selection" :
                  (categories[index] == 2) ? "Combat" :
                  (categories[index] == 3) ? "Sensors" :
                  (categories[index] == 4) ? "Tactical Overlay" :
                  (categories[index] == 5) ? "Construction" :
                  (categories[index] == 6) ? "Research" :
                  (categories[index] == 7) ? "Unit Orders" :
                  (categories[index] == 8) ? "Formations" :
                  (categories[index] == 9) ? "View" :
                  "Misc");
        y += fontHeight(" ") + 2;
    }

    fontMakeCurrent(oldFont);
}

udword ioDrawDebugKeyBindingsOverlay(void)
{
    static int leftCategories[] = {0, 1, 2, 3, -1};
    static int middleCategories[] = {4, 5, 6, 7, -1};
    static int rightCategories[] = {8, 9, 10, -1};
    fonthandle oldFont;

    oldFont = fontMakeCurrent(ioOverlayDebugFont);
    fontPrint(24, 24, colRGB(255, 255, 0), "Key Binding Categories");
    fontMakeCurrent(oldFont);

    ioDrawKeyBindingsColumn(leftCategories, 24);
    ioDrawKeyBindingsColumn(middleCategories, MAIN_WindowWidth / 3);
    ioDrawKeyBindingsColumn(rightCategories, (2 * MAIN_WindowWidth) / 3);
    return TRUE;
}

int __cdecl ioHandleOverlayClick(int pressed)
{
    sdword *pageFlags;
    int primaryHit;
    int secondaryHit;
    int tertiaryHit;
    int primaryHitSaved;
    int hitShiptype;
    bool32 handled;
    sdword n;
    sdword x, y;
    Ship *ship;
    sdword *pageConfig;
    sdword *item;
    sdword itemType;
    sdword curIdx;
    sdword endIdx;

    primaryHit = 0;
    secondaryHit = 0;
    handled = FALSE;
    pageFlags = ioCurrentOverlayPageFlags();

    if (piePointSpecMode || (IsTabStatsEnabled() && kbKeyIsHit(98)) || byte_A2DCD4 || smSensorsActiveGet() || mrDisabled)
        return FALSE;

    x = screenX;
    y = screenY;
    hitShiptype = -1;
    tertiaryHit = 0;
    primaryHitSaved = 0;

    /* Ship-type list click */
    for (n = 0; n < TOTAL_NUM_SHIPS; n++)
    {
        if (ioShipCountPrimary[n].enabled == TRUE &&
            x >= ioShipCountPrimary[n].rect.x0 && y >= ioShipCountPrimary[n].rect.y0 &&
            x < ioShipCountPrimary[n].rect.x1 && y < ioShipCountPrimary[n].rect.y1)
        {
            primaryHit = 1;
            hitShiptype = n;
            primaryHitSaved = 1;
        }
        if (ioShipCountSecondary[n].enabled == TRUE &&
            x >= ioShipCountSecondary[n].rect.x0 && y >= ioShipCountSecondary[n].rect.y0 &&
            x < ioShipCountSecondary[n].rect.x1 && y < ioShipCountSecondary[n].rect.y1)
        {
            primaryHit = primaryHitSaved;
            hitShiptype = n;
            secondaryHit = 1;
        }
        if (ioShipCountTertiary[n].enabled == TRUE &&
            x >= ioShipCountTertiary[n].rect.x0 && y >= ioShipCountTertiary[n].rect.y0 &&
            x < ioShipCountTertiary[n].rect.x1 && y < ioShipCountTertiary[n].rect.y1)
        {
            hitShiptype = n;
            tertiaryHit = 1;
        }
        if (hitShiptype != -1)
        {
            if (pressed)
            {
                if (primaryHit)
                    ioShipCountPrimary[hitShiptype].pressed = TRUE;
                else if (secondaryHit)
                    ioShipCountSecondary[hitShiptype].pressed = TRUE;
                else if (tertiaryHit)
                    ioShipCountTertiary[hitShiptype].pressed = TRUE;
            }
            else
            {
                sdword si = 0;
                while (si < selSelected.numShips)
                {
                    ship = selSelected.ShipPtr[si];
                    if (ship->shiptype == hitShiptype &&
                        (primaryHit || shipHasCataBusyState(ship)) &&
                        (secondaryHit || !shipHasCataBusyState(ship)))
                    {
                        si++;
                    }
                    else
                    {
                        selSelectionRemoveSingleShip((MaxSelection *)&selSelected, selSelected.ShipPtr[si]);
                        si = 0;
                    }
                }
            }
            handled = TRUE;
            ioUpdateShipTotals();
            tutGameMessage("ShipListClick");
            if (selSelected.numShips > 0)
                speechEventQueue(selSelected.ShipPtr[0], 67110912, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
            x = screenX;
            y = screenY;
            break;
        }
    }

    /* Objectives click */
    if (!overlayPauseActive)
    {
        pageConfig = singlePlayerGame ? &ioBuildQueueConfigSP[ioBuildQueuePageIndex][0]
                                      : &ioBuildQueueConfig[ioBuildQueuePageIndex][0];
        if (pageFlags[IO_PageObjectives])
        {
            if (!handled)
            {
                for (n = 0; n < ioObjectiveCount; n++)
                {
                    if (x >= ioObjectiveEntries[n].rect.x0 && y >= ioObjectiveEntries[n].rect.y0 &&
                        x < ioObjectiveEntries[n].rect.x1 && y < ioObjectiveEntries[n].rect.y1)
                    {
                        soundEventPlay(0, 265, 0);
                        if (_strcmpi(ioObjectiveEntries[n].objective->label, "Hyperspace") != 0)
                            poPlayerObjectivesBegin(ioObjectiveEntries[n].objective);
                        else
                            kbInvokeCommandCallbacks(0, 8);
                        x = screenX;
                        y = screenY;
                        handled = TRUE;
                    }
                    if (handled)
                        break;
                }
            }
        }
    }

    /* Hotkey areas */
    if (!handled)
    {
        for (n = 0; n < IO_NUM_HOTKEY_AREAS; n++)
        {
            if (x >= ioHotkeyAreas[n].x0 && y >= ioHotkeyAreas[n].y0 &&
                x < ioHotkeyAreas[n].x1 && y < ioHotkeyAreas[n].y1)
            {
                kbInvokeCommandCallbacks(smSensorsActive != 0, ioHotkeyAreas[n].commandId);
                x = screenX;
                y = screenY;
            }
        }
    }

    /* Build queue left-scroll */
    if (ioBuildScrollLeftEnabled)
    {
        if (x >= ioBuildScrollLeftRect.x0 && y >= ioBuildScrollLeftRect.y0 &&
            x < ioBuildScrollLeftRect.x1 && y < ioBuildScrollLeftRect.y1)
        {
            handled = TRUE;
            ioBuildScrollLeftPressed = TRUE;
            ioBuildScrollPage--;
            goto scrollDone;
        }
        ioBuildScrollLeftPressed = FALSE;
    }
    /* Build queue right-scroll */
    if (ioBuildScrollRightEnabled)
    {
        if (x >= ioBuildScrollRightRect.x0 && y >= ioBuildScrollRightRect.y0 &&
            x < ioBuildScrollRightRect.x1 && y < ioBuildScrollRightRect.y1)
        {
            handled = TRUE;
            ioBuildScrollRightPressed = TRUE;
            ioBuildScrollPage++;
            goto scrollDone;
        }
        ioBuildScrollRightPressed = FALSE;
    }
    /* Construction scroll up */
    if (ioConstructScrollUpEnabled)
    {
        if (x >= ioConstructScrollUpRect.x0 && y >= ioConstructScrollUpRect.y0 &&
            x < ioConstructScrollUpRect.x1 && y < ioConstructScrollUpRect.y1)
        {
            handled = TRUE;
            ioConstructScrollUpPressed = TRUE;
            ioConstructScrollOffset = ioBuildQueueScrollIndex - 1;
            ioBuildQueueScrollIndex = ioConstructScrollOffset;
            goto scrollDone;
        }
        ioConstructScrollUpPressed = FALSE;
    }
    /* Construction scroll down */
    if (ioConstructScrollDownEnabled)
    {
        if (x >= ioConstructScrollDownRect.x0 && y >= ioConstructScrollDownRect.y0 &&
            x < ioConstructScrollDownRect.x1 && y < ioConstructScrollDownRect.y1)
        {
            handled = TRUE;
            ioConstructScrollDownPressed = TRUE;
            ioConstructScrollOffset = ioBuildQueueScrollIndex + 1;
            ioBuildQueueScrollIndex = ioConstructScrollOffset;
            goto scrollDone;
        }
        ioConstructScrollDownPressed = FALSE;
    }
    goto afterScrollButtons;
scrollDone:
    handled = TRUE;
afterScrollButtons:

    /* Construction panel items */
    pageConfig = singlePlayerGame ? &ioBuildQueueConfigSP[ioBuildQueuePageIndex][0]
                                  : &ioBuildQueueConfig[ioBuildQueuePageIndex][0];
    if (pageConfig[12])
    {
        if (handled)
            goto afterConstruct;
        curIdx = ioBuildQueueScrollIndex;
        endIdx = ioBuildQueueScrollIndex;
        if (ioBuildQueueScrollIndex > ioBuildQueueScrollIndex - ioBuildQueuePageSize)
        {
            while (1)
            {
                if (curIdx < 0 || curIdx >= IO_MAX_CONSTRUCT_ITEMS)
                    goto afterConstructItem;
                item = ioConstructItems[curIdx];
                itemType = item[6];
                if (!itemType || x < item[0] || y < item[1] || x >= item[2] || y >= item[3])
                    goto afterConstructItem;
                if (itemType == 1)  /* ship launch */
                {
                    ship = (Ship *)item[7];
                    if (ship)
                    {
                        selSelected.numShips = 1;
                        selSelected.ShipPtr[0] = ship;
                        attackcom.numTargets = 1;
                        attackcom.TargetPtr[0] = ship;
                        ioUpdateShipTotals();
                        soundEventPlay(0, 264, 0);
                        if (smSensorsActive)
                            cat_smZoomInOnFocus();
                        else
                        {
                            soundEventPlay(0, 311, 0);
                            ccFocus(&universe.mainCameraCommand, (FocusCommand *)&selSelected);
                        }
                        curIdx = ioBuildQueueScrollIndex;
                    }
                }
                else if (itemType == 2)  /* research */
                {
                    mrResearch(NULL, NULL);
                    curIdx = ioBuildQueueScrollIndex;
                }
                handled = TRUE;
afterConstructItem:
                endIdx--;
                curIdx--;
                if (curIdx <= ioBuildQueueScrollIndex - ioBuildQueuePageSize || curIdx < 0)
                    break;
            }
        }
        y = screenY;
    }
    else
    {
afterConstruct:
        y = screenY;
        x = screenX;
        if (handled)
            goto afterKbRects;
    }

    /* Main overlay button clicks (binary 0x4abe18) */
    if (!handled && showTaskbarButtons && tutorial != 1)
    {
        if (x >= ioMainOverlayButtonRects[0].x0 && y >= ioMainOverlayButtonRects[0].y0 &&
            x < ioMainOverlayButtonRects[0].x1 && y < ioMainOverlayButtonRects[0].y1)
        {
            kbInvokeCommandCallbacks(smSensorsActive != 0, 79);
        }
        else if (x >= ioMainOverlayButtonRects[1].x0 && y >= ioMainOverlayButtonRects[1].y0 &&
                 x < ioMainOverlayButtonRects[1].x1 && y < ioMainOverlayButtonRects[1].y1)
        {
            kbInvokeCommandCallbacks(smSensorsActive != 0, 80);
        }
        else if (x >= ioMainOverlayButtonRects[2].x0 && y >= ioMainOverlayButtonRects[2].y0 &&
                 x < ioMainOverlayButtonRects[2].x1 && y < ioMainOverlayButtonRects[2].y1)
        {
            kbInvokeCommandCallbacks(smSensorsActive != 0, 81);
        }
        else if (x >= ioMainOverlayButtonRects[3].x0 && y >= ioMainOverlayButtonRects[3].y0 &&
                 x < ioMainOverlayButtonRects[3].x1 && y < ioMainOverlayButtonRects[3].y1)
        {
            kbInvokeCommandCallbacks(smSensorsActive != 0, 82);
        }
        x = screenX;
        y = screenY;
        handled = TRUE;
    }
afterKbRects:

    /* Build queue paged scroll arrows */
    pageConfig = singlePlayerGame ? &ioBuildQueueConfigSP[ioBuildQueuePageIndex][0]
                                  : &ioBuildQueueConfig[ioBuildQueuePageIndex][0];
    if (pageConfig[12] && !handled)
    {
        if (ioBuildQueueUpArrowEnabled &&
            x >= ioBuildQueueUpArrowX0 && y >= ioBuildQueueUpArrowY0 &&
            x < ioBuildQueueUpArrowX1 && y < ioBuildQueueUpArrowY1)
        {
            handled = TRUE;
            soundEventPlay(0, 289, 0);
            if (++ioBuildQueueScrollIndex >= ioBuildQueueTotal)
                ioBuildQueueScrollIndex = ioBuildQueueTotal - 1;
            return handled;
        }
        else if (ioBuildQueueDownArrowEnabled &&
                 x >= ioBuildQueueDownArrowX0 && y >= ioBuildQueueDownArrowY0 &&
                 x < ioBuildQueueDownArrowX1 && y < ioBuildQueueDownArrowY1)
        {
            sdword newPageSize;
            handled = TRUE;
            soundEventPlay(0, 290, 0);
            if (ioBuildQueuePageSize == 5 && --ioBuildQueueScrollIndex < 0)
                ioBuildQueueScrollIndex = 0;
            newPageSize = ioBuildQueueTotal + 1;
            ioBuildQueuePageSize = ioBuildQueueTotal + 1;
            if (ioBuildQueueTotal + 1 > 5)
                ioBuildQueuePageSize = 5;
            if (ioBuildQueueTotal > 0)
            {
                sdword k;
                for (k = 0; k < ioBuildQueueTotal; k++)
                    ioConstructItems[k][4] = 0;
            }
        }
    }

    return handled;
}
