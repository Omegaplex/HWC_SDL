/*=============================================================================
    Name    : CommandLayer.c
    Purpose : This is the ship command layer of homeworld.  All commands to
              ship go through this layer, whether the commands originate from
              across a network, a computer player, a human player, etc.

    Created 7/5/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <intrin.h>

#include "types.h"
#include "fastmath.h"
#include "debug.h"
#include "memory.h"
#include "formation.h"
#include "commandlayer.h"
#include "aiship.h"
#include "aitrack.h"
#include "statscript.h"
#include "universe.h"
#include "univupdate.h"
#include "etg.h"
#include "CommandWrap.h"
#include "soundevent.h"
#include "select.h"
#include "collision.h"
#include "flightman.h"
#include "attack.h"
#include "aiplayer.h"
#include "ACV.h"
#include "GenericInterceptor.h"
#include "DDDFrigate.h"
#include "SalCapCorvette.h"
#include "MinelayerCorvette.h"
#include "GravWellGenerator.h"

#include "CloakGenerator.h"
#include "ResearchShip.h"
#include "researchAPI.h"
#include "strings.h"
#include "tactics.h"

#include "singleplayer.h"
#include "Ship.h"
#include "CrashLog.h"
#include "repaircorvette.h"
#include "genericdefender.h"

#include "alliance.h"
#include "probe.h"
#include "heavycorvette.h"
#include "HiveFrigate.h"
#include "Mimic.h"
#include "madLinkIn.h"
#include "proximitysensor.h"
#include "consmgr.h"
#include "rescollect.h"
#include "tutor.h"
#include "randy.h"
#include "battle.h"

#include "ping.h"
#include "netcheck.h"
#include "commandnetwork.h"
#include "dock.h"
#include "Clamp.h"
#include "BigGun.h"

#include "Dreadnought.h"
#include "DefaultShip.h"
#include "Globals.h"
#include "Sensors.h"

sdword shipGetSlotStatus(Ship *ship, sdword slotIndex);
sdword fogObjectVisibleToPlayer(Player *player, SpaceObj *obj, sdword treatMultiplayerAsVisible);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
extern real32 DELTA_FORMATION_PADDING_SCALE;

#ifdef gshaw
#ifndef HW_Release
//#define DEBUG_FORMATIONS

//#define DEBUG_LAUNCHSHIP

//#define DEBUG_COLLECTRESOURCES

//#define DEBUG_COMMANDLAYER

#endif

#endif

/*=============================================================================
    Private Functions:
=============================================================================*/

static void FormationCleanup(struct CommandToDo *formationtodo);

static ubyte commandLayerAttackStateForShip(const Ship *ship)
{
    if (ship == NULL)
    {
        return 0;
    }

    if (ShipTypeIsHomeworldHeavyInterceptorType(ship->shiptype))
    {
        return ship->rcstate2;
    }

    return ship->aistateattack;
}

char *RUTransferReceivedFmt = NULL;
static void MoveCleanup(struct CommandToDo *movetodo);
static void MpHyperspaceCleanup(struct CommandToDo *movetodo);
static void clClearShipCommandState(CommandToDo *command);
static Node *clTraverseCommandList(Node **head);
static bool32 clShipCanRetreatForRepairs(Ship *ship, bool32 considerSingleShip);
static bool32 clShipShouldRetreatForRepairs(Ship *ship);
static void clFilterShipsForOrders(SelectCommand *selection);
static void clAttackTargetsForSelection(LinkedList *todolist, SelectCommand *selectcom, AttackCommand *attackcom);
static void ChangeOrderToAttackTargets(CommandToDo *command, AttackCommand *attackcom);
static void SetupExtraAttackInfo(SelectCommand *selection, AttackCommand *attack, bool32 findwingmen);
void formationInit(CommandToDo *command, TypeOfFormation formationType, ShipPtr leader, bool32 resetLeader);
void dockWorkerClampFunc(Ship *ship);
static void RemoveShipFromMoving(Ship *ship);
//static void RemoveShipFromAttacking(Ship *ship);
//static void RemoveShipReferencesFromExtraAttackInfo(Ship *shiptoremove,CommandToDo *todo);
//static void RemoveAttackTargetFromExtraAttackInfo(SpaceObjRotImpTarg *targettoremove,CommandToDo *todo);
static SpaceObjRotImpTarg *AttackClosestOpponent(Ship *ship,AttackCommand *attack);
static void AttackCleanup(struct CommandToDo *attacktodo);
static AttackTargets *allocateMultipleAttackTargets(Ship *ship);
static void pickMultipleAttackTargets(Ship *ship,AttackCommand *attack);
static bool32 clIsSelectionNearProtectTargets(CommandToDo *command);
static bool32 clIsCrystal(Ship *ship);
static void clClearLinkState(Ship *ship);
static void clClearSelectionLinkState(const SelectCommand *selection);
static bool32 processLinkToDo(CommandToDo *linktodo);
void ChangeOrderToHalt(CommandToDo *alreadycommand);
bool32 processMoveToDo(CommandToDo *movetodo,bool32 passiveAttacked);
bool32 processAttackToDoInFormation(CommandToDo *attacktodo);
bool32 processAttackMoveToDo(CommandToDo *attacktodo,bool32 passiveAttacked,sdword shipIndex,bool32 useTravelVelocity);
bool32 processMpHyperspaceingToDo(CommandToDo *movetodo);
void clSetShipMoveToPositions(SelectCommand *selection,vector from,vector to);
void clSetAttribute(sdword ship, sdword value, sdword attribute);
void telldir(void);
bool32 processMoveLeaderToDo(CommandToDo *movetodo,bool32 passiveAttacked);
bool32 targetIsntNearShip(Ship *ship,SpaceObjRotImpTarg *target);
void formGroups(SelectCommand *selection,SelectCommand *selectionDefenders,SelectCommand *selectionCorvettes,SelectCommand *selectionCapitalShips,SelectCommand *selectionInterceptors);
bool32 delegateCommand(CommandToDo *attacktodo,sdword group,sdword doform, SelectCommand *globalselection);
static void clMoveTheseInternal(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove,bool32 skipEquivalentSearch);
void clMoveThese(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove);

// Cataclysm-specific: per-ship byte fields cleared when finalizing a COMMAND_LINK or
// COMMAND_LINKED_LAUNCH command's selection (retail clLinkSelection @0x44dbc0 and
// clLaunchShipSelection @0x44d7a0 both clear these same three byte offsets on each ship).
#define CATA_LINK_CLEAR0_OFFSET        0x1C0
#define CATA_LINK_CLEAR1_OFFSET        0x350
#define CATA_LINK_CLEAR2_OFFSET        0x355
void clMpHyperspaceThese(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to);
bool32 isSelectionExclusivlyStrikeCraft(SelectCommand *selection);
bool32 isSelectionExclusivlyStrikeCraftorNoMoveAndAttackShips(SelectCommand *selection);
void removeShipFromMpHyperspaceing(Ship *ship);
bool32 acolyteCanLink(Ship *ship);
sdword acolyteGetLinkState(Ship *ship);
void posSetShipPosition(Ship *ship, vector *newPosition);
void selSelectionAddSingleShipNoCheck(MaxSelection *dest, Ship *ship);
void ioUpdateShipTotals(void);

static void clClearMoveCompletionChatter(Ship *ship)
{
    if (ship->unk_1D0)
    {
        battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_RetreatSuccessful, SOUND_EVENT_DEFAULT, NULL);
        ship->unk_1D0 = 0;
    }
}

static void clClearMoveCompletionChatterSelection(SelectCommand *selection)
{
    sdword i;
    bool32 chatterReported = FALSE;

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];

        if (ship->unk_1D0)
        {
            if (!chatterReported)
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Grp_RetreatSuccessful, SOUND_EVENT_DEFAULT, NULL);
                chatterReported = TRUE;
            }
            ship->unk_1D0 = 0;
        }
    }
}

#define COMMAND_SIEGECANNON_TARGET_OFFSET 0x2DC
static_assert(sizeof(CommandToDo) >= COMMAND_SIEGECANNON_TARGET_OFFSET + sizeof(vector), "CommandToDo too small for siege cannon target");

#define COMMAND_EXTERNALCONSTRUCTION_OPERATION_OFFSET 0x2E8
#define COMMAND_EXTERNALCONSTRUCTION_SHIPTYPE_OFFSET  0x2EC
#define COMMAND_EXTERNALCONSTRUCTION_PARAM2_OFFSET    0x2F0
#define COMMAND_EXTERNALCONSTRUCTION_PARAM3_OFFSET    0x2F4
static_assert(sizeof(CommandToDo) >= COMMAND_EXTERNALCONSTRUCTION_PARAM3_OFFSET + sizeof(real32), "CommandToDo too small for external construction state");
#define COMMAND_RESEARCH_PLAYER_INDEX_OFFSET          0x2F8
#define COMMAND_RESEARCH_LABNUM_OFFSET                0x2FC
#define COMMAND_RESEARCH_TECH_OFFSET                  0x300
#define COMMAND_RESEARCH_TYPE_OFFSET                  0x304
static_assert(sizeof(CommandToDo) >= COMMAND_RESEARCH_TYPE_OFFSET + sizeof(udword), "CommandToDo too small for research state");

#define ATTACKMOVE_PATH_ONCE         0
#define ATTACKMOVE_PATH_LOOP         1
#define ATTACKMOVE_PATH_PING_FORWARD 2
#define ATTACKMOVE_PATH_PING_BACK    3
#define ATTACKMOVE_MAX_POINTS        21

typedef struct
{
    sdword pathState;
    sdword pointCount;
    vector points[ATTACKMOVE_MAX_POINTS];
    sdword currentPointIndex;
} AttackMovePathState;

static_assert(sizeof(AttackMovePathState) == 264, "AttackMovePathState must match the retail CommandToDo move overlay");
static_assert(sizeof(real32) == sizeof(sdword), "Attack-move packing assumes 32-bit sdword-sized real32 values");
static_assert(sizeof(real32) == sizeof(void *), "Attack-move packing assumes 32-bit pointer-sized real32 values");

typedef union
{
    real32 real;
    sdword signedValue;
    void *pointer;
} AttackMovePackedValue;

static sdword clUnpackAttackMoveSdword(real32 packed)
{
    AttackMovePackedValue value;

    value.real = packed;
    return value.signedValue;
}

static vector *clUnpackAttackMovePoints(real32 packed)
{
    AttackMovePackedValue value;

    value.real = packed;
    return (vector *)value.pointer;
}

static real32 clPackAttackMoveSdword(sdword value)
{
    AttackMovePackedValue packed;

    packed.signedValue = value;
    return packed.real;
}

static real32 clPackAttackMovePoints(vector *points)
{
    AttackMovePackedValue packed;

    packed.pointer = points;
    return packed.real;
}

static MoveCommandPacked clBuildAttackMoveCommand(sdword pathState, vector initialPoint, sdword pointCount, vector *points)
{
    MoveCommandPacked packedMove;

    packedMove.from.x = clPackAttackMoveSdword(pathState);
    packedMove.from.y = initialPoint.x;
    packedMove.from.z = initialPoint.y;
    packedMove.to.x = initialPoint.z;
    packedMove.to.y = clPackAttackMoveSdword(pointCount);
    packedMove.to.z = clPackAttackMovePoints(points);
    return packedMove;
}

MoveCommandPacked clBuildMovePathCommand(sdword pathState, vector initialPoint, sdword pointCount, vector *points)
{
    return clBuildAttackMoveCommand(pathState, initialPoint, pointCount, points);
}

static void clUnpackMovePathCommand(MoveCommandPacked packedMove, sdword *pathState, vector *initialPoint, sdword *pointCount, vector **points);
#define NUM_STACK_MOVECALCS 100
static void CalculateMoveOffsets(SelectCommand *selection, vector from, vector to, vector *offsets);
void CalculateMoveToPoints(SelectCommand *selection,vector from,vector to);

static MoveCommandPacked clBuildShipSpecificMovePathCommand(SelectCommand *selection, Ship *ship, MoveCommandPacked packedMove, vector *adjustedPoints)
{
    sdword pathState;
    vector initialPoint;
    sdword pointCount;
    vector *points;
    sdword shipIndex;
    vector adjustedInitialPoint;
    vector offset;
    vector stackOffsets[NUM_STACK_MOVECALCS];
    vector *moveOffsets;
    bool32 moveOffsetsAllocated;
    sdword i;

    clUnpackMovePathCommand(packedMove, &pathState, &initialPoint, &pointCount, &points);
    dbgAssert(pointCount > 0);
    dbgAssert(pointCount <= ATTACKMOVE_MAX_POINTS);

    if (selection->numShips > NUM_STACK_MOVECALCS)
    {
        moveOffsetsAllocated = TRUE;
        moveOffsets = memAlloc(sizeof(vector) * selection->numShips, "moveoffsets", 0);
    }
    else
    {
        moveOffsetsAllocated = FALSE;
        moveOffsets = stackOffsets;
    }

    CalculateMoveOffsets(selection, initialPoint, points[0], moveOffsets);

    shipIndex = -1;
    for (i = 0; i < selection->numShips; i++)
    {
        if (selection->ShipPtr[i] == ship)
        {
            shipIndex = i;
            break;
        }
    }
    dbgAssert(shipIndex != -1);

    offset = moveOffsets[shipIndex];
    vecAdd(adjustedInitialPoint, initialPoint, offset);

    for (i = 0; i < pointCount; i++)
    {
        vecAdd(adjustedPoints[i], points[i], offset);
    }

    if (moveOffsetsAllocated)
    {
        memFree(moveOffsets);
    }

    return clBuildMovePathCommand(pathState, adjustedInitialPoint, pointCount, adjustedPoints);
}

MoveCommandPacked clBuildMovePointCommand(vector initialPoint, vector *destination)
{
    return clBuildAttackMoveCommand(ATTACKMOVE_PATH_ONCE, initialPoint, 1, destination);
}

static void clUnpackMovePathCommand(MoveCommandPacked packedMove, sdword *pathState, vector *initialPoint, sdword *pointCount, vector **points)
{
    *pathState = clUnpackAttackMoveSdword(packedMove.from.x);
    vecSet(*initialPoint, packedMove.from.y, packedMove.from.z, packedMove.to.x);
    *pointCount = clUnpackAttackMoveSdword(packedMove.to.y);
    *points = clUnpackAttackMovePoints(packedMove.to.z);
}

static void clUnpackMovePointCommand(MoveCommandPacked packedMove, vector *initialPoint, vector *destination)
{
    sdword pathState;
    sdword pointCount;
    vector *points;

    clUnpackMovePathCommand(packedMove, &pathState, initialPoint, &pointCount, &points);
    dbgAssert(pointCount > 0);
    *destination = points[0];
}

static AttackMovePathState *clMovePathState(CommandToDo *command)
{
    return (AttackMovePathState *)&command->move;
}

static const AttackMovePathState *clMovePathStateConst(const CommandToDo *command)
{
    return (const AttackMovePathState *)&command->move;
}

static void clStoreMovePath(CommandToDo *command, MoveCommandPacked packedMove)
{
    AttackMovePathState *movePath = clMovePathState(command);
    sdword pointCount;
    sdword pathState;
    vector initialPoint;
    vector *points;

    clUnpackMovePathCommand(packedMove, &pathState, &initialPoint, &pointCount, &points);
    dbgAssert(pointCount > 0);
    dbgAssert(pointCount <= ATTACKMOVE_MAX_POINTS);

    movePath->pathState = pathState;
    movePath->pointCount = pointCount;
    memcpy(movePath->points, points, pointCount * sizeof(vector));
    movePath->currentPointIndex = 0;
}

void clGetMoveDestination(CommandToDo *command, vector *destination)
{
    const AttackMovePathState *movePath = clMovePathStateConst(command);

    dbgAssert(movePath->pointCount > 0);
    dbgAssert(movePath->currentPointIndex >= 0);
    dbgAssert(movePath->currentPointIndex < movePath->pointCount);
    *destination = movePath->points[movePath->currentPointIndex];
}

static void clResumeAttackMoveAsMove(CommandToDo *command)
{
    Ship *leader = command->selection->ShipPtr[0];
    MoveCommandPacked packedMove;
    vector pathPoints[1];

    pathPoints[0] = leader->moveTo;
    packedMove = clBuildMovePathCommand(ATTACKMOVE_PATH_ONCE, leader->moveFrom, 1, pathPoints);
    bitSet(leader->specialFlags1C8,CATA_RUNTIME_ATTACKMOVECANCEL);
    ChangeOrderToMove(command, packedMove);
}

#define CL_ATTRIBUTE_ARRAY_OFFSET 0x1DC
#define CL_ATTRIBUTE_COUNT        13
#define CL_ATTRIBUTE_ANYSET_OFFSET 0x210
#define CL_UPGRADE_ACTIVE_OFFSET  0x598
#define CL_UPGRADE_START_OFFSET   0x59C
#define CL_UPGRADE_END_OFFSET     0x5A0

static_assert(sizeof(Ship) >= CL_UPGRADE_END_OFFSET + sizeof(real32), "Ship too small for upgrade timing state");

static vector *clSiegeCannonTarget(CommandToDo *command)
{
    return (vector *)((ubyte *)command + COMMAND_SIEGECANNON_TARGET_OFFSET);
}

static sdword *clExternalConstructionOperation(CommandToDo *command)
{
    return (sdword *)((ubyte *)command + COMMAND_EXTERNALCONSTRUCTION_OPERATION_OFFSET);
}

static ShipType *clExternalConstructionShipType(CommandToDo *command)
{
    return (ShipType *)((ubyte *)command + COMMAND_EXTERNALCONSTRUCTION_SHIPTYPE_OFFSET);
}

static real32 *clExternalConstructionParam2(CommandToDo *command)
{
    return (real32 *)((ubyte *)command + COMMAND_EXTERNALCONSTRUCTION_PARAM2_OFFSET);
}

static real32 *clExternalConstructionParam3(CommandToDo *command)
{
    return (real32 *)((ubyte *)command + COMMAND_EXTERNALCONSTRUCTION_PARAM3_OFFSET);
}

static uword *clResearchPlayerIndex(CommandToDo *command)
{
    return (uword *)((ubyte *)command + COMMAND_RESEARCH_PLAYER_INDEX_OFFSET);
}

static udword *clResearchLabnum(CommandToDo *command)
{
    return (udword *)((ubyte *)command + COMMAND_RESEARCH_LABNUM_OFFSET);
}

static TechnologyType *clResearchTech(CommandToDo *command)
{
    return (TechnologyType *)((ubyte *)command + COMMAND_RESEARCH_TECH_OFFSET);
}

static udword *clResearchType(CommandToDo *command)
{
    return (udword *)((ubyte *)command + COMMAND_RESEARCH_TYPE_OFFSET);
}

// Decompiled from cataclysm.exe @ 0x5C23B0
void clDeleteCommandEffect(CommandToDo *command)
{
    udword *commandEffect = (udword *)command->formation.formationSpecificInfo;

    if ((*(ubyte *)(commandEffect[8] + 104) & 1) != 0)
    {
        *(udword *)(commandEffect[72] + 8) = commandEffect[61];
    }
    else
    {
        etgEffectDelete((Effect *)commandEffect);
        univRemoveObjFromRenderList((SpaceObj *)commandEffect);
        listDeleteNode((Node *)commandEffect);
    }
    command->formation.formationSpecificInfo = NULL;
}

void clSetAttribute(sdword ship, sdword value, sdword attribute)
{
    Ship *shipPtr = (Ship *)ship;
    sdword i;
    sdword anySet = 0;

    *(sdword *)((ubyte *)shipPtr + CL_ATTRIBUTE_ARRAY_OFFSET + sizeof(sdword) * attribute) = value;

    for (i = 0; i < CL_ATTRIBUTE_COUNT; i++)
    {
        if (*(sdword *)((ubyte *)shipPtr + CL_ATTRIBUTE_ARRAY_OFFSET + sizeof(sdword) * i) != 0)
        {
            anySet = 1;
            break;
        }
    }

    *(sdword *)((ubyte *)shipPtr + CL_ATTRIBUTE_ANYSET_OFFSET) = anySet;

}

/*=============================================================================
    Tweakables:
=============================================================================*/

#define DISTMINRATIO        1.0f
real32 DISTMAXRATIO = 5.0f;
real32 SHRINKMIN = 0.1f;
real32 SHRINKMAX = 1.0f;

real32 MAX_DIST_JOIN_GROUP_SQR = (2000.0f*2000.0f);
real32 PROTECT_GET_THIS_CLOSE_CAP = 2000.0f;
real32 PROTECT_GET_THIS_CLOSE_STRIKE = 2000.0f;
real32 PROTECT_STILL_VELOCITY = 20.0f;

real32 WINGMAN_JOIN_MAX_DIST     = (200.0f*200.0f);

real32 FIGHTER_MAX_THREAT_DIST[NUM_TACTICS_TYPES] = { (2000.0f*2000.0f),(2000.0f*2000.0f),(2000.0f*2000.0f) };

udword FIGHTER_PAIRUP_PROB       = 255;

udword FIGHTER_PAIR_CHECK_RATE[NUM_TACTICS_TYPES] = { 7,7,7 };
udword FIGHTER_SINGLE_CHECK_RATE[NUM_TACTICS_TYPES] = { 31,31,31 };

udword multipleTargetsRetargetMask = 31;

bool8 HYPERSPACE = FALSE;

real32 GLOBAL_SHIP_HEALTH_MODIFIER = 1.0f;

real32 RESCONTROLLER_FOLLOW_BEHIND = 1.5f;

extern sdword FIGHTER_BREAK_ANGLE_MIN;
extern sdword FIGHTER_BREAK_ANGLE_MAX;

extern sdword FIGHTER_BREAK_VERTICAL_ANGLE_MIN;
extern sdword FIGHTER_BREAK_VERTICAL_ANGLE_MAX;

extern bool32 BombersUseBombingRun;

extern real32 FLY_INTO_WORLD_PERCENT_DIST;

scriptEntry CommandLayerTweaks[] =
{
    makeEntry(MAX_DIST_JOIN_GROUP_SQR,scriptSetReal32SqrCB),
    makeEntry(PROTECT_GET_THIS_CLOSE_CAP,scriptSetReal32CB),
    makeEntry(PROTECT_GET_THIS_CLOSE_STRIKE,scriptSetReal32CB),
    makeEntry(PROTECT_STILL_VELOCITY,scriptSetReal32CB),
    makeEntry(HYPERSPACE,scriptSetBool8),
    makeEntry(WINGMAN_JOIN_MAX_DIST,scriptSetReal32SqrCB),
    makeEntry(FIGHTER_PAIRUP_PROB,scriptSetUdwordCB),
    makeEntry(FIGHTER_PAIR_CHECK_RATE[Evasive],scriptSetUdwordCB),
    makeEntry(FIGHTER_PAIR_CHECK_RATE[Neutral],scriptSetUdwordCB),
    makeEntry(FIGHTER_PAIR_CHECK_RATE[Aggressive],scriptSetUdwordCB),
    makeEntry(FIGHTER_SINGLE_CHECK_RATE[Evasive],scriptSetUdwordCB),
    makeEntry(FIGHTER_SINGLE_CHECK_RATE[Neutral],scriptSetUdwordCB),
    makeEntry(FIGHTER_SINGLE_CHECK_RATE[Aggressive],scriptSetUdwordCB),
    makeEntry(FIGHTER_MAX_THREAT_DIST[Evasive],scriptSetReal32SqrCB),
    makeEntry(FIGHTER_MAX_THREAT_DIST[Neutral],scriptSetReal32SqrCB),
    makeEntry(FIGHTER_MAX_THREAT_DIST[Aggressive],scriptSetReal32SqrCB),
    makeEntry(multipleTargetsRetargetMask,scriptSetUdwordCB),
    makeEntry(GLOBAL_SHIP_HEALTH_MODIFIER,scriptSetReal32CB),
    makeEntry(FIGHTER_BREAK_ANGLE_MIN,scriptSetSdwordCB),
    makeEntry(FIGHTER_BREAK_ANGLE_MAX,scriptSetSdwordCB),
    makeEntry(FIGHTER_BREAK_VERTICAL_ANGLE_MIN,scriptSetSdwordCB),
    makeEntry(FIGHTER_BREAK_VERTICAL_ANGLE_MAX,scriptSetSdwordCB),
    makeEntry(BombersUseBombingRun,scriptSetBool),
    makeEntry(DISTMAXRATIO,scriptSetReal32CB),
    makeEntry(SHRINKMIN,scriptSetReal32CB),
    makeEntry(SHRINKMAX,scriptSetReal32CB),
    makeEntry(FLY_INTO_WORLD_PERCENT_DIST,scriptSetReal32CB),
    makeEntry(RESCONTROLLER_FOLLOW_BEHIND,scriptSetReal32CB),
    endEntry
};

/*-----------------------------------------------------------------------------
    Name        : RemoveShipsFromCommand
    Description : removes ships from command
    Inputs      : command
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipsFromCommand(CommandToDo *command)
{
    sdword i;
    SelectCommand *selection = command->selection;

    for (i=0;i<selection->numShips;i++)
    {
        selection->ShipPtr[i]->command = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromCommand
    Description : removes a ship from command
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromCommand(Ship *ship)
{
    ship->command = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : PrepareShipsForCommand
    Description : prepares ships for executing a new command
    Inputs      : command, rowClear - flag indicating if rightOfWay should be removed
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PrepareShipsForCommand(CommandToDo *command,bool32 rowClear)
{
    sdword i;
    SelectCommand *selection = command->selection;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        ship->command = command;
        ship->unk_1D0 = 0;
        if ((rowClear) && (ship->specialFlags1C8 & CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY))
        {
            rowRemoveShipFromGettingOutOfWay(ship);
        }
        if (ship->shiptype == sLeech && command->ordertype.order == COMMAND_SPECIAL)
        {
            ((LeechSpecifics *)ship->ShipSpecifics)->state = 14;
        }
    }
    tacticsMakeShipsNotLookForOtherShips(command->selection);
}

/*-----------------------------------------------------------------------------
    Name        : PrepareOneShipForCommand
    Description : prepares ship for executing a new command
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PrepareOneShipForCommand(Ship *ship,CommandToDo *command,bool32 rowClear)
{
    ship->command = command;
    ship->unk_1D0 = 0;
    if ((rowClear) && (ship->specialFlags1C8 & CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY))
    {
        rowRemoveShipFromGettingOutOfWay(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitShipsForAI
    Description : Initializes selection ships AI state variables
    Inputs      : fresh (if we're starting a new command, this should be TRUE
                  otherwise, it should be false
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitShipsForAI(SelectCommand *selection,bool32 fresh)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        ship->aistate = 0;
        ship->aistateattack = 0;
        if(fresh)
            ship->aistatecommand = 0;
    }
}

void InitShipsForAIDontResetAttack(SelectCommand *selection,bool32 fresh)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        ship->aistate = 0;
        if(fresh)
            ship->aistatecommand = 0;
    }
}

static bool32 clIsSelectionNearProtectTargets(CommandToDo *command)
{
    vector defendAverage;
    vector protectAverage;
    vector delta;
    real32 inverseCount;
    real32 distanceSquared;
    sdword i;

    dbgAssert(command->protect != NULL);

    vecZeroVector(defendAverage);
    for (i = 0; i < command->selection->numShips; i++)
    {
        vecAddTo(defendAverage,command->selection->ShipPtr[i]->posinfo.position);
    }
    inverseCount = 1.0f / (real32)command->selection->numShips;
    vecScalarMultiply(defendAverage,defendAverage,inverseCount);

    vecZeroVector(protectAverage);
    for (i = 0; i < command->protect->numShips; i++)
    {
        vecAddTo(protectAverage,command->protect->ShipPtr[i]->posinfo.position);
    }
    inverseCount = 1.0f / (real32)command->protect->numShips;
    vecScalarMultiply(protectAverage,protectAverage,inverseCount);

    vecSub(delta,protectAverage,defendAverage);
    distanceSquared = vecMagnitudeSquared(delta);
    return (distanceSquared <= tacticsInfo.tacticsGuardReturnToGuardingDistanceSqr);
}

static bool32 clIsCrystal(Ship *ship)
{
    return IsCrystal(ship);
}

/*-----------------------------------------------------------------------------
    Name        : getRangeToClosestTarget
    Description : REturns the square of the range from command selection to its closest target
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
real32 getRangeToClosestTarget(CommandToDo *command)
{
    vector commandPosition;
    real32 tempSqr,minDistanceSqr = REALlyBig;
    vector distvec;
    sdword i;

    dbgAssert(command->selection->numShips > 0);
    dbgAssert(command->ordertype.order == COMMAND_ATTACK);

    commandPosition = command->selection->ShipPtr[0]->posinfo.position;

    for(i=0;i<command->attack->numTargets;i++)
    {
        vecSub(distvec,commandPosition,command->attack->TargetPtr[0]->posinfo.position);
        tempSqr = vecMagnitudeSquared(distvec);
        if(tempSqr < minDistanceSqr)
        {
            minDistanceSqr = tempSqr;
        }
    }
    return minDistanceSqr;
}

/*-----------------------------------------------------------------------------
    Name        : InitShipAI
    Description : Initializes ship's AI state variables
    Inputs      : if fresh is TRUE, aicommand will be set (should only be true if
                  this is called from a new command generation function
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitShipAI(Ship *ship,bool32 fresh)
{
    *(ubyte *)&ship->specialFlags = 0;
    ship->aistateattack = 0;
    if(fresh)
        ship->aistatecommand = 0;
}

/*-----------------------------------------------------------------------------
    Name        : protectShip
    Description : ship "protects" or flys nearby protectThisShip
    Inputs      : ship, protectThisShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define COLLECTOR_STEADY_DIST   3000.0f
void protectShip(Ship *ship,Ship *protectThisShip,bool32 passiveAttacked)
{
    vector diff;
    real32 dist;
    real32 desiredrange;
    real32 shipSize = ((ShipStaticInfo *)(ship->staticinfo))->staticheader.staticCollInfo.collspheresize;
    real32 protectThisShipSize = ((ShipStaticInfo *)(protectThisShip->staticinfo))->staticheader.staticCollInfo.collspheresize;
    bool32 dontrotate;
    vector protectPoint = protectThisShip->posinfo.position;
    vector heading;

    if (protectThisShipSize < shipSize)
    {
        protectThisShipSize = shipSize;     // if guarding a smaller ship, give more room
    }

    if(isCapitalShip(ship) || ShipTypeIsGravWellGeneratorType(ship->shiptype) || ShipTypeIsCloakGeneratorType(ship->shiptype))
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_CAP + shipSize + protectThisShipSize;
    }
    else
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_STRIKE + shipSize + protectThisShipSize;
    }

    // Let's have some ships like the ResourceController follow behind the ship so we don't accidentally block them.
    if (ShipTypeIsProcessorType(ship->shiptype))
    {
        real32 followbehind = desiredrange * RESCONTROLLER_FOLLOW_BEHIND;      // follow behind 1.5 radii
        matGetVectFromMatrixCol3(heading,protectThisShip->rotinfo.coordsys);
        vecSubFromScalarMultiply(protectPoint,heading,followbehind);
    }

    vecSub(diff,ship->posinfo.position,protectPoint);
    dist = getVectDistSloppy(diff);

//    ship->shipidle = FALSE;

    dontrotate = ((passiveAttacked) & (bool32)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate);

    if (ShipTypeIsDefenseFieldFrigateType(ship->shiptype))
    {
        DDDFrigateMakeSureItCanGuard(ship);
    }

    if (protectThisShip->dockvars.dockship == ship)     // if docking with me
    {
        if(MoveReachedDestinationVariable(ship,&protectThisShip->posinfo.position,COLLECTOR_STEADY_DIST))
        {
            //within range of steadyness
            goto juststeady;
        }
    }

    if (dist < desiredrange)
    {
        if ((ship->specialFlags1C8 & CATA_RUNTIME_CIRCULAR_GUARD) ||
            (getVectDistSloppy(protectThisShip->posinfo.velocity) < PROTECT_STILL_VELOCITY) )
        {
juststeady:
            if (dontrotate)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
            else
            {
                aitrackSteadyShip(ship);
            }
        }
        else
        {
            // just maintain velocity
            if (dontrotate)
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShipDriftOnly(ship);
                }
            }
            else
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShip(ship);
                }
            }
        }
    }
    else
    {
        // fly to protectThisShip
        if (dontrotate)
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&protectPoint,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                   0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
        }
        else
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&protectPoint,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                0.0f,&protectThisShip->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShip(ship);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : protectShipsAvg
    Description : ship "protects" or flys nearby median position of protectships
    Inputs      : ship, protectThisShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void protectShipsAvg(Ship *ship,ProtectCommand *protectShips,bool32 passiveAttacked)
{
    vector diff;
    real32 dist;
    real32 desiredrange;
    real32 shipSize = ((ShipStaticInfo *)(ship->staticinfo))->staticheader.staticCollInfo.collspheresize;
    bool32 dontrotate;
    sdword numShips = protectShips->numShips;
    vector temp,guardposition;
    sdword i;
    vector protectShipsAvgVel;
    real32 realtemp;
    real32 minx,maxx,miny,maxy,minz,maxz;
    bool32 anyshipdockingwithme;

    dbgAssert(numShips > 1);

    temp = protectShips->ShipPtr[0]->posinfo.position;
    minx = maxx = temp.x;
    miny = maxy = temp.y;
    minz = maxz = temp.z;

    protectShipsAvgVel = protectShips->ShipPtr[0]->posinfo.velocity;

    for (i=1;i<numShips;i++)
    {
        temp = protectShips->ShipPtr[i]->posinfo.position;
        vecAddTo(protectShipsAvgVel,protectShips->ShipPtr[i]->posinfo.velocity);

        if (temp.x < minx) minx = temp.x;
        else if (temp.x > maxx) maxx = temp.x;

        if (temp.y < miny) miny = temp.y;
        else if (temp.y > maxy) maxy = temp.y;

        if (temp.z < minz) minz = temp.z;
        else if (temp.z > maxz) maxz = temp.z;
    }

    guardposition.x = (minx + maxx) * 0.5f;
    guardposition.y = (miny + maxy) * 0.5f;
    guardposition.z = (minz + maxz) * 0.5f;

    vecDivideByScalar(protectShipsAvgVel,(real32)numShips,realtemp);

    // we now have the optimum guardposition and protectShipsAvgVel

    vecSub(diff,ship->posinfo.position,guardposition);
    dist = getVectDistSloppy(diff);

    if(isCapitalShip(ship) || ShipTypeIsGravWellGeneratorType(ship->shiptype) || ShipTypeIsCloakGeneratorType(ship->shiptype))
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_CAP + shipSize;
    }
    else
    {
        desiredrange = PROTECT_GET_THIS_CLOSE_STRIKE + shipSize;
    }

//    ship->shipidle = FALSE;

    dontrotate = ((passiveAttacked) & (bool32)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate);

    if (ShipTypeIsDefenseFieldFrigateType(ship->shiptype))
    {
        DDDFrigateMakeSureItCanGuard(ship);
    }

    anyshipdockingwithme = FALSE;

    if (ship->staticinfo->canReceiveSomething)
    {
        for (i=0;i<numShips;i++)
        {
            if (protectShips->ShipPtr[i]->dockvars.dockship == ship)
            {
                anyshipdockingwithme = TRUE;
                break;
            }
        }
    }

    if (anyshipdockingwithme)     // if docking with me
    {
        goto juststeady;
    }

    if (dist < desiredrange)
    {
        if ((ship->specialFlags1C8 & CATA_RUNTIME_CIRCULAR_GUARD) ||
            (getVectDistSloppy(protectShipsAvgVel) < PROTECT_STILL_VELOCITY))
        {
juststeady:
            if (dontrotate)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
            else
            {
                aitrackSteadyShip(ship);
            }
        }
        else
        {
            // just maintain velocity
            if (dontrotate)
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShipDriftOnly(ship);
                }
            }
            else
            {
                if (aishipFlyToPointAvoidingObjsWithVel(ship,NULL,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                        0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    aitrackSteadyShip(ship);
                }
            }
        }
    }
    else
    {
        // fly to protectThisShip
        if (dontrotate)
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&guardposition,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                                                   0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShipDriftOnly(ship);
            }
        }
        else
        {
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&guardposition,AISHIP_PointInDirectionFlying|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured,
                0.0f,&protectShipsAvgVel) & AISHIP_FLY_OBJECT_IN_WAY)
            {
                aitrackSteadyShip(ship);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : processLaunchShipToDo
    Description : processes a launch ship command
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship has finished launching
----------------------------------------------------------------------------*/
bool32 processLaunchShipToDo(CommandToDo *launchtodo)
{
    Ship *ship = launchtodo->selection->ShipPtr[0];
    Ship *receiverShip = launchtodo->launchship.receiverShip;

    dbgAssert(launchtodo->selection->numShips == 1);
    dbgAssert(receiverShip);

    if (ship->shiptype == sMothershipSpecial ||
        ship->shiptype == sMothershipArmour ||
        ship->shiptype == sMothershipMicro ||
        ship->shiptype == sMothershipWeapons)
    {
        if (receiverShip->clampInfo != NULL && receiverShip->clampInfo->selection != NULL)
        {
            SelectCommand *selection = receiverShip->clampInfo->selection;
            sdword i;

            for (i = 0; i < selection->numShips; i++)
            {
                Ship *clampedShip = selection->ShipPtr[i];

                if (clampedShip->shiptype == sMothershipOreCanArmour)
                {
                    shipInitModuleEject(clampedShip, TRUE, 7.0f, 10.0f);
                }
            }
        }
    }

    if (!processSingleLaunchShipToDo(ship, receiverShip, 2))
    {
        return FALSE;
    }


    if (ship->dockvars.dockPointIndex != -1 &&
        ship->staticinfo->shipclass != CLASS_Component)
    {
        Ship *dockShip = ship->dockvars.dockship;
        DockStaticInfo *dockStaticInfo = dockShip->staticinfo->dockStaticInfo;
        DockPointActivity *dockPointActivity = dockShip->dockvars.dockPointActivity;
        sdword dockPointIndex = ship->dockvars.dockPointIndex;
        DockPointType dockPointType = dockStaticInfo->dockstaticpoints[dockPointIndex].type;

        dockPointActivity->reserved[dockPointIndex] = 0;
        dockPointActivity->numDockedOfType[dockPointType]--;
        ship->dockvars.dockPointIndex = -1;
        ship->dockvars.dockship = NULL;
    }

    ship->specialFlags1C8 &= ~BIT12;
    ship->specialFlags2 &= ~0x40u;
    ship->flags2 &= ~0x1u;
    ship->flags &= 0x7fffffffu;
    return TRUE;
}

bool32 isThisShipThreateningMe(SpaceObjRotImpTarg *target,Ship *me)
{
    SpecificAttackVars *targetattackvars;
    Ship *targetsLeader;
    Ship *targetsWingman;

    if (target->objtype != OBJ_ShipType)
    {
        return FALSE;       // non-ship objects are not considered threatening
    }

    targetattackvars = &((Ship *)target)->attackvars;

    // check if target is attacking me
    if (targetattackvars->attacktarget == (SpaceObjRotImpTarg *)me)
    {
        if (targetattackvars->attacksituation == 0)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    // check if target's leader or wingman is attacking me
    if ((targetsLeader = targetattackvars->myLeaderIs) != NULL)
    {
        dbgAssert(targetattackvars->myWingmanIs == NULL);
        if (targetsLeader->attackvars.attacktarget == (SpaceObjRotImpTarg *)me)
        {
            if (targetattackvars->attacksituation == 0)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else if ((targetsWingman = targetattackvars->myWingmanIs) != NULL)
    {
        dbgAssert(targetattackvars->myLeaderIs == NULL);
        if (targetsWingman->attackvars.attacktarget == (SpaceObjRotImpTarg *)me)
        {
            if (targetattackvars->attacksituation == 0)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }


    return FALSE;
}

static SpaceObjRotImpTarg *attackFormationResolveTarget(Ship *ship, SpaceObjRotImpTarg *target)
{
    Ship *targetShip;
    ShipType runtimeShipType;
    SpaceObjSelection *attachedObjects;
    SpaceObjRotImpTarg *bestTarget;
    SpaceObjRotImpTarg *candidate;
    sdword startIndex;
    sdword i;
    real32 bestDistanceSquared;
    real32 candidateDistanceSquared;
    vector offset;

    if (target == NULL || target->objtype != OBJ_ShipType)
    {
        return target;
    }

    targetShip = (Ship *)target;
    runtimeShipType = targetShip->shiptype;
    if (runtimeShipType != sCarrier &&
        runtimeShipType != sMothership &&
        runtimeShipType != bMothership)
    {
        return target;
    }

    attachedObjects = NULL;
    if (targetShip->clampInfo != NULL &&
        targetShip->clampInfo->host == NULL &&
        targetShip->clampInfo->selection != NULL)
    {
        attachedObjects = (SpaceObjSelection *)targetShip->clampInfo->selection;
    }
    if (attachedObjects == NULL || attachedObjects->numSpaceObjs <= 0)
    {
        return target;
    }

    if (targetShip->shiptype == sMothership || targetShip->shiptype == bMothership)
    {
        bestTarget = (SpaceObjRotImpTarg *)attachedObjects->SpaceObjPtr[0];
        startIndex = 1;
    }
    else
    {
        bestTarget = target;
        startIndex = 0;
    }

    vecSub(offset, ship->posinfo.position, bestTarget->posinfo.position);
    bestDistanceSquared = vecDotProduct(offset, offset);

    for (i = startIndex; i < attachedObjects->numSpaceObjs; i++)
    {
        candidate = (SpaceObjRotImpTarg *)attachedObjects->SpaceObjPtr[i];
        if (candidate->objtype != OBJ_ShipType ||
            ((Ship *)candidate)->staticinfo->shipclass != CLASS_Component)
        {
            continue;
        }

        vecSub(offset, ship->posinfo.position, candidate->posinfo.position);
        candidateDistanceSquared = vecDotProduct(offset, offset);
        if (candidateDistanceSquared < bestDistanceSquared)
        {
            bestDistanceSquared = candidateDistanceSquared;
            bestTarget = candidate;
        }
    }


    return bestTarget;
}

static bool32 attackFormationShouldRotateAndShoot(Ship *ship, Ship *leader, SpaceObjRotImpTarg *target)
{
    vector leadhead;
    vector othervec;
    real32 dot;
    bool32 rotate;

    dbgAssert(ship != NULL);
    dbgAssert(leader != NULL);
    dbgAssert(target != NULL);

    matGetVectFromMatrixCol3(leadhead, leader->rotinfo.coordsys);
    vecSub(othervec, ship->posinfo.position, target->posinfo.position);
    dot = vecDotProduct(othervec, leadhead);
    rotate = (dot < 0.0f);


    return rotate;
}

static void attackFormationMoveToTargetOffset(Ship *ship, Ship *leader, bool32 rotate)
{
    ShipStaticInfo *leaderStaticInfo = leader->staticinfo;
    ShipStaticInfo *shipStaticInfo = ship->staticinfo;
    vector desiredHeading;
    vector destination;
    vector slotOffset;
    real32 separationDistance;

    matGetVectFromMatrixCol2(destination, leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(desiredHeading, leader->rotinfo.coordsys);

    separationDistance =
        (leaderStaticInfo->staticheader.staticCollInfo.collspheresize +
         shipStaticInfo->staticheader.staticCollInfo.collspheresize) *
        DELTA_FORMATION_PADDING_SCALE;

    destination.x = (destination.x - desiredHeading.x) * separationDistance + leader->posinfo.position.x;
    destination.y = (destination.y - desiredHeading.y) * separationDistance + leader->posinfo.position.y;
    destination.z = (destination.z - desiredHeading.z) * separationDistance + leader->posinfo.position.z;
    vecSub(slotOffset, destination, leader->posinfo.position);


    if (rotate)
    {
        real32 maxDistanceToDestination = MoveLeftToGoDistance(ship, &destination);
        udword aishipFlags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;

        if (maxDistanceToDestination <= 100.0f)
        {
            aitrackHeadingWithBank(ship, &desiredHeading, FLYSHIP_HEADINGACCURACY, shipStaticInfo->sinbank);
            aishipFlags = AISHIP_FastAsPossible;
        }

        if (maxDistanceToDestination >= 10.0f)
        {
            aishipFlyToPointAvoidingObjsWithVel(ship, &destination, aishipFlags, 0.0f, &leader->posinfo.velocity);
        }
    }
    else if (!MoveReachedDestinationVariable(ship, &destination, 10.0f))
    {
        aishipFlyToPointAvoidingObjsWithVel(ship, &destination, AISHIP_FastAsPossible, 0.0f, &leader->posinfo.velocity);
    }
}

static void attackFormationResetShipTargetState(Ship *ship)
{
    *(ubyte *)&ship->specialFlags = 0;
    ship->aistateattack = 0;
}

bool32 EnemyIsBehindMe(SpaceObjRotImpTarg *enemy,Ship *me);

static udword attackFormationGetEvasiveSubtype(Ship *ship, SpaceObjRotImpTarg *target)
{
    return EnemyIsBehindMe(target, ship) ? FLIGHTMAN_EVASIVE_BEHIND : FLIGHTMAN_EVASIVE_FRONT;
}

SpaceObjRotImpTarg *FindNewThreateningTarget(Ship *ship,AttackCommand *attack,SpaceObjRotImpTarg *oldtarget)
{
    sdword numShipsToAttack = attack->numTargets;
    SpaceObjRotImpTarg *newtarget;
    vector dist;
    real32 distsqr;
    real32 mindist = REALlyBig;
    sdword mindistj = -1;
    sdword j;
    TacticsType tacticstype = ship->tacticstype;
    real32 fighterMaxThreatDist = FIGHTER_MAX_THREAT_DIST[tacticstype];

    dbgAssert(tacticstype < NUM_TACTICS_TYPES);

    // for all ships/targets we are attacking
    for (j=0;j<numShipsToAttack;j++)
    {
        newtarget = attack->TargetPtr[j];
        if (newtarget == oldtarget)
        {
            continue;
        }
        if (newtarget->objtype == OBJ_ShipType &&
            ((Ship *)newtarget)->clampInfo != NULL &&
            ((Ship *)newtarget)->clampInfo->host == (SpaceObjRotImpTarg *)ship)
        {
            continue;
        }
        if (isThisShipThreateningMe(newtarget,ship))
        {
            vecSub(dist,newtarget->posinfo.position,ship->posinfo.position);
            distsqr = vecMagnitudeSquared(dist);


            if (distsqr < fighterMaxThreatDist)
            {
                if (distsqr < mindist)
                {
                    mindist = distsqr;
                    mindistj = j;
                }
            }
        }
    }

    if (mindistj != -1)
    {
        return attack->TargetPtr[mindistj];
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : EnemyIsBehindMe
    Description : returns TRUE if enemy is behind me
    Inputs      : enemy, ship
    Outputs     :
    Return      : returns TRUE if enemy is behind me
----------------------------------------------------------------------------*/
bool32 EnemyIsBehindMe(SpaceObjRotImpTarg *enemy,Ship *me)
{
    vector myheading;
    vector enemytome;

    matGetVectFromMatrixCol3(myheading,me->rotinfo.coordsys);
    vecSub(enemytome,me->posinfo.position,enemy->posinfo.position);

    if (vecDotProduct(myheading,enemytome) > 0.0f)
    {
        // enemy is behind us
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : processSpecialToDo
    Description : processes a special
    Inputs      :
    Outputs     :
    Return      : returns TRUE if special has been completed
----------------------------------------------------------------------------*/
bool32 processSpecialToDo(CommandToDo *todo)
{
    SelectCommand *selection = todo->selection;
    sdword numShips = selection->numShips;
    Ship *ship;
    ShipStaticInfo *shipstaticinfo;
    bool32 alldone = TRUE;
    bool32 shipdone;
    sdword i;

    dbgAssert(numShips > 0);

    if (todo->specialtargets)
    {
        // do the special target command

        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            shipdone = TRUE;

            if (shipstaticinfo->custshipheader.CustShipSpecialTargetRaw != NULL)
            {
                shipdone = shipstaticinfo->custshipheader.CustShipSpecialTargetRaw(ship, todo->specialtargets);
            }

            alldone &= shipdone;
        }
        return alldone;
    }
    else
    {
        // Retail runs the queued no-target special callback directly here.

        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            shipdone = TRUE;

            if (shipstaticinfo->custshipheader.CustShipSpecialActivateNoTarget != NULL)
            {
                shipdone = shipstaticinfo->custshipheader.CustShipSpecialActivateNoTarget(ship);
            }

            alldone &= shipdone;
        }
        return alldone;
    }
}

/*-----------------------------------------------------------------------------
    Name        : processHaltToDo
    Description : returns TRUE if halt command is finished
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 processHaltToDo(CommandToDo *command,bool32 passiveAttacked)
{
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;
    bool32 done = TRUE;
    sdword i;
    Ship *ship;

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if ((passiveAttacked) & (bool32)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
        {
            done &= aitrackSteadyShipDriftOnly(ship);
        }
        else
        {
            done &= aitrackSteadyShip(ship);
        }
    }

    return done;
}

/*-----------------------------------------------------------------------------
    Name        : ShipsOutOfPassiveAttackRange
    Description : returns TRUE if ships being attacked are out of selection's passive attack range
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ships being attacked are out of selection's passive attack range
----------------------------------------------------------------------------*/
bool32 ShipsInPassiveAttackRange(SelectCommand *selection,AttackCommand *attack)
{
    sdword numShips = selection->numShips;
    sdword numShipsToAttack = attack->numTargets;
    sdword i;
    vector avgposition = { 0.0f, 0.0f, 0.0f };
    Ship *ship;
    vector diff;
    real32 retaliateZone;
    real32 negRetaliateZone;
    bool32 aShipInRange = FALSE;
    real32 temp;

    dbgAssert(numShips > 0);
    dbgAssert(numShipsToAttack > 0);

    for (i=0;i<numShipsToAttack;i++)
    {
        vecAddTo(avgposition,attack->TargetPtr[i]->posinfo.position);
    }

    vecDivideByScalar(avgposition,(real32)numShipsToAttack,temp);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        retaliateZone = ship->staticinfo->passiveRetaliateZone;
        negRetaliateZone = -retaliateZone;

        vecSub(diff,ship->posinfo.position,avgposition);

        if (isBetweenExclusive(diff.x,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.y,negRetaliateZone,retaliateZone) &&
            isBetweenExclusive(diff.z,negRetaliateZone,retaliateZone) )
        {
            aShipInRange = TRUE;
        }
    }

    return aShipInRange;
}

/*-----------------------------------------------------------------------------
    Name        : processPassiveAttackToDo
    Description : processes a passive attack command (attack without moving)
    Inputs      :
    Outputs     :
    Return      : returns TRUE if passive attack has been completed
----------------------------------------------------------------------------*/
static void clPassiveAttackShip(ShipStaticInfo *shipstaticinfo, Ship *ship, SpaceObjRotImpTarg *target, bool32 rotate)
{
    if (shipstaticinfo->custshipheader.CustShipRotateAndShoot)
    {
        shipstaticinfo->custshipheader.CustShipRotateAndShoot(ship, target, rotate);
    }
}

static void clAttackShip(ShipStaticInfo *shipstaticinfo, Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    bool32 (*attackCommand)(Ship *, SpaceObjRotImpTarg *, real32);

    attackCommand = shipstaticinfo->custshipheader.CustShipActiveAttackCommand;
    if (attackCommand)
    {
        attackCommand(ship, target, maxdist);
    }
    else if (shipstaticinfo->custshipheader.CustShipAttack)
    {
        shipstaticinfo->custshipheader.CustShipAttack(ship, target, maxdist);
    }
}

// Retail reuses the formation.formationLocked/formation.coordsys fields as scratch storage
// for a single-ship attack maneuver's "stored target coordsys" while the command isn't actually
// flying in formation. Confirmed via IDA disassembly of clAttackStoreTargetCoordsys @ 0x486270:
//   lea edi,[eax+238h]; mov ecx,9; rep movsd   -> memcpy(&command->formation.coordsys, ...)
//   mov [eax+234h],ecx                         -> command->formation.formationLocked = TRUE
// The heading negation touches eax+250h/254h/258h, i.e. coordsys.m13/m23/m33 (the matrix's
// heading column - see matGetVectFromMatrixCol3 in matrix.h).
static void clAttackStoreTargetCoordsys(CommandToDo *command, bool32 flipHeading)
{
    command->formation.coordsys = command->selection->ShipPtr[0]->rotinfo.coordsys;

    if (flipHeading)
    {
        command->formation.coordsys.m13 = -command->formation.coordsys.m13;
        command->formation.coordsys.m23 = -command->formation.coordsys.m23;
        command->formation.coordsys.m33 = -command->formation.coordsys.m33;
    }

    command->formation.formationLocked = TRUE;
}

static void clAttackClearStoredTargetCoordsys(CommandToDo *command)
{
    command->formation.formationLocked = FALSE;
}

bool32 processPassiveAttackToDo(CommandToDo *attacktodo)
{
    SelectCommand *selection = attacktodo->selection;
    AttackCommand *attack = attacktodo->attack;
    sdword numShips = selection->numShips;
    sdword numShipsToAttack = attack->numTargets;
    Ship *ship;
    ShipStaticInfo *shipstaticinfo;
    SpecificAttackVars *attackvars;
    SpaceObjRotImpTarg *target;
    sdword i;
    bool32 rotate;
    bool32 inPassiveRange;

    dbgAssert(numShips > 0);


    if ((numShipsToAttack == 0)
        || (!(inPassiveRange = ShipsInPassiveAttackRange(selection,attack)))
        || ((singlePlayerGame) && (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE ) ))
    {
#ifndef HW_Release
#ifdef gshaw
        if (numShipsToAttack != 0)
        {
            dbgMessagef("\nDropping out of passive attack");
        }
#endif

#endif


        for (i=0;i<numShips;i++)
        {
            soundEventShipRemove(selection->ShipPtr[i]);
        }

        return TRUE;
    }

//fix later and optimize by passing in a rotate flag
    if(attacktodo->ordertype.order == COMMAND_MOVE ||
        attacktodo->ordertype.order == COMMAND_DOCK)
        rotate = FALSE;
    else
        rotate = TRUE;

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];

        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if (target == NULL)
        {
            soundEventShipRemove(ship);
            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            }
            ship->specialFlags &= 0xFFFFFF00u;
            ship->aistateattack = 0;
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & multipleTargetsRetargetMask) == (ship->shipID.shipNumber & multipleTargetsRetargetMask))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }
        //dbgAssert(target->objtype == OBJ_ShipType);     // only target SHIPs should be passive attacked
        clPassiveAttackShip(shipstaticinfo, ship, target, rotate);
        attackvars->attackevents = 0;       // clear any events we received
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : processAttackToDo
    Description : processes an attack command
    Inputs      :
    Outputs     :
    Return      : returns TRUE if attack has been completed
----------------------------------------------------------------------------*/
bool32 processAttackToDo(CommandToDo *attacktodo)
{
    // NOTE: the per-ship resets below (here and in ChangeOrderToAttackTargets,
    // clAttackTargetsForSelection, GroupShip) write ship+0x350 (aistateattack)
    // per retail raw disasm, not rcstate2 (0x354) -- see ChangeOrderToAttack's
    // comment for the full decompiler-mislabeling explanation.
    sdword donness = 0;
    CommandToDo fakeCommand;
    MaxSelection selectionDefenders, selectionCorvettes,selectionCaptialShips,selectionInterceptors;
    AttackCommand *attack = attacktodo->attack;
    sdword numShipsToAttack = attack->numTargets;
    SelectCommand *selection = attacktodo->selection;
    bool32 alldone;
    ShipStaticInfo *shipstaticinfo;
    Ship *ship;
    SpaceObjRotImpTarg *target;

    sdword i;
    sdword should = TRUE;
    bool32 focusfire = FALSE;

    SpecificAttackVars *attackvars;

    dbgAssert(selection->numShips > 0);

    if (selection->ShipPtr[0]->tacticstype == Aggressive)
    {
        focusfire = TRUE;
    }

    if (numShipsToAttack == 0)
    {                                                               // don't do holding pattern for swarmers (don't want them to go into delta formation)
        if(isSelectionExclusivlyStrikeCraft(attacktodo->selection))
        {
            attacktodo->holdingStartTime = universe.totaltimeelapsed;
            attacktodo->turnAroundTimes = 0;
            matGetVectFromMatrixCol3(attacktodo->holdingPatternVec,selection->ShipPtr[0]->rotinfo.coordsys);
            vecScalarMultiply(attacktodo->holdingPatternVec,attacktodo->holdingPatternVec,TW_HOLDING_PATTERN_PATH_DISTANCE);
            for (i=0;i<selection->numShips;i++)
            {
                attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
                soundEventShipRemove(selection->ShipPtr[i]);
            }
            if(attacktodo->selection->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Strike_AttackComplete, SOUND_EVENT_DEFAULT, NULL);
            }
            attacktodo->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
            return TRUE;
        }
        alldone = TRUE;

        for (i=0;i<selection->numShips;i++)
        {
            if (selection->ShipPtr[i]->collMyBlob == NULL)
            {
                return TRUE;
            }
            attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
            soundEventShipRemove(selection->ShipPtr[i]);
        }
        if (attacktodo->ordertype.attributes)
        {
            return TRUE;
        }

        for (i=0;i<selection->numShips;i++)
        {
            ship = selection->ShipPtr[i];
            if (!aitrackSteadyShip(ship))
            {
                alldone = FALSE;
            }
        }

        return alldone;

    }

    //check attack conditions as to whether or not we need to
    //go back to guarding a ship...

    if((tacticsInfo.tacticsGuardConditionCheckRate & universe.univUpdateCounter) == tacticsInfo.tacticsGuardConditionCheckFrame)
    {
        if(attacktodo->ordertype.attributes & COMMAND_IS_PROTECTING)
        if(clIsSelectionNearProtectTargets(attacktodo))
        {
            for (i=0;i<selection->numShips;i++)
            {
                soundEventShipRemove(selection->ShipPtr[i]);
            }
            return TRUE;
        }
    }

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        ship->shipidle = FALSE;
        target = attackvars->attacktarget;
        if (target == NULL)
        {
            soundEventShipRemove(ship);
            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            }
            *(ubyte *)&ship->specialFlags = 0;
            ship->aistateattack = 0;
        }

        if (attackvars->multipleAttackTargets &&
            ((universe.univUpdateCounter & multipleTargetsRetargetMask) ==
             ((multipleTargetsRetargetMask & ship->shipID.shipNumber))))
        {
            pickMultipleAttackTargets(ship, attack);
        }

        if (target == NULL)
        {
            ship->aistateattack = 0;
            attackvars->attackevents = 0;
            continue;
        }

        if(i >= 1)
        {
            if(attacktodo->formation.formationtype == SPHERE_FORMATION ||
               attacktodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
            {
                //do this check infrequently
                if ((universe.univUpdateCounter & SPHERE_CHECK_TARGET_MASK) == SPHERE_CHECK_TARGET_FRAME)
                {
                    if (ship->attackvars.attacktarget != NULL)
                    {
                        vector distvect;
                        real32 distSqr;

                        vecSub(distvect, ship->attackvars.attacktarget->posinfo.position, ship->posinfo.position);
                        distSqr = vecMagnitudeSquared(distvect);
                        if (fsqrt(distSqr) > ship->staticinfo->bulletRange[ship->tacticstype])
                        {
                            soundEventShipRemove(ship);
                            {
                                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                                attackvars->attacktarget = AttackClosestOpponent(ship,attack);
                            }
                            *(ubyte *)&ship->specialFlags = 0;
                            ship->aistateattack = 0;
                        }
                    }
                }
            }
        }
        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & multipleTargetsRetargetMask) == (ship->shipID.shipNumber & multipleTargetsRetargetMask))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }
    }

    //ships should each be guaranteed to have their proper targets now!


    //GET TARGET HERE!

    //then, if in sphere formation, and target doesn't = center ship,
    //    make it the center ship, otherwise, center ship is target, so keep
    //    pumpleing it


    fakeCommand = *attacktodo;
    formGroups(selection,(SelectCommand *)&selectionDefenders,(SelectCommand *)&selectionCorvettes,(SelectCommand *)&selectionCaptialShips,(SelectCommand *)&selectionInterceptors);
    if(attacktodo->formation.formationtype == SPHERE_FORMATION ||
       attacktodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
    {
        //if(attacktodo->formation.enders != TRUE || attacktodo->formation.endership != (Ship *)leadertarget)
        if(attacktodo->formation.enders != TRUE)
        {
            //FormationSphereInfo *sphereInfo = (FormationSphereInfo *)attacktodo->formation.formationSpecificInfo;
            //SphereVar *sphereVar;
            attacktodo->formation.enders = TRUE;
            //attacktodo->formation.endership = (Ship *)leadertarget;
            //sphere formation...lets do enders game..
            //
            //first we need to save old formation type, so we can change it back

            //attacktodo->selection->ShipPtr[attacktodo->selection->numShips] = attacktodo->selection->ShipPtr[0];
            //attacktodo->selection->numShips++;
            //attacktodo->selection->ShipPtr[0] = (Ship *)target;
            formationContentHasChanged(attacktodo);
            should = FALSE;
        }
        else
        {
            should = FALSE;
        }
        processFormationToDo(attacktodo,FALSE,TRUE);
    }
    else
    {
        should = TRUE;
    }

    if (fakeCommand.attack->numTargets <= 0)
    {
        return TRUE;
    }

    if(selectionInterceptors.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionInterceptors;
        donness |= delegateCommand(&fakeCommand,11,should,attacktodo->selection);
    }

    if(selectionDefenders.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionDefenders;
        donness |= delegateCommand(&fakeCommand,10,should,attacktodo->selection);
    }
    if(selectionCorvettes.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionCorvettes;
        donness |= delegateCommand(&fakeCommand,13,should,attacktodo->selection);
    }

    if(selectionCaptialShips.numShips > 0)
    {
        fakeCommand.selection = (SelectCommand *)&selectionCaptialShips;
        donness |= delegateCommand(&fakeCommand,12,should,attacktodo->selection);
        //processAttackToDo(&fakeCommand);
    }

    return(donness);
}
/*-----------------------------------------------------------------------------
    Name        : fixWallFlipTurnSelection
    Description : Fixes selection when ships in WALL only formation attempt a flipturn
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void fixWallFlipTurnSelection(SelectCommand *selection)
{
    sdword i;

    for(i=1;i<selection->numShips;i++)
    {
        selection->ShipPtr[i]->formationOffset.y = -selection->ShipPtr[i]->formationOffset.y;
    }

}
/*-----------------------------------------------------------------------------
    Name        : fixFlipTurnSelection
    Description : Fixes selection when ships in formation attempt a flipturn
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void fixFlipTurnSelection(SelectCommand *selection,SelectCommand *global)
{
    sdword i;//,j,switcher;
    Ship *tempship;
    //vector tempvec,tempvec2; //temporary formation vector
    //j=0;
    for(i=1;i<selection->numShips;i++)
    {
        //save temporary ship
        tempship = selection->ShipPtr[i];

        /*
        tempvec = selection->ShipPtr[i]->formationOffset;

        i++;
        if(i >= selection->numShips)
        {
            */
            tempship->formationOffset.y = -tempship->formationOffset.y;
        /*    break;              //done
        }

        while(tempship != global->ShipPtr[j])
            j++;
        //while loop exits when j points to same ship as temp ship

        switcher = j;
        while(global->ShipPtr[j] != selection->ShipPtr[i])
            j++;
        //while loop exits when j points to other ship to switch

        //swap ships and continue;
        selection->ShipPtr[i-1] = selection->ShipPtr[i];
        tempvec2 = selection->ShipPtr[i]->formationOffset;
        selection->ShipPtr[i-1]->formationOffset = tempvec;

        selection->ShipPtr[i] = tempship;
        selection->ShipPtr[i]->formationOffset = tempvec2;

        global->ShipPtr[switcher] = selection->ShipPtr[i-1];
        global->ShipPtr[j] = tempship;
        //global->ShipPtr[switcher]->formationOffset = selection->ShipPtr[i-1]->formationOffset;
        //global->ShipPtr[j]->formationOffset = tempvec;
        */
    }
}
void formGroups(SelectCommand *selection,SelectCommand *selectionDefenders,SelectCommand *selectionCorvettes,SelectCommand *selectionCapitalShips,SelectCommand *selectionInterceptors)
{
    sdword a,i,j,k,l;
    selectionDefenders->numShips = 0;       //i
    selectionCorvettes->numShips = 0;       //j
    selectionCapitalShips->numShips = 0;    //k
    selectionInterceptors->numShips = 0;    //l
    i = j = k = l = 0;

    for(a=0;a<selection->numShips;a++)
    {
        if(selection->ShipPtr[a]->staticinfo->shipclass == CLASS_Fighter)
        {
            //check if defender, else fighter
            if(selection->ShipPtr[a]->staticinfo->shiptype == kHeavyDefender ||
               selection->ShipPtr[a]->staticinfo->shiptype == tHeavyDefender)
            {
                selectionDefenders->ShipPtr[i] = selection->ShipPtr[a];
                i++;
                selectionDefenders->timeLastStatus = selection->timeLastStatus;
                selectionDefenders->numShips++;
            }
            else
            {
                selectionInterceptors->ShipPtr[l] = selection->ShipPtr[a];
                l++;
                selectionInterceptors->timeLastStatus = selection->timeLastStatus;
                selectionInterceptors->numShips++;
            }
        }
        else if(selection->ShipPtr[a]->staticinfo->shipclass == CLASS_Corvette)
        {
            selectionCorvettes->ShipPtr[j] = selection->ShipPtr[a];
            j++;
            selectionCorvettes->timeLastStatus = selection->timeLastStatus;
            selectionCorvettes->numShips++;

        }
        else        // capital ship and miscellaneous ships
        {
            //captital ship
            selectionCapitalShips->ShipPtr[k] = selection->ShipPtr[a];
            k++;
            selectionCapitalShips->timeLastStatus = selection->timeLastStatus;
            selectionCapitalShips->numShips++;
        }
    }

}
//group types
#define DEFENDERS       10
#define INTERCEPTORS    11
#define CAPITALS        12
#define CORVETTES       13

bool32 delegateCommand(CommandToDo *attacktodo,sdword group,sdword doform, SelectCommand *globalselection)
{
    sdword i;
    SelectCommand *selection = attacktodo->selection;
    AttackCommand *attack = attacktodo->attack;
    sdword numShips = selection->numShips;
    sdword allNotAttacking;
    sdword formationoverride = FALSE;
    sdword numShipsToAttack = attack->numTargets;
    Ship *ship;
    sdword pee;
    Ship *tempship;
    ShipStaticInfo *shipstaticinfo;
    SpecificAttackVars *attackvars;
    SpaceObjRotImpTarg *target;
    Ship *formationLeader;
    vector leadhead,othervec;
    real32 maxdist;
    bool32 attackAndMoving;
    formationLeader = selection->ShipPtr[0];   //set leader ship

    attackAndMoving = FALSE;
    if(attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
    {
        //specialized movement + combat
        attackAndMoving = TRUE;             //set flag for future movement reference
        processAttackMoveToDo(attacktodo,TRUE,0,TRUE);   //move the group to the new attack location

    }
    //perform fancy pants reordering of selection if we leader does a flip turn
    if(group == INTERCEPTORS)
    {
        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                if (shipstaticinfo->custshipheader.CustShipActiveAttackCommand != NULL)
                {
                    shipstaticinfo->custshipheader.CustShipActiveAttackCommand(
                        ship,
                        target,
                        ship->staticinfo->bulletRange[ship->tacticstype]);
                }
            }
        }
        else
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
            }
        }
if(ship->tacticsTalk != 0)
{
    if(ship->command->formation.flipselectionfixed != TRUE)
    {
        if(!bitTest(ship->tacticsTalk,TACTALK_BARRELROLL))
        {
            ship->command->formation.needFix = TRUE;
            clAttackStoreTargetCoordsys(ship->command, TRUE);
        }
        else
        {
            ship->command->formation.needFix = FALSE;
            clAttackStoreTargetCoordsys(ship->command, FALSE);
        }

        ship->command->formation.flipselectionfixed = TRUE;
        for(i=1;i<numShips;i++)
        {
            flightmanInitFunc(selection->ShipPtr[i],ship->flightman,0);
            selection->ShipPtr[i]->dockwaitforNorm = TRUE;
        }
    }
}
else
{
    if(ship->command->formation.flipselectionfixed == TRUE)
    {
        clAttackClearStoredTargetCoordsys(ship->command);
        if(ship->command->formation.needFix == TRUE)
        {
            for (i = 1; i < numShips; i++)
            {
                selection->ShipPtr[i]->formationOffset.z = -selection->ShipPtr[i]->formationOffset.z;
            }
            ship->command->formation.needFix = FALSE;
            ship->command->formation.flipState = !ship->command->formation.flipState;
        }
    }
            ship->command->formation.flipselectionfixed = FALSE;
        }


        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;
             attackvars->attackevents = 0;       // clear any events we received

            if(ship->dockwaitforNorm)
            {
                if(flightmanExecute(ship))
                {
                    ship->dockwaitforNorm = FALSE;
                }
                else
                {
                    formationoverride = TRUE;
                }
                continue;
            }
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipRotateAndShoot)
            {
                //only do the attack if in proper attack state...
                //quick fix
                if(attacktodo->formation.formationtype == SPHERE_FORMATION ||
                   attacktodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
                {
                    clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
                    ship->recentlyAttacked |= 4;
                }
                else
                {
                    matGetVectFromMatrixCol3(leadhead,formationLeader->rotinfo.coordsys);
                    vecSub(othervec,ship->posinfo.position,target->posinfo.position);
                    if(vecDotProduct(othervec,leadhead) < 0)
                    {
                        clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
                        ship->recentlyAttacked |= 4;
                    }
                    else
                    {
                        ship->recentlyAttacked &= (ubyte)~4;
                    }
                }
            }

            /*
            if (shipstaticinfo->custshipheader.CustShipAttackPassive)
            {
                //only do the attack if in proper attack state...
                if(!doform)
                {
                    shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                    continue;
                }
                if(ship->tacticsFormationVar1 == TRUE)
                    if(selection->ShipPtr[0]->aistateattack != 4)
                    {
                        //set flag so we can try to attack because leader has finished attack pass
                        continue;
                    }
                    else
                    {
                        ship->tacticsFormationVar1 = FALSE;
                    }


                if(InterceptorInRange(ship,target))
                {
                    //set a state flag here so when it tries to passive attack again later, it won't
                    //unless the leader is again attacking...
                    ship->tacticsFormationVar1 = FALSE;
                    shipstaticinfo->custshipheader.CustShipAttackPassive(ship,(Ship *)target,TRUE);
                }
                else
                {
                    //ship is too close
                    ship->tacticsFormationVar1 = TRUE;
                    allNotAttacking++;
                }
            }
            */
        }
        if(doform)
        {
            if(!formationoverride)
            {
                processFormationToDo(attacktodo,FALSE,FALSE);
            }
            else
            {
                //do formation, but loosish...
                processFormationToDo(attacktodo,FALSE,TRUE);
            }
        }

        for(i=0;i<selection->numShips;i++)
        {
            selection->ShipPtr[i]->recentlyAttacked &= (ubyte)~4;
        }
    }
    else if(group == DEFENDERS)
    {
        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;


        if (target == NULL)
        {
            soundEventShipRemove(ship);
            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            }
            attackFormationResetShipTargetState(ship);
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & multipleTargetsRetargetMask) == (ship->shipID.shipNumber & multipleTargetsRetargetMask))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }

        maxdist = tacticsMaxDistToTarget(selection,target);
        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                clAttackShip(shipstaticinfo, ship, target, maxdist);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
            }
        }


        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;

            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipRotateAndShoot)
            {
                //only do the attack if in proper attack state...
                clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
            }
        attackvars->attackevents = 0;       // clear any events we received
        }
        //these need to be moved up...

        if(doform)
        {
            processFormationToDo(attacktodo,FALSE,TRUE);
        }

    }
    else if(group == CORVETTES)
    {
        if(selection->ShipPtr[0]->shiptype == tMinelayerCorvette)
        {
            for(pee=1;pee<selection->numShips;pee++)
            {
                if(selection->ShipPtr[pee]->shiptype != tMinelayerCorvette)
                {
                    tempship = selection->ShipPtr[0];
                    selection->ShipPtr[0] = selection->ShipPtr[pee];
                    selection->ShipPtr[pee] = tempship;
                }
            }
        }

        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;

        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                if (shipstaticinfo->custshipheader.CustShipActiveAttackCommand != NULL)
                {
                    shipstaticinfo->custshipheader.CustShipActiveAttackCommand(
                        ship,
                        target,
                        ship->staticinfo->bulletRange[ship->tacticstype]);
                }
            }
        }
        else
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
            }
        }

        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;

            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else if (shipstaticinfo->custshipheader.CustShipRotateAndShoot)

            {
                //only do the attack if in proper attack state...
                clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
            }
            else if(shipstaticinfo->shiptype == tMinelayerCorvette)
            {
                bitSet(ship->specialFlags1C8, CATA_RUNTIME_BROKEN_FORMATION);
                clAttackShip(shipstaticinfo, ship, target, ship->staticinfo->bulletRange[ship->tacticstype]);
            }
            attackvars->attackevents = 0;       // clear any events we received
        }

        if(doform)
        {
            ubyte leaderAttackState = commandLayerAttackStateForShip(selection->ShipPtr[0]);

            if(leaderAttackState == 3 || leaderAttackState == 4)
            {
            //leader of formation is turning around or reaproaching, so lets do a real formation...
                processFormationToDo(attacktodo,FALSE,FALSE);
            }
            else
            {
                processFormationToDo(attacktodo,FALSE,TRUE);
            }
        }
    }
    else if(group == CAPITALS)
    {
        i=0;
        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        target = attackvars->attacktarget;


        if (target == NULL)
        {
            soundEventShipRemove(ship);
            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = target = AttackClosestOpponent(ship,attack);
            }
            attackFormationResetShipTargetState(ship);
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & multipleTargetsRetargetMask) == (ship->shipID.shipNumber & multipleTargetsRetargetMask))
            {
                pickMultipleAttackTargets(ship,attack);
            }
        }

        maxdist = tacticsMaxDistToTarget(selection,target);
        if(doform && !attackAndMoving)
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                clAttackShip(shipstaticinfo, ship, target, maxdist);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
            }
        }


        allNotAttacking=0;
        for (i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
            attackvars = &ship->attackvars;
            target = attackvars->attacktarget;

            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship,target);
            }
            else
            {
                if (shipstaticinfo->custshipheader.CustShipRotateAndShoot)
                {
                    //only do the attack if in proper attack state...
                    clPassiveAttackShip(shipstaticinfo, ship, target, TRUE);
                }
            }
            ship->formationcommand = NULL;
            ship->shipidle = FALSE;
            attackvars->attackevents = 0;       // clear any events we received
        }

        if(doform)
        {
            processFormationToDo(attacktodo,FALSE,TRUE);
        }


    }
    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : targetIsntNearShip
    Description : determines if ships target is in or out of gun range

    Inputs      :
    Outputs     :
    Return      : returns TRUE if ships target is out of gun range
----------------------------------------------------------------------------*/

bool32 targetIsntNearShip(Ship *ship,SpaceObjRotImpTarg *target)
{
    vector distvect;
    real32 dist;
    vecSub(distvect,target->posinfo.position,ship->posinfo.position);
    dist = vecMagnitudeSquared(distvect);
    dist = fsqrt(dist);

    return(dist > ship->staticinfo->bulletRange[ship->tacticstype]);
}

/*-----------------------------------------------------------------------------
    Name        : processAttackToDoInFormation
    Description : processes an attack command for a group of ships in
                  formation...maintains formation
    Inputs      :
    Outputs     :
    Return      : returns TRUE if attack has been completed
----------------------------------------------------------------------------*/
bool32 processAttackToDoInFormation(CommandToDo *attacktodo)
{
    AttackCommand *attack = attacktodo->attack;
    sdword numShipsToAttack = attack->numTargets;
    SelectCommand *selection = attacktodo->selection;
    bool32 alldone;
    bool32 attackAndMoving = (attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING) != 0;
    ShipStaticInfo *shipstaticinfo;
    Ship *ship;
    SpaceObjRotImpTarg *target;
    SpaceObjRotImpTarg *newTarget;
    sdword i;
    SpecificAttackVars *attackvars;

    dbgAssert(selection->numShips > 0);

    if (numShipsToAttack > 0)
    {
        bool32 anyVisibleTarget = FALSE;
        Player *player = selection->ShipPtr[0]->playerowner;

        for (i = 0; i < numShipsToAttack; i++)
        {
            target = attack->TargetPtr[i];
            if (((target->objtype == OBJ_ShipType) &&
                 (((Ship *)target)->collMyBlob == NULL) &&
                 clIsCrystal((Ship *)target)) ||
                fogObjectVisibleToPlayer(player, (SpaceObj *)target, TRUE))
            {
                anyVisibleTarget = TRUE;
                break;
            }
        }

        if (!anyVisibleTarget)
        {
            if (multiPlayerGame)
            {
                clWrapHalt(&universe.mainCommandLayer, selection);
                return TRUE;
            }
            else if (!(selection->ShipPtr[0]->specialFlags2 & SPECIAL_2_DisabledState))
            {
                numShipsToAttack = 0;
            }
        }
    }

    if (numShipsToAttack == 0)
    {
        if(isSelectionExclusivlyStrikeCraft(attacktodo->selection))
        {
            attacktodo->holdingStartTime = universe.totaltimeelapsed;
            attacktodo->turnAroundTimes = 0;
            matGetVectFromMatrixCol3(attacktodo->holdingPatternVec,selection->ShipPtr[0]->rotinfo.coordsys);
            vecScalarMultiply(attacktodo->holdingPatternVec,attacktodo->holdingPatternVec,TW_HOLDING_PATTERN_PATH_DISTANCE);
            for (i=0;i<selection->numShips;i++)
            {
                attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
                soundEventShipRemove(selection->ShipPtr[i]);
            }

            if(attacktodo->selection->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_STAT_Strike_AttackComplete, SOUND_EVENT_DEFAULT, NULL);
            }

            attacktodo->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
            return TRUE;
        }
        alldone = TRUE;

        for (i=0;i<selection->numShips;i++)
        {
            attacktodo->selection->ShipPtr[i]->holdingPatternPos = selection->ShipPtr[i]->posinfo.position;
            soundEventShipRemove(selection->ShipPtr[i]);
        }
        if (attacktodo->ordertype.attributes)
        {
            return TRUE;
        }

        for (i=0;i<selection->numShips;i++)
        {
            ship = selection->ShipPtr[i];
            if (!aitrackSteadyShip(ship))
            {
                alldone = FALSE;
            }
        }

        return alldone;

    }


    if((tacticsInfo.tacticsGuardConditionCheckRate & universe.univUpdateCounter) == tacticsInfo.tacticsGuardConditionCheckFrame)
    {
        if(attacktodo->ordertype.attributes & COMMAND_IS_PROTECTING)
        if(clIsSelectionNearProtectTargets(attacktodo))
        {
            for (i=0;i<selection->numShips;i++)
            {
                soundEventShipRemove(selection->ShipPtr[i]);
            }
            return TRUE;
        }
    }

    if (attackAndMoving)
    {
        for (i = 0; i < selection->numShips; i++)
        {
            processAttackMoveToDo(attacktodo, TRUE, i, FALSE);
        }
    }

    for (i=0;i<selection->numShips;i++)
    {
        Ship *myLeader;
        Ship *myWingman;
        ubyte *attackRecord;
        udword flightman;
        udword evasiveSubtype;
        udword checkRate;
        bool32 pairedShip;

        ship = selection->ShipPtr[i];
        shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);
        attackvars = &ship->attackvars;
        ship->shipidle = FALSE;
        target = attackvars->attacktarget;
        if (ship->collMyBlob == NULL)
        {
            dbgFatalf(DBG_Loc, "processAttackToDoInFormation: ship without collMyBlob");
            return TRUE;
        }

        if (shipstaticinfo->custshipheader.CustShipActiveAttackCommand == shipSharedFlightManAttackCommand)
        {
            TacticsType tacticstype = ship->tacticstype;

            if (attackvars->flightmansLeft)
            {
                if (flightmanExecute(ship))
                {
                    if (--attackvars->flightmansLeft)
                    {
                        attackRecord = shipGetAttackRecord(ship, target);
                        flightman = flightmanGetRandom((FlightManProb *)attackRecord, FLIGHTMAN_EVASIVE_PURE);
                        flightmanInitFunc(ship, flightman, -1);
                    }
                    else
                    {
                        attackvars->attacksituation = 0;
                    }
                }
                goto nextShip;
            }

            myLeader = attackvars->myLeaderIs;
            if (tacticstype != Aggressive && myLeader != NULL && target == NULL)
            {
                SpaceObjRotImpTarg *myLeaderTarget = myLeader->attackvars.attacktarget;

                if (myLeaderTarget != NULL)
                {
                    SpaceObjRotImpTarg *formationTarget = attackFormationResolveTarget(ship, myLeaderTarget);
                    bool32 rotateAndShoot = FALSE;

                    if (formationTarget != NULL)
                    {
                        rotateAndShoot = attackFormationShouldRotateAndShoot(ship, myLeader, formationTarget);
                    }

                    if (shipstaticinfo->custshipheader.CustShipRotateAndShoot != NULL &&
                        formationTarget != NULL &&
                        rotateAndShoot)
                    {
                        shipstaticinfo->custshipheader.CustShipRotateAndShoot(ship, formationTarget, TRUE);
                        attackFormationMoveToTargetOffset(ship, myLeader, FALSE);
                        goto nextShip;
                    }
                }
                else
                {
                    soundEventShipRemove(ship);
                }

                attackFormationMoveToTargetOffset(ship, myLeader, TRUE);
                goto nextShip;
            }

            if (tacticstype != Aggressive)
            {
                pairedShip = (attackvars->myLeaderIs != NULL || attackvars->myWingmanIs != NULL);
                checkRate = pairedShip ? FIGHTER_PAIR_CHECK_RATE[tacticstype]
                                       : FIGHTER_SINGLE_CHECK_RATE[tacticstype];

                if (target != NULL)
                {
                    if ((checkRate & universe.univUpdateCounter) == (checkRate & 1))
                    {
                        bool32 threatening = isThisShipThreateningMe(target, ship);
                        if (threatening)
                        {
                            if (tacticstype == Evasive &&
                                ((universe.univUpdateCounter & FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKRATE) == FLIGHTMAN_TOKEN_EVASIVEPURE_CHECKFRAME) &&
                                ((gamerand() & 255) < FLIGHTMAN_TOKEN_EVASIVEPURE_PROB))
                            {
                                attackRecord = shipGetAttackRecord(ship, target);
                                flightman = flightmanGetRandom((FlightManProb *)attackRecord, FLIGHTMAN_EVASIVE_PURE);
                                attackvars->flightmansLeft = FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[tacticstype];
                                attackvars->attacksituation = ATTACKSITUATION_JUSTEVADING;
                                flightmanInitFunc(ship, flightman, -1);
                                goto nextShip;
                            }
                        }
                        else
                        {
                            newTarget = FindNewThreateningTarget(ship, attacktodo->attack, target);
                            if (newTarget != NULL)
                            {
                                soundEventShipRemove(ship);
                                if (pairedShip &&
                                    ((gamerand() & 255) < FLIGHTMAN_SPLIT_PROB[tacticstype]) &&
                                    (myWingman = attackvars->myWingmanIs) != NULL)
                                {
                                    myWingman->attackvars.flightmansLeft = 1;
                                    myWingman->attackvars.attacksituation = ATTACKSITUATION_SPLITTING;
                                    if (gamerand() & 1)
                                    {
                                        flightmanInitFunc(myWingman, FLIGHTMAN_IMMELMAN, 1);
                                    }
                                    else
                                    {
                                        flightmanInitFunc(myWingman, FLIGHTMAN_HARDBANK, -1);
                                    }

                                    attackvars->flightmansLeft = 1;
                                    attackvars->attacksituation = ATTACKSITUATION_SPLITTING;
                                    flightmanInitFunc(ship, FLIGHTMAN_IMMELMAN, 0);
                                }
                                else
                                {
                                    if ((gamerand() & 255) >= FLIGHTMAN_EVASIVEPURE_PROB[tacticstype])
                                    {
                                        attackRecord = shipGetAttackRecord(ship, newTarget);
                                        evasiveSubtype = attackFormationGetEvasiveSubtype(ship, newTarget);
                                        flightman = flightmanGetRandom((FlightManProb *)attackRecord, evasiveSubtype);
                                        attackvars->flightmansLeft = 1;
                                        attackvars->attacksituation = ATTACKSITUATION_EVADINGREPOS;
                                    }
                                    else
                                    {
                                        attackRecord = shipGetAttackRecord(ship, newTarget);
                                        flightman = flightmanGetRandom((FlightManProb *)attackRecord, FLIGHTMAN_EVASIVE_PURE);
                                        attackvars->flightmansLeft = FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[tacticstype];
                                        attackvars->attacksituation = ATTACKSITUATION_JUSTEVADING;
                                    }

                                    flightmanInitFunc(ship, flightman, -1);
                                }

                                {
                                    SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                                    attackvars->attacktarget = target = newTarget;
                                }
                                attackFormationResetShipTargetState(ship);
                                goto nextShip;
                            }
                        }
                    }
                }
                else
                {
                    soundEventShipRemove(ship);
                    newTarget = FindNewThreateningTarget(ship, attacktodo->attack, NULL);
                    if (newTarget != NULL)
                    {
                        if ((gamerand() & 255) >= FLIGHTMAN_EVASIVEPURE_PROB[tacticstype])
                        {
                            attackRecord = shipGetAttackRecord(ship, newTarget);
                            evasiveSubtype = attackFormationGetEvasiveSubtype(ship, newTarget);
                            flightman = flightmanGetRandom((FlightManProb *)attackRecord, evasiveSubtype);
                            attackvars->flightmansLeft = 1;
                            attackvars->attacksituation = ATTACKSITUATION_EVADINGREPOS;
                        }
                        else
                        {
                            attackRecord = shipGetAttackRecord(ship, newTarget);
                            flightman = flightmanGetRandom((FlightManProb *)attackRecord, FLIGHTMAN_EVASIVE_PURE);
                            attackvars->flightmansLeft = FLIGHTMAN_NUM_EVASIVEPURE_MANEUVERS[tacticstype];
                            attackvars->attacksituation = ATTACKSITUATION_JUSTEVADING;
                        }

                        flightmanInitFunc(ship, flightman, -1);
                        {
                            SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                            attackvars->attacktarget = target = newTarget;
                        }
                        attackFormationResetShipTargetState(ship);
                        goto nextShip;
                    }


                    {
                        SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                        attackvars->attacktarget = target = AttackClosestOpponent(ship, attacktodo->attack);
                    }
                    attackFormationResetShipTargetState(ship);
                    if (pairedShip &&
                        target != NULL &&
                        target->objtype == OBJ_ShipType &&
                        ((Ship *)target)->formationcommand == NULL &&
                        ((gamerand() & 255) < FLIGHTMAN_SANDWICH_PROB[tacticstype]) &&
                        (myWingman = attackvars->myWingmanIs) != NULL)
                    {
                        myWingman->attackvars.attacktarget = target;
                        attackFormationResetShipTargetState(myWingman);
                        myWingman->flightmanState2 = SANDWICH_RIGHT;
                        flightmanInitFunc(myWingman, FLIGHTMAN_SANDWICH, (sdword)target);
                        myWingman->attackvars.flightmansLeft = 1;
                        myWingman->attackvars.attacksituation = ATTACKSITUATION_SANDWICH;

                        ship->flightmanState2 = SANDWICH_LEFT;
                        flightmanInitFunc(ship, FLIGHTMAN_SANDWICH, (sdword)target);
                        attackvars->flightmansLeft = 1;
                        attackvars->attacksituation = ATTACKSITUATION_SANDWICH;
                        goto nextShip;
                    }

                    if (target != NULL && (attackvars->attackevents & 1))
                    {
                        flightmanInitFunc(ship, FLIGHTMAN_BARREL_ROLL, -1);
                        attackvars->flightmansLeft = 1;
                        goto nextShip;
                    }
                }
            }
        }

        if (target == NULL)
        {
            soundEventShipRemove(ship);
            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = target = AttackClosestOpponent(ship, attack);
            }
            attackFormationResetShipTargetState(ship);
        }

        if (attackvars->multipleAttackTargets)
        {
            dbgAssert(shipstaticinfo->canTargetMultipleTargets);

            if ((universe.univUpdateCounter & multipleTargetsRetargetMask) == (ship->shipID.shipNumber & multipleTargetsRetargetMask))
            {
                pickMultipleAttackTargets(ship, attack);
            }
        }

        if (target == NULL)
        {
            goto nextShip;
        }

        if (attackAndMoving)
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship, target);
            }
            else if (shipstaticinfo->custshipheader.CustShipRotateAndShoot != NULL)
            {
                shipstaticinfo->custshipheader.CustShipRotateAndShoot(ship, target, TRUE);
            }
        }
        else
        {
            if(bitTest(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE))
            {
                doKamikazeAttack(ship, target);
            }
            else if (shipstaticinfo->custshipheader.CustShipActiveAttackCommand != NULL)
            {
                shipstaticinfo->custshipheader.CustShipActiveAttackCommand(
                    ship,
                    target,
                    ship->staticinfo->bulletRange[ship->tacticstype]);
            }
        }

nextShip:
        ship->aistateattack = 0;
        attackvars->attackevents = 0;
    }

    return FALSE;

}

/*-----------------------------------------------------------------------------
    Name        : hyperspaceOutPingTimeOut
    Description : timeout cb function that determines if the ping should time out
    Inputs      : hellaPing - the ping we're evaluating
                  userID - cast to ship * for pointer to proximity sensor.
                  userData - ignore
                  bRemoveReferneces - if TRUE, remove references to pingDyingObject
    Outputs     :
    Return      : TRUE if the ping times out.
----------------------------------------------------------------------------*/
bool32 hyperspaceOutPingTimeOut(struct ping *hellaPing, udword userID, ubyte *userData, bool32 bRemoveReferences)
{
    if (bRemoveReferences)
    {
        if ((SpaceObj *)userID == pingDyingObject)
        {
            return(TRUE);
        }
        return(FALSE);
    }

    if(((Ship *)userID)->flags & SOF_Hyperspace)
    {
        return(TRUE);
    }
    return(FALSE);
}
/*-----------------------------------------------------------------------------
    Name        : hyperspaceInPingTimeOut
    Description : timeout cb function that determines if the ping should time out
    Inputs      : hellaPing - the ping we're evaluating
                  userID - cast to ship * for pointer to proximity sensor.
                  userData - ignore
                  bRemoveReferneces - if TRUE, remove references to pingDyingObject
    Outputs     :
    Return      : TRUE if the ping times out.
----------------------------------------------------------------------------*/
 bool32 hyperspaceInPingTimeOut(struct ping *hellaPing, udword userID, ubyte *userData, bool32 bRemoveReferences)
{
    if (bRemoveReferences)
    {
        if ((SpaceObj *)userID == pingDyingObject)
        {
            return(TRUE);
        }
        return(FALSE);
    }
    if(((Ship *)userID)->shipSinglePlayerGameInfo->shipHyperspaceState == SHIPHYPERSPACE_NONE)
    {
        return(TRUE);
    }
    return(FALSE);
}

static bool32 mpHyperspaceShipHasDefenders(Ship *ship)
{
    Node *head;

    if (ship->shiptype == kAdvanceSupportFrigate || ship->shiptype == tAdvanceSupportFrigate)
    {
        head = universe.mainCommandLayer.todolist.head;
        while (head != NULL)
        {
            CommandToDo *command = (CommandToDo *)listGetStructOfNode(head);
            if (command->ordertype.order == COMMAND_DOCK)
            {
                sdword i;
                for (i = 0; i < command->selection->numShips; i++)
                {
                    if (command->selection->ShipPtr[i]->dockvars.dockship == ship &&
                        ship->collMyBlob != NULL)
                    {
                        return TRUE;
                    }
                }
            }
            head = head->next;
        }
        return FALSE;
    }

    if (ship->staticinfo->shipclass != CLASS_Mothership)
    {
        return FALSE;
    }

    head = universe.mainCommandLayer.todolist.head;
    while (head != NULL)
    {
        CommandToDo *command = (CommandToDo *)listGetStructOfNode(head);
        if (command->ordertype.order == COMMAND_LAUNCHSHIP &&
            command->launchship.receiverShip == ship &&
            command->selection->ShipPtr[0]->collMyBlob != NULL)
        {
            return TRUE;
        }
        head = head->next;
    }

    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : processMpHyperspaceingToDo
    Description : processes the hyperspaceing command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define HW_ETHER_WAIT       TW_MULTIPLAYER_HYPERSPACE_WAIT_IN_ETHER_TIME

bool32 processMpHyperspaceingToDo(CommandToDo *movetodo)
{
    sdword i,j,donehsflag;
    vector shipheading,othervector;
    real32 dotproduct;
    real32 timefraction;
    sdword shipslice;
    bool32 dontgoyet=FALSE;
    Ship *ship;
    for(i=0;i<movetodo->selection->numShips;i++)
    {
       movetodo->selection->ShipPtr[i]->posinfo.isMoving = TRUE;
    }
    switch(movetodo->hyperspaceState)
    {
    case COM_HYPERSPACE_START:
        //check if any ASF's in selection
        //if yes, we won't hs out until all slots on asf are full
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            ship = movetodo->selection->ShipPtr[i];
            if(aitrackZeroRotationAnywhere(ship)
               && aitrackZeroVelocity(ship))
            {
                ship->rotinfo.rotspeed.x=0.0f;
                ship->rotinfo.rotspeed.y=0.0f;
                ship->rotinfo.rotspeed.z=0.0f;
                ship->posinfo.velocity.x=0.0f;
                ship->posinfo.velocity.y=0.0f;
                ship->posinfo.velocity.z=0.0f;
                continue;
            }
            dontgoyet=TRUE;

        }
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            if(mpHyperspaceShipHasDefenders(movetodo->selection->ShipPtr[i]))
            {
                return FALSE;
            }
        }
        if(dontgoyet)
        {
            return FALSE;
        }
        //set to hyperspace out!
        spHyperspaceSelectionIn(movetodo->selection);
        //setup timer to count down...
        movetodo->hyperSpaceingTime = universe.totaltimeelapsed;
        movetodo->hyperspaceState=COM_HYPERSPACE_OUT;

        //reset ping flag status
        for(j=0;j<movetodo->selection->numShips;j++)
        {
            movetodo->selection->ShipPtr[j]->hyperspacePing = FALSE;
        }
        return FALSE;

    case COM_HYPERSPACE_OUT:
        //check if all hyperspaced out...
        timefraction = universe.totaltimeelapsed - movetodo->hyperSpaceingTime;
        shipslice = (sdword) (timefraction/TW_PING_MAX_DISPERSAL)*movetodo->selection->numShips;
        if(shipslice-movetodo->selection->numShips == 1 ||
            shipslice > movetodo->selection->numShips )
        {
            shipslice = movetodo->selection->numShips;
        }
        else if(shipslice < 1)
        {
            shipslice = 1;
        }
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            for(j=0;j<shipslice;j++)
            {
                if(!movetodo->selection->ShipPtr[j]->hyperspacePing)
                {
                    ping *newPing;

                    movetodo->selection->ShipPtr[j]->hyperspacePing=TRUE;
                    //create ping HERE!
                    newPing = pingCreate(NULL, (SpaceObj *)movetodo->selection->ShipPtr[j], hyperspaceOutPingTimeOut, NULL, 0, (udword)movetodo->selection->ShipPtr[j]);
                    newPing->c = TW_HW_PING_COLOUR_OUT;
                    newPing->size = TW_HW_PING_MAX_SIZE_OUT;
                    newPing->minScreenSize = primScreenToGLScaleX(2);
                    newPing->minSize = TW_HW_PING_MIN_SIZE_OUT;
                    newPing->pingDuration = 1.0f / TW_HW_PING_RATE_OUT;
                    newPing->interPingPause = 0.25;
                    newPing->TOMask         = PTOM_Hyperspace;

                    ///////////////////////////////
                    //PUT SPEECH EVENTS HERE!
                    ///////////////////////////////
                }
            }
            if(!(movetodo->selection->ShipPtr[0]->flags & SOF_Hyperspace))
            {
                return(FALSE);
            }
        }

        //tell code that all ships have no pings...so we can create them for hyping in..
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            movetodo->selection->ShipPtr[i]->hyperspacePing=FALSE;
        }

        movetodo->hyperSpaceingTime = universe.totaltimeelapsed+HW_ETHER_WAIT;
        movetodo->hyperspaceState=COM_HYPERSPACE_WAIT;
        break;
    case COM_HYPERSPACE_WAIT:
        //check if done in the ether
        if(movetodo->hyperSpaceingTime<= universe.totaltimeelapsed)
        {
            //hyperspaced out!
            hvector movetoplace;
            movetoplace.x = movetodo->move.destination.x;
            movetoplace.y = movetodo->move.destination.y;
            movetoplace.z = movetodo->move.destination.z;

            matGetVectFromMatrixCol3(shipheading,movetodo->selection->ShipPtr[0]->rotinfo.coordsys);
            vecSet(othervector,1.0f,0.0f,0.0f);

            dotproduct = vecDotProduct(shipheading,othervector);
            if(dotproduct >= 1.0)
                dotproduct = 1.0f;
            else if(dotproduct <=-1.0f)
                dotproduct = -1.0f;

            movetoplace.w = RAD_TO_DEG(acos(dotproduct));

            vecSet(othervector,0.0f,1.0f,0.0f);
            dotproduct = vecDotProduct(shipheading,othervector);
            if(dotproduct < 0)
            {
                movetoplace.w = 360-movetoplace.w;
            }

            spHyperspaceSetSelectionDestination(movetodo->selection, &movetoplace);
            movetodo->hyperspaceState=COM_HYPERSPACE_IN;
            movetodo->hyperSpaceingTime = universe.totaltimeelapsed;
        }
        break;
    case COM_HYPERSPACE_IN:
        //check if all hyperspaced in...
        donehsflag = TRUE;
        timefraction = universe.totaltimeelapsed - movetodo->hyperSpaceingTime;
        shipslice = (sdword) (timefraction/TW_PING_MAX_DISPERSAL)*movetodo->selection->numShips;
        if(shipslice-movetodo->selection->numShips == 1 ||
            shipslice > movetodo->selection->numShips )
        {
            shipslice = movetodo->selection->numShips;
        }
        else if(shipslice < 1)
        {
            shipslice = 1;
        }
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            if(movetodo->selection->ShipPtr[i]->playerowner != universe.curPlayerPtr)
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_COMM_F_HyperspaceDetected, SOUND_EVENT_DEFAULT, &movetodo->selection->ShipPtr[i]->posinfo.position);
            }


            //do hs ping for ALL ships
            for(j=0;j<shipslice;j++)
            {
                if(!movetodo->selection->ShipPtr[j]->hyperspacePing)
                {
                    ping *newPing;
                    movetodo->selection->ShipPtr[j]->hyperspacePing=TRUE;
                    //create ping HERE!
                    newPing = pingCreate(NULL, (SpaceObj *)movetodo->selection->ShipPtr[j], hyperspaceInPingTimeOut, NULL, 0, (udword)movetodo->selection->ShipPtr[j]);
                    newPing->c = TW_HW_PING_COLOUR_IN;
                    newPing->size = TW_HW_PING_MAX_SIZE_IN;
                    newPing->minScreenSize = primScreenToGLScaleX(2);
                    newPing->minSize = TW_HW_PING_MIN_SIZE_IN;
                    newPing->pingDuration = 1.0f / TW_HW_PING_RATE_IN;
                    newPing->interPingPause = 0.25;
                    newPing->TOMask         = PTOM_Hyperspace;

                    ///////////////////////////////
                    //PUT SPEECH EVENTS HERE!
                    ///////////////////////////////

                }
            }
            if((movetodo->selection->ShipPtr[i]->shipSinglePlayerGameInfo->shipHyperspaceState != SHIPHYPERSPACE_NONE))
            {
                donehsflag = FALSE;
            }
            else
            {
                clSetAttribute((sdword)movetodo->selection->ShipPtr[i],0,6);
            }
        }
        if(donehsflag == FALSE)
        {
            return FALSE;
        }
        for(j=0;j<movetodo->selection->numShips;j++)
        {
            movetodo->selection->ShipPtr[j]->hyperspacePing=FALSE;
        }
        //all out..
        for(i=0;i<movetodo->selection->numShips;i++)
        {
            clSetAttribute((sdword)movetodo->selection->ShipPtr[i],0,6);
        }
        return(TRUE);
        break;
    }

    return(FALSE);
}

real32 hyperspaceCost(real32 distance,SelectCommand *selection)
{
    sdword i;
    Ship *ship;
    real32 tempcost;
    real32 cost=0.0f;
    for(i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        if(distance < TW_HyperSpaceCostStruct[ship->shiptype].minDistance)
        {
            tempcost = TW_HyperSpaceCostStruct[ship->shiptype].min;
        }
        else
        {
            distance-= TW_HyperSpaceCostStruct[ship->shiptype].minDistance;
            tempcost = TW_HyperSpaceCostStruct[ship->shiptype].min +
                TW_HyperSpaceCostStruct[ship->shiptype].distanceSlope*distance;
        }
        if(tempcost > TW_HyperSpaceCostStruct[ship->shiptype].max)
        {
            tempcost = TW_HyperSpaceCostStruct[ship->shiptype].max;
        }
        cost+= tempcost;
    }
    return(cost);
}


/*-----------------------------------------------------------------------------
    Name        : processMoveToDo
    Description : processes the move command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 processMoveToDo(CommandToDo *movetodo,bool32 passiveAttacked)
{
    sdword i;
    SelectCommand *selection = movetodo->selection;
    sdword numShips = selection->numShips;
    const AttackMovePathState *movePath = clMovePathStateConst(movetodo);
    Ship *ship;
    bool32 thisShipDone;
    bool32 allDone = TRUE;
    ShipStaticInfo *shipstatic;
    udword aiflags;
    sdword j;
    bool32 dontrotate;
    real32 MOVE_ARRIVE_TOLERANCE;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        shipstatic = (ShipStaticInfo *)ship->staticinfo;

        if(ship->shiptype == sSentinel)
        {
            if(ship->flags & SOF_Slaveable)
            {
                dontrotate = TRUE;
            }
            else
            {
                dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
            }
        }
        else
        {
            if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
            {
                dontrotate = TRUE;
            }
            else
            {
                dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
            }
        }

        thisShipDone = FALSE;

        switch (ship->aistatecommand)
        {
        case 0:
                if ((ship->isDodging) || (universe.totaltimeelapsed < ship->DodgeTime))
                {
                    //ship is dodging...be a little more forgiving
                    MOVE_ARRIVE_TOLERANCE = 250.0f;
                }
                else
                {
                    MOVE_ARRIVE_TOLERANCE = 50.0f;
                }
                if (MoveReachedDestinationVariable(ship,&ship->moveTo,MOVE_ARRIVE_TOLERANCE))
                {
                    ship->aistatecommand = 1;
                    if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                    {
                        for(j=1;j<movetodo->selection->numShips;j++)
                        {
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                        }
                    }

                    goto state1;
                }
                else
                {
                    aiflags = (dontrotate) ? (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured) : (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FirstPointInDirectionFlying|AISHIP_PointInDirectionFlying|AISHIP_CarTurn);
                    if (aishipFlyToPointAvoidingObjs(ship,&ship->moveTo,aiflags,(ship->shiptype == HeadShotAsteroid) ? (-tacticsGetShipsMaxVelocity(ship)) : -20.0f) & AISHIP_FLY_OBJECT_IN_WAY)
                    {
                        ship->aistatecommand = 1;
                        if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                        {
                            for(j=1;j<movetodo->selection->numShips;j++)
                            {
                                vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                                vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                            }
                        }

                        goto state1;
                    }
                }
                break;
state1:
            case 1:
                if ((dontrotate) ||
                    ((shipstatic->pitchdescend != 0.0f) && (ship->aidescend < 0.0f)))   // -ve aidescend indicates ship is primarily moving down
                {
                    thisShipDone = aitrackSteadyShipDriftOnly(ship);
                }
                else
                {
                    vector trackthisheading;
                    vecSub(trackthisheading,ship->moveTo,ship->moveFrom);
                    vecNormalize(&trackthisheading);
                    thisShipDone = (aitrackSteadyShipDriftOnly(ship) & aitrackHeadingWithBank(ship,&trackthisheading,FLYSHIP_HEADINGACCURACY,shipstatic->sinbank));
                }
                if (thisShipDone &&
                    movePath->pathState == ATTACKMOVE_PATH_ONCE &&
                    movePath->currentPointIndex >= movePath->pointCount - 1)
                {
                    vecZeroVector(ship->moveTo);
                    vecZeroVector(ship->moveFrom);
                }
                break;
        }

        allDone &= thisShipDone;
    }

    return allDone;
}

/*-----------------------------------------------------------------------------
    Name        : processMoveLeaderToDo
    Description : processes the move command, but only for the leader (this routine
                  is only used for ships in formation moving, and the other ships maintain
                  formation around the leader)
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 processMoveLeaderToDo(CommandToDo *movetodo,bool32 passiveAttacked)
{
    Ship *ship;
    SelectCommand *selection = movetodo->selection;
    const AttackMovePathState *movePath = clMovePathStateConst(movetodo);
    ShipStaticInfo *shipstatic;
    bool32 dontrotate;
    udword aiflags;
    sdword j;
    udword moveresult;
    real32 MOVE_ARRIVE_TOLERANCE;
    vector destination;

    dbgAssert(selection->numShips >= 1);

    ship = selection->ShipPtr[0];
    clGetMoveDestination(movetodo, &destination);
    shipstatic = (ShipStaticInfo *)ship->staticinfo;

    if(ship->shiptype == sSentinel)
    {
        if(ship->flags & SOF_Slaveable)
        {
            dontrotate = TRUE;
        }
        else
        {
            dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
        }
    }
    else
    {
        dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
    }

    switch (ship->aistatecommand)
    {
        case 0:
            if ((ship->isDodging) || (universe.totaltimeelapsed < ship->DodgeTime))
            {
                //ship is dodging...be a little more forgiving
                MOVE_ARRIVE_TOLERANCE = 250.0f;
            }
            else
            {
                MOVE_ARRIVE_TOLERANCE = 50.0f;
            }
            if (MoveReachedDestinationVariable(ship,&destination,MOVE_ARRIVE_TOLERANCE))
            {
                ship->aistatecommand = 1;

                if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                {
                    for(j=1;j<movetodo->selection->numShips;j++)
                    {
                        vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                        vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                    }
                }

                goto state1;
            }
            else
            {
                aiflags = (dontrotate) ? (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured) : (AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_FirstPointInDirectionFlying|AISHIP_PointInDirectionFlying|AISHIP_CarTurn);
                moveresult = aishipFlyToPointAvoidingObjs(ship,&destination,aiflags,movetodo->formation.travelvel);
                if (moveresult & AISHIP_FLY_OBJECT_IN_WAY)
                {
                    ship->aistatecommand = 1;
                    if(movetodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                    {
                        for(j=1;j<movetodo->selection->numShips;j++)
                        {
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveTo);
                            vecZeroVector(movetodo->selection->ShipPtr[j]->moveFrom);
                        }
                    }

                    goto state1;
                }
            }
            break;
state1:
        case 1:
            if ((dontrotate) ||
                ((shipstatic->pitchdescend != 0.0f) && (ship->aidescend < 0.0f)))   // -ve aidescend indicates ship is primarily moving down
            {
                if (aitrackSteadyShipDriftOnly(ship))
                {
                    if (movePath->pathState == ATTACKMOVE_PATH_ONCE &&
                        movePath->currentPointIndex >= movePath->pointCount - 1)
                    {
                        vecZeroVector(ship->moveTo);
                        vecZeroVector(ship->moveFrom);
                    }
                    return TRUE;
                }
                return FALSE;
            }
            else
            {
                vector trackthisheading;
                vecSub(trackthisheading,ship->moveTo,ship->moveFrom);
                vecNormalize(&trackthisheading);
                if (aitrackSteadyShipDriftOnly(ship) & aitrackHeadingWithBank(ship,&trackthisheading,FLYSHIP_HEADINGACCURACY,shipstatic->sinbank))
                {
                    if (movePath->pathState == ATTACKMOVE_PATH_ONCE &&
                        movePath->currentPointIndex >= movePath->pointCount - 1)
                    {
                        vecZeroVector(ship->moveTo);
                        vecZeroVector(ship->moveFrom);
                    }
                    return TRUE;
                }
                return FALSE;
            }
    }
    return FALSE;
}

// Cataclysm-specific: move-attack path follower for the retail attack-move overlay.
bool32 processAttackMoveToDo(CommandToDo *attacktodo,bool32 passiveAttacked,sdword shipIndex,bool32 useTravelVelocity)
{
    SelectCommand *selection = attacktodo->selection;
    Ship *ship = selection->ShipPtr[shipIndex];
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    AttackMovePathState *attackMove = (AttackMovePathState *)&attacktodo->move;
    bool32 dontrotate;
    udword aiflags;
    sdword previousPointIndex = 0;
    real32 moveArriveTolerance;
    real32 moveVelocity;
    udword moveResult;
    sdword i;

    if (attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
    {
        dontrotate = TRUE;
    }
    else if (salCapIsCapturing(ship))
    {
        dontrotate = TRUE;
    }
    else
    {
        dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
    }

    if (ship->aistatecommand)
    {
        if (ship->aistatecommand != 1)
        {
            return FALSE;
        }
    }
    else
    {
        if ((ship->isDodging) || (universe.totaltimeelapsed < ship->DodgeTime))
        {
            moveArriveTolerance = 250.0f;
        }
        else if ((attackMove->pathState != ATTACKMOVE_PATH_ONCE) ||
                 (attackMove->currentPointIndex != attackMove->pointCount - 1))
        {
            moveArriveTolerance = 800.0f;
        }
        else
        {
            moveArriveTolerance = 50.0f;
        }

        {
            bool32 reachedDestination = MoveReachedDestinationVariable(ship,&ship->moveTo,moveArriveTolerance);
            if (reachedDestination)
            {
                switch (attackMove->pathState)
                {
                    case ATTACKMOVE_PATH_ONCE:
                        attackMove->currentPointIndex++;
                        if (attackMove->currentPointIndex < attackMove->pointCount)
                        {
                            previousPointIndex = attackMove->pointCount - 1;
                            clSetShipMoveToPositions(selection,
                                                     attackMove->points[previousPointIndex],
                                                     attackMove->points[attackMove->currentPointIndex]);
                            return FALSE;
                        }
                        ship->aistatecommand = 1;
                        vecZeroVector(ship->moveTo);
                        vecZeroVector(ship->moveFrom);
                        if (attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                        {
                            for (i = 1; i < selection->numShips; i++)
                            {
                                vecZeroVector(selection->ShipPtr[i]->moveTo);
                                vecZeroVector(selection->ShipPtr[i]->moveFrom);
                            }
                        }
                        return aitrackSteadyShipDriftOnly(ship) &&
                               (attackMove->pathState == ATTACKMOVE_PATH_ONCE) &&
                               (attackMove->currentPointIndex >= attackMove->pointCount - 1);

                    case ATTACKMOVE_PATH_LOOP:
                        attackMove->currentPointIndex++;
                        if (attackMove->currentPointIndex < attackMove->pointCount)
                        {
                            previousPointIndex = (attackMove->currentPointIndex != 0)
                                               ? (attackMove->currentPointIndex - 1)
                                               : (attackMove->pointCount - 1);
                        }
                        else
                        {
                            attackMove->currentPointIndex = 0;
                            previousPointIndex = attackMove->pointCount - 1;
                        }
                        clSetShipMoveToPositions(selection,
                                                 attackMove->points[previousPointIndex],
                                                 attackMove->points[attackMove->currentPointIndex]);
                        return FALSE;

                    case ATTACKMOVE_PATH_PING_FORWARD:
                        attackMove->currentPointIndex++;
                        if (attackMove->currentPointIndex < attackMove->pointCount)
                        {
                            previousPointIndex = attackMove->currentPointIndex - 1;
                        }
                        else
                        {
                            attackMove->pathState = ATTACKMOVE_PATH_PING_BACK;
                            attackMove->currentPointIndex = attackMove->pointCount - 2;
                            previousPointIndex = attackMove->pointCount - 1;
                        }
                        clSetShipMoveToPositions(selection,
                                                 attackMove->points[previousPointIndex],
                                                 attackMove->points[attackMove->currentPointIndex]);
                        return FALSE;

                    case ATTACKMOVE_PATH_PING_BACK:
                        attackMove->currentPointIndex--;
                        if (attackMove->currentPointIndex >= 0)
                        {
                            previousPointIndex = attackMove->currentPointIndex + 1;
                        }
                        else
                        {
                            attackMove->pathState = ATTACKMOVE_PATH_PING_FORWARD;
                            attackMove->currentPointIndex = 1;
                        }
                        clSetShipMoveToPositions(selection,
                                                 attackMove->points[previousPointIndex],
                                                 attackMove->points[attackMove->currentPointIndex]);
                        return FALSE;

                    default:
                        return FALSE;
                }
            }
        }

        aiflags = dontrotate
                ? (AISHIP_DontFlyToObscuredPoints | AISHIP_ReturnImmedIfPointObscured)
                : (AISHIP_DontFlyToObscuredPoints | AISHIP_ReturnImmedIfPointObscured |
                   AISHIP_FirstPointInDirectionFlying | AISHIP_PointInDirectionFlying | AISHIP_CarTurn);
        moveVelocity = useTravelVelocity ? attacktodo->formation.travelvel : -20.0f;
        moveResult = aishipFlyToPointAvoidingObjsFunc(ship,&ship->moveTo,aiflags,moveVelocity,0);
        if (!(moveResult & AISHIP_FLY_OBJECT_IN_WAY))
        {
            return FALSE;
        }

        ship->aistatecommand = 1;
        vecZeroVector(ship->moveTo);
        vecZeroVector(ship->moveFrom);
        if (attacktodo->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
        {
            for (i = 1; i < selection->numShips; i++)
            {
                vecZeroVector(selection->ShipPtr[i]->moveTo);
                vecZeroVector(selection->ShipPtr[i]->moveFrom);
            }
        }
    }

    return aitrackSteadyShipDriftOnly(ship) &&
           (attackMove->pathState == ATTACKMOVE_PATH_ONCE) &&
           (attackMove->currentPointIndex >= attackMove->pointCount - 1);
}

void SalCapCancelOrder(Ship *ship);

void RemoveShipFromSpecial(Ship *shiptoremove)
{
    if (shiptoremove->specialFlags & SPECIAL_IsASalvager)
    {
        SalCapCancelOrder(shiptoremove);
        SalCapOrderChangedCleanUp(shiptoremove);
    }
    else if (shiptoremove->shiptype == tMinelayerCorvette)
    {
        MinelayerCorvetteOrderChangedCleanUp(shiptoremove);
    }
    else if(shiptoremove->shiptype == sRepairDroid)
    {
        RepairCorvetteOrderChanged(shiptoremove);
    }
    else if(shiptoremove->shiptype == kHeavyCorvette)
    {
        heavyCorvetteOrderChanged(shiptoremove);
    }
    else if(shiptoremove->shiptype == kAdvanceSupportFrigate)
    {
        if(shiptoremove->rceffect != NULL)
        {
            //need to turn off effect
            stopRepairEffect(shiptoremove);
        }
    }
    else if (shiptoremove->shiptype == sHiveFrigate || shiptoremove->shiptype == bHiveFrigate)
    {
        HiveFrigateCancelSpecial(shiptoremove);
    }
}

void SpecialCleanup(CommandToDo *command)
{
    sdword i;
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;

    //for all special ships using the repair effect
    for (i=0;i<numShips;i++)
    {
        if(selection->ShipPtr[i]->rceffect != NULL)
        {
            if(selection->ShipPtr[i]->staticinfo->repairBeamCapable)
            {
                //ship must be using the repair effect
                stopRepairEffect(selection->ShipPtr[i]);
            }
        }
        if ((selection->ShipPtr[i]->shiptype == sHiveFrigate || selection->ShipPtr[i]->shiptype == bHiveFrigate) &&
            selection->ShipPtr[i]->staticinfo->custshipheader.CustShipHaltFilter != NULL)
        {
            selection->ShipPtr[i]->staticinfo->custshipheader.CustShipHaltFilter(selection->ShipPtr[i]);
        }
        else
        {
            RemoveShipFromSpecial(selection->ShipPtr[i]);
        }
    }

    if (command->specialtargets)
    {
        memFree(command->specialtargets);
        command->specialtargets = NULL;
    }
}

void RemoveShipFromResourceCollecting(Ship *shiptoremove)
{
    if (shiptoremove->rceffect != NULL)
    {
        TurnHarvestEffectOff(shiptoremove);
    }
    shiptoremove->rcstate1 = 0;
    shiptoremove->rcstate2 = 0;
}

void CollectResourceCleanup(CommandToDo *command)
{
    sdword i;
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromResourceCollecting(selection->ShipPtr[i]);
    }
}

static void clClearShipCommandState(CommandToDo *command)
{
    SelectCommand *selection = command->selection;
    Ship *ship;

    if (selection->numShips <= 0)
    {
        return;
    }

    ship = selection->ShipPtr[0];
    if (ship->dockvars.dockPointIndex != -1 &&
        ship->dockvars.dockship != NULL &&
        ship->staticinfo->shipclass != CLASS_Component)
    {
        Ship *dockwith = ship->dockvars.dockship;
        DockPointActivity *dockPointActivity = dockwith->dockvars.dockPointActivity;
        DockStaticInfo *dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
        sdword dockPointIndex = ship->dockvars.dockPointIndex;

        if (dockPointActivity != NULL)
        {
            DockPointType dockPointType = dockStaticInfo->dockstaticpoints[dockPointIndex].type;

            if (dockPointActivity->reserved[dockPointIndex])
            {
                dockPointActivity->reserved[dockPointIndex] = 0;
                if (dockPointActivity->numDockedOfType[dockPointType] > 0)
                {
                    dockPointActivity->numDockedOfType[dockPointType]--;
                }
            }
        }

        ship->dockvars.dockPointIndex = -1;
        ship->dockvars.dockship = NULL;
    }

    ship->flags2 &= ~SOF2_BayDocked;
    ship->specialFlags1C8 &= ~BIT12;
    ship->flags &= ~SOF_AvoidanceHidden;
    ship->specialFlags2 &= ~0x40u;
}

void FreeCommandToDoContents(CommandToDo *command)
{
    sdword i;

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        memFree(command->protect);
        bitClear(command->ordertype.attributes,COMMAND_IS_PROTECTING);
        command->protect = NULL;
    }

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        FormationCleanup(command);
        bitClear(command->ordertype.attributes,COMMAND_IS_FORMATION);
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        dbgAssert(command->ordertype.order != COMMAND_ATTACK);
        AttackCleanup(command);
        dbgAssert(command->attack);
        memFree(command->attack);
        bitClear(command->ordertype.attributes,COMMAND_IS_PASSIVEATTACKING);
        command->attack = NULL;
    }

    RemoveShipsFromCommand(command);

    switch (command->ordertype.order)
    {
        case COMMAND_ATTACK:
            AttackCleanup(command);
            dbgAssert(command->attack);
            memFree(command->attack);
            command->attack = NULL;
            break;

        case COMMAND_MOVE:
            MoveCleanup(command);
            break;
        case COMMAND_MP_HYPERSPACEING:
            MpHyperspaceCleanup(command);
            break;

        case COMMAND_NULL:
        case COMMAND_HALT:
        case COMMAND_BUILDINGSHIP:
        case COMMAND_RESEARCH:
            break;

        case COMMAND_COLLECTRESOURCE:
            CollectResourceCleanup(command);
            break;

        case COMMAND_MILITARYPARADE:
            dbgAssert(command->militaryParade);
            FreeMilitaryParadeContents(command->militaryParade);
            memFree(command->militaryParade);
            command->militaryParade = NULL;
            break;

        case COMMAND_LAUNCHSHIP:
            clClearShipCommandState(command);
            break;

        case COMMAND_DOCK:
            DockCleanup(command);
            break;

        case COMMAND_SPECIAL:
            for (i=0;i<command->selection->numShips;i++)
            {
                if (command->selection->ShipPtr[i]->staticinfo->custshipheader.CustShipSpecialOrderCleanup != NULL)
                {
                    ((void (__cdecl *)(Ship *))command->selection->ShipPtr[i]->staticinfo->custshipheader.CustShipSpecialOrderCleanup)(command->selection->ShipPtr[i]);
                }
            }
            if (command->specialtargets != NULL)
            {
                memFree(command->specialtargets);
                command->specialtargets = NULL;
            }
            break;

        case COMMAND_LINK:
        case COMMAND_LINKED_LAUNCH:
        case COMMAND_EXTERNALCONSTRUCTION:
        case COMMAND_SIEGECANNON:
            clClearSelectionLinkState(command->selection);
            break;

        default:
            dbgAssert(FALSE);   // should never get here
            break;
    }
    memFree(command->selection);
    command->selection = NULL;
}

void FreeLastOrder(CommandToDo *command)
{
    sdword dontClearHoldingPattern = FALSE;
    sdword i;

    // deliberately do not call
    // RemoveShipsFromCommand(command)
    // because last order is being freed in order to be changed, the actual command is not freed.

    switch (command->ordertype.order)
    {
        case COMMAND_ATTACK:
            AttackCleanup(command);
            dbgAssert(command->attack);
            memFree(command->attack);
            command->attack = NULL;
            dontClearHoldingPattern = TRUE;
            break;

        case COMMAND_MOVE:
            MoveCleanup(command);
            break;
        case COMMAND_MP_HYPERSPACEING:
            MpHyperspaceCleanup(command);
            break;

        case COMMAND_NULL:
        case COMMAND_HALT:
        case COMMAND_BUILDINGSHIP:
        case COMMAND_RESEARCH:
            break;

        case COMMAND_LINK:
        case COMMAND_LINKED_LAUNCH:
        case COMMAND_EXTERNALCONSTRUCTION:
        case COMMAND_SIEGECANNON:
            clClearSelectionLinkState(command->selection);
            break;

        case COMMAND_COLLECTRESOURCE:
            CollectResourceCleanup(command);
            break;

        case COMMAND_MILITARYPARADE:
            dbgAssert(command->militaryParade);
            FreeMilitaryParadeContents(command->militaryParade);
            memFree(command->militaryParade);
            command->militaryParade = NULL;
            break;

        case COMMAND_LAUNCHSHIP:
            clClearShipCommandState(command);
            break;

        case COMMAND_DOCK:
            DockCleanup(command);
            break;

        case COMMAND_SPECIAL:
            for (i=0;i<command->selection->numShips;i++)
            {
                if (command->selection->ShipPtr[i]->staticinfo->custshipheader.CustShipSpecialOrderCleanup != NULL)
                {
                    ((void (__cdecl *)(Ship *))command->selection->ShipPtr[i]->staticinfo->custshipheader.CustShipSpecialOrderCleanup)(command->selection->ShipPtr[i]);
                }
            }
            if (command->specialtargets != NULL)
            {
                memFree(command->specialtargets);
                command->specialtargets = NULL;
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
    if(!dontClearHoldingPattern)
    {
        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
    }
}

void ClearPassiveAttacking(CommandToDo *command)
{
    bitClear(command->ordertype.attributes,COMMAND_IS_PASSIVEATTACKING);
    AttackCleanup(command);
    dbgAssert(command->attack);
    memFree(command->attack);
    command->attack = NULL;

/*
    Locked formation cleanup not needed because a check is done in AttackCleanup to
    see if a formation is locked and unlocks it if need be...
*/

/*
    if(command->ordertype.order == COMMAND_NULL)
    {
        //command is a NULL command
        if(command->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            //command is a NULL command and in FORMATION
            //There for the formation had to be locked
            dbgAssert(command->formation.formationLocked);

            //unlock the formation
            unlockFormation(command);
        }
    }
*/
}

void ClearProtecting(CommandToDo *command)
{
    bitClear(command->ordertype.attributes,COMMAND_IS_PROTECTING);
    dbgAssert(command->protect);
    memFree(command->protect);
    command->protect = NULL;

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        if ((command->formation.formationtype == SPHERE_FORMATION) && (command->selection->numShips > 1))
        {
            formationContentHasChanged(command);
        }
        else if (command->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
        {
            command->formation.formationtype = FORCEFIELD_WALL_FORMATION;
            formationContentHasChanged(command);
        }
    }
}

void RemoveShipsFromDoingStuff(CommandLayer *comlayer,SelectCommand *selectcom)
{
    sdword i, j;
    ShipPtr shiptoremove;
    Node *todonode;
    CommandToDo *todo;
    SelectCommand *todoselection;

    for (i=0;i<selectcom->numShips;i++)
    {
        shiptoremove = selectcom->ShipPtr[i];
        if (shiptoremove->specialFlags1C8 & CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY)
        {
            rowRemoveShipFromGettingOutOfWay(shiptoremove);
        }
    }

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);
        todoselection = todo->selection;

        for (i=0;i<selectcom->numShips;i++)
        {
            shiptoremove = selectcom->ShipPtr[i];

            if (todo->ordertype.attributes & COMMAND_IS_FORMATION)
            {
                sdword removedShipFromFormation;
                if ((removedShipFromFormation = formationRemoveShipFromSelection(todo,shiptoremove)) >= 0)
                {
                    RemoveShipFromCommand(shiptoremove);
                    RemoveShipFromFormation(shiptoremove);

                    if ((todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) || (todo->ordertype.order == COMMAND_ATTACK))
                    {
                        RemoveShipFromAttacking(shiptoremove);
                        RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                    }

                    if (todo->ordertype.order == COMMAND_DOCK)
                    {
                        dockWorkerClampFunc(shiptoremove);
                    }

                    if (todo->ordertype.order == COMMAND_MOVE)
                    {
                        RemoveShipFromMoving(shiptoremove);
                    }

                    if(todo->ordertype.order == COMMAND_SPECIAL)
                    {
                        if (shiptoremove->staticinfo->custshipheader.CustShipSpecialOrderCleanup != NULL)
                        {
                            ((void (__cdecl *)(Ship *))shiptoremove->staticinfo->custshipheader.CustShipSpecialOrderCleanup)(shiptoremove);
                        }
                    }

                    if(todo->ordertype.order == COMMAND_MP_HYPERSPACEING)
                    {
                        removeShipFromMpHyperspaceing(shiptoremove);
                    }

                    if (todoselection->numShips == 0)
                    {
                        // Retail (0x4449da-0x4449ec) only frees the CommandToDo here when
                        // order != COMMAND_LINK(11) && order != COMMAND_LINKED_LAUNCH(12); our
                        // code was freeing unconditionally, which could free a todo still
                        // referenced elsewhere for those two order types.
                        if (todo->ordertype.order != COMMAND_LINK &&
                            todo->ordertype.order != COMMAND_LINKED_LAUNCH)
                        {
                            Node *nextnode = todonode->next;
                            FreeCommandToDoContents(todo);
                            listDeleteNode(todonode);
                            todonode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    else if (todoselection->numShips < 2)
                    {
                        // Retail (0x4449fa) branches on numTargets/numShips >= 2, not on
                        // ABSOLUTE_MIN_SHIPS_IN_FORMATION (which is 1 and made this branch dead
                        // code). With the dead branch never taken, a formation reduced to exactly
                        // 1 remaining ship fell through to the numShips>=2 case below and called
                        // formationContentHasChanged -> FormationCalculateOffsets on a 1-ship
                        // formation, which retail never does; this is the root cause of the
                        // FormationCalculateOffsets AV. Retail (0x444a12) also gates this branch's
                        // detach loop on order != COMMAND_LINK(11) && order != COMMAND_LINKED_LAUNCH(12),
                        // not COMMAND_EXTERNALCONSTRUCTION/COMMAND_SIEGECANNON.
                        if (todo->ordertype.order != COMMAND_LINK &&
                            todo->ordertype.order != COMMAND_LINKED_LAUNCH)
                        {
                            for (j = 0; j < todoselection->numShips; j++)
                            {
                                RemoveShipFromFormation(todoselection->ShipPtr[j]);
                            }
                            if (todo->dock.dockType != 0)
                            {
                                memFree((void *)todo->dock.dockType);
                                todo->dock.dockType = 0;
                            }
                            bitClear(todo->ordertype.attributes, COMMAND_IS_FORMATION);
                        }
                    }
                    else
                    {
                        setFormationTravelVelocity(todo);
                        //formationArrangeOptimum(todo);

                        // Retail (0x444a8e-0x444abb) gates the single formationContentHasChanged
                        // call solely on: skip for WALL_FORMATION (always), skip for
                        // SPHERE_FORMATION (always), skip for CUSTOM_FORMATION only when
                        // removedShipFromFormation != 0. There is no WALL-to-CUSTOM formation
                        // type mutation or TargetDrone special case anywhere in retail's
                        // RemoveShipsFromDoingStuff or formRemoveShipFromSelection (0x483520);
                        // our prior WALL_FORMATION branch fabricated that conversion and mutated
                        // todo->formation.formationtype to CUSTOM_FORMATION, leaving the
                        // CommandToDo in a state FormationCalculateOffsets never expects to see
                        // -- this was the root cause of a FormationCalculateOffsets crash reading
                        // a garbage tacticalState.
                        if (todo->formation.formationtype != WALL_FORMATION &&
                            todo->formation.formationtype != SPHERE_FORMATION &&
                            (todo->formation.formationtype != CUSTOM_FORMATION || removedShipFromFormation != 0))
                        {
                            formationContentHasChanged(todo);
                        }
                    }
                }
            }
            else
            {
                switch (todo->ordertype.order)
                {
                    case COMMAND_ATTACK:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            RemoveShipFromAttacking(shiptoremove);
                            RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_COLLECTRESOURCE:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            RemoveShipFromResourceCollecting(shiptoremove);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_MOVE:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            RemoveShipFromMoving(shiptoremove);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_LINK:
                    case COMMAND_LINKED_LAUNCH:
                        if (clRemoveShipFromSelection(todoselection, shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove, todo);
                            }

                            clClearLinkState(shiptoremove);
                        }
                        break;

                    case COMMAND_NULL:
                    case COMMAND_HALT:
                    case COMMAND_EXTERNALCONSTRUCTION:
                    case COMMAND_SIEGECANNON:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_SPECIAL:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            dbgAssert((todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) == 0);

                            RemoveShipFromCommand(shiptoremove);
                            if (shiptoremove->staticinfo->custshipheader.CustShipSpecialOrderCleanup != NULL)
                            {
                                ((void (__cdecl *)(Ship *))shiptoremove->staticinfo->custshipheader.CustShipSpecialOrderCleanup)(shiptoremove);
                            }

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_MILITARYPARADE:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            RemoveShipFromMilitaryParade(shiptoremove,todo->militaryParade);

                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_LAUNCHSHIP:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            dockFreeDockPoint(shiptoremove);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_DOCK:
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);

                            if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                            {
                                RemoveShipFromAttacking(shiptoremove);
                                RemoveShipReferencesFromExtraAttackInfo(shiptoremove,todo);
                            }

                            dockWorkerClampFunc(shiptoremove);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                        break;

                    case COMMAND_BUILDINGSHIP:
                        break;

                    case COMMAND_MP_HYPERSPACEING:
#ifdef HW_DEBUG
                        dbgMessage("\nHyperspaceing ship removed from doing stuff!");
#endif
                        if (clRemoveShipFromSelection(todoselection,shiptoremove))
                        {
                            RemoveShipFromCommand(shiptoremove);
                            removeShipFromMpHyperspaceing(shiptoremove);
                            if (todoselection->numShips == 0)
                            {
                                Node *nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }

                        break;
                    default:
                        dbgAssert(FALSE);
                        break;
                }

            }
        }

        todonode = todonode->next;
readyfornextnode:
        ;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clDerelictDied
    Description : Removes derelict from command layer, such that it is
                  not the target of any ship.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clDerelictDied(CommandLayer *comlayer,DerelictPtr derelict)
{
    Node *todonode;
    CommandToDo *todo;

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            dbgAssert(todo->ordertype.order != COMMAND_ATTACK);
            if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)derelict))
            {
                RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)derelict,todo);
            }
        }

        switch (todo->ordertype.order)
        {
            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
                break;
            case COMMAND_SPECIAL:
                if (todo->specialtargets)
                {
                    clRemoveShipFromSelection(todo->specialtargets,(SpaceObjRotImpTarg *)derelict);
                }
                break;
            case COMMAND_HALT:
            case COMMAND_BUILDINGSHIP:
            case COMMAND_COLLECTRESOURCE:
            case COMMAND_MILITARYPARADE:
            case COMMAND_LINK:
            case COMMAND_EXTERNALCONSTRUCTION:
            case COMMAND_SIEGECANNON:
                break;

            case COMMAND_ATTACK:
                if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)derelict))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)derelict,todo);
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clResourceDied
    Description : Removes resource from command layer, such that it is
                  not the target of any ship.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clResourceDied(CommandLayer *comlayer,ResourcePtr resource)
{
    Node *todonode;
    CommandToDo *todo;

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            dbgAssert(todo->ordertype.order != COMMAND_ATTACK);
            if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)resource))
            {
                RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)resource,todo);
            }
        }

        switch (todo->ordertype.order)
        {
            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
            case COMMAND_SPECIAL:
            case COMMAND_HALT:
            case COMMAND_BUILDINGSHIP:
            case COMMAND_MILITARYPARADE:
            case COMMAND_LINK:
            case COMMAND_EXTERNALCONSTRUCTION:
            case COMMAND_SIEGECANNON:
                break;

            case COMMAND_ATTACK:
                if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)resource))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)resource,todo);
                }
                break;

            case COMMAND_COLLECTRESOURCE:
                if (resource == todo->collect.resource)
                {
                    TurnOffAnyResourcingEffects(todo);
                    todo->collect.resource = NULL;
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clMissileDied
    Description : Removes missile from command layer, such that it is
                  not the target of any ship.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMissileDied(CommandLayer *comlayer,MissilePtr missile)
{
    Node *todonode;
    CommandToDo *todo;

    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        switch (todo->ordertype.order)
        {
            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_DOCK:
            case COMMAND_LAUNCHSHIP:
            case COMMAND_SPECIAL:
            case COMMAND_COLLECTRESOURCE:
            case COMMAND_HALT:
            case COMMAND_BUILDINGSHIP:
            case COMMAND_MILITARYPARADE:
            case COMMAND_LINK:
            case COMMAND_EXTERNALCONSTRUCTION:
            case COMMAND_SIEGECANNON:
                break;

            case COMMAND_ATTACK:
                if (clRemoveTargetFromSelection(todo->attack,(SpaceObjRotImpTarg *)missile))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)missile,todo);
                }
                break;

            default:
                dbgAssert(FALSE);
                break;
        }

        todonode = todonode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : shipHasJustCloaked
    Description : cleans up te games such that the ship that just cloaked
                  isn';t referenced by ships n' such that it shouldn't be.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shipHasJustCloaked(Ship *ship)
{
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_CLOAKED,0);
    if(ship->playerowner != universe.curPlayerPtr &&
       !shipGetSlotStatus(ship, universe.curPlayerPtr->playerIndex))
    {           //if ship isn't players...remove it from camera stack
        ccRemoveTheseShips(&universe.mainCameraCommand,(SelectCommand *)ship);
    }
}

void shipHasJustDisappeared(Ship *ship)
{
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_DISAPPEARED,0);
}

/*-----------------------------------------------------------------------------
    Name        : shipHasJustBeenDisabled
    Description : cleans up the game such that the ship that has just been disabled isn't
                  being attacked by ships, etc.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void shipHasJustBeenDisabled(Ship *ship)
{
    RemoveShipFromBeingTargeted(&universe.mainCommandLayer,ship,REMOVE_DISABLED,0);
    ccRemoveTheseShips(&universe.mainCameraCommand,(SelectCommand *)ship);
    if (ShipTypeIsGravWellGeneratorType(ship->shiptype))
    {
        GravWellGeneratorJustDisabled(ship);
    }
    madLinkInCloseGunsShip(ship);
    madLinkInCloseDoor(ship);
    madLinkInCloseSpecialShip(ship);
}

// removeShipsFromDockingWithThisShip
//
// If there are ships docking with this ship, they will be cancelled
void removeShipsFromDockingWithThisShip(struct Player *player)
{
    Node *todonode = universe.mainCommandLayer.todolist.head;
    while (todonode != NULL)
    {
        CommandToDo *todo = (CommandToDo *)listGetStructOfNode(todonode);
        Node *nextnode = todonode->next;
        if (todo->ordertype.order == COMMAND_DOCK)
        {
            SelectCommand *selection = todo->selection;
            sdword i = 0;
            while (i < selection->numShips)
            {
                Ship *ship = selection->ShipPtr[i];
                if (ship->playerowner == player &&
                    !isCapitalShip(ship) &&
                    ship->staticinfo->shipclass != CLASS_Component)
                {
                    dockFreeDockPoint(ship);
                    if (clRemoveTargetFromSelection((SelectAnyCommand *)selection, (TargetPtr)ship))
                    {
                        ship->command = NULL;
                        if (selection->numShips == 0)
                        {
                            FreeCommandToDoContents(todo);
                            listDeleteNode(todonode);
                            goto readyfornextnode;
                        }
                    }
                    else
                    {
                        i++;
                    }
                }
                else
                {
                    i++;
                }
            }
        }
readyfornextnode:
        todonode = nextnode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromBeingTargeted
    Description : removes shiptoremove from command layer, such that it is
                  not the target of any other ship
    Inputs      : shiptoremove, removeFlag which can be:
                  REMOVE_PROTECT - should remove ship from being protected
                  REMOVE_CLOAKED - ship was just cloaked.  If this flag is true, when the ship is removed,
                  it is only removed if the reference that is being considered CAN'T see the ship
                  REMOVE_DISABLED - ship was SOF_Disable'd (e.g. Salvaged)
                  REMOVE_DISAPPEARED - ship was hidden (SOF_Hide'd)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromBeingTargeted(CommandLayer *comlayer,ShipPtr shiptoremove,udword removeFlag, sdword playerIndex)
{
    Node *todonode;
    Node *nextnode;
    CommandToDo *todo;
    udword REMOVE_PROTECT_FLAG = removeFlag & REMOVE_PROTECT;
    udword CLOAKED_REMOVAL_FLAG = removeFlag & REMOVE_CLOAKED;
    udword SHIP_JUST_DISABLED_FLAG = removeFlag & REMOVE_DISABLED;
    udword SHIP_DISAPPEARED_FLAG = removeFlag & REMOVE_DISAPPEARED;
    udword SHIP_HYPERSPACEING_FLAG = removeFlag & REMOVE_HYPERSPACING;
    udword SALCAP_TARGETING_FLAG = removeFlag & REMOVE_SALCAPTARGETING;
    todonode = comlayer->todolist.head;

    while (todonode != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(todonode);

        if (SALCAP_TARGETING_FLAG &&
            todo->selection->numShips > 0 &&
            todo->selection->ShipPtr[0]->playerowner->playerIndex == playerIndex)
        {
            todonode = todonode->next;
            goto readyfornextnode;
        }

        //if ship is hyperspaceing out...via mp hp move
        if(SHIP_HYPERSPACEING_FLAG)
        {
            //prevent ships from docking with it!

            if(todo->ordertype.order == COMMAND_DOCK)
            {
                SelectCommand *selection = todo->selection;
                sdword numShips = selection->numShips;
                bool32 removefromdocking;
                Ship *ship;
                sdword i;
                for (i=0;i<numShips;i++)
                {
                    ship = selection->ShipPtr[i];
                    removefromdocking = FALSE;
                    if (ship->dockvars.dockship == shiptoremove)
                    {
                        removefromdocking = TRUE;
                    }
                    if (removefromdocking)
                        dockWorkerClampFunc(ship);
                }
            }
            else if(todo->ordertype.order == COMMAND_SPECIAL)
            {
                sdword i;
                for(i=0;i<todo->selection->numShips;i++)
                {
                    if(ShipTypeIsSalCapCorvetteType(todo->selection->ShipPtr[i]->shiptype))
                    {
                        if((((SalCapCorvetteSpec *)(todo->selection->ShipPtr[i])->ShipSpecifics))->target != NULL &&
                           ((Ship *)(((SalCapCorvetteSpec *)(todo->selection->ShipPtr[i])->ShipSpecifics))->target) == shiptoremove)
                        {
                            if(((ShipStaticInfo *)todo->selection->ShipPtr[i]->staticinfo)->custshipheader.CustShipRemoveShipReferences != NULL)
                            {
                                ((ShipStaticInfo *)todo->selection->ShipPtr[i]->staticinfo)->custshipheader.CustShipRemoveShipReferences(todo->selection->ShipPtr[i], shiptoremove);
                            }
                        }
                    }
                }
                if (SHIP_DISAPPEARED_FLAG ||
                    !(CLOAKED_REMOVAL_FLAG &&
                      shipGetSlotStatus(shiptoremove, todo->selection->ShipPtr[0]->playerowner->playerIndex)))
            {
                if (!SHIP_JUST_DISABLED_FLAG)
                {
                    if (todo->specialtargets)
                    {
                        clRemoveShipFromSelection(todo->specialtargets,shiptoremove);
                    }
                }
            }

            }
            todonode = todonode->next;
            goto readyfornextnode;
        }

        if(REMOVE_PROTECT_FLAG)
        {
            if (todo->ordertype.attributes & COMMAND_IS_PROTECTING)
            {
                if (SHIP_DISAPPEARED_FLAG ||
                    !(CLOAKED_REMOVAL_FLAG &&
                      shipGetSlotStatus(shiptoremove, todo->selection->ShipPtr[0]->playerowner->playerIndex)))
                {
                    clRemoveShipFromSelection((SelectCommand *)todo->protect,shiptoremove);

                    if (todo->protect->numShips == 0)
                    {
                        ClearProtecting(todo);

                        if (todo->ordertype.attributes == 0)
                        {
                            if (todo->ordertype.order == COMMAND_NULL)
                            {
                                nextnode = todonode->next;
                                FreeCommandToDoContents(todo);
                                listDeleteNode(todonode);
                                todonode = nextnode;
                                goto readyfornextnode;
                            }
                        }
                    }
                }
            }
        }
        if (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            dbgAssert(todo->ordertype.order != COMMAND_ATTACK);
            if (SHIP_DISAPPEARED_FLAG ||
                !(CLOAKED_REMOVAL_FLAG &&
                  shipGetSlotStatus(shiptoremove, todo->selection->ShipPtr[0]->playerowner->playerIndex)))
            {
                if (clRemoveShipFromSelection((SelectCommand *)todo->attack,shiptoremove))
                {
                    RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)shiptoremove,todo);
                }
            }
        }

        switch (todo->ordertype.order)
        {
            case COMMAND_ATTACK:
                if (SHIP_DISAPPEARED_FLAG ||
                    !(CLOAKED_REMOVAL_FLAG &&
                      shipGetSlotStatus(shiptoremove, todo->selection->ShipPtr[0]->playerowner->playerIndex)))
                {
                    if (clRemoveShipFromSelection((SelectCommand *)todo->attack,shiptoremove))
                    {
                        RemoveAttackTargetFromExtraAttackInfo((SpaceObjRotImpTarg *)shiptoremove,todo);
                    }
                }
                break;

            case COMMAND_SPECIAL:
                if (SHIP_DISAPPEARED_FLAG ||
                    !(CLOAKED_REMOVAL_FLAG &&
                      shipGetSlotStatus(shiptoremove, todo->selection->ShipPtr[0]->playerowner->playerIndex)))
                {
                    if (!SHIP_JUST_DISABLED_FLAG)
                    {
                        if (todo->specialtargets)
                        {
                            sdword h;
                            clRemoveShipFromSelection(todo->specialtargets,shiptoremove);
                            for(h=0;h<todo->selection->numShips;h++)
                            {
                                if(ShipTypeIsSalCapCorvetteType(todo->selection->ShipPtr[h]->shiptype) ||
                                    todo->selection->ShipPtr[h]->shiptype == JunkYardDawg)
                                {
                                    salCapCleanUpCloakingTarget(todo->selection->ShipPtr[h],shiptoremove);
                                }
                            }
                        }
                    }
                }
                break;

            case COMMAND_DOCK:
                if (!SHIP_DISAPPEARED_FLAG)
                {
                    SelectCommand *selection = todo->selection;
                    sdword numShips = selection->numShips;
                    Ship *ship;
                    sdword i;
                    for (i=0;i<numShips;i++)
                    {
                        ship = selection->ShipPtr[i];
                        if (ship->dockvars.dockship == shiptoremove)
                        {
                            dockWorkerClampFunc(ship);
                        }
                    }
                }
                break;

            case COMMAND_LAUNCHSHIP:
                if (!SHIP_DISAPPEARED_FLAG)
                if (shiptoremove == todo->launchship.receiverShip)
                {
                    Ship *ship = todo->selection->ShipPtr[0];

                    dbgAssert(todo->selection->numShips == 1);
                    dockFreeDockPoint(ship);

                    nextnode = todonode->next;
                    FreeCommandToDoContents(todo);
                    listDeleteNode(todonode);
                    todonode = nextnode;
                    goto readyfornextnode;
                }
                break;

            case COMMAND_BUILDINGSHIP:
                if (!SHIP_DISAPPEARED_FLAG)
                if (shiptoremove == todo->buildingship.creator)
                {
                    nextnode = todonode->next;
                    FreeCommandToDoContents(todo);
                    listDeleteNode(todonode);
                    todonode = nextnode;
                    goto readyfornextnode;
                }
                break;

            case COMMAND_MILITARYPARADE:
                if (!SHIP_DISAPPEARED_FLAG)
                if (shiptoremove == todo->militaryParade->aroundShip)
                {
                    ChangeOrderToHalt(todo);
                }
                break;

            case COMMAND_NULL:
            case COMMAND_MOVE:
            case COMMAND_MP_HYPERSPACEING:
            case COMMAND_COLLECTRESOURCE:
            case COMMAND_HALT:
            case COMMAND_LINK:
            case COMMAND_EXTERNALCONSTRUCTION:
            case COMMAND_SIEGECANNON:
                break;

            default:
                dbgAssert(FALSE);
                break;
        }
        todonode = todonode->next;
readyfornextnode:
        ;
    }
}

/*=============================================================================
    Public Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : clInit
    Description : Initializes the command layer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clInit(CommandLayer *comlayer)
{
    RUTransferReceivedFmt = strGetString(strReceivedResources);
    listInit(&comlayer->todolist);
}

/*-----------------------------------------------------------------------------
    Name        : clClose
    Description : Closes the command layer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clClose(CommandLayer *comlayer)
{
    Node *nextnode;
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    // free contents of each node of list
    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        nextnode = curnode->next;

        if (singlePlayerGameLoadNewLevelFlag)
        {
            // preserve parade formation throughout hyperspace
            if (command->ordertype.order == COMMAND_MILITARYPARADE)
            {
                Ship *aroundship = command->militaryParade->aroundShip;
                if ((aroundship) && (aroundship->playerowner == universe.curPlayerPtr))
                {
                    goto dontfreeit;
                }
            }
        }

        FreeCommandToDoContents(command);
        listDeleteNode(curnode);

dontfreeit:
        curnode = nextnode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clCancelAllLaunchOrdersFromPlayer
    Description : Cancels all launch orders so ships in process of launching
                  will be available to dock and to fix a fatal bug with the single player game
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    Name        : clReset
    Description : Resets the command layer
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clReset(CommandLayer *comlayer)
{
    clClose(comlayer);
}

/*-----------------------------------------------------------------------------
    Name        : IsSelectionAlreadyDoingSomething
    Description : sees if the selection specified is alread in the command layer
    Inputs      :
    Outputs     :
    Return      : CommandToDo pointer if the selection specified is already
                  present, NULL otherwise
----------------------------------------------------------------------------*/
CommandToDo *IsSelectionAlreadyDoingSomething(CommandLayer *comlayer,SelectCommand *selectcom)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (SelectionsAreEquivalent(selectcom,command->selection))
        {
            return command;
        }

        curnode = curnode->next;
    }

    return NULL;
}

//returns TRUE if a strike craft is in the selection <corvette or fighter>
bool32 selectionHasStrikeCraft(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette)
        {
            return TRUE;
        }
    }
    return FALSE;
}

bool32 isSelectionExclusivlyStrikeCraft(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(!(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette))
        {
            return FALSE;
        }
    }
    return TRUE;
}

bool32 isSelectionExclusivlyStrikeCraftorNoMoveAndAttackShips(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(!(selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
           selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette ||
             selection->ShipPtr[i]->staticinfo->cantMoveAndAttack))
        {
            return FALSE;
        }
    }
    return TRUE;
}


/*
;=============================================================================
; Solution for ships moving to single point nastiness:
;=============================================================================

; If it looks like a bunch of ships in different parts of the world are moving to a "central location" then
; their relative positions will be scaled near to a single point using the factor SHRINKMIN.
; If it looks like a bunch of ships are moving to a relatively far location, they will preserve their
; relative positions (SHRINKMAX, which is typically 1.0)

; Here is the technical description:

; consider a distance d, which is the average position the ships travel.
; consider a radius r, within which all ships ordered to move are within

; Take the ratio d/r.
; if d/r is less than DISTMINRATIO, the relative scaling factor will be SHRINKMIN
; if d/r is greater than DISTMAXRATIO, the relative scaling factor will be SHRINKMAX
; in between,
; scales from a     d/r ratio of DISTMINRATIO      to      a scale factor of SHRINKMIN
;                                DISTMAXRATIO                                SHRINKMAX
*/

typedef struct
{
    vector fromtoship;
    vector totoship;
} ShipMoveCalcs;

/*-----------------------------------------------------------------------------
    Name        : CalculateMoveToPoints
    Description :
    Inputs      : selection, from, to
    Outputs     : moveTo vectors are filled in
    Return      :
----------------------------------------------------------------------------*/
static void CalculateMoveOffsets(SelectCommand *selection, vector from, vector to, vector *offsets)
{
    sdword numShips = selection->numShips;
    ShipMoveCalcs stackMoveCalcs[NUM_STACK_MOVECALCS];
    bool32 moveCalcsAllocated = FALSE;
    ShipMoveCalcs *movecalcs;
    sdword i;
    vector d,r;
    real32 dsqr,rsqr,dmag;
    real32 maxrsqr,maxrmag;
    real32 scale,doverr;

    dbgAssert(numShips > 0);


    if (numShips == 1)
    {
        vecZeroVector(offsets[0]);
        return;
    }

    vecSub(d,to,from);
    dsqr = vecMagnitudeSquared(d);
    dmag = fsqrt(dsqr);

    if (numShips > NUM_STACK_MOVECALCS)
    {
        moveCalcsAllocated = TRUE;
        movecalcs = memAlloc(sizeof(ShipMoveCalcs)*numShips,"movecalcs",0);
    }
    else
    {
        movecalcs = stackMoveCalcs;
    }

    maxrsqr = -1.0f;

    for (i=0;i<numShips;i++)
    {
        vecSub(r,selection->ShipPtr[i]->posinfo.position,from);
        movecalcs[i].fromtoship = r;
        movecalcs[i].totoship = r;

        rsqr = vecMagnitudeSquared(r);
        if (rsqr > maxrsqr)
        {
            maxrsqr = rsqr;
        }
    }

    dbgAssert(maxrsqr >= 0.0f);
    maxrmag = fsqrt(maxrsqr);

    // scales from a d/r ratio of DISTMINRATIO      to      SHRINKMIN
    //                            DISTMAXRATIO              SHRINKMAX

    if (dmag < maxrmag)      // assumes DISTMINRATIO = 1.0
    {
        // shrink to a point:
        scale = SHRINKMIN;
    }
    else
    {
        // preserve relative movement
        doverr = dmag / maxrmag;

        if (doverr >= DISTMAXRATIO)
        {
            // skip scaling
            scale = SHRINKMAX;
        }
        else
        {
            // somewhere between moving to a point and relative movement

            scale = ((doverr - DISTMINRATIO) * (SHRINKMAX - SHRINKMIN) / (DISTMAXRATIO - DISTMINRATIO)) + SHRINKMIN;
        }
    }

    if (scale != 1.0f)
    for (i=0;i<numShips;i++)
    {
        vecMultiplyByScalar(movecalcs[i].totoship,scale);
    }

    for (i=0;i<numShips;i++)
    {
        offsets[i] = movecalcs[i].totoship;
    }

    if (moveCalcsAllocated)
    {
        memFree(movecalcs);
    }
}

void CalculateMoveToPoints(SelectCommand *selection,vector from,vector to)
{
    sdword numShips = selection->numShips;
    vector stackOffsets[NUM_STACK_MOVECALCS];
    vector *offsets;
    bool32 offsetsAllocated = FALSE;
    sdword i;

    if (numShips == 1)
    {
        selection->ShipPtr[0]->moveTo = to;
        return;
    }

    if (numShips > NUM_STACK_MOVECALCS)
    {
        offsetsAllocated = TRUE;
        offsets = memAlloc(sizeof(vector) * numShips, "moveoffsets", 0);
    }
    else
    {
        offsets = stackOffsets;
    }

    CalculateMoveOffsets(selection, from, to, offsets);

    for (i = 0; i < numShips; i++)
    {
        vecAdd(selection->ShipPtr[i]->moveTo, to, offsets[i]);
    }

    if (offsetsAllocated)
    {
        memFree(offsets);
    }
}

// Retail clSetShipMoveToPositions at 0x445BF0 matches CalculateMoveToPoints.
void clSetShipMoveToPositions(SelectCommand *selection,vector from,vector to)
{
    CalculateMoveToPoints(selection,from,to);
}

bool32 selectionHasOtherForceNoAttackAndMoveShips(SelectCommand *selection)
{
    sdword i;
    for(i=0;i<selection->numShips;i++)
    {
        if(selection->ShipPtr[i]->staticinfo->cantMoveAndAttack)
        {
            return TRUE;
        }
    }
    return FALSE;
}
void ChangeOrderToMove(CommandToDo *alreadycommand,MoveCommandPacked packedMove)
{
    SelectCommand *selectcom = alreadycommand->selection;
    sdword i,j;
    vector initialPoint;
    vector destination;


    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (selectcom->numShips > 1 &&
        !(alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION))
    {
        while (selectcom->numShips > 1)
        {
            Ship *ship = selectcom->ShipPtr[0];
            SelectCommand singleShipSelection;
            vector shipAdjustedPoints[ATTACKMOVE_MAX_POINTS];
            MoveCommandPacked shipPackedMove =
                clBuildShipSpecificMovePathCommand(selectcom, ship, packedMove, shipAdjustedPoints);

            singleShipSelection.numShips = 1;
            singleShipSelection.ShipPtr[0] = ship;
            clMoveTheseInternal(&universe.mainCommandLayer, &singleShipSelection, shipPackedMove, TRUE);
        }
    }

    if(alreadycommand->ordertype.order == COMMAND_ATTACK)
    {
        //selection is actually attacking...so this should be a move attack
        if(selectionHasStrikeCraft(alreadycommand->selection) ||
           selectionHasOtherForceNoAttackAndMoveShips(alreadycommand->selection))
        {
            //filter strike craft...give strike craft a move order,
            //and give give capital ships a majik command
            if(!isSelectionExclusivlyStrikeCraftorNoMoveAndAttackShips(alreadycommand->selection))
            {
                //some capitals some strike craft
                MaxSelection tempSelection;
                tempSelection.numShips = 0;
                j = 0;

                dbgAssert(alreadycommand->selection->numShips < COMMAND_MAX_SHIPS);

                for(i=0;i<alreadycommand->selection->numShips;i++)
                {
                    if(alreadycommand->selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter ||
                       alreadycommand->selection->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette ||
                       alreadycommand->selection->ShipPtr[i]->staticinfo->cantMoveAndAttack)
                    {
                        tempSelection.numShips++;
                        tempSelection.ShipPtr[j] = alreadycommand->selection->ShipPtr[i];
                        j++;
                    }
                }
                RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&tempSelection);
                clMoveThese(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,packedMove);
                if(alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
                {
                    clFormation(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,alreadycommand->formation.formationtype);
                }
                goto StillDoAttackMoveWithCapitals;
            }
        }
        else
       {
StillDoAttackMoveWithCapitals:
           Ship *leader = alreadycommand->selection->ShipPtr[0];

           // just capital ships in selection
           if(!bitTest(leader->specialFlags1C8,CATA_RUNTIME_ATTACKMOVECANCEL))
           {
               alreadycommand->ordertype.attributes |= COMMAND_IS_ATTACKINGANDMOVING;
               {
                   AttackMovePathState *attackMove = (AttackMovePathState *)&alreadycommand->move;
                   sdword pointCount = clUnpackAttackMoveSdword(packedMove.to.y);
                   vector initialPoint;
                   vector *points = clUnpackAttackMovePoints(packedMove.to.z);

                   attackMove->pathState = clUnpackAttackMoveSdword(packedMove.from.x);
                   attackMove->pointCount = pointCount;
                   memcpy(attackMove->points,points,pointCount * sizeof(vector));
                   attackMove->currentPointIndex = 0;

                   vecSet(initialPoint,packedMove.from.y,packedMove.from.z,packedMove.to.x);
                   for (i = 0; i < selectcom->numShips; i++)
                   {
                       InitShipAI(selectcom->ShipPtr[i],TRUE);
                   }
                   clSetShipMoveToPositions(selectcom,initialPoint,points[0]);
               }

               for (i=0;i<selectcom->numShips;i++)
               {
                   Ship *ship = selectcom->ShipPtr[i];

                   ship->moveFrom = ship->posinfo.position;
                   soundEventStartEngine(ship);
               }

               for (i=0;i<selectcom->numShips;i++)
               {
                   Ship *ship = selectcom->ShipPtr[i];

                   ship->command = alreadycommand;
                   ship->unk_1D0 = 0;
                   if ((ship->specialFlags1C8 & 0x10000) != 0)
                   {
                       rowRemoveShipFromGettingOutOfWay(ship);
                   }
               }
               tacticsMakeShipsNotLookForOtherShips(selectcom);
               return;
           }
           bitClear(leader->specialFlags1C8,CATA_RUNTIME_ATTACKMOVECANCEL);
       }
    }

    if (alreadycommand->ordertype.order == COMMAND_MOVE)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging movement direction");
#endif

    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging command to move");
#endif
        FreeLastOrder(alreadycommand);

        alreadycommand->ordertype.order = COMMAND_MOVE;
    }

    clUnpackMovePointCommand(packedMove, &initialPoint, &destination);
    clStoreMovePath(alreadycommand, packedMove);

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        InitShipAI(selectcom->ShipPtr[0],TRUE);
    }
    else
    {
        for (i = 0; i < selectcom->numShips; i++)
        {
            InitShipAI(selectcom->ShipPtr[i],TRUE);
        }
    }

    CalculateMoveToPoints(selectcom,initialPoint,destination);

    for (i=0;i<selectcom->numShips;i++)
    {
        //selectcom->ShipPtr[i]->moveTo   = to;
        selectcom->ShipPtr[i]->moveFrom = selectcom->ShipPtr[i]->posinfo.position;
        soundEventStartEngine(selectcom->ShipPtr[i]);
    }

    PrepareShipsForCommand(alreadycommand,TRUE);

    return;
}

/*-----------------------------------------------------------------------------
    Name        : clMoveThese
    Description : Command to move ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void clMoveTheseInternal(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove,bool32 skipEquivalentSearch)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips = selectcom->numShips;
    sdword i;
    vector initialPoint;
    vector destination;


    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships To Move");
#endif
        return;
    }

    if (!skipEquivalentSearch &&
        (alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        ChangeOrderToMove(alreadycommand,packedMove);
        return;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to move");
#endif

    if (!skipEquivalentSearch)
    {
        sdword sizeofcopycom;
        sdword copyCount;
        sdword pathState;
        sdword pointCount;
        vector *points;
        vector initialPointUnused;
        SelectCommand *copycom;

        sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
        copycom = memAlloc(sizeofcopycom,"cc1(copycom)", Pyrophoric);
        memcpy(copycom,selectcom,sizeofcopycom);
        copyCount = copycom->numShips;
        clUnpackMovePathCommand(packedMove, &pathState, &initialPointUnused, &pointCount, &points);

        if (pointCount == 1)
        {
            for (i = 0; i < copyCount; i++)
            {
                CommandToDo *currentShipCommand = getShipAndItsCommand(&universe.mainCommandLayer,copycom->ShipPtr[i]);

                if(currentShipCommand != NULL && currentShipCommand->ordertype.order == COMMAND_ATTACK)
                {
                    MaxSelection tempSelection;
                    MaxAnySelection targetSelection;
                    TypeOfFormation formation;
                    bool32 needFormation;
                    sdword j;

                    tempSelection.numShips = 0;

                    for (j = 0; j < currentShipCommand->selection->numShips; j++)
                    {
                        if (ShipInSelection(copycom,currentShipCommand->selection->ShipPtr[j]))
                        {
                            tempSelection.ShipPtr[tempSelection.numShips] = currentShipCommand->selection->ShipPtr[j];
                            tempSelection.numShips++;
                            clRemoveShipFromSelection(copycom,currentShipCommand->selection->ShipPtr[j]);
                        }
                    }
                    dbgAssert(tempSelection.numShips > 0);
                    for (j = 0; j < currentShipCommand->attack->numTargets; j++)
                    {
                        targetSelection.TargetPtr[j] = currentShipCommand->attack->TargetPtr[j];
                    }
                    targetSelection.numTargets = currentShipCommand->attack->numTargets;

                    if(currentShipCommand->ordertype.attributes & COMMAND_IS_FORMATION)
                    {
                        formation = currentShipCommand->formation.formationtype;
                        needFormation = TRUE;
                    }
                    else
                    {
                        needFormation = FALSE;
                    }
                    RemoveShipsFromDoingStuff(&universe.mainCommandLayer,(SelectCommand *)&tempSelection);
                    if(needFormation)
                    {
                        clFormation(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,formation);
                    }
                    clAttackThese(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,(SelectAnyCommand *)&targetSelection);
                    clMoveTheseInternal(&universe.mainCommandLayer,(SelectCommand *)&tempSelection,packedMove,FALSE);
                }
            }
            sizeofcopycom = sizeofSelectCommand(copycom->numShips);
            memcpy(selectcom,copycom,sizeofcopycom);
            memFree(copycom);
            numShips = selectcom->numShips;
            if (numShips == 0)
            {
                return;
            }
        }
        else
        {
            memFree(copycom);
        }
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    if (numShips > 1)
    {
        for (i = 0; i < numShips; i++)
        {
            SelectCommand singleShipSelection;
            vector shipAdjustedPoints[ATTACKMOVE_MAX_POINTS];
            MoveCommandPacked shipPackedMove =
                clBuildShipSpecificMovePathCommand(selectcom, selectcom->ShipPtr[i], packedMove, shipAdjustedPoints);

            singleShipSelection.numShips = 1;
            singleShipSelection.ShipPtr[0] = selectcom->ShipPtr[i];
            clMoveTheseInternal(comlayer, &singleShipSelection, shipPackedMove, TRUE);
        }
        return;
    }

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    clUnpackMovePointCommand(packedMove, &initialPoint, &destination);
    clStoreMovePath(newcommand, packedMove);
    newcommand->ordertype.order = COMMAND_MOVE;
    newcommand->ordertype.attributes = 0;

    for (i = 0; i < numShips; i++)
    {
        InitShipAI(selection->ShipPtr[i],TRUE);
    }

    CalculateMoveToPoints(selection,initialPoint,destination);

    for (i = 0; i < numShips; i++)
    {
        selection->ShipPtr[i]->moveFrom = selection->ShipPtr[i]->posinfo.position;
        soundEventStartEngine(selectcom->ShipPtr[i]);
    }

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

void clMoveThese(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove)
{
    clMoveTheseInternal(comlayer, selectcom, packedMove, FALSE);
}

void clSiegeCannon(CommandLayer *comlayer,SelectCommand *selectcom,vector *target)
{
    CommandToDo *newcommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword i;

    if (selectcom->numShips == 0)
    {
        return;
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_SIEGECANNON;
    newcommand->ordertype.attributes = 0;
    *clSiegeCannonTarget(newcommand) = *target;

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];

        ship->command = newcommand;
        ship->aistatecommand = 0;
        if (ship->specialFlags1C8 & CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY)
        {
            rowRemoveShipFromGettingOutOfWay(ship);
        }
    }

    tacticsMakeShipsNotLookForOtherShips(selection);
    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : clMove
    Description : Command to move ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMove(CommandLayer *comlayer,SelectCommand *selectcom,MoveCommandPacked packedMove)
{
    typedef bool32 (__cdecl *ShipMoveOrderCleanupCB)(Ship *ship, MoveCommandPacked packedMove);
    SelectCommand *copycom;
    udword sizeofcopycom;
    sdword i;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;

    if (selectcom->numShips < 1)
    {
        return;
    }
    clTraverseCommandList(&comlayer->todolist.head);
    clTraverseCommandList(&comlayer->todolist.head);

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc2(copycom)", Pyrophoric);
    copycom->numShips = 0;
    for (i = 0; i < selectcom->numShips; i++)
    {
        Ship *ship = selectcom->ShipPtr[i];
        ShipMoveOrderCleanupCB moveConstraintCB = (ShipMoveOrderCleanupCB)ship->staticinfo->custshipheader.CustShipMoveOrderCleanup;
        if (moveConstraintCB == NULL || moveConstraintCB(ship, packedMove))
        {
            copycom->ShipPtr[copycom->numShips++] = ship;
        }
    }

    clFilterShipsForOrders(copycom);

    if (copycom->numShips < 1)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships to move");
#endif
        memFree(copycom);
        return;
    }

#ifdef DEBUG_TACTICS
    if(tacticsOn)
#endif
    {
        tacticsReportMove(comlayer,copycom);
    }

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to move (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {
            MakeShipsNotIncludeTheseShips(copycom,command->selection);
            if (command->selection->numShips == 1)
            {
                vector shipAdjustedPoints[ATTACKMOVE_MAX_POINTS];
                MoveCommandPacked shipPackedMove =
                    clBuildShipSpecificMovePathCommand(selectcom, command->selection->ShipPtr[0], packedMove, shipAdjustedPoints);
                ChangeOrderToMove(command, shipPackedMove);
            }
            else
            {
                ChangeOrderToMove(command,packedMove);
            }
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        clMoveThese(comlayer,copycom,packedMove);
    }

    memFree(copycom);
}

CommandToDo *clExternalConstructionSelection(
    CommandLayer *comlayer,
    SelectCommand *selectcom,
    sdword operation,
    sdword arg1,
    sdword arg2,
    sdword arg3)
{
    CommandToDo *command;
    SelectCommand *selectionCopy;
    udword selectionSize;
    sdword i;

    if (selectcom->numShips == 0)
    {
        return NULL;
    }

    command = memAlloc(sizeof(CommandToDo), "ToDo", NonVolatile);
    InitCommandToDo(command);

    selectionSize = sizeofSelectCommand(selectcom->numShips);
    selectionCopy = memAlloc(selectionSize, "ToDoSelection", NonVolatile);
    memcpy(selectionCopy, selectcom, selectionSize);

    RemoveShipsFromDoingStuff(comlayer, selectionCopy);

    command->selection = selectionCopy;
    command->ordertype.order = COMMAND_EXTERNALCONSTRUCTION;
    command->ordertype.attributes = 0;
    *clExternalConstructionOperation(command) = operation;
    *clExternalConstructionShipType(command) = (ShipType)arg1;
    *clExternalConstructionParam2(command) = *(real32 *)&arg2;
    *clExternalConstructionParam3(command) = *(real32 *)&arg3;

    for (i = 0; i < selectionCopy->numShips; i++)
    {
        Ship *ship = selectionCopy->ShipPtr[i];

        ship->command = command;
        ship->aistatecommand = 0;
        if (ship->specialFlags1C8 & CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY)
        {
            rowRemoveShipFromGettingOutOfWay(ship);
        }
    }

    tacticsMakeShipsNotLookForOtherShips(selectionCopy);
    listAddNode(&comlayer->todolist, &command->todonode, command);
    return command;
}

void processUpdateExternalConstructionToDo(
    CommandLayer *comlayer,
    uword playerIndex,
    Ship *creator,
    Ship *ship,
    real32 progress,
    sdword operation)
{
    (void)comlayer;
    (void)playerIndex;
    (void)creator;

    if (ship == NULL)
    {
        telldir();
        return;
    }

    cmUpdateExternalBuildShip(ship, progress, operation);
}


void TurnAllGravwellsOff(SelectCommand *selectcom)
{
    sdword i;

    for(i=0;i<selectcom->numShips;i++)
    {
        if(ShipTypeIsGravWellGeneratorType(selectcom->ShipPtr[i]->shiptype))
        {
            if( ((GravWellGeneratorSpec *)(selectcom->ShipPtr[i]->ShipSpecifics))->GravFieldOn)
            {
                turnoffGravwell(selectcom->ShipPtr[i]);
            }
        }
    }
}

void ChangeOrderToHyperSpace(CommandToDo *alreadycommand,vector from,vector to)
{
    SelectCommand *selectcom = alreadycommand->selection;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }
    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (alreadycommand->ordertype.order == COMMAND_MP_HYPERSPACEING)
    {
        dbgMessagef("\nA little wierd:  Ships that were hyperspaceing have been given an order to hyperspace again.  Ignoreing...");
        return;
        dbgFatalf(DBG_Loc,"Shouldn't Be able to Order these guys...they're hyperspaceing!");
    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging command to Hyperspace");
#endif
        FreeLastOrder(alreadycommand);

        alreadycommand->ordertype.order = COMMAND_MP_HYPERSPACEING;
    }

    alreadycommand->move.destination = to;
    vecSub(alreadycommand->move.heading,to,from);
    vecNormalize(&alreadycommand->move.heading);

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        InitShipAI(selectcom->ShipPtr[0],TRUE);
    }
    else
    {
        InitShipsForAI(selectcom,TRUE);
    }

    //CalculateMoveToPoints(selectcom,from,to);

    //for (i=0;i<selectcom->numShips;i++)
    //{
    //    //selectcom->ShipPtr[i]->moveTo   = to;
    //    selectcom->ShipPtr[i]->moveFrom = selectcom->ShipPtr[i]->posinfo.position;
    //    //soundEventStartEngine(selectcom->ShipPtr[i]);
    //}
    alreadycommand->hyperSpaceingTime = 0.0f;
    alreadycommand->hyperspaceState = COM_HYPERSPACE_START;
    PrepareShipsForCommand(alreadycommand,TRUE);

    return;
}

/*-----------------------------------------------------------------------------
    Name        : clMpHyperspaceThese
    Description : Command to mp hyperspace ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMpHyperspaceThese(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips = selectcom->numShips;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships To hyperspace");
#endif
        return;
    }
    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        ChangeOrderToHyperSpace(alreadycommand,from,to);
        return;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to hyperspace");
#endif

    TurnAllGravwellsOff(selectcom);

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->move.destination = to;
    vecSub(newcommand->move.heading,to,from);
    vecNormalize(&newcommand->move.heading);
    newcommand->ordertype.order = COMMAND_MP_HYPERSPACEING;
    newcommand->ordertype.attributes = 0;
    newcommand->hyperSpaceingTime = 0.0f;
    newcommand->hyperspaceState = COM_HYPERSPACE_START;

    InitShipsForAI(selection,TRUE);

    //CalculateMoveToPoints(selection,from,to);

    /*
      for (i=0;i<numShips;i++)
    {
        //selection->ShipPtr[i]->moveTo = to;
        selection->ShipPtr[i]->moveFrom = selection->ShipPtr[i]->posinfo.position;
        soundEventStartEngine(selectcom->ShipPtr[i]);
    }
    */
    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : getNewHyperspaceingDestination
    Description : calculates the correct destination point for a group of ships hyperspacing
    Inputs      : selection, from, to,newTo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void getNewHyperspaceingDestination(vector *newTo,vector *from,vector *to,SelectCommand *selection)
{
    //sdword i;
//    vector groupPos;
    //real32 temp;

    //groupPos.x=0.0f;
    //groupPos.y=0.0f;
    //groupPos.z=0.0f;
    //for(i=0;i<selection->numShips;i++)
    //{
    //    vecAddTo(groupPos,selection->ShipPtr[i]->posinfo.position);
    //}
    //vecDivideByScalar(groupPos,((float)selection->numShips),temp);
    //grouppos is the average posiiton of the group
    vecSub(*newTo,selection->ShipPtr[0]->posinfo.position,*from);
    //add the offset to the desired desitination
    vecAddTo(*newTo,*to);
}

/*-----------------------------------------------------------------------------
    Name        : clMpHyperspace
    Description : Command to hyperspace ships in multiplayer game ships
    Inputs      : selectcom, from, to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;
    sdword cost;
    vector distsqr;
    real32 tempdist,distance;
    sdword i;
    vector newTo;
    sdword playerindex;

    makeSelectionHyperspaceCapable(selectcom);

    //should probably leave in this check...you never know
    if (selectcom->numShips < 1)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships to hyperspace");
#endif

        return;
    }

    vecSub(distsqr,from,to);
    tempdist = vecMagnitudeSquared(distsqr);
    distance = fsqrt(tempdist);
    cost = (sdword)hyperspaceCost(distance,selectcom);

    playerindex = selectcom->ShipPtr[0]->playerowner->playerIndex;
    if ( (playerindex >= tpGameCreated.numPlayers) ||    // computer player RU's arene't deterministic, always let it spend RUs (computer RUs can go negative)
         (universe.players[playerindex].resourceUnits >= cost) )
    {
        universe.players[playerindex].resourceUnits -= cost;
        universe.gameStats.playerStats[playerindex].totalResourceUnitsSpent += cost;
    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nCan't afford HS - cancelling");
#endif
        return;
    }
#ifdef DEBUG_TACTICS
    if(tacticsOn)
#endif
    {
        tacticsReportMove(comlayer,selectcom);
    }

    //remove ships from being targeted...
    for(i=0;i<selectcom->numShips;i++)
    {
        RemoveShipFromBeingTargeted(comlayer,selectcom->ShipPtr[i],REMOVE_PROTECT|REMOVE_HYPERSPACING,0);
    }
    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc3(copycom)", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to hyperspace (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {

            MakeShipsNotIncludeTheseShips(copycom,command->selection);
            getNewHyperspaceingDestination(&newTo,&from,&to,command->selection);
            ChangeOrderToHyperSpace(command,from,newTo);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        getNewHyperspaceingDestination(&newTo,&from,&to,copycom);
        clMpHyperspaceThese(comlayer,copycom,from,newTo);
    }

    memFree(copycom);
}


/*-----------------------------------------------------------------------------
    Name        : AttackClosestOpponent
    Description : finds and returns nearest opponent to ship to attack
    Inputs      : ship, todo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SpaceObjRotImpTarg *AttackClosestOpponent(Ship *ship,AttackCommand *attack)
{
    sdword numTargetsToAttack = attack->numTargets;
    sdword i;
    vector dist;
    real32 distsqr;
    real32 mindist;
    sdword attacki;
    SpaceObjRotImpTarg *attackme;

    if (numTargetsToAttack <= 0)
    {
        return NULL;
    }

    mindist = REALlyBig;
    attacki = -1;
    for (i=0;i<numTargetsToAttack;i++)
    {
        attackme = attack->TargetPtr[i];

        vecSub(dist,ship->posinfo.position,attackme->posinfo.position);
        distsqr = vecMagnitudeSquared(dist);

        if (distsqr < mindist)
        {
            mindist = distsqr;
            attacki = i;
        }
    }

    dbgAssert(attacki != -1);

    return attack->TargetPtr[attacki];
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromAttacking
    Description : Removes ship from attacking
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromAttacking(Ship *ship)
{
    CommandToDo *command;

    if (ship->staticinfo->shipclass == CLASS_Resource)
    {
        TurnHarvestEffectOff(ship);
    }
    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        memFree(ship->attackvars.multipleAttackTargets);
        ship->attackvars.multipleAttackTargets = NULL;
    }

    {
        ship->attackvars.attacktarget = NULL;
    }
    ship->attackvars.myLeaderIs = NULL;
    ship->attackvars.myWingmanIs = NULL;
    ship->shipisattacking = FALSE;
    bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE | CATA_RUNTIME_BROKEN_FORMATION);
    bitClear(ship->specialFlags, SPECIAL_AttackFromAbove);
    if (ship->flightmanInfo != NULL)
    {
        univFreeNebulaContents((Nebula *)ship);
    }
    command = ship->command;
    if ((command != NULL) && bitTest(command->ordertype.attributes, COMMAND_IS_FORMATION))
    {
        if (command->formation.formationLocked)
        {
            command->formation.formationLocked = FALSE;
        }

        command->formation.flipselectionfixed = FALSE;
        command->formation.needFix = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : AttackCleanup
    Description : cleans up anything from a attack command
    Inputs      : attacktodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AttackCleanup(struct CommandToDo *attacktodo)
{
    sdword i;
    SelectCommand *selection = attacktodo->selection;
    sdword numShips = selection->numShips;
    bool32 clearmove = TRUE;

    if (bitTest(attacktodo->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING))
    {
        bitClear(attacktodo->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING);
    }

    if(attacktodo->ordertype.order == COMMAND_MOVE)
    {
        clearmove = FALSE;
    }


    if(attacktodo->ordertype.attributes & COMMAND_IS_FORMATION)
    {
#ifdef DEBUG_TACTICS
        if(tacticsOn)
#endif
        {
            if ((attacktodo->formation.formationtype == SPHERE_FORMATION ||
                 attacktodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION) &&
                (attacktodo->selection->numShips > 1))
            {
                attacktodo->formation.enders = FALSE;
                formationContentHasChanged(attacktodo);
            }
        }
    }

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromAttacking(selection->ShipPtr[i]);
        // don't clear unless clearing a move otherwise some moves will get erroneously cancelled
        if(clearmove)
        {
            vecZeroVector(selection->ShipPtr[i]->moveTo);
            vecZeroVector(selection->ShipPtr[i]->moveFrom);
        }
        bitClear(selection->ShipPtr[i]->specialFlags1C8, CATA_RUNTIME_BROKEN_FORMATION);


    }

    //attack has been cancelled, lets cue up a gun closing animation
    //need a delay item here...
    madLinkInCloseGuns(attacktodo,1);
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromFormation
    Description : cleans up ship stuff that was in a formation
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromFormation(Ship *ship)
{
    if (ship->cataBusyStateWindow.sentinelFormationState == 1)
    {
        clSetAttribute((sdword)ship,0,5);
    }
    if (ship->shiptype == sSentinel || ship->shiptype == bSentinel)
    {
        clDetachShipFromCommand(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : FormationCleanup
    Description : cleans up anything from a formation
    Inputs      : formationtodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FormationCleanup(struct CommandToDo *formationtodo)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromFormation(selection->ShipPtr[i]);
    }

    if (formationtodo->formation.formationSpecificInfo)
    {
        memFree(formationtodo->formation.formationSpecificInfo);
        formationtodo->formation.formationSpecificInfo = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipFromMoving
    Description : removes ship from moving
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipFromMoving(Ship *ship)
{
    vecZeroVector(ship->moveTo);
    vecZeroVector(ship->moveFrom);
}

/*-----------------------------------------------------------------------------
    Name        : MoveCleanup
    Description : cleans up anything from a move command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MoveCleanup(struct CommandToDo *movetodo)
{
    sdword i;
    SelectCommand *selection = movetodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        RemoveShipFromMoving(selection->ShipPtr[i]);
    }
}

void removeShipFromMpHyperspaceing(Ship *ship)
{
    vecZeroVector(ship->moveTo);
    vecZeroVector(ship->moveFrom);
    clSetAttribute((sdword)ship,0,6);
    bitSet(ship->flags,SOF_Selectable);
}

static Node *clTraverseCommandList(Node **head)
{
    Node *node;

    for (node = *head; node != NULL; node = node->next)
    {
        ;
    }

    return node;
}

static bool32 clShipCanRetreatForRepairs(Ship *ship, bool32 considerSingleShip)
{
    ShipStaticInfo *staticInfo;

    staticInfo = ship->staticinfo;
    if (staticInfo->shipclass == CLASS_NonCombat ||
        staticInfo->shipclass == CLASS_Mothership ||
        staticInfo->externalBuild)
    {
        return FALSE;
    }

    return !considerSingleShip || ship->cataBusyStateWindow.dockingship == NULL;
}

static bool32 clShipShouldRetreatForRepairs(Ship *ship)
{
    return ship->command == NULL ||
           ship->command->ordertype.order != COMMAND_MOVE ||
           ship->tacticstype != 0 ||
           !clShipCanRetreatForRepairs(ship, TRUE) ||
           ship->staticinfo->oneOverMaxHealth * ship->health >= 0.333f;
}

static bool32 ShipCanAcceptAttackOrder(Ship *ship)
{
    return ship->collMyBlob != NULL &&
           (ship->flags != 0) &&
           ((ship->unk_1C & 0x8000) == 0);
}

static bool32 clAttackTargetKeepsShipBusy(Ship *ship, SpaceObjRotImpTarg *target)
{
    return target->flags2 == 0 &&
           target->objtype == OBJ_ShipType &&
           ((Ship *)target)->shiptype == sLeech &&
           ((Ship *)target)->clampInfo != NULL &&
           ((Ship *)target)->clampInfo->host == (SpaceObjRotImpTarg *)ship;
}

static void clFilterShipsForOrders(SelectCommand *selection)
{
    MakeShipsOnlyFollowConstraints(selection, clShipShouldRetreatForRepairs);
}

/*-----------------------------------------------------------------------------
    Name        : MpHyperspaceCleanup
    Description : cleans up anything from a hyperspaceing move command
    Inputs      : movetodo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MpHyperspaceCleanup(struct CommandToDo *movetodo)
{
    sdword i;
    SelectCommand *selection = movetodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        removeShipFromMpHyperspaceing(selection->ShipPtr[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveAttackTargetFromExtraAttackInfo
    Description : removes shiptoremove from being targeted for attack. Whatever ship was attacking it
                  will choose a new target.
    Inputs      : shiptoremove, extraattackinfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveAttackTargetFromExtraAttackInfo(SpaceObjRotImpTarg *targettoremove,CommandToDo *todo)
{
    sdword i;
    SelectCommand *selection = todo->selection;
    sdword numShips = selection->numShips;
    AttackCommand *attack = todo->attack;
    sdword numShipsToAttack = attack->numTargets;
    Ship *ship;
    AttackTargets *multipleAttackTargets;

    dbgAssert(numShips > 0);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];

        if (ship->attackvars.attacktarget == targettoremove)
        {
            {
                ship->attackvars.attacktarget = NULL;
            }
            ship->attackvars.attackevents |= ATTACKEVENT_ENEMYTARGET_DIED;
        }

        if ((multipleAttackTargets = ship->attackvars.multipleAttackTargets) != NULL)
        {
            sdword j;
            sdword numAttackTargets = multipleAttackTargets->numAttackTargets;

            for (j=0;j<numAttackTargets;j++)
            {
                if (multipleAttackTargets->TargetPtr[j] == targettoremove)
                {
                    multipleAttackTargets->TargetPtr[j] = NULL;
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : RemoveShipReferencesFromExtraAttackInfo
    Description : removes shiptoremove from being reference by extraattackinfo.  For example, if a wingman ship
                  was removed, the reference to the wingman would be removed.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveShipReferencesFromExtraAttackInfo(Ship *shiptoremove,CommandToDo *todo)
{
    SelectCommand *selection = todo->selection;
    sdword numShips = selection->numShips;
    SpecificAttackVars *attackvars;
    sdword i;
    Ship *ship;

    dbgAssert(shiptoremove);

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        attackvars = &ship->attackvars;

        if (ship == shiptoremove)
        {
            continue;       // don't process one that is going to be removed
        }

        if (attackvars->myLeaderIs == shiptoremove)
        {
            // this ship's leader has fallen, so we should not be his wingman anymore.
            attackvars->myLeaderIs = NULL;
            dbgAssert(attackvars->myWingmanIs == NULL);

            attackvars->attackevents |= ATTACKEVENT_LEADER_DIED;
        }

        if (attackvars->myWingmanIs == shiptoremove)
        {
            // lost wingman
            dbgAssert(attackvars->myLeaderIs == NULL);
            attackvars->myWingmanIs = NULL;

            // we lost our wingman
            attackvars->attackevents |= ATTACKEVENT_WINGMAN_DIED;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : allocateMultipleAttackTargets
    Description : allocates a AttackTargets structure based on the number of guns on ship
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
AttackTargets *allocateMultipleAttackTargets(Ship *ship)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword numGuns;
    AttackTargets *attacktargets;
    sdword sizeofAttackTargets;

    dbgAssert(gunInfo != NULL);
    numGuns = gunInfo->numGuns;
    dbgAssert(numGuns > 0);
    sizeofAttackTargets = sizeofAttackTargets(numGuns);
    attacktargets = memAlloc(sizeofAttackTargets,"AttackTargets",0);
    memset(attacktargets,0,sizeofAttackTargets);

    attacktargets->numAttackTargets = numGuns;

    return attacktargets;
}

typedef struct
{
    vector targetInShipCoordSys;
    real32 pickedMe;
} PickMultipleTargetsInfo;

/*-----------------------------------------------------------------------------
    Name        : pickMultipleAttackTargets
    Description : picks best attack target for each gun on a ship
    Inputs      : ship, attack
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pickMultipleAttackTargets(Ship *ship,AttackCommand *attack)
{
    sdword numShipsToAttack = attack->numTargets;
    AttackTargets *attacktargets = ship->attackvars.multipleAttackTargets;
    sdword numAttackTargets = attacktargets->numAttackTargets;
    PickMultipleTargetsInfo *multipleTargetsInfo;
    PickMultipleTargetsInfo *thisTargetInfo;
    SpaceObjRotImpTarg *target;
    SpaceObjRotImpTarg *target1 = NULL;
    SpaceObjRotImpTarg *target2 = NULL;
    sdword i;
    sdword j;
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    Gun *gun;
    GunStatic *gunstatic;
    sdword bestIndex;
    real32 bestDist;
    vector targetInWorldCoordSys;
    vector targetInShipCoordSys;
    real32 dist;
    real32 range;
    real32 temp;
    real32 dotprod,bonus,gunRange;

    dbgAssert(numShipsToAttack > 0);
    dbgAssert(numAttackTargets > 0);
    dbgAssert(gunInfo != NULL);
    dbgAssert(numAttackTargets == gunInfo->numGuns);

    multipleTargetsInfo = memAlloc(sizeof(PickMultipleTargetsInfo)*numShipsToAttack,"mTI(multTargInfo)",Pyrophoric);

    for (i=0,thisTargetInfo=multipleTargetsInfo;i<numShipsToAttack;i++,thisTargetInfo++)
    {
        target = attack->TargetPtr[i];

        aishipGetTrajectory(ship,target,&targetInWorldCoordSys);
        matMultiplyVecByMat(&thisTargetInfo->targetInShipCoordSys,&targetInWorldCoordSys,&ship->rotinfo.coordsys);
        thisTargetInfo->pickedMe = 1.0f;
    }
    if(ship->staticinfo->shipclass == CLASS_Fighter)
    {
        bonus = tacticsInfo.BulletRangeBonus[Tactics_Fighter][ship->tacticstype];
    }
    else if(ship->staticinfo->shipclass == CLASS_Corvette)
    {
        bonus = tacticsInfo.BulletRangeBonus[Tactics_Corvette][ship->tacticstype];
    }
    else
    {
        bonus = 1.0f;
    }

    if (ship->staticinfo->shipclass == CLASS_Frigate)
    {
        target = ship->attackvars.attacktarget;
        if (target->objtype == OBJ_ShipType)
        {
           if (((Ship *)target)->staticinfo->shipclass == CLASS_Corvette)
           {
               target1 = target;
               for (i=0;i<numShipsToAttack;i++)
               {
                   if (attack->TargetPtr[i] != target1)
                   {
                       target2 = attack->TargetPtr[i];
                       break;
                   }
               }
           }
        }
    }

    for (i=0;i<numAttackTargets;i++)
    {
        gun = &gunInfo->guns[i];
        gunstatic = gun->gunstatic;

        gunRange = gunstatic->bulletrange*bonus;
        bestDist = REALlyBig;
        bestIndex = -1;

        for (j=0,thisTargetInfo=multipleTargetsInfo;j<numShipsToAttack;j++,thisTargetInfo++)
        {
            target = attack->TargetPtr[j];
            if (target1)
            {
                // concentrate firepower: skip this loop for other ships than target1, target2
                if ((target != target1) && (target != target2))
                {
                    continue;
                }
            }

            vecSub(targetInShipCoordSys,thisTargetInfo->targetInShipCoordSys,gunstatic->position);

            dist = fsqrt(vecMagnitudeSquared(targetInShipCoordSys));

            range = RangeToTargetGivenDist(ship,target,dist);

            if (range < gunRange)
            {
                vecDivideByScalar(targetInShipCoordSys,dist,temp);

                dotprod = vecDotProduct(targetInShipCoordSys,gunstatic->gunnormal);

                dist *= (2.0f - dotprod);
                dist *= thisTargetInfo->pickedMe;   // treat distance as if it were bigger for targets already picked
                if (dist < bestDist)
                {
                    bestDist = dist;
                    bestIndex = j;
                }
            }
        }

        if (bestIndex != -1)
        {
            attacktargets->TargetPtr[i] = attack->TargetPtr[bestIndex];
            multipleTargetsInfo[bestIndex].pickedMe *= 1.2f;
        }
        else
        {
            attacktargets->TargetPtr[i] = NULL;
        }
    }

    memFree(multipleTargetsInfo);
}

#define WINGMANSHIP_USED       1
#define WINGMANSHIP_IGNORE     2

typedef struct
{
    sdword numShips;
    sdword flags[1];
} FindWingmenInfo;

#define sizeofFindWingmenInfo(n) (sizeof(FindWingmenInfo) + (((n)-1)*sizeof(sdword)))

/*-----------------------------------------------------------------------------
    Name        : BreakupIntoLeaderWingman
    Description : breaks up the selection of ships into leader/wingman pairs
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void BreakupIntoLeaderWingman(SelectCommand *selection)
{
    sdword numShips = selection->numShips;
    udword sizeofFindwingmen;
    FindWingmenInfo *findwingmen;
    sdword *flags;
    vector dist;
    real32 distsqr;
    Ship *compareto;
    Ship *wingman;
    sdword wingmanj;
    sdword i,j;
    Ship *leader;
    real32 mindist;
    Ship *ship;

    sizeofFindwingmen = sizeofFindWingmenInfo(numShips);
    findwingmen = memAlloc(sizeofFindwingmen,"findwingmen",0);

    findwingmen->numShips = numShips;
    for (i=0,flags=&findwingmen->flags[0];i<numShips;i++,flags++)
    {
        ship = selection->ShipPtr[i];

        if (ship->staticinfo->custshipheader.CustShipActiveAttackCommand == shipSharedFlightManAttackCommand &&
            ship->tacticstype != Aggressive &&
            ((gamerand() & 255) <= FIGHTER_PAIRUP_PROB))
        {
            *flags = 0;
        }
        else
        {
            *flags = WINGMANSHIP_IGNORE;
        }
    }

    for (i=0;i<numShips;i++)
    {
        if (findwingmen->flags[i])
        {
            continue;
        }

        leader = selection->ShipPtr[i];

        mindist = REALlyBig;
        wingmanj = 0;
        for (j=i+1;j<numShips;j++)
        {
            if (findwingmen->flags[j])
            {
                continue;
            }

            compareto = selection->ShipPtr[j];

            vecSub(dist,leader->posinfo.position,compareto->posinfo.position);
            distsqr = vecMagnitudeSquared(dist);

            if (distsqr >= WINGMAN_JOIN_MAX_DIST)
            {
                continue;
            }

            if (distsqr < mindist)
            {
                mindist = distsqr;
                wingmanj = j;
            }
        }

        if (wingmanj != 0)
        {
            // found a wingman
            wingman = selection->ShipPtr[wingmanj];
            findwingmen->flags[wingmanj] |= WINGMANSHIP_USED;

            leader->attackvars.myLeaderIs = NULL;
            leader->attackvars.myWingmanIs = wingman;
            wingman->attackvars.myLeaderIs = leader;
            wingman->attackvars.myWingmanIs = NULL;
        }
    }

    memFree(findwingmen);
}

typedef struct
{
    sdword numFlaggedShips;
    sdword flags[1];
} AssignTargetsInfo;

#define sizeofAssignTargetsInfo(n) (sizeof(AssignTargetsInfo) + (((n)-1)*sizeof(sdword)))

/*-----------------------------------------------------------------------------
    Name        : InitExtraAttackInfo
    Description : intializes the extra attack information in the ships based on attackers (selection)
                  and ships getting attacked (attack)
    Inputs      : selection, attack
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void InitExtraAttackInfo(SelectCommand *selection,AttackCommand *attack,bool32 findwingmen)
{
    sdword numShips = selection->numShips;
    sdword numTargetsToAttack = attack->numTargets;
    sdword i,j,k;
    SpecificAttackVars *attackvars;
    sdword sizeofAssigntargets;
    AssignTargetsInfo *assigntargets;
    sdword closestshipj;
    vector dist;
    real32 distsqr;
    real32 mindist;
    SpaceObjRotImpTarg *target;
    Ship *ship;
    sdword *flags;
    vector avgposSelection = { 0.0f, 0.0f, 0.0f };
    vector avgposTargets = { 0.0f, 0.0f, 0.0f };
    vector diff;
    real32 temp;
    Ship *leader = selection->ShipPtr[0];
    bool32 focusfire = FALSE;

    dbgAssert(numShips > 0);
    dbgAssert(numTargetsToAttack > 0);

    if (leader->tacticstype == Aggressive)
    {
        focusfire = TRUE;
    }

    for (i=0;i<numShips;i++)
    {
        ship = selection->ShipPtr[i];
        //dbgAssert(ship->attackvars.multipleAttackTargets == NULL);
        ship->attackvars.multipleAttackTargets = NULL;
        ship->attackvars.attacktarget = NULL;
        ship->attackvars.myLeaderIs = NULL;
        ship->attackvars.myWingmanIs = NULL;
        ship->attackvars.flightmansLeft = 0;
        ship->attackvars.attacksituation = 0;
        ship->attackvars.attackevents = 0;
        bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE | CATA_RUNTIME_BROKEN_FORMATION);
        bitClear(ship->specialFlags, SPECIAL_AttackFromAbove);

        ship->tacticsNeedToFlightMan = FALSE;
        ship->tacticsTalk = 0;

        vecAddTo(avgposSelection,ship->posinfo.position);
    }

    for (i=0;i<numTargetsToAttack;i++)
    {
        target = attack->TargetPtr[i];
        vecAddTo(avgposTargets,target->posinfo.position);
    }

    vecDivideByScalar(avgposSelection,((real32)numShips),temp);
    vecDivideByScalar(avgposTargets,((real32)numTargetsToAttack),temp);
    vecSub(diff,avgposSelection,avgposTargets);
    diff.z = abs(diff.z);

    if ((diff.z >= ATTACKING_FROM_ABOVE_MIN_DIST) &&
        (diff.z >= (fsqrt(diff.x*diff.x + diff.y*diff.y)*ATTACKING_FROM_ABOVE_MIN_RATIO)))
    {
        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            bitSet(ship->specialFlags,SPECIAL_AttackFromAbove);
        }
    }

    // break fighters up into leader/wingman.
    if (findwingmen)
    {
        BreakupIntoLeaderWingman(selection);
    }

    if (numTargetsToAttack == 1)
    {
        for (i=0;i<numShips;i++)
        {
            attackvars = &selection->ShipPtr[i]->attackvars;
            if (!IAmAWingman(attackvars))
            {
                {
                    SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                    attackvars->attacktarget = attack->TargetPtr[0];
                }
            }
        }
    }
    else
    {
        sizeofAssigntargets = sizeofAssignTargetsInfo(numTargetsToAttack);
        assigntargets = memAlloc(sizeofAssigntargets,"assigntargets",Pyrophoric);
        memset(assigntargets,0,sizeofAssigntargets);

        for (i=0;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            attackvars = &ship->attackvars;
            if (IAmAWingman(attackvars))
            {
                continue;
            }

            closestshipj = -1;
            mindist = REALlyBig;
            for (j=0,flags=&assigntargets->flags[0];j<numTargetsToAttack;j++,flags++)
            {
                if (*flags != 0)
                {
                    continue;
                }

                target = attack->TargetPtr[j];

                vecSub(dist,ship->posinfo.position,target->posinfo.position);
                distsqr = vecMagnitudeSquared(dist);

                if (distsqr < mindist)
                {
                    mindist = distsqr;
                    closestshipj = j;
                }
            }

            dbgAssert(closestshipj != -1);

            dbgAssert(assigntargets->flags[closestshipj] == 0);
            assigntargets->flags[closestshipj] = 1;
            assigntargets->numFlaggedShips++;
            dbgAssert(assigntargets->numFlaggedShips <= numTargetsToAttack);
            if (assigntargets->numFlaggedShips == numTargetsToAttack)
            {
                for (k=0,flags=&assigntargets->flags[0];k<numTargetsToAttack;k++,flags++)
                {
                    *flags = 0;
                }
                assigntargets->numFlaggedShips = 0;
            }

            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = attack->TargetPtr[closestshipj];
            }

            if (focusfire && (i > 0) && leader->attackvars.attacktarget)
            {
                SpaceObjRotImpTarg *oldTarget = attackvars->attacktarget;
                attackvars->attacktarget = leader->attackvars.attacktarget;     // override for focus fire
            }

            if (((ShipStaticInfo *)(ship->staticinfo))->canTargetMultipleTargets)
            {
                dbgAssert(attackvars->multipleAttackTargets == NULL);
                attackvars->multipleAttackTargets = allocateMultipleAttackTargets(ship);
                pickMultipleAttackTargets(ship,attack);
            }

        }

        memFree(assigntargets);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToAttack
    Description : changes order to attack
    Inputs      : alreadycommand, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToAttack(CommandToDo *alreadycommand,AttackCommand *attackcom)
{
    AttackCommand *attack;
    udword sizeofattack;
    SelectCommand *selection = alreadycommand->selection;
    bool32 dontresetattack = FALSE;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (alreadycommand->ordertype.order == COMMAND_MP_HYPERSPACEING)
    {
        return;     // don't want to attack under any circumstances if hyperspacing!
    }

    if (alreadycommand->ordertype.order == COMMAND_ATTACK)
    {
        if (SelectionsAreTotallyEquivalent((SelectCommand *)alreadycommand->attack,(SelectCommand *)attackcom))
        {
            if(alreadycommand->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
            {
                bitClear(alreadycommand->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING);
            }
            return;
        }
        dontresetattack = TRUE;
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nReceived Order to change target of attack");
#endif
    }
    else
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nChanging command to attack");
#endif
    }

    dbgAssert(AreAllShipsAttackCapable(alreadycommand->selection));

    FreeLastOrder(alreadycommand);

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    // Retail (ChangeOrderToAttack @ 0x448460) does not call InitExtraAttackInfo here -- HW1's
    // ChangeOrderToAttack did (src\Game\CommandLayer.c:5950). Cataclysm instead
    // calls InitShipsForAI + SetupExtraAttackInfo (raw disasm 0x448512-0x448524: push attack as
    // "fresh", push selection, call InitShipsForAI; then push 1 "findwingmen", push attack, push
    // selection, call SetupExtraAttackInfo, Cataclysm's own near-identical twin of
    // InitExtraAttackInfo -- same focus-fire/closest-target reassignment logic, migrated to
    // specialFlags1C8 for its kamikaze/broken-formation/attack-from-above flags).
    InitShipsForAI(selection,TRUE);
    SetupExtraAttackInfo(selection,attack,TRUE);

    alreadycommand->ordertype.order = COMMAND_ATTACK;
    alreadycommand->attack = attack;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        alreadycommand->formation.doneInitialAttack = FALSE;
    }

    PrepareShipsForCommand(alreadycommand,TRUE);

    if (dontresetattack)
    {
        sdword i;

        for (i = 0; i < selection->numShips; i++)
        {
            *(ubyte *)&selection->ShipPtr[i]->specialFlags = 0;
            selection->ShipPtr[i]->aistatecommand = 0;
        }
    }
    else
    {
        sdword i;

        for (i = 0; i < selection->numShips; i++)
        {
            *(ubyte *)&selection->ShipPtr[i]->specialFlags = 0;
            selection->ShipPtr[i]->aistateattack = 0;
            selection->ShipPtr[i]->aistatecommand = 0;
        }
    }
}

/*=============================================================================
    Auto-attack resolution subsystem (Cataclysm-only; no HW1 equivalent).

    Reconstructed from retail cataclysm.exe decompilation. This subsystem is
    what univSetupShipForControl (univupdate.c) calls to route a raw list of
    "ships that just rocked me" / "nearby hostile ships" through to an actual
    attack CommandToDo, replacing the old HW1 tacticsDelegateAttackCommand /
    tacticsDelegateSingleAttack call sites that retail no longer uses here.
=============================================================================*/

static bool32 clShipIsNotMothership(Ship *ship)
{
    return ship->shiptype != sMothership && ship->shiptype != bMothership;
}

static bool32 shipIsSectOrBeastAcolyte(Ship *ship)
{
    return ship->shiptype == sAcolyte || ship->shiptype == bAcolyte;
}

// Averages the positions of all ships in selection into *avgPosition.
static void selSelectionGetAveragePosition(SelectCommand *selection, vector *avgPosition)
{
    sdword i;

    if (selection->numShips <= 1)
    {
        *avgPosition = selection->ShipPtr[0]->posinfo.position;
        return;
    }

    avgPosition->x = avgPosition->y = avgPosition->z = 0.0f;
    for (i = 0; i < selection->numShips; i++)
    {
        vecAddTo(*avgPosition, selection->ShipPtr[i]->posinfo.position);
    }
    vecMultiplyByScalar(*avgPosition, 1.0f / (real32)selection->numShips);
}

// Filters selection in-place (swap-remove), keeping only ships owned by the
// same player as selection->ShipPtr[0].
static void MakeShipsSameOwnerAsFirstShip(SelectCommand *selection)
{
    Player *owner;
    sdword i;

    if (selection->numShips <= 0)
    {
        return;
    }

    owner = selection->ShipPtr[0]->playerowner;
    for (i = 1; i < selection->numShips; )
    {
        if (selection->ShipPtr[i]->playerowner != owner)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
        }
        else
        {
            i++;
        }
    }

    ioUpdateShipTotals();
}

// Unions two raw target lists into dest, de-duplicating by pointer.
static void clMergeUniqueShipLists(AttackCommandMax6 *dest, AttackCommand *listA, AttackCommand *listB)
{
    sdword i, j;
    bool32 alreadyIn;

    dest->numTargets = 0;

    if (listA != NULL)
    {
        for (i = 0; i < listA->numTargets && dest->numTargets < 6; i++)
        {
            dest->TargetPtr[dest->numTargets++] = listA->TargetPtr[i];
        }
    }

    if (listB != NULL)
    {
        for (i = 0; i < listB->numTargets && dest->numTargets < 6; i++)
        {
            alreadyIn = FALSE;
            for (j = 0; j < dest->numTargets; j++)
            {
                if (dest->TargetPtr[j] == listB->TargetPtr[i])
                {
                    alreadyIn = TRUE;
                    break;
                }
            }
            if (!alreadyIn)
            {
                dest->TargetPtr[dest->numTargets++] = listB->TargetPtr[i];
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clIsAttackTargetEligible
    Description : returns TRUE if target is a legal auto-attack target for player
                  (fog-of-war visible if player given, not a crystal, not a
                  mission-scripted special target, not derelict/junk, and inside
                  the playable universe bounds).
-----------------------------------------------------------------------------*/
static bool32 clIsAttackTargetEligible(Player *player, SpaceObjRotImpTarg *target)
{
    bool32 eligible = TRUE;
    Ship *targetShip;

    if (player != NULL)
    {
        eligible = shipGetSlotStatus((Ship *)target, player->playerIndex) != 0;
    }

    if (IsCrystal((Ship *)target))
    {
        return FALSE;
    }

    if (target->objtype != OBJ_ShipType)
    {
        return FALSE;
    }

    targetShip = (Ship *)target;

    if (targetShip->specialTargetShip != NULL)
    {
        return FALSE;
    }

    switch (targetShip->shiptype)
    {
        // derelict/junk ship types are never valid auto-attack targets
        case 110: case 111: case 115: case 116: case 117: case 118:
        case 119: case 120: case 121: case 122: case 123: case 141:
            return FALSE;
        default:
            break;
    }

    if (target->posinfo.position.x < -smUniverseSizeX || target->posinfo.position.x > smUniverseSizeX ||
        target->posinfo.position.y < -smUniverseSizeY || target->posinfo.position.y > smUniverseSizeY ||
        target->posinfo.position.z < -smUniverseSizeZ || target->posinfo.position.z > smUniverseSizeZ)
    {
        return FALSE;
    }

    return eligible;
}

/*-----------------------------------------------------------------------------
    Name        : clAutoAttackBlockedByFriendlyStatus
    Description : returns TRUE if ship (or its formation, or command's
                  selection) has specialFlags1C8 BIT7 set, blocking auto-attack.
-----------------------------------------------------------------------------*/
static bool32 clAutoAttackBlockedByFriendlyStatus(Ship *ship, CommandToDo *command)
{
    sdword i;
    SelectCommand *selection;

    if (ship->specialFlags1C8 & BIT7)
    {
        return TRUE;
    }

    if (ship->command != NULL &&
        (ship->command->ordertype.attributes & COMMAND_MASK_FORMATION) &&
        ship->command->selection != NULL &&
        ship->command->selection->numShips > 1)
    {
        selection = ship->command->selection;
        for (i = 0; i < selection->numShips; i++)
        {
            if (selection->ShipPtr[i]->specialFlags1C8 & BIT7)
            {
                return TRUE;
            }
        }
    }

    if (command != NULL && command->selection != NULL && command->selection->numShips > 1)
    {
        selection = command->selection;
        for (i = 0; i < selection->numShips; i++)
        {
            if (selection->ShipPtr[i]->specialFlags1C8 & BIT7)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : clShipCanAttackNow
    Description : returns TRUE if ship is not currently doing something that
                  should preclude it from being handed an auto-attack order.
-----------------------------------------------------------------------------*/
static bool32 clShipCanAttackNow(Ship *ship)
{
    bool32 blocked;

    blocked = shipIsRammingFrigateActive(ship) ||
              salCapIsCapturing(ship) ||
              acvIsDeploying(ship) ||
              mothershipCanFireBigGun(ship) ||
              (ship->specialFlags1C8 & BIT7) != 0;

    // TODO: retail also ORs in a LAUNCHSHIP-related clause here gated on an
    // unresolved ShipStaticInfo field and specialFlags1C8 bit 0x800; omitted
    // conservatively (see ChangeOrderToAttack parity notes) since including a
    // guess risks blocking legitimate attacks, whereas omitting it only makes
    // ships eligible to attack slightly sooner than retail in a narrow case.

    return !blocked;
}

/*-----------------------------------------------------------------------------
    Name        : clAttackTargetsInPassiveRange
    Description : Neutral-tactics ship path: filters targets to eligible ones,
                  merges with the ship's existing attack list (if any), and if
                  the resulting selection is in passive attack range, hands it
                  to clAttackTargetsForSelection.
-----------------------------------------------------------------------------*/
static void clAttackTargetsInPassiveRange(Ship *ship, CommandToDo *command, AttackCommand *targets)
{
    sdword i;
    AttackCommandMax6 merged;
    MaxSelection selection;
    SelectCommand *selectcom = (SelectCommand *)&selection;

    for (i = 0; i < targets->numTargets; )
    {
        if (!clIsAttackTargetEligible(ship->playerowner, targets->TargetPtr[i]))
        {
            targets->numTargets--;
            targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
        }
        else
        {
            i++;
        }
    }

    if (targets->numTargets <= 0 || !isShipPassiveAttackCapable(ship))
    {
        return;
    }

    clMergeUniqueShipLists(&merged, command != NULL ? command->attack : NULL, targets);
    if (merged.numTargets <= 0)
    {
        return;
    }

    if (command != NULL && command->selection != NULL)
    {
        selSelectionCopyBounded((MaxSelection *)selectcom, (MaxSelection *)command->selection);
    }
    else
    {
        selectcom->numShips = 1;
        selectcom->ShipPtr[0] = ship;
    }

    if (ShipsInPassiveAttackRange(selectcom, (AttackCommand *)&merged))
    {
        clAttackTargetsForSelection(&universe.mainCommandLayer.todolist, selectcom, (AttackCommand *)&merged);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clMoveAttackersIntoPassiveRange
    Description : Evasive-tactics ship path: moves ship away from its targets'
                  average position (into passive attack range) before falling
                  through to clAttackTargetsInPassiveRange.
-----------------------------------------------------------------------------*/
static void clMoveAttackersIntoPassiveRange(Ship *ship, CommandToDo *command, AttackCommand *targets)
{
    sdword i;
    MaxSelection selection;
    SelectCommand *selectcom = (SelectCommand *)&selection;
    vector avgPosition, direction, destination;
    real32 mag;

    if (command == NULL || command->ordertype.order != COMMAND_DOCK)
    {
        for (i = 0; i < targets->numTargets; )
        {
            if (!clIsAttackTargetEligible(ship->playerowner, targets->TargetPtr[i]))
            {
                targets->numTargets--;
                targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            }
            else
            {
                i++;
            }
        }

        if (targets->numTargets > 0)
        {
            if (command != NULL && command->selection != NULL)
            {
                selSelectionCopyBounded((MaxSelection *)selectcom, (MaxSelection *)command->selection);
            }
            else
            {
                selectcom->numShips = 1;
                selectcom->ShipPtr[0] = ship;
            }

            MakeShipsOnlyFollowConstraints(selectcom, clShipIsNotMothership);
            MakeShipsOnlyFollowConstraints(selectcom, clShipCanAttackNow);
            MakeShipsSameOwnerAsFirstShip(selectcom);

            if (selectcom->numShips > 0)
            {
                selSelectionGetAveragePosition(selectcom, &avgPosition);
                vecSub(direction, ship->posinfo.position, avgPosition);
                mag = fsqrt(vecMagnitudeSquared(direction));
                if (mag > 0.0f)
                {
                    vecMultiplyByScalar(direction, 1.0f / mag);
                }
                vecScalarMultiply(destination, direction, univGetMaxHealth(ship, FALSE));
                vecAddTo(destination, ship->posinfo.position);
                univClipPointToWorldBox(&ship->posinfo.position, &destination);

                clMove(&universe.mainCommandLayer, selectcom, clBuildMovePointCommand(ship->posinfo.position, &destination));
            }
        }
    }

    clAttackTargetsInPassiveRange(ship, command, targets);
}

/*-----------------------------------------------------------------------------
    Name        : clAutoAttackShipTargets
    Description : Dispatches a raw auto-attack target list based on ship's
                  current order and tactics setting.
-----------------------------------------------------------------------------*/
static void clAutoAttackShipTargets(Ship *ship, CommandToDo *command, AttackCommand *rawTargets)
{
    AttackCommand *targets;
    udword sizeoftargets;
    bool32 alreadyProtecting;
    bool32 suppressAttack;

    if (rawTargets == NULL ||
        clAutoAttackBlockedByFriendlyStatus(ship, command) ||
        (ship->flags & SOF_Hide))
    {
        return;
    }

    alreadyProtecting = FALSE;
    if (command != NULL)
    {
        switch (command->ordertype.order)
        {
            case 2: case 3: case 4: case 5: case 7: case 8:
            case 10: case 11: case 12: case 15: case 17:
                return;     // ship is busy with an order that blocks auto-attack
            default:
                alreadyProtecting = (command->ordertype.attributes & COMMAND_MASK_PROTECTING) != 0;
                break;
        }
    }

    // Mimics/Sect+Beast Acolytes linked into a module chain don't auto-attack
    suppressAttack = shipIsSectOrBeastAcolyte(ship) && ship->clampInfo != NULL;
    if ((ship->shiptype == sMimic && ship->clampInfo != NULL) || suppressAttack)
    {
        return;
    }

    sizeoftargets = sizeofAttackCommand(rawTargets->numTargets);
    targets = memAlloc(sizeoftargets, "AutoAttackTargets", 0);
    memcpy(targets, rawTargets, sizeoftargets);

    switch (ship->tacticstype)
    {
        case Evasive:
            clMoveAttackersIntoPassiveRange(ship, command, targets);
            memFree(targets);
            break;

        case Neutral:
            clAttackTargetsInPassiveRange(ship, command, targets);
            memFree(targets);
            break;

        case Aggressive:
        {
            MaxSelection selection;
            SelectCommand *selectcom = (SelectCommand *)&selection;

            if (!alreadyProtecting)
            {
                sdword i;

                for (i = 0; i < targets->numTargets; )
                {
                    if (!clIsAttackTargetEligible(ship->playerowner, targets->TargetPtr[i]))
                    {
                        targets->numTargets--;
                        targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
                    }
                    else
                    {
                        i++;
                    }
                }

                if (command != NULL && command->selection != NULL)
                {
                    MakeShipsAttackCapable(selectcom, command->selection);
                }
                else
                {
                    selectcom->numShips = 1;
                    selectcom->ShipPtr[0] = ship;
                    MakeShipsAttackCapable(selectcom, selectcom);
                }

                MakeShipsOnlyFollowConstraints(selectcom, clShipCanAttackNow);
                MakeShipsSameOwnerAsFirstShip(selectcom);

                if (selectcom->numShips > 0 &&
                    (command == NULL || command->ordertype.order != COMMAND_DOCK))
                {
                    clAttack(&universe.mainCommandLayer, selectcom, targets);
                }
            }
            memFree(targets);
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clResolveAutoAttackShip
    Description : Resolves ship to the ship that should actually receive the
                  auto-attack order (e.g. a formation leader), when ship isn't
                  independently controllable.
-----------------------------------------------------------------------------*/
static Ship *clResolveAutoAttackShip(Ship *ship)
{
    bool32 isIndependent;

    isIndependent = TRUE;
    if (ship->command != NULL &&
        (ship->command->ordertype.attributes & COMMAND_MASK_FORMATION) &&
        ship->command->selection != NULL &&
        ship != ship->command->selection->ShipPtr[0])
    {
        isIndependent = FALSE;
    }
    else if (ship->staticinfo->shipclass == CLASS_Component && ship->clampInfo != NULL)
    {
        isIndependent = FALSE;
    }

    if (ship->shiptype == dNomadFieldEmitter && !isIndependent)
    {
        // TODO: retail calls nebRemoveTendril(...) here with arguments that
        // are ambiguous in the decompile (uninitialized-looking stack locals);
        // omitted conservatively rather than guessing its parameters.
    }

    return ship;
}

/*-----------------------------------------------------------------------------
    Name        : clAutoAttackResolvedShipTargets
    Description : Entry point for the auto-attack subsystem. Resolves ship to
                  the ship that should receive the order (formation leader,
                  clamped module host, etc.) before dispatching the target list.
    Inputs      : ship - the ship the raw target list logically belongs to
                  command - ship's current CommandToDo, or NULL if idle
                  rawTargets - the raw (unfiltered) list of targets to consider
-----------------------------------------------------------------------------*/
void clAutoAttackResolvedShipTargets(Ship *ship, CommandToDo *command, AttackCommand *rawTargets)
{
    Ship *resolvedShip = ship;
    CommandToDo *resolvedCommand = command;
    bool32 isIndependent;

    isIndependent = TRUE;
    if (ship->command != NULL &&
        (ship->command->ordertype.attributes & COMMAND_MASK_FORMATION) &&
        ship->command->selection != NULL &&
        ship != ship->command->selection->ShipPtr[0])
    {
        isIndependent = FALSE;
    }
    else if (ship->staticinfo->shipclass == CLASS_Component && ship->clampInfo != NULL)
    {
        isIndependent = FALSE;
    }

    if (ship->shiptype == dNomadFieldEmitter || !isIndependent)
    {
        resolvedShip = clResolveAutoAttackShip(ship);
        resolvedCommand = getShipAndItsCommand(&universe.mainCommandLayer, resolvedShip);
    }

    clAutoAttackShipTargets(resolvedShip, resolvedCommand, rawTargets);
}

/*-----------------------------------------------------------------------------
    Name        : SetupExtraAttackInfo
    Description : Cataclysm's replacement for HW1's InitExtraAttackInfo (see
                  ChangeOrderToAttack/clAttackThese, which call this instead --
                  ChangeOrderToPassiveAttack/clPassiveAttack still call the
                  original, unchanged InitExtraAttackInfo): resets each ship's
                  per-attack runtime state for a new attack order, flags the
                  selection with CATA_RUNTIME_ATTACK_FROM_ABOVE (Cataclysm's
                  specialFlags1C8 equivalent of HW1's SPECIAL_AttackFromAbove)
                  when the attack run closes mostly along the depth axis
                  (head-on/from-behind) rather than broadside -- same geometry
                  check and script-configurable ATTACKING_FROM_ABOVE_MIN_DIST/
                  RATIO thresholds as HW1 -- splits the selection into
                  leader/wingman pairs, and assigns each independent
                  (non-wingman) ship an attack target, fairly round-robining
                  targets when there is more than one and populating per-gun
                  targets via pickMultipleAttackTargets for ships that can
                  target multiple enemies at once.
    Inputs      : selection - ships receiving the new attack order
                  attack - target list being assigned
                  findwingmen - TRUE to run BreakupIntoLeaderWingman first
-----------------------------------------------------------------------------*/
static void SetupExtraAttackInfo(SelectCommand *selection, AttackCommand *attack, bool32 findwingmen)
{
    sdword numShips = selection->numShips;
    sdword numTargetsToAttack = attack->numTargets;
    sdword i, j, k;
    SpecificAttackVars *attackvars;
    sdword sizeofAssigntargets;
    AssignTargetsInfo *assigntargets;
    sdword closestshipj;
    vector dist;
    real32 distsqr;
    real32 mindist;
    SpaceObjRotImpTarg *target;
    Ship *ship;
    sdword *flags;
    vector avgposSelection = {0.0f, 0.0f, 0.0f};
    vector avgposTargets = {0.0f, 0.0f, 0.0f};
    vector diff;
    real32 temp;
    Ship *leader = selection->ShipPtr[0];
    bool32 focusfire = FALSE;

    dbgAssert(numShips > 0);
    dbgAssert(numTargetsToAttack > 0);

    if (leader->tacticstype == Aggressive)
    {
        focusfire = TRUE;
    }

    for (i = 0; i < numShips; i++)
    {
        ship = selection->ShipPtr[i];

        ship->attackvars.multipleAttackTargets = NULL;
        ship->attackvars.attacktarget = NULL;
        ship->attackvars.myLeaderIs = NULL;
        ship->attackvars.myWingmanIs = NULL;
        ship->attackvars.flightmansLeft = 0;
        ship->attackvars.attacksituation = 0;
        ship->attackvars.attackevents = 0;
        bitClear(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE | CATA_RUNTIME_BROKEN_FORMATION | CATA_RUNTIME_ATTACK_FROM_ABOVE);

        ship->tacticsNeedToFlightMan = FALSE;
        ship->tacticsTalk = 0;

        vecAddTo(avgposSelection, ship->posinfo.position);
    }

    for (i = 0; i < numTargetsToAttack; i++)
    {
        target = attack->TargetPtr[i];
        vecAddTo(avgposTargets, target->posinfo.position);
    }

    vecDivideByScalar(avgposSelection, ((real32)numShips), temp);
    vecDivideByScalar(avgposTargets, ((real32)numTargetsToAttack), temp);
    vecSub(diff, avgposSelection, avgposTargets);
    diff.z = (real32)fabs(diff.z);

    if ((diff.z >= ATTACKING_FROM_ABOVE_MIN_DIST) &&
        (diff.z >= (fsqrt(diff.x * diff.x + diff.y * diff.y) * ATTACKING_FROM_ABOVE_MIN_RATIO)))
    {
        for (i = 0; i < numShips; i++)
        {
            ship = selection->ShipPtr[i];
            bitSet(ship->specialFlags1C8, CATA_RUNTIME_ATTACK_FROM_ABOVE);
        }
    }

    // break fighters up into leader/wingman.
    if (findwingmen)
    {
        BreakupIntoLeaderWingman(selection);
    }

    if (numTargetsToAttack == 1)
    {
        for (i = 0; i < numShips; i++)
        {
            attackvars = &selection->ShipPtr[i]->attackvars;
            if (!IAmAWingman(attackvars))
            {
                attackvars->attacktarget = attack->TargetPtr[0];
            }
        }
    }
    else
    {
        sizeofAssigntargets = sizeofAssignTargetsInfo(numTargetsToAttack);
        assigntargets = memAlloc(sizeofAssigntargets, "assigntargets", Pyrophoric);
        memset(assigntargets, 0, sizeofAssigntargets);

        for (i = 0; i < numShips; i++)
        {
            ship = selection->ShipPtr[i];
            attackvars = &ship->attackvars;
            if (IAmAWingman(attackvars))
            {
                continue;
            }

            closestshipj = -1;
            mindist = REALlyBig;
            for (j = 0, flags = &assigntargets->flags[0]; j < numTargetsToAttack; j++, flags++)
            {
                if (*flags != 0)
                {
                    continue;
                }

                target = attack->TargetPtr[j];

                vecSub(dist, ship->posinfo.position, target->posinfo.position);
                distsqr = vecMagnitudeSquared(dist);

                if (distsqr < mindist)
                {
                    mindist = distsqr;
                    closestshipj = j;
                }
            }

            dbgAssert(closestshipj != -1);

            dbgAssert(assigntargets->flags[closestshipj] == 0);
            assigntargets->flags[closestshipj] = 1;
            assigntargets->numFlaggedShips++;
            dbgAssert(assigntargets->numFlaggedShips <= numTargetsToAttack);
            if (assigntargets->numFlaggedShips == numTargetsToAttack)
            {
                for (k = 0, flags = &assigntargets->flags[0]; k < numTargetsToAttack; k++, flags++)
                {
                    *flags = 0;
                }
                assigntargets->numFlaggedShips = 0;
            }

            attackvars->attacktarget = attack->TargetPtr[closestshipj];

            if (focusfire && (i > 0) && leader->attackvars.attacktarget)
            {
                attackvars->attacktarget = leader->attackvars.attacktarget;     // override for focus fire
            }

            if (((ShipStaticInfo *)(ship->staticinfo))->canTargetMultipleTargets)
            {
                dbgAssert(attackvars->multipleAttackTargets == NULL);
                attackvars->multipleAttackTargets = allocateMultipleAttackTargets(ship);
                pickMultipleAttackTargets(ship, attack);
            }
        }

        memFree(assigntargets);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToAttackTargets
    Description : Cataclysm-only: merges a new raw target list into an
                  existing CommandToDo's passive-attack list (used by
                  clAttackTargetsForSelection when a matching command already
                  exists in the command layer's todolist).
-----------------------------------------------------------------------------*/
static void ChangeOrderToAttackTargets(CommandToDo *command, AttackCommand *attackcom)
{
    SelectCommand *selection = command->selection;
    sdword numUnderConstruction = 0;
    sdword i;
    udword sizeofattack;
    AttackCommand *newattack;

    for (i = 0; i < selection->numShips; i++)
    {
        if (selection->ShipPtr[i]->buildTimerElapsed > universe.totaltimeelapsed)
        {
            numUnderConstruction++;
        }
    }

    // if more than half the selection is still under construction, don't
    // retask them yet
    if ((real32)numUnderConstruction / (real32)selection->numShips > 0.5f)
    {
        return;
    }

    if (command->ordertype.attributes & COMMAND_MASK_PASSIVE_ATTACKING)
    {
        AttackCleanup(command);
        memFree(command->attack);
        command->ordertype.attributes &= ~COMMAND_MASK_PASSIVE_ATTACKING;
        command->attack = NULL;
    }
    else if (command->ordertype.order == COMMAND_ATTACK)
    {
        AttackCleanup(command);
        if (command->attack != NULL)
        {
            memFree(command->attack);
            command->attack = NULL;
        }
    }

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    newattack = memAlloc(sizeofattack, "AttackTargetsForSelection", 0);
    memcpy(newattack, attackcom, sizeofattack);

    SetupExtraAttackInfo(selection, newattack, FALSE);

    command->attack = newattack;
    command->ordertype.attributes = (command->ordertype.attributes & ~(COMMAND_MASK_PROTECTING | COMMAND_MASK_HOLDING_PATTERN)) | COMMAND_MASK_PASSIVE_ATTACKING;

    for (i = 0; i < selection->numShips; i++)
    {
        *(ubyte *)&selection->ShipPtr[i]->specialFlags = 0;
        selection->ShipPtr[i]->aistateattack = 0;
    }

    for (i = 0; i < selection->numShips; i++)
    {
        selection->ShipPtr[i]->command = command;
        // TODO: retail also clears an ambiguous per-ship byte here (rendered
        // by Hex-Rays as ship[1].objlink.structptr, i.e. tail-storage past
        // Ship, at the same raw offset (1592) referenced below in
        // clAttackTargetsForSelection); left alone since neither the field
        // nor its retail-race/order gating (RACE_Taiidan && COMMAND_SPECIAL)
        // could be confirmed against our struct layout.
    }

    tacticsMakeShipsNotLookForOtherShips(command->selection);

    if (command->ordertype.order == COMMAND_NULL && (command->ordertype.attributes & COMMAND_MASK_FORMATION))
    {
        clAttackStoreTargetCoordsys(command, FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clAttackTargetsForSelection
    Description : Finds (or creates) a CommandToDo tracking selectcom's
                  auto-attack of attackcom, used by the Neutral/Evasive
                  tactics auto-attack paths.
    TODO        : Level C. A handful of fields in the busy-ship pruning pass
                  and the battle-chatter gating in the merge path are
                  ambiguous in the retail decompile (raw tail-storage offsets
                  past Ship/CommandToDo that couldn't be confirmed against our
                  struct layout) and have been conservatively omitted rather
                  than guessed; see individual TODOs below.
-----------------------------------------------------------------------------*/
static void clAttackTargetsForSelection(LinkedList *todolist, SelectCommand *selectcom, AttackCommand *attackcom)
{
    Node *node;
    CommandToDo *matchingCommand = NULL;
    CommandToDo *newcommand;
    SelectCommand *selection;
    AttackCommand *attack;
    udword sizeofselection, sizeofattack;
    sdword numUnderConstruction, i;

    // TODO: retail also prunes selectcom in-place here, removing ships that
    // are already "busy" with one of attackcom's targets (tested via a raw
    // tail-storage field on the target and a Node::prev->next comparison
    // that couldn't be confirmed against our struct layout). Omitted
    // conservatively; worst case a ship gets (re)assigned to a command it's
    // already effectively part of.

    if (selectcom->numShips <= 0 || attackcom->numTargets <= 0)
    {
        return;
    }

    for (node = todolist->head; node != NULL; node = node->next)
    {
        CommandToDo *candidate = (CommandToDo *)listGetStructOfNode(node);
        if (SelectionsAreEquivalent(selectcom, candidate->selection))
        {
            matchingCommand = candidate;
            break;
        }
    }

    if (matchingCommand == NULL)
    {
        numUnderConstruction = 0;
        for (i = 0; i < selectcom->numShips; i++)
        {
            if (selectcom->ShipPtr[i]->buildTimerElapsed > universe.totaltimeelapsed)
            {
                numUnderConstruction++;
            }
        }

        if ((real32)numUnderConstruction / (real32)selectcom->numShips > 0.5f)
        {
            return;
        }

        newcommand = memAlloc(sizeof(CommandToDo), "AutoAttackToDo", NonVolatile);
        InitCommandToDo(newcommand);

        sizeofselection = sizeofSelectCommand(selectcom->numShips);
        selection = memAlloc(sizeofselection, "AutoAttackToDoSelection", NonVolatile);
        memcpy(selection, selectcom, sizeofselection);

        sizeofattack = sizeofAttackCommand(attackcom->numTargets);
        attack = memAlloc(sizeofattack, "AutoAttackToDoAttack", 0);
        memcpy(attack, attackcom, sizeofattack);

        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, selection);
        SetupExtraAttackInfo(selection, attack, FALSE);

        newcommand->selection = selection;
        newcommand->ordertype.order = COMMAND_NULL;
        newcommand->ordertype.attributes = COMMAND_MASK_PASSIVE_ATTACKING;
        newcommand->attack = attack;

        for (i = 0; i < selection->numShips; i++)
        {
            *(ubyte *)&selection->ShipPtr[i]->specialFlags = 0;
            selection->ShipPtr[i]->aistateattack = 0;
            selection->ShipPtr[i]->command = newcommand;
            // TODO: see ChangeOrderToAttackTargets - same ambiguous
            // tail-storage byte write omitted here for the same reason.
        }

        tacticsMakeShipsNotLookForOtherShips(selection);

        listAddNode(todolist, &newcommand->todonode, newcommand);
        return;
    }

    if (matchingCommand->ordertype.attributes & COMMAND_MASK_HOLDING_PATTERN)
    {
        return;
    }

    if (matchingCommand->ordertype.attributes & COMMAND_MASK_PASSIVE_ATTACKING)
    {
        if (matchingCommand->attack == NULL)
        {
            return;
        }
        if (!SelectionsAreEquivalent(matchingCommand->attack, attackcom))
        {
            ChangeOrderToAttackTargets(matchingCommand, attackcom);
        }
        return;
    }

    if ((matchingCommand->ordertype.attributes & COMMAND_MASK_FORMATION) &&
        matchingCommand->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
    {
        return;
    }

    switch (matchingCommand->ordertype.order)
    {
        case COMMAND_ATTACK: case COMMAND_DOCK: case COMMAND_LAUNCH_SHIP:
        case COMMAND_COLLECT_RESOURCES: case COMMAND_BUILDING_SHIP:
        case COMMAND_SPECIAL: case COMMAND_MP_HYPERSPACING: case COMMAND_LINK:
        case COMMAND_LINKED_LAUNCH: case COMMAND_SIEGECANNON: case COMMAND_EXTERNALCONSTRUCTION:
        case COMMAND_RESEARCH:
            return;     // busy with an order that blocks auto-attack merging
        default:
            ChangeOrderToAttackTargets(matchingCommand, attackcom);
            break;
    }

    // TODO: retail also fires battle chatter (battleChatterFleetAttempt /
    // battleChatterAttempt) here for the local player's selection, gated on
    // an ambiguous bitmask field and a cooldown timer; omitted since it is
    // speech-only and has no gameplay effect.
}

/*-----------------------------------------------------------------------------
    Name        : clAttackThese
    Description : Command to attack
    Inputs      : selectcom, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
CommandToDo *clAttackThese(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    Node *node;
    SelectCommand *selection;
    AttackCommand *attack;
    udword sizeofselection;
    udword sizeofattack;
    sdword i;
    sdword targetIndex;
    Ship *ship;
    bool32 selectionChanged;

    // for safety
    if (!AreAllShipsAttackCapable(selectcom))
    {
        MakeShipsAttackCapable(selectcom,selectcom);
        dbgMessage("\nWARNING: ships not capable of attacking told to attack");
    }

    selectionChanged = FALSE;
    for (i = 0; i < selectcom->numShips; )
    {
        ship = selectcom->ShipPtr[i];
        for (targetIndex = 0; targetIndex < attackcom->numTargets; targetIndex++)
        {
            if (clAttackTargetKeepsShipBusy(ship, attackcom->TargetPtr[targetIndex]))
            {
                selectcom->ShipPtr[i] = selectcom->ShipPtr[selectcom->numShips - 1];
                selectcom->numShips--;
                selectionChanged = TRUE;
                break;
            }
        }
        if (targetIndex == attackcom->numTargets)
        {
            i++;
        }
    }

    if (selectcom->numShips == 0)
    {
        return NULL;
    }

    if (attackcom->numTargets <= 0)
    {
        dbgMessage("\nWARNING: NO SHIPS TO ATTACK.");
        return NULL;
    }

    alreadycommand = NULL;
    for (node = comlayer->todolist.head; node != NULL; node = node->next)
    {
        CommandToDo *candidate;

        candidate = (CommandToDo *)listGetStructOfNode(node);
        if (SelectionsAreEquivalent(selectcom, candidate->selection))
        {
            alreadycommand = candidate;
            break;
        }
    }

    if (alreadycommand != NULL && !selectionChanged)
    {
        if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            ClearProtecting(alreadycommand);
        }
        ChangeOrderToAttack(alreadycommand,attackcom);

        //link in animation to open guns here:
        //madLinkInOpenGuns(alreadycommand,1);

        return alreadycommand;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to attack");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    // Retail (clAttackThese @ 0x448620) does not call InitExtraAttackInfo here -- same
    // Cataclysm-vs-HW1 divergence documented in ChangeOrderToAttack above. It calls
    // InitShipsForAI then SetupExtraAttackInfo before installing the new command's fields.
    InitShipsForAI(selection,TRUE);
    SetupExtraAttackInfo(selection,attack,TRUE);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_ATTACK;
    newcommand->ordertype.attributes = 0;
    newcommand->attack = attack;

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    //link in animation to open guns here:
    //madLinkInOpenGuns(newcommand,1);


    return newcommand;
}

/*-----------------------------------------------------------------------------
    Name        : clAttack
    Description : Command to attack
    Inputs      : selectcom, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc2(copycom)", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);
    clFilterShipsForOrders(copycom);
    MakeShipsOnlyFollowConstraints(copycom, ShipCanAcceptAttackOrder);

    if (!AreAllShipsAttackCapable(copycom))
    {
        MakeShipsAttackCapable(copycom,selectcom);
        dbgMessage("\nWARNING: ships not capable of attacking told to attack");
    }

    if (attackcom->numTargets <= 0)
    {
        dbgMessage("\nWARNING: NO SHIPS TO ATTACK.");
        memFree(copycom);
        return;
    }

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to move (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {
            MakeShipsNotIncludeTheseShips(copycom,command->selection);

            if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
            {
                ClearProtecting(command);
            }

            if (command->ordertype.order == COMMAND_MOVE && command->move.heading.x != 0.0f)
            {
                memcpy(&command->cataCommandState0[244], &command->move, sizeof(MoveCommand));
                *(sdword *)&command->cataCommandState0[240] = 1;
            }

            ChangeOrderToAttack(command,attackcom);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        clAttackThese(comlayer,copycom,attackcom);
    }

    memFree(copycom);
}
/*-----------------------------------------------------------------------------
    Name        : canChangeOrderToPassiveAttack
    Description : returns TRUE if alreadycommand can passive attack
    Inputs      :
    Outputs     :
    Return      : returns TRUE if alreadycommand can passive attack
----------------------------------------------------------------------------*/
bool32 canChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attack)
{
    if (alreadycommand->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN)
    {
        return FALSE;
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        if ((alreadycommand->attack == NULL) || (attack == NULL))
        {
            return FALSE;
        }

        if (!SelectionsAreEquivalent((SelectCommand *)alreadycommand->attack,(SelectCommand *)attack))
        {
            return TRUE;
        }
    }

    switch (alreadycommand->ordertype.order)
    {
        case COMMAND_NULL:
        case COMMAND_HALT:
        case COMMAND_MILITARYPARADE:
        case COMMAND_LINK:
        case COMMAND_EXTERNALCONSTRUCTION:
        case COMMAND_SIEGECANNON:
            return TRUE;

        case COMMAND_MOVE:
        case COMMAND_DOCK:
#ifdef DEBUG_TACTICS
            if(tacticsOn)
#endif
            {
                if (!tacticsAreStrikeCraftInSelection(alreadycommand->selection))
                    return TRUE;        //this is a new exceptions
                else
                    return FALSE;
            }
        case COMMAND_ATTACK:
        case COMMAND_MP_HYPERSPACEING:
            return FALSE;
        case COMMAND_LAUNCHSHIP:
        case COMMAND_COLLECTRESOURCE:
        case COMMAND_SPECIAL:
        case COMMAND_BUILDINGSHIP:
            return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToPassiveAttack
    Description : Activates passive attack for command
    Inputs      : alreadycommand, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToPassiveAttack(CommandToDo *alreadycommand,AttackCommand *attackcom)
{
    AttackCommand *attack;
    udword sizeofattack;
    SelectCommand *selection = alreadycommand->selection;
    sdword count,i;

    // should check canChangeOrderToPassiveAttack before calling this routine
    dbgAssert(canChangeOrderToPassiveAttack(alreadycommand,attackcom));
    count=0;
    for(i=0;i<alreadycommand->selection->numShips;i++)
    {
        if(alreadycommand->selection->ShipPtr[i]->passiveAttackCancelTimer > universe.totaltimeelapsed)
        {
            //count a bad ship
            count++;
        }
    }
    if( (((real32) count)/((real32) alreadycommand->selection->numShips)) > 0.5f)
    {
        //more than 50% in group bad, so don't passive attack
        return;
    }

#ifdef gshaw
    dbgMessage("\nChanging Order to passive attack");
#endif

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    InitExtraAttackInfo(selection,attack,FALSE);

    alreadycommand->ordertype.attributes |= COMMAND_IS_PASSIVEATTACKING;
    bitClear(alreadycommand->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);      // don't do holding pattern if passive attacking
    alreadycommand->attack = attack;

    InitShipsForAI(alreadycommand->selection,FALSE);

    PrepareShipsForCommand(alreadycommand,FALSE);

    //setup formation properly here
    if(alreadycommand->ordertype.order == COMMAND_NULL)
    {
        if(alreadycommand->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            //NULL command and in formation
            lockFormation(alreadycommand,0);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clPassiveAttack
    Description : Command to attack
    Inputs      : selectcom, attackcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clPassiveAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    AttackCommand *attack;
    udword sizeofselection;
    udword sizeofattack;
    sdword count,i;

    if (selectcom->numShips == 0)
    {
#ifdef gshaw
        dbgMessage("\nNo Ships For Passive attack");
#endif
        return;
    }
    if (attackcom->numTargets <= 0)
    {
        dbgMessage("\nWARNING: NO SHIPS TO PASSIVE ATTACK.");
        return;
    }

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (canChangeOrderToPassiveAttack(alreadycommand,attackcom))
        {
            ChangeOrderToPassiveAttack(alreadycommand,attackcom);
        }
        else
        {
            dbgMessage("\nWARNING: Couldn't change order to passive attack");
        }
        return;
    }

#ifdef gshaw
    dbgMessage("\nReceived Order to passive attack");
#endif

    count=0;
    for(i=0;i<selectcom->numShips;i++)
    {
        if(selectcom->ShipPtr[i]->passiveAttackCancelTimer > universe.totaltimeelapsed)
        {
            //count a bad ship
            count++;
        }
    }
    if( (((real32) count)/((real32) selectcom->numShips)) > 0.5f)
    {
        //more than 50% in group bad, so don't passive attack
        return;
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    sizeofattack = sizeofAttackCommand(attackcom->numTargets);
    attack = memAlloc(sizeofattack,"ToDoAttack",0);
    memcpy(attack,attackcom,sizeofattack);

    InitExtraAttackInfo(selection,attack,FALSE);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_NULL;
    newcommand->ordertype.attributes = COMMAND_IS_PASSIVEATTACKING;
    newcommand->attack = attack;

    InitShipsForAI(selection,FALSE);

    PrepareShipsForCommand(newcommand,FALSE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToSpecial
    Description : changes order to special
    Inputs      : alreadycommand, targets
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToSpecial(CommandToDo *alreadycommand,SpecialCommand *targets)
{
    SelectCommand *selection = alreadycommand->selection;
    SpecialCommand *specialtargets;
    udword sizeofspecialtargets;

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    FreeLastOrder(alreadycommand);

    if (targets != NULL)
    {
        sizeofspecialtargets = sizeofSelectCommand(targets->numTargets);
        specialtargets = memAlloc(sizeofspecialtargets,"ToDoSpecial",0);
        memcpy(specialtargets,targets,sizeofspecialtargets);
        alreadycommand->specialtargets = specialtargets;
    }
    else
    {
        alreadycommand->specialtargets = NULL;
    }

    alreadycommand->ordertype.order = COMMAND_SPECIAL;

    PrepareShipsForCommand(alreadycommand,TRUE);
}

bool32 ShipsAreSalCapCorvettes(Ship *ship)
{
    return ShipTypeIsSalCapCorvetteType(ship->shiptype);
}

void clPlaySpeechEventsForSupportCraft(SelectCommand *selection)
{
    Ship **iter;
    sdword i;

    iter = selection->ShipPtr;
    if (selection->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
    {
        i = 0;
        if (selection->numShips > 0)
        {
            while (2)
            {
                switch ((*iter)->shiptype)
                {
                    case 0xE:
                    case 0x31:
                    case 0x48:
                    case 0x59:
                        speechEventQueue(
                            selection->ShipPtr[i],
                            67111062,
                            selection->ShipPtr[i]->shiptype,
                            -1, -1, -1, -1, 0.0f, -1);
                        break;
                    default:
                        ++i;
                        ++iter;
                        if (i < selection->numShips)
                            continue;
                        break;
                }
                break;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSpecial
    Description : Command to do special
    Inputs      : selectcom, targets
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    CommandToDo *formationCommand;
    SelectCommand *selection;
    SpecialCommand *specialtargets;
    TypeOfFormation formationType;
    udword sizeofselection;
    udword sizeofspecialtargets;
    bool32 preserveFormation;

    if (selectcom->numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships for special");
#endif
        return;
    }

    preserveFormation = FALSE;
    formationType = (TypeOfFormation)-1;
    if (selectcom->ShipPtr[0]->shiptype == sAcolyte ||
        selectcom->ShipPtr[0]->shiptype == sACV ||
        selectcom->ShipPtr[0]->shiptype == bAcolyte ||
        selectcom->ShipPtr[0]->shiptype == bACV)
    {
        formationCommand = selectcom->ShipPtr[0]->command;
        if (formationCommand != NULL &&
            (formationCommand->ordertype.attributes & COMMAND_IS_FORMATION))
        {
            preserveFormation = TRUE;
            formationType = formationCommand->formation.formationtype;
        }
    }

    if(selectcom->numShips == 1)
    {
        if(ShipTypeIsSalCapCorvetteType(selectcom->ShipPtr[0]->shiptype))
        {
            CommandToDo *oldcommand=getShipAndItsCommand(&universe.mainCommandLayer,selectcom->ShipPtr[0]);
            if(oldcommand != NULL)
            {
                if(oldcommand->ordertype.order == COMMAND_SPECIAL)
                {
                    if((((SalCapCorvetteSpec *)selectcom->ShipPtr[0]->ShipSpecifics)->target) != NULL)
                    {
                        if(ShipInSelection((SelectCommand *)targets,(Ship *)(((SalCapCorvetteSpec *)selectcom->ShipPtr[0]->ShipSpecifics)->target)))
                        {
                            //ship is targetting a ship in the list already...lets ignore the command

                            //if we get bugs..we could only do this if the target list is the same exactly
                            return;
                        }
                    }
                }

            }

        }
    }

    sizeofselection = sizeofSelectCommand(selectcom->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    if (targets == NULL)
    {
        // this is just a special activate command, so activate it for each ship and return without modifying
        // the command layer as long as the specialActivateIsContinuous flag isn't set.
        sdword numShips = selectcom->numShips;
        sdword i;
        Ship *ship;
        ShipStaticInfo *shipstaticinfo;

        for (i=0;i<numShips;i++)
        {
            ship = selectcom->ShipPtr[i];
            shipstaticinfo = (ShipStaticInfo *)(ship->staticinfo);

            if (!shipstaticinfo->specialActivateIsContinuous)
            {
                clRemoveShipFromSelection(selection,ship);

                if (shipstaticinfo->custshipheader.CustShipSpecialActivateNoTarget != NULL)
                {
                    shipstaticinfo->custshipheader.CustShipSpecialActivateNoTarget(ship);
                }

            }
        }

        if (selection->numShips == 0)
        {
            // all ships have been activated, so return
            memFree(selection);
            return;
        }
    }

    if (targets != NULL ||
        !DoAnyShipsFollowConstraints(selection,ShipsAreSalCapCorvettes))        // Bryce later make salcap corvettes be able to salvage in formation fix later
    {
        if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selection)) != NULL)
        {
            memFree(selection);
            ChangeOrderToSpecial(alreadycommand,targets);
            return;
        }
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to do special");
#endif

    RemoveShipsFromDoingStuff(comlayer,selection);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    if (targets != NULL)
    {
        sizeofspecialtargets = sizeofSelectCommand(targets->numTargets);
        specialtargets = memAlloc(sizeofspecialtargets,"ToDoSpecial",0);
        memcpy(specialtargets,targets,sizeofspecialtargets);
        newcommand->specialtargets = specialtargets;
    }
    else
    {
        newcommand->specialtargets = NULL;
    }

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_SPECIAL;
    newcommand->ordertype.attributes = 0;
    PrepareShipsForCommand(newcommand,TRUE);
    if (preserveFormation)
    {
        newcommand->ordertype.attributes |= COMMAND_IS_FORMATION;
        newcommand->formation.formationtype = formationType;
        newcommand->formation.formationSpecificInfo = NULL;
        formationInit(newcommand, formationType, NULL, TRUE);
        PrepareShipsForCommand(newcommand,TRUE);
    }
    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

void clCloak(CommandLayer *comlayer, SelectCommand *selectcom, AttackCommand *attackcom, bool32 cloak)
{
    sdword numShips;
    Ship **iter;
    sdword i;
    Ship *ship;
    sdword j;
    Ship **v9;

    numShips = selectcom->numShips;
    if (numShips > 0 && !cloak)
    {
        iter = selectcom->ShipPtr;
        if (selectcom->ShipPtr[0]->playerowner->playerIndex == universe.curPlayerIndex)
        {
            i = 0;
            if (numShips > 0)
            {
                while (2)
                {
                    switch ((*iter)->shiprace)
                    {
                        case 0xC:
                        case 0xE:
                        case 0x2F:
                        case 0x31:
                        case 0x3C:
                        case 0x3E:
                        case 0x4A:
                        case 0x4C:
                        case 0x58:
                            ship = selectcom->ShipPtr[i];
                            if (!(ship->specialFlags2 & BIT20))
                                speechEventQueue(ship, 33556626, -1, -1, -1, -1, -1, 0.0f, -1);
                            break;
                        default:
                            ++i;
                            ++iter;
                            if (i < numShips)
                                continue;
                            break;
                    }
                    break;
                }
            }
        }
    }

    clSpecial(comlayer, selectcom, (SpecialCommand *)attackcom);

    j = 0;
    if (selectcom->numShips > 0)
    {
        v9 = selectcom->ShipPtr;
        do
        {
            if (shipIsAliveCarrierType(*v9))
                ((WorkerShipSpecifics *)(*v9)->ShipSpecifics)->unk_08 = (cloak != 0);
            ++j;
            ++v9;
        }
        while (j < selectcom->numShips);
    }
}

void tacticsChangedForShipCB(Ship *ship,TacticsType oldtactics,TacticsType newtactics)
{
#if 0               // take this out due to micromanagement abuse
    if(newtactics == Evasive)
    {
        tacticsSetShipToDoDodge(ship);
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : clSetTactics
    Description : Sets tactics for a group of ships
    Inputs      : selectcom, tacticstype
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;

#ifdef DEBUG_TACTICS
    if(!tacticsOn) return;  //don't change tactics settings if not enabled
#endif

    dbgAssert(tacticstype >= 0);
    dbgAssert(tacticstype < NUM_TACTICS_TYPES);

    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        //adjust ships maximum bullet range (don't forget to adjust
        //individual guns ranges on the fly
        /*
        if(ship->staticinfo->shipclass == CLASS_Fighter)
        {
            ship->bulletrage*=tacticsInfo.BulletRangeBonus[CLASS_Fighter][ship->tacticstype];
        }
        else if (ship->staticinfo->shipclass == CLASS_Corvette)
        {
            ship->bulletrage*=tacticsInfo.BulletRangeBonus[Tactics_Corvette][ship->tacticstype];
        }
        */

        if (tacticstype == Evasive &&
            ship->tacticstype != Evasive &&
            ship->attackvars.myLeaderIs != NULL)
        {
            ship->attackvars.attacktarget = NULL;
        }
        ship->tacticstype = tacticstype;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clRUTransfer
    Description : Transfers RU's from selectcom.numShips to selectcom.timeLastStatus
    Inputs      : selectcom,resourceUntis
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clRUTransfer(CommandLayer *comlayer, sdword toIndex, sdword fromIndex, sdword resourceUnits, ubyte flags)
{
    char CommandMessage[512];
    sdword newResourceUnits;

    if (resourceUnits > 0
        && (flags < RUTRANS_BUILDCANCEL || flags > RUTRANS_GENERATEDRUS)
        && universe.players[toIndex].playerState != PLAYER_DEAD
        && universe.players[fromIndex].playerState != PLAYER_DEAD)
    {
        newResourceUnits = universe.players[fromIndex].resourceUnits - resourceUnits;
        if (newResourceUnits >= 0)
        {
            universe.players[fromIndex].resourceUnits = newResourceUnits;
            universe.players[toIndex].resourceUnits += resourceUnits;

            universe.gameStats.playerStats[fromIndex].typeOfPlayerDeath -= resourceUnits;
            universe.gameStats.playerStats[fromIndex].totalResourceUnitsSpent += resourceUnits;

            universe.gameStats.playerStats[toIndex].typeOfPlayerDeath += resourceUnits;
            universe.gameStats.playerStats[toIndex].totalResourceUnitsGiven += resourceUnits;

            if (fromIndex == universe.curPlayerIndex)
            {
                speechEventQueue(0, 67113029, -1, -1, 0, fromIndex, -1, 5.0f, -1);
            }
            else if (toIndex == universe.curPlayerIndex)
            {
                soundEventPlay(0, 297, 0);
                speechEventQueue(0, 67113030, -1, -1, 0, toIndex, -1, 5.0f, -1);
                sprintf(CommandMessage, RUTransferReceivedFmt, resourceUnits, playerNames[fromIndex]);
                clCommandMessage(CommandMessage, -65536);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSetKamikaze
    Description : Sets the kamikaze bit for all ships that are kamikaze capable
                  provided they are doing an attack...ideally, we do that check before we transmitt
    Inputs      : selectcom,
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom)
{
     sdword numShips = selectcom->numShips;
     sdword i;
     Ship *ship;

     for (i=0;i<numShips;i++)
     {
         ship = selectcom->ShipPtr[i];
         CommandToDo *command = ship->command;
         if(command == NULL)
         {

         }
         else if(command->ordertype.order != COMMAND_ATTACK)
         {
            if(command->ordertype.order == COMMAND_SPECIAL)
            {
                if(ShipTypeIsSalCapCorvetteType(ship->shiptype))
                {
                    bitSet(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE | CATA_RUNTIME_BROKEN_FORMATION);
                    continue;
                }
            }
         }
         else
         {
            bitSet(ship->specialFlags1C8, CATA_RUNTIME_KAMIKAZE | CATA_RUNTIME_BROKEN_FORMATION);
         }
     }
}

/*-----------------------------------------------------------------------------
    Name        : ChangeOrderToHalt
    Description : changes order to Halt
    Inputs      : alreadycommand
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ChangeOrderToHalt(CommandToDo *alreadycommand)
{
    if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(alreadycommand);
    }
    if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(alreadycommand);
    }

    if (alreadycommand->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
    {
        bitClear(alreadycommand->ordertype.attributes,COMMAND_IS_ATTACKINGANDMOVING);
    }

    FreeLastOrder(alreadycommand);
    alreadycommand->ordertype.order = COMMAND_HALT;

    PrepareShipsForCommand(alreadycommand,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : clHaltThese
    Description : command for ships to halt
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clHaltThese(CommandLayer *comlayer,SelectCommand *selectcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips = selectcom->numShips;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to halt!");
#endif
        return;
    }

    salCapExtraSpecialOrderCleanUp(selectcom,COMMAND_HALT,NULL,NULL);

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadycommand->ordertype.order == COMMAND_MOVE)
        {
            speechEventQueue(
                selectcom->ShipPtr[0],
                67110914,
                selectcom->ShipPtr[0]->shiprace,
                -1,
                -1,
                -1,
                -1,
                0.0f,
                -1);
        }
        else if (alreadycommand->ordertype.order == COMMAND_ATTACK)
        {
            speechEventQueue(
                selectcom->ShipPtr[0],
                67110918,
                selectcom->ShipPtr[0]->shiprace,
                -1,
                -1,
                -1,
                -1,
                0.0f,
                -1);
        }

        #ifdef HW_DEBUG
              dbgAssert(alreadycommand->ordertype.order != COMMAND_MP_HYPERSPACEING);
        #endif


        ChangeOrderToHalt(alreadycommand);
        return;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to halt");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_HALT;
    newcommand->ordertype.attributes = 0;

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
}

sdword clUpgrade(CommandLayer *comlayer,SelectCommand *selectcom,sdword upgradecom)
{
    sdword index;
    sdword startedUpgrade = FALSE;

    clHalt(comlayer, selectcom);

    for (index = 0; index < selectcom->numShips; index++)
    {
        Ship *ship = selectcom->ShipPtr[index];
        bool32 *upgradeActive = &ship->upgradeActive;
        real32 *upgradeStartTime = &ship->upgradeStartTime;
        real32 *upgradeEndTime = &ship->upgradeEndTime;

        if (upgradecom != 0)
        {
            *upgradeActive = FALSE;
            clSetAttribute((sdword)ship, 0, 0);
        }
        else
        {
            *upgradeActive = TRUE;
            *upgradeStartTime = universe.totaltimeelapsed;
            *upgradeEndTime = shipGetUpgradeBuildTime(ship) + universe.totaltimeelapsed;
            clSetAttribute((sdword)ship, 1, 0);

            if ((shipGetAvailablePlayerUpgradeMask(ship) & 0x400) != 0)
            {
                dreadnoughtCreateCannon(ship);
            }

            if (*upgradeStartTime < *upgradeEndTime)
            {
                startedUpgrade = TRUE;
            }
            else
            {
                ShipTypeToNiceStr(ship->shiptype);
                telldir();
                *upgradeActive = FALSE;
                clSetAttribute((sdword)ship, 0, 0);
                shipApplyPlayerUpgrades(ship);
            }
        }
    }

    if (startedUpgrade)
    {
        return speechEventQueue(0, 33556628, -1, -1, 0, selectcom->ShipPtr[0]->playerowner->playerIndex, -1, 5.0f, -1);
    }
    return startedUpgrade;
}

/*-----------------------------------------------------------------------------
    Name        : clHalt
    Description : command for ships to halt
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clHalt(CommandLayer *comlayer,SelectCommand *selectcom)
{
    typedef bool32 (__cdecl *ShipHaltFilterCB)(Ship *ship);
    SelectCommand *copycom;
    udword sizeofcopycom;
    Node *curnode;
    Node *endnode;
    Node *nextnode;
    CommandToDo *command;
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    bool32 stoppassiveattacking;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to halt!");
#endif
        return;
    }

    for(i=0;i<selectcom->numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        stoppassiveattacking = FALSE;

        if (ship->command == NULL)
            stoppassiveattacking = TRUE;
        else
        {
            switch (ship->command->ordertype.order)
            {
                case COMMAND_NULL:
                case COMMAND_HALT:
                case COMMAND_MILITARYPARADE:
                case COMMAND_LINK:
                case COMMAND_EXTERNALCONSTRUCTION:
                case COMMAND_SIEGECANNON:
                    stoppassiveattacking = TRUE;
            }
        }

        if (stoppassiveattacking)
            ship->passiveAttackCancelTimer=universe.totaltimeelapsed+TW_GLOBAL_PASSIVE_ATTACK_CANCEL_MODIFIER;
        else
            ship->passiveAttackCancelTimer=0.0f;
    }

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"cc2(copycom)", Pyrophoric);
    copycom->numShips = 0;
    for (i = 0; i < selectcom->numShips; i++)
    {
        ShipHaltFilterCB haltFilterCB;

        ship = selectcom->ShipPtr[i];
        haltFilterCB = ship->staticinfo->custshipheader.CustShipHaltFilter;
        if (haltFilterCB == NULL || haltFilterCB(ship))
        {
            copycom->ShipPtr[copycom->numShips++] = ship;
        }
    }

    if (copycom->numShips == 0)
    {
        memFree(copycom);
        return;
    }

    // Now see if the selected ships are already in the command layer, and if so, individually order these groups
    // to move (to preserve formations)

    curnode = comlayer->todolist.head;
    endnode = comlayer->todolist.tail;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        nextnode = curnode->next;

        if (TheseShipsAreInSelection(command->selection,copycom))
        {
            MakeShipsNotIncludeTheseShips(copycom,command->selection);

            ChangeOrderToHalt(command);
        }

        if (copycom->numShips == 0)
        {
            break;
        }

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }

    // tell rest of ships left to move too.
    if (copycom->numShips != 0)
    {
        clHaltThese(comlayer,copycom);
    }

    memFree(copycom);
}

/*-----------------------------------------------------------------------------
    Name        : clScuttle
    Description : command for ship to scuttle itself
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clScuttle(CommandLayer *comlayer,SelectCommand *selectcom)
{
    sdword numShips = selectcom->numShips;
    sdword i;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bScuttle)
        return;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to scuttle!");
#endif
        return;
    }

    for (i=0;i<numShips;i++)
    {
        Ship *ship = selectcom->ShipPtr[i];

        if (tutorial != TUTORIAL_ONLY || ship->shiptype != sMothership)
        {
            if ((ship->flags2 & BIT7) == 0 || ship->statusWindow.activeUntil == 0.0f)
            {
                ship->deathtime = universe.totaltimeelapsed + frandombetween(SCUTTLE_SHIPDEATHTIMEMIN,SCUTTLE_SHIPDEATHTIMEMAX);
            }
        }
        if ((ship->flags2 & BIT3) != 0)
        {
            shipDisable(ship, FALSE);
        }
    }

    return;
}

/*-----------------------------------------------------------------------------
    Name        : clLaunchShip
    Description : command for a ship to launch from a ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clLaunchShip(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr receiverShip)
{
    typedef bool32 (__cdecl *ShipLaunchFilterCB)(Ship *ship, Ship *receiverShip);
    CommandToDo *newcommand;
    SelectCommand *selection;
    Ship *ship = selectcom->ShipPtr[0];
    ShipStaticInfo *receiverShipStaticInfo = (ShipStaticInfo *)receiverShip->staticinfo;
    ShipLaunchFilterCB launchFilterCB;
    bool32 receiverBlocksLaunch = FALSE;

    dbgAssert(selectcom->numShips == 1);
    dbgAssert(receiverShip != NULL);
    dbgAssert(receiverShipStaticInfo->canReceiveShips|receiverShipStaticInfo->canReceiveShipsPermanently);
    if (!ship->staticinfo->rotateToRetaliate &&
        (receiverShip->attributes & ATTRIBUTES_HeadShotKillerCollDamage))
    {
        receiverBlocksLaunch = TRUE;
    }

    launchFilterCB = (ShipLaunchFilterCB)ship->staticinfo->custshipheader.CustShip_UnknownSlot28;
    if (launchFilterCB != NULL && !launchFilterCB(ship, receiverShip))
    {
        return;
    }

    if (receiverBlocksLaunch)
    {
        return;
    }

    if ((ship->specialFlags1C8 & BIT25) != 0)
    {
        return;
    }

    if (ship->shiplink.belongto != NULL)
    {
        return;         // already outside
    }

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    selection = memAlloc(sizeof(SelectCommand),"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeof(SelectCommand));

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_LAUNCHSHIP;
    newcommand->ordertype.attributes = 0;

    newcommand->launchship.receiverShip = receiverShip;

    dockPrepareSingleShipForLaunch(ship, receiverShip, DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP);

    InitShipsForAI(selection,TRUE);

    PrepareShipsForCommand(newcommand,TRUE);

    //listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    //insert at begining to take priority over other commands that will be occurring
    //i.e. retiring
    listAddNodeBeginning(&comlayer->todolist,&newcommand->todonode,newcommand);
}

// Cataclysm-specific: launches a selection of already-docked Beast ACV/MCV linked
// ships by building/reusing a COMMAND_LINKED_LAUNCH command directly (retail
// clLaunchShipSelection @0x44d7a0). This used to route through clDock/COMMAND_DOCK,
// which incorrectly ran dockProcessLinkedFission in the wrong command context and
// corrupted the SpaceObjList for unrelated ships.
void clLaunchShipSelection(CommandLayer *comlayer, SelectCommand *selectcom)
{
    CommandToDo *command;
    CommandToDo *existing;
    SelectCommand *launchSelection;
    MaxSelection filtered;
    udword sizeofSelection;
    sdword i;
    bool32 isNewCommand;

    if (selectcom->numShips <= 0)
    {
        telldir();
        return;
    }

    filtered.numShips = 0;
    filtered.timeLastStatus = selectcom->timeLastStatus;
    for (i = 0; i < selectcom->numShips; i++)
    {
        Ship *ship = selectcom->ShipPtr[i];

        if (!(ship->staticinfo->shipFlags & SSTATF_LinkedLaunchCapable))
        {
            continue;
        }

        dockInitShipForLaunch(ship);
        filtered.ShipPtr[filtered.numShips++] = ship;
    }

    if (filtered.numShips == 0)
    {
        return;
    }

    existing = IsSelectionAlreadyDoingSomething(comlayer, (SelectCommand *)&filtered);
    isNewCommand = (existing == NULL);
    if (isNewCommand)
    {
        RemoveShipsFromDoingStuff(comlayer, (SelectCommand *)&filtered);
        command = memAlloc(sizeof(CommandToDo), "ToDo", NonVolatile);
        InitCommandToDo(command);
    }
    else
    {
        command = existing;
        if (command->selection != NULL)
        {
            for (i = 0; i < command->selection->numShips; i++)
            {
                Ship *ship = command->selection->ShipPtr[i];
                if (ship->cataBusyStateWindow.sentinelFormationState == 1)
                {
                    clSetAttribute((sdword)ship, 0, 5);
                }
                if (ship->shiptype == 17 || ship->shiptype == 52)
                {
                    ship->command = NULL;
                }
            }
        }
        if (command->militaryParade != NULL)
        {
            memFree(command->militaryParade);
            command->militaryParade = NULL;
        }
        FreeLastOrder(command);
    }

    sizeofSelection = sizeofSelectCommand(filtered.numShips);
    launchSelection = memAlloc(sizeofSelection, "ToDoSelection", NonVolatile);
    memcpy(launchSelection, &filtered, sizeofSelection);
    if (!isNewCommand && command->selection != NULL)
    {
        memFree(command->selection);
    }
    command->selection = launchSelection;
    command->ordertype.order = COMMAND_LINKED_LAUNCH;
    command->ordertype.attributes = 0;

    for (i = 0; i < launchSelection->numShips; i++)
    {
        Ship *ship = launchSelection->ShipPtr[i];

        ship->command = command;
        ship->unk_1D0 = 0;
        if ((ship->specialFlags1C8 & 0x10000) != 0)
        {
            rowRemoveShipFromGettingOutOfWay(ship);
        }
    }

    tacticsMakeShipsNotLookForOtherShips(launchSelection);

    for (i = 0; i < launchSelection->numShips; i++)
    {
        Ship *ship = launchSelection->ShipPtr[i];
        *((ubyte *)ship + CATA_LINK_CLEAR0_OFFSET) = 0;
        *((ubyte *)ship + CATA_LINK_CLEAR1_OFFSET) = 0;
        *((ubyte *)ship + CATA_LINK_CLEAR2_OFFSET) = 0;
    }

    if (isNewCommand)
    {
        listAddNode(&comlayer->todolist, &command->todonode, command);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clLaunchMultipleShips
    Description : command for multiple ships to launch from a ship
    Inputs      : selectcom, receiverShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    SelectCommand selectone;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ships to launch");
#endif
        return;
    }

    if (launchFrom == NULL)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo Ship to launch from");
#endif
        return;
    }

    selectone.numShips = 1;
    for (i=0;i<numShips;i++)
    {
        if (!(selectcom->ShipPtr[i]->attributes & ATTRIBUTES_Defector))      // don't let the defector launch
        {
            selectone.ShipPtr[0] = selectcom->ShipPtr[i];
            clLaunchShip(comlayer,&selectone,launchFrom);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : LaunchAllInternalShipsOfPlayerThatMustBeLaunched
    Description : launches all internal ships of given player that must be launched
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void LaunchAllInternalShipsOfPlayerThatMustBeLaunched(struct Player *player)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player &&
            !(ship->specialFlags & SPECIAL_Hyperspacing))
        {
            ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;

            if (shipsInsideMe != NULL)
            {
                Node *insidenode = shipsInsideMe->insideList.head;
                InsideShip *insideship;
                SelectCommand selectone;

                while (insidenode != NULL)
                {
                    insideship = (InsideShip *)listGetStructOfNode(insidenode);
                    dbgAssert(insideship->ship->objtype == OBJ_ShipType);

                        // make defector launch
                    if ((insideship->ship->attributes & ATTRIBUTES_Defector) || (ShipHasToLaunch(ship, insideship->ship)))
                    {
                        selectone.numShips = 1;
                        selectone.ShipPtr[0] = insideship->ship;
                        clLaunchShip(&universe.mainCommandLayer,&selectone,ship);
                    }

                    insidenode = insidenode->next;
                }
            }
        }

        objnode = objnode->next;
    }
}

void FillInCarrierMothershipInfo(struct Player *player,Ship **mothership,Ship *carrierX[])
{
    sdword i;
    sdword numCarriersFound = 0;

    *mothership = NULL;
    carrierX[0] = NULL;
    carrierX[1] = NULL;
    carrierX[2] = NULL;

    if (player->PlayerMothership != NULL &&
        player->PlayerMothership->staticinfo->shipclass == CLASS_Mothership)
    {
        *mothership = player->PlayerMothership;
    }

    for (i = 0; i < cmNumCarriers; i++)
    {
        if (cmCarriers[i].owner == player && numCarriersFound < 3)
        {
            carrierX[numCarriersFound] = cmCarriers[i].ship;
            numCarriersFound++;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : LaunchAllInternalShipsOfPlayer
    Description : launches all internal ships of given player
    Inputs      : carriermask: 0=momship, 1=carrier1, 2=carrier2
    Outputs     :
    Return      : the number of ships launched
----------------------------------------------------------------------------*/
sdword LaunchAllInternalShipsOfPlayer(struct Player *player, udword carriermask)
{
    Node *objnode = universe.ShipList.head;
    Ship *ship;
    Ship *mothership;
    Ship *carriers[3];
    sdword numShips = 0;
    FillInCarrierMothershipInfo(player,&mothership,carriers);

    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            if ((ship->shipsInsideMe != NULL) && !ShipTypeIsDefenseFieldFrigateType(ship->shiptype))
            {
                if ( ((carriermask & BIT0) && (ship == mothership)) ||
                     ((carriermask & BIT1) && (ship == carriers[0])) ||
                     ((carriermask & BIT2) && (ship == carriers[1])) )
                {
                    Node *insidenode = ship->shipsInsideMe->insideList.head;
                    InsideShip *insideship;
                    SelectCommand selectone;

                    while (insidenode != NULL)
                    {
                        insideship = (InsideShip *)listGetStructOfNode(insidenode);
                        dbgAssert(insideship->ship->objtype == OBJ_ShipType);

                        if (!(insideship->ship->attributes & ATTRIBUTES_Defector))      // don't let the defector launch
                        {
                            selectone.numShips = 1;
                            selectone.ShipPtr[0] = insideship->ship;
                            clLaunchShip(&universe.mainCommandLayer,&selectone,ship);
                            numShips++;
                        }

                        insidenode = insidenode->next;
                    }

                }

            }
        }

        objnode = objnode->next;
    }

    return (numShips);
}

/*-----------------------------------------------------------------------------
    Name        : clAutoLaunch
    Description : command to turn the autoLaunch flag on or off for a given player
    Inputs      : OnOff - bit0=Mothership, bit1=Carrier 1, bit2=Carrier2, bit3=Carrier3, bit4=Carrier4
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clAutoLaunch(udword OnOff,udword playerIndex)
{
    Player *player = &universe.players[playerIndex];
    bool32 speechevent = FALSE;
    sdword i;
    udword bit = BIT0;
    for (i=0;i<(4+1);i++,bit<<=1)       // 4 Carriers + 1 Mothership        BIT0..BIT4 are valid
    {
        if ((player->autoLaunch & bit) != (OnOff & bit))
        {
            if (OnOff & bit)
            {
                // changing autoLaunch from Off to On
                player->autoLaunch |= bit;
            }
            else
            {
                // changing autoLaunch from On to Off
                player->autoLaunch &= ~bit;
            }
        }

        if ((player->autoLaunch & bit) != 0 && LaunchAllInternalShipsOfPlayer(player, bit) > 0)
        {
            speechevent = TRUE;
        }
    }

    if (speechevent)
    {
        speechEventQueue(0, 67113064, -1, -1, 0, playerIndex, -1, 5.0f, -1);
    }
}

/*-----------------------------------------------------------------------------
    Name        : clSetAlliance
    Description : sets an alliance based on a player bitmask
    Inputs      : player bitmask
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void clSetAlliance(udword AllianceType, uword curalliance, uword newalliance)
{
    allianceSetAlliance(AllianceType, curalliance, newalliance);
}

void clSetResearch(udword type, udword playernum, udword labnum, udword tech)
{
    if (type == RESEARCH_SUBCOMMAND_START)
    {
        rmAssignPlayersLabToResearch(&universe.players[playernum], labnum, tech);
    }
    else if (type == RESEARCH_SUBCOMMAND_STOP)
    {
        rmClearResearchlab(&universe.players[playernum], labnum);
    }
}

static void clApplyResearchStatusChange(CommandToDo *command)
{
    clSetResearch(
        *clResearchType(command),
        *clResearchPlayerIndex(command),
        *clResearchLabnum(command),
        *clResearchTech(command));
}

bool32 ShipHasToLaunch(Ship *InsideShip, Ship *ship)
{
    ShipsInsideMe *shipsInsideMe;
    bool32 result;
    if (singlePlayerGame)
    {
        if (singlePlayerGameInfo.hyperspaceState != NO_HYPERSPACE)
        {
            return FALSE;
        }
    }

    if (ship->playerowner->playerIndex != InsideShip->playerowner->playerIndex)
    {
        result = TRUE;
        goto finish;
    }

    if (ship->shiptype == sSwarmer ||
        ship->shiptype == bSwarmer ||
        (ship->specialFlags & SPECIAL_STAY_TILL_EXPLICITLAUNCH))
    {
        result = FALSE;
        goto finish;
    }

    shipsInsideMe = InsideShip->shipsInsideMe;
    dbgAssert(shipsInsideMe != NULL);

    if (ship->staticinfo->shipclass == CLASS_Fighter ||
        ship->staticinfo->shipclass == CLASS_Micro)
    {
        result = (shipsInsideMe->FightersInsideme > InsideShip->staticinfo->maxDockableFighters);
        goto finish;
    }

    if (ship->staticinfo->shipclass != CLASS_Corvette &&
        !(ship->objtype == OBJ_ShipType &&
          (ship->shiptype == sWorker || ship->shiptype == bWorker)))
    {
        result = TRUE;
        goto finish;
    }

    result = (shipsInsideMe->CorvettesInsideme > InsideShip->staticinfo->maxDockableCorvettes);

finish:
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : clCreateShip
    Description : creates a new ship
    Inputs      : shipType, shipRace, playerIndex, creator
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Ship *clCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    Ship *ship;
    SelectCommand select;


    Player *player = &universe.players[playerIndex];
    Ship *mothership, *carrier1=NULL, *carrier2=NULL;
    sword i;
    udword launchmask = 0;

    mothership = player->PlayerMothership;
    if ((mothership) && mothership->staticinfo->shipclass == CLASS_Carrier)
    {
        carrier1 = mothership;
        mothership = NULL;
    }

    for (i = 0; i < cmNumCarriers; i++)
    {
        if (cmCarriers[i].owner == player)
        {
            if (carrier1)
            {
                carrier2 = cmCarriers[i].ship;
                break;
            }
            else
            {
                carrier1 = cmCarriers[i].ship;
            }
        }
    }

    if (creator == mothership)
    {
        launchmask = BIT0;
    }
    else if (creator == carrier1)
    {
        launchmask = BIT1;
    }
    else if (creator == carrier2)
    {
        launchmask = BIT2;
    }





    if (creator == NULL)
    {
        return NULL;         // if creator died, too bad for ships inside
    }

    if (universe.players[playerIndex].playerState != PLAYER_ALIVE)
    {
        return NULL;         // if player is not alive, don't allow ship creation
    }

    dbgAssert(playerIndex < universe.numPlayers);

    // create ship
    ship = univCreateShipWithContext(shipType,
                                     shipRace,
                                     &creator->posinfo.position,
                                     &universe.players[playerIndex],
                                     UNIV_CREATE_SHIP_BUILT | UNIV_CREATE_SHIP_NO_UNIVERSE_ADD,
                                     creator,
                                     0);

    gameStatsAddShip(ship,playerIndex);

    // update unit cap variables
    cat_unitCapCreateShip(ship,&universe.players[playerIndex]);

    if (shipType != sSwarmer && shipType != bSwarmer)
    {
        universe.gameStats.playerStats[playerIndex].totalForEachClass[ship->staticinfo->shipclass]++;
        if (shipType == sACV || shipType == sMCV || shipType == bACV)
        {
            universe.gameStats.playerStats[playerIndex].totalForEachClass[ship->staticinfo->shipclass]++;
        }
    }

    // update the special internet ghost totals, because of lag.
    if ((multiPlayerGame) && (playerIndex==sigsPlayerIndex))
    {
        shiplagtotals[shipType]--;
    }

    // put ship inside creator
    dockPutShipInside(ship,creator);

    {
        bool32 forceLaunch = ShipHasToLaunch(creator, ship);
        if ( (universe.players[playerIndex].autoLaunch & launchmask) ||
             forceLaunch )
        {
            select.numShips = 1;
            select.ShipPtr[0] = ship;


            clLaunchShip(comlayer,&select,(ShipPtr)creator);
        }
    }
    return(ship);
}

/*-----------------------------------------------------------------------------
    Name        : processBuildingShipToDo
    Description : processes ship building command
    Inputs      :
    Outputs     :
    Return      : TRUE if command is done
----------------------------------------------------------------------------*/
bool32 processBuildingShipToDo(CommandToDo *command)
{
    Player *player;
    Ship *creator;

    if (universe.univUpdateCounter > command->buildingship.frameAtWhichToCreate)
    {
        creator = command->buildingship.creator;
        dbgAssert(creator != NULL);
        player = creator->playerowner;

        clCreateShip(&universe.mainCommandLayer,command->buildingship.shipType,command->buildingship.shipRace,
                     player->playerIndex,creator);

        return TRUE;
    }

    return FALSE;
}

udword NumberOfOtherShipsOfSameTypeBeingBuilt(ShipType shipType,ShipPtr creator,udword playerIndex)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    udword numOtherBeingBuilt = 0;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        if ((command->ordertype.order == COMMAND_BUILDINGSHIP) && (command->buildingship.playerIndex == playerIndex))
        {
            if (command->buildingship.shipType == shipType)
            {
                // if they don't have a creator or both have the same creator
                if ( ((creator == NULL) || (command->buildingship.creator == NULL)) ||
                     (creator == command->buildingship.creator))
                {
                    numOtherBeingBuilt++;
                }
            }
        }

        curnode = curnode->next;
    }

    return numOtherBeingBuilt;
}

void clDeterministicBuild(udword command, CommandLayer* comLayer,
                          sdword numShips,
                          ShipType shipType, ShipRace shipRace,
                          uword playerIndex, ShipPtr creator)
{
    Player* player = &universe.players[playerIndex];

    dbgAssert(playerIndex < universe.numPlayers);

    if (creator == NULL)
    {
        //creator died, ships die too
        return;
    }

    if (player->playerState != PLAYER_ALIVE)
    {
        //!alive players can't build ships
        return;
    }

    cmDeterministicBuild(command, numShips, shipType, shipRace, playerIndex, creator);
}

/*-----------------------------------------------------------------------------
    Name        : clBuildShip
    Description : builds a new ship (including waiting time for it to be built)
    Inputs      : shipType, shipRace, playerIndex, creator
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    Player *player = &universe.players[playerIndex];
    ShipStaticInfo *shipstaticinfo = GetShipStaticInfo(shipType,shipRace);
    CommandToDo *newcommand;

    dbgAssert(playerIndex < universe.numPlayers);

    if (creator == NULL)
    {
        return;         // if creator died, too bad for ships inside
    }

    if (player->playerState != PLAYER_ALIVE)
    {
        return;         // if player is not alive, don't allow ship creation
    }

    //if (shipstaticinfo->buildCost <= player->resourceUnits)
    {
        player->resourceUnits -= shipstaticinfo->buildCost;     // subtract cost right away
        universe.gameStats.playerStats[player->playerIndex].totalResourceUnitsSpent += shipstaticinfo->buildCost;
        // make new command indicating delayed build:

        newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
        InitCommandToDo(newcommand);

        newcommand->selection = memAlloc(sizeofSelectCommand(1),"NoSelection",0);
        newcommand->selection->numShips = 0;
        newcommand->ordertype.order = COMMAND_BUILDINGSHIP;
        newcommand->ordertype.attributes = 0;
        newcommand->buildingship.shipType = shipType;
        newcommand->buildingship.shipRace = shipRace;
        newcommand->buildingship.creator = creator;
        newcommand->buildingship.playerIndex = (udword)playerIndex;
        newcommand->buildingship.frameAtWhichToCreate = universe.univUpdateCounter + ((NumberOfOtherShipsOfSameTypeBeingBuilt(shipType,creator,playerIndex)+1) * shipstaticinfo->buildTime);

        PrepareShipsForCommand(newcommand,TRUE);

        listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    }
}

/*-----------------------------------------------------------------------------
    Name        : FindShipsWereGuarding
    Description : finds ships we're guarding, either directly or indirectly
    Inputs      :
    Outputs     :
    Return      : returns TRUE if terminating condition was finding that shipsWereGuarding are directly or
                  indirectly guarding us - returns FALSE if terminating condition is we can't find any more ships we're guarding
----------------------------------------------------------------------------*/
bool32 FindShipsWereGuarding(CommandToDo *originalus,SelectCommand *us,GrowSelection *shipsWereGuarding,sdword iteration)
{
    sdword i,j;
    sdword usnumShips = us->numShips;
    Ship *ship;
    CommandToDo *shipcommand;
    GrowSelection newshipsWereGuarding;
    bool32 result;

    if (iteration > 10)
    {
        return FALSE;       // safety for recursive function
    }

    for (i=0;i<usnumShips;i++)
    {
        ship = us->ShipPtr[i];
        shipcommand = ship->command;
        if (shipcommand && (shipcommand->ordertype.attributes & COMMAND_IS_PROTECTING))
        {
            for (j=0;j<shipcommand->protect->numShips;j++)
            {
                growSelectAddShipNoDuplication(shipsWereGuarding,shipcommand->protect->ShipPtr[j]);
            }
        }
    }

    if (shipsWereGuarding->selection->numShips == 0)
    {
        return FALSE;
    }

    if (AnyOfTheseShipsAreInSelection(shipsWereGuarding->selection,originalus->selection))
    {
        return TRUE;
    }

    growSelectInit(&newshipsWereGuarding);

    result = FindShipsWereGuarding(originalus,shipsWereGuarding->selection,&newshipsWereGuarding,iteration+1);

    growSelectClose(&newshipsWereGuarding);

    return result;
}

/*-----------------------------------------------------------------------------
    Name        : UpdateCircularGuardFlag
    Description : Checks for circular guard, and if it is sets the Ship+0x1C8 circular-guard bit for each ship otherwise clears it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void UpdateCircularGuardFlag(CommandToDo *command)
{
    sdword i;
    GrowSelection shipsWereGuarding;

    growSelectInit(&shipsWereGuarding);

    if (FindShipsWereGuarding(command,command->selection,&shipsWereGuarding,0))
    {
        // ships we're guarding are directly or indirectly guarding us:
        SelectCommand *selection = command->selection;

        for (i=0;i<selection->numShips;i++)
        {
            bitSet(selection->ShipPtr[i]->specialFlags1C8, CATA_RUNTIME_CIRCULAR_GUARD);
        }
    }
    else
    {
        // ships we're guarding are NOT directly or indirectly guarding us:
        SelectCommand *selection = command->selection;

        for (i=0;i<selection->numShips;i++)
        {
            bitClear(selection->ShipPtr[i]->specialFlags1C8, CATA_RUNTIME_CIRCULAR_GUARD);
        }
    }

    growSelectClose(&shipsWereGuarding);
}

void CheckCircularGuard(CommandToDo *command,ProtectCommand *protectcom)
{
    ProtectCommand *protect;
    udword sizeofprotect;

    command->protectFlags = 0;

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        if (SelectionsAreTotallyEquivalent(command->protect,protectcom))
        {
            return;
        }
        ClearProtecting(command);
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nChanged Order to protect");
#endif

    FreeLastOrder(command);

    command->ordertype.order = COMMAND_NULL;
    command->ordertype.attributes |= COMMAND_IS_PROTECTING;

    sizeofprotect = sizeofProtectCommand(protectcom->numShips);
    protect = memAlloc(sizeofprotect,"ToDoProtect",0);
    memcpy(protect,protectcom,sizeofprotect);

    command->protect = protect;

    PrepareShipsForCommand(command,TRUE);
    UpdateCircularGuardFlag(command);

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        if (command->formation.formationtype == SPHERE_FORMATION ||
            command->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
        {
            formationContentHasChanged(command);
        }
    }
}

void ChangeOrderToProtect(CommandToDo *alreadycommand,ProtectCommand *protectcom)
{
    CheckCircularGuard(alreadycommand,protectcom);
}

/*-----------------------------------------------------------------------------
    Name        : clProtect
    Description : command to protect a ship
    Inputs      : selectcom, protectThisShip
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
CommandToDo *clProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    ProtectCommand *protect;
    udword sizeofselection;
    udword sizeofprotect;
    sdword numShips = selectcom->numShips;
    sdword numShipsToProtect = protectcom->numShips;

    if (numShips == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships selected as protectors");
#endif
        return NULL;
    }

    if (numShipsToProtect == 0)
    {
#ifdef DEBUG_COMMANDLAYER
        dbgMessage("\nNo ships to protect");
#endif
        return NULL;
    }

    if ((alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        CheckCircularGuard(alreadycommand,protectcom);
        return alreadycommand;
    }

#ifdef DEBUG_COMMANDLAYER
    dbgMessage("\nReceived Order to protect");
#endif

    RemoveShipsFromDoingStuff(comlayer,selectcom);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,selectcom,sizeofselection);

    sizeofprotect = sizeofProtectCommand(numShipsToProtect);
    protect = memAlloc(sizeofprotect,"ToDoProtect",0);
    memcpy(protect,protectcom,sizeofprotect);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_NULL;
    newcommand->ordertype.attributes = COMMAND_IS_PROTECTING;
    newcommand->protect = protect;
    newcommand->protectFlags = 0;

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
    UpdateCircularGuardFlag(newcommand);
    return newcommand;
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToGroup
    Description : Adds ship to CommandToDo group
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToGroup(ShipPtr ship,CommandToDo *group)
{
    SelectCommand *newselection;
    sdword sizeofnewselection;
    sdword sizeofselection;
    sdword numShips;

    numShips = group->selection->numShips;
    dbgAssert(numShips >= 1);

    sizeofnewselection = sizeofSelectCommand(numShips+1);
    sizeofselection = sizeofSelectCommand(numShips);

    newselection = memAlloc(sizeofnewselection,"ToDoSelection",NonVolatile);
    memcpy(newselection,group->selection,sizeofselection);

    memFree(group->selection);
    group->selection = newselection;

    newselection->ShipPtr[numShips] = ship;
    newselection->numShips = numShips + 1;

    PrepareOneShipForCommand(ship,group,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToFormationGroup
    Description : adds a ship to group
    Inputs      : ship, group
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToFormationGroup(ShipPtr ship,CommandToDo *group)
{
#ifdef DEBUG_LAUNCHSHIP
    dbgMessage("\nAdding ship to group");
#endif

    AddShipToGroup(ship,group);
    FillInShipFormationStuff(ship,group);

    dbgAssert(group->ordertype.attributes & COMMAND_IS_FORMATION);
    formationContentHasChanged(group);

    InitShipAI(ship,TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : GroupShipIntoMilitaryParade
    Description : groups ship into military parade around aroundShip
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void GroupShipIntoMilitaryParade(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip)
{
    CommandToDo *militaryGroup = NULL;
    Node *node;
    if (ship->staticinfo->shipclass == CLASS_Component)
    {
        return;
    }

    node = comlayer->todolist.head;
    while (node != NULL)
    {
        militaryGroup = listGetStructOfNode(node);
        if (militaryGroup->ordertype.order == COMMAND_MILITARYPARADE &&
            militaryGroup->militaryParade->aroundShip == aroundShip)
        {
            AddShipToMilitaryGroup(ship,militaryGroup);
            return;
        }
        node = node->next;
    }

    CreateMilitaryGroupAroundShip(comlayer,ship,aroundShip);
}

/*-----------------------------------------------------------------------------
    Name        : clSetMilitaryParade
    Description : sets ships into a military parade
    Inputs      : selectcom
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom)
{
    sdword index;
    sdword i;
    Ship *aroundShip;
    Ship *ship;
    CommandToDo *command;

    index = MothershipOrCarrierIndexInSelection(selectcom);
    if (index < 0)
    {
        return;
    }

    dbgAssert(index < selectcom->numShips);
    aroundShip = selectcom->ShipPtr[index];

    for (i=0;i<selectcom->numShips;i++)
    {
        if (i != index)
        {
            ship = selectcom->ShipPtr[i];
            command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
            if ((command != NULL) && (command->ordertype.order == COMMAND_MILITARYPARADE) &&
                (command->militaryParade->aroundShip  == aroundShip))
            {
                // already in military Parade around aroundShip
                // do nothing
            }
            else
            {
                SelectCommand selectone;
                selectone.ShipPtr[0] = ship;
                selectone.numShips = 1;
                RemoveShipsFromDoingStuff(comlayer,&selectone);
                GroupShipIntoMilitaryParade(comlayer,selectcom->ShipPtr[i],aroundShip);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : GroupShip
    Description : puts a ship into an appropriate group around the aroundShip
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void GroupShip(CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip)
{
    CommandToDo *shipGroup = aroundShip->command;

    if (shipGroup != NULL && (shipGroup->ordertype.attributes & COMMAND_IS_FORMATION))
    {
        AddShipToGroup(ship,shipGroup);
        formationContentHasChanged(shipGroup);
        *(ubyte *)&ship->specialFlags = 0;
        ship->aistateattack = 0;
        ship->aistatecommand = 0;
        return;
    }

    if (ship->staticinfo->shipclass == CLASS_Component)
    {
        return;
    }

    GroupShipIntoMilitaryParade(comlayer,ship,aroundShip);

#if 0
    if (groupSize == 1)
    {
        // just position at random location
        selection.numShips = 1;
        selection.ShipPtr[0] = ship;
        GetRandomLocationAroundShip(&destination,aroundShip);
        clMove(comlayer,&selection,clBuildMovePointCommand(ship->posinfo.position,&destination));
    }
    else
    {
        shipGroup = FindGroupForShip(comlayer,ship,groupSize);

        if (shipGroup == NULL)
        {
            // make new group, and position ship at random location
            selection.numShips = 1;
            selection.ShipPtr[0] = ship;
            GetRandomLocationAroundShip(&destination,aroundShip);
            clFormation(comlayer,&selection,DELTA_FORMATION);
            clMove(comlayer,&selection,clBuildMovePointCommand(ship->posinfo.position,&destination));
        }
        else
        {
            // put ship in that group
            AddShipToFormationGroup(ship,shipGroup);
        }
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : clShipDied
    Description : Call this function to update ship command layer when ship dies
    Inputs      : deadship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clShipDied(CommandLayer *comlayer,ShipPtr deadship)
{
    CommandToDo *militaryGroup;
    SelectCommand selection;
    MaxSelection militarySelection;

    militaryGroup = GetMilitaryGroupAroundShip(comlayer,deadship);
    if (militaryGroup != NULL)
    {
        selSelectionCopyBounded(&militarySelection,(MaxSelection *)militaryGroup->selection);
        RemoveShipsFromDoingStuff(comlayer,(SelectCommand *)&militarySelection);
    }

    selection.numShips = 1;
    selection.ShipPtr[0] = deadship;

    RemoveShipsFromDoingStuff(comlayer,&selection);

    RemoveShipFromBeingTargeted(comlayer,deadship,REMOVE_PROTECT,0);

    // Update player unit caps info
    cat_unitCapDeleteShip_(deadship,deadship->playerowner);

    if (deadship->shiprace == RACE_Sect)
    {
        ModuleDestroyed(deadship->playerowner,deadship->shiptype);
    }
    else if (deadship->shiptype == bMothership)
    {
        BeastMothershipDestroyed(deadship->playerowner);
    }
}

/*-----------------------------------------------------------------------------
    Name        : TargetWithinAttackRange
    Description : returns TRUE if ship within attack range of target
    Inputs      : ship,target
    Outputs     :
    Return      : returns TRUE if ship within attack range of target
----------------------------------------------------------------------------*/
bool32 TargetWithinAttackRange(Ship *ship,SpaceObjRotImpTarg *target)
{
    vector trajectory;
    real32 range;

    aishipGetTrajectory(ship,target,&trajectory);
    range = RangeToTarget(ship,target,&trajectory);

    if (range < 10000.0f)
    {
        return TRUE;        // consider within attack range sooner
    }
#if 0
    if(((ShipStaticInfo *)(ship->staticinfo))->bulletRange[ship->tacticstype] == 0.0f)
    {
        if(ship->shiptype == CloakGenerator ||
            ship->shiptype == DFGFrigate ||
            ship->shiptype == GravWellGenerator)
        {
            if(range < 7000.0f)
            {
                return TRUE;
            }
        }
    }
#endif
    if (range < ((ShipStaticInfo *)(ship->staticinfo))->bulletRange[ship->tacticstype])
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void putFightersInSelection(MaxSelection *dst,MaxSelection *src)
{
    sdword i,k;
    k=0;
    for(i=0;i<src->numShips;i++)
    {
        if(src->ShipPtr[i]->staticinfo->shipclass == CLASS_Fighter)
        {
            dst->ShipPtr[k] = src->ShipPtr[i];
            k++;
        }
    }
    dst->numShips = k;
}
void putCorvettesInSelection(MaxSelection *dst,MaxSelection *src)
{
    sdword i,k;
    k=0;
    for(i=0;i<src->numShips;i++)
    {
        if(src->ShipPtr[i]->staticinfo->shipclass == CLASS_Corvette)
        {
            dst->ShipPtr[k] = src->ShipPtr[i];
            k++;
        }
    }
    dst->numShips = k;
}
void putCapitalsInSelection(MaxSelection *dst,MaxSelection *src)
{
    sdword i,k;
    k=0;
    for(i=0;i<src->numShips;i++)
    {
        if(src->ShipPtr[i]->staticinfo->shipclass != CLASS_Fighter
           && src->ShipPtr[i]->staticinfo->shipclass != CLASS_Corvette)
        {
            dst->ShipPtr[k] = src->ShipPtr[i];
            k++;
        }
    }
    dst->numShips = k;
}



/*-----------------------------------------------------------------------------
    Name        : clHoldingPattern
    Description : Puts command into a holding pattern by
                  breaking into small formations n' such.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#define MAX_UNORDERED_GROUP_SIZE    11  //maximum ships in a group after
                                        //attacking {ships ungrouped previously)

void clHoldingPattern(CommandLayer *comlayer,CommandToDo *command)
{
    bool32 needToPassiveAttack = FALSE;
    bool32 needToProtect = FALSE;
    MaxSelection passiveAttackSelection;
    MaxSelection protectSelection;
    MaxSelection fighters,corvettes,capitals;
    MaxSelection working;
    MaxSelection *curselection;
    sdword i,j,k;

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        return;         // already in formation, don't need to split them up into deltas
    }

    if(command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        if(command->attack->numTargets > 0)
        {
            //if group was passive attacking, set a flag so we remember that that was the case
            needToPassiveAttack = TRUE;
            //store the target list acordingly
            selSelectionCopy((MaxAnySelection *)&passiveAttackSelection,(MaxAnySelection *)command->attack);
        }
    }
    if(command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        if(command->protect->numShips > 0)
        {
            //group was protecting a ship, set the protection list and
            //set a flag accordingly
            selSelectionCopy((MaxAnySelection *)&protectSelection,(MaxAnySelection *)command->protect);
            needToProtect = TRUE;
        }
    }

    putFightersInSelection(&fighters,(MaxSelection *)command->selection);
    putCorvettesInSelection(&corvettes,(MaxSelection *)command->selection);
    putCapitalsInSelection(&capitals,(MaxSelection *)command->selection);

    // want to make sure all ships go into either fighters, corvettes, or frigates
    dbgAssert(fighters.numShips + corvettes.numShips + capitals.numShips == command->selection->numShips);

    //loop through all types of ships {capitals,fighters and corvetters}
    for(j = 0; j < 3; j++)
    {
        working.numShips = 0;
        if(j == 0)
            curselection = &fighters;
        else if (j==1)
            curselection = &corvettes;
        else
            curselection = &capitals;
        k =0;

        //loop through the current selection and divide type into groups
        //of MAX_UNORDERED_GROUP_SIZE
        for(i=0;i<curselection->numShips;i++)
        {
            working.ShipPtr[k] = curselection->ShipPtr[i];
            k++;
            if(k == MAX_UNORDERED_GROUP_SIZE)
            {
                //maximum # reached for this group
                //set their formation,targets,and protection flags accordingly
                working.numShips = k;
                clFormation(comlayer,(SelectCommand *)&working,DELTA_FORMATION);

                //if group was previously passiveattacking or
                //protecting set them to doing the same thing again with the
                //same target list
                if(needToPassiveAttack)
                {
                    clPassiveAttack(comlayer,(SelectCommand *)&working,(SelectAnyCommand *)&passiveAttackSelection);
                }
                if(needToProtect)
                    clProtect(comlayer,(SelectCommand *)&working,(SelectCommand *)&protectSelection);
                working.numShips = 0;
                k = 0;
            }
        }
        //done looping through groups
        working.numShips = k;
        if(k > 0)
        {
            //set remaining ships to formation + other tasks
            clFormation(comlayer,(SelectCommand *)&working,DELTA_FORMATION);
            //if group was previously passiveattacking or
            //protecting set them to doing the same thing again with the
            //same target list
            if(needToPassiveAttack)
            {
                clPassiveAttack(comlayer,(SelectCommand *)&working,(SelectAnyCommand *)&passiveAttackSelection);
            }
            if(needToProtect)
                clProtect(comlayer,(SelectCommand *)&working,(SelectCommand *)&protectSelection);
            working.numShips = 0;
            k = 0;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : salCapCorvettesInSelectionCantDoFormation
    Description : returns true if salcaps in selection can't dealwith
                  a formation at the present time
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 salCapCorvettesInSelectionCantDoFormation(SelectCommand *selection)
{
    sdword i;

    for(i=0;i<selection->numShips;i++)
    {
        if(ShipTypeIsSalCapCorvetteType(selection->ShipPtr[i]->shiptype))
        {
            if(selection->ShipPtr[i]->specialFlags1C8 & CATA_RUNTIME_BROKEN_FORMATION)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
/*-----------------------------------------------------------------------------
    Name        : processHoldingPattern
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 processHoldingPattern(CommandToDo *command,SelectCommand *selection)
{
    sdword i,count;
    vector destination,heading,right;

    if(selection->numShips == 0)
        return TRUE;

    //flight forward for 2 secs...then turn
    if((universe.totaltimeelapsed - command->holdingStartTime) > TW_HOLDING_PATTERN_PATHTIME)

    {
        //turn right
        command->holdingStartTime = universe.totaltimeelapsed;
        matGetVectFromMatrixCol2(right,selection->ShipPtr[0]->rotinfo.coordsys);
        matGetVectFromMatrixCol3(heading,selection->ShipPtr[0]->rotinfo.coordsys);
        vecScalarMultiply(right,right,TW_HOLDING_PATTERN_RIGHT_COEFFICIENT);
        vecScalarMultiply(heading,heading,TW_HOLDING_PATTERN_HEADING_COEFFICIENT);
        vecAdd(command->holdingPatternVec,right,heading);
        vecScalarMultiply(command->holdingPatternVec,command->holdingPatternVec,TW_HOLDING_PATTERN_PATH_DISTANCE);

        command->turnAroundTimes++;
        if(command->turnAroundTimes > TW_HOLDING_PATTERN_NUM_TURNS)
            return TRUE;
    }
    count = selection->numShips;
    if(command->ordertype.attributes & COMMAND_IS_FORMATION)
        count = 1;  //only process leader if command is in formation!
    for(i=0;i<count;i++)
    {
        vecAdd(destination,command->holdingPatternVec,selection->ShipPtr[i]->holdingPatternPos);
        aishipFlyToPointAvoidingObjs(selection->ShipPtr[i],&destination,AISHIP_PointInDirectionFlying|AISHIP_FastAsPossible,TW_HOLDING_PATTERN_PATH_MAX_SPEED);
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : clProcess
    Description : processes the command layer (executes all commands)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clProcess(CommandLayer *comlayer)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;
    Node *nextnode;
    bool32 passiveAttacked;
    bool32 done;

    Ship *ship;
    Ship *target;
    SpaceObjRotImpTarg *attacktarget;
    sdword i;
    SelectCommand *commandselection;
#ifndef HW_Release
    flightmanTest();
#endif


    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

#ifdef DEBUG_COMMANDLAYER
//        dbgMessagef("\nclProcessing %d %d",command->ordertype.order,command->ordertype.attributes);
#endif

        commandselection = command->selection;
        if(singlePlayerGame)
        {
            for (i=0;i<commandselection->numShips;i++)
            {
                ship = commandselection->ShipPtr[i];
                if (ship->flags & (SOF_Hyperspace|SOF_Crazy))
                {
                    goto nextcommand;       // don't process command if any ships in hyperspace ether or Crazy
                }
                ship->autostabilizeship = FALSE; // don't autostabilize ship if ship has a command to do
            }
        }
        else
        {
            //multiplayer version...allowing hyperspace!
            for (i=0;i<commandselection->numShips;i++)
            {
                ship = commandselection->ShipPtr[i];
                if (ship->flags & (SOF_Crazy))
                {
                    goto nextcommand;       // don't process command if any ships  Crazy
                }
                ship->autostabilizeship = FALSE; // don't autostabilize ship if ship has a command to do
            }
        }
        if (command->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            switch (command->ordertype.order)
            {
                case COMMAND_ATTACK:
                    ship = command->selection->ShipPtr[0];
                    attacktarget = command->attack->TargetPtr[0];
#ifdef DEBUG_TACTICS
                    if(tacticsOn)
#endif
                    {
                        if(!tacticsInfo.holdFormationDuringBattle[ship->tacticstype] || command->selection->numShips == 1)
                        {   //evasive tactics, so attack as per olden times
                            if ((command->formation.doneInitialAttack) || (command->attack->numTargets == 0) || (TargetWithinAttackRange(ship,attacktarget)))
                            {
                                command->formation.doneInitialAttack = TRUE;
                                if (processAttackToDo(command))
                                {
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING &&
                                        command->selection->ShipPtr[0]->aistatecommand == 0)
                                    {
                                        clResumeAttackMoveAsMove(command);
                                    }
                                    else
                                    {
                                        // free COMMAND_ATTACK stuff
                                        FreeLastOrder(command);
                                        // attack has finished so convert it back to a formation
                                        command->ordertype.order = COMMAND_NULL;
                                    }
                                }
                                else
                                {
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                                    {
                                        if((universe.univUpdateCounter & CHECK_ATTACKMOVE_TO_MOVE_RATE)==CHECK_ATTACKMOVE_TO_MOVE_FRAME)
                                        if(getRangeToClosestTarget(command) >= tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr)
                                        {
                                            if(command->selection->ShipPtr[0]->aistatecommand == 0)
                                            {
                                                //it is at this distance that the group of ships turns and hitails it out of there...
                                                clResumeAttackMoveAsMove(command);
                                                break;
                                            }
                                            else
                                            {
                                                ChangeOrderToHalt(command);
                                            }
                                        }
                                    }

                                }
                            }
                            else
                            {
                                aishipFlyToShipAvoidingObjs(ship,attacktarget,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,
                                                            command->formation.travelvel);
                                ship->shipidle = FALSE;
                                processFormationToDo(command,FALSE,FALSE);
                            }
                        }
                        else
                        {  // neutral or aggressive leader tactics,
                            //formation type is held during combat, be it loose or tight
                            //command->formation.doneInitialAttack = TRUE;    // we attack in formation anyway, so let's just attack
                            if ((command->formation.doneInitialAttack) || (command->attack->numTargets == 0) || (TargetWithinAttackRange(ship,attacktarget)))
                            {
                                command->formation.doneInitialAttack = TRUE;
                                if (processAttackToDoInFormation(command))
                                {
                                    // free COMMAND_ATTACK stuff
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING &&
                                        command->selection->ShipPtr[0]->aistatecommand == 0)
                                    {
                                        //attacking and moving and HAVEN'T reached destination!
                                        clResumeAttackMoveAsMove(command);
                                    }
                                    else
                                    {
                                        // free COMMAND_ATTACK stuff
                                        FreeLastOrder(command);
                                        // attack has finished so convert it back to a formation
                                        command->ordertype.order = COMMAND_NULL;
                                    }                                     //command->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
                                }
                                else
                                {
                                    if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                                    {
                                        if((universe.univUpdateCounter & CHECK_ATTACKMOVE_TO_MOVE_RATE)==CHECK_ATTACKMOVE_TO_MOVE_FRAME)
                                        if(getRangeToClosestTarget(command) >= tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr)
                                        {
                                            if(command->selection->ShipPtr[0]->aistatecommand == 0)
                                            {
                                                //it is at this distance that the group of ships turns and hitails it out of there...
                                                clResumeAttackMoveAsMove(command);
                                                break;
                                            }
                                            else
                                            {
                                                ChangeOrderToHalt(command);
                                            }
                                        }
                                    }

                                }
                            }
                            else
                            {
                                aishipFlyToShipAvoidingObjs(ship,attacktarget,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,
                                                            command->formation.travelvel);
                                ship->shipidle = FALSE;
                                processFormationToDo(command,FALSE,FALSE);
                            }
                        }
                    }
#ifdef DEBUG_TACTICS
                    else
                    {
                        if ((command->formation.doneInitialAttack) || (command->attack->numTargets == 0) || (TargetWithinAttackRange(ship,attacktarget)))
                        {
                            command->formation.doneInitialAttack = TRUE;
                            if (processAttackToDo(command))
                            {
                                // free COMMAND_ATTACK stuff
                                FreeLastOrder(command);
                                // attack has finished so convert it back to a formation
                                command->ordertype.order = COMMAND_NULL;
                            }
                        }
                        else
                        {
                            aishipFlyToShipAvoidingObjs(ship,attacktarget,AISHIP_PointInDirectionFlying + AISHIP_CarTurn,
                                                    command->formation.travelvel);
                            ship->shipidle = FALSE;
                            processFormationToDo(command,FALSE,FALSE);
                        }
                    }
#endif
                    break;
                case COMMAND_DOCK:
                    DockType dockMode = dockCommandGetMode(command);
                    if (dockMode == DOCK_MODE_INSTANT_REFUELREPAIR ||
                        dockMode == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH ||
                        dockMode == DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP)
                    {
                        goto processdock;
                    }
                    if (dockCommandGetAllDockingWithSameShip(command) && !dockCommandGetAllNearShip(command))
                    {
                        ship = command->selection->ShipPtr[0];
                        target = ship->dockvars.dockship;

                        if ((target == NULL) || ShipWithinDockRange(ship,target))
                        {
                            dockCommandSetAllNearShip(command, TRUE);
                            goto processdock;
                        }

                        if (aishipFlyToShipAvoidingObjs(ship,target,AISHIP_PointInDirectionFlying + AISHIP_CarTurn + AISHIP_DontFlyToObscuredPoints + AISHIP_ReturnImmedIfPointObscured,
                                command->formation.travelvel) & AISHIP_FLY_OBJECT_IN_WAY)
                        {
                            dockCommandSetAllNearShip(command, TRUE);
                            goto processdock;
                        }

                        ship->shipidle = FALSE;
                        processFormationToDo(command,FALSE,FALSE);
                    }
                    else
                    {
processdock:
                        if (processDockToDo(command))
                        {
                            if (command->selection->numShips == 0)
                            {
                                // All ships have been removed from docking.  Therefore totally wipe out command
                                nextnode = curnode->next;         // dock command has finished
                                FreeCommandToDoContents(command);
                                listDeleteNode(curnode);
                                curnode = nextnode;
                                goto readyfornextnode;
                            }
                            else
                            {
                                FreeLastOrder(command);  // free COMMAND_DOCK
                                command->ordertype.order = COMMAND_NULL;
                            }
                        }
                    }
                    break;

                case COMMAND_HALT:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        passiveAttacked = TRUE;
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                    }

                    ship = command->selection->ShipPtr[0];
                    if ((passiveAttacked) & (bool32)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
                    {
                        done = aitrackSteadyShipDriftOnly(ship);
                    }
                    else
                    {
                        done = aitrackSteadyShip(ship);
                    }
                    processFormationToDo(command,TRUE,passiveAttacked);

                    if (done)
                    {
                        FreeLastOrder(command);
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_NULL:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            passiveAttacked = TRUE;
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        }

//                        if (command->formation.formAroundProtectedShip)
                        if ((command->ordertype.attributes & COMMAND_IS_FORMATION) &&
                            (command->selection->numShips > 1) &&
                            (command->formation.formationtype == SPHERE_FORMATION ||
                             command->formation.formationtype == FORCEFIELD_WALL_FORMATION ||
                             command->formation.formationtype == FORCEFIELD_SPHERE_FORMATION))
                        {                                                           // sphere/forcefield protecting formations fly through the formation runtime
                            processFormationToDo(command,FALSE,passiveAttacked);
                        }
                        else
                        {
                            if (command->protect->numShips > 1)
                            {
                                protectShipsAvg(command->selection->ShipPtr[0],command->protect,passiveAttacked);
                                processFormationToDo(command,FALSE,passiveAttacked);
                            }
                            else
                            {
                                protectShip(command->selection->ShipPtr[0],command->protect->ShipPtr[0],passiveAttacked);
                                processFormationToDo(command,!command->protect->ShipPtr[0]->posinfo.isMoving,passiveAttacked);
                            }
                        }
                    }
                    else
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            passiveAttacked = TRUE;
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        }

                        if(command->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN)
                        {
                            if(processHoldingPattern(command,command->selection))
                            {
                                bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                            }
                        }
                        else
                        {
                            ship = command->selection->ShipPtr[0];
                            if ((passiveAttacked) & (bool32)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
                            {
                                //if(ship->tacticstype != Evasive)
                                    aitrackSteadyShipDriftOnly(ship);
                            }
                            else
                            {
                                aitrackSteadyShip(ship);
                            }
                        }
                        if(passiveAttacked)// && command->selection->ShipPtr[0]->tacticstype == Evasive)
                            processFormationToDo(command,FALSE,passiveAttacked);
                        else
                            processFormationToDo(command,bitTest(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN) ? FALSE : TRUE,passiveAttacked);
                    }
                    break;
                case COMMAND_MP_HYPERSPACEING:
                    if(processMpHyperspaceingToDo(command))
                    {
                        FreeLastOrder(command); // free COMMAND_MP_HYPERSPACEING stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_LINK:
                    if (processLinkToDo(command))
                    {
                        FreeLastOrder(command); // free COMMAND_LINK stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_LINKED_LAUNCH:
                    if (processLinkedLaunchToDo(command))
                    {
                        FreeLastOrder(command); // free COMMAND_LINKED_LAUNCH stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_MOVE:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        //don't set to true as it controls whether or not a ship
                        //faces in movement direction
                        //passiveAttacked = TRUE;
                    }

                    passiveAttacked = TRUE;
                    for (i = 0; i < command->selection->numShips; i++)
                    {
                        passiveAttacked &= processAttackMoveToDo(command,FALSE,i,FALSE);
                    }

                    if (passiveAttacked)
                    {
                        // reached destination, so change COMMAND_MOVE | COMMAND_IS_FORMATION to COMMAND_NULL | COMMAND_IS_FORMATION
#ifdef DEBUG_FORMATIONS
                        dbgMessage("\nReached Destination in Formation.");
#endif
                        if (universe.curPlayerPtr->PlayerMothership != NULL)
                        {
                            // Mothership can be carrier, so make sure it is really a mothership
                            if (ShipTypeIsMothershipType(universe.curPlayerPtr->PlayerMothership->shiptype) && (selShipInSelection(command->selection->ShipPtr, command->selection->numShips, universe.curPlayerPtr->PlayerMothership)))
                            {
                                /* Cataclysm queues fleet-intel event 0x400101A here, not HW1's COMM_F_MoShip_Arrived. */
                                speechEventQueue(
                                    NULL,
                                    67112986,
                                    -1,
                                    -1,
                                    0,
                                    command->selection->ShipPtr[0]->playerowner->playerIndex,
                                    -1,
                                    5.0f,
                                    -1);
                            }
                        }

                        if (command->ordertype.attributes & (COMMAND_IS_PROTECTING|COMMAND_IS_PASSIVEATTACKING))
                        {
                            clClearMoveCompletionChatter(command->selection->ShipPtr[0]);
                            FreeLastOrder(command); // free COMMAND_MOVE stuff
                            command->ordertype.order = COMMAND_NULL;
                        }
                        else
                        {
                            clClearMoveCompletionChatterSelection(command->selection);
                            nextnode = curnode->next;
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    break;

                case COMMAND_SPECIAL:
                    if (processSpecialToDo(command))
                    {
                        FreeLastOrder(command);  // free COMMAND_SPECIAL
                        command->ordertype.order = COMMAND_NULL;
                    }
                    if(salCapCorvettesInSelectionCantDoFormation(command->selection))
                    {
                        processFormationToDo(command,FALSE,FALSE);
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }
        }
        else
        {
            switch (command->ordertype.order)
            {
                case COMMAND_ATTACK:
                    //open guns if in range or if we need to
                    ship = command->selection->ShipPtr[0];
                    if (command->selection->numShips > 1 &&
                        tacticsInfo.holdFormationDuringBattle[ship->tacticstype])
                    {
                        done = processAttackToDoInFormation(command);
                    }
                    else
                    {
                        done = processAttackToDo(command);
                    }
                    if (done)
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                        {
                            if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                            {
                                clResumeAttackMoveAsMove(command);
                            }
                            else
                            {
                                // free COMMAND_ATTACK stuff
                                FreeLastOrder(command);
                                // attack has finished so convert it back to a formation
                                command->ordertype.order = COMMAND_NULL;
                            }
                        }
                        else
                        {
                            /*
                            nextnode = curnode->next;         // attack has finished
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                            */
                            if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                            {
                                clResumeAttackMoveAsMove(command);
                            }
                            else
                            {
                                // free COMMAND_ATTACK stuff
                                FreeLastOrder(command);
                                // attack has finished so convert it back to a formation
                                command->ordertype.order = COMMAND_NULL;
                            }
                            //command->ordertype.attributes |= COMMAND_IS_HOLDINGPATTERN;
                        }
                    }
                    else
                    {
                        if(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING)
                        {
                            if((universe.univUpdateCounter & CHECK_ATTACKMOVE_TO_MOVE_RATE)==CHECK_ATTACKMOVE_TO_MOVE_FRAME)
                            if(getRangeToClosestTarget(command) >= tacticsInfo.movingAttackTurnsIntoMoveCommandDistanceSqr)
                            {
                                //it is at this distance that the group of ships turns and hitails it out of there...
                                clResumeAttackMoveAsMove(command);
                                break;
                            }
                        }

                    }
                    break;
                case COMMAND_MP_HYPERSPACEING:
                    if(processMpHyperspaceingToDo(command))
                    {
                        FreeLastOrder(command); // free COMMAND_MOVE stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_LINK:
                    if (processLinkToDo(command))
                    {
                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_LINKED_LAUNCH:
                    if (processLinkedLaunchToDo(command))
                    {
                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_MOVE:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        //don't set to true so we face direction we're
                        //moving
                        //passiveAttacked = TRUE;
                    }

                    passiveAttacked = TRUE;
                    for (i = 0; i < command->selection->numShips; i++)
                    {
                        passiveAttacked &= processAttackMoveToDo(command,FALSE,i,FALSE);
                    }

                    if (passiveAttacked)
                    {
                        if (universe.curPlayerPtr->PlayerMothership != NULL)
                        {
                            // Mothership can be carrier, so make sure it is really a mothership
                            if (ShipTypeIsMothershipType(universe.curPlayerPtr->PlayerMothership->shiptype) && (selShipInSelection(command->selection->ShipPtr, command->selection->numShips, universe.curPlayerPtr->PlayerMothership)))
                            {
                                /* Cataclysm queues fleet-intel event 0x400101A here, not HW1's COMM_F_MoShip_Arrived. */
                                speechEventQueue(
                                    NULL,
                                    67112986,
                                    -1,
                                    -1,
                                    0,
                                    command->selection->ShipPtr[0]->playerowner->playerIndex,
                                    -1,
                                    5.0f,
                                    -1);
                            }
                        }

                        clClearMoveCompletionChatter(command->selection->ShipPtr[0]);
                        FreeLastOrder(command);   // free COMMAND_MOVE stuff
                        command->ordertype.order = COMMAND_NULL;
                    }
                    break;

                case COMMAND_DOCK:
                    if (processDockToDo(command))
                    {
                        if (dockCommandGetWasHarvesting(command))
                        {
                            FreeLastOrder(command);  // free COMMAND_DOCK
                            ChangeSingleShipToCollectResource(command, NULL);
                        }
                        else
                        {
                            nextnode = curnode->next;         // dock command has finished
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    break;

                case COMMAND_SPECIAL:
                    if (processSpecialToDo(command))
                    {
                        nextnode = curnode->next;         // special command has finished
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_COLLECTRESOURCE:
                    if (processCollectResource(command))
                    {
                        Ship *ship = command->selection->ShipPtr[0];
                        dbgAssert(command->selection->numShips >= 1);

                        nextnode = curnode->next;         // collect resource command has finished
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;

                        if ((ship->playerowner) && (ship->playerowner->PlayerMothership))
                        {
                            GroupShipIntoMilitaryParade(&universe.mainCommandLayer,ship,ship->playerowner->PlayerMothership);
                        }

                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_LAUNCHSHIP:
                    if (processLaunchShipToDo(command))
                    {
                        Ship *ship;
                        Ship *receiverShip;

                        dbgAssert(command->selection->numShips == 1);

                        ship = command->selection->ShipPtr[0];
                        receiverShip = command->launchship.receiverShip;

                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;

                        GroupShip(comlayer,ship,receiverShip);

                        aiplayerShipLaunchedCallback(ship);

                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_HALT:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        passiveAttacked = TRUE;
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                    }

                    if (processHaltToDo(command,passiveAttacked))
                    {
                        if (command->ordertype.attributes)
                        {
                            FreeLastOrder(command);
                            command->ordertype.order = COMMAND_NULL;
                        }
                        else
                        {
                            nextnode = curnode->next;
                            FreeCommandToDoContents(command);
                            listDeleteNode(curnode);
                            curnode = nextnode;
                            goto readyfornextnode;
                        }
                    }
                    break;

                case COMMAND_MILITARYPARADE:
                    passiveAttacked = FALSE;
                    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                    {
                        if (processPassiveAttackToDo(command))
                        {
                            ClearPassiveAttacking(command);
                        }
                        passiveAttacked = TRUE;
                        bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                    }
                    processMilitaryParadeToDo(command,passiveAttacked);
                    break;

                case COMMAND_SIEGECANNON:
                    nextnode = curnode->next;
                    mothershipProcessBigGunCommand(command);
                    FreeCommandToDoContents(command);
                    listDeleteNode(curnode);
                    curnode = nextnode;
                    goto readyfornextnode;

                case COMMAND_EXTERNALCONSTRUCTION:
                    nextnode = curnode->next;
                    for (i = 0; i < command->selection->numShips; i++)
                    {
                        shipProcessModuleSlotOrder(
                            command->selection->ShipPtr[i],
                            *clExternalConstructionOperation(command),
                            *clExternalConstructionShipType(command),
                            *clExternalConstructionParam2(command),
                            *clExternalConstructionParam3(command));
                    }
                    FreeCommandToDoContents(command);
                    listDeleteNode(curnode);
                    curnode = nextnode;
                    goto readyfornextnode;

                case COMMAND_BUILDINGSHIP:
                    dbgAssert(command->ordertype.attributes == 0);
                    // don't allow COMMAND_IS_PASSIVEATTACKING, COMMAND_IS_PROTECTING, COMMAND_IS_FORMATION
                    if (processBuildingShipToDo(command))
                    {
                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    break;

                case COMMAND_RESEARCH:
                    clApplyResearchStatusChange(command);
                    nextnode = curnode->next;
                    FreeCommandToDoContents(command);
                    listDeleteNode(curnode);
                    curnode = nextnode;
                    goto readyfornextnode;

                case COMMAND_NULL:
                    if (command->ordertype.attributes == 0)
                    {
                        // NULL command, and no attributes, so let's remove it from the command layer
                        nextnode = curnode->next;
                        FreeCommandToDoContents(command);
                        listDeleteNode(curnode);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }
                    if(command->ordertype.attributes & COMMAND_IS_HOLDINGPATTERN)
                    {
                        //put ships in holding pattern...maintain all relevant info though (protecting and passiveattacking!)
                        nextnode = curnode->next;

                        //this function will result in command being turned into a formation flying a holding pattern
                        clHoldingPattern(comlayer,command);
                        curnode = nextnode;
                        goto readyfornextnode;
                    }


                    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
                    {
                        ProtectCommand *commandprotect = command->protect;
                        SelectCommand *commandselect = command->selection;
                        sdword numShipsToProtect = commandprotect->numShips;
                        sdword numShips = commandselect->numShips;
                        Ship *protectShipPtr;
                        sdword protectThisShip;
                        sdword i;

                        passiveAttacked = FALSE;
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            passiveAttacked = TRUE;
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);
                        }

                        dbgAssert(numShipsToProtect > 0);
                        if (numShipsToProtect == 1)
                        {
                            protectShipPtr = commandprotect->ShipPtr[0];
                            for (i=0;i<numShips;i++)
                            {
                                protectShip(commandselect->ShipPtr[i],protectShipPtr,passiveAttacked);
                            }
                        }
                        else
                        {
                            if (numShips == 1)
                            {
                                // we only have 1 ship, and > 1 ships to protect, so try to stay in middle
                                protectShipsAvg(commandselect->ShipPtr[0],commandprotect,passiveAttacked);
                            }
                            else
                            {
                                protectThisShip = 0;

                                for (i=0;i<numShips;i++)
                                {
                                    protectShip(commandselect->ShipPtr[i],commandprotect->ShipPtr[protectThisShip],passiveAttacked);

                                    protectThisShip++;
                                    if (protectThisShip >= numShipsToProtect)
                                    {
                                        protectThisShip = 0;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                        {
                            if (processPassiveAttackToDo(command))
                            {
                                ClearPassiveAttacking(command);
                            }
                            bitClear(command->ordertype.attributes,COMMAND_IS_HOLDINGPATTERN);

                            ship = command->selection->ShipPtr[0];
                            if ((bool32)((ShipStaticInfo *)ship->staticinfo)->rotateToRetaliate)
                            {
                                if(ship->tacticstype != Evasive)
                                    aitrackSteadyShipDriftOnly(ship);
                            }
                            else
                            {
                                aitrackSteadyShip(ship);
                            }
                        }
                    }
                    break;

                default:
                    dbgAssert(FALSE);
                    break;
            }

        }
nextcommand:
        curnode = curnode->next;
readyfornextnode:
        ;
    }
}

static void clRemoveNonCombatShipsFromSelection(SelectCommand *selection)
{
    sdword i;
    Ship *ship;

    for (i = 0; i < selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        if (ship->shiptype == kCloakGenerator ||
            ship->shiptype == kGravWellGenerator ||
            ship->shiptype == tCloakGenerator ||
            ship->shiptype == tGravWellGenerator ||
            ship->shiptype == bDFGFrigate ||
            ship->shiptype == tDFGFrigate)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : clPostProcess
    Description : processes all of the command layer (2nd pass), for actions such as protecting
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clPostProcess(CommandLayer *comlayer)
{
    Node *curnode = comlayer->todolist.head;
    Node *endnode = comlayer->todolist.tail;
    CommandToDo *command;
    Node *nextnode;
    AttackCommand *attack;
    ShipPtr protectMe;
    sdword i;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        nextnode = curnode->next;

        if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            dbgAssert(command->protect->numShips > 0);

            if (bitTest(command->protectFlags,PROTECTFLAGS_JUST_FOLLOW))
            {
                goto dontretaliate;
            }

            for (i=0;i<command->protect->numShips;i++)
            {
                protectMe = command->protect->ShipPtr[i];

                if ((protectMe->gettingrocked != NULL) &&
                    (!univIsObjFriendly((SpaceObj *)command->selection->ShipPtr[0],protectMe->gettingrocked->playerowner)))
                {
                    if ((command->ordertype.order == COMMAND_NULL) || (command->ordertype.order == COMMAND_MOVE) || (command->ordertype.order == COMMAND_ATTACK))
                    {
                        if ((command->ordertype.attributes & COMMAND_IS_FORMATION) &&
                            (command->formation.formationtype == SPHERE_FORMATION ||
                             command->formation.formationtype == FORCEFIELD_SPHERE_FORMATION))
                        {
                            if (i == 0)     // 0th ship is always protected by a surrounded sphere of ships, and these ships
                            {               // should stay nearby (not attack, just passive attack).
                                goto dontretaliate;
                            }
                        }

                       /*
                       if (AreAllShipsAttackCapable(command->selection))
                        {
                            attack = (AttackCommand *)getShipAndItsFormation(&universe.mainCommandLayer,protectMe->gettingrocked);
                            //filter gravwell and other related ships for attack during guarding
                            makeShipsNotHaveNonCombatShipsForGuardAttack(command->selection);
                            //ChangeOrderToAttack(command,attack);
                            clAttack(&universe.mainCommandLayer,command->selection,attack);
                            memFree(attack);
                        }
                        else
                        */
                          {

//                            if (AreAnyShipsAttackCapable(command->selection))
                            MaxSelection attackingships;
                            if (MakeShipsAttackCapable((SelectCommand *)&attackingships, command->selection))
                            {
                                // just tell the ships that are attack capable to attack
                                CommandToDo *newcommand;
                                attack = (AttackCommand *)getShipAndItsFormation(&universe.mainCommandLayer,protectMe->gettingrocked);
                                dbgAssert(attackingships.numShips > 0);
                                clRemoveNonCombatShipsFromSelection((SelectCommand *)&attackingships);
                                if(attackingships.numShips > 0)
                                {
                                newcommand = clProtect(comlayer,(SelectCommand *)&attackingships,command->protect);
                                //filter gravwell and other related ships for attack during guarding
                                ChangeOrderToAttack(newcommand,attack);
                                }
                                memFree(attack);
//                                memFree(attackingships);
                            }
                        }
                        goto dontretaliate;
                    }
                }

            }
        }
dontretaliate:

        if (curnode == endnode)
        {
            break;
        }
        curnode = nextnode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsCommand
    Description : Returns allocated command containing the ship
    Inputs      :
    Outputs     :
    Return      : Returns allocated command containing the ship
----------------------------------------------------------------------------*/
CommandToDo *getShipAndItsCommand(CommandLayer *comlayer,ShipPtr ship)
{
#if 0
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (selShipInSelection(command->selection->ShipPtr,command->selection->numShips,ship))
        {
            dbgAssert(ship->command == command);
            return command;
        }

        curnode = curnode->next;
    }

    dbgAssert(ship->command == NULL);
    return NULL;
#else
    return ship->command;
#endif
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsCommandSelection
    Description : Returns selection for allocated command containing the ship.
                  If there's no command for the ship, it'll return the formation
                  of the ship.
    Inputs      :
    Outputs     :
    Return      : Returns allocated command containing the ship
----------------------------------------------------------------------------*/
SelectCommand *getShipAndItsCommandSelection(CommandLayer *comlayer,ShipPtr ship,bool32 *parade)
{
    CommandToDo *command = getShipAndItsCommand(comlayer,ship);
    SelectCommand *returnselect;
    udword sizeofselect;

    if (command != NULL)
    {
        if (command->ordertype.order == COMMAND_MILITARYPARADE)
        {
            *parade = TRUE;
        }
        else
        {
            *parade = FALSE;
        }
        sizeofselect = sizeofSelectCommand(command->selection->numShips);
        returnselect = memAlloc(sizeofselect,"commandselection",Pyrophoric);
        memcpy(returnselect,command->selection,sizeofselect);
    }
    else
    {
        //no command found containing this ship; return just the single ship
        returnselect = memAlloc(sizeofSelectCommand(1),"sf(shipandform)", Pyrophoric);
        returnselect->numShips = 1;
        returnselect->ShipPtr[0] = ship;
        *parade = FALSE;
    }
    return returnselect;
}

/*-----------------------------------------------------------------------------
    Name        : GetMilitaryGroupAroundShip
    Description : gets the
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
CommandToDo *GetMilitaryGroupAroundShip(CommandLayer *comlayer,Ship *aroundShip)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (command->ordertype.order == COMMAND_MILITARYPARADE)
        {
            if (command->militaryParade->aroundShip == aroundShip)
            {
                return command;
            }
        }

        curnode = curnode->next;
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : clPresetShipsToPosition
    Description : sets ships to correct position according to command layer
    Inputs      : comlayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clPresetShipsToPosition(CommandLayer *comlayer)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if (command->ordertype.order == COMMAND_MILITARYPARADE)
        {
            setMilitaryParade(command);
        }
        else if (command->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            setFormationToDo(command);
        }

        curnode = curnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsFormationCommand
    Description : Returns allocated command containing the ship and its formation
    Inputs      :
    Outputs     :
    Return      : Returns allocated command containing the ship and its formation
----------------------------------------------------------------------------*/
CommandToDo *getShipAndItsFormationCommand(CommandLayer *comlayer,ShipPtr ship)
{
    return ship->formationcommand;
}

/*-----------------------------------------------------------------------------
    Name        : getShipAndItsFormation
    Description : Returns allocated selectcommand containing the ship and its formation
    Inputs      :
    Outputs     :
    Return      : Returns allocated selectcommand containing the ship and its formation
----------------------------------------------------------------------------*/
SelectCommand *getShipAndItsFormation(CommandLayer *comlayer,ShipPtr ship)
{
    CommandToDo *command = getShipAndItsCommand(comlayer,ship);
    SelectCommand *returnselect;
    udword sizeofselect;

    if (command != NULL && (command->ordertype.attributes & COMMAND_IS_FORMATION))
    {
        sizeofselect = sizeofSelectCommand(command->selection->numShips);
        returnselect = memAlloc(sizeofselect,"sf(shipandform)",Pyrophoric);
        memcpy(returnselect,command->selection,sizeofselect);
    }
    else
    {
        returnselect = memAlloc(sizeofSelectCommand(1),"sf(shipandform)", Pyrophoric);
        returnselect->numShips = 1;
        returnselect->ShipPtr[0] = ship;
    }

    return returnselect;
}

/*-----------------------------------------------------------------------------
    Name        : CommandInCommandLayer
    Description : returns TRUE if searchfor is in comlayer
    Inputs      : comlayer, searchfor
    Outputs     :
    Return      : returns TRUE if searchfor is in comlayer
----------------------------------------------------------------------------*/
bool32 CommandInCommandLayer(CommandLayer *comlayer,CommandToDo *searchfor)
{
    Node *curnode = comlayer->todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        if (command == searchfor)
        {
            return TRUE;
        }

        curnode = curnode->next;
    }

    return FALSE;
}

static const real32 cataLinkArrivalEpsilon = 5.0f;
static const real32 cataLinkAnimationDuration = 2.0f;
static const real32 cataLinkTrackAccuracy = 0.99900001f;
static const real32 cataLinkAlignmentThreshold = 0.996f;

static bool32 clIsSectOrBeastAcolyte(const Ship *ship)
{
    return ship->shiptype == sAcolyte || ship->shiptype == bAcolyte;
}

static bool32 clLinkMatricesAlignedWithinThreshold(const matrix *source, const matrix *target, real32 threshold)
{
    return source->m11 * target->m11 + source->m21 * target->m21 + source->m31 * target->m31 >= threshold &&
           source->m13 * target->m13 + source->m23 * target->m23 + source->m33 * target->m33 >= threshold &&
           source->m12 * target->m12 + source->m22 * target->m22 + source->m32 * target->m32 >= threshold;
}

static Ship *clGetLinkPartner(const Ship *ship)
{
    switch (ship->shiptype)
    {
        case sAcolyte:
        case bAcolyte:
            return acolyteShipSpecificsConst(ship)->linkPartner;

        case sMimic:
            return mimicShipSpecificsConst(ship)->linkPartner;

        default:
            return NULL;
    }
}

static void clSetLinkPartner(Ship *ship, Ship *partner)
{
    switch (ship->shiptype)
    {
        case sAcolyte:
        case bAcolyte:
            acolyteShipSpecifics(ship)->linkPartner = partner;
            break;

        case sMimic:
            mimicShipSpecifics(ship)->linkPartner = partner;
            break;

        default:
            break;
    }
}

static sdword *clLinkStatePtr(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sAcolyte:
        case bAcolyte:
            return &acolyteShipSpecifics(ship)->linkState;

        case sMimic:
            return &mimicShipSpecifics(ship)->linkState;

        default:
            return NULL;
    }
}

static sdword *clLinkAnimationIndexPtr(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sAcolyte:
        case bAcolyte:
            return &acolyteShipSpecifics(ship)->linkAnimationIndex;

        case sMimic:
            return &mimicShipSpecifics(ship)->linkAnimationIndex;

        default:
            return NULL;
    }
}

static void clCopyLinkRuntimeState(Ship *linkedShip, const Ship *ship, const Ship *partner)
{
    ACVSpecifics *linkedSpecifics;
    sdword i;
    const ShipDamageLevelCounters *shipRuntime;
    const ShipDamageLevelCounters *partnerRuntime;

    linkedSpecifics = acvShipSpecifics(linkedShip);
    shipRuntime = &ship->damageLevelCounters;
    partnerRuntime = &partner->damageLevelCounters;
    for (i = 0; i < (sdword)(sizeof(linkedShip->damageLevelCounters.words) / sizeof(linkedShip->damageLevelCounters.words[0])); i++)
    {
        udword shipValue = shipRuntime->words[i];
        udword partnerValue = partnerRuntime->words[i];

        linkedShip->damageLevelCounters.words[i] = shipValue + partnerValue;
        linkedSpecifics->firstSourceDamageLevelCounters.words[i] = shipValue;
        linkedSpecifics->secondSourceDamageLevelCounters.words[i] = partnerValue;
    }
}

static void clStoreLinkedAcolyteState(Ship *linkedShip, Ship *ship, Ship *partner)
{
    ACVSpecifics *linkedSpecifics;

    linkedSpecifics = acvShipSpecifics(linkedShip);
    linkedSpecifics->firstSourceLinkEnabled = acolyteCanLink(ship);
    linkedSpecifics->secondSourceLinkEnabled = acolyteCanLink(partner);
    linkedSpecifics->firstSourceLinkState = acolyteGetLinkState(ship);
    linkedSpecifics->secondSourceLinkState = acolyteGetLinkState(partner);
    if (universe.totaltimeelapsed > cataLinkAnimationDuration)
    {
        linkedSpecifics->linkedAcolyteState = 2;
        linkedSpecifics->linkedAcolyteStateStartTime = universe.totaltimeelapsed - cataLinkAnimationDuration;
    }
}

static bool32 processLinkToDo(CommandToDo *linktodo)
{
    SelectCommand *selection = linktodo->selection;
    bool32 done = TRUE;
    sdword numShips = selection->numShips;
    sdword i;
    for (i = 0; i < numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];
        sdword *linkState = clLinkStatePtr(ship);
        sdword *linkAnimationIndex = clLinkAnimationIndexPtr(ship);
        Ship *partner = clGetLinkPartner(ship);

        if (partner == NULL || linkState == NULL || linkAnimationIndex == NULL)
        {
            continue;
        }

        switch (*linkState)
        {
            case 2:
            {
                vector destination;
                vector desiredUp;
                real32 separation = ShipStaticInfoGetLinkDistanceValue(ship->staticinfo) * 2.0f;
                udword flyResult;

                desiredUp.x = -partner->rotinfo.coordsys.m11;
                desiredUp.y = -partner->rotinfo.coordsys.m21;
                desiredUp.z = -partner->rotinfo.coordsys.m31;
                destination.x = partner->posinfo.position.x + separation * desiredUp.x;
                destination.y = partner->posinfo.position.y + separation * desiredUp.y;
                destination.z = partner->posinfo.position.z + separation * desiredUp.z;

                flyResult = aishipFlyToPointAvoidingObjsFunc(ship, &destination, 0xB3u, -80.0f, &partner->posinfo.velocity);
                if (fabs(ship->posinfo.position.x - destination.x) < cataLinkArrivalEpsilon &&
                    fabs(ship->posinfo.position.y - destination.y) < cataLinkArrivalEpsilon &&
                    fabs(ship->posinfo.position.z - destination.z) < cataLinkArrivalEpsilon)
                {
                    posSetShipPosition(ship, &destination);
                    ship->posinfo.velocity = partner->posinfo.velocity;
                    *linkState = 3;
                }
                done = FALSE;
                break;
            }

            case 3:
            {
                vector desiredUp;
                vector desiredHeading;
                vector midpoint;
                vector newPosition;
                matrix targetMatrix;
                real32 separation = ShipStaticInfoGetLinkDistanceValue(ship->staticinfo);
                bool32 matrixAligned;
                sdword *partnerAnimationIndex;

                desiredUp.x = -partner->rotinfo.coordsys.m11;
                desiredUp.y = -partner->rotinfo.coordsys.m21;
                desiredUp.z = -partner->rotinfo.coordsys.m31;
                desiredHeading.x = partner->rotinfo.coordsys.m13;
                desiredHeading.y = partner->rotinfo.coordsys.m23;
                desiredHeading.z = partner->rotinfo.coordsys.m33;

                targetMatrix.m11 = desiredUp.x;
                targetMatrix.m21 = desiredUp.y;
                targetMatrix.m31 = desiredUp.z;
                targetMatrix.m12 = -partner->rotinfo.coordsys.m12;
                targetMatrix.m22 = -partner->rotinfo.coordsys.m22;
                targetMatrix.m32 = -partner->rotinfo.coordsys.m32;
                targetMatrix.m13 = desiredHeading.x;
                targetMatrix.m23 = desiredHeading.y;
                targetMatrix.m33 = desiredHeading.z;

                midpoint.x = partner->posinfo.position.x + separation * desiredUp.x;
                midpoint.y = partner->posinfo.position.y + separation * desiredUp.y;
                midpoint.z = partner->posinfo.position.z + separation * desiredUp.z;
                newPosition.x = midpoint.x + separation * desiredUp.x;
                newPosition.y = midpoint.y + separation * desiredUp.y;
                newPosition.z = midpoint.z + separation * desiredUp.z;

                aitrackHeadingAndUp(ship, &desiredHeading, &desiredUp, cataLinkTrackAccuracy);
                posSetShipPosition(ship, &newPosition);
                ship->posinfo.velocity = partner->posinfo.velocity;
                matrixAligned = clLinkMatricesAlignedWithinThreshold(&ship->rotinfo.coordsys, &targetMatrix, cataLinkAlignmentThreshold);
                if (matrixAligned)
                {
                    partnerAnimationIndex = clLinkAnimationIndexPtr(partner);
                    dbgAssert(partnerAnimationIndex != NULL);
                    posSetShipPosition(ship, &midpoint);
                    posSetShipPosition(partner, &midpoint);
                    ship->rotinfo.coordsys = targetMatrix;
                    ship->posinfo.velocity = partner->posinfo.velocity;
                    madAnimationStart(ship, *linkAnimationIndex);
                    madAnimationStart(partner, *partnerAnimationIndex);
                    ship->flags |= SOF_AvoidanceHidden;
                    partner->flags |= SOF_AvoidanceHidden;
                    TimerInitTimerStruct(&ship->linkTransformTimer, cataLinkAnimationDuration);
                    TimerInitTimerStruct(&partner->linkTransformTimer, cataLinkAnimationDuration);
                    *linkState = 4;
                    clSetAttribute((sdword)ship, 1, 3);
                    clSetAttribute((sdword)partner, 1, 3);
                }
                done = FALSE;
                break;
            }

            case 4:
            {
                ShipType linkedShipType;
                Ship *linkedShip;

                if ((ship->madBindings->bPaused && partner->madBindings->bPaused) ||
                    TimerExpired((int)&ship->linkTransformTimer) ||
                    TimerExpired((int)&partner->linkTransformTimer))
                {
                    switch (ship->shiptype)
                    {
                        case sAcolyte:
                            linkedShipType = sACV;
                            break;

                        case sMimic:
                            linkedShipType = sMCV;
                            break;

                        case bAcolyte:
                            linkedShipType = bACV;
                            break;

                        default:
                            done = FALSE;
                            break;
                    }
                    if (!done)
                    {
                        break;
                    }

                    linkedShip = univCreateShip(linkedShipType, ship->shiprace, &partner->posinfo.position, ship->playerowner, FALSE);
                    cat_unitCapCreateShip(linkedShip, linkedShip->playerowner);
                    if (linkedShip->shiptype == sMCV)
                    {
                        linkedShip->rotinfo.coordsys.m11 = -partner->rotinfo.coordsys.m11;
                        linkedShip->rotinfo.coordsys.m21 = -partner->rotinfo.coordsys.m21;
                        linkedShip->rotinfo.coordsys.m31 = -partner->rotinfo.coordsys.m31;
                        linkedShip->rotinfo.coordsys.m12 = -partner->rotinfo.coordsys.m12;
                        linkedShip->rotinfo.coordsys.m22 = -partner->rotinfo.coordsys.m22;
                        linkedShip->rotinfo.coordsys.m32 = -partner->rotinfo.coordsys.m32;
                        linkedShip->rotinfo.coordsys.m13 = partner->rotinfo.coordsys.m13;
                        linkedShip->rotinfo.coordsys.m23 = partner->rotinfo.coordsys.m23;
                        linkedShip->rotinfo.coordsys.m33 = partner->rotinfo.coordsys.m33;
                    }
                    else if (linkedShip->shiptype == sACV || linkedShip->shiptype == bACV)
                    {
                        linkedShip->rotinfo.coordsys = partner->rotinfo.coordsys;
                    }

                    univUpdateObjRotInfo((SpaceObjRot *)linkedShip);
                    univAddObjToRenderListIf((SpaceObj *)linkedShip, (SpaceObj *)ship);
                    if (ShipInSelection((SelectCommand *)&selSelected, ship))
                    {
                        selSelectionAddSingleShipNoCheck((MaxSelection *)&selSelected, linkedShip);
                        ioUpdateShipTotals();
                    }
                    if (ShipInSelection(currentFocus(&universe.mainCameraCommand), ship))
                    {
                        MaxSelection focusSelection;
                        sdword focusIndex;

                        focusSelection.numShips = currentFocus(&universe.mainCameraCommand)->numShips;
                        for (focusIndex = 0; focusIndex < focusSelection.numShips; focusIndex++)
                        {
                            focusSelection.ShipPtr[focusIndex] = currentFocus(&universe.mainCameraCommand)->ShipPtr[focusIndex];
                        }
                        selSelectionAddSingleShipNoCheck(&focusSelection, linkedShip);
                        ccFocus(&universe.mainCameraCommand, (FocusCommand *)&focusSelection);
                    }

                    linkedShip->linkSourceShipHealth0 = ship->health;
                    linkedShip->linkSourceShipHealth1 = partner->health;
                    linkedShip->linkCombinedHealth =
                        ((ship->health / ship->staticinfo->maxhealth) +
                         (partner->health / partner->staticinfo->maxhealth)) *
                        0.5f *
                        linkedShip->staticinfo->maxhealth;
                    linkedShip->health = linkedShip->linkCombinedHealth;
                    if (ship->tacticstype == partner->tacticstype)
                    {
                        linkedShip->tacticstype = ship->tacticstype;
                    }
                    else
                    {
                        linkedShip->tacticstype = Neutral;
                    }

                    if ((((ship->hotKeyGroup & 0xFF) - 1) & 0xF) != 0xF ||
                        ((((partner->hotKeyGroup & 0xFF) - 1) & 0xF) != 0xF))
                    {
                        sdword hotKeyIndex = (((ship->hotKeyGroup & 0xFF) - 1) & 0xF);

                        if (hotKeyIndex == 0xF)
                        {
                            hotKeyIndex = (((partner->hotKeyGroup & 0xFF) - 1) & 0xF);
                        }

                        linkedShip->hotKeyGroup = ((linkedShip->hotKeyGroup | (1 << (hotKeyIndex + 5))) & 0xFFF0) + hotKeyIndex + 1;
                        selSelectionAddSingleShip(&selHotKeyGroup[hotKeyIndex], linkedShip);
                    }

                    clCopyLinkRuntimeState(linkedShip, ship, partner);
                    if (clIsSectOrBeastAcolyte(ship))
                    {
                        clStoreLinkedAcolyteState(linkedShip, ship, partner);
                    }

                    univReallyDeleteThisShipRightNow(ship);
                    univReallyDeleteThisShipRightNow(partner);
                    soundEventPlay(linkedShip, 2100, 0);
                    break;
                }
                done = FALSE;
                break;
            }
        }
    }

    return done;
}

static void clClearLinkState(Ship *ship)
{
    udword shiptype;
    Ship *partner;

    shiptype = ship->shiptype;
    if (shiptype == 1 || shiptype == 2 || shiptype == 4 || shiptype == 5 || shiptype == 34 || shiptype == 38)
    {
        partner = clGetLinkPartner(ship);
        if (partner != NULL)
        {
            ship->specialFlags1C8 &= ~0x40u;
            clSetLinkPartner(ship, NULL);
            partner->specialFlags2 &= ~0x40u;
            clSetLinkPartner(partner, NULL);
            clSetAttribute((sdword)partner, 0, 3);
        }
        clSetAttribute((sdword)ship, 0, 3);
    }
}

static void clClearSelectionLinkState(const SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; i++)
    {
        clClearLinkState(selection->ShipPtr[i]);
    }
}

static void clLinkTwoShips(Ship *ship, Ship *otherShip)
{
    sdword *shipLinkState = clLinkStatePtr(ship);
    sdword *otherLinkState = clLinkStatePtr(otherShip);
    sdword *shipAnimationIndex = clLinkAnimationIndexPtr(ship);
    sdword *otherAnimationIndex = clLinkAnimationIndexPtr(otherShip);

    clSetLinkPartner(ship, otherShip);
    clSetLinkPartner(otherShip, ship);
    dbgAssert(shipLinkState != NULL);
    dbgAssert(otherLinkState != NULL);
    dbgAssert(shipAnimationIndex != NULL);
    dbgAssert(otherAnimationIndex != NULL);
    *shipLinkState = 2;
    *otherLinkState = 5;

    speechEventQueue(ship, 67111068, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);

    switch (ship->shiptype)
    {
        case 1:
        case 34:
            *shipAnimationIndex = 2;
            *otherAnimationIndex = 2;
            break;

        case 2:
            *shipAnimationIndex = 0;
            *otherAnimationIndex = 0;
            shipDisable(ship, TRUE);
            shipDisable(otherShip, TRUE);
            break;
    }

}

static Ship *clFindNearestLinkTarget(SelectCommand *selection, Ship *ship)
{
    Node *node;
    Ship *nearest;
    real32 nearestDistance;

    node = universe.ShipList.head;
    nearest = NULL;
    nearestDistance = 1.0e20f;
    while (node != NULL)
    {
        Ship *candidate = (Ship *)listGetStructOfNode(node);
        if (ShipHasUpgrade(candidate, RESEARCH_UPGRADE_MASK_LINK_PRIMARY) &&
            ShipInSelection(selection, candidate) &&
            candidate->shiptype == ship->shiptype &&
            candidate != ship &&
            clGetLinkPartner(candidate) == NULL)
        {
            real32 dx = candidate->posinfo.position.x - ship->posinfo.position.x;
            real32 dy = candidate->posinfo.position.y - ship->posinfo.position.y;
            real32 dz = candidate->posinfo.position.z - ship->posinfo.position.z;
            real32 distance = (real32)fmathSqrtDouble(dx * dx + dy * dy + dz * dz);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearest = candidate;
            }
        }
        node = node->next;
    }

    return nearest;
}

static void clLinkSelectionRemoveShip(SelectCommand *selection, sdword index)
{
    Ship *ship;

    ship = selection->ShipPtr[index];
    selection->numShips--;
    selection->ShipPtr[index] = selection->ShipPtr[selection->numShips];

    ship->command = NULL;
    if (ship->cataBusyStateWindow.sentinelFormationState == 1)
    {
        clSetAttribute((sdword)ship, 0, 5);
    }
    ship->command = NULL;
}

// Cataclysm-specific: pair selected link-capable ships and assign a link command group.
CommandToDo *clLinkSelection(CommandLayer *comlayer, SelectCommand *selectcom)
{
    CommandToDo *command;
    CommandToDo *existing;
    SelectCommand *linkSelection;
    udword sizeofSelection;
    sdword i;
    bool32 isNewCommand;

    if (selectcom->numShips <= 0)
    {
        telldir();
        return NULL;
    }

    existing = IsSelectionAlreadyDoingSomething(comlayer, selectcom);
    isNewCommand = (existing == NULL);
    if (isNewCommand)
    {
        RemoveShipsFromDoingStuff(comlayer, selectcom);
        command = memAlloc(sizeof(CommandToDo), "ToDo", NonVolatile);
        InitCommandToDo(command);
    }
    else
    {
        command = existing;
        if (command->selection != NULL)
        {
            for (i = 0; i < command->selection->numShips; i++)
            {
                Ship *ship = command->selection->ShipPtr[i];
                if (ship->cataBusyStateWindow.sentinelFormationState == 1)
                {
                    clSetAttribute((sdword)ship, 0, 5);
                }
                if (ship->shiptype == 17 || ship->shiptype == 52)
                {
                    ship->command = NULL;
                }
            }
        }
        if (command->militaryParade != NULL)
        {
            memFree(command->militaryParade);
            command->militaryParade = NULL;
        }
        FreeLastOrder(command);
    }

    sizeofSelection = sizeofSelectCommand(selectcom->numShips);
    linkSelection = memAlloc(sizeofSelection, "ToDoSelection", NonVolatile);
    memcpy(linkSelection, selectcom, sizeofSelection);
    if (!isNewCommand && command->selection != NULL)
    {
        memFree(command->selection);
    }
    command->selection = linkSelection;
    command->ordertype.order = COMMAND_LINK;
    command->ordertype.attributes = 0;

    for (i = 0; i < linkSelection->numShips; i++)
    {
        clClearLinkState(linkSelection->ShipPtr[i]);
    }

    i = 0;
    while (i < linkSelection->numShips)
    {
        Ship *ship = linkSelection->ShipPtr[i];
        Ship *target;

        if (clGetLinkPartner(ship) != NULL)
        {
            clLinkSelectionRemoveShip(linkSelection, i);
            continue;
        }

        target = clFindNearestLinkTarget(linkSelection, ship);
        if (target == NULL)
        {
            clLinkSelectionRemoveShip(linkSelection, i);
            continue;
        }

        if (target->posinfo.position.z < ship->posinfo.position.z)
        {
            sdword targetIndex;
            Ship *tmp;

            targetIndex = -1;
            for (targetIndex = 0; targetIndex < linkSelection->numShips; targetIndex++)
            {
                if (linkSelection->ShipPtr[targetIndex] == target)
                {
                    break;
                }
            }
            if (targetIndex >= 0 && targetIndex < linkSelection->numShips)
            {
                tmp = linkSelection->ShipPtr[i];
                linkSelection->ShipPtr[i] = target;
                linkSelection->ShipPtr[targetIndex] = tmp;
                ship = linkSelection->ShipPtr[i];
                target = linkSelection->ShipPtr[targetIndex];
            }
        }

        clLinkTwoShips(ship, target);
        i++;
    }

    for (i = 0; i < linkSelection->numShips; i++)
    {
        Ship *ship = linkSelection->ShipPtr[i];

        ship->command = command;
        ship->unk_1D0 = 0;
        if ((ship->specialFlags1C8 & 0x10000) != 0)
        {
            rowRemoveShipFromGettingOutOfWay(ship);
        }
    }

    tacticsMakeShipsNotLookForOtherShips(linkSelection);

    for (i = 0; i < linkSelection->numShips; i++)
    {
        Ship *ship = linkSelection->ShipPtr[i];
        *((ubyte *)ship + CATA_LINK_CLEAR0_OFFSET) = 0;
        *((ubyte *)ship + CATA_LINK_CLEAR1_OFFSET) = 0;
        *((ubyte *)ship + CATA_LINK_CLEAR2_OFFSET) = 0;
    }

    if (isNewCommand)
    {
        listAddNode(&comlayer->todolist, &command->todonode, command);
    }
    return command;
}

#if BINNETLOG

void binwriteselection(udword header,SelectCommand *selection)
{
    binnetselectionMax sel;
    sdword i;

    sel.header = header;
    if (selection == NULL)
    {
        sel.numShips = 0;
    }
    else
    {
        sel.numShips = selection->numShips;
        for (i=0;i<sel.numShips;i++)
        {
            sel.ShipID[i] = selection->ShipPtr[i]->shipID.shipNumber;
        }
    }

    fwrite(&sel,sizeofbinnetselection(sel.numShips),1,netlogfile);
}

void binwriteship(Ship *ship)
{
    uword shipid = (uword)-1;

    if (ship != NULL)
    {
        shipid = ship->shipID.shipNumber;
    }

    fwrite(&shipid,sizeof(uword),1,netlogfile);
}

void binwriteanyselection(udword header,SelectAnyCommand *selection)
{
    binnetanyselectionMax sel;
    SpaceObjRotImpTarg *target;
    uword objtype;
    uword id;
    sdword i;

    sel.header = header;
    if (selection == NULL)
    {
        sel.numTargets = 0;
    }
    else
    {
        sel.numTargets = selection->numTargets;
        for (i=0;i<sel.numTargets;i++)
        {
            target = selection->TargetPtr[i];
            objtype = target->objtype;
            id = 0;

            switch (objtype)
            {
                case OBJ_ShipType:
                    id = ((Ship *)target)->shipID.shipNumber;
                    break;

                case OBJ_MissileType:
                    id = ((Missile *)target)->missileID.missileNumber;
                    break;

                case OBJ_AsteroidType:
                case OBJ_NebulaType:
                case OBJ_GasType:
                case OBJ_DustType:
                    id = ((Resource *)target)->resourceID.resourceNumber;
                    break;

                case OBJ_DerelictType:
                    id = ((Derelict *)target)->derelictID.derelictNumber;
                    break;
            }

            sel.TargetID[i] = objtype | (id << 16);
        }
    }

    fwrite(&sel,sizeofbinnetanyselection(sel.numTargets),1,netlogfile);
}

void clChecksum(void)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    SelectCommand *selection;
    binnetCmdLayerInfoMax cmd;
    udword header;
    sdword i;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        selection = command->selection;

        cmd.header = makenetcheckHeader('C','C','C','C');
        cmd.order = command->ordertype.order;
        cmd.attributes = command->ordertype.attributes;
        cmd.numShips = selection->numShips;

        for (i=0;i<cmd.numShips;i++)
        {
            cmd.ShipID[i] = selection->ShipPtr[i]->shipID.shipNumber;
        }

        fwrite(&cmd,sizeofbinnetCmdLayerInfo(cmd.numShips),1,netlogfile);

        if (cmd.attributes & COMMAND_IS_FORMATION)
        {
            header = makenetcheckHeader('F','O','R','M');
            fwrite(&header,sizeof(header),1,netlogfile);
            fwrite(&command->formation,sizeof(TypeOfFormation)+sizeof(bool32),1,netlogfile);
            fwrite(&command->formation.tacticalState,44,1,netlogfile);
        }

        if (cmd.attributes & COMMAND_IS_PROTECTING)
        {
            binwriteselection(makenetcheckHeader('S','E','P','T'),command->protect);
        }

        if (cmd.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            binwriteanyselection(makenetcheckHeader('S','E','P','A'),command->attack);
        }
#if 0
        if (cmd.attributes & COMMAND_IS_ATTACKINGANDMOVING)
        {
            header = makenetcheckHeader('A','T','M','V');
            fwrite(&header,sizeof(header),1,netlogfile);
            fwrite(&command->move,sizeof(MoveCommand),1,netlogfile);
        }
#endif
        switch (cmd.order)
        {
            case COMMAND_MOVE:
                header = makenetcheckHeader('M','O','V','E');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->move,sizeof(MoveCommand),1,netlogfile);
                break;

            case COMMAND_ATTACK:
                binwriteanyselection(makenetcheckHeader('S','E','A','T'),command->attack);
                break;

            case COMMAND_DOCK:
                header = makenetcheckHeader('D','O','C','K');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->dock,sizeof(DockCommand),1,netlogfile);
                break;

            case COMMAND_LAUNCHSHIP:
                header = makenetcheckHeader('L','A','U','N');
                fwrite(&header,sizeof(header),1,netlogfile);
                binwriteship(command->launchship.receiverShip);
                break;

            case COMMAND_COLLECTRESOURCE:
                break;      // don't bother - ship already has this info

            case COMMAND_BUILDINGSHIP:
                header = makenetcheckHeader('B','U','L','D');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->buildingship,sizeof(ShipType)+sizeof(ShipRace),1,netlogfile);
                break;

            case COMMAND_SPECIAL:
                binwriteanyselection(makenetcheckHeader('S','P','E','C'),command->specialtargets);
                break;

            case COMMAND_MILITARYPARADE:
                header = makenetcheckHeader('M','I','L','P');
                fwrite(&header,sizeof(header),1,netlogfile);
                binwriteship(command->militaryParade->aroundShip);
                fwrite(&command->militaryParade->paradeType,sizeof(sdword),1,netlogfile);
                break;

            case COMMAND_MP_HYPERSPACEING:
                header = makenetcheckHeader('M','P','H','P');
                fwrite(&header,sizeof(header),1,netlogfile);
                fwrite(&command->hyperspaceState,sizeof(sdword),1,netlogfile);
                break;
        }

        curnode = curnode->next;
    }
}

#endif

// Cataclysm-specific: returns the current player's owned count for a ship type.
sdword cmGetShipTypeCount(Player *player, ShipType shipType)
{
    return player->shiptotals[shipType];
}






















































void clSetModuleSlotActive(Ship *ship, ShipType moduleType, sdword iconId)
{
    SelectCommand selectcom;

    memset(&selectcom, 0, sizeof(selectcom));
    selectcom.numShips = 1;
    selectcom.ShipPtr[0] = ship;
    clWrapSetShipModuleState(&universe.mainCommandLayer, &selectcom, moduleType, iconId);
}

#if 0  // Cat3: clCreateBeamEffect
/*=== clCreateBeamEffect (0x5c22c0) ===*/
void __cdecl clCreateBeamEffect(int a2, int arg4, vector *heading)
{
  _DWORD *v3; // edi
  int v4; // eax
  struct etgeffectstatic *v5; // edi
  vector a3; // [esp+8h] [ebp-30h] BYREF
  matrix a5; // [esp+14h] [ebp-24h] BYREF

  v3 = (_DWORD *)dword_C41D08;
  matMultiplyMatByVec(&a3, (matrix *)(a2 + 152), *(vector **)(*(_DWORD *)(a2 + 32) + 1020));
  matCreateCoordSysFromHeading(&a5, heading);
  a3.x = a3.x + *(float *)(a2 + 88);
  a3.y = a3.y + *(float *)(a2 + 92);
  a3.z = a3.z + *(float *)(a2 + 96);
  if ( v3
    && (v4 = *(unsigned __int8 *)(a2 + 52), v4 < *v3)
    && (v5 = (struct etgeffectstatic *)v3[v4 + 1]) != 0
    && !etgFrequencyExceeded(v5) )
  {
    *(_DWORD *)(a2 + 908) = etgEffectCreate((int)v5, a2, &a3, (_DWORD *)(a2 + 100), &a5, 1.0, 96, 2);
  }
  else
  {
    *(_DWORD *)(a2 + 908) = 0;
  }
}
#endif

#if 0  // Cat3: clUpdateBeamAttachment
/*=== clUpdateBeamAttachment (0x5c2410) ===*/
int __cdecl clUpdateBeamAttachment(int a1, int a2, vector *heading, float a4, int a5)
{
  double v5; // st7
  int v6; // eax
  vector result; // [esp+8h] [ebp-Ch] BYREF

  matMultiplyMatByVec(&result, (matrix *)(a2 + 152), *(vector **)(*(_DWORD *)(a2 + 32) + 1020));
  *(float *)(a1 + 88) = result.x + *(float *)(a2 + 88);
  *(float *)(a1 + 92) = result.y + *(float *)(a2 + 92);
  *(float *)(a1 + 96) = result.z + *(float *)(a2 + 96);
  matCreateCoordSysFromHeading((matrix *)(a1 + 152), heading);
  v5 = a4;
  if ( flt_72A440 > (double)a4 )
    v5 = flt_72A440;
  if ( !*(_DWORD *)(a5 + 16) && !*(_DWORD *)(*(_DWORD *)(a5 + 32) + 488) && flt_72BFE8 > v5 )
    v5 = flt_72BFE8;
  **(float **)(a1 + 288) = v5;
  v6 = *(_DWORD *)(a5 + 32);
  *(_DWORD *)(*(_DWORD *)(a1 + 288) + 4) = *(_DWORD *)(v6 + 76);
  return v6;
}
#endif

static AttackSideStepParameters clBeamStaticParameters;

static scriptStructEntry clBeamStaticScriptTable[] =
{
    { "repositionTime", scriptSetReal32CB, (udword)&(clBeamStaticParameters.repositionTime), (udword)&(clBeamStaticParameters) },
    { "circleRange", scriptSetReal32CB, (udword)&(clBeamStaticParameters.circleRange), (udword)&(clBeamStaticParameters) },
    { "fullMovementFreedom", scriptSetBool, (udword)&(clBeamStaticParameters.fullMovementFreedom), (udword)&(clBeamStaticParameters) },
    { NULL, NULL, 0, 0 }
};

void clBeamStaticScriptLoad(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    scriptSetStruct(directory, filename, clBeamStaticScriptTable, (ubyte *)&clBeamStaticParameters);
    statinfo->custstatinfo = &clBeamStaticParameters;
}

BOOL clGunIsTargetInCone(float *a1, float *a2)
{
    vector a;
    real32 v3;

    if (!a1)
    {
        telldir();
        return 1;
    }
    a.x = a1[2] - *a2;
    a.y = a1[3] - a2[1];
    a.z = a1[4] - a2[2];
    vecNormalize(&a);
    v3 = a1[8] * (1.0f / PI);
    if (v3 < 0.0f)
        v3 = -v3;
    return a.z * a1[7] + a.y * a1[6] + a.x * a1[5] >= v3;
}

/*=============================================================================
    Name    : clUpdateWeaponTrail
    Description: Updates or creates the weapon trail effect on a collecting
                 ship.  Rotates the dock attachment point into world space,
                 advances it one physics step along the ship's velocity, then
                 builds a heading coordinate system toward the resource and
                 either updates an existing trail effect's pose or creates a
                 new one.
=============================================================================*/
void clUpdateWeaponTrail(Ship *ship, Resource *resource, sdword commandCode, DockStaticPoint *dockPoint)
{
    etglod *lod;
    etgeffectstatic *effectStat;
    sdword currentLOD;
    vector gunWorldPos;
    vector heading;
    matrix headingCoordSys;
    real32 dist, oneOverDist;
    Effect *rcEff;

    lod = etgSpecialPurposeEffectTable[commandCode];

    // Rotate the dock attachment point into world space and offset by the ship's
    // position, then advance one physics frame along the ship's velocity so the
    // trail origin leads the gun barrel slightly.
    matMultiplyMatByVec(&gunWorldPos, &ship->rotinfo.coordsys, &dockPoint->position);
    gunWorldPos.x += ship->posinfo.position.x;
    gunWorldPos.y += ship->posinfo.position.y;
    gunWorldPos.z += ship->posinfo.position.z;
    gunWorldPos.x += universe.phystimeelapsed * ship->posinfo.velocity.x;
    gunWorldPos.y += universe.phystimeelapsed * ship->posinfo.velocity.y;
    gunWorldPos.z += universe.phystimeelapsed * ship->posinfo.velocity.z;

    // Build a normalized heading from the gun world position toward the resource.
    heading.x = resource->posinfo.position.x - gunWorldPos.x;
    heading.y = resource->posinfo.position.y - gunWorldPos.y;
    heading.z = resource->posinfo.position.z - gunWorldPos.z;
    dist = (real32)fmathSqrtDouble(heading.z * heading.z + heading.y * heading.y + heading.x * heading.x);
    oneOverDist = 1.0f / dist;
    heading.x *= oneOverDist;
    heading.y *= oneOverDist;
    heading.z *= oneOverDist;

    matCreateCoordSysFromHeading(&headingCoordSys, &heading);

    currentLOD = ship->currentLOD;
    if (lod != NULL && currentLOD < lod->nLevels)
        effectStat = lod->level[currentLOD];
    else
        effectStat = NULL;

    rcEff = ship->rceffect;
    if (rcEff != NULL)
    {
        rcEff->posinfo.position = gunWorldPos;
        memcpy(&rcEff->rotinfo.coordsys, &headingCoordSys, sizeof(matrix));
    }
    else if (!effectStat || etgFrequencyExceeded(effectStat))
    {
        ship->rceffect = NULL;
    }
    else
    {
        // Pass beam length (dist to resource) and beam half-width (half the resource's
        // avoidance sphere) as ETG_ResourceLengthParam and ETG_ResourceRadiusParam.
        // Binary: arg8 LODWORD slot is overwritten with dist, HIDWORD slot with
        // avoidcollspheresize*0.5, before the etgEffectCreate call at 0x5C8FA9.
        real32 beamRadius = resource->staticinfo->staticheader.staticCollInfo.avoidcollspheresize * 0.5f;
        ship->rceffect = (Effect *)etgEffectCreate(effectStat, NULL, &gunWorldPos, &ship->posinfo.velocity, &headingCoordSys, 1.0f, 32, 2, TreatAsUdword(dist), TreatAsUdword(beamRadius));
    }
}

/*=============================================================================
    Name    : clGunConeCheckAndTrack
    Description: Checks whether the resource is inside the ship's gun cone
                 (90-degree full-angle).  If mode is 0 (track mode) and the
                 resource is already in the cone, rotation is stopped;
                 otherwise the ship is rotated toward the resource.  Returns
                 TRUE if the resource is in the cone.
=============================================================================*/
bool32 clGunConeCheckAndTrack(SpaceObjRotImpTargGuidance *ship, Resource *resource, sdword mode)
{
    real32 cosHalfAngle;
    vector dir;
    real32 dotProduct;
    bool32 inCone;

    // Half-angle cosine for a 90-degree full cone (45-degree half-angle).
    cosHalfAngle = (real32)cos(DEG_TO_RAD(45.0f));

    dir.x = resource->posinfo.position.x - ship->posinfo.position.x;
    dir.y = resource->posinfo.position.y - ship->posinfo.position.y;
    dir.z = resource->posinfo.position.z - ship->posinfo.position.z;
    vecNormalize(&dir);

    dotProduct = ship->rotinfo.coordsys.m13 * dir.x +
                 ship->rotinfo.coordsys.m23 * dir.y +
                 ship->rotinfo.coordsys.m33 * dir.z;
    inCone = dotProduct >= cosHalfAngle;

    if (!mode)
    {
        if (dotProduct >= cosHalfAngle)
        {
            ship->rotinfo.rotspeed.x = 0.0f;
            ship->rotinfo.rotspeed.y = 0.0f;
            ship->rotinfo.rotspeed.z = 0.0f;
            ship->rotinfo.torque.x = 0.0f;
            ship->rotinfo.torque.y = 0.0f;
            ship->rotinfo.torque.z = 0.0f;
            return inCone;
        }
        // Recompute direction to resource before handing off to the heading tracker
        // (mirrors the binary's redundant recompute before aitrackHeadingFunc).
        dir.x = resource->posinfo.position.x - ship->posinfo.position.x;
        dir.y = resource->posinfo.position.y - ship->posinfo.position.y;
        dir.z = resource->posinfo.position.z - ship->posinfo.position.z;
        vecNormalize(&dir);
        aitrackHeadingFunc(ship, &dir, 0.99000001f, 0.99000001f, 0.0f, 0);
    }
    return inCone;
}
