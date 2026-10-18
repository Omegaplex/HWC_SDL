//
//  implementations of KAS functions
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shipselect.h"
#include "select.h"
#include "pieplate.h"
#include "AITeam.h"
#include "KASFunc.h"
#include "BigGun.h"
#include "AIPlayer.h"
#include "AIUtilities.h"
#include "vector.h"
#include "kas.h"
#include "Volume.h"
#include "Timer.h"
#include "CommandWrap.h"
#include "AIVar.h"
#include "AIMoves.h"
#include "FormationDefs.h"
#include "randy.h"

#include "SinglePlayer.h"
#include "font.h"
#include "render.h"
#include "prim2d.h"
#include "ping.h"
#include "Objectives.h"
#include "Sensors.h"
#include "univupdate.h"

#include "tutor.h"
#include "soundevent.h"
#include "soundeventdefs.h"
#include "speechevent.h"
#include "SalCapCorvette.h"
#include "LaunchMgr.h"

sdword shipGetSlotStatus(Ship *ship, sdword slot);

#include "hs.h"

#include "SaveGame.h"

#include "KEY.h"
#include "consMgr.h"
#include "dock.h"

extern sdword currentSensorCoverage;
extern sdword kasCurrentTeamPtr;
extern AITeam *CurrentTeamP;
extern ubyte byte_88F454;
unsigned int tutHideBackButtonRegion(void);

static AITeam *kasfScriptTeam(void)
{
    return (AITeam *)kasCurrentTeamPtr;
}

static sdword kasfSensorRangeValueAtPoint(real32 x, real32 y, real32 z)
{
    sdword gridX;
    sdword gridY;
    sdword gridZ;
    real32 invCellSize;

    if (currentSensorCoverage == 0)
    {
        return -524288;
    }

    invCellSize = 1.0f / *(real32 *)(currentSensorCoverage + 28);
    gridX = (sdword)((x - *(real32 *)(currentSensorCoverage + 16)) * invCellSize);
    gridY = (sdword)((y - *(real32 *)(currentSensorCoverage + 20)) * invCellSize);
    gridZ = (sdword)((z - *(real32 *)(currentSensorCoverage + 24)) * invCellSize);
    if (gridX < 0
        || gridX >= *(sdword *)(currentSensorCoverage + 4)
        || gridY < 0
        || gridY >= *(sdword *)(currentSensorCoverage + 8)
        || gridZ < 0
        || gridZ >= *(sdword *)(currentSensorCoverage + 12))
    {
        return -524288;
    }

    return *(sdword *)(32 * (gridX
        + *(sdword *)(currentSensorCoverage + 4) * (gridY + gridZ * *(sdword *)(currentSensorCoverage + 8)))
        + *(sdword *)currentSensorCoverage);
}
#include "collision.h"
#include "mainrgn.h"
#include "nis.h"
#include "texreg.h"
#include "sensors.h"
#include "Subtitle.h"
#include "btg.h"
#include "aitrack.h"
#include "utility.h"
#include "fastmath.h"
#include "animatic.h"
#include "TradeMgr.h"
#include "StringsOnly.h"
#include "taskbar.h"
#include "DDDFrigate.h"
#include "consmgr.h"
#include "universe.h"
#include "Ship.h"
void shipSetDelayedDeath(Ship *ship, real32 duration);
void shipClearDelayedDeath(Ship *ship);
void shipNameReserve(Ship *ship);
void shipNameRelease(Ship *ship);
udword shipSetDisabledState(Ship *ship, sdword disabled);
void ioUpdateShipTotals(void);
#include "researchapi.h"
#include "ResearchGUI.h"
#include "Options.h"


extern char SinglePlayerSavedGamesPath[];
extern sdword CurrentMissionSkillLevel;
bool32 kasfTutCameraFocusActive = FALSE;
sdword kasfWideScreenPreserveMainScreenLockout = 0;
sdword kasfWideScreenActive = 0;

extern vector smCameraLookatPoint;
extern ubyte turboTimeCompressionFactor;
void ccChangeAngleDeclination(CameraCommand *cameracommand, real32 angle, real32 declination);
void ccSetDistance(CameraCommand *cameracommand, real32 distance);
void ccSetDistanceIgnoreZoom(CameraCommand *cameracommand, real32 distance);
void ccAdjustDistanceTowardMinimumZoom(CameraCommand *cameracommand, real32 scale);
void cameraSetEyePosition(Camera *camera);
void dockPutShipInside(Ship *ship, Ship *dockwith);
void FancyFightPreLoad(sdword closeSensors);
void singlePlayerShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation);
sdword univGetShipTypeCount(sdword shipType, sdword playerIndex);
sdword univGetReplacementShipTypeCount(sdword shipType, sdword playerIndex);
sdword Watch_Tutorial1_Tutorial10_CameraMoveDescription(char *techName);

bool32 shipGetWaitingForShipsPoints(Ship *ship, vector *fromPoint, vector *toPoint);
void shipStartWaitingForShips(Ship *ship, vector *position);
bool32 shipWaitingForShipsDone(Ship *ship);
bool32 shipWaitingForShipsActive(Ship *ship);
bool32 shipIsBeastShip(Ship *ship);
real32 acvGetEMPChargeProgress(Ship *ship);
bool32 shipHasReadySwarmers(Ship *ship);
void *shipGetBeastWeaponInfo(Ship *ship);
sdword shipQueueBeastConversion(Ship *ship, Ship *attacker);
sdword shipGetModuleSlotTimer(Ship *ship);
void shipDecrementModuleSlotTimer(Ship *ship);
sdword shipGetModuleCount(Ship *ship);
sdword shipCanStartModuleSlotTimer(Ship *ship);
sdword shipTryStartModuleSlotTimer(Ship *ship);
extern sdword kasfMothershipBigGunDisabled;
//extern char *getWord(char *dest, char *source); // Found in researchgui.c

//
//  these are for the placeholder for a real fleet intelligence message window
//
extern fonthandle selGroupFont2;
#define POPUPTEXT_TITLE_COLOR      colRGB(255, 255, 255)
#define POPUPTEXT_COLOR            colRGB(200, 200, 0)
#define POPUPTEXT_BACKGROUND       colRGB(0, 0, 0)
#define POPUPTEXT_BORDER_COLOR     colRGB(100, 100, 100)
sdword popupTextNumLines = 0;
char popupTextLines[POPUPTEXT_MAX_LINES][POPUPTEXT_MAX_LINE_LENGTH+1];
real32 popupTextEndTime;
sdword popupTextWidth;

// this can help eliminate the need to pass the current team ptr to some functions
extern AITeam *CurrentTeamP;
extern AITeam *kasUnpausedTeam;

// run-time scoping for variables, timers, etc.
extern sdword CurrentMissionScope;  // what level we're currently executing at
extern char CurrentMissionScopeName[];

extern LabelledVector **LabelledVectors;
extern sdword LabelledVectorsUsed;


nisstatic stat1 = {0, 50, 25,  5,  3, 240, 173, 0.5f, 0.5f, 0.9f, 0.1f, 0.2f, 0.1f, 0.6f, 0.2f, TRUE, FALSE, 0};

//current sensors manager zoom length
extern real32 smInitialDistance;
//stores the current sensors manager zoom length
sdword kasStoreSMInitialDistance = 0;

extern real32 SUBMESSAGE_SAFETY_TIMEOUT;
extern real32 subMessageReturnedFalseTime;

//for fading to black at the end of the game
extern real32 nisBlackFadeDest;
extern real32 nisBlackFadeRate;

// handle of the last queued fleet speech event, used for linking
sdword linkto = -1;

typedef struct KasEntryTableEntry
{
    sdword values[3];
} KasEntryTableEntry;

static KasEntryTableEntry *kasfEntryTable = NULL;
static sdword kasfEntryTableEntries = 0;
static sdword kasfEntryTableCapacity = 0;
static sdword kasfEntryTableNextId = 0;

sdword kasfEntryTableInit(void)
{
    if (kasfEntryTable != NULL)
    {
        memFree(kasfEntryTable);
        kasfEntryTable = NULL;
    }

    kasfEntryTableEntries = 0;
    kasfEntryTableCapacity = 0;
    kasfEntryTableNextId = 0x06AAC470;
    return 0;
}

bool32 kasfShipMeetsCriteria(ShipPtr ship, sdword CriteriaFlag);

void kasfEntryTableSave(void)
{
    sdword i;

    SaveInfoNumber(kasfEntryTableEntries);
    SaveInfoNumber(kasfEntryTableCapacity);
    SaveInfoNumber(kasfEntryTableNextId);

    for (i = 0; i < kasfEntryTableEntries; i++)
    {
        SaveStructureOfSize(&kasfEntryTable[i], sizeof(KasEntryTableEntry));
    }
}

void kasfEntryTableLoad(void)
{
    sdword i;

    kasfEntryTableEntries = LoadInfoNumber();
    kasfEntryTableCapacity = LoadInfoNumber();
    kasfEntryTableNextId = LoadInfoNumber();

    if (kasfEntryTable != NULL)
    {
        memFree(kasfEntryTable);
        kasfEntryTable = NULL;
    }

    if (kasfEntryTableCapacity > 0)
    {
        dbgAssert(kasfEntryTableEntries <= kasfEntryTableCapacity);
        kasfEntryTable = memAlloc(sizeof(KasEntryTableEntry) * kasfEntryTableCapacity, "kasfEntryTable", 0);
        for (i = 0; i < kasfEntryTableEntries; i++)
        {
            LoadStructureOfSizeToAddress(&kasfEntryTable[i], sizeof(KasEntryTableEntry));
        }
    }
}

//
//  Cataclysm integer-hash entry store (reconstruction of cataclysm.exe @ 0x491Cxx).
//  Entries are keyed by (scope, id) and persisted via kasfEntryTableSave/Load.
//  values[0] = scope, values[1] = id, values[2] = value
//
static sdword kasfEntryResolveScope(sdword *pId)
{
    sdword id = *pId;
    if (id >= 0)
    {
        *pId = id;
        return (kasCurrentStep != 0) ? kasCurrentScopeHash : 0;
    }
    *pId = id & 0x7FFFFFFF;
    return 0;
}

static KasEntryTableEntry *kasfEntryFind(sdword scope, sdword id)
{
    sdword i;
    for (i = 0; i < kasfEntryTableEntries; i++)
    {
        if (kasfEntryTable[i].values[0] == scope && kasfEntryTable[i].values[1] == id)
            return &kasfEntryTable[i];
    }
    return NULL;
}

sdword *kasfEntryFindByScopeAndId(sdword scope, sdword id)
{
    KasEntryTableEntry *entry = kasfEntryFind(scope, id);
    return (entry != NULL) ? entry->values : NULL;
}

static KasEntryTableEntry *kasfEntryFindOrCreate(sdword scope, sdword id)
{
    KasEntryTableEntry *entry = kasfEntryFind(scope, id);
    if (entry != NULL)
        return entry;

    if (kasfEntryTableEntries >= kasfEntryTableCapacity)
    {
        kasfEntryTableCapacity += 16;
        kasfEntryTable = memRealloc(kasfEntryTable,
                                    sizeof(KasEntryTableEntry) * kasfEntryTableCapacity,
                                    "kasfEntryTable", 0);
    }
    entry = &kasfEntryTable[kasfEntryTableEntries++];
    entry->values[0] = scope;
    entry->values[1] = id;
    entry->values[2] = 0;
    return entry;
}

sdword *kasfEntryFindOrCreateByScopeAndId(sdword scope, sdword id)
{
    KasEntryTableEntry *entry = kasfEntryFindOrCreate(scope, id);
    return entry->values;
}

void kasfEntryGetOrCreate(sdword id)
{
    sdword scope = kasfEntryResolveScope(&id);
    kasfEntryFindOrCreate(scope, id);
}

void kasfEntryGetOrCreateSetValue(sdword id, sdword value)
{
    sdword scope = kasfEntryResolveScope(&id);
    KasEntryTableEntry *entry = kasfEntryFindOrCreate(scope, id);
    entry->values[2] = value;
}

void kasfEntrySetValue(sdword id, sdword value)
{
    sdword scope = kasfEntryResolveScope(&id);
    KasEntryTableEntry *entry = kasfEntryFind(scope, id);
    if (entry != NULL)
        entry->values[2] = value;
}

void kasfEntryIncrement(sdword id)
{
    sdword scope = kasfEntryResolveScope(&id);
    KasEntryTableEntry *entry = kasfEntryFind(scope, id);
    if (entry != NULL)
        ++entry->values[2];
}

void kasfEntryDecrement(sdword id)
{
    sdword scope = kasfEntryResolveScope(&id);
    KasEntryTableEntry *entry = kasfEntryFind(scope, id);
    if (entry != NULL)
        --entry->values[2];
}

sdword kasfEntryDelete(sdword *entry)
{
    sdword i;

    for (i = 0; i < kasfEntryTableEntries; i++)
    {
        if (kasfEntryTable[i].values == entry)
        {
            if (i < kasfEntryTableEntries - 1)
            {
                memmove(&kasfEntryTable[i],
                        &kasfEntryTable[i + 1],
                        sizeof(KasEntryTableEntry) * (kasfEntryTableEntries - i - 1));
            }
            kasfEntryTableEntries--;
            return i;
        }
    }
    return 0;
}

sdword kasfEntryValueGet(sdword id)
{
    sdword scope = kasfEntryResolveScope(&id);
    KasEntryTableEntry *entry = kasfEntryFind(scope, id);
    return (entry != NULL) ? entry->values[2] : 0;
}

unsigned int kasfEntryDestroyAll(void)
{
    unsigned int result = (unsigned int)kasfEntryTable;

    if (kasfEntryTable != NULL)
    {
        memFree(kasfEntryTable);
        kasfEntryTable = NULL;
        kasfEntryTableEntries = 0;
        kasfEntryTableCapacity = 0;
        kasfEntryTableNextId = 0;
    }

    return result;
}

//
//  Cataclysm integer-hash timer engine (reconstruction of cataclysm.exe @ 0x52Cxxx / 0x491Fxx).
//  Timers are keyed by (scope, id) and stored in the shared Timer pool (timers/timersUsed/timersAllocated
//  from Timer.c), which is saved and loaded by timTimerSave/timTimerLoad. This matches the original
//  binary where both string-named and integer-keyed timers share a single 24-byte-entry pool.
//
extern NamedTimer *timers;
extern sdword timersUsed;
extern sdword timersAllocated;

//  Reproduces the binary's double->int round-to-nearest trick (add 1.5*2^52, read low dword).
static sdword kasTimerRound(real32 value)
{
    union { double d; struct { udword lo, hi; } u; } conv;
    conv.d = (double)value + 6755399441055744.0;
    return (sdword)conv.u.lo;
}

static sdword kasTimerFindIndex(sdword scope, sdword id)
{
    sdword i;
    for (i = 0; i < timersUsed; i++)
    {
        if (timers[i].scopeId == scope && timers[i].timerId == id)
            return i;
    }
    return -1;
}

static NamedTimer *kasTimerCreateRecord(sdword scope, sdword id)
{
    NamedTimer *timer;
    sdword index = kasTimerFindIndex(scope, id);
    if (index >= 0)
        return &timers[index];

    if (timersUsed >= timersAllocated)
    {
        timersAllocated += TIMER_ALLOC_INCREMENT;
        timers = memRealloc(timers,
                            sizeof(NamedTimer) * timersAllocated,
                            "timers", 0);
    }
    timer = &timers[timersUsed++];
    timer->scopeId   = scope;
    timer->timerId   = id;
    timer->startTime = 0.0f;
    timer->duration  = 0.0f;
    timer->pauseTime = 0.0f;
    timer->enabled   = 0;
    timer->bPaused   = 0;
    return timer;
}

static void kasTimerStartRecord(NamedTimer *timer)
{
    if (timer->duration <= 0.0f)
    {
        timer->enabled = 0;
    }
    else if (!timer->enabled)
    {
        timer->startTime = universe.totaltimeelapsed;
        timer->enabled   = 1;
        timer->bPaused   = 0;
    }
}

void kasTimerCreate(sdword scope, sdword id)
{
    kasTimerCreateRecord(scope, id);
}

void kasTimerCreateSetStart(sdword scope, sdword id, sdword duration)
{
    NamedTimer *timer = kasTimerCreateRecord(scope, id);
    timer->duration = (real32)duration;
    kasTimerStartRecord(timer);
}

void kasTimerStart(sdword scope, sdword id)
{
    sdword index = kasTimerFindIndex(scope, id);
    if (index >= 0)
        kasTimerStartRecord(&timers[index]);
}

sdword kasTimerStop(sdword scope, sdword id)
{
    sdword result = 0;
    NamedTimer *timer;
    NamedTimer *scan;

    if (timersUsed > 0)
    {
        for (scan = timers; scan->scopeId != scope || scan->timerId != id; ++scan)
        {
            if (++result >= timersUsed)
                return result;
        }

        if (result < timersUsed)
        {
            timer = &timers[result];
            result = 0;
            for (scan = timers; scan->scopeId != scope || scan->timerId != id; ++scan)
            {
                if (++result >= timersUsed)
                {
                    timer->enabled = 0;
                    timer->duration = 0.0f;
                    return result;
                }
            }

            if (scan->bPaused)
            {
                timer->enabled = 0;
                timer->duration = (real32)0x7FFFFFFF;
            }
            else if (scan->enabled)
            {
                sdword remaining = kasTimerRound(scan->duration - (universe.totaltimeelapsed - scan->startTime));
                if (remaining <= 0)
                {
                    scan->enabled = 0;
                    scan->duration = 0.0f;
                    remaining = 0;
                }
                timer->enabled = 0;
                timer->duration = (real32)remaining;
            }
            else
            {
                result = kasTimerRound(scan->duration);
                timer->enabled = 0;
                timer->duration = (real32)result;
            }
        }
    }

    return result;
}

void kasTimerSet(sdword scope, sdword id, sdword duration)
{
    sdword index = kasTimerFindIndex(scope, id);
    if (index >= 0)
        timers[index].duration = (real32)duration;
}

sdword kasTimerRemaining(sdword scope, sdword id)
{
    NamedTimer *timer;
    sdword remaining;
    sdword index = kasTimerFindIndex(scope, id);
    if (index < 0)
        return 0;
    timer = &timers[index];
    if (timer->bPaused)
        return 0x7FFFFFFF;
    if (!timer->enabled)
        return kasTimerRound(timer->duration);
    remaining = kasTimerRound(timer->duration - (universe.totaltimeelapsed - timer->startTime));
    if (remaining <= 0)
    {
        timer->enabled  = 0;
        timer->duration = 0.0f;
        return 0;
    }
    return remaining;
}

//  Non-removing expiry check.
sdword kasTimerExpiredDestroyCheck(sdword scope, sdword id)
{
    NamedTimer *timer;
    sdword remaining;
    sdword index = kasTimerFindIndex(scope, id);
    if (index < 0)
        return 0;
    timer = &timers[index];
    if (timer->bPaused)
        return 0;
    if (!timer->enabled)
        return kasTimerRound(timer->duration) == 0;
    remaining = kasTimerRound(timer->duration - (universe.totaltimeelapsed - timer->startTime));
    if (remaining <= 0)
    {
        timer->enabled  = 0;
        timer->duration = 0.0f;
        remaining = 0;
    }
    return remaining == 0;
}

static void kasTimerRemoveAt(sdword index)
{
    sdword i;
    for (i = index; i < timersUsed - 1; i++)
        timers[i] = timers[i + 1];
    if (timersUsed > 0)
        timersUsed--;
}

//  Expiry check that removes the timer once it has expired.
sdword kasTimerExpiredDestroy(sdword scope, sdword id)
{
    NamedTimer *timer;
    sdword expired;
    sdword index = kasTimerFindIndex(scope, id);
    if (index < 0)
        return 0;
    timer = &timers[index];
    if (timer->bPaused)
        return 0;
    if (timer->enabled)
    {
        sdword remaining = kasTimerRound(timer->duration - (universe.totaltimeelapsed - timer->startTime));
        if (remaining > 0)
            return 0;
        timer->enabled  = 0;
        timer->duration = 0.0f;
        expired = 1;
    }
    else
    {
        expired = (kasTimerRound(timer->duration) == 0);
    }
    if (!expired)
        return 0;
    kasTimerRemoveAt(index);
    return 1;
}

void kasTimerDestroy(sdword scope, sdword id)
{
    sdword index = kasTimerFindIndex(scope, id);
    if (index >= 0)
        kasTimerRemoveAt(index);
}

unsigned int kasLabelledTimerDestroyAll(void)
{
    if (timers != NULL)
    {
        memFree(timers);
        timers = NULL;
    }
    timersAllocated = 0;
    timersUsed = 0;
    return 0;
}

void kasTimerPauseAllNotScoped(sdword scope)
{
    sdword i;

    for (i = 0; i < timersUsed; i++)
    {
        if (timers[i].scopeId != scope)
        {
            timers[i].pauseTime = universe.totaltimeelapsed;
            timers[i].bPaused = 1;
        }
    }
}

//
//  ById wrappers: negative ids map to global scope (0); non-negative ids use the current scope.
//
static sdword kasTimerScopeForId(sdword *pId)
{
    sdword id = *pId;
    if (id >= 0)
    {
        *pId = id;
        return (kasCurrentStep != 0) ? kasCurrentScopeHash : 0;
    }
    *pId = id & 0x7FFFFFFF;
    return 0;
}

void kasTimerCreateById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    kasTimerCreate(scope, id);
}

void kasTimerCreateSetStartById(sdword id, sdword duration)
{
    sdword scope = kasTimerScopeForId(&id);
    kasTimerCreateSetStart(scope, id, duration);
}

sdword kasTimerStopById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    return kasTimerStop(scope, id);
}

void kasTimerSetById(sdword id, sdword duration)
{
    sdword scope = kasTimerScopeForId(&id);
    kasTimerSet(scope, id, duration);
}

void kasTimerStartById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    kasTimerStart(scope, id);
}

sdword kasTimerRemainingById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    return kasTimerRemaining(scope, id);
}

sdword kasTimerExpiredDestroyById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    return kasTimerExpiredDestroyCheck(scope, id);
}

sdword kasTimerExpiredById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    return kasTimerExpiredDestroy(scope, id);
}

void kasTimerDestroyById(sdword id)
{
    sdword scope = kasTimerScopeForId(&id);
    kasTimerDestroy(scope, id);
}

static void kasTimerUnpauseAll(void)
{
    sdword i;

    for (i = 0; i < timersUsed; i++)
    {
        NamedTimer *timer = &timers[i];
        if (timer->bPaused)
        {
            timer->bPaused = FALSE;
            timer->duration += universe.totaltimeelapsed - timer->pauseTime;
        }
    }
}

//
//  Integer-labelled KAS vector store.  Cataclysm references mission vectors by
//  hashed integer label; entries persist for the mission's lifetime.
//
typedef struct KasIntVector
{
    sdword label;
    hvector value;
} KasIntVector;

static KasIntVector *kasfIntVectors = NULL;
static sdword kasfIntVectorCount = 0;
static sdword kasfIntVectorCapacity = 0;

hvector *kasfIntVectorPtrIfExists(sdword label)
{
    sdword i;
    for (i = 0; i < kasfIntVectorCount; ++i)
    {
        if (kasfIntVectors[i].label == label)
            return &kasfIntVectors[i].value;
    }
    return NULL;
}

static hvector *kasfIntVectorAdd(sdword label, real32 x, real32 y, real32 z, real32 w)
{
    KasIntVector *entry;

    if (kasfIntVectors == NULL)
    {
        kasfIntVectorCapacity = 16;
        kasfIntVectorCount = 0;
        kasfIntVectors = memAlloc(kasfIntVectorCapacity * sizeof(KasIntVector), "kasIntVec", MBF_Volatile);
    }
    else if (kasfIntVectorCount >= kasfIntVectorCapacity)
    {
        kasfIntVectors = memRealloc(kasfIntVectors,
                                    (kasfIntVectorCapacity + 16) * sizeof(KasIntVector),
                                    "kasIntVec", MBF_Volatile);
        kasfIntVectorCapacity += 16;
    }

    entry = &kasfIntVectors[kasfIntVectorCount++];
    entry->label = label;
    entry->value.x = x;
    entry->value.y = y;
    entry->value.z = z;
    entry->value.w = w;
    return &entry->value;
}

//
//  Camera helpers (Cataclysm KAS).  The "Set" functions update the active
//  camera stack entry's angle/declination/distance and refresh the eye
//  position so the change takes effect immediately.
//
void kasfCameraSetAngleDeg(sdword degrees)
{
    real32 angleRad = (real32)DEG_TO_RAD((real32)degrees);
    real32 declRad = currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.declination;
    ccChangeAngleDeclination(&universe.mainCameraCommand, angleRad, declRad);
    cameraSetEyePosition(&universe.mainCameraCommand.actualcamera);
}

void kasfCameraSetDeclinationDeg(sdword degrees)
{
    real32 angleRad = currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.angle;
    real32 declRad = (real32)DEG_TO_RAD((real32)degrees);
    ccChangeAngleDeclination(&universe.mainCameraCommand, angleRad, declRad);
    cameraSetEyePosition(&universe.mainCameraCommand.actualcamera);
}

void kasfCameraSetDistance(sdword distance)
{
    universe.mainCameraCommand.actualcamera.distance = (real32)distance;
    currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.distance = (real32)distance;
    cameraSetEyePosition(&universe.mainCameraCommand.actualcamera);
}

void kasfTutCameraSetDistanceIgnoreZoom(sdword distance)
{
    ccSetDistanceIgnoreZoom(&universe.mainCameraCommand, (real32)distance);
}

void kasfTutCameraFocusClose(GrowSelection *ships)
{
    MaxSelection tempselected;

    if (ships->selection->numShips <= 0)
    {
        tempselected = selSelected;
        ccFocusCloseIgnoreZoom(&universe.mainCameraCommand, (SelectCommand *)&tempselected);
    }
    else
    {
        kasfTutCameraFocusActive = TRUE;
        if (piePointSpecMode)
        {
            piePointModeOnOff();
        }
        FancyFightPreLoad(TRUE);
        ccFocusCloseIgnoreZoom(&universe.mainCameraCommand, ships->selection);
    }
}

//
//  Select all ships in source for which predicate() returns TRUE.  When result
//  and source are the same selection the filtering is done in place.
//
sdword kasfShipsSelectByPredicate(GrowSelection *result, GrowSelection *source,
                                  sdword (*predicate)(Ship *ship, sdword arg1, sdword arg2),
                                  sdword arg1, sdword arg2)
{
    sdword i;
    sdword inPlace;
    Ship *ship;

    if (result == NULL || result->selection == NULL ||
        source == NULL || source->selection == NULL)
        return 0;

    if (result == source)
    {
        inPlace = 1;
    }
    else
    {
        inPlace = 0;
        kasGrowSelectionClear(result);
    }

    i = 0;
    while (i < source->selection->numShips)
    {
        ship = source->selection->ShipPtr[i];
        if (predicate(ship, arg1, arg2))
        {
            if (!inPlace)
                growSelectAddShip(result, ship);
            ++i;
        }
        else if (inPlace)
        {
            //  removing shifts the next ship into slot i, so do not advance
            clRemoveTargetFromSelection((SelectAnyCommand *)source->selection, (TargetPtr)ship);
        }
        else
        {
            ++i;
        }
    }

    return result->selection->numShips;
}

