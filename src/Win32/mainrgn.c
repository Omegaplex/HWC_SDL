/*=============================================================================
    Name    : mainrgn.c
    Purpose : Logic for handling region events for the main game screen.

    Created 6/27/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#undef UTY_SCREEN_SHOT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "glinc.h"
#include <math.h>
#include <float.h>
#include "switches.h"
#include "types.h"
#include "fastmath.h"
#include "region.h"
#include "feflow.h"
#include "debug.h"
#include "render.h"
#include "demo.h"
#include "camera.h"
#include "cameracommand.h"
#include "mouse.h"
#include "utility.h"
#include "prim2d.h"
#include "prim3d.h"

#include "select.h"
#include "universe.h"
#include "univupdate.h"
#include "soundevent.h"
#include "consmgr.h"
#include "trademgr.h"
#include "CommandWrap.h"
#include "Undo.h"
#include "tactical.h"

#include "globals.h"
#include "font.h"
#include "sensors.h"
#include "teams.h"
#include "mainrgn.h"
#include "flightman.h"
#include "particle.h"
#include "etg.h"
#include "NumberDefs.h"
#include "nis.h"
#include "netcheck.h"
#include "tweak.h"
#include "main.h"
#include "fontreg.h"
#include "probe.h"
#include "gun.h"
#include "pieplate.h"
#include "researchship.h"
#include "launchMgr.h"
#include "researchgui.h"
#include "tactics.h"
#include "soundlow.h"
#include "InfoOverlay.h"
#include "glcaps.h"
#include "singleplayer.h"

#include "KAS.h"
#include "gamechat.h"
#include "meshanim.h"
#include "chatting.h"
#include "Alliance.h"
#include "shader.h"
#include "Objectives.h"
#include "KASFunc.h"
#include "tutor.h"
#include "strings.h"
#include "BattleChat.h"
#include "options.h"
#include "taskbar.h"

extern sdword mouseCursorOverShipType;
extern sdword mouseCursorEnemyType;
extern vector smCameraLookatPoint;
extern ubyte byte_A2DCD4;
char *mrMoveWaypointLinearCommandMessage = NULL;
char *mrMoveWaypointCircularCommandMessage = NULL;
char *mrMoveWaypointEndToEndCommandMessage = NULL;
#include "shipview.h"

#include "AIvar.h"

void selSelectionAddSingleShipNoCheck(MaxSelection *dest, Ship *ship);
#include "gamepick.h"


#include "battle.h"
#include "Captaincy.h"
#include "Commandnetwork.h"
#include "BigGun.h"
#include "SalCapCorvette.h"
#include "bink.h"
#include "KeyBindings.h"
#include "key.h"

void telldir(void);
sdword __cdecl univIsObjFriendly(SpaceObj *obj, Player *player);
bool32 univIsShipCloaked(SpaceObj *obj);
Resource *clCollectNearestResource(CommandLayer *comlayer, SelectCommand *selectcom, bool32 issueFeedback, bool32 playFoundResourceFeedback);
extern sdword smPathMarkersEnabled;

static Effect *mrShipGetModuleSlotEffect(Ship *ship)
{
    if (ship->objtype != OBJ_ShipType || (ship->flags2 & BIT3) != BIT3)
    {
        return NULL;
    }
    return (Effect *)shipModuleSlotRuntime(ship)->activeEffect;
}

/*=============================================================================
    Data:
=============================================================================*/

//global flag controlling rendering of main game screen
sdword mrRenderMainScreen = TRUE;

//camera for main screen
Camera *mrCamera = NULL;

sdword mrMouseDragActive = 0;
sdword mrDragEndX = 0;
sdword mrDragStartX = 0;
sdword mrDragEndY = 0;
sdword mrDragStartY = 0;
sdword mrDragLastX = 0;
sdword mrDragLastY = 0;

bool32 mrWhiteOut = FALSE;
real32 mrWhiteOutT = 0.0f;
real32 cat_mrBlackOut = 0.0f;
real32 cat_mrWhiteOutT = 0.0f;
static sdword mrSavedPathMarkersEnabled = 0;

//rectangle of currently dragged selection
rectangle mrSelectionRect;
static color smBandboxProtectColor = colRGB(0, 0, 250);
static color smBandboxFocusColor = colRGB(100, 100, 100);
static color smBandboxAttackColor = colRGB(250, 100, 100);
static color smBandboxHarvestColor = colRGB(255, 0, 0);
static color smBandboxSpecialColor = colRGB(75, 100, 100);
MaxAnySelection attackcom;
sdword smBandboxEnabled = 0;
sdword kbSuppressSpecialReleaseOverlay = 0;
sdword kbSuppressNextSpecialActivate = 0;
sdword mrOldMouseX = 0, mrOldMouseY = 0;
sdword mrMoveWaypointCount = 0;
sdword mrMoveWaypointsFlags[2] = {0, 0};
vector mrMoveWaypointDestinations[32] = {0};
vector mrMoveWaypointOverflowDestination = {0};
sdword mrMoveWaypointType = MR_MoveWaypointTypeLinear;

char *mrProtectChangeToSphereFormationCommandMessage = NULL;
char *mrProtectChangeToForcefieldWallCommandMessage = NULL;
char *mrSalCapNeedsCrystalTechCommandMessage = NULL;
char *mrSalCapNeedsInfectionTechCommandMessage = NULL;
char *mrSalCapNoSalvageableShipsCommandMessage = NULL;
char *mrSalCapNeedMoreSalvagersCommandMessage = NULL;
char *mrNoHyperspacingShipsAvailableCommandMessage = NULL;
char *mrCantEMPTargetsCommandMessage = NULL;
char *mrNoShipToRepairFoundBattleChatMessage = NULL;
char *mrNoShipToLeechFoundBattleChatMessage = NULL;
char *mrNoShipToEMPFoundBattleChatMessage = NULL;
char *mrNoShipToSwarmFoundBattleChatMessage = NULL;
char *mrNoShipToInfectFoundBattleChatMessage = NULL;
char *mrNoShipToRamFoundBattleChatMessage = NULL;
char *mrNoShipToFireMissileAtFoundBattleChatMessage = NULL;
char *mrNoShipToSalvageFoundBattleChatMessage = NULL;

//logic handlers for main region
void mrNULL(void);
void (*mrHoldLeft)(void) = mrNULL;
void (*mrHoldRight)(void) = mrNULL;

static void mrInitSingleClickSpecialNoTargetMessages(void)
{
    mrProtectChangeToSphereFormationCommandMessage = strGetString(str_GM_ChangeToSphereFormation);
    mrProtectChangeToForcefieldWallCommandMessage = strGetString(str_GM_ChangeToForceFieldWall);
    mrSalCapNeedsCrystalTechCommandMessage = strGetString(strWorkerNeedsCrystalTech);
    mrSalCapNeedsInfectionTechCommandMessage = strGetString(strDepotNeedsInfectionTech);
    mrSalCapNoSalvageableShipsCommandMessage = strGetString(strWorkerNoSalvageableShips);
    mrSalCapNeedMoreSalvagersCommandMessage = strGetString(strNeedMoreSalvagers);
    mrNoShipToRepairFoundBattleChatMessage = strGetString(strNoShipToRepairFound);
    mrNoShipToLeechFoundBattleChatMessage = strGetString(strNoShipToLeechFound);
    mrNoShipToEMPFoundBattleChatMessage = strGetString(strNoShipToEMPFound);
    mrNoShipToSwarmFoundBattleChatMessage = strGetString(strNoShipToSwarmFound);
    mrNoShipToInfectFoundBattleChatMessage = strGetString(strNoShipToInfectFound);
    mrNoShipToRamFoundBattleChatMessage = strGetString(strNoShipToRamFound);
    mrNoShipToFireMissileAtFoundBattleChatMessage = strGetString(strNoShipToFireMissileAtFound);
    mrNoShipToSalvageFoundBattleChatMessage = strGetString(strNoShipToSalvageFound);
    mrNoHyperspacingShipsAvailableCommandMessage = strGetString(strNoHyperspacingShipsAvailable);
    mrCantEMPTargetsCommandMessage = strGetString(strCantEMPTargets);

    mrMoveWaypointLinearCommandMessage = strGetString(strWaypointModeNormal);
    mrMoveWaypointCircularCommandMessage = strGetString(strWaypointModePatrolCircular);
    mrMoveWaypointEndToEndCommandMessage = strGetString(strWaypointModePatrolEndToEnd);
}

static bool32 mrShipCanUseBigGun(Ship *ship)
{
    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT) != 0;

        default:
            return FALSE;
    }
}

int mrUpdateMouseDrag(void)
{
    int result;

    result = mrMouseDragActive;
    if (!mrMouseDragActive)
    {
        return result;
    }

    result = mouseCursorXPosition;
    mrDragEndX += mouseCursorXPosition - mrDragLastX;
    mrDragStartX += mouseCursorXPosition - mrDragLastX;
    mrDragEndY += mouseCursorYPosition - mrDragLastY;
    mrDragStartY += mouseCursorYPosition - mrDragLastY;
    mrDragLastX = mouseCursorXPosition;
    mrDragLastY = mouseCursorYPosition;

    return result;
}

bool32 shipIsBeastShip(Ship *ship);
void *shipGetBeastWeaponInfo(Ship *ship);

sdword mrDetermineTargetRightClickCursorType(sdword *selectionActions, sdword *targetActions, Ship *ship)
{
    if (targetActions[6])
    {
        if (selectionActions[0] && !selectionActions[3])
        {
            return 1;
        }
    }
    else
    {
        if (targetActions[4])
        {
            if (selectionActions[32])
            {
                return 10;
            }
            if (selectionActions[33])
            {
                return homeworldCSA ? 3 : 13;
            }
            if (selectionActions[35])
            {
                return homeworldCSA ? 3 : 14;
            }
            if (selectionActions[36])
            {
                return homeworldCSA ? 3 : 15;
            }
            if (selectionActions[38])
            {
                return homeworldCSA ? 3 : 16;
            }
            if (selectionActions[37] && shipGetBeastWeaponInfo(ship))
            {
                return homeworldCSA ? 3 : 17;
            }
        }

        if (targetActions[5])
        {
            if (selectionActions[2] && !selectionActions[3])
            {
                return 4;
            }
            if (selectionActions[39] && !targetActions[26])
            {
                return 5;
            }
        }
        else if (targetActions[8])
        {
            if (selectionActions[5])
            {
                return 6;
            }
        }
        else if (targetActions[9])
        {
            if (selectionActions[5])
            {
                return 7;
            }
        }
        else if (targetActions[10])
        {
            if (selectionActions[5])
            {
                return 8;
            }
        }
        else if (targetActions[7] && selectionActions[1])
        {
            return 2;
        }
    }

    if (targetActions[4] && selectionActions[3])
    {
        return 3;
    }
    if (targetActions[7] && !selectionActions[3] && selectionActions[1])
    {
        return 2;
    }
    return 0;
}

sdword mrDetermineCursorType(udword *keyState, udword *mouseState, sdword *selectedShips, Ship *ship)
{
    sdword v17 = 0;
    Ship *v6;
    sdword v8, v9, v11, shipa, v14, v15, v16;
    Ship **v10, **v12;
    ObjType objtype;

    if (playPackets)
        return 0;

    if (mouseState[6])
    {
        if (keyState[0] && !keyState[3])
            return 1;
    }
    else
    {
        if (mouseState[4])
        {
            if (keyState[32])
                return 10;
            if (keyState[33])
                return homeworldCSA ? 3 : 13;
            if (keyState[35])
                return homeworldCSA ? 3 : 14;
            if (keyState[36])
                return homeworldCSA ? 3 : 15;
            if (keyState[38])
                return homeworldCSA ? 3 : 16;
            v6 = ship;
            if (keyState[37] && shipGetBeastWeaponInfo(ship))
                return homeworldCSA ? 3 : 17;
            goto LABEL_40;
        }
        if (mouseState[5])
        {
            if (keyState[2] && !keyState[3])
                return 4;
            if (keyState[39] && !mouseState[26])
                return 5;
        }
        else if (mouseState[8])
        {
            if (keyState[5])
                return 6;
        }
        else if (mouseState[9])
        {
            if (keyState[5])
                return 7;
        }
        else if (mouseState[10])
        {
            if (keyState[5])
                return 8;
        }
        else if (mouseState[7] && keyState[1])
        {
            return 2;
        }
    }

    v6 = ship;
LABEL_40:
    if (mouseState[4] && keyState[3])
        return 3;
    if (mouseState[7] && !keyState[3] && keyState[1])
        return 2;
    if (v6->objtype == OBJ_ShipType && v6->playerowner == universe.curPlayerPtr && *selectedShips > 0)
    {
        if ((v6->flags & SOF_Dead) == 0 && v6->specialFlags3 == 0 && !shipIsBeastShip(v6))
        {
            v9 = 1;
            if (*selectedShips <= 0)
            {
                v17 = 9;
            }
            else
            {
                v10 = (Ship **)(selectedShips + 2);
                v8 = 0;
                do
                {
                    if (!ThisShipCanDockWith(*v10, ship, 0, FALSE))
                        v9 = 0;
                    ++v8;
                    ++v10;
                } while (v8 < *selectedShips);
                if (v9)
                    v17 = 9;
            }
        }
        if (ShipHasUpgrade(v6, RESEARCH_UPGRADE_MASK_LINK_PRIMARY))
        {
            shipa = 1;
            if (*selectedShips <= 0)
                return 11;
            v12 = (Ship **)(selectedShips + 2);
            v11 = 0;
            do
            {
                if (!ShipHasUpgrade(*v12, RESEARCH_UPGRADE_MASK_LINK_PRIMARY) || (*v12)->shiptype != v6->shiptype)
                    shipa = 0;
                ++v11;
                ++v12;
            } while (v11 < *selectedShips);
            if (shipa)
                return 11;
        }
        if (v17)
            return v17;
    }
    if (keyState[34])
    {
        objtype = v6->objtype;
        if (objtype != OBJ_ShipType)
        {
            if (objtype == OBJ_AsteroidType)
                return 12;
        }
        else
        {
            v14 = smMimicGetEntryCount(v6->playerowner->playerIndex, (sdword)mouseState[23]);
            v15 = 0;
            v16 = 0;
            if (v14 > 0)
            {
                do
                {
                    if (*(sdword *)smMimicGetEntryAddress(v6->playerowner->playerIndex, (sdword)mouseState[23], v16) == v6->shiptype)
                        v15 = 1;
                    ++v16;
                } while (v16 < v14);
                if (v15)
                    return 12;
            }
        }
    }
    return v17;
}

static ubyte mrLeechGetMode(Ship *ship)
{
    return ((LeechSpecifics *)ship->ShipSpecifics)->requestedMode;
}

void smDrawBandbox(bool32 bOverlayOnly);
bool32 shipHasReadySwarmers(Ship *ship);
bool32 acolyteIsLinked(Ship *ship);
bool32 shipModuleSlotIsEligible(Ship *ship, Player *player);
bool32 shipHasModuleSlotState(Ship *ship);
sdword __cdecl mrIssueSelectionSingleClickSpecial(SelectAnyCommand *targetSelection, sdword specialType);
sdword __cdecl mrActivateSingleClickSpecialOnNearestFriendly(SelectAnyCommand *targetSelection);
sdword __cdecl mrLeechSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrInfectSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrFireMissilesAtNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrRamSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrRepulseSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrSalvageSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrDockSelectionToShip(Ship *targetShip);
void ioDrawBoundedLabel(rectangle *bounds, char *text, sdword highlightedIndex, fonthandle font, sdword capWidth, sdword leftIcon, sdword leftHoverIcon, sdword centerIcon, sdword centerHoverIcon, sdword rightIcon, sdword rightHoverIcon, bool32 hoverWholeButton);
void primRectGradient2(rectangle *rect, color leftColor, color rightColor);
static bool32 gameStatsOverlayActive(void);
static void gameStatsOverlayResetSelectionTimer(void);
static void gameStatsOverlaySortPlayersBySurvivalTime(void);
static void gameStatsOverlayProcess(void);
static void gameStatsOverlayDraw(void);
static void smDrawOverlayColumnHeaders(rectangle *bounds, bool32 fillBackground, udword borderSize);

//flag indicating mouse has moved since right mouse button pressed
sdword mrMouseHasMoved = 0;
sdword mrMouseMovementClickLimit = MR_MouseMovementClickLimit;

//global handle to the main region
regionhandle ghMainRegion = NULL;

//tactical overlay on/off flag
bool32 mrDrawTactical = FALSE, mrSaveTactical = FALSE;

//help ifno screen up or not
bool32 helpinfoactive = FALSE;

regionhandle helpinforegion;

//debug for turning off enemy AI
bool32 mrNoAI = FALSE;

//fonthandle for Big Font
static fonthandle mrBigFont = 0;
static fonthandle mrTabStatsFont = FONT_InvalidFontHandle;

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

static bool32 gameStatsOverlayActive(void)
{
    return mrDisabled && gameStatsOverlayEnabled != 0 && gameStatsSnapshotBuffer != NULL;
}

static void gameStatsOverlayResetSelectionTimer(void)
{
    gameStatsOverlayResetTimer();
    if (!gameStatsOverlayInitialized)
    {
        gameStatsOverlayInitialized = 1;
    }
}

