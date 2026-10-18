/*=============================================================================
    Name    : mouse.c
    Purpose : Hardware abstraction for the mouse.

    Created 6/26/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "DDDFrigate.h"
#include "debug.h"
#include "dock.h"
#include "font.h"
#include "fontreg.h"
#include "glinc.h"
#include "globals.h"
#include "GravWellGenerator.h"
#include "main.h"
#include "mainrgn.h"
#include "mouse.h"
#include "nis.h"
#include "prim2d.h"
#include "select.h"
#include "sensors.h"
#include "shipselect.h"
#include "soundevent.h"
#include "spaceobj.h"
#include "texreg.h"
#include "tweak.h"
#include "types.h"
#include "universe.h"
#include "utility.h"
#include "CommandWrap.h"
#include "strings.h"
#include "demo.h"
#include "feflow.h"
#include "researchGUI.h"
#include "glcaps.h"
#include "render.h"

#include "Options.h"
#include "KeyBindings.h"
#include "InfoOverlay.h"
#include "devstats.h"
#include "singleplayer.h"
#include "tutor.h"


#define GLC_STORE 1

extern udword gDevcaps2;
extern sdword smBandboxEnabled;
extern bool32 playPackets;

udword ioConsumePendingOverlayClick(void);
signed int __cdecl kbKeyIsHit(int functionIndex);

/*=============================================================================
    Local Type Definitions:
=============================================================================*/
//flag settings
#define MCF_SpecialOp                   0x00000001  //has a special operation
#define MCF_SpecialOpDeployed           0x00000002  //has a special operation already deployed
#define MCF_Dockable                    0x00000004  //can dock (big ship)
#define MCF_SmallDockable               0x00000008  //can dock (small ship)
#define MCF_GUICapable                  0x00000010  //is associated with a GUI
#define MCF_NonCombat                   0x00000020  //ship is non combat ship
#define MCF_SupportOverItself           0x00000040  //the mouse is over a selected repair corvette
#define MCF_SingleClickSpecialActivate  0x00000080  //selection includes 1 or more single-click special activate ships
#define MCF_DisableAll                  0x00000100  //disable all cursor bitmaps
#define MCF_DisableMovementMech         0x00000200  //disable cursors when movement pie plate is activated
#define MCF_ShipSelecting               0x00001000  //ships are being selected
#define MCF_BuildGUI                    0x00002000  //ship opens the build manager
#define MCF_SensorsGUI                  0x00004000  //ship opens the sensors manager
#define MCF_ResearchGUI                 0x00008000  //ship opens the research manager
#define MCF_SpecialAttack               0x00010000  //ship has a special attack
#define MCF_SpecialSCSupport            0x00020000  //ship has a special support function for strikecraft
#define MCF_SpecialSupport              0x00040000  //ship has a special support function
#define MCF_SalvageOnly                 0x00080000  //shiplist only contains salvage ships
#define MCF_Salvage                     0x00100000  //shiplist contains at least one salvage ship

typedef struct
{
    udword ship_selected;
    udword resource;
    udword flags;
    mouseCursor cursorover;
} mouseInfoType;



/*=============================================================================
    Data:
=============================================================================*/
//we don't want the player to be able to salvage cryotrays in mission1
//so we need to include singlePlayerGameInfo
extern SinglePlayerGameInfo singlePlayerGameInfo;
extern sdword gameStatsOverlayEnabled;

sdword mouseCursorXPosition = 0, mouseCursorYPosition = 0;
sdword screenX = 0, screenY = 0;
uword mouseButtons;
udword mouseModuleInit;
rectangle mouseClipRect;
bool8 mouseClip = FALSE;
bool8 mouseIsVisible = TRUE;
uword mouseWillBeVisible = 0;
bool8 mouseDisabled = FALSE;
uword mouseInsideClient = FALSE;

static bool32 mouseGLInitialized;

sdword       mouseCursorTypeRaw = 0;
udword       mouseCursorMode = 0;
MaxSelection mouseCursorSelect;
real32       mouseDoubleClickTime   = 0.0;
real32       mouseDoubleClickExpire = REALlyBig;

real32      mouseCursorOverDistance = REALlyBig;
sdword      mouseCursorOverShipType = MC_NO_SHIP;
sdword      mouseCursorEnemyType = -1;
sdword      mouseCursorEnemyPlayer = 1;
real32      mouseCursorTextExpire   = 0.0f;
SpaceObj    *mouseCursorObjPtr      = NULL;
SpaceObj    *mouseCursorObjLast     = NULL;
real32      mouseCursorLastObjTime  = REALlyNegative;
sdword      mouseCursorLastX        = SDWORD_Max;
sdword      mouseCursorLastY        = SDWORD_Max;
ubyte       mouseCursorOverLODs     = 0;

//cursor colors
static color mouseCursorContentColor = MC_ContentColor;
static color mouseCursorEdgeColor    = MC_EdgeColor;

//cursor font
fonthandle mouseCursorFont = 0;

// cursor hold state (type 9 special hold-on)
static sdword  mouseCursorHoldType   = 0;
static real32  mouseCursorHoldExpire = 0.0f;

// Legacy flags struct for mouseLDoubleClick; always zero in retail cursor architecture
static mouseInfoType mouseInfo = {0, 0, 0, 0};

// cursor bitmaps and GL texture handles (array indexed by load order)
static lifheader *mouseCursorBitmaps[31];
static GLuint     mouseCursorTextures[31];

/*=============================================================================
    Functions:
=============================================================================*/
// ============================================================================
// Utility functions
// ============================================================================
/*-----------------------------------------------------------------------------
    Name        : mouseMirrorBitmap
    Description : Flips a bitmap 180 degrees
    Inputs      : bitmap - pointer to the bitmap data
                  width - width of the bitmap
                  height - height of the bitmap
    Outputs     : replaces "bitmap" with a bitmap flipped 90 degrees clockwise
    Return      :
----------------------------------------------------------------------------*/
#define PIXEL(bm, x, y, offset, width) bm[((x) + (y)*(width))+(offset)]

void mouseMirrorBitmap(udword *bitmap, sdword width, sdword height)
{
    udword i, j;
    udword new_bitmap[1024];

    for (i = 0; i < (udword)width; i++)
    {
        for (j = 0; j < (udword)height; j++)
        {
            PIXEL(new_bitmap, i, (height - 1) - j, 0, width) = PIXEL(bitmap, i, j, 0, width);
        }
    }

    memcpy(bitmap,new_bitmap,sizeof(udword)*width*height);
}
#undef PIXEL