static udword kasSelectionPredicateIndex = 0;

static sdword kasShipIsAttackingSelection(Ship *ship, sdword targetSelectionArg, sdword arg2)
{
    GrowSelection *targetSelection = (GrowSelection *)targetSelectionArg;
    AttackCommand *attackSelection;
    sdword i;
    sdword j;

    (void)arg2;

    if (ship == NULL || ship->command == NULL ||
        targetSelection == NULL || targetSelection->selection == NULL)
    {
        return FALSE;
    }

    attackSelection = ship->command->attack;
    if (attackSelection == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < targetSelection->selection->numShips; i++)
    {
        Ship *targetShip = targetSelection->selection->ShipPtr[i];
        for (j = 0; j < attackSelection->numTargets; j++)
        {
            if (attackSelection->TargetPtr[j] == (TargetPtr)targetShip)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static sdword kasShipIsEnemyPlayerOwned(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    return (ship->playerowner == NULL || ship->playerowner->playerIndex == 0);
}

static sdword kasShipIsFriendlyPlayerOwned(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    return (ship->playerowner != NULL && ship->playerowner->playerIndex != 0);
}

static sdword kasSelectionIndexIsBelowLimit(Ship *ship, sdword limit, sdword arg2)
{
    udword index = kasSelectionPredicateIndex++;

    (void)ship;
    (void)arg2;
    return index < (udword)limit;
}

static sdword kasShipIsMemberOfSelection(Ship *ship, sdword selectionArg, sdword arg2)
{
    GrowSelection *selection = (GrowSelection *)selectionArg;
    sdword i;

    (void)arg2;

    if (selection == NULL || selection->selection == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < selection->selection->numShips; i++)
    {
        if (selection->selection->ShipPtr[i] == ship)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static sdword kasShipIsInsideVolume(Ship *ship, sdword volumeArg, sdword arg2)
{
    (void)arg2;
    return volPointInside((Volume *)volumeArg, &ship->posinfo.position);
}

static sdword kasShipMatchesClass(Ship *ship, sdword shipClass, sdword arg2)
{
    (void)arg2;
    return isShipOfClass(ship, (ShipClass)shipClass);
}

static sdword kasShipMatchesType(Ship *ship, sdword shipType, sdword arg2)
{
    (void)arg2;
    return (ship->shiptype == (ShipType)shipType);
}

static sdword kasShipIsCapital(Ship *ship, sdword capitalFlag, sdword arg2)
{
    (void)arg2;
    if (capitalFlag)
    {
        return isCapitalShip(ship);
    }
    return !isCapitalShip(ship);
}

static sdword kasShipMatchesNotDockedFilter(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    return ((ship->flags & (SOF_Hide | SOF_Hyperspace)) != SOF_Hide);
}

static sdword kasShipHealthIsAtMostPercent(Ship *ship, sdword maxHealthPercent, sdword arg2)
{
    (void)arg2;
    return (ship->health <= maxHealthPercent * ship->staticinfo->maxhealth / 100);
}

static sdword kasShipHealthIsAtLeastPercent(Ship *ship, sdword minHealthPercent, sdword arg2)
{
    (void)arg2;
    return (ship->health >= minHealthPercent * ship->staticinfo->maxhealth / 100);
}

static sdword kasShipIsMoving(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    return (ship->posinfo.isMoving & ISMOVING_MOVING);
}

static sdword kasShipIsFartherThanDistanceFromPoint(Ship *ship, sdword pointArg, sdword rangeSquaredArg)
{
    hvector *point = (hvector *)pointArg;
    vector location;

    vecGrabVecFromHVec(location, *point);
    return aiuFindDistanceSquared(ship->posinfo.position, location) > (real32)rangeSquaredArg;
}

static sdword kasShipIsWithinDistanceOfPoint(Ship *ship, sdword pointArg, sdword rangeSquaredArg)
{
    hvector *point = (hvector *)pointArg;
    vector location;

    vecGrabVecFromHVec(location, *point);
    return aiuFindDistanceSquared(ship->posinfo.position, location) < (real32)rangeSquaredArg;
}

static sdword kasShipMatchesSpecialSelectionFlag(Ship *ship, sdword specialFlag, sdword arg2)
{
    (void)arg2;
    return kasfShipMeetsCriteria(ship, specialFlag);
}

static sdword kasShipMatchesRace(Ship *ship, sdword race, sdword arg2)
{
    (void)arg2;
    return race == GetRaceForShipTypeRange(ship->shiptype);
}

static sdword kasShipClearSinglePlayerInParadeFlag(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    if ((ship->specialFlags2 & SPECIAL_2_SinglePlayerInParade) == 0)
    {
        return FALSE;
    }
    ship->specialFlags2 &= ~SPECIAL_2_SinglePlayerInParade;
    return TRUE;
}

static sdword kasShipClearRowGettingOutOfWayFlag(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    if ((ship->specialFlags2 & SPECIAL_2_RowGettingOutOfWay) == 0)
    {
        return FALSE;
    }
    ship->specialFlags2 &= ~SPECIAL_2_RowGettingOutOfWay;
    return TRUE;
}

static sdword kasShipFuelIsBelowPercent(Ship *ship, sdword percent, sdword arg2)
{
    (void)arg2;
    return (ship->fuel / ship->staticinfo->maxfuel) < ((real32)percent / 100.0f);
}

static void kasfClampVectorToUniverse(vector *point)
{
    if (point->x > smUniverseSizeX)  point->x = smUniverseSizeX;
    if (point->x < -smUniverseSizeX) point->x = -smUniverseSizeX;
    if (point->y > smUniverseSizeY)  point->y = smUniverseSizeY;
    if (point->y < -smUniverseSizeY) point->y = -smUniverseSizeY;
    if (point->z > smUniverseSizeZ)  point->z = smUniverseSizeZ;
    if (point->z < -smUniverseSizeZ) point->z = -smUniverseSizeZ;
}

sdword kasShipIsNotSalvager(Ship *ship, sdword arg1, sdword arg2)
{
    return (ship->specialFlags4 & SPECIAL_IsASalvager) == 0;
}

static sdword kasShipIsAttachedToSelectedShip(Ship *ship, sdword targetShipValue, sdword arg2)
{
    Ship *targetShip = (Ship *)targetShipValue;
    (void)arg2;

    return bitTest(ship->specialFlags4, SPECIAL_IsASalvager) && shipDockingTarget(ship) == targetShip;
}

sdword kasfShipsSelectNotSalvagers(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsNotSalvager, 0, 0);
}

static sdword kasShipCanSpecialTargetSelection(Ship *ship, sdword targetsValue, sdword arg2)
{
    GrowSelection *targets = (GrowSelection *)targetsValue;
    sdword i;
    (void)arg2;

    if (targets == NULL || targets->selection == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < targets->selection->numShips; i++)
    {
        Ship *targetShip = targets->selection->ShipPtr[i];

        if (shipCanSpecialTarget(targetShip) && shipCanBeSpecialTargeted(ship))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static sdword kasShipIsResourcingOrHyperspacing(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg1;
    (void)arg2;
    return (ship->flags & 0x2008000) != 0;
}

sdword kasfShipsSelectCanSpecialTargetSelection(GrowSelection *result, GrowSelection *source, GrowSelection *targets)
{
    return kasfShipsSelectByPredicate(result, source, kasShipCanSpecialTargetSelection, (sdword)targets, 0);
}

sdword kasfShipsSelectResourcingOrHyperspacing(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsResourcingOrHyperspacing, 0, 0);
}

sdword kasfShipsSelectAttackingSelection(GrowSelection *result, GrowSelection *source, GrowSelection *targets)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsAttackingSelection, (sdword)targets, 0);
}

sdword kasfShipsSelectInsideVolume(GrowSelection *result, GrowSelection *source, Volume *volume)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsInsideVolume, (sdword)volume, 0);
}

sdword kasfShipsSelectFartherThanDistanceFromPoint(GrowSelection *result, GrowSelection *source, hvector *location, sdword distance)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsFartherThanDistanceFromPoint,
                                      (sdword)location, distance * distance);
}

sdword kasfShipsSelectRace(GrowSelection *result, GrowSelection *source, sdword race)
{
    return kasfShipsSelectByPredicate(result, source, kasShipMatchesRace, race, 0);
}

sdword kasfShipsSelectFuelBelowPercent(GrowSelection *result, GrowSelection *source, sdword percent)
{
    return kasfShipsSelectByPredicate(result, source, kasShipFuelIsBelowPercent, percent, 0);
}

static sdword kasShipIsSinglePlayerCombatShip(Ship *ship, sdword arg1, sdword arg2)
{
    (void)arg2;

    if (!arg1)
    {
        return !ship->playerowner->playerIndex &&
               (ship->specialFlags4 & 0x2000000) == 0 &&
               ship->shiptype != sSwarmer;
    }

    if (!ship->playerowner->playerIndex || ship->shiptype == sSwarmer)
    {
        return FALSE;
    }

    return ship->shiptype != dCrystal0 && ship->shiptype != dCrystal1;
}

sdword kasfShipsSelectSinglePlayerCombatShips(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsSinglePlayerCombatShip, 0, 0);
}

sdword kasfShipsSelectSinglePlayerInParade(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipClearSinglePlayerInParadeFlag, 0, 0);
}

sdword kasfShipsSelectRowGettingOutOfWay(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipClearRowGettingOutOfWayFlag, 0, 0);
}

void kasfPingAddShipsIndexed(GrowSelection *ships, const char *label)
{
    SelectCommand pingSelection;
    sdword i;
    char pingLabel[256];

    if (ships == NULL || ships->selection == NULL || ships->selection->numShips <= 0)
    {
        return;
    }

    pingSelection.numShips = 1;
    pingSelection.timeLastStatus = 0.0f;
    for (i = 0; i < ships->selection->numShips; i++)
    {
        sprintf(pingLabel, "%s%d", label, i);
        pingSelection.ShipPtr[0] = ships->selection->ShipPtr[i];
        pingAnomalySelectionPingAdd(pingLabel, &pingSelection);
    }
}

GrowSelection *kasfShipsSpecialToggle(GrowSelection *ships)
{
    if (ships != NULL && ships->selection != NULL && ships->selection->numShips > 0)
    {
        clWrapSpecial(&universe.mainCommandLayer, ships->selection, NULL);
    }
    return ships;
}

sdword kasfEnableShip(char *shipTypeName, char *shipRaceName, sdword bEnabled)
{
    rmEnableShip(StrToShipRace(shipRaceName), StrToShipType(shipTypeName), bEnabled);
    if (cmActive)
    {
        cmUpdateShipsAvailable();
    }
    return cmActive;
}

void kasfShipsSetPosition(GrowSelection *ships, sdword x, sdword y, sdword z)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        ships->selection->ShipPtr[i]->posinfo.position.x = (real32)x;
        ships->selection->ShipPtr[i]->posinfo.position.y = (real32)y;
        ships->selection->ShipPtr[i]->posinfo.position.z = (real32)z;
    }
}

void kasfShipsSetPositionFromVector(GrowSelection *ships, hvector *position)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL || position == NULL)
    {
        return;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        ships->selection->ShipPtr[i]->posinfo.position.x = position->x;
        ships->selection->ShipPtr[i]->posinfo.position.y = position->y;
        ships->selection->ShipPtr[i]->posinfo.position.z = position->z;
    }
}

void kasfShipsSetPositionNearVector(GrowSelection *ships, hvector *position, sdword distance)
{
    vector center;
    vector origin = {0.0f, 0.0f, 0.0f};
    vector point;
    sdword i;

    if (ships == NULL || ships->selection == NULL || position == NULL)
    {
        return;
    }

    vecGrabVecFromHVec(center, *position);
    point = aiuGenerateRandomStandoffPoint(center, (real32)distance, origin, 0);
    kasfClampVectorToUniverse(&point);

    for (i = 0; i < ships->selection->numShips; i++)
    {
        ships->selection->ShipPtr[i]->posinfo.position = point;
    }
}

hvector *kasfVectorCrossNormalize(hvector *result, hvector *lhs, hvector *rhs)
{
    vector cross;

    cross.x = rhs->z * lhs->y - rhs->y * lhs->z;
    cross.y = rhs->x * lhs->z - rhs->z * lhs->x;
    cross.z = rhs->y * lhs->x - rhs->x * lhs->y;
    vecNormalize(&cross);

    result->x = cross.x;
    result->y = cross.y;
    result->z = cross.z;
    result->w = 1.0f;
    return result;
}

sdword kasfVectorDotProductScaled100(hvector *lhs, hvector *rhs)
{
    return (sdword)floor((lhs->x * rhs->x + lhs->y * rhs->y + lhs->z * rhs->z) * 100.0 + 0.5);
}

void kasfVectorBlendClampToUniverse(hvector *result, hvector *from, hvector *to, sdword numerator, sdword denominator)
{
    real32 fromScale = (real32)(denominator - numerator) / (real32)denominator;
    real32 toScale = (real32)numerator / (real32)denominator;
    vector point;

    point.x = fromScale * from->x + toScale * to->x;
    point.y = fromScale * from->y + toScale * to->y;
    point.z = fromScale * from->z + toScale * to->z;
    kasfClampVectorToUniverse(&point);

    result->x = point.x;
    result->y = point.y;
    result->z = point.z;
}

void kasfVectorAddClampToUniverse(hvector *result, hvector *delta)
{
    vector point;

    point.x = result->x + delta->x;
    point.y = result->y + delta->y;
    point.z = result->z + delta->z;
    kasfClampVectorToUniverse(&point);

    result->x = point.x;
    result->y = point.y;
    result->z = point.z;
}

void kasfVectorSubtractClampToUniverse(hvector *result, hvector *delta)
{
    vector point;

    point.x = result->x - delta->x;
    point.y = result->y - delta->y;
    point.z = result->z - delta->z;
    kasfClampVectorToUniverse(&point);

    result->x = point.x;
    result->y = point.y;
    result->z = point.z;
}

hvector *kasfVectorScaleByRatio(hvector *vectorToScale, sdword numerator, sdword denominator)
{
    real32 scale = (real32)numerator / (real32)denominator;

    vectorToScale->x *= scale;
    vectorToScale->y *= scale;
    vectorToScale->z *= scale;
    return vectorToScale;
}

sdword kasfShipsAnyDying(GrowSelection *ships)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        if (univObjIsDying(ships->selection->ShipPtr[i]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

sdword kasfShipsHealthAverage(GrowSelection *ships)
{
    real32 totalMaxHealth = 0.0f;
    real32 totalHealth = 0.0f;
    sdword i;

    if (ships == NULL || ships->selection == NULL || ships->selection->numShips <= 0)
    {
        return 0;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        Ship *ship = ships->selection->ShipPtr[i];
        totalMaxHealth += ship->staticinfo->maxhealth;
        totalHealth += ship->health;
    }

    return (sdword)(totalHealth * 100.0f / totalMaxHealth);
}

GrowSelection *kasfShipsShuffle(GrowSelection *ships)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return ships;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        sdword swapIndex = ranRandom(RAN_AIPlayer) % ships->selection->numShips;
        Ship *ship = ships->selection->ShipPtr[i];

        ships->selection->ShipPtr[i] = ships->selection->ShipPtr[swapIndex];
        ships->selection->ShipPtr[swapIndex] = ship;
    }

    return ships;
}

sdword kasfShipsSelectFirst(GrowSelection *newShips, GrowSelection *originalShips)
{
    kasGrowSelectionClear(newShips);
    if (originalShips != NULL &&
        originalShips->selection != NULL &&
        originalShips->selection->numShips > 0)
    {
        growSelectAddShip(newShips, originalShips->selection->ShipPtr[0]);
    }
    return newShips->selection->numShips;
}

sdword kasfShipsSelectAttachedToSelection(GrowSelection *newShips, GrowSelection *originalShips, GrowSelection *attachedTo)
{
    Ship *targetShip;

    if (newShips == NULL || originalShips == NULL || attachedTo == NULL ||
        attachedTo->selection == NULL || attachedTo->selection->numShips <= 0)
    {
        if (newShips != NULL)
        {
            kasGrowSelectionClear(newShips);
        }
        return 0;
    }

    targetShip = attachedTo->selection->ShipPtr[0];
    if (SalCapGetFirstAttachedShip(targetShip) == NULL)
    {
        kasGrowSelectionClear(newShips);
        return 0;
    }

    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsAttachedToSelectedShip, (sdword)targetShip, 0);
}

sdword kasfFindMissilesNearTeam(GrowSelection *ships, sdword radius)
{
    Ship *medianShip;
    Node *node;
    real32 rangeSquared = (real32)radius * (real32)radius;

    if (ships == NULL || ships->selection == NULL || CurrentTeamP == NULL || CurrentTeamP->shipList.selection->numShips <= 0)
    {
        return 0;
    }

    ships->selection->numShips = 0;
    medianShip = CurrentTeamP->shipList.selection->ShipPtr[0];
    for (node = universe.MissileList.head; node != NULL; node = node->next)
    {
        Missile *missile = listGetStructOfNode(node);

        if (missile->objtype == OBJ_MissileType &&
            missile->missileType == MISSILE_Mine &&
            aiuFindDistanceSquared(medianShip->posinfo.position, missile->posinfo.position) <= rangeSquared)
        {
            growSelectAddShip(ships, (Ship *)missile);
        }
    }

    return ships->selection->numShips;
}

GrowSelection *kasfShipsSetNotBumpable(GrowSelection *ships, bool32 enable)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return ships;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        Ship *ship = ships->selection->ShipPtr[i];
        ship->specialFlags2 =
            ((enable & 1) << 21) | (ship->specialFlags2 & ~SPECIAL_2_NotBumpable);
    }

    return ships;
}

sdword kasfShipDelayedDeathPending(GrowSelection *ships)
{
    if (ships != NULL && ships->selection != NULL && ships->selection->numShips > 0)
    {
        return ships->selection->ShipPtr[0]->deathtime != 0.0f;
    }
    return FALSE;
}

sdword kasfTutSetSelectedShips(GrowSelection *ships)
{
    if (ships == NULL || ships->selection == NULL)
    {
        return 0;
    }
    return tutSetSelectedShips((int *)ships->selection);
}

//
//  Select every ship linked to a ship in source.  Each source ship carries a
//  list of linked-ship associations; each node references the linked Ship.
//
sdword kasfShipsSelectLinkedShips(GrowSelection *result, GrowSelection *source)
{
    sdword i;
    Ship *ship;
    udword *listHead;
    udword *node;

    kasGrowSelectionClear(result);

    if (source == NULL || source->selection == NULL)
        return result->selection->numShips;

    for (i = 0; i < source->selection->numShips; ++i)
    {
        ship = source->selection->ShipPtr[i];
        listHead = (udword *)ship->shipsInsideMe;
        if (listHead)
        {
            node = (udword *)listHead[0];
            while (node)
            {
                growSelectAddShipNoDuplication(result, (Ship *)*(udword *)(node[3] + 16));
                node = (udword *)node[0];
            }
        }
    }

    return result->selection->numShips;
}

sdword kasfFindCloakedShips(GrowSelection *ships)
{
    Node *node;
    SpaceObj *obj;

    kasGrowSelectionClear(ships);

    node = universe.SpaceObjList.head;
    while (node)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        if (obj->objtype == OBJ_ShipType && (obj->flags & SOF_Cloaked) != 0)
            growSelectAddShip(ships, (Ship *)obj);
        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  Retail Cataclysm toggles Ship+0x18 BIT2 (flags2 0x4), despite the legacy name.
//
void kasfShipsSetDontApplyPhysicsFlag(GrowSelection *ships, sdword set)
{
    sdword i;
    Ship *ship;

    if (ships == NULL || ships->selection == NULL)
        return;

    for (i = 0; i < ships->selection->numShips; ++i)
    {
        ship = ships->selection->ShipPtr[i];
        if (set)
            bitSet(ship->flags2, SOF2_Unknown4);
        else
            bitClear(ship->flags2, SOF2_Unknown4);
    }
}

//
//  Set the value of an integer-labelled KAS vector (creating it if needed).
//
void kasfLabelledVectorSet(sdword label, sdword x, sdword y, sdword z)
{
    hvector *vec = kasfIntVectorPtrIfExists(label);
    if (vec)
    {
        vec->x = (real32)x;
        vec->y = (real32)y;
        vec->z = (real32)z;
        vec->w = 1.0f;
    }
    else
    {
        kasfIntVectorAdd(label, (real32)x, (real32)y, (real32)z, 1.0f);
    }
}

//
//  Pick a random point within radius of *center and store it in *out.  The
//  result is clamped to the universe bounds.
//
void kasfVectorRandomizeInRange(hvector *out, hvector *center, sdword radius)
{
    vector centerVec;
    vector origin = {0.0f, 0.0f, 0.0f};
    vector point;

    centerVec.x = center->x;
    centerVec.y = center->y;
    centerVec.z = center->z;

    point = aiuGenerateRandomStandoffPoint(centerVec, (real32)radius, origin, 0);

    if (point.x > smUniverseSizeX)  point.x = smUniverseSizeX;
    if (point.x < -smUniverseSizeX) point.x = -smUniverseSizeX;
    if (point.y > smUniverseSizeY)  point.y = smUniverseSizeY;
    if (point.y < -smUniverseSizeY) point.y = -smUniverseSizeY;
    if (point.z > smUniverseSizeZ)  point.z = smUniverseSizeZ;
    if (point.z < -smUniverseSizeZ) point.z = -smUniverseSizeZ;

    out->x = point.x;
    out->y = point.y;
    out->z = point.z;
    out->w = 1.0f;
}

void kasfSensorsSetLookatPoint(hvector *point)
{
    if (smSensorsActive)
    {
        smCameraLookatPoint.x = point->x;
        smCameraLookatPoint.y = point->y;
        smCameraLookatPoint.z = point->z;
    }
}

typedef struct FogGridRuntime
{
    ubyte *data;
    sdword width;
    sdword height;
    sdword depth;
    real32 originX;
    real32 originY;
    real32 originZ;
    real32 cellSize;
} FogGridRuntime;

static sdword kasfFogGridValueAtPoint(FogGridRuntime *fogGrid, vector *position)
{
    sdword gridX;
    sdword gridY;
    sdword gridZ;
    real32 invCellSize;

    if (fogGrid == NULL)
    {
        return -524288;
    }

    invCellSize = 1.0f / fogGrid->cellSize;
    gridX = (sdword)((position->x - fogGrid->originX) * invCellSize);
    gridY = (sdword)((position->y - fogGrid->originY) * invCellSize);
    gridZ = (sdword)((position->z - fogGrid->originZ) * invCellSize);
    if (gridX < 0 || gridX >= fogGrid->width
        || gridY < 0 || gridY >= fogGrid->height
        || gridZ < 0 || gridZ >= fogGrid->depth)
    {
        return -524288;
    }

    return *(sdword *)(fogGrid->data
        + 32 * (gridX + fogGrid->width * (gridY + gridZ * fogGrid->height)));
}

sdword kasfShipsAnyInSensorRange(GrowSelection *ships)
{
    sdword sensorCoverage;
    sdword savedSensorCoverage;
    SelectCommand *selection;
    sdword i;

    if (ships == NULL || ships->selection == NULL || ships->selection->numShips <= 0)
    {
        return 0;
    }

    sensorCoverage = (sdword)universe.curPlayerPtr->fogGrid;
    if (sensorCoverage == 0)
    {
        return 0;
    }

    savedSensorCoverage = univSetCurrentSensorCoverage(sensorCoverage);
    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship;

        ship = selection->ShipPtr[i];
        if (kasfSensorRangeValueAtPoint(ship->posinfo.position.x, ship->posinfo.position.y, ship->posinfo.position.z) > 0
            && (ship->flags & SOF_DockMarker) == 0)
        {
            univSetCurrentSensorCoverage(savedSensorCoverage);
            return 1;
        }
    }

    univSetCurrentSensorCoverage(savedSensorCoverage);
    return 0;
}

//
//  Create count ships of the given type and dock them inside the first ship in
//  target.  Used to spawn reinforcements inside the player's mothership.
//
sdword kasfShipsCreateInside(GrowSelection *result, char *shipTypeStr, sdword count,
                             GrowSelection *target, sdword playerIndex)
{
    vector shippos;
    ShipType shiptype;
    ShipRace shiprace;
    Ship *newship;
    Ship *targetShip;
    sdword i;

    if (target == NULL || target->selection == NULL || target->selection->numShips <= 0)
        return 0;

    memset(&shippos, 0, sizeof(shippos));
    shiptype = (ShipType)StrToShipType(shipTypeStr);
    shiprace = GetRaceForShipTypeRange(shiptype);
    kasGrowSelectionClear(result);

    if (shiptype != (ShipType)-1 && count > 0)
    {
        for (i = 0; i < count; ++i)
        {
            newship = univCreateShip(shiptype, shiprace, &shippos,
                                     &universe.players[playerIndex], 0);
            if (newship)
                growSelectAddShip(result, newship);
        }
    }

    targetShip = target->selection->ShipPtr[0];
    if (playerIndex == 0 && result->selection->numShips > 0)
    {
        bool32 disabled = ((~targetShip->specialFlags4 >> 20) & 1) != 0;

        for (i = 0; i < result->selection->numShips; ++i)
        {
            newship = result->selection->ShipPtr[i];
            shipSetDisabledState(newship, disabled);
            clSetAttribute((sdword)newship, disabled, 1);
        }
    }

    for (i = 0; i < result->selection->numShips; ++i)
    {
        newship = result->selection->ShipPtr[i];
        univRemoveShipFromOutside(newship);
        dockPutShipInside(newship, targetShip);
    }

    if (targetShip->shiptype == sHiveFrigate || targetShip->shiptype == bHiveFrigate)
    {
        shipClearSwarmers(targetShip);
    }

    for (i = 0; i < result->selection->numShips; ++i)
    {
        newship = result->selection->ShipPtr[i];
        newship->specialFlags1C8 |= CATA_RUNTIME_LAUNCHED_FROM_KAS | CATA_RUNTIME_KAS_CHECK_DONE_LAUNCHING;
    }

    return aiuWrapDock(result->selection, DOCK_FOR_REFUEL, targetShip);
}

void kasfMissionCompleted(void)
{
    if (tutorial == TUTORIAL_ONLY)
    {
        //put the student out of it's misery
        tutorialdone = TRUE;
    }
    else
    {
        clCommandMessage("MISSION COMPLETED", -1);
        // play the end of Mission animatic
        // if you want to finish a level without hyperspacing, you put the stuff that happens here
        speechEventCleanup();
        singlePlayerMissionCompleteCB();
        animBinkPlay(singlePlayerGameInfo.currentMission, singlePlayerGameInfo.currentMission + 1);
    }
}

