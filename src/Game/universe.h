/*=============================================================================
    Name    : Universe.h
    Purpose : Definitions for Universe.c

    This module will be able to create mission spheres, and the universe
    conceptually contains all of the mission spheres in a game.

    It will also contain information global to all mission spheres, such as the
    static properties of the different ship types.

    Created 6/19/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___UNIVERSE_H
#define ___UNIVERSE_H

#include "CameraCommand.h"
#include "CommandLayer.h"
#include "ConsMgr.h"
#include "GameStats.h"
#include "ResearchAPI.h"
#include "Star3d.h"
#include "Task.h"

/*=============================================================================
    Switches:
=============================================================================*/
#ifdef HW_BUILD_FOR_DEBUGGING

#define UNIV_SHIP_LOADFREE_LOG          1       //spit out a list of what ships were loaded and freed
#define UNIV_LOAD_FREE_FILENAME         "loadfree.log"

#else

#define UNIV_SHIP_LOADFREE_LOG          0       //spit out a list of what ships were loaded and freed

#endif

/*=============================================================================
    Defines:
=============================================================================*/

#define UNIVERSE_UPDATE_RATE_ORIG   16          // original update rate
extern ubyte UNIVERSE_UPDATE_RATE_SHIFT;          // define multiplier in shift values (e.g. 2^n) to preserve original timing, set to 2 in universe.c
                                                // original behaviour with 16 updates per second (shift=0) can be restored with /legacy command line option.
#define UNIVERSE_UPDATE_RATE_FACTOR (1<<UNIVERSE_UPDATE_RATE_SHIFT)
#define UNIVERSE_UPDATE_RATE (UNIVERSE_UPDATE_RATE_ORIG*UNIVERSE_UPDATE_RATE_FACTOR)
#define UNIVERSE_UPDATE_PERIOD (1.0f/(real32)UNIVERSE_UPDATE_RATE)
#define UNIVERSE_UPDATE_STACK_SIZE  100000

// macro to adapt the original update rate to the higher one
#define UNIVERSE_WOODPECKER(r, f) (((universe.univUpdateCounter % UNIVERSE_UPDATE_RATE_FACTOR) == 0) && (((universe.univUpdateCounter >> UNIVERSE_UPDATE_RATE_SHIFT) & r) == (f)))

#define RENDER_STACK_SIZE 100000

#define UNIV_NUMBER_WORLDS          3           //number of planets we can keep track of

/*=============================================================================
    Types:
=============================================================================*/

#define MAXNUM_RESOURCETYPES   10

// for regrowing resources
typedef struct ResourceDistribution
{
    uword probResources[MAXNUM_RESOURCETYPES][2];
    uword cumResources[MAXNUM_RESOURCETYPES][2];
    uword sumTotal[2];
} ResourceDistribution;

typedef enum ResourceVolumeType {
    ResourceSphereType,
    ResourceCylinderType,
    ResourceRectangleType
} ResourceVolumeType;

typedef struct ResourceVolume
{
    Node link;
    ResourceVolumeType resourceVolumeType;
    vector centre;
    real32 radius;
    sdword number;
    sdword actualnumber;
    udword attributes;
    sword attributesParam;
    ubyte attributesPad[2];
    ObjType resourceObjType;
    ResourceDistribution resourceDistribution;

    real32 length;      // only valid for ResourceCylinderType and ResourceRectangleType
    real32 roty;
    real32 rotz;
} ResourceVolume;

typedef enum PlayerState
{
    PLAYER_ALIVE = 0,
    PLAYER_DEAD
} PlayerState;

typedef struct
{
    sdword level;
    bool32 moduleTechEnabled[9];
    udword mask;
} PlayerShipUpgradeState;

#define PLAYER_SHIP_PREREQ_TECH_COUNT 41
#define PLAYER_SHIP_PREREQ_STATE_INDEX PLAYER_SHIP_PREREQ_TECH_COUNT
#define PLAYER_SHIP_PREREQ_ENTRY_COUNT (PLAYER_SHIP_PREREQ_STATE_INDEX + 1)

