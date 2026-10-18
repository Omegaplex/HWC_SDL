//
//  support code for the KAS language and its missions, FSMs, and states
//
//  implementations of most functions are in kasfunc.c
//

#include <stdio.h>
#include <string.h>
#include "glinc.h"
#include "mainrgn.h"
#include "shipselect.h"
#include "select.h"
#include "AITeam.h"
#include "AIPlayer.h"
#include "AIUtilities.h"
#include "AIVar.h"
#include "Timer.h"
#include "vector.h"
#include "kas.h"
#include "KASFunc.h"
#include "file.h"
#include "volume.h"
#include "font.h"
#include "render.h"
#include "prim3d.h"
#include "Objectives.h"
#include "SinglePlayer.h"
#include "researchapi.h"
#include "SaveGame.h"
#include "hs.h"
#include "CommandWrap.h"

extern sdword objectivesUsed;
extern sdword g_GameDifficulty;
extern void aifAddShipToTeam(AITeam *team, Ship *ship);
sdword WatchFunctionToIndex(KASWatchFunction watchFunction);
KASWatchFunction IndexToWatchFunction(sdword index);
#if SP_DEBUGKEYS
//#if SP_DEBUGLEVEL2
// for debug overlay
extern fonthandle selGroupFont2;
extern AIVar **vars;
extern sdword varsUsed;
extern sdword timersUsed;
extern NamedTimer *timers;
sdword mrKASDebugDrawStates = FALSE;
sdword mrKASDebugDrawTimers = FALSE;
sdword mrKASDebugDrawVars = FALSE;
sdword mrKASDebugDrawVolumes = FALSE;
sdword mrKASSkipFactor = 0;
#endif //SP_DEBUGLEVEL2

#define OBJECTIVES_COMPLETE_DELAY 10

// run-time scoping for variables, timers, etc.
sdword CurrentMissionScope;  // what level we're currently executing at
char CurrentMissionScopeName[KAS_MISSION_NAME_MAX_LENGTH+1];

char CurrentMissionName[KAS_MISSION_NAME_MAX_LENGTH+1];
KASWatchFunction CurrentMissionWatchFunction = NULL;
AITeam *CurrentTeamP = NULL;
AITeam *kasUnpausedTeam = NULL;
sdword CurrentMissionSkillLevel = 0;

// ---------------------------------------------------------------------------
//  Cataclysm KAS runtime state (matches cataclysm.exe data model).
//
//  Cataclysm replaced HW1's labelled-entity arrays (Selections[],
//  LabelledVectors[], etc.) with hash-keyed tables.  Each table is an array
//  of pointers to [hash, payload] records.  The runtime functions below
//  operate on these tables instead of the HW1 arrays.
// ---------------------------------------------------------------------------
KASWatchFunction kasMissionWatchFunction = NULL;  // mission watch function (0xA43EE4)
sdword kasCurrentTeamPtr = 0;                    // current team ptr during execution (0xA43EE8, "team")
static sdword kasSingleExecutionTeamPtr = 0;    // single-FSM execution node (0xA43EEC)
static sdword kasRuntimeCounter = 0;            // runtime counter/state slot (0xA43F20)
static ubyte *kasGrowSelectionTable = NULL;   // GrowSelection/flag hash table (0xA43EF0, "flags")
static sdword kasGrowSelectionCount = 0;      // GrowSelection count (0xA43EF4)
static sdword kasGrowSelectionCapacity = 0;   // GrowSelection capacity (0xA43EF8)
static ubyte *kasLabelledPathTable = NULL;    // Path hash table (0xA43EFC)
static sdword kasLabelledPathCount = 0;       // Path count (0xA43F00)
static sdword kasLabelledPathCapacity = 0;    // Path capacity (0xA43F04)
static ubyte *kasLabelledVectorTable = NULL;  // Vector hash table (0xA43F08)
static sdword kasLabelledVectorCount = 0;     // Vector count (0xA43F0C, "nVectors")
static sdword kasLabelledVectorCapacity = 0;  // Vector capacity (0xA43F10)
static ubyte *kasLabelledVolumeTable = NULL;  // Volume hash table (0xA43F14)
static sdword kasLabelledVolumeCount = 0;     // Volume count (0xA43F18)
static sdword kasLabelledVolumeCapacity = 0;  // Volume capacity (0xA43F1C)
sdword kasCurrentStep = 0;                    // current scoped KAS step ordinal (0xC3749C)
sdword kasCurrentScopeHash = 0;               // current mission/team scope hash (0xC374A0, "scope")
static hvector kasVolumeVectorResult = {0, 0, 0, 1.0f};  // static return buffer (0xA43E90)

// Implemented in KASFunc.c, no shared declaration.
unsigned int kasfEntryDestroyAll(void);
unsigned int kasLabelledTimerDestroyAll(void);


#define kasThisTeamPtr CurrentTeamP

//
//  variable space for user selections (the labelled "SHIPS" type in KAS)
//
KasSelection **Selections = NULL;
static sdword SelectionsUsed = 0;
static sdword SelectionsAllocated = 0;

//
//  table of labelled entities in mission layout file
//  (filled in at mission load time)
//

//  team labels are stored in AITeam struct
//LabelledAITeams *LabelledAITeam;
//static sdword LabelledAITeamsUsed = 0;
//static sdword LabelledAITeamsAllocated = 0;

LabelledPath **LabelledPaths = NULL;
static sdword LabelledPathsUsed = 0;
static sdword LabelledPathsAllocated = 0;

LabelledVector **LabelledVectors = NULL;
sdword LabelledVectorsUsed = 0;
sdword LabelledVectorsAllocated = 0;

LabelledVolume **LabelledVolumes = NULL;
static sdword LabelledVolumesUsed = 0;
static sdword LabelledVolumesAllocated = 0;

real32 subMessageReturnedFalseTime = 0;

#if 0  // HW1 version - replaced by Cataclysm version
// need some closing stuff for kas - global variables need to be
// made NULL etc.
void kasClose()
{
    kasUnpausedTeam = NULL;
    subMessageReturnedFalseTime = 0.0f;
}

void kasInit()
{
    subMessageReturnedFalseTime = 0.0f;
}
#endif

void kasTeamDied(struct AITeam *team)
{
    if (CurrentTeamP == team)
    {
        CurrentTeamP = NULL;
    }
}

//
//  JUMP keyword of KAS language
//
//  sets state of current team to new state (for subsequently WATCHing) and executes its
//  init function right now
//
void kasJump(char *stateName, KASInitFunction initFunction, KASWatchFunction watchFunction)
{
#ifndef HW_Release
{
    char teamName[KAS_TEAM_NAME_MAX_LENGTH+1];
    aiplayerLog((aiCurrentAIPlayer->player->playerIndex,"KAS: TEAM(\"%s\") JUMP %s", kasAITeamName(CurrentTeamP, teamName), stateName));
}
#endif
    CurrentTeamP->kasStateWatchFunction = watchFunction;

    initFunction();
}





// Cataclysm-specific: case-insensitive ELF/PJW hash used to key all KAS labelled entities
// (binary aivarFindAnyFSM 0x491860 — completely different from HW1 aivarFindAnyFSM)
sdword kasHashLabel(char *label)
{
    sdword result = 0;
    for (char *p = label; *p; p++)
    {
        result = 16 * result + (*p & 0xDF);  // 0xDF: bit-5 clear → case fold to uppercase
        if (result & 0xF0000000)
            result ^= (result & 0xF0000000) ^ ((result & 0xF0000000) >> 24);
    }
    return result;
}

void kasAddShipToTeam(Ship *ship, sdword labelHash)
{
    AITeam *teamP = kasAITeamPtrIfExists(labelHash);
    if (!teamP)
    {
        teamP = aitCreate(ScriptTeam);
        teamP->kasLabelHash = labelHash;  // team label hash (DWORD[22], offset 88)
        teamP->kasOrigShipsCount = 0;
    }
    ++teamP->kasOrigShipsCount;
    teamP->kasOrigShipsType = ship->shiprace;
    aifAddShipToTeam(teamP, ship);
    --teamP->newships;
}



