#include <stdlib.h>
#include <string.h>
#include "AITeam.h"
#include "AIEvents.h"
#include "AIVar.h"
#include "AIMoves.h"
#include "memory.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "CommandWrap.h"
#include "randy.h"
#include "AIUtilities.h"
#include "SaveGame.h"

bool32 aimIssueMoveCommand(SelectCommand *ships, vector destination)
{
    MaxSelection filtered;

    if (ships->numShips == 0)
    {
        return FALSE;
    }

    filtered.numShips = 0;
    if (!aiuFilterSelectableShips(ships, &filtered))
    {
        return FALSE;
    }

    clWrapMovePoint(&universe.mainCommandLayer, (SelectCommand *)&filtered, ships->ShipPtr[0]->posinfo.position, destination);
    return TRUE;
}

sdword pingEvalAlwaysTrue(void)
{
    return TRUE;
}

sdword aimCheckIfShipsArrived(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;

    if (team->shipList.selection->numShips == 0)
    {
        return TRUE;
    }

    if (thisMove->processing)
    {
        if (!aitTeamIsFinishedMoving(team, thisMove->params.move.destination, AIM_DIST_MOVE_END))
        {
            return !thisMove->wait;
        }
        return TRUE;
    }

    if (aimIssueMoveCommand(team->shipList.selection, thisMove->params.move.destination))
    {
        thisMove->processing = TRUE;
        return !thisMove->wait;
    }

    return FALSE;
}

