/*=============================================================================
    Name    : Universe.c
    Purpose : The universe represents the entire game world, including any
              mission spheres contained within.

    Created 6/19/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "debug.h"
#include "task.h"
#include "file.h"
#include "linkedlist.h"
#include "spaceobj.h"
#include "collision.h"
#include "univupdate.h"
#include "stats.h"
#include "camera.h"
#include "cameracommand.h"
#include "render.h"
#include "statscript.h"
#include "ACV.h"
#include "ships.h"
#include "formation.h"
#include "commandlayer.h"
#include "mex.h"
#include "tweak.h"
#include "globals.h"
#include "commandnetwork.h"
#include "utility.h"
#include "levelload.h"
#include "aiship.h"
#include "gun.h"
#include "etg.h"
#include "dock.h"

#include "universe.h"

#include "aiplayer.h"
#include "consMgr.h"
#include "strings.h"
#include "soundevent.h"
#include "horserace.h"
#include "meshanim.h"
#include "multiplayergame.h"
#include "Alliance.h"
#include "gamechat.h"
#include "Randy.h"
#include "researchapi.h"
#include "singleplayer.h"
#include "navlights.h"
#include "teams.h"
#include "captaincy.h"
#include "bounties.h"
#include "InfoOverlay.h"
#include "tutor.h"
#include "Leech.h"
#include "SalCapCorvette.h"

#include "ping.h"
#include "SaveGame.h"
#include "Sensors.h"
#include "autodownloadmap.h"
#include "LagPrint.h"
#include "scenpick.h"
#include "fastmath.h"
#include "netcheck.h"

extern bool32 cheapShips;
extern sdword kbSuppressNextSpecialActivate;
bool32 shipHasModuleSlotState(Ship *ship);
sdword currentSensorCoverage = 0;
void tabStatsResetDrag(void);
void univResetSpaceObjs(void);
void rmCloseIfOpen(void);
void lmCloseIfOpen(void);
extern sdword spStateA5B1AC;

extern real32 GLOBAL_SHIP_HEALTH_MODIFIER;
extern real32 GLOBAL_SHIP_SPEED_MODIFIER;

/*=============================================================================
    Public Data:
=============================================================================*/

Universe universe;
real32 universeRealtimeElapsed = 0.0f;
ubyte UNIVERSE_UPDATE_RATE_SHIFT = 0;
AsteroidStaticInfo asteroidStaticInfos[NUM_ASTEROIDTYPES];
DustCloudStaticInfo dustcloudStaticInfos[NUM_DUSTCLOUDTYPES];
GasCloudStaticInfo gascloudStaticInfos[NUM_GASCLOUDTYPES];
NebulaStaticInfo nebulaStaticInfos[NUM_NEBULATYPES];

DerelictStaticInfo derelictStaticInfos[NUM_DERELICTTYPES];

MissileStaticInfo missileStaticInfos[NUM_RACES];
MissileStaticInfo mineStaticInfos[NUM_RACES];           //one per race in Cataclysm
static udword shipSupportUnits[NUM_RACES][TOTAL_NUM_SHIPS];
static udword shipProvidesSU[NUM_RACES][TOTAL_NUM_SHIPS];

struct SphereStaticInfo *sphereStaticInfo = NULL;

Camera defaultCamera;

real32 maxMineCollSphereSize;
//real32 maxShipCollSphereSize;
//real32 maxResourceCollSphereSize;
//real32 maxDerelictCollSphereSize;
//real32 maxCollSphereSize;

meshdata *defaultmesh = NULL;

#define UNIVERSE_TURBO_REPEAT       19
bool32 universeTurbo = FALSE;

bool32 universePause = FALSE;

sdword cdMaxShipsAllowed;            // max number of ships allowed
sdword cdLimitCaps[TOTAL_NUM_SHIPS]; // max number of ships per player allowed
sdword cdClassCaps[NUM_CLASSES];     // max number of ships per class allowed
bool32   cdEnabled=TRUE;               // flag specifying whether unit caps enabled

uword RacesAllowedForGivenShip[TOTAL_NUM_SHIPS] =
{
    R1_VALID|R2_VALID,                   //AdvanceSupportFrigate
    R1_VALID|R2_VALID,                   //AttackBomber
    R1_VALID|R2_VALID,                   //Carrier
    R1_VALID         ,                   //CloakedFighter
    R1_VALID|R2_VALID,                   //CloakGenerator
    R1_VALID         ,                   //DDDFrigate
             R2_VALID,                   //DefenseFighter
             R2_VALID,                   //DFGFrigate
    R1_VALID|R2_VALID,                   //GravWellGenerator
    R1_VALID|R2_VALID,                   //HeavyCorvette
    R1_VALID|R2_VALID,                   //HeavyCruiser
    R1_VALID|R2_VALID,                   //HeavyDefender
    R1_VALID|R2_VALID,                   //HeavyInterceptor
    R1_VALID|R2_VALID,                   //IonCannonFrigate
    R1_VALID|R2_VALID,                   //LightCorvette
    R1_VALID|R2_VALID,                   //LightDefender
    R1_VALID|R2_VALID,                   //LightInterceptor
    R1_VALID|R2_VALID,                   //MinelayerCorvette
    R1_VALID|R2_VALID,                   //MissileDestroyer
    R1_VALID|R2_VALID,                   //Mothership
    R1_VALID|R2_VALID,                   //MultiGunCorvette
    R1_VALID|R2_VALID,                   //Probe
    R1_VALID|R2_VALID,                   //ProximitySensor
    R1_VALID|R2_VALID,                   //RepairCorvette
    R1_VALID|R2_VALID,                   //ResearchShip
    R1_VALID|R2_VALID,                   //ResourceCollector
    R1_VALID|R2_VALID,                   //ResourceController
    R1_VALID|R2_VALID,                   //SalCapCorvette
    R1_VALID|R2_VALID,                   //SensorArray
    R1_VALID|R2_VALID,                   //StandardDestroyer
    R1_VALID|R2_VALID,                   //StandardFrigate
    R1_VALID         ,                   //Drone
    R1_VALID|R2_VALID,                   //TargetDrone
    R1_VALID|R2_VALID,                   //HeadShotAsteroid
    R1_VALID|R2_VALID,                   //CryoTray
    P1_VALID         ,                   //P1Fighter
    P1_VALID         ,                   //P1IonArrayFrigate
    P1_VALID         ,                   //P1MissileCorvette
    P1_VALID         ,                   //P1Mothership
    P1_VALID         ,                   //P1StandardCorvette
    TAIIDAN_VALID    ,                   //P2AdvanceSwarmer
    TAIIDAN_VALID    ,                   //P2FuelPod
    TAIIDAN_VALID    ,                   //P2Mothership
    TAIIDAN_VALID    ,                   //P2MultiBeamFrigate
    TAIIDAN_VALID    ,                   //P2Swarmer
    KUSHAN_VALID     ,                   //P3Destroyer
    KUSHAN_VALID     ,                   //P3Frigate
    KUSHAN_VALID     ,                   //P3Megaship
    Traders_VALID    ,                   //FloatingCity
    Traders_VALID    ,                   //CargoBarge
    Traders_VALID    ,                   //MiningBase
    Traders_VALID    ,                   //ResearchStation
    Traders_VALID    ,                   //JunkYardDawg
    Traders_VALID                        //JunkYardHQ
};

bool32 universeForceDefaultShip = FALSE;   //flag for forcing ships to be loaded as rave borgs

#if UNIV_SHIP_LOADFREE_LOG
bool32 univLoadFreeLog = FALSE;
#endif

/*=============================================================================
    Private Function prototypes:
=============================================================================*/

static void loadMexDataCB(char *directory,char *field,void *dataToFillIn);
//static void setRaceCB(char *directory,char *field,void *dataToFillIn);
static void setClassCB(char *directory,char *field,void *dataToFillIn);
static void scriptSetFramesCB(char *directory,char *field,void *dataToFillIn);
static void LODScriptLoad(char *directory,char *field,void *dataToFillIn);
static void setExplosionType(char *directory,char *field,void *dataToFillIn);
static void scriptSetShipsCB(char *directory,char *field,void *dataToFillIn);
static void setGunBindInfo(char *directory,char *field,void *dataToFillIn);
static void setAnimationBindInfo(char *directory,char *field,void *dataToFillIn);
static void setSupportUnitsInfo(char *directory, char *field, void *dataToFillIn);
static void setProvidesSUInfo(char *directory, char *field, void *dataToFillIn);
static void setNumberOfAnimsInfo(char *directory,char *field,void *dataToFillIn);
static void setAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setMadStartInfo(char *directory,char *field,void *dataToFillIn);
static void setGunAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setDockAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setMothershipDoorAnimInfo(char *directory,char *field,void *dataToFillIn);
static void setSpecialCaseAnimInfo(char *directory,char *field,void *dataToFillIn);

/*=============================================================================
    Private Data:
=============================================================================*/

lodinfo *defaultlod = NULL;         // remove later when all stubs are taken out
void *defaultmex = NULL;            // remove later when all stubs are taken out

udword universeUpdateCounter = 0;

// Cataclysm uses a flat 1D array indexed by ship type only.
// Each ship type is unique to one race, so no per-race duplication needed.
ShipStaticInfo shipStaticInfos[TOTAL_NUM_SHIPS + 1]; // +1 for DefaultShip

// Legacy pointer array kept for any code that indexes by race
ShipStaticInfo *RaceShipStaticInfos[NUM_RACES] =
{
    shipStaticInfos,    // R1/Sect: offset 0 works because STD_FIRST_SHIP = 0
    shipStaticInfos,    // R2/Beast: same flat array, offset by type
    shipStaticInfos,    // P1
    shipStaticInfos,    // P2
    shipStaticInfos,    // P3
    shipStaticInfos,    // Traders
    shipStaticInfos     // Derelict
};

CustShipHeader *shipHeaders[TOTAL_NUM_SHIPS] =
{
    &GenericInterceptorHeader,            // sRecon (0)
    &AcolyteHeader,                       // sAcolyte (1)
    &MimicHeader,                         // sMimic (2)
    &LeechHeader,                         // sLeech (3)
    &ACVHeader,                           // sACV (4)
    &MCVHeader,                           // sMCV (5)
    &RammingFrigateHeader,                // sRammingFrigate (6)
    &MultiBeamFrigateHeader,              // sMultiBeamFrigate (7)
    &HiveFrigateHeader,                   // sHiveFrigate (8)
    &SwarmerHeader,                       // sSwarmer (9)
    &DreadnoughtHeader,                   // sDreadnought (10)
    &ComponentShipHeader,                 // sDreadnoughtCannon (11)
    &CarrierHeader,                       // sCarrier (12)
    &ComponentShipHeader,                 // sCarrierSupport (13)
    &WorkerHeader,                        // sWorker (14)
    &RepairDroidHeader,                   // sRepairDroid (15)
    &ProcessorHeader,                     // sProcessor (16)
    &SentinelHeader,                      // sSentinel (17)
    &MothershipHeader,                    // sMothership (18)
    &ComponentShipHeader,                 // sMothershipSpecial (19)
    &ComponentShipHeader,                 // sMothershipEngineering (20)
    &ComponentShipHeader,                 // sMothershipArmour (21)
    &ComponentShipHeader,                 // sMothershipMicro (22)
    &ComponentShipHeader,                 // sMothershipWeapons (23)
    &ComponentShipHeader,                 // sMothershipDockingBay (24)
    &StandardDestroyerHeader,             // sDestroyer (25)
    &ComponentShipHeader,                 // sMothershipSupport (26)
    &ComponentShipHeader,                 // sMothershipCommand (27)
    &ComponentShipHeader,                 // sMothershipEnginesLower (28)
    &ComponentShipHeader,                 // sMothershipEnginesUpper (29)
    &ComponentShipHeader,                 // sMothershipLowerSpine (30)
    &ComponentShipHeader,                 // sMothershipBigGun (31)
    &ComponentShipHeader,                 // sMothershipOreCanArmour (32)
    &GenericInterceptorHeader,            // bRecon (33)
    &AcolyteHeader,                       // bAcolyte (34)
    &GenericInterceptorHeader,            // bInterceptor (35)
    &BeastCloakedFighterHeader,           // bCloakedFighter (36)
    &AttackBomberHeader,                  // bAttackBomber (37)
    &ACVHeader,                           // bACV (38)
    &MultiGunCorvetteHeader,              // bMultiGunCorvette (39)
    &HeavyCorvetteHeader,                 // bHeavyCorvette (40)
    &MultiBeamFrigateHeader,              // bMultiBeamFrigate (41)
    &HiveFrigateHeader,                   // bHiveFrigate (42)
    &BeastSwarmerHeader,                  // bSwarmer (43)
    &IonArrayFrigateHeader,               // bIonArrayFrigate (44)
    &DFGFrigateHeader,                    // bDFGFrigate (45)
    &BeastHeavyCruiserHeader,             // bHeavyCruiser (46)
    &CarrierHeader,                       // bCarrier (47)
    &RammingFrigateHeader,                // bRammingFrigate (48)
    &WorkerHeader,                        // bWorker (49)
    &RepairDroidHeader,                   // bRepairDroid (50)
    &ProcessorHeader,                     // bProcessor (51)
    &SentinelHeader,                      // bSentinel (52)
    &BMothershipHeader,                   // bMothership (53)
    &ComponentShipHeader,                 // bMothershipDockingBay (54)
    &ComponentShipHeader,                 // bMothershipEnginesLower (55)
    &ComponentShipHeader,                 // bSectMoship (56)
    &ComponentShipHeader,                 // bMothershipSupport (57)
    &CruiseMissileHeader,                 // bCruiseMissile (58)
    &P1MissileCorvetteHeader,             // bMissileCorvette (59)
    &AdvanceSupportFrigateHeader,         // kAdvanceSupportFrigate (60)
    &AttackBomberHeader,                  // kAttackBomber (61)
    &CarrierHeader,                       // kCarrier (62)
    NULL,                                 // kCloakGenerator (63)
    &GravWellGeneratorHeader,             // kGravWellGenerator (64)
    &HeavyCorvetteHeader,                 // kHeavyCorvette (65)
    &HeavyCruiserHeader,                  // kHeavyCruiser (66)
    &GenericDefenderHeader,               // kHeavyDefender (67)
    &GenericInterceptorHeader,            // kHeavyInterceptor (68)
    &IonCannonFrigateHeader,              // kIonCannonFrigate (69)
    &MultiGunCorvetteHeader,              // kMultiGunCorvette (70)
    &ProximitySensorHeader,               // kProximitySensor (71)
    &SalCapCorvetteHeader,                // kSalCapCorvette (72)
    &StandardDestroyerHeader,             // kStandardDestroyer (73)
    &AdvanceSupportFrigateHeader,         // tAdvanceSupportFrigate (74)
    &AttackBomberHeader,                  // tAttackBomber (75)
    &CarrierHeader,                       // tCarrier (76)
    NULL,                                 // tCloakGenerator (77)
    &DFGFrigateHeader,                    // tDFGFrigate (78)
    &GravWellGeneratorHeader,             // tGravWellGenerator (79)
    &HeavyCruiserHeader,                  // tHeavyCruiser (80)
    &GenericDefenderHeader,               // tHeavyDefender (81)
    &GenericInterceptorHeader,            // tHeavyInterceptor (82)
    &IonCannonFrigateHeader,              // tIonCannonFrigate (83)
    &MinelayerCorvetteHeader,             // tMinelayerCorvette (84)
    &MissileDestroyerHeader,              // tMissileDestroyer (85)
    &MultiGunCorvetteHeader,              // tMultiGunCorvette (86)
    &ProximitySensorHeader,               // tProximitySensor (87)
    &ResourceCollectorHeader,             // tResourceCollector (88)
    &SalCapCorvetteHeader,                // tSalCapCorvette (89)
    &StandardDestroyerHeader,             // tStandardDestroyer (90)
    &StandardFrigateHeader,               // tStandardFrigate (91)
    &DoNothingShipHeader,                 // p1Outpost (92)
    &GenericInterceptorHeader,            // p1Fighter (93)
    &IonArrayFrigateHeader,               // p1IonArrayFrigate (94)
    &P1MissileCorvetteHeader,             // p1MissileCorvette (95)
    &P1MothershipHeader,                  // p1Mothership (96)
    &P1StandardCorvetteHeader,            // p1StandardCorvette (97)
    &FloatingCityHeader,                  // trFloatingCity (98)
    &CargoBargeHeader,                    // trCargoBarge (99)
    &MiningBaseHeader,                    // trMiningBase (100)
    &CargoBargeHeader,                    // trResearchStation (101)
    &JunkYardDawgHeader,                 // trJunkYardDawg (102)
    &DoNothingShipHeader,                 // trJunkYardHQ (103)
    &MothershipHeader,                    // trGhostship (104)
    &StationaryGunHeader,                 // trJunk_LGun (105)
    &StationaryGunHeader,                 // trJunk_SGun (106)
    &DoNothingShipHeader,                 // trResearchStationBridge (107)
    &DoNothingShipHeader,                 // trResearchStationTower (108)
    &RKOHeader,                           // dRKO (109)
    &DoNothingShipHeader,                 // dBigGun (110)
    &DoNothingShipHeader,                 // dBigGunUnit (111)
    &SlipgateHeader,                      // dSlipgateSmall (112)
    &SlipgateHeader,                      // dSlipgateMedium (113)
    &SlipgateHeader,                      // dSlipgateLarge (114)
    &DoNothingShipHeader,                 // dCrate (115)
    &DoNothingShipHeader,                 // dJunk1PartA (116)
    &DoNothingShipHeader,                 // dJunk1Shell (117)
    &DoNothingShipHeader,                 // dJunk1Strut (118)
    &DoNothingShipHeader,                 // dJunk2Shipwreck (119)
    &DoNothingShipHeader,                 // dJunk3Boiler (120)
    &DoNothingShipHeader,                 // dJunk3BoilerCasing (121)
    &DoNothingShipHeader,                 // dMessagePod (122)
    &DoNothingShipHeader,                 // dBuoy (123)
    &DoNothingShipHeader,                 // dFirstPlanet (124)
    &DoNothingShipHeader,                 // dDeadPlanet (125)
    &CrystalHeader,                       // dCrystal0 (126)
    &CrystalHeader,                       // dCrystal1 (127)
    &DoNothingShipHeader,                 // dBeastDockingBay (128)
    &DoNothingShipHeader,                 // dNISBuoy (129)
    &BeastMothershipHeader,              // dBeastMothership (130)
    &NomadMoonHeader,                     // dNomadMoon (131)
    &NomadFieldEmitterHeader,             // dNomadFieldEmitter (132)
    &AcolyteHeader,                       // dAcolyte (133)
    &DoNothingShipHeader,                 // dPrisonShip (134)
    &DoNothingShipHeader,                 // dPrisonShipInfected (135)
    &DoNothingShipHeader,                 // dSlipgateNode (136)
    &CarrierHeader,                       // dKushanCarrier (137)
    &IonCannonFrigateHeader,              // dIonArrayFrigate (138)
    &IonCannonFrigateHeader,              // dIonCannonFrigate (139)
    &DoNothingShipHeader,                 // dEcclesiam (140)
    &DoNothingShipHeader,                 // dEcclesiamInfected (141)
    &DoNothingShipHeader,                 // nisLightSource1 (142)
    &DoNothingShipHeader,                 // nisLightSource2 (143)
    &DoNothingShipHeader                  // nisLensflareSource1 (144)
};

real32 rowAvoidBy[TOTAL_NUM_SHIPS] =
{
    0.0,          // sRecon
    0.0,          // sAcolyte
    0.0,          // sMimic
    0.0,          // sLeech
    0.0,          // sACV
    0.0,          // sMCV
    500.0,        // sRammingFrigate
    500.0,        // sMultiBeamFrigate
    500.0,        // sHiveFrigate
    0.0,          // sSwarmer
    1200.0,       // sDreadnought
    1200.0,       // sDreadnoughtCannon
    1200.0,       // sCarrier
    1200.0,       // sCarrierSupport
    400.0,        // sWorker
    0.0,          // sRepairDroid
    1000.0,       // sProcessor
    0.0,          // sSentinel
    1800.0,       // sMothership
    1800.0,       // sMothershipSpecial
    1800.0,       // sMothershipEngineering
    1800.0,       // sMothershipArmour
    1800.0,       // sMothershipMicro
    1800.0,       // sMothershipWeapons
    1800.0,       // sMothershipDockingBay
    1000.0,       // sDestroyer
    1800.0,       // sMothershipSupport
    1800.0,       // sMothershipCommand
    1800.0,       // sMothershipEnginesLower
    1800.0,       // sMothershipEnginesUpper
    1800.0,       // sMothershipLowerSpine
    1800.0,       // sMothershipBigGun
    1800.0,       // sMothershipOreCanArmour
    0.0,          // bRecon
    0.0,          // bAcolyte
    0.0,          // bInterceptor
    0.0,          // bCloakedFighter
    0.0,          // bAttackBomber
    0.0,          // bACV
    0.0,          // bMultiGunCorvette
    0.0,          // bHeavyCorvette
    500.0,        // bMultiBeamFrigate
    500.0,        // bHiveFrigate
    0.0,          // bSwarmer
    500.0,        // bIonArrayFrigate
    500.0,        // bDFGFrigate
    1200.0,       // bHeavyCruiser
    1200.0,       // bCarrier
    500.0,        // bRammingFrigate
    400.0,        // bWorker
    0.0,          // bRepairDroid
    800.0,        // bProcessor
    0.0,          // bSentinel
    1800.0,       // bMothership
    1800.0,       // bMothershipDockingBay
    1800.0,       // bMothershipEnginesLower
    1800.0,       // bSectMoship
    1800.0,       // bMothershipSupport
    100.0,        // bCruiseMissile
    0.0,          // bMissileCorvette
    500.0,        // kAdvanceSupportFrigate
    0.0,          // kAttackBomber
    1200.0,       // kCarrier
    500.0,        // kCloakGenerator
    500.0,        // kGravWellGenerator
    0.0,          // kHeavyCorvette
    1200.0,       // kHeavyCruiser
    0.0,          // kHeavyDefender
    0.0,          // kHeavyInterceptor
    500.0,        // kIonCannonFrigate
    0.0,          // kMultiGunCorvette
    100.0,        // kProximitySensor
    0.0,          // kSalCapCorvette
    1000.0,       // kStandardDestroyer
    500.0,        // tAdvanceSupportFrigate
    0.0,          // tAttackBomber
    1200.0,       // tCarrier
    500.0,        // tCloakGenerator
    500.0,        // tDFGFrigate
    500.0,        // tGravWellGenerator
    1200.0,       // tHeavyCruiser
    0.0,          // tHeavyDefender
    0.0,          // tHeavyInterceptor
    500.0,        // tIonCannonFrigate
    0.0,          // tMinelayerCorvette
    1000.0,       // tMissileDestroyer
    0.0,          // tMultiGunCorvette
    100.0,        // tProximitySensor
    500.0,        // tResourceCollector
    0.0,          // tSalCapCorvette
    1000.0,       // tStandardDestroyer
    500.0,        // tStandardFrigate
    1800.0,       // p1Outpost
    0.0,          // p1Fighter
    600.0,        // p1IonArrayFrigate
    0.0,          // p1MissileCorvette
    1200.0,       // p1Mothership
    0.0,          // p1StandardCorvette
    1800.0,       // trFloatingCity
    800.0,        // trCargoBarge
    2000.0,       // trMiningBase
    1500.0,       // trResearchStation
    800.0,        // trJunkYardDawg
    1500.0,       // trJunkYardHQ
    1800.0,       // trGhostship
    800.0,        // trJunk_LGun
    800.0,        // trJunk_SGun
    0.0,          // trResearchStationBridge
    0.0,          // trResearchStationTower
    0.0,          // dRKO
    0.0,          // dBigGun
    0.0,          // dBigGunUnit
    0.0,          // dSlipgateSmall
    0.0,          // dSlipgateMedium
    0.0,          // dSlipgateLarge
    0.0,          // dCrate
    0.0,          // dJunk1PartA
    0.0,          // dJunk1Shell
    0.0,          // dJunk1Strut
    0.0,          // dJunk2Shipwreck
    0.0,          // dJunk3Boiler
    0.0,          // dJunk3BoilerCasing
    0.0,          // dMessagePod
    0.0,          // dBuoy
    0.0,          // dFirstPlanet
    0.0,          // dDeadPlanet
    0.0,          // dCrystal0
    0.0,          // dCrystal1
    0.0,          // dBeastDockingBay
    0.0,          // dNISBuoy
    0.0,          // dBeastMothership
    0.0,          // dNomadMoon
    0.0,          // dNomadFieldEmitter
    0.0,          // dAcolyte
    0.0,          // dPrisonShip
    0.0,          // dPrisonShipInfected
    0.0,          // dSlipgateNode
    0.0,          // dKushanCarrier
    0.0,          // dIonArrayFrigate
    0.0,          // dIonCannonFrigate
    800.0,        // dEcclesiam
    0.0,          // dEcclesiamInfected
    0.0,          // nisLightSource1
    0.0,          // nisLightSource2
    0.0           // nisLensflareSource1
};

// rightOfWay for ships.
// -1 indicates lowest priority of the low (e.g. pebbles in space)
// 0 indicates strikecraft, probes, etc.
// 1 indicates frigates
// 2 indicates cruisers/destroyers
// 3 indicates carriers
// 4 indicates mothership

sbyte rightOfWays[TOTAL_NUM_SHIPS] =
{
    0,             // sRecon
    0,             // sAcolyte
    0,             // sMimic
    0,             // sLeech
    0,             // sACV
    0,             // sMCV
    1,             // sRammingFrigate
    1,             // sMultiBeamFrigate
    1,             // sHiveFrigate
    0,             // sSwarmer
    2,             // sDreadnought
    2,             // sDreadnoughtCannon
    3,             // sCarrier
    3,             // sCarrierSupport
    1,             // sWorker
    0,             // sRepairDroid
    2,             // sProcessor
    0,             // sSentinel
    4,             // sMothership
    4,             // sMothershipSpecial
    4,             // sMothershipEngineering
    4,             // sMothershipArmour
    4,             // sMothershipMicro
    4,             // sMothershipWeapons
    4,             // sMothershipDockingBay
    2,             // sDestroyer
    4,             // sMothershipSupport
    4,             // sMothershipCommand
    4,             // sMothershipEnginesLower
    4,             // sMothershipEnginesUpper
    4,             // sMothershipLowerSpine
    4,             // sMothershipBigGun
    4,             // sMothershipOreCanArmour
    0,             // bRecon
    0,             // bAcolyte
    0,             // bInterceptor
    0,             // bCloakedFighter
    0,             // bAttackBomber
    0,             // bACV
    0,             // bMultiGunCorvette
    0,             // bHeavyCorvette
    1,             // bMultiBeamFrigate
    1,             // bHiveFrigate
    0,             // bSwarmer
    1,             // bIonArrayFrigate
    1,             // bDFGFrigate
    2,             // bHeavyCruiser
    3,             // bCarrier
    1,             // bRammingFrigate
    1,             // bWorker
    0,             // bRepairDroid
    2,             // bProcessor
    0,             // bSentinel
    4,             // bMothership
    4,             // bMothershipDockingBay
    4,             // bMothershipEnginesLower
    4,             // bSectMoship
    4,             // bMothershipSupport
    1,             // bCruiseMissile
    0,             // bMissileCorvette
    1,             // kAdvanceSupportFrigate
    0,             // kAttackBomber
    4,             // kCarrier
    1,             // kCloakGenerator
    1,             // kGravWellGenerator
    0,             // kHeavyCorvette
    2,             // kHeavyCruiser
    0,             // kHeavyDefender
    0,             // kHeavyInterceptor
    1,             // kIonCannonFrigate
    0,             // kMultiGunCorvette
    1,             // kProximitySensor
    0,             // kSalCapCorvette
    2,             // kStandardDestroyer
    1,             // tAdvanceSupportFrigate
    0,             // tAttackBomber
    3,             // tCarrier
    1,             // tCloakGenerator
    1,             // tDFGFrigate
    1,             // tGravWellGenerator
    4,             // tHeavyCruiser
    0,             // tHeavyDefender
    0,             // tHeavyInterceptor
    1,             // tIonCannonFrigate
    0,             // tMinelayerCorvette
    2,             // tMissileDestroyer
    0,             // tMultiGunCorvette
    1,             // tProximitySensor
    1,             // tResourceCollector
    0,             // tSalCapCorvette
    2,             // tStandardDestroyer
    1,             // tStandardFrigate
    4,             // p1Outpost
    0,             // p1Fighter
    1,             // p1IonArrayFrigate
    0,             // p1MissileCorvette
    3,             // p1Mothership
    0,             // p1StandardCorvette
    4,             // trFloatingCity
    1,             // trCargoBarge
    4,             // trMiningBase
    4,             // trResearchStation
    1,             // trJunkYardDawg
    4,             // trJunkYardHQ
    4,             // trGhostship
    1,             // trJunk_LGun
    1,             // trJunk_SGun
    0,             // trResearchStationBridge
    0,             // trResearchStationTower
    0,             // dRKO
    0,             // dBigGun
    0,             // dBigGunUnit
    0,             // dSlipgateSmall
    0,             // dSlipgateMedium
    0,             // dSlipgateLarge
    0,             // dCrate
    0,             // dJunk1PartA
    0,             // dJunk1Shell
    0,             // dJunk1Strut
    0,             // dJunk2Shipwreck
    0,             // dJunk3Boiler
    0,             // dJunk3BoilerCasing
    0,             // dMessagePod
    0,             // dBuoy
    0,             // dFirstPlanet
    0,             // dDeadPlanet
    0,             // dCrystal0
    0,             // dCrystal1
    0,             // dBeastDockingBay
    0,             // dNISBuoy
    0,             // dBeastMothership
    0,             // dNomadMoon
    0,             // dNomadFieldEmitter
    0,             // dAcolyte
    0,             // dPrisonShip
    0,             // dPrisonShipInfected
    0,             // dSlipgateNode
    0,             // dKushanCarrier
    0,             // dIonArrayFrigate
    0,             // dIonCannonFrigate
    3,             // dEcclesiam
    0,             // dEcclesiamInfected
    0,             // nisLightSource1
    0,             // nisLightSource2
    0              // nisLensflareSource1
};