typedef struct
{
    sdword value[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
} PlayerShipPrereqMask;

typedef struct
{
    union
    {
        ubyte bytes[0x250];
        sdword beastTechGroupUnlockedByShipType[0x250 / sizeof(sdword)];
        struct
        {
            ubyte cataUnknown00[0x04];
            sdword beastCanBuildAcolyte;
            ubyte cataUnknown08[0x08];
            sdword beastCanBuildACV;
            ubyte cataUnknown14[0x04];
            sdword beastCanBuildRammingFrigate;
            sdword beastCanBuildMultiBeamFrigate;
            sdword beastCanBuildHiveFrigate;
            ubyte cataUnknown24[0x20];
            sdword beastCanBuildSentinel;
            ubyte cataUnknown48[0x18];
            sdword beastCanBuildByShipType[(0x250 - 0x60) / sizeof(sdword)];
        };
    };
} PlayerCataPreFogUnknown2Runtime;

typedef struct FogGridSaveState
{
    ubyte *data;
    sdword width;
    sdword height;
    sdword depth;
    udword unknown[4];
} FogGridSaveState;

typedef struct
{
    sdword                        classTotalsPrefix[CLASS_Mine];
    PlayerShipPrereqMask          currentTechMask;
    bool32                        canDoResearch;
    PlayerShipPrereqMask         *discoveredShipPrereqs;
    PlayerCataResearchRuntime     researchRuntime;
    PlayerShipPrereqMask          referenceShipPrereqs[TOTAL_NUM_SHIPS];
    PlayerCataPreFogUnknown2Runtime preFogUnknown2;
} PlayerCataResearchOverlay;

typedef struct Player
{
    Ship *PlayerMothership;
    sdword resourceUnits;
    udword unk;
    sdword supportUnitsMax;
    sdword supportUnitsUsed;
    ShipRace race;
    PlayerState playerState;
    struct AIPlayer *aiPlayer;      // if non-null, then player has a computer AI Player
    udword autoLaunch;
    uword playerIndex;
    ubyte sensorLevel;
    ubyte bounty;                   //range from 0 to 100
    real32 timeMoShipAttacked;
    sdword totalships;                   // total ships for player
    sdword shiptotals[TOTAL_NUM_SHIPS];  // totals for each type of ship
    union
    {
        struct
        {
            sdword classtotals[NUM_CLASSES];
            sdword cataUnknown0;
            sdword cataUnknown1;
            PlayerResearchInfo researchinfo;    // legacy/HW1-shaped overlap, not Cataclysm authoritative
            uword Allies;                       // bitmask of players that i am allied with
            uword AllianceProposals;
            udword AllianceProposalsTimeout;    // timeout counter for proposals to expire!
            sdword initialShipCost;				// total ship cost at game start
            union
            {
                ubyte cataPreFogState[25084];
                struct
                {
                    ubyte cataPreFogUnknown0[0x4C];
                    PlayerShipPrereqMask *discoveredShipPrereqs;
                    PlayerCataResearchRuntime cataResearchRuntime;
                    PlayerShipPrereqMask referenceShipPrereqs[TOTAL_NUM_SHIPS];
                    PlayerCataPreFogUnknown2Runtime cataPreFogUnknown2;
                };
            };
        };
        PlayerCataResearchOverlay cataResearchOverlay;
    };
    FogGridSaveState *fogGrid;
    union
    {
        ubyte cataPostFogState[44];
        PlayerShipUpgradeState shipUpgrades;
    };
} Player;

static_assert(offsetof(Player, aiPlayer) == 0x1C, "Player aiPlayer offset mismatch");
static_assert(offsetof(Player, fogGrid) == 25856, "Player fogGrid offset mismatch");
static_assert(sizeof(PlayerShipPrereqMask) == 0xA8, "PlayerShipPrereqMask size mismatch");
static_assert(sizeof(PlayerShipUpgradeState) == 44, "PlayerShipUpgradeState size mismatch");
static_assert(sizeof(PlayerCataPreFogUnknown2Runtime) == 0x250, "Player cataPreFogUnknown2 runtime size mismatch");
static_assert(sizeof(PlayerCataResearchOverlay) == 0x628C, "Player Cataclysm research overlay size mismatch");
static_assert(offsetof(PlayerCataResearchOverlay, currentTechMask) == 0x30, "Player Cataclysm currentTechMask layout mismatch");
static_assert(offsetof(PlayerCataResearchOverlay, canDoResearch) == 0xD8, "Player Cataclysm canDoResearch layout mismatch");
static_assert(offsetof(PlayerCataResearchOverlay, discoveredShipPrereqs) == 0xDC, "Player Cataclysm discoveredShipPrereqs layout mismatch");
static_assert(offsetof(PlayerCataResearchOverlay, researchRuntime) == 0xE0, "Player Cataclysm researchRuntime layout mismatch");
static_assert(offsetof(PlayerCataResearchOverlay, referenceShipPrereqs) == 0x114, "Player Cataclysm referenceShipPrereqs layout mismatch");
static_assert(offsetof(PlayerCataResearchOverlay, preFogUnknown2) == 0x603C, "Player Cataclysm preFogUnknown2 layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildAcolyte) == 0x04, "Player cataPreFogUnknown2 beastCanBuildAcolyte layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildACV) == 0x10, "Player cataPreFogUnknown2 beastCanBuildACV layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildRammingFrigate) == 0x18, "Player cataPreFogUnknown2 beastCanBuildRammingFrigate layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildMultiBeamFrigate) == 0x1C, "Player cataPreFogUnknown2 beastCanBuildMultiBeamFrigate layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildHiveFrigate) == 0x20, "Player cataPreFogUnknown2 beastCanBuildHiveFrigate layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildSentinel) == 0x44, "Player cataPreFogUnknown2 beastCanBuildSentinel layout mismatch");
static_assert(offsetof(PlayerCataPreFogUnknown2Runtime, beastCanBuildByShipType) == 0x60, "Player cataPreFogUnknown2 beastCanBuildByShipType layout mismatch");
static_assert(offsetof(Player, supportUnitsMax) == 0x0C, "Player struct layout mismatch");
static_assert(offsetof(Player, supportUnitsUsed) == 0x10, "Player struct layout mismatch");
static_assert(offsetof(Player, race) == 0x14, "Player struct layout mismatch");
static_assert(offsetof(Player, aiPlayer) == 0x1C, "Player struct layout mismatch");
static_assert(offsetof(Player, playerIndex) == 0x24, "Player struct layout mismatch");
static_assert(offsetof(Player, cataResearchOverlay.currentTechMask) == 0x2A4, "Player Cataclysm currentTechMask layout mismatch");
static_assert(offsetof(Player, cataResearchOverlay.canDoResearch) == 0x34C, "Player Cataclysm canDoResearch layout mismatch");
static_assert(offsetof(Player, discoveredShipPrereqs) == 0x350, "Player discoveredShipPrereqs layout mismatch");
static_assert(offsetof(Player, cataResearchRuntime) == 0x354, "Player cataResearchRuntime layout mismatch");
static_assert(offsetof(Player, referenceShipPrereqs) == 0x388, "Player referenceShipPrereqs layout mismatch");
static_assert(offsetof(Player, cataResearchOverlay.preFogUnknown2) == 0x62B0, "Player preFogUnknown2 absolute layout mismatch");
static_assert(offsetof(Player, cataResearchOverlay.preFogUnknown2.beastTechGroupUnlockedByShipType) == 0x62B0, "Player beastTechGroupUnlockedByShipType layout mismatch");
static_assert(offsetof(Player, shipUpgrades) == 25860, "Player shipUpgrades layout mismatch");
static_assert(sizeof(Player) == 25904, "Player struct size mismatch");