#if 0  // HW1 version - replaced by Cataclysm version
//
//  FSMCreate keyword of KAS language
//
//  "creates" an FSM and hands control of a team to it
//
void kasFSMCreate(char *fsmName, KASInitFunction initFunction, KASWatchFunction watchFunction, AITeam *teamP)
{
    AITeam *saveP;
    sdword saveScope;
    char SaveMissionScopeName[KAS_MISSION_NAME_MAX_LENGTH+1];

    // save current team & scope
    saveP = CurrentTeamP;
    saveScope = CurrentMissionScope;
    strcpy(SaveMissionScopeName, CurrentMissionScopeName);

    CurrentTeamP = teamP;

#ifndef HW_Release
{
    char teamName[KAS_TEAM_NAME_MAX_LENGTH+1];
    aiplayerLog((aiCurrentAIPlayer->player->playerIndex,"\nKAS: FSMCreate(\"%s\", TEAM(\"%s\")", fsmName, kasAITeamName(teamP, teamName)));
}
#endif

    teamP->teamType = ScriptTeam;
    memStrncpy(teamP->kasFSMName, fsmName, KAS_FSM_NAME_MAX_LENGTH);
    teamP->kasFSMWatchFunction = watchFunction;
    teamP->kasStateWatchFunction = NULL;  // just in case it doesn't get set later
    teamP->kasTactics   = Neutral;        // default to neutral tactics
    teamP->kasFormation = NO_FORMATION;   // default to no formation

    snprintf(CurrentMissionScopeName, KAS_MISSION_NAME_MAX_LENGTH, "HASH_%08X", (udword)kasThisTeamPtr->kasLabelHash);

    initFunction();

    // restore current team & scope
    CurrentTeamP = saveP;
    CurrentMissionScope = saveScope;
    strcpy(CurrentMissionScopeName, SaveMissionScopeName);
}

//
//  all the processing of KAS controlled stuff happens here
//
//  must be called once per AI cycle
//
void kasExecute(void)
{
    sdword i;

    // no current mission?
    if (!CurrentMissionName[0] || !CurrentMissionWatchFunction)
        return;

    // watch for all objectives (if >= 1 defined) completed
#if 0
    if (timTimerExpiredDestroy("ObjectivesComplete"))
        kasfMissionCompleted();
    else if (!timTimerRemaining("ObjectivesComplete") && objectivesUsed && objectiveGetAll())
        timTimerCreateSetStart("ObjectivesComplete", OBJECTIVES_COMPLETE_DELAY);
#endif

    if (kasUnpausedTeam == NULL)
    {                                                       //if KAS is not paused
        CurrentMissionWatchFunction();          // watch at the mission level

        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            CurrentTeamP = aiCurrentAIPlayer->teams[i];
            if (CurrentTeamP->teamType == ScriptTeam)
            {
                if (CurrentTeamP->kasFSMWatchFunction)   // watch at the FSM level
                    CurrentTeamP->kasFSMWatchFunction();
                if (CurrentTeamP->kasStateWatchFunction) // watch at the state level
                    CurrentTeamP->kasStateWatchFunction();
            }
        }
    }
    else
    {                                                       //else all FSM's but one are paused
        CurrentTeamP = kasUnpausedTeam;                     //only execute this 1 unpaused team
        if (CurrentTeamP->teamType == ScriptTeam)
        {
            if (CurrentTeamP->kasFSMWatchFunction)   // watch at the FSM level
                CurrentTeamP->kasFSMWatchFunction();
            if (CurrentTeamP->kasStateWatchFunction) // watch at the state level
                CurrentTeamP->kasStateWatchFunction();
        }
    }
}
#endif  // HW1 kasFSMCreate / kasExecute - replaced by Cataclysm versions

//
//  starts a mission
//
void kasMissionStart(char *name, KASInitFunction initFunction, KASWatchFunction watchFunction)
{
    aiplayerLog((0,"\nKAS: Starting %s", name));
    memStrncpy(CurrentMissionName, name, KAS_MISSION_NAME_MAX_LENGTH);
    kasMissionWatchFunction = watchFunction;    // Cataclysm mission watch function

    //hyperspace init
    hsStaticInit(LabelledVectorsUsed);

    initFunction();         // must go last - save game is called at end of this function, we want everything setup
}

//
//  return reference to labelled team
//
AITeam *kasAITeamPtr(char *label)
{
    sdword i = 0;
    AITeam *aiteamp = NULL;

    while (i < aiCurrentAIPlayer->teamsUsed)
    {
        aiteamp = aiCurrentAIPlayer->teams[i];
        if (aiteamp->teamType == ScriptTeam && aiteamp->kasLabelHash == kasHashLabel(label))
            return aiteamp;
        ++i;
    }

    // special reference to sender of last message received
    if (!stricmp(label, "MsgSender"))
        return CurrentTeamP->msgSender;
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to team %s", label);
#endif
    return NULL;
}

//
//  return reference to labelled team's ships (HW1 version — internal use only)
//
static GrowSelection *kasAITeamShipsPtrByLabel(char *label)
{
    sdword i = 0;
    AITeam *aiteamp;

    while (i < aiCurrentAIPlayer->teamsUsed)
    {
        aiteamp = aiCurrentAIPlayer->teams[i];
        if (aiteamp->teamType == ScriptTeam && aiteamp->kasLabelHash == kasHashLabel(label))
            return &aiteamp->shipList;
        ++i;
    }
    // special reference to sender of last message received
    if (!stricmp(label, "MsgSender"))
        if (CurrentTeamP->msgSender)
            return &CurrentTeamP->msgSender->shipList;
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to team %s", label);
#endif
    return NULL;
}

