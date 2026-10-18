// =============================================================================
//  GameStats.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 06/19/1999 by bpasechnik
// =============================================================================

#ifndef ___GAMESTATS_H
#define ___GAMESTATS_H

//[x] beside statistics that are functional

//resource units doesn't handle retired resources...

typedef struct PlayerStats
{
    real32 timeOfDeath;                                 //[x]
    sdword typeOfPlayerDeath;                           //[x]
    sdword totalResourceUnits;                          //[]    //this value always goes up...except when giving RU's away
    sdword totalResourceUnitsCollected;                 //[x]   //all resource units collected via harvesting/salvaging of ships
    sdword totalResourceUnitsSpent;                     //[x]
    sdword totalResourceUnitsGiven;                     //[x]
    sdword totalResourceUnitsRecieved;                  //[x]
    sdword totalRegeneratedResourceUnits;               //[x]
    sdword totalInjectedResources;                      //[x]
    sdword totalResourceUnitsViaBounties;               //[x]
    sdword totalCrystalsRetrieved;                      //[x]   // Cataclysm-only crystal harvesting count

    sdword totalRUsInCurrentShips;                      //[x]  //value of ships player currently has
    sdword totalShips;                                  //[x]
    sdword totalForEachShip[TOTAL_NUM_SHIPS];           //[x]
    sdword totalForEachClass[NUM_CLASSES];              //[x]

    sdword totalKills;                                  //[x]
    sdword totalRUsKilled;                              //[x]
    sdword totalKillsForEachShip[TOTAL_NUM_SHIPS];      //[x]
    sdword totalKillsForEachClass[NUM_CLASSES];         //[x]

    sdword totalLosses;                                 //[x]
    sdword totalRUsLost;                                //[x]
    sdword totalLossesForEachShip[TOTAL_NUM_SHIPS];     //[x]
    sdword totalLossesForEachClass[NUM_CLASSES];        //[x]

    real32 pointsOfDamageDoneToWho[MAX_MULTIPLAYER_PLAYERS];                            //[]
    sdword totalPlayersKilled;                          //[x]   // retail offset 0x7CC
    // Cataclysm extends the HW1 player-stats block by 11 dwords; one is the
    // crystals-retrieved counter above and the retail binary still has 10 more.
    sdword cataUnknown[10];
}PlayerStats;

typedef struct GameStats
{
    sdword totalResourcesInGame;                        //[]
    sdword totalShipsInGame;                            //[] built/aquired
    sdword totalKills;                                  //[x]
    sdword totalRUsInAllShips;                          //[x]
    sdword totalRUsKilled;                              //[x]
    sdword totalRUsLost;                                //[x]
    sdword totalResourceUnitsCollected;                 //[x]  //via harvesting
    real32 updatedRUValuesTime;
    sdword universeRUWorth;
    sdword startingResources;
    PlayerStats playerStats[MAX_MULTIPLAYER_PLAYERS+1];   //[]
}GameStats;

static_assert(offsetof(PlayerStats, totalResourceUnits) == 0x8, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalResourceUnitsCollected) == 0xC, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalResourceUnitsViaBounties) == 0x24, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalCrystalsRetrieved) == 0x28, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalForEachClass) == 0x278, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalKills) == 0x2AC, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalRUsKilled) == 0x2B0, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalKillsForEachShip) == 0x2B4, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalKillsForEachClass) == 0x4F8, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalLosses) == 0x52C, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalRUsLost) == 0x530, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalLossesForEachShip) == 0x534, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalLossesForEachClass) == 0x778, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, pointsOfDamageDoneToWho) == 0x7AC, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, totalPlayersKilled) == 0x7CC, "PlayerStats layout mismatch");
static_assert(offsetof(PlayerStats, cataUnknown) == 0x7D0, "PlayerStats layout mismatch");
static_assert(sizeof(PlayerStats) == 2040, "PlayerStats size mismatch");

#endif