typedef struct
{
    CommandLayer mainCommandLayer;      // everything goes through main command layer
    CameraCommand mainCameraCommand;    // camera command (focus stack)

    LinkedList RenderList;      // Linked list of all objects (SpaeceObj's to be rendered)
    LinkedList MinorRenderList;
    bool32 dontUpdateRenderList;
    union
    {
        real32 unknown1;            /* legacy name still used by many callers */
        real32 phystimeelapsed;     /* retail current frame physics delta at 0xBC */
    };
    real32 totaltimeelapsed;        /* cumulative simulation time */
    real32 realtimeelapsed;     /* cumulative wall-clock time */
    real32 radius;

    LinkedList SpaceObjList;
    LinkedList MinorSpaceObjList;
    LinkedList effectList;
    LinkedList ShipList;
    LinkedList BulletList;
    LinkedList ResourceList;
    LinkedList DerelictList;
    LinkedList ImpactableList;
    LinkedList MissileList;
    LinkedList MineFormationList;

    LinkedList DeleteResourceList;
    LinkedList DeleteDerelictList;
    LinkedList DeleteShipList;
    LinkedList DeleteMissileList;

    LinkedList collBlobList;

    // tactics stuff
    LinkedList RetreatList;

    // regrow resources stuff
    LinkedList ResourceVolumeList;

    GrowSelection HousekeepShipList;
    GrowSelection unk1;

    Star3dInfo *star3dinfo;
    udword univUpdateCounter;
    color backgroundColor;
    uword resourceNumber;
    uword derelictNumber;
    uword shipNumber;
    uword missileNumber;

    real32 wintime;             // time to display the win game message
    real32 quittime;            // time to quit the game

    udword lasttimeadded;       // for resource injections

    uword shipMaxUnits[TOTAL_NUM_SHIPS];

    sbyte aiplayerEnemy[MAX_MULTIPLAYER_PLAYERS];
    bool32  aiplayerProcessing;

    sdword CapitalShipCaptured;     // bit field of players who have had a ship captured
    sdword PlayerWhoWon;            // bit field of players who have captured a enemy's ship

    sdword bounties;
    sdword bountySize;

    GameStats gameStats;
    bool32 DerelictTech;              //variable that needs to be saved that controls the salvagability of technology holding derelicts

    real32 crateTimer;
    udword numCratesInWorld;
    bool32 collUpdateAllBlobs;

    uword numPlayers;
    uword curPlayerIndex;
    Player *curPlayerPtr;
    Player players[MAX_MULTIPLAYER_PLAYERS+1];      // players[MAX_MULTIPLAYER_PLAYER] is a enemy of all players and used for autoguns, etc.
    Derelict *world[UNIV_NUMBER_WORLDS];        //list of planets
} Universe;