//
//  return reference to the current team's centerpoint.  Cataclysm retail
//  ignores the label argument and always uses teamtoreinforce->shipList.
//  Translated from cataclysm.exe @ 0x4909F0.
//
hvector *kasShipsVectorPtr(char *label)
{
    static hvector errorPos= {0, 0, 0, 1};  // safe fallback
    static hvector location;
    vector loc;
    GrowSelection *grow;
    real32 dummy;

    (void)label;

    if (CurrentTeamP == NULL && kasCurrentTeamPtr != 0)
        CurrentTeamP = (AITeam *)kasCurrentTeamPtr;

    if (CurrentTeamP == NULL)
        return &errorPos;

    grow = &CurrentTeamP->shipList;

    if (grow && grow->selection && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    return &errorPos;
}

//
// return reference to the labelled team's centerpoint, or a safe fallback
// when the team does not exist or has no ships.  Translated from
// cataclysm.exe @ 0x4908C0.
//
hvector *kasTeamsVectorPtr(sdword labelHash)
{
    static hvector errorPos = {0,0,0,1};
    static hvector location;
    vector loc;
    GrowSelection *grow;
    real32 dummy;

    grow = kasAITeamShipsPtrIfExists(labelHash);
    if (grow && grow->selection && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    return &errorPos;
}

#if 0  // HW1 version - replaced by Cataclysm version
//
// return reference to labelled volume's center
//
hvector *kasVolumeVectorPtr(char *label)
{
    static hvector returnhvec;
    vector volcenter;
    Volume *vol = kasVolumePtr(label);

    if (vol)
    {
        volcenter = volFindCenter(vol);
        vecMakeHVecFromVec(returnhvec, volcenter);
        return &returnhvec;
    }
    else
    {
        return NULL;
    }
}
#endif

//
// return reference to the team's centerpoint
//
hvector *kasThisTeamsVectorPtr(void)
{
    static hvector errorPos = {0,0,0,1};
    static hvector location;
    vector loc;
    GrowSelection *grow;
    real32 dummy;

    if (CurrentTeamP == NULL && kasCurrentTeamPtr != 0)
        CurrentTeamP = (AITeam *)kasCurrentTeamPtr;

    grow = &CurrentTeamP->shipList;

    if (grow && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    else
    {
        return &errorPos;
    }

#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference in kasThisTeamsVectorPtr");
#endif
    return NULL;

}

//
//  return reference to labelled path
//
Path *kasPathPtrNoErrorChecking(sdword label)
{
    sdword i;
    sdword **table;

    table = (sdword **)kasLabelledPathTable;
    for (i = 0; i < kasLabelledPathCount; i++)
    {
        if (table[i][0] == label)
            return (Path *)table[i][1];
    }
    return NULL;
}

//
//  return reference to labelled path
//
Path *kasPathPtr(sdword label)
{
    Path *path = kasPathPtrNoErrorChecking(label);
    if (!path)
        dbgFatalf(DBG_Loc, "\nKAS: unresolved reference to path HASH_%08X", (udword)label);
    return path;
}

#if 0  // HW1 version - replaced by Cataclysm version
//
//  return reference to labelled volume
//
Volume *kasVolumePtr(char *label)
{
    sdword i = 0;
    while (i < LabelledVolumesUsed)
    {
        if (!strnicmp(LabelledVolumes[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return LabelledVolumes[i]->volume;
        ++i;
    }
#ifndef HW_Release
    dbgFatalf(DBG_Loc,"\nKAS: unresolved reference to volume %s", label);
#endif
    return NULL;
}
#endif

//
//  return reference to labelled point
//
hvector *kasVectorPtrIfExists(sdword label)
{
    sdword i;
    sdword **table;

    table = (sdword **)kasLabelledVectorTable;
    for (i = 0; i < kasLabelledVectorCount; i++)
    {
        if (table[i][0] == label)
            return (hvector *)table[i][1];
    }
    return NULL;
}

hvector *kasVectorPtr(sdword label)
{
    hvector *vec = kasVectorPtrIfExists(label);
    if (!vec)
        dbgFatalf(DBG_Loc, "\nKAS: unresolved reference to point HASH_%08X", (udword)label);
    return vec;
}

GrowSelection *kasGetGrowSelectionPtrIfExists(char *label)
{
    sdword i = 0;

    while (i < SelectionsUsed)
    {
        if (!strnicmp(Selections[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return &(Selections[i]->shipList);
        ++i;
    }

    return NULL;
}

#if 0  // HW1 version - replaced by Cataclysm version
//
//  on first reference to a new GrowSelection, create one and return its pointer
//  otherwise, return the pointer
//
GrowSelection *kasGrowSelectionPtr(char *label)
{
    sdword i = 0;

    while (i < SelectionsUsed)
    {
        if (!strnicmp(Selections[i]->label, label, KAS_MAX_LABEL_LENGTH))
            return &(Selections[i]->shipList);
        ++i;
    }

    if (!Selections)
    {
        Selections = memAlloc(sizeof(KasSelection *) * KASSELECTION_ALLOC_INCREMENT, "kasSelections", 0);
        SelectionsAllocated = KASSELECTION_ALLOC_INCREMENT;
        SelectionsUsed = 0;
    }
    // keep track of all allocations
    if (SelectionsUsed >= SelectionsAllocated)
    {
        // allocate more if necessary
        Selections = memRealloc(Selections, sizeof(KasSelection *) * (SelectionsAllocated + KASSELECTION_ALLOC_INCREMENT), "kasSelections", 0);
        SelectionsAllocated += KASSELECTION_ALLOC_INCREMENT;
    }
    Selections[SelectionsUsed] = memAlloc(sizeof(KasSelection), "kasGrowSelection", 0);
    memStrncpy(Selections[SelectionsUsed]->label, label, KAS_MAX_LABEL_LENGTH);
    growSelectInit(&Selections[SelectionsUsed]->shipList);

    return &(Selections[SelectionsUsed++]->shipList);
}
#endif

void kasGrowSelectionClear(GrowSelection *ships)
{
    if (ships && ships->selection)
    {
        ships->selection->numShips = 0;
    }
}

//
//  add new labelled path (with no points, yet), return pointer to it
//
Path *kasLabelledPathAdd(sdword label, sdword numPoints, sdword closed)
{
    sdword **table;
    sdword *entry;
    Path *path;

    if (kasLabelledPathTable)
    {
        if (kasLabelledPathCount >= kasLabelledPathCapacity)
        {
            kasLabelledPathTable = memRealloc(kasLabelledPathTable, 4 * kasLabelledPathCapacity + 64, "kasPaths", MBF_Volatile);
            kasLabelledPathCapacity += 16;
        }
    }
    else
    {
        kasLabelledPathTable = memAlloc(64, "kasPaths", MBF_Volatile);
        kasLabelledPathCapacity = 16;
        kasLabelledPathCount = 0;
    }

    table = (sdword **)kasLabelledPathTable;
    entry = memAlloc(8, "kasPath", MBF_NonVolatile);
    table[kasLabelledPathCount] = entry;
    path = aiuCreatePathStruct(numPoints, closed);
    entry[0] = label;
    entry[1] = (sdword)path;
    kasLabelledPathCount++;
    return path;
}

//
//  add new labelled volume (with no contents, yet), return pointer to it
//
Volume *kasLabelledVolumeAdd(sdword label)
{
    sdword **table;
    sdword *entry;
    Volume *volume;

    if (kasLabelledVolumeTable)
    {
        if (kasLabelledVolumeCount >= kasLabelledVolumeCapacity)
        {
            kasLabelledVolumeTable = memRealloc(kasLabelledVolumeTable, 4 * kasLabelledVolumeCapacity + 64, "kasVolumes", MBF_Volatile);
            kasLabelledVolumeCapacity += 16;
        }
    }
    else
    {
        kasLabelledVolumeTable = memAlloc(64, "kasVolumes", MBF_Volatile);
        kasLabelledVolumeCapacity = 16;
        kasLabelledVolumeCount = 0;
    }

    table = (sdword **)kasLabelledVolumeTable;
    entry = memAlloc(8, "kasVolume", MBF_NonVolatile);
    table[kasLabelledVolumeCount] = entry;
    volume = memAlloc(sizeof(Volume), "kasVolume", MBF_NonVolatile);
    entry[0] = label;
    entry[1] = (sdword)volume;
    kasLabelledVolumeCount++;
    return volume;
}

#if 0  // HW1 version - replaced by Cataclysm version
//
//  add new labelled vector, return pointer to it
//
hvector *kasLabelledVectorAdd(char *label, real32 x, real32 y, real32 z, real32 w)
{
    if (!LabelledVectors)
    {
        LabelledVectors = memAlloc(sizeof(LabelledVector *) * KAS_LABELLED_ENTITY_ALLOC_INCREMENT, "kasLabelledVectors", 0);
        LabelledVectorsAllocated = KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
        LabelledVectorsUsed = 0;
    }
    // keep track of all allocations
    if (LabelledVectorsUsed >= LabelledVectorsAllocated)
    {
        // allocate more if necessary
        LabelledVectors = memRealloc(LabelledVectors, sizeof(LabelledVector *) * (LabelledVectorsAllocated + KAS_LABELLED_ENTITY_ALLOC_INCREMENT), "kasLabelledVectors", 0);
        LabelledVectorsAllocated += KAS_LABELLED_ENTITY_ALLOC_INCREMENT;
    }
    LabelledVectors[LabelledVectorsUsed] = memAlloc(sizeof(LabelledVector), "kasLabelledVector", MBF_NonVolatile);
    LabelledVectors[LabelledVectorsUsed]->hvector = memAlloc(sizeof(hvector), "kasVector", MBF_NonVolatile);
    memStrncpy(LabelledVectors[LabelledVectorsUsed]->label, label, KAS_MAX_LABEL_LENGTH);
    LabelledVectors[LabelledVectorsUsed]->hvector->x = x;
    LabelledVectors[LabelledVectorsUsed]->hvector->y = y;
    LabelledVectors[LabelledVectorsUsed]->hvector->z = z;
    LabelledVectors[LabelledVectorsUsed]->hvector->w = w;

    return LabelledVectors[LabelledVectorsUsed++]->hvector;
}

//
//  remove all labelled entities
//
void kasLabelledEntitiesDestroy(void)
{
    sdword i;

    CurrentTeamP = NULL;

    // selections
    for (i = 0; i < SelectionsUsed; ++i)
    {
        growSelectClose(&Selections[i]->shipList);
        memFree(Selections[i]);
    }
    if (Selections)
        memFree(Selections);
    SelectionsUsed = 0;
    SelectionsAllocated = 0;
    Selections = NULL;

    // vectors
    for (i = 0; i < LabelledVectorsUsed; ++i)
    {
        memFree(LabelledVectors[i]->hvector);
        memFree(LabelledVectors[i]);
    }
    if (LabelledVectors)
        memFree(LabelledVectors);
    LabelledVectorsUsed = 0;
    LabelledVectorsAllocated = 0;
    LabelledVectors = NULL;

    // volumes
    for (i = 0; i < LabelledVolumesUsed; ++i)
    {
        memFree(LabelledVolumes[i]->volume);
        memFree(LabelledVolumes[i]);
    }
    if (LabelledVolumes)
        memFree(LabelledVolumes);
    LabelledVolumesUsed = 0;
    LabelledVolumesAllocated = 0;
    LabelledVolumes = NULL;

    // paths
    for (i = 0; i < LabelledPathsUsed; ++i)
    {
        memFree(LabelledPaths[i]->path);
        memFree(LabelledPaths[i]);
    }
    if (LabelledPaths)
        memFree(LabelledPaths);
    LabelledPathsUsed = 0;
    LabelledPathsAllocated = 0;
    LabelledPaths = NULL;

    // vars
    aivarDestroyAll();

    // timers
    timTimerDestroyAll();
}
#endif

//
//  Find team label from pointer.
//  Returns teamName (& fills it in).
//  If not found, returns "NULL" or something like that.
//
char *kasAITeamName(AITeam *team, char *teamName)
{
    if (team)
        sprintf(teamName, "HASH_%08X", (udword)team->kasLabelHash);
    else
        strcpy(teamName, "NULL");
    return teamName;
}

/*-----------------------------------------------------------------------------
    Name        : kasAddShipToTeam
    Description : utility function to add a ship to a team (creates team if doesn't exist)
    Inputs      : ship, str
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if 0  // HW1 version — replaced by Cataclysm version above
void kasAddShipToTeam(Ship *ship,char *label)
{
    sdword i;
    AITeam *teamp;

    // if labelled team doesn't currently exist, create it
    // otherwise, add ships to existing labelled team
    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        teamp = aiCurrentAIPlayer->teams[i];
        if (teamp->teamType == ScriptTeam && !strnicmp(teamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH))
            break;
    }
    if (i >= aiCurrentAIPlayer->teamsUsed)
    {
        teamp = aitCreate(ScriptTeam);
        memStrncpy(teamp->kasLabel, label, KAS_TEAM_NAME_MAX_LENGTH);
        teamp->kasOrigShipsCount = 0;
    }

    teamp->kasOrigShipsCount++;
    teamp->kasOrigShipsType = ship->shiptype;

    aitAddShip(teamp, ship);
    teamp->newships--;
}
#endif

//
//  remove ship reference from any ship lists
//
void kasShipDied(Ship *ship)
{
    sdword i;
    sdword **table = (sdword **)kasGrowSelectionTable;

    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        GrowSelection *grow = (GrowSelection *)&table[i][1];
        clRemoveTargetFromSelection((SelectAnyCommand *)grow->selection, (TargetPtr)ship);
    }
}

sdword kasRemoveTargetFromSelections(SpaceObjRotImpTarg *target)
{
    sdword i;
    sdword **table = (sdword **)kasGrowSelectionTable;

    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        GrowSelection *grow = (GrowSelection *)&table[i][1];
        clRemoveTargetFromSelection((SelectAnyCommand *)grow->selection, (TargetPtr)target);
    }
    return kasGrowSelectionCount;
}

//
//  wipe the labelled object tables clean
//  (do this before reading in a new mission layout)
//
void kasLabelsInit(void)
{
    LabelledPathsUsed = 0;
    LabelledVectorsUsed = 0;
    LabelledVolumesUsed = 0;
    kasLabelledPathCount = 0;
    kasLabelledVectorCount = 0;
    kasLabelledVolumeCount = 0;

    //hyperspace reset
    hsStaticReset();
}

//////////////////////////////////////////////////////////////////
//Encase all this debug stuff in this ifdef

//
//  this macro takes care of column wrapping
//  for the debug display
//
#define COLUMN_CHECK \
if (y > MAIN_WindowHeight - 2*rowHeight)    \
{                                         \
    y = rowHeight;                        \
    x += MAIN_WindowWidth/2 - rowHeight;  \
}


#if SP_DEBUGKEYS

/*-----------------------------------------------------------------------------
    Name        : kasTakeADump
    Description : Dumps all the singleplayer information into a file
    Inputs      : None
    Outputs     : A brand new file!
    Return      : void
----------------------------------------------------------------------------*/
void kasTakeADump(void)
{
    FILE *fp;
    char fileName[256] = "StatusDump.txt", *fullName;
    udword i, remaining;
    NamedTimer *tp;

    fullName = filePathPrepend(fileName, 0);

    fp = fopen(fullName, "wt");

    fprintf(fp, "%s\n\n", CurrentMissionName);

    //printout states
    fprintf(fp, "STATES:\n");
    for (i=0;i<aiCurrentAIPlayer->teamsUsed;i++)
    {
        CurrentTeamP = aiCurrentAIPlayer->teams[i];

        if (CurrentTeamP->teamType == ScriptTeam)
        {
            fprintf(fp, "HASH_%08X {%d} : FSM_%08X STATE_%08X\n",
                    (udword)CurrentTeamP->kasLabelHash,
                    CurrentTeamP->shipList.selection->numShips,
                    (udword)CurrentTeamP->kasFSMNameHash,
                    (udword)CurrentTeamP->kasStateWatchFunction);
        }
    }

    //printout variables
    fprintf(fp, "\nVARIABLES:\n");
    for (i = 0; i < varsUsed; ++i)
    {
        if (vars[i]->label[0] != '_' && vars[i]->label[1] != 'V')  // skip non KAS vars
        {
            fprintf(fp, "%s = %d\n", vars[i]->label, vars[i]->value);
        }
    }

    //printout timers
    fprintf(fp, "\nTIMERS:\n");
    for (i=0;i<timersUsed;i++)
    {
        tp = &timers[i];
        remaining = (sdword)(tp->duration - (universe.totaltimeelapsed - tp->startTime));
        if (remaining <= 0)
        {
            fprintf(fp, "[%d:%d] EXPIRED\n", tp->scopeId, tp->timerId);
        }
        else
        {
            fprintf(fp, "[%d:%d] %s : %ds\n", tp->scopeId, tp->timerId, tp->enabled ? "ON" : "OFF", remaining);
        }
    }
    fclose(fp);
    clCommandMessage("Taking a Dump");
}

/*-----------------------------------------------------------------------------
    Name        : kasCountDisplayLines
    Description : Counts the number of lines the debug draw routine will be drawing
    Inputs      : none
    Outputs     : none
    Return      : Returns the number of lines to draw
----------------------------------------------------------------------------*/
udword kasCountDisplayLines(udword *states, udword *variables)
{
    udword display = 0, i=0;

    *states    = 0;
    *variables = 0;

    //count the number of states to draw
    if (mrKASDebugDrawStates)
    {
        for (i=0; i<aiCurrentAIPlayer->teamsUsed;i++)
        {
            if (aiCurrentAIPlayer->teams[i]->teamType == ScriptTeam)
            {
                display++;
                *states++;
            }
        }
    }
    //count the number of Variables to draw
    if (mrKASDebugDrawVars)
    {
        for (i=0; i<varsUsed;i++)
        {
            if (vars[i]->label[0] != '_' && vars[i]->label[1] != 'V')
            {
                display++;
                *variables++;
            }
        }
    }
    //count the number of Timers to draw
    if (mrKASDebugDrawTimers)
    {
        display += timersUsed;
    }
    return display;
}

#define KAS_NUM_SCREEN_LINES 80
void kasDebugDraw(void)
{
    sdword rowHeight, y, x, i;
//    fonthandle fhSave;
    AITeam *CurrentTeamP;
    sdword remaining;
    NamedTimer *tp;
    char buf[256];
    vector vec0, vec1;
    Volume *volumep;
    udword numlines = 0, numstates = 0, numvars = 0, skiplines = 0;

    if (!gameIsRunning)
        return;

    if (!CurrentMissionName[0] || !kasMissionWatchFunction)
        return;

    // set up textual stuff
//    fhSave = fontCurrentGet();                  //save the current font
//    fontMakeCurrent(selGroupFont2);  // use a common, fairly small font

    rowHeight = fontHeight("M")-2; // used to space the text
    y = rowHeight;
    x = (sdword)rowHeight;

#if SP_DEBUGLEVEL2
    //count the number of lines to display
    numlines  = kasCountDisplayLines(&numstates, &numvars);
    skiplines = mrKASSkipFactor*KAS_NUM_SCREEN_LINES;

    // if we're displaying less than the screen can hold,
    // or if the user has paged over
    if ((numlines < KAS_NUM_SCREEN_LINES) ||
        (numlines < skiplines))
    {
        skiplines       = 0;
        mrKASSkipFactor = 0;
    }

    // states
    if (mrKASDebugDrawStates)
    {
        // mission name
        fontPrint(x-rowHeight/2, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, CurrentMissionName);
        y += rowHeight + 1;
        COLUMN_CHECK

        // each team's state
        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            CurrentTeamP = aiCurrentAIPlayer->teams[i];
            if (CurrentTeamP->teamType == ScriptTeam)
            {
                if (skiplines)
                {
                    skiplines--;
                }
                else
                {
                    sprintf(buf, "HASH_%08X {%d} : FSM_%08X STATE_%08X",
                            (udword)CurrentTeamP->kasLabelHash,
                            CurrentTeamP->shipList.selection->numShips,
                            (udword)CurrentTeamP->kasFSMNameHash,
                            (udword)CurrentTeamP->kasStateWatchFunction);
                    fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
                    y += rowHeight + 1;
                    COLUMN_CHECK
                }
            }
        }
        y += rowHeight/3;
        COLUMN_CHECK
    }

    // vars
    if (mrKASDebugDrawVars)
    {
        sprintf(buf,"Enemy RU = %d",universe.players[1].resourceUnits);
        fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
        y += rowHeight + 1;
        COLUMN_CHECK

        for (i = 0; i < varsUsed; ++i)
        {
            if (vars[i]->label[0] != '_' && vars[i]->label[1] != 'V')  // skip non KAS vars
            {
                if (skiplines)
                {
                    skiplines--;
                }
                else
                {
                    sprintf(buf, "%s = %d", vars[i]->label, vars[i]->value);
                    fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
                    y += rowHeight + 1;
                    COLUMN_CHECK
                }
            }
        }
        y += rowHeight/3;
        COLUMN_CHECK
    }

    // timers
    if (mrKASDebugDrawTimers)
    {
        for (i = 0; i < timersUsed; ++i)
        {
            if (skiplines)
            {
                skiplines--;
            }
            else
            {
                tp = &timers[i];
                remaining = (sdword)(tp->duration - (universe.totaltimeelapsed - tp->startTime));
                if (remaining <= 0)
                    sprintf(buf, "[%d:%d] EXPIRED", tp->scopeId, tp->timerId);
                else
                    sprintf(buf, "[%d:%d] %s : %ds", tp->scopeId, tp->timerId, tp->enabled ? "ON" : "OFF", remaining);
                fontPrint(x, y + (rowHeight>>2), KAS_DEBUG_TEXTCOLOR, buf);
                y += rowHeight + 1;
                COLUMN_CHECK
            }
        }
        y += rowHeight/3;
        COLUMN_CHECK
    }

    // volumes
    if (mrKASDebugDrawVolumes)
    {
        primModeClear2();
        rndLightingEnable(FALSE);
        rndTextureEnable(FALSE);
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        rgluPerspective(mrCamera->fieldofview, rndAspectRatio, mrCamera->clipPlaneNear, mrCamera->clipPlaneFar * 15.0f);
        glMatrixMode(GL_MODELVIEW);

        #if 0
        {
            Ship *m0p = aiuFindEnemyMothership(&universe.players[0]),
                 *m1p = aiuFindEnemyMothership(&universe.players[1]);
            primCircleOutline3(&m0p->posinfo.position,
                               10000,
                               32, 4, KAS_DEBUG_TEXTCOLOR, X_AXIS);
            primCircleOutline3(&m0p->posinfo.position,
                               10000,
                               32, 4, KAS_DEBUG_TEXTCOLOR, Y_AXIS);
            primCircleOutline3(&m0p->posinfo.position,
                               10000,
                               32, 8, KAS_DEBUG_TEXTCOLOR, Z_AXIS);
            primCircleOutline3(&m1p->posinfo.position,
                               10000,
                               32, 0, KAS_DEBUG_TEXTCOLOR, X_AXIS);
            primCircleOutline3(&m1p->posinfo.position,
                               10000,
                               32, 0, KAS_DEBUG_TEXTCOLOR, Y_AXIS);
            primCircleOutline3(&m1p->posinfo.position,
                               10000,
                               32, 8, KAS_DEBUG_TEXTCOLOR, Z_AXIS);

        }
        #endif

        for (i = 0; i < LabelledVolumesUsed; ++i)
        {
            volumep = LabelledVolumes[i]->volume;
            switch (volumep->type)
            {
                case VOLUME_AA_BOX:
                    vec0.x = volumep->attribs.aaBox.x0;  vec1.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y0;  vec1.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x0;
                    vec1.y = volumep->attribs.aaBox.y1;
                    vec1.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x0;
                    vec0.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z0;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x0;
                    vec1.y = volumep->attribs.aaBox.y0;
                    vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x1;
                    vec1.y = volumep->attribs.aaBox.y1;
                    vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x0;
                    vec0.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec1, &vec0, KAS_DEBUG_TEXTCOLOR);
                    vec1.x = volumep->attribs.aaBox.x0;
                    vec1.y = volumep->attribs.aaBox.y0;
                    vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x0;  vec1.x = volumep->attribs.aaBox.x0;
                    vec0.y = volumep->attribs.aaBox.y1;  vec1.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;  vec1.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y1;  vec1.y = volumep->attribs.aaBox.y1;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    vec0.x = volumep->attribs.aaBox.x1;  vec1.x = volumep->attribs.aaBox.x1;
                    vec0.y = volumep->attribs.aaBox.y0;  vec1.y = volumep->attribs.aaBox.y0;
                    vec0.z = volumep->attribs.aaBox.z0;  vec1.z = volumep->attribs.aaBox.z1;
                    primLine3(&vec0, &vec1, KAS_DEBUG_TEXTCOLOR);
                    break;

                case VOLUME_SPHERE:
                    primCircleOutline3(&volumep->attribs.sphere.center,
                                       volumep->attribs.sphere.radius,
                                       32, 4, KAS_DEBUG_TEXTCOLOR, X_AXIS);
                    primCircleOutline3(&volumep->attribs.sphere.center,
                                       volumep->attribs.sphere.radius,
                                       32, 4, KAS_DEBUG_TEXTCOLOR, Y_AXIS);
                    primCircleOutline3(&volumep->attribs.sphere.center,
                                       volumep->attribs.sphere.radius,
                                       32, 4, KAS_DEBUG_TEXTCOLOR, Z_AXIS);
                    break;
            }
        }

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf((GLfloat*)&rndProjectionMatrix);
        glMatrixMode(GL_MODELVIEW);
        primModeSet2();
    }
#endif //SP_DEBUGLEVEL2

//    fontMakeCurrent(fhSave);
}
#endif //SP_DEBUGKEYS

/*=============================================================================
    Save Game Stuff
=============================================================================*/

void SaveKasSelection(KasSelection *kasselection)
{
    SaveStructureOfSize(kasselection,sizeof(KasSelection));
    SaveGrowSelection(&kasselection->shipList);
}

// Build a placeholder label string from a 32-bit hash (for backward-compat loads).
static void kasMakeDebugLabelFromHash(char *label, sdword labelHash)
{
    sprintf(label, "hash_%08X", (udword)labelHash);
}

KasSelection *LoadKasSelection(void)
{
    KasSelection *kasselection;
    SaveChunk *chunk = LoadNextChunk();

    kasselection = memAlloc(sizeof(KasSelection), "kasGrowSelection", 0);
    memset(kasselection, 0, sizeof(KasSelection));

    if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == sizeof(KasSelection))
    {
        memcpy(kasselection, chunkContents(chunk), sizeof(KasSelection));
    }
    else if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == 12)
    {
        // Compact format from intermediate save: {labelHash, maxNumShips, selPtr}
        kasMakeDebugLabelFromHash(kasselection->label, *(sdword *)chunkContents(chunk));
    }
    else
    {
        dbgFatalf(DBG_Loc, "LoadKasSelection: unexpected type=0x%x size=%d", chunk->type, chunk->contentsSize);
    }
    memFree(chunk);
    LoadGrowSelectionAndFix(&kasselection->shipList);
    return kasselection;
}

void SaveLabelledPath(LabelledPath *labelledpath)
{
    SaveStructureOfSize(labelledpath,sizeof(LabelledPath));
    SavePath(labelledpath->path);
}

LabelledPath *LoadLabelledPath(void)
{
    LabelledPath *labelledpath;
    SaveChunk *chunk = LoadNextChunk();

    labelledpath = memAlloc(sizeof(LabelledPath), "kasLabelledPath", 0);
    memset(labelledpath, 0, sizeof(LabelledPath));

    if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == sizeof(LabelledPath))
    {
        memcpy(labelledpath, chunkContents(chunk), sizeof(LabelledPath));
    }
    else if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == 8)
    {
        kasMakeDebugLabelFromHash(labelledpath->label, *(sdword *)chunkContents(chunk));
    }
    else
    {
        dbgFatalf(DBG_Loc, "LoadLabelledPath: unexpected type=0x%x size=%d", chunk->type, chunk->contentsSize);
    }
    memFree(chunk);
    labelledpath->path = LoadPath();
    return labelledpath;
}

void SaveLabelledVector(LabelledVector *labelledvector)
{
    SaveStructureOfSize(labelledvector,sizeof(LabelledVector));
    SaveStructureOfSize(labelledvector->hvector,sizeof(hvector));
}

LabelledVector *LoadLabelledVector(void)
{
    LabelledVector *labelledvector;
    SaveChunk *chunk = LoadNextChunk();

    labelledvector = memAlloc(sizeof(LabelledVector), "kasLabelledVector", 0);
    memset(labelledvector, 0, sizeof(LabelledVector));

    if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == sizeof(LabelledVector))
    {
        memcpy(labelledvector, chunkContents(chunk), sizeof(LabelledVector));
    }
    else if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == 8)
    {
        kasMakeDebugLabelFromHash(labelledvector->label, *(sdword *)chunkContents(chunk));
    }
    else
    {
        dbgFatalf(DBG_Loc, "LoadLabelledVector: unexpected type=0x%x size=%d", chunk->type, chunk->contentsSize);
    }
    memFree(chunk);
    labelledvector->hvector = LoadStructureOfSize(sizeof(hvector));
    return labelledvector;
}

void SaveLabelledVolume(LabelledVolume *labelledvolume)
{
    SaveStructureOfSize(labelledvolume,sizeof(LabelledVolume));
    SaveStructureOfSize(labelledvolume->volume,sizeof(Volume));
}

LabelledVolume *LoadLabelledVolume(void)
{
    LabelledVolume *labelledvolume;
    SaveChunk *chunk = LoadNextChunk();

    labelledvolume = memAlloc(sizeof(LabelledVolume), "kasLabelledVolume", 0);
    memset(labelledvolume, 0, sizeof(LabelledVolume));

    if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == sizeof(LabelledVolume))
    {
        memcpy(labelledvolume, chunkContents(chunk), sizeof(LabelledVolume));
    }
    else if (chunk->type == BASIC_STRUCTURE && chunk->contentsSize == 8)
    {
        kasMakeDebugLabelFromHash(labelledvolume->label, *(sdword *)chunkContents(chunk));
    }
    else
    {
        dbgFatalf(DBG_Loc, "LoadLabelledVolume: unexpected type=0x%x size=%d", chunk->type, chunk->contentsSize);
    }
    memFree(chunk);
    labelledvolume->volume = LoadStructureOfSize(sizeof(Volume));
    return labelledvolume;
}

