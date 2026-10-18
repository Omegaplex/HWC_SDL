
#ifndef ___FORMATION_H
#define ___FORMATION_H

#include "FormationDefs.h"
#include "ShipSelect.h"
#include "SpaceObj.h"

/*=============================================================================
    Defines:
=============================================================================*/

#define DEBUG_FORMATIONS 0

#define MIN_SHIPS_IN_FORMATION 2
#define ABSOLUTE_MIN_SHIPS_IN_FORMATION 1

#define FORMATION_SORT_BIGGEST_THEN_CLOSEST 1

/*=============================================================================
    Types:
=============================================================================*/

// specific types for sphere formation

typedef struct
{
    real32 cosdeclination;
    real32 sindeclination;
    sdword numAnglePoints;
    real32 cosrotang;
    real32 sinrotang;
} SphereDeclination;

typedef struct
{
    sdword numShipsCanHandle;
    sdword numDeclinations;
    SphereDeclination sphereDeclinations[1];
} SphereTableEntry;

typedef struct SphereStaticInfo
{
    sdword numTableEntries;
    SphereTableEntry *sphereTableEntryPtrs[1];
} SphereStaticInfo;

// general formation types

// changed to defines for speech tool (in FormationDefs.h)
typedef udword TypeOfFormation;

typedef struct
{
    TypeOfFormation formationtype;
    bool32 formationLocked;            //flag indicating status of formation
    matrix coordsys;
    //fix later to make these bit masks
    sdword tacticalState;            //loose or tight
    real32 tacticsUpdate;            //time formation was updated..so it is only updated once per univupdate
    bool32 flipselectionfixed;        //flag to indicate a certain state of the formation
    bool32 needFix;                   //another state flag for formation
    bool32 enders;                    //flag set if formation has been 'modified' to include target at center...should fix later and put in sphere specifics
    real32 travelvel;               // formation's travel velocity
    real32 percentmaxspeed;         // how much error is in formation positioning
    bool16 flagTravelSlowestShip;
    bool16 doneInitialAttack;
    udword sortorder;
    //for the tactical overlay
    bool32 pulse;
    sdword flipState;               //variable used to determine that flipped state of the formation <if it has been flipped or not during a combat turnaround>
    void *formationSpecificInfo;    // if you use this, first sdword of custom structure should be size of structure
//    bool32 formAroundProtectedShip;
} FormationCommand;

/*=============================================================================
    Stuff for Military parade
=============================================================================*/

typedef struct
{
    ShipPtr ship;
} Subslot;

typedef struct
{
    sdword highestSlotUsed;
    sdword numSubslots;
    Subslot subslots[1];
} MilitarySlot;

#define sizeofMilitarySlot(n) (sizeof(MilitarySlot) + sizeof(Subslot)*((n)-1))

// Somtaaw (s-prefix) ship parade slots
#define SLOT_SRECON              0
#define SLOT_SMIMIC              1
#define SLOT_SMCV                2
#define SLOT_SWORKER             3
#define SLOT_SPROCESSOR          4
#define SLOT_SCARRIER0           5
#define SLOT_SCARRIER1           6
#define SLOT_SCARRIER2           7
#define SLOT_SSENTINEL           8
#define SLOT_SACOLYTE            9
#define SLOT_SACV               10
#define SLOT_SRAMMINGFRIGATE    11
#define SLOT_SMULTIBEAMFRIGATE  12
#define SLOT_SHIVEFRIGATE       13
#define SLOT_SDREADNOUGHT       14
#define SLOT_SLEECH             15
#define SLOT_SDESTROYER         16
#define SLOT_SMOTHERSHIP        17

// Beast (b-prefix) ship parade slots
#define SLOT_BRECON             18
#define SLOT_BWORKER            19
#define SLOT_BPROCESSOR         20
#define SLOT_BHIVEFRIGATE       21
#define SLOT_BMULTIBEAMFRIGATE  22
#define SLOT_BCARRIER0          23
#define SLOT_BCARRIER1          24
#define SLOT_BCARRIER2          25
#define SLOT_BSENTINEL          26
#define SLOT_BACOLYTE           27
#define SLOT_BACV               28
#define SLOT_BINTERCEPTOR       29
#define SLOT_BATTACKBOMBER      30
#define SLOT_BCLOAKEDFIGHTER    31
#define SLOT_BMULTIGUNCORVETTE  32
#define SLOT_BDFGFRIGATE        33
#define SLOT_BHEAVYCORVETTE     34
#define SLOT_BIONARRAYFRIGATE   35
#define SLOT_BHEAVYCRUISER      36
#define SLOT_BMISSILECORVETTE   37
#define SLOT_BCRUISEMISSILE     38
#define SLOT_BRAMMINGFRIGATE    39
#define SLOT_BMOTHERSHIP        40