scriptEntry ShipStaticCapLimits[] =
{
    { "ShipTotalCaps",              scriptSetSdwordCB, &cdMaxShipsAllowed },
    { "CapsRecon",                  scriptSetSdwordCB, &cdLimitCaps[sRecon] },
    { "CapsAcolyte",                scriptSetSdwordCB, &cdLimitCaps[sAcolyte] },
    { "CapsMimic",                  scriptSetSdwordCB, &cdLimitCaps[sMimic] },
    { "CapsLeech",                  scriptSetSdwordCB, &cdLimitCaps[sLeech] },
    { "CapsACV",                    scriptSetSdwordCB, &cdLimitCaps[sACV] },
    { "CapsMCV",                    scriptSetSdwordCB, &cdLimitCaps[sMCV] },
    { "CapsRammingFrigate",         scriptSetSdwordCB, &cdLimitCaps[sRammingFrigate] },
    { "CapsMultiBeamFrigate",       scriptSetSdwordCB, &cdLimitCaps[sMultiBeamFrigate] },
    { "CapsHiveFrigate",            scriptSetSdwordCB, &cdLimitCaps[sHiveFrigate] },
    { "CapsSwarmer",                scriptSetSdwordCB, &cdLimitCaps[sSwarmer] },
    { "CapsDreadnought",            scriptSetSdwordCB, &cdLimitCaps[sDreadnought] },
    { "CapsDreadnoughtCannon",      scriptSetSdwordCB, &cdLimitCaps[sDreadnoughtCannon] },
    { "CapsCarrier",                scriptSetSdwordCB, &cdLimitCaps[sCarrier] },
    { "CapsCarrierSupport",         scriptSetSdwordCB, &cdLimitCaps[sCarrierSupport] },
    { "CapsWorker",                 scriptSetSdwordCB, &cdLimitCaps[sWorker] },
    { "CapsRepairDroid",            scriptSetSdwordCB, &cdLimitCaps[sRepairDroid] },
    { "CapsProcessor",              scriptSetSdwordCB, &cdLimitCaps[sProcessor] },
    { "CapsSentinel",               scriptSetSdwordCB, &cdLimitCaps[sSentinel] },
    { "CapsMothership",             scriptSetSdwordCB, &cdLimitCaps[sMothership] },
    { "CapsMothershipSpecial",      scriptSetSdwordCB, &cdLimitCaps[sMothershipSpecial] },
    { "CapsMothershipEngineering",  scriptSetSdwordCB, &cdLimitCaps[sMothershipEngineering] },
    { "CapsMothershipArmour",       scriptSetSdwordCB, &cdLimitCaps[sMothershipArmour] },
    { "CapsMothershipMicro",        scriptSetSdwordCB, &cdLimitCaps[sMothershipMicro] },
    { "CapsMothershipWeapons",      scriptSetSdwordCB, &cdLimitCaps[sMothershipWeapons] },
    { "CapsMothershipDockingBay",   scriptSetSdwordCB, &cdLimitCaps[sMothershipDockingBay] },
    { "CapsDestroyer",              scriptSetSdwordCB, &cdLimitCaps[sDestroyer] },
    { "CapsMothershipSupport",      scriptSetSdwordCB, &cdLimitCaps[sMothershipSupport] },
    { "CapsMothershipCommand",      scriptSetSdwordCB, &cdLimitCaps[sMothershipCommand] },
    { "CapsMothershipEnginesLower", scriptSetSdwordCB, &cdLimitCaps[sMothershipEnginesLower] },
    { "CapsMothershipEnginesUpper", scriptSetSdwordCB, &cdLimitCaps[sMothershipEnginesUpper] },
    { "CapsMothershipLowerSpine",   scriptSetSdwordCB, &cdLimitCaps[sMothershipLowerSpine] },
    { "CapsMothershipBigGun",       scriptSetSdwordCB, &cdLimitCaps[sMothershipBigGun] },
    { "CapsMothershipOreCanArmour", scriptSetSdwordCB, &cdLimitCaps[sMothershipOreCanArmour] },
    { "CapbRecon",                  scriptSetSdwordCB, &cdLimitCaps[bRecon] },
    { "CapbAcolyte",                scriptSetSdwordCB, &cdLimitCaps[bAcolyte] },
    { "CapbInterceptor",            scriptSetSdwordCB, &cdLimitCaps[bInterceptor] },
    { "CapbCloakedFighter",         scriptSetSdwordCB, &cdLimitCaps[bCloakedFighter] },
    { "CapbAttackBomber",           scriptSetSdwordCB, &cdLimitCaps[bAttackBomber] },
    { "CapbACV",                    scriptSetSdwordCB, &cdLimitCaps[bACV] },
    { "CapbMultiGunCorvette",       scriptSetSdwordCB, &cdLimitCaps[bMultiGunCorvette] },
    { "CapbHeavyCorvette",          scriptSetSdwordCB, &cdLimitCaps[bHeavyCorvette] },
    { "CapbMultiBeamFrigate",       scriptSetSdwordCB, &cdLimitCaps[bMultiBeamFrigate] },
    { "CapbHiveFrigate",            scriptSetSdwordCB, &cdLimitCaps[bHiveFrigate] },
    { "CapbSwarmer",                scriptSetSdwordCB, &cdLimitCaps[bSwarmer] },
    { "CapbIonArrayFrigate",        scriptSetSdwordCB, &cdLimitCaps[bIonArrayFrigate] },
    { "CapbDFGFrigate",             scriptSetSdwordCB, &cdLimitCaps[bDFGFrigate] },
    { "CapbHeavyCruiser",           scriptSetSdwordCB, &cdLimitCaps[bHeavyCruiser] },
    { "CapbCarrier",                scriptSetSdwordCB, &cdLimitCaps[bCarrier] },
    { "CapbRammingFrigate",         scriptSetSdwordCB, &cdLimitCaps[bRammingFrigate] },
    { "CapbWorker",                 scriptSetSdwordCB, &cdLimitCaps[bWorker] },
    { "CapbRepairDroid",            scriptSetSdwordCB, &cdLimitCaps[bRepairDroid] },
    { "CapbProcessor",              scriptSetSdwordCB, &cdLimitCaps[bProcessor] },
    { "CapbSentinel",               scriptSetSdwordCB, &cdLimitCaps[bSentinel] },
    { "CapbMothership",             scriptSetSdwordCB, &cdLimitCaps[bMothership] },
    { "CapbMothershipDockingBay",   scriptSetSdwordCB, &cdLimitCaps[bMothershipDockingBay] },
    { "CapbMothershipEnginesLower", scriptSetSdwordCB, &cdLimitCaps[bMothershipEnginesLower] },
    { "CapbSectMoship",             scriptSetSdwordCB, &cdLimitCaps[bSectMoship] },
    { "CapbMothershipSupport",      scriptSetSdwordCB, &cdLimitCaps[bMothershipSupport] },
    { "CapbCruiseMissile",          scriptSetSdwordCB, &cdLimitCaps[bCruiseMissile] },
    { "CapbMissileCorvette",        scriptSetSdwordCB, &cdLimitCaps[bMissileCorvette] },
    { "CapkAdvanceSupportFrigate",  scriptSetSdwordCB, &cdLimitCaps[kAdvanceSupportFrigate] },
    { "CapkAttackBomber",           scriptSetSdwordCB, &cdLimitCaps[kAttackBomber] },
    { "CapkCarrier",                scriptSetSdwordCB, &cdLimitCaps[kCarrier] },
    { "CapkCloakGenerator",         scriptSetSdwordCB, &cdLimitCaps[kCloakGenerator] },
    { "CapkGravWellGenerator",      scriptSetSdwordCB, &cdLimitCaps[kGravWellGenerator] },
    { "CapkHeavyCorvette",          scriptSetSdwordCB, &cdLimitCaps[kHeavyCorvette] },
    { "CapkHeavyCruiser",           scriptSetSdwordCB, &cdLimitCaps[kHeavyCruiser] },
    { "CapkHeavyDefender",          scriptSetSdwordCB, &cdLimitCaps[kHeavyDefender] },
    { "CapkHeavyInterceptor",       scriptSetSdwordCB, &cdLimitCaps[kHeavyInterceptor] },
    { "CapkIonCannonFrigate",       scriptSetSdwordCB, &cdLimitCaps[kIonCannonFrigate] },
    { "CapkMultiGunCorvette",       scriptSetSdwordCB, &cdLimitCaps[kMultiGunCorvette] },
    { "CapkProximitySensor",        scriptSetSdwordCB, &cdLimitCaps[kProximitySensor] },
    { "CapkSalCapCorvette",         scriptSetSdwordCB, &cdLimitCaps[kSalCapCorvette] },
    { "CapkStandardDestroyer",      scriptSetSdwordCB, &cdLimitCaps[kStandardDestroyer] },
    { "CaptAdvanceSupportFrigate",  scriptSetSdwordCB, &cdLimitCaps[tAdvanceSupportFrigate] },
    { "CaptAttackBomber",           scriptSetSdwordCB, &cdLimitCaps[tAttackBomber] },
    { "CaptCarrier",                scriptSetSdwordCB, &cdLimitCaps[tCarrier] },
    { "CaptCloakGenerator",         scriptSetSdwordCB, &cdLimitCaps[tCloakGenerator] },
    { "CaptDFGFrigate",             scriptSetSdwordCB, &cdLimitCaps[tDFGFrigate] },
    { "CaptGravWellGenerator",      scriptSetSdwordCB, &cdLimitCaps[tGravWellGenerator] },
    { "CaptHeavyCruiser",           scriptSetSdwordCB, &cdLimitCaps[tHeavyCruiser] },
    { "CaptHeavyDefender",          scriptSetSdwordCB, &cdLimitCaps[tHeavyDefender] },
    { "CaptHeavyInterceptor",       scriptSetSdwordCB, &cdLimitCaps[tHeavyInterceptor] },
    { "CaptIonCannonFrigate",       scriptSetSdwordCB, &cdLimitCaps[tIonCannonFrigate] },
    { "CaptMinelayerCorvette",      scriptSetSdwordCB, &cdLimitCaps[tMinelayerCorvette] },
    { "CaptMissileDestroyer",       scriptSetSdwordCB, &cdLimitCaps[tMissileDestroyer] },
    { "CaptMultiGunCorvette",       scriptSetSdwordCB, &cdLimitCaps[tMultiGunCorvette] },
    { "CaptProximitySensor",        scriptSetSdwordCB, &cdLimitCaps[tProximitySensor] },
    { "CaptResourceCollector",      scriptSetSdwordCB, &cdLimitCaps[tResourceCollector] },
    { "CaptSalCapCorvette",         scriptSetSdwordCB, &cdLimitCaps[tSalCapCorvette] },
    { "CaptStandardDestroyer",      scriptSetSdwordCB, &cdLimitCaps[tStandardDestroyer] },
    { "CaptStandardFrigate",        scriptSetSdwordCB, &cdLimitCaps[tStandardFrigate] },
    { "Capp1Outpost",               scriptSetSdwordCB, &cdLimitCaps[p1Outpost] },
    { "Capp1Fighter",               scriptSetSdwordCB, &cdLimitCaps[p1Fighter] },
    { "Capp1IonArrayFrigate",       scriptSetSdwordCB, &cdLimitCaps[p1IonArrayFrigate] },
    { "Capp1MissileCorvette",       scriptSetSdwordCB, &cdLimitCaps[p1MissileCorvette] },
    { "Capp1Mothership",            scriptSetSdwordCB, &cdLimitCaps[p1Mothership] },
    { "Capp1StandardCorvette",      scriptSetSdwordCB, &cdLimitCaps[p1StandardCorvette] },
    { "CaptrFloatingCity",          scriptSetSdwordCB, &cdLimitCaps[trFloatingCity] },
    { "CapdAcolyte",                scriptSetSdwordCB, &cdLimitCaps[dAcolyte] },
    { "CapCLASS_Mothership",        scriptSetSdwordCB, &cdClassCaps[0]  },
    { "CapCLASS_HeavyCruiser",      scriptSetSdwordCB, &cdClassCaps[1]  },
    { "CapCLASS_Carrier",           scriptSetSdwordCB, &cdClassCaps[2]  },
    { "CapCLASS_Destroyer",         scriptSetSdwordCB, &cdClassCaps[3]  },
    { "CapCLASS_Frigate",           scriptSetSdwordCB, &cdClassCaps[4]  },
    { "CapCLASS_Corvette",          scriptSetSdwordCB, &cdClassCaps[5]  },
    { "CapCLASS_Fighter",           scriptSetSdwordCB, &cdClassCaps[6]  },
    { "CapCLASS_Resource",          scriptSetSdwordCB, &cdClassCaps[7]  },
    { "CapCLASS_NonCombat",         scriptSetSdwordCB, &cdClassCaps[8]  },
    { "CapCLASS_Composite",         scriptSetSdwordCB, &cdClassCaps[CLASS_MonsterShip]  },
    { "CapCLASS_Component",         scriptSetSdwordCB, &cdClassCaps[CLASS_Component]  },
    { "CapCLASS_Micro",             scriptSetSdwordCB, &cdClassCaps[CLASS_Micro]  },

    { "MaxJobsPerClass[Mothership]",    scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Mothership]  },
    { "MaxJobsPerClass[HeavyCruiser]",  scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_HeavyCruiser]},
    { "MaxJobsPerClass[Carrier]",       scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Carrier]},
    { "MaxJobsPerClass[Destroyer]",     scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Destroyer]},
    { "MaxJobsPerClass[Frigate]",       scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Frigate]},
    { "MaxJobsPerClass[Corvette]",      scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Corvette]},
    { "MaxJobsPerClass[Fighter]",       scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Fighter]},
    { "MaxJobsPerClass[Resource]",      scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_Resource]},
    { "MaxJobsPerClass[NonCombat]",     scriptSetSdwordCB, &cmMaxJobsPerClass[CLASS_NonCombat]},
    endEntry
};

scriptStructEntry ShipStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(shipStaticInfos[0].staticheader.LOD), (udword) &(shipStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(shipStaticInfos[0].staticheader.pMexData), (udword) &(shipStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(shipStaticInfos[0].staticheader.mass),      (udword) &(shipStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(shipStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(shipStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(shipStaticInfos[0]) },

    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(shipStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(shipStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(shipStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(shipStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(shipStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(shipStaticInfos[0]) },



    { "immobile",        scriptSetBool8,    (udword) &(shipStaticInfos[0].staticheader.immobile), (udword) &(shipStaticInfos[0]) },
    { "rightOfWay",      scriptSetSbyteCB,  (udword) &(shipStaticInfos[0].staticheader.rightOfWay), (udword) &(shipStaticInfos[0]) },
    { "collspheresize", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.collspheresize),  (udword) &(shipStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(shipStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(shipStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].staticheader.maxvelocity),  (udword) &(shipStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(shipStaticInfos[0].staticheader.maxrot),  (udword) &(shipStaticInfos[0]) },
    { "bulletRange[0]", scriptSetReal32CB, (udword) &(shipStaticInfos[0].bulletRange[0]),  (udword) &(shipStaticInfos[0]) },
    { "bulletRange[1]", scriptSetReal32CB, (udword) &(shipStaticInfos[0].bulletRange[1]),  (udword) &(shipStaticInfos[0]) },
    { "bulletRange[2]", scriptSetReal32CB, (udword) &(shipStaticInfos[0].bulletRange[2]),  (udword) &(shipStaticInfos[0]) },
    { "minBulletRange[0]", scriptSetReal32CB, (udword) &(shipStaticInfos[0].minBulletRange[0]),  (udword) &(shipStaticInfos[0]) },
    { "minBulletRange[1]", scriptSetReal32CB, (udword) &(shipStaticInfos[0].minBulletRange[1]),  (udword) &(shipStaticInfos[0]) },
    { "minBulletRange[2]", scriptSetReal32CB, (udword) &(shipStaticInfos[0].minBulletRange[2]),  (udword) &(shipStaticInfos[0]) },
    { "blastRadiusShockWave", scriptSetReal32CB, (udword) &(shipStaticInfos[0].blastRadiusShockWave),  (udword) &(shipStaticInfos[0]) },
    { "blastRadiusDamage", scriptSetReal32CB, (udword) &(shipStaticInfos[0].blastRadiusDamage),  (udword) &(shipStaticInfos[0]) },
    { "passiveRetaliateZone", scriptSetReal32CB, (udword) &(shipStaticInfos[0].passiveRetaliateZone),  (udword) &(shipStaticInfos[0]) },
    { "shipclass",      setClassCB,         (udword) &(shipStaticInfos[0].shipclass), (udword) &(shipStaticInfos[0]) },
    { "externalBuild",  scriptSetBool,      (udword) &(shipStaticInfos[0].externalBuild), (udword) &(shipStaticInfos[0]) },
    { "forceFieldProtectionDistanceScale", scriptSetReal32CB, (udword) &(shipStaticInfos[0].forceFieldProtectionDistanceScale), (udword) &(shipStaticInfos[0]) },
    { "upgradeTimeModifier", scriptSetReal32CB, (udword) &(shipStaticInfos[0].upgradeTimeModifier), (udword) &(shipStaticInfos[0]) },
    { "rotateToRetaliate",scriptSetBool8,     (udword) &(shipStaticInfos[0].rotateToRetaliate), (udword) &(shipStaticInfos[0]) },
    { "canTargetMultipleTargets",scriptSetBool8,(udword) &(shipStaticInfos[0].canTargetMultipleTargets), (udword) &(shipStaticInfos[0]) },
    { "canBuildShips",  scriptSetBool8,     (udword) &(shipStaticInfos[0].canBuildShips), (udword) &(shipStaticInfos[0]) },
    { "canReceiveShips",scriptSetBool8,     (udword) &(shipStaticInfos[0].canReceiveShips), (udword) &(shipStaticInfos[0]) },
    { "canReceiveResources",scriptSetBool8, (udword) &(shipStaticInfos[0].canReceiveResources), (udword) &(shipStaticInfos[0]) },
    { "canReceiveShipsPermanently",scriptSetBool8, (udword) &(shipStaticInfos[0].canReceiveShipsPermanently), (udword) &(shipStaticInfos[0]) },
    { "canReceiveShipsForRetire",scriptSetBool8, (udword) &(shipStaticInfos[0].canReceiveShipsForRetire), (udword) &(shipStaticInfos[0]) },
    { "canBuildBigShips",scriptSetBool8,     (udword) &(shipStaticInfos[0].canBuildBigShips), (udword) &(shipStaticInfos[0]) },
    { "canReceiveFighters",scriptSetBool8,     (udword) &(shipStaticInfos[0].canReceiveTheseShips[THISSHIPIS_FIGHTER]), (udword) &(shipStaticInfos[0]) },
    { "canReceiveCorvettes",scriptSetBool8,     (udword) &(shipStaticInfos[0].canReceiveTheseShips[THISSHIPIS_CORVETTE]), (udword) &(shipStaticInfos[0]) },
    { "canReceiveResourceCollectors",scriptSetBool8,     (udword) &(shipStaticInfos[0].canReceiveTheseShips[THISSHIPIS_RESOURCER]), (udword) &(shipStaticInfos[0]) },
    { "canReceiveOtherNonCapitalShips",scriptSetBool8,     (udword) &(shipStaticInfos[0].canReceiveTheseShips[THISSHIPIS_OTHERNONCAPITALSHIP]), (udword) &(shipStaticInfos[0]) },
    { "canHandleNumShipsDocking",scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].canHandleNumShipsDocking), (udword) &(shipStaticInfos[0]) },
    { "canHandleNumShipsDepositingRU",scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].canHandleNumShipsDepositingRU), (udword) &(shipStaticInfos[0]) },
    { "svManeuverability",scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].svManeuverability), (udword) &(shipStaticInfos[0]) },
    { "svCoverage",scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].svCoverage), (udword) &(shipStaticInfos[0]) },
    { "svFirePower",scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].svFirePower), (udword) &(shipStaticInfos[0]) },
    { "RKOEatMeTime",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].RKOEatMeTime), (udword) &(shipStaticInfos[0]) },

    { "maxDockableFighters" ,scriptSetSwordCB, (udword) &(shipStaticInfos[0].maxDockableFighters),(udword) &(shipStaticInfos[0])},
    { "maxDockableCorvettes",scriptSetSwordCB, (udword) &(shipStaticInfos[0].maxDockableCorvettes),(udword) &(shipStaticInfos[0])},
    { "specialActivateIsContinuous",scriptSetBool8,     (udword) &(shipStaticInfos[0].specialActivateIsContinuous), (udword) &(shipStaticInfos[0]) },
    { "canSpecialBandBoxFriendlies",scriptSetBool8,     (udword) &(shipStaticInfos[0].canSpecialBandBoxFriendlies), (udword) &(shipStaticInfos[0]) },
    { "canSingleClickSpecialActivate",scriptSetBool8,     (udword) &(shipStaticInfos[0].canSingleClickSpecialActivate), (udword) &(shipStaticInfos[0]) },
    { "buildCost",      scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].buildCost), (udword) &(shipStaticInfos[0]) },
    { "buildTime",      scriptSetFramesCB,  (udword) &(shipStaticInfos[0].buildTime), (udword) &(shipStaticInfos[0]) },
    { "fowRadius",      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].fowRadius), (udword) &(shipStaticInfos[0]) },
    { "resourcesAtOneTime",   scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].resourcesAtOneTime), (udword) &(shipStaticInfos[0]) },
    { "maxresources",   scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].maxresources), (udword) &(shipStaticInfos[0]) },
    { "harvestAmount",  scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].harvestAmount), (udword) &(shipStaticInfos[0]) },
    { "harvestRate",    scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].harvestRate), (udword) &(shipStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].maxhealth), (udword) &(shipStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].minCollDamage), (udword) &(shipStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].maxCollDamage), (udword) &(shipStaticInfos[0]) },
    { "groupSize",      scriptSetSdwordCB,  (udword) &(shipStaticInfos[0].groupSize), (udword) &(shipStaticInfos[0]) },
    { "maxfuel",        scriptSetReal32CB,  (udword) &(shipStaticInfos[0].maxfuel),   (udword) &(shipStaticInfos[0]) },
    { "lowfuelpoint",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].lowfuelpoint),   (udword) &(shipStaticInfos[0]) },
    { "fuelburnrate",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].fuelburnrate), (udword) &(shipStaticInfos[0]) },
    { "attackfuelburnrate", scriptSetReal32CB, (udword) &(shipStaticInfos[0].attackfuelburnrate), (udword) &(shipStaticInfos[0]) },
    { "repairTime",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].repairTime), (udword) &(shipStaticInfos[0]) },
    { "repairDamage",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].repairDamage), (udword) &(shipStaticInfos[0]) },
    { "repairCombatTime",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].repairCombatTime), (udword) &(shipStaticInfos[0]) },
    { "repairCombatDamage",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].repairCombatDamage), (udword) &(shipStaticInfos[0]) },
    { "pumpFuelRate",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].pumpFuelRate), (udword) &(shipStaticInfos[0]) },
    { "repairOtherShipRate",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].repairOtherShipRate), (udword) &(shipStaticInfos[0]) },
    { "repairResourceCollectorRate",   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].repairResourceCollectorRate), (udword) &(shipStaticInfos[0]) },
    { "dockShipRange",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].dockShipRange), (udword) &(shipStaticInfos[0]) },
    { "clearanceDistance",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].clearanceDistance), (udword) &(shipStaticInfos[0]) },
    { "clearanceDirection",   scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].clearanceDirection), (udword) &(shipStaticInfos[0]) },
    { "formationPaddingModifier", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].formationPaddingModifier), (udword) &(shipStaticInfos[0]) },
    { "cannotForceAttackIfOwnShip",scriptSetBool,     (udword) &(shipStaticInfos[0].cannotForceAttackIfOwnShip), (udword) &(shipStaticInfos[0]) },
    { "passiveAttackPenaltyExempt",scriptSetBool,     (udword) &(shipStaticInfos[0].passiveAttackPenaltyExempt), (udword) &(shipStaticInfos[0]) },
    { "isCapitalShip",scriptSetBool,     (udword) &(shipStaticInfos[0].shipIsCapital), (udword) &(shipStaticInfos[0]) },
    { "cantMoveAndAttack",scriptSetBool,     (udword) &(shipStaticInfos[0].cantMoveAndAttack), (udword) &(shipStaticInfos[0]) },

    { "repairBeamCapable",scriptSetBool,     (udword) &(shipStaticInfos[0].repairBeamCapable), (udword) &(shipStaticInfos[0]) },
    { "healthPerSecond", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].healthPerSecond), (udword) &(shipStaticInfos[0]) },
    { "CapitalDistanceRepairStart2", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].CapitalDistanceRepairStart2), (udword) &(shipStaticInfos[0]) },
    { "CapitalDistanceRepairStart", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].CapitalDistanceRepairStart), (udword) &(shipStaticInfos[0]) },
    { "AngleDotProdThreshold", scriptSetReal32CB,  (udword) &(shipStaticInfos[0].AngleDotProdThreshold), (udword) &(shipStaticInfos[0]) },

    { "collSideModifiers[TRANS_UP]",       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].collSideModifiers[TRANS_UP]), (udword) &(shipStaticInfos[0]) },
    { "collSideModifiers[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].collSideModifiers[TRANS_DOWN]), (udword) &(shipStaticInfos[0]) },
    { "collSideModifiers[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].collSideModifiers[TRANS_RIGHT]), (udword) &(shipStaticInfos[0]) },
    { "collSideModifiers[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].collSideModifiers[TRANS_LEFT]), (udword) &(shipStaticInfos[0]) },
    { "collSideModifiers[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(shipStaticInfos[0].collSideModifiers[TRANS_FORWARD]), (udword) &(shipStaticInfos[0]) },
    { "collSideModifiers[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].collSideModifiers[TRANS_BACKWARD]), (udword) &(shipStaticInfos[0]) },
    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(shipStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(shipStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(shipStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(shipStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(shipStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(shipStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(shipStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(shipStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(shipStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(shipStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(shipStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(shipStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(shipStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(shipStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(shipStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(shipStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(shipStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(shipStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(shipStaticInfos[0]) },
    { "N-LIPS",                         scriptSetReal32CB,  (udword) &(shipStaticInfos[0].scaleCap), (udword) &(shipStaticInfos[0]) },
//    { "scaleCap",                       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].scaleCap), (udword) &(shipStaticInfos[0]) },
#if SO_CLOOGE_SCALE
    { "scaleFactor",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].scaleFactor), (udword) &(shipStaticInfos[0]) },
#endif
    { "repairDroids",                   scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].nRepairDroids), (udword) &(shipStaticInfos[0]) },
    { "trailStyle",                     scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].trailStyle[0]), (udword) &(shipStaticInfos[0]) },
    { "trailWidth",                     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailWidth[0]), (udword) &(shipStaticInfos[0]) },
    { "trailHeight",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailHeight[0]), (udword) &(shipStaticInfos[0]) },
    { "trailAngle",                     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailAngle[0]), (udword) &(shipStaticInfos[0]) },
    { "trailRibbonAdjust",              scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailRibbonAdjust[0]), (udword) &(shipStaticInfos[0]) },
    { "trailLength",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailLength[0]), (udword) &(shipStaticInfos[0]) },
    { "trailScaleCap",                  scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailScaleCap[0]), (udword) &(shipStaticInfos[0]) },
    { "trailStyle2",                    scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].trailStyle[1]), (udword) &(shipStaticInfos[0]) },
    { "trailWidth2",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailWidth[1]), (udword) &(shipStaticInfos[0]) },
    { "trailHeight2",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailHeight[1]), (udword) &(shipStaticInfos[0]) },
    { "trailAngle2",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailAngle[1]), (udword) &(shipStaticInfos[0]) },
    { "trailRibbonAdjust2",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailRibbonAdjust[1]), (udword) &(shipStaticInfos[0]) },
    { "trailLength2",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailLength[1]), (udword) &(shipStaticInfos[0]) },
    { "trailScaleCap2",                 scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailScaleCap[1]), (udword) &(shipStaticInfos[0]) },
    { "trailStyle3",                    scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].trailStyle[2]), (udword) &(shipStaticInfos[0]) },
    { "trailWidth3",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailWidth[2]), (udword) &(shipStaticInfos[0]) },
    { "trailHeight3",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailHeight[2]), (udword) &(shipStaticInfos[0]) },
    { "trailAngle3",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailAngle[2]), (udword) &(shipStaticInfos[0]) },
    { "trailRibbonAdjust3",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailRibbonAdjust[2]), (udword) &(shipStaticInfos[0]) },
    { "trailLength3",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailLength[2]), (udword) &(shipStaticInfos[0]) },
    { "trailScaleCap3",                 scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailScaleCap[2]), (udword) &(shipStaticInfos[0]) },
    { "trailStyle4",                    scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].trailStyle[3]), (udword) &(shipStaticInfos[0]) },
    { "trailWidth4",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailWidth[3]), (udword) &(shipStaticInfos[0]) },
    { "trailHeight4",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailHeight[3]), (udword) &(shipStaticInfos[0]) },
    { "trailAngle4",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailAngle[3]), (udword) &(shipStaticInfos[0]) },
    { "trailRibbonAdjust4",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailRibbonAdjust[3]), (udword) &(shipStaticInfos[0]) },
    { "trailLength4",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailLength[3]), (udword) &(shipStaticInfos[0]) },
    { "trailScaleCap4",                 scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailScaleCap[3]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteRadius",              scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteRadius[0]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteRadius2",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteRadius[1]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteRadius3",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteRadius[2]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteRadius4",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteRadius[3]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteOffset",              scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteOffset[0]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteOffset2",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteOffset[1]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteOffset3",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteOffset[2]), (udword) &(shipStaticInfos[0]) },
    { "trailSpriteOffset4",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].trailSpriteOffset[3]), (udword) &(shipStaticInfos[0]) },
    { "minimumZoomDistance",            scriptSetReal32CB,  (udword) &(shipStaticInfos[0].minimumZoomDistance), (udword) &(shipStaticInfos[0]) },
    { "renderlistFade",                 scriptSetReal32CB,  (udword) &(shipStaticInfos[0].renderlistFade), (udword) &(shipStaticInfos[0]) },
    { "renderlistLimit",                scriptSetReal32SqrCB, (udword) &(shipStaticInfos[0].renderlistLimitSqr), (udword) &(shipStaticInfos[0]) },
    { "dockLightNear",                  scriptSetReal32CB,  (udword) &(shipStaticInfos[0].dockLightNear), (udword) &(shipStaticInfos[0]) },
    { "dockLightFar",                   scriptSetReal32CB,  (udword) &(shipStaticInfos[0].dockLightFar), (udword) &(shipStaticInfos[0]) },
    { "dockLightColor",                 scriptSetRGBCB,     (udword) &(shipStaticInfos[0].dockLightColor), (udword) &(shipStaticInfos[0]) },
    { "hyperspaceColor",                scriptSetRGBCB,     (udword) &(shipStaticInfos[0].hyperspaceColor), (udword) &(shipStaticInfos[0]) },
    { "hyperspaceBoxScale",             scriptSetReal32CB,  (udword) &(shipStaticInfos[0].hyperspaceBoxScale), (udword) &(shipStaticInfos[0]) },
    { "leechRUsPerSecond",              scriptSetReal32CB,  (udword) &(shipStaticInfos[0].leechRUsPerSecond), (udword) &(shipStaticInfos[0]) },
    { "leechHPsPerSecond",              scriptSetReal32CB,  (udword) &(shipStaticInfos[0].leechHPsPerSecond), (udword) &(shipStaticInfos[0]) },
    { "beastAttackCascadeCost",         scriptSetReal32CB,  (udword) &(shipStaticInfos[0].beastAttackCascadeCost), (udword) &(shipStaticInfos[0]) },
    { "beastAttacksRequired",           scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].beastAttacksRequired), (udword) &(shipStaticInfos[0]) },
    { "beastAttackConvertTo",           scriptSetShipTypeCB,(udword) &(shipStaticInfos[0].beastAttackConvertTo), (udword) &(shipStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { "madGunOpenDamagedHealthThreshold",  scriptSetReal32CB,(udword) &(shipStaticInfos[0].madGunOpenDamagedHealthThreshold), (udword) &(shipStaticInfos[0]) },
    { "madWingOpenDamagedHealthThreshold", scriptSetReal32CB,(udword) &(shipStaticInfos[0].madWingOpenDamagedHealthThreshold), (udword) &(shipStaticInfos[0]) },
    { "Overlay_Selection_Scalar",       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].tacticalSelectionScale), (udword) &(shipStaticInfos[0]) },
    { "bUseRayTriangleIntersectionForHitLocation", scriptSetBool, (udword) ((ubyte *) &shipStaticInfos[0] + SHIP_STATIC_INFO_CATA_TAIL_OFFSET(bUseRayTriangleIntersectionForHitLocation)), (udword) &(shipStaticInfos[0]) },
    { "linkDistance",                   scriptSetReal32CB,  (udword) ((ubyte *) &shipStaticInfos[0] + SHIP_STATIC_INFO_CATA_TAIL_OFFSET(linkDistance)), (udword) &(shipStaticInfos[0]) },
    { "supportUnits",                   setSupportUnitsInfo, 0, 0 },
    { "providesSU",                     setProvidesSUInfo,   0, 0 },
    { "RamDamage",                      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].RamDamage), (udword) &(shipStaticInfos[0]) },
    { "cuttingBeamDamagePerSecond",     scriptSetReal32CB,  (udword) &(shipStaticInfos[0].cuttingBeamDamagePerSecond), (udword) &(shipStaticInfos[0]) },
    { "cuttingBeamUpgradedDamagePerSecond", scriptSetReal32CB, (udword) &(shipStaticInfos[0].cuttingBeamUpgradedDamagePerSecond), (udword) &(shipStaticInfos[0]) },
    { "RamPushTime",                    scriptSetReal32CB,  (udword) &(shipStaticInfos[0].RamPushTime), (udword) &(shipStaticInfos[0]) },
    { "RamWaitAfterPushTime",           scriptSetReal32CB,  (udword) &(shipStaticInfos[0].RamWaitAfterPushTime), (udword) &(shipStaticInfos[0]) },
    { "TimeShockedByEMP",               scriptSetReal32CB,  (udword) &(shipStaticInfos[0].TimeShockedByEMP), (udword) &(shipStaticInfos[0]) },
    { "EMP_Range",                      scriptSetReal32CB,  (udword) &(shipStaticInfos[0].EMP_Range), (udword) &(shipStaticInfos[0]) },
    { "EMP_Max_Combine_Distance",       scriptSetReal32CB,  (udword) &(shipStaticInfos[0].EMP_Max_Combine_Distance), (udword) &(shipStaticInfos[0]) },
    { "UpgradeByDocking",               scriptSetBool,      (udword) ((ubyte *) &shipStaticInfos[0] + SHIP_STATIC_INFO_CATA_TAIL_OFFSET(UpgradeByDocking)), (udword) &(shipStaticInfos[0]) },
    { "pilotViewOffsetX",               scriptSetReal32CB,  (udword) &(shipStaticInfos[0].pilotViewOffsetX), (udword) &(shipStaticInfos[0]) },
    { "pilotViewOffsetY",               scriptSetReal32CB,  (udword) &(shipStaticInfos[0].pilotViewOffsetY), (udword) &(shipStaticInfos[0]) },
    { "pilotViewOffsetZ",               scriptSetReal32CB,  (udword) &(shipStaticInfos[0].pilotViewOffsetZ), (udword) &(shipStaticInfos[0]) },
    { "RKOGetsMissiles",                scriptSetUdwordCB,  (udword) &(shipStaticInfos[0].RKOGetsMissiles), (udword) &(shipStaticInfos[0]) },

    { NULL,NULL,0,0 }
};

