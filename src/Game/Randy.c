/*=============================================================================
    Name    : Randy.c
    Purpose : Generic random-number generators with multiple streams

    Created 11/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

// unfortunately need this for system time
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <stdio.h>

// Force release-mode random functions (debug caller tracking not in retail binary)
#define HW_Release

#include "types.h"
#include "debug.h"
#include "randy.h"
#include "universe.h"
#include "mainswitches.h"

#undef HW_Release

#pragma intrinsic(_ReturnAddress)

void aisParityLogChannel(const char *channel, const char *format, ...);

#if RAN_DEBUG_CALLER
#include "SaveGame.h"
#include "universe.h"
#include "file.h"
#include <stdio.h>
#endif

/*=============================================================================
    Data:
=============================================================================*/
ranstream ranStream[RAN_NumberStreams];
static bool32 ranTraceLogging = FALSE;

#if RAN_DEBUG_CALLER
#define RAN_LogFile             "Randylog.txt"
#define RAN_LogBufferLength     512
#define RAN_LogStringLength     24
bool32 ranCallerDebug = FALSE;
bool32 ranLogCleared = FALSE;
typedef struct
{
    char file[RAN_LogStringLength];
    sdword line;
    udword univCounter;
}
ranlogentry;
ranlogentry *ranLogBuffer = NULL;
sdword ranLogIndex = 0;
#endif

/* Non-debug builds (HW_Release) see ranRandomFnSimple as a macro for ranRandomFn,
   and ranCallerDebug is not defined. Randy.c forces HW_Release before including
   Randy.h, so this translation unit always compiles with RAN_DEBUG_CALLER=0.
   Provide the definitions here for callers that see RAN_DEBUG_CALLER=1. */
#if !RAN_DEBUG_CALLER
bool32 ranCallerDebug = FALSE;
#undef ranRandomFnSimple
udword ranRandomFnSimple(sdword ranIndex) { return ranRandomFn(ranIndex); }
#endif

/*=============================================================================
    Functions:
=============================================================================*/
enum
{
    RAN_TRACE_MAX_STACK = 8
};

static bool32 ranTraceShouldLog(sdword ranIndex)
{
    sdword frame;

    if (!ranTraceEnabled || ranTraceLogging)
    {
        return FALSE;
    }
    if (ranTraceStreamFilter != -1 && ranIndex != ranTraceStreamFilter)
    {
        return FALSE;
    }

    frame = (sdword)universe.univUpdateCounter;
    if (frame < ranTraceFrameStart)
    {
        return FALSE;
    }
    if (ranTraceFrameEnd >= 0 && frame > ranTraceFrameEnd)
    {
        return FALSE;
    }
    return TRUE;
}

