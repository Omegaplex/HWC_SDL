/*=============================================================================
    Name    : SpeechEvent.c
    Purpose : Speech Event processor

    Created 10/01/1998 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "SoundEventPrivate.h"
#include "SoundEvent.h"
#include "select.h"
#include "blobs.h"
#include "soundcmn.h"
#include "nis.h"
#include "soundmusic.h"
#include "fquant.h"
#include "chatting.h"
#include "singleplayer.h"
#include "Subtitle.h"
#include "bink.h"
#include "utility.h"

#undef CATACLYSM_DIAG
#define CATACLYSM_DIAG 1

extern real32 SUBMESSAGE_SAFETY_TIMEOUT;

#define STATE_BREAK     3

#define MAX_BATTLES     20

#define Asteroids       0
#define Nebulae         1
#define GasClouds       2
#define DustClouds      3

#define NISSTREAM       0
#define AMBIENTSTREAM   1

#define STINGER_FADE_TIME   0.5f

#define MUSIC_FADE_FAST     0.5f
#define MUSIC_FADE_SLOW     1.0f

#define MUSIC_MIN_FADE  80

#define SPEECH_FLEET        0
#define SPEECH_STRIKECRAFT  1
#define SPEECH_CAPSHIPS     2
#define SPEECH_CONSTRUCTION 3
#define SPEECH_INTEL        4
#define SPEECH_PILOT1       1
#define SPEECH_PILOT2       2
#define SPEECH_PILOT3       3

#define NUM_SPEAKERS        5

#define SE_MAX_QUEUE        10

#define SPEECH_PAUSE        1.5f

#define MAX_RELIC_NAMES     33

#define SPEECH_SUPERNOVA_LEVEL  10

/*=============================================================================
    Structures
=============================================================================*/
typedef struct
{
    sdword      offset;
    sword       flags;
    sword       bitrate;
    ubyte       volume;
    ubyte       numchannels;
    sword       pad;
} MUSICSTREAM;

typedef struct
{
    sdword      ID;
    udword      checksum;
    sdword      numstreams;
    MUSICSTREAM mstreams[];
} MUSICHEADER;

#if 1
typedef struct
{
    sdword      handle;     // this is the handle for this event (might not be unique if linked)
    sdword      index;      // this is the index of linked events
    sdword      status;

    sdword      actornum;   // need this for linked speech events

    sdword      numlinks;


    Ship        *pShip;     // ship pointer associated with this event
    sdword      event;      // actual event, listed in speechevent.h
    sdword      variable;   // optional variable for this event
    sdword      variation;  // overriding variation to play
    uword       priority;   // priority of this event
    sword       volume;     // overriding volume

    real32      timein;     // at what time was this event put in the queue?
    real32      timeout;    // at what point is this going to timeout at?
} QUEUEEVENT;

typedef struct
{
    QUEUEEVENT  queue[SE_MAX_QUEUE];    // these are the queued events
    QUEUEEVENT  current;    // the event that is currently playing

    sdword      status;     // status of this speech channel
    sdword      nextevent;  // index into the queue of the next event

    bool32        locked;     // set when deleting a bunch of stuff so another event doesn't play

    real32      timeover;   // what time will this event be over (approx)

    sdword      numqueued;

    sdword      lastpriority;
    sdword      laststarttick;
    sdword      lastendtick;
    sdword      lastevent;
    Ship        *lastship;
} SPEECHQUEUE;

typedef struct
{
    sdword      handle;
    sdword      status;
    sdword      tracknum;
    sdword      nexttrack;
    sword       targetvol;
    sword       pad;
} MUSICINFO;
#endif


/*=============================================================================
    Prototypes
=============================================================================*/

void speechPlayQueue(SPEECHQUEUE *pSQueue, sdword streamchannel);
static void speechPlayQueueSentence(SPEECHQUEUE *pSQueue, sdword streamchannel);
sdword SEselectsentence(sdword actor, sdword event, sdword variable, sdword setvariation, sdword **pOffsets, udword *pDuration);
void speechQueueUpdate(void);
sdword SEreorderqueue(SPEECHQUEUE *pSQueue);
sdword SEcleanqueue(SPEECHQUEUE *pSQueue);
void soundEventDebugPrint(char *pszInformation);
static const char *speechEventRacePrefix(ShipRace playerRace);
static void speechEventResetQueues(void);


/*=============================================================================
    Private Data
=============================================================================*/

SPEECHQUEUE speechqueue[SE_NUM_ACTORS];
sdword  streamhandle[SE_NUM_ACTORS];
sdword  speechfilehandle;

sdword  numSinglePlayerEvents;

bool32    MoShipMoving[MAX_MULTIPLAYER_PLAYERS] = {FALSE};

ubyte   *pspeechstream;

ubyte   *pspeechbuffer0;
ubyte   *pspeechbuffer1;
ubyte   *pspeechbuffer2;
ubyte   *pspeechbuffer3;
ubyte   *pspeechbuffer4;

Ship    *lastshiptospeak = NULL;
sdword  lastgrouptospeak = -1;
Ship    *currentSpeechEvent = NULL;  // ship currently playing a speech event; set by speechPlaySentence, cleared by soundEventStopActorStreams

/* speech lookup tables */
SENTENCELUT *SentenceLUT;
PHRASELUT   *PhraseLUT;

/* special fx vars */
STREAMDELAY streamdelay[NUM_SPEAKERS];
STREAMEQ    streamEQ[NUM_SPEAKERS];
EFFECT      cleaneffect[NUM_SPEAKERS];
EFFECT      mixedeffect[NUM_SPEAKERS];
EFFECT      damageeffect;


/* music variables */
MUSICINFO   musicinfo[2];
sdword  musicstreamhandle[2];
sdword  musicfilehandle;
ubyte   *pmusicbuffer0;
ubyte   *pmusicbuffer1;
MUSICHEADER *musicheader;
sdword  levelTrack = -1;
static bool32 speechEventLoaded = FALSE;
sdword speechEnemyPresencePendingUnderAttackFromAlly = 0;
sdword speechEnemyPresencePendingFleetStatus76 = 0;
sdword speechEnemyPresencePendingResourceInjectionReceived = 0;
real32 speechEnemyPresenceLastWarningTime = 0.0f;

char relicPlayerNames[MAX_RELIC_NAMES][MAX_PERSONAL_NAME_LEN] = {
    "Rupract",
    "SpankMaster3000",
    "SM3K",
    "Plaz",
    "dav",
    "Briscola",
    "EFMagnet",
    "Midnight",
    "Quinn_Deadly",
    "YourMammaStinks",
    "Wintermute",
    "Chimpden",
    "Lunarboy",
    "Unspacey",
    "Antonalogue",
    "=A=",
    "B1FF",
    "Hootchi",
    "rhithyn",
    "Psyco-Kitty",
    "Rasolf",
    "GargleTroth",
    "Flako",
    "Yonderboy",
    "DrakeBird",
    "TheDoctor",
    "Tender",
    "Killroy",
    "Rocketboy",
    "TheEmporersLover",
    "DrMagneto",
    "DrTongue",
    "NoWhere2Camp"
};

/* name of music header file */
#if defined(CGW)
char musicHeaderName[] = "CGW_Music.wxh";
#elif defined(Downloadable)
char musicHeaderName[] = "DL_Music.wxh";
#elif defined(DLPublicBeta)
char musicHeaderName[] = "PB_Music.wxh";
#elif defined(OEM)
char musicHeaderName[] = "OEM_Music.wxh";
#else
char musicHeaderName[] = "Music4Cat.wxh";
#endif

/* externs */
extern bool32 soundpaused;

/* global volume settings */
extern real32 volSpeech;
extern real32 volMusic;
extern real32 volNIS;
extern bool32 bVolMusicNoFade;

/* global speech settings */
extern bool32 bActorOn[];
extern bool32 bCommandsOn;
extern bool32 bStatusOn;
extern bool32 bChatterOn;

extern sdword actorFlagsEnabled;

extern real32 nearestShipDistance;
extern real32 musicVolFactor;
extern real32 battleMusicVolFactor;
extern real32 musicInactiveFactor;

static const char *speechEventRacePrefix(ShipRace playerRace)
{
#ifdef CGW
    return "CGW";
#elif defined(Downloadable) || defined(DLPublicBeta)
    return "DL";
#else
    if (playerRace == RACE_Beast)
    {
        return "Beast";
    }
    return "Sect";
#endif
}

uword speechEventBitrate(void)
{
    dbgAssert(SentenceLUT != NULL);
    return ((CataSentenceLUTHeader *)SentenceLUT)->streamBitrate;
}

static void speechEventResetQueues(void)
{
    sdword i, j;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        memset(&speechqueue[i], 0, sizeof(SPEECHQUEUE));
        speechqueue[i].current.handle = SOUND_NOTINITED;
        speechqueue[i].status = SOUND_STOPPED;
        speechqueue[i].nextevent = SOUND_NOTINITED;
        speechqueue[i].locked = FALSE;
        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            speechqueue[i].queue[j].handle = SOUND_NOTINITED;
            speechqueue[i].queue[j].variation = SOUND_NOTINITED;
        }
    }

    numSinglePlayerEvents = 0;
}
extern bool32 nearestShipMoving;

extern bool32 FalkosFuckedUpTutorialFlag;

/*=============================================================================
    Scripts
=============================================================================*/

scriptStructEntry streamEQScriptTable[] =
{
    { "flags", scriptSetSdwordCB, (udword)&streamEQ[0].flags, (udword)&streamEQ[0] },
    { "eq0", scriptSetReal32CB, (udword)&streamEQ[0].eq[0], (udword)&streamEQ[0] },
    { "eq1", scriptSetReal32CB, (udword)&streamEQ[0].eq[1], (udword)&streamEQ[0] },
    { "eq2", scriptSetReal32CB, (udword)&streamEQ[0].eq[2], (udword)&streamEQ[0] },
    { "eq3", scriptSetReal32CB, (udword)&streamEQ[0].eq[3], (udword)&streamEQ[0] },
    { "eq4", scriptSetReal32CB, (udword)&streamEQ[0].eq[4], (udword)&streamEQ[0] },
    { "eq5", scriptSetReal32CB, (udword)&streamEQ[0].eq[5], (udword)&streamEQ[0] },
    { "eq6", scriptSetReal32CB, (udword)&streamEQ[0].eq[6], (udword)&streamEQ[0] },
    { "eq7", scriptSetReal32CB, (udword)&streamEQ[0].eq[7], (udword)&streamEQ[0] },
    endEntry
};

scriptStructEntry streamDelaySciptTable[] =
{
    { "flags", scriptSetSdwordCB, (udword)&streamdelay[0].flags, (udword)&streamdelay[0] },
    { "level", scriptSetReal32CB, (udword)&streamdelay[0].level, (udword)&streamdelay[0] },
    { "duration", scriptSetUdwordCB, (udword)&streamdelay[0].duration, (udword)&streamdelay[0] },
    { "eq0", scriptSetReal32CB, (udword)&streamdelay[0].eq[0], (udword)&streamdelay[0] },
    { "eq1", scriptSetReal32CB, (udword)&streamdelay[0].eq[1], (udword)&streamdelay[0] },
    { "eq2", scriptSetReal32CB, (udword)&streamdelay[0].eq[2], (udword)&streamdelay[0] },
    { "eq3", scriptSetReal32CB, (udword)&streamdelay[0].eq[3], (udword)&streamdelay[0] },
    { "eq4", scriptSetReal32CB, (udword)&streamdelay[0].eq[4], (udword)&streamdelay[0] },
    { "eq5", scriptSetReal32CB, (udword)&streamdelay[0].eq[5], (udword)&streamdelay[0] },
    { "eq6", scriptSetReal32CB, (udword)&streamdelay[0].eq[6], (udword)&streamdelay[0] },
    { "eq7", scriptSetReal32CB, (udword)&streamdelay[0].eq[7], (udword)&streamdelay[0] },
    endEntry
};

