/*=============================================================================
    Name    : SoundEvent.c
    Purpose : Sound Event processor

    Created 7/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "SoundEventPrivate.h"
#include "SoundEvent.h"
#include "gun.h"
#include "matrix.h"
#include "select.h"
#include "GenericInterceptor.h"
#include "Slipgate.h"
#include "render.h"
#include "singleplayer.h"
#include "nis.h"
#include "shockwave.h"
#include "SalCapCorvette.h"
#include "Alliance.h"

#define DIRECTSOUND     1
// Retail soundEventUpdate inlines ShipEvents LUT row 1 for hsState==3 and row 20 for hsState==4.
#define CATA_RETAIL_HYPERDRIVE_LUT_EVENT       1
#define CATA_RETAIL_HYPERDRIVE_OFF_LUT_EVENT  20

#define SFX_MAX_STRIKEENGINES   5
#define SFX_MAX_CAPENGINES      3

#define CATA_WORKER_RESOURCE_ASTEROID_STATE 4
#define CATA_WORKER_RESOURCE_DUST_STATE     5
#define CATA_WORKER_RESOURCE_ASTEROID_EVENT 2
#define CATA_WORKER_RESOURCE_DUST_EVENT     3
// Retail soundEventUpdate computes 56 * (numvariations + 1) before indexing ShipEvents.
#define CATA_BEAST_CONVERSION_LOOP_EVENT 56
#define CATA_BEAST_CONVERSION_MAX_ACTIVE_LOOPS 3
#define CATA_BEAST_CONVERSION_PILOT_LOOP_VOLUME 120
#define CATA_SLIPGATE_SOUND_CLASS          64
#define CATA_SLIPGATE_SOUND_CACHE_SIZE     16

sword SEgetHeadingAngle(float *obj);
void SEinitShipClassLUT(void);
void soundEventShipRemoveHandles(Ship *ship);


typedef struct
{
    Ship *ship;
    ubyte lastState;
} SlipgateSoundState;

static SlipgateSoundState seSlipgateSoundState[CATA_SLIPGATE_SOUND_CACHE_SIZE];

real32 dword_A6AE4C = 0.0f; // sound-system camera-proximity metric; written by soundEventUpdate and musicEventUpdateVolume

static bool32 SEisSlipgateShipType(sdword shiptype)
{
    return shiptype >= dSlipgateSmall &&
           shiptype <= dSlipgateLarge;
}

static SlipgateSoundState *SEslipgateSoundStateSlot(Ship *ship)
{
    sdword i;
    SlipgateSoundState *empty = NULL;

    for (i = 0; i < CATA_SLIPGATE_SOUND_CACHE_SIZE; i++)
    {
        if (seSlipgateSoundState[i].ship == ship)
        {
            return &seSlipgateSoundState[i];
        }
        if (empty == NULL && seSlipgateSoundState[i].ship == NULL)
        {
            empty = &seSlipgateSoundState[i];
        }
    }

    if (empty == NULL)
    {
        empty = &seSlipgateSoundState[0];
    }
    empty->ship = ship;
    empty->lastState = 0;
    return empty;
}

static ubyte SEslipgateLastState(Ship *ship)
{
    if (ship == NULL)
    {
        return 0;
    }

    return SEslipgateSoundStateSlot(ship)->lastState;
}

static void SEslipgateSetLastState(Ship *ship, ubyte state)
{
    if (ship == NULL)
    {
        return;
    }

    SEslipgateSoundStateSlot(ship)->lastState = state;
}

static void SEslipgateClearState(Ship *ship)
{
    sdword i;

    if (ship == NULL)
    {
        return;
    }

    for (i = 0; i < CATA_SLIPGATE_SOUND_CACHE_SIZE; i++)
    {
        if (seSlipgateSoundState[i].ship == ship)
        {
            seSlipgateSoundState[i].ship = NULL;
            seSlipgateSoundState[i].lastState = 0;
            return;
        }
    }
}

static void SEslipgatePlayCommonEvent(Ship *ship, sdword event, sword pan, sword vol, real32 *tempEQ)
{
    splayFPRVL(ShipBank,
               ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ship->shiptype, event)],
               tempEQ,
               (real32)SOUND_DEFAULT,
               pan,
               SOUND_PRIORITY_NORMAL,
               vol,
               TRUE,
               TRUE,
               FALSE);
}

static bool32 SEslipgateHandleShip(Ship *ship, real32 dist, sword pan, sword *vol, real32 *tempEQ)
{
    ubyte slipgateState;
    ubyte lastSlipgateState;
    sdword transitionEvent = SOUND_ERR;
    sword slipgateVol;

    if (!SEisSlipgateShipType(ship->shiptype) ||
        !SEinrange(CATA_SLIPGATE_SOUND_CLASS, dist))
    {
        return FALSE;
    }

    slipgateState = slipgateGetState(ship);
    lastSlipgateState = SEslipgateLastState(ship);
    if (lastSlipgateState != slipgateState)
    {
        if (slipgateState == 1)
        {
            transitionEvent = ShipCmn_AnimOpening;
        }
        else if (slipgateState == 3)
        {
            transitionEvent = ShipCmn_AnimClosing;
        }

        if (transitionEvent != SOUND_ERR)
        {
            slipgateVol = SEequalize(CATA_SLIPGATE_SOUND_CLASS, dist, tempEQ);
            SEslipgatePlayCommonEvent(ship, transitionEvent, pan, slipgateVol, tempEQ);
        }
        SEslipgateSetLastState(ship, slipgateState);
    }

    if (slipgateState == 0)
    {
        return FALSE;
    }

    *vol = SEequalize(CATA_SLIPGATE_SOUND_CLASS, dist, tempEQ);
    if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
        soundover(ship->soundevent.specialHandle))
    {
        ship->soundevent.specialHandle = splayFPRVL(ShipBank,
                                                    ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ship->shiptype, ShipCmn_RandomAmbient)],
                                                    tempEQ,
                                                    (real32)SOUND_DEFAULT,
                                                    pan,
                                                    SOUND_PRIORITY_NORMAL,
                                                    *vol,
                                                    TRUE,
                                                    TRUE,
                                                    FALSE);
    }

    return TRUE;
}

/*=============================================================================
    Tweaks
=============================================================================*/

scriptEntry SoundeventTweaks[] =
{
    makeEntry(SFX_VOL_FACTOR,scriptSetReal32CB),
    makeEntry(SFX_MAX_ENGINES,scriptSetSdwordCB),
    makeEntry(SFX_MIN_CAPSHIPS,scriptSetSdwordCB),
    makeEntry(SFX_AMBIENT_VOLUME,scriptSetSdwordCB),
    makeEntry(SFX_NIS_MAX_ENGINES,scriptSetSdwordCB),
    makeEntry(SFX_NIS_MIN_CAPSHIPS,scriptSetSdwordCB),
    makeEntry(SFX_MAX_ENGINE_RANGE,scriptSetReal32CB),
    makeEntry(SFX_CAPSHIPS_ALWAYS_ON,scriptSetBool),
    makeEntry(SFX_MAX_AMBIENT,scriptSetSdwordCB),
    makeEntry(SFX_FLOAT_VELOCITY,scriptSetReal32CB),
    makeEntry(SFX_NIS_FLOAT_VELOCITY,scriptSetReal32CB),
	makeEntry(SFX_MIN_PERCEPTABLE_VOL, scriptSetReal32CB),
	makeEntry(SFX_HYPERSPACE_VOLUME,scriptSetSwordCB),

    makeEntry(SFX_DAMAGERATIO_LIGHT,scriptSetReal32CB),
    makeEntry(SFX_DAMAGERATIO_MEDIUM,scriptSetReal32CB),
    makeEntry(SFX_DAMAGERATIO_HEAVY,scriptSetReal32CB),
    makeEntry(SFX_DAMAGERATIO_ENABLE,scriptSetBool),

    makeEntry(SFX_CARDIOD_FACTOR,scriptSetReal32CB),
    makeEntry(SFX_CARDIOD_MIN,scriptSetReal32CB),

    makeEntry(FIGHTER_VELOCITY_LOWPITCH,scriptSetReal32CB),
    makeEntry(FIGHTER_VELOCITY_HIGHPITCH,scriptSetReal32CB),
    makeEntry(FIGHTER_VELOCITY_SCALE,scriptSetReal32CB),

    makeEntry(CORVETTE_VELOCITY_LOWPITCH,scriptSetReal32CB),
    makeEntry(CORVETTE_VELOCITY_HIGHPITCH,scriptSetReal32CB),
    makeEntry(CORVETTE_VELOCITY_SCALE,scriptSetReal32CB),

    makeEntry(FIGHTER_DOPPLER_SCALE,scriptSetReal32CB),
    makeEntry(FIGHTER_DOPPLER_LOW,scriptSetSdwordCB),
    makeEntry(FIGHTER_DOPPLER_HIGH,scriptSetSwordCB),
    makeEntry(FIGHTER_DOPPLER_USEVELOCITY,scriptSetBool),

    makeEntry(CORVETTE_DOPPLER_SCALE,scriptSetReal32CB),
    makeEntry(CORVETTE_DOPPLER_LOW,scriptSetSwordCB),
    makeEntry(CORVETTE_DOPPLER_HIGH,scriptSetSwordCB),
    makeEntry(CORVETTE_DOPPLER_USEVELOCITY,scriptSetBool),

    makeEntry(SPEECH_VOL_FACTOR,scriptSetReal32CB),
    makeEntry(SPEECH_VOL_LOW,scriptSetSwordCB),
    makeEntry(SPEECH_VOL_MAX,scriptSetSwordCB),
    makeEntry(SPEECH_NOISE_FACTOR,scriptSetReal32CB),
    makeEntry(SPEECH_NOISE_LOW,scriptSetReal32CB),
    makeEntry(SPEECH_NOISE_HIGH,scriptSetReal32CB),
    makeEntry(SPEECH_FILTER_LOW,scriptSetUdwordCB),
    makeEntry(SPEECH_FILTER_HIGH,scriptSetUdwordCB),
    makeEntry(SPEECH_BREAK_THRESHOLD,scriptSetReal32CB),
    makeEntry(SPEECH_BREAK_RATE_FACTOR,scriptSetReal32CB),
    makeEntry(SPEECH_BREAK_RATE_LOW,scriptSetUdwordCB),
    makeEntry(SPEECH_BREAK_RATE_HIGH,scriptSetUdwordCB),
    makeEntry(SPEECH_BREAK_LENGTH_FACTOR,scriptSetReal32CB),
    makeEntry(SPEECH_BREAK_LENGTH_LOW,scriptSetUdwordCB),
    makeEntry(SPEECH_BREAK_LENGTH_HIGH,scriptSetUdwordCB),
    makeEntry(SPEECH_CAPSHIP_CHATTER_RANGE,scriptSetReal32CB),
    makeEntry(SPEECH_CAPSHIP_CHATTER_TIME,scriptSetSdwordCB),
    makeEntry(SPEECH_COMBAT_CHATTER_RANGE,scriptSetReal32CB),
    makeEntry(SPEECH_COMBAT_CHATTER_TIME,scriptSetSdwordCB),
    makeEntry(SPEECH_MOSHIP_WARNING_TIME,scriptSetReal32CB),
    makeEntry(SPEECH_WARNING_TIME,scriptSetReal32CB),
    makeEntry(SPEECH_DISOBEY_FORCEDATTACK,scriptSetReal32CB),
    makeEntry(SPEECH_MIN_PERCEPTABLE_VOL,scriptSetReal32CB),
    makeEntry(SPEECH_AMBIENT_LEVEL,scriptSetReal32CB),
    makeEntry(SPEECH_AMBIENT_ENABLE,scriptSetBool),
	makeEntry(SPEECH_STIKEDAMAGE_MULT,scriptSetReal32CB),
	makeEntry(SPEECH_CAPDAMAGE_MULT,scriptSetReal32CB),
	makeEntry(SPEECH_SINGLEPLAYER_RATIO, scriptSetReal32CB),
	makeEntry(SPEECH_STATUS_RATIO, scriptSetReal32CB),
	makeEntry(SPEECH_CHATTER_RATIO, scriptSetReal32CB),
	makeEntry(MUSIC_DISTANCE_SILENT,scriptSetReal32CB),
    makeEntry(MUSIC_DISTANCE_MAX,scriptSetReal32CB),
    makeEntry(MUSIC_MAXGAME_VOL,scriptSetReal32CB),
    makeEntry(MUSIC_MINACTIVE_VOL,scriptSetReal32CB),
    makeEntry(MUSIC_MININACTIVE_VOL,scriptSetReal32CB),
    makeEntry(MUSIC_SENSORS_VOL,scriptSetReal32CB),
    makeEntry(MUSIC_MANAGERS_VOL,scriptSetReal32CB),
    makeEntry(MUSIC_TUTORIAL_VOL,scriptSetReal32CB),
    makeEntry(MUSIC_FADE_TIME,scriptSetReal32CB),
	makeEntry(MUSIC_MAXBATTLE_VOL,scriptSetReal32CB),
	makeEntry(MUSIC_MINBATTLE_VOL,scriptSetReal32CB),

	makeEntry(RANDOM_AMBIENCE_MINFREQ,scriptSetReal32CB),
    makeEntry(RANDOM_AMBIENCE_ADDRANDOM,scriptSetSdwordCB),
    endEntry
};


