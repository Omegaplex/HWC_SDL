/*=============================================================================
    Name    : KeyBindings.c
    Purpose : This file handles all of the logic for the key bindings

    Created 8/04/1999 by Drew Dunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "KeyBindings.h"
#include "uicontrols.h"
#include "key.h"
#include "fontreg.h"
#include "FEColour.h"
#include "strings.h"
#include "statscript.h"
#include "AIUtilities.h"
#include "CameraCommand.h"
#include "CommandWrap.h"
#include "ConsMgr.h"
#include "InfoOverlay.h"

#include "Sensors.h"
#include "Shader.h"
#include "SoundEvent.h"
#include "Subtitle.h"
#include "Tutor.h"
#include "formation.h"
#include "main.h"
#include "mainrgn.h"
#include "pieplate.h"
#include "render.h"
#include "select.h"
#include "shipselect.h"
#include "task.h"
#include "universe.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern bool32 mrDisabled;
extern bool32 mrMenuDontDisappear;
extern bool32 mrDrawTactical;
extern sdword smPathMarkersEnabled;
extern TypeOfFormation mrNewFormation;
extern char *mrFormationName;
extern real32 mrFormationTime;
extern bool32 mrDrawFormation;
extern real32 mrDrawFormationTime;
extern sdword kbSuppressSpecialReleaseOverlay;
extern sdword kbSuppressNextSpecialActivate;
extern sdword smBandboxEnabled;
extern sdword dword_A5B1B0;

#define KB_RGL_GAMMA_UP 0x6000
#define KB_RGL_GAMMA_DN 0x6001
#define KB_PARADE_FORMATION_FLAG ((udword)-1)

sdword rglFeature(udword feature);

/*=============================================================================
    Defines :
=============================================================================*/


/*=============================================================================
    Data definitions :
=============================================================================*/

udword kbKeySavedKeys[20] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

kbBoundKeys *kbKeyTable = NULL;

