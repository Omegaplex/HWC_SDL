/*=============================================================================
    Name    : AIResourceMan.c
    Purpose : Code for the Resource Manager of the computer player

    Created 5/27/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "universe.h"
#include "univupdate.h"
#include "rescollect.h"
#include "CommandWrap.h"
#include "AIResourceMan.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIOrders.h"
#include "AITeam.h"
#include "AIUtilities.h"
#include "AIDefenseMan.h"
#include "KAS.h"
#include "KASFunc.h"
#include "KASHashes.h"
#include "researchapi.h"
#include "select.h"
#include "blobs.h"
#include "consMgr.h"
#include "MultiplayerGame.h"
#include "randy.h"
#include "statscript.h"

udword UPDATE_RU_COUNT_RATE = 3;

sdword BUILD_RC_CASH_TABLE_BASE = 400;
sdword BUILD_RC_CASH_TABLE_INC = 200;

sdword MAX_RCOLLECTORS_PER_DOCK_POINT = 3;

sdword MAX_RCOLLECTORS_TO_BUILD = 5;

sdword MIN_WORTHWHILE_RUs = 400;

sdword RUs_PER_RCOLLECTOR = 1000;


sdword MIN_WORTHWHILE_RUs_FOR_CONTROLLER = 3000;

sdword MIN_RCOLLECTORS_FOR_CONTROLLER = 4;

sdword BUILD_RCONTROLLER_CASH_BASE = 1500;

sdword BUILD_ASF_CASH_BASE = 6000;
sdword BUILD_SUPPORT_CAP_CUSHION = 30;

real32 ASF_POSITION_TO_ENEMY_MOTHERSHIP = 0.4f;

real32 RCONTROLLER_POS_FACTOR_FRIENDLIES_PRESENT = 1.1f;
real32 RCONTROLLER_POS_FACTOR_ENEMIESPRESENT_BUT_OUTNUMBERED = 0.6f;
real32 RCONTROLLER_POS_FACTOR_ENEMIESPRESENT = 0.3f;

real32 RCONTROLLER_POS_FACTOR_INBETWEEN = 1.0f;
real32 RCONTROLLER_POS_FACTOR_NOTINBETWEEN = 0.1f;

scriptEntry AIResourceManTweaks[] =
{
    makeEntry(UPDATE_RU_COUNT_RATE,scriptSetUdwordCB),
    makeEntry(BUILD_RC_CASH_TABLE_BASE,scriptSetSdwordCB),
    makeEntry(BUILD_RC_CASH_TABLE_INC,scriptSetSdwordCB),
    makeEntry(MAX_RCOLLECTORS_PER_DOCK_POINT,scriptSetSdwordCB),
    makeEntry(MAX_RCOLLECTORS_TO_BUILD,scriptSetSdwordCB),
    makeEntry(MIN_WORTHWHILE_RUs,scriptSetSdwordCB),
    makeEntry(RUs_PER_RCOLLECTOR,scriptSetSdwordCB),
    makeEntry(MIN_WORTHWHILE_RUs_FOR_CONTROLLER,scriptSetSdwordCB),
    makeEntry(MIN_RCOLLECTORS_FOR_CONTROLLER,scriptSetSdwordCB),
    makeEntry(BUILD_RCONTROLLER_CASH_BASE,scriptSetSdwordCB),
    makeEntry(ASF_POSITION_TO_ENEMY_MOTHERSHIP,scriptSetReal32CB),
    makeEntry(BUILD_ASF_CASH_BASE,scriptSetSdwordCB),
    makeEntry(RCONTROLLER_POS_FACTOR_FRIENDLIES_PRESENT,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_ENEMIESPRESENT_BUT_OUTNUMBERED,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_ENEMIESPRESENT,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_INBETWEEN,scriptSetReal32CB),
    makeEntry(RCONTROLLER_POS_FACTOR_NOTINBETWEEN,scriptSetReal32CB),
    endEntry
};

/*-----------------------------------------------------------------------------
    Name        : NumberOfEasilyAccesibleRUs
    Description : returns total number of easily accesible RU's to player
    Inputs      :
    Outputs     :
    Return      : returns total number of easily accesible RU's to player
----------------------------------------------------------------------------*/
sdword NumberOfEasilyAccesibleRUs(Player *player)
{
    // for now, just return the total number of RU's.
    Node *objnode = universe.ResourceList.head;
    Resource *resource;
    sdword numRUs = 0;

    while (objnode != NULL)
    {
        resource = (Resource *)listGetStructOfNode(objnode);
        dbgAssert(resource->objtype >= OBJ_AsteroidType && resource->objtype <= OBJ_DustType);

        numRUs += resource->resourcevalue;
        objnode = objnode->next;
    }

    return numRUs;
}