void kasfMissionFailed(void)
{
    //clCommandMessage("MISSION FAILED");
    singlePlayerMissionFailedCB();
}

void kasfFadeToWhite(void)
{
    soundEventStopSFX(3.0);
    nisBlackFadeDest = 1.0f;
    nisBlackFadeRate = 3.0f;
    nisTaskResume();
}

//
//  for scoping symbol names
//
static void kasfScopeName(char *scopedName, char *name)
{
    if (!name || strlen(name) < 2)
        scopedName[0] = 0;
    else if (toupper(name[0]) == 'G' && name[1] == '_')
        // user is requesting global scope explicitly
        strcpy(scopedName, name+2);
    else if (CurrentMissionScope != KAS_SCOPE_MISSION)
        // local scope
        sprintf(scopedName, "%s.%s", CurrentMissionScopeName, name);
    else
        // user is in global scope
        strcpy(scopedName, name);
}

void kasfVarCreate(char *name)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarCreate(scopedName);
}

void kasfVarCreateSet(char *name, sdword value)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarCreate(scopedName);
    aivarValueSet(aivarFind(scopedName), value);
}

void kasfVarValueSet(char *name, sdword value)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarValueSet(aivarFind(scopedName), value);
}

void kasfVarValueInc(char *name)
{
    AIVar *var;
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    var =  aivarFind(scopedName);
    aivarValueSet(var, aivarValueGet(var) + 1);
}

void kasfVarValueDec(char *name)
{
    AIVar *var;
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    var = aivarFind(scopedName);
    aivarValueSet(var, aivarValueGet(var) - 1);
}

sdword kasfVarValueGet(char *name)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return aivarValueGet(aivarFind(scopedName));
}

void kasfVarDestroy(char *name)
{
    char scopedName[AIVAR_LABEL_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    aivarDestroy(aivarFind(scopedName));
}

void kasfCommandMessage(char *message)
{
    clCommandMessage(message, -1);
}

void kasfTimerCreate(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerCreate(scopedName);
}

void kasfTimerSet(char *name, sdword duration)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerSet(scopedName, duration);
}

void kasfTimerStart(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerStart(scopedName);
}

void kasfTimerCreateSetStart(char *name, sdword duration)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerCreateSetStart(scopedName, duration);
}

void kasfTimerStop(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerStart(scopedName);
}

sdword kasfTimerRemaining(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return timTimerRemaining(scopedName);
}

sdword kasfTimerExpired(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return timTimerExpired(scopedName);
}

sdword kasfTimerExpiredDestroy(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    return timTimerExpiredDestroy(scopedName);
}

void kasfTimerDestroy(char *name)
{
    char scopedName[TIMER_NAME_MAX_LENGTH*2+2]; // allow for extra scope name -- this might be truncated
    kasfScopeName(scopedName, name);
    timTimerDestroy(scopedName);
}

sdword kasfMsgReceived(char *msg)
{
    // Cataclysm: messages are integer hashes stored in the current team's queue
    return aitQueueRemovePointer(CurrentTeamP, msg);
}

void kasfAttackMothership(void)
{
    MaxSelection filtered;

    if (!CurrentTeamP)
        return;

    filtered.numShips = 0;
    aiuFilterDisabledShips(CurrentTeamP->shipList.selection, &filtered);
    RemoveShipsFromDoingStuff(&universe.mainCommandLayer, (SelectCommand *)&filtered);
    spHyperspaceSelectionIn((SelectCommand *)&filtered);
    spHyperspaceDelay = 0.0f;
}

void kasfAttack(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
//*** Note changed to Advanced attack!
        aimCreateAdvancedAttack(CurrentTeamP, dupeTargets, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    }
}

// gives a set of ships the attack command
// this allows individual ships from a team to attack a target
void kasfShipsAttack(GrowSelection *targets, GrowSelection *attackers)
{
    clWrapAttack(&universe.mainCommandLayer, attackers->selection, (AttackCommand *)targets->selection);
}

void kasfCurrentTeamAttack(GrowSelection *targets)
{
    sdword i;
    MaxSelection attackcom;
    SelectCommand *dupeTargets;

    if (targets->selection->numShips <= COMMAND_MAX_SHIPS)
    {
        attackcom.numShips = 0;
        for (i = 0; i < targets->selection->numShips; i++)
        {
            selSelectionAddSingleShip(&attackcom, targets->selection->ShipPtr[i]);
        }
        aitDeleteAllTeamMoves(CurrentTeamP);
        clWrapAttack(&universe.mainCommandLayer, CurrentTeamP->shipList.selection, (AttackCommand *)&attackcom);
    }
    else
    {
        dupeTargets = selectDupSelection(targets->selection);
        aitDeleteAllTeamMoves(CurrentTeamP);
        clWrapAttack(&universe.mainCommandLayer, CurrentTeamP->shipList.selection, (AttackCommand *)dupeTargets);
        memFree((ubyte *)dupeTargets);
    }
}

void kasfAttackSpecial(GrowSelection *targets)
{
    MaxSelection filteredShips;
    MaxAnySelection specialTargets;
    sdword i;

    specialTargets.numTargets = 0;
    specialTargets.timeLastStatus = targets->selection->timeLastStatus;
    for (i = 0; i < targets->selection->numShips; i++)
    {
        specialTargets.TargetPtr[specialTargets.numTargets++] = (TargetPtr)targets->selection->ShipPtr[i];
    }

    if ((specialTargets.numTargets == 0) || (specialTargets.TargetPtr[0] == NULL))
    {
        return;
    }

    filteredShips.numShips = 0;
    filteredShips.timeLastStatus = CurrentTeamP->shipList.selection->timeLastStatus;
    for (i = 0; i < CurrentTeamP->shipList.selection->numShips; i++)
    {
        Ship *ship = CurrentTeamP->shipList.selection->ShipPtr[i];

        if (shipCanSpecialTarget(ship))
        {
            filteredShips.ShipPtr[filteredShips.numShips++] = ship;
        }
    }

    if ((filteredShips.numShips == 0) || (filteredShips.ShipPtr[0] == NULL))
    {
        return;
    }

    clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&filteredShips, (SpecialCommand *)&specialTargets);

    for (i = 0; i < filteredShips.numShips; i++)
    {
        Ship *ship = filteredShips.ShipPtr[i];

        if ((ship->shiptype == sWorker) || (ship->shiptype == bWorker))
        {
            ((ubyte *)ship->ShipSpecifics)[8] = 1;
        }
        else if (ship->shiptype == p1StandardCorvette)
        {
            ((ubyte *)ship->ShipSpecifics)[356] = 2;
        }
    }
}

void kasfMoveAttack(GrowSelection *targets)
{
    //alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateMoveAttack(CurrentTeamP, dupeTargets, TRUE, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    }
}


/*-----------------------------------------------------------------------------
    Name        : kasfBulgeAttack
    Description : Finds ships in the current team that are near the enemy ships,
                  and only uses those to attack the enemy.  This function is an
                  optimization for Mission 14 (though it could be used anywhere)
    Inputs      : targets      - the ships to attack,
                  bulgetargets - the ships that end up being attacked,
                  attackers    - the ships in the team that split off to attack
                  radius       - the maximum distance the team members have to be in
                                 order to attack
    Outputs     : Various ships in the current team get attack orders
    Return      : the number of ships chosen to take out
----------------------------------------------------------------------------*/
sdword kasfBulgeAttack(GrowSelection *targets, GrowSelection *bulgetargets,
                       GrowSelection *attackers, sdword radius)
{
    ShipPtr teamship;
    SelectCommand *teamships   = CurrentTeamP->shipList.selection,
                  *targetships = targets->selection,
                  onesel;
    MaxSelection tempsel;
    sdword i, j;
    sdword num_teamships = teamships->numShips,
           num_targets   = targetships->numShips;
    real32 radiussq = (real32)radius*(real32)radius, distsq;

    // initialize variables
    bulgetargets->selection->numShips = 0;
    attackers->selection->numShips    = 0;
    tempsel.numShips = 0;
    onesel.numShips  = 1;

    //go through each individual team ship
    for (i=0; i < num_teamships;i++)
    {
        teamship = teamships->ShipPtr[i];

        // only use ships that aren't attacking already
        if (!teamship->command ||
            (teamship->command->ordertype.order != COMMAND_ATTACK))
        {
            // find target ships within the alloted radius
            for (j=0;j < num_targets;j++)
            {
                distsq = aiuFindDistanceSquared(targets->selection->ShipPtr[j]->posinfo.position,
                                                teamship->posinfo.position);

                if (distsq < radiussq)
                {
                    selSelectionAddSingleShip(&tempsel, targets->selection->ShipPtr[j]);
                }
            }

            // if any targets close enough were found, attack them with this ship
            if (tempsel.numShips)
            {
                onesel.ShipPtr[0] = teamship;
                clWrapAttack(&universe.mainCommandLayer, &onesel, (AttackCommand *)&tempsel);

                // add the ship to the attackers grow selection
                growSelectAddShip(attackers, teamship);

                // add each new target to the bulgetargets grow selection
                for (j=0;j<tempsel.numShips;j++)
                {
                    growSelectAddShipNoDuplication(bulgetargets, tempsel.ShipPtr[j]);
                }
                tempsel.numShips = 0;
            }
        }
    }
    return bulgetargets->selection->numShips;
}


void kasfIntercept(GrowSelection *targets)
{
    //alloc memory that the move can later free itself
    if (targets->selection->numShips && targets->selection->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateIntercept(CurrentTeamP, targets->selection->ShipPtr[0], 2.0, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    }
}


// for salvage/capture ships
void kasfTargetDrop(void)
{
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    sdword i;

    for (i=0; i<selection->numShips; i++)
        SalCapDropTarget(selection->ShipPtr[i]);
}

void kasfAttackFlank(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateFlankAttack(CurrentTeamP, dupeTargets, FALSE, FALSE, TRUE);
    }
}

void kasfAttackHarass(void)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateHarassAttack(CurrentTeamP, FALSE, TRUE);
}

//sets the swarmers to target mode and adds ships to the target list
void kasfSetSwarmerTargets(GrowSelection *targets)
{
    aiumemFree(aiCurrentAIPlayer->Targets);
    aiCurrentAIPlayer->Targets = selectDupSelection(targets->selection);
    bitSet(aiCurrentAIPlayer->AlertStatus, ALERT_SWARMER_TARGETS);
}

void kasfSwarmMoveTo(GrowSelection *targets)
{
    MaxSelection shipsToGuard;
    MaxSelection ships;
    sdword i;
    real32 temp;
    vector destination;

    ships.numShips = 0;
    shipsToGuard.numShips = 0;

    for (i = 0; i < CurrentTeamP->shipList.selection->numShips; i++)
    {
        selSelectionAddSingleShip(&shipsToGuard, CurrentTeamP->shipList.selection->ShipPtr[i]);
    }

    destination = selCentrePointComputeGeneral((MaxSelection *)targets->selection, &temp);

    if (shipsToGuard.numShips)
    {
        if (ships.numShips)
        {
            aiuWrapProtect((SelectCommand *)&ships, (SelectCommand *)&shipsToGuard);
            aimIssueMoveCommand((SelectCommand *)&shipsToGuard, destination);
        }
        else
        {
            aimIssueMoveCommand((SelectCommand *)&shipsToGuard, destination);
        }
    }
    else if (ships.numShips)
    {
        aimIssueMoveCommand((SelectCommand *)&ships, destination);
    }
}

void kasfFormationDelta(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, DELTA_FORMATION);
    CurrentTeamP->kasFormation = DELTA_FORMATION;
}
void kasfFormationBroad(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, BROAD_FORMATION);
    CurrentTeamP->kasFormation = BROAD_FORMATION;
}
void kasfFormationDelta3D(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, DELTA3D_FORMATION);
    CurrentTeamP->kasFormation = DELTA3D_FORMATION;
}
void kasfFormationClaw(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, CLAW_FORMATION);
    CurrentTeamP->kasFormation = CLAW_FORMATION;
}
void kasfFormationWall(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, WALL_FORMATION);
    CurrentTeamP->kasFormation = WALL_FORMATION;
}
void kasfFormationSphere(void)
{
    aiuKasWrapFormation(CurrentTeamP->shipList.selection, SPHERE_FORMATION);
    CurrentTeamP->kasFormation = SPHERE_FORMATION;
}
void kasfFormationCustom(GrowSelection *ships)
{
    aiuKasWrapFormation(ships->selection, CUSTOM_FORMATION);
}

sdword kasfRandom(sdword lowestNum, sdword highestNum)
{
    if (lowestNum > highestNum)
    {
        sdword temp = lowestNum;
        lowestNum = highestNum;
        highestNum = temp;
    }
    return lowestNum + (ranRandom(RAN_AIPlayer) % (highestNum - lowestNum + 1));
}

void kasfGuardMothership(void)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDefendMothership(CurrentTeamP, FALSE, TRUE);
}

void kasfGuardShips(GrowSelection *ships)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeShips = selectDupSelection(ships->selection);
    if (dupeShips->numShips && dupeShips->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateGuardShips(CurrentTeamP, dupeShips, FALSE, TRUE);
    }
}

void kasfPatrolPath(Path *path)
{
    Path *dupePath = aiuPathDupe(path);
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreatePatrolMove(CurrentTeamP, dupePath, 0, SAME_FORMATION, CurrentTeamP->kasTactics, FALSE, TRUE);
}

void kasfPatrolActive(void)
{
    // an intelligent patrol that chooses its own path
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateActivePatrol(CurrentTeamP, AIT_SLOW_PATROL, FALSE, TRUE);
}

void kasfLog(char *string)
{
    aiplayerLog((0,"KAS: %s", string));
}

void kasfLogInteger(char *string, sdword integer)
{
    char tempstr[256];
    char tempstr2[256];

    // ensure not too long and that there is an integer field
    if (strlen(string) > 200 || !strstr(string, "%d"))
        return;

    sprintf(tempstr, "KAS: %s", string);
    sprintf(tempstr2, tempstr, integer);
    aiplayerLog((0, tempstr2));
}

#if 0       // this function is obsolete now
sdword kasfTeamMemberHandle(sdword Index)
{
    sdword numShips;

    numShips = CurrentTeamP->shipList.selection->numShips;
    if(Index >= numShips)
        return 0;

    return (sdword)(CurrentTeamP->shipList.selection->ShipPtr[Index]);
}
#endif

void kasfShipsSetClamped(GrowSelection *ships, bool32 clamped)
{
    sdword i;
    SelectCommand *selection;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; ++i)
    {
        Ship *ship = selection->ShipPtr[i];
        ShipModuleSlotRuntime *moduleRuntime = shipModuleSlotRuntime(ship);

        bitSetTo(ship->specialFlags3, SPECIAL3_Unknown27, clamped);
        moduleRuntime->slotAvailability[1] = 0;
    }
}

bool32 __cdecl shipIsLinkedWith(struct Ship *ship, struct Ship *otherShip)
{
    udword *linkList;
    udword *linkNode;

    linkList = (udword *)otherShip->shipsInsideMe;
    if (linkList == NULL)
    {
        return FALSE;
    }

    linkNode = (udword *)*linkList;
    while (linkNode != NULL)
    {
        if (ship == *(Ship **)(linkNode[3] + 16))
        {
            return TRUE;
        }
        linkNode = (udword *)*linkNode;
    }
    return FALSE;
}

sdword kasfShipsOrder(GrowSelection *ships)
{
    Ship *ship;

    if (ships->selection->numShips > 0)
    {
        ship = ships->selection->ShipPtr[0];
        if (ship->command)
        {
            return ship->command->ordertype.order;
        }
    }

    return 0;
}

sdword kasfShipsOrderAttributes(GrowSelection *ships)
{
    sdword i;
    sdword result = 0;

    for (i = 0; i < ships->selection->numShips; i++)
    {
        Ship *ship = ships->selection->ShipPtr[i];

        if (ship->command)
        {
            result |= ship->command->ordertype.attributes;
        }
    }

    return result;
}

//  returns percentage
sdword kasfTeamHealthAverage(void)
{
    sdword i, numShips;
    real32 maxHealth = 0.0, actualHealth = 0.0;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
        maxHealth += CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxhealth;
        actualHealth += CurrentTeamP->shipList.selection->ShipPtr[i]->health;
    }
    return (sdword) (100 * actualHealth / maxHealth);
}

//  returns percentage
sdword kasfTeamHealthLowest(void)
{
    sdword i, numShips;
    sdword thisOne = 0, lowestOne = 110;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
        thisOne = (sdword) (100 * CurrentTeamP->shipList.selection->ShipPtr[i]->health / CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxhealth);
        if (thisOne < lowestOne)
            lowestOne = thisOne;
    }
    return lowestOne;
}

//  returns percentage
sdword kasfTeamFuelAverage(void)
{
    sdword i, numShips;
    real32 maxFuel = 0.0, actualFuel = 0.0;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
            maxFuel += CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxfuel;
            actualFuel += CurrentTeamP->shipList.selection->ShipPtr[i]->fuel;
    }
    return (sdword) (100 * actualFuel / maxFuel);
}

//  returns percentage
sdword kasfTeamFuelLowest(void)
{
    sdword i, numShips;
    sdword thisOne = 0, lowestOne = 110;

    if (!CurrentTeamP)
        return 0;
    numShips = CurrentTeamP->shipList.selection->numShips;
    if (!numShips)
        return 0;

    for (i = 0; i < numShips; ++i)
    {
        thisOne = (sdword) (100 * CurrentTeamP->shipList.selection->ShipPtr[i]->fuel / CurrentTeamP->shipList.selection->ShipPtr[i]->staticinfo->maxfuel);
        if (thisOne < lowestOne)
            lowestOne = thisOne;
    }
    return lowestOne;
}

sdword kasfTeamCount(void)
{
    return CurrentTeamP->shipList.selection->numShips;
}

AITeamMove *kasfTeamSetShips(AITeam *team, GrowSelection *ships)
{
    sdword i;
    AITeamMove *currentMove;

    aitDeleteAllTeamMoves(team);
    currentMove = team->curMove;
    for (i = 0; i < ships->selection->numShips; i++)
    {
        growSelectAddShip(&team->shipList, ships->selection->ShipPtr[i]);
    }
    if (currentMove != NULL)
    {
        currentMove->processing = FALSE;
    }
    return currentMove;
}

sdword kasfNewShipsAdded(void)
{
    sdword return_value = (sdword)CurrentTeamP->newships;

    CurrentTeamP->newships = 0;

    return return_value;
}

void kasfGrowSelectionClear(GrowSelection *ships)
{
    kasGrowSelectionClear(ships);
}

sdword kasfShipsCount(GrowSelection *ships)
{
    return ships->selection->numShips;
}

sdword kasfPointInside(Volume *volume, hvector *location)
{
    vector position;

    vecGrabVecFromHVec(position, *location);

    return volPointInside(volume, &position);
}

//
//  given a volume, find all ships inside it.  make this the selection.
//  return the number of ships in this selection.
//
sdword kasfFindShipsInside(Volume *volume, GrowSelection *ships)
{
    Ship *ship;
    Node *node;

    if (ships == NULL)
        return 0;

    kasGrowSelectionClear(ships);

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            if (volPointInside(volume, &(ship->posinfo.position)))
                // add it
                growSelectAddShip(ships, ship);
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  given a volume, find all enemy ships inside it.  make this the selection.
//  return the number of ships in this selection.
//
sdword kasfFindEnemyShipsInside(Volume *volume, GrowSelection *ships)
{
    Ship *ship;
    Node *node;

    if (ships == NULL || ships->selection == NULL || volume == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            if (!ship->playerowner->playerIndex &&
                !bitTest(ship->flags, SOF_Crazy | SOF_Hide | SOF_Hyperspace | SOF_Disabled) &&
                shipGetSlotStatus(ship, 1) &&
                volPointInside(volume, &(ship->posinfo.position)))
                // add it
                growSelectAddShip(ships, ship);
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  given a selection of ships, find all enemies within a radius
//  of those ships.  make this the new selection.  return the number
//  of ships in the new selection.
//
sdword kasfFindEnemiesNearby(GrowSelection *ships, sdword radius)
{
    Ship *medianShip, *ship;
    real32 rangeSquared = (real32)radius*(real32)radius;
    Node *node;
    sdword i;

    if (ships == NULL || ships->selection == NULL || !(ships->selection->numShips))
        return 0;

    //  we should find the median ship, but since we're dealing with a selection
    //  and we probably don't intend the radius to exclude any existing ships,
    //  we'll assume the radius is large enough to consume the whole group from
    //  any (enemy) ship within the group (and then some, since the radius is supposed
    //  to add to the group if anything).  this is heavy-duty cheating, but it
    //  saves mucho CPU time if this routine were to be in many regular watch
    //  sections of script code.
    i = 0;
    do
    {
        medianShip = ships->selection->ShipPtr[i];
        ++i;
    } while (medianShip->playerowner->playerIndex && i < ships->selection->numShips); // not enemy yet

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            // is it an enemy?  (simplifying assumption: "is it the human"?)
            if ((!ship->playerowner->playerIndex) &&
                (!bitTest(ship->flags,SOF_Crazy|SOF_Slaveable|SOF_DontDrawTrails)) &&
                shipGetSlotStatus(ship, 1))
            {
                // within the radius?
                if (medianShip == ship || aiuFindDistanceSquared(medianShip->posinfo.position, ship->posinfo.position) <= rangeSquared)
                {
                    // add it
                    growSelectAddShip(ships, ship);
                }
            }
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  like FindEnemiesNearby, only check the radius relative to the current team,
//  ignoring what might be in the initial ships list
//
sdword kasfFindEnemiesNearTeam(GrowSelection *ships, sdword radius)
{
    Ship *medianShip, *ship;
    real32 rangeSquared = (real32)radius*(real32)radius;
    Node *node;

    if (ships == NULL || ships->selection == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    if (!CurrentTeamP->shipList.selection->numShips)
        return 0;

    //
    //  TBD: could try to select a better median ship among the team
    //
    medianShip = CurrentTeamP->shipList.selection->ShipPtr[0];

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_Dead|SOF_Hide)) == 0)
        {
            // is it an enemy?  (simplifying assumption: "is it the human"?)
            if (!ship->playerowner->playerIndex &&
                (!bitTest(ship->flags,SOF_Crazy|SOF_Slaveable|SOF_DontDrawTrails)) &&
                shipGetSlotStatus(ship, 1))
            {
                // within the radius?
                if (medianShip == ship || aiuFindDistanceSquared(medianShip->posinfo.position, ship->posinfo.position) <= rangeSquared)
                {
                    // add it
                    growSelectAddShip(ships, ship);
                }
            }
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  Find all the ships near a point
//
sdword kasfFindShipsNearPoint(GrowSelection *ships, hvector *location, sdword radius)
{
    Ship *ship;
    const udword excludedFlags = 0x02A08800;
    real32 rangeSquared = (real32)radius*(real32)radius;
    vector position;
    Node *node;

    if (ships == NULL || ships->selection == NULL || location == NULL)
        return 0;

    vecGrabVecFromHVec(position, *location);

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & excludedFlags) == 0)
        {
            // within the radius?
            if (aiuFindDistanceSquared(position, ship->posinfo.position) <= rangeSquared)
            {
                // add it
                growSelectAddShip(ships, ship);
            }
        }

        node = node->next;
    }

    return ships->selection->numShips;
}

//
//  combine FindShipsInside and FindEnemiesNearby because they commonly go together
//
sdword kasfFindEnemiesInside(Volume *volume, GrowSelection *ships, sdword neighborRadius)
{
    return kasfFindEnemyShipsInside(volume, ships);
    //    return kasfFindEnemiesNearby(ships, neighborRadius);
    //else
    //    return 0;
}

void kasfTeamSkillSet(sdword skillLevel)
{
    // Falko, please implement this!

    //  CurrentTeamP->skillLevel = skillLevel;
}

sdword kasfTeamSkillGet(void)
{
    // Falko, please implement this!

    //  return CurrentTeamP->skillLevel;

    return 0;
}

void kasfShipsAttributesBitSet(GrowSelection *ships,sdword attributes)
{
    SelectCommand *selection = ships->selection;
    sdword numShips = selection->numShips;
    sdword i;

    for (i=0;i<numShips;i++)
    {
        bitSet(selection->ShipPtr[i]->attributes,attributes);
    }
}

void kasfShipsAttributesBitClear(GrowSelection *ships,sdword attributes)
{
    SelectCommand *selection = ships->selection;
    sdword numShips = selection->numShips;
    bool32 clearHeadShot = bitTest(attributes, ATTRIBUTES_HeadShotKillerCollDamage);
    sdword i;

    for (i=0;i<numShips;i++)
    {
        Ship *ship = selection->ShipPtr[i];
        bool32 launchAttached = FALSE;

        if (clearHeadShot && bitTest(ship->attributes, ATTRIBUTES_HeadShotKillerCollDamage))
        {
            launchAttached = TRUE;
        }

        bitClear(ship->attributes,attributes);

        if (launchAttached)
        {
            udword *listHead = (udword *)ship->shipsInsideMe;
            MaxSelection launchSelection;
            udword *node;

            launchSelection.numShips = 0;
            if (listHead != NULL)
            {
                for (node = (udword *)listHead[0]; node != NULL; node = (udword *)node[0])
                {
                    Ship *linkedShip = (Ship *)*(udword *)(node[3] + 16);

                    if ((lmShipAutoLaunchEnabled(ship) || ShipHasToLaunch(ship, linkedShip)) &&
                        !(ship->specialFlags3 & SPECIAL3_Unknown12))
                    {
                        launchSelection.ShipPtr[launchSelection.numShips++] = linkedShip;
                    }
                }
            }

            if (launchSelection.numShips > 0)
            {
                clLaunchMultipleShips(&universe.mainCommandLayer, (SelectCommand *)&launchSelection, ship);
            }
        }
    }
}

void kasfShipsAttributesSet(GrowSelection *ships,sdword attributes)
{
    SelectCommand *selection = ships->selection;
    sdword numShips = selection->numShips;
    sdword i;

    for (i=0;i<numShips;i++)
    {
        selection->ShipPtr[i]->attributes = attributes;
    }
}

void kasfTeamAttributesBitSet(sdword attributes)
{
    sdword i, numShips;
    Ship *shipP;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;

    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = team->shipList.selection->ShipPtr[i];
        bitSet(shipP->attributes,attributes);
    }
}

void kasfTeamAttributesBitClear(sdword attributes)
{
    sdword i, numShips;
    Ship *shipP;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;

    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = team->shipList.selection->ShipPtr[i];
        bitClear(shipP->attributes,attributes);
    }
}

void kasfTeamAttributesSet(sdword attributes)
{
    sdword i, numShips;
    Ship *shipP;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;

    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = team->shipList.selection->ShipPtr[i];
        shipP->attributes = attributes;
    }
}

void kasfTeamMakeCrazy(sdword makecrazy)
{
    sdword i, numShips;
    Ship *shipP;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;

    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = team->shipList.selection->ShipPtr[i];
        if (makecrazy)
        {
            ApplyCareenRotationDirectly(shipP);
            bitSet(shipP->flags,SOF_Crazy);
        }
        else
        {
            bitClear(shipP->flags,SOF_Crazy);
        }
    }
}