static void gameStatsOverlaySortPlayersBySurvivalTime(void)
{
    sdword i = 0;

    for (i = 0; i < universe.numPlayers; i++)
    {
        gameStatsPlayerOrder[i] = i;
    }

    i = 0;
    while (i < universe.numPlayers - 1)
    {
        real32 nextStatValue = utyGameStatsGetStatValue(gameStatsPlayerOrder[i + 1], 0);

        if (utyGameStatsGetStatValue(gameStatsPlayerOrder[i], 0) >= nextStatValue)
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

static void gameStatsOverlayProcess(void)
{
    sdword statIndex;

    if (screenX >= gameStatsExitRect.x0 &&
        screenY >= gameStatsExitRect.y0 &&
        screenX < gameStatsExitRect.x1 &&
        screenY < gameStatsExitRect.y1)
    {
        gameStatsQuitToMain();
        return;
    }

    for (statIndex = 0; statIndex < GAME_STATS_COUNT; statIndex++)
    {
        if (screenX >= gameStatsHitLeft[statIndex] &&
            screenY >= gameStatsHitTop[statIndex] &&
            screenX < gameStatsHitRight[statIndex] &&
            screenY < gameStatsHitBottom[statIndex])
        {
            gameStatsSelectedStat = statIndex;
            gameStatsOverlaySortPlayersBySurvivalTime();
            gameStatsOverlayResetSelectionTimer();
            soundEventPlay(0, 265, 0);
            return;
        }
    }
}

static void smDrawOverlayColumnHeaders(rectangle *bounds, bool32 fillBackground, udword borderSize)
{
    if (fillBackground)
    {
        primRectTranslucent2(bounds, colRGBA(0, 0, 0, 64));
    }

    opOverlayIconDraw(0, bounds->y0 - borderSize, bounds->x0 - borderSize, borderSize, borderSize, colWhite);
    opOverlayIconDraw(1, bounds->y0 - borderSize, bounds->x1, borderSize, borderSize, colWhite);
    opOverlayIconDraw(3, bounds->y1, bounds->x1, borderSize, borderSize, colWhite);
    opOverlayIconDraw(2, bounds->y1, bounds->x0 - borderSize, borderSize, borderSize, colWhite);
    opOverlayIconDraw(5, bounds->y0 - borderSize, bounds->x0, bounds->x1 - bounds->x0, borderSize, colWhite);
    opOverlayIconDraw(6, bounds->y1, bounds->x0, bounds->x1 - bounds->x0, borderSize, colWhite);
    opOverlayIconDraw(7, bounds->y0, bounds->x0 - borderSize, borderSize, bounds->y1 - bounds->y0, colWhite);
    opOverlayIconDraw(8, bounds->y0, bounds->x1, borderSize, bounds->y1 - bounds->y0, colWhite);
}

static void gameStatsOverlayDraw(void)
{
    fonthandle previousFont;
    rectangle rect;
    rectangle headerRect;
    sdword fontHeightPixels;
    sdword statIndex;
    sdword playerSlot;
    real32 maxStatValue = 0.0f;
    char valueText[128];

    if (keyIsHit(ARRDOWN))
    {
        gameStatsSelectedStat++;
        if (gameStatsSelectedStat >= GAME_STATS_COUNT)
        {
            gameStatsSelectedStat = 0;
        }
        gameStatsOverlaySortPlayersBySurvivalTime();
        gameStatsOverlayResetSelectionTimer();
    }
    else if (keyIsHit(ARRUP))
    {
        gameStatsSelectedStat--;
        if (gameStatsSelectedStat < 0)
        {
            gameStatsSelectedStat = GAME_STATS_COUNT - 1;
        }
        gameStatsOverlaySortPlayersBySurvivalTime();
        gameStatsOverlayResetSelectionTimer();
    }
    else if (!gameStatsOverlayInitialized)
    {
        gameStatsOverlayInitialized = 1;
    }

    previousFont = fontMakeCurrent(gameStatsOverlayLabelFont);
    fontHeightPixels = fontHeight(" ");

    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    if (!primModeEnabled)
    {
        primModeSetFunction2();
    }

    headerRect.x0 = gameStatsPanelLeft;
    headerRect.y0 = (universe.numPlayers > 0) ? gameStatsRankIconTop[0] - 6 : gameStatsPanelTop;
    headerRect.x1 = gameStatsPanelRight;
    headerRect.y1 = gameStatsPanelBottom;
    rect.x0 = gameStatsPanelLeft - 16;
    rect.y0 = headerRect.y0 - 16;
    rect.x1 = gameStatsPanelRight + 16;
    rect.y1 = headerRect.y1 + 16;
    primRectTranslucent2(&rect, 0xD2323232);
    smDrawOverlayColumnHeaders(&headerRect, FALSE, 20);

    ioDrawBoundedLabel(
        &gameStatsExitRect,
        strGetString(str_GSD_Exit),
        1,
        gameStatsOverlayValueFont,
        30,
        19,
        22,
        21,
        24,
        20,
        23,
        TRUE);

    for (statIndex = 0; statIndex < GAME_STATS_COUNT; statIndex++)
    {
        color statColor = (statIndex & 1) ? 0xFFB4B4B4 : colWhite;

        if (gameStatsSelectedStat == statIndex)
        {
            statColor = colRGB(0, 255, 0);
        }

        fontPrint(gameStatsLabelX[statIndex], gameStatsLabelY[statIndex], statColor, strGetString((strGamesMessages)gameStatsLabelStringIds[statIndex]));

        if ((screenX >= gameStatsHitLeft[statIndex] &&
             screenY >= gameStatsHitTop[statIndex] &&
             screenX < gameStatsHitRight[statIndex] &&
             screenY < gameStatsHitBottom[statIndex]) ||
            gameStatsSelectedStat == statIndex)
        {
            sdword leftIcon = 25;
            sdword centerIcon = 27;
            sdword rightIcon = 29;

            if (screenX >= gameStatsHitLeft[statIndex] &&
                screenY >= gameStatsHitTop[statIndex] &&
                screenX < gameStatsHitRight[statIndex] &&
                screenY < gameStatsHitBottom[statIndex])
            {
                leftIcon = 26;
                centerIcon = 28;
                rightIcon = 30;
            }

            opOverlayIconDraw(leftIcon,
                              gameStatsHitTop[statIndex],
                              gameStatsHitLeft[statIndex],
                              gameStatsIconSplit0[statIndex] - gameStatsHitLeft[statIndex],
                              gameStatsHitBottom[statIndex] - gameStatsHitTop[statIndex],
                              colWhite);
            opOverlayIconDraw(centerIcon,
                              gameStatsHitTop[statIndex],
                              gameStatsIconSplit1[statIndex],
                              gameStatsHitRight[statIndex] - gameStatsIconSplit1[statIndex],
                              gameStatsHitBottom[statIndex] - gameStatsHitTop[statIndex],
                              colWhite);
            opOverlayIconDraw(rightIcon,
                              gameStatsHitTop[statIndex],
                              gameStatsIconSplit0[statIndex],
                              gameStatsIconSplit1[statIndex] - gameStatsIconSplit0[statIndex],
                              gameStatsHitBottom[statIndex] - gameStatsHitTop[statIndex],
                              colWhite);
        }

        for (playerSlot = 0; playerSlot < universe.numPlayers; playerSlot++)
        {
            utyGameStatsFormatStatValue(gameStatsPlayerOrder[playerSlot], statIndex, valueText);
            fontPrint(gameStatsBarRight[playerSlot] - fontWidth(valueText), gameStatsLabelY[statIndex], statColor, valueText);
        }
    }

    for (playerSlot = 0; playerSlot < universe.numPlayers; playerSlot++)
    {
        real32 playerStat;
        sdword playerIndex = gameStatsPlayerOrder[playerSlot];

        playerStat = utyGameStatsGetStatValue(playerIndex, gameStatsSelectedStat);
        if (playerStat > maxStatValue)
        {
            maxStatValue = playerStat;
        }
    }

    rect.x0 = gameStatsGraphLeft - 16;
    rect.y0 = gameStatsGraphTop - 16;
    rect.x1 = gameStatsGraphRight + 16;
    rect.y1 = gameStatsGraphBottom + 16;
    primRectSolid2(&rect, 0xFF000000);
    headerRect.x0 = gameStatsGraphLeft;
    headerRect.y0 = gameStatsGraphTop;
    headerRect.x1 = gameStatsGraphRight;
    headerRect.y1 = gameStatsGraphBottom;
    smDrawOverlayColumnHeaders(&headerRect, FALSE, 20);

    for (playerSlot = 0; playerSlot < universe.numPlayers; playerSlot++)
    {
        rectangle barRect;
        sdword playerIndex = gameStatsPlayerOrder[playerSlot];
        bool32 rowHighlighted = screenX >= gameStatsBarLeft[playerSlot] &&
                                screenY >= gameStatsBarTop[playerSlot] &&
                                screenX < gameStatsBarRight[playerSlot] &&
                                screenY < gameStatsBarBottom[playerSlot];
        real32 playerStat = utyGameStatsGetStatValue(playerIndex, gameStatsSelectedStat);
        real32 playerBarScale = 0.0f;
        color playerTextColor;
        color playerBarColor = teColorSchemes[playerIndex].textureColor.base;
        real32 timeRemaining = gameStatsOverlayTimeRemaining();

        if (maxStatValue > 0.0f)
        {
            playerBarScale = playerStat / maxStatValue;
        }

        sprintf(valueText, "%d. %s", playerSlot + 1, playerNames[playerIndex]);
        if (playerIndex == universe.curPlayerIndex)
        {
            playerTextColor = colRGB(0, 255, 0);
        }
        else if (allianceArePlayersAllied(&universe.players[playerIndex], universe.curPlayerPtr))
        {
            playerTextColor = colRGB(255, 0, 0);
        }
        else
        {
            playerTextColor = colRGB(0, 0, 255);
        }
        if (rowHighlighted)
        {
            playerTextColor = colWhite;
        }

        fontPrint(fontHeightPixels + gameStatsPlayerColumnX[playerSlot] + 5, gameStatsPlayerRowY[playerSlot], playerTextColor, valueText);
        opOverlayIconDraw(universe.players[playerIndex].race != RACE_Sect ? 16 : 15,
                          gameStatsPlayerRowY[playerSlot],
                          gameStatsPlayerColumnX[playerSlot],
                          fontHeightPixels,
                          fontHeightPixels,
                          colWhite);

        if (playerBarScale > timeRemaining)
        {
            playerBarScale = timeRemaining;
        }

        barRect.x0 = gameStatsBarLeft[0];
        barRect.x1 = gameStatsBarLeft[0] + (sdword)((real32)(gameStatsGraphRight - gameStatsBarLeft[0]) * playerBarScale);
        barRect.y0 = gameStatsPlayerRowY[playerSlot];
        barRect.y1 = gameStatsPlayerRowY[playerSlot] + fontHeightPixels - 1;
        if (barRect.x1 > barRect.x0)
        {
            primRectGradient2(&barRect, playerBarColor, colWhite);
        }

        primLine2(gameStatsBarRight[playerSlot],
                  gameStatsBarTop[playerSlot],
                  gameStatsBarRight[playerSlot],
                  gameStatsBarBottom[playerSlot],
                  playerSlot * 4);
        opOverlayIconDraw(32 + playerSlot,
                          gameStatsRankIconTop[playerSlot],
                          gameStatsRankIconLeft[playerSlot],
                          gameStatsRankIconRight[playerSlot] - gameStatsRankIconLeft[playerSlot],
                          gameStatsRankIconBottom[playerSlot] - gameStatsRankIconTop[playerSlot],
                          colWhite);
    }

    fontMakeCurrent(previousFont);
}

static bool32 mrTabStatsEnabled = FALSE;
static bool32 mrTabStatsDragging = FALSE;
static bool32 mrTabStatsShowGuns = TRUE;
static bool32 mrTabStatsShowArmour = TRUE;
static bool32 mrTabStatsShowTechShipHas = TRUE;
static bool32 mrTabStatsShowAllTech = TRUE;
static bool32 mrTabStatsShowMaxTech = TRUE;
static bool32 mrTabStatsShowTechShipNeeds = TRUE;
static bool32 mrTabStatsShowClass = TRUE;
static bool32 mrTabStatsShowHealth = TRUE;
static rectangle mrTabStatsRect = {0, 0, 0, 0};
static sdword mrTabStatsX = 30;
static sdword mrTabStatsY = 200;
static sdword mrTabStatsDragMouseX = 0;
static sdword mrTabStatsDragMouseY = 0;
static sdword mrTabStatsDragAnchorX = 30;
static sdword mrTabStatsDragAnchorY = 200;

typedef struct
{
    udword mask;
    strGamesMessages string;
} mrTabStatsTechEntry;

static const mrTabStatsTechEntry mrTabStatsTechTable[] =
{
    {BIT0,  str_SO_AdvancedFighterDrive},
    {BIT1,  str_SO_AdvancedFrigateDrive},
    {BIT2,  str_SO_AdvancedDestroyerDrive},
    {BIT3,  str_SO_LinkingAbility},
    {BIT5,  str_SO_MimicAbility},
    {BIT6,  str_SO_LeechingAbility},
    {BIT7,  str_SO_ExplosiveCharge},
    {BIT8,  str_SO_Afterburners},
    {BIT9,  str_SO_EMPWeapon},
    {BIT10, str_SO_RepulseWeapon},
    {BIT11, str_SO_SwarmingAbility},
    {BIT12, str_SO_ForceField1},
    {BIT13, str_SO_ForceField2},
    {BIT14, str_SO_ForceField3},
    {BIT15, str_SO_DeflectionAbility},
    {BIT16, str_SO_InfectingAbility},
    {BIT17, str_SO_CloakingAbility},
    {BIT18, str_SO_RepairingAbility},
    {BIT19, str_SO_SalvagingAbility},
    {BIT20, str_SO_CrystalHarvestAbility},
    {BIT21, str_SO_SelfRepairAbility},
    {BIT22, str_SO_BeastSalvagingAbility},
    {BIT23, str_SO_AdvancedSensors},
    {BIT26, str_SO_AbilityOrdinance},
};

//key definitions
struct
{
    keyindex key[REG_NumberKeys];
    ubyte nKeys;
    udword filter;
}
mrKeyFunction[] =
{
    {{ESCKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{TABKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{ARRLEFT,   0,      0,      0}, 1, RPE_KeyDown},
    {{ARRRIGHT,  0,      0,      0}, 1, RPE_KeyDown},
    {{BKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{AKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{CKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{DKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{EKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{FKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{RKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{MMOUSE_BUTTON,0,   0,      0}, 1, RPE_KeyDown},
    {{MMOUSE_DOUBLE,0,   0,      0}, 1, RPE_KeyDown},
    {{HKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{IKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{JKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{KKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{LKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{MKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{LBRACK,    0,      0,      0}, 1, RPE_KeyDown},
    {{RBRACK,    0,      0,      0}, 1, RPE_KeyDown},
    {{SKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{VKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{ZKEY,      0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{CAPSLOCKKEY,0,     0,      0}, 1, RPE_KeyDown},
    {{F1KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F2KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F3KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F4KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{TILDEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{ZEROKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{ONEKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{TWOKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{THREEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{FOURKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{FIVEKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{XKEY,      0,      0,      0}, 1, RPE_KeyDown},
    {{SIXKEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{SEVENKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{EIGHTKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{NINEKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{SPACEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{ENTERKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{F5KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F6KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F7KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F8KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F9KEY,     0,      0,      0}, 1, RPE_KeyDown},
    {{F10KEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{F11KEY,    0,      0,      0}, 1, RPE_KeyDown},
    {{HOMEKEY,   0,      0,      0}, 1, RPE_KeyDown},
    {{PAGEDOWNKEY,0,     0,      0}, 1, RPE_KeyDown},
    {{PAGEUPKEY, 0,      0,      0}, 1, RPE_KeyDown},
    {{BACKSLASHKEY,      0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},






    {{PKEY,              0,      0,      0}, 1, RPE_KeyDown},
#if MR_TEST_GUNS
    {{TKEY,      0,      0,      0}, 1, RPE_KeyDown},
#else
    {{TKEY,      0,      0,      0}, 1, RPE_KeyDown},
#endif

#if GUN_TUNE_MODE
    {{YKEY,      0,      0,      0}, 1, RPE_KeyDown},
#endif
/*
#if MR_SCREENSHOTS
    {{PAUSEKEY,  0,      0,      0}, 1, RPE_KeyDown},
    {{SCROLLKEY, 0,      0,      0}, 1, RPE_KeyDown},
#endif
*/
    {{SHIFTKEY,  0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#if ETG_RELOAD_KEY
    {{ETG_RELOAD_KEY,0,  0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#endif
#if NIS_TEST
    {{NUMPAD1,   0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#endif
#if MR_TEST_HPB
    {{OKEY,   0,      0,      0}, 1, RPE_KeyDown},
#endif
    {{NUMPLUSKEY,   0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},
    {{PLUSKEY,      0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},
    {{NUMMINUSKEY,  0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},
    {{MINUSKEY,     0,      0,      0},     1,  RPE_KeyDown | RPE_KeyUp},

// Keys for music selection in multiplayer game
    {{LESSTHAN    , 0,      0,      0}, 1, RPE_KeyDown},
    {{GREATERTHAN , 0,      0,      0}, 1, RPE_KeyDown},

// Extra keys that are mappable that the mainregion must capture
#ifndef HW_Debug  //only add these for release builds so that random debug keys will work
    {{PAUSEKEY    , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{PRINTKEY    , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{ARRUP       , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{ARRDOWN     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{ENDKEY      , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{INSERTKEY   , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{DELETEKEY   , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{GKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{OKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{QKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{UKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{WKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{XKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{YKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD0     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD1     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD2     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD3     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD4     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD5     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD6     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD7     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD8     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMPAD9     , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMSTARKEY  , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMSLASHKEY , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{NUMDOTKEY   , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#else
    {{GKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{UKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{YKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
    {{QKEY        , 0,      0,      0}, 1, RPE_KeyDown | RPE_KeyUp},
#endif

    {{0,         0,      0,      0}, 0, RPE_KeyDown}
};

//stuff for changing formations
#define PARADE_FORMATION_FLAG   ((udword)-1)
TypeOfFormation mrNewFormation = 0;                 //index of current formation we're selecting
char *mrFormationName = NULL;                   //if non-NULL, we're in the changing formations state
real32 mrFormationTime;                         //time a particular formation was set
bool32   mrDrawFormation=FALSE;
real32 mrDrawFormationTime=0.0;
fonthandle mrFormationFont = FONT_InvalidFontHandle;
TypeOfFormation mrCycleFormationType = DELTA_FORMATION;
real32 mrCycleFormationExpire = 0.0f;
TacticsType mrCycleTacticsType = Neutral;
real32 mrCycleTacticsExpire = 0.0f;

#if MAD_TEST_ANIMATION
sdword mrTestAnimationIndex = 0;                //index of mesh animation we're testing
#endif

// for kas text display
extern sdword popupTextNumLines;

// number indicates which player was right clicked on in the player list
sdword playerClickedOn=-1;

//for handling bot z-bandbox and just pressing z
bool32 mrSpecialBandBox;

//for double-press number keys to focus on ships
real32 mrNumberDoublePressTime = MR_NumberDoublePressTime;
sdword mrLastKeyPressed = -1;
real32 mrLastKeyTime = 0;

//to help clicking on fast-moving ships:
real32 mrFastMoveShipClickTime = MR_FastMoveShipClickTime;
sdword mrFastMoveShipClickX = MR_FastMoveShipClickX;
sdword mrFastMoveShipClickY = MR_FastMoveShipClickY;

#if MR_CAN_FOCUS_ROIDS
bool32 mrCanFocusRoids = FALSE;
#endif

bool32 mrMenuDontDisappear = FALSE;       //forces the context menus to stay on the screen

#define TIMEOUT_TIME 15.0 // timer for the printing of player dropped messages

#ifdef HW_Debug
extern real32 pilotupoffset;
#endif

/*=============================================================================
    Private Function prototypes:
=============================================================================*/

//sdword mrClickedOnPlayer(void);
void mrSetTheFormation(TypeOfFormation formationtype);
static void mrTabStatsStartup(void);
static void mrTabStatsToggle(void);
static bool32 mrTabStatsIsEnabled(void);
static bool32 mrTabStatsStartDrag(void);
static bool32 mrTabStatsEndDrag(void);
static bool32 mrTabStatsIsDragging(void);
static void mrTabStatsUpdateDrag(void);
sdword tabStatsToggleDisplayOption(char keycode);
static void mrTabStatsDrawDisplayOptionStatus(void);
static Node *mrBandboxSelectionStartNode(sdword clickNode);
bool32 mrHandleBandboxPressLeft(void);
void mrHandleBandboxPressRight(rectangle *playerRect);
void mrHandleBandboxHoldLeft(sdword clickNode);
void mrHandleBandboxReleaseLeft(sdword clickNode);
static void mrSelectLeftRelease(sdword clickNode);
static bool32 mrProcessBandboxSelection(Camera *camera, sdword clickNode);
extern bool32 ccFocusIsLegal(SpaceObj *obj);
extern void cat_smZoomInOnFocus(void);
extern void makeShipsCommandControllable(SelectCommand *selection, sdword requireCommand);
extern regionhandle spHyperspaceRollCallScreen;
void mrDrawTabStats(Ship *ship);

/*=============================================================================
    Functions:
=============================================================================*/

static udword originalMrFilter;
bool32   mrDisabled = FALSE;
Ship *mrTrackedShipReference = NULL;

void mrDisable(void)
{
    if (!mrDisabled)
    {
        dbgAssert(ghMainRegion);
        originalMrFilter = ghMainRegion->flags;
    //    regFilterSet(ghMainRegion, 0);
        regFilterSet(ghMainRegion, RPM_MouseEvents);
        mrDisabled = TRUE;
    }
}

void mrEnable(void)
{
    if (mrDisabled)
    {
        regFilterSet(ghMainRegion,originalMrFilter);
        mrDisabled = FALSE;
    }
}

static void mrTabStatsStartup(void)
{
    mrTabStatsFont = frFontRegister("default.hff");
    mrTabStatsEnabled = FALSE;
    mrTabStatsDragging = FALSE;
    mrTabStatsShowGuns = TRUE;
    mrTabStatsShowArmour = TRUE;
    mrTabStatsShowTechShipHas = TRUE;
    mrTabStatsShowAllTech = TRUE;
    mrTabStatsShowMaxTech = TRUE;
    mrTabStatsShowTechShipNeeds = TRUE;
    mrTabStatsShowClass = TRUE;
    mrTabStatsShowHealth = TRUE;
    mrTabStatsX = 30;
    mrTabStatsY = 200;
    mrTabStatsRect.x0 = mrTabStatsRect.y0 = mrTabStatsRect.x1 = mrTabStatsRect.y1 = 0;
}

void tabStatsResetDrag(void)
{
    mrTabStatsDragging = FALSE;
}

static void mrTabStatsToggle(void)
{
    mrTabStatsEnabled ^= TRUE;
    if (!mrTabStatsEnabled)
    {
        mrTabStatsDragging = FALSE;
    }
}

static bool32 mrTabStatsIsEnabled(void)
{
    return (mrTabStatsEnabled &&
            selSelected.numShips > 0 &&
            selSelected.ShipPtr[0] != NULL &&
            selSelected.ShipPtr[0]->playerowner == universe.curPlayerPtr);
}

bool32 IsTabStatsEnabled(void)
{
    return mrTabStatsIsEnabled();
}

static udword mrTabStatsHealthColor(Ship *ship)
{
    real32 maxHealth;
    real32 percent;

    maxHealth = ship->staticinfo->maxhealth;
    if (maxHealth <= 0.0f)
    {
        return colWhite;
    }

    percent = ship->health / maxHealth;
    if (percent >= 0.66f)
    {
        return colRGB(0, 255, 0);
    }
    if (percent >= 0.33f)
    {
        return colRGB(255, 255, 0);
    }
    return colRGB(255, 96, 96);
}

static char *mrTabStatsUpgradeTypeString(sdword upgradeType)
{
    switch (upgradeType)
    {
        case 0: return strGetString(str_SO_Projectile);
        case 1: return strGetString(str_SO_Energy);
        case 2: return strGetString(str_SO_Ion);
        case 3: return strGetString(str_SO_PlasmaBomb);
        case 4: return strGetString(str_SO_Laser);
        case 5: return strGetString(str_SO_Missile);
        case 6: return strGetString(str_SO_Mine);
        case 7: return strGetString(str_SO_BeastInfect);
        case 8: return strGetString(str_SO_BeastInfect);
        default: return "";
    }
}

static char *mrTabStatsGunSizeString(GunSize gunSize)
{
    switch (gunSize)
    {
        case 0: return strGetString(str_SO_Small);
        case 1: return strGetString(str_SO_Medium);
        case 2: return strGetString(str_SO_Large);
        case 3: return strGetString(str_SO_VeryLarge);
        default: return strGetString(strNotApplicable);
    }
}

static char *mrTabStatsArmourLevelString(Ship *ship)
{
    if (ship->upgradeState.level >= 2)
    {
        return strGetString(str_SO_ArmourLevel3);
    }
    if (ship->upgradeState.level == 1)
    {
        return strGetString(str_SO_ArmourLevel2);
    }
    return strGetString(str_SO_ArmourLevel1);
}

static bool32 mrTabStatsStartDrag(void)
{
    if (!kbCommandIsHitByName("Toggle_Overlay_Options") ||
        !mrTabStatsIsEnabled() ||
        mrTabStatsDragging ||
        mouseCursorX() < mrTabStatsRect.x0 ||
        mouseCursorY() < mrTabStatsRect.y0 ||
        mouseCursorX() >= mrTabStatsRect.x1 ||
        mouseCursorY() >= mrTabStatsRect.y1)
    {
        return FALSE;
    }

    mrTabStatsDragging = TRUE;
    mrTabStatsDragMouseX = mouseCursorX();
    mrTabStatsDragMouseY = mouseCursorY();
    mrTabStatsDragAnchorX = mrTabStatsX;
    mrTabStatsDragAnchorY = mrTabStatsY;
    return TRUE;
}

static bool32 mrTabStatsEndDrag(void)
{
    if (!mrTabStatsDragging)
    {
        return FALSE;
    }

    mrTabStatsUpdateDrag();
    mrTabStatsDragging = FALSE;
    return TRUE;
}

static bool32 mrTabStatsIsDragging(void)
{
    return mrTabStatsDragging;
}

static void mrTabStatsUpdateDrag(void)
{
    if (!mrTabStatsDragging)
    {
        return;
    }

    mrTabStatsX = mrTabStatsDragAnchorX + (mouseCursorX() - mrTabStatsDragMouseX);
    mrTabStatsY = mrTabStatsDragAnchorY + (mouseCursorY() - mrTabStatsDragMouseY);
}

sdword tabStatsToggleDisplayOption(char keycode)
{
    switch (keycode)
    {
        case ONEKEY:
            mrTabStatsShowGuns ^= TRUE;
            return TRUE;
        case TWOKEY:
            mrTabStatsShowArmour ^= TRUE;
            return TRUE;
        case THREEKEY:
            mrTabStatsShowTechShipHas ^= TRUE;
            return TRUE;
        case FOURKEY:
            mrTabStatsShowAllTech ^= TRUE;
            return TRUE;
        case FIVEKEY:
            mrTabStatsShowMaxTech ^= TRUE;
            return TRUE;
        case SIXKEY:
            mrTabStatsShowTechShipNeeds ^= TRUE;
            return TRUE;
        case SEVENKEY:
            mrTabStatsShowClass ^= TRUE;
            return TRUE;
        case EIGHTKEY:
            mrTabStatsShowHealth ^= TRUE;
            return TRUE;
        default:
            return FALSE;
    }
}

static void mrTabStatsDrawOptionLine(sdword index, bool32 enabled, strGamesMessages labelString)
{
    char line[256];
    sdword y;
    char *status;

    status = enabled ? strGetString(strMO_OptionEnabled) : strGetString(strMO_OptionDisabled);

    sprintf(line, "(%d) %s %s",
            index + 1,
            strGetString(labelString),
            status);
    y = 20 + index * fontHeight(" ");
    fontPrint(20, y, enabled ? colWhite : colRGB(160, 160, 160), line);
}

static void mrTabStatsDrawDisplayOptionStatus(void)
{
    rectangle rect;
    sdword rowHeight;
    sdword width;

    rowHeight = fontHeight(" ");
    width = fontWidth("Show tech ship needs OFF") + 50;

    rect.x0 = 10;
    rect.y0 = 10;
    rect.x1 = width;
    rect.y1 = 20 + 8 * rowHeight + 10;
    primRectTranslucent2(&rect, colRGBA(50, 50, 50, 100));
    primRoundRectOutline(&rect, 2, colWhite, 4, 4);

    mrTabStatsDrawOptionLine(0, mrTabStatsShowGuns, str_SO_ShowGuns);
    mrTabStatsDrawOptionLine(1, mrTabStatsShowArmour, str_SO_ShowArmour);
    mrTabStatsDrawOptionLine(2, mrTabStatsShowTechShipHas, str_SO_ShowTechShipHas);
    mrTabStatsDrawOptionLine(3, mrTabStatsShowAllTech, str_SO_ShowAllTech);
    mrTabStatsDrawOptionLine(4, mrTabStatsShowMaxTech, str_SO_ShowMaxTech);
    mrTabStatsDrawOptionLine(5, mrTabStatsShowTechShipNeeds, str_SO_ShowTechShipNeeds);
    mrTabStatsDrawOptionLine(6, mrTabStatsShowClass, str_SO_ShowClass);
    mrTabStatsDrawOptionLine(7, mrTabStatsShowHealth, str_SO_ShowHealth);
}

static void mrTabStatsPrintMeasuredString(sdword x, sdword y, color c, char *string, sdword *maxX, sdword *maxY, bool32 draw)
{
    sdword right;
    sdword bottom;

    right = x + fontWidth(string);
    bottom = y + fontHeight(" ");
    if (right > *maxX)
    {
        *maxX = right + 10;
    }
    if (bottom > *maxY)
    {
        *maxY = bottom + 10;
    }
    if (draw)
    {
        fontPrint(x, y, c, string);
    }
}

static char *mrShipExperienceLevelName(Ship *ship)
{
    if (ship->experienceLevel > 4)
    {
        return NULL;
    }

    return strGetString((strGamesMessages)(strExperienceLevel0 + ship->experienceLevel));
}

static sdword mrTabStatsDrawTechList(Ship *ship, sdword *maxX, sdword *maxY, bool32 draw)
{
    sdword index;
    sdword visibleCount;
    sdword widestLabel;
    sdword lineY;
    udword installedMask;
    udword availableMask;
    udword supportedMask;

    visibleCount = 0;
    widestLabel = 10;
    installedMask = ship->upgradeState.mask;
    availableMask = shipGetAvailablePlayerUpgradeMask(ship);
    supportedMask = shipTypeGetUpgradeMask(ship->shiptype);

    for (index = 0; index < sizeof(mrTabStatsTechTable) / sizeof(mrTabStatsTechTable[0]); index++)
    {
        udword bitMask = mrTabStatsTechTable[index].mask;
        color textColor;
        bool32 visible;
        char *label;

        if ((supportedMask & bitMask) == 0)
        {
            continue;
        }

        visible = FALSE;
        textColor = colWhite;
        if (mrTabStatsShowTechShipHas && ((installedMask & bitMask) != 0))
        {
            visible = TRUE;
            textColor = colRGB(0, 255, 0);
        }
        else if (mrTabStatsShowTechShipNeeds && ((availableMask & bitMask) != 0))
        {
            visible = TRUE;
            textColor = colRGB(255, 255, 0);
        }
        else if (!singlePlayerGame && mrTabStatsShowMaxTech)
        {
            visible = TRUE;
            textColor = colRGB(255, 0, 0);
        }
        else if (mrTabStatsShowAllTech)
        {
            visible = TRUE;
            textColor = colWhite;
        }

        if (!visible)
        {
            continue;
        }

        label = strGetString(mrTabStatsTechTable[index].string);
        if ((label != NULL) && (*label != '\0'))
        {
            lineY = mrTabStatsY + 20 + (visibleCount + 2) * fontHeight(" ");
            mrTabStatsPrintMeasuredString(mrTabStatsX, lineY, textColor, label, maxX, maxY, draw);
            widestLabel = max(widestLabel, fontWidth(label) + 10);
            visibleCount++;
        }
    }

    if (visibleCount > 0)
    {
        mrTabStatsPrintMeasuredString(mrTabStatsX, mrTabStatsY + 20, colWhite, strGetString(str_SO_Technology), maxX, maxY, draw);
    }

    return widestLabel;
}

static void mrTabStatsDrawShipSpecificStats(Ship *ship, sdword valueX, sdword *maxX, sdword *maxY, bool32 draw)
{
    sdword i;
    sdword rowCount;
    sdword headingY;
    sdword columnHeadingY;
    sdword gunLabelWidth;
    sdword sizeColumnX;
    sdword typeColumnX;
    sdword damageColumnX;
    sdword maxTypeWidth;
    char rowLabel[64];
    char damageText[64];
    udword installedTypeFlags[9];
    ShipUpgradeSlotStateBlock *slotStateBlock;

    slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;
    if (!mrTabStatsShowGuns)
    {
        return;
    }

    headingY = mrTabStatsY + 70;
    columnHeadingY = headingY + (sdword)(fontHeight(" ") * 1.5f);
    if ((slotStateBlock == NULL) || (slotStateBlock->count <= 0))
    {
        char *noGunsText;
        if (ship->shiptype == sRammingFrigate)
        {
            noGunsText = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_ION) ?
                strGetString(str_SO_RammingFrigateLaserUpgraded) :
                strGetString(str_SO_RammingFrigateLaser);
            mrTabStatsPrintMeasuredString(valueX, headingY, colRGB(0, 255, 0), noGunsText, maxX, maxY, draw);
        }
        else
        {
            mrTabStatsPrintMeasuredString(valueX, headingY, colWhite, strGetString(str_SO_NoGuns), maxX, maxY, draw);
        }
        return;
    }

    shipCollectInstalledUpgradeTypeFlags(ship, installedTypeFlags);

    gunLabelWidth = 0;
    maxTypeWidth = fontWidth(strGetString(str_SO_Type));
    rowCount = 0;
    for (i = 0; i < slotStateBlock->count; i++)
    {
        ShipUpgradeSlotStateEntry *slotState = &slotStateBlock->entries[i];
        sdword upgradeType = (sdword)slotState->state;
        char *typeString;

        if (((upgradeType == 5) && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE)) ||
            ((upgradeType == 8) && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT)) ||
            !shipCanDisplayUpgradeType(ship, upgradeType))
        {
            continue;
        }

        sprintf(rowLabel, "%s %d:", strGetString(str_SO_Gun), rowCount + 1);
        gunLabelWidth = max(gunLabelWidth, fontWidth(rowLabel));
        typeString = mrTabStatsUpgradeTypeString(upgradeType);
        maxTypeWidth = max(maxTypeWidth, fontWidth(typeString));
        rowCount++;
    }

    if (rowCount == 0)
    {
        if (ship->shiptype == sRammingFrigate)
        {
            char *ramText = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_ION) ?
                strGetString(str_SO_RammingFrigateLaserUpgraded) :
                strGetString(str_SO_RammingFrigateLaser);
            mrTabStatsPrintMeasuredString(valueX, headingY, colRGB(0, 255, 0), ramText, maxX, maxY, draw);
        }
        else
        {
            mrTabStatsPrintMeasuredString(valueX, headingY, colWhite, strGetString(str_SO_NoGuns), maxX, maxY, draw);
        }
        return;
    }

    sizeColumnX = valueX + gunLabelWidth + 10;
    typeColumnX = sizeColumnX + fontWidth(strGetString(str_SO_VeryLarge)) + 10;
    damageColumnX = typeColumnX + maxTypeWidth + 10;

    mrTabStatsPrintMeasuredString(valueX, headingY, colWhite, strGetString(str_SO_Weapons), maxX, maxY, draw);
    mrTabStatsPrintMeasuredString(sizeColumnX, columnHeadingY, colWhite, strGetString(str_SO_Size), maxX, maxY, draw);
    mrTabStatsPrintMeasuredString(typeColumnX, columnHeadingY, colWhite, strGetString(str_SO_Type), maxX, maxY, draw);
    mrTabStatsPrintMeasuredString(damageColumnX, columnHeadingY, colWhite, strGetString(str_SO_Damage), maxX, maxY, draw);

    rowCount = 0;
    for (i = 0; i < slotStateBlock->count; i++)
    {
        ShipUpgradeSlotStateEntry *slotState = &slotStateBlock->entries[i];
        GunStatic *gunStatic = slotState->gunStatic;
        sdword upgradeType = (sdword)slotState->state;
        sdword weaponClass;
        sdword rowY;
        char *typeString;
        char *sizeString;
        color typeColor;

        if (gunStatic == NULL ||
            ((upgradeType == 5) && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE)) ||
            ((upgradeType == 8) && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT)) ||
            !shipCanDisplayUpgradeType(ship, upgradeType))
        {
            continue;
        }

        sprintf(rowLabel, "%s %d:", strGetString(str_SO_Gun), rowCount + 1);
        sizeString = mrTabStatsGunSizeString(gunStatic->gunSize);
        typeString = mrTabStatsUpgradeTypeString(upgradeType);
        weaponClass = gunGetActiveSoundType(ship, (Gun *)slotState);
        if ((weaponClass == GS_BeastWeapon) || (weaponClass == GS_BigGun))
        {
            sprintf(damageText, "%s", strGetString(strNotApplicable));
        }
        else
        {
            sprintf(damageText, "%.0f",
                    tacticsGetShipDamageModifier(ship) *
                    univGetDamageMultiplier(weaponClass, 0) *
                    gunStatic->damageMult);
        }

        typeColor = installedTypeFlags[min(max(upgradeType, 0), 8)] ? colRGB(0, 255, 0) : colWhite;
        rowY = headingY + (sdword)((rowCount + 2.5f) * fontHeight(" "));
        mrTabStatsPrintMeasuredString(valueX, rowY, colWhite, rowLabel, maxX, maxY, draw);
        mrTabStatsPrintMeasuredString(sizeColumnX, rowY, colWhite, sizeString, maxX, maxY, draw);
        mrTabStatsPrintMeasuredString(typeColumnX, rowY, typeColor, typeString, maxX, maxY, draw);
        mrTabStatsPrintMeasuredString(damageColumnX, rowY, colWhite, damageText, maxX, maxY, draw);
        rowCount++;
    }
}

void mrDrawTabStats(Ship *ship)
{
    fonthandle oldFont;
    rectangle rect;
    char title[256];
    char classLine[256];
    char healthLine[256];
    char healthPercentLine[256];
    char armourLine[256];
    sdword maxX;
    sdword maxY;
    sdword valueX;
    sdword percentX;
    sdword labelWidth;
    real32 maxHealth;
    real32 healthPercent;

    if (!mrTabStatsIsEnabled() || mrTabStatsFont == FONT_InvalidFontHandle)
    {
        mrTabStatsRect.x0 = mrTabStatsRect.y0 = mrTabStatsRect.x1 = mrTabStatsRect.y1 = 0;
        return;
    }

    oldFont = fontMakeCurrent(mrTabStatsFont);
    if (ship->tacticsTalk < 2)
    {
        char *experienceName = mrShipExperienceLevelName(ship);
        if ((experienceName != NULL) && (*experienceName != '\0'))
        {
            sprintf(title, "%s   (%s)", ShipTypeToNiceStr(ship->shiptype), experienceName);
        }
        else
        {
            sprintf(title, "%s", ShipTypeToNiceStr(ship->shiptype));
        }
    }
    else
    {
        sprintf(title, "%s", ShipTypeToNiceStr(ship->shiptype));
    }

    maxHealth = ship->staticinfo->maxhealth;
    healthPercent = (maxHealth > 0.0f) ? (ship->health / maxHealth) * 100.0f : 0.0f;
    sprintf(classLine, "%s: %s", strGetString(str_SO_Class), ShipClassToNiceStr(ship->staticinfo->shipclass));
    sprintf(healthLine, "%s: %.0f / %.0f", strGetString(str_SO_Health), ship->health, maxHealth);
    sprintf(healthPercentLine, "   %.0f %%", healthPercent);
    sprintf(armourLine, "%s", mrTabStatsArmourLevelString(ship));

    maxX = 0;
    maxY = 0;
    mrTabStatsPrintMeasuredString(mrTabStatsX, mrTabStatsY, colWhite, title, &maxX, &maxY, FALSE);
    labelWidth = mrTabStatsDrawTechList(ship, &maxX, &maxY, FALSE);
    valueX = mrTabStatsX + labelWidth;
    if (mrTabStatsShowClass)
    {
        mrTabStatsPrintMeasuredString(valueX, mrTabStatsY + 20, colWhite, classLine, &maxX, &maxY, FALSE);
    }
    if (mrTabStatsShowHealth)
    {
        mrTabStatsPrintMeasuredString(valueX, mrTabStatsY + 35, colWhite, healthLine, &maxX, &maxY, FALSE);
        percentX = valueX + fontWidth(healthLine);
        mrTabStatsPrintMeasuredString(percentX, mrTabStatsY + 35, mrTabStatsHealthColor(ship), healthPercentLine, &maxX, &maxY, FALSE);
    }
    if (mrTabStatsShowArmour)
    {
        mrTabStatsPrintMeasuredString(valueX, mrTabStatsY + 50,
                                      shipHasAvailableUpgradeLevel(ship) ? colRGB(255, 255, 0) : colRGB(0, 255, 0),
                                      armourLine, &maxX, &maxY, FALSE);
    }
    mrTabStatsDrawShipSpecificStats(ship, valueX, &maxX, &maxY, FALSE);

    rect.x0 = mrTabStatsX - 10;
    rect.y0 = mrTabStatsY - 10;
    rect.x1 = maxX;
    rect.y1 = maxY;
    mrTabStatsRect = rect;

    primRectTranslucent2(&rect, colRGBA(50, 50, 50, 100));
    primRoundRectOutline(&rect, 2, colWhite, 4, 4);

    mrTabStatsPrintMeasuredString(mrTabStatsX, mrTabStatsY, colWhite, title, &maxX, &maxY, TRUE);
    maxX = rect.x1 - 10;
    maxY = rect.y1 - 10;
    labelWidth = mrTabStatsDrawTechList(ship, &maxX, &maxY, TRUE);
    valueX = mrTabStatsX + labelWidth;
    if (mrTabStatsShowClass)
    {
        mrTabStatsPrintMeasuredString(valueX, mrTabStatsY + 20, colWhite, classLine, &maxX, &maxY, TRUE);
    }
    if (mrTabStatsShowHealth)
    {
        mrTabStatsPrintMeasuredString(valueX, mrTabStatsY + 35, colWhite, healthLine, &maxX, &maxY, TRUE);
        percentX = valueX + fontWidth(healthLine);
        mrTabStatsPrintMeasuredString(percentX, mrTabStatsY + 35, mrTabStatsHealthColor(ship), healthPercentLine, &maxX, &maxY, TRUE);
    }
    if (mrTabStatsShowArmour)
    {
        mrTabStatsPrintMeasuredString(valueX, mrTabStatsY + 50,
                                      shipHasAvailableUpgradeLevel(ship) ? colRGB(255, 255, 0) : colRGB(0, 255, 0),
                                      armourLine, &maxX, &maxY, TRUE);
    }
    mrTabStatsDrawShipSpecificStats(ship, valueX, &maxX, &maxY, TRUE);

    if (kbCommandIsHitByName("Toggle_Overlay_Options"))
    {
        mrTabStatsDrawDisplayOptionStatus();
    }

    fontMakeCurrent(oldFont);
}

/*-----------------------------------------------------------------------------
    Name        : mrNeedProbeHack()
    Description : checks if a single probe is selcted and if so returns true,
                  otherwise removes it from the selection.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 mrNeedProbeHack(void)
{
    if(selSelected.numShips == 1)
    {
        if (ShipTypeIsProximitySensorType(selSelected.ShipPtr[0]->shiptype))
        {    //woo hoo!  1 ship selected and its a probe
            ProbeSpec *spec;
            spec = (ProbeSpec *) selSelected.ShipPtr[0]->ShipSpecifics;
            //no longer do checks for if a probe has moved..allow infinite moves!
            if(!spec->HaveMoved)
            {

                return TRUE;
            }
            else
            {
                //fix later
                //should give SOUND EVENT to indicate probe is no good anymore
                soundEvent(NULL, UI_ClickCancel);
            }
        }
    }
    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : mrRemoveAllProbesFromSelection
    Description : removes any probes from the current selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrRemoveAllProbesFromSelection()
{
    sdword i;
    for(i=0;i<selSelected.numShips;)
    {
        if (ShipTypeIsProximitySensorType(selSelected.ShipPtr[i]->shiptype))
        {
            selSelected.numShips--;
            selSelected.ShipPtr[i] = selSelected.ShipPtr[selSelected.numShips];
            continue;
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : salCapCorvetteInSelection
    Description : returns TRUE if a salcap corvette is in the selection
                  otherwise returns false
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 salCapCorvetteInSelection(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->specialFlags & SPECIAL_IsASalvager)
        {
            return TRUE;
        }
    }
    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : mrRemoveDerelictsAndAllPlayerShipsFromSelecting
    Description : removes any of player's own ships from the current selection
                  also removes any drones from selection too
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrRemoveDerelictsAndAllPlayerShipsFromSelecting()
{
    sdword i;
    SpaceObjRotImpTarg *target;
    for(i=0;i<selSelecting.numTargets;)
    {
        target = selSelecting.TargetPtr[i];
        if (target->objtype == OBJ_DerelictType)
        {
            if (((Derelict *)target)->derelicttype != HyperspaceGate)
            {
                if(!salCapCorvetteInSelection((SelectCommand *)&selSelected))
                {
                    //special case luke suggested hack
                    goto removeit;
                }
            }
        }
        if (target->objtype == OBJ_ShipType)
        {
            if ((((Ship *)target)->playerowner == universe.curPlayerPtr) ||
                (allianceIsShipAlly((Ship *)target, universe.curPlayerPtr)) ||
                ShipTypeIsTargetDroneType(((Ship *)target)->shiptype))
            {
removeit:
                selSelecting.numTargets--;
                selSelecting.TargetPtr[i] = selSelecting.TargetPtr[selSelecting.numTargets];
                continue;
            }
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        :  mrProbeHack
    Description :  Called when a single probe was selected and the Movement mech
                    was being started...
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrProbeHack()
{
    if(!smSensorsActive)
    {   //start sensors manager if it isn't active already
        smSensorsBegin(NULL, NULL);     //step one complete!
    }
    if(!smSensorsDisable && !smZoomingIn)
    {   //if sensors aren't disabled AND we aren't zooming in
        piePointModeOnOff();
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrStartup
    Description : Start main region processor logic
    Inputs      : void
    Outputs     : Many many things...
    Return      : void
----------------------------------------------------------------------------*/
void mrStartup(void)
{
    sdword index;

    mrInitSingleClickSpecialNoTargetMessages();

    ghMainRegion = regChildAlloc(NULL, 0, 0, 0,             //create main region
                   MAIN_WindowWidth, MAIN_WindowHeight, 0,
                   RPE_LeftClickButton | RPE_HoldLeft |
                   RPE_RightClickButton | RPE_HoldRight |
                   RPE_WheelUp | RPE_WheelDown | RPE_DoubleLeft | RPE_DoubleCentre);
    regDrawFunctionSet(ghMainRegion, mrRegionDraw);         //set it's functions
    regFunctionSet(ghMainRegion, mrRegionProcess);

    for (index = 0; mrKeyFunction[index].nKeys != 0; index++)
    {                                                       //now set all the keys
        regKeyChildAlloc(ghMainRegion, mrKeyFunction[index].key[0], mrKeyFunction[index].filter,
                         mrRegionProcess, mrKeyFunction[index].nKeys,
                         mrKeyFunction[index].key[0], mrKeyFunction[index].key[1],
                         mrKeyFunction[index].key[2], mrKeyFunction[index].key[3]);
    }

    mrBigFont = frFontRegister("Arial_b17.hff");
    mrFormationFont = frFontRegister("default.hff");
    mrTabStatsStartup();
}

/*-----------------------------------------------------------------------------
    Name        : mrShutdown
    Description : Shuts down main region
    Inputs      : void
    Outputs     : Destroys ghMainRegion and all child windows
    Return      : void
----------------------------------------------------------------------------*/
void mrShutdown(void)
{
    regRegionDelete(ghMainRegion);
}

/*-----------------------------------------------------------------------------
    Callback functions for right-click menus:
-----------------------------------------------------------------------------*/
void mrDockingOrders(char *string, featom *atom)
{
    if ((!playPackets) && (!universePause) && (!mrDisabled))
    {
        if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bDock)
        {
            mrMenuDontDisappear = TRUE;
        }
        else
        {
            MakeShipHostsIncludeAttachedShips((SelectCommand *)&selSelected);
            if (selSelected.numShips <= 0)
            {
                soundEvent(NULL, UI_MovementGUIoff);
            }
            else
            {
                soundEvent(NULL, UI_Click);
                clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selSelected, 0, NULL);
            }
        }
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nDocking orders");
#endif
}
void mrDeltaFormation(char *string, featom *atom)
{
    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormDelta)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(DELTA_FORMATION);
}
void mrBroadFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormBroad)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(BROAD_FORMATION);
}

void mrXFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormX)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(DELTA3D_FORMATION);
}

void mrClawFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormClaw)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(CLAW_FORMATION);
}

void mrWallFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormWall)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(WALL_FORMATION);
}

void mrSphereFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormSphere)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(SPHERE_FORMATION);
}

void mrPicketFormation(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextFormCustom)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetTheFormation(CUSTOM_FORMATION);
}

void mrHarvestResourcesTargetResource(Resource *resource)
{
    MaxSelection tempSelection;
    sdword i;
    bool32 playFoundResourceFeedback;

    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bHarvest)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    if (selSelected.numShips > 0)
    {
        MakeShipsHarvestCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected, FALSE);
        if (tempSelection.numShips > 0)
        {
            if (resource != NULL)
            {
                Resource *targetResource = resource;

                if (resource->unk_1C == 0 &&
                    shipModuleSlotIsEligible((Ship *)resource, universe.curPlayerPtr))
                {
                    targetResource = (Resource *)mrShipGetModuleSlotEffect((Ship *)resource);
                }
                speechEventQueue(tempSelection.ShipPtr[0], 67111075, tempSelection.ShipPtr[0]->shiprace, -1, -1, -1, -1, 0.0f, -1);
                clWrapCollectResource(&universe.mainCommandLayer, (SelectCommand *)&tempSelection, targetResource);
                soundEventStartEngine(tempSelection.ShipPtr[0]);
                tutGameMessage("Game_ClickHarvest");
            }
            else
            {
                playFoundResourceFeedback = TRUE;
                for (i = 0; i < tempSelection.numShips; i++)
                {
                    SelectCommand selectOne;

                    selectOne.numShips = 1;
                    selectOne.ShipPtr[0] = tempSelection.ShipPtr[i];
                    clCollectNearestResource(&universe.mainCommandLayer, &selectOne, TRUE, playFoundResourceFeedback);
                    playFoundResourceFeedback = FALSE;
                }
            }
        }
    }
}

void mrHarvestResources(char *string, featom *atom)
{
    (void)atom;
    mrHarvestResourcesTargetResource((Resource *)string);
}
void mrBuildShips(char *string, featom *atom)
{
    (void)atom;

    if ((playPackets) || (universePause) || (mrDisabled)) return;

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildManager)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    soundEvent(NULL, UI_Click);
    if (selSelected.numShips == 1)
    {
        Ship *selectedShip = selSelected.ShipPtr[0];
        ShipType shiptype = selectedShip->shiptype;

        if (!(selectedShip->specialFlags2 & SPECIAL_2_DockFlag) &&
            selectedShip->specialFlags3 == 0 &&
            (shiptype == sMothership ||
             shiptype == sCarrier ||
             shiptype == bMothership ||
             shiptype == bCarrier))
        {
            cmConstructionBegin(ghMainRegion, (sdword)selectedShip, 0, 0);
            goto buildships_done;
        }
    }

    cmConstructionBegin(ghMainRegion, 0, 0, 0);

buildships_done:
    ;
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nBuild ships.");
#endif
}

void mrTradeStuff(char *string, featom *atom)
{

        tmTradeBegin(ghMainRegion, 0, 0, 0);
}





void mrMoveSelectionPointSpecified(vector *destination)
{
    MaxSelection attackingShips;
    Ship *ship;
    vector distancevec;
    sdword index;

    MakeShipsMoveCapable((int *)&selSelected, 1);
    if (selSelected.numShips > 0)
    {
        if (!_isnan(destination->x) && !_isnan(destination->y) && !_isnan(destination->z))
        {
            selSelectionCopyBounded(&attackingShips, (MaxSelection *)&selSelected);
            index = 0;
            if (attackingShips.numShips > 0)
            {
                sdword numShips = attackingShips.numShips;
                Ship **iter = attackingShips.ShipPtr;
                do
                {
                    ship = *iter;
                    if (ship->tacticstype == Aggressive &&
                        ship->command != NULL &&
                        ship->command->ordertype.order == COMMAND_ATTACK)
                    {
                        index++;
                        iter++;
                    }
                    else
                    {
                        attackingShips.numShips = numShips - 1;
                        *iter = attackingShips.ShipPtr[numShips - 1];
                        numShips = attackingShips.numShips;
                    }
                }
                while (index < numShips);
                if (numShips > 0)
                    clWrapSetTactics(&universe.mainCommandLayer, (SelectCommand *)&attackingShips, Neutral);
            }

            if (MP_HyperSpaceFlag)
            {
                vecSub(distancevec, selCentrePoint, *destination);
                if (universe.curPlayerPtr->resourceUnits >= hyperspaceCost(fsqrt(vecMagnitudeSquared(distancevec)), (SelectCommand *)&selSelected))
                {
                    clWrapMpHyperspace(&universe.mainCommandLayer, (SelectCommand *)&selSelected, selCentrePoint, *destination);
                }
                else if (battleCanChatterAtThisTime(BCE_CannotComply, selSelected.ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selSelected.ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
            }
            else
            {
                clWrapMovePoint(&universe.mainCommandLayer, (SelectCommand *)&selSelected, selCentrePoint, *destination);
                if (*(sdword *)((ubyte *)selSelected.ShipPtr[0]->staticinfo + 488))
                {
                    speechEventQueue(selSelected.ShipPtr[0], 0x04000801, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
                }
                else
                {
                    speechEventQueue(
                        selSelected.ShipPtr[0],
                        0x04000801,
                        selSelected.ShipPtr[0]->shiptype,
                        -1,
                        0,
                        selSelected.ShipPtr[0]->playerowner->playerIndex,
                        -1,
                        5.0f,
                        -1);
                }
            }
        }
    }

    soundEventPlay(0, 279, 0);
    piePointModeOnOff();
}

void mrMoveShips(char *string, featom *atom)
{
    vector dist;

    if ((playPackets) || (mrDisabled))
    {
        return;
    }

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bMove)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    makeShipsControllable((SelectCommand *)&selSelected,COMMAND_MOVE);
    if(mrNeedProbeHack())                                   //performs needed stuff for probe hack and
    {                                                       //removes probes from selection if other ships there!
        mrProbeHack();
    }
    else
    {
        if (mrHoldRight != mrNULL || mrHoldLeft != mrNULL)
        {
            return;
        }
        else if (selSelected.numShips >= 1)
        {   //single probe not selected so do normal movement....
    //        universe.mainCameraCommand.actualcamera
            selCentrePointCompute();
            vecSub(dist,selCentrePoint,universe.mainCameraCommand.actualcamera.lookatpoint);
            if (piePointSpecMode != PSM_Idle)
            {
                soundEvent(NULL, UI_MovementGUIoff);
                mrRemoveAllProbesFromSelection();
                piePointModeOnOff();
            }
            else if (vecMagnitudeSquared(dist) <= MAX_MOVE_DISTANCE)
            {
                mrRemoveAllProbesFromSelection();
                piePointModeBegin(PiePointSpecTypeMove, mrMoveSelectionPointSpecified);
            }
            else
            {
                if (piePointSpecMode==PSM_Idle)
                {
                    smSensorsBegin(NULL,NULL);
                    mrRemoveAllProbesFromSelection();
                    piePointModeBegin(PiePointSpecTypeMove, mrMoveSelectionPointSpecified);
                }
            }
        }
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nMove ships.");
#endif
}
void mrInfo(char *string, featom *atom)
{
    dbgMessagef("\nInfo - stubbed out.");
}
void mrCancel(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bCancelCommand)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }

    if (selSelected.numShips > 0)
    {
        clWrapHalt(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nAll stop - cancel orders.");
#endif
}
void mrScuttle(char *string, featom *atom)
{
    static real32 confirmTime=-7.0f;    //initial value

    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bScuttle)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if(selSelected.numShips == 0)
        return;

    if (!singlePlayerGame)
    {
        if (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
        {
            MakeShipsCanBeRetiredOrScuttled((int *)&selSelected);
            if(selSelected.numShips <= 0)
            {
                if (battleCanChatterAtThisTime(BCE_CannotComply, selSelected.ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selSelected.ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
                return;
            }
        }
    }

    makeShipsScuttleable((int *)&selSelected, 1);
    makeShipsRetireable((SelectCommand *)&selSelected);

    if (selSelected.numShips <= 0)
        return;

    if((universe.realtimeelapsed - confirmTime) < TW_ScuttleReconfirmTime)
    {
        tutGameMessage("Game_ScuttleConfirm");
        soundEventPlay(0, 264, 0);
        if (battleCanChatterAtThisTime(BCE_COMM_F_ScuttleReaffirm, selSelected.ShipPtr[0]))
        {
            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_F_ScuttleReaffirm, selSelected.ShipPtr[0], selSelected.ShipPtr[0]->shiptype);
        }
        confirmTime = -TW_SCUTTLE_CONFIRM_TIME;
        clWrapScuttle(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
    }
    else
    {
        speechEventQueue(NULL, 0x4002088, -1, -1, 0,
                         selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
        tutGameMessage("Game_ScuttleRequest");
        confirmTime = universe.realtimeelapsed;
    }
}
void mrRetire(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bRetire)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    if(selSelected.numShips == 0)
        return;

    if (!singlePlayerGame)
    {
        if (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
        {
            MakeShipsCanBeRetiredOrScuttled((int *)&selSelected);
            if(selSelected.numShips <= 0)
            {
                if (battleCanChatterAtThisTime(BCE_CannotComply, selSelected.ShipPtr[0]))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CannotComply, selSelected.ShipPtr[0], SOUND_EVENT_DEFAULT);
                }
                return;
            }
        }
    }
    makeShipsDockCapable((SelectCommand *)&selSelected, TRUE);
    makeShipsRetireable((SelectCommand *)&selSelected);
    if (selSelected.numShips > 0)
    {
        tutGameMessage("KB_Retire");
        soundEventPlay(0, 264, 0);
        clWrapDock(&universe.mainCommandLayer,(SelectCommand *)&selSelected,DOCK_FOR_RETIRE,NULL);
    }
#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nRetire orders");
#endif
}
void mrUpdateHyperspaceStatus(bool32 goForLaunch)
{
}
void mrHyperspace(char *string, featom *atom)
{
    (void)string;
    (void)atom;

    if (singlePlayerGame)
    {
        if (spHyperspaceObjectiveActive)
        {
            spHyperspaceButtonPushed();
        }
    }
    else if (bitTest(tpGameCreated.flag, MG_Hyperspace))
    {
        MakeShipsMobile((SelectCommand *)&selSelected);
        makeShipsControllable((SelectCommand *)&selSelected, COMMAND_MP_HYPERSPACEING);
        makeSelectionHyperspaceCapable((SelectCommand *)&selSelected);
        if (selSelected.numShips != 0)
        {
            if (!smSensorsActive)
            {
                smSensorsBegin(NULL, NULL);
            }
            if (!smSensorsDisable && !smZoomingIn)
            {
                MP_HyperSpaceFlag = TRUE;
                if (piePointSpecMode == PSM_Idle)
                {
                    piePointModeBegin(PiePointSpecTypeMove, mrMoveSelectionPointSpecified);
                }
            }
        }
        else
        {
            if (piePointSpecMode != PSM_Idle)
            {
                soundEvent(NULL, UI_MovementGUIoff);
                piePointModeOnOff();
            }
            clCommandMessage(mrNoHyperspacingShipsAvailableCommandMessage, 0xFF0000FF);
        }
    }
}
void mrLaunch(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    soundEvent(NULL, UI_Click);

    if (selSelected.numShips == 1)
    {
        lmLaunchBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
    }
    else
    {
        lmLaunchBegin(ghMainRegion, 0, 0, 0);
    }

    dbgMessagef("\nLaunch Ships.");
}
void mrResearch(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    soundEvent(NULL, UI_Click);

    rmResearchGUIBegin(ghMainRegion, 0, 0, 0);

#if MR_VERBOSE_LEVEL >= 1
    dbgMessage("\nResearch.");
#endif
}
void mrEvasiveTactics(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bEvasive)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetSelectionTactics(Evasive, FALSE);
}
void mrNeutralTactics(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bNeutral)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetSelectionTactics(Neutral, FALSE);
}
void mrAgressiveTactics(char *string, featom *atom)
{
    if ((playPackets) || (universePause) || (mrDisabled))
    {
        return;
    }

    if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bAgressive)
    {
        mrMenuDontDisappear = TRUE;
        return;
    }
    mrSetSelectionTactics(Aggressive, FALSE);
}

void mrFormAlliance(char *string, featom *atom)
{
    if (playerClickedOn!=-1)
    {
        allianceFormWith(playerClickedOn);
    }
}

void mrBreakAlliance(char *string, featom *atom)
{
    if (playerClickedOn!=-1)
    {
        allianceBreakWith(playerClickedOn);
    }
}

void mrTransferRUS(char *string, featom *atom)
{
    if (playerClickedOn!=-1)
    {
        gcRUTransferStart((uword)playerClickedOn);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrSelectRectBuild
    Description : builds a selection rect based upon mouse coords
    Inputs      : dest - where to put the result
                  anchorX, anchorY - coordinates of the 'anchor' point of
                    the selection rect
    Outputs     : fills in mrSelectionRect
    Return      : void
----------------------------------------------------------------------------*/
void mrSelectRectBuild(rectangle *dest, sdword anchorX, sdword anchorY)
{
    dest->x0 = anchorX;                                     //build selection rect
    dest->y0 = anchorY;
    dest->x1 = mouseCursorX();
    dest->y1 = mouseCursorY();
    if (dest->x0 > dest->x1)                                //make sure first corner above and left of second corner
    {
        swapInt(dest->x0, dest->x1);
    }
    if (dest->y0 > dest->y1)
    {
        swapInt(dest->y0, dest->y1);
    }
    dest->x1++;                                             //make sure rect draws inclusively
    dest->y1++;                                             // to lower right corner
}

/*-----------------------------------------------------------------------------
    Name        : mrSelectionUndo
    Description : Undo callback to reset selection
    Inputs      : length, userData - MaxSelection substructure
    Outputs     : Copies a selection of length userID to selSelected
    Return      : TRUE if successful
----------------------------------------------------------------------------*/
/*
bool32 mrSelectionUndo(sdword userID, ubyte *userData, sdword length)
{
    selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)userData);
    ioUpdateShipTotals();
    return TRUE;
}
*/
/*-----------------------------------------------------------------------------
    Name        : mrHotKeyAssignUndo
    Description : Undo callback to reset selection
    Inputs      : length, userData - MaxSelection substructure
                  userID - hot key #
    Outputs     : Copies a selection of length userID to selSelected
    Return      :
----------------------------------------------------------------------------*/
/*
bool32 mrHotKeyAssignUndo(sdword userID, ubyte *userData, sdword length)
{
    selHotKeyNumbersSet(&selHotKeyGroup[userID], SEL_InvalidHotKey);
    selSelectionCopy((MaxAnySelection *)&selHotKeyGroup[userID], (MaxAnySelection *)userData);
    selHotKeyNumbersSet(&selHotKeyGroup[userID], selHotKeyNumber(userID));
    return TRUE;
}
*/
/*-----------------------------------------------------------------------------
    Name        : mrCanZBandBox
    Description : Determine if we can z-bandbox right now
    Inputs      : bFriendlies - we can bandbox friendlies
                  bEnemies - we can bandbox enemies
    Outputs     :
    Return      : we can bandbox at all
----------------------------------------------------------------------------*/
bool32 mrCanZBandBox(bool32 *bFriendlies, bool32 *bEnemies)
{
    sdword index ;
    ShipStaticInfo *shipstatic;
    *bEnemies = *bFriendlies = FALSE;
    if (selSelected.numShips == 0)
    {
        return(FALSE);
    }
    for (index = 0; index < selSelected.numShips; index++)
    {
        shipstatic = (ShipStaticInfo *)selSelected.ShipPtr[index]->staticinfo;
        if (shipstatic->custshipheader.CustShipSpecialTargetRaw != NULL)
        {
            if (shipstatic->canSpecialBandBoxFriendlies)
            {
                *bFriendlies = TRUE;
                return(TRUE);
            }
            else
            {
                *bEnemies = TRUE;
                return(TRUE);
            }
        }
    }
    return(*bFriendlies || *bEnemies);
}

/*-----------------------------------------------------------------------------
    Name        : various
    Description : Main region processor functions, called by function pointer
    Inputs      : void
    Outputs     : various
    Return      : void
----------------------------------------------------------------------------*/
//do nothing
void mrNULL(void)
{
    ;                                                       //do nothing
}
//drag the selection box
void mrSelectHold(Camera *camera, sdword clickNode)
{
    Node *startNode = mrBandboxSelectionStartNode(clickNode);
    bool32 bFriendlies, bEnemies;
    MaxSelection tempSelection;

    if (universePause || mrDisabled)
    {
        attackcom.numTargets = 0;
    }
    if (mrDisabled)
    {
        return;
    }
    if (abs(mouseCursorX() - mrOldMouseX) >= selClickBoxWidth ||
        abs(mouseCursorY() - mrOldMouseY) >= selClickBoxHeight)
    {                                                       //if mouse has moved from anchor point
        mrSelectRectBuild(&mrSelectionRect, mrOldMouseX, mrOldMouseY);//create a selection rect
        if (keyIsHit(GKEY) || (keyIsHit(CONTROLKEY) && keyIsHit(ALTKEY)))
        {                                                   //guard mode
            selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, TRUE, FALSE, FALSE);
            if ((playPackets) || (universePause) || (mrDisabled))
            {                           // if in recorded packet playback then can't issue a gaurd order.
                attackcom.numTargets = 0;
            }
        }
        else if (keyIsHit(CONTROLKEY))
        {
            if (keyIsHit(SHIFTKEY))
            {                                               //ctrl-shift - select anything targetable
                selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, FALSE, TRUE, TRUE);
                if ((playPackets) || (universePause) || (mrDisabled))
                {                           // if in recorded packet playback then can't issue a gaurd order.
                    attackcom.numTargets = 0;
                }
            }
            else
            {                                               //ctrl - select all enemies
                selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, FALSE, TRUE, TRUE);
                if ((playPackets) || (universePause) || (mrDisabled))
                {                           // if in recorded packet playback then can't issue a gaurd order.
                    attackcom.numTargets = 0;
                }

                selSelectionCopyBounded(&tempSelection, (MaxSelection *)&selSelected);
                if (!MakeShipsSingleClickSpecialCapable((SelectCommand *)&tempSelection, (SelectCommand *)&tempSelection))
                {                                           //if no ships single-click special attack capable
                    mrRemoveDerelictsAndAllPlayerShipsFromSelecting();
                }

                if (ShiptypeInSelection((SelectCommand *)&selSelected, kSalCapCorvette))
                {
                    MakeTargetsSalvageable((SelectAnyCommand *)&attackcom, universe.curPlayerPtr);
                }
                else
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&attackcom, universe.curPlayerPtr);//always remove allied/player ships from selection
                }
            }
        }
        else if (keyIsHit(ALTKEY))
        {                                                   //alt - select any ships
            selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, FALSE, FALSE, FALSE);
        }
        else if (kbCommandKeyIsHit(kbSHIP_SPECIAL))
        {                                                   //special action modifier
            if (mrCanZBandBox(&bFriendlies, &bEnemies))
            {
                if ((playPackets) || (universePause) || (mrDisabled))
                {                           // if in recorded packet playback then can't issue a gaurd order.
                    attackcom.numTargets = 0;
                }

                if (bEnemies)
                {                                           //select enemies and friends
                    selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, FALSE, FALSE, FALSE);
                }
                else
                {                                           //select just friends
                    selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, TRUE, FALSE, FALSE);
                }
                if (!bFriendlies)
                {                                           //remove friends from selection
                    if (ShiptypeInSelection((SelectCommand *)&selSelected, kSalCapCorvette))
                    {
                        MakeTargetsSalvageable((SelectAnyCommand *)&attackcom, universe.curPlayerPtr);
                    }
                    else
                    {
                        mrRemoveDerelictsAndAllPlayerShipsFromSelecting();
                    }
                }
            }
            else
            {
                selRectNone();
            }
        }
        else
        {                                                   //select player's ships
            selRectDragFunction(startNode, camera, &mrSelectionRect, attackcom.TargetPtr, &attackcom.numTargets, TRUE, FALSE, FALSE);
        }
    }
    else
    {                                                       //else mouse hasn't moved far enough
        mrSelectionRect.x0 = mrSelectionRect.x1 =           //select nothing
            mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
        attackcom.numTargets = 0;
    }
}
//rotate and/or zoom the camera
void mrCameraMotion(void)
{
    rectangle rect;
    sdword cx, cy;
    if (helpinfoactive)
    {
        if (screenX < helpinforegion->rect.x0 ||
            screenY < helpinforegion->rect.y0 ||
            screenX >= helpinforegion->rect.x1 ||
            screenY >= helpinforegion->rect.y1)
        {
            cx = helpinforegion->rect.x0 / 2;
            cy = MAIN_WindowHeight / 2;

            if (screenX != cx ||
                screenY != cy)
            {
                mrMouseHasMoved += abs(screenX - cx);
                mrMouseHasMoved += abs(screenY - cy);
                camMouseX = cx - screenX;
                camMouseY = cy - screenY;
                mousePositionSet(cx, cy);

                rect.x0 = 0;
                rect.y0 = 0;
                rect.x1 = helpinforegion->rect.x0;
                rect.y1 = MAIN_WindowHeight;
                mouseClipToRect(&rect);
            }
        }
    }
    else
    {
        cx = MAIN_WindowWidth / 2;
        cy = MAIN_WindowHeight / 2;

        if (screenX != cx ||
            screenY != cy)
        {
            mrMouseHasMoved += abs(screenX - cx);
            mrMouseHasMoved += abs(screenY - cy);
            camMouseX = cx - screenX;
            camMouseY = cy - screenY;
            mousePositionSet(cx, cy);
        }

    }
}

/*-----------------------------------------------------------------------------
    Name        : mrKeyRelease
    Description : handle key releases
    Inputs      : ID - keyindex of key being released
    Outputs     : dispatches approproiate function(s) for specified key
    Return      : void
----------------------------------------------------------------------------*/
void mrKeyRelease(sdword ID)
{
    // Drew's keybinding
    ID = (sdword)kbCheckBindings(ID);

    switch (ID)
    {
        case SHIFTKEY:
            if (mrRightMouseCameraMode != 2)
            {
                piePointModeToggle(FALSE);
                ioSetSelection(TRUE);
            }
            break;
        case ZKEY:
            if (selSelected.numShips > 0 && mrHoldLeft == mrNULL && mrSpecialBandBox == FALSE)
            {
                kbActivateSelectionSpecial();
            }
            break;
        case NUMPLUSKEY:
        case PLUSKEY:
            zoomInNow = FALSE;
            break;
        case MINUSKEY:
        case NUMMINUSKEY:
            zoomOutNow = FALSE;
            break;

        default:
#if MR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nmrKeyRelease: unprocessed key = 0x%x", ID);
#endif
            break;
    }
}

#if MR_KEYBOARD_CHEATS
void mrScanDebugCodes(sdword ID);
#endif
void gpStartInGameEscapeMenu(void);

void mrHandleEscape(void)
{
    if (singlePlayerGame && popupTextNumLines)
    {
        popupTextNumLines = 0;
    }
    else if (multiPlayerGame && ViewingBuffer)
    {
        gcCancelViewingBuffer();
    }
    else if (thisNisPlaying)
    {
        if (nisCameraCutTime == 0.0f && nisScissorFadeIn == 0.0f)
        {
            nisGoToEnd(thisNisPlaying);
        }
    }
    else if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
        soundEvent(NULL, UI_MovementGUIoff);
    }
    else if (smBandboxEnabled == 1)
    {
        mrSelectionRect.x0 = mrSelectionRect.x1 =
            mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
        selSelecting.numTargets = 0;
        smBandboxEnabled = 0;
    }
    else if ((selSelected.numShips > 0) &&
             ((tutorial != TUTORIAL_ONLY) || tutEnable.bCancelSelect))
    {
        soundEvent(NULL, UI_Click);
        tutGameMessage("KB_SelectCancel");
        selSelected.numShips = 0;
        ioUpdateShipTotals();
    }
    else
    {
        if (!multiPlayerGame)
        {
            universePause = TRUE;
        }
        soundEvent(NULL, UI_Click);
        PossiblyResetTaskbar();
        tutGameMessage("SP_EscMenu");
        gpStartInGameEscapeMenu();
    }
}

bool32 NoShift(void)
{
    if (keyIsHit(SHIFTKEY) || keyIsHit(CONTROLKEY) || keyIsHit(ALTKEY))
        return FALSE;
    else
        return TRUE;
}

void mrSetTheFormation(TypeOfFormation formationtype)
{
    char msgName[256];      // for the tutorial message

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)
        return;

    strcpy(msgName, "Game_Formation_");

    makeShipsFormationCapable((SelectCommand *)&selSelected);

    if(selSelected.numShips > 0)
    {
        if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
        {
            soundEvent(NULL, UI_Click);
            mrNewFormation = PARADE_FORMATION_FLAG;
            mrFormationName = strGetString(strPARADE_FORMATION);
            speechEventQueue(NULL, 67113058, 8, -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex,
                             -1, 5.0f, -1);
            clWrapSetMilitaryParade(&universe.mainCommandLayer,(SelectCommand *)&selSelected);

            strcat(msgName, strGetString(strPARADE_FORMATION));
        }
        else
        {
            soundEvent(NULL, UI_Click);
            mrNewFormation = formationtype;
            mrFormationName = TypeOfFormationToNiceStr(formationtype);
            speechEventQueue(NULL, 67113058, (formationtype == CUSTOM_FORMATION) ? 7 : formationtype,
                             -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
            clWrapFormation(&universe.mainCommandLayer,     //set new formation
                        (SelectCommand *)&selSelected,formationtype);

            strcat(msgName, TypeOfFormationToStr(formationtype));

        }
    }

    // Draw the formation set to the screen
    mrDrawFormationTime = universe.totaltimeelapsed;
    mrDrawFormation = TRUE;

    tutGameMessage(msgName);
    mrFormationName = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : mrMothershipInFocus
    Description : Return TRUE if the specified mothership-class ship is in focus.
    Inputs      : mothership - the specified mothership
    Outputs     :
    Return      : TRUE if motherhip in focus, FALSE otherwise.
----------------------------------------------------------------------------*/
#define MR_ONLY_MOTHERSHIP          1                       //1=focus only on mothership, 0 = focus on mothership along with other ships
#define MR_NumberMothershipTypes    10
ubyte mrMothershipShipTypes[] = {sCarrier, bCarrier, kCarrier, tCarrier, sMothership, bMothership, P1Mothership, FloatingCity, ResearchStation, JunkYardHQ, 0};
Ship *mrMothershipPtr = NULL;
bool32 mrMothershipInFocus(Ship *mothership)
{
    FocusCommand *focus;

    dbgAssert(memchr(mrMothershipShipTypes, mothership->shiptype, MR_NumberMothershipTypes));
    focus = &universe.mainCameraCommand.currentCameraStack->focus;
#if MR_ONLY_MOTHERSHIP
    if (focus->numShips == 1 && focus->ShipPtr[0] == mothership)
    {                                                       //if focussed only on the mothership
        return(TRUE);
    }
    return(FALSE);
#else     11
    return(selShipInSelection(focus->ShipPtr, focus->numShips, mothership));
#endif
}

/*-----------------------------------------------------------------------------
    Name        : mrNextMothershipPtr
    Description : Return a pointer to the next mothership if the player has more
                    than one mothership.
    Inputs      : mothership - the currently focused mothership
    Outputs     :
    Return      : Next player mothership in shiplist (may be NULL or the same as mothership)
----------------------------------------------------------------------------*/
Ship *mrNextMothershipPtr(Ship *mothership)
{
    Node *shipNode;
    Ship *ship, *startingShip;
    bool32 bFirstShip = TRUE;
    sdword count = 0;

    if (mothership == NULL)
    {                                                       //mothership not defined yet
        if (universe.curPlayerPtr->PlayerMothership != NULL)
        {                                                   //get link from player's mothership
            shipNode = &universe.curPlayerPtr->PlayerMothership->shiplink;
        }
        else
        {                                                   //get link from start of shiplist
            shipNode = universe.ShipList.head;
        }
        if (shipNode == NULL) return NULL;
        startingShip = (Ship *)listGetStructOfNode(shipNode);
        if (startingShip == NULL) return NULL;      // believe it or not this check is required or it will crash under certain conditions
    }
    else
    {                                                       //get link from specified mothership
        shipNode = mothership->shiplink.next;
        if (shipNode == NULL)
        {
            shipNode = universe.ShipList.head;
        }
        if (shipNode == NULL) return NULL;
        startingShip = mothership;
    }

    while (1)
    {
        if (shipNode == NULL) return NULL;
        ship = listGetStructOfNode(shipNode);
        if (ship == NULL) return NULL;          // believe it or not this check is required or it will crash under certain conditions
        if (ship == startingShip && (!bFirstShip))
        {                                                   //if we've come all the way back to the player's mothership
            return(NULL);
        }
        if (ship->playerowner == universe.curPlayerPtr &&
            memchr(mrMothershipShipTypes, ship->shiptype, MR_NumberMothershipTypes) &&
            bitTest(ship->flags, SOF_Selectable) &&
            ship->specialFlags3 == 0)
        {                                                   //if it's the player's ship and it's a mothership type
            break;                                          //use this one as the mothership
        }
        shipNode = shipNode->next;
        if (shipNode == NULL)
        {                                                   //if reached end of shiplist
            shipNode = universe.ShipList.head;              //go back to start of shiplist
        }
        if (count >= universe.ShipList.num)
        {                                                   //if we've already looked at every ship in list
            return(NULL);
        }
        count++;
        if (shipNode == NULL) return NULL;
        bFirstShip = FALSE;
    }
    return(ship);
}

/*-----------------------------------------------------------------------------
    Name        : mrKeyPress
    Description : handle key presses
    Inputs      : ID - keyindex of key being pressed
    Outputs     : dispatches approproiate function(s) for specified key
    Return      : void
----------------------------------------------------------------------------*/
void mrKeyPress(sdword ID)
{
    if (!gameIsRunning)
    {
        return;
    }
    if (nisIsRunning && thisNisPlaying && (ID == ESCKEY || ID == SPACEKEY || ID == ENTERKEY))
    {
        if (singlePlayerGame)
        {
#if BINK
            if (binkDonePlaying)
#endif
            {
                goto processEscapeKey;
            }
        }
        else
        {
            goto processEscapeKey;
        }
    }

    //now translate
    //ID = (sdword)opKeyTranslate((keyindex)ID);

    // Drew's keybinding
    ID = (sdword)kbCheckBindings(ID);

    if (ID == CAPSLOCKKEY)
    {
        goto docapslock;        // TO always on
    }
    if (mrDisabled)
    {
        return;
    }
#if SP_DEBUGKEYS
//    if (singlePlayerGame)     // check occurs in singlePlayerCheckDebugKeys now
//    {
        if (keyIsHit(WKEY))
        {
            singlePlayerCheckDebugKeys(ID);
            return;
        }
//    }
#endif //SP_DEBUGKEYS
#if MR_KEYBOARD_CHEATS
    if keyIsHit(SHIFTKEY) mrScanDebugCodes(ID);
#endif

    switch (ID)
    {
        case SHIFTKEY:
            if (mrRightMouseCameraMode != 2)
            {
                piePointModeToggle(TRUE);
            }
            break;
        case HOMEKEY:
#if LOD_PRINT_DISTANCE
            if (!lodTuningMode)
            {
#endif
                tutGameMessage("KB_FocusMothership");
                goto focusOnMothership;
#if LOD_PRINT_DISTANCE
            }
            else
            {
                keySetSticky(HOMEKEY);
            }
#endif
            break;
        case PAGEDOWNKEY:
            gcPageDownProcess();
            break;
        case PAGEUPKEY:
            gcPageUpProcess();
            break;
        case ZEROKEY:
        case ONEKEY:
        case TWOKEY:
        case THREEKEY:
        case FOURKEY:
        case FIVEKEY:
        case SIXKEY:
        case SEVENKEY:
        case EIGHTKEY:
        case NINEKEY:
            dbgAssert((ID - ZEROKEY) < COMMAND_MAX_SHIPS);
#if NIS_PRINT_INFO
            if (keyIsHit(NKEY))
            {                                               //n-#: play an NIS or NISlet
                if (ID == ZEROKEY)
                {
                    AIVar *var;
                    var = aivarCreate("G_PlayNis");
                    aivarValueSet(var, singlePlayerGameInfo.currentMission);
                }
                else
                {
                    if (ID <= THREEKEY)
                    {
                        AIVar *var;
                        sdword nisLetNumber;
                        char *nisName, *scriptName;

                        nisLetNumber = ID - ZEROKEY;
                        if (singlePlayerNISletNamesGet(&nisName, &scriptName, nisLetNumber))
                        {
                            var = aivarCreate("G_PlayNislet");
                            aivarValueSet(var, nisLetNumber);
                        }
                    }
                }
            }
            else
#endif
            if (keyIsHit(CONTROLKEY))
            {                                               //control-# assign a hot key group
#if SP_NISLET_TEST
                if (keyIsHit(ALTKEY) && singlePlayerGame)
                {                                           //control-alt-#: attempt to test a NISlet
                    spNISletTestAttempt(ID - ZEROKEY);
                }
#endif
                if (selSelected.numShips != 0)
                {
                    bool32 bReinforced;
//                    selHotKeyNumbersSet(&selHotKeyGroup[ID - ZEROKEY], (uword)SEL_InvalidHotKey);

                    tutGameMessage("KB_GroupAssign");

                    selHotKeyGroupRemoveReferences(ID - ZEROKEY);
                    bReinforced = selSelectionIsReinforced((MaxAnySelection *)&selHotKeyGroup[ID - ZEROKEY],(MaxAnySelection *)&selSelected);
                    selSelectionCopy((MaxAnySelection *)&selHotKeyGroup[ID - ZEROKEY],(MaxAnySelection *)&selSelected);
                    selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
#if MR_VERBOSE_LEVEL >= 2
                    dbgMessagef("\nHot key group %d assigned.", ID - ZEROKEY);
#endif
                   soundEvent(NULL, UI_MovementGUIon);
                   speechEventQueue(NULL, bReinforced ? 67113060 : 67113059, ID - ZEROKEY, -1, 0,
                                    selSelected.ShipPtr[0]->playerowner->playerIndex,
                                    -1, 5.0f, -1);
                }
            }
            else if (keyIsHit(ALTKEY))
            {                                               //alt-# select and focus on a hot key group
altCase:
                if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                {
                    tutGameMessage("KB_GroupFocus");

                    selSelectHotKeyGroup(&selHotKeyGroup[ID - ZEROKEY]);
                    selHotKeyNumbersSet(ID - ZEROKEY);
                    ioUpdateShipTotals();
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
                    if (selSelected.numShips > 0)
                    {
                        ccFocus(&(universe.mainCameraCommand),(FocusCommand *)&selSelected);
#if MR_VERBOSE_LEVEL >= 2
                        dbgMessagef("\nHot key group %d selected and focused upon.", ID - ZEROKEY);
#endif
                        soundEvent(NULL, UI_Click);
                        speechEventQueue(NULL, 67113052, ID - ZEROKEY, -1, 0,
                                         selHotKeyGroup[ID - ZEROKEY].ShipPtr[0]->playerowner->playerIndex,
                                         -1, 5.0f, -1);
                    }
               }
            }
            else if (keyIsHit(SHIFTKEY))
            {                                               //shift-# add hot key group to current selection
                sdword index;
                if(tutorial && selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                    tutGameMessage("KB_GroupAddSelect");

                for (index = selHotKeyGroup[ID - ZEROKEY].numShips - 1; index >= 0; index--)
                {                                           //for all ships in hot key group
                    if( selHotKeyGroup[ID - ZEROKEY].ShipPtr[index]->collMyBlob != NULL &&
                        selHotKeyGroup[ID - ZEROKEY].ShipPtr[index]->flags & SOF_Selectable )
                    {
                        selSelectionAddSingleShip(&selSelected, selHotKeyGroup[ID - ZEROKEY].ShipPtr[index]);
                    }
                }
                selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
                selHotKeyGroupsVerify();
#endif
#if MR_VERBOSE_LEVEL >= 2
                dbgMessagef("\nHot key group %d added to selection.", ID - ZEROKEY);
#endif
                ioUpdateShipTotals();
                if (selHotKeyGroup[ID - ZEROKEY].numShips > 0)
                {
                    soundEvent(NULL, UI_Click);
                    speechEventQueue(NULL, 67113052, ID - ZEROKEY, -1, 0,
                                     selHotKeyGroup[ID - ZEROKEY].ShipPtr[0]->playerowner->playerIndex,
                                     -1, 5.0f, -1);
                }
            }
            else if (ID == mrLastKeyPressed && universe.totaltimeelapsed <= mrLastKeyTime + mrNumberDoublePressTime)
            {                                               //double-#: focus on hot-key group
                tutGameMessage("KB_GroupSelectFocus");
                goto altCase;                               //same as alt-#
            }
            else
            {                                               //plain# select a hot key group
                if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
                {
                    tutGameMessage("KB_GroupSelect");

                    selSelectHotKeyGroup(&selHotKeyGroup[ID - ZEROKEY]);
                    selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
                    ioUpdateShipTotals();
                    if (selSelected.numShips > 0)
                    {
                        soundEvent(NULL, UI_Click);
                        speechEventQueue(NULL, 67113052, ID - ZEROKEY, -1, 0,
                                         selHotKeyGroup[ID - ZEROKEY].ShipPtr[0]->playerowner->playerIndex,
                                         -1, 5.0f, -1);
                    }
                }
            }
            break;
        case SPACEKEY:
            tutGameMessage("KB_Sensors");
            smSensorsBegin(NULL, NULL);
            break;
        case ENTERKEY:
//            mrResearch(NULL, NULL);
            if (lastshiptospeak != NULL)
            {
                if (lastshiptospeak == (Ship *)-1)
                {
                    if (selHotKeyGroup[lastgrouptospeak].numShips != 0)
                    {
                        tutGameMessage("KB_FocusLast");

                        selSelectHotKeyGroup(&selHotKeyGroup[lastgrouptospeak]);
                        selHotKeyNumbersSet(lastgrouptospeak);
                        ioUpdateShipTotals();
#if SEL_ERROR_CHECKING
                        selHotKeyGroupsVerify();
#endif
                        if (selSelected.numShips > 0)
                        {
#if MR_VERBOSE_LEVEL >= 2
                            dbgMessagef("\nHot key group %d selected and focused upon.", lastgrouptospeak);
#endif
                            soundEvent(NULL, UI_Click);
                            ccFocus(&(universe.mainCameraCommand),(FocusCommand *)&selSelected);
                        }
                   }
                }
                else
                {
                    dbgMessagef("Focus on last ship: shipptr %d\n", lastshiptospeak);
                    tutGameMessage("KB_FocusLast");

                    selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)lastshiptospeak; //make a temporary dummy selection
                    selSelecting.numTargets = 1;
                    soundEvent(NULL, UI_Click);
                    ccFocus(&universe.mainCameraCommand,
                            (FocusCommand *)&selSelecting);//focus on these ships
                    selSelecting.numTargets = 0;
                }
            }
            break;
processEscapeKey:
    case ESCKEY:
            mrHandleEscape();
            break;
        case ARRLEFT:
#if NIS_TEST
            if (testPlaying != NULL && !multiPlayerGame)
            {
                if ((selSelected.numShips > 0) && (!multiPlayerGame))
                {
                    nisTestAnother(-1);
                }
            }
            else
#endif
#ifdef HW_Debug
            if (pilotView)
            {
                pilotupoffset *= 0.95f;
                dbgMessagef("\nUpOff %f",pilotupoffset);
            }
#endif
            if (keyIsHit(ALTKEY))
            {
                goto cancelfocus;
            }
            break;
        case ARRRIGHT:
#if NIS_TEST
            if (testPlaying != NULL && !multiPlayerGame)
            {
                if ((selSelected.numShips > 0) && (!multiPlayerGame))
                {
                    nisTestAnother(1);
                }
            }
            else
#endif
#ifdef HW_Debug
            if (pilotView)
            {
                pilotupoffset *= 1.05f;
                dbgMessagef("\nUpOff %f",pilotupoffset);
            }
#endif
            if (keyIsHit(ALTKEY))
            {
                goto forwardfocus;
            }
            break;
        case TABKEY:
#if 0
            if((!multiPlayerGame) && (!singlePlayerGame))
            if(keyIsHit(CONTROLKEY))
            {
                univAddDerelict(Crate,&mrCamera->lookatpoint);
                break;
            }
#endif
            if (selSelected.numShips >= MIN_SHIPS_IN_FORMATION)
            {
                if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
                {
                    mrNewFormation = PARADE_FORMATION_FLAG;
                    mrFormationName = strGetString(strPARADE_FORMATION);
                    mrFormationTime = universe.totaltimeelapsed;
                    mrDrawFormationTime = universe.totaltimeelapsed;
                    mrDrawFormation = TRUE;
                }
                else
                {
                    TypeOfFormation alreadyformed = clSelectionAlreadyInFormation(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
                    if ((alreadyformed == NO_FORMATION && mrFormationName == NULL) || (mrNewFormation == PARADE_FORMATION_FLAG))
                    {
                        mrNewFormation = 0;
                    }
                    else
                    {
                        if (keyIsHit(SHIFTKEY))
                        {
                            if (mrNewFormation <= 0)
                            {
                                mrNewFormation = CUSTOM_FORMATION - 1;
                            }
                            else
                            {
                                mrNewFormation--;
                            }
                            tutGameMessage("KB_FormationPrevious");
                        }
                        else
                        {
                            mrNewFormation++;
                            if (mrNewFormation >= CUSTOM_FORMATION)
                            {
                                mrNewFormation = 0;
                            }
                            tutGameMessage("KB_FormationNext");
                        }
                    }
                    mrFormationName = TypeOfFormationToNiceStr(mrNewFormation);
                    mrFormationTime = universe.totaltimeelapsed;
                    mrDrawFormationTime = universe.totaltimeelapsed;
                    mrDrawFormation = TRUE;
                }
                soundEvent(NULL, UI_Click);
            }
            break;
        case AKEY:
/*            if (keyIsHit(CONTROLKEY))
            {
                goto caseEKey;
            }*/
#if MAD_TEST_ANIMATION
            if (keyIsHit(ALTKEY))
            {                                               //alt-a: test mesh animation
                if (selSelected.numShips >= 1)
                {                                           //if ships selected
                    if (selSelected.ShipPtr[0]->madBindings != NULL)
                    {                                       //if this ship has an animation structure
                        if (selSelected.ShipPtr[0]->madBindings->nCurrentAnim == MAD_NoAnimation)
                        {                                   //if nothing currently playing
                            if (mrTestAnimationIndex >= selSelected.ShipPtr[0]->madBindings->header->nAnimations)
                            {                                   //if already tested them all
                                mrTestAnimationIndex = 0;       //go back to first animation
                            }
                            madAnimationStart(selSelected.ShipPtr[0], mrTestAnimationIndex);
                            mrTestAnimationIndex++;
                        }
                        else
                        {
                            if (bitTest(selSelected.ShipPtr[0]->madBindings->header->anim[selSelected.ShipPtr[0]->madBindings->nCurrentAnim].flags, MAF_Loop))
                            {                               //if looping animation
                                madAnimationStop(selSelected.ShipPtr[0]);
                            }
                            else
                            {                               //else non-looping animation
                                madAnimationPause(selSelected.ShipPtr[0], !selSelected.ShipPtr[0]->madBindings->bPaused);
                            }
                        }
                    }
                }
            }
#endif
            break;


        case BKEY:
            if (keyIsHit(CONTROLKEY) && singlePlayerGame)
            {
                //mrTradeStuffTest(NULL, NULL);

            }
            else if (NoShift())
            {
                if (playPackets)
                    break;  // packet playback or universe paused, can't go into build manager!!

                tutGameMessage("KB_Build");
                mrBuildShips(NULL, NULL);
            }
            break;

        case CKEY:
            if (NoShift())
            {
cancelfocus:
                tutGameMessage("KB_CancelFocus");
                ccCancelFocus(&(universe.mainCameraCommand));

                soundEvent(NULL, UI_ClickCancel);
            }
            break;

        case DKEY:

            if (NoShift())
            {
                tutGameMessage("KB_Dock");
                mrDockingOrders(NULL, NULL);
            }
#if MR_SOUND_RELOAD_VOLUMES
            else if (keyIsHit(CONTROLKEY))
            {
                if (keyIsHit(SHIFTKEY))
                {
                    soundMixerSetMode(SOUND_MODE_LOW);
                }
                else
                {
                    soundMixerSetMode(SOUND_MODE_NORM);
                }
            }
#endif
            break;

        case EKEY:
            caseEKey:

            if (NoShift() && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bBandSelect))
            {                                               //'E': select everyone onscreen
                rectangle fullScreen = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};
                MaxSelection tempSelection;

                tutGameMessage("KB_SelectEveryone");

                selSelectionCopy((MaxAnySelection *)&tempSelection,(MaxAnySelection *)&selSelected);
                selRectSelect(&(universe.mainCameraCommand.actualcamera),
                              &fullScreen);
                if ((tempSelection.numShips != 0) &&
                    (selSelectionCompare((MaxAnySelection *)&selSelected,(MaxAnySelection *)&tempSelection) == 0))
                {                                           //if identical selection last time E was hit
                    selSelected.numShips = 0;               //select nothing
                }
                selSelecting.numTargets = 0;
                ioUpdateShipTotals();

                soundEvent(NULL, UI_Click);
            }
            break;
        case MMOUSE_BUTTON:
        case MMOUSE_DOUBLE:
        case FKEY:
            if (keyIsHit(ALTKEY))
            {
                goto focusOnMothership;
            }
            else if ((selSelected.numShips != 0) && NoShift())
            {
                MaxSelection tempselected;

                tutGameMessage("KB_Focus");

                soundEvent(NULL, UI_Click);

                tempselected = selSelected;     //copy structure
                MakeShipHostsIncludeAttachedShips((SelectCommand *)&tempselected);
                ccFocus(&universe.mainCameraCommand,(FocusCommand *)&tempselected);
            }
            break;
        case HKEY:

            if (NoShift())
            {
                tutGameMessage("KB_Harvest");
                mrHarvestResources(NULL, NULL);

            }
            break;

        case LKEY:
#if LOD_AUTO_SAVE
            if (keyIsHit(CONTROLKEY))
            {
                lodinfo *LOD;
                if (keyIsHit(SHIFTKEY))
                {                                           //ctrl-shift-L - save LOD file
                    if (selSelected.numShips == 1)
                    {
                        if (lodTuningMode)
                        {
                            //Cataclysm: the runtime lodinfo struct no longer tracks a
                            //source path (that slot is useHomeworldLODSystem); this
                            //debug-only save is disabled until a source path is threaded through.
                            LOD = selSelected.ShipPtr[0]->staticinfo->staticheader.LOD;
                            if (lodAutoSave(LOD, NULL) != ERROR)
                            {
                                dbgMessagef("\nLOD file saved.");
                            }
                        }
                    }
                }
                else if (lodTuningMode)
                {                                           //ctrl-L - remember LOD
                    if (selSelected.numShips == 1)
                    {
                        LOD = selSelected.ShipPtr[0]->staticinfo->staticheader.LOD;
                        if (rndLOD >= 0 && rndLOD < LOD->nLevels)
                        {
                            vector shippos = selSelected.ShipPtr[0]->posinfo.position;
                            vector length;

                            vecSub(length,mrCamera->eyeposition,shippos);

                            LOD->level[rndLOD].mOn = LOD->level[rndLOD].mOff = 1.0f;
                            if ((LOD->level[rndLOD].flags & LM_LODType) == LT_SubPixel)
                            {
                                LOD->level[rndLOD].bOff = REALlyBig;
                            }
                            else
                            {
                                LOD->level[rndLOD].bOff = vecMagnitudeSquared(length) / lodScaleFactor;
                            }
                            LOD->level[rndLOD].bOn = LOD->level[rndLOD].bOff * 0.9f;
                            dbgMessagef("\nLOD #%d remembered at %.2f", rndLOD, LOD->level[rndLOD].bOff);
                        }
                    }
                }
                break;
            }
#endif //LOD_AUTO_SAVE
#if RND_GL_STATE_DEBUG
            if (keyIsHit(GKEY))
            {
                keySetSticky(LKEY);
                break;
            }
#endif //RND_GL_STATE_DEBUG


            if (NoShift())
            {
                tutGameMessage("KB_Launch");
                mrLaunch(NULL, NULL);
            }
            break;

        case LBRACK:
            if(keyIsHit(SHIFTKEY))
            {
                if (RGL) rglFeature(RGL_GAMMA_DN);
                shGammaDown();
                break;
            }
            if(selSelected.numShips > 0)
            {
                sdword curTactics = selSelected.ShipPtr[0]->tacticstype;
                curTactics--;
                if(curTactics < 0)
                    curTactics = 2;

                soundEvent(NULL, UI_Click);

                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,curTactics);

                tutGameMessage("KB_TacticsPrevious");
            }
            break;
        case RBRACK:
            if(keyIsHit(SHIFTKEY))
            {
                if (RGL) rglFeature(RGL_GAMMA_UP);
                shGammaUp();
                break;
            }
            if(selSelected.numShips > 0)
            {
                sdword curTactics = selSelected.ShipPtr[0]->tacticstype;
                curTactics++;
                if(curTactics > 2)
                    curTactics = 0;

                soundEvent(NULL, UI_Click);

                clWrapSetTactics(&universe.mainCommandLayer,(SelectCommand *)&selSelected,curTactics);

                tutGameMessage("KB_TacticsNext");
            }
            break;
        case JKEY:
            if ((!singlePlayerGame) && (bitTest(tpGameCreated.flag,MG_Hyperspace)))
            {
                makeShipsNotIncludeSinglePlayerMotherships((SelectCommand *)&selSelected);
                makeShipsControllable((SelectCommand *)&selSelected,COMMAND_MP_HYPERSPACEING);
                makeSelectionHyperspaceCapable((SelectCommand *)&selSelected);
                if(selSelected.numShips == 0)
                {
                    //removed all ships!
                    //if normal gui up..bring it down!
                    if (piePointSpecMode != PSM_Idle)
                    {
                        //turn it off!
                        soundEvent(NULL, UI_MovementGUIoff);
                        piePointModeOnOff();
                    }
                }
                else
                {
                    if(!smSensorsActive)
                    {   //start sensors manager if it isn't active already
                        smSensorsBegin(NULL, NULL);     //step one complete!
                    }
                    if(!smSensorsDisable && !smZoomingIn)
                    {   //if sensors aren't disabled AND we aren't zooming in
                        MP_HyperSpaceFlag=TRUE;
                        if (piePointSpecMode == PSM_Idle)
                        {
                            //turn it on if it was off!
                            soundEvent(NULL, UI_MovementGUIon);
                            piePointModeOnOff();
                        }
                    }
                    //we are now going into the sensors manager
                }
            }
            else if ( (singlePlayerGame) && (singlePlayerGameInfo.playerCanHyperspace) )
            {
                spHyperspaceButtonPushed();
            }
            break;
        case MKEY:
            if (keyIsHit(ALTKEY))
            {
                focusOnMothership:;
                {
                    Ship *mothership = mrMothershipPtr;
                    FocusCommand focus;
                    //focus on mothership
                    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bMothershipFocus)
                        return;

                    if (mrMothershipPtr == NULL || mrMothershipInFocus(mrMothershipPtr))
                    {                                       //if focussed on nothing or on a mothership
                        mrMothershipPtr = mrNextMothershipPtr(mrMothershipPtr);
                    }
                    if (mrMothershipPtr != NULL && !bitTest(mrMothershipPtr->flags, SOF_Dead))
                    {                                       //if there's a mothership to focus upon
                        focus.ShipPtr[0] = mrMothershipPtr;
                        focus.numShips = 1;
                        ccFocus(&universe.mainCameraCommand, &focus);
                        soundEvent(NULL, UI_Click);

                        tutGameMessage("KB_FocusMothership");
                    }
                }
            }
            else if (NoShift())
            {
                tutGameMessage("KB_Move");
                mrMoveShips(NULL, NULL);
            }
            break;
        case RKEY:
#if ETG_RELOAD_KEY
            if (!multiPlayerGame && keyIsHit(CONTROLKEY))
            {
                etgReset();
                break;
            }
#endif
            if (NoShift())
            {
                tutGameMessage("KB_Research");
                mrResearch(NULL, NULL);

            }
            break;

        case SKEY:

            if (NoShift())
            {
                tutGameMessage("KB_Scuttle");
                mrScuttle(NULL, NULL);
            }
            break;
        case BACKSLASHKEY:
            break;
        case IKEY:
            {
                tutGameMessage("KB_Retire");
                mrRetire(NULL, NULL);
            }
            break;
        case PKEY:
            if ((!multiPlayerGame) && NoShift())
            {
                tutGameMessage("KB_Pause");
                if ((tutorial==TUTORIAL_ONLY) && (!tutEnable.bPauseGame))
                {
                    break;
                }
                if (!universePause)
                {
                    if (piePointSpecMode!=PSM_Idle)
                    {
                        piePointModeOnOff();
                    }
                }

                universePause = !universePause;
                dbgMessage(universePause ? "\nPause ON" : "\nPause OFF");
                soundEvent(NULL, UI_Click);
            }
#if MR_SOUND_RELOAD_VOLUMES
            else if (keyIsHit(SHIFTKEY))
            {
                if (keyIsHit(CONTROLKEY))
                {
                    soundEventPause(TRUE);
                }
                else
                {
                    soundEventPause(FALSE);
                }
            }
#endif
            break;
        case QKEY:
            if (pilotView)
                bitToggle(universe.mainCameraCommand.ccMode,CCMODE_PILOT_SHIP);
            break;

#if MR_TEST_GUNS
        case TKEY:
            if (selSelected.numShips == 1 && !multiPlayerGame)
            {
                Ship *ship = selSelected.ShipPtr[0];
                ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
                if (shipstatic->custshipheader.CustShipRotateAndShoot != NULL)
                {
                    shipstatic->custshipheader.CustShipRotateAndShoot(ship, NULL, FALSE);
                }
            }
            else
                if (multiPlayerGame)
                {
                    gcChatEntryStart(keyIsHit(CONTROLKEY));
                }
            break;
#else
        case TKEY:
            if ((multiPlayerGame) && !keyIsHit(SHIFTKEY))
            {
                gcChatEntryStart(keyIsHit(CONTROLKEY));
            }
            break;
#endif
#ifndef HW_Release
#if GUN_TUNE_MODE
        case YKEY:
            if (selSelected.numShips == 1 && !multiPlayerGame)
            {
                gunTuningMode = !gunTuningMode;
            }
            break;
#endif
#endif





















        case VKEY:

            if (NoShift())
            {
forwardfocus:
            tutGameMessage("KB_ForwardFocus");
            ccForwardFocus(&universe.mainCameraCommand);

            soundEvent(NULL, UI_ClickAccept);
            }

            break;
#if NIS_TEST
        case NUMPAD1:
            if (testPlaying == NULL && !multiPlayerGame)
            {
                matrix *identity = (matrix *)&IdentityMatrix;
                if ((selSelected.numShips > 0) && (!multiPlayerGame))
                {
                    nisTest(&selSelected.ShipPtr[0]->posinfo.position, identity);
                }
            }
            else
            {
                keySetSticky(NUMPAD1);                      //reset the sticky key
            }
            break;
#endif
        case CAPSLOCKKEY:
docapslock:
            smPathMarkersEnabled ^= TRUE;
            mrDrawTactical = smPathMarkersEnabled;

            if(smPathMarkersEnabled)
            {
                soundEvent(NULL, UI_TacOverlayOn);
                tutGameMessage("KB_TacticalDisplayOn");
            }
            else
            {
                soundEvent(NULL, UI_TacOverlayOff);
                tutGameMessage("KB_TacticalDisplayOff");
            }
            break;
        case KB_CMD_TOGGLE_SHIP_INFO:
            mrTabStatsToggle();
            break;
        case KB_CMD_NEXT_OVERLAY_VIEW:
            ioOverlayPageCycle(TRUE);
            break;
        case KB_CMD_PREV_OVERLAY_VIEW:
            ioOverlayPageCycle(FALSE);
            break;
        case KB_CMD_TOGGLE_OVERLAY_OPTIONS:
            break;
        case KB_CMD_TOGGLE_OVERLAY_BACKDROPS:
            ioToggleBackdropOptions();
            break;
        case KB_CMD_BEGIN_BIG_GUN_TARGET:
            mrSelectMothership();
            break;

    case XKEY:
#ifndef HW_Release
            if(keyIsHit(CONTROLKEY))
            {
                if (!multiPlayerGame)
                {
                    universeSwitchToNextPlayer();
                }
            }
#endif
            break;

    case F1KEY:
            if(!keyIsHit(CONTROLKEY))
            {
                tutGameMessage("KB_FleetView");
                ccViewToggleMissionSphere(&universe.mainCameraCommand);
            }
            else
            {
                if(!multiPlayerGame)
                {
                    if(keyIsHit(ALTKEY))
                    {
                        universeRealTimeTweak((SelectCommand *)&selSelected);
                    }
                    else
                    {
                        tacticsShutDown();
                        tacticsStartUp();
                    }
                }
            }
            break;

        case KKEY:
            if(MakeSelectionKamikazeCapable((SelectCommand *)&selSelected) && NoShift())
            {
                CommandToDo *command;
                sdword i;
                tutGameMessage("KB_Kamikaze");
                for(i=0;i<selSelected.numShips;i++)
                {
                    command = getShipAndItsCommand(&universe.mainCommandLayer,selSelected.ShipPtr[i]);
                    if(command == NULL || (command->ordertype.order != COMMAND_ATTACK &&
                        command->ordertype.order != COMMAND_SPECIAL))
                    {
                        continue;
                    }
                    //if we get here...there is at least 1 valid ship that can kamikaze
                    break;
                }
                if(i==selSelected.numShips)
                {
                    //no valid ships!
                    if (selSelected.numShips > 0)
                        speechEvent(selSelected.ShipPtr[0],COMM_Kamikaze_NoTargets,0);
                }
                else
                {
                    soundEvent(NULL, UI_Click);
                    clWrapSetKamikaze(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
                }
            }
            break;

        case ZKEY:
            mrSpecialBandBox = FALSE;
            break;

        case F2KEY:
            if((selSelected.numShips > 0) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bEvasive))
            {
                mrSetSelectionTactics(Evasive, TRUE);
            }
            break;
        case F3KEY:
            if((selSelected.numShips > 0) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bNeutral))
            {
                mrSetSelectionTactics(Neutral, TRUE);
            }
            break;
        case F4KEY:
            if((selSelected.numShips > 0) && ((!(tutorial==TUTORIAL_ONLY)) || tutEnable.bAgressive))
            {
                mrSetSelectionTactics(Aggressive, TRUE);
            }
            break;
        case F5KEY:
        case F6KEY:
        case F7KEY:
        case F8KEY:
        case F9KEY:
        case F10KEY:
        case F11KEY:
            if (keyIsHit(SHIFTKEY))
            {
                if (ID == F6KEY)
                {
                    (void)gpQuickSave();
                }
                else if (ID == F7KEY)
                {
                    (void)gpQuickLoad();
                }
            }
            if (selSelected.numShips >= MIN_SHIPS_IN_FORMATION)
            {
                mrSetTheFormation(ID - F5KEY);
            }
            break;

#if MR_SCREENSHOTS
        case SCROLLKEY:
#if MAIN_Password
            if (mainScreenShotsEnabled)
#endif //MAIN_Password
            {
                soundEvent(NULL, UI_Click);
                rndTakeScreenshot = TRUE;
            }
            break;
/*        case PAUSEKEY:
            if (RGL)
            {
                rglFeature(RGL_MULTISHOT_END);
            }*/
            break;
#endif //MR_SCREENSHOTS
#if MR_TEST_HPB
        case OKEY:
            if (keyIsHit(CONTROLKEY))
            {
                if (madTestHPBIndex > 0)
                {
                    madTestHPBIndex--;
                }
            }
            else if (keyIsHit(SHIFTKEY))
            {
                madTestHPBIndex++;
            }
            break;
#endif
        case TILDEKEY:
            if (keyIsHit(SHIFTKEY))
            {                                               //shift-tilde select nothing
                tutGameMessage("KB_SelectNone");
                soundEvent(NULL, UI_Click);
                selSelectNone();
            }
            else if (keyIsHit(CONTROLKEY))
            {                                               //control-tilde, remove current hot-key group
                sdword index = SEL_InvalidHotKey, i;

                //find the first ship with a hotkey number displaying
                for (i=0;(index == SEL_InvalidHotKey) && (i < selSelected.numShips);i++)
                {
                    index = selHotKeyGroupNumberTest(selSelected.ShipPtr[i]);
                }

                // if the first ship is displaying a hotkey
                // and the entire selection is the same hotkey
                if ((index != SEL_InvalidHotKey) && (i==1) &&
                    selSelectionCompare((MaxAnySelection *)&selSelected,
                        (MaxAnySelection *)&selHotKeyGroup[index]) == 0)
                {
//                        selHotKeyNumbersSet(&selHotKeyGroup[index], SEL_InvalidHotKey);
                    soundEvent(NULL, UI_Click);
                    selHotKeyGroupRemoveReferences(index);
                    selHotKeyGroup[index].numShips = 0;  //select nothing for this hot key group
                    tutGameMessage("KB_GroupDelete");
#if SEL_ERROR_CHECKING
                    selHotKeyGroupsVerify();
#endif
                    break;
                }
                // if no hot key group was removed, then remove the selselected for all groups
                else if (index != SEL_InvalidHotKey)
                {
                    selHotKeyGroupRemoveReferencesFromAllGroups();
                }
            }
            else
            {                                               //else just undo
                if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bCancelCommand)
                    break;
                tutGameMessage("KB_Undo");
                udLatestThingUndo();
                soundEvent(NULL, UI_Click);
            }
            break;
        case NUMPLUSKEY:
        case PLUSKEY:
            soundEvent(NULL, UI_Click);
            zoomInNow = TRUE;
            break;
        case MINUSKEY:
        case NUMMINUSKEY:
            soundEvent(NULL, UI_Click);
            zoomOutNow = TRUE;
            break;
        case LESSTHAN:
            musicEventPrevTrack();
            break;
        case GREATERTHAN:
            musicEventNextTrack();
            break;

        default:
#if MR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nmrKeyPress: unprocessed key = 0x%x", ID);
#endif
            break;
    }
    mrLastKeyPressed = ID;
    mrLastKeyTime = universe.totaltimeelapsed;
}

//bit fields for the action mask
#define MAM_Info                0x00000001
#define MAM_Formations          0x00000002
#define MAM_Tactics             0x00000004
#define MAM_Move                0x00000008
#define MAM_Scuttle             0x00000010
#define MAM_Retire              0x00000020
#define MAM_Divider             0x00000040
#define MAM_Dock                0x00000080
#define MAM_Launch              0x00000100
#define MAM_Harvest             0x00000200
#define MAM_Build               0x00000400
#define MAM_Research            0x00000800
#define MAM_Trade               0x00001000
#define MAM_Hyperspace          0x00002000
#define MAM_BasicSet            (MAM_Info | MAM_Formations | MAM_Tactics | MAM_Move)
#define MAM_MedSet              MAM_BasicSet | MAM_Scuttle
#define MAM_ExtSet              MAM_BasicSet | MAM_Scuttle | MAM_Retire

struct
{
    udword mask;
    char *string;
}
mrActionString[] =
{
    {MAM_Info,          "CSM_Info"},
    {MAM_Formations,    "Group_menu-TO-Group_Formations_submenu"},
    {MAM_Tactics,       "Group_menu-TO-Group_Tactics_submenu"},
    {MAM_Move,          "CSM_Move"},
    {MAM_Scuttle,       "CSM_Scuttle"},
    {MAM_Retire,        "CSM_Retire"},
    {MAM_Dock,          "CSM_Dock"},
    {MAM_Hyperspace,    "CSM_Hyperspace"},
    {MAM_Launch,        "CSM_Launch"},
    {MAM_Harvest,       "CSM_Harvest"},
    {MAM_Build,         "CSM_Build"},
    {MAM_Research,      "CSM_Research"},
    {MAM_Trade,         "CSM_Trade"},
    {0,      NULL},
};
udword mrMenuActionsByShipType[TOTAL_NUM_SHIPS] =
{
    /* AdvanceSupportFrigate */  MAM_ExtSet,
    /* AttackBomber          */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* Carrier               */  MAM_ExtSet | MAM_Divider | MAM_Build | MAM_Launch,
    /* CloakedFighter        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* CloakGenerator        */  MAM_ExtSet,
    /* DDDFrigate            */  MAM_ExtSet,
    /* DefenseFighter        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* DFGFrigate            */  MAM_ExtSet,
    /* GravWellGenerator     */  MAM_ExtSet,
    /* HeavyCorvette         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* HeavyCruiser          */  MAM_ExtSet,
    /* HeavyDefender         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* HeavyInterceptor      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* IonCannonFrigate      */  MAM_ExtSet,
    /* LightCorvette         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* LightDefender         */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* LightInterceptor      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* MinelayerCorvette     */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* MissileDestroyer      */  MAM_ExtSet,
    /* Mothership            */  MAM_BasicSet | MAM_Divider | MAM_Scuttle | MAM_Build | MAM_Launch,
    /* MultiGunCorvette      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* Probe                 */  MAM_MedSet | MAM_Divider | MAM_Dock,
    /* ProximitySensor       */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* RepairCorvette        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* ResearchShip          */  MAM_ExtSet | MAM_Divider | MAM_Research,
    /* ResourceCollector     */  MAM_ExtSet | MAM_Divider | MAM_Harvest | MAM_Dock,
    /* ResourceController    */  MAM_ExtSet,
    /* SalCapCorvette        */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* SensorArray           */  MAM_ExtSet,
    /* StandardDestroyer     */  MAM_ExtSet,
    /* StandardFrigate       */  MAM_ExtSet,
    /* Drone                 */  MAM_ExtSet,
    /* TargetDrone           */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* HeadShotAsteroid      */  MAM_ExtSet,
    /* CryoTray              */  MAM_ExtSet,
    /* P1Fighter             */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P1IonArrayFrigate     */  MAM_ExtSet,
    /* P1MissileCorvette     */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P1Mothership          */  MAM_ExtSet,
    /* P1StandardCorvette    */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P2AdvanceSwarmer      */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P2FuelPod             */  MAM_ExtSet,
    /* P2Mothership          */  MAM_ExtSet,
    /* P2MultiBeamFrigate    */  MAM_ExtSet,
    /* P2Swarmer             */  MAM_ExtSet | MAM_Divider | MAM_Dock,
    /* P3Destroyer           */  MAM_ExtSet,
    /* P3Frigate             */  MAM_ExtSet,
    /* P3Megaship            */  MAM_ExtSet,
    /* FloatingCity          */  MAM_ExtSet,
    /* CargoBarge            */  MAM_ExtSet,
    /* MiningBase            */  MAM_ExtSet,
    /* ResearchStation       */  MAM_ExtSet,
    /* JunkYardDawg           */  MAM_ExtSet
};

char *mrMenuItemByFormation[] =
{
    /* DELTA_FORMATION   */ "CSM_DeltaFormation",
    /* BROAD_FORMATION   */ "CSM_BroadFormation",
    /* DELTA3D_FORMATION */ "CSM_XFormation",
    /* CLAW_FORMATION    */ "CSM_ClawFormation",
    /* WALL_FORMATION    */ "CSM_WallFormation",
    /* SPHERE_FORMATION  */ "CSM_SphereFormation",
    /* PICKET_FORMATION  */ "CSM_PicketFormation",
};

char *mrMenuItemByTactic[] =
{
    /* Evasive,     */ "CSM_Evasive",
    /* Neutral,     */ "CSM_Neutral",
    /* Aggressive,  */ "CSM_Agressive",
};

/*-----------------------------------------------------------------------------
    Name        : mrAtomSortCompare
    Description : Callback for sorting an atom list by y position.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int mrAtomSortCompare(const void *p0, const void *p1)
{
//    return(((featom *)p0)->y - ((featom *)p1)->y);
    if (((featom *)p0)->y > ((featom *)p1)->y)
    {
        return(1);
    }
    else if (((featom *)p0)->y == ((featom *)p1)->y)
    {
        return(((featom *)p1)->width - ((featom *)p0)->width);
    }
    else
    {
        return(-1);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrDeleteAllAtomsWithin
    Description : Delete all atoms from the atom list of specified screen that
                    are within the bounds of the specified atom
    Inputs      : screen - screen to delete atoms from
                  atomIndex - index of atom to get the bounds from
    Outputs     : Will delete at least 1 atom, the one specified.
    Return      : lowest index of a deleted atom
----------------------------------------------------------------------------*/
sdword mrDeleteAllAtomsWithin(fescreen *screen, sdword atomIndex)
{
    sdword index, j, lowest = SDWORD_Max;
    sdword thisX, thisY, thisWidth, thisHeight;

    thisX = screen->atoms[atomIndex].x;                     //remember location of this atom
    thisY = screen->atoms[atomIndex].y;
    thisWidth = screen->atoms[atomIndex].width;
    thisHeight = screen->atoms[atomIndex].height;
    for (index = 0; index < screen->nAtoms; index++)
    {                                                       //search all atoms
        if (screen->atoms[index].x >= thisX && screen->atoms[index].y >= thisY &&
            screen->atoms[index].x + screen->atoms[index].width <= thisX + thisWidth &&
            screen->atoms[index].y + screen->atoms[index].height <= thisY + thisHeight)
        {                                                   //if atom is inside the specified atom
            lowest = min(index, lowest);                    //remember the lowest index
            for (j = index; j < screen->nAtoms - 1; j++)
            {                                               //move the rest of the list back 1 space
                screen->atoms[j] = screen->atoms[j + 1];
            }
            index--;                                        //one less atom to check
            screen->nAtoms--;
        }
    }
    dbgAssert(lowest != SDWORD_Max);
    return(lowest);
}

/*-----------------------------------------------------------------------------
    Name        : mrMenuDisplay
    Description : Display a right-click menu for a certain set of ships
    Inputs      : actionMask - bit fields for ship types
                  currentFormation - what formation the selected ships are in, if any
                  tacticsBits - one bit for each type of tactic present in the group
    Outputs     : finds best matching menu in mrRightClickTable and starts it.
    Return      : void
----------------------------------------------------------------------------*/
void mrMenuDisplay(udword actionMask, TypeOfFormation currentFormation, udword tacticsBits)
{
#define NUMBER_GAPS         12
    fescreen *newScreen, *staticScreen;
    sdword index, j;
    char *name;
    sdword nGaps = 0;
    struct
    {
        sdword position, height;
    }
    gap[NUMBER_GAPS];
    featom *atom;

    //set the selected bit for the current formation
    staticScreen = feScreenFind("Group_Formations_submenu");
    for (index = 0; index < staticScreen->nAtoms; index++)
    {                                                       //clear all atoms of the selected bit
        bitClear(staticScreen->atoms[index].status, FAS_Checked);
    }
    //now set the formation bit on the correct formation
    if (currentFormation != NO_FORMATION)
    {
        atom = feAtomFindInScreen(staticScreen, mrMenuItemByFormation[currentFormation]);
        dbgAssert(atom != NULL);
        bitSet(atom->status, FAS_Checked);
    }
    //set the selected bit(s) for the current tactics
    staticScreen = feScreenFind("Group_Tactics_submenu");
    for (index = 0; index < staticScreen->nAtoms; index++)
    {                                                       //clear all atoms of the selected bit
        bitClear(staticScreen->atoms[index].status, FAS_Checked);
    }
    for (index = 0; index < NUM_TACTICS_TYPES; index++)
    {
        if (tacticsBits & (1 << index))
        {
            atom = feAtomFindInScreen(staticScreen, mrMenuItemByTactic[index]);
            dbgAssert(atom != NULL);
            bitSet(atom->status, FAS_Checked);
        }
    }
    //take the full menu and remove any atoms which represent the entries we are to remove
    staticScreen = feScreenFind("RightClickMenu");          //find the screen
    newScreen = memAlloc(sizeof(fescreen), "tempMenu", 0);  //allocate the temp screen
    *newScreen = *staticScreen;                             //copy the static screen to the temp screen
    if (staticScreen->nLinks > 0)
    {
        newScreen->links = memAlloc(sizeof(felink) * staticScreen->nLinks, "tempMenuLink", 0);
        memcpy(newScreen->links, staticScreen->links, sizeof(felink) * staticScreen->nLinks);
    }
    else
    {
        newScreen->links = NULL;
    }
    newScreen->atoms = memAlloc(sizeof(featom) * staticScreen->nAtoms, "tempMenuAtoms", 0);
    memcpy(newScreen->atoms, staticScreen->atoms, sizeof(featom) * staticScreen->nAtoms);
    qsort(newScreen->atoms, staticScreen->nAtoms, sizeof(featom), mrAtomSortCompare);

    //scan through the menu and remove useless atoms
    for (index = 0; index < newScreen->nAtoms; index++)
    {
        if (bitTest(newScreen->atoms[index].flags, FAF_Function))
        {                                                   //if a function atom
            name = newScreen->atoms[index].name;             //keep pointer to string
            for (j = 0; mrActionString[j].string != 0; j++)
            {                                               //for all possible actions
                if ((actionMask & mrActionString[j].mask) == 0)
                {                                           //if this action is not to be
                    if (feNamesEqual(name, mrActionString[j].string))
                    {                                       //and this atom has the action name
                        dbgAssert(nGaps < NUMBER_GAPS);
                        gap[nGaps].position = newScreen->atoms[index].y;
                        gap[nGaps].height = newScreen->atoms[index].height;
                        nGaps++;
                        index = mrDeleteAllAtomsWithin(newScreen, index);
                        index--;
                        break;
                    }
                }
            }
        }
        if (newScreen->atoms[index].type == FA_Divider)
        {                                                   //if this is a divider
            if (!bitTest(actionMask, MAM_Divider))
            {                                               //if we are not allowed dividers
                dbgAssert(nGaps < NUMBER_GAPS);
                gap[nGaps].position = newScreen->atoms[index].y;
                gap[nGaps].height = newScreen->atoms[index].height;
                nGaps++;
                index = mrDeleteAllAtomsWithin(newScreen, index);
                index--;
            }
        }
    }
    //now scan through the remaining atoms and move up any atoms below a given gap
    for (index = nGaps - 1; index >= 0; index--)
    {                                                       //for each gap
        for (j = 0; j < newScreen->nAtoms; j++)
        {                                                   //for each atom
            if (newScreen->atoms[j].y >= gap[index].position)
            {                                               //move all atoms up
                newScreen->atoms[j].y -= gap[index].height;
            }
        }
        newScreen->atoms[0].height -= gap[index].height;//decrease height of screen
    }
    //start the actual menu
    feMenuStart(ghMainRegion, newScreen, mouseCursorX(), mouseCursorY());
    feTempMenuScreen = newScreen;
#undef NUMBER_GAPS
}

/*-----------------------------------------------------------------------------
    Name        : mrClickedOnPlayer
    Description : returns the a pointer to the player you right clicked on.  NULL otherwise/
    Inputs      : none
    Outputs     : player clicked on or NULL
    Return      : void
----------------------------------------------------------------------------*/
/*sdword mrClickedOnPlayer(void)
{
    fonthandle oldfont;
    rectangle  playerColorRect;
    sdword     index;

    if ((mrDrawTactical)&&(multiPlayerGame)&&(multiPlayerGameUnderWay))
    {
        oldfont = fontMakeCurrent(selGroupFont2);

        playerColorRect.y1 = TO_PLAYERLIST_Y-fontHeight("M");
        playerColorRect.y0 = playerColorRect.y1-fontHeight("M");
        playerColorRect.x0 = TO_PLAYERLIST_X;
        playerColorRect.x1 = playerColorRect.x0 + fontHeight(" ");

        //draw the list of player names/colors
        for (index = universe.numPlayers - 1; index >= 0; index--)
        {
            playerColorRect.x1 = fontWidth(playerNames[index]) + fontHeight(" ")*2;
            if ((mouseInRect(&playerColorRect)) && (index!=sigsPlayerIndex))
            {
                return (index);
            }
            playerColorRect.y0 -= fontHeight("M")+1;//update the position
            playerColorRect.y1 -= fontHeight("M")+1;
        }

        fontMakeCurrent(oldfont);
    }

    return (-1);
}*/

bool32 mrSelectionCanProtectTarget(SelectCommand *selection, SpaceObjRotImpTarg *target)
{
    sdword selectionActions[41];
    sdword targetActions[41];
    SelectAnyCommandMax6 targetSelection;

    if (selection->numShips <= 0 || target == NULL)
    {
        return FALSE;
    }

    memset(selectionActions, 0, sizeof(selectionActions));
    memset(targetActions, 0, sizeof(targetActions));
    targetSelection.numTargets = 1;
    targetSelection.TargetPtr[0] = (TargetPtr)target;

    mrDetermineSelectionActions(selectionActions, (SelectAnyCommand *)selection);
    mrDetermineSelectionActions(targetActions, (SelectAnyCommand *)&targetSelection);
    return (mrDetermineTargetRightClickCursorType(selectionActions, targetActions, (Ship *)target) == 5);
}

sdword mrBuildProtectTargetSelection(MaxSelection *dest, SelectAnyCommand *targets)
{
    sdword targetIndex;

    dest->numShips = 0;
    if (targets == NULL)
    {
        return 0;
    }

    for (targetIndex = 0; targetIndex < targets->numTargets; targetIndex++)
    {
        Ship *targetShip = (Ship *)targets->TargetPtr[targetIndex];

        if (targetShip->objtype != OBJ_ShipType || targetShip->playerowner != universe.curPlayerPtr)
        {
            continue;
        }

        selSelectionAddSingleShipNoCheck(dest, targetShip);
    }

    return dest->numShips;
}


void mrRightClickMenu(sdword clickNode)
{
    Node *selectionListHead;
    Ship *ship;
    MaxSelection tempSelection;
    MaxSelection filteredSelection;
    SelectAnyCommand targetSelection;
    sdword selectionActions[41];
    sdword targetActions[41];
    sdword cursorType;
    sdword playerDefault;
    rectangle defaultRect = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};

    selectionListHead = (clickNode != 0) ? universe.SpaceObjList.head : universe.RenderList.head;

    playerDefault = smClickedOnPlayer(&defaultRect);
    if (mrTabStatsIsDragging() || playerDefault != -1 || mouseCursorTypeGet())
    {
        return;
    }

    piePointModePause(FALSE);
    if (mrRightMouseCameraMode == 2)
    {
        mousePositionSet(mrOldMouseX, mrOldMouseY);
        if (!mrDisabled || spHyperspaceRollCallScreen)
        {
            mouseCursorShow();
        }
        mrRightMouseCameraMode = 0;
        mouseClipToRect(&defaultRect);
    }

    if (mrDisabled || mrMouseHasMoved > mrMouseMovementClickLimit)
    {
        return;
    }

    if (piePointSpecMode != PSM_Idle)
    {
        smOverlayOpen(FALSE, NULL);
        return;
    }

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bContextMenus)
    {
        return;
    }

    if ((ship = selSelectionClick(selectionListHead,
            &(universe.mainCameraCommand.actualcamera),
            screenX, screenY, TRUE, TRUE)) != NULL)
    {                                                       //if right clicked on a ship
        if (selSelected.numShips > 0 &&
            !ShipInSelection((SelectCommand *)&selSelected, ship))
        {
            memset(selectionActions, 0, sizeof(selectionActions));
            memset(targetActions, 0, sizeof(targetActions));
            targetSelection.numTargets = 1;
            targetSelection.timeLastStatus = 0.0f;
            targetSelection.TargetPtr[0] = (TargetPtr)ship;
            mrDetermineSelectionActions(selectionActions, (SelectAnyCommand *)&selSelected);
            mrDetermineSelectionActions(targetActions, &targetSelection);
            cursorType = mrDetermineTargetRightClickCursorType(selectionActions, targetActions, ship);
            switch (cursorType)
            {
                case 1:
                    mrHarvestResourcesTargetResource((Resource *)ship);
                    soundEventPlay(NULL, 265, NULL);
                    return;

                case 2:
                    mrSalvageSelectionNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 3:
                    if (targetSelection.numTargets > 0)
                    {
                        MakeShipsAttackCapable((SelectCommand *)&filteredSelection, (SelectCommand *)&selSelected);
                        selSelectionCopyBounded((MaxSelection *)&selSelected, &filteredSelection);
                        if (selSelected.numShips > 0)
                        {
                            MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&targetSelection, universe.curPlayerPtr);
                            MakeShipsNotIncludeTheseShips((SelectCommand *)&targetSelection, (SelectCommand *)&selSelected);
                            if (targetSelection.numTargets > 0)
                            {
                                clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&targetSelection);
                            }
                        }
                    }
                    soundEvent(NULL, UI_Click);
                    return;

                case 4:
                    mrActivateSingleClickSpecialOnNearestFriendly(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 5:
                    mrProtectSelection((SelectCommand *)&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 6:
                case 7:
                case 8:
                    mrDockSelectionToShip(ship);
                    soundEvent(NULL, UI_Click);
                    return;

                case 9:
                    selSelectionCopyBounded(&tempSelection, (MaxSelection *)&selSelected);
                    MakeShipHostsIncludeAttachedShips((SelectCommand *)&tempSelection);
                    clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&tempSelection, DOCK_AT_SPECIFIC_SHIP, ship);
                    soundEventPlay(NULL, 265, NULL);
                    return;

                case 10:
                    mrLeechSelectionNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 11:
                    selSelectionCopyBounded(&tempSelection, (MaxSelection *)&selSelected);
                    selSelectionAddSingleShipNoCheck(&tempSelection, ship);
                    if (tempSelection.numShips > 0)
                    {
                        clWrapLinkSelection(&universe.mainCommandLayer, (SelectCommand *)&tempSelection);
                    }
                    soundEvent(NULL, UI_Click);
                    return;

                case 12:
                    mrIssueExternalConstructionSelection(ship);
                    soundEventPlay(NULL, 265, NULL);
                    return;

                case 13:
                    mrRepulseSelectionNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 14:
                    mrLaunchSwarmersNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 15:
                    mrRamSelectionNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 16:
                    mrFireMissilesAtNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                case 17:
                    mrInfectSelectionNearestTarget(&targetSelection);
                    soundEvent(NULL, UI_Click);
                    return;

                default:
                    break;
            }
        }

        if (ship->playerowner == universe.curPlayerPtr)
        {
            if (ship->flags & SOF_Selectable)
            {
                if (selSelected.numShips == 0)
                {                                           //else there are ships selected
                    selSelectionSetSingleShip(ship);
                }

                if (tutorial)
                    tutFEContextMenuShipType = ship->shiptype;

                smOverlayOpen(FALSE, NULL);
                soundEventPlay(NULL, 265, NULL);
                return;
            }
        }
        else
        {
#if MR_VERBOSE_LEVEL >= 1
            dbgMessagef("\nTry clicking on some of your own ships!");
#endif
        }
    }
#if MR_VERBOSE_LEVEL >= 1
    else
    {
    dbgMessagef("\nDin't click on nuttin'!");
}
#endif
}

#if MR_TEST_HPB
#define MR_ROTATE_SCALAR        0.007f
extern sdword madTestHPBIndex;
bool32 mrTestHPBMode = FALSE;
real32 mrHeading, mrPitch, mrBank;
void mrHeadingPitchHold(void)
{
    if (mouseCursorX() != MAIN_WindowWidth / 2)
    {
        mrHeading += (real32)(mouseCursorX() - MAIN_WindowWidth / 2) * MR_ROTATE_SCALAR;
        mousePositionSet(MAIN_WindowWidth / 2, mouseCursorY());
    }
    if (mouseCursorY() != MAIN_WindowHeight / 2)
    {
        mrPitch += (real32)(mouseCursorY() - MAIN_WindowHeight / 2) * MR_ROTATE_SCALAR;
        mousePositionSet(mouseCursorX(), MAIN_WindowHeight / 2);
    }
    mrTestHPBMode = TRUE;
}
void mrBankHold(void)
{
    if (mouseCursorX() != MAIN_WindowWidth / 2)
    {
        mrBank += (real32)(MAIN_WindowWidth / 2 - mouseCursorX()) * MR_ROTATE_SCALAR;
        mousePositionSet(MAIN_WindowWidth / 2, mouseCursorY());
    }
    mrTestHPBMode = TRUE;
}
#endif

// later put in shipselect.c
#define ShipIsntSelectable(x)        bitTest((x)->flags, SOF_Hide|SOF_Disabled|SOF_Crazy)

/*-----------------------------------------------------------------------------
    Name        : MakeShipsSelectable
    Description : Makes sure that the ships in this selection are capable of
                  attack.  Removes any ships from this selection that are not
                  capable of attacking.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsSelectable(SelectCommand *selection)
{
    Ship *ship;
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        if (ShipIsntSelectable(ship) || !(ship->flags & SOF_Selectable) || ship->playerowner != universe.curPlayerPtr)
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrFormationShips
    Description : Find the ships for a given mouse click, considering the
                    formation it is in.
    Inputs      : ship - ship clicked on.
    Outputs     : selection - where to store the selection
    Return      : void
----------------------------------------------------------------------------*/
void mrFormationShips(MaxSelection *selection, Ship *ship)
{
    SelectCommand *formSelection;
    MaxSelection selectedInFormation;
    bool32 parade;
    sdword i;

    formSelection = getShipAndItsCommandSelection(&universe.mainCommandLayer, ship, &parade);
    for (i = 0; i < formSelection->numShips;)
    {                                                           //if this ship is actually in formation
        Ship *candidate = formSelection->ShipPtr[i];

        if (!(candidate->flags & SOF_AvoidanceHidden) &&
            (candidate->flags & SOF_Selectable) &&
            candidate->playerowner == universe.curPlayerPtr)
        {
            i++;
        }
        else
        {
            formSelection->numShips--;
            formSelection->ShipPtr[i] = formSelection->ShipPtr[formSelection->numShips];
        }
    }

    if (formSelection->numShips <= 1)
    {
        selection->ShipPtr[0] = ship;                       //select just this ship
        selection->numShips = 1;
        memFree(formSelection);
        return;
    }

    if (parade)
    {
        selSelectionCopyByType(selection, (MaxSelection *)formSelection,
                         ship->shiptype);

        selSelectionCopyByType(&selectedInFormation, &selSelected,
                         ship->staticinfo->shiptype);
    }
    else
    {
        selSelectionCopy((MaxAnySelection *)selection, (MaxAnySelection *)formSelection);

        selSelectionCopy((MaxAnySelection *)&selectedInFormation, (MaxAnySelection *)&selSelected);
    }

    if (selSelectionCompare((MaxAnySelection *)(selection),
                            (MaxAnySelection *)(&selectedInFormation)) == 0)
    {                                                   //if the current selection is the same as the formation clicked on
        selection->numShips = 0;
    }
    if (selShipInSelection(selSelected.ShipPtr, selSelected.numShips, ship) && keyIsHit(SHIFTKEY))
    {
        selection->numShips = 0;
    }

    if (selection->numShips == 0)
    {                                                   //if everything was pruned
        selection->ShipPtr[0] = ship;                   //select at least this ship
        selection->numShips = 1;
    }
    tutGameMessage("Game_ClickSelectGroup");
    memFree(formSelection);

/*
    for(i=0;i<selSelected.numShips;)
    {
        if(selSelected.ShipPtr[i]->playerowner != universe.curPlayerPtr)
        {
            selSelected.numShips--;
            selSelected.ShipPtr[i]=selSelected.ShipPtr[selSelected.numShips];
            continue;
        }
        i++;
    }
*/
    //free the formation selection
}

/*-----------------------------------------------------------------------------
    Name        : mrObjectClick
    Description : Called when someone clicks on a single ship.  Handles modifiers etc.
    Inputs      : ship - object clicked on (can be a ship, asteroid, dust cloud or derelict)
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrObjectClick(Ship *ship)
{
    ProtectCommandMax6 protectOne;
    AttackCommandMax6 attackOne;
    udword mouseState[41];
    sdword result[41];
    udword keyState[41];
    MaxSelection tempSelection;
    sdword cursorType;
    Node *node;
    SpaceObjRotImpTarg *focusTarget;

    if (universePause || mrDisabled)
        return;

    protectOne.numShips = 0;
    if (ship)
    {
        protectOne.numShips = 1;
        protectOne.ShipPtr[0] = ship;
    }

    if (kbKeyIsHit(76))
    {
        if (!playPackets && protectOne.numShips > 0)
        {
            MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected);
            selSelectionCopyBounded((MaxSelection *)&selSelected, &tempSelection);
            if (selSelected.numShips > 0)
            {
                MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&protectOne, 0);
                MakeShipsNotIncludeTheseShips((SelectCommand *)&protectOne, (SelectCommand *)&selSelected);
                if (protectOne.numShips > 0)
                {
                    speechEventReportProtectTargetSelection();
                    clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&protectOne);
                    tutGameMessage("Game_BandboxForceAttack");
                }
            }
        }
        return;
    }

    if (kbKeyIsHit(78))
    {
        if (ship->objtype == OBJ_ShipType && ship->playerowner == universe.curPlayerPtr)
        {
            if (selShipInSelection(selSelected.ShipPtr, selSelected.numShips, ship))
                selSelectionRemoveSingleShip((MaxSelection *)&selSelected, ship);
            else
                selSelectionAddSingleShip((MaxSelection *)&selSelected, ship);
            ioUpdateShipTotals();
        }
        tutGameMessage("MultipleSelect");
        return;
    }

    if (kbKeyIsHit(74))
    {
        if (!playPackets && protectOne.numShips > 0)
        {
            makeShipsCommandControllable((SelectCommand *)&selSelected, 1);
            if (selSelected.numShips > 0)
            {
                MakeShipsFriendlyShips((SelectCommand *)&protectOne, universe.curPlayerPtr);
                MakeShipsNotIncludeTheseShips((SelectCommand *)&protectOne, (SelectCommand *)&selSelected);
                if (protectOne.numShips > 0)
                {
                    speechEventQueue(0, 67113057, -1, -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
                    clWrapProtect(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (ProtectCommand *)&protectOne);
                    tutGameMessage("Game_BandboxGuard");
                }
            }
        }
        return;
    }

    if (kbKeyIsHit(7))
    {
        if (!playPackets)
        {
            MakeShipsSpecialTargetCapable((SelectCommand *)&selSelected, FALSE);
            if (selSelected.numShips > 0)
            {
                selSelecting.numTargets = 1;
                selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;
                smOverlayOpen(TRUE, (SelectAnyCommand *)&selSelecting);
                tutGameMessage("Game_ClickSpecial");
            }
        }
        return;
    }

    if (kbKeyIsHit(77))
    {
        selSelecting.numTargets = 1;
        selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;

        if (smSensorsActive)
        {
            sdword i = 0;
            SpaceObjRotImpTarg **targetPtr = selSelecting.TargetPtr;
            do
            {
                if ((*targetPtr)->objtype || ccFocusIsLegal((SpaceObj *)*targetPtr))
                {
                    ++i;
                    ++targetPtr;
                }
                else
                {
                    *targetPtr = selSelecting.TargetPtr[--selSelecting.numTargets];
                }
            }
            while (i < selSelecting.numTargets);

            if (selSelecting.numTargets > 0)
                cat_smZoomInOnFocus();
            tutGameMessage("AltFocus");
        }
        else
        {
            ccFocusClose(&universe.mainCameraCommand, (FocusCommand *)&selSelecting);
            selSelecting.numTargets = 0;
            tutGameMessage("Game_BandboxFocus");
            tutGameMessage("AltFocus");
        }
        return;
    }

    if (kbKeyIsHit(75))
    {
        if (!playPackets)
        {
            if (protectOne.numShips > 0)
            {
                MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected);
                selSelectionCopyBounded((MaxSelection *)&selSelected, &tempSelection);
                if (selSelected.numShips > 0)
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&protectOne, 0);
                    MakeShipsNotIncludeTheseShips((SelectCommand *)&protectOne, (SelectCommand *)&selSelected);
                    MakeTargetsSalvageable((SelectAnyCommand *)&protectOne, universe.curPlayerPtr);
                    if (protectOne.numShips > 0)
                    {
                        speechEventReportProtectTargetSelection();
                        clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&protectOne);
                        tutGameMessage("Game_BandboxAttack");
                    }
                }
            }
            tutGameMessage("ClickAttack");
        }
        return;
    }

    if (smSensorsActive || playPackets || ShipInSelection((SelectCommand *)&selSelected, ship))
        goto regularFriendlyCase;

    memcpy(keyState, mrDetermineSelectionActions(result, (SelectAnyCommand *)&selSelected), sizeof(keyState));
    memset(mouseState, 0, sizeof(mouseState));
    attackOne.numTargets = 1;
    attackOne.TargetPtr[0] = (SpaceObjRotImpTarg *)ship;
    memcpy(mouseState, mrDetermineSelectionActions(result, (SelectAnyCommand *)&attackOne), sizeof(mouseState));
    cursorType = mrDetermineCursorType((udword *)keyState, (udword *)mouseState, &selSelected.numShips, ship);

    if (singlePlayerGame && singlePlayerGameInfo.currentMission == MISSION_10_SUPER_NOVA_STATION && ship->shiprace == 110 && ship != NULL)
    {
        focusTarget = NULL;
        node = universe.ShipList.head;
        if (node != NULL)
        {
            do
            {
                Ship *candidate = (Ship *)node->structptr;
                if (candidate->shiprace == 111)
                    focusTarget = (SpaceObjRotImpTarg *)candidate;
                node = node->next;
            }
            while (node != NULL);
        }
        if (focusTarget != NULL)
        {
            attackOne.TargetPtr[0] = focusTarget;
            if (!playPackets && !universePause && !mrDisabled)
                mrIssueSelectionSingleClickSpecial((SelectAnyCommand *)&attackOne, 0);
            goto label98;
        }
        goto regularFriendlyCase;
    }

    switch (cursorType)
    {
        case 1:
            if (homeworldCSA != 1)
                goto regularFriendlyCase;
            mrHarvestResources((char *)ship, NULL);
            soundEventPlay(0, 265, 0);
            return;

        case 2:
            if (homeworldCSA != 1)
                goto regularFriendlyCase;
            if (!playPackets && !universePause && !mrDisabled)
                mrIssueSelectionSingleClickSpecial((SelectAnyCommand *)&attackOne, 0);
            goto label98;

        case 3:
            if (attackOne.numTargets > 0)
            {
                MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected);
                selSelectionCopyBounded((MaxSelection *)&selSelected, &tempSelection);
                if (selSelected.numShips > 0)
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&attackOne, 0);
                    MakeShipsNotIncludeTheseShips((SelectCommand *)&attackOne, (SelectCommand *)&selSelected);
                    if (attackOne.numTargets > 0)
                        goto attackTarget;
                }
            }
            break;

        case 4:
            if (homeworldCSA != 1 || ship == NULL)
                goto regularFriendlyCase;
            if (!playPackets && !universePause && !mrDisabled)
                mrIssueSelectionSingleClickSpecial((SelectAnyCommand *)&attackOne, 1);
            goto label98;

        case 5:
            if (homeworldCSA != 1 || ship == NULL)
                goto regularFriendlyCase;
            mrProtectSelection((SelectCommand *)&attackOne);
            soundEventPlay(0, 265, 0);
            return;

        case 6:
        case 7:
        case 8:
            if (homeworldCSA != 1)
                goto regularFriendlyCase;
            mrDockSelectionToShip(ship);
            soundEventPlay(0, 265, 0);
            return;

        case 9:
            if (homeworldCSA != 1 || ship == NULL)
                goto regularFriendlyCase;
            selSelectionCopyBounded(&tempSelection, (MaxSelection *)&selSelected);
            MakeShipHostsIncludeAttachedShips((SelectCommand *)&tempSelection);
            clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&tempSelection, 0, ship);
            soundEventPlay(0, 265, 0);
            return;

        case 10:
            if (!playPackets && !universePause && !mrDisabled)
                mrIssueSelectionSingleClickSpecial((SelectAnyCommand *)&attackOne, 2);
            goto label98;

        case 11:
            if (homeworldCSA != 1 || ship == NULL)
                goto regularFriendlyCase;
            selSelectionCopyBounded(&tempSelection, (MaxSelection *)&selSelected);
            selSelectionAddSingleShipNoCheck(&tempSelection, ship);
            if (tempSelection.numShips > 0)
                clWrapLinkSelection(&universe.mainCommandLayer, (SelectCommand *)&tempSelection);
            goto label98;

        case 12:
            if (homeworldCSA != 1)
                goto regularFriendlyCase;
            mrIssueExternalConstructionSelection(ship);
            soundEventPlay(0, 265, 0);
            return;

        case 13:
        case 14:
            if (attackOne.numTargets > 0)
            {
                MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected);
                selSelectionCopyBounded((MaxSelection *)&selSelected, &tempSelection);
                if (selSelected.numShips > 0)
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&attackOne, 0);
                    MakeShipsNotIncludeTheseShips((SelectCommand *)&attackOne, (SelectCommand *)&selSelected);
                    if (attackOne.numTargets > 0)
                        goto attackTargetReport;
                }
            }
            break;

        case 15:
            if (!playPackets && !universePause && !mrDisabled)
                mrIssueSelectionSingleClickSpecial((SelectAnyCommand *)&attackOne, 6);
            goto label98;

        case 16:
        case 17:
            if (attackOne.numTargets > 0)
            {
                MakeShipsAttackCapable((SelectCommand *)&tempSelection, (SelectCommand *)&selSelected);
                selSelectionCopyBounded((MaxSelection *)&selSelected, &tempSelection);
                if (selSelected.numShips > 0)
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&attackOne, 0);
                    MakeShipsNotIncludeTheseShips((SelectCommand *)&attackOne, (SelectCommand *)&selSelected);
                    if (attackOne.numTargets > 0)
                    {
attackTarget:
                        speechEventReportProtectTargetSelection();
                        clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&attackOne);
                        tutGameMessage("Game_BandboxForceAttack");
                        break;
                    }
                }
            }
            break;

        default:
            goto regularFriendlyCase;
    }

afterSwitch:
    soundEventPlay(0, 265, 0);
    tutGameMessage("ClickAttack");
    return;

label98:
    soundEventPlay(0, 265, 0);
    return;

attackTargetReport:
    speechEventReportProtectTargetSelection();
    clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&attackOne);
    tutGameMessage("Game_BandboxForceAttack");
    soundEventPlay(0, 265, 0);
    tutGameMessage("ClickAttack");
    return;

regularFriendlyCase:
    if (ship->objtype == OBJ_ShipType && ship->playerowner == universe.curPlayerPtr)
    {
        Ship *mothership;

        mrFormationShips(&tempSelection, ship);
        selSelectionCopy((MaxAnySelection *)&selSelected, (MaxAnySelection *)&tempSelection);
        ioUpdateShipTotals();

        if (!playPackets)
        {
            mothership = universe.curPlayerPtr->PlayerMothership;
            if (mothership != NULL
                && selShipInSelection(tempSelection.ShipPtr, tempSelection.numShips, mothership)
                && !((ShipStaticInfo *)mothership->staticinfo)->shipclass)
            {
                speechEventQueue(NULL, COMM_Selection, mothership->shiptype,
                                 -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
            }
            else
            {
                speechEventQueue(ship, COMM_Selection, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrRegionProcess
    Description : Region logic handler for main region.
    Inputs      : same as any region processor
    Outputs     : ..
    Return      : same as any region processor
----------------------------------------------------------------------------*/
udword mrRegionProcess(regionhandle reg, sdword ID, udword event, udword data)
{
    rectangle defaultRect = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};

    while (feMenuLevel > 0)
    {
        feMenuDisappear(NULL, NULL);
    }
    if (byte_A2DCD4 || smZoomingIn || smZoomingOut)
    {
        return(0);
    }
    switch (event)
    {
        case RPE_HoldLeft:
            if (mrTabStatsIsDragging())
            {
                mrTabStatsUpdateDrag();
            }
            else
            {
                mrHandleBandboxHoldLeft(0);
            }
            break;
        case RPE_HoldRight:
            if (mrRightMouseCameraMode == 2)
            {
                mrCameraMotion();
            }
            break;
       case RPE_PressLeft:
            (void)mrHandleBandboxPressLeft();
            break;
        case RPE_PressRight:
            mrHandleBandboxPressRight(&reg->rect);
            break;
        case RPE_DoubleLeft:
if (!mrTabStatsIsDragging() && !smSensorsActiveGet() && !mouseCursorTypeGet())
{
    mrSelectLeftRelease(0);
}
break;
        case RPE_ReleaseLeft:
            mrHandleBandboxReleaseLeft(0);
#if MR_TEST_HPB
            if (mrHoldLeft == mrHeadingPitchHold)
            {
                mrHoldLeft = mrNULL;                        //idle mode
            }
            if (mrHoldRight == mrBankHold)
            {
                mrHoldRight = mrNULL;
            }
#endif
            break;
    case RPE_ReleaseRight:
            mrRightClickMenu(0);
#if MR_TEST_HPB
            if (mrHoldRight == mrBankHold)
            {
                mrHoldRight = mrNULL;
            }
#endif
            break;
        case RPE_KeyDown:
            if (gameStatsOverlayActive())
            {
                if (ID == ESCKEY || ID == XKEY)
                {
                    gameStatsQuitToMain();
                }
                if (ID == SPACEKEY || ID == ESCKEY)
                {
                    kbInvokeCommandCallbacks(0, 107);
                }
                break;
            }
            if (nisIsRunning_ && thisNisPlaying && (ID == ESCKEY || ID == SPACEKEY || ID == ENTERKEY))
            {
                if (!singlePlayerGame)
                {
                    kbInvokeCommandCallbacks(0, 90);
                }
                else
                {
#if BINK
                    if (binkDonePlaying)
#endif
                    {
                        kbInvokeCommandCallbacks(0, 90);
                    }
                }
                break;
            }
            if (mrDisabled)
            {
                if (mouseCursorTypeGet() && (ID == ESCKEY || ID == XKEY))
                {
                    gameStatsQuitToMain();
                }
                if (ID == SPACEKEY || ID == ESCKEY)
                {
                    kbInvokeCommandCallbacks(0, 107);
                }
                break;
            }
            if (!(smZoomingIn | smZoomingOut))
            {
                if (kbHandleGameScreenKeyDown(FALSE, ID))
                {
                    break;
                }
                mrKeyPress(ID);
            }
            break;
        case RPE_KeyUp:
            if (!(smZoomingIn | smZoomingOut))
            {
                kbCallCommandsBoundToKey(FALSE, ID);
                switch (ID)
                {
                    case SHIFTKEY:
                        if (mrRightMouseCameraMode != 2)
                        {
                            piePointModeToggle(FALSE);
                            ioSetSelection(TRUE);
                        }
                        break;
                    case NUMPLUSKEY:
                    case PLUSKEY:
                        zoomInNow = FALSE;
                        break;
                    case MINUSKEY:
                    case NUMMINUSKEY:
                        zoomOutNow = FALSE;
                        break;
                }
            }
            break;
        case RPE_WheelUp:
            wheel_up = TRUE;
            break;
        case RPE_WheelDown:
            wheel_down = TRUE;
            break;
        default:
#if MR_VERBOSE_LEVEL >= 2
            dbgMessagef("\nmrRegionProcess: unprocessed message. reg = 0x%x, ID = 0x%x, event = 0x%x, data = %d", reg, ID, event, data);
#endif
            break;
    }
    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : mrPlayerNameDraw
    Description : Draw the name of a particular player
    Inputs      : playerIndex - index of player who's name we are to print
                  x, y - location to print
    Outputs     : Draws names of specified multi-player player
    Return      : void
----------------------------------------------------------------------------*/
/*
void mrPlayerNameDraw(sdword playerIndex, sdword x, sdword y)
{
    color c;

#if TO_STANDARD_COLORS
    if (&universe.players[playerIndex] == universe.curPlayerPtr)
    {
        c = teFriendlyColor;
    }
    else if (allianceArePlayersAllied(&universe.players[playerIndex], universe.curPlayerPtr))
    {
        c = teAlliedColor;
    }
    else
    {
        c = teHostileColor;
    }

#else
    c = teColorSchemes[playerIndex].tacticalColor;
#endif
    fontPrint(x, y, c, playerNames[playerIndex]);
}
*/

/*-----------------------------------------------------------------------------
    Name        : mrCommandMessageDraw
    Description : Draw the current list of player commands
    Inputs      : None (uses global variable gMessage)
    Outputs     : Draws list of latest player commands
    Return      : void
----------------------------------------------------------------------------*/
void mrCommandMessageDraw(void)
{
    ubyte  i = MAX_MESSAGES;
    udword j = MR_MESSAGE_LINE_SPACING;

    LockMutex(gMessageMutex);

    if (gMessage[0].message[0] == (char)NULL)
        goto done;

    // find the last valid message in global array
    while (gMessage[i].message[0] == (char)NULL)
    {
        i--;
    }

    // print it
    fontPrint(MR_MESSAGE_LINE_START, 0, colWhite, gMessage[i].message);

    // print remaining messages in global array
    while (i > 0)
    {
        i--;
        fontPrint(MR_MESSAGE_LINE_START, j, colWhite, gMessage[i].message);
        j += MR_MESSAGE_LINE_SPACING;
    }

    // remove expired messages
    while ((gMessage[0].MessageExpire < universe.totaltimeelapsed) &&
           (gMessage[0].message[0] != (char)NULL))
    {
        // shift up remaining messages to fill in void left by expired message
        while (gMessage[i+1].message[0] != (char)NULL)
        {
            strcpy(gMessage[i].message, gMessage[i+1].message);
            gMessage[i].MessageExpire = gMessage[i+1].MessageExpire;
            i++;
        }
        gMessage[i].message[0]  = (char)NULL;
        i = 0;
    }

done:
    UnLockMutex(gMessageMutex);
}

void bigmessageDisplay(char *msg,sdword position)
{
    bMessage[position].messageOn = TRUE;
    strcpy(bMessage[position].message,msg);
}

void bigmessageErase(sdword position)
{
    bMessage[position].messageOn = FALSE;
}

static sdword bigmessageYPosition[MAX_BIGMESSAGES] = { 200,260 };

void mrBigMessageDraw(void)
{
    sdword i;
    fonthandle fhSave;

    dbgAssert(mrBigFont != 0);

    for (i=0;i<MAX_BIGMESSAGES;i++)
    {
        if (bMessage[i].messageOn)
        {
            fhSave = fontCurrentGet();
            fontMakeCurrent(mrBigFont);

            fontPrintCentre(bigmessageYPosition[i],colWhite,bMessage[i].message);

            fontMakeCurrent(fhSave);
        }
    }
}

//
// Move Line Functions
//
// NOTE: This code is supposed to reside in tactical.c, but for some reason nothing
//       gets rendered when it is moved there.  Therefore, these functions should stay
//       here.  Move them at your own risk!
#define TO_MOVE_LINE_COLOR              TW_MOVETO_LINE_COLOR
#define TO_MOVE_LINE_PULSE_COLOR        TW_MOVETO_PULSE_COLOR
#define TO_ATT_MOVE_LINE_COLOR          TW_ATTMOVETO_LINE_COLOR
#define TO_ATT_MOVE_LINE_PULSE_COLOR    TW_ATTMOVETO_PULSE_COLOR
#define TO_PULSE_CLOSE_SHRINK           0.8f
#define TO_FADE_SIZE                    2  //*pulsesize
#define TO_MOVE_LINE_RADIUS_STRETCH     TW_MOVETO_CIRCLE_RADIUS
#define TO_PULSE_SPEED_SCALE            TW_MOVETO_PULSE_SPEED_SCALE
#define TO_MOVETO_ENDCIRCLE_RADIUS      TW_MOVETO_ENDCIRCLE_RADIUS

//a kludgy global variable
bool32 pulse_at_beginning = FALSE;

/*-----------------------------------------------------------------------------
    Name        : toDrawPulsedLine
    Description : Draws a line with a cool ass pulse flying down it at incredible velocities
    Inputs      : linestart  - where the line starts
                  lineend    - where the line ends
                  pulsesize  - how big that cool ass pulse is
                  linecolor  - the color of the line
                  pulsecolor - the color of the cool ass pulse
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
bool32 toDrawPulsedLine(vector linestart, vector lineend, real32 pulsesize, color linecolor, color pulsecolor)
{
    static real32 lasttime = 0.0, pulsestartfraction = 0.0;
    real32 distance, pulsedistance, fadesize;
    vector dirvect, pulsestart, pulseend, fadestart, fadeend;
    bool32 pulse_at_end = FALSE;
    udword test = 0;
    bool32 draw_pulse_beg = TRUE, draw_fadein = FALSE, draw_fadeout = FALSE;

    pulse_at_beginning = FALSE;

    if (taskTimeElapsed != lasttime)
    {
        pulsestartfraction += (taskTimeElapsed - lasttime)/TO_PULSE_SPEED_SCALE;
        lasttime            = taskTimeElapsed;

        // - first check is for the very first iteration of toDrawPulsedLine where
        //   "taskTimeElapsed" will be much larger that "lasttime" and
        //   therefore "pulsestartfraction" is quite large
        // - the second check is to fix a very obscure bug where "lasttime" somehow
        //   gets set to a value larger than "taskTimeElapsed".  We can't
        //   figure out why "lasttime" does this, though...
        if ((pulsestartfraction > 1.0) || (pulsestartfraction < 0.0))
        {
            pulsestartfraction = 0.0;
            pulse_at_beginning = TRUE;
        }
    }

    //calculate the pulse
    //get the unit direction vector of the pulse line
    vecSub(dirvect, lineend, linestart);
    distance = (real32) fsqrt(vecMagnitudeSquared(dirvect));
    vecNormalize(&dirvect);

    //find the start and end points of the pulse
    pulsedistance = pulsestartfraction * distance;

    while (pulsesize > distance/2)
    {
        pulsesize *= TO_PULSE_CLOSE_SHRINK;
    }

    //if the pulse is at the end of the moveto line
    if (pulsedistance > (distance + pulsesize))
    {
        pulsestartfraction = 0.0;
        pulse_at_beginning = TRUE;
    }

    //calculate the fade regions
    fadesize = TO_FADE_SIZE*pulsesize;

    //if the pulse is far enough away from the beginning of the moveto line to have a fadein region
    if (pulsedistance > fadesize)
    {
        vecScalarMultiply(fadestart, dirvect, pulsedistance - fadesize);
        vecAddTo(fadestart, linestart);
        draw_fadein = TRUE;
    }
    else if (pulsedistance < pulsesize + fadesize)
    {
        pulse_at_beginning = TRUE;

        if (pulsedistance < pulsesize)
        {
            draw_pulse_beg = FALSE;
        }
    }

    //if the pulse is far enough away from the end of the moveto line to have a fadeout region
    if (pulsedistance < (distance - fadesize - pulsesize))
    {
        vecScalarMultiply(fadeend, dirvect, (pulsedistance + pulsesize + fadesize));
        vecAddTo(fadeend, linestart);
        draw_fadeout = TRUE;
    }

    if ((pulsedistance > (distance - fadesize/3 - pulsesize)) ||
        (distance < TW_MOVETO_ENDCIRCLE_RADIUS))
    {
        pulse_at_end = TRUE;
    }

    //find the point locations of the start and end of the pulse
    vecScalarMultiply(pulsestart, dirvect, (pulsedistance /*- (pulsestartfraction * pulsesize)*/));
    vecAddTo(pulsestart, linestart);
    vecScalarMultiply(pulseend, dirvect, (pulsedistance + (pulsestartfraction * pulsesize)));
    vecAddTo(pulseend, linestart);


    glBegin(GL_LINES);
    {
        if (draw_pulse_beg)
        {
            //draw the line from the ship to the pulse
            glColor3ub(colRed(linecolor), colGreen(linecolor), colBlue(linecolor));
            glVertex3fv((GLfloat *)&linestart);

            //draw the fadein
            if (draw_fadein)
            {
                glVertex3fv((GLfloat *)&fadestart);
                glVertex3fv((GLfloat *)&fadestart);
            }
            glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
            glVertex3fv((GLfloat *)&pulsestart);

        }
        else
        {
            glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
            glVertex3fv((GLfloat *)&linestart);
            glVertex3fv((GLfloat *)&pulsestart);
        }

        //draw the pulse
        glVertex3fv((GLfloat *)&pulsestart);

        if (!pulse_at_end)
        {
            glVertex3fv((GLfloat *)&pulseend);

            //draw the line from the pulse to the end of the line
            glVertex3fv((GLfloat *)&pulseend);
            glColor3ub(colRed(linecolor), colGreen(linecolor), colBlue(linecolor));

            //draw the fadeout
            if (draw_fadeout)
            {
                glVertex3fv((GLfloat *)&fadeend);
                glVertex3fv((GLfloat *)&fadeend);
            }
        }
        glVertex3fv((GLfloat *)&lineend);
    }
    glEnd();

    return pulse_at_end;
}


/*-----------------------------------------------------------------------------
    Name        : toDrawMoveToCircle
    Description : Draws a circle at the destination of the ship
    Inputs      : ship - the ship
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveToEndCircle(ShipPtr ship, bool32 pulse, color linecolor, color pulsecolor)
{
    if (pulse)
    {
        primCircleOutline3(&ship->moveTo, TO_MOVETO_ENDCIRCLE_RADIUS,
                           32, 0, pulsecolor, Z_AXIS);
    }
    else
    {
        primCircleOutline3(&ship->moveTo, TO_MOVETO_ENDCIRCLE_RADIUS,
                            32, 0, linecolor, Z_AXIS);
    }
}



/*-----------------------------------------------------------------------------
    Name        : toDrawMoveToLine
    Description : Draws a single move line from a ship to it's move point
    Inputs      : ship - the aformentioned ship
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveToLine(ShipPtr ship, color linecolor, color pulsecolor)
{
    vector shipfront, shipright, dirfrontproj, dirrightproj, dirvect, dirproj,
           zero = {0,0,0}, negdir;
    real32 distance, pulsesize, shipradius;
    bool32 pulse = FALSE;

    //initialize a bunch of vectors and distances
    shipradius  = ship->magnitudeSquared*ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;
    pulsesize   = 2*shipradius;

    matGetVectFromMatrixCol3(shipfront, ship->rotinfo.coordsys);
    matGetVectFromMatrixCol2(shipright, ship->rotinfo.coordsys);

    vecSub(dirvect, ship->moveTo, ship->collInfo.collPosition);
    distance = (real32) fsqrt(vecMagnitudeSquared(dirvect));

    //find the moveto starting point
    vecScalarMultiply(dirfrontproj, shipfront, vecDotProduct(dirvect, shipfront));
    vecScalarMultiply(dirrightproj, shipright, vecDotProduct(dirvect, shipright));
    vecAdd(dirproj, dirfrontproj, dirrightproj);
    vecNormalize(&dirproj);

    //calculate the edge of the MoveToEndCircle
    //find the negative of the direction
    vecSub(negdir, zero, dirvect);
    negdir.z = 0;
    vecNormalize(&negdir);
    vecMultiplyByScalar(negdir, TO_MOVETO_ENDCIRCLE_RADIUS);
    vecAddTo(negdir, ship->moveTo);

    vecMultiplyByScalar(dirproj, (shipradius)*TO_MOVE_LINE_RADIUS_STRETCH);
    vecAddTo(dirproj, ship->collInfo.collPosition);


    //if the ship is pretty close to the end of the move, don't draw the line
    if (distance > pulsesize)
    {
        pulse = toDrawPulsedLine(dirproj, negdir, pulsesize, linecolor, pulsecolor);
    }

    if (distance > ((shipradius*TO_MOVE_LINE_RADIUS_STRETCH) + TO_MOVETO_ENDCIRCLE_RADIUS))
    {
        toDrawMoveToEndCircle(ship, pulse, linecolor, pulsecolor);
    }


}


/*-----------------------------------------------------------------------------
    Name        : toDrawMoveFromLine
    Description : Draws a single move line from a ship's movefrom point to the ship
    Inputs      : ship - the aformentioned ship
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveFromLine(ShipPtr ship)
{
    vector shipback, shippos, dirvect;
    real32 distance,shipradius;

    shipradius = TO_MOVE_LINE_RADIUS_STRETCH*ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    //calculate the line
    matGetVectFromMatrixCol3(shipback, ship->rotinfo.coordsys);
    vecMultiplyByScalar(shipback, (shipradius));
    shippos = ship->collInfo.collPosition;
    vecSubFrom(shippos, shipback);
    shipback = shippos;

    vecSub(dirvect, shipback, ship->moveFrom);
    distance = (real32) fsqrt(vecMagnitudeSquared(dirvect));

    if (distance < shipradius)
    {
        return;
    }

    glBegin(GL_LINES);
    {
        //draw the line from the ship to the pulse
        glColor3ub(colRed(TO_MOVE_LINE_COLOR), colGreen(TO_MOVE_LINE_COLOR), colBlue(TO_MOVE_LINE_COLOR));
        glVertex3fv((GLfloat *)&shipback);
        glVertex3fv((GLfloat *)&ship->moveFrom);
    }
    glEnd();


}



/*-----------------------------------------------------------------------------
    Name        : toDrawMoveCircle
    Description : Draws a circle around the moving ship
    Inputs      : ship - the ship
                  scale - the tactical overlay color scale factor to fade in the move
                          circle (not implemented)
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toDrawMoveCircle(ShipPtr ship, real32 scale, color linecolor, color pulsecolor)
{
    real32  shipradius;
    hmatrix rotmat;
    vector origin = {0.0, 0.0, 0.0};
    CommandToDo *command;

    hmatMakeHMatFromMat(&rotmat, &ship->rotinfo.coordsys);
    hmatPutVectIntoHMatrixCol4(ship->collInfo.collPosition, rotmat);

    glPushMatrix();
    glMultMatrixf((GLfloat *)&rotmat);

    shipradius = ship->magnitudeSquared*TO_MOVE_LINE_RADIUS_STRETCH*ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize;

    command = ship->command;
    if ((command != NULL) &&
        ((command->ordertype.attributes & COMMAND_IS_FORMATION) != 0) &&
        (command->formation.pulse != 0))
    {
        primCircleOutline3(&origin, shipradius, 32, 0, pulsecolor, X_AXIS);
    }
    else
    {
        primCircleOutline3(&origin, shipradius, 32, 0, linecolor, X_AXIS);
    }

    glPopMatrix();

}




/*-----------------------------------------------------------------------------
    Name        : toMoveLineDraw
    Description : Draws a line with a cool pulse from the ship's current position to
                  it's moveTo position.  Also draws a circle around the moving ship
    Inputs      : ship - the ship
                  scale - the tactical overlay color scale factor to fade in the move
                          circle (not used at the moment)
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toMoveLineDraw(ShipPtr ship, real32 scale)
{
    bool32 primEnabled;
    color linecolor;
    color pulsecolor;
    CommandToDo *command;

    primEnabled = primModeEnabled;
    if (primEnabled)
    {
        primModeClear2();
    }
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    command = ship->command;
    if ((command != NULL) && (command->ordertype.order == COMMAND_ATTACK))
    {
        linecolor = TO_ATT_MOVE_LINE_COLOR;
        pulsecolor = TO_ATT_MOVE_LINE_PULSE_COLOR;
    }
    else
    {
        linecolor = TO_MOVE_LINE_COLOR;
        pulsecolor = TO_MOVE_LINE_PULSE_COLOR;
    }

    if ((command == NULL) ||
        ((command->ordertype.attributes & COMMAND_IS_FORMATION) == 0) ||
        (ship == command->selection->ShipPtr[0]))
    {
        toDrawMoveToLine(ship, linecolor, pulsecolor);
        if ((command != NULL) && (command->ordertype.attributes & COMMAND_IS_FORMATION))
        {
            command->formation.pulse = pulse_at_beginning;
        }
    }

    toDrawMoveCircle(ship, scale, linecolor, pulsecolor);

    if (primEnabled)
    {
        primModeSet2();
    }
}

/*-----------------------------------------------------------------------------
    Name        : toMoveLineDraw
    Description : Draws a line with a cool pulse from the ship's current position to
                  it's moveTo position.  Also draws a circle around the moving ship
    Inputs      : ship - the ship
                  scale - the tactical overlay color scale factor to fade in the move
                          circle (not used at the moment)
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toFieldSphereDraw(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{
    hmatrix rotmat;
    vector origin = {0.0, 0.0, 0.0};
    vector up =    {1.0, 0.0, 0.0};
    vector right = {0.0, 1.0, 0.0};
    vector head =  {0.0, 0.0, 1.0};
    matrix tmpmat;

    //Turn off 2D primmode
    primModeClear2();
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    //Draw the circles
    //    hmatMakeHMatFromMat(&rotmat, &ship->rotinfo.coordsys);
        //hmatPutVectIntoHMatrixCol4(ship->collInfo.collPosition, rotmat);
    //     hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


    matPutVectIntoMatrixCol1(up,tmpmat);
    matPutVectIntoMatrixCol2(right,tmpmat);
    matPutVectIntoMatrixCol3(head,tmpmat);

    hmatMakeHMatFromMat(&rotmat, &tmpmat);
    hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


    glPushMatrix();
        glMultMatrixf((GLfloat *)&rotmat);

        primCircleOutline3(&origin, radius, 32, 4, passedColour, Z_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, X_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, Y_AXIS);

        glPopMatrix();
    //stop drawing circles


    //Turn on 2D primmode
    primModeSet2();
}

void toFieldSphereDrawGeneral(vector position, real32 radius,color passedColour)
{
    hmatrix rotmat;
    vector origin = {0.0, 0.0, 0.0};
    vector up =    {1.0, 0.0, 0.0};
    vector right = {0.0, 1.0, 0.0};
    vector head =  {0.0, 0.0, 1.0};
    matrix tmpmat;

    //Turn off 2D primmode
    primModeClear2();
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    //Draw the circles
    //    hmatMakeHMatFromMat(&rotmat, &ship->rotinfo.coordsys);
        //hmatPutVectIntoHMatrixCol4(ship->collInfo.collPosition, rotmat);
    //     hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


    matPutVectIntoMatrixCol1(up,tmpmat);
    matPutVectIntoMatrixCol2(right,tmpmat);
    matPutVectIntoMatrixCol3(head,tmpmat);

    hmatMakeHMatFromMat(&rotmat, &tmpmat);
    hmatPutVectIntoHMatrixCol4(position, rotmat);


    glPushMatrix();
        glMultMatrixf((GLfloat *)&rotmat);

        primCircleOutline3(&origin, radius, 32, 4, passedColour, Z_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, X_AXIS);
        primCircleOutline3(&origin, radius, 32, 4, passedColour, Y_AXIS);

        glPopMatrix();
    //stop drawing circles


    //Turn on 2D primmode
    primModeSet2();
}

real32 pulse1_timeelapsed=0.0f;
real32 pulse1_lasttime=0.0f;
real32 pulse1_timeelapsed_fast=0.0f;
real32 pulse1_lasttime_fast=0.0f;
real32 pulsedistance = 0.0f;
real32 pulsefastdistance = 0.0f;
real32 delay = 0.0f;

#define CROSS_HALF_HIGHT   TO_CROSS_HALF_SIZE


void toPulse1_positionupdate();
void toPulse1(vector linestart, vector lineend, real32 pulsesize, color pulsecolor);


//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator1(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};

   matrix tmpmat;
   vector lineend,tmpvec1,tmpvec2,tmpvecup1,tmpvecup2,tmpvecright1,tmpvecright2,tmpvechead1,tmpvechead2;

   //Turn off 2D primmode
   primModeClear2();
   rndLightingEnable(FALSE);
   rndTextureEnable(FALSE);

   matPutVectIntoMatrixCol1(up,tmpmat);
   matPutVectIntoMatrixCol2(right,tmpmat);
   matPutVectIntoMatrixCol3(head,tmpmat);

   hmatMakeHMatFromMat(&rotmat, &tmpmat);
   //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


   glPushMatrix();
   glMultMatrixf((GLfloat *)&rotmat);

   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Z_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

   vecScalarMultiply(tmpvecup1,up,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecup2,up,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright1,right,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright2,right,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead1,head,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead2,head,-CROSS_HALF_HIGHT);

   toPulse1_positionupdate();                //update time information
    if(delay > 0.0f)
    {
        delay -= pulse1_timeelapsed;
        if(delay <= 0.0f)
            delay = 0.0f;

    }
////
   vecScalarMultiply(lineend,right,radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecScalarMultiply(lineend,right,-radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

////
   vecScalarMultiply(lineend,up, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecScalarMultiply(lineend,up, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

////
   vecScalarMultiply(lineend,head, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecScalarMultiply(lineend,head, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse1(ship->posinfo.position, lineend, TO_PULSE1_SIZE       , TO_PULSE1_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR1);

   glPopMatrix();
   //stop drawing circles


   //Turn on 2D primmode
   primModeSet2();

}

#define TO_PULSE1_SPEED    TO_PULSE1_SPEED
#define TO_DELAY_PULSE1    TO_DELAY_PULSE1

void toPulse1_positionupdate()
{
   pulse1_timeelapsed = universe.totaltimeelapsed - pulse1_lasttime;
   pulse1_lasttime = universe.totaltimeelapsed;
   if(delay == 0.0f)
   {
       pulsedistance += pulse1_timeelapsed*TO_PULSE1_SPEED;
   }

}

/*-----------------------------------------------------------------------------
    Name        : toPulse1
    Description : Draws a pulse from linestart to lineend
    Inputs      : linestart  - where the line starts
                  lineend    - where the line ends
                  pulsesize  - how big that cool ass pulse is
                  linecolor  - the color of the line
                  pulsecolor - the color of the cool ass pulse
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void toPulse1(vector linestart, vector lineend, real32 pulsesize, color pulsecolor)
{
    real32 distance, pulseenddistance;
    vector dirvect, pulsestart, pulseend;

    if(delay > 0.0f)
    {
       return;
    }

    vecSub(dirvect, lineend, linestart);
    distance = fsqrt(vecMagnitudeSquared(dirvect));
    vecNormalize(&dirvect);


    pulseenddistance = pulsedistance+pulsesize;
   if((pulseenddistance) > distance)
   {
     //pulse has overshot end so
     pulsedistance = 0.0f;
     pulseenddistance = pulsesize;
     delay = TO_DELAY_PULSE1;
     //pulse1_timeelapsed = 0.0f;
     //pulse1_lasttime = 0.0f;
   }
   vecScalarMultiply(pulsestart, dirvect, pulsedistance);

    vecScalarMultiply(pulseend, dirvect, pulseenddistance);
    vecAddTo(pulsestart,linestart);
    vecAddTo(pulseend,linestart);

    glBegin(GL_LINES);
    {
        glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
        glVertex3fv((GLfloat *)&pulsestart);
        glVertex3fv((GLfloat *)&pulseend);

    }
    glEnd();

}

/*-----------------------------------------------------------------------------
Radial Indicator # 2...same as above but different speeds
-----------------------------------------------------------------------------*/

real32 pulse2_timeelapsed=0.0f;
real32 pulse2_lasttime=0.0f;
real32 pulse2_timeelapsed_fast=0.0f;
real32 pulse2_lasttime_fast=0.0f;
real32 pulsedistance2 = 0.0f;
real32 pulsefastdistance2 = 0.0f;
real32 delay2 = 0.0f;


void toPulse2_positionupdate();
void toPulse2(vector linestart, vector lineend, real32 pulsesize, color pulsecolor);


//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator2(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};

   matrix tmpmat;
   vector lineend,tmpvec1,tmpvec2,tmpvecup1,tmpvecup2,tmpvecright1,tmpvecright2,tmpvechead1,tmpvechead2;

   //Turn off 2D primmode
   primModeClear2();
   rndLightingEnable(FALSE);
   rndTextureEnable(FALSE);

   matPutVectIntoMatrixCol1(up,tmpmat);
   matPutVectIntoMatrixCol2(right,tmpmat);
   matPutVectIntoMatrixCol3(head,tmpmat);

   hmatMakeHMatFromMat(&rotmat, &tmpmat);
   //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


   glPushMatrix();
   glMultMatrixf((GLfloat *)&rotmat);

   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Z_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
   //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

   vecScalarMultiply(tmpvecup1,up,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecup2,up,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright1,right,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvecright2,right,-CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead1,head,CROSS_HALF_HIGHT);
   vecScalarMultiply(tmpvechead2,head,-CROSS_HALF_HIGHT);

   toPulse2_positionupdate();                //update time information
    if(delay2 > 0.0f)
    {
        delay2 -= pulse2_timeelapsed;
        if(delay2 <= 0.0f)
            delay2 = 0.0f;

    }
////
   vecScalarMultiply(lineend,right,radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecScalarMultiply(lineend,right,-radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

////
   vecScalarMultiply(lineend,up, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecScalarMultiply(lineend,up, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvechead1);
   vecAdd(tmpvec2,lineend,tmpvechead2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

////
   vecScalarMultiply(lineend,head, radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecScalarMultiply(lineend,head, -radius);
   vecAddTo(lineend,ship->posinfo.position);
   toPulse2(ship->posinfo.position, lineend, TO_PULSE2_SIZE       , TO_PULSE2_COLOR);

   vecAdd(tmpvec1,lineend,tmpvecup1);
   vecAdd(tmpvec2,lineend,tmpvecup2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   vecAdd(tmpvec1,lineend,tmpvecright1);
   vecAdd(tmpvec2,lineend,tmpvecright2);
   primLine3(&tmpvec1,&tmpvec2,TO_CROSS_COLOR2);

   glPopMatrix();
   //stop drawing circles


   //Turn on 2D primmode
   primModeSet2();

}

#define TO_PULSE2_SPEED    TO_PULSE2_SPEED
#define TO_DELAY_PULSE2    TO_DELAY_PULSE2

void toPulse2_positionupdate()
{
   pulse2_timeelapsed = universe.totaltimeelapsed - pulse2_lasttime;
   pulse2_lasttime = universe.totaltimeelapsed;
   if(delay2 == 0.0f)
   {
      pulsedistance2 += pulse2_timeelapsed*TO_PULSE2_SPEED;
   }
}

/*-----------------------------------------------------------------------------
    Name        : toPulse2
      -same as above but faster...
      ..need because of different time variables
----------------------------------------------------------------------------*/
void toPulse2(vector linestart, vector lineend, real32 pulsesize, color pulsecolor)
{
    real32 distance, pulseenddistance;
    vector dirvect, pulsestart, pulseend;

    if(delay2 > 0.0f)
    {
       return;
    }

    vecSub(dirvect, lineend, linestart);
    distance = fsqrt(vecMagnitudeSquared(dirvect));
    vecNormalize(&dirvect);


    pulseenddistance = pulsedistance2+pulsesize;
   if((pulseenddistance) > distance)
   {
     //pulse has overshot end so
     pulsedistance2 = 0.0f;
     pulseenddistance = pulsesize;
     delay2 = TO_DELAY_PULSE2;
     //pulse1_timeelapsed = 0.0f;
     //pulse1_lasttime = 0.0f;
   }
   vecScalarMultiply(pulsestart, dirvect, pulsedistance2);

    vecScalarMultiply(pulseend, dirvect, pulseenddistance);
    vecAddTo(pulsestart,linestart);
    vecAddTo(pulseend,linestart);

    glBegin(GL_LINES);
    {
        glColor3ub(colRed(pulsecolor), colGreen(pulsecolor), colBlue(pulsecolor));
        glVertex3fv((GLfloat *)&pulsestart);
        glVertex3fv((GLfloat *)&pulseend);

    }
    glEnd();

}

//Radial Indicator 3  Circles pulsing to Outline a Sphere

#define TO_CIRCLE3_SPEED    4500.0f
#define TO_CIRCLE3_DELAY    3.0f
#define TO_CIRCLE3_DELAY_SPACEING 0.05f
#define TO_NUM_CIRCLES_3      1

real32 circle3_timeelapsed[TO_NUM_CIRCLES_3];
real32 circle3_lasttime[TO_NUM_CIRCLES_3];
real32 circle3_timeelapsed_fast[TO_NUM_CIRCLES_3];
real32 circle3_lasttime_fast[TO_NUM_CIRCLES_3];
real32 circle3distance[TO_NUM_CIRCLES_3];
real32 circle3fastdistance[TO_NUM_CIRCLES_3];
real32 circle3delay[TO_NUM_CIRCLES_3];

//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator3(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   real32 newrad;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};
   sdword count;
   matrix tmpmat;
   vector circlestart,circlestart2;


   count = 0;

   while(count < TO_NUM_CIRCLES_3)
   {
        vecScalarMultiply(circlestart,head,radius);

        circle3_timeelapsed[count] = universe.totaltimeelapsed - circle3_lasttime[count];
        circle3_lasttime[count] = universe.totaltimeelapsed;
        if(circle3delay[count] <= 0.0f)
        {
            circle3delay[count] = 0.0f;
            circle3distance[count] += circle3_timeelapsed[count]*TO_CIRCLE3_SPEED;
        }
        else
        {
            circle3delay[count] -= circle3_timeelapsed[count];
            count++;
            continue;
        }
        if(circle3distance[count] > 2*radius)      //make faster :)    precalc diameter
        {
            circle3distance[count] = 0.0f;
            if(count == 0)
            {
                circle3delay[count] = TO_CIRCLE3_DELAY;
            }
            else
            {
                circle3delay[count] = TO_CIRCLE3_DELAY_SPACEING + circle3delay[count-1];
            }
            count++;
            continue;
        }

        vecScalarMultiply(circlestart2,head,-circle3distance[count]);
        vecAddTo(circlestart,circlestart2);
        vecAddTo(circlestart,ship->posinfo.position);
        if(circle3distance[count] > radius)
        {
            newrad = 2*radius - circle3distance[count];
        }
        else
        {
            newrad = circle3distance[count];
        }
       newrad = fsqrt(radius*radius - (radius - newrad)*(radius - newrad));

       //Turn off 2D primmode
       primModeClear2();
       rndLightingEnable(FALSE);
       rndTextureEnable(FALSE);

       matPutVectIntoMatrixCol1(up,tmpmat);
       matPutVectIntoMatrixCol2(right,tmpmat);
       matPutVectIntoMatrixCol3(head,tmpmat);

       hmatMakeHMatFromMat(&rotmat, &tmpmat);
       //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


       glPushMatrix();
       glMultMatrixf((GLfloat *)&rotmat);

       primCircleOutline3(&circlestart, newrad, 32, 0, passedColour, Z_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

       glPopMatrix();
       //stop drawing circles


       //Turn on 2D primmode
       primModeSet2();
        count++;
   }

}
//Radial Indicator 4  Circles pulsing to Outline a Sphere

#define TO_CIRCLE4_SPEED    7500.0f
#define TO_CIRCLE4_DELAY    0.5f
#define TO_CIRCLE4_DELAY_SPACEING 0.05f


real32 circle4_timeelapsed[TO_NUM_CIRCLES_3];
real32 circle4_lasttime[TO_NUM_CIRCLES_3];
real32 circle4_timeelapsed_fast[TO_NUM_CIRCLES_3];
real32 circle4_lasttime_fast[TO_NUM_CIRCLES_3];
real32 circle4distance[TO_NUM_CIRCLES_3];
real32 circle4fastdistance[TO_NUM_CIRCLES_3];
real32 circle4delay[TO_NUM_CIRCLES_3];

//Draws a special radial display for the TO of special ships: Proximity Sensor
void toDrawRadialIndicator4(ShipPtr ship, real32 radius, real32 scale,color passedColour)
{

   hmatrix rotmat;
   real32 newrad;
   vector origin = {0.0, 0.0, 0.0};
   vector up =    {1.0, 0.0, 0.0};
   vector right = {0.0, 1.0, 0.0};
   vector head =  {0.0, 0.0, 1.0};
   sdword count;
   matrix tmpmat;
   vector circlestart,circlestart2;

   count = 0;

   while(count < TO_NUM_CIRCLES_3)
   {
        vecScalarMultiply(circlestart,head,radius);

        circle4_timeelapsed[count] = taskTimeElapsed - circle4_lasttime[count];
        circle4_lasttime[count] = taskTimeElapsed;
        if(circle4delay[count] <= 0.0f)
        {
            circle4delay[count] = 0.0f;
            circle4distance[count] += circle4_timeelapsed[count]*TO_CIRCLE4_SPEED;
        }
        else
        {
            circle4delay[count] -= circle4_timeelapsed[count];
            count++;
            continue;
        }
        if(circle4distance[count] > 2*radius)      //make faster :)    precalc diameter
        {
            circle4distance[count] = 0.0f;
            if(count == 0)
            {
                circle4delay[count] = TO_CIRCLE4_DELAY;
            }
            else
            {
                circle4delay[count] = TO_CIRCLE4_DELAY_SPACEING + circle4delay[count-1];
            }
            count++;
            continue;
        }

        vecScalarMultiply(circlestart2,head,-circle4distance[count]);
        vecAddTo(circlestart,circlestart2);
        vecAddTo(circlestart,ship->posinfo.position);
        if(circle4distance[count] > radius)
        {
            newrad = 2*radius - circle4distance[count];
        }
        else
        {
            newrad = circle4distance[count];
        }
       newrad = fsqrt(radius*radius - (radius - newrad)*(radius - newrad));

       //Turn off 2D primmode
       primModeClear2();
       rndLightingEnable(FALSE);
       rndTextureEnable(FALSE);

       matPutVectIntoMatrixCol1(up,tmpmat);
       matPutVectIntoMatrixCol2(right,tmpmat);
       matPutVectIntoMatrixCol3(head,tmpmat);

       hmatMakeHMatFromMat(&rotmat, &tmpmat);
       //hmatPutVectIntoHMatrixCol4(ship->posinfo.position, rotmat);


       glPushMatrix();
       glMultMatrixf((GLfloat *)&rotmat);

       primCircleOutline3(&circlestart, newrad, 32, 0, passedColour, Z_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, X_AXIS);
       //primCircleOutline3(&ship->posinfo.position, radius, 32, 0, TO_CROSS_COLOR1, Y_AXIS);

       glPopMatrix();
       //stop drawing circles


       //Turn on 2D primmode
       primModeSet2();
        count++;
   }

}

void smDrawBandbox(bool32 bOverlayOnly)
{
    color outlineColor;
    rectangle rect;
    sdword minX = mrOldMouseX;
    sdword minY = mrOldMouseY;
    sdword maxX = screenX;
    sdword maxY = screenY;

    if (keyIsHit(GKEY) || (keyIsHit(CONTROLKEY) && keyIsHit(ALTKEY)))
    {
        outlineColor = smBandboxProtectColor;
    }
    else if (keyIsHit(ALTKEY))
    {
        outlineColor = smBandboxFocusColor;
    }
    else if (keyIsHit(CONTROLKEY))
    {
        outlineColor = smBandboxHarvestColor;
    }
    else if (kbCommandKeyIsHit(kbSHIP_SPECIAL))
    {
        outlineColor = smBandboxSpecialColor;
    }
    else
    {
        outlineColor = colWhite;
    }

    rect.x0 = mrOldMouseX;
    rect.y0 = mrOldMouseY;
    if (mrOldMouseX > screenX)
    {
        maxX = mrOldMouseX;
        minX = screenX;
        rect.x0 = screenX;
    }
    if (mrOldMouseY > screenY)
    {
        maxY = mrOldMouseY;
        minY = screenY;
        rect.y0 = screenY;
    }
    rect.x1 = maxX + 1;
    rect.y1 = maxY + 1;

    if ((maxX + 1 - minX) > selClickBoxWidth ||
        (maxY + 1 - minY) > selClickBoxHeight)
    {
        if (!bOverlayOnly)
        {
            selProtectDraw();
        }
        primGLRectOutline2(&rect, 1, outlineColor);
    }
    else if (!bOverlayOnly)
    {
        Ship *ship = selSelectionClick(
            universe.RenderList.head,
            &universe.mainCameraCommand.actualcamera,
            screenX,
            screenY,
            TRUE,
            TRUE);
        if (ship != NULL && !ioMouseOverObjectivesList())
        {
            smDrawProjectedShipOutline(ship, outlineColor);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrRegionDraw
    Description : Region callback for drawing the main region.
    Inputs      : standard region callback crap
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword mrFrameCounter = 0;
void mrRegionDraw(regionhandle reg)
{
    udword shiftBits;
    static bool32 mrClearRenderEverything = FALSE;
    sdword index;

    if (singlePlayerGame)
    {
        mrFrameCounter++;
    }
    if (aivRenderMainScreen)
    {
        //doing this directly for efficiency
//        aivarValueSet(aivRenderMainScreen, mrRenderMainScreen);
        aivRenderMainScreen->value = mrRenderMainScreen;
    }

    if ((!mrRenderMainScreen) || nisBlackFade == 1.0f)
    {
        if (nisIsRunning_ && nisFullyScissored && rndScissorEnabled)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, NIS_LetterHeight, MAIN_WindowWidth, MAIN_WindowHeight - NIS_LetterHeight * 2);
        }
        if (mrClearRenderEverything)
        {
            mrClearRenderEverything = FALSE;
            feRenderEverything = FALSE;
        }
        if (feRenderEverything)
        {
            mrClearRenderEverything = TRUE;
        }
        if (nisTextCardIndex)
        {                                                   //render text cards on a black screen
            nisTextCardListDraw();
        }
        if (nisSMPTECounter != NULL)
        {
            nisSMPTECounterDraw(thisNisPlaying, nisSMPTECounter);
        }
        for (index = 0; index < NIS_NumberStatics; index++)
        {
            if (nisStatic[index] != NULL)
            {
                nisStaticDraw(nisStatic[index]);
            }
        }

        if (gameStatsOverlayActive())
        {
            gameStatsOverlayDraw();
        }

        /*
        if (!rndScissorEnabled)
        {                                                   //if we can't scissor properly
            rndDrawScissorBars(rndScissorEnabled);
        }
        */
        return;
    }

    if (!glCapFastFeature(GL_BLEND))
    {
        if (mrWhiteOut)
        {
            rectangle rect = { 0,0,MAIN_WindowWidth,MAIN_WindowHeight };
            sdword c;
            real32 t;

            if (mrWhiteOutT < 0.5f)
            {
                t = mrWhiteOutT * 2.0f;
                c = (sdword)(t * 255.0f);
                primRectSolid2(&rect, colRGB((70*c)>>8,(70*c)>>8,(255*c)>>8));
            }
            else
            {
                t = 2.0f * (mrWhiteOutT - 0.5f);
                c = (sdword)(t * 155.0f);
                primRectSolid2(&rect, colRGB(70+c,70+c,255));
            }

            return;
        }
    }

    if (piePointSpecMode != PSM_Idle)
    {                                                       //draw point spec
        rndPostObjectCallback = piePointSpecDraw;
    }

    if (nisIsRunning_ && nisFullyScissored && rndScissorEnabled)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, NIS_LetterHeight, MAIN_WindowWidth, MAIN_WindowHeight - NIS_LetterHeight * 2);
    }
    if (nisCaptureCamera)
    {
        mrCamera = nisCamera;
    }
    else if (gameIsRunning)
    {
        mrCamera = &(universe.mainCameraCommand.actualcamera);
        ccControl(&universe.mainCameraCommand);
    }
    else
    {
        mrCamera = &defaultCamera;
    }
    if (demDemoPlaying && (demPlaybackModeFlags & DEM_ModeB))
    {
        demApplyModeBRenderState();
    }
    else if (demDemoRecording && (demRecordModeFlags & DEM_ModeB))
    {
        demRecordModeBRenderState();
    }
    rndMainViewRender(mrCamera);
    rndTextureEnable(FALSE);
    glDisable(GL_TEXTURE_2D);

    if (!thisNisPlaying)
    {
        mrTacticalOverlayState(utyCapsLockToggleState());   //make sure we have the right TO state
    }
    if (smPathMarkersEnabled)
    {                                                       //need better key mechanism
        rndTextureEnable(FALSE);
        glDisable(GL_TEXTURE_2D);

        toAllShipsDraw();                                   //draw tactical overlays
    }

    selSelectedDraw();                                      //draw overlays for selected ships, if any
    smDrawPlayerAndBorderIcons();
    if (selSelected.numShips > 0 && selSelected.ShipPtr[0] != NULL)
    {
        mrDrawTabStats(selSelected.ShipPtr[0]);
    }
    smTacticalOverlayDraw();
    ioUpdateOverlayStates();
    mrUpdateMouseDrag();
    smTacticalOverlayProcess(FALSE, 0);
    if (smBandboxEnabled == 1)                              //and then draw the current selection progress
    {
        selSelectingDraw();
        smDrawBandbox(FALSE);
    }

#if SP_DEBUGKEYS
    kasDebugDraw();
#endif

    if (singlePlayerGame)
    {
        objectiveDrawStatus();
        kasfPopupTextDraw();
    }

    if (nisTextCardIndex)
    {
        nisTextCardListDraw();
    }

    if (nisSMPTECounter != NULL)
    {
        nisSMPTECounterDraw(thisNisPlaying, nisSMPTECounter);
    }
    for (index = 0; index < NIS_NumberStatics; index++)
    {
        if (nisStatic[index] != NULL)
        {
            nisStaticDraw(nisStatic[index]);
        }
    }

    //glDisable(GL_SCISSOR_TEST);

    //this should be reworked to use the actual formation CSM

    dbgAssert(mrFormationFont != FONT_InvalidFontHandle);
    if (mrDrawFormation && (!((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)))
    {
        fonthandle oldFont = fontMakeCurrent(mrFormationFont);
        fontPrint(0, 0, colWhite, TypeOfFormationToNiceStr(mrNewFormation));

        if (universe.totaltimeelapsed - mrDrawFormationTime >= MR_FormationDelay)
            mrDrawFormation = FALSE;
        fontMakeCurrent(oldFont);
    }

    if (printCaptainMessage)
    {
        fonthandle fhSave;
        udword y;

        y = mainWindowHeight/2 - ((fontHeight("W") + 4)*numPlayerDropped)/2 - fontHeight("W") - 6;

        fhSave = fontMakeCurrent(mrBigFont);

        fontPrintCentre(y,colWhite,strGetString(strCaptainTransfering));

        fontMakeCurrent(fhSave);
    }

    if ((multiPlayerGame) && (numPlayerDropped>0))
    {
        if (universe.totaltimeelapsed < (printTimeout + TIMEOUT_TIME))
        {
            udword y;
            char   temp[80];
            fonthandle fhSave;

            fhSave = fontMakeCurrent(mrBigFont);
            y = mainWindowHeight/2 - ((fontHeight("W") + 4)*numPlayerDropped)/2;

            for (index=0;index<numPlayerDropped;index++)
            {
                sprintf(temp, "%s %s", playerNames[playersDropped[index]], (playersReadyToGo[index] == PLAYER_QUIT) ? strGetString(strQuit) : strGetString(strDroppedOut));
                fontPrintCentre(y,colWhite,temp);
                y += fontHeight("W") + 4;
            }

            fontMakeCurrent(fhSave);
        }
        else
        {
            numPlayerDropped=0;
        }
    }

    if ((mrFormationName != NULL) && (!((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)))                            //if changing formations
    {
        //fontPrint(0, 0, colWhite, mrFormationName);
        if (universe.totaltimeelapsed - mrFormationTime >= MR_FormationDelay)
        {                                                   //if on this formation long enough
            char msgName[128];

            //mrFormationName = TypeOfFormationToStr(mrNewFormation);
            mrFormationTime = universe.totaltimeelapsed;

            makeShipsFormationCapable((SelectCommand *)&selSelected);
            if(selSelected.numShips >0)
            {
                // check again if mothership or carrier in - don't want them in delta, etc.
                if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
                {
                    mrNewFormation = PARADE_FORMATION_FLAG;
                }

                strcpy(msgName, "Game_Formation_");
                if (mrNewFormation == PARADE_FORMATION_FLAG)
                {
                    strcat(msgName, strGetString(strPARADE_FORMATION));
                }
                else
                {
                    strcat(msgName, TypeOfFormationToStr(mrNewFormation));
                }
                tutGameMessage(msgName);

                mrFormationName = NULL;                         //no longer selecting formations
                if (mrNewFormation == PARADE_FORMATION_FLAG)
                {
                    speechEventQueue(NULL, 67113058, 8, -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex,
                                     -1, 5.0f, -1);
                    clWrapSetMilitaryParade(&universe.mainCommandLayer,(SelectCommand *)&selSelected);
                }
                else
                {
                    speechEventQueue(NULL, 67113058, (mrNewFormation == CUSTOM_FORMATION) ? 7 : mrNewFormation,
                                     -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
                    clWrapFormation(&universe.mainCommandLayer,     //set new formation
                        (SelectCommand *)&selSelected,mrNewFormation);
                }
            }
        }
    }

    ioDrawEventFeed();

    mrBigMessageDraw();

    //draw the bearing numbers on the horizon line
    if (smTickTextIndex > 0)
    {
        smTickTextDraw();
    }

    //compute animating stipple pattern every time mainrgn is drawn
    pieLineStippleCounter += pieLineStippleSpeed;
    shiftBits = ((udword)pieLineStippleCounter) % 16;
    pieStipplePattern = pieLineStipple | ((udword)pieLineStipple << 16);
    if ((sdword)shiftBits < 0)
    {
        pieStipplePattern >>= 16 - shiftBits;
    }
    else
    {
        pieStipplePattern >>= shiftBits;
    }

    //draw a big black rectangle to do a fade between the scissor bars
#define NIS_EXCESSSCISSORMARGIN     7       //!!! Keith fix this
    if (nisBlackFade != 0.0f)
    {                                                       //if there is some sort of NIS fade going down
        sdword y;
        if (nisScissorFade != 0.0f)
        {
            y = (sdword)((real32)NIS_LetterHeight * nisScissorFade);
        }
        else
        {
            y = NIS_LetterHeight;
        }
        glEnable(GL_BLEND);
        glColor4f(0.0f, 0.0f, 0.0f, nisBlackFade);
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(y - NIS_EXCESSSCISSORMARGIN));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight - y + NIS_EXCESSSCISSORMARGIN));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(MAIN_WindowHeight - y + NIS_EXCESSSCISSORMARGIN));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth), primScreenToGLY(y - NIS_EXCESSSCISSORMARGIN));
        glEnd();
        glDisable(GL_BLEND);
    }

    if (!nisIsRunning_ && cat_mrBlackOut > 0.0f)
    {
        glEnable(GL_BLEND);
        rndAdditiveBlends(FALSE);
        glColor4f(0.0f, 0.0f, 0.0f, cat_mrBlackOut);
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(-1));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight + 1));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth + 1), primScreenToGLY(MAIN_WindowHeight + 1));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth + 1), primScreenToGLY(-1));
        glEnd();
        glDisable(GL_BLEND);
    }

    if (cat_mrWhiteOutT > 0.0f)
    {
        glEnable(GL_BLEND);
        rndAdditiveBlends(TRUE);
        glColor4f(1.0f, 1.0f, 1.0f, cat_mrWhiteOutT);
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(-1));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight + 1));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth + 1), primScreenToGLY(MAIN_WindowHeight + 1));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth + 1), primScreenToGLY(-1));
        glEnd();
        rndAdditiveBlends(FALSE);
        glDisable(GL_BLEND);
    }

    if (mrWhiteOut)
    {
        rectangle rect = { -1,0,MAIN_WindowWidth,MAIN_WindowHeight };
        sdword c;
        real32 t;

        if (mrWhiteOutT < 0.5f)
        {
            t = mrWhiteOutT * 2.0f;
            c = (sdword)(t * 255.0f);
            glEnable(GL_BLEND);
            rndAdditiveBlends(TRUE);
            primRectSolid2(&rect, colRGBA(70,70,255,c));
            rndAdditiveBlends(FALSE);
            glDisable(GL_BLEND);
        }
        else
        {
            t = 2.0f * (mrWhiteOutT - 0.5f);
            c = (sdword)(t * 155.0f);
            primRectSolid2(&rect, colRGB(70 + c, 70 + c, 255));
        }
    }
    if (spPostLoadNisBlackout)
    {
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(-1));
        glVertex2f(primScreenToGLX(-1), primScreenToGLY(MAIN_WindowHeight + 1));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth + 1), primScreenToGLY(MAIN_WindowHeight + 1));
        glVertex2f(primScreenToGLX(MAIN_WindowWidth + 1), primScreenToGLY(-1));
        glEnd();
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrReset
    Description : Reset the state of the main region, such as turning off camera rotation.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrReset(void)
{
    smBandboxEnabled = 0;
    mrRightMouseCameraMode = 0;
    mouseCursorShow();

    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }

    if (RGLtype == SWtype)
    {
        rglFeature(RGL_SPEEDY);
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrNISStarting
    Description : Called from NIS.c when an NIS, this function will clear out
                    any persistant control variables.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrNISStarting(void)
{
    if (smBandboxEnabled)
    {
        attackcom.numTargets = 0;
        smBandboxEnabled = 0;
    }
    if (mrRightMouseCameraMode)
    {
        mrRightMouseCameraMode = 0;
    }
    mrSavedPathMarkersEnabled = smPathMarkersEnabled;
    smPathMarkersEnabled = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : mrNISStopping
    Description : Like the above function, except it's called when an NIS ends.
                    It can optionally reset previously cleared variables.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrNISStopping(void)
{
    if (!nisNoLockout)
    {
        if (smBandboxEnabled)
        {
            selRectNone();
            smBandboxEnabled = 0;
            selSelecting.numTargets = 0;
        }
        if (mrRightMouseCameraMode)
        {
            mrRightMouseCameraMode = 0;
        }
        smPathMarkersEnabled = mrSavedPathMarkersEnabled;
    }
    if (mrHoldLeft != mrNULL)
    {                                                       //if player is selecting
        mrSelectionRect.x0 = mrSelectionRect.x1 =           //selecting nothing
            mrSelectionRect.y0 = mrSelectionRect.y1 = 0;
        selRectNone();
        mrHoldLeft = mrNULL;
    }
    if (mrHoldRight != mrNULL)
    {                                                       //if rotating mouse
        mrHoldRight = mrNULL;                               //stop it!
    }
    mrDrawTactical = mrSaveTactical;                        //turn on the tactical overlay if it was on before
}

/*-----------------------------------------------------------------------------
    Name        : mrShipDied
    Description : Called when a ship died to remove any persistant references.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mrShipDied(Ship *ship)
{
    if (mrMothershipPtr == ship)
    {
        mrMothershipPtr = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mrTacticalOverlayState
    Description : Sets the state of the tactical overlay.
    Inputs      : bTactical - TRUE for on, FALSE for off.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mrTacticalOverlayState(bool32 bTactical)
{
    if (kbKeyBoundToCommand(kbTACTICAL_OVERLAY)==CAPSLOCKKEY)
    {
        mrDrawTactical = bTactical;
        smPathMarkersEnabled = bTactical;
    }
}

#if MR_KEYBOARD_CHEATS
void mrAddResources(void)
{
    #define MR_RESOURCE_INCREASE 1000

    dbgMessage("\nAdding Resources");
    universe.curPlayerPtr->resourceUnits += MR_RESOURCE_INCREASE;
}

void mrAllTechnology(void)
{
    dbgMessage("\nAdding All Technologies");
    rmInitializeResearchStruct(universe.curPlayerPtr,TRUE,TECH_ALLTECHNOLOGY);
}

extern bool32 cmCheapShips;
void mrCheapShips(void)
{
    cmCheapShips = !cmCheapShips;
    if (cmCheapShips)
        dbgMessage("\nCheap Ships On");
    else
        dbgMessage("\nCheap Ships Off");

}

void mrToggleAI(void)
{
    mrNoAI = !mrNoAI;
    if (mrNoAI)
        dbgMessage("\nComputer AI On");
    else
        dbgMessage("\nComputer AI Off");
}


void mrAsteroids(void)
{

    dbgMessage("\nAsteroids");
    mrTradeStuffTest(NULL, NULL);
}

bool32 gMosaic = 0;
void mrBackground(void)
{
    gMosaic = !gMosaic;
}

void mrFilter(void)
{
    texLinearFiltering = !texLinearFiltering;
    trFilterEnable(texLinearFiltering);
}

#define MR_DEBUG_SIZE 16
#define MR_SINGLEPLAYER     0x0001      //allow in singleplayer
#define MR_NETGAME          0x0002      //allow in netgames

typedef void (*debugFunc)(void);    //callback function


typedef struct debugStructX
{
    char *string;
    debugFunc callback;
    udword mask;
}
debugStruct;

debugStruct mrDebugFunctions[] = {
    //for now, don't use QWIOGJN in the string - they're intercepted elsewhere

    //backwards string,        callback,             permission masks

    {"YMMUY",                   mrAddResources,      MR_SINGLEPLAYER},
    {"HCETLLA",                 mrAllTechnology,     MR_SINGLEPLAYER},
    {"PAEHC",                   mrCheapShips,        MR_SINGLEPLAYER},
    {"YMMUD",                   mrToggleAI,          MR_SINGLEPLAYER},
    {"RETSA",                   mrAsteroids,         0},
    {"YVURJ",                   mrBackground,        MR_SINGLEPLAYER | MR_NETGAME},
    {"RTLF",                    mrFilter,            MR_SINGLEPLAYER | MR_NETGAME},
    {"",                        NULL,                0},
};





char mrDebugKeyBuffer[MR_DEBUG_SIZE];
sdword mrDebugPointer;



bool32 mrPlayerHasCheated = FALSE;

void mrScanDebugCodes(sdword ID)
{
    sdword i,j,k;
    bool32 match;

    mrDebugPointer = (mrDebugPointer+1) & (MR_DEBUG_SIZE-1);
    mrDebugKeyBuffer[mrDebugPointer] = ID;

    for (i=0; mrDebugFunctions[i].callback!=NULL; i++)
    {
        match = TRUE;
        for (k=0,j = mrDebugPointer; mrDebugFunctions[i].string[k]>0; k++,j=(j+MR_DEBUG_SIZE-1) & (MR_DEBUG_SIZE-1))
        {
            if (mrDebugKeyBuffer[j] != mrDebugFunctions[i].string[k])
            {
                match = FALSE;
                break;
            }
        }
        if (match)
        {
            if (singlePlayerGame)
            {
                if (!(mrDebugFunctions[i].mask & MR_SINGLEPLAYER))
                {
                    return;
                }
            }

            if (multiPlayerGame)
            {
                if (!(mrDebugFunctions[i].mask & MR_NETGAME))
                {
                    return;
                }
            }

            mrPlayerHasCheated = TRUE;
            dbgMessage("\nDebug code entered");
            //add SFX
            (mrDebugFunctions[i].callback)();

        }

    }
}

#endif //MR_KEYBOARD_CHEATS
/* ============================================================================
   Cataclysm decompiled references migrated from former src\\Game\\mainrgn.c
   Live authoritative code remains above; reconcile Cataclysm deltas here.
   ============================================================================ */

static Node *mrBandboxSelectionStartNode(sdword clickNode)
{
    return (clickNode == 1) ? universe.SpaceObjList.head : universe.RenderList.head;
}

bool32 mrHandleBandboxPressLeft(void)
{
    if (smSensorsActiveGet())
    {
        return TRUE;
    }
    if (mrTabStatsStartDrag())
    {
        return TRUE;
    }
    if (mrRightMouseCameraMode || piePointSpecMode != PSM_Idle)
    {
        return FALSE;
    }

    mrOldMouseX = mouseCursorX();
    mrOldMouseY = mouseCursorY();
    smBandboxEnabled = 1;
    return TRUE;
}

void mrHandleBandboxPressRight(rectangle *playerRect)
{
    if (mrTabStatsIsDragging() || mouseCursorTypeGet())
    {
        return;
    }

    if (smSensorsActiveGet())
    {
        if (!smOverlayHandleMouseClick() && smClickedOnPlayer(playerRect) != -1)
        {
            return;
        }
        smOverlayClose();
    }

    if (smBandboxEnabled == 1)
    {
        smBandboxEnabled = 0;
        selSelecting.numTargets = 0;
        attackcom.numTargets = 0;
    }

    piePointModePause(TRUE);
    mrRightMouseCameraMode = 2;
    mrOldMouseX = mouseCursorX();
    mrOldMouseY = mouseCursorY();
    mouseCursorHide();

    if (helpinfoactive)
    {
        mousePositionSet(helpinforegion->rect.x0 / 2, MAIN_WindowHeight / 2);
    }
    else
    {
        mousePositionSet(MAIN_WindowWidth / 2, MAIN_WindowHeight / 2);
    }

    mrMouseHasMoved = 0;
}

void mrHandleBandboxHoldLeft(sdword clickNode)
{
    if (!mrDisabled && smBandboxEnabled == 1)
    {
        mrSelectHold(&(universe.mainCameraCommand.actualcamera), clickNode);
    }
}

static void mrSelectLeftRelease(sdword clickNode)
{
    Node *listHead = mrBandboxSelectionStartNode(clickNode);
    Ship *clickedShip;
    bool32 addToSelection = kbCommandIsHitByName("Multiple_Select");

    if (mrDisabled)
    {
        kbInvokeCommandCallbacks(clickNode, 107);
        gameStatsOverlayProcess();
        return;
    }

    if (ioHandleOverlayClick(addToSelection) || mrRightMouseCameraMode)
    {
        return;
    }

    clickedShip = selSelectionClick(
        listHead,
        &(universe.mainCameraCommand.actualcamera),
        screenX,
        screenY,
        TRUE,
        TRUE);

    if (clickedShip != NULL)
    {
        if ((tutorial != TUTORIAL_ONLY) || tutEnable.bClickSelect)
        {
            mrObjectClick(clickedShip);
        }
        return;
    }

    if (mouseCursorObjLast != NULL &&
        mouseCursorObjLast->objtype == OBJ_ShipType &&
        mouseCursorLastObjTime + mrFastMoveShipClickTime >= universeRealtimeElapsed &&
        abs(screenX - mouseCursorLastX) < mrFastMoveShipClickX &&
        abs(screenY - mouseCursorLastY) < mrFastMoveShipClickY)
    {
        if (((tutorial != TUTORIAL_ONLY) || tutEnable.bClickSelect) &&
            bitTest(mouseCursorObjLast->flags, SOF_Selectable))
        {
            mrObjectClick((Ship *)mouseCursorObjLast);
        }
        return;
    }

    if (!addToSelection && ((tutorial != TUTORIAL_ONLY) || tutEnable.bCancelSelect))
    {
        selSelected.numShips = 0;
        ioUpdateShipTotals();
    }
}

void mrHandleBandboxReleaseLeft(sdword clickNode)
{
    if (smOverlayIsOpenGet())
    {
        smBandboxEnabled = 0;
        selSelecting.numTargets = 0;
        attackcom.numTargets = 0;
        {
            rectangle defaultRect = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};
            smOverlayHandlePopupClick(&defaultRect);
        }
        return;
    }
    if (smSensorsActiveGet())
    {
        if (piePointSpecMode == PSM_XY || piePointSpecMode == PSM_Z)
        {
            piePointSpecInvokeCallback();
        }
        else
        {
            smOverlayHandleMouseClick();
        }
        return;
    }

    mrTabStatsEndDrag();
    if (!kbSuppressSpecialReleaseOverlay)
    {
        if (piePointSpecMode == PSM_XY || piePointSpecMode == PSM_Z)
        {
            piePointSpecInvokeCallback();
        }
        else if (smBandboxEnabled == 1)
        {
            if (!mrProcessBandboxSelection(&(universe.mainCameraCommand.actualcamera), clickNode))
            {
                mrSelectLeftRelease(clickNode);
            }
            smBandboxEnabled = 0;
            selSelecting.numTargets = 0;
            attackcom.numTargets = 0;
            piePointModePause(FALSE);
        }
    }
}

static bool32 mrProcessBandboxSelection(Camera *camera, sdword clickNode)
{
    Node *startNode = mrBandboxSelectionStartNode(clickNode);
    rectangle selectionRect;
    MaxSelection filteredSelection;
    sdword targetIndex;

    if (universePause || mrDisabled)
    {
        return FALSE;
    }
    if (abs(mouseCursorX() - mrOldMouseX) < selClickBoxWidth &&
        abs(mouseCursorY() - mrOldMouseY) < selClickBoxHeight)
    {
        return FALSE;
    }

    mrSelectHold(camera, clickNode);
    selectionRect = mrSelectionRect;
    if (kbCommandIsHitByName("Bandbox_ForceAttack"))
    {
        if (!playPackets && attackcom.numTargets > 0)
        {
            MakeShipsAttackCapable((SelectCommand *)&filteredSelection, (SelectCommand *)&selSelected);
            selSelectionCopyBounded(&selSelected, &filteredSelection);
            if (selSelected.numShips > 0)
            {
                MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&attackcom, universe.curPlayerPtr);
                MakeShipsNotIncludeTheseShips((SelectCommand *)&attackcom, (SelectCommand *)&selSelected);
                if (attackcom.numTargets > 0)
                {
                    speechEventReportProtectTargetSelection();
                    clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&attackcom);
                    tutGameMessage("Game_BandBoxForceAttack");
                }
            }
        }
        goto bandboxDone;
    }

    if (kbCommandIsHitByName("Multiple_Select"))
    {
        if (piePointSpecMode != PSM_Waiting &&
            ((tutorial != TUTORIAL_ONLY) || tutEnable.bBandSelect))
        {
            selRectDragAddFunction(startNode, camera, &selectionRect);
            ioUpdateShipTotals();
            tutGameMessage("MultipleSelect");
        }
        goto bandboxDone;
    }

    if (kbCommandIsHitByName("Bandbox_Guard"))
    {
        if (!playPackets && attackcom.numTargets > 0)
        {
            makeShipsCommandControllable((int *)&selSelected.numShips, 1);
            MakeShipsFriendlyShips((SelectCommand *)&attackcom, universe.curPlayerPtr);
            MakeShipsNotIncludeTheseShips((SelectCommand *)&attackcom, (SelectCommand *)&selSelected);
            if (attackcom.numTargets > 0 && selSelected.numShips > 0)
            {
                speechEventQueue(NULL, 67113057, -1, -1, 0,
                                 selSelected.ShipPtr[0]->playerowner->playerIndex,
                                 -1, 5.0f, -1);
                clWrapProtect(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (ProtectCommand *)&attackcom);
                tutGameMessage("Game_BandBoxGuard");
            }
        }
        goto bandboxDone;
    }

    if (kbCommandIsHitByName("Ship_Special"))
    {
        if (!playPackets)
        {
            MakeShipsSpecialTargetCapable((SelectCommand *)&selSelected, FALSE);
            if (selSelected.numShips > 0)
            {
                kbSuppressSpecialReleaseOverlay = 1;
                MakeShipsNotIncludeTheseShips((SelectCommand *)&attackcom, (SelectCommand *)&selSelected);
                MakeTargetsNotIncludeMissiles((SelectAnyCommand *)&attackcom);
                if (attackcom.numTargets > 0)
                {
                    smOverlayOpen(TRUE, (SelectAnyCommand *)&attackcom);
                    tutGameMessage("Game_BandBoxSpecial");
                }
            }
        }
        goto bandboxDone;
    }

    if (!kbCommandIsHitByName("Bandbox_Focus"))
    {
        if (kbCommandIsHitByName("Bandbox_Attack"))
        {
            if (!playPackets && attackcom.numTargets > 0)
            {
                MakeShipsAttackCapable((SelectCommand *)&filteredSelection, (SelectCommand *)&selSelected);
                selSelectionCopyBounded(&selSelected, &filteredSelection);
                if (selSelected.numShips > 0)
                {
                    MakeTargetsOnlyNonForceAttackTargets((SelectAnyCommand *)&attackcom, universe.curPlayerPtr);
                    MakeShipsNotIncludeTheseShips((SelectCommand *)&attackcom, (SelectCommand *)&selSelected);
                    MakeTargetsSalvageable((SelectAnyCommand *)&attackcom, universe.curPlayerPtr);
                    if (attackcom.numTargets > 0)
                    {
                        speechEventReportProtectTargetSelection();
                        clWrapAttack(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (AttackCommand *)&attackcom);
                        tutGameMessage("Game_BandBoxAttack");
                    }
                }
                tutGameMessage("BandAttack");
            }
        }
        else if (piePointSpecMode != PSM_Waiting &&
                 ((tutorial != TUTORIAL_ONLY) ||
                  (tutEnable.bBandSelect &&
                   (attackcom.numTargets != 0 || tutEnable.bCancelSelect))))
        {
            selRectDragFunction(
                startNode,
                camera,
                &selectionRect,
                (SpaceObjRotImpTarg **)selSelected.ShipPtr,
                &selSelected.numShips,
                TRUE,
                FALSE,
                FALSE);
            ioUpdateShipTotals();
            if (selSelected.numShips > 0)
            {
                if (!playPackets)
                {
                    if (universe.curPlayerPtr->PlayerMothership != NULL &&
                        selShipInSelection(selSelected.ShipPtr, selSelected.numShips, universe.curPlayerPtr->PlayerMothership) &&
                        universe.curPlayerPtr->PlayerMothership->staticinfo->shipclass == CLASS_Mothership)
                    {
                        speechEventQueue(NULL, COMM_Selection, universe.curPlayerPtr->PlayerMothership->shiptype,
                                         -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
                    }
                    else
                    {
                        speechEventQueue(selSelected.ShipPtr[0], COMM_Selection, selSelected.ShipPtr[0]->shiptype,
                                         -1, -1, -1, -1, 0.0f, -1);
                    }
                }
                tutGameMessage("Game_BandBoxSelect");
            }
        }
        goto bandboxDone;
    }

    if (attackcom.numTargets > 0)
    {
        if (clickNode != 1)
        {
            ccFocusClose(&(universe.mainCameraCommand), (FocusCommand *)&attackcom);
            attackcom.numTargets = 0;
            tutGameMessage("Game_BandBoxFocus");
            tutGameMessage("AltFocus");
            goto bandboxDone;
        }

        for (targetIndex = 0; targetIndex < attackcom.numTargets; )
        {
            if (attackcom.TargetPtr[targetIndex]->objtype != OBJ_ShipType ||
                ccFocusIsLegal((SpaceObj *)attackcom.TargetPtr[targetIndex]))
            {
                targetIndex++;
            }
            else
            {
                attackcom.TargetPtr[targetIndex] = attackcom.TargetPtr[--attackcom.numTargets];
            }
        }
        if (attackcom.numTargets > 0)
        {
            cat_smZoomInOnFocus();
        }
    }
    tutGameMessage("AltFocus");

bandboxDone:
    ioUpdateShipTotals();
    attackcom.numTargets = 0;
    return TRUE;
}

sdword mrUpgradeSelection(void)
{
    if (playPackets || universePause || mrDisabled)
    {
        return 0;
    }

    if (tutorial == 1 && ((*(udword *)&tutEnable & 0x8000) == 0))
    {
        mrMenuDontDisappear = 1;
        return 0;
    }

    MakeShipsUpgradeCapable(&selSelected.numShips, TRUE);
    if (selSelected.numShips > 0)
    {
        clWrapHalt(&universe.mainCommandLayer, (SelectCommand *)&selSelected);
        return clWrapUpgrade(&universe.mainCommandLayer, (SelectCommand *)&selSelected, 0);
    }

    return 0;
}

void smViewportSelectLeftRelease(sdword clickNode)
{
    if (!mrTabStatsIsDragging() && !smSensorsActiveGet() && !mouseCursorTypeGet())
    {
        mrSelectLeftRelease(clickNode);
    }
}

extern sdword kasfMothershipBigGunDisabled;

void *mrSelectMothership(void)
{
    Ship *mothership;

    if (playPackets || universePause || mrDisabled)
    {
        return NULL;
    }

    mothership = universe.curPlayerPtr->PlayerMothership;
    if (mothership == NULL)
    {
        return NULL;
    }

    if (singlePlayerGame && kasfMothershipBigGunDisabled)
    {
        return (void *)clCommandMessage(strGetString(strCantFireBigGun), -1);
    }

    if (!mothershipHasReadyBigGun(mothership))
    {
        telldir();
        return NULL;
    }

    selSelected.numShips = 1;
    selSelected.ShipPtr[0] = mothership;
    ioUpdateShipTotals();
    return mothershipBeginBigGunTarget(mothership);
}

sdword *__cdecl mrDetermineSelectionActions(sdword *result, SelectAnyCommand *selection)
{
    sdword actionState[41];
    bool32 haveLeechMode;
    sdword i;

    memset(actionState, 0, sizeof(actionState));
    haveLeechMode = FALSE;

    if (selection->numTargets > 0)
    {
        actionState[32] = 1;
        actionState[33] = 1;
        actionState[34] = 1;
        actionState[35] = 1;
        actionState[36] = 1;
        actionState[37] = 1;
        actionState[38] = 1;
        actionState[39] = 1;
    }

    for (i = 0; i < selection->numTargets; i++)
    {
        TargetPtr target;

        target = selection->TargetPtr[i];
        if (target->objtype >= OBJ_AsteroidType && target->objtype <= OBJ_DustType)
        {
            actionState[6] = 1;
            continue;
        }

        if (target->objtype == OBJ_MissileType)
        {
            if (((Missile *)target)->missileType == MISSILE_Mine)
            {
                actionState[4] = 1;
            }
            continue;
        }

        if (target->objtype == OBJ_ShipType && ((Ship *)target)->playerowner != NULL)
        {
            Ship *ship;
            uword reasonCounts[9];

            ship = (Ship *)target;
            if (univIsObjFriendly((SpaceObj *)ship, universe.curPlayerPtr))
            {
                if (!(singlePlayerGame &&
                      singlePlayerGameInfo.currentMission == MISSION_2_OUTSKIRTS_OF_KHARAK_SYSTEM &&
                      ship->shiptype == dMessagePod))
                {
                    actionState[5] = 1;
                }
            }
            else if (shipModuleSlotIsEligible(ship, universe.curPlayerPtr))
            {
                actionState[6] = 1;
            }
            else if (!shipIsModuleSlotActive(ship, universe.curPlayerPtr))
            {
                if (ship->shiptype != dBigGunUnit &&
                    !bitTest(ship->staticinfo->shipFlags, SSTATF_BeastShip) &&
                    !bitTest(ship->attributes, ATTRIBUTES_KillerCollDamage) &&
                    ship->shiptype != dEcclesiamInfected)
                {
                    actionState[4] = 1;
                }
            }

            if (shipCanSingleClickSpecialActivate(ship))
            {
                actionState[2] = 1;
            }
            if (shipCanSpecialTarget(ship))
            {
                actionState[1] = 1;
            }
            if ((singlePlayerGame || bitTest(tpGameCreated.flag, MG_HarvestinEnabled)) &&
                !shipHasCataBusyState(ship) &&
                ShipTypeIsWorker(ship->shiptype))
            {
                actionState[0] = 1;
            }
            if ((ship->shiptype == sSentinel || ship->shiptype == bSentinel) &&
                !salCapIsActiveOrBeaming(ship) &&
                ship->cataBusyStateWindow.dockingship == NULL)
            {
                actionState[3] = 1;
            }

            memset(reasonCounts, 0, sizeof(reasonCounts));
            if ((singlePlayerGame &&
                 singlePlayerGameInfo.currentMission == MISSION_10_SUPER_NOVA_STATION &&
                 ship->shiptype == dBigGun) ||
                SalCapCanTargetShipWithReasonCounts(universe.curPlayerPtr, target, reasonCounts) ||
                (reasonCounts[0] == 0 &&
                 reasonCounts[2] == 0 &&
                 reasonCounts[4] == 0 &&
                 reasonCounts[5] == 0 &&
                 reasonCounts[8] == 0))
            {
                actionState[7] = 1;
            }

            switch (ship->shiptype)
            {
                case dBigGun:
                    actionState[8] = 1;
                    break;
                case dBigGunUnit:
                    actionState[9] = 1;
                    break;
                case dSlipgateLarge:
                    actionState[10] = 1;
                    break;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EMP))
            {
                actionState[13] = 1;
            }
            else
            {
                actionState[33] = 0;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY))
            {
                actionState[11] = 1;
                if (shipHasReadySwarmers(ship))
                {
                    actionState[12] = 1;
                }
            }
            else
            {
                actionState[35] = 0;
            }

            if (mothershipHasReadyBigGun(ship))
            {
                actionState[16] = 1;
                if (mothershipBigGunCanFire(ship))
                {
                    actionState[17] = 1;
                }
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_LINK_PRIMARY))
            {
                actionState[18] = 1;
            }
            else if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_LINK_SECONDARY))
            {
                actionState[19] = 1;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY))
            {
                actionState[14] = 1;
                if (shipIsInfected(ship))
                {
                    actionState[15] = 1;
                }
                if (haveLeechMode)
                {
                    if ((ubyte)actionState[40] != mrLeechGetMode(ship))
                    {
                        actionState[40] = 0;
                    }
                }
                else
                {
                    actionState[40] = mrLeechGetMode(ship);
                    haveLeechMode = TRUE;
                }
            }
            else
            {
                actionState[32] = 0;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MIMIC))
            {
                actionState[22] = 1;
                if (shipHasModuleSlotState(ship))
                {
                    actionState[24] = 1;
                }
                if (ship->shiptype == sMCV)
                {
                    actionState[23] = 1;
                }
            }
            else
            {
                actionState[34] = 0;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPULSE))
            {
                actionState[25] = 1;
            }

            if (mrShipCanUseBigGun(ship))
            {
                actionState[20] = 1;
            }
            else
            {
                actionState[37] = 0;
            }

            if (ship->shiptype == sRammingFrigate || ship->shiptype == bRammingFrigate)
            {
                actionState[21] = 1;
            }
            else
            {
                actionState[36] = 0;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_1) ||
                ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_2) ||
                ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_3))
            {
                actionState[26] = 1;
                if (salCapIsActiveOrBeaming(ship))
                {
                    actionState[27] = 1;
                }
            }
            else
            {
                actionState[39] = 0;
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CLOAKING))
            {
                if (univIsShipCloaked((SpaceObj *)ship))
                {
                    actionState[29] = 1;
                }
                else
                {
                    actionState[28] = 1;
                }
            }

            if (acolyteIsLinked(ship))
            {
                actionState[31] = 1;
            }
            else
            {
                actionState[38] = 0;
            }

            continue;
        }

        memset(&actionState[32], 0, sizeof(sdword) * 6);
        actionState[39] = 0;
    }

    memcpy(result, actionState, sizeof(actionState));
    return result;
}

// Cataclysm-specific: retail Ship Special activation only issues the generic special
// order for upgrade-0x400 ships and does not open the Sentinel forcefield submenu.
udword mrActivateSelectionSpecial(void)
{
    MaxSelection selection;
    sdword i;

    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    if (playPackets || universePause || mrDisabled)
    {
        return FALSE;
    }

    for (i = 0; i < selection.numShips;)
    {
        if (ShipHasUpgrade(selection.ShipPtr[i], RESEARCH_UPGRADE_MASK_REPULSE))
        {
            i++;
            continue;
        }

        selection.numShips--;
        selection.ShipPtr[i] = selection.ShipPtr[selection.numShips];
    }
    MakeShipsNotDockedExceptSentinelLeechTypes((SelectCommand *)&selection);
    if (selection.numShips > 0)
    {
        clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selection, NULL);
    }

    return selection.numShips;
}

