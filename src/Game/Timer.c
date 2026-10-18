#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Timer.h"
#include "memory.h"
#include "SaveGame.h"
#include "task.h"
#include "universe.h"

//
//  Created 1997/07/01  Darren Stone
//

//
//  support for named, low-resolution timers
//  (specified in seconds, based on universe time)
//
//  the named referencing of these timers make using
//  them a little slower, but friendly for scripting, etc.
//

sdword timersAllocated = 0;
sdword timersUsed = 0;
NamedTimer *timers = NULL;

// Scope ID used by string-named timers (kasfTimer* wrappers)
#define TIMER_STRING_SCOPE 0

static sdword timHashName(const char *name)
{
    sdword hash = 5381;
    unsigned char c;
    while ((c = (unsigned char)*name++) != 0)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static NamedTimer *timTimerFind(const char *name)
{
    sdword id = timHashName(name);
    sdword i;

    for (i = 0; i < timersUsed; ++i)
        if (timers[i].scopeId == TIMER_STRING_SCOPE && timers[i].timerId == id)
            return timers + i;

    return NULL;
}

void timTimerStart(char *name)
{
    NamedTimer *tp;

    tp = timTimerFind(name);
    if (!tp)
        return;

    if (tp->duration > 0)
    {
        tp->startTime = universe.totaltimeelapsed;
        tp->enabled = 1;
        tp->bPaused = FALSE;
    }
    else
        tp->enabled = 0;
}

void timTimerStop(char *name)
{
    NamedTimer *tp;

    tp = timTimerFind(name);
    if (!tp)
        return;

    tp->duration = (real32)timTimerRemaining(name);
    tp->enabled = 0;
}

sdword timTimerExpired(char *name)
{
    if (!timTimerFind(name))
        return 0;  // non-existent timers haven't "expired"

    return !timTimerRemaining(name);
}

sdword timTimerExpiredDestroy(char *name)
{
    if (!timTimerFind(name))
        return 0;  // non-existent timers haven't "expired"

    if (!timTimerRemaining(name))
    {
       timTimerDestroy(name);
       return 1;
    }
    else
       return 0;
}

sdword timTimerRemaining(char *name)
{
    NamedTimer *tp;
    sdword rem;

    tp = timTimerFind(name);
    if (!tp)
        return 0;
    if (tp->bPaused)
    {
        return(SDWORD_Max);
    }
    if ((!tp->enabled))
        return 0;
    rem = (sdword)(tp->duration - (universe.totaltimeelapsed - tp->startTime));
    if (rem <= 0)
    {
        tp->enabled = 0;
        rem = 0;
    }
    return rem;
}

void timTimerSet(char *name, sdword duration)
{
    NamedTimer *tp;

    tp = timTimerFind(name);
    if (!tp)
        return;

    tp->duration = (real32)duration;
}

void timTimerDestroy(char *name)
{
    sdword id = timHashName(name);
    sdword i;

    for (i = 0; i < timersUsed; ++i)
        if (timers[i].scopeId == TIMER_STRING_SCOPE && timers[i].timerId == id)
            break;

    if (i >= timersUsed)
        return;  // doesn't exist

    while (i < timersUsed-1)
    {
        memcpy(timers + i, timers + i+1, sizeof(NamedTimer));
        ++i;
    }

    --timersUsed;
}

void timTimerDestroyAll(void)
{
    if (timers)
    {
        memFree(timers);
        timers = NULL;
    }
    timersAllocated = 0;
    timersUsed = 0;
}

void timTimerCreate(char *name)
{
    if (timTimerFind(name))
        return;  // already exists

    // allocate more if necessary
    if (timersUsed >= timersAllocated)
    {
        timers = memRealloc(timers, sizeof(NamedTimer) * (timersAllocated + TIMER_ALLOC_INCREMENT), "timers", 0);
        timersAllocated += TIMER_ALLOC_INCREMENT;
    }

    timers[timersUsed].scopeId = TIMER_STRING_SCOPE;
    timers[timersUsed].timerId = timHashName(name);
    timers[timersUsed].duration = 0;
    timers[timersUsed].enabled = 0;
    timers[timersUsed].bPaused = 0;

    ++timersUsed;
}

//
//  saves having to do a create, then a set, then a start
//  with separate operations.
//
void timTimerCreateSetStart(char *name, sdword duration)
{
    timTimerCreate(name);
    timTimerSet(name, duration);
    timTimerStart(name);
}

/*-----------------------------------------------------------------------------
    Name        : timTimerPause
    Description : Pause a timer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerPause(NamedTimer *tim)
{
    dbgAssert(tim);
    tim->pauseTime = universe.totaltimeelapsed;
    tim->bPaused = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : timTimerPauseAllNotScoped
    Description : Pauses all timers that do not have the specified scope name
    Inputs      : scopeName - name of the FSM that called this function.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerPauseAllNotScoped(char *scopeName)
{
    // In Cataclysm's ID-based timer system, string-scoped timers all share
    // scopeId=TIMER_STRING_SCOPE. Pause all non-string-scope timers.
    NamedTimer *tim = timers;
    sdword index;
    (void)scopeName;

    for (index = 0; index < timersUsed; index++, tim++)
    {
        if (tim->scopeId != TIMER_STRING_SCOPE)
        {
            timTimerPause(tim);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : timTimerUnpause
    Description : Unpause a timer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerUnpause(NamedTimer *tim)
{
    tim->duration += universe.totaltimeelapsed - tim->pauseTime;
    tim->bPaused = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : timTimerUnpauseAll
    Description : Resume all previously paused timers.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void timTimerUnpauseAll(void)
{
    NamedTimer *tim = timers;
    sdword index;

    for (index = 0; index < timersUsed; index++, tim++)
    {
        if (tim->bPaused)
        {                                                   //if this timer is paused
            timTimerUnpause(tim);
        }
    }
}

/*=============================================================================
    Save Game Stuff below:
=============================================================================*/

void timTimerSave(void)
{
    SaveInfoNumber(timersAllocated);
    SaveInfoNumber(timersUsed);

    if (timersAllocated > 0)
    {
        dbgAssert(timers);
        // just number data, no pointers in Timer so we can just save this way
        SaveStructureOfSize(timers,timersAllocated * sizeof(NamedTimer));
    }
}

void timTimerLoad(void)
{
    timersAllocated = LoadInfoNumber();
    timersUsed = LoadInfoNumber();

    if (timersAllocated > 0)
    {
        timers = LoadStructureOfSize(timersAllocated * sizeof(NamedTimer));
    }
    else
    {
        timers = NULL;
    }
}


typedef Timer TaskTimer;

void *TimerInitTaskTimerStruct(void *timer, real32 duration)
{
    TaskTimer *taskTimer = (TaskTimer *)timer;

    taskTimer->duration = duration;
    taskTimer->pauseTime = 1234567890.0f;
    taskTimer->startTime = taskTimeElapsedGame;
    taskTimer->enabled = TRUE;
    return timer;
}

Timer *TimerInitTimerStruct(Timer *timer, real32 duration)
{
    timer->duration = duration;
    timer->pauseTime = 1234567890.0f;
    timer->startTime = universe.totaltimeelapsed;
    timer->enabled = FALSE;
    return timer;
}

// Cataclysm (0x528470): return elapsed time since timer started
real32 TimerElapsed(Timer *timer)
{
    return (timer->enabled ? taskTimeElapsedGame : universe.totaltimeelapsed) - timer->startTime;
}

bool32 TimerExpired(int timerAddress)
{
    Timer *timer = (Timer *)timerAddress;
    real32 currentTime = timer->enabled ? taskTimeElapsedGame : universe.totaltimeelapsed;

    return (currentTime - timer->startTime) >= timer->duration;
}

real64 TimerRemaining(int timerAddress)
{
    Timer *timer = (Timer *)timerAddress;

    if (timer->enabled)
    {
        return timer->duration - (taskTimeElapsed - timer->startTime);
    }
    else
    {
        return timer->duration - (universe.totaltimeelapsed - timer->startTime);
    }
}

real32 TimerTimeRemaining(void *timer)
{
    TaskTimer *taskTimer = (TaskTimer *)timer;
    real32 elapsed;

    if (taskTimer->enabled)
    {
        elapsed = taskTimeElapsedGame - taskTimer->startTime;
    }
    else
    {
        elapsed = universe.totaltimeelapsed - taskTimer->startTime;
    }

    if (taskTimer->duration > 0.0f)
    {
        return elapsed / taskTimer->duration;
    }

    return 0.0f;
}

Timer *TimerSetRemaining(Timer *timer, real32 duration)
{
    real32 elapsed;
    real32 currentTime;

    currentTime = timer->enabled ? taskTimeElapsedGame : universe.totaltimeelapsed;
    elapsed = currentTime - timer->startTime;
    timer->pauseTime = 1234567890.0f;
    timer->duration = timer->duration - elapsed + duration;
    timer->startTime = universe.totaltimeelapsed;
    timer->enabled = FALSE;
    return timer;
}
