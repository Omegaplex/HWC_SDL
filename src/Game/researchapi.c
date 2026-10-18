/*=============================================================================
    Name    : researchapi.c
    Purpose : logic for the research manager api functions.

    Created 5/27/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>

#include "types.h"
#include "shipdefs.h"
#include "objtypes.h"
#include "researchapi.h"
#include "universe.h"
#include "ModuleDefs.h"
#include "statscript.h"
#include "researchGUI.h"
#include "BattleChat.h"
#include "CommandWrap.h"
#include "strings.h"
#include "linkedlist.h"
#include "soundevent.h"
#include "BeastMothership.h"

#include "fereg.h"
#include "region.h"
#include "trademgr.h"

#include "multiplayergame.h"
#include "consmgr.h"
#include "singleplayer.h"
#include "Physics.h"

sdword univPingShipDied(sdword ship);
void univUpdateShipProximity(Ship *ship);
sdword playerGetSensorCoverage(Player *player);
bool32 rmPrereqTechIsResearched(const sdword *prereqMask, sdword tech);
sdword *rmBuildUnresearchedTechMask(sdword *dest, sdword *source);

#define makeTechEntry(prefix, statics, token, tech) \
    { prefix token, scriptSetSdwordCB, &(statics.TimeToComplete[tech]) }

#define CATACLYSM_TECH_ENTRY_LIST(prefix, statics) \
    makeTechEntry(prefix, statics, "TECH_CLOAKING", TechCloaking), \
    makeTechEntry(prefix, statics, "TECH_TARGETING_SYSTEMS", TechTargetingSystems), \
    makeTechEntry(prefix, statics, "TECH_PLASMA_WEAPONS", TechPlasmaWeapons), \
    makeTechEntry(prefix, statics, "TECH_MEDIUM_GUNS", TechMediumGuns), \
    makeTechEntry(prefix, statics, "TECH_DEFENSE_FIELD", TechDefenseField), \
    makeTechEntry(prefix, statics, "TECH_HEAVY_GUNS", TechHeavyGuns), \
    makeTechEntry(prefix, statics, "TECH_REPAIR", TechRepair), \
    makeTechEntry(prefix, statics, "TECH_SALVAGE", TechSalvage), \
    makeTechEntry(prefix, statics, "TECH_DRIVE_FIGHTER", TechDriveFighter), \
    makeTechEntry(prefix, statics, "TECH_ADRIVE_FIGHTER", TechADriveFighter), \
    makeTechEntry(prefix, statics, "TECH_DRIVE_FRIGATE", TechDriveFrigate), \
    makeTechEntry(prefix, statics, "TECH_ADRIVE_FRIGATE", TechADriveFrigate), \
    makeTechEntry(prefix, statics, "TECH_DRIVE_DESTROYER", TechDriveDestroyer), \
    makeTechEntry(prefix, statics, "TECH_ADRIVE_DESTROYER", TechADriveDestroyer), \
    makeTechEntry(prefix, statics, "TECH_ARMOUR2", TechArmour2), \
    makeTechEntry(prefix, statics, "TECH_ARMOUR3", TechArmour3), \
    makeTechEntry(prefix, statics, "TECH_SUPPORT_SYSTEMS", TechSupportSystems), \
    makeTechEntry(prefix, statics, "TECH_MICRO_SHIP", TechMicroShip), \
    makeTechEntry(prefix, statics, "TECH_ENERGY", TechEnergy), \
    makeTechEntry(prefix, statics, "TECH_ION", TechIon), \
    makeTechEntry(prefix, statics, "TECH_MISSILE", TechMissile), \
    makeTechEntry(prefix, statics, "TECH_REPULSE", TechRepulse), \
    makeTechEntry(prefix, statics, "TECH_EMP", TechEMP), \
    makeTechEntry(prefix, statics, "TECH_EXPLOSIVE_CHARGE", TechExplosiveCharge), \
    makeTechEntry(prefix, statics, "TECH_LINK", TechLink), \
    makeTechEntry(prefix, statics, "TECH_MIMIC", TechMimic), \
    makeTechEntry(prefix, statics, "TECH_AFTERBURNER", TechAfterburner), \
    makeTechEntry(prefix, statics, "TECH_BIG_GUN", TechBigGun), \
    makeTechEntry(prefix, statics, "TECH_ADVANCED_SENSORS", TechAdvancedSensors), \
    makeTechEntry(prefix, statics, "TECH_FORCE_FIELD1", TechForceField1), \
    makeTechEntry(prefix, statics, "TECH_FORCE_FIELD2", TechForceField2), \
    makeTechEntry(prefix, statics, "TECH_FORCE_FIELD3", TechForceField3), \
    makeTechEntry(prefix, statics, "TECH_INFECT", TechInfect), \
    makeTechEntry(prefix, statics, "TECH_REPAIRBOTS", TechRepairBots), \
    makeTechEntry(prefix, statics, "TECH_CRYSTAL_HARVEST", TechCrystalHarvest), \
    makeTechEntry(prefix, statics, "TECH_RESEARCH_CLEESAN_ATTACK", TechResearchCleesanAttack), \
    makeTechEntry(prefix, statics, "TECH_BEAST_VACCINE", TechBeastVaccine), \
    makeTechEntry(prefix, statics, "TECH_PHASED_TELEMETRY", TechPhasedTelemetry), \
    makeTechEntry(prefix, statics, "TECH_INERTIALESS_DRIVE", TechInertialessDrive), \
    makeTechEntry(prefix, statics, "TECH_BIOWARHEAD", TechBiowarhead), \
    makeTechEntry(prefix, statics, "TECH_DRIVE_CORVETTE", TechDriveCorvette)

/*=============================================================================
    Data:
=============================================================================*/

TechStatics            SinglePlayerTechStatic;

TechStatics            MultiPlayerSectTechStatic;
TechStatics            MultiPlayerBeastTechStatic;

TechStatics            MultiPlayerCarrierSectTechStatic;
TechStatics            MultiPlayerCarrierBeastTechStatic;

#define RM_PREREQ_TABLE_ENTRY_COUNT (TOTAL_NUM_SHIPS + NumTechnologies)
#define RM_PREREQ_TECH_ENTRY_BASE TOTAL_NUM_SHIPS
#define RM_PREREQ_MASK_DISABLED_STATE (-2)
#define RM_PREREQ_MASK_CLEAR_STATE (-1)
#define RM_MODULE_TECH_NONE PLAYER_SHIP_PREREQ_TECH_COUNT

static_assert(NumTechnologies == PLAYER_SHIP_PREREQ_TECH_COUNT, "Prereq mask tech count mismatch");

static PlayerShipPrereqMask SinglePlayerPrereqTable[RM_PREREQ_TABLE_ENTRY_COUNT];
static PlayerShipPrereqMask MultiPlayerSectPrereqTable[RM_PREREQ_TABLE_ENTRY_COUNT];
static PlayerShipPrereqMask MultiPlayerBeastPrereqTable[RM_PREREQ_TABLE_ENTRY_COUNT];

real32                 labdec[NUM_RESEARCHLABS+1]=
{
    (real32)0.0,    // time decrement for combined lab research
    (real32)1.0,    // 1 lab = 1.0
    (real32)1.6,
    (real32)2.25,
    (real32)2.8,
    (real32)3.25,
    (real32)3.6
};

static const real32 rmTutorialResearchDecrement = 8.0f;   /* cataclysm.exe 0x72A558 */

static_assert(sizeof(PlayerCataResearchRuntime) == 0x34, "Cataclysm research runtime size mismatch");