void kasSave(void)
{
    sdword i;
    sdword **table;

    SaveHyperspaceGates();

    SaveInfoNumber(kasCurrentStep);
    SaveInfoNumber(kasCurrentScopeHash);
    SaveStructureOfSize(CurrentMissionName, 48);

    SaveInfoNumber(WatchFunctionToIndex(kasMissionWatchFunction));
    SaveInfoNumber(AITeamToTeamIndex((AITeam *)kasCurrentTeamPtr));
    SaveInfoNumber(g_GameDifficulty);

    SaveInfoNumber(kasGrowSelectionCount);
    SaveInfoNumber(kasGrowSelectionCapacity);
    table = (sdword **)kasGrowSelectionTable;
    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        SaveStructureOfSize(table[i], 12);
        SaveGrowSelection((GrowSelection *)((ubyte *)table[i] + 4));
    }

    SaveInfoNumber(kasLabelledPathCount);
    SaveInfoNumber(kasLabelledPathCapacity);
    table = (sdword **)kasLabelledPathTable;
    for (i = 0; i < kasLabelledPathCount; i++)
    {
        SaveStructureOfSize(table[i], 8);
        SavePath((Path *)table[i][1]);
    }

    SaveInfoNumber(kasLabelledVectorCount);
    SaveInfoNumber(kasLabelledVectorCapacity);
    table = (sdword **)kasLabelledVectorTable;
    for (i = 0; i < kasLabelledVectorCount; i++)
    {
        SaveStructureOfSize(table[i], 8);
        SaveStructureOfSize((void *)table[i][1], 16);
    }

    SaveInfoNumber(kasLabelledVolumeCount);
    SaveInfoNumber(kasLabelledVolumeCapacity);
    table = (sdword **)kasLabelledVolumeTable;
    for (i = 0; i < kasLabelledVolumeCount; i++)
    {
        SaveStructureOfSize(table[i], 8);
        SaveStructureOfSize((void *)table[i][1], 28);
    }

    timTimerSave();

    if (singlePlayerGame)
    {
        SaveInfoNumber(rmCanBuildShip(&universe.players[0], sMothershipArmour));
        SaveInfoNumber(rmCanBuildShip(&universe.players[0], sMothershipDockingBay));
        SaveInfoNumber(rmCanBuildShip(&universe.players[0], sMothershipMicro));
        SaveInfoNumber(rmCanBuildShip(&universe.players[0], sMothershipSpecial));
        SaveInfoNumber(rmCanBuildShip(&universe.players[0], sMothershipWeapons));
        SaveInfoNumber(rmCanBuildShip(&universe.players[0], dAcolyte));
    }
}

