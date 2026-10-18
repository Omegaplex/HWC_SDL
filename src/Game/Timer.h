#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"

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

#define TIMER_NAME_MAX_LENGTH 47

#define TIMER_ALLOC_INCREMENT 16

typedef struct {
    real32 startTime;   // 0x00
    real32 duration;    // 0x04
    real32 pauseTime;   // 0x08
    bool32 enabled;     // 0x0C: 0 = universe time, 1 = task time
} Timer;                // retail runtime/task timer, sizeof == 16

typedef struct {
    Timer timer;        // 0x00
    ubyte unk_10_17[8]; // 0x10
} TimerStorage24;       // retail 24-byte storage used by some ship specifics

typedef struct {
    sdword scopeId;     // 0x00: scope identifier (0 = string-hashed scope)
    sdword timerId;     // 0x04: timer identifier
    real32 startTime;   // 0x08
    real32 duration;    // 0x0C
    real32 pauseTime;   // 0x10
    bool8  enabled;     // 0x14
    bool8  bPaused;     // 0x15
    ubyte  pad[2];      // 0x16
} NamedTimer;           // retail named/KAS timer record, sizeof == 24

void timTimerCreate(char *name);
void timTimerStart(char *name);
void timTimerStop(char *name);
sdword timTimerExpired(char *name);
sdword timTimerExpiredDestroy(char *name);
sdword timTimerRemaining(char *name);
void timTimerSet(char *name, sdword duration);
void timTimerCreateSetStart(char *name, sdword duration);
void timTimerDestroy(char *name);
void timTimerDestroyAll(void);
void timTimerPause(NamedTimer *tim);
void timTimerPauseAllNotScoped(char *scopeName);
void timTimerUnpause(NamedTimer *tim);
void timTimerUnpauseAll(void);
Timer *TimerInitTimerStruct(Timer *timer, real32 duration);
Timer *TimerSetRemaining(Timer *timer, real32 duration);
bool32 TimerExpired(int timer);
real64 TimerRemaining(int timer);
void *TimerInitTaskTimerStruct(void *timer, real32 duration);
real32 TimerTimeRemaining(void *timer);
real32 TimerElapsed(Timer *timer);

void timTimerSave(void);
void timTimerLoad(void);

#endif