void kasfDisablePlayerHyperspace(void)
{
    singlePlayerGameInfo.hyperspaceState = HYPERSPACE_LAUNCH_DOCKED;
    telldir();
}

void kasfHoldHyperspaceWindow(bool32 hold)
{
    if (hold)
    {
        spMainScreenAndLockout(SPLOCKOUT_EVERYTHING);
    }
    else
    {
        spClearScreenLockout();
    }
}

void kasfTeamHyperspaceIn(hvector *destination)
{
    if (!CurrentTeamP)
        return;

    spHyperspaceSetSelectionDestination(CurrentTeamP->shipList.selection, destination);
    spHyperspaceDelay = 0.0f;
}

void kasfTeamHyperspaceInNear(hvector *destination, sdword distance)
{
    vector origin = {0.0,0.0,0.0};
    vector dest, hypepoint;
    hvector finalHyperspacePoint;
    real32 universeHalfX = smUniverseSizeX * 0.5f;
    real32 universeHalfY = smUniverseSizeY * 0.5f;
    real32 universeHalfZ = smUniverseSizeZ * 0.5f;

    if (!CurrentTeamP)
        return;

    vecGrabVecFromHVec(dest, *destination);
    hypepoint = aiuGenerateRandomStandoffPoint(dest, (real32)distance, origin, RSP_NORMAL);

    if (hypepoint.x > universeHalfX)
        hypepoint.x = universeHalfX;
    else if (hypepoint.x < -universeHalfX)
        hypepoint.x = -universeHalfX;

    if (hypepoint.y > universeHalfY)
        hypepoint.y = universeHalfY;
    else if (hypepoint.y < -universeHalfY)
        hypepoint.y = -universeHalfY;

    if (hypepoint.z > universeHalfZ)
        hypepoint.z = universeHalfZ;
    else if (hypepoint.z < -universeHalfZ)
        hypepoint.z = -universeHalfZ;

    vecMakeHVecFromVec(finalHyperspacePoint, hypepoint);

    spHyperspaceSetSelectionDestination(CurrentTeamP->shipList.selection, &finalHyperspacePoint);
    spHyperspaceDelay = 0.0f;
}

void kasfHyperspaceDelay(sdword milliseconds)
{
    spHyperspaceDelay = ((real32)milliseconds) / 1000.0f;
}

void kasfTeamHyperspaceOut(void)
{
    MaxSelection filtered;

    if (!CurrentTeamP)
        return;

    filtered.numShips = 0;

    aiuFilterDisabledShips(CurrentTeamP->shipList.selection, &filtered);

    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&filtered);
    spHyperspaceSelectionOut((SelectCommand *)&filtered);
    spHyperspaceDelay = 0.0f;
}