void kasLoad(void)
{
    sdword i;
    sdword **table;
    sdword *entry;
    sdword teamIndex;
    sdword savedDifficulty;
    sdword missionHash;
    real32 savedPrimaryModifier;
    real32 currentPrimaryModifier;

    LoadHyperspaceGates();

    kasCurrentStep = LoadInfoNumber();
    kasCurrentScopeHash = LoadInfoNumber();
    LoadStructureOfSizeToAddress(CurrentMissionName, 48);

    kasMissionWatchFunction = (KASWatchFunction)(uintptr_t)IndexToWatchFunction(LoadInfoNumber());
    CurrentMissionWatchFunction = kasMissionWatchFunction;
    teamIndex = LoadInfoNumber();
    kasCurrentTeamPtr = (sdword)AITeamIndexToTeam(aiCurrentAIPlayer, teamIndex);
    savedDifficulty = LoadInfoNumber();
    gameDifficultyGetModifiers(&savedPrimaryModifier, NULL, NULL, savedDifficulty);
    gameDifficultyGetModifiers(&currentPrimaryModifier,
                               &GameDifficultyBuildTimeModifier,
                               &GameDifficultyProductionModifier,
                               g_GameDifficulty);
    GameDifficultyPrimaryModifier = currentPrimaryModifier / savedPrimaryModifier;

    kasGrowSelectionCount = LoadInfoNumber();
    kasGrowSelectionCapacity = LoadInfoNumber();
    if (kasGrowSelectionCapacity > 0)
    {
        kasGrowSelectionTable = memAlloc(4 * kasGrowSelectionCapacity, "kasFlags", MBF_Volatile);
    }
    else
        kasGrowSelectionTable = NULL;
    table = (sdword **)kasGrowSelectionTable;
    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        entry = memAlloc(12, "kasFlag", MBF_Volatile);
        LoadStructureOfSizeToAddress(entry, 12);
        LoadGrowSelectionAndFix((GrowSelection *)&entry[1]);
        table[i] = entry;
    }

    kasLabelledPathCount = LoadInfoNumber();
    kasLabelledPathCapacity = LoadInfoNumber();
    if (kasLabelledPathCapacity > 0)
    {
        kasLabelledPathTable = memAlloc(4 * kasLabelledPathCapacity, "kasPaths", MBF_Volatile);
    }
    else
        kasLabelledPathTable = NULL;
    table = (sdword **)kasLabelledPathTable;
    for (i = 0; i < kasLabelledPathCount; i++)
    {
        entry = memAlloc(8, "kasPath", MBF_Volatile);
        LoadStructureOfSizeToAddress(entry, 8);
        entry[1] = (sdword)LoadPath();
        table[i] = entry;
    }

    kasLabelledVectorCount = LoadInfoNumber();
    kasLabelledVectorCapacity = LoadInfoNumber();
    if (kasLabelledVectorCapacity > 0)
    {
        kasLabelledVectorTable = memAlloc(4 * kasLabelledVectorCapacity, "kasVectors", MBF_Volatile);
    }
    else
        kasLabelledVectorTable = NULL;
    table = (sdword **)kasLabelledVectorTable;
    for (i = 0; i < kasLabelledVectorCount; i++)
    {
        entry = memAlloc(8, "kasVector", MBF_Volatile);
        LoadStructureOfSizeToAddress(entry, 8);
        entry[1] = (sdword)LoadStructureOfSize(16);
        table[i] = entry;
    }

    kasLabelledVolumeCount = LoadInfoNumber();
    kasLabelledVolumeCapacity = LoadInfoNumber();
    if (kasLabelledVolumeCapacity > 0)
    {
        kasLabelledVolumeTable = memAlloc(4 * kasLabelledVolumeCapacity, "kasVolumes", MBF_Volatile);
    }
    else
        kasLabelledVolumeTable = NULL;
    table = (sdword **)kasLabelledVolumeTable;
    for (i = 0; i < kasLabelledVolumeCount; i++)
    {
        entry = memAlloc(8, "kasVolume", MBF_Volatile);
        LoadStructureOfSizeToAddress(entry, 8);
        entry[1] = (sdword)LoadStructureOfSize(28);
        table[i] = entry;
    }

    timTimerLoad();

    missionHash = kasHashLabel(CurrentMissionName);
    entry = kasfEntryFindByScopeAndId(0, missionHash);
    if (entry == NULL)
        entry = kasfEntryFindOrCreateByScopeAndId(0, missionHash);
    entry[2] = 1;

    if (singlePlayerGame)
    {
        rmEnableShip(RACE_Sect, sMothershipArmour, LoadInfoNumber());
        rmEnableShip(RACE_Sect, sMothershipDockingBay, LoadInfoNumber());
        rmEnableShip(RACE_Sect, sMothershipMicro, LoadInfoNumber());
        rmEnableShip(RACE_Sect, sMothershipSpecial, LoadInfoNumber());
        rmEnableShip(RACE_Sect, sMothershipWeapons, LoadInfoNumber());
        rmEnableShip(RACE_Sect, dAcolyte, LoadInfoNumber());
    }
}



