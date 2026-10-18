/*=============================================================================
    Name    : crates.c
    Purpose : contains all code for all crate gameplay

    Created bryce in november
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>

#include "types.h"
#include "ShipDefs.h"
#include "spaceobj.h"
#include "univupdate.h"
#include "universe.h"
#include "blobs.h"
#include "aitrack.h"
#include "linkedlist.h"
#include "researchapi.h"
#include "multiplayergame.h"
#include "levelload.h"
#include "randy.h"
#include "soundevent.h"
#include "sensors.h"
#include "pieplate.h"
#include "statscript.h"


real32 CRATES_DoWeNeedToAddCratesCheckTime =380.0f;  //every X seconds, the crate functions will check to see if it should add a crate to the world

sdword CRATES_ArePlayersNearUpdateMask= 63;  //bitmask to compare with univupdatecounter to see if players are near the crate
sdword CRATES_ArePlayersNearUpdateFrame=52;  //frame to check must be <= CRATES_ArePlayersNearUpdateMask
real32 CRATES_PlayerNearRadius = 2500.0f;     //distance a player needs to get to a crate in order to 'get' the crate

real32 CRATES_MaximumCratesInTheWorldPerPlayer =1;  //don't a   //probability that you get a ship as opposed to something else
real32 CRATES_GetResearch                          = 0.25f;

real32 CRATES_RandomLowDistance  =  7000.0f;
real32 CRATES_RandomHighDistance =  20000.0f;

real32 CRATES_AddACrateProb   =  0.15f;         //probability that we add a crate

sdword CRATES_MAX_RUS_GIVEN =   10000;
sdword CRATES_MIN_RUS_GIVEN =   500 ;

real32 CRATES_GetMotherShipCarrierIfDontHaveOne  = 0.80f;   //probability that you get a carrier/mothership if you didn't have one
real32 CRATES_GetAShip                         =    0.50f;//research probability
//defaults
real32 CrateClassProbCLASS_HeavyCruiser = 0.99f;
real32 CrateClassProbCLASS_Carrier = 0.97f;
real32 CrateClassProbCLASS_Destroyer = 0.90f;
real32 CrateClassProbCLASS_Frigate = 0.75f;
real32 CrateClassProbCLASS_Corvette = 0.45f;
real32 CrateClassProbCLASS_Fighter = 0.10f;
real32 CrateClassProbCLASS_Resource = 0.5f;
real32 CrateClassProbCLASS_NonCombat = 0.0f;
sdword NUM_IN_GROUPS[STD_LAST_SHIP + 1];  //number of ships to be placed in agroup
real32 SHIP_PROBS[STD_LAST_SHIP + 1];
real32 CRATE_EXPIRY_TIME = 300.0;

///RU's will be given instead
scriptEntry cratesScriptTable[] =
{
    makeEntry(CRATES_DoWeNeedToAddCratesCheckTime,scriptSetReal32CB),
    makeEntry(CRATES_ArePlayersNearUpdateMask,scriptSetSdwordCB),
    makeEntry(CRATES_ArePlayersNearUpdateFrame,scriptSetSdwordCB),
    makeEntry(CRATES_PlayerNearRadius,scriptSetReal32CB),
    makeEntry(CRATES_MaximumCratesInTheWorldPerPlayer,scriptSetReal32CB),
    makeEntry(CRATES_GetResearch,scriptSetReal32CB),
    makeEntry(CRATES_RandomLowDistance,scriptSetReal32CB),
    makeEntry(CRATES_RandomHighDistance,scriptSetReal32CB),
    makeEntry(CRATES_AddACrateProb,scriptSetReal32CB),
    makeEntry(CRATES_MAX_RUS_GIVEN,scriptSetSdwordCB),
    makeEntry(CRATES_MIN_RUS_GIVEN,scriptSetSdwordCB),
    makeEntry(CRATES_GetMotherShipCarrierIfDontHaveOne,scriptSetReal32CB),
    makeEntry(CRATES_GetAShip,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_HeavyCruiser,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Carrier,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Destroyer,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Frigate,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Corvette,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Fighter,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_Resource,scriptSetReal32CB),
    makeEntry(CrateClassProbCLASS_NonCombat,scriptSetReal32CB),
    makeEntry(CRATE_EXPIRY_TIME,scriptSetReal32CB),


    {"shipProbability", scriptSetShipProbCB, SHIP_PROBS },
    {"shipGroupSize", scriptSetShipGroupSizeCB, NUM_IN_GROUPS },


    endEntry
};

//crate reward types
#define CRATE_SHIP  0
#define CRATE_MONEY 1
#define CRATE_RESEARCH 2

void cratesPlaceCrateSelectivly();
void cratesReportCratePlacement(Derelict *crate);
void cratesPlayerFoundACrate(Derelict *crate, Player *player);
void cratesGetBestCrateLocation(vector *location);
sdword cratesRewardPlayer(Derelict *crate, Player *player);
void expireCrate(Derelict *crate);


void cratesScanForPlayersNearCrateAndReward(Derelict *crate)
{
    blob *crateBlob;
    sdword i;
    Ship *ship;

    crateBlob = crate->collMyBlob;

    if(crateBlob != NULL)
    {
        for(i=0;i<crateBlob->blobShips->numShips;)
        {
            ship = crateBlob->blobShips->ShipPtr[i];
            if(MoveReachedDestinationVariable(ship,&crate->posinfo.position,CRATES_PlayerNearRadius))
            {
                if(ship->playerowner != NULL)
                {
                    cratesPlayerFoundACrate(crate,ship->playerowner);
                    break;
                }
            }
            i++;    //move to next ship
        }
    }
}

void cratesPlaceCrateSelectivly()
{
    vector location;
    Derelict *crate;
    //etglod *etgLOD;
    //etgeffectstatic *stat;


    cratesGetBestCrateLocation(&location);

    crate = univAddDerelict(Crate,&location);   //calls reporting code!
    //cratesReportCratePlacement(crate);
    //play etg effect for placement
    //play sound event?
}

void cratesReportCratePlacement(Derelict *crate)
{
    etglod *etgLOD;
    etgeffectstatic *stat;
    vector zero = {0.0f,0.0f,0.0f};
    matrix ident = IdentityMatrix;

    universe.numCratesInWorld++;

    if(gameIsRunning)
    {
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_GENERATED];

        if (etgLOD != NULL)
        {
            if (crate->currentLOD >= etgLOD->nLevels)
            {
                stat = NULL;
            }
            else
            {
                stat = etgLOD->level[crate->currentLOD];
            }
        }
        else
        {
            stat = NULL;
        }
        #if ETG_DISABLEABLE
        if (stat != NULL && etgEffectsEnabled)
        #else
        if (stat != NULL)
        #endif
        {
            etgEffectCreate(stat, NULL, &crate->posinfo.position, &zero, &ident, 1.0f, EAF_AllButNLips, 0);
        }
    }
    dbgAssert(universe.numCratesInWorld <= CRATES_MaximumCratesInTheWorldPerPlayer*MAX_MULTIPLAYER_PLAYERS);

}

void cratesPlayerFoundACrate(Derelict *crate, Player *player)
{
    etglod *etgLOD;
    etgeffectstatic *stat;
    vector zero = {0.0f,0.0f,0.0f};
    matrix ident = IdentityMatrix;
    sdword type;

    universe.numCratesInWorld--;

    type = cratesRewardPlayer(crate,player);

    // only play sound for this player
    if(player->playerIndex == universe.curPlayerIndex)
    {
        soundEvent(NULL, UI_CrateFound);
    }
    /* mabe there should be a different sound for each type of crate */

    switch(type)
    {
    case CRATE_SHIP:
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_IS_FOUND_SHIP];
        if(player->playerIndex == universe.curPlayerIndex)
        {
            speechEventFleet(STAT_F_CrateFoundShips,0,player->playerIndex);
        }
        break;
    case CRATE_MONEY:
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_IS_FOUND_MONEY];
        if(player->playerIndex == universe.curPlayerIndex)
        {
            speechEventFleet(STAT_F_CrateFoundResources,0,player->playerIndex);
        }
        break;
    case CRATE_RESEARCH:
        etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_IS_FOUND_RESEARCH];
        if(player->playerIndex == universe.curPlayerIndex)
        {
            speechEventFleet(STAT_F_CrateFoundTech,0,player->playerIndex);
        }
        break;
    default:
        dbgFatalf(DBG_Loc,"\nUnknown crate contents!");
        break;
    }

    if (etgLOD != NULL)
    {
        if (crate->currentLOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[crate->currentLOD];
        }
    }
    else
    {
        stat = NULL;
    }
    #if ETG_DISABLEABLE
    if (stat != NULL && etgEffectsEnabled)
    #else
    if (stat != NULL)
    #endif
    {
        etgEffectCreate(stat, NULL, &crate->posinfo.position, &zero, &ident, 1.0f, EAF_AllButNLips, 0);
    }

    AddTargetToDeleteList((SpaceObjRotImpTarg *)crate,-1);
    }

