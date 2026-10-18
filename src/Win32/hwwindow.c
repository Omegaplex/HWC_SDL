/*=============================================================================
    Name    : hwwindow.c
    Purpose : Window creation and display mode management for Cataclysm.
              Decompiled from cataclysm.exe via IDA Hex-Rays.
=============================================================================*/

#include <windows.h>
#include "types.h"
#include "main.h"
#include "debug.h"

/*=============================================================================
    Data
=============================================================================*/
static sdword hwWidth = 0;
static sdword hwHeight = 0;
static sdword hwDepth = 0;
static bool32 hwSetResResult = 0;

extern bool32 fullScreen;
extern bool32 mainDirectDraw;

/*=============================================================================
    Functions
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : hwSetRes
    Description : Set display resolution via ChangeDisplaySettings
    Inputs      : width, height, depth (0 = restore default)
    Outputs     :
    Return      : TRUE on success
-----------------------------------------------------------------------------*/
bool32 hwSetRes(sdword width, sdword height, sdword depth)
{
    DEVMODE devMode;
    sdword iModeNum;

    if (width == -1)
    {
        width = hwWidth;
        height = hwHeight;
        depth = hwDepth;
    }

    if (width == 0)
    {
        return (ChangeDisplaySettingsA(NULL, 0) == DISP_CHANGE_SUCCESSFUL);
    }

    // Try to find an exact mode match
    iModeNum = 0;
    while (EnumDisplaySettingsA(NULL, iModeNum, &devMode))
    {
        if ((sdword)devMode.dmBitsPerPel == depth &&
            (sdword)devMode.dmPelsWidth == width &&
            (sdword)devMode.dmPelsHeight == height)
        {
            if (ChangeDisplaySettingsA(&devMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
            {
                hwWidth = width;
                hwHeight = height;
                hwDepth = depth;
                return TRUE;
            }
        }
        iModeNum++;
    }

    // No exact match found, try forcing
    EnumDisplaySettingsA(NULL, iModeNum - 1, &devMode);
    devMode.dmBitsPerPel = depth;
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if (ChangeDisplaySettingsA(&devMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
    {
        hwWidth = width;
        hwHeight = height;
        hwDepth = depth;
        return TRUE;
    }

    // Try without depth
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    if (ChangeDisplaySettingsA(&devMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
    {
        hwWidth = width;
        hwHeight = height;
        hwDepth = depth;
        return TRUE;
    }

    ChangeDisplaySettingsA(NULL, 0);
    hwDepth = 0;
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : hwCreateWindow
    Description : Create game window or set fullscreen mode
    Inputs      : hWnd, width, height, depth
    Outputs     :
    Return      : TRUE on success
-----------------------------------------------------------------------------*/
bool32 hwCreateWindow(sdword hWnd, sdword width, sdword height, sdword depth)
{
    if (fullScreen)
    {
        hwSetResResult = hwSetRes(width, height, depth);
        return hwSetResResult;
    }
    else
    {
        hwSetResResult = FALSE;
        hwDepth = 0;
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : hwDeleteWindow
    Description : Restore display mode
    Inputs      :
    Outputs     :
    Return      : TRUE
-----------------------------------------------------------------------------*/
bool32 hwDeleteWindow(void)
{
    if (hwSetResResult)
    {
        hwSetResResult = FALSE;
        return hwSetRes(0, 0, 0);
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : hwActivate
    Description : Handle window activation
    Inputs      : bActivate
    Outputs     :
    Return      : TRUE
-----------------------------------------------------------------------------*/
bool32 hwActivate(bool32 bActivate)
{
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x555950
unsigned int hwGetDepth(void)
{
    return hwDepth;
}

#if 0  // Cat2: Binary hwGetDepth; live code uses typed sdword return (0x555950)
/*=== hwGetDepth (0x555950) ===*/
int hwGetDepth()
{
  return hwDepth;
}
#endif
