/*=============================================================================
    Name    : AIDefenseMan
    Purpose : Defense Manager

    Created 1998/05/28 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIHandler.h"
#include "AITeam.h"
#include "AIOrders.h"
#include "AIUtilities.h"
#include "select.h"
#include "stats.h"
#include "AIMoves.h"
#include "fastmath.h"
#include "randy.h"

bool32 aitAnyTeamOfPlayerGuardingThisShip(struct AIPlayer *aiplayer,Ship *ship)
{
    sdword i;
    Node *node;
    AITeamMove *move;
    AITeam *team;

    for (i = 0; i < aiplayer->teamsUsed; ++i)
    {
        team = aiplayer->teams[i];
        node = team->moves.head;
        while (node != NULL)
        {
            move = (AITeamMove *)listGetStructOfNode(node);

            if (move->type == MOVE_GUARDSHIPS)
            {
                if (move->params.guardShips.ships)
                {
                    if (ShipInSelection(move->params.guardShips.ships,ship))
                    {
                        return TRUE;
                    }
                }
            }
            node = node->next;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : aidSendDistressSignal
    Description : Adds ships to the list of ships in distress
    Inputs      : ships - the ships in distress
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidSendDistressSignal(SelectCommand *ships)
{
    aiCurrentAIPlayer->aidDefenseTargets = selectMergeTwoSelections(aiCurrentAIPlayer->aidDefenseTargets, ships, DEALLOC1);
    aiCurrentAIPlayer->aidDistressShips  = selectMergeTwoSelections(aiCurrentAIPlayer->aidDistressShips, ships, DEALLOC1);
}


/*-----------------------------------------------------------------------------
    Name        : aidSendInvasionSignal
    Description : Adds ships to the list of ships invading the computer player's space
    Inputs      : ships - the ships invading
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidSendInvasionSignal(SelectCommand *ships)
{
    aiCurrentAIPlayer->aidDefenseTargets = selectMergeTwoSelections(aiCurrentAIPlayer->aidDefenseTargets, ships, DEALLOC1);
    aiCurrentAIPlayer->aidInvadingShips  = selectMergeTwoSelections(aiCurrentAIPlayer->aidInvadingShips, ships, DEALLOC1);
}


/*-----------------------------------------------------------------------------
    Name        : aidClearDistressSignal
    Description : Clears the shipsindistress variable (note - name has changed
                  to aidDefenseTargets
    Inputs      : aiplayer - the player to do the clearing for
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidClearDistressSignal(AIPlayer *aiplayer)
{
    if (aiplayer->aidDefenseTargets)
    {
        memFree(aiplayer->aidDefenseTargets);
        aiplayer->aidDefenseTargets = NULL;
    }
    if (aiplayer->aidInvadingShips)
    {
        memFree(aiplayer->aidInvadingShips);
        aiplayer->aidInvadingShips = NULL;
    }
    if (aiplayer->aidDistressShips)
    {
        memFree(aiplayer->aidDistressShips);
        aiplayer->aidDistressShips = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : aidCountDefendableShips
    Description : Counts the number of unarmed or lightly armed ships in the computer player's fleet
    Inputs      :
    Outputs     :
    Return      : The number of defendable ships
----------------------------------------------------------------------------*/
udword aidCountDefendableShips(void)
{
    udword i, numdefships = 0;
    ShipPtr ship;

    for (i=0;i<aiCurrentAIPlayer->airResourceReserves.selection->numShips;i++)
    {
        ship = aiCurrentAIPlayer->airResourceReserves.selection->ShipPtr[i];
        if (ShipTypeIsWorker(ship->shiptype) || ShipTypeIsRepairShipType(ship->shiptype)
            || (ship->shiptype == kSalCapCorvette))
        {
            numdefships++;
        }
        else if (ShipTypeIsProcessorType(ship->shiptype) ||
                 (ship->shiptype == kAdvanceSupportFrigate) ||
                 ShipTypeIsCarrierType(ship->shiptype))
        {
            //these ships count as two
            numdefships += AID_RESCON_ASF_DEFENDABLE_MULTIPLIER;
        }
    }
    return numdefships;
}