/*=============================================================================
    Private Data
=============================================================================*/

sword   soundeventinited = FALSE;
ubyte   *bankLoadAddress;
sdword  ambienthandle = SOUND_DEFAULT;
bool32    soundpaused = FALSE;

/* new banks */
ubyte   *GunBank;
ubyte   *ShipBank;
ubyte   *SpecialEffectBank;
ubyte   *UIBank;

/* SFX lookup tables */
SFXLUT  *GunEventsLUT;
SFXLUT  *GunSpecEventsLUT;
SFXLUT  *ShipCmnEventsLUT;
SFXLUT  *ShipEventsLUT;
SFXLUT  *DerelictEventsLUT;
SFXLUT  *SpecEffectEventsLUT;
SFXLUT  *SpecExpEventsLUT;
SFXLUT  *SpecHitEventsLUT;
SFXLUT  *UIEventsLUT;

TABLELUT        *VolumeLUT;
real32			*VolumeFloatLUT;
TABLELUT        *RangeLUT;
real32			*RangeFloatLUT;
FREQUENCYLUT    *FrequencyLUT;
real32		    *FreqLUT;

real32  DefaultEQ[SOUND_EQ_SIZE];
real32  MasterEQ[SOUND_EQ_SIZE] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

extern HWND ghMainWindow;

real32 cardiod[CARDIOD_POINTS];

real32 volSpeech = 1.0f;
real32 volMusic = 1.0f;
real32 volSFX = 1.0f;
real32 volNIS = 1.0f;
bool32 bVolMusicNoFade = FALSE;
static bool32 seBackgroundMuted = FALSE;

// Cat2 Easter egg flags (set by SEloadbank, read by speechPlayQueue/battle)
bool32 Eegah = FALSE;
bool32 PrinceOfSpace = FALSE;
static sdword seCataclysmEasterEggState = 0;

// Cat2 LUT used for ship-class to sound-event mapping
static sdword shipClassLUT[360];

// Cataclysm-only sound event tables retained for parity with the retail sound bank layout.
static SFXLUT *seShockwaveAsteroidEventsLUT;
static SFXLUT *seCataclysmSfxEventsLUT1;
static SFXLUT *seCataclysmSfxEventsLUT2;
static SFXLUT *seCataclysmSfxEventsLUT3;
static SFXLUT *seCataclysmSfxEventsLUT4;
static SFXLUT *seCataclysmSfxEventsLUT5;

// Legacy Cataclysm speech/script allocations still owned by the sound event shutdown path.
static void *seCataclysmSpeechScriptAlloc0;
static void *seCataclysmSpeechScriptAlloc1;
static void *seCataclysmSpeechScriptAlloc2;
static void *seCataclysmSpeechScriptAlloc3;
static void *seCataclysmSpeechScriptAlloc4;
static void *seCataclysmSpeechScriptAlloc5;
static void *seCataclysmSpeechScriptAlloc6;
static void *seCataclysmSpeechScriptAlloc7;

bool32    bActorOn[4] = {TRUE, TRUE, TRUE, TRUE};
bool32    bCommandsOn = TRUE;
bool32    bStatusOn = TRUE;
bool32    bChatterOn = TRUE;

sdword  actorFlagsEnabled = (ACTOR_PILOT_FLAG | ACTOR_FLEETCOMMAND_FLAG |
							 ACTOR_FLEETINTEL_FLAG | ACTOR_TRADERS_FLAG |
							 ACTOR_PIRATES2_FLAG | ACTOR_ALLSHIPSENEMY_FLAG |
							 ACTOR_AMBASSADOR_FLAG | ACTOR_NARRATOR_FLAG |
                             ACTOR_DEFECTOR_FLAG | ACTOR_GENERAL_FLAG |
                             ACTOR_EMPEROR_FLAG | ACTOR_KHARSELIM_FLAG);

// used for calculating the music volume
real32 nearestShipDistance = 0.0f;
real32 musicVolFactor = 0.0f;
real32 battleMusicVolFactor = 0.0f;
bool32 nearestShipMoving = FALSE;
real32 musicInactiveFactor = 0.0f;

extern sdword  streamhandle[];
extern sdword  speechfilehandle;
extern SENTENCELUT *SentenceLUT;

sdword SEselectsentence(sdword actor, sdword event, sdword variable, sdword setvariation, sdword **pOffsets, udword *pDuration);
void SEinitHandles(void);
void soundEventStopActorStreams(Ship *ship);  // defined in speechevent.c
int soundEventResetStreamGlobals(void);       // defined in speechevent.c
void *GetShipInfectionData(Ship *ship);       // defined in Ship.c
extern void battleChatterResetTimers(void);
extern void singlePlayerSetWideScreenSpeedScale(bool32 enabled);
extern char playerNames[MAX_MULTIPLAYER_PLAYERS][MAX_PERSONAL_NAME_LEN];
extern ShipRace whichRaceSelected;

static real32 SEapplyBackgroundMute(real32 level)
{
    if (seBackgroundMuted)
    {
        return 0.0f;
    }
    return level;
}

void soundEventSetBackgroundMute(bool32 muted)
{
    seBackgroundMuted = muted;
    soundSetBackgroundMute(muted);
    if (muted)
    {
        volSpeech = 0.0f;
        volMusic = 0.0f;
        volSFX = 0.0f;
        bVolMusicNoFade = TRUE;
    }
}

/*=============================================================================
    Functions
=============================================================================*/


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
sdword GetPatch(SFXLUT *lut, sdword object, sdword event)
{
    sdword numvars;
    sdword index;

    event = (ubyte)event;

    if (lut == NULL)
    {
        return (SOUND_ERR);
    }

    if (object > lut->numobjects)
    {
        return (SOUND_ERR);
    }

    index = (object * lut->numevents * (lut->numvariations + 1)) + ((ubyte)event * (lut->numvariations + 1));
    numvars = lut->lookup[index];

    if (numvars <= 1)
    {
        return (index + 1);
    }

    return (index + 1 + (ranRandom(RAN_SoundGameThread) % numvars));
}

static sdword SEambientEventFromDamage(real32 damageratio)
{
    if (damageratio > SFX_DAMAGERATIO_LIGHT)
    {
        return ShipCmn_Ambient;
    }

    if (damageratio > SFX_DAMAGERATIO_MEDIUM)
    {
        return ShipCmn_RandomAmbient;
    }

    if (damageratio > SFX_DAMAGERATIO_HEAVY)
    {
        return ShipCmn_AnimClosing;
    }

    return ShipCmn_AnimDamagedOpen;
}

static bool32 SEshipHasExternalBuildAmbientState(Ship *ship)
{
    return ((ship->specialFlags4 & SPECIAL4_ExternalBuildMember) != 0);
}

static bool32 SEshipHasReplacementAmbientState(Ship *ship)
{
    return (ship->replacementStateActive != 0);
}

static sdword SEambientEventForShip(Ship *ship, real32 damageratio)
{
    if (SEshipHasExternalBuildAmbientState(ship))
    {
        return Ship_CloakOff;
    }

    if (SEshipHasReplacementAmbientState(ship))
    {
        return Ship_ResearchLockOn;
    }

    return SEambientEventFromDamage(damageratio);
}

static sdword SErandomAmbientEventForShip(Ship *ship, real32 damageratio)
{
    if (damageratio > SFX_DAMAGERATIO_LIGHT)
    {
        return SOUND_ERR;
    }

    if (damageratio > SFX_DAMAGERATIO_MEDIUM)
    {
        return ShipCmn_LightDamage;
    }

    if (SEshipHasExternalBuildAmbientState(ship))
    {
        return (Ship_Flag + 32);
    }

    return ShipCmn_AnimDamagedClose;
}

static real32 SErandomAmbientCooldown(Ship *ship)
{
    if (SEshipHasExternalBuildAmbientState(ship))
    {
        return (RANDOM_AMBIENCE_MINFREQ * 0.25f)
             + ((real32)(ranRandom(RAN_SoundGameThread) % RANDOM_AMBIENCE_ADDRANDOM) * 0.0025f);
    }

    return RANDOM_AMBIENCE_MINFREQ
         + ((real32)(ranRandom(RAN_SoundGameThread) % RANDOM_AMBIENCE_ADDRANDOM) / 100.0f);
}

