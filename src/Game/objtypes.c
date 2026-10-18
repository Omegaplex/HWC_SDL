/*=============================================================================
    Name    : Objtypes.c
    Purpose : Provides conversions for datatypes found in objtypes.h

    Created 6/24/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "statscript.h"
#include "strings.h"
#include "objtypes.h"
#include "ClassDefs.h"

/*=============================================================================
    Public Data:
=============================================================================*/

ShipType FirstShipTypeOfRace[NUM_RACES] =
{
    SECT_FIRST_SHIP,
    BEAST_FIRST_SHIP,
    P1_FIRST_SHIP,
    P2_FIRST_SHIP,
    P3_FIRST_SHIP,
    TRADERS_FIRST_SHIP,
    DERELICT_FIRST_SHIP
};

ShipType LastShipTypeOfRace[NUM_RACES] =
{
    SECT_LAST_SHIP,
    BEAST_LAST_SHIP,
    P1_LAST_SHIP,
    P2_LAST_SHIP,
    P3_LAST_SHIP,
    TRADERS_LAST_SHIP,
    DERELICT_LAST_SHIP
};

uword NumShipTypesInRace[NUM_RACES] =
{
    TOTAL_SECT_SHIPS,
    TOTAL_BEAST_SHIPS,
    TOTAL_P1_SHIPS,
    TOTAL_P2_SHIPS,
    TOTAL_P3_SHIPS,
    TOTAL_TRADERS_SHIPS,
    TOTAL_DERELICT_SHIPS
};

/*=============================================================================
    Private Data:
=============================================================================*/