scriptStructEntry streamEffectScriptTable[] =
{
    { "ClockCount", scriptSetUdwordCB, (udword)&cleaneffect[0].nClockCount, (udword)&cleaneffect[0] },
    { "FilterMinFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nFiltMinFreq, (udword)&cleaneffect[0] },
    { "FilterMaxFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nFiltMaxFreq, (udword)&cleaneffect[0] },
    { "ToneMinFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneMinFreq, (udword)&cleaneffect[0] },
    { "ToneMaxFreq", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneMaxFreq, (udword)&cleaneffect[0] },
    { "ToneDuration", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneDur, (udword)&cleaneffect[0] },
    { "ToneMute", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneMute, (udword)&cleaneffect[0] },
    { "ToneCount", scriptSetUdwordCB, (udword)&cleaneffect[0].nToneCount, (udword)&cleaneffect[0] },
    { "BreakMaxRate", scriptSetUdwordCB, (udword)&cleaneffect[0].nBreakMaxRate, (udword)&cleaneffect[0] },
    { "BreakMaxDur", scriptSetUdwordCB, (udword)&cleaneffect[0].nBreakMaxDur, (udword)&cleaneffect[0] },
    { "QNoiseMaxRate", scriptSetUdwordCB, (udword)&cleaneffect[0].nQNoiseMaxRate, (udword)&cleaneffect[0] },
    { "QNoiseMaxDur", scriptSetUdwordCB, (udword)&cleaneffect[0].nQNoiseMaxDur, (udword)&cleaneffect[0] },
    { "ScaleLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fScaleLev, (udword)&cleaneffect[0] },
    { "NoiseLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fNoiseLev, (udword)&cleaneffect[0] },
    { "ToneLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fToneLev, (udword)&cleaneffect[0] },
    { "LimitLevel", scriptSetReal32CB, (udword)&cleaneffect[0].fLimitLev, (udword)&cleaneffect[0] },
    { "PitchShift", scriptSetReal32CB, (udword)&cleaneffect[0].fPitchShift, (udword)&cleaneffect[0] },
    endEntry
};


/*=============================================================================
    Functions
=============================================================================*/
void SEloadscripts(void)
{
    sdword i;
    sdword buffersize;
    sdword streamersize;
    char loadfile[100];
    uword bitrate;

    soundstreamquery(SE_NUM_STREAMS, &buffersize, &streamersize);
    dbgAssert(buffersize != 0);

    pspeechstream = memAlloc(streamersize, "StreamStructures", NonVolatile);
    soundstreaminit(pspeechstream, streamersize, SE_NUM_STREAMS, soundEventDebugPrint);
    bitrate = speechEventBitrate();

    pspeechbuffer0 = memAlloc(buffersize, "SpeechBuffer0", NonVolatile);
    streamhandle[0] = soundstreamcreatebuffer(pspeechbuffer0, buffersize, bitrate);
    pspeechbuffer1 = memAlloc(buffersize, "SpeechBuffer1", NonVolatile);
    streamhandle[1] = soundstreamcreatebuffer(pspeechbuffer1, buffersize, bitrate);
    pspeechbuffer2 = memAlloc(buffersize, "SpeechBuffer2", NonVolatile);
    streamhandle[2] = soundstreamcreatebuffer(pspeechbuffer2, buffersize, bitrate);
#if defined(Downloadable) || defined(DLPublicBeta)
    pspeechbuffer4 = memAlloc(buffersize, "SpeechBuffer4", NonVolatile);
    streamhandle[4] = soundstreamcreatebuffer(pspeechbuffer4, buffersize, bitrate);
#else
    pspeechbuffer3 = memAlloc(buffersize, "SpeechBuffer3", NonVolatile);
    streamhandle[3] = soundstreamcreatebuffer(pspeechbuffer3, buffersize, bitrate);
    pspeechbuffer4 = memAlloc(buffersize, "SpeechBuffer4", NonVolatile);
    streamhandle[4] = soundstreamcreatebuffer(pspeechbuffer4, buffersize, bitrate);
#endif
    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, musicHeaderName);
    fileLoadAlloc(loadfile, &musicheader, NonVolatile);

    strcpy(loadfile, utyMusicFilename);
    {
        sdword musicChecksum = soundstreamopenfile(loadfile, &musicfilehandle);
        if (musicChecksum != musicheader->checksum)
        {
            dbgMessage("Music data file does not match lookup table.  Not from same generate.\n");
            dbgAssert(FALSE);
        }
    }

    pmusicbuffer0 = memAlloc(buffersize, "MusicBuffer", NonVolatile);
    musicinfo[NISSTREAM].handle = soundstreamcreatebuffer(pmusicbuffer0, buffersize, (uword)FQ_BR88);
    pmusicbuffer1 = memAlloc(buffersize, "MusicBuffer", NonVolatile);
    musicinfo[AMBIENTSTREAM].handle = soundstreamcreatebuffer(pmusicbuffer1, buffersize, (uword)FQ_BR88);

    for (i = NISSTREAM; i <= AMBIENTSTREAM; i++)
    {
        musicinfo[i].status = SOUND_FREE;
        musicinfo[i].tracknum = SOUND_DEFAULT;
        musicinfo[i].nexttrack = SOUND_DEFAULT;
        musicinfo[i].targetvol = SOUND_DEFAULT;
    }

    scriptSetStruct("SoundFX\\Scripts\\", "eq_fleet.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_FLEET]);
    scriptSetStruct("SoundFX\\Scripts\\", "eq_intel.script", streamEQScriptTable, (ubyte *)&streamEQ[SPEECH_INTEL]);

    // Cataclysm only ships fleet/intel EQ scripts; the remaining speech classes stay at neutral defaults.
    streamEQ[SPEECH_FLEET].eq[8] = streamEQ[SPEECH_FLEET].eq[7];
    streamEQ[SPEECH_INTEL].eq[8] = streamEQ[SPEECH_INTEL].eq[7];

    /* copy the unchanging effects to the mixed effect */
    for (i = 0;  i < SE_NUM_ACTORS; i++)
    {
        mixedeffect[i].nFiltMinFreq = cleaneffect[i].nFiltMinFreq;
        mixedeffect[i].nFiltMaxFreq = cleaneffect[i].nFiltMaxFreq;
        mixedeffect[i].fScaleLev = cleaneffect[i].fScaleLev;
        mixedeffect[i].fLimitLev = cleaneffect[i].fLimitLev;
    }

    streamStartThread();
}


void SEinitdelayEQ(void)
{
    sdword i;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        streamdelay[i].flags = STREAM_FLAGS_NOEFFECT;
        streamdelay[i].level = 0.0f;
        streamdelay[i].duration = 0;
        memset(streamdelay[i].eq, 0, SOUND_EQ_SIZE * sizeof(real32));
//      streamEQ[i].flags = STREAM_FLAGS_NOEFFECT;
//      memset(streamEQ[i].eq, 0, SOUND_EQ_SIZE * sizeof(real32));
        fqInitE(&cleaneffect[i]);
        fqInitE(&mixedeffect[i]);
    }
    fqInitE(&damageeffect);
}


void soundEventDebugPrint(char *pszInformation)
{
#if SE_VERBOSE_LEVEL >= 1
    dbgMessagef("\n%s", pszInformation);
#endif
}


sdword speechEventInit(ShipRace playerRace)
{
    char loadfile[100];
    const char *racePrefix = speechEventRacePrefix(playerRace);

    if (SentenceLUT != NULL)
    {
        memFree(SentenceLUT);
        SentenceLUT = NULL;
    }
    if (PhraseLUT != NULL)
    {
        memFree(PhraseLUT);
        PhraseLUT = NULL;
    }
    if (speechfilehandle != 0)
    {
        fileClose(speechfilehandle);
        speechfilehandle = 0;
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, racePrefix);
    strcat(loadfile, "Sentence.lut");
    fileLoadAlloc(loadfile, &SentenceLUT, NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, racePrefix);
    strcat(loadfile, "Phrase.lut");
    fileLoadAlloc(loadfile, &PhraseLUT, NonVolatile);

    if (SentenceLUT->checksum != PhraseLUT->checksum)
    {
        dbgMessage("Sentence and Phrase lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, racePrefix);
    strcat(loadfile, "Speech.vce");
    if (soundstreamopenfile(loadfile, &speechfilehandle) != SentenceLUT->checksum)
    {
        dbgMessagef("\nVoice file '%s' does not match lookup tables (checksum mismatch). Speech may not work correctly.", loadfile);
        if (speechfilehandle == 0)
        {
            dbgMessagef("\nVoice file not found - speech disabled.");
            return (SOUND_ERR);
        }
    }

    if (!speechEventLoaded)
    {
        SEinitdelayEQ();
        SEloadscripts();
        speechEventResetQueues();
        speechEventLoaded = TRUE;
    }
    else
    {
        numSinglePlayerEvents = 0;
    }

    return (SOUND_OK);
}


void speechEventClose(void)
{
    if (musicfilehandle != 0)
    {
        fileClose(musicfilehandle);
        musicfilehandle = 0;
    }
    if (speechfilehandle != 0)
    {
        fileClose(speechfilehandle);
        speechfilehandle = 0;
    }

    memFree(pmusicbuffer0);
    pmusicbuffer0 = NULL;
    memFree(pmusicbuffer1);
    pmusicbuffer1 = NULL;
    memFree(musicheader);
    musicheader = NULL;

    memFree(pspeechstream);
    pspeechstream = NULL;
    memFree(pspeechbuffer0);
    pspeechbuffer0 = NULL;
    memFree(pspeechbuffer1);
    pspeechbuffer1 = NULL;
    memFree(pspeechbuffer2);
    pspeechbuffer2 = NULL;
#if defined(Downloadable) || defined(DLPublicBeta)
    memFree(pspeechbuffer4);
    pspeechbuffer4 = NULL;
#else
    memFree(pspeechbuffer3);
    pspeechbuffer3 = NULL;
    memFree(pspeechbuffer4);
    pspeechbuffer4 = NULL;
#endif
    memFree(SentenceLUT);
    SentenceLUT = NULL;
    memFree(PhraseLUT);
    PhraseLUT = NULL;
    speechEventLoaded = FALSE;
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechEventUpdate(void)
{
    /* must call this so the streamer gets updated */
    soundupdate();
    speechQueueUpdate();

    if (speechEnemyPresenceLastWarningTime + 60.0f < universe.totaltimeelapsed)
    {
        if (speechEnemyPresencePendingFleetStatus76)
        {
            speechEventQueue(0, 33558604, -1, -1, 0, -1, -1, 5.0f, -1);
            speechEnemyPresencePendingFleetStatus76 = 0;
            speechEnemyPresenceLastWarningTime = universe.totaltimeelapsed;
        }

        if (speechEnemyPresencePendingResourceInjectionReceived)
        {
            speechEventQueue(0, 33558714, -1, -1, 0, -1, -1, 5.0f, -1);
            speechEnemyPresencePendingResourceInjectionReceived = 0;
            speechEnemyPresenceLastWarningTime = universe.totaltimeelapsed;
        }

        if (speechEnemyPresencePendingUnderAttackFromAlly)
        {
            speechEventQueue(0, 33558715, -1, -1, 0, -1, -1, 5.0f, -1);
            speechEnemyPresencePendingUnderAttackFromAlly = 0;
            speechEnemyPresenceLastWarningTime = universe.totaltimeelapsed;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :   this function updates the queues, starts playing new events,
                    reorders and cleans up the queue
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if 1
void speechQueueUpdate(void)
{
    sdword  i;
    SPEECHQUEUE *pSQueue;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        pSQueue = &speechqueue[i];

        /* is this channel playing something? */
        if (pSQueue->status >= SOUND_PLAYING)
        {
            /* is it over yet? */
            if (soundstreamover(streamhandle[i]))
            {
#if SE_VERBOSE_LEVEL >= 2
                dbgMessagef("\nQueue %d, stopped", i);
#endif
                /* was it a single player event? */
                if (pSQueue->current.event & SPEECH_SINGLEPLAYER_FLAG)
                {
                    numSinglePlayerEvents--;
                }
                if (pSQueue->nextevent >= SOUND_OK)
                {
                    if ((pSQueue->current.event == pSQueue->queue[pSQueue->nextevent].event) &&
                        (pSQueue->current.variable == pSQueue->queue[pSQueue->nextevent].variable))
                    {
                        memset(&pSQueue->queue[pSQueue->nextevent], 0, sizeof(QUEUEEVENT));
                        pSQueue->queue[pSQueue->nextevent].handle = SOUND_NOTINITED;
                        pSQueue->queue[pSQueue->nextevent].timeout = -1.0f;
                    }
                }
                pSQueue->current.event = SOUND_DEFAULT;
                pSQueue->status = SOUND_STOPPED;
                pSQueue->timeover = universe.totaltimeelapsed;
            }
            else
            {
                /* should it be shut off? */
            }
        }

        /* are we ready to play something? */
        if ((pSQueue->status == SOUND_STOPPED) && (pSQueue->nextevent >= SOUND_OK) && (!pSQueue->locked) &&
            (((universe.totaltimeelapsed - pSQueue->timeover) >= SPEECH_PAUSE) || !gameIsRunning 
#if BINK
                || !binkDonePlaying
#endif
                || FalkosFuckedUpTutorialFlag))
        {
            /* find the event that we should play */

            /* remove it from the queue */
            memcpy(&pSQueue->current, &pSQueue->queue[pSQueue->nextevent], sizeof(QUEUEEVENT));

            memset(&pSQueue->queue[pSQueue->nextevent], 0, sizeof(QUEUEEVENT));
            pSQueue->queue[pSQueue->nextevent].handle = SOUND_NOTINITED;
            pSQueue->queue[pSQueue->nextevent].timeout = -1.0f;

            /* find the next event */

            /* play this one */
            speechPlayQueue(pSQueue, i);
            if (pSQueue->current.event & SPEECH_SINGLEPLAYER_FLAG)
            {
                numSinglePlayerEvents++;
            }
            SEreorderqueue(pSQueue);

            pSQueue->status = SOUND_PLAYING;
        }

        /* find the event that we should play */
        /* remove it from the queue */

        /* reorder the queue */
        /* clean up the queue */

    }

}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechPlayQueue(SPEECHQUEUE *pSQueue, sdword streamchannel)
{
    speechPlayQueueSentence(pSQueue, streamchannel);
}

static void speechPlayQueueSentence(SPEECHQUEUE *pSQueue, sdword streamchannel)
{
    sdword i;
    sdword numoffsets;
    udword duration;
    sdword *phraseoffsets = NULL;
    real32 vol;
    sword queuedVol;
    sword finalVol;
    uword bitrate;
    sdword event;
    sdword actornum;

    event = pSQueue->current.event;
    actornum = pSQueue->current.actornum;
    if (event == SOUND_NOTINITED)
    {
        return;
    }

    bitrate = speechEventBitrate();
    numoffsets = SEselectsentence(actornum, (event & SPEECH_EVENT_MASK),
                                  pSQueue->current.variable, pSQueue->current.variation, &phraseoffsets, &duration);

    vol = (real32)SOUND_VOL_MAX * volSpeech;
    if ((numSinglePlayerEvents > 0) && !(event & SPEECH_SINGLEPLAYER_FLAG))
    {
        vol *= SPEECH_SINGLEPLAYER_RATIO;
    }

    if ((numoffsets > 0) && (vol >= SPEECH_MIN_PERCEPTABLE_VOL))
    {
        queuedVol = (sword)vol;
        finalVol = (sword)(vol + 0.5f);

        for (i = 0; i < numoffsets - 1; i++)
        {
            soundstreamqueuePatch(streamhandle[streamchannel], speechfilehandle, *phraseoffsets++, SOUND_FLAGS_QUEUESTREAM, queuedVol, SOUND_PAN_CENTER,
                                  SOUND_MONO, bitrate, NULL, &streamEQ[0], NULL, NULL, SOUND_VOL_MIN, 0.0f, 0.0f, actornum, event, TRUE);
        }

        soundstreamqueuePatch(streamhandle[streamchannel], speechfilehandle, *phraseoffsets++, SOUND_FLAGS_QUEUESTREAM, finalVol, SOUND_PAN_CENTER,
                              SOUND_MONO, bitrate, NULL, &streamEQ[0], NULL, NULL, SOUND_VOL_MIN, 0.0f, 0.0f, actornum, event, FALSE);

        if (pSQueue->current.pShip != NULL)
        {
            lastshiptospeak = pSQueue->current.pShip;
        }
        else if (event & SPEECH_GROUP_FLAG)
        {
            lastshiptospeak = (Ship *)-1;
            lastgrouptospeak = pSQueue->current.variable;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : SERandomSequenceCreate
    Description : Creates a number which is a random BCD sequence numVariations
                    in length containing all numbers between 0 and numVarations
                    once and only once.
    Inputs      : numVariations - number of variations to encode.
                  lookupsy - the lookup table which contains probabilities
                  width - width of the lookup table
    Outputs     :
    Return      : BCD-encoded random series
----------------------------------------------------------------------------*/
udword SERandomSequenceCreate(sdword numVariations, sdword *lookupsy, sdword width)
{
    sdword index, whichIndex;
    udword sequence[16];
    ubyte probs[16], accumProb = 0;
    udword finalSequence;

    dbgAssert(numVariations <= 16);                         //can only fit 16 BCD's in a DWORD
    //make an order 0, 1, 2, 3, ... and a list of matching probabilities
    for (index = 0; index < numVariations; index++)
    {
        sequence[index] = index;
        accumProb += probs[index] = ((lookupinterp *)(lookupsy))->probability - accumProb;
        lookupsy += width;
    }
    //partially bubble sort this list (this will favor the most common variations first)
    for (index = 0; index < numVariations - 1; index++)     //!!! might want to bubble sort a different number of times
    {
        whichIndex = ranRandom(RAN_SoundBothThreads) % (numVariations - 1);
        if (probs[whichIndex] <= probs[whichIndex + 1])
        {
            swap(sequence[whichIndex], sequence[whichIndex + 1], finalSequence);
            swap(probs[whichIndex], probs[whichIndex + 1], accumProb);
        }
    }
    //create the final sequence DWORD
    finalSequence = 0;
    for (index = numVariations - 1; index >= 0; index--)
    {
        finalSequence <<= 4;                                //free up a new BCD digit
        finalSequence = (finalSequence & 0xfffffff0) | (sequence[index] + 1);//stick in the new digit
    }
    return(finalSequence);                                  //done
}

/*-----------------------------------------------------------------------------
    Name        : SENextVariationInSeries
    Description : Select a variation of a speech event based on a random sequence
    Inputs      : numVariations - how many variations of this event
                  lookupsy - the lookup table
                  width - width of the lookup table
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword SENextVariationInSeries(sdword numVariations, sdword *lookupsy, sdword width)
{
    lookupinterp *interp = (lookupinterp *)(lookupsy);
    udword sequence;
    udword iVariation;

#if defined(Downloadable) || defined(DLPublicBeta)
    if (numVariations > 0xf)
    {                                                       //could be the case if this actor is disabled
        return(0);
    }
#endif
    dbgAssert(numVariations < 0xf);
    //get the random sequence into a dword
    sequence = (interp->high[1] << 24) + (interp->high[0] << 16) + (interp->low[1] << 8) + (interp->low[0]);

    if (sequence == 0)
    {                                                       //if no numbers left in sequence (tried all variations)
        sequence = SERandomSequenceCreate(numVariations, lookupsy, width);//start a new sequence
    }
    iVariation = (sequence & 0xf) - 1;                      //get a new variation
    sequence >>= 4;                                         //shift off those 4 bits
    dbgAssert(iVariation < numVariations);
    interp->high[1] = sequence >> 24;                       //put the sequence back in the high words
    interp->high[0] = (sequence >> 16) & 0xff;
    interp->low[1] = (sequence >> 8) & 0xff;
    interp->low[0] = (sequence) & 0xff;
#ifdef lmoloney
#if SE_VERBOSE_LEVEL >= 1
    dbgMessagef("\nSelected variation %d of %d for event at 0x%x of width %d", iVariation, numVariations, lookupsy, width);
#endif
#endif
    return(iVariation);                                     //return the variation we've found
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :   actor - which actor?
                    event - which event?
                    variable - value of the variable for this event
                    setvariation - select a particular variation, -1 is random
    Outputs     :   pOffsets - sets a pointer to the first offset for the sentence, NULL if there is an error
                    pDuration - sets the approximate duration of this sentence
    Return      :   the number of offsets in this sentence, -1 if there is an error
----------------------------------------------------------------------------*/
sdword SEselectsentence(sdword actor, sdword event, sdword variable, sdword setvariation, sdword **pOffsets, udword *pDuration)
{
    uword *sentenceRow;
    uword numVariations;
    uword *pNumSentences;
    sdword result;
    udword sentenceIdx;
    uword baseOffset;
    sdword numSentences;
    sdword entryIdx;
    uword bankStride;
    sdword bankStrideFull;
    ubyte *bankEntry;
    udword variationSequence;
    ubyte selectedVariation;
    udword shiftedSequence;
    udword *phraseRow;
    sdword *patchList;
    udword validCount;
    uword lookupIndex;

    if ((udword)event >= (udword)SentenceLUT->numevents)
    {
        *pOffsets = NULL;
        return SOUND_ERR;
    }

    lookupIndex = SentenceLUT->numcolumns * (event + actor * SentenceLUT->numevents);
    sentenceRow = &SentenceLUT->lookup[lookupIndex];
    numVariations = sentenceRow[1];
    pNumSentences = &sentenceRow[2];
    if (numVariations == 0)
    {
        *pOffsets = NULL;
        return SOUND_ERR;
    }

    sentenceIdx = variable;
    baseOffset = sentenceRow[3];
    if (variable >= SOUND_OK)
    {
        numSentences = *pNumSentences;
        if (variable >= numSentences)
        {
#if !defined(HW_Release)
            // Non-parity safeguard: retail returns SOUND_ERR here, but keep the fatal
            // during reconstruction so bad HW1-era speech IDs are surfaced immediately.
            dbgFatalf(DBG_Loc,
                      "speech variable out of range: actor=%d event=%d var=%d max=%d",
                      actor,
                      event,
                      variable,
                      numSentences);
#endif
            *pOffsets = NULL;
            return SOUND_ERR;
        }
    }
    else
    {
        numSentences = *pNumSentences;
        sentenceIdx = ranRandom(RAN_SoundBothThreads) % numSentences;
    }

    if (setvariation == SOUND_NOTINITED || setvariation >= numVariations)
    {
        if (numVariations == 1)
        {
            entryIdx = sentenceIdx + baseOffset;
        }
        else
        {
            bankStride = PhraseLUT->numcolumns;
            bankStrideFull = numSentences * bankStride;
            bankEntry = (ubyte *)PhraseLUT + sizeof(PHRASELUT) +
                        sizeof(sdword) * bankStride * (sentenceIdx + baseOffset);
            variationSequence = bankEntry[6]
                              | ((udword)bankEntry[7] << 8)
                              | ((udword)bankEntry[10] << 16)
                              | ((udword)bankEntry[11] << 24);
            if (variationSequence == 0)
            {
                variationSequence = SERandomSequenceCreate(numVariations, (sdword *)bankEntry, bankStrideFull);
            }

            selectedVariation = (ubyte)variationSequence;
            shiftedSequence = variationSequence >> 4;
            bankEntry[11] = (ubyte)(shiftedSequence >> 24);
            bankEntry[7] = (ubyte)(shiftedSequence >> 8);
            bankEntry[10] = (ubyte)(shiftedSequence >> 16);
            bankEntry[6] = (ubyte)shiftedSequence;
            entryIdx = sentenceIdx + baseOffset + ((selectedVariation & 0xF) - 1) * numSentences;
        }
    }
    else
    {
        entryIdx = setvariation * numSentences + sentenceIdx + baseOffset;
    }

    phraseRow = (udword *)((ubyte *)PhraseLUT + sizeof(PHRASELUT) +
                           sizeof(sdword) * entryIdx * PhraseLUT->numcolumns);
    *pDuration = *phraseRow;
    result = (ubyte)phraseRow[2];
    patchList = (sdword *)(phraseRow + 3);
    *pOffsets = patchList;
    if (result)
    {
        validCount = 0;
        while (*patchList != SOUND_NOTINITED)
        {
            validCount++;
            patchList++;
            if (validCount >= (udword)result)
            {
                return result;
            }
        }
        return 0;
    }

    return result;
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :   this function updates the actual speech that is playing,
                    decides when to fade volumes or cut off particular events
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechUpdateSpeech(void)
{
    /* what is the most important speech? */
    /* should something else be shut off? */
    /* are we past the max # of events that should be playing? */


}


/*-----------------------------------------------------------------------------
    Name        :   speechEventQueue
    Description :   Adds speech events to the queue if there is space for them.  Does trivial rejection on
                    low priority speech events.
    Inputs      :   object - pointer to a ship object that this speech event is attached to (can be NULL for Fleet Command, etc)
                    event - event # to play, define in speechevent.h
                    var - variable that may be attached to an event (ie a build ship event would pass the ship variable #)
                    variation - only used if you want a specific variation of an event to play
                    actornum - used if you want to override the actor that the speech code has selected
                    playernum - the playerindex in the array player in the universe structure, only want to hear your own speech events
                    linkto - handle of the speech event that you want to tie this one to, will play this event directly after that event
                    timeout - amount of time this event can wait in the queue, 0 means play now, < 0 means wait indefinatly and don't drop this from the queue
                    volume - volume at which to play this event, if you want to override the code that figures that out
    Outputs     :
    Return      :   handle to this speech event, can be used to stop the event or link another event to this one
                    or SOUND_ERR (-1) if there is an error or the event can't be queued
----------------------------------------------------------------------------*/
sdword speechEventQueue(void *object, sdword event, sdword var, sdword variation, sdword actornum, sdword playernum, sdword linkto, real32 timeout, sword volume)
{
    sdword actor = 0;
    sdword i, j;
    Ship *pShip = NULL;
    sdword handle = SOUND_ERR;
    SPEECHQUEUE *pSQueue;
    QUEUEEVENT *pQEvent;
    SpaceObj *spaceobject;
    sdword channel = 0;
    sdword ally = SOUND_ERR;

#if !defined(HW_Release) && CATACLYSM_DIAG
    if ((event & SPEECH_EVENT_MASK) == 133)
    {
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL
                    "queue row133 event=0x%X var=%d variation=%d actor=%d player=%d link=%d object=0x%X caller=0x%X",
                    event,
                    var,
                    variation,
                    actornum,
                    playernum,
                    linkto,
                    (udword)object,
                    (udword)_ReturnAddress());
    }
#endif

    if (!enableSpeech || soundpaused)
    {
        return (SOUND_ERR);
    }

    if ((event & 0x7FFu) > 0x35Eu)
    {
        return (SOUND_ERR);
    }

    /* if an NIS is playing, only want to hear NIS events */
//  if (nisIsRunning && (event < SPEECH_FIRST_SP_EVENT))
if (nisIsRunning && !(event & (SPEECH_NIS_FLAG + SPEECH_ANIMATIC_FLAG)))
{
    return (handle);
}

    /* if single player events are happening, don't want any chatter */
    if ((numSinglePlayerEvents > 0) && (event & SPEECH_CHATTER_FLAG))
    {
        return (handle);
    }

    /* toss out events the user doesn't want to hear */
    if ((event & SPEECH_CHATTER_FLAG) && !bChatterOn)
    {
        return (handle);
    }
    else if ((event & SPEECH_STATUS_FLAG) && !bStatusOn)
    {
        return (handle);
    }
    else if ((event & SPEECH_COMMAND_FLAG) && !bCommandsOn)
    {
        return (handle);
    }

#if 1       // enable when new speech is put in game
    if (!((event & SPEECH_FLAG_MASK) & actorFlagsEnabled))
    {
        return (handle);
    }
#endif

    if ((object != NULL) && !(event & ACTOR_FLEETCOMMAND_FLAG))
    {
        /* this is probably a ship, so this is one of the ALL SHIPS voices */
        spaceobject = (SpaceObj *)object;

        if (!univSpaceObjInRenderList(spaceobject) && ((event & SPEECH_STATUS_FLAG) || (event & SPEECH_CHATTER_FLAG)))
        {
            if (!(event & SPEECH_ALWAYSPLAY_FLAG))
            {
                return (SOUND_ERR);        // only do sound events for objects in current mission sphere we are viewing
            }
        }

        if (spaceobject->objtype != OBJ_ShipType)
        {
            return (SOUND_ERR);
        }

        pShip = (Ship *)object;

        if ((universe.curPlayerIndex != pShip->playerowner->playerIndex) && (!nisIsRunning))
        {
            return (SOUND_ERR);
        }

        if ((event == COMM_Selection) &&
            ((pShip->specialFlags2 & (SPECIAL_2_DisabledState | SPECIAL_2_DockFlag)) != 0))
        {
            return (SOUND_ERR);
        }

    }
    else
    {
        if (object != NULL)
        {
            spaceobject = (SpaceObj *)object;
            if (spaceobject->objtype == OBJ_ShipType)
            {
                pShip = (Ship *)object;
            }
        }

        if (((udword)playernum != universe.curPlayerIndex) && (!nisIsRunning) && (playernum != SOUND_EVENT_DEFAULT))
        {
            return (SOUND_ERR);
        }

        /* this is either Fleet Command, Fleet Intel, Traders, Pirates or NIS speech*/
//      if ()
//      {
//      }

    }

    /* figure out which speech channel to use */
    if (((event & SPEECH_FLAG_MASK) == ACTOR_FLEETINTEL_FLAG) ||
        ((event & SPEECH_FLAG_MASK) == ACTOR_TRADERS_FLAG))
    {
        channel = 1;
    }
    else if ((event & SPEECH_CHATTER_FLAG) ||
             ((event & SPEECH_FLAG_MASK) == ACTOR_KHARSELIM_FLAG))
    {
        channel = 2;
    }
    else
    {
        channel = 0;
    }

    if (linkto >= SOUND_OK)
    {
        channel = linkto & 0xF;
    }

    pSQueue = &speechqueue[channel];

    /* is the queue full? */
    if (pSQueue->numqueued == SE_MAX_QUEUE)
    {
        /* should see if I can bump one of these? */
        return (SOUND_ERR);
    }

    /* is this going to time out before it'll play? */
    if ((timeout > 0.0f) && (pSQueue->timeover > universe.totaltimeelapsed + timeout))
    {
        return (SOUND_ERR);
    }

    if ((event == STAT_Strike_OutOfFuel) && (var > SOUND_DEFAULT))
    {
        event = STAT_Group_OutOfFuel;
    }
    else if ((event == STAT_Strike_LowOnFuel) && (var > SOUND_DEFAULT))
    {
        event = STAT_Group_LowOnFuel;
    }

    if (var == SOUND_DEFAULT)
    {
        var = 0;
    }


    if ((pSQueue->current.event == event) && (pSQueue->current.variable == var))
    {
        return (SOUND_ERR);
    }

    /* special relic users events */
    if ((event == COMM_F_AllianceFormed) || (event == COMM_F_AllianceBroken))
    {
        ally = var;
        var = 0;

        for (i = 0; i < MAX_RELIC_NAMES; i++)
        {
            if (strnicmp(playerNames[ally], relicPlayerNames[i], MAX_PERSONAL_NAME_LEN) == 0)
            {
                if (event == COMM_F_AllianceFormed)
                {
                    event = COMM_F_RelicAllianceFormed;
                }
                else
                {
                    event = COMM_F_RelicAllianceBroken;
                }
                var = i;
                break;
            }
        }
    }
    else if (event == STAT_F_RelicPlayerDies)
    {
        for (i = 0; i < MAX_RELIC_NAMES; i++)
        {
            if (strnicmp(playerNames[var], relicPlayerNames[i], MAX_PERSONAL_NAME_LEN) == 0)
            {
                var = i;
                break;
            }
        }
        if ((i == MAX_RELIC_NAMES) || (i == universe.curPlayerIndex))
        {
            return (handle);
        }
    }

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (pSQueue->queue[i].handle == SOUND_NOTINITED)
        {
            pQEvent = &pSQueue->queue[i];

            /* put it in this space */
            if (linkto < SOUND_OK)
            {
                handle = SNDcreatehandle(channel);
                pQEvent->index = 0;
            }
            else
            {
                handle = linkto;
                for (j = 0; j < SE_MAX_QUEUE; j++)
                {
                    if ((pSQueue->queue[j].handle == linkto) && (pSQueue->queue[j].index == 0))
                    {
                        break;
                    }
                }
                if (j < SE_MAX_QUEUE)
                {
                    pQEvent->index = pSQueue->queue[j].numlinks++;
                }
                else if (pSQueue->current.handle == linkto)
                {
                    pQEvent->index = pSQueue->current.numlinks++;
                }
                else
                {
                    /* the one you wanted to link to is gone */
                    return (SOUND_ERR);
                }
            }

#if SE_VERBOSE_LEVEL >= 2
            dbgMessagef("Speech Event Queued.  Actor: %d, Event: %d\n", actor, event);
#endif
            pQEvent->status = SOUND_OK;
            pQEvent->actornum = actor;

            if (event & SPEECH_GROUP_FLAG)
            {
                pQEvent->pShip = NULL;
            }
            else
            {
                pQEvent->pShip = pShip;
            }
            pQEvent->event = event;
            pQEvent->variable = (var >= 60) ? 60 : var;
            pQEvent->variation = variation;
            if ((event & SPEECH_EVENT_MASK) < (udword)SentenceLUT->numevents)
                pQEvent->priority = SentenceLUT->lookup[(event & SPEECH_EVENT_MASK) * SentenceLUT->numcolumns];
            else
                pQEvent->priority = 0;
            pQEvent->volume = volume;

            pQEvent->timein = universe.totaltimeelapsed;
            if (timeout >= 0.0f)
            {
                pQEvent->timeout = universe.totaltimeelapsed + timeout;
            }
            else
            {
                pQEvent->timeout = timeout;
            }
            pQEvent->handle = handle;
            SEreorderqueue(pSQueue);
            break;
        }
    }

    return (handle);
}


/*-----------------------------------------------------------------------------
    Name        :   speechEventStop
    Description :   stops a speech event that is currently playing and deletes any
                    linked speech events from the queue.
    Inputs      :   handle - the handle for this event that was return by speechEventQueue
                    fadetime - the amount of time to fade out this speech event (in seconds)
                    addstatic - add static noise ass the speech fades out
    Outputs     :
    Return      :   SOUND_OK (0) if successful, SOUND_ERR (-1) if not
----------------------------------------------------------------------------*/
sdword speechEventStop(sdword handle, real32 fadetime, bool32 addstatic)
{
    /* is this event still playing? */
    /* are there any events linked to this one in the queue? */
    return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
    Name        : speechEventActorStop
    Description : Stops all speech events from a particular actor.  Queued
                    events are discarded; playing events faded out.
    Inputs      : actorMask - bit fields for what actors' speech events to
                    delete.  Same as bit fields added to speech events.
                  fadeTime - amount of time to fade out over.
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword speechEventActorStop(udword actorMask, real32 fadetime)
{
    sdword i, j;

    dbgAssert((actorMask & (~ACTOR_ALL_ACTORS)) == 0);

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            if (speechqueue[i].queue[j].event & actorMask)
            {
                memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
                speechqueue[i].queue[j].handle = SOUND_NOTINITED;
                speechqueue[i].queue[j].timeout = -1.0f;
            }
        }

        if (speechqueue[i].current.event & actorMask)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, fadetime);
        }
    }

    return (SOUND_OK);
}


sdword speechEventStopAllSpecific(real32 fadetime, sdword speechType)
{
    sdword i, j;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        speechqueue[i].locked = TRUE;
        speechqueue[i].timeover = 0.0f;

        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            if (speechqueue[i].queue[j].event & speechType)
            {
                memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
                speechqueue[i].queue[j].handle = SOUND_NOTINITED;
                speechqueue[i].queue[j].timeout = -1.0f;
            }
        }

        /* need to stop the currently playing one */
        if (speechqueue[i].current.event & speechType)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, fadetime);
        }

        speechqueue[i].locked = FALSE;
    }

    return (SOUND_OK);
}


sdword speechEventRemoveShip(Ship *pShip)
{
    sdword i, j;

    if (pShip == lastshiptospeak)
    {
        lastshiptospeak = NULL;
    }

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        speechqueue[i].locked = TRUE;

        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            if (speechqueue[i].queue[j].pShip == pShip)
            {
                memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
                speechqueue[i].queue[j].handle = SOUND_NOTINITED;
                speechqueue[i].queue[j].timeout = -1.0f;
                speechqueue[i].queue[j].pShip = NULL;
            }
        }

        /* need to stop the currently playing one */
        if (speechqueue[i].current.pShip == pShip)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, 100);
            speechqueue[i].current.pShip = NULL;
        }

        speechqueue[i].locked = FALSE;
    }

    return (SOUND_OK);
}


sdword SEreorderqueue(SPEECHQUEUE *pSQueue)
{
    sdword i;
    sdword highpriority = 0;
    bool32 bSomethingQueued = FALSE;
    /* need to get the next event */

    SEcleanqueue(pSQueue);

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (pSQueue->queue[i].handle >= SOUND_OK)
        {
            bSomethingQueued = TRUE;
            if (pSQueue->queue[highpriority].handle < SOUND_OK)
            {
                highpriority = i;
            }
            else if ((pSQueue->queue[i].event & SPEECH_TYPE_MASK) > (pSQueue->queue[highpriority].event & SPEECH_TYPE_MASK))
            {
                highpriority = i;
            }
            else if ((pSQueue->queue[i].event & SPEECH_TYPE_MASK) == (pSQueue->queue[highpriority].event & SPEECH_TYPE_MASK))
            {
                if (pSQueue->queue[i].priority > pSQueue->queue[highpriority].priority)
                {
                    highpriority = i;
                }
                else if (pSQueue->queue[i].priority == pSQueue->queue[highpriority].priority)
                {
                    if (pSQueue->queue[i].timein < pSQueue->queue[highpriority].timein)
                    {
                        highpriority = i;
                    }
                }
            }
            else if (pSQueue->queue[i].priority > pSQueue->queue[highpriority].priority)
            {
                highpriority = i;
            }
            else if (pSQueue->queue[i].priority == pSQueue->queue[highpriority].priority)
            {
                if (pSQueue->queue[i].timein < pSQueue->queue[highpriority].timein)
                {
                    highpriority = i;
                }
            }
        }
    }

    if (bSomethingQueued)
    {
        pSQueue->nextevent = highpriority;
    }
    else
    {
        pSQueue->nextevent = SOUND_NOTINITED;
    }

    return (SOUND_OK);
}