static SFXLUT *SElutForEvent(sdword event)
{
    if ((event & SFX_Flag_Mask) == Ship_Flag)
    {
        return ShipEventsLUT;
    }

    return ShipCmnEventsLUT;
}

static sdword SEobjectForEvent(Ship *ship, sdword event)
{
    if ((event & SFX_Flag_Mask) == Ship_Flag)
    {
        return 0;
    }

    return ship->shiptype;
}

static sdword SEdisableStateEvent(Ship *ship)
{
    if (ship->disableStateStarted == 1)
    {
        return (Ship_Flag + 50);
    }

    return SOUND_ERR;
}

static void SEplayAmbientHandle(sdword *handle, Ship *ship, sdword event, sword pan, sword vol, real32 *tempEQ)
{
    SFXLUT *lut = SElutForEvent(event);
    sdword object = SEobjectForEvent(ship, event);
    bool32 handleOver;
    sdword patch;

    if (*handle == SOUND_NOTINITED)
    {
        handleOver = TRUE;
    }
    else
    {
        handleOver = soundover(*handle);
    }

    patch = GetPatch(lut, object, event);

    if (handleOver)
    {
        *handle = splayEPRV(ShipBank, lut->lookup[patch],
                            tempEQ, pan, SOUND_PRIORITY_LOW, vol);
    }
    else
    {
        soundvolume(*handle, vol);
        soundpan(*handle, pan);
        soundequalize(*handle, tempEQ);
    }
}