// ===========================================================================
//  Cataclysm KAS runtime (matches cataclysm.exe).
//
//  Cataclysm replaced HW1's string-keyed labelled-entity arrays with
//  hash-keyed tables.  Each table is a memAlloc'd array of pointers; each
//  pointer references a small record whose first dword is the label hash and
//  whose remaining bytes hold the payload.  AITeam KAS fields are accessed by
//  their raw binary offsets because Cataclysm stores label hashes (ints) where
//  HW1 stored character arrays.
//
//  AITeam raw dword offsets used below:
//    [0]  teamType         (ScriptTeam == 3)
//    [7]  shipList         (GrowSelection, offset 28)
//    [22] kasLabel hash    (offset 88)
//    [23] kasFSMName hash  (offset 92)
//    [25] kasFSMWatchFunction   (offset 100)
//    [26] kasStateWatchFunction (offset 104)
//    [29] kasFormation     (offset 116)
//    [30] kasTactics       (offset 120)
// ===========================================================================

void kasClose(void)
{
    kasSingleExecutionTeamPtr = 0;
    kasRuntimeCounter = 0;
}

void kasInit(void)
{
    kasRuntimeCounter = 0;
}

//
//  FSMCreate keyword: bind an FSM/state to a team and run its init function.
//
void kasFSMCreate(sdword fsmNameHash, KASInitFunction initFunction, KASWatchFunction watchFunction, AITeam *teamP)
{
    AITeam *saveCurrentTeam;
    sdword saveScope;
    sdword saveTeam;
    sdword saveStep;

    saveCurrentTeam = CurrentTeamP;
    saveScope = kasCurrentScopeHash;
    saveTeam  = kasCurrentTeamPtr;
    kasCurrentTeamPtr = (sdword)teamP;
    saveStep  = kasCurrentStep;
    CurrentTeamP = teamP;

    ((sdword *)teamP)[0]  = ScriptTeam;
    teamP->kasFSMNameHash        = fsmNameHash;
    teamP->kasFSMWatchFunction   = watchFunction;
    teamP->kasStateWatchFunction = NULL;
    teamP->kasTactics            = 1;
    teamP->kasFormation          = 7;

    kasCurrentScopeHash = teamP->kasLabelHash;
    initFunction();

    kasCurrentStep = saveStep;
    kasCurrentTeamPtr = saveTeam;
    kasCurrentScopeHash = saveScope;
    CurrentTeamP = saveCurrentTeam;
}