sdword mrUnleechSelection(void)
{
    MaxSelection selection;

    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    if (!playPackets && !universePause && !mrDisabled)
    {
        MakeShipsOnlyInfected(&selection.numShips);
        MakeShipsNotDockedExceptSentinelLeechTypes((SelectCommand *)&selection);
        if (selection.numShips > 0)
        {
            clWrapSetUnleech(&universe.mainCommandLayer, (SelectCommand *)&selection);
        }
        else
        {
            telldir();
        }
        return selection.numShips;
    }
    return 0;
}

// Cataclysm-specific: finds the nearest visible slipgate to the current selection and issues the retail transit dock order.
sdword __cdecl mrEnterNearestSlipgate(void)
{
    Ship *nearestSlipgate = NULL;
    vector averagePosition = { 0.0f, 0.0f, 0.0f };
    real32 nearestDistance = 1.0e21f;
    real32 temp;
    Node *node;
    sdword i;

    if (playPackets || universePause || mrDisabled)
    {
        return playPackets ? playPackets : (universePause ? universePause : mrDisabled);
    }

    if (selSelected.numShips > 0)
    {
        for (i = 0; i < selSelected.numShips; i++)
        {
            vecAddTo(averagePosition, selSelected.ShipPtr[i]->posinfo.position);
        }
        vecDivideByScalar(averagePosition, (real32)selSelected.numShips, temp);
    }

    if (!(singlePlayerGame && singlePlayerGameInfo.currentMission == 15))
    {
        univSetCurrentSensorCoverage((sdword)universe.curPlayerPtr->fogGrid);
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *candidate = (Ship *)node->structptr;
            real32 distance;

            switch (candidate->shiptype)
            {
                case dSlipgateSmall:
                case dSlipgateMedium:
                case dSlipgateLarge:
                    break;
                default:
                    continue;
            }

            if (fogGetGridValue(candidate->posinfo.position.x,
                                candidate->posinfo.position.y,
                                candidate->posinfo.position.z) != 1)
            {
                continue;
            }

            distance = GetDistance(&averagePosition, &candidate->posinfo.position);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestSlipgate = candidate;
            }
        }
    }

    if (nearestSlipgate == NULL)
    {
        clCommandMessage(strGetString(strNoSlipgateFoundNotify), -16776961);
        telldir();
        return FALSE;
    }

    MakeShipsDockCapableWithShipType(&selSelected, nearestSlipgate->shiptype, TRUE);
    if (selSelected.numShips > 0)
    {
        telldir();
        clWrapDock(&universe.mainCommandLayer, &selSelected, 10, nearestSlipgate);
    }

    return selSelected.numShips;
}

