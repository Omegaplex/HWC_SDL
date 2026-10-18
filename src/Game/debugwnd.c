/*=============================================================================
    Name    : debugwnd.c
    Purpose : Decompiled functions from cataclysm.exe (debugwnd module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x555D70
LRESULT __stdcall dbwWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    struct tagRECT Rect;

    switch ( Msg )
    {
        case 2u:
            return 0;
        case 3u:
            GetWindowRect(hWnd, &Rect);
            Y = Rect.top;
            X = Rect.left;
            return 0;
        case 7u:
            SetFocus(ghMainWindow);
            return DefWindowProcA(hWnd, Msg, wParam, lParam);
        case 0xFu:
            dbwAllPanesRepaint();
            ValidateRect(dword_B3A834, 0);
            return 0;
        default:
            return DefWindowProcA(hWnd, Msg, wParam, lParam);
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x555E30
bool32 dbwFindString(char *fileName, char *keyString, char *defaultString, char *returnString, sdword length)
{
    size_t keyLen;
    FILE *f;
    FILE *fp;
    char *valuePtr;
    char ch;
    char line[1024];

    keyLen = strlen(keyString);
    f = fopen(fileName, "rt");
    fp = f;
    if ( !f )
        goto notfound;
    if ( !fgets(line, 1024, f) )
    {
closefile:
        fclose(fp);
notfound:
        strcpy(returnString, defaultString);
        return 0;
    }
    while ( 1 )
    {
        if ( !strncmp(line, keyString, keyLen) )
        {
            valuePtr = &line[keyLen];
            if ( strchr(" \t=", line[keyLen]) )
            {
                ch = *valuePtr;
                if ( *valuePtr )
                    break;
            }
        }
        if ( !fgets(line, 1024, fp) )
            goto closefile;
    }
    while ( strchr(" \t=", ch) )
    {
        ch = *++valuePtr;
        if ( !ch )
            goto closefile;  // skip lines where key has no value after whitespace
    }
    fclose(fp);
    strncpy(returnString, valuePtr, length);
    returnString[length - 1] = 0;
    if ( strchr(returnString, '\n') )
        *strchr(returnString, '\n') = 0;
    return 1;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x555F80
bool32 dbwFindBinary(char *fileName, char *keyString, ubyte *buffer, udword length)
{
    size_t keyLen;
    FILE *f;
    FILE *fp;
    char *valuePtr;
    char ch;
    unsigned int i;
    char c1;
    int lowNybble;
    char c0;
    char c0upper;
    int highNybble;
    char c1upper;
    char line[1024];

    keyLen = strlen(keyString);
    f = fopen(fileName, "rt");
    fp = f;
    if ( !f )
        return 0;
    if ( !fgets(line, 1024, f) )
    {
closefile:
        fclose(fp);
        return 0;
    }
    while ( 1 )
    {
        if ( !strncmp(line, keyString, keyLen) )
        {
            valuePtr = &line[keyLen];
            if ( strchr(" \t=", line[keyLen]) )
            {
                ch = *valuePtr;
                if ( *valuePtr )
                    break;
            }
        }
        if ( !fgets(line, 1024, fp) )
            goto closefile;
    }
    while ( strchr(" \t=", ch) )
    {
        ch = *++valuePtr;
        if ( !ch )
            goto closefile;
    }
    fclose(fp);
    if ( strlen(valuePtr) <= 2 * length )
        return 0;
    i = 0;
    if ( length )
    {
        while ( 1 )
        {
            c1 = valuePtr[1];
            if ( c1 >= '0' && c1 <= '9' )
            {
                lowNybble = c1 - '0';
            }
            else
            {
                c1upper = toupper(c1);
                if ( c1upper >= 'A' && c1upper <= 'F' )
                    lowNybble = c1upper - 55;
                else
                    lowNybble = -1;
            }
            c0 = *valuePtr;
            if ( *valuePtr >= '0' && c0 <= '9' )
            {
                highNybble = c0 - '0';
            }
            else
            {
                c0upper = toupper(c0);
                if ( c0upper >= 'A' && c0upper <= 'F' )
                    highNybble = c0upper - 55;
                else
                    highNybble = -1;
            }
            if ( lowNybble < 0 || highNybble < 0 )
                break;
            buffer[i] = lowNybble + 16 * highNybble;
            ++i;
            valuePtr += 2;
            if ( i >= length )
                return 1;
        }
        return 0;
    }
    return 1;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5566F0
void dbwPaneClear(sdword pane)
{
    int result;
    int i;

    result = dbwEnabled;
    if ( dbwEnabled )
    {
        result = 5 * pane;
        if ( (dbwPane[11 * pane].flags & 1) != 0 )
        {
            dbwPane[11 * pane].cursorX = 0;
            dbwPane[11 * pane].cursorY = 0;
            memset((void *)dbwPane[11 * pane].buffer, 0x20u, dbwPane[11 * pane].width * dbwPane[11 * pane].scrollHeight);
            result = dbwPane[11 * pane].height;
            for ( i = 0; i < result; ++i )
            {
                dbwPane[11 * pane].scrollY = i;
                dbwCharsPrint(pane, (LPCSTR)dbwPane[11 * pane].buffer, dbwPane[11 * pane].width, 0);
                result = dbwPane[11 * pane].height;
            }
            dbwPane[11 * pane].scrollY = 0;
        }
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x556680
void dbwClose(void)
{
    int result;
    int *pane;

    result = dbwEnabled;
    if ( dbwEnabled )
    {
        pane = dbwPane;
        do
        {
            if ( (*(_BYTE *)pane & 1) != 0 )
            {
                free((void *)pane[9]);
                *pane &= ~1u;
            }
            pane += 11;
        }
        while ( (int)pane < (int)&dbwCharWidth );
        DeleteObject(ho);
        ReleaseDC(dbwHWnd, hDC);
        result = DestroyWindow(dbwHWnd);
        dbwEnabled = 0;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x556570
void dbwWriteOptions(void)
{
  FILE *result; // eax
  FILE *v1; // edi
  LOGFONTA *v2; // esi
  int v3; // ebx
  int v4; // ebp
  int *v5; // esi

  result = (FILE *)dword_A70FC4;
  if ( dword_A70FC4 )
  {
    result = fopen(aCataclysmCfg, aAt);
    v1 = result;
    if ( result )
    {
      fprintf(result, "%s    %d, %d\n", aLocation, X, Y);
      fprintf(v1, "%s    %d, %d\n", aSize, dword_B3A844, dword_B3A734);
      fprintf(v1, "%s    ", aFont);
      v2 = &lf;
      v3 = 60;
      do
      {
        fprintf(
          v1,
          "%c%c",
          *((char *)&dword_8B6FE0 + (LOBYTE(v2->lfHeight) >> 4)),
          *((char *)&dword_8B6FE0 + (v2->lfHeight & 0xF)));
        v2 = (LOGFONTA *)((char *)v2 + 1);
        --v3;
      }
      while ( v3 );
      fprintf(v1, "\n");
      v4 = 0;
      v5 = dword_B3A7A0;
      do
      {
        if ( (*(_BYTE *)(v5 - 8) & 1) != 0 )
          fprintf(v1, "%s%d    %d, %d, %d, %d, %d\n\n", aPane, v4, *(v5 - 7), *(v5 - 6), *(v5 - 5), *(v5 - 4), *v5);
        v5 += 11;
        ++v4;
      }
      while ( (int)v5 < (int)&binkFrameAdd );
      return (FILE *)fclose(v1);
    }
  }
  return result;
}
