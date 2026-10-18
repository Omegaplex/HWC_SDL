/*=============================================================================
    Name    : Demo.c
    Purpose : Code to record and play demos.

    Created 5/21/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "file.h"
#include "memory.h"
#include "debug.h"
#include "key.h"
#include "region.h"
#include "mouse.h"
#include "univupdate.h"
#include "universe.h"
#include "randy.h"
#include "camera.h"
#include "cameracommand.h"
#include "mainrgn.h"
#include "NIS.h"
#include "select.h"
#include "render.h"
#include "netcheck.h"
#include "Globals.h"
#include "demo.h"

extern sdword MAIN_WindowWidth, MAIN_WindowHeight, MAIN_WindowDepth;

/*=============================================================================
    Data:
=============================================================================*/
bool32 demDemoRecording = FALSE;
bool32 demDemoPlaying = FALSE;
bool32 wasDemoPlaying = FALSE;
char demDemoFilename[128];
taskhandle demDemoTask;
char demFileSaveName[128] = "";
udword demRecordModeFlags = 0;
udword demPlaybackModeFlags = 0;

demoheader *demLoadFile = NULL;
sdword demFileSize;
ubyte *demFilePointer;
demplayfinished demFinishFunction = NULL;
sdword demHeaderSize = sizeof(demoheader);

sdword nDemoPlays = 0, nUnivUpdates = 0;

#if DEM_RANDOM_VERIFY
bool8 demCheckStream[DEM_NumberStreamsToSave] =
{
    0,                          // RAN_ParticleStream
    0,                          // RAN_ETG
    0,                          // RAN_Sound
    1,                          // RAN_Trails
    1,                          // RAN_Clouds
    1,                          // RAN_Nebulae
    1,                          // RAN_AIPlayer
    0,                          // RAN_Damage
    1,                          // RAN_Battle
    0,                          // RAN_Static
    1,                          // RAN_Game
    1,                          // RAN_SoundGameThread
    0,                          // RAN_SoundBothThreads
    1,                          // RAN_Trails0
    1,                          // RAN_Trails1
    1,                          // RAN_Trails2
    1,                          // RAN_Trails3
    1,                          // RAN_Trails4
    1,                          // RAN_SoundGameThread0
    1,                          // RAN_SoundGameThread1
    1,                          // RAN_SoundGameThread2
    1,                          // RAN_SoundGameThread3
    1,                          // RAN_SoundGameThread4
    1,                          // RAN_SoundGameThread5
    1,                          // RAN_SoundGameThread6
};
#endif

extern udword utyNFrameTicks;
extern udword demFakeRenderCount;
extern udword demRecordedTicks;
extern udword demPlaybackTicks;
extern udword demPacketNumber;
void utyTaskTimerClear(void);
void demBlockWrite(void *data, sdword length);
bool32 demBlockRead(void *dest, sdword size);
bool32 demBlockSkip(sdword size);
void demFrameExtensionWriteEmpty(void);
bool32 demFrameExtensionRead(void);
bool32 demModeBInputPacketAvailable(void);
static bool32 demModeBStatePacketPlausible(ubyte *packetPointer);
static bool32 demFrameExtensionAvailableAt(ubyte *chunkPointer);

typedef enum
{
    DEM_CameraSourceNone = 0,
    DEM_CameraSourceMain = 1,
    DEM_CameraSourceNIS = 2,
    DEM_CameraSourceDefault = 3
}
demcamerasource;

typedef struct
{
    real32 angle;
    real32 declination;
    real32 distance;
    vector eyeposition;
    vector lookatpoint;
    vector oldlookatpoint;
    vector upvector;
    real32 fieldofview;
    real32 clipPlaneNear;
    real32 clipPlaneFar;
    real32 closestZoom;
    real32 farthestZoom;
    ubyte ignoreZoom;
    ubyte pad[3];
}
democamerapose;

typedef struct
{
    udword source;
    ubyte hasCurrentCamera;
    ubyte hasRememberCamera;
    ubyte pad[2];
    democamerapose currentCamera;
    democamerapose rememberCamera;
}
demomodebcameraextension;

static bool32 demModeBExtensionPending = FALSE;
static bool32 demModeBExtensionAwaitingWrite = FALSE;
static demomodebcameraextension demModeBCameraExtension;