sdword SEcleanqueue(SPEECHQUEUE *pSQueue)
{
    sdword i;

    for (i = 0; i < SE_MAX_QUEUE; i++)
    {
        if (pSQueue->queue[i].timeout >= 0.0f)
        {
            if ((pSQueue->queue[i].timeout < universe.totaltimeelapsed)
                && ((pSQueue->queue[i].event & SPEECH_TYPE_MASK) < SPEECH_TUTORIAL_FLAG))   // don't throw out Animatic, NIS, Single Player
            {                                                                               // or Tutorial speech events.
                /* get ride of this one */
                /* IS IT LINKED? */
                memset(&pSQueue->queue[i], 0, sizeof(QUEUEEVENT));
                pSQueue->queue[i].handle = SOUND_NOTINITED;
                pSQueue->queue[i].timeout = -1.0f;
            }
        }
    }
    return (SOUND_OK);
}


sdword speechEventCleanup(void)
{
    sdword i, j;

    numSinglePlayerEvents = 0;
    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
            speechqueue[i].queue[j].handle = SOUND_NOTINITED;
            speechqueue[i].queue[j].timeout = -1.0f;
        }

        if (speechqueue[i].current.handle > SOUND_NOTINITED)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, 0.5f);
        }

        memset(&speechqueue[i].current, 0, sizeof(QUEUEEVENT));
        speechqueue[i].current.handle = SOUND_NOTINITED;
        speechqueue[i].status = SOUND_STOPPED;
        speechqueue[i].nextevent = SOUND_NOTINITED;
        speechqueue[i].timeover = 0.0f;
        speechqueue[i].numqueued = 0;
    }
    return (SOUND_OK);
}

