/*=============================================================================
    Name    : AIFleetMan
    Purpose : Controls construction of ships requested from combatman, defenseman, resourceman

    Created 5/28/1998 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "universe.h"
#include "AIPlayer.h"
#include "AIFleetMan.h"
#include "AIResourceMan.h"
#include "AIAttackMan.h"
#include "AIDefenseMan.h"
#include "AITeam.h"
#include "AIUtilities.h"
#include "AIVar.h"
#include "CommandWrap.h"
#include "select.h"
#include "objtypes.h"
#include "researchapi.h"
#include "SinglePlayer.h"
#include "MultiplayerGame.h"
#include "alliance.h"
#include "randy.h"

#define AIF_TECHNOLOGYDEFICIT_RESOURCE_MASK 0x00FF0000u

Player *aifFindEnemyOf(Player *player)
{
    uword i;
    Player *enemy;

    for (i=0;i<universe.numPlayers;i++)
    {
        enemy = &universe.players[i];
        if (enemy != player)
        {
            return enemy;
        }
    }

    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : aifInit
    Description : initialize resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aifInit(struct AIPlayer *aiplayer)
{
    Player *player;

    *(udword *)&aiplayer->ResourcersToBuild = 1;
    aiplayer->firstTurn = TRUE;
    player = aiplayer->player;
    aiplayer->ResearchDelay = 0;
    aiplayer->AICreator = player->PlayerMothership;
    aiplayer->NumRUsSpentOnResourceman = (sdword)player->PlayerMothership;

    if (aiplayer->aiplayerDifficultyLevel == AI_ADVANCED &&
        (singlePlayerGame || bitTest(tpGameCreated.flag, MG_Hyperspace)))
    {
        aiplayer->ResourceFeatures |= AIF_HYPERSPACING;
    }
}

/*-----------------------------------------------------------------------------
    Name        : airClose
    Description : Closes resource manager stuff in aiplayer
    Inputs      : aiplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void aifClose(struct AIPlayer *aiplayer)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : aifShipDied
    Description : Removes a dead ship from the Fleet Command's internal structures
    Inputs      : aiplayer, ship - the ship that died
    Outputs     :
    Return      : TRUE if the ship was found and removed
----------------------------------------------------------------------------*/
bool32 aifShipDied(struct AIPlayer *aiplayer,ShipPtr ship)
{
    bool32 return_value = FALSE;

    if (ship->playerowner == aiplayer->player &&
        ShipTypeIsMothershipOrCarrierType(ship->shiptype))
    {
        RequestShips *request;
        Node *tempnode;

        //could be a creator ship
        if (ship == aiplayer->AICreator)
        {
            aiplayer->AICreator = NULL;
            return_value = TRUE;
        }
        else if (ship == aiplayer->ScriptCreator)
        {
            aiplayer->ScriptCreator = NULL;
            return_value = TRUE;
        }

        //now go through all the requests to make sure
        //this ship isn't referred to as a creator

        //check the resource requests
        if (aiplayer->ResourceManRequestShips.creator == ship)
        {
            aiplayer->ResourceManRequestShips.creator = NULL;
            return_value = TRUE;
        }

        //check the attack requests
        if (aiplayer->AttackManRequestShipsQ.head != NULL)
        {
            tempnode = aiplayer->AttackManRequestShipsQ.head;

            while (tempnode != NULL)
            {
                request = (RequestShips *)listGetStructOfNode(tempnode);

                if (request->creator == ship)
                {
                    request->creator = NULL;
                    return_value     = TRUE;
                }
                tempnode = tempnode->next;
            }
        }
        //check the defense requests
        if (aiplayer->DefenseManRequestShipsQ.head != NULL)
        {
            tempnode = aiplayer->DefenseManRequestShipsQ.head;

            while (tempnode != NULL)
            {
                request = (RequestShips *)listGetStructOfNode(tempnode);

                if (request->creator == ship)
                {
                    request->creator = NULL;
                    return_value     = TRUE;
                }
                tempnode = tempnode->next;
            }
        }
        //check the script requests
        if (aiplayer->ScriptManRequestShipsQ.head != NULL)
        {
            tempnode = aiplayer->ScriptManRequestShipsQ.head;

            while (tempnode != NULL)
            {
                request = (RequestShips *)listGetStructOfNode(tempnode);

                if (request->creator == ship)
                {
                    request->creator = NULL;
                    return_value     = TRUE;
                }
                tempnode = tempnode->next;
            }
        }
    }

    return return_value;
}

#define AIF_TECHNOLOGYDEFICIT_RESOURCE_MASK 0x00FF0000u
#define AIF_TECHNOLOGYDEFICIT_RESOURCE_BYTE(aiplayer) ((aiplayer)->TechnologyDeficitResourceFlag)
#define AIF_TECHNOLOGYDEFICIT_RESEARCH_DELAY(aiplayer) ((aiplayer)->TechnologyDeficitResearchDelay)
#define AIF_SCRIPTMAN_BUILT_TAIL_DWORD(aiplayer) (*(udword *)((ubyte *)(aiplayer) + offsetof(AIPlayer, ScriptManShipsBeingBuilt) + TOTAL_NUM_SHIPS - 3))
#define AIF_PREREQ_TECH_ENTRY_BASE TOTAL_NUM_SHIPS

sdword *rmBuildUnresearchedTechMask(sdword *dest, sdword *source);