static NumStrXlate shiptypeinfo[] =
{
    { (uword)sRecon, "sRecon" },
    { (uword)sAcolyte, "sAcolyte" },
    { (uword)sMimic, "sMimic" },
    { (uword)sLeech, "sLeech" },
    { (uword)sACV, "sACV" },
    { (uword)sMCV, "sMCV" },
    { (uword)sRammingFrigate, "sRammingFrigate" },
    { (uword)sMultiBeamFrigate, "sMultiBeamFrigate" },
    { (uword)sHiveFrigate, "sHiveFrigate" },
    { (uword)sSwarmer, "sSwarmer" },
    { (uword)sDreadnought, "sDreadnought" },
    { (uword)sDreadnoughtCannon, "sDreadnoughtCannon" },
    { (uword)sCarrier, "sCarrier" },
    { (uword)sCarrierSupport, "sCarrierSupport" },
    { (uword)sWorker, "sWorker" },
    { (uword)sRepairDroid, "sRepairDroid" },
    { (uword)sProcessor, "sProcessor" },
    { (uword)sSentinel, "sSentinel" },
    { (uword)sMothership, "sMothership" },
    { (uword)sMothershipSpecial, "sMothershipSpecial" },
    { (uword)sMothershipEngineering, "sMothershipEngineering" },
    { (uword)sMothershipArmour, "sMothershipArmour" },
    { (uword)sMothershipMicro, "sMothershipMicro" },
    { (uword)sMothershipWeapons, "sMothershipWeapons" },
    { (uword)sMothershipDockingBay, "sMothershipDockingBay" },
    { (uword)sDestroyer, "sDestroyer" },
    { (uword)sMothershipSupport, "sMothershipSupport" },
    { (uword)sMothershipCommand, "sMothershipCommand" },
    { (uword)sMothershipEnginesLower, "sMothershipEnginesLower" },
    { (uword)sMothershipEnginesUpper, "sMothershipEnginesUpper" },
    { (uword)sMothershipLowerSpine, "sMothershipLowerSpine" },
    { (uword)sMothershipBigGun, "sMothershipBigGun" },
    { (uword)sMothershipOreCanArmour, "sMothershipOreCanArmour" },
    { (uword)bRecon, "bRecon" },
    { (uword)bAcolyte, "bAcolyte" },
    { (uword)bInterceptor, "bInterceptor" },
    { (uword)bCloakedFighter, "bCloakedFighter" },
    { (uword)bAttackBomber, "bAttackBomber" },
    { (uword)bACV, "bACV" },
    { (uword)bMultiGunCorvette, "bMultiGunCorvette" },
    { (uword)bHeavyCorvette, "bHeavyCorvette" },
    { (uword)bMultiBeamFrigate, "bMultiBeamFrigate" },
    { (uword)bHiveFrigate, "bHiveFrigate" },
    { (uword)bSwarmer, "bSwarmer" },
    { (uword)bIonArrayFrigate, "bIonArrayFrigate" },
    { (uword)bDFGFrigate, "bDFGFrigate" },
    { (uword)bHeavyCruiser, "bHeavyCruiser" },
    { (uword)bCarrier, "bCarrier" },
    { (uword)bRammingFrigate, "bRammingFrigate" },
    { (uword)bWorker, "bWorker" },
    { (uword)bRepairDroid, "bRepairDroid" },
    { (uword)bProcessor, "bProcessor" },
    { (uword)bSentinel, "bSentinel" },
    { (uword)bMothership, "bMothership" },
    { (uword)bMothershipDockingBay, "bMothershipDockingBay" },
    { (uword)bMothershipEnginesLower, "bMothershipEnginesLower" },
    { (uword)bSectMoship, "bSectMoship" },
    { (uword)bMothershipSupport, "bMothershipSupport" },
    { (uword)bCruiseMissile, "bCruiseMissile" },
    { (uword)bMissileCorvette, "bMissileCorvette" },
    { (uword)kAdvanceSupportFrigate, "kAdvanceSupportFrigate" },
    { (uword)kAttackBomber, "kAttackBomber" },
    { (uword)kCarrier, "kCarrier" },
    { (uword)kCloakGenerator, "kCloakGenerator" },
    { (uword)kGravWellGenerator, "kGravWellGenerator" },
    { (uword)kHeavyCorvette, "kHeavyCorvette" },
    { (uword)kHeavyCruiser, "kHeavyCruiser" },
    { (uword)kHeavyDefender, "kHeavyDefender" },
    { (uword)kHeavyInterceptor, "kHeavyInterceptor" },
    { (uword)kIonCannonFrigate, "kIonCannonFrigate" },
    { (uword)kMultiGunCorvette, "kMultiGunCorvette" },
    { (uword)kProximitySensor, "kProximitySensor" },
    { (uword)kSalCapCorvette, "kSalCapCorvette" },
    { (uword)kStandardDestroyer, "kStandardDestroyer" },
    { (uword)tAdvanceSupportFrigate, "tAdvanceSupportFrigate" },
    { (uword)tAttackBomber, "tAttackBomber" },
    { (uword)tCarrier, "tCarrier" },
    { (uword)tCloakGenerator, "tCloakGenerator" },
    { (uword)tDFGFrigate, "tDFGFrigate" },
    { (uword)tGravWellGenerator, "tGravWellGenerator" },
    { (uword)tHeavyCruiser, "tHeavyCruiser" },
    { (uword)tHeavyDefender, "tHeavyDefender" },
    { (uword)tHeavyInterceptor, "tHeavyInterceptor" },
    { (uword)tIonCannonFrigate, "tIonCannonFrigate" },
    { (uword)tMinelayerCorvette, "tMinelayerCorvette" },
    { (uword)tMissileDestroyer, "tMissileDestroyer" },
    { (uword)tMultiGunCorvette, "tMultiGunCorvette" },
    { (uword)tProximitySensor, "tProximitySensor" },
    { (uword)tResourceCollector, "tResourceCollector" },
    { (uword)tSalCapCorvette, "tSalCapCorvette" },
    { (uword)tStandardDestroyer, "tStandardDestroyer" },
    { (uword)tStandardFrigate, "tStandardFrigate" },
    { (uword)p1Outpost, "p1Outpost" },
    { (uword)p1Fighter, "p1Fighter" },
    { (uword)p1IonArrayFrigate, "p1IonArrayFrigate" },
    { (uword)p1MissileCorvette, "p1MissileCorvette" },
    { (uword)p1Mothership, "p1Mothership" },
    { (uword)p1StandardCorvette, "p1StandardCorvette" },
    { (uword)trFloatingCity, "trFloatingCity" },
    { (uword)trCargoBarge, "trCargoBarge" },
    { (uword)trMiningBase, "trMiningBase" },
    { (uword)trResearchStation, "trResearchStation" },
    { (uword)trJunkYardDawg, "trJunkYardDawg" },
    { (uword)trJunkYardHQ, "trJunkYardHQ" },
    { (uword)trGhostship, "trGhostship" },
    { (uword)trJunk_LGun, "trJunk_LGun" },
    { (uword)trJunk_SGun, "trJunk_SGun" },
    { (uword)trResearchStationBridge, "trResearchStationBridge" },
    { (uword)trResearchStationTower, "trResearchStationTower" },
    { (uword)dRKO, "dRKO" },
    { (uword)dBigGun, "dBigGun" },
    { (uword)dBigGunUnit, "dBigGunUnit" },
    { (uword)dSlipgateSmall, "dSlipgateSmall" },
    { (uword)dSlipgateMedium, "dSlipgateMedium" },
    { (uword)dSlipgateLarge, "dSlipgateLarge" },
    { (uword)dCrate, "dCrate" },
    { (uword)dJunk1PartA, "dJunk1PartA" },
    { (uword)dJunk1Shell, "dJunk1Shell" },
    { (uword)dJunk1Strut, "dJunk1Strut" },
    { (uword)dJunk2Shipwreck, "dJunk2Shipwreck" },
    { (uword)dJunk3Boiler, "dJunk3Boiler" },
    { (uword)dJunk3BoilerCasing, "dJunk3BoilerCasing" },
    { (uword)dMessagePod, "dMessagePod" },
    { (uword)dBuoy, "dBuoy" },
    { (uword)dFirstPlanet, "dFirstPlanet" },
    { (uword)dDeadPlanet, "dDeadPlanet" },
    { (uword)dCrystal0, "dCrystal0" },
    { (uword)dCrystal1, "dCrystal1" },
    { (uword)dBeastDockingBay, "dBeastDockingBay" },
    { (uword)dNISBuoy, "dNISBuoy" },
    { (uword)dBeastMothership, "dBeastMothership" },
    { (uword)dNomadMoon, "dNomadMoon" },
    { (uword)dNomadFieldEmitter, "dNomadFieldEmitter" },
    { (uword)dAcolyte, "dAcolyte" },
    { (uword)dPrisonShip, "dPrisonShip" },
    { (uword)dPrisonShipInfected, "dPrisonShipInfected" },
    { (uword)dSlipgateNode, "dSlipgateNode" },
    { (uword)dKushanCarrier, "dKushanCarrier" },
    { (uword)dIonArrayFrigate, "dIonArrayFrigate" },
    { (uword)dIonCannonFrigate, "dIonCannonFrigate" },
    { (uword)dEcclesiam, "dEcclesiam" },
    { (uword)dEcclesiamInfected, "dEcclesiamInfected" },
    { (uword)nisLightSource1, "nisLightSource1" },
    { (uword)nisLightSource2, "nisLightSource2" },
    { (uword)nisLensflareSource1, "nisLensflareSource1" },
    { 0, NULL }
};