static void SEplayDamageHandle(sdword *handle, sdword event, sword pan, sword vol, real32 *tempEQ)
{
    if (event == SOUND_ERR)
    {
        if (*handle != SOUND_NOTINITED)
        {
            SEstopsoundhandle(handle, SOUND_FADE_STOPNOW);
        }
        return;
    }

    if ((*handle == SOUND_NOTINITED) || soundover(*handle))
    {
        *handle = splayEPRV(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, event)],
                            tempEQ, pan, SOUND_PRIORITY_LOW, vol);
    }
    else
    {
        soundvolume(*handle, vol);
        soundpan(*handle, pan);
        soundequalize(*handle, tempEQ);
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void SEinitcardiod(void)
{
    sdword i;

    for (i = 0; i < CARDIOD_POINTS; i++)
    {
        cardiod[i] = (1.0f + (real32)cos((CARDIOD_POINTS - i) / PI_UNDER_180)) * SFX_CARDIOD_FACTOR;
        if (cardiod[i] < SFX_CARDIOD_MIN)
        {
            cardiod[i] = SFX_CARDIOD_MIN;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : soundEventGetVolume
    Description : returns the current volume settings
    Inputs      : 3 pointers to hold the current volume settings
    Outputs     : sfxvol: current volume for sound effects
				  speechvol: current volume for the speech
				  musicvol: current volume for the music
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventGetVolume(real32 *sfxvol, real32 *speechvol, real32 *musicvol)
{
    *sfxvol = volSFX;
    *speechvol = volSpeech;
    *musicvol = volMusic;
}


/*-----------------------------------------------------------------------------
    Name        : soundEventSFXVol
    Description : sets the current sound effects volume, this function should
				  only be called in the NIS or animatic code since it scales
				  the input level by the NIS volume scalar.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventSFXVol(real32 level)
{
    if (level < 0.0f)
        level = 0.0f;
    else if (level > 1.0f)
        level = 1.0f;

    volSFX = SEapplyBackgroundMute(level * volNIS);
}

/*-----------------------------------------------------------------------------
    Name        : soundEventSFXMasterVol
    Description : sets the current sound effects master volume, also resets the
				  NIS volume scalar.  This should be called by the front end.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventSFXMasterVol(real32 level)
{
    volNIS = 1.0f;
    if (level < 0.0f)
        volSFX = SEapplyBackgroundMute(0.0f);
    else if (level > 1.0f)
        volSFX = SEapplyBackgroundMute(1.0f);
    else
        volSFX = SEapplyBackgroundMute(level);
}

/*-----------------------------------------------------------------------------
    Name        : soundEventSpeechVol
    Description : sets the current speech volume, this function should
				  only be called in the NIS or animatic code since it scales
				  the input level by the NIS volume scalar.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventSpeechVol(real32 level)
{
    if (level < 0.0f)
        level = 0.0f;
    else if (level > 1.0f)
        level = 1.0f;

	volSpeech = SEapplyBackgroundMute(level * volNIS);
}

/*-----------------------------------------------------------------------------
    Name        : soundEventSpeechMasterVol
    Description : sets the current speech master volume, also resets the
				  NIS volume scalar.  This should be called by the front end.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventSpeechMasterVol(real32 level)
{
    volNIS = 1.0f;
    if (level < 0.0f)
        volSpeech = SEapplyBackgroundMute(0.0f);
    else if (level > 1.0f)
        volSpeech = SEapplyBackgroundMute(1.0f);
    else
        volSpeech = SEapplyBackgroundMute(level);
}

/*-----------------------------------------------------------------------------
    Name        : soundEventMusicVol
    Description : sets the current music volume, this function should
				  only be called in the NIS or animatic code since it scales
				  the input level by the NIS volume scalar.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventMusicVol(real32 level)
{
    level *= volNIS;
    if (level < 0.0f)
        level = 0.0f;
    else if (level > 1.0f)
        level = 1.0f;

    volMusic = SEapplyBackgroundMute(level);
}

/*-----------------------------------------------------------------------------
    Name        : soundEventMusicMasterVol
    Description : sets the current music master volume, also resets the
				  NIS volume scalar.  This should be called by the front end.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventMusicMasterVol(real32 level)
{
    volNIS = 1.0f;
    if (level < 0.0f)
        volMusic = SEapplyBackgroundMute(0.0f);
    else if (level > 1.0f)
        volMusic = SEapplyBackgroundMute(1.0f);
    else
        volMusic = SEapplyBackgroundMute(level);
}

/*-----------------------------------------------------------------------------
    Name        : soundEventMusicMasterVolNOW
    Description : sets the current music master volume, also resets the NIS volume
				  scalar.  This ignores the volume fading code and sets the volume
				  immediatley. This should be called by the front end.
    Inputs      : level: volume to attempt to set
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void soundEventMusicMasterVolNOW(real32 level)
{
    if (level < 0.0f)
        volMusic = SEapplyBackgroundMute(0.0f);
    else if (level > 1.0f)
        volMusic = SEapplyBackgroundMute(1.0f);
    else
        volMusic = SEapplyBackgroundMute(level);
    bVolMusicNoFade = TRUE;
    volNIS = 1.0f;
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void soundEventMasterEQ(real32 *pmasterEQ)
{
    sdword i;

    for (i = 0; i < SOUND_EQ_SIZE; i++)
    {
        MasterEQ[i] = *pmasterEQ++;
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void soundEventSetActor(sdword actornum, bool32 bOn)
{
    if ((actornum > 0) && (actornum < 4))
    {
        bActorOn[actornum] = bOn;
    }
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void soundEventVocalSettings(bool32 bCommands, bool32 bStatus, bool32 bChatter)
{
    bCommandsOn = bCommands;
    bStatusOn = bStatus;
    bChatterOn = bChatter;
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void soundEventHearActor(sdword actornum)
{
	udword duration;
	sdword *phraseoffsets;
    if ((actornum > 0) && (actornum < 4))
    {
		if (SEselectsentence(actornum, (COMM_Selection & SPEECH_EVENT_MASK), 0, 0, &phraseoffsets, &duration) > 0)
		{
			soundstreamqueuePatch(streamhandle[actornum], speechfilehandle, *phraseoffsets++, SOUND_FLAGS_QUEUESTREAM, (sword)(SOUND_VOL_MAX * volSpeech), SOUND_PAN_CENTER,
							  SOUND_MONO, speechEventBitrate(), NULL, NULL, NULL, NULL, 0, 0.0f, 0.0f, actornum, COMM_Selection, FALSE);
		}
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void soundEventSetActorFlag(sdword actorflag, bool32 bOn)
{
    if (bOn)
    {
        bitSet(actorFlagsEnabled, actorflag);
    }
    else
    {
        bitClear(actorFlagsEnabled, actorflag);
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void SEprecalcVolTables(void)
{
    sdword i, j, index;
	
	/* do some precalculations on the volume and range tables, saves a divide, compare, 2 [] and 2 subtractions for every volume update */
	for (j = 0; j < (VolumeLUT->rows * VolumeLUT->columns); j += VolumeLUT->columns)
	{
		RangeFloatLUT[j] = 1.0f / (real32)RangeLUT->lookup[j];
		VolumeFloatLUT[j] = (real32)(SOUND_VOL_MAX - VolumeLUT->lookup[j]);

		for (i = 1; i < RangeLUT->columns; i++)
		{
			VolumeFloatLUT[j+i] = (real32)(VolumeLUT->lookup[j+i-1] - VolumeLUT->lookup[j+i]);
			RangeFloatLUT[j+i] = 1.0f / (real32)(RangeLUT->lookup[j+i] - RangeLUT->lookup[j+i-1]);
		}
	}

	/* do some precalcs on the Frequency table, saves a compare, [], addition and 2 subtractions for every SEequalize */
	for (j = 0; j < FrequencyLUT->columns; j++)
	{
		FreqLUT[j] = DefaultEQ[j] - FrequencyLUT->lookup[j];

		for (i = 1; i < FrequencyLUT->rows; i++)
		{
			index = i * FrequencyLUT->columns + j;
			FreqLUT[index] = FrequencyLUT->lookup[(i-1) * FrequencyLUT->columns + j] - FrequencyLUT->lookup[index];
		}
	}
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
-----------------------------------------------------------------------------*/
void soundEventInit(void)
{
#if SOUND

#if DIRECTSOUND
    char loadfile[100];
	sdword i, size;

    if (!enableSFX && !enableSpeech)
    {
        return;
    }

    if (soundinit(ghMainWindow, useWaveout) == SOUND_ERR)
    {
        enableSFX = FALSE;
        enableSpeech = FALSE;
    }

    for (i = 0; i < SOUND_EQ_SIZE; i++)
    {
        DefaultEQ[i] = 1.0f;
    }

    // used for calculating music volume in the game
    musicVolFactor = (MUSIC_MAXGAME_VOL - MUSIC_MINACTIVE_VOL) / (MUSIC_DISTANCE_MAX - MUSIC_DISTANCE_SILENT);
    musicInactiveFactor = (MUSIC_MAXGAME_VOL - MUSIC_MININACTIVE_VOL) / (MUSIC_DISTANCE_MAX - MUSIC_DISTANCE_SILENT);
	battleMusicVolFactor = (MUSIC_MAXBATTLE_VOL - MUSIC_MINBATTLE_VOL) / (MUSIC_DISTANCE_MAX - MUSIC_DISTANCE_SILENT);

    /* LOAD volume curves */
    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "Volume.lut");
    size = fileLoadAlloc(loadfile, &VolumeLUT, NonVolatile);
    VolumeFloatLUT = memAlloc(size, "Volume Table", NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "Range.lut");
    size = fileLoadAlloc(loadfile, &RangeLUT, NonVolatile);
	RangeFloatLUT = memAlloc(size, "Range Table", NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "Frequency.lut");
    size = fileLoadAlloc(loadfile, &FrequencyLUT, NonVolatile);
	FreqLUT = memAlloc(size, "Frequency Table", NonVolatile);

    /* do some pre-calculations on the volume, range and frequency tables */
	SEprecalcVolTables();

    /* initialze the cardiod lookup table */
    SEinitcardiod();

	SEinitHandles();
    memset(seSlipgateSoundState, 0, sizeof(seSlipgateSoundState));

    if (enableSFX)
    {
        SEloadbank();
    }

#if SPEECH
    if (enableSpeech)
    {
        if (speechEventInit(RACE_Sect) != SOUND_OK)
        {
            enableSpeech = FALSE;
        }
    }
#endif
    soundeventinited = TRUE;
    SEinitShipClassLUT();
#endif
#endif
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventClose(void)
{
    if (enableSFX || enableSpeech)
    {
        soundrestore();
        if (enableSFX || enableSpeech)
        {
            memFree(VolumeLUT);
            memFree(RangeLUT);
            memFree(FrequencyLUT);
            memFree(VolumeFloatLUT);
            memFree(RangeFloatLUT);
            memFree(FreqLUT);
            if (enableSFX)
            {
                memFree(seShockwaveAsteroidEventsLUT);
                memFree(ShipCmnEventsLUT);
                memFree(GunSpecEventsLUT);
                memFree(ShipEventsLUT);
                memFree(DerelictEventsLUT);
                memFree(SpecExpEventsLUT);
                memFree(SpecHitEventsLUT);
                memFree(UIEventsLUT);
                memFree(GunBank);
                memFree(ShipBank);
                memFree(SpecialEffectBank);
                memFree(UIBank);
                memFree(GunEventsLUT);
            }
            if (enableSpeech)
            {
                memFree(seCataclysmSpeechScriptAlloc6);
                memFree(seCataclysmSpeechScriptAlloc7);
                memFree(seCataclysmSpeechScriptAlloc0);
                memFree(seCataclysmSpeechScriptAlloc1);
                memFree(seCataclysmSpeechScriptAlloc2);
                memFree(seCataclysmSpeechScriptAlloc3);
                memFree(seCataclysmSpeechScriptAlloc4);
                memFree(seCataclysmSpeechScriptAlloc5);
            }
        }
    }
}


void soundEventShutdown(void)
{
    if (enableSFX || enableSpeech)
		soundclose();
//        sounddeactivate(TRUE);
//	soundrestore();
}


void soundEventRestart(void)
{
	if (enableSFX || enableSpeech)
	{
		soundreinit(ghMainWindow);
		soundEventPlayMusic(SOUND_FRONTEND_TRACK);
	}

//        sounddeactivate(FALSE);
//    if (soundinit(ghMainWindow, useWaveout) == SOUND_ERR)
//    {
//        enableSFX = FALSE;
//        enableSpeech = FALSE;
//    }
//	soundpause(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventStopSFX(real32 fadetime)
{
    Ship *ship;
    Derelict *pDerelict;
    Node *objnode = universe.RenderList.tail;
	
	if (enableSFX)
	{
		soundstopallSFX(fadetime, FALSE);
	
		while (objnode != NULL)
		{
			ship = (Ship *)listGetStructOfNode(objnode);
	
			if (ship->objtype == OBJ_DerelictType)
			{
				pDerelict = (Derelict *)ship;
                soundEventDerelictRemove(pDerelict);
			}
			else if (ship->objtype == OBJ_ShipType)
			{
				soundEventShipRemoveHandles(ship);
			}
			objnode = objnode->prev;
		}
	}
}


void soundEventPause(bool32 bPause)
{
    if (!soundeventinited)
    {
        return;
    }

    if (bPause != soundpaused)
    {
        soundpaused = bPause;
        soundpause(bPause);

        if (bPause)
        {
            /* clean up the speech queue */
            speechEventCleanup();
        }
    }
}
/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventUpdate(void)
{
#if SOUND
    Node *objnode = universe.RenderList.tail;
    Ship *ship;
    ShipClass shipclass;
    sword vol;
    sword pan;
    real32 dist;
    real32 velocity;
    real32 velratio = 1.0f;
    udword iscapship;
    sdword  numships = 0,
            numcapships = 0,
            numabmient = 0;
    static sdword numbattlechatter = 0;
    sdword beastStatusWindowLoopCount = 0;
    sword shipangle = 0;
    static real32 tempEQ[SOUND_EQ_SIZE];
    GunInfo *gunInfo;
    sdword i;
    Gun *gun;
    bool32 specialon;
    bool32 ambienton;
    bool32 statusWindowSpecialHandled;
	CloakGeneratorSpec *CGspec;
    GravWellGeneratorSpec *GWspec;
    CommandToDo *command;
    Derelict *pDerelict;
    real32 damageratio = 1.0f;
    sdword ambient;
    sdword randomAmbient;
	ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;
	static real32 strikescale = 5.0f;
	bool32 noAmbient = FALSE;

    if (!soundeventinited || soundpaused)
    {
        return;
    }

    // update the music volume
    musicEventUpdateVolume();


	if (smSensorsActive || !mrRenderMainScreen)
	{
		return;
	}

    nearestShipDistance = REALlyBig;
    nearestShipMoving = FALSE;

	while (objnode != NULL)
    {
		noAmbient = FALSE;
        ship = (Ship *)listGetStructOfNode(objnode);

		// only want to do this if sound fx are on
		if (!enableSFX)
		{
			break;
		}

		// deal with derelicts first
        if (ship->objtype == OBJ_DerelictType)
        {
            // play the derelict ambience here
            pDerelict = (Derelict *)ship;

            dist = (real32)fsqrt(pDerelict->cameraDistanceSquared);

            if (dist < nearestShipDistance)
            {
                nearestShipDistance = dist;
            }

            if (SEinrange(DerelictAmbience, dist))
            {
                vol = SEequalize(DerelictAmbience, dist, tempEQ);
				pan = getPanAngle(pDerelict->posinfo.position, pDerelict->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);

                if ((pDerelict->ambientSoundHandle == SOUND_NOTINITED) ||
					soundover(pDerelict->ambientSoundHandle))
                {
                    pDerelict->ambientSoundHandle = splayEPRV(ShipBank, DerelictEventsLUT->lookup[GetPatch(DerelictEventsLUT, pDerelict->derelicttype, Derelict_Ambience)],
                                                               tempEQ, pan, SOUND_PRIORITY_LOW, vol);
                }
                else
                {
                    soundvolume(pDerelict->ambientSoundHandle, vol);
                    soundpan(pDerelict->ambientSoundHandle, pan);
                    soundequalize(pDerelict->ambientSoundHandle, tempEQ);
                }

				// now we have the random ambience
                if ((pDerelict->randomSoundHandle == SOUND_NOTINITED) ||
					soundover(pDerelict->randomSoundHandle))
                {
                    if (universe.phystimeelapsed > pDerelict->nextRandomTime)
                    {
                        pDerelict->randomSoundHandle = splayEPRV(ShipBank, DerelictEventsLUT->lookup[GetPatch(DerelictEventsLUT, pDerelict->derelicttype, Derelict_RandomAmbience)],
                                                            tempEQ, pan, SOUND_PRIORITY_LOW, vol);
                        if (pDerelict->randomSoundHandle == SOUND_NOTINITED)
                        {
							pDerelict->nextRandomTime = universe.phystimeelapsed + RANDOM_AMBIENCE_MINFREQ;
                        }
                    }
                }
                else
                {
                    if (soundover(pDerelict->randomSoundHandle))
                    {
                        pDerelict->randomSoundHandle = SOUND_NOTINITED;
                        pDerelict->nextRandomTime = universe.phystimeelapsed + RANDOM_AMBIENCE_MINFREQ
                                                    + ((real32)(ranRandom(RAN_SoundGameThread) % RANDOM_AMBIENCE_ADDRANDOM) / 100.0f);
                    }
                    else
                    {
                        soundvolume(pDerelict->randomSoundHandle, vol);
                        soundpan(pDerelict->randomSoundHandle, pan);
                        soundequalize(pDerelict->randomSoundHandle, tempEQ);
                    }
                }
            }
            else
            {
                SEstopsoundhandle(&(pDerelict->ambientSoundHandle), SOUND_FADE_STOPNOW);
                SEstopsoundhandle(&(pDerelict->randomSoundHandle), SOUND_FADE_STOPNOW);
            }

            goto nextship;
        }
        else if (ship->objtype != OBJ_ShipType)
        {
			// if this wasn't a Derelict or a Ship then skip it
            goto nextship;
        }

		// ok, now we're into the SHIP stuff
        shipclass = ship->staticinfo->shipclass;
        dist = (real32)fsqrt(ship->cameraDistanceSquared);
		
		// a little hack here so that these ships have reasonable volume curves
		if (ship->shiptype == MiningBase)
		{
			shipclass = CLASS_Mothership;
		}
		else if (ship->shiptype == ResearchStation)
		{
			shipclass = CLASS_Carrier;
		}

		// need to do this for the ships that aren't playing sounds
		if ((shipclass == CLASS_Fighter) || (shipclass == CLASS_Micro))
		{
			ship->soundevent.coverage = rndComputeOverlap(ship, 5.0f);
		}
		else if (shipclass == CLASS_Corvette)
		{
			ship->soundevent.coverage = rndComputeOverlap(ship, 2.2f);
		}
		else
		{
			// all other ships don't have velocity ratio or have the coverage affect the engine volume
			ship->soundevent.coverage = rndComputeOverlap(ship, 0.4f);
		}

        // have we reached the maximum number of sounds for these types of ships?
		if (((numships >= SFX_MAX_STRIKEENGINES) &&
             ((shipclass == CLASS_Fighter) || (shipclass == CLASS_Corvette) || (shipclass == CLASS_Micro))) ||
            ((numcapships >= SFX_MAX_CAPENGINES) &&
             (shipclass != CLASS_Fighter) &&
             (shipclass != CLASS_Mothership)))
        {
            if (shipclass != CLASS_Mothership)
            {
				SEstopengine(ship, TRUE);
				SEstopsoundhandle(&(ship->soundevent.specialHandle), SOUND_FADE_STOPNOW);
				SEstopsoundhandle(&(ship->soundevent.ambientHandle), SOUND_FADE_STOPNOW);
				SEstopsoundhandle(&(ship->soundevent.damageHandle), SOUND_FADE_STOPNOW);
				SEstopsoundhandle(&(ship->soundevent.randomHandle), SOUND_FADE_STOPNOW);
				SEstopsoundhandle(&(ship->soundevent.gunHandle), SOUND_FADE_STOPNOW);
				SEstopsoundhandle(&(ship->soundevent.hyperspaceHandle), SOUND_FADE_STOPNOW);
				goto playloopingguns;	// this is only the gun firing sounds
            }
        }

		// has this ship just exploded?
		if (ship->soundevent.engineHandle == SOUND_SHIP_EXPLODED)
		{
			soundEventShipRemove(ship);

			goto nextship;
		}

		// is this ship even in range to be heard?
		if (!SEinrange(shipclass, dist))
		{
			// then shut the engind down if its playing
			SEstopengine(ship, TRUE);
			goto othersounds;
		}

		// need to keep track of the number of ships
		if (shipclass != CLASS_Mothership)
		{
			numships++;
			if (isCapitalShip(ship))
			{
				numcapships++;
			}
		}

		// figure out all the variables
		pan = getPanAngle(ship->enginePosition, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
		vol = SEequalize(shipclass, dist, tempEQ);
		shipangle = SEgetHeadingAngle((float *)ship);
		velocity = fsqrt(vecMagnitudeSquared(ship->posinfo.velocity));
		
		// need this for setting the music volume
		if (dist < nearestShipDistance)
		{
			nearestShipDistance = dist;
			if (velocity > SFX_FLOAT_VELOCITY)
			{
				nearestShipMoving = TRUE;
			}
		}

		// figure out the coverage for this ship (how much it is visibly overlapped on screen)
		// also figure out the volume scalar for the coverage and the velocity to max velocity ratio
		if ((shipclass == CLASS_Fighter) || (shipclass == CLASS_Micro))
		{
			vol = (sword)(vol * (1.0f - ship->soundevent.coverage));
			velratio = FIGHTER_VELOCITY_LOWPITCH; + (velocity / ship->staticinfo->staticheader.maxvelocity * FIGHTER_VELOCITY_SCALE);
			if (velratio > FIGHTER_VELOCITY_HIGHPITCH)
			{
				velratio = FIGHTER_VELOCITY_HIGHPITCH;
			}
		}
		else if (shipclass == CLASS_Corvette)
		{
			vol = (sword)(vol * (1.0f - ship->soundevent.coverage));
			velratio = CORVETTE_VELOCITY_LOWPITCH; + (velocity / ship->staticinfo->staticheader.maxvelocity * CORVETTE_VELOCITY_SCALE);
			if (velratio > CORVETTE_VELOCITY_HIGHPITCH)
			{
				velratio = CORVETTE_VELOCITY_HIGHPITCH;
			}
		}
		else
		{
			// all other ships don't have velocity ratio or have the coverage affect the engine volume
			ship->soundevent.coverage = rndComputeOverlap(ship, 0.4f);
		}

		// need to figure out if this ship is actually doing something and not just drifting
		if (ship->soundevent.engineState != SOUND_STARTING)
		{
			if ((shipclass == CLASS_Fighter) || (shipclass == CLASS_Corvette) || (shipclass == CLASS_Micro))
			{
				// if this ship isn't actually doing anything, then don't turn on the engines
				command = getShipAndItsCommand(&universe.mainCommandLayer, ship);
				if (command != NULL)
				{
					if ((command->ordertype.order == COMMAND_NULL) ||
						(command->ordertype.order == COMMAND_MP_HYPERSPACEING))
					{
						if (velocity <= 100.0f)
						{
							vol = 0;
						}
					}
					else if ((command->ordertype.order == COMMAND_MILITARYPARADE)  &&
							 (velocity <= 2.0f * SFX_FLOAT_VELOCITY))
					{
						vol = 0;
					}
					else
					{
						if (nisIsRunning && (velocity <= SFX_NIS_FLOAT_VELOCITY))
						{
							vol = 0;
						}
						else if (velocity <= SFX_FLOAT_VELOCITY)
						{
							vol = 0;
						}
					}
				}
				else
				{
					vol = 0;
				}
			}
			else
			{
				// cap ships should always play unless they're moving
				if (velocity <= 0.0f)
				{
					vol = 0;
				}
			}

		}

		// ENGINE SOUNDS
		if (vol <= SOUND_VOL_MIN)
		{
			if (ship->soundevent.engineHandle > SOUND_NOTINITED)
			{
				SEstopengine(ship, TRUE);
			}
			goto othersounds;
		}
		else
		{
			if ((ship->soundevent.engineHandle == SOUND_NOTINITED) ||
				soundover(ship->soundevent.engineHandle))
			{
				// start the engine sound
				if (velratio > 1.0f)
				{
					if (shipclass == CLASS_Corvette)
					{
						ship->soundevent.engineHandle = splayFPRVL(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ship->shiptype, ShipCmn_Engine)],
											   tempEQ, velratio, pan, SOUND_PRIORITY_MAX - 1, vol, TRUE, TRUE, FALSE);
					}
					else if ((shipclass == CLASS_Fighter) || (shipclass == CLASS_Micro))
					{
						ship->soundevent.engineHandle = splayFPRVL(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ship->shiptype, ShipCmn_Engine)],
											   tempEQ, velratio, pan, SOUND_PRIORITY_MAX - 2, vol, TRUE, TRUE, FALSE);
					}
				}
				else
				{
					ship->soundevent.engineHandle = splayEPRVL(ShipBank, ShipCmnEventsLUT->lookup[GetPatch(ShipCmnEventsLUT, ship->shiptype, ShipCmn_Engine)],
										   tempEQ, pan, SOUND_PRIORITY_MAX, vol, TRUE);
				}
			}
			else
			{
				if ((ship->soundevent.engineState == SOUND_STARTING) && (velocity > SFX_FLOAT_VELOCITY))
				{
					ship->soundevent.engineState = SOUND_PLAYING;
				}
	
				soundvolume(ship->soundevent.engineHandle, vol);
				soundpan(ship->soundevent.engineHandle, pan);
				if (velratio > 1.0f)
				{
					soundfrequency(ship->soundevent.engineHandle, velratio);
				}
				soundequalize(ship->soundevent.engineHandle, tempEQ);
			}
	
			if ((shipclass == CLASS_Fighter) || (shipclass == CLASS_Micro))
			{
				if (FIGHTER_DOPPLER_USEVELOCITY)
				{
					soundshipheading(ship->soundevent.engineHandle, shipangle, FIGHTER_DOPPLER_HIGH, FIGHTER_DOPPLER_LOW, velratio, FIGHTER_DOPPLER_SCALE);
				}
				else
				{
					soundshipheading(ship->soundevent.engineHandle, shipangle, FIGHTER_DOPPLER_HIGH, FIGHTER_DOPPLER_LOW, 1.0f, FIGHTER_DOPPLER_SCALE);
				}
			}
			if (shipclass == CLASS_Corvette)
			{
				if (CORVETTE_DOPPLER_USEVELOCITY)
				{
					soundshipheading(ship->soundevent.engineHandle, shipangle, CORVETTE_DOPPLER_HIGH, CORVETTE_DOPPLER_LOW, velratio, CORVETTE_DOPPLER_SCALE);
				}
				else
				{
					soundshipheading(ship->soundevent.engineHandle, shipangle, CORVETTE_DOPPLER_HIGH, CORVETTE_DOPPLER_LOW, 1.0f, CORVETTE_DOPPLER_SCALE);
				}
			}
		}

othersounds:
		// now we deal with the special sounds, ambients, damage ambients and random ambients			
		if (!SEinrange(shipclass + AMBIENT_OFFSET, dist) &&
            (!SEisSlipgateShipType(ship->shiptype) ||
             !SEinrange(CATA_SLIPGATE_SOUND_CLASS, dist)))
		{
			SEstopsoundhandle(&(ship->soundevent.specialHandle), SOUND_FADE_STOPNOW);
			SEstopsoundhandle(&(ship->soundevent.ambientHandle), SOUND_FADE_STOPNOW);
			SEstopsoundhandle(&(ship->soundevent.damageHandle), SOUND_FADE_STOPNOW);
			SEstopsoundhandle(&(ship->soundevent.randomHandle), SOUND_FADE_STOPNOW);
			SEstopsoundhandle(&(ship->soundevent.hyperspaceHandle), SOUND_FADE_STOPNOW);
			goto playgimbles;
		}
		vol = SEequalize(shipclass + AMBIENT_OFFSET, dist, tempEQ);
		vol = (sword)(vol * (1.0f - ship->soundevent.coverage));
		pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);
		specialon = FALSE;
		ambienton = TRUE;
        statusWindowSpecialHandled = FALSE;

        if (ship->statusWindow.state != 0)
        {
            statusWindowSpecialHandled = TRUE;
            if (beastStatusWindowLoopCount < CATA_BEAST_CONVERSION_MAX_ACTIVE_LOOPS &&
                ((sdword)ship->shiptype < BEAST_FIRST_SHIP ||
                 (sdword)ship->shiptype >= BEAST_LAST_SHIP + 1) &&
                ship->shiptype != dCrystal0 &&
                ship->shiptype != dCrystal1)
            {
                sword beastLoopVol;

                if (ccIsInPilotMode() && ccIsPilotingShip(ship))
                {
                    beastLoopVol = CATA_BEAST_CONVERSION_PILOT_LOOP_VOLUME;
                }
                else
                {
                    beastLoopVol = SEequalize(shipclass, dist, tempEQ);
                }

                if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
                    soundover(ship->soundevent.specialHandle))
                {
                    ship->soundevent.specialHandle =
                        splayFPRVL(ShipBank,
                                   ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, CATA_BEAST_CONVERSION_LOOP_EVENT)],
                                   tempEQ,
                                   (real32)SOUND_DEFAULT,
                                   pan,
                                   SOUND_PRIORITY_NORMAL,
                                   beastLoopVol,
                                   FALSE,
                                   TRUE,
                                   FALSE);
                }
                else
                {
                    soundvolumeF(ship->soundevent.specialHandle, beastLoopVol, 0.0f);
                    soundpanF(ship->soundevent.specialHandle, pan, 0.0f);
                    soundequalize(ship->soundevent.specialHandle, tempEQ);
                }

                beastStatusWindowLoopCount++;
            }
            else if (ship->soundevent.specialHandle > SOUND_NOTINITED)
            {
                SEstopsoundhandle(&(ship->soundevent.specialHandle), SOUND_FADE_STOPNOW);
            }
        }

        if (!statusWindowSpecialHandled)
        {
		    switch (ship->shiptype)
		    {
			    case bCloakedFighter:
				    break;

			    case kCloakGenerator:
				    CGspec = (CloakGeneratorSpec *)ship->ShipSpecifics;
				    if (CGspec->CloakOn &&
					    (universe.curPlayerIndex == ship->playerowner->playerIndex))
				    {
					    specialon = TRUE;
					    ambienton = FALSE;
					    
					    if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
						    soundover(ship->soundevent.specialHandle))
					    {
						    ship->soundevent.specialHandle = splayEPRVL(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, Ship_CloakingLoop)],
																		  tempEQ, pan, SOUND_PRIORITY_MAX, vol, TRUE);
					    }
				    }
				    break;

			    case kGravWellGenerator:
				    GWspec = (GravWellGeneratorSpec *)ship->ShipSpecifics;
				    if (GWspec->GravFieldOn)
				    {
					    if (SEinrange(Spec_GravWellGeneratorOn, dist))
					    {
						    specialon = TRUE;
						    ambienton = FALSE;

						    vol = SEequalize(Spec_GravWellGeneratorOn, dist, tempEQ);

						    if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
							    soundover(ship->soundevent.specialHandle))
						    {
							    ship->soundevent.specialHandle = splayEPRV(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, Ship_GravWellGenerator)],
																			  tempEQ, pan, SOUND_PRIORITY_MAX, vol);
						    }
					    }
				    }
				    break;

			    case sWorker:
			    case bWorker:
				    if ((ship->rceffect != NULL) &&
					    ((ship->rcstate1 == CATA_WORKER_RESOURCE_ASTEROID_STATE) ||
					     (ship->rcstate1 == CATA_WORKER_RESOURCE_DUST_STATE)))
				    {
					    specialon = TRUE;
					    ambienton = FALSE;

					    if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
						    soundover(ship->soundevent.specialHandle))
					    {
						    sdword workerEvent = (ship->rcstate1 == CATA_WORKER_RESOURCE_ASTEROID_STATE)
						        ? CATA_WORKER_RESOURCE_ASTEROID_EVENT
						        : CATA_WORKER_RESOURCE_DUST_EVENT;
						    ship->soundevent.specialHandle = splayFPRVL(ShipBank,
																	    ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, workerEvent)],
																	    tempEQ,
																	    (real32)SOUND_DEFAULT,
																	    pan,
																	    SOUND_PRIORITY_NORMAL,
																	    vol,
																	    TRUE,
																	    TRUE,
																	    FALSE);
					    }
				    }
				    break;

			    case kSalCapCorvette:
				    // is the tractor beam on?
				    if ((((SalCapCorvetteSpec *)ship->ShipSpecifics)->tractorBeam == TRUE) &&
					    (SEinrange(shipclass, dist)))
				    {
					    // is the sound for it playing?
					    if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
						    soundover(ship->soundevent.specialHandle))
					    {
						    ship->soundevent.specialHandle = splayEPRVL(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, Ship_Salvage)],
																		  tempEQ, pan, SOUND_PRIORITY_MAX, (sword)(vol * (1.0f - ship->soundevent.coverage)), TRUE);
					    }
					    specialon = TRUE;
					    ambienton = FALSE;
				    }
				    break;

			    case sRepairDroid:
			    case bRepairDroid:
				    if (ship->rceffect != NULL)
				    {
					    if ((ship->soundevent.specialHandle == SOUND_NOTINITED) ||
						    soundover(ship->soundevent.specialHandle))
					    {
						    ship->soundevent.specialHandle = splayEPRVL(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, Ship_RepairLoop)],
																		  tempEQ, pan, SOUND_PRIORITY_MAX, vol, TRUE);
					    }
					    specialon = TRUE;
					    ambienton = FALSE;
				    }
				    break;

                case dSlipgateSmall:
                case dSlipgateMedium:
                case dSlipgateLarge:
                    if (SEslipgateHandleShip(ship, dist, pan, &vol, tempEQ))
                    {
                        specialon = TRUE;
                        ambienton = FALSE;
                    }
                    break;
		    }

		    // deal with the special sound
		    if ((specialon) && (ship->soundevent.specialHandle != SOUND_NOTINITED))
		    {
			    soundvolume(ship->soundevent.specialHandle, vol);
			    soundpan(ship->soundevent.specialHandle, pan);
			    soundequalize(ship->soundevent.specialHandle, tempEQ);
		    }
		    else if (ship->soundevent.specialHandle > SOUND_NOTINITED)
		    {
			    SEstopsoundhandle(&(ship->soundevent.specialHandle), SOUND_FADE_STOPTIME);
		    }
        }

		// DAMAGE AMBIENT
		if (ambienton)
		{
			damageratio = (real32)ship->health * (real32)((ShipStaticInfo *)ship->staticinfo)->oneOverMaxHealth;
			ambient = SEambientEventForShip(ship, damageratio);

			if (ship->soundevent.lastAmbient != ambient)
			{
				if (ship->soundevent.ambientHandle != SOUND_NOTINITED)
				{
					SEstopsoundhandle(&(ship->soundevent.ambientHandle), SOUND_FADE_STOPNOW);
				}
				ship->soundevent.lastAmbient = ambient;
			}

			SEplayAmbientHandle(&(ship->soundevent.ambientHandle), ship, ambient, pan, vol, tempEQ);

			if (ship->soundevent.randomHandle == SOUND_NOTINITED)
			{
				if (universe.phystimeelapsed > ship->soundevent.nextRandom)
				{
					randomAmbient = SErandomAmbientEventForShip(ship, damageratio);

					if (randomAmbient != SOUND_ERR)
					{
						SEplayAmbientHandle(&(ship->soundevent.randomHandle), ship, randomAmbient, pan, vol, tempEQ);
						if (ship->soundevent.randomHandle == SOUND_NOTINITED)
						{
							ship->soundevent.nextRandom = universe.phystimeelapsed + RANDOM_AMBIENCE_MINFREQ;
						}
					}
				}
			}
			else if (soundover(ship->soundevent.randomHandle))
			{
				ship->soundevent.randomHandle = SOUND_NOTINITED;
				ship->soundevent.nextRandom = universe.phystimeelapsed + SErandomAmbientCooldown(ship);
			}
			else
			{
				soundvolume(ship->soundevent.randomHandle, vol);
				soundpan(ship->soundevent.randomHandle, pan);
				soundequalize(ship->soundevent.randomHandle, tempEQ);
			}

			SEplayDamageHandle(&(ship->soundevent.damageHandle), SEdisableStateEvent(ship), pan, vol, tempEQ);
		}
		else
		{
			// out of range so turn these puppies off
			SEstopsoundhandle(&(ship->soundevent.ambientHandle), SOUND_FADE_STOPNOW);
			SEstopsoundhandle(&(ship->soundevent.randomHandle), SOUND_FADE_STOPNOW);
			SEstopsoundhandle(&(ship->soundevent.damageHandle), SOUND_FADE_STOPNOW);
		}

		// HYPERSPACING
		shipSinglePlayerGameInfo = ship->shipSinglePlayerGameInfo;
		if (!singlePlayerGame ||
		    (ship->playerowner == universe.curPlayerPtr &&
		     (ship->specialFlags2 & 0x00100000) == 0))
		{
			if (shipSinglePlayerGameInfo->hsState == HS_SLICING_INTO)
			{
				if ((ship->soundevent.hyperspaceHandle == SOUND_NOTINITED) ||
					soundover(ship->soundevent.hyperspaceHandle))
				{
					ship->soundevent.hyperspaceHandle = splayFPRVL(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, CATA_RETAIL_HYPERDRIVE_LUT_EVENT)],
										   tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MAX, vol, TRUE, TRUE, FALSE);
				}
				else
				{
					soundvolume(ship->soundevent.hyperspaceHandle, vol);
					soundpan(ship->soundevent.hyperspaceHandle, pan);
					soundequalize(ship->soundevent.hyperspaceHandle, tempEQ);
				}
			}
			else if (shipSinglePlayerGameInfo->hsState == HS_SLICING_OUTOF)
			{
				if ((ship->soundevent.hyperspaceHandle == SOUND_NOTINITED) ||
					soundover(ship->soundevent.hyperspaceHandle))
				{
					ship->soundevent.hyperspaceHandle = splayFPRVL(ShipBank, ShipEventsLUT->lookup[GetPatch(ShipEventsLUT, 0, CATA_RETAIL_HYPERDRIVE_OFF_LUT_EVENT)],
										   tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MAX, vol, TRUE, TRUE, FALSE);
				}
				else
				{
					soundvolume(ship->soundevent.hyperspaceHandle, vol);
					soundpan(ship->soundevent.hyperspaceHandle, pan);
					soundequalize(ship->soundevent.hyperspaceHandle, tempEQ);
				}
			}
		}