//maximum number of ships in anyone class
#define MAX_NUM_IN_CLASS 10
sdword cratesCLASS_HeavyCruiser[] =
{
    HeavyCruiser
};
sdword cratesCLASS_Carrier[] =
{
    Carrier
};
sdword cratesCLASS_Destroyer[] =
{
    MissileDestroyer,
    StandardDestroyer
};
sdword cratesCLASS_Frigate[] =
{
    AdvanceSupportFrigate,
    DFGFrigate,
    DDDFrigate,
    CloakGenerator,
    GravWellGenerator,
    IonCannonFrigate,
    StandardFrigate
};
sdword cratesCLASS_Corvette[] =
{
    RepairCorvette,
    SalCapCorvette,
    MultiGunCorvette,
    MinelayerCorvette,
    LightCorvette,
    HeavyCorvette
};
sdword cratesCLASS_Fighter[] =
{
    LightInterceptor,
    LightDefender,
    HeavyInterceptor,
    HeavyDefender,
    DefenseFighter,
    CloakedFighter,
    AttackBomber
};

sdword cratesCLASS_Resource[] =
{
    sWorker,
    bWorker,
    ResourceController
};
sdword cratesCLASS_NonCombat[] =
{
    SensorArray,
    Probe,
    ProximitySensor
};

