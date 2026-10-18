/*=============================================================================
    Name    : Demo.h
    Purpose : Definitions for recording and playing back demos.

    Created 5/21/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___DEMO_H
#define ___DEMO_H

#include "types.h"
#include "key.h"
#include "task.h"
#include "randy.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define DEM_AUTO_DEMO           0               //automatically play demos if you chill on the startup screen
#define DEM_RANDY_SAVE          1               //save the randy state at demo start?
#define DEM_INTERRUPTABLE       1               //can demo playback be interrupted?

#ifndef HW_Release

#define DEM_MODULE_TEST         0               //test the module
#define DEM_ERROR_CHECKING      1               //basic error checking
#define DEM_VERBOSE_LEVEL       1               //control verbose printing
#define DEM_CHECKSUM            1               //put a checksum in the state packets
#define DEM_RANDOM_VERIFY       1               //verify that the random number stream is consistant
#define DEM_CAMERA_CHECKSUM     1
#define DEM_SELECT_CHECKSUM     1               //verify the selections (selSelected/Selecting)
#define DEM_FAKE_RENDER_SWITCH  1               //allow the turning off of fake renders

#else //HW_Debug

#define DEM_MODULE_TEST         0               //don't test the module
#define DEM_ERROR_CHECKING      0               //no error ckecking in retail
#define DEM_VERBOSE_LEVEL       0               //control verbose printing
#define DEM_CHECKSUM            0               //put a checksum in the state packets
#define DEM_RANDOM_VERIFY       0               //verify that the random number stream is consistant
#define DEM_CAMERA_CHECKSUM     0
#define DEM_SELECT_CHECKSUM     0               //verify the selections (selSelected/Selecting)
#define DEM_FAKE_RENDER_SWITCH  0               //allow the turning off of fake renders

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
#define DEM_HeaderString        "BaNng"
#define DEM_OriginalVersionNumber 0x007
#define DEM_VersionNumber       0x009
#define DEM_NoChecksum          1234567.0f
#define DEM_NoCameraChecksum    0xffffffff
#define DEM_NumberStreamsToSave RAN_NumberStreams
#define DEM_NumberTries         3               //try to play a demo this many times
#define DEM_AUTO_DELAY          30              //for this many seconds

#define DEM_NumberNames         32
#define DEM_ChecksumLength      128

#define DEM_MaxFakeRenders      6

#define DEM_LogFileName         "demolog.txt"
#define DEM_ModeB               0x00000001
#define DEM_MAKE_TAG(a,b,c,d)   ((udword)(ubyte)(a) | ((udword)(ubyte)(b) << 8) | ((udword)(ubyte)(c) << 16) | ((udword)(ubyte)(d) << 24))
#define DEM_ChunkTagFrameExtension DEM_MAKE_TAG('F','X','T','1')

/*=============================================================================
    Type definitions:
=============================================================================*/
//header of a demo file
typedef struct
{
    char ident[6];                              //identify the file
    uword version;                              //identify the version
    real32 startTime;                           //time the demo started
    sdword initialStateSize;                    //size of user-defined state information
#if DEM_RANDY_SAVE
    ranstream randyState[DEM_NumberStreamsToSave]; //state of all random number streams at start of demo
#endif
}
demoheader;

typedef struct
{
    demoheader header;
    udword modeFlags;
    udword recordedWidth;
    udword recordedHeight;
    udword recordedDepth;
}
demoheaderextended;

typedef struct
{
    udword tag;
    sdword size;
}
demochunkheader;

//state of a single key, and what key it is
typedef struct
{
    ubyte index;                                //index of the key
    keyScanType key;                            //state of the key
}
demokey;