/*-----------------------------------------------------------------------------
    Name        : mouseCreateGLHandle
    Description : creates a GL texture object for a given liff
    Inputs      : lif - the lifheader structure of the bitmap
                  thandle - target for the texture name
    Outputs     : thandle contains the texture name
    Return      :
----------------------------------------------------------------------------*/
void mouseCreateGLHandle(lifheader* lif, GLuint* thandle)
{
    sdword y;
    ubyte* data;
    ubyte* sp;
    ubyte* dp;

    data = (ubyte*)memAlloc(4 * lif->width * lif->height, "temp mouse texture", 0);

    for (y = 0; y < lif->height; y++)
    {
        dp = data + 4 * lif->width * (lif->height - y - 1);
        sp = lif->data + 4 * lif->width * y;
        memcpy(dp, sp, 4 * lif->width);
    }

    if (*thandle != 0)
    {
        glDeleteTextures(1, thandle);
    }

    trClearCurrent();

    glGenTextures(1, thandle);
    glBindTexture(GL_TEXTURE_2D, *thandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lif->width, lif->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    memFree(data);
}

// ============================================================================
// Mouse functions
// ============================================================================
/*-----------------------------------------------------------------------------
    Name        : mouseStartup
    Description : Start the mouse module
    Inputs      : void
    Outputs     : sets the mouse position by getting information from the
                    Windows mouse system.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword mouseStartup(void)
{
    RECT clientRect, windowRect;
    POINT mousePoint;

    GetWindowRect(ghMainWindow, &windowRect);
    GetClientRect(ghMainWindow, &clientRect);               //get location of window
    GetCursorPos(&mousePoint);                              //get location of Windows mouse cursor
    mouseCursorXPosition = mousePoint.x - windowRect.left;  //compute relative coords
    mouseCursorYPosition = mousePoint.y - windowRect.top;
    screenX = mouseCursorXPosition;
    screenY = mouseCursorYPosition;

    mouseCursorBitmaps[ 0] = trLIFFileLoad(MC_NORMAL_BM, NonVolatile);               // MCT_NORMAL (0), MCT_MOVEMENT (5)
    mouseCursorBitmaps[ 1] = trLIFFileLoad(MC_ADD_SHIPS_BM, NonVolatile);             // MCT_ADD_SHIPS (3)
    mouseCursorBitmaps[ 2] = trLIFFileLoad(MC_FOCUS_NO_SELECT_BM, NonVolatile);       // MCT_FOCUS (4)
    mouseCursorBitmaps[ 3] = trLIFFileLoad(MC_BAND_BOX_ATTACK_BM, NonVolatile);       // MCT_ATTACK (6)
    mouseCursorBitmaps[ 4] = trLIFFileLoad(MC_FORCE_ATTACK_BM, NonVolatile);          // MCT_FORCE_ATTACK (7)
    mouseCursorBitmaps[ 5] = trLIFFileLoad(MC_GUARD_BM, NonVolatile);                 // MCT_GUARD (8)
    mouseCursorBitmaps[ 6] = trLIFFileLoad(MC_SPECIAL_OP_ACTIVATE_BM, NonVolatile);   // MCT_SPECIAL_ACTIVATE (9)
    mouseCursorBitmaps[ 7] = trLIFFileLoad(MC_SPECIAL_OP_ACTIVATE_BM, NonVolatile);   // MCT_SPECIAL_ACTIVATE_HOLD (10)
    mouseCursorBitmaps[ 8] = trLIFFileLoad(MC_SPECIAL_OP_DEACTIVATE_BM, NonVolatile); // MCT_SPECIAL_DEACTIVATE (11)
    mouseCursorBitmaps[ 9] = trLIFFileLoad(MC_GUI_BM, NonVolatile);                   // MCT_DIALOG (12)
    mouseCursorBitmaps[10] = trLIFFileLoad(MC_RESOURCE_BM, NonVolatile);              // MCT_RESOURCE (13)
    mouseCursorBitmaps[11] = trLIFFileLoad(MC_DOCKING_BM, NonVolatile);               // MCT_DOCK (15), MCT_DOCK_SMALL (16)
    mouseCursorBitmaps[12] = trLIFFileLoad(MC_GUI_BM, NonVolatile);                   // MCT_RESEARCH (17)
    mouseCursorBitmaps[13] = trLIFFileLoad(MC_GUI_BM, NonVolatile);                   // MCT_SENSORS (18)
    mouseCursorBitmaps[14] = trLIFFileLoad(MC_GUI_BM, NonVolatile);                   // MCT_BUILD (19)
    mouseCursorBitmaps[15] = trLIFFileLoad(MC_SALVAGE_BM, NonVolatile);               // MCT_SALVAGE (20)
    mouseCursorBitmaps[16] = trLIFFileLoad(MC_SUPPORT_BM, NonVolatile);               // MCT_SUPPORT (21)
    mouseCursorBitmaps[17] = trLIFFileLoad(MC_FORCE_FIELD_BM, NonVolatile);           // MCT_FORCE_FIELD (22)
    mouseCursorBitmaps[18] = trLIFFileLoad(MC_TRADERS_BM, NonVolatile);               // MCT_TRADER (23)
    mouseCursorBitmaps[19] = trLIFFileLoad(MC_SLIPGATE_SMALL_BM, NonVolatile);        // MCT_SLIPGATE_SMALL (27)
    mouseCursorBitmaps[20] = trLIFFileLoad(MC_SLIPGATE_MEDIUM_BM, NonVolatile);       // MCT_SLIPGATE_MEDIUM (28)
    mouseCursorBitmaps[21] = trLIFFileLoad(MC_SLIPGATE_LARGE_BM, NonVolatile);        // MCT_SLIPGATE_LARGE (29)
    mouseCursorBitmaps[22] = trLIFFileLoad(MC_LEECH_BM, NonVolatile);                 // MCT_LEECH (30)
    mouseCursorBitmaps[23] = trLIFFileLoad(MC_LINK_BM, NonVolatile);                  // MCT_LINK (31)
    mouseCursorBitmaps[24] = trLIFFileLoad(MC_MIMIC_BM, NonVolatile);                 // MCT_MIMIC (32)
    mouseCursorBitmaps[25] = trLIFFileLoad(MC_EMP_BM, NonVolatile);                   // MCT_EMP (33)
    mouseCursorBitmaps[26] = trLIFFileLoad(MC_SWARM_BM, NonVolatile);                 // MCT_SWARM (34)
    mouseCursorBitmaps[27] = trLIFFileLoad(MC_FIRE_MISSILE_BM, NonVolatile);          // MCT_FIRE_MISSILE (35)
    mouseCursorBitmaps[28] = trLIFFileLoad(MC_RAMMING_BM, NonVolatile);               // MCT_RAMMING (36)
    mouseCursorBitmaps[29] = trLIFFileLoad(MC_BEAST_WEAPON_BM, NonVolatile);          // MCT_BEAST_WEAPON (37)
    mouseCursorBitmaps[30] = trLIFFileLoad(MC_OBJECTIVES_BM, NonVolatile);            // MCT_OBJECTIVES (38)

    for (sdword i = 0; i < 31; i++)
    {
        mouseMirrorBitmap((udword *)mouseCursorBitmaps[i]->data,
                          mouseCursorBitmaps[i]->width,
                          mouseCursorBitmaps[i]->height);
    }

    mouseDoubleClickTime = GetDoubleClickTime() / 1000.0f;
    mouseCursorFont      = frFontRegister(TW_CURSORTEXT_FONT);

    mouseGLInitialized = FALSE;

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : mouseInitGL
    Description : initialize the GL texture handles for mouse bitmaps
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mouseInitGL(void)
{
    for (sdword i = 0; i < 31; i++)
    {
        mouseCreateGLHandle(mouseCursorBitmaps[i], &mouseCursorTextures[i]);
    }
}
void mouseReset(void)
{
    if (mouseGLInitialized)
    {
        glDeleteTextures(31, mouseCursorTextures);
        memset(mouseCursorTextures, 0, sizeof(mouseCursorTextures));
        mouseGLInitialized = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : mouseShutdown
    Description : Close mouse module
    Inputs      : void
    Outputs     : not much
    Return      : void
----------------------------------------------------------------------------*/
void mouseShutdown(void)
{
    mouseReset();
}

/*-----------------------------------------------------------------------------
    Name        : mouseCursorHide
    Description : Show the cursor if it is hidden
    Inputs      : void
    Outputs     : sets the visibility flag to TRUE.  Subsequent mouse
                    mouseDraw() calls will not display a visible cursor.
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorHide(void)
{
    mouseIsVisible = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : mouseCursorShow
    Description : Show the cursor if it is hidden
    Inputs      : void
    Outputs     : sets the visibility flag to TRUE.  Subsequent mouse
                    mouseDraw() calls will not display a visible cursor.
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorShow(void)
{
    mouseIsVisible = TRUE;
}

sdword mouseCursorTypeGet(void)
{
    return gameStatsOverlayEnabled;
}

void mouseCursorDelayShow(uword delay)
{
    mouseWillBeVisible = delay+1;
}

/*-----------------------------------------------------------------------------
    Name        : mousePositionSet
    Description : Set new mouse position.
    Inputs      : x, y - new location of mouse.  May be clamped to the edge of
                    the window.
    Outputs     : Sets the Windows cursor location and the internal cursor location.
    Return      : void
----------------------------------------------------------------------------*/
static void mousePositionStore(sdword *x, sdword *y, rectangle *clientRect)
{
    utyClientRectGet(clientRect);                          //get window location
//  GetWindowRect(ghMainWindow, &windowRect);
//  GetClientRect(ghMainWindow, &clientRect);               //get location of window
#if MOUSE_CURSOR_CLAMP
    //clamp specified mouse location
    if (*x < 0)
    {
        *x = 0;
    }
    if (*y < 0)
    {
        *y = 0;
    }
    if (*x >= clientRect->x1 - clientRect->x0)
    {
        *x = clientRect->x1 - clientRect->x0 - 1;
    }
    if (*y >= clientRect->y1 - clientRect->y0)
    {
        *y = clientRect->y1 - clientRect->y0 - 1;
    }
#endif
    mouseCursorXPosition = *x;                               //set internal mouse location
    mouseCursorYPosition = *y;
    screenX = *x;
    screenY = *y;
}

void mousePositionSetGameOnly(sdword x, sdword y)
{
    rectangle clientRect;

    mousePositionStore(&x, &y, &clientRect);
}

void mousePositionSet(sdword x, sdword y)
{
    rectangle clientRect;
//    RECT clientRect, windowRect;
    POINT mousePoint;

    mousePositionStore(&x, &y, &clientRect);
    if (demDemoPlaying && (demPlaybackModeFlags & DEM_ModeB))
    {
        return;
    }
    mousePoint.x = clientRect.x0 + x;                       //compute Windows mouse location
    mousePoint.y = clientRect.y0 + y;
    SetCursorPos(mousePoint.x, mousePoint.y);               //set Windows mouse location
}

/*-----------------------------------------------------------------------------
    Name        : mouseClipToRect
    Description : Clips the mouse to a given rectangle.
    Inputs      : rect - rectangle to clip to.  NULL means no rectangle.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void mouseClipToRect(rectangle *rect)
{
    if (rect == NULL)
    {
        mouseClip = FALSE;
    }
    else
    {
        mouseClip = TRUE;
        mouseClipRect = *rect;
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseLDoubleClick
    Description : Processes a double click on the left mouse button
    Inputs      : void
    Outputs     : Very specific depending on the state of mouseInfo
    Return      : TRUE if some processing has been done (overrides the processing
                  for a single left click)
                  FALSE if no processing was done (processing for the single
                  left click continues)
----------------------------------------------------------------------------*/
bool32 mouseLDoubleClick(void)
{
    MaxSelection tempSelection;

    //if a menu is up, don't do any of this
//    if (feStackIndex > 0)
//    {
//        if (rmGUIActive)
//        {
//            return TRUE;
//        }
//        else
//        {
//            return FALSE;
//        }
//    }

//**** big chunks taken out because this function is being called
//     from mainrgn now.

//    if (bitTest(mouseCursorMode, MCM_SingleClick))
//    {
//        if (bitTest(mouseCursorMode, MCM_DoubleClick))
//        {
//            bitClear(mouseCursorMode, MCM_DoubleClick);
//            bitClear(mouseCursorMode, MCM_SingleClick);
//            mouseCursorSelect.numShips = 0;
//        }
//        else
//        {
//            bitSet(mouseCursorMode, MCM_DoubleClick);
//            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
//            return FALSE;
//        }
//    }

    if (selSelected.numShips)
    {
#if !MR_GUI_SINGLECLICK
//        if (bitTest(mouseInfo.flags, MCF_BuildGUI))
//        {
//            tutGameMessage("Game_DoubleClickBuild");
//            cmConstructionBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
//            return TRUE;
//        }
//        else if (bitTest(mouseInfo.flags, MCF_SensorsGUI))
//        {
//            tutGameMessage("Game_DoubleClickSensors");
//            smSensorsBegin(NULL, NULL);
//            return TRUE;
//        }
//        else if (bitTest(mouseInfo.flags, MCF_ResearchGUI))
//        {
//            tutGameMessage("Game_DoubleClickResearch");
//            rmResearchGUIBegin(ghMainRegion,(sdword)selSelected.ShipPtr[0], 0, 0);
//        }
//        else
#endif !MR_GUI_SINGLECLICK
//        if (ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, RepairCorvette) ||
//            ((ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, AdvanceSupportFrigate) ||
//              ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, Carrier)) &&
//             (mouseCursorObjPtr &&
//              (isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Fighter) ||
//               isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Corvette)))))

        if (mouseCursorObjPtr &&                // if there's an object being clicked on
            ((isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Fighter) ||        // Carriers only support strikecraft
              isShipOfClass((Ship *)mouseCursorObjPtr, CLASS_Corvette)) &&
             (ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, sCarrier) ||
              ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, bCarrier) ||
              ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, kCarrier) ||
              ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, tCarrier))) ||
            (ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, sRepairDroid) ||
             ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, bRepairDroid) ||
             ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, kAdvanceSupportFrigate) ||
             ShiptypeInSelection((SelectCommand *)&mouseCursorSelect, tAdvanceSupportFrigate)))
        {
            // make a backup of mouseCursorSelect
            tempSelection.numShips = 0;
            selSelectionCopy((MaxAnySelection *)&tempSelection, (MaxAnySelection *)&mouseCursorSelect);

            MakeShipsSpecialTargetCapable((SelectCommand *)&mouseCursorSelect,TRUE);
            MakeShipsNotIncludeTheseShips((SelectCommand *)&mouseCursorSelect, (SelectCommand *)&selSelected);

            if (mouseCursorSelect.numShips > 0)
            {
                SelectCommand sel;

                selSelectionCopy((MaxAnySelection *)&selSelected,
                                 (MaxAnySelection *)&mouseCursorSelect);

				ioUpdateShipTotals();

                if (mouseCursorSelect.numShips < tempSelection.numShips)
                {
                    // the selection being activated has other ships as well
                    tutGameMessage("Game_DoubleClickSpecial");
                    sel.numShips = 1;
                    sel.ShipPtr[0] = (Ship *)mouseCursorObjPtr;
                    clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&mouseCursorSelect, (SpecialCommand *)&sel);
                    selSelectionCopy((MaxAnySelection *)&mouseCursorSelect,
                                     (MaxAnySelection *)&tempSelection);
                    return FALSE; //we may not be done...
                }
                else
                {
                    // the selection being activated only contains support ships
                    mouseCursorSelect.numShips = 0;
                    tutGameMessage("Game_DoubleClickSpecial");
                    sel.numShips   = 1;
                    sel.ShipPtr[0] = (Ship *)mouseCursorObjPtr;
                    clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selSelected,(SpecialCommand *)&sel);
                    return TRUE;
                }
            }
            bitClear(mouseCursorMode, MCM_DoubleClick);
            bitClear(mouseCursorMode, MCM_SingleClick);
        }
        if (bitTest(mouseInfo.flags, MCF_SpecialOp))
        {
            MakeShipsSpecialActivateCapable((SelectCommand *)&mouseCursorSelect, FALSE);

            if (mouseCursorSelect.numShips > 0)
            {
                selSelectionCopy((MaxAnySelection *)&selSelected,
                                 (MaxAnySelection *)&mouseCursorSelect);
                ioUpdateShipTotals();
                mouseCursorSelect.numShips = 0;
                tutGameMessage("Game_DoubleClickSpecial");
                clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selSelected,NULL);
                return TRUE;
            }
            bitClear(mouseCursorMode, MCM_DoubleClick);
            bitClear(mouseCursorMode, MCM_SingleClick);

        }
    }