playgimbles:		
		// NEW Gimble stuff
		if (ship->cataclysmGunInfo != NULL)
		{
            GunSoundType activeSoundType;
			gunInfo = ship->cataclysmGunInfo;
            activeSoundType = gunGetActiveSoundType(ship, &gunInfo->guns[0]);

			if (SEinrange(activeSoundType + GUNMOVE_OFFSET, dist))
			{
				// check to see if they are playing
				vol = SEequalize(activeSoundType + GUNMOVE_OFFSET, dist, tempEQ);
				vol = (sword)(vol * (1.0f - ship->soundevent.coverage));

				for (i = 0; i < gunInfo->numGuns; i++)
				{
					gun = &gunInfo->guns[i];
					if (((gun->gimblehandle != SOUND_NOTINITED) &&
						!soundover(gun->gimblehandle)) &&
						(vol > SOUND_VOL_MIN))
					{
						soundvolume(gun->gimblehandle, vol);
						soundpan(gun->gimblehandle, pan);
						soundequalize(gun->gimblehandle, tempEQ);
					}
				}
			}
			else
			{
				// out of range so see if we need to shut off the gimbles
				for (i = 0; i < gunInfo->numGuns; i++)
				{
					gun = &gunInfo->guns[i];
					if (gun->gimblehandle != SOUND_NOTINITED)
					{
						SEstopsoundhandle(&(gun->gimblehandle), SOUND_FADE_STOPNOW);
					}
				}
			}
		}