void mrMoveSelection(void)
{
    vector dist;

    tutGameMessage("KB_Move");
    if (!playPackets && !mrDisabled)
    {
        if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bMove)
        {
            mrMenuDontDisappear = TRUE;
            return;
        }
        MakeShipsMobile((SelectCommand *)&selSelected);
        makeShipsControllable((SelectCommand *)&selSelected, COMMAND_MOVE);
        if (!mrRightMouseCameraMode && !smBandboxEnabled && selSelected.numShips >= 1)
        {
            selCentrePointCompute();
            if (piePointSpecMode != PSM_Idle)
            {
                piePointModeOnOff();
            }
            else
            {
                vecSub(dist, selCentrePoint, universe.mainCameraCommand.actualcamera.lookatpoint);
                if (vecMagnitudeSquared(dist) > MAX_MOVE_DISTANCE)
                {
                    if (!smSensorsActive)
                    {
                        smSensorsBegin(NULL, NULL);
                    }
                }
                piePointModeBegin(PiePointSpecTypeMove, mrMoveSelectionPointSpecified);
            }
        }
    }
}


/* ============================================================================
   Additional decompiled mainrgn references imported from IDA
   ============================================================================ */

// Cataclysm-specific: dispatches targeted single-click specials selected from the right-click path.
sdword __cdecl mrIssueSelectionSingleClickSpecial(SelectAnyCommand *targetSelection, sdword specialType)
{
    MaxSelection selection;
    SelectCommand selectcom;
    AttackCommand attackcom;
    sdword result;
    sdword targetIndex;
    bool32 playSupportSpeech = TRUE;
    sdword msgSentReason1 = FALSE;
    sdword msgSentReason2 = FALSE;
    sdword msgSentReason4 = FALSE;
    sdword msgSentReason5 = FALSE;
    sdword battleChatSent = FALSE;

    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    MakeShipsNotDockedExceptSentinelLeechTypes((SelectCommand *)&selection);

    switch (specialType)
    {
        case 0:
            if (targetSelection != NULL)
            {
                MakeShipsOnlyHaveUpgrade(&selection.numShips, 0x80000);
                targetIndex = 0;
                while (targetIndex < targetSelection->numTargets)
                {
                    uword reasonCounts[9];

                    memset(reasonCounts, 0, sizeof(reasonCounts));
                    if (SalCapCanTargetShipWithReasonCounts(universe.curPlayerPtr,
                                                           (SpaceObjRotImpTargGuidanceShipDerelict *)targetSelection->TargetPtr[targetIndex],
                                                           reasonCounts))
                    {
                        targetIndex++;
                    }
                    else
                    {
                        if (reasonCounts[1] > 0)
                        {
                            if (!msgSentReason1)
                            {
                                clCommandMessage(mrSalCapNoSalvageableShipsCommandMessage, 0xFF0000FF);
                                msgSentReason1 = TRUE;
                            }
                        }
                        else if (reasonCounts[2] > 0 && reasonCounts[2] != reasonCounts[3])
                        {
                            playSupportSpeech = FALSE;
                            speechEventQueue(0, 33558586, -1, -1, 0, -1, -1, 5.0f, -1);
                            if (!msgSentReason2)
                            {
                                clCommandMessage(mrSalCapNeedMoreSalvagersCommandMessage, 0xFF0000FF);
                                msgSentReason2 = TRUE;
                            }
                        }
                        else if (reasonCounts[5] > 0 && reasonCounts[5] != reasonCounts[3])
                        {
                            playSupportSpeech = FALSE;
                            if (!msgSentReason5)
                            {
                                clCommandMessage(mrSalCapNeedsInfectionTechCommandMessage, 0xFF0000FF);
                                msgSentReason5 = TRUE;
                            }
                        }
                        else if (reasonCounts[4] > 0)
                        {
                            playSupportSpeech = FALSE;
                            if (!msgSentReason4)
                            {
                                clCommandMessage(mrSalCapNeedsCrystalTechCommandMessage, 0xFF0000FF);
                                msgSentReason4 = TRUE;
                            }
                        }
                        targetSelection->numTargets--;
                        targetSelection->TargetPtr[targetIndex] = targetSelection->TargetPtr[targetSelection->numTargets];
                    }
                }
            }
            goto LABEL_checkNumShips;

        case 1:
            MakeShipsSingleClickSpecialCapable((SelectCommand *)&selection, (SelectCommand *)&selection);
            goto LABEL_checkNumShips;

        case 2:
            MakeShipsInfectionCapable(&selection.numShips, TRUE);
            goto LABEL_checkNumShips;

        case 3:
            MakeShipsOnlyHaveUpgrade(&selection.numShips, 0x800);
            goto LABEL_checkNumShips;

        case 4:
            MakeShipsOnlyHaveUpgrade(&selection.numShips, 0x200);
            result = selection.numShips;
            if (result <= 0)
                return result;
            makeShipsNotIncludeSinglePlayerMotherships((SelectCommand *)&selection);
            result = selection.numShips;
            if (result > 0)
                goto LABEL_issueCommand;
            clCommandMessage(mrCantEMPTargetsCommandMessage, 0xFF0000FF);
            goto LABEL_checkNumShips;

        case 5:
            MakeShipsOnlyHaveUpgrade(&selection.numShips, 0x10000);
            goto LABEL_checkNumShips;

        case 6:
            MakeShipsOnlyRammingFrigates((SelectCommand *)&selection, TRUE);
            goto LABEL_checkNumShips;

        case 7:
            MakeShipsOnlyAcolytesWithLinkMissiles(&selection.numShips);
            goto LABEL_checkNumShips;

        default:
            goto LABEL_checkNumShips;
    }

LABEL_checkNumShips:
    result = selection.numShips;
    if (result <= 0)
        return result;

LABEL_issueCommand:
    if (targetSelection != NULL)
    {
        switch (specialType)
        {
            case 0:
                clWrapSpecialCloak(&universe.mainCommandLayer, (SelectCommand *)&selection, (AttackCommand *)targetSelection, TRUE);
                result = playSupportSpeech;
                if (playSupportSpeech)
                    clPlaySpeechEventsForSupportCraft((SelectCommand *)&selection);
                break;

            case 1:
                clWrapSpecialCloak(&universe.mainCommandLayer, (SelectCommand *)&selection, (AttackCommand *)targetSelection, FALSE);
                break;

            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selection, targetSelection);
                break;

            default:
                return result;
        }
        return selection.numShips;
    }
    else
    {
        sdword i;
        for (i = 0; i < selection.numShips; i++)
        {
            Ship *ship = selection.ShipPtr[i];
            SpaceObjRotImpTarg *defaultTarget = NULL;

            switch (specialType)
            {
                case 0:
                    defaultTarget = (SpaceObjRotImpTarg *)smFindNearestDefaultTargetForView(ship, 0);
                    break;
                case 1:
                    defaultTarget = (SpaceObjRotImpTarg *)smFindNearestDefaultTargetForView(ship, 1);
                    break;
                case 2:
                case 3:
                case 5:
                case 6:
                case 7:
                    defaultTarget = (SpaceObjRotImpTarg *)smFindNearestDefaultTargetForView(ship, 2);
                    break;
                case 4:
                    defaultTarget = (SpaceObjRotImpTarg *)smFindNearestDefaultTargetForView(ship, 3);
                    break;
                default:
                    break;
            }

            if (defaultTarget != NULL)
            {
                selectcom.numShips = 1;
                selectcom.timeLastStatus = 0.0f;
                selectcom.ShipPtr[0] = ship;
                attackcom.numTargets = 1;
                attackcom.timeLastStatus = 0.0f;
                attackcom.TargetPtr[0] = (TargetPtr)defaultTarget;

                switch (specialType)
                {
                    case 0:
                        clWrapSpecialCloak(&universe.mainCommandLayer, &selectcom, &attackcom, TRUE);
                        if (playSupportSpeech)
                            clPlaySpeechEventsForSupportCraft(&selectcom);
                        break;

                    case 1:
                        clWrapSpecialCloak(&universe.mainCommandLayer, &selectcom, &attackcom, FALSE);
                        break;

                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        clWrapSpecial(&universe.mainCommandLayer, &selectcom, (SelectAnyCommand *)&attackcom);
                        break;

                    default:
                        break;
                }
            }
            else if (!battleChatSent)
            {
                switch (specialType)
                {
                    case 0:
                        battleChatAddMessage(mrNoShipToSalvageFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 1:
                        battleChatAddMessage(mrNoShipToRepairFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 2:
                        battleChatAddMessage(mrNoShipToLeechFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 3:
                        battleChatAddMessage(mrNoShipToSwarmFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 4:
                        battleChatAddMessage(mrNoShipToEMPFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 5:
                        battleChatAddMessage(mrNoShipToInfectFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 6:
                        battleChatAddMessage(mrNoShipToRamFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                    case 7:
                    default:
                        battleChatAddMessage(mrNoShipToFireMissileAtFoundBattleChatMessage, 0xFF0000FF, 1, (sdword)ship);
                        break;
                }
                battleChatSent = TRUE;
            }

            result = i + 1;
        }
    }

    return result;
}


sdword __cdecl mrActivateSingleClickSpecialOnNearestFriendly(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 1);
}

sdword __cdecl mrLeechSelectionNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 2);
}

sdword __cdecl mrInfectSelectionNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 5);
}

sdword __cdecl mrFireMissilesAtNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 7);
}

