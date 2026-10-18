// =============================================================================
//  CommandLayer.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 7/2/1997 by gshaw
// =============================================================================

#ifndef ___COMMANDLAYER_H
#define ___COMMANDLAYER_H

#include "Dock.h"
#include "Formation.h"
#include "ResCollect.h"
#include "CommandDefs.h"


#ifdef HW_BUILD_FOR_DEBUGGING

#define CL_TEXTFEEDBACK     1

#else

#define CL_TEXTFEEDBACK     0

#endif
/*=============================================================================
    Types:
=============================================================================*/

//Ru Transfer types for one person
#define RUTRANS_BUILDCANCEL     101
#define RUTRANS_BUILTSHIP       102
#define RUTRANS_GENERATEDRUS    103

//defines for hyperspaceing states
#define COM_HYPERSPACE_START    0
#define COM_HYPERSPACE_OUT      1
#define COM_HYPERSPACE_WAIT     2
#define COM_HYPERSPACE_IN       3

// Packed move orders reuse two vector lanes as a transport blob:
// from.x = packed pathState, {from.y, from.z, to.x} = initialPoint,
// to.y = packed pointCount, to.z = packed vector* points.
typedef struct
{
    vector from;
    vector to;
} MoveCommandPacked;

typedef struct
{
    vector heading;
    vector destination;
} MoveCommand;

typedef SelectAnyCommand AttackCommand;
typedef SelectAnyCommandMax6 AttackCommandMax6;
typedef SelectAnyCommand SpecialCommand;

#define PROTECTFLAGS_JUST_FOLLOW     1
typedef SelectCommand ProtectCommand;
typedef SelectCommandMax6 ProtectCommandMax6;

#define IAmAWingman(attackvar) ((attackvar)->myLeaderIs)
#define IAmALeader(attackvar) ((attackvar)->myWingmanIs)

typedef struct
{
    ShipPtr receiverShip;
} LaunchShipCommand;

typedef struct
{
    ShipType shipType;
    ShipRace shipRace;
    ShipPtr creator;
    udword frameAtWhichToCreate;
    udword playerIndex;
} BuildingShipCommand;

typedef struct
{
    uword order;
    uword attributes;
} CommandOrder;

typedef struct CommandToDo
{
    Node todonode;
    SelectCommand *selection;
    CommandOrder ordertype;
    MoveCommand move;
    ubyte cataCommandState0[508];
    AttackCommand *attack;
    FormationCommand formation;
    DockCommand dock;
    ubyte cataCommandState1[4];
    LaunchShipCommand launchship;
    CollectResourceCommand collect;
    ProtectCommand *protect;
    udword protectFlags;
    SpecialCommand *specialtargets;
    MilitaryParadeCommand *militaryParade;
    BuildingShipCommand buildingship;
    real32 tacticalupdatetime;                 //needed for optimal scanning of ship list in enemy detection for tactics
    real32 holdingStartTime;                //time that a command entered a holding pattern
    real32 hyperSpaceingTime;
    sdword hyperspaceState;
    sdword turnAroundTimes;
    vector holdingPatternVec;
    real32 pingUpdateTime;                  //last time this command was processed by pings
    void *updatedPing;                      //what ping it was last updated for
    ubyte cataCommandState2[48];
} CommandToDo;

static_assert(offsetof(CommandToDo, ordertype) == 0x14);
static_assert(offsetof(CommandToDo, formation) == 0x230);

#define dockCommandGetMode(command) ((command)->dock.formationToGoIntoAfterDockingID)
#define dockCommandSetMode(command, mode) \
    do { \
        (command)->dock.dockType = 0; \
        (command)->dock.formationToGoIntoAfterDockingID = (mode); \
    } while (0)
#define dockCommandGetWasHarvesting(command) ((bool32)((command)->cataCommandState1[0] != 0))
#define dockCommandSetWasHarvesting(command, value) ((command)->cataCommandState1[0] = (ubyte)((value) ? 1 : 0))
#define dockCommandGetAllDockingWithSameShip(command) ((bool32)((command)->cataCommandState1[1] != 0))
#define dockCommandSetAllDockingWithSameShip(command, value) ((command)->cataCommandState1[1] = (ubyte)((value) ? 1 : 0))
#define dockCommandGetAllNearShip(command) ((bool32)((command)->cataCommandState1[2] != 0))
#define dockCommandSetAllNearShip(command, value) ((command)->cataCommandState1[2] = (ubyte)((value) ? 1 : 0))
#define dockCommandGetHaveFlyedFarEnoughAway(command) ((bool32)((command)->cataCommandState1[3] != 0))
#define dockCommandSetHaveFlyedFarEnoughAway(command, value) ((command)->cataCommandState1[3] = (ubyte)((value) ? 1 : 0))
#define dockCommandClearLegacyState(command) \
    do { \
        (command)->dock.wasHarvesting = FALSE; \
        (command)->dock.allDockingWithSameShip = FALSE; \
        (command)->dock.allNearShip = FALSE; \
        (command)->dock.haveFlyedFarEnoughAway = FALSE; \
    } while (0)