sdword NUM_IN_CLASSES[NUM_CLASSES];



sdword getShipClass()
{
    real32 prob;
    prob = frandombetween(0.0,1.0f);

    if(prob <= CrateClassProbCLASS_HeavyCruiser)
        return CLASS_HeavyCruiser;
    else if(prob <= CrateClassProbCLASS_Carrier)
        return CLASS_Carrier;
    else if(prob <= CrateClassProbCLASS_Destroyer)
        return CLASS_Destroyer;
    else if(prob <= CrateClassProbCLASS_Frigate)
        return CLASS_Frigate;
    else if(prob <= CrateClassProbCLASS_Corvette)
        return CLASS_Corvette;
    else if(prob <= CrateClassProbCLASS_Fighter)
        return CLASS_Fighter;
    else if(prob <= CrateClassProbCLASS_Resource)
        return CLASS_Resource;

    //default
    return CLASS_NonCombat;
}

ShipType gettype(sdword classtype,sdword index)
{
    if(classtype == CLASS_HeavyCruiser)
        return(cratesCLASS_HeavyCruiser[index]);
    else if(classtype == CLASS_Carrier)
        return(cratesCLASS_Carrier[index]);
    else if(classtype == CLASS_Destroyer)
        return(cratesCLASS_Destroyer[index]);
    else if(classtype == CLASS_Frigate)
        return(cratesCLASS_Frigate[index]);
    else if(classtype == CLASS_Corvette)
        return(cratesCLASS_Corvette[index]);
    else if(classtype == CLASS_Fighter)
        return(cratesCLASS_Fighter[index]);
    else if(classtype == CLASS_Resource)
        return(cratesCLASS_Resource[index]);

    return(cratesCLASS_NonCombat[index]);
}
ShipType getShipType(ShipClass classtype)
{
    sdword i;
    real32 prob,probcurrent,probtest;
    ShipType shipcurrent,shiptype;

    prob = frandombetween(0.0,1.0f);
    probcurrent = 1.0f;
    shipcurrent = 9999;

    for(i=0;i<NUM_IN_CLASSES[classtype];i++)
    {
        shiptype = gettype(classtype,i);
        if(prob <= SHIP_PROBS[shiptype])
        {
            probtest = SHIP_PROBS[shiptype]-prob;
            if(probtest <= probcurrent)
            {
                probcurrent = probtest;
                shipcurrent = shiptype;
            }
        }
    }
    dbgAssert(shipcurrent != 9999); //should have selected SOME ship from list!
    return shipcurrent;
}

typedef struct CrateShipReward
{
    real32 upperBound;
    ShipType shipType;
    sdword shipCount;
} CrateShipReward;

static const CrateShipReward cratesSomtaawShipRewards[] =
{
    {0.01f, sDreadnought, 1},
    {0.04f, sDestroyer, 1},
    {0.10f, sMultiBeamFrigate, 1},
    {0.15f, sMultiBeamFrigate, 2},
    {0.20f, sRammingFrigate, 2},
    {0.30f, sSentinel, 5},
    {0.40f, sACV, 4},
    {0.50f, sMCV, 4},
    {0.60f, sAcolyte, 8},
    {0.75f, sMimic, 8},
    {0.80f, sRecon, 10},
    {1.00f, sWorker, 1},
};

static const CrateShipReward cratesBeastShipRewards[] =
{
    {0.01f, bHeavyCruiser, 1},
    {0.04f, bIonArrayFrigate, 3},
    {0.10f, bHiveFrigate, 2},
    {0.15f, bMultiBeamFrigate, 2},
    {0.20f, bRammingFrigate, 2},
    {0.30f, bSentinel, 5},
    {0.40f, bACV, 4},
    {0.50f, bMultiGunCorvette, 3},
    {0.60f, bInterceptor, 8},
    {0.75f, bHeavyCorvette, 3},
    {0.80f, bRecon, 10},
    {1.00f, bWorker, 1},
};

static const CrateShipReward *cratesGetShipRewardTable(Player *player, sdword *count)
{
    if (player->race == RACE_Sect)
    {
        *count = sizeof(cratesSomtaawShipRewards) / sizeof(cratesSomtaawShipRewards[0]);
        return cratesSomtaawShipRewards;
    }

    *count = sizeof(cratesBeastShipRewards) / sizeof(cratesBeastShipRewards[0]);
    return cratesBeastShipRewards;
}

static const CrateShipReward *cratesPickShipReward(Player *player, real32 probability)
{
    const CrateShipReward *table;
    sdword count;
    sdword i;

    table = cratesGetShipRewardTable(player, &count);
    for (i = 0; i < count; i++)
    {
        if (probability < table[i].upperBound)
        {
            return &table[i];
        }
    }

    return &table[count - 1];
}