static bool32 aifGetBuildableRequestedShipStatic(Player *player, ShipRace race, ShipType shiptype, ShipStaticInfo **shipstatic)
{
    ShipStaticInfo *staticInfo;

    dbgAssert(shipstatic != NULL);

    *shipstatic = NULL;
    if (!(RacesAllowedForGivenShip[shiptype] & RaceToRaceBits(race)))
    {
        return FALSE;
    }

    staticInfo = GetShipStaticInfo(shiptype, race);
    if (!bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
    {
        return FALSE;
    }

    if (race != RACE_Beast &&
        staticInfo->externalBuild &&
        staticInfo->shipclass != CLASS_Component &&
        !univPlayerHasShipType(sMothershipDockingBay, player))
    {
        return FALSE;
    }

    *shipstatic = staticInfo;
    return TRUE;
}

static bool32 aifResearchMaskHasEntries(const sdword *mask)
{
    sdword techIndex;

    for (techIndex = 0; techIndex < NumTechnologies; techIndex++)
    {
        if (mask[41] == 2 || mask[techIndex] == 1)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static bool32 aifPrereqMaskHasMissingEntries(const sdword *mask)
{
    sdword techIndex;

    if (mask[41] == 2)
    {
        return TRUE;
    }

    for (techIndex = 0; techIndex < NumTechnologies; techIndex++)
    {
        if (mask[techIndex] > 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static bool32 aifTryResearchForShipMask(Player *player, sdword *shipMaskBase)
{
    sdword shipMask[42];
    sdword accumulatedPrereqs[42];
    sdword unresearchedMask[42];
    sdword techPrereqMask[42];
    sdword techIndex;
    sdword freeLab;

    memcpy(shipMask, shipMaskBase, sizeof(shipMask));
    memset(accumulatedPrereqs, 0, sizeof(accumulatedPrereqs));

    for (techIndex = 0; techIndex < NumTechnologies; techIndex++)
    {
        if (shipMask[41] == 2 || shipMask[techIndex] == 1)
        {
            rmAccumulatePrerequisitesRecursive(accumulatedPrereqs, player, techIndex);
            rmOrPrereqMasks(shipMask, shipMask, accumulatedPrereqs);
        }
    }

    rmBuildUnresearchedTechMask(unresearchedMask, rmPlayerCurrentPrereqMask(player));
    rmPreRequisitesAnd(shipMask, shipMask, unresearchedMask);
    if (!aifResearchMaskHasEntries(shipMask))
    {
        return FALSE;
    }

    for (techIndex = 0; techIndex < NumTechnologies; techIndex++)
    {
        if (shipMask[41] != 2 && shipMask[techIndex] != 1)
        {
            continue;
        }

        if (player->race == RACE_Beast)
        {
            memset(unresearchedMask, 0, sizeof(unresearchedMask));
        }
        else
        {
            sdword prereqIndex;

            for (prereqIndex = 0; prereqIndex < NumTechnologies; prereqIndex++)
            {
                unresearchedMask[prereqIndex] = (rmPlayerCurrentPrereqMask(player)[prereqIndex] == 0);
            }
            unresearchedMask[41] = 0;
        }

        rmPreRequisitesAnd(
            techPrereqMask,
            player->discoveredShipPrereqs[AIF_PREREQ_TECH_ENTRY_BASE + techIndex].value,
            unresearchedMask);

        if (!aifPrereqMaskHasMissingEntries(techPrereqMask))
        {
            freeLab = rmFindFreeLab(player, techIndex);
            if (freeLab != -1)
            {
                clWrapResearch(0, player->playerIndex, freeLab, techIndex);
                return TRUE;
            }
        }
    }

    return FALSE;
}

static bool32 aifResearchForTeamBuild(Player *player, ShipType shiptype)
{
    if (player->race == RACE_Beast &&
        player->cataPreFogUnknown2.beastCanBuildByShipType[shiptype] == 0)
    {
        return FALSE;
    }

    if (aifTryResearchForShipMask(player, player->discoveredShipPrereqs[shiptype].value))
    {
        return TRUE;
    }

    return aifTryResearchForShipMask(player, player->referenceShipPrereqs[shiptype].value);
}

static bool32 aifValidateBuildRequest(RequestShips **request, Player *player, ShipRace race, ubyte researchDelay, bool32 *stopFurtherTeams)
{
    ShipStaticInfo *shipstatic;
    RequestShips *candidate;

    candidate = *request;
    if (candidate == NULL || singlePlayerGame)
    {
        return candidate != NULL;
    }

    if (player->race == RACE_Beast &&
        cmGetShipTypeCount(player, candidate->shiptype) == 0)
    {
        *request = NULL;
        return FALSE;
    }

    shipstatic = GetShipStaticInfo(candidate->shiptype, race);
    if (*(sdword *)&shipstatic->canBuildShips != 10 &&
        !cmCanBuildShipType(player->PlayerMothership, candidate->shiptype, FALSE))
    {
        *request = NULL;
        return FALSE;
    }

    if (rmCountBuildPrerequisites(player, candidate->shiptype))
    {
        if (researchDelay == 0)
        {
            aifResearchForTeamBuild(player, candidate->shiptype);
        }
        *request = NULL;
        return FALSE;
    }

    if (rmTechRequiredForShip(player, candidate->shiptype) &&
        researchDelay == 0)
    {
        aifResearchForTeamBuild(player, candidate->shiptype);
    }

    if (*request != NULL &&
        *(sdword *)&shipstatic->canBuildShips != 10 &&
        shipstatic->rotateToRetaliate)
    {
        *stopFurtherTeams = TRUE;
    }

    return *request != NULL;
}

void BuildShip(ShipStaticInfo *shipstatic,Player *player,sdword *incRUs,sdword *decRUs, ShipPtr creator)
{
#if(0)	
	Node *shipnode;
	Ship *ship;
	sdword shipcount,classcount;
	CommandToDo *command;
	ShipStaticInfo *tempstat;
#endif

    aiplayerLog((player->playerIndex,"Building ship %s; Total %i",ShipTypeToStr(shipstatic->shiptype),player->shiptotals[shipstatic->shiptype]));

	// unit caps checking code
#if(0)	
	shipnode = universe.ShipList.head;
	shipcount = 0;
	classcount = 0;
	while(shipnode != NULL)
	{
		ship = (Ship *)listGetStructOfNode(shipnode);

        if((ship->playerowner->playerIndex == player->playerIndex) &&
		   (!bitTest(ship->flags, SOF_Dead)))
		{
		  if(ship->shiptype == shipstatic->shiptype)
			{
				shipcount++;
			}
			if(ship->staticinfo->shipclass == shipstatic->shipclass)
			{
				classcount++;
			}
		}
		
		shipnode = shipnode->next;
	}

	shipnode = universe.mainCommandLayer.todolist.head;
	while(shipnode != NULL)
	{
	
		command = (CommandToDo *)listGetStructOfNode(shipnode);
	
		if(command->ordertype.order == COMMAND_BUILDINGSHIP)
		{
			if(command->buildingship.playerIndex == player->playerIndex)
			{
				if(command->buildingship.shipType == shipstatic->shiptype)
				{
					shipcount++;
				}

				tempstat = GetShipStaticInfo(command->buildingship.shipType, player->race);
				if(tempstat->shipclass == shipstatic->shipclass)
				{
					classcount++;
				}
			}
		}
		shipnode = shipnode->next;
	}
	if((cdLimitCaps[shipstatic->shiptype] != -1) && (shipcount+1 > cdLimitCaps[shipstatic->shiptype]))
	{
		_asm nop;
	}
	if(classcount+1 > cdClassCaps[shipstatic->shipclass])	
	{
		_asm nop;
	}
#endif

    switch (shipstatic->shiptype)
    {
        case sWorker:
        case bWorker:
        case tResourceCollector:
            aiCurrentAIPlayer->NumRCollectorsBeingBuilt++;
            break;

        case sProcessor:
        case bProcessor:
            aiCurrentAIPlayer->NumRControllersBeingBuilt++;
            break;

        case kAdvanceSupportFrigate:
        case tAdvanceSupportFrigate:
            aiCurrentAIPlayer->NumASFBeingBuilt++;
            break;

        case sCarrierSupport:
        case sMothershipSupport:
        case bMothershipSupport:
            aiCurrentAIPlayer->NumSupportSectionBeingBuilt++;
            break;
    }

    if (!creator)
    {
        creator = player->PlayerMothership;
    }

    if (creator &&
        (singlePlayerGame ||
         ShipStaticInfoGetSupportUnits(shipstatic) == 0 ||
         player->supportUnitsUsed < player->supportUnitsMax))
    {
        if (shipstatic->externalBuild)
        {
            clWrapCreateShip(&universe.mainCommandLayer, shipstatic->shiptype, creator->shiprace, player->playerIndex, creator);
        }
        else
        {
            clWrapBuildShip(&universe.mainCommandLayer, shipstatic->shiptype, creator->shiprace, player->playerIndex, creator);
        }

        *incRUs += shipstatic->buildCost;
        *decRUs -= shipstatic->buildCost;
    }
}

ShipStaticInfo *GetSubstituteShipStatic(ShipRace race)
{
    ShipStaticInfo *shipstatic = NULL;

    switch (race)
    {
        case RACE_Sect:
        case RACE_Beast:
            shipstatic = GetShipStaticInfo(sRecon,race);
            break;

        case P1:
            shipstatic = GetShipStaticInfo(P1Fighter,race);
            break;
        case RACE_Taiidan:
            shipstatic = GetShipStaticInfo(tStandardFrigate,race);
            break;

        default:
            dbgAssert(FALSE);
    }

    if (shipstatic)
    {
        if (!bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
        {
            shipstatic = NULL;
        }
    }

    return shipstatic;
}

/*-----------------------------------------------------------------------------
    Name        : aifBuildRequestedShips
    Description : Decides which build request to process depending on
                  which is more important - attack, defense, or resources
    Inputs      :
    Outputs     :
    Return      : returns the chosen request
----------------------------------------------------------------------------*/
void aifBuildRequestedShips(RequestShips *scriptrequest,RequestShips *attrequest,RequestShips *defrequest,RequestShips *resourcerequest)
{
    ShipStaticInfo *scriptshipstatic = NULL;
    sdword scriptnumships = 0;
    sdword scripttotalcost = 0;

    ShipStaticInfo *attshipstatic = NULL;
    sdword attnumships = 0;
    sdword atttotalcost = 0;

    ShipStaticInfo *defshipstatic = NULL;
    sdword defnumships = 0;
    sdword deftotalcost = 0;

    ShipStaticInfo *resourceshipstatic = NULL;
    sdword resourcenumships = 0;
    sdword resourcetotalcost = 0;

    Player *player = aiCurrentAIPlayer->player;
    Ship *playerMothership = player->PlayerMothership;
    sdword i;
    sdword RUsLeft = player->resourceUnits;
    ShipRace race;

    if (playerMothership == NULL || bitTest(playerMothership->flags, SOF_Dead))
    {
        return;     // can't build anything, return
    }

    race = playerMothership->shiprace;

    if ((scriptrequest != NULL) && (scriptrequest->num_ships > 0))
    {
        if (aifGetBuildableRequestedShipStatic(player, race, scriptrequest->shiptype, &scriptshipstatic))
        {
            scriptnumships = (sdword)scriptrequest->num_ships;
            scripttotalcost = scriptnumships * scriptshipstatic->buildCost;
        }
        else
        {
            scriptrequest->num_ships = -1;
        }
    }

    if ((attrequest != NULL) && (attrequest->num_ships > 0))
    {
        if (aifGetBuildableRequestedShipStatic(player, race, attrequest->shiptype, &attshipstatic))
        {
            attnumships = (sdword)attrequest->num_ships;
            atttotalcost = attnumships * attshipstatic->buildCost;
        }
        else
        {
            attrequest->num_ships = -1;
        }
    }

    if ((defrequest != NULL) && (defrequest->num_ships > 0))
    {
        if (aifGetBuildableRequestedShipStatic(player, race, defrequest->shiptype, &defshipstatic))
        {
            defnumships = (sdword)defrequest->num_ships;
            deftotalcost = defnumships * defshipstatic->buildCost;
        }
        else
        {
            defrequest->num_ships = -1;
        }
    }

    if ((resourcerequest != NULL) && (resourcerequest->num_ships > 0))
    {
        if (resourcerequest->shiptype == sMothershipSupport && playerMothership->shiptype != sMothership)
        {
            resourcerequest->shiptype = sCarrierSupport;
            resourcerequest->num_ships = -1;
            aiCurrentAIPlayer->TechnologyDeficit &= ~AIF_TECHNOLOGYDEFICIT_RESOURCE_MASK;
        }
        else if (resourcerequest->shiptype == bMothershipSupport && playerMothership->shiptype != bMothership)
        {
            resourcerequest->num_ships = -1;
            aiCurrentAIPlayer->TechnologyDeficit &= ~AIF_TECHNOLOGYDEFICIT_RESOURCE_MASK;
        }
        else if (aifGetBuildableRequestedShipStatic(player, race, resourcerequest->shiptype, &resourceshipstatic))
        {
            resourcenumships = (sdword)resourcerequest->num_ships;
            resourcetotalcost = resourcenumships * resourceshipstatic->buildCost;
        }
        else
        {
            resourcerequest->num_ships = -1;
            aiCurrentAIPlayer->TechnologyDeficit &= ~AIF_TECHNOLOGYDEFICIT_RESOURCE_MASK;
        }
    }

    if ((scripttotalcost + resourcetotalcost + atttotalcost + deftotalcost) <= RUsLeft)
    {
        // we can build everything, so let's do that

        // build everything resourceman wants
        for (i=0;i<resourcenumships;i++)
        {
            BuildShip(resourceshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnResourceman,&RUsLeft,resourcerequest->creator);
        }

        // build everything scriptman wants
        for (i=0;i<scriptnumships;i++)
        {
            BuildShip(scriptshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnScriptman,&RUsLeft,scriptrequest->creator);
            scriptrequest->num_ships--;
        }

        // build everything attman wants
        for (i=0;i<attnumships;i++)
        {
            BuildShip(attshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnAttman,&RUsLeft,attrequest->creator);
            attrequest->num_ships--;
        }

        // build everything defman wants
        for (i=0;i<defnumships;i++)
        {
            BuildShip(defshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnDefman,&RUsLeft,defrequest->creator);
            defrequest->num_ships--;
        }

        dbgAssert(RUsLeft >= 0);
    }
    else
    {
        // now choose who gets to go first, second, and third

        real32 totalRUsSpent = (real32) (aiCurrentAIPlayer->NumRUsSpentOnAttman +
                                  aiCurrentAIPlayer->NumRUsSpentOnDefman +
                                  aiCurrentAIPlayer->NumRUsSpentOnResourceman);
        real32 attmanbudgetratio = ((real32)aiCurrentAIPlayer->NumRUsSpentOnAttman) / totalRUsSpent;
        real32 defmanbudgetratio = ((real32)aiCurrentAIPlayer->NumRUsSpentOnDefman) / totalRUsSpent;
        real32 resourcemanbudgetratio = ((real32)aiCurrentAIPlayer->NumRUsSpentOnResourceman) / totalRUsSpent;

        real32 attmanbudgetoverflow = attmanbudgetratio / ATTMAN_BUILDPRIORITY_RATIO;
        real32 defmanbudgetoverflow = defmanbudgetratio / DEFMAN_BUILDPRIORITY_RATIO;
        real32 resourcemanbudgetoverflow = resourcemanbudgetratio / RESMAN_BUILDPRIORITY_RATIO;

        if ((resourcenumships) && (resourcerequest->priority & REQUESTSHIPS_HIPRI))
        {
            resourcemanbudgetoverflow = -1.0f;
        }

        if ((attnumships) && (attrequest->priority & REQUESTSHIPS_HIPRI))
        {
            attmanbudgetoverflow = -1.0f;
        }

        if ((defnumships) && (defrequest->priority & REQUESTSHIPS_HIPRI))
        {
            defmanbudgetoverflow = -1.0f;
        }

        if (attnumships == 0) attmanbudgetoverflow = REALlyBig;
        if (defnumships == 0) defmanbudgetoverflow = REALlyBig;
        if (resourcenumships == 0) resourcemanbudgetoverflow = REALlyBig;

//#define buildscriptships   \

//#define buildattships   \

//#define builddefships   \

//#define buildresships   \

        if (scriptnumships)
        {
			for (i=0;(i<scriptnumships) && (RUsLeft >= scriptshipstatic->buildCost);i++)                              \
			{                                                                                                   \
				BuildShip(scriptshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnScriptman,&RUsLeft,scriptrequest->creator);\
				scriptrequest->num_ships--;                                                                        \
			}
//            buildscriptships
            return;
        }

        if (attmanbudgetoverflow <= defmanbudgetoverflow)
        {
            // attackman goes before defenseman
            if (attmanbudgetoverflow <= resourcemanbudgetoverflow)
            {
                if (defmanbudgetoverflow <= resourcemanbudgetoverflow)
                {
//                    buildattships
					for (i=0;(i<attnumships) && (RUsLeft >= attshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(attshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnAttman,&RUsLeft,attrequest->creator); \
						attrequest->num_ships--;                                                                        \
					}
//                    builddefships
//                    buildresships
                }
                else
                {
//                    buildattships
					for (i=0;(i<attnumships) && (RUsLeft >= attshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(attshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnAttman,&RUsLeft,attrequest->creator); \
						attrequest->num_ships--;                                                                        \
					}
//                    buildresships
//                    builddefships
                }
            }
            else
            {
				for (i=0;(i<resourcenumships) && (RUsLeft >= resourceshipstatic->buildCost);i++)                    \
				{                                                                                                   \
					BuildShip(resourceshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnResourceman,&RUsLeft,resourcerequest->creator);\
				}
//                buildresships
//                buildattships
//                builddefships
            }
        }
        else
        {
            // defenseman goes before attackman
            if (defmanbudgetoverflow <= resourcemanbudgetoverflow)
            {
                if (attmanbudgetoverflow <= resourcemanbudgetoverflow)
                {
					for (i=0;(i<defnumships) && (RUsLeft >= defshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(defshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnDefman,&RUsLeft,defrequest->creator); \
						defrequest->num_ships--;                                                                        \
					}
//                    builddefships
//                    buildattships
//                    buildresships
                }
                else
                {
					for (i=0;(i<defnumships) && (RUsLeft >= defshipstatic->buildCost);i++)                              \
					{                                                                                                   \
						BuildShip(defshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnDefman,&RUsLeft,defrequest->creator); \
						defrequest->num_ships--;                                                                        \
					}
//                    builddefships
//                    buildresships
//                    buildattships
                }
            }
            else
            {
				for (i=0;(i<resourcenumships) && (RUsLeft >= resourceshipstatic->buildCost);i++)                    \
				{                                                                                                   \
					BuildShip(resourceshipstatic,player,&aiCurrentAIPlayer->NumRUsSpentOnResourceman,&RUsLeft,resourcerequest->creator);\
				}
//                buildresships
//                builddefships
//                buildattships
            }
        }
    }

    return;
}

void aifAttackManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    // add requests to Q
    RequestShips *requestShips = memAlloc(sizeof(RequestShips),"attackreqships",0);

    dbgAssert(number > 0);

    requestShips->shiptype  = shiptype;
    requestShips->num_ships = number;
    requestShips->priority  = priority;
    requestShips->creator   = aiCurrentAIPlayer->AICreator;

    if (priority & REQUESTSHIPS_HIPRI)
    {
        listAddNodeBeginning(&aiCurrentAIPlayer->AttackManRequestShipsQ,&requestShips->node,requestShips);
    }
    else
    {
        listAddNode(&aiCurrentAIPlayer->AttackManRequestShipsQ,&requestShips->node,requestShips);
    }
}

void aifDefenseManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    // add requests to Q
    RequestShips *requestShips = memAlloc(sizeof(RequestShips),"defreqships",0);

    dbgAssert(number > 0);

    requestShips->shiptype  = shiptype;
    requestShips->num_ships = number;
    requestShips->priority  = priority;
    requestShips->creator   = aiCurrentAIPlayer->AICreator;

    if (priority & REQUESTSHIPS_HIPRI)
    {
        listAddNodeBeginning(&aiCurrentAIPlayer->DefenseManRequestShipsQ,&requestShips->node,requestShips);
    }
    else
    {
        listAddNode(&aiCurrentAIPlayer->DefenseManRequestShipsQ,&requestShips->node,requestShips);
    }
}

void aifScriptManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    // add requests to Q
    RequestShips *requestShips = memAlloc(sizeof(RequestShips),"defreqships",0);

    dbgAssert(number > 0);

    requestShips->shiptype  = shiptype;
    requestShips->num_ships = number;
    requestShips->priority  = priority;
    requestShips->creator   = aiCurrentAIPlayer->ScriptCreator;

    if (priority & REQUESTSHIPS_HIPRI)
    {
        listAddNodeBeginning(&aiCurrentAIPlayer->ScriptManRequestShipsQ,&requestShips->node,requestShips);
    }
    else
    {
        listAddNode(&aiCurrentAIPlayer->ScriptManRequestShipsQ,&requestShips->node,requestShips);
    }
}

void aifResourceManRequestsShipsCB(ShipType shiptype,sdword number,sdword priority)
{
    dbgAssert(number > 0);

    aiCurrentAIPlayer->ResourceManRequestShips.shiptype  = shiptype;
    aiCurrentAIPlayer->ResourceManRequestShips.num_ships = number;
    aiCurrentAIPlayer->ResourceManRequestShips.priority  = priority;
    aiCurrentAIPlayer->ResourceManRequestShips.creator   = aiCurrentAIPlayer->AICreator;
}

#define constructTeamWaiting(twaiting,stype,nships,tm,dSetVar)   \
            twaiting = memAlloc(sizeof(TeamWaitingForTheseShips),"teamwaitingships",0);     \
            twaiting##->shiptype = stype;                                                   \
            twaiting##->num_ships = nships;                                                 \
            twaiting##->team = tm;                                                          \
            dbgAssert(strlen(dSetVar) <= AIVAR_LABEL_MAX_LENGTH);                           \
            strcpy(twaiting##->doneSetVarStr,dSetVar)

void aifTeamRequestsShipsCB(ShipType shiptype,sdword number,struct AITeam *team,char *doneSetVar, sdword priority)
{
    TeamWaitingForTheseShips *teamWaiting;

	aiplayerLog((aiIndex, "%x Requesting %i %ss", team, number, ShipTypeToStr(shiptype)));

    switch (team->teamType)
    {
        case AttackTeam:
            // put request in Q under attackman
            aifAttackManRequestsShipsCB(shiptype,number,priority);
            // add to AttackManTeamWaitingForShipsQ
            constructTeamWaiting(teamWaiting,shiptype,number,team,doneSetVar);
            if (priority & REQUESTSHIPS_HIPRI)
            {
                listAddNodeBeginning(&aiCurrentAIPlayer->AttackManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            else
            {
                listAddNode(&aiCurrentAIPlayer->AttackManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            break;

        case DefenseTeam:
            // put request in Q under defenseman
            aifDefenseManRequestsShipsCB(shiptype,number,priority);
            // add to DefenseManTeamWaitingForShipsQ
            constructTeamWaiting(teamWaiting,shiptype,number,team,doneSetVar);
            if (priority & REQUESTSHIPS_HIPRI)
            {
                listAddNodeBeginning(&aiCurrentAIPlayer->DefenseManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            else
            {
                listAddNode(&aiCurrentAIPlayer->DefenseManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            break;

        case ScriptTeam:
            // put request in Q under scriptman
            aifScriptManRequestsShipsCB(shiptype,number,priority);
            // add to DefenseManTeamWaitingForShipsQ
            constructTeamWaiting(teamWaiting,shiptype,number,team,doneSetVar);
            if (priority & REQUESTSHIPS_HIPRI)
            {
                listAddNodeBeginning(&aiCurrentAIPlayer->ScriptManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            else
            {
                listAddNode(&aiCurrentAIPlayer->ScriptManTeamsWaitingForShipsQ,&teamWaiting->node,teamWaiting);
            }
            break;

        case ResourceTeam:
        default:
            dbgAssert(FALSE);       // not supported yet
            break;
    }
}

void RemoveTeamFromTeamWaitingQ(LinkedList *TeamWaitingQ,AITeam *team)
{
    Node *node = TeamWaitingQ->head;
    Node *nextnode;
    TeamWaitingForTheseShips *teamWaiting;

    while (node != NULL)
    {
        teamWaiting = (TeamWaitingForTheseShips *)listGetStructOfNode(node);
        nextnode = node->next;

        if (teamWaiting->team == team)
        {
            listDeleteNode(node);
        }

        node = nextnode;
    }
}

void aifTeamDied(struct AIPlayer *aiplayer,struct AITeam *team, bool32 removeAllReferencesToTeam)
{
    sdword i, j;
    MsgQueue *msgQP;

    RemoveTeamFromTeamWaitingQ(&aiplayer->AttackManTeamsWaitingForShipsQ,team);
    RemoveTeamFromTeamWaitingQ(&aiplayer->DefenseManTeamsWaitingForShipsQ,team);
    RemoveTeamFromTeamWaitingQ(&aiplayer->ScriptManTeamsWaitingForShipsQ,team);

    if (!removeAllReferencesToTeam)
    {
        //don't do anything else
        return;
    }

    // remove any messages from team that may still be queued up
    for (i = 0; i < aiCurrentAIPlayer->teamsUsed; ++i)
    {
        msgQP = aiCurrentAIPlayer->teams[i]->msgQueue;
        if (!msgQP) continue;
        aitQueueRemoveSenderReferences(aiCurrentAIPlayer->teams[i], team);
    }

    if (team->teamType == AttackTeam)
    {
        aiaTeamDied(aiplayer,team);
    }
    else if (team->teamType == DefenseTeam)
    {
        aidTeamDied(aiplayer,team);
    }
    else if (team->teamType == ResourceTeam)
    {
        airTeamDied(aiplayer,team);
    }
}

bool32 checkAddToTeam(LinkedList *TeamWaitingQ,Ship *ship)
{
    Node *node = TeamWaitingQ->head;
    TeamWaitingForTheseShips *teamWaiting;

    while (node != NULL)
    {
        teamWaiting = (TeamWaitingForTheseShips *)listGetStructOfNode(node);

        dbgAssert(teamWaiting->num_ships > 0);
        if (teamWaiting->shiptype == ship->shiptype)
        {
            teamWaiting->num_ships--;
            aitAddShip(teamWaiting->team,ship);
            if (teamWaiting->num_ships == 0)
            {
                aivarValueSet(aivarFind(teamWaiting->doneSetVarStr),TRUE);
                listDeleteNode(node);
            }
            return TRUE;
        }

        node = node->next;
    }

    return FALSE;
}

static bool32 aifCheckAddToTeam(void)
{
    Player *player;
    Ship *mothership;
    ShipType shiptype;

    if (singlePlayerGame)
    {
        return FALSE;
    }

    player = aiCurrentAIPlayer->player;
    mothership = player->PlayerMothership;
    if (mothership == NULL ||
        AIF_TECHNOLOGYDEFICIT_RESOURCE_BYTE(aiCurrentAIPlayer) != 0)
    {
        return FALSE;
    }

    if (player->race == RACE_Sect)
    {
        shiptype = (mothership->shiptype != sMothership) ? sCarrierSupport : sMothershipSupport;
    }
    else
    {
        if (player->race != RACE_Beast ||
            mothership->shiptype != bMothership)
        {
            return FALSE;
        }
        shiptype = bMothershipSupport;
    }

    if (player->shiptotals[shiptype] >= cdLimitCaps[shiptype])
    {
        return FALSE;
    }

    if (player->shiptotals[shiptype] == 0 &&
        rmCountBuildPrerequisites(player, shiptype))
    {
        aifResearchForTeamBuild(player, shiptype);
        return FALSE;
    }

    aifResourceManRequestsShipsCB(shiptype, 1, 1);
    AIF_TECHNOLOGYDEFICIT_RESOURCE_BYTE(aiCurrentAIPlayer) = 1;
    return TRUE;
}

static void aifCountQueuedShipsForCaps(
    AIPlayer *aiplayer,
    Node *node,
    ShipClass shipclass,
    ShipType shiptype,
    sdword *shipTotal,
    sdword *classTotal,
    sdword *typeTotal)
{
    RequestShips *request;
    ShipStaticInfo *requestStatic;

    while (node != NULL)
    {
        request = (RequestShips *)listGetStructOfNode(node);
        *shipTotal += request->num_ships;

        requestStatic = GetShipStaticInfo(request->shiptype, aiplayer->player->race);
        if (requestStatic->shipclass == shipclass)
        {
            *classTotal += request->num_ships;
            if (request->shiptype == shiptype)
            {
                *typeTotal += request->num_ships;
            }
        }

        node = node->next;
    }
}

sdword aifShipLimitAdjust(AIPlayer *aiplayer, ShipType shiptype, sdword numRequested)
{
    sdword shipTotal = 0;
    sdword classTotal = 0;
    sdword typeTotal = 0;
    Player *player = aiplayer->player;
    ShipStaticInfo *requestedStatic;
    ShipClass shipclass;
    ShipRace race;
    ShipType currentType;
    SelectCommand *newShips;
    sdword remainingCap;
    sdword i;

    if (!cdEnabled || singlePlayerGame)
    {
        return numRequested;
    }

    race = player->race;
    requestedStatic = GetShipStaticInfo(shiptype, race);
    shipclass = requestedStatic->shipclass;

    for (currentType = FirstShipTypeOfRace[race];
         currentType <= LastShipTypeOfRace[race];
         currentType++)
    {
        sdword queuedCount;
        ShipStaticInfo *currentStatic;

        currentStatic = GetShipStaticInfo(currentType, race);
        queuedCount =
            aiplayer->AttackManShipsBeingBuilt.NumShipsOfType[currentType] +
            aiplayer->DefenseManShipsBeingBuilt.NumShipsOfType[currentType] +
            aiplayer->ScriptManShipsBeingBuilt.NumShipsOfType[currentType];
        shipTotal += queuedCount;

        if (currentStatic->shipclass == shipclass)
        {
            classTotal += queuedCount;
            if (currentType == shiptype)
            {
                typeTotal += queuedCount;
            }
        }
    }

    aifCountQueuedShipsForCaps(
        aiplayer,
        aiplayer->AttackManRequestShipsQ.head,
        shipclass,
        shiptype,
        &shipTotal,
        &classTotal,
        &typeTotal);
    aifCountQueuedShipsForCaps(
        aiplayer,
        aiplayer->DefenseManRequestShipsQ.head,
        shipclass,
        shiptype,
        &shipTotal,
        &classTotal,
        &typeTotal);
    aifCountQueuedShipsForCaps(
        aiplayer,
        aiplayer->ScriptManRequestShipsQ.head,
        shipclass,
        shiptype,
        &shipTotal,
        &classTotal,
        &typeTotal);

    newShips = aiplayer->newships.selection;
    shipTotal += newShips->numShips;
    for (i = 0; i < newShips->numShips; i++)
    {
        Ship *ship = newShips->ShipPtr[i];

        if (ship->staticinfo->shipclass == shipclass)
        {
            classTotal++;
            if (ship->shiptype == shiptype)
            {
                typeTotal++;
            }
        }
    }

    shipTotal += player->totalships;
    shipTotal += aiplayer->NumRCollectorsBeingBuilt;
    shipTotal += aiplayer->NumRControllersBeingBuilt;
    shipTotal += aiplayer->NumASFBeingBuilt;
    shipTotal += aiplayer->NumResearchShipsBeingBuilt;

    switch (shiptype)
    {
        case sWorker:
        case bWorker:
            typeTotal += aiplayer->NumRCollectorsBeingBuilt;
            classTotal += aiplayer->NumRCollectorsBeingBuilt;
            classTotal += aiplayer->NumRControllersBeingBuilt;
            break;

        case sProcessor:
        case bProcessor:
            typeTotal += aiplayer->NumRControllersBeingBuilt;
            classTotal += aiplayer->NumRControllersBeingBuilt;
            classTotal += aiplayer->NumRCollectorsBeingBuilt;
            break;

        case sRepairDroid:
        case bRepairDroid:
            typeTotal += aiplayer->NumASFBeingBuilt;
            classTotal += aiplayer->NumASFBeingBuilt;
            break;

        default:
            switch (shipclass)
            {
                case CLASS_Resource:
                    classTotal += aiplayer->NumRCollectorsBeingBuilt;
                    classTotal += aiplayer->NumRControllersBeingBuilt;
                    break;

                case CLASS_Frigate:
                    classTotal += aiplayer->NumASFBeingBuilt;
                    break;

                case CLASS_NonCombat:
                    classTotal += aiplayer->NumResearchShipsBeingBuilt;
                    break;
            }
            break;
    }

    remainingCap = cdMaxShipsAllowed - shipTotal;
    if (remainingCap < numRequested)
    {
        return (remainingCap < 0) ? 0 : remainingCap;
    }

    if (cdLimitCaps[shiptype] != -1)
    {
        remainingCap = cdLimitCaps[shiptype] - (player->shiptotals[shiptype] + typeTotal);
        if (remainingCap < numRequested)
        {
            return (remainingCap < 0) ? 0 : remainingCap;
        }
    }

    if (cdClassCaps[shipclass] != -1)
    {
        remainingCap = cdClassCaps[shipclass] - (player->classtotals[shipclass] + classTotal);
        if (remainingCap < numRequested)
        {
            return (remainingCap < 0) ? 0 : remainingCap;
        }
    }

    return numRequested;
}


/*-----------------------------------------------------------------------------
    Name        : aifProcessShipBuildRequests
    Description : Goes through the attack, defense, resource and script ship build queues
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aifProcessShipBuildRequests(void)
{
   RequestShips *attrequest = NULL,*defrequest = NULL,*scriptrequest = NULL;
   RequestShips *creatorrequest = NULL;
   RequestShips resourcerequestcompat;
   RequestShips *resourcerequest = NULL;
   sdword olddefnumships = 0;
   sdword oldattnumships = 0;
   sdword oldscriptnumships = 0;
   sdword oldcreatornumships = 0;
   Player *player = aiCurrentAIPlayer->player;
   ShipRace race = player->race;
   ubyte research_delay;
   Node *tempnode;
   ShipPtr creator;
   sdword freelab;
   bool32 stopFurtherTeams = FALSE;

   research_delay = AIF_TECHNOLOGYDEFICIT_RESEARCH_DELAY(aiCurrentAIPlayer);
   if (research_delay)
   {
       research_delay--;
       AIF_TECHNOLOGYDEFICIT_RESEARCH_DELAY(aiCurrentAIPlayer) = research_delay;
   }
   else
   {
       AIF_TECHNOLOGYDEFICIT_RESEARCH_DELAY(aiCurrentAIPlayer) = AIF_RESEARCH_DELAY[aiCurrentAIPlayer->aiplayerDifficultyLevel];
   }

   AIF_SCRIPTMAN_BUILT_TAIL_DWORD(aiCurrentAIPlayer) = 0;

   if (research_delay == 0)
   {
       aifCheckAddToTeam();
   }

   if (aiCurrentAIPlayer->ResourceManRequestShips.num_ships > 0)
   {
       memset(&resourcerequestcompat, 0, sizeof(resourcerequestcompat));
       resourcerequestcompat.shiptype  = aiCurrentAIPlayer->ResourceManRequestShips.shiptype;
       resourcerequestcompat.num_ships = aiCurrentAIPlayer->ResourceManRequestShips.num_ships;
       resourcerequestcompat.priority  = aiCurrentAIPlayer->ResourceManRequestShips.priority;
       resourcerequestcompat.creator   = aiCurrentAIPlayer->ResourceManRequestShips.creator;
       resourcerequest = &resourcerequestcompat;

       aifValidateBuildRequest(&resourcerequest, player, race, research_delay, &stopFurtherTeams);
   }

   if (aiCurrentAIPlayer->AttackManRequestShipsQ.head != NULL)
   {
       tempnode   = aiCurrentAIPlayer->AttackManRequestShipsQ.head;
       attrequest = (RequestShips *)listGetStructOfNode(tempnode);
       while (attrequest != NULL)
       {
           if (aifValidateBuildRequest(&attrequest, player, race, research_delay, &stopFurtherTeams))
           {
               break;
           }

           tempnode = tempnode->next;
           if (tempnode == NULL)
           {
               break;
           }
           attrequest = (RequestShips *)listGetStructOfNode(tempnode);
       }

       if (attrequest != NULL)
       {
           oldattnumships = attrequest->num_ships;
           dbgAssert(oldattnumships > 0);
       }
   }

   if (aiCurrentAIPlayer->ResourceManRequestShips.creator != NULL &&
       !stopFurtherTeams)
   {
       creator = aiCurrentAIPlayer->ResourceManRequestShips.creator;
       while (creatorrequest == NULL)
       {
           creatorrequest = (RequestShips *)creator->objlink.structptr;
           aifValidateBuildRequest(&creatorrequest, player, race, research_delay, &stopFurtherTeams);

           creator = (ShipPtr)creator->objlink.next;
           if (creator == NULL)
           {
               break;
           }
       }

       if (creatorrequest != NULL)
       {
           oldcreatornumships = creatorrequest->num_ships;
       }
   }

   if (aiCurrentAIPlayer->DefenseManRequestShipsQ.head != NULL &&
       !stopFurtherTeams)
   {
       tempnode   = aiCurrentAIPlayer->DefenseManRequestShipsQ.head;
       defrequest = (RequestShips *)listGetStructOfNode(tempnode);
       olddefnumships = defrequest->num_ships;
   }

   //it is assumed that any ship requested by the script is researched
    //therefore no research is done for the script system and no checking is done either
    if (aiCurrentAIPlayer->ScriptManRequestShipsQ.head != NULL)
    {
        scriptrequest     = (RequestShips *)listGetStructOfNode(aiCurrentAIPlayer->ScriptManRequestShipsQ.head);
        oldscriptnumships = scriptrequest->num_ships;
        dbgAssert(oldscriptnumships > 0);
    }

    if (!singlePlayerGame)
    {
        freelab = rmFindFreeLab(player, -1);
        if (freelab != -1)
        {
            if (race == RACE_Beast)
            {
                if (!rmTechIsResearched(player, 32))
                {
                    clWrapResearch(0, player->playerIndex, freelab, 32);
                }
            }
            else if (rmTechRequiredForShip(player, sMothership))
            {
                aifResearchForTeamBuild(player, sMothership);
            }
            else if (rmTechRequiredForShip(player, sCarrier))
            {
                aifResearchForTeamBuild(player, sCarrier);
            }
        }
    }

    aifBuildRequestedShips(scriptrequest, attrequest, defrequest, resourcerequest);

    if (resourcerequest != NULL)
    {
        aiCurrentAIPlayer->ResourceManRequestShips.priority = resourcerequest->num_ships;
    }

    if (scriptrequest != NULL)
    {
        // check and see if we built any ships, and if we should remove from Q
//        dbgAssert(scriptrequest->num_ships >= 0);
        if (oldscriptnumships != scriptrequest->num_ships)
        {
            dbgAssert((oldscriptnumships - scriptrequest->num_ships) > 0);
            aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[scriptrequest->shiptype] += (ubyte)(oldscriptnumships - scriptrequest->num_ships);
        }

        if (scriptrequest->num_ships <= 0)
        {
            // remove from Q
            listDeleteNode(&scriptrequest->node);
        }
    }

    if (attrequest != NULL)
    {
        // check and see if we built any ships, and if we should remove from Q
//        dbgAssert(attrequest->num_ships >= 0);
        if (oldattnumships != attrequest->num_ships)
        {
            dbgAssert((oldattnumships - attrequest->num_ships) > 0);
            aiCurrentAIPlayer->AttackManShipsBeingBuilt.NumShipsOfType[attrequest->shiptype] += (ubyte)(oldattnumships - attrequest->num_ships);
        }

        if (attrequest->num_ships <= 0)
        {
            // remove from Q
            listDeleteNode(&attrequest->node);
        }
    }

    if (creatorrequest != NULL)
    {
        if (oldcreatornumships != creatorrequest->num_ships)
        {
            dbgAssert((oldcreatornumships - creatorrequest->num_ships) > 0);
            aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[creatorrequest->shiptype] += (ubyte)(oldcreatornumships - creatorrequest->num_ships);
        }

        if (creatorrequest->num_ships <= 0)
        {
            listDeleteNode(&creatorrequest->node);
        }
    }

    if (defrequest != NULL)
    {
        // check and see if we built any ships, and if we should remove from Q
//        dbgAssert(defrequest->num_ships >= 0);
        if (olddefnumships != defrequest->num_ships)
        {
            dbgAssert((olddefnumships - defrequest->num_ships) > 0);
            aiCurrentAIPlayer->DefenseManShipsBeingBuilt.NumShipsOfType[defrequest->shiptype] += (ubyte)(olddefnumships - defrequest->num_ships);
        }

        if (defrequest->num_ships <= 0)
        {
            // remove from Q
            listDeleteNode(&defrequest->node);
        }
    }
}



/*-----------------------------------------------------------------------------
    Name        : aifAssignNewShips
    Description : Checks who is requesting new ships and assigns them to the proper manager
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aifAssignNewShips(void)
{
    GrowSelection *newships = &aiCurrentAIPlayer->newships;
    udword i;
    ShipPtr ship;

    for (i=0;i<newships->selection->numShips;)
    {
        ship = newships->selection->ShipPtr[i];

        if ( (aiCurrentAIPlayer->DefenseManShipsBeingBuilt.NumShipsOfType[ship->shiptype] > 0) &&
             (checkAddToTeam(&aiCurrentAIPlayer->DefenseManTeamsWaitingForShipsQ,ship)) )
        {
            // defense manager requested this ship to be built, so give it to defenseman.
            aiCurrentAIPlayer->DefenseManShipsBeingBuilt.NumShipsOfType[ship->shiptype]--;
            growSelectRemoveShip(newships,ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else if ( (aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[ship->shiptype] > 0) &&
                  checkAddToTeam(&aiCurrentAIPlayer->ScriptManTeamsWaitingForShipsQ,ship) )
        {
            // script manager requested this ship to be built, so give it to scriptman.
            aiCurrentAIPlayer->ScriptManShipsBeingBuilt.NumShipsOfType[ship->shiptype]--;
            growSelectRemoveShip(newships,ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else if ( (aiCurrentAIPlayer->AttackManShipsBeingBuilt.NumShipsOfType[ship->shiptype] > 0) &&
                  (checkAddToTeam(&aiCurrentAIPlayer->AttackManTeamsWaitingForShipsQ,ship)) )
        {
            // attack manager requested this ship to be built, so give it to attackman.
            aiCurrentAIPlayer->AttackManShipsBeingBuilt.NumShipsOfType[ship->shiptype]--;
            growSelectRemoveShip(newships,ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else if ((ship->staticinfo->shipclass == CLASS_Resource) ||
            (ship->staticinfo->shipclass == CLASS_NonCombat) ||
            ShipTypeIsProcessorType(ship->shiptype) ||
            ShipTypeIsRepairShipType(ship->shiptype) ||
            ShipTypeIsMothershipOrCarrierType(ship->shiptype))
        {
            growSelectRemoveShip(newships,ship);
            airAddNewShip(ship);
            continue;       // check same index again because we removed a ship from selection we are scanning
        }
        else
        {
            // leave in reserves
        }
        i++;
    }
}


/*=============================================================================
    Hyperspacing Logic Code:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aifHyperspaceInit
    Description : Initializes any hyperspacing stuff
    Inputs      : aiplayer - the CPU to do hyperspacing initialization for
    Outputs     : Whatever's needed to initialize hyperspacing
    Return      : void
----------------------------------------------------------------------------*/
void aifHyperspaceInit(AIPlayer *aiplayer)
{
    //determine what percentage of RUs to skim for hyperspacing
    //for now, just take off 8%
    aiplayer->aifHyperSkimming = 0.08f;
}


/*-----------------------------------------------------------------------------
    Name        : aifSkimHyperspaceRUs
    Description : Banks RUs for Hyperspacing
    Inputs      : none
    Outputs     : transfers some new RUs to the aiplayer structure
    Return      : void
----------------------------------------------------------------------------*/
void aifSkimHyperspaceRUs(void)
{
    udword newResources, skimResources;
    Player *player = aiCurrentAIPlayer->player;

    if (aiCurrentAIPlayer->aifHyperSavings < 0)
    {   // if the CPU is in debt, all new resources go to pay off the debt
        if (player->resourceUnits)
        {
            if (player->resourceUnits < (-aiCurrentAIPlayer->aifHyperSavings))
            {   // the players resources do not exceed the debt.  Pay off as much as possible
                aiplayerLog((aiIndex, "Paying off the debt.  Savings = %i, deposit = %i", aiCurrentAIPlayer->aifHyperSavings, player->resourceUnits));
                aiCurrentAIPlayer->aifHyperSavings += player->resourceUnits;
                player->resourceUnits = 0;
            }
            else
            {   // the players resources exceed the debt.  Pay off everything.
                aiplayerLog((aiIndex, "Paying off the entire debt."));
                player->resourceUnits += aiCurrentAIPlayer->aifHyperSavings;
                aiCurrentAIPlayer->aifHyperSavings = 0;
            }
        }
    }
    else if (player->resourceUnits > aiCurrentAIPlayer->aifLastRUCount)
    {   // if the CPU has acquired new resources, skim some off
        newResources  = player->resourceUnits - aiCurrentAIPlayer->aifLastRUCount;
        skimResources = (udword)((real32)newResources * aiCurrentAIPlayer->aifHyperSkimming);
        aiCurrentAIPlayer->aifHyperSavings += skimResources;
        player->resourceUnits              -= skimResources;

        aiplayerLog((aiIndex, "Skimming %i RUs for Hyperspacing.  Savings = %i, RUs = %i", skimResources, aiCurrentAIPlayer->aifHyperSavings, player->resourceUnits));
    }
    aiCurrentAIPlayer->aifLastRUCount = player->resourceUnits;
}


/*=============================================================================
    Ally Logic:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aifFindAllies
    Description : Looks at the other CPUs and allies with those that
                  have the same primary enemy
    Inputs      :
    Outputs     : Sets up some alliances
    Return      : void
----------------------------------------------------------------------------*/
extern udword AIPLAYER_BIGOTRY[];

void aifFindAllies(void)
{
    udword i;
    AIPlayer *OtherCPU;
    udword HowMuchDoILoveMyFriends;

    HowMuchDoILoveMyFriends = randyrandom(RAN_AIPlayer, 100);

    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].aiPlayer &&
            (universe.players[i].playerState == PLAYER_ALIVE) &&
            (universe.players[i].aiPlayer != aiCurrentAIPlayer))
        {                   // if the player is a living CPU but not this CPU
            OtherCPU = universe.players[i].aiPlayer;

            if (OtherCPU->primaryEnemyPlayer == aiCurrentAIPlayer->primaryEnemyPlayer)
            {               // if their primary enemies are the same
                if ((HowMuchDoILoveMyFriends < AIPLAYER_BIGOTRY[i]) &&
                    !allianceArePlayersAllied(OtherCPU->player, aiCurrentAIPlayer->player))
                {           // if the CPU wants to be allied if they aren't allied
                    //create an alliance
                    clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE,
                                      OtherCPU->player->playerIndex,
                                      aiCurrentAIPlayer->player->playerIndex);
                }
            }
            else
            {               // their primary enemies aren't the same
                if (allianceArePlayersAllied(OtherCPU->player, aiCurrentAIPlayer->player))
                {           // if they are already allied
                    //break the alliance
                    clWrapSetAlliance(ALLIANCE_BREAKALLIANCE,
                                      OtherCPU->player->playerIndex,
                                      aiCurrentAIPlayer->player->playerIndex);
                }
            }
        }
    }
    aiCurrentAIPlayer->firstTurn = FALSE;
}


/*=============================================================================
    Fleet Commands:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : aifP2FleetCommand
    Description : Logic for fleet command for the P2 pirates
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void aifP2FleetCommand(void)
{
    Player *player = aiCurrentAIPlayer->player;

    aiuCreateBlobArrays(player);
    aiuUpdateKnowledgeOfEnemyShips(aiCurrentAIPlayer);
    aidClearDistressSignal(aiCurrentAIPlayer);

    aifAssignNewShips();

    aiaP2AttackManager();
    aitExecute();

    aifProcessShipBuildRequests();

    aiuDeleteBlobArrays();
}




/*-----------------------------------------------------------------------------
    Name        : aifFleetCommand
    Description : Logic for the top of the computer player command structure
    Inputs      : aiplayer - the computer player structure
    Outputs     : lotsa stuff
    Return      : void
----------------------------------------------------------------------------*/
void aifFleetCommand(void)
{
    Player *player = aiCurrentAIPlayer->player;
    bool32 hasFleetControl = (singlePlayerGame) ? singlePlayerGameInfo.giveComputerFleetControl : TRUE;

    aiIndex = aiCurrentAIPlayer->player->playerIndex;

    if (aiCurrentAIPlayer->firstTurn)
    {
        aifFindAllies();
    }

    aiuCreateBlobArrays(player);
    aiuUpdateKnowledgeOfEnemyShips(aiCurrentAIPlayer);
    aidClearDistressSignal(aiCurrentAIPlayer);

    aifAssignNewShips();

    if (aiCurrentAIPlayer->firstTurn)
    {
        if (hasFleetControl)
        {
            aiaProcessSpecialTeams();
//        airProcessSpecialTeams();
        }
        aiCurrentAIPlayer->firstTurn = FALSE;
    }
    else
    {
        if (hasFleetControl)
        {
            if (aiuResourceFeatureEnabled(AIF_HYPERSPACING))
            {
                aifSkimHyperspaceRUs();
            }

            aiCurrentAIPlayer->ResourceManRequestShips.priority = 0;

            //call AttackMan - any ships requested will result in call to aifAttackManRequestsShipsCB
            aiaAttackManager();

            //call DefenseMan - any ships requested will result in call to aifDefenseManRequestsShipsCB
            aidDefenseManager();

            //call Resource Manager - any ships requested will result in call to aifResourceManRequestsShipsCB
            airResourceManager();
        }
    }

    // call Team Manager - any ships requested will result in call to aifAttackManRequestsShipsCB or aifDefenseManRequestsShipsCB
    aitExecute();

    aifProcessShipBuildRequests();

    aiuDeleteBlobArrays();
}






#if 0  // Cat2: Binary aifResearchForTeamBuild; live implementation at AIFleetMan.c L1338 (0x4F3380)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x4F3380
sdword aifResearchForTeamBuild(AIPlayer *aiplayer, sdword teamIndex)
{
    sdword result = 0;
    sdword teamOffset = 168 * teamIndex;
    sdword shipReqs[42], techReqs[42], availTech[42], needed[42];
    sdword researchable[42], prereqs[42];
    sdword numBuildable = 0;

    if (aiplayer->playerType == 1 && !aiplayer->teamCanBuild[teamIndex])
        return 0;

    // Copy team ship request mask
    memcpy(shipReqs, aiplayer->teamRequests + teamOffset, sizeof(shipReqs));
    memset(techReqs, 0, sizeof(techReqs));

    // Gather tech requirements for each requested ship type
    for (int i = 0; i < 41; i++)
    {
        if (shipReqs[41] == 2 || shipReqs[i] == 1)
        {
            rmAccumulatePrerequisitesRecursive(techReqs, aiplayer, i);
            sub_4F4130(shipReqs, shipReqs, techReqs);
        }
    }

    // Apply current research state
    sub_4F4300(availTech, aiplayer->researchState);
    sub_4F4070(shipReqs, shipReqs, availTech);

    // Count buildable types
    for (int i = 0; i < 41; i++)
        if (shipReqs[41] == 2 || shipReqs[i] == 1)
            numBuildable++;

    if (numBuildable > 0)
    {
        // For each buildable type, check if all prereqs met
        for (int shipType = 0; shipType < NUM_SHIP_TYPES; shipType++)
        {
            if (needed[41] == 2 || needed[shipType] == 1)
            {
                // Check if no remaining prereqs
                bool allMet = true;
                for (int j = 0; j < 41; j++)
                    if (prereqs[j] > 0) { allMet = false; break; }
                if (allMet)
                {
                    sdword lab = rmFindFreeLab(aiplayer->player, shipType);
                    if (lab != -1)
                    {
                        clWrapResearch(0, aiplayer->playerIndex, lab, shipType);
                        result = 1;
                    }
                    break;
                }
            }
        }
    }
    // If nothing buildable, try fallback team requests (offset 904)
    // ... similar logic with secondary request array ...
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x416F40
void aifAddShipToTeam(AITeam *team, Ship *ship)
{
    growSelectAddShip(&team->shipList, ship);
    int *sel = team->shipList;
    int firstType = *(int *)(sel[2] + 688);
    team->flags &= ~0x300;
    for (int i = 1; i < *sel; i++) {
        if (*(int *)(sel[i+2] + 688) != firstType)
            team->flags |= 0x200;
    }
    team->flags |= 0x100;
    int lastShipType = *(int *)(sel[*sel + 1] + 688);
    if (!(team->flags & 0x1000000) && lastShipType == 36)
        team->flags |= 0x1000000;
    if (!(team->flags & 0x4000000) && (lastShipType == 63 || lastShipType == 77))
        team->flags |= 0x4000000;
    team->totalFirepower = 0;
    team->totalHealth = 0;
    for (int i = 0; i < *sel; i++) {
        sub_41ACB0(&fp, &hp, sel[i+2]);
        team->totalFirepower += fp;
        team->totalHealth += hp;
    }
    team->numShips++;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x41E500
sdword aifShipLimitAdjust(AIPlayer *aiplayer, ShipType shiptype, sdword numRequested)
{
int v4; // ebp
ShipClassEnum shipclass; // ecx
int v6; // eax
unsigned int v7; // esi
ShipStaticInfo2 *ShipStaticInfo; // eax
int v9; // ecx
_DWORD *i; // ebx
int v11; // esi
int v12; // eax
_DWORD *j; // ebx
int v14; // esi
int v15; // eax
_DWORD *k; // ebx
int v17; // esi
int v18; // eax
int *v19; // eax
int v20; // ecx
_DWORD *v21; // edx
int v22; // eax
int v23; // ecx
int v24; // esi
int v25; // ebx
int v26; // ebp
int v27; // eax
int result; // eax
int v29; // ecx
int v30; // ecx
int v31; // ecx
int v32; // ecx
int v33; // [esp+Ch] [ebp-Ch]
int v34; // [esp+10h] [ebp-8h]
int v35; // [esp+14h] [ebp-4h]
ShipClassEnum v36; // [esp+1Ch] [ebp+4h]

v4 = 0;
v35 = *a1;
v33 = 0;
v34 = 0;
if ( !cdEnabled || singlePlayerGame )
return a3;
shipclass = GetShipStaticInfo(shiptype)->shipclass;
v6 = *(_DWORD *)(*a1 + 20);
v7 = ::shiptype[v6];
v36 = shipclass;
if ( v7 < dword_889504[v6] )
{
do
{
ShipStaticInfo = GetShipStaticInfo((ShipTypeEnum)v7);
v9 = *((unsigned __int8 *)a1 + v7 + 2661)
+ *((unsigned __int8 *)a1 + v7 + 2806)
+ *((unsigned __int8 *)a1 + v7 + 2516);
v4 += v9;
if ( ShipStaticInfo->shipclass == v36 )
{
v33 += v9;
if ( v7 == shiptype )
v34 += v9;
}
++v7;
}
while ( v7 < dword_889504[*(_DWORD *)(*a1 + 20)] );
}
for ( i = (_DWORD *)a1[611]; i; i = (_DWORD *)*i )
{
v11 = i[3];
v4 += *(_DWORD *)(v11 + 20);
if ( GetShipStaticInfo(*(ShipTypeEnum *)(v11 + 16))->shipclass == v36 )
{
v12 = *(_DWORD *)(v11 + 20);
v33 += v12;
if ( *(_DWORD *)(v11 + 16) == shiptype )
v34 += v12;
}
}
for ( j = (_DWORD *)a1[614]; j; j = (_DWORD *)*j )
{
v14 = j[3];
v4 += *(_DWORD *)(v14 + 20);
if ( GetShipStaticInfo(*(ShipTypeEnum *)(v14 + 16))->shipclass == v36 )
{
v15 = *(_DWORD *)(v14 + 20);
v33 += v15;
if ( *(_DWORD *)(v14 + 16) == shiptype )
v34 += v15;
}
}
for ( k = (_DWORD *)a1[617]; k; k = (_DWORD *)*k )
{
v17 = k[3];
v4 += *(_DWORD *)(v17 + 20);
if ( GetShipStaticInfo(*(ShipTypeEnum *)(v17 + 16))->shipclass == v36 )
{
v18 = *(_DWORD *)(v17 + 20);
v33 += v18;
if ( *(_DWORD *)(v17 + 16) == shiptype )
v34 += v18;
}
}
v19 = (int *)a1[10];
v20 = *v19;
if ( *v19 )
{
v21 = v19 + 2;
v4 += v20;
do
{
if ( *(_DWORD *)(*(_DWORD *)(*v21 + 32) + 488) == v36 )
{
++v33;
if ( *(_DWORD *)(*v21 + 688) == shiptype )
++v34;
}
++v21;
--v20;
}
while ( v20 );
}
v22 = a1[592];
v23 = a1[593];
v24 = a1[594];
v25 = a1[595];
v26 = v25 + v24 + v23 + v22 + *(_DWORD *)(*a1 + 44) + v4;
switch ( v36 )
{
case CLASS_Frigate:
v27 = v24 + v33;
LABEL_31:
v33 = v27;
break;
case CLASS_Resource:
v33 += v23 + v22;
break;
case CLASS_NonCombat:
v27 = v25 + v33;
goto LABEL_31;
}
result = a3;
if ( dword_BA1774 - v26 < a3 )
return (dword_BA1774 - v26) & ((dword_BA1774 - v26 < 0) - 1);
v29 = dword_BA03C0[shiptype];
if ( v29 != -1 )
{
v30 = v29 - *(_DWORD *)(v35 + 4 * shiptype + 48) - v34;
if ( v30 < a3 )
return v30 < 0 ? 0 : v30;
}
v31 = dword_B3ABA0[v36];
if ( v31 != -1 )
{
v32 = v31 - *(_DWORD *)(v35 + 4 * v36 + 628) - v33;
if ( v32 < a3 )
return v32 < 0 ? 0 : v32;
}
return result;
}


#endif