static void ranTraceBuildStackString(char *buffer, size_t bufferSize, void **stackBase)
{
    void *frames[RAN_TRACE_MAX_STACK];
    USHORT captured;
    USHORT index;
    size_t offset;

    if (bufferSize == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (ranTraceStackDepth <= 0)
    {
        return;
    }

    captured = CaptureStackBackTrace(3, (DWORD)min(ranTraceStackDepth, RAN_TRACE_MAX_STACK), frames, NULL);
    offset = 0;
    for (index = 0; index < captured && offset < bufferSize; index++)
    {
        int written;

        written = snprintf(
            buffer + offset,
            bufferSize - offset,
            "%s0x%08X",
            index == 0 ? "" : ",",
            (unsigned int)(uintptr_t)frames[index]);
        if (written <= 0)
        {
            break;
        }
        offset += (size_t)written;
        if (offset >= bufferSize)
        {
            buffer[bufferSize - 1] = '\0';
            break;
        }
    }

    if (captured < (USHORT)min(ranTraceStackDepth, RAN_TRACE_MAX_STACK) && stackBase != NULL && offset < bufferSize)
    {
        USHORT rawCount = (USHORT)(min(ranTraceStackDepth, RAN_TRACE_MAX_STACK) - captured);
        if (rawCount > 6)
        {
            rawCount = 6;
        }

        for (index = 0; index < rawCount && offset < bufferSize; index++)
        {
            int written;

            written = snprintf(
                buffer + offset,
                bufferSize - offset,
                "%sesp[%u]=0x%08X",
                offset == 0 ? "" : ",",
                (unsigned int)index,
                (unsigned int)(uintptr_t)stackBase[index]);
            if (written <= 0)
            {
                break;
            }
            offset += (size_t)written;
            if (offset >= bufferSize)
            {
                buffer[bufferSize - 1] = '\0';
                break;
            }
        }
    }
}

static void ranTraceLogDraw(sdword ranIndex, udword value, void *caller, void **stackBase)
{
    char stackText[160];
    char callerText[32];
    udword x, y, z, c, n;

    if (!ranTraceShouldLog(ranIndex))
    {
        return;
    }

    ranTraceLogging = TRUE;
    ranParametersGet(ranIndex, &x, &y, &z, &c, &n);
    ranTraceBuildStackString(stackText, sizeof(stackText), stackBase);
    if (caller != NULL)
    {
        snprintf(callerText, sizeof(callerText), "0x%08X", (unsigned int)(uintptr_t)caller);
    }
    else
    {
        snprintf(callerText, sizeof(callerText), "0x00000000");
    }
    aisParityLogChannel(
        "rng",
        "event=draw stream=%d caller=%s value=%u x=%u y=%u z=%u c=%u n=%u%s%s",
        ranIndex,
        callerText,
        (unsigned int)value,
        (unsigned int)x,
        (unsigned int)y,
        (unsigned int)z,
        (unsigned int)c,
        (unsigned int)n,
        stackText[0] != '\0' ? " stack=" : "",
        stackText);
    ranTraceLogging = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : ranRandom
    Description : Get a random number from a random number sequence.
    Inputs      : ranIndex - index of random number sequence.
                  file, line - where it was called from
    Outputs     :
    Return      : New random number.
----------------------------------------------------------------------------*/
#if RAN_DEBUG_CALLER
udword ranRandomFn(sdword ranIndex, char *file, sdword line)
#else
udword ranRandomFn(sdword ranIndex)
#endif
{
    long int s;
    void **stackBase;
    void *caller;
    ranstream *stream;

#if RAN_DEBUG_CALLER
    char *truncatedFile;
    FILE *fp;

    if ((ranCallerDebug || logEnable == LOG_ON || logEnable == LOG_VERBOSE) && ranIndex == RAN_Game && file != NULL)
    {
        if (ranLogBuffer == NULL)
        {
            ranLogBuffer = memAlloc(sizeof(ranlogentry) * RAN_LogBufferLength, "ranLogBuffer", NonVolatile);
        }
        if (ranLogIndex >= RAN_LogBufferLength)
        {
            fp = fopen(RAN_LogFile, ranLogCleared ? "at" : "wt");
            if (fp != NULL)
            {
                ranLogCleared = TRUE;
                for (ranLogIndex = 0; ranLogIndex < RAN_LogBufferLength; ranLogIndex++)
                {
                    fprintf(fp, "%s(%d): %d\n", ranLogBuffer[ranLogIndex].file, ranLogBuffer[ranLogIndex].line, ranLogBuffer[ranLogIndex].univCounter);
                }
                fclose(fp);
            }
            ranLogIndex = 0;
        }
        truncatedFile = file + strlen(file) - (RAN_LogStringLength - 1);
        strcpy(ranLogBuffer[ranLogIndex].file, max(file, truncatedFile));
        ranLogBuffer[ranLogIndex].line = line;
        ranLogBuffer[ranLogIndex].univCounter = universe.univUpdateCounter;
        ranLogIndex++;
    }
#endif
    dbgAssert(ranIndex < RAN_NumberStreams);
    stackBase = (void **)_AddressOfReturnAddress();
    caller = _ReturnAddress();
    stream = &ranStream[ranIndex];
    if(stream->y>stream->x+stream->c){s=stream->y-(stream->x+stream->c);stream->c=0;}
    else {s=(stream->x+stream->c)-stream->y-18;stream->c=1;}
    stream->x=stream->y; stream->y=stream->z;
    s = ((stream->z=s) + (stream->n=69069*stream->n+1013904243));
    ranTraceLogDraw(ranIndex, (udword)s, caller, stackBase);
    return (udword)s;
}

/*-----------------------------------------------------------------------------
    Name        : ranRandomFnSimple
    Description : A special version of the random number generator with no file
                    or line parmeters
    Inputs      :ranIndex - index of random number sequence.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RAN_DEBUG_CALLER
udword ranRandomFnSimple(sdword ranIndex)
{
    return(ranRandomFn(ranIndex, NULL, 0));
}
#endif
/*-----------------------------------------------------------------------------
    Name        : ranNumberGet
    Description : Get position in random-number sequence.
    Inputs      : ranIndex - index of random number sequence.
    Outputs     :
    Return      : Current random stream position for this stream.
----------------------------------------------------------------------------*/
udword ranNumberGet(sdword ranIndex)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    return(ranStream[ranIndex].n);
}

/*-----------------------------------------------------------------------------
    Name        : ranNumberSet
    Description : Set position in random-number sequence.
    Inputs      : ranIndex - index of random number sequence.
                  nn - random number to start at.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranNumberSet(sdword ranIndex, udword nn)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    ranStream[ranIndex].n=nn;
}

/*-----------------------------------------------------------------------------
    Name        : ranParametersSet
    Description : Set the parameters used to define a random-number sequence.
                    These will have defaults.
    Inputs      : ranIndex - index of random number sequence.
                  xx, yy, zz, cc, nn - parameters for the random stream
    Outputs     : stores parameters for later usage
    Return      : void
----------------------------------------------------------------------------*/
void ranParametersSet(sdword ranIndex, udword xx,udword yy,udword zz,udword cc,udword nn)
{
    ranStream[ranIndex].x=xx; ranStream[ranIndex].y=yy; ranStream[ranIndex].z=zz; ranStream[ranIndex].c=cc; ranStream[ranIndex].n=nn;
}

/*-----------------------------------------------------------------------------
    Name        : ranParametersGet
    Description : Get the parameters used to define a random-number sequence.
    Inputs      : ranIndex - index of random number sequence.
    Outputs     : xx, yy, zz, cc, nn - parameters for the random stream
    Return      : void
----------------------------------------------------------------------------*/
void ranParametersGet(sdword ranIndex, udword *xx,udword *yy,udword *zz,udword *cc,udword *nn)
{
    *xx = ranStream[ranIndex].x; *yy = ranStream[ranIndex].y; *zz = ranStream[ranIndex].z; *cc = ranStream[ranIndex].c; *nn = ranStream[ranIndex].n;
}

/*-----------------------------------------------------------------------------
    Name        : ranParametersReset
    Description : Reset a stream's parameters to default values.
    Inputs      : ranIndex - index of random number sequence.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranParametersReset(sdword ranIndex)
{
    dbgAssert(ranIndex < RAN_NumberStreams);
    ranStream[ranIndex].x=521288629;
    ranStream[ranIndex].y=362436069;
    ranStream[ranIndex].z=1613801;
    ranStream[ranIndex].c=1;
    ranStream[ranIndex].n=1131199209;
}

/*-----------------------------------------------------------------------------
    Name        : ranRandomize
    Description : randomizes the number stream based on system time
    Inputs      : ranIndex - index of random number sequence.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranRandomize(sdword ranIndex)
{
    SYSTEMTIME systime;
    ranstream *stream;
    sdword index;
    uword iter;

    for (index = 0; index < RAN_NumberStreams; index++)
    {
        ranParametersReset(index);
    }

    GetSystemTime(&systime);
    iter = (uword)(systime.wSecond & 0xFF);
    if (iter == 0)
    {
        return;
    }

    stream = &ranStream[0];
    do
    {
        sdword s;

        if (stream->y > stream->x + stream->c)
        {
            s = stream->y - (stream->x + stream->c);
            stream->c = 0;
        }
        else
        {
            s = (stream->x + stream->c) - stream->y - 18;
            stream->c = 1;
        }
        stream->x = stream->y;
        stream->y = stream->z;
        stream->z = s;
        stream->n = 69069 * stream->n + 1013904243;
        iter--;
    }
    while (iter != 0);
}

void ranRandomFn_0(sdword ranIndex)
{
    SYSTEMTIME systime;
    udword iter;
    ranstream *stream;

    dbgAssert(ranIndex < RAN_NumberStreams);

    GetSystemTime(&systime);
    iter = systime.wSecond & 0xFF;
    stream = &ranStream[ranIndex];

    while (iter != 0)
    {
        long int s;

        if (stream->y > stream->x + stream->c)
        {
            s = stream->y - (stream->x + stream->c);
            stream->c = 0;
        }
        else
        {
            s = (stream->x + stream->c) - stream->y - 18;
            stream->c = 1;
        }

        stream->x = stream->y;
        stream->y = stream->z;
        stream->z = s;
        stream->n = 69069 * stream->n + 1013904243;
        iter--;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ranStartup
    Description : Startup random module
    Inputs      :
    Outputs     : inits all the random streams to default values
    Return      :
----------------------------------------------------------------------------*/
void ranStartup(void)
{
    sdword ranIndex;

    for (ranIndex = 0; ranIndex < RAN_NumberStreams; ranIndex++)
    {
        ranParametersReset(ranIndex);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ranShutdown
    Description : Shuts down the random module, currently does nothing
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ranShutdown(void)
{
#if RAN_DEBUG_CALLER
    FILE *fp;
    sdword index;

    if (ranLogBuffer != NULL)
    {
        fp = fopen(RAN_LogFile, ranLogCleared ? "at" : "wt");
        if (fp != NULL)
        {
            ranLogCleared = TRUE;
            for (index = 0; index < ranLogIndex; index++)
            {
                fprintf(fp, "%s(%d): %d\n", ranLogBuffer[index].file, ranLogBuffer[index].line, ranLogBuffer[index].univCounter);
            }
            fclose(fp);
        }
        ranLogIndex = 0;
        memFree(ranLogBuffer);
    }

#endif
    ;
}

/*=============================================================================
    Save Game stuff
=============================================================================*/

void ranSave(void)
{
    sdword ranIndex;

    for (ranIndex = 0; ranIndex < RAN_NumberStreams; ranIndex++)
    {
        SaveStructureOfSize(&ranStream[ranIndex],sizeof(ranstream));
    }
}

void ranLoad(void)
{
    sdword ranIndex;

    for (ranIndex = 0; ranIndex < RAN_NumberStreams; ranIndex++)
    {
        LoadStructureOfSizeToAddress(&ranStream[ranIndex],sizeof(ranstream));
    }
}


#if 0  // duplicate of above
// ============================================================================
// Decompiled from cataclysm.exe @ 0x4EFB50
void ranSave(void)
{
    int *stream;
    unsigned int result;

    stream = ranStream;
    do
    {
        result = SaveStructureOfSize(stream, 20);
        stream += 5;
    }
    while ( (int)stream < (int)&ranStreamEnd );
    return result;
}
#endif

#if 0  // duplicate of above
// ============================================================================
// Decompiled from cataclysm.exe @ 0x4EFB70
void ranLoad(void)
{
    ubyte *stream;
    unsigned int result;

    stream = (ubyte *)ranStream;
    do
    {
        result = LoadStructureOfSizeToAddress(stream, 0x14u);
        stream += 20;
    }
    while ( (int)stream < (int)&ranStreamEnd );
    return result;
}
#endif