static TechnologyType cratesPickResearchReward(Player *player, real32 probability)
{
    if (player->race == RACE_Sect)
    {
        if (probability < 0.05f) return TechRepair;
        if (probability < 0.10f) return TechSalvage;
        if (probability < 0.15f) return TechDriveDestroyer;
        if (probability < 0.20f) return TechArmour2;
        if (probability < 0.25f) return TechMicroShip;
        if (probability < 0.30f) return TechEnergy;
        if (probability < 0.35f) return TechIon;
        if (probability < 0.40f) return TechMissile;
        if (probability < 0.45f) return TechExplosiveCharge;
        if (probability < 0.50f) return TechBigGun;
        if (probability < 0.55f) return TechAdvancedSensors;
        if (probability < 0.60f) return TechForceField1;
        if (probability < 0.65f) return TechRepairBots;
        if (probability < 0.75f) return TechCrystalHarvest;
        if (probability < 0.80f) return TechBeastVaccine;
        if (probability < 0.85f) return TechPhasedTelemetry;
        if (probability < 0.90f) return TechMimic;
        if (probability < 0.95f) return TechLink;
        return probability >= 1.0f ? TechRepulse : TechEMP;
    }

    if (probability < 0.05f) return TechCloaking;
    if (probability < 0.10f) return TechTargetingSystems;
    if (probability < 0.15f) return TechPlasmaWeapons;
    if (probability < 0.20f) return TechMediumGuns;
    if (probability < 0.25f) return TechDefenseField;
    if (probability < 0.30f) return TechHeavyGuns;
    if (probability < 0.35f) return TechRepair;
    if (probability < 0.40f) return TechDriveFighter;
    if (probability < 0.45f) return TechMicroShip;
    if (probability < 0.50f) return TechEnergy;
    if (probability < 0.55f) return TechIon;
    if (probability < 0.60f) return TechMissile;
    if (probability < 0.65f) return TechLink;
    if (probability < 0.75f) return TechAfterburner;
    if (probability < 0.80f) return TechAdvancedSensors;
    if (probability < 0.85f) return TechInfect;
    if (probability < 0.90f) return TechCrystalHarvest;
    if (probability < 0.95f) return TechBiowarhead;
    return probability >= 1.0f ? TechArmour2 : TechDriveCorvette;
}

sdword cratesRewardPlayer(Derelict *crate, Player *player)
{
    real32 prob;
    sdword rus;
    sdword i;
    Ship *ship;
    ShipStaticInfo *teststatic;

    prob = frandombetween(0.0f, 1.0f);
    if(prob <= CRATES_GetAShip)
    {
        const CrateShipReward *reward;
        sdword shiptype;
        sdword shipnumber;
        vector cratepos;

        cratepos.x  = crate->posinfo.position.x;
        cratepos.y  = crate->posinfo.position.y;
        cratepos.z  = crate->posinfo.position.z;

        reward = cratesPickShipReward(player, frandombetween(0.0f, 1.0f));
        shiptype = reward->shipType;
        shipnumber = reward->shipCount;

        teststatic = GetShipStaticInfo(shiptype,player->race);

        if (bitTest(teststatic->staticheader.infoFlags, IF_InfoLoaded))
        {
            sdword sizeofselect = sizeofSelectCommand(shipnumber);
            SelectCommand *selectcom = memAlloc(sizeofselect,"selectform",0);
            selectcom->numShips = shipnumber;

            for (i=0;i<shipnumber;i++)
            {
                ship = univAddShip(shiptype,player->race,&cratepos,player,0);
                gameStatsAddShip(ship,player->playerIndex);
                universe.players[ship->playerowner->playerIndex].initialShipCost += ship->staticinfo->svFirePower;
                cat_unitCapCreateShip(ship,player);

                selectcom->ShipPtr[i] = ship;

                cratepos.z += (ship->staticinfo->staticheader.staticCollInfo.collspheresize * 2.0f);
            }
            clFormation(&universe.mainCommandLayer,selectcom,BROAD_FORMATION);
            memFree(selectcom);
            return(CRATE_SHIP);
        }
    }

    prob = frandombetween(0.0f, 1.0f);
    if(prob <= CRATES_GetResearch)
    {
        for(i=0;i<5;i++)
        {
            TechnologyType techType = cratesPickResearchReward(player, frandombetween(0.0f, 1.0f));

            if(!rmTechIsResearched(player, techType))
            {
                rmGiveTechToPlayerByType(player,techType);
                return(CRATE_RESEARCH);
            }
        }
    }


    rus = randombetween(CRATES_MIN_RUS_GIVEN,CRATES_MAX_RUS_GIVEN);
    player->resourceUnits += rus;
    universe.gameStats.playerStats[player->playerIndex].totalResourceUnitsCollected += rus;
    universe.gameStats.playerStats[player->playerIndex].totalResourceUnits += rus;
    return(CRATE_MONEY);
}

void getPlayerShipCenter(vector *shipCenter,sdword index)
{
    Node *node=universe.ShipList.head;
    Ship *ship;
    shipCenter->x =0.0f;
    shipCenter->y =0.0f;
    shipCenter->z =0.0f;
    while(node!=NULL)
    {
        ship = (Ship *)listGetStructOfNode(node);

        if(ship->playerowner != NULL)
        {
            if(ship->playerowner->playerIndex == index)
            {
                shipCenter->x = ship->posinfo.position.x;
                shipCenter->y = ship->posinfo.position.y;
                shipCenter->z = ship->posinfo.position.z;
                return;
            }
        }

        node = node->next;
    }

}