/*-----------------------------------------------------------------------------
    Name        : aidCleanupUnusedTeams
    Description : Puts any ships from teams that don't do anything into the
                  combat reserves
    Inputs      : None
    Outputs     : See "description"
    Return      : void
----------------------------------------------------------------------------*/
void aidCleanupUnusedTeams(void)
{
    sdword i;

    for (i=0;i<aiCurrentAIPlayer->numGuardTeams;)
    {
        if (aitIsTargetAlive(aiCurrentAIPlayer->guardTeams[i]))
        {
            // return any members of team to reserves:
            AITeam *team = aiCurrentAIPlayer->guardTeams[i];
            sdword j;

            for (j=0;j<team->shipList.selection->numShips;j++)
            {
                growSelectAddShip(&aiCurrentAIPlayer->newships, team->shipList.selection->ShipPtr[j]);
            }

            // delete this team
            aitDestroy(aiCurrentAIPlayer,aiCurrentAIPlayer->guardTeams[i],TRUE);

//            aiCurrentAIPlayer->numGuardTeams--;
//            aiCurrentAIPlayer->guardTeams[i] = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams];
//            aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = NULL;
            continue;       // check same index again
        }
        i++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : aidCheckSphereOfInfluence
    Description : Checks the Mothership's sphere of influence for invasions
                  and KILLS THEM
                  Note that sphere of influence measures in distance squared for
                  efficiency.
    Inputs      : None
    Outputs     : Gets the defense teams to annihilate anything that crosses
                  the sphere of influence line`
    Return      : void
----------------------------------------------------------------------------*/
void aidCheckSphereOfInfluence(void)
{
    ShipPtr enemy_mothership;
    real32 distsq, spherewithextra;
    vector mothership_pos;
    SelectCommand *enemy_invaders;
    ShipPtr mothership = aiCurrentAIPlayer->player->PlayerMothership;
    udword i;

    if (!mothership)
    {
        return;
    }
    mothership_pos = mothership->posinfo.position;

    //calculates sphere of influence if there isn't one,
    //or every 15 seconds
    if ((!aiCurrentAIPlayer->sphereofinfluence) ||
        (!(((udword)universe.totaltimeelapsed) & AID_SPHERE_OF_INFLUENCE_INTERVAL)))
    {
        enemy_mothership = aiuFindEnemyMothership(aiCurrentAIPlayer->player);
        if (enemy_mothership == NULL)
        {
            return;
        }
        distsq = aiuFindDistanceSquared(enemy_mothership->posinfo.position, mothership_pos);
        aiCurrentAIPlayer->sphereofinfluence = distsq/4;
    }

    if (aiuDefenseFeatureEnabled(AID_SPHERE_OF_INFLUENCE_INVADERS))
    {
        enemy_invaders = aiuFindLeadShipInSphereOfInfluence(mothership_pos, aiCurrentAIPlayer->sphereofinfluence);

        //if there invaders
        if ((enemy_invaders))
        {
            aidSendInvasionSignal(enemy_invaders);
            memFree(enemy_invaders);
        }
    }

    //check for guard teams outside of the sphere of influence and
    //recall them
    //later have a chance of no recall depending on "personality"
    spherewithextra = aiCurrentAIPlayer->sphereofinfluence * 1.4;
    for (i=0;i<aiCurrentAIPlayer->numGuardTeams;i++)
    {
        if (aitNumTeamShips(aiCurrentAIPlayer->guardTeams[i]))
        {
            distsq = aiuFindDistanceSquared(mothership_pos, aitApproxTeamPos(aiCurrentAIPlayer->guardTeams[i]));

            if (distsq > spherewithextra)
            {
                aitRecallGuardTeam(aiCurrentAIPlayer->guardTeams[i]);
            }
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aidSetupResourceDefenseTeams
    Description : Sets up defenses for resource collectors
    Inputs      : None
    Outputs     : Creates new teams if needed
    Return      : void
----------------------------------------------------------------------------*/
void aidSetupResourceDefenseTeams(void)
{
    ShipPtr ship;
    sdword i;
    SelectCommand *selectone;

    //if the difficulty level is high enough, when a certain number of resource
    //defense teams have been setup, they change to patrolling defense teams that
    //actively defend ships rather than passively guard them
    for (i=0;i<aiCurrentAIPlayer->airResourceReserves.selection->numShips;i++)
    {
        ship = aiCurrentAIPlayer->airResourceReserves.selection->ShipPtr[i];
        if (ShipTypeIsWorker(ship->shiptype) ||
            ShipTypeIsProcessorType(ship->shiptype) ||
            (ship->shiptype == kAdvanceSupportFrigate) ||
            (ship->shiptype == tAdvanceSupportFrigate) ||
            (ship->shiptype == tResourceCollector))
        {
            if (!aitAnyTeamOfPlayerGuardingThisShip(aiCurrentAIPlayer,ship))
            {
                if (aiCurrentAIPlayer->numGuardTeams < AIPLAYER_MAX_NUM_GUARDTEAMS)
                {
                    selectone = memAlloc(sizeofSelectCommand(1),"guardreser",0);
                    selectone->numShips = 1;
                    selectone->ShipPtr[0] = ship;

                    // create new defense team to guard ship
                    aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = aitCreate(DefenseTeam);
                    aioCreateGuardShips(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams],selectone);
                    aiCurrentAIPlayer->numGuardTeams++;
                }
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aidCreateFastGuardTeam
    Description : Creates an active guard team consisting of fast ships (fighters)
    Inputs      : none
    Outputs     : Creates a new team
    Return      : void
----------------------------------------------------------------------------*/
void aidCreateFastGuardTeam(void)
{
    aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = aitCreate(DefenseTeam);

    //identify team as fast roving team
    bitSet(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]->teamFlags, FAST_ROVING_GUARD);
    aioCreateFastRovingDefense(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]);
    aiCurrentAIPlayer->numGuardTeams++;
}


/*-----------------------------------------------------------------------------
    Name        : aidCreateSlowGuardTeam
    Description : Creates an active guard team consisting of slow ships (corvettes
                  and frigates)
    Inputs      : none
    Outputs     : Creates a new team
    Return      : void
----------------------------------------------------------------------------*/
void aidCreateSlowGuardTeam(void)
{
    aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams] = aitCreate(DefenseTeam);

    //identify team as slow roving team
    bitSet(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]->teamFlags, SLOW_ROVING_GUARD);
    aioCreateSlowRovingDefense(aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams]);
    aiCurrentAIPlayer->numGuardTeams++;
}


/*-----------------------------------------------------------------------------
    Name        : aidSetupRovingDefenseTeams
    Description : Sets up and manages the roving defense teams
    Inputs      :
    Outputs     : Creates new teams if needed
    Return      : void
----------------------------------------------------------------------------*/
void aidSetupRovingDefenseTeams(void)
{
    udword numDefendableShips = aidCountDefendableShips();

    if (aiCurrentAIPlayer->numGuardTeams >= AID_ROVING_DEFENSE_LOW_COUNT_CUTOFF)
    {
        if (numDefendableShips/AID_ROVING_DEFENSE_LOW_COUNT_DIVISOR > aiCurrentAIPlayer->numGuardTeams)
        {
            if (num_is_odd(aiCurrentAIPlayer->numGuardTeams))
            {
                aidCreateSlowGuardTeam();
            }
            else
            {
                aidCreateFastGuardTeam();
            }
        }
    }
    else if (numDefendableShips/AID_ROVING_DEFENSE_HIGH_COUNT_DIVISOR > aiCurrentAIPlayer->numGuardTeams)
    {
        if (num_is_odd(aiCurrentAIPlayer->numGuardTeams))
        {
            aidCreateSlowGuardTeam();
        }
        else
        {
            aidCreateFastGuardTeam();
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : aidPositionProximitySensorRoving
    Description : Hooks up a proximity sensor to a moving team
    Inputs      : Index of the proximity sensor (i.e. location in the aiCurrentAIPlayer->aidProximitySensors array
    Outputs     : Does that guard command thing with the proximity sensor
    Return      : void
----------------------------------------------------------------------------*/
void aidPositionProximitySensorRoving(udword index)
{
    AITeam *team;
    SelectCommand selone;

    selone.numShips   = 1;
    selone.ShipPtr[0] = aiCurrentAIPlayer->aidProximitySensors->ShipPtr[index];

    if (index > 1)
    {
        team = aitFindMostValuable(index-1);
    }
    else
    {
        team = aitFindMostValuable(index);
    }

    aiuWrapProtect(&selone, team->shipList.selection);
}


/*-----------------------------------------------------------------------------
    Name        : aidPositionProximitySensorGuard
    Description : Positions the proximity sensor(s) in a guard position close to the mothership
    Inputs      : index - the index of the proximity sensor (i.e. location in the aiCurrentAIPlayer->aidProximitySensors structure
    Outputs     : Moves some ships around
    Return      : void
----------------------------------------------------------------------------*/
void aidPositionProximitySensorGuard(udword index)
{
    vector moveto0, moveto2;
    vector originvect = {0,0,0};
    SelectCommand selone;

    if ((!vecAreEqual(aiCurrentAIPlayer->aidProximitySensors->ShipPtr[0]->moveTo, originvect))||(index==2))
    {
        //the ship for index 2 is moved when index 0 is moved
        //ships are only moved when they've done moving
        return;
    }

    selone.numShips   = 1;
    selone.ShipPtr[0] = aiCurrentAIPlayer->aidProximitySensors->ShipPtr[index];

    if (index == 0)
    {
        moveto0 = aiuGenerateRandomStandoffPoint(aiuPlayerMothershipCoords(aiCurrentAIPlayer->player), 6000, originvect, RSP_NEAR);
        aiuWrapMove(&selone, moveto0);
    }

    if (aiCurrentAIPlayer->aidProximitySensors->numShips > 2)
    {
        selone.ShipPtr[0] = aiCurrentAIPlayer->aidProximitySensors->ShipPtr[2];

        moveto2 = aiuGenerateFlankCoordinates(aiuPlayerMothershipCoords(aiCurrentAIPlayer->player), moveto0, originvect, 6000);
        aiuWrapMove(&selone, moveto2);
    }


}




/*-----------------------------------------------------------------------------
    Name        : aidCloakerDefense
    Description : Looks for cloakers, and reacts to their presence in a
                  paranoid delusional way
                  note: due to the way the build queues work, it is simpler
                        to have the proximity sensors be moved and positioned
                        by ResourceMan, hence the ResMan callback instead of
                        defenseman callback.
    Inputs      : Nahtink
    Outputs     : Creates new teams if needed
    Return      : void
----------------------------------------------------------------------------*/
void aidCloakDefense(void)
{
    udword num_sensors, i;

    if (aiuFindCloakersInEnemyShipsIAmAwareOf(aiuDefenseFeatureEnabled(AID_CLOAK_DEFENSE_RED)))
    {
        if (bitTest(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_RED))
        {
            num_sensors = randyrandombetween(RAN_AIPlayer, 3, 6);

            if (aiCurrentAIPlayer->NumProxSensorsRequested < num_sensors)
            {
                if (aiCurrentAIPlayer->aidProximitySensors &&
                    (aiCurrentAIPlayer->aidProximitySensors->numShips < 6))
                {
                    aifResourceManRequestsShipsCB(kProximitySensor, num_sensors - aiCurrentAIPlayer->NumProxSensorsRequested, 0);
                    aiCurrentAIPlayer->NumProxSensorsRequested = num_sensors;
                }
            }
            //make sure we have a few proximity sensors
            //send them to where the cloaked ships were sighted
        }
        else if (bitTest(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_YELLOW))
        {
            if (!aiCurrentAIPlayer->NumProxSensorsRequested)
            {
                if (aiCurrentAIPlayer->aidProximitySensors &&
                    (aiCurrentAIPlayer->aidProximitySensors->numShips < 6))
                {
                    aiCurrentAIPlayer->NumProxSensorsRequested = randyrandombetween(RAN_AIPlayer, 1,3);
                    aifResourceManRequestsShipsCB(kProximitySensor, aiCurrentAIPlayer->NumProxSensorsRequested, 0);
                }
            }
            //make sure we have 1 or 2 proximity sensors
            //position them in guard positions - 1 close to mothership,
            //another roving with roving guard
        }
    }
    else
    {
        if ((!(((udword)universe.totaltimeelapsed) & AID_SPHERE_OF_INFLUENCE_INTERVAL)) &&
            (aiCurrentAIPlayer->aidProximitySensors) &&
            ((aiCurrentAIPlayer->NumProxSensorsRequested > aiCurrentAIPlayer->aidProximitySensors->numShips)) &&
            (aiCurrentAIPlayer->aidProximitySensors->numShips < 3))
        {
            aifResourceManRequestsShipsCB(kProximitySensor, (aiCurrentAIPlayer->NumProxSensorsRequested - aiCurrentAIPlayer->aidProximitySensors->numShips), 0);
        }

        //reset Red alert status every every 15 seconds to check if it's still valid
        if ((bitTest(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_RED)) &&
            (!(((udword)universe.totaltimeelapsed) & AID_SPHERE_OF_INFLUENCE_INTERVAL)))
        {
            bitClear(aiCurrentAIPlayer->AlertStatus, ALERT_CLOAK_RED);
        }
    }

    if (aiCurrentAIPlayer->aidProximitySensors)
    {
        for (i=0;i < aiCurrentAIPlayer->aidProximitySensors->numShips;i++)
        {
            //if i is odd or larger than three
            if ((i > 3) || (i & 0x1))
            {
                aidPositionProximitySensorRoving(i);
            }
            else
            {
                aidPositionProximitySensorGuard(i);
            }
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aidAddSlackerShips
    Description : Gets ships that are idle to defend the mothership
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
SelectCommand *aidAddSlackerShips(void)
{
    udword i;
    AITeam *slackteam;
    struct AITeamMove *newMove;
    SelectCommand *slackerships = NULL;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        slackteam = aiCurrentAIPlayer->teams[i];

        //if team isn't doing anything, and the team has ships and
        //the first ship is dangerous (presumably the rest of the
        //team is dangerous if the first ship is
        if ((slackteam->teamType != ScriptTeam) &&
            (aitTeamIsIdle(slackteam)) &&
            (slackteam->shipList.selection->numShips) &&
            (aiuIsShipDangerous(slackteam->shipList.selection->ShipPtr[0])))
        {
            newMove = aimCreateDefendMothershipNoAdd(slackteam, TRUE, TRUE);
            aimInsertMove(slackteam, newMove);

            slackerships = selectMergeTwoSelections(slackerships, aiCurrentAIPlayer->teams[i]->shipList.selection, DEALLOC1);
        }
    }
    return slackerships;
}


/*-----------------------------------------------------------------------------
    Name        : aidAddGuardingShips
    Description : Finds teams that are guarding and adds them to the mothership defense
    Inputs      :
    Outputs     : Inserts a move into the guarding team's movelist
    Return      : The selection of ships that were added
----------------------------------------------------------------------------*/
SelectCommand *aidAddGuardingShips(void)
{
    udword i;
    AITeam *guardteam;
    SelectCommand *guardships = NULL;
    AITeamMove *newMove;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        guardteam = aiCurrentAIPlayer->teams[i];

        if ((guardteam->teamType != ScriptTeam) &&
            (aitTeamIsGuarding(guardteam)) &&
            (guardteam->shipList.selection->numShips) &&
            (aiuIsShipDangerous(guardteam->shipList.selection->ShipPtr[0])))
        {
            newMove = aimCreateDefendMothershipNoAdd(guardteam, TRUE, TRUE);
            aimInsertMove(guardteam, newMove);

            guardships = selectMergeTwoSelections(guardships, guardteam->shipList.selection, DEALLOC1);
        }
    }
    return guardships;
}


/*-----------------------------------------------------------------------------
    Name        : aidAddAllShips
    Description : Finds all teams and adds them to the mothership defense
    Inputs      : enemyships - the ships attacking the mothership
    Outputs     : Inserts a move into the guarding team's movelist
    Return      : The selection of ships that were added
----------------------------------------------------------------------------*/
SelectCommand *aidAddAllShips(SelectCommand *enemyships)
{
    udword i;
    AITeam *team;
    SelectCommand *newships = NULL;
    AITeamMove *newMove;

    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; i++)
    {
        team = aiCurrentAIPlayer->teams[i];

        if ((team->teamType != ScriptTeam) &&
            (team->shipList.selection->numShips) &&
            (team->shipList.selection->ShipPtr[0]->staticinfo->shipclass != CLASS_Mothership) &&
            (team->shipList.selection->ShipPtr[0]->staticinfo->shipclass != CLASS_Carrier) &&
            (aiuIsShipDangerous(team->shipList.selection->ShipPtr[0])) &&
            (aitTeamIsntDefendingMothership(team, enemyships)) &&
            (aitTeamIsInMothershipRange(team)))
        {
            //add a defend mothership move
            newMove = aimCreateDefendMothershipNoAdd(team, TRUE, TRUE);
            aimInsertMove(team, newMove);

            newships = selectMergeTwoSelections(newships, team->shipList.selection, DEALLOC1);
        }
    }
    return newships;
}



/*-----------------------------------------------------------------------------
    Name        : aidMothershipDefense
    Description : Checks to see if the mothership is being attacked.
                  If it is, ships around the mothership are given defend
                  mothership orders
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidMothershipDefense(void)
{
    SelectCommand *enemyships = NULL, *newships = NULL, *goodguyships = NULL, *combatreserves;
    ShipPtr Ship;
    udword i, newteam;
    AITeam *mdteam = NULL;
    ShipPtr mothership = aiCurrentAIPlayer->player->PlayerMothership;

    enemyships = aiuEnemyShipsInMothershipBlob();

    if ((!enemyships) && (mothership) && (mothership->gettingrocked))
    {
        enemyships = aiuFindNearbyEnemyShips(mothership->gettingrocked, AID_MOTHERSHIP_ATTACKER_FLEET_RADIUS);
        selSelectionAddSingleShip((MaxSelection *)enemyships, mothership->gettingrocked);
    }

    if ((enemyships) &&
        (!((enemyships->numShips == 1) && (isShipOfClass(enemyships->ShipPtr[0], CLASS_Fighter)))))
    {
        aiumemFree(aiCurrentAIPlayer->shipsattackingmothership);
        aiCurrentAIPlayer->shipsattackingmothership = selectMemDupSelection(enemyships, "msdp", Pyrophoric);
        goodguyships = (SelectCommand *)memAlloc(sizeofSelectCommand(aiCurrentAIPlayer->newships.selection->numShips), "moshipdefships", 0);
        goodguyships->numShips = 0;

        //if there are ships in the combat reserves
        if (aiCurrentAIPlayer->newships.selection->numShips)
        {
            combatreserves = aiCurrentAIPlayer->newships.selection;

            //find all reserve ships and add them to the new defense team
            for (i = 0; i < combatreserves->numShips; i++)
            {
                //only select dangerous ships, not puny girly ships
                if (aiuIsShipDangerous(combatreserves->ShipPtr[i]))
                {
                    //if a mothership defense team doesn't exist yet
                    //later maybe have more than one mothership defense team
                    if (aiCurrentAIPlayer->mothershipdefteam == -1)
                    {
                        //create a mothership defense team
                        newteam = aiCurrentAIPlayer->numGuardTeams;
                        aiCurrentAIPlayer->guardTeams[newteam] = aitCreate(DefenseTeam);
                        mdteam = aiCurrentAIPlayer->guardTeams[newteam];
                        aioCreateDefendMothership(mdteam);

                        aiCurrentAIPlayer->mothershipdefteam = newteam;
                        aiCurrentAIPlayer->numGuardTeams++;
                    }
                    else
                    {
                        mdteam = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->mothershipdefteam];
                    }

                    Ship = combatreserves->ShipPtr[i];
                    growSelectRemoveShip(&aiCurrentAIPlayer->newships, Ship);
                    combatreserves = aiCurrentAIPlayer->newships.selection;
                    aitAddShip(mdteam, Ship);
                }
            }
        }

        if (mdteam)
        {
            dbgAssert(mdteam->shipList.selection->numShips);
        }

        newships = aidAddSlackerShips();

        goodguyships = selectMergeTwoSelections(newships, goodguyships, DEALLOC_BOTH);

        aiuMakeShipsOnlyDangerousToMothership(enemyships);

        if (enemyships->numShips > 0)
        {
            //if there are no defending ships or the defending ships are weaker than the attacking ships
            if (aiuDefenseFeatureEnabled(AID_MOTHERSHIP_DEFENSE_MEDIUM) &&
                ((!goodguyships->numShips) ||
                 (statsGetRelativeFleetStrengths(enemyships, goodguyships) > AID_MOTHERSHIP_DEFENSE_FLEET_STRENGTH)))
            {
                //get all ships that are currently guarding to join in the fray
                newships = aidAddGuardingShips();
                goodguyships = selectMergeTwoSelections(newships, goodguyships, DEALLOC_BOTH);

                //if there are still no defending ships or the defending ships are still weaker than the attacking ships
                if (aiuDefenseFeatureEnabled(AID_MOTHERSHIP_DEFENSE_HARDCORE) &&
                    ((!goodguyships->numShips) ||
                     (statsGetRelativeFleetStrengths(enemyships, goodguyships) > AID_MOTHERSHIP_DEFENSE_FLEET_STRENGTH)))
                {
                    //get every available ship to join in the fray
                    newships = aidAddAllShips(enemyships);
                    goodguyships = selectMergeTwoSelections(newships, goodguyships, DEALLOC_BOTH);
                }
            }
        }
    }
    else
    {
        if (enemyships &&
            (enemyships->numShips == 1) &&
            (isShipOfClass(enemyships->ShipPtr[0], CLASS_Fighter)))
        {
            ranRandom(RAN_AIPlayer);
        }

        aiumemFree(aiCurrentAIPlayer->shipsattackingmothership);

        if (aiCurrentAIPlayer->mothershipdefteam != -1)
        {
            mdteam = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->mothershipdefteam];

            for (i=0;i<mdteam->shipList.selection->numShips;i++)
            {
                growSelectAddShip(&aiCurrentAIPlayer->newships, mdteam->shipList.selection->ShipPtr[i]);
            }

            aitDestroy(aiCurrentAIPlayer,mdteam,TRUE);

            if (aiCurrentAIPlayer->mothershipdefteam != aiCurrentAIPlayer->numGuardTeams)
            {
                aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->mothershipdefteam] = aiCurrentAIPlayer->guardTeams[aiCurrentAIPlayer->numGuardTeams];
            }
            aiCurrentAIPlayer->mothershipdefteam = -1;
        }
    }

    aiumemFree(enemyships);
    aiumemFree(goodguyships);

}




/*-----------------------------------------------------------------------------
    Name        : aidDefenseManager
    Description : Manages defense of the computer player's fleet.
    Inputs      : None
    Outputs     : Lots of intricate calculations
    Return      : void
----------------------------------------------------------------------------*/
void aidDefenseManager(void)
{
    //different levels of defense:
    //1. defend individual resource collectors and support ships
    //2. when enough ships have been built to guard, change to an active defense
    //   which keeps ships out of the computer player's sphere of influence
    //3. If sphere of influence broken, a large enough fleet of ships is sent to
    //   intercept it.
    //4. Second sphere of influence is immediate mothership area - already dealt with
    //
    //Note: sphere of influence does not include blobs created by harass ships or ships
    //      in a frontal assault.

    aidCleanupUnusedTeams();

    if (aiuDefenseFeatureEnabled(AID_GUARDING))
    {
        if (aiuDefenseFeatureEnabled(AID_ACTIVE_GUARD))
        {
            aidSetupRovingDefenseTeams();
        }
        else
        {
            aidSetupResourceDefenseTeams();
        }
    }

    aidCheckSphereOfInfluence();

    if (aiuDefenseFeatureEnabled(AID_CLOAK_DEFENSE))
    {
        aidCloakDefense();
    }

    //sets up defense teams for support ships (ASF, Repair Corvette, etc.)
    //aidSetupSupportDefenseTeams();

    //sets up defense for mothership - takes priority over all if deemed necessary
    if (aiuDefenseFeatureEnabled(AID_MOTHERSHIP_DEFENSE))
    {
        aidMothershipDefense();
    }
}


/*=============================================================================
    Defense Startup, Shutdown and Other Stuff Functions:
=============================================================================*/
void aidTeamDied(struct AIPlayer *aiplayer,struct AITeam *team)
{
    sdword i;

    for (i=0;i<aiplayer->numGuardTeams;i++)
    {
        if (team == aiplayer->guardTeams[i])
        {
			aiplayer->numGuardTeams--;
			
			//check if the mothership defense team will replace this dead team
			if (aiplayer->mothershipdefteam == aiplayer->numGuardTeams)
			{
				aiplayer->mothershipdefteam = i;
			}

            aiplayer->guardTeams[i] = aiplayer->guardTeams[aiplayer->numGuardTeams];
				aiplayer->guardTeams[aiplayer->numGuardTeams] = NULL;

        }
    }
}

bool32 aidShipDied(struct AIPlayer *aiplayer, ShipPtr ship)
{
    bool32 return_value = FALSE;

    if ((aiplayer->aidProximitySensors) &&
        (clRemoveShipFromSelection(aiplayer->aidProximitySensors, ship)))
    {
        if (aiplayer->NumProxSensorsRequested)
        {
            aiplayer->NumProxSensorsRequested--;
        }
        return_value = TRUE;
    }
    if ((aiplayer->aidDefenseTargets) &&
        (clRemoveShipFromSelection(aiplayer->aidDefenseTargets, ship)))
    {
        return_value = TRUE;
    }

    if ((aiplayer->aidDistressShips) &&
        (clRemoveShipFromSelection(aiplayer->aidDistressShips, ship)))
    {
        return_value = TRUE;
    }

    if ((aiplayer->aidInvadingShips) &&
        (clRemoveShipFromSelection(aiplayer->aidInvadingShips, ship)))
    {
        return_value = TRUE;
    }

    if ((aiplayer->shipsattackingmothership) &&
        (clRemoveShipFromSelection(aiplayer->shipsattackingmothership, ship)))
    {
        return_value = TRUE;
    }

    return return_value;
}

void aidInit(struct AIPlayer *aiplayer)
{
    aiplayer->numGuardTeams            = 0;
    aiplayer->shipsattackingmothership = NULL;
    aiplayer->mothershipdefteam        = -1;
    aiplayer->NumProxSensorsRequested  = 0;
    aiplayer->aidProximitySensors      = NULL;
    aiplayer->aidDefenseTargets        = NULL;
    aiplayer->aidInvadingShips         = NULL;
    aiplayer->aidDistressShips         = NULL;
    aiplayer->sphereofinfluence        = 0;
    aiplayer->AlertStatus              = 0;
//    growSelectInit(&aiplayer->newdefenseships);

    switch (aiplayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            aiuEnableDefenseFeature(AID_ACTIVE_GUARD);
            aiuEnableDefenseFeature(AID_SPHERE_OF_INFLUENCE_INVADERS);
            aiuEnableDefenseFeature(AID_MOTHERSHIP_DEFENSE_HARDCORE);
            aiuEnableDefenseFeature(AID_CLOAK_DEFENSE_RED);
        case AI_INT:
            aiuEnableDefenseFeature(AID_GUARDING);
            aiuEnableDefenseFeature(AID_MOTHERSHIP_DEFENSE);
            aiuEnableDefenseFeature(AID_MOTHERSHIP_DEFENSE_MEDIUM);
            aiuEnableDefenseFeature(AID_CLOAK_DEFENSE);
        case AI_BEG:
            break;
        default:
            dbgAssert(FALSE);
    }
}

void aidClose(struct AIPlayer *aiplayer)
{
    sdword i;

//    growSelectClose(&aiplayer->newdefenseships);

    for (i=0;i<aiplayer->numGuardTeams;i++)
    {
        dbgAssert(aiplayer->guardTeams[i]);
        aitDestroy(aiplayer,aiplayer->guardTeams[i],FALSE);
    }

    aiumemFree(aiplayer->shipsattackingmothership);
    aiumemFree(aiplayer->aidProximitySensors);
    aiumemFree(aiplayer->aidDefenseTargets);
    aiumemFree(aiplayer->aidInvadingShips);
    aiumemFree(aiplayer->aidDistressShips);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : aidAddNewShip
    Description : Adds new ship to defense manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aidAddNewShip(Ship *ship)
{
    growSelectAddShip(&aiCurrentAIPlayer->newdefenseships, ship);
}
#endif

#if 0  // Cat2: Binary aidCheckSphereOfInfluence; live implementation at AIDefenseMan.c L192 (0x4028D0)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x4028D0
Ship *aidCheckSphereOfInfluence(void)
{
   AIPlayer *v0; // ecx
   Ship *result; // eax
   float *p_x; // eax
   float v3; // edi
   float v4; // ebx
   float v5; // ebp
   unsigned int *v6; // eax
   unsigned int *v7; // esi
   int v8; // esi
   _DWORD *autoLaunch; // eax
   AIPlayerLevel v10; // [esp+10h] [ebp-8h]
   float v11; // [esp+14h] [ebp-4h]

   v0 = aiCurrentAIPlayer;
   result = aiCurrentAIPlayer->player->PlayerMothership;
   if ( result )
   {
     p_x = &result->posinfo.position.x;
     v3 = *p_x;
     v4 = p_x[1];
     v5 = p_x[2];
     if ( *(float *)&aiCurrentAIPlayer[368].DefenseFeatures == ZERO
       || (*(double *)&dbl_C16840 = universe.totaltimeelapsed + dbl_72A458, (*(_DWORD *)&dbl_C16840 & dword_74AA00) == 0) )
     {
       result = (Ship *)aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
       if ( !result )
         return result;
       *(float *)&aiCurrentAIPlayer[368].DefenseFeatures = aiuFindDistanceSquared(
                                                             result->posinfo.position.x,
                                                             result->posinfo.position.y,
                                                             result->posinfo.position.z,
                                                             v3,
                                                             v4,
                                                             v5)
                                                         * flt_72A450;
       v0 = aiCurrentAIPlayer;
     }
     if ( (v0->DefenseFeatures & 4) != 0 )
     {
       v6 = sub_41CF90(v3);
       v7 = v6;
       if ( v6 )
       {
         aiCurrentAIPlayer[368].aiplayerDifficultyLevel = (AIPlayerLevel)aidSendDistressSignal(
                                                                           (char *)aiCurrentAIPlayer[368].aiplayerDifficultyLevel,
                                                                           v6,
                                                                           1);
         aiCurrentAIPlayer[368].ResourceFeatures = (udword)aidSendDistressSignal(
                                                             (char *)aiCurrentAIPlayer[368].ResourceFeatures,
                                                             v7,
                                                             1);
         memFree((ubyte *)v7);
       }
       v0 = aiCurrentAIPlayer;
     }
     result = (Ship *)v0[369].aiplayerDifficultyLevel;
     v10 = AI_BEG;
     v11 = *(float *)&v0[368].DefenseFeatures * dbl_72A448;
     if ( result )
     {
       v8 = 7388;
       do
       {
         autoLaunch = (_DWORD *)(*(Player **)((char *)&v0->player + v8))->autoLaunch;
         if ( *autoLaunch )
         {
           if ( aiuFindDistanceSquared(
                  v3,
                  v4,
                  v5,
                  *(float *)(autoLaunch[2] + 88),
                  *(float *)(autoLaunch[2] + 92),
                  *(float *)(autoLaunch[2] + 96)) > v11 )
             sub_4176B0(*(int *)((char *)&aiCurrentAIPlayer->player + v8));
           v0 = aiCurrentAIPlayer;
         }
         result = (Ship *)(v10 + 1);
         v8 += 4;
         ++v10;
       }
       while ( (unsigned int)v10 < v0[369].aiplayerDifficultyLevel );
     }
   }
   return result;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x402AA0
int *aidSetupResourceDefenseTeams(void)
{
   AIPlayer *v0; // edx
   int *result; // eax
   int v2; // edi
   AIPlayerLevel *p_aiplayerDifficultyLevel; // eax
   int v4; // ebx
   udword *p_DefenseFeatures; // ebp
   _DWORD *v6; // esi
   int v7; // eax
   int *v8; // eax
   _DWORD *v9; // esi
   bool v10; // cc
   int v11; // [esp+0h] [ebp-Ch]
   int v12; // [esp+4h] [ebp-8h]
   AIPlayerLevel *v13; // [esp+8h] [ebp-4h]

   v0 = aiCurrentAIPlayer;
   result = (int *)aiCurrentAIPlayer[149].aiplayerDifficultyLevel;
   v12 = 0;
   if ( *result > 0 )
   {
     v11 = 2;
     do
     {
       v2 = result[v11];
       switch ( *(_DWORD *)(v2 + 688) )
       {
         case 0xE:
         case 0x10:
         case 0x31:
         case 0x33:
         case 0x3C:
         case 0x4A:
         case 0x58:
           p_aiplayerDifficultyLevel = &v0[372].aiplayerDifficultyLevel;
           v4 = 0;
           v13 = &v0[372].aiplayerDifficultyLevel;
           if ( v0[372].aiplayerDifficultyLevel <= AI_BEG )
           {
 LABEL_13:
             if ( v0[369].aiplayerDifficultyLevel < 12 )
             {
               v9 = memAllocFunction(12, MBF_Volatile);
               *v9 = 1;
               v9[2] = v2;
               *(&aiCurrentAIPlayer[369].ResourceFeatures + aiCurrentAIPlayer[369].aiplayerDifficultyLevel) = (udword)aitCreate(1);
               aioCreateGuardShips(
                 *(&aiCurrentAIPlayer[369].ResourceFeatures + aiCurrentAIPlayer[369].aiplayerDifficultyLevel),
                 v9);
               ++aiCurrentAIPlayer[369].aiplayerDifficultyLevel;
             }
           }
           else
           {
             p_DefenseFeatures = &v0[371].DefenseFeatures;
             while ( 1 )
             {
               v6 = *(_DWORD **)(*(_DWORD *)(*p_DefenseFeatures + 4 * v4) + 48);
               if ( v6 )
                 break;
 LABEL_12:
               if ( ++v4 >= *p_aiplayerDifficultyLevel )
                 goto LABEL_13;
             }
             while ( 1 )
             {
               v7 = v6[3];
               if ( *(_DWORD *)(v7 + 16) == 1 )
               {
                 v8 = *(int **)(v7 + 20);
                 if ( v8 )
                 {
                   if ( ShipInSelection(v8, (Ship *)v2) )
                     break;
                 }
               }
               v6 = (_DWORD *)*v6;
               if ( !v6 )
               {
                 v0 = aiCurrentAIPlayer;
                 p_aiplayerDifficultyLevel = v13;
                 goto LABEL_12;
               }
             }
           }
           break;
         default:
           break;
       }
       v0 = aiCurrentAIPlayer;
       result = (int *)aiCurrentAIPlayer[149].aiplayerDifficultyLevel;
       v10 = ++v12 < *result;
       ++v11;
     }
     while ( v10 );
   }
   return result;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x402E00
AIPlayer *aidCloakDefense(void)
{
   AIPlayer *v0; // esi
   udword v1; // eax
   char v2; // al
   char v3; // di
   udword v4; // eax
   unsigned int v5; // edi
   Player *v6; // ecx
   ShipRaceEnum v7; // ecx
   Player *v8; // eax
   ShipRaceEnum v9; // eax
   Player *player; // ecx
   udword DefenseFeatures; // eax
   int PlayerMothership; // ecx
   ShipRaceEnum race; // edx
   udword AttackFeatures; // ebx
   Player *v15; // ecx
   AIPlayer *result; // eax
   unsigned int v17; // ebx
   unsigned int v18; // esi
   int v19; // eax
   unsigned int v20[3]; // [esp+Ch] [ebp-Ch] BYREF

   if ( !aiuFindCloakersInEnemyShipsIAmAwareOf(aiCurrentAIPlayer->DefenseFeatures & 0x200) )
   {
     v0 = aiCurrentAIPlayer;
     *(double *)&dbl_C16840 = universe.totaltimeelapsed + dbl_72A458;
     if ( (*(_DWORD *)&dbl_C16840 & dword_74AA00) == 0 )
     {
       player = aiCurrentAIPlayer[368].player;
       if ( player )
       {
         DefenseFeatures = aiCurrentAIPlayer[367].DefenseFeatures;
         PlayerMothership = (int)player->PlayerMothership;
         if ( DefenseFeatures > PlayerMothership && PlayerMothership < 3 )
         {
           race = aiCurrentAIPlayer->player->race;
           if ( race == 2 )
           {
             aifResourceManRequestsShipsCB(0x47u, DefenseFeatures - PlayerMothership, 0);
 LABEL_28:
             v0 = aiCurrentAIPlayer;
             goto LABEL_29;
           }
           if ( race == 3 )
           {
             aifResourceManRequestsShipsCB(0x57u, DefenseFeatures - PlayerMothership, 0);
             goto LABEL_28;
           }
         }
       }
     }
 LABEL_29:
     AttackFeatures = v0[367].AttackFeatures;
     if ( (AttackFeatures & 2) != 0 && ((__int64)universe.totaltimeelapsed & (unsigned int)dword_74AA00) == 0 )
     {
       v0[367].AttackFeatures = AttackFeatures & 0xFFFFFFFD;
       goto LABEL_32;
     }
     goto LABEL_33;
   }
   v0 = aiCurrentAIPlayer;
   v1 = aiCurrentAIPlayer[367].AttackFeatures;
   if ( (v1 & 2) != 0 )
   {
     v2 = ranRandomFn(6);
     v0 = aiCurrentAIPlayer;
     v3 = v2;
     v4 = aiCurrentAIPlayer[367].DefenseFeatures;
     v5 = (v3 & 3) + 3;
     if ( v4 < v5 )
     {
       v6 = aiCurrentAIPlayer[368].player;
       if ( v6 )
       {
         if ( (int)v6->PlayerMothership < 6 )
         {
           v7 = aiCurrentAIPlayer->player->race;
           if ( v7 == 2 )
           {
             aifResourceManRequestsShipsCB(0x47u, v5 - v4, 0);
           }
           else
           {
             if ( v7 != 3 )
             {
 LABEL_11:
               v0[367].DefenseFeatures = v5;
 LABEL_32:
               v0 = aiCurrentAIPlayer;
               goto LABEL_33;
             }
             aifResourceManRequestsShipsCB(0x57u, v5 - v4, 0);
           }
           v0 = aiCurrentAIPlayer;
           goto LABEL_11;
         }
       }
     }
   }
   else if ( (v1 & 1) != 0 && !aiCurrentAIPlayer[367].DefenseFeatures )
   {
     v8 = aiCurrentAIPlayer[368].player;
     if ( v8 )
     {
       if ( (int)v8->PlayerMothership < 6 )
       {
         aiCurrentAIPlayer[367].DefenseFeatures = ranRandomFn(6) % 3 + 1;
         v0 = aiCurrentAIPlayer;
         v9 = aiCurrentAIPlayer->player->race;
         if ( v9 == 2 )
         {
           aifResourceManRequestsShipsCB(0x47u, aiCurrentAIPlayer[367].DefenseFeatures, 0);
           goto LABEL_32;
         }
         if ( v9 == 3 )
         {
           aifResourceManRequestsShipsCB(0x57u, aiCurrentAIPlayer[367].DefenseFeatures, 0);
           goto LABEL_32;
         }
       }
     }
   }
 LABEL_33:
   v15 = v0[368].player;
   result = v0 + 368;
   if ( v15 )
   {
     v17 = 0;
     if ( v15->PlayerMothership )
     {
       v18 = 8;
       do
       {
         if ( v18 > 0x14 || (v17 & 1) != 0 )
         {
           v20[0] = 1;
           v20[2] = *(unsigned int *)((char *)&result->player->PlayerMothership + v18);
           if ( v18 <= 0xC )
             v19 = sub_4178E0(v17);
           else
             v19 = sub_4178E0(v17 - 1);
           aiuWrapProtect(v20, *(unsigned int **)(v19 + 32));
         }
         else
         {
           sub_402C40(v17);
         }
         result = aiCurrentAIPlayer + 368;
         ++v17;
         v18 += 4;
       }
       while ( (Ship *)v17 < aiCurrentAIPlayer[368].player->PlayerMothership );
     }
   }
   return result;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x403130
void aidMothershipDefense(void)
{
   Ship *PlayerMothership; // esi
   unsigned int *v1; // ebp
   int *NearbyShips; // edi
   int gettingrocked; // eax
   AIPlayer *v4; // ecx
   Player *v5; // eax
   int v6; // edi
   unsigned int *v7; // eax
   unsigned int v8; // esi
   int v9; // ebx
   AIPlayer *v10; // eax
   Player *v11; // ecx
   AIPlayerLevel v12; // edx
   unsigned int *v13; // eax
   Player *player; // ebx
   int v15; // ebp
   AIPlayer *v16; // eax
   Player *v17; // ecx
   Player *aiplayerDifficultyLevel; // esi
   int v19; // edi
   int v20; // esi
   AIPlayer *v21; // eax
   char *v22; // ebx
   AIPlayerLevel i; // edi
   udword DefenseFeatures; // edx
   _DWORD *v25; // esi
   _DWORD *v26; // eax
   int *v27; // eax
   AIPlayer *v28; // eax
   char *v29; // edi
   AIPlayerLevel j; // ebx
   udword v31; // edx
   int v32; // esi
   _DWORD *v33; // eax
   int *v34; // eax
   signed int *v35; // eax
   char *v36; // eax
   int *v37; // [esp+10h] [ebp-Ch]
   Ship *v38; // [esp+14h] [ebp-8h]
   unsigned int *v39; // [esp+18h] [ebp-4h]

   PlayerMothership = aiCurrentAIPlayer->player->PlayerMothership;
   v1 = 0;
   NearbyShips = aiuEnemyShipsInMothershipBlob();
   v37 = NearbyShips;
   if ( NearbyShips
     || PlayerMothership
     && (gettingrocked = (int)PlayerMothership->gettingrocked) != 0
     && (NearbyShips = aiuFindNearbyShips(gettingrocked, dword_74AA14),
         v37 = NearbyShips,
         selSelectionAddSingleShip(NearbyShips, (int)PlayerMothership->gettingrocked),
         NearbyShips) )
   {
     if ( *NearbyShips != 1 || *(_DWORD *)(*(_DWORD *)(NearbyShips[2] + 32) + 488) != 6 )
     {
       if ( aiCurrentAIPlayer[367].aiplayerDifficultyLevel )
       {
         memFree((ubyte *)aiCurrentAIPlayer[367].aiplayerDifficultyLevel);
         aiCurrentAIPlayer[367].aiplayerDifficultyLevel = AI_BEG;
       }
       aiCurrentAIPlayer[367].aiplayerDifficultyLevel = (AIPlayerLevel)selectMemDupSelection(
                                                                         NearbyShips,
                                                                         (int)aMsdp,
                                                                         (char *)4);
       v13 = (unsigned int *)memAllocFunction(4 * (int)aiCurrentAIPlayer[2].player->PlayerMothership + 8, MBF_Volatile);
       *v13 = 0;
       player = aiCurrentAIPlayer[2].player;
       v39 = v13;
       if ( player->PlayerMothership )
       {
         v38 = 0;
         v15 = 8;
         do
         {
           if ( aiuIsShipDangerous(*(int *)((char *)&player->PlayerMothership + v15)) )
           {
             v16 = aiCurrentAIPlayer;
             v17 = aiCurrentAIPlayer[369].player;
             if ( v17 == (Player *)-1 )
             {
               aiplayerDifficultyLevel = (Player *)aiCurrentAIPlayer[369].aiplayerDifficultyLevel;
               *(&aiCurrentAIPlayer[369].ResourceFeatures + (_DWORD)aiplayerDifficultyLevel) = (udword)aitCreate(1);
               v19 = *(&aiCurrentAIPlayer[369].ResourceFeatures + (_DWORD)aiplayerDifficultyLevel);
               sub_40E950(v19);
               aiCurrentAIPlayer[369].player = aiplayerDifficultyLevel;
               ++aiCurrentAIPlayer[369].aiplayerDifficultyLevel;
               v16 = aiCurrentAIPlayer;
             }
             else
             {
               v19 = *(&aiCurrentAIPlayer[369].ResourceFeatures + (_DWORD)v17);
             }
             v20 = *(int *)((char *)&player->PlayerMothership + v15);
             growSelectRemoveShip((int)&v16[1].DefenseFeatures, v20);
             player = aiCurrentAIPlayer[2].player;
             aifAddShipToTeam(v19, v20);
           }
           v15 += 4;
           v38 = (Ship *)((char *)v38 + 1);
         }
         while ( v38 < player->PlayerMothership );
       }
       v21 = aiCurrentAIPlayer;
       v22 = 0;
       for ( i = AI_BEG; (unsigned int)i < aiCurrentAIPlayer[372].aiplayerDifficultyLevel; ++i )
       {
         DefenseFeatures = v21[371].DefenseFeatures;
         v25 = *(_DWORD **)(DefenseFeatures + 4 * i);
         if ( *v25 != 3 )
         {
           if ( sub_417260(*(_DWORD *)(DefenseFeatures + 4 * i)) )
           {
             v26 = (_DWORD *)v25[8];
             if ( *v26 )
             {
               if ( aiuIsShipDangerous(v26[2]) )
               {
                 v27 = sub_40C370((int)v25, 1, 1);
                 aimInsertMove((int)v25, v27);
                 v22 = aidSendDistressSignal(
                         v22,
                         *(unsigned int **)(*(_DWORD *)(aiCurrentAIPlayer[371].DefenseFeatures + 4 * i) + 32),
                         1);
               }
             }
           }
         }
         v21 = aiCurrentAIPlayer;
       }
       v1 = (unsigned int *)aidSendDistressSignal(v22, v39, 3);
       aiuMakeShipsOnlyDangerousToMothership((signed int)v1, v37);
       if ( *v37 > 0
         && (aiCurrentAIPlayer->DefenseFeatures & 0x20) != 0
         && (!*v1 || sub_524ED0(v37, (signed int *)v1) > flt_74A9F8) )
       {
         v28 = aiCurrentAIPlayer;
         v29 = 0;
         for ( j = AI_BEG; (unsigned int)j < aiCurrentAIPlayer[372].aiplayerDifficultyLevel; ++j )
         {
           v31 = v28[371].DefenseFeatures;
           v32 = *(_DWORD *)(v31 + 4 * j);
           if ( *(_DWORD *)v32 != 3 )
           {
             if ( sub_417290(*(_DWORD *)(v31 + 4 * j)) )
             {
               v33 = *(_DWORD **)(v32 + 32);
               if ( *v33 )
               {
                 if ( aiuIsShipDangerous(v33[2]) )
                 {
                   v34 = sub_40C370(v32, 1, 1);
                   aimInsertMove(v32, v34);
                   v29 = aidSendDistressSignal(v29, *(unsigned int **)(v32 + 32), 1);
                 }
               }
             }
           }
           v28 = aiCurrentAIPlayer;
         }
         v35 = (signed int *)aidSendDistressSignal(v29, v1, 3);
         v1 = (unsigned int *)v35;
         if ( (aiCurrentAIPlayer->DefenseFeatures & 0x40) != 0 && (!*v35 || sub_524ED0(v37, v35) > flt_74A9F8) )
         {
           v36 = sub_403070(v37);
           v1 = (unsigned int *)aidSendDistressSignal(v36, v1, 3);
         }
       }
       goto LABEL_18;
     }
     ranRandomFn(6);
   }
   v4 = aiCurrentAIPlayer;
   if ( aiCurrentAIPlayer[367].aiplayerDifficultyLevel )
   {
     memFree((ubyte *)aiCurrentAIPlayer[367].aiplayerDifficultyLevel);
     aiCurrentAIPlayer[367].aiplayerDifficultyLevel = AI_BEG;
     v4 = aiCurrentAIPlayer;
   }
   v5 = v4[369].player;
   if ( v5 != (Player *)-1 )
   {
     v6 = *(&v4[369].ResourceFeatures + (_DWORD)v5);
     v7 = *(unsigned int **)(v6 + 32);
     v8 = 0;
     if ( *v7 )
     {
       v9 = 2;
       do
       {
         growSelectAddShip((int *)&v4[1].DefenseFeatures, v7[v9]);
         v7 = *(unsigned int **)(v6 + 32);
         ++v8;
         ++v9;
         v4 = aiCurrentAIPlayer;
       }
       while ( v8 < *v7 );
     }
     aitDestroy(v4, v6, 1);
     v10 = aiCurrentAIPlayer;
     v11 = aiCurrentAIPlayer[369].player;
     v12 = aiCurrentAIPlayer[369].aiplayerDifficultyLevel;
     if ( v11 != (Player *)v12 && !*(&aiCurrentAIPlayer[369].ResourceFeatures + (_DWORD)v11) )
     {
       aiCurrentAIPlayer[369].aiplayerDifficultyLevel = v12 - 1;
       *(&aiCurrentAIPlayer[369].ResourceFeatures + (int)aiCurrentAIPlayer[369].player) = *(&aiCurrentAIPlayer[369].ResourceFeatures
                                                                                          + aiCurrentAIPlayer[369].aiplayerDifficultyLevel);
       v10 = aiCurrentAIPlayer;
     }
     v10[369].player = (Player *)-1;
 LABEL_18:
     NearbyShips = v37;
   }
   if ( NearbyShips )
     memFree((ubyte *)NearbyShips);
   if ( v1 )
     memFree((ubyte *)v1);
 }
#endif

#if 0  // Cat2: aidPositionProximitySensorGuard live code differs from binary
/*=== aidPositionProximitySensorGuard (0x402C40) ===*/
void __cdecl aidPositionProximitySensorGuard(unsigned int index)
{
AIPlayer *v1; // ecx
float *unk; // edx
vector *v3; // eax
real32 x; // ebx
__int64 v5; // rdi
int *player; // eax
vector *v7; // eax
vector v8; // [esp-28h] [ebp-68h]
vector v9; // [esp-20h] [ebp-60h]
vector v10; // [esp-1Ch] [ebp-5Ch]
vector v11[2]; // [esp-10h] [ebp-50h] BYREF
SelectCommand selection; // [esp+1Ch] [ebp-24h] BYREF
vector v13; // [esp+28h] [ebp-18h] BYREF
vector v14; // [esp+34h] [ebp-Ch] BYREF

v1 = aiCurrentAIPlayer;
unk = (float *)aiCurrentAIPlayer[368].player->unk;
if ( unk[147] == ZERO && unk[148] == ZERO && unk[149] == ZERO && index != 2 )
{
selection.numShips = 1;
selection.ShipPtr[0] = (ShipPtr)*(&aiCurrentAIPlayer[368].player->unk + index);
if ( index )
{
v5 = *(_QWORD *)&selection.timeLastStatus;
x = *(float *)&selection.numShips;
}
else
{
memset(v11, 0, 12);
v9 = *aiuPlayerMothershipCoords(&v13, aiCurrentAIPlayer->player);
v3 = aiuGenerateRandomStandoffPoint(&v14, v9, 6000.0, v11[0], 1u);
x = v3->x;
v5 = *(_QWORD *)&v3->y;
aimIssueMoveCommand(&selection, v3->x, *(real32 *)&v5, *((real32 *)&v5 + 1));
v1 = aiCurrentAIPlayer;
}
player = (int *)v1[368].player;
if ( *player > 2 )
{
selection.ShipPtr[0] = (ShipPtr)player[4];
memset(v11, 0, 12);
v10.x = x;
*(_QWORD *)&v10.y = v5;
v8 = *aiuPlayerMothershipCoords(&v14, v1->player);
v7 = aiuGenerateFlankCoordinates(&v13, v8, v10, v11[0], 6000.0);
aimIssueMoveCommand(&selection, v7->x, v7->y, v7->z);
}
}
}
#endif

#if 0  // Cat2: aidAddAllShips live code differs from binary
/*=== aidAddAllShips (0x403070) ===*/
SelectCommand *__cdecl aidAddAllShips(SelectCommand *enemyships)
{
float v1; // ebp
struct Ship *v2; // esi
AIPlayer *v3; // eax
SelectCommand *v4; // edi
AIPlayerLevel v5; // ebx
int v6; // esi
_DWORD *v7; // eax
Ship *v8; // ecx
ShipClassEnum shipclass; // eax
struct AITeamMove *DefendMothershipNoAdd; // eax
SelectCommand *v11; // eax
struct Ship *v13; // [esp-8h] [ebp-10h]
real32 v14; // [esp-4h] [ebp-Ch]

v3 = aiCurrentAIPlayer;
v4 = 0;
v5 = AI_BEG;
if ( aiCurrentAIPlayer[372].aiplayerDifficultyLevel == AI_BEG )
return 0;
v14 = v1;
v13 = v2;
do
{
v6 = *(_DWORD *)(v3[371].DefenseFeatures + 4 * v5);
if ( *(_DWORD *)v6 != 3 )
{
v7 = *(_DWORD **)(v6 + 32);
if ( *v7 )
{
v8 = (Ship *)v7[2];
shipclass = v8->staticinfo->shipclass;
if ( shipclass )
{
if ( shipclass != CLASS_Carrier
&& aiuIsShipDangerous(v8)
&& aitTeamIsntDefendingMothership((struct AITeam *)v6, enemyships) )
{
if ( aitTeamIsInRange((struct AITeam *)v6, v13, v14) )
{
DefendMothershipNoAdd = aimCreateDefendMothershipNoAdd((struct AITeam *)v6, 1, 1);
aimInsertMove((struct AITeam *)v6, DefendMothershipNoAdd);
aidSendDistressSignal(v4);
v4 = v11;
}
}
}
}
}
v3 = aiCurrentAIPlayer;
++v5;
}
while ( (unsigned int)v5 < aiCurrentAIPlayer[372].aiplayerDifficultyLevel );
return v4;
}
#endif

#if 0  // Cat2: aidClose live code differs from binary
/*=== aidClose (0x4039B0) ===*/
void __cdecl aidClose(struct AIPlayer *aiplayer)
{
AIPlayerLevel v1; // edi
struct AITeam **p_ResourceFeatures; // ebx

v1 = AI_BEG;
if ( aiplayer[369].aiplayerDifficultyLevel > AI_BEG )
{
p_ResourceFeatures = (struct AITeam **)&aiplayer[369].ResourceFeatures;
do
{
aitDestroy(aiplayer, *p_ResourceFeatures, 0);
++v1;
++p_ResourceFeatures;
}
while ( v1 < aiplayer[369].aiplayerDifficultyLevel );
}
if ( aiplayer[367].aiplayerDifficultyLevel )
{
memFree((ubyte *)aiplayer[367].aiplayerDifficultyLevel);
aiplayer[367].aiplayerDifficultyLevel = AI_BEG;
}
if ( aiplayer[368].player )
{
memFree((ubyte *)aiplayer[368].player);
aiplayer[368].player = 0;
}
if ( aiplayer[368].aiplayerDifficultyLevel )
{
memFree((ubyte *)aiplayer[368].aiplayerDifficultyLevel);
aiplayer[368].aiplayerDifficultyLevel = AI_BEG;
}
if ( aiplayer[368].ResourceFeatures )
{
memFree((ubyte *)aiplayer[368].ResourceFeatures);
aiplayer[368].ResourceFeatures = 0;
}
if ( aiplayer[368].AttackFeatures )
{
memFree((ubyte *)aiplayer[368].AttackFeatures);
aiplayer[368].AttackFeatures = 0;
}
}
#endif
