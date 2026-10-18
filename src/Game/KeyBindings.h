/*=============================================================================
    Name    : KeyBindings.h
    Purpose : This file contains all of the defines and headers for the Key
              bindings system.

    Created 8/04/1999 by Drew Dunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___KEYBINDINGS_H
#define ___KEYBINDINGS_H

#include "feflow.h"
#include "font.h"
#include "key.h"
#include "strings.h"

/*=============================================================================
    Defines :
=============================================================================*/
#define KB_MAX_COMMAND_BINDINGS   3
#define KB_BINARY_COMMAND_COUNT   150
#define KB_MAX_COMMAND_DEFS       (KB_BINARY_COMMAND_COUNT + 1)

#define KB_CMD_TOGGLE_SHIP_INFO          0x6002
#define KB_CMD_NEXT_OVERLAY_VIEW         0x6003
#define KB_CMD_PREV_OVERLAY_VIEW         0x6004
#define KB_CMD_TOGGLE_OVERLAY_OPTIONS    0x6005
#define KB_CMD_TOGGLE_OVERLAY_BACKDROPS  0x6006
#define KB_CMD_BEGIN_BIG_GUN_TARGET      0x6007


/*=============================================================================
    Structure Prototypes :
=============================================================================*/

typedef enum
{
    kbNEXT_FORMATION=0,
    kbBUILD_MANAGER,
    kbPREVIOUS_FOCUS,
    kbNEXT_FOCUS,
    kbDOCK,
    kbSELECT_ALL_VISIBLE,
    kbFOCUS,
    kbRESEARCH_MANAGER,
    kbHARVEST,
    kbMOVE,
    kbNEXT_TACTIC,
    kbPREVIOUS_TACTIC,
    kbSCUTTLE,
    kbSHIP_SPECIAL,
    kbTACTICAL_OVERLAY,
//    kbSENSORS_MANAGER,
    kbMOTHERSHIP,
    kbKAMIKAZE,
    kbCANCEL_ORDERS,
    kbLAUNCH_MANAGER,
    kbTOTAL_COMMANDS
} CommandsToBind;

#define KB_NormalBound      0
#define KB_NotBound         1
#define KB_GetKeyPress      2

typedef struct kbBoundKeys
{
    udword      command;
    udword      defaultkey;
    udword      primarykey;
    udword      resettokey;
    udword      status;
} kbBoundKeys;

typedef struct kbBinding
{
    ubyte keycode;
    ubyte pad[3];
    bool32 shift;
    bool32 ctrl;
    bool32 alt;
} kbBinding;

typedef struct kbCommandBindingList
{
    sdword numBindings;
    kbBinding bindings[KB_MAX_COMMAND_BINDINGS];
} kbCommandBindingList;

typedef void (*kbCommandCallback)(sdword keycode);

typedef struct kbCommandCallbackPair
{
    kbCommandCallback keyDown;
    kbCommandCallback keyUp;
} kbCommandCallbackPair;


/*=============================================================================
    External Variables :
=============================================================================*/

extern fonthandle kbKeyListFont;

extern udword kbKeySavedKeys[20];

/*=============================================================================
    Function Prototypes :
=============================================================================*/

void kbListWindowCB(char *string, featom *atom);
void kbKeyResetToDefault(char *string, featom *atom);

void kbPoolListWindowCB(char *string, featom *atom);

sdword kbCheckBindings(sdword keypressed);
bool32 kbInvokeBoundCommandPressCallbacks(sdword keycode);
bool32 kbInvokeBoundCommandReleaseCallbacks(sdword keycode);

void kbRestoreSavedSettings(void);
void kbSaveSettings(void);

udword kbActivateSelectionSpecial(void);
udword kbKeyBoundToCommand(udword command);
bool32 kbCommandKeyIsHit(udword command);

bool32 kbCommandIsHitByName(const char *configName);
int kbGetCommandsBoundToKey(ubyte keycode, int *commandsBound);
bool32 kbTryToggleSensorsManager(sdword keycode);
FILE *kbLoadSettingsNoReset(void);
void kbRegisterListFont(void);
void kbInitKeyBindings(void);

extern const kbCommandCallbackPair kbCommandCallbackTable[];
extern kbCommandBindingList kbCommandBindingTable[];

#endif
