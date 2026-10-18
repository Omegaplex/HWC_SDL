/*=============================================================================
    Name    : Researchapi.h
    Purpose : definitions for the research manager

    Created 5/25/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___RESEARCHAPI_H
#define ___RESEARCHAPI_H

#include "LinkedList.h"
#include "ObjTypes.h"
#include "ShipDefs.h"
#include "Types.h"

/*=============================================================================
    Definitions:
=============================================================================*/

// convert technology from enum to bit position in masks
#define TechToBit(tech) (udword)((udword)1<<(tech))
#define AllTechnology   (udword)(0xFFFFFFFF)

// lab status defines
#define LS_NORESEARCHSHIP   0
#define LS_NORESEARCHITEM   1
#define LS_RESEARCHITEM     2
#define LS_RESEARCHITEMSOON 3

// number of labs
#define NUM_RESEARCHLABS         6
#define NUM_CATA_RESEARCHLABS    5

// Retail rmSetPrereqMaskState sentinel values
#define TECH_NOTECHNOLOGY       -1
#define TECH_ALLTECHNOLOGY      2

#define RM_Disabled             BIT31
/*=============================================================================
    Type definitions:
=============================================================================*/


// list of all technologies that can be researched
typedef enum
{
    TechCloaking = 0,
    TechTargetingSystems,
    TechPlasmaWeapons,
    TechMediumGuns,
    TechDefenseField,
    TechHeavyGuns,
    TechRepair,
    TechSalvage,
    TechDriveFighter,
    TechADriveFighter,
    TechDriveFrigate,
    TechADriveFrigate,
    TechDriveDestroyer,
    TechADriveDestroyer,
    TechArmour2,
    TechArmour3,
    TechSupportSystems,
    TechMicroShip,
    TechEnergy,
    TechIon,
    TechMissile,
    TechRepulse,
    TechEMP,
    TechExplosiveCharge,
    TechLink,
    TechMimic,
    TechAfterburner,
    TechBigGun,
    TechAdvancedSensors,
    TechForceField1,
    TechForceField2,
    TechForceField3,
    TechInfect,
    TechRepairBots,
    TechCrystalHarvest,
    TechResearchCleesanAttack,
    TechBeastVaccine,
    TechPhasedTelemetry,
    TechInertialessDrive,
    TechBiowarhead,
    TechDriveCorvette,
    NumTechnologies
} TechnologyType;

/* Ship upgrade bits unlocked by research techs. TechLink and TechMicroShip each map to two distinct runtime bits. */
#define RESEARCH_UPGRADE_MASK_ADRIVE_FIGHTER      BIT0
#define RESEARCH_UPGRADE_MASK_ADRIVE_FRIGATE      BIT1
#define RESEARCH_UPGRADE_MASK_ADRIVE_DESTROYER    BIT2
#define RESEARCH_UPGRADE_MASK_LINK_PRIMARY        BIT3
#define RESEARCH_UPGRADE_MASK_LINK_SECONDARY      BIT4
#define RESEARCH_UPGRADE_MASK_MIMIC               BIT5
#define RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY   BIT6
#define RESEARCH_UPGRADE_MASK_EXPLOSIVE_CHARGE    BIT7
#define RESEARCH_UPGRADE_MASK_AFTERBURNER         BIT8
#define RESEARCH_UPGRADE_MASK_EMP                 BIT9
#define RESEARCH_UPGRADE_MASK_REPULSE             BIT10
#define RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY BIT11
#define RESEARCH_UPGRADE_MASK_FORCE_FIELD_1       BIT12
#define RESEARCH_UPGRADE_MASK_FORCE_FIELD_2       BIT13
#define RESEARCH_UPGRADE_MASK_FORCE_FIELD_3       BIT14
#define RESEARCH_UPGRADE_MASK_DEFENSE_FIELD       BIT15
#define RESEARCH_UPGRADE_MASK_INFECT              BIT16
#define RESEARCH_UPGRADE_MASK_CLOAKING            BIT17
#define RESEARCH_UPGRADE_MASK_REPAIR              BIT18
#define RESEARCH_UPGRADE_MASK_SALVAGE             BIT19
#define RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST     BIT20
#define RESEARCH_UPGRADE_MASK_REPAIR_BOTS         BIT21
#define RESEARCH_UPGRADE_MASK_BEAST_VACCINE       BIT22
#define RESEARCH_UPGRADE_MASK_ADVANCED_SENSORS    BIT23
#define RESEARCH_UPGRADE_MASK_ION                 BIT25
#define RESEARCH_UPGRADE_MASK_MISSILE             BIT26