static_assert(offsetof(Universe, phystimeelapsed) == 0xBC, "Universe struct layout mismatch");
static_assert(offsetof(Universe, totaltimeelapsed) == 0xC0, "Universe struct layout mismatch");
static_assert(offsetof(Universe, realtimeelapsed) == 0xC4, "Universe struct layout mismatch");
static_assert(offsetof(Universe, radius) == 0xC8, "Universe struct layout mismatch");
static_assert(offsetof(Universe, ResourceVolumeList) == 0x18C, "Universe struct layout mismatch");
static_assert(offsetof(Universe, HousekeepShipList) == 0x198, "Universe struct layout mismatch");
static_assert(offsetof(Universe, unk1) == 0x1A0, "Universe struct layout mismatch");
static_assert(offsetof(Universe, star3dinfo) == 0x1A8, "Universe struct layout mismatch");
static_assert(offsetof(Universe, shipNumber) == 0x1B8, "Universe struct layout mismatch");
static_assert(offsetof(Universe, bountySize) == 0x304, "Universe struct layout mismatch");
static_assert(offsetof(Universe, gameStats) == 0x308, "Universe struct layout mismatch");
static_assert(offsetof(Universe, collUpdateAllBlobs) == 0x4AF4, "Universe struct layout mismatch");
static_assert(offsetof(Universe, numPlayers) == 0x4AF8, "Universe struct layout mismatch");
static_assert(offsetof(Universe, curPlayerPtr) == 0x4AFC, "Universe struct layout mismatch");
static_assert(sizeof(Universe) == 0x3D9BC, "Universe struct layout mismatch");

extern GrowSelection ClampedShipList;       // special GrowSelect which can have NULL's in it


typedef void (*univstatcallback)(StaticInfo *info, ObjType objType);

/*=============================================================================
    Public Variables
=============================================================================*/

extern Universe universe;
extern real32 universeRealtimeElapsed;
#define collUpdateAllBlobs universe.collUpdateAllBlobs
extern sdword currentSensorCoverage;
extern sdword overlayPauseActive;
extern AsteroidStaticInfo asteroidStaticInfos[NUM_ASTEROIDTYPES];
extern DustCloudStaticInfo dustcloudStaticInfos[NUM_DUSTCLOUDTYPES];
extern GasCloudStaticInfo gascloudStaticInfos[NUM_GASCLOUDTYPES];
extern DerelictStaticInfo derelictStaticInfos[NUM_DERELICTTYPES];
extern NebulaStaticInfo nebulaStaticInfos[NUM_NEBULATYPES];

extern Camera defaultCamera;

extern MissileStaticInfo missileStaticInfos[NUM_RACES];
extern MissileStaticInfo mineStaticInfos[NUM_RACES];

extern struct SphereStaticInfo *sphereStaticInfo;

extern real32 maxMineCollSphereSize;
//extern real32 maxShipCollSphereSize;
//extern real32 maxResourceCollSphereSize;
//extern real32 maxDerelictCollSphereSize;
//extern real32 maxCollSphereSize;