scriptEntry TechnologyDependancies[] =
{
    { "SPRSShipDepend", rmSetShipDependCB, SinglePlayerPrereqTable },
    { "SPRSTechDepend", rmSetTechDependCB, &SinglePlayerPrereqTable[RM_PREREQ_TECH_ENTRY_BASE] },
    CATACLYSM_TECH_ENTRY_LIST("SPRS", SinglePlayerTechStatic),

    { "MPRSShipDepend", rmSetShipDependCB, MultiPlayerSectPrereqTable },
    { "MPRSTechDepend", rmSetTechDependCB, &MultiPlayerSectPrereqTable[RM_PREREQ_TECH_ENTRY_BASE] },
    CATACLYSM_TECH_ENTRY_LIST("MPRS", MultiPlayerSectTechStatic),

    { "MPRBShipDepend", rmSetShipDependCB, MultiPlayerBeastPrereqTable },
    { "MPRBTechDepend", rmSetTechDependCB, &MultiPlayerBeastPrereqTable[RM_PREREQ_TECH_ENTRY_BASE] },
    CATACLYSM_TECH_ENTRY_LIST("MPRB", MultiPlayerBeastTechStatic),

    endEntry
};

static NumStrXlate techtypeinfo[] =
{
    { (udword)TechCloaking,             "TECH_CLOAKING"},
    { (udword)TechTargetingSystems,     "TECH_TARGETING_SYSTEMS"},
    { (udword)TechPlasmaWeapons,        "TECH_PLASMA_WEAPONS"},
    { (udword)TechMediumGuns,           "TECH_MEDIUM_GUNS"},
    { (udword)TechDefenseField,         "TECH_DEFENSE_FIELD"},
    { (udword)TechHeavyGuns,            "TECH_HEAVY_GUNS"},
    { (udword)TechRepair,               "TECH_REPAIR"},
    { (udword)TechSalvage,              "TECH_SALVAGE"},
    { (udword)TechDriveFighter,         "TECH_DRIVE_FIGHTER"},
    { (udword)TechADriveFighter,        "TECH_ADRIVE_FIGHTER"},
    { (udword)TechDriveFrigate,         "TECH_DRIVE_FRIGATE"},
    { (udword)TechADriveFrigate,        "TECH_ADRIVE_FRIGATE"},
    { (udword)TechDriveDestroyer,       "TECH_DRIVE_DESTROYER"},
    { (udword)TechADriveDestroyer,      "TECH_ADRIVE_DESTROYER"},
    { (udword)TechArmour2,              "TECH_ARMOUR2"},
    { (udword)TechArmour3,              "TECH_ARMOUR3"},
    { (udword)TechSupportSystems,       "TECH_SUPPORT_SYSTEMS"},
    { (udword)TechMicroShip,            "TECH_MICRO_SHIP"},
    { (udword)TechEnergy,               "TECH_ENERGY"},
    { (udword)TechIon,                  "TECH_ION"},
    { (udword)TechMissile,              "TECH_MISSILE"},
    { (udword)TechRepulse,              "TECH_REPULSE"},
    { (udword)TechEMP,                  "TECH_EMP"},
    { (udword)TechExplosiveCharge,      "TECH_EXPLOSIVE_CHARGE"},
    { (udword)TechLink,                 "TECH_LINK"},
    { (udword)TechMimic,                "TECH_MIMIC"},
    { (udword)TechAfterburner,          "TECH_AFTERBURNER"},
    { (udword)TechBigGun,               "TECH_BIG_GUN"},
    { (udword)TechAdvancedSensors,      "TECH_ADVANCED_SENSORS"},
    { (udword)TechForceField1,          "TECH_FORCE_FIELD1"},
    { (udword)TechForceField2,          "TECH_FORCE_FIELD2"},
    { (udword)TechForceField3,          "TECH_FORCE_FIELD3"},
    { (udword)TechInfect,               "TECH_INFECT"},
    { (udword)TechRepairBots,           "TECH_REPAIRBOTS"},
    { (udword)TechCrystalHarvest,       "TECH_CRYSTAL_HARVEST"},
    { (udword)TechResearchCleesanAttack,"TECH_RESEARCH_CLEESAN_ATTACK"},
    { (udword)TechBeastVaccine,         "TECH_BEAST_VACCINE"},
    { (udword)TechPhasedTelemetry,      "TECH_PHASED_TELEMETRY"},
    { (udword)TechInertialessDrive,     "TECH_INERTIALESS_DRIVE"},
    { (udword)TechBiowarhead,           "TECH_BIOWARHEAD"},
    { (udword)TechDriveCorvette,        "TECH_DRIVE_CORVETTE"},
    { (udword)NewAlloys,                str$(NewAlloys) },
    { (udword)MassDrive1Kt,             str$(MassDrive1Kt) },
    { (udword)CoolingSystems,           str$(CoolingSystems) },
    { (udword)CloakDefenseFighter,      str$(CloakDefenseFighter) },
    { (udword)TargetingSystems,         str$(TargetingSystems) },
    { (udword)PlasmaWeapons,            str$(PlasmaWeapons) },
    { (udword)Chassis1,                 str$(Chassis1) },
    { (udword)MassDrive10Kt,            str$(MassDrive10Kt) },
    { (udword)MediumGuns,               str$(MediumGuns) },
    { (udword)MineLayerTech,            str$(MineLayerTech) },
    { (udword)Chassis2,                 str$(Chassis2) },
    { (udword)AdvancedCoolingSystems,   str$(AdvancedCoolingSystems) },
    { (udword)MassDrive100Kt,           str$(MassDrive100Kt) },
    { (udword)FireControl,              str$(FireControl) },
    { (udword)SupportRefuelTech,        str$(SupportRefuelTech) },
    { (udword)AdvanceTacticalSupport,   str$(AdvanceTacticalSupport) },
    { (udword)IonWeapons,               str$(IonWeapons) },
    { (udword)DDDFDFGFTech,             str$(DDDFDFGFTech) },
    { (udword)Chassis3,                 str$(Chassis3) },
    { (udword)MassDrive1Mt,             str$(MassDrive1Mt) },
    { (udword)AdvancedFireControl,      str$(AdvancedFireControl) },
    { (udword)MissileWeapons,           str$(MissileWeapons) },
    { (udword)ConstructionTech,         str$(ConstructionTech) },
    { (udword)HeavyGuns,                str$(HeavyGuns) },
    { (udword)ProximityDetector,        str$(ProximityDetector) },
    { (udword)SensorsArrayTech,         str$(SensorsArrayTech) },
    { (udword)GravityWellGeneratorTech, str$(GravityWellGeneratorTech) },
    { (udword)CloakGeneratorTech,       str$(CloakGeneratorTech) },
    { (udword)RepairTech,               str$(RepairTech) },
    { (udword)SalvageTech,              str$(SalvageTech) },
    { (udword)NumTechnologies,          str$(NumTechnologies)},
    { 0, NULL }
};

static const sdword rmTechLabIndexByTechType[NumTechnologies] =
{
    -1, -1, -1, -1, -1, -1, 2, 2, 0, 0,
     0,  0,  0,  0,  1,  1, 2, 3, 4, 4,
     4,  4,  4,  4,  0,  2, 2, 4, 3, 1,
     1,  1, -1,  3,  3,  2, 3, 3, 3, -1,
    -1
};



/*=============================================================================
    Functions:
=============================================================================*/

static void rmInitializeReferenceShipPrereqs(Player *player);
TechPrintList *rmGetCurrentPlayerTechList(void);
static sdword rmMothershipModuleShipTypeToTechType(sdword shipType);
static sdword rmBeastShipTechGroupUnlocked(const Player *player, sdword shipType);
static void rmSetBeastShipTechGroupUnlocked(Player *player, sdword shipType, sdword value);
static void rmUnlockBeastTechListEntry(TechPrintList *techList, const Player *player, ShipType shipType, sdword techIndex, sword unlockType);
void rmSetPrereqMaskState(sdword *prereqMask, sdword state);

char *RaceSpecificTechTypeToNiceString(TechnologyType tech, ShipRace race)
{
    (void)race;
    return TechTypeToNiceString(tech);
}

char *TechTypeToNiceString(TechnologyType tech)
{
    return (strGetString((uword)tech+strTechOffset));
}

char *TechTypeToString(TechnologyType tech)
{
    return NumToStr(techtypeinfo,(uword)tech);
}

TechnologyType StrToTechType(char *tech)
{
    return StrToNum(techtypeinfo, tech);
}