kbBoundKeys kbKeyTableEnglish[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , DKEY          , DKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , MKEY          , MKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableFrench[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , XKEY          , XKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , AKEY          , AKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , CKEY          , CKEY          },
    { kbMOVE                , MKEY          , DKEY          , DKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableGerman[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , DKEY          , DKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , WKEY          , WKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableSpanish[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , AKEY          , AKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , DKEY          , DKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};

kbBoundKeys kbKeyTableItalian[kbTOTAL_COMMANDS] =
{  // command                 defaultkey      primary key     key to reset to
    { kbNEXT_FORMATION      , TABKEY        , TABKEY        , TABKEY        },
    { kbBUILD_MANAGER       , BKEY          , BKEY          , BKEY          },
    { kbPREVIOUS_FOCUS      , CKEY          , CKEY          , CKEY          },
    { kbNEXT_FOCUS          , VKEY          , VKEY          , VKEY          },
    { kbDOCK                , DKEY          , AKEY          , AKEY          },
    { kbSELECT_ALL_VISIBLE  , EKEY          , EKEY          , EKEY          },
    { kbFOCUS               , FKEY          , FKEY          , FKEY          },
    { kbRESEARCH_MANAGER    , RKEY          , RKEY          , RKEY          },
    { kbHARVEST             , HKEY          , HKEY          , HKEY          },
    { kbMOVE                , MKEY          , MKEY          , MKEY          },
    { kbNEXT_TACTIC         , RBRACK        , RBRACK        , RBRACK        },
    { kbPREVIOUS_TACTIC     , LBRACK        , LBRACK        , LBRACK        },
    { kbSCUTTLE             , SKEY          , SKEY          , SKEY          },
    { kbSHIP_SPECIAL        , ZKEY          , ZKEY          , ZKEY          },
    { kbTACTICAL_OVERLAY    , CAPSLOCKKEY   , CAPSLOCKKEY   , CAPSLOCKKEY   },
//    { kbSENSORS_MANAGER     , SPACEKEY      , SPACEKEY      , SPACEKEY      },
    { kbMOTHERSHIP          , HOMEKEY       , HOMEKEY       , HOMEKEY       },
    { kbKAMIKAZE            , KKEY          , KKEY          , KKEY          },
    { kbCANCEL_ORDERS       , TILDEKEY      , TILDEKEY      , TILDEKEY      },
    { kbLAUNCH_MANAGER      , LKEY          , LKEY          , LKEY          }
};


kbBoundKeys kbKeySaveTable[kbTOTAL_COMMANDS];

// This is a look-up table that has a value of TRUE for
bool8 kbCanMapKey[KEY_TOTAL_KEYS];

// This is a look-up table for the strings
sdword kbKeyToString[KEY_TOTAL_KEYS];

// fonthandle for the keyboard list window font initialized in the options init
fonthandle kbKeyListFont;

// pointer to the listwindow handle
listwindowhandle kbListWindow=NULL;

bool32 bkDisableKeyRemap=FALSE;

typedef struct kbBinaryCommandDescriptor
{
    const char *configName;
    const char *displayName;
    sdword category;
    kbCommandCallback press;
    kbCommandCallback release;
} kbBinaryCommandDescriptor;

typedef struct kbDirectCommandTranslation
{
    const char *configName;
    sdword translatedKey;
} kbDirectCommandTranslation;

static kbCommandBindingList kbCommandDefaults[KB_MAX_COMMAND_DEFS];
kbCommandBindingList kbCommandBindingTable[KB_MAX_COMMAND_DEFS];
static kbCommandBindingList kbCommandSaved[KB_MAX_COMMAND_DEFS];
static sdword kbCommandCount = KB_BINARY_COMMAND_COUNT;
static sdword kbConfigVersion = 0;
static sdword kbCommandWaitingForKey = -1;
static bool32 kbCataclysmBindingsLoaded = FALSE;
static char kbKeyStringBuffer[32];

static const char *kbLegacyCommandNames[kbTOTAL_COMMANDS] =
{
    "Next_Formation",
    "Open_BuildManager",
    "Cancel_Focus",
    "Forward_Focus",
    "Dock",
    "Select_All_Visible_Ships",
    "Focus",
    "Open_ResearchManager",
    "Harvest",
    "Move",
    "Next_Tactic_Level",
    "Prev_Tactic_Level",
    "Scuttle_Ships",
    "Ship_Special",
    "Toggle_Tactical_Display",
    "Focus_On_Mothership",
    "Kamikaze",
    "Cancel_Orders",
    "Open_LaunchManager"
};

#define kbCommandCurrent kbCommandBindingTable

/*=============================================================================
    Private Function Prototypes :
=============================================================================*/

void kbSetNewKey(udword keypressed);
bool32 kbKeyUsed(udword keytocheck);
bool32 parseLine(char *line,char **returnName,char **returnValue);
void kbLoadSettings(sdword reset);
FILE *kbLoadSettingsNoReset(void);
static void kbCommandAccelerateTime(sdword keycode);
static void kbCommandDecelerateTime(sdword keycode);
static void kbCommandToggleGameSpeed(sdword keycode);
static void kbCommandNextMusicTrack(sdword keycode);
static void kbCommandPrevMusicTrack(sdword keycode);
static void kbCommandGammaUp(sdword keycode);
static void kbCommandGammaDown(sdword keycode);
static void kbCommandPause(sdword keycode);
static void kbCommandEscape(sdword keycode);
static void kbCommandBuildNarratorUpdate(sdword keycode);
static void kbCommandTakeScreenshot(sdword keycode);
static void kbCommandMove(sdword keycode);
static void kbCommandDock(sdword keycode);
static void kbCommandRetire(sdword keycode);
static void kbCommandCancelOrders(sdword keycode);
static void kbCommandScuttleShips(sdword keycode);
static void kbCommandKamikaze(sdword keycode);
static void kbClearSelectionPopupSuppressFlagCallback(sdword keycode);
static void kbCommandShipSpecialRelease(sdword keycode);
static void kbCommandHyperspace(sdword keycode);
static void kbCommandUpgradeShips(sdword keycode);
static void kbCommandHarvest(sdword keycode);
static void kbCommandRepair(sdword keycode);
static void kbCommandSalvage(sdword keycode);
static void kbCommandTacticsEvasive(sdword keycode);
static void kbCommandTacticsNeutral(sdword keycode);
static void kbCommandTacticsAggresive(sdword keycode);
static void kbCommandNextTacticLevel(sdword keycode);
static void kbCommandPrevTacticLevel(sdword keycode);
static void kbCommandFormationDelta(sdword keycode);
static void kbCommandFormationBroad(sdword keycode);
static void kbCommandFormationDelta3D(sdword keycode);
static void kbCommandFormationClaw(sdword keycode);
static void kbCommandFormationWall(sdword keycode);
static void kbCommandFormationSphere(sdword keycode);
static void kbCommandFormationCustom(sdword keycode);
static void kbCommandFormationParade(sdword keycode);
static void kbCommandNextFormation(sdword keycode);
static void kbCommandPrevFormation(sdword keycode);
static void kbCommandFocus(sdword keycode);
static void kbCommandFocusOnMothership(sdword keycode);
static void kbCommandFocusOnLastShipToSpeak(sdword keycode);
static void kbCommandCancelFocus(sdword keycode);
static void kbCommandForwardFocus(sdword keycode);
static void kbCommandSelectAllVisibleShips(sdword keycode);
static void kbCommandSelectNone(sdword keycode);
static void kbCommandOpenBuildManager(sdword keycode);
static void kbCommandOpenResearchManager(sdword keycode);
static void kbCommandOpenLaunchManager(sdword keycode);
static void kbCommandToggleSensorsManager(sdword keycode);
static void kbCommandToggleTacticalDisplay(sdword keycode);
static void kbCommandForceFieldOptions(sdword keycode);
static void kbCommandForceFieldUnlink(sdword keycode);
static void kbCommandLaunchSwarmers(sdword keycode);
static void kbCommandCancelSwarmers(sdword keycode);
static void kbCommandCloak(sdword keycode);
static void kbCommandCancelCloak(sdword keycode);
static void kbCommandSelectLeechType(sdword keycode);
static void kbCommandLeechSpy(sdword keycode);
static void kbCommandLeechLeech(sdword keycode);
static void kbCommandLeechVect(sdword keycode);
static void kbCommandLeech(sdword keycode);
static void kbCommandUnLeech(sdword keycode);
static void kbCommandUnlink(sdword keycode);
static void kbCommandLink(sdword keycode);
static void kbCommandOpenSpecialOverlay33(sdword keycode);
static void kbCommandOpenBigGunOverlay(sdword keycode);
static void kbCommandCancelExternalConstruction(sdword keycode);
static void kbCommandSelectMothership(sdword keycode);
static void kbCommandBeginBigGunTarget(sdword keycode);
static void kbCommandToggleMoveWaypoints(sdword keycode);
static void kbCommandCycleMoveWaypointType(sdword keycode);
static void kbCommandSetMoveWaypointNormal(sdword keycode);
static void kbCommandSetMoveWaypointEndToEnd(sdword keycode);
static void kbCommandSetMoveWaypointCircular(sdword keycode);
static void kbCommandRemoveLastMoveWaypoint(sdword keycode);
static void kbCommandCancelBigGun(sdword keycode);
static void kbCommandInfectSelectionNearestTarget(sdword keycode);
static void kbCommandRepulseSelectionNearestTarget(sdword keycode);
static void kbCommandRamSelectionNearestTarget(sdword keycode);
static void kbCommandEnterNearestSlipgate(sdword keycode);
static void kbCommandActivateSelectionSpecial(sdword keycode);
static void kbCommandFireMissilesAtNearestTarget(sdword keycode);

int kbBindKeyToCommand(int command, int keycode);
int kbGetCommandsBoundToKey(ubyte keycode, int *commandsBound);
void kbCopyCommandBindings(int command, kbCommandBindingList *bindings);
char *kbGetCommandName(int command);
int kbFormatCommandBindings(int command, char *dest);
int kbGetCommandCategory(int command);
sdword kbGetCommandCategoryName(sdword category);
int kbLoadBindingLine(int unused, char *line);
bool32 kbBindingKeysEquivalent(ubyte left, ubyte right);
int kbFormatSingleBinding(ubyte *bindingBytes, char *dest, int pretty);
int kbValidateBinding(ubyte *bindingBytes, int command, char *errorBuffer);
BOOL kbParseKeyCodeString(char *text, ubyte *keycode);
char *kbKeyCodeToString(int keycode);
static sdword kbSelectedCommand(void);
static void kbRemoveBindingAt(kbCommandBindingList *commandBindings, sdword bindingIndex);
static void kbWriteCurrentSettingsFile(void);
static sdword kbFindCommandByName(const char *configName);
static bool32 kbLoadCommandFile(const char *fileName, kbCommandBindingList *target, sdword *outVersion);
static bool32 kbAddBinding(kbCommandBindingList *target, sdword commandIndex, const kbBinding *binding);
static bool32 kbBindingMatchesCurrentKey(const kbBinding *binding, ubyte keycode);
static bool32 kbBindingIsHeld(const kbBinding *binding);
static bool32 kbIsFirstCommandInCategory(sdword commandIndex);
static bool32 kbSelectionCommandsAllowed(void);
static sdword kbApplySelectionTactics(TacticsType tactics);
static sdword kbApplySelectionFormation(TypeOfFormation formation);
static TacticsType kbStepSelectionTactics(bool32 backward);
void kbMoveSelection(void);
sdword kbSetSelectionEvasiveTactics(void);
sdword kbSetSelectionNeutralTactics(void);
sdword kbSetSelectionAggressiveTactics(void);
sdword kbCycleSelectionTacticsForward(void);
sdword kbCycleSelectionTacticsBackward(void);
sdword kbSetSelectionDeltaFormation(void);
sdword kbSetSelectionBroadFormation(void);
sdword kbSetSelectionDelta3DFormation(void);
sdword kbSetSelectionClawFormation(void);
sdword kbSetSelectionMilitaryParade(void);
udword kbActivateSelectionSpecial(void);
void *kbSelectMothership(void);

/*=============================================================================
    Function Logic :
=============================================================================*/

static const kbBinaryCommandDescriptor kbBinaryCommandDescriptors[KB_MAX_COMMAND_DEFS] =
{
    { NULL, NULL, -1, NULL, NULL },
    { "Move", "Move", 0, kbCommandMove, NULL },
    { "Dock", "Dock", 0, kbCommandDock, NULL },
    { "Retire", "Retire", 0, kbCommandRetire, NULL },
    { "Cancel_Orders", "Cancel Orders", 0, kbCommandCancelOrders, NULL },
    { "Scuttle_Ships", "Scuttle Ships", 0, kbCommandScuttleShips, NULL },
    { "Kamikaze", "Kamikaze", 0, kbCommandKamikaze, NULL },
    { "Ship_Special", "Ship Special", 0, kbClearSelectionPopupSuppressFlagCallback, kbCommandShipSpecialRelease },
    { "Hyperspace", "Hyperspace", 0, NULL, NULL },
    { "Waypoints", "Waypoints", 0, NULL, NULL },
    { "Toggle_Waypoint_Modes", "Toggle Waypoint Modes", 0, NULL, NULL },
    { "Waypoint_Mode_Normal", "Waypoint Mode Normal", 0, NULL, NULL },
    { "Waypoint_Mode_EndToEnd", "Waypoint Mode End To End", 0, NULL, NULL },
    { "Waypoint_Mode_Circular", "Waypoint Mode Circular", 0, NULL, NULL },
    { "Cancel_Last_Waypoint", "Cancel Last Waypoint", 0, NULL, NULL },
    { "Upgrade_Ships", "Upgrade Ships", 0, kbCommandUpgradeShips, NULL },
    { "Enter_Slipgate", "Enter Slipgate", 0, NULL, NULL },
    { "Harvest", "Harvest", 1, kbCommandHarvest, NULL },
    { "Repair", "Repair", 1, kbCommandRepair, NULL },
    { "Salvage", "Salvage", 1, kbCommandSalvage, NULL },
    { "Select_Leech_Type", "Select Leech Type", 1, kbCommandSelectLeechType, NULL },
    { "Leech_Spy", "Leech Spy", 1, kbCommandLeechSpy, NULL },
    { "Leech_Leech", "Leech Leech", 1, kbCommandLeechLeech, NULL },
    { "Leech_Vect", "Leech Vect", 1, kbCommandLeechVect, NULL },
    { "Leech", "Leech", 1, kbCommandLeech, NULL },
    { "UnLeech", "Un Leech", 1, kbCommandUnLeech, NULL },
    { "Link", "Link", 1, kbCommandLink, NULL },
    { "Unlink", "Unlink", 1, NULL, NULL },
    { "Select_Mimic_Type", "Select Mimic Type", 1, NULL, NULL },
    { "Select_Mimic_Item", "Select Mimic Item", 1, NULL, NULL },
    { "Cancel_Mimic", "Cancel Mimic", 1, NULL, NULL },
    { "Force_Field_Options", "Force Field Options", 1, kbCommandForceFieldOptions, NULL },
    { "Force_Field_Unlink", "Force Field Unlink", 1, kbCommandForceFieldUnlink, NULL },
    { "Cloak", "Cloak", 1, kbCommandCloak, NULL },
    { "Cancel_Cloak", "Cancel Cloak", 1, kbCommandCancelCloak, NULL },
    { "Launch_Swarmers", "Launch Swarmers", 1, kbCommandLaunchSwarmers, NULL},
    { "Cancel_Swarmers", "Cancel Swarmers", 1, kbCommandCancelSwarmers, NULL},
    { "Big_Gun_Attack", "Big Gun Attack", 1, kbCommandBeginBigGunTarget, NULL },
    { "Cancel_BigGun", "Cancel Big Gun", 1, NULL, NULL },
    { "Beast_Attack", "Beast Attack", 1, kbCommandInfectSelectionNearestTarget, NULL },
    { "EMP", "EMP", 1, NULL, NULL },
    { "Ram", "Ram", 1, kbCommandRamSelectionNearestTarget, NULL },
    { "Repulse", "Repulse", 1, NULL, NULL },
    { "Fire_Missiles", "Fire Missiles", 2, kbCommandFireMissilesAtNearestTarget, NULL },
    { "Tactics_Evasive", "Tactics Evasive", 2, kbCommandTacticsEvasive, NULL },
    { "Tactics_Neutral", "Tactics Neutral", 2, kbCommandTacticsNeutral, NULL },
    { "Tactics_Aggresive", "Tactics Aggresive", 2, kbCommandTacticsAggresive, NULL },
    { "Next_Tactic_Level", "Next Tactic Level", 2, kbCommandNextTacticLevel, NULL },
    { "Prev_Tactic_Level", "Prev Tactic Level", 3, kbCommandPrevTacticLevel, NULL },
    { "Formation_Delta", "Formation Delta", 3, kbCommandFormationDelta, NULL },
    { "Formation_Broad", "Formation Broad", 3, kbCommandFormationBroad, NULL },
    { "Formation_Delta3D", "Formation Delta3D", 3, kbCommandFormationDelta3D, NULL },
    { "Formation_Claw", "Formation Claw", 3, kbCommandFormationClaw, NULL },
    { "Formation_Wall", "Formation Wall", 3, kbCommandFormationWall, NULL },
    { "Formation_Sphere", "Formation Sphere", 3, kbCommandFormationSphere, NULL },
    { "Formation_Custom", "Formation Custom", 3, kbCommandFormationCustom, NULL },
    { "Formation_Parade", "Formation Parade", 3, kbCommandFormationParade, NULL },
    { "Next_Formation", "Next Formation", 3, kbCommandNextFormation, NULL },
    { "Prev_Formation", "Prev Formation", 4, kbCommandPrevFormation, NULL },
    { "Focus", "Focus", 4, kbCommandFocus, NULL },
    { "Focus_On_Mothership", "Focus On Mothership", 4, kbCommandFocusOnMothership, NULL },
    { "Focus_On_LastShipToSpeak", "Focus On Last Ship To Speak", 4, kbCommandFocusOnLastShipToSpeak, NULL },
    { "Cancel_Focus", "Cancel Focus", 4, kbCommandCancelFocus, NULL },
    { "Forward_Focus", "Forward Focus", 4, kbCommandForwardFocus, NULL },
    { "Fleet_View", "Fleet View", 4, NULL, NULL },
    { "Pan_Sensors_Left", "Pan Sensors Left", 4, NULL, NULL },
    { "Pan_Sensors_Right", "Pan Sensors Right", 4, NULL, NULL },
    { "Pan_Sensors_Up", "Pan Sensors Up", 4, NULL, NULL },
    { "Pan_Sensors_Down", "Pan Sensors Down", 4, NULL, NULL },
    { "Zoom_In", "Zoom In", 4, NULL, NULL },
    { "Zoom_Out", "Zoom Out", 4, NULL, NULL },
    { "Toggle_Chase_Mode", "Toggle Chase Mode", 5, NULL, NULL },
    { "Select_All_Visible_Ships", "Select All Visible Ships", 5, kbCommandSelectAllVisibleShips, NULL },
    { "Select_None", "Select None", 5, kbCommandSelectNone, NULL },
    { "Bandbox_Guard", "Bandbox Guard", 5, NULL, NULL },
    { "Bandbox_Attack", "Bandbox Attack", 5, NULL, NULL },
    { "Bandbox_ForceAttack", "Bandbox Force Attack", 5, NULL, NULL },
    { "Bandbox_Focus", "Bandbox Focus", 5, NULL, NULL },
    { "Multiple_Select", "Multiple Select", 6, NULL, NULL },
    { "Toggle_SensorsManager", "Toggle Sensors Manager", 6, NULL, NULL },
    { "Open_BuildManager", "Open Build Manager", 6, kbCommandOpenBuildManager, NULL },
    { "Open_ResearchManager", "Open Research Manager", 6, kbCommandOpenResearchManager, NULL },
    { "Open_LaunchManager", "Open Launch Manager", 6, kbCommandOpenLaunchManager, NULL },
    { "Toggle_Tactical_Display", "Toggle Tactical Display", 6, kbCommandToggleTacticalDisplay, NULL },
    { "Toggle_Ship_Info", "Toggle Ship Info", 6, NULL, NULL },
    { "Next_Overlay_View", "Next Overlay View", 8, NULL, NULL },
    { "Open_Menu", "Open Menu", 8, NULL, NULL },
    { "Enable_Height_Move", "Enable Height Move", 8, NULL, NULL },
    { "PageDown_GameChat", "Page Down Game Chat", 8, NULL, NULL },
    { "PageUp_GameChat", "Page Up Game Chat", 8, NULL, NULL },
    { "Escape", "Escape", 8, kbCommandEscape, NULL },
    { "Start_Ally_Message", "Start Ally Message", 8, NULL, NULL },
    { "Start_Global_Message", "Start Global Message", 8, NULL, NULL },
    { "Accelerate_Time", "Accelerate Time", 8, kbCommandAccelerateTime, NULL },
    { "Decelerate_Time", "Decelerate Time", 8, kbCommandDecelerateTime, NULL },
    { "Toggle_Game_Speed", "Toggle Game Speed", 8, kbCommandToggleGameSpeed, NULL },
    { "Next_Music_Track", "Next Music Track", 8, kbCommandNextMusicTrack, NULL },
    { "Prev_Music_Track", "Prev Music Track", 8, kbCommandPrevMusicTrack, NULL },
    { "Toggle_Overlay_Options", "Toggle Overlay Options", 8, NULL, NULL },
    { "Change_Overlay_Backdrops_Toggle", "Change Overlay Backdrops Toggle", 8, NULL, NULL },
    { "Show_Keys_1", "Show Keys 1", 8, NULL, NULL },
    { "Show_Keys_2", "Show Keys 2", 8, NULL, NULL },
    { "Show_Keys_3", "Show Keys 3", 8, NULL, NULL },
    { "GammaUp", "Gamma Up", 8, kbCommandGammaUp, NULL },
    { "GammaDown", "Gamma Down", 8, kbCommandGammaDown, NULL },
    { "Pause", "Pause", 8, kbCommandPause, NULL },
    { "Take_Screenshot", "Take Screenshot", 8, kbCommandTakeScreenshot, NULL },
    { "Skip_Sensors", "Skip Sensors", 7, NULL, NULL },
    { "Create_ShipGroup_1", "Create Ship Group 1", 7, NULL, NULL },
    { "Create_ShipGroup_2", "Create Ship Group 2", 7, NULL, NULL },
    { "Create_ShipGroup_3", "Create Ship Group 3", 7, NULL, NULL },
    { "Create_ShipGroup_4", "Create Ship Group 4", 7, NULL, NULL },
    { "Create_ShipGroup_5", "Create Ship Group 5", 7, NULL, NULL },
    { "Create_ShipGroup_6", "Create Ship Group 6", 7, NULL, NULL },
    { "Create_ShipGroup_7", "Create Ship Group 7", 7, NULL, NULL },
    { "Create_ShipGroup_8", "Create Ship Group 8", 7, NULL, NULL },
    { "Create_ShipGroup_9", "Create Ship Group 9", 7, NULL, NULL },
    { "Select_ShipGroup_1", "Select Ship Group 1", 7, NULL, NULL },
    { "Select_ShipGroup_2", "Select Ship Group 2", 7, NULL, NULL },
    { "Select_ShipGroup_3", "Select Ship Group 3", 7, NULL, NULL },
    { "Select_ShipGroup_4", "Select Ship Group 4", 7, NULL, NULL },
    { "Select_ShipGroup_5", "Select Ship Group 5", 7, NULL, NULL },
    { "Select_ShipGroup_6", "Select Ship Group 6", 7, NULL, NULL },
    { "Select_ShipGroup_7", "Select Ship Group 7", 7, NULL, NULL },
    { "Select_ShipGroup_8", "Select Ship Group 8", 7, NULL, NULL },
    { "Select_ShipGroup_9", "Select Ship Group 9", 7, NULL, NULL },
    { "AddTo_ShipGroup_1", "Add To Ship Group 1", 7, NULL, NULL },
    { "AddTo_ShipGroup_2", "Add To Ship Group 2", 7, NULL, NULL },
    { "AddTo_ShipGroup_3", "Add To Ship Group 3", 7, NULL, NULL },
    { "AddTo_ShipGroup_4", "Add To Ship Group 4", 7, NULL, NULL },
    { "AddTo_ShipGroup_5", "Add To Ship Group 5", 7, NULL, NULL },
    { "AddTo_ShipGroup_6", "Add To Ship Group 6", 7, NULL, NULL },
    { "AddTo_ShipGroup_7", "Add To Ship Group 7", 7, NULL, NULL },
    { "AddTo_ShipGroup_8", "Add To Ship Group 8", 7, NULL, NULL },
    { "AddTo_ShipGroup_9", "Add To Ship Group 9", 7, NULL, NULL },
    { "Remove_Selected_Ship_Group", "Remove Selected Ship Group", 7, NULL, NULL },
    { "ToggleSV_Asteroids", "Toggle SV Asteroids", 7, NULL, NULL },
    { "ToggleSV_FOWRanges", "Toggle SV FOW Ranges", 7, NULL, NULL },
    { "ToggleSV_AreasVisited", "Toggle SV Areas Visited", 9, NULL, NULL },
    { "ToggleSV_AreasOccupied", "Toggle SV Areas Occupied", 9, NULL, NULL },
    { "ToggleSV_Icons", "Toggle SV Icons", 9, NULL, NULL },
    { "ToggleSV_ShipMeshes", "Toggle SV Ship Meshes", 9, NULL, NULL },
    { "ToggleSV_PieplateLines", "Toggle SV Pieplate Lines", 9, NULL, NULL },
    { "ToggleSV_Background", "Toggle SV Background", 9, NULL, NULL },
    { "ToggleSV_DecorativeAsteroids", "Toggle SV Decorative Asteroids", 9, NULL, NULL },
    { "ToggleSV_Compass", "Toggle SV Compass", 9, NULL, NULL },
    { "Toggle_SV_ObjPoint", "Toggle SV Obj Point", 9, NULL, NULL },
    { "Toggle_SV_SlipgatePaths", "Toggle SV Slipgate Paths", 9, NULL, NULL },
    { "Play_NIS", "Play NIS", 9, NULL, NULL },
    { "Play_NISLET_1", "Play NISLET 1", 9, NULL, NULL },
    { "Play_NISLET_2", "Play NISLET 2", 0, NULL, NULL },
};

static const kbDirectCommandTranslation kbDirectCommandTranslations[] =
{
    { "Next_Formation", TABKEY },
    { "Open_BuildManager", BKEY },
    { "Cancel_Focus", CKEY },
    { "Forward_Focus", VKEY },
    { "Dock", DKEY },
    { "Select_All_Visible_Ships", EKEY },
    { "Focus", FKEY },
    { "Open_ResearchManager", RKEY },
    { "Harvest", HKEY },
    { "Move", MKEY },
    { "Next_Tactic_Level", RBRACK },
    { "Prev_Tactic_Level", LBRACK },
    { "Scuttle_Ships", SKEY },
    { "Toggle_Tactical_Display", CAPSLOCKKEY },
    { "Toggle_Ship_Info", KB_CMD_TOGGLE_SHIP_INFO },
    { "Next_Overlay_View", KB_CMD_NEXT_OVERLAY_VIEW },
    { "Prev_Overlay_View", KB_CMD_PREV_OVERLAY_VIEW },
    { "Toggle_Overlay_Options", KB_CMD_TOGGLE_OVERLAY_OPTIONS },
    { "Change_Overlay_Backdrops_Toggle", KB_CMD_TOGGLE_OVERLAY_BACKDROPS },
    { "Big_Gun_Attack", KB_CMD_BEGIN_BIG_GUN_TARGET },
    { "Focus_On_Mothership", HOMEKEY },
    { "Kamikaze", KKEY },
    { "Cancel_Orders", TILDEKEY },
    { "Open_LaunchManager", LKEY },
    { "Zoom_In", PLUSKEY },
    { "Zoom_Out", MINUSKEY },
    { "Next_Music_Track", GREATERTHAN },
    { "Prev_Music_Track", LESSTHAN },
};

const kbCommandCallbackPair kbCommandCallbackTable[KB_MAX_COMMAND_DEFS] =
{
    { NULL, NULL },
    { kbCommandMove, NULL },
    { kbCommandDock, NULL },
    { kbCommandRetire, NULL },
    { kbCommandCancelOrders, NULL },
    { kbCommandScuttleShips, NULL },
    { kbCommandKamikaze, NULL },
    { kbClearSelectionPopupSuppressFlagCallback, kbCommandShipSpecialRelease },
    { kbCommandHyperspace, NULL },
    { kbCommandToggleMoveWaypoints, NULL },
    { kbCommandCycleMoveWaypointType, NULL },
    { kbCommandSetMoveWaypointNormal, NULL },
    { kbCommandSetMoveWaypointEndToEnd, NULL },
    { kbCommandSetMoveWaypointCircular, NULL },
    { kbCommandRemoveLastMoveWaypoint, NULL },
    { kbCommandUpgradeShips, NULL },
    { kbCommandEnterNearestSlipgate, NULL },
    { kbCommandHarvest, NULL },
    { kbCommandRepair, NULL },
    { kbCommandSalvage, NULL },
    { kbCommandSelectLeechType, NULL },
    { kbCommandLeechSpy, NULL },
    { kbCommandLeechLeech, NULL },
    { kbCommandLeechVect, NULL },
    { kbCommandLeech, NULL },
    { kbCommandUnLeech, NULL },
    { kbCommandLink, NULL },
    { kbCommandUnlink, NULL },
    { kbCommandOpenSpecialOverlay33, NULL },
    { kbCommandOpenBigGunOverlay, NULL },
    { kbCommandCancelExternalConstruction, NULL },
    { kbCommandForceFieldOptions, NULL },
    { kbCommandForceFieldUnlink, NULL },
    { kbCommandCloak, NULL },
    { kbCommandCancelCloak, NULL },
    { kbCommandLaunchSwarmers, NULL },
    { kbCommandCancelSwarmers, NULL },
    { kbCommandBeginBigGunTarget, NULL },
    { kbCommandCancelBigGun, NULL },
    { kbCommandInfectSelectionNearestTarget, NULL },
    { kbCommandRepulseSelectionNearestTarget, NULL },
    { kbCommandRamSelectionNearestTarget, NULL },
    { kbCommandActivateSelectionSpecial, NULL },
    { kbCommandFireMissilesAtNearestTarget, NULL },
    { kbCommandTacticsEvasive, NULL },
    { kbCommandTacticsNeutral, NULL },
    { kbCommandTacticsAggresive, NULL },
    { kbCommandNextTacticLevel, NULL },
    { kbCommandPrevTacticLevel, NULL },
    { kbCommandFormationDelta, NULL },
    { kbCommandFormationBroad, NULL },
    { kbCommandFormationDelta3D, NULL },
    { kbCommandFormationClaw, NULL },
    { kbCommandFormationWall, NULL },
    { kbCommandFormationSphere, NULL },
    { kbCommandFormationCustom, NULL },
    { kbCommandFormationParade, NULL },
    { kbCommandNextFormation, NULL },
    { kbCommandPrevFormation, NULL },
    { kbCommandFocus, NULL },
    { kbCommandFocusOnMothership, NULL },
    { kbCommandFocusOnLastShipToSpeak, NULL },
    { kbCommandCancelFocus, NULL },
    { kbCommandForwardFocus, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { kbCommandSelectAllVisibleShips, NULL },
    { kbCommandSelectNone, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { kbCommandToggleSensorsManager, NULL },
    { kbCommandOpenBuildManager, NULL },
    { kbCommandOpenResearchManager, NULL },
    { kbCommandOpenLaunchManager, NULL },
    { kbCommandToggleTacticalDisplay, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { kbCommandEscape, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { kbCommandAccelerateTime, NULL },
    { kbCommandDecelerateTime, NULL },
    { kbCommandToggleGameSpeed, NULL },
    { kbCommandNextMusicTrack, NULL },
    { kbCommandPrevMusicTrack, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { kbCommandGammaUp, NULL },
    { kbCommandGammaDown, NULL },
    { kbCommandPause, NULL },
    { kbCommandTakeScreenshot, NULL },
    { kbCommandBuildNarratorUpdate, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
};


/*-----------------------------------------------------------------------------
    Name        : kbListTitleDraw
    Description : This function draws the title for the keyboard list window
    Inputs      : Rectangle of draw area
    Outputs     : none
    Parameters  : rectangle *rect
    Return      : void
-----------------------------------------------------------------------------*/
void kbListTitleDraw(rectangle *rect)
{
    fonthandle oldfont;
    rectangle r = *rect;

    oldfont = fontMakeCurrent(kbKeyListFont);

    fontPrintf(rect->x0+6, rect->y0, colWhite, strGetString(strCommandToBind));
    fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, colWhite, strGetString(strKeyBound));

    r.x0 -= 2;
    r.x1++;
    r.y1--;

    primRectTranslucent2(&r, colRGBA(0,160,100,63));
//    primLine2(r.x0,r.y1,r.x1,r.y1,colRGB(20,200,20));
//    primRectOutline2(&r, 1, colRGB(20,200,20));

    fontMakeCurrent(oldfont);
}


/*-----------------------------------------------------------------------------
    Name        : kbListItemDraw
    Description : This function draws each item in the list window
    Inputs      : rect, and the item pointer
    Outputs     : none
    Parameters  : rectangle *rect, listitemhandle data
    Return      : void
-----------------------------------------------------------------------------*/
void kbListItemDraw(rectangle *rect, listitemhandle data)
{
    fonthandle      oldfont;
    color           c = FEC_ListItemStandard;
    sdword          commandIndex;
    char            bindingText[128];
    const char     *commandName;
    const char     *categoryName;
    sdword          category;

    oldfont = fontMakeCurrent(kbKeyListFont);

    // if selected then this color
    if (bitTest(data->flags, UICLI_Selected))
    {
        c = FEC_ListItemSelected;
    }

    if (kbCataclysmBindingsLoaded)
    {
        commandIndex = (sdword)(udword)data->data;
        commandName = kbGetCommandName(commandIndex);
        category = kbGetCommandCategory(commandIndex);
        categoryName = (const char *)kbGetCommandCategoryName(category);

        if (kbCommandWaitingForKey == commandIndex)
        {
            c = colRGB(255,255,20);
            fontPrintf(rect->x0 + 6, rect->y0, c, "%s%s%s",
                       (categoryName != NULL && kbIsFirstCommandInCategory(commandIndex)) ? categoryName : "",
                       (categoryName != NULL && kbIsFirstCommandInCategory(commandIndex)) ? "  " : "  ",
                       commandName);
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "??????");
        }
        else
        {
            kbFormatCommandBindings(commandIndex, bindingText);
            if (kbCommandCurrent[commandIndex].numBindings == 0)
            {
                c = colRGB(255,20,20);
            }

            fontPrintf(rect->x0 + 6, rect->y0, c, "%s%s%s",
                       (categoryName != NULL && kbIsFirstCommandInCategory(commandIndex)) ? categoryName : "",
                       (categoryName != NULL && kbIsFirstCommandInCategory(commandIndex)) ? "  " : "  ",
                       commandName);
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "%s", bindingText);
        }

        fontMakeCurrent(oldfont);
        return;
    }

    {
        kbBoundKeys *bkey = (kbBoundKeys *)data->data;

        if (bkey->status == KB_NotBound)
        {
            // if not bound then print in red
            c = colRGB(255,20,20);
        }
        else if (bkey->status == KB_GetKeyPress)
        {
            // if not bound then print in yellow color
            c = colRGB(255,255,20);
        }

        fontPrintf(rect->x0 + 6, rect->y0, c, strGetString(bkey->command + strKeyCommandOffset));

        if (bkey->status == KB_GetKeyPress)
        {
            // if getting keypress then show it with ????
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "??????");
        }
        else if (bkey->primarykey == 0)
        {
            // no key assigned to this
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "%s", strGetString(strNoKeyBound));
        }
        else
        {
            // key assigned so print the name of that key
            dbgAssert(bkey->primarykey < KEY_TOTAL_KEYS);
            fontPrintf(rect->x0 + (((rect->x1-rect->x0)*3)/5), rect->y0, c, "%s", strGetString(kbKeyToString[bkey->primarykey]));
        }
    }

    fontMakeCurrent(oldfont);
}


/*-----------------------------------------------------------------------------
    Name        : kbListWindowCB
    Description : This function initializes the key bindings list window
    Inputs      : name, and atom
    Outputs     : none
    Parameters  : char *string, featom *atom
    Return      : void
-----------------------------------------------------------------------------*/
void kbListWindowCB(char *string, featom *atom)
{
    fonthandle oldfont;
    sdword     index;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(kbKeyListFont);
        kbListWindow = (listwindowhandle)atom->pData;

        uicListWindowInit(kbListWindow,
                          kbListTitleDraw,                      //  title draw
                          NULL,                                 //  title click process, no title
                          fontHeight(" ")+(fontHeight(" ")>>1), //  title height, no title
                          kbListItemDraw,                       // item draw funtcion
                          fontHeight(" ")+(fontHeight(" ")>>1),       // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);

        if (kbCataclysmBindingsLoaded)
        {
            for (index = 1; index <= kbCommandCount; index++)
            {
                uicListAddItem(kbListWindow, (ubyte *)index, UICLI_CanSelect, UICLW_AddToHead);
            }
        }
        else
        {
            for (index = 0; index < kbTOTAL_COMMANDS; index++)
            {
                uicListAddItem(kbListWindow, (ubyte *)&kbKeyTable[index], UICLI_CanSelect, UICLW_AddToHead);
            }
        }

        fontMakeCurrent(oldfont);
        return;
    }
    else if (FELASTCALL(atom))
    {
        bitClear(kbListWindow->windowflags, UICLW_GetKeyPressed);
        kbCommandWaitingForKey = -1;

        if (!kbCataclysmBindingsLoaded)
        {
            for (index = 0; index < kbTOTAL_COMMANDS; index++)
            {
                if (kbKeyTable[index].primarykey == 0)
                {
                    kbKeyTable[index].status = KB_NotBound;
                }
                else
                {
                    kbKeyTable[index].status = KB_NormalBound;
                }
            }
        }

        kbListWindow = NULL;
        return;
    }
    // General callbacks because of user input
    switch (kbListWindow->message)
    {
        case CM_AcceptText:
        case CM_DoubleClick:
            bitSet(kbListWindow->windowflags, UICLW_GetKeyPressed);
#ifdef DEBUG_STOMP
            regVerify((regionhandle)&kbListWindow->reg);
#endif
            bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
            if (kbCataclysmBindingsLoaded)
            {
                kbCommandWaitingForKey = kbSelectedCommand();
            }
            else
            {
                ((kbBoundKeys *)kbListWindow->CurLineSelected->data)->status = KB_GetKeyPress;
            }
            break;
        case CM_KeyCaptured:
            kbSetNewKey(kbListWindow->keypressed);
            break;
        case CM_LoseFocus:
        case CM_NewItemSelected:
            bitClear(kbListWindow->windowflags, UICLW_GetKeyPressed);
            kbCommandWaitingForKey = -1;
            if (!kbCataclysmBindingsLoaded)
            {
                for (index = 0; index < kbTOTAL_COMMANDS; index++)
                {
                    if (kbKeyTable[index].primarykey == 0)
                    {
                        kbKeyTable[index].status = KB_NotBound;
                    }
                    else
                    {
                        kbKeyTable[index].status = KB_NormalBound;
                    }
                }
            }
            break;
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbPoolListItemDraw
    Description : This function draws each item in the key pool list window
    Inputs      : rect, and the item pointer
    Outputs     : none
    Parameters  : rectangle *rect, listitemhandle data
    Return      : void
-----------------------------------------------------------------------------*/
void kbPoolListItemDraw(rectangle *rect, listitemhandle data)
{
    fonthandle      oldfont;
    color           c = FEC_ListItemStandard;
    sword           key1, key2;
    udword          udata = (udword)data->data;

    oldfont = fontMakeCurrent(kbKeyListFont);

    // unpack the two key's
    key1 = (sword) (udata&(0x0000FFFF));
    key2 = (sword) ((udata&(0xFFFF0000))>>16);

    // if selected then this color
    if (bitTest(data->flags, UICLI_Selected))
    {
        c = FEC_ListItemSelected;
    }

    if (key1 != -1)
    {
        fontPrintf(rect->x0 + 6, rect->y0, c, strGetString(kbKeyToString[key1]));
    }

    if (key2 != -1)
    {
        fontPrintf(rect->x0 + ( (rect->x1-rect->x0)>>1 ), rect->y0, c, strGetString(kbKeyToString[key2]));
    }

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : kbPoolListWindowCB
    Description : This function Initializes the list of keys window
    Inputs      : keypressed
    Outputs     : none
    Parameters  : udword keypressed
    Return      : void
-----------------------------------------------------------------------------*/
void kbPoolListWindowCB(char *string, featom *atom)
{
    fonthandle          oldfont;
    static sdword       index;
    listwindowhandle    keypool;
    sdword              done;
    udword              data;
    sword               key1, key2;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(kbKeyListFont);
        keypool = (listwindowhandle)atom->pData;

        uicListWindowInit(keypool,
                          NULL,                      //  title draw
                          NULL,                      //  title click process, no title
                          0,                         //  title height, no title
                          kbPoolListItemDraw,                       // item draw funtcion
                          fontHeight(" ")+(fontHeight(" ")>>1),       // item height
                          UICLW_CanSelect|UICLW_CanHaveFocus);

        done = 0;
        for (index=0;index<KEY_TOTAL_KEYS;index++)
        {
            if (kbCanMapKey[index] == TRUE)
            {
                if (done == 0)
                {
                    key1 = (sword)index;
                    done = 1;
                }
                else if (done == 1)
                {
                    key2 = (sword)index;
                    done = 2;
                }
            }

            if (done == 2)
            {
                // pack the two 16-bit swords into the on udword
                data = (udword) ( (key1)| ((key2<<16)) );
                key1 = -1;
                key2 = -1;
                done = 0;
                uicListAddItem(keypool, (ubyte *)data,  UICLI_CanSelect, UICLW_AddToTail);
            }
        }
        if (done == 1)
        {
            // pack the two 16-bit swords into the on udword
            key2 = -1;
            data = (udword) ( (key1)| ((key2<<16)) );
            key1 = -1;
            done = 0;
            uicListAddItem(keypool, (ubyte *)data,  UICLI_CanSelect, UICLW_AddToTail);
        }

        fontMakeCurrent(oldfont);
        return;
    }
    else if (FELASTCALL(atom))
    {
        return;
    }
}

/*-----------------------------------------------------------------------------
    Name        : kbSetNewKey
    Description : This function sets the new keypressed to the currently selected command.
    Inputs      : keypressed
    Outputs     : none
    Parameters  : udword keypressed
    Return      : void
-----------------------------------------------------------------------------*/
void kbSetNewKey(udword keypressed)
{
    sdword       index;
    sdword       commandIndex;
    kbBinding    binding;

    keypressed&=0x000000FF;

    dbgAssert(keypressed < KEY_TOTAL_KEYS);

    if (kbCataclysmBindingsLoaded)
    {
        commandIndex = kbSelectedCommand();

        if ((commandIndex <= 0) || (commandIndex > kbCommandCount))
        {
            kbCommandWaitingForKey = -1;
            return;
        }

        if (kbCanMapKey[keypressed])
        {
            memset(&binding, 0, sizeof(binding));
            binding.keycode = (ubyte)keypressed;
            binding.shift = (keypressed != SHIFTKEY) && keyIsHit(SHIFTKEY);
            binding.ctrl = (keypressed != CONTROLKEY) && keyIsHit(CONTROLKEY);
            binding.alt = (keypressed != ALTKEY) && keyIsHit(ALTKEY);

            for (index = 1; index <= kbCommandCount; index++)
            {
                sdword bindingIndex;

                for (bindingIndex = kbCommandCurrent[index].numBindings - 1; bindingIndex >= 0; bindingIndex--)
                {
                    if (!memcmp(&kbCommandCurrent[index].bindings[bindingIndex], &binding, sizeof(binding)))
                    {
                        kbRemoveBindingAt(&kbCommandCurrent[index], bindingIndex);
                    }
                }
            }

            kbCommandCurrent[commandIndex].numBindings = 0;
            kbAddBinding(kbCommandCurrent, commandIndex, &binding);
            kbCommandWaitingForKey = -1;
        }
        else
        {
            kbCommandWaitingForKey = -1;
            if (gameIsRunning)
            {
                feScreenStart(feStack[feStackIndex].baseRegion, "In_Game_Invalid_Key");
            }
            else
            {
                feScreenStart(feStack[feStackIndex].baseRegion, "Invalid_Key");
            }
        }
#ifdef DEBUG_STOMP
        regVerify((regionhandle)&kbListWindow->reg);
#endif
        bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
        return;
    }

    {
        kbBoundKeys *bkey = (kbBoundKeys *)kbListWindow->CurLineSelected->data;

    if (kbCanMapKey[keypressed])
    {
        for (index=0;index<kbTOTAL_COMMANDS;index++)
        {
            if (kbKeyTable[index].primarykey == keypressed)
            {
                kbKeyTable[index].status = KB_NotBound;
                kbKeyTable[index].primarykey = 0;
            }
        }
        bkey->primarykey = keypressed;
        bkey->status = KB_NormalBound;
#ifdef DEBUG_STOMP
        regVerify((regionhandle)&kbListWindow->reg);
#endif
        bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
    }
    else
    {
        if (bkey->primarykey != 0)
            bkey->status = KB_NormalBound;
        else
            bkey->status = KB_NotBound;

        if (gameIsRunning)
        {
            feScreenStart(feStack[feStackIndex].baseRegion, "In_Game_Invalid_Key");
        }
        else
        {
            feScreenStart(feStack[feStackIndex].baseRegion, "Invalid_Key");
        }
        // this key is not a key that can be remapped.
    }
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbKeyResetToDefault
    Description : This function will reset all of the keyboard bindings to
                  default.
    Inputs      : name, and atom
    Outputs     : none
    Parameters  : char *string, featom *atom
    Return      : void
-----------------------------------------------------------------------------*/
void kbKeyResetToDefault(char *string, featom *atom)
{
    (void)string;
    (void)atom;

    kbLoadSettings(TRUE);
    bitSet(kbListWindow->reg.status, RSF_DrawThisFrame);
}


static void kbCommandAccelerateTime(sdword keycode)
{
    (void)keycode;
    taskTimeCompressionIncrease();
}

static void kbCommandDecelerateTime(sdword keycode)
{
    (void)keycode;
    taskTimeCompressionDecrease();
}

static void kbCommandToggleGameSpeed(sdword keycode)
{
    (void)keycode;
    taskTimeCompressionToggle();
}

static void kbCommandNextMusicTrack(sdword keycode)
{
    (void)keycode;
    musicEventNextTrack();
}

static void kbCommandPrevMusicTrack(sdword keycode)
{
    (void)keycode;
    musicEventPrevTrack();
}

static void kbCommandGammaUp(sdword keycode)
{
    (void)keycode;
    if (RGL)
    {
        rglFeature(KB_RGL_GAMMA_UP);
    }
    shGammaUp();
}

static void kbCommandGammaDown(sdword keycode)
{
    (void)keycode;
    if (RGL)
    {
        rglFeature(KB_RGL_GAMMA_DN);
    }
    shGammaDown();
}

static void kbCommandPause(sdword keycode)
{
    (void)keycode;
    if (!multiPlayerGame)
    {
        tutGameMessage("KB_Pause");
        if ((tutorial == TUTORIAL_ONLY) && (!tutEnable.bPauseGame))
        {
            return;
        }

        if (!universePause && (piePointSpecMode != PSM_Idle))
        {
            piePointModeOnOff();
        }

        universePause = !universePause;
        dbgMessage(universePause ? "\nPause ON" : "\nPause OFF");
        soundEvent(NULL, UI_Click);
    }
}

static void kbCommandEscape(sdword keycode)
{
    (void)keycode;
    mrHandleEscape();
}

static void kbCommandBuildNarratorUpdate(sdword keycode)
{
    (void)keycode;
    cmBuildNarratorUpdate();
}

static void kbCommandTakeScreenshot(sdword keycode)
{
    (void)keycode;
    soundEvent(NULL, UI_Click);
    rndTakeScreenshot = TRUE;
}

static void kbCommandMove(sdword keycode)
{
    (void)keycode;
    kbMoveSelection();
}

static void kbCommandHyperspace(sdword keycode)
{
    (void)keycode;
    mrHyperspace(NULL, NULL);
}

static void kbCommandUpgradeShips(sdword keycode)
{
    (void)keycode;
    mrUpgrade(NULL, NULL);
}

static void kbCommandDock(sdword keycode)
{
    (void)keycode;
    mrDockingOrders(NULL, NULL);
}

static void kbCommandRetire(sdword keycode)
{
    (void)keycode;
    mrRetire(NULL, NULL);
}

static void kbCommandCancelOrders(sdword keycode)
{
    (void)keycode;
    mrCancel(NULL, NULL);
}

static void kbCommandScuttleShips(sdword keycode)
{
    (void)keycode;
    mrScuttle(NULL, NULL);
}

static void kbCommandKamikaze(sdword keycode)
{
    sdword index;
    sdword numShips;
    CommandToDo *command;

    (void)keycode;

    if (!MakeSelectionKamikazeCapable((SelectCommand *)&selSelected))
        return;

    numShips = selSelected.numShips;
    for (index = 0; index < numShips; index++)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer, selSelected.ShipPtr[index]);
        if (command != NULL &&
            (command->ordertype.order == COMMAND_ATTACK ||
             command->ordertype.order == COMMAND_SPECIAL))
        {
            soundEventPlay(0, 264, 0);
            speechEventQueue(selSelected.ShipPtr[0], 67111079, selSelected.ShipPtr[0]->shiprace,
                -1, -1, -1, -1, 0.0f, -1);
            clWrapSetKamikaze(&universe.mainCommandLayer, (SelectCommand *)&selSelected);
            return;
        }
    }

    if (numShips > 0)
    {
        speechEventQueue(selSelected.ShipPtr[0], 67110921, selSelected.ShipPtr[0]->shiprace,
            -1, -1, -1, -1, 0.0f, -1);
    }
}

static void kbClearSelectionPopupSuppressFlagCallback(sdword keycode)
{
    (void)keycode;
    kbSuppressSpecialReleaseOverlay = 0;
}

static void kbCommandShipSpecialRelease(sdword keycode)
{
    sdword index;

    (void)keycode;
    if (selSelected.numShips > 0 && !smBandboxEnabled && !kbSuppressSpecialReleaseOverlay)
    {
        MaxAnySelection selection;

        selSelectionCopy(&selection, (MaxAnySelection *)&selSelected);
        if (selection.numTargets > 0)
        {
            kbSuppressNextSpecialActivate = 0;
            for (index = 0; index < selSelected.numShips; index++)
            {
                if (ShipHasUpgrade(selSelected.ShipPtr[index], RESEARCH_UPGRADE_MASK_CLOAKING))
                {
                    kbSuppressNextSpecialActivate = 1;
                    break;
                }
            }
            smOverlayOpen(TRUE, NULL);
        }
    }
}

static void kbCommandHarvest(sdword keycode)
{
    (void)keycode;
    mrHarvestResources(NULL, NULL);
}

static void kbCommandRepair(sdword keycode)
{
    (void)keycode;
    mrActivateSingleClickSpecialOnNearestFriendly(NULL);
}

static void kbCommandSalvage(sdword keycode)
{
    (void)keycode;
    mrSalvageSelectionNearestTarget(NULL);
}

static void kbCommandTacticsEvasive(sdword keycode)
{
    (void)keycode;
    (void)mrSetSelectionTactics(Evasive, TRUE);
}

static void kbCommandTacticsNeutral(sdword keycode)
{
    (void)keycode;
    (void)mrSetSelectionTactics(Neutral, TRUE);
}

static void kbCommandTacticsAggresive(sdword keycode)
{
    (void)keycode;
    (void)mrSetSelectionTactics(Aggressive, TRUE);
}

static void kbCommandNextTacticLevel(sdword keycode)
{
    (void)keycode;
    (void)kbCycleSelectionTacticsForward();
}

static void kbCommandPrevTacticLevel(sdword keycode)
{
    (void)keycode;
    (void)kbCycleSelectionTacticsBackward();
}

static void kbCommandFormationDelta(sdword keycode)
{
    (void)keycode;
    (void)kbSetSelectionDeltaFormation();
}

static void kbCommandFormationBroad(sdword keycode)
{
    (void)keycode;
    (void)kbSetSelectionBroadFormation();
}

static void kbCommandFormationDelta3D(sdword keycode)
{
    (void)keycode;
    (void)kbSetSelectionDelta3DFormation();
}

static void kbCommandFormationClaw(sdword keycode)
{
    (void)keycode;
    (void)kbSetSelectionClawFormation();
}

static void kbCommandFormationWall(sdword keycode)
{
    (void)keycode;
    (void)kbApplySelectionFormation(WALL_FORMATION);
}

static void kbCommandFormationSphere(sdword keycode)
{
    (void)keycode;
    (void)kbApplySelectionFormation(SPHERE_FORMATION);
}

static void kbCommandFormationCustom(sdword keycode)
{
    (void)keycode;
    (void)kbApplySelectionFormation(CUSTOM_FORMATION);
}

static void kbCommandFormationParade(sdword keycode)
{
    (void)keycode;
    (void)kbSetSelectionMilitaryParade();
}

static void kbCommandNextFormation(sdword keycode)
{
    TypeOfFormation alreadyformed;

    (void)keycode;

    if (!kbSelectionCommandsAllowed() || selSelected.numShips < MIN_SHIPS_IN_FORMATION)
    {
        return;
    }

    if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
    {
        mrNewFormation = (TypeOfFormation)KB_PARADE_FORMATION_FLAG;
        mrFormationName = strGetString(strPARADE_FORMATION);
    }
    else
    {
        alreadyformed = clSelectionAlreadyInFormation(&universe.mainCommandLayer, (SelectCommand *)&selSelected);
        if ((alreadyformed == NO_FORMATION && mrFormationName == NULL) ||
            (mrNewFormation == (TypeOfFormation)KB_PARADE_FORMATION_FLAG))
        {
            mrNewFormation = 0;
        }
        else
        {
            mrNewFormation++;
            if (mrNewFormation >= CUSTOM_FORMATION)
            {
                mrNewFormation = 0;
            }
        }
        mrFormationName = TypeOfFormationToNiceStr(mrNewFormation);
    }

    mrFormationTime = universe.totaltimeelapsed;
    mrDrawFormationTime = universe.totaltimeelapsed;
    mrDrawFormation = TRUE;
    soundEvent(NULL, UI_Click);
}

static void kbCommandPrevFormation(sdword keycode)
{
    TypeOfFormation alreadyformed;

    (void)keycode;

    if (!kbSelectionCommandsAllowed() || selSelected.numShips < MIN_SHIPS_IN_FORMATION)
    {
        return;
    }

    if (MothershipOrCarrierIndexInSelection((SelectCommand *)&selSelected) >= 0)
    {
        mrNewFormation = (TypeOfFormation)KB_PARADE_FORMATION_FLAG;
        mrFormationName = strGetString(strPARADE_FORMATION);
    }
    else
    {
        alreadyformed = clSelectionAlreadyInFormation(&universe.mainCommandLayer, (SelectCommand *)&selSelected);
        if ((alreadyformed == NO_FORMATION && mrFormationName == NULL) ||
            (mrNewFormation == (TypeOfFormation)KB_PARADE_FORMATION_FLAG))
        {
            mrNewFormation = 0;
        }
        else if (mrNewFormation <= 0)
        {
            mrNewFormation = CUSTOM_FORMATION - 1;
        }
        else
        {
            mrNewFormation--;
        }
        mrFormationName = TypeOfFormationToNiceStr(mrNewFormation);
    }

    mrFormationTime = universe.totaltimeelapsed;
    mrDrawFormationTime = universe.totaltimeelapsed;
    mrDrawFormation = TRUE;
    soundEvent(NULL, UI_Click);
}

static void kbCommandFocus(sdword keycode)
{
    MaxSelection tempSelection;

    (void)keycode;

    if (!kbSelectionCommandsAllowed() || selSelected.numShips == 0)
    {
        return;
    }

    tutGameMessage("KB_Focus");
    soundEvent(NULL, UI_Click);

    tempSelection = selSelected;
    MakeShipHostsIncludeAttachedShips((SelectCommand *)&tempSelection);
    ccFocus(&universe.mainCameraCommand, (FocusCommand *)&tempSelection);
}

static void kbCommandFocusOnMothership(sdword keycode)
{
    (void)keycode;
    (void)kbSelectMothership();
}

static void kbCommandFocusOnLastShipToSpeak(sdword keycode)
{
    (void)keycode;

    if (lastshiptospeak == NULL)
    {
        return;
    }

    if (lastshiptospeak == (Ship *)-1)
    {
        if (selHotKeyGroup[lastgrouptospeak].numShips != 0)
        {
            tutGameMessage("KB_FocusLast");
            selSelectHotKeyGroup(&selHotKeyGroup[lastgrouptospeak]);
            selHotKeyNumbersSet(lastgrouptospeak);
            ioUpdateShipTotals();
            if (selSelected.numShips > 0)
            {
                soundEvent(NULL, UI_Click);
                ccFocus(&universe.mainCameraCommand, (FocusCommand *)&selSelected);
            }
        }
    }
    else
    {
        tutGameMessage("KB_FocusLast");
        selSelecting.TargetPtr[0] = (SpaceObjRotImpTarg *)lastshiptospeak;
        selSelecting.numTargets = 1;
        soundEvent(NULL, UI_Click);
        ccFocus(&universe.mainCameraCommand, (FocusCommand *)&selSelecting);
        selSelecting.numTargets = 0;
    }
}

static void kbCommandCancelFocus(sdword keycode)
{
    (void)keycode;
    tutGameMessage("KB_CancelFocus");
    ccCancelFocus(&universe.mainCameraCommand);
    soundEvent(NULL, UI_ClickCancel);
}

static void kbCommandForwardFocus(sdword keycode)
{
    (void)keycode;
    tutGameMessage("KB_ForwardFocus");
    ccForwardFocus(&universe.mainCameraCommand);
    soundEvent(NULL, UI_ClickAccept);
}

static void kbCommandSelectAllVisibleShips(sdword keycode)
{
    rectangle fullScreen = {0, 0, MAIN_WindowWidth, MAIN_WindowHeight};
    MaxSelection tempSelection;

    (void)keycode;

    if ((tutorial == TUTORIAL_ONLY) && !tutEnable.bBandSelect)
    {
        return;
    }

    tutGameMessage("KB_SelectEveryone");
    selSelectionCopy((MaxAnySelection *)&tempSelection, (MaxAnySelection *)&selSelected);
    selRectSelect(&(universe.mainCameraCommand.actualcamera), &fullScreen);
    if ((tempSelection.numShips != 0) &&
        (selSelectionCompare((MaxAnySelection *)&selSelected, (MaxAnySelection *)&tempSelection) == 0))
    {
        selSelected.numShips = 0;
    }
    selSelecting.numTargets = 0;
    ioUpdateShipTotals();
    soundEvent(NULL, UI_Click);
}

static void kbCommandSelectNone(sdword keycode)
{
    (void)keycode;
    tutGameMessage("KB_SelectNone");
    soundEvent(NULL, UI_Click);
    selSelectNone();
}

static void kbCommandOpenBuildManager(sdword keycode)
{
    (void)keycode;
    mrBuildShips(NULL, NULL);
}

static void kbCommandOpenResearchManager(sdword keycode)
{
    (void)keycode;
    mrResearch(NULL, NULL);
}

static void kbCommandOpenLaunchManager(sdword keycode)
{
    (void)keycode;
    mrLaunch(NULL, NULL);
}

static void kbCommandToggleTacticalDisplay(sdword keycode)
{
    (void)keycode;
    smPathMarkersEnabled ^= TRUE;
    mrDrawTactical = smPathMarkersEnabled;
    if (smPathMarkersEnabled)
    {
        soundEvent(NULL, UI_TacOverlayOn);
        tutGameMessage("KB_TacticalDisplayOn");
    }
    else
    {
        soundEvent(NULL, UI_TacOverlayOff);
        tutGameMessage("KB_TacticalDisplayOff");
    }
}

static void kbCommandForceFieldOptions(sdword keycode)
{
    (void)keycode;
    mrProtectSelection(NULL);
}

static void kbCommandForceFieldUnlink(sdword keycode)
{
    (void)keycode;
    mrCycleSelectionLeechMode();
}

static void kbCommandLaunchSwarmers(sdword keycode)
{
    (void)keycode;
    mrLaunchSwarmersNearestTarget(NULL);
}

static void kbCommandCancelSwarmers(sdword keycode)
{
    (void)keycode;
    mrLaunchSelectionSwarmers();
}

static void kbCommandCloak(sdword keycode)
{
    (void)keycode;
    kbSuppressNextSpecialActivate = 0;
    mrCloakSelection();
}

static void kbCommandCancelCloak(sdword keycode)
{
    (void)keycode;
    kbSuppressNextSpecialActivate = 0;
    mrDecloakSelection();
}

static void kbCommandSelectLeechType(sdword keycode)
{
    MaxSelection selection;

    (void)keycode;
    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    MakeShipsOnlyHaveUpgrade(&selection.numShips, 64);
    MakeShipsSpecialActivateCapable((SelectCommand *)&selection, TRUE);
    if (selection.numShips > 0)
    {
        smOverlayOpenView28();
    }
}

static void kbCommandLeechSpy(sdword keycode)
{
    (void)keycode;
    mrSetSelectionLeechMode(6);
}

static void kbCommandLeechLeech(sdword keycode)
{
    (void)keycode;
    mrSetSelectionLeechMode(7);
}

static void kbCommandLeechVect(sdword keycode)
{
    (void)keycode;
    mrSetSelectionLeechMode(8);
}

static void kbCommandLeech(sdword keycode)
{
    (void)keycode;
    mrLeechSelectionNearestTarget(NULL);
}

static void kbCommandUnLeech(sdword keycode)
{
    (void)keycode;
    mrUnleechSelection();
}

static void kbCommandUnlink(sdword keycode)
{
    (void)keycode;
    mrLaunchSelectionShips();
}

static void kbCommandToggleMoveWaypoints(sdword keycode)
{
    (void)keycode;
    mrToggleMoveWaypoints();
}

static void kbCommandCycleMoveWaypointType(sdword keycode)
{
    (void)keycode;
    // TODO(cataclysm): Confirm the remaining retail waypoint UI side effects around this helper.
    mrCycleMoveWaypointType();
}

static void kbCommandSetMoveWaypointNormal(sdword keycode)
{
    (void)keycode;
    mrSetMoveWaypointType(MR_MoveWaypointTypeLinear);
}

static void kbCommandSetMoveWaypointEndToEnd(sdword keycode)
{
    (void)keycode;
    mrSetMoveWaypointType(MR_MoveWaypointTypeEndToEnd);
}

static void kbCommandSetMoveWaypointCircular(sdword keycode)
{
    (void)keycode;
    mrSetMoveWaypointType(MR_MoveWaypointTypeCircular);
}

static void kbCommandRemoveLastMoveWaypoint(sdword keycode)
{
    (void)keycode;
    // TODO(cataclysm): Confirm the remaining retail cleanup/UI refresh side effects here.
    mrRemoveLastMoveWaypoint();
}

static void kbCommandCancelBigGun(sdword keycode)
{
    (void)keycode;
    // TODO(cataclysm): Retail cancel-big-gun handling is still unreconciled.
}

static void kbCommandLink(sdword keycode)
{
    (void)keycode;
    mrLinkSelection();
}

static void kbCommandOpenSpecialOverlay33(sdword keycode)
{
    MaxSelection selection;

    (void)keycode;
    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    MakeShipsOnlyHaveUpgrade(&selection.numShips, 32);
    MakeShipsSpecialActivateCapable((SelectCommand *)&selection, TRUE);
    if (selection.numShips > 0)
    {
        smOverlayOpenView33();
    }
}

static void kbCommandOpenBigGunOverlay(sdword keycode)
{
    MaxSelection selection;

    (void)keycode;
    selSelectionCopyBounded(&selection, (MaxSelection *)&selSelected);
    MakeShipsOnlyHaveUpgrade(&selection.numShips, 32);
    MakeShipsSpecialActivateCapable((SelectCommand *)&selection, TRUE);
    if (selection.numShips > 0)
    {
        smOverlayOpenView34();
    }
}

static void kbCommandCancelExternalConstruction(sdword keycode)
{
    (void)keycode;
    mrCancelExternalConstructionSelection();
}

static void kbCommandSelectMothership(sdword keycode)
{
    (void)keycode;
    (void)kbSelectMothership();
}

static void kbCommandBeginBigGunTarget(sdword keycode)
{
    (void)keycode;
    (void)mrSelectMothership();
}

static void kbCommandInfectSelectionNearestTarget(sdword keycode)
{
    (void)keycode;
    mrInfectSelectionNearestTarget(NULL);
}

static void kbCommandRepulseSelectionNearestTarget(sdword keycode)
{
    (void)keycode;
    mrRepulseSelectionNearestTarget(NULL);
}

static void kbCommandRamSelectionNearestTarget(sdword keycode)
{
    (void)keycode;
    mrRamSelectionNearestTarget(NULL);
}

static void kbCommandEnterNearestSlipgate(sdword keycode)
{
    (void)keycode;
    mrEnterNearestSlipgate();
}

static void kbCommandActivateSelectionSpecial(sdword keycode)
{
    (void)keycode;
    (void)kbActivateSelectionSpecial();
}

static void kbCommandFireMissilesAtNearestTarget(sdword keycode)
{
    (void)keycode;
    mrFireMissilesAtNearestTarget(NULL);
}

static void kbCommandToggleSensorsManager(sdword keycode)
{
    (void)keycode;
    if (!smSensorsActive)
    {
        tutGameMessage("KB_Sensors");
        smSensorsBegin(NULL, NULL);
    }
    else if (!dword_A5B1B0)
    {
        smSensorsClose(NULL, NULL);
    }
}

bool32 kbTryToggleSensorsManager(sdword keycode)
{
    static sdword toggleSensorsCommand = -2;
    kbCommandBindingList *bindings;
    sdword bindingIndex;

    if (!kbCataclysmBindingsLoaded)
    {
        if (keycode != SPACEKEY)
        {
            return FALSE;
        }
    }
    else
    {
        if (toggleSensorsCommand == -2)
        {
            toggleSensorsCommand = kbFindCommandByName("Toggle_SensorsManager");
        }
        if (toggleSensorsCommand <= 0)
        {
            return FALSE;
        }

        bindings = &kbCommandCurrent[toggleSensorsCommand];
        for (bindingIndex = 0; bindingIndex < bindings->numBindings; bindingIndex++)
        {
            if (kbBindingMatchesCurrentKey(&bindings->bindings[bindingIndex], (ubyte)keycode))
            {
                goto toggleSensors;
            }
        }
        return FALSE;
    }

toggleSensors:
    if (smSensorsActive)
    {
        smSensorsClose(NULL, NULL);
    }
    else
    {
        smSensorsBegin(NULL, NULL);
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : kbCheckBindings
    Description : This function will checks to see if this key is a remappable
                  key and then do the conversion.
    Inputs      : keypressed
    Outputs     : none
    Parameters  : sdword keypressed
    Return      : void
-----------------------------------------------------------------------------*/
sdword kbCheckBindings(sdword keypressed)
{
    sdword index;

    if (kbCataclysmBindingsLoaded)
    {
        for (index = 0; index < (sdword)(sizeof(kbDirectCommandTranslations) / sizeof(kbDirectCommandTranslations[0])); index++)
        {
            sdword command;
            sdword bindingIndex;

            command = kbFindCommandByName(kbDirectCommandTranslations[index].configName);
            if (command <= 0)
            {
                continue;
            }

            for (bindingIndex = 0; bindingIndex < kbCommandCurrent[command].numBindings; bindingIndex++)
            {
                if (kbBindingMatchesCurrentKey(&kbCommandCurrent[command].bindings[bindingIndex], (ubyte)keypressed))
                {
                    return kbDirectCommandTranslations[index].translatedKey;
                }
            }
        }

        if (kbCanMapKey[keypressed])
        {
            return 0;
        }
        return keypressed;
    }

    if (kbCanMapKey[keypressed])
    {
        for (index=0;index<kbTOTAL_COMMANDS;index++)
        {
            if (kbKeyTable[index].primarykey == keypressed)
            {
                return (kbKeyTable[index].defaultkey);
            }
        }

        // key not bound to anything !!!!!
        return(0);
    }
    else
    {
        return (keypressed);
    }
}

bool32 kbInvokeBoundCommandPressCallbacks(sdword keycode)
{
    sdword commandsBound[151];
    sdword commandCount;
    sdword index;
    bool32 handled;

    handled = FALSE;
    commandCount = kbGetCommandsBoundToKey((ubyte)keycode, commandsBound);
    for (index = 0; index < commandCount; index++)
    {
        sdword command;
        kbCommandCallback callback;

        command = commandsBound[index];
        callback = kbBinaryCommandDescriptors[command].press;
        if (callback == NULL)
        {
            continue;
        }

        callback(keycode);
        handled = TRUE;
    }

    return handled;
}

bool32 kbInvokeBoundCommandReleaseCallbacks(sdword keycode)
{
    sdword commandsBound[151];
    sdword commandCount;
    sdword index;
    bool32 handled;

    handled = FALSE;
    commandCount = kbGetCommandsBoundToKey((ubyte)keycode, commandsBound);
    for (index = 0; index < commandCount; index++)
    {
        sdword command;
        kbCommandCallback callback;

        command = commandsBound[index];
        callback = kbBinaryCommandDescriptors[command].release;
        if (callback == NULL)
        {
            continue;
        }

        callback(keycode);
        handled = TRUE;
    }

    return handled;
}


/*-----------------------------------------------------------------------------
    Name        : kbRestoreSavedSettings
    Description : This function will restore the previosly saved settings.
    Inputs      : none
    Outputs     : none
    Parameters  : void
    Return      : void
-----------------------------------------------------------------------------*/
void kbRestoreSavedSettings(void)
{
    sdword index;

    if (kbCataclysmBindingsLoaded)
    {
        memcpy(kbCommandCurrent, kbCommandSaved, sizeof(kbCommandCurrent));
        kbCommandWaitingForKey = -1;
        return;
    }

    for (index=0; index < kbTOTAL_COMMANDS; index++)
    {
        kbKeyTable[index] = kbKeySaveTable[index];
    }
}


/*-----------------------------------------------------------------------------
    Name        : kbSaveSettings
    Description : This function will save the current settings to a temporary
                  structure.  This is called when the options are started so
                  That a user can cancel any changes they don't like.
    Inputs      : void
    Outputs     : none
    Parameters  : void
    Return      : void
-----------------------------------------------------------------------------*/
void kbSaveSettings(void)
{
    if (kbCataclysmBindingsLoaded)
    {
        kbWriteCurrentSettingsFile();
    }
}

/*-----------------------------------------------------------------------------
    Name        : kbKeyUsed
    Description : This functino returns true if the key is already bound to something, FALSE otherwise
    Inputs      : key to check
    Outputs     : boolean
    Parameters  : sdword keytocheck
    Return      : bool32
-----------------------------------------------------------------------------*/
bool32 kbKeyUsed(udword keytocheck)
{
    sdword index, count=0;

    for (index=0; index < kbTOTAL_COMMANDS; index++)
    {
        if (kbKeyTable[index].primarykey == keytocheck)
        {
            count++;
        }
    }

    if (count > 1) return(TRUE);

    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : kbCommandKeyIsHit
    Description : This function will return true if the key is hit for the command specified
    Inputs      : command
    Outputs     : boolean
    Parameters  : udword command
    Return      : bool32
-----------------------------------------------------------------------------*/
bool32 kbCommandKeyIsHit(udword command)
{
    sdword resolvedCommand;
    sdword bindingIndex;

    if (kbCataclysmBindingsLoaded)
    {
        if (command >= kbTOTAL_COMMANDS)
        {
            return FALSE;
        }

        resolvedCommand = kbFindCommandByName(kbLegacyCommandNames[command]);
        if ((resolvedCommand <= 0) || (resolvedCommand > kbCommandCount))
        {
            return FALSE;
        }

        for (bindingIndex = 0; bindingIndex < kbCommandCurrent[resolvedCommand].numBindings; bindingIndex++)
        {
            if (kbBindingIsHeld(&kbCommandCurrent[resolvedCommand].bindings[bindingIndex]))
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    return(keyIsHit(kbKeyTable[command].primarykey));
}

bool32 kbCommandIsHitByName(const char *configName)
{
    sdword command;
    sdword bindingIndex;

    if (!kbCataclysmBindingsLoaded)
    {
        return FALSE;
    }

    command = kbFindCommandByName(configName);
    if (command <= 0)
    {
        return FALSE;
    }

    for (bindingIndex = 0; bindingIndex < kbCommandCurrent[command].numBindings; bindingIndex++)
    {
        if (kbBindingIsHeld(&kbCommandCurrent[command].bindings[bindingIndex]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : kbKeyBoundToCommand
    Description : This function returns the key that is bound to that command
    Inputs      : Command to get the key for
    Outputs     : key bound
    Parameters  : udword command
    Return      : udword
-----------------------------------------------------------------------------*/
udword kbKeyBoundToCommand(udword command)
{
    if (kbKeyTable != NULL) return(kbKeyTable[command].primarykey);

    return (0);
}

void kbRegisterListFont(void)
{
    kbKeyListFont = frFontRegister("hw_eurosecond_11.hff");
}


/*-----------------------------------------------------------------------------
    Name        : kbInitKeyBindings
    Description : This function Initializes the keybindings.
    Inputs      : void
    Outputs     : none
    Parameters  : void
    Return      : void
-----------------------------------------------------------------------------*/
void kbInitKeyBindings(void)
{
    sdword index;
    bool32   bInFile=FALSE;

    kbRegisterListFont();

    for (index=0;index<KEY_TOTAL_KEYS;index++)
    {
        kbCanMapKey[index] = FALSE;
    }

    if (!bkDisableKeyRemap)
    {
        kbCanMapKey[TABKEY]         = TRUE;
        kbCanMapKey[BKEY]           = TRUE;
        kbCanMapKey[CKEY]           = TRUE;
        kbCanMapKey[VKEY]           = TRUE;
        kbCanMapKey[DKEY]           = TRUE;
        kbCanMapKey[EKEY]           = TRUE;
        kbCanMapKey[FKEY]           = TRUE;
        kbCanMapKey[RKEY]           = TRUE;
        kbCanMapKey[HKEY]           = TRUE;
        kbCanMapKey[MKEY]           = TRUE;
        kbCanMapKey[RBRACK]         = TRUE;
        kbCanMapKey[LBRACK]         = TRUE;
        kbCanMapKey[SKEY]           = TRUE;
        kbCanMapKey[ZKEY]           = TRUE;
        kbCanMapKey[CAPSLOCKKEY]    = TRUE;
//        kbCanMapKey[SPACEKEY]       = TRUE;
        kbCanMapKey[HOMEKEY]        = TRUE;
        kbCanMapKey[KKEY]           = TRUE;
        kbCanMapKey[TILDEKEY]       = TRUE;
        kbCanMapKey[LKEY]           = TRUE;
        // start of extra keys
//        kbCanMapKey[PRINTKEY]       = TRUE;
        kbCanMapKey[ARRUP]          = TRUE;
        kbCanMapKey[ARRDOWN]        = TRUE;
        kbCanMapKey[ENDKEY]         = TRUE;
        kbCanMapKey[INSERTKEY]      = TRUE;
        kbCanMapKey[DELETEKEY]      = TRUE;
//        kbCanMapKey[LESSTHAN]       = TRUE;
//        kbCanMapKey[GREATERTHAN]    = TRUE;
        kbCanMapKey[AKEY]           = TRUE;
        kbCanMapKey[NKEY]           = TRUE;
        kbCanMapKey[OKEY]           = TRUE;

        if (pilotView)
        {
            kbCanMapKey[QKEY]           = FALSE;
        }
        else
        {
            kbCanMapKey[QKEY]           = TRUE;
        }

        kbCanMapKey[UKEY]           = TRUE;
        kbCanMapKey[WKEY]           = TRUE;
        kbCanMapKey[XKEY]           = TRUE;
        kbCanMapKey[YKEY]           = TRUE;
        kbCanMapKey[NUMPAD0]        = TRUE;
        kbCanMapKey[NUMPAD1]        = TRUE;
        kbCanMapKey[NUMPAD2]        = TRUE;
        kbCanMapKey[NUMPAD3]        = TRUE;
        kbCanMapKey[NUMPAD4]        = TRUE;
        kbCanMapKey[NUMPAD5]        = TRUE;
        kbCanMapKey[NUMPAD6]        = TRUE;
        kbCanMapKey[NUMPAD7]        = TRUE;
        kbCanMapKey[NUMPAD8]        = TRUE;
        kbCanMapKey[NUMPAD9]        = TRUE;
        kbCanMapKey[NUMSTARKEY]     = TRUE;
        kbCanMapKey[NUMSLASHKEY]    = TRUE;
        kbCanMapKey[NUMDOTKEY]      = TRUE;
 //       kbCanMapKey[BACKSLASHKEY]   = TRUE;
    }

    kbKeyToString[AKEY] =         strAKEY;
    kbKeyToString[BKEY] =         strBKEY;
    kbKeyToString[CKEY] =         strCKEY;
    kbKeyToString[DKEY] =         strDKEY;
    kbKeyToString[EKEY] =         strEKEY;
    kbKeyToString[FKEY] =         strFKEY;
    kbKeyToString[GKEY] =         strGKEY;
    kbKeyToString[HKEY] =         strHKEY;
    kbKeyToString[IKEY] =         strIKEY;
    kbKeyToString[JKEY] =         strJKEY;
    kbKeyToString[KKEY] =         strKKEY;
    kbKeyToString[LKEY] =         strLKEY;
    kbKeyToString[MKEY] =         strMKEY;
    kbKeyToString[NKEY] =         strNKEY;
    kbKeyToString[OKEY] =         strOKEY;
    kbKeyToString[PKEY] =         strPKEY;
    kbKeyToString[QKEY] =         strQKEY;
    kbKeyToString[RKEY] =         strRKEY;
    kbKeyToString[SKEY] =         strSKEY;
    kbKeyToString[TKEY] =         strTKEY;
    kbKeyToString[UKEY] =         strUKEY;
    kbKeyToString[VKEY] =         strVKEY;
    kbKeyToString[WKEY] =         strWKEY;
    kbKeyToString[XKEY] =         strXKEY;
    kbKeyToString[YKEY] =         strYKEY;
    kbKeyToString[ZKEY] =         strZKEY;
    kbKeyToString[BACKSPACEKEY] = strBACKSPACEKEY;
    kbKeyToString[TABKEY] =       strTABKEY;
    kbKeyToString[SHIFTKEY] =     strSHIFTKEY;
    kbKeyToString[CONTROLKEY] =   strCTRLKEY;
    kbKeyToString[ALTKEY] =       strALTKEY;
    kbKeyToString[ARRLEFT] =      strARRLEFT;
    kbKeyToString[ARRRIGHT] =     strARRRIGHT;
    kbKeyToString[ARRUP] =        strARRUP;
    kbKeyToString[ARRDOWN] =      strARRDOWN;
    kbKeyToString[ENDKEY] =       strENDKEY;
    kbKeyToString[LBRACK] =       strLBRACK;
    kbKeyToString[RBRACK] =       strRBRACK;
    kbKeyToString[CAPSLOCKKEY] =  strCAPSLOCKKEY;
    kbKeyToString[SPACEKEY] =     strSPACEKEY;
    kbKeyToString[ENTERKEY] =     strENTERKEY;
    kbKeyToString[HOMEKEY] =      strHOMEKEY;
    kbKeyToString[PAGEDOWNKEY] =  strPAGEDOWNKEY;
    kbKeyToString[PAGEUPKEY] =    strPAGEUPKEY;
    kbKeyToString[BACKSLASHKEY] = strBACKSLASHKEY;
    kbKeyToString[PAUSEKEY] =     strPAUSEKEY;
    kbKeyToString[SCROLLKEY] =    strSCROLLKEY;
    kbKeyToString[PRINTKEY] =     strPRINTKEY;
    kbKeyToString[INSERTKEY] =    strINSERTKEY;
    kbKeyToString[DELETEKEY] =    strDELETEKEY;
    kbKeyToString[LESSTHAN] =     strLESSTHAN;
    kbKeyToString[GREATERTHAN] =  strGREATERTHAN;
    kbKeyToString[TILDEKEY] =     strTILDEKEY;
    kbKeyToString[NUMPAD0] =      strNUMPAD0;
    kbKeyToString[NUMPAD1] =      strNUMPAD1;
    kbKeyToString[NUMPAD2] =      strNUMPAD2;
    kbKeyToString[NUMPAD3] =      strNUMPAD3;
    kbKeyToString[NUMPAD4] =      strNUMPAD4;
    kbKeyToString[NUMPAD5] =      strNUMPAD5;
    kbKeyToString[NUMPAD6] =      strNUMPAD6;
    kbKeyToString[NUMPAD7] =      strNUMPAD7;
    kbKeyToString[NUMPAD8] =      strNUMPAD8;
    kbKeyToString[NUMPAD9] =      strNUMPAD9;
    kbKeyToString[NUMMINUSKEY] =  strNUMMINUSKEY;
    kbKeyToString[NUMPLUSKEY] =   strNUMPLUSKEY;
    kbKeyToString[NUMSTARKEY] =   strNUMSTARKEY;
    kbKeyToString[NUMSLASHKEY] =  strNUMSLASHKEY;
    kbKeyToString[NUMDOTKEY] =    strNUMDOTKEY;
    kbKeyToString[MINUSKEY] =     strMINUSKEY;
    kbKeyToString[PLUSKEY] =      strPLUSKEY;
    kbKeyToString[F1KEY] =        strF1KEY;
    kbKeyToString[F2KEY] =        strF2KEY;
    kbKeyToString[F3KEY] =        strF3KEY;
    kbKeyToString[F4KEY] =        strF4KEY;
    kbKeyToString[F5KEY] =        strF5KEY;
    kbKeyToString[F6KEY] =        strF6KEY;
    kbKeyToString[F7KEY] =        strF7KEY;
    kbKeyToString[F8KEY] =        strF8KEY;
    kbKeyToString[F9KEY] =        strF9KEY;
    kbKeyToString[F10KEY] =       strF10KEY;
    kbKeyToString[F11KEY] =       strF11KEY;
    kbKeyToString[F12KEY] =       strF12KEY;

    for (index=0;index<kbTOTAL_COMMANDS;index++)
    {
        if (kbKeySavedKeys[index] != 0) bInFile = TRUE;
    }

    switch (strCurLanguage)
    {
        case languageEnglish:
            kbKeyTable = &kbKeyTableEnglish[0];
        break;
        case languageFrench:
            kbKeyTable = &kbKeyTableFrench[0];
        break;
        case languageGerman:
            kbKeyTable = &kbKeyTableGerman[0];
        break;
        case languageSpanish:
            kbKeyTable = &kbKeyTableSpanish[0];
        break;
        case languageItalian:
            kbKeyTable = &kbKeyTableItalian[0];
        break;
    }

    if (bInFile)
    {
        for (index=0;index<kbTOTAL_COMMANDS;index++)
        {
            kbKeyTable[index].primarykey = kbKeySavedKeys[index];
        }
    }

    // check the key-bindings and reset anything that's messed up
    for (index=0;index<kbTOTAL_COMMANDS;index++)
    {
        if (kbKeyTable[index].primarykey == 0)
        {
            // i guess the user can have no key for a function ?? don't know why but ...
            kbKeyTable[index].status = KB_NotBound;
            continue;
        }
        // is the key even a mappable one ?
        if (!kbCanMapKey[kbKeyTable[index].primarykey])
        {
            // just assign it to default key
            kbKeyTable[index].primarykey = kbKeyTable[index].defaultkey;
            kbKeyTable[index].status = KB_NormalBound;
        }
        // make sure that it's only used once
        if (kbKeyUsed(kbKeyTable[index].primarykey))
        {   // set to not mapped, can't have the same key do multiple things
            kbKeyTable[index].primarykey = 0;
            kbKeyTable[index].status = KB_NotBound;
        }
    }

    kbLoadSettingsNoReset();
    if (kbCataclysmBindingsLoaded)
    {
        memcpy(kbCommandSaved, kbCommandCurrent, sizeof(kbCommandCurrent));
    }
}

static sdword kbSelectedCommand(void)
{
    if ((kbListWindow == NULL) || (kbListWindow->CurLineSelected == NULL))
    {
        return -1;
    }
    return (sdword)(udword)kbListWindow->CurLineSelected->data;
}

static void kbRemoveBindingAt(kbCommandBindingList *commandBindings, sdword bindingIndex)
{
    sdword index;

    if ((bindingIndex < 0) || (bindingIndex >= commandBindings->numBindings))
    {
        return;
    }

    for (index = bindingIndex; index < commandBindings->numBindings - 1; index++)
    {
        commandBindings->bindings[index] = commandBindings->bindings[index + 1];
    }
    memset(&commandBindings->bindings[commandBindings->numBindings - 1], 0, sizeof(commandBindings->bindings[0]));
    commandBindings->numBindings--;
}

static sdword kbFindCommandByName(const char *configName)
{
    sdword index;

    for (index = 1; index <= kbCommandCount; index++)
    {
        if (!_stricmp(kbBinaryCommandDescriptors[index].configName, configName))
        {
            return index;
        }
    }
    return -1;
}

static bool32 kbAddBinding(kbCommandBindingList *target, sdword commandIndex, const kbBinding *binding)
{
    sdword index;

    if ((commandIndex <= 0) || (commandIndex >= KB_MAX_COMMAND_DEFS))
    {
        return FALSE;
    }

    for (index = 0; index < target[commandIndex].numBindings; index++)
    {
        if (!memcmp(&target[commandIndex].bindings[index], binding, sizeof(*binding)))
        {
            return TRUE;
        }
    }

    if (target[commandIndex].numBindings >= KB_MAX_COMMAND_BINDINGS)
    {
        return FALSE;
    }

    target[commandIndex].bindings[target[commandIndex].numBindings++] = *binding;
    return TRUE;
}

BOOL kbParseKeyCodeString(char *text, ubyte *keycode)
{
    sdword value;

    if ((text == NULL) || (keycode == NULL))
    {
        return FALSE;
    }

    value = atoi(text);
    if ((value <= 0) || (value >= KEY_TOTAL_KEYS))
    {
        return FALSE;
    }

    *keycode = (ubyte)value;
    return TRUE;
}

typedef struct
{
    ubyte keycode;
    ubyte unshifted;
    ubyte shifted;
} kbPrintableKeyPair;

static const kbPrintableKeyPair kbPrintableKeyPairsEnglish[] =
{
    { 48, '0', ')' }, { 49, '1', '!' }, { 50, '2', '@' }, { 51, '3', '#' },
    { 52, '4', '$' }, { 53, '5', '%' }, { 54, '6', '^' }, { 55, '7', '&' },
    { 56, '8', '*' }, { 57, '9', '(' }, { 186, ';', ':' }, { 187, '=', '+' },
    { 188, ',', '<' }, { 189, '-', '_' }, { 190, '.', '>' }, { 191, '/', '?' },
    { 192, '`', '~' }, { 219, '[', '{' }, { 220, '\\', '|' }, { 221, ']', '}' },
    { 222, '\'', '"' }
};

static const kbPrintableKeyPair kbPrintableKeyPairsFrench[] =
{
    { 48, 0xe0, '0' }, { 49, '&', '1' }, { 50, 0xe9, '2' }, { 51, '"', '3' },
    { 52, '\'', '4' }, { 53, '(', '5' }, { 54, '-', '6' }, { 55, 0xe8, '7' },
    { 56, '_', '8' }, { 57, 0xe7, '9' }, { 186, ';', ':' }, { 187, '=', '+' },
    { 188, ';', '.' }, { 189, ')', 0xb0 }, { 190, ':', '/' }, { 191, '/', '?' },
    { 192, 0xb2, 0 }, { 219, 0x88, 0xa8 }, { 220, '*', 0xb5 }, { 221, '$', 0xa3 },
    { 222, '\'', '"' }
};

static const kbPrintableKeyPair kbPrintableKeyPairsGerman[] =
{
    { 48, '0', '=' }, { 49, '1', '!' }, { 50, '2', '"' }, { 51, '3', 0xa7 },
    { 52, '4', '$' }, { 53, '5', '%' }, { 54, '6', '&' }, { 55, '7', '/' },
    { 56, '8', '(' }, { 57, '9', ')' }, { 186, ';', ':' }, { 187, 0xb4, '`' },
    { 188, ',', ';' }, { 189, 0xdf, '?' }, { 190, '.', ':' }, { 191, '/', '?' },
    { 192, 0x88, 0xb0 }, { 219, 0xfc, 0xdc }, { 220, '#', '\'' }, { 221, '+', '*' },
    { 222, '\'', '"' }
};

static const kbPrintableKeyPair kbPrintableKeyPairsSpanish[] =
{
    { 48, '0', '=' }, { 49, '1', '!' }, { 50, '2', '"' }, { 51, '3', 0xb7 },
    { 52, '4', '$' }, { 53, '5', '%' }, { 54, '6', '&' }, { 55, '7', '/' },
    { 56, '8', '(' }, { 57, '9', ')' }, { 186, ';', ':' }, { 187, 0xa1, 0xbf },
    { 188, ',', ';' }, { 189, '\'', '?' }, { 190, '.', ':' }, { 191, '/', '?' },
    { 192, 0xba, 0xaa }, { 219, '`', '^' }, { 220, 0xe7, 0xc7 }, { 221, '+', '*' },
    { 222, '\'', '"' }
};

static const kbPrintableKeyPair kbPrintableKeyPairsItalian[] =
{
    { 48, '0', '=' }, { 49, '1', '!' }, { 50, '2', '"' }, { 51, '3', 0xa3 },
    { 52, '4', '$' }, { 53, '5', '%' }, { 54, '6', '&' }, { 55, '7', '/' },
    { 56, '8', '(' }, { 57, '9', ')' }, { 186, ';', ':' }, { 187, 0xec, '^' },
    { 188, ',', ';' }, { 189, '\'', '?' }, { 190, '.', ':' }, { 191, '/', '?' },
    { 192, '\\', '|' }, { 219, 0xe8, 0xe9 }, { 220, 0xf9, 0xa7 }, { 221, '+', '*' },
    { 222, 0xe0, 0xb0 }
};

static bool32 kbLookupPrintableKeyPair(ubyte keycode, ubyte *unshifted, ubyte *shifted)
{
    const kbPrintableKeyPair *pairs = NULL;
    sdword pairCount = 0;
    sdword index;

    if ((keycode >= 'A') && (keycode <= 'Z'))
    {
        *unshifted = (ubyte)('a' + (keycode - 'A'));
        *shifted = keycode;
        return TRUE;
    }

    if ((keycode >= NUMPAD0) && (keycode <= NUMPAD9))
    {
        *unshifted = (ubyte)('0' + (keycode - NUMPAD0));
        *shifted = 0;
        return TRUE;
    }

    switch (keycode)
    {
        case SPACEKEY:
            *unshifted = ' ';
            *shifted = ' ';
            return TRUE;
        case NUMSTARKEY:
            *unshifted = '*';
            *shifted = 0;
            return TRUE;
        case NUMPLUSKEY:
            *unshifted = '+';
            *shifted = 0;
            return TRUE;
        case NUMMINUSKEY:
            *unshifted = '-';
            *shifted = 0;
            return TRUE;
        case NUMDOTKEY:
            *unshifted = '.';
            *shifted = 0;
            return TRUE;
        case NUMSLASHKEY:
            *unshifted = '/';
            *shifted = 0;
            return TRUE;
        default:
            break;
    }

    switch (strCurKeyboardLanguage)
    {
        case languageFrench:
            pairs = kbPrintableKeyPairsFrench;
            pairCount = sizeof(kbPrintableKeyPairsFrench) / sizeof(kbPrintableKeyPairsFrench[0]);
        break;
        case languageGerman:
            pairs = kbPrintableKeyPairsGerman;
            pairCount = sizeof(kbPrintableKeyPairsGerman) / sizeof(kbPrintableKeyPairsGerman[0]);
        break;
        case languageSpanish:
            pairs = kbPrintableKeyPairsSpanish;
            pairCount = sizeof(kbPrintableKeyPairsSpanish) / sizeof(kbPrintableKeyPairsSpanish[0]);
        break;
        case languageItalian:
            pairs = kbPrintableKeyPairsItalian;
            pairCount = sizeof(kbPrintableKeyPairsItalian) / sizeof(kbPrintableKeyPairsItalian[0]);
        break;
        case languageEnglish:
        default:
            pairs = kbPrintableKeyPairsEnglish;
            pairCount = sizeof(kbPrintableKeyPairsEnglish) / sizeof(kbPrintableKeyPairsEnglish[0]);
        break;
    }

    for (index = 0; index < pairCount; index++)
    {
        if (pairs[index].keycode == keycode)
        {
            *unshifted = pairs[index].unshifted;
            *shifted = pairs[index].shifted;
            return TRUE;
        }
    }

    return FALSE;
}

char *kbKeyCodeToString(int keycode)
{
    sdword stringId;
    ubyte unshifted;
    ubyte shifted;
    ubyte printable;

    switch (keycode)
    {
        case SHIFTKEY:      stringId = strSHIFTKEY; break;
        case ALTKEY:        stringId = strALTKEY; break;
        case CONTROLKEY:    stringId = strCTRLKEY; break;
        case HOMEKEY:       stringId = strHOMEKEY; break;
        case PAGEDOWNKEY:   stringId = strPAGEDOWNKEY; break;
        case PAGEUPKEY:     stringId = strPAGEUPKEY; break;
        case BACKSLASHKEY:  stringId = strBACKSLASHKEY; break;
        case SCROLLKEY:     stringId = strSCROLLKEY; break;
        case PRINTKEY:      stringId = strPRINTKEY; break;
        case INSERTKEY:     stringId = strINSERTKEY; break;
        case DELETEKEY:     stringId = strDELETEKEY; break;
        case F1KEY:         stringId = strF1KEY; break;
        case F2KEY:         stringId = strF2KEY; break;
        case F3KEY:         stringId = strF3KEY; break;
        case F4KEY:         stringId = strF4KEY; break;
        case F5KEY:         stringId = strF5KEY; break;
        case F6KEY:         stringId = strF6KEY; break;
        case F7KEY:         stringId = strF7KEY; break;
        case F8KEY:         stringId = strF8KEY; break;
        case F9KEY:         stringId = strF9KEY; break;
        case F10KEY:        stringId = strF10KEY; break;
        case F11KEY:        stringId = strF11KEY; break;
        case F12KEY:        stringId = strF12KEY; break;
        case ENTERKEY:      stringId = strENTERKEY; break;
        case BACKSPACEKEY:  stringId = strBACKSPACEKEY; break;
        case TABKEY:        stringId = strTABKEY; break;
        case ARRLEFT:       stringId = strARRLEFT; break;
        case ARRRIGHT:      stringId = strARRRIGHT; break;
        case ARRUP:         stringId = strARRUP; break;
        case ARRDOWN:       stringId = strARRDOWN; break;
        case ENDKEY:        stringId = strENDKEY; break;
        case CAPSLOCKKEY:   stringId = strCAPSLOCKKEY; break;
        case SPACEKEY:      stringId = strSPACEKEY; break;
        case ESCKEY:        stringId = strESCKEY; break;
        default:            stringId = -1; break;
    }

    if (stringId >= 0)
    {
        return strGetString(stringId);
    }

    if (kbLookupPrintableKeyPair((ubyte)keycode, &unshifted, &shifted))
    {
        printable = unshifted;
        if ((shifted != 0) && ((keycode < '0') || (keycode > '9')))
        {
            printable = shifted;
        }
        sprintf(kbKeyStringBuffer, "%c", printable);
        return kbKeyStringBuffer;
    }

    sprintf(kbKeyStringBuffer, "%c", keycode);
    return kbKeyStringBuffer;
}

int kbFormatSingleBinding(ubyte *bindingBytes, char *dest, int pretty)
{
    kbBinding *binding = (kbBinding *)bindingBytes;

    dest[0] = '\0';

    if (pretty)
    {
        char ctrlString[128];
        char shiftString[128];
        char altString[128];

        sprintf(ctrlString, "%s+", strGetString(strCTRLKEY));
        sprintf(shiftString, "%s+", strGetString(strSHIFTKEY));
        sprintf(altString, "%s+", strGetString(strALTKEY));

        if (binding->ctrl)
        {
            strcat(dest, ctrlString);
        }
        if (binding->shift)
        {
            strcat(dest, shiftString);
        }
        if (binding->alt)
        {
            strcat(dest, altString);
        }
        strcat(dest, kbKeyCodeToString(binding->keycode));
    }
    else
    {
        if (binding->alt)
        {
            strcat(dest, "ALT+");
        }
        if (binding->shift)
        {
            strcat(dest, "SHIFT+");
        }
        if (binding->ctrl)
        {
            strcat(dest, "CTRL+");
        }
        sprintf(dest + strlen(dest), "%d", binding->keycode);
    }

    return strlen(dest);
}

bool32 kbBindingKeysEquivalent(ubyte left, ubyte right)
{
    if (left == right)
    {
        return TRUE;
    }

    switch (left)
    {
        case WKEY:
            return (right == NKEY);
        case ARRUP:
            return (right == NUMPAD8);
        case ARRDOWN:
            return (right == NUMPAD2);
        case ARRRIGHT:
            return (right == NUMPAD6);
        case ARRLEFT:
            return (right == NUMPAD4);
        case HOMEKEY:
            return (right == NUMPAD7);
        case ENDKEY:
            return (right == NUMPAD1);
        case PAGEUPKEY:
            return (right == NUMPAD9);
        case PAGEDOWNKEY:
            return (right == NUMPAD3);
        default:
            break;
    }

    return FALSE;
}

int kbValidateBinding(ubyte *bindingBytes, int command, char *errorBuffer)
{
    kbBinding *binding = (kbBinding *)bindingBytes;
    sdword index;
    sdword bindingIndex;
    char bindingText[64];

    if ((binding->keycode == SHIFTKEY) || (binding->keycode == CONTROLKEY) || (binding->keycode == ALTKEY))
    {
        if (errorBuffer != NULL)
        {
            sprintf(errorBuffer, "Modifier keys must be combined with another key");
        }
        return 0;
    }

    for (index = 1; index <= kbCommandCount; index++)
    {
        if (index == command)
        {
            continue;
        }

        for (bindingIndex = 0; bindingIndex < kbCommandCurrent[index].numBindings; bindingIndex++)
        {
            kbBinding *existing = &kbCommandCurrent[index].bindings[bindingIndex];

            if (!memcmp(existing, binding, sizeof(*binding)) ||
                kbBindingKeysEquivalent(existing->keycode, binding->keycode))
            {
                if (errorBuffer != NULL)
                {
                    kbFormatSingleBinding(bindingBytes, bindingText, TRUE);
                    sprintf(errorBuffer, "%s is already bound to %s", bindingText, kbGetCommandName(index));
                }
                return 0;
            }
        }
    }

    return 1;
}

int kbGetCommandCategory(int command)
{
    if ((command <= 0) || (command > kbCommandCount))
    {
        return -1;
    }
    return kbBinaryCommandDescriptors[command].category;
}

sdword kbGetCommandCategoryName(sdword category)
{
    switch (category)
    {
        case 0: return (sdword)strGetString(strUIGroup_ShipActions);
        case 1: return (sdword)strGetString(strUIGroup_ShipSpecialActions);
        case 2: return (sdword)strGetString(strUIGroup_ShipTactics);
        case 3: return (sdword)strGetString(strUIGroup_ShipFormations);
        case 4: return (sdword)strGetString(strUIGroup_Camera);
        case 5: return (sdword)strGetString(strUIGroup_Selection);
        case 6: return (sdword)strGetString(strUIGroup_Views);
        case 7: return (sdword)strGetString(strUIGroup_ShipGroupings);
        case 8: return (sdword)strGetString(strUIGroup_Misc);
        case 9: return (sdword)strGetString(strUIGroup_SensorsViewOptions);
        case 10: return (sdword)strGetString(strUIGroup_Debug);
        default: return 0;
    }
}

char *kbGetCommandName(int command)
{
    if ((command <= 0) || (command > kbCommandCount))
    {
        return "";
    }
    return (char *)kbBinaryCommandDescriptors[command].displayName;
}

int kbFormatCommandBindings(int command, char *dest)
{
    sdword index;
    char bindingText[64];

    if ((command <= 0) || (command > kbCommandCount))
    {
        dest[0] = '\0';
        return 0;
    }

    if (kbCommandCurrent[command].numBindings <= 0)
    {
        strcpy(dest, strGetString(str_UI_NoKeyBinding));
        return strlen(dest);
    }

    dest[0] = '\0';
    for (index = 0; index < kbCommandCurrent[command].numBindings; index++)
    {
        if (index > 0)
        {
            strcat(dest, ", ");
        }
        kbFormatSingleBinding((ubyte *)&kbCommandCurrent[command].bindings[index], bindingText, TRUE);
        strcat(dest, bindingText);
    }
    return strlen(dest);
}

void kbCopyCommandBindings(int command, kbCommandBindingList *bindings)
{
    if ((command <= 0) || (command > kbCommandCount))
    {
        memset(bindings, 0, sizeof(*bindings));
        return;
    }
    *bindings = kbCommandCurrent[command];
}

static bool32 kbBindingMatchesCurrentKey(const kbBinding *binding, ubyte keycode)
{
    bool32 shift;
    bool32 ctrl;
    bool32 alt;

    if (binding->keycode != keycode)
    {
        return FALSE;
    }

    shift = (keycode == SHIFTKEY) ? binding->shift : keyIsHit(SHIFTKEY);
    ctrl = (keycode == CONTROLKEY) ? binding->ctrl : keyIsHit(CONTROLKEY);
    alt = (keycode == ALTKEY) ? binding->alt : keyIsHit(ALTKEY);

    return ((binding->shift == shift) &&
            (binding->ctrl == ctrl) &&
            (binding->alt == alt));
}

static bool32 kbBindingIsHeld(const kbBinding *binding)
{
    if (!keyIsHit(binding->keycode))
    {
        return FALSE;
    }

    if ((binding->keycode != SHIFTKEY) && (binding->shift != keyIsHit(SHIFTKEY)))
    {
        return FALSE;
    }

    if ((binding->keycode != CONTROLKEY) && (binding->ctrl != keyIsHit(CONTROLKEY)))
    {
        return FALSE;
    }

    if ((binding->keycode != ALTKEY) && (binding->alt != keyIsHit(ALTKEY)))
    {
        return FALSE;
    }

    return TRUE;
}

int kbGetCommandsBoundToKey(ubyte keycode, int *commandsBound)
{
    sdword command;
    sdword bindingIndex;
    sdword count;

    count = 0;
    for (command = 1; command <= kbCommandCount; command++)
    {
        for (bindingIndex = 0; bindingIndex < kbCommandCurrent[command].numBindings; bindingIndex++)
        {
            if (kbBindingMatchesCurrentKey(&kbCommandCurrent[command].bindings[bindingIndex], keycode))
            {
                commandsBound[count++] = command;
                break;
            }
        }
    }
    return count;
}

int kbBindKeyToCommand(int command, int keycode)
{
    kbBinding binding;
    char errorBuffer[128];

    if ((command <= 0) || (command > kbCommandCount))
    {
        return 0;
    }

    memset(&binding, 0, sizeof(binding));
    binding.keycode = (ubyte)keycode;
    binding.shift = (keycode != SHIFTKEY) && keyIsHit(SHIFTKEY);
    binding.ctrl = (keycode != CONTROLKEY) && keyIsHit(CONTROLKEY);
    binding.alt = (keycode != ALTKEY) && keyIsHit(ALTKEY);

    if (!kbValidateBinding((ubyte *)&binding, command, errorBuffer))
    {
        return 0;
    }

    kbCommandCurrent[command].numBindings = 0;
    return kbAddBinding(kbCommandCurrent, command, &binding);
}

int kbLoadBindingLine(int unused, char *line)
{
    char localLine[256];
    char valueLine[256];
    char *name;
    char *value;
    char *commandName;
    char *bindingText;
    kbBinding binding;
    sdword commandIndex;

    (void)unused;

    strcpy(localLine, line);
    if (!parseLine(localLine, &name, &value) || _stricmp(name, "key"))
    {
        return FALSE;
    }

    strcpy(valueLine, value);
    if (!parseLine(valueLine, &commandName, &bindingText))
    {
        return FALSE;
    }

    commandIndex = kbFindCommandByName(commandName);
    if (commandIndex < 0)
    {
        return FALSE;
    }

    memset(&binding, 0, sizeof(binding));
    {
        char bindingCopy[64];
        char *token;
        char *keyToken;

        strcpy(bindingCopy, bindingText);
        keyToken = NULL;
        for (token = strtok(bindingCopy, "+"); token != NULL; token = strtok(NULL, "+"))
        {
            if (!_stricmp(token, "SHIFT"))
            {
                binding.shift = TRUE;
            }
            else if (!_stricmp(token, "CTRL"))
            {
                binding.ctrl = TRUE;
            }
            else if (!_stricmp(token, "ALT"))
            {
                binding.alt = TRUE;
            }
            else
            {
                keyToken = token;
            }
        }

        if ((keyToken == NULL) || !kbParseKeyCodeString(keyToken, &binding.keycode))
        {
            return FALSE;
        }
    }

    return kbAddBinding(kbCommandCurrent, commandIndex, &binding);
}

static bool32 kbLoadCommandFile(const char *fileName, kbCommandBindingList *target, sdword *outVersion)
{
    FILE *file;
    char line[256];
    char localLine[256];
    char valueLine[256];
    char *name;
    char *value;
    char *commandName;
    char *bindingText;
    kbBinding binding;
    sdword commandIndex;
    sdword version;

    file = fopen(fileName, "rt");
    if (file == NULL)
    {
        return FALSE;
    }

    version = 0;
    if (fgets(line, sizeof(line), file) != NULL)
    {
        strcpy(localLine, line);
        if (parseLine(localLine, &name, &value) && !_stricmp(name, "v"))
        {
            version = atoi(value);
        }
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        strcpy(localLine, line);
        if (!parseLine(localLine, &name, &value) || _stricmp(name, "key"))
        {
            continue;
        }

        strcpy(valueLine, value);
        if (!parseLine(valueLine, &commandName, &bindingText))
        {
            continue;
        }

        commandIndex = kbFindCommandByName(commandName);
        if (commandIndex < 0)
        {
            continue;
        }

        memset(&binding, 0, sizeof(binding));
        {
            char bindingCopy[64];
            char *token;
            char *keyToken;

            strcpy(bindingCopy, bindingText);
            keyToken = NULL;
            for (token = strtok(bindingCopy, "+"); token != NULL; token = strtok(NULL, "+"))
            {
                if (!_stricmp(token, "SHIFT"))
                {
                    binding.shift = TRUE;
                }
                else if (!_stricmp(token, "CTRL"))
                {
                    binding.ctrl = TRUE;
                }
                else if (!_stricmp(token, "ALT"))
                {
                    binding.alt = TRUE;
                }
                else
                {
                    keyToken = token;
                }
            }

            if ((keyToken == NULL) || !kbParseKeyCodeString(keyToken, &binding.keycode))
            {
                continue;
            }
        }

        kbAddBinding(target, commandIndex, &binding);
    }

    fclose(file);
    if (outVersion != NULL)
    {
        *outVersion = version;
    }
    return TRUE;
}

static void kbWriteCurrentSettingsFile(void)
{
    FILE *file;
    sdword commandIndex;
    sdword bindingIndex;
    char bindingText[64];

    if (!kbCataclysmBindingsLoaded)
    {
        return;
    }

    file = fopen("keys.cfg", "wt");
    if (file == NULL)
    {
        return;
    }

    fprintf(file, "v %d\n", kbConfigVersion);
    fprintf(file, "; Generated Keys Configuration File - DO NOT EDIT!\n\n");

    for (commandIndex = 1; commandIndex <= kbCommandCount; commandIndex++)
    {
        for (bindingIndex = 0; bindingIndex < kbCommandCurrent[commandIndex].numBindings; bindingIndex++)
        {
            kbFormatSingleBinding((ubyte *)&kbCommandCurrent[commandIndex].bindings[bindingIndex], bindingText, FALSE);
            fprintf(file, "key  %s  %s\n", kbBinaryCommandDescriptors[commandIndex].configName, bindingText);
        }
    }

    fclose(file);
}

void kbLoadSettings(sdword reset)
{
    sdword loadedVersion;

    memset(kbCommandDefaults, 0, sizeof(kbCommandDefaults));
    memset(kbCommandCurrent, 0, sizeof(kbCommandCurrent));
    memset(kbCommandSaved, 0, sizeof(kbCommandSaved));
    kbConfigVersion = 0;
    kbCataclysmBindingsLoaded = FALSE;

    if (!kbLoadCommandFile("default_keys.cfg", kbCommandDefaults, &kbConfigVersion))
    {
        return;
    }

    memcpy(kbCommandCurrent, kbCommandDefaults, sizeof(kbCommandDefaults));
    kbCataclysmBindingsLoaded = TRUE;

    if (reset)
    {
        kbWriteCurrentSettingsFile();
        return;
    }

    memset(kbCommandCurrent, 0, sizeof(kbCommandCurrent));
    loadedVersion = 0;
    if (!kbLoadCommandFile("keys.cfg", kbCommandCurrent, &loadedVersion) || (loadedVersion != kbConfigVersion))
    {
        memcpy(kbCommandCurrent, kbCommandDefaults, sizeof(kbCommandDefaults));
        kbWriteCurrentSettingsFile();
        return;
    }
}

FILE *kbLoadSettingsNoReset(void)
{
    kbLoadSettings(FALSE);
    return NULL;
}

static bool32 kbIsFirstCommandInCategory(sdword commandIndex)
{
    sdword index;

    if ((commandIndex <= 0) || (commandIndex > kbCommandCount))
    {
        return FALSE;
    }

    for (index = 1; index < commandIndex; index++)
    {
        if (kbBinaryCommandDescriptors[index].category == kbBinaryCommandDescriptors[commandIndex].category)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static bool32 kbSelectionCommandsAllowed(void)
{
    return (!playPackets && !universePause && !mrDisabled);
}

static sdword kbApplySelectionTactics(TacticsType tactics)
{
    if (!kbSelectionCommandsAllowed())
    {
        return FALSE;
    }

    return aiuWrapSetTactics((SelectCommand *)&selSelected, tactics);
}

static sdword kbApplySelectionFormation(TypeOfFormation formation)
{
    return mrSetSelectionFormation(formation, TRUE, FALSE);
}

static TacticsType kbStepSelectionTactics(bool32 backward)
{
    TacticsType tactics;

    if (selSelected.numShips <= 0)
    {
        return Neutral;
    }

    tactics = selSelected.ShipPtr[0]->tacticstype;

    if (backward)
    {
        switch (tactics)
        {
            case Evasive:
                return Aggressive;
            case Neutral:
                return Evasive;
            case Aggressive:
            default:
                return Neutral;
        }
    }

    switch (tactics)
    {
        case Evasive:
            return Neutral;
        case Neutral:
            return Aggressive;
        case Aggressive:
        default:
            return Evasive;
    }
}






















void kbMoveSelection(void)
{
    mrMoveSelection();
}












sdword kbSetSelectionEvasiveTactics(void)
{
    return kbApplySelectionTactics(Evasive);
}


sdword kbSetSelectionNeutralTactics(void)
{
    return kbApplySelectionTactics(Neutral);
}


sdword kbSetSelectionAggressiveTactics(void)
{
    return kbApplySelectionTactics(Aggressive);
}


sdword kbCycleSelectionTacticsForward(void)
{
    return kbApplySelectionTactics(kbStepSelectionTactics(FALSE));
}


sdword kbCycleSelectionTacticsBackward(void)
{
    return kbApplySelectionTactics(kbStepSelectionTactics(TRUE));
}


sdword kbSetSelectionDeltaFormation(void)
{
    return kbApplySelectionFormation(DELTA_FORMATION);
}


sdword kbSetSelectionBroadFormation(void)
{
    return kbApplySelectionFormation(BROAD_FORMATION);
}


sdword kbSetSelectionDelta3DFormation(void)
{
    return kbApplySelectionFormation(DELTA3D_FORMATION);
}


sdword kbSetSelectionClawFormation(void)
{
    return kbApplySelectionFormation(CLAW_FORMATION);
}


sdword kbSetSelectionMilitaryParade(void)
{
    return mrSetSelectionFormation(SAME_FORMATION, TRUE, FALSE);
}




udword kbActivateSelectionSpecial(void)
{
    return mrActivateSelectionSpecial();
}


void *kbSelectMothership(void)
{
    if (!kbSelectionCommandsAllowed() || universe.curPlayerPtr->PlayerMothership == NULL)
    {
        return NULL;
    }

    selSelected.numShips = 1;
    selSelected.ShipPtr[0] = universe.curPlayerPtr->PlayerMothership;
    ioUpdateShipTotals();
    ccFocusOnMyMothership(&universe.mainCameraCommand);
    return universe.curPlayerPtr->PlayerMothership;
}

void kbClearSelectionPopupSuppressFlag(void)
{
    kbSuppressSpecialReleaseOverlay = 0;
}
