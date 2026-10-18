#ifndef ___KAS_H
#define ___KAS_H

#include "AIUtilities.h"
#include "ShipSelect.h"
#include "Vector.h"

// run-time scoping for variables, timers, etc.
enum {
    KAS_SCOPE_MISSION,
    KAS_SCOPE_FSM,
    KAS_SCOPE_STATE};

typedef void (*KASInitFunction) (void);
typedef void (*KASWatchFunction) (void);

//  these have corresponding #defines in kas2c.h
#define KAS_MISSION_NAME_MAX_LENGTH 47
#define KAS_FSM_NAME_MAX_LENGTH 47
#define KAS_STATE_NAME_MAX_LENGTH 47

//  user variables for SHIPS
#define KASSELECTION_ALLOC_INCREMENT 16
#define KASSELECTION_MAX_LABEL_LENGTH 47
typedef struct {
    char label[KASSELECTION_MAX_LABEL_LENGTH+1];
    GrowSelection shipList;
} KasSelection;

//
//  labelled entities in mission layout file
//
#define KAS_MAX_LABEL_LENGTH 47
#define KAS_LABELLED_ENTITY_ALLOC_INCREMENT 16
//  team labels are stored in AITeam struct
typedef struct {
    char label[KAS_MAX_LABEL_LENGTH+1];
    Path *path;
} LabelledPath;
typedef struct {
    char label[KAS_MAX_LABEL_LENGTH+1];
    hvector *hvector;
} LabelledVector;
typedef struct {
    char label[KAS_MAX_LABEL_LENGTH+1];
    Volume *volume;
} LabelledVolume;


/*=============================================================================
    Functions
=============================================================================*/
//  to fill in table of labelled mission layout entities
void kasLabelsInit(void);
//void kasLabelledAITeamAdd(char *label);
Path *kasLabelledPathAdd(sdword label, sdword numPoints, sdword closed);
hvector *kasLabelledVectorAdd(sdword label, real32 x, real32 y, real32 z,real32 w);
Volume *kasLabelledVolumeAdd(sdword label);

void kasMissionStart(char *name, KASInitFunction initFunction, KASWatchFunction watchFunction);
void kasExecute(void);

// keywords of KAS language
void kasJump(char *stateName, KASInitFunction initFunction, KASWatchFunction watchFunction);
void kasFSMCreate(sdword fsmNameHash, KASInitFunction initFunction, KASWatchFunction watchFunction, struct AITeam *team);

// labels: resolves references from script to mission layout file
// (Cataclysm keys labelled entities by an int hash, not the label string)
struct AITeam *kasAITeamPtr(char *label);
void kasAITeamShipsPtr(GrowSelection *entry, sdword value);
struct AITeam *kasAITeamPtrIfExists(sdword labelHash);
GrowSelection *kasAITeamShipsPtrIfExists(sdword labelHash);
sdword *kasFlagsPtrIfExists(sdword flagNameHash);
hvector *kasShipsVectorPtr(char *label);
hvector *kasShipsVectorPtrIfExists(sdword labelHash);
hvector *kasTeamsVectorPtr(sdword labelHash);
hvector *kasVolumeVectorPtr(sdword label);
hvector *kasThisTeamsVectorPtr(void);
Path   *kasPathPtr(sdword label);
Path *kasPathPtrNoErrorChecking(sdword label);
Volume *kasVolumePtr(sdword label);
hvector *kasVectorPtr(sdword label);
hvector *kasVectorPtrIfExists(sdword label);
hvector *kasLabelledVectorSet(sdword label, hvector *vector);
GrowSelection *kasGrowSelectionPtr(sdword label);

GrowSelection *kasGetGrowSelectionPtrIfExists(char *label);

void kasShipDied(Ship *ship);
sdword kasRemoveTargetFromSelections(SpaceObjRotImpTarg *target);

void kasGrowSelectionClear(GrowSelection *ships);

void kasLabelledEntitiesDestroy(void);

void kasAddShipToTeam(Ship *ship, sdword labelHash);
sdword kasHashLabel(char *label);

#define KAS_TEAM_NAME_MAX_LENGTH 47
char *kasAITeamName(struct AITeam *team, char *teamName);

void kasTakeADump(void);
void kasDebugDraw(void);
#define KAS_DEBUG_TEXTCOLOR            colRGB(64, 255, 196)

void kasTeamDied(struct AITeam *team);

void kasClose(void);
void kasInit(void);

void Watch_NoOpTeamContext(void);

// Save Game Stuff
void kasSave(void);
void kasLoad(void);
void *kasConvertOffsetToFuncPtr(sdword offset);
sdword kasConvertFuncPtrToOffset(void *func);

extern sdword kasCurrentStep;
extern sdword kasCurrentScopeHash;
extern sdword kasCurrentTeamPtr;
extern sdword g_GameDifficulty;


#endif