sdword aimProcessGuardShips(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    SelectCommand *selection = team->shipList.selection;

    if (team->shipList.selection->numShips == 0)
    {
        return FALSE;
    }

    if (!thisMove->processing)
    {
        if ((selection->numShips > 0) && (thisMove->params.guardShips.ships->numShips > 0))
        {
            if (aiuWrapProtect(selection, thisMove->params.guardShips.ships))
            {
                thisMove->processing = TRUE;        // only consider move processed if was successful issuing it
            }
        }
        else
        {
            aiplayerLog((aiIndex,"Warning: no ships to guard"));
            thisMove->processing = TRUE;
        }
        return FALSE;
    }
    else
    {
        if ((selection->numShips == 0) || (thisMove->params.guardShips.ships->numShips == 0))
        {
            memFree(thisMove->params.guardShips.ships);
            thisMove->params.guardShips.ships = NULL;

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

void aimShipDiedGuardShips(AITeam *team,AITeamMove *move,Ship *ship)
{
    SelectCommand *ships = move->params.guardShips.ships;
    if (ships != NULL)
    {
        if (clRemoveShipFromSelection(ships,ship))
        {
            // note! one of the ships we were guarding died!
            ;
        }
    }
}

void aimCloseGuardShips(AITeam *team,AITeamMove *move)
{
    if (move->params.guardShips.ships != NULL)
    {
        memFree(move->params.guardShips.ships);
        move->params.guardShips.ships = NULL;
    }
}

AITeamMove *aimCreateGuardShips(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "guardshipsmove", 0);

    InitNewMove(newMove,MOVE_GUARDSHIPS,wait,remove,formation,Neutral,aimProcessGuardShips,aimShipDiedGuardShips,aimCloseGuardShips);

    newMove->params.guardShips.ships  = ships;

//    aiplayerLog((aiIndex,"Created GuardShips Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_GuardShips(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessGuardShips,aimShipDiedGuardShips,aimCloseGuardShips);
}

void aimSave_GuardShips(AITeamMove *move)
{
    if (move->params.guardShips.ships) SaveSelection((SpaceObjSelection *)move->params.guardShips.ships);
}

void aimLoad_GuardShips(AITeamMove *move)
{
    if (move->params.guardShips.ships) move->params.guardShips.ships = (SelectCommand *)LoadSelectionAndFix();
}

sdword aimProcessGetShips(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    sdword numShipsRequested, typeRequested,numShipsToBuild;
    AIVar *doneVar;
    char label[AIVAR_LABEL_MAX_LENGTH+1];

    if (!thisMove->processing)
    {
        typeRequested = thisMove->params.getShips.shipType;
        numShipsRequested = thisMove->params.getShips.numShips;
        doneVar = aivarCreate(aivarLabelGenerate(label));
        aivarValueSet(doneVar, 0);
        thisMove->params.getShips.doneVar = doneVar;

        numShipsToBuild = aifShipLimitAdjust(aiCurrentAIPlayer, typeRequested, numShipsRequested);

        //if the ships are already built
        if (aiuAlreadyHasShipType(typeRequested, numShipsRequested))
        {
            aifTeamRequestsShipsCB(typeRequested,
                                   numShipsRequested,
                                   team,
                                   aivarLabelGet(doneVar),
                                   thisMove->params.getShips.priority);
        }
        //if the shiptype can be built
        //and if the numships unit caps will let us build is more than the number requested (with a bit of leeway)
        else if (aiuCanBuildShipType(typeRequested,(team->teamType == ScriptTeam)) &&
                 (numShipsToBuild > (numShipsRequested*0.75)))
        {
            //build only the number of ships unit caps will let us build
            aifTeamRequestsShipsCB(typeRequested,
                                  numShipsToBuild,
                                  team,
                                  aivarLabelGet(doneVar),
                                  thisMove->params.getShips.priority);
        }
        else
        {
            aiplayerLog((aiIndex, "Get Ships - can't build shiptype %i", thisMove->params.getShips.shipType));
        }

        thisMove->processing = TRUE;
    }

    // normally in a move processing function, we'd check the
    // thisMove->wait flag, but...
    //
    // always wait for the ships to be built (we could loosen this a bit later, maybe)
    // but consider what happens when the rest of the ships are actually built if we
    // leave early
    return aivarValueGet(thisMove->params.getShips.doneVar);
}

void aimCloseGetShips(AITeam *team,AITeamMove *move)
{
    if (move->params.getShips.doneVar != NULL)
    {
        aivarDestroy(move->params.getShips.doneVar);
        move->params.getShips.doneVar = NULL;
    }
}

AITeamMove *aimCreateGetShipsNoAdd(AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AlternativeShips alternatives;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "getshipsmove", 0);

    SetNumAlternatives(alternatives, 0);
    newMove = aimCreateFancyGetShipsNoAdd(team, shiptype, num_ships, &alternatives, priority, wait, remove);

/*    dbgAssert(num_ships > 0);

    InitNewMove(newMove,MOVE_GETSHIPS,wait,remove,formation,Neutral,aimProcessGetShips,NULL,aimCloseGetShips);

    newMove->params.getShips.shipType = shiptype;
    newMove->params.getShips.numShips = num_ships;
    newMove->params.getShips.priority = priority;
    newMove->params.getShips.doneVar = NULL;

    aiplayerLog((aiIndex,"Created GetShips Move"));
*/
    return newMove;
}

AITeamMove *aimCreateGetShips(AITeam *team, ShipType shiptype, sbyte num_ships, sdword priority, bool8 wait, bool8 remove)
{
    AITeamMove *newMove;

    dbgAssert(num_ships > 0);

    newMove = aimCreateGetShipsNoAdd(team, shiptype, num_ships, priority, wait, remove);

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_GetShips(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessGetShips,NULL,aimCloseGetShips);
    move->params.getShips.doneVar = NumberToAIVar((sdword)move->params.getShips.doneVar);
}

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

void aimPreFix_GetShips(AITeamMove *move)
{
    move->params.getShips.doneVar = AIVarToNumber(move->params.getShips.doneVar);
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

sdword aimProcessVarWait(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varWait.varName);

    thisMove->processing = TRUE;
    if (var &&
        thisMove->params.varWait.value == aivarValueGet(var))
        return TRUE;

    // variable hasn't reached its target value, so pay attention to the wait flag
    return !(thisMove->wait);
}


AITeamMove *aimCreateVarWait(AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varwaitmove", 0);

    InitNewMove(newMove,MOVE_VARWAIT,wait,remove,formation,Neutral,aimProcessVarWait,NULL,NULL);

    memStrncpy(newMove->params.varWait.varName, varName, AIVAR_LABEL_MAX_LENGTH);
/*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varWait.varName[strlen(varName)] = 0;
    else
        newMove->params.varWait.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
*/
    newMove->params.varWait.value     = value;

//    aiplayerLog((aiIndex,"Created VarWait Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarWait(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarWait,NULL,NULL);
}

sdword aimProcessVarDec(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varDec.varName);

    if (!thisMove->processing)
    {
        if (var)
            aivarValueSet(var, aivarValueGet(var)-1);
        thisMove->processing = TRUE;
    }

    // nothing to wait on
    return TRUE;
}


AITeamMove *aimCreateVarDec(AITeam *team, char *varName, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "vardecmove", 0);

    InitNewMove(newMove,MOVE_VARDEC,wait,remove,formation,Neutral,aimProcessVarDec,NULL,NULL);

    memStrncpy(newMove->params.varDec.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varDec.varName[strlen(varName)] = 0;
    else
        newMove->params.varDec.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
        */

//    aiplayerLog((aiIndex,"Created VarDec Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarDec(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarDec,NULL,NULL);
}

sdword aimProcessVarInc(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varInc.varName);

    if (!thisMove->processing)
    {
        if (var)
            aivarValueSet(var, aivarValueGet(var)+1);
        thisMove->processing = TRUE;
    }

    // nothing to wait on
    return TRUE;
}


AITeamMove *aimCreateVarInc(AITeam *team, char *varName, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varincmove", 0);

    InitNewMove(newMove,MOVE_VARINC,wait,remove,formation,Neutral,aimProcessVarInc,NULL,NULL);

    memStrncpy(newMove->params.varInc.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varInc.varName[strlen(varName)] = 0;
    else
        newMove->params.varInc.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
        */

//    aiplayerLog((aiIndex,"Created VarInc Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarInc(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarInc,NULL,NULL);
}

sdword aimProcessVarSet(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varSet.varName);

    if (!thisMove->processing)  // probably not necessary to check for this simple command
    {
        aivarValueSet(var, thisMove->params.varSet.value);
        thisMove->processing = TRUE;
    }

    return TRUE;
}