static sdword rmModuleLabIndexFromShipType(ShipType shiptype)
{
    switch (shiptype)
    {
        case sMothershipDockingBay:
            return 0;
        case sMothershipArmour:
            return 1;
        case sMothershipMicro:
            return 2;
        case sMothershipSpecial:
            return 3;
        case sMothershipWeapons:
            return 4;
        default:
            return -1;
    }
}

LinkedList *rmPlayerResearchTopicList(Player *player)
{
    if (player->race == RACE_Sect || player->race == RACE_Beast)
    {
        return &player->cataResearchRuntime.listoftopics;
    }

    return &player->researchinfo.listoftopics;
}

ResearchLab *rmPlayerResearchLab(Player *player, sdword labnumber)
{
    if (player->race == RACE_Sect || player->race == RACE_Beast)
    {
        if (labnumber < 0 || labnumber >= NUM_CATA_RESEARCHLABS)
        {
            return NULL;
        }

        return &player->cataResearchRuntime.researchlabs[labnumber];
    }

    if (labnumber < 0 || labnumber >= NUM_RESEARCHLABS)
    {
        return NULL;
    }

    return &player->researchinfo.researchlabs[labnumber];
}

sdword rmPlayerResearchLabCount(Player *player)
{
    if (player->race == RACE_Sect || player->race == RACE_Beast)
    {
        return NUM_CATA_RESEARCHLABS;
    }
    return NUM_RESEARCHLABS;
}

sdword *rmPlayerCurrentPrereqMask(Player *player)
{
    return player->cataResearchOverlay.currentTechMask.value;
}

static bool32 *rmPlayerCanDoResearch(Player *player)
{
    return &player->cataResearchOverlay.canDoResearch;
}

static PlayerShipPrereqMask *rmPlayerDiscoveredPrereqTable(Player *player)
{
    if (singlePlayerGame)
    {
        if (player->race == RACE_Sect)
        {
            return SinglePlayerPrereqTable;
        }
        if (player->race == RACE_Beast)
        {
            return MultiPlayerBeastPrereqTable;
        }
    }
    else
    {
        if (player->race == RACE_Sect)
        {
            return MultiPlayerSectPrereqTable;
        }
        if (player->race == RACE_Beast)
        {
            return MultiPlayerBeastPrereqTable;
        }
    }

    return NULL;
}

TechStatics *rmPlayerTechStatics(Player *player)
{
    if (player == NULL)
    {
        return NULL;
    }

    if (singlePlayerGame)
    {
        return &SinglePlayerTechStatic;
    }
    if (player->race == RACE_Beast)
    {
        return &MultiPlayerBeastTechStatic;
    }
    return &MultiPlayerSectTechStatic;
}

sdword *rmPlayerResearchTimeTable(Player *player)
{
    TechStatics *techstat = rmPlayerTechStatics(player);

    return (techstat != NULL) ? techstat->TimeToComplete : NULL;
}


/*-----------------------------------------------------------------------------
    Name        : Reseaching
    Description : checks to see if the passed in tech is currently being researched.
    Inputs      : tech
    Outputs     : true false
    Return      : bool32
----------------------------------------------------------------------------*/
ResearchTopic *Researching(Player *player, TechnologyType tech)
{
    ResearchTopic      *topic;
    Node               *walk;

    walk = rmPlayerResearchTopicList(player)->head;

    // walk through list of topics being researched
    while (walk != NULL)
    {
        topic = (ResearchTopic *)listGetStructOfNode(walk);
        if (topic->techresearch == tech)
            return(topic);
        walk = walk->next;
    }

    return(NULL);
}

//
//  returns 1 if the player is currently researching anything
//  returns 0 otherwise
//
sdword rmResearchingAnything(struct Player *player)
{
    return rmPlayerResearchTopicList(player)->head != NULL;
}

void rmAddTechToPlayer(struct Player *player, TechnologyType techtype)
{
    rmActivateTech((udword *)rmPlayerCurrentPrereqMask(player), techtype);
    rmUpdateBuildCapabilities(player);
}