#else

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword speechEventPlay(void *object, sdword event, sdword var, sdword playernum, sdword variation)
{
#if SPEECH
//    udword i;
    sdword actor = ACTOR_FLEETCOMMAND;
    sdword requestedEvent = event;
    sword vol = SOUND_VOL_MAX;
    sword pan = SOUND_PAN_CENTER;
    double dist;
    float damageratio = 0.0f;
    float distratio = (float)1.0;
    Ship *ship;
    SpaceObj *spaceobject;
    ShipStaticInfo *shipstatic;
    sdword curstream = SOUND_ERR;
    bool32 bookend = FALSE;
    ShipClass shipclass = 0;

    if (!enableSpeech)
    {
        return (SOUND_ERR);
    }

    if (event == STAT_Cap_ShipDocked)
    {
        return (SOUND_ERR);
    }

    /* figure out if there is a stream free to be played on */
//    for (i = 0; i < SE_NUM_STREAMS; i++)
//    {
//        if (soundstreamnumqueued(streamhandle[i]) == 0)
//        {
//            curstream = i;
//            break;
//        }
//    }
    if (soundstreamnumqueued(streamhandle[0]) == 0)
    {
        curstream = 0;
    }

    if (curstream == SOUND_ERR)
    {
        if ((event != COMM_Const_BuildCapShip) || (event != STAT_F_Research_CompletedShip))
        {
            return (SOUND_ERR);
        }

        curstream = 0;
    }

    if (object != NULL)
    {
        spaceobject = (SpaceObj *)object;

        if (spaceobject->objtype != OBJ_ShipType)
        {
            return (SOUND_ERR);
        }

        ship = (Ship *)object;
        shipstatic = (ShipStaticInfo *)ship->staticinfo;

        if (universe.curPlayerIndex != ship->playerowner->playerIndex)
        {
            return (SOUND_ERR);
        }

        if (!univSpaceObjInRenderList(spaceobject))
        {
            return (SOUND_ERR);        // only do sound events for objects in current mission sphere we are viewing
        }

//      if (event >= SPEECH_FIRST_SP_EVENT)
        if (event & SPEECH_SINGLEPLAYER_FLAG)
        {
            shipclass = CLASS_Fighter;
        }
        else
        {
            shipclass = ship->staticinfo->shipclass;
        }

        if (shipclass == CLASS_Mothership)
        {
            if (event == COMM_Selection)
            {
                event = COMM_F_MoShip_Selected;
            }
            playernum = universe.curPlayerIndex;
        }
        else
        {
            if ((ship->soundevent.actorNum <= SOUND_NOTINITED) || (ship->soundevent.actorNum >= SentenceLUT->numactors))
            {
                ship->soundevent.actorNum = actor = SEselectactor();
            }
            else
            {
                actor = ship->soundevent.actorNum;
            }

            damageratio = 1.0f - ((real32)ship->health * (real32)shipstatic->oneOverMaxHealth);      // (real32)shipstatic->maxhealth);

            dist = fsqrt(spaceobject->cameraDistanceSquared);

            vol = SOUND_VOL_MAX - (sword)(SPEECH_VOL_FACTOR * dist);
            if (vol < SPEECH_VOL_LOW)
            {
                vol = SPEECH_VOL_LOW;
            }
            else if (vol > SPEECH_VOL_MAX)
            {
                vol = SPEECH_VOL_MAX;
            }

            /* add the digital bleeps */
            bookend = TRUE;
        }
    }

    if (actor == ACTOR_FLEETCOMMAND)
    {
        if ((udword)playernum != universe.curPlayerIndex)
        {
            return (SOUND_ERR);
        }

        if (event == COMM_F_MoShip_Arrived)
        {
            if (MoShipMoving[universe.curPlayerIndex])
            {
                MoShipMoving[universe.curPlayerIndex] = FALSE;
            }
            else
            {
                return (SOUND_ERR);
            }
        }
        vol = SPEECH_VOL_MAX;
    }

    if (SEspeechevent(curstream, actor, event, var, vol, pan, 0.0f, 0.0f, variation, bookend, shipclass) == SOUND_OK)
    {
        /* set the last ship to speak variable so the hot key can be used later */
        if (actor == ACTOR_FLEETCOMMAND)
        {
            /* MAKE LAST SHIP BE THE MOTHERSHIP */
            if (universe.curPlayerPtr != NULL)
            {
                lastshiptospeak = universe.curPlayerPtr->PlayerMothership;
            }
            else
            {
                lastshiptospeak = NULL;
            }
        }
        else
        {
            lastshiptospeak = ship;
        }

    }


    if (event == COMM_Cloak_CloakingOn)
    {
        if (++actor >= SentenceLUT->numactors - 1)
        {
            actor = 1;
        }

        SEspeechevent(curstream, actor, STAT_Cloak_CloakingOn_Resp, var, vol, pan, 0.0f, 0.0f, variation, bookend, shipclass);
    }
    if ((event == STAT_AssGrp_UnderAttack) || (event == STAT_Grp_UnderAttack))
    {
        if (++actor >= SentenceLUT->numactors - 1)
        {
            actor = 1;
        }

        SEspeechevent(curstream, actor, STAT_Grp_UnderAttack_Rsp, var, vol, pan, 0.0f, 0.0f, variation, bookend, shipclass);
    }

    if (event == COMM_F_MoShip_Move)
    {
        MoShipMoving[universe.curPlayerIndex] = TRUE;
    }

    return (streamhandle[curstream]);
#endif
}


