// =============================================================================
//  MinelayerCorvette.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 11/5/1997 by khent
// =============================================================================

#ifndef ___MINELAYERCORVETTE_H
#define ___MINELAYERCORVETTE_H

#include "Attack.h"

#define MINE_DROP_ATTACK            0
#define MINE_DROP_FORMATION         1
#define MINE_SEARCH_AND_STOP        2
#define MINE_ATTACK_RUN             3
#define MINE_WALL_FORMATION         4

typedef struct
{
    AttackSideStepParameters sidestepParameters;
    udword NumMinesInSide;
    real32 MINE_STOP_FRICTION;
    real32 MineSpacing;
    real32 MineDropDistance;
    real32 DropStopRadiusSqr;
    real32 breakInAwayDist;
    real32 DropRange;
    real32 DropStopRange;
    real32 FlyAwayDist[NUM_CLASSES];
    real32 FlyAwayTolerance;
    real32 Break2SphereizeFreq;
    udword NumMinesInSideSqr;
    real32 MineClearDistanceSQR;
    real32 MineClearDistance;
    real32 forced_drop_damage_hi;
    real32 forced_drop_damage_lo;
    real32 gunReFireTime;
    real32 forced_drop_lifetime;
    real32 mineRegenerateTime;
} MinelayerCorvetteStatics;

typedef struct
{
    AttackSideStep attacksidestep;
    sdword MiningStatus;
    udword dropcount;
    vector aivec;
    real32 aispheretime;
    udword mineaistate;
    vector formation_position;
    vector formation_up;
    vector formation_right;
    vector formation_left;
    vector formation_down;
    vector formation_heading;
    sdword MineDropNumber;
    sdword MiningSideMax;
    sdword MiningSideCount;
    MineFormationInfo *mineforminfo;
    sdword formation_number_X;
    sdword formation_number_Y;
    real32 lasttimeRegeneratedMines;
} MinelayerCorvetteSpec;

/* Retail tMinelayerCorvette reuses the BeastSwarmer callback family. */
static_assert(sizeof(MinelayerCorvetteSpec) == sizeof(BeastSwarmerSpecifics), "MinelayerCorvetteSpec size mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, MiningStatus) == offsetof(BeastSwarmerSpecifics, miningState), "MinelayerCorvetteSpec MiningStatus offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, dropcount) == offsetof(BeastSwarmerSpecifics, dropcount), "MinelayerCorvetteSpec dropcount offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, aivec) == offsetof(BeastSwarmerSpecifics, attackVector), "MinelayerCorvetteSpec aivec offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, aispheretime) == offsetof(BeastSwarmerSpecifics, attackTimer), "MinelayerCorvetteSpec aispheretime offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, mineaistate) == offsetof(BeastSwarmerSpecifics, mineFormationActive), "MinelayerCorvetteSpec mineaistate offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_position) == offsetof(BeastSwarmerSpecifics, formationPosition), "MinelayerCorvetteSpec formation_position offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_up) == offsetof(BeastSwarmerSpecifics, formationUp), "MinelayerCorvetteSpec formation_up offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_right) == offsetof(BeastSwarmerSpecifics, formationRight), "MinelayerCorvetteSpec formation_right offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_left) == offsetof(BeastSwarmerSpecifics, formationLeft), "MinelayerCorvetteSpec formation_left offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_down) == offsetof(BeastSwarmerSpecifics, formationDown), "MinelayerCorvetteSpec formation_down offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_heading) == offsetof(BeastSwarmerSpecifics, formationHeading), "MinelayerCorvetteSpec formation_heading offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, MineDropNumber) == offsetof(BeastSwarmerSpecifics, mineDropNumber), "MinelayerCorvetteSpec MineDropNumber offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, MiningSideMax) == offsetof(BeastSwarmerSpecifics, miningSideMax), "MinelayerCorvetteSpec MiningSideMax offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, MiningSideCount) == offsetof(BeastSwarmerSpecifics, miningSideCount), "MinelayerCorvetteSpec MiningSideCount offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, mineforminfo) == offsetof(BeastSwarmerSpecifics, mineFormationInfo), "MinelayerCorvetteSpec mineforminfo offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_number_X) == offsetof(BeastSwarmerSpecifics, formationNumberX), "MinelayerCorvetteSpec formation_number_X offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, formation_number_Y) == offsetof(BeastSwarmerSpecifics, formationNumberY), "MinelayerCorvetteSpec formation_number_Y offset mismatch");
static_assert(offsetof(MinelayerCorvetteSpec, lasttimeRegeneratedMines) == offsetof(BeastSwarmerSpecifics, lastMineRegenerateTime), "MinelayerCorvetteSpec lasttimeRegeneratedMines offset mismatch");

extern CustShipHeader MinelayerCorvetteHeader;

void MinelayerCorvetteOrderChangedCleanUp(Ship *ship);
void univUpdateMineWallFormations();

#endif