sdword __cdecl mrRamSelectionNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 6);
}

sdword __cdecl mrRepulseSelectionNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 4);
}

sdword __cdecl mrSalvageSelectionNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 0);
}

sdword mrCancelExternalConstructionSelection(void)
{
    sdword i;
    MaxSelection moduleShips;
    MaxSelection selectcom;

    if (playPackets || universePause || mrDisabled)
    {
        return mrDisabled;
    }

    moduleShips.numShips = 0;
    for (i = 0; i < selSelected.numShips; i++)
    {
        Ship *ship = selSelected.ShipPtr[i];

        if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MIMIC) || !shipHasModuleSlotState(ship))
        {
            selSelectionAddSingleShipNoCheck(&moduleShips, ship);
        }
    }

    selSelectionCopyBounded(&selectcom, (MaxSelection *)&selSelected);
    MakeShipsNotIncludeTheseShips((SelectCommand *)&selectcom, (SelectCommand *)&moduleShips);
    MakeShipsNotDockedExceptSentinelLeechTypes((SelectCommand *)&selectcom);
    if (selectcom.numShips > 0)
    {
        clWrapExternalConstructionSelection(&universe.mainCommandLayer,
                                           (SelectCommand *)&selectcom,
                                           0,
                                           -1,
                                           -1,
                                           -1);
    }
    return selectcom.numShips;
}


