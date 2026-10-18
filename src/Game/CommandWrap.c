/*=============================================================================
    Name    : CommandWrap.c
    Purpose : Wrapper functions for calling or sending over the network
              CommandLayer commands.

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "CommandLayer.h"
#include "CommandNetwork.h"
#include "CommandWrap.h"
#include "FeFlow.h"
#include "FontReg.h"
#include "universe.h"
#include "globals.h"
#include "mainswitches.h"
#include "SoundEvent.h"
#include "singleplayer.h"
#include "tutor.h"
#include "utility.h"
#include "multiplayergame.h"
#include "salcapcorvette.h"
#include "Ship.h"
#include "shipselect.h"
#include "select.h"
#include "BattleChat.h"
#include "InfoOverlay.h"
#include "StringsOnly.h"

// Decompiled from cataclysm.exe xrefs at 0x45A381/0x45A454/0x45A4AF/0x45B223:
// retail keeps this console-screen pause override byte zero-initialized until the
// console UI toggles it.
ubyte byte_A2DCD4 = 0;

#define CM_ConsoleOutputHistoryCount   28
#define CM_ConsoleInputHistoryCount    8
#define CM_ConsoleLineLength           77
#define CM_ConsoleScrollLimit          300
#define CM_ConsoleCursorBlinkPeriod    10

static char **cmConsoleOutputHistory = NULL;
static char **cmConsoleInputHistory = NULL;
static char *cmConsoleAutocompleteBackup = NULL;
static sdword cmConsoleScrollOffset = 0;
static sdword scrollY = 0;
static sdword cmConsoleCurrentInputLength = 1;
static sdword cmConsoleCurrentInputHistory = 0;
static sdword cmConsoleAutocompleteIndex = 0;
static sdword cmConsoleCursorBlinkTicks = CM_ConsoleCursorBlinkPeriod;
static sdword cmConsoleFont = 0;
static bool32 cmConsoleCursorVisible = TRUE;
static bool32 cmConsoleAutocompleteActive = FALSE;
static bool32 cmConsoleStarted = FALSE;
static regionhandle cmConsoleBaseRegion = NULL;
static fibfileheader *cmConsoleScreens = NULL;
static char *cmConsoleConfigBasePath = NULL;

static void cmConsoleDraw(featom *atom, regionhandle region);

static fedrawcallback cmConsoleDrawCallbacks[] =
{
    {cmConsoleDraw, "CON_TextDraw"},
    {NULL, NULL},
};

static void cmConsoleResetPrompt(char *line)
{
    line[0] = '>';
    line[1] = '\0';
}

static char *cmConsoleInitConfigBasePath(void)
{
    cmConsoleConfigBasePath = _strdup("cataclysm.exe");
    return cmConsoleConfigBasePath;
}

static void cmConsoleFreeConfigBasePath(void)
{
    if (cmConsoleConfigBasePath != NULL)
    {
        free(cmConsoleConfigBasePath);
        cmConsoleConfigBasePath = NULL;
    }
}

char *cmConsoleStartup(void)
{
    sdword i;

    cmConsoleScrollOffset = 0;
    scrollY = 0;

    cmConsoleOutputHistory = malloc(sizeof(char *) * CM_ConsoleOutputHistoryCount);
    cmConsoleInputHistory = malloc(sizeof(char *) * CM_ConsoleInputHistoryCount);

    for (i = 0; i < CM_ConsoleOutputHistoryCount; i++)
    {
        cmConsoleOutputHistory[i] = calloc(CM_ConsoleLineLength, 1);
    }

    for (i = 0; i < CM_ConsoleInputHistoryCount; i++)
    {
        cmConsoleInputHistory[i] = calloc(CM_ConsoleLineLength, 1);
        cmConsoleResetPrompt(cmConsoleInputHistory[i]);
    }

    cmConsoleAutocompleteBackup = calloc(CM_ConsoleLineLength, 1);
    cmConsoleResetPrompt(cmConsoleAutocompleteBackup);

    cmConsoleCurrentInputLength = 1;
    cmConsoleCurrentInputHistory = 0;
    cmConsoleFont = frFontRegister("Arial_12.hff");

    if (cmConsoleScreens == NULL)
    {
        feDrawCallbackAddMultiple(cmConsoleDrawCallbacks);
        cmConsoleScreens = feScreensLoad("FEMan\\console.fib");
    }

    byte_A2DCD4 = 0;
    cmConsoleAutocompleteActive = FALSE;
    cmConsoleScrollOffset = 0;
    scrollY = 0;
    cmConsoleCursorBlinkTicks = CM_ConsoleCursorBlinkPeriod;
    cmConsoleCursorVisible = TRUE;
    cmConsoleCurrentInputHistory = 0;
    cmConsoleAutocompleteIndex = 0;
    cmConsoleStarted = TRUE;

    return cmConsoleInitConfigBasePath();
}

sdword cmConsoleShutdown(void)
{
    sdword i;
    sdword result = 0;

    if (!cmConsoleStarted)
    {
        return 0;
    }

    for (i = 0; i < CM_ConsoleOutputHistoryCount; i++)
    {
        free(cmConsoleOutputHistory[i]);
    }
    free(cmConsoleOutputHistory);
    cmConsoleOutputHistory = NULL;

    for (i = 0; i < CM_ConsoleInputHistoryCount; i++)
    {
        free(cmConsoleInputHistory[i]);
    }
    free(cmConsoleInputHistory);
    cmConsoleInputHistory = NULL;

    free(cmConsoleAutocompleteBackup);
    cmConsoleAutocompleteBackup = NULL;
    cmConsoleFreeConfigBasePath();

    if (cmConsoleBaseRegion != NULL)
    {
        feScreenDeleteFlags(cmConsoleBaseRegion, FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND);
        cmConsoleBaseRegion = NULL;
        byte_A2DCD4 = 0;
    }

    cmConsoleStarted = FALSE;
    return result;
}

static void cmConsoleDraw(featom *atom, regionhandle region)
{
    sdword previousFont;
    sdword lineHeight;
    sdword bottom;
    sdword drawIndex;
    sdword y;
    char currentLine[80];

    atom = atom;

    cmConsoleScrollOffset += scrollY;
    if (cmConsoleScrollOffset < 0)
    {
        cmConsoleScrollOffset = 0;
        scrollY = 0;

        if (cmConsoleBaseRegion != NULL)
        {
            feScreenDeleteFlags(cmConsoleBaseRegion, FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND);
            cmConsoleBaseRegion = NULL;
            if (singlePlayerGame)
            {
                universePause = FALSE;
            }
            byte_A2DCD4 = 0;
        }
    }
    else if (cmConsoleScrollOffset > CM_ConsoleScrollLimit)
    {
        cmConsoleScrollOffset = CM_ConsoleScrollLimit;
        scrollY = 0;
    }

    if (scrollY != 0 && cmConsoleBaseRegion != NULL)
    {
        regRegionScroll(cmConsoleBaseRegion, 0, scrollY);
    }

    previousFont = fontMakeCurrent(cmConsoleFont);
    if (cmConsoleScrollOffset > 0)
    {
        lineHeight = fontHeight(NULL) + 2;
        bottom = region->rect.y1;

        for (drawIndex = CM_ConsoleOutputHistoryCount - 1; drawIndex >= 0; drawIndex--)
        {
            y = bottom - ((CM_ConsoleOutputHistoryCount - drawIndex + 1) * lineHeight) - 15;
            if (y >= 0)
            {
                fontPrintf(16, y + 1, 0xff000000, "%s", cmConsoleOutputHistory[drawIndex]);
                fontPrintf(15, y, 0xffffffff, "%s", cmConsoleOutputHistory[drawIndex]);
            }
        }

        if (cmConsoleCursorBlinkTicks <= 0 || --cmConsoleCursorBlinkTicks <= 0)
        {
            cmConsoleCursorBlinkTicks = CM_ConsoleCursorBlinkPeriod;
            cmConsoleCursorVisible = !cmConsoleCursorVisible;
        }

        y = bottom - fontHeight(NULL) - 17;
        if (cmConsoleCursorVisible)
        {
            strcpy(currentLine, cmConsoleInputHistory[cmConsoleCurrentInputHistory]);
            strcat(currentLine, "_");
            if (y >= 0)
            {
                fontPrintf(16, y + 1, 0xff000000, "%s", currentLine);
                fontPrintf(15, y, 0xffffffff, "%s", currentLine);
            }
        }
        else if (y >= 0)
        {
            fontPrintf(16, y + 1, 0xff000000, "%s", cmConsoleInputHistory[cmConsoleCurrentInputHistory]);
            fontPrintf(15, y, 0xffffffff, "%s", cmConsoleInputHistory[cmConsoleCurrentInputHistory]);
        }
    }

    fontMakeCurrent(previousFont);
}

/*-----------------------------------------------------------------------------
    Name        : clCommandMessage
    Description : Inserts a command message into the global gMessage variable
                  to be displayed on screen later
    Inputs      : String to be inserted into gMessage
    Outputs     : Modifies global gMessage variable
    Return      : void
----------------------------------------------------------------------------*/
bool32 clCommandMessage(char *CommandMessage, ...)
{
    va_list args;
    color messageColor;

    va_start(args, CommandMessage);
    messageColor = va_arg(args, color);
    va_end(args);

    return battleChatAddMessage(CommandMessage, messageColor, 0, 0) != 0;
}