static NumStrXlate shipraceinfo[] =
{
    { (uword)RACE_Sect, "RACE_SECT" },
    { (uword)RACE_Beast, "RACE_BEAST" },
    { (uword)RACE_Kushan, "RACE_KUSHAN" },
    { (uword)RACE_Taiidan, "RACE_TAIDAN" },
    { (uword)RACE_P1, "RACE_P1" },
    { (uword)RACE_Traders, "RACE_TRADERS" },
    { (uword)RACE_Derelict, "RACE_DERELICT" },
    { 0,NULL }
};

static NumStrXlate nisraceinfo[] =
{
    { (uword)RACE_Sect, "RACE_SECT" },
    { (uword)RACE_Beast, "RACE_BEAST" },
    { (uword)RACE_Kushan, "RACE_KUSHAN" },
    { (uword)RACE_Taiidan, "RACE_TAIDAN" },
    { (uword)RACE_P1, "RACE_P1" },
    { (uword)RACE_Traders, "RACE_TRADERS" },
    { (uword)RACE_Derelict, "RACE_DERELICT" },
    { (uword)NSR_Asteroid , "Asteroids" },
    { (uword)NSR_DustCloud, "DustClouds" },
    { (uword)NSR_GasCloud , "GasClouds" },
    { (uword)NSR_Nebula   , "Nebulae" },
    { (uword)NSR_Derelict , "Derelicts" },
    { (uword)NSR_Effect   , "ETG" },
    { (uword)NSR_Generic  , "Misc" },
    { (uword)NSR_Missile  , "Missile" },
    { 0,NULL }
};
static NumStrXlate shipclassinfo[] =
{
    { (uword)CLASS_Mothership,     str$(CLASS_Mothership) },
    { (uword)CLASS_HeavyCruiser,   str$(CLASS_HeavyCruiser) },
    { (uword)CLASS_Carrier,        str$(CLASS_Carrier) },
    { (uword)CLASS_Destroyer,      str$(CLASS_Destroyer) },
    { (uword)CLASS_Frigate,        str$(CLASS_Frigate) },
    { (uword)CLASS_Corvette,       str$(CLASS_Corvette) },
    { (uword)CLASS_Fighter,        str$(CLASS_Fighter) },
    { (uword)CLASS_Resource,       str$(CLASS_Resource) },
    { (uword)CLASS_NonCombat,      str$(CLASS_NonCombat) },
    { (uword)CLASS_MonsterShip,    str$(CLASS_MonsterShip) },
    { (uword)CLASS_Component,      str$(CLASS_Component) },
    { (uword)CLASS_Micro,          str$(CLASS_Micro) },
    { (uword)CLASS_Mine,           str$(CLASS_Mine) },
    { 0,NULL }
};

/*static NumStrXlate shipclassniceinfo[] =
{
    { (uword)CLASS_Mothership,     "MOTHERSHIP"},
    { (uword)CLASS_HeavyCruiser,   "CRUISER"},
    { (uword)CLASS_Carrier,        "CARRIER"},
    { (uword)CLASS_Destroyer,      "DESTROYER"},
    { (uword)CLASS_Frigate,        "FRIGATE"},
    { (uword)CLASS_Corvette,       "CORVETTE"},
    { (uword)CLASS_Fighter,        "FIGHTER"},
    { (uword)CLASS_Resource,       "RESOURCE"},
    { (uword)CLASS_NonCombat,      "NON-COMBAT"},
    { 0,NULL }
};*/

static NumStrXlate asteroidtypeinfo[] =
{
    { (uword)Asteroid0, str$(Asteroid0) },
    { (uword)Asteroid1, str$(Asteroid1) },
    { (uword)Asteroid2, str$(Asteroid2) },
    { (uword)Asteroid3, str$(Asteroid3) },
    { (uword)Asteroid4, str$(Asteroid4) },
    { 0,NULL }
};

static NumStrXlate dustcloudtypeinfo[] =
{
    { (uword)DustCloud0, str$(DustCloud0) },
    { (uword)DustCloud1, str$(DustCloud1) },
    { (uword)DustCloud2, str$(DustCloud2) },
    { (uword)DustCloud3, str$(DustCloud3) },
    { 0,NULL }
};

