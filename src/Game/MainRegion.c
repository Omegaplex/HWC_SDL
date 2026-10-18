/*=============================================================================
    Name    : MainRegion.c
    Purpose : Decompiled functions from cataclysm.exe (MainRegion module)
=============================================================================*/

#include <stdio.h>
#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "font.h"
#include "fontreg.h"
#include "prim2d.h"
#include "universe.h"
#include "Gun.h"
#include "Tactics.h"
#include "Globals.h"

/*-----------------------------------------------------------------------------
    Forward declarations
-----------------------------------------------------------------------------*/
void tabStatsResetDrag(void);
sdword kbKeyIsHit(sdword functionIndex);
sdword SEgetShipClass(Ship *ship);
void spRestoreManagerScreen(void);

/*-----------------------------------------------------------------------------
    Module-local mutable state (tabStats overlay system)
-----------------------------------------------------------------------------*/
static sdword tabStatsDragActive    = 0;    /* dword_A5E208 - drag in progress */
static sdword tabStatsDragLastMouseY = 0;   /* dword_A5E20C */
static sdword tabStatsDragInitMouseX = 0;   /* dword_A5E210 */
static sdword tabStatsDrawEnabled   = 0;    /* dword_A5E214 - permit fontPrint calls */
static sdword tabStatsDragInitWindowX = 0;  /* dword_A5E218 */
static sdword tabStatsDragInitWindowY = 0;  /* dword_A5E21C */
static sdword tabStatsDragInitWindowR = 0;  /* dword_A5E220 */
static sdword tabStatsDragInitWindowB = 0;  /* dword_A5E224 */
static sdword tabStatsDragInitMouseXSave = 0; /* dword_A5E228 */
static sdword tabStatsDragInitMouseYSave = 0; /* dword_A5E22C */
static sdword tabStatsOption4Toggle = 0;    /* dword_A5E230 - misc status display */
static sdword tabStatsColumnWidth   = 0;    /* dword_A5E234 - width of widest flag col */
static sdword tabStatsWindowBottom  = 0;    /* dword_A5E238 - tracked max height */
static sdword tabStatsWindowRight   = 0;    /* dword_A5E23C - tracked max width */
static sdword tabStatsFontHandle    = 0;    /* dword_A5E240 */
static sqword tabStatsWindowPos     = 0;    /* qword_893794: LODWORD=x, HIDWORD=y */

/* display option toggle flags */
static sdword tabStatsOption1 = 0;          /* dword_893778 */
static sdword tabStatsOption2 = 0;          /* dword_89377C */
static sdword tabStatsOption3 = 0;          /* dword_893780 */
static sdword tabStatsOption5 = 0;          /* dword_893784 */
static sdword tabStatsOption6 = 0;          /* dword_893788 */
static sdword tabStatsOption7 = 0;          /* dword_89378C */
static sdword tabStatsOption8 = 0;          /* dword_893790 */

/*-----------------------------------------------------------------------------
    Binary data segment references (defined in binary; TBD in source)
-----------------------------------------------------------------------------*/
/* current mouse position (HW1 mainrgn.c, also at binary 0xB38BD8/0xB38BD0) */
extern sdword dword_B38BD8;  /* mouse X position for drag hit-test */
extern sdword dword_B38BD0;  /* mouse Y position for drag hit-test */
extern ubyte  byte_A2DCD4;   /* drag-suppression flag */

/* color constants from binary data segment (0x72Bxxx) */
extern sdword dword_72B114;
extern sdword dword_72B118;
extern sdword dword_72B11C;
extern sdword dword_72B120;
extern sdword dword_72B124;
extern sdword dword_72B128;
extern sdword dword_72B12C;
extern sdword dword_72B130;
extern sdword dword_72B134;
extern sdword dword_72B138;
extern sdword dword_72B13C;
extern sdword dword_72B144;
extern sdword dword_72B14C;
extern sdword dword_72B150;
extern sdword dword_72B160;  /* enabled color */
extern sdword dword_72B164;  /* disabled color */
extern sdword dword_72B174;  /* rect fill color */
extern sdword dword_72B178;  /* rect outline color */
extern udword unk_72B188;    /* border thickness (float as bits) */
extern uword  word_72B18A;   /* border corner radius */
extern color  unk_72B198;    /* first entry in health color table (color1) */
extern real64 dbl_72AB18;    /* row offset base constant */
extern real64 dbl_72B1E0;    /* health color table end sentinel / header row offset scale */
extern color  dword_72B1D0;  /* default health color 2 */
extern color  dword_72B1D4;  /* default health color 1 */

