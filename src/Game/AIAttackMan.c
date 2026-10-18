/*=============================================================================
    Name    : AIAttackMan
    Purpose : Attack Manager

    Created 1998/05/28 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "spaceobj.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AITeam.h"
#include "AIOrders.h"
#include "AIMoves.h"
#include "AIHandler.h"
#include "select.h"
#include "stats.h"
#include "AIUtilities.h"
#include "randy.h"
#include "MultiplayerGame.h"

#define MAX_NUM_HARASS_TEAMS    2

/*-----------------------------------------------------------------------------
    Name        : aiaPriorityShipsConstraints
    Description : Decides which ship should be a priority to take out
                  Note: only prioritizes with respect to dangerous combat ships,
                        other risky non-combat ships will be dealt with in different
                        ways.
    Inputs      : shipstatic - the static info of the shiptype to constraint
    Outputs     :
    Return      : TRUE if the ship is one to take out
----------------------------------------------------------------------------*/
static bool32 aiaPriorityShipsConstraints(Ship *ship)
{
    ShipStaticInfo *shipstatic = ship->staticinfo;
    bool32 dangerous = FALSE;

    if (aiuShipIsntAnEnemyMothership(ship))
    {
        switch (shipstatic->shipclass)
        {
            case CLASS_HeavyCruiser:
                //oh yeah... this one's dangerous
            case CLASS_Carrier:
                //take out this one
            case CLASS_Destroyer:
                //dangerous as hell
                dangerous = TRUE;
                break;
            case CLASS_Frigate:
                if (!ShipTypeIsRepairShipType(shipstatic->shiptype) && !ShipTypeIsProcessorType(shipstatic->shiptype) && !ShipTypeIsDefenseFieldFrigateType(shipstatic->shiptype))
                {
                    dangerous = TRUE;
                }
                break;
            case CLASS_Corvette:
            case CLASS_Fighter:
            case CLASS_Resource:
            case CLASS_NonCombat:
            default:
                break;
        }
    }

    if (dangerous)
    {
        if (aitAnyTeamOfPlayerAttackingThisShip(aiCurrentAIPlayer,ship))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}



/*-----------------------------------------------------------------------------
    Name        : aiaSendRecon
    Description : Sends a recon mission to check out a selection of ships
    Inputs      : ships - the ships to check out
    Outputs     : Creates a few new moves for the recon team
    Return      : A pointer to the recon team if it was sent off
----------------------------------------------------------------------------*/
AITeam *aiaSendRecon(SelectCommand *ships)
{
    udword i;
    real32 distsq, min_distsq = REALlyBig, avg_size;
    vector ships_location = selCentrePointComputeGeneral((MaxSelection *)ships, &avg_size), team_location;
    AITeam *team, *best_team = NULL;
    AITeamMove *newMove;

    // find the closest recon team
    for (i=0;i < aiCurrentAIPlayer->numReconTeams;i++)
    {
        team = aiCurrentAIPlayer->reconTeam[i];

        if (team->shipList.selection->numShips)
        {
            team_location = team->shipList.selection->ShipPtr[0]->posinfo.position;
        }
        else
        {
            continue;
        }


        distsq = aiuFindDistanceSquared(ships_location, team_location);

        if (distsq < min_distsq)
        {
            best_team  = team;
            min_distsq = distsq;
        }
    }

    if (min_distsq < REALlyBig)
    {
        newMove = aimCreateShipReconNoAdd(best_team, ships, NO_FORMATION, Evasive, TRUE, TRUE);
        aieHandlerSetTeamDied(newMove, aihReconaissanceTeamDiedHandler);
        aimInsertMove(best_team, newMove);
        return best_team;
    }
    else
    {
        //maybe later check for harass teams as well...
        //or maybe even build a new ship for the task
        return NULL;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aiaArmada
    Description : Finds the best target for armada ships and coordinates
                  armada teams for attack
    Inputs      :
    Outputs     : Lots of blowing up of enemy ships
    Return      : void
----------------------------------------------------------------------------*/
void aiaArmada(void)
{
    vector dest_target;
    udword num_armada = aitFindNumTeamsWithFlag(TEAM_ARMADA);

    //build up armada strength more if enemy player doesn't have many battle
    //ships away from the mothership.  Then do a coordinated attack on mothership.
    //Otherwise, smaller coordinated attacks on enemy buildups (other than the mothership)

    if (num_armada > aiCurrentAIPlayer->aiaArmada.numTeams)
    {
        if (!aiCurrentAIPlayer->aiaArmada.targets.numTargets)
        {
            aiCurrentAIPlayer->aiaArmada.targets.numTargets = 0;
            aiCurrentAIPlayer->aiaArmada.visibility =
                aiuFindArmadaTargetSelection(&dest_target, &aiCurrentAIPlayer->aiaArmada.targets, NULL);
        }
        if (aiCurrentAIPlayer->aiaArmada.targets.numTargets > 0 &&
            !aiCurrentAIPlayer->aiaArmada.visibility &&
            aiCurrentAIPlayer->aiaArmada.recon_team == NULL)
        {
            aiCurrentAIPlayer->aiaArmada.recon_team =
                aiaSendRecon((SelectCommand *)&aiCurrentAIPlayer->aiaArmada.targets);
        }
    }
}




/*-----------------------------------------------------------------------------
    Name        : aiaGetTakeoutTarget
    Description : Looks through EnemyShipsIamAwareOf and finds the most dangerous of them
    Inputs      : none
    Outputs     :
    Return      : The most dangerous enemy ship the computer player has seen
----------------------------------------------------------------------------*/
ShipPtr aiaGetTakeoutTarget(void)
{
    udword i,j=0;
    ShipPtr targetship;
    SelectCommand *temp_sel;
    SelectCommand *target_sel = memAlloc(sizeofSelectCommand(TOTAL_NUM_SHIPS), "tata", Pyrophoric);

    target_sel->numShips = 0;

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        temp_sel = aiCurrentAIPlayer->primaryEnemyShipsIAmAwareOf[i].selection;
        if (temp_sel->numShips)
        {
            selSelectionAddSingleShip((MaxSelection *)target_sel, temp_sel->ShipPtr[0]);
        }
    }

    targetship = statsGetMostDangerousShipNonStatConstraints(target_sel,aiaPriorityShipsConstraints);
    memFree(target_sel);
    return targetship;
}



/*-----------------------------------------------------------------------------
    Name        : aiaGenerateAttackType
    Description : Generates a team with a certain attack type
    Inputs      : newteam - the new attack team
                  attacktype - the type of attack
                  ForceBig - whether or not to force building ships that require a Mothership
    Outputs     : Creates a new team or two
    Return      : void
----------------------------------------------------------------------------*/
bool32 aiaGenerateAttackType(AITeam *newteam, AttackType attacktype, bool32 ForceBig)
{
    ShipPtr ship;
    bool32 return_value = FALSE;

    switch (attacktype)
    {
        case ATTACK_FLEET_FAST:
            if (aiuAttackFeatureEnabled(AIA_ATTACK_FLEET_FAST))
            {
                ship = aiuFindEnemyMothership(aiCurrentAIPlayer->player); //temporary
                if (ship)
                {
                    aioCreateTakeoutMothershipFast(newteam,ship);
                    return_value = TRUE;
                }
            }
            break;
        case ATTACK_FLEET_GUARD:
            if (aiuAttackFeatureEnabled(AIA_ATTACK_FLEET_GUARD))
            {
                ship = aiuFindEnemyMothership(aiCurrentAIPlayer->player); //temporary
                if (ship)
                {
                    aioCreateTakeoutMothershipGuard(newteam,ship);
                    return_value = TRUE;
                }
            }
            break;
        case ATTACK_FLEET_BIG:
            if (aiuAttackFeatureEnabled(AIA_ATTACK_FLEET_BIG))
            {
                ship = aiuFindEnemyMothership(aiCurrentAIPlayer->player); //temporary
                if (ship)
                {
                    aioCreateTakeoutMothershipBig(newteam,ship, ForceBig);
                    return_value = TRUE;
                }
            }
            break;
        case ATTACK_FLEET_HUGE:
            if (aiuAttackFeatureEnabled(AIA_ATTACK_FLEET_HUGE) &&
                ((aiCurrentAIPlayer->player->PlayerMothership &&
                  aiCurrentAIPlayer->player->PlayerMothership->staticinfo->shipclass == CLASS_Mothership) ||
                 ForceBig))
            {
                ship = aiuFindEnemyMothership(aiCurrentAIPlayer->player); //temporary
                if (ship)
                {
                    aioCreateTakeoutMothershipHuge(newteam, ship);
                    return_value = TRUE;
                }
            }
            break;
        case TAKEOUT_TARGET:
            if (aiuAttackFeatureEnabled(AIA_TAKEOUT_TARGET))
            {
                ship = aiaGetTakeoutTarget();
                if (ship)
                {
                    aioCreateTakeoutTarget(newteam, ship);
                    return_value = TRUE;
                }
            }
            break;
        case FANCY_TAKEOUT_TARGET:
            if (aiuAttackFeatureEnabled(AIA_FANCY_TAKEOUT_TARGET))
            {
                ship = aiaGetTakeoutTarget();
                if (ship)
                {
                    aioCreateFancyTakeoutTarget(newteam, ship);
                    return_value = TRUE;
                }
            }
            break;
        case FIGHTER_STRIKE:
            if (aiuAttackFeatureEnabled(AIA_FIGHTER_STRIKE))
            {
                aioCreateFighterStrike(newteam);
                return_value = TRUE;
            }
            break;
        case CORVETTE_STRIKE:
            if (aiuAttackFeatureEnabled(AIA_CORVETTE_STRIKE))
            {
                aioCreateCorvetteStrike(newteam);
                return_value = TRUE;
            }
            break;
        case FRIGATE_STRIKE:
            if (aiuAttackFeatureEnabled(AIA_FRIGATE_STRIKE))
            {
                aioCreateFrigateStrike(newteam);
                return_value = TRUE;
            }
            break;
//later differentiate between harass small/big
        case HARASS_SMALL:
            if (aiuAttackFeatureEnabled(AIA_HARASS))
            {
                aioCreateHarass(newteam);
                return_value = TRUE;
            }
            break;
        case HARASS_BIG:
            if (aiuAttackFeatureEnabled(AIA_HARASS))
            {
                aioCreateHarass(newteam);
                return_value = TRUE;
            }
            break;
        case CAPTURE:
            if (aiuAttackFeatureEnabled(AIA_CAPTURE) && aiuGameTimePassed(600/*AIA_CAPTURE_TIME_DELAY*/))
            {
                aioCreateCapture(newteam);
                return_value = TRUE;
            }
            break;
        case MINE:
            if (aiuAttackFeatureEnabled(AIA_MINE))
            {
                aioCreateMine(newteam);
                return_value = TRUE;
            }
            break;
        case CLOAKGEN:
            if (aiuAttackFeatureEnabled(AIA_CLOAKGEN))
            {
                aioCreateSpecialDefense(newteam, kCloakGenerator);
                return_value = TRUE;
            }
            break;
        case GRAVWELL:
            if (aiuAttackFeatureEnabled(AIA_GRAVWELL))
            {
                aioCreateSpecialDefense(newteam, kGravWellGenerator);
                return_value = TRUE;
            }
            break;
        default:
            aiplayerLog((aiIndex,"Error - Attack Type Not Found"));
    }
    return return_value;
}


/*-----------------------------------------------------------------------------
    Name        : aiaGenerateNewAttackOrder
    Description : Creates a team and generates an order (assumes the ships already exist)
    Inputs      : attack type
    Outputs     : Creates a team and an order
    Return      : TRUE if the team was created
----------------------------------------------------------------------------*/
bool32 aiaGenerateNewAttackOrder(AttackType attacktype)
{
    sdword index = aiCurrentAIPlayer->numAttackTeams;
    aiCurrentAIPlayer->attackTeam[aiCurrentAIPlayer->numAttackTeams++] = aitCreate(AttackTeam);

    if (!aiaGenerateAttackType(aiCurrentAIPlayer->attackTeam[index], attacktype, TRUE))
    {
        aitDestroy(aiCurrentAIPlayer, aiCurrentAIPlayer->attackTeam[index], TRUE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}



/*-----------------------------------------------------------------------------
    Name        : aiaGenerateNewAttackTeam
    Description : Generates a team with the task to attack the enemy in some way
    Inputs      : AttackTeamNumber - the ID number of the attack team
    Outputs     : Creates new teams
    Return      : void
----------------------------------------------------------------------------*/
void aiaGenerateNewAttackTeam(sdword AttackTeamNumber)
{
    AttackType random;
    udword probability_of_attack;
    bool32 attack_type_found = FALSE;
    udword i=0;

    aiCurrentAIPlayer->attackTeam[AttackTeamNumber] = aitCreate(AttackTeam);
    aiCurrentAIPlayer->numAttackTeams++;

    //repeat until an attack team is found
    //upper limit to avoid weird infinite loops
    while ((!attack_type_found) && (i<100))
    {
        random = (AttackType)(ranRandom(RAN_AIPlayer)%NUM_ATTACK_TYPES);
        probability_of_attack = ranRandom(RAN_AIPlayer)&255;

        if (probability_of_attack < aiCurrentAIPlayer->aiaAttackProbability[random])
        {
            attack_type_found = aiaGenerateAttackType(aiCurrentAIPlayer->attackTeam[AttackTeamNumber], random, FALSE);
        }
        else
        {
            i++;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aiaProcessAttackTeams
    Description : Creates and directs the normal attack teams
    Inputs      : None
    Outputs     : Creates and modifies teams
    Return      : void
----------------------------------------------------------------------------*/
void aiaProcessAttackTeams(void)
{
    sdword i;
    bool32 attackTypeFound;
    sdword num_getships;
    udword attackTypeLimit;
    udword attempts;
    AttackType randomAttackType;
    AITeam *team;
    SelectCommand *nearbyShips;
    Ship *enemyMothership;

    //when special attack teams are finished forming, they check with Attackman to
    //find out if they can go ahead and takeout their target.  If not, they become
    //temporary defense teams, then attack when the order is given from the attackman

    for (i=0;i<AIPLAYER_NUM_ATTACKTEAMS;i++)
    {
        attackTypeFound = FALSE;

        if (aiCurrentAIPlayer->attackTeam[i] != NULL)
        {
            if (aitIsTargetAlive(aiCurrentAIPlayer->attackTeam[i]))
            {
                team = aiCurrentAIPlayer->attackTeam[i];
                if (team->shipList.selection->numShips <= 0)
                {
                    aitDestroy(aiCurrentAIPlayer, team, TRUE);
                    aiCurrentAIPlayer->attackTeam[i] = NULL;
                    aiCurrentAIPlayer->numAttackTeams--;
                }
                else
                {
                    nearbyShips = aiuFindNearbyShips(team->shipList.selection->ShipPtr[0], 30000.0f);
                    MakeShipsOnlyFollowConstraints(nearbyShips, aiuShipIsntAnEnemyMothership);
                    if (nearbyShips->numShips)
                    {
                        aitDeleteAllTeamMoves(team);
                        aioCreateTakeoutTargetsWithCurrentTeam(team, nearbyShips);
                    }
                    else
                    {
                        memFree(nearbyShips);
                        enemyMothership = aiuFindEnemyMothership(aiCurrentAIPlayer->player);
                        if (enemyMothership != NULL)
                        {
                            aitDeleteAllTeamMoves(team);
                            aioCreateTakeoutTargetWithCurrentTeam(team, enemyMothership);
                        }
                    }
                }
            }
        }
        else
        {
            num_getships = aitCountTeamsWaitingForShips(AttackTeam);
            if ((num_getships < 4) || ((num_getships < 10) && (aiCurrentAIPlayer->player->resourceUnits > 12000)))
            {
                attempts = 0;
                aiCurrentAIPlayer->attackTeam[i] = aitCreate(AttackTeam);
                aiCurrentAIPlayer->numAttackTeams++;
                attackTypeLimit = singlePlayerGame ? 15 : 11;

                do
                {
                    if (attempts >= 100)
                    {
                        break;
                    }
                    randomAttackType = (AttackType)(ranRandom(RAN_AIPlayer) % attackTypeLimit);
                    if ((ubyte)ranRandom(RAN_AIPlayer) >= aiCurrentAIPlayer->aiaAttackProbability[randomAttackType])
                    {
                        attempts++;
                    }
                    else
                    {
                        attackTypeFound = aiaGenerateAttackType(aiCurrentAIPlayer->attackTeam[i], randomAttackType, FALSE);
                    }
                }
                while (!attackTypeFound);
            }
        }
    }
    aiaArmada();
}



/*-----------------------------------------------------------------------------
    Name        : aiaProcessSpecialTeams
    Description : Creates special teams if they are needed.
                  Special teams take care of themselves and need little
                  directing from the AttackMan
    Inputs      : None
    Outputs     : Creates teams
    Return      : void
----------------------------------------------------------------------------*/
void aiaProcessSpecialTeams(void)
{
    udword harass_probability, i = 0;

    if (!aiCurrentAIPlayer->numReconTeams)
    {
        if (aiuAttackFeatureEnabled(AIA_STATIC_RECONAISSANCE))
        {
            aiCurrentAIPlayer->reconTeam[i] = aitCreate(AttackTeam);
            aioCreateReconaissance(aiCurrentAIPlayer->reconTeam[i], RECON_MOTHERSHIP);
            i++;
        }

        if (aiuAttackFeatureEnabled(AIA_ACTIVE_RECONAISSANCE))
        {
            aiCurrentAIPlayer->reconTeam[i] = aitCreate(AttackTeam);
            aioCreateReconaissance(aiCurrentAIPlayer->reconTeam[i], RECON_ACTIVE_ENEMY);
            i++;
        }
        aiCurrentAIPlayer->numReconTeams = i;
    }

    if ((!aiCurrentAIPlayer->harassTeam) && (aiuAttackFeatureEnabled(AIA_HARASS)))
    {
        //large harass
        harass_probability = ranRandom(RAN_AIPlayer)&255;

        if (harass_probability < aiCurrentAIPlayer->aiaAttackProbability[HARASS_BIG])
        {
            aiCurrentAIPlayer->harassTeam = aitCreate(AttackTeam);
            aioCreateHarass(aiCurrentAIPlayer->harassTeam);
        }

        //large harass
//        harass_probability = ranRandom(RAN_AIPlayer)&255;

//        if (harass_probability < aiCurrentAIPlayer->aiaAttackProbability[HARASS_BIG])
//        {
//            aiCurrentAIPlayer->harassTeam = aitCreate(AttackTeam);
//            aioCreateHarass(aiCurrentAIPlayer->harassTeam);
//        }

    }
}


/*-----------------------------------------------------------------------------
    Name        : aiaTimeAttack
    Description : Times the attack moves so that all big ships go for it at
                  the same time.
                  Each attacking team is waiting on the aivar with the label
                  "attackVarLabel" in the aiplayer structure.  This aivar counts
                  the number of attack teams that are ready.  When the number is
                  at a certain point, the value is set to -1 which is the attack order
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aiaTimeAttack(void)
{
    sdword varValue;
    AITeam *team = NULL;

    switch (aiCurrentAIPlayer->aiplayerDifficultyLevel)
    {
        case AI_BEG:
            aivarValueSet(aivarFind(aiCurrentAIPlayer->attackVarLabel), -1);
            break;
        case AI_INT:
        case AI_ADV:
            if (aiCurrentAIPlayer->airResourceCollectors.selection == NULL)
            {
                team = aitFindNextTeamWithFlag(team, TEAM_TempGuard);
                while (team)
                {
                    aitDeleteMovesUntilMoveType(team, MOVE_TEMPGUARD);
                    team = aitFindNextTeamWithFlag(team, TEAM_TempGuard);
                }

                aivarValueSet(aivarFind(aiCurrentAIPlayer->attackVarLabel), -1);
            }
            else if (aiCurrentAIPlayer->aiaTimeout < universe.totaltimeelapsed)
            {
                aivarValueSet(aivarFind(aiCurrentAIPlayer->attackVarLabel), -1);
                aiCurrentAIPlayer->aiaTimeout += frandyrandombetween(RAN_AIPlayer, 3400.0f, 3800.0f);
            }
            else
            {
                varValue = aivarValueGet(aivarFind(aiCurrentAIPlayer->attackVarLabel));

                if ((varValue < -300) && (aiCurrentAIPlayer->airResourceCollectors.selection != NULL))
                {
                    varValue = randyrandombetween(RAN_AIPlayer, 2, 4);
                    aivarValueSet(aivarFind(aiCurrentAIPlayer->attackVarLabel), varValue);
                }
                else if (varValue < -1)
                {
                    team = aitFindNextTeamWithFlag(team, TEAM_TempGuard);
                    while (team)
                    {
                        aitDeleteMovesUntilMoveType(team, MOVE_TEMPGUARD);
                        team = aitFindNextTeamWithFlag(team, TEAM_TempGuard);
                    }
                    varValue -= 100;
                    aivarValueSet(aivarFind(aiCurrentAIPlayer->attackVarLabel), varValue);
                }
            }
        default:
            break;
    }


}


/*-----------------------------------------------------------------------------
    Name        : aiaProcessHarassTeams
    Description : Directs harass teams - first gets all of them into a list
                  then checks to see if they are in a mothership blob.  If so,
                  pulls them away.
    Inputs      :
    Outputs     : Modifies team moves
    Return      : void
----------------------------------------------------------------------------*/
void aiaProcessHarassTeams(void)
{
    AITeam *harassTeams[MAX_NUM_HARASS_TEAMS], *team;
    AITeamMove *move;
    udword i, team_num = 0, numAttTeams = aiCurrentAIPlayer->numAttackTeams, retreat_probability;
    ShipPtr teamShip;
    SelectCommand *shipList;
    vector destination;

    //put all harass teams into an array
    if (aiCurrentAIPlayer->harassTeam)
    {
        harassTeams[team_num] = aiCurrentAIPlayer->harassTeam;
        team_num++;
    }

    for (i=0;i<numAttTeams;i++)
    {
        team = aiCurrentAIPlayer->attackTeam[i];

        if (bitTest(team->teamFlags, HARASS_TEAM))
        {
            harassTeams[team_num] = team;
            team_num++;
        }
    }

    //for each harass team, check how close it is to the enemy mothership
    //if it's close, move it away - later: ignore if attacking research ships
    for (i=0;i<team_num;i++)
    {
        shipList = harassTeams[i]->shipList.selection;

        if ((shipList) && (shipList->numShips) &&
            (!bitTest(harassTeams[i]->teamFlags, TEAM_RETREATING)) &&
            (aiuShipsCloseToEnemyMothership(aiCurrentAIPlayer->player, shipList, 5500)))
        {
            retreat_probability = ranRandom(RAN_AIPlayer)&255;

            if (retreat_probability < 200)
            {
                teamShip = shipList->ShipPtr[0];

                //find a move location away from the mothership
                destination = aiuFindSafestStandoffPoint(shipList, 20000);

                //delete attack move if there is one
                //clear moves until "harass attack"
                aitDeleteMovesUntilMoveType(harassTeams[i], MOVE_HARASSATTACK);

                move = aimCreateMoveTeamNoAdd(harassTeams[i], destination, SAME_FORMATION, TRUE, FALSE);
                move->tactics = Evasive;    //temporary

                aimInsertMove(harassTeams[i], move);

                bitSet(harassTeams[i]->teamFlags, TEAM_RETREATING);
            }
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aiaProcessReconTeams
    Description : Creates and directs reconaissance teams
    Inputs      : nothing
    Outputs     : Creates and modifies teams
    Return      : void
----------------------------------------------------------------------------*/
void aiaProcessReconTeams(void)
{
    udword numEnemyBlobs = aiuGetNumEnemyBlobs();

    if ((!aiCurrentAIPlayer->numReconTeams) ||
        ((aiCurrentAIPlayer->numReconTeams < AIPLAYER_NUM_RECONTEAMS) &&
         (numEnemyBlobs/aiCurrentAIPlayer->numReconTeams > AIA_ENEMYBLOBS_PER_RECON)))
    {
        aiCurrentAIPlayer->reconTeam[aiCurrentAIPlayer->numReconTeams] = aitCreate(AttackTeam);
        aioCreateReconaissance(aiCurrentAIPlayer->reconTeam[aiCurrentAIPlayer->numReconTeams], RECON_ACTIVE_ENEMY);
        aiCurrentAIPlayer->numReconTeams++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aiaDivideNewShips
    Description : Divides the new ships available into nice neat and tidy teams
    Inputs      :
    Outputs     : Creates some orders
    Return      : void
----------------------------------------------------------------------------*/
bool32 aiaDivideNewShips(void)
{
    SelectCommand *NewShips = aiCurrentAIPlayer->newships.selection;
    udword numShipType[TOTAL_NUM_SHIPS];
    udword i;
    bool32 newteam = FALSE;

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        numShipType[i] = 0;
    }

    for (i=0;i<NewShips->numShips;i++)
    {
        numShipType[NewShips->ShipPtr[i]->shiptype]++;
    }

    if ((numShipType[sDreadnought] && numShipType[sDestroyer]) ||
        (numShipType[bHeavyCruiser] && numShipType[bIonArrayFrigate]))
    {
        return aiaGenerateNewAttackOrder(ATTACK_FLEET_HUGE);
    }

    if ((numShipType[sMultiBeamFrigate] && numShipType[sRammingFrigate]) ||
        (numShipType[sHiveFrigate] && numShipType[sRammingFrigate]))
    {
        return aiaGenerateNewAttackOrder(ATTACK_FLEET_BIG);
    }

    if (singlePlayerGame)
    {
        if (numShipType[tHeavyCruiser] && numShipType[tMissileDestroyer])
        {
            return aiaGenerateNewAttackOrder(ATTACK_FLEET_HUGE);
        }

        if (numShipType[tStandardDestroyer] && (numShipType[tStandardFrigate] > 1))
        {
            return aiaGenerateNewAttackOrder(ATTACK_FLEET_BIG);
        }

        if (numShipType[kGravWellGenerator] || numShipType[tGravWellGenerator])
        {
            return aiaGenerateNewAttackOrder(GRAVWELL);
        }

        if (numShipType[kCloakGenerator] || numShipType[tCloakGenerator])
        {
            return aiaGenerateNewAttackOrder(CLOAKGEN);
        }
    }

    return newteam;
}



/*-----------------------------------------------------------------------------
    Name        : aiaCleanupTeams
    Description : Cleans up the messiness of the teams
    Inputs      :
    Outputs     : Deletes a few teams sometimes
    Return      : void
----------------------------------------------------------------------------*/
void aiaCleanupTeams(void)
{
    AITeam *team = NULL;
    udword i;

    for (i=0;i<aiCurrentAIPlayer->numAttackTeams;)
    {
        team = aiCurrentAIPlayer->attackTeam[i];

        if (aitTeamIsDone(team) && (!team->shipList.selection->numShips))
        {
//            for (j=0;j<team->shipList.selection->numShips;j++)
//            {
//                growSelectAddShip(&aiCurrentAIPlayer->newships, team->shipList.selection->ShipPtr[j]);
//            }

            aitDestroy(aiCurrentAIPlayer, team, TRUE);

//            aiCurrentAIPlayer->numAttackTeams--;
//            aiCurrentAIPlayer->attackTeam[i] = aiCurrentAIPlayer->attackTeam[aiCurrentAIPlayer->numAttackTeams];
//            aiCurrentAIPlayer->attackTeam[aiCurrentAIPlayer->numAttackTeams] = NULL;
            continue;
        }
        //later figure out if we need to do something if the team still has ships
        i++;
    }
}



/*-----------------------------------------------------------------------------
    Name        : aiaAttackManager
    Description : Deals with all attack related tasks of the computer player
    Inputs      : None
    Outputs     : Lotsa stuff
    Return      : void
----------------------------------------------------------------------------*/
void aiaAttackManager(void)
{
    bool32 newteams = FALSE;
    udword i;
    AITeam *team;

    if (aiCurrentAIPlayer->numAttackTeams)
    {
        for (i = 0; i < aiCurrentAIPlayer->numAttackTeams; i++)
        {
            team = aiCurrentAIPlayer->attackTeam[i];
            if (team != NULL)
            {
                if (aitIsTargetAlive(team) && !team->shipList.selection->numShips)
                {
                    aitDestroy(aiCurrentAIPlayer, team, TRUE);
                }
            }
        }
    }

    if (aiCurrentAIPlayer->newships.selection->numShips)
    {
        newteams = aiaDivideNewShips();
    }

    if (aiuAttackFeatureEnabled(AIA_ACTIVE_RECONAISSANCE))
    {
        aiaProcessReconTeams();
    }

    if (aiuAttackFeatureEnabled(AIA_HARASS))
    {
//        aiaProcessHarassTeams();
    }

    if (aiCurrentAIPlayer->AttackFeatures && !newteams) aiaProcessAttackTeams();

    aiaTimeAttack();
}



/*=============================================================================
    P2 Specific Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aiaProcessMultiBeamFrigates
    Description : Controls the attack pattern of the multi beam frigates
    Inputs      :
    Outputs     : Creates a few teams
    Return      : void
----------------------------------------------------------------------------*/
void aiaProcessFrigates(void)
{
    //does nothing for now
}


/*-----------------------------------------------------------------------------
    Name        : aiaCleanUpSwarmers
    Description : Cleans up the swarm teams: deletes groups that have no ships or only pods left.  Puts the pods back into the newships selection
    Inputs      : nah-zink
    Outputs     : deleting of teams and such
    Return      : void
----------------------------------------------------------------------------*/
void aiaCleanUpSwarmers(void)
{
    udword i;
    AITeam *team;

    for (i=0;i<aiCurrentAIPlayer->numSupportTeams;i++)
    {
        team = aiCurrentAIPlayer->supportTeam[i];

//        if (team->teamType == ResourceTeam)
        {
            //if the attack team and the defense team are dead
            if ((!team->cooperatingTeam->shipList.selection->numShips) &&
                 (!team->cooperatingTeam->cooperatingTeam->shipList.selection->numShips))
            {
                //move the pod (if it still exists) back to the newships selection
                while (team->shipList.selection->numShips)
                {
                    growSelectAddShip(&aiCurrentAIPlayer->newships, team->shipList.selection->ShipPtr[0]);
                    growSelectRemoveShip(&team->shipList, team->shipList.selection->ShipPtr[0]);
                }
                aitDestroy(aiCurrentAIPlayer, team->cooperatingTeam->cooperatingTeam, TRUE);
                aitDestroy(aiCurrentAIPlayer, team->cooperatingTeam, TRUE);
                aitDestroy(aiCurrentAIPlayer, team, TRUE);
            }
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aiaProcessSwarm
    Description : Controls the attacks of the P2 swarmers
    Inputs      : None
    Outputs     : Creates a bunch of teams
    Return      : void
----------------------------------------------------------------------------*/
void aiaProcessSwarm(void)
{
    bitClear(aiCurrentAIPlayer->AlertStatus, ALERT_SWARMER_TARGETS);
}



/*-----------------------------------------------------------------------------
    Name        : aiaP2AttackManager
    Description : Deals with all attack related tasks of the P2 Pirates
    Inputs      : None
    Outputs     : A whole whack of things
    Return      : void
----------------------------------------------------------------------------*/
void aiaP2AttackManager(void)
{
    aiaProcessSwarm();

    aiaProcessFrigates();
}



/*=============================================================================
    Non AI Related Stuff:
=============================================================================*/
//temporary bug fixin'
bool32 aiaPlayerCanBuildShipType(ShipType shiptype, AIPlayer *aiplayer)
{
    ShipStaticInfo *teststatic;
    ShipRace race;

    race = aiplayer->player->race;

    if ((RacesAllowedForGivenShip[shiptype] & RaceToRaceBits(race)) == 0)
    {
        // Can't build this shiptype for this race
        return FALSE;
    }

    teststatic = GetShipStaticInfo(shiptype,race);
    if (!bitTest(teststatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        return FALSE;       // this ship static info not loaded, so can't build
    }

    if (ShipTypeIsWorker(shiptype) && (!singlePlayerGame) && (!bitTest(tpGameCreated.flag,MG_HarvestinEnabled)))
    {
        return FALSE;
    }

    return TRUE;     // later set to TRUE to take into account research
}






void aiaTeamDied(struct AIPlayer *aiplayer,struct AITeam *team)
{
    if (team == aiplayer->harassTeam)
    {
        aiplayer->harassTeam = NULL;
    }
    //temporary... later we'll have a bigger array
    else if (team == aiplayer->reconTeam[0])
    {
        aiplayer->reconTeam[0] = NULL;
    }
    else if (team == aiplayer->reconTeam[1])
    {
        aiplayer->reconTeam[1] = NULL;
    }
    else if (team == aiplayer->aiaArmada.recon_team)
    {
        aiplayer->aiaArmada.recon_team = NULL;
    }
    else
    {
        sdword i;

        for (i=0;i<AIPLAYER_NUM_ATTACKTEAMS;i++)
        {
            if (aiplayer->attackTeam[i] == team)
            {
                aiplayer->numAttackTeams--;
                aiplayer->attackTeam[i] = aiplayer->attackTeam[aiplayer->numAttackTeams];
                aiplayer->attackTeam[aiplayer->numAttackTeams] = NULL;
                break;
            }
        }
    }
}

bool32 aiaShipDied(struct AIPlayer *aiplayer, ShipPtr ship)
{
#if 0
    if (ship->playerowner == aiplayer->player)
    {
        if (growSelectRemoveShip(&aiplayer->newattackships, ship))
        {
            return TRUE;
        }
    }
#endif

    if (aiplayer->aiaArmada.targets.numTargets > 0)
    {
        clRemoveTargetFromSelection((SelectAnyCommand *)&aiplayer->aiaArmada.targets, (TargetPtr)ship);
    }

    if ((aiplayer->Targets) && (aiplayer->Targets->numShips > 0) && (clRemoveTargetFromSelection((SelectAnyCommand *)aiplayer->Targets, (TargetPtr)ship)))
    {
        aiplayerLog((aiplayer->player->playerIndex, "Ship removed from Swarm Targets"));

        if (!aiplayer->Targets->numShips)
        {
            aiumemFree(aiplayer->Targets);
        }
    }
    return FALSE;
}

void aiaInit(struct AIPlayer *aiplayer)
{
    sdword i;

//    growSelectInit(&aiplayer->newattackships);

    for (i=0;i<AIPLAYER_NUM_RECONTEAMS;i++)
    {
        aiplayer->reconTeam[i] = NULL;
    }
    aiplayer->numReconTeams = 0;

    aiplayer->harassTeam = NULL;

    for (i=0;i<AIPLAYER_NUM_ATTACKTEAMS;i++)
    {
        aiplayer->attackTeam[i] = NULL;
    }
    aiplayer->numAttackTeams = 0;

    aiplayer->haveAttackedMothership = 0;
    aivarLabelGenerate(aiplayer->attackVarLabel);
    aivarValueSet(aivarCreate(aiplayer->attackVarLabel), randyrandombetween(RAN_AIPlayer, 2, 4));
	aiplayer->aiaTimeout = frandyrandombetween(RAN_AIPlayer, 2900.0f, 3400.0f);

    //init attack type array
    aiplayer->aiaAttackProbability[ATTACK_FLEET_FAST]    = aiuRandomRange(AIA_ATTACK_FLEET_FAST_PROB[aiplayer->aiplayerDifficultyLevel], AIA_ATTACK_FLEET_FAST_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[ATTACK_FLEET_GUARD]   = aiuRandomRange(AIA_ATTACK_FLEET_GUARD_PROB[aiplayer->aiplayerDifficultyLevel], AIA_ATTACK_FLEET_GUARD_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[ATTACK_FLEET_BIG]     = aiuRandomRange(AIA_ATTACK_FLEET_BIG_PROB[aiplayer->aiplayerDifficultyLevel], AIA_ATTACK_FLEET_BIG_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[ATTACK_FLEET_HUGE]    = aiuRandomRange(AIA_ATTACK_FLEET_HUGE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_ATTACK_FLEET_HUGE_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[TAKEOUT_TARGET]       = aiuRandomRange(AIA_TAKEOUT_TARGET_PROB[aiplayer->aiplayerDifficultyLevel], AIA_TAKEOUT_TARGET_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[FANCY_TAKEOUT_TARGET] = aiuRandomRange(AIA_FANCY_TAKEOUT_TARGET_PROB[aiplayer->aiplayerDifficultyLevel], AIA_FANCY_TAKEOUT_TARGET_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[FIGHTER_STRIKE]       = aiuRandomRange(AIA_FIGHTER_STRIKE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_FIGHTER_STRIKE_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[CORVETTE_STRIKE]      = aiuRandomRange(AIA_CORVETTE_STRIKE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_CORVETTE_STRIKE_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[FRIGATE_STRIKE]       = aiuRandomRange(AIA_FRIGATE_STRIKE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_FRIGATE_STRIKE_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[HARASS_BIG]           = aiuRandomRange(AIA_HARASS_BIG_PROB[aiplayer->aiplayerDifficultyLevel], AIA_HARASS_BIG_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[HARASS_SMALL]         = aiuRandomRange(AIA_HARASS_SMALL_PROB[aiplayer->aiplayerDifficultyLevel], AIA_HARASS_SMALL_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[CAPTURE]              = aiuRandomRange(AIA_CAPTURE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_CAPTURE_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[MINE]                 = aiuRandomRange(AIA_MINE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_MINE_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[CLOAKGEN]             = aiuRandomRange(AIA_CLOAKGEN_PROB[aiplayer->aiplayerDifficultyLevel], AIA_CLOAKGEN_RANGE[aiplayer->aiplayerDifficultyLevel]);
    aiplayer->aiaAttackProbability[GRAVWELL]             = aiuRandomRange(AIA_GRAVWELL_PROB[aiplayer->aiplayerDifficultyLevel], AIA_GRAVWELL_RANGE[aiplayer->aiplayerDifficultyLevel]);

    if ((!singlePlayerGame) && bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
    {
        aiplayer->aiaAttackProbability[CAPTURE] = aiuRandomRange(180, 25);
    }

    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Attack Fleet Fast    = %i", aiplayer->aiaAttackProbability[ATTACK_FLEET_FAST]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Attack Fleet Guard   = %i", aiplayer->aiaAttackProbability[ATTACK_FLEET_GUARD]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Attack Fleet Big     = %i", aiplayer->aiaAttackProbability[ATTACK_FLEET_BIG]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Attack Fleet Huge    = %i", aiplayer->aiaAttackProbability[ATTACK_FLEET_HUGE]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Takeout Target       = %i", aiplayer->aiaAttackProbability[TAKEOUT_TARGET]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Fancy Takeout Target = %i", aiplayer->aiaAttackProbability[FANCY_TAKEOUT_TARGET]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Fighter Strike       = %i", aiplayer->aiaAttackProbability[FIGHTER_STRIKE]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Corvette Strike      = %i", aiplayer->aiaAttackProbability[CORVETTE_STRIKE]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Frigate Strike       = %i", aiplayer->aiaAttackProbability[FRIGATE_STRIKE]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Harass Big           = %i", aiplayer->aiaAttackProbability[HARASS_BIG]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Harass Small         = %i", aiplayer->aiaAttackProbability[HARASS_SMALL]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Capture              = %i", aiplayer->aiaAttackProbability[CAPTURE]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability Mine                 = %i", aiplayer->aiaAttackProbability[MINE]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability CloakGen             = %i", aiplayer->aiaAttackProbability[CLOAKGEN]));
    aiplayerLog((aiplayer->player->playerIndex, "Attack Probability GravWell             = %i\n", aiplayer->aiaAttackProbability[GRAVWELL]));

    aiplayer->aiaArmada.numTeams = AIA_NUM_ARMADA_TEAMS[aiplayer->aiplayerDifficultyLevel];
    memset(&aiplayer->aiaArmada.targets, 0, sizeof(aiplayer->aiaArmada.targets));
    aiplayer->aiaArmada.visibility = FALSE;
    aiplayer->aiaArmada.recon_team = NULL;

    aiplayer->Targets = NULL;

    if ((!singlePlayerGame) && (bitTest(tpGameCreated.flag,MG_CaptureCapitalShip)))
    {
        aiplayer->aiaAttackProbability[CAPTURE] += 120;
    }

    switch (aiplayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            aiuEnableAttackFeature(AIA_ACTIVE_RECONAISSANCE);

            if (aiaPlayerCanBuildShipType(kHeavyCruiser, aiplayer))
            {
                aiuEnableAttackFeature(AIA_ATTACK_FLEET_HUGE);
            }

            aiuEnableAttackFeature(AIA_FANCY_TAKEOUT_TARGET);
            aiuEnableAttackFeature(AIA_FIGHTER_STRIKE);
            aiuEnableAttackFeature(AIA_CORVETTE_STRIKE);
            aiuEnableAttackFeature(AIA_HARASS);

            if (aiaPlayerCanBuildShipType(kSalCapCorvette, aiplayer))
            {
                aiuEnableAttackFeature(AIA_CAPTURE);
            }
            if (aiaPlayerCanBuildShipType(tMinelayerCorvette, aiplayer))
            {
                aiuEnableAttackFeature(AIA_MINE);
            }
            if (aiaPlayerCanBuildShipType(kCloakGenerator, aiplayer))
            {
                aiuEnableAttackFeature(AIA_CLOAKGEN);
            }
            if (aiaPlayerCanBuildShipType(kGravWellGenerator, aiplayer))
            {
                aiuEnableAttackFeature(AIA_GRAVWELL);
            }
        case AI_INT:
            aiuEnableAttackFeature(AIA_STATIC_RECONAISSANCE);
            aiuEnableAttackFeature(AIA_ATTACK_FLEET_FAST);
            aiuEnableAttackFeature(AIA_TAKEOUT_TARGET);
            aiuEnableAttackFeature(AIA_FRIGATE_STRIKE);
        case AI_BEG:
            aiuEnableAttackFeature(AIA_ATTACK_FLEET_BIG);
            aiuEnableAttackFeature(AIA_ATTACK_FLEET_GUARD);

            if ((!singlePlayerGame) && bitTest(tpGameCreated.flag,MG_CaptureCapitalShip))
            {
                aiuEnableAttackFeature(AIA_CAPTURE);
            }
            break;
        default:
            dbgAssert(FALSE);
    }

    if (randyrandombetween(RAN_AIPlayer, 0, 100) < aiuRandomRange(AIA_KAMIKAZE_PROB[aiplayer->aiplayerDifficultyLevel], AIA_KAMIKAZE_RANGE[aiplayer->aiplayerDifficultyLevel]))
    {
        aiuEnableAttackFeature(AIA_KAMIKAZE);
    }
}

void aiaClose(struct AIPlayer *aiplayer)
{
    sdword i;

//    growSelectClose(&aiplayer->newattackships);
    for (i=0;i<AIPLAYER_NUM_RECONTEAMS;i++)
    {
        if (aiplayer->reconTeam[i] != NULL)
        {
            aitDestroy(aiplayer, aiplayer->reconTeam[i],FALSE);
            aiplayer->reconTeam[i] = NULL;
        }
    }
    if (aiplayer->harassTeam != NULL)
    {
        aitDestroy(aiplayer, aiplayer->harassTeam,FALSE);
        aiplayer->harassTeam = NULL;
    }

    for (i=0;i<AIPLAYER_NUM_ATTACKTEAMS;i++)
    {
        if (aiplayer->attackTeam[i] != NULL)
        {
            aitDestroy(aiplayer, aiplayer->attackTeam[i],FALSE);
            aiplayer->attackTeam[i] = NULL;
        }
    }

    aiumemFree(aiplayer->Targets);
}


#if 0
/*-----------------------------------------------------------------------------
    Name        : aiaAddNewShip
    Description : Adds new ship to attack manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aiaAddNewShip(Ship *ship)
{
    growSelectAddShip(&aiCurrentAIPlayer->newattackships, ship);
}
#endif



#if 0  // Cat2: aiaProcessAttackTeams/aiaTimeAttack live code differs from binary (raw struct offsets)
{
   int i; // ebx
   signed int v1; // ebp
   int v2; // eax
   unsigned int v3; // esi
   int v4; // edi
   int v5; // eax
   int *v6; // ecx
   int *NearbyShips; // esi
   int EnemyMothership; // esi
   unsigned int v10; // [esp+10h] [ebp-4h]

   for ( i = 3100; i < 3196; i += 4 )
   {
     v1 = 0;
     if ( *(Player **)((char *)&aiCurrentAIPlayer->player + i) )
     {
       if ( aitIsTargetAlive(*(int *)((char *)&aiCurrentAIPlayer->player + i)) )
       {
         v5 = *(int *)((char *)&aiCurrentAIPlayer->player + i);
         v6 = *(int **)(v5 + 32);
         if ( *v6 <= 0 )
         {
           aitDestroy(aiCurrentAIPlayer, v5, 1);
           *(Player **)((char *)&aiCurrentAIPlayer->player + i) = 0;
           --aiCurrentAIPlayer[159].DefenseFeatures;
         }
         else
         {
           NearbyShips = aiuFindNearbyShips(v6[2], 30000.0);
           MakeShipsOnlyFollowConstraints(NearbyShips, sub_41A9F0);
           if ( *NearbyShips )
           {
             aitDeleteAllTeamMoves(*(int *)((char *)&aiCurrentAIPlayer->player + i));
             sub_410670(*(int *)((char *)&aiCurrentAIPlayer->player + i), (int)NearbyShips);
           }
           else
           {
             memFree((ubyte *)NearbyShips);
             EnemyMothership = aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
             if ( EnemyMothership )
             {
               aitDeleteAllTeamMoves(*(int *)((char *)&aiCurrentAIPlayer->player + i));
               sub_4106B0(*(int *)((char *)&aiCurrentAIPlayer->player + i), EnemyMothership);
             }
           }
         }
       }
     }
     else
     {
       v2 = sub_4171F0(0);
       if ( v2 < 4 || v2 < 10 && aiCurrentAIPlayer->player->resourceUnits > 12000 )
       {
         v10 = 0;
         *(Player **)((char *)&aiCurrentAIPlayer->player + i) = (Player *)aitCreate(0);
         ++aiCurrentAIPlayer[159].DefenseFeatures;
         v3 = singlePlayerGame != 0 ? 15 : 11;
         do
         {
           if ( v10 >= 0x64 )
             break;
           v4 = ranRandomFn(6) % v3;
           if ( (unsigned __int8)ranRandomFn(6) >= *(&aiCurrentAIPlayer[162].AttackFeatures + v4) )
             ++v10;
           else
             v1 = sub_4012A0(*(int **)((char *)&aiCurrentAIPlayer->player + i), v4, 0);
         }
         while ( !v1 );
       }
     }
   }
   return sub_401120();
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4019B0
int aiaTimeAttack(void)
{
   int result; // eax
   int j; // esi
   _DWORD *v2; // eax
   float *v3; // esi
   int *v4; // eax
   int v5; // edi
   _DWORD *v6; // eax
   int i; // esi
   int v8; // [esp-4h] [ebp-14h]

   result = aiCurrentAIPlayer->aiplayerDifficultyLevel;
   if ( !result )
     goto LABEL_17;
   if ( result > 0 && result <= 2 )
   {
     if ( aiCurrentAIPlayer[149].DefenseFeatures )
     {
       if ( *(float *)&aiCurrentAIPlayer[367].player < (double)universe.totaltimeelapsed )
       {
         v2 = (_DWORD *)aivarFind((char *)&aiCurrentAIPlayer[160].aiplayerDifficultyLevel);
         aivarValueSet(v2, -1);
         v3 = (float *)&aiCurrentAIPlayer[367];
         result = ranRandomFn(6);
         *v3 = (double)(unsigned int)result * flt_72A43C + flt_72A438 + *v3;
         return result;
       }
       v4 = (int *)aivarFind((char *)&aiCurrentAIPlayer[160].aiplayerDifficultyLevel);
       result = aivarValueGet(v4);
       v5 = result;
       if ( result < -300 && (result = aiCurrentAIPlayer[149].DefenseFeatures) != 0 )
       {
         v8 = ranRandomFn(6) % 3 + 2;
         v6 = (_DWORD *)aivarFind((char *)&aiCurrentAIPlayer[160].aiplayerDifficultyLevel);
       }
       else
       {
         if ( v5 >= -1 )
           return result;
         for ( i = sub_417180(0, 0x40000); i; i = sub_417180(i, 0x40000) )
           sub_416970(i, 3);
         v8 = v5 - 100;
         v6 = (_DWORD *)aivarFind((char *)&aiCurrentAIPlayer[160].aiplayerDifficultyLevel);
       }
       return (int)aivarValueSet(v6, v8);
     }
     for ( j = sub_417180(0, 0x40000); j; j = sub_417180(j, 0x40000) )
       sub_416970(j, 3);
 LABEL_17:
     v8 = -1;
     v6 = (_DWORD *)aivarFind((char *)&aiCurrentAIPlayer[160].aiplayerDifficultyLevel);
     return (int)aivarValueSet(v6, v8);
   }
   return result;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x401B50
signed int aiaDivideNewShips(void)
{
   AIPlayer *v0; // esi
   Player *player; // ebx
   int v2; // edx
   udword *p_unk; // ecx
   Ship *PlayerMothership; // edi
   int v5; // eax
   udword DefenseFeatures; // esi
   signed int v8; // eax
   int v9[145]; // [esp+Ch] [ebp-244h] BYREF

   v0 = aiCurrentAIPlayer;
   player = aiCurrentAIPlayer[2].player;
   memset(v9, 0, sizeof(v9));
   v2 = 0;
   if ( player->PlayerMothership )
   {
     p_unk = &player->unk;
     PlayerMothership = player->PlayerMothership;
     do
     {
       v5 = *(_DWORD *)(*p_unk++ + 688);
       PlayerMothership = (Ship *)((char *)PlayerMothership - 1);
       ++v9[v5];
     }
     while ( PlayerMothership );
   }
   if ( v9[10] && v9[25] )
     goto LABEL_22;
   if ( v9[7] && v9[6] || v9[8] && v9[6] )
     goto LABEL_23;
   if ( v9[46] && v9[44] )
   {
 LABEL_22:
     DefenseFeatures = v0[159].DefenseFeatures;
     *((_DWORD *)&aiCurrentAIPlayer[155].player + aiCurrentAIPlayer[159].DefenseFeatures++) = aitCreate(0);
     v8 = sub_4012A0(*((int **)&aiCurrentAIPlayer[155].player + DefenseFeatures), 3, 1);
     goto LABEL_26;
   }
   if ( singlePlayerGame )
   {
     if ( !v9[80] || !v9[85] )
     {
       if ( !v9[90] || v9[91] <= 1u )
       {
         if ( v9[64] || v9[79] )
         {
           DefenseFeatures = v0[159].DefenseFeatures;
           *((_DWORD *)&aiCurrentAIPlayer[155].player + aiCurrentAIPlayer[159].DefenseFeatures++) = aitCreate(0);
           v8 = sub_4012A0(*((int **)&aiCurrentAIPlayer[155].player + DefenseFeatures), 14, 1);
         }
         else
         {
           if ( !v9[63] )
           {
             if ( !v9[77] )
               return 0;
             DefenseFeatures = v0[159].DefenseFeatures;
             *((_DWORD *)&aiCurrentAIPlayer[155].player + aiCurrentAIPlayer[159].DefenseFeatures++) = aitCreate(0);
             if ( !sub_4012A0(*((int **)&aiCurrentAIPlayer[155].player + DefenseFeatures), 13, 1) )
               goto LABEL_29;
             return 1;
           }
           DefenseFeatures = v0[159].DefenseFeatures;
           *((_DWORD *)&aiCurrentAIPlayer[155].player + aiCurrentAIPlayer[159].DefenseFeatures++) = aitCreate(0);
           v8 = sub_4012A0(*((int **)&aiCurrentAIPlayer[155].player + DefenseFeatures), 13, 1);
         }
 LABEL_26:
         if ( !v8 )
         {
 LABEL_29:
           aitDestroy(aiCurrentAIPlayer, *((_DWORD *)&aiCurrentAIPlayer[155].player + DefenseFeatures), 1);
           return 0;
         }
         return 1;
       }
 LABEL_23:
       DefenseFeatures = v0[159].DefenseFeatures;
       *((_DWORD *)&aiCurrentAIPlayer[155].player + aiCurrentAIPlayer[159].DefenseFeatures++) = aitCreate(0);
       v8 = sub_4012A0(*((int **)&aiCurrentAIPlayer[155].player + DefenseFeatures), 2, 1);
       goto LABEL_26;
     }
     goto LABEL_22;
   }
   return v2;
 }
#endif

#if 0  // Cat2: aiaSendRecon live code differs from binary (different handlers, different logic)
struct AITeam *__cdecl aiaSendRecon(SelectCommand *ships)
{
vector *v1; // eax
real32 x; // ecx
real32 y; // edx
AIPlayer *v4; // eax
udword DefenseFeatures; // ecx
struct AITeam *v6; // edi
udword v7; // ebp
int v8; // ebx
int v9; // esi
_DWORD *v10; // eax
double DistanceSquared; // st7
struct AITeamMove *ShipReconNoAdd; // esi
float v14; // [esp+10h] [ebp-20h]
float v15; // [esp+14h] [ebp-1Ch] BYREF
vector v16; // [esp+18h] [ebp-18h]
vector point2; // [esp+24h] [ebp-Ch] BYREF
v14 = 1.0e21;
v1 = selCentrePointComputeGeneral(&point2, (MaxSelection *)ships, &v15);
x = v1->x;
y = v1->y;
v16.z = v1->z;
v4 = aiCurrentAIPlayer;
v16.x = x;
DefenseFeatures = aiCurrentAIPlayer[154].DefenseFeatures;
v6 = 0;
v7 = 0;
v16.y = y;
if ( !DefenseFeatures )
return 0;
v8 = 3076;
do
{
v9 = *(int *)((char *)&v4->player + v8);
v10 = *(_DWORD **)(v9 + 32);
if ( *v10 )
{
DistanceSquared = aiuFindDistanceSquared(v16, *(vector *)(v10[2] + 88));
if ( DistanceSquared < v14 )
{
v14 = DistanceSquared;
v6 = (struct AITeam *)v9;
}
}
v4 = aiCurrentAIPlayer;
++v7;
v8 += 4;
}
while ( v7 < aiCurrentAIPlayer[154].DefenseFeatures );
if ( v14 >= (double)flt_72A430 )
return 0;
ShipReconNoAdd = aimCreateShipReconNoAdd(v6, ships, 7u, Evasive, 1, 1);
aieHandlerSetTeamDied(ShipReconNoAdd, aihReconaissanceTeamDiedHandler);
aimInsertMove(v6, ShipReconNoAdd);
return v6;
}
#endif


#if 0  // Cat2: aiaArmada live code differs from binary
void __cdecl aiaArmada()
{
udword NumTeamsWithFlag; // eax
AIPlayer *v1; // ecx
udword DefenseFeatures_low; // edx
vector dest_target; // [esp+0h] [ebp-Ch] BYREF
NumTeamsWithFlag = aitFindNumTeamsWithFlag(0x10000u);
v1 = aiCurrentAIPlayer;
DefenseFeatures_low = LOBYTE(aiCurrentAIPlayer[165].DefenseFeatures);
if ( NumTeamsWithFlag > DefenseFeatures_low )
{
if ( aiCurrentAIPlayer[166].player )
{
if ( NumTeamsWithFlag <= DefenseFeatures_low )
return;
}
else
{
aiCurrentAIPlayer[166].player = 0;
aiCurrentAIPlayer[366].ResourceFeatures = aiuFindArmadaTarget(
&dest_target,
(SelectCommand **)&aiCurrentAIPlayer[166]);
v1 = aiCurrentAIPlayer;
}
if ( (int)v1[166].player > 0 && !v1[366].ResourceFeatures && !v1[366].AttackFeatures )
aiCurrentAIPlayer[366].AttackFeatures = (udword)aiaSendRecon((SelectCommand *)&v1[166]);
}
}
#endif


#if 0  // Cat2: aiaPriorityShipsConstraints live code differs from binary (missing CLASS_MonsterShip, different frigate check)
bool32 __cdecl aiaPriorityShipsConstraints(Ship *ship)
{
ShipStaticInfo2 *staticinfo; // esi
staticinfo = ship->staticinfo;
if ( aiuShipIsntAnEnemyMothership(ship) )
{
switch ( staticinfo->shipclass )
{
case CLASS_HeavyCruiser:
case CLASS_Carrier:
case CLASS_Destroyer:
case CLASS_MonsterShip:
return aitAnyTeamOfPlayerAttackingThisShip(aiCurrentAIPlayer, ship) == 0;
case CLASS_Frigate:
switch ( staticinfo->shiptype )
{
case sProcessor:
case bDFGFrigate:
case bProcessor:
case kAdvanceSupportFrigate:
case tAdvanceSupportFrigate:
case tDFGFrigate:
return 0;
default:
return aitAnyTeamOfPlayerAttackingThisShip(aiCurrentAIPlayer, ship) == 0;
}
default:
return 0;
}
}
return 0;
}
#endif


#if 0  // Cat2: aiaGenerateAttackType live code differs from binary
bool32 __cdecl aiaGenerateAttackType(struct AITeam *newteam, int attacktype, bool32 ForceBig)
{
bool32 v3; // ebx
Ship *EnemyMothership; // eax
Ship *v6; // eax
Ship *v7; // eax
Ship *PlayerMothership; // eax
Ship *v9; // eax
MaxSelection *v10; // edi
unsigned int i; // esi
int v12; // eax
Ship *MostDangerousShipStaticConstraints; // esi
MaxSelection *v14; // edi
unsigned int j; // esi
int v16; // eax
Ship *v17; // esi
ShipRaceEnum race; // eax
ShipRaceEnum v19; // eax
int v20; // [esp-4h] [ebp-10h]
v3 = 0;
switch ( attacktype )
{
case 0:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x10) == 0 )
return v3;
EnemyMothership = (Ship *)aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
if ( !EnemyMothership )
return v3;
aioCreateTakeoutMothershipFast(newteam, EnemyMothership);
return 1;
case 1:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x20) == 0 )
return v3;
v6 = (Ship *)aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
if ( !v6 )
return v3;
aioCreateTakeoutMothershipGuard(newteam, v6);
return 1;
case 2:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x40) == 0 )
return v3;
v7 = (Ship *)aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
if ( !v7 )
return v3;
aioCreateTakeoutMothershipBig(newteam, v7, ForceBig);
return 1;
case 3:
if ( SLOBYTE(aiCurrentAIPlayer->AttackFeatures) >= 0 )
return v3;
PlayerMothership = aiCurrentAIPlayer->player->PlayerMothership;
if ( (!PlayerMothership || PlayerMothership->staticinfo->shipclass) && !ForceBig )
return v3;
v9 = (Ship *)aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
if ( !v9 )
return v3;
aioCreateTakeoutMothershipHuge(newteam, v9);
return 1;
case 4:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x100) == 0 )
return v3;
v10 = (MaxSelection *)memAllocFunction(588, MBF_SmallBlockHeap);
v10->numShips = 0;
for ( i = 1208; i < 0x940; i += 8 )
{
v12 = *(int *)((char *)&aiCurrentAIPlayer->player + i);
if ( *(_DWORD *)v12 )
selSelectionAddSingleShip(v10, *(Ship **)(v12 + 8));
}
MostDangerousShipStaticConstraints = (Ship *)statsGetMostDangerousShipStaticConstraints(
&v10->numShips,
(int (__cdecl *)(int))aiaPriorityShipsConstraints);
memFree((ubyte *)v10);
if ( !MostDangerousShipStaticConstraints )
return v3;
aioCreateTakeoutTarget(newteam, MostDangerousShipStaticConstraints);
return 1;
case 5:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x200) == 0 )
return v3;
v14 = (MaxSelection *)memAllocFunction(588, MBF_SmallBlockHeap);
v14->numShips = 0;
for ( j = 1208; j < 0x940; j += 8 )
{
v16 = *(int *)((char *)&aiCurrentAIPlayer->player + j);
if ( *(_DWORD *)v16 )
selSelectionAddSingleShip(v14, *(Ship **)(v16 + 8));
}
v17 = (Ship *)statsGetMostDangerousShipStaticConstraints(
&v14->numShips,
(int (__cdecl *)(int))aiaPriorityShipsConstraints);
memFree((ubyte *)v14);
if ( !v17 )
return v3;
aioCreateFancyTakeoutTarget(newteam, v17);
return 1;
case 6:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x1000) == 0 )
return v3;
aioCreateFighterStrike(newteam);
return 1;
case 7:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x2000) == 0 )
return v3;
aioCreateCorvetteStrike(newteam);
return 1;
case 8:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x4000) == 0 )
return v3;
aioCreateFrigateStrike(newteam);
return 1;
case 9:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x10000) == 0 )
return v3;
goto LABEL_38;
case 10:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x10000) == 0 )
return v3;
LABEL_38:
aioCreateHarass(newteam);
return 1;
case 11:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x20000) == 0 || universe.totaltimeelapsed <= (double)flt_72A434 )
return v3;
aioCreateCapture(newteam);
return 1;
case 12:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x40000) == 0 )
return v3;
aioCreateMine(newteam);
return 1;
case 13:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x100000) == 0 )
return v3;
race = aiCurrentAIPlayer->player->race;
if ( race == 2 )
{
aioCreateSpecialDefense(newteam, 63);
return 1;
}
if ( race != 3 )
return 1;
v20 = 77;
goto LABEL_56;
case 14:
if ( (aiCurrentAIPlayer->AttackFeatures & 0x200000) == 0 )
return v3;
v19 = aiCurrentAIPlayer->player->race;
if ( v19 == 2 )
{
aioCreateSpecialDefense(newteam, 64);
return 1;
}
else
{
if ( v19 == 3 )
{
v20 = 79;
LABEL_56:
aioCreateSpecialDefense(newteam, v20);
}
return 1;
}
default:
return v3;
}
}
#endif