scriptStructEntry HierarchyBindingTable[] =
{
    { "BindGun",                        setGunBindInfo,     0, 0 },
    { "BindAnimations",                 setAnimationBindInfo,0, 0 },
    { "NUMBER_OF_ANIMS",                setNumberOfAnimsInfo, 0, 0 },
    { "ANIM",                           setAnimInfo, 0, 0 },
    { NULL,NULL,0,0 }
};

scriptStructEntry MadMaxMadInfoLoad[] =
{
    { "MadStartInfo",                   setMadStartInfo,0, 0 },
    { "MadGunAnims",                    setGunAnimInfo,0, 0 },
    { "MadDockAnims",                   setDockAnimInfo,0, 0 },
    { "MadMothershipDockingAnims",      setMothershipDoorAnimInfo,0,0 },
    { "MadSpecialCaseAnims",      setSpecialCaseAnimInfo,0,0 },

    { NULL,NULL,0,0 }
};

scriptStructEntry AsteroidStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(asteroidStaticInfos[0].staticheader.LOD), (udword) &(asteroidStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(asteroidStaticInfos[0].staticheader.pMexData), (udword) &(asteroidStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.mass),      (udword) &(asteroidStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(asteroidStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(asteroidStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(asteroidStaticInfos[0]) },


    { "immobile",        scriptSetBool8,    (udword) &(asteroidStaticInfos[0].staticheader.immobile), (udword) &(asteroidStaticInfos[0]) },
    { "rightOfWay",      scriptSetSbyteCB,  (udword) &(asteroidStaticInfos[0].staticheader.rightOfWay), (udword) &(asteroidStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(asteroidStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(asteroidStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.maxvelocity),  (udword) &(asteroidStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].staticheader.maxrot),  (udword) &(asteroidStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].maxhealth), (udword) &(asteroidStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].minCollDamage), (udword) &(asteroidStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(asteroidStaticInfos[0].maxCollDamage), (udword) &(asteroidStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(asteroidStaticInfos[0].resourcevalue), (udword) &(asteroidStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB,  (udword) &(asteroidStaticInfos[0].harvestableByMultipleShips), (udword) &(asteroidStaticInfos[0]) },

    { "asteroidCanShrink",scriptSetBool8, (udword) &(asteroidStaticInfos[0].asteroidCanShrink), (udword) &(asteroidStaticInfos[0]) },
    { "asteroidCanBreak", scriptSetBool8, (udword) &(asteroidStaticInfos[0].asteroidCanBreak),  (udword) &(asteroidStaticInfos[0]) },
    { "asteroidMinShrink",scriptSetReal32CB, (udword) &(asteroidStaticInfos[0].asteroidMinShrink), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid0]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid0]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid1]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid1]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid2]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid2]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid3]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid3]), (udword) &(asteroidStaticInfos[0]) },
    { "breakinto[Asteroid4]",scriptSetUdwordCB, (udword) &(asteroidStaticInfos[0].breakinto[Asteroid4]), (udword) &(asteroidStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry DustCloudStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(dustcloudStaticInfos[0].staticheader.LOD), (udword) &(dustcloudStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(dustcloudStaticInfos[0].staticheader.pMexData), (udword) &(dustcloudStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].staticheader.mass),     (udword) &(dustcloudStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(dustcloudStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(dustcloudStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(dustcloudStaticInfos[0]) },

    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(dustcloudStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(dustcloudStaticInfos[0]) },

    { "immobile",        scriptSetBool8,    (udword) &(dustcloudStaticInfos[0].staticheader.immobile), (udword) &(dustcloudStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(dustcloudStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(dustcloudStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].staticheader.maxvelocity),  (udword) &(dustcloudStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].staticheader.maxrot),  (udword) &(dustcloudStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].maxhealth), (udword) &(dustcloudStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].minCollDamage), (udword) &(dustcloudStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(dustcloudStaticInfos[0].maxCollDamage), (udword) &(dustcloudStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(dustcloudStaticInfos[0].resourcevalue), (udword) &(dustcloudStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB, (udword) &(dustcloudStaticInfos[0].harvestableByMultipleShips), (udword) &(dustcloudStaticInfos[0]) },

    { "dustcloudCanShrink",scriptSetBool8, (udword) &(dustcloudStaticInfos[0].dustcloudCanShrink), (udword) &(dustcloudStaticInfos[0]) },
    { "dustcloudMinShrink",scriptSetReal32CB, (udword) &(dustcloudStaticInfos[0].dustcloudMinShrink), (udword) &(dustcloudStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry NebulaStaticScriptTable[] =
{
    { "LODFile", LODScriptLoad, (udword)&(nebulaStaticInfos[0].staticheader.LOD), (udword)&(nebulaStaticInfos[0]) },
    { "pMexData", loadMexDataCB, (udword)&(nebulaStaticInfos[0].staticheader.pMexData), (udword)&(nebulaStaticInfos[0]) },
    { "mass", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.mass), (udword)&(nebulaStaticInfos[0]) },
    { "momentOfInertiaX", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.momentOfInertiaX), (udword)&(nebulaStaticInfos[0]) },
    { "momentOfInertiaY", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.momentOfInertiaY), (udword)&(nebulaStaticInfos[0]) },
    { "momentOfInertiaZ", scriptSetReal32CB, (udword)&(nebulaStaticInfos[0].staticheader.momentOfInertiaZ), (udword)&(nebulaStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(nebulaStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(nebulaStaticInfos[0]) },
    { "immobile", scriptSetBool8, (udword)&(nebulaStaticInfos[0].staticheader.immobile), (udword)&(nebulaStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(nebulaStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(nebulaStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].staticheader.maxvelocity),  (udword) &(nebulaStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].staticheader.maxrot),  (udword) &(nebulaStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].maxhealth), (udword) &(nebulaStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].minCollDamage), (udword) &(nebulaStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(nebulaStaticInfos[0].maxCollDamage), (udword) &(nebulaStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(nebulaStaticInfos[0].resourcevalue), (udword) &(nebulaStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB, (udword) &(nebulaStaticInfos[0].harvestableByMultipleShips), (udword) &(nebulaStaticInfos[0]) },

    { "nebulaCanShrink",scriptSetBool8, (udword) &(nebulaStaticInfos[0].nebulaCanShrink), (udword) &(nebulaStaticInfos[0]) },
    { "nebulaMinShrink",scriptSetReal32CB, (udword) &(nebulaStaticInfos[0].nebulaMinShrink), (udword) &(nebulaStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry GasCloudStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(gascloudStaticInfos[0].staticheader.LOD), (udword) &(gascloudStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(gascloudStaticInfos[0].staticheader.pMexData), (udword) &(gascloudStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].staticheader.mass),     (udword) &(gascloudStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(gascloudStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(gascloudStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(gascloudStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(gascloudStaticInfos[0]) },

    { "immobile",        scriptSetBool8,    (udword) &(gascloudStaticInfos[0].staticheader.immobile), (udword) &(gascloudStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(gascloudStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(gascloudStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].staticheader.maxvelocity),  (udword) &(gascloudStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].staticheader.maxrot),  (udword) &(gascloudStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].maxhealth), (udword) &(gascloudStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].minCollDamage), (udword) &(gascloudStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(gascloudStaticInfos[0].maxCollDamage), (udword) &(gascloudStaticInfos[0]) },
    { "resourcevalue",  scriptSetSdwordCB,  (udword) &(gascloudStaticInfos[0].resourcevalue), (udword) &(gascloudStaticInfos[0]) },
    { "harvestableByMultipleShips",  scriptSetUdwordCB, (udword) &(gascloudStaticInfos[0].harvestableByMultipleShips), (udword) &(gascloudStaticInfos[0]) },

    { "gascloudCanShrink",scriptSetBool8, (udword) &(gascloudStaticInfos[0].gascloudCanShrink), (udword) &(gascloudStaticInfos[0]) },
    { "gascloudMinShrink",scriptSetReal32CB, (udword) &(gascloudStaticInfos[0].gascloudMinShrink), (udword) &(gascloudStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry DerelictStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(derelictStaticInfos[0].staticheader.LOD), (udword) &(derelictStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(derelictStaticInfos[0].staticheader.pMexData), (udword) &(derelictStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.mass),      (udword) &(derelictStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(derelictStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(derelictStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(derelictStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(derelictStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(derelictStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(derelictStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(derelictStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(derelictStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(derelictStaticInfos[0]) },
    { "rightOfWay",      scriptSetSbyteCB,  (udword) &(derelictStaticInfos[0].staticheader.rightOfWay), (udword) &(derelictStaticInfos[0]) },
    { "immobile",        scriptSetBool8,    (udword) &(derelictStaticInfos[0].staticheader.immobile), (udword) &(derelictStaticInfos[0]) },
    { "approxcollmodifier", scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.approxcollmodifier),  (udword) &(derelictStaticInfos[0]) },
    { "avoidcollmodifier", scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.staticCollInfo.avoidcollmodifier),  (udword) &(derelictStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.maxvelocity),  (udword) &(derelictStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].staticheader.maxrot),  (udword) &(derelictStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].maxhealth), (udword) &(derelictStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].minCollDamage), (udword) &(derelictStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].maxCollDamage), (udword) &(derelictStaticInfos[0]) },

    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(derelictStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(derelictStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(derelictStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(derelictStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(derelictStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(derelictStaticInfos[0]) },

    { "pitchdescend",               scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].pitchdescend), (udword) &(derelictStaticInfos[0]) },
    { "sinbank",                    scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].sinbank), (udword) &(derelictStaticInfos[0]) },
    { "pitchturn",                  scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].pitchturn), (udword) &(derelictStaticInfos[0]) },

    { "preciseSelection",           scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { "NUMBER_OF_SALVAGE_POINTS",   scriptSetSalvageStatCB,(udword) &(derelictStaticInfos[0]),(udword) &(derelictStaticInfos[0]) },

    { "scaleFactor",          scriptSetReal32CB,  (udword) &(derelictStaticInfos[0].scaleFactor), (udword) &(derelictStaticInfos[0]) },
    { "worldRender",                scriptSetBool,  (udword) &(derelictStaticInfos[0].worldRender), (udword) &(derelictStaticInfos[0]) },

    { "salvageable",                scriptSetBool,  (udword) &(derelictStaticInfos[0].salvageable), (udword) &(derelictStaticInfos[0]) },

    { "minimumZoomDistance",  scriptSetReal32CB,    (udword) &(derelictStaticInfos[0].minimumZoomDistance), (udword) &(derelictStaticInfos[0]) },
    { "renderlistFade",       scriptSetReal32CB,    (udword) &(derelictStaticInfos[0].renderlistFade),      (udword) &(derelictStaticInfos[0]) },
    { "renderlistLimit",      scriptSetReal32SqrCB, (udword) &(derelictStaticInfos[0].renderlistLimitSqr),  (udword) &(derelictStaticInfos[0]) },

    { NULL,NULL,0,0 }
};

scriptStructEntry MissileStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(missileStaticInfos[0].staticheader.LOD), (udword) &(missileStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(missileStaticInfos[0].staticheader.pMexData), (udword) &(missileStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(missileStaticInfos[0].staticheader.mass),      (udword) &(missileStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(missileStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(missileStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(missileStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(missileStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(missileStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(missileStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(missileStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(missileStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(missileStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(missileStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(missileStaticInfos[0].staticheader.maxvelocity),  (udword) &(missileStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(missileStaticInfos[0].staticheader.maxrot),  (udword) &(missileStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].maxhealth), (udword) &(missileStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].minCollDamage), (udword) &(missileStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].maxCollDamage), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(missileStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(missileStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(missileStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(missileStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(missileStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(missileStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(missileStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(missileStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(missileStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(missileStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },
    { NULL,NULL,0,0 }
};

scriptStructEntry MineStaticScriptTable[] =
{
    { "LODFile",        LODScriptLoad,      (udword) &(mineStaticInfos[0].staticheader.LOD), (udword) &(mineStaticInfos[0]) },
    { "pMexData",       loadMexDataCB,      (udword) &(mineStaticInfos[0].staticheader.pMexData), (udword) &(mineStaticInfos[0]) },
    { "mass",           scriptSetReal32CB,  (udword) &(mineStaticInfos[0].staticheader.mass),      (udword) &(mineStaticInfos[0]) },
    { "momentOfInertiaX",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.momentOfInertiaX), (udword) &(mineStaticInfos[0]) },
    { "momentOfInertiaY",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.momentOfInertiaY), (udword) &(mineStaticInfos[0]) },
    { "momentOfInertiaZ",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.momentOfInertiaZ), (udword) &(mineStaticInfos[0]) },
    { "collBoxScaleRight",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxScaleRight), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleUp",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxScaleUp), (udword) &(asteroidStaticInfos[0]) },
    { "collBoxScaleForward",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxScaleForward), (udword) &(asteroidStaticInfos[0]) },
    { "maxvelocity",    scriptSetReal32CB,  (udword) &(mineStaticInfos[0].staticheader.maxvelocity),  (udword) &(mineStaticInfos[0]) },
    { "collBoxOffsetX",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetX), (udword) &(mineStaticInfos[0]) },
    { "collBoxOffsetY",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetY), (udword) &(mineStaticInfos[0]) },
    { "collBoxOffsetZ",scriptSetReal32CB, (udword) &(mineStaticInfos[0].staticheader.staticCollInfo.collBoxOffsetZ), (udword) &(mineStaticInfos[0]) },
    { "maxvelocity_FORCED",    scriptSetReal32CB,  (udword) &(mineStaticInfos[0].maxvelocity_FORCED),  (udword) &(mineStaticInfos[0]) },
    { "maxrot",         scriptSetReal32CB,  (udword) &(mineStaticInfos[0].staticheader.maxrot),  (udword) &(mineStaticInfos[0]) },
    { "maxhealth",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].maxhealth), (udword) &(mineStaticInfos[0]) },
    { "minCollDamage",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].minCollDamage), (udword) &(mineStaticInfos[0]) },
    { "maxCollDamage",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].maxCollDamage), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_UP]",       scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_UP]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_DOWN]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_DOWN]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_RIGHT]",    scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_RIGHT]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_LEFT]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_LEFT]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_FORWARD]",  scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_FORWARD]), (udword) &(mineStaticInfos[0]) },
    { "thruststrength[TRANS_BACK]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].thruststrengthstat[TRANS_BACKWARD]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_YAWLEFT]",       scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_YAWLEFT]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_YAWRIGHT]",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_YAWRIGHT]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_PITCHUP]",       scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_PITCHUP]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_PITCHDOWN]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_PITCHDOWN]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_ROLLRIGHT]",     scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_ROLLRIGHT]), (udword) &(mineStaticInfos[0]) },
    { "rotstrength[ROT_ROLLLEFT]",      scriptSetReal32CB,  (udword) &(mineStaticInfos[0].rotstrengthstat[ROT_ROLLLEFT]), (udword) &(mineStaticInfos[0]) },
    { "turnspeed[TURN_YAW]",            scriptSetReal32CB,  (udword) &(mineStaticInfos[0].turnspeedstat[TURN_YAW]), (udword) &(mineStaticInfos[0]) },
    { "turnspeed[TURN_PITCH]",          scriptSetReal32CB,  (udword) &(mineStaticInfos[0].turnspeedstat[TURN_PITCH]), (udword) &(mineStaticInfos[0]) },
    { "turnspeed[TURN_ROLL]",           scriptSetReal32CB,  (udword) &(mineStaticInfos[0].turnspeedstat[TURN_ROLL]), (udword) &(mineStaticInfos[0]) },
    { "MINE_STOP_FRICTION",           scriptSetReal32CB,  (udword) &(mineStaticInfos[0].MINE_STOP_FRICTION), (udword) &(mineStaticInfos[0]) },
    { "MINE_RANGESQR",           scriptSetReal32SqrCB,  (udword) &(mineStaticInfos[0].MINE_RANGESQR), (udword) &(mineStaticInfos[0]) },
    { "MINE_RANGESQR_FORCED",           scriptSetReal32SqrCB,  (udword) &(mineStaticInfos[0].MINE_RANGESQR_FORCED), (udword) &(mineStaticInfos[0]) },
    { "preciseSelection",scriptSetUdwordCB,(udword) &(shipStaticInfos[0].staticheader.staticCollInfo.preciseSelection), (udword) &(shipStaticInfos[0]) },

    { NULL,NULL,0,0 }
};

/*=============================================================================
    Private Functions:
=============================================================================*/

/*=============================================================================
    The following functions are callback functions which fill in
    information in the static structures.  They are called by scriptSetStruct
=============================================================================*/
//bind a hierarchy element to a gun and set the hierarchy size
static void setGunBindInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char *hString, *string, *gString;
    sdword gunIndex, nScanned, index, nBound;
    mhbinding *binding;
    lod *level;
    udword flags = 0;
    sdword frequency;

    //first string is always name of hierarchy member.
    hString = strtok(field, " ,");
    //second string is the gun mex name
    gString = strtok(NULL, " ,");
    nScanned = sscanf(gString, "Gun%d", &gunIndex);         //get gun #
    dbgAssert(nScanned == 1);                               //verify it
    dbgAssert(gunIndex >= 0 && gunIndex < info->gunStaticInfo->numGuns);

    //parse the modifiers
    while ((string = strtok(NULL, " ,")) != NULL)
    {
        if (_stricmp(string, "Azimuth") == 0)
        {
            flags |= HBF_Azimuth;
        }
        else if (_stricmp(string, "Declination") == 0)
        {
            flags |= HBF_Declination;
        }
        else if (_stricmp(string, "Recoil") == 0)
        {
            flags |= HBF_Recoil;
        }
        else if (_stricmp(string, "Frequency") == 0)
        {
            string = strtok(NULL, " ,");
            dbgAssert(string);
            nScanned = sscanf(string, "%d", &frequency);
            dbgAssert(nScanned == 1);
            dbgAssert(frequency >= 1 && frequency < HB_UpdateFrequencyMax);
            flags |= frequency;
        }
    }
    info->hierarchySize = lodHierarchySizeCompute(info->staticheader.LOD);
    nBound = 0;
    for (index = 0; index < info->staticheader.LOD->nLevels; index++)
    {                                                       //for all levels of detail
        level = &info->staticheader.LOD->level[index];
        binding = NULL;
        if ((level->flags & LM_LODType) != LT_Mesh)
        {
            continue;
        }
        while ((binding = meshBindingFindByName(binding, level->hBindings, (meshdata *)level->pData, hString)) != NULL)
        {
            binding->function = gunMatrixUpdate;            //bind all named members to this gun
            binding->userData = &info->gunStaticInfo->gunstatics[gunIndex];
            binding->userID = gunIndex;
            binding->flags = flags;
            nBound++;
        }
    }
#if MESH_VERBOSE_LEVEL >= 1
    if (nBound == 0)
    {
        dbgWarningf(DBG_Loc, "Could not bind '%s' to '%s' because object not found.", hString, gString);
    }
#endif
#if MESH_VERBOSE_LEVEL >= 2
    dbgMessagef("\nsetGunBindInfo: bound '%s' to '%s' %d times", hString, gString, nBound++);
#endif
}

static void setSupportUnitsInfo(char *directory, char *field, void *dataToFillIn)
{
    ShipStaticInfo *info = dataToFillIn;
    udword value = 0;

    (void)directory;
    dbgAssert(info != NULL);
    sscanf(field, "%u", &value);
    shipSupportUnits[info->shiprace][info->shiptype] = value;
}

static void setProvidesSUInfo(char *directory, char *field, void *dataToFillIn)
{
    ShipStaticInfo *info = dataToFillIn;
    udword value = 0;

    (void)directory;
    dbgAssert(info != NULL);
    sscanf(field, "%u", &value);
    shipProvidesSU[info->shiprace][info->shiptype] = value;
}

udword ShipStaticInfoGetSupportUnits(ShipStaticInfo *statinfo)
{
    dbgAssert(statinfo != NULL);
    return shipSupportUnits[statinfo->shiprace][statinfo->shiptype];
}

udword ShipStaticInfoGetProvidesSU(ShipStaticInfo *statinfo)
{
    dbgAssert(statinfo != NULL);
    return shipProvidesSU[statinfo->shiprace][statinfo->shiptype];
}

static void setAnimationBindInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char *shipName;
    char fileName[256];

    shipName = ShipTypeToStr(info->shiptype);
    sprintf(fileName, "%s%s%s", directory, shipName, MAD_Extension);//make path to file
    info->madStatic = memAlloc(sizeof(madstatic), "madBindings", NonVolatile);
    memset(info->madStatic,0,sizeof(madstatic));
    info->madStatic->header = madFileLoad(fileName);
    info->hierarchySize = lodHierarchySizeCompute(info->staticheader.LOD);
}

// Cataclysm addition: handles NUMBER_OF_ANIMS keyword in .shp files.
// Ships with .mad animations but no BindAnimations keyword use this to load
// the .mad file and set up hierarchySize for mesh bindings.
static void setNumberOfAnimsInfo(char *directory, char *field, void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char *shipName;
    char fileName[256];
    sdword numAnims;
    sdword i;

    sscanf(field, "%d", &numAnims);

    shipName = ShipTypeToStr(info->shiptype);
    sprintf(fileName, "%s%s%s", directory, shipName, MAD_Extension);

    if (!fileExists(fileName, 0))
    {
        return;
    }

    info->madStatic = memAlloc(sizeof(madstatic), "madBindings", NonVolatile);
    memset(info->madStatic, 0, sizeof(madstatic));
    info->madStatic->header = madFileLoad(fileName);
    info->hierarchySize = lodHierarchySizeCompute(info->staticheader.LOD);

    info->madStatic->animIndexRemapCount = numAnims;
    if (numAnims > 0)
    {
        info->madStatic->animIndexRemap = memAlloc(sizeof(sdword) * numAnims, "AnimIndexRemap", NonVolatile);
        for (i = 0; i < numAnims; i++)
        {
            info->madStatic->animIndexRemap[i] = i;
        }
    }
}

// Cataclysm addition: handles ANIM keyword, stores animation name-to-index mapping.
static void setAnimInfo(char *directory, char *field, void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char *shipName;
    char fileName[256];
    sdword animIndex;
    sdword madIndex;
    char animName[256];

    sscanf(field, "%d %s", &animIndex, animName);

    if (info->madStatic != NULL && info->madStatic->header != NULL &&
        animIndex >= 0 && animIndex < info->madStatic->animIndexRemapCount &&
        info->madStatic->animIndexRemap != NULL)
    {
        for (madIndex = 0; madIndex < info->madStatic->header->nAnimations; madIndex++)
        {
            if (_stricmp(info->madStatic->header->anim[madIndex].name, animName) == 0)
            {
                info->madStatic->animIndexRemap[animIndex] = madIndex;
                return;
            }
        }

        shipName = ShipTypeToStr(info->shiptype);
        sprintf(fileName, "%s%s%s", directory, shipName, MAD_Extension);
        dbgMessagef("\nsetMadAnim(): Animation %s not found in %s", animName, fileName);
    }
}

static void setMadStartInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    char madAnimName[256];
    udword startFlag;
    char madAnimName2[256];
    udword startFlag2;
    sscanf(field,"%d , %s , %d , %s",&startFlag,madAnimName,&startFlag2,madAnimName2);

    if(startFlag == 1)
    {
        info->madStatic->needStartPlacedAnimation = TRUE;
        info->madStatic->startPlacedAnimationIndex = madAnimIndexFindByName(info->madStatic->header, madAnimName);
    }
    else
    {
        //no need to start and then pause an animation on creation
        info->madStatic->needStartPlacedAnimation = FALSE;
    }

    if(startFlag2 == 1)
    {
        info->madStatic->needStartBuiltAnimation = TRUE;
        info->madStatic->startBuiltAnimationIndex = madAnimIndexFindByName(info->madStatic->header, madAnimName2);
    }
    else
    {
        //no need to start and then pause an animation on creation
        info->madStatic->needStartBuiltAnimation = FALSE;
    }


}
 /*
    sdword *gunOpenIndexs;                      //arrays for gunopening/closing animation indexes both damaged and undamaged.  So we can support a number of different animationsfor the same ship
    sdword *gunOpenDamagedIndexes;
    sdword *gunCloseIndexes;
    sdword *gunCloseDamagedIndexes;
 */
static void setGunAnimInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    sdword i;
    char madAnimName[256];   //maximum of 4 animations so far
    sdword numAnims;
    char *token;
    char seps[] = ", ";

    token = strtok(field,seps);

    //do normal gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunOpenIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunOpenIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunOpenIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do normal gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunCloseIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunCloseIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunCloseIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunOpenDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunOpenDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunOpenDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numGunCloseDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->gunCloseDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->gunCloseDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }
}

static void setDockAnimInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    sdword i;
    char madAnimName[256];   //maximum of 4 animations so far
    sdword numAnims;
    char *token;
    char seps[] = ", ";

    token = strtok(field,seps);

    //do normal gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numDockIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->PostDockIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->PostDockIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do normal gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numPostDockIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->DockIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
           info->madStatic->DockIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


       token = strtok(NULL,seps);
    //do damaged gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numPostDockDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->PostDockDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->PostDockDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do damaged gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numDockDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->DockDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"GunOpenMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->DockDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

}
void setMothershipDoorAnimInfo(char *directory,char *field,void *dataToFillIn)
{
     ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
     sdword i;
     char madAnimName[256];   //maximum of 4 animations so far
     sdword numAnims;
     char *token;
     char seps[] = ", ";

     token = strtok(field,seps);

     //do normal gun opening animations
     sscanf(token,"%d",&numAnims);
     info->madStatic->numDoorOpenIndexes = numAnims;
     if(numAnims > 0)
     {
         info->madStatic->DoorOpenIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"DoorOpenMadAnimIndex",NonVolatile);
         for(i=0;i<numAnims;i++)
         {
             token = strtok(NULL,seps);
             sscanf(token,"%s",madAnimName);
             info->madStatic->DoorOpenIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
         }
     }

     token = strtok(NULL,seps);
     //do normal gun closing animations
     sscanf(token,"%d",&numAnims);
     info->madStatic->numDoorCloseIndexes = numAnims;
     if(numAnims > 0)
     {
         info->madStatic->DoorCloseIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"DoorCloseMadAnimIndex",NonVolatile);
         for(i=0;i<numAnims;i++)
         {
             token = strtok(NULL,seps);
             sscanf(token,"%s",madAnimName);
            info->madStatic->DoorCloseIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
         }
     }
}
void setSpecialCaseAnimInfo(char *directory,char *field,void *dataToFillIn)
{
    ShipStaticInfo *info = (ShipStaticInfo *)dataToFillIn;
    sdword i;
    char madAnimName[256];   //maximum of 4 animations so far
    sdword numAnims;
    char *token;
    char seps[] = ", ";

    token = strtok(field,seps);

    //do normal gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialOpenIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialOpenIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialOpenIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

    token = strtok(NULL,seps);
    //do normal gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialCloseIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialCloseIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialCloseIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun opening animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialOpenDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialOpenDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialOpenDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }


    token = strtok(NULL,seps);
    //do damaged gun closing animations
    sscanf(token,"%d",&numAnims);
    info->madStatic->numSpecialCloseDamagedIndexes = numAnims;
    if(numAnims > 0)
    {
        info->madStatic->specialCloseDamagedIndexes = (sdword *) memAlloc(sizeof(sdword)*numAnims,"SpecialMadAnimIndex",NonVolatile);
        for(i=0;i<numAnims;i++)
        {
            token = strtok(NULL,seps);
            sscanf(token,"%s",madAnimName);
            info->madStatic->specialCloseDamagedIndexes[i] = madAnimIndexFindByName(info->madStatic->header, madAnimName);
        }
    }

}
static void LODScriptLoad(char *directory,char *field,void *dataToFillIn)
{
    if (_stricmp(field,"DefaultShip") == 0)
    {
        *((lodinfo **)dataToFillIn) = defaultlod;
    }
    else
    {
        *((lodinfo **)dataToFillIn) = lodTableReadScript(directory, field);
    }
}
static void loadMexDataCB(char *directory,char *field,void *dataToFillIn)
{
    char fullfilename[80];

    if (_stricmp(field,"DefaultShip") == 0)
    {
        *((void **)dataToFillIn) = defaultmex;
    }
    else
    {
        if (directory != NULL)
        {
            strcpy(fullfilename,directory);
            strcat(fullfilename,field);
        }
        else
        {
            strcpy(fullfilename,field);
        }

        if (fileExists(fullfilename,0))
        {
            *((void **)dataToFillIn) = mexLoad(fullfilename);
        }
        else
        {
            *((void **)dataToFillIn) = defaultmex;
        }
    }
}

static void setClassCB(char *directory,char *field,void *dataToFillIn)
{
    *((ShipClass *)dataToFillIn) = StrToShipClass(field);
}

static void scriptSetFramesCB(char *directory,char *field,void *dataToFillIn)
{
    sscanf(field,"%d",(sdword *)dataToFillIn);
    *((sdword *)dataToFillIn) *= UNIVERSE_UPDATE_RATE;
}

void gunGetDefaultOrientationFromBindings(GunStatic *gunstatic,ShipStaticInfo *statinfo)
{
    lod *level = &statinfo->staticheader.LOD->level[0];
    udword numHierarchys = ((meshdata *)level->pData)->nPolygonObjects;
    hmatrix concatenated, nonConcatenated;
//    mhbinding *binding;

    if (meshFindHierarchyMatrixByUserData(&concatenated, &nonConcatenated, level->hBindings, gunstatic) != FALSE)
    {
        matGetMatFromHMat(&gunstatic->defaultgunorientation,&concatenated);
        matGetMatFromHMat(&gunstatic->defaultGunOrientationNonConcat,&nonConcatenated);
        if (memcmp(&gunstatic->defaultGunOrientationNonConcat,
                   &gunstatic->defaultgunorientation, sizeof(matrix)))
        {                                                   //if these guys are different
            bitSet(gunstatic->flags, GF_MultiLevelMatrix);
        }
    }
    else
    {
        dbgMessagef("\nWARNING: could not set gunbindings for ship %d",statinfo->shiptype);
    }

/*
    binding = level->hBindings;

    // later, take into account multiple hierarchies for a single gun
    while (numHierarchys > 0)
    {
        if (binding->userData == gunstatic)
        {
            matGetMatFromHMat(&gunstatic->defaultgunorientation,&binding->object->localMatrix);
            return;
        }
        binding++;
        numHierarchys--;
    }
*/
}

void gunSetStaticGunInfoFromBindings(ShipStaticInfo *statinfo)
{
    sdword i;
    GunStaticInfo *gunStaticInfo = statinfo->gunStaticInfo;
    sdword numGuns = gunStaticInfo->numGuns;
    GunStatic *gunstatic;

    for (i=0;i<numGuns;i++)
    {
        gunstatic = &gunStaticInfo->gunstatics[i];

        switch (gunstatic->guntype)
        {
            case GUN_NewGimble:
                gunGetDefaultOrientationFromBindings(gunstatic,statinfo);
                break;

            default:
                // ignore
                break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : precalcStaticHeader
    Description : precalculates certain useful information in the static header
                  so these calculations don't have to be done every frame
    Inputs      : *staticheader
    Outputs     : *staticheader updated appropriately
    Return      :
----------------------------------------------------------------------------*/
void precalcStaticHeader(StaticHeader *staticheader)
{
    real32 avoidcollsize;

    mexSetCollisionInfo(&staticheader->staticCollInfo,staticheader->pMexData,
                        staticheader->staticCollInfo.collBoxScaleForward,
                        staticheader->staticCollInfo.collBoxScaleUp,
                        staticheader->staticCollInfo.collBoxScaleRight,
                        staticheader->staticCollInfo.collBoxOffsetX,
                        staticheader->staticCollInfo.collBoxOffsetY,
                        staticheader->staticCollInfo.collBoxOffsetZ);

    if (staticheader->staticCollInfo.approxcollmodifier != 0.0f)
    {
        staticheader->staticCollInfo.approxcollspheresize *= staticheader->staticCollInfo.approxcollmodifier;
    }

    if (staticheader->staticCollInfo.avoidcollmodifier != 0.0f)
    {
        staticheader->staticCollInfo.avoidcollspheresize *= staticheader->staticCollInfo.avoidcollmodifier;
    }

    avoidcollsize = staticheader->staticCollInfo.avoidcollspheresize;
    staticheader->staticCollInfo.avoidcollspherepad = avoidcollsize * ((avoidcollsize > AISHIP_BIGSHIP) ? AVOID_OBJ_PADDING_SCALE_BIG : AVOID_OBJ_PADDING_SCALE);

    staticheader->oneOverMass =             1.0f /      staticheader->mass;
    staticheader->oneOverMomentOfInertiaX = 1.0f /      staticheader->momentOfInertiaX;
    staticheader->oneOverMomentOfInertiaY = 1.0f /      staticheader->momentOfInertiaY;
    staticheader->oneOverMomentOfInertiaZ = 1.0f /      staticheader->momentOfInertiaZ;
}

void precalcStaticInfoHealth(StaticInfoHealth *staticinfo)
{
    if ((staticinfo->minCollDamage == 0.0f) && (staticinfo->maxCollDamage == 0.0f))
    {
        // put in defaults:
        staticinfo->minCollDamage = ((staticinfo->maxhealth) * COLLDAMAGE_DEFAULTMIN_FRACTION);
        staticinfo->maxCollDamage = ((staticinfo->maxhealth) * COLLDAMAGE_DEFAULTMAX_FRACTION);
    }
    if(staticinfo->maxhealth != 0.0f)
    {
        //precalc 1/maxhealth..duh
        staticinfo->oneOverMaxHealth = 1.0f/staticinfo->maxhealth;
    }
}

void precalcStaticInfoGuidance(StaticInfoHealthGuidance *staticinfo)
{
    sdword i;

    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        staticinfo->thruststrengthstat[i] *= staticinfo->staticheader.mass * GLOBAL_SHIP_SPEED_MODIFIER;
    }

    staticinfo->staticheader.maxvelocity *= GLOBAL_SHIP_SPEED_MODIFIER;

    staticinfo->rotstrengthstat[ROT_ABOUTXCCW] *= staticinfo->staticheader.momentOfInertiaX;
    staticinfo->rotstrengthstat[ROT_ABOUTXCW]  *= staticinfo->staticheader.momentOfInertiaX;
    staticinfo->rotstrengthstat[ROT_ABOUTYCCW] *= staticinfo->staticheader.momentOfInertiaY;
    staticinfo->rotstrengthstat[ROT_ABOUTYCW]  *= staticinfo->staticheader.momentOfInertiaY;
    staticinfo->rotstrengthstat[ROT_ABOUTZCCW] *= staticinfo->staticheader.momentOfInertiaZ;
    staticinfo->rotstrengthstat[ROT_ABOUTZCW]  *= staticinfo->staticheader.momentOfInertiaZ;
}

/*-----------------------------------------------------------------------------
    Name        : InitStatAsteroidInfo
    Description : initializes static asteroid information
    Inputs      : asteroidType
    Outputs     : asteroidStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatAsteroidInfo(AsteroidStaticInfo *asteroidStatInfo,AsteroidType asteroidtype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *asteroidtypestr;

    asteroidStatInfo->asteroidtype = asteroidtype;

    asteroidtypestr = AsteroidTypeToStr(asteroidtype);

    strcpy(directory,"Resources\\Asteroids\\");
//    strcat(directory,asteroidtypestr);
//    strcat(directory,"\\");

    strcpy(shipname,asteroidtypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        scriptSetStruct(directory,shipname,AsteroidStaticScriptTable,(ubyte *)asteroidStatInfo);

        precalcStaticHeader(&asteroidStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth *)asteroidStatInfo);

        // optimization for Asteroid0
        if (asteroidtype == Asteroid0) collZeroRectInfo(&asteroidStatInfo->staticheader.staticCollInfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatDustCloudInfo
    Description : initializes static dustcloud information
    Inputs      : dustcloudType
    Outputs     : dustcloudStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatDustCloudInfo(DustCloudStaticInfo *dustcloudStatInfo, DustCloudType dustcloudtype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *dustcloudtypestr;

    dbgAssert(dustcloudtype < NUM_DUSTCLOUDTYPES);

    dustcloudStatInfo->dustcloudtype = dustcloudtype;

    dustcloudtypestr = DustCloudTypeToStr(dustcloudtype);

    strcpy(directory,"Resources\\DustClouds\\");
//    strcat(directory,dustcloudtypestr);
//    strcat(directory,"\\");

    strcpy(shipname,dustcloudtypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        StaticCollInfo *coll;
        real32 r = DUSTCLOUD_RADII[(sdword)dustcloudtype] * DUSTCLOUD_RADIAL_SCALE;

        scriptSetStruct(directory,shipname,DustCloudStaticScriptTable,(ubyte *)dustcloudStatInfo);

        precalcStaticHeader(&dustcloudStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth *)dustcloudStatInfo);

        coll = &dustcloudStatInfo->staticheader.staticCollInfo;
        collZeroRectInfo(coll);

        coll->originalcollspheresize = r;
        coll->approxcollspheresize = r;
        coll->avoidcollspheresize = r;
        coll->avoidcollspherepad = r;
        coll->collspheresize = r;
        coll->collspheresizeSqr = r * r;
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatGasCloudInfo
    Description : initializes static gascloud information
    Inputs      : gascloudType
    Outputs     : gascloudStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatGasCloudInfo(GasCloudStaticInfo *gascloudStatInfo, GasCloudType gascloudtype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *gascloudtypestr;

    gascloudStatInfo->gascloudtype = gascloudtype;

    gascloudtypestr = GasCloudTypeToStr(gascloudtype);

    strcpy(directory,"Resources\\GasClouds\\");
//    strcat(directory,gascloudtypestr);
//    strcat(directory,"\\");

    strcpy(shipname,gascloudtypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        scriptSetStruct(directory,shipname,GasCloudStaticScriptTable,(ubyte *)gascloudStatInfo);

        precalcStaticHeader(&gascloudStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth *)gascloudStatInfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatNebulaInfo
    Description : initializes static nebula information
    Inputs      : nebulaType
    Outputs     : nebulaStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatNebulaInfo(NebulaStaticInfo* nebulaStatInfo, NebulaType nebulatype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char* nebulatypestr;

    dbgAssert(nebulatype < NUM_NEBULATYPES);

    nebulaStatInfo->nebulatype = nebulatype;

    nebulatypestr = NebulaTypeToStr(nebulatype);

    strcpy(directory,"Resources\\Nebulae\\");
//    strcat(directory,nebulatypestr);
//    strcat(directory,"\\");

    strcpy(shipname,nebulatypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (fileExists(fullshipname,0))
    {
        StaticCollInfo *coll;

        scriptSetStruct(directory, shipname, NebulaStaticScriptTable, (ubyte*)nebulaStatInfo);

        precalcStaticHeader(&nebulaStatInfo->staticheader);
        precalcStaticInfoHealth((StaticInfoHealth*)nebulaStatInfo);

        coll = &nebulaStatInfo->staticheader.staticCollInfo;
        collZeroRectInfo(coll);

        coll->originalcollspheresize = NEB_RADIUS;
        coll->approxcollspheresize = NEB_RADIUS;
        coll->avoidcollspheresize = NEB_RADIUS;
        coll->avoidcollspherepad = NEB_RADIUS;
        coll->collspheresize = NEB_RADIUS;
        coll->collspheresizeSqr = NEB_RADIUS * NEB_RADIUS;
    }
}

void InitStatHyperspaceGateInfo(DerelictStaticInfo *derelictStatInfo)
{
    StaticCollInfo *sci;
    real32 r;

    memset(derelictStatInfo, 0, sizeof(DerelictStaticInfo));

    derelictStatInfo->maxhealth = HYPERSPACEGATE_HEALTH;
    derelictStatInfo->oneOverMaxHealth = (1.0f / derelictStatInfo->maxhealth);

    derelictStatInfo->renderlistLimitSqr = 5.0e8f;

    sci = &derelictStatInfo->staticheader.staticCollInfo;

    sci->uplength = HYPERSPACEGATE_HEIGHT;
    sci->rightlength = HYPERSPACEGATE_WIDTH;
    sci->forwardlength = 50.0f;     // give it a small thickness for collision's sake

    vecSet(sci->collrectoffset,-HYPERSPACEGATE_HEIGHT/2,-HYPERSPACEGATE_WIDTH/2,-50.0/2);

    r = max(HYPERSPACEGATE_HEIGHT,HYPERSPACEGATE_WIDTH);

    sci->approxcollspheresize = r;
    sci->avoidcollspheresize = r;
    sci->avoidcollspherepad = r;
    sci->avoidcollmodifier = 1.0f;
    sci->approxcollmodifier = 1.0f;

    sci->originalcollspheresize = r;
    sci->collspheresize = r;
    sci->collspheresizeSqr = r*r;

    vecSet(sci->collsphereoffset,0.0f,0.0f,0.0f);
}

/*-----------------------------------------------------------------------------
    Name        : InitStatDerelictInfo
    Description : initializes static derelict information
    Inputs      : derelicttype
    Outputs     : derelictStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatDerelictInfo(DerelictStaticInfo *derelictStatInfo, DerelictType derelicttype)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *derelicttypestr;

    if (derelicttype == HyperspaceGate)
    {
        InitStatHyperspaceGateInfo(derelictStatInfo);
        return;
    }

    derelicttypestr = DerelictTypeToStr(derelicttype);

    strcpy(directory,"Derelicts\\");
//    strcat(directory,derelicttypestr);
//    strcat(directory,"\\");

    strcpy(shipname,derelicttypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    derelictStatInfo->minimumZoomDistance = 0.0f;
    derelictStatInfo->renderlistFade = 0.0f;
    derelictStatInfo->renderlistLimitSqr = 0.0f;

    if (fileExists(fullshipname,0))
    {
        InitStatDerelictInfoByPath(derelictStatInfo, derelicttype, directory,shipname);
    }
    scriptSetNAVLightStatics(directory, shipname, (ShipStaticInfo *)derelictStatInfo);
    if (derelictStatInfo->navlightStaticInfo != NULL)
    {
        MEXNAVLightContext navLightContext;

        navLightContext.lodInfo = derelictStatInfo->staticheader.LOD;
        navLightContext.rawMexData = derelictStatInfo->staticheader.pMexData;
        mexGetNAVLightStaticInfo(derelictStatInfo->navlightStaticInfo, &navLightContext);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatDerelictInfoByPath
    Description : Init the static info for a derelict based on a type AND a path.
                  Mostly used for creating generic objects for NIS's.
    Inputs      : derelictStatInfo - where to load the data to.
                  derelictType - type of derelict.  If only used in NIS, this
                    field is not used.
                  directory - what directory the .shp file is in.
                  shipFile - what .shp file to load in.
    Outputs     : reads in the representative .shp file into derelictStatInfo.
    Return      : void
----------------------------------------------------------------------------*/
void InitStatDerelictInfoByPath(DerelictStaticInfo *derelictStatInfo, DerelictType derelicttype, char *directory, char *shipFile)
{
    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];

    memcpy(teamColor, derelictStatInfo->teamColor, sizeof(teamColor));//save the default team colors
    derelictStatInfo->derelicttype = derelicttype;
    memset(derelictStatInfo, 0, sizeof(DerelictStaticInfo));
    memcpy(derelictStatInfo->teamColor, teamColor, sizeof(teamColor));
    derelictStatInfo->staticheader.rightOfWay = 1;      // default to derelicts having a right of way same as frigates
    derelictStatInfo->salvageable = FALSE;              //default salvageability to FALSE!
    scriptSetStruct(directory,shipFile,DerelictStaticScriptTable,(ubyte *)derelictStatInfo);

    precalcStaticHeader(&derelictStatInfo->staticheader);
    precalcStaticInfoHealth((StaticInfoHealth *)derelictStatInfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)derelictStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : InitStatMissileInfo
    Description : initializes static missile information
    Inputs      : race
    Outputs     : missileStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatMissileInfo(MissileStaticInfo *missileStatInfo,ShipRace race)
{
    char directory[80];
    char fullshipname[100];
    char *shipname = "Missile.shp";

    strcpy(directory,ShipRaceToStr(race));
//    strcat(directory,"\\Missile\\");
    strcat(directory, "\\");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (!fileExists(fullshipname,0))
    {
        dbgMessagef("\nWarning: Could not find Missile for %s",ShipRaceToStr(race));
        return;
    }

    missileStatInfo->turnspeedstat[TURN_YAW]   = 1.0f;
    missileStatInfo->turnspeedstat[TURN_PITCH] = 1.0f;
    missileStatInfo->turnspeedstat[TURN_ROLL]  = 1.0f;

    missileStatInfo->staticheader.rightOfWay = -1;

    scriptSetStruct(directory,shipname,MissileStaticScriptTable,(ubyte *)missileStatInfo);

    precalcStaticHeader(&missileStatInfo->staticheader);
    precalcStaticInfoHealth((StaticInfoHealth *)missileStatInfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)missileStatInfo);

    //missile trail stuff
    missileStatInfo->trailWidth = -1.0f;
    missileStatInfo->trailHeight = -1.0f;
    missileStatInfo->trailStatic = trailStaticInfoParse(directory, shipname);
}

/*-----------------------------------------------------------------------------
    Name        : InitStatMineInfo
    Description : initializes static missile-mine information
    Inputs      : race
    Outputs     : missileStatInfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatMineInfo(MissileStaticInfo *mineStatInfo,ShipRace race)
{
    char directory[80];
    char fullshipname[100];
    char *shipname = "Mine.shp";

    strcpy(directory,ShipRaceToStr(race));
//    strcat(directory,"\\Mine\\");
    strcat(directory, "\\");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (!fileExists(fullshipname,0))
    {
        dbgMessagef("\nWarning: Could not find Mine for %s",ShipRaceToStr(race));
        return;
    }

    mineStatInfo->turnspeedstat[TURN_YAW]   = 1.0f;
    mineStatInfo->turnspeedstat[TURN_PITCH] = 1.0f;
    mineStatInfo->turnspeedstat[TURN_ROLL]  = 1.0f;

    mineStatInfo->staticheader.rightOfWay = -1;

    scriptSetStruct(directory,shipname,MineStaticScriptTable,(ubyte *)mineStatInfo);

    mineStatInfo->mineRange = fsqrt(mineStatInfo->MINE_RANGESQR);
    mineStatInfo->mineRangeForced = fsqrt(mineStatInfo->MINE_RANGESQR_FORCED);

    precalcStaticHeader(&mineStatInfo->staticheader);
    precalcStaticInfoHealth((StaticInfoHealth *)mineStatInfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)mineStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : isStrikeCraft
    Description : returns TRUE if ship is a strikecraft
    Inputs      : shiptype
    Outputs     :
    Return      : returns TRUE if ship is a strikecraft
----------------------------------------------------------------------------*/
bool32 isStrikeCraft(ShipType shiptype)
{
    switch (shiptype)
    {
        case sRecon:
        case sAcolyte:
        case sMimic:
        case sLeech:
        case sACV:
        case sMCV:
        case bRecon:
        case bAcolyte:
        case bInterceptor:
        case bCloakedFighter:
        case bAttackBomber:
        case bACV:
        case kAttackBomber:
        case kHeavyCorvette:
        case kHeavyDefender:
        case kHeavyInterceptor:
        case kMultiGunCorvette:
        case kSalCapCorvette:
        case tAttackBomber:
        case tHeavyDefender:
        case tHeavyInterceptor:
        case tMinelayerCorvette:
        case tMultiGunCorvette:
        case tSalCapCorvette:
        case P1Fighter:
        case P1MissileCorvette:
        case P1StandardCorvette:
            return TRUE;

        default:
            return FALSE;
    }
}

static void InitStatShipTypeFlags(ShipStaticInfo *statinfo)
{
    statinfo->shipFlags = 0;

    switch (statinfo->shiptype)
    {
        case sRecon:
        case sSentinel:
        case bRecon:
        case bSentinel:
        case kProximitySensor:
        case tProximitySensor:
        case trFloatingCity:
        case trResearchStation:
        case dAcolyte:
            statinfo->shipFlags |= 0x10000;
            break;

        case sAcolyte:
        case bAcolyte:
            statinfo->shipFlags |= 0x20;
            break;

        case sMimic:
            statinfo->shipFlags |= 0x20020;
            break;

        case sLeech:
            statinfo->shipFlags |= 0x20300;
            break;

        case sACV:
        case bACV:
            statinfo->shipFlags |= 0x40;
            break;

        case sMCV:
            statinfo->shipFlags |= 0x20040;
            break;

        case sMultiBeamFrigate:
        case bMultiBeamFrigate:
            statinfo->shipFlags |= 0x80;
            break;

        case sHiveFrigate:
        case bHiveFrigate:
        case dRKO:
            statinfo->shipFlags |= 0x8000;
            break;

        case sDreadnought:
        case sDestroyer:
        case tMissileDestroyer:
        case p1MissileCorvette:
            statinfo->shipFlags |= 0x400;
            break;

        case sCarrier:
        case bCarrier:
        case kCarrier:
        case tCarrier:
            statinfo->shipFlags |= 0x8012;
            break;

        case sWorker:
        case bWorker:
        case tResourceCollector:
            statinfo->shipFlags |= 0x1;
            break;

        case sMothership:
        case bMothership:
        case dBeastMothership:
            statinfo->shipFlags |= 0x800E;
            break;

        case sMothershipSpecial:
        case sMothershipEngineering:
        case sMothershipArmour:
        case sMothershipMicro:
        case sMothershipWeapons:
        case sMothershipDockingBay:
        case sMothershipSupport:
        case sMothershipCommand:
        case sMothershipEnginesLower:
        case sMothershipEnginesUpper:
        case sMothershipLowerSpine:
        case sMothershipBigGun:
        case sMothershipOreCanArmour:
        case bMothershipDockingBay:
        case bMothershipEnginesLower:
        case bSectMoship:
        case bMothershipSupport:
            statinfo->shipFlags |= 0x1000;
            break;

        case bCloakedFighter:
        case bIonArrayFrigate:
        case p1IonArrayFrigate:
        case p1StandardCorvette:
            statinfo->shipFlags |= 0x20000;
            statinfo->shipFlags |= 0x20;
            break;

        case tMinelayerCorvette:
            statinfo->shipFlags |= 0x800;
            break;

        case dSlipgateSmall:
        case dSlipgateMedium:
        case dSlipgateLarge:
            statinfo->shipFlags |= 0xC000;
            break;

        default:
            break;
    }
}

static void shipStaticCacheCenterDockPoint(DockStaticInfo *dockStaticInfo)
{
    sdword i;

    dockStaticInfo->pad_4C = 0;
    for (i = 0; i < dockStaticInfo->numDockPoints; i++)
    {
        DockStaticPoint *dockStaticPoint = &dockStaticInfo->dockstaticpoints[i];

        if (_stricmp(dockStaticPoint->name, "Center") == 0)
        {
            dockStaticInfo->pad_4C = (udword)(uintptr_t)dockStaticPoint;
            break;
        }
    }
}

static void shipStaticApplyDockStorageFlags(ShipStaticInfo *statinfo)
{
    DockStaticInfo *dockStaticInfo;
    sdword i;

    dockStaticInfo = statinfo->dockStaticInfo;
    if (dockStaticInfo == NULL)
    {
        return;
    }

    for (i = 0; i < dockStaticInfo->numDockPoints; i++)
    {
        if (dockStaticInfo->dockstaticpoints[i].type == DPT_Swarmer_In)
        {
            bitSet(statinfo->shipFlags, BIT15);
            break;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatShipInfo
    Description : Initializes static ship information for all ship types.
    Inputs      : type, race
    Outputs     : statinfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatShipInfo(ShipStaticInfo *statinfo,ShipType type,ShipRace race)
{
    char shipname[80];
    char directory[80];
    char fullshipname[160];
    char *shiptypestr;
    sdword i;
    bool32 stubbedOut;

    statinfo->shiptype = type;
    statinfo->shiprace = race;
    shipSupportUnits[race][type] = 0;
    shipProvidesSU[race][type] = 0;

    statinfo->custshipheader = *(shipHeaders[type]);
    dbgAssert((statinfo->shiptype == (ShipType)-1) || (type == statinfo->shiptype));

    shiptypestr = ShipTypeToStr(type);

    strcpy(directory,ShipRaceToStr(race));
//    strcat(directory,"\\");
//    strcat(directory,shiptypestr);
    strcat(directory,"\\");

    strcpy(shipname,shiptypestr);
    strcat(shipname,".shp");

    strcpy(fullshipname,directory);
    strcat(fullshipname,shipname);

    if (!fileExists(fullshipname,0) || universeForceDefaultShip)
    {
        //we now disable this ship if we can't find it...
        SetInfoNeededForShipAndRelatedStaticInfo(type,race,FALSE);
        rmEnableShip(race, type, FALSE);
        return;
/*

#ifndef HW_Release
        dbgMessagef("\nStubbing out %s\\%s", ShipRaceToStr(race), ShipTypeToStr(type));
#endif
        strcpy(directory,"DefaultShip\\");
        strcpy(shipname,"DefaultShip.shp");

        strcpy(fullshipname,directory);
        strcat(fullshipname,shipname);
        stubbedOut = TRUE;
*/
    }
    else
    {
        stubbedOut = FALSE;
    }

#if SO_CLOOGE_SCALE
    statinfo->scaleFactor = 1.0f;
#endif

    statinfo->turnspeedstat[TURN_YAW]   = 1.0f;
    statinfo->turnspeedstat[TURN_PITCH] = 1.0f;
    statinfo->turnspeedstat[TURN_ROLL]  = 1.0f;

    statinfo->minimumZoomDistance = 0.0f;

    statinfo->forceFieldProtectionDistanceScale = 1.0f;
    statinfo->upgradeTimeModifier = 1.0f;

    statinfo->renderlistFade = 0.0f;
    statinfo->renderlistLimitSqr = 0.0f;

    statinfo->dockLightNear = 0.0f;
    statinfo->dockLightFar  = 0.1f;
    statinfo->dockLightColor = colRGB(120, 194, 255);   //default blue glow

    statinfo->hyperspaceColor = colRGB(0,0,0);  //default is to use the effect's default
    statinfo->hyperspaceBoxScale = 1.0f;
    statinfo->beastAttackCascadeCost = 1.0f;
    statinfo->beastAttacksRequired = (udword)-1;
    statinfo->beastAttackConvertTo = bWorker;

    statinfo->tacticalSelectionScale = 1.0f;  // This is used to scale the R1 probes tactiacl overlay size, and the defenders selection box size

    if (isStrikeCraft(type))
        statinfo->formationPaddingModifier = STRIKECRAFT_PADDING_MODIFIER;          // default formation padding modifier
    else
        statinfo->formationPaddingModifier = 1.0f;          // default formation padding modifier

    //default to no explosions at all
//    statinfo->smallExplosion = statinfo->bigExplosion = statinfo->specialExplosion = NULL;
//    statinfo->bigExplosionFactor = 1.0f;

    //engine trail defaults
    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        statinfo->trailStyle[i] = 1;
        statinfo->trailWidth[i] = -1.0f;
        statinfo->trailHeight[i] = -1.0f;
        statinfo->trailAngle[i] = -1.0f;
        statinfo->trailRibbonAdjust[i] = -1.0f;
        statinfo->trailLength[i] = -1.0f;
        statinfo->trailScaleCap[i] = -1.0f;
        statinfo->trailSpriteRadius[i] = -1.0f;
        statinfo->trailSpriteOffset[i] = 0.0f;
    }
    statinfo->staticheader.staticCollInfo.preciseSelection = 0xff;//default is no precise selection

    // collSideModifiers reset
    for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
    {
        statinfo->collSideModifiers[i] = 0.0f;
    }

    statinfo->staticheader.rightOfWay = rightOfWays[type];      // defaults for right of way

    scriptSetStruct(directory,shipname,ShipStaticScriptTable,(ubyte *)statinfo);

    if ((statinfo->repairTime) && (statinfo->repairCombatTime == 0.0f))
    {
        statinfo->repairCombatTime = statinfo->repairTime;
    }

    if ((statinfo->repairDamage) && (statinfo->repairCombatDamage == 0.0f))
    {
        statinfo->repairCombatDamage = statinfo->repairDamage;
    }

    InitStatShipTypeFlags(statinfo);

    if (statinfo->collSideModifiers[0] == 0.0f)
    {
        // collSideModifiers haven't been explicitly set, so set them:
        if (isCapitalShipStatic(statinfo))
        {
            for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
            {
                statinfo->collSideModifiers[i] = capitalSideModifiers[i];
            }
        }
        else
        {
            for (i=0;i<NUM_TRANS_DEGOFFREEDOM;i++)
            {
                statinfo->collSideModifiers[i] = 1.0f;
            }
        }
    }
/*
#ifndef HW_Release
    if ((cheapShips) && (!multiPlayerGame))
    {
        statinfo->buildTime = 1;
        statinfo->buildCost = 1;
    }
#endif
*/
    if (statinfo->blastRadiusShockWave == 0.0f)
    {
        statinfo->blastRadiusShockWave = BLAST_RADIUS_MULTIPLE_DEFAULT;
    }



    if (statinfo->passiveRetaliateZone == 0.0f)
    {
        statinfo->passiveRetaliateZone = RETALIATE_ZONE;
    }
    else if (statinfo->passiveRetaliateZone < 0.0f)
    {
        statinfo->passiveRetaliateZone = 0.0f;       // turn of retaliation zone feature if -ve
    }

    if (statinfo->attackfuelburnrate == 0.0f)
    {
        statinfo->attackfuelburnrate = statinfo->fuelburnrate;
    }

    statinfo->canReceiveSomething = (bool8)(statinfo->canReceiveResources | statinfo->canReceiveShips | statinfo->canReceiveShipsPermanently | statinfo->canReceiveShipsForRetire);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        statinfo->trailStatic[i] = trailStaticInfoParse(directory, shipname);
    }
    if (mexGetChunk(statinfo->staticheader.pMexData, "Eng", "Eng1") != NULL)
    {
        statinfo->multipleEngines = (udword)TRUE;
    }
    else
    {
        statinfo->multipleEngines = FALSE;
    }

    statinfo->lowfuelpoint *= statinfo->maxfuel;

    precalcStaticHeader(&statinfo->staticheader);
#if 1  // comment out when MiningBase properly moved hack,  fix later
    if (type == MiningBase)
    {
        StaticCollInfo *staticCollInfo = &statinfo->staticheader.staticCollInfo;
        staticCollInfo->collsphereoffset.x += MINING_BASE_VERTICAL_DIST_HACK;
        staticCollInfo->collrectoffset.x += MINING_BASE_VERTICAL_DIST_HACK;
    }
#endif
    precalcStaticInfoHealth((StaticInfoHealth *)statinfo);
    precalcStaticInfoGuidance((StaticInfoHealthGuidance *)statinfo);

    aishipPrecalcInfo(statinfo);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        mexSetEngineNozzle(statinfo, statinfo->staticheader.pMexData, i);
    }

    if (stubbedOut == FALSE)
    {
        scriptSetDockStatics(directory,shipname,statinfo);
        scriptSetDockOverideStatics(directory,shipname,statinfo);
    }

    if (statinfo->dockStaticInfo != NULL)
    {
        mexGetDockStaticInfo(statinfo->dockStaticInfo,statinfo->staticheader.pMexData);
        shipStaticCacheCenterDockPoint(statinfo->dockStaticInfo);
        shipStaticApplyDockStorageFlags(statinfo);
    }

    if (stubbedOut == FALSE)
    {
        // dockInitializeCustomFunctions was removed in Cataclysm; dock init is now handled
        // by ship-type-specific code in the 0x5C0000 range.
    }

    if (ShipCanHarvest(statinfo))
    {
        // find resource collection nozzle
        statinfo->resNozzleStatic = mexGetResNozzleStatic(statinfo->staticheader.pMexData);
    }

    if(statinfo->repairBeamCapable)
    {
        //find repair beam nozzle
        statinfo->repairNozzleStatic = mexGetRepairNozzleStatic(statinfo->staticheader.pMexData);
    }

    if(ShipTypeIsSalCapCorvetteType(type))
    {
        //find tractor beam emmitter
        statinfo->tractorEmitterStatic = mexGetTractorBeamStatic(statinfo->staticheader.pMexData);
    }

    scriptSetNAVLightStatics(directory, shipname, statinfo);
    if (statinfo->navlightStaticInfo != NULL)
    {
        MEXNAVLightContext navLightContext;

        navLightContext.lodInfo = statinfo->staticheader.LOD;
        navLightContext.rawMexData = statinfo->staticheader.pMexData;
        mexGetNAVLightStaticInfo(statinfo->navlightStaticInfo, &navLightContext);
    }

    scriptSetGunStatics(directory,shipname,statinfo);
    if (statinfo->gunStaticInfo != NULL)
    {
        GunStaticInfo *gunstaticinfo = statinfo->gunStaticInfo;
        for (i=0;i<gunstaticinfo->numGuns;i++)
        {
            gunstaticinfo->gunstatics[i].gunindex = i;
        }
        mexGetGunStaticInfo(gunstaticinfo,statinfo->staticheader.pMexData);
    }

    if (statinfo->bulletRange[0] != 0.0f)
    {
        for(i=0;i<NUM_TACTICS_TYPES;i++)
        {
            statinfo->bulletRangeSquared[i] = statinfo->bulletRange[i] * statinfo->bulletRange[i];
        }
    }


    //load in the hierarchy binding information
    scriptSetStruct(directory,shipname,HierarchyBindingTable,(ubyte *)statinfo);

    //load in More Mesh Animation info
    //must be AFTER we've loaded in all other MAD stuff, otherwise
    //I'll get MAD!
    if(statinfo->madStatic != NULL)
    {
        //default these to 0
        statinfo->madStatic->needStartPlacedAnimation = 0;
        statinfo->madStatic->needStartBuiltAnimation = 0;
        statinfo->madStatic->numGunOpenIndexes=0;
        statinfo->madStatic->numGunOpenDamagedIndexes=0;
        statinfo->madStatic->numGunCloseIndexes=0;
        statinfo->madStatic->numGunCloseDamagedIndexes=0;
        statinfo->madStatic->numDockIndexes=0;                   //number of below indices
        statinfo->madStatic->numDockDamagedIndexes=0;                   //number of below indices
        statinfo->madStatic->numPostDockIndexes=0;                   //number of below indices
        statinfo->madStatic->numPostDockDamagedIndexes=0;                   //number of below indices
        statinfo->madStatic->numDoorOpenIndexes=0;
        statinfo->madStatic->numDoorCloseIndexes=0;
        statinfo->madStatic->numSpecialOpenIndexes=0;
        statinfo->madStatic->numSpecialCloseIndexes=0;
        statinfo->madStatic->numSpecialOpenDamagedIndexes=0;
        statinfo->madStatic->numSpecialCloseDamagedIndexes=0;

    }

    scriptSetStruct(directory,shipname,MadMaxMadInfoLoad,(ubyte *)statinfo);

    if (statinfo->gunStaticInfo != NULL && stubbedOut == FALSE)
    {
        gunSetStaticGunInfoFromBindings(statinfo);
    }

    if (statinfo->custshipheader.CustShipStaticInit != NULL)
    {
        statinfo->custshipheader.CustShipStaticInit(directory,shipname,statinfo);
    }
}

/*-----------------------------------------------------------------------------
    Name        : InitStatShipInfoPost
    Description : Initializes static ship information for all ship types. Post operation
                  uses information from previous pass to complete the initialization.
    Inputs      : type, race
    Outputs     : statinfo is filled in
    Return      :
----------------------------------------------------------------------------*/
void InitStatShipInfoPost(ShipStaticInfo *statinfo)
{
    if (GLOBAL_SHIP_HEALTH_MODIFIER != 1.0f)
    {
        statinfo->maxhealth = ((statinfo->maxhealth) * GLOBAL_SHIP_HEALTH_MODIFIER);
        if (statinfo->maxhealth == 0.0f)
        {
            statinfo->maxhealth = 1.0f;
        }
    }

    statinfo->unk10 = TreatAsUdword(statinfo->maxhealth);
    statinfo->oneOverMaxHealth = 1.0f / statinfo->maxhealth;

    if (statinfo->custshipheader.CustShipStaticInitPost != NULL)
    {
        statinfo->custshipheader.CustShipStaticInitPost(statinfo);
    }

    if (!singlePlayerGame || statinfo->shiprace != RACE_Sect)
    {
        statinfo->maxhealth = TreatAsReal32(statinfo->unk10);
    }
    else
    {
        statinfo->maxhealth = GameDifficultyPrimaryModifier * TreatAsReal32(statinfo->unk10);
    }

    statinfo->oneOverMaxHealth = 1.0f / statinfo->maxhealth;

}

/*-----------------------------------------------------------------------------
    Name        : CloseStatInfo
    Description : Close base static info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatInfo(StaticInfo *statinfo)
{
    if ((statinfo->staticheader.pMexData != defaultmex) && (statinfo->staticheader.pMexData != NULL))
    {
        mexFree(statinfo->staticheader.pMexData);
        statinfo->staticheader.pMexData = NULL;
    }

    if ((statinfo->staticheader.LOD != defaultlod) && (statinfo->staticheader.LOD != NULL))
    {
        lodFree(statinfo->staticheader.LOD);
        statinfo->staticheader.LOD = NULL;
    }
    //statinfo->staticheader.infoLoaded = statinfo->staticheader.infoNeeded = FALSE;
    statinfo->staticheader.infoFlags = 0;
    memset(statinfo, 0, sizeof(StaticInfo));
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatShipInfo
    Description : Closes static ship info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatShipInfo(ShipStaticInfo *statinfo)
{
    sdword i;

    CloseStatInfo((StaticInfo *)statinfo);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (statinfo->trailStatic[i] != NULL)
        {
            trailStaticDelete(statinfo->trailStatic[i]);
            statinfo->trailStatic[i] = NULL;
        }
    }

    if (statinfo->madStatic != NULL)
    {
        if(statinfo->madStatic->gunOpenIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunOpenIndexes);
            statinfo->madStatic->gunOpenIndexes = NULL;
        }
        if(statinfo->madStatic->gunCloseIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunCloseIndexes);
            statinfo->madStatic->gunCloseIndexes = NULL;
        }
        if(statinfo->madStatic->gunOpenDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunOpenDamagedIndexes);
            statinfo->madStatic->gunOpenDamagedIndexes = NULL;
        }
        if(statinfo->madStatic->gunCloseDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->gunCloseDamagedIndexes);
            statinfo->madStatic->gunCloseDamagedIndexes = NULL;
        }
        //dock anims
        if(statinfo->madStatic->PostDockIndexes != NULL)
        {
            memFree(statinfo->madStatic->PostDockIndexes);
            statinfo->madStatic->PostDockIndexes = NULL;
        }
        if(statinfo->madStatic->DockIndexes != NULL)
        {
            memFree(statinfo->madStatic->DockIndexes);
            statinfo->madStatic->DockIndexes = NULL;
        }
        if(statinfo->madStatic->PostDockDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->PostDockDamagedIndexes);
            statinfo->madStatic->PostDockDamagedIndexes = NULL;
        }
        if(statinfo->madStatic->DockDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->DockDamagedIndexes);
            statinfo->madStatic->DockDamagedIndexes = NULL;
        }
        //mothership door anims
        if(statinfo->madStatic->DoorOpenIndexes != NULL)
        {
            memFree(statinfo->madStatic->DoorOpenIndexes);
            statinfo->madStatic->DoorOpenIndexes = NULL;
        }
        if(statinfo->madStatic->DoorCloseIndexes != NULL)
        {
            memFree(statinfo->madStatic->DoorCloseIndexes);
            statinfo->madStatic->DoorCloseIndexes = NULL;
        }
        //special case
        if(statinfo->madStatic->specialOpenIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialOpenIndexes);
            statinfo->madStatic->specialOpenIndexes = NULL;
        }
        if(statinfo->madStatic->specialCloseIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialCloseIndexes);
            statinfo->madStatic->specialCloseIndexes = NULL;
        }
        if(statinfo->madStatic->specialOpenDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialOpenDamagedIndexes);
            statinfo->madStatic->specialOpenDamagedIndexes = NULL;
        }
        if(statinfo->madStatic->specialCloseDamagedIndexes != NULL)
        {
            memFree(statinfo->madStatic->specialCloseDamagedIndexes);
            statinfo->madStatic->specialCloseDamagedIndexes = NULL;
        }
        if (statinfo->madStatic->animIndexRemap != NULL)
        {
            memFree(statinfo->madStatic->animIndexRemap);
            statinfo->madStatic->animIndexRemap = NULL;
        }


        madHeaderDelete(statinfo->madStatic->header);
        //... other mesh binding freeing stuffs
        memFree(statinfo->madStatic);
        statinfo->madStatic = NULL;

    }
    if (statinfo->dockOverideInfo != NULL)
    {
        memFree(statinfo->dockOverideInfo);
        statinfo->dockOverideInfo = NULL;
    }
    if (statinfo->resNozzleStatic != NULL)
    {
        memFree(statinfo->resNozzleStatic);
        statinfo->resNozzleStatic = NULL;
    }
    if(statinfo->repairNozzleStatic != NULL)
    {
        memFree(statinfo->repairNozzleStatic);
        statinfo->repairNozzleStatic = NULL;
    }
    if (statinfo->dockStaticInfo != NULL)
    {
        memFree(statinfo->dockStaticInfo);
        statinfo->dockStaticInfo = NULL;
    }

    if (statinfo->navlightStaticInfo != NULL)
    {
        navLightStaticInfoDelete(statinfo->navlightStaticInfo);
        statinfo->navlightStaticInfo = NULL;
    }

    if (statinfo->gunStaticInfo != NULL)
    {
        memFree(statinfo->gunStaticInfo);
        statinfo->gunStaticInfo = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatAsteroidInfo
    Description : Closes static asteroid info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatAsteroidInfo(AsteroidStaticInfo *asteroidStatInfo)
{
    CloseStatInfo((StaticInfo *)asteroidStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatDustCloudInfo
    Description : Closes static dustcloud info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatDustCloudInfo(DustCloudStaticInfo *dustcloudStatInfo)
{
    CloseStatInfo((StaticInfo *)dustcloudStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatGasCloudInfo
    Description : Closes static gascloud info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatGasCloudInfo(GasCloudStaticInfo *gascloudStatInfo)
{
    CloseStatInfo((StaticInfo *)gascloudStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatNebulaInfo
    Description : Closes static nebula info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatNebulaInfo(NebulaStaticInfo *nebulaStatInfo)
{
//    CloseStatInfo((StaticInfo *)nebulaStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatDerelictInfo
    Description : Closes static derelict info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatDerelictInfo(DerelictStaticInfo *derelictStatInfo)
{
    if (derelictStatInfo->salvageStaticInfo != NULL)
    {
        memFree(derelictStatInfo->salvageStaticInfo);
        derelictStatInfo->salvageStaticInfo = NULL;
    }
    CloseStatInfo((StaticInfo *)derelictStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : CloseStatMissileInfo
    Description : Closes static missile info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatMissileInfo(MissileStaticInfo *missileStatInfo)
{
    CloseStatInfo((StaticInfo *)missileStatInfo);
}
/*-----------------------------------------------------------------------------
    Name        : CloseStatMineInfo
    Description : Closes static mine info
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CloseStatMineInfo(MissileStaticInfo *mineStatInfo)
{
    CloseStatInfo((StaticInfo *)mineStatInfo);
}

void CloseStatEffectInfo(etgeffectstatic *effectStatInfo)
{
//    CloseStatInfo((EffectStaticInfo *)effectStatInfo);
}

/*-----------------------------------------------------------------------------
    Name        : universeInfoNeededSetCB
    Description : Static info scan callback to flag a static info as needed or
                    not needed.
    Inputs      : info - static info to look at
                  objType - type of object to work with
    Outputs     : modifies the IF_InfoNeeded bit of infoFlags to bNeededParam
    Return      : void
----------------------------------------------------------------------------*/
static sdword bNeededParam;
void universeInfoNeededSetCB(StaticInfo *info, ObjType objType)
{
    bitSetTo(info->staticheader.infoFlags, IF_InfoNeeded, bNeededParam);
}

/*=============================================================================
    Public Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : universeFlagRaceNeeded
    Description : Marks an entire race of staticinfo's as needed or not
    Inputs      : shiprace - what race to mark
                  bNeeded - TRUE if we need this static info loaded this level
    Outputs     :
    Return      : void
    Note        : Missiles/mines will be turned on/off with the associated ships.
----------------------------------------------------------------------------*/
void universeFlagRaceNeeded(ShipRace shiprace, bool8 bNeeded)
{
    bNeededParam = bNeeded;
    universeRaceStaticsScan(shiprace, universeInfoNeededSetCB, universeInfoNeededSetCB);
}

/*-----------------------------------------------------------------------------
    Name        : universeDefaultTeamColors
    Description : Sets the defualts for what colors the ships can be in -
                    whatever players are that race.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeDefaultTeamColors(void)
{
    bool8 racePlayerTable[NUM_RACES][MAX_MULTIPLAYER_PLAYERS];
    sdword index;
    ShipRace shiprace;
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    DerelictType derelicttype;
    ShipStaticInfo *shipstaticinfo;
    MissileStaticInfo *missileInfo;

    //make a boolean table of color schemes by race
    memset(racePlayerTable, 0, sizeof(racePlayerTable));
    for (index = 0; index < numPlayers; index++)
    {
        racePlayerTable[universe.players[index].race][index] = TRUE;
    }
    //!!! somewhat oversimple assignment of default color schemes for the non-player races
    racePlayerTable[Traders][7] = TRUE;
    racePlayerTable[RACE_Kushan][6] = TRUE;
    racePlayerTable[RACE_Taiidan][5] = TRUE;
    racePlayerTable[P1][4] = TRUE;
    //now plug that default table into all the ship static infos
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
            memcpy(shipstaticinfo->teamColor, racePlayerTable[shiprace], sizeof(shipstaticinfo->teamColor));
        }
        missileInfo = &missileStaticInfos[shiprace];
        memcpy(missileInfo->teamColor, racePlayerTable[shiprace], sizeof(missileInfo->teamColor));
        if (shiprace <= RACE_Beast)
        {
            missileInfo = &mineStaticInfos[shiprace];
            memcpy(missileInfo->teamColor, racePlayerTable[shiprace], sizeof(missileInfo->teamColor));
        }
    }
    //plug default team colors into derelicts (last team color scheme, like traders)
    for (derelicttype = 0; derelicttype < NUM_DERELICTTYPES; derelicttype++)
    {
        memcpy(derelictStaticInfos[derelicttype].teamColor, racePlayerTable[Traders], sizeof(racePlayerTable[Traders]));
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeFlagEverythingNeeded
    Description : Flags all static infos to be reloaded during the next level
    Inputs      : void
    Outputs     : sets the infoNeeded flag true for all static infos
    Return      : void
----------------------------------------------------------------------------*/
void universeFlagEverythingNeeded(void)
{
    bNeededParam = TRUE;
    universeAllStaticsScan(universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB);
}

/*-----------------------------------------------------------------------------
    Name        : universeRaceStaticsScan
    Description : Scans all static infos needed for a race and calls appropriate
                    callbacks.
    Inputs      : shiprace - what race to scan
                  forShips - called for ship static infos
                  forMisc - called for other statics, like mines and missiles
    Outputs     :
    Return      : void
    Note        : Missiles/mines will be turned on/off with the associated ships.
----------------------------------------------------------------------------*/
void universeRaceStaticsScan(ShipRace shiprace, univstatcallback forShips, univstatcallback forMisc)
{
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    ShipStaticInfo *shipstaticinfo;

    firstshiptype = FirstShipTypeOfRace[shiprace];
    lastshiptype = LastShipTypeOfRace[shiprace];
    for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
    {
        shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
        if (forShips)
        {
            forShips((StaticInfo *)shipstaticinfo, OBJ_ShipType);
        }
        if (forMisc)
        {
            if (ShipTypeIsHomeworldMissileDestroyerType(shiptype) ||
                shiptype == sDreadnought ||
                shiptype == P1MissileCorvette ||
                shiptype == sDestroyer)
            {
                forMisc((StaticInfo *)&missileStaticInfos[shiprace], OBJ_MissileType);
            }
            else if (shiprace < 2 && shiptype == tMinelayerCorvette)
            {
                forMisc((StaticInfo *)&mineStaticInfos[shiprace], OBJ_MissileType);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeAllStaticsScan
    Description : Scans through the static info structures for all spaceobj
                    types and calls a callback with that static info.
    Inputs      : forShips - called for ship static infos
                  forDerelicts, forResources, forMisc - same, but for derelicts,
                  resources and whatnot.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeAllStaticsScan(univstatcallback forShips, univstatcallback forDerelicts, univstatcallback forResources, univstatcallback forMisc)
{
    ShipRace shiprace;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        universeRaceStaticsScan(shiprace, forShips, forMisc);
    }
    if (forResources != NULL)
    {
        for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
        {
            forResources((StaticInfo *)&asteroidStaticInfos[asteroidtype], OBJ_AsteroidType);
        }
        for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
        {
            forResources((StaticInfo *)&dustcloudStaticInfos[dustcloudtype], OBJ_DustType);
        }
        for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
        {
            forResources((StaticInfo *)&gascloudStaticInfos[gascloudtype], OBJ_GasType);
        }
        for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
        {
            forResources((StaticInfo *)&nebulaStaticInfos[nebulatype], OBJ_NebulaType);
        }
    }
    if (forDerelicts != NULL)
    {
        for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
        {
            forDerelicts((StaticInfo *)&derelictStaticInfos[derelicttype], OBJ_DerelictType);
        }
    }
    if (forMisc != NULL)
    {
        for (shiprace=0;shiprace<NUM_RACES;shiprace++)
        {
            forMisc((StaticInfo *)&missileStaticInfos[shiprace], OBJ_MissileType);
        }
/*
    for (shiprace=0;shiprace<2;shiprace++)
    {
        bitSet(mineStaticInfos[shiprace].staticheader.infoFlags, IF_InfoNeeded);
    }
*/
    }
}

/*-----------------------------------------------------------------------------
    Name        : universeFlagEverythingNeeded
    Description : Flags all static infos to be reloaded during the next level
    Inputs      : void
    Outputs     : sets the infoNeeded flag FALSE for all static infos
    Return      : void
----------------------------------------------------------------------------*/
void universeFlagNothingNeeded(void)
{
    bNeededParam = FALSE;
    universeAllStaticsScan(universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB, universeInfoNeededSetCB);
}

ubyte bitpackTeamColor(bool8 teamColor[])
{
    sdword i;
    ubyte bitmask = 0;

    for (i=0;i<8;i++)
    {
        bitmask <<= 1;
        if (teamColor[i])
        {
            bitmask |= 1;
        }
    }

    return bitmask;
}

void unbitpackTeamColor(bool8 teamColor[],ubyte pack)
{
    sdword i;

    for (i=0;i<8;i++)
    {
        if (pack & 128)
        {
            teamColor[i] = TRUE;
        }
        else
        {
            teamColor[i] = FALSE;
        }
        pack <<= 1;
    }
}

#define INFO_NEEDED_FLAGS_DELIMITER         0xf1a5deed

void universeSaveEverythingNeeded(void)
{
    ShipRace shiprace;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;

    ShipType shiptype;
    ShipStaticInfo *shipstatic;

    ubyte data[4];

    SaveInfoNumber(INFO_NEEDED_FLAGS_DELIMITER);

    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        shipstatic = GetShipStaticInfo(shiptype, GetValidRaceForShipType(shiptype));
        data[0] = bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded);      // anything loaded is needed
        data[1] = bitpackTeamColor(shipstatic->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        SaveInfoNumber(bitTest(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        SaveInfoNumber(bitTest(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        SaveInfoNumber(bitTest(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        SaveInfoNumber(bitTest(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoLoaded));
    }
    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        DerelictStaticInfo *dstaticinfo = &derelictStaticInfos[derelicttype];
        data[0] = bitTest(dstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        data[1] = bitpackTeamColor(dstaticinfo->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }
    for (shiprace = 0; shiprace < NUM_RACES; shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &missileStaticInfos[shiprace];
        data[0] = bitTest(misstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        data[1] = bitpackTeamColor(misstaticinfo->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &mineStaticInfos[shiprace];
        data[0] = bitTest(misstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
        data[1] = bitpackTeamColor(misstaticinfo->teamColor);
        data[2] = 0;
        data[3] = 0;
        SaveInfoNumber(TreatAsUdword(data[0]));
    }

    SaveInfoNumber(INFO_NEEDED_FLAGS_DELIMITER);
}

void universeLoadEverythingNeeded(void)
{
    ShipRace shiprace;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;

    ShipType shiptype;
    ShipStaticInfo *shipstatic;

    udword data;
    ubyte *datab;

    datab = (ubyte *)&data;

    data = LoadInfoNumber();
    if (data != INFO_NEEDED_FLAGS_DELIMITER)
    {
        dbgMessagef("universeLoadEverythingNeeded: delimiter mismatch (got 0x%X)", data);
    }
    for (shiprace = 0; shiprace < NUM_RACES; shiprace++)
    {
        for (shiptype = FirstShipTypeOfRace[shiprace];
             shiptype <= LastShipTypeOfRace[shiprace];
             shiptype++)
        {
            shipstatic = GetShipStaticInfo(shiptype, shiprace);
            data = LoadInfoNumber();
            bitSetTo(shipstatic->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
            rmEnableShip(shiprace, shiptype, datab[0]);
            unbitpackTeamColor(shipstatic->teamColor, datab[1]);
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        bitSetTo(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        bitSetTo(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        bitSetTo(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        bitSetTo(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoNeeded, LoadInfoNumber());
    }
    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        DerelictStaticInfo *dstaticinfo = &derelictStaticInfos[derelicttype];
        data = LoadInfoNumber();
        bitSetTo(dstaticinfo->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
        unbitpackTeamColor(dstaticinfo->teamColor,datab[1]);
    }
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &missileStaticInfos[shiprace];
        data = LoadInfoNumber();
        bitSetTo(misstaticinfo->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
        unbitpackTeamColor(misstaticinfo->teamColor,datab[1]);
    }
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        MissileStaticInfo *misstaticinfo = &mineStaticInfos[shiprace];
        data = LoadInfoNumber();
        bitSetTo(misstaticinfo->staticheader.infoFlags, IF_InfoNeeded, datab[0]);
        unbitpackTeamColor(misstaticinfo->teamColor,datab[1]);
    }

    data = LoadInfoNumber();
    dbgAssert(data == INFO_NEEDED_FLAGS_DELIMITER);
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticInit
    Description : Initializes static data for the Universe.  Only the ships,
                    resources, missiles etc. that are needed will be loaded.
                    Anything that was previously loaded and is no longer
                    needed will be deleted.
    Inputs      : none
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
extern void *utyTeaserHeader;
void universeStaticInit(void)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;
    ShipStaticInfo *shipstaticinfo;
    StaticInfo *staticinfo;
    udword max,count;
#if UNIV_SHIP_LOADFREE_LOG
    FILE *logFile = NULL;

    if (univLoadFreeLog)
    {
        logFile = fopen(UNIV_LOAD_FREE_FILENAME, "at");
    }
    if (logFile)
    {
        if (singlePlayerGame)
        {
            fprintf(logFile, "\n-----------\nCurrent level = %d\n", singlePlayerGameInfo.currentMission);
        }
    }
#endif

    //load some things shared by everyone
    if (sphereStaticInfo == NULL)
    {
        sphereStaticInfo = createSphereStaticInfo();
    }
    if (defaultmesh == NULL)
    {
        defaultmesh = meshLoad("DefaultShip\\DefaultShip.geo");
    }
    if (defaultlod == NULL)
    {
        defaultlod = lodTableReadScript("DefaultShip\\", "DefaultShip.lod");
    }
    if (defaultmex == NULL)
    {
        defaultmex = mexLoad("DefaultShip\\DefaultShip.mex");
    }

    max = 0;
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        max += (LastShipTypeOfRace[shiprace] - FirstShipTypeOfRace[shiprace]);
    }
    max += NUM_ASTEROIDTYPES + NUM_DUSTCLOUDTYPES + NUM_NEBULATYPES + NUM_DERELICTTYPES + NUM_RACES + 2;

    HorseRaceBeginBar(UNIVERSE_BAR);
    count = 0;
    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            HorseRaceNext(((real32)count)/((real32)max));
            if (hrAbortLoadingGame)
            {
                goto abortloadinggame;
            }
            count++;
            shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
            if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded))
            {                                               //if this info needed
                if (!bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
                {                                           //if this info not loaded
                    InitStatShipInfo(shipstaticinfo,shiptype,shiprace);
                    bitSet(shipstaticinfo->staticheader.infoFlags, IF_NotPostLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                    if (logFile)
                    {
                        fprintf(logFile, "Loading ship: %8s %s\n", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
                    }
#endif
                }
                else
                {
                    if (shipstaticinfo->staticheader.LOD != NULL)
                    {
                        lodAllMeshesRecolorize(shipstaticinfo->staticheader.LOD);
                    }
                }
            }
            else
            {                                               //static info not needed this round
                if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded))
                {                                           //if it's been loaded
                    CloseStatShipInfo(shipstaticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                    if (logFile)
                    {
                        fprintf(logFile, "Freeing ship: %8s %s\n", ShipRaceToStr(shiprace), ShipTypeToStr(shiptype));
                    }
#endif
                }
            }
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&asteroidStaticInfos[asteroidtype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatAsteroidInfo((AsteroidStaticInfo *)staticinfo,asteroidtype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading asteroid: %s\n", AsteroidTypeToStr(asteroidtype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatAsteroidInfo((AsteroidStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing asteroid: %s\n", AsteroidTypeToStr(asteroidtype));
                }
#endif
            }
        }
        count++;
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&dustcloudStaticInfos[dustcloudtype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatDustCloudInfo((DustCloudStaticInfo *)staticinfo, dustcloudtype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading ship: %s\n", DustCloudTypeToStr(dustcloudtype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatDustCloudInfo((DustCloudStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing dustcloud: %s\n", DustCloudTypeToStr(dustcloudtype));
                }
#endif
            }
        }
        count++;
    }

#if 0
    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        staticinfo = (StaticInfo *)&gascloudStaticInfos[gascloudtype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatGasCloudInfo((GasCloudStaticInfo *)staticinfo, gascloudtype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading gas cloud: %s\n", GasCloudTypeToStr(gascloudtype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatGasCloudInfo((GasCloudStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing gascloud: %s\n", GasCloudTypeToStr(gascloudtype));
                }
#endif
            }
        }
    }
#endif
    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&nebulaStaticInfos[nebulatype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatNebulaInfo((NebulaStaticInfo *)staticinfo, nebulatype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading nebula: %s\n", NebulaTypeToStr(nebulatype));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatNebulaInfo((NebulaStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing nebula: %s\n", NebulaTypeToStr(nebulatype));
                }
#endif
            }
        }
        count++;
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&derelictStaticInfos[derelicttype];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatDerelictInfo((DerelictStaticInfo *)staticinfo,derelicttype);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading derelict: %s\n", DerelictTypeToStr(derelicttype));
                }
#endif
            }
            else
            {
                if (staticinfo->staticheader.LOD != NULL)
                {
                    lodAllMeshesRecolorize(staticinfo->staticheader.LOD);
                }
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatDerelictInfo((DerelictStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing derelict: %s\n", DerelictTypeToStr(derelicttype));
                }
#endif
            }
        }
        count++;
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&missileStaticInfos[shiprace];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatMissileInfo((MissileStaticInfo *)staticinfo,shiprace);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading missile: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatMissileInfo((MissileStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing missile: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        count++;
    }

    maxMineCollSphereSize = 0.0f;
    for (shiprace=0;shiprace<2;shiprace++)
    {
        HorseRaceNext(((real32)count)/((real32)max));
        if (hrAbortLoadingGame)
        {
            goto abortloadinggame;
        }
        staticinfo = (StaticInfo *)&mineStaticInfos[shiprace];
        if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoNeeded))
        {                                                   //if info needed
            if (!bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info needed and not loaded
                InitStatMineInfo((MissileStaticInfo *)staticinfo,shiprace);
                bitSet(staticinfo->staticheader.infoFlags, IF_InfoLoaded);
                if (mineStaticInfos[shiprace].staticheader.staticCollInfo.collspheresize > maxMineCollSphereSize)
                {
                    maxMineCollSphereSize = mineStaticInfos[shiprace].staticheader.staticCollInfo.collspheresize;
                }
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Loading mine: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        else
        {                                                   //info not needed
            if (bitTest(staticinfo->staticheader.infoFlags, IF_InfoLoaded))
            {                                               //info loaded
                CloseStatMineInfo((MissileStaticInfo *)staticinfo);
#if UNIV_SHIP_LOADFREE_LOG
                if (logFile)
                {
                    fprintf(logFile, "Freeing mine: %8s\n", ShipRaceToStr(shiprace));
                }
#endif
            }
        }
        count++;
    }

    maxMineCollSphereSize *= 2.0f;

//    InitStatEffectInfo(&effectStaticInfo);

abortloadinggame:

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
            if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded))
            {
                if (bitTest(shipstaticinfo->staticheader.infoFlags, IF_NotPostLoaded))
                {                                           //if ship just now loaded
                    InitStatShipInfoPost(shipstaticinfo);
                    //shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded) = TRUE;
                    bitClear(shipstaticinfo->staticheader.infoFlags, IF_NotPostLoaded);
                    bitSet(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded);
                }
            }
        }
    }


#if UNIV_SHIP_LOADFREE_LOG
    if (logFile)
    {
        fclose(logFile);
    }
#endif

}

/*-----------------------------------------------------------------------------
    Name        : universeStaticInit
    Description : Reinitializes some static data:
                                                    *.Shp variables
    Inputs      : none
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void universeRealTimeTweak(SelectCommand *selection)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipStaticInfo *shipstaticinfo;
    udword shiparray[100];
    udword shiptypedone;
    udword shipdoneptr=0;
    sdword index;
    sdword flag = FALSE;

    for(shiptypedone = 0;shiptypedone < 100; shiptypedone++)
    {
        shiparray[shiptypedone] = 99999;    //fill with garbage ships
    }
    for (index = 0;index < selection->numShips;index++)
    {
        shiprace = selection->ShipPtr[index]->staticinfo->shiprace;
        shiptype = selection->ShipPtr[index]->shiptype;

        for(shiptypedone = 0;shiptypedone <=shipdoneptr; shiptypedone++)
        {
            if(shiparray[shiptypedone] == shiptype)
            {
                //shiptype already loaded, so don't load it
                flag = TRUE;
                break;
            }
        }
        if(flag == TRUE)
        {
            flag = FALSE;
            continue;
        }

        shiparray[shipdoneptr] = shiptype;
        shipdoneptr++;
        shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);
        InitStatShipInfo(shipstaticinfo,shiptype,shiprace);
    }
}
void universeSetZeroPlayers(void)
{
    universe.numPlayers = 0;
    cameraInit(&defaultCamera,5000.0f);
}

/*-----------------------------------------------------------------------------
    Name        : universeNoStaticLoadedOrNeeded
    Description : Marks all static static infos as not loaded and not needed
                  in fact, it sets the entire static infos to NULL
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeNoStaticLoadedOrNeeded(void)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipStaticInfo *shipStaticInfo;
    AsteroidStaticInfo *asteroidStaticInfo;
    DustCloudStaticInfo *dustcloudStaticInfo;
    GasCloudStaticInfo *gascloudStaticInfo;
    NebulaStaticInfo *nebulaStaticInfo;
    DerelictStaticInfo *derelictStaticInfo;
    MissileStaticInfo *missileStaticInfo;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    NebulaType nebulatype;
    DerelictType derelicttype;
    ShipType firstshiptype;
    ShipType lastshiptype;

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            shipStaticInfo = GetShipStaticInfo(shiptype,shiprace);
            memset(shipStaticInfo,0,sizeof(ShipStaticInfo));
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        asteroidStaticInfo = &asteroidStaticInfos[asteroidtype];
        memset(asteroidStaticInfo,0,sizeof(*asteroidStaticInfo));
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        dustcloudStaticInfo = &dustcloudStaticInfos[dustcloudtype];
        memset(dustcloudStaticInfo,0,sizeof(*dustcloudStaticInfo));
    }

    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        gascloudStaticInfo = &gascloudStaticInfos[gascloudtype];
        memset(gascloudStaticInfo,0,sizeof(*gascloudStaticInfo));
    }

    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        nebulaStaticInfo = &nebulaStaticInfos[nebulatype];
        memset(nebulaStaticInfo,0,sizeof(*nebulaStaticInfo));
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        derelictStaticInfo = &derelictStaticInfos[derelicttype];
        memset(derelictStaticInfo,0,sizeof(*derelictStaticInfo));
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        missileStaticInfo = &missileStaticInfos[shiprace];
        memset(missileStaticInfo,0,sizeof(*missileStaticInfo));
        if (shiprace <= RACE_Beast)
        {
            missileStaticInfo = &mineStaticInfos[shiprace];
            memset(missileStaticInfo,0,sizeof(*missileStaticInfo));
        }
    }
}

void universeInit(void)
{
    sdword index;

    univupdateInit();

    clInit(&universe.mainCommandLayer);
    ccInit(&universe.mainCameraCommand);

    universeSetZeroPlayers();

    universeNoStaticLoadedOrNeeded();

    for (index = 0; index < MAX_MULTIPLAYER_PLAYERS + 1; index++)
    {
        universe.players[index].PlayerMothership = NULL;
    }

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        universe.world[index] = NULL;
    }
}

void universeClosePlayers(void)
{
    udword i;

    for (i=0;i<universe.numPlayers;i++)
    {
        if (universe.players[i].aiPlayer != NULL)
        {
            aiplayerClose(universe.players[i].aiPlayer);
            universe.players[i].aiPlayer = NULL;
        }
        if (&universe.players[i] == universe.curPlayerPtr)
        {
            rmCloseIfOpen();
            cmCloseIfOpen();
            lmCloseIfOpen();
        }
        universe.players[i].PlayerMothership = NULL;
        if (i != 0 || !isSinglePlayerCampaignGame())
        {
            playerInitUpgradeLevels(&universe.players[i]);
        }
    }
}

void universeReset(void)
{
    sdword index;
    Player *player;

    salCapCleanUpCloakingTarget(NULL, NULL);
    shockwaveShutdown();

    for (index = 0; index < universe.numPlayers; index++)
    {
        player = &universe.players[index];
        if (player->fogGrid != NULL)
        {
            fogDestroyGrid((ubyte *)player->fogGrid);
            player->fogGrid = NULL;
        }
    }

    clClose(&universe.mainCommandLayer);
    ccReset(&universe.mainCameraCommand);
    if (byte_8B8844)
    {
        ccSetMode(&pilotEffect, 1);
        ccSetDirtyBit(&pilotEffect);
    }

    pingAllPingsDelete();

    cmReset(); // Free carrier factory pointers in construction manager

    for (index = 0; index < universe.numPlayers; index++)
    {
        player = &universe.players[index];
        if (player->aiPlayer != NULL)
        {
            aiplayerClose(player->aiPlayer);
            player->aiPlayer = NULL;
        }
        if (player == universe.curPlayerPtr)
        {
            rmCloseIfOpen();
            cmCloseIfOpen();
            lmCloseIfOpen();
        }
        player->PlayerMothership = NULL;
        if (index != 0 || !isSinglePlayerCampaignGame())
        {
            playerInitUpgradeLevels(player);
        }
    }

    univResetSpaceObjs();

    for (index = 0; index < UNIV_NUMBER_WORLDS; index++)
    {
        universe.world[index] = NULL;
    }

    ioMainOverlayStateReset();
    tabStatsResetDrag();
    taskUpdateTimeCompression();
    leechResetFeedbackState();
    spStateA5B1AC = 0;
    tutResetPointersAfterLoad();
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticCloseAllStatics
    Description : Closes all universe static structures for good.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void universeStaticCloseAllStatics(void)
{
    ShipRace shiprace;
    ShipType shiptype;
    ShipType firstshiptype;
    ShipType lastshiptype;
    AsteroidType asteroidtype;
    DustCloudType dustcloudtype;
    GasCloudType gascloudtype;
    DerelictType derelicttype;
    StaticInfo *staticInfo;

//    CloseStatEffectInfo(&effectStaticInfo);

    if (sphereStaticInfo != NULL)
    {
        freeSphereStaticInfo(sphereStaticInfo);
        sphereStaticInfo = NULL;
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        staticInfo = (StaticInfo *)&missileStaticInfos[shiprace];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatMissileInfo((MissileStaticInfo *)staticInfo);
        }
    }

    for (shiprace=0;shiprace<2;shiprace++)        //only two races have mines
    {
        staticInfo = (StaticInfo *)&mineStaticInfos[shiprace];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatMineInfo((MissileStaticInfo *)staticInfo);
        }
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        staticInfo = (StaticInfo *)&derelictStaticInfos[derelicttype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatDerelictInfo((DerelictStaticInfo *)staticInfo);
        }
    }

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        staticInfo = (StaticInfo *)&asteroidStaticInfos[asteroidtype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatAsteroidInfo((AsteroidStaticInfo *)staticInfo);
        }
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        staticInfo = (StaticInfo *)&dustcloudStaticInfos[dustcloudtype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatDustCloudInfo((DustCloudStaticInfo *)staticInfo);
        }
    }

    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        staticInfo = (StaticInfo *)&gascloudStaticInfos[gascloudtype];
        if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            CloseStatGasCloudInfo((GasCloudStaticInfo *)staticInfo);
        }
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        firstshiptype = FirstShipTypeOfRace[shiprace];
        lastshiptype = LastShipTypeOfRace[shiprace];
        for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
        {
            staticInfo = (StaticInfo *)GetShipStaticInfo(shiptype,shiprace);
            if (bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
            {
                CloseStatShipInfo((ShipStaticInfo *)staticInfo);
            }
        }
    }

    if (defaultlod != NULL)
    {
        lodFree(defaultlod);
        defaultlod = NULL;
    }

    if (defaultmex != NULL)
    {
        mexFree(defaultmex);
        defaultmex = NULL;
    }

    if (defaultmesh != NULL)
    {
        meshFree(defaultmesh);
        defaultmesh = NULL;
    }
}

void universeClose(void)
{
    udword i;
    Player *player;

    taskUpdateTimeCompression();
    clClose(&universe.mainCommandLayer);
    ccClose(&universe.mainCameraCommand);

    for (i = 0; i < universe.numPlayers; i++)
    {
        player = &universe.players[i];
        if (player->aiPlayer != NULL)
        {
            aiplayerClose(player->aiPlayer);
            player->aiPlayer = NULL;
        }
        if (player == universe.curPlayerPtr)
        {
            rmCloseIfOpen();
            cmCloseIfOpen();
            lmCloseIfOpen();
        }
        player->PlayerMothership = NULL;
        if (i != 0 || !isSinglePlayerCampaignGame())
        {
            playerInitUpgradeLevels(player);
        }
    }

    univupdateClose();

    universeStaticCloseAllStatics();
}

/*-----------------------------------------------------------------------------
    Name        : universeStaticClose
    Description : Closes static data of the Universe
    Inputs      : none
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void universeStaticClose(void)
{
}

/*-----------------------------------------------------------------------------
    Name        : universeUpdateTask
    Description : the main loop of the task which updates the universe (all
                  mission spheres) and executes all of the physics stuff
                  and moves all of the ships.  This task runs independent of
                  the screen refresh rate.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void universeUpdateTask(void)
{
    static WaitPacketStatus waitpacketstatus;
    static sdword repeatuniv;
    static sdword repeattimes;

    taskYield(0);

    for(;;)
    {
        if ((multiPlayerGame) && (startingGame) && (gameIsRunning) && ((IAmCaptain) && (!multiPlayerGameUnderWay)))
        {
            if (checkPlayersReady())
            {
                multiPlayerGameUnderWay = TRUE;
            }
        }

        if (startingGameState == CHECK_AUTODOWNLOAD_MAP)
        {
            taskStackSaveCond(0);
            universeUpdateCounter++;

            if (!LANGame) chatClose();
            if (sigsPlayerIndex == 0)
                autodownloadmapGotMapName(spScenarios[spCurrentSelected].fileSpec,numPlayers,spScenarios[spCurrentSelected].minplayers,spScenarios[spCurrentSelected].maxplayers);
            else
                autodownloadmapGotMapName(tpGameCreated.MapName,numPlayers,0,0);
            horseRaceInit();
            if (autodownloadmapRequired())
            {
                HorseRaceBeginBar(DOWNLOADMAP_BAR);

                if (sigsPlayerIndex == 0)
                {
                    autodownloadmapGetFilesOfMap();
                }

                autodownloadmapPrintStatusStart();
                startingGameState = AUTODOWNLOAD_MAP;
            }
            else
            {
                startingGameState = PROCESS_TEXTURES;
                goto processtextures;
            }
        }
        else if (startingGameState == AUTODOWNLOAD_MAP)
        {
            taskStackSaveCond(0);
repeatagainwithoutyielding:         // for horse race hack we need this silly hack
            universeUpdateCounter++;

            autodownloadmapPrintStatus();
            if (sigsPlayerIndex == 0)
            {
                // for now, lets just bang everything through:
                if (autodownloadmapSendAFile())
                {
                    HorseRaceNext(1.0f);
                    startingGameState = PROCESS_TEXTURES;
                    goto processtextures;
                }
                else
                {
                    HorseRaceNext(autodownloadmapPercentSentFiles());
                    // don't check for aborting - will mess up everyone since files must be sent!
                }
            }
            else
            {
                if (autodownloadmapReceivedAllFiles())
                {
                    HorseRaceNext(1.0f);
                    startingGameState = PROCESS_TEXTURES;
                    goto processtextures;
                }
                else
                {
                    HorseRaceNext(autodownloadmapPercentReceivedFiles());
                    if (hrAbortLoadingGame)
                    {
                        startingGameState = PROCESS_TEXTURES;
                        goto processtextures;
                    }
                }
            }

            goto repeatagainwithoutyielding;
        }
        else if (startingGameState == PROCESS_TEXTURES)
        {
            taskStackSaveCond(0);
            universeUpdateCounter++;
processtextures:
            if (!hrAbortLoadingGame)
            {
                gameStart(NULL);
            }
            horseRaceShutdown();

            startingGameState = GAME_STARTED;

            if (hrAbortLoadingGame)
            {
                utyGameQuitToMain(NULL,NULL);
                hrAbortLoadingGame = FALSE;
            }
            else if (multiPlayerGame)
            {
                TransferCaptaincyGameStarted();
            }

            taskStackRestoreCond();
        }
        else if (gameIsRunning)
        {
            taskStackSaveCond(0);
            universeUpdateCounter++;

            if ((multiPlayerGame) || (playPackets|recordFakeSendPackets))
            {
                if (!universe.dontUpdateRenderList)
                {
                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_RENDERLIST_RATE) == REFRESH_RENDERLIST_FRAME))
                    {
                        univUpdateRenderList();
                    }

                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_MINORRENDERLIST_RATE) == REFRESH_MINORRENDERLIST_FRAME))        // make it 3 to mix things up a bit
                    {
                        univUpdateMinorRenderList();
                    }
                }

                if (!(playPackets|recordFakeSendPackets))
                {
                    if ((!multiPlayerGameUnderWay) && (!IAmCaptain))
                    {
                        if (!HaveSentNonCaptainReadyPacket)
                        {
                            SendNonCaptainReadyPacket(sigsPlayerIndex);
                            HaveSentNonCaptainReadyPacket = TRUE;
                        }
                    }
                }

                if (multiPlayerGame)
                {
                    gcPollForNewChat();
                    TransferCaptaincyUpdate();
                    KeepAliveUpdate();

                    if ((universeUpdateCounter & INTERNET_LAG_UPDATERATE) == 0)
                    {
                        lagUpdateInternetLag();
                    }
                }

                if (playPackets|recordFakeSendPackets)
                {
                    if (universePause)
                    {
                        cat_TimerFunc2();
                        goto alldone;
                    }
                }

                if ((playPackets|recordFakeSendPackets) && universeTurbo)
                {
                    repeattimes = UNIVERSE_TURBO_REPEAT>>1;
                    dbgAssert(repeattimes >= 1);
                }
                else
                    repeattimes = 1;



                for (repeatuniv=0;repeatuniv<repeattimes;repeatuniv++)
                {
                    waitpacketstatus = clWaitSyncPacket(&universe.mainCommandLayer);
                    if (waitpacketstatus != NO_PACKET)
                    {
                        if (multiPlayerGame)
                        {
                            multiPlayerGameUnderWay = TRUE;
                            TransferCaptaincySyncPacketReceivedNotify();
                        }

                        netCheck();

                        if (univUpdate(UNIVERSE_UPDATE_PERIOD))
                        {
                            goto alldone;
                        }
                        if (! ((waitpacketstatus == TOO_MANY_PACKETS) ||        // if TOO_MANY_PACKETS, don't yield, and have throughput at twice normal to catch up
                              (repeattimes > 1)) )
                        {
                            taskStackRestoreCond();
                            taskYield(0);
                            taskStackSaveCond(0);
                        }
                        if (!gameIsRunning)
                        {
                            goto alldone;
                        }
                        if (univUpdate(UNIVERSE_UPDATE_PERIOD))
                        {
                            goto alldone;
                        }
                    }
                }
    alldone:;

            }
            else
            {
                if (!universe.dontUpdateRenderList)
                {
                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_RENDERLIST_RATE) == REFRESH_RENDERLIST_FRAME))
                    {
                        univUpdateRenderList();
                    }

                    if ((universe.univUpdateCounter == 1) || ((universeUpdateCounter & REFRESH_MINORRENDERLIST_RATE) == REFRESH_MINORRENDERLIST_FRAME))        // make it 3 to mix things up a bit
                    {
                        univUpdateMinorRenderList();
                    }
                }

                if (universePause)
                {
                    cat_TimerFunc2();
                    goto alldone2;
                }

                if((tutorial==TUTORIAL_ONLY) && !tutEnable.bGameRunning)
                {
                    tutEnable.bKASFrame ^= 1;
                    if(tutEnable.bKASFrame)
                        kasExecute();       // Make sure KAS gets called

                    goto alldone2;
                }

#if (!UNIVERSE_TURBORECORD_ONLY)
                if (universeTurbo)
                {
                    for (repeatuniv=0;repeatuniv<UNIVERSE_TURBO_REPEAT;repeatuniv++)
                    {
                        if (univUpdate(UNIVERSE_UPDATE_PERIOD))
                        {
                            goto alldone2;
                        }
#ifndef HW_Release
                        if (ShowFancyFights) statsShowFancyFightUpdate();
#endif
                    }
                }
#endif
                univUpdate(UNIVERSE_UPDATE_PERIOD);
alldone2:;
#ifndef HW_Release
                if (ShowFancyFights) statsShowFancyFightUpdate();
#endif

            }
            taskStackRestoreCond();
        }
        else if (!startingGame)
        {
            if (sigsPressedStartGame)
            {
                utyNewGameStart(NULL, NULL);
            }
        }

        if (startRecordingGameWhenSafe)
        {
            recPackInGameStartCBSafeToStart();
            startRecordingGameWhenSafe = FALSE;
        }

        taskYield(0);
    }
    taskExit();
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : universeSwitchToNextPlayer
    Description : switches current player to the next player (wraps around)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void universeSwitchToNextPlayer(void)
{
    universe.curPlayerIndex++;
    if (universe.curPlayerIndex >= universe.numPlayers)
    {
        universe.curPlayerIndex = 0;
    }
    universe.curPlayerPtr = &universe.players[universe.curPlayerIndex];

    collUpdateAllBlobs = TRUE;                              //next update will update all blobs

    dbgMessagef("\nCurrent player: %d %x",universe.curPlayerIndex,universe.curPlayerPtr);
}

/*-----------------------------------------------------------------------------
    Name        : universeSwitchToPlayer
    Description : switches current player to player indicated by playerIndex
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void universeSwitchToPlayer(uword playerIndex)
{
    dbgAssert(playerIndex < universe.numPlayers);
    universe.curPlayerIndex = playerIndex;
    universe.curPlayerPtr = &universe.players[universe.curPlayerIndex];
    univSetCurrentSensorCoverage((sdword)universe.curPlayerPtr->fogGrid);
}

/*-----------------------------------------------------------------------------
    Name        : GetShipStaticInfo
    Description : returns ShipStaticInfo of shiptype, shiprace
    Inputs      : shiptype, shiprace
    Outputs     :
    Return      : returns ShipStaticInfo of shiptype, shiprace
----------------------------------------------------------------------------*/
ShipStaticInfo *GetShipStaticInfo(ShipType shiptype,ShipRace shiprace)
{
    dbgAssert(shiptype < TOTAL_NUM_SHIPS);
    return &shipStaticInfos[shiptype];
}

/*-----------------------------------------------------------------------------
    Name        : GetShipStaticInfoSafe
    Description : returns ShipStaticInfo of shiptype, shiprace, NULL if not valid
    Inputs      : shiptype, shiprace
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
ShipStaticInfo *GetShipStaticInfoSafe(ShipType shiptype,ShipRace shiprace)
{
    ShipStaticInfo *shipstatic;

    if (shiptype >= TOTAL_NUM_SHIPS)
    {
        return NULL;
    }

    shipstatic = &shipStaticInfos[shiptype];
    if (!bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        return NULL;
    }

    return shipstatic;
}

/*-----------------------------------------------------------------------------
    Name        : GetValidRaceForShipType
    Description : returns valid race for shiptype, -1 if none found
    Inputs      : shiptype
    Outputs     :
    Return      : returns valid race for shiptype, -1 if none found
----------------------------------------------------------------------------*/
ShipRace GetValidRaceForShipType(ShipType shiptype)
{
    ShipRace race;

    dbgAssert(shiptype < TOTAL_NUM_SHIPS);

    for (race = 0; race < NUM_RACES; race++)
    {
        if (shiptype >= FirstShipTypeOfRace[race] && shiptype <= LastShipTypeOfRace[race])
        {
            return race;
        }
    }

    dbgAssert(FALSE);
    return RACE_Sect;
}

/*-----------------------------------------------------------------------------
    Name        : GetShipStaticInfoValidate
    Description : returns ShipStaticInfo of shiptype, shiprace.  Makes sure data loaded
    Inputs      : shiptype, shiprace
    Outputs     :
    Return      : returns ShipStaticInfo of shiptype, shiprace
----------------------------------------------------------------------------*/
ShipStaticInfo *GetShipStaticInfoValidate(ShipType shiptype,ShipRace shiprace)
{
    ShipStaticInfo *shipstatic = GetShipStaticInfo(shiptype,shiprace);
    if (!bitTest(shipstatic->staticheader.infoFlags, IF_InfoLoaded))
    {
        dbgFatalf(DBG_Loc,"Ship %s %s not loaded",ShipRaceToStr(shiprace),ShipTypeToStr(shiptype));
    }
    return shipstatic;
}

/*  Unit Capping functions */

/*-----------------------------------------------------------------------------
    Name        : unitCapCreateShip
    Description : Updates ship totals for player
    Inputs      : uword shiptype, Player *player
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void unitCapCreateShip(Ship *ship, Player *player)
{
//    if (player->aiPlayer)
//    {
//        return;
//    }
    // don't keep tabs on these ship types
    if ((cdEnabled) &&
        (!ShipTypeIsTargetDroneType(ship->shiptype)) &&
        (ship->shiptype != TargetDrone) &&
        (ship->shiptype != HeadShotAsteroid) &&
        (ship->shiptype != CryoTray) &&
        (!(bitTest(ship->specialFlags1C8, CATA_RUNTIME_IN_UNIT_CAPS))))
    {
        player->totalships++;
        player->shiptotals[ship->shiptype]++;
        player->classtotals[ship->staticinfo->shipclass]++;
        bitSet(ship->specialFlags1C8, CATA_RUNTIME_IN_UNIT_CAPS);
    }
}

/*-----------------------------------------------------------------------------
    Name        : unitCapDeleteShip
    Description : Updates ship totals for player
    Inputs      : uword shiptype, Player *player
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void unitCapDeleteShip(Ship *ship, Player *player)
{

    // don't keep tabs on these ship types
//    if (player->aiPlayer)
//    {
//        return;
//    }
    if ((cdEnabled) &&
        (!ShipTypeIsTargetDroneType(ship->shiptype)) &&
        (ship->shiptype != TargetDrone) &&
        (ship->shiptype != HeadShotAsteroid) &&
        (ship->shiptype != CryoTray) &&
        (bitTest(ship->specialFlags1C8, CATA_RUNTIME_IN_UNIT_CAPS)))
    {
        player->totalships--;

//        dbgAssert(player->totalships >= 0);
        if (player->totalships < 0)
        {
            player->totalships = 0;
            dbgMessagef("\nWarning: total shipcount in unit caps went below 0");
        }

        player->shiptotals[ship->shiptype]--;

//        dbgAssert(player->shiptotals[ship->shiptype] >= 0);
        if (player->shiptotals[ship->shiptype] < 0)
        {
            player->shiptotals[ship->shiptype] = 0;
            dbgMessagef("\nWarning: shipcount of shiptype %d went below 0");
        }

        player->classtotals[ship->staticinfo->shipclass]--;

//        dbgAssert(player->classtotals[ship->staticinfo->shipclass] >= 0);
        if (player->classtotals[ship->staticinfo->shipclass] < 0)
        {
            player->classtotals[ship->staticinfo->shipclass] = 0;
            dbgMessagef("\nWarning: classtotals of class %d went below 0",ship->staticinfo->shipclass);
        }
        bitClear(ship->specialFlags1C8, CATA_RUNTIME_IN_UNIT_CAPS);
    }
}

/*-----------------------------------------------------------------------------
    Name        : unitCapCanCreateShip
    Description : Can we build this ship ??
    Inputs      : uword shiptype, Player *player
    Outputs     : TRUE if ship is within limit caps
                  FLASE if ship will exceed limit caps
    Return      : bool8
-----------------------------------------------------------------------------*/
bool32 unitCapCanCreateShip(ShipType ship, shipsinprogress *factory, shipavailable *cmShipsAvail)
{
    Player *player=factory->ship->playerowner;
    shipsinprogress *allfactories=(shipsinprogress *)factory->node.belongto->head;
    sdword totalships = 1, typeships = 1, classships = 1, index;
    ShipStaticInfo *statinfo, *shipstatic;

    if ((!bitTest(tpGameCreated.flag,MG_UnitCapsEnabled)) &&
        (ship!=sSentinel)&&(!singlePlayerGame))
    {
        return(TRUE);
    }

    shipstatic = GetShipStaticInfo(ship, player->race);

    // total up all ships in the queue but not yet started
    for (index=0;cmShipsAvail[index].nJobs != -1; index++)
    {
        statinfo = GetShipStaticInfo(cmShipsAvail[index].type, player->race);
        totalships+=cmShipsAvail[index].nJobs;
        if (cmShipsAvail[index].type == shipstatic->shiptype) typeships+=cmShipsAvail[index].nJobs;
        if (statinfo->shipclass == shipstatic->shipclass) classships+=cmShipsAvail[index].nJobs;
    }

    // walk through list of factories
    do
    {
        // if factory is owned by player then total all ships being built
        if (allfactories->ship->playerowner==player)
        {
            for (index=0;index<TOTAL_NUM_SHIPS;index++)
            {
                statinfo = GetShipStaticInfo(index, player->race);
                totalships+=allfactories->progress[index].nJobs + shiplagtotals[index];
                if (index==shipstatic->shiptype) typeships+=allfactories->progress[index].nJobs + shiplagtotals[index];
                if (statinfo->shipclass == shipstatic->shipclass) classships+=allfactories->progress[index].nJobs + shiplagtotals[index];
            }

        }
        // next factory
        allfactories=(shipsinprogress *)allfactories->node.next;
    }
    while (allfactories != NULL);

    if (player->totalships+totalships > cdMaxShipsAllowed)
    {
        speechEventFleet(STAT_F_Const_TotalLimit, 0, player->playerIndex);
        // player already has too many ships cannot build anymore
        return(FALSE);
    }

    if (cdLimitCaps[shipstatic->shiptype] != -1)
    {
        if ((shipstatic->shiptype == sCarrierSupport) &&
            (factory->ship != NULL) &&
            (factory->ship->shiptype == sCarrier))
        {
            sdword localCarrierSupportShips = 1 + factory->progress[shipstatic->shiptype].nJobsTotal +
                                              cmCountAvailableCarrierSupport(factory->ship);

            for (index = 0; cmShipsAvail[index].nJobs != -1; index++)
            {
                if (cmShipsAvail[index].type == shipstatic->shiptype)
                {
                    localCarrierSupportShips += cmShipsAvail[index].nJobs;
                }
            }

            if (localCarrierSupportShips > cdLimitCaps[shipstatic->shiptype])
            {
                speechEventFleet(STAT_F_Const_UnitLimit, shipstatic->shiptype, player->playerIndex);
                return(FALSE);
            }
        }
        else if (player->shiptotals[shipstatic->shiptype]+typeships > cdLimitCaps[shipstatic->shiptype])
        {
            speechEventFleet(STAT_F_Const_UnitLimit, shipstatic->shiptype, player->playerIndex);
            // player has to many of this type of ship
            return(FALSE);
        }
    }

    if ((cdClassCaps[shipstatic->shipclass] != -1) &&
        (player->classtotals[shipstatic->shipclass]+classships > cdClassCaps[shipstatic->shipclass]))
    {
        speechEventFleet(STAT_F_Const_UnitLimit, shipstatic->shiptype, player->playerIndex);
        // player has to many of this class of ship
        return(FALSE);
    }

    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : unitCapInitialize
    Description : Initialize the unit totals to zero
    Inputs      : Player *player
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void unitCapInitialize(Player *player)
{
    player->totalships=0;
    memset(player->shiptotals, 0,sizeof(sdword)*TOTAL_NUM_SHIPS);
    memset(player->classtotals,0,sizeof(sdword)*NUM_CLASSES);
}

/*-----------------------------------------------------------------------------
    Name        : unitCapEnable
    Description : Enables unit cap tracking
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void unitCapEnable(void)
{
    cdEnabled = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : unitCapDisable
    Description : disables unit cap tracking
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void unitCapDisable(void)
{
    cdEnabled = FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : resourceInjectionTask
    Description : this is setup to be called at a specified time if it was enabled.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void unitCapInitStatics(udword numplayers)
{
    char temp[128];

    sprintf(temp,"UnitLimitCaps%i.script", numplayers);
    scriptSet(NULL,temp,ShipStaticCapLimits);
}

/*-----------------------------------------------------------------------------
    Name        : resourceInjectionTask
    Description : this is setup to be called at a specified time if it was enabled.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void resourceInjectionTask(void)
{
    sdword index;

    for (index=0;index<universe.numPlayers;index++)
    {
        if (universe.players[index].playerState != PLAYER_DEAD)
        {
            universe.players[index].resourceUnits += tpGameCreated.resourceInjectionsAmount;
            universe.gameStats.playerStats[index].totalResourceUnits += tpGameCreated.resourceInjectionsAmount;
            universe.gameStats.playerStats[index].totalInjectedResources += tpGameCreated.resourceInjectionsAmount;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : resourceLumpSum
    Description : this will give each player a lump sum amount of resources, once during the game
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void resourceLumpSum(void)
{
    sdword index;

    for (index=0;index<universe.numPlayers;index++)
    {
        if (universe.players[index].playerState != PLAYER_DEAD)
        {
            universe.players[index].resourceUnits += tpGameCreated.resourceLumpSumAmount;
            universe.gameStats.playerStats[index].totalResourceUnits += tpGameCreated.resourceLumpSumAmount;
            universe.gameStats.playerStats[index].totalInjectedResources += tpGameCreated.resourceLumpSumAmount;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : initPlayerStats
    Description : zeros the players inital multiplayer statistics
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void initGameStats(Universe *universe)
{
    sdword j,i;

    universe->gameStats.totalResourcesInGame = 0;
    universe->gameStats.totalShipsInGame = 0;
    universe->gameStats.totalKills = 0;
    universe->gameStats.totalRUsInAllShips = 0;
    universe->gameStats.totalRUsKilled = 0;
    universe->gameStats.totalResourceUnitsCollected = 0;
    universe->gameStats.totalRUsLost = 0;
    universe->gameStats.updatedRUValuesTime = 0.0f;
    universe->gameStats.universeRUWorth = 0;


    for(i=0;i<universe->numPlayers;i++)
    {
        universe->gameStats.playerStats[i].timeOfDeath = 0;
        universe->gameStats.playerStats[i].typeOfPlayerDeath = 0;
        universe->gameStats.playerStats[i].totalResourceUnits = 0;
        universe->gameStats.playerStats[i].totalResourceUnitsCollected = 0;                 //[]
        universe->gameStats.playerStats[i].totalResourceUnitsSpent = 0;                     //[]
        universe->gameStats.playerStats[i].totalResourceUnitsGiven = 0;                     //[]
        universe->gameStats.playerStats[i].totalResourceUnitsRecieved = 0;
        universe->gameStats.playerStats[i].totalRegeneratedResourceUnits = 0;               //[x]
        universe->gameStats.playerStats[i].totalInjectedResources = 0;               //[x]
        universe->gameStats.playerStats[i].totalResourceUnitsViaBounties = 0;               //[x]
        universe->gameStats.playerStats[i].totalCrystalsRetrieved = 0;



        universe->gameStats.playerStats[i].totalRUsInCurrentShips = 0;
        universe->gameStats.playerStats[i].totalShips = 0;

        universe->gameStats.playerStats[i].totalKills = 0;
        universe->gameStats.playerStats[i].totalRUsKilled = 0;

        universe->gameStats.playerStats[i].totalLosses = 0;
        universe->gameStats.playerStats[i].totalRUsLost = 0;


        for(j=0;j<universe->numPlayers;j++)
        {
            universe->gameStats.playerStats[i].pointsOfDamageDoneToWho[j] = 0.0f;
        }

        for(j=0;j<TOTAL_NUM_SHIPS;j++)
        {
            universe->gameStats.playerStats[i].totalForEachShip[j]=0;
            universe->gameStats.playerStats[i].totalKillsForEachShip[j]=0;
            universe->gameStats.playerStats[i].totalLossesForEachShip[j]=0;
        }
        for(j=0;j<NUM_CLASSES;j++)
        {
            universe->gameStats.playerStats[i].totalForEachClass[j]=0;
            universe->gameStats.playerStats[i].totalKillsForEachClass[j]=0;
            universe->gameStats.playerStats[i].totalLossesForEachClass[j]=0;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : gameStatsAddShip
    Description : Updates stats for ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void gameStatsAddShip(Ship *ship,sdword playerIndex)
{
    universe.gameStats.playerStats[playerIndex].totalShips++;
    universe.gameStats.playerStats[playerIndex].totalForEachShip[ship->shiptype]++;
    universe.gameStats.playerStats[playerIndex].totalForEachClass[ship->staticinfo->shipclass]++;
    universe.gameStats.totalRUsInAllShips += ship->staticinfo->buildCost;
    universe.gameStats.playerStats[playerIndex].totalRUsInCurrentShips += ship->staticinfo->buildCost;
    universe.gameStats.totalShipsInGame;
}


void gameStatsShipDied(Ship *ship)
{
    sdword shipownerindex,killerPlayerIndex;

    enum
    {
        GAMESTATS_SKIP_CLASS_TYPE0 = 9,
        GAMESTATS_SKIP_CLASS_TYPE1 = 43,
        GAMESTATS_DOUBLE_CLASS_TYPE0 = 4,
        GAMESTATS_DOUBLE_CLASS_TYPE1 = 5,
        GAMESTATS_DOUBLE_CLASS_TYPE2 = 38
    };

    if(ship->playerowner != NULL)
        shipownerindex = ship->playerowner->playerIndex;
    else
        dbgFatalf(DBG_Loc,"\nShip player pointer NULL...shouldn't be");
    switch(ship->howDidIDie)
    {
    case DEATH_Killed_By_Player:
    case DEATH_Killed_By_Player_Explosion:
    case DEATH_I_DIDNT_DIE_I_WAS_SALAVAGED:
    case DEATH_Killed_By_Kamikaze:
        killerPlayerIndex = ship->whoKilledMe;
        universe.gameStats.totalRUsKilled += ship->staticinfo->buildCost;
        universe.gameStats.totalKills++;
        break;
    case DEATH_BY_GAME_END:
        return;

    //non player depenedant:  Only register losses, not kills
    case DEATH_Killed_By_Collision:
    case DEATH_Killed_By_Cloud:
    case DEATH_Killed_By_Dead_Player:
    case DEATH_BY_RETIREMENT:
    case DEATH_BY_AFTER_DEAD:
    case DEATH_Left_Behind_At_Hyperspace:
    default:
        killerPlayerIndex = 99;
        break;
    }

    if(killerPlayerIndex != shipownerindex && killerPlayerIndex < MAX_MULTIPLAYER_PLAYERS)
    {
        universe.gameStats.playerStats[killerPlayerIndex].totalRUsKilled += ship->staticinfo->buildCost;
        universe.gameStats.playerStats[killerPlayerIndex].totalKills++;
        universe.gameStats.playerStats[killerPlayerIndex].totalKillsForEachShip[ship->shiptype]++;
        if (ship->shiptype != GAMESTATS_SKIP_CLASS_TYPE0 && ship->shiptype != GAMESTATS_SKIP_CLASS_TYPE1)
        {
            universe.gameStats.playerStats[killerPlayerIndex].totalKillsForEachClass[ship->staticinfo->shipclass]++;
            if (ship->shiptype == GAMESTATS_DOUBLE_CLASS_TYPE0 ||
                ship->shiptype == GAMESTATS_DOUBLE_CLASS_TYPE1 ||
                ship->shiptype == GAMESTATS_DOUBLE_CLASS_TYPE2)
            {
                universe.gameStats.playerStats[killerPlayerIndex].totalKillsForEachClass[ship->staticinfo->shipclass]++;
            }
        }
    }

    universe.gameStats.playerStats[shipownerindex].totalLosses++;
    universe.gameStats.playerStats[shipownerindex].totalLossesForEachShip[ship->shiptype]++;
    if (ship->shiptype != GAMESTATS_SKIP_CLASS_TYPE0 && ship->shiptype != GAMESTATS_SKIP_CLASS_TYPE1)
    {
        universe.gameStats.playerStats[shipownerindex].totalLossesForEachClass[ship->staticinfo->shipclass]++;
        if (ship->shiptype == GAMESTATS_DOUBLE_CLASS_TYPE0 ||
            ship->shiptype == GAMESTATS_DOUBLE_CLASS_TYPE1 ||
            ship->shiptype == GAMESTATS_DOUBLE_CLASS_TYPE2)
        {
            universe.gameStats.playerStats[shipownerindex].totalLossesForEachClass[ship->staticinfo->shipclass]++;
        }
    }
    universe.gameStats.playerStats[shipownerindex].totalRUsLost += ship->staticinfo->buildCost;
    universe.gameStats.playerStats[shipownerindex].totalRUsInCurrentShips -= ship->staticinfo->buildCost;

    universe.gameStats.totalRUsLost += ship->staticinfo->buildCost;
    universe.gameStats.totalRUsInAllShips -= ship->staticinfo->buildCost;
}
//DEBUG FUNCTION

#ifdef DEBUG_GAME_STATS

char gamestatsfilename[] = "gamestats.log";
sdword needToInit=FALSE;

void gameStatsInitLog()
{
    needToInit=TRUE;
}
void gameStatsInitForReal()
{
    filehandle fh;
    FILE *fp;
    sdword i;
    GameStatsDebugHeader header;
    fileDelete(gamestatsfilename);

    fh = fileOpen(gamestatsfilename,FF_AppendMode);
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    header.numPlayers = universe.numPlayers;
    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        strcpy(header.names[i],playerNames[i]);
    }
    //now create and save header file
    fwrite(&header,sizeof(GameStatsDebugHeader),1,fp);

    fileClose(fh);
}
void gameStatsUpdateLogFile(sdword framenum)
{
    filehandle fh;
    FILE *fp;
    //sdword i;

    if(needToInit)
    {
        gameStatsInitForReal();
        needToInit=FALSE;
    }
    fh = fileOpen(gamestatsfilename,FF_AppendMode);
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    //write frame number
    fwrite(&framenum,sizeof(sdword),1,fp);
    //write global game stats
    fwrite(&universe.gameStats,sizeof(GameStats)-sizeof(PlayerStats)*(MAX_MULTIPLAYER_PLAYERS+1-universe.numPlayers),1,fp);
    //write individual player stats
    //for(i=0;i<universe.numPlayers;i++)
    //{
    //    fwrite(&universe.gameStats.playerStats[i],sizeof(PlayerStats),1,fp);
    //}
    fileClose(fh);
}

#endif

// this function calculates the total ship cost for each player (typically at game start)
void gameStatsCalcShipCostTotals(void)
{
    ShipStaticInfo *pInfo;

    sdword i,j;

    // check for single player game
    if(singlePlayerGame) return;

    for(i=0;i<universe.numPlayers;i++)
    {
        // reset cost
        universe.players[i].initialShipCost=0;

        // calculate cost
        for(j=0;j<TOTAL_NUM_SHIPS;j++)
        {
            if(universe.gameStats.playerStats[i].totalForEachShip[j] > 0)
            {
                pInfo=GetShipStaticInfoSafe(j,universe.players[i].race);
                if(pInfo != NULL)
                {
                    universe.players[i].initialShipCost+=universe.gameStats.playerStats[i].totalForEachShip[j]*
                        pInfo->buildCost;
                }
            }
        }
    }

    return;
}

// this function returns the actual ship class based on ship type, -1 otherwise
sdword gameStatsGetShipClass(sdword nShipType)
{
    if (ShipTypeIsCarrierType(nShipType))
    {
        return(CLASS_Carrier);
    }

    if (ShipTypeIsHomeworldMultiGunCorvetteType(nShipType) ||
        ShipTypeIsSalCapCorvetteType(nShipType))
    {
        return(CLASS_Corvette);
    }

    if (ShipTypeIsHomeworldMissileDestroyerType(nShipType) ||
        ShipTypeIsHomeworldStandardDestroyerType(nShipType))
    {
        return(CLASS_Destroyer);
    }

    if (ShipTypeIsHomeworldAttackBomberType(nShipType) ||
        ShipTypeIsHomeworldHeavyDefenderType(nShipType) ||
        ShipTypeIsHomeworldHeavyInterceptorType(nShipType))
    {
        return(CLASS_Fighter);
    }

    if ((nShipType == kAdvanceSupportFrigate) ||
        ShipTypeIsDefenseFieldFrigateType(nShipType) ||
        ShipTypeIsHomeworldIonCannonFrigateType(nShipType) ||
        ShipTypeIsHomeworldStandardFrigateType(nShipType))
    {
        return(CLASS_Frigate);
    }

    if ((nShipType == kHeavyCruiser) || (nShipType == tHeavyCruiser))
    {
        return(CLASS_HeavyCruiser);
    }

    if (ShipTypeIsMothershipType(nShipType))
    {
        return(CLASS_Mothership);
    }

    if (ShipTypeIsCloakGeneratorType(nShipType) ||
        ShipTypeIsGravWellGeneratorType(nShipType) ||
        ShipTypeIsProximitySensorType(nShipType) ||
        ShipTypeIsProcessorType(nShipType) ||
        ShipTypeIsTargetDroneType(nShipType))
    {
        return(CLASS_NonCombat);
    }

    switch(nShipType)
    {
        case JunkYardHQ:
            return(CLASS_Carrier);

        case sACV:
        case kHeavyCorvette:
        case tMinelayerCorvette:
        case sRepairDroid:
        case bRepairDroid:
        case P1MissileCorvette:
        case P1StandardCorvette:
        case JunkYardDawg:
            return(CLASS_Corvette);

        case MiningBase:
            return(CLASS_Destroyer);

        case bCloakedFighter:
        case sRecon:
        case P1Fighter:
            return(CLASS_Fighter);

        case P1IonArrayFrigate:
        case Junk_LGun:
        case Junk_SGun:
            return(CLASS_Frigate);

        case P1Mothership:
        case FloatingCity:
            return(CLASS_Mothership);

        case sSentinel:
        case CryoTray:
        case CargoBarge:
        case ResearchStation:
        case ResearchStationBridge:
        case ResearchStationTower:
        case Ghostship:
            return(CLASS_NonCombat);

        case sWorker:
        case bWorker:
        case tResourceCollector:
            return(CLASS_Resource);

        default:
            return(-1);
    }
}

#define isPlayerIndexCmptPlayer(i)  ((i) >= tpGameCreated.numPlayers)

// this function checks the bounds for RUs
bool32 verifyGameStatsResUnitsBounds(FILE *pStatsFile,sdword nPlayerNum,sdword nResourceUnits)
{
    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    if((nResourceUnits < 0) || (nResourceUnits >= 100000))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadResUnitsBounds));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the totals for RUs
bool32 verifyGameStatsResUnitsTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword nStartingResources=0;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    switch(tpGameCreated.startingResources)
    {
        case MG_Small :
            nStartingResources=resourceStartSmall;
            break;
        case MG_Medium :
            nStartingResources=resourceStartMedium;
            break;
        case MG_Large :
            nStartingResources=resourceStartLarge;
            break;
    }

    if(universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsSpent > (
        universe.gameStats.playerStats[nPlayerNum].totalInjectedResources+
        universe.gameStats.playerStats[nPlayerNum].totalRegeneratedResourceUnits+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsCollected+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsGiven+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsRecieved+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsViaBounties+
        nStartingResources
        ))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadResUnitTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the bounds for the number of ships (based on the number of players)
bool32 verifyGameStatsShipsBounds(FILE *pStatsFile,sdword nPlayerNum,sdword nNumShips,sdword nNumPlayers)
{
    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    if((nNumShips < 0) || (nNumShips >= 1000*nNumPlayers))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipBounds));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the ship cost totals
bool32 verifyGameStatsShipCostTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    ShipStaticInfo *pInfo;

    sdword n,nShipTotal,nStartingResources=0;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    switch(tpGameCreated.startingResources)
    {
        case MG_Small :
            nStartingResources=resourceStartSmall;
            break;
        case MG_Medium :
            nStartingResources=resourceStartMedium;
            break;
        case MG_Large :
            nStartingResources=resourceStartLarge;
            break;
    }

    // reset total
    nShipTotal=-universe.players[nPlayerNum].initialShipCost;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n] > 0)
        {
            pInfo=GetShipStaticInfoSafe(n,universe.players[nPlayerNum].race);
            if(pInfo != NULL)
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n]*
                    pInfo->buildCost;
            }
        }
    }

    // compare totals
    if(((nShipTotal - nShipTotal/50 ) > (
        universe.gameStats.playerStats[nPlayerNum].totalInjectedResources+
        universe.gameStats.playerStats[nPlayerNum].totalRegeneratedResourceUnits+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsCollected+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsGiven+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsRecieved+
        universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsViaBounties+
        nStartingResources
        )) || ((nShipTotal - nShipTotal/50 ) > universe.gameStats.playerStats[nPlayerNum].totalResourceUnitsSpent))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipCostTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the total number of ships acquired
bool32 verifyGameStatsShipsAcquiredTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword n,nShipTotal,nClassTotal;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n] > 0)
        {
            nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n];
        }
    }

    // reset total
    nClassTotal=0;

    // calculate total
    for(n=0;n<NUM_CLASSES;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachClass[n] > 0)
        {
            nClassTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachClass[n];
        }
    }

    // compare total
    if((universe.gameStats.playerStats[nPlayerNum].totalShips != nShipTotal) ||
        (universe.gameStats.playerStats[nPlayerNum].totalShips != nClassTotal))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsAcquiredTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the total number of ships killed
bool32 verifyGameStatsShipsKilledTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword n,nShipTotal,nClassTotal;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n] > 0)
        {
            nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n];
        }
    }

    // reset total
    nClassTotal=0;

    // calculate total
    for(n=0;n<NUM_CLASSES;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalKillsForEachClass[n] > 0)
        {
            nClassTotal+=universe.gameStats.playerStats[nPlayerNum].totalKillsForEachClass[n];
        }
    }

    // compare total
    if((universe.gameStats.playerStats[nPlayerNum].totalKills != nShipTotal) ||
        (universe.gameStats.playerStats[nPlayerNum].totalKills != nClassTotal))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsKilledTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the total number of ships lost
bool32 verifyGameStatsShipsLostTotals(FILE *pStatsFile,sdword nPlayerNum)
{
    sdword n,nShipTotal,nClassTotal;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n] > 0)
        {
            nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n];
        }
    }

    // reset total
    nClassTotal=0;

    // calculate total
    for(n=0;n<NUM_CLASSES;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalLossesForEachClass[n] > 0)
        {
            nClassTotal+=universe.gameStats.playerStats[nPlayerNum].totalLossesForEachClass[n];
        }
    }

    // compare total
    if((universe.gameStats.playerStats[nPlayerNum].totalLosses != nShipTotal) ||
        (universe.gameStats.playerStats[nPlayerNum].totalLosses != nClassTotal))
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsLostTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the number of ships acquired per class
bool32 verifyGameStatsShipsAcquiredClassTotals(FILE *pStatsFile,sdword nPlayerNum,sdword nShipClass)
{
    sdword n,nShipTotal,nActualClass;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n] > 0)
        {
            // get actual ship class
            nActualClass=gameStatsGetShipClass(n);
            if((nActualClass >= 0) && (nActualClass == nShipClass))
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalForEachShip[n];
            }
        }
    }

    // compare totals
    if(nShipTotal != universe.gameStats.playerStats[nPlayerNum].totalForEachClass[nShipClass])
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsAcquiredClassTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the number of ships killed per class
bool32 verifyGameStatsShipsKilledClassTotals(FILE *pStatsFile,sdword nPlayerNum,sdword nShipClass)
{
    sdword n,nShipTotal,nActualClass;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n] > 0)
        {
            // get actual ship class
            nActualClass=gameStatsGetShipClass(n);
            if((nActualClass >= 0) && (nActualClass == nShipClass))
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalKillsForEachShip[n];
            }
        }
    }

    // compare totals
    if(nShipTotal != universe.gameStats.playerStats[nPlayerNum].totalKillsForEachClass[nShipClass])
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsKilledClassTotals));
        return(FALSE);
    }

    return(TRUE);
}

// this function checks the number of ships lost per class
bool32 verifyGameStatsShipsLostClassTotals(FILE *pStatsFile,sdword nPlayerNum,sdword nShipClass)
{
    sdword n,nShipTotal,nActualClass;

    // check for single player game
    if(singlePlayerGame) return(TRUE);

    // check computer player flag
    if(isPlayerIndexCmptPlayer(nPlayerNum)) return(TRUE);

    // reset total
    nShipTotal=0;

    // calculate total
    for(n=0;n<TOTAL_NUM_SHIPS;n++)
    {
        if(universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n] > 0)
        {
            // get actual ship class
            nActualClass=gameStatsGetShipClass(n);
            if((nActualClass >= 0) && (nActualClass == nShipClass))
            {
                nShipTotal+=universe.gameStats.playerStats[nPlayerNum].totalLossesForEachShip[n];
            }
        }
    }

    // compare totals
    if(nShipTotal != universe.gameStats.playerStats[nPlayerNum].totalLossesForEachClass[nShipClass])
    {
        // write warning to stats file
        fprintf(pStatsFile,"\t%s",strGetString(strStatsBadShipsLostClassTotals));
        return(FALSE);
    }

    return(TRUE);
}

// why are there no comments below?
void writeGameStatsToFile(char *filename)
{
    sdword i,j,startingresources=0;
    filehandle statsFH;
    FILE *statsFile = NULL;

    if (singlePlayerGame)
    {
        return;
    }

    statsFH = fileOpen(filename, FF_WriteMode | FF_TextMode | FF_ReturnNULLOnFail);
    if (!statsFH)
    {
        return;
    }
    dbgAssert(!fileUsingBigfile(statsFH));
    statsFile = fileStream(statsFH);

    if (statsFile)
    {
        if (netcheckRandSyncErr)
        {
            fprintf(statsFile,"\nRAND SYNC ERR: Frame %d",netcheckRandSyncErrFrame);
        }
        if (netcheckUnivSyncErr)
        {
            fprintf(statsFile,"\nUNIV SYNC ERR: Frame %d",netcheckUnivSyncErrFrame);
        }
        if (multiPlayerGame)        // only print cheat detect in multiplayer game
        {
            if (netcheckBlobSyncErr)
            {
                fprintf(statsFile,"\n");
                fprintf(statsFile,strGetString(strCheatDetect),netcheckBlobSyncErrFrame);
            }
        }
        if (netcheckPktSyncErr)
        {
            fprintf(statsFile,"\nPKT SYNC ERR");
        }

        for(i=0;i<universe.numPlayers;i++)
        {
            fprintf(statsFile,"\n*************");
            fprintf(statsFile,"\n*************");
            fprintf(statsFile,"\n");
            fprintf(statsFile,strGetString(strStatsPlayerName),playerNames[i]);
            fprintf(statsFile,"\n");
            fprintf(statsFile,strGetString(strStatsPlayerRace),ShipRaceToStr(universe.players[i].race));
            fprintf(statsFile,"\n___________________\n");

            if (universe.gameStats.playerStats[i].timeOfDeath)
            {
                fprintf(statsFile,"\n");
                fprintf(statsFile,strGetString(strStatsTimeOfDeath), universe.gameStats.playerStats[i].timeOfDeath);
                if (universe.gameStats.playerStats[i].typeOfPlayerDeath)
                {
                    fprintf(statsFile,"\n");
                    switch (universe.gameStats.playerStats[i].typeOfPlayerDeath)
                    {
                        case PLAYERKILLED_DROPPEDOUT    : fprintf(statsFile,strGetString(strStatsDeathByDropout));                  break;
                        case PLAYERKILLED_CAPTUREDSHIP  : fprintf(statsFile,strGetString(strStatsDeathByShipCapture));              break;
                        case PLAYERKILLED_LOSTMISSION   : fprintf(statsFile,strGetString(strStatsDeathByLossOfMission));            break;
                    }
                }
            }
            else
            {
                fprintf(statsFile,"\n");
                fprintf(statsFile,strGetString(strStatsDeathStillAlive));
            }

            switch(tpGameCreated.startingResources)
            {
                case MG_Small:
                    startingresources=resourceStartSmall;
                    break;
                case MG_Medium:
                    startingresources=resourceStartMedium;
                    break;
                case MG_Large:
                    startingresources=resourceStartLarge;
                    break;
            }

            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsStart)    ,startingresources);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,startingresources);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsCollected),universe.gameStats.playerStats[i].totalResourceUnitsCollected);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsCollected);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsSpent)    ,universe.gameStats.playerStats[i].totalResourceUnitsSpent);
                // check stats
                if(verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsSpent) == TRUE)
                    verifyGameStatsResUnitsTotals(statsFile,i);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsGiven)    ,universe.gameStats.playerStats[i].totalResourceUnitsGiven);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsGiven);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsReceived) ,universe.gameStats.playerStats[i].totalResourceUnitsRecieved);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsRecieved);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsInjected) ,universe.gameStats.playerStats[i].totalInjectedResources);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalInjectedResources);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsByBounties),universe.gameStats.playerStats[i].totalResourceUnitsViaBounties);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalResourceUnitsViaBounties);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsRUsGenerated),universe.gameStats.playerStats[i].totalRegeneratedResourceUnits);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.gameStats.playerStats[i].totalRegeneratedResourceUnits);
            fprintf(statsFile,"\n");    fprintf(statsFile,strGetString(strStatsCurrentRUs),universe.players[i].resourceUnits);
                // check stats
                verifyGameStatsResUnitsBounds(statsFile,i,universe.players[i].resourceUnits);
            fprintf(statsFile,"\n___________________\n");

            for (j=0;j<universe.numPlayers;j++)
            {
                if (j!=i)
                {
                    fprintf(statsFile,"\n");
                    fprintf(statsFile, strGetString(strStatsTotalDmgAgainstPlayer), j, universe.gameStats.playerStats[i].pointsOfDamageDoneToWho[j]);
                }
            }

            fprintf(statsFile,"\n\n");
            fprintf(statsFile,strGetString(strStatsTotalAcquiredShips),universe.gameStats.playerStats[i].totalShips);
                // check stats
                if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalShips,1) == TRUE)
                    verifyGameStatsShipsAcquiredTotals(statsFile,i);
            fprintf(statsFile,"\n\n");
            fprintf(statsFile,strGetString(strStatsTotalAcqShipsByType));
            fprintf(statsFile,"\n");

            for(j=0;j<TOTAL_NUM_SHIPS;j++)
            {
                if(universe.gameStats.playerStats[i].totalForEachShip[j] > 0)
                {
                    fprintf(statsFile,"\n%-40s:%d",ShipTypeToNiceStr(j),universe.gameStats.playerStats[i].totalForEachShip[j]);
                        // check stats
                        verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalForEachShip[j],1);
                }
            }
            fprintf(statsFile,"\n___________________\n\n\n");
            fprintf(statsFile,strGetString(strStatsTotalAcqShipsByClass));
            for(j=0;j<NUM_CLASSES;j++)
            {
                if(universe.gameStats.playerStats[i].totalForEachClass[j] > 0)
                {
                    fprintf(statsFile,"\n%-40s:%d",ShipClassToNiceStr(j),universe.gameStats.playerStats[i].totalForEachClass[j]);
                        // check stats
                        if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalForEachClass[j],1) == TRUE)
                            verifyGameStatsShipsAcquiredClassTotals(statsFile,i,j);
                }
            }
            fprintf(statsFile,"\n___________________\n\n");

            fprintf(statsFile,strGetString(strStatsTotalKills),universe.gameStats.playerStats[i].totalKills);
                // check stats
                if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalKills,universe.numPlayers) == TRUE)
                    verifyGameStatsShipsKilledTotals(statsFile,i);
            for(j=0;j<TOTAL_NUM_SHIPS;j++)
            {
                if(universe.gameStats.playerStats[i].totalKillsForEachShip[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t%d",ShipTypeToNiceStr(j),strGetString(strStatsKilled),universe.gameStats.playerStats[i].totalKillsForEachShip[j]);
                        // check stats
                        verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalKillsForEachShip[j],universe.numPlayers);
                }
            }
            for(j=0;j<NUM_CLASSES;j++)
            {
                if(universe.gameStats.playerStats[i].totalKillsForEachClass[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t%d",ShipClassToNiceStr(j),strGetString(strStatsKilled),universe.gameStats.playerStats[i].totalKillsForEachClass[j]);
                        // check stats
                        if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalKillsForEachClass[j],universe.numPlayers) == TRUE)
                            verifyGameStatsShipsKilledClassTotals(statsFile,i,j);
                }
            }
            fprintf(statsFile,"\n___________________\n\n");

            fprintf(statsFile,strGetString(strStatsTotalLosses),universe.gameStats.playerStats[i].totalLosses);
                // check stats
                if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalLosses,universe.numPlayers) == TRUE)
                    verifyGameStatsShipsLostTotals(statsFile,i);
            for(j=0;j<TOTAL_NUM_SHIPS;j++)
            {
                if(universe.gameStats.playerStats[i].totalLossesForEachShip[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t %d",ShipTypeToNiceStr(j),strGetString(strStatsLost),universe.gameStats.playerStats[i].totalLossesForEachShip[j]);
                        // check stats
                        verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalLossesForEachShip[j],universe.numPlayers);
                }
            }
            for(j=0;j<NUM_CLASSES;j++)
            {
                if(universe.gameStats.playerStats[i].totalLossesForEachClass[j] > 0)
                {
                    fprintf(statsFile,"\n%s %s\t\t %d",ShipClassToNiceStr(j),strGetString(strStatsLost),universe.gameStats.playerStats[i].totalLossesForEachClass[j]);
                        // check stats
                        if(verifyGameStatsShipsBounds(statsFile,i,universe.gameStats.playerStats[i].totalLossesForEachClass[j],universe.numPlayers) == TRUE)
                            verifyGameStatsShipsLostClassTotals(statsFile,i,j);
                }
            }
            fprintf(statsFile,"\n___________________\n");
            fprintf(statsFile,"\n___________________\n");
        }
    }

    fileClose(statsFH);
}


/* -----------------------------------------------------------------------------
   ShipSpecifics overlays for the per-player slot/cloak status arrays.

   Several ship types keep a per-player "slot status" array (sdword indexed by
   playerIndex) plus, for the cloaking ships, a small cloak state machine. The
   storage lives inside the post-Ship ShipSpecifics payload (begins at Ship+0x638)
   and at a type-specific absolute offset confirmed against cataclysm.exe:

     sMimic / sMCV / p1StandardCorvette : slotAvailability in ShipModuleSlotRuntime
                                          (Ship+0x6E4), so it uses the named runtime
                                          overlay rather than a local struct.
     bCloakedFighter                    : cloak @ Ship+0x650, slotStatus @ Ship+0x668
     bIonArrayFrigate / p1IonArray      : cloak @ Ship+0x638, slotStatus @ Ship+0x650
     sLeech                             : slotStatus @ Ship+0x654 (no cloak)

   The cloak state machine is six 32-bit fields followed immediately by the
   per-player slotStatus array; shipCloakUpdate consumes it as real32[] with the
   first word reinterpreted as the sdword state (see shipGetCloakStateStorage).
   ----------------------------------------------------------------------------- */
typedef struct
{
    sdword state;                               // cloak state machine phase (0..3)
    real32 charge;                              // cloakState[1]
    real32 chargeRate;                          // cloakState[2]
    real32 dischargeRate;                       // cloakState[3]
    real32 reappearTime;                        // cloakState[4]
    real32 blend;                               // cloakState[5]
    sdword slotStatus[MAX_MULTIPLAYER_PLAYERS]; // per-player slot/detection status
} ShipCloakSpec;
static_assert(offsetof(ShipCloakSpec, slotStatus) == 0x18);

typedef struct
{
    ShipCloakSpec cloak;                        // Ship+0x638
} IonArrayFrigateCloakOverlay;
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(IonArrayFrigateCloakOverlay, cloak) == 0x638);
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(IonArrayFrigateCloakOverlay, cloak) + offsetof(ShipCloakSpec, slotStatus) == 0x650);

typedef struct
{
    ubyte pad[0x650 - 0x638];
    ShipCloakSpec cloak;                        // Ship+0x650
} CloakedFighterCloakOverlay;
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(CloakedFighterCloakOverlay, cloak) == 0x650);
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(CloakedFighterCloakOverlay, cloak) + offsetof(ShipCloakSpec, slotStatus) == 0x668);

typedef struct
{
    ubyte pad[0x654 - 0x638];
    sdword slotStatus[MAX_MULTIPLAYER_PLAYERS]; // Ship+0x654
} LeechSlotStatusOverlay;
static_assert(offsetof(Ship, ShipSpecifics) + offsetof(LeechSlotStatusOverlay, slotStatus) == 0x654);

static sdword *shipGetSlotStatusStorage(Ship *ship)
{
   switch (ship->shiptype)
   {
       case sMimic:
       case sMCV:
       case p1StandardCorvette:
           return (sdword *)shipModuleSlotRuntime(ship)->slotAvailability;
       case sLeech:
           return ((LeechSlotStatusOverlay *)ship->ShipSpecifics)->slotStatus;
       case bCloakedFighter:
           return ((CloakedFighterCloakOverlay *)ship->ShipSpecifics)->cloak.slotStatus;
       case bIonArrayFrigate:
       case p1IonArrayFrigate:
           return ((IonArrayFrigateCloakOverlay *)ship->ShipSpecifics)->cloak.slotStatus;
       default:
           return NULL;
   }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x53F9F0
sdword shipGetSlotStatus(Ship *ship, sdword slotIndex)
{
   sdword *slotStatuses;

   if (slotIndex < 0 || slotIndex >= MAX_MULTIPLAYER_PLAYERS)
   {
       return TRUE;
   }
   if ((ship->staticinfo->shipFlags & 0x20000) == 0)
   {
       return TRUE;
   }

   slotStatuses = shipGetSlotStatusStorage(ship);
   if (slotStatuses == NULL)
   {
       return TRUE;
   }

   return slotStatuses[slotIndex];
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x420EC0
sdword __cdecl univIsObjFriendly(SpaceObj *obj, Player *player)
{
   Player *ownerPlayer;

   if (obj == NULL || player == NULL || obj->objtype != OBJ_ShipType)
   {
       return FALSE;
   }
   if (((Ship *)obj)->shiptype == dCrystal0 || ((Ship *)obj)->shiptype == dCrystal1)
   {
       return FALSE;
   }

   ownerPlayer = ((Ship *)obj)->playerowner;
   if (ownerPlayer == NULL)
   {
       return FALSE;
    }
   if (ownerPlayer == player || (obj->attributes & 0x100) != 0)
   {
       return TRUE;
   }

   return (player->Allies & (1 << ownerPlayer->playerIndex)) != 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x53F620
bool32 __cdecl univPlayerHasShipType(ShipType shiptype, Player *player)
{
   Node *node;
   Ship *ship;

   if (player == NULL || player->shiptotals[shiptype] == 0)
   {
       return FALSE;
   }

   for (node = universe.ShipList.head; node != NULL; node = node->next)
   {
       ship = (Ship *)node->structptr;
       if (ship->shiptype == shiptype &&
           ship->playerowner == player &&
           (ship->specialFlags4 & 8) == 0)
       {
           return TRUE;
       }
   }

   return FALSE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B97E0
sdword univGetShipRace(SpaceObj *obj)
{
   const Ship *ship;

   if (obj == NULL || obj->objtype != OBJ_ShipType)
   {
       return -1;
   }
   ship = (const Ship *)obj;
   if ((ship->flags2 & BIT3) == 0)
   {
       return ship->shiptype;
   }
   return shipModuleSlotRuntimeConst(ship)->activeBuildTarget.buildTargetType;
}

static real32 *shipGetCloakStateStorage(Ship *ship);

enum
{
    COMM_Cloak_CloakingOn_Retail = 67111049,
    COMM_Cloak_InsufficientPower_Retail = 67111051,
    COMM_Cloak_Decloak_Retail = 67111053,
};

// ============================================================================
// Decompiled from cataclysm.exe @ 0x435F10
void shipInitCloakState(Ship *ship, real32 *cloakState, real32 cloakDrainPerTick, real32 decloakDrainPerTick)
{
    ship->flags |= 0x200u;
    cloakState[0] = 0.0f;
    cloakState[1] = 1000.0f;
    cloakState[2] = cloakDrainPerTick;
    cloakState[3] = decloakDrainPerTick;
    cloakState[4] = 0.0f;
    cloakState[5] = 0.0f;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x435F50
BOOL shipToggleCloak(Ship *ship)
{
    real32 *cloakState;
    CommandToDo *command;
    sword orderType;
    real32 randomDelay;

    cloakState = shipGetCloakStateStorage(ship);
    dbgAssert(cloakState != NULL);
    if (kbSuppressNextSpecialActivate)
    {
        kbSuppressNextSpecialActivate = 0;
        return TRUE;
    }

    if (*(sdword *)cloakState != 0)
    {
        if (*(sdword *)cloakState == 1)
        {
            ship->flags &= ~SOF_Cloaking;
            *(sdword *)cloakState = 2;
            randomDelay = (frandombetween(0.0f, 1.0f) * -0.005f) + universe.totaltimeelapsed;
            cloakState[5] = 1.0f;
            cloakState[4] = randomDelay;
            soundEventPlay(ship, 2066, 0);
            speechEventQueue(ship, COMM_Cloak_Decloak_Retail, -1, -1, -1, -1, -1, 0.0f, -1);
        }

        return TRUE;
    }

    command = ship->command;
    if (command != NULL)
    {
        orderType = command->ordertype.order;
        if (orderType == COMMAND_ATTACK || orderType == COMMAND_LAUNCHSHIP)
        {
            return TRUE;
        }
    }

    if (cloakState[1] < 100.0f)
    {
        speechEventQueue(ship, COMM_Cloak_InsufficientPower_Retail, -1, -1, -1, -1, -1, 0.0f, -1);
        return TRUE;
    }

    *(sdword *)cloakState = 3;
    randomDelay = (frandombetween(0.0f, 1.0f) * -0.005f) + universe.totaltimeelapsed;
    cloakState[5] = 0.0f;
    cloakState[4] = randomDelay;
    soundEventPlay(ship, 2065, 0);
    speechEventQueue(ship, COMM_Cloak_CloakingOn_Retail, -1, -1, -1, -1, -1, 0.0f, -1);
    return TRUE;
}

bool32 shipGetCloakChargeFraction(Ship *ship, real32 *fraction)
{
    real32 *cloakState;

    if (fraction == NULL)
    {
        return FALSE;
    }

    cloakState = shipGetCloakStateStorage(ship);
    if (cloakState == NULL)
    {
        return FALSE;
    }

    *fraction = cloakState[1] / 1000.0f;
    if (*fraction < 0.0f)
    {
        *fraction = 0.0f;
    }
    else if (*fraction > 1.0f)
    {
        *fraction = 1.0f;
    }
    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x436130
bool32 shipGetCloakTransitionBlend(Ship *ship, real32 *blend)
{
    real32 *cloakState;

    if (blend == NULL)
    {
        return FALSE;
    }

    cloakState = shipGetCloakStateStorage(ship);
    if (cloakState == NULL)
    {
        return FALSE;
    }

    if (*(sdword *)cloakState >= 2 && *(sdword *)cloakState <= 3)
    {
        *blend = cloakState[5];
        return TRUE;
    }

    return FALSE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x436110
BOOL univIsShipCloaked(SpaceObj *obj)
{
   return obj != NULL &&
          (obj->flags & SOF_Cloaked) != 0 &&
          (obj->flags & SOF_Cloaking) != 0;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x514A30
real32 univGetMaxHealth(Ship *ship, bool32 isEnemy)
{
   Player *player;
   real32 sensorRadius;

   player = isEnemy ? &universe.players[ship->playerowner->playerIndex] : ship->playerowner;
   sensorRadius = ship->staticinfo->fowRadius;
   if (rmTechIsResearched(player, 28))
   {
       sensorRadius *= 1.3f;
   }
   if (singlePlayerGame && singlePlayerGameInfo.currentMission == 6 && ship->shiptype != 0)
   {
       sensorRadius *= 0.125f;
   }

   return sensorRadius;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5C6E50
BOOL univObjIsDying(SpaceObj *obj)
{
   if (obj == NULL || obj->objtype != OBJ_ShipType)
   {
       return FALSE;
   }
   {
       /* Dying ships flag their state in the first two words of the active
          ShipSpecifics payload (Ship+0x638 / Ship+0x63C); meaning is layout
          dependent, matching the retail IsShipInfected ShipSpecifics access. */
       const sdword *specifics = (const sdword *)((const Ship *)obj)->ShipSpecifics;
       return specifics[0] == 1 && *(const real32 *)&specifics[1] == 0.0f;
   }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47ED70
sdword univSetCurrentSensorCoverage(sdword sensorCoverage)
{
   sdword previousSensorCoverage;

   previousSensorCoverage = currentSensorCoverage;
   currentSensorCoverage = sensorCoverage;
   return previousSensorCoverage;
}

static bool32 univClipAxis(real32 minValue, real32 maxValue, real32 srcValue, real32 deltaValue, real32 *tEnter, real32 *tLeave)
{
   real32 axisEnter;
   real32 axisLeave;
   real32 inverseDelta;

   if (deltaValue == 0.0f)
   {
       return srcValue >= minValue && srcValue <= maxValue;
   }

   inverseDelta = 1.0f / deltaValue;
   axisEnter = (minValue - srcValue) * inverseDelta;
   axisLeave = (maxValue - srcValue) * inverseDelta;
   if (axisEnter > axisLeave)
   {
       real32 temp;

       temp = axisEnter;
       axisEnter = axisLeave;
       axisLeave = temp;
   }
   if (axisEnter > *tEnter)
   {
       *tEnter = axisEnter;
   }
   if (axisLeave < *tLeave)
   {
       *tLeave = axisLeave;
   }

   return *tEnter <= *tLeave;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4E76B0
sdword univClipPointToWorldBox(vector *src, vector *dst)
{
   real32 maxX;
   real32 maxY;
   real32 maxZ;
   real32 tEnter;
   real32 tLeave;
   real32 clipT;
   vector delta;
   bool32 srcInside;

   maxX = smUniverseSizeX - 8000.0f;
   maxY = smUniverseSizeY - 8000.0f;
   maxZ = smUniverseSizeZ - 8000.0f;

   if (dst->x >= -maxX && dst->x <= maxX &&
       dst->y >= -maxY && dst->y <= maxY &&
       dst->z >= -maxZ && dst->z <= maxZ)
   {
       return TRUE;
   }

   delta.x = dst->x - src->x;
   delta.y = dst->y - src->y;
   delta.z = dst->z - src->z;
   tEnter = 0.0f;
   tLeave = 1.0f;

   if (!univClipAxis(-maxX, maxX, src->x, delta.x, &tEnter, &tLeave) ||
       !univClipAxis(-maxY, maxY, src->y, delta.y, &tEnter, &tLeave) ||
       !univClipAxis(-maxZ, maxZ, src->z, delta.z, &tEnter, &tLeave))
   {
       *dst = *src;
       return FALSE;
   }

   srcInside = src->x >= -maxX && src->x <= maxX &&
               src->y >= -maxY && src->y <= maxY &&
               src->z >= -maxZ && src->z <= maxZ;
   clipT = srcInside ? tLeave : tEnter;
   if (clipT < 0.0f)
   {
       clipT = 0.0f;
   }
   if (clipT > 1.0f)
   {
       clipT = 1.0f;
   }

   dst->x = src->x + delta.x * clipT;
   dst->y = src->y + delta.y * clipT;
   dst->z = src->z + delta.z * clipT;
   return TRUE;
}

bool32 shipStaticCanResearch(ShipStaticInfo *shipstatic);

sdword shipCanResearch(Ship *ship, sdword requireIdleResearchShip)
{
    if (!shipStaticCanResearch(ship->staticinfo))
    {
        return FALSE;
    }
    if (!singlePlayerGame && !bitTest(tpGameCreated.flag, MG_HarvestinEnabled))
    {
        return FALSE;
    }
    if (!requireIdleResearchShip)
    {
        return TRUE;
    }
    if (ship->specialFlags3 == 0 &&
        (ship->shiptype == sWorker || ship->shiptype == bWorker))
    {
        return TRUE;
    }
    return FALSE;
}

bool32 shipStaticCanResearch(ShipStaticInfo *shipstatic)
{
    return shipstatic->shipclass == CLASS_Resource &&
           shipstatic->harvestAmount > 0;
}
int *universeSetMissionShipAvailability(void)
{
    ubyte availability[NUM_RACES][MAX_MULTIPLAYER_PLAYERS];
    sdword playerIndex;
    sdword race;
    ShipType shiptype;

#define AVAIL(_index) (((ubyte *)availability)[(_index)])

    memset(availability, 0, sizeof(availability));
    for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++)
    {
        AVAIL(universe.players[playerIndex].race * MAX_MULTIPLAYER_PLAYERS + playerIndex) = 1;
    }

    if (tutorial == 1)
    {
        AVAIL(0) = 1;
        AVAIL(4) = 1;
        AVAIL(24) = 1;
        AVAIL(26) = 1;
        AVAIL(48) = 1;
    }
    else if (!singlePlayerGame)
    {
        AVAIL(22) = 1;
        AVAIL(26) = 1;
        AVAIL(37) = 1;
        AVAIL(45) = 1;
        AVAIL(48) = 1;
    }
    else
    {
        AVAIL(0) = 1;
        AVAIL(1) = 1;
        AVAIL(9) = 1;
        switch (singlePlayerGameInfo.currentMission)
        {
            case 1:
                AVAIL(20) = 1;
                AVAIL(22) = 1;
                AVAIL(23) = 1;
                AVAIL(26) = 1;
                AVAIL(48) = 1;
                break;
            case 2:
                AVAIL(20) = 1;
                AVAIL(37) = 1;
                AVAIL(48) = 1;
                break;
            case 3:
                AVAIL(32) = 1;
                AVAIL(37) = 1;
                AVAIL(29) = 1;
                AVAIL(48) = 1;
                AVAIL(49) = 1;
                break;
            case 4:
                AVAIL(8) = 1;
                AVAIL(29) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(37) = 1;
                AVAIL(48) = 1;
                break;
            case 5:
                AVAIL(20) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(45) = 1;
                break;
            case 6:
                AVAIL(9) = 1;
                AVAIL(20) = 1;
                AVAIL(24) = 1;
                AVAIL(26) = 1;
                AVAIL(32) = 1;
                AVAIL(42) = 1;
                AVAIL(48) = 1;
                AVAIL(49) = 1;
                break;
            case 7:
                AVAIL(1) = 1;
                AVAIL(20) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(49) = 1;
                AVAIL(51) = 1;
                break;
            case 8:
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(27) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(48) = 1;
                break;
            case 9:
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(48) = 1;
                break;
            case 10:
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(37) = 1;
                AVAIL(49) = 1;
                AVAIL(48) = 1;
                break;
            case 11:
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(32) = 1;
                AVAIL(33) = 1;
                AVAIL(40) = 1;
                AVAIL(49) = 1;
                break;
            case 12:
                AVAIL(5) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(29) = 1;
                AVAIL(37) = 1;
                AVAIL(48) = 1;
                AVAIL(49) = 1;
                break;
            case 13:
                AVAIL(16) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(40) = 1;
                AVAIL(48) = 1;
                break;
            case 14:
                AVAIL(16) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(37) = 1;
                AVAIL(48) = 1;
                AVAIL(49) = 1;
                break;
            case 15:
                AVAIL(16) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(45) = 1;
                AVAIL(53) = 1;
                break;
            case 16:
                AVAIL(16) = 1;
                AVAIL(20) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(49) = 1;
                AVAIL(48) = 1;
                break;
            case 17:
                AVAIL(16) = 1;
                AVAIL(24) = 1;
                AVAIL(25) = 1;
                AVAIL(26) = 1;
                AVAIL(27) = 1;
                AVAIL(45) = 1;
                AVAIL(48) = 1;
                AVAIL(49) = 1;
                break;
            default:
                break;
        }
    }

    for (race = 0; race < NUM_RACES; race++)
    {
        memcpy(missileStaticInfos[race].teamColor, availability[race], sizeof(missileStaticInfos[race].teamColor));
        memcpy(mineStaticInfos[race].teamColor, availability[race], sizeof(mineStaticInfos[race].teamColor));
    }
    for (shiptype = 0; shiptype < TOTAL_NUM_SHIPS; shiptype++)
    {
        race = GetValidRaceForShipType(shiptype);
        memcpy(shipStaticInfos[shiptype].teamColor, availability[race], sizeof(shipStaticInfos[shiptype].teamColor));
    }
    for (shiptype = 0; shiptype < NUM_DERELICTTYPES; shiptype++)
    {
        memcpy(derelictStaticInfos[shiptype].teamColor, availability[RACE_Derelict], sizeof(derelictStaticInfos[shiptype].teamColor));
    }

#undef AVAIL
    return NULL;
}



Ship *univFindNearestShipWithConstraints(Ship *ship, bool32 (__cdecl *constraint)(void *))
{
    Node *node;
    Ship *closestShip;
    real32 closestDistanceSquared;

    closestShip = NULL;
    closestDistanceSquared = 1.0e20f;
    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *candidate;
        real32 dx, dy, dz;
        real32 distanceSquared;

        candidate = (Ship *)node->structptr;
        if (constraint != NULL && !constraint(candidate))
        {
            continue;
        }

        dx = candidate->posinfo.position.x - ship->posinfo.position.x;
        dy = candidate->posinfo.position.y - ship->posinfo.position.y;
        dz = candidate->posinfo.position.z - ship->posinfo.position.z;
        distanceSquared = dx * dx + dy * dy + dz * dz;
        if (distanceSquared < closestDistanceSquared)
        {
            closestDistanceSquared = distanceSquared;
            closestShip = candidate;
        }
    }

    return closestShip;
}

static ubyte shipGetSlotAltRace(Ship *ship)
{
    return (ubyte)shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetType;
}

static sdword shipGetModuleSlotValue(Ship *ship)
{
    return (sdword)shipModuleSlotRuntime(ship)->slotAvailability[0];
}

bool32 shipIsModuleSlotActive(Ship *ship, Player *player)
{
    if (ship->objtype != OBJ_ShipType ||
        (ship->flags2 & BIT3) != BIT3 ||
        (ubyte)shipModuleSlotRuntime(ship)->state != 1 ||
        shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetType == -1)
    {
        return FALSE;
    }

    return !shipGetSlotStatus(ship, player->playerIndex) &&
           shipModuleSlotRuntime(ship)->activeBuildTarget.slotIndex == player->playerIndex;
}

static sdword shipSetSlotStatusNotifyEnemyPresence(Ship *ship, sdword playerIndex)
{
    if (playerIndex != universe.curPlayerIndex)
    {
        return playerIndex;
    }

    return !allianceArePlayersAllied(ship->playerowner, &universe.players[playerIndex]);
}

bool32 univShipModuleSlotIsEligible(Ship *ship, Player *player)
{
    return ship->objtype == OBJ_ShipType &&
           (ship->flags2 & BIT3) == BIT3 &&
           !shipGetSlotStatus(ship, player->playerIndex) &&
           (ubyte)shipModuleSlotRuntime(ship)->state == 1 &&
           shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetType == -1 &&
           ship->health / ship->staticinfo->maxhealth > 0.4f;
}

bool32 univShipModuleSlotIsEligibleUpgrade(Ship *ship, Player *player)
{
    return ship->objtype == OBJ_ShipType &&
           (ship->flags2 & BIT3) == BIT3 &&
           !shipGetSlotStatus(ship, player->playerIndex) &&
           (ubyte)shipModuleSlotRuntime(ship)->state == 1 &&
           shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetType > 97 &&
           ship->health / ship->staticinfo->maxhealth > 0.4f;
}

sdword shipSetSlotStatus(Ship *ship, sdword playerIndex, sdword status, sdword skipEnemyCheck)
{
    sdword result;
    Player *player;

    player = &universe.players[playerIndex];
    result = shipGetSlotStatus(ship, playerIndex);

    switch (ship->shiptype)
    {
        case sMimic:
        case sMCV:
        case p1StandardCorvette:
            if (!result && status == 1)
            {
                result = skipEnemyCheck;
                if (!skipEnemyCheck)
                {
                    result = shipHasModuleSlotState(ship);
                    if (result)
                    {
                        result = shipSetSlotStatusNotifyEnemyPresence(ship, playerIndex);
                    }
                }
            }
            ((sdword *)shipModuleSlotRuntime(ship)->slotAvailability)[playerIndex] = status;
            if (!status)
            {
                if (shipIsModuleSlotActive(ship, player) ||
                    univShipModuleSlotIsEligible(ship, player) ||
                    univShipModuleSlotIsEligibleUpgrade(ship, player))
                {
                    result = 1;
                    ((sdword *)shipModuleSlotRuntime(ship)->slotAvailability)[playerIndex] = 0;
                }
                else
                {
                    ((sdword *)shipModuleSlotRuntime(ship)->slotAvailability)[playerIndex] = 1;
                }
            }
            break;

        case sLeech:
            if (!result && status == 1)
            {
                result = skipEnemyCheck;
                if (!skipEnemyCheck)
                {
                    result = shipSetSlotStatusNotifyEnemyPresence(ship, playerIndex);
                }
            }
            ((LeechSlotStatusOverlay *)ship->ShipSpecifics)->slotStatus[playerIndex] = status;
            break;

        case bCloakedFighter:
            if (!result && status == 1)
            {
                result = skipEnemyCheck;
                if (!skipEnemyCheck)
                {
                    result = univIsShipCloaked((SpaceObj *)ship);
                    if (result)
                    {
                        result = shipSetSlotStatusNotifyEnemyPresence(ship, playerIndex);
                    }
                }
            }
            ((CloakedFighterCloakOverlay *)ship->ShipSpecifics)->cloak.slotStatus[playerIndex] = status;
            break;

        case bIonArrayFrigate:
        case p1IonArrayFrigate:
            if (!result && status == 1)
            {
                result = skipEnemyCheck;
                if (!skipEnemyCheck)
                {
                    result = univIsShipCloaked((SpaceObj *)ship);
                    if (result)
                    {
                        result = shipSetSlotStatusNotifyEnemyPresence(ship, playerIndex);
                    }
                }
            }
            ((IonArrayFrigateCloakOverlay *)ship->ShipSpecifics)->cloak.slotStatus[playerIndex] = status;
            break;

        default:
            break;
    }

    return result;
}

void shipResetSlotStatusesForAllPlayers(Ship *ship)
{
    sdword playerIndex;
    sdword status;

    for (playerIndex = 0; playerIndex < MAX_MULTIPLAYER_PLAYERS; playerIndex++)
    {
        switch (ship->shiptype)
        {
            case sMimic:
            case sMCV:
            case bCloakedFighter:
            case bIonArrayFrigate:
            case p1IonArrayFrigate:
            case p1StandardCorvette:
                status = 1;
                break;
            case sLeech:
                status = 0;
                break;
            default:
                continue;
        }
        shipSetSlotStatus(ship, playerIndex, status, 1);
    }
}

static bool32 shipOwnedByPlayer(Ship *ship, Player *player)
{
    return ship->playerowner == player;
}

void univFreeSpaceObjContents(SpaceObj *obj)
{
    univFreeAttackTargets((Ship *)obj);

    switch (obj->objtype)
    {
        case OBJ_AsteroidType:
            if (((Asteroid *)obj)->attributes != 0)
            {
                memFree((ubyte *)((Asteroid *)obj)->attributes);
                ((Asteroid *)obj)->attributes = 0;
            }
            break;

        case OBJ_NebulaType:
            if (((Nebula *)obj)->attributes != 0)
            {
                memFree((ubyte *)((Nebula *)obj)->attributes);
                ((Nebula *)obj)->attributes = 0;
            }
            if (((Nebula *)obj)->stub != NULL)
            {
                nebDeleteChunk(((Nebula *)obj)->stub);
                ((Nebula *)obj)->stub = NULL;
            }
            break;

        case OBJ_GasType:
            if (((GasCloud *)obj)->attributes != 0)
            {
                memFree((ubyte *)((GasCloud *)obj)->attributes);
                ((GasCloud *)obj)->attributes = 0;
            }
            break;

        case OBJ_DustType:
            if (((DustCloud *)obj)->attributes != 0)
            {
                memFree((ubyte *)((DustCloud *)obj)->attributes);
                ((DustCloud *)obj)->attributes = 0;
            }
            if (((DustCloud *)obj)->stub != NULL)
            {
                cloudDeleteSystem(((DustCloud *)obj)->stub);
                ((DustCloud *)obj)->stub = NULL;
            }
            break;

        default:
            break;
    }

    if ((obj->flags & SOF_StaticInfoIsDynamicCata) != 0)
    {
        memFree((ubyte *)obj->staticinfo);
        obj->staticinfo = NULL;
    }
}















#define CATA_UNIV_COUNT_TABLE_ENTRIES (MAX_MULTIPLAYER_PLAYERS * TOTAL_NUM_SHIPS)
#define CATA_UNIV_COUNT_AUX_ENTRIES   96

sdword cataShipTypeCounts[CATA_UNIV_COUNT_TABLE_ENTRIES];
sdword cataReplacementShipTypeCounts[CATA_UNIV_COUNT_TABLE_ENTRIES];
static sdword cataCountTableAux0[CATA_UNIV_COUNT_AUX_ENTRIES];
static sdword cataCountTableAux1[CATA_UNIV_COUNT_AUX_ENTRIES];
static sdword cataCountTableInfo = 0;

sdword overlayPauseActive = 0;

void univSaveCountTables(void)
{
    SaveChunk *chunk;

    chunk = CreateChunk(BASIC_STRUCTURE | SAVE_UNIVSHIPTYPECOUNTS, sizeof(cataShipTypeCounts), cataShipTypeCounts);
    SaveThisChunk(chunk);
    memFree(chunk);

    chunk = CreateChunk(BASIC_STRUCTURE | SAVE_UNIVREPLACEMENTCOUNTS, sizeof(cataReplacementShipTypeCounts), cataReplacementShipTypeCounts);
    SaveThisChunk(chunk);
    memFree(chunk);

    chunk = CreateChunk(BASIC_STRUCTURE | SAVE_UNIVCOUNTAUX0, sizeof(cataCountTableAux0), cataCountTableAux0);
    SaveThisChunk(chunk);
    memFree(chunk);

    chunk = CreateChunk(BASIC_STRUCTURE | SAVE_UNIVCOUNTAUX1, sizeof(cataCountTableAux1), cataCountTableAux1);
    SaveThisChunk(chunk);
    memFree(chunk);

    SaveInfoNumber(cataCountTableInfo);
}

void univLoadCountTables(void)
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, (BASIC_STRUCTURE | SAVE_UNIVSHIPTYPECOUNTS));
    dbgAssert(chunk->contentsSize <= sizeof(cataShipTypeCounts));
    memcpy(cataShipTypeCounts, chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, (BASIC_STRUCTURE | SAVE_UNIVREPLACEMENTCOUNTS));
    dbgAssert(chunk->contentsSize <= sizeof(cataReplacementShipTypeCounts));
    memcpy(cataReplacementShipTypeCounts, chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, (BASIC_STRUCTURE | SAVE_UNIVCOUNTAUX0));
    dbgAssert(chunk->contentsSize <= sizeof(cataCountTableAux0));
    memcpy(cataCountTableAux0, chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, (BASIC_STRUCTURE | SAVE_UNIVCOUNTAUX1));
    dbgAssert(chunk->contentsSize <= sizeof(cataCountTableAux1));
    memcpy(cataCountTableAux1, chunkContents(chunk), chunk->contentsSize);
    memFree(chunk);

    cataCountTableInfo = LoadInfoNumber();
    univResetMultiplayerAutosaveState();
}




void shipUpdateSlotStatusesForAllPlayers(Ship *ship)
{
    sdword slotStatus[MAX_MULTIPLAYER_PLAYERS];
    Node *head;
    sdword playerIndex;

    memset(slotStatus, 0, sizeof(slotStatus));
    for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++)
    {
        if (allianceArePlayersAllied(ship->playerowner, &universe.players[playerIndex]) &&
            (!singlePlayerGame ||
             singlePlayerGameInfo.currentMission != 14 ||
             ship->playerowner->playerIndex != 1))
        {
            slotStatus[playerIndex] = TRUE;
        }
    }

    for (head = universe.ShipList.head; head != NULL; head = head->next)
    {
        Ship *candidate = (Ship *)head->structptr;

        if ((candidate->staticinfo->shipFlags & 0x10000) &&
            ShipHasUpgrade(candidate, RESEARCH_UPGRADE_MASK_ADVANCED_SENSORS) &&
            !slotStatus[candidate->playerowner->playerIndex])
        {
            real32 range = univGetMaxHealth(candidate, FALSE);
            real32 x = ship->posinfo.position.x - candidate->posinfo.position.x;
            real32 y = ship->posinfo.position.y - candidate->posinfo.position.y;
            real32 z = ship->posinfo.position.z - candidate->posinfo.position.z;

            if (x * x + y * y + z * z < range * range)
            {
                slotStatus[candidate->playerowner->playerIndex] = TRUE;
            }
        }
    }

    for (playerIndex = 0; playerIndex < universe.numPlayers; playerIndex++)
    {
        if (slotStatus[playerIndex] || ship->shiptype == sLeech)
        {
            shipSetSlotStatus(ship, playerIndex, slotStatus[playerIndex], FALSE);
        }
    }
}

static real32 *shipGetCloakStateStorage(Ship *ship)
{
    if (ship->shiptype == bCloakedFighter)
    {
        return (real32 *)&((CloakedFighterCloakOverlay *)ship->ShipSpecifics)->cloak;
    }
    if (ship->shiptype == bIonArrayFrigate || ship->shiptype == p1IonArrayFrigate)
    {
        return (real32 *)&((IonArrayFrigateCloakOverlay *)ship->ShipSpecifics)->cloak;
    }
    return NULL;
}

void shipCloakUpdate(Ship *ship)
{
    real32 *cloakState = shipGetCloakStateStorage(ship);
    CommandToDo *command;
    sdword playerIndex;
    const real32 cloakChargeLimit = 1000.0f;
    const real32 cloakBlendStep = 0.05f;
    const real32 cloakDelayScale = 9.313226e-08f;
    const real32 cloakDelayFactor = -0.005f;

    if (cloakState == NULL)
    {
        return;
    }

    switch (*(sdword *)cloakState)
    {
        case 0:
            if (cloakState[1] < cloakChargeLimit)
            {
                cloakState[1] += cloakState[2];
                if (cloakState[1] >= cloakChargeLimit)
                {
                    cloakState[1] = 1000.0f;
                }
            }
            break;

        case 1:
            cloakState[1] -= cloakState[3];
            if (cloakState[1] <= 0.0f)
            {
                double randomExponent = (double)ranRandom(1) * cloakDelayScale * cloakDelayFactor * 1.442695040888963387;

                ship->flags &= ~SOF_Cloaking;
                *(sdword *)cloakState = 2;
                cloakState[5] = 1.0f;
                cloakState[1] = 0.0f;
                cloakState[4] = (real32)(1.0
                    - pow(2.0, randomExponent)
                    + universe.totaltimeelapsed);
                soundEventPlay(ship, 2066, 0);
            }
            break;

        case 2:
            if (universe.totaltimeelapsed > cloakState[4])
            {
                cloakState[5] -= cloakBlendStep;
                if (cloakState[5] <= 0.0f)
                {
                    ship->flags &= ~SOF_Cloaked;
                    ship->shipDeCloakTime = universe.totaltimeelapsed;
                    *(sdword *)cloakState = 0;
                    cloakState[5] = 0.0f;
                    for (playerIndex = 0; playerIndex < MAX_MULTIPLAYER_PLAYERS; playerIndex++)
                    {
                        shipSetSlotStatus(ship, playerIndex, TRUE, FALSE);
                    }
                }
            }
            break;

        case 3:
            command = ship->command;
            if (command != NULL &&
                (command->ordertype.order == COMMAND_ATTACK ||
                 command->ordertype.order == COMMAND_LAUNCHSHIP))
            {
                *(sdword *)cloakState = 2;
            }
            else if (universe.totaltimeelapsed > cloakState[4])
            {
                cloakState[5] += cloakBlendStep;
                if (cloakState[5] >= 1.0f)
                {
                    ship->flags |= SOF_Cloaked;
                    ship->flags |= SOF_Cloaking;
                    *(sdword *)cloakState = 1;
                    cloakState[5] = 1.0f;
                    for (playerIndex = 0; playerIndex < MAX_MULTIPLAYER_PLAYERS; playerIndex++)
                    {
                        shipSetSlotStatus(ship, playerIndex, FALSE, FALSE);
                    }
                    shipHasJustCloaked(ship);
                }
            }
            break;

        default:
            break;
    }
}