sdword cratesIsLocationGood(vector *location)
{
    Node *node = universe.collBlobList.head;
    blob *curblob;
    real32 blobradiusSqr,seperationSqr;
    vector seperation;

    while(node != NULL)
    {
        curblob = (blob *) listGetStructOfNode(node);
        blobradiusSqr = curblob->radius*curblob->radius;
        vecSub(seperation,*location,curblob->centre);
        seperationSqr = vecMagnitudeSquared(seperation);
        if(seperationSqr < blobradiusSqr)
        {
            sdword i;
            //in blob..need to do detecting
            for(i=0;i<curblob->blobShips->numShips;i++)
            {
                vecSub(seperation,*location,curblob->blobShips->ShipPtr[i]->collInfo.collPosition);
                if(vecMagnitudeSquared(seperation) < curblob->blobShips->ShipPtr[i]->staticinfo->staticheader.staticCollInfo.collspheresizeSqr)
                {
                    //WITHIN collision radius of ship
                    return FALSE;
                }
                //fine...so continue
            }
        }

        node = node->next;
    }

    return TRUE;

}
void cratesGetBestCrateLocation(vector *location)
{
    vector destinationVectors[MAX_MULTIPLAYER_PLAYERS];
    vector center = {0.0f, 0.0f, 0.0f};
    vector fallbackPosition;
    vector displacement;
    sdword counted = 0;
    sdword i;

    for (i = 0; i < universe.numPlayers; i++)
    {
        Player *player = &universe.players[i];
        Ship *ship;
        Node *node;

        if (player->playerState == PLAYER_DEAD)
        {
            continue;
        }

        counted++;
        if (player->PlayerMothership != NULL)
        {
            vecAddTo(center, player->PlayerMothership->posinfo.position);
            continue;
        }

        fallbackPosition.x = fallbackPosition.y = fallbackPosition.z = 0.0f;
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            ship = (Ship *)listGetStructOfNode(node);
            if (ship->playerowner != NULL && ship->playerowner->playerIndex == i)
            {
                fallbackPosition = ship->posinfo.position;
                break;
            }
        }
        vecAddTo(center, fallbackPosition);
    }

    vecScalarMultiply(center, center, 1.0f / counted);

    for (i = 0; i < universe.numPlayers; i++)
    {
        Player *player = &universe.players[i];
        Ship *ship;
        Node *node;

        if (player->playerState == PLAYER_DEAD)
        {
            continue;
        }

        if (player->PlayerMothership != NULL)
        {
            fallbackPosition = player->PlayerMothership->posinfo.position;
        }
        else
        {
            fallbackPosition.x = fallbackPosition.y = fallbackPosition.z = 0.0f;
            for (node = universe.ShipList.head; node != NULL; node = node->next)
            {
                ship = (Ship *)listGetStructOfNode(node);
                if (ship->playerowner != NULL && ship->playerowner->playerIndex == i)
                {
                    fallbackPosition = ship->posinfo.position;
                    break;
                }
            }
        }

        vecSub(destinationVectors[i], fallbackPosition, center);
        vecNormalize(&destinationVectors[i]);
    }

    for (i = 0; i < universe.numPlayers; i++)
    {
        Player *player = &universe.players[i];
        real32 bountyScale;

        if (player->playerState == PLAYER_DEAD)
        {
            continue;
        }

        bountyScale = max((real32)player->bounty, 1.0f);
        vecScalarMultiply(destinationVectors[i], destinationVectors[i], 1.0f / bountyScale);
    }

    location->x = 0.0f;
    location->y = 0.0f;
    location->z = 0.0f;
    for (i = 0; i < universe.numPlayers; i++)
    {
        if (universe.players[i].playerState == PLAYER_DEAD)
        {
            continue;
        }
        vecAddTo(*location, destinationVectors[i]);
    }

    vecNormalize(location);
    location->x *= frandombetween(CRATES_RandomLowDistance, CRATES_RandomHighDistance);
    location->y *= frandombetween(CRATES_RandomLowDistance, CRATES_RandomHighDistance);
    location->z *= frandombetween(CRATES_RandomLowDistance, CRATES_RandomHighDistance);

    vecAddTo(*location, center);
    location->x += frandombetween(-smUniverseSizeX / 3, smUniverseSizeX / 3);
    location->y += frandombetween(-smUniverseSizeY / 3, smUniverseSizeY / 3);
    location->z += frandombetween(-smUniverseSizeZ / 3, smUniverseSizeZ / 3);

    displacement.x = frandombetween(-1000.0f, 1000.0f);
    displacement.y = frandombetween(0.0f, 2000.0f);
    displacement.z = frandombetween(-1000.0f, 1000.0f);
    while (!cratesIsLocationGood(location))
    {
        vecAddTo(*location, displacement);
    }

    univClipPointToWorldBox(&center, location);
}