playloopingguns:
		// get the pan from the center of the ship
		pan = getPanAngle(ship->posinfo.position, ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize, dist);

		// check the burst fire guns
		if (ship->soundevent.burstfiring)
		{
            GunSoundType activeSoundType;
            Gun *burstGun = &ship->cataclysmGunInfo->guns[ship->burstGunIndex];
            sdword patchObject = 0;
            sdword patchEvent = Gun_WeaponFireLooped;
            activeSoundType = (GunSoundType)SEgetShipClass((Ship *)burstGun);

			if (SEinrange(activeSoundType + GUNSHOT_OFFSET, dist))
			{
				if (ship->aistateattack == STATE_BREAK)
				{
					soundEventBurstStop(ship, burstGun);
				}
				else
				{
					vol = SEequalize(activeSoundType + GUNSHOT_OFFSET, dist, tempEQ);
					vol = (sdword)(vol * (1.0f - (ship->soundevent.coverage * 0.5)));

					if (((ship->soundevent.burstHandle == SOUND_NOTINITED) ||	// do I think the sound is done
						soundover(ship->soundevent.burstHandle)) &&				// is the sound actually not playing
						(vol > SOUND_VOL_MIN))									// is the volume > 0
					{
                        if (ship->shiptype == sMultiBeamFrigate || ship->shiptype == bMultiBeamFrigate)
                        {
                            patchEvent = Gun_WeaponFireMultiBeamLooped;
                        }
                        else if (ship->shiptype == dAcolyte)
                        {
                            patchEvent = Gun_WeaponFireAcolyteLooped;
                        }
                        else
                        {
                            patchObject = activeSoundType;
                        }

                        if (patchObject == 0 && patchEvent != Gun_WeaponFireLooped)
                        {
                            ship->soundevent.burstHandle = splayEPRVL(GunBank, GunSpecEventsLUT->lookup[GetPatch(GunSpecEventsLUT, patchObject, patchEvent)],
                                                            tempEQ, pan, SOUND_PRIORITY_MAX, vol, TRUE);
                        }
                        else
                        {
                            ship->soundevent.burstHandle = splayEPRVL(GunBank, GunEventsLUT->lookup[GetPatch(GunEventsLUT, patchObject, patchEvent)],
                                                            tempEQ, pan, SOUND_PRIORITY_MAX, vol, TRUE);
                        }
					}
					else
					{
						soundvolume(ship->soundevent.burstHandle, vol);
						soundpan(ship->soundevent.burstHandle, pan);
						soundequalize(ship->soundevent.burstHandle, tempEQ);
					}
				}
			}
			else
			{
				SEstopsoundhandle(&(ship->soundevent.burstHandle), SOUND_FADE_STOPNOW);
			}
		}


		// do the Ion Carving code here

