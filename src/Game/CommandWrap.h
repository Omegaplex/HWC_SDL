/*=============================================================================
    Name    : CommandWrap.h
    Purpose : Definitions for CommandWrap.c

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___COMMAND_WRAP_H
#define ___COMMNAD_WRAP_H

#include "types.h"
#include "commandlayer.h"

bool32 clCommandMessage(char *message, ...);
char *cmConsoleStartup(void);
sdword cmConsoleShutdown(void);

void clWrapMove(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove);
void clWrapMovePoint(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clWrapAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom);
void clWrapFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation);
void clWrapDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith);
sdword clWrapUpgrade(CommandLayer *comlayer,SelectCommand *selectcom,sdword upgradecom);
Ship *clWrapCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);
void clWrapBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator);

void clWrapDeterministicBuild(udword command,
                              CommandLayer* comlayer, sdword numShips,
                              ShipType shipType, ShipRace shipRace,
                              uword playerIndex, ShipPtr creator);

void clWrapCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource);
void clWrapProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom);
void clWrapSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets);
void clWrapSpecialCloak(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom, bool32 cloak);
void clWrapHalt(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapScuttle(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapAutoLaunch(udword OnOff,udword playerIndex);
void clWrapSetAlliance(udword AllianceType, uword curalliance, uword newalliance);
void clWrapLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom);
void clWrapSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype);
void clWrapSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapLinkSelection(CommandLayer *comlayer,SelectCommand *selectcom);
void clWrapSetUnleech(CommandLayer *comlayer, SelectCommand *selectcom);
void clWrapSetLeech(CommandLayer *comlayer, SelectCommand *selectcom, ubyte leechMode);
void clWrapSiegeCannon(CommandLayer *comlayer,SelectCommand *selectcom,vector *target);
void clWrapExternalConstruction(CommandLayer *comlayer, uword playerIndex, Ship *ship, void *context, real32 progress, sdword operation);
CommandToDo *clWrapExternalConstructionSelection(CommandLayer *comlayer, SelectCommand *selectcom, sdword operation, sdword arg1, sdword arg2, sdword arg3);
void clWrapSetShipDisabled(CommandLayer *comlayer, SelectCommand *selectcom);
void clWrapSetShipModuleState(CommandLayer *comlayer, SelectCommand *selectcom, ShipType moduleFrom, sdword moduleTo);
void clWrapSetBeastSystem(CommandLayer *comlayer, struct Ship *ship, udword system);
void clWrapRUTransfer(CommandLayer *comlayer,sdword fromIndex,sdword toIndex, sdword resourceUnits,ubyte flags);
void clWrapMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
void clWrapResearch(udword type, udword playernum, udword labnum, udword tech);
void clWrapLaunchShipSelection(CommandLayer *comlayer, SelectCommand *selectcom);
#endif
