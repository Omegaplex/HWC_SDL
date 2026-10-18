/*=============================================================================
    Name    : TimeoutTimer.h
    Purpose : Definitions for TimeoutTimers

    Created 98/11/13 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___TIMEOUT_TIMER_H
#define ___TIMEOUT_TIMER_H

#include "types.h"

typedef struct TTimer
{
    bool32 enabled;
    bool32 timedOut;
    __int64 timerLast;
    udword timeoutTicks;
} TTimer;

void TTimerInit(TTimer *timer);
void TTimerClose(TTimer *timer);
void TTimerDisable(TTimer *timer);
bool32 TTimerUpdate(TTimer *timer);
void TTimerReset(TTimer *timer);
void TTimerStart(TTimer *timer,real32 timeout);
bool32 TTimerIsTimedOut(TTimer *timer);

void GetRawTime(sqword *time);

#endif