void kasfTeamSetMaxVelocity(sdword maxVelocity)
{
    sdword i, numShips;
    Ship *shipP;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;
    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = team->shipList.selection->ShipPtr[i];
        shipP->singlePlayerSpeedLimiter = (real32)maxVelocity;
        bitSet(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

void kasfTeamClearMaxVelocity(void)
{
    sdword i, numShips;
    Ship *shipP;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;
    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = team->shipList.selection->ShipPtr[i];
        bitClear(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfShipsSetMaxVelocity
    Description : Same as TeamSetMaxVelocity, except that it works on arbitrary
                    ship selections.
    Inputs      : ships - ships to cap velocity of, be they friendly or enemy.
                  maxVelocity - velocity to cap them at.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsSetMaxVelocity(GrowSelection *ships, sdword maxVelocity)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        shipP->singlePlayerSpeedLimiter = (real32)maxVelocity;
        bitSet(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfShipsClearMaxVelocity
    Description : Similar to TeamClearMaxVelocity, except that it works on an
                    arbitrary selection of ships.
    Inputs      : ships - ships to cap velocity of, be they friendly or enemy.
                  maxVelocity - velocity to cap them at.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsClearMaxVelocity(GrowSelection *ships, sdword maxVelocity)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        bitClear(shipP->specialFlags,SPECIAL_SinglePlayerLimitSpeed);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfShipsSetDamageFactor
    Description : Sets a modifier in the ship structure to
                    modify the ammount of damage a ship does.
    Inputs      : ships - ships to modify damage of.
                  damagePercent (0 to 100) - divided by 100 then multiplied by ships damage
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsSetDamageFactor(GrowSelection *ships, sdword damagePercent)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        shipP->damageModifier = ((real32)damagePercent)/100.0f;
        bitSet(shipP->specialFlags,SPECIAL_KasSpecialDamageModifier);
    }
}
/*-----------------------------------------------------------------------------
    Name        : kasfShipsClearDamageFactor
    Description : Sets a modifier in the ship structure to
                    modify the ammount of damage a ship does.
    Inputs      : ships - ships to modify damage of.
                  damagePercent (0 to 100) - divided by 100 then multiplied by ships damage
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfShipsClearDamageFactor(GrowSelection *ships)
{
    sdword i, numShips;
    Ship *shipP;

    if (!ships)
        return;
    numShips = ships->selection->numShips;

    for (i = 0; i < numShips; ++i)
    {
        shipP = ships->selection->ShipPtr[i];
        bitClear(shipP->specialFlags,SPECIAL_KasSpecialDamageModifier);
    }
}


// this function is a bit of a misnomer: it actually sets the percentage of health
// therefore 0 "percentDamaged", actually means the health is at 0 and the ship is dead
void kasfTeamSetPercentDamaged(sdword percentDamaged)
{
    sdword i, numShips;
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;
    numShips = team->shipList.selection->numShips;

    for (i = 0; i < numShips; ++i)
        team->shipList.selection->ShipPtr[i]->health =
            team->shipList.selection->ShipPtr[i]->staticinfo->maxhealth * percentDamaged / 100;
}

void kasfShipsSetPercentDamaged(GrowSelection *ships, sdword percentDamaged)
{
    sdword i, numShips;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }
    numShips = ships->selection->numShips;
    for (i = 0; i < numShips; ++i)
    {
        ships->selection->ShipPtr[i]->health =
            ships->selection->ShipPtr[i]->staticinfo->maxhealth * percentDamaged / 100;
    }
}

void kasfSetSalCapProcessingShip(GrowSelection *ships)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->numShips <= 0)
    {
        return;
    }
    SalCapSetProcessingShip(ships->selection->ShipPtr[0]);
}

void kasfTeamSetPercentFueled(sdword percentFueled)
{
    AITeam *team = kasfScriptTeam();
    sdword i;
    sdword numShips;

    if (team == NULL)
    {
        return;
    }

    numShips = team->shipList.selection->numShips;
    for (i = 0; i < numShips; ++i)
    {
        team->shipList.selection->ShipPtr[i]->fuel =
            team->shipList.selection->ShipPtr[i]->staticinfo->maxfuel * percentFueled / 100;
    }
}

void kasfMissionSkillSet(sdword skillLevel)
{
    CurrentMissionSkillLevel = skillLevel;
}

sdword kasfMissionSkillGet(void)
{
    return CurrentMissionSkillLevel;
}

void kasfRequestShips(char *shipType, sdword numShips)
{
    AITeam *team = kasfScriptTeam();
    AITeam *teamp;
    ShipType st = StrToShipType(shipType);
//    if (st < 0)
//        st = LightInterceptor;

    // make a temp team and get them to reinforce the first team
    teamp = aitCreate(ScriptTeam);
    teamp->kasLabelHash = team->kasLabelHash + 1;
    teamp->kasOrigShipsCount = 0;
    aimCreateGetShips(teamp, st, (sbyte)numShips, (sdword)REQUESTSHIPS_HIPRI, TRUE, TRUE);
    aimCreateReinforce(teamp, team, SAME_FORMATION, team->kasTactics, TRUE, TRUE);
    aimCreateDeleteTeam(teamp);
}

void kasfRequestShipsOriginal(sdword percentOriginal)
{
    AITeam *teamp;
    sbyte numShips = (sbyte)((CurrentTeamP->kasOrigShipsCount * (real32)percentOriginal * 0.01f) + 0.5f);
    if (numShips <= 0)
    {
        numShips = 1;
    }

    // make a temp team and get them to reinforce the first team
    teamp = aitCreate(ScriptTeam);
    teamp->kasOrigShipsCount = 0;
    aimCreateGetShips(teamp, CurrentTeamP->kasOrigShipsType, numShips, (sdword)REQUESTSHIPS_HIPRI, TRUE, TRUE);
    aimCreateReinforce(teamp, CurrentTeamP, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
    aimCreateDeleteTeam(teamp);
}

void kasfReinforceTeamWithShips(struct AITeam *teamtoreinforce,GrowSelection *shipstoadd)
{
    sdword i;
    sdword numShips = shipstoadd->selection->numShips;
    AITeamMove *reinforceMove = teamtoreinforce->curMove;

    for (i=0;i<numShips;i++)
    {
        growSelectAddShip(&teamtoreinforce->shipList, shipstoadd->selection->ShipPtr[i]);
    }

    if (reinforceMove)
    {
        reinforceMove->processing = FALSE;
    }
}

void kasfReinforce(AITeam *team)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateReinforce(CurrentTeamP, team, SAME_FORMATION, CurrentTeamP->kasTactics, TRUE, TRUE);
}

void kasfForceCombatStatus(GrowSelection *ships, sdword on)
{
    sdword i;

    if (on)
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            bitSet(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_ForcedAttackStatus);
        }
    }
    else
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            bitClear(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_ForcedAttackStatus);
        }
    }
}

sdword kasfThisTeamIs(AITeam *team)
{
    return (team == CurrentTeamP);
}

sdword kasfTeamCountOriginal(void)
{
    return CurrentTeamP->kasOrigShipsCount;
}

void kasfTeamGiveToAI(void)
{
    Ship *ship;
    sdword i;

    if (!CurrentTeamP)
        return;

    for (i=0;i<CurrentTeamP->shipList.selection->numShips;)
    {
        ship = CurrentTeamP->shipList.selection->ShipPtr[i];
        aitRemoveShip(CurrentTeamP, ship);
        growSelectAddShip(&aiCurrentAIPlayer->newships, ship);

        if (ShipTypeIsMothershipOrCarrierType(ship->shiptype))
        {
            aiCurrentAIPlayer->AICreator = ship;
        }
    }
}

void kasfDisableAIFeature(sdword feature, sdword type)
{
    switch (type)
    {
        case ResourceFeature:
            aiCurrentAIPlayer->ResourceFeatures &= ~feature;
            break;
        case DefenseFeature:
            aiCurrentAIPlayer->DefenseFeatures &= ~feature;
            break;
        case AttackFeature:
            aiCurrentAIPlayer->AttackFeatures &= ~feature;
            break;
        case TeamFeature:
            CurrentTeamP->TeamFeatures |= feature;
            break;
        default:
            return;
    }
}
void kasfEnableAIFeature(sdword feature, sdword type)
{
    switch (type)
    {
        case ResourceFeature:
            aiCurrentAIPlayer->ResourceFeatures |= feature;
            break;
        case DefenseFeature:
            aiCurrentAIPlayer->DefenseFeatures |= feature;
            break;
        case AttackFeature:
            aiCurrentAIPlayer->AttackFeatures |= feature;
            break;
        case TeamFeature:
            CurrentTeamP->TeamFeatures |= feature;
            break;
        default:
            return;
    }
}

void kasfDisableAllAIFeatures(void)
{
    aiCurrentAIPlayer->ResourceFeatures = 0;
    aiCurrentAIPlayer->DefenseFeatures = 0;
    aiCurrentAIPlayer->AttackFeatures = 0;
//    aiCurrentAIPlayer->TeamFeatures = 0;
}
void kasfEnableAllAIFeatures(void)
{
    aiCurrentAIPlayer->ResourceFeatures = 0xFFFFFFFF;
    aiCurrentAIPlayer->DefenseFeatures = 0xFFFFFFFF;
    aiCurrentAIPlayer->AttackFeatures = 0xFFFFFFFF;
//    aiCurrentAIPlayer->TeamFeatures = 0xFFFFFFFF;
}


#if 0
void kasfTeamGiveToPlayer(void)
{
    Ship *ship;

    if (!CurrentTeamP)
        return;

    while (CurrentTeamP->shipList.selection->numShips)
    {
        ship = CurrentTeamP->shipList.selection->ShipPtr[0];
        aitRemoveShip(CurrentTeamP, ship);        // remove ship from team - and give to no one so it is free
        bitSet(ship->flags,SOF_Selectable);         // make sure player can select it
    }
}
#endif

static void SelectionSwitchPlayerOwnerInternal(SelectCommand *selection, bool32 fullTransferToPlayer)
{
    sdword i;
    sdword numShips = selection->numShips;
    Ship *ship;
    Player *oldOwner;

    RemoveShipsFromDoingStuff(&universe.mainCommandLayer, selection);

    for (i = 0; i < numShips; i++)
    {
        ship = selection->ShipPtr[i];
        if (ship->clampInfo != NULL && ship->clampInfo->selection != NULL)
        {
            SelectionSwitchPlayerOwnerInternal((SelectCommand *)ship->clampInfo->selection, fullTransferToPlayer);
        }

        RemoveShipFromBeingTargeted(&universe.mainCommandLayer, ship, REMOVE_PROTECT, 0);      // remove ships from being targeted when switching sides
        if (ship->playerowner == &universe.players[0])
        {
            oldOwner = ship->playerowner;
            univPingShipDied((sdword)ship);
            clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)ship);
            univRemoveShipFromHotkeyGroup((sdword)ship, FALSE);

            ship->attributes |= ATTRIBUTES_Defector;
            bitClear(ship->flags, SOF_Selectable);
            ship->playerowner = &universe.players[1];
            cat_unitCapCreateShip(ship, ship->playerowner);
            unitCapDeleteShip(ship, oldOwner);
            shipNameRelease(ship);
            if ((ship->specialFlags2 & SPECIAL_2_DisabledState) == 0)
            {
                shipRemoveSupportUsage(ship);
            }
            shipSetDisabledState(ship, FALSE);
        }
        else
        {
            oldOwner = ship->playerowner;
            if (fullTransferToPlayer)
            {
                aiplayerShipDied(ship);
            }
            else
            {
                clSetAttribute((sdword)ship, 1, 1);
                shipSetDisabledState(ship, TRUE);
            }

            bitClear(ship->attributes, ATTRIBUTES_Defector);
            bitClear(ship->flags, SOF_Slaveable);
            bitSet(ship->flags, SOF_Selectable);
            ship->playerowner = &universe.players[0];
            if (ship->clampInfo != NULL)
            {
                univUpdateShipProximity(ship);
            }
            cat_unitCapCreateShip(ship, ship->playerowner);
            unitCapDeleteShip(ship, oldOwner);
            shipNameReserve(ship);
            if (fullTransferToPlayer)
            {
                if (ship->staticinfo->canBuildShips)
                {
                    cmAddFactory(ship, ship->staticinfo->canBuildBigShips);
                }
                if (ship->shiptype == sMothership && !universe.curPlayerPtr->PlayerMothership)
                {
                    universe.curPlayerPtr->PlayerMothership = ship;
                }
                shipAddSupportUsage(ship, &universe.players[0]);
            }
        }
    }

    collUpdateAllBlobs = 1;
}

void SelectionSwitchPlayerOwner(SelectCommand *selection, bool32 fullTransferToPlayer)
{
    SelectionSwitchPlayerOwnerInternal(selection, fullTransferToPlayer);
}

void kasfTeamSwitchPlayerOwner(bool32 fullTransferToPlayer)
{
    AITeam *team = kasfScriptTeam();

    if (!team)
        return;

    SelectionSwitchPlayerOwner(team->shipList.selection, fullTransferToPlayer);
}

void kasfShipsSwitchPlayerOwner(GrowSelection *ships, bool32 fullTransferToPlayer)
{
    SelectionSwitchPlayerOwner(ships->selection, fullTransferToPlayer);
}

void kasfMoveTo(hvector *destination)
{
    AITeam *team = kasfScriptTeam();
    vector dest;

    if (team == NULL)
    {
        return;
    }

    vecGrabVecFromHVec(dest,*destination);
    aitDeleteAllTeamMoves(team);
    aimCreateMoveTeam(team, dest, SAME_FORMATION, FALSE, TRUE);
}

void kasfShipsMoveTo(GrowSelection *ships, hvector *destination)
{
    vector dest;
    vecGrabVecFromHVec(dest,*destination);
    aimIssueMoveCommand(ships->selection, dest);
}

void kasfTacticsAggressive(void)
{
    CurrentTeamP->kasTactics = Aggressive;
    aiuWrapSetTactics(CurrentTeamP->shipList.selection, Aggressive);
}

void kasfTacticsNeutral(void)
{
    CurrentTeamP->kasTactics = Neutral;
    aiuWrapSetTactics(CurrentTeamP->shipList.selection, Neutral);
}

void kasfTacticsEvasive(void)
{
    CurrentTeamP->kasTactics = Evasive;
    aiuWrapSetTactics(CurrentTeamP->shipList.selection, Evasive);
}

//
//  returns True if any ship in the team is a given distance from the given location
//
sdword kasfNearby(hvector *location, sdword distance)
{
    real32 rangeSquared = (real32)distance*(real32)distance;
    sdword i, numShips;
    vector loc;
    vecGrabVecFromHVec(loc,*location);

    numShips = CurrentTeamP->shipList.selection->numShips;
    for (i = 0; i < numShips; ++i)
    {
        if (aiuFindDistanceSquared(CurrentTeamP->shipList.selection->ShipPtr[i]->posinfo.position, loc) <= rangeSquared)
            return TRUE;
    }
    return FALSE;
}

//
// returns the distance between two points
//
sdword kasfFindDistance(hvector *location1, hvector *location2)
{
    vector loc1, loc2;
    real32 distsq;

    vecGrabVecFromHVec(loc1, *location1);
    vecGrabVecFromHVec(loc2, *location2);

    distsq = aiuFindDistanceSquared(loc1, loc2);

    return (sdword)fsqrt(distsq);
}


sdword kasfUnderAttackElsewhere(struct AITeam *otherTeam, GrowSelection *attackers)
{
    sdword i;
    SelectCommand *teamSelection = otherTeam->shipList.selection;
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    Ship *ship;

    if (attackers == NULL)
        return 0;

    //  wipe the selection clean
    attackers->selection->numShips = 0;

    if (teamSelection->numShips == 0)
        return 0;

    // lets walk the command layer and see who's attacking us:
    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if ((command->ordertype.order == COMMAND_ATTACK) ||
            (command->ordertype.attributes & (COMMAND_IS_PASSIVEATTACKING|COMMAND_IS_ATTACKINGANDMOVING)))
        {
            if (command->attack)
            {
                if (AnyOfTheseShipsAreInSelection(teamSelection,(SelectCommand *)command->attack))
                {
                    SelectCommand *attackedby = command->selection;
                    for (i = 0;i < attackedby->numShips; i++)
                    {
                        ship = attackedby->ShipPtr[i];
                        growSelectAddShipNoDuplication(attackers,ship);
                    }
                }
            }
        }

        curnode = curnode->next;
    }

    return attackers->selection->numShips;
}

sdword kasfUnderAttack(GrowSelection *attackers)
{
    return kasfUnderAttackElsewhere(CurrentTeamP, attackers);
}

sdword kasfShipsCountType(GrowSelection *ships, char *shipType)
{
    sdword i, count = 0, numShips = ships->selection->numShips;
    ShipType st = StrToShipType(shipType);
//    if (st < 0)
//        return 0;
    for (i = 0; i < numShips; ++i)
        if (ships->selection->ShipPtr[i]->shiptype == st)
            ++count;
    return count;
}

void kasfDock(struct AITeam *withTeam)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDock(CurrentTeamP, dockmoveFlags_Normal, NULL, FALSE, TRUE);
}

void kasfDockSupport()
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDock(CurrentTeamP, dockmoveFlags_Normal, NULL, FALSE, TRUE);
}

void DockSupportWith(struct AITeam * withTeam,sdword flags)
{
    Ship *dockwith = NULL;
    if (withTeam->shipList.selection->numShips >= 1)
    {
        dockwith = withTeam->shipList.selection->ShipPtr[0];
        dbgAssert(dockwith);
        if (aiuShipIsntSelectable(dockwith))
        {
            dockwith = NULL;
        }
        else
        {
            flags |= dockmoveFlags_AtShip;
        }
    }

    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateDock(CurrentTeamP, flags, dockwith, FALSE, TRUE);
}

void kasfDockSupportWith(struct AITeam * withTeam)
{
    DockSupportWith(withTeam,0);
}

void kasfShipsDockSupportWith(GrowSelection *ships, GrowSelection *withShips)
{
    aiuWrapDock(ships->selection, DOCK_AT_SPECIFIC_SHIP, withShips->selection->ShipPtr[0]);
}

void kasfDockStay(struct AITeam * withTeam)
{
    SelectCommand *selection;

    if (CurrentTeamP)
    {
        selection = CurrentTeamP->shipList.selection;
        if (selection->numShips)
        {
            aiuWrapDock(selection, DOCK_MODE_BEAST, withTeam->shipList.selection->ShipPtr[0]);
        }
    }
}

void kasfShipsDockStay(GrowSelection *ships, GrowSelection *withShips)
{
    sdword i;
    SelectCommand *selection;

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        selection->ShipPtr[i]->specialFlags1C8 |= CATA_RUNTIME_LAUNCHED_FROM_KAS | CATA_RUNTIME_KAS_CHECK_DONE_LAUNCHING;
    }

    aiuWrapDock(selection, DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP, withShips->selection->ShipPtr[0]);
}

static bool32 kasfDockTargetIsFiltered(Ship *dockWith)
{
    return dockWith == NULL ||
           ((dockWith->flags & SOF_DontDrawTrails) != 0) ||
           ShipInSelection(singlePlayerGameInfo.ShipsToHyperspace.selection, dockWith) ||
           ((dockWith->specialFlags2 & SPECIAL_2_DockFlag) != 0);
}

void kasfDockStayMothership(void)
{
    Ship *dockwith;

    if (CurrentTeamP->shipList.selection->numShips >= 1)
    {
        dockwith = CurrentTeamP->shipList.selection->ShipPtr[0]->playerowner->PlayerMothership;
        if (!kasfDockTargetIsFiltered(dockwith))
        {
            aitDeleteAllTeamMoves(CurrentTeamP);
            aimCreateDock(CurrentTeamP, dockmoveFlags_AtShip|dockmoveFlags_Stay, dockwith, FALSE, TRUE);
        }
    }
}

void kasfDockInstant(struct AITeam * withTeam)
{
    AITeam *team = kasfScriptTeam();
    Ship *dockShip;
    sdword i;

    if (team == NULL || withTeam == NULL || withTeam->shipList.selection->numShips <= 0)
    {
        return;
    }
    if (team->shipList.selection->numShips <= 0)
    {
        return;
    }

    dockShip = withTeam->shipList.selection->ShipPtr[0];
    for (i = 0; i < team->shipList.selection->numShips; i++)
    {
        univRemoveShipFromOutside(team->shipList.selection->ShipPtr[i]);
        dockPutShipInside(team->shipList.selection->ShipPtr[i], dockShip);
    }

    if (dockShip->shiptype == sHiveFrigate || dockShip->shiptype == bHiveFrigate)
    {
        shipClearSwarmers(dockShip);
    }
}

void kasfLaunch()
{
    sdword i;

    for (i=0;i<CurrentTeamP->shipList.selection->numShips;i++)
    {
        CurrentTeamP->shipList.selection->ShipPtr[i]->specialFlags1C8 |= CATA_RUNTIME_LAUNCHED_FROM_KAS | CATA_RUNTIME_KAS_CHECK_DONE_LAUNCHING;
    }

    aitDeleteAllTeamMoves(CurrentTeamP);
    aimCreateLaunch(CurrentTeamP, FALSE, TRUE);
}

sdword kasfTeamDocking(void)
{
    //return bitTest(CurrentTeamP->teamFlags, TEAM_DOCKING);
    return ((CurrentTeamP->teamFlags & TEAM_DOCKING) != 0);  // ensure 0/1 return value
}

bool32 ShipReadyForLaunch(Ship *ship)
{
    if ((ship->flags & SOF_DockMarker) != 0 &&
        (ship->flags & 0x02000000) == 0)
    {
        return !ShipIsWaitingForSoftLaunch(ship);
    }

    return (ship->specialFlags1C8 & BIT12) &&
           ShipIsRefuelingAtCarrierMother(ship);
}

bool32 ShipFinishedLaunching(Ship *ship)
{
    return ((ship->specialFlags1C8 & CATA_RUNTIME_KAS_CHECK_DONE_LAUNCHING) == 0) &&
           ship->shiplink.belongto != NULL;
}

sdword kasfTeamFinishedLaunching(void)
{
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    sdword i;

    if (selection->numShips == 0)
    {
        return FALSE;
    }

    for (i=0;i<selection->numShips;i++)
    {
        if (!ShipFinishedLaunching(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

sdword kasfTeamDockedReadyForLaunch(void)
{
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    sdword i;

    if (selection->numShips == 0)
    {
        return FALSE;
    }

    for (i=0;i<selection->numShips;i++)
    {
        if (!ShipReadyForLaunch(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//
//  send to all script teams
//
void kasfMsgSendAll(char *msg)
{
    AITeam *teamp;
    sdword i;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        teamp = aiCurrentAIPlayer->teams[i];
          //not really sure we need this...  CPUTeams deserve to get messages too.
//        if ((teamp->teamType == ScriptTeam) || (teamp->teamType == AnyTeam))
            aitQueuePointer(CurrentTeamP, teamp, msg);
    }
}

void kasfMsgSend(struct AITeam *team, char *msg)
{
    aitQueuePointer(CurrentTeamP, team, msg);
}

sdword kasfRUsEnemyCollected(void)
{
    return singlePlayerGameInfo.resourceUnitsCollected;
}

//  serves the "friendly" and "enemy" functions below
static sdword kasfFindPlayersShipsOfType(GrowSelection *ships, char *shipType, sdword playerIndex)
{
    Node *node;
    ShipType st = StrToShipType(shipType);
    Ship *ship;

//    if (st < 0)
//        return 0;

    if (ships == NULL || ships->selection == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_DeCloaking | SOF_DockMarker)) == 0)
        {
            //  1. is it an enemy?  (simplifying assumption: "is it the human"?)
            //  2. is the right type?
            if (ship->playerowner->playerIndex == playerIndex &&
                ship->shiptype == st &&
                (ship->specialFlags4 & SPECIAL4_ExternalBuildMember) == 0)
                // add it
                growSelectAddShip(ships, ship);
        }
        node = node->next;
    }

    return ships->selection->numShips;
}

//  (enemy == human)
sdword kasfFindEnemyShipsOfType(GrowSelection *ships, char *shipType)
{
    return kasfFindPlayersShipsOfType(ships, shipType, 0);
}

//  (friendly == not human)
sdword kasfFindFriendlyShipsOfType(GrowSelection *ships, char *shipType)
{
    return kasfFindPlayersShipsOfType(ships, shipType, 1);
}

sdword kasfModuleSlotOrderSetShipType(sdword orderGroup, char *shipType)
{
    sdword shipTypeId;

    shipTypeId = StrToShipType(shipType);
    if (orderGroup == 1)
    {
        return smMimicAddShipType(1, shipTypeId);
    }
    if (orderGroup == 2)
    {
        return smMimicAddShipType(8, shipTypeId);
    }

    return smMimicAddShipType((ubyte)orderGroup, shipTypeId);
}

sdword kasfModuleSlotOrderSetCount(sdword orderGroup, sdword count)
{
    if (orderGroup == 1)
    {
        return titan_5BA150(1, count);
    }
    if (orderGroup == 2)
    {
        return titan_5BA150(8, count);
    }

    return titan_5BA150((ubyte)orderGroup, count);
}

// Cataclysm-specific: rebuild the mission-script enemy mothership selection.
sdword kasfFindEnemyMothership(GrowSelection *result)
{
    Ship *enemyMothership;

    kasGrowSelectionClear(result);
    enemyMothership = aiuFindEnemyMothership(&universe.players[1]);
    if (enemyMothership != NULL)
    {
        growSelectAddShip(result, enemyMothership);
    }

    return result->selection->numShips;
}

//  serves the "friendly" and "enemy" functions below
static sdword kasfFindPlayersShipsOfClass(GrowSelection *ships, char *shipClass, sdword playerIndex)
{
    Node *node;
    ShipClass sc = StrToShipClass(shipClass);
    Ship *ship;

//    if (sc < 0)
//        return 0;

    if (ships == NULL || ships->selection == NULL)
        return 0;

    //  wipe the selection clean
    ships->selection->numShips = 0;

    //  check all ships in the damn universe
    node = universe.ShipList.head;
    while (node != NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if ((ship->flags & (SOF_DeCloaking | SOF_DockMarker)) == 0)
        {
            //  1. is it an enemy?  (simplifying assumption: "is it the human"?)
            //  2. is the right class?
            if (ship->playerowner->playerIndex == playerIndex && ship->staticinfo->shipclass == sc)
                // add it
                growSelectAddShip(ships, ship);
        }
        node = node->next;
    }

    return ships->selection->numShips;
}

//  (enemy == human)
sdword kasfFindEnemyShipsOfClass(GrowSelection *ships, char *shipClass)
{
    return kasfFindPlayersShipsOfClass(ships, shipClass, 0);
}

//  (friendly == not human)
sdword kasfFindFriendlyShipsOfClass(GrowSelection *ships, char *shipClass)
{
    return kasfFindPlayersShipsOfClass(ships, shipClass, 1);
}

sdword kasfPathNextPoint(void)
{
    AITeamMove *thisMove = CurrentTeamP->curMove;

    if (CurrentTeamP->shipList.selection->numShips <= 0)
        return -1;

    if (!thisMove)
    {
        if (CurrentTeamP->moves.num > 0)
            thisMove = (AITeamMove *)listGetStructOfNode(CurrentTeamP->moves.tail);
        if (!thisMove)
            return -1;
    }

    if (thisMove->type == MOVE_PATROLMOVE)
    {
        if (thisMove->processing)
            // end of path or loop
            return 0;
        else
            return 1;  // first point
    }
    if (thisMove->type != MOVE_MOVETEAMINDEX)
        return -1;  // not patrolling a path

    return thisMove->params.move.index + 1;
}

//
//  PETER: I think we can nuke this now, right?
//

//
//  this is a placeholder for a real fleet intelligence message window
//
void kasfPopupTextDraw(void)
{
#if 1
    sdword rowHeight, x, y, i, popupTextBorder;
    rectangle rect;
    fonthandle fhSave;
    //char buf[256];

    if (!gameIsRunning || !popupTextNumLines)
        return;

    if (nisIsRunning)
        popupTextNumLines = 0;

    // set up textual stuff
    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    // to center it:
    x = 320 - popupTextWidth/2;
    //x = 640 - popupTextWidth - rowHeight*2;

    rowHeight = fontHeight("M") + 3; // used to space the text
    // to center it:
    y = (480 - (popupTextNumLines+2) * rowHeight) / 2;
    //y = rowHeight*2;
    popupTextBorder = rowHeight + 6;

    // border
    //primLineLoopStart2(1, POPUPTEXT_COLOR);
    //primLineLoopPoint3F(x - popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopPoint3F(x - popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopEnd2();

    rect.x0 = x - popupTextBorder;
    rect.x1 = x + popupTextWidth + popupTextBorder;
    rect.y0 = y - popupTextBorder;
    rect.y1 = y + rowHeight * (popupTextNumLines+2) + popupTextBorder;
    primBeveledRectSolid(&rect, POPUPTEXT_BACKGROUND, 4, 4);
    primSeriesOfRoundRects(&rect, 1, POPUPTEXT_COLOR, POPUPTEXT_BORDER_COLOR, 5, 4, 4);

    // title
    fontPrint(x, y, POPUPTEXT_TITLE_COLOR, "FLEET INTELLIGENCE");
    y += rowHeight*2;

    // each line of text
    for (i = 0; i < popupTextNumLines; i++)
    {
        fontPrint(x, y, POPUPTEXT_COLOR, popupTextLines[i]);
        y += rowHeight;
    }

    // stop displaying when expired
    if (taskTimeElapsed > popupTextEndTime)
        popupTextNumLines = 0;

    fontMakeCurrent(fhSave);
#endif
#if 0
       //sdword rowHeight, x, y, i, popupTextBorder;
    sdword i, width, x, y;
    bool32 done, justified;
    //rectangle rect;
    fonthandle fhSave;
    char *pos, *oldpos;
    char oldline[256], line[256];
    //char buf[256];

    if (!gameIsRunning || !popupTextNumLines)
        return;

    // set up textual stuff
    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    //x = 320 - popupTextWidth/2;

    //rowHeight = fontHeight("M") + 3; // used to space the text
    //y = (480 - (popupTextNumLines+2) * rowHeight) / 2;
    //popupTextBorder = rowHeight + 6;

    // border
    //primLineLoopStart2(1, POPUPTEXT_COLOR);
    //primLineLoopPoint3F(x - popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y - popupTextBorder);
    //primLineLoopPoint3F(x + popupTextWidth + popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopPoint3F(x - popupTextBorder, y + rowHeight * (popupTextNumLines+2) + popupTextBorder);
    //primLineLoopEnd2();

    //rect.x0 = x - popupTextBorder;
    //rect.x1 = x + popupTextWidth + popupTextBorder;
    //rect.y0 = y - popupTextBorder;
    //rect.y1 = y + rowHeight * (popupTextNumLines+2) + popupTextBorder;
    //primBeveledRectSolid(&rect, POPUPTEXT_BACKGROUND, 4, 4);
    //primSeriesOfRoundRects(&rect, 1, POPUPTEXT_COLOR, POPUPTEXT_BORDER_COLOR, 5, 4, 4);

    // title
    //fontPrint(x, y, POPUPTEXT_TITLE_COLOR, "FLEET INTELLIGENCE");
    //y += rowHeight*2;

    x = rect->x0;
    y = rect->y0;

    // each line of text
    for (i = 0; i < popupTextNumLines; i++)
    {
        //fontPrint(x, y, POPUPTEXT_COLOR, popupTextLines[i]);
        pos = popupTextLines[i];

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    justified = TRUE;
                    pos++;
                    while ( pos[0] == ' ' ) pos++;
                }
                else
                {
                    if ( (width=fontWidth(line)) > (rect->x1 - rect->x0))
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while ( pos[0] == ' ' ) pos++;

                        justified = TRUE;
                    }
                    if (pos[0]==0)
                    {
                        justified = TRUE;
                        done      = TRUE;
                    }
                }
            }

            fontPrintf(x,y,POPUPTEXT_COLOR,"%s",line);
            y += fontHeight(" ");
            if (y > rect->y1 + fontHeight(" ")) done=TRUE;
        }
        //y += rowHeight;
    }

    // stop displaying when expired
    //if (universe.totaltimeelapsed > popupTextEndTime)
    //    popupTextNumLines = 0;

    fontMakeCurrent(fhSave);
#endif
}


//
//  PETER: I think we can nuke this now, right?
//
void kasfPopupText(char *line)
{
    sdword width;
    fonthandle fhSave;

    if (popupTextNumLines >= POPUPTEXT_MAX_LINES)
    {
        strcpy(popupTextLines[POPUPTEXT_MAX_LINES-1], " - - message truncated - -");
        return;
    }

    // set up textual stuff
    fhSave = fontCurrentGet();                  //save the current font
    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    strncpy(popupTextLines[popupTextNumLines++], line, POPUPTEXT_MAX_LINE_LENGTH);
    width = fontWidth(popupTextLines[popupTextNumLines-1]);

    // set duration relative to number of lines
    if (popupTextNumLines == 1)
    {
        popupTextEndTime = universe.totaltimeelapsed;
        popupTextWidth = POPUPTEXT_BOX_WIDTH;
    }

    if (width > popupTextWidth)
        popupTextWidth = width;

    popupTextEndTime += POPUPTEXT_DURATION_PER_LINE;


    fontMakeCurrent(fhSave);
}

//
//  PETER: I think we can nuke this now, right?
//
void kasfPopupTextClear(void)
{
    popupTextNumLines = 0;
}

//
//  new fleet-intelligence style popup
//
void kasfPopup(char *text)
{
    fleetIntelligenceCreate(text, 1);
}

void kasfPopupInteger(char *text, sdword integer)
{
    char tempstr[256];

    // ensure not too long and that there is an integer field
    if (strlen(text) > 200 || !strstr(text, "%d"))
        return;

    sprintf(tempstr, text, integer);
    fleetIntelligenceCreate(tempstr, 1);
}

//
//  new fleet-intelligence style objectives & popup
//
void kasfObjectiveCreate(char *label, char *briefText, char *fullText)
{
    if (fullText != NULL && fullText[0] != 0)
    {
        objectiveAndFleetIntelligenceCreate(label, briefText, fullText, 0, TRUE);
    }
    else if (tutorial == TUTORIAL_ONLY)
    {
        objectiveAndFleetIntelligenceCreate(label, briefText, "", 0, TRUE);
    }
    else
    {
        objectiveAndFleetIntelligenceCreate(label, briefText, briefText, 0, TRUE);
    }
}

void kasfObjectiveCreateSecondary(char* label, char* briefText, char* fullText)
{
    if ((fullText == NULL) || (fullText[0] == 0))
    {
    objectiveAndFleetIntelligenceCreate(label, briefText, briefText, 0, FALSE);
    }
    else
    {
    objectiveAndFleetIntelligenceCreate(label, briefText, fullText, 0, FALSE);
    }
}

void kasfObjectiveSet(char *label, sdword status)
{
    objectiveSet(label, status);
}

//void kasfObjectivePopupAll(void)
//{
//    objectivePopupAll();
//}

sdword kasfObjectiveGet(char *label)
{
    return objectiveGet(label);
}

sdword kasfObjectiveGetAll(void)
{
    return objectiveGetAll();
}

void kasfObjectiveDestroy(char *label)
{
    objectiveDestroy(label);
}

void kasfObjectiveDestroyAll(void)
{
    objectiveDestroyAll();
}


sdword kasfWideScreenIsActive(void)
{
    return kasfWideScreenActive;
}

sdword kasfShipsSelectEnemy(GrowSelection *newShips, GrowSelection *originalShips)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsEnemyPlayerOwned, 0, 0);
}

sdword kasfShipsSelectFriendly(GrowSelection *newShips, GrowSelection *originalShips)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsFriendlyPlayerOwned, 0, 0);
}

sdword kasfShipsSelectClass(GrowSelection *newShips, GrowSelection *originalShips, char *shipClass)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;
    ShipClass sc = StrToShipClass(shipClass);

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = isShipOfClass(ship, sc);  // class matches
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectType(GrowSelection *newShips, GrowSelection *originalShips, char *shipType)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;
    ShipType st = StrToShipType(shipType);

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        // start with empty new selection
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    // find all originalShips that satisfy the selection criteria to newShips
    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = (ship->shiptype == st);  // class matches
        if (!selected && sourceIsDest)
            // remove from source (dest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            // add to dest
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectCapital(GrowSelection *newShips, GrowSelection *originalShips)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsCapital, 1, 0);
}

sdword kasfShipsSelectNonCapital(GrowSelection *newShips, GrowSelection *originalShips)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsCapital, 0, 0);
}

sdword kasfShipsSelectNotDocked(GrowSelection *newShips, GrowSelection *originalShips)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipMatchesNotDockedFilter, 0, 0);
}

sdword kasfShipsSelectDamaged(GrowSelection *newShips, GrowSelection *originalShips, sdword maxHealthPercent)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipHealthIsAtMostPercent, maxHealthPercent, 0);
}

sdword kasfShipsSelectHealthAtLeastPercent(GrowSelection *newShips, GrowSelection *originalShips, sdword minHealthPercent)
{
    Ship *ship;
    sdword i, selected, sourceIsDest = FALSE;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    if (newShips == originalShips)
        sourceIsDest = TRUE;
    else
        while (newShips->selection->numShips)
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);

    for (i = 0; i < originalShips->selection->numShips; ++i)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = (ship->health >= minHealthPercent * ship->staticinfo->maxhealth / 100);
        if (!selected && sourceIsDest)
            clRemoveShipFromSelection(originalShips->selection, ship);
        else if (selected && !sourceIsDest)
            growSelectAddShip(newShips, ship);
    }

    return newShips->selection->numShips;
}

sdword kasfShipsSelectMoving(GrowSelection *newShips, GrowSelection *originalShips)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsMoving, 0, 0);
}

sdword kasfShipsSelectIndex(GrowSelection *newShips, GrowSelection *originalShips, sdword Index)
{
    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
        return 0;

    // if index is larger than the numships, or if numships is zero
    // return 0
    if ((originalShips->selection->numShips <= Index) ||
        (!originalShips->selection->numShips))
    {
        return 0;
    }

    kasGrowSelectionClear(newShips);
    growSelectAddShip(newShips, originalShips->selection->ShipPtr[Index]);

    return newShips->selection->numShips;
}


// selects the ships in originalShips that are within distance of location
sdword kasfShipsSelectNearby(GrowSelection *newShips, GrowSelection *originalShips, hvector *location, sdword distance)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipIsWithinDistanceOfPoint,
                                      (sdword)location, distance * distance);
}


//local function
/*-----------------------------------------------------------------------------
    Name        : kasfShipsMeetsCriteria
    Description : Returns TRUE if the ship meets the criteria determined by the CriteriaFlag
    Inputs      : ship - the ship being tested, CriteriaFlag - the criteria to test
    Outputs     :
    Return      : TRUE if the ship meets the criteria
----------------------------------------------------------------------------*/
bool32 kasfShipMeetsCriteria(ShipPtr ship, sdword CriteriaFlag)
{
    switch (CriteriaFlag)
    {
        case kasSpecial_ReturningTechnology:
            if (ShipTypeIsSalCapCorvetteType(ship->shiptype) &&
                (bitTest(ship->specialFlags, SPECIAL_SalvageTakingHomeTechnology)))
                return TRUE;
            break;
        case kasSpecial_ShipsDisabled:
            if (bitTest(ship->flags, SOF_Disabled))
                return TRUE;
            break;
        case kasSpecial_ShipsAreArmed:
            switch (ship->staticinfo->shipclass)
            {
                case CLASS_Fighter:
                case CLASS_HeavyCruiser:
                case CLASS_Carrier:
                case CLASS_Destroyer:
                    return TRUE;
                    break;
                case CLASS_Frigate:
                    switch (ship->shiptype)
                    {
                        case kCloakGenerator:
                        case tCloakGenerator:
                        case sProcessor:
                        case bProcessor:
                            return FALSE;
                        default:
                            return TRUE;
                    }
                    break;
                case CLASS_Corvette:
                    switch (ship->shiptype)
                    {
                        case sRepairDroid:
                        case bRepairDroid:
                        case kSalCapCorvette:
                        case tSalCapCorvette:
                            return FALSE;
                        default:
                            return TRUE;
                    }
                    break;
                default:
                    return FALSE;
            }
        case kasSpecial_ShipsDefected:
            if (bitTest(ship->attributes, ATTRIBUTES_Defector))
                return TRUE;
            break;
        default:
            return FALSE;
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : kasfShipsSelectSpecial
    Description : Selects the ships using different criteria depending on the
                  SpecialFlag setting
    Inputs      : newShips - pointer to growselection (pass by reference)
                  originalShips - the ships to filter
                  SpecialFlag - indicates the criteria
    Outputs     :
    Return      : The number of ships selected
----------------------------------------------------------------------------*/
sdword kasfShipsSelectSpecial(GrowSelection *newShips, GrowSelection *originalShips, sdword SpecialFlag)
{
    return kasfShipsSelectByPredicate(newShips, originalShips, kasShipMatchesSpecialSelectionFlag, SpecialFlag, 0);
}


//
//  A = A + B
//
sdword kasfShipsAdd(GrowSelection *shipsA, GrowSelection *shipsB)
{
    sdword i;

    if (shipsA == NULL || shipsA->selection == NULL ||
        shipsB == NULL || shipsB->selection == NULL)
        return 0;

    if (shipsA == shipsB)
        return shipsA->selection->numShips;

    for (i = 0; i < shipsB->selection->numShips; ++i)
        growSelectAddShipNoDuplication(shipsA, shipsB->selection->ShipPtr[i]);

    return shipsA->selection->numShips;
}

//
//  A = A - B
//
sdword kasfShipsRemove(GrowSelection *shipsA, GrowSelection *shipsB)
{
    sdword i;

    if (shipsA == NULL || shipsA->selection == NULL ||
        shipsB == NULL || shipsB->selection == NULL)
        return 0;

    if (shipsA == shipsB)
    {
        while (shipsA->selection->numShips)
            clRemoveShipFromSelection(shipsA->selection, shipsA->selection->ShipPtr[0]);
        return 0;
    }

    for (i = 0; i < shipsB->selection->numShips; ++i)
        growSelectRemoveShip(shipsA, shipsB->selection->ShipPtr[i]);
    return shipsA->selection->numShips;
}


//returns TRUE if all the selected ships are in the specified formation
sdword kasfSelectedShipsInFormation(sdword formation)
{
    sdword i;
    CommandToDo *command;

    if (!selSelected.numShips)
    {
        return FALSE;
    }

    for (i=0;i<selSelected.numShips;i++)
    {
        command = selSelected.ShipPtr[i]->command;
        if (command == NULL ||
            (command->ordertype.attributes & COMMAND_IS_FORMATION) == 0 ||
            command->formation.formationtype != (TypeOfFormation)formation)
        {
            return FALSE;
        }
    }

    return TRUE;
}

//returns TRUE if any of the ships are in the specific formation
sdword kasfShipsInFormation(GrowSelection *ships, sdword formation)
{
    sdword i;

    if (formation == -1)
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            if (ships->selection->ShipPtr[i]->formationcommand)
            {
                return TRUE;
            }
        }
    }
    else
    {
        for (i=0;i<ships->selection->numShips;i++)
        {
            if (ships->selection->ShipPtr[i]->formationcommand &&
                (ships->selection->ShipPtr[i]->formationcommand->formation.formationtype == (TypeOfFormation)formation))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//
// sets the ships so that the player's ships don't autoretaliate against them
//
void kasfShipsSetNonRetaliation(GrowSelection *ships)
{
    sdword i;

    for (i=0;i<ships->selection->numShips;i++)
    {
        bitSet(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_FriendlyStatus);
    }
}

//
// sets the ships so that the player's ships don't autoretaliate against them
//
void kasfShipsSetRetaliation(GrowSelection *ships)
{
    sdword i;

    for (i=0;i<ships->selection->numShips;i++)
    {
        bitClear(ships->selection->ShipPtr[i]->specialFlags, SPECIAL_FriendlyStatus);
    }
}

/*-----------------------------------------------------------------------------
    Name        : kasfPingAddSingleShip
    Description : Add a ping for a single ship, the first in the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfPingAddSingleShip(GrowSelection *ships, char *label)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->ShipPtr[0] == NULL)
        return;

    pingAnomalyObjectPingAdd(label, (SpaceObj *)(ships->selection->ShipPtr[0]));
}

/*-----------------------------------------------------------------------------
    Name        : kasfPingAddShips
    Description : Add a ping for a group of ships.  The ping will be centred
                    about the entire group of ships.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfPingAddShips(GrowSelection *ships, char *label)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->ShipPtr[0] == NULL)
        return;

    pingAnomalySelectionPingAdd(label, ships->selection);
}

void kasfPingAddPoint(hvector *point, char *label)
{
    pingAnomalyPositionPingAdd(label, (vector *)point);
}

void kasfPingRemove(char *label)
{
    pingAnomalyPingRemove(label);
}

void kasfStop(void)
{
    aitDeleteAllTeamMoves(CurrentTeamP);
    clWrapHalt(&universe.mainCommandLayer, CurrentTeamP->shipList.selection);
}

void kasfHarvest(void)
{
    MaxSelection tempSelection;
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    //aitDeleteAllTeamMoves(CurrentTeamP);

    if (selection->numShips > 0)
    {
        if (MakeShipsHarvestCapable((SelectCommand *)&tempSelection, selection, FALSE))
        {
            clWrapCollectResource(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,NULL);
        }
    }
}

void kasfBuildControl(sdword on)
{
    singlePlayerGameInfo.giveComputerFleetControl = on;
}

// sets the team as a building team for the KAS script
void kasfBuildingTeam(struct AITeam *builder)
{
    //for now, just set the first ship on the team (assume it only has one ship)
    aiCurrentAIPlayer->ScriptCreator = builder->shipList.selection->ShipPtr[0];
}

void kasfKamikaze(GrowSelection *targets)
{
    // alloc memory that the move can later free itself
    SelectCommand *dupeTargets = selectDupSelection(targets->selection);
    if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateKamikaze(CurrentTeamP, dupeTargets, SAME_FORMATION, FALSE, TRUE);
    }
}

void kasfKamikazeEveryone(GrowSelection *targets)
{
    AITeam *teamp;
    sdword i;

    if (targets->selection->numShips && targets->selection->ShipPtr[0] != NULL)
    {
        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            teamp = aiCurrentAIPlayer->teams[i];

            if (teamp->shipList.selection->numShips)
            {
                aitDeleteAllTeamMoves(teamp);
                aimCreateKamikaze(teamp, selectDupSelection(targets->selection), SAME_FORMATION, FALSE, TRUE);
            }
        }
    }

}

sdword kasfSpecialToggle(GrowSelection *ships)
{
    SelectCommand *selection;

    if (ships == NULL)
    {
        return 0;
    }

    selection = ships->selection;
    if (selection == NULL)
    {
        return 0;
    }

    clWrapSpecial(&universe.mainCommandLayer, selection, NULL);
    return 1;
}

void kasfShipsDamage(GrowSelection *ships, sdword points)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
        return;

    for (i = 0; i < ships->selection->numShips; ++i)
        ApplyDamageToTarget((struct SpaceObjRotImpTarg *)ships->selection->ShipPtr[i], (real32)points, 0, 0, 0);
}


void kasfForceTaskbar()
{
    kbInvokeCommandCallbacks(0, 71);
}


/*
----------------------------------------------------------------------------
 The following functions have been added to facilitate the tutorial mission.
----------------------------------------------------------------------------
*/

void kasfBuilderRestrictShipTypes(char *shipTypes)
{
    tutBuilderSetRestrictions(shipTypes, 1);
}

void kasfBuilderUnrestrictShipTypes(char *shipTypes)
{
    tutBuilderSetRestrictions(shipTypes, 0);
}

void kasfBuilderRestrictAll(void)
{
    tutBuilderRestrictAll();
}

void kasfBuilderRestrictNone(void)
{
    tutBuilderRestrictNone();
}

void kasfBuilderCloseIfOpen(void)
{
    cmCloseIfOpen();
}

// figure out how to do this someday....
void kasfForceBuildShipType(char *shipType)
{
    ShipType st;

    st = StrToShipType(shipType);
    cmForceBuildShipType(st);
//    clWrapBuildShip(&universe.mainCommandLayer,st,universe.curPlayerPtr->race,0,universe.curPlayerPtr->PlayerMothership);
}

// Range from 0 to 359 as universe moves from right to left
sdword kasfCameraGetAngleDeg(void)
{
    return (sdword)RAD_TO_DEG(currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.angle);
}

// Range from -90 (Above) to +90 (Below)
sdword kasfCameraGetDeclinationDeg(void)
{
    return (sdword)RAD_TO_DEG(currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.declination);
}

sdword kasfCameraGetDistance(void)
{
    return (sdword)currentCameraStackEntry(&universe.mainCameraCommand)->remembercam.distance;
}

sdword kasfSelectNumSelected(void)
{
    return selSelected.numShips;
}

sdword kasfSelectIsSelectionShipType(sdword Index, char *shipType)
{
ShipType    st;

    st = StrToShipType(shipType);
    return ((selSelected.numShips > Index) && selSelected.ShipPtr[Index]->shiptype == st);
}

sdword kasfSelectContainsShipTypes(char *shipTypes)
{
    return tutSelectedContainsShipTypes(shipTypes);
}

void kasfTutSetPointerTargetXY(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXY(name, x,y);
}

void kasfTutSetPointerTargetXYRight(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYRight(name, x,y);
}

void kasfTutSetPointerTargetXYBottomRight(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYBottomRight(name, x,y);
}

void kasfTutSetPointerTargetXYTaskbar(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYTaskbar(name, x,y);
}

void kasfTutSetPointerTargetXYFE(char *name, sdword x, sdword y)
{
    tutSetPointerTargetXYFE(name, x,y);
}

void kasfTutSetPointerTargetShip(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        tutSetPointerTargetShip(name, ships->selection->ShipPtr[0], 0);
    }
}

void kasfTutSetPointerTargetShipSelection(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        tutSetPointerTargetShipSelection(name, ships->selection, 1);
    }
}

void kasfTutSetPointerTargetShipHealth(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
        tutSetPointerTargetShipHealth(name, ships->selection->ShipPtr[0]);
}

void kasfTutSetPointerTargetShipGroup(char *name, GrowSelection *ships)
{
    if(ships->selection->numShips > 0)
    {
        tutSetPointerTargetShipGroup(name, ships->selection->ShipPtr[0]);
    }
}

void kasfTutSetPointerTargetFERegion(char *name, char *pAtomName)
{
    tutSetPointerTargetFERegion(name, pAtomName);
}

void kasfTutSetPointerTargetRect(char *name, sdword x0, sdword y0, sdword x1, sdword y1)
{
    tutSetPointerTargetRect(name, x0, y0, x1, y1);
}

void kasfTutSetPointerTargetAIVolume(char *name, Volume *volume)
{
    tutSetPointerTargetAIVolume(name, volume, 0);
}

void kasfTutRemovePointer(char *name)
{
    tutRemovePointerByName(name);
    if (!RGLtype && (cmConstructionManagerActive() || rmGUIActive))
    {
        if (MAIN_WindowWidth > 640)
        {
            rndClear();
        }
        byte_88F454 |= 1;
    }
}

void kasfTutRemoveAllPointers(void)
{
    tutRemoveAllPointers();
}

void kasfTutSetTextDisplayBoxGame(sdword x, sdword y, sdword width, sdword height)
{
    tutSetTextDisplayBox(x, y, width, height, TRUE);
}

void kasfTutSetTextDisplayBoxToSubtitleRegion(void)
{
    tutSetTextDisplayBoxToSubtitleRegion();
}
void kasfTutSetTextDisplayBoxFE(sdword x, sdword y, sdword width, sdword height)
{
    tutSetTextDisplayBox(x, y, width, height, FALSE);
}

void kasfTutShowText(char *szText)
{
    tutShowText(szText);
}

void kasfTutHideText(void)
{
    tutHideText();
}

void kasfTutShowNextButton(void)
{
    tutShowNextButton();
}

void kasfTutHideNextButton(void)
{
    tutHideNextButton();
}

sdword kasfTutNextButtonClicked(void)
{
    return tutNextButtonClicked();
}

void kasfTutShowBackButton(void)
{
    tutShowBackButton();
}

void kasfTutHideBackButton(void)
{
    tutHideBackButton();
}

void kasfTutShowPrevButton(void)
{
    tutShowPrevButtonRegion();
}

void kasfTutHidePrevButton(void)
{
    tutHidePrevButtonRegion();
}

sdword kasfTutPrevButtonClicked(void)
{
    return tutPrevButtonClicked();
}

void kasfTutSaveLesson(sdword Num, char *pName)
{
    tutSaveLesson(Num, pName);
}

void kasfTutShowImages(char *szImages)
{
    tutShowImages(szImages);
}

void kasfTutHideImages(void)
{
    tutHideImages();
}

void kasfTutEnableEverything(void)
{
    tutEnableEverything();
}

void kasfTutDisableEverything(void)
{
    tutDisableEverything();
}

void kasfTutEnableFlags(char *pFlags)
{
    tutSetEnableFlags(pFlags, 1);
}

void kasfTutDisableFlags(char *pFlags)
{
    tutSetEnableFlags(pFlags, 0);
}

void kasfTutForceUnpaused(void)
{
    universePause = FALSE;
}

sdword kasfTutGameSentMessage(char *commandNames)
{
    return tutGameSentMessage(commandNames);
}

void kasfTutResetGameMessageQueue(void)
{
    tutResetGameMessageQueue();
}

sdword kasfTutContextMenuDisplayedForShipType(char *shipType)
{
    return tutContextMenuDisplayedForShipType(shipType);
}

void kasfTutResetContextMenuShipTypeTest(void)
{
    tutResetContextMenuShipTypeTest();
}

void kasfTutRedrawEverything(void)
{
    bitSet(regRootRegion.status, RSF_ReallyDirty);
}

sdword kasfSubtitleDraw(void)
{
    sdword index;

    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].bEnabled && subRegion[index].cardIndex > 0)
        {
            subTimeElapsed = &universe.totaltimeelapsed;
            subTitlesDraw(&subRegion[index]);
        }
    }

    if (!RGLtype && (cmConstructionManagerActive() || rmGUIActive))
    {
        if (MAIN_WindowWidth > 640)
        {
            rndClear();
        }
        byte_88F454 |= 1;
    }
    return 0;
}

sdword kasfBuildManagerShipTypeInBatchQueue(char *shipType)
{
    return tutBuildManagerShipTypeInBatchQueue(shipType);
}

sdword kasfBuildManagerShipTypeInBuildQueue(char *shipType)
{
    return cmCurrentPlayerShipInProgressByType(shipType);
}

sdword kasfBuildManagerShipTypeSelected(char *shipType)
{
    return tutBuildManagerShipTypeSelected(shipType);
}

sdword kasfTutCameraFocusedOnShipType(char *shipTypes)
{
    return tutCameraFocusedOnShipType(shipTypes);
}

void kasfTutCameraFocus(GrowSelection *ships)
{
    MaxSelection tempselected;

    if (ships->selection->numShips <= 0)
    {
        tempselected = selSelected;
        ccFocus(&universe.mainCameraCommand, (FocusCommand *)&tempselected);
    }
    else
    {
        kasfTutCameraFocusActive = TRUE;
        if (piePointSpecMode)
        {
            piePointModeOnOff();
        }
        FancyFightPreLoad(TRUE);
        ccFocus(&universe.mainCameraCommand, (FocusCommand *)ships->selection);
    }
}

void kasfTutCameraFocusDerelictType(char *derelictType)
{
    Derelict *derelict;
    Node *node;
    DerelictSelection focus;
    DerelictType dt = StrToDerelictType(derelictType);

    focus.numDerelicts = 1;

    // go through the entire list of derelicts
    node = universe.DerelictList.head;
    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if (derelict->derelicttype == dt)
        {
            kasfTutCameraFocusActive = TRUE;
            if (piePointSpecMode)
            {
                piePointModeOnOff();
            }
            FancyFightPreLoad(TRUE);
            focus.DerelictPtr[0] = derelict;
            ccFocus(&universe.mainCameraCommand, (FocusCommand *)&focus);
            return;
        }
        node = node->next;
    }
}