/* localized string tables from binary data segment (0xBBxxxx) */
extern sdword dword_BB3620;  /* string pointer table indexed by display option */
extern sdword dword_BB4C7C;  /* "OFF" string pointer */
extern sdword dword_BB4C80;  /* "ON" string pointer */
extern sdword dword_BB4F54;
extern sdword dword_BB4F80;  /* status flags section header string pointer */
extern sdword dword_BB4F8C;  /* upgrade level string 0 */
extern sdword dword_BB4F90;  /* upgrade level string 1 */
extern sdword dword_BB4F94;  /* upgrade level string 2 */
extern sdword dword_BB4F98;  /* weapon slot label */
extern sdword dword_BB4F9C;  /* weapon type 0 label */
extern sdword dword_BB4FA0;  /* weapon type 1 label */
extern sdword dword_BB4FA4;  /* weapon type 2 label */
extern sdword dword_BB4FA8;  /* weapon type 3 label */
extern sdword dword_BB4FAC;  /* upgrade type 0 label */
extern sdword dword_BB4FB0;  /* upgrade type 1 label */
extern sdword dword_BB4FB4;  /* upgrade type 2 label */
extern sdword dword_BB4FB8;  /* upgrade type 3 label */
extern sdword dword_BB4FBC;  /* upgrade type 4 label */
extern sdword dword_BB4FC0;  /* upgrade type 5 label */
extern sdword dword_BB4FC4;  /* upgrade type 6 label */
extern sdword dword_BB4FC8;  /* upgrade type 8 label */
extern sdword dword_BB4FCC;  /* weapon stats header */
extern sdword dword_BB4FD0;  /* weapon col header 1 */
extern sdword dword_BB4FD4;  /* weapon col header 2 */
extern sdword dword_BB4FD8;  /* weapon col header 3 */
extern sdword dword_BB4FDC;  /* no weapons label */
extern sdword dword_BB4FE0;  /* display option 1 label */
extern sdword dword_BB4FE4;  /* display option 2 label */
extern sdword dword_BB4FE8;  /* display option 3 label */
extern sdword dword_BB4FEC;  /* display option 4 label */
extern sdword dword_BB4FF0;  /* display option 5 label */
extern sdword dword_BB4FF4;  /* display option 6 label */
extern sdword dword_BB4FF8;  /* display option 7 label */
extern sdword dword_BB4FFC;  /* display option 8 label */
extern sdword dword_BB5010;  /* ramming frigate upgrade on label */
extern sdword dword_BB500C;  /* ramming frigate no upgrade label */
extern sdword dword_BB5034;  /* capital ship label */
extern sdword dword_BB4F80;  /* status section header */

/* string used as column-width measurement spacer */
extern char  location[];
extern char  *a1ShowMaxTechno; /* widest display-option label; used to size the popup panel */

/*-----------------------------------------------------------------------------
    Status flag display array (binary: 0x8937E0, 26 entries x 5 dwords)

    Each entry:
      flagMask    - bitmask of ship status flags tested against ship->unk_3F4
      stringIndex - index into dword_BB3620[] string pointer table
      visible     - 1 if this flag is active and should be rendered
      drawColor   - color to use when rendering
      timestamp   - universe.totaltimeelapsed at last draw
-----------------------------------------------------------------------------*/
typedef struct
{
    udword flagMask;
    sdword stringIndex;
    sdword visible;
    color  drawColor;
    udword timestamp;
} TabStatusFlagEntry;

#define TAB_STATUS_FLAG_COUNT 26
static TabStatusFlagEntry tabStatusFlags[TAB_STATUS_FLAG_COUNT];