static NumStrXlate gascloudtypeinfo[] =
{
    { (uword)GasCloud0, str$(GasCloud0) },
    { (uword)GasCloud1, str$(GasCloud1) },
    { 0,NULL }
};

static NumStrXlate nebulatypeinfo[] =
{
    { (uword)Nebula0, str$(Nebula0) },
    { 0, NULL }
};

static NumStrXlate derelicttypeinfo[] =
{
    { (uword)Crate,                         str$(Crate)},
    { (uword)Homeworld,                     str$(Homeworld)},
    { (uword)PlanetOfOrigin,                str$(PlanetOfOrigin)},
    { (uword)PlanetOfOrigin_scarred,        str$(PlanetOfOrigin_scarred)},
    { (uword)Galaxy,                        str$(Galaxy)},
    { (uword)LavaBall,                      str$(LavaBall)},
    { (uword)MattWorld,                     str$(MattWorld)},
    { (uword)HyperspaceGate,                str$(HyperspaceGate)},
    { 0,NULL }
};

static NumStrXlate explosiontypeinfo[] =
{
    { ET_Explosion,           "ET_Explosion"           },

    { ET_LightLaserHit,       "ET_LightLaserHit"       },
    { ET_MedLaserHit,         "ET_MedLaserHit"         },
    { ET_HeavyLaserHit,       "ET_HeavyLaserHit"       },
    { ET_PlasmaBombHit,       "ET_PlasmaBombHit"       },

    { ET_InterceptorExplosion,"ET_InterceptorExplosion"},
    { ET_CorvetteExplosion,   "ET_CorvetteExplosion"   },
    { ET_FrigateExplosion,    "ET_FrigateExplosion"    },
    { ET_CarrierExplosion,    "ET_CarrierExplosion"    },
    { ET_ResourcerExplosion,  "ET_ResourcerExplosion"  },

    { 0,NULL }
};

static NumStrXlate guntypeinfo[] =
{
    { (uword)GUN_Fixed,  str$(GUN_Fixed) },
    { (uword)GUN_Gimble, str$(GUN_Gimble) },
    { (uword)GUN_NewGimble, str$(GUN_NewGimble) },
    { (uword)GUN_MissileLauncher, str$(GUN_MissileLauncher) },
    { (uword)GUN_MineLauncher, str$(GUN_MineLauncher) },
    { (uword)GUN_Gimble, str$(GUN_HiddenGimble) },
    { (uword)GUN_NewGimble, str$(GUN_VisibleGimble) },
    { 0,NULL }
};

static NumStrXlate dockpointtypeinfo[] =
{
    { (uword)DPT_Small_In, str$(DPT_Small_In) },
    { (uword)DPT_Small_Out, str$(DPT_Small_Out) },
    { (uword)DPT_Small_In_Engineering, str$(DPT_Small_In_Engineering) },
    { (uword)DPT_Small_Out_Engineering, str$(DPT_Small_Out_Engineering) },
    { (uword)DPT_Fighter_Latch, str$(DPT_Fighter_Latch) },
    { (uword)DPT_Corvette_Latch, str$(DPT_Corvette_Latch) },
    { (uword)DPT_Med_In, str$(DPT_Med_In) },
    { (uword)DPT_Med_Out, str$(DPT_Med_Out) },
    { (uword)DPT_Med_In_Engineering, str$(DPT_Med_In_Engineering) },
    { (uword)DPT_Med_Out_Engineering, str$(DPT_Med_Out_Engineering) },
    { (uword)DPT_Large_In, str$(DPT_Large_In) },
    { (uword)DPT_Large_Out, str$(DPT_Large_Out) },
    { (uword)DPT_Leech, str$(DPT_Leech) },
    { (uword)DPT_Leech_RU_Transfer, str$(DPT_Leech_RU_Transfer) },
    { (uword)DPT_Worker_RU_Transfer, str$(DPT_Worker_RU_Transfer) },
    { (uword)DPT_Tow, str$(DPT_Tow) },
    { (uword)DPT_Link, str$(DPT_Link) },
    { (uword)DPT_Repairbot, str$(DPT_Repairbot) },
    { (uword)DPT_Destroyer_EB, str$(DPT_Destroyer_EB) },
    { (uword)DPT_Module_EB, str$(DPT_Module_EB) },
    { (uword)DPT_Support_EB, str$(DPT_Support_EB) },
    { (uword)DPT_Swarmer_In, str$(DPT_Swarmer_In) },
    { (uword)DPT_Swarmer_Out, str$(DPT_Swarmer_Out) },
    { (uword)DPT_Crystal_Process, str$(DPT_Crystal_Process) },
    { (uword)DPT_Crystal_Effect, str$(DPT_Crystal_Effect) },
    { (uword)DPT_ForceFieldOrigin, str$(DPT_ForceFieldOrigin) },
    { (uword)DPT_Resourcer_RU_Transfer, str$(DPT_Resourcer_RU_Transfer) },
    { (uword)DPT_Salvage, str$(DPT_Salvage) },
    { (uword)DPT_SalvageLatch, str$(DPT_SalvageLatch) },
    { (uword)DPT_SalvageProcess, str$(DPT_SalvageProcess) },
    { (uword)DPT_RKOWeaponSource, str$(DPT_RKOWeaponSource) },
    { (uword)DPT_AutoRepair, str$(DPT_AutoRepair) },
    { (uword)DPT_CleesanBoarding, str$(DPT_CleesanBoarding) },
    { (uword)DPT_Mothership_In, str$(DPT_Mothership_In) },
    { (uword)DPT_Mothership_Out, str$(DPT_Mothership_Out) },

    { 0,NULL }
};