void kasfTutCameraFocusFar(GrowSelection *ships)
{
    MaxSelection tempselected;

    if (ships->selection->numShips <= 0)
    {
        tempselected = selSelected;
        ccFocusFar(&universe.mainCameraCommand, (FocusCommand *)&tempselected, &universe.mainCameraCommand.actualcamera);
    }
    else
    {
        FancyFightPreLoad(TRUE);
        kasfTutCameraFocusActive = TRUE;
        ccFocusFar(&universe.mainCameraCommand, (FocusCommand *)ships->selection, &universe.mainCameraCommand.actualcamera);
    }
}


void kasfTutCameraFocusCancel(void)
{
    kasfTutCameraFocusActive = FALSE;
    ccCancelFocus(&universe.mainCameraCommand);
}


/*-----------------------------------------------------------------------------
    Name        : kasfDisablePlayer
    Description : Disables player commands
    Inputs      : toggle - if TRUE the player's commands gets disabled,
                           if FALSE, the player's commands get enabled
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfDisablePlayer(bool32 toggle)
{
    if (toggle)
    {
        mrDisable();
    }
    else
    {
        mrEnable();
    }
}


sdword kasfTutShipsInView(GrowSelection *ships)
{
    sdword i;
    udword inview = 0;

    for (i=0;i<ships->selection->numShips;i++)
    {
        if (rndShipVisible((SpaceObj *)ships->selection->ShipPtr[i], mrCamera))
        {
            inview++;
        }
    }

    return inview;
}

sdword kasfTutShipsTactics(GrowSelection *ships)
{
    if (ships->selection->numShips)
        return ships->selection->ShipPtr[0]->tacticstype;
    else
        return -1;
}

sdword kasfTutPieDistance(void)
{
    extern real32 pieDistance;

    return (sdword)pieDistance;
}

sdword kasfTutPieHeight(void)
{
    extern real32 pieHeight;

    return (sdword)pieHeight;
}

void kasfForceFISensors(void)
{
    smFleetIntel = TRUE;
}

void kasfOpenSensors(sdword flag)
{
    universeTurbo = FALSE;
    if (flag > 1)
    {
        kasStoreSMInitialDistance = (sdword)smInitialDistance;
        smFleetIntel = TRUE;
        smInitialDistance = (real32)flag;
    }
    else
    {
        smFleetIntel = flag;
    }

    FancyFightPreLoad(FALSE);
    smSensorsForceOpen = TRUE;
    smSensorsBegin(NULL, NULL);
    smSensorsForceOpen = FALSE;
}

void kasfCloseSensors(sdword flag)
{
    /*
#ifdef gshaw
    if (smFleetIntel != TRUE)
    {
        return;
    }
#endif
    dbgAssert(smFleetIntel==TRUE);
    */
    if ((smSensorsActive) && (!smZoomingIn) && (!smZoomingOut))
    {
        if (kasStoreSMInitialDistance)
        {
            //restore the sensors manager zoom length
            smInitialDistance    = (real32)kasStoreSMInitialDistance;
            kasStoreSMInitialDistance = 0;
        }

        //flags don't do anything yet
        smSensorsClose(NULL, NULL);
    }
    smFleetIntel = FALSE;
}

sdword kasfSensorsIsOpen(sdword flag)
{
    if (flag)
    {
        //returns true if the fleet intel window is open
        return smFleetIntel;
    }
    else
    {
        //returns true if the player is in the sensors manager but not fleet intel
        return ((smSensorsActive || smZoomingIn || smZoomingOut) && !smFleetIntel);
    }
}

void kasfSensorsWeirdness(sdword value)
{
//    static nisstatic stat2;
//    static nisstatic stat3;

    smSensorWeirdness = value;

/*    if (flag)
    {
        stat1.index = 0;

        nisStaticOnExp(&stat1);
//        nisStaticOnExp(&stat2);
//        nisStaticOnExp(&stat3);
        smSensorWeirdnessInit();
    }
    else
    {
        nisStaticOffExp(0);
//        nisStaticOffExp(1);
//        nisStaticOffExp(2);
    }
*/
}

void kasfAllowPlayerToResearch(char *name)
{
    AllowPlayerToResearch(name);
}

void kasfAllowPlayerToPurchase(char *name)
{
    AllowPlayerToPurchase(name);
}

void kasfPlayerAcquiredTechnology(char *name)
{
    PlayerAcquiredTechnology(name);
}

sdword kasfCanPlayerResearch(char *name)
{
    return CanPlayerResearch(name);
}

sdword kasfCanPlayerPurchase(char *name)
{
    return CanPlayerPurchase(name);
}

sdword kasfDoesPlayerHave(char *name)
{
    return DoesPlayerHave(name);
}

void kasfSetBaseTechnologyCost(char *name, sdword cost)
{
    SetBaseTechnologyCost(name, cost);
}

sdword kasfGetBaseTechnologyCost(char *name)
{
    return GetBaseTechnologyCost(name);
}

sdword kasfTechIsResearching(void)
{
    return rmResearchingAnything(&universe.players[0]);
}

void kasfEnableTraderGUI(void)
{
    tmEnableTraderGUI();
}

sdword kasfTraderGUIActive(void)
{
    return tmTraderGUIActive();
}

void kasfSetTraderDialog(sdword dialogNum, char *text)
{
    tmSetDialog(dialogNum, text);
}

void kasfSetTraderPriceScale(sdword percent)
{
    tmSetPriceScale(percent);
}

void kasfSetTraderDisabled(sdword disable)
{
    tmSetTradeDisabled(disable);
}

sdword kasfGetTraderPriceScale(void)
{
    return tmGetPriceScale();
}

sdword kasfRUsGet(sdword player)
{
    return universe.players[player].resourceUnits;
}

void kasfRUsSet(sdword player, sdword RUs)
{
    universe.players[player].resourceUnits = RUs;
}

sdword kasfGetWorldResources(void)
{
    sdword total, numHarvestables, numAsteroid0s;

    univGetResourceStatistics(&total, &numHarvestables, &numAsteroid0s);

    return total;
}

void kasfSoundEvent(sdword event)
{
    soundEventPlay(NULL, event, NULL);
}

void kasfSoundEventShips(GrowSelection *ships, sdword event)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->numShips == 0)
        return;

    soundEventPlay(ships->selection->ShipPtr[0], event, NULL);
}

void kasfSpeechEvent(sdword event, sdword initFn, sdword watchFn)
{
    ((sdword *)kasCurrentTeamPtr)[24] = event;
    ((AITeam *)kasCurrentTeamPtr)->kasStateWatchFunction = (KASWatchFunction)watchFn;
    ((void (*)(void))initFn)();
}

void kasfSpeechEventQueueCurrentPlayer(sdword event, sdword variable)
{
    subMessageEnded = 0;
    linkto = -1;
    linkto = speechEventQueue(NULL, event, variable, -1, -1, universe.curPlayerIndex, -1, -1.0f, -1);

}

void kasfSpeechEventQueueCurrentPlayerLinked(sdword event, sdword variable, sdword resetLink)
{
    sdword handle;

    subMessageEnded = 0;

    if (resetLink)
    {
        linkto = -1;
    }

    handle = speechEventQueue(NULL, event, variable, -1, -1, universe.curPlayerIndex, linkto, -1.0f, -1);
    if (!resetLink && handle == -1)
    {
        handle = speechEventQueue(NULL, event, variable, -1, -1, universe.curPlayerIndex, -1, -1.0f, -1);
    }
    linkto = handle;
}

void kasfSpeechEventShips(GrowSelection *ships, sdword event, sdword variable)
{
    if (ships == NULL || ships->selection == NULL || ships->selection->numShips == 0)
        return;

    speechEventQueue((sdword)ships->selection->ShipPtr[0], event, variable, -1, -1, -1, -1, 0.0f, -1);
}

void kasfToggleActor(sdword Actor, sdword on)
{
    soundEventSetActorFlag(Actor, on);
}

void kasfMusicPlay(sdword trackNum)
{
    soundEventPlayMusic((udword)trackNum);
}

void kasfMusicStop(sdword fadeTime)
{
    soundEventStopMusic((real32)fadeTime);
}

sdword kasfRenderedShips(GrowSelection *ships, sdword LOD)
{
    sdword i;
    ubyte LODmask;

    for (LODmask = 0, i = 8; i > 0; i--)
    {                                                       //prepare a mask for the given LOD
        if (i <= LOD)
        {
            LODmask |= 1;
        }
        LODmask <<= 1;
    }

    if (ships == NULL || ships->selection == NULL)
        return 0;

    for (i = 0; i < ships->selection->numShips; ++i)        //see if ship rendered at the specified LOD or lower
    {
        if (bitTest(ships->selection->ShipPtr[i]->renderedLODs, LODmask))
        {
            return 1;
        }
    }

    return 0;
}

void kasfResetShipRenderFlags(GrowSelection *ships)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
        return;

    for (i=0; i < ships->selection->numShips; i++)
        ships->selection->ShipPtr[i]->renderedLODs = 0;
}

sdword kasfRenderedDerelicts(char *derelictType, sdword LOD)
{
    Derelict *derelict;
    Node *node;
    DerelictType dt = StrToDerelictType(derelictType);

    // go through the entire list of derelicts
    node = universe.DerelictList.head;
    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if ((derelict->derelicttype == dt) &&
            bitTest(derelict->renderedLODs, 1 << LOD))
            return 1;
        node = node->next;
    }
    return 0;
}

void kasfResetDerelictRenderFlags(char *derelictType)
{
    Derelict *derelict;
    Node *node;
    DerelictType dt = StrToDerelictType(derelictType);

    // go through the entire list of derelicts
    node = universe.DerelictList.head;
    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if (derelict->derelicttype == dt)
        {
            derelict->renderedLODs = 0;
            break;
        }
        node = node->next;
    }
}

void kasfGateMoveToNearest(void)
{
    real32 temp;
    real32 closestDistSquared = -1;
    sdword i;
    vector loc;
    vector shippos;
    SelectCommand *selection = CurrentTeamP->shipList.selection;
    Derelict *gate = NULL;
    Derelict *closest = NULL;

    if (selection == NULL || !selection->numShips)
        return;

    shippos = selection->ShipPtr[0]->posinfo.position;

    for (i = 0; i < LabelledVectorsUsed; ++i)
    {
        if (!strnicmp(LabelledVectors[i]->label, "GATE", 4))
        {
            vecGrabVecFromHVec(loc, *(LabelledVectors[i]->hvector));
            temp = aiuFindDistanceSquared(shippos,loc);

            if (closestDistSquared == -1 || temp <= closestDistSquared)
            {
                if ((gate = GetHyperspaceGateFromVector(&loc)) != NULL)
                {
                    closestDistSquared = temp;
                    closest = gate;
                }
            }
        }
    }

    if (closestDistSquared != -1 && closest)
    {
        // we now have a pointer to the hyperspace gate derelict
        vector shipToGate;
        vector gateNormal;
        vector waypoint;
        vector gatePosition;

        gatePosition = closest->posinfo.position;

        vecSub(shipToGate,gatePosition,shippos);
        matGetVectFromMatrixCol3(gateNormal,closest->rotinfo.coordsys);

        if (vecDotProduct(shipToGate,gateNormal) > 0.0f)
        {
            // shipToGate and gateNormal are pointing in same direction...pick back direction;
            vecNegate(gateNormal);
        }

        vecMultiplyByScalar(gateNormal,HYPERSPACEGATE_WAYPOINTDIST);
        vecAdd(waypoint,gateNormal,gatePosition);

        aitDeleteAllTeamMoves(CurrentTeamP);
        aimCreateMoveTeam(CurrentTeamP, waypoint, SAME_FORMATION, TRUE, TRUE);
        aimCreateMoveTeam(CurrentTeamP, gatePosition, SAME_FORMATION, TRUE, TRUE);
    }
}

void kasfGateDestroy(hvector *gatePoint)
{
    //hyperspace
    hsStaticDestroy(gatePoint);
}

void kasfGateShipsIn(GrowSelection *ships, hvector *gatePoint)
{
    //hyperspace
    spHyperspaceSelectionInStatic(ships->selection, gatePoint);
}

void kasfGateShipsOut(GrowSelection *ships, hvector *gatePoint)
{
    //hyperspace
    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,ships->selection);
    spHyperspaceSelectionOutStatic(ships->selection);
}


sdword kasfGateShipsOutNearest(GrowSelection *ships)
{
    hvector *closest = NULL;
    real32 temp;
    real32 closestDistSquared = -1;
    sdword i;
    vector loc;
    SelectCommand *selection;

    selection = ships->selection;

    if (selection == NULL || !selection->numShips)
        return 0;

    for (i = 0; i < LabelledVectorsUsed; ++i)
    {
        if (!strnicmp(LabelledVectors[i]->label, "GATE", 4))
        {
            vecGrabVecFromHVec(loc, *(LabelledVectors[i]->hvector));
            temp = aiuFindDistanceSquared(
                        selection->ShipPtr[0]->posinfo.position,
                        loc);
            if (closestDistSquared == -1 || temp <= closestDistSquared)
            {
                closestDistSquared = temp;
                closest = LabelledVectors[i]->hvector;
            }
        }
    }

    if (closestDistSquared != -1 && closest)
    {
        if (closestDistSquared < 4000000)
        {
            RemoveShipsFromDoingStuff(&universe.mainCommandLayer,selection);
            spHyperspaceSelectionOutStatic(selection);
            return TRUE;
        }
    }
    return FALSE;
}

//creates a list of the player's currently selected ships
//added for the tutorial
udword kasfFindSelectedShips(GrowSelection *ships)
{
    sdword i;

    if (ships == NULL)
        return 0;

    while (ships->selection->numShips)
        clRemoveShipFromSelection(ships->selection, ships->selection->ShipPtr[0]);

    for (i=0;i<selSelected.numShips;i++)
        growSelectAddShip(ships, selSelected.ShipPtr[i]);

    return selSelected.numShips;
}

//
//  returns how many ships in the selection are disabled
//
sdword kasfShipsDisabled(GrowSelection *ships)
{
    sdword i;
    sdword count = 0;

    if (ships == NULL)
        return 0;

    for (i = 0; i < ships->selection->numShips; ++i)
    {
        if (bitTest(ships->selection->ShipPtr[i]->flags, SOF_Disabled))
            ++count;
    }

    return count;
}

//saves a single player level
void kasfSaveLevel(sdword LevelNum, char *LevelName)
{
//    char levelName[256];

//LM:Why is this here?  It trips an assertion...    btgSetColourMultiplier(-1.0f);

    collUpdateCollBlobs();
    collUpdateObjsInCollBlobs();

    sprintf(CurrentLevelName, "%s%s", SinglePlayerSavedGamesPath, LevelName);
    if (SaveGame(CurrentLevelName))
        clCommandMessage(strGetString(strSavedGame), -1);
    else
        clCommandMessage(strGetString(strPatchUnableWriteFile));
}

/*-----------------------------------------------------------------------------
    Name        : kasfClearScreen
    Description : Refreshes the screen
    Inputs      : none
    Outputs     : the player's monitor feels a clearing sensation
    Return      : void
----------------------------------------------------------------------------*/
void kasfClearScreen(void)
{
    rndClear();
}


//starts the NIS widescreen thing under KAS control
void kasfWideScreenIn(sdword frames)
{
    universeTurbo = FALSE;
    turboTimeCompressionFactor = (ubyte)taskTimeCompressionDefaultGet();
    taskTimeCompressionApply();
    taskTimeCompressionDisplay();
    if (!nisWideScreenActive)
    {
        if (nisScissorFadeOut != 0.0f)
        {
            nisScissorFadeTime = 0.0f;
            nisScissorFadeOut = 0.0f;
            nisScissorFade = 0.0f;
        }
        soundEventPlay(NULL, 291, NULL);
        nisScissorFadeIn = (real32)frames / NIS_FrameRate;
        nisTaskResume();
    }
    kasfWideScreenPreserveMainScreenLockout = 0;
    kasfWideScreenActive = 1;
    if ((universe.mainCameraCommand.ccMode & 8) != 0)
    {
        universe.mainCameraCommand.ccMode &= ~8u;
    }
    singlePlayerSetWideScreenSpeedScale(TRUE);
}

//ends the NIS widescreen thing under KAS control
void kasfWideScreenOut(sdword frames)
{
    if (nisScissorFadeIn != 0.0f)
    {
        nisScissorFadeTime = 0.0f;
        nisScissorFadeIn = 0.0f;
        nisScissorFade = 0.0f;
    }
    soundEventPlay(NULL, 292, NULL);
    nisScissorFadeOut = (real32)frames / NIS_FrameRate;
    nisTaskResume();
    spClearScreenLockout();
    singlePlayerSetWideScreenSpeedScale(FALSE);
    mouseCursorShow();
}

//simulate a subtitle
static sdword kasfDummyEventNumber = 0;         //incremented to ensure uniquie event numbers
void kasfSubtitleSimulate(sdword actor, sdword milliseconds, char *speech)
{
    subTitleAdd(actor, kasfDummyEventNumber, speech, strlen(speech), (real32)milliseconds / 1000.0f);
    subMessageEnded = 0;
    kasfDummyEventNumber++;
}

//display a location card
void kasfLocationCard(sdword milliseconds, char *location)
{
    char buffer[SUB_SubtitleLength];

    _snprintf(buffer, SUB_SubtitleLength - 1, "#r%x#t%x%s", STR_LocationCard, STT_LocationCard, location);
    subTitleAdd(STA_LocationCard, kasfDummyEventNumber, buffer, strlen(buffer), (real32)milliseconds / 1000.0f);
    subMessageEnded = 0;
    kasfDummyEventNumber++;
}

//display easter egg location card
void kasfLocationCardSpecial(sdword milliseconds, char *string)
{
//    char buffer[SUB_SubtitleLength];

    dbgAssert(milliseconds > 10);
    subTitleAdd(STA_LocationCard, kasfDummyEventNumber, string, strlen(string), (real32)milliseconds / 1000.0f);
    kasfDummyEventNumber++;
}

sdword kasfRaceOfHuman(void)
{
    return universe.players[0].race;
}

//hides some ships and remembers which ones were already hidden so the NIS can restore them
void kasfHideShips(GrowSelection *ships)
{
    sdword index;

    for (index = 0; index < ships->selection->numShips; index++)
    {
        if (bitTest(ships->selection->ShipPtr[index]->flags, SOF_Hide))
        {                                                   //if ship already hidden
            bitSet(ships->selection->ShipPtr[index]->flags, SOF_PreviouslyHiddenByKAS);
        }
        else
        {
            univRemoveObjFromRenderList((SpaceObj *)ships->selection->ShipPtr[index]);
            bitSet(ships->selection->ShipPtr[index]->flags, SOF_Hide);

        }
    }
}