/*-----------------------------------------------------------------------------
    Name        : NumRUDockPointsOnThisShip
    Description : returns number of RU dock points on ship
    Inputs      :
    Outputs     :
    Return      : returns number of RU dock points on ship
----------------------------------------------------------------------------*/
sdword NumRUDockPointsOnThisShip(Ship *ship)
{
    if ((ship->shiptype == kCarrier) || (ship->shiptype == tCarrier))
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

/*-----------------------------------------------------------------------------
    Name        : airInit
    Description : initialize resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airInit(struct AIPlayer *aiplayer)
{
    udword i;

    growSelectInit(&aiplayer->airResourceReserves);
    growSelectInit(&aiplayer->airResourceCollectors);
    aiplayer->airEasilyAccesibleRUsInWorld = -1;
    aiplayer->airNumRCollectors            = 0;
    aiplayer->airNumRControllers           = 0;
    aiplayer->airNumASF                    = 0;
    aiplayer->airUnused0                   = 0;
    aiplayer->airNumResearchShips          = 0;
    aiplayer->numSupportTeams              = 0;
    aiplayer->ResourcersToBuild            = 0;
    aiplayer->ResearchDelay                = 0;

    for (i=0;i<AIPLAYER_NUM_SUPPORTTEAMS;i++)
    {
        aiplayer->supportTeam[i] = NULL;
    }

    switch (aiplayer->aiplayerDifficultyLevel)
    {
        case AI_ADV:
            aiuEnableResourceFeature(AIR_ACTIVE_MOTHERSHIP);
            aiuEnableResourceFeature(AIR_ACTIVE_RESOURCE_COLLECTION);
            aiuEnableResourceFeature(AIR_ACTIVE_SUPPORT_FRIGATE);
            aiuEnableResourceFeature(AIR_RESOURCE_DISTRESS_SIGNALS);
            aiuEnableResourceFeature(AIR_SMART_COLLECTOR_REQUESTS);
            aiuEnableResourceFeature(AIR_SMART_RESEARCH_SHIP_REQUESTS);
        case AI_INT:
            aiuEnableResourceFeature(AIR_AGGRESSIVE_RESOURCING);
            aiuEnableResourceFeature(AIR_RESOURCE_CONTROLLER_REQUESTS);
            aiuEnableResourceFeature(AIR_SUPPORT_FRIGATE_REQUESTS);
        case AI_BEG:
            aiuEnableResourceFeature(AIR_ACTIVE_RESOURCE_CONTROLLER);
            break;
        default:
            dbgAssert(FALSE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : airClose
    Description : Closes resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airClose(struct AIPlayer *aiplayer)
{
    udword i;

    growSelectClose(&aiplayer->airResourceReserves);
    growSelectClose(&aiplayer->airResourceCollectors);

    for (i=0;i<AIPLAYER_NUM_SUPPORTTEAMS;i++)
    {
        if (aiplayer->supportTeam[i])
        {
            aitDestroy(aiplayer, aiplayer->supportTeam[i], FALSE);
            aiplayer->supportTeam[i] = NULL;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : airShipDied
    Description : Removes a dead ship from the Combat Manager's internal structures
    Inputs      : aiplayer, ship - the ship that died
    Outputs     :
    Return      : TRUE if the ship was found and removed
----------------------------------------------------------------------------*/
bool32 airShipDied(struct AIPlayer *aiplayer,ShipPtr ship)
{
    if (!growSelectRemoveShip(&aiplayer->airResourceReserves, ship))
    {
        return FALSE;
    }

    switch (ship->shiptype)
    {
        case sCarrierSupport:
        case sMothershipSupport:
        case bMothershipSupport:
            aiplayer->airNumResearchShips--;
            dbgAssert(aiplayer->airNumResearchShips >= 0);
            break;

        case sWorker:
        case bWorker:
        case tResourceCollector:
            growSelectRemoveShip(&aiplayer->airResourceReserves, ship);
            aiplayer->airEasilyAccesibleRUsInWorld--;
            break;

        case sProcessor:
        case bProcessor:
            aiplayer->airNumRCollectors--;
            dbgAssert(aiplayer->airNumRCollectors >= 0);
            break;

        case kAdvanceSupportFrigate:
        case tAdvanceSupportFrigate:
            aiplayer->airNumRControllers--;
            dbgAssert(aiplayer->airNumRControllers >= 0);
            break;
    }

    return TRUE;
}


/*-----------------------------------------------------------------------------
    Name        : airTeamDied
    Description : Removes a team from the resource stuff and all that jazz
    Inputs      : aiplayer, team
    Outputs     : does some things
    Return      : void
----------------------------------------------------------------------------*/
void airTeamDied(struct AIPlayer *aiplayer, struct AITeam *team)
{
    udword i;

    for (i=0;i<aiplayer->numSupportTeams;i++)
    {
        if (aiplayer->supportTeam[i] == team)
        {
            aiplayer->numSupportTeams--;
            aiplayer->supportTeam[i] = aiplayer->supportTeam[aiplayer->numSupportTeams];
            aiplayer->supportTeam[aiplayer->numSupportTeams] = NULL;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : airAddedResourceCollector
    Description : called when a resource collector is added
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airAddedResourceCollector(Ship *ship)
{
    Resource *resource;
    SelectCommand selectone;
    SelectCommand dockSelection;
    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;
    AITeam *team;

    growSelectAddShip(&aiCurrentAIPlayer->airResourceCollectors, ship);

    aiCurrentAIPlayer->airNumRCollectors++;
    if (aiCurrentAIPlayer->NumRCollectorsBeingBuilt > 0) aiCurrentAIPlayer->NumRCollectorsBeingBuilt--;

    if (singlePlayerGame || bitTest(tpGameCreated.flag, MG_HarvestinEnabled))
    {
        if (aiuResourceFeatureEnabled(AIR_ACTIVE_RESOURCE_COLLECTION))
        {
            //add the resource collector to an existing team, or make a new team
            team = aitFindNextTeamWithFlag(NULL, RESOURCECOLLECT_TEAM);

            while (team)
            {
                if (team->shipList.selection->numShips < 3)
                {
                    aifAddShipToTeam(team, ship);
                    break;
                }
                team = aitFindNextTeamWithFlag(team, RESOURCECOLLECT_TEAM);
            }

            if (!team)
            {
                team = aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
                bitSet(aiCurrentAIPlayer->supportTeam[new_team_num]->teamFlags, RESOURCECOLLECT_TEAM);
                growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
                aioCreateResourcer(aiCurrentAIPlayer->supportTeam[new_team_num]);
                aiCurrentAIPlayer->numSupportTeams++;
            }
            if (aiuResourceFeatureEnabled(AIR_AGGRESSIVE_RESOURCING))
            {
                aiuWrapSetTactics(team->shipList.selection, Aggressive);
            }
            else
            {
                aiuWrapSetTactics(team->shipList.selection, Evasive);
            }
        }
        else
        {
            resource = univFindNearestResource(ship, 0, NULL);

            selectone.numShips = 1;
            selectone.ShipPtr[0] = ship;

            if (aiuResourceFeatureEnabled(AIR_AGGRESSIVE_RESOURCING))
            {
                aiuWrapSetTactics(&selectone, Aggressive);
            }
            else
            {
                aiuWrapSetTactics(&selectone, Evasive);
            }
            aiuWrapCollectResource(&selectone, resource);
        }
    }
    else
    {
        dockSelection.numShips = 1;
        dockSelection.ShipPtr[0] = ship;
        clWrapDock(&universe.mainCommandLayer, &dockSelection, 7, 0);
    }
}

bool32 FindBestPlaceForAdvanceSupportFrigate(vector *destination)
{
    Ship *myMothership = aiCurrentAIPlayer->player->PlayerMothership;
    Ship *enemyMothership = aiuFindEnemyMothership(aiCurrentAIPlayer->player);
    vector diff;

    if (myMothership == NULL)
    {
        return FALSE;
    }

    if (enemyMothership == NULL)
    {
        return FALSE;
    }

    vecSub(diff,enemyMothership->posinfo.position,myMothership->posinfo.position);

    vecMultiplyByScalar(diff,ASF_POSITION_TO_ENEMY_MOTHERSHIP);

    vecAdd(*destination,myMothership->posinfo.position,diff);

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : airAddedResourceController
    Description : called when a resource controller is added
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airAddedResourceController(Ship *ship)
{
    SelectCommand selectone;
    vector destination;
    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_RESOURCE_CONTROLLER))
    {
        aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
        growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
        aioCreateActiveSupport(aiCurrentAIPlayer->supportTeam[new_team_num], NULL, SUPPORT_RESOURCE);
        aiCurrentAIPlayer->numSupportTeams++;
    }
    else
    {
        if (aiuFindBestResourceBlob(&destination))
        {
            selectone.numShips = 1;
            selectone.ShipPtr[0] = ship;

            aiuWrapMove(&selectone,destination);
        }
    }

    aiCurrentAIPlayer->airNumRControllers++;
    if (aiCurrentAIPlayer->NumRControllersBeingBuilt > 0) aiCurrentAIPlayer->NumRControllersBeingBuilt--;

    // tell new resource controller to do something here

}

/*-----------------------------------------------------------------------------
    Name        : airAddedAdvanceSupportFrigate
    Description : called when a Advance Support Frigate is added
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void airAddedAdvanceSupportFrigate(Ship *ship)
{
    vector destination;
    SelectCommand selectone;

    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_SUPPORT_FRIGATE))
    {
        aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
        growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
        aioCreateActiveSupport(aiCurrentAIPlayer->supportTeam[new_team_num], NULL, SUPPORT_STRIKECRAFT);
        aiCurrentAIPlayer->numSupportTeams++;
    }
    else
    {
        if (FindBestPlaceForAdvanceSupportFrigate(&destination))
        {
            selectone.numShips = 1;
            selectone.ShipPtr[0] = ship;

            aiuWrapMove(&selectone,destination);
        }
    }

    aiCurrentAIPlayer->airNumASF++;
    if (aiCurrentAIPlayer->NumASFBeingBuilt > 0) aiCurrentAIPlayer->NumASFBeingBuilt--;
}


/*-----------------------------------------------------------------------------
    Name        : airAddedResearchShip
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airAddedResearchShip(Ship *ship)
{
    (void)ship;

    kasCurrentStep = 1;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (!kasfTeamCount())
    {
        kasfMsgSend(kasAITeamPtrIfExists(KH_FleetIntel), (char *)0x0FD97064);
    }
}


/*-----------------------------------------------------------------------------
    Name        : airAddedProximitySensor
    Description : Does that proximity sensor adding stuff that we all read about
                  in Newsweek.  Proximity Sensor positioning and stuff is taken care
                  of by the defense manager
    Inputs      : ship - the new ship
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airAddedProximitySensor(ShipPtr ship)
{
    if (!aiCurrentAIPlayer->aidProximitySensors)
    {
        aiuNewSelection(aiCurrentAIPlayer->aidProximitySensors, 10, "dproxsens");
    }
    selSelectionAddSingleShip((MaxSelection *)aiCurrentAIPlayer->aidProximitySensors, ship);
}


/*-----------------------------------------------------------------------------
    Name        : airAddedMothership
    Description : Adds a mothership team to the support teams
    Inputs      : ship - the new mothership
    Outputs     : Creates a new team
    Return      : void
----------------------------------------------------------------------------*/
void airAddedMothership(ShipPtr ship)
{
    udword new_team_num = aiCurrentAIPlayer->numSupportTeams;

    if (aiuResourceFeatureEnabled(AIR_ACTIVE_MOTHERSHIP))
    {
        aiCurrentAIPlayer->supportTeam[new_team_num] = aitCreate(ResourceTeam);
        growSelectAddShip(&aiCurrentAIPlayer->supportTeam[new_team_num]->shipList, ship);
        aioCreateActiveSupport(aiCurrentAIPlayer->supportTeam[new_team_num], NULL, SUPPORT_MOTHERSHIP);
        aiCurrentAIPlayer->numSupportTeams++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : airAddNewShip
    Description : Adds newly assigned ships to the shipreserve structure
    Inputs      : newsel - selection of the new ships
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airAddNewShip(Ship *ship)
{
    growSelectAddShip(&aiCurrentAIPlayer->airResourceReserves, ship);

    if (ShipTypeIsWorker(ship->shiptype))
    {
        airAddedResourceCollector(ship);
    }
    else if (ship->shiptype == sCarrierSupport ||
             ship->shiptype == sMothershipSupport ||
             ship->shiptype == bMothershipSupport)
    {
        airAddedResearchShip(ship);
    }
    else if (ShipTypeIsProcessorType(ship->shiptype))
    {
        airAddedResourceController(ship);
    }
    else if (ShipTypeIsRepairShipType(ship->shiptype))
    {
        airAddedAdvanceSupportFrigate(ship);
    }
    else if (ShipTypeIsProximitySensorType(ship->shiptype))
    {
        airAddedProximitySensor(ship);
    }
    else if (ShipTypeIsMothershipOrCarrierType(ship->shiptype))
    {
        airAddedMothership(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : aiuCanBuildShipType
    Description : Returns TRUE if current AI player can build ships of shiptype
    Inputs      : shiptype
    Outputs     :
    Return      : Returns TRUE if current AI player can build ships of shiptype
----------------------------------------------------------------------------*/
bool32 aiuCanBuildShipType(ShipType shiptype,bool32 scriptteam)
{
    Ship *playerMothership = aiCurrentAIPlayer->player->PlayerMothership;
    ShipStaticInfo *teststatic;

    if (playerMothership == NULL)
    {
        return FALSE;
    }

    if (playerMothership->flags & SOF_Dead)
    {
        return FALSE;
    }

    if ((RacesAllowedForGivenShip[shiptype] & RaceToRaceBits(playerMothership->shiprace)) == 0)
    {
        return FALSE;
    }

    teststatic = GetShipStaticInfo(shiptype, playerMothership->shiprace);
    if (!bitTest(teststatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        return FALSE;
    }

    if ((teststatic->shipclass == CLASS_Resource) &&
        (!singlePlayerGame) &&
        (!bitTest(tpGameCreated.flag, MG_HarvestinEnabled)))
    {
        return FALSE;
    }

    if (scriptteam)
    {
        return TRUE;
    }

    return cmCanBuildShipType(playerMothership,shiptype,FALSE);
}


/*-----------------------------------------------------------------------------
    Name        : airCheckForDistressSignals
    Description : Checks all resource ships to make sure they aren't being attacked.
                  If they are, a distress signal is sent to the defense manager
    Inputs      : None
    Outputs     : Creates a selection of distressed ships, if there are any
    Return      : void
----------------------------------------------------------------------------*/
void airCheckForDistressSignals(void)
{
    udword i, numShips = aiCurrentAIPlayer->airResourceReserves.selection->numShips;
    ShipPtr ship;
    MaxSelection distressed_ships;

    distressed_ships.numShips = 0;

    for (i = 0; i < numShips; i++)
    {
        ship = aiCurrentAIPlayer->airResourceReserves.selection->ShipPtr[i];

        if (ship->recentAttacker)
        {
            selSelectionAddSingleShip(&distressed_ships, ship);
        }
    }

    if (distressed_ships.numShips)
    {
        aidSendDistressSignal((SelectCommand *)&distressed_ships);
    }
}


/*-----------------------------------------------------------------------------
    Name        : airDivideSupportTasks
    Description : Divides the support tasks among the support teams
                  Looks through the team array to find any ship with
    Inputs      : none
    Outputs     : Fiddles with the support team structures
    Return      : void
----------------------------------------------------------------------------*/
void airDivideSupportTasks(void)
{
    udword num_teams, num_support_teams, num_teams_per_support, num_teams_leftover,i;
    udword num_ASF_teams = 0;
    AITeam *teams[AIPLAYER_NUM_SUPPORTTEAMS];
    AITeam *team;

    num_teams         = aitFindNumTeamsWithFlag(TEAM_NEEDS_SUPPORT);
    num_support_teams = aiCurrentAIPlayer->numSupportTeams;

    if (!num_teams)
    {
        return;
    }

    //only get non-resourcecontroller support teams
    //later fix this to support more than one team needing support
    //also make AdvanceSupportFrigate support roving guarding teams
    for (i=0; i<num_support_teams;i++)
    {
        team = aiCurrentAIPlayer->supportTeam[i];
        if ((team->shipList.selection->numShips) &&
            ((team->shipList.selection->ShipPtr[0]->shiptype == kAdvanceSupportFrigate) ||
             (team->shipList.selection->ShipPtr[0]->shiptype == tAdvanceSupportFrigate)))
        {
            teams[num_ASF_teams++] = team;
        }
    }

    if (!num_ASF_teams)
    {
        return;
    }

    num_teams_per_support = num_teams / num_support_teams;
    num_teams_leftover    = num_teams % num_support_teams;

    team = aitFindNextTeamWithFlag(NULL, TEAM_NEEDS_SUPPORT);

    i = 0;

    while (team && (i < num_ASF_teams))
    {
        aitMakeTeamSupportShips(teams[i++], team->shipList.selection);
        team = aitFindNextTeamWithFlag(team, TEAM_NEEDS_SUPPORT);
    }

/*    if (!num_teams_leftover)
    {
        for (i=0;i<num_support_teams;i++)
        {
            for (j=0;j<num_teams_per_support;j++)
            {

            }
        }
    }*/
}


/*-----------------------------------------------------------------------------
    Name        : airProcessSpecialTeams
    Description : Processes any early ships to be built by the resource manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void airProcessSpecialTeams(void)
{
}


/*-----------------------------------------------------------------------------
    Name        : airDumbResourcerRequests
    Description : Uses the ResourcersToBuild variable to determine if a new resourcer needs to be built
    Inputs      :
    Outputs     :
    Return      : Whether or not a new resourcer has been requested at high priority
----------------------------------------------------------------------------*/
bool32 airDumbResourcerRequests(void)
{
    if (aiCurrentAIPlayer->ResourcersToBuild)
    {
        sdword totalRCollectors = aiCurrentAIPlayer->airNumRCollectors + aiCurrentAIPlayer->NumRCollectorsBeingBuilt;

        if (aiCurrentAIPlayer->ResourcersToBuild > totalRCollectors)
        {
            ShipType workerShipType;

            if (aiuCanBuildShipType(tResourceCollector, FALSE))
            {
                workerShipType = tResourceCollector;
            }
            else if (aiuCanBuildShipType(bWorker, FALSE))
            {
                workerShipType = bWorker;
            }
            else if (aiuCanBuildShipType(sWorker, FALSE))
            {
                workerShipType = sWorker;
            }
            else
            {
                return FALSE;
            }

            aifResourceManRequestsShipsCB(workerShipType, aiCurrentAIPlayer->ResourcersToBuild - totalRCollectors, REQUESTSHIPS_HIPRI);
            return TRUE;
        }
    }
    else
    {
        aiCurrentAIPlayer->ResourcersToBuild = randyrandombetween(RAN_AIPlayer, (2 + (udword)aiCurrentAIPlayer->aiplayerDifficultyLevel), (4 + (udword)aiCurrentAIPlayer->aiplayerDifficultyLevel));
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : airDumbResearchRequests
    Description : Uses the Researchers to build variable to determine of a new
                  research ship needs to be built
    Inputs      :
    Outputs     :
    Return      : Whether or not a new research ship has been requested at high priority
----------------------------------------------------------------------------*/
bool32 airDumbResearchRequests(bool32 *requestedResearch)
{
    *requestedResearch = FALSE;
    return FALSE;
}



/*-----------------------------------------------------------------------------
    Name        : airSmartResearchRequests
    Description : Uses the technology deficit variable to determine if a new
                  research ship needs to be built
    Inputs      :
    Outputs     :
    Return      : Whether or not a new research ship has been requested at high priority
----------------------------------------------------------------------------*/
bool32 airSmartResearchRequests(bool32 *requestedResearch)
{
    *requestedResearch = FALSE;

    if ((aiCurrentAIPlayer->player->PlayerMothership == NULL) ||
        (aiCurrentAIPlayer->player->PlayerMothership->shiptype != sMothership))
    {
        if ((ranRandomFnSimple(RAN_AIPlayer) % 101) < 5)
        {
            rmResearchTechForShip(aiCurrentAIPlayer->player, 28);
        }
        return FALSE;
    }

    if (aiCurrentAIPlayer->ResearchDelay)
    {
        aiCurrentAIPlayer->ResearchDelay--;
        return FALSE;
    }

    if (!univPlayerHasShipType(sMothershipDockingBay, aiCurrentAIPlayer->player))
    {
        aifResourceManRequestsShipsCB(sMothershipDockingBay, 1, REQUESTSHIPS_HIPRI);
        *requestedResearch = TRUE;
        aiCurrentAIPlayer->ResearchDelay = 100;
    }
    else if (!univPlayerHasShipType(sMothershipMicro, aiCurrentAIPlayer->player))
    {
        aifResourceManRequestsShipsCB(sMothershipMicro, 1, REQUESTSHIPS_HIPRI);
        *requestedResearch = TRUE;
        aiCurrentAIPlayer->ResearchDelay = 100;
    }
    else
    {
        switch (ranRandomFnSimple(RAN_AIPlayer) % 3)
        {
            case 0:
                if (!univPlayerHasShipType(sMothershipSpecial, aiCurrentAIPlayer->player))
                {
                    aifResourceManRequestsShipsCB(sMothershipSpecial, 1, REQUESTSHIPS_HIPRI);
                    *requestedResearch = TRUE;
                    aiCurrentAIPlayer->ResearchDelay = 100;
                }
                break;
            case 1:
                if (!univPlayerHasShipType(sMothershipWeapons, aiCurrentAIPlayer->player))
                {
                    aifResourceManRequestsShipsCB(sMothershipWeapons, 1, REQUESTSHIPS_HIPRI);
                    *requestedResearch = TRUE;
                    aiCurrentAIPlayer->ResearchDelay = 100;
                }
                break;
            case 2:
                if (!univPlayerHasShipType(sMothershipArmour, aiCurrentAIPlayer->player))
                {
                    aifResourceManRequestsShipsCB(sMothershipArmour, 1, REQUESTSHIPS_HIPRI);
                    *requestedResearch = TRUE;
                    aiCurrentAIPlayer->ResearchDelay = 100;
                }
                break;
        }
    }

    if ((aiCurrentAIPlayer->ResearchDelay != 100) &&
        ((ranRandomFnSimple(RAN_AIPlayer) % 101) < 5) &&
        !rmResearchTechForShip(aiCurrentAIPlayer->player, 28))
    {
        rmResearchTechForShip(aiCurrentAIPlayer->player, 6);
    }

    return FALSE;
}




/*-----------------------------------------------------------------------------
    Name        : airResourceManager
    Description : Manages all the resources and non-combat ships for the computer player
    Inputs      : orders - orders from the fleet manager
    Outputs     : sets resource collectors to collect resources and optimizes resource
                  controller and other non-combat ships
    Return      : requests for new ships
----------------------------------------------------------------------------*/
void airResourceManager(void)
{
    sdword cashGenerated;
    ShipType workerShipType;
    bool32 requestedSomething = FALSE, requestedResearch = FALSE;

    // Update continuously updated variables

    if (((aiCurrentAIPlayer->aiplayerFrameCount & UPDATE_RU_COUNT_RATE) == 0)||
        (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld == -1))
    {
        aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld = NumberOfEasilyAccesibleRUs(aiCurrentAIPlayer->player);
    }

    cashGenerated = aiCurrentAIPlayer->player->resourceUnits + aiCurrentAIPlayer->NumRUsSpentOnAttman + aiCurrentAIPlayer->NumRUsSpentOnDefman + aiCurrentAIPlayer->NumRUsSpentOnScriptman;

    //determine if we should build any research ships
    if (singlePlayerGame || (!singlePlayerGame && bitTest(tpGameCreated.flag,MG_ResearchEnabled)))
    {
        if (airSmartResearchRequests(&requestedResearch))
        {
            return;
        }
    }

    //determine if we should build any resource collectors
    if ((singlePlayerGame || (!singlePlayerGame && bitTest(tpGameCreated.flag,MG_HarvestinEnabled))) &&
        (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld > MIN_WORTHWHILE_RUs))
    {
        sdword totalRCollectors = aiCurrentAIPlayer->airNumRCollectors + aiCurrentAIPlayer->NumRCollectorsBeingBuilt;

        if (aiuResourceFeatureEnabled(AIR_SMART_COLLECTOR_REQUESTS))
        {
            if (totalRCollectors < MAX_RCOLLECTORS_TO_BUILD)
            {
                if (totalRCollectors < aiCurrentAIPlayer->NumRUDockPoints*MAX_RCOLLECTORS_PER_DOCK_POINT)
                {
                    if ((totalRCollectors == 0) || (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld > (totalRCollectors+1)*RUs_PER_RCOLLECTOR))
                    {
                        // see if we have enough cash to make it worthwhile.  (The more resource collectors we
                        // already have, the more cash we want before building a new one because the Resource
                        // Manager should always be providing an increasing amount of RU's).

                        // Note that when we say cash, we mean cashGenerated
                        if ((totalRCollectors == 0) ||      // if this is our first RCollector, we always want to build one
                            (cashGenerated > BUILD_RC_CASH_TABLE_BASE + BUILD_RC_CASH_TABLE_INC * (totalRCollectors-1)))
                        {
                            workerShipType = WorkerShipTypeForRace(aiCurrentAIPlayer->player->race);
                            if (aiuCanBuildShipType(tResourceCollector, FALSE))
                            {
                                workerShipType = tResourceCollector;
                            }
                            else if (aiuCanBuildShipType(bWorker, FALSE))
                            {
                                workerShipType = bWorker;
                            }

                            if (totalRCollectors == 0)
                            {
                                if (aiuCanBuildShipType(workerShipType, FALSE))
                                {
                                    aifResourceManRequestsShipsCB(workerShipType, 1, REQUESTSHIPS_HIPRI);
                                    return;
                                }
                            }
                            else
                            {
                                if (aiuCanBuildShipType(workerShipType, FALSE))
                                {
                                    aifResourceManRequestsShipsCB(workerShipType, 1, 0);
                                    requestedSomething = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (airDumbResourcerRequests())
            {
                //a resource collector has been requested at high priority
                return;
            }
        }

        //determine if we should build any resource controllers
        if (aiuResourceFeatureEnabled(AIR_RESOURCE_CONTROLLER_REQUESTS) &&
            (totalRCollectors >= MIN_RCOLLECTORS_FOR_CONTROLLER))
        {
            sdword totalRControllers = aiCurrentAIPlayer->airNumRControllers + aiCurrentAIPlayer->NumRControllersBeingBuilt;
            if ((totalRControllers == 0) && (!requestedResearch))
            {
                if (aiCurrentAIPlayer->airEasilyAccesibleRUsInWorld > (MIN_WORTHWHILE_RUs_FOR_CONTROLLER * universe.numPlayers))
                {
                    if (cashGenerated > BUILD_RCONTROLLER_CASH_BASE)
                    {
                        if ((aiCurrentAIPlayer->player->PlayerMothership != NULL) &&
                            (aiCurrentAIPlayer->player->PlayerMothership->shiptype == sMothership) &&
                            univPlayerHasShipType(sMothershipDockingBay, aiCurrentAIPlayer->player) &&
                            aiuCanBuildShipType(sProcessor, FALSE))
                        {
                            aifResourceManRequestsShipsCB(sProcessor, 1, 0);
                            return;
                        }

                        if (aiuCanBuildShipType(bProcessor, FALSE))
                        {
                            aifResourceManRequestsShipsCB(bProcessor, 1, 0);
                            return;
                        }
                    }
                }
            }
        }
    }

    if ((requestedSomething || !requestedResearch) &&
        (!singlePlayerGame) &&
        (cashGenerated > BUILD_ASF_CASH_BASE))
    {
        if (aiCurrentAIPlayer->player->supportUnitsUsed >=
            (udword)(aiCurrentAIPlayer->player->supportUnitsMax - BUILD_SUPPORT_CAP_CUSHION))
        {
            if (aiuCanBuildShipType(sCarrier, FALSE) &&
                (aiCurrentAIPlayer->player->PlayerMothership != NULL) &&
                (aiCurrentAIPlayer->player->PlayerMothership->shiptype == sMothership) &&
                univPlayerHasShipType(sMothershipDockingBay, aiCurrentAIPlayer->player) &&
                aiuUnitCapCanBuildShip(aiCurrentAIPlayer, sCarrier, 1))
            {
                aifResourceManRequestsShipsCB(sCarrier, 1, 0);
            }
            else if (aiuCanBuildShipType(bCarrier, FALSE) &&
                     aiuUnitCapCanBuildShip(aiCurrentAIPlayer, bCarrier, 1))
            {
                aifResourceManRequestsShipsCB(bCarrier, 1, 0);
            }
        }
    }

    if (aiCurrentAIPlayer->numSupportTeams)
    {
        airDivideSupportTasks();
    }

    //Distress signals only work with active guard,
    //so that feature has to be activated as well
    if (aiuResourceFeatureEnabled(AIR_RESOURCE_DISTRESS_SIGNALS))
    {
        airCheckForDistressSignals();
    }
}



#if 0  // Cat2: Binary airDivideSupportTasks; live implementation at AIResourceMan.c L602 (0x412D40)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x412D40
_DWORD *airDivideSupportTasks(void)
{
   unsigned int v0; // ebp
   _DWORD *result; // eax
   char *v2; // esi
   AIPlayerLevel *p_aiplayerDifficultyLevel; // edi
   Player *player; // ebx
   int v5; // edx
   _DWORD *v6; // esi
   unsigned int v7; // ebx
   char *v8; // edi
   _DWORD *v9; // [esp-8h] [ebp-48h]
   char v10[48]; // [esp+10h] [ebp-30h] BYREF

   v0 = 0;
   result = (_DWORD *)sub_417150(4096);
   if ( result && aiCurrentAIPlayer[151].player )
   {
     v2 = v10;
     p_aiplayerDifficultyLevel = &aiCurrentAIPlayer[151].aiplayerDifficultyLevel;
     player = aiCurrentAIPlayer[151].player;
     do
     {
       result = *(_DWORD **)(*p_aiplayerDifficultyLevel + 32);
       if ( *result )
       {
         if ( (result = (_DWORD *)result[2], v5 = result[173], v5 == 2) && result[172] == 60
           || v5 == 3 && result[172] == 74 )
         {
           *(AIPlayerLevel *)v2 = *p_aiplayerDifficultyLevel;
           ++v0;
           v2 += 4;
         }
       }
       ++p_aiplayerDifficultyLevel;
       player = (Player *)((char *)player - 1);
     }
     while ( player );
     if ( v0 )
     {
       result = (_DWORD *)sub_417180(0, 4096);
       v6 = result;
       v7 = 0;
       if ( result )
       {
         v8 = v10;
         do
         {
           if ( v7 >= v0 )
             break;
           v9 = *(_DWORD **)v8;
           ++v7;
           v8 += 4;
           aitMakeTeamSupportShips(v9, (_DWORD *)v6[8]);
           result = (_DWORD *)sub_417180((int)v6, 4096);
           v6 = result;
         }
         while ( result );
       }
     }
   }
   return result;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x412E10
signed int airDumbResourcerRequests(void)
{
   char *v0; // edi
   Player *player; // eax
   ShipRaceEnum race; // ecx
   Ship *PlayerMothership; // esi
   ShipStaticInfo2 *ShipStaticInfo; // eax
   Ship *v6; // esi
   ShipStaticInfo2 *v7; // eax
   Ship *v8; // esi
   ShipStaticInfo2 *v9; // eax

   if ( LOBYTE(aiCurrentAIPlayer[147].DefenseFeatures) )
   {
     v0 = (char *)aiCurrentAIPlayer[150].player + aiCurrentAIPlayer[118].ResourceFeatures;
     if ( LOBYTE(aiCurrentAIPlayer[147].DefenseFeatures) > (int)v0 )
     {
       player = aiCurrentAIPlayer->player;
       race = aiCurrentAIPlayer->player->race;
       if ( race == 3 )
       {
         PlayerMothership = player->PlayerMothership;
         if ( player->PlayerMothership )
         {
           if ( (PlayerMothership->flags & 0x800) == 0 && ((1 << PlayerMothership->shiprace) & asc_8B18C0[2]) != 0 )
           {
             ShipStaticInfo = GetShipStaticInfo(tResourceCollector);
             if ( (ShipStaticInfo->staticheader.infoFlags & 2) != 0
               && (ShipStaticInfo->shipclass != CLASS_Resource || singlePlayerGame || (tpGameCreated_flag & 0x20) != 0) )
             {
               if ( cmCanBuildShipType(PlayerMothership, tResourceCollector, 0) )
               {
                 aifResourceManRequestsShipsCB(0x58u, LOBYTE(aiCurrentAIPlayer[147].DefenseFeatures) - (_DWORD)v0, 1u);
                 return 1;
               }
             }
           }
         }
       }
       else if ( race )
       {
         if ( race == Race_Beast )
         {
           v8 = player->PlayerMothership;
           if ( player->PlayerMothership )
           {
             if ( (v8->flags & 0x800) == 0
               && ((unsigned __int16)(1 << v8->shiprace) & (unsigned __int16)word_8B1876) != 0 )
             {
               v9 = GetShipStaticInfo(bWorker);
               if ( (v9->staticheader.infoFlags & 2) != 0
                 && (v9->shipclass != CLASS_Resource || singlePlayerGame || (tpGameCreated_flag & 0x20) != 0) )
               {
                 if ( cmCanBuildShipType(v8, bWorker, 0) )
                 {
                   aifResourceManRequestsShipsCB(0x31u, LOBYTE(aiCurrentAIPlayer[147].DefenseFeatures) - (_DWORD)v0, 1u);
                   return 1;
                 }
               }
             }
           }
         }
       }
       else
       {
         v6 = player->PlayerMothership;
         if ( player->PlayerMothership )
         {
           if ( (v6->flags & 0x800) == 0 && ((1 << v6->shiprace) & (unsigned __int16)dword_8B1830) != 0 )
           {
             v7 = GetShipStaticInfo(sWorker);
             if ( (v7->staticheader.infoFlags & 2) != 0
               && (v7->shipclass != CLASS_Resource || singlePlayerGame || (tpGameCreated_flag & 0x20) != 0) )
             {
               if ( cmCanBuildShipType(v6, sWorker, 0) )
               {
                 aifResourceManRequestsShipsCB(0xEu, LOBYTE(aiCurrentAIPlayer[147].DefenseFeatures) - (_DWORD)v0, 1u);
                 return 1;
               }
             }
           }
         }
       }
     }
   }
   else
   {
     LOBYTE(aiCurrentAIPlayer[147].DefenseFeatures) = LOBYTE(aiCurrentAIPlayer->aiplayerDifficultyLevel)
                                                    + ranRandomFn(6) % 3
                                                    + 2;
   }
   return 0;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x413070
int airSmartResearchRequests(_DWORD *a1)
{
   Ship *PlayerMothership; // eax
   char v2; // al
   udword v4; // edx
   udword v5; // edx

   PlayerMothership = aiCurrentAIPlayer->player->PlayerMothership;
   if ( !PlayerMothership || PlayerMothership->shiptype != sMothership )
   {
     if ( ranRandomFn(0) % 0x65 < 5 )
       sub_4F3320((int)aiCurrentAIPlayer->player, 28);
     return 0;
   }
   v2 = BYTE1(aiCurrentAIPlayer[147].DefenseFeatures);
   if ( v2 )
   {
     BYTE1(aiCurrentAIPlayer[147].DefenseFeatures) = v2 - 1;
     return 0;
   }
   if ( !univPlayerHasShipType(sMothershipDockingBay, aiCurrentAIPlayer->player) )
   {
     aifResourceManRequestsShipsCB(0x18u, 1u, 1u);
 LABEL_19:
     *a1 = 1;
     BYTE1(aiCurrentAIPlayer[147].DefenseFeatures) = 100;
     goto LABEL_20;
   }
   if ( !univPlayerHasShipType(sMothershipMicro, aiCurrentAIPlayer->player) )
   {
     aifResourceManRequestsShipsCB(0x16u, 1u, 1u);
 LABEL_9:
     *a1 = 1;
     BYTE1(aiCurrentAIPlayer[147].DefenseFeatures) = 100;
     goto LABEL_20;
   }
   v4 = ranRandomFn(0) % 3;
   if ( v4 )
   {
     v5 = v4 - 1;
     if ( v5 )
     {
       if ( v5 == 1 && !univPlayerHasShipType(sMothershipArmour, aiCurrentAIPlayer->player) )
       {
         aifResourceManRequestsShipsCB(0x15u, 1u, 1u);
         goto LABEL_9;
       }
     }
     else if ( !univPlayerHasShipType(sMothershipWeapons, aiCurrentAIPlayer->player) )
     {
       aifResourceManRequestsShipsCB(0x17u, 1u, 1u);
       *a1 = 1;
       BYTE1(aiCurrentAIPlayer[147].DefenseFeatures) = 100;
     }
   }
   else if ( !univPlayerHasShipType(sMothershipSpecial, aiCurrentAIPlayer->player) )
   {
     aifResourceManRequestsShipsCB(0x13u, 1u, 1u);
     goto LABEL_19;
   }
 LABEL_20:
   if ( BYTE1(aiCurrentAIPlayer[147].DefenseFeatures) != 100
     && ranRandomFn(0) % 0x65 < 5
     && !sub_4F3320((int)aiCurrentAIPlayer->player, 28) )
   {
     sub_4F3320((int)aiCurrentAIPlayer->player, 6);
     return 0;
   }
   return 0;
 }

// ============================================================================
// Decompiled from cataclysm.exe @ 0x412CB0
bool32 aiuCanBuildShipType(ShipType shiptype, bool32 scriptteam)
{
Ship *PlayerMothership; // esi
ShipStaticInfo2 *ShipStaticInfo; // eax

PlayerMothership = aiCurrentAIPlayer->player->PlayerMothership;
if ( !PlayerMothership )
return 0;
if ( (PlayerMothership->flags & 0x800) != 0 )
return 0;
if ( ((unsigned __int16)(1 << PlayerMothership->shiprace) & (unsigned __int16)word_8B1814[shiptype]) == 0 )
return 0;
ShipStaticInfo = GetShipStaticInfo(shiptype);
if ( (ShipStaticInfo->staticheader.infoFlags & 2) == 0
|| ShipStaticInfo->shipclass == CLASS_Resource && !singlePlayerGame && (tpGameCreated_flag & 0x20) == 0 )
{
return 0;
}
if ( a2 )
return 1;
return cmCanBuildShipType(PlayerMothership, shiptype, 0);
}

#endif

#if 0  // Cat2: FindBestPlaceForAdvanceSupportFrigate live code differs from binary
/*=== FindBestPlaceForAdvanceSupportFrigate (0x4128C0) ===*/
int __cdecl FindBestPlaceForAdvanceSupportFrigate(float *a1)
{
Ship *PlayerMothership; // esi
int result; // eax
float v3; // [esp+8h] [ebp-8h]
float v4; // [esp+8h] [ebp-8h]
float v5; // [esp+Ch] [ebp-4h]
float v6; // [esp+Ch] [ebp-4h]
PlayerMothership = aiCurrentAIPlayer->player->PlayerMothership;
result = aiuFindEnemyMothership((int *)aiCurrentAIPlayer->player);
if ( !PlayerMothership )
return 0;
if ( result )
{
v3 = *(float *)(result + 92) - PlayerMothership->posinfo.position.y;
v5 = *(float *)(result + 96) - PlayerMothership->posinfo.position.z;
v4 = flt_74D344 * v3;
v6 = flt_74D344 * v5;
*a1 = (*(float *)(result + 88) - PlayerMothership->posinfo.position.x) * flt_74D344
+ PlayerMothership->posinfo.position.x;
a1[1] = v4 + PlayerMothership->posinfo.position.y;
a1[2] = v6 + PlayerMothership->posinfo.position.z;
return 1;
}
return result;
}
#endif

#if 0  // Cat2: airAddedResourceCollector live code differs from binary
/*=== airAddedResourceCollector (0x4126F0) ===*/
void __cdecl airAddedResourceCollector(Ship *ship)
{
  unsigned int new_team_num; // edi
  AIPlayer *v2; // ecx
  signed int ResourceFeatures; // eax
  struct AITeam *team; // esi
  struct Resource *resource; // esi
  SelectCommand selectone; // [esp+Ch] [ebp-18h] BYREF
  SelectCommand dockSelection; // [esp+18h] [ebp-Ch] BYREF

  new_team_num = (unsigned int)aiCurrentAIPlayer[151].player;
  ++aiCurrentAIPlayer[150].player;
  v2 = aiCurrentAIPlayer;
  ResourceFeatures = aiCurrentAIPlayer[118].ResourceFeatures;
  if ( ResourceFeatures > 0 )
  {
    aiCurrentAIPlayer[118].ResourceFeatures = ResourceFeatures - 1;
    v2 = aiCurrentAIPlayer;
  }
  if ( singlePlayerGame || (tpGameCreated_flag & 0x20) != 0 )
  {
    growSelectAddShip((int *)&v2[149].ResourceFeatures, (int)ship);
    if ( (aiCurrentAIPlayer->ResourceFeatures & 2) != 0 )
    {
      team = (struct AITeam *)aitFindNextTeamWithFlag(0, 8);
      if ( team )
      {
        while ( **((int **)team + 8) >= 3 )
        {
          team = (struct AITeam *)aitFindNextTeamWithFlag((int)team, 8);
          if ( !team )
            goto LABEL_12;
        }
        aifAddShipToTeam(team, ship);
      }
      else
      {
LABEL_12:
        *((_DWORD *)&aiCurrentAIPlayer[151].aiplayerDifficultyLevel + new_team_num) = aitCreate(2);
        team = (struct AITeam *)*((_DWORD *)&aiCurrentAIPlayer[151].aiplayerDifficultyLevel + new_team_num);
        *((_DWORD *)team + 1) |= 8u;
        growSelectAddShip(
          (int *)(*((_DWORD *)&aiCurrentAIPlayer[151].aiplayerDifficultyLevel + new_team_num) + 28),
          (int)ship);
        aioCreateResourcer(*((struct AITeam **)&aiCurrentAIPlayer[151].aiplayerDifficultyLevel + new_team_num));
        ++aiCurrentAIPlayer[151].player;
      }
      if ( (aiCurrentAIPlayer->ResourceFeatures & 0x1000) != 0 )
        aiuWrapSetTactics(*((SelectCommand **)team + 8), Aggressive);
      else
        aiuWrapSetTactics(*((SelectCommand **)team + 8), Evasive);
    }
    else
    {
      resource = (struct Resource *)univFindNearestResource((int)ship, 0.0, 0, 0);
      selectone.numShips = 1;
      selectone.ShipPtr[0] = ship;
      if ( (aiCurrentAIPlayer->ResourceFeatures & 0x1000) != 0 )
        aiuWrapSetTactics(&selectone, Aggressive);
      else
        aiuWrapSetTactics(&selectone, Evasive);
      aiuWrapCollectResource(&selectone, resource);
    }
  }
  else
  {
    dockSelection.ShipPtr[0] = ship;
    dockSelection.numShips = 1;
    clWrapDock(&universe.mainCommandLayer, &dockSelection, 7, 0);
  }
}
#endif