sdword mrClearDeadShipReference(Ship *ship)
{
    Ship *result;

    result = mrTrackedShipReference;
    if (mrTrackedShipReference == ship)
    {
        mrTrackedShipReference = NULL;
    }
    return (sdword)result;
}


sdword mrCloakSelection(void)
{
    SelectCommand selection;
    sdword i;

    if (playPackets || universePause || mrDisabled)
    {
        return FALSE;
    }

    for (i = 0; i < selSelected.numShips; i++)
    {
        Ship *ship = selSelected.ShipPtr[i];

        if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CLOAKING) || univIsShipCloaked((SpaceObj *)ship))
        {
            continue;
        }
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        clWrapSpecial(&universe.mainCommandLayer, &selection, NULL);
    }

    return selSelected.numShips;
}


int mrCycleMoveWaypointType(void)
{
    int result = 0;

    if (mrMoveWaypointType)
    {
        if (mrMoveWaypointType == 1)
        {
            mrMoveWaypointType = 2;
            clCommandMessage(mrMoveWaypointEndToEndCommandMessage, -1);
        }
        else
        {
            result = mrMoveWaypointType - 2;
            if (mrMoveWaypointType == 2)
            {
                mrMoveWaypointType = 0;
                clCommandMessage(mrMoveWaypointLinearCommandMessage, -1);
            }
        }
    }
    else
    {
        mrMoveWaypointType = 1;
        clCommandMessage(mrMoveWaypointCircularCommandMessage, -1);
    }
    return result;
}



