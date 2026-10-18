/*=============================================================================
    Name    : screenshot.c
    Purpose : take screenshots via OpenGL

    Created 7/15/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "debug.h"
#include "jpginterfce.h"
#include "screenshot.h"
#include "interfce.h"
#include "CommandWrap.h"

static void _numberify(char* dirname)
{
    int   i;
    FILE* outfile;
    char  filename[32], testname[128];

    filename[0] = '\0';

    for (i = 0; i < 200; i++)
    {
        sprintf(filename, "shota%03d.jpg", i);
        strcpy(testname, dirname);
        strcat(testname, filename);

        if ((outfile = fopen(testname, "rt")) == NULL)
        {
            strcat(dirname, filename);
            return;
        }

        fclose(outfile);
    }

    strcat(dirname, "shot200.jpg");
}

void ssSaveScreenshot(ubyte* buf)
{
    char* dir;
    char fname[128];
    char message[256];
    FILE* out;
    unsigned char *pTempLine;
    long Top, Bot, i, Size;

    JPEGDATA jp;

    dir = getenv("HW_Data");
    if (dir == NULL)
    {
        strcpy(fname, "screenshots\\");
    }
    else
    {
        strcpy(fname, dir);
        strcat(fname, "\\screenshots\\");
    }

    _numberify(fname);

#if SS_VERBOSE_LEVEL >= 1
    dbgMessagef("\nSaving %dx%d screenshot to '%s'.", MAIN_WindowWidth, MAIN_WindowHeight, fname);
#endif

    out = fopen(fname, "wb");
    if (out == NULL)
    {
        return;
    }

    Size = MAIN_WindowWidth*3;
    pTempLine = (unsigned char *)malloc(Size);

    for (i = 0; i < (MAIN_WindowHeight / 2); i++)
    {
        Top = i;
        Bot = (MAIN_WindowHeight - 1) - i;

        memcpy(pTempLine, buf + (Size * Top), Size);
        memcpy(buf + (Size * Top), buf + (Size * Bot), Size);
        memcpy(buf + (Size * Bot), pTempLine, Size);
    }

    free(pTempLine);

    // Fill out the JPG lib info structure
    memset(&jp, 0, sizeof(jp));

    jp.ptr = buf;
    jp.width = MAIN_WindowWidth;
    jp.height = MAIN_WindowHeight;
    jp.output_file = out;
    jp.aritcoding = 0;
    jp.quality = 97;

    JpegWrite(&jp);

    fclose(out);
    sprintf(message, "Screenshot Saved: %s", fname);
    clCommandMessage(message, -1);
}

#if 0  // Cat2: ssSaveScreenshot live code differs from binary
/*=== ssSaveScreenshot (0x57EF80) ===*/
FILE *__cdecl ssSaveScreenshot(char *a1)
{
  char *v1; // eax
  int v2; // ebx
  FILE *v3; // eax
  char *v4; // edi
  FILE *result; // eax
  FILE *v6; // esi
  unsigned int v7; // ebp
  char *v8; // eax
  GLsizei v9; // ecx
  int v10; // ebx
  char *v11; // [esp+10h] [ebp-1E0h]
  FILE *v12; // [esp+18h] [ebp-1D8h]
  int v13[13]; // [esp+1Ch] [ebp-1D4h] BYREF
  char v14[32]; // [esp+50h] [ebp-1A0h] BYREF
  char v15[128]; // [esp+70h] [ebp-180h] BYREF
  char v16[256]; // [esp+F0h] [ebp-100h] BYREF

  v1 = getenv(aHwData);
  if ( v1 )
  {
    strcpy(v15, v1);
    strcat(v15, aScreenshots_0);
  }
  else
  {
    strcpy(v15, "screenshots\\");
  }
  v14[0] = 0;
  v2 = 0;
  while ( 1 )
  {
    sprintf(v14, "shota%03d.jpg", v2);
    strcpy(v16, v15);
    strcat(v16, v14);
    v3 = fopen(v16, aRt);
    if ( !v3 )
      break;
    fclose(v3);
    if ( ++v2 >= 200 )
    {
      v4 = aShot200Jpg;
      goto LABEL_8;
    }
  }
  v4 = v14;
LABEL_8:
  strcat(v15, v4);
  result = fopen(v15, aWb);
  v6 = result;
  v12 = result;
  if ( result )
  {
    v7 = 3 * MAIN_WindowWidth;
    v8 = (char *)malloc_34(3 * MAIN_WindowWidth);
    v9 = MAIN_WindowHeight;
    v10 = 0;
    if ( MAIN_WindowHeight / 2 > 0 )
    {
      v11 = a1;
      do
      {
        qmemcpy(v8, v11, v7);
        qmemcpy(v11, &a1[v7 * (v9 - v10 - 1)], v7);
        qmemcpy(&a1[v7 * (v9 - v10 - 1)], v8, v7);
        v11 += v7;
        v9 = MAIN_WindowHeight;
        ++v10;
      }
      while ( v10 < MAIN_WindowHeight / 2 );
      v6 = v12;
    }
    free(v8);
    memset(v13, 0, sizeof(v13));
    v13[0] = (int)a1;
    *(_QWORD *)&v13[1] = __PAIR64__(MAIN_WindowHeight, MAIN_WindowWidth);
    v13[3] = (int)v6;
    v13[5] = 0;
    v13[8] = 97;
    sub_5C9DC0(v13);
    fclose(v6);
    sprintf(v16, "Screenshot Saved: %s", v15);
    return (FILE *)clCommandMessage(v16, -1);
  }
  return result;
}
#endif