#define NewAlloys                   TechCloaking
#define MassDrive1Kt                TechTargetingSystems
#define CoolingSystems              TechPlasmaWeapons
#define CloakDefenseFighter         TechMediumGuns
#define TargetingSystems            TechDefenseField
#define PlasmaWeapons               TechHeavyGuns
#define Chassis1                    TechRepair
#define MassDrive10Kt               TechSalvage
#define MediumGuns                  TechDriveFighter
#define MineLayerTech               TechADriveFighter
#define Chassis2                    TechDriveFrigate
#define AdvancedCoolingSystems      TechADriveFrigate
#define MassDrive100Kt              TechDriveDestroyer
#define FireControl                 TechADriveDestroyer
#define SupportRefuelTech           TechArmour2
#define AdvanceTacticalSupport      TechArmour3
#define IonWeapons                  TechSupportSystems
#define DDDFDFGFTech                TechMicroShip
#define Chassis3                    TechEnergy
#define MassDrive1Mt                TechIon
#define AdvancedFireControl         TechMissile
#define MissileWeapons              TechRepulse
#define ConstructionTech            TechEMP
#define HeavyGuns                   TechExplosiveCharge
#define ProximityDetector           TechLink
#define SensorsArrayTech            TechMimic
#define GravityWellGeneratorTech    TechAfterburner
#define CloakGeneratorTech          TechBigGun
#define RepairTech                  TechAdvancedSensors
#define SalvageTech                 TechForceField1
#define ForceField2Tech             TechForceField2
#define ForceField3Tech             TechForceField3
#define InfectTech                  TechInfect
#define RepairBotsTech              TechRepairBots
#define CrystalHarvestTech          TechCrystalHarvest
#define ResearchCleesanAttackTech   TechResearchCleesanAttack
#define BeastVaccineTech            TechBeastVaccine
#define PhasedTelemetryTech         TechPhasedTelemetry
#define InertialessDriveTech        TechInertialessDrive
#define BiowarheadTech              TechBiowarhead
#define DriveCorvetteTech           TechDriveCorvette


// defines for the technology dependancies and ship technology requirements

typedef struct
{
    udword TechNeededToBuildShip[TOTAL_NUM_SHIPS];   // technologies needed before this ship can be built
    udword TechNeededToResearch[NumTechnologies];  // technology needed before this can be researched
    sdword TimeToComplete[NumTechnologies];        // time in seconds technology takes to finish
} TechStatics;


// defines for the research labs, and player status.

typedef struct
{
    struct Node     link;               // link for linkedlist of research topics
    TechnologyType  techresearch;       // what tech is being researched
    sdword          numlabsresearching; // how many labs are working on this
    real32          timeleft;           // how much time left in seconds
} ResearchTopic;

typedef struct
{
    sdword          labstatus;          // what is the status of this lab
    ResearchTopic  *topic;              // pointer to topic this lab is researching
} ResearchLab;

typedef struct
{
    LinkedList   listoftopics;
    ResearchLab  researchlabs[NUM_CATA_RESEARCHLABS];
} PlayerCataResearchRuntime;