static NumStrXlate salvagepointtypeinfo[] =
{
    { (uword)Heading, str$(Heading) },
    { (uword)Up, str$(Up) },
    { (uword)AttachPoint, str$(AttachPoint) },

    { 0,NULL }
};

static NumStrXlate navlighttypeinfo[] =
{
    { (uword)NAVLIGHT_Default, str$(NAVLIGHT_Default) },
    { (uword)NAVLIGHT_FadeOnOff, str$(NAVLIGHT_FadeOnOff) },
    { 0,NULL }
};

static NumStrXlate gunsoundtypeinfo[] =
{
    { (uword)GS_LargeEnergyCannon, str$(GS_LargeEnergyCannon) },
    { (uword)GS_LargeIonCannon, str$(GS_LargeIonCannon) },
    { (uword)GS_LargePlasmaBomb, str$(GS_LargePlasmaBomb) },
    { (uword)GS_LargeProjectile, str$(GS_LargeProjectile) },
    { (uword)GS_MediumEnergyCannon, str$(GS_MediumEnergyCannon) },
    { (uword)GS_MediumIonCannon, str$(GS_MediumIonCannon) },
    { (uword)GS_MediumPlasmaBomb, str$(GS_MediumPlasmaBomb) },
    { (uword)GS_MediumProjectile, str$(GS_MediumProjectile) },
    { (uword)GS_MineLauncher, str$(GS_MineLauncher) },
    { (uword)GS_MissleLauncher, str$(GS_MissleLauncher) },
    { (uword)GS_SmallEnergyCannon, str$(GS_SmallEnergyCannon) },
    { (uword)GS_SmallIonCannon, str$(GS_SmallIonCannon) },
    { (uword)GS_SmallPlasmaBomb, str$(GS_SmallPlasmaBomb) },
    { (uword)GS_SmallProjectile, str$(GS_SmallProjectile) },
    { (uword)GS_VeryLargeEnergyCannon, str$(GS_VeryLargeEnergyCannon) },
    { (uword)GS_VeryLargeIonCannon, str$(GS_VeryLargeIonCannon) },
    { (uword)GS_VeryLargePlasmaBomb, str$(GS_VeryLargePlasmaBomb) },
    { (uword)GS_Laser, str$(GS_Laser) },
    { (uword)GS_BigGun, str$(GS_BigGun) },
    { (uword)GS_BeastWeapon, str$(GS_BeastWeapon) },
    { (uword)GS_VeryLargeProjectile, str$(GS_VeryLargeProjectile) },
    { 0,NULL }
};

static NumStrXlate weapontypeinfo[] =
{
    { (uword)WEAPON_Projectile, "WEAPON_PROJECTILE" },
    { (uword)WEAPON_Energy, "WEAPON_ENERGY" },
    { (uword)WEAPON_Ion, "WEAPON_ION" },
    { (uword)WEAPON_PlasmaBomb, "WEAPON_PLASMABOMB" },
    { (uword)WEAPON_Laser, "WEAPON_LASER" },
    { (uword)WEAPON_Missile, "WEAPON_MISSILE" },
    { (uword)WEAPON_Mine, "WEAPON_MINE" },
    { (uword)WEAPON_BigGun, "WEAPON_BIGGUN" },
    { (uword)WEAPON_BeastInfection, "WEAPON_BEASTINFECT" },
    { 0,NULL }
};

static NumStrXlate gunsizeinfo[] =
{
    { (uword)GUNSIZE_Small, "GUNSIZE_SMALL" },
    { (uword)GUNSIZE_Medium, "GUNSIZE_MEDIUM" },
    { (uword)GUNSIZE_Large, "GUNSIZE_LARGE" },
    { (uword)GUNSIZE_VeryLarge, "GUNSIZE_VERYLARGE" },
    { 0,NULL }
};

static NumStrXlate bullettypeinfo[] =
{
    { (uword)BULLET_Projectile, str$(BULLET_Projectile) },
    { (uword)BULLET_PlasmaBomb, str$(BULLET_PlasmaBomb) },
    { (uword)BULLET_Beam, str$(BULLET_Beam) },
    { 0,NULL }
};

static NumStrXlate missiletypeinfo[] =
{
    { (uword)MISSILE_Regular, str$(MISSILE_Regular) },
    { (uword)MISSILE_Mine, str$(MISSILE_Mine) },
    { 0,NULL }
};