// HW1 k/t-race ship parade slots (used in R1/R2 carrier parade scripts)
// Indices 41-57 cover the k/t-race ships from formGetShipSlot
#define SLOT_Fighter3           41   // virtual; no Cataclysm k-ship maps here
#define SLOT_Fighter2           42   // kHeavyDefender, tHeavyDefender
#define SLOT_Fighter4           43   // virtual
#define SLOT_Fighter0           44   // kHeavyInterceptor, tHeavyInterceptor
#define SLOT_Fighter1           45   // kAttackBomber, tAttackBomber
#define SLOT_Fighter5           46   // virtual
#define SLOT_Frigate            47   // virtual; no k/t frigate in Cataclysm switch
#define SLOT_Corvette0          48   // kHeavyCorvette
#define SLOT_Corvette1          49   // kMultiGunCorvette, tMultiGunCorvette
#define SLOT_Corvette3          50   // tMinelayerCorvette
#define SLOT_SupportCorvette    51   // virtual; generic support corvette position
#define SLOT_SensorArray        52   // virtual; no k/t sensor array in Cataclysm
#define SLOT_Destroyer          53   // kStandardDestroyer, tStandardDestroyer
#define SLOT_HeavyCruiser       54   // kHeavyCruiser, tHeavyCruiser
#define SLOT_NonCombat          55   // kCloakGenerator, kGravWellGenerator, t-equivalents
#define SLOT_Carrier            56   // kCarrier
#define SLOT_Corvette2          57   // kSalCapCorvette, tSalCapCorvette

// P1 ship parade slots
#define SLOT_P1Fighter          58
#define SLOT_P1IonArrayFrigate  59
#define SLOT_P1MissileCorvette  60
#define SLOT_P1StandardCorvette 61

// Fallback slot for unrecognized ship types
#define SLOT_Misc               62
#define SLOT_ResCollector       SLOT_Misc
#define SLOT_ResourceController SLOT_Misc
#define SLOT_ResearchShip       SLOT_Misc

#define MAX_MILITARY_SLOTS      63

typedef struct
{
    Ship *aroundShip;
    sdword paradeType;
    MilitarySlot *militarySlots[MAX_MILITARY_SLOTS];
} MilitaryParadeCommand;

struct CommandLayer;

struct CommandToDo *CreateMilitaryGroupAroundShip(struct CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip);
void processMilitaryParadeToDo(struct CommandToDo *command,bool32 passiveAttacked);
void AddShipToMilitaryGroup(ShipPtr ship,struct CommandToDo *militaryGroup);
void RemoveShipFromMilitaryParade(Ship *shiptoremove,MilitaryParadeCommand *militaryParade);
void FreeMilitaryParadeContents(MilitaryParadeCommand *militaryParade);
void setMilitaryParade(struct CommandToDo *command);
bool32 shipInMilitaryParade(ShipPtr ship);
void paradeSetTweakables();

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofFormationSphereInfo(n) (sizeof(FormationSphereInfo) + sizeof(SphereVar)*((n)-1))
#define sizeofSphereStaticInfo(n) (sizeof(SphereStaticInfo) + sizeof(SphereTableEntry *)*((n)-1))
#define sizeofSphereTableEntry(n) (sizeof(SphereTableEntry) + sizeof(SphereDeclination)*((n)-1))

/*=============================================================================
    Functions:
=============================================================================*/

// routines for converting between string and enumeration for TypeOfFormation
char *TypeOfFormationToStr(TypeOfFormation formation);
TypeOfFormation StrToTypeOfFormation(char *str);

char *TypeOfFormationToNiceStr(TypeOfFormation formation);
TypeOfFormation NiceStrToTypeOfFormation(char *str);

void formationContentHasChanged(struct CommandToDo *command);
void formationTypeHasChanged(struct CommandToDo *command);
void formationArrangeOptimum(struct CommandToDo *formationtodo);
void processFormationToDo(struct CommandToDo *formationtodo,bool32 steadyFormation,bool32 passiveAttacked);
void setFormationToDo(struct CommandToDo *formationtodo);

void freeSphereStaticInfo(struct SphereStaticInfo *sphereStaticInfo);
struct SphereStaticInfo *createSphereStaticInfo(void);

void formationWingmanTrackLeader(struct Ship *ship,struct Ship *leader,bool32 rotate);

void FillInShipFormationStuff(Ship *ship,struct CommandToDo *formationcommand);

//formation locking variables
void lockFormation(struct CommandToDo *formationcommand,udword specialEffect);
void unlockFormation(struct CommandToDo *formationcommand);

//function that recalculates a formations minimum travel velocity...
void setFormationTravelVelocity(struct CommandToDo *formationCommand);

void FormationCalculateOffsets(struct CommandToDo *formationtodo);

// returns the selection's velocity at which it can all travel, scaled by scalevel
real32 GetShipsTravelVel(SelectCommand *selection,real32 scalevel);

// returns -1 if not found, ship index if found
sdword formationRemoveShipFromSelection(struct CommandToDo *formationtodo,Ship *removeship);

/*=============================================================================
    Data:
=============================================================================*/

extern udword formationSortType[FORMATION_TYPE_COUNT];
extern real32 STRIKECRAFT_PADDING_MODIFIER;

extern sdword FORMATION_ERROR_CALCULATE_RATE;
#endif