typedef struct
{
    ShipType moduleType;
    udword mask;
} ShipModuleStateMask;

static const ShipModuleStateMask clShipModuleFromMasks[] =
{
    { sMothershipSpecial,    0x001 },
    { sMothershipArmour,     0x002 },
    { sMothershipMicro,      0x004 },
    { sMothershipWeapons,    0x008 },
    { sMothershipDockingBay, 0x010 },
    { sMothershipSupport,    0x020 },
    { sMothershipBigGun,     0x040 },
};

static const ShipModuleStateMask clShipModuleToMasks[] =
{
    { 0, 0x080 },
    { 1, 0x100 },
    { 2, 0x200 },
};

static udword clShipModuleStateEncodeMask(const ShipModuleStateMask *table, sdword count, sdword value)
{
    sdword i;

    for (i = 0; i < count; i++)
    {
        if ((sdword)table[i].moduleType == value)
        {
            return table[i].mask;
        }
    }

    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : clFlashShips
    Description : Sets all ships in selection to flash
    Inputs      : selection - the selection to flash
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void clFlashShips(SelectCommand *selection)
{
    udword i;

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];
        const SelectCommand *attachedSelection;
        udword attachedIndex;
        ship->flashtimer = taskTimeElapsed;

        if (ship->objtype != OBJ_ShipType)
        {
            continue;
        }

        if (ship->clampInfo == NULL || ship->clampInfo->selection == NULL)
        {
            continue;
        }

        attachedSelection = (const SelectCommand *)ship->clampInfo->selection;
        for (attachedIndex = 0; attachedIndex < attachedSelection->numShips; attachedIndex++)
        {
            attachedSelection->ShipPtr[attachedIndex]->flashtimer = taskTimeElapsed;
        }
    }
}

