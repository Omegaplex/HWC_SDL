#ifndef ___AISKIRMISH_H
#define ___AISKIRMISH_H

#include "types.h"
#include "spaceobj.h"
#include "MissionScriptSupport.h"

sdword cat_AICountShipClass(AISTeamEntry *team, Ship *ship);

void aisSetupTeams(void);
void aisAllianceUpdate(void);
void aisBuildModules(void);
Ship *aisGetTeamShipByFlatIndex(sdword flatIndex, AISTeamEntry *teamEntry);
void aisBuildShip(Player *player, sdword shipType, sdword buildCost);
void aisFleetUpdate(void);
bool32 aisShipHasActiveCommand(Ship *ship);
void aisCollectResources(void);
void aisMoveProcessor(void);
void aisAttackUpdate(void);
AISTeamSelection *aisFindClosestTeamSelectionToPoint(sdword teamType, vector *point);
void aisManageTeam(AISTeamEntry *teams, sdword minimum, sdword maximum, ubyte randomizeFormation, ubyte startExternalConstruction, ubyte allowHyperspace, ubyte enableKamikaze, sdword targetTeamCount, sdword *targetTeams);
void aisMoveMothership(void);
void aisMoveHarvesterGroups(void);
void aisSentinelUpdate(void);
void aisLeechUpdate(void);
void aisInitUpdateTimes(void);
void aisFleetCommand(void);
void aisParityLogChannel(const char *channel, const char *format, ...);
void aisParityLogAIRandomState(const char *source, udword x, udword y, udword z, udword c, udword n);
void aisParityLogAISeedOverride(const char *source, udword seed, udword x, udword y, udword z, udword c, udword n);

#endif