static NumStrXlate objtypeinfo[] =
{
    { (uword)OBJ_ShipType, "Ship" },
    { (uword)OBJ_BulletType, "Bullet" },
    { (uword)OBJ_AsteroidType, "Asteroid" },
    { (uword)OBJ_NebulaType, "Nebula" },
    { (uword)OBJ_GasType, "GasCloud" },
    { (uword)OBJ_DustType, "DustCloud" },
    { (uword)OBJ_DerelictType, "Derelict" },
    { (uword)OBJ_EffectType, "Effect" },
    { (uword)OBJ_MissileType, "Missile" },
    { 0,NULL }
};

static NumStrXlate SpecialEffectinfo[] =
{
    { (uword)EGT_MINE_WALL_EFFECT, "EGT_MINE_WALL_EFFECT"},
    { (uword)EGT_BURST_CHARING_EFFECT, "EGT_BURST_CHARING_EFFECT"},
    { (uword)EGT_BURST_MUZZLE_EFFECT, "EGT_BURST_MUZZLE_EFFECT"},
    { (uword)EGT_BURST_BULLET_EFFECT, "EGT_BURST_BULLET_EFFECT"},
    { (uword)EGT_BURST_HIT_EFFECT, "EGT_BURST_HIT_EFFECT"},
    { (uword)EGT_BURST_EXPLOSION_EFFECT, "EGT_BURST_EXPLOSION_EFFECT"},
    { (uword)EGT_CLOAK_ON      , "EGT_CLOAK_ON"},
    { (uword)EGT_CLOAK_OFF     , "EGT_CLOAK_OFF"},
    { (uword)EGT_GRAVWELL_ON   , "EGT_GRAVWELL_ON"},
    { (uword)EGT_GRAVWELL_OFF  , "EGT_GRAVWELL_OFF"},
    { (uword)EGT_LIGHTNING_GLOW, "EGT_LIGHTNING_GLOW"},
    { (uword)EGT_BULLET_DEFLECT, "EGT_BULLET_DEFLECT"},
    { (uword)EGT_CRATE_GENERATED, "EGT_CRATE_GENERATED"},
    { (uword)EGT_CRATE_IS_FOUND_SHIP, "EGT_CRATE_IS_FOUND_SHIP"},
    { (uword)EGT_CRATE_IS_FOUND_MONEY, "EGT_CRATE_IS_FOUND_MONEY"},
    { (uword)EGT_CRATE_IS_FOUND_RESEARCH, "EGT_CRATE_IS_FOUND_RESEARCH"},
    { (uword)EGT_CRATE_TIME_OUT, "EGT_CRATE_TIME_OUT"},
    { (uword)EGT_REPAIR_BEAM, "EGT_REPAIR_BEAM"},
    { (uword)EGT_CAUGHT_GRAVWELL, "EGT_CAUGHT_GRAVWELL"},
    // Cataclysm additions
    { (uword)EGT_AFTER_BURNER, "EGT_AFTER_BURNER"},
    { (uword)EGT_BIG_GUN_BULLET_EXPLOSION, "EGT_BIG_GUN_BULLET_EXPLOSION"},
    { (uword)EGT_BIG_GUN_CHARGING, "EGT_BIG_GUN_CHARGING"},
    { (uword)EGT_BULLET_CUTTING_BEAM, "EGT_BULLET_CUTTING_BEAM"},
    { (uword)EGT_CRYSTAL_SHOCK_WAVE, "EGT_CRYSTAL_SHOCK_WAVE"},
    { (uword)EGT_DISTORTION_FIELD, "EGT_DISTORTION_FIELD"},
    { (uword)EGT_ELECTRO_MAGNETIC_PULSE, "EGT_ELECTRO_MAGNETIC_PULSE"},
    { (uword)EGT_LEECH_COLLECT_EFFECT, "EGT_LEECH_COLLECT_EFFECT"},
    { (uword)EGT_LEECH_VENT_EFFECT, "EGT_LEECH_VENT_EFFECT"},
    { (uword)EGT_LINKING_EFFECT, "EGT_LINKING_EFFECT"},
    { (uword)EGT_MOTHER_EXPLOSION_SHOCKWAVE, "EGT_MOTHER_EXPLOSION_SHOCKWAVE"},
    { (uword)EGT_RAMMING_FRIGATE_IMPACT_EFFECT, "EGT_RAMMING_FRIGATE_IMPACT_EFFECT"},
    { (uword)EGT_RAM_VENT, "EGT_RAM_VENT"},
    { (uword)EGT_RESOURCE_SHIP, "EGT_RESOURCE_SHIP"},
    { (uword)EGT_RESOURCE_SOLID, "EGT_RESOURCE_SOLID"},
    { (uword)EGT_SHOCKWAVE_STAR, "EGT_SHOCKWAVE_STAR"},
    { (uword)EGT_SLIPGATE_CLOSING_EFFECT, "EGT_SLIPGATE_CLOSING_EFFECT"},
    { (uword)EGT_SLIPGATE_OPENING_EFFECT, "EGT_SLIPGATE_OPENING_EFFECT"},
    { (uword)EGT_SLIPGATE_PERMANENT_EFFECT, "EGT_SLIPGATE_PERMANENT_EFFECT"},
    { (uword)EGT_SUCK_SHIP, "EGT_SUCK_SHIP"},
    { (uword)EGT_VORTEX_EFFECT, "EGT_VORTEX_EFFECT"},
    { (uword)EGT_WELDING_EFFECT, "EGT_WELDING_EFFECT"},
    { (uword)EGT_WORKER_RESOURCE_SOLID, "EGT_WORKER_RESOURCE_SOLID"},

    { 0,NULL }
};