extern meshdata *defaultmesh;

extern uword RacesAllowedForGivenShip[TOTAL_NUM_SHIPS];

extern ubyte turboTimeCompressionFactor;
extern bool32 universeTurbo;
extern bool32 universePause;

extern sdword cdMaxShipsAllowed;            // max number of ships allowed
extern sdword cdLimitCaps[TOTAL_NUM_SHIPS]; // max number of ships per player allowed
extern sdword cdClassCaps[NUM_CLASSES];     // max number of ships per class allowed
extern bool32   cdEnabled;                    // flag specifying whether unit caps enabled

extern lodinfo *defaultlod;                 // remove later when all stubs are taken out
extern void *defaultmex;                    // remove later when all stubs are taken out
extern bool32 universeForceDefaultShip;       //flag for forcing ships to be loaded as rave borgs

#if UNIV_SHIP_LOADFREE_LOG
extern bool32 univLoadFreeLog;
#endif

extern real32 rowAvoidBy[TOTAL_NUM_SHIPS];
extern sbyte rightOfWays[TOTAL_NUM_SHIPS];

#ifdef DEBUG_GAME_STATS

typedef struct
{
    sdword numPlayers;
    char names[MAX_MULTIPLAYER_PLAYERS+1][20];
}GameStatsDebugHeader;

void gameStatsInitLog();
void gameStatsUpdateLogFile(sdword framenum);

#endif
/*=============================================================================
    Functions:
=============================================================================*/

void universeClosePlayers(void);
void universeInit(void);
void universeClose(void);
void universeReset(void);
void universeSetZeroPlayers(void);
void universeStaticInit(void);
void universeStaticClose(void);
DECLARE_TASK(universeUpdateTask);
void universeSwitchToNextPlayer(void);
void universeSwitchToPlayer(uword playerIndex);
sdword univSetCurrentSensorCoverage(sdword sensorCoverage);
void univSaveCountTables(void);
void univLoadCountTables(void);

ShipRace GetValidRaceForShipType(ShipType shiptype);    // returns valid race for shiptype, -1 if none found
ShipStaticInfo *GetShipStaticInfo(ShipType shiptype,ShipRace shiprace);
ShipStaticInfo *GetShipStaticInfoValidate(ShipType shiptype,ShipRace shiprace); // same as above, but makes sure data loaded
ShipStaticInfo *GetShipStaticInfoSafe(ShipType shiptype,ShipRace shiprace); // same as above, but always safe (returns NULL if fails)
udword ShipStaticInfoGetSupportUnits(ShipStaticInfo *statinfo);
udword ShipStaticInfoGetProvidesSU(ShipStaticInfo *statinfo);
void InitStatShipInfo(ShipStaticInfo *statinfo,ShipType type,ShipRace race);
void CloseStatShipInfo(ShipStaticInfo *statinfo);
void CloseStatAsteroidInfo(AsteroidStaticInfo *asteroidStatInfo);
void CloseStatDerelictInfo(DerelictStaticInfo *derelictStatInfo);
void CloseStatDustCloudInfo(DustCloudStaticInfo *dustcloudStatInfo);
void CloseStatGasCloudInfo(GasCloudStaticInfo *gascloudStatInfo);
void CloseStatMissileInfo(MissileStaticInfo *missileStatInfo);
void CloseStatMineInfo(MissileStaticInfo *mineStatInfo);
void CloseStatEffectInfo(etgeffectstatic *effectStatInfo);
void InitStatDerelictInfoByPath(DerelictStaticInfo *derelictStatInfo, DerelictType derelicttype, char *directory, char *shipFile);

void unitCapCreateShip(Ship *ship, Player *player);
void unitCapDeleteShip(Ship *ship, Player *player);
bool32 unitCapCanCreateShip(ShipType ship, shipsinprogress *factory, shipavailable *cmShipsAvail);
void unitCapInitialize(Player *player);
void unitCapEnable(void);
void unitCapDisable(void);
void cat_unitCapCreateShip(Ship *ship, Player *player);
void cat_unitCapDeleteShip_(Ship *ship, Player *player);