static void demCameraPoseCapture(democamerapose *dest, Camera *source)
{
    dest->angle = source->angle;
    dest->declination = source->declination;
    dest->distance = source->distance;
    dest->eyeposition = source->eyeposition;
    dest->lookatpoint = source->lookatpoint;
    dest->oldlookatpoint = source->oldlookatpoint;
    dest->upvector = source->upvector;
    dest->fieldofview = source->fieldofview;
    dest->clipPlaneNear = source->clipPlaneNear;
    dest->clipPlaneFar = source->clipPlaneFar;
    dest->closestZoom = source->closestZoom;
    dest->farthestZoom = source->farthestZoom;
    dest->ignoreZoom = (ubyte)source->ignoreZoom;
    dest->pad[0] = 0;
    dest->pad[1] = 0;
    dest->pad[2] = 0;
}

static void demCameraPoseApply(Camera *dest, democamerapose *source)
{
    dest->angle = source->angle;
    dest->declination = source->declination;
    dest->distance = source->distance;
    dest->eyeposition = source->eyeposition;
    dest->lookatpoint = source->lookatpoint;
    dest->oldlookatpoint = source->oldlookatpoint;
    dest->upvector = source->upvector;
    dest->fieldofview = source->fieldofview;
    dest->clipPlaneNear = source->clipPlaneNear;
    dest->clipPlaneFar = source->clipPlaneFar;
    dest->closestZoom = source->closestZoom;
    dest->farthestZoom = source->farthestZoom;
    dest->ignoreZoom = (bool8)source->ignoreZoom;
}

static void demModeBCameraExtensionClear(void)
{
    demModeBExtensionPending = FALSE;
    memset(&demModeBCameraExtension, 0, sizeof(demModeBCameraExtension));
}

static void demFrameExtensionWriteCameraState(void)
{
    demochunkheader chunkHeader;

    memset(&demModeBCameraExtension, 0, sizeof(demModeBCameraExtension));
    if (mrCamera == NULL)
    {
        demFrameExtensionWriteEmpty();
        return;
    }

    if (mrCamera == &universe.mainCameraCommand.actualcamera)
    {
        CameraStackEntry *currentEntry = currentCameraStackEntry(&universe.mainCameraCommand);

        demModeBCameraExtension.source = DEM_CameraSourceMain;
        demModeBCameraExtension.hasCurrentCamera = TRUE;
        demCameraPoseCapture(&demModeBCameraExtension.currentCamera, &universe.mainCameraCommand.actualcamera);
        if (currentEntry != NULL)
        {
            demModeBCameraExtension.hasRememberCamera = TRUE;
            demCameraPoseCapture(&demModeBCameraExtension.rememberCamera, &currentEntry->remembercam);
        }
    }
    else if (nisCamera != NULL && mrCamera == nisCamera)
    {
        demModeBCameraExtension.source = DEM_CameraSourceNIS;
        demModeBCameraExtension.hasCurrentCamera = TRUE;
        demCameraPoseCapture(&demModeBCameraExtension.currentCamera, nisCamera);
    }
    else if (mrCamera == &defaultCamera)
    {
        demModeBCameraExtension.source = DEM_CameraSourceDefault;
        demModeBCameraExtension.hasCurrentCamera = TRUE;
        demCameraPoseCapture(&demModeBCameraExtension.currentCamera, &defaultCamera);
    }

    if (!demModeBCameraExtension.hasCurrentCamera)
    {
        demFrameExtensionWriteEmpty();
        return;
    }

    chunkHeader.tag = DEM_ChunkTagFrameExtension;
    chunkHeader.size = sizeof(demModeBCameraExtension);
    demBlockWrite(&chunkHeader, sizeof(chunkHeader));
    demBlockWrite(&demModeBCameraExtension, sizeof(demModeBCameraExtension));
}

void demSetRecordModeFlags(udword flags)
{
    demRecordModeFlags = flags;
}

void demEnableRecordModeB(void)
{
    demSetRecordModeFlags(DEM_ModeB);
}

