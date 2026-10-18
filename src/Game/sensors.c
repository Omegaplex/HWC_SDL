/*=============================================================================
	Name    : Sensors.c
	Purpose : Code for handling and rendering the sensors manager.

	Created 10/8/1997 by lmoloney
	Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "glinc.h"
#include "types.h"
#include "debug.h"
#include "memory.h"
#include "prim2d.h"
#include "prim3d.h"
#include "mainrgn.h"
#include "pieplate.h"
#include "utility.h"
#include "font.h"
#include "region.h"
#include "feflow.h"
#include "camera.h"
#include "cameracommand.h"
#include "vector.h"

#include "vector.h"
#include "universe.h"
#include "univupdate.h"
#include "physics.h"

#include "render.h"
#include "mouse.h"
#include "options.h"
#include "select.h"
#include "shipselect.h"
#include "SalCapCorvette.h"
#include "commandwrap.h"
#include "teams.h"
#include "taskbar.h"
#include "soundevent.h"
#include "blobs.h"
#include "light.h"
#include "fastmath.h"
#include "nis.h"

#include "sensors.h"
#include "proximitysensor.h"
#include "singleplayer.h"
#include "probe.h"
#include "strings.h"
#include "mainrgn.h"
#include "main.h"
#include "KeyBindings.h"
#include "InfoOverlay.h"

#define COMM_MoveSelectionPointSpecified 0x04000801
#include "alliance.h"
#include "statscript.h"
#include "ping.h"
#include "tactical.h"
#include "teams.h"
#include "tutor.h"
#include "multiplayergame.h"
#include "bounties.h"
#include "btg.h"
#include "glcaps.h"
#include "subtitle.h"
#include "SaveGame.h"
#include "randy.h"
#include "transformer.h"
#include "battle.h"
#include "BattleChat.h"
#include "gravwellgenerator.h"
#include "shader.h"
#include "devstats.h"
#include "ACV.h"

#include "BigGun.h"
#include "linkedlist.h"

#include "rglu.h"
#include "GameChat.h"

//located in mainrg.c
//falko's fault...not mine..long story
void toFieldSphereDraw(ShipPtr ship, real32 radius, real32 scale);
sdword __cdecl univIsObjFriendly(SpaceObj* obj, Player* player);
bool32 shipHasReadySwarmers(Ship* ship);
sdword shipGetSlotStatus(Ship* ship, sdword slot);
bool32 shipIsBeastShip(Ship* ship);
void CubicLogUnsigned(real32* pos, real32* speed, real32 targ, real32 threshold, real32 step, real32 logBase);
void telldir(void);
bool32 singlePlayerHyperspaceObjectiveActive(void);
bool32 ccFocusIsLegal(SpaceObj* obj);
int kbFormatCommandBindings(int command, char* dest);
void selSelectionAddSingleShipNoCheck(MaxSelection* dest, Ship* ship);
void mrFormationShips(MaxSelection* selection, Ship* ship);
CommandToDo* getShipAndItsCommand(CommandLayer* comlayer, ShipPtr ship);
bool32 etgEffectUpdate(struct Effect* effect, real32 timeElapsed);
void etgEffectDraw(struct Effect* effect);
sdword shipNamesCountFreeByType(ShipType shipType);
sdword shipNameReserveByIndex(Ship* ship, sdword freeIndex);
char* shipNamesGetFreeByTypeIndex(ShipType shipType, sdword freeIndex);
char* shipNameGet(Ship* ship);
bool32 shipCanHaveName(Ship* ship);
real32* __cdecl fogCreateGrid(real32* minimums, real32* maximums);
void fogResetGrid(void);
sdword __cdecl fogAllocRenderState(sdword initializeDirtyFlags);
sdword fogDrawGrid(void);
sdword fogRebuildDirtyRenderLists(void);
void __cdecl fogRebuildDirtyRenderList(sdword dirtyListBase, sdword* dirtyCount, sdword renderStateBase, sdword renderMode, LinkedList* list);
sdword fogDrawRenderLists(void);
sdword __cdecl fogDrawRenderList(LinkedList* list, color* palette);
sdword __cdecl fogAddRenderLine(real32* startPosition, real32* endPosition);
sdword __cdecl fogAddCellCornerLines(real32* corner, sdword cornerIndex);
sdword __cdecl fogGetGridCellOccupancy(sdword x, sdword y, sdword z);
sdword __cdecl cat_DrawUniverseBoxEdge(vector* from, vector* to, color lineColor);
sdword cat_DrawUniverseBox(void);
bool32 __cdecl cat_smRenderShipAndSlaves(SpaceObj* spaceobj);
void __cdecl cat_smRenderShips(hmatrix* modelView, hmatrix* projection);
sdword __cdecl cat_smGetShipRenderBucketIndex(Ship* ship);
static color cat_smShipPointColor(Ship* ship);
color __cdecl smComputeOverlayFadeColor(color baseColor, sdword invert);
color __cdecl smGetOverlayFadeColor(void);
color __cdecl UpdateNebulaBrightness(color baseColor, real32 fade);
bool32 __cdecl smShipVisibleToPlayer(Ship* ship);
static bool32 smClShipCanRetreatForRepairs(Ship* ship, bool32 considerSingleShip);
static bool32 smShipCanDockForRepairs(Ship* ship, bool32 considerSingleShip);
extern sdword currentSensorCoverage;
extern sdword showDust;
extern sdword showResources;
extern sdword showFOWBlobs;
extern sdword showSensorBackgrounds;
extern sdword showAreasVisited;
extern sdword showCapitalShipOverlay;
extern sdword showEnemyShips;
extern sdword showFriendlyShips;
extern sdword showMeteors;
extern sdword showObjectivePointers;
extern sdword showSlipgatePaths;
extern sdword rndTextureEnabled;
extern real32 taskTimeElapsed;

typedef struct
{
	char* labelOverride;
	sdword labelStringIndex;
	sdword commandId;
	sdword hasValueColumn;
	sdword groupIndex;
	sdword enabled;
	sdword selectionLocked;
	sdword pulseHighlight;
	sdword forceSubmenuColumn;
	sdword hovered;
	sdword x0;
	sdword y0;
	sdword x1;
	sdword y1;
	sdword unused2;
} smOverlayEntry;

typedef struct
{
	char* labelOverride;
	sdword labelStringIndex;
	sdword commandId;
	sdword hasValueColumn;
	sdword groupIndex;
	sdword enabled;
	sdword selectionLocked;
	sdword pulseHighlight;
	sdword forceSubmenuColumn;
} smOverlayEntryTemplate;

static const smOverlayEntryTemplate smOverlayMasterTemplates[49] =
{
	{NULL, 0, 0, 0, 0, 0, 0, 0, 0}, {NULL, strMenuSpecialActionDesc, 7, 0, 1, 0, 0, 0, 0},
	{NULL, 0, 15, 0, 2, 0, 0, 0, 0}, {NULL, strMenuMoveDesc, 1, 0, 3, 0, 0, 0, 0},
	{NULL, strMenuHyperspaceDesc, 8, 0, 3, 0, 0, 0, 0}, {NULL, strMenuCancelMoveDesc, 90, 0, 3, 0, 0, 0, 0},
	{NULL, strMenuStartWaypointsDesc, 9, 0, 3, 0, 0, 0, 0}, {NULL, strMenuEndWaypointsDesc, 9, 0, 3, 0, 0, 0, 0},
	{NULL, strMenuWaypointTypeDesc, 0, 8, 3, 0, 0, 0, 0}, {NULL, strMenuCancelWaypointsDesc, 90, 0, 3, 0, 0, 0, 0},
	{NULL, strMenuRemoveLastWaypointDesc, 14, 0, 3, 0, 0, 0, 0}, {NULL, strMenuDockDesc, 2, 0, 3, 0, 0, 0, 0},
	{NULL, strMenuRetireDesc, 3, 0, 3, 0, 0, 0, 0}, {NULL, strMenuCancelDesc, 4, 0, 3, 0, 0, 0, 0},
	{NULL, strMenuTacticsDesc, 0, 1, 4, 0, 0, 0, 0}, {NULL, strMenuFormationsDesc, 0, 2, 4, 0, 0, 0, 0},
	{NULL, strMenuFocusDesc, 0, 5, 5, 0, 0, 0, 0}, {NULL, strMenuViewDesc, 0, 6, 6, 0, 0, 0, 0},
	{NULL, strMenuSensorsViewDesc, 0, 3, 7, 0, 0, 0, 0}, {NULL, strMenuSetShipNameDesc, 0, 7, 8, 0, 0, 0, 0},
	{NULL, 0, 8, 0, 9, 0, 0, 0, 0}, {NULL, strMenuAllianceFormDesc, 0, 11, 10, 0, 0, 0, 0},
	{NULL, strMenuAllianceBreakDesc, 0, 12, 10, 0, 0, 0, 0}, {NULL, strMenuTransferRusDesc, 0, 13, 10, 0, 0, 0, 0},
	{NULL, strMenuNoSpecialActionDesc, 0, 0, 11, 0, 0, 0, 0}, {NULL, strMenuSpecialHarvestDesc, 17, 0, 12, 0, 0, 0, 0},
	{NULL, strMenuSpecialRepairDesc, 18, 0, 12, 0, 0, 0, 0}, {NULL, strMenuSpecialSalvageDesc, 19, 0, 12, 0, 0, 0, 0},
	{NULL, strMenuSpecialSelectLeechTypeDesc, 20, 4, 13, 0, 0, 0, 0}, {NULL, strMenuSpecialLeechDesc, 24, 0, 13, 0, 0, 0, 0},
	{NULL, strMenuSpecialUnleechDesc, 25, 0, 13, 0, 0, 0, 0}, {NULL, strMenuSpecialLinkDesc, 26, 0, 14, 0, 0, 0, 0},
	{NULL, strMenuSpecialUnlinkDesc, 27, 0, 14, 0, 0, 0, 0}, {NULL, strMenuSpecialSelectMimicTypeDesc, 28, 9, 15, 0, 0, 0, 0},
	{NULL, strMenuSpecialSelectMimicItemDesc, 29, 10, 15, 0, 0, 0, 0}, {NULL, strMenuSpecialUnmimickDesc, 30, 0, 15, 0, 0, 0, 0},
	{NULL, strMenuSpecialForceFieldDesc, 31, 0, 16, 0, 0, 0, 0}, {NULL, strMenuSpecialUnlinkForceFieldDesc, 32, 0, 16, 0, 0, 0, 0},
	{NULL, strMenuSpecialCloakDesc, 33, 0, 17, 0, 0, 0, 0}, {NULL, strMenuSpecialCancelCloakDesc, 34, 0, 17, 0, 0, 0, 0},
	{NULL, strMenuSpecialLaunchSwarmersDesc, 35, 0, 18, 0, 0, 0, 0}, {NULL, strMenuSpecialCancelSwarmersDesc, 36, 0, 18, 0, 0, 0, 0},
	{NULL, strMenuSpecialFireBigGunDesc, 37, 0, 18, 0, 0, 0, 0}, {NULL, strMenuSpecialCancelBigGunDesc, 38, 0, 18, 0, 0, 0, 0},
	{NULL, strMenuSpecialBeastAttackDesc, 39, 0, 18, 0, 0, 0, 0}, {NULL, strMenuSpecialEMPDesc, 40, 0, 18, 0, 0, 0, 0},
	{NULL, strMenuSpecialRamDesc, 41, 0, 18, 0, 0, 0, 0}, {NULL, strMenuSpecialRepulseDesc, 42, 0, 18, 0, 0, 0, 0},
	{NULL, strMenuSpecialFireMissilesDesc, 43, 0, 18, 0, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView14Templates[3] =
{
	{NULL, strMenuEvasiveDesc, 44, 0, 4, 1, 0, 0, 0},
	{NULL, strMenuNeutralDesc, 45, 0, 4, 1, 0, 0, 0},
	{NULL, strMenuAggressiveDesc, 46, 0, 4, 1, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView15Templates[8] =
{
	{NULL, strMenuDeltaDesc, 49, 0, 4, 1, 0, 0, 0}, {NULL, strMenuBroadDesc, 50, 0, 4, 1, 0, 0, 0},
	{NULL, strMenuDelta3DDesc, 51, 0, 4, 1, 0, 0, 0}, {NULL, strMenuClawDesc, 52, 0, 4, 1, 0, 0, 0},
	{NULL, strMenuWallDesc, 53, 0, 4, 1, 0, 0, 0}, {NULL, strMenuSphereDesc, 54, 0, 4, 1, 0, 0, 0},
	{NULL, strMenuCustomDesc, 55, 0, 4, 1, 0, 0, 0}, {NULL, strMenuParadeDesc, 56, 0, 4, 1, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView18Templates[12] =
{
	{NULL, strMenuSV_AreasVisited, 141, 0, 7, 1, 0, 0, 0}, {NULL, strMenuSV_ShipFOWRange, 140, 0, 7, 1, 0, 0, 0},
	{NULL, strMenuSV_AreasOccupied, 142, 0, 7, 1, 0, 0, 0}, {NULL, strMenuSV_Asteriods, 139, 0, 7, 1, 0, 0, 0},
	{NULL, strMenuSV_DecorativeAsteroids, 147, 0, 7, 1, 0, 0, 0}, {NULL, strMenuSV_Background, 146, 0, 7, 1, 0, 0, 0},
	{NULL, strMenuSV_Compass, 148, 0, 7, 1, 0, 0, 0}, {NULL, strMenuSV_ObjectivesPointer, 149, 0, 7, 1, 0, 0, 0},
	{NULL, strMenuSV_Icons, 143, 0, 7, 1, 0, 0, 0}, {NULL, strMenuSV_SlipgatePaths, 150, 0, 7, 1, 0, 0, 0},
	{NULL, strMenuSV_ShipMeshes, 144, 0, 7, 1, 0, 0, 0}, {NULL, strMenuSV_PieplateLines, 145, 0, 7, 1, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView28Templates[4] =
{
	{NULL, strMenuSpyDesc, 21, 0, 11, 1, 0, 0, 0},
	{NULL, strMenuLeechDesc, 22, 0, 11, 1, 0, 0, 0},
	{NULL, strMenuVentDesc, 23, 0, 11, 1, 0, 0, 0},
	{NULL, 0, 0, 0, 0, 0, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView16Templates[4] =
{
	{NULL, strMenuFocucSelectedDesc, 59, 0, 5, 1, 0, 0, 0},
	{NULL, strMenuFocusOnMothershipDesc, 60, 0, 5, 1, 0, 0, 0},
	{NULL, strMenuFocusOnLastShipToSpeakDesc, 61, 0, 5, 1, 0, 0, 0},
	{NULL, strMenuFocusFleetViewDesc, 64, 0, 5, 1, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView17Templates[8] =
{
	{NULL, strMenuViewMainRgnDesc, 79, 0, 6, 1, 0, 0, 0}, {NULL, strMenuViewSensorsDesc, 79, 0, 6, 1, 0, 0, 0},
	{NULL, strMenuViewBuildManagerDesc, 80, 0, 6, 1, 0, 0, 0}, {NULL, strMenuViewResearchManagerDesc, 81, 0, 6, 1, 0, 0, 0},
	{NULL, strMenuViewLaunchManagerDesc, 82, 0, 6, 1, 0, 0, 0}, {NULL, strMenuViewChaseModeDesc, 84, 0, 6, 1, 0, 0, 0},
	{NULL, strMenuViewShipOverlayDesc, 83, 0, 6, 1, 0, 0, 0}, {NULL, strMenuViewTacticalOverlayDesc, 71, 0, 6, 1, 0, 0, 0},
};

static const smOverlayEntryTemplate smOverlayView8Templates[3] =
{
	{NULL, strMenuWaypointTypeNormal, 11, 0, 3, 1, 0, 0, 0},
	{NULL, strMenuWaypointTypeEndToEnd, 12, 0, 3, 1, 0, 0, 0},
	{NULL, strMenuWaypointTypeCircular, 13, 0, 3, 1, 0, 0, 0},
};

#define SM_MIMIC_PLAYER_LABEL_COUNT 9
#define SM_MIMIC_ENTRY_LABEL_COUNT  84

sdword smFogRenderListsDirty = 0;
sdword smFogDirtyUpdateCount = 0;
sdword smFogOccupiedRenderState = 0;
sdword smFogVisibleRenderState = 0;
sdword smFogUnknownRenderState = 0;
sdword smFogOccupiedDirtyList = 0;
sdword smFogVisibleDirtyList = 0;
sdword smFogUnknownDirtyList = 0;
real32 smEnemySpottedNotificationTime = 0.0f;
udword fogFreeRenderState(void);

static bool32 smShipIsSelected(Ship* ship)
{
	return ShipInSelection((SelectCommand*)&selSelected, ship);
}

void smDrawProjectedShipOutline(Ship* ship, color c)
{
	rectangle rect;
	vector* point;
	real32 x0, y0, x1, y1;
	real32 radius;
	sdword index;

	if (ship->collInfo.selCircleRadius <= 0.0f || ship->collInfo.selCircleDepth <= 0.0f)
	{
		return;
	}

	if (ship->collInfo.precise != NULL &&
		ship->currentLOD <= ship->staticinfo->staticheader.staticCollInfo.preciseSelection)
	{
		point = ship->collInfo.precise->worldRectPos;
		x0 = y1 = REALlyBig;
		x1 = y0 = REALlyNegative;
		for (index = 0; index < 8; index++)
		{
			x0 = min(x0, point[index].x);
			y0 = max(y0, point[index].y);
			x1 = max(x1, point[index].x);
			y1 = min(y1, point[index].y);
		}

		rect.x0 = primGLToScreenX(x0);
		rect.y0 = primGLToScreenY(y0);
		rect.x1 = primGLToScreenX(x1);
		rect.y1 = primGLToScreenY(y1);
	}
	else
	{
		radius = max(
			ship->collInfo.selCircleRadius * ship->staticinfo->tacticalSelectionScale,
			selMinSelectionRadius);
		rect.x0 = primGLToScreenX(ship->collInfo.selCircleX - radius);
		rect.x1 = primGLToScreenX(ship->collInfo.selCircleX + radius);
		rect.y0 = primGLToScreenY(ship->collInfo.selCircleY + radius);
		rect.y1 = primGLToScreenY(ship->collInfo.selCircleY - radius);
	}

	primGLRectOutline2(&rect, 1, c);
}

static void smZoomInOnShip(Ship* ship);
sdword smBlobsSortByZ(vector* p0, vector* p1);

static bool32 smIsCrystal(Ship* ship)
{
	return ship != NULL &&
		(ship->shiptype == dCrystal0 || ship->shiptype == dCrystal1);
}

static bool32 smShipIsBeastShip(Ship* ship)
{
	return ship != NULL &&
		((ship->staticinfo->shipFlags >> 14) & 1) != 0;
}

static bool32 smShipUsesFriendlySensorsSetting(Ship* ship)
{
	return ship != NULL &&
		(ship->playerowner == universe.curPlayerPtr ||
			allianceArePlayersAllied(ship->playerowner, universe.curPlayerPtr));
}

static bool32 smShipAllowedBySensorsSettings(Ship* ship)
{
	if (ship == NULL)
	{
		return FALSE;
	}
	if (smShipUsesFriendlySensorsSetting(ship))
	{
		return showFriendlyShips != 0;
	}
	return showEnemyShips != 0;
}

typedef struct
{
	Ship* linkedShip;
	udword unk_04;
	udword unk_08;
	Effect* effect;
	udword unk_10;
	udword unk_14;
	color linkColor;
	ubyte playerTransitState[8];
} catSmShipLinkOverlay;

static const ubyte catSmShipRenderFlags[128] =
{
	0, 0, 0, 0, 0, 0, 1, 1, 1, 4, 3, 4, 3, 4, 1, 4,
	3, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 4, 1, 1, 3, 3,
	1, 1, 4, 3, 0, 6, 6, 6, 6, 6, 0, 0, 1, 0, 3, 0,
	0, 0, 3, 0, 0, 1, 0, 4, 0, 1, 1, 0, 3, 0, 1, 0,
	3, 0, 0, 1, 0, 1, 0, 4, 1, 0, 1, 1, 1, 0, 1, 0,
	3, 0, 3, 1, 3, 3, 0, 0, 3, 0, 0, 0, 0, 6, 6, 6,
	0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0
};

typedef struct
{
	ubyte* data;
	sdword width;
	sdword height;
	sdword depth;
	real32 originX;
	real32 originY;
	real32 originZ;
	real32 cellSize;
} smFogGridRuntimeView;

typedef struct
{
	sdword occupancy;
	sdword visibility;
	vector center;
	sdword renderFlags[3];
} smFogGridCell;

typedef struct
{
	vector start;
	vector end;
	real32 startDistance;
	real32 endDistance;
} smFogRenderLine;

typedef struct
{
	smFogRenderLine* lines;
	sdword count;
	sdword capacity;
} smFogRenderLineBuffer;

typedef struct
{
	Node link;
	sdword edgeVisible[12];
	sdword cellIndex;
} smFogRenderCellState;

typedef struct
{
	sdword x;
	sdword y;
	sdword z;
} smFogDirtyCellCoord;

static smFogGridCell* smFogGridCells(smFogGridRuntimeView* fogGrid);
static sdword smFogGridIndexIfValid(smFogGridRuntimeView* fogGrid, sdword x, sdword y, sdword z);
static void smFogMarkDirtyGridCells(sdword x, sdword y, sdword z, sdword expand);
real32 univGetMaxHealth(Ship* ship, bool32 isEnemy);

typedef struct
{
	sdword unused;
	sdword baseCount;
	sdword countVariance;
	real32 xMax;
	real32 xMin;
	real32 yCenter;
	real32 yHalfHeight;
	real32 hueCenter;
	real32 hueVariance;
	real32 saturationCenter;
	real32 saturationVariance;
	real32 luminosityCenter;
	real32 luminosityVariance;
	real32 alphaCenter;
	real32 alphaVariance;
	bool32 blend;
	sdword texture;
} smNoiseParams;

static smFogRenderLineBuffer smFogRenderLines = {NULL, 0, 0};
static color smFogOccupiedColors[16];
static color smFogVisibleColors[16];
static color smFogUnknownColors[16];
static bool32 smFogRenderPaletteDirty = TRUE;
static const ubyte smFogEdgeCornerPairs[12][2] =
{
	{0, 1}, {1, 2}, {2, 3}, {3, 0},
	{4, 5}, {5, 6}, {6, 7}, {7, 4},
	{4, 0}, {5, 1}, {6, 2}, {7, 3}
};
static const ubyte smFogCornerEdges[8][3] =
{
	{3, 8, 0},
	{0, 9, 1},
	{1, 10, 2},
	{2, 11, 3},
	{7, 8, 4},
	{4, 9, 5},
	{5, 10, 6},
	{6, 11, 7}
};
static const sbyte smFogEdgeNeighborOffsets[12][3][3] =
{
	{{0, -1, 0}, {0, -1, 1}, {0, 0, 1}},
	{{0, -1, 0}, {1, -1, 0}, {1, 0, 0}},
	{{0, -1, 0}, {0, -1, -1}, {0, 0, -1}},
	{{0, -1, 0}, {-1, -1, 0}, {-1, 0, 0}},
	{{0, 1, 0}, {0, 1, 1}, {0, 0, 1}},
	{{0, 1, 0}, {1, 1, 0}, {1, 0, 0}},
	{{0, 1, 0}, {0, 1, -1}, {0, 0, -1}},
	{{0, 1, 0}, {-1, 1, 0}, {-1, 0, 0}},
	{{-1, 0, 0}, {-1, 0, 1}, {0, 0, 1}},
	{{0, 0, 1}, {1, 0, 1}, {1, 0, 0}},
	{{1, 0, 0}, {1, 0, -1}, {0, 0, -1}},
	{{0, 0, -1}, {-1, 0, -1}, {-1, 0, 0}}
};
static const color smFogOccupiedColorNear = colRGB(0, 0, 50);
static const color smFogOccupiedColorFar = colRGB(0, 0, 255);
static const color smFogVisibleColorNear = colRGB(50, 50, 50);
static const color smFogVisibleColorFar = colRGB(200, 200, 200);
static const color smFogUnknownColorNear = colRGB(50, 50, 50);
static const color smFogUnknownColorFar = colRGB(200, 200, 200);
static const real32 smFogPaletteBlendStep = 0.06666667f;
static const real32 smFogPaletteIndexScale = 15.0f;
static const real32 smFogCornerTickScale = 0.2f;
LinkedList smFogOccupiedRenderList = UNINITIALISED_LINKED_LIST;
LinkedList smFogUnknownRenderList = UNINITIALISED_LINKED_LIST;
LinkedList smFogVisibleRenderList = UNINITIALISED_LINKED_LIST;
sdword smFogOccupiedDirtyCount = 0;
sdword smFogVisibleDirtyCount = 0;
sdword smFogUnknownDirtyCount = 0;
sdword smFogVisibilityMode = 2;
sdword smOverlaySensorViewStateValid = 0;
sdword smOverlaySensorResourcesEnabled = 0;
sdword smOverlaySensorFowBlobsEnabled = 0;
sdword smOverlaySensorAreasVisitedEnabled = 0;
sdword smOverlaySensorAreasOccupiedEnabled = 0;
sdword smOverlaySensorCapitalShipOverlayEnabled = 0;
sdword smOverlaySensorFriendlyShipsEnabled = 0;
sdword smOverlaySensorEnemyShipsEnabled = 0;
sdword smOverlaySensorBackgroundEnabled = 0;
sdword smOverlaySensorDustEnabled = 0;
sdword smOverlaySensorCompassEnabled = 0;
sdword smOverlaySensorObjectivePointersEnabled = 0;
sdword smOverlaySensorSlipgatePathsEnabled = 0;
sdword smOverlaySensorDecorativeAsteroidsEnabled = 0;
sdword smOverlaySensorShipFowRangeEnabled = 0;
sdword smOverlaySensorAsteroidsEnabled = 0;
sdword smOverlaySensorShipMeshesEnabled = 0;
sdword smOverlaySensorPieplateLinesEnabled = 0;
sdword smOverlaySensorUnknown16Enabled = 0;
sdword smOverlaySensorUnknown17Enabled = 0;
sdword smOverlaySensorUnknown18Enabled = 0;
sdword smOverlaySensorUnknown19Enabled = 0;
sdword smOverlaySensorUnknown20Enabled = 0;
sdword smOverlaySensorUnknown21Enabled = 0;
sdword smOverlaySensorUnknown22Enabled = 0;
sdword smOverlaySensorUnknown23Enabled = 0;
MaxAnySelection smOverlayExplicitTargets = {0};
MaxAnySelection smOverlayDefaultTargets = {0};
sdword smPathMarkersEnabled = 0;
sdword nameIndex = -1;
sdword smOverlayIsOpen = 0;
sdword smOverlayAnchorX = 0;
sdword smOverlayAnchorY = 0;
sdword smOverlayIsActionMenu = 0;
sdword smOverlayRowPadding = 0;
sdword smOverlayHighlightCapWidth = 0;
sdword smOverlaySubmenuIndicatorWidth = 0;
sdword smOverlayValueColumnWidth = 0;
sdword smOverlayLabelLeftPadding = 0;
sdword smOverlayLabelRightPadding = 0;
sdword smOverlayCommandColumnGap = 0;
sdword smOverlayGroupSpacing = 0;
sdword smOverlayHighlightSidePadding = 0;
sdword smOverlayHeaderBorderSize = 0;
sdword smOverlayRootColumnXOffset = 5;
sdword smOverlayRootColumnYOffset = 5;
sdword smOverlaySubviewXOffset = 10;
sdword smOverlaySubviewYOffset = 0;
rectangle smOverlayMasterRect = {0, 0, 0, 0};
rectangle smOverlayView14Rect = {0, 0, 0, 0};
rectangle smOverlayView15Rect = {0, 0, 0, 0};
rectangle smOverlayView18Rect = {0, 0, 0, 0};
rectangle smOverlayView28Rect = {0, 0, 0, 0};
rectangle smOverlayView16Rect = {0, 0, 0, 0};
rectangle smOverlayView17Rect = {0, 0, 0, 0};
rectangle smOverlayView19Rect = {0, 0, 0, 0};
rectangle smOverlayView8Rect = {0, 0, 0, 0};
rectangle smOverlayView33Rect = {0, 0, 0, 0};
rectangle smOverlayView34Rect = {0, 0, 0, 0};
rectangle smOverlayView21Rect = {0, 0, 0, 0};
rectangle smOverlayView22Rect = {0, 0, 0, 0};
rectangle smOverlayView23Rect = {0, 0, 0, 0};
sdword smOverlayMasterHasSubmenuColumn = 0;
sdword smOverlayMasterLabelWidth = 0;
sdword smOverlayView18HasSubmenuColumn = 0;
sdword smOverlayView14HasSubmenuColumn = 0;
sdword smOverlayView15HasSubmenuColumn = 0;
sdword smOverlayView28HasSubmenuColumn = 0;
sdword smOverlayView16HasSubmenuColumn = 0;
sdword smOverlayView17HasSubmenuColumn = 0;
sdword smOverlayView18LabelWidth = 0;
sdword smOverlayView14LabelWidth = 0;
sdword smOverlayView15LabelWidth = 0;
sdword smOverlayView28LabelWidth = 0;
sdword smOverlayView16LabelWidth = 0;
sdword smOverlayView17LabelWidth = 0;
sdword smOverlayView19HasSubmenuColumn = 0;
sdword smOverlayView19LabelWidth = 0;
sdword smOverlayView8HasSubmenuColumn = 0;
sdword smOverlayView8LabelWidth = 0;
sdword smOverlayView33HasSubmenuColumn = 0;
sdword smOverlayView33LabelWidth = 0;
sdword smOverlayView34HasSubmenuColumn = 0;
sdword smOverlayView34LabelWidth = 0;
sdword smOverlayView21HasSubmenuColumn = 0;
sdword smOverlayView21LabelWidth = 0;
sdword smOverlayView22HasSubmenuColumn = 0;
sdword smOverlayView22LabelWidth = 0;
sdword smOverlayView23HasSubmenuColumn = 0;
sdword smOverlayView23LabelWidth = 0;
sdword smOverlayView19Count = 0;
sdword smOverlayView33Count = 0;
sdword smOverlayView34Count = 0;
sdword smOverlayView21Count = 0;
sdword smOverlayView22Count = 0;
static sdword smOverlayRuTransferPlayerCount = 0;
sdword smOverlayMimicGroupIndex = 0;
sdword smOverlayLastMouseX = 0;
sdword smOverlayLastMouseY = 0;
fonthandle smOverlayLargeFont = 0;
fonthandle smOverlaySmallFont = 0;
sdword smOverlayActiveViewIndex = -1;
// Master tactical-overlay column entries (retail data at 0x88DE88).
smOverlayEntry smOverlayMasterEntries[49] = {0};
smOverlayEntry unk_88EA08[49] = {0};
smOverlayEntry unk_88EAC0[49] = {0};
smOverlayEntry unk_88ECA0[49] = {0};
smOverlayEntry unk_88EF70[49] = {0};
smOverlayEntry unk_88F060[49] = {0};
smOverlayEntry unk_88F150[49] = {0};
smOverlayEntry unk_88F330[49] = {0};
smOverlayEntry unk_A51D98[49] = {0};
smOverlayEntry unk_A52950[49] = {0};
smOverlayEntry smOverlayView22Entries[49] = {0};
smOverlayEntry smOverlayView23Entries[49] = {0};
smOverlayEntry unk_A57270[49] = {0};
smOverlayEntry unk_A53CC0[49] = {0};
char smMimicPlayerLabels[SM_MIMIC_PLAYER_LABEL_COUNT][50] = {0};
char smMimicEntryLabels[SM_MIMIC_ENTRY_LABEL_COUNT][50] = {0};
extern sdword showAreasOccupied;
extern sdword autoFocusOnWaypoints;
extern sdword showUniverseExtents;
extern sdword largePopupMenuFonts;

sdword smOverlayRglFillByte0 = 0;
sdword smOverlayRglFillByte1 = 0;
sdword smOverlayRglFillByte2 = 0x3c;
sdword smOverlayRglFillByte3 = 0x96;
sdword smOverlaySoftwareFillByte0 = -1;
sdword smOverlaySoftwareFillByte1 = -1;
sdword smOverlaySoftwareFillByte2 = -1;
sdword smOverlaySoftwareFillByte3 = -1;
real32 smOverlayPulsePeriod = 5.0f;
real32 smOverlayPulseStartTime = 0.0f;
color smOverlayPulseColorStart = 0xff0000ff;
color smOverlayPulseColorEnd = 0xff00ffff;
color smOverlayLockedRowColor = 0xff787878;
color smOverlayDefaultRowColor = 0xffffffff;
color smOverlaySelectedMasterRowColor = 0xff00c800;
real64 dbl_72A480 = 0.5;
real64 dbl_72A548 = 1.0;
real64 dbl_72AEC8 = -2.0;

static real32 smShipHealthRatio(Ship* ship)
{
	return ship->health / ship->staticinfo->maxhealth;
}

static smFogGridRuntimeView* smCurrentFogGrid(void)
{
	if (currentSensorCoverage == 0)
	{
		return NULL;
	}

	return (smFogGridRuntimeView*)currentSensorCoverage;
}


static real32 smNoiseRandomUnit(void)
{
	return frandom(1.0f);
}

static real32 smNoiseRandomRange(real32 minimum, real32 maximum)
{
	return smNoiseRandomUnit() * (maximum - minimum) + minimum;
}

static bool32 smFogPointInBounds(vector* position)
{
	smFogGridRuntimeView* fogGrid;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		return FALSE;
	}

	return position->x >= fogGrid->originX &&
		position->x < fogGrid->originX + fogGrid->width * fogGrid->cellSize &&
		position->y >= fogGrid->originY &&
		position->y < fogGrid->originY + fogGrid->height * fogGrid->cellSize &&
		position->z >= fogGrid->originZ &&
		position->z < fogGrid->originZ + fogGrid->depth * fogGrid->cellSize;
}

static sdword smFogGridRangeValueAtPoint(vector* position)
{
	smFogGridRuntimeView* fogGrid;
	real32 invCellSize;
	sdword gridX;
	sdword gridY;
	sdword gridZ;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		return -524288;
	}

	invCellSize = 1.0f / fogGrid->cellSize;
	gridX = (sdword)((position->x - fogGrid->originX) * invCellSize);
	gridY = (sdword)((position->y - fogGrid->originY) * invCellSize);
	gridZ = (sdword)((position->z - fogGrid->originZ) * invCellSize);
	if (gridX < 0 || gridX >= fogGrid->width ||
		gridY < 0 || gridY >= fogGrid->height ||
		gridZ < 0 || gridZ >= fogGrid->depth)
	{
		return -524288;
	}

	return *(sdword*)(fogGrid->data
		+ 32 * (gridX + fogGrid->width * (gridY + gridZ * fogGrid->height)));
}

static sdword smFogGridVisibilityValueAtPoint(vector* position)
{
	smFogGridRuntimeView* fogGrid;
	real32 invCellSize;
	sdword gridX;
	sdword gridY;
	sdword gridZ;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		return -524288;
	}

	invCellSize = 1.0f / fogGrid->cellSize;
	gridX = (sdword)((position->x - fogGrid->originX) * invCellSize);
	gridY = (sdword)((position->y - fogGrid->originY) * invCellSize);
	gridZ = (sdword)((position->z - fogGrid->originZ) * invCellSize);
	if (gridX < 0 || gridX >= fogGrid->width ||
		gridY < 0 || gridY >= fogGrid->height ||
		gridZ < 0 || gridZ >= fogGrid->depth)
	{
		return -524288;
	}

	return *(sdword*)(fogGrid->data
		+ 32 * (gridX + fogGrid->width * (gridY + gridZ * fogGrid->height))
		+ sizeof(sdword));
}

static void smFogComputeGridIndices(real32 worldX, real32 worldY, real32 worldZ, real32 radius,
	sdword* gridX, sdword* gridY, sdword* gridZ, sdword* gridRadius)
{
	smFogGridRuntimeView* fogGrid;
	real32 invCellSize;

	fogGrid = smCurrentFogGrid();
	smFogRenderListsDirty = 0;
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return;
	}

	invCellSize = 1.0f / fogGrid->cellSize;
	if (gridX != NULL)
	{
		*gridX = (sdword)((worldX - fogGrid->originX) * invCellSize);
	}
	if (gridY != NULL)
	{
		*gridY = (sdword)((worldY - fogGrid->originY) * invCellSize);
	}
	if (gridZ != NULL)
	{
		*gridZ = (sdword)((worldZ - fogGrid->originZ) * invCellSize);
	}
	if (gridRadius != NULL)
	{
		*gridRadius = (sdword)(radius * invCellSize) + 1;
	}
}

static void smFogIncrementGridCell(sdword x, sdword y, sdword z)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cell;
	sdword cellIndex;

	if (x < 0 || y < 0 || z < 0)
	{
		return;
	}

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL ||
		x >= fogGrid->width ||
		y >= fogGrid->height ||
		z >= fogGrid->depth)
	{
		return;
	}

	smFogRenderListsDirty = 0;
	smFogMarkDirtyGridCells(x, y, z, 1);
	cellIndex = smFogGridIndexIfValid(fogGrid, x, y, z);
	cell = &smFogGridCells(fogGrid)[cellIndex];
	cell->occupancy++;
	cell->visibility = 1;
}

static void smFogFillGridCells(sdword gridX, sdword gridY, sdword gridZ, sdword gridRadius)
{
	smFogGridRuntimeView* fogGrid;
	sdword minX;
	sdword maxX;
	sdword minY;
	sdword maxY;
	sdword minZ;
	sdword maxZ;
	sdword curX;
	sdword curY;
	sdword curZ;

	fogGrid = smCurrentFogGrid();
	smFogRenderListsDirty = 0;
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return;
	}

	minX = max(0, gridX - gridRadius);
	maxX = min(fogGrid->width - 1, gridX + gridRadius);
	minY = max(0, gridY - gridRadius);
	maxY = min(fogGrid->height - 1, gridY + gridRadius);
	minZ = max(0, gridZ - gridRadius);
	maxZ = min(fogGrid->depth - 1, gridZ + gridRadius);

	for (curZ = minZ; curZ <= maxZ; ++curZ)
	{
		for (curY = minY; curY <= maxY; ++curY)
		{
			for (curX = minX; curX <= maxX; ++curX)
			{
				smFogIncrementGridCell(curX, curY, curZ);
			}
		}
	}
}

static bool32 smFogHasUnseenGridCells(sdword gridX, sdword gridY, sdword gridZ, sdword gridRadius)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	sdword minX;
	sdword maxX;
	sdword minY;
	sdword maxY;
	sdword minZ;
	sdword maxZ;
	sdword curX;
	sdword curY;
	sdword curZ;

	fogGrid = smCurrentFogGrid();
	smFogRenderListsDirty = 0;
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return FALSE;
	}

	cells = smFogGridCells(fogGrid);
	minX = max(0, gridX - gridRadius);
	maxX = min(fogGrid->width - 1, gridX + gridRadius);
	minY = max(0, gridY - gridRadius);
	maxY = min(fogGrid->height - 1, gridY + gridRadius);
	minZ = max(0, gridZ - gridRadius);
	maxZ = min(fogGrid->depth - 1, gridZ + gridRadius);

	for (curZ = minZ; curZ <= maxZ; ++curZ)
	{
		for (curY = minY; curY <= maxY; ++curY)
		{
			for (curX = minX; curX <= maxX; ++curX)
			{
				if (cells[smFogGridIndexIfValid(fogGrid, curX, curY, curZ)].visibility == 0)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static sdword smFogGetGridCellVisibility(sdword gridX, sdword gridY, sdword gridZ)
{
	smFogGridRuntimeView* fogGrid;
	sdword cellIndex;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return -524288;
	}
	if (gridX < 0 || gridX >= fogGrid->width ||
		gridY < 0 || gridY >= fogGrid->height ||
		gridZ < 0 || gridZ >= fogGrid->depth)
	{
		return -524288;
	}

	smFogRenderListsDirty = 0;
	cellIndex = smFogGridIndexIfValid(fogGrid, gridX, gridY, gridZ);
	return smFogGridCells(fogGrid)[cellIndex].visibility;
}

static sdword smPlayerSensorCoverage(Player* player)
{
	if (!multiPlayerGame)
	{
		return (sdword)player->fogGrid;
	}
	if (allianceArePlayersAllied(universe.curPlayerPtr, player))
	{
		return (sdword)universe.curPlayerPtr->fogGrid;
	}

	return (sdword)player->fogGrid;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x542240
sdword univUpdateShipProximity(Ship* ship)
{
	sdword sensorCoverage;
	sdword savedSensorCoverage;
	real32 sensorRadius;
	sdword gridX;
	sdword gridY;
	sdword gridZ;
	sdword gridRadius;

	sensorCoverage = smPlayerSensorCoverage(ship->playerowner);
	if (sensorCoverage == 0)
	{
		return 0;
	}

	savedSensorCoverage = univSetCurrentSensorCoverage(sensorCoverage);
	if (physShipIsMovable(ship))
	{
		sensorRadius = univGetMaxHealth(ship, FALSE);
		smFogComputeGridIndices(ship->posinfo.position.x, ship->posinfo.position.y, ship->posinfo.position.z, sensorRadius,
			&gridX, &gridY, &gridZ, &gridRadius);
		smFogFillGridCells(gridX, gridY, gridZ, gridRadius);
	}

	return univSetCurrentSensorCoverage(savedSensorCoverage);
}

static smFogGridCell* smFogGridCells(smFogGridRuntimeView* fogGrid)
{
	return (smFogGridCell*)fogGrid->data;
}

static smFogRenderCellState* smFogRenderStates(sdword renderStateBase)
{
	return (smFogRenderCellState*)renderStateBase;
}

static smFogDirtyCellCoord* smFogDirtyCells(sdword dirtyListBase)
{
	return (smFogDirtyCellCoord*)dirtyListBase;
}

static sdword smFogGridIndexIfValid(smFogGridRuntimeView* fogGrid, sdword x, sdword y, sdword z)
{
	if (x < 0 || x >= fogGrid->width ||
		y < 0 || y >= fogGrid->height ||
		z < 0 || z >= fogGrid->depth)
	{
		return -1;
	}

	return x + fogGrid->width * (y + z * fogGrid->height);
}

static bool32 smFogCellMatchesRenderMode(smFogGridCell* cell, sdword renderMode)
{
	switch (renderMode)
	{
		case 0:
			return cell->visibility != 0 && cell->occupancy <= 0;
		case 1:
			return cell->visibility != 0;
		case 2:
			return cell->occupancy > 0;
		default:
			return FALSE;
	}
}

static sdword* smFogCellDirtyFlag(smFogGridCell* cell, sdword renderMode)
{
	switch (renderMode)
	{
		case 0:
			return &cell->renderFlags[2];
		case 1:
			return &cell->renderFlags[1];
		case 2:
			return &cell->renderFlags[0];
		default:
			return NULL;
	}
}

static void smFogAppendDirtyCell(sdword dirtyListBase, sdword* dirtyCount, sdword x, sdword y, sdword z)
{
	smFogDirtyCellCoord* dirtyCell;

	dirtyCell = &smFogDirtyCells(dirtyListBase)[*dirtyCount];
	dirtyCell->x = x;
	dirtyCell->y = y;
	dirtyCell->z = z;
	(*dirtyCount)++;
}

static void smFogMarkDirtyGridCells(sdword x, sdword y, sdword z, sdword expand)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	sdword curZ;
	sdword prevZ;
	sdword cellIndex;
	smFogGridCell* cell;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL || x < 0 || x >= fogGrid->width || y < 0 || y >= fogGrid->height)
	{
		return;
	}

	cells = smFogGridCells(fogGrid);
	curZ = z;
	prevZ = z - 1;
	while (prevZ >= -1 && curZ < fogGrid->depth)
	{
		cellIndex = smFogGridIndexIfValid(fogGrid, x, y, curZ);
		if (cellIndex < 0)
		{
			break;
		}

		cell = &cells[cellIndex];
		if (cell->renderFlags[0] == 0)
		{
			cell->renderFlags[0] = 1;
			smFogAppendDirtyCell(smFogOccupiedDirtyList, &smFogOccupiedDirtyCount, x, y, curZ);
		}
		if (cell->renderFlags[1] == 0)
		{
			cell->renderFlags[1] = 1;
			smFogAppendDirtyCell(smFogVisibleDirtyList, &smFogVisibleDirtyCount, x, y, curZ);
		}
		if (cell->renderFlags[2] == 0)
		{
			cell->renderFlags[2] = 1;
			smFogAppendDirtyCell(smFogUnknownDirtyList, &smFogUnknownDirtyCount, x, y, curZ);
		}

		if (!expand)
		{
			break;
		}

		smFogMarkDirtyGridCells(x - 1, y, curZ, 0);
		smFogMarkDirtyGridCells(x + 1, y, curZ, 0);
		smFogMarkDirtyGridCells(x, y - 1, curZ, 0);
		smFogMarkDirtyGridCells(x, y + 1, curZ, 0);
		smFogMarkDirtyGridCells(x, y, prevZ, 0);
		curZ++;
		prevZ++;
		expand = 0;
	}
}

static color* smFogPaletteForMode(sdword renderMode)
{
	switch (renderMode)
	{
		case 0:
			return smFogUnknownColors;
		case 1:
			return smFogVisibleColors;
		case 2:
			return smFogOccupiedColors;
		default:
			return smFogVisibleColors;
	}
}

static void smFogBuildRenderPalette(color* palette, color nearColor, color farColor)
{
	sdword i;

	for (i = 0; i < 16; ++i)
	{
		palette[i] = colBlend(nearColor, farColor, i * smFogPaletteBlendStep);
	}
}

sdword __cdecl CheckIfShipInRange(real32 x, real32 y, real32 z)
{
	vector position;

	position.x = x;
	position.y = y;
	position.z = z;
	return smFogGridRangeValueAtPoint(&position);
}

bool32 fogPointInGridBounds(real32 x, real32 y, real32 z)
{
	vector position;

	position.x = x;
	position.y = y;
	position.z = z;
	return smFogPointInBounds(&position);
}

sdword fogGetGridValue(real32 x, real32 y, real32 z)
{
	vector position;

	position.x = x;
	position.y = y;
	position.z = z;
	return smFogGridVisibilityValueAtPoint(&position);
}

bool32 fogCheckPositionVisible(vector* position, bool32 useGridLookup)
{
	if (universe.curPlayerPtr == NULL)
	{
		return FALSE;
	}
	if (universe.curPlayerPtr->fogGrid == NULL)
	{
		return TRUE;
	}
	if (!fogPointInGridBounds(position->x, position->y, position->z))
	{
		return FALSE;
	}

	if (useGridLookup)
	{
		return fogGetGridValue(position->x, position->y, position->z) <= 0;
	}

	return CheckIfShipInRange(position->x, position->y, position->z) <= 0;
}

/*=============================================================================
	Data:
=============================================================================*/
//set this to TRUE if you want it to be the Fleet Intel version of the SM
bool32 smFleetIntel = FALSE;

//toggles for what to display
Camera smCamera;
Camera smTempCamera;

sdword smTacticalOverlay = FALSE;
sdword smResources = TRUE;
sdword smNonResources = TRUE;
bool32 smFocusOnMothershipOnClose = FALSE;

//arrays of stars for quick star rendering
extern ubyte* stararray;

extern color HorseRaceDropoutColor;

extern udword gDevcaps2;

//region handle for the sensors manager viewport
regionhandle smViewportRegion;
regionhandle smBaseRegion;
rectangle smViewRectangle;

sdword smClicking = FALSE;

//table of functions to draw a mission sphere at varying levels of visibility
void smSelectHold(void);
//point specification stuffs
/*
sdword smPointSpecMode = SPM_Idle;
sdword smCrosshairSize = SM_CrosshairSize;
sdword smMousePauseX;
sdword smMousePauseY;
*/
//colors for differing sphere statuses
color spcNormal = SPC_Normal;
color spcSelected = SPC_Selected;
color spcTeamColor = SPC_TeamColor;

// color for printing the A for alliances
color alliescolor = colRGB(255, 255, 0);

//list of blobs encompassing all objects in universe
//LinkedList smBlobList;
//BlobProperties smBlobProperties;

//fog-of-war sub-blob stuff
BlobProperties smFOWBlobProperties;
real32 smFOW_AsteroidK1;
real32 smFOW_AsteroidK2;
real32 smFOW_AsteroidK3;
real32 smFOW_AsteroidK4;
real32 smFOW_DustGasK2;
real32 smFOW_DustGasK3;
real32 smFOW_DustGasK4;
real32 smFOWBlobUpdateTime = SM_FOWBlobUpdateTime;

//global flag which says the sensors manager is active
bool32 smSensorsActive = FALSE;

bool32 smSensorsDisable = FALSE;
bool32 smSensorsForceOpen = FALSE;

//for updating blobs
sdword smRenderCount;
color smBackgroundColor = colFuscia, smIntBackgroundColor;
color smBlobColor = SM_BlobColor;
color smBlobColorHigh = SM_BlobColorHigh;
color smBlobColorLow = SM_BlobColorLow;
color smIntBlobColor;
color smIntBlobColorHigh;
color smIntBlobColorLow;

//for zooming
real32 smZoomTime;
region** smScrollListLeft, ** smScrollListTop, ** smScrollListRight, ** smScrollListBottom;
sdword smScrollCountLeft, smScrollCountTop, smScrollCountRight, smScrollCountBottom;
sdword smScrollDistLeft, smScrollDistTop, smScrollDistRight, smScrollDistBottom;
sdword smScrollLeft, smScrollTop, smScrollRight, smScrollBottom;
vector smLookStart, smLookEnd;
vector smEyeStart, smEyeEnd;
bool8 smZoomingIn = FALSE, smZoomingOut = FALSE, smFocus = FALSE, smFocusTransition = FALSE;
bool8 smFocusClose = FALSE;
FocusCommand* smFocusCommand = NULL;

static void smStartFocusZoom(Ship** ships, sdword numShips, vector* centre, real32 focusDistance, bool32 closeFocus)
{
	sdword index;

	smFocus = TRUE;
	smFocusClose = (bool8)closeFocus;
	smFocusCommand = memAlloc(ccFocusSize(numShips), "SensorsMultiFocus", 0);
	for (index = 0; index < numShips; index++)
	{
		smFocusCommand->ShipPtr[index] = ships[index];
	}
	smFocusCommand->numShips = numShips;
	dbgAssert(!smZoomingOut);
	smZoomingIn = TRUE;

	/* call the sound event for closing the Sensors manager */
	soundEvent(NULL, UI_SensorsExit);

	smFocusTransition = FALSE;
	smEyeEnd = smCamera.eyeposition;
	smLookEnd = smCamera.lookatpoint;
	smLookStart = *centre;
	vecSub(smEyeStart, smLookEnd, smEyeEnd);
	vecNormalize(&smEyeStart);
	dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x >(real32) - 1e19);
	vecMultiplyByScalar(smEyeStart, focusDistance);
	vecSub(smEyeStart, smLookStart, smEyeStart);
	smInitialDistance = smCamera.distance;
	selRectNone();
	smTempCamera.lookatpoint = smLookStart;
	smTempCamera.eyeposition = smEyeStart;
	mrCamera = &smTempCamera;
	univUpdateRenderList();
	dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x >(real32) - 1e19);

	smFocusOnMothershipOnClose = FALSE;
}

void cat_smZoomInOnFocus(void)
{
	vector centre = {0.0f, 0.0f, 0.0f};

	if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bSensorsClose)
	{
		return;
	}

	if (ccFocusCullRadiusMean((FocusCommand*)&attackcom, SM_FocusRadius * SM_FocusRadius, &centre) > 0)
	{
		smStartFocusZoom((Ship**)attackcom.TargetPtr, attackcom.numTargets, &centre, SM_FocusScalar, TRUE);
		attackcom.numTargets = 0;
	}
}


//for selections
rectangle smSelectionRect;
//blob *smLastClosestBlob = NULL;

blob* closestBlob = NULL;
blob* probeBlob = NULL;
//for enabling 'ghost mode' after a player dies
bool32 smGhostMode = FALSE;
bool32 ioSaveState;

//option for fuzzy sensors blobs
sdword smFuzzyBlobs = TRUE;

smblurry smBlurryArray[SM_BlurryArraySize];
sdword smBlurryIndex = 0;

//cursor text font, from mouse.c
extern fonthandle mouseCursorFont;
color smCurrentWorldPlaneColor;
real32 smCurrentCameraDistance;

//switch for 'instant' transitions to sensors manager
bool32 smInstantTransition = FALSE;
real32 smCurrentZoomLength;
real32 smCurrentMainViewZoomLength;

//for horizon tick marks
smticktext smTickText[SM_MaxTicksOnScreen];
sdword smTickTextIndex;

//for panning the world plane about whilley-nilley
vector smCameraLookatPoint = {0.0f, 0.0f, 0.0f};
vector smCameraLookVelocity = {0.0f, 0.0f, 0.0f};
bool32 smCentreWorldPlane = FALSE;

//info for sorting the blobs
blob** smBlobSortList = NULL;
sdword smBlobSortListLength = 0;
sdword smNumberBlobsSorted = 0;

//for multiplayer hyperspace
sdword MP_HyperSpaceFlag = FALSE;

//for sensors weirdness
#define NUM_WEIRD_POINTS         600
#define MAX_REPLACEMENTS         5
#define WEIRDUNIVERSESTRETCH     5/4
typedef struct
{
	vector location;
	color  color;
	real32 fade;
	bool32   ping;
} weirdStruct;

static weirdStruct smWeird[NUM_WEIRD_POINTS];
udword   smSensorWeirdness = 0;

//region for hyperspace button
regionhandle smHyperspaceRegion = NULL;

//what ships have a TO in the SM
ubyte smShipTypeRenderFlags[TOTAL_NUM_SHIPS] =
{
	SM_TO,                                      //AdvanceSupportFrigate
	0,                                          //AttackBomber
	SM_TO | SM_Mesh,                            //Carrier
	0,                                          //CloakedFighter
	SM_TO,                                      //CloakGenerator
	SM_TO,                                      //DDDFrigate
	0,                                          //DefenseFighter
	SM_TO,                                      //DFGFrigate
	SM_TO,                                      //GravWellGenerator
	0,                                          //HeavyCorvette
	SM_TO | SM_Mesh,                            //HeavyCruiser
	0,                                          //HeavyDefender
	0,                                          //HeavyInterceptor
	SM_TO,                                      //IonCannonFrigate
	0,                                          //LightCorvette
	0,                                          //LightDefender
	0,                                          //LightInterceptor
	0,                                          //MinelayerCorvette
	SM_TO,                                      //MissileDestroyer
	SM_Mesh | SM_Exclude,                       //Mothership
	0,                                          //MultiGunCorvette
	SM_Exclude,                                 //Probe
	SM_Exclude,                                 //ProximitySensor
	0,                                          //RepairCorvette
	SM_TO,                                      //ResearchShip
	SM_TO,                                      //ResourceCollector
	SM_TO,                                      //ResourceController
	0,                                          //SalCapCorvette
	SM_TO,                                      //SensorArray
	SM_TO,                                      //StandardDestroyer
	SM_TO,                                      //StandardFrigate
	SM_Exclude,                                 //Drone
	SM_Exclude,                                 //TargetDrone
	SM_Mesh,                                    //HeadShotAsteroid
	SM_Mesh | SM_Exclude,                       //CryoTray
	0,                                          //P1Fighter
	SM_TO,                                      //P1IonArrayFrigate
	0,                                          //P1MissileCorvette
	SM_TO | SM_Mesh,                            //P1Mothership
	0,                                          //P1StandardCorvette
	0,                                          //P2AdvanceSwarmer
	SM_TO,                                      //P2FuelPod
	SM_TO | SM_Mesh,                            //P2Mothership
	SM_TO,                                      //P2MultiBeamFrigate
	0,                                          //P2Swarmer
	SM_TO,                                      //P3Destroyer
	SM_TO,                                      //P3Frigate
	SM_TO | SM_Mesh,                            //P3Megaship
	SM_TO | SM_Mesh,                            //FloatingCity
	SM_TO,                                      //CargoBarge
	SM_TO | SM_Mesh,                            //MiningBase
	SM_TO | SM_Mesh,                            //ResearchStation
	0,                                          //JunkYardDawg
	SM_TO,                                      //JunkYardHQ
	0,                                          //Ghostship
	0,                                          //Junk_LGun
	0,                                          //Junk_SGun
	0,                                          //ResearchStationBridge
	0                                           //ResearchStationTower
};
ubyte smDerelictTypeMesh[NUM_DERELICTTYPES] =
{
	0,                                          //Crate,
	0,                                          //Homeworld,
	0,                                          //PlanetOfOrigin,
	0,                                          //PlanetOfOrigin_scarred,
	0,                                          //Galaxy,
	0,                                          //LavaBall,
	0,                                          //MattWorld,
	0,                                          //HyperspaceGate
};

GLboolean smBigPoints;

/*-----------------------------------------------------------------------------
	tweakables for the sensors manager from sensors.script
-----------------------------------------------------------------------------*/
//for blobs
sdword smBlobUpdateRate = SM_BlobUpdateRate;
real32 smClosestMargin = SM_ClosestMargin;
real32 smFarthestMargin = SM_FarthestMargin;
//for selections
real32 smSelectedFlashSpeed = SM_SelectedFlashSpeed;
real32 smFocusScalar = SM_FocusScalar;
real32 smFocusRadius = SM_FocusRadius;
sdword smSelectionWidth = SM_SelectionWidth;
sdword smSelectionHeight = SM_SelectionHeight;
//zooming in/out
real32 smZoomLength = SM_ZoomLength;
real32 smMainViewZoomLength = SM_MainViewZoomLength;
//for tactical overlay
color smPlayerListTextColor = SM_PlayerListTextColor;
color smCursorTextColor = SM_CursorTextColor;
color smTOColor = SM_TOColor;
sdword smPlayerListTextMargin = SM_PlayerListTextMargin;
sdword smPlayerListMarginX = SM_PlayerListMarginX;
sdword smPlayerListMarginY = SM_PlayerListMarginY;
sdword smTOBottomCornerX = SM_TOBottomCornerX;
sdword smTOBottomCornerY = SM_TOBottomCornerY;
sdword smTOLineSpacing = SM_TOLineSpacing;
real32 smTORadius = 1.0f;
//for world plane
color smWorldPlaneColor = SM_WorldPlaneColor;
color smMovePlaneColor = SM_MovePlaneColor;
color smHeightCircleColor = SM_HeightCircleColor;
real32 smWorldPlaneDistanceFactor = SM_WorldPlaneDistanceFactor;
sdword smWorldPlaneSegments = SM_WorldPlaneSegments;
real32 smMovementWorldPlaneDim = SM_MovementWorldPlaneDim;
real32 smRadialTickStart = SM_RadialTickStart;
real32 smRadialTickInc = SM_RadialTickInc;
real32 smTickInnerMult = SM_TickInnerMult;
real32 smTickOuterMult = SM_TickOuterMult;
real32 smTickExtentFactor = SM_TickExtentFactor;
real32 smBlobCircleSize = SM_BlobCircleSize;
real32 smShortFootLength = SM_ShortFootLength;
real32 smBackgroundDim = SM_BackgroundDim;
//for horizon ticks
real32 smHorizTickAngle = SM_HorizTickAngle;
real32 smHorizTickVerticalFactor = SM_HorizTickVerticalFactor;
real32 smHorizonLineDistanceFactor = SM_HorizonLineDistanceFactor;
real32 smHorizonTickHorizFactor = SM_HorizonTickHorizFactor;
sdword smTickTextSpacing = SM_TickTextSpacing;
//for movement/selections
real32 smTickLength = SM_TickLength;
real32 smClosestDistance = SM_ClosestDistance;
//for rendering
real32 smProjectionScale = SM_ProjectionScale;
//for hot key groups
sdword smHotKeyOffsetX = 0;
sdword smHotKeyOffsetY = 0;
//for panning the world plane about whilley-nilley
real32 smPanSpeedMultiplier = SM_PanSpeedMultiplier;
real32 smPanTrack = SM_PanTrack;
real32 smPanEvalThreshold = SM_PanEvalThreshold;
real32 smPanUnivExtentMult = SM_PanUnivExtentMult;
sdword smCursorPanX = SM_CursorPanX;
sdword smCursorPanY = SM_CursorPanY;
sdword smMaxFrameTicks = SM_MaxFrameTicks;

/*-----------------------------------------------------------------------------
	tweakables for sensors manager from level files
-----------------------------------------------------------------------------*/
//depth cuing
real32 smDepthCueRadius = SM_DepthCueRadius;
real32 smDepthCueStartRadius = SM_DepthCueStartRadius;
real32 smCircleBorder = SM_CircleBorder;
real32 smZoomMax = SM_ZoomMax;
real32 smZoomMin = SM_ZoomMin;
real32 smZoomMinFactor = SM_ZoomMinFactor;
real32 smZoomMaxFactor = SM_ZoomMaxFactor;
real32 smInitialDistance = SM_InitialDistance;
real32 smUniverseSizeX = SM_UniverseSizeX;
real32 smUniverseSizeY = SM_UniverseSizeY;
real32 smUniverseSizeZ = SM_UniverseSizeZ;

static void smZoomInOnShip(Ship* ship)
{
	SelectCommand focusCommand;
	vector centre;

	if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bSensorsClose)
	{
		return;
	}

	focusCommand.numShips = 1;
	focusCommand.timeLastStatus = 0.0f;
	focusCommand.ShipPtr[0] = ship;
	centre = ship->posinfo.position;
	smStartFocusZoom(focusCommand.ShipPtr, focusCommand.numShips, &centre, smFocusScalar, TRUE);
}

/*-----------------------------------------------------------------------------
	Tweakables for the fleet intel version of the SM
-----------------------------------------------------------------------------*/
real32 smSkipFadeoutTime = SM_SkipFadeoutTime;

/*-----------------------------------------------------------------------------
	Tweak table for sensors.script
-----------------------------------------------------------------------------*/
scriptEntry smTweaks[] =
{
	//sensors manager tweaks
	makeEntry(smBlobUpdateRate, scriptSetSdwordCB),
	makeEntry(smSelectedFlashSpeed, scriptSetReal32CB),
	makeEntry(smFocusScalar, scriptSetReal32CB),
	makeEntry(smFocusRadius, scriptSetReal32CB),
	makeEntry(smZoomLength, scriptSetReal32CB),
	makeEntry(smMainViewZoomLength, scriptSetReal32CB),
	makeEntry(smPlayerListTextColor, scriptSetRGBCB),
	makeEntry(smCursorTextColor, scriptSetRGBCB),
	makeEntry(smTOColor, scriptSetRGBCB),
	makeEntry(smWorldPlaneColor, scriptSetRGBCB),
	makeEntry(smPlayerListTextMargin, scriptSetSdwordCB),
	makeEntry(smPlayerListMarginX, scriptSetSdwordCB),
	makeEntry(smPlayerListMarginY, scriptSetSdwordCB),
	makeEntry(smTOBottomCornerX, scriptSetSdwordCB),
	makeEntry(smTOBottomCornerY, scriptSetSdwordCB),
	makeEntry(smTOLineSpacing, scriptSetSdwordCB),
	makeEntry(smTickLength, scriptSetReal32CB),
	makeEntry(smClosestDistance, scriptSetReal32CB),
	makeEntry(smProjectionScale, scriptSetReal32CB),
	makeEntry(smSelectionWidth, scriptSetSdwordCB),
	makeEntry(smSelectionHeight, scriptSetSdwordCB),
	makeEntry(smZoomMaxFactor, scriptSetReal32CB),
	makeEntry(smZoomMinFactor, scriptSetReal32CB),
	makeEntry(smHotKeyOffsetX, scriptSetSdwordCB),
	makeEntry(smHotKeyOffsetY, scriptSetSdwordCB),
	makeEntry(smClosestMargin, scriptSetReal32CB),
	makeEntry(smFarthestMargin, scriptSetReal32CB),
	makeEntry(smBlobCircleSize, scriptSetReal32CB),
	makeEntry(smShortFootLength, scriptSetReal32CB),
	makeEntry(smBlobColor, scriptSetRGBCB),
	makeEntry(smBlobColorHigh, scriptSetRGBCB),
	makeEntry(smBlobColorLow, scriptSetRGBCB),
	makeEntry(smTORadius, scriptSetReal32CB),
	makeEntry(smPanSpeedMultiplier, scriptSetReal32CB),
	makeEntry(smPanTrack, scriptSetReal32CB),
	makeEntry(smPanEvalThreshold, scriptSetReal32CB),
	makeEntry(smCursorPanX, scriptSetSdwordCB),
	makeEntry(smCursorPanY, scriptSetSdwordCB),
	makeEntry(smMaxFrameTicks, scriptSetSdwordCB),
	makeEntry(smSkipFadeoutTime, scriptSetReal32CB),
	makeEntry(smWorldPlaneDistanceFactor, scriptSetReal32CB),
	makeEntry(smPanUnivExtentMult, scriptSetReal32CB),
	//    { "smBobDensityLow",            scriptSetReal32CB, &smBlobProperties.bobDensityLow },
	//    { "smBobDensityHigh",           scriptSetReal32CB, &smBlobProperties.bobDensityHigh },
	//    { "smBobStartSphereSize",       scriptSetReal32CB, &smBlobProperties.bobStartSphereSize },
	//    { "smBobRadiusCombineMargin",   scriptSetReal32CB, &smBlobProperties.bobRadiusCombineMargin },
	//    { "smBobOverlapFactor",         scriptSetReal32CB, &smBlobProperties.bobOverlapFactor },
	//    { "smBobSmallestRadius",         scriptSetReal32CB, &smBlobProperties.bobSmallestRadius },
	//    { "smBobBiggestRadius",         scriptSetReal32CB, &smBlobProperties.bobBiggestRadius },
	//    { "smBobDoingCollisionBobs",    scriptSetBool, &smBlobProperties.bobDoingCollisionBobs },
	{"FOWBobDensityLow", scriptSetReal32CB, &smFOWBlobProperties.bobDensityLow},
	{"FOWBobDensityHigh", scriptSetReal32CB, &smFOWBlobProperties.bobDensityHigh},
	{"FOWBobStartSphereSize", scriptSetReal32CB, &smFOWBlobProperties.bobStartSphereSize},
	{"FOWBobRadiusCombineMargin", scriptSetReal32CB, &smFOWBlobProperties.bobRadiusCombineMargin},
	{"FOWBobOverlapFactor", scriptSetBlobPropertyOverlap, &smFOWBlobProperties},
	{"FOWBobSmallestRadius", scriptSetReal32CB, &smFOWBlobProperties.bobSmallestRadius},
	{"FOWBobBiggestRadius", scriptSetBlobBiggestRadius, &smFOWBlobProperties},
	//    { "FOWBobDoingCollisionBobs",   scriptSetBool, &smFOWBlobProperties.bobDoingCollisionBobs },
	{"FOW_AsteroidK1", scriptSetReal32CB, &smFOW_AsteroidK1},
	{"FOW_AsteroidK2", scriptSetReal32CB, &smFOW_AsteroidK2},
	{"FOW_AsteroidK3", scriptSetReal32CB, &smFOW_AsteroidK3},
	{"FOW_AsteroidK4", scriptSetReal32CB, &smFOW_AsteroidK4},
	{"FOW_DustGasK2", scriptSetReal32CB, &smFOW_DustGasK2},
	{"FOW_DustGasK3", scriptSetReal32CB, &smFOW_DustGasK3},
	{"FOW_DustGasK4", scriptSetReal32CB, &smFOW_DustGasK4},
	{"smFOWBlobUpdateTime", scriptSetReal32CB, &smFOWBlobUpdateTime},
	endEntry
};

/*=============================================================================
	Private Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
	Name        : smStrchr
	Description : Like strchr but returns NULL when searching for a NULL terminator.
	Inputs      :
	Outputs     :
	Return      : NULL
----------------------------------------------------------------------------*/
void* smStrchr(char* string, char character)
{
	while (*string)
	{
		if (character == *string)
		{
			return(string);
		}
		string++;
	}
	return(NULL);
}

#define SM_NumberTickMarkSpacings       4
#define SM_TickSwitchHysteresis         0.9f
#define SM_SpokeLengthFactor            1.05f
typedef struct
{
	real32 radius;                              //'on' radius of the ticks
	real32 spacing;                             //spacing between ticks, meters
	real32 length;                              //factor of the circle radius
}
ticklod;
real32 smTickSwitchHysteresis = SM_TickSwitchHysteresis;
real32 smSpokeLengthFactor = SM_SpokeLengthFactor;
ticklod smTickMarkInfo[SM_NumberTickMarkSpacings] =
{
	{100000.0f, 20000.0f, 0.025f},
	{50000.0f, 10000.0f, 0.015f},
	{10000.0f, 2000.0f, 0.007f},
};
/*-----------------------------------------------------------------------------
	Name        : smWorldPlaneDraw
	Description : Draw the sensors manager world plane.
	Inputs      : z - height to draw it at so we can use same drawing code for
					the movement pie plate.
				  c - color to use for drawing the beast
				  bDrawSpokes - true if we are to draw spokes
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
static real32 smTickFactorX[4] = {0.0f, 1.0f, 0.0f, -1.0f};
static real32 smTickFactorY[4] = {1.0f, 0.0f, -1.0f, 0.0f};
void smWorldPlaneDraw(vector* centre, bool32 bDrawSpokes, color c)
{
	real32 radius = smCamera.distance * smWorldPlaneDistanceFactor;
	real32 tickExtent = radius * smTickExtentFactor;
	real32 ringRadius;
	real32 angle, sinTheta, cosTheta;
	sdword spoke, LOD, thisLOD;
	vector spokeRim, tickEnd0, tickEnd1, p0, p1;
	real32 tickFactorX = 0.0f, tickFactorY = 1.0f, tickRadius;

	//draw the circle
	rndTextureEnable(FALSE);
	ringRadius = (smZoomMin + smZoomMax) / 2.0f * smWorldPlaneDistanceFactor;
	primCircleOutlineZ(centre, ringRadius, smWorldPlaneSegments, c);

	//draw the radial ticks
	tickEnd0.z = tickEnd1.z = centre->z;
	for (angle = smRadialTickStart; angle < 2.0f * PI; angle += smRadialTickInc)
	{
		sinTheta = (real32)sin((double)angle);
		cosTheta = (real32)cos((double)angle);
		tickEnd0.x = tickEnd1.x = ringRadius * sinTheta;
		tickEnd0.y = tickEnd1.y = ringRadius * cosTheta;
		tickEnd0.x *= smTickInnerMult;
		tickEnd0.y *= smTickInnerMult;
		tickEnd1.x *= smTickOuterMult;
		tickEnd1.y *= smTickOuterMult;
		vecAdd(p0, *centre, tickEnd0);
		vecAdd(p1, *centre, tickEnd1);
		primLine3Smooth(&p0, &p1, c);
	}
	//draw the spokes with gradations, if applicable
	if (bDrawSpokes)
	{
		spokeRim.z = centre->z;

		for (LOD = 0; LOD < SM_NumberTickMarkSpacings; LOD++)
		{
			if (smTickMarkInfo[LOD].radius <= radius)
			{
				break;
			}
		}
		for (spoke = 0; spoke < 4; spoke++)
		{
			tickFactorX = smTickFactorX[spoke];
			tickFactorY = smTickFactorY[spoke];
			spokeRim.y = spokeRim.x = ringRadius * smSpokeLengthFactor;
			spokeRim.x *= tickFactorX;
			spokeRim.y *= tickFactorY;
			vecAdd(p0, *centre, spokeRim);
			primLine3Smooth(centre, &p0, c);

			for (thisLOD = 0; thisLOD < LOD; thisLOD++)
			{
				tickRadius = smTickMarkInfo[thisLOD].spacing;
				while (tickRadius <= tickExtent)
				{
					tickEnd0.x = radius * smTickMarkInfo[thisLOD].length * tickFactorX;
					tickEnd0.y = radius * smTickMarkInfo[thisLOD].length * tickFactorY;
					tickEnd1.x = -tickEnd0.x;
					tickEnd1.y = -tickEnd0.y;
					tickEnd0.x += tickRadius * tickFactorY;
					tickEnd0.y += tickRadius * tickFactorX;
					tickEnd1.x += tickRadius * tickFactorY;
					tickEnd1.y += tickRadius * tickFactorX;
					vecAdd(p0, *centre, tickEnd0);
					vecAdd(p1, *centre, tickEnd1);
					primLine3Smooth(&p0, &p1, c);
					tickRadius += smTickMarkInfo[thisLOD].spacing;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	Name        : makeShipsNotBeDisabled
	Description : removes disabled ships from selection
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/

void makeShipsNotBeDisabled(SelectCommand* selection)
{
	sdword i;
	for (i = 0; i < selection->numShips; i++)
	{
		if (selection->ShipPtr[0]->flags & SOF_Disabled)
		{
			selection->numShips--;
			selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
			i--;
		}
	}
}

/*-----------------------------------------------------------------------------
	Name        : smHorizonLineDraw
	Description : Draw the horizon line with compass tick marks
	Inputs      : cam - context we're rendering from
				  thisBlob - blob we're rendering
				  modelView, projection - current matrices
				  distance - the distance to draw the ring at
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
#define cam     ((Camera *)voidCam)
void smHorizonLineDraw(void* voidCam, hmatrix* modelView, hmatrix* projection, real32 distance)
{
	real32 startAngle, endAngle;
	real32 x, y, radius;
	vector startPoint, horizPoint, endPoint;
	real32 angle, degAngle;
	bool32 isEnabled = FALSE;
	sdword nPrintfed;
	fonthandle oldFont = fontCurrentGet();
	//    distance = smCurrentCameraDistance; //!!!
	distance *= smWorldPlaneDistanceFactor;

	startAngle = cam->angle - PI - DEG_TO_RAD(cam->fieldofview) / 2.0f - smHorizTickAngle * 2.0f;
	endAngle = cam->angle - PI + DEG_TO_RAD(cam->fieldofview) / 2.0f + smHorizTickAngle * 2.0f;

	angle = (real32)floor((double)(startAngle / smHorizTickAngle)) * smHorizTickAngle;
	startPoint.x = cam->lookatpoint.x + (real32)cos((double)angle) * distance;   //position of starting point
	startPoint.y = cam->lookatpoint.y + (real32)sin((double)angle) * distance;
	endPoint.z = startPoint.z = cam->lookatpoint.z;

	fontMakeCurrent(mouseCursorFont);

	if (glIsEnabled(GL_DEPTH_TEST))
	{
		isEnabled = TRUE;
		glDisable(GL_DEPTH_TEST);
	}
	for (smTickTextIndex = 0; angle < endAngle; angle += smHorizTickAngle)
	{
		endPoint.x = cam->lookatpoint.x + (real32)cos((double)(angle + smHorizTickAngle)) * distance;//position of current point
		endPoint.y = cam->lookatpoint.y + (real32)sin((double)(angle + smHorizTickAngle)) * distance;

		primLine3(&startPoint, &endPoint, smCurrentWorldPlaneColor);//draw the vertical tick
		horizPoint.x = endPoint.x;
		horizPoint.y = endPoint.y;
		if (cam->eyeposition.z > cam->lookatpoint.z)
		{
			horizPoint.z = cam->lookatpoint.z + cam->clipPlaneFar * smHorizTickVerticalFactor;
		}
		else
		{
			horizPoint.z = cam->lookatpoint.z - cam->clipPlaneFar * smHorizTickVerticalFactor;
		}

		//figure out where to draw the tick text
		selCircleComputeGeneral(modelView, projection, &horizPoint, 1.0f, &x, &y, &radius);
		dbgAssert(smTickTextIndex < SM_MaxTicksOnScreen);
		smTickText[smTickTextIndex].x = primGLToScreenX(x);
		smTickText[smTickTextIndex].y = primGLToScreenY(y);
		degAngle = -RAD_TO_DEG(angle - smHorizTickAngle);
		if (degAngle < 0.0f)
		{
			degAngle += 360.0f;
		}
		if (degAngle > 360.0f)
		{
			degAngle -= 360.0f;
		}
		if (degAngle < 10)
		{
			nPrintfed = sprintf(smTickText[smTickTextIndex].text, "00%.0f", degAngle);
		}
		else if (degAngle < 100)
		{
			nPrintfed = sprintf(smTickText[smTickTextIndex].text, "0%.0f", degAngle);
		}
		else
		{
			nPrintfed = sprintf(smTickText[smTickTextIndex].text, "%.0f", degAngle);
		}
		dbgAssert(nPrintfed < SM_TickTextChars);
		if (cam->eyeposition.z > cam->lookatpoint.z)
		{
			smTickText[smTickTextIndex].y -= fontHeight(" ") + smTickTextSpacing;
		}
		else
		{
			smTickText[smTickTextIndex].y += smTickTextSpacing;
		}
		smTickText[smTickTextIndex].x -= fontWidth(smTickText[smTickTextIndex].text) / 2;
		smTickTextIndex++;
		//draw the horizontal tick
		primLine3(&endPoint, &horizPoint, smCurrentWorldPlaneColor);
		horizPoint.x = (endPoint.x - cam->lookatpoint.x) * smHorizonTickHorizFactor + cam->lookatpoint.x;
		horizPoint.y = (endPoint.y - cam->lookatpoint.y) * smHorizonTickHorizFactor + cam->lookatpoint.y;
		horizPoint.z = cam->lookatpoint.z;
		primLine3(&endPoint, &horizPoint, smCurrentWorldPlaneColor);
		startPoint = endPoint;                              //draw from this point to next point next time through
	}
	if (isEnabled)
	{
		glEnable(GL_DEPTH_TEST);
	}
	fontMakeCurrent(oldFont);
}
#undef cam

/*-----------------------------------------------------------------------------
	Name        : smTickTextDraw
	Description : Draw the tick text created in smHorizonLineDraw
	Inputs      :
	Outputs     :
	Return      : void
	Note        : must be in 2d mode for this function
----------------------------------------------------------------------------*/
void smTickTextDraw(void)
{
	fonthandle oldFont = fontCurrentGet();

	fontMakeCurrent(mouseCursorFont);

	for (smTickTextIndex--; smTickTextIndex >= 0; smTickTextIndex--)
	{
		fontPrint(smTickText[smTickTextIndex].x, smTickText[smTickTextIndex].y,
			smCurrentWorldPlaneColor, smTickText[smTickTextIndex].text);
	}
	fontMakeCurrent(oldFont);
}

sdword smFindNearestDefaultTargetForView(Ship* sourceShip, sdword mode)
{
	Player* curPlayer;
	Node* node;
	Ship* nearestTarget;
	real32 bestDistanceSquared;

	curPlayer = universe.curPlayerPtr;
	nearestTarget = NULL;
	bestDistanceSquared = 1.0e21f;
	if (curPlayer == NULL || curPlayer->fogGrid == NULL || universe.ShipList.head == NULL)
	{
		return 0;
	}

	for (node = universe.ShipList.head; node != NULL; node = node->next)
	{
		Ship* target;
		bool32 includeTarget;
		vector delta;
		real32 distanceSquared;

		target = (Ship*)node->structptr;
		if (target == sourceShip || (target->flags & (SOF_Cloaking | SOF_DeCloaking | SOF_Dead)) != 0)
		{
			continue;
		}

		includeTarget = FALSE;
		switch (mode)
		{
			case 0:
				includeTarget = shipCanBeSpecialTargeted(target) &&
					!fogCheckPositionVisible(&target->posinfo.position, FALSE);
				break;
			case 1:
				includeTarget = univIsObjFriendly((SpaceObj*)target, curPlayer);
				break;
			case 2:
				if (!univIsObjFriendly((SpaceObj*)target, curPlayer) &&
					!smIsCrystal(target) &&
					!smShipIsBeastShip(target))
				{
					includeTarget = !fogCheckPositionVisible(&target->posinfo.position, FALSE);
				}
				break;
			case 3:
				includeTarget = !univIsObjFriendly((SpaceObj*)target, curPlayer) &&
					!smIsCrystal(target) &&
					!smShipIsBeastShip(target) &&
					acvCanTargetShipForEMP(target, NULL) &&
					!fogCheckPositionVisible(&target->posinfo.position, FALSE);
				break;
		}

		if (!includeTarget)
		{
			continue;
		}

		vecSub(delta, target->posinfo.position, sourceShip->posinfo.position);
		distanceSquared = vecMagnitudeSquared(delta);
		if (distanceSquared < bestDistanceSquared)
		{
			bestDistanceSquared = distanceSquared;
			nearestTarget = target;
		}
		else if (mode == 1 &&
			nearestTarget != NULL &&
			distanceSquared - bestDistanceSquared < 1000000.0f &&
			smShipHealthRatio(target) < smShipHealthRatio(nearestTarget))
		{
			bestDistanceSquared = distanceSquared;
			nearestTarget = target;
		}
	}

	return (sdword)nearestTarget;
}

bool32 smSensorsActiveGet(void)
{
	return smOverlayIsOpen;
}

/*-----------------------------------------------------------------------------
	Name        : smBlobDrawClear
	Description : Render all the ships inside a blob.  This would be for blobs
					with player ships or when player has full sensors researched.
	Inputs      : camera - POV to render from
				  thisBlob - blob we're rendering
				  modelView, projection - current matrices
				  background - color of the blob being drawn to
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
//render utility functions
//bool8 rndShipVisible(SpaceObj* spaceobj, Camera* camera);
//string of ship types to be rendered as meshes
void smBlobDrawClear(Camera* camera, blob* thisBlob, hmatrix* modelView, hmatrix* projection, color background)
{
	sdword index, i;
	SpaceObj** objPtr, * obj;
	color c;
	lod* level;
	hmatrix coordMatrixForGL;
	ShipStaticInfo* shipStaticInfo;
	bool32 bFlashOn;
	SpaceObjSelection* blobObjects = thisBlob->blobObjects;
	real32 radius;
	hvector objectPos, cameraSpace, screenSpace;
	smblurry* blurry;
	sdword nShipTOs = 0;
	struct
	{
		real32 x, y;
		real32 radius;
		color c;
		ShipClass shipClass;
	}
	shipTO[SM_NumberTOs];
	sdword nBigDots = 0;
	struct
	{
		real32 x, y;
		color c;
	}
	bigDot[SM_NumberBigDots];
	real32 pointSize;
	rectangle rect;

	//flash the selected ships
	if (fmod((double)universe.totaltimeelapsed, (double)smSelectedFlashSpeed) > smSelectedFlashSpeed / 2.0)
	{
		bFlashOn = TRUE;
	}
	else
	{
		bFlashOn = FALSE;
	}
	//draw all objects in the sphere
	for (index = 0, objPtr = blobObjects->SpaceObjPtr; index < blobObjects->numSpaceObjs; index++, objPtr++)
	{
		obj = *objPtr;
		//#ifdef DEBUG_COLLBLOBS
		if (obj->flags & SOF_Targetable)
			//#endif
		{
			selCircleCompute(modelView, projection, (SpaceObjRotImpTarg*)obj);//compute selection circle
		}
		switch (obj->objtype)
		{
			case OBJ_ShipType:
				if (smNonResources == FALSE)
				{
					break;
				}
				if (bitTest(((Ship*)obj)->flags, SOF_Hide))
				{                                           //don't show hidden ships
					break;
				}
				if (!allianceArePlayersAllied(((Ship*)obj)->playerowner, universe.curPlayerPtr) &&
					!smShipVisibleToPlayer((Ship*)obj))
				{
					break;
				}
				if (!smShipAllowedBySensorsSettings((Ship*)obj))
				{
					break;
				}
				if (smTacticalOverlay)
				{
					if (showCapitalShipOverlay &&
						((Ship*)obj)->playerowner == universe.curPlayerPtr &&
						bitTest(smShipTypeRenderFlags[((Ship*)obj)->shiptype], SM_TO))
					{
						if (nShipTOs < SM_NumberTOs)
						{
							shipTO[nShipTOs].x = ((Ship*)obj)->collInfo.selCircleX;
							shipTO[nShipTOs].y = ((Ship*)obj)->collInfo.selCircleY;
							shipTO[nShipTOs].radius = ((Ship*)obj)->collInfo.selCircleRadius;
							shipTO[nShipTOs].c = colRGB(colRed(c) / TO_IconColorFade, colGreen(c) / TO_IconColorFade, colBlue(c) / TO_IconColorFade);
							shipTO[nShipTOs].shipClass = ((Ship*)obj)->staticinfo->shipclass;
							nShipTOs++;
						}
					}
					if (((Ship*)obj)->playerowner == universe.curPlayerPtr)
					{
						if (ShipTypeIsGravWellGeneratorType(((Ship*)obj)->shiptype))
						{
							if (((GravWellGeneratorSpec*)((Ship*)obj)->ShipSpecifics)->GravFieldOn)
							{
								toFieldSphereDraw(((Ship*)obj), ((GravWellGeneratorStatics*)((ShipStaticInfo*)(((Ship*)obj)->staticinfo))->custstatinfo)->GravWellRadius, 1.0f);
							}
						}
					}
				}
				if (bitTest(smShipTypeRenderFlags[((Ship*)obj)->shiptype], SM_Mesh))
				{                                           //if it's a ship type to render as a mesh
					level = lodLevelGetSimple((void*)obj, &camera->eyeposition, &obj->posinfo.position);
					if ((level->flags & LM_LODType) == LT_Mesh)
					{
						if (!(bFlashOn && smShipIsSelected((Ship*)obj)))
						{
							if (!bitTest(obj->flags, SOF_Cloaked) || ((Ship*)obj)->playerowner == universe.curPlayerPtr)
							{       //if ship isn't cloaked, draw, or if ship is players, draw
								glPushMatrix();
								shipStaticInfo = (ShipStaticInfo*)obj->staticinfo;
								hmatMakeHMatFromMat(&coordMatrixForGL, &((SpaceObjRot*)obj)->rotinfo.coordsys);
								hmatPutVectIntoHMatrixCol4(obj->posinfo.position, coordMatrixForGL);
								glMultMatrixf((float*)&coordMatrixForGL);//ship's rotation matrix
								rndLightingEnable(TRUE);
								shPushLightMatrix(&coordMatrixForGL);
								i = ((Ship*)obj)->colorScheme;

								if (rndShipVisible(obj, camera))
								{
									bool32 wireOn;
									extern bool8 g_WireframeHack;

									wireOn = g_WireframeHack;
									g_WireframeHack = FALSE;
									if (((Ship*)obj)->bindings != NULL)
									{
										meshRenderShipHierarchyForShip((Ship *)obj,
											((Ship*)obj)->bindings,
											((Ship*)obj)->currentLOD,
											(meshdata*)level->pData, i);
									}
									else
									{
										meshRender((meshdata*)level->pData, i);
									}
									g_WireframeHack = (ubyte)wireOn;
								}
								shPopLightMatrix();
								glDisable(GL_BLEND);
								glDisable(GL_ALPHA_TEST);
								rndLightingEnable(FALSE);
								rndTextureEnable(FALSE);
								glPopMatrix();
							}
						}
					}
					else
					{                                       //else no LOD at this level
						goto justRenderAsDot;
					}
				}                                           //else draw it as a pixel
				else if ((!bitTest(obj->flags, SOF_Cloaked) || allianceIsShipAlly((Ship*)obj, universe.curPlayerPtr)) &&
					((obj->attributes & ATTRIBUTES_SMColorField) != ATTRIBUTES_SMColorInvisible))
				{       //if ship isn't cloaked, draw, or if ship is players, draw
				justRenderAsDot:
#if TO_STANDARD_COLORS
					c = cat_smShipPointColor((Ship*)obj);
					if (obj->attributes & ATTRIBUTES_SMColorField)  // overide with special colors
					{
						if ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorYellow)
						{
							c = teAlliedColor;
						}
						else if ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorGreen)
						{
							c = teFriendlyColor;
						}
					}
#else //SM_STANDARD_COLORS
					c = teColorSchemes[((Ship*)obj)->colorScheme].tacticalColor;
#endif //SM_STANDARD_COLORS
					if (smTacticalOverlay)
					{
						if (showCapitalShipOverlay &&
							((Ship*)obj)->playerowner == universe.curPlayerPtr &&
							bitTest(smShipTypeRenderFlags[((Ship*)obj)->shiptype], SM_TO))
							//if (((Ship *)obj)->shiptype == ResourceCollector)
						{
							if (nShipTOs < SM_NumberTOs)
							{
								shipTO[nShipTOs].x = ((Ship*)obj)->collInfo.selCircleX;
								shipTO[nShipTOs].y = ((Ship*)obj)->collInfo.selCircleY;
								shipTO[nShipTOs].radius = ((Ship*)obj)->collInfo.selCircleRadius;
								shipTO[nShipTOs].c = colRGB(colRed(c) / TO_IconColorFade, colGreen(c) / TO_IconColorFade, colBlue(c) / TO_IconColorFade);
								shipTO[nShipTOs].shipClass = ((Ship*)obj)->staticinfo->shipclass;
								nShipTOs++;
							}
						}
					}
					if (smShipIsSelected((Ship*)obj))
					{
						if (bFlashOn)
						{
							c = background;
						}
					}

					//                    if ((((Ship *)obj)->playerowner->playerIndex == universe.curPlayerIndex))
					//                    {                                       //enable double-size points
					pointSize = 2.0f;
					//                        //glPointSize(2.0f);
					//                    }
					//                    else
					//                    {
					//                        pointSize = 1.0f;
					//                        //glPointSize(1.0f);
					//                    }
					rndTextureEnable(FALSE);
					if ((!smBigPoints) && pointSize != 1.0f && ((Ship*)obj)->collInfo.selCircleRadius > 0.0f)
					{
						dbgAssert(nBigDots < SM_NumberBigDots);
						bigDot[nBigDots].x = ((Ship*)obj)->collInfo.selCircleX;
						bigDot[nBigDots].y = ((Ship*)obj)->collInfo.selCircleY;
						bigDot[nBigDots].c = c;
						nBigDots++;
					}
					else
					{
						glPointSize(pointSize);
						primPoint3(&obj->posinfo.position, c);  //everything is rendered as a point
						glPointSize(1.0f);
					}
				}
				break;
			case OBJ_AsteroidType:
				if (showResources == FALSE)
				{
					break;
				}
				radius = obj->staticinfo->staticheader.staticCollInfo.collspheresize;

				if (radius > SM_LargeResourceSize)
				{
					pointSize = 2.0f;
					//glPointSize(2.0f);
				}
				rndTextureEnable(FALSE);
#if TO_STANDARD_COLORS
				c = teResourceColor;
#else
				c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
				if ((!smBigPoints) && pointSize != 1.0f && ((Ship*)obj)->collInfo.selCircleRadius > 0.0f)
				{
					dbgAssert(nBigDots < SM_NumberBigDots);
					bigDot[nBigDots].x = ((Ship*)obj)->collInfo.selCircleX;
					bigDot[nBigDots].y = ((Ship*)obj)->collInfo.selCircleY;
					bigDot[nBigDots].c = c;
					nBigDots++;
				}
				else
				{
					glPointSize(pointSize);
					primPoint3(&obj->posinfo.position, c);     //everything is rendered as a point
					glPointSize(1.0f);
				}
				pointSize = 1.0f;
				break;
			case OBJ_NebulaType:
			case OBJ_GasType:
			case OBJ_DustType:
				if (showResources == FALSE)
				{
					break;
				}
				memcpy(&objectPos, &obj->posinfo.position, sizeof(vector));
				objectPos.w = 1.0f;
				//                c = obj->staticinfo->staticheader.LOD->pointColor;
				//                c = colWhite;
				hmatMultiplyHMatByHVec(&cameraSpace, modelView, &objectPos);//in camera space
				hmatMultiplyHMatByHVec(&screenSpace, projection, &cameraSpace);//in screen space
				//                primBlurryPoint22(primGLToScreenX(screenSpace.x / screenSpace.w),    //everything is rendered as a blurry point
				//                                 primGLToScreenY(screenSpace.y / screenSpace.w), c);
				if (screenSpace.z > 0.0f && smBlurryIndex < SM_BlurryArraySize)
				{
#if TO_STANDARD_COLORS
					smBlurryArray[smBlurryIndex].c = teResourceColor;
#else
					smBlurryArray[smBlurryIndex].c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
					smBlurryArray[smBlurryIndex].x = primGLToScreenX(screenSpace.x / screenSpace.w);
					smBlurryArray[smBlurryIndex].y = primGLToScreenY(screenSpace.y / screenSpace.w);
					smBlurryIndex++;
				}
				break;
			case OBJ_DerelictType:
				if (((Derelict*)obj)->derelicttype == HyperspaceGate)
				{
					goto renderDerelictAsDot;
				}
				level = lodLevelGet((void*)obj, &camera->eyeposition, &obj->posinfo.position);
				if (bitTest(smDerelictTypeMesh[((Derelict*)obj)->derelicttype], SM_Mesh) && (level->flags & LM_LODType) == LT_Mesh)
				{
					if ((!bitTest(obj->flags, SOF_Cloaked)) &&
						((obj->attributes & ATTRIBUTES_SMColorField) != ATTRIBUTES_SMColorInvisible))
					{       //if it isn't cloaked, draw
						glPushMatrix();
						shipStaticInfo = (ShipStaticInfo*)obj->staticinfo;
						hmatMakeHMatFromMat(&coordMatrixForGL, &((SpaceObjRot*)obj)->rotinfo.coordsys);
						hmatPutVectIntoHMatrixCol4(obj->posinfo.position, coordMatrixForGL);
						glMultMatrixf((float*)&coordMatrixForGL);//ship's rotation matrix
						rndLightingEnable(TRUE);
						shPushLightMatrix(&coordMatrixForGL);
						i = ((Derelict*)obj)->colorScheme;

						if (rndShipVisible(obj, camera))
						{
							bool32 wireOn;
							extern bool8 g_WireframeHack;

							wireOn = g_WireframeHack;
							g_WireframeHack = FALSE;
							meshRender((meshdata*)level->pData, i);
							g_WireframeHack = (ubyte)wireOn;
						}
						shPopLightMatrix();
						glDisable(GL_BLEND);
						glDisable(GL_ALPHA_TEST);
						rndLightingEnable(FALSE);
						rndTextureEnable(FALSE);
						glPopMatrix();
					}
				}
				else
				{
				renderDerelictAsDot:
#if TO_STANDARD_COLORS
					c = teNeutralColor;
#else
					c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
					//PrisonShipOld no longer exists in Cataclysm's DerelictType enum

					rndTextureEnable(FALSE);
					primPoint3(&obj->posinfo.position, c);   //everything is rendered as a point
				}
				break;

			default:
				break;
		}
	}

	//display the nebulae tendrils as strings
	rndTextureEnable(FALSE);
	rndLightingEnable(FALSE);

	if (glCapFastFeature(GL_BLEND))
	{
		glEnable(GL_BLEND);
		rndAdditiveBlends(FALSE);
	}
	else
	{
		glDisable(GL_BLEND);
	}
	glShadeModel(GL_FLAT);
	for (index = 0; (udword)index < numNebulae; index++)
	{
		sdword t;
		nebulae_t* neb = &nebNebulae[index];

		glBegin(GL_LINES);
		for (t = 0; (udword)t < neb->numTendrils; t++)
		{
			color c = neb->tendrilTable[t].colour;
			glColor4ub(colRed(c), colGreen(c), colBlue(c), 192);
			glVertex3fv((GLfloat*)&neb->tendrilTable[t].a->position);
			glVertex3fv((GLfloat*)&neb->tendrilTable[t].b->position);
		}
		glEnd();
	}
	glDisable(GL_BLEND);

	if (smBlurryIndex > 0 || nShipTOs > 0 || nBigDots > 0)
	{
		toicon* icon;
		color col;
		real32 radius;

		primModeSet2();
		for (blurry = smBlurryArray; smBlurryIndex > 0; smBlurryIndex--, blurry++)
		{
			primBlurryPoint22(blurry->x, blurry->y, blurry->c);
		}

		for (index = 0; index < nShipTOs; index++)
		{
			icon = toClassIcon[shipTO[index].shipClass];
			toClassUsed[shipTO[index].shipClass][0] = TRUE;

			if (shipTO[index].radius > 0.0f)
			{
				radius = max(shipTO[index].radius, smTORadius);
				col = shipTO[index].c;
				primLineLoopStart2(1, col);

				for (i = icon->nPoints - 1; i >= 0; i--)
				{
					primLineLoopPoint3F(shipTO[index].x + icon->loc[i].x * radius,
						shipTO[index].y + icon->loc[i].y * radius);
				}
			}
			primLineLoopEnd2();

		}
		for (index = 0; index < nBigDots; index++)
		{
			rect.x0 = primGLToScreenX(bigDot[index].x);
			rect.y0 = primGLToScreenY(bigDot[index].y);
			rect.x1 = rect.x0 + 2;
			rect.y1 = rect.y0 + 2;
			primRectSolid2(&rect, bigDot[index].c);
		}
		primModeClear2();
		rndLightingEnable(FALSE);
		rndTextureEnable(FALSE);
	}
}

/*-----------------------------------------------------------------------------
	Name        : SensorsWeirdnessUtilities
	Description : Utility functions for the sensors weirdness in the single player game
	Inputs      : various
	Outputs     : various
	Return      : various
----------------------------------------------------------------------------*/
//generates a random location within (and slightly outside) the bounds of the universe
vector smwGenerateRandomPoint(void)
{
	vector vec;

	vec.x = (real32)frandyrandombetween(RAN_AIPlayer, -smUniverseSizeX * WEIRDUNIVERSESTRETCH, smUniverseSizeX * WEIRDUNIVERSESTRETCH);
	vec.y = (real32)frandyrandombetween(RAN_AIPlayer, -smUniverseSizeY * WEIRDUNIVERSESTRETCH, smUniverseSizeY * WEIRDUNIVERSESTRETCH);
	vec.z = (real32)frandyrandombetween(RAN_AIPlayer, -smUniverseSizeZ * WEIRDUNIVERSESTRETCH, smUniverseSizeZ * WEIRDUNIVERSESTRETCH);

	return vec;
}

//generates a random color for the point
color smwGenerateRandomColor(void)
{
	udword colornum;
	color  col;

	colornum = (udword)randyrandom(RAN_AIPlayer, 7);

	switch (colornum)
	{
		case 0:
			//            col = teFriendlyColor;
			//            break;
		case 1:
		case 2:
		case 3:
			col = teHostileColor;
			break;
		case 4:
		case 5:
		case 6:
		default:
			col = teResourceColor;
			break;
	}
	return col;
}

void smwGeneratePing(vector location)
{
	sdword pingcnt;
	char name[20] = "0";

	//very small chance of a new ping
	if (((udword)randyrandom(RAN_AIPlayer, 200)) <= 1)
	{
		pingcnt = randyrandom(RAN_AIPlayer, 3);

		sprintf(name, "Weirdness%i", pingcnt);
		pingAnomalyPingRemove(name);
		pingAnomalyPositionPingAdd(name, &location);
	}
}


/*-----------------------------------------------------------------------------
	Name        : smSensorWeirdnessDraw
	Description : Draws the sensor manager malfunction in Missions 7 and 8
	Inputs      :
	Outputs     : Draws a bunch of weirdness on the screen
	Return      : void
----------------------------------------------------------------------------*/
void smSensorWeirdnessDraw(hmatrix* modelView, hmatrix* projection)
{
	udword i, j, num_replaces;
	static udword num_points;
	static udword redraw = 0;
	vector tempvec;
	rectangle rect;

	if (!redraw)
	{
		num_points = (udword)randyrandom(RAN_AIPlayer, (smSensorWeirdness / 3));
		num_points += (2 * smSensorWeirdness / 3);
		redraw = randyrandombetween(RAN_AIPlayer, 20, 30);
	}
	else
	{
		redraw--;
	}

	num_replaces = (udword)randyrandom(RAN_AIPlayer, MAX_REPLACEMENTS);

	for (i = 0; i < num_replaces; i++)
	{
		//chose a random point
		j = (udword)randyrandom(RAN_AIPlayer, smSensorWeirdness);

		//generate a random location for it
		smWeird[j].location = smwGenerateRandomPoint();
		smWeird[j].color = smwGenerateRandomColor();
		smWeird[j].fade = 0.0f;
		smwGeneratePing(smWeird[j].location);
	}


	primModeSet2();
	//    rndGLStateLog("RocksAreBig");
	for (i = 0; i < num_points; i++)
	{
		transSingleTotalTransform(&tempvec, modelView, projection, &smWeird[i].location);
		rect.x0 = primGLToScreenX(tempvec.x);
		rect.y0 = primGLToScreenY(tempvec.y);
		rect.x1 = rect.x0 + 2;
		rect.y1 = rect.y0 + 2;
		primRectSolid2(&rect, smWeird[i].color);
	}
	primModeClear2();
}

/*-----------------------------------------------------------------------------
	Name        : smSensorsWeirdnessInit
	Description : Initializes the sensors weirdness stuff
	Inputs      :
	Outputs     : Fills the smWeirdCol and smWeirdLoc arrays
	Return      : void
----------------------------------------------------------------------------*/
void smSensorWeirdnessInit(void)
{
	udword i;

	for (i = 0; i < NUM_WEIRD_POINTS; i++)
	{
		smWeird[i].location = smwGenerateRandomPoint();
		smWeird[i].color = smwGenerateRandomColor();
		smWeird[i].ping = FALSE;
	}
}



/*-----------------------------------------------------------------------------
	Name        : smEnemyFogOfWarBlobCallback
	Description : Sub-blob creation callback for creating fog-of-war sub-blobs
	Inputs      : superBlob - the blob we're looking in
				  obj - object to evaluate for inclusion
	Outputs     :
	Return      : TRUE if it's an enemy ship or a foggy resource
----------------------------------------------------------------------------*/
bool32 smEnemyFogOfWarBlobCallback(blob* superBlob, SpaceObj* obj)
{
	switch (obj->objtype)
	{
		case OBJ_ShipType:                                  //make sure it's an enemy ship
			if (allianceIsShipAlly((Ship*)obj, universe.curPlayerPtr))
			{                                               //if this is an allied ship (alliance formed while in SM)
				return(FALSE);
			}
			if (((Ship*)obj)->flags & (SOF_Hide | SOF_Cloaked))
			{                                               //don't show hidden or cloaked ships
				return(FALSE);
			}
			if ((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorInvisible)
			{
				return(FALSE);
			}
			return(TRUE);
		case OBJ_BulletType:
			break;
		case OBJ_AsteroidType:
			return(TRUE);
		case OBJ_NebulaType:
			break;
		case OBJ_GasType:
		case OBJ_DustType:
			return(TRUE);
		case OBJ_DerelictType:
		case OBJ_EffectType:
		case OBJ_MissileType:
			break;
	}
	return(FALSE);
}

/*-----------------------------------------------------------------------------
	Name        : smBlobDrawCloudy
	Description : Draw a cloudy sensors blob, such as one in the shroud.
	Inputs      : same as for the clear draw
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
#define SM_VISUALIZE_SUBBLOBS   0
void smBlobDrawCloudy(Camera* camera, blob* thisBlob, hmatrix* modelView, hmatrix* projection, color background)
{
	sdword index;
	SpaceObj** objPtr, * obj;
	color c;
	hvector objectPos, cameraSpace, screenSpace;
	SpaceObjSelection* blobObjects = thisBlob->blobObjects;
	Node* subBlobNode;
	blob* subBlob;
	sdword nShips;
	smblurry* blurry;
	sdword nBigDots = 0;
	struct
	{
		real32 x, y;
		color c;
	}
	bigDot[SM_NumberBigDots];
	rectangle rect;
	real32 radius;
	real32 pointSize;

	glPointSize(2.0f);
	//compute a list of sub-blobs for the enemies.  It will be deleted with the parent blob.
	if (thisBlob->subBlobs.num == BIT31)
	{                                                       //if list not yet created
		bobSubBlobListCreate(&smFOWBlobProperties, &thisBlob->subBlobs, thisBlob, smEnemyFogOfWarBlobCallback);
		thisBlob->subBlobTime = universe.totaltimeelapsed;
	}
	else
	{
		if (universe.totaltimeelapsed - thisBlob->subBlobTime >= smFOWBlobUpdateTime)
		{
			bobListDelete(&thisBlob->subBlobs);
			bobSubBlobListCreate(&smFOWBlobProperties, &thisBlob->subBlobs, thisBlob, smEnemyFogOfWarBlobCallback);
			thisBlob->subBlobTime = universe.totaltimeelapsed;
		}
	}

	//go through all sub-blobs and remove any that are hidden due to too many resources
	thisBlob->nHiddenShips = 0;
	for (subBlobNode = thisBlob->subBlobs.head; subBlobNode != NULL; subBlobNode = subBlobNode->next)
	{
		subBlob = (blob*)listGetStructOfNode(subBlobNode);
		if (subBlob->shipMass == 0.0f)
		{                                                   //if no ships in blob
			continue;                                       //skip it
		}
		// if ((k1 * #rocks + k2 * #rockRUs) / (k3 * #ships + k4 * shipMass) > 1.0)
		nShips = subBlob->nCapitalShips + subBlob->nAttackShips + subBlob->nNonCombat;
		if ((smFOW_AsteroidK1 * (real32)subBlob->nRocks + smFOW_AsteroidK2 * subBlob->nRockRUs) /
			(smFOW_AsteroidK3 * (nShips)+smFOW_AsteroidK4 * subBlob->shipMass) > 1.0f)
		{                                                   //if there are too many asteroids to see these ships
#if SM_VISUALIZE_SUBBLOBS
			primCircleOutlineZ(&subBlob->centre, subBlob->radius, 16, colRGB(0, 255, 0));
#endif
			thisBlob->nHiddenShips += nShips;
			continue;                                       //don't draw them
		}
		// if (k2 * #dustRUs / (k3 * #ships + k4 * shipMass) > 1.0)
		if ((smFOW_DustGasK2 * subBlob->nDustRUs) /
			(smFOW_DustGasK3 * nShips + smFOW_DustGasK4 * subBlob->shipMass) > 1.0f)
		{                                                   //if there's too much dust
#if SM_VISUALIZE_SUBBLOBS
//           selCircleComputeGeneral(modelView, projection, &subBlob->centre, subBlob->radius, &subBlob->screenX, &subBlob->screenY, &subBlob->screenRadius);
			primCircleOutlineZ(&subBlob->centre, subBlob->radius, 16, colRGB(255, 255, 0));
#endif
			thisBlob->nHiddenShips += nShips;
			continue;                                       //don't draw the ships
		}
		//there's not too many resources; enemy ship points are drawn by cat_smRenderShips.
#if SM_VISUALIZE_SUBBLOBS
		primCircleOutlineZ(&subBlob->centre, subBlob->radius, 16, colWhite);
#endif
	}
	glPointSize(1.0f);
	//draw all objects in the sphere
	for (index = 0, objPtr = blobObjects->SpaceObjPtr; index < blobObjects->numSpaceObjs; index++, objPtr++)
	{
		obj = *objPtr;
		//#ifdef DEBUG_COLLBLOBS
		if (obj->flags & SOF_Targetable)
			//#endif
		{
			selCircleCompute(modelView, projection, (SpaceObjRotImpTarg*)obj);//compute selection circle
		}
		switch (obj->objtype)
		{
			case OBJ_ShipType:
				if (((obj->flags & (SOF_Cloaked | SOF_Hide))) ||  //if hidden or cloaked
					((obj->attributes & ATTRIBUTES_SMColorField) == ATTRIBUTES_SMColorInvisible))
				{
					thisBlob->nHiddenShips++;
				}
				continue;                                   //ships were drawn from sub-blobs
			case OBJ_NebulaType:
				continue;                                   //don't even draw nebulae
			case OBJ_GasType:
			case OBJ_DustType:
				if (showResources == FALSE)
				{
					goto dontRenderThisResource;
				}
				memcpy(&objectPos, &obj->posinfo.position, sizeof(vector));
				objectPos.w = 1.0f;
				hmatMultiplyHMatByHVec(&cameraSpace, modelView, &objectPos);//in camera space
				hmatMultiplyHMatByHVec(&screenSpace, projection, &cameraSpace);//in screen space
				if (screenSpace.z > 0.0f && smBlurryIndex < SM_BlurryArraySize)
				{
#if TO_STANDARD_COLORS
					smBlurryArray[smBlurryIndex].c = teResourceColor;
#else
					smBlurryArray[smBlurryIndex].c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
					smBlurryArray[smBlurryIndex].x = primGLToScreenX(screenSpace.x / screenSpace.w);
					smBlurryArray[smBlurryIndex].y = primGLToScreenY(screenSpace.y / screenSpace.w);
					smBlurryIndex++;
				}
				break;
			case OBJ_AsteroidType:
				if (showResources == FALSE)
				{
					goto dontRenderThisResource;
				}
#if TO_STANDARD_COLORS
				c = teResourceColor;
#else
				c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
				radius = obj->staticinfo->staticheader.staticCollInfo.collspheresize;

				if (radius > SM_LargeResourceSize)
				{
					pointSize = 2.0f;
				}
				else
				{
					pointSize = 1.0f;
				}
				if ((!smBigPoints) && pointSize != 1.0f && ((Ship*)obj)->collInfo.selCircleRadius > 0.0f)
				{
					dbgAssert(nBigDots < SM_NumberBigDots);
					bigDot[nBigDots].x = ((Ship*)obj)->collInfo.selCircleX;
					bigDot[nBigDots].y = ((Ship*)obj)->collInfo.selCircleY;
					bigDot[nBigDots].c = c;
					nBigDots++;
				}
				else
				{
					glPointSize(pointSize);
					primPoint3(&obj->posinfo.position, c);
					glPointSize(1.0f);
				}
				break;
			case OBJ_DerelictType:
				if (((Derelict*)obj)->derelicttype == Crate)
				{                                           //crates are draw as grey dots
					c = teCrateColor;
				}
				else
				{
#if TO_STANDARD_COLORS
					c = teNeutralColor;
#else
					c = obj->staticinfo->staticheader.LOD->pointColor;
#endif
				}
				primPoint3(&obj->posinfo.position, c);
				break;
				//            default:
				//#ifndef DEBUG_COLLBLOBS
				//                dbgAssert(FALSE);       // don't assert if DEBUG_COLLBLOBS because debug collblobs can have bullets,etc.
				//#endif
				//                break;
		}

	dontRenderThisResource:;
	}

	if (smBlurryIndex > 0 || nBigDots > 0)
	{
		primModeSet2();
		for (blurry = smBlurryArray; smBlurryIndex > 0; smBlurryIndex--, blurry++)
		{
			primBlurryPoint22(blurry->x, blurry->y, blurry->c);
		}
		for (index = 0; index < nBigDots; index++)
		{
			rect.x0 = primGLToScreenX(bigDot[index].x);
			rect.y0 = primGLToScreenY(bigDot[index].y);
			rect.x1 = rect.x0 + 2;
			rect.y1 = rect.y0 + 2;
			primRectSolid2(&rect, bigDot[index].c);
		}
		primModeClear2();
	}
	rndLightingEnable(FALSE);
	rndTextureEnable(FALSE);
}

/*-----------------------------------------------------------------------------
	Name        : smBlobListSort
	Description : qsort callback for sorting a list of blob pointers by sortDistance
	Inputs      :
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
int smBlobListSort(const void* p0, const void* p1)
{
	blob* b0 = *((blob**)p0);
	blob* b1 = *((blob**)p1);

	if (b0->cameraSortDistance < b1->cameraSortDistance)
	{
		return(-1);
	}
	else
	{
		return(1);
	}
}

/*-----------------------------------------------------------------------------
	Name        : smBlobsSortToCamera
	Description : list - linked list of blobs
				  camera - camera to sort to
	Inputs      :
	Outputs     : (Re)allocates and fills in smBlobSortList with pointers to
					the blobs in list, sorted by distance from camera->eyeposition
	Return      :
----------------------------------------------------------------------------*/
void smBlobsSortToCamera(LinkedList* list, Camera* camera)
{
	blob* thisBlob;
	Node* node;
	vector distance;

	//grow the blob sorting list if needed
	if ((sdword)list->num > smBlobSortListLength)
	{
		smBlobSortListLength = list->num;
		smBlobSortList = memRealloc(smBlobSortList, sizeof(blob**) * smBlobSortListLength, "smBlobSortList", NonVolatile);
	}
	node = list->head;
	//do a pass through the blobs to figure their sorting distances and count the blobs
	for (node = list->head, smNumberBlobsSorted = 0; node != NULL; node = node->next, smNumberBlobsSorted++)
	{
		thisBlob = (blob*)listGetStructOfNode(node);

		vecSub(distance, camera->eyeposition, thisBlob->centre);
		thisBlob->cameraSortDistance = vecMagnitudeSquared(distance);//figure out a distance for sorting
		smBlobSortList[smNumberBlobsSorted] = thisBlob;     //store reference to blob
	}
	dbgAssert(smNumberBlobsSorted == list->num);
	qsort(smBlobSortList, smNumberBlobsSorted, sizeof(blob**), smBlobListSort);
}

/*-----------------------------------------------------------------------------
	Name        : smBlobDropDistance
	Description : Compute the distance from the movement cursor to a drop-down
					line for a blob when the move mechanism is up.
	Inputs      : thisBlob - blob to use for the drop-down line
	Outputs     :
	Return      : distance from center of blob's plane point to the centre
					point of the movement plane point.
----------------------------------------------------------------------------*/
real32 smBlobDropLineDistance(blob* thisBlob)
{
	vector planePoint = thisBlob->centre;

	planePoint.z = selCentrePoint.z;
	return (fabsf(piePlanePoint.x - planePoint.x) +            //manhattan distance
		fabsf(piePlanePoint.y - planePoint.y));
}

/*-----------------------------------------------------------------------------
	Name        : smBlobsDraw
	Description : Draws all blobs for the current sensors screen
	Inputs      : list - linked list of objects
	Outputs     :
	Return      : Pointer to blob most directly under the mouse cursor or NULL if none.
----------------------------------------------------------------------------*/
typedef struct smBlobRenderEntry
{
	sdword active;
	Ship* ship;
	real32 cameraDistance;
} smBlobRenderEntry;

blob* smBlobsDraw(Camera* camera, LinkedList* list, hmatrix* modelView, hmatrix* projection, sdword sensorLevel)
{
	Node* node;
	smBlobRenderEntry* entries = NULL;
	Ship* ship;
	real32 minDistance = 1.0e21f;
	real32 maxDistance = 0.0f;
	real32 destX, destY, destRadius;
	real32 sensorRadius, factor;
	color blobColor, drawColor;
	sdword numEntries = 0;
	sdword i, j, screenX, screenY, innerRadius, outerRadius, nSegments;
	bool32 reActivateStipple;

	(void)list;
	(void)sensorLevel;

	mouseCursorObjPtr = NULL;
	closestBlob = NULL;

	if (!showFOWBlobs)
	{
		return (blob*)showFOWBlobs;
	}

	for (node = universe.ShipList.head; node != NULL; node = node->next)
	{
		ship = (Ship*)listGetStructOfNode(node);
		if (!bitTest(ship->flags, SOF_Hide) &&
			ship->playerowner == universe.curPlayerPtr &&
			(ship->staticinfo->shipclass != CLASS_Component ||
			 ship->clampInfo == NULL ||
			 ship->clampInfo->host == NULL))
		{
			numEntries++;
		}
	}

	if (numEntries > 0)
	{
		entries = memAlloc(numEntries * sizeof(smBlobRenderEntry), "smBlobRender", 0);
		numEntries = 0;
		for (node = universe.ShipList.head; node != NULL; node = node->next)
		{
			ship = (Ship*)listGetStructOfNode(node);
			if (!bitTest(ship->flags, SOF_Hide) &&
				ship->playerowner == universe.curPlayerPtr &&
				(ship->staticinfo->shipclass != CLASS_Component ||
				 ship->clampInfo == NULL ||
				 ship->clampInfo->host == NULL))
			{
				entries[numEntries].active = TRUE;
				entries[numEntries].ship = ship;
				entries[numEntries].cameraDistance = 0.0f;
				numEntries++;
			}
		}
	}

	for (i = 0; i < numEntries; i++)
	{
		if (!entries[i].active)
		{
			continue;
		}
		for (j = 0; j < numEntries; j++)
		{
			real32 otherRadius, dx, dy, dz;

			if (i == j ||
				!entries[j].active ||
				entries[i].ship->collMyBlob != entries[j].ship->collMyBlob)
			{
				continue;
			}

			sensorRadius = univGetMaxHealth(entries[i].ship, FALSE);
			otherRadius = univGetMaxHealth(entries[j].ship, FALSE);
			if (otherRadius > sensorRadius)
			{
				dx = entries[i].ship->posinfo.position.x - entries[j].ship->posinfo.position.x;
				dy = entries[i].ship->posinfo.position.y - entries[j].ship->posinfo.position.y;
				dz = entries[i].ship->posinfo.position.z - entries[j].ship->posinfo.position.z;
				if (dx * dx + dy * dy + dz * dz < 0.5f * otherRadius * sensorRadius)
				{
					entries[i].active = FALSE;
					break;
				}
			}
		}
	}

	for (i = 0; i < numEntries; i++)
	{
		if (!entries[i].active)
		{
			continue;
		}

		entries[i].cameraDistance = fsqrt(
			(camera->eyeposition.x - entries[i].ship->posinfo.position.x) * (camera->eyeposition.x - entries[i].ship->posinfo.position.x) +
			(camera->eyeposition.y - entries[i].ship->posinfo.position.y) * (camera->eyeposition.y - entries[i].ship->posinfo.position.y) +
			(camera->eyeposition.z - entries[i].ship->posinfo.position.z) * (camera->eyeposition.z - entries[i].ship->posinfo.position.z));

		maxDistance = max(maxDistance, entries[i].cameraDistance);
		minDistance = min(minDistance, entries[i].cameraDistance);
	}

	qsort(entries, numEntries, sizeof(smBlobRenderEntry), (int(__cdecl*)(const void*, const void*))smBlobsSortByZ);

	if (!primModeEnabled)
	{
		primModeSetFunction2();
	}

	if (RGL && glIsEnabled(GL_POLYGON_STIPPLE))
	{
		reActivateStipple = TRUE;
		glDisable(GL_POLYGON_STIPPLE);
	}
	else
	{
		reActivateStipple = FALSE;
	}

	for (i = 0; i < numEntries; i++)
	{
		if (!entries[i].active)
		{
			continue;
		}

		sensorRadius = univGetMaxHealth(entries[i].ship, FALSE);
		selCircleComputeGeneral(modelView, projection, &entries[i].ship->posinfo.position, sensorRadius, &destX, &destY, &destRadius);
		if (destRadius <= 0.0f)
		{
			continue;
		}

		screenX = primGLToScreenX(destX);
		screenY = primGLToScreenY(destY);
		innerRadius = primGLToScreenScaleX(destRadius);
		outerRadius = primGLToScreenScaleX(smCircleBorder * destRadius);
		nSegments = 30 * outerRadius / MAIN_WindowHeight + 8;
		if (nSegments > 30)
		{
			nSegments = 30;
		}

		factor = (entries[i].cameraDistance - minDistance) / (maxDistance - minDistance);
		if (factor >= 0.0f)
		{
			if (factor > 1.0f)
			{
				factor = 1.0f;
			}
			blobColor = colBlend(smIntBlobColorLow, smIntBlobColor, factor);
		}
		else
		{
			blobColor = colBlend(smIntBlobColorLow, smIntBlobColor, 0.0f);
		}

		drawColor = colBlend(blobColor, smIntBackgroundColor, 1.0f);
		if (smFuzzyBlobs)
		{
			primCircleBorder(screenX, screenY, innerRadius, outerRadius, nSegments, drawColor);
		}
		else
		{
			primCircleSolid2(screenX, screenY, outerRadius, nSegments, drawColor);
		}
	}

	if (reActivateStipple)
	{
		glEnable(GL_POLYGON_STIPPLE);
	}
	if (primModeEnabled)
	{
		primModeClearFunction2();
	}
	rndSetDepthTesting(FALSE);
	rndLightingEnable(FALSE);
	rndTextureEnable(FALSE);
	if (entries != NULL)
	{
		memFree(entries);
	}
	return (blob*)0;
}

/*-----------------------------------------------------------------------------
	Name        : smBlobRenderSelected
	Description : Renders the specifed blob as though it was selected, using a colored outline.
	Inputs      : b - blob to render
				  c - color of outline to draw
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
/*
void smBlobRenderSelected(blob *b, color c)
{
	oval o;
	sdword index;
	SpaceObj **obj;
	SpaceObjSelection *blobObjects = b->blobObjects;

	primModeSet2();
	o.centreX = primGLToScreenX(b->screenX);
	o.centreY = primGLToScreenY(b->screenY);
	o.radiusX = o.radiusY = primGLToScreenScaleX(b->screenRadius);
	primOvalArcOutline2(&o, 0.0f, TWOPI, 1, 16, c);
	primModeClear2();
	rndTextureEnable(FALSE);
	rndLightingEnable(FALSE);

	for (index = 0, obj = blobObjects->SpaceObjPtr; index < blobObjects->numSpaceObjs; index++, obj++)
	{
		rndTextureEnable(FALSE);
		primPoint3(&(*obj)->posinfo.position, c);           //everything is rendered as a point
//            (*obj)->staticinfo->staticheader.staticCollInfo.collspheresize,

	}
}
*/

/*-----------------------------------------------------------------------------
	Name        : smXXXFactorGet
	Description : Gets the current zoom factors based on the current zoom time.
	Inputs      : current - current point in function, in  frames
				  range - extent of function, in frames
	Outputs     :
	Return      : current zoom factor
	Notes       : currently a linear interpolation.
----------------------------------------------------------------------------*/
real32 smScrollFactorGet(real32 current, real32 range)
{
	real32 val;

	val = current / range;
	if (val < 0.0f)
	{
		val = 0.0f;
	}
	if (val > 1.0f)
	{
		val = 1.0f;
	}
	return(val);
}
/*
real32 smEyeFactorGet(real32 current, real32 range)
{
	return(current / range);
}
real32 smLookFactorGet(real32 current, real32 range)
{
	return(current / range);
}
*/
/*-----------------------------------------------------------------------------
	Name        : smZoomUpdate
	Description : Updates the zoom function while transitioning TO sensors manager.
	Inputs      : current - current point in function, in time
				  range - extent of function, in time
				  bUpdateCamera - update camera if true
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smZoomUpdate(real32 current, real32 range, bool32 bUpdateCamera)
{
	real32 scrollFactor;
	real32 eyeFactor;
	real32 lookFactor;
	vector distvec;

	scrollFactor = smScrollFactorGet(current, range);
	eyeFactor = lookFactor = scrollFactor;
	//    eyeFactor = smEyeFactorGet(current, range);
	//    lookFactor = smLookFactorGet(current, range);

	smIntBackgroundColor = colMultiplyClamped(smBackgroundColor, scrollFactor);
	smIntBlobColor = colMultiplyClamped(smBlobColor, scrollFactor);
	smIntBlobColorHigh = colMultiplyClamped(smBlobColorHigh, scrollFactor);
	smIntBlobColorLow = colMultiplyClamped(smBlobColorLow, scrollFactor);
	rndSetClearColor(colRGBA(colRed(smIntBackgroundColor),
		colGreen(smIntBackgroundColor),
		colBlue(smIntBackgroundColor),
		255));
	btgSetColourMultiplier(0, (1.0f - smBackgroundDim) * (1.0f - scrollFactor) + smBackgroundDim);
	if (piePointSpecMode != PSM_Idle)
	{
		smCurrentWorldPlaneColor = smWorldPlaneColor;
	}
	else
	{
		smCurrentWorldPlaneColor = colMultiplyClamped(smWorldPlaneColor, scrollFactor);
	}
	//blend between the far clip distances of the 2 cameras
	smCurrentCameraDistance = (smCamera.clipPlaneFar - mrCamera->clipPlaneFar) * scrollFactor + mrCamera->clipPlaneFar;

	//animate the camera position based upon the current zoom factors
	if (bUpdateCamera)
	{
		vecVectorsBlend(&smCamera.lookatpoint, &smLookStart, &smLookEnd, lookFactor);
		vecVectorsBlend(&smCamera.eyeposition, &smEyeStart, &smEyeEnd, eyeFactor);

		vecSub(distvec, smCamera.eyeposition, smCamera.lookatpoint);
		//        distvec.z *= -1.0f;
		GetDistanceAngleDeclination(&smCamera, &distvec);
	}
	//mouseClipToRect(NULL);
}

/*-----------------------------------------------------------------------------
	Name        : smSensorsCloseForGood
	Description : Closes sensors manager after zooming in.
	Inputs      :
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
void smSensorsCloseForGood(void)
{
#if SM_VERBOSE_LEVEL >= 1
	dbgMessage("\nsmSensorsCloseForGood");
#endif
	if (tutorial)
	{
		tutGameMessage("Game_SensorsClose");
	}
	mrRenderMainScreen = TRUE;
	feScreenDeleteFlags(smBaseRegion, 0);
	smBaseRegion = NULL;
	//    bobListDelete(&smBlobList);
	//    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	rndSetClearColor(colRGBA(colRed(universe.backgroundColor),
		colGreen(universe.backgroundColor),
		colBlue(universe.backgroundColor),
		255));
	smSensorsActive = FALSE;
	universe.dontUpdateRenderList = FALSE;
	if (RGL)
		rglFeature(RGL_NORMDEPTH);
	universe.mainCameraCommand.actualcamera.ignoreZoom = FALSE;
	mouseCursorShow();
	mrRightMouseCameraMode = 0;
	smBandboxEnabled = 0;

	if (smFocusOnMothershipOnClose)
	{
		//      CameraStackEntry *entry;
		//      entry = currentCameraStackEntry(&universe.mainCameraCommand);

		ccFocusOnMyMothership(&universe.mainCameraCommand);
		smFocusOnMothershipOnClose = FALSE;

		//      if(entry != currentCameraStackEntry(&universe.mainCameraCommand))
		//          listDeleteNode(&entry->stacklink);
	}

	smZoomingIn = FALSE;
	smZoomingOut = FALSE;
	mouseClipToRect(NULL);                                  //make sure mouse does not stay locked to the SM viewport region
	MP_HyperSpaceFlag = FALSE;
	btgSetColourMultiplier(0, 1.0f);                       //make sure we always come out of the SM with the BTG in good order
	if (byte_8B8844)
	{
		ccSetMode(&pilotEffect, 1);
		ccSetDirtyBit(&pilotEffect);
	}
}

void smBlobSphereDraw(blob* closestBlob)
{
	udword angle = 0;

	vector origin = {0.0, 0.0, 0.0};
	hmatrix rotmat;
	while (angle < 180)
	{
		hmatMakeRotAboutZ(&rotmat, cosf(DEG_TO_RAD(angle)), sinf(DEG_TO_RAD(angle)));
		hmatPutVectIntoHMatrixCol4(closestBlob->centre, rotmat);

		glPushMatrix();
		glMultMatrixf((GLfloat*)&rotmat);

		primCircleOutline3(&origin, closestBlob->radius, 8, 0, colWhite, X_AXIS);

		glPopMatrix();
		angle += 40;
	}

	angle = 0;
	while (angle < 180)
	{
		hmatMakeRotAboutX(&rotmat, cosf(DEG_TO_RAD(angle)), sinf(DEG_TO_RAD(angle)));
		hmatPutVectIntoHMatrixCol4(closestBlob->centre, rotmat);

		glPushMatrix();
		glMultMatrixf((GLfloat*)&rotmat);

		primCircleOutline3(&origin, closestBlob->radius, 8, 0, colWhite, Z_AXIS);

		glPopMatrix();
		angle += 40;
	}


}

/*-----------------------------------------------------------------------------
	Name        : smAllBlobsPiePlateDraw
	Description : Draws lines from all blobs to the pie plate for the
					movement mechanism.
	Inputs      : distance -distance from camera to centre of pie plate
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
/*
void smAllBlobsPiePlateDraw(real32 distance)
{
	Node *node;
	blob *thisBlob;
	bool32 dohackloop = TRUE;
	real32 closestDistance = REALlyBig, length;
	vector mouse_pos,tempvec;
	real32 tempreal;
	blob *closestBlob = NULL;

	probeBlob = NULL;


	if(mrNeedProbeHack())
		dohackloop = TRUE;
	else
		dohackloop = FALSE;


	node = smBlobList.head;

	if (piePointSpecZ != 0.0f)
	{                                       //if a height was specified
		mouse_pos = pieHeightPoint;
	}
	else
	{
		mouse_pos = piePlanePoint;         //else move to point on plane
	}


	while (node)
	{
		thisBlob = (blob *)listGetStructOfNode(node);
		node = node->next;

		if(!dohackloop)
		{
			if (!bitTest(thisBlob->flags, BTF_Explored) &&
				universe.curPlayerPtr->sensorLevel != 2)
			{
				continue;
			}
			length = smBlobDropLineDraw(thisBlob, TW_SHIP_LINE_COLOR);
			if (length < closestDistance)
			{                                                   //if this ship closest so far
				closestDistance = length;
				closestBlob = thisBlob;
			}
		}
		else
		{
			//do closestblob checking first in this loop

			vecSub(tempvec,mouse_pos,thisBlob->centre);
			tempreal  = fsqrt(vecMagnitudeSquared(tempvec));
			if(tempreal < closestDistance)
			{
				closestDistance = tempreal;
				closestBlob = thisBlob;
			}
			if (!bitTest(thisBlob->flags, BTF_Explored) &&
				universe.curPlayerPtr->sensorLevel != 2)
			{
				continue;
			}
			length = smBlobDropLineDraw(thisBlob, TW_SHIP_LINE_COLOR);
		}
	}
	glDisable(GL_DEPTH_TEST);
	if(!dohackloop)
	{
		if (closestBlob != NULL && closestDistance < closestBlob->radius / SM_BlobClosenessFactor)
		{
			smBlobDropLineDraw(closestBlob, TW_SHIP_LINE_CLOSEST_COLOR);
		}
	}
	else
	{
		if (closestBlob != NULL)
		{
			probeBlob = closestBlob;
			vecSub(tempvec,mouse_pos,probeBlob->centre);
			tempreal = fsqrt(vecMagnitudeSquared(tempvec));
			if(tempreal < probeBlob->radius)
			{
				smBlobSphereDraw(closestBlob);
			}
			else
			{
				probeBlob = NULL;
			}
		}
	}
	glEnable(GL_DEPTH_TEST);
}
*/

/*-----------------------------------------------------------------------------
	Name        : smMovePlaneDraw
	Description : Draw the movement plane for the sensors manager.
	Inputs      : distance - distance from camera to selection centre
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
/*
void smMovePlaneDraw(real32 distance)
{
	smWorldPlaneDraw(pieHeightPoint.z, FALSE, smMovePlaneColor);
}
*/
/*-----------------------------------------------------------------------------
	Name        : smBabyRattleDraw
	Description : Draw the movement lines and cursor for the movement mechanism
					while in the sensors manager.
	Inputs      : distance - distance from camera to selection centre
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
/*
void smBabyRattleDraw(real32 distance)
{
	;
}
*/
/*-----------------------------------------------------------------------------
	Name        : smClickedOnPlayer
	Description : returns the a pointer to the player you right clicked on.  NULL otherwise/
	Inputs      : none
	Outputs     : player clicked on or NULL
	Return      : void
----------------------------------------------------------------------------*/
sdword smClickedOnPlayer(rectangle* viewportRect)
{
	fonthandle oldfont;
	rectangle  playerColorRect;
	sdword     index;

	oldfont = fontMakeCurrent(selGroupFont2);

	playerColorRect.y1 = viewportRect->y1 - smPlayerListMarginY;
	playerColorRect.y0 = playerColorRect.y1 - fontHeight(" ");
	playerColorRect.x0 = viewportRect->x0 + smPlayerListMarginX;
	playerColorRect.x1 = playerColorRect.x0 + fontHeight(" ");

	//draw the list of player names/colors
	for (index = universe.numPlayers - 1; index >= 0; index--)
	{
		if (universe.players[index].playerState == PLAYER_ALIVE)
		{
			playerColorRect.x1 = fontWidth(playerNames[index]) + fontHeight(" ") * 2;
			if ((mouseInRect(&playerColorRect)) && (index != sigsPlayerIndex))
			{
				return (index);
			}
			playerColorRect.y0 -= smPlayerListMarginY + fontHeight(" ");//update the position
			playerColorRect.y1 -= smPlayerListMarginY + fontHeight(" ");
		}
	}

	fontMakeCurrent(oldfont);

	return (-1);
}


/*-----------------------------------------------------------------------------
	Name        : smPlayerNamesDraw
	Description : Draw the names of the players in a multi-player game.
	Inputs      : viewportRect - boundaries of the current viewport
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smPlayerNamesDraw(rectangle* viewportRect)
{
	fonthandle oldfont;
	sdword index;
	rectangle playerColorRect;
	color c;
	char buffer[50];
	sdword bounty;

	oldfont = fontMakeCurrent(selGroupFont2);

	playerColorRect.y1 = viewportRect->y1 - smPlayerListMarginY;
	playerColorRect.y0 = playerColorRect.y1 - fontHeight(" ");
	playerColorRect.x0 = viewportRect->x0 + smPlayerListMarginX;
	playerColorRect.x1 = playerColorRect.x0 + fontHeight(" ");

	//draw the list of player names/colors
	for (index = universe.numPlayers - 1; index >= 0; index--)
	{
		if (universe.players[index].playerState == PLAYER_ALIVE)
		{
			sdword x;
			bool32 playerhasdroppedOutOrQuit = playerHasDroppedOutOrQuit(index);

			c = teColorSchemes[index].textureColor.base;
			if (playerhasdroppedOutOrQuit)
			{
				c = HorseRaceDropoutColor;
			}

			primRectSolid2(&playerColorRect, c);                 //draw colored rectangle
			x = playerColorRect.x1 + smPlayerListTextMargin;
			fontPrint(x, //print the player name in a stock color
				playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : smPlayerListTextColor, playerNames[index]);
			x += fontWidth(playerNames[index]) + fontWidth(" ");
			if (index != sigsPlayerIndex)
			{
				if ((allianceArePlayersAllied(&universe.players[sigsPlayerIndex], &universe.players[index])) ||
					((universe.players[sigsPlayerIndex].Allies != 0) && (index == sigsPlayerIndex)))
				{
					fontPrint(x,
						playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : alliescolor, strGetString(strPlayersAllied));
				}
			}

			x += fontWidth(strGetString(strPlayersAllied));
			if ((!singlePlayerGame) && (tpGameCreated.bountySize != MG_BountiesOff))
			{
				//draw bounty values
				fontPrint(x,
					playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : colWhite, "B: ");
				bounty = getPlayerBountyRender(&universe.players[index]);
				itoa(bounty, buffer, 10);
				x += fontWidth("B: ");

				fontPrint(x, playerColorRect.y0, playerhasdroppedOutOrQuit ? HorseRaceDropoutColor : colWhite, buffer);

				x += fontWidth(buffer) + fontWidth(" ");
			}

			if (playerhasdroppedOutOrQuit)
			{
				fontPrint(x, playerColorRect.y0, c, (playersReadyToGo[index] == PLAYER_QUIT) ? strGetString(strQuit) : strGetString(strDroppedOut));
			}

			playerColorRect.y0 -= smPlayerListMarginY + fontHeight(" ");//update the position
			playerColorRect.y1 -= smPlayerListMarginY + fontHeight(" ");
		}
	}

	fontMakeCurrent(oldfont);
}
/*-----------------------------------------------------------------------------
	Name        : smCursorTextDraw
	Description : Draw sensors manager tactical overlay.  This includes the
					player names/colors and a breakdown of what's in a mission sphere.
	Inputs      : viewportRect - boundaries of the current viewport
				  selectedBlob - blob under mouse cursor, or NULL if none.
				  sensorLevel - current sensors level
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smCursorTextDraw(rectangle* viewportRect, blob* selectedBlob, sdword sensorLevel)
{
	char* cursorText = NULL;
	SpaceObj* object;
	fonthandle fhSave;

	//now draw the cursor text to reflect what is under the mouse cursor
	if (selectedBlob != NULL)
	{                                                       //if the mouse is over a blob
		if ((selectedBlob->flags & (BTF_Explored | BTF_ProbeDroid)))
		{                                                   //if this blob has been explored
			if ((object = selClickFromArray(selectedBlob->blobObjects->SpaceObjPtr,
				selectedBlob->blobObjects->numSpaceObjs, mouseCursorX(), mouseCursorY())) != NULL)
			{
				switch (object->objtype)
				{
					case OBJ_ShipType:
						if (((Ship*)object)->playerowner == universe.curPlayerPtr)
						{
							//                            cursorText = strGetString(strPlayerUnits);
							cursorText = ShipTypeToNiceStr(((Ship*)object)->shiptype);
						}
						else if (allianceIsShipAlly((Ship*)object, universe.curPlayerPtr))
						{
							cursorText = strGetString(strAlliedUnits);
						}
						else
						{
							cursorText = strGetString(strEnemyUnits);
						}
						break;
					case OBJ_AsteroidType:
					case OBJ_NebulaType:
					case OBJ_GasType:
					case OBJ_DustType:
						cursorText = strGetString(strResources);
						break;
					case OBJ_DerelictType:
						cursorText = NULL;
						break;
				}
			}
		}
		else
		{                                                   //else unexplored blob
			if (sensorLevel == 0)
			{                                               //don't know anything on plain blobs
				cursorText = strGetString(strUnexplored);
			}
			else
			{                                               //else we've got some sensors available
				if ((object = selClickFromArray(selectedBlob->blobObjects->SpaceObjPtr,
					selectedBlob->blobObjects->numSpaceObjs, mouseCursorX(), mouseCursorY())) != NULL)
				{
					switch (object->objtype)
					{
						case OBJ_ShipType:
							cursorText = strGetString(strEnemyUnits);
							break;
						case OBJ_AsteroidType:
							cursorText = strGetString(strAsteroids);
							break;
						case OBJ_NebulaType:
							cursorText = strGetString(strNebula);
							break;
						case OBJ_GasType:
							cursorText = strGetString(strGasClouds);
							break;
						case OBJ_DustType:
							cursorText = strGetString(strDustClouds);
							break;
						case OBJ_DerelictType:
							cursorText = strGetString(strDerelictShip);
							break;
					}
				}
			}
		}
	}
	//cursor text chosen, print the cursor text if applicable
	if (cursorText != NULL)
	{
		fhSave = fontMakeCurrent(mouseCursorFont);
		fontPrint(viewportRect->x0 + smPlayerListMarginX,
			viewportRect->y1 - fontHeight(cursorText) - smPlayerListMarginY,
			TW_CURSORTEXT_COLOR, cursorText);
		fontMakeCurrent(fhSave);
	}
}

/*-----------------------------------------------------------------------------
	Name        : smTacticalOverlayDraw
	Description : Draw the tactical overlay for sensors manager, including
				  resource types and such
	Inputs      : thisBlob - blob to draw
				  viewportRect - rectangle encompassing the sensors manager
				  sensorLevel - current sensors level
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
int rndSetDepthTesting(int enable);
smOverlayEntry* __cdecl smOverlayLayoutViews(sdword view23Count);
sdword smOverlayBuildEligibleSelection(void);
sdword smOverlayBuildActionAvailability(void);
sdword smOverlayBuildCategoryData(void);
sdword smOverlayActivateItem(sdword index);
sdword smOverlayActivateMimicModeByIndex(sdword index);
sdword __cdecl smDrawOverlayColumnHeaders(rectangle* rect, sdword fillBackground, udword borderSize);
sdword __cdecl smDrawOverlayShipList(
	smOverlayEntry* entries,
	sdword count,
	sdword hasSubmenuColumn,
	sdword labelColumnWidth,
	sdword masterList);

void smTacticalOverlayDraw(void)
{
	fonthandle previousFont;
	sdword depthTestingEnabled;
	sdword spaceWidth;
	sdword spaceHeight;

	previousFont = fontMakeCurrent(largePopupMenuFonts ? smOverlayLargeFont : smOverlaySmallFont);
	depthTestingEnabled = rndSetDepthTesting(FALSE);
	if (smOverlayIsOpen)
	{
		spaceWidth = fontWidth(" ");
		spaceHeight = fontHeight(" ");
		smOverlayCommandColumnGap = (sdword)((real32)spaceWidth * 4.0f);
		smOverlayLabelLeftPadding = (sdword)((real32)spaceWidth * 0.4f);
		smOverlayLabelRightPadding = (sdword)((real32)spaceWidth * 0.4f);
		smOverlayHeaderBorderSize = (sdword)((real32)spaceWidth * 3.0f);
		smOverlaySubmenuIndicatorWidth = (sdword)((real32)spaceWidth * 3.0f);
		smOverlayValueColumnWidth = (sdword)((real32)spaceWidth * 3.0f);
		smOverlayHighlightCapWidth = (sdword)((real32)spaceWidth * 3.0f);
		smOverlayHighlightSidePadding = (sdword)((real32)spaceWidth * 0.8f);
		smOverlayRowPadding = (sdword)((real32)spaceHeight * 0.5f);
		smOverlayGroupSpacing = (sdword)((real32)spaceHeight * 0.6f);

		if (smOverlayIsActionMenu)
		{
			smOverlayBuildActionAvailability();
		}
		else
		{
			smOverlayBuildCategoryData();
		}
		smOverlayLayoutViews(smOverlayRuTransferPlayerCount);
		smOverlayBuildEligibleSelection();
		smDrawOverlayColumnHeaders(&smOverlayMasterRect, TRUE, smOverlayHeaderBorderSize);
		smDrawOverlayShipList(smOverlayMasterEntries, 49, smOverlayMasterHasSubmenuColumn, smOverlayMasterLabelWidth, TRUE);
		switch (smOverlayActiveViewIndex)
		{
			case 14:
				smDrawOverlayColumnHeaders(&smOverlayView14Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88EA08, 3, smOverlayView14HasSubmenuColumn, smOverlayView14LabelWidth, FALSE);
				break;
			case 15:
				smDrawOverlayColumnHeaders(&smOverlayView15Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88EAC0, 8, smOverlayView15HasSubmenuColumn, smOverlayView15LabelWidth, FALSE);
				break;
			case 28:
				smDrawOverlayColumnHeaders(&smOverlayView28Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88EF70, 4, smOverlayView28HasSubmenuColumn, smOverlayView28LabelWidth, FALSE);
				break;
			case 18:
				smDrawOverlayColumnHeaders(&smOverlayView18Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88ECA0, 12, smOverlayView18HasSubmenuColumn, smOverlayView18LabelWidth, FALSE);
				break;
			case 16:
				smDrawOverlayColumnHeaders(&smOverlayView16Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88F060, 4, smOverlayView16HasSubmenuColumn, smOverlayView16LabelWidth, FALSE);
				break;
			case 17:
				smDrawOverlayColumnHeaders(&smOverlayView17Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88F150, 8, smOverlayView17HasSubmenuColumn, smOverlayView17LabelWidth, FALSE);
				break;
			case 19:
				smDrawOverlayColumnHeaders(&smOverlayView19Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_A51D98, smOverlayView19Count, smOverlayView19HasSubmenuColumn, smOverlayView19LabelWidth, FALSE);
				break;
			case 8:
				smDrawOverlayColumnHeaders(&smOverlayView8Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_88F330, 3, smOverlayView8HasSubmenuColumn, smOverlayView8LabelWidth, FALSE);
				break;
			case 33:
				smDrawOverlayColumnHeaders(&smOverlayView33Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_A57270, smOverlayView33Count, smOverlayView33HasSubmenuColumn, smOverlayView33LabelWidth, FALSE);
				break;
			case 34:
				smDrawOverlayColumnHeaders(&smOverlayView34Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_A53CC0, smOverlayView34Count, smOverlayView34HasSubmenuColumn, smOverlayView34LabelWidth, FALSE);
				break;
			case 21:
				smDrawOverlayColumnHeaders(&smOverlayView21Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(unk_A52950, smOverlayView21Count, smOverlayView21HasSubmenuColumn, smOverlayView21LabelWidth, FALSE);
				break;
			case 22:
				smDrawOverlayColumnHeaders(&smOverlayView22Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(smOverlayView22Entries, smOverlayView22Count, smOverlayView22HasSubmenuColumn, smOverlayView22LabelWidth, FALSE);
				break;
			case 23:
				smDrawOverlayColumnHeaders(&smOverlayView23Rect, TRUE, smOverlayHeaderBorderSize);
				smDrawOverlayShipList(smOverlayView23Entries, smOverlayRuTransferPlayerCount, smOverlayView23HasSubmenuColumn, smOverlayView23LabelWidth, FALSE);
				break;
		}
	}
	fontMakeCurrent(previousFont);
	rndSetDepthTesting(depthTestingEnabled);
}

/*-----------------------------------------------------------------------------
	Name        : smHotkeyGroupsDraw
	Description : Draw the location of hotkey groups.
	Inputs      : void
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smHotkeyGroupsDraw(void)
{
	sdword index;
	real32 averageSize, x, y, radius;
	vector centre;
	fonthandle currentFont = fontCurrentGet();
	MaxSelection selection;

	fontMakeCurrent(selGroupFont3);
	for (index = ZEROKEY; index <= NINEKEY; index++)
	{
		MakeShipsNotHidden((SelectCommand*)&selection, (SelectCommand*)&selHotKeyGroup[index - ZEROKEY]);
		if (selection.numShips > 0)
		{
			centre = selCentrePointComputeGeneral(&selection, &averageSize);
			selCircleComputeGeneral(&rndCameraMatrix, &rndProjectionMatrix, &centre, 1.0f, &x, &y, &radius);
			if (radius > 0.0f)
			{
				fontPrint(primGLToScreenX(x) + smHotKeyOffsetX,
					primGLToScreenY(y) + smHotKeyOffsetY,
					selHotKeyNumberColor, selHotKeyString[index - ZEROKEY]);
			}
		}
	}
	fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
	Name        : smPan
	Description : Special-function button that handles both clicks and pan operations.
	Inputs      :
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
void smPan(char* name, featom* atom)
{
	vector clipFrom;

	(void)name;
	(void)atom;

	if (uicDragX == 0 && uicDragY == 0)
	{                                                       //if it's just a click
		smCameraLookatPoint.x = smCameraLookatPoint.y = smCameraLookatPoint.z = 0.0f;//recentre the camera
	}
	else
	{                                                       //else it's a drag operation
		vector addVector, lookVector;
		real32 mult;
		real32 projectedX;

		vecSub(lookVector, smCamera.lookatpoint, smCamera.eyeposition);
		addVector.x = lookVector.x;
		addVector.y = lookVector.y;
		addVector.z = 0.0f;
		vecNormalize(&addVector);
		mult = smCamera.distance * smPanSpeedMultiplier;
		projectedX = addVector.x * mult;
		addVector.y *= mult;
		addVector.x = addVector.y;
		addVector.z *= mult;
		vecScalarMultiply(lookVector, addVector, (real32)uicDragY);
		vecSubFrom(smCameraLookatPoint, lookVector);          //pan into/out of screen

		addVector.y = -projectedX;                           //left/right vector

		vecScalarMultiply(lookVector, addVector, (real32)uicDragX);
		vecAddTo(smCameraLookatPoint, lookVector);          //pan left/right

		memset(&clipFrom, 0, sizeof(clipFrom));
		univClipPointToWorldBox(&clipFrom, &smCameraLookatPoint);
		smGetOverlayFadeColor();
	}
	//    smCamera.lookatpoint = smCameraLookatPoint;
}

/*-----------------------------------------------------------------------------
	Name        : smViewportRender
	Description : Callback which draws the main sensors manager viewport.
	Inputs      : standard FE callback
	Outputs     : ..
	Return      : void
----------------------------------------------------------------------------*/
void smViewportRender(featom* atom, regionhandle region)
{
	blob* selectedBlob;
	CameraStackEntry* curentry = currentCameraStackEntry(&universe.mainCameraCommand);
	sdword frameTicks, sensorLevel;
	real32 frameStep;
	vector rememberVec, diffVec;
	sdword index;

	atom = atom;
	region = region;

	if (spPostLoadNisBlackout)
	{
		return;
	}

	frameTicks = utyNFrameTicks;
	if (frameTicks > smMaxFrameTicks)
		frameTicks = smMaxFrameTicks;

	frameStep = (real32)frameTicks / (real32)taskFrequency;
	cameraControl(&smCamera, FALSE);                         //update the camera
	curentry->remembercam.angle = smCamera.angle;
	curentry->remembercam.declination = smCamera.declination;

	if (smZoomingOut)
	{
		smZoomTime += frameStep;
		smZoomUpdate(smZoomTime, smCurrentZoomLength, TRUE);
		if (smZoomTime >= smCurrentZoomLength)
		{                                                   //if done zooming out
			soundEvent(NULL, UI_BuildManager);
			smZoomingOut = FALSE;                           //stop zooming
		}
		if (smZoomTime < smCurrentMainViewZoomLength)
		{                                                   //if still in the regular renderer part of the zoom
			cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
			universe.mainCameraCommand.actualcamera.ignoreZoom = TRUE;
			return;                                         //let mainrgn.c handle the rendering
		}
		else
		{                                                   //else we're in the SM rendering part
			mrRenderMainScreen = FALSE;

			dbgAssert(smZoomTime >= smCurrentMainViewZoomLength);
			universe.mainCameraCommand.actualcamera.ignoreZoom = FALSE;
		}
	}
	else if (smZoomingIn)
	{
		dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x >(real32) - 1e19);
		smZoomTime -= frameStep;
		smZoomUpdate(smZoomTime, smCurrentZoomLength, TRUE);
		if (smZoomTime <= 0.0f)
		{                                                   //fully zoomed in
			smZoomingIn = FALSE;
			//...actually close the sensors manager
			smSensorsCloseForGood();                        //stop zooming in already!
		}

		if (smZoomTime >= smCurrentMainViewZoomLength)
		{                                                   //if in sm part of zoom
			cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
			universe.mainCameraCommand.actualcamera.ignoreZoom = TRUE;
		}
		else
		{                                                   //else rendering main game screen
			if (!smFocusTransition)
			{                                               //if at transition point
				smFocusTransition = TRUE;                   //make sure we only do this once a zoom
				//... execute the ccfocus thingus
				if (smFocus)
				{
					universe.mainCameraCommand.actualcamera.ignoreZoom = FALSE;
					if (smFocusClose)
					{
						ccFocusClose(&universe.mainCameraCommand, smFocusCommand);
					}
					else
					{
						ccFocusFar(&universe.mainCameraCommand, smFocusCommand, &universe.mainCameraCommand.actualcamera);
					}
					memFree(smFocusCommand);
					smFocusCommand = NULL;
					smFocusClose = FALSE;
				}
				mrRenderMainScreen = TRUE;

				soundEvent(NULL, UI_SoundOfSpace);
			}
			else
			{                                               //else the transition has already been made
				if (!smFocus && smZoomTime > 0.0f)
				{                                           //if not at end of zoom in with no focus
					cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
					universe.mainCameraCommand.actualcamera.ignoreZoom = TRUE;
				}
				return;
			}
		}
	}
	else
	{
		smCurrentWorldPlaneColor = smWorldPlaneColor;
		smCurrentCameraDistance = smCamera.clipPlaneFar;
	}
	smViewportRegion->rect = smViewRectangle;
	if (piePointSpecMode != PSM_Idle)
	{
		smCurrentWorldPlaneColor = colMultiplyClamped(smCurrentWorldPlaneColor, smMovementWorldPlaneDim);
	}

	//perform some render-time monkey work of the camera position
	if (!smZoomingIn && !smZoomingOut && !smSensorsActiveGet())
	{                                               //don't move the eye point if zooming
		uicDragX = uicDragY = 0;
		if (keyIsHit(ARRLEFT))
		{                                                       //pan the camera with the cursor keys
			uicDragX -= smCursorPanX * frameTicks / (udword)taskFrequency;
		}
		if (keyIsHit(ARRRIGHT))
		{
			uicDragX += smCursorPanX * frameTicks / (udword)taskFrequency;
		}
		if (keyIsHit(ARRUP))
		{
			uicDragY -= smCursorPanY * frameTicks / (udword)taskFrequency;
		}
		if (keyIsHit(ARRDOWN))
		{
			uicDragY += smCursorPanY * frameTicks / (udword)taskFrequency;
		}
		if (uicDragX != 0 || uicDragY != 0)
		{
			smPan(NULL, NULL);
		}
		//perform cubic spline interpolation of the panned camera point
		vecSub(rememberVec, smCamera.lookatpoint, smCameraLookatPoint);
		if (fabsf(rememberVec.x) + fabsf(rememberVec.y) + fabsf(rememberVec.z) > smPanEvalThreshold)
		{                                                   //if there is any distance between pan point and camera point
			while (frameTicks)
			{
				rememberVec = smCamera.lookatpoint;
				EvalCubic(&smCamera.lookatpoint.x, &smCameraLookVelocity.x, smCameraLookatPoint.x, smPanTrack);
				EvalCubic(&smCamera.lookatpoint.y, &smCameraLookVelocity.y, smCameraLookatPoint.y, smPanTrack);
				EvalCubic(&smCamera.lookatpoint.z, &smCameraLookVelocity.z, smCameraLookatPoint.z, smPanTrack);

				vecSub(diffVec, smCamera.lookatpoint, rememberVec);
				vecAddTo(smCamera.eyeposition, diffVec);    //pan the eyeposition with the lookatpoint
				frameTicks--;
			}
		}
	}
	//now on to the actual rendering code:
	cameraSetEyePosition(&smCamera);

	primModeClear2();
	rndLightingEnable(FALSE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	rgluPerspective(smCamera.fieldofview, rndAspectRatio,    //set projection matrix
		smCamera.clipPlaneNear, smCamera.clipPlaneFar);
	glScalef(smProjectionScale, smProjectionScale, smProjectionScale);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
#if RND_CAMERA_OFFSET
	rgluLookAt(smCamera.eyeposition.x + RND_CameraOffsetX, smCamera.eyeposition.y,
		smCamera.eyeposition.z, smCamera.lookatpoint.x + RND_CameraOffsetX,
		smCamera.lookatpoint.y, smCamera.lookatpoint.z,
		smCamera.upvector.x, smCamera.upvector.y, smCamera.upvector.z);
#else
	rgluLookAt(smCamera.eyeposition.x, smCamera.eyeposition.y,
		smCamera.eyeposition.z, smCamera.lookatpoint.x,
		smCamera.lookatpoint.y, smCamera.lookatpoint.z,
		smCamera.upvector.x, smCamera.upvector.y, smCamera.upvector.z);
#endif
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)(&rndCameraMatrix));
	glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)(&rndProjectionMatrix));

	if (showSensorBackgrounds)
	{
		//Draw the BTG background
		glPushMatrix();
		glTranslatef(smCamera.eyeposition.x, smCamera.eyeposition.y, smCamera.eyeposition.z);
		rndBackgroundRender(BG_RADIUS, &smCamera, FALSE);  //render the background
		glPopMatrix();

		lightSetLighting();
		rndLightingEnable(TRUE);

		//render all worlds in the universe
		for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
		{
			if (universe.world[index] != NULL)
			{
				if (universe.world[index]->objtype == OBJ_DerelictType)
				{
					rndRenderAHomeworld(&smCamera, universe.world[index]);
				}
			}
			else
			{
				break;
			}
		}
	}

	//update and draw all the sensors manager blobs
	smRenderCount++;
	if (smRenderCount >= smBlobUpdateRate)
	{
		smRenderCount = 0;
	}
	if (smGhostMode)
	{
		sensorLevel = 2;
	}
	else
	{
		sensorLevel = universe.curPlayerPtr->sensorLevel;
	}
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)(&rndCameraMatrix));
	glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)(&rndProjectionMatrix));

	selectedBlob = smBlobsDraw(&smCamera, &universe.collBlobList, &rndCameraMatrix, &rndProjectionMatrix, universe.curPlayerPtr->sensorLevel);
	if (primModeEnabled)
	{
		primModeClearFunction2();
	}
	lightPositionSet();
	rndLightingEnable(FALSE);
	rndTextureEnable(FALSE);
	cat_DrawUniverseBox();
	if (universe.curPlayerPtr->fogGrid != NULL)
	{
		univSetCurrentSensorCoverage((sdword)universe.curPlayerPtr->fogGrid);
		fogDrawGrid();
	}
	cat_smRenderShips(&rndCameraMatrix, &rndProjectionMatrix);

	if (piePointSpecMode != PSM_Idle)
	{                                                       //draw point spec
		mrCamera = &smCamera;                               //point spec draw uses mrCamera
		piePointSpecDraw();                                 //draw the movement mechanism
	}
	primModeSet2();
	primModeSet2();
	pingListDraw(&smCamera, &rndCameraMatrix, &rndProjectionMatrix, &smViewRectangle);

	//draw the tactical overlay, if applicable
	if (smTacticalOverlay)
	{
#if !TO_STANDARD_COLORS
		if (!singlePlayerGame)
		{                                                   //only print player names if multi-player game is underway
			smPlayerNamesDraw(&smViewRectangle);            //draw the list of player names
		}
#endif
		smHotkeyGroupsDraw();
	}
	if (smBandboxEnabled == 1)
	{
		smDrawBandbox(TRUE);
	}
	smDrawPlayerAndBorderIcons();
	if (selSelected.numShips > 0 &&
		selSelected.ShipPtr[0]->playerowner == universe.curPlayerPtr)
	{
		mrDrawTabStats(selSelected.ShipPtr[0]);
	}
	smTacticalOverlayDraw();
	ioUpdateOverlayStates();
	mrUpdateMouseDrag();
	smTacticalOverlayProcess(FALSE, smOverlayRuTransferPlayerCount);
	if (nisStatic[NIS_SMStaticIndex] != NULL)
	{
		nisStaticDraw(nisStatic[NIS_SMStaticIndex]);
	}
}

/*-----------------------------------------------------------------------------
	Name        : smNULL
	Description : NULL logic handler for when the user's not doing anything with
					the sensors manager.
	Inputs      : void
	Outputs     : void
	Return      : void
----------------------------------------------------------------------------*/
void smNULL(void)
{
	;
}

/*-----------------------------------------------------------------------------
	Name        : mrSelectHold
	Description : Function for dragging a selection box
	Inputs      :
	Outputs     :
	Return      :
----------------------------------------------------------------------------*/
void smSelectHold(void)
{
	if (abs(mouseCursorX() - mrOldMouseX) >= selClickBoxWidth ||
		abs(mouseCursorY() - mrOldMouseY) >= selClickBoxHeight)
	{                                                       //if mouse has moved from anchor point
		mrSelectRectBuild(&smSelectionRect, mrOldMouseX, mrOldMouseY);//create a selection rect
		selRectNone();
	}
	else
	{                                                       //else mouse hasn't moved far enough
		smSelectionRect.x0 = smSelectionRect.x1 =           //select nothing
			smSelectionRect.y0 = smSelectionRect.y1 = 0;
		selRectNone();
	}
}

/*-----------------------------------------------------------------------------
	Name        : smCullAndFocusSelecting
	Description : Culls the current selection list to a reasonable size and
					focuses on it.
	Inputs      : void
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smCullAndFocusSelecting(void)
{
	sdword nToFocusOn;
	vector centre = {0.0f, 0.0f, 0.0f};

	if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bSensorsClose)
	{
		return;
	}

	nToFocusOn = ccFocusCullRadiusMean((FocusCommand*)&selSelecting, smFocusRadius * smFocusRadius, &centre);
	if (nToFocusOn == 0)
	{                                                       //nothing to focus on
		return;
	}
	//now we have a full listing of what we can focus on.  Let's focus on it.
	smStartFocusZoom((Ship**)selSelecting.TargetPtr, selSelecting.numTargets, &centre, CAMERA_FOCUSFAR_DISTANCE, FALSE);
}

/*-----------------------------------------------------------------------------
	Name        : smViewportProcess
	Description : process messages for the main viewport of the sensors manager.
	Inputs      : standard region callback parameters
	Outputs     : ..
	Return      : ..
----------------------------------------------------------------------------*/
udword smViewportProcess(regionhandle region, sdword ID, udword event, udword data)
{
	if ((smZoomingIn) || (smZoomingOut))
	{
		return 0;
	}

	switch (event)
	{
		case RPE_HoldLeft:
			mrHandleBandboxHoldLeft(1);
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
			mrHandleBandboxPressRight(&smViewRectangle);
			break;
		case RPE_DoubleLeft:
			smViewportSelectLeftRelease(1);
			break;
		case RPE_ReleaseLeft:
			mrHandleBandboxReleaseLeft(1);
			break;
		case RPE_ReleaseRight:
			mrRightClickMenu(1);
			break;
		case RPE_WheelUp:
			wheel_up = TRUE;
			break;
		case RPE_WheelDown:
			wheel_down = TRUE;
			break;
		case RPE_KeyDown:
			if (smZoomingOut)
			{                                               //if sensors manager is just starting
				break;                                      //don't process key clicks
			}
            if (smFleetIntel &&
                (kbTryCommandKeyBinding(TRUE, 90, ID, FALSE) ||
                 kbTryCommandKeyBinding(TRUE, 107, ID, FALSE)))
            {
                kbInvokeCommandCallbacks(0, 107);
                break;
            }
			if (kbHandleGameScreenKeyDown(TRUE, ID))
			{
				break;
			}
			switch (ID)
			{
				case SHIFTKEY:
					if (smHoldRight != mrCameraMotion)
					{
						piePointModeToggle(TRUE);
					}
					break;
				case MMOUSE_BUTTON:
				case FKEY:
					if (!smFleetIntel)
					{
						selSelectionCopy((MaxAnySelection*)&selSelecting, (MaxAnySelection*)&selSelected);
						smCullAndFocusSelecting();
					}
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
					if (!smFleetIntel)
					{
						if (keyIsHit(ALTKEY))
						{                                   //alt-# select and focus on a hot key group
						altCase:
							if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
							{
								selSelectionCopy((MaxAnySelection*)&selSelected, (MaxAnySelection*)&selHotKeyGroup[ID - ZEROKEY]);
								selSelectionCopy((MaxAnySelection*)&selSelecting, (MaxAnySelection*)&selSelected);
								if (MP_HyperSpaceFlag)
								{
									makeSelectionHyperspaceCapable((SelectCommand*)&selSelected);
								}
								smCullAndFocusSelecting();
#if SM_VERBOSE_LEVEL >= 2
								dbgMessagef("\nHot key group %d selected and focused upon.", ID - ZEROKEY);
#endif
							}
						}
						else if (ID == mrLastKeyPressed && universe.totaltimeelapsed <= mrLastKeyTime + mrNumberDoublePressTime)
						{                                   //double-#: focus on hot-key group
							tutGameMessage("KB_GroupSelectFocus");
							goto altCase;                   //same as alt-#
						}
						else
						{                                   //plain# select a hot key group
							if (selHotKeyGroup[ID - ZEROKEY].numShips != 0)
							{
								tutGameMessage("KB_GroupSelect");

								selSelectHotKeyGroup(&selHotKeyGroup[ID - ZEROKEY]);
								selHotKeyNumbersSet(ID - ZEROKEY);
#if SEL_ERROR_CHECKING
								selHotKeyGroupsVerify();
#endif
								if (MP_HyperSpaceFlag)
								{
									makeSelectionHyperspaceCapable((SelectCommand*)&selSelected);
								}

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
					}
					break;
			}
			mrLastKeyPressed = ID;
			mrLastKeyTime = universe.totaltimeelapsed;
			break;
		case RPE_KeyUp:
			if (smZoomingOut)
			{                                               //if sensors manager is just starting
				break;                                      //don't process key clicks
			}
			if (kbTryToggleSensorsManager(ID))
			{
				break;
			}
			kbCallCommandsBoundToKey(TRUE, ID);
			switch (ID)
			{
				case SHIFTKEY:
					if (smHoldRight != mrCameraMotion)
					{
						piePointModeToggle(FALSE);
					}
					break;
			}
			break;
#if SM_VERBOSE_LEVEL >= 1
		default:
			dbgMessagef("\nsmViewportProcess: unimplemented or unsupported event 0x%x. ID 0x%x, data 0x%x", event, ID, data);
#endif
	}
	return(0);
}

/*-----------------------------------------------------------------------------
	Name        : various
	Description : Processor callbacks for FE buttons
	Inputs      : standard FE callback parameters
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smDispatch(char* name, featom* atom)
{
#if SM_VERBOSE_LEVEL >= 1
	dbgMessage("\nsmDispatch");
#endif
	if (smHoldRight == smNULL)
	{                                                       //cannot bring up MM with RMB held down
		if (!smFleetIntel && ((!(tutorial == TUTORIAL_ONLY)) || tutEnable.bMove))
		{
			if (mrNeedProbeHack())
			{
				mrProbeHack();
			}
			else
			{
				mrRemoveAllProbesFromSelection();
				if (MP_HyperSpaceFlag)
				{
					MP_HyperSpaceFlag = FALSE;
				}
				else
				{
					if (piePointSpecMode == PSM_Idle)
					{                                       //if bringing up the MM
						makeShipsControllable((SelectCommand*)&selSelected, COMMAND_MP_HYPERSPACEING);
						makeShipsNotIncludeSinglePlayerMotherships((SelectCommand*)&selSelected);
						if (selSelected.numShips > 0)
						{                                   //if there are ships we can control
							piePointModeBegin(PiePointSpecTypeMove, mrMoveSelectionPointSpecified);
						}
					}
					else
					{                                       //always allow the MM to be turned off
						piePointModeOnOff();
					}
				}
			}
		}
	}
	feToggleButtonSet(name, piePointSpecMode != PSM_Idle);
}

void smUpdateHyperspaceStatus(bool32 goForLaunch)
{
	mrUpdateHyperspaceStatus(goForLaunch);

	if (smHyperspaceRegion == NULL)
	{
		return;
	}

	if (goForLaunch)
	{
		bitClear(smHyperspaceRegion->status, RSF_RegionDisabled);
	}
	else
	{
		bitSet(smHyperspaceRegion->status, RSF_RegionDisabled);
	}
}

void smHyperspace(char* name, featom* atom)
{
	regionhandle region = (regionhandle)atom->region;

#if SM_VERBOSE_LEVEL >= 1
	dbgMessage("\nsmHyperspace");
#endif

	if (FEFIRSTCALL(atom))
	{
		smHyperspaceRegion = region;

		if (singlePlayerGame)
		{
			if (singlePlayerGameInfo.playerCanHyperspace)
			{
				bitClear(region->status, RSF_RegionDisabled);
			}
			else
			{
				bitSet(region->status, RSF_RegionDisabled);
			}
		}
		else
		{
			//check research capabilities...
			//if not researched...black out button!

			if (!bitTest(tpGameCreated.flag, MG_Hyperspace))
			{
				bitSet(region->status, RSF_RegionDisabled);
			}
			//multiplayer Hyperspace initial Call!

		}
	}
	else if (FELASTCALL(atom))
	{
		smHyperspaceRegion = NULL;
		//check if movement plate for HYPERSPACE is up...
		//close it...
		if (!singlePlayerGame)
		{
			if (!bitTest(tpGameCreated.flag, MG_Hyperspace))
			{
				return;
			}
		}
		if (piePointSpecMode != PSM_Idle
			&& MP_HyperSpaceFlag)
		{
			soundEvent(NULL, UI_MovementGUIoff);
			MP_HyperSpaceFlag = FALSE;
			piePointModeOnOff();
		}
	}
	else
	{
		if (!singlePlayerGame)
		{
			if (!bitTest(tpGameCreated.flag, MG_Hyperspace))
			{
				return;
			}
		}
		if (singlePlayerGame && singlePlayerGameInfo.playerCanHyperspace)
		{
			spHyperspaceButtonPushed();
		}
		else
		{
			if (!smFleetIntel)
			{
				if (MP_HyperSpaceFlag)
				{
					MP_HyperSpaceFlag = FALSE;
					piePointModeOnOff();
					soundEvent(NULL, UI_MovementGUIoff);
				}
				else
				{
					makeShipsControllable((SelectCommand*)&selSelected, COMMAND_MP_HYPERSPACEING);
					makeSelectionHyperspaceCapable((SelectCommand*)&selSelected);
					if (selSelected.numShips == 0)
					{
						//removed all ships!
						//if normal gui up..bring it down!
						if (piePointSpecMode != PSM_Idle)
						{
							//turn it off!
							soundEvent(NULL, UI_MovementGUIoff);
							MP_HyperSpaceFlag = FALSE;
							piePointModeOnOff();
						}
					}
					else
					{
						MP_HyperSpaceFlag = TRUE;
						if (piePointSpecMode == PSM_Idle)
						{
							//plate isn't up...turn it on!
							piePointModeBegin(PiePointSpecTypeMove, mrMoveSelectionPointSpecified);
						}
					}
				}
			}
		}
	}
}

void smTacticalOverlayToggle(char* name, featom* atom)
{
	smTacticalOverlay ^= TRUE;
#if SM_VERBOSE_LEVEL >= 1
	dbgMessagef("\nsmTacticalOverlayToggle: %s", smTacticalOverlay ? "ON" : "OFF");
#endif
}

void smNonResourceToggle(char* name, featom* atom)
{
	smNonResources ^= TRUE;
#if SM_VERBOSE_LEVEL >= 1
	dbgMessagef("\nsmNonResourceToggle: %s", smNonResources ? "ON" : "OFF");
#endif
}

void smResourceToggle(char* name, featom* atom)
{
	smResources ^= TRUE;
#if SM_VERBOSE_LEVEL >= 1
	dbgMessagef("\nsmResourceToggle: %s", smResources ? "ON" : "OFF");
#endif
}

void smSensorsClose(char* name, featom* atom)
{
	vector direction, dist;
	real32 dStart;

	(void)name;
	(void)atom;

	if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bSensorsClose)
	{
		return;
	}

	//    dbgAssert(!smFleetIntel);
	if (smZoomingIn || smZoomingOut)
	{                                                       //if hit while already zooming
		return;
	}
#if SM_VERBOSE_LEVEL >= 1
	dbgMessage("\nsmSensorsClose");
#endif

	if (pieCurrentPointSpecTypeGet() == PiePointSpecTypeSiegeCannon &&
		piePointSpecMode != PSM_Idle)
	{
		piePointModeOnOff();
	}

	if (smFocusOnMothershipOnClose)
	{
		ccFocusOnMyMothership(&universe.mainCameraCommand);
		smFocusOnMothershipOnClose = FALSE;
	}

	//!!! Jason: set the value of smLookStart to the centre of the focussed ships.
	ccLockOnTargetNow(&universe.mainCameraCommand);

	smLookStart = universe.mainCameraCommand.actualcamera.lookatpoint;
	smEyeStart = universe.mainCameraCommand.actualcamera.eyeposition;

	vecSub(direction, smEyeStart, smLookStart);
	dStart = fmathSqrt(vecMagnitudeSquared(direction));
	smEyeEnd = smCamera.eyeposition;
	smLookEnd = smCamera.lookatpoint;
	vecSub(direction, smLookEnd, smEyeEnd);
	vecNormalize(&direction);
	vecMultiplyByScalar(direction, dStart);
	vecSub(smEyeStart, smLookStart, direction);
	dbgAssert(!smZoomingOut);
	smZoomingIn = TRUE;

	/* call the sound event for closing the Sensors manager */
	soundEvent(NULL, UI_SensorsExit);

	smFocusTransition = FALSE;
	smInitialDistance = smCamera.distance;
	smFocus = FALSE;

	//now create a render list as it will be when fully zoomed in
	smTempCamera.lookatpoint = smLookStart;
	smTempCamera.eyeposition = smEyeStart;
	mrCamera = &smTempCamera;
	univUpdateRenderList();
	dbgAssert(smEyeStart.x < (real32)1e19 && smEyeStart.x >(real32) - 1e19);

	//Probe Hack Start   ******************
	if (mrNeedProbeHack())
	{
		if (piePointSpecMode != PSM_Idle)
		{   //turn off pie plate movement mech.
			MP_HyperSpaceFlag = FALSE;
			piePointModeOnOff();
		}
	}
	// check to see if pieplate active if it is and out of range turn it off.
	if (piePointSpecMode != PSM_Idle)
	{
		selCentrePointCompute();
		vecSub(dist, selCentrePoint, universe.mainCameraCommand.actualcamera.lookatpoint);
		if (vecMagnitudeSquared(dist) >= MAX_MOVE_DISTANCE)
		{
			MP_HyperSpaceFlag = FALSE;
			piePointModeOnOff();
		}
	}

	//Probe Hack END     ******************

	MP_HyperSpaceFlag = FALSE;
}

void smSensorsSkip(char* name, featom* atom)
{
	subMessageEnded = 2;                                   //the fleet intel thing was skipped
	speechEventActorStop(ACTOR_ALL_ACTORS, smSkipFadeoutTime);
	subTitlesFadeOut(&subRegion[STR_LetterboxBar], smSkipFadeoutTime);
}

void smCancelDispatch(char* name, featom* atom)
{
#if SM_VERBOSE_LEVEL >= 1
	dbgMessage("\nsmCancelDispatch");
#endif
	feToggleButtonSet(SM_TacticalOverlay, FALSE);
	piePointSpecMode = SPM_Idle;
}

#if SM_TOGGLE_SENSORLEVEL
void smToggleSensorsLevel(char* name, featom* atom)
{
	universe.curPlayerPtr->sensorLevel++;
	if (universe.curPlayerPtr->sensorLevel > 2)
	{
		universe.curPlayerPtr->sensorLevel = 0;
	}
#if SM_VERBOSE_LEVEL >= 1
	dbgMessagef("\nsmToggleSensorsLevel: level set to %d", universe.curPlayerPtr->sensorLevel);
#endif
}
#endif //SM_TOGGLE_SENSORLEVEL

void smCancelMoveOrClose(char* name, featom* atom)
{
	MP_HyperSpaceFlag = FALSE;          //no matter what, set to FALSE
	if (piePointSpecMode != PSM_Idle)
	{
		piePointModeOnOff();
		soundEvent(NULL, UI_MovementGUIoff);
	}
	else
	{
		smSensorsClose(SM_Close, atom);
	}
}
/*=============================================================================
	Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
	Name        : smStartup
	Description : Start the sensors manager module.
	Inputs      : void
	Outputs     : adds some callbacks, loads some stuff etc, etc...
	Return      : void
----------------------------------------------------------------------------*/
fecallback smCallbacks[] =
{
	{smDispatch, SM_Dispatch},
	{smHyperspace, SM_Hyperspace},
	{smTacticalOverlayToggle, SM_TacticalOverlay},
	{smNonResourceToggle, SM_NonResource},
	{smResourceToggle, SM_Resource},
	{smSensorsClose, SM_Close},
	{smSensorsSkip, SM_Skip},
	{smCancelDispatch, SM_CancelDispatch},
	{smPan, SM_Pan},
	{smCancelMoveOrClose, SM_CancelMoveOrClose},
#if SM_TOGGLE_SENSORLEVEL
	{smToggleSensorsLevel, SM_ToggleSensorsLevel},
#endif
	{NULL, NULL},
};
void smStartup(void)
{
	scriptSet(NULL, "sensors.script", smTweaks);
	feDrawCallbackAdd(SM_ViewportName, smViewportRender);   //add render callback
	cameraInit(&smCamera, SM_InitialCameraDist);
	smCamera.closestZoom = smZoomMin;
	smCamera.farthestZoom = smZoomMax;
	smCamera.distance = smInitialDistance;
	smCircleBorder = SM_CircleBorder;
	MP_HyperSpaceFlag = FALSE;      //set to FALSE...should manage itself from here on...
	smCamera.clipPlaneNear = smZoomMin / smZoomMinFactor;
	smCamera.clipPlaneFar = smZoomMax * smZoomMaxFactor;

	//    listInit(&smBlobList);
}

/*-----------------------------------------------------------------------------
	Name        : smUpdateParameters
	Description : Update the sensors manager parameters after a level has
					loaded some sensors parameters.
	Inputs      :
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smUpdateParameters(void)
{
	smCamera.clipPlaneNear = smZoomMin / smZoomMinFactor;
	smCamera.clipPlaneFar = smZoomMax * smZoomMaxFactor;
	smCamera.closestZoom = smZoomMin;
	smCamera.farthestZoom = smZoomMax;
	smCamera.distance = smInitialDistance;
	smCircleBorder = SM_CircleBorder;                       //ignore the value set in the level file
}

/*-----------------------------------------------------------------------------
	Name        : smShutdown
	Description : Shut down the sensors manager module.
	Inputs      : void
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smShutdown(void)
{
}

/*-----------------------------------------------------------------------------
	Name        : smSensorsBegin
	Description : Starts the sensors manager
	Inputs      : name, atom - ignored
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smSensorsBegin(char* name, featom* atom)
{
	regionhandle baseRegion, reg;
	sdword index;
	fescreen* screen;
	char* screenName;
	(void)name;
	(void)atom;

	bitClear(ghMainRegion->status, RSF_MouseInside);
	mrRightMouseCameraMode = 0;
	smBandboxEnabled = 0;

	if (!smSensorsForceOpen && (spScreenLockoutActiveGet() || ((tutorial == TUTORIAL_ONLY) && !tutEnable.bSensorsManager)))
	{
		return;
	}

	screenName = smFleetIntel ? SM_FleetIntelScreenName : SM_ScreenName;
	screen = feScreenFind(screenName);
	smBaseRegion = baseRegion = feScreenStart(ghMainRegion, screenName);
	reg = baseRegion->child;
	smViewportRegion = NULL;
	for (index = 1; index < screen->nAtoms; index++)
	{
		regVerify(reg);
		if (((featom*)reg->userID)->pData == (ubyte*)smViewportRender)
		{
			smBackgroundColor = ((featom*)reg->userID)->contentColor;
			smViewportRegion = reg;
			break;
		}
		reg = reg->next;
	}
	dbgAssert(smViewportRegion != NULL);
	regFunctionSet(smViewportRegion, smViewportProcess);
	regFilterSet(smViewportRegion, SM_ViewportFilter);
	smScrollListTop = smScrollListLeft = smScrollListRight = smScrollListBottom = NULL;
	smScrollCountLeft = smScrollCountTop = smScrollCountRight = smScrollCountBottom = 0;
	smScrollDistLeft = smScrollDistTop = smScrollDistRight = smScrollDistBottom = 0;
	smScrollLeft = smScrollTop = smScrollRight = smScrollBottom = 0;
	smViewRectangle.x0 = 0;
	smViewRectangle.y0 = 0;
	smViewRectangle.x1 = MAIN_WindowWidth;
	smViewRectangle.y1 = MAIN_WindowHeight;
	smViewportRegion->rect = smViewRectangle;
	smZoomTime = 0.0f;
	if (smInstantTransition && !smFleetIntel)
	{
		smCurrentZoomLength = 0.1f;
		smCurrentMainViewZoomLength = 0.05f;
	}
	else
	{
		smCurrentZoomLength = smZoomLength;
		smCurrentMainViewZoomLength = smMainViewZoomLength;
	}

	//figure out the start/end points for the camera animation
	smLookStart = universe.mainCameraCommand.actualcamera.lookatpoint;
	smEyeStart = universe.mainCameraCommand.actualcamera.eyeposition;
	//smLookEnd.x = smLookEnd.y = smLookEnd.z = 0.0f;
	smLookEnd = universe.mainCameraCommand.actualcamera.lookatpoint;
	smCameraLookatPoint = smLookEnd;
	smCameraLookVelocity.x = smCameraLookVelocity.y = smCameraLookVelocity.z = 0.0f;
	vecSub(smEyeEnd, smEyeStart, smLookStart);
	vecNormalize(&smEyeEnd);
	vecMultiplyByScalar(smEyeEnd, smInitialDistance);
	vecAddTo(smEyeEnd, smLookEnd);
	smZoomingIn = FALSE;
	smZoomingOut = TRUE;

	mouseClipToRect(NULL);
	smSensorsActive = TRUE;

	soundEventStopSFX(0.5f);

	/* call the sound event for opening the Sensors manager */
	soundEvent(NULL, UI_SensorsIntro);

	universe.dontUpdateRenderList = TRUE;
	if (RGL)
		rglFeature(RGL_SANSDEPTH);

	smViewportRegisterAllKeys(smViewportRegion, smViewportProcess);
	mouseCursorShow();
	cameraCopyPositionInfo(&smCamera, mrCamera);
	if (smFleetIntel)
	{                                                       //if we are in the fleet intel screen, no movement mechanism!
		piePointSpecMode = SPM_Idle;
	}
}

/*-----------------------------------------------------------------------------
	Name        : smObjectDied
	Description : Called when an object dies, this function removes it from the
					sensors manager blob in which it exists.
	Inputs      : object - object to delete
	Outputs     :
	Return      : void
----------------------------------------------------------------------------*/
void smObjectDied(void* object)
{
	sdword index;

	//    bobObjectDied(object,&smBlobList);

		//remove it from the focus command, if there is one.
	if (smFocusCommand)
	{
		for (index = 0; index < smFocusCommand->numShips; index++)
		{
			if ((SpaceObj*)smFocusCommand->ShipPtr[index] == object)
			{
				for (; index < smFocusCommand->numShips - 1; index++)
				{
					smFocusCommand->ShipPtr[index] = smFocusCommand->ShipPtr[index + 1];
				}
				smFocusCommand->numShips--;
				if (smFocusCommand->numShips <= 0)
				{
					memFree(smFocusCommand);
					smFocusCommand = NULL;
					smFocus = FALSE;
				}
				return;
			}
		}
	}
}

/*=============================================================================
	Save Game Stuff:
=============================================================================*/

void smSave(void)
{
	SaveInfoNumber(TreatAsUdword(smDepthCueRadius));
	SaveInfoNumber(TreatAsUdword(smDepthCueStartRadius));
	SaveInfoNumber(TreatAsUdword(smCircleBorder));
	SaveInfoNumber(TreatAsUdword(smZoomMax));
	SaveInfoNumber(TreatAsUdword(smZoomMin));
	SaveInfoNumber(TreatAsUdword(smZoomMinFactor));
	SaveInfoNumber(TreatAsUdword(smZoomMaxFactor));
	SaveInfoNumber(TreatAsUdword(smInitialDistance));
	SaveInfoNumber(TreatAsUdword(smUniverseSizeX));
	SaveInfoNumber(TreatAsUdword(smUniverseSizeY));
	SaveInfoNumber(TreatAsUdword(smUniverseSizeZ));

	SaveInfoNumber(smSensorWeirdness);
}

void smLoad(void)
{
	sdword loadnum;

	loadnum = LoadInfoNumber(); smDepthCueRadius = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smDepthCueStartRadius = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smCircleBorder = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smZoomMax = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smZoomMin = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smZoomMinFactor = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smZoomMaxFactor = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smInitialDistance = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smUniverseSizeX = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smUniverseSizeY = TreatAsReal32(loadnum);
	loadnum = LoadInfoNumber(); smUniverseSizeZ = TreatAsReal32(loadnum);

	smSensorWeirdness = LoadInfoNumber();

	smUpdateParameters();
}



void smDrawNoise(smNoiseParams* noise)
{
	sdword count = noise->baseCount;
	real32 xHalfWidth = (noise->xMax - noise->xMin) * 0.5f;
	real32 xCenter = (noise->xMin + noise->xMax) * 0.5f;
	real32 yMin = noise->yCenter - noise->yHalfHeight;
	real32 yMax = noise->yCenter + noise->yHalfHeight;
	real32 hueMin = max(0.0f, noise->hueCenter - noise->hueVariance);
	real32 hueMax = min(1.0f, noise->hueCenter + noise->hueVariance);
	real32 saturationMin = max(0.0f, noise->saturationCenter - noise->saturationVariance);
	real32 saturationMax = min(1.0f, noise->saturationCenter + noise->saturationVariance);
	real32 luminosityMin = max(0.0f, noise->luminosityCenter - noise->luminosityVariance);
	real32 luminosityMax = min(1.0f, noise->luminosityCenter + noise->luminosityVariance);
	real32 alphaMin = max(0.0f, noise->alphaCenter - noise->alphaVariance);
	real32 alphaMax = min(1.0f, noise->alphaCenter + noise->alphaVariance);

	if (noise->countVariance != 0)
	{
		count += random(2 * noise->countVariance + 1) - noise->countVariance;
	}

	if (noise->blend)
	{
		glEnable(GL_BLEND);
	}

	if (noise->texture == -1)
	{
		glBegin(GL_LINES);
		while (count > 0)
		{
			real32 red, green, blue;

			colHLSToRGB(
				&red,
				&green,
				&blue,
				smNoiseRandomRange(hueMin, hueMax),
				smNoiseRandomRange(luminosityMin, luminosityMax),
				smNoiseRandomRange(saturationMin, saturationMax));
			glColor4f(red, green, blue, smNoiseRandomRange(alphaMin, alphaMax));
			glVertex3f(smNoiseRandomRange(noise->xMin, noise->xMax), smNoiseRandomRange(yMin, yMax), 0.0f);
			glVertex3f(smNoiseRandomRange(noise->xMin, noise->xMax), smNoiseRandomRange(yMin, yMax), 0.0f);
			count--;
		}
		glEnd();
	}
	else
	{
		real32 screenPixelHeight = 2.0f / (real32)MAIN_WindowHeight;

		rndTextureEnable(TRUE);
		trClearCurrent();
		rndTextureEnvironment(RTE_Modulate);
		rndLightingEnable(FALSE);
		trMakeCurrent(noise->texture);
		glBegin(GL_QUADS);
		while (count > 0)
		{
			real32 halfWidthOffset = smNoiseRandomUnit() * (2.0f * xHalfWidth) - xHalfWidth;
			real32 left = xCenter + halfWidthOffset;
			real32 right = left + 2.0f * halfWidthOffset;
			real32 y = smNoiseRandomRange(yMin, yMax);
			real32 bottom = y - MAIN_WindowHeight * screenPixelHeight;
			real32 red, green, blue;

			colHLSToRGB(
				&red,
				&green,
				&blue,
				smNoiseRandomRange(hueMin, hueMax),
				smNoiseRandomRange(luminosityMin, luminosityMax),
				smNoiseRandomRange(saturationMin, saturationMax));
			glColor4f(red, green, blue, smNoiseRandomRange(alphaMin, alphaMax));
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(left, y, 0.0f);
			glVertex3f(right, y, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(left, bottom, 0.0f);
			glVertex3f(right, bottom, 0.0f);
			count--;
		}
		glEnd();
		rndTextureEnable(FALSE);
	}

	glDisable(GL_BLEND);
}

void smFocusRemoveObj(SpaceObj* obj)
{
	smObjectDied(obj);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4EC6F0
int smOverlayClose(void)
{
	smOverlayIsOpen = 0;
	telldir();
	return 0;
}


sdword fogGridStorageSize(sdword width, sdword height, sdword depth)
{
	return 32 * width * height * depth;
}

void fogSaveState(Player* player)
{
	SaveChunk* chunk;
	sdword size;

	if (player->fogGrid == NULL)
	{
		return;
	}

	chunk = CreateChunk(BASIC_STRUCTURE | SAVE_FOGGRIDHEADER, sizeof(FogGridSaveState), player->fogGrid);
	SaveThisChunk(chunk);
	memFree(chunk);

	size = fogGridStorageSize(player->fogGrid->width, player->fogGrid->height, player->fogGrid->depth);
	if (size <= 0 || player->fogGrid->data == NULL)
	{
		return;
	}

	chunk = CreateChunk(BASIC_STRUCTURE | SAVE_FOGGRIDDATA, size, player->fogGrid->data);
	SaveThisChunk(chunk);
	memFree(chunk);
}

void fogLoadState(Player* player)
{
	SaveChunk* chunk;
	SaveChunk header;
	sdword size;

	if (player->playerIndex != universe.curPlayerIndex)
	{
		return;
	}

	if (!PeekNextChunkHeader(&header) ||
		header.type != (BASIC_STRUCTURE | SAVE_FOGGRIDHEADER))
	{
		return;
	}

	chunk = LoadNextChunk();

	VerifyChunk(chunk, BASIC_STRUCTURE | SAVE_FOGGRIDHEADER, sizeof(FogGridSaveState));

	player->fogGrid = memAlloc(sizeof(FogGridSaveState), "FogGridSaveState", NonVolatile);
	memcpy(player->fogGrid, chunkContents(chunk), sizeof(FogGridSaveState));
	memFree(chunk);

	size = fogGridStorageSize(player->fogGrid->width, player->fogGrid->height, player->fogGrid->depth);
	chunk = LoadNextChunk();
	VerifyChunk(chunk, BASIC_STRUCTURE | SAVE_FOGGRIDDATA, size);

	player->fogGrid->data = memAlloc(size, "FogGridSaveData", NonVolatile);
	memcpy(player->fogGrid->data, chunkContents(chunk), size);
	memFree(chunk);

	univSetCurrentSensorCoverage((sdword)player->fogGrid);
	fogAllocRenderState(TRUE);
}

sdword __cdecl cat_DrawUniverseBoxEdge(vector* from, vector* to, color lineColor)
{
	vector innerFrom;
	vector innerTo;
	vector delta;

	delta.x = (to->x - from->x) * 0.05f;
	delta.y = (to->y - from->y) * 0.05f;
	delta.z = (to->z - from->z) * 0.05f;

	innerFrom.x = from->x + delta.x;
	innerFrom.y = from->y + delta.y;
	innerFrom.z = from->z + delta.z;
	innerTo.x = to->x - delta.x;
	innerTo.y = to->y - delta.y;
	innerTo.z = to->z - delta.z;

	primLine3(from, &innerFrom, lineColor);
	primLine3(to, &innerTo, lineColor);
	return 0;
}

sdword cat_DrawUniverseBox(void)
{
	color lineColor;
	vector minCorner;
	vector maxCorner;
	vector corner001;
	vector corner010;
	vector corner011;
	vector corner100;
	vector corner101;
	vector corner110;

	if (!showUniverseExtents)
	{
		return showUniverseExtents;
	}

	lineColor = smComputeOverlayFadeColor(0xFFA26B40, 0);
	if (lineColor == 0xFF000000)
	{
		return showUniverseExtents;
	}

	minCorner.x = -smUniverseSizeX;
	minCorner.y = -smUniverseSizeY;
	minCorner.z = -smUniverseSizeZ;
	maxCorner.x = smUniverseSizeX;
	maxCorner.y = smUniverseSizeY;
	maxCorner.z = smUniverseSizeZ;

	corner001 = minCorner;
	corner001.z = maxCorner.z;
	corner010 = minCorner;
	corner010.y = maxCorner.y;
	corner100 = minCorner;
	corner100.x = maxCorner.x;
	corner011 = corner010;
	corner011.z = maxCorner.z;
	corner101 = corner100;
	corner101.z = maxCorner.z;
	corner110 = corner100;
	corner110.y = maxCorner.y;

	cat_DrawUniverseBoxEdge(&minCorner, &corner001, lineColor);
	cat_DrawUniverseBoxEdge(&minCorner, &corner010, lineColor);
	cat_DrawUniverseBoxEdge(&minCorner, &corner100, lineColor);
	cat_DrawUniverseBoxEdge(&corner011, &corner001, lineColor);
	cat_DrawUniverseBoxEdge(&corner011, &corner010, lineColor);
	cat_DrawUniverseBoxEdge(&corner011, &maxCorner, lineColor);
	cat_DrawUniverseBoxEdge(&corner101, &corner001, lineColor);
	cat_DrawUniverseBoxEdge(&corner101, &corner100, lineColor);
	cat_DrawUniverseBoxEdge(&corner101, &maxCorner, lineColor);
	cat_DrawUniverseBoxEdge(&corner110, &corner010, lineColor);
	cat_DrawUniverseBoxEdge(&corner110, &corner100, lineColor);
	return cat_DrawUniverseBoxEdge(&corner110, &maxCorner, lineColor);
}


bool32 __cdecl smShipVisibleToPlayer(Ship* ship)
{
	bool32 visible;

	visible = !univIsShipCloaked((SpaceObj*)ship);
	if (smFogVisibilityMode)
	{
		if (universe.curPlayerPtr->fogGrid == NULL)
		{
			if ((udword)smFogVisibilityMode > 1)
			{
				visible = FALSE;
			}
		}
		else
		{
			univSetCurrentSensorCoverage((sdword)universe.curPlayerPtr->fogGrid);
			if (fogPointInGridBounds(ship->posinfo.position.x, ship->posinfo.position.y, ship->posinfo.position.z))
			{
				if (CheckIfShipInRange(ship->posinfo.position.x, ship->posinfo.position.y, ship->posinfo.position.z) <= 0)
				{
					visible = FALSE;
				}
			}
			else if ((udword)smFogVisibilityMode > 1)
			{
				visible = FALSE;
			}
		}
	}

	if (singlePlayerGame)
	{
		return visible;
	}
	if ((ship->flags & SOF_Hide) != 0 && shipIsModuleSlotActive(ship, universe.curPlayerPtr))
	{
		return FALSE;
	}
	if (!shipGetSlotStatus(ship, universe.curPlayerPtr->playerIndex))
	{
		return FALSE;
	}

	return visible;
}

sdword smBlobsSortByZ(vector* p0, vector* p1)
{
	return (p0->z <= p1->z) ? 1 : -1;
}

// Cataclysm-specific: scales overlay colors through the retail nebula-brightness path.
color __cdecl UpdateNebulaBrightness(color baseColor, real32 fade)
{
	sdword brightness;

	brightness = (sdword)(fade * 255.0f + 0.5f);
	if (brightness > 255)
	{
		brightness = 255;
	}

	return colRGB(
		(ubyte)(((uword)brightness * colRed(baseColor)) >> 8),
		(ubyte)(((uword)brightness * colGreen(baseColor)) >> 8),
		(ubyte)(((uword)brightness * colBlue(baseColor)) >> 8));
}

color __cdecl smComputeOverlayFadeColor(color baseColor, sdword invert)
{
	real32 fade;

	fade = smScrollFactorGet(smZoomTime, smCurrentZoomLength);
	if (invert == 1)
	{
		fade = 1.0f - fade;
	}
	if (fade >= 1.0f)
	{
		return baseColor;
	}
	if (fade <= 0.0f)
	{
		return 0xFF000000;
	}

	return UpdateNebulaBrightness(baseColor, fade);
}

TargetPtr __cdecl smOverlaySelectDefaultTargetForView(TargetPtr target)
{
	TargetPtr result;
	sdword mode;

	result = target;
	switch ((udword)target)
	{
		case (udword)0x1A:
			mode = 1;
			result = (TargetPtr)smFindNearestDefaultTargetForView(selSelected.ShipPtr[0], mode);
			break;
		case (udword)0x1B:
			result = (TargetPtr)smFindNearestDefaultTargetForView(selSelected.ShipPtr[0], 0);
			break;
		case (udword)0x1D:
		case (udword)0x28:
		case (udword)0x2C:
		case (udword)0x2E:
		case (udword)0x2F:
		case (udword)0x30:
			result = (TargetPtr)smFindNearestDefaultTargetForView(selSelected.ShipPtr[0], 2);
			break;
		case (udword)0x2D:
			mode = 3;
			result = (TargetPtr)smFindNearestDefaultTargetForView(selSelected.ShipPtr[0], mode);
			break;
		default:
			smOverlayDefaultTargets.numTargets = 0;
			return result;
	}

	if (result == NULL)
	{
		smOverlayDefaultTargets.numTargets = 0;
		return result;
	}

	smOverlayDefaultTargets.numTargets = 1;
	smOverlayDefaultTargets.TargetPtr[0] = result;
	return result;
}

bool32 __cdecl smOverlayMouseInRect(sdword* rect)
{
	sdword cursorX;
	sdword cursorY;

	cursorX = mouseCursorX();
	cursorY = mouseCursorY();
	return cursorX >= rect[0] + 1 &&
		cursorY >= rect[1] + 1 &&
		cursorX < rect[2] - 1 &&
		cursorY < rect[3] - 1;
}

sdword smOverlaySyncSensorMode(void)
{
	if (!smSensorsActive)
	{
		smOverlaySensorViewStateValid = 0;
		return FALSE;
	}

	smOverlaySensorResourcesEnabled = showResources;
	smOverlaySensorFowBlobsEnabled = showFOWBlobs;
	smOverlaySensorAreasVisitedEnabled = showAreasVisited;
	smOverlaySensorAreasOccupiedEnabled = showAreasOccupied;
	smOverlaySensorCapitalShipOverlayEnabled = showCapitalShipOverlay;
	smOverlaySensorFriendlyShipsEnabled = showFriendlyShips;
	smOverlaySensorEnemyShipsEnabled = showEnemyShips;
	smOverlaySensorBackgroundEnabled = showSensorBackgrounds;
	smOverlaySensorDustEnabled = showDust;
	smOverlaySensorCompassEnabled = showCompass;
	smOverlaySensorViewStateValid = 1;
	smOverlaySensorObjectivePointersEnabled = showObjectivePointers;
	smOverlaySensorSlipgatePathsEnabled = showSlipgatePaths;
	smOverlaySensorDecorativeAsteroidsEnabled = 1;
	smOverlaySensorShipFowRangeEnabled = 1;
	smOverlaySensorAsteroidsEnabled = 1;
	smOverlaySensorShipMeshesEnabled = 1;
	smOverlaySensorPieplateLinesEnabled = 1;
	smOverlaySensorUnknown16Enabled = 1;
	smOverlaySensorUnknown17Enabled = 1;
	smOverlaySensorUnknown18Enabled = 1;
	smOverlaySensorUnknown19Enabled = 1;
	smOverlaySensorUnknown20Enabled = 1;
	smOverlaySensorUnknown21Enabled = 1;
	smOverlaySensorUnknown22Enabled = 1;
	smOverlaySensorUnknown23Enabled = 1;
	return TRUE;
}

// Cataclysm-specific: initializes tactical-overlay fonts and resets overlay-open state.
static void smOverlayApplyTemplates(
	smOverlayEntry* entries,
	sdword count,
	const smOverlayEntryTemplate* templates)
{
	sdword index;

	for (index = 0; index < count; index++)
	{
		entries[index].labelOverride = templates[index].labelOverride;
		entries[index].labelStringIndex = templates[index].labelStringIndex;
		entries[index].commandId = templates[index].commandId;
		entries[index].hasValueColumn = templates[index].hasValueColumn;
		entries[index].groupIndex = templates[index].groupIndex;
		entries[index].enabled = templates[index].enabled;
		entries[index].selectionLocked = templates[index].selectionLocked;
		entries[index].pulseHighlight = templates[index].pulseHighlight;
		entries[index].forceSubmenuColumn = templates[index].forceSubmenuColumn;
		entries[index].hovered = FALSE;
		entries[index].x0 = 0;
		entries[index].y0 = 0;
		entries[index].x1 = 0;
		entries[index].y1 = 0;
		entries[index].unused2 = 0;
	}
}

sdword smOverlayInit(void)
{
	smOverlayLargeFont = frFontRegister("arial_15.hff");
	smOverlaySmallFont = frFontRegister("arial_12.hff");
	smOverlayApplyTemplates(smOverlayMasterEntries, 49, smOverlayMasterTemplates);
	smOverlayApplyTemplates(unk_88EA08, 3, smOverlayView14Templates);
	smOverlayApplyTemplates(unk_88EAC0, 8, smOverlayView15Templates);
	smOverlayApplyTemplates(unk_88ECA0, 12, smOverlayView18Templates);
	smOverlayApplyTemplates(unk_88EF70, 4, smOverlayView28Templates);
	smOverlayApplyTemplates(unk_88F060, 4, smOverlayView16Templates);
	smOverlayApplyTemplates(unk_88F150, 8, smOverlayView17Templates);
	smOverlayApplyTemplates(unk_88F330, 3, smOverlayView8Templates);
	smOverlayIsOpen = 0;
	return smOverlaySmallFont;
}







TargetPtr __cdecl smOverlayFilterTargetsByFriendliness(sdword keepFriendly)
{
	sdword index;

	index = 0;
	while (index < smOverlayExplicitTargets.numTargets)
	{
		TargetPtr target;

		target = smOverlayExplicitTargets.TargetPtr[index];
		if (target->objtype != OBJ_ShipType)
		{
			smOverlayExplicitTargets.TargetPtr[index] = smOverlayExplicitTargets.TargetPtr[--smOverlayExplicitTargets.numTargets];
			continue;
		}
		if ((keepFriendly && !univIsObjFriendly((SpaceObj*)target, universe.curPlayerPtr)) ||
			(!keepFriendly && univIsObjFriendly((SpaceObj*)target, universe.curPlayerPtr)))
		{
			smOverlayExplicitTargets.TargetPtr[index] = smOverlayExplicitTargets.TargetPtr[--smOverlayExplicitTargets.numTargets];
			continue;
		}
		index++;
	}

	return (TargetPtr)smOverlayExplicitTargets.numTargets;
}








bool32 __cdecl smOverlayRemoveTargetFromSelections(TargetPtr removeTargetPtr)
{
	clRemoveTargetFromSelection((SelectAnyCommand*)&smOverlayExplicitTargets, removeTargetPtr);
	return clRemoveTargetFromSelection((SelectAnyCommand*)&smOverlayDefaultTargets, removeTargetPtr);
}

static char* smOverlayEntryLabel(smOverlayEntry* entry)
{
	if (entry->labelOverride != NULL)
	{
		return entry->labelOverride;
	}

	return strGetString(entry->labelStringIndex);
}

static sdword smOverlayRootEntrySubviewX(sdword rootEntryIndex)
{
	return smOverlayMasterEntries[rootEntryIndex].x1;
}

static sdword smOverlayRootEntrySubviewY(sdword rootEntryIndex)
{
	return smOverlayMasterEntries[rootEntryIndex].y0;
}

// Cataclysm-specific: lays out one tactical-overlay table and its item rects.
smOverlayEntry* __cdecl smOverlayLayoutViewList(
	sdword x,
	sdword y,
	smOverlayEntry* entries,
	sdword count,
	rectangle* viewportRect,
	sdword* hasSubmenuColumn,
	sdword* maxLabelWidth,
	sdword forceSubmenuColumn,
	sdword useGroupSpacing)
{
	sdword hotkeyWidth;
	sdword rowHalfHeight;
	sdword currentYOffset;
	sdword totalHeight;
	sdword totalWidth;
	sdword xAdjust;
	sdword yAdjust;
	sdword totalGroupSpacing;
	bool32 hasHotkeyColumn;
	sdword groupOffsets[19];
	bool32 seenGroup[19];
	char bindingText[256];
	sdword i;
	smOverlayEntry* lastEntry;

	hotkeyWidth = smOverlayValueColumnWidth;
	rowHalfHeight = smOverlayRowPadding / 2;
	currentYOffset = rowHalfHeight;
	totalGroupSpacing = 0;
	hasHotkeyColumn = FALSE;
	lastEntry = NULL;

	if (useGroupSpacing)
	{
		memset(groupOffsets, 0, sizeof(groupOffsets));
		memset(seenGroup, 0, sizeof(seenGroup));
	}

	viewportRect->x0 = x;
	viewportRect->y0 = y;
	*hasSubmenuColumn = FALSE;
	*maxLabelWidth = 0;

	for (i = 0; i < count; i++)
	{
		smOverlayEntry* entry;
		char* label;

		entry = &entries[i];
		lastEntry = entry;
		if (!entry->enabled)
		{
			continue;
		}

		entry->x0 = viewportRect->x0;
		entry->y0 = viewportRect->y0 + currentYOffset;
		entry->y1 = entry->y0 + fontHeight(NULL);
		currentYOffset += rowHalfHeight + fontHeight(NULL);

		if (useGroupSpacing && entry->groupIndex >= 0 && entry->groupIndex < 19)
		{
			seenGroup[entry->groupIndex] = TRUE;
		}

		label = smOverlayEntryLabel(entry);
		if (fontWidth((ubyte*)label) > *maxLabelWidth)
		{
			*maxLabelWidth = fontWidth((ubyte*)label);
		}

		if (entry->commandId != 0)
		{
			kbFormatCommandBindings(entry->commandId, bindingText);
			if (fontWidth((ubyte*)bindingText) > hotkeyWidth)
			{
				hotkeyWidth = fontWidth((ubyte*)bindingText);
			}
			hasHotkeyColumn = TRUE;
		}

		if (entry->hasValueColumn != 0)
		{
			hasHotkeyColumn = TRUE;
		}
		if (entry->forceSubmenuColumn != 0)
		{
			*hasSubmenuColumn = TRUE;
		}
	}

	totalHeight = rowHalfHeight + currentYOffset;
	if (forceSubmenuColumn)
	{
		*hasSubmenuColumn = TRUE;
	}

	totalWidth = smOverlayLabelLeftPadding + smOverlayLabelRightPadding + *maxLabelWidth;
	if (*hasSubmenuColumn)
	{
		totalWidth += smOverlaySubmenuIndicatorWidth;
	}
	if (hasHotkeyColumn)
	{
		totalWidth += hotkeyWidth + smOverlayCommandColumnGap;
	}

	if (useGroupSpacing)
	{
		bool32 sawFirstGroup;

		sawFirstGroup = FALSE;
		for (i = 0; i < 19; i++)
		{
			if (seenGroup[i])
			{
				if (sawFirstGroup)
				{
					totalGroupSpacing += smOverlayGroupSpacing;
					groupOffsets[i] = totalGroupSpacing;
				}
				else
				{
					sawFirstGroup = TRUE;
				}
			}
		}
	}

	viewportRect->x1 = viewportRect->x0 + totalWidth;
	viewportRect->y1 = viewportRect->y0 + totalHeight;
	if (useGroupSpacing)
	{
		viewportRect->y1 += totalGroupSpacing;
	}

	if (viewportRect->x1 <= MAIN_WindowWidth)
	{
		xAdjust = 0;
	}
	else
	{
		xAdjust = MAIN_WindowWidth - viewportRect->x1;
		if (!useGroupSpacing)
		{
			// Binary behavior: subviews clamp against the master column's left edge, not the full window.
			xAdjust = smOverlayMasterRect.x0 - viewportRect->x1;
		}
		viewportRect->x0 += xAdjust;
		viewportRect->x1 += xAdjust;
	}

	if (viewportRect->y1 <= MAIN_WindowHeight)
	{
		yAdjust = 0;
	}
	else
	{
		yAdjust = MAIN_WindowHeight - viewportRect->y1;
		viewportRect->y0 += yAdjust;
		viewportRect->y1 += yAdjust;
	}

	for (i = 0; i < count; i++)
	{
		smOverlayEntry* entry;

		entry = &entries[i];
		if (!entry->enabled)
		{
			continue;
		}

		if (useGroupSpacing && entry->groupIndex >= 0 && entry->groupIndex < 19)
		{
			entry->y0 += groupOffsets[entry->groupIndex];
			entry->y1 += groupOffsets[entry->groupIndex];
		}
		entry->x1 = totalWidth + entry->x0;
		entry->x0 += xAdjust;
		entry->x1 += xAdjust;
		entry->y0 += yAdjust;
		entry->y1 += yAdjust;
	}

	return lastEntry;
}

// Cataclysm-specific: lays out the master tactical-overlay column and the active subview.
smOverlayEntry* __cdecl smOverlayLayoutViews(sdword view23Count)
{
	smOverlayEntry* result;

	(void)view23Count;

	result = smOverlayLayoutViewList(
		smOverlayAnchorX + smOverlayRootColumnXOffset,
		smOverlayAnchorY + smOverlayRootColumnYOffset,
		smOverlayMasterEntries,
		49,
		&smOverlayMasterRect,
		&smOverlayMasterHasSubmenuColumn,
		&smOverlayMasterLabelWidth,
		0,
		1);

	switch (smOverlayActiveViewIndex)
	{
		case 14:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(14) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(14) + smOverlaySubviewYOffset, unk_88EA08, 3, &smOverlayView14Rect, &smOverlayView14HasSubmenuColumn, &smOverlayView14LabelWidth, 1, 0);

		case 15:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(15) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(15) + smOverlaySubviewYOffset, unk_88EAC0, 8, &smOverlayView15Rect, &smOverlayView15HasSubmenuColumn, &smOverlayView15LabelWidth, 1, 0);

		case 28:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(28) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(28) + smOverlaySubviewYOffset, unk_88EF70, 4, &smOverlayView28Rect, &smOverlayView28HasSubmenuColumn, &smOverlayView28LabelWidth, 1, 0);

		case 18:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(18) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(18) + smOverlaySubviewYOffset, unk_88ECA0, 12, &smOverlayView18Rect, &smOverlayView18HasSubmenuColumn, &smOverlayView18LabelWidth, 1, 0);

		case 16:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(16) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(16) + smOverlaySubviewYOffset, unk_88F060, 4, &smOverlayView16Rect, &smOverlayView16HasSubmenuColumn, &smOverlayView16LabelWidth, 0, 0);

		case 17:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(17) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(17) + smOverlaySubviewYOffset, unk_88F150, 8, &smOverlayView17Rect, &smOverlayView17HasSubmenuColumn, &smOverlayView17LabelWidth, 1, 0);

		case 19:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(19) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(19) + smOverlaySubviewYOffset, unk_A51D98, smOverlayView19Count, &smOverlayView19Rect, &smOverlayView19HasSubmenuColumn, &smOverlayView19LabelWidth, 0, 0);

		case 8:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(8) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(8) + smOverlaySubviewYOffset, unk_88F330, 3, &smOverlayView8Rect, &smOverlayView8HasSubmenuColumn, &smOverlayView8LabelWidth, 0, 0);

		case 33:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(33) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(33) + smOverlaySubviewYOffset, unk_A57270, smOverlayView33Count, &smOverlayView33Rect, &smOverlayView33HasSubmenuColumn, &smOverlayView33LabelWidth, 0, 0);

		case 34:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(34) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(34) + smOverlaySubviewYOffset, unk_A53CC0, smOverlayView34Count, &smOverlayView34Rect, &smOverlayView34HasSubmenuColumn, &smOverlayView34LabelWidth, 0, 0);

		case 21:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(21) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(21) + smOverlaySubviewYOffset, unk_A52950, smOverlayView21Count, &smOverlayView21Rect, &smOverlayView21HasSubmenuColumn, &smOverlayView21LabelWidth, 0, 0);

		case 22:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(22) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(22) + smOverlaySubviewYOffset, smOverlayView22Entries, smOverlayView22Count, &smOverlayView22Rect, &smOverlayView22HasSubmenuColumn, &smOverlayView22LabelWidth, 0, 0);

		case 23:
			return smOverlayLayoutViewList(smOverlayRootEntrySubviewX(23) + smOverlaySubviewXOffset, smOverlayRootEntrySubviewY(23) + smOverlaySubviewYOffset, smOverlayView23Entries, smOverlayRuTransferPlayerCount, &smOverlayView23Rect, &smOverlayView23HasSubmenuColumn, &smOverlayView23LabelWidth, 0, 0);

		default:
			return result;
	}
}

// Cataclysm-specific: updates hover state and active overlay view from the mouse cursor.
sdword __cdecl smOverlayHitTest(sdword rootView, smOverlayEntry* entries, sdword count, rectangle* viewportRect)
{
	sdword cursorX;
	sdword cursorY;
	sdword index;
	sdword rowHit;

	cursorX = screenX;
	cursorY = screenY;
	rowHit = FALSE;

	for (index = 0; index < count; index++)
	{
		smOverlayEntry* entry;

		entry = &entries[index];
		if (!entry->enabled)
		{
			continue;
		}

		if (cursorX < entry->x0 - smOverlayHighlightSidePadding ||
			cursorY < entry->y0 - smOverlayRowPadding / 2 ||
			cursorX >= entry->x1 + smOverlayHighlightSidePadding ||
			cursorY >= entry->y1 + smOverlayRowPadding / 2 ||
			rowHit)
		{
			entry->hovered = FALSE;
			continue;
		}

		rowHit = TRUE;
		entry->hovered = TRUE;
		if (!entry->selectionLocked)
		{
			if (rootView)
			{
				sdword activeView;
				bool32 wrapped;

				activeView = index;
				wrapped = FALSE;
				smOverlayActiveViewIndex = index;
				while ((smOverlayMasterEntries[activeView].selectionLocked == 1 || !smOverlayMasterEntries[activeView].enabled) && !wrapped)
				{
					if (++activeView >= 49)
					{
						activeView = 0;
					}
					if (activeView == index)
					{
						wrapped = TRUE;
					}
				}

				smOverlayActiveViewIndex = activeView;
				if (activeView >= 24 && activeView <= 48 && !smOverlayDefaultTargets.numTargets)
				{
					smOverlaySelectDefaultTargetForView((TargetPtr)activeView);
				}
				nameIndex = -1;
			}
			else
			{
				nameIndex = index;
			}
		}
	}

	if (cursorX < viewportRect->x0 - smOverlayHighlightSidePadding ||
		cursorY < viewportRect->y0 - smOverlayHighlightSidePadding ||
		cursorX >= viewportRect->x1 + smOverlayHighlightSidePadding ||
		cursorY >= viewportRect->y1 + smOverlayHighlightSidePadding)
	{
		return rowHit;
	}

	return TRUE;
}

// Cataclysm-specific: processes overlay hover updates for the master column and active subview.
sdword __cdecl smTacticalOverlayProcess(sdword forceUpdate, sdword view23Count)
{
	bool32 mouseMoved;
	sdword result;

	(void)view23Count;

	mouseMoved = TRUE;
	if (!forceUpdate && smOverlayLastMouseX == screenX)
	{
		mouseMoved = smOverlayLastMouseY != screenY;
	}

	if (!smSensorsActiveGet() && !smOverlayIsOpen)
	{
		smOverlayLastMouseX = screenX;
		smOverlayLastMouseY = screenY;
		return screenY;
	}

	if (!mouseMoved)
	{
		smOverlayLastMouseX = screenX;
		smOverlayLastMouseY = screenY;
		return screenX;
	}

	switch (smOverlayActiveViewIndex)
	{
		case 14:
			result = smOverlayHitTest(FALSE, unk_88EA08, 3, &smOverlayView14Rect);
			break;

		case 15:
			result = smOverlayHitTest(FALSE, unk_88EAC0, 8, &smOverlayView15Rect);
			break;

		case 28:
			result = smOverlayHitTest(FALSE, unk_88EF70, 4, &smOverlayView28Rect);
			break;

		case 18:
			result = smOverlayHitTest(FALSE, unk_88ECA0, 12, &smOverlayView18Rect);
			break;

		case 16:
			result = smOverlayHitTest(FALSE, unk_88F060, 4, &smOverlayView16Rect);
			break;

		case 17:
			result = smOverlayHitTest(FALSE, unk_88F150, 8, &smOverlayView17Rect);
			break;

		case 19:
			result = smOverlayHitTest(FALSE, unk_A51D98, smOverlayView19Count, &smOverlayView19Rect);
			break;

		case 8:
			result = smOverlayHitTest(FALSE, unk_88F330, 3, &smOverlayView8Rect);
			break;

		case 33:
			result = smOverlayHitTest(FALSE, unk_A57270, smOverlayView33Count, &smOverlayView33Rect);
			break;

		case 34:
			result = smOverlayHitTest(FALSE, unk_A53CC0, smOverlayView34Count, &smOverlayView34Rect);
			break;

		case 21:
			result = smOverlayHitTest(FALSE, unk_A52950, smOverlayView21Count, &smOverlayView21Rect);
			break;

		case 22:
			result = smOverlayHitTest(FALSE, smOverlayView22Entries, smOverlayView22Count, &smOverlayView22Rect);
			break;

		case 23:
			result = smOverlayHitTest(FALSE, smOverlayView23Entries, smOverlayRuTransferPlayerCount, &smOverlayView23Rect);
			break;

		default:
			result = FALSE;
			break;
	}

	if (!result)
	{
		smOverlayHitTest(TRUE, smOverlayMasterEntries, 49, &smOverlayMasterRect);
		smOverlayLastMouseX = screenX;
		smOverlayLastMouseY = screenY;
		return screenY;
	}

	smOverlayLastMouseX = screenX;
	smOverlayLastMouseY = screenY;
	return result;
}

static sdword smOverlayEnemyPlayerIndexByVisibleIndex(sdword visibleIndex)
{
	Player* player;
	sdword playerIndex;
	sdword currentVisibleIndex;

	currentVisibleIndex = 0;
	player = universe.players;
	for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++, player++)
	{
		if (playerIndex != universe.curPlayerIndex &&
			player->playerState != PLAYER_DEAD &&
			!allianceArePlayersAllied(universe.curPlayerPtr, player))
		{
			if (currentVisibleIndex == visibleIndex)
			{
				return playerIndex;
			}
			currentVisibleIndex++;
		}
	}
	return -1;
}

static sdword smOverlayAllyPlayerIndexByVisibleIndex(sdword visibleIndex)
{
	Player* player;
	sdword playerIndex;
	sdword currentVisibleIndex;

	currentVisibleIndex = 0;
	player = universe.players;
	for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++, player++)
	{
		if (playerIndex != universe.curPlayerIndex &&
			player->playerState != PLAYER_DEAD &&
			allianceArePlayersAllied(universe.curPlayerPtr, player))
		{
			if (currentVisibleIndex == visibleIndex)
			{
				return playerIndex;
			}
			currentVisibleIndex++;
		}
	}
	return -1;
}

static smOverlayEntry* smOverlayCurrentSubviewEntries(sdword* count)
{
	switch (smOverlayActiveViewIndex)
	{
		case 14:
			*count = 3;
			return unk_88EA08;

		case 15:
			*count = 8;
			return unk_88EAC0;

		case 28:
			*count = 4;
			return unk_88EF70;

		case 18:
			*count = 12;
			return unk_88ECA0;

		case 16:
			*count = 4;
			return unk_88F060;

		case 17:
			*count = 8;
			return unk_88F150;

		case 19:
			*count = smOverlayView19Count;
			return unk_A51D98;

		case 8:
			*count = 3;
			return unk_88F330;

		case 33:
			*count = smOverlayView33Count;
			return unk_A57270;

		case 34:
			*count = smOverlayView34Count;
			return unk_A53CC0;

		case 21:
			*count = smOverlayView21Count;
			return unk_A52950;

		case 22:
			*count = smOverlayView22Count;
			return smOverlayView22Entries;

		case 23:
			*count = smOverlayRuTransferPlayerCount;
			return smOverlayView23Entries;

		default:
			*count = 0;
			return NULL;
	}
}

static sdword smOverlayActivateMasterEntryInternal(sdword index, bool32* closeOverlay)
{
	if (index < 0 || index >= 49)
	{
		return FALSE;
	}
	if (!smOverlayMasterEntries[index].enabled ||
		smOverlayMasterEntries[index].selectionLocked ||
		smOverlayMasterEntries[index].hasValueColumn)
	{
		return FALSE;
	}

	if (index == 36)
	{
		MaxSelection protectTargets;

		if (mrBuildProtectTargetSelection(&protectTargets, (SelectAnyCommand*)&smOverlayExplicitTargets) > 0)
		{
			mrProtectSelection((SelectCommand*)&protectTargets);
		}
		else
		{
			mrProtectSelection(NULL);
		}
		*closeOverlay = FALSE;
	}
	else if (index == 40)
	{
		SelectAnyCommand* targetSelection = NULL;

		if (smOverlayExplicitTargets.numTargets > 0)
		{
			targetSelection = (SelectAnyCommand*)&smOverlayExplicitTargets;
		}

		mrLaunchSwarmersNearestTarget(targetSelection);
		*closeOverlay = TRUE;
	}
	else if (index == 37)
	{
		mrCycleSelectionLeechMode();
		*closeOverlay = FALSE;
	}
	else if (index == 25)
	{
		SelectAnyCommand* targetSelection;
		bool32 handledTarget;
		sdword targetIndex;

		targetSelection = (SelectAnyCommand*)&smOverlayDefaultTargets;
		if (smOverlayExplicitTargets.numTargets > 0)
		{
			targetSelection = (SelectAnyCommand*)&smOverlayExplicitTargets;
		}

		handledTarget = FALSE;
		for (targetIndex = 0; targetIndex < targetSelection->numTargets; targetIndex++)
		{
			SpaceObj* target;

			target = (SpaceObj*)targetSelection->TargetPtr[targetIndex];
			if (target->objtype == OBJ_ShipType)
			{
				if (shipModuleSlotIsEligible((Ship*)target, universe.curPlayerPtr))
				{
					mrHarvestResourcesTargetResource((Resource*)target);
					handledTarget = TRUE;
					break;
				}
			}
			else if (target->objtype >= OBJ_AsteroidType && target->objtype <= OBJ_DustType)
			{
				mrHarvestResourcesTargetResource((Resource*)target);
				handledTarget = TRUE;
				break;
			}
		}

		if (!handledTarget)
		{
			mrHarvestResourcesTargetResource(NULL);
		}
		*closeOverlay = TRUE;
	}
	else if (index == 35)
	{
		mrCancelExternalConstructionSelection();
		*closeOverlay = TRUE;
	}
	else if (index == 47)
	{
		// Retail's flat switch(index) in smOverlayActivateItem calls mrActivateSelectionSpecial()
		// directly for this master entry (the "Repulse"/generic special-ability activation item);
		// it does not go through the commandId/kbInvokeCommandCallbacks indirection used below.
		mrActivateSelectionSpecial();
		*closeOverlay = TRUE;
	}
	else
	{
		if (index != 1 && smOverlayMasterEntries[index].commandId != 0)
		{
			kbInvokeCommandCallbacks(smSensorsActive != 0, smOverlayMasterEntries[index].commandId);
		}
		*closeOverlay = (index != 1 && index != 10);
	}

	return TRUE;
}

static sdword smOverlayActivateSubviewEntryInternal(sdword index, bool32* closeOverlay)
{
	sdword entryCount;
	smOverlayEntry* entries;

	entries = smOverlayCurrentSubviewEntries(&entryCount);
	if (entries == NULL || index < 0 || index >= entryCount || !entries[index].enabled || entries[index].selectionLocked)
	{
		return FALSE;
	}

	switch (smOverlayActiveViewIndex)
	{
		case 14:
		case 15:
		case 28:
		case 16:
		case 17:
		case 8:
			if (entries[index].commandId == 0)
			{
				return FALSE;
			}
			kbInvokeCommandCallbacks(smSensorsActive != 0, entries[index].commandId);
			*closeOverlay = TRUE;
			return TRUE;

		case 18:
			if (entries[index].commandId == 0)
			{
				return FALSE;
			}
			kbInvokeCommandCallbacks(smSensorsActive != 0, entries[index].commandId);
			*closeOverlay = FALSE;
			return TRUE;

		case 19:
			shipNameReserveByIndex(selSelected.ShipPtr[0], index);
			*closeOverlay = TRUE;
			return TRUE;

		case 33:
			if (smMimicIsPlayerIndexValid(index))
			{
				smMimicSetCurrentPlayerIndex(index);
			}
			*closeOverlay = FALSE;
			return TRUE;

		case 34:
			*closeOverlay = smOverlayActivateMimicModeByIndex(index);
			return TRUE;

		case 21:
		{
			sdword playerIndex;

			playerIndex = smOverlayEnemyPlayerIndexByVisibleIndex(index);
			if (playerIndex != -1)
			{
				allianceFormWith(playerIndex);
			}
			*closeOverlay = TRUE;
			return TRUE;
		}

		case 22:
		{
			sdword playerIndex;

			playerIndex = smOverlayAllyPlayerIndexByVisibleIndex(index);
			if (playerIndex != -1)
			{
				allianceBreakWith(playerIndex);
			}
			*closeOverlay = TRUE;
			return TRUE;
		}

		case 23:
		{
			sdword playerIndex;

			playerIndex = smOverlayAllyPlayerIndexByVisibleIndex(index);
			if (smSensorsActive)
			{
				smSensorsClose(NULL, NULL);
			}
			else if (playerIndex != -1)
			{
				gcRUTransferStart((uword)playerIndex);
			}
			*closeOverlay = TRUE;
			return TRUE;
		}

		default:
			return FALSE;
	}
}

sdword smOverlayActivateItem(sdword index)
{
	bool32 closeOverlay;
	sdword activated;

	closeOverlay = FALSE;
	activated = smOverlayActivateSubviewEntryInternal(index, &closeOverlay);
	if (!activated)
	{
		activated = smOverlayActivateMasterEntryInternal(index, &closeOverlay);
	}

	if (activated && closeOverlay)
	{
		smOverlayClose();
	}

	return activated;
}

sdword smOverlayActivateCurrentSelection(void)
{
	if (!smOverlayIsOpen)
	{
		return FALSE;
	}

	if (nameIndex >= 0)
	{
		return smOverlayActivateItem(nameIndex);
	}

	return smOverlayActivateItem(smOverlayActiveViewIndex);
}

sdword smOverlayTryBoundKeyInRange(sdword key, sdword first, sdword last)
{
	sdword entryCount;
	sdword index;
	smOverlayEntry* entries;

	entries = smOverlayCurrentSubviewEntries(&entryCount);
	if (entries == NULL)
	{
		return FALSE;
	}

	for (index = 0; index < entryCount; index++)
	{
		smOverlayEntry* entry;

		entry = &entries[index];
		if (!entry->enabled || entry->selectionLocked || entry->commandId < first || entry->commandId > last)
		{
			continue;
		}

		if (kbTryCommandKeyBinding(smSensorsActive != 0, entry->commandId, key, FALSE))
		{
			nameIndex = index;
			return smOverlayActivateCurrentSelection();
		}
	}

	return FALSE;
}

static sdword smOverlayTryBoundKeyInMasterEntries(sdword key)
{
	sdword index;

	for (index = 0; index < 49; index++)
	{
		smOverlayEntry* entry;

		entry = &smOverlayMasterEntries[index];
		if (!entry->enabled || entry->selectionLocked || entry->hasValueColumn || entry->commandId == 0)
		{
			continue;
		}

		if (kbTryCommandKeyBinding(smSensorsActive != 0, entry->commandId, key, FALSE))
		{
			smOverlayActiveViewIndex = index;
			nameIndex = -1;
			return smOverlayActivateCurrentSelection();
		}
	}

	return FALSE;
}

sdword smOverlayHandleKeyCommand(sdword key)
{
	if (!smOverlayIsOpen)
	{
		return FALSE;
	}

	if (key == ESCKEY || key == XKEY)
	{
		smOverlayClose();
		return TRUE;
	}

	if (key == ENTERKEY)
	{
		return smOverlayActivateCurrentSelection();
	}

	switch (smOverlayActiveViewIndex)
	{
		case 14:
			if (smOverlayTryBoundKeyInRange(key, 44, 46))
			{
				return TRUE;
			}
			break;

		case 15:
			if (smOverlayTryBoundKeyInRange(key, 49, 56))
			{
				return TRUE;
			}
			break;

		case 28:
			if (smOverlayTryBoundKeyInRange(key, 21, 23))
			{
				return TRUE;
			}
			break;

		case 18:
			if (smOverlayTryBoundKeyInRange(key, 139, 150))
			{
				return TRUE;
			}
			break;

		case 16:
			if (smOverlayTryBoundKeyInRange(key, 59, 64))
			{
				return TRUE;
			}
			break;

		case 17:
			if (smOverlayTryBoundKeyInRange(key, 71, 84))
			{
				return TRUE;
			}
			break;

		case 8:
			if (smOverlayTryBoundKeyInRange(key, 11, 13))
			{
				return TRUE;
			}
			break;
	}

	return smOverlayTryBoundKeyInMasterEntries(key);
}

static sdword smOverlayHandleMouseClickInternal(sdword actionMenuOnly)
{
	bool32 clickedEntry;
	bool32 closeOverlay;
	sdword index;
	sdword playerIndex;

	if (actionMenuOnly && !smOverlayIsActionMenu)
	{
		return FALSE;
	}

	clickedEntry = FALSE;
	closeOverlay = FALSE;
	smTacticalOverlayProcess(TRUE, smOverlayRuTransferPlayerCount);

	switch (smOverlayActiveViewIndex)
	{
		case 14:
			for (index = 0; index < 3; index++)
			{
				if (unk_88EA08[index].hovered)
				{
					if (unk_88EA08[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88EA08[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 15:
			for (index = 0; index < 8; index++)
			{
				if (unk_88EAC0[index].hovered)
				{
					if (unk_88EAC0[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88EAC0[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 28:
			for (index = 0; index < 4; index++)
			{
				if (unk_88EF70[index].hovered)
				{
					if (unk_88EF70[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88EF70[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 18:
			for (index = 0; index < 12; index++)
			{
				if (unk_88ECA0[index].hovered)
				{
					if (unk_88ECA0[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88ECA0[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = FALSE;
				}
			}
			break;

		case 16:
			for (index = 0; index < 4; index++)
			{
				if (unk_88F060[index].hovered)
				{
					if (unk_88F060[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88F060[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 17:
			for (index = 0; index < 8; index++)
			{
				if (unk_88F150[index].hovered)
				{
					if (unk_88F150[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88F150[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 19:
			for (index = 0; index < smOverlayView19Count; index++)
			{
				if (unk_A51D98[index].hovered)
				{
					shipNameReserveByIndex(selSelected.ShipPtr[0], index);
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 8:
			for (index = 0; index < 3; index++)
			{
				if (unk_88F330[index].hovered)
				{
					if (unk_88F330[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, unk_88F330[index].commandId);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 33:
			for (index = 0; index < smOverlayView33Count; index++)
			{
				if (unk_A57270[index].hovered)
				{
					if (smMimicIsPlayerIndexValid(index))
					{
						smMimicSetCurrentPlayerIndex(index);
					}
					clickedEntry = TRUE;
					closeOverlay = FALSE;
				}
			}
			break;

		case 34:
			for (index = 0; index < smOverlayView34Count; index++)
			{
				if (unk_A53CC0[index].hovered)
				{
					closeOverlay = smOverlayActivateMimicModeByIndex(index);
					clickedEntry = TRUE;
				}
			}
			break;

		case 21:
			for (index = 0; index < smOverlayView21Count; index++)
			{
				if (unk_A52950[index].hovered)
				{
					playerIndex = smOverlayEnemyPlayerIndexByVisibleIndex(index);
					if (playerIndex != -1)
					{
						allianceFormWith(playerIndex);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 22:
			for (index = 0; index < smOverlayView22Count; index++)
			{
				if (smOverlayView22Entries[index].hovered)
				{
					playerIndex = smOverlayAllyPlayerIndexByVisibleIndex(index);
					if (playerIndex != -1)
					{
						allianceBreakWith(playerIndex);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 23:
			for (index = 0; index < smOverlayRuTransferPlayerCount; index++)
			{
				if (smOverlayView23Entries[index].hovered)
				{
					playerIndex = smOverlayAllyPlayerIndexByVisibleIndex(index);
					if (smSensorsActive)
					{
						smSensorsClose(NULL, NULL);
					}
					else if (playerIndex != -1)
					{
						gcRUTransferStart((uword)playerIndex);
					}
					clickedEntry = TRUE;
					closeOverlay = TRUE;
				}
			}
			break;

		case 40:
		{
			SelectAnyCommand* targetSelection = NULL;

			if (smOverlayMasterEntries[40].enabled)
			{
				if (smOverlayExplicitTargets.numTargets > 0)
				{
					targetSelection = (SelectAnyCommand*)&smOverlayExplicitTargets;
				}

				mrLaunchSwarmersNearestTarget(targetSelection);
				clickedEntry = TRUE;
				closeOverlay = TRUE;
			}
			break;
		}

		case 41:
			if (smOverlayMasterEntries[41].enabled &&
				smOverlayMasterEntries[41].commandId != 0)
			{
				kbInvokeCommandCallbacks(smSensorsActive != 0, smOverlayMasterEntries[41].commandId);
				clickedEntry = TRUE;
				closeOverlay = TRUE;
			}
			break;
	}

	if (!clickedEntry)
	{
		for (index = 0; index < 49; index++)
		{
			if (!clickedEntry &&
				smOverlayMasterEntries[index].hovered &&
				!smOverlayMasterEntries[index].hasValueColumn &&
				!smOverlayMasterEntries[index].selectionLocked &&
				smOverlayMasterEntries[index].enabled)
			{
				if (index == 36)
				{
					MaxSelection protectTargets;

					if (mrBuildProtectTargetSelection(&protectTargets, (SelectAnyCommand*)&smOverlayExplicitTargets) > 0)
					{
						mrProtectSelection((SelectCommand*)&protectTargets);
					}
					else
					{
						mrProtectSelection(NULL);
					}
					closeOverlay = FALSE;
				}
				else if (index == 40)
				{
					SelectAnyCommand* targetSelection = NULL;

					if (smOverlayExplicitTargets.numTargets > 0)
					{
						targetSelection = (SelectAnyCommand*)&smOverlayExplicitTargets;
					}

					mrLaunchSwarmersNearestTarget(targetSelection);
					closeOverlay = TRUE;
				}
				else if (index == 37)
				{
					mrCycleSelectionLeechMode();
					closeOverlay = FALSE;
				}
				else if (index == 25)
				{
					SelectAnyCommand* targetSelection;
					bool32 handledTarget;
					sdword targetIndex;

					targetSelection = (SelectAnyCommand*)&smOverlayDefaultTargets;
					if (smOverlayExplicitTargets.numTargets > 0)
					{
						targetSelection = (SelectAnyCommand*)&smOverlayExplicitTargets;
					}

					handledTarget = FALSE;
					for (targetIndex = 0; targetIndex < targetSelection->numTargets; targetIndex++)
					{
						SpaceObj* target;

						target = (SpaceObj*)targetSelection->TargetPtr[targetIndex];
						if (target->objtype == OBJ_ShipType)
						{
							if (shipModuleSlotIsEligible((Ship*)target, universe.curPlayerPtr))
							{
								mrHarvestResourcesTargetResource((Resource*)target);
								handledTarget = TRUE;
								break;
							}
						}
						else if (target->objtype >= OBJ_AsteroidType && target->objtype <= OBJ_DustType)
						{
							mrHarvestResourcesTargetResource((Resource*)target);
							handledTarget = TRUE;
							break;
						}
					}

					if (!handledTarget)
					{
						mrHarvestResourcesTargetResource(NULL);
					}
					closeOverlay = TRUE;
				}
				else if (index == 35)
				{
					mrCancelExternalConstructionSelection();
					closeOverlay = TRUE;
				}
				else
				{
					if (index != 1 && smOverlayMasterEntries[index].commandId != 0)
					{
						kbInvokeCommandCallbacks(smSensorsActive != 0, smOverlayMasterEntries[index].commandId);
					}
					closeOverlay = (index != 1 && index != 10);
				}
				clickedEntry = TRUE;
			}
		}
	}

	if (clickedEntry && closeOverlay)
	{
		smOverlayClose();
	}

	return clickedEntry;
}

static sdword smOverlayHandlePopupClickInternal(rectangle* viewportRect, sdword actionMenuOnly)
{
	sdword clickedEntry;

	clickedEntry = smOverlayHandleMouseClickInternal(actionMenuOnly);
	if (!clickedEntry && smClickedOnPlayer(viewportRect) == -1)
	{
		smOverlayClose();
		clickedEntry = TRUE;
	}

	return clickedEntry;
}

sdword smOverlayHandleSensorClick(void)
{
	return smOverlayHandlePopupClickInternal(&smViewRectangle, FALSE);
}

sdword smOverlayHandleMouseClick(void)
{
	return smOverlayHandlePopupClickInternal(&smViewRectangle, TRUE);
}

sdword smOverlayHandlePopupClick(rectangle* viewportRect)
{
	return smOverlayHandlePopupClickInternal(viewportRect, FALSE);
}

// Cataclysm-specific: draws the framed header box around one overlay column.
sdword __cdecl smDrawOverlayColumnHeaders(rectangle* rect, sdword fillBackground, udword borderSize)
{
	rectangle expanded;

	if (fillBackground)
	{
		expanded.x0 = rect->x0 - borderSize;
		expanded.y0 = rect->y0 - borderSize;
		expanded.x1 = rect->x1 + borderSize;
		expanded.y1 = rect->y1 + borderSize;
		if (RGLtype)
		{
			primRectTranslucent2(
				&expanded,
				smOverlayRglFillByte0 | ((smOverlayRglFillByte1 | ((smOverlayRglFillByte2 | (smOverlayRglFillByte3 << 8)) << 8)) << 8));
		}
		else
		{
			primRectSolid2(
				&expanded,
				smOverlaySoftwareFillByte0 | ((smOverlaySoftwareFillByte1 | ((smOverlaySoftwareFillByte2 | (smOverlaySoftwareFillByte3 << 8)) << 8)) << 8));
		}
	}

	opOverlayIconDraw(OPI_PopupMenu_BorderCorner_TopLeft, rect->y0 - borderSize, rect->x0 - borderSize, borderSize, borderSize, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderCorner_TopRight, rect->y0 - borderSize, rect->x1, borderSize, borderSize, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderCorner_BottomRight, rect->y1, rect->x1, borderSize, borderSize, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderCorner_BottomLeft, rect->y1, rect->x0 - borderSize, borderSize, borderSize, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderEdge_Top, rect->y0 - borderSize, rect->x0, rect->x1 - rect->x0, borderSize, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderEdge_Bottom, rect->y1, rect->x0, rect->x1 - rect->x0, borderSize, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderEdge_Left, rect->y0, rect->x0 - borderSize, borderSize, rect->y1 - rect->y0, colWhite);
	opOverlayIconDraw(OPI_PopupMenu_BorderEdge_Right, rect->y0, rect->x1, borderSize, rect->y1 - rect->y0, colWhite);
	return 0;
}

// Cataclysm-specific: draws one overlay entry list with hover and selection framing.
sdword __cdecl smDrawOverlayShipList(
	smOverlayEntry* entries,
	sdword count,
	sdword hasSubmenuColumn,
	sdword labelColumnWidth,
	sdword masterList)
{
	sdword submenuOffset;
	sdword index;
	char bindingText[256];
	char overrideBindingText[256];

	submenuOffset = (hasSubmenuColumn != 0) ? smOverlaySubmenuIndicatorWidth : 0;
	for (index = 0; index < count; index++)
	{
		smOverlayEntry* entry;
		color rowColor;
		sdword halfRowHeight;
		sdword highlightTop;
		sdword highlightHeight;
		sdword highlightLeft;

		entry = &entries[index];
		if (!entry->enabled)
		{
			continue;
		}

		if (entry->pulseHighlight)
		{
			real64 pulse;
			real64 blendValue;

			if (smOverlayPulsePeriod + smOverlayPulseStartTime < taskTimeElapsed)
			{
				smOverlayPulseStartTime = taskTimeElapsed;
			}
			else if (smOverlayPulseStartTime > taskTimeElapsed)
			{
				smOverlayPulseStartTime = taskTimeElapsed;
			}

			pulse = (taskTimeElapsed - smOverlayPulseStartTime) / smOverlayPulsePeriod;
			if (pulse > dbl_72A480)
			{
				blendValue = (pulse - dbl_72A548) * dbl_72AEC8;
			}
			else
			{
				blendValue = pulse + pulse;
			}
			rowColor = colBlend(smOverlayPulseColorStart, smOverlayPulseColorEnd, (real32)blendValue);
		}
		else if (entry->selectionLocked)
		{
			rowColor = smOverlayLockedRowColor;
		}
		else
		{
			rowColor = smOverlayDefaultRowColor;
		}

		if (masterList)
		{
			if (index == 0)
			{
				rowColor = smOverlaySelectedMasterRowColor;
			}
			if (index == smOverlayActiveViewIndex)
			{
				halfRowHeight = smOverlayRowPadding / 2;
				highlightTop = entry->y0 - halfRowHeight;
				highlightHeight = entry->y1 - entry->y0 + smOverlayRowPadding;
				highlightLeft = entry->x0 - smOverlayHighlightSidePadding;

				opOverlayIconDraw(OPI_PopupMenu_MouseOverLeftEnd, highlightTop, highlightLeft, smOverlayHighlightSidePadding + smOverlayHighlightCapWidth, highlightHeight, colWhite);
				opOverlayIconDraw(OPI_PopupMenu_MouseOverMiddle, highlightTop, entry->x0 + smOverlayHighlightCapWidth, entry->x1 - 2 * smOverlayHighlightCapWidth - entry->x0, highlightHeight, colWhite);
				opOverlayIconDraw(OPI_PopupMenu_MouseOverRightEnd, highlightTop, entry->x1 - smOverlayHighlightCapWidth, smOverlayHighlightCapWidth + smOverlayHighlightSidePadding, highlightHeight, colWhite);
			}
		}
		else if (index == nameIndex)
		{
			halfRowHeight = smOverlayRowPadding / 2;
			highlightTop = entry->y0 - halfRowHeight;
			highlightHeight = entry->y1 - entry->y0 + smOverlayRowPadding;
			highlightLeft = entry->x0 - smOverlayHighlightSidePadding;

			opOverlayIconDraw(OPI_PopupMenu_MouseOverLeftEnd, highlightTop, highlightLeft, smOverlayHighlightSidePadding + smOverlayHighlightCapWidth, highlightHeight, colWhite);
			opOverlayIconDraw(OPI_PopupMenu_MouseOverMiddle, highlightTop, entry->x0 + smOverlayHighlightCapWidth, entry->x1 - 2 * smOverlayHighlightCapWidth - entry->x0, highlightHeight, colWhite);
			opOverlayIconDraw(OPI_PopupMenu_MouseOverRightEnd, highlightTop, entry->x1 - smOverlayHighlightCapWidth, smOverlayHighlightCapWidth + smOverlayHighlightSidePadding, highlightHeight, colWhite);
		}

		if (entry->forceSubmenuColumn)
		{
			opOverlayIconDraw(OPI_PopupMenu_SelectedIndicator, entry->y0, entry->x0, smOverlaySubmenuIndicatorWidth, entry->y1 - entry->y0, colWhite);
		}

		rndAdditiveBlends(FALSE);
		if (entry->labelOverride != NULL)
		{
			fontPrint(submenuOffset + entry->x0, entry->y0, rowColor, entry->labelOverride);
			if (!entry->hasValueColumn && entry->commandId != 0)
			{
				kbFormatCommandBindings(entry->commandId, overrideBindingText);
				fontPrint(labelColumnWidth + submenuOffset + smOverlayCommandColumnGap + entry->x0, entry->y0, rowColor, overrideBindingText);
			}
		}
		else
		{
			fontPrint(submenuOffset + entry->x0, entry->y0, rowColor, strGetString(entry->labelStringIndex));
			if (entry->hasValueColumn)
			{
				opOverlayIconDraw(OPI_PopupMenu_SubMenuIndicator + (entry->selectionLocked != 0),
					entry->y0,
					labelColumnWidth + submenuOffset + smOverlayCommandColumnGap + entry->x0,
					smOverlayValueColumnWidth,
					entry->y1 - entry->y0,
					colWhite);
			}
			else if (entry->commandId != 0)
			{
				kbFormatCommandBindings(entry->commandId, bindingText);
				fontPrint(labelColumnWidth + submenuOffset + smOverlayCommandColumnGap + entry->x0, entry->y0, rowColor, bindingText);
			}
		}
	}

	return count;
}

color __cdecl smGetOverlayFadeColor(void)
{
	return smComputeOverlayFadeColor(0xffb8b838, 0);
}

bool32 smPathMarkersEnabledGet(void)
{
	return smPathMarkersEnabled;
}

bool32 smSensorBottomButtonPressedGet(void)
{
	return ioSensorBottomButtonPressedGet();
}

bool32 smSensorLeftButtonPressedGet(void)
{
	return ioSensorLeftButtonPressedGet();
}

bool32 smSensorRightButtonPressedGet(void)
{
	return ioSensorRightButtonPressedGet();
}

sdword smOverlayBuildActionAvailability(void);

static sdword smOverlayFindSelectableEntryIndex(smOverlayEntry* entries, sdword count, sdword startIndex)
{
	bool32 wrapped;
	sdword index;

	wrapped = FALSE;
	index = startIndex;
	while ((entries[index].selectionLocked == 1 || !entries[index].enabled) && !wrapped)
	{
		if (++index >= count)
		{
			index = 0;
		}
		if (index == startIndex)
		{
			wrapped = TRUE;
		}
	}
	return index;
}

static void smOverlayResetEntryState(smOverlayEntry* entries, sdword count)
{
	sdword index;

	for (index = 0; index < count; index++)
	{
		entries[index].enabled = FALSE;
		entries[index].selectionLocked = FALSE;
		entries[index].pulseHighlight = FALSE;
		entries[index].forceSubmenuColumn = FALSE;
		entries[index].hovered = FALSE;
	}
}

static void smOverlayClearEntryRuntimeState(smOverlayEntry* entries, sdword count)
{
	sdword index;

	for (index = 0; index < count; index++)
	{
		entries[index].selectionLocked = FALSE;
		entries[index].pulseHighlight = FALSE;
		entries[index].forceSubmenuColumn = FALSE;
		entries[index].hovered = FALSE;
	}
}

static mrMoveWaypointMode smOverlayMoveWaypointTypeGet(void)
{
	return (mrMoveWaypointMode)mrMoveWaypointType;
}

static char* smOverlayGetUpgradeLabel(void)
{
	return strGetString(
		(universe.curPlayerPtr->race == RACE_Beast) ? strMenuUpgradeBeastShipsDesc : strMenuUpgradeSectShipsDesc);
}

sdword smOverlayBuildCategoryData(void)
{
	sdword index;
	sdword validOwnedSelectionCount;
	sdword tacticFlags[3];
	sdword commandFlags[9];
	sdword enemyCount;
	sdword allyCount;
	bool32 rootEntry1Enabled;
	bool32 rootEntry2Enabled;
	bool32 rootEntry3Enabled;
	bool32 rootEntry4Enabled;
	bool32 rootEntry5Enabled;
	bool32 rootEntry6Enabled;
	bool32 rootEntry7Enabled;
	bool32 rootEntry8Enabled;
	bool32 rootEntry9Enabled;
	bool32 rootEntry10Enabled;
	bool32 rootEntry11Enabled;
	bool32 rootEntry12Enabled;
	bool32 rootEntry13Enabled;
	bool32 rootEntry14Enabled;
	bool32 rootEntry15Enabled;
	MaxSelection hyperspaceSelection;
	Ship* ship;
	CommandToDo* command;
	Player* player;
	sdword playerIndex;
	sdword pointSpecType;

	validOwnedSelectionCount = 0;
	enemyCount = 0;
	allyCount = 0;
	rootEntry1Enabled = FALSE;
	rootEntry2Enabled = FALSE;
	rootEntry3Enabled = FALSE;
	rootEntry4Enabled = FALSE;
	rootEntry5Enabled = FALSE;
	rootEntry6Enabled = FALSE;
	rootEntry7Enabled = FALSE;
	rootEntry8Enabled = FALSE;
	rootEntry9Enabled = FALSE;
	rootEntry10Enabled = FALSE;
	rootEntry11Enabled = FALSE;
	rootEntry12Enabled = FALSE;
	rootEntry13Enabled = FALSE;
	rootEntry14Enabled = FALSE;
	rootEntry15Enabled = FALSE;
	memset(tacticFlags, 0, sizeof(tacticFlags));
	memset(commandFlags, 0, sizeof(commandFlags));

	if (!singlePlayerGame && bitTest(tpGameCreated.flag, MG_Hyperspace))
	{
		selSelectionCopyBounded(&hyperspaceSelection, (MaxSelection*)&selSelected);
		MakeShipsMobile((SelectCommand*)&hyperspaceSelection.numShips);
		makeShipsControllable((SelectCommand*)&hyperspaceSelection.numShips, COMMAND_MP_HYPERSPACEING);
		makeSelectionHyperspaceCapable((SelectCommand*)&hyperspaceSelection.numShips);
		rootEntry4Enabled = (hyperspaceSelection.numShips != 0);
	}

	pointSpecType = pieCurrentPointSpecTypeGet();
	if (pointSpecType != PiePointSpecTypeNone)
	{
		if (pointSpecType <= 2)
		{
			rootEntry5Enabled = TRUE;
		}
		else if (pointSpecType == 4)
		{
			rootEntry7Enabled = TRUE;
			rootEntry8Enabled = TRUE;
			rootEntry9Enabled = TRUE;
			rootEntry10Enabled = TRUE;
		}
	}
	else if (selSelected.numShips > 0)
	{
		rootEntry1Enabled = TRUE;
		for (index = 0; index < selSelected.numShips; index++)
		{
			ship = selSelected.ShipPtr[index];
			if (bitTest(ship->flags, SOF_Hide | SOF_Disabled | SOF_Crazy) ||
				!(ship->flags & SOF_Selectable) ||
				ship->playerowner != universe.curPlayerPtr)
			{
				continue;
			}

			validOwnedSelectionCount++;

			if (shipIsMoveCapable(ship, TRUE))
			{
				rootEntry3Enabled = TRUE;
				rootEntry6Enabled = TRUE;
			}
			if (smClShipCanRetreatForRepairs(ship, TRUE))
			{
				rootEntry11Enabled = TRUE;
				rootEntry12Enabled = TRUE;
			}
			if (smShipCanDockForRepairs(ship, TRUE))
			{
				rootEntry14Enabled = TRUE;
				if (ship->tacticstype >= 0 && ship->tacticstype < 3)
				{
					tacticFlags[ship->tacticstype] = TRUE;
				}
			}
			if (shipIsNotDocked(ship, TRUE))
			{
				rootEntry13Enabled = TRUE;
				command = getShipAndItsCommand(&universe.mainCommandLayer, ship);
				if (command != NULL)
				{
					rootEntry15Enabled = TRUE;
					if (command->ordertype.order == COMMAND_MILITARYPARADE)
					{
						commandFlags[8] = TRUE;
					}
					else if ((command->ordertype.attributes & COMMAND_IS_FORMATION) != 0)
					{
						commandFlags[command->formation.formationtype] = TRUE;
					}
					else
					{
						commandFlags[7] = TRUE;
					}
				}
			}
			if (shipHasAvailableUpgradeChanges(ship))
			{
				rootEntry2Enabled = TRUE;
			}
		}
	}

	smOverlayResetEntryState(smOverlayMasterEntries, 49);
	smOverlayClearEntryRuntimeState(unk_88EA08, 3);
	smOverlayClearEntryRuntimeState(unk_88EAC0, 8);
	smOverlayClearEntryRuntimeState(unk_88ECA0, 12);
	smOverlayClearEntryRuntimeState(unk_88F060, 4);
	smOverlayClearEntryRuntimeState(unk_88F150, 8);
	smOverlayClearEntryRuntimeState(unk_88F330, 3);
	smOverlayResetEntryState(unk_A51D98, 49);
	smOverlayResetEntryState(unk_A52950, 49);
	smOverlayResetEntryState(smOverlayView22Entries, 49);
	smOverlayResetEntryState(smOverlayView23Entries, 49);

	smOverlayMasterEntries[1].enabled = rootEntry1Enabled;
	smOverlayMasterEntries[2].enabled = rootEntry2Enabled;
	if (rootEntry2Enabled)
	{
		smOverlayMasterEntries[2].pulseHighlight = TRUE;
		smOverlayMasterEntries[2].labelOverride = smOverlayGetUpgradeLabel();
	}
	smOverlayMasterEntries[3].enabled = rootEntry3Enabled;
	smOverlayMasterEntries[4].enabled = rootEntry4Enabled;
	smOverlayMasterEntries[5].enabled = rootEntry5Enabled;
	smOverlayMasterEntries[6].enabled = rootEntry6Enabled;
	smOverlayMasterEntries[7].enabled = rootEntry7Enabled;
	smOverlayMasterEntries[8].enabled = rootEntry8Enabled;
	smOverlayMasterEntries[9].enabled = rootEntry9Enabled;
	smOverlayMasterEntries[10].enabled = rootEntry10Enabled;
	smOverlayMasterEntries[11].enabled = rootEntry11Enabled;
	smOverlayMasterEntries[12].enabled = rootEntry12Enabled;
	smOverlayMasterEntries[13].enabled = rootEntry13Enabled;
	smOverlayMasterEntries[14].enabled = rootEntry14Enabled;
	smOverlayMasterEntries[15].enabled = FALSE;
	smOverlayMasterEntries[16].enabled = TRUE;
	smOverlayMasterEntries[17].enabled = TRUE;
	smOverlayMasterEntries[18].enabled = smOverlaySyncSensorMode();

	if (rootEntry8Enabled)
	{
		if (smOverlayMoveWaypointTypeGet() == MR_MoveWaypointTypeEndToEnd)
		{
			unk_88F330[2].forceSubmenuColumn = TRUE;
		}
		else if (smOverlayMoveWaypointTypeGet() == MR_MoveWaypointTypeCircular)
		{
			unk_88F330[1].forceSubmenuColumn = TRUE;
		}
		else
		{
			unk_88F330[0].forceSubmenuColumn = TRUE;
		}
	}

	if (rootEntry14Enabled && smOverlayActiveViewIndex == 14)
	{
		selSelectionCopyBounded(&hyperspaceSelection, (MaxSelection*)&selSelected);
		MakeShipsNonCapital((SelectCommand*)&hyperspaceSelection.numShips);
		if (hyperspaceSelection.numShips == 0)
		{
			unk_88EA08[0].selectionLocked = TRUE;
		}
		validOwnedSelectionCount = 0;
		for (index = 0; index < 3; index++)
		{
			if (tacticFlags[index])
			{
				validOwnedSelectionCount++;
			}
		}
		for (index = 0; index < 3; index++)
		{
			if (validOwnedSelectionCount == 1 && tacticFlags[index])
			{
				unk_88EA08[index].forceSubmenuColumn = TRUE;
			}
		}
	}

	if (rootEntry15Enabled && smOverlayActiveViewIndex == 15)
	{
		validOwnedSelectionCount = 0;
		for (index = 0; index < 9; index++)
		{
			if (commandFlags[index])
			{
				validOwnedSelectionCount++;
			}
		}
		smOverlayMasterEntries[15].enabled = (validOwnedSelectionCount != 0);
		for (index = 0; index < 7; index++)
		{
			if (validOwnedSelectionCount == 1 && commandFlags[index])
			{
				unk_88EAC0[index].forceSubmenuColumn = TRUE;
			}
		}
		if (validOwnedSelectionCount == 1 && commandFlags[8])
		{
			unk_88EAC0[7].forceSubmenuColumn = TRUE;
		}
		unk_88EAC0[7].enabled = (MothershipOrCarrierIndexInSelection((SelectCommand*)&selSelected) != -1);
	}
	else if (rootEntry15Enabled)
	{
		for (index = 0; index < 9; index++)
		{
			if (commandFlags[index])
			{
				smOverlayMasterEntries[15].enabled = TRUE;
				break;
			}
		}
	}

	unk_88F060[0].enabled = (selSelected.numShips > 0);
	unk_88F060[1].enabled = TRUE;
	unk_88F060[2].enabled = TRUE;

	unk_88F150[0].enabled = !smSensorsActive;
	unk_88F150[1].enabled = smSensorsActive;
	unk_88F150[6].forceSubmenuColumn = smPathMarkersEnabledGet();
	unk_88F150[7].forceSubmenuColumn = (((sdword)universe.mainCameraCommand.dontFocusOnMe & 8) != 0);

	smOverlayView19Count = 0;
	if (selSelected.numShips == 1 && shipCanHaveName(selSelected.ShipPtr[0]))
	{
		smOverlayMasterEntries[0].enabled = TRUE;
		smOverlayMasterEntries[0].selectionLocked = TRUE;
		smOverlayMasterEntries[0].labelOverride = shipNameGet(selSelected.ShipPtr[0]);

		if (!(singlePlayerGame && selSelected.ShipPtr[0]->shiptype == 18))
		{
			smOverlayMasterEntries[19].enabled = TRUE;
			if (smOverlayActiveViewIndex == 19)
			{
				smOverlayView19Count = shipNamesCountFreeByType(selSelected.ShipPtr[0]->shiptype);
				for (index = 0; index < smOverlayView19Count; index++)
				{
					unk_A51D98[index].enabled = TRUE;
					unk_A51D98[index].labelOverride = shipNamesGetFreeByTypeIndex(selSelected.ShipPtr[0]->shiptype, index);
				}
			}
		}
	}

	if (singlePlayerHyperspaceObjectiveActive())
	{
		smOverlayMasterEntries[20].enabled = TRUE;
		smOverlayMasterEntries[20].pulseHighlight = TRUE;
		smOverlayMasterEntries[20].labelOverride = singlePlayerGetHyperspaceObjectiveLabel();
	}

	if (multiPlayerGame &&
		bitTest(tpGameCreated.flag, BIT4) &&
		universe.curPlayerPtr != NULL &&
		universe.curPlayerPtr->playerState != PLAYER_DEAD)
	{
		player = universe.players;
		for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++, player++)
		{
			if (playerIndex != universe.curPlayerIndex &&
				player->playerState != PLAYER_DEAD &&
				!allianceArePlayersAllied(universe.curPlayerPtr, player))
			{
				unk_A52950[enemyCount].enabled = TRUE;
				unk_A52950[enemyCount].labelOverride = playerNames[playerIndex];
				enemyCount++;
			}
		}
		smOverlayView21Count = enemyCount;
		smOverlayMasterEntries[21].enabled = (enemyCount != 0);

		player = universe.players;
		for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++, player++)
		{
			if (playerIndex != universe.curPlayerIndex &&
				player->playerState != PLAYER_DEAD &&
				allianceArePlayersAllied(universe.curPlayerPtr, player))
			{
				smOverlayView22Entries[allyCount].enabled = TRUE;
				smOverlayView22Entries[allyCount].labelOverride = playerNames[playerIndex];
				smOverlayView23Entries[allyCount].enabled = TRUE;
				smOverlayView23Entries[allyCount].labelOverride = playerNames[playerIndex];
				allyCount++;
			}
		}
		smOverlayView22Count = allyCount;
		smOverlayRuTransferPlayerCount = allyCount;
		smOverlayMasterEntries[22].enabled = (allyCount != 0);
		smOverlayMasterEntries[23].enabled = (allyCount != 0);
	}
	else
	{
		smOverlayView21Count = 0;
		smOverlayView22Count = 0;
		smOverlayRuTransferPlayerCount = 0;
		smOverlayMasterEntries[21].enabled = FALSE;
		smOverlayMasterEntries[22].enabled = FALSE;
		smOverlayMasterEntries[23].enabled = FALSE;
	}

	return 0;
}

static void smOverlaySelectActionMasterView(sdword viewIndex)
{
	smOverlayActiveViewIndex = smOverlayFindSelectableEntryIndex(smOverlayMasterEntries, 49, viewIndex);
	if (smOverlayActiveViewIndex >= 24 && !smOverlayExplicitTargets.numTargets)
	{
		smOverlaySelectDefaultTargetForView((TargetPtr)smOverlayActiveViewIndex);
	}
}

static void smOverlayEnsureActionMenuOpen(void)
{
	if (playPackets || (smOverlayIsOpen && smOverlayIsActionMenu == 1))
	{
		return;
	}

	smOverlayIsOpen = 1;
	smOverlayAnchorX = screenX;
	smOverlayAnchorY = screenY;
	smOverlayIsActionMenu = 1;
	smOverlayExplicitTargets.numTargets = 0;
	smOverlayDefaultTargets.numTargets = 0;
	smOverlayBuildActionAvailability();
	smOverlaySelectActionMasterView(0);
	nameIndex = -1;
	telldir();
	tutGameMessage("Game_PopupOpen");
}

void smOverlayOpen(sdword actionMenu, SelectAnyCommand* selection)
{
	sdword index;

	if (playPackets || (smOverlayIsOpen && actionMenu == smOverlayIsActionMenu))
	{
		return;
	}

	smOverlayIsOpen = 1;
	smOverlayAnchorX = screenX;
	smOverlayAnchorY = screenY;
	smOverlayIsActionMenu = actionMenu;
	smOverlayExplicitTargets.numTargets = 0;
	if (selection != NULL)
	{
		smOverlayExplicitTargets.numTargets = selection->numTargets;
		for (index = 0; index < selection->numTargets; index++)
		{
			smOverlayExplicitTargets.TargetPtr[index] = selection->TargetPtr[index];
		}
	}

	smOverlayDefaultTargets.numTargets = 0;
	if (actionMenu)
	{
		smOverlayBuildActionAvailability();
	}
	else
	{
		smOverlayBuildCategoryData();
	}
	smOverlayActiveViewIndex = smOverlayFindSelectableEntryIndex(smOverlayMasterEntries, 49, 0);
	if (smOverlayActiveViewIndex >= 24 && !smOverlayExplicitTargets.numTargets)
	{
		smOverlaySelectDefaultTargetForView((TargetPtr)smOverlayActiveViewIndex);
	}

	nameIndex = -1;
	telldir();
	tutGameMessage("Game_PopupOpen");
}

bool32 smOverlayIsOpenGet(void)
{
	return smOverlayIsOpen;
}

sdword smOverlayBuildActionAvailability(void)
{
	sdword anyActionEnabled;
	sdword targetActions[41];
	sdword selectionActions[41];
	MaxSelection selection;
	SelectAnyCommand* currentTargets;
	sdword numTargets;
	sdword currentView;
	sdword mimicGroupIndex;
	sdword index;
	sdword entryCount;
	sdword currentPlayerIndex;

	anyActionEnabled = FALSE;
	currentTargets = (smOverlayExplicitTargets.numTargets != 0) ? (SelectAnyCommand*)&smOverlayExplicitTargets : (SelectAnyCommand*)&smOverlayDefaultTargets;
	mrDetermineSelectionActions(targetActions, currentTargets);

	selSelectionCopyBounded(&selection, (MaxSelection*)&selSelected);
	MakeShipsNotDockedExceptSentinelLeechTypes((SelectCommand*)&selection.numShips);
	mrDetermineSelectionActions(selectionActions, (SelectAnyCommand*)&selection.numShips);

	for (index = 0; index < 49; index++)
	{
		smOverlayMasterEntries[index].enabled = FALSE;
	}

	numTargets = smOverlayExplicitTargets.numTargets;
	if (selectionActions[0] && (!smOverlayExplicitTargets.numTargets || targetActions[6]))
	{
		smOverlayMasterEntries[25].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[2] && (!smOverlayExplicitTargets.numTargets || targetActions[5]))
	{
		smOverlayMasterEntries[26].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[1] && (!smOverlayExplicitTargets.numTargets || targetActions[7]))
	{
		smOverlayMasterEntries[27].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[11] && (!smOverlayExplicitTargets.numTargets || targetActions[4]))
	{
		smOverlayMasterEntries[40].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[13] && (!smOverlayExplicitTargets.numTargets || targetActions[4]))
	{
		smOverlayMasterEntries[45].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[20] && (!smOverlayExplicitTargets.numTargets || targetActions[4]))
	{
		smOverlayMasterEntries[44].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[14] && (!smOverlayExplicitTargets.numTargets || targetActions[4]))
	{
		smOverlayMasterEntries[29].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[16] && !smOverlayExplicitTargets.numTargets)
	{
		smOverlayMasterEntries[42].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[17] && !smOverlayExplicitTargets.numTargets)
	{
		smOverlayMasterEntries[43].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[21])
	{
		smOverlayMasterEntries[46].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[31])
	{
		smOverlayMasterEntries[48].enabled = TRUE;
		anyActionEnabled = TRUE;
	}

	currentView = smOverlayActiveViewIndex;
	if (selectionActions[14])
	{
		smOverlayMasterEntries[28].enabled = TRUE;
		anyActionEnabled = TRUE;
		if (currentView == 28)
		{
			unk_88EF70[0].enabled = TRUE;
			unk_88EF70[0].forceSubmenuColumn = FALSE;
			unk_88EF70[1].enabled = TRUE;
			unk_88EF70[1].forceSubmenuColumn = FALSE;
			unk_88EF70[2].enabled = TRUE;
			unk_88EF70[2].forceSubmenuColumn = FALSE;
			if ((ubyte)selectionActions[40] == 6)
			{
				unk_88EF70[0].forceSubmenuColumn = TRUE;
			}
			else if ((ubyte)selectionActions[40] == 7)
			{
				unk_88EF70[1].forceSubmenuColumn = TRUE;
			}
			else
			{
				unk_88EF70[2].forceSubmenuColumn = ((ubyte)selectionActions[40] == 8);
			}
		}
	}

	if (selectionActions[12] && !smOverlayExplicitTargets.numTargets)
	{
		smOverlayMasterEntries[41].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[15] && !smOverlayExplicitTargets.numTargets)
	{
		smOverlayMasterEntries[30].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[18] && !smOverlayExplicitTargets.numTargets)
	{
		smOverlayMasterEntries[31].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[19] && !smOverlayExplicitTargets.numTargets)
	{
		smOverlayMasterEntries[32].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[22] && !numTargets)
	{
		smOverlayMasterEntries[33].enabled = TRUE;
		smOverlayMasterEntries[33].selectionLocked = FALSE;
		if (currentView == 33)
		{
			for (index = 0; index < SM_MIMIC_PLAYER_LABEL_COUNT; index++)
			{
				if (smMimicIsPlayerIndexValid(index))
				{
					unk_A57270[index].groupIndex = 0;
					unk_A57270[index].enabled = TRUE;
					unk_A57270[index].selectionLocked = (smMimicGetCurrentPlayerIndex() == index);
					unk_A57270[index].pulseHighlight = FALSE;
					unk_A57270[index].forceSubmenuColumn = FALSE;
					sprintf(smMimicPlayerLabels[index], "%d. %s", index, smMimicGetPlayerLabel(index));
					unk_A57270[index].labelOverride = smMimicPlayerLabels[index];
				}
				else
				{
					unk_A57270[index].enabled = FALSE;
				}
			}
			smOverlayView33Count = SM_MIMIC_PLAYER_LABEL_COUNT;
			numTargets = smOverlayExplicitTargets.numTargets;
			currentView = smOverlayActiveViewIndex;
		}

		mimicGroupIndex = selectionActions[23];
		smOverlayMasterEntries[34].enabled = TRUE;
		smOverlayMasterEntries[34].selectionLocked = FALSE;
		if (currentView == 34)
		{
			currentPlayerIndex = smMimicGetCurrentPlayerIndex();
			entryCount = smMimicGetEntryCount(currentPlayerIndex, mimicGroupIndex);
			dbgAssert(entryCount <= SM_MIMIC_ENTRY_LABEL_COUNT);
			for (index = 0; index < entryCount; index++)
			{
				unk_A53CC0[index].selectionLocked = FALSE;
				unk_A53CC0[index].enabled = TRUE;
				unk_A53CC0[index].pulseHighlight = FALSE;
				unk_A53CC0[index].groupIndex = 0;
				sprintf(smMimicEntryLabels[index], "%d. %s", index, smMimicGetEntryLabel(currentPlayerIndex, mimicGroupIndex, index));
				unk_A53CC0[index].labelOverride = smMimicEntryLabels[index];
			}
			smOverlayView34Count = entryCount;
			numTargets = smOverlayExplicitTargets.numTargets;
		}
		if (selectionActions[24])
		{
			smOverlayMasterEntries[35].enabled = TRUE;
			smOverlayMasterEntries[35].selectionLocked = FALSE;
		}
		else
		{
			smOverlayMasterEntries[35].enabled = FALSE;
		}
		anyActionEnabled = TRUE;
		smOverlayMimicGroupIndex = mimicGroupIndex;
	}
	if (selectionActions[25] && !numTargets)
	{
		smOverlayMasterEntries[47].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[29] && !numTargets)
	{
		smOverlayMasterEntries[39].enabled = TRUE;
		anyActionEnabled = TRUE;
	}
	if (selectionActions[28] && !numTargets)
	{
		smOverlayMasterEntries[38].enabled = TRUE;
		anyActionEnabled = TRUE;
	}

	if (selectionActions[26])
	{
		smOverlayMasterEntries[24].enabled = FALSE;
		if (!selectionActions[27])
		{
			smOverlayMasterEntries[36].enabled = TRUE;
			return selectionActions[27];
		}

		smOverlayMasterEntries[37].enabled = TRUE;
		return selectionActions[27];
	}
	if (selectionActions[27])
	{
		smOverlayMasterEntries[24].enabled = FALSE;
		smOverlayMasterEntries[37].enabled = TRUE;
		return selectionActions[27];
	}

	smOverlayMasterEntries[24].enabled = !anyActionEnabled;
	return selectionActions[27];
}

sdword smOverlayOpenView28(void)
{
	smOverlayEnsureActionMenuOpen();
	smOverlaySelectActionMasterView(28);
	nameIndex = smOverlayFindSelectableEntryIndex(unk_88EF70, 4, 0);
	return nameIndex * sizeof(smOverlayEntry);
}

sdword smOverlayOpenView33(void)
{
	smOverlayEnsureActionMenuOpen();
	smOverlaySelectActionMasterView(33);
	nameIndex = smOverlayFindSelectableEntryIndex(unk_A57270, smOverlayView33Count, 0);
	return nameIndex * sizeof(smOverlayEntry);
}

sdword smOverlayOpenView34(void)
{
	smOverlayEnsureActionMenuOpen();
	smOverlaySelectActionMasterView(34);
	nameIndex = smOverlayFindSelectableEntryIndex(unk_A53CC0, smOverlayView34Count, 0);
	return nameIndex * sizeof(smOverlayEntry);
}

static bool32 smShipCanUseBigGun(Ship* ship)
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

static bool32 smClShipCanRetreatForRepairs(Ship* ship, bool32 considerSingleShip)
{
	ShipStaticInfo* staticInfo;
	sdword v3;

	staticInfo = ship->staticinfo;
	v3 = staticInfo->shipclass;
	if (v3 == 0 || v3 == 10 || staticInfo->externalBuild != 0)
	{
		return FALSE;
	}

	return !considerSingleShip || ship->cataBusyStateWindow.dockingship == NULL;
}

static bool32 smShipCanDockForRepairs(Ship* ship, bool32 considerSingleShip)
{
	if (considerSingleShip && ship->cataBusyStateWindow.dockingship != NULL)
	{
		return FALSE;
	}
	if (ship->shiprace == 9)
	{
		return FALSE;
	}

	return ((~*(ubyte*)&ship->flags2) >> 1) & 1;
}

static bool32 smOverlayShipMatchesCurrentView(Ship* ship, MaxSelection* leechSelection)
{
	switch (smOverlayActiveViewIndex)
	{
		case 2:
		return !ShipStaticInfoUpgradeByDockingEnabled(ship->staticinfo) && shipHasAvailableUpgradeChanges(ship);

		case 3:
		case 6:
			return shipIsMoveCapable(ship, TRUE);

		case 11:
		case 12:
			return smClShipCanRetreatForRepairs(ship, TRUE);

		case 13:
			return shipIsNotDocked(ship, TRUE) &&
				getShipAndItsCommand(&universe.mainCommandLayer, ship) != NULL;

		case 14:
			return smShipCanDockForRepairs(ship, TRUE);

		case 15:
			return shipIsNotDocked(ship, TRUE);

		case 25:
			return shipCanResearch(ship, TRUE);

		case 26:
			return shipCanSingleClickSpecialActivate(ship);

		case 27:
			return shipCanSpecialTarget(ship);

		case 28:
		case 29:
			return shipCanUseInfectionAbility(ship, TRUE);

		case 30:
			return shipIsInfected(ship);

		case 31:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_LINK_PRIMARY);

		case 32:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_LINK_SECONDARY);

		case 33:
		case 34:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MIMIC);

		case 35:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MIMIC) && shipHasModuleSlotState(ship);

		case 36:
			selSelectionAddSingleShipNoCheck(leechSelection, ship);
			MakeShipsLeechCapable((SelectCommand*)leechSelection);
			return FALSE;

		case 37:
			return salCapIsActiveOrBeaming(ship);

		case 38:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CLOAKING) && !univIsShipCloaked((SpaceObj*)ship);

		case 39:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_CLOAKING) && univIsShipCloaked((SpaceObj*)ship);

		case 40:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY);

		case 41:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY) && shipHasReadySwarmers(ship);

		case 42:
			return mothershipHasReadyBigGun(ship);

		case 43:
			return mothershipBigGunCanFire(ship);

		case 44:
			return smShipCanUseBigGun(ship);

		case 45:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EMP);

		case 46:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER);

		case 47:
			return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPULSE);

		case 48:
			return acolyteIsLinked(ship);

		default:
			return FALSE;
	}
}

static const color catSmEnemySpecialColor = colRGB(120, 120, 120);
static const color catSmCrystalPointColor = colRGB(255, 0, 255);

static bool32 cat_smPointVisibleByFog(real32 x, real32 y, real32 z, bool32 useShipRangeCheck)
{
	if (!smFogVisibilityMode)
	{
		return TRUE;
	}
	if (!universe.curPlayerPtr->fogGrid)
	{
		return (udword)smFogVisibilityMode <= 1;
	}

	univSetCurrentSensorCoverage((sdword)universe.curPlayerPtr->fogGrid);
	if (fogPointInGridBounds(x, y, z))
	{
		if (useShipRangeCheck)
		{
			return CheckIfShipInRange(x, y, z) > 0;
		}
		return fogGetGridValue(x, y, z) > 0;
	}
	return (udword)smFogVisibilityMode <= 1;
}

static color cat_smShipPointColor(Ship* ship)
{
	// Retail cat_smRenderShips (0x50A310) shows player-owned ships with Ship+0x1CC BIT20
	// using the allied sensors color instead of the friendly one.
	if (ship->playerowner == universe.curPlayerPtr)
	{
		if ((ship->specialFlags2 & BIT20) == 0)
		{
			return teFriendlyColor;
		}
		return teAlliedColor;
	}
	if (allianceArePlayersAllied(ship->playerowner, universe.curPlayerPtr))
	{
		return teAlliedColor;
	}
	if (smIsCrystal(ship))
	{
		return catSmCrystalPointColor;
	}
	if ((ship->flags2 & 8) != 0 &&
		shipModuleSlotIsEligible(ship, universe.curPlayerPtr) &&
		singlePlayerGame)
	{
		return teResourceColor;
	}
	if ((ship->flags2 & 0x10) != 0)
	{
		return catSmEnemySpecialColor;
	}
	return teHostileColor;
}

static real32 cat_smCapitalOverlayMinScale(void)
{
	return primScreenToGLScaleX(TO_MinimumScreenSize);
}

// Cataclysm-specific: render a ship mesh and any slaved children in sensors view.
bool32 __cdecl cat_smRenderShipAndSlaves(SpaceObj* spaceobj)
{
	Ship* ship;
	lod* level;
	hmatrix modelMatrix;
	bool32 lightingEnabled;
	bool32 texturesEnabled;
	bool8 previousWireframeHack;
	SpaceObjSelection* attachedSelection;
	sdword attachedIndex;

	ship = (Ship*)spaceobj;
	level = lodLevelGetSimple(spaceobj, &smCamera.eyeposition, &spaceobj->posinfo.position);
	if ((level->flags & LM_LODType) != LT_Mesh)
	{
		return FALSE;
	}

	glPushMatrix();
	hmatMakeHMatFromMat(&modelMatrix, &((SpaceObjRot*)spaceobj)->rotinfo.coordsys);
	hmatPutVectIntoHMatrixCol4(spaceobj->posinfo.position, modelMatrix);
	glMultMatrixf(&modelMatrix.m11);

	lightingEnabled = rndLightingEnable(TRUE);
	texturesEnabled = rndTextureEnable(TRUE);
	shPushLightMatrix(&modelMatrix);
	if (rndShipVisible(spaceobj, &smCamera))
	{
		extern bool8 g_WireframeHack;

		previousWireframeHack = g_WireframeHack;
		g_WireframeHack = FALSE;
		if (ship->bindings != NULL)
		{
			meshRenderShipHierarchyForShip(ship, ship->bindings, ship->currentLOD, (meshdata*)level->pData, ship->colorScheme);
		}
		else
		{
			meshRender((meshdata*)level->pData, ship->colorScheme);
		}

		attachedSelection = (ship->clampInfo != NULL) ? (SpaceObjSelection*)ship->clampInfo->selection : NULL;
		if (attachedSelection != NULL)
		{
			for (attachedIndex = 0; attachedIndex < attachedSelection->numSpaceObjs; attachedIndex++)
			{
				SpaceObj* attachedObj;
				Ship* attachedShip;
				lod* attachedLevel;

				attachedObj = attachedSelection->SpaceObjPtr[attachedIndex];
				attachedShip = (Ship*)attachedObj;
				attachedLevel = lodLevelGetSimple(attachedObj, &smCamera.eyeposition, &attachedObj->posinfo.position);
				if ((attachedLevel->flags & LM_LODType) == LT_Mesh)
				{
					if (attachedShip->bindings != NULL)
					{
						meshRenderShipHierarchyForShip(
							attachedShip,
							attachedShip->bindings,
							attachedShip->currentLOD,
							(meshdata*)attachedLevel->pData,
							attachedShip->colorScheme);
					}
					else
					{
						meshRender((meshdata*)attachedLevel->pData, attachedShip->colorScheme);
					}
				}
			}
		}
		g_WireframeHack = previousWireframeHack;
	}
	shPopLightMatrix();
	rndLightingEnable(lightingEnabled);
	rndTextureEnable(texturesEnabled);
	glPopMatrix();
	return TRUE;
}

// Cataclysm-specific: builds the tactical-overlay highlight set for the current view.
sdword smOverlayBuildEligibleSelection(void)
{
	MaxSelection eligibleSelection = {0};
	SelectAnyCommand* targetSelection;
	sdword index;
	sdword result;

	result = smSensorsActive;
	if (smSensorsActive)
	{
		return result;
	}

	for (index = 0; index < selSelected.numShips; index++)
	{
		Ship* ship;

		ship = selSelected.ShipPtr[index];
		if (smOverlayShipMatchesCurrentView(ship, &eligibleSelection))
		{
			selSelectionAddSingleShipNoCheck(&eligibleSelection, ship);
		}
	}

	for (index = 0; index < eligibleSelection.numShips; index++)
	{
		smDrawProjectedShipOutline(eligibleSelection.ShipPtr[index], colWhite);
	}

	result = smOverlayIsActionMenu;
	if (!smOverlayIsActionMenu)
	{
		return result;
	}

	targetSelection = (SelectAnyCommand*)&smOverlayDefaultTargets;
	if (smOverlayExplicitTargets.numTargets > 0)
	{
		targetSelection = (SelectAnyCommand*)&smOverlayExplicitTargets;
	}

	result = targetSelection->numTargets;
	for (index = 0; index < targetSelection->numTargets; index++)
	{
		Ship* target;

		target = (Ship*)targetSelection->TargetPtr[index];
		if (target->objtype == OBJ_ShipType &&
			(univIsObjFriendly((SpaceObj*)target, universe.curPlayerPtr) ||
				!bitTest(target->flags, SOF_Cloaked) ||
				shipGetSlotStatus(target, universe.curPlayerIndex) ||
				(!shipModuleSlotIsEligible(target, universe.curPlayerPtr) &&
					shipIsModuleSlotActive(target, universe.curPlayerPtr))))
		{
			smDrawProjectedShipOutline(target, colWhite);
		}
	}

	return result;
}

sdword smOverlayActivateMimicModeByIndex(sdword index)
{
	sdword currentGroupIndex;
	sdword* entryData;
	sdword operation;
	sdword selectionIndex;
	sdword currentPlayerIndex;
	Ship** shipPtr;

	currentGroupIndex = smOverlayMimicGroupIndex;
	currentPlayerIndex = smMimicGetCurrentPlayerIndex();
	entryData = (sdword*)smMimicGetEntryAddress((ubyte)currentPlayerIndex, currentGroupIndex, index);
	operation = (currentPlayerIndex == 8) ? 2 : 1;
	selectionIndex = 0;
	shipPtr = selSelected.ShipPtr;
	while (selectionIndex < selSelected.numShips)
	{
		Ship* ship;

		ship = *shipPtr;
		if (ship->dockvars.dockship != NULL)
		{
			selSelected.numShips--;
			*shipPtr = selSelected.ShipPtr[selSelected.numShips];
			continue;
		}
		if (ship->shiptype != sMimic && ship->shiptype != sMCV)
		{
			selSelected.numShips--;
			*shipPtr = selSelected.ShipPtr[selSelected.numShips];
			continue;
		}
		if (ship->shiptype == sMimic &&
			index >= smMimicGetEntryCount((ubyte)currentPlayerIndex, 0))
		{
			selSelected.numShips--;
			*shipPtr = selSelected.ShipPtr[selSelected.numShips];
			continue;
		}

		selectionIndex++;
		shipPtr++;
	}

	if (selSelected.numShips > 0)
	{
		clWrapExternalConstructionSelection(
			&universe.mainCommandLayer,
			(SelectCommand*)&selSelected,
			operation,
			entryData[0],
			currentPlayerIndex,
			entryData[1]);
	}

	return TRUE;
}

#if 0  // Audited missing Cataclysm sensors/tactical-overlay functions
sdword smOverlayActivateAttackOnCurrentTargets(void);
sdword smOverlayActivateMimicModeByIndex(sdword index);
sdword smOverlayBuildCategoryData(void);
sdword smOverlayGetViewInfo(sdword* viewInfo);
#endif





// Cataclysm-specific: render per-ship sensors embellishments that are separate from blob drawing.
void __cdecl cat_smRenderShips(hmatrix* modelView, hmatrix* projection)
{
	Node* node;
	bool32 selectionFlashOff;

	selectionFlashOff = fmod(taskTimeElapsed, smSelectedFlashSpeed) > (smSelectedFlashSpeed * 0.5f);
	for (node = universe.ShipList.head; node != NULL; node = node->next)
	{
		Ship* ship;
		sdword bucketIndex;
		bool32 drawShipMarker;
		bool32 drawPoint;
		bool32 drawPointIcon;
		color pointColor;

		ship = (Ship*)listGetStructOfNode(node);
		drawPoint = TRUE;
		drawPointIcon = TRUE;
		if ((ship->flags & (SOF_Targetable | SOF_Selectable)) != 0 || smShipIsBeastShip(ship))
		{
			selCircleCompute(modelView, projection, (SpaceObjRotImpTarg*)ship);
		}

		if (!allianceArePlayersAllied(ship->playerowner, universe.curPlayerPtr) &&
			universe.curPlayerPtr->playerState != PLAYER_DEAD)
		{
			bool32 visible;

			visible = smShipVisibleToPlayer(ship);
			if ((!singlePlayerGame &&
				((((ship->flags2 & 8) != 0) && shipIsModuleSlotActive(ship, universe.curPlayerPtr)) ||
					!shipGetSlotStatus(ship, universe.curPlayerPtr->playerIndex))) ||
				!visible)
			{
				drawPoint = FALSE;
			}
		}

		bucketIndex = cat_smGetShipRenderBucketIndex(ship);
		if (ship->staticinfo->shipclass == 10 ||
			ship->shiptype == 9 ||
			ship->shiptype == 43)
		{
			drawPoint = FALSE;
		}
		if (ShipInSelection((SelectCommand*)&selSelected, ship) && !selectionFlashOff)
		{
			drawPoint = FALSE;
		}

		if ((ship->staticinfo->shipFlags & 0x4000) != 0)
		{
			catSmShipLinkOverlay* linkOverlay;

			linkOverlay = (catSmShipLinkOverlay*)ship->ShipSpecifics;
			if (linkOverlay->effect != NULL &&
				cat_smPointVisibleByFog(
					ship->posinfo.position.x,
					ship->posinfo.position.y,
					ship->posinfo.position.z,
					FALSE))
			{
				etgEffectUpdate(linkOverlay->effect, 0.0f);
				etgEffectDraw(linkOverlay->effect);
			}

			rndSetDepthTesting(FALSE);
			rndTextureEnabled = TRUE;
			rndTextureEnable(FALSE);
			if (showSlipgatePaths &&
				linkOverlay->linkedShip != NULL &&
				ship > linkOverlay->linkedShip &&
				cat_smPointVisibleByFog(
					linkOverlay->linkedShip->posinfo.position.x,
					linkOverlay->linkedShip->posinfo.position.y,
					linkOverlay->linkedShip->posinfo.position.z,
					FALSE))
			{
				catSmShipLinkOverlay* linkedOverlay;
				vector p0;
				vector p1;
				vector tangent0;
				vector tangent1;
				vector diff;
				real32 speedX;
				real32 speedY;
				real32 speedZ;
				real32 linkDistance;
				sdword stepIndex;

				linkedOverlay = (catSmShipLinkOverlay*)linkOverlay->linkedShip->ShipSpecifics;
				if (linkOverlay->playerTransitState[universe.curPlayerIndex] != 0 ||
					linkedOverlay->playerTransitState[universe.curPlayerIndex] != 0)
				{
					p0 = ship->posinfo.position;
					p1 = linkOverlay->linkedShip->posinfo.position;
					vecSub(diff, p1, p0);
					linkDistance = fsqrt(vecMagnitudeSquared(diff));
					tangent0.x = 0.0f;
					tangent0.y = 0.0f;
					tangent0.z = (sinf(0.05f * taskTimeElapsed * 3.1415927f) * 0.1f + 1.2f) * linkDistance;
					matMultiplyMatByVec(&tangent0, &linkOverlay->linkedShip->rotinfo.coordsys, &tangent0);
					tangent1.x = 0.0f;
					tangent1.y = 0.0f;
					tangent1.z = (cosf(0.05f * taskTimeElapsed * 3.1415927f) * 0.1f + 1.2f) * -linkDistance;
					matMultiplyMatByVec(&tangent1, &ship->rotinfo.coordsys, &tangent1);

					for (stepIndex = 1; stepIndex < 20; stepIndex++)
					{
						real32 step;
						vector startPos;

						step = (real32)stepIndex / 19.0f;
						startPos = p0;
						speedX = tangent1.x;
						speedY = tangent1.y;
						speedZ = tangent1.z;
						CubicLogUnsigned(&startPos.x, &speedX, p1.x, tangent0.x, step, 3.1415927f);
						CubicLogUnsigned(&startPos.y, &speedY, p1.y, tangent0.y, step, 3.1415927f);
						CubicLogUnsigned(&startPos.z, &speedZ, p1.z, tangent0.z, step, 3.1415927f);
						primLine3Smooth(&p0, &startPos, linkOverlay->linkColor);
						p0 = startPos;
					}
				}
			}
			drawPoint = FALSE;
		}

		drawShipMarker = smIsCrystal(ship) || drawPoint;
		if (drawShipMarker)
		{
			if ((ship->flags2 & 8) != 0 &&
				shipHasModuleSlotState(ship) &&
				univGetShipRace((SpaceObj*)ship) >= 0 &&
				ship->playerowner != universe.curPlayerPtr)
			{
				if ((catSmShipRenderFlags[univGetShipRace((SpaceObj*)ship)] & 2) != 0 &&
					showEnemyShips &&
					cat_smRenderShipAndSlaves((SpaceObj*)ship))
				{
					drawPointIcon = FALSE;
				}
			}
			else if ((catSmShipRenderFlags[ship->shiptype] & 2) != 0 &&
				showEnemyShips &&
				cat_smRenderShipAndSlaves((SpaceObj*)ship))
			{
				drawPointIcon = FALSE;
			}

			rndSetDepthTesting(FALSE);
			if (drawPointIcon)
			{
				pointColor = cat_smShipPointColor(ship);
				glPointSize(2.0f);
				primPoint3(&ship->posinfo.position, pointColor);
				glPointSize(1.0f);
			}
		}

		if (drawShipMarker && showCapitalShipOverlay && bucketIndex >= 0 && ship->collInfo.selCircleDepth > 0.0f)
		{
			toicon* icon;
			color overlayColor;
			real32 overlayScale;
			sdword pointIndex;

			icon = toClassIcon[bucketIndex];
			if (icon != NULL && icon->nPoints > 0)
			{
				overlayScale = ship->staticinfo->tacticalSelectionScale * ship->collInfo.selCircleRadius;
				if (overlayScale < cat_smCapitalOverlayMinScale())
				{
					overlayScale = cat_smCapitalOverlayMinScale();
				}

				if (!primModeEnabled)
				{
					primModeSetFunction2();
				}
				overlayColor = cat_smShipPointColor(ship);
				primLineLoopStart2(1, overlayColor);
				for (pointIndex = icon->nPoints - 1; pointIndex >= 0; pointIndex--)
				{
					primLineLoopPoint3F(
						ship->collInfo.selCircleX + overlayScale * icon->loc[pointIndex].x,
						ship->collInfo.selCircleY + overlayScale * icon->loc[pointIndex].y);
				}
				primLineLoopEnd2();
				if (primModeEnabled)
				{
					primModeClearFunction2();
				}
				rndLightingEnable(FALSE);
				rndTextureEnable(FALSE);
			}
		}
	}
}

sdword __cdecl cat_smGetShipRenderBucketIndex(Ship* ship)
{
	sdword shipType;

	if (ship->staticinfo->shipclass == CLASS_Mothership)
	{
		return 0;
	}

	shipType = ship->shiptype;
	switch (shipType)
	{
		case sCarrier:
		case bCarrier:
		case kCarrier:
		case tCarrier:
			return 2;

		case sDreadnought:
		case bHeavyCruiser:
		case kHeavyCruiser:
		case tHeavyCruiser:
			return 1;

		case trFloatingCity:
		case trMiningBase:
		case trResearchStation:
			return 8;

		case sProcessor:
		case bProcessor:
			return 7;

		case sDestroyer:
		case dRKO:
			return 3;
	}

	return -1;
}
sdword __cdecl fogObjectVisibleToPlayer(Player* player, SpaceObj* obj, sdword treatMultiplayerAsVisible)
{
	smFogGridRuntimeView* fogGrid;
	real32 inverseCellSize;
	sdword gridX;
	sdword gridY;
	sdword gridZ;
	sdword occupancy;
	sdword visible;

	visible = FALSE;
	if (!singlePlayerGame && treatMultiplayerAsVisible)
	{
		return TRUE;
	}

	fogGrid = (smFogGridRuntimeView*)player->fogGrid;
	if (fogGrid != NULL)
	{
		currentSensorCoverage = (sdword)fogGrid;
		smFogRenderListsDirty = 0;
		inverseCellSize = 1.0f / fogGrid->cellSize;
		gridX = (sdword)((obj->posinfo.position.x - fogGrid->originX) * inverseCellSize);
		gridY = (sdword)((obj->posinfo.position.y - fogGrid->originY) * inverseCellSize);
		gridZ = (sdword)((obj->posinfo.position.z - fogGrid->originZ) * inverseCellSize);
		occupancy = fogGetGridCellOccupancy(gridX, gridY, gridZ);
		if (occupancy > 0 &&
			(!univIsShipCloaked((SpaceObj*)obj) || shipGetSlotStatus((Ship*)obj, player->playerIndex)))
		{
			visible = TRUE;
		}
		if (occupancy == -524288)
		{
			visible = TRUE;
		}
		if (obj != NULL && obj->objtype == OBJ_ShipType)
		{
			if ((obj->flags & SOF_Hide) != 0)
			{
				visible = FALSE;
			}
			if (((Ship*)obj)->shiplink.structptr == NULL)
			{
				return FALSE;
			}
		}
	}
	else if (player->aiPlayer == NULL ||
		!univIsShipCloaked((SpaceObj*)obj) ||
		shipGetSlotStatus((Ship*)obj, player->playerIndex))
	{
		return TRUE;
	}

	return visible;
}
real32* __cdecl fogCreateGrid(real32* minimums, real32* maximums)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	real32 minX, minY, minZ;
	real32 maxX, maxY, maxZ;
	real32 halfCell;
	sdword x, y, z;

	smFogDirtyUpdateCount = 0;
	smFogRenderListsDirty = 0;
	if (minimums == NULL || maximums == NULL)
	{
		smFogDirtyUpdateCount++;
		smFogRenderListsDirty = 1;
		return NULL;
	}

	fogGrid = memAlloc(sizeof(smFogGridRuntimeView), "fogGrid", 0);
	memset(fogGrid, 0, sizeof(smFogGridRuntimeView));

	minX = minimums[0] <= maximums[0] ? minimums[0] : maximums[0];
	minY = minimums[1] <= maximums[1] ? minimums[1] : maximums[1];
	minZ = minimums[2] <= maximums[2] ? minimums[2] : maximums[2];
	maxX = minimums[0] >= maximums[0] ? minimums[0] : maximums[0];
	maxY = minimums[1] >= maximums[1] ? minimums[1] : maximums[1];
	maxZ = minimums[2] >= maximums[2] ? minimums[2] : maximums[2];

	fogGrid->originX = minX;
	fogGrid->originY = minY;
	fogGrid->originZ = minZ;
	fogGrid->cellSize = 10000.0f;
	fogGrid->width = (sdword)((maxX - minX) / fogGrid->cellSize);
	fogGrid->height = (sdword)((maxY - minY) / fogGrid->cellSize);
	fogGrid->depth = (sdword)((maxZ - minZ) / fogGrid->cellSize);
	fogGrid->data = memAlloc(
		sizeof(smFogGridCell) * fogGrid->width * fogGrid->height * fogGrid->depth,
		"fogGridData",
		0);
	memset(
		fogGrid->data,
		0,
		sizeof(smFogGridCell) * fogGrid->width * fogGrid->height * fogGrid->depth);

	currentSensorCoverage = (sdword)fogGrid;
	fogResetGrid();

	halfCell = fogGrid->cellSize * 0.5f;
	cells = (smFogGridCell*)fogGrid->data;
	for (x = 0; x < fogGrid->width; ++x)
	{
		for (y = 0; y < fogGrid->height; ++y)
		{
			for (z = 0; z < fogGrid->depth; ++z)
			{
				smFogGridCell* cell = &cells[x + fogGrid->width * (y + z * fogGrid->height)];
				cell->center.x = fogGrid->originX + (real32)x * fogGrid->cellSize + halfCell;
				cell->center.y = fogGrid->originY + (real32)y * fogGrid->cellSize + halfCell;
				cell->center.z = fogGrid->originZ + (real32)z * fogGrid->cellSize + halfCell;
			}
		}
	}

	return (real32*)fogGrid;
}
udword __cdecl fogDestroyGrid(ubyte* grid)
{
	currentSensorCoverage = 0;
	if (grid != NULL)
	{
		if (*(ubyte**)grid != NULL)
		{
			memFree(*(ubyte**)grid);
		}
		memFree(grid);
	}

	return fogFreeRenderState();
}
void fogResetGrid(void)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	sdword cellCount;
	sdword index;

	fogGrid = smCurrentFogGrid();
	smFogRenderListsDirty = 0;
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return;
	}

	cells = (smFogGridCell*)fogGrid->data;
	cellCount = fogGrid->width * fogGrid->height * fogGrid->depth;
	for (index = 0; index < cellCount; ++index)
	{
		cells[index].occupancy = 0;
		cells[index].visibility = 0;
	}

	fogFreeRenderState();
	fogAllocRenderState(FALSE);
}
void __cdecl fogRemoveSphereFromGrid(real32 x, real32 y, real32 z, real32 radius)
{
	smFogGridRuntimeView* fogGrid;
	real32 inverseCellSize;
	sdword gridX;
	sdword gridY;
	sdword gridZ;
	sdword gridRadius;
	sdword minX;
	sdword maxX;
	sdword minY;
	sdword maxY;
	sdword minZ;
	sdword maxZ;
	sdword curX;
	sdword curY;
	sdword curZ;
	smFogGridCell* cell;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return;
	}

	smFogRenderListsDirty = 0;
	inverseCellSize = 1.0f / fogGrid->cellSize;
	gridX = (sdword)((x - fogGrid->originX) * inverseCellSize);
	gridY = (sdword)((y - fogGrid->originY) * inverseCellSize);
	gridZ = (sdword)((z - fogGrid->originZ) * inverseCellSize);
	gridRadius = (sdword)(radius * inverseCellSize) + 1;

	minX = max(0, gridX - gridRadius);
	maxX = min(fogGrid->width - 1, gridX + gridRadius);
	minY = max(0, gridY - gridRadius);
	maxY = min(fogGrid->height - 1, gridY + gridRadius);
	minZ = max(0, gridZ - gridRadius);
	maxZ = min(fogGrid->depth - 1, gridZ + gridRadius);

	for (curZ = minZ; curZ <= maxZ; ++curZ)
	{
		for (curY = minY; curY <= maxY; ++curY)
		{
			for (curX = minX; curX <= maxX; ++curX)
			{
				cell = &((smFogGridCell*)fogGrid->data)[curX + fogGrid->width * (curY + fogGrid->height * curZ)];
				if (cell->occupancy > 0)
				{
					cell->occupancy--;
					smFogMarkDirtyGridCells(curX, curY, curZ, 1);
				}
			}
		}
	}
}

// Retail pingUpdateShipSensorGrid gates repeated enemy-spotted feed messages with a 10-second cooldown.
#define SM_EnemySpottedNotificationInterval 10.0f

sdword __cdecl pingUpdateShipSensorGrid(Ship* ship, vector* oldPos, vector* newPos)
{
	sdword sensorCoverage;
	sdword savedSensorCoverage;
	real32 sensorRadius;
	sdword oldGridX;
	sdword oldGridY;
	sdword oldGridZ;
	sdword newGridX;
	sdword newGridY;
	sdword newGridZ;
	sdword gridRadius;
	Node* node;

	sensorCoverage = smPlayerSensorCoverage(ship->playerowner);
	if (sensorCoverage == 0)
	{
		return 0;
	}

	savedSensorCoverage = univSetCurrentSensorCoverage(sensorCoverage);
	sensorRadius = univGetMaxHealth(ship, FALSE);
	smFogComputeGridIndices(oldPos->x, oldPos->y, oldPos->z, sensorRadius,
		&oldGridX, &oldGridY, &oldGridZ, &gridRadius);
	smFogComputeGridIndices(newPos->x, newPos->y, newPos->z, 0.0f,
		&newGridX, &newGridY, &newGridZ, NULL);

	if (oldGridX != newGridX || oldGridY != newGridY || oldGridZ != newGridZ)
	{
		fogRemoveSphereFromGrid(oldPos->x, oldPos->y, oldPos->z, sensorRadius);
		if (ship->playerowner->fogGrid != NULL)
		{
			if (smFogHasUnseenGridCells(newGridX, newGridY, newGridZ, gridRadius) &&
				smEnemySpottedNotificationTime + SM_EnemySpottedNotificationInterval < universeRealtimeElapsed)
			{
				for (node = universe.ShipList.head; node != NULL; node = node->next)
				{
					Ship* enemyShip;
					sdword enemyGridX;
					sdword enemyGridY;
					sdword enemyGridZ;

					enemyShip = (Ship*)node->structptr;
					if (univIsObjFriendly((SpaceObj*)enemyShip, universe.curPlayerPtr) ||
						smIsCrystal(enemyShip))
					{
						continue;
					}

					smFogComputeGridIndices(enemyShip->posinfo.position.x,
						enemyShip->posinfo.position.y,
						enemyShip->posinfo.position.z,
						0.0f,
						&enemyGridX, &enemyGridY, &enemyGridZ, NULL);
					if (enemyGridX > newGridX + gridRadius || enemyGridX < newGridX - gridRadius ||
						enemyGridY > newGridY + gridRadius || enemyGridY < newGridY - gridRadius ||
						enemyGridZ > newGridZ + gridRadius || enemyGridZ < newGridZ - gridRadius)
					{
						continue;
					}
					if (smFogGetGridCellVisibility(enemyGridX, enemyGridY, enemyGridZ) != 0)
					{
						break;
					}

					smEnemySpottedNotificationTime = universeRealtimeElapsed;
					if (shipIsBeastShip(enemyShip))
					{
						battleChatAddMessage(strGetString(strSlipgateSpottedNotification), -65536, 1, (sdword)ship);
					}
					else if (!singlePlayerGame || singlePlayerGameInfo.currentMission != 7 || enemyShip->shiptype != dPrisonShip)
					{
						if (shipGetSlotStatus(enemyShip, universe.curPlayerIndex))
						{
							battleChatAddMessage(strGetString(strEnemySpottedNotification), -16776961, 1, (sdword)ship);
						}
					}
					break;
				}
			}
			smFogFillGridCells(newGridX, newGridY, newGridZ, gridRadius);
		}
	}

	return univSetCurrentSensorCoverage(savedSensorCoverage);
}

sdword __cdecl fogGetGridCellOccupancy(sdword x, sdword y, sdword z)
{
	smFogGridRuntimeView* fogGrid;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		++smFogDirtyUpdateCount;
		smFogRenderListsDirty = 1;
		return -524288;
	}
	if (x < 0 || x >= fogGrid->width ||
		y < 0 || y >= fogGrid->height ||
		z < 0 || z >= fogGrid->depth)
	{
		return -524288;
	}

	smFogRenderListsDirty = 0;
	return *(sdword*)(fogGrid->data + 32 * (x + fogGrid->width * (y + z * fogGrid->height)));
}
sdword fogDrawGrid(void)
{
	GLboolean depthEnabled;
	sdword result;
	depthEnabled = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	fogRebuildDirtyRenderLists();
	result = fogDrawRenderLists();
	if (depthEnabled)
	{
		glEnable(GL_DEPTH_TEST);
	}
	return result;
}
sdword __cdecl fogAllocRenderState(sdword initializeDirtyFlags)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	sdword cellCount;
	sdword index;
	sdword x;
	sdword y;
	sdword z;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		return 0;
	}

	cellCount = fogGrid->width * fogGrid->height * fogGrid->depth;
	smFogOccupiedRenderState = (sdword)memAlloc(68 * cellCount, "fogRenderState0", 0);
	memset((void*)smFogOccupiedRenderState, 0, 68 * cellCount);
	smFogVisibleRenderState = (sdword)memAlloc(68 * cellCount, "fogRenderState1", 0);
	memset((void*)smFogVisibleRenderState, 0, 68 * cellCount);
	smFogUnknownRenderState = (sdword)memAlloc(68 * cellCount, "fogRenderState2", 0);
	memset((void*)smFogUnknownRenderState, 0, 68 * cellCount);

	listInit(&smFogOccupiedRenderList);
	listInit(&smFogVisibleRenderList);
	listInit(&smFogUnknownRenderList);
	smFogOccupiedDirtyCount = 0;
	smFogVisibleDirtyCount = 0;
	smFogUnknownDirtyCount = 0;

	smFogOccupiedDirtyList = (sdword)memAlloc(12 * cellCount, "fogDirtyList0", 0);
	memset((void*)smFogOccupiedDirtyList, 0, 12 * cellCount);
	smFogVisibleDirtyList = (sdword)memAlloc(12 * cellCount, "fogDirtyList1", 0);
	memset((void*)smFogVisibleDirtyList, 0, 12 * cellCount);
	smFogUnknownDirtyList = (sdword)memAlloc(12 * cellCount, "fogDirtyList2", 0);
	memset((void*)smFogUnknownDirtyList, 0, 12 * cellCount);

	if (initializeDirtyFlags)
	{
		cells = (smFogGridCell*)fogGrid->data;
		for (index = 0; index < cellCount; ++index)
		{
			cells[index].renderFlags[0] = 0;
			cells[index].renderFlags[1] = 0;
			cells[index].renderFlags[2] = 0;
		}
		for (x = 0; x < fogGrid->width; ++x)
		{
			for (y = 0; y < fogGrid->height; ++y)
			{
				for (z = 0; z < fogGrid->depth; ++z)
				{
					smFogMarkDirtyGridCells(x, y, z, 0);
				}
			}
		}
	}

	return fogGrid->width;
}
udword fogFreeRenderState(void)
{
	memFree((ubyte*)smFogOccupiedRenderState);
	smFogOccupiedRenderState = 0;
	memFree((ubyte*)smFogVisibleRenderState);
	smFogVisibleRenderState = 0;
	memFree((ubyte*)smFogUnknownRenderState);
	smFogUnknownRenderState = 0;
	memFree((ubyte*)smFogOccupiedDirtyList);
	smFogOccupiedDirtyList = 0;
	memFree((ubyte*)smFogVisibleDirtyList);
	smFogVisibleDirtyList = 0;
	memFree((ubyte*)smFogUnknownDirtyList);
	smFogUnknownDirtyList = 0;
	return 0;
}
sdword fogRebuildDirtyRenderLists(void)
{
	if (showAreasOccupied)
	{
		fogRebuildDirtyRenderList(smFogOccupiedDirtyList, &smFogOccupiedDirtyCount, smFogOccupiedRenderState, 2, &smFogOccupiedRenderList);
		if (showAreasVisited)
		{
			fogRebuildDirtyRenderList(smFogUnknownDirtyList, &smFogUnknownDirtyCount, smFogUnknownRenderState, 0, &smFogUnknownRenderList);
		}
	}
	else if (showAreasVisited)
	{
		fogRebuildDirtyRenderList(smFogVisibleDirtyList, &smFogVisibleDirtyCount, smFogVisibleRenderState, 1, &smFogVisibleRenderList);
	}

	return showAreasVisited;
}
void __cdecl fogRebuildDirtyRenderList(sdword dirtyListBase, sdword* dirtyCount, sdword renderStateBase, sdword renderMode, LinkedList* list)
{
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	smFogDirtyCellCoord* dirtyCells;
	smFogRenderCellState* renderStates;
	sdword dirtyIndex;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL || *dirtyCount <= 0)
	{
		*dirtyCount = 0;
		return;
	}

	cells = smFogGridCells(fogGrid);
	dirtyCells = smFogDirtyCells(dirtyListBase);
	renderStates = smFogRenderStates(renderStateBase);
	for (dirtyIndex = 0; dirtyIndex < *dirtyCount; ++dirtyIndex)
	{
		smFogDirtyCellCoord* dirtyCell;
		sdword cellIndex;
		smFogGridCell* cell;
		smFogRenderCellState* renderState;
		bool32 shouldRender;
		sdword edgeIndex;
		bool32 hasVisibleEdges;
		sdword* dirtyFlag;

		dirtyCell = &dirtyCells[dirtyIndex];
		cellIndex = smFogGridIndexIfValid(fogGrid, dirtyCell->x, dirtyCell->y, dirtyCell->z);
		if (cellIndex < 0)
		{
			continue;
		}

		cell = &cells[cellIndex];
		renderState = &renderStates[cellIndex];
		shouldRender = smFogCellMatchesRenderMode(cell, renderMode);
		if (!shouldRender)
		{
			memset(renderState->edgeVisible, 0, sizeof(renderState->edgeVisible));
			if (renderState->link.belongto != NULL)
			{
				listRemoveNode(&renderState->link);
			}
		}
		else
		{
			for (edgeIndex = 0; edgeIndex < 12; ++edgeIndex)
			{
				sdword neighborIndex[3];
				bool32 neighborMatches[3];
				bool32 neighborOccupied[3];
				bool32 edgeVisible;
				sdword i;

				for (i = 0; i < 3; ++i)
				{
					neighborIndex[i] = smFogGridIndexIfValid(
						fogGrid,
						dirtyCell->x + smFogEdgeNeighborOffsets[edgeIndex][i][0],
						dirtyCell->y + smFogEdgeNeighborOffsets[edgeIndex][i][1],
						dirtyCell->z + smFogEdgeNeighborOffsets[edgeIndex][i][2]);
					neighborMatches[i] = FALSE;
					neighborOccupied[i] = FALSE;
					if (neighborIndex[i] >= 0)
					{
						neighborMatches[i] = smFogCellMatchesRenderMode(&cells[neighborIndex[i]], renderMode);
						neighborOccupied[i] = cells[neighborIndex[i]].occupancy > 0;
					}
				}

				edgeVisible = TRUE;
				if (neighborMatches[0] || neighborMatches[1])
				{
					if (neighborMatches[2] || neighborMatches[1])
					{
						if (neighborMatches[0] && neighborMatches[1] && neighborMatches[2])
						{
							edgeVisible = FALSE;
						}
					}
					else if (neighborMatches[0])
					{
						edgeVisible = FALSE;
					}
				}
				else if (neighborMatches[2])
				{
					edgeVisible = FALSE;
				}
				if (renderMode == 0 &&
					(neighborOccupied[0] || neighborOccupied[1] || neighborOccupied[2]))
				{
					edgeVisible = FALSE;
				}

				renderState->edgeVisible[edgeIndex] = edgeVisible;
			}

			hasVisibleEdges = FALSE;
			for (edgeIndex = 0; edgeIndex < 12; ++edgeIndex)
			{
				if (renderState->edgeVisible[edgeIndex] != 0)
				{
					hasVisibleEdges = TRUE;
					break;
				}
			}

			if (hasVisibleEdges)
			{
				if (renderState->link.belongto == NULL)
				{
					listAddNode(list, &renderState->link, renderState);
				}
				renderState->cellIndex = cellIndex;
			}
			else if (renderState->link.belongto != NULL)
			{
				listRemoveNode(&renderState->link);
			}
		}

		dirtyFlag = smFogCellDirtyFlag(cell, renderMode);
		if (dirtyFlag != NULL)
		{
			*dirtyFlag = 0;
		}
	}

	*dirtyCount = 0;
}
real32* __cdecl fogGetCellCornerPosition(sdword cornerIndex, real32* center, real32* corner)
{
	real32 halfCell;

	halfCell = *(real32*)(currentSensorCoverage + 28) * 0.5f;
	corner[0] = center[0];
	corner[1] = center[1];
	corner[2] = center[2];

	switch (cornerIndex)
	{
		case 0:
			corner[0] -= halfCell;
			corner[1] -= halfCell;
			corner[2] += halfCell;
			break;
		case 1:
			corner[0] += halfCell;
			corner[1] -= halfCell;
			corner[2] += halfCell;
			break;
		case 2:
			corner[0] += halfCell;
			corner[1] -= halfCell;
			corner[2] -= halfCell;
			break;
		case 3:
			corner[0] -= halfCell;
			corner[1] -= halfCell;
			corner[2] -= halfCell;
			break;
		case 4:
			corner[0] -= halfCell;
			corner[1] += halfCell;
			corner[2] += halfCell;
			break;
		case 5:
			corner[0] += halfCell;
			corner[1] += halfCell;
			corner[2] += halfCell;
			break;
		case 6:
			corner[0] += halfCell;
			corner[1] += halfCell;
			corner[2] -= halfCell;
			break;
		case 7:
			corner[0] -= halfCell;
			corner[1] += halfCell;
			corner[2] -= halfCell;
			break;
		default:
			break;
	}

	return corner;
}
sdword fogDrawRenderLists(void)
{
	if (smFogRenderPaletteDirty)
	{
		smFogBuildRenderPalette(smFogVisibleColors, smFogVisibleColorNear, smFogVisibleColorFar);
		smFogBuildRenderPalette(smFogOccupiedColors, smFogOccupiedColorNear, smFogOccupiedColorFar);
		smFogBuildRenderPalette(smFogUnknownColors, smFogUnknownColorNear, smFogUnknownColorFar);
		smFogRenderPaletteDirty = FALSE;
	}
	if (showAreasOccupied)
	{
		fogDrawRenderList(&smFogOccupiedRenderList, smFogPaletteForMode(2));
		if (showAreasVisited)
		{
			fogDrawRenderList(&smFogUnknownRenderList, smFogPaletteForMode(0));
		}
	}
	else if (showAreasVisited)
	{
		fogDrawRenderList(&smFogVisibleRenderList, smFogPaletteForMode(1));
	}
	return showAreasVisited;
}

sdword __cdecl fogDrawRenderList(LinkedList* list, color* palette)
{
	Node* node;
	smFogGridRuntimeView* fogGrid;
	smFogGridCell* cells;
	real32 minimumDistance;
	real32 maximumDistance;
	real32 paletteScale;
	sdword lineIndex;

	fogGrid = smCurrentFogGrid();
	if (fogGrid == NULL)
	{
		return 0;
	}

	cells = smFogGridCells(fogGrid);
	smFogRenderLines.count = 0;
	for (node = list->head; node != NULL; node = node->next)
	{
		smFogRenderCellState* renderState;
		smFogGridCell* cell;
		sdword edgeIndex;

		renderState = (smFogRenderCellState*)((ubyte*)node - offsetof(smFogRenderCellState, link));
		cell = &cells[renderState->cellIndex];
		for (edgeIndex = 0; edgeIndex < 12; ++edgeIndex)
		{
			if (renderState->edgeVisible[edgeIndex] != 0)
			{
				vector startCorner;
				vector endCorner;
				vector startPoint;
				vector endPoint;
				vector edgeDirection;
				sdword startCornerIndex;
				sdword endCornerIndex;
				sdword i;

				startCornerIndex = smFogEdgeCornerPairs[edgeIndex][0];
				endCornerIndex = smFogEdgeCornerPairs[edgeIndex][1];
				fogGetCellCornerPosition(startCornerIndex, &cell->center.x, &startCorner.x);
				fogGetCellCornerPosition(endCornerIndex, &cell->center.x, &endCorner.x);
				startPoint = startCorner;
				endPoint = endCorner;
				for (i = 0; i < 3; ++i)
				{
					if (renderState->edgeVisible[smFogCornerEdges[startCornerIndex][i]] == 0)
					{
						goto start_done;
					}
				}
				vecSub(edgeDirection, endCorner, startCorner);
				vecNormalize(&edgeDirection);
				edgeDirection.x *= smFogCornerTickScale * fogGrid->cellSize;
				edgeDirection.y *= smFogCornerTickScale * fogGrid->cellSize;
				edgeDirection.z *= smFogCornerTickScale * fogGrid->cellSize;
				vecAdd(startPoint, startPoint, edgeDirection);
				if (edgeIndex == smFogCornerEdges[startCornerIndex][0])
				{
					fogAddCellCornerLines(&startCorner.x, startCornerIndex);
				}
			start_done:
				for (i = 0; i < 3; ++i)
				{
					if (renderState->edgeVisible[smFogCornerEdges[endCornerIndex][i]] == 0)
					{
						goto end_done;
					}
				}
				vecSub(edgeDirection, startCorner, endCorner);
				vecNormalize(&edgeDirection);
				edgeDirection.x *= smFogCornerTickScale * fogGrid->cellSize;
				edgeDirection.y *= smFogCornerTickScale * fogGrid->cellSize;
				edgeDirection.z *= smFogCornerTickScale * fogGrid->cellSize;
				vecAdd(endPoint, endPoint, edgeDirection);
				if (edgeIndex == smFogCornerEdges[endCornerIndex][0])
				{
					fogAddCellCornerLines(&endCorner.x, endCornerIndex);
				}
			end_done:
				fogAddRenderLine(&startPoint.x, &endPoint.x);
			}
		}
	}

	minimumDistance = 1.0e21f;
	maximumDistance = 0.0f;
	for (lineIndex = 0; lineIndex < smFogRenderLines.count; ++lineIndex)
	{
		smFogRenderLine* line;

		line = &smFogRenderLines.lines[lineIndex];
		if (line->startDistance < minimumDistance)
		{
			minimumDistance = line->startDistance;
		}
		else if (line->startDistance > maximumDistance)
		{
			maximumDistance = line->startDistance;
		}
		if (line->endDistance < minimumDistance)
		{
			minimumDistance = line->endDistance;
		}
		else if (line->endDistance > maximumDistance)
		{
			maximumDistance = line->endDistance;
		}
	}
	if (smFogRenderLines.count == 0)
	{
		return 0;
	}

	paletteScale = (maximumDistance > minimumDistance) ?
		(smFogPaletteIndexScale / (maximumDistance - minimumDistance)) :
		0.0f;
	glBegin(GL_LINES);
	for (lineIndex = 0; lineIndex < smFogRenderLines.count; ++lineIndex)
	{
		smFogRenderLine* line;
		sdword startPaletteIndex;
		sdword endPaletteIndex;
		color startColor;
		color endColor;

		line = &smFogRenderLines.lines[lineIndex];
		startPaletteIndex = (sdword)((line->startDistance - minimumDistance) * paletteScale);
		if (startPaletteIndex < 0)
		{
			startPaletteIndex = 0;
		}
		else if (startPaletteIndex > 15)
		{
			startPaletteIndex = 15;
		}
		endPaletteIndex = (sdword)((line->endDistance - minimumDistance) * paletteScale);
		if (endPaletteIndex < 0)
		{
			endPaletteIndex = 0;
		}
		else if (endPaletteIndex > 15)
		{
			endPaletteIndex = 15;
		}

		startColor = palette[startPaletteIndex];
		endColor = palette[endPaletteIndex];
		glColor3ub(colRed(startColor), colGreen(startColor), colBlue(startColor));
		glVertex3f(line->start.x, line->start.y, line->start.z);
		glColor3ub(colRed(endColor), colGreen(endColor), colBlue(endColor));
		glVertex3f(line->end.x, line->end.y, line->end.z);
		if ((lineIndex & 0x3F) == 0)
		{
			glEnd();
			glBegin(GL_LINES);
		}
	}
	glEnd();
	smFogRenderLines.count = 0;
	return 0;
}
sdword __cdecl fogAddRenderLine(real32* from, real32* to)
{
	smFogRenderLine* line;
	vector offset;

	if (smFogRenderLines.count >= smFogRenderLines.capacity)
	{
		smFogRenderLines.capacity += 128;
		smFogRenderLines.lines = memRealloc(
			smFogRenderLines.lines,
			sizeof(smFogRenderLine) * smFogRenderLines.capacity,
			"smFogRenderLines",
			0);
	}

	line = &smFogRenderLines.lines[smFogRenderLines.count];
	line->start.x = from[0];
	line->start.y = from[1];
	line->start.z = from[2];
	line->end.x = to[0];
	line->end.y = to[1];
	line->end.z = to[2];

	vecSub(offset, smCamera.eyeposition, line->start);
	line->startDistance = getVectDistSloppy(offset);
	vecSub(offset, smCamera.eyeposition, line->end);
	line->endDistance = getVectDistSloppy(offset);

	smFogRenderLines.count++;
	return smFogRenderLines.count;
}

sdword __cdecl fogAddCellCornerLines(real32* corner, sdword cornerIndex)
{
	real32 lineLength;
	vector start;
	vector first;
	vector second;
	vector third;

	lineLength = 0.2f * *(real32*)(currentSensorCoverage + 28);
	start.x = corner[0];
	start.y = corner[1];
	start.z = corner[2];
	first = start;
	second = start;
	third = start;

	switch (cornerIndex)
	{
		case 0:
			first.x += lineLength;
			second.y += lineLength;
			third.z -= lineLength;
			break;
		case 1:
			first.x -= lineLength;
			second.y += lineLength;
			third.z -= lineLength;
			break;
		case 2:
			first.x -= lineLength;
			second.y += lineLength;
			third.z += lineLength;
			break;
		case 3:
			first.x += lineLength;
			second.y += lineLength;
			third.z += lineLength;
			break;
		case 4:
			first.x += lineLength;
			second.y -= lineLength;
			third.z -= lineLength;
			break;
		case 5:
			first.x -= lineLength;
			second.y -= lineLength;
			third.z -= lineLength;
			break;
		case 6:
			first.x -= lineLength;
			second.y -= lineLength;
			third.z += lineLength;
			break;
		case 7:
			first.x += lineLength;
			second.y -= lineLength;
			third.z += lineLength;
			break;
		default:
			return smFogRenderLines.count;
	}

	fogAddRenderLine(&start.x, &first.x);
	fogAddRenderLine(&start.x, &second.x);
	return fogAddRenderLine(&start.x, &third.x);
}

regionhandle smViewportRegisterAllKeys(regionhandle parent, regionfunction function)
{
	sdword index;
	regionhandle result;

	result = NULL;
	for (index = 0; index < 256; ++index)
	{
		result = regKeyChildAlloc(parent, index, RPE_KeyUp | RPE_KeyDown, function, 1, index);
	}

	return result;
}