// ============================================================================
// Decompiled from cataclysm.exe @ 0x518DB0
void __cdecl mrManagerScreenBegin(int flags)
{
    if (dword_A6ADB0)
        spRestoreManagerScreen();

    dword_A6ADB0 = flags;

    if ((flags & 4) != 0)
    {
        selSelected.numShips = 0;
        ioUpdateShipTotals();
    }

    dword_A5B1CC = 1;

    if ((flags & 2) != 0)
    {
        mrDisable();
        mouseCursorHide();
    }

    if ((flags & 1) != 0)
        sub_56F8D0();
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x511690
int mrUpdateMouseDrag(void)
{
    int result;

    result = mrMouseDragActive;
    if (!mrMouseDragActive)
        return result;

    result = mouseCurX;
    mrDragEndX   += mouseCurX - mrDragLastX;
    mrDragStartX += mouseCurX - mrDragLastX;
    mrDragEndY   += mouseCurY - mrDragLastY;
    mrDragStartY += mouseCurY - mrDragLastY;
    mrDragLastX   = mouseCurX;
    mrDragLastY   = mouseCurY;

    return result;
}

// Cataclysm-specific: print a string and track the bounding box of all rendered text
sdword tabStatsPrintString(sdword x, sdword y, color c, char *str)
{
    udword right  = (udword)(x + fontWidth(str));
    udword bottom = (udword)(y + fontHeight(" "));

    if (right > (udword)tabStatsWindowRight)
        tabStatsWindowRight = right + 10;
    if (bottom > (udword)tabStatsWindowBottom)
        tabStatsWindowBottom = bottom + 10;

    if (tabStatsDrawEnabled)
        return fontPrint(x, y, c, str);
    return tabStatsDrawEnabled;
}

// Cataclysm-specific: set per-flag visibility and colors, then draw all active status flags
sdword tabStatsDrawStatusFlagList(Ship *ship)
{
    sdword  rowHeight = fontHeight(" ");
    sdword  i;
    sdword  drawY;
    sdword  winX;
    char   *flagStr;
    udword  right, bottom;
    sdword  visCount = 0;
    sdword  shipFlags = ship->upgradeState.mask; /* status flag bits */
    udword  upgradeMask = shipTypeGetUpgradeMask(ship->shiprace); /* binary passes shiprace (ship+0x2B4) */
    Player *playerData = ship->playerowner;

    /* pass 1: determine visibility and color for each status flag entry */
    for (i = 0; i < TAB_STATUS_FLAG_COUNT; i++)
    {
        udword mask = tabStatusFlags[i].flagMask;
        if (tabStatsOption3 && (mask & (udword)shipFlags) != 0)
        {
            tabStatusFlags[i].visible   = 1;
            tabStatusFlags[i].drawColor = dword_72B11C;
        }
        else if (tabStatsOption6 && (playerData->shipUpgrades.mask & upgradeMask & mask) != 0)
        {
            tabStatusFlags[i].visible   = 1;
            tabStatusFlags[i].drawColor = dword_72B124;
        }
        else if (!singlePlayerGame && tabStatsOption5 && (upgradeMask & mask) != 0)
        {
            tabStatusFlags[i].visible   = 1;
            tabStatusFlags[i].drawColor = dword_72B120;
        }
        else if (tabStatsOption4Toggle)
        {
            tabStatusFlags[i].visible   = 1;
            tabStatusFlags[i].drawColor = dword_72B118;
        }
        else
        {
            tabStatusFlags[i].visible = 0;
        }
    }

    /* pass 2: render visible flags and track bounds */
    tabStatsColumnWidth = 10;
    drawY = 2 * rowHeight;
    winX  = (sdword)LODWORD(tabStatsWindowPos);

    for (i = 0; i < TAB_STATUS_FLAG_COUNT; i++)
    {
        if (tabStatusFlags[i].visible == 1)
        {
            flagStr = (char *)((&dword_BB3620)[tabStatusFlags[i].stringIndex]);
            right   = (udword)(winX + fontWidth(flagStr));
            bottom  = (udword)(drawY + (sdword)HIDWORD(tabStatsWindowPos) + 20 + fontHeight(" "));
            if (right > (udword)tabStatsWindowRight)
                tabStatsWindowRight = right + 10;
            if (bottom > (udword)tabStatsWindowBottom)
                tabStatsWindowBottom = bottom + 10;
            if (tabStatsDrawEnabled)
                fontPrint(winX, drawY + (sdword)HIDWORD(tabStatsWindowPos) + 20,
                          tabStatusFlags[i].drawColor, flagStr);
            if (tabStatsColumnWidth < fontWidth(flagStr) + 10)
                tabStatsColumnWidth = fontWidth(flagStr) + 10;
            tabStatusFlags[i].timestamp = LODWORD(universe.totaltimeelapsed);
            visCount++;
            drawY += rowHeight;
        }
    }

    /* draw section header if any flags were visible */
    if (visCount)
    {
        char   *hdr    = (char *)dword_BB4F80;
        sdword  hdrX   = (sdword)LODWORD(tabStatsWindowPos);
        sdword  hdrY   = (sdword)HIDWORD(tabStatsWindowPos) + 20;
        right  = (udword)(hdrX + fontWidth(hdr));
        bottom = (udword)(hdrY + fontHeight(" "));
        if (right > (udword)tabStatsWindowRight)
            tabStatsWindowRight = right + 10;
        if (bottom > (udword)tabStatsWindowBottom)
            tabStatsWindowBottom = bottom + 10;
        if (tabStatsDrawEnabled)
            return fontPrint(hdrX, hdrY, dword_72B114, hdr);
    }
    return visCount;
}

// Cataclysm-specific: toggle one of the 8 tabStats display options by key character
sdword tabStatsToggleDisplayOption(char key)
{
    switch (key)
    {
    case '1': tabStatsOption1       = (tabStatsOption1 == 0);       break;
    case '2': tabStatsOption2       = (tabStatsOption2 == 0);       break;
    case '3': tabStatsOption3       = (tabStatsOption3 == 0);       break;
    case '4': tabStatsOption4Toggle = (tabStatsOption4Toggle == 0);  break;
    case '5': tabStatsOption5       = (tabStatsOption5 == 0);       break;
    case '6': tabStatsOption6       = (tabStatsOption6 == 0);       break;
    case '7': tabStatsOption7       = (tabStatsOption7 == 0);       break;
    case '8': tabStatsOption8       = (tabStatsOption8 == 0);       break;
    default:  return telldir();
    }
    return telldir();
}

void tabStatsStartup(void)
{
    tabStatsFontHandle = frFontRegister("default.hff");
    tabStatsResetDrag();
}

void tabStatsResetDrag(void)
{
    tabStatsDragActive = 0;
}

// Cataclysm-specific: determine the two health-display colors for ship's current health ratio
void tabStatsGetHealthColors(Ship *ship, color *c1, color *c2)
{
    /* Walk binary health color table: each entry = [threshold(float), c2(color), c1(color)] */
    /* Table starts at unk_72B198 (= color1 of entry 0), ends before &dbl_72B1E0.           */
    color  *entry = &unk_72B198;
    real32  ratio = ship->health / ship->staticinfo->maxhealth * (real32)dbl_72AED0;
    bool32  found = 0;

    while (entry < (color *)&dbl_72B1E0)
    {
        if (ratio >= (real64)*((real32 *)entry - 2))
        {
            *c1 = *entry;
            *c2 = *(entry - 1);
            found = 1;
        }
        entry += 3;
        if (found)
            return;
    }
    *c1 = dword_72B1D4;
    *c2 = dword_72B1D0;
}

// Cataclysm-specific: initiate a drag operation on the stats overlay window
bool32 tabStatsStartDrag(void)
{
    if (!kbKeyIsHit(98)
        || byte_A2DCD4
        || dword_B38BD8 < (sdword)LODWORD(tabStatsWindowPos) - 10
        || dword_B38BD0 < (sdword)HIDWORD(tabStatsWindowPos) - 10
        || dword_B38BD8 >= tabStatsWindowRight
        || dword_B38BD0 >= tabStatsWindowBottom)
    {
        return 0;
    }
    tabStatsDragInitWindowB  = tabStatsWindowBottom;
    tabStatsDragInitWindowY  = (sdword)HIDWORD(tabStatsWindowPos) - 10;
    tabStatsDragInitMouseXSave = dword_B38BD8;
    tabStatsDragInitMouseX   = dword_B38BD8;
    tabStatsDragInitWindowR  = tabStatsWindowRight;
    tabStatsDragActive       = 1;
    tabStatsDragInitMouseYSave = dword_B38BD0;
    tabStatsDragLastMouseY   = dword_B38BD0;
    tabStatsDragInitWindowX  = (sdword)LODWORD(tabStatsWindowPos) - 10;
    return 1;
}

// Cataclysm-specific: end a drag and commit window position delta
sdword tabStatsEndDrag(void)
{
    sdword result = tabStatsDragActive;
    if (tabStatsDragActive)
    {
        LODWORD(tabStatsWindowPos) = dword_B38BD8 - tabStatsDragInitMouseXSave + (sdword)LODWORD(tabStatsWindowPos);
        HIDWORD(tabStatsWindowPos) += dword_B38BD0 - tabStatsDragInitMouseYSave;
        tabStatsDragActive = 0;
    }
    return result;
}

bool32 tabStatsIsDragging(void)
{
    return tabStatsDragActive;
}


// Cataclysm-specific: draw the display-options status popup (8 toggle rows with ON/OFF labels)
sdword tabStatsDrawDisplayOptionStatus(void)
{
    sdword     rowH = fontHeight(" ");
    const char *state;
    color       c;
    char        line[256];
    rectangle   rect;

    rect.x0 = 10;
    rect.y0 = 10;
    rect.x1 = fontWidth(a1ShowMaxTechno) + 50;
    rect.y1 = 7 * rowH + 40;
    primRectSolid2(&rect, (color)dword_72B174);
    primRoundRectOutline(&rect, (sdword)unk_72B188, (color)dword_72B178, word_72B18A, word_72B18A);

    /* row 1 */
    state = tabStatsOption1 ? (const char *)dword_BB4C80 : (const char *)dword_BB4C7C;
    c     = tabStatsOption1 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '1', (const char *)dword_BB4FE0, state);
    fontPrint(20, 20, c, line);

    /* row 2 */
    state = tabStatsOption2 ? (const char *)dword_BB4C80 : (const char *)dword_BB4C7C;
    c     = tabStatsOption2 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '2', (const char *)dword_BB4FE4, state);
    fontPrint(20, rowH + 20, c, line);

    /* row 3 */
    state = tabStatsOption3 ? (const char *)dword_BB4C80 : (const char *)dword_BB4C7C;
    c     = tabStatsOption3 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '3', (const char *)dword_BB4FE8, state);
    fontPrint(20, 2 * rowH + 20, c, line);

    /* row 4 */
    state = tabStatsOption4Toggle ? (const char *)dword_BB4C80 : (const char *)dword_BB4C7C;
    c     = tabStatsOption4Toggle ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '4', (const char *)dword_BB4FEC, state);
    fontPrint(20, 3 * rowH + 20, c, line);

    /* row 5 */
    state = tabStatsOption5 ? (const char *)dword_BB4C80 : (const char *)dword_BB4C7C;
    c     = tabStatsOption5 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '5', (const char *)dword_BB4FF0, state);
    fontPrint(20, 4 * rowH + 20, c, line);

    /* row 6: state depends on option4 override; color depends on option6 */
    state = tabStatsOption5 ? (const char *)dword_BB4C80 : (const char *)(&dword_BB3620)[tabStatsOption6];
    c     = tabStatsOption6 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '6', (const char *)dword_BB4FF4, state);
    fontPrint(20, 5 * rowH + 20, c, line);

    /* row 7 */
    state = tabStatsOption7 ? (const char *)dword_BB4C80 : (const char *)(&dword_BB3620)[tabStatsOption6];
    c     = tabStatsOption7 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '7', (const char *)dword_BB4FF8, state);
    fontPrint(20, 6 * rowH + 20, c, line);

    /* row 8 */
    state = tabStatsOption8 ? (const char *)dword_BB4C80 : (const char *)(&dword_BB3620)[tabStatsOption6];
    c     = tabStatsOption8 ? dword_72B160 : dword_72B164;
    sprintf(line, "(%c) %s %s", '8', (const char *)dword_BB4FFC, state);
    return fontPrint(20, 7 * rowH + 20, c, line);
}