//unhides some ships and preserves ships that were already hidden before kasfHideShips
void kasfUnhideShips(GrowSelection *ships)
{
    sdword index;

    for (index = 0; index < ships->selection->numShips; index++)
    {
        if (bitTest(ships->selection->ShipPtr[index]->flags, SOF_PreviouslyHiddenByKAS))
        {                                                   //if ship was hidden to begin with
            bitClear(ships->selection->ShipPtr[index]->flags, SOF_PreviouslyHiddenByKAS);
        }
        else
        {
            bitClear(ships->selection->ShipPtr[index]->flags, SOF_Hide);

        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : kasfDeleteShips
    Description : Deletes a select of ships from the game
    Inputs      : ships - the ships to delete
    Outputs     : wipes the ships from the game
    Return      : void
----------------------------------------------------------------------------*/
void kasfDeleteShips(GrowSelection *ships)
{
    Ship *ship;
    sdword i;

    while (ships->selection->numShips)
    {
        ship = ships->selection->ShipPtr[0];

        if (shipHasModuleSlotState(ship))
        {
            udword *moduleSlots;
            sdword slotIndex;

            if ((ship->flags2 & 0x08000000) == 0)
            {
                *(ubyte *)&shipModuleSlotRuntime(ship)->state = 0;
                shipModuleSlotRuntime(ship)->transitionArmed = 0;

                if (shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetBindings != NULL)
                {
                    memFree(shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetBindings);
                    shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetBindings = NULL;
                }
                if (shipModuleSlotRuntime(ship)->pendingBuildTarget.buildTargetBindings != NULL)
                {
                    memFree(shipModuleSlotRuntime(ship)->pendingBuildTarget.buildTargetBindings);
                    shipModuleSlotRuntime(ship)->pendingBuildTarget.buildTargetBindings = NULL;
                }
                if (shipModuleSlotRuntime(ship)->activeEffect != NULL)
                {
                    univReallyDeleteThisResourceRightNow(shipModuleSlotRuntime(ship)->activeEffect);
                    shipModuleSlotRuntime(ship)->activeEffect = NULL;
                }

                moduleSlots = shipModuleSlotRuntime(ship)->slotAvailability;
                for (slotIndex = 0; slotIndex < 8; slotIndex++)
                {
                    moduleSlots[slotIndex] = 1;
                }
            }
        }

        univPingShipDied((sdword)ship);

        if (ship->beastAttack.effect != NULL)
        {
            memFree(ship->beastAttack.effect);
            ship->beastAttack.effect = NULL;
        }
        if (ship->replacementEffect != NULL)
        {
            memFree(ship->replacementEffect);
            ship->replacementEffect = NULL;
        }

        if (ship->shiplink.belongto != NULL)
        {
            listRemoveNode(&ship->shiplink);
        }
        if (ship->impactablelink.belongto != NULL)
        {
            listRemoveNode(&ship->impactablelink);
        }

        nisObjectDied((SpaceObj *)ship);
        etgShipDied(ship);
        univRemoveObjFromRenderList((SpaceObj *)ship);
        univFreeShipContents(ship);

        if (ship->objlink.belongto != NULL)
        {
            listDeleteNode(&ship->objlink);
        }

        ccRemoveTheseShips(&universe.mainCameraCommand, (SelectCommand *)ship);
        clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)ship);
        clRemoveTargetFromSelection((SelectAnyCommand *)&attackcom, (TargetPtr)ship);
        ioUpdateShipTotals();
        smFocusRemoveObj((SpaceObj *)ship);

        for (i = 0; i < universe.numPlayers; i++)
        {
            if (universe.players[i].PlayerMothership == ship)
            {
                universe.players[i].PlayerMothership = NULL;
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : kasfRotateDerelictType
    Description : Rotates a type of derelict
    Inputs      : derelictType - string of derelict type
                  rot_x - rotation speed around x axis  (*1000)
                  rot_y - rotation speed around y axis  (*1000)
                  rot_z - rotation speed around z axis  (*1000)
                  variation - amount of random rotation variation
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void kasfRotateDerelictType(char *derelictType, sdword rot_x, sdword rot_y, sdword rot_z, sdword variation)
{
    Derelict *derelict;
    Node *node;
    DerelictType dt = StrToDerelictType(derelictType);
    real32 realrot_x     = (real32)rot_x,
           realrot_y     = (real32)rot_y,
           realrot_z     = (real32)rot_z,
           realvariation = (real32)variation;

    // scale rotation values to their proper values
    realrot_x     /= 1000;
    realrot_y     /= 1000;
    realrot_z     /= 1000;
    realvariation /= 1000;

    //go through the entire list of derelicts and
    //give the appropriate ones a rotation
    node = universe.DerelictList.head;

    while (node != NULL)
    {
        derelict = (Derelict *)listGetStructOfNode(node);
        if (derelict->derelicttype == dt)
        {
            derelict->rotinfo.rotspeed.x = realrot_x + frandyrandombetween(RAN_AIPlayer, -realvariation, realvariation);
            derelict->rotinfo.rotspeed.y = realrot_y + frandyrandombetween(RAN_AIPlayer, -realvariation, realvariation);
            derelict->rotinfo.rotspeed.z = realrot_z + frandyrandombetween(RAN_AIPlayer, -realvariation, realvariation);
        }
        node = node->next;
    }
}


/*-----------------------------------------------------------------------------
    Name        : kasfUniversePause
    Description : Pause all ships in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfUniversePause(void)
{
    nisUniversePause = TRUE;
#if NIS_VERBOSE_LEVEL >= 1
    dbgMessage("\nKAS has PAUSED the universe.");
#endif
    //!!! pause KAS timers and FSM's
}

/*-----------------------------------------------------------------------------
    Name        : kasfUniverseUnpause
    Description : Pause all ships in the universe
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfUniverseUnpause(void)
{
    nisUniversePause = FALSE;
    //!!! unpause KAS timers and FSM's
}

// pause the build

void kasfPauseBuildShips(void)
{
    cmPauseAllJbos();
}

void kasfUnpauseBuildShips(void)
{
    cmUnPauseAllJobs();
}


/*-----------------------------------------------------------------------------
    Name        : kasfOtherKASPause/kasfOtherKASUnpause
    Description : Pause/unPause global KAS watches and timers not associated with
                    this FSM.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfOtherKASPause(void)
{
    kasUnpausedTeam = kasfScriptTeam();
    kasTimerPauseAllNotScoped(kasCurrentScopeHash);
}
void kasfOtherKASUnpause(void)
{
    kasUnpausedTeam = NULL;
    kasTimerUnpauseAll();
}

/*-----------------------------------------------------------------------------
    Name        : kasfIntelEventEnded
    Description : Returns TRUE only once if a fleet intel/command subtitled
                    speech event ends or the player hits SPACE to short-cut
                    the event.
    Inputs      :
    Outputs     :
    Return      : 1 if the current fleet intel subtitle expired, 2 if the user
                    hit space in the sensors manager.
----------------------------------------------------------------------------*/
sdword kasfIntelEventEnded(void)
{
    sdword returnValue = subMessageEnded;
    bool32 cardsOnScreen;
    subMessageEnded = 0;                                   //only return TRUE once for each event

    // safety to make sure eventualy intel event ends
    if (!returnValue)
    {
        cardsOnScreen = subAnyCardsOnScreen();
        if (subMessageReturnedFalseTime == 0.0f)
        {
            subMessageReturnedFalseTime = universe.totaltimeelapsed;
        }
        else
        {
            if ( ((universe.totaltimeelapsed - subMessageReturnedFalseTime) > SUBMESSAGE_SAFETY_TIMEOUT) &&
                 (!cardsOnScreen) )
            {
                subMessageReturnedFalseTime = 0.0f;
                return 1;           // subtitle finished by timeout
            }
        }
    }
    else
    {
        subMessageReturnedFalseTime = 0.0f;
    }

    return(returnValue);
}

/*-----------------------------------------------------------------------------
    Name        : kasfIntelEventNotEnded
    Description : Clears the fleet intel event-ended flag.  Should be called at the
                    init of any FSM that will use "IntelEventEnded"
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfIntelEventNotEnded(void)
{
    subMessageEnded = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : kasfForceIntelEventEnded
    Description : Forces the fleet intel event-ended flag.  Should be called at the
                    init of any FSM that will use "IntelEventEnded"
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfForceIntelEventEnded(void)
{
    subMessageEnded = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : kasfSensorsStaticOn
    Description : Turns on sensors manager static
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfSensorsStaticOn(void)
{
    static nisstatic newStatic;                             //needs to be persistant

    newStatic.index = NIS_SMStaticIndex;
    //!!! make these values tweakable (see nisStaticOnSet for an example)
    newStatic.nLines = 50;
    newStatic.nLinesVariation = 5;
    newStatic.width = (real32)(((real64)10.0f / (real64)MAIN_WindowWidth)
        + ((real64)10.0f / (real64)MAIN_WindowWidth));
    newStatic.widthVariation = (real32)(((real64)1.0f / (real64)MAIN_WindowWidth)
        + ((real64)1.0f / (real64)MAIN_WindowWidth));
    newStatic.y = (real32)(1.0
        - (((real64)(MAIN_WindowHeight / 2) + (real64)0.325f) / (real64)MAIN_WindowHeight
            + ((real64)(MAIN_WindowHeight / 2) + (real64)0.325f) / (real64)MAIN_WindowHeight));
    newStatic.yVariation = (real32)(((real64)(MAIN_WindowHeight / 2) / (real64)MAIN_WindowHeight)
        + ((real64)(MAIN_WindowHeight / 2) / (real64)MAIN_WindowHeight));
    newStatic.hue = 0.5f;
    newStatic.hueVariation = 0.5f;
    newStatic.lum = 0.9f;
    newStatic.lumVariation = 0.1f;
    newStatic.sat = 0.5f;
    newStatic.satVariation = 0.1f;
    newStatic.alpha = 0.6f;
    newStatic.alphaVariation = 0.1f;
    newStatic.bAlpha = TRUE;
    newStatic.bTextured = FALSE;
    newStatic.texture = TR_InvalidInternalHandle;

    nisStaticOnExp(&newStatic);
}

/*-----------------------------------------------------------------------------
    Name        : kasfSensorsStaticOff
    Description : Turns off static in the sensors manager.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfSensorsStaticOff(void)
{
    nisStaticOffExp(NIS_SMStaticIndex);
}

/*-----------------------------------------------------------------------------
    Name        : kasfGameEnd
    Description : Quit the game and play the exit plugscreens.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void kasfGameEnd(void)
{
// this stuff only happens during demos
#if defined(CGW) || defined (Downloadable) || defined(OEM)
    // quit the game here
    universe.quittime = universe.totaltimeelapsed;
    utyPlugScreens = TRUE;
    //spMainScreen();
    //utyGameQuitToPlugScreens(NULL,NULL);
#endif
}

/*-----------------------------------------------------------------------------
    Name        : kasfSpawnEffect
    Description : Spawns an effect with a single float parameter
    Inputs      : ships - ship(s) to spawn the effect on.
                  effectName - name of the effect
                  parameter - parameter to pass in (to be converted to a float)
    Outputs     :
    Return      :
    Note        : the first parameter to the effect will be the size of the ship.
----------------------------------------------------------------------------*/
void kasfSpawnEffect(GrowSelection *ships, char *effectName, sdword parameter)
{
    udword index, count;
    static char *lastName = NULL;
    static etgeffectstatic *lastEffect;
    Ship **shipPtr;

    //retain name so this lookup does not have ot happen every single time
    if (effectName != lastName)
    {
        lastName = effectName;
        lastEffect = etgEffectStaticFind(effectName, FALSE);
#if ETG_ERROR_CHECKING
        if (lastEffect == NULL)
        {
            dbgFatalf(DBG_Loc, "Effect '%s' not found for KAS execution", effectName);
        }
#endif
    }
    (void)parameter;
    count = ships->selection->numShips;
    shipPtr = ships->selection->ShipPtr;
    for (index = 0; index < count; index++, shipPtr++)
    {
        etgEffectCreate(lastEffect, *shipPtr, NULL, NULL, NULL, 0.0f, EAF_Full, 2);
    }
}


sdword kasfShipsSelectFirstN(GrowSelection *newShips, GrowSelection *originalShips, sdword limit)
{
    sdword i;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL)
    {
        return 0;
    }

    if (limit < 0)
    {
        limit = 0;
    }
    if (limit > originalShips->selection->numShips)
    {
        limit = originalShips->selection->numShips;
    }

    if (newShips == originalShips)
    {
        while (newShips->selection->numShips > limit)
        {
            growSelectRemoveShip(newShips, newShips->selection->ShipPtr[newShips->selection->numShips - 1]);
        }
        return newShips->selection->numShips;
    }

    kasfGrowSelectionClear(newShips);
    for (i = 0; i < limit; i++)
    {
        growSelectAddShip(newShips, originalShips->selection->ShipPtr[i]);
    }
    return newShips->selection->numShips;
}


sdword kasfShipsSelectMembersOfSelection(GrowSelection *newShips, GrowSelection *originalShips, GrowSelection *members)
{
    Ship *ship;
    sdword i, j, sourceIsDest = FALSE;
    bool32 selected;

    if (newShips == NULL || newShips->selection == NULL ||
        originalShips == NULL || originalShips->selection == NULL ||
        members == NULL || members->selection == NULL)
    {
        return 0;
    }

    if (newShips == originalShips)
    {
        sourceIsDest = TRUE;
    }
    else
    {
        while (newShips->selection->numShips)
        {
            clRemoveShipFromSelection(newShips->selection, newShips->selection->ShipPtr[0]);
        }
    }

    for (i = 0; i < originalShips->selection->numShips; i++)
    {
        ship = originalShips->selection->ShipPtr[i];
        selected = FALSE;
        for (j = 0; j < members->selection->numShips; j++)
        {
            if (members->selection->ShipPtr[j] == ship)
            {
                selected = TRUE;
                break;
            }
        }

        if (!selected && sourceIsDest)
        {
            clRemoveShipFromSelection(originalShips->selection, ship);
        }
        else if (selected && !sourceIsDest)
        {
            growSelectAddShip(newShips, ship);
        }
    }

    return newShips->selection->numShips;
}


sdword kasShipHasAvailableUpgradeChanges(Ship *ship, sdword arg1, sdword arg2)
{
    return shipHasAvailableUpgradeChanges(ship);
}

sdword kasfShipsSelectAvailableUpgradeChanges(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipHasAvailableUpgradeChanges, 0, 0);
}


void kasfShipsStop(GrowSelection *ships)
{
    clWrapHalt((SelectCommand **)&universe.mainCommandLayer, ships->selection);
}

sdword kasfResearchGUIActive(void)
{
    return rmGUIActive;
}

sdword kasfWatchTutorial1Tutorial10CameraMoveDescription(char *techName)
{
    return Watch_Tutorial1_Tutorial10_CameraMoveDescription(techName);
}

sdword kasfSubtitleCleanup(void)
{
    subReset();
    subMessageEnded = 1;
    return speechEventActorStop(ACTOR_FLEETINTEL_FLAG, 0.0f);
}

sdword kasfShipsProcessModuleSlotOrder(GrowSelection *ships, char *shipTypeName, sdword orderValue)
{
    sdword i;
    bool32 transformOrder;
    ShipType shipType;

    transformOrder = (shipTypeName[0] == 'A' || shipTypeName[0] == 'a');
    if (transformOrder)
    {
        shipType = shipTypeName[1] - '0';
    }
    else
    {
        shipType = StrToShipType(shipTypeName);
    }

    if (ships != NULL && ships->selection != NULL)
    {
        for (i = 0; i < ships->selection->numShips; i++)
        {
            if (transformOrder)
            {
                shipProcessModuleSlotOrder(ships->selection->ShipPtr[i], 2, sRecon, *(real32 *)&orderValue, *(real32 *)&shipType);
            }
            else
            {
                shipProcessModuleSlotOrder(ships->selection->ShipPtr[i], 1, shipType, *(real32 *)&orderValue, 0.0f);
            }
        }
    }

    return 0;
}

void kasfDisableShips(GrowSelection *ships)
{
   sdword i;
   SelectCommand *selection;

   if (ships == NULL || ships->selection == NULL)
   {
       return;
   }
   selection = ships->selection;
   for (i = 0; i < selection->numShips; i++)
   {
       shipDisable(selection->ShipPtr[i], 1);
   }
}

sdword kasfActivateTech(char *techName)
{
    TechnologyType techType;
    Player *techPlayer;

    techPlayer = &universe.players[1];

    if (techName[0] == '\0')
    {
        return rmClearPrereqMask((udword *)rmPlayerCurrentPrereqMask(techPlayer));
    }

    techType = StrToTechType(techName);
    dbgAssert(techType != -1);
    rmActivateTech((udword *)rmPlayerCurrentPrereqMask(techPlayer), techType);
    return rmUpdateBuildCapabilities(techPlayer);
}

GrowSelection *kasfShipsApplyPlayerUpgrades(GrowSelection *ships)
{
    sdword i;

    if (ships != NULL && ships->selection != NULL)
    {
        for (i = 0; i < ships->selection->numShips; i++)
        {
            shipApplyPlayerUpgrades(ships->selection->ShipPtr[i]);
        }
    }

    return ships;
}

void kasfCloak(GrowSelection *ships)
{
    sdword i;
    Ship *ship;
    AITeam *team;
    SelectCommand *teamShips;
    SelectCommand selectcom[334];
    AttackCommand attackcom[334];

    memset(selectcom, 0, sizeof(selectcom));
    memset(attackcom, 0, sizeof(attackcom));

    team = kasfScriptTeam();
    if (team == NULL)
    {
        return;
    }

    teamShips = team->shipList.selection;
    if (teamShips->numShips <= 0)
    {
        return;
    }

    for (i = 0; i < teamShips->numShips; i++)
    {
        ship = teamShips->ShipPtr[i];
        if (shipCanSingleClickSpecialActivate(ship) && shipIsAliveCarrierType(ship))
        {
            selectcom[0].ShipPtr[selectcom[0].numShips++] = ship;
        }
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        attackcom[0].TargetPtr[attackcom[0].numTargets++] = (TargetPtr)ships->selection->ShipPtr[i];
    }

    clCloak(&universe.mainCommandLayer, selectcom, attackcom, FALSE);
}

void kasfFindBeastShipsInside(Volume *volume, GrowSelection *result)
{
    sdword i;
    Node *node;
    Ship *ship;

    if (result)
    {
        kasGrowSelectionClear(result);
        node = universe.ShipList.head;
        while (node != NULL)
        {
            ship = (Ship *)listGetStructOfNode(node);
            if ((ship->flags & (SOF_Dead | SOF_Hide)) == 0 &&
                volPointInside(volume, &ship->posinfo.position))
            {
                growSelectAddShip(result, ship);
            }
            node = node->next;
        }
    }

    for (i = 0; i < result->selection->numShips; i++)
    {
        if (!shipIsBeastShip(result->selection->ShipPtr[i]))
        {
            growSelectRemoveShipIndex(result, i--);
        }
    }
}

sdword kasfShipsDockInstant(GrowSelection *ships, GrowSelection *dockWith)
{
    sdword i;
    Ship *dockShip;

    dockShip = dockWith->selection->ShipPtr[0];
    for (i = 0; i < ships->selection->numShips; i++)
    {
        Ship *ship;

        ship = ships->selection->ShipPtr[i];
        univRemoveShipFromOutside(ship);
        dockPutShipInside(ship, dockShip);
    }
    return 0;
}

sdword kasfShipTypeBuildTime(char *shipType)
{
    ShipType shipTypeEnum;
    ShipRace shipRace;

    shipTypeEnum = StrToShipType(shipType);
    shipRace = GetValidRaceForShipType(shipTypeEnum);
    return GetShipStaticInfo(shipTypeEnum, shipRace)->buildTime / GameDifficultyBuildTimeModifier;
}

sdword kasfShipTypeBuildCost(char *shipType)
{
    ShipType shipTypeEnum;
    ShipRace shipRace;

    shipTypeEnum = StrToShipType(shipType);
    shipRace = GetValidRaceForShipType(shipTypeEnum);
    return GetShipStaticInfo(shipTypeEnum, shipRace)->buildCost;
}

Ship *kasfShipsCreate(GrowSelection *result, char *shipTypeStr, sdword count, sdword playerIndex)
{
    vector shippos;
    ShipType shipType;
    ShipRace shipRace;
    Ship *newShip;
    sdword i;

    memset(&shippos, 0, sizeof(shippos));
    shipType = (ShipType)StrToShipType(shipTypeStr);
    shipRace = universe.players[playerIndex].race;
    kasfGrowSelectionClear(result);

    newShip = NULL;
    if (shipType != (ShipType)-1 && count > 0)
    {
        for (i = 0; i < count; i++)
        {
            newShip = univCreateShip(shipType, shipRace, &shippos, &universe.players[playerIndex], 0);
            if (newShip != NULL)
            {
                growSelectAddShip(result, newShip);
            }
        }
    }

    return newShip;
}

sdword kasfShipsCreateHyperspaceIn(GrowSelection *result, char *shipType, sdword count, hvector *destination, sdword playerIndex)
{
    ShipType type;
    ShipRace race;
    vector shipPos;
    MaxSelection filtered;
    sdword i;

    if (result == NULL || result->selection == NULL)
    {
        return 0;
    }

    vecZeroVector(shipPos);
    type = StrToShipType(shipType);
    race = GetRaceForShipTypeRange(type);
    kasGrowSelectionClear(result);
    if (type != (ShipType)-1 && count > 0)
    {
        for (i = 0; i < count; ++i)
        {
            Ship *ship = univCreateShip(type, race, &shipPos, &universe.players[playerIndex], 0);
            if (ship != NULL)
            {
                growSelectAddShip(result, ship);
            }
        }
    }

    kasfShipsAttributesBitSet(result, 0x80);
    if (result->selection->numShips > 0)
    {
        filtered.numShips = 0;
        aiuFilterDisabledShips(result->selection, &filtered);
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, (SelectCommand *)&filtered);
        spHyperspaceSelectionIn((SelectCommand *)&filtered);
        spHyperspaceDelay = 0.0f;
        spHyperspaceSetSelectionDestination(result->selection, destination);
        spHyperspaceDelay = 0.0f;
    }
    kasfShipsAttributesBitClear(result, 0x80);
    return result->selection->numShips;
}

void kasfShipsHyperspaceIn(GrowSelection *ships, hvector *destination)
{
    MaxSelection filtered;

    if (ships == NULL || ships->selection == NULL || ships->selection->numShips == 0)
    {
        return;
    }

    filtered.numShips = 0;
    aiuFilterDisabledShips(ships->selection, &filtered);
    RemoveShipsFromDoingStuff(&universe.mainCommandLayer, (SelectCommand *)&filtered);
    spHyperspaceSelectionIn((SelectCommand *)&filtered);
    spHyperspaceDelay = 0.0f;
}

void kasfShipsSetHyperspaceDestination(GrowSelection *ships, hvector *destination)
{
    if (ships != NULL && ships->selection != NULL && ships->selection->numShips > 0)
    {
        spHyperspaceSetSelectionDestination(ships->selection, destination);
        spHyperspaceDelay = 0.0f;
    }
}

void kasfShipsSetHyperspaceDestinationToCurrentPosition(GrowSelection *ships)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }

    for (i = 0; i < ships->selection->numShips; ++i)
    {
        SelectCommand singleShipSelection;
        hvector destination;
        Ship *ship = ships->selection->ShipPtr[i];

        singleShipSelection.numShips = 1;
        singleShipSelection.ShipPtr[0] = ship;
        destination.x = ship->posinfo.position.x;
        destination.y = ship->posinfo.position.y;
        destination.z = ship->posinfo.position.z;
        destination.w = 1.0f;
        spHyperspaceSetSelectionDestination(&singleShipSelection, &destination);
        spHyperspaceDelay = 0.0f;
    }
}

void kasfShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation)
{
    singlePlayerShipsSetMilitaryParade(ships, formation);
}

sdword kasfShipsSelectTargets(GrowSelection *newShips, GrowSelection *targets)
{
   Node *node;

   if (newShips == NULL || targets == NULL || targets->selection == NULL)
   {
       return 0;
   }

   kasfGrowSelectionClear(newShips);
   if (targets->selection->numShips == 0)
   {
       return 0;
   }

   for (node = universe.mainCommandLayer.todolist.head; node != NULL; node = node->next)
   {
       CommandToDo *command;
       sdword i;

       command = listGetStructOfNode(node);
       if (!bitTest(command->ordertype.attributes, COMMAND_IS_PASSIVEATTACKING) || command->attack == NULL)
       {
           continue;
       }
       if (!AnyOfTheseShipsAreInSelection(targets->selection, command->selection))
       {
           continue;
       }

       for (i = 0; i < command->attack->numTargets; i++)
       {
           growSelectAddShipNoDuplication(newShips, command->attack->TargetPtr[i]);
       }
   }
   return newShips->selection->numShips;
}

sdword kasfShipsSelectLinkedWith(GrowSelection *originalShips, GrowSelection *newShips, GrowSelection *withShips)
{
    sdword i;
    sdword j;

    kasfGrowSelectionClear(newShips);
    for (i = 0; i < withShips->selection->numShips; i++)
    {
        for (j = 0; j < originalShips->selection->numShips; j++)
        {
            if (shipIsLinkedWith(originalShips->selection->ShipPtr[j], withShips->selection->ShipPtr[i]))
            {
                growSelectAddShipNoDuplication(newShips, withShips->selection->ShipPtr[i]);
            }
        }
    }
    return newShips->selection->numShips;
}

void kasfShipsMakeCrazy(GrowSelection *ships, sdword makecrazy)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        if (makecrazy)
        {
            ApplyCareenRotationDirectly(ships->selection->ShipPtr[i]);
            bitSet(ships->selection->ShipPtr[i]->flags, SOF_Crazy);
        }
        else
        {
            bitClear(ships->selection->ShipPtr[i]->flags, SOF_Crazy);
        }
    }
}

void kasfShipsSetSlaveableFlag(GrowSelection *ships, sdword slaveable)
{
   sdword i;

   if (ships == NULL || ships->selection == NULL)
   {
       return;
   }

   for (i = 0; i < ships->selection->numShips; i++)
   {
       if (slaveable)
       {
           bitSet(ships->selection->ShipPtr[i]->flags, SOF_Slaveable);
       }
       else
       {
           bitClear(ships->selection->ShipPtr[i]->flags, SOF_Slaveable);
       }
   }
}

/*=== kasfShipsSetDelayedDeath (0x49a2d0) ===*/
void __cdecl kasfShipsSetDelayedDeath(GrowSelection *ships, bool32 delayedDeath)
{
    SelectCommand *selection;
    sdword i;

    if (ships == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        if (delayedDeath)
        {
            shipSetDelayedDeath(selection->ShipPtr[i], 1.0e21f);
        }
        else
        {
            shipClearDelayedDeath(selection->ShipPtr[i]);
        }
    }
}

sdword kasfShipSetTargetableFlag(GrowSelection *ships, sdword targetable)
{
    if (ships != NULL && ships->selection != NULL && ships->selection->numShips > 0)
    {
        Ship *ship = ships->selection->ShipPtr[0];

        if (targetable)
        {
            bitClear(ship->flags, SOF_TargetableHW1);
        }
        else
        {
            bitSet(ship->flags, SOF_TargetableHW1);
        }
        return (sdword)ship;
    }
    return 0;
}

GrowSelection *kasfShipsSetAttachVelocityFlag(GrowSelection *ships, sdword enable)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return ships;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        if (enable)
        {
            bitSet(ships->selection->ShipPtr[i]->flags, SOF_AttachVelocity);
        }
        else
        {
            bitClear(ships->selection->ShipPtr[i]->flags, SOF_AttachVelocity);
        }
    }

    return ships;
}

sdword kasfSetPilotMode(sdword enable)
{
    sdword result;

    if (enable)
    {
        result = (sdword)universe.mainCameraCommand.dontFocusOnMe | 8;
        universe.mainCameraCommand.dontFocusOnMe = (CameraStackEntry *)((sdword)universe.mainCameraCommand.dontFocusOnMe | 8);
    }
    else
    {
        result = (sdword)universe.mainCameraCommand.dontFocusOnMe & ~8;
        universe.mainCameraCommand.dontFocusOnMe = (CameraStackEntry *)((sdword)universe.mainCameraCommand.dontFocusOnMe & ~8);
    }
    return result;
}

sdword kasfShipsSelectSpecialTargets(GrowSelection *result, GrowSelection *source)
{
    SelectCommand *selection;
    int *specialTargets;
    sdword i;
    sdword j;
    sdword special;

    kasGrowSelectionClear(result);
    selection = source->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        special = *(sdword *)((ubyte *)selection->ShipPtr[i] + 880);
        if (special != 0 && *(uword *)(special + 20) == 7)
        {
            specialTargets = *(int **)(special + 700);
            for (j = 0; j < *specialTargets; j++)
            {
                growSelectAddShipNoDuplication(result, (Ship *)specialTargets[j + 2]);
            }
        }
    }
    return result->selection->numShips;
}

GrowSelection *kasfShipsSetRotatableFlag(GrowSelection *ships, sdword enable)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return ships;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        if (enable)
        {
            bitSet(ships->selection->ShipPtr[i]->flags, SOF_Rotatable);
        }
        else
        {
            bitClear(ships->selection->ShipPtr[i]->flags, SOF_Rotatable);
        }
    }

    return ships;
}

sdword kasfVectorRoundX(hvector *value)
{
    return (sdword)floor(value->x + 0.5f);
}

sdword kasfVectorRoundY(hvector *value)
{
    return (sdword)floor(value->y + 0.5f);
}

sdword kasfVectorRoundZ(hvector *value)
{
    return (sdword)floor(value->z + 0.5f);
}

sdword kasfVectorRoundW(hvector *value)
{
    return (sdword)floor(value->w + 0.5f);
}