static NumStrXlate tacticsinfo[] =
{
    { (uword)Evasive, str$(Evasive) },
    { (uword)Neutral, str$(Neutral) },
    { (uword)Aggressive, str$(Aggressive) },
    { 0,NULL }
};

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : RaceBitsToRace
    Description : Converts bit positions representing race (e.g. RACE1_VALID)
                  to race enumeration (e.g. RACE_Sect)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
ShipRace RaceBitsToRace(uword raceBits)
{
    if (raceBits & R1_VALID)
    {
        return RACE_Sect;
    }
    else if (raceBits & R2_VALID)
    {
        return RACE_Beast;
    }
    else if (raceBits & P1_VALID)
    {
        return P1;
    }
    else if (raceBits & TAIIDAN_VALID)
    {
        return RACE_Taiidan;
    }
    else if (raceBits & KUSHAN_VALID)
    {
        return RACE_Kushan;
    }
    else if (raceBits & Traders_VALID)
    {
        return Traders;
    }
}

/*-----------------------------------------------------------------------------
    Name        : NumToStr
    Description : Converts a DEFINE to a string
    Inputs      : numstrtab - table cross-referencing numbers and strings
                  num - number to convert
    Outputs     :
    Return      : pointer to converted string if found, NULL otherwise
----------------------------------------------------------------------------*/
char *NumToStr(NumStrXlate numstrtab[],uword num)
{
    uword i;
    NumStrXlate *curnumstr;

    for (i=0,curnumstr=numstrtab;curnumstr->str != NULL;i++,curnumstr++)
    {
        if (num == curnumstr->number)
        {
            return curnumstr->str;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : StrToNum
    Description : Converts a string to a DEFINE
    Inputs      : numstrtab - table cross-referencing numbers and strings
                  str - string to convert
    Outputs     :
    Return      : number if converted correctly, -1 otherwise
----------------------------------------------------------------------------*/
sdword StrToNum(NumStrXlate numstrtab[],char *str)
{
    sdword i;
    NumStrXlate *curnumstr;

    for (i=0,curnumstr=numstrtab;curnumstr->str != NULL;i++,curnumstr++)
    {
        if (_stricmp(str,curnumstr->str) == 0)
        {
            return curnumstr->number;
        }
    }
    return -1;
}

/*=============================================================================
    The following functions convert between ShipClass, ShipRace, ShipType
    and the strings representing them and vice-versa.
=============================================================================*/

char *TacticsTypeToStr(TacticsType tactics)
{
    return NumToStr(tacticsinfo,(uword)tactics);
}

TacticsType StrToTacticsType(char *str)
{
    return StrToNum(tacticsinfo,str);
}

char *ShipClassToStr(ShipClass shipclass)
{
    return NumToStr(shipclassinfo,(uword)shipclass);
}

char *ShipClassToNiceStr(ShipClass shipclass)
{
    return(strGetString((uword) shipclass+strClassOffset));
}

// Locks in the parity fix in StringsOnly.h: strCLASS_Mothership..strCLASS_Micro must stay
// contiguous and ordered to match ClassDefs.h's ShipClass values exactly, since this
// function does strGetString(shipclass+strClassOffset) -- confirmed against retail
// (cataclysm.exe ShipClassToNiceStr @ 0x4d6b70 indexes dword_BB3858[shipclass], where
// dword_BB3858 == &MessageStrings[strClassOffset] exactly, i.e. the same shared string
// table, not a separate array).
static_assert(strCLASS_Mothership + CLASS_HeavyCruiser == strCLASS_HeavyCruiser, "strCLASS_ ordering must match ClassDefs.h ShipClass values");
static_assert(strCLASS_Mothership + CLASS_MonsterShip == strCLASS_Composite, "strCLASS_ ordering must match ClassDefs.h ShipClass values");
static_assert(strCLASS_Mothership + CLASS_Component == strCLASS_Component, "strCLASS_ ordering must match ClassDefs.h ShipClass values");
static_assert(strCLASS_Mothership + CLASS_Micro == strCLASS_Micro, "strCLASS_ ordering must match ClassDefs.h ShipClass values");

char *ShipRaceToStr(ShipRace shiprace)
{
    return NumToStr(shipraceinfo,(uword)shiprace);
}

char *ShipTypeToStr(ShipType shiptype)
{
    return NumToStr(shiptypeinfo,(uword)shiptype);
}

char *ShipStatToNiceStr(ShipStatsType stattype)
{
    // There are 6 stat types for each ship type in the ship view stat window
    return(strGetString((uword) (stattype + strShipStatNameOffset)));
}

/*
char *ShipTypeStatToNiceStr(ShipType shiptype, ShipStatsType stattype)
{
    // There are 6 stat types for each ship type in the ship view stat window
    return(strGetString((uword) ((shiptype * 6 + stattype) + strShipStatOffset)));
}
*/

ShipRace StrToNisRace(char *string)
{
    return((ShipRace)StrToNum(nisraceinfo, string));
}

char *NisRaceToStr(ShipRace race)
{
    return(NumToStr(nisraceinfo, (uword)race));
}

char *ShipTypeToNiceStr(ShipType shiptype)
{
    return(strGetString((uword) shiptype + strShipOffset));
}

char *GunTypeToStr(GunType guntype)
{
    return NumToStr(guntypeinfo,(uword)guntype);
}

char *WeaponTypeToStr(WeaponType weaponType)
{
    return NumToStr(weapontypeinfo, (uword)weaponType);
}

char *GunSizeToStr(GunSize gunSize)
{
    return NumToStr(gunsizeinfo, (uword)gunSize);
}

char *GunSoundTypeToStr(GunSoundType gunsoundtype)
{
    return NumToStr(gunsoundtypeinfo,(uword)gunsoundtype);
}

char *DockPointTypeToStr(DockPointType dockpointtype)
{
    return NumToStr(dockpointtypeinfo,(uword)dockpointtype);
}

char *SalvagePointTypeToStr(SalvagePointType salvagepointtype)
{
    return NumToStr(salvagepointtypeinfo,(uword)salvagepointtype);
}

char *ExplosionTypeToStr(EffectType type)
{
    return NumToStr(explosiontypeinfo,(uword)type);
}

char *AsteroidTypeToStr(AsteroidType asteroidtype)
{
    return NumToStr(asteroidtypeinfo,(uword)asteroidtype);
}

char *DustCloudTypeToStr(DustCloudType dustcloudtype)
{
    return NumToStr(dustcloudtypeinfo,(uword)dustcloudtype);
}

char *GasCloudTypeToStr(GasCloudType gascloudtype)
{
    return NumToStr(gascloudtypeinfo,(uword)gascloudtype);
}

char *NebulaTypeToStr(NebulaType nebulatype)
{
    return NumToStr(nebulatypeinfo, (uword)nebulatype);
}

char *DerelictTypeToStr(DerelictType derelicttype)
{
    return NumToStr(derelicttypeinfo,(uword)derelicttype);
}

char *BulletTypeToStr(BulletType bulletType)
{
    return NumToStr(bullettypeinfo,(uword)bulletType);
}

char *MissileTypeToStr(MissileType missileType)
{
    return NumToStr(missiletypeinfo,(uword)missileType);
}

char *NAVLightTypeToStr(NAVLightType navlightType)
{
    return(NumToStr(navlighttypeinfo, (uword)navlightType));
}

char *ObjTypeToStr(ObjType objtype)
{
    return(NumToStr(objtypeinfo,(uword)objtype));
}

ShipClass StrToShipClass(char *str)
{
    return StrToNum(shipclassinfo,str);
}

ShipRace StrToShipRace(char *str)
{
    return StrToNum(shipraceinfo,str);
}

SpecialEffectType StrToEffectNum(char *str)
{
    return StrToNum(SpecialEffectinfo,str);
}

ShipType StrToShipType(char *str)
{
    return StrToNum(shiptypeinfo,str);
}

AsteroidType StrToAsteroidType(char *str)
{
    return StrToNum(asteroidtypeinfo,str);
}

DustCloudType StrToDustCloudType(char *str)
{
    return StrToNum(dustcloudtypeinfo, str);
}

GasCloudType StrToGasCloudType(char *str)
{
    return StrToNum(gascloudtypeinfo, str);
}

NebulaType StrToNebulaType(char *str)
{
    return StrToNum(nebulatypeinfo, str);
}

DerelictType StrToDerelictType(char *str)
{
    return StrToNum(derelicttypeinfo,str);
}

EffectType StrToExplosionType(char *str)
{
    return StrToNum(explosiontypeinfo,str);
}

GunType StrToGunType(char *str)
{
    return StrToNum(guntypeinfo,str);
}

WeaponType StrToWeaponType(char *str)
{
    return StrToNum(weapontypeinfo, str);
}

GunSize StrToGunSize(char *str)
{
    return StrToNum(gunsizeinfo, str);
}

GunSoundType StrToGunSoundType(char *str)
{
    return StrToNum(gunsoundtypeinfo,str);
}

DockPointType StrToDockPointType(char *str)
{
    return StrToNum(dockpointtypeinfo,str);
}

SalvagePointType StrToSalvagePointType(char *str)
{
    return StrToNum(salvagepointtypeinfo,str);
}

BulletType StrToBulletType(char *str)
{
    return StrToNum(bullettypeinfo,str);
}

MissileType StrToMissileType(char *str)
{
    return StrToNum(missiletypeinfo,str);
}

NAVLightType StrToNAVLightType(char *str)
{
    return StrToNum(navlighttypeinfo, str);
}

ObjType StrToObjType(char *str)
{
    return StrToNum(objtypeinfo, str);
}



sdword GetRaceForShipTypeRange(udword shipType)
{
    sdword result;

    for (result = 0; result < NUM_RACES; ++result)
    {
        if (shipType >= (udword)FirstShipTypeOfRace[result] &&
            shipType <= (udword)LastShipTypeOfRace[result])
        {
            return result;
        }
    }

    return 0;
}