void cratesUpdate()
{
    Derelict *crate;
    Node *node;
    sdword numChecked;

    //only allow crates for multiplayer games
    if(singlePlayerGame)
        return;

    if (gatherStats|showStatsFight|showStatsFancyFight)
        return;

    if((tpGameCreated.flag & MG_CratesEnabled) == 0)
        return;

    universe.crateTimer += universe.unknown1;
    if(universe.crateTimer >= CRATES_DoWeNeedToAddCratesCheckTime)
    {
        if (universe.numCratesInWorld < (CRATES_MaximumCratesInTheWorldPerPlayer*universe.numPlayers))
        {
            //time to do a crate addition check
            real32 prob;universe.crateTimer = 0.0f;
            prob = frandombetween(0.0f,1.0f);
            if(prob > CRATES_AddACrateProb)
            {
                cratesPlaceCrateSelectivly();
            }
        }
    }

    if( (universe.univUpdateCounter & CRATES_ArePlayersNearUpdateMask) == CRATES_ArePlayersNearUpdateFrame)
    {
        //time to do a crate nearity check ;)

        if(universe.numCratesInWorld > 0)
        {
            //lets do the distance checking for all crates;
            node = universe.DerelictList.head;
            numChecked = 0;
            while(node != NULL && numChecked < universe.numCratesInWorld)
            {
                crate = (Derelict *) listGetStructOfNode(node);

                if(crate->derelicttype == Crate)
                {
                    numChecked++;

                    //do time expiry checked here...
                    if((universe.totaltimeelapsed - crate->creationTime) > CRATE_EXPIRY_TIME)
                    {
                        expireCrate(crate);
                    }
                    else
                    {
                        cratesScanForPlayersNearCrateAndReward(crate);
                    }
                }

                node = node->next;
            }

        }
    }
}
void expireCrate(Derelict *crate)
{
    etglod *etgLOD;
    etgeffectstatic *stat;
    vector zero = {0.0f,0.0f,0.0f};
    matrix ident = IdentityMatrix;

    universe.numCratesInWorld--;
#ifdef HW_DEBUG
    //don't drop below 0!
    dbgAssert(universe.numCratesInWorld >= 0);
#endif

    //delete target
    AddTargetToDeleteList((SpaceObjRotImpTarg *)crate,-1);

    //play effect
    etgLOD = etgSpecialPurposeEffectTable[EGT_CRATE_TIME_OUT];

    if (etgLOD != NULL)
    {
        if (crate->currentLOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[crate->currentLOD];
        }
    }
    else
    {
        stat = NULL;
    }
    #if ETG_DISABLEABLE
    if (stat != NULL && etgEffectsEnabled)
    #else
    if (stat != NULL)
    #endif
    {
        etgEffectCreate(stat, NULL, &crate->posinfo.position, &zero, &ident, 1.0f, EAF_AllButNLips, 0);
    }

}

void crateInit()
{
    sdword i;
    for(i=0;i<=STD_LAST_SHIP;i++)
    {
        NUM_IN_GROUPS[i] = 1;
        SHIP_PROBS[i] = 0.0f;
    }

    scriptSet(NULL,"Crates.script",cratesScriptTable);

    NUM_IN_CLASSES[0] = 0;    //mothership not applicable
    NUM_IN_CLASSES[CLASS_HeavyCruiser] = sizeof(cratesCLASS_HeavyCruiser)/4;
    NUM_IN_CLASSES[CLASS_Carrier] = sizeof(cratesCLASS_Carrier)/4;
    NUM_IN_CLASSES[CLASS_Destroyer] = sizeof(cratesCLASS_Destroyer)/4;
    NUM_IN_CLASSES[CLASS_Frigate] = sizeof(cratesCLASS_Frigate)/4;
    NUM_IN_CLASSES[CLASS_Corvette] = sizeof(cratesCLASS_Corvette)/4;
    NUM_IN_CLASSES[CLASS_Fighter] = sizeof(cratesCLASS_Fighter)/4;
    NUM_IN_CLASSES[CLASS_Resource] = sizeof(cratesCLASS_Resource)/4;
    NUM_IN_CLASSES[CLASS_NonCombat] = sizeof(cratesCLASS_NonCombat)/4;




}