//
//  all the processing of KAS controlled stuff happens here, once per AI cycle.
//
void kasExecute(void)
{
    sdword i;
    AITeam *teamP;
    AITeam *saveCurrentTeam;
    KASWatchFunction fsmWatch;
    KASWatchFunction stateWatch;

    if (!CurrentMissionName[0] || !kasMissionWatchFunction)
        return;

    saveCurrentTeam = CurrentTeamP;

    if (kasSingleExecutionTeamPtr)
    {
        // a single FSM is unpaused; execute only it
        kasCurrentTeamPtr = kasSingleExecutionTeamPtr;
        CurrentTeamP = (AITeam *)kasCurrentTeamPtr;
        if (((AITeam *)kasSingleExecutionTeamPtr)->teamType == ScriptTeam)
        {
            fsmWatch = ((AITeam *)kasSingleExecutionTeamPtr)->kasFSMWatchFunction;
            if (fsmWatch)
                fsmWatch();
            stateWatch = ((AITeam *)kasCurrentTeamPtr)->kasStateWatchFunction;
            if (stateWatch)
                stateWatch();
        }
    }
    else
    {
        kasMissionWatchFunction();                                 // watch at the mission level
        for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
        {
            teamP = aiCurrentAIPlayer->teams[i];
            kasCurrentTeamPtr = (sdword)teamP;
            CurrentTeamP = teamP;
            if (teamP->teamType == ScriptTeam)
            {
                fsmWatch = teamP->kasFSMWatchFunction;
                if (fsmWatch)
                {
                    fsmWatch();                         // watch at the FSM level
                    teamP = (AITeam *)kasCurrentTeamPtr;
                    CurrentTeamP = teamP;
                }
                stateWatch = teamP->kasStateWatchFunction;
                if (stateWatch)
                    stateWatch();                       // watch at the state level
            }
        }
    }

    CurrentTeamP = saveCurrentTeam;
}