void SpeechEventsForFuel(SelectCommand *selectcom)
{
    sdword i;
    for(i=0;i<selectcom->numShips;i++)
    {
        if(selectcom->ShipPtr[0]->fuel == 0.0f)
        {
            /////////////////////
            //fuel speech event....
            //event num: COMM_Strike_OutOfFuelMove
            //battle chatter


            //////////////////
            return;
        }
    }


}

void clWrapMove(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;


    makeShipsControllable(selectcom,COMMAND_MOVE);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Move Command Received");
#endif

    }

    // ships selected to move flash for a few frames to indicate the selection
    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clMove(comlayer,selectcom,packedMove);
    }
    else
    {
        clSendMove(comlayer,selectcom,packedMove);
    }
}

void clWrapMovePoint(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    clWrapMove(comlayer, selectcom, clBuildMovePointCommand(from, &to));
}

void clWrapMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    udword i;

    if (playPackets | (universePause && !byte_A2DCD4)) return;

    //makeShipsControllable(selectcom,COMMAND_MP_HYPERSPACEING);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Hyperspace Command Received");
#endif
    }

    // make the ships unselectable

    // ships selected to move flash for a few frames to indicate the selection
    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clMpHyperspace(comlayer,selectcom,from,to);
    }
    else
    {
        clSendMpHyperspace(comlayer,selectcom,from,to);
    }
    for(i=0;i<selectcom->numShips;)
    {
        bitClear(selectcom->ShipPtr[i]->flags,SOF_Selectable);
        if(clRemoveShipFromSelection(&selSelected, selectcom->ShipPtr[i]))
        {
            ioUpdateShipTotals();
            continue;
        }
        i++;
    }
}


void clWrapAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    bool32 pausedWithoutOverride;

    pausedWithoutOverride = universePause && !byte_A2DCD4;
    if (playPackets || pausedWithoutOverride) return;

    makeShipsControllable(selectcom,COMMAND_ATTACK);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips((SelectCommand *)attackcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Attack Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bAttack)
            return;

        clAttack(comlayer,selectcom,attackcom);
    }
    else
    {
        clSendAttack(comlayer,selectcom,attackcom);
    }
}

void clWrapFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!FORMATION_IS_VALID(formation))
    {
        dbgFatalf(DBG_Loc,"Invalid formation %d",formation);
    }

    if ((!universe.aiplayerProcessing) && (!((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)))
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Formation Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)
            return;

        clFormation(comlayer,selectcom,formation);
    }
    else
    {
        clSendFormation(comlayer,selectcom,formation);
    }
}

void clWrapDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    //Although research ships have docking capabilities, they aren't allowed to be
    //ordered to dock:
    makeShipsControllable(selectcom,COMMAND_DOCK);
    if (selectcom->numShips == 0) return;

    if (dockType != DOCK_MODE_RETIRE)
    {
        ioUpdateShipTotals();
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Dock Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bDock)
            return;

        clDock(comlayer,selectcom,dockType,dockwith);
    }
    else
    {
        clSendDock(comlayer,selectcom,dockType,dockwith);
    }

    if (!universe.aiplayerProcessing)
    {
        Ship *ship = selectcom->ShipPtr[0];

        if (ship->dockvars.dockship != NULL)
        {
            if (dockType == DOCK_FOR_RETIRE)
            {
                speechEventQueue(NULL, 67113066, -1, -1, 0, ship->playerowner->playerIndex, -1, 5.0f, -1);
            }
            else if (dockType == DOCK_MODE_BEAST)
            {
                speechEventQueue(ship, 0x04000801, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);
            }
            else
            {
                speechEventQueue(ship, 0x04000803, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);
            }
        }
    }
}

sdword clWrapUpgrade(CommandLayer *comlayer,SelectCommand *selectcom,sdword upgradecom)
{
    if (playPackets)
    {
        return playPackets;
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        clSendUpgrade(comlayer, selectcom, upgradecom);
        return 0;
    }

    return clUpgrade(comlayer, selectcom, upgradecom);
}

void clWrapDeterministicBuild(udword command,
                              CommandLayer* comlayer, sdword numShips,
                              ShipType shipType, ShipRace shipRace,
                              uword playerIndex, ShipPtr creator)
{
    if (playPackets) return;

    if (!universe.aiplayerProcessing)
    {
#if CL_TEXTFEEDBACK
        //@todo verbosify this lame message
        if (enableTextFeedback) clCommandMessage("New Build Command");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clDeterministicBuild(command, comlayer, numShips, shipType, shipRace, playerIndex, creator);
    }
    else
    {
        clSendDeterministicBuild(command, comlayer, numShips, shipType, shipRace, playerIndex, creator);
    }
}

Ship *clWrapCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    if (playPackets)
    {
        return NULL;
    }

    if (!universe.aiplayerProcessing)
    {
#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("New Ship Created");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        return clCreateShip(comlayer,shipType,shipRace,playerIndex,creator);
    }
    else
    {
        clSendCreateShip(comlayer,shipType,shipRace,playerIndex,creator);
        return NULL;
    }
}

void clWrapBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    if (playPackets) return;

    if (!universe.aiplayerProcessing)
    {
#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("New Ship Being Built");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clBuildShip(comlayer,shipType,shipRace,playerIndex,creator);
    }
    else
    {
        clSendBuildShip(comlayer,shipType,shipRace,playerIndex,creator);
    }

}

void clWrapCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource)
{
    bool32 pausedWithoutOverride;

    pausedWithoutOverride = universePause && !byte_A2DCD4;
    if (playPackets || pausedWithoutOverride) return;

    makeShipsControllable(selectcom,COMMAND_COLLECTRESOURCE);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

        if (resource != NULL)
        {
            resource->flashtimer = taskTimeElapsed;
        }
#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Harvest Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clCollectResource(comlayer,selectcom,resource);
    }
    else
    {
        clSendCollectResource(comlayer,selectcom,resource);
    }

}

void clWrapProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips((SelectCommand *)protectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Protect Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clProtect(comlayer,selectcom,protectcom);
    }
    else
    {
        clSendProtect(comlayer,selectcom,protectcom);
    }

}

void clWrapSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets)
{
    bool32 pausedWithoutOverride;
    sdword i;
    SelectCommand *copycom=NULL;
    sdword sizeofcopycom;

    pausedWithoutOverride = universePause && !byte_A2DCD4;
    if (playPackets || pausedWithoutOverride) return;

    makeShipsControllable(selectcom,COMMAND_SPECIAL);
    if (selectcom->numShips == 0) return;

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"clspecialfix", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    if (!universe.aiplayerProcessing)
    {
        //if the special operation has a target, flash the targets,
        //else flash the ships performing the operation
        if (targets)
        {
            clFlashShips((SelectCommand *)targets);
        }
        else
        {
            clFlashShips(selectcom);
        }

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Special Ability Activated");
#endif
    }

    if (selectcom->ShipPtr[0]->playerowner == universe.curPlayerPtr)
    {
        if (targets)
        {
            for (i = 0; i < selectcom->numShips; i++)
            {
                salCapCantDoFormation(selectcom->ShipPtr[i]);
            }
        }
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        if (copycom->numShips > 0)
        {
            clSendSpecial(comlayer,copycom,targets);
        }
    }
    else if ((tutorial != TUTORIAL_ONLY || tutEnable.bSpecial) && copycom->numShips > 0)
    {
        clSpecial(comlayer,copycom,targets);
    }
    memFree(copycom);
}

void clWrapSpecialCloak(CommandLayer *comlayer, SelectCommand *selectcom, AttackCommand *attackcom, bool32 cloak)
{
    if (playPackets | universePause)
    {
        return;
    }

    makeShipsControllable(selectcom, COMMAND_SPECIAL);
    if (selectcom->numShips == 0)
    {
        return;
    }

    if (!universe.aiplayerProcessing)
    {
        if (attackcom != NULL)
        {
            clFlashShips((SelectCommand *)attackcom);
        }
        else
        {
            clFlashShips(selectcom);
        }
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clCloak(comlayer, selectcom, attackcom, cloak);
    }
    else if (cloak)
    {
        clSendCloak(comlayer, selectcom, attackcom);
    }
    else
    {
        clSendDecloak(comlayer, selectcom, attackcom);
    }
}

void clWrapHalt(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    makeShipsControllable(selectcom,COMMAND_HALT);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Halt Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clHalt(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_HALT,0);
    }
}

void clWrapScuttle(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    makeShipsControllable(selectcom,COMMAND_HALT);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Scuttle Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clScuttle(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_SCUTTLE,0);
    }
}

void clWrapAutoLaunch(udword OnOff,udword playerIndex)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clAutoLaunch(OnOff,playerIndex);
    }
    else
    {
        clSendAutoLaunch(OnOff,playerIndex);
    }
}

void clWrapSetAlliance(udword AllianceType, uword playerone, uword playertwo)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetAlliance(AllianceType, playerone, playertwo);
    }
    else
    {
        clSendSetAlliance(AllianceType, playerone, playertwo);
    }

}

void clWrapLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clLaunchMultipleShips(comlayer,selectcom,launchFrom);
    }
    else
    {
        clSendLaunchMultipleShips(comlayer,selectcom,launchFrom);
    }
}

void clWrapSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Tactics Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetTactics(comlayer,selectcom,tacticstype);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_TACTICS,tacticstype);
    }
}

void clWrapRUTransfer(CommandLayer *comlayer,sdword fromIndex,sdword toIndex, sdword resourceUnits,ubyte flags)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        //needed for ResourceDeductions that are deterministic (building crud)
        clRUTransfer(comlayer,toIndex,fromIndex,resourceUnits,flags);
    }
    else
    {
        clSendRuTransfer(comlayer,fromIndex,toIndex,resourceUnits,flags);
    }

}

void clWrapSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Kamikaze Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetKamikaze(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_KAMIKAZE,0);  //0 = dummy num
    }
}

void clWrapSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets | (universePause && !byte_A2DCD4)) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Parade Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetMilitaryParade(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_PARADE,0);  //0 = dummy num
    }
}

void clWrapSiegeCannon(CommandLayer *comlayer,SelectCommand *selectcom,vector *target)
{
    if (!playPackets && selectcom->numShips)
    {
        if (!universe.aiplayerProcessing)
        {
            clFlashShips(selectcom);
        }
        if (multiPlayerGame || recordFakeSendPackets)
        {
            clSendSiegeCannon(comlayer,selectcom,target);
        }
        else
        {
            clSiegeCannon(comlayer,selectcom,target);
        }
    }
}

void clWrapSetBeastSystem(CommandLayer *comlayer, Ship *ship, udword system)
{
    SelectCommand selectcom;

    if (playPackets)
    {
        return;
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        selectcom.ShipPtr[0] = ship;
        selectcom.numShips = 1;
        clSendMisc(comlayer, &selectcom, MISCCOMMAND_SETBEASTSYSTEM, system);
    }
    else
    {
        clSetBeastSystem(comlayer, ship, system);
    }
}

void clWrapSetUnleech(CommandLayer *comlayer, SelectCommand *selectcom)
{
    bool32 pausedWithoutOverride;

    pausedWithoutOverride = universePause && !byte_A2DCD4;
    if ((playPackets || pausedWithoutOverride) || selectcom->numShips == 0)
    {
        return;
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        clSendMisc(comlayer, selectcom, MISCCOMMAND_SETUNLEECH, 0);
    }
    else
    {
        clSetUnleech(comlayer, selectcom);
    }
}

void clWrapSetLeech(CommandLayer *comlayer, SelectCommand *selectcom, ubyte leechMode)
{
    bool32 pausedWithoutOverride;

    pausedWithoutOverride = universePause && !byte_A2DCD4;
    if ((playPackets || pausedWithoutOverride) || selectcom->numShips == 0)
    {
        return;
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        clSendMisc(comlayer, selectcom, MISCCOMMAND_SETLEECH, leechMode);
    }
    else
    {
        clSetLeech(comlayer, selectcom, leechMode);
    }
}

// Cataclysm-specific: issue Acolyte link command for selected ships.
void clWrapLinkSelection(CommandLayer *comlayer, SelectCommand *selectcom)
{
    if (playPackets)
    {
        return;
    }
    if (selectcom->numShips == 0)
    {
        return;
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        clSendMisc(comlayer, selectcom, MISCCOMMAND_LINKSELECTION, 0);
    }
    else
    {
        clLinkSelection(comlayer, selectcom);
    }
}

void clWrapResearch(udword type, udword playernum, udword labnum, udword tech)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetResearch(type, playernum, labnum, tech);
    }
    else
    {
        clSendResearch(type, playernum, labnum, tech);
    }
}