void universeRealTimeTweak(SelectCommand *selection);
void universeDefaultTeamColors(void);
int *universeSetMissionShipAvailability(void);
extern void universeFlagRaceNeeded(ShipRace shiprace, bool8 bNeeded);
void universeFlagEverythingNeeded(void);
void universeSaveEverythingNeeded(void);
void universeLoadEverythingNeeded(void);
void universeRaceStaticsScan(ShipRace shiprace, univstatcallback forShips, univstatcallback forMisc);
void universeAllStaticsScan(univstatcallback forShips, univstatcallback forDerelicts, univstatcallback forResources, univstatcallback forMisc);

void resourceInjectionTask(void);
void resourceLumpSum(void);

void initGameStats(Universe *universe);
void gameStatsAddShip(Ship *ship,sdword playerIndex);
void writeGameStatsToFile(char *filename);
void gameStatsShipDied(Ship *ship);
void gameStatsCalcShipCostTotals(void);

bool32 isStrikeCraft(ShipType type);
Ship *univFindNearestShipWithConstraints(Ship *ship, bool32 (__cdecl *constraint)(void *));
sdword univClipPointToWorldBox(vector *src, vector *dst);
sdword shipCanResearch(Ship *ship, sdword requireIdleResearchShip);
udword shipTypeGetUpgradeMask(ShipType shipType);
sdword shipTypeGetMaxUpgradeLevel(ShipType shipType);
sdword shipTypeGetUpgradeMaxLevels(ShipType shipType);
bool32 shipCanDisplayUpgradeType(Ship *ship, sdword upgradeType);
bool32 shipHasAvailableUpgradeLevel(Ship *ship);
udword shipGetAvailablePlayerUpgradeMask(Ship *ship);
real32 shipUpgradeDurationUnitsGet(udword upgradeMask);
real32 shipGetUpgradeBuildTime(Ship *ship);
udword shipCollectInstalledUpgradeTypeFlags(Ship *ship, udword *typeFlags);
bool32 shipHasAvailableUpgradeChanges(Ship *ship);
BOOL IsShipInfected(Ship *ship);
void shipApplyPlayerUpgrades(Ship *ship);
bool32 shipTypeSupportsModuleSlots(ShipType shipType);
void shipUpdateModuleSlots(Ship *ship);
void playerInitUpgradeLevels(Player *player);
sdword shipDisable(Ship *ship, sdword allowDelay);
void shipUpdateResearchableUpgradeMask(Ship *ship);
sdword shipSetSlotStatus(Ship *ship, sdword playerIndex, sdword status, sdword skipEnemyCheck);
void shipResetSlotStatusesForAllPlayers(Ship *ship);
void shipUpdateSlotStatusesForAllPlayers(Ship *ship);
BOOL shipToggleCloak(Ship *ship);
bool32 shipGetCloakChargeFraction(Ship *ship, real32 *fraction);
bool32 shipGetCloakTransitionBlend(Ship *ship, real32 *blend);
BOOL univIsShipCloaked(SpaceObj *obj);
void shipCloakUpdate(Ship *ship);
void shipInitCloakState(Ship *ship, real32 *cloakState, real32 cloakDrainPerTick, real32 decloakDrainPerTick);
typedef struct Shockwave Shockwave;
bool32 bulletIsShockwaveProjectile(SpaceObjRotImpTarg *projectile);
void shockwaveCreateBasic(real32 x, real32 y, real32 z);
Shockwave *shockwaveCreateTyped(sdword shockwaveType, udword x, udword y, udword z, uword playerIndex);
Shockwave *shockwaveCreateProjectileImpact(real32 x, real32 y, real32 z, uword playerIndex, real32 damageScale);
bool32 shockwaveHasActiveShockwaves(void);
sdword shockwaveProjectileHitStationary(Bullet *projectile);
sdword shockwaveProjectileBulletHit(vector *position, Bullet *projectile);
void shockwaveUpdateAll(void);
sdword shockwaveStartup(void);
void shockwaveShutdown(void);
sdword shockwaveRenderAll(void);
void shockwaveRegisterProjectile(SpaceObjRotImpTarg *projectile);
void shockwaveUnregisterProjectile(SpaceObjRotImpTarg *projectile);
bool32 univShipModuleSlotIsEligible(Ship *ship, Player *player);
bool32 univShipModuleSlotIsEligibleUpgrade(Ship *ship, Player *player);
bool32 shipHasModuleSlotState(Ship *ship);
sdword univGetShipRace(SpaceObj *obj);
bool32 shipIsModuleSlotActive(Ship *ship, Player *player);
bool32 univPlayerHasShipType(ShipType shiptype, Player *player);

void unitCapInitStatics(udword numplayers);

#endif
