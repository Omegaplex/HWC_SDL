/*=============================================================================
    Name    : mainrgn.h
    Purpose : Logic for handling region events for the main game screen.

    Created 6/27/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MAINRGN_H
#define ___MAINRGN_H

#include "types.h"
#include "region.h"
#include "feflow.h"
#include "Camera.h"
#include "spaceobj.h"
#include "shipselect.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define MR_GUI_SINGLECLICK          1

#ifndef HW_Release

#define MR_ERROR_CHECKING           1           //general error checking
#define MR_VERBOSE_LEVEL            2           //control specific output code
#define MR_TEST_HPB                 0           //test heading/pitch/bank interactively
#define MR_RELEASE_MOUSE            1           //allow the mouse to be freed from the window
#define MR_TEST_GUNS                1           //allow the game testing mode
#define MR_SCREENSHOTS              0           //enable screen shots
#define MR_SOUND_RELOAD_VOLUMES     0           //permits reloading of volume tables
#define MR_CAN_FOCUS_ROIDS          1           //can focus on asteroids,dust clouds and derelicts with an alt-click
#define MR_KEYBOARD_CHEATS          1           //enable typing in cheats on the keyboard

#else //HW_Debug

#define MR_ERROR_CHECKING           0           //general error checking
#define MR_VERBOSE_LEVEL            0           //control specific output code
#define MR_TEST_PING                0           //test ping time
#define MR_TEST_HPB                 0           //test heading/pitch/bank interactively
#define MR_RELEASE_MOUSE            0           //allow the mouse to be freed from the window
#define MR_TEST_GUNS                0           //allow the game testing mode
#define MR_SCREENSHOTS              0           //enable screen shots
#define MR_SOUND_RELOAD_VOLUMES     0           //permits reloading of volume tables
#define MR_CAN_FOCUS_ROIDS          0           //can focus on asteroids,dust clouds and derelicts with an alt-click
#define MR_KEYBOARD_CHEATS          0           //enable typing in cheats on the keyboard

// special case stuff so that cheats are enabled for release for bryce and drew
#ifndef HW_Release
    #ifdef bpasechnik
        #undef  MR_KEYBOARD_CHEATS
        #define MR_KEYBOARD_CHEATS      1           //enable typing in cheats on the keyboard
    #endif

    #ifdef ddunlop
        #undef  MR_KEYBOARD_CHEATS
        #define MR_KEYBOARD_CHEATS      1           //enable typing in cheats on the keyboard
    #endif
#endif

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//message display definitions
#define MR_MESSAGE_LINE_SPACING     12
#define MR_MESSAGE_LINE_START       400
#define MR_MouseMovementClickLimit  12
#define MR_FormationDelay           1.0f
#define MR_NumberDoublePressTime    0.5
#define MR_FastMoveShipClickTime    0.5
#define MR_FastMoveShipClickX       5
#define MR_FastMoveShipClickY       5

#define MR_LetterboxGrey            0.5f, 0.5f, 0.5f

/*=============================================================================
    Data:
=============================================================================*/
//data exported for the benefit of other modules with similar functionality
extern bool32 mrWhiteOut;
extern real32 mrWhiteOutT;
extern real32 cat_mrBlackOut;
extern real32 cat_mrWhiteOutT;
extern sdword mrRenderMainScreen;
extern sdword mrOldMouseX, mrOldMouseY;
extern sdword mrMouseHasMoved;
extern sdword mrMouseMovementClickLimit;
extern sdword mrRightMouseCameraMode;
extern sdword smBandboxEnabled;
extern MaxAnySelection attackcom;

//extern ShipPtr mrLastClosestShip;
extern real32 MAX_INGAME_MOVECOMMAND_DISTANCE;
extern real32 mrClosestDistance;

//camera rendering in the main region
extern Camera *mrCamera;

extern void (*mrHoldLeft)(void);
extern void (*mrHoldRight)(void);

extern char *mrMenuItemByTactic[];

extern bool32 helpinfoactive;

extern real32 mrNumberDoublePressTime;
extern sdword mrLastKeyPressed;
extern real32 mrLastKeyTime;

extern bool32 mrDisabled;
typedef enum
{
    MR_MoveWaypointTypeLinear = 0,
    MR_MoveWaypointTypeCircular = 1,
    MR_MoveWaypointTypeEndToEnd = 2,
}
mrMoveWaypointMode;
extern sdword mrMoveWaypointType;
sdword mrCancelExternalConstructionSelection(void);
void mrToggleMoveWaypoints(void);

#if MR_CAN_FOCUS_ROIDS
bool32 mrCanFocusRoids;
#endif

/*=============================================================================
    Functions:
=============================================================================*/
//startup/shutdown of main region code
void mrStartup(void);
void mrShutdown(void);
void mrReset(void);
int mrUpdateMouseDrag(void);
void mrHandleEscape(void);
sdword tabStatsToggleDisplayOption(char keycode);

void mrShipDied(struct Ship *ship);

