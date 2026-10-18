/*=============================================================================
    Name    : main.c
    Purpose : Main entry point for Cataclysm decompilation project.
              Minimal skeleton for build verification.
=============================================================================*/

#include <windows.h>
#include "Types.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    MessageBoxA(NULL, "Cataclysm decompilation skeleton", "Cataclysm", MB_OK);
    return 0;
}


#if 0  // Cat2: mainFreeNISTexture and mainFreeDerelictTexture live in Win32/main.c (0x57B800, 0x57B7B0)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x57B800
int mainFreeNISTexture(void)
{
    int result = nisTextureHandle;
    if (nisTextureHandle)
    {
        result = trTextureUnregister(nisTextureHandle);
        nisTextureHandle = 0;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x57B7B0
int mainFreeDerelictTexture(void)
{
    int result = derelictTextureHandle;
    if (derelictTextureHandle)
    {
        result = trTextureUnregister(derelictTextureHandle);
        derelictTextureHandle = 0;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x584C80
int mainInitRGLState(void)
{
    if (!trNoPalettes)
        palExtFunc = (void*)rwglGetProcAddress("glColorTableEXT");
    litColorTableFunc = 0;
    if (RGLtype)
    {
        maxPaletteSize = 0;
    }
    else
    {
        litColorTableFunc = rwglGetProcAddress("glLitColorTable");
        litColorTableFunc(0, 0, 0, 0, 0, &maxPaletteSize);
    }
    if (RGL)
    {
        if (cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }
    hasClipHint = glCapFeatureExists(0x81FB);
    if (hasClipHint)
        return glEnable(0x81FB);
    return hasClipHint;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4F76F0
int mainTexturesInit(void)
{
    int i = 0;
    char path[128];
    dword_A576EC = 0;
    // Initialize texture state array
    int *entry = dword_C16788;
    do { entry[-2]=0; entry[0]=-2; entry[1]=-2; entry[2]=0; entry[-1]=-1; entry+=5; }
    while ((int)entry < (int)dword_C16800);
    // Load 15 main textures from LIF files
    for (i = 0; i < 15; ++i) {
        strcpy(path, mainTexBasePath); strcat(path, mainTexSubdir); strcat(path, mainTexNames[i]);
        void *lif = trLIFFileLoad(path, 8);
        mainTexData[i] = (int)lif;
        if (lif->flags & 2) { mainTexPaletted=1; mainTexHandles[i]=trPalettedTextureCreate(lif->data,lif->palette,lif->width,lif->height); }
        else { mainTexPaletted=0; mainTexHandles[i]=trRGBTextureCreate(lif->data,lif->width,lif->height,1); }
    }
    // Load 6 secondary textures
    for (i = 0; i < 6; ++i) {
        strcpy(path, secTexBasePath); strcat(path, secTexSubdir); strcat(path, secTexNames[i]);
        void *lif = trLIFFileLoad(path, 8);
        secTexData[i] = (int)lif;
        if (lif->flags & 2) { mainTexPaletted=1; secTexHandles[i]=trPalettedTextureCreate(lif->data,lif->palette,lif->width,lif->height); }
        else { mainTexPaletted=0; secTexHandles[i]=trRGBTextureCreate(lif->data,lif->width,lif->height,1); }
    }
    dword_891F54=-1; dword_A5774C=0; dword_891F58=-1;
    mainFont1 = frFontRegister("HW_Eurosecond_11");
    mainFont2 = frFontRegister("default.hff");
    mainFont3 = frFontRegister("EurosewideHeavy");
    return mainFont3;
}


// ============================================================================
// Decompiled from cataclysm.exe @ 0x560350
HWND mainDestroyWindow(void)
{
    HWND hWindow;

    mainActuallyQuit = FALSE;
    DestroyWindow(ghMainWindow);

    mainWindowTotalWidth = MAIN_WindowWidth + mainWidthAdd;
    mainWindowTotalHeight = MAIN_WindowHeight + mainHeightAdd;

    if (DebugWindow)
        dbwClose();

    hWindow = CreateWindowExA(
        0,
        "Emergence",
        "Emergence",
        dwStyle,
        0, 0,
        mainWindowTotalWidth,
        mainWindowTotalHeight,
        NULL, NULL,
        hInstance,
        NULL);

    if (hWindow)
    {
        ShowWindow(hWindow, SW_HIDE);
        utyForceTopmost(fullScreen);
        UpdateWindow(hWindow);
        ghMainWindow = hWindow;
        if (DebugWindow)
            dbwPrint(hInstance, hWindow);
    }
    return hWindow;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x55F880
sdword ProcessCommandLine(char *commandLine)
{
  char *v1; // ebp
  int v2; // edi
  char **i; // esi
  int *v4; // eax
  char *v5; // eax

  v1 = strtok(a1, asc_8B9C4C);
  if ( v1 )
  {
    while ( 2 )
    {
      if ( *v1 == 45 )
        *v1 = 47;
      v2 = 0;
      for ( i = &off_8B8874; ; i += 6 )
      {
        if ( !*i )
          goto LABEL_18;
        if ( i[4] && !_strcmpi(v1, *i) )
          break;
        ++v2;
      }
      v4 = (&off_8B887C)[6 * v2];
      if ( v4 )
        *v4 = dword_8B8880[6 * v2];
      if ( !dword_8B8878[6 * v2] )
        goto LABEL_16;
      if ( (byte_8B8870[24 * v2] & 2) != 0 )
      {
        v5 = strtok(0, asc_8B9C4C);
        if ( !v5 )
          break;
        v1 = v5;
      }
      if ( ((int (__cdecl *)(char *))dword_8B8878[6 * v2])(v1) )
      {
LABEL_16:
        v1 = strtok(0, asc_8B9C4C);
        if ( v1 )
          continue;
        return 0;
      }
      break;
    }
LABEL_18:
    DebugHelpDefault(v1);
    return -1;
  }
  return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5602F0
void mainRegisterClass(HANDLE hInstance)
{
  WNDCLASSA WndClass; // [esp+4h] [ebp-28h] BYREF

  WndClass.style = 11;
  WndClass.lpfnWndProc = WindowProc;
  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;
  WndClass.hInstance = hInstance;
  WndClass.hIcon = LoadIconA(hInstance, (LPCSTR)0x67);
  WndClass.hCursor = 0;
  WndClass.hbrBackground = (HBRUSH)GetStockObject(4);
  WndClass.lpszMenuName = 0;
  WndClass.lpszClassName = aEmergence_0;
  return RegisterClassA(&WndClass);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x55FB90
void DeactivateMe(HWND hWnd)
{
  int result; // eax

  result = gl95;
  if ( !gl95 )
  {
    sounddeactivate(1);
    if ( RGL )
      rglFeature(4104);
    else
      hwActivate(0);
    if ( !noMinimizeAltTab && byte_AB7657 )
      ShowWindow(hWnd, 7);
    unk_A2DF24 = HIDWORD(qword_A2DF1C);
    HIDWORD(qword_A2DF1C) = 0;
    utyClipMouse(0);
    utyTaskTimerClear();
    if ( multiPlayerGame || dword_A751AC )
    {
      taskSavePauseStatus();
      taskPause(dword_AB76C8);
      taskPause(dword_C2FD6C);
    }
    else
    {
      taskFreezeAll();
    }
    keyClearAll_0();
    keyBufferClear();
    result = dword_8B6D94;
    systemActive = 0;
    if ( !dword_8B6D94 )
      return bink_551D40(1);
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x560550
void mainSetupSoftware(void)
{
  int result; // eax

  strcpy(glToSelect, "rgl.dll");
  strcpy((char *)&dword_A74E50, "rgl.dll");
  strcpy(byte_A750D0, "sw");
  strcpy((char *)&word_A74DD0, "sw");
  byte_A75050[0] = Default;
  MAIN_WindowWidth = 640;
  mainWindowWidth = 640;
  MAIN_WindowHeight = 480;
  mainWindowHeight = 480;
  MAIN_WindowDepth = 16;
  mainWindowDepth = 16;
  result = 0;
  opDeviceIndex = -1;
  gDevcaps2 = 0;
  gDevcaps = 0;
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x55FFE0
void *mainMemAlloc(unsigned int size)
{
  return memAlloc(a1, 40);
}
#endif