bool32 demReadModeBHeader(char *fileName, demoheaderextended *header)
{
    demoheader fileHeader;
    FILE *file;

    file = fopen(fileName, "rb");
    if (file == NULL)
    {
        return FALSE;
    }

    if (fread(&fileHeader, sizeof(fileHeader), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (strcmp(fileHeader.ident, DEM_HeaderString))
    {
        fclose(file);
        return FALSE;
    }
    if (fileHeader.version == DEM_VersionNumber)
    {
        rewind(file);
        if (fread(header, sizeof(*header), 1, file) != 1)
        {
            fclose(file);
            return FALSE;
        }
    }
    else
    {
        fclose(file);
        return FALSE;
    }
    fclose(file);

    return TRUE;
}

void demInputPlaybackStart(char *fileName, demstateload loadFunction, demplayfinished finishFunction)
{
    sdword index;
    demoheaderextended *extendedHeader;

    demPlaybackInterrupted = FALSE;
    demFileSize = fileLoadAlloc(fileName, (void **)&demLoadFile, NonVolatile);
    demHeaderSize = sizeof(demoheader);
    demPlaybackModeFlags = 0;

    if (strcmp(demLoadFile->ident, DEM_HeaderString))
    {
        dbgFatalf(DBG_Loc, "Invalid demo file '%s'", fileName);
    }

    if (demLoadFile->version == DEM_OriginalVersionNumber)
    {
        demHeaderSize = sizeof(demoheader);
    }
    else if (demLoadFile->version == DEM_VersionNumber)
    {
        if (demFileSize < sizeof(demoheaderextended))
        {
            dbgFatalf(DBG_Loc, "Truncated extended demo file '%s'", fileName);
        }
        extendedHeader = (demoheaderextended *)demLoadFile;
        demPlaybackModeFlags = extendedHeader->modeFlags;
        demHeaderSize = sizeof(demoheaderextended);
    }
    else
    {
        dbgFatalf(DBG_Loc, "Invalid version of file '%s'. Expected 0x%x or 0x%x, found 0x%x.", fileName, DEM_OriginalVersionNumber, DEM_VersionNumber, demLoadFile->version);
    }

    demFilePointer = (ubyte *)demLoadFile + demHeaderSize;

    for (index = 0; index < DEM_NumberStreamsToSave; index++)
    {
        ranParametersSet(index,
                         demLoadFile->randyState[index].x,
                         demLoadFile->randyState[index].y,
                         demLoadFile->randyState[index].z,
                         demLoadFile->randyState[index].c,
                         demLoadFile->randyState[index].n);
    }

    if (loadFunction != NULL && demLoadFile->initialStateSize > 0)
    {
        loadFunction(demFilePointer, demLoadFile->initialStateSize);
    }
    demFilePointer += demLoadFile->initialStateSize;

    demFinishFunction = finishFunction;
    demFakeRenderCount = 0;
    demPlaybackTicks = 0;
    demRecordedTicks = 0;
    demPacketNumber = 0;
    demModeBExtensionAwaitingWrite = FALSE;
    demModeBCameraExtensionClear();
    if (demPlaybackModeFlags & DEM_ModeB)
    {
        utyTaskTimerClear();
    }
    demInputPlaybackStep();
}

void demInputPlaybackStop(void)
{
    rndMainViewRender = rndMainViewRenderFunction;
    if (demLoadFile != NULL)
    {
        memFree(demLoadFile);
        demLoadFile = NULL;
    }
    demPlaybackModeFlags = 0;
    demHeaderSize = sizeof(demoheader);
    demModeBExtensionAwaitingWrite = FALSE;
    demModeBCameraExtensionClear();
    demDemoPlaying = FALSE;
    if (demFinishFunction != NULL)
    {
        demFinishFunction();
        demFinishFunction = NULL;
    }
}

sdword demInputPlaybackStep(void)
{
    demoinputstate state;
    demokey keyState[KEY_TOTAL_KEYS];
    bool32 mouseLButton = keyScanCode[LMOUSE_BUTTON].keypressed;
    sdword index;

    if (demPlaybackModeFlags & DEM_ModeB)
    {
        while (demFrameExtensionAvailableAt(demFilePointer))
        {
            if (!demFrameExtensionRead())
            {
                demInputPlaybackStop();
                return 0;
            }
        }
        if (!demModeBInputPacketAvailable())
        {
            return taskNumberCalls;
        }
    }
    if (!demBlockRead(&state, sizeof(state)))
    {
        demInputPlaybackStop();
        return 0;
    }
    if (!demBlockRead(keyState, sizeof(demokey) * state.nKeys))
    {
        demInputPlaybackStop();
        return 0;
    }
    if ((demPlaybackModeFlags & DEM_ModeB) &&
        demFrameExtensionAvailableAt(demFilePointer) &&
        !demFrameExtensionRead())
    {
        demInputPlaybackStop();
        return 0;
    }

    memset((ubyte *)&keyScanCode[0], 0, sizeof(keyScanCode));
    memset((ubyte *)&keySaveScan[0], 0, sizeof(keySaveScan));
    for (index = 0; index < state.nKeys; index++)
    {
        keyScanCode[keyState[index].index] = keyState[index].key;
        keySaveScan[keyState[index].index] = keyState[index].key;
    }

    if ((state.mouseButtons & MB_Left) != 0 && !keyScanCode[LMOUSE_BUTTON].keypressed)
    {
        keyScanCode[LMOUSE_BUTTON].keypressed = 1;
        keySaveScan[LMOUSE_BUTTON].keypressed = 1;
    }
    if ((state.mouseButtons & MB_Right) != 0 && !keyScanCode[RMOUSE_BUTTON].keypressed)
    {
        keyScanCode[RMOUSE_BUTTON].keypressed = 1;
        keySaveScan[RMOUSE_BUTTON].keypressed = 1;
    }
    if ((state.mouseButtons & MB_Centre) != 0 && !keyScanCode[MMOUSE_BUTTON].keypressed)
    {
        keyScanCode[MMOUSE_BUTTON].keypressed = 1;
        keySaveScan[MMOUSE_BUTTON].keypressed = 1;
    }

    if (demPlaybackModeFlags & DEM_ModeB)
    {
        mousePositionSetGameOnly((sdword)state.mouseX, (sdword)state.mouseY);
    }
    else
    {
        mousePositionSet((sdword)state.mouseX, (sdword)state.mouseY);
    }
    mouseButtons = state.mouseButtons;
    taskNumberCalls = (udword)state.taskNumber;
    utyNFrameTicks = state.frameTicks;

    if (!mouseLButton && keyScanCode[LMOUSE_BUTTON].keypressed)
    {
        mouseLClick();
    }
    demPacketNumber++;
    return taskNumberCalls;
}

extern udword utyNFrameTicks;

//auto-demo stuff
#if DEM_AUTO_DEMO
bool32 demAutoDemo = TRUE;
real32 demAutoDemoWaitTime = DEM_AUTO_DELAY;
#endif

#if DEM_CHECKSUM
bool32 demChecksumError = FALSE;
char demChecksumString[DEM_ChecksumLength];
#endif

#if DEM_INTERRUPTABLE
bool32 demPlaybackInterrupted = FALSE;
#endif

//stuff for keeping track of fake vs. real renders
udword demFakeRenderCount;                      //number of times we have to do fake renders before we're allowed to render again.
udword demRecordedTicks;                        //accumulated number of ticks from recording
udword demPlaybackTicks;                        //accumulated number of ticks from playback, including the remainder of last division
udword demPacketNumber;                         //what demo 'packet' we're on
#if DEM_FAKE_RENDER_SWITCH
bool32 demFakeRenders = TRUE;                     //fake renders on by default
#endif

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : demBlockWrite
    Description : Write a block to the demo file.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demBlockWrite(void *data, sdword length)
{
    FILE *appendfile;

    if (*demFileSaveName)
    {
        appendfile = fopen(demFileSaveName, "ab");
        if (appendfile != NULL)
        {
            fwrite(data, length, 1, appendfile);
            fclose(appendfile);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : demRecordStart
    Description : Start recording a demo.
    Inputs      : fileName - name of file to save to.
                  saveFunction - user function to call to save user state info.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void demRecordStart(char *fileName, demstatesave saveFunction)
{
    demoheader header;
    demoheaderextended extendedHeader;
    ubyte *stateBuffer;
    void *headerToWrite = &header;
    sdword headerSize = sizeof(header);
#if DEM_RANDY_SAVE
    sdword index;
#endif

    dbgAssert(demDemoPlaying != TRUE);
    //create a header structure
    strcpy(header.ident, DEM_HeaderString);
    header.version = (demRecordModeFlags & DEM_ModeB) ? DEM_VersionNumber : DEM_OriginalVersionNumber;
    header.startTime = taskTimeElapsed;
#if DEM_RANDY_SAVE
    for (index = 0; index < DEM_NumberStreamsToSave; index++)
    {                                                       //save the state of all random number streams
         ranParametersGet(index, &header.randyState[index].x, &header.randyState[index].y, &header.randyState[index].z, &header.randyState[index].c, &header.randyState[index].n);
    }
#endif

    if (saveFunction != NULL)
    {                                                       //save initial configuration, if applicable
        saveFunction(&stateBuffer, &header.initialStateSize);
        dbgAssert(header.initialStateSize > 0);
    }
    else
    {
        header.initialStateSize = 0;
    }

    if (demRecordModeFlags & DEM_ModeB)
    {
        memcpy(&extendedHeader.header, &header, sizeof(header));
        extendedHeader.modeFlags = demRecordModeFlags;
        extendedHeader.recordedWidth = MAIN_WindowWidth;
        extendedHeader.recordedHeight = MAIN_WindowHeight;
        extendedHeader.recordedDepth = MAIN_WindowDepth;
        headerToWrite = &extendedHeader;
        headerSize = sizeof(extendedHeader);
    }

    //open the file         (actually, delete it, and every time we will open it in append write mode)
    strcpy(demFileSaveName,fileName);
    remove(fileName);

    demBlockWrite(headerToWrite, headerSize);
    if (header.initialStateSize > 0)
    {                                                       //if there is state info to save
        demBlockWrite(stateBuffer, header.initialStateSize);//save it
        memFree(stateBuffer);                               //and free the state buffer
    }
#if DEM_CHECKSUM
    if (logEnable == LOG_VERBOSE)
    {
        netlogfile = fopen(DEM_LogFileName, "wb");
        fclose(netlogfile);
        netlogfile = NULL;
    }
#endif
    demPacketNumber = 0;
    demModeBExtensionAwaitingWrite = FALSE;
    demModeBCameraExtensionClear();
    demStateSave();
    if ((demRecordModeFlags & DEM_ModeB) && demModeBExtensionAwaitingWrite)
    {
        demFrameExtensionWriteCameraState();
        demModeBExtensionAwaitingWrite = FALSE;
    }

    demDemoRecording = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : demStateSave
    Description : Save the state of the mouse and keyboard at a given instant.
    Inputs      : void
    Outputs     : Creates and saves a state structure.
    Return      : void
----------------------------------------------------------------------------*/
void demStateSave(void)
{
    demoinputstate state;
    demokey keyState[KEY_TOTAL_KEYS];
    sdword index;
    sdword nKeys = 0;

    dbgAssert(demDemoRecording);
    if ((demRecordModeFlags & DEM_ModeB) && demModeBExtensionAwaitingWrite)
    {
        demFrameExtensionWriteEmpty();
        demModeBExtensionAwaitingWrite = FALSE;
    }
    state.time = taskTimeElapsed;
    state.mouseX = (sword)mouseCursorX();
    state.mouseY = (sword)mouseCursorY();
    state.mouseButtons = (ubyte)mouseButtons;
    dbgAssert(taskNumberCalls >= 0 && taskNumberCalls < 256);
    state.taskNumber = (ubyte)taskNumberCalls;
    state.pad = 0;
    state.frameTicks = utyNFrameTicks;

    for (index = 0; index < KEY_TOTAL_KEYS; index++)
    {
        if (*((ubyte *)&keyScanCode[index]) != 0)
        {
            keyState[nKeys].index = (ubyte)index;
            keyState[nKeys].key = keyScanCode[index];
            nKeys++;
        }
    }
    state.nKeys = (ubyte)nKeys;

    demBlockWrite(&state, sizeof(state));
    if (nKeys > 0)
    {
        demBlockWrite(keyState, sizeof(demokey) * nKeys);
    }
    if (demRecordModeFlags & DEM_ModeB)
    {
        demModeBExtensionAwaitingWrite = TRUE;
        if (!gameIsRunning)
        {
            demFrameExtensionWriteEmpty();
            demModeBExtensionAwaitingWrite = FALSE;
        }
    }

    if (demoEndKeysHit())
    {
        demRecordEnd();
    }
    demPacketNumber++;
}

/*-----------------------------------------------------------------------------
    Name        : demNumberTicksSave
    Description : Store the number of task ticks before any tasks are called
    Inputs      : nTicks - number of task ticks for this time through
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demNumberTicksSave(sdword nTicks)
{
    dbgAssert(demDemoRecording);
    demBlockWrite(&nTicks, sizeof(sdword));
}

/*-----------------------------------------------------------------------------
    Name        : demRecordEnd
    Description : End a demo recording session, closing the file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demRecordEnd(void)
{
    dbgAssert(demDemoRecording);
    if ((demRecordModeFlags & DEM_ModeB) && demModeBExtensionAwaitingWrite)
    {
        demFrameExtensionWriteEmpty();
        demModeBExtensionAwaitingWrite = FALSE;
    }
    demModeBCameraExtensionClear();
    demFileSaveName[0] = 0;
    demDemoRecording = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : demBlockRead
    Description : Read in a block of data from the demo file in memory.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 demBlockRead(void *dest, sdword size)
{
    if (demFilePointer + size <= (ubyte *)demLoadFile + demFileSize)
    {
        memcpy(dest, demFilePointer, size);
        demFilePointer += size;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool32 demBlockSkip(sdword size)
{
    if (demFilePointer + size <= (ubyte *)demLoadFile + demFileSize)
    {
        demFilePointer += size;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void demFrameExtensionWriteEmpty(void)
{
    demochunkheader chunkHeader;

    chunkHeader.tag = DEM_ChunkTagFrameExtension;
    chunkHeader.size = 0;
    demBlockWrite(&chunkHeader, sizeof(chunkHeader));
}

static bool32 demModeBStatePacketPlausible(ubyte *packetPointer)
{
    demoinputstate *state;
    demokey *keyState;
    sdword packetSize;
    sdword index;

    if (packetPointer + sizeof(demoinputstate) > (ubyte *)demLoadFile + demFileSize)
    {
        return FALSE;
    }

    state = (demoinputstate *)packetPointer;
    if (state->nKeys > KEY_TOTAL_KEYS)
    {
        return FALSE;
    }
    if (state->time < 0.0f)
    {
        return FALSE;
    }
    if ((state->mouseButtons & ~7) != 0)
    {
        return FALSE;
    }
    if (state->frameTicks > 1000)
    {
        return FALSE;
    }
    if (state->mouseX < -4096 || state->mouseX > 4096 ||
        state->mouseY < -4096 || state->mouseY > 4096)
    {
        return FALSE;
    }

    packetSize = sizeof(demoinputstate) + sizeof(demokey) * state->nKeys;
    if (packetPointer + packetSize > (ubyte *)demLoadFile + demFileSize)
    {
        return FALSE;
    }

    keyState = (demokey *)(packetPointer + sizeof(demoinputstate));
    for (index = 0; index < state->nKeys; index++)
    {
        if (keyState[index].index >= KEY_TOTAL_KEYS)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static bool32 demFrameExtensionAvailableAt(ubyte *chunkPointer)
{
    demochunkheader *chunkHeader;

    if (chunkPointer + sizeof(demochunkheader) > (ubyte *)demLoadFile + demFileSize)
    {
        return FALSE;
    }

    chunkHeader = (demochunkheader *)chunkPointer;
    if (chunkHeader->tag != DEM_ChunkTagFrameExtension || chunkHeader->size < 0)
    {
        return FALSE;
    }
    if (chunkPointer + sizeof(demochunkheader) + chunkHeader->size > (ubyte *)demLoadFile + demFileSize)
    {
        return FALSE;
    }

    return TRUE;
}

bool32 demFrameExtensionRead(void)
{
    demochunkheader chunkHeader;

    if (!demBlockRead(&chunkHeader, sizeof(chunkHeader)))
    {
        return FALSE;
    }
    if (chunkHeader.tag != DEM_ChunkTagFrameExtension || chunkHeader.size < 0)
    {
        return FALSE;
    }
    if (chunkHeader.size == 0)
    {
        demModeBCameraExtensionClear();
        return TRUE;
    }
    if (chunkHeader.size != sizeof(demModeBCameraExtension))
    {
        return FALSE;
    }
    if (!demBlockRead(&demModeBCameraExtension, sizeof(demModeBCameraExtension)))
    {
        return FALSE;
    }
    demModeBExtensionPending = demModeBCameraExtension.hasCurrentCamera;
    return TRUE;
}

bool32 demModeBInputPacketAvailable(void)
{
    demoinputstate *state;
    sdword packetSize;
    ubyte *packetPointer;
    ubyte *packetEnd;

    packetPointer = demFilePointer;
    if (!demModeBStatePacketPlausible(packetPointer))
    {
        return FALSE;
    }

    state = (demoinputstate *)packetPointer;
    packetSize = sizeof(demoinputstate) + sizeof(demokey) * state->nKeys;
    packetEnd = packetPointer + packetSize;
    return packetEnd <= (ubyte *)demLoadFile + demFileSize;
}

void demRecordModeBRenderState(void)
{
    if (!demDemoRecording || !(demRecordModeFlags & DEM_ModeB) || !demModeBExtensionAwaitingWrite)
    {
        return;
    }

    demFrameExtensionWriteCameraState();
    demModeBExtensionAwaitingWrite = FALSE;
}

void demApplyModeBRenderState(void)
{
    CameraStackEntry *currentEntry;

    if (!demDemoPlaying || !(demPlaybackModeFlags & DEM_ModeB) || !demModeBExtensionPending)
    {
        return;
    }

    switch (demModeBCameraExtension.source)
    {
        case DEM_CameraSourceMain:
            currentEntry = currentCameraStackEntry(&universe.mainCameraCommand);
            if (currentEntry == NULL)
            {
                return;
            }
            demCameraPoseApply(&universe.mainCameraCommand.actualcamera, &demModeBCameraExtension.currentCamera);
            if (demModeBCameraExtension.hasRememberCamera)
            {
                demCameraPoseApply(&currentEntry->remembercam, &demModeBCameraExtension.rememberCamera);
            }
            mrCamera = &universe.mainCameraCommand.actualcamera;
            break;
        case DEM_CameraSourceNIS:
            if (nisCamera == NULL)
            {
                return;
            }
            demCameraPoseApply(nisCamera, &demModeBCameraExtension.currentCamera);
            mrCamera = nisCamera;
            break;
        case DEM_CameraSourceDefault:
            demCameraPoseApply(&defaultCamera, &demModeBCameraExtension.currentCamera);
            mrCamera = &defaultCamera;
            break;
        default:
            return;
    }
}

/*-----------------------------------------------------------------------------
    Name        : demPlayStart
    Description : Start playing a demo
    Inputs      : fileName - name of the file to start play
                  loadfunction - function to call with state information
                  finishFunction - function to call whent the demo playback is completed
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demPlayStart(char *fileName, demstateload loadFunction, demplayfinished finishFunction)
{
    demInputPlaybackStart(fileName, loadFunction, finishFunction);
}

/*-----------------------------------------------------------------------------
    Name        : demPlayEnd
    Description : Stop playing a demo
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void demPlayEnd(void)
{
    demInputPlaybackStop();
}

/*-----------------------------------------------------------------------------
    Name        : demStateLoad
    Description : Load in the state of the mouse and keyboard
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void demStateLoad(void)
{
    demInputPlaybackStep();
}

void demLoadStateTransition(void)
{
    if (demDemoRecording)
    {
        memcpy((ubyte *)&keyScanCode[0], (ubyte *)&keySaveScan[0], sizeof(keyScanCode));
        demStateSave();
    }
    else if (demDemoPlaying)
    {
        demStateLoad();
    }
}

/*-----------------------------------------------------------------------------
    Name        : demNumberTicksLoad
    Description : Load in the number of ticks for processing tasks, before any
                    tasks get called.
    Inputs      : initial - default number of ticks
    Outputs     :
    Return      : number of ticks for this process pass
----------------------------------------------------------------------------*/
sdword demNumberTicksLoad(sdword initial)
{
    sdword nTicks;

    if (demLoadFile == NULL)
    {
        return(initial);
    }
    demPlaybackTicks += initial;
    if (!demBlockRead(&nTicks, sizeof(sdword)))
    {
        demInputPlaybackStop();
        return(initial);
    }

    demRecordedTicks += nTicks;

    if (demFakeRenderCount != 0)
    {
        if (gameIsRunning)
        {
            rndMainViewRender = rndMainViewRenderNothingFunction;
        }
        demFakeRenderCount--;
        return(nTicks);
    }

    rndMainViewRender = rndMainViewRenderFunction;
    if (demRecordedTicks >= demPlaybackTicks)
    {
        demFakeRenderCount = 0;
        demPlaybackTicks = 0;
        demRecordedTicks = 0;
    }
    else if (demRecordedTicks == 0)
    {
        demFakeRenderCount = 0;
    }
    else
    {
        demFakeRenderCount = min(demPlaybackTicks / demRecordedTicks, DEM_MaxFakeRenders) - 1;
        demPlaybackTicks = demPlaybackTicks % demRecordedTicks;
        demRecordedTicks = 0;
    }
    return(nTicks);
}



#if 0  // Cat2: Binary demRecordStart; live implementation at demo.c (0x462B60)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x462B60
char demRecordStart(char *fileName, void (*saveFunction)(void **, size_t *))
{
    int i;
    char *statePtr;
    FILE *f;
    FILE *fp;
    size_t stateSize;
    void *stateData;
    FILE *f2;
    FILE *fp2;
    char result;
    void *saveData;
    _WORD header[4];
    int headerVersion;
    size_t saveSize[4];
    char stateEntries;

    strcpy((char *)header, "BaNng");
    header[3] = 7;
    headerVersion = gameVersion;
    i = 0;
    statePtr = &stateEntries;
    do
    {
        sub_4EF910(i++, (int *)(statePtr - 12), (int *)(statePtr - 8), (int *)(statePtr - 4), statePtr, (int *)statePtr + 1);
        statePtr += 20;
    }
    while ( i < 27 );
    if ( saveFunction )
        saveFunction(&saveData, saveSize);
    else
        saveSize[0] = 0;
    strcpy(&demFileName, fileName);
    remove(fileName);
    if ( demFileName )
    {
        f = fopen(&demFileName, "ab");
        fp = f;
        if ( f )
        {
            fwrite(header, 0x22Cu, 1u, f);
            fclose(fp);
        }
    }
    stateSize = saveSize[0];
    if ( (int)saveSize[0] > 0 )
    {
        stateData = saveData;
        if ( demFileName )
        {
            f2 = fopen(&demFileName, "ab");
            fp2 = f2;
            if ( f2 )
            {
                fwrite(stateData, stateSize, 1u, f2);
                fclose(fp2);
            }
        }
        memFree((ubyte *)saveData);
    }
    demFrameCount = 0;
    result = demStateSave();
    demRecording = 1;
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x462CC0
void demStateSave(void)
{
    int i;
    FILE *f;
    FILE *fp;
    int frameData;
    __int16 mouseX;
    __int16 mouseY;
    int frameTicks;
    char mouseButtons;
    char taskCalls;
    unsigned __int8 nKeys;
    _BYTE keyEntries[512];

    frameData = gameVersion;
    mouseX = mouseCursorX;
    mouseY = mouseCursorY;
    mouseButtons = mouseButtons;
    taskCalls = taskNumberCalls;
    frameTicks = utyNFrameTicks;
    nKeys = 0;
    for ( i = 0; i < 256; ++i )
    {
        if ( keyScanCode[i] )
        {
            keyEntries[2 * nKeys] = i;
            keyEntries[2 * nKeys++ + 1] = keyScanCode[i];
        }
    }
    if ( demFileName )
    {
        f = fopen(&demFileName, "ab");
        fp = f;
        if ( f )
        {
            fwrite(&frameData, 2 * nKeys + 16, 1u, f);
            fclose(fp);
        }
    }
    if ( (demFlag1 & 1) != 0 && (demFlag2 & 1) != 0 && (demFlag3 & 1) != 0 )
    {
        demFileName = 0;
        demRecording = 0;
    }
    ++demFrameCount;
}
#endif