//region-process routine for the main region
udword mrRegionProcess(struct tagRegion *reg, sdword ID, udword event, udword data);
void mrMoveSelection(void);
void mrMoveSelectionPointSpecified(vector *destination);
//region-draw routine for the main region
void mrRegionDraw(regionhandle reg);

//right-click callback functions
void mrDockingOrders(char *string, featom *atom);
void mrUpgrade(char *string, featom *atom);
void mrDeltaFormation(char *string, featom *atom);
void mrBroadFormation(char *string, featom *atom);
void mrXFormation(char *string, featom *atom);
void mrClawFormation(char *string, featom *atom);
void mrWallFormation(char *string, featom *atom);
void mrSphereFormation(char *string, featom *atom);
void mrPicketFormation(char *string, featom *atom);
void mrHarvestResources(char *string, featom *atom);
void mrHarvestResourcesTargetResource(struct Resource *resource);
void mrBuildShips(char *string, featom *atom);
void mrTradeStuff(char *string, featom *atom);
void mrMoveShips(char *string, featom *atom);
void mrInfo(char *string, featom *atom);
void mrCancel(char *string, featom *atom);
void mrScuttle(char *string, featom *atom);
void mrRetire(char *string, featom *atom);
void mrUpdateHyperspaceStatus(bool32 goForLaunch);
void mrHyperspace(char *string, featom *atom);
void mrLaunch(char *string, featom *atom);
void mrResearch(char *string, featom *atom);
void mrEvasiveTactics(char *string, featom *atom);
void mrNeutralTactics(char *string, featom *atom);
void mrAgressiveTactics(char *string, featom *atom);
int __cdecl mrSetSelectionTactics(int tacticsType, int showStatusMessage);
void mrFormAlliance(char *string, featom *atom);
void mrBreakAlliance(char *string, featom *atom);
void mrTransferRUS(char *string, featom *atom);
udword mrActivateSelectionSpecial(void);
sdword __cdecl mrActivateSingleClickSpecialOnNearestFriendly(SelectAnyCommand *targetSelection);
sdword mrCloakSelection(void);
sdword mrDecloakSelection(void);
sdword __cdecl mrCycleSelectionLeechMode(void);
int mrCycleMoveWaypointType(void);
int __cdecl mrSetMoveWaypointType(int waypointType);
sdword __cdecl mrFireMissilesAtNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrInfectSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrLeechSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrIssueExternalConstructionSelection(struct Ship *targetShip);
sdword __cdecl mrLinkSelection(void);
void mrLaunchSelectionShips(void);
void mrRemoveLastMoveWaypoint(void);
sdword mrUnleechSelection(void);
sdword __cdecl mrSetSelectionLeechMode(unsigned __int8 leechMode);
void *mrSelectMothership(void);
sdword mrFireSelectionBigGun(void);
sdword __cdecl mrProtectSelection(SelectCommand *targets);
sdword __cdecl mrSetSelectionFormation(int formationType, sdword showStatusMessage, int suppressSpeech);
sdword mrLaunchSelectionSwarmers(void);
sdword __cdecl mrLaunchSwarmersNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrEnterNearestSlipgate(void);
sdword __cdecl mrRepulseSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrRamSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword __cdecl mrSalvageSelectionNearestTarget(SelectAnyCommand *targetSelection);
sdword mrBuildProtectTargetSelection(MaxSelection *dest, SelectAnyCommand *targets);
bool32 mrSelectionCanProtectTarget(SelectCommand *selection, SpaceObjRotImpTarg *target);
sdword mrDetermineTargetRightClickCursorType(sdword *selectionActions, sdword *targetActions, Ship *ship);
sdword mrDetermineCursorType(udword *keyState, udword *mouseState, sdword *selectedShips, Ship *ship);

//misc functions
void mrCameraMotion(void);
bool32 mrHandleBandboxPressLeft(void);
void mrHandleBandboxPressRight(rectangle *playerRect);
void mrHandleBandboxHoldLeft(sdword clickNode);
void mrHandleBandboxReleaseLeft(sdword clickNode);
void smDrawBandbox(bool32 bOverlayOnly);
void smViewportSelectLeftRelease(sdword clickNode);
void mrPlayerNameDraw(sdword playerIndex, sdword x, sdword y);
void mrCommandMessageDraw(void);
void mrSelectRectBuild(rectangle *dest, sdword anchorX, sdword anchorY);
sdword mrCursorText(SpaceObj *cursorobj);
sdword *__cdecl mrDetermineSelectionActions(sdword *result, SelectAnyCommand *selection);
void mrEnable(void);
void mrDisable(void);
void mrNULL(void);
void mrSelectHold(Camera *camera, sdword clickNode);
void mrDrawTabStats(Ship *ship);
void mrShipDied(Ship *ship);
void mrTacticalOverlayState(bool32 bTactical);
bool32 IsTabStatsEnabled(void);

void bigmessageDisplay(char *msg,sdword position);
void bigmessageErase(sdword position);

//probe hack functions
bool32 mrNeedProbeHack(void);
void mrProbeHack();
void mrRemoveAllProbesFromSelection();

//nis strangeness prevention:
void mrNISStarting(void);
void mrNISStopping(void);

#endif //___MAINRGN_H