AITeamMove *aimCreateVarSet(AITeam *team, char *varName, sdword value, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varsetmove", 0);

    InitNewMove(newMove,MOVE_VARSET,wait,remove,formation,Neutral,aimProcessVarSet,NULL,NULL);

    memStrncpy(newMove->params.varSet.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varSet.varName[strlen(varName)] = 0;
    else
        newMove->params.varSet.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
*/
    newMove->params.varSet.value = value;

//    aiplayerLog((aiIndex,"Created VarSet Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarSet(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarSet,NULL,NULL);
}

sdword aimProcessVarDestroy(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    AIVar *var = aivarFind(thisMove->params.varDestroy.varName);

    if (!thisMove->processing)  // probably not necessary to check for this simple command
    {
        aivarDestroy(var);
        thisMove->processing = TRUE;
    }

    return TRUE;
}

AITeamMove *aimCreateVarDestroy(AITeam *team, char *varName, bool8 wait, bool8 remove)
{
    TypeOfFormation formation = SAME_FORMATION;
    AITeamMove *newMove = (AITeamMove *)memAlloc(sizeof(AITeamMove), "varsetmove", 0);

    InitNewMove(newMove,MOVE_VARDESTROY,wait,remove,formation,Neutral,aimProcessVarDestroy,NULL,NULL);

    memStrncpy(newMove->params.varDestroy.varName, varName, AIVAR_LABEL_MAX_LENGTH);
    /*
    if (strlen(varName) < AIVAR_LABEL_MAX_LENGTH)
        newMove->params.varDestroy.varName[strlen(varName)] = 0;
    else
        newMove->params.varDestroy.varName[AIVAR_LABEL_MAX_LENGTH] = 0;
        */

//    aiplayerLog((aiIndex,"Created VarDestroy Move"));

    listAddNode(&(team->moves), &(newMove->listNode), newMove);

    return newMove;
}

void aimFix_VarDestroy(AITeamMove *move)
{
    FixMoveFuncPtrs(move,aimProcessVarDestroy,NULL,NULL);
}

// Cataclysm-specific: generic ship-died callback for moves that store a target selection at params slot 5.
void aimShipDied(AITeam *team, AITeamMove *move, Ship *deadShip)
{
    SelectAnyCommand *targets = *((SelectAnyCommand **)move + 5);

    if (targets != NULL)
    {
        clRemoveTargetFromSelection(targets, (TargetPtr)deadShip);
    }
}

// Cataclysm-specific: generic close callback for moves that own a selection at params slot 5.
void aimCloseGeneric(AITeam *team, AITeamMove *move)
{
    if (*((ubyte **)move + 5) != NULL)
    {
        memFree(*((ubyte **)move + 5));
        *((ubyte **)move + 5) = NULL;
    }
}

//
//  temporary contention relief
//
#include "AIMoves1.c"  // Falko
#include "AIMoves2.c"  // Gary



#if 0  // Cat2: raw IDA offsets, multiple functions not compilable as-is
void __cdecl aimShipDied(AITeam *team, AITeamMove *move, Ship *deadShip)
{
    SelectCommand *targets;

    targets = move->params.attack.targets;
    if (targets)
        clRemoveTargetFromSelection(targets, deadShip);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x40D5B0
signed int aimDoAttack(AITeam *team)
{
  int v1;
  unsigned int *v2;
  int v3;
  char v4;
  signed int result;

  v1 = *(_DWORD *)(a1 + 60);   // move/team data struct
  v2 = *(unsigned int **)(a1 + 32);  // ship selection
  if ( !*v2 )
    return 1;  // no ships, done
  v3 = *v2;    // numShips
  if ( *(_BYTE *)(v1 + 356) )  // processing flag
  {
    // Phase 2: check completion
    if ( v3 )
    {
      if ( **(_DWORD **)(v1 + 20) )  // targets exist
      {
        result = sub_41A9A0(*(unsigned int **)(a1 + 32));  // check if targets dead
        if ( !result )
          return result;  // not done yet
      }
    }
    memFree(*(ubyte **)(v1 + 20));
    *(_DWORD *)(v1 + 20) = 0;
    return 1;  // attack complete
  }
  // Phase 1: initiate attack
  if ( v3 > 0 && **(int **)(v1 + 20) > 0 )
    sub_41ED00(v2, *(unsigned int **)(v1 + 20));  // assign ships to targets
  v4 = *(_BYTE *)(v1 + 357);
  *(_BYTE *)(v1 + 356) = 1;  // set processing flag
  return v4 == 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4078A0
int *aimRemoveTargetFromMove(AITeam *team, AITeamMove *move, SpaceObjRotImpTarg *target)
{
    int *result;
    result = *(int **)(move + 20);  // move->params.attack.targets
    if (result)
    {
        result = (int *)clRemoveTargetFromSelection(result, target);
        if (result)
        {
            if (*(int *)(team + 60) == move)  // if team->curMove == move
                *(BYTE *)(move + 356) = 0;    // clear processing flag
        }
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x407C20
BOOL aimCheckIfShipsArrived(AITeam *team)
{
    unsigned int *selection = *(unsigned int **)(team + 32);  // team->shipList.selection
    int move = *(int *)(team + 60);                           // team->curMove
    if (!*selection)                                           // numShips == 0
        return TRUE;
    if (*(BYTE *)(move + 356))                                 // if already arrived
    {
        if (!sub_417310(team, *(float *)(move + 20), *(int *)(move + 24), *(int *)(move + 28), dword_74AAA4))
            return *(BYTE *)(move + 357) == 0;
        return TRUE;
    }
    else
    {
        if (sub_41EEA0(selection, *(int *)(move + 20)))        // check ships reached dest
        {
            *(BYTE *)(move + 356) = 1;                         // set arrived flag
            return *(BYTE *)(move + 357) == 0;
        }
        return FALSE;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4082A0
int aimProcessMoveWait(AITeam *team)
{
    SelectCommand *ships = team->shipList.selection;
    AIMoveData *moveData = team->curMove;
    
    // If no ships, move is complete
    if (!ships->numShips)
        return 1;
    
    if (!moveData->processing)
    {
        // First call: check if ships already at destination
        if (!aimShipsAtDestination(ships))
        {
            // Issue move command to target
            aimIssueMoveCommand(ships, moveData->target);
            moveData->startTime = universe.totaltimeelapsed;
            vector centre;
            selCentrePointComputeGeneral(&centre, ships, &team);
            moveData->startPos = centre;
            moveData->processing = 1;
        }
        return 0;
    }
    
    int checkType = moveData->checkType;
    if (checkType == 0)
    {
        // Time-based: wait for duration
        if (universe.totaltimeelapsed - moveData->startTime <= moveData->threshold)
            return 0;
        aimCleanupMove(ships);
        return 1;
    }
    else if (checkType == 1)
    {
        // Distance-based: wait until ships moved far enough
        vector centre;
        selCentrePointComputeGeneral(&centre, ships, &team);
        real32 dist = fmathSqrtDouble(aiuFindDistanceSquared(
            centre.x, centre.y, centre.z,
            moveData->startPos.x, moveData->startPos.y, moveData->startPos.z));
        if (dist <= moveData->threshold)
            return 0;
        aimCleanupMove(ships);
        return 1;
    }
    return 0;
}


// ============================================================================
// Decompiled from cataclysm.exe @ 0x40B9B0
AITeamMove *aimCreateGetShipsNoAdd(AITeam *team, ShipType shipType, sbyte numShips, AlternativeShips *alternatives, sdword priority, bool8 wait, bool8 remove)
{
    _DWORD *newMove;
    newMove = memAllocFunction(368, MBF_Volatile);
    newMove[4] = 6;                     // MOVE_GETSHIPS
    *((_BYTE *)newMove + 356) = 0;
    *((_BYTE *)newMove + 357) = wait;
    *((_BYTE *)newMove + 358) = remove;
    newMove[90] = priority;
    newMove[91] = 2;                    // tactics = Neutral
    newMove[85] = aimCheckIfShipsArrived;
    newMove[86] = 0;
    newMove[87] = 0;
    aieHandlersClear(newMove);
    telldir();
    newMove[5] = shipType;
    newMove[6] = numShips;
    newMove[7] = alternatives;
    return newMove;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x40DA50
sdword aimProcessFancyGetShips(AITeam *team)
{
    AITeamMove *thisMove = team->curMove;
    ShipType shiptype;
    sdword numPointsLeftToGet;
    AlternativeShips *alternativeShips;
    AIVar *doneVar;
    char label[48];
    AIPlayer *aiplayer;
    Player *player;
    sdword i, j;

    if (thisMove->processing)
        return aivarValueGet(thisMove->params.fancyGetShips.doneVar);

    shiptype = thisMove->params.fancyGetShips.shipType;
    numPointsLeftToGet = thisMove->params.fancyGetShips.numShips * ALTERNATIVE_SHIP_BASE;
    alternativeShips = &thisMove->params.fancyGetShips.alternatives;

    doneVar = aivarCreate(aivarLabelGenerate(label));
    aivarValueSet(doneVar, 0);
    thisMove->params.fancyGetShips.doneVar = doneVar;

    // Try to get ships of the primary type from reserves
    if (numPointsLeftToGet > 0)
    {
        aiplayer = aiCurrentAIPlayer;
        player = aiplayer->player;
        i = 0;
        while (i < (sdword)player->PlayerMothership)
        {
            Ship *ship = (&player->unk)[i];
            if (ship->staticinfo->shiptype == shiptype)
            {
                growSelectRemoveShipIndex(&aiCurrentAIPlayer->newships, i);
                aifAddShipToTeam(team, ship);
                numPointsLeftToGet -= ALTERNATIVE_SHIP_BASE;
                if (numPointsLeftToGet <= 0)
                    break;
                continue;  // restart search
            }
            i++;
        }
    }

    // Try alternative ship types from reserves
    j = 0;
    if (alternativeShips->numNextPicks > 0)
    {
        sdword *altList = &thisMove->params.fancyGetShips.alternatives.shipNextPicks[0];
        while (numPointsLeftToGet > 0)
        {
            ShipType altType = *altList;
            sdword altWeight = altList[20];

            aiplayer = aiCurrentAIPlayer;
            player = aiplayer->player;
            i = 0;
            while (i < (sdword)player->PlayerMothership)
            {
                Ship *ship = (&player->unk)[i];
                if (ship->staticinfo->shiptype == altType)
                {
                    growSelectRemoveShipIndex(&aiCurrentAIPlayer->newships, i);
                    aifAddShipToTeam(team, ship);
                    numPointsLeftToGet -= altWeight;
                    if (numPointsLeftToGet <= 0)
                        break;
                    continue;
                }
                i++;
            }
            altList++;
            if (++j >= alternativeShips->numNextPicks)
            {
                shiptype = thisMove->params.fancyGetShips.shipType;
                goto checkDone;
            }
        }
        goto allDone;
    }

checkDone:
    if (numPointsLeftToGet <= 0)
    {
allDone:
        aivarValueSet(doneVar, 1);
        goto finish;
    }

    // Try random alternative if ALTERNATIVE_RANDOM flag set
    if (thisMove->params.fancyGetShips.alternatives.flags & ALTERNATIVE_RANDOM)
    {
        udword pick = ranRandomFn(6) % (alternativeShips->numNextPicks + 1);
        if (pick)
        {
            pick--;
            sdword tried = 0;
            while (1)
            {
                if (sub_412CB0(thisMove->params.fancyGetShips.alternatives.shipNextPicks[pick],
                               team->teamType == 3))
                {
                    sdword numShipsToGet = numPointsLeftToGet / thisMove->params.fancyGetShips.alternatives.shipNumEquivNextPicks[pick];
                    if (!numShipsToGet) numShipsToGet = 1;
                    sdword numAvail = sub_41E500(aiCurrentAIPlayer, thisMove->params.fancyGetShips.alternatives.shipNextPicks[pick], numShipsToGet);
                    if (numAvail == numShipsToGet)
                    {
                        aifTeamRequestsShipsCB(thisMove->params.fancyGetShips.alternatives.shipNextPicks[pick],
                            numShipsToGet, team, aivarLabelGet(doneVar), thisMove->params.fancyGetShips.priority);
                        goto finish;
                    }
                    if ((double)numShipsToGet * 0.5 < (double)numAvail)
                    {
                        numPointsLeftToGet = thisMove->params.fancyGetShips.alternatives.shipNumEquivNextPicks[pick] * (numShipsToGet - numAvail);
                        aifTeamRequestsShipsCB(thisMove->params.fancyGetShips.alternatives.shipNextPicks[pick],
                            numAvail, team, aivarLabelGet(doneVar), thisMove->params.fancyGetShips.priority);
                    }
                    shiptype = thisMove->params.fancyGetShips.shipType;
                }
                if (++tried >= alternativeShips->numNextPicks) break;
                if ((sdword)++pick >= alternativeShips->numNextPicks) pick = 0;
            }
        }
    }

    // Fall back to primary ship type
    if (sub_412CB0(shiptype, team->teamType == 3))
    {
        sdword numShipsToGet = numPointsLeftToGet / ALTERNATIVE_SHIP_BASE;
        if (!numShipsToGet) numShipsToGet = 1;
        sdword numAvail = sub_41E500(aiCurrentAIPlayer, shiptype, numShipsToGet);
        if (numAvail == numShipsToGet)
        {
            aifTeamRequestsShipsCB(shiptype, numShipsToGet, team, aivarLabelGet(doneVar), thisMove->params.fancyGetShips.priority);
            goto finish;
        }
        if ((double)numShipsToGet * 0.5 < (double)numAvail)
        {
            aifTeamRequestsShipsCB(shiptype, numAvail, team, aivarLabelGet(doneVar), thisMove->params.fancyGetShips.priority);
            goto finish;
        }

        aivarDestroy(doneVar);
        thisMove->params.fancyGetShips.doneVar = NULL;

        // Check if existing team ships match requirements
        SelectCommand *sel = team->shipList;
        if (sel->numShips == 0)
            return 0;
        ShipType existingType = sel->ShipPtr[0]->staticinfo->shiptype;
        if (existingType == thisMove->params.fancyGetShips.shipType &&
            (sdword)sel->numShips >= thisMove->params.fancyGetShips.numShips)
            return 1;

        for (i = 0; i < alternativeShips->numNextPicks; i++)
        {
            if (existingType == alternativeShips->shipNextPicks[i] &&
                sel->numShips >= (10 * thisMove->params.fancyGetShips.numShips) / alternativeShips->shipNumEquivNextPicks[i])
                return 1;
        }
        return 0;
    }
    else
    {
        // Try each alternative type
        sdword altIdx = 0;
        if (alternativeShips->numNextPicks > 0)
        {
            sdword *altShipTypes = &thisMove->params.fancyGetShips.alternatives.shipNextPicks[0];
            while (1)
            {
                if (sub_412CB0(*altShipTypes, team->teamType == 3))
                {
                    sdword numShipsToGet = numPointsLeftToGet / altShipTypes[20];
                    if (!numShipsToGet) numShipsToGet = 1;
                    sdword numAvail = sub_41E500(aiCurrentAIPlayer, *altShipTypes, numShipsToGet);
                    if (numAvail == numShipsToGet)
                    {
                        aifTeamRequestsShipsCB(thisMove->params.fancyGetShips.alternatives.shipNextPicks[altIdx],
                            numShipsToGet, team, aivarLabelGet(doneVar), thisMove->params.fancyGetShips.priority);
                        goto finish;
                    }
                    if ((double)numShipsToGet * 0.5 < (double)numAvail)
                    {
                        numPointsLeftToGet = ALTERNATIVE_SHIP_BASE * (numAvail - numShipsToGet);
                        aifTeamRequestsShipsCB(*altShipTypes, numAvail, team, aivarLabelGet(doneVar), thisMove->params.fancyGetShips.priority);
                    }
                }
                altShipTypes++;
                if (++altIdx >= alternativeShips->numNextPicks)
                    break;
            }
        }
        aivarDestroy(doneVar);
        thisMove->params.fancyGetShips.doneVar = NULL;
        team->teamFlags |= 0x10;
    }
    return 1;

finish:
    thisMove->processing = TRUE;
    return aivarValueGet(thisMove->params.fancyGetShips.doneVar);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x40BA50
AITeamMove *aimCreateMoveTeam(AITeam *team, vector destination, TypeOfFormation formation, bool8 wait, bool8 remove)
{
    AITeamMove *newMove = memAlloc(sizeof(AITeamMove), MBF_Volatile);

    newMove->type = MOVE_MOVETEAM;
    newMove->processing = FALSE;
    newMove->wait = wait;
    newMove->remove = remove;
    newMove->formation = formation;
    newMove->tactics = Neutral;
    newMove->processFunc = aimCheckIfShipsArrived;
    newMove->preFunc = NULL;
    newMove->closeFunc = NULL;

    aieHandlersClear(newMove);
    aieHandlerSetGettingRocked(newMove, FALSE, aihGenericGettingRockedHandler);

    newMove->params.move.destination = destination;

    listAddNode(&team->moves, &newMove->listNode, newMove);

    return newMove;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x40DA30
int __cdecl aimCreateFancyInit(int team, int moveData)
{
    int result;

    result = *(sdword *)(moveData + 28);
    if (result)
    {
        result = aivarDestroy(*(ubyte **)(moveData + 28));
        *(sdword *)(moveData + 28) = 0;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x406E90
sdword aimProcessGuardShips(AITeam *team)
{
int v1; // esi
unsigned int *v2; // eax
int v4; // ecx

v1 = *(_DWORD *)(a1 + 60);
v2 = *(unsigned int **)(a1 + 32);
if ( !*v2 )
return 0;
v4 = *v2;
if ( *(_BYTE *)(v1 + 356) )
{
if ( v4 && **(_DWORD **)(v1 + 20) )
{
return 0;
}
else
{
memFree(*(ubyte **)(v1 + 20));
*(_DWORD *)(v1 + 20) = 0;
return 1;
}
}
else
{
if ( v4 <= 0 || **(int **)(v1 + 20) <= 0 || aiuWrapProtect(v2, *(unsigned int **)(v1 + 20)) )
*(_BYTE *)(v1 + 356) = 1;
return 0;
}
}


// ============================================================================
// Decompiled from cataclysm.exe @ 0x406F20
AITeamMove *aimCreateGuardShips(AITeam *team, SelectCommand *ships, bool8 wait, bool8 remove)
{
_DWORD *v4; // esi

v4 = memAllocFunction(368, MBF_Volatile);
v4[4] = 1;
*((_BYTE *)v4 + 356) = 0;
*((_BYTE *)v4 + 357) = a3;
*((_BYTE *)v4 + 358) = a4;
v4[90] = 8;
v4[91] = 2;
v4[85] = aimProcessGuardShips;
v4[86] = aimShipDied;
v4[87] = aimCloseGeneric;
aieHandlersClear(v4);
telldir();
v4[5] = a2;
listAddNode((_DWORD *)(a1 + 48), v4, (int)v4);
return v4;
}


#if 0  // Cat2: aimProcessGetShips live code differs from binary or raw IDA offsets
/*=== aimProcessGetShips (0x406fe0) ===*/
int __cdecl aimProcessGetShips(_DWORD *a1)
{
  int v1; // esi
  int v2; // ebp
  int v3; // edi
  char *v4; // eax
  _DWORD *v5; // ebx
  const char *v7; // [esp-14h] [ebp-50h]
  const char *v8; // [esp-14h] [ebp-50h]
  int v9; // [esp-10h] [ebp-4Ch]
  int v10; // [esp-10h] [ebp-4Ch]
  int v11; // [esp+4h] [ebp-38h]
  char v12[48]; // [esp+Ch] [ebp-30h] BYREF

  v1 = a1[15];
  if ( !*(_BYTE *)(v1 + 356) )
  {
    v2 = *(_DWORD *)(v1 + 24);
    v3 = *(_DWORD *)(v1 + 20);
    v4 = aivarLabelGenerate(v12);
    v5 = aivarCreate(v4);
    aivarValueSet(v5, 0);
    *(_DWORD *)(v1 + 28) = v5;
    v11 = aifShipLimitAdjust(aiCurrentAIPlayer, (ShipTypeEnum)v3, v2);
    if ( sub_41A6B0(v3, v2) )
    {
      v9 = *(_DWORD *)(v1 + 32);
      v7 = (const char *)aivarLabelGet((int)v5);
      aifTeamRequestsShipsCB(v3, v2, a1, v7, v9);
    }
    else if ( aiuCanBuildShipType((ShipTypeEnum)v3, *a1 == 3) && (double)v2 * dbl_72A478 < (double)v11 )
    {
      v10 = *(_DWORD *)(v1 + 32);
      v8 = (const char *)aivarLabelGet((int)v5);
      aifTeamRequestsShipsCB(v3, v11, a1, v8, v10);
    }
    *(_BYTE *)(v1 + 356) = 1;
  }
  return aivarValueGet(*(int **)(v1 + 28));
}
#endif
#endif

sdword aimAreTargetsDead(SelectCommand *targets)
{
    CommandToDo *command;
    udword i;

    if (!targets->numShips)
    {
        return TRUE;
    }

    for (i = 0; i < targets->numShips; i++)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer, targets->ShipPtr[i]);
        if ((command != NULL) && (command->ordertype.order == COMMAND_ATTACK))
        {
            return FALSE;
        }
    }

    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x41AB60
// Returns TRUE if any ship in the selection has SOF_DockMarker set (docked).
bool32 aimShipsAtDestination(SelectCommand *ships)
{
    sdword v1 = 0;
    sdword i = 0;

    if (!ships->numShips)
        return FALSE;
    for (; (ships->ShipPtr[i]->flags & SOF_DockMarker) == 0; i++)
    {
        if (++v1 >= ships->numShips)
            return FALSE;
    }
    return TRUE;
}

#if 0  // Cat3: aimSelectionMovedFarEnough
/*=== aimSelectionMovedFarEnough (0x417310) ===*/
BOOL __cdecl aimSelectionMovedFarEnough(float a1, float a2, float a3, float a4, float a5)
{
  float v5; // esi
  signed int *v6; // eax
  float *v7; // eax
  double DistanceSquared; // st7
  float *v9; // ecx
  BOOL result; // eax
  float v11; // [esp-Ch] [ebp-10h]
  float v12; // [esp-8h] [ebp-Ch]
  float v13; // [esp-4h] [ebp-8h]

  v5 = a1;
  v6 = *(signed int **)(LODWORD(a1) + 32);
  result = 1;
  if ( *v6 )
  {
    v11 = a2;
    v12 = a3;
    v13 = a4;
    v7 = selCentrePointComputeGeneral(&a2, v6, &a1);
    DistanceSquared = aiuFindDistanceSquared(*v7, v7[1], v7[2], v11, v12, v13);
    if ( a5 * a5 <= DistanceSquared )
    {
      v9 = *(float **)(*(_DWORD *)(LODWORD(v5) + 32) + 8);
      if ( v9[147] != ZERO || v9[148] != ZERO || v9[149] != ZERO )
        return 0;
    }
  }
  return result;
}
#endif

#if 0  // Cat3: aimIssueAttackCommands
/*=== aimIssueAttackCommands (0x41ED00) ===*/
signed int __cdecl aimIssueAttackCommands(unsigned int *a1, AttackCommand *a2)
{
  unsigned int v2; // edi
  signed int result; // eax
  bool v4; // zf
  int *v5; // esi
  unsigned int v6; // ebp
  int v7; // edi
  signed int v8; // ebp
  bool v9; // cc
  int *v10; // edi
  int v11; // [esp+Ch] [ebp-1F58h]
  int v12[1002]; // [esp+14h] [ebp-1F50h] BYREF
  SelectCommand v13[334]; // [esp+FBCh] [ebp-FA8h] BYREF

  v2 = 0;
  result = 0;
  v4 = *a1 == 0;
  v11 = 0;
  v13[0].numShips = 0;
  if ( !v4 )
  {
    v5 = (int *)(a1 + 2);
    do
    {
      if ( ((unsigned int)&aisTeams[29].selection[3].sel.ShipPtr[555] & *(_DWORD *)(*v5 + 20)) == 0
        && !aiuFilterSelectableShips((Ship *)*v5)
        && (*(_BYTE *)(*v5 + 460) & 8) == 0 )
      {
        selSelectionAddSingleShip(&v13[0].numShips, *v5);
      }
      ++v2;
      ++v5;
    }
    while ( v2 < *a1 );
    if ( !v13[0].numShips )
      return v11;
    v6 = 0;
    if ( a2->numTargets )
    {
      v7 = 8;
      do
      {
        if ( (*(_DWORD *)(*(sdword *)((char *)&a2->numTargets + v7) + 20) & 0x2008000) != 0
          || !sub_52A080((int)aiCurrentAIPlayer->player, *(sdword *)((char *)&a2->numTargets + v7)) )
        {
          *(sdword *)((char *)&a2->numTargets + v7) = *((_DWORD *)&a2->timeLastStatus + a2->numTargets);
          --v6;
          --a2->numTargets;
          v7 -= 4;
        }
        ++v6;
        v7 += 4;
      }
      while ( v6 < a2->numTargets );
    }
    v8 = 0;
    if ( !a2->numTargets )
      return v11;
    v9 = (int)*a1 <= 0;
    v12[0] = 0;
    if ( !v9 )
    {
      v10 = (int *)(a1 + 2);
      do
      {
        if ( *(_DWORD *)(*v10 + 688) == 3 )
        {
          clRemoveTargetFromSelection(&v13[0].numShips, *v10);
          if ( !IsShipInfected(*v10) )
            v12[v12[0]++ + 2] = *v10;
        }
        ++v8;
        ++v10;
      }
      while ( v8 < (int)*a1 );
      if ( v12[0] > 0 )
      {
        clWrapSpecial((int **)&universe, v12, (unsigned int *)a2);
        v11 = 1;
      }
    }
    if ( v13[0].numShips > 0 )
    {
      clWrapAttack(&universe.mainCommandLayer, v13, a2);
      return 1;
    }
    else
    {
      return v11;
    }
  }
  return result;
}
#endif

#if 0  // Cat2: aimIssueMoveCommand live code differs from binary or raw IDA offsets
/*=== aimIssueMoveCommand (0x41EEA0) ===*/
signed int __cdecl aimIssueMoveCommand(unsigned int *a1, int a2)
{
  unsigned int v2; // edi
  bool v3; // zf
  unsigned int *v4; // ebx
  int *v5; // esi
  vector v7; // [esp-18h] [ebp-FD0h]
  vector v8; // [esp-Ch] [ebp-FC4h]
  SelectCommand selectcom; // [esp+10h] [ebp-FA8h] BYREF

  v2 = 0;
  v3 = *a1 == 0;
  selectcom.numShips = 0;
  if ( v3 )
    return 0;
  v4 = a1 + 2;
  v5 = (int *)(a1 + 2);
  do
  {
    if ( ((unsigned int)&aisTeams[29].selection[3].sel.ShipPtr[555] & *(_DWORD *)(*v5 + 20)) == 0
      && !aiuFilterSelectableShips((Ship *)*v5)
      && (*(_BYTE *)(*v5 + 460) & 8) == 0 )
    {
      selSelectionAddSingleShip(&selectcom.numShips, *v5);
    }
    ++v2;
    ++v5;
  }
  while ( v2 < *a1 );
  if ( !selectcom.numShips )
    return 0;
  sub_5126B0(&selectcom.numShips, 0);
  LODWORD(v8.z) = &a2;
  LODWORD(v8.y) = 1;
  *(_QWORD *)&v7.y = *(_QWORD *)(*v4 + 88);
  v8.x = *(real32 *)(*v4 + 96);
  v7.x = 0.0;
  {
    MoveCommandPacked packedMove;

    packedMove.from = v7;
    packedMove.to = v8;
    clWrapMove(&universe.mainCommandLayer, &selectcom, packedMove);
  }
  return 1;
}
#endif

// Cataclysm-specific: halt any cleanup-eligible ships in the selection.
sdword aimCleanupMove(SelectCommand *ships)
{
    MaxSelection selection;
    udword i;

    selection.numShips = 0;
    if (ships->numShips == 0)
    {
        return 0;
    }

    for (i = 0; i < (udword)ships->numShips; i++)
    {
        ShipPtr ship = ships->ShipPtr[i];

        if ((ship->flags & 0x02008000) == 0 &&
            !aiuShipIsntSelectable(ship) &&
            ((ship->specialFlags2 & SPECIAL_2_DockFlag) == 0))
        {
            selSelectionAddSingleShip(&selection, ship);
        }
    }

    if (selection.numShips == 0)
    {
        return 0;
    }

    clWrapHalt(&universe.mainCommandLayer, (SelectCommand *)&selection);
    return 1;
}