void clWrapExternalConstruction(
    CommandLayer *comlayer,
    uword playerIndex,
    Ship *ship,
    void *context,
    real32 progress,
    sdword operation)
{
    if (playPackets)
    {
        return;
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        sendProcessUpdateExternalConstructionToDo(comlayer, playerIndex, ship, (Ship *)context, progress, operation);
    }
    else
    {
        processUpdateExternalConstructionToDo(comlayer, playerIndex, ship, (Ship *)context, progress, operation);
    }
}

CommandToDo *clWrapExternalConstructionSelection(
    CommandLayer *comlayer,
    SelectCommand *selectcom,
    sdword operation,
    sdword arg1,
    sdword arg2,
    sdword arg3)
{
    if (playPackets)
    {
        return NULL;
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        clSendExternalConstructionSelection(comlayer, selectcom, operation, arg1, arg2, arg3);
        return NULL;
    }

    return clExternalConstructionSelection(comlayer, selectcom, operation, arg1, arg2, arg3);
}

void clWrapSetShipDisabled(CommandLayer *comlayer, SelectCommand *selectcom)
{
    if (playPackets || selectcom == NULL || selectcom->numShips == 0)
    {
        return;
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        clSendMisc(comlayer, selectcom, MISCCOMMAND_SETSHIPDISABLED, 0);
    }
    else
    {
        clSetShipDisabled(comlayer, selectcom);
    }
}

void clWrapSetShipModuleState(CommandLayer *comlayer, SelectCommand *selectcom, ShipType moduleFrom, sdword moduleTo)
{
    udword miscData;

    if (playPackets || selectcom->numShips == 0)
    {
        return;
    }

    if (multiPlayerGame || recordFakeSendPackets)
    {
        miscData =
            clShipModuleStateEncodeMask(clShipModuleFromMasks, sizeof(clShipModuleFromMasks) / sizeof(clShipModuleFromMasks[0]), moduleFrom) |
            clShipModuleStateEncodeMask(clShipModuleToMasks, sizeof(clShipModuleToMasks) / sizeof(clShipModuleToMasks[0]), moduleTo);
        clSendMisc(comlayer, selectcom, MISCCOMMAND_SETSHIPMODULESTATE, miscData);
    }
    else
    {
        clSetShipModuleState(comlayer, selectcom, moduleFrom, moduleTo);
    }
}


#if 0  // Cat2: clWrapSetShipModuleState live code differs from binary
#if 0  // Cat2: clWrapSetShipModuleState live code differs from binary
/*=== clWrapSetShipModuleState (0x453580) ===*/
_DWORD *__cdecl sub_453580(int a1, int *a2, int a3, int a4)
{
  _DWORD *result; // eax
  int v5; // esi
  int v6; // eax

  result = (_DWORD *)playPackets;
  if ( !playPackets && *a2 )
  {
    if ( multiPlayerGame || recordFakeSendPackets )
    {
      v5 = sub_44E130(a3);
      v6 = sub_44E150(a4);
      return (_DWORD *)clSendMisc(a1, a2, 9, v6 | v5);
    }
    else
    {
      return clSetShipModuleState(a1, a2, a3, a4);
    }
  }
  return result;
}
#endif

#if 0  // Cat2: clWrapSetShipDisabled live code differs from binary
/*=== clWrapSetShipDisabled (0x453660) ===*/
int *__cdecl sub_453660(int a1, int *a2)
{
  int *result; // eax

  result = (int *)playPackets;
  if ( !playPackets )
  {
    result = a2;
    if ( *a2 )
    {
      if ( multiPlayerGame || recordFakeSendPackets )
        return (int *)clSendMisc(a1, a2, 11, 0);
      else
        return clSetShipDisabled(a1, a2);
    }
  }
  return result;
}
#endif
#endif

/*=== clWrapLaunchShipSelection (0x4539A0) ===*/
void __cdecl clWrapLaunchShipSelection(CommandLayer *comlayer, SelectCommand *selectcom)
{
    if (!playPackets && selectcom->numShips)
    {
        if (!universe.aiplayerProcessing)
            clFlashShips(selectcom);
        if (multiPlayerGame || recordFakeSendPackets)
            clSendMisc(comlayer, selectcom, 6u, 0);
        else
            clLaunchShipSelection(comlayer, selectcom);
    }
}
