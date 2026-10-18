#ifndef _BINK_H
#define _BINK_H

#include "types.h"

typedef void *HBINK;

typedef void (*binkDisplayCallback_proc)(void *surface, sdword pitch, sdword xOfs, sdword yOfs);
typedef void (*binkDisplay_proc)(binkDisplayCallback_proc callback);
typedef void (*binkDecodeCallback_proc)(sdword frame);
typedef void (*binkEndCallback_proc)(void);

#define S_RGB555 0
#define S_RGB565 1

extern real32 binkFrameRate;

uword *binkGetSurface(void);
bool32 binkInit(sdword rgltype);
bool32 binkCleanup(void);
bool32 binkPlay(char *filename,
                binkDisplay_proc displayProc,
                binkDecodeCallback_proc decodeProc,
                sdword surfType,
                bool32 rev,
                sdword trackNum);

#endif