//    else if ((mouseCursorObjPtr != NULL) &&
//             (mouseCursorObjPtr->objtype != OBJ_ShipType))
//    {
//        if (mouseCursorSelect.numShips > 0)
//        {
//            selSelectionCopy((MaxAnySelection *)&selSelected,
//                             (MaxAnySelection *)&mouseCursorSelect);
//            mouseCursorSelect.numShips = 0;
//            tutGameMessage("Game_DoubleClickHarvest");
//            clWrapCollectResource(&universe.mainCommandLayer, (SelectCommand *)&selSelected, (Resource*)mouseCursorObjPtr);
//        }
//        return TRUE;
//    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : mouseDoubleClickCheck
    Description : Check to see if the double click time has expired and performs
                  special tasks if it has
    Inputs      : void
    Outputs     : Very specific depending on the state of mouseInfo
    Return      : TRUE if processing was done, FALSE if not
----------------------------------------------------------------------------*/
bool32 mouseDoubleClickCheck(void)
{
    if (mouseDoubleClickExpire == 0.0)
    {
        return FALSE;
    }

    if (mouseDoubleClickExpire < universe.totaltimeelapsed)
    {
        if (bitTest(mouseCursorMode, MCM_DoubleClick))
        {
            bitClear(mouseCursorMode, MCM_DoubleClick);
            bitClear(mouseCursorMode, MCM_SingleClick);
            mouseDoubleClickExpire = 0.0;

            if (mouseCursorObjPtr != NULL)
            {
                if (mouseCursorSelect.numShips > 0)
                {
                    selSelectionCopy((MaxAnySelection *)&selSelected,
                                     (MaxAnySelection *)&mouseCursorSelect);
                    mouseCursorSelect.numShips = 0;
                    makeShipsDockCapable((SelectCommand *)&selSelected, TRUE); //filter ships that can't dock
                    clWrapDock(&universe.mainCommandLayer, (SelectCommand *)&selSelected,
                               DOCK_AT_SPECIFIC_SHIP, (Ship *)mouseCursorObjPtr);
                    mouseCursorTypeRaw = MCT_DOCK_SMALL;
                }
                return TRUE;
            }
        }
        mouseCursorSelect.numShips = 0;
        mouseDoubleClickExpire     = 0.0;
        bitClear(mouseCursorMode, MCM_SingleClick);
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : mouseLClick
    Description : Special processing a single click on the left mouse button
    Inputs      : void
    Outputs     : Very specific depending on the state of mouseInfo
    Return      : void
----------------------------------------------------------------------------*/
void mouseLClick(void)
{
    //if a menu is up, don't do any of this
    if (feStackIndex > 0)
    {
        return;
    }


    if (!bitTest(mouseCursorMode, MCM_SingleClick))
    {
        if ((mouseCursorTypeRaw == MCT_DOCK) ||
            (mouseCursorTypeRaw == MCT_DOCK_SMALL))
        {
            selSelectionCopy((MaxAnySelection *)&mouseCursorSelect, (MaxAnySelection *)&selSelected);
            bitSet(mouseCursorMode, MCM_SingleClick);
            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
        }
        else if ((mouseCursorTypeRaw == MCT_SPECIAL_ACTIVATE) ||
                 (mouseCursorTypeRaw == MCT_SPECIAL_ACTIVATE_HOLD) ||
                 (mouseCursorTypeRaw == MCT_SPECIAL_DEACTIVATE))
        {
            selSelectionCopy((MaxAnySelection *)&mouseCursorSelect, (MaxAnySelection *)&selSelected);
            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
        }
        else if (mouseCursorTypeRaw == MCT_SUPPORT)
        {
            selSelectionCopy((MaxAnySelection *)&mouseCursorSelect, (MaxAnySelection *)&selSelected);
            mouseDoubleClickExpire = universe.totaltimeelapsed + mouseDoubleClickTime;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseClickShipDied
    Description : Takes care of ships dying at inopportune times by clearing the
                  ship from any file global selection lists or storage areas
    Inputs      : deadship - the ship that has just died
    Outputs     : removes the ship from mouseCursorSelect and the mouseCursorObjPtr
    Return      : void
----------------------------------------------------------------------------*/
void mouseClickShipDied(ShipPtr deadship)
{
    clRemoveShipFromSelection(&mouseCursorSelect, deadship);

    if ((mouseCursorObjPtr != NULL) && (mouseCursorObjPtr == (SpaceObj *)deadship))
    {
        mouseCursorObjPtr = NULL;
    }

    if (mouseCursorObjLast == (SpaceObj *)deadship)
    {
        mouseCursorObjLast = NULL;
    }
}



/*-----------------------------------------------------------------------------
    Name        : mouseDrawTriangle
    Description : Draws the mouse triangle on the screen
    Inputs      : void
    Outputs     : Draws the mouse triangle on the screen
    Return      : void
----------------------------------------------------------------------------*/
void mouseDrawTriangle(void)
{
    triangle mouseTri, mouseTri2;

    mouseTri.x0 = mouseCursorXPosition + 0;
    mouseTri.y0 = mouseCursorYPosition + 0;
    mouseTri.x1 = mouseCursorXPosition + 4;
    mouseTri.y1 = mouseCursorYPosition + 16;
    mouseTri.x2 = mouseCursorXPosition + 14;
    mouseTri.y2 = mouseCursorYPosition + 7;
    mouseTri2 = mouseTri;
    mouseTri2.x2++;
    primTriSolid2(&mouseTri2, mouseCursorContentColor);      //draw mouse cursor
    primTriOutline2(&mouseTri, 1, mouseCursorEdgeColor);
}


/*-----------------------------------------------------------------------------
    Name        : mouseGetLIF
    Description : Returns the cursor bitmap for the current cursor type
    Inputs      :
    Outputs     :
    Return      : lifheader pointer, or NULL for no cursor
----------------------------------------------------------------------------*/
lifheader* mouseGetLIF(void)
{
    switch (mouseCursorTypeRaw)
    {
        case MCT_NORMAL:
        case MCT_MOVEMENT:          return mouseCursorBitmaps[0];
        case MCT_ADD_SHIPS:         return mouseCursorBitmaps[1];
        case MCT_FOCUS:             return mouseCursorBitmaps[2];
        case MCT_ATTACK:            return mouseCursorBitmaps[3];
        case MCT_FORCE_ATTACK:      return mouseCursorBitmaps[4];
        case MCT_GUARD:             return mouseCursorBitmaps[5];
        case MCT_SPECIAL_ACTIVATE:  return mouseCursorBitmaps[6];
        case MCT_SPECIAL_ACTIVATE_HOLD: return mouseCursorBitmaps[7];
        case MCT_SPECIAL_DEACTIVATE:return mouseCursorBitmaps[8];
        case MCT_DIALOG:            return mouseCursorBitmaps[9];
        case MCT_RESOURCE:          return mouseCursorBitmaps[10];
        case MCT_DOCK:
        case MCT_DOCK_SMALL:        return mouseCursorBitmaps[11];
        case MCT_RESEARCH:          return mouseCursorBitmaps[12];
        case MCT_SENSORS:           return mouseCursorBitmaps[13];
        case MCT_BUILD:             return mouseCursorBitmaps[14];
        case MCT_SALVAGE:           return mouseCursorBitmaps[15];
        case MCT_SUPPORT:           return mouseCursorBitmaps[16];
        case MCT_FORCE_FIELD:       return mouseCursorBitmaps[17];
        case MCT_TRADER:            return mouseCursorBitmaps[18];
        case MCT_SLIPGATE_SMALL:    return mouseCursorBitmaps[19];
        case MCT_SLIPGATE_MEDIUM:   return mouseCursorBitmaps[20];
        case MCT_SLIPGATE_LARGE:    return mouseCursorBitmaps[21];
        case MCT_LEECH:             return mouseCursorBitmaps[22];
        case MCT_LINK:              return mouseCursorBitmaps[23];
        case MCT_MIMIC:             return mouseCursorBitmaps[24];
        case MCT_EMP:               return mouseCursorBitmaps[25];
        case MCT_SWARM:             return mouseCursorBitmaps[26];
        case MCT_FIRE_MISSILE:      return mouseCursorBitmaps[27];
        case MCT_RAMMING:           return mouseCursorBitmaps[28];
        case MCT_BEAST_WEAPON:      return mouseCursorBitmaps[29];
        case MCT_OBJECTIVES:        return mouseCursorBitmaps[30];
        default:                    return NULL;
    }
}

GLubyte cursorUnderContents[4096];

sdword lastUnderWidth = 0, lastUnderHeight = 0;
static sdword lastUnderX = 0, lastUnderY = 0;

void mouseRestoreCursorUnder(void)
{
    if (!feShouldSaveMouseCursor())
    {
        return;
    }
    if (RGLtype == SWtype)
    {
        rglRestoreCursorUnder(cursorUnderContents,
                              lastUnderWidth, lastUnderHeight,
                              lastUnderX, lastUnderY);
    }
#if GLC_STORE
    else
    {
        glcCursorUnder((ubyte*)cursorUnderContents,
                       lastUnderWidth, lastUnderHeight,
                       lastUnderX, lastUnderY,
                       FALSE);
    }
#endif
}

void mouseStoreCursorUnder(void)
{
    lifheader* texture;

    if (!feShouldSaveMouseCursor())
    {
        return;
    }

    texture = mouseGetLIF();
    if (texture == NULL)
    {
        memset(cursorUnderContents, 0, 4096);
    }
    else
    {
        lastUnderWidth = texture->width + 2;
        lastUnderHeight = texture->height + 2;
        if (RGLtype == SWtype)
        {
            lastUnderX = mouseCursorXPosition - 1;
            lastUnderY = mouseCursorYPosition - 1;
            rglSaveCursorUnder(cursorUnderContents, lastUnderWidth, lastUnderHeight, lastUnderX, lastUnderY);
        }
#if GLC_STORE
        else
        {
            lastUnderX = mouseCursorXPosition;
            lastUnderY = mouseCursorYPosition;
            glcCursorUnder((ubyte*)cursorUnderContents,
                           lastUnderWidth, lastUnderHeight,
                           lastUnderX, lastUnderY,
                           TRUE);
        }
#endif
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseDraw
    Description : Update the position of the mouse by polling the system and
                    draw it at it's new location.
    Inputs      : void
    Outputs     : updates mouse cursor position and button states to reflect
                    new mouse status.
    Return      : void
    Note        : because this routine performs drawing operations, it should
                    be called near the end rendering, before a buffer swap.
                  Checks to see if the mouse is inside or outside the client area
----------------------------------------------------------------------------*/
extern regionhandle spHyperspaceRollCallScreen;
#define SX primScreenToGLX
#define SY primScreenToGLY
void mouseDraw(void)
{
    bool32 primModeOn = TRUE;
    lifheader *texture = NULL;
    GLuint thandle = 0;
    bool32 textureMouse;
    sdword cursorType;
    sdword depthTestingState;

    if (mouseWillBeVisible > 0)
    {
        mouseWillBeVisible--;
        if (!mouseWillBeVisible)
            mouseIsVisible = TRUE;
    }

    if ((!mouseIsVisible) || (mouseDisabled))
    {
        return;
    }
    if (nisWideScreenActive && mrDisabled && gameIsRunning)
    {
        return;
    }

    if (RGLtype == SWtype)
    {
        textureMouse = FALSE;
    }
    else
    {
        textureMouse = TRUE;
    }

    if (textureMouse && !mouseGLInitialized)
    {
        mouseInitGL();
        mouseGLInitialized = TRUE;
    }

    if (!primModeEnabled)
    {
        primModeOn = FALSE;
        primModeSetFunction2();
    }

    depthTestingState = rndSetDepthTesting(FALSE);
    glEnable(GL_BLEND);
    rndAdditiveBlends(FALSE);
    rndTextureEnvironment(RTE_Modulate);

    // hold-type restore/expire
    if (mouseCursorHoldType != 0 && mouseCursorTypeRaw == 0)
    {
        if (mouseCursorHoldExpire < universe.totaltimeelapsed)
            mouseCursorHoldType = 0;
        else
            mouseCursorTypeRaw = mouseCursorHoldType;
    }

    cursorType = mouseCursorTypeRaw;

    // front-end / sensors override: force normal cursor
    if ((feStackIndex > 0 && !spHyperspaceRollCallScreen && !smSensorsActive) ||
        (bitTest(mouseCursorMode, MCM_Pointmode) && cursorType != MCT_FOCUS))
    {
        cursorType = 0;
    }

    if (smSensorsActiveGet())
    {
        cursorType = 0;
    }

    switch (cursorType)
    {
        case MCT_NO_CURSOR:
        case MCT_ROTATE:
        case MCT_ZOOM:
            // no cursor draw; fall through to triangle
            break;
        case MCT_NORMAL:
            texture = mouseCursorBitmaps[0];
            thandle = mouseCursorTextures[0];
            break;
        case MCT_MOVEMENT:
            texture = mouseCursorBitmaps[0];
            thandle = mouseCursorTextures[0];
            break;
        case MCT_ADD_SHIPS:
            texture = mouseCursorBitmaps[1];
            thandle = mouseCursorTextures[1];
            break;
        case MCT_FOCUS:
            texture = mouseCursorBitmaps[2];
            thandle = mouseCursorTextures[2];
            break;
        case MCT_ATTACK:
            texture = mouseCursorBitmaps[3];
            thandle = mouseCursorTextures[3];
            break;
        case MCT_FORCE_ATTACK:
            texture = mouseCursorBitmaps[4];
            thandle = mouseCursorTextures[4];
            break;
        case MCT_GUARD:
            texture = mouseCursorBitmaps[5];
            thandle = mouseCursorTextures[5];
            break;
        case MCT_SPECIAL_ACTIVATE:
            if (!mouseCursorHoldType)
            {
                mouseCursorHoldType   = MCT_SPECIAL_ACTIVATE;
                mouseCursorHoldExpire = universe.totaltimeelapsed + 1.0f;
            }
            texture = mouseCursorBitmaps[6];
            thandle = mouseCursorTextures[6];
            break;
        case MCT_SPECIAL_ACTIVATE_HOLD:
            texture = mouseCursorBitmaps[7];
            thandle = mouseCursorTextures[7];
            break;
        case MCT_SPECIAL_DEACTIVATE:
            texture = mouseCursorBitmaps[8];
            thandle = mouseCursorTextures[8];
            break;
        case MCT_DIALOG:
            texture = mouseCursorBitmaps[9];
            thandle = mouseCursorTextures[9];
            break;
        case MCT_RESOURCE:
            texture = mouseCursorBitmaps[10];
            thandle = mouseCursorTextures[10];
            break;
        case MCT_DOCK:
        case MCT_DOCK_SMALL:
            texture = mouseCursorBitmaps[11];
            thandle = mouseCursorTextures[11];
            break;
        case MCT_RESEARCH:
            texture = mouseCursorBitmaps[12];
            thandle = mouseCursorTextures[12];
            break;
        case MCT_SENSORS:
            texture = mouseCursorBitmaps[13];
            thandle = mouseCursorTextures[13];
            break;
        case MCT_BUILD:
            texture = mouseCursorBitmaps[14];
            thandle = mouseCursorTextures[14];
            break;
        case MCT_SALVAGE:
            texture = mouseCursorBitmaps[15];
            thandle = mouseCursorTextures[15];
            break;
        case MCT_SUPPORT:
            texture = mouseCursorBitmaps[16];
            thandle = mouseCursorTextures[16];
            break;
        case MCT_FORCE_FIELD:
            texture = mouseCursorBitmaps[17];
            thandle = mouseCursorTextures[17];
            break;
        case MCT_TRADER:
            texture = mouseCursorBitmaps[18];
            thandle = mouseCursorTextures[18];
            break;
        case MCT_SLIPGATE_SMALL:
            texture = mouseCursorBitmaps[19];
            thandle = mouseCursorTextures[19];
            break;
        case MCT_SLIPGATE_MEDIUM:
            texture = mouseCursorBitmaps[20];
            thandle = mouseCursorTextures[20];
            break;
        case MCT_SLIPGATE_LARGE:
            texture = mouseCursorBitmaps[21];
            thandle = mouseCursorTextures[21];
            break;
        case MCT_LEECH:
            texture = mouseCursorBitmaps[22];
            thandle = mouseCursorTextures[22];
            break;
        case MCT_LINK:
            texture = mouseCursorBitmaps[23];
            thandle = mouseCursorTextures[23];
            break;
        case MCT_MIMIC:
            texture = mouseCursorBitmaps[24];
            thandle = mouseCursorTextures[24];
            break;
        case MCT_EMP:
            texture = mouseCursorBitmaps[25];
            thandle = mouseCursorTextures[25];
            break;
        case MCT_SWARM:
            texture = mouseCursorBitmaps[26];
            thandle = mouseCursorTextures[26];
            break;
        case MCT_FIRE_MISSILE:
            texture = mouseCursorBitmaps[27];
            thandle = mouseCursorTextures[27];
            break;
        case MCT_RAMMING:
            texture = mouseCursorBitmaps[28];
            thandle = mouseCursorTextures[28];
            break;
        case MCT_BEAST_WEAPON:
            texture = mouseCursorBitmaps[29];
            thandle = mouseCursorTextures[29];
            break;
        case MCT_OBJECTIVES:
            texture = mouseCursorBitmaps[30];
            thandle = mouseCursorTextures[30];
            break;
        default:
            break;
    }

    if (texture != NULL)
    {
        if (textureMouse)
        {
            bool32 texOn;

            trClearCurrent();
            texOn = rndTextureEnable(TRUE);
            glBindTexture(GL_TEXTURE_2D, thandle);

            glColor3ub(255, 255, 255);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(SX(mouseCursorXPosition), SY(mouseCursorYPosition));
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(SX(mouseCursorXPosition), SY(mouseCursorYPosition + texture->height));
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(SX(mouseCursorXPosition + texture->width), SY(mouseCursorYPosition + texture->height));
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(SX(mouseCursorXPosition + texture->width), SY(mouseCursorYPosition));
            glEnd();

            rndTextureEnable(texOn);
        }
        else
        {
            glRasterPos2f(primScreenToGLX(mouseCursorXPosition),
                          primScreenToGLY(mouseCursorYPosition + texture->height));
            glDrawPixels(texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
        }
    }
    else
    {
        mouseDrawTriangle();
    }

  finish:

    glDisable(GL_BLEND);
    rndSetDepthTesting(depthTestingState);

    if (!primModeOn)
    {
        primModeClearFunction2();
    }
    mouseCursorOverDistance = REALlyBig;
    mouseCursorOverShipType = MC_NO_SHIP;

    if (bitTest(mouseCursorMode, MCM_CursorOverObj))
    {
        bitClear(mouseCursorMode, MCM_CursorOverObj);
    }
    else
    {
        mouseCursorObjPtr = NULL;
    }
    if (mouseCursorObjPtr != NULL)
    {
        mouseCursorObjLast     = mouseCursorObjPtr;
        mouseCursorLastObjTime = universe.totaltimeelapsed;
        mouseCursorLastX       = mouseCursorX();
        mouseCursorLastY       = mouseCursorY();
    }
}
#undef SX
#undef SY

char *tutMouseDrawMessage[] =
{   "Mouse_NormalMouse",
    "Mouse_RotateCamera",
    "Mouse_ZoomCamera",
    "Mouse_AddShips",
    "Mouse_FocusNoSelect",
    "Mouse_Movement",
    "Mouse_BandBoxAttack",
    "Mouse_ForceAttack",
    "Mouse_Guard",
    "Mouse_SpecialAttack",
    "Mouse_SpecialOpActivate",
    "Mouse_SpecialOpDeactivate",
    "Mouse_Gui",
    "Mouse_Resource",
    "Mouse_Derelict",
    "Mouse_Docking",
    "Mouse_SmallDocking",
    "Mouse_Research",
    "Mouse_Sensors",
    "Mouse_Build",
    "Mouse_Salvage",
    "Mouse_Support",
    "Mouse_Traders",
    "Mouse_EnemyShip",
    "Mouse_AlliedShip",
};



/*-----------------------------------------------------------------------------
    Name        : mouseCursorSet
    Description : sets the cursor mode for special case cursor settings
    Inputs      : mode - the mode to set cursormode to
    Outputs     : changes the cursormode flag
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorSet(udword mode)
{
    bitSet(mouseCursorMode, mode);
}

/*-----------------------------------------------------------------------------
    Name        : mouseCursorClear
    Description : Clears the cursor mode for special case cursor settings
    Inputs      : mode - the mode to clear
    Outputs     : changes the cursormode flag
    Return      : void
----------------------------------------------------------------------------*/
void mouseCursorClear(udword mode)
{
    bitClear(mouseCursorMode, mode);
}

/*-----------------------------------------------------------------------------
    Name        : mouseSetCursorSetting
    Description : Sets the mouse cursor type based on key/mouse state and what
                  is under the cursor.
    Inputs      : void
    Outputs     : Sets mouseCursorTypeRaw
    Return      : void
----------------------------------------------------------------------------*/
void mouseSetCursorSetting(void)
{
    SelectAnyCommand selection;
    udword mouseState[41];
    sdword keyState[41];
    sdword result[41];

    if (!gameIsRunning)
        return;

    mouseCursorTypeRaw = MCT_NORMAL;

    if (!mrDisabled)
    {
        memcpy(keyState, mrDetermineSelectionActions(result, (SelectAnyCommand *)&selSelected), sizeof(keyState));
        memset(mouseState, 0, sizeof(mouseState));

        if (mouseCursorObjPtr)
        {
            selection.numTargets = 1;
            selection.TargetPtr[0] = (TargetPtr)mouseCursorObjPtr;
            memcpy(mouseState, mrDetermineSelectionActions(result, &selection), sizeof(mouseState));
        }

        if (ioConsumePendingOverlayClick())
        {
            mouseCursorTypeRaw = MCT_OBJECTIVES;
        }
        else if (kbKeyIsHit(76))
        {
            if (!playPackets)
                mouseCursorTypeRaw = MCT_FORCE_ATTACK;
        }
        else if (kbKeyIsHit(78))
        {
            mouseCursorTypeRaw = MCT_ADD_SHIPS;
        }
        else if (kbKeyIsHit(74))
        {
            if (!playPackets)
                mouseCursorTypeRaw = MCT_GUARD;
        }
        else if (kbKeyIsHit(7))
        {
            if (!playPackets)
                mouseCursorTypeRaw = MCT_SPECIAL_ACTIVATE;
        }
        else if (kbKeyIsHit(77))
        {
            mouseCursorTypeRaw = MCT_FOCUS;
        }
        else if (kbKeyIsHit(75))
        {
            if (!playPackets)
                mouseCursorTypeRaw = MCT_ATTACK;
        }
        else if ((tutorial == TUTORIAL_ONLY || homeworldCSA == 1 || rightClickCSA == 1) &&
                 mouseCursorObjPtr &&
                 smBandboxEnabled != 1 &&
                 !smSensorsActive &&
                 !ShipInSelection(&selSelected, (Ship *)mouseCursorObjPtr))
        {
            switch (mrDetermineCursorType((udword *)keyState, mouseState, &selSelected.numShips, (Ship *)mouseCursorObjPtr))
            {
                case 1:
                    mouseCursorTypeRaw = MCT_RESOURCE;
                    break;
                case 2:
                    mouseCursorTypeRaw = MCT_SALVAGE;
                    break;
                case 3:
                    mouseCursorTypeRaw = MCT_ATTACK;
                    break;
                case 4:
                    mouseCursorTypeRaw = MCT_SUPPORT;
                    break;
                case 5:
                    mouseCursorTypeRaw = MCT_FORCE_FIELD;
                    break;
                case 6:
                    mouseCursorTypeRaw = MCT_SLIPGATE_SMALL;
                    break;
                case 7:
                    mouseCursorTypeRaw = MCT_SLIPGATE_MEDIUM;
                    break;
                case 8:
                    mouseCursorTypeRaw = MCT_SLIPGATE_LARGE;
                    break;
                case 9:
                    mouseCursorTypeRaw = MCT_DOCK_SMALL;
                    break;
                case 10:
                    mouseCursorTypeRaw = MCT_LEECH;
                    break;
                case 11:
                    mouseCursorTypeRaw = MCT_LINK;
                    break;
                case 12:
                    mouseCursorTypeRaw = MCT_MIMIC;
                    break;
                case 13:
                    mouseCursorTypeRaw = MCT_EMP;
                    break;
                case 14:
                    mouseCursorTypeRaw = MCT_SWARM;
                    break;
                case 15:
                    mouseCursorTypeRaw = MCT_RAMMING;
                    break;
                case 16:
                    mouseCursorTypeRaw = MCT_FIRE_MISSILE;
                    break;
                case 17:
                    mouseCursorTypeRaw = MCT_BEAST_WEAPON;
                    break;
                default:
                    return;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
   Cursor Text Code
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : mouseCursorTextDraw
    Description : Draws the name of the ship type associated with mouseCursorOverShipType
    Inputs      : void
    Outputs     : Screen text roughly resembling the name of the ship type
    Return      :
----------------------------------------------------------------------------*/
void mouseCursorTextDraw(void)
{
    static sdword cursortexttype = MC_NO_SHIP;
    fonthandle fhSave;

    if ((!mrRenderMainScreen) || (!gameIsRunning) || (mrWhiteOut) || (nisIsRunning) || (keyIsHit(RMOUSE_BUTTON)))
    {
        return;
    }

    // if there's something in that cool global variable
    if ((mouseCursorOverShipType != MC_NO_SHIP) && (mouseCursorOverShipType != MC_RESOURCE) && (mouseCursorOverShipType != MC_DERELICT))
    {
        cursortexttype = mouseCursorOverShipType;
    }
    else
    {
        if (mouseCursorTextExpire < universe.totaltimeelapsed)
        {
            cursortexttype = MC_NO_SHIP;
        }
    }

    if (cursortexttype != MC_NO_SHIP)
    {
        // display that baby!
        fhSave = fontMakeCurrent(mouseCursorFont);

        if (cursortexttype == MC_ENEMY_SHIP)
        {
            if (mouseCursorOverLODs & MOUSE_IdentifyLODMask)
            {                                               //if it's been seen closely
                if (!singlePlayerGame)
                {
                    fontPrintf(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR, "%s's %s",
                              playerNames[mouseCursorEnemyPlayer], ShipTypeToNiceStr(mouseCursorEnemyType));
                }
                else
                {
                    fontPrintf(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                               ShipTypeToNiceStr(mouseCursorEnemyType));
                }
            }
            else
            {
                fontPrint(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                          strGetString(strEnemyShip));
            }
        }
        else if (cursortexttype == MC_ALLIED_SHIP)
        {
            if (!singlePlayerGame)
            {
                fontPrintf(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR, "%s's %s",
                          playerNames[mouseCursorEnemyPlayer], ShipTypeToNiceStr(mouseCursorEnemyType));
            }
            else
            {
                // allied ships in the single player game (defector and traders) only
                // display their shiptypes and not the player name
                fontPrint(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                           ShipTypeToNiceStr(mouseCursorEnemyType));
            }
        }
        else
        {
            fontPrint(TW_CURSORTEXT_X, MAIN_WindowHeight - TW_CURSORTEXT_Y, TW_CURSORTEXT_COLOR,
                       ShipTypeToNiceStr(cursortexttype));
        }


        fontMakeCurrent(fhSave);
    }
}


/*-----------------------------------------------------------------------------
    Name        : mouseClipPointToRect
    Description : Clip an x/y coordinate to a rectangle.
    Inputs      : x, y - position to clip
                  rect - rectangle to clip to
    Outputs     : clips x, y to rect
    Return      :
----------------------------------------------------------------------------*/
void mouseClipPointToRect(sdword *x, sdword *y, rectangle *rect)
{
    if (*x < rect->x0)
    {
        *x = rect->x0;
    }
    if (*x >= rect->x1)
    {
        *x = rect->x1 - 1;
    }
    if (*y < rect->y0)
    {
        *y = rect->y0;
    }
    if (*y >= rect->y1)
    {
        *y = rect->y1 - 1;
    }
}

/*=============================================================================
    Name    : mousePoll
    Purpose : Poll current mouse location and store internally

    Created 7/7/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
void mousePoll(void)
{
    rectangle clientRect;
    POINT mousePoint;

    if (mouseDisabled)
    {
        return;
    }
    if (demDemoPlaying)
    {
        return;
    }
    utyClientRectGet(&clientRect);                          //get window location
    GetCursorPos(&mousePoint);                              //get location of Windows mouse cursor
    mouseCursorXPosition = mousePoint.x - clientRect.x0;    //compute relative coords
    mouseCursorYPosition = mousePoint.y - clientRect.y0;
    screenX = mouseCursorXPosition;
    screenY = mouseCursorYPosition;

    if (mouseClip)
    {
        mouseClipPointToRect(&mouseCursorXPosition, &mouseCursorYPosition, &mouseClipRect);
        mousePositionSet(mouseCursorXPosition, mouseCursorYPosition);
    }
    //perform client area enter/exit logic to hide/show Windows system cursor
    if (primPointInRectXY2(&clientRect, mousePoint.x, mousePoint.y))
    {                                                       //mouse inside window's client area
        if (!mouseInsideClient)
        {                                                   //just entered region
            ShowCursor(FALSE);
            mouseInsideClient = TRUE;
        }
    }
    else
    {                                                       //else outside region
        if (mouseInsideClient)
        {                                                   //just exited region
            ShowCursor(TRUE);
            mouseInsideClient = FALSE;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : mouseDisable/mouseEnable
    Description : Disable and re-enable the mouse
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void mouseDisable(void)
{
    mouseCursorHide();
    mouseDisabled = TRUE;
}
void mouseEnable(void)
{
    mouseCursorShow();
    mouseDisabled = FALSE;
}

void mouseForceInvisible(void)
{
    mouseDisable();
}