sdword SEspeechevent(sdword stream, sdword actor, sdword event, sdword var, sword vol, sword pan, double dist, float damageratio, sdword setVariation, bool32 bookend, ShipClass shipclass)
{
    udword duration = 0;
    udword variation = 0;
#if SPEECH
    uword numvariations;
    uword maxvar;
    uword index;
    udword numoffsets;
    udword i;
    udword phrase;
    EFFECT *fqeffect = NULL;
    sword priority = 0;
    sword *pSentence;
    sdword *pPhrase;
    STREAMEQ *pEQ = NULL;
    STREAMDELAY *pdelay = NULL;
    void *ppatch = NULL;
    sword level = 0;

// NEW SPEECH
    udword probability;

    pSentence = &SentenceLUT->lookup[event * SentenceLUT->numcolumns];

    priority = *pSentence++;
    numvariations = *pSentence++;

#if 0
    for (i = 0; i < SentenceLUT->prioritylevels; i++)
    {
        pSentence = &SentenceLUT->lookup[((actor * SentenceLUT->numevents + event) * SentenceLUT->prioritylevels + i) * 3];
        if (*pSentence > 0)
        {
            numvariations = *pSentence++;
            break;
        }
    }
#endif
    if (numvariations == 0)
    {
        return (SOUND_ERR);
    }

    if (var < SOUND_OK)
    {
        var = 0;
//      return (SOUND_ERR);
    }

    maxvar = *pSentence++;
    index = *pSentence;

    if ((uword)var >= maxvar)
    {
#if !defined(HW_Release)
        dbgFatalf(DBG_Loc,
                  "speech variable out of range: actor=%d event=%d var=%d max=%u",
                  actor,
                  event,
                  var,
                  maxvar);
#endif
        var = 0;
    }

    if ((setVariation != SOUND_NOTINITED) && (setVariation < numvariations))
    {
        variation = setVariation;
    }
    else
    {
        variation = ranRandom(RAN_SoundGameThread) % numvariations;
    }

    if (event == COMM_F_Hyper_Engage)
    {
        variation = 1;
    }
    else if (event == STAT_Grav_Collapse)
    {
        variation = 1;
    }

    phrase = index + (variation * maxvar) + var;

    pPhrase = &PhraseLUT->lookup[phrase * PhraseLUT->numcolumns];

    duration = *pPhrase++;
// NEW SPEECH
    probability = *pPhrase++;
    numoffsets = *pPhrase++;

    if (numoffsets > 0)
    {
        for (i = 0; i < numoffsets; i++)
        {
            if (*(pPhrase + i) == SOUND_NOTINITED)
            {
                // bad, this won't work
                numoffsets = 0;
                break;
            }
        }
    }

    /* set up the effect structure */
//    fqInitE(&fqeffect);

    if (actor != ACTOR_FLEETCOMMAND)
    {
        pEQ = &streamEQ[SPEECH_PILOT1];

        if ((event >= CHAT_Const_DuringBattle) && (event <= CHAT_Const_More))
        {
            pdelay = &streamdelay[SPEECH_CONSTRUCTION];
        }

        if ((shipclass == CLASS_Corvette) || (shipclass == CLASS_Fighter))
        {
            /* filter the fighters */
            pdelay = &streamdelay[SPEECH_STRIKECRAFT];
            fqeffect = &cleaneffect[SPEECH_STRIKECRAFT];
//          vol = (sword)(vol * 1.5f);
//          if (vol > SOUND_VOL_MAX)
//          {
//              vol = SOUND_VOL_MAX;
//          }
        }
        if ((shipclass >= CLASS_HeavyCruiser) && (shipclass <= CLASS_Frigate))
        {
            ppatch = SNDgetpatch(SpecialEffectBank, SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_IntCapShipCalm)]);
            level = 128;
        }
        if (shipclass == CLASS_Resource)
        {
            ppatch = SNDgetpatch(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, sWorker, ShipCmn_Ambient)]);
            level = 128;
        }
#if 0
        /* adjust frequency effects here */
        fqeffect.nFiltMinFreq = SPEECH_FILTER_LOW;
        fqeffect.nFiltMaxFreq = SPEECH_FILTER_HIGH;

        fqeffect.fNoiseLev = SPEECH_NOISE_LOW + ((float)dist * SPEECH_NOISE_FACTOR);    /// (float)150.0;
        if (fqeffect.fNoiseLev > SPEECH_NOISE_HIGH)
        {
            fqeffect.fNoiseLev = SPEECH_NOISE_HIGH;
        }
        fqeffect.fNoiseLev *= FQ_FNOISE;

        if (damageratio > SPEECH_BREAK_THRESHOLD)
        {
            fqeffect.nBreakMaxRate = SPEECH_BREAK_RATE_LOW + (udword)(SPEECH_BREAK_RATE_FACTOR * damageratio);
            if (fqeffect.nBreakMaxRate > SPEECH_BREAK_RATE_HIGH)
            {
                fqeffect.nBreakMaxRate = SPEECH_BREAK_RATE_HIGH;
            }
            fqeffect.nBreakMaxDur = SPEECH_BREAK_LENGTH_LOW + (udword)(SPEECH_BREAK_LENGTH_FACTOR * damageratio);
            if (fqeffect.nBreakMaxDur < SPEECH_BREAK_LENGTH_HIGH)
            {
                fqeffect.nBreakMaxDur = SPEECH_BREAK_LENGTH_HIGH;
            }
        }
#endif
    }
    else
    {
        pEQ = &streamEQ[SPEECH_FLEET];
    }

    if (numoffsets > 0)
    {
        if (bookend && enableSFX)
        {
            soundstreamqueue(streamhandle[stream], (sdword)SpecialEffectBank,
                             (sdword)SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_RadioBeepStart)],
                             SOUND_FLAGS_QUEUEPATCH, vol, pan, SOUND_MONO, SOUND_DEFAULT, NULL, NULL, NULL, pSQueue->current.event);

//          soundstreamqueueSilence(streamhandle[stream], SOUND_FLAGS_QUEUESILENCE, vol, pan, SOUND_MONO,
//                                  SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, SEsilence());
        }

        for (i = 0; i < numoffsets; i++)
        {
//            soundstreamqueue(streamhandle[stream], speechfilehandle, *pPhrase++, 0, vol, pan, SOUND_MONO, SentenceLUT->compbitrate, &fqeffect, pEQ, pdelay);
//            soundstreamqueue(streamhandle[stream], speechfilehandle, *pPhrase++, SOUND_FLAGS_QUEUESTREAM, vol, pan, SOUND_MONO, SentenceLUT->compbitrate, fqeffect, pEQ, pdelay);
            soundstreamqueuePatch(streamhandle[stream], speechfilehandle, *pPhrase++, SOUND_FLAGS_QUEUESTREAM, vol, pan,
                                  SOUND_MONO, speechEventBitrate(), fqeffect, pEQ, pdelay, ppatch, level, 0.0f, actor);
        }

        if (bookend && enableSFX)
        {
//          soundstreamqueueSilence(streamhandle[stream], SOUND_FLAGS_QUEUESILENCE, vol, pan, SOUND_MONO,
//                                  SentenceLUT->compbitrate, fqeffect, pEQ, pdelay, SEsilence());

            soundstreamqueue(streamhandle[stream], (sdword)SpecialEffectBank,
                             (sdword)SpecEffectEventsLUT->lookup[GetPatch(SpecEffectEventsLUT, 0, Spec_RadioBeepEnd)],
                             SOUND_FLAGS_QUEUEPATCH, vol, pan, SOUND_MONO, SOUND_DEFAULT, NULL, NULL, NULL, pSQueue->current.event);
        }
    }