//state of the mouse/keyboard at a given instant
typedef struct
{
    real32 time;                                //time this state snapshot was taken
#if DEM_CHECKSUM
    real32 univCheckSum;                            //universe checksum at the time (for error checking)
#if DEM_RANDOM_VERIFY
    udword randyStream[DEM_NumberStreamsToSave];//checksum for ranRandom()
#endif //DEM_RANDOM_VERIFY
#if DEM_CAMERA_CHECKSUM
    udword cameraCRC;                           //make sure the camera is deterministic
#endif //DEM_CAMERA_CHECKSUM
#if DEM_SELECT_CHECKSUM
    ubyte nSelected;                            //ships in selSelected
    ubyte nSelecting;                           //ships in selSelecting
    ubyte padSel[2];
#endif //DEM_SELECT_CHECKSUM
#endif //DEM_CHECKSUM
    sword  mouseX;                              //mouse state
    sword  mouseY;
    udword frameTicks;                          //value of utyNFrameTicks
    ubyte  mouseButtons;
    ubyte  taskNumber;                          //value of taskNumberCalls at save time
    ubyte  pad;
    ubyte  nKeys;                               //number of keys in the key state
//    demokey key[1];                             //key state array
}
demostate;
typedef struct
{
    real32 time;                                //time this state snapshot was taken
#if DEM_CHECKSUM
    real32 univCheckSum;                            //universe checksum at the time (for error checking)
#if DEM_RANDOM_VERIFY
    udword randyStream[DEM_NumberStreamsToSave];//checksum for ranRandom()
#endif //DEM_RANDOM_VERIFY
#if DEM_CAMERA_CHECKSUM
    udword cameraCRC;
#endif //DEM_CAMERA_CHECKSUM
#if DEM_SELECT_CHECKSUM
    ubyte nSelected;                            //ships in selSelected
    ubyte nSelecting;                           //ships in selSelecting
#endif //DEM_SELECT_CHECKSUM
#endif //DEM_CHECKSUM
    sword  mouseX;                              //mouse state
    sword  mouseY;
    udword frameTicks;                          //value of utyNFrameTicks.  Needed to keep the camera deterministic.
    ubyte  mouseButtons;                        //state of the mouse
    ubyte  taskNumber;                          //value of taskNumberCalls at save time
    ubyte  pad;
    ubyte  nKeys;                               //number of keys in the key state
    demokey key[KEY_TOTAL_KEYS];                //key state array
}
demomaxstate;

typedef struct
{
    real32 time;
    sword mouseX;
    sword mouseY;
    udword frameTicks;
    ubyte mouseButtons;
    ubyte taskNumber;
    ubyte pad;
    ubyte nKeys;
}
demoinputstate;

//callbacks for saving and loading user-defined state info at start of recording and playing back
typedef void (*demstatesave)(ubyte **buffer, sdword *size); //function called at start of recording
typedef void (*demstateload)(ubyte *buffer, sdword size);   //function called at start of start of playback
typedef void (*demplayfinished)(void);                      //function called at end of playback

/*=============================================================================
    Data:
=============================================================================*/
extern bool32 demDemoRecording;
extern bool32 demDemoPlaying;
extern bool32 wasDemoPlaying;
extern char demDemoFilename[128];
extern udword demRecordModeFlags;
extern udword demPlaybackModeFlags;
extern taskhandle demDemoTask;
extern sdword nDemoPlays, nUnivUpdates;
#if DEM_AUTO_DEMO
extern bool32 demAutoDemo;
extern real32 demAutoDemoWaitTime;
#endif
#if DEM_CHECKSUM
extern bool32 demChecksumError;
extern char demChecksumString[DEM_ChecksumLength];
#endif

#if DEM_INTERRUPTABLE
extern bool32 demPlaybackInterrupted;
#endif

#if DEM_FAKE_RENDER_SWITCH
extern bool32 demFakeRenders;                     //fake renders on by default
#endif

/*=============================================================================
    Macros:
=============================================================================*/
#define demStateSize(n)     ((sdword)sizeof(demostate) + (sdword)sizeof(demokey) * ((n)))
#define demoEndKeysHit()    (keyIsHit(CONTROLKEY) && keyIsHit(DKEY) && keyIsHit(EKEY))
#define demoInterruptKeysHit() (keyIsHit(CONTROLKEY) && keyIsHit(DKEY) && keyIsHit(IKEY))

/*=============================================================================
    Functions:
=============================================================================*/

void demRecordStart(char *fileName, demstatesave saveFunction);
bool32 demReadModeBHeader(char *fileName, demoheaderextended *header);
void demSetRecordModeFlags(udword flags);
void demEnableRecordModeB(void);
void demStateSave(void);
void demRecordEnd(void);
//void demDemoSaveTask(void);
//void demDemoPlayTask(void);
void demInputPlaybackStart(char *fileName, demstateload loadFunction, demplayfinished finishFunction);
void demInputPlaybackStop(void);
sdword demInputPlaybackStep(void);
void demRecordModeBRenderState(void);
void demApplyModeBRenderState(void);
void demPlayStart(char *fileName, demstateload loadFunction, demplayfinished finishFunction);
void demPlayEnd(void);
void demStateLoad(void);
void demLoadStateTransition(void);
sdword demNumberTicksLoad(sdword initial);
void demNumberTicksSave(sdword nTicks);

#endif //___DEMO_H