nextship:
		objnode = objnode->prev;
    }

#endif
}

void soundEventInitStruct(SOUNDEVENT *pseStruct)
{
    pseStruct->engineHandle = SOUND_NOTINITED;
    pseStruct->engineState = SOUND_NOTINITED;
    pseStruct->ambientHandle = SOUND_NOTINITED;
	pseStruct->damageHandle = SOUND_NOTINITED;
    pseStruct->specialHandle = SOUND_NOTINITED;
	pseStruct->hyperspaceHandle = SOUND_NOTINITED;
    pseStruct->actorNum = SOUND_NOTINITED;

	pseStruct->coverage = 0.0f;
	pseStruct->distance = REALlyBig;
	pseStruct->pan = SOUND_PAN_CENTER;
	pseStruct->engineVol = SOUND_NOTINITED;

	pseStruct->randomHandle = SOUND_NOTINITED;
    pseStruct->nextRandom = universe.totaltimeelapsed;

    pseStruct->lastAmbient = SOUND_NOTINITED;

    pseStruct->timeLastStatus = universe.totaltimeelapsed;

    pseStruct->burstHandle = SOUND_NOTINITED;
    pseStruct->burstfiring = FALSE;

    pseStruct->gunHandle = SOUND_NOTINITED;
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventShipDied(Ship *deadship)
{
    if (!enableSFX)
    {
        if (deadship != NULL)
            deadship->soundevent.engineHandle = SOUND_SHIP_EXPLODED;
    }
    else
    {
        SEstopengine(deadship, TRUE);
        soundEventShipRemove(deadship);
        SEstopsoundhandle(&deadship->soundevent.specialHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&deadship->soundevent.ambientHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&deadship->soundevent.damageHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&deadship->soundevent.randomHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&deadship->soundevent.gunHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&deadship->soundevent.hyperspaceHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&deadship->soundevent.burstHandle, SOUND_FADE_STOPNOW);
        if (deadship != NULL)
        {
            deadship->soundevent.engineHandle = SOUND_NOTINITED;
            deadship->soundevent.engineHandle = SOUND_SHIP_EXPLODED;
        }
    }

    if (enableSpeech)
    {
        if (deadship != NULL)
        {
            soundEventStopActorStreams(deadship);
            if (deadship->playerowner == universe.curPlayerPtr &&
                !(deadship->specialFlags2 & BIT20))
            {
                if (deadship->staticinfo->shipIsCapital ||
                    deadship->staticinfo->shipclass == CLASS_Resource ||
                    deadship->staticinfo->shipclass == CLASS_Component)
                {
                    speechEventFleet(STAT_F_MajorShip_Dies, deadship->shiptype, deadship->playerowner->playerIndex);
                }
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventShipRemove(Ship *ship)
{
    if (enableSFX && ship->soundevent.burstfiring)
    {
        sdword burstHandle = ship->soundevent.burstHandle;

        ship->soundevent.burstfiring = FALSE;
        if (burstHandle > SOUND_NOTINITED)
        {
            soundstop(burstHandle, 0.5f);
            ship->soundevent.burstHandle = SOUND_NOTINITED;
            ship->soundevent.gunHandle = SOUND_NOTINITED;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : soundEventShipRemoveHandles
    Description : Stops all sound handles for a ship being removed from the game.
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventShipRemoveHandles(Ship *ship)
{
    if (enableSFX)
    {
        SEstopengine(ship, TRUE);
        soundEventShipRemove(ship);
        SEstopsoundhandle(&ship->soundevent.specialHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&ship->soundevent.ambientHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&ship->soundevent.damageHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&ship->soundevent.randomHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&ship->soundevent.gunHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&ship->soundevent.hyperspaceHandle, SOUND_FADE_STOPNOW);
        SEstopsoundhandle(&ship->soundevent.burstHandle, SOUND_FADE_STOPNOW);
        if (ship != NULL)
            ship->soundevent.engineHandle = SOUND_NOTINITED;
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventDerelictRemove(Derelict *pDerelict)
{
    if (enableSFX)
    {
		SEstopsoundhandle(&(pDerelict->ambientSoundHandle), SOUND_FADE_STOPNOW);
		SEstopsoundhandle(&(pDerelict->randomSoundHandle), SOUND_FADE_STOPNOW);
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 SEinrange(sdword shipclass, real32 distance)
{
    if (RangeLUT != NULL)
    {
        if (shipclass < RangeLUT->rows)
        {
            if (distance <= (real32)RangeLUT->lookup[shipclass * RangeLUT->columns + 2])
            {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 SEinrangeSqr(sdword shipclass, real32 distancesqr)
{
    if (RangeLUT != NULL)
    {
        if (shipclass < RangeLUT->rows)
        {
            real32 rangelut = (real32)RangeLUT->lookup[shipclass * RangeLUT->columns + 2];
            if (distancesqr <= (rangelut*rangelut))
            {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sword SEequalize(sdword objtype, real32 distance, real32 *eq)
{
    sdword i, pos, index = SOUND_ERR, Findex = SOUND_ERR;
    real32 mult, vol = 0.0f;

    if ((RangeLUT != NULL) && (FrequencyLUT != NULL) && (VolumeLUT != NULL))
    {
        if ((objtype < RangeLUT->rows) && (objtype < VolumeLUT->rows))
        {
            pos = objtype * RangeLUT->columns;

            for (i = 0; i < RangeLUT->columns; i++)
            {
                if (distance <= (real32)RangeLUT->lookup[pos + i])
                {
                    Findex = i * FrequencyLUT->columns;
                    index = pos + i;
                    break;
                }
            }

            if (index < SOUND_OK)
            {
                for (i = 0; i < FrequencyLUT->columns; i++)
                {
                    eq[i] = DefaultEQ[i];
                }

                return (0);
            }

			mult = ((real32)RangeLUT->lookup[index] - distance) * RangeFloatLUT[index];
			vol = volSFX * ((real32)VolumeLUT->lookup[index] + (mult * VolumeFloatLUT[index]));
			
			if (vol < SFX_MIN_PERCEPTABLE_VOL)
			{
				vol = 0;
			}
			else
			{
				if (vol > SOUND_VOL_MAX)
				{
					vol = SOUND_VOL_MAX;
				}

				for (i = 0; i < FrequencyLUT->columns; i++)
				{
					eq[i] = FrequencyLUT->lookup[Findex + i] + (mult * FreqLUT[Findex + i]);
				}
			}
        }
    }

    return ((sword)vol);
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SEstopengine(Ship *ship, bool32 stopnow)
{
    if (ship == NULL)
        return;

    ship->soundevent.engineState = SOUND_NOTINITED;

    if (ship->soundevent.engineHandle > SOUND_NOTINITED)
    {
        if (stopnow)
        {
            soundstop(ship->soundevent.engineHandle, SOUND_FADE_STOPNOW);
        }
        else
        {
            soundstop(ship->soundevent.engineHandle, 5.0f);
        }
        ship->soundevent.engineHandle = SOUND_NOTINITED;
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SEstopsoundhandle(sdword *shandle, real32 fadetime)
{
    if (*shandle > SOUND_NOTINITED)
    {
        soundstop(*shandle, fadetime);
        *shandle = SOUND_NOTINITED;
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SEloadbank(void)
{
    char loadfile[100];

    soundEventResetStreamGlobals();
    battleChatterResetTimers();
    singlePlayerSetWideScreenSpeedScale(FALSE);
    Eegah = FALSE;
    seCataclysmEasterEggState = 0;
    PrinceOfSpace = FALSE;

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "GunEvents.lut");
    fileLoadAlloc(loadfile, &GunEventsLUT, NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "GunSpecEvents.lut");
    fileLoadAlloc(loadfile, &GunSpecEventsLUT, NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "ShipCmnEvents.lut");
    fileLoadAlloc(loadfile, &ShipCmnEventsLUT, NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "ShipEvents.lut");
    fileLoadAlloc(loadfile, &ShipEventsLUT, NonVolatile);
    if (ShipEventsLUT->checksum != ShipCmnEventsLUT->checksum)
    {
        dbgMessage("Lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "DerelictEvents.lut");
    fileLoadAlloc(loadfile, &DerelictEventsLUT, NonVolatile);
    if (DerelictEventsLUT->checksum != ShipCmnEventsLUT->checksum)
    {
        dbgMessage("Lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "SpecExpEvents.lut");
    fileLoadAlloc(loadfile, &SpecExpEventsLUT, NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "SpecHitEvents.lut");
    fileLoadAlloc(loadfile, &SpecHitEventsLUT, NonVolatile);
    if (SpecHitEventsLUT->checksum != SpecExpEventsLUT->checksum)
    {
        dbgMessage("Lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "UIEvents.lut");
    fileLoadAlloc(loadfile, &UIEventsLUT, NonVolatile);

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "Guns.bnk");
    fileLoadAlloc(loadfile, &GunBank, NonVolatile);
    if (soundbankadd(GunBank) != GunEventsLUT->checksum)
    {
        dbgMessage("Lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "Ships.bnk");
    fileLoadAlloc(loadfile, &ShipBank, NonVolatile);
    if (soundbankadd(ShipBank) != ShipCmnEventsLUT->checksum)
    {
        dbgMessage("Ship bank file does not match Lookup tables.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "SpecialEffects.bnk");
    fileLoadAlloc(loadfile, &SpecialEffectBank, NonVolatile);
    if (soundbankadd(SpecialEffectBank) != SpecExpEventsLUT->checksum)
    {
        dbgMessage("Lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    strcpy(loadfile, SOUNDFXDIR);
    strcat(loadfile, "UI.bnk");
    fileLoadAlloc(loadfile, &UIBank, NonVolatile);
    if (soundbankadd(UIBank) != UIEventsLUT->checksum)
    {
        dbgMessage("Lookup tables do not match.  Not from same generate.\n");
        dbgAssert(FALSE);
    }

    if (universe.numPlayers)
    {
        sdword i = 0;
        const char *v1 = playerNames[0];

        do
        {
            if (!_strnicmp(v1, "Eegah", MAX_PERSONAL_NAME_LEN))
            {
                Eegah = TRUE;
            }
            ++i;
            v1 += MAX_PERSONAL_NAME_LEN;
        }
        while (i < universe.numPlayers);
    }
    if (!_strnicmp(playerNames[universe.curPlayerIndex], "PrinceOfSpace", MAX_PERSONAL_NAME_LEN) && !whichRaceSelected)
    {
        PrinceOfSpace = TRUE;
    }
}


/*-----------------------------------------------------------------------------
    Name        :
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void soundEventPlayMusic(sdword tracknum)
{
    musicEventPlay(tracknum);

    return;
}


void soundEventStopMusic(real32 fadetime)
{
    musicEventStop(-1, fadetime);
    return;
}


void soundEventStopTrack(sdword tracknum, real32 fadetime)
{
    musicEventStop(tracknum, fadetime);
    return;
}


////**********  Panning Angle code  ************/////

/*-----------------------------------------------------------------------------
    Name        : getPanAngle
    Description : calculates the angle between the passed vector and the cameras
                lookat vector (projected onto the cameras 2D horizontal plane).

                            0|
                             |
                             |
                -90 --------------------90
                             |
                             |
                             |
                         -179/179

    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

sword getPanAngle(vector WorldVector, real32 objsize, real32 distance)
{
    vector CameraToPoint;
    vector CameraRightVec;
    vector CameraLookAt;
    vector ProjOnRight;
    vector ProjOnLookAt;
    vector Projection;
    real32 Prright;
    real32 Prlook;
    real32 dotprod;
    real32 angle;

    if (distance < (objsize * 2.0f))
	{
		return ((sword)SOUND_PAN_CENTER);
	}
	
	vecSub(CameraToPoint, WorldVector, mrCamera->eyeposition);
    vecSub(CameraLookAt, mrCamera->lookatpoint, mrCamera->eyeposition);

    //if NIS isn't playing...we know upvector is 0,0,1 so we can simplify cross product a grreat
    // deal
    vecCrossProduct(CameraRightVec,CameraLookAt,mrCamera->upvector);

    vecNormalize(&CameraLookAt);
    Prlook = vecDotProduct(CameraLookAt,CameraToPoint);
    vecScalarMultiply(ProjOnLookAt,CameraLookAt,Prlook);

    Prright = vecDotProduct(CameraRightVec,CameraToPoint)/vecMagnitudeSquared(CameraRightVec);
    vecScalarMultiply(ProjOnRight,CameraRightVec,Prright);

    vecAdd(Projection,ProjOnLookAt,ProjOnRight);

    vecNormalize(&Projection);
    dotprod = vecDotProduct(Projection,CameraLookAt);
    if(dotprod <-1.0f)
        dotprod = -1.0f;
    else if(dotprod > 1.0f)
        dotprod = 1.0f;
    angle = (float) acos(dotprod);

    if(Prright < 0.0f)
       angle = -angle;

    return((sword) (angle*PI_UNDER_180));
}


sword SEgetHeadingAngle(float *obj)
{
    vector velDir;
    vector camDir;

    camDir.x = mrCamera->eyeposition.x - obj[22];
    camDir.y = mrCamera->eyeposition.y - obj[23];
    camDir.z = mrCamera->eyeposition.z - obj[24];
    velDir.x = (obj[44] + obj[22]) - obj[22];
    velDir.y = (obj[45] + obj[23]) - obj[23];
    velDir.z = (obj[46] + obj[24]) - obj[24];
    vecNormalize(&camDir);
    vecNormalize(&velDir);
    return (sword)(acos(vecDotProduct(velDir, camDir)) * PI_UNDER_180 + 0.5f);
}


/*-----------------------------------------------------------------------------
    Name        : SEgetShipClass
    Description : Returns the sound class for a ship/gun, accounting for upgrades.
    Inputs      : ship
    Outputs     :
    Return      : sound class
----------------------------------------------------------------------------*/
sdword SEgetShipClass(Ship *ship)
{
    Gun *gun = (Gun *)ship;

    if (gun->weaponMode == gun->gunstatic->endUpgradeLevel &&
        gun->gunstatic->endUpgradeLevel != gun->gunstatic->startUpgradeLevel)
    {
        return gun->gunstatic->bulletType;
    }
    return gun->gunstatic->gunsoundtype;
}


/*-----------------------------------------------------------------------------
    Name        : SEinitShipClassLUT
    Description : Initialises the ship class lookup table with hardcoded event IDs.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SEinitShipClassLUT(void)
{
    memset(shipClassLUT, 0xFF, 0x5A0u);
    shipClassLUT[7] = 2075;
    shipClassLUT[205] = 2075;
    shipClassLUT[6] = 2074;
    shipClassLUT[8] = 1029;
    shipClassLUT[9] = 1030;
    shipClassLUT[204] = 2074;
    shipClassLUT[206] = 1029;
    shipClassLUT[207] = 1030;
    shipClassLUT[86] = 1030;
    shipClassLUT[87] = 1029;
    shipClassLUT[84] = 2078;
    shipClassLUT[85] = 2079;
    shipClassLUT[100] = 1030;
    shipClassLUT[101] = 1029;
    shipClassLUT[98] = 2078;
    shipClassLUT[99] = 2079;
    shipClassLUT[36] = 2076;
    shipClassLUT[18] = 2082;
    shipClassLUT[19] = 2083;
    shipClassLUT[48] = 1029;
    shipClassLUT[49] = 1030;
    shipClassLUT[252] = 1029;
    shipClassLUT[253] = 1030;
    shipClassLUT[12] = 1029;
    shipClassLUT[13] = 1030;
    shipClassLUT[72] = 1029;
    shipClassLUT[73] = 1030;
    shipClassLUT[102] = 1029;
    shipClassLUT[103] = 1030;
    shipClassLUT[116] = 1029;
    shipClassLUT[117] = 1030;
}


/*-----------------------------------------------------------------------------
    Name        : SEcheckEnemyShipPresence
    Description : Checks if a ship is an enemy and sets speech pending flags.
    Inputs      : ship, playerIndex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword SEcheckEnemyShipPresence(Ship *ship, sdword playerIndex)
{
    sdword allied;
    void *infectionData;

    if (playerIndex == universe.curPlayerIndex)
    {
        allied = allianceArePlayersAllied(ship->playerowner, &universe.players[playerIndex]);
        if (!allied)
        {
            switch (ship->shiptype)
            {
                case 2:
                case 5:
                case 97:
                    speechEnemyPresencePendingResourceInjectionReceived = 1;
                    break;
                case 3:
                    if (IsShipInfected(ship))
                    {
                        infectionData = GetShipInfectionData(ship);
                        if (((LeechSpecifics *)infectionData)->currentTarget)
                            speechEnemyPresencePendingUnderAttackFromAlly = 1;
                    }
                    break;
                case 36:
                case 44:
                case 94:
                    speechEnemyPresencePendingFleetStatus76 = 1;
                    break;
                default:
                    return ship->shiptype;
            }
        }
        return allied;
    }
    return playerIndex;
}


extern Camera *nisCamera;

#define SHOCKWAVE_SOUND_NIS_DISTANCE_SCALE 0.25f
#define SHOCKWAVE_SOUND_EQ_ROW             42
#define SHOCKWAVE_SOUND_SIZE               350.0f
#define SHOCKWAVE_SOUND_PRIORITY           50

sdword soundEventUpdateShockwaveLoop(Shockwave *shockwave)
{
    vector listenerPosition;
    vector offset;
    vector soundPosition;
    real32 tempEQ[9];
    real32 distance;
    sword panAngle;
    sword volume;
    sdword patch;

    if (nisIsRunning_ && nisCamera != NULL)
    {
        listenerPosition = nisCamera->eyeposition;
    }
    else
    {
        listenerPosition = universe.mainCameraCommand.actualcamera.eyeposition;
    }

    vecSub(offset, listenerPosition, shockwave->position);
    vecNormalize(&offset);
    vecScalarMultiply(offset, offset, shockwave->currentRadius);
    vecAdd(soundPosition, shockwave->position, offset);

    distance = GetDistance(&listenerPosition, &soundPosition);
    if (nisIsRunning_)
    {
        distance *= SHOCKWAVE_SOUND_NIS_DISTANCE_SCALE;
    }

    if (RangeLUT != NULL &&
        RangeLUT->rows > SHOCKWAVE_SOUND_EQ_ROW &&
        (real32)RangeLUT->lookup[SHOCKWAVE_SOUND_EQ_ROW * RangeLUT->columns] >= distance)
    {
        panAngle = getPanAngle(soundPosition, SHOCKWAVE_SOUND_SIZE, distance);
        volume = SEequalize(SHOCKWAVE_SOUND_EQ_ROW, distance, tempEQ);

        if (shockwave->soundHandle == SOUND_NOTINITED || soundover(shockwave->soundHandle))
        {
            if (ShipEventsLUT == NULL || ShipEventsLUT->numobjects < 0)
            {
                return SOUND_ERR;
            }

            patch = GetPatch(ShipEventsLUT, 0, shockwave->soundEvent);
            shockwave->soundHandle = splayFPRVL(
                ShipBank,
                ShipEventsLUT->lookup[patch],
                tempEQ,
                -1.0f,
                panAngle,
                SHOCKWAVE_SOUND_PRIORITY,
                volume,
                TRUE,
                TRUE,
                FALSE);
        }

        soundvolumeF(shockwave->soundHandle, volume, 0.0f);
        soundpanF(shockwave->soundHandle, panAngle, 0.0f);
        return soundequalize(shockwave->soundHandle, tempEQ);
    }

    if (shockwave->soundHandle != SOUND_NOTINITED)
    {
        soundEventStopSound(shockwave->soundHandle, 0);
        shockwave->soundHandle = SOUND_NOTINITED;
    }

    return SOUND_OK;
}