#if 0  // Cat3: cratesPlaceCrateSelectivly + cratesGetBestCrateLocation (0x460CB0, 0x4618C0) — complex Player struct raw offsets; needs implementation
_DWORD *cratesPlaceCrateSelectivly()
{
_DWORD v1[3]; // [esp+0h] [ebp-Ch] BYREF

sub_4618C0((int)v1);
return univAddDerelict(0, v1);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4618C0
signed int __cdecl cratesGetBestCrateLocation(vector *location)
{
int v1; // edi
Player *players; // eax
float *PlayerMothership; // ecx
Node *head; // ecx
double v5; // st7
void *structptr; // esi
int v7; // edx
int v8; // edi
double v9; // st7
vector *p_a; // esi
Player *v11; // ebx
Node *v12; // eax
double z; // st7
float *v14; // edx
int v15; // ecx
ubyte *p_bounty; // edx
real32 *p_y; // ecx
int numPlayers; // esi
double v19; // st7
double v20; // st6
double v21; // st6
int v22; // ecx
real32 *v23; // eax
PlayerState *p_playerState; // edx
__int64 v26; // [esp+10h] [ebp-84h]
float v27; // [esp+14h] [ebp-80h]
float v28; // [esp+18h] [ebp-7Ch]
int v29; // [esp+1Ch] [ebp-78h]
float v30; // [esp+1Ch] [ebp-78h]
float v31; // [esp+1Ch] [ebp-78h]
float v32; // [esp+1Ch] [ebp-78h]
float v33; // [esp+1Ch] [ebp-78h]
float v34; // [esp+20h] [ebp-74h]
float i; // [esp+24h] [ebp-70h]
float v36; // [esp+28h] [ebp-6Ch] BYREF
float v37; // [esp+2Ch] [ebp-68h]
float v38; // [esp+30h] [ebp-64h]
vector a; // [esp+34h] [ebp-60h] BYREF

v1 = 0;
v36 = 0.0;
v37 = 0.0;
v38 = 0.0;
v29 = 0;
if ( universe.numPlayers )
{
players = universe.players;
do
{
if ( players->playerState != PLAYER_DEAD )
{
PlayerMothership = (float *)players->PlayerMothership;
++v29;
if ( players->PlayerMothership )
{
v36 = v36 + PlayerMothership[22];
v37 = v37 + PlayerMothership[23];
v38 = v38 + PlayerMothership[24];
}
else
{
head = universe.ShipList.head;
v5 = ZERO;
v27 = 0.0;
v28 = 0.0;
if ( universe.ShipList.head )
{
while ( 1 )
{
structptr = head->structptr;
v7 = *((_DWORD *)structptr + 203);
if ( v7 )
{
if ( *(unsigned __int16 *)(v7 + 36) == v1 )
break;
}
head = head->next;
if ( !head )
goto LABEL_11;
}
v5 = *((float *)structptr + 22);
v27 = *((float *)structptr + 23);
v28 = *((float *)structptr + 24);
}
LABEL_11:
v36 = v5 + v36;
v37 = v27 + v37;
v38 = v28 + v38;
}
}
++v1;
players = players + 1;
}
while ( v1 < universe.numPlayers );
}
v8 = 0;
v9 = ONE / (double)v29;
v36 = v9 * v36;
v37 = v9 * v37;
v38 = v9 * v38;
if ( universe.numPlayers )
{
p_a = &a;
v11 = universe.players;
do
{
if ( v11->playerState != PLAYER_DEAD )
{
if ( v11->PlayerMothership )
{
z = v11->PlayerMothership->posinfo.position.z;
v26 = *(_QWORD *)&v11->PlayerMothership->posinfo.position.x;
}
else
{
v12 = universe.ShipList.head;
z = ZERO;
v26 = 0i64;
if ( universe.ShipList.head )
{
while ( 1 )
{
v14 = (float *)v12->structptr;
v15 = *((_DWORD *)v14 + 203);
if ( v15 )
{
if ( *(unsigned __int16 *)(v15 + 36) == v8 )
break;
}
v12 = v12->next;
if ( !v12 )
goto LABEL_25;
}
z = v14[24];
v26 = *((_QWORD *)v14 + 11);
}
}
LABEL_25:
p_a->x = *(float *)&v26 - v36;
p_a->y = *((float *)&v26 + 1) - v37;
p_a->z = z - v38;
vecNormalize(p_a);
}
++v8;
v11 = v11 + 1;
++p_a;
}
while ( v8 < universe.numPlayers );
}
if ( universe.numPlayers )
{
p_bounty = &universe.players[0].bounty;
p_y = &a.y;
numPlayers = universe.numPlayers;
do
{
if ( *(_DWORD *)(p_bounty - 15) != 1 )
{
v19 = (double)*p_bounty;
if ( v19 <= ONE )
v20 = ONE;
else
v20 = v19;
*(p_y - 1) = ONE / v20 * *(p_y - 1);
if ( v19 <= ONE )
v21 = ONE;
else
v21 = v19;
*p_y = ONE / v21 * *p_y;
if ( v19 <= ONE )
v19 = ONE;
p_y[1] = ONE / v19 * p_y[1];
}
p_bounty += 25904;
p_y += 3;
--numPlayers;
}
while ( numPlayers );
}
v22 = 0;
a1->x = 0.0;
a1->y = 0.0;
a1->z = 0.0;
if ( universe.numPlayers )
{
v23 = &a.y;
p_playerState = &universe.players[0].playerState;
do
{
if ( *p_playerState != PLAYER_DEAD )
{
a1->x = *(v23 - 1) + a1->x;
a1->y = *v23 + a1->y;
a1->z = v23[1] + a1->z;
}
++v22;
p_playerState += 6476;
v23 += 3;
}
while ( v22 < universe.numPlayers );
}
vecNormalize(a1);
a1->x = ((double)ranRandomFn(10) * ((flt_8787CC - flt_8787C8) * flt_72A4E0) + flt_8787C8) * a1->x;
a1->y = ((double)ranRandomFn(10) * ((flt_8787CC - flt_8787C8) * flt_72A4E0) + flt_8787C8) * a1->y;
a1->z = ((double)ranRandomFn(10) * ((flt_8787CC - flt_8787C8) * flt_72A4E0) + flt_8787C8) * a1->z;
a1->x = v36 + a1->x;
a1->y = v37 + a1->y;
a1->z = v38 + a1->z;
v30 = smUniverseSizeX * flt_72A524;
a1->x = (double)ranRandomFn(10) * ((smUniverseSizeX * flt_72A520 - v30) * flt_72A4E0) + v30 + a1->x;
v31 = smUniverseSizeY * flt_72A524;
a1->y = (double)ranRandomFn(10) * ((smUniverseSizeY * flt_72A520 - v31) * flt_72A4E0) + v31 + a1->y;
v32 = smUniverseSizeZ * flt_72A524;
a1->z = (double)ranRandomFn(10) * ((smUniverseSizeZ * flt_72A520 - v32) * flt_72A4E0) + v32 + a1->z;
v33 = (double)ranRandomFn(10) * flt_72A7B8 - flt_72A438;
v34 = (double)ranRandomFn(10) * flt_72A7B8 - flt_72A438 + flt_72A438;
for ( i = (double)ranRandomFn(10) * flt_72A7B8 - flt_72A438; !sub_4617F0(&a1->x); a1->z = i + a1->z )
{
a1->x = v33 + a1->x;
a1->y = v34 + a1->y;
}
return univClipPointToWorldBox(&v36, &a1->x);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4617F0
signed int __cdecl cratesIsLocationGood(vector *location)
{
Node *head; // ebx
unsigned __int8 v2; // c0
unsigned __int8 v3; // c3
int *v4; // ecx
int v5; // edi
int v6; // esi
_DWORD *v7; // ecx
char v9; // c0

head = universe.collBlobList.head;
if ( !universe.collBlobList.head )
return 1;
while ( 1 )
{
if ( !(v2 | v3) )
{
v4 = (int *)*((_DWORD *)head->structptr + 34);
v5 = *v4;
v6 = 0;
if ( *v4 > 0 )
break;
}
LABEL_7:
head = head->next;
if ( !head )
return 1;
}
v7 = v4 + 2;
while ( !v9 )
{
++v6;
++v7;
if ( v6 >= v5 )
goto LABEL_7;
}
return 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x460D60
char __cdecl cratesPlayerFoundACrate(Derelict *crate, Player *player)
{
signed int v2; // edi
uword curPlayerIndex; // ax
struct Player *v4; // edi
int v5; // eax
int v6; // edi
int v8[3]; // [esp+Ch] [ebp-30h] BYREF
int v9[9]; // [esp+18h] [ebp-24h] BYREF

qmemcpy(v9, &unk_72AF18, sizeof(v9));
memset(v8, 0, sizeof(v8));
--universe.numCratesInWorld;
v2 = cratesRewardPlayer((float *)a1, a2);
curPlayerIndex = universe.curPlayerIndex;
if ( a2->playerIndex == universe.curPlayerIndex )
{
soundEventPlay(0.0, 296, 0);
curPlayerIndex = universe.curPlayerIndex;
}
if ( v2 )
{
if ( v2 == 1 )
{
v4 = (struct Player *)dword_C41CFC;
if ( a2->playerIndex == curPlayerIndex )
speechEventQueue(0, 33558600, -1, -1, 0, a2->playerIndex, -1, 5.0, -1);
}
else if ( v2 == 2 )
{
v4 = (struct Player *)dword_C41D00;
if ( a2->playerIndex == curPlayerIndex )
speechEventQueue(0, 33558602, -1, -1, 0, a2->playerIndex, -1, 5.0, -1);
}
else
{
dbgFatalf(0, 0, aUnknownCrateCo);
v4 = a2;
}
}
else
{
v4 = (struct Player *)dword_C41CF8;
if ( a2->playerIndex == curPlayerIndex )
speechEventQueue(0, 33558601, -1, -1, 0, a2->playerIndex, -1, 5.0, -1);
}
if ( v4 )
{
v5 = *(unsigned __int8 *)(a1 + 52);
if ( v5 < (int)v4->PlayerMothership )
{
v6 = *(&v4->resourceUnits + v5);
if ( v6 )
etgEffectCreate(v6, 0, (_DWORD *)(a1 + 88), v8, v9, 1.0, 56, 0);
}
}
return sub_545DD0(a1, -1);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x460CD0
_DWORD *__cdecl cratesReportCratePlacement(Derelict *crate)
{
_DWORD *result; // eax
int v2; // ecx
int v3[3]; // [esp+8h] [ebp-30h] BYREF
int v4[9]; // [esp+14h] [ebp-24h] BYREF

result = (_DWORD *)gameIsRunning;
qmemcpy(v4, &unk_72AF18, sizeof(v4));
memset(v3, 0, sizeof(v3));
++universe.numCratesInWorld;
if ( gameIsRunning )
{
result = (_DWORD *)dword_C41CF4;
if ( dword_C41CF4 )
{
v2 = *(unsigned __int8 *)(a1 + 52);
if ( v2 < *(_DWORD *)dword_C41CF4 )
{
result = *(_DWORD **)(dword_C41CF4 + 4 * v2 + 4);
if ( result )
return etgEffectCreate((int)result, 0, (_DWORD *)(a1 + 88), v3, v4, 1.0, 56, 0);
}
}
}
return result;
}
#endif