/*-----------------------------------------------------------------------------
    Name        : rmCanBuildShip
    Description : returns true if player has tech needed to build that ship.
    Inputs      : player
    Outputs     : true/false
    Return      : bool32
----------------------------------------------------------------------------*/
bool32 rmCanBuildShip(Player *player, ShipType type)
{
    sdword result[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword *currentPrereqMask;
    ShipStaticInfo *shipStatic;

    if (player->aiPlayer != NULL &&
        !singlePlayerGame &&
        player->race == RACE_Beast)
    {
        switch (type)
        {
            case bAcolyte:
                return player->cataPreFogUnknown2.beastCanBuildAcolyte;
            case bACV:
                return player->cataPreFogUnknown2.beastCanBuildACV;
            case bMultiBeamFrigate:
                return player->cataPreFogUnknown2.beastCanBuildMultiBeamFrigate;
            case bHiveFrigate:
                return player->cataPreFogUnknown2.beastCanBuildHiveFrigate;
            case bRammingFrigate:
                return player->cataPreFogUnknown2.beastCanBuildRammingFrigate;
            case bSentinel:
                return player->cataPreFogUnknown2.beastCanBuildSentinel;
        }
    }

    if (player->discoveredShipPrereqs[type].value[PLAYER_SHIP_PREREQ_STATE_INDEX] == RM_PREREQ_MASK_DISABLED_STATE)
    {
        return FALSE;
    }
    if (!singlePlayerGame && type == dAcolyte)
    {
        return FALSE;
    }

    shipStatic = GetShipStaticInfo(type, player->race);
    if (player->race == RACE_Sect
        && shipStatic->externalBuild
        && shipStatic->shipclass != CLASS_Component
        && !univPlayerHasShipType(sMothershipDockingBay, player))
    {
        return FALSE;
    }

    currentPrereqMask = rmPlayerCurrentPrereqMask(player);
    rmPreRequisitesAnd(result, player->discoveredShipPrereqs[type].value, currentPrereqMask);
    return rmTechDependenciesEqual(result, player->discoveredShipPrereqs[type].value);
}


/*-----------------------------------------------------------------------------
    Name        : rmFindFreeLab
    Description : finds a free lab
    Inputs      : player
    Outputs     : ResearchLab, or -1
    Return      : sdword
----------------------------------------------------------------------------*/
sdword rmFindFreeLab(Player *player, sdword techType)
{
    sdword       labindex;
    ResearchLab *lab;

    if (player->race == RACE_Beast)
    {
        lab = rmPlayerResearchLab(player, 0);
        if (lab != NULL && lab->labstatus == LS_NORESEARCHITEM)
        {
            if ((rmGUIActive) && (player->aiPlayer == NULL))
            {
                rmUpdateTechList();
            }
            return 0;
        }
        return -1;
    }

    for (labindex = 0; labindex < NUM_CATA_RESEARCHLABS; labindex++)
    {
        lab = rmPlayerResearchLab(player, labindex);
        if (lab->labstatus == LS_NORESEARCHITEM &&
            (techType == -1 || rmTechLabIndexByTechType[techType] == labindex))
        {
            if ((rmGUIActive) && (player->aiPlayer == NULL))
            {
                rmUpdateTechList();
            }
            return labindex;
        }
    }
    return -1;
}


/*-----------------------------------------------------------------------------
    Name        : rmClearResearchlab
    Description : Clears the specified labs research status.
    Inputs      : lab to clear, and player
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmClearResearchlab(Player *player, sdword labnumber)
{
    ResearchLab   *lab = rmPlayerResearchLab(player, labnumber);
    ResearchTopic *topic;

    if (lab == NULL)
    {
        return;
    }

    topic = lab->topic;

    if (lab->labstatus == LS_RESEARCHITEM)
    {
        lab->topic = NULL;
        listDeleteNode(&topic->link);
        lab->labstatus = LS_NORESEARCHITEM;
        if ((player == universe.curPlayerPtr) && (rmGUIActive))
        {
            rmUpdateTechList();
        }
    }
    else if (lab->labstatus == LS_RESEARCHITEMSOON)
    {
        lab->labstatus = LS_NORESEARCHITEM;
    }
}


/*-----------------------------------------------------------------------------
    Name        : rmAssignPlayersLabToResearch
    Description : assigns a lab to a certain research item
    Inputs      : Lab number, research topic number
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmAssignPlayersLabToResearch(Player *player, sdword labnumber, TechnologyType tech)
{
    ResearchTopic *topic;
    ResearchLab   *lab = rmPlayerResearchLab(player, labnumber);

    dbgAssert(lab != NULL);
    if (player->race == RACE_Beast && rmPlayerResearchTopicList(player)->num != 0)
    {
        return;
    }
    if ((topic=Researching(player, tech))!=NULL)
    {
        lab->labstatus = LS_RESEARCHITEM;
        lab->topic = topic;
    }
    else
    {
        topic = (ResearchTopic *)memAlloc(sizeof(ResearchTopic),"ResearchTopic",NonVolatile);
        listAddNode(rmPlayerResearchTopicList(player), &topic->link, topic);
        lab->labstatus = LS_RESEARCHITEM;
        lab->topic = topic;
        topic->numlabsresearching = 1;
        topic->techresearch       = tech;
        topic->timeleft           = (real32)rmPlayerResearchTimeTable(player)[tech];
    }

    if ((player == universe.curPlayerPtr)&&(rmGUIActive)) rmUpdateTechList();
}

/*-----------------------------------------------------------------------------
    Name        : rmDeactivateLab
    Description : deactivates a research lab, if ship dies lab dies too.
    Inputs      : player
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : rmActivateFreeLab
    Description : Activates a lab.
    Inputs      : player that requires a lab to be activated
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : rmUpdateResearch
    Description : updates all research
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmUpdateResearch(void)
{
    sdword              index, labindex, i, handle;
    sdword              firstshiptype, lastshiptype;
    ResearchLab        *lab;
    Player             *player;
    ResearchTopic      *topic;
    Node               *walk;
    LinkedList          deletelist;
    bool32              shipcanbuild[TOTAL_NUM_SHIPS];
    bool32              canBuildNow;
    real32              timeDecrement;

    listInit(&deletelist);

    // search through list of players
    for (index=0;index<universe.numPlayers;index++)
    {
        player = &universe.players[index];
        if (*rmPlayerCanDoResearch(player))
        {
            walk = rmPlayerResearchTopicList(player)->head;

            // walk through list of topics being researched
            while (walk != NULL)
            {
                topic = (ResearchTopic *)listGetStructOfNode(walk);
                if (tutorial == 1)
                {
                    timeDecrement = rmTutorialResearchDecrement;
                }
                else if (player->race == RACE_Beast && player->PlayerMothership != NULL)
                {
                    timeDecrement = mothershipGetResearchBonus(player->PlayerMothership);
                }
                else
                {
                    timeDecrement = 1.0f;
                }

                topic->timeleft -= timeDecrement;
                if (topic->timeleft < 0)
                {
                    firstshiptype = FirstShipTypeOfRace[player->race];
                    lastshiptype = LastShipTypeOfRace[player->race];
                    for (i = firstshiptype; i <= lastshiptype; i++)
                    {
                        if (player->race == RACE_Beast && !player->cataPreFogUnknown2.beastCanBuildByShipType[i])
                        {
                            shipcanbuild[i] = FALSE;
                        }
                        else
                        {
                            shipcanbuild[i] = rmCanBuildShip(player, i);
                        }
                    }

                    // Insert sound for technology completed
                    handle = speechEventQueue(NULL, STAT_F_Research_Completed, -1, -1, 0, index, -1, 5.0f, -1);
                    if (handle != -1)
                    {
                        speechEventQueue(NULL, STAT_F_Research_CompletedTopic, topic->techresearch, -1, 0, index, handle, 5.0f, -1);
                    }

                    topic->timeleft = 0;

                    rmActivateTech((udword *)rmPlayerCurrentPrereqMask(player), topic->techresearch);
                    rmUpdateBuildCapabilities(player);
                    listRemoveNode(&topic->link);
                    listAddNode(&deletelist,&topic->link,topic);

                    for (labindex=0;labindex<rmPlayerResearchLabCount(player);labindex++)
                    {
                        lab = rmPlayerResearchLab(player, labindex);
                        if (lab->topic==topic)
                        {
                            lab->labstatus = LS_NORESEARCHITEM;
                            lab->topic     = NULL;
                            if (rmGUIActive)
                                rmClearLab(labindex);
                        }
                    }

                    for (i = firstshiptype; i <= lastshiptype; i++)
                    {
                        if (player->race == RACE_Beast && !player->cataPreFogUnknown2.beastCanBuildByShipType[i])
                        {
                            canBuildNow = FALSE;
                        }
                        else
                        {
                            canBuildNow = rmCanBuildShip(player, i);
                        }

                        if (shipcanbuild[i] != canBuildNow)
                        {
                            speechEventFleet(STAT_F_Research_CompletedShip, i, index);
                            if (cmActive)
                            {
                                cmUpdateShipsAvailable();
                            }
                        }
                    }

                    if (singlePlayerGame && (index == 0))
                    {
                        tmTechForSale[topic->techresearch] = TM_TECH_IS_ALREADY_OWNED;
                    }

                    if (rmGUIActive)
                        rmUpdateTechList();
                }

                walk = walk->next;
            }

            listDeleteAll(&deletelist);
        }
    }
}

sdword *rmAccumulatePrerequisitesRecursive(sdword *result, struct Player *player, sdword techType)
{
    sdword i;
    sdword accumulated[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    PlayerShipPrereqMask *techMask;

    techMask = &player->discoveredShipPrereqs[RM_PREREQ_TECH_ENTRY_BASE + techType];
    memcpy(result, techMask->value, sizeof(techMask->value));

    for (i = 0; i < NumTechnologies; i++)
    {
        if (techMask->value[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || techMask->value[i] == 1)
        {
            rmAccumulatePrerequisitesRecursive(accumulated, player, i);
            rmOrPrereqMasks(result, result, accumulated);
        }
    }

    return result;
}

sdword rmCountBuildPrerequisites(struct Player *player, sdword shipType)
{
    sdword shipMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword combinedTechMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword prerequisiteMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword unresearchedMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword techIndex;
    sdword count;

    memcpy(shipMask, player->discoveredShipPrereqs[shipType].value, sizeof(shipMask));
    memset(combinedTechMask, 0, sizeof(combinedTechMask));

    for (techIndex = 0; techIndex < NumTechnologies; techIndex++)
    {
        if (shipMask[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || shipMask[techIndex] == 1)
        {
            rmAccumulatePrerequisitesRecursive(prerequisiteMask, player, techIndex);
            rmOrPrereqMasks(combinedTechMask, combinedTechMask, prerequisiteMask);
        }
    }

    rmOrPrereqMasks(shipMask, shipMask, combinedTechMask);
    rmBuildUnresearchedTechMask(unresearchedMask, rmPlayerCurrentPrereqMask(player));
    rmPreRequisitesAnd(shipMask, shipMask, unresearchedMask);

    count = 0;
    for (techIndex = 0; techIndex < NumTechnologies; techIndex++)
    {
        if (shipMask[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || shipMask[techIndex] == 1)
        {
            count++;
        }
    }

    return count;
}

/*-----------------------------------------------------------------------------
    Name        : rmTechRequiredForShip
    Description : This function returns the number of research topics required
                  to build the ship specified
    Inputs      : player and shiptype
    Outputs     : number of research topics required
    Return      : sdword
----------------------------------------------------------------------------*/
sdword rmTechRequiredForShip(Player *player, ShipType type)
{
    return rmCountBuildPrerequisites(player, type);
}

/*-----------------------------------------------------------------------------
    Name        : rmResearchTechForShip
    Description : This function researches technology required to build a ship.
    Inputs      : player and shiptype
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
bool32 rmResearchTechForShip(struct Player *player, TechnologyType techType)
{
    sdword *currentPrereqMask;
    sdword freelab;

    currentPrereqMask = rmPlayerCurrentPrereqMask(player);
    if (currentPrereqMask[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || currentPrereqMask[techType] == 1)
    {
        return FALSE;
    }

    freelab = rmFindFreeLab(player, techType);
    if (freelab == -1)
    {
        return FALSE;
    }

    clWrapResearch(0, player->playerIndex, freelab, techType);
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : rmGiveTechToPlayerByName
    Description : Gives player technology 'techName' and all previous technologies
                  needed to get it
    Inputs      : player - player to get tech
                  techName - name of technology as defined in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rmGiveTechToPlayerByType(struct Player *player, TechnologyType techtype)
{
    sdword prerequisiteDiff[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword *currentPrereqMask;
    sdword techCanGet;

    currentPrereqMask = rmPlayerCurrentPrereqMask(player);

    while (TRUE)
    {
        rmPreRequisitesAnd(prerequisiteDiff,
                           player->discoveredShipPrereqs[RM_PREREQ_TECH_ENTRY_BASE + techtype].value,
                           currentPrereqMask);

        for (techCanGet = NumTechnologies - 1; techCanGet >= 0; techCanGet--)
        {
            if (prerequisiteDiff[techCanGet] == 1)
            {
                break;
            }
        }

        if (techCanGet <= 0)
        {
            break;
        }

        rmActivateTech((udword *)currentPrereqMask, techCanGet);
        rmUpdateBuildCapabilities(player);
    }

    rmActivateTech((udword *)currentPrereqMask, techtype);
    rmUpdateBuildCapabilities(player);
}

/*-----------------------------------------------------------------------------
    Name        : rmInitializeResearchStatics
    Description : Reset the research statics for a given player
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void rmInitializeResearchStatics(struct Player *player)
{
    if (singlePlayerGame)
    {
        if (player->race == RACE_Sect)
        {
            player->discoveredShipPrereqs = SinglePlayerPrereqTable;
        }
        else if (player->race == RACE_Beast)
        {
            player->discoveredShipPrereqs = MultiPlayerBeastPrereqTable;
        }
    }
    else if (player->race == RACE_Sect)
    {
        player->discoveredShipPrereqs = MultiPlayerSectPrereqTable;
    }
    else if (player->race == RACE_Beast)
    {
        player->discoveredShipPrereqs = MultiPlayerBeastPrereqTable;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmInitializeResearchStruct
    Description : initializes research structure for player
    Inputs      : Player
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmInitializeResearchStruct(Player *player, bool32 candoresearch, sdword techlevel)
{
    sdword index;

    *rmPlayerCanDoResearch(player) = candoresearch;
    rmSetPrereqMaskState(rmPlayerCurrentPrereqMask(player), techlevel);
    rmInitializeResearchStatics(player);
    rmInitializeReferenceShipPrereqs(player);

    for (index=0;index<NUM_CATA_RESEARCHLABS;index++)
    {
        player->cataResearchRuntime.researchlabs[index].labstatus = LS_NORESEARCHSHIP;
        player->cataResearchRuntime.researchlabs[index].topic = NULL;
    }

    listInit(&player->cataResearchRuntime.listoftopics);
    if (player->race == RACE_Beast)
    {
        player->cataResearchRuntime.researchlabs[0].labstatus = LS_NORESEARCHITEM;
    }

    rmUpdateBuildCapabilities(player);
}


/*-----------------------------------------------------------------------------
    Name        : rmAPIStartup
    Description : starts up the API for the research manager loading all dependancies
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmAPIStartup(void)
{
    rmResetStaticInfo();
    rmGUIStartup();
}


/*-----------------------------------------------------------------------------
    Name        : rmAPIShutdown
    Description : shut down the API for the research manager
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmAPIShutdown(void)
{
    rmGUIShutdown();
}


/*-----------------------------------------------------------------------------
    Name        : rmEnableShipFunction
    Description : disables a ship so that a player cannot build it
    Inputs      : race - race of shipt to disable
                  ship - type of ship to disable
                  bEnable - TRUE to enable, FALSE to disable
    Outputs     : none
    Return      : none
----------------------------------------------------------------------------*/
sdword ModuleCompleted(Player *player, ShipType shiptype, bool32 announce)
{
    sdword moduleType;
    ResearchLab *lab;
    sdword result = shiptype - sMothershipSpecial;

    switch (shiptype)
    {
        case sMothershipDockingBay:
            moduleType = MODULE_HANGAR;
            break;
        case sMothershipArmour:
            moduleType = MODULE_ARMOUR;
            break;
        case sMothershipMicro:
            moduleType = MODULE_ENGINEERING;
            break;
        case sMothershipSpecial:
            moduleType = MODULE_ADVANCED_ENGINEERING;
            break;
        case sMothershipWeapons:
            moduleType = MODULE_WEAPON;
            break;
        default:
            return result;
    }

    if (announce &&
        universe.curPlayerPtr == player &&
        (moduleType != MODULE_HANGAR || !singlePlayerGame || singlePlayerGameInfo.currentMission != 4))
    {
        speechEventQueue(NULL, 33558576, moduleType, -1, 0, player->playerIndex, -1, 5.0f, -1);
        battleChatAddMessage(strGetString(strResearchModuleActivated), colRGB(255, 0, 0), 2, 0);
    }

    lab = rmPlayerResearchLab(player, moduleType);
    result = lab->labstatus;
    if (result == LS_NORESEARCHSHIP)
    {
        lab->labstatus = LS_NORESEARCHITEM;
        if (rmGUIActive && player == universe.curPlayerPtr)
        {
            rmUpdateTechList();
        }
    }

    return result;
}

sdword ModuleDestroyed(Player *player, ShipType shiptype)
{
    sdword moduleType;
    ResearchLab *lab;
    sdword result = shiptype - sMothershipSpecial;

    switch (shiptype)
    {
        case sMothershipDockingBay:
            moduleType = MODULE_HANGAR;
            break;
        case sMothershipArmour:
            moduleType = MODULE_ARMOUR;
            break;
        case sMothershipMicro:
            moduleType = MODULE_ENGINEERING;
            break;
        case sMothershipSpecial:
            moduleType = MODULE_ADVANCED_ENGINEERING;
            break;
        case sMothershipWeapons:
            moduleType = MODULE_WEAPON;
            break;
        default:
            return result;
    }

    lab = rmPlayerResearchLab(player, moduleType);
    if (lab == NULL)
    {
        return result;
    }

    rmClearResearchlab(player, moduleType);
    lab->labstatus = LS_NORESEARCHSHIP;
    if (rmGUIActive && player == universe.curPlayerPtr)
    {
        rmUpdateTechList();
    }

    return result;
}

sdword BeastMothershipDestroyed(Player *player)
{
    ResearchLab *lab = rmPlayerResearchLab(player, 0);

    if (lab == NULL)
    {
        return 0;
    }

    rmClearResearchlab(player, 0);
    lab->labstatus = LS_NORESEARCHSHIP;
    if (rmGUIActive && player == universe.curPlayerPtr)
    {
        rmUpdateTechList();
    }

    return 0;
}

void rmEnableShip(ShipRace race, ShipType ship, bool32 bEnabled)
{
    sdword state = bEnabled ? 0 : -2;

    if ((udword)ship <= (udword)bMissileCorvette || ship == dAcolyte)
    {
        if (race == RACE_Sect)
        {
            rmSetPrereqMaskState(SinglePlayerPrereqTable[ship].value, state);
            if (ship == dAcolyte)
            {
                state = -2;
            }
            rmSetPrereqMaskState(MultiPlayerSectPrereqTable[ship].value, state);
        }
        else if (race == RACE_Beast)
        {
            rmSetPrereqMaskState(MultiPlayerBeastPrereqTable[ship].value, state);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmResetStaticInfo
    Description : resets the static info to what was in the script file.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmResetStaticInfo(void)
{
    memset(&SinglePlayerTechStatic, 0, sizeof(SinglePlayerTechStatic));
    memset(&MultiPlayerSectTechStatic, 0, sizeof(MultiPlayerSectTechStatic));
    memset(&MultiPlayerBeastTechStatic, 0, sizeof(MultiPlayerBeastTechStatic));
    memset(SinglePlayerPrereqTable, 0, sizeof(SinglePlayerPrereqTable));
    memset(MultiPlayerSectPrereqTable, 0, sizeof(MultiPlayerSectPrereqTable));
    memset(MultiPlayerBeastPrereqTable, 0, sizeof(MultiPlayerBeastPrereqTable));

#if (defined(CGW) || defined(Downloadable) /*|| defined(OEM)*/)
    scriptSet(NULL,"DemoResearchDepend.script",TechnologyDependancies);
#else
    scriptSet(NULL,"ResearchDepend.script",TechnologyDependancies);
#endif

    rmSetPrintList(RM_SP, &SinglePlayerTechStatic);
    rmSetPrintList(RM_MPS, &MultiPlayerSectTechStatic);
    rmSetPrintList(RM_MPB, &MultiPlayerBeastTechStatic);
}


/*-----------------------------------------------------------------------------
    Name        : rmSetDependCB
    Description : sets the ship technology dependancies based on a list of strings
    Inputs      : standard callback inputs
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSetShipDependCB(char *directory, char *field, void *dataToFillIn)
{
    PlayerShipPrereqMask *prereqBase = dataToFillIn;
    sdword index;
    char temp[64];
    ShipType shiptype;
    TechnologyType techtype;
    sdword prereqMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT] = { 0 };
    (void)directory;

    for (index = 0; (*field != ' ') && (*field != '\t') && (*field != 0); field++)
    {
        dbgAssert(index < (sdword)(sizeof(temp) - 1));
        temp[index++] = *field;
    }
    temp[index] = 0;

    while (((*field == ' ') || (*field == '\t')) && (*field != 0))
        field++;

    if (*field == 0) return;

    if (*field != 0)
    {
        RemoveCommasFromString(field);

        shiptype = StrToShipType(temp);

        dbgAssert(shiptype!=-1);

        while (*field != 0)
        {
            for (index = 0; (*field != ' ') && (*field != '\t') && (*field != 0); field++)
            {
                dbgAssert(index < (sdword)(sizeof(temp) - 1));
                temp[index++] = *field;
            }
            temp[index] = 0;

            techtype = StrToTechType(temp);

            dbgAssert(techtype != -1);

            rmActivateTech((udword *)prereqMask, techtype);

            while ((*field == ' ') || (*field == '\t'))
                field++;

        }
    }

    memcpy(prereqBase[shiptype].value, prereqMask, sizeof(prereqMask));
}

/*-----------------------------------------------------------------------------
    Name        : rmSetTechDependCB
    Description : sets the technology dependancies based on a list of strings
    Inputs      : standard callback inputs
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSetTechDependCB(char *directory, char *field, void *dataToFillIn)
{
    PlayerShipPrereqMask *prereqBase = dataToFillIn;
    sdword index;
    char temp[64];
    TechnologyType techtype, techset;
    sdword prereqMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT] = { 0 };
    (void)directory;

    for (index = 0; (*field != ' ') && (*field != '\t') && (*field != 0); field++)
    {
        dbgAssert(index < (sdword)(sizeof(temp) - 1));
        temp[index++] = *field;
    }
    temp[index] = 0;

    while (((*field == ' ') || (*field == '\t')) && (*field != 0))
        field++;

    if (*field == 0) return;

    if (*field != 0)
    {

        RemoveCommasFromString(field);

        techset = StrToTechType(temp);

        dbgAssert(techset!=-1);

        while (*field != 0)
        {
            for (index = 0; (*field != ' ') && (*field != '\t') && (*field != 0); field++)
            {
                dbgAssert(index < (sdword)(sizeof(temp) - 1));
                temp[index++] = *field;
            }
            temp[index] = 0;

            techtype = StrToTechType(temp);

            dbgAssert(techtype != -1);

            rmActivateTech((udword *)prereqMask, techtype);

            while ((*field == ' ') || (*field == '\t'))
                field++;

        }
    }

    memcpy(prereqBase[techset].value, prereqMask, sizeof(prereqMask));
}


sdword *rmPreRequisitesAnd(sdword *result, sdword *prereq1, sdword *prereq2)
{
    sdword i;

    if (prereq1[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY)
    {
        if (prereq2[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY)
        {
            memset(result, 0, sizeof(sdword) * PLAYER_SHIP_PREREQ_ENTRY_COUNT);
            result[PLAYER_SHIP_PREREQ_STATE_INDEX] = TECH_ALLTECHNOLOGY;
            return result;
        }

        memcpy(result, prereq2, sizeof(sdword) * PLAYER_SHIP_PREREQ_TECH_COUNT);
        result[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
        return result;
    }

    if (prereq2[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY)
    {
        memcpy(result, prereq1, sizeof(sdword) * PLAYER_SHIP_PREREQ_TECH_COUNT);
        result[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
        return result;
    }

    for (i = 0; i < PLAYER_SHIP_PREREQ_TECH_COUNT; i++)
    {
        result[i] = prereq1[i] & prereq2[i];
    }
    result[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
    return result;
}

bool32 rmTechDependenciesEqual(sdword *techA, sdword *techB)
{
    if (techA[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY &&
        techB[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY)
    {
        return TRUE;
    }

    return memcmp(techA, techB, sizeof(sdword) * PLAYER_SHIP_PREREQ_TECH_COUNT) == 0;
}

typedef struct rmBuildMaskEntry
{
    sdword techType;
    udword primaryMask;
    udword secondaryMask;
} rmBuildMaskEntry;

typedef struct rmModuleTechEntry
{
    sdword techType;
    sdword enabledFlagOffset;
} rmModuleTechEntry;

static const rmBuildMaskEntry rmBuildMaskEntries[] =
{
    { TechADriveFighter,   RESEARCH_UPGRADE_MASK_ADRIVE_FIGHTER,      0 },
    { TechADriveFrigate,   RESEARCH_UPGRADE_MASK_ADRIVE_FRIGATE,      0 },
    { TechADriveDestroyer, RESEARCH_UPGRADE_MASK_ADRIVE_DESTROYER,    0 },
    { TechLink,            RESEARCH_UPGRADE_MASK_LINK_PRIMARY,        RESEARCH_UPGRADE_MASK_LINK_SECONDARY },
    { TechMimic,           RESEARCH_UPGRADE_MASK_MIMIC,               0 },
    { TechMicroShip,       RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY,   0 },
    { TechExplosiveCharge, RESEARCH_UPGRADE_MASK_EXPLOSIVE_CHARGE,    0 },
    { TechAfterburner,     RESEARCH_UPGRADE_MASK_AFTERBURNER,         0 },
    { TechEMP,             RESEARCH_UPGRADE_MASK_EMP,                 0 },
    { TechRepulse,         RESEARCH_UPGRADE_MASK_REPULSE,             0 },
    { TechMicroShip,       RESEARCH_UPGRADE_MASK_MICROSHIP_SECONDARY, 0 },
    { TechForceField1,     RESEARCH_UPGRADE_MASK_FORCE_FIELD_1,       0 },
    { TechForceField2,     RESEARCH_UPGRADE_MASK_FORCE_FIELD_2,       0 },
    { TechForceField3,     RESEARCH_UPGRADE_MASK_FORCE_FIELD_3,       0 },
    { TechDefenseField,    RESEARCH_UPGRADE_MASK_DEFENSE_FIELD,       0 },
    { TechInfect,          RESEARCH_UPGRADE_MASK_INFECT,              0 },
    { TechCloaking,        RESEARCH_UPGRADE_MASK_CLOAKING,            0 },
    { TechRepairBots,      RESEARCH_UPGRADE_MASK_REPAIR_BOTS,         0 },
    { TechSalvage,         RESEARCH_UPGRADE_MASK_SALVAGE,             0 },
    { TechCrystalHarvest,  RESEARCH_UPGRADE_MASK_CRYSTAL_HARVEST,     0 },
    { TechRepair,          RESEARCH_UPGRADE_MASK_REPAIR,              0 },
    { TechBeastVaccine,    RESEARCH_UPGRADE_MASK_BEAST_VACCINE,       0 },
    { TechAdvancedSensors, RESEARCH_UPGRADE_MASK_ADVANCED_SENSORS,    0 },
    { TechIon,             RESEARCH_UPGRADE_MASK_ION,                 0 },
    { TechMissile,         RESEARCH_UPGRADE_MASK_MISSILE,             0 },
};

static const rmModuleTechEntry rmModuleTechEntries[] =
{
    { RM_MODULE_TECH_NONE, 0 }, { 18, 1 }, { 19, 2 }, { 2, 3 }, { RM_MODULE_TECH_NONE, 4 },
    { 20, 5 }, { RM_MODULE_TECH_NONE, 6 }, { 27, 7 }, { 32, 8 },
};

/* Retail keeps upgrade-level tech unlocks in a separate table from module slot states. */
static const sdword rmUpgradeLevelTechEntries[] =
{
    1, 1, 1, 1, 1, 1, 2, 2, 2,
};

static bool32 rmPrereqTechIsResearched(const sdword *researchMask, sdword techType)
{
    return researchMask[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || researchMask[techType] == 1;
}

void rmActivateTech(udword *researchMask, int techIndex)
{
    bool32 refreshSensors = FALSE;
    Node *node;

    if (techIndex == -1 || techIndex == 2 || techIndex == 28)
    {
        refreshSensors = TRUE;
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *ship = node->structptr;
            if (playerGetSensorCoverage(ship->playerowner) && physShipIsMovable(ship))
            {
                univPingShipDied((sdword)ship);
            }
        }
    }

    researchMask[techIndex] = 1;

    if (refreshSensors)
    {
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *ship = node->structptr;
            if (playerGetSensorCoverage(ship->playerowner) && physShipIsMovable(ship))
            {
                univUpdateShipProximity(ship);
            }
        }
    }
}

BOOL rmUpdateBuildCapabilities(Player *player)
{
    sdword i;
    sdword shipLevel;
    udword shipMask;
    sdword *currentPrereqMask = rmPlayerCurrentPrereqMask(player);

    if (rmPrereqTechIsResearched(currentPrereqMask, 15))
    {
        shipLevel = 2;
    }
    else if (rmPrereqTechIsResearched(currentPrereqMask, 14))
    {
        shipLevel = 1;
    }
    else
    {
        shipLevel = 0;
    }

    player->shipUpgrades.level = shipLevel;

    for (i = 0; i < (sdword)(sizeof(rmModuleTechEntries) / sizeof(rmModuleTechEntries[0])); i++)
    {
        bool32 enabled = (rmModuleTechEntries[i].techType == RM_MODULE_TECH_NONE) ||
                         rmPrereqTechIsResearched(currentPrereqMask, rmModuleTechEntries[i].techType);
        player->shipUpgrades.moduleTechEnabled[rmModuleTechEntries[i].enabledFlagOffset] = enabled;
    }

    shipMask = player->shipUpgrades.mask;
    for (i = 0; i < (sdword)(sizeof(rmBuildMaskEntries) / sizeof(rmBuildMaskEntries[0])); i++)
    {
        if (rmPrereqTechIsResearched(currentPrereqMask, rmBuildMaskEntries[i].techType))
        {
            shipMask |= rmBuildMaskEntries[i].primaryMask;
            shipMask |= rmBuildMaskEntries[i].secondaryMask;
        }
    }
    player->shipUpgrades.mask = shipMask;

    return shipMask != 0;
}

void shipBuildUpgradeStatusArray(Ship *ship, udword *statusArray)
{
    sdword i;
    ShipUpgradeSlotStateBlock *slotStateBlock;

    memset(statusArray, 0, sizeof(udword) * PLAYER_SHIP_PREREQ_ENTRY_COUNT);

    for (i = 0; i < (sdword)(sizeof(rmBuildMaskEntries) / sizeof(rmBuildMaskEntries[0])); i++)
    {
        if ((ship->upgradeState.mask & rmBuildMaskEntries[i].primaryMask) != 0 ||
            (ship->upgradeState.mask & rmBuildMaskEntries[i].secondaryMask) != 0)
        {
            statusArray[rmBuildMaskEntries[i].techType] = 1;
        }
    }

    slotStateBlock = (ShipUpgradeSlotStateBlock *)ship->unk_490;
    if (slotStateBlock != NULL)
    {
        sdword slotIndex;

        for (i = 0; i < (sdword)(sizeof(rmModuleTechEntries) / sizeof(rmModuleTechEntries[0])); i++)
        {
            for (slotIndex = 0; slotIndex < slotStateBlock->count; slotIndex++)
            {
                if (slotStateBlock->entries[slotIndex].state == (udword)rmModuleTechEntries[i].enabledFlagOffset)
                {
                    statusArray[rmModuleTechEntries[i].techType] = 1;
                    break;
                }
            }
        }
    }

    if (ship->upgradeState.level >= 0)
    {
        for (i = 0; i <= ship->upgradeState.level &&
                    i < (sdword)(sizeof(rmUpgradeLevelTechEntries) / sizeof(rmUpgradeLevelTechEntries[0])); i++)
        {
            if (rmUpgradeLevelTechEntries[i] != RM_MODULE_TECH_NONE)
            {
                statusArray[rmUpgradeLevelTechEntries[i]] = 1;
            }
        }
    }
}

static ShipRace rmShipTypeStaticRace(ShipType shipType)
{
    if (shipType <= sMothershipOreCanArmour)
    {
        return RACE_Sect;
    }
    if (shipType <= bMissileCorvette)
    {
        return RACE_Beast;
    }
    return RACE_Sect;
}

static void shipTypeBuildUpgradeStatusArray(ShipType shipType, PlayerShipPrereqMask *statusMask)
{
    ShipStaticInfo *staticInfo;
    TractorBeamStatic *tractorEmitterStatic;
    udword upgradeMask;
    sdword i;
    sdword maxLevel;

    memset(statusMask, 0, sizeof(*statusMask));

    if (shipType > bMissileCorvette)
    {
        return;
    }

    upgradeMask = shipTypeGetUpgradeMask(shipType);
    for (i = 0; i < (sdword)(sizeof(rmBuildMaskEntries) / sizeof(rmBuildMaskEntries[0])); i++)
    {
        if ((upgradeMask & rmBuildMaskEntries[i].primaryMask) != 0 ||
            (upgradeMask & rmBuildMaskEntries[i].secondaryMask) != 0)
        {
            statusMask->value[rmBuildMaskEntries[i].techType] = 1;
        }
    }

    staticInfo = GetShipStaticInfo(shipType, rmShipTypeStaticRace(shipType));
    tractorEmitterStatic = staticInfo->tractorEmitterStatic;
    if (statusMask->value[18] == 0 && tractorEmitterStatic != NULL)
    {
        sdword beamIndex;
        sdword beamCount = (sdword)tractorEmitterStatic->position.x;
        sdword *beamState = (sdword *)&tractorEmitterStatic->nozzlenormal.z;

        for (beamIndex = 0; beamIndex < beamCount; beamIndex++, beamState += 71)
        {
            if (beamState[-1] != *beamState && *beamState == 1)
            {
                statusMask->value[18] = 1;
                break;
            }
        }
    }

    maxLevel = shipTypeGetMaxUpgradeLevel(shipType);
    for (i = 0; i < maxLevel && i < (sdword)(sizeof(rmUpgradeLevelTechEntries) / sizeof(rmUpgradeLevelTechEntries[0])); i++)
    {
        if (rmUpgradeLevelTechEntries[i] != RM_MODULE_TECH_NONE)
        {
            statusMask->value[rmUpgradeLevelTechEntries[i]] = 1;
        }
    }
}

static void rmInitializeReferenceShipPrereqs(Player *player)
{
    sdword shipType;

    memset(player->referenceShipPrereqs, 0, sizeof(player->referenceShipPrereqs));

    if (player->race == RACE_Beast)
    {
        for (shipType = bRecon; shipType <= bMissileCorvette; shipType++)
        {
            PlayerShipPrereqMask *referenceMask = &player->referenceShipPrereqs[shipType];

            switch (shipType)
            {
                case bRecon:
                    referenceMask->value[28] = 1;
                    break;
                case bIonArrayFrigate:
                    referenceMask->value[0] = 1;
                    break;
                case bHeavyCruiser:
                case bCruiseMissile:
                    referenceMask->value[32] = 1;
                    break;
                case bCarrier:
                case bMothership:
                    referenceMask->value[34] = 1;
                    referenceMask->value[32] = 1;
                    break;
                case bWorker:
                    referenceMask->value[7] = 1;
                    break;
            }
        }
        return;
    }

    for (shipType = sRecon; shipType <= sMothershipOreCanArmour; shipType++)
    {
        shipTypeBuildUpgradeStatusArray(shipType, &player->referenceShipPrereqs[shipType]);
    }
}

static sdword rmBeastShipTechGroupUnlocked(const Player *player, sdword shipType)
{
    return player->cataPreFogUnknown2.beastTechGroupUnlockedByShipType[shipType];
}

static void rmSetBeastShipTechGroupUnlocked(Player *player, sdword shipType, sdword value)
{
    player->cataPreFogUnknown2.beastTechGroupUnlockedByShipType[shipType] = value;
}

static void rmUnlockBeastTechListEntry(TechPrintList *techList, const Player *player, ShipType shipType, sdword techIndex, sword unlockType)
{
    bool32 groupEnabled = TRUE;
    TechPrintList *entry;

    for (entry = techList; entry->itemtype != ITEM_ENDLIST; entry++)
    {
        if (entry->itemtype != ITEM_CLASSHEADER)
        {
            if (groupEnabled &&
                entry->itemtype == ITEM_TECHHIDDEN &&
                entry->itemID == techIndex)
            {
                entry->itemtype = unlockType;
            }
        }
        else
        {
            sdword groupShipType = rmMothershipModuleShipTypeToTechType(entry->itemID);

            groupEnabled = (groupShipType == shipType) ||
                           rmBeastShipTechGroupUnlocked(player, groupShipType);
        }
    }
}

Player *rmRecomputeShipPrereqMask(Ship *ship)
{
    Player *player = ship->playerowner;
    PlayerShipPrereqMask *discoveredTable = player->discoveredShipPrereqs;
    PlayerShipPrereqMask *referenceMask;
    TechPrintList *techList;
    udword statusArray[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword i;

    if (discoveredTable == NULL)
    {
        return player;
    }

    referenceMask = &player->referenceShipPrereqs[ship->shiptype];
    shipBuildUpgradeStatusArray(ship, statusArray);

    if (statusArray[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY ||
        referenceMask->value[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY)
    {
        memset(statusArray, 0, sizeof(statusArray));
        statusArray[PLAYER_SHIP_PREREQ_STATE_INDEX] = TECH_ALLTECHNOLOGY;
    }
    else
    {
        for (i = 0; i < PLAYER_SHIP_PREREQ_TECH_COUNT; i++)
        {
            statusArray[i] |= (udword)referenceMask->value[i];
        }
        statusArray[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
    }

    if (player == universe.curPlayerPtr && !player->aiPlayer)
    {
        techList = rmGetCurrentPlayerTechList();

        for (i = 0; i < PLAYER_SHIP_PREREQ_TECH_COUNT; i++)
        {
            if (i == TechRepairBots ||
                i == TechSalvage ||
                i == TechCrystalHarvest ||
                i == TechSupportSystems ||
                i == TechInfect ||
                i == TechAdvancedSensors ||
                (ship->shiptype == bAcolyte && i == TechLink))
            {
                continue;
            }

            if ((discoveredTable[ship->shiptype].value[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY ||
                 discoveredTable[ship->shiptype].value[i] == 1) &&
                techList->itemtype != ITEM_ENDLIST)
            {
                rmUnlockBeastTechListEntry(techList, player, ship->shiptype, i, ITEM_MODULETECH);
            }
        }

        for (i = 0; i < PLAYER_SHIP_PREREQ_TECH_COUNT; i++)
        {
            if (i == TechRepairBots ||
                i == TechSalvage ||
                i == TechCrystalHarvest ||
                i == TechSupportSystems ||
                i == TechInfect ||
                i == TechAdvancedSensors ||
                (ship->shiptype == bACV && i == TechLink))
            {
                continue;
            }

            if ((statusArray[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || statusArray[i] == 1) &&
                techList->itemtype != ITEM_ENDLIST)
            {
                rmUnlockBeastTechListEntry(techList, player, ship->shiptype, i, ITEM_SHIPTECH);
            }
        }

        if (rmGUIActive)
        {
            rmUpdateTechList();
        }
    }

    memcpy(referenceMask->value, statusArray, sizeof(statusArray));
    rmSetBeastShipTechGroupUnlocked(player, ship->shiptype, TRUE);

    return player;
}

int rmClearPrereqMask(udword *techMask)
{
    memset(techMask, 0, sizeof(sdword) * PLAYER_SHIP_PREREQ_ENTRY_COUNT);
    return 0;
}






void rmOrPrereqMasks(sdword *dest, sdword *lhs, sdword *rhs)
{
    sdword i;

    if ((lhs[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY) ||
        (rhs[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY))
    {
        memset(dest, 0, sizeof(sdword) * PLAYER_SHIP_PREREQ_ENTRY_COUNT);
        dest[PLAYER_SHIP_PREREQ_STATE_INDEX] = TECH_ALLTECHNOLOGY;
        return;
    }

    for (i = 0; i < PLAYER_SHIP_PREREQ_TECH_COUNT; i++)
    {
        dest[i] = lhs[i] | rhs[i];
    }
    dest[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
}



void rmSetPrereqMaskState(sdword *prereqMask, sdword state)
{
    Node *node;
    bool32 refreshSensors = FALSE;

    if (state == RM_PREREQ_MASK_CLEAR_STATE || state == TECH_ALLTECHNOLOGY || state == TechAdvancedSensors)
    {
        refreshSensors = TRUE;
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *ship = node->structptr;
            univPingShipDied((sdword)ship);
        }
    }

    switch (state)
    {
        case RM_PREREQ_MASK_DISABLED_STATE:
            prereqMask[PLAYER_SHIP_PREREQ_STATE_INDEX] = RM_PREREQ_MASK_DISABLED_STATE;
            break;
        case RM_PREREQ_MASK_CLEAR_STATE:
            memset(prereqMask, 0, sizeof(sdword) * PLAYER_SHIP_PREREQ_ENTRY_COUNT);
            break;
        case TECH_ALLTECHNOLOGY:
            prereqMask[PLAYER_SHIP_PREREQ_STATE_INDEX] = TECH_ALLTECHNOLOGY;
            break;
        default:
            prereqMask[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
            break;
    }

    if (refreshSensors)
    {
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *ship = node->structptr;
            univUpdateShipProximity(ship);
        }
    }
}

sdword *rmBuildUnresearchedTechMask(sdword *dest, sdword *source)
{
    sdword i;

    if (source[PLAYER_SHIP_PREREQ_STATE_INDEX] != 0)
    {
        memset(dest, 0, sizeof(sdword) * PLAYER_SHIP_PREREQ_ENTRY_COUNT);
        return NULL;
    }

    for (i = 0; i < PLAYER_SHIP_PREREQ_TECH_COUNT; i++)
    {
        dest[i] = (source[i] == 0);
    }
    dest[PLAYER_SHIP_PREREQ_STATE_INDEX] = 0;
    return dest;
}

static sdword rmMothershipModuleShipTypeToTechType(sdword shipType)
{
    switch (shipType)
    {
        case 10: return 33;
        case 11: return 49;
        case 12: return 51;
        case 13: return 34;
        case 14: return 38;
        case 15: return 41;
        case 16: return 42;
        case 17: return 52;
        case 18: return 35;
        case 19: return 36;
        case 20: return 37;
        case 21: return 58;
        case 22: return 59;
        case 23: return 39;
        case 24: return 40;
        case 25: return 45;
        case 26: return 44;
        case 27: return 46;
        case 28: return 47;
        case 29: return 48;
        default: return -1;
    }
}