typedef struct CommandLayer
{
    LinkedList todolist;
} CommandLayer;

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofAttackCommand sizeofSelectCommand
#define sizeofProtectCommand sizeofSelectCommand

// any variables you want to reset in CommandToDo upon initialization, put here
#define InitCommandToDo(command) memset(command,0,sizeof(CommandToDo))

/*=============================================================================
    Data:
=============================================================================*/
#if CL_TEXTFEEDBACK
extern bool32 enableTextFeedback;
#endif

/*=============================================================================
    Functions:
=============================================================================*/

// Initializing, Closing, Processing
void clInit(CommandLayer *comlayer);
void clClose(CommandLayer *comlayer);
void clReset(CommandLayer *comlayer);
void clProcess(CommandLayer *comlayer);
void clPostProcess(CommandLayer *comlayer);
void clBeamStaticScriptLoad(char *directory, char *filename, struct ShipStaticInfo *statinfo);
void clDeleteCommandEffect(CommandToDo *command);
void clSetAttribute(sdword ship, sdword value, sdword attribute);

void clChecksum(void);

// Ship Command Layer
void clMove(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove);
CommandToDo *clAttackThese(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clAutoAttackResolvedShipTargets(Ship *ship, CommandToDo *command, AttackCommand *rawTargets);
void clPassiveAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation);
sdword clDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith);
sdword clUpgrade(CommandLayer *comlayer,SelectCommand *selectcom,sdword upgradecom);
Ship *clCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource);
Resource *clCollectNearestResource(CommandLayer *comlayer,SelectCommand *selectcom,bool32 issueFeedback,bool32 playFoundResourceFeedback);
CommandToDo *clProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom);
void clSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets);
void clCloak(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom, bool32 cloak);
void clHalt(CommandLayer *comlayer,SelectCommand *selectcom);
void clHaltThese(CommandLayer *comlayer,SelectCommand *selectcom);
void clScuttle(CommandLayer *comlayer,SelectCommand *selectcom);
void clAutoLaunch(udword OnOff,udword playerIndex);
void clSetAlliance(udword AllianceType, uword curalliance, uword newalliance);
void clLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom);
void clSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype);
void clSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom);
void clSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom);
void clSetShipDisabled(CommandLayer *comlayer, SelectCommand *selectcom);
CommandToDo *clLinkSelection(CommandLayer *comlayer, SelectCommand *selectcom);
void clSiegeCannon(CommandLayer *comlayer,SelectCommand *selectcom,vector *target);
CommandToDo *clExternalConstructionSelection(CommandLayer *comlayer, SelectCommand *selectcom, sdword operation, sdword arg1, sdword arg2, sdword arg3);
void processUpdateExternalConstructionToDo(CommandLayer *comlayer, uword playerIndex, Ship *creator, Ship *ship, real32 progress, sdword operation);
void clRUTransfer(CommandLayer *comlayer, sdword toIndex, sdword fromIndex, sdword resourceUnits,ubyte flags);
void clMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clSetResearch(udword type, udword playernum, udword labnum, udword tech);
void clDeterministicBuild(udword command, CommandLayer* comlayer, sdword numShips, ShipType shipType, ShipRace shipRace, uword playerIndex, ShipPtr creator);
sdword cmGetShipTypeCount(struct Player *player, ShipType shipType);
void clSetModuleSlotActive(Ship *ship, ShipType moduleType, sdword iconId);
MoveCommandPacked clBuildMovePathCommand(sdword pathState, vector initialPoint, sdword pointCount, vector *points);
MoveCommandPacked clBuildMovePointCommand(vector initialPoint, vector *destination);
void clGetMoveDestination(CommandToDo *command, vector *destination);

// does ship have to launch
bool32 ShipHasToLaunch(Ship *InsideShip, Ship *ship);

// launch functions
void LaunchAllInternalShipsOfPlayerThatMustBeLaunched(struct Player *player);
sdword LaunchAllInternalShipsOfPlayer(struct Player *player, udword carriermask);

// checks if can ChangeOrderToPassiveAttack
bool32 canChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attack);

// changes order to passive attack
// WARNING: Must call canChangeOrderToPassiveAttack to see if you can call this
void ChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attackcom);

// changes order to attack
void ChangeOrderToAttack(CommandToDo *alreadycommand,AttackCommand *attackcom);

// changes order to move
void ChangeOrderToMove(CommandToDo *alreadycommand,MoveCommandPacked packedMove);

// Returns allocated command containing the ship
CommandToDo *getShipAndItsCommand(CommandLayer *comlayer,ShipPtr ship);
SelectCommand *getShipAndItsCommandSelection(CommandLayer *comlayer,ShipPtr ship,bool32 *parade);

// Returns allocated command containing the ship and its formation
CommandToDo *getShipAndItsFormationCommand(CommandLayer *comlayer,ShipPtr ship);

// Returns allocated selectcommand containing the ship and its formation
SelectCommand *getShipAndItsFormation(CommandLayer *comlayer,ShipPtr ship);

