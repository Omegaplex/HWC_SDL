/*=============================================================================
    Name    : TimeoutTimer.c
    Purpose : Implementation of Timeout Timers

    Created 98/11/13 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

// unfortunately need this for the timers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "types.h"
#include "qpcfix.h"
#include "TimeoutTimer.h"
#include "utility.h"

extern LONGLONG utyTimerDivisor;

void TTimerInit(TTimer *timer)
{
    timer->enabled = FALSE;
}

void TTimerClose(TTimer *timer)
{
    timer->enabled = FALSE;
}

void TTimerDisable(TTimer *timer)
{
    timer->enabled = FALSE;
}

bool32 TTimerUpdate(TTimer *timer)
{
    LARGE_INTEGER perftimer;
    LONGLONG difference;
    udword nTicks;

    if (!timer->enabled)
    {
        return FALSE;
    }

    if (timer->timedOut)
    {
        return TRUE;
    }

    catQueryPerformanceCounter(&perftimer);

    difference = perftimer.QuadPart - timer->timerLast;
    nTicks = (udword)(difference / utyTimerDivisor);

    if (nTicks >= timer->timeoutTicks)
    {
        timer->timedOut = TRUE;
        return TRUE;
    }

    return FALSE;
}

bool32 TTimerIsTimedOut(TTimer *timer)
{
    if (!timer->enabled)
    {
        return FALSE;
    }

    return timer->timedOut;
}

void TTimerReset(TTimer *timer)
{
    LARGE_INTEGER perftimer;

    if (!timer->enabled)
    {
        return;
    }

    catQueryPerformanceCounter(&perftimer);

    timer->timedOut = FALSE;
    timer->timerLast = perftimer.QuadPart;
}

void TTimerStart(TTimer *timer,real32 timeout)
{
    LARGE_INTEGER perftimer;

    catQueryPerformanceCounter(&perftimer);

    timer->enabled = TRUE;
    timer->timedOut = FALSE;
    timer->timerLast = perftimer.QuadPart;
    timer->timeoutTicks = (udword) (timeout * UTY_TimerResloutionMax);
}

void GetRawTime(sqword *time)
{
    catQueryPerformanceCounter((LARGE_INTEGER *)time);
}