typedef struct
{
    udword       HasTechnology;         // what tech this player has
    bool32         CanDoResearch;         // are they allowed to do research
    TechStatics *techstat;              // statics for that technology
    LinkedList   listoftopics;          // list of topics being researched for this player
    ResearchLab  researchlabs[NUM_RESEARCHLABS];  // list of labs for this player
} PlayerResearchInfo;


/*=============================================================================
    Data:
=============================================================================*/

//extern TechnologyType         SetTechLevel;
extern udword         SetTechLevel;

/*=============================================================================
    Function Prototypes:
=============================================================================*/

// Forward declaration to get around circular dependency:
// Universe.h: Player <-> ResearchAPI.h: PlayerResearchInfo
struct Player;

// callback to set dependancies
void rmSetShipDependCB(char *directory, char *field, void *dataToFillIn);
void rmSetTechDependCB(char *directory, char *field, void *dataToFillIn);

char *RaceSpecificTechTypeToNiceString(TechnologyType tech, ShipRace race);
char *TechTypeToNiceString(TechnologyType tech);
char *TechTypeToString    (TechnologyType tech);
TechnologyType StrToTechType(char *tech);

// api function calls
ResearchTopic *Researching(struct Player *player, TechnologyType tech);

LinkedList *rmPlayerResearchTopicList(struct Player *player);
ResearchLab *rmPlayerResearchLab(struct Player *player, sdword labnumber);
sdword rmPlayerResearchLabCount(struct Player *player);
sdword *rmPlayerCurrentPrereqMask(struct Player *player);
TechStatics *rmPlayerTechStatics(struct Player *player);
sdword *rmPlayerResearchTimeTable(struct Player *player);

sdword rmResearchingAnything(struct Player *player);

void rmGiveTechToPlayerByType(struct Player *player, TechnologyType techtype);
bool32 rmTechIsResearched(struct Player *player, int techIndex);

void   rmAddTechToPlayer(struct Player *player, TechnologyType techtype);
sdword rmFindFreeLab(struct Player *player, sdword techType);
bool32   rmCanBuildShip(struct Player *player, ShipType type);
//void   rmResearchTechForShip(struct Player *player, ShipType type);
void   rmClearResearchlab(struct Player *player, sdword labnumber);
void   rmAssignPlayersLabToResearch(struct Player *player, sdword labnumber, TechnologyType tech);
void   rmUpdateResearch(void);
void   rmInitializeResearchStruct(struct Player *player, bool32 candoresearch, sdword techlevel);

sdword rmTechRequiredForShip(struct Player *player, ShipType type);
bool32   rmResearchTechForShip(struct Player *player, TechnologyType techType);
sdword *rmAccumulatePrerequisitesRecursive(sdword *result, struct Player *player, sdword techType);
sdword rmCountBuildPrerequisites(struct Player *player, sdword shipType);

void rmInitializeResearchStatics(struct Player *player);

sdword ModuleCompleted(struct Player *player, ShipType shiptype, bool32 announce);
sdword ModuleDestroyed(struct Player *player, ShipType shiptype);
sdword BeastMothershipDestroyed(struct Player *player);
void rmEnableShip(ShipRace race, ShipType ship, bool32 bEnabled);
void rmResetStaticInfo(void);

//start the Research Manager.
void   rmAPIStartup(void);
void   rmAPIShutdown(void);

void rmActivateTech(udword *researchMask, int techIndex);
void rmSetPrereqMaskState(sdword *prereqMask, sdword state);
BOOL rmUpdateBuildCapabilities(struct Player *player);
int rmClearPrereqMask(udword *techMask);
void rmOrPrereqMasks(sdword *dest, sdword *lhs, sdword *rhs);
sdword *rmPreRequisitesAnd(sdword *result, sdword *prereq1, sdword *prereq2);
bool32 rmTechDependenciesEqual(sdword *techA, sdword *techB);
struct Player *rmRecomputeShipPrereqMask(struct Ship *ship);

#endif