void kasfShipsSortByDistanceToPoint(GrowSelection *ships, vector *point)
{
    sdword i;
    sdword j;
    SelectCommand *selection;

    selection = ships->selection;
    for (i = 1; i < selection->numShips; i++)
    {
        j = i;
        while (j > 0)
        {
            Ship *previousShip;
            Ship *currentShip;
            vector previousOffset;
            vector currentOffset;
            real32 previousDistanceSquared;
            real32 currentDistanceSquared;

            previousShip = selection->ShipPtr[j - 1];
            currentShip = selection->ShipPtr[j];
            previousOffset.x = previousShip->posinfo.position.x - point->x;
            previousOffset.y = previousShip->posinfo.position.y - point->y;
            previousOffset.z = previousShip->posinfo.position.z - point->z;
            currentOffset.x = currentShip->posinfo.position.x - point->x;
            currentOffset.y = currentShip->posinfo.position.y - point->y;
            currentOffset.z = currentShip->posinfo.position.z - point->z;
            previousDistanceSquared =
                previousOffset.x * previousOffset.x +
                previousOffset.y * previousOffset.y +
                previousOffset.z * previousOffset.z;
            currentDistanceSquared =
                currentOffset.x * currentOffset.x +
                currentOffset.y * currentOffset.y +
                currentOffset.z * currentOffset.z;
            if (previousDistanceSquared <= currentDistanceSquared)
            {
                break;
            }
            selection->ShipPtr[j] = previousShip;
            selection->ShipPtr[j - 1] = currentShip;
            j--;
        }
    }
}

sdword kasfShipsAnyBigGunReady(GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection;

    if (ships == NULL || ships->selection == NULL)
    {
        return FALSE;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; ++i)
    {
        if (bigGunChargeFraction(selection->ShipPtr[i]) >= 1.0f)
        {
            return TRUE;
        }
    }
    return FALSE;
}

sdword kasfShipsAnySwarmersReady(GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection;

    if (ships == NULL || ships->selection == NULL)
    {
        return FALSE;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; ++i)
    {
        if (shipHasReadySwarmers(selection->ShipPtr[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

sdword kasfShipGetModuleSlotTimer(GrowSelection *shipSelection)
{
    if (shipSelection == NULL || shipSelection->selection == NULL || shipSelection->selection->numShips < 1)
    {
        return 0;
    }
    return shipGetModuleSlotTimer(shipSelection->selection->ShipPtr[0]);
}

sdword kasfShipDecrementModuleSlotTimer(GrowSelection *shipSelection)
{
    if (shipSelection == NULL || shipSelection->selection == NULL || shipSelection->selection->numShips < 1)
    {
        return 0;
    }

    shipDecrementModuleSlotTimer(shipSelection->selection->ShipPtr[0]);
    return 0;
}

void kasfTDSalvageTrialStartWaitingForShips(GrowSelection *shipSelection, vector *position)
{
    if (shipSelection != NULL && shipSelection->selection != NULL && shipSelection->selection->numShips >= 1)
    {
        shipStartWaitingForShips(shipSelection->selection->ShipPtr[0], position);
    }
}

sdword kasfTDSalvageTrialWaitingForShipsDone(GrowSelection *shipSelection)
{
    return (shipSelection != NULL
        && shipSelection->selection != NULL
        && shipSelection->selection->numShips >= 1
        && shipWaitingForShipsDone(shipSelection->selection->ShipPtr[0]));
}

sdword kasfTDSalvageTrialGetWaitingForShipsPoints(GrowSelection *shipSelection, vector *fromPoint, vector *toPoint)
{
    if (shipSelection == NULL || shipSelection->selection == NULL || shipSelection->selection->numShips < 1)
    {
        return FALSE;
    }
    return shipGetWaitingForShipsPoints(shipSelection->selection->ShipPtr[0], fromPoint, toPoint);
}

sdword kasfTDSalvageTrialWaitingForShipsActive(GrowSelection *shipSelection)
{
    return (shipSelection != NULL
        && shipSelection->selection != NULL
        && shipSelection->selection->numShips >= 1
        && shipWaitingForShipsActive(shipSelection->selection->ShipPtr[0]));
}

void kasfDisableMothershipBigGunLaunch(void)
{
    kasfMothershipBigGunDisabled = 1;
}

sdword kasfShipsAnyHyperspaceReady(GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection;

    if (ships == NULL || ships->selection == NULL)
    {
        return FALSE;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; ++i)
    {
        if (acvGetEMPChargeProgress(selection->ShipPtr[i]) >= 1.0f)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void kasfAttackEnemyMothership(void)
{
 Player *player;
 Ship *enemyMothership;
 MaxSelection enemySelection;

 if (CurrentTeamP == NULL || CurrentTeamP->shipList.selection == NULL || CurrentTeamP->shipList.selection->numShips == 0)
 {
     return;
 }

 player = CurrentTeamP->shipList.selection->ShipPtr[0]->playerowner;
 enemyMothership = aiuFindEnemyMothership(player);
 if (enemyMothership == NULL)
 {
     return;
 }

 enemySelection.numShips = 0;
 selSelectionAddSingleShip(&enemySelection, enemyMothership);
 aitDeleteAllTeamMoves(CurrentTeamP);
 aimCreateAttack(CurrentTeamP, selectDupSelection((SelectCommand *)&enemySelection), SAME_FORMATION, TRUE, TRUE);
}

void kasfIssueSpecialAttack(GrowSelection *targets)
{
 SelectCommand *dupeTargets;

 if (targets == NULL || targets->selection == NULL)
 {
   return;
 }

 dupeTargets = selectDupSelection(targets->selection);
 if (dupeTargets->numShips && dupeTargets->ShipPtr[0] != NULL)
 {
   aitDeleteAllTeamMoves(CurrentTeamP);
   aimCreateSpecial(CurrentTeamP, dupeTargets, SAME_FORMATION, CurrentTeamP->kasTactics, FALSE, TRUE);
 }
}

void kasfTeamSpecialToggle(void)
{
    SelectCommand *selection;

    if (kasCurrentTeamPtr == 0)
    {
        return;
    }
    selection = *(SelectCommand **)(kasCurrentTeamPtr + 32);
    if (selection->numShips)
    {
        clWrapSpecial(&universe.mainCommandLayer, selection, 0);
    }
}

void kasfTutSetPointerTargetFirstShip(char *name, GrowSelection *ships)
{
    if (ships->selection->numShips > 0)
    {
        tutSetPointerTargetShip(name, ships->selection->ShipPtr[0], 0);
    }
}

sdword kasfShipsAnyIdle(GrowSelection *ships)
{
   sdword i;

   if (ships == NULL || ships->selection == NULL || ships->selection->numShips <= 0)
   {
       return 0;
   }

   for (i = 0; i < ships->selection->numShips; i++)
   {
       CommandToDo *command = ships->selection->ShipPtr[i]->command;
       if (command == NULL || command->ordertype.order == COMMAND_NULL)
       {
           return 1;
       }
   }
   return 0;
}

bool32 kasfTeamIsIdle(struct AITeam *team)
{
    return aitTeamIsIdle(team);
}

bool32 std_ctype_char_Term(void)
{
    return aitTeamIsIdle((AITeam *)kasCurrentTeamPtr);
}

sdword kasfTutSetCataclysmInterfaceStyle(void)
{
    showDust = 1;
    showResources = 1;
    showFOWBlobs = 1;
    showSensorBackgrounds = 1;
    showAreasVisited = 1;
    showAreasOccupied = 0;
    showCapitalShipOverlay = 1;
    showEnemyShips = 1;
    showFriendlyShips = 1;
    showMeteors = 1;
    showCompass = 0;
    showObjectivePointers = 1;
    showSlipgatePaths = 1;
    return 1;
}

sdword kasfShipsSelectAttachedShips(GrowSelection *newShips, GrowSelection *source)
{
    Ship *ship;

    kasGrowSelectionClear(newShips);
    if (source == NULL || source->selection == NULL || source->selection->numShips <= 0)
    {
        return newShips->selection->numShips;
    }

    ship = SalCapGetFirstAttachedShip(source->selection->ShipPtr[0]);
    while (ship != NULL)
    {
        growSelectAddShip(newShips, ship);
        ship = SalCapGetNextAttachedShip();
    }

    return newShips->selection->numShips;
}

unsigned int j_tutHideBackButtonRegion(void)
{
    return tutHideBackButtonRegion();
}

void kasfTutCameraAdjustDistanceTowardMinimumZoom(sdword scalePercent)
{
    ccAdjustDistanceTowardMinimumZoom(&universe.mainCameraCommand, (real32)scalePercent * 0.01f);
}

sdword kasfConstructionManagerActive(void)
{
    return cmActive;
}

sdword kasfSpeechEventQueueCurrentPlayerUnlinked(sdword event, sdword shipType)
{
    subMessageEnded = 0;
    return speechEventQueue(0, event, shipType, -1, -1, universe.curPlayerIndex, -1, -1.0f, -1);
}

sdword kasShipIsSpecialTargeting(Ship *ship, sdword arg1, sdword arg2)
{
    CommandToDo *command;
    SpecialCommand *specialTargets;
    sdword i;

    (void)arg1;
    (void)arg2;

    if (ship == NULL || ship->playerowner == NULL)
    {
        return FALSE;
    }

    command = getShipAndItsCommand(&universe.mainCommandLayer, ship);
    if (command == NULL || command->ordertype.order != COMMAND_SPECIAL || command->specialtargets == NULL)
    {
        return FALSE;
    }

    specialTargets = command->specialtargets;
    for (i = 0; i < specialTargets->numTargets; i++)
    {
        Ship *target = (Ship *)specialTargets->TargetPtr[i];

        if (SalCapCanTargetShip(ship->playerowner, (SpaceObjRotImpTargGuidanceShipDerelict *)target) &&
            (target->playerowner == NULL || target->playerowner->playerIndex == ship->playerowner->playerIndex))
        {
            return TRUE;
        }
    }

    return FALSE;
}

sdword kasfShipsSelectSpecialTargeting(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(result, source, kasShipIsSpecialTargeting, 0, 0);
}

void kasfTutSetPointerTargetShipWithLine(char *name, GrowSelection *ships)
{
    if (ships->selection->numShips > 0)
    {
        tutSetPointerTargetShip(name, ships->selection->ShipPtr[0], 1);
    }
}

void kasfTutSetPointerTargetCursor(char *name)
{
    tutSetPointerTargetCursor(name);
}

/*=== kasfShipsEnableAttachNLips (0X497ED0) ===*/
void __cdecl kasfShipsEnableAttachNLips(GrowSelection *ships)
{
    SelectCommand *selection;
    sdword i;

    if (ships == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        selection->ShipPtr[i]->flags |= SOF_AttachNLips;
    }
}

void kasfShipsSetSpeedBurstFlag(GrowSelection *ships, bool32 flag)
{
    sdword i;
    SelectCommand *selection;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        if (flag)
        {
            bitSet(selection->ShipPtr[i]->specialFlags2, SPECIAL_SpeedBurst);
        }
        else
        {
            bitClear(selection->ShipPtr[i]->specialFlags2, SPECIAL_SpeedBurst);
        }
    }
}

sdword kasfShipsSelectAllAttachedShips(GrowSelection *result, GrowSelection *source)
{
    sdword i;

    if (result == NULL || result->selection == NULL || source == NULL || source->selection == NULL)
    {
        return 0;
    }

    kasfGrowSelectionClear(result);
    for (i = 0; i < source->selection->numShips; i++)
    {
        Ship *ship;
        SelectCommand *attachedSelection;
        sdword j;

        ship = source->selection->ShipPtr[i];
        // Retail reads ship->clampInfo->selection (the clamp-attached-ships list), not the
        // legacy HW1 slaveinfo/SF_MASTER slave-trade system; "attached ships" here refers to
        // clamped children (e.g. carrier support modules), matching clampInfo->selection usage
        // everywhere else in this codebase.
        if (ship->clampInfo == NULL || ship->clampInfo->selection == NULL)
        {
            continue;
        }

        attachedSelection = (SelectCommand *)ship->clampInfo->selection;
        for (j = 0; j < attachedSelection->numShips; j++)
        {
            growSelectAddShip(result, attachedSelection->ShipPtr[j]);
        }
    }
    return result->selection->numShips;
}

sdword __cdecl kasfTeamSpecial(GrowSelection *targets)
{
    SelectCommand *selection;
    SpecialCommand *specialTargets;
    sdword i;

    if (CurrentTeamP == NULL)
    {
        return FALSE;
    }
    if (CurrentTeamP->shipList.selection == NULL || CurrentTeamP->shipList.selection->numShips == 0)
    {
        return FALSE;
    }
    if (targets == NULL || targets->selection == NULL)
    {
        return FALSE;
    }

    selection = targets->selection;
    specialTargets = memAlloc(sizeof(sdword) * selection->numShips + 8, "TeamSpecial", MBF_Volatile);
    specialTargets->numTargets = selection->numShips;
    for (i = 0; i < selection->numShips; i++)
    {
        specialTargets->TargetPtr[i] = selection->ShipPtr[i];
    }
    clWrapSpecial(&universe.mainCommandLayer, CurrentTeamP->shipList.selection, specialTargets);
    memFree(specialTargets);
    return TRUE;
}

sdword kasfShipsSpecial(GrowSelection *ships, GrowSelection *targets)
{
    sdword i;
    SelectCommand *selection;
    SpecialCommand *specialTargets;

    if (ships == NULL || ships->selection == NULL || targets == NULL || targets->selection == NULL)
    {
        return FALSE;
    }

    selection = targets->selection;
    specialTargets = memAlloc(sizeofSelectCommand(selection->numShips), "KASShipsSpecial", MBF_Volatile);
    specialTargets->numTargets = selection->numShips;
    for (i = 0; i < selection->numShips; i++)
    {
        specialTargets->TargetPtr[i] = selection->ShipPtr[i];
    }
    clWrapSpecial(&universe.mainCommandLayer, ships->selection, specialTargets);
    memFree(specialTargets);
    return TRUE;
}

void kasfHaltAllNotDockedShips(void)
{
    Node *node;
    SelectCommand selection;

    selection.numShips = 1;
    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = node->structptr;

        if (shipIsNotDocked(ship, TRUE))
        {
            selection.ShipPtr[0] = ship;
            clWrapHalt(&universe.mainCommandLayer, &selection);
        }
    }
}

sdword kasfGetShipTypeCount(char *shipType, sdword playerIndex)
{
    return univGetShipTypeCount(StrToShipType(shipType), playerIndex);
}

sdword kasfGetReplacementShipTypeCount(char *shipType, sdword playerIndex)
{
    return univGetReplacementShipTypeCount(StrToShipType(shipType), playerIndex);
}

sdword kasfShipsQueueBeastConversionOrScuttle(GrowSelection *ships, GrowSelection *attacker)
{
    GrowSelection scuttleSelection;
    SelectCommand *selection;
    Ship *attackerShip;
    sdword i;
    sdword scuttleCount;

    if (ships == NULL || ships->selection == NULL || attacker == NULL || attacker->selection == NULL || attacker->selection->numShips < 1)
    {
        return 0;
    }

    attackerShip = attacker->selection->ShipPtr[0];
    growSelectInit(&scuttleSelection);
    selection = ships->selection;
    for (i = 0; i < selection->numShips; ++i)
    {
        Ship *ship = selection->ShipPtr[i];

        if (shipGetBeastWeaponInfo(ship) != NULL)
        {
            shipQueueBeastConversion(ship, attackerShip);
        }
        else
        {
            growSelectAddShip(&scuttleSelection, ship);
        }
    }

    scuttleCount = scuttleSelection.selection->numShips;
    if (scuttleCount > 0)
    {
        clScuttle(&universe.mainCommandLayer, scuttleSelection.selection);
    }
    growSelectClose(&scuttleSelection);
    return scuttleCount;
}


void kasfShipsSetResourcingFlag(sdword shipsAddress, bool32 flag)
{
    sdword i;
    GrowSelection *ships = (GrowSelection *)shipsAddress;
    SelectCommand *selection;

    if (ships == NULL || ships->selection == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        sdword j;
        Ship *ship = selection->ShipPtr[i];
        ShipUpgradeSlotStateBlock *slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->runtimeData490;

        bitSetTo(ship->specialFlags2, BIT15, flag);
        if (slotStateBlock == NULL)
        {
            continue;
        }

        for (j = 0; j < slotStateBlock->count; j++)
        {
            ShipUpgradeSlotStateEntry *slotState = &slotStateBlock->entries[j];
            udword *slotWords = (udword *)slotState;

            if (slotState->gunStatic != NULL && slotState->gunStatic->gunsoundtype == GS_BigGun)
            {
                slotWords[28] = 0x3C23D70A;    // 0.01f in the shipped binary
            }
        }
    }
}

sdword kasfShipGetModuleCount(GrowSelection *shipSelection)
{
    if (shipSelection == NULL || shipSelection->selection == NULL || shipSelection->selection->numShips < 1)
    {
        return 0;
    }
    return shipGetModuleCount(shipSelection->selection->ShipPtr[0]);
}

sdword kasfShipCanStartModuleSlotTimer(GrowSelection *shipSelection)
{
    if (shipSelection == NULL || shipSelection->selection == NULL || shipSelection->selection->numShips < 1)
    {
        return FALSE;
    }
    return shipCanStartModuleSlotTimer(shipSelection->selection->ShipPtr[0]);
}

sdword kasfShipTryStartModuleSlotTimer(GrowSelection *shipSelection)
{
    if (shipSelection == NULL || shipSelection->selection == NULL || shipSelection->selection->numShips < 1)
    {
        return FALSE;
    }
    return shipTryStartModuleSlotTimer(shipSelection->selection->ShipPtr[0]);
}

void kasfCreateBasicShockwave(hvector *position)
{
    if (position != NULL)
    {
        shockwaveCreateBasic(position->x, position->y, position->z);
    }
}

/*=== kasfEntryScopeName (0x491e70) ===*/
sdword __cdecl kasfEntryScopeName(sdword scopeName)
{
    sdword scopeId;

    if (scopeName >= 0)
    {
        scopeId = (kasCurrentStep != 0) ? kasCurrentScopeHash : 0;
    }
    else
    {
        scopeName &= 0x7FFFFFFF;
        scopeId = 0;
    }

    if (scopeName != 0)
    {
        kasfEntryFindOrCreate(scopeId, scopeName);
    }

    return scopeId;
}
void kasfWideScreenCleanup(void)
{
    if (piePointSpecMode)
    {
        piePointModeOnOff();
    }
    if (kasfWideScreenPreserveMainScreenLockout)
    {
        spMainScreenAndLockout(2u);
    }
    else
    {
        spMainScreenCleanupAndLockout(2);
    }
    kasfWideScreenActive = 0;
    overlayPauseActive = 0;
    mouseCursorHide();
}
GrowSelection *kasfShipsSetSupportFlag(GrowSelection *ships, sdword enable)
{
    sdword i;
    SelectCommand *selection;
    sdword disabled;

    if (ships == NULL || ships->selection == NULL)
    {
        return ships;
    }

    selection = ships->selection;
    disabled = (enable == 0);
    for (i = 0; i < selection->numShips; ++i)
    {
        Ship *ship = selection->ShipPtr[i];

        if (disabled)
        {
            if ((ship->specialFlags2 & SPECIAL_2_DisabledState) == 0)
            {
                shipRemoveSupportUsage(ship);
            }
            ship->specialFlags2 |= SPECIAL_2_DisabledState;
        }
        else if ((ship->specialFlags2 & SPECIAL_2_DisabledState) != 0)
        {
            ship->specialFlags2 &= ~SPECIAL_2_DisabledState;
            shipAddSupportUsage(ship, ship->playerowner);
        }

        clSetAttribute((sdword)ship, disabled, 1);
    }

    return ships;
}

void kasfTutCameraFocusIgnoreZoom(GrowSelection *ships)
{
    SelectCommand focuscom;

    if (ships->selection->numShips <= 0)
    {
        memcpy(&focuscom, &selSelected, sizeof(focuscom));
        ccFocusIgnoreZoom(&universe.mainCameraCommand, &focuscom);
    }
    else if (!SelectionsAreEquivalent((SelectCommand *)universe.mainCameraCommand.currentCameraStack + 9,
                                      ships->selection))
    {
        kasfTutCameraFocusActive = TRUE;
        if (piePointSpecMode)
        {
            piePointModeOnOff();
        }
        FancyFightPreLoad(FALSE);
        ccFocusIgnoreZoom(&universe.mainCameraCommand, ships->selection);
    }
}

GrowSelection *kasfShipsSetAttachCoordsysFlag(GrowSelection *ships, sdword enable)
{
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return ships;
    }

    for (i = 0; i < ships->selection->numShips; i++)
    {
        if (enable)
        {
            bitSet(ships->selection->ShipPtr[i]->flags, SOF_AttachCoordsys);
        }
        else
        {
            bitClear(ships->selection->ShipPtr[i]->flags, SOF_AttachCoordsys);
        }
    }

    return ships;
}


sdword kasfFindMissilesInside(Volume *vol, GrowSelection *ships)
{
    Node *node;

    if (ships == NULL)
    {
        return 0;
    }

    kasGrowSelectionClear(ships);
    for (node = universe.MissileList.head; node != NULL; node = node->next)
    {
        Missile *missile = listGetStructOfNode(node);

        if (missile->objtype == OBJ_MissileType &&
            missile->missileType == MISSILE_Mine &&
            volPointInside(vol, &missile->posinfo.position))
        {
            growSelectAddShip(ships, (Ship *)missile);
        }
    }

    return ships->selection->numShips;
}


sdword kasfFindEnemyShipsInsideVolume(Volume *vol, GrowSelection *growSelect)
{
    return kasfFindEnemyShipsInside(vol, growSelect);
}

sdword kasfFindEnemiesNearSelection(GrowSelection *ships, sdword radius)
{
    return kasfFindEnemiesNearby(ships, radius);
}

sdword kasfFindEnemiesNearCurrentTeam(GrowSelection *growSelect, sdword radius)
{
    return kasfFindEnemiesNearTeam(growSelect, radius);
}

sdword kasfSetHyperspaceWindowHold(sdword hold)
{
    spHoldHyperspaceWindow = hold;
    return hold;
}

AITeamMove *kasfDockStayWithTeam(AITeam *withTeam)
{
    Ship *dockWith = NULL;
    sdword flags = dockmoveFlags_Stay;

    if (withTeam != NULL && withTeam->shipList.selection->numShips >= 1)
    {
        dockWith = withTeam->shipList.selection->ShipPtr[0];
        if (kasfDockTargetIsFiltered(dockWith))
        {
            dockWith = NULL;
        }
        else
        {
            flags |= dockmoveFlags_AtShip;
        }
    }

    aitDeleteAllTeamMoves(CurrentTeamP);
    return aimCreateDock(CurrentTeamP, flags, dockWith, FALSE, TRUE);
}

AITeamMove *kasfDockInstantWithTeam(AITeam *withTeam)
{
    Ship *dockWith = NULL;
    sdword flags = dockmoveFlags_Stay | dockmoveFlags_Instantaneously;

    if (withTeam != NULL && withTeam->shipList.selection->numShips >= 1)
    {
        dockWith = withTeam->shipList.selection->ShipPtr[0];
        if (kasfDockTargetIsFiltered(dockWith))
        {
            dockWith = NULL;
        }
        else
        {
            flags |= dockmoveFlags_AtShip;
        }
    }

    aitDeleteAllTeamMoves(CurrentTeamP);
    return aimCreateDock(CurrentTeamP, flags, dockWith, FALSE, TRUE);
}

void kasfShipsSetFormation(GrowSelection *ships, sdword formation)
{
    if (formation == 8)
    {
        clWrapSetMilitaryParade(&universe.mainCommandLayer, ships->selection);
    }
    else
    {
        clWrapFormation(&universe.mainCommandLayer, ships->selection, formation);
    }
}

void kasfProtect(GrowSelection *targets)
{
    if (targets->selection->numShips)
    {
        aitDeleteAllTeamMoves(CurrentTeamP);
        clWrapProtect(&universe.mainCommandLayer, CurrentTeamP->shipList.selection, targets->selection);
    }
}

void kasfShipsProtect(GrowSelection *ships, GrowSelection *targets)
{
    SelectCommand *selection;

    if (targets->selection->numShips)
    {
        selection = ships->selection;
        if (selection->numShips)
        {
            clWrapProtect(&universe.mainCommandLayer, selection, targets->selection);
        }
    }
}

bool32 kasfTacticsSet(TacticsType tactics)
{
    CurrentTeamP->kasTactics = tactics;
    return aiuWrapSetTactics(CurrentTeamP->shipList.selection, tactics);
}

sdword kasfShipsFinishedLaunching(GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection = ships->selection;

    for (i = 0; i < selection->numShips; i++)
    {
        if (!ShipFinishedLaunching(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}


void kasfShipsSetPlayerMothershipParade(GrowSelection *ships)
{
    SelectCommand *selection;
    sdword i;

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        GroupShipIntoMilitaryParade(
            &universe.mainCommandLayer,
            selection->ShipPtr[i],
            universe.players[0].PlayerMothership);
    }
    clPresetShipsToPosition(&universe.mainCommandLayer);
}

/*=== kasfShipsSetPercentFueled (0x493a10) ===*/
void __cdecl kasfShipsSetPercentFueled(GrowSelection *ships, real32 percentFueled)
{
    SelectCommand *selection;
    sdword i;

    if (ships == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship;

        ship = selection->ShipPtr[i];
        ship->damageModifier = percentFueled * 0.01f;
        ship->specialFlags1C8 |= 0x2000;
    }
}

void kasfShipsClearPercentFueled(GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection;

    if (ships == NULL)
    {
        return;
    }

    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        selection->ShipPtr[i]->flags &= ~0x2000u;
    }
}

sdword kasfTeamAttributesAnySet(sdword attributes)
{
    sdword i;
    sdword result = 0;

    if (CurrentTeamP == NULL || CurrentTeamP->shipList.selection == NULL)
    {
        return 0;
    }

    for (i = 0; i < CurrentTeamP->shipList.selection->numShips; ++i)
    {
        result |= CurrentTeamP->shipList.selection->ShipPtr[i]->attributes & attributes;
    }

    return result;
}

bool32 kasfShipsDockWithSelection(GrowSelection *ships, GrowSelection *withShips)
{
    SelectCommand *selection;

    selection = ships->selection;
    if (selection->numShips >= 1
     && selection->ShipPtr[0]->shiptype == tResourceCollector
     && singlePlayerGameInfo.currentMission == 9)
    {
        return aiuWrapDock(selection, 7, withShips->selection->ShipPtr[0]);
    }
    return aiuWrapDock(selection, 1, withShips->selection->ShipPtr[0]);
}

sdword kasfSelectionIsInsideVolume(Volume *volume, GrowSelection *ships)
{
    sdword i;
    SelectCommand *selection = ships->selection;

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];

        if ((ship->flags & (SOF_Hide | SOF_Hyperspace)) != 0 ||
            !volPointInside(volume, &ship->posinfo.position))
        {
            return FALSE;
        }
    }
    return TRUE;
}

void kasfForceAttack(GrowSelection *ships, hvector *destination, sdword a3) { (void)ships; (void)destination; (void)a3; }