// returns TRUE if searchfor is in comlayer
bool32 CommandInCommandLayer(CommandLayer *comlayer,CommandToDo *searchfor);

// Call this function to see if this selection is already in formation
TypeOfFormation clSelectionAlreadyInFormation(CommandLayer *comlayer,SelectCommand *selectcom);

// Call this function to update ship command layer when ship dies
void clShipDied(CommandLayer *comlayer,ShipPtr deadship);

// Call this function to update ship command layer when resource dies
void clResourceDied(CommandLayer *comlayer,ResourcePtr resource);

// Call this function to update ship command layer when derelict dies
void clDerelictDied(CommandLayer *comlayer,DerelictPtr derelict);

// Call this function to update ship command layer when missile dies
void clMissileDied(CommandLayer *comlayer,MissilePtr missile);

// Tells if selectcom is already doing something in the command layer
CommandToDo *IsSelectionAlreadyDoingSomething(CommandLayer *comlayer,SelectCommand *selectcom);

// Removes ships in selectcom from doing stuff in the command layer
void RemoveShipsFromDoingStuff(CommandLayer *comlayer,SelectCommand *selectcom);

// values for removeFlag for RemoveShipFromBeingTargeted
#define REMOVE_PROTECT          1
#define REMOVE_CLOAKED          2
#define REMOVE_DISABLED         4
#define REMOVE_DISAPPEARED      8
#define REMOVE_HYPERSPACING     16
#define REMOVE_SALCAPTARGETING  32

// Removes ship from being targeted in the command layer
void RemoveShipFromBeingTargeted(CommandLayer *comlayer,ShipPtr shiptoremove,udword removeFlag, sdword playerIndex);

// cleans up game when a ship goes out of view..
void shipHasJustCloaked(Ship *ship);

// cleans up game when a ship becomes disabled
void shipHasJustBeenDisabled(Ship *ship);

// when ship just disappeared (SOF_Hide) e.g. univRemoveShipFromOutside
void shipHasJustDisappeared(Ship *ship);

// removes target info from command regarding shiptoremove
void RemoveShipReferencesFromExtraAttackInfo(Ship *shiptoremove,CommandToDo *todo);
void RemoveAttackTargetFromExtraAttackInfo(SpaceObjRotImpTarg *targettoremove,CommandToDo *todo);
void removeShipsFromDockingWithThisShip(struct Player *player);

void RemoveShipFromAttacking(Ship *ship);

// Initializes ship's AI state variables
void InitShipAI(Ship *ship,bool32 fresh);

// Initializes selection ships AI state variables
void InitShipsForAI(SelectCommand *selection,bool32 fresh);

// clears any protecting in command
void ClearProtecting(CommandToDo *command);

// clears any passive attack command
void ClearPassiveAttacking(CommandToDo *command);

// frees last order of command, so new order can be put in
void FreeLastOrder(CommandToDo *command);

// adds a ship to a formation group
void AddShipToFormationGroup(ShipPtr ship,CommandToDo *group);

// adds a ship to a command group
void AddShipToGroup(ShipPtr ship,CommandToDo *group);

// Adds a ship to the proper group around aroundShip
void GroupShip(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip);

// groups ship into military parade around aroundShip
void GroupShipIntoMilitaryParade(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip);

// sets ships to correct position according to command layer
void clPresetShipsToPosition(CommandLayer *comlayer);

// cleans up formation ship stuff
void RemoveShipFromFormation(Ship *ship);

void fixFlipTurnSelection(SelectCommand *selection,SelectCommand *global);

CommandToDo *GetMilitaryGroupAroundShip(CommandLayer *comlayer,Ship *aroundShip);

// call whenever creating a new command in command layer or ships changing their command
void PrepareShipsForCommand(CommandToDo *command,bool32 rowClear);
void PrepareOneShipForCommand(Ship *ship,CommandToDo *command,bool32 rowClear);

// call whenever removing command
void RemoveShipsFromCommand(CommandToDo *command);
// call whenever removing a ship from command (e.g. removing ship from command->selection)
void RemoveShipFromCommand(Ship *ship);

//returns cost of selSelected to hyperspace to distance
real32 hyperspaceCost(real32 distance,SelectCommand *selection);

void FillInCarrierMothershipInfo(struct Player *player,Ship **mothership,Ship *carrierX[]);

// tells ship to stay nearby protectThisShip - actual logic for attack if protectThisShip being attacked not in this routine
void protectShip(Ship *ship,Ship *protectThisShip,bool32 passiveAttacked);

/*=============================================================================
    Data:
=============================================================================*/

BOOL clGunIsTargetInCone(float *a1, float *a2);

void clLaunchShipSelection(CommandLayer *comlayer, SelectCommand *selectcom);

void clUpdateWeaponTrail(Ship *ship, Resource *resource, sdword commandCode, struct DockStaticPoint *dockPoint);
bool32 clGunConeCheckAndTrack(SpaceObjRotImpTargGuidance *ship, Resource *resource, sdword mode);

#endif