// Cataclysm-specific: draw the current upgrade-level label for the ship's weapon/upgrade system
sdword tabStatsDrawUpgradeLevel(Ship *ship)
{
    sdword     result = tabStatsOption2;
    char       label[256];
    sdword     x, y;
    udword     right, bottom;
    color      c;
    sdword upgradeLevel = ship->upgradeState.level;

    if (!tabStatsOption2)
        return result;

    switch (upgradeLevel)
    {
    case 0:  sprintf(label, (const char *)dword_BB4F8C); break;
    case 1:  sprintf(label, (const char *)dword_BB4F90); break;
    case 2:  sprintf(label, (const char *)dword_BB4F94); break;
    default: break;
    }

    x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth;
    y = (sdword)HIDWORD(tabStatsWindowPos) + 50;

    if (shipHasAvailableUpgradeLevel(ship))
    {
        c = (color)dword_72B150;
    }
    else
    {
        c = (color)dword_72B14C;
    }

    right  = (udword)(x + fontWidth(label));
    bottom = (udword)(y + fontHeight(" "));
    if (right > (udword)tabStatsWindowRight)
        tabStatsWindowRight = right + 10;
    if (bottom > (udword)tabStatsWindowBottom)
        tabStatsWindowBottom = bottom + 10;
    result = tabStatsDrawEnabled;
    if (tabStatsDrawEnabled)
        return fontPrint(x, y, c, label);
    return result;
}
// Cataclysm-specific: draw weapon/upgrade statistics columns for a selected ship in the tabStats panel
sdword tabStatsDrawShipSpecificStats(Ship *ship)
{
    sdword  rowH          = fontHeight(" ");
    sdword  result        = tabStatsOption1;
    sdword *upgradeSlots;
    sdword  labelColWidth = 0;
    sdword  classColWidth = 0;
    sdword  typeColWidth  = 0;
    sdword  visibleIdx    = 0;
    sdword  visibleRow    = 0;
    sdword  i, slotOff;
    sdword  installedFlags[9];
    sdword  disabled[30];
    char    label[256];
    color   textColor;
    sdword  x, y, rowOff;
    udword  right, bottom;

    if (!tabStatsOption1)
        return result;

    upgradeSlots = (sdword *)ship->unk_490;

    if (upgradeSlots)
    {
        /* pass 1: disabled[i] = 1 if this slot's upgrade is locked out for the ship */
        if (upgradeSlots[0] > 0)
        {
            for (i = 0, slotOff = 0; i < upgradeSlots[0]; i++, slotOff += 34)
            {
                sdword upgradeType = upgradeSlots[slotOff + 1];
                disabled[i] = (upgradeType == 5 && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE))
                           || (upgradeType == 8 && !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT));
            }
        }

        /* pass 2: compute max width of "Weapon N:" slot-label column */
        if (upgradeSlots[0] > 0)
        {
            for (i = 0; i < upgradeSlots[0]; i++)
            {
                if (!disabled[i])
                {
                    sprintf(label, "%s %d:", (const char *)dword_BB4F98, i);
                    if (fontWidth(label) > labelColWidth)
                        labelColWidth = fontWidth(label);
                }
            }
        }

        /* pass 3: compute max width of weapon-class column */
        for (i = 0, slotOff = 0; i < (sdword)upgradeSlots[0]; i++, slotOff += 34)
        {
            if (!disabled[i] && shipCanDisplayUpgradeType(ship, upgradeSlots[slotOff + 1]))
            {
                sdword *staticInfo = (sdword *)upgradeSlots[slotOff + 34];
                sdword  shipClass  = staticInfo[5]; /* offset 20 = field 5 */
                switch (shipClass)
                {
                case 0: sprintf(label, dword_BB4F9C); break;
                case 1: sprintf(label, dword_BB4FA0); break;
                case 2: sprintf(label, dword_BB4FA4); break;
                case 3: sprintf(label, dword_BB4FA8); break;
                default: label[0] = '\0'; break;
                }
                {
                    sdword w = fontWidth(location) + labelColWidth + fontWidth(label);
                    if (w > classColWidth) classColWidth = w;
                }
            }
        }

        /* seed typeColWidth from column-2 header width */
        {
            sdword hdrW = fontWidth((char *)dword_BB4FD4);
            sdword w    = hdrW + classColWidth + fontWidth(location);
            if (w > 0) typeColWidth = w;
        }

        /* pass 4: compute max width of upgrade-type column */
        for (i = 0, slotOff = 0; i < (sdword)upgradeSlots[0]; i++, slotOff += 34)
        {
            if (!disabled[i] && shipCanDisplayUpgradeType(ship, upgradeSlots[slotOff + 1]))
            {
                sdword upgradeType = upgradeSlots[slotOff + 1];
                switch (upgradeType)
                {
                case 0: sprintf(label, (const char *)dword_BB4FAC); break;
                case 1: sprintf(label, (const char *)dword_BB4FB0); break;
                case 2: sprintf(label, dword_BB4FB4);               break;
                case 3: sprintf(label, (const char *)dword_BB4FB8); break;
                case 4: sprintf(label, (const char *)dword_BB4FBC); break;
                case 5: sprintf(label, (const char *)dword_BB4FC0); break;
                case 6: sprintf(label, (const char *)dword_BB4FC4); break;
                case 7: sprintf(label, (const char *)dword_BB4F54);  break;
                case 8: sprintf(label, (const char *)dword_BB4FC8); break;
                default: label[0] = '\0'; break;
                }
                {
                    sdword w = fontWidth(location) + classColWidth + fontWidth(label);
                    if (w > typeColWidth) typeColWidth = w;
                }
            }
        }

        /* pass 5: draw each visible slot's four columns */
        for (i = 0, slotOff = 0; i < (sdword)upgradeSlots[0]; i++, slotOff += 34)
        {
            if (!disabled[i])
            {
                sdword *slotPtr = &upgradeSlots[slotOff + 1];
                if (shipCanDisplayUpgradeType(ship, *slotPtr))
                {
                    sdword *staticInfo;
                    sdword  shipClass;
                    sdword  nextVisibleIdx = visibleIdx + 1;

                    rowOff = (sdword)(((double)visibleRow + dbl_72AB18) * (double)rowH);

                    /* col 0: weapon-slot label "Weapon N:" */
                    sprintf(label, "%s %d:", (const char *)dword_BB4F98, nextVisibleIdx);
                    x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth;
                    y = (sdword)HIDWORD(tabStatsWindowPos) + 70 + rowOff;
                    right  = (udword)(x + fontWidth(label));
                    bottom = (udword)(y + fontHeight(" "));
                    if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
                    if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
                    if (tabStatsDrawEnabled) fontPrint(x, y, (color)dword_72B128, label);

                    /* col 1: weapon class */
                    staticInfo = (sdword *)slotPtr[33];
                    shipClass  = staticInfo[5];
                    textColor  = (color)dword_72B12C;
                    switch (shipClass)
                    {
                    case 0: sprintf(label, dword_BB4F9C); textColor = (color)dword_72B12C; break;
                    case 1: sprintf(label, dword_BB4FA0); textColor = (color)dword_72B130; break;
                    case 2: sprintf(label, dword_BB4FA4); textColor = (color)dword_72B134; break;
                    case 3: sprintf(label, dword_BB4FA8); textColor = (color)dword_72B138; break;
                    default: label[0] = '\0'; break;
                    }
                    x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth + labelColWidth;
                    y = (sdword)HIDWORD(tabStatsWindowPos) + 70 + rowOff;
                    right  = (udword)(x + fontWidth(label));
                    bottom = (udword)(y + fontHeight(" "));
                    if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
                    if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
                    if (tabStatsDrawEnabled) fontPrint(x, y, textColor, label);

                    /* col 2: upgrade type (highlight if installed) */
                    textColor = (color)dword_72B144;
                    switch (*slotPtr)
                    {
                    case 0: sprintf(label, (const char *)dword_BB4FAC); break;
                    case 1: sprintf(label, (const char *)dword_BB4FB0); break;
                    case 2: sprintf(label, dword_BB4FB4);               break;
                    case 3: sprintf(label, (const char *)dword_BB4FB8); break;
                    case 4: sprintf(label, (const char *)dword_BB4FBC); break;
                    case 5: sprintf(label, (const char *)dword_BB4FC0); break;
                    case 6: sprintf(label, (const char *)dword_BB4FC4); break;
                    case 7: sprintf(label, (const char *)dword_BB4F54);  break;
                    case 8: sprintf(label, (const char *)dword_BB4FC8); break;
                    default: label[0] = '\0'; break;
                    }
                    if (shipCollectInstalledUpgradeTypeFlags(ship, installedFlags) && installedFlags[*slotPtr])
                        textColor = (color)dword_72B13C;
                    x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth + classColWidth;
                    y = (sdword)HIDWORD(tabStatsWindowPos) + 70 + rowOff;
                    right  = (udword)(x + fontWidth(label));
                    bottom = (udword)(y + fontHeight(" "));
                    if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
                    if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
                    if (tabStatsDrawEnabled) fontPrint(x, y, textColor, label);

                    /* col 3: damage value (special-cased for non-combat ship classes) */
                    if (SEgetShipClass((Ship *)slotPtr) == 18 || SEgetShipClass((Ship *)slotPtr) == 19)
                    {
                        sprintf(label, dword_BB5034);
                    }
                    else
                    {
                        real32 baseDamage = *(real32 *)(&staticInfo[6]); /* field at offset 24 */
                        real32 dmgMult    = univGetDamageMultiplier(shipClass, 0);
                        real64 dmgMod     = tacticsGetShipDamageModifier(ship);
                        sprintf(label, "%.0f", dmgMod * (real64)dmgMult * (real64)baseDamage);
                    }
                    x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth + typeColWidth;
                    y = (sdword)HIDWORD(tabStatsWindowPos) + 70 + rowOff;
                    right  = (udword)(x + fontWidth(label));
                    bottom = (udword)(y + fontHeight(" "));
                    if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
                    if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
                    if (tabStatsDrawEnabled) fontPrint(x, y, (color)-1, label);

                    visibleIdx = nextVisibleIdx;
                    visibleRow = nextVisibleIdx;
                }
            }
        }

        /* draw column headers if any slots were rendered */
        if (visibleIdx > 0)
        {
            sdword headerRowOff = (sdword)((real64)rowH * dbl_72B1E0);

            x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth;
            y = (sdword)HIDWORD(tabStatsWindowPos) + 70;
            right  = (udword)(x + fontWidth((char *)dword_BB4FCC));
            bottom = (udword)(y + fontHeight(" "));
            if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
            if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
            if (tabStatsDrawEnabled) fontPrint(x, y, (color)-1, (char *)dword_BB4FCC);

            x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth + labelColWidth;
            y = (sdword)HIDWORD(tabStatsWindowPos) + 70 - headerRowOff;
            right  = (udword)(x + fontWidth((char *)dword_BB4FD0));
            bottom = (udword)(y + fontHeight(" "));
            if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
            if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
            if (tabStatsDrawEnabled) fontPrint(x, y, (color)-1, (char *)dword_BB4FD0);

            x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth + classColWidth;
            y = (sdword)HIDWORD(tabStatsWindowPos) + 70 - headerRowOff;
            right  = (udword)(x + fontWidth((char *)dword_BB4FD4));
            bottom = (udword)(y + fontHeight(" "));
            if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
            if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
            if (tabStatsDrawEnabled) fontPrint(x, y, (color)-1, (char *)dword_BB4FD4);

            x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth + typeColWidth;
            y = (sdword)HIDWORD(tabStatsWindowPos) + 70 - headerRowOff;
            right  = (udword)(x + fontWidth((char *)dword_BB4FD8));
            bottom = (udword)(y + fontHeight(" "));
            if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
            if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
            result = tabStatsDrawEnabled;
            if (tabStatsDrawEnabled)
                return fontPrint(x, y, (color)-1, (char *)dword_BB4FD8);
            return result;
        }
    }

    /* sRammingFrigate: show whether the ramming upgrade is installed */
    if (ship->shiptype == sRammingFrigate)
    {
        bool32 hasUpgrade    = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_ION);
        char  *rammingLabel  = (char *)(hasUpgrade ? dword_BB5010 : dword_BB500C);
        x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth;
        y = (sdword)HIDWORD(tabStatsWindowPos) + 70;
        right  = (udword)(x + fontWidth(rammingLabel));
        bottom = (udword)(y + fontHeight(" "));
        if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
        if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
        result = tabStatsDrawEnabled;
        if (tabStatsDrawEnabled)
            return fontPrint(x, y, (color)-16711936, rammingLabel);
        return result;
    }

    /* no weapons or upgrade slots */
    x = (sdword)LODWORD(tabStatsWindowPos) + tabStatsColumnWidth;
    y = (sdword)HIDWORD(tabStatsWindowPos) + 70;
    right  = (udword)(x + fontWidth((char *)dword_BB4FDC));
    bottom = (udword)(y + fontHeight(" "));
    if (right  > (udword)tabStatsWindowRight)  tabStatsWindowRight  = (sdword)(right  + 10);
    if (bottom > (udword)tabStatsWindowBottom) tabStatsWindowBottom = (sdword)(bottom + 10);
    result = tabStatsDrawEnabled;
    if (tabStatsDrawEnabled)
        return fontPrint(x, y, (color)-1, (char *)dword_BB4FDC);
    return result;
}