#endif

//    dbgMessagef("\nspeechEvent: %d, variable: %d, variation: %d, duration %d msec, text: %s",
//                event, var, variation, duration, aSpeechEventDebug[event]);

    return (SOUND_OK);
}

#endif

real32 SEsilence(void)
{
    return (0.5f);
}


sdword SEselectactor(void)
{
    sdword actor;

#if defined(Downloadable) || defined(DLPublicBeta)
    actor = (ranRandom(RAN_SoundGameThread) % 2) + 1;
#else
    actor = (ranRandom(RAN_SoundGameThread) % 3) + 1;
#endif

    if (!bActorOn[actor])
    {
        actor++;

#if defined(Downloadable) || defined(DLPublicBeta)
            if (actor > 2)
#else
            if (actor > 3)
#endif
        {
            actor = 1;
        }

        if (!bActorOn[actor])
        {
            actor++;

#if defined(Downloadable) || defined(DLPublicBeta)
            if (actor > 2)
#else
            if (actor > 3)
#endif
            {
                actor = 1;
            }

            if (!bActorOn[actor])
            {
                actor = SOUND_NOTINITED;
            }
        }
    }

    return (actor);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 speechEventAttack(void)
{
    sdword objtype,
            i;
    Ship *ship;
    sdword  numFriendlies = 0,
            numEnemies = 0,
            resourceTypes = 0,
            numResources = 0;
    static bool32 lastreturn;

    if (keyIsHit(SHIFTKEY))
    {
        for (i = 0; i < selSelecting.numTargets; i++)
        {
            objtype = selSelecting.TargetPtr[i]->objtype;
            if (objtype == OBJ_ShipType)
            {
                ship = (Ship *)selSelecting.TargetPtr[i];
                if (ship->playerowner == universe.curPlayerPtr)
                {
                    numFriendlies++;
                }
                else
                {
                    numEnemies++;
                }
            }
            else if ((objtype >= OBJ_AsteroidType) && (objtype <= OBJ_DustType))
            {
                resourceTypes |= (1 << objtype);
                numResources++;
            }
        }

        // are there any friendlies
        if (numFriendlies > 0)
        {
            // do we attack?
            if (((ranRandom(RAN_SoundGameThread) % 100) <= SPEECH_DISOBEY_FORCEDATTACK) && (lastreturn))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackPlayerUnits_Cond, 0);
                lastreturn = FALSE;
                return (lastreturn);
            }
            speechEvent(selSelected.ShipPtr[0], COMM_AttackPlayerUnits, 0);
        }
        else if (numEnemies > 0)
        {
            //else its only enemies
            speechEvent(selSelected.ShipPtr[0], COMM_Attack, 0);
        }
        // are there any resources
        else if (numResources > 0)
        {
            if (resourceTypes == (1 << OBJ_AsteroidType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, Asteroids);
            }
            else if (resourceTypes == (1 << OBJ_NebulaType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, Nebulae);
            }
            else if (resourceTypes == (1 << OBJ_GasType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, GasClouds);
            }
            else if (resourceTypes == (1 << OBJ_DustType))
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResources, DustClouds);
            }
            else
            {
                speechEvent(selSelected.ShipPtr[0], COMM_AttackResourcesGeneric, 0);
            }
        }
    }
    else
    {
        speechEvent(selSelected.ShipPtr[0], COMM_Attack, 0);
    }

    lastreturn = TRUE;
    return (lastreturn);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void speechEventUnderAttack(Ship *target)
{
    if (target->playerowner == universe.curPlayerPtr &&
        (target->specialFlags2 & 0x00100000) == 0)
    {
        if (target->staticinfo->shipclass == CLASS_Mothership ||
            target->staticinfo->shipclass == CLASS_Component)
        {
            if (target->shiptype != sDreadnoughtCannon &&
                target->shiptype != sCarrierSupport &&
                universe.totaltimeelapsed > (universe.curPlayerPtr->timeMoShipAttacked + SPEECH_MOSHIP_WARNING_TIME))
            {
                speechEventFleetSpec(target, STAT_F_MoShip_UnderAttack, 0, target->playerowner->playerIndex);
                universe.curPlayerPtr->timeMoShipAttacked = universe.totaltimeelapsed;
            }
        }
        else if (universe.totaltimeelapsed > (target->soundevent.timeLastStatus + SPEECH_WARNING_TIME))
        {
            if (univSpaceObjInRenderList((SpaceObj *)target))
            {
                if (target->staticinfo->shipclass == CLASS_Carrier ||
                    ShipTypeIsWorker(target->shiptype) ||
                    target->shiptype == kHeavyCruiser ||
                    target->shiptype == tHeavyCruiser ||
                    target->shiptype == bHeavyCruiser)
                {
                    if ((target->health / target->staticinfo->maxhealth) < 0.5f)
                    {
                        speechEvent(target, STAT_Local_BigShip_Damaged, target->shiptype);
                        target->soundevent.timeLastStatus = universe.totaltimeelapsed;
                    }
                }
                else if (selAnyHotKeyTest(target) && (universe.totaltimeelapsed > (selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus + SPEECH_WARNING_TIME)))
                {
                    speechEventQueue(NULL, STAT_F_AssGrp_UnderAttack_Alert, selHotKeyGroupNumberTest(target), -1, 0, -1, -1, 5.0f, -1);
                    selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus = universe.totaltimeelapsed;
                }
            }
            else
            {
                if (target->staticinfo->shipclass == CLASS_Carrier ||
                    target->shiptype == kHeavyCruiser ||
                    target->shiptype == tHeavyCruiser ||
                    target->shiptype == bHeavyCruiser)
                {
                    speechEventFleet(STAT_F_SuperCap_Damaged_Unrendered, target->shiptype, target->playerowner->playerIndex);
                    target->soundevent.timeLastStatus = universe.totaltimeelapsed;
                }
                else if (selAnyHotKeyTest(target) && (universe.totaltimeelapsed > (selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus + SPEECH_WARNING_TIME)))
                {
                    speechEventFleet(STAT_F_AssGrp_UnderAttack_Alert, selHotKeyGroupNumberTest(target), target->playerowner->playerIndex);
                    selHotKeyGroup[selHotKeyGroupNumberTest(target)].timeLastStatus = universe.totaltimeelapsed;
                }
            }
        }
    }
}


sdword musictranslatetracknum(sdword tracknum)
{
    sdword track;
#ifdef OEM
    switch (tracknum)
    {
        case AMB_Mission1:
            track = 0;
            break;
        case AMB_Mission2:
            track = 1;
            break;
        case AMB_Mission3:
            track = 2;
            break;
        case AMB_Mission5:
            track = 3;
            break;
        case AMB13_Tutorial:
            track = 4;
            break;
        case NIS01_R1:
            track = 5;
            break;
        case NIS01_R2:
            track = 6;
            break;
        case NIS02:
            track = 7;
            break;
        case NIS03:
            track = 8;
            break;
        case NIS04:
            track = 9;
            break;
        case ANIM00_Sierra:
            track = 10;
            break;
        case ANIM00_Relic:
            track = 11;
            break;
        case ANIM00_Opening:
            track = 12;
            break;
        case ANIM01_02:
            track = 13;
            break;
        case ANIM02_03:
            track = 14;
            break;
        case ANIM03_04:
            track = 15;
            break;
        case B01_TuranicRaidersLong:
            track = 16;
            break;
        case B02_TuranicRaidersShort:
            track = 17;
            break;
        case B04_EvilEmpire:
            track = 18;
            break;
        case RollCredits:
            track = 19;
            break;
        default:
            if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
            {
                track = 0;
            }
            else
            {
                track = -1;
            }
            break;
    }
#elif defined(CGW)
    switch (tracknum)
    {
        case AMB_Mission1:
            track = 0;
            break;
        case AMB_Mission2:
            track = 1;
            break;
        case AMB12_FrontEnd:
            track = 2;
            break;
        case AMB13_Tutorial:
            track = 3;
            break;
        case NIS01_R1:
            track = 4;
            break;
        case NIS01_R2:
            track = 5;
            break;
        case NIS02:
            track = 6;
            break;
        case ANIM00_Sierra:
            track = 7;
            break;
        case ANIM00_Relic:
            track = 8;
            break;
        case ANIM00_Opening:
            track = 9;
            break;
        case ANIM01_02:
            track = 10;
            break;
        case ANIM02_03:
            track = 11;
            break;
        default:
            if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
            {
                track = 0;
            }
            else
            {
                track = -1;
            }
            break;
    }
#elif defined(Downloadable)
    if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
    {
        track = 0;
    }
    else if (tracknum == NIS01_R1)
    {
        track = 1;
    }
    else if (tracknum == NIS02)
    {
        track = 2;
    }
    else if (tracknum == ANIM01_02)
    {
        track = 3;
    }
    else
    {
        track = -1;
    }
#elif defined(DLPublicBeta)
    if ((tracknum >= MUS_FIRST_AMBIENT) && (tracknum <= MUS_LAST_BATTLE))
    {
        track = 0;
    }
    else
    {
        track = -1;
    }
#else
    track = tracknum;
#endif
    return (track);
}


void MEstarttrack(MUSICINFO *pinfo, MUSICSTREAM *pstream)
{
    if (pinfo->tracknum == RollCredits)
    {
        pinfo->targetvol = (sword)(pstream->volume * volNIS);
    }
    else
    {
        pinfo->targetvol = (sword)(pstream->volume * volMusic);
    }

    soundstreamqueue(pinfo->handle, musicfilehandle,
                    pstream->offset,
                    pstream->flags | SOUND_FLAGS_QUEUESTREAM,
                    pinfo->targetvol,
                    SOUND_PAN_CENTER,
                    (sword)pstream->numchannels,
                    (sword)pstream->bitrate,
                    NULL, NULL, NULL, -1);

    pinfo->status = SOUND_STARTING;
}