sdword __cdecl mrCycleSelectionFormation(sdword direction)
{
    TypeOfFormation formationType;
    sdword messageIndex;

    if (selSelected.numShips < 2)
    {
        return FALSE;
    }

    if (universeRealtimeElapsed <= mrCycleFormationExpire)
    {
        formationType = mrCycleFormationType;
    }
    else
    {
        formationType = clSelectionAlreadyInFormation(&universe.mainCommandLayer, (SelectCommand *)&selSelected);
    }

    if (formationType == NO_FORMATION || formationType == SAME_FORMATION)
    {
        formationType = DELTA_FORMATION;
    }
    else if (direction)
    {
        formationType = (formationType == DELTA_FORMATION) ? SPHERE_FORMATION : (formationType - 1);
    }
    else
    {
        formationType = (formationType + 1) % (SPHERE_FORMATION + 1);
    }

    mrNewFormation = formationType;
    mrDrawFormation = TRUE;
    mrDrawFormationTime = universe.totaltimeelapsed;
    mrCycleFormationExpire = universeRealtimeElapsed + 1.0f;
    mrCycleFormationType = formationType;

    switch (formationType)
    {
        case DELTA_FORMATION:
            messageIndex = str_GM_DeltaFormation;
            break;
        case BROAD_FORMATION:
            messageIndex = str_GM_BroadFormation;
            break;
        case DELTA3D_FORMATION:
            messageIndex = str_GM_Delta3DFormation;
            break;
        case CLAW_FORMATION:
            messageIndex = str_GM_ClawFormation;
            break;
        case WALL_FORMATION:
            messageIndex = str_GM_WallFormation;
            break;
        case SPHERE_FORMATION:
            messageIndex = str_GM_SphereFormation;
            break;
        case CUSTOM_FORMATION:
            messageIndex = str_GM_PicketFormation;
            break;
        default:
            soundEventPlay(0, UI_Click, 0);
            return FALSE;
    }

    clCommandMessage(strGetString(messageIndex), -1);
    return soundEventPlay(0, UI_Click, 0);
}