//
//  on first reference to a new GrowSelection, create one; otherwise return it.
//
GrowSelection *kasGrowSelectionPtr(sdword label)
{
    sdword i;
    sdword *entry;
    sdword **table;

    table = (sdword **)kasGrowSelectionTable;
    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        if (table[i][0] == label)
            return (GrowSelection *)&table[i][1];
    }

    if (kasGrowSelectionTable)
    {
        if (kasGrowSelectionCount >= kasGrowSelectionCapacity)
        {
            kasGrowSelectionTable = memRealloc(kasGrowSelectionTable, 4 * kasGrowSelectionCapacity + 64, "kasFlags", MBF_Volatile);
            kasGrowSelectionCapacity += 16;
        }
    }
    else
    {
        kasGrowSelectionTable = memAlloc(64, "kasFlags", MBF_Volatile);
        kasGrowSelectionCapacity = 16;
        kasGrowSelectionCount = 0;
    }
    table = (sdword **)kasGrowSelectionTable;
    entry = memAlloc(12, "kasFlag", MBF_Volatile);
    table[kasGrowSelectionCount] = entry;
    entry[0] = label;
    growSelectInit((GrowSelection *)&entry[1]);
    return (GrowSelection *)&table[kasGrowSelectionCount++][1];
}

//
//  return reference to an existing GrowSelection ("flag"), or NULL.
//
sdword *kasFlagsPtrIfExists(sdword flagNameHash)
{
    sdword i;
    sdword **table;

    table = (sdword **)kasGrowSelectionTable;
    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        if (table[i][0] == flagNameHash)
            return &table[i][1];
    }
    return NULL;
}

//
//  return reference to the centre of a labelled ship selection, or a safe
//  fallback hvector when the selection does not exist or is empty.
//  Translated from cataclysm.exe @ 0x490820.
//
hvector *kasShipsVectorPtrIfExists(sdword labelHash)
{
    static hvector errorPos = {0, 0, 0, 1};
    static hvector location;
    vector loc;
    real32 dummy;
    GrowSelection *grow;

    grow = (GrowSelection *)kasFlagsPtrIfExists(labelHash);
    if (grow && grow->selection && grow->selection->numShips)
    {
        loc = selCentrePointComputeGeneral((MaxSelection *)grow->selection, &dummy);
        vecMakeHVecFromVec(location, loc);
        return &location;
    }
    return &errorPos;
}

//
//  return reference to labelled volume, or NULL.
//
Volume *kasVolumePtr(sdword label)
{
    sdword i;
    sdword **table;

    table = (sdword **)kasLabelledVolumeTable;
    for (i = 0; i < kasLabelledVolumeCount; i++)
    {
        if (table[i][0] == label)
            return *(Volume **)&table[i][1];
    }
    return NULL;
}

//
//  return reference to a labelled volume's center, or NULL.
//
hvector *kasVolumeVectorPtr(sdword label)
{
    Volume *vol = kasVolumePtr(label);
    vector volcenter;

    if (vol)
    {
        volcenter = volFindCenter(vol);
        vecMakeHVecFromVec(kasVolumeVectorResult, volcenter);
        return &kasVolumeVectorResult;
    }
    return NULL;
}

//
//  return the script team carrying the given label hash, or NULL.
//
AITeam *kasAITeamPtrIfExists(sdword labelHash)
{
    sdword i;
    AITeam *teamP;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        teamP = aiCurrentAIPlayer->teams[i];
        if (teamP->teamType == ScriptTeam && teamP->kasLabelHash == labelHash)
            return teamP;
    }
    return NULL;
}

//
//  return the ship GrowSelection of the script team carrying the given label
//  hash, or NULL.
//
GrowSelection *kasAITeamShipsPtrIfExists(sdword labelHash)
{
    sdword i;
    AITeam *teamP;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        teamP = aiCurrentAIPlayer->teams[i];
        if (teamP->teamType == ScriptTeam && teamP->kasLabelHash == labelHash)
            return &teamP->shipList;  // binary uses [7]=28; our struct has shipList at [6]=24
    }
    return NULL;
}

//
//  Set colorScheme of every ship in the team selection pointed to by entry.
//  entry is the value returned by kasAITeamShipsPtrIfExists.
//  Translated from cataclysm.exe @ 0x49aae0.
//
void kasAITeamShipsPtr(GrowSelection *entry, sdword value)
{
    sdword *inner;
    sdword numShips;
    sdword i;

    inner = (sdword *)(((sdword *)entry)[1]);
    numShips = inner[0];
    for (i = 0; i < numShips; i++)
    {
        Ship *ship = (Ship *)inner[2 + i];
        ship->colorScheme = (sbyte)value;
    }
}

//
//  Minimal FSM watch function used when a team needs no special logic.
//  Sets kasCurrentStep = 1 and updates scope from the current team.
//  Translated from cataclysm.exe @ 0x62ca80.
//
void Watch_NoOpTeamContext(void)
{
    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
}

//
//  label key and the 4 coordinate dwords verbatim in the vector hash table.
//
hvector *kasLabelledVectorAdd(sdword label, real32 x, real32 y, real32 z, real32 w)
{
    sdword **table;
    sdword *entry;
    hvector *vec;

    if (kasLabelledVectorTable)
    {
        if (kasLabelledVectorCount >= kasLabelledVectorCapacity)
        {
            kasLabelledVectorTable = memRealloc(kasLabelledVectorTable, 4 * kasLabelledVectorCapacity + 64, "kasVectors", MBF_Volatile);
            kasLabelledVectorCapacity += 16;
        }
    }
    else
    {
        kasLabelledVectorTable = memAlloc(64, "kasVectors", MBF_Volatile);
        kasLabelledVectorCapacity = 16;
        kasLabelledVectorCount = 0;
    }
    table = (sdword **)kasLabelledVectorTable;
    entry = memAlloc(8, "kasVector", MBF_NonVolatile);
    table[kasLabelledVectorCount] = entry;
    vec = memAlloc(sizeof(hvector), "kasVec", MBF_NonVolatile);
    entry[0] = label;
    entry[1] = (sdword)vec;
    vec->x = x;
    vec->y = y;
    vec->z = z;
    vec->w = w;
    kasLabelledVectorCount++;
    return vec;
}

//
//  remove all labelled entities (selections/flags, vectors, volumes, paths).
//
void kasLabelledEntitiesDestroy(void)
{
    sdword i;
    sdword **table;

    kasCurrentTeamPtr = 0;

    // selections / flags
    table = (sdword **)kasGrowSelectionTable;
    for (i = 0; i < kasGrowSelectionCount; i++)
    {
        growSelectClose((GrowSelection *)&table[i][1]);
        memFree(table[i]);
    }
    if (kasGrowSelectionTable)
        memFree(kasGrowSelectionTable);
    kasGrowSelectionCount = 0;
    kasGrowSelectionCapacity = 0;
    kasGrowSelectionTable = NULL;

    // vectors
    table = (sdword **)kasLabelledVectorTable;
    for (i = 0; i < kasLabelledVectorCount; i++)
    {
        memFree((void *)table[i][1]);
        memFree(table[i]);
    }
    if (kasLabelledVectorTable)
        memFree(kasLabelledVectorTable);
    kasLabelledVectorCount = 0;
    kasLabelledVectorCapacity = 0;
    kasLabelledVectorTable = NULL;

    // volumes
    table = (sdword **)kasLabelledVolumeTable;
    for (i = 0; i < kasLabelledVolumeCount; i++)
    {
        memFree((void *)table[i][1]);
        memFree(table[i]);
    }
    if (kasLabelledVolumeTable)
        memFree(kasLabelledVolumeTable);
    kasLabelledVolumeCount = 0;
    kasLabelledVolumeCapacity = 0;
    kasLabelledVolumeTable = NULL;

    // paths
    table = (sdword **)kasLabelledPathTable;
    for (i = 0; i < kasLabelledPathCount; i++)
    {
        memFree((void *)table[i][1]);
        memFree(table[i]);
    }
    if (kasLabelledPathTable)
        memFree(kasLabelledPathTable);
    kasLabelledPathCount = 0;
    kasLabelledPathCapacity = 0;
    kasLabelledPathTable = NULL;

    aivarShutdown();
    kasfEntryDestroyAll();
    kasLabelledTimerDestroyAll();
}






hvector *kasLabelledVectorSet(sdword label, hvector *source)
{
    hvector *result;

    result = kasVectorPtrIfExists(label);
    if (result == NULL)
    {
        return kasLabelledVectorAdd(label, source->x, source->y, source->z, source->w);
    }
    *result = *source;
    return result;
}