// Cataclysm: all music plays on a single stream (AMBIENTSTREAM). Tracks 0-16
// are the ambient pool; track 1102 selects one at random. No separate NIS stream.
sdword musicEventPlay(sdword tracknum)
{
    MUSICINFO *pinfo;
    MUSICSTREAM *pmstream;


    if (!enableSpeech)
        return SOUND_ERR;

    if (tracknum < 0)
        return SOUND_ERR;

    if (tracknum == MUS_RANDOM_AMBIENT)
        tracknum = ranRandom(0) % 17;

    pmstream = &musicheader->mstreams[tracknum];
    pinfo = &musicinfo[AMBIENTSTREAM];

    if (soundstreamover(pinfo->handle))
    {
        pinfo->tracknum = tracknum;
        pinfo->nexttrack = SOUND_NOTINITED;
        MEstarttrack(pinfo, pmstream);
    }
    else
    {
        pinfo->nexttrack = tracknum;
        pinfo->status = SOUND_RESTART;
        pinfo->targetvol = SOUND_VOL_AUTOSTOP;
        soundstreamvolume(pinfo->handle, SOUND_VOL_AUTOSTOP, MUSIC_FADE_FAST);
    }

    return SOUND_OK;
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword musicEventStop(sdword tracknum, real32 fadetime)
{
    if (enableSpeech)
    {
        if (tracknum == SOUND_DEFAULT)
        {
            if ((musicinfo[AMBIENTSTREAM].status == SOUND_PLAYING) || (musicinfo[AMBIENTSTREAM].status == SOUND_STARTING))
            {
                tracknum = musicinfo[AMBIENTSTREAM].tracknum;
            }
        }

        if (tracknum >= MUS_FIRST_AMBIENT)
        {
            if (tracknum != musicinfo[AMBIENTSTREAM].tracknum)
            {
                return (SOUND_ERR);
            }

            if ((musicinfo[AMBIENTSTREAM].status == SOUND_PLAYING) || (musicinfo[AMBIENTSTREAM].status == SOUND_STARTING))
            {
                musicinfo[AMBIENTSTREAM].targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(musicinfo[AMBIENTSTREAM].handle, musicinfo[AMBIENTSTREAM].targetvol, fadetime);
                musicinfo[AMBIENTSTREAM].status = SOUND_STOPPING;
                musicinfo[AMBIENTSTREAM].tracknum = SOUND_ERR;
            }
        }
    }

    return (SOUND_OK);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword musicEventUpdateVolume(void)
{
    sword volume;
    MUSICINFO *pinfo;
    MUSICSTREAM *pmstream;

    if (enableSpeech)
    {
        /* Check the AMBIENT */
        pinfo = &musicinfo[AMBIENTSTREAM];

        if (pinfo->status == SOUND_FREE)
            return SOUND_OK;

        pmstream = &musicheader->mstreams[pinfo->tracknum];

        if (pinfo->status == SOUND_STARTING)
        {
            if (soundstreamgetvol(pinfo->handle) == pinfo->targetvol)
            {
                pinfo->status = SOUND_PLAYING;
            }
            else if ((!soundstreamfading(pinfo->handle)) && (!soundstreamover(pinfo->handle)))
            {
                soundstreamvolume(pinfo->handle, pinfo->targetvol, MUSIC_FADE_FAST);
            }
        }
        else if (pinfo->status == SOUND_PLAYING)
        {
            if (!soundstreamover(pinfo->handle))
            {
                if (mrRenderMainScreen)
                {
                    if (pinfo->tracknum == AMB13_Tutorial)
                    {
                        /* we're in the tutorial, play the music at a constant volume */
                        volume = (sword)(pmstream->volume * MUSIC_TUTORIAL_VOL * volMusic);
                    }
                    else
                    {
                        /* we're in the game, volume depends on the distance from the nearst ship */
                        volume = (sword)(pmstream->volume * volMusic);

                        if (nearestShipDistance <= MUSIC_DISTANCE_SILENT)
                        {
                            if (pinfo->tracknum < MUS_FIRST_BATTLE)
                            {
                                if (nearestShipMoving)
                                {
                                    volume = (sword)((real32)volume * MUSIC_MINACTIVE_VOL);
                                }
                                else
                                {
                                    volume = (sword)((real32)volume * MUSIC_MININACTIVE_VOL);
                                }
                            }
                            else
                            {
                                volume = (sword)((real32)volume * MUSIC_MINBATTLE_VOL);
                            }
                        }
                        else if (nearestShipDistance <= MUSIC_DISTANCE_MAX)
                        {
                            if (pinfo->tracknum < MUS_FIRST_BATTLE)
                            {
                                if (nearestShipMoving)
                                {
                                    volume = (sword)((real32)volume * (((nearestShipDistance - MUSIC_DISTANCE_SILENT) * musicVolFactor) + MUSIC_MINACTIVE_VOL));
                                }
                                else
                                {
                                    volume = (sword)((real32)volume * (((nearestShipDistance - MUSIC_DISTANCE_SILENT) * musicInactiveFactor) + MUSIC_MININACTIVE_VOL));
                                }
                            }
                            else
                            {
                                volume = (sword)((real32)volume * (((nearestShipDistance - MUSIC_DISTANCE_SILENT) * battleMusicVolFactor) + MUSIC_MINBATTLE_VOL));
                            }
                        }
                        else
                        {
                            if (pinfo->tracknum < MUS_FIRST_BATTLE)
                            {
                                volume = (sword)((real32)volume * MUSIC_MAXGAME_VOL);
                            }
                            else
                            {
                                volume = (sword)((real32)volume * MUSIC_MAXBATTLE_VOL);
                            }
                        }


                        if (volume > pmstream->volume)
                        {
                            volume = pmstream->volume;
                        }
                        else if (volume < 0)
                        {
                            volume = 0;
                        }
                    }
                }
                else
                {
                    if (smSensorsActive)
                    {
                        /* we're in the sensors manager, play the music at the volume defined in the script */
                        if (pinfo->tracknum < MUS_FIRST_BATTLE)
                        {
                            volume = (sword)(pmstream->volume * MUSIC_SENSORS_VOL * volMusic);
                        }
                        else
                        {
                            volume = (sword)(pmstream->volume * MUSIC_MINBATTLE_VOL * volMusic);
                        }
                    }
                    else
                    {
                        /* we're in one of the other manager screens */
                        if (pinfo->tracknum < MUS_FIRST_BATTLE)
                        {
                            volume = (sword)(pmstream->volume * MUSIC_MANAGERS_VOL * volMusic);
                        }
                        else
                        {
                            volume = (sword)(pmstream->volume * MUSIC_MINBATTLE_VOL * volMusic);
                        }
                    }
                }

                if (!bVolMusicNoFade)
                {
                    soundstreamvolume(pinfo->handle, volume, MUSIC_FADE_TIME);
                }
                else
                {
                    soundstreamvolume(pinfo->handle, volume, 0.0f);
                    bVolMusicNoFade = FALSE;
                }
            }
            else
            {
                pinfo->status = SOUND_FREE;
                musicEventPlay(pinfo->tracknum);
            }
        }
        else if (pinfo->status == SOUND_STOPPING)
        {
            if (soundstreamover(pinfo->handle))
            {
                pinfo->status = SOUND_FREE;
            }
            else if ((pinfo->targetvol != SOUND_VOL_AUTOSTOP) ||
                (!soundstreamfading(pinfo->handle) &&
                (soundstreamgetvol(pinfo->handle) > SOUND_VOL_MIN)))
            {
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
            }
        }
        else if (pinfo->status == SOUND_RESTART)
        {
            if (soundstreamover(pinfo->handle))
            {
                pinfo->tracknum = pinfo->nexttrack;
                pinfo->nexttrack = SOUND_NOTINITED;
                pmstream = &musicheader->mstreams[pinfo->tracknum];

                MEstarttrack(pinfo, pmstream);
            }
            else if ((pinfo->targetvol != SOUND_VOL_AUTOSTOP) ||
                (!soundstreamfading(pinfo->handle) &&
                (soundstreamgetvol(pinfo->handle) > SOUND_VOL_MIN)))
            {
                pinfo->targetvol = SOUND_VOL_AUTOSTOP;
                soundstreamvolume(pinfo->handle, pinfo->targetvol, SOUND_FADE_STOPTIME);
            }
        }
    }

    return (SOUND_OK);
}


#if defined(CGW) || defined(Downloadable) || defined(DLPublicBeta) || defined(OEM)

void musicEventNextTrack(void)
{
    return;
}

void musicEventPrevTrack(void)
{
    return;
}
#else

void musicEventNextTrack(void)
{
    MUSICINFO *pinfo;

    if (!singlePlayerGame && enableSpeech)
    {
        pinfo = &musicinfo[AMBIENTSTREAM];

        pinfo->nexttrack = pinfo->tracknum + 1;
        if (pinfo->nexttrack > MUS_LAST_BATTLE)
        {
            pinfo->nexttrack = MUS_FIRST_AMBIENT;
        }

#if SE_VERBOSE_LEVEL >= 2
        dbgMessagef("Starting track # %d\n", pinfo->nexttrack);
#endif
        pinfo->status = SOUND_RESTART;
        soundstreamvolume(pinfo->handle, SOUND_VOL_AUTOSTOP, MUSIC_FADE_FAST);
   }
}


void musicEventPrevTrack(void)
{
    MUSICINFO *pinfo;

    if (!singlePlayerGame && enableSpeech)
    {
        pinfo = &musicinfo[AMBIENTSTREAM];

        pinfo->nexttrack = pinfo->tracknum - 1;
        if (pinfo->nexttrack < MUS_FIRST_AMBIENT)
        {
            pinfo->nexttrack = MUS_LAST_BATTLE;
        }

#if SE_VERBOSE_LEVEL >= 2
        dbgMessagef("Starting track # %d\n", pinfo->nexttrack);
#endif
        pinfo->status = SOUND_RESTART;
        soundstreamvolume(pinfo->handle, SOUND_VOL_AUTOSTOP, MUSIC_FADE_FAST);
   }
}
#endif

sdword musicEventCurrentTrack(void)
{
    return (musicinfo[AMBIENTSTREAM].tracknum);
}

#if 0  // Cat2: Binary speech play functions; live code reimplements speechPlayQueue (L733), SEreorderqueue (L1446), and speechPlaySentence with a different actor structure (0x522E60, 0x521EB0, 0x5220E0)


// ============================================================================
// Decompiled from cataclysm.exe @ 0x522E60
sdword SEreorderqueue(SPEECHQUEUE *pSQueue)
{
  int v1; // ebp
  float *v2; // edx
  int v3; // ebx
  int v4; // esi
  int v5; // edx
  int *v6; // ecx
  int v7; // edi
  unsigned __int16 v8; // bx
  unsigned __int16 v9; // ax
  int v11; // [esp+10h] [ebp-8h]
  int v12; // [esp+14h] [ebp-4h]

  v1 = a1;
  v11 = 0;
  v12 = 0;
  v2 = (float *)(a1 + 44);
  v3 = 10;
  do
  {
    if ( *v2 >= (double)ZERO
      && *v2 < (double)universe.totaltimeelapsed
      && (*(_DWORD *)(v2 - 5) & 0xF0000000) < 0x8000000 )
    {
      memset(v2 - 11, 0, 0x30u);
      *(v2 - 11) = NAN;
      *v2 = -1.0;
      *(v2 - 5) = NAN;
    }
    v2 += 12;
    --v3;
  }
  while ( v3 );
  v4 = 0;
  v5 = 0;
  v6 = (int *)(a1 + 36);
  v7 = -36 - a1;
  do
  {
    if ( *(v6 - 9) < 0 )
      goto LABEL_21;
    v12 = 1;
    if ( *(int *)(v5 + v1) < 0 )
    {
LABEL_20:
      v11 = v4;
      v5 = (int)v6 + v7;
      goto LABEL_21;
    }
    if ( (*(v6 - 3) & 0xF0000000) > (*(_DWORD *)(v5 + v1 + 24) & 0xF0000000) )
    {
      v1 = a1;
      v11 = v4;
      v5 = (int)v6 + v7;
      goto LABEL_21;
    }
    v1 = a1;
    v8 = *(_WORD *)(v5 + a1 + 36);
    v9 = *(_WORD *)v6;
    if ( ((*(_DWORD *)(v5 + a1 + 24) ^ *(v6 - 3)) & 0xF0000000) != 0 )
    {
      if ( v9 > v8 || v9 == v8 && *((float *)v6 + 1) < (double)*(float *)(v5 + a1 + 40) )
        goto LABEL_20;
    }
    else if ( v9 > v8 || v9 == v8 && *((float *)v6 + 1) < (double)*(float *)(v5 + a1 + 40) )
    {
      goto LABEL_20;
    }
LABEL_21:
    ++v4;
    v6 += 12;
  }
  while ( v4 < 10 );
  if ( v12 )
    *(_DWORD *)(v1 + 532) = v11;
  else
    *(_DWORD *)(v1 + 532) = -1;
  return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x521EB0
void speechPlayQueue(void)
{
  int *actor;
  int flags;
  int queueIdx;
  int *queueEntry;
  int currentSlot;
  int *slotBase;
  int actorIdx;
  int *streamHandle;

  actorIdx = 0;
  streamHandle = speechStreamHandles;
  actor = (int *)&speechActors_currentSlotInfo;
  do
  {
    if ( *(actor - 1) >= 4 && soundstreamover(*streamHandle) )
    {
      flags = *(actor - 7);
      if ( (flags & 0x10000000) != 0 )
        --speechPriorityCount;
      queueIdx = *actor;
      if ( *actor >= 0 )
      {
        queueEntry = &actor[12 * queueIdx - 133];
        if ( flags == actor[12 * queueIdx - 127] && *(actor - 6) == queueEntry[7] )
        {
          memset(queueEntry, 0, 0x30u);
          actor[12 * *actor - 133] = -1;
          actor[12 * *actor - 122] = -1082130432;
          actor[12 * *actor - 127] = -1;
        }
      }
      *(actor - 7) = -1;
      *(actor - 1) = 1;
      actor[2] = 0;
    }
    if ( *(actor - 1) == 1 )
    {
      currentSlot = *actor;
      if ( *actor >= 0
        && !actor[1]
        && (universe.totaltimeelapsed - *((float *)actor + 2) >= speechMinDelay
         || !gameIsRunning
         || !enableSpeech
         || nisIsRunning) )
      {
        slotBase = actor - 133;
        *((float *)actor + 2) = universe.totaltimeelapsed + *(float *)&actor[12 * currentSlot - 122];
        qmemcpy(actor - 13, &actor[12 * currentSlot - 133], 0x30u);
        memset(&actor[12 * currentSlot - 133], 0, 0x30u);
        slotBase[12 * *actor] = -1;
        slotBase[12 * *actor + 11] = -1082130432;
        slotBase[12 * *actor + 6] = -1;
        speechPlaySentence(actor - 133, actorIdx);
        if ( (*(actor - 7) & 0x10000000) != 0 )
          ++speechPriorityCount;
        SEreorderqueue((int)(actor - 133));
        *(actor - 1) = 4;
      }
    }
    actor += 142;
    ++actorIdx;
    ++streamHandle;
  }
  while ( (int)actor < (int)&speechActorsEnd );
  if ( Eegah && universe.totaltimeelapsed > eegahTimeThreshold && !eegahPlayed )
  {
    eegahPlayed = 1;
    speechEventQueue(0, 33559249, -1, -1, 0, -1, -1, 5.0, -1);
  }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5220E0
void speechPlaySentence(int *actorQueue, sdword actorIdx)
{
  int actorNum;
  int flags;
  short numActors;
  signed int numPatches;
  double vol;
  int patchHandle;
  int streamHandle;
  int patchIdx;
  int *patchList;
  int duration;
  int volScaled;
  int numQueued;
  short volShort;
  int dummy;
  float volume;
  int patchCount;

  actorNum = actorQueue[123];
  flags = actorQueue[126];
  patchList = 0;
  if ( flags == -1 )
    return;
  numActors = SentenceLUT->compbitrate;
  numPatches = SEselectsentence(numActors, actorNum, flags & 0x7FF, actorQueue[127], actorQueue[128], &patchList, &dummy);
  volume = speechMasterVol * 255.0f;
  if ( speechPriorityCount <= 0 || (flags & 0x10000000) != 0 )
  {
    if ( (flags & 0x800) == 0 )
      goto LABEL_8;
    vol = volume * speechNISVolScale;
  }
  else
  {
    vol = speechBattleVolScale * volume;
  }
  volume = vol;
LABEL_8:
  if ( musicPlaying && (flags & 0x3000000) != 0 )
    volume = speechBattleVolScale * volume;
  volScaled = (int)(speechVolume * volume + 0.5);
  if ( numPatches > 0 && volume >= (double)speechMinVolume )
  {
    numQueued = numPatches - 1;
    if ( numPatches - 1 > 0 )
    {
      volShort = (short)(int)volume;
      patchCount = numQueued;
      do
      {
        patchHandle = *patchList;
        patchIdx = actorQueue[123];
        streamHandle = speechStreamHandles[actorIdx];
        ++patchList;
        soundstreamqueuePatch(streamHandle, speechBank, patchHandle, 4, volShort, 0, 1, numActors, 0, (int)&speechEQ, 0, 0, volScaled, 0, 0, patchIdx, flags, 1);
        --patchCount;
      }
      while ( patchCount );
    }
    volume = volume + 0.5;
    patchIdx = *patchList++;
    soundstreamqueuePatch(
      speechStreamHandles[actorIdx],
      speechBank,
      patchIdx,
      4,
      (short)(int)volume,
      0, 1, numActors, 0, (int)&speechEQ, 0, 0,
      volScaled, 0, 0,
      actorQueue[123], flags, 0);
    if ( actorQueue[125] )
    {
      currentSpeechEvent = actorQueue[125];
    }
    else if ( (0x800000 & flags) != 0 )
    {
      currentSpeechEvent = -1;
      currentSpeechActor = actorQueue[127];
    }
  }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5223C0
udword SEselectsentence(sdword numActors, sdword actorNum, sdword variation, sdword sentence, sdword *outPatchList, sdword *outDuration)
{
  int lutIndex;
  unsigned short numVariations;
  unsigned short *pNumSentences;
  unsigned int result;
  udword sentenceIdx;
  unsigned short baseOffset;
  signed int numSentences;
  int entryIdx;
  unsigned short bankStride;
  int bankIndex;
  int sentenceCount;
  int bankStrideFull;
  int bankDataOffset;
  unsigned char *bankEntry;
  unsigned int roundRobin;
  char selectedVar;
  unsigned int shifted;
  _DWORD *patchPtr;
  _DWORD *patchIter;
  unsigned int validCount;
  unsigned short savedNumVar;
  int savedBase;

  lutIndex = SentenceLUT->numcolumns * (actorNum + numActors * SentenceLUT->numevents);
  numVariations = *(short *)((char *)SentenceLUT + 2 * lutIndex + 18);
  pNumSentences = (unsigned short *)((char *)SentenceLUT + 2 * lutIndex + 20);
  savedNumVar = numVariations;
  if ( !numVariations )
    return -1;
  sentenceIdx = variation;
  baseOffset = *(short *)((char *)SentenceLUT + 2 * lutIndex + 22);
  if ( variation >= 0 )
  {
    numSentences = *pNumSentences;
    if ( variation >= numSentences )
      return -1;
  }
  else
  {
    numSentences = *pNumSentences;
    sentenceIdx = ranRandomFn(12) %% numSentences;
  }
  if ( sentence == -1 || sentence >= numVariations )
  {
    if ( numVariations == 1 )
    {
      entryIdx = sentenceIdx + baseOffset;
    }
    else
    {
      bankStride = *(short *)(SentenceBank + 8);
      savedBase = baseOffset;
      bankIndex = bankStride * (sentenceIdx + baseOffset);
      sentenceCount = *(unsigned char *)(SentenceBank + 4 * bankIndex + 22);
      bankStrideFull = numSentences * bankStride;
      bankDataOffset = *(unsigned char *)(SentenceBank + 4 * bankIndex + 23);
      bankEntry = (unsigned char *)(SentenceBank + 4 * bankIndex + 12);
      roundRobin = bankEntry[6] + ((bankEntry[7] + ((sentenceCount + (bankDataOffset << 8)) << 8)) << 8);
      if ( !roundRobin )
        roundRobin = SEinitRoundRobin(savedNumVar, (int)bankEntry, bankStrideFull);
      selectedVar = roundRobin;
      shifted = roundRobin >> 4;
      bankEntry[11] = (shifted >> 24) & 0xFF;
      bankEntry[7] = (shifted >> 8) & 0xFF;
      bankEntry[10] = (shifted >> 16) & 0xFF;
      bankEntry[6] = shifted & 0xFF;
      entryIdx = sentenceIdx + savedBase + ((selectedVar & 0xF) - 1) * numSentences;
    }
  }
  else
  {
    entryIdx = sentence * numSentences + sentenceIdx + baseOffset;
  }
  patchPtr = (_DWORD *)(SentenceBank + 4 * entryIdx * *(unsigned short *)(SentenceBank + 8) + 12);
  *outDuration = *patchPtr;
  result = (unsigned char)patchPtr[2];
  patchIter = patchPtr + 3;
  *outPatchList = patchIter;
  if ( result )
  {
    validCount = 0;
    while ( *patchIter != -1 )
    {
      ++validCount;
      ++patchIter;
      if ( validCount >= result )
        return result;
    }
    return 0;
  }
  return result;
}
#endif

#if 0  // Cat2: Called from binary SEselectsentence; live SEselectsentence reimplemented differently; no direct live equivalent (0x522300)
int __cdecl SErandomizeSentencePriorityOrder(int a1, int a2, int a3)
{
  char v3; // dl
  int v4; // eax
  _BYTE *v5; // esi
  int *v6; // edi
  char v7; // cl
  int v8; // ebp
  udword v9; // edx
  unsigned __int8 v10; // bl
  char *v11; // esi
  int v12; // ebp
  char *v13; // ecx
  char v14; // al
  int result; // eax
  char *v16; // ecx
  int v17; // edx
  char v18[16]; // [esp+10h] [ebp-50h] BYREF
  int v19; // [esp+20h] [ebp-40h] BYREF
  char v20[60]; // [esp+24h] [ebp-3Ch] BYREF
  int v21; // [esp+64h] [ebp+4h]
  int v22; // [esp+68h] [ebp+8h]
  unsigned int v23; // [esp+6Ch] [ebp+Ch]

  v3 = 0;
  v4 = 0;
  if ( a1 > 0 )
  {
    v5 = (_BYTE *)(a2 + 4);
    v6 = &v19;
    do
    {
      v7 = *v5 - v3;
      *v6 = v4;
      v3 += v7;
      v5 += 4 * a3;
      v18[v4++] = v7;
      ++v6;
    }
    while ( v4 < a1 );
  }
  v8 = a1 - 1;
  v22 = a1 - 1;
  if ( a1 - 1 > 0 )
  {
    v23 = a1 - 1;
    v21 = a1 - 1;
    do
    {
      v9 = ranRandomFn(12) % v23;
      v10 = v18[v9];
      v11 = &v18[v9 + 1];
      if ( v10 <= (unsigned __int8)v18[v9 + 1] )
      {
        v12 = *(_DWORD *)&v20[4 * v9 - 4];
        v13 = &v20[4 * v9];
        *(_DWORD *)&v20[4 * v9 - 4] = *(_DWORD *)v13;
        v14 = *v11;
        *(_DWORD *)v13 = v12;
        v8 = v22;
        v18[v9] = v14;
        *v11 = v10;
      }
      --v21;
    }
    while ( v21 );
  }
  result = 0;
  if ( v8 >= 0 )
  {
    v16 = &v20[4 * v8 - 4];
    v17 = v8 + 1;
    do
    {
      result = (*(_DWORD *)v16 + 1) | (16 * result);
      v16 -= 4;
      --v17;
    }
    while ( v17 );
  }
  return result;
}
#endif

// Decompiled from cataclysm.exe @ 0x522B00
void soundEventStopActorStreams(Ship *ship)
{
    sdword i, j;

    if (ship == currentSpeechEvent)
        currentSpeechEvent = NULL;

    for (i = 0; i < SE_NUM_ACTORS; i++)
    {
        speechqueue[i].locked = TRUE;
        for (j = 0; j < SE_MAX_QUEUE; j++)
        {
            if (speechqueue[i].queue[j].pShip == ship)
            {
                memset(&speechqueue[i].queue[j], 0, sizeof(QUEUEEVENT));
                speechqueue[i].queue[j].handle = SOUND_NOTINITED;
                speechqueue[i].queue[j].timeout = -1.0f;
                speechqueue[i].queue[j].pShip = NULL;
                speechqueue[i].queue[j].event = SOUND_NOTINITED;
            }
        }
        if (speechqueue[i].current.pShip == ship)
        {
            soundstreamvolume(speechqueue[i].current.handle, -1, -1.0f);
            speechqueue[i].current.pShip = NULL;
        }
        speechqueue[i].locked = FALSE;
    }
}

#if 0  // Cat2: Binary soundEventStopActorStreams iterates raw actor array; live code uses typed speechqueue[] with same logic (0x522B00)
int __cdecl soundEventStopActorStreams(int a1)
{
  int v1; // edi
  _DWORD *v2; // esi
  _DWORD *v3; // edx
  int v4; // ebx

  v1 = a1;
  if ( a1 == dword_A6AE80 )
    dword_A6AE80 = 0;
  v2 = &unk_C09A34;
  do
  {
    v2[9] = 1;
    v3 = v2 - 125;
    v4 = 10;
    do
    {
      if ( v3[5] == v1 )
      {
        memset(v3, 0, 0x30u);
        v1 = a1;
        *v3 = -1;
        v3[11] = -1082130432;
        v3[5] = 0;
        v3[6] = -1;
      }
      v3 += 12;
      --v4;
    }
    while ( v4 );
    if ( *v2 == v1 )
    {
      soundstreamvolume(*(v2 - 5), -1, -1.0);
      *v2 = 0;
    }
    v2[9] = 0;
    v2 += 142;
  }
  while ( (int)v2 < (int)&unk_C0A0DC );
  return 0;
}
#endif

// Decompiled from cataclysm.exe @ 0x522BA0
void soundEventResetStreamGlobals(void)
{
    speechEnemyPresencePendingUnderAttackFromAlly = 0;
    speechEnemyPresencePendingFleetStatus76 = 0;
    speechEnemyPresencePendingResourceInjectionReceived = 0;
    speechEnemyPresenceLastWarningTime = 0.0f;
}

#if 0  // Cat2: Binary soundEventResetStreamGlobals; live code matches exactly (0x522BA0)
int soundEventResetStreamGlobals()
{
  int result; // eax

  result = 0;
  speechEnemyPresencePendingUnderAttackFromAlly = 0;
  speechEnemyPresencePendingFleetStatus76 = 0;
  speechEnemyPresencePendingResourceInjectionReceived = 0;
  speechEnemyPresenceLastWarningTime = 0;
  return result;
}
#endif

sdword speechEventReportProtectTargetSelection(void)
{
    sdword resourceCount = 0;
    sdword friendlyCount = 0;
    sdword enemyCount = 0;
    sdword index;
    Ship *speaker = selSelected.ShipPtr[0];

    if (!keyIsHit(SHIFTKEY))
    {
        speechEventQueue(speaker, 67110917, speaker->shiptype, -1, -1, -1, -1, 0.0f, -1);
    }
    else if (protectcom.numShips > 0)
    {
        for (index = 0; index < protectcom.numShips; index++)
        {
            SpaceObjRotImpTarg *target = (SpaceObjRotImpTarg *)protectcom.ShipPtr[index];

            if (target->objtype == OBJ_ShipType)
            {
                if (((Ship *)target)->playerowner == universe.curPlayerPtr)
                {
                    friendlyCount++;
                }
                else
                {
                    enemyCount++;
                }
            }
            else if (target->objtype >= OBJ_AsteroidType && target->objtype <= OBJ_DustType)
            {
                resourceCount++;
            }
        }

        if (friendlyCount > 0)
        {
            speechEventQueue(speaker, 67110919, speaker->shiptype, -1, -1, -1, -1, 0.0f, -1);
        }
        else if (enemyCount <= 0 && resourceCount > 0)
        {
            speechEventQueue(speaker, 67110920, speaker->shiptype, -1, -1, -1, -1, 0.0f, -1);
        }
    }

    /* Retail also stores 1 to the runtime flag at 0xA6AE64 here. We are leaving that dead
       speech-local side effect out until a real consumer is identified. */
    return 1;
}