// Cataclysm-specific: active Sentinel forcefields unlink back to wall/sphere via the same
// retail command path used by the Force Field Unlink action.
sdword __cdecl mrCycleSelectionLeechMode(void)
{
    MaxAnySelection selection;
    sdword index;

    if (playPackets || universePause || mrDisabled)
    {
        return FALSE;
    }

    selSelectionCopy(&selection, (MaxAnySelection *)&selSelected);
    MakeShipsLeechCapable((SelectCommand *)&selection);
    for (index = 0; index < selection.numTargets; index++)
    {
        Ship *ship = (Ship *)selection.TargetPtr[index];
        CommandToDo *command = ship->command;

        if (command == NULL ||
            (command->ordertype.attributes & COMMAND_IS_FORMATION) == 0 ||
            command->selection == NULL ||
            command->selection->ShipPtr[0] != ship)
        {
            continue;
        }

        if (command->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
        {
            soundEventPlay(0, 264, 0);
            clWrapFormation(&universe.mainCommandLayer, command->selection, SPHERE_FORMATION);
        }
        else if (command->formation.formationtype == FORCEFIELD_WALL_FORMATION)
        {
            soundEventPlay(0, 264, 0);
            clWrapFormation(&universe.mainCommandLayer, command->selection, WALL_FORMATION);
        }
    }

    return selection.numTargets;
}


sdword __cdecl mrCycleSelectionTactics(sdword direction)
{
    sdword messageIndex;

    if (selSelected.numShips <= 0)
    {
        return selSelected.numShips;
    }

    if (universeRealtimeElapsed > mrCycleTacticsExpire)
    {
        mrCycleTacticsType = selSelected.ShipPtr[0]->tacticstype;
    }

    if (direction)
    {
        mrCycleTacticsType = (mrCycleTacticsType == Evasive) ? Aggressive : (mrCycleTacticsType - 1);
    }
    else
    {
        mrCycleTacticsType = (mrCycleTacticsType == Aggressive) ? Evasive : (mrCycleTacticsType + 1);
    }

    mrCycleTacticsExpire = universeRealtimeElapsed + 1.0f;
    switch (mrCycleTacticsType)
    {
        case Evasive:
            messageIndex = str_GM_TacticsEvasive;
            break;
        case Neutral:
            messageIndex = str_GM_TacticsNeutral;
            break;
        case Aggressive:
            messageIndex = str_GM_TacticsAggressive;
            break;
        default:
            return soundEventPlay(0, UI_Click, 0);
    }

    clCommandMessage(strGetString(messageIndex), -1);
    return soundEventPlay(0, UI_Click, 0);
}


sdword mrDecloakSelection(void)
{
    SelectCommand selection;
    sdword i;

    if (playPackets || universePause || mrDisabled)
    {
        return FALSE;
    }

    for (i = 0; i < selSelected.numShips; i++)
    {
        Ship *ship = selSelected.ShipPtr[i];

        if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CLOAKING) || !univIsShipCloaked((SpaceObj *)ship))
        {
            continue;
        }

        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        clWrapSpecial(&universe.mainCommandLayer, &selection, NULL);
    }

    return selSelected.numShips;
}

// Cataclysm-specific: issues a dock order against the clicked dock host.
sdword __cdecl mrDockSelectionToShip(Ship *targetShip)
{
   MaxSelection selection;

   selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
   MakeShipsDockCapableWithShipType((SelectCommand *)&selection, targetShip->shiptype, TRUE);
   if (selection.numShips > 0)
   {
       telldir();
       clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selection, 10, targetShip);
   }

   return selSelected.numShips;
}





sdword mrFireSelectionBigGun(void)
{
    sdword i;
    SelectCommand selectcom;

    if (playPackets || universePause || mrDisabled)
    {
        return mrDisabled;
    }

    for (i = 0; i < selSelected.numShips; i++)
    {
        Ship* ship = selSelected.ShipPtr[i];

        if (mothershipCanFireBigGun(ship))
        {
            selectcom.numShips = 1;
            selectcom.ShipPtr[0] = ship;
            clWrapSpecial(&universe.mainCommandLayer, &selectcom, 0);
        }
    }

    return selSelected.numShips;
}


sdword __cdecl mrIssueExternalConstructionSelection(Ship *targetShip)
{
    sdword result;
    sdword v2;
    ShipPtr *ShipPtr;
    Ship *v4;
    SelectCommand v5;
    SelectCommand v6;
    MaxSelection selectcom;

    result = selSelectionCopyBounded(&selectcom, (MaxSelection *)&selSelected);
    if (selectcom.numShips > 0)
    {
        if (targetShip->objtype)
        {
            v2 = 0;
            ShipPtr = selectcom.ShipPtr;
            do
            {
                v4 = *ShipPtr;
                if ((*ShipPtr)->shiptype == sMCV)
                {
                    v6.numShips = 1;
                    v6.ShipPtr[0] = v4;
                    clWrapExternalConstructionSelection(&universe.mainCommandLayer, &v6, 2, 0, 8, 3);
                }
                else
                {
                    v5.ShipPtr[0] = *ShipPtr;
                    v5.numShips = 1;
                    clWrapExternalConstructionSelection(&universe.mainCommandLayer, &v5, 2, 0, 8, 1);
                }
                result = selectcom.numShips;
                ++v2;
                ++ShipPtr;
            }
            while (v2 < selectcom.numShips);
        }
        else
        {
            return (sdword)clWrapExternalConstructionSelection(
                             &universe.mainCommandLayer,
                             (SelectCommand *)&selectcom,
                             1,
                             targetShip->shiptype,
                             targetShip->playerowner->playerIndex,
                             0);
        }
    }
    return result;
}


void mrLaunchSelectionShips(void)
{
    MaxSelection dest;

    if (!playPackets && !universePause && !mrDisabled)
    {
        selSelectionCopyBounded(&dest, (MaxSelection *)&selSelected);
        MakeShipsOnlyHaveUpgrade(&dest.numShips, 16);
        MakeShipsNotDockedExceptSentinelLeechTypes(&dest.numShips);
        if (dest.numShips > 0)
            clWrapLaunchShipSelection(&universe.mainCommandLayer, (SelectCommand *)&dest);
    }
}



sdword mrLaunchSelectionSwarmers(void)
{
    SelectCommand selectcom;
    sdword i;

    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }

    for (i = 0; i < selSelected.numShips; i++)
    {
        Ship *ship = selSelected.ShipPtr[i];

        if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY) || !shipHasReadySwarmers(ship))
        {
            continue;
        }

        selectcom.numShips = 1;
        selectcom.ShipPtr[0] = ship;
        clWrapSpecial(&universe.mainCommandLayer, &selectcom, NULL);
    }

    return selSelected.numShips;
}

sdword __cdecl mrLaunchSwarmersNearestTarget(SelectAnyCommand *targetSelection)
{
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    return mrIssueSelectionSingleClickSpecial(targetSelection, 3);
}

// Cataclysm-specific: trigger Acolyte/Sentinel link action for currently selected ships.
sdword __cdecl mrLinkSelection(void)
{
    MaxSelection dest;
    sdword i;

    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }

    selSelectionCopyBounded(&dest, (MaxSelection *)&selSelected);
    for (i = 0; i < dest.numShips;)
    {
        if (!ShipHasUpgrade(dest.ShipPtr[i], RESEARCH_UPGRADE_MASK_LINK_PRIMARY))
        {
            dest.numShips--;
            dest.ShipPtr[i] = dest.ShipPtr[dest.numShips];
            continue;
        }
        i++;
    }
    MakeShipsNotDockedExceptSentinelLeechTypes((SelectCommand *)&dest);
    if (dest.numShips > 0)
    {
        clWrapLinkSelection(&universe.mainCommandLayer, (SelectCommand *)&dest);
    }

    return dest.numShips;
}
// Cataclysm-specific: Sentinel forcefield creation uses the protect command plus the
// retail-only forcefield formations 9 and 10.
sdword __cdecl mrProtectSelection(SelectCommand *targets)
{
    Ship *targetShip;
    ShipType shiprace;
    Node *head;
    MaxSelection *existingForcefield;
    CommandToDo *command;
    sdword formationtype;
    SelectCommand *commandSel;
    sdword v9;
    Ship **v10;
    sdword targetIndex;
    MaxSelection selection;
    MaxSelection selectcom;
    ProtectCommand protectcom;

    if (playPackets || universePause || mrDisabled)
        return FALSE;
    if (tutorial == 1 && !tutEnable.bFormation)
        return FALSE;

    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    MakeShipsLeechCapable((SelectCommand *)&selection);
    if (selection.numShips <= 0)
        return selection.numShips;

    if (targets)
    {
        targetIndex = 0;
        while (targetIndex < targets->numShips)
        {
            targetShip = targets->ShipPtr[targetIndex];
            shiprace = targetShip->shiprace;
            if (shiprace != 17 && shiprace != 52)
            {
                head = universe.mainCommandLayer.todolist.head;
                existingForcefield = NULL;
                protectcom.numShips = 1;
                protectcom.ShipPtr[0] = targetShip;
                if (head)
                {
                    do
                    {
                        command = head->structptr;
                        if ((command->ordertype.attributes & COMMAND_IS_PROTECTING) &&
                            ShipInSelection(command->protect, targetShip) &&
                            (command->ordertype.attributes & COMMAND_IS_FORMATION))
                        {
                            formationtype = command->formation.formationtype;
                            if (formationtype == SPHERE_FORMATION)
                            {
                                commandSel = command->selection;
                                if (commandSel->numShips > 0)
                                    clWrapFormation(&universe.mainCommandLayer, commandSel, DELTA_FORMATION);
                            }
                            else if (formationtype == FORCEFIELD_SPHERE_FORMATION)
                            {
                                existingForcefield = (MaxSelection *)command->selection;
                            }
                        }
                        head = head->next;
                    }
                    while (head);
                }
                if (existingForcefield)
                    selSelectionCopyBounded(&selectcom, existingForcefield);
                else
                    selectcom.numShips = 0;

                v9 = 0;
                if (selection.numShips > 0)
                {
                    v10 = selection.ShipPtr;
                    do
                    {
                        if (selectcom.numShips >= 12)
                            break;
                        if (!ShipInSelection((SelectCommand *)&selectcom, *v10))
                            selSelectionAddSingleShipNoCheck(&selectcom, *v10);
                        ++v9;
                        ++v10;
                    }
                    while (v9 < selection.numShips);
                }
                MakeShipsNotIncludeTheseShips((SelectCommand *)&selection, (SelectCommand *)&selectcom);
                soundEventPlay(0, 264, 0);
                if (selectcom.numShips <= 2)
                {
                    speechEventQueue(
                        selectcom.ShipPtr[0],
                        67110928,
                        selectcom.ShipPtr[0]->shiprace,
                        -1, -1, -1, -1, 0.0f, -1);
                }
                else
                {
                    speechEventQueue(
                        selectcom.ShipPtr[1],
                        67111055,
                        selectcom.ShipPtr[1]->shiprace,
                        -1, -1, -1, -1, 0.0f, -1);
                    clWrapProtect(&universe.mainCommandLayer, (SelectCommand *)&selectcom, &protectcom);
                    clWrapFormation(&universe.mainCommandLayer, (SelectCommand *)&selectcom, FORCEFIELD_SPHERE_FORMATION);
                    clCommandMessage(mrProtectChangeToSphereFormationCommandMessage, -1);
                }
            }
            if (selection.numShips <= 0)
                break;
            ++targetIndex;
        }
    }
    else
    {
        soundEventPlay(0, 264, 0);
        if (selection.numShips <= 2)
        {
            return speechEventQueue(
                selection.ShipPtr[0],
                67110928,
                selection.ShipPtr[0]->shiprace,
                -1, -1, -1, -1, 0.0f, -1);
        }
        else
        {
            speechEventQueue(
                selection.ShipPtr[1],
                67111055,
                selection.ShipPtr[1]->shiprace,
                -1, -1, -1, -1, 0.0f, -1);
            clWrapFormation(&universe.mainCommandLayer, (SelectCommand *)&selection, FORCEFIELD_WALL_FORMATION);
            return clCommandMessage(mrProtectChangeToForcefieldWallCommandMessage, -1);
        }
    }

    return selection.numShips;
}


void mrRemoveLastMoveWaypoint(void)
{
    if (--mrMoveWaypointCount < 0)
        mrMoveWaypointCount = 0;
}



sdword __cdecl mrResetBandboxAndRestorePathMarkers(void)
{
    sdword savedPathMarkersEnabled = 0;

    if (smBandboxEnabled)
    {
        selSelecting.numTargets = 0;
        smBandboxEnabled = 0;
    }
    if (mrRightMouseCameraMode)
    {
        mrRightMouseCameraMode = 0;
        savedPathMarkersEnabled = mrSavedPathMarkersEnabled;
    }
    smPathMarkersEnabled = mrSavedPathMarkersEnabled;
    return savedPathMarkersEnabled;
}


int __cdecl mrSetMoveWaypointType(int waypointType)
{
    int result = 0;

    if (waypointType)
    {
        if (waypointType == 1)
        {
            mrMoveWaypointType = 1;
            clCommandMessage(mrMoveWaypointCircularCommandMessage, -1);
        }
        else
        {
            result = waypointType - 2;
            if (waypointType == 2)
            {
                mrMoveWaypointType = 2;
                clCommandMessage(mrMoveWaypointEndToEndCommandMessage, -1);
            }
        }
    }
    else
    {
        mrMoveWaypointType = 0;
        clCommandMessage(mrMoveWaypointLinearCommandMessage, -1);
    }
    return result;
}

#define MR_MOVE_WAYPOINT_STORE_MAX 19

static void mrMoveWaypointPointSpecified(vector *destination)
{
    sdword pointIndex;

    if (mrMoveWaypointCount < MR_MOVE_WAYPOINT_STORE_MAX)
    {
        mrMoveWaypointDestinations[mrMoveWaypointCount] = *destination;
        mrMoveWaypointCount++;
        soundEventPlay(0, 280, 0);
    }
    else
    {
        mrMoveWaypointOverflowDestination = *destination;
    }

    pointIndex = mrMoveWaypointCount - 1;
    if (autoFocusOnWaypoints && pointIndex >= 0 && smSensorsActive)
    {
        smCameraLookatPoint = mrMoveWaypointDestinations[pointIndex];
    }
}

void mrToggleMoveWaypoints(void)
{
    vector dist;
    sdword pathState;
    MoveCommandPacked packedMove;

    if (mrMoveWaypointsFlags[0])
    {
        if (tutorial != TUTORIAL_ONLY || ((*(udword *)&tutEnable & 0x1000) != 0))
        {
            if (selSelected.numShips > 0 && mrMoveWaypointCount > 0)
            {
                MakeShipsMoveCapable((int *)&selSelected, 1);
                if (selSelected.numShips > 0)
                {
                    if (mrMoveWaypointType == MR_MoveWaypointTypeCircular)
                    {
                        pathState = 1;
                    }
                    else if (mrMoveWaypointType == MR_MoveWaypointTypeEndToEnd)
                    {
                        pathState = 2;
                    }
                    else
                    {
                        pathState = 0;
                    }

                    packedMove = clBuildMovePathCommand(pathState, selCentrePoint, mrMoveWaypointCount, mrMoveWaypointDestinations);
                    clWrapMove(&universe.mainCommandLayer, (SelectCommand *)&selSelected, packedMove);
                    speechEventQueue(selSelected.ShipPtr[0], 0x04000801, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
                    soundEventPlay(0, 280, 0);
                    if (tutorial == TUTORIAL_ONLY)
                    {
                        if (mrMoveWaypointType == MR_MoveWaypointTypeCircular)
                        {
                            tutGameMessage("Waypointcircul");
                        }
                        else if (mrMoveWaypointType == MR_MoveWaypointTypeEndToEnd)
                        {
                            tutGameMessage("Waypointendtoe");
                        }
                        else
                        {
                            tutGameMessage("Waypointnormal");
                        }
                    }
                }
            }

            if (piePointSpecMode != PSM_Idle)
            {
                piePointModeOnOff();
            }
        }
    }
    else if (tutorial != TUTORIAL_ONLY || ((*(udword *)&tutEnable & 0x800) != 0))
    {
        if (piePointSpecMode != PSM_Idle)
        {
            piePointModeOnOff();
        }

        MakeShipsMoveCapable((int *)&selSelected, 1);
        if (!mrRightMouseCameraMode && !smBandboxEnabled && selSelected.numShips >= 1)
        {
            selCentrePointCompute();
            if (piePointSpecMode == PSM_Idle)
            {
                if (mrRenderMainScreen)
                {
                    vecSub(dist, selCentrePoint, universe.mainCameraCommand.actualcamera.lookatpoint);
                    if (vecMagnitudeSquared(dist) > MAX_MOVE_DISTANCE)
                    {
                        smSensorsBegin(NULL, NULL);
                    }
                }

                piePointModeOnOffSpecified(PiePointSpecTypeWaypoints, mrMoveWaypointPointSpecified);
                mrMoveWaypointCount = 0;
                mrMoveWaypointsFlags[0] = 1;
            }
        }
    }
}



sdword __cdecl mrSetSelectionFormation(int formationType, sdword showStatusMessage, int suppressSpeech)
{
    char msgName[128];
    sdword messageIndex;
    sdword speechIndex;

    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    if (mrDisabled)
    {
        return mrDisabled;
    }
    if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bFormation)
    {
        return FALSE;
    }

    MakeShipsNotDocked((SelectCommand *)&selSelected);
    if (selSelected.numShips <= 0)
    {
        return 0;
    }

    strcpy(msgName, "Game_Formation_");
    if (formationType == SAME_FORMATION)
    {
        if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) == -1)
        {
            return clCommandMessage(strGetString(str_GM_NeedMothershipOrCarrierForParadeFormation), -1);
        }

        soundEventPlay(0, UI_Click, 0);
        speechEventQueue(NULL, 67113058, 8, -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
        clWrapSetMilitaryParade(&universe.mainCommandLayer, (SelectCommand *)&selSelected);
        strcat(msgName, strGetString(strPARADE_FORMATION));
        tutGameMessage(msgName);
        if (showStatusMessage)
        {
            clCommandMessage(strGetString(str_GM_ChangeToMilitaryParade), -1);
        }
        return showStatusMessage;
    }

    soundEventPlay(0, UI_Click, 0);
    if (suppressSpeech)
    {
        speechEventQueue(selSelected.ShipPtr[0], 67111056, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
    }
    else
    {
        speechIndex = (formationType == CUSTOM_FORMATION) ? 7 : formationType;
        speechEventQueue(NULL, 67113058, speechIndex, -1, 0, selSelected.ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
    }

    clWrapFormation(&universe.mainCommandLayer, (SelectCommand *)&selSelected, formationType);

    if (FORMATION_IS_VALID(formationType))
    {
        strcat(msgName, TypeOfFormationToStr(formationType));
        tutGameMessage(msgName);
    }

    if (!showStatusMessage)
    {
        return showStatusMessage;
    }

    switch (formationType)
    {
        case DELTA_FORMATION:
            messageIndex = str_GM_ChangeToDeltaFormation;
            break;
        case BROAD_FORMATION:
            messageIndex = str_GM_ChangeToBroadFormation;
            break;
        case DELTA3D_FORMATION:
            messageIndex = str_GM_ChangeToDelta3DFormation;
            break;
        case CLAW_FORMATION:
            messageIndex = str_GM_ChangeToClawFormation;
            break;
        case WALL_FORMATION:
            messageIndex = str_GM_ChangeToWallFormation;
            break;
        case SPHERE_FORMATION:
            messageIndex = str_GM_ChangeToSphereFormation;
            break;
        case CUSTOM_FORMATION:
            messageIndex = str_GM_ChangeToPicketFormation;
            break;
        default:
            return showStatusMessage;
    }

    clCommandMessage(strGetString(messageIndex), -1);
    return showStatusMessage;
}


sdword __cdecl mrSetSelectionLeechMode(unsigned __int8 leechMode)
{
    MaxSelection selectcom;

    selSelectionCopyBounded(&selectcom, (MaxSelection *)&selSelected);
    if (!playPackets && !universePause && !mrDisabled)
    {
        MakeShipsInfectionCapable(&selectcom.numShips, 1);
        MakeShipsNotDockedExceptSentinelLeechTypes(&selectcom.numShips);
        if (selectcom.numShips > 0)
        {
            clWrapSetLeech(&universe.mainCommandLayer, (SelectCommand *)&selectcom, leechMode);
        }
        telldir();
        return selectcom.numShips;
    }
    if (playPackets)
    {
        return playPackets;
    }
    if (universePause)
    {
        return universePause;
    }
    return mrDisabled;
}

int __cdecl mrSetSelectionTactics(int tacticsType, int showStatusMessage)
{
    sdword messageIndex;
    MaxSelection selectcom;

    if (playPackets || universePause || mrDisabled)
    {
        return FALSE;
    }

    selSelectionCopyBounded(&selectcom, (MaxSelection *)&selSelected);
    MakeShipsNotDockedExceptSentinelLeechTypes(&selectcom.numShips);
    if (tacticsType == Evasive)
    {
        MakeShipsNonCapital((SelectCommand *)&selectcom);
    }

    if (selectcom.numShips <= 0)
    {
        return FALSE;
    }

    switch (tacticsType)
    {
        case Evasive:
            speechEventQueue(selSelected.ShipPtr[0], 67110925, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
            tutGameMessage("KB_TacticsEvasive");
            messageIndex = str_GM_TacticsSetToEvasive;
            break;
        case Neutral:
            speechEventQueue(selSelected.ShipPtr[0], 67110926, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
            tutGameMessage("KB_TacticsNeutral");
            messageIndex = str_GM_TacticsSetToNeutral;
            break;
        case Aggressive:
            speechEventQueue(selSelected.ShipPtr[0], 67110927, selSelected.ShipPtr[0]->shiptype, -1, -1, -1, -1, 0.0f, -1);
            tutGameMessage("KB_TacticsAggressive");
            messageIndex = str_GM_TacticsSetToAggressive;
            break;
        default:
            return FALSE;
    }

    clWrapSetTactics(&universe.mainCommandLayer, (SelectCommand *)&selectcom, tacticsType);
    if (showStatusMessage)
    {
        clCommandMessage(strGetString(messageIndex), -1);
    }

    return TRUE;
}


#if 0  // Cat3: mrToggleMoveWaypoints (0x565070) — complex UI function with waypoint mode; uses dword_B39CBC/dword_B39CB0/dword_A752B4/qword_B39BC0/p2; needs live waypoint state structs (0x565070)
// local variable allocation has failed, the output may be wrong!
void __thiscall mrToggleMoveWaypoints(void *this)
{
  int v1; // eax
  int v2; // [esp-10h] [ebp-1Ch]
  vector v3; // [esp-Ch] [ebp-18h] OVERLAPPED
  float v4; // [esp+0h] [ebp-Ch]
  unsigned __int64 v5; // [esp+0h] [ebp-Ch] OVERLAPPED
  float v6; // [esp+4h] [ebp-8h]
  float v7; // [esp+8h] [ebp-4h]

  if ( dword_B39CBC[0] )
  {
    if ( tutorial != 1 || (tutEnable[0] & 0x1000) != 0 )
    {
      if ( selSelected.numShips > 0 && dword_B39CB0 > 0 )
      {
        MakeShipsMoveCapable(&selSelected.numShips, 1);
        if ( dword_A752B4 )
        {
          if ( dword_A752B4 == 1 )
            v1 = 1;
          else
            v1 = dword_A752B4 == 2 ? 2 : (int)this;
        }
        else
        {
          v1 = 0;
        }
        v5 = __PAIR64__(&qword_B39BC0, dword_B39CB0);
        v3 = p2;
        v2 = v1;
        clWrapMovePoint(&universe.mainCommandLayer, &selSelected, *(vector *)((char *)&v3 - 4), *(vector *)((char *)&v5 - 4));
        speechEventQueue(
          selSelected.ShipPtr[0],
          67110913,
          selSelected.ShipPtr[0]->tactics_ordertype,
          -1,
          -1,
          -1,
          -1,
          0.0,
          -1);
        soundEventPlay(0, 280, 0);
        if ( tutorial == 1 )
        {
          if ( dword_A752B4 )
          {
            if ( dword_A752B4 == 1 )
            {
              tutGameMessage(aWaypointcircul);
            }
            else if ( dword_A752B4 == 2 )
            {
              tutGameMessage(aWaypointendtoe);
            }
          }
          else
          {
            tutGameMessage(aWaypointnormal);
          }
        }
      }
      if ( piePointSpecMode )
        piePointModeOnOff();
    }
  }
  else if ( tutorial != 1 || (tutEnable[0] & 0x800) != 0 )
  {
    if ( piePointSpecMode )
      piePointModeOnOff();
    MakeShipsMoveCapable(&selSelected.numShips, 1);
    if ( !mrRightMouseCameraMode && !smBandboxEnabled && selSelected.numShips >= 1 )
    {
      selCentrePointCompute();
      if ( !piePointSpecMode )
      {
        if ( mrRenderMainScreen )
        {
          v6 = p2.y - universe.mainCameraCommand.actualcamera.lookatpoint.y;
          v7 = p2.z - universe.mainCameraCommand.actualcamera.lookatpoint.z;
          v4 = p2.x - universe.mainCameraCommand.actualcamera.lookatpoint.x;
          if ( (double)(unsigned int)dword_8AC4F8 < v7 * v7 + v6 * v6 + v4 * v4 )
            smSensorsBegin();
        }
        piePointModeOnOff();
        dword_B39CB0 = 0;
        dword_B39CBC[0] = 1;
      }
    }
  }
}
#endif


sdword mrUpgradeOrDockSelection(void)
{
  sdword i;
  MaxSelection dockSelection;
  SelectCommand upgradeSelection;

  if (playPackets || universePause || mrDisabled)
  {
      return 0;
  }

  dockSelection.numShips = 0;
  upgradeSelection.numShips = 0;
  if (selSelected.numShips <= 0)
  {
      return 0;
  }

  for (i = 0; i < selSelected.numShips; i++)
  {
      Ship *ship = selSelected.ShipPtr[i];

      if (shipHasCataBusyState(ship))
      {
          continue;
      }
      if (shipHasAvailableUpgradeChanges(ship))
      {
          if (ship->staticinfo->UpgradeByDocking)
          {
              selSelectionAddSingleShipNoCheck(&dockSelection, ship);
          }
          else
          {
              selSelectionAddSingleShipNoCheck((MaxSelection *)&upgradeSelection, ship);
          }
      }
    }

  if (upgradeSelection.numShips > 0)
  {
      clWrapUpgrade(&universe.mainCommandLayer, &upgradeSelection, 0);
  }
  if (dockSelection.numShips > 0)
  {
      battleChatAddMessage(strGetString(strUpgradingByDockingNotification), -1, 1, (int)dockSelection.ShipPtr[0]);
      clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&dockSelection, 0, 0);
  }

  return dockSelection.numShips;
}

void mrUpgrade(char *string, featom *atom)
{
  if (mrUpgradeSelection() != 0)
  {
      soundEvent(NULL, UI_Click);
  }
  else
  {
      soundEvent(NULL, UI_ClickCancel);
  }
}
