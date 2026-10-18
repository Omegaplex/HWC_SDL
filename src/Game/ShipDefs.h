/*=============================================================================
    Name    : ShipDefs.h
    Purpose : Ship type definitions for Homeworld: Cataclysm
=============================================================================*/

#ifndef ___SHIPDEFS_H
#define ___SHIPDEFS_H

#include "Types.h"
#include "RaceDefs.h"

typedef udword ShipType;
typedef udword ShipRace;
typedef udword ShipClass;

#define sRecon 0
#define sAcolyte 1
#define sMimic 2
#define sLeech 3
#define sACV 4
#define sMCV 5
#define sRammingFrigate 6
#define sMultiBeamFrigate 7
#define sHiveFrigate 8
#define sSwarmer 9
#define sDreadnought 10
#define sDreadnoughtCannon 11
#define sCarrier 12
#define sCarrierSupport 13
#define sWorker 14
#define sRepairDroid 15
#define sProcessor 16
#define sSentinel 17
#define sMothership 18
#define sMothershipSpecial 19
#define sMothershipEngineering 20
#define sMothershipArmour 21
#define sMothershipMicro 22
#define sMothershipWeapons 23
#define sMothershipDockingBay 24
#define sDestroyer 25
#define sMothershipSupport 26
#define sMothershipCommand 27
#define sMothershipEnginesLower 28
#define sMothershipEnginesUpper 29
#define sMothershipLowerSpine 30
#define sMothershipBigGun 31
#define sMothershipOreCanArmour 32
#define bRecon 33
#define bAcolyte 34
#define bInterceptor 35
#define bCloakedFighter 36
#define bAttackBomber 37
#define bACV 38
#define bMultiGunCorvette 39
#define bHeavyCorvette 40
#define bMultiBeamFrigate 41
#define bHiveFrigate 42
#define bSwarmer 43
#define bIonArrayFrigate 44
#define bDFGFrigate 45
#define bHeavyCruiser 46
#define bCarrier 47
#define bRammingFrigate 48
#define bWorker 49
#define bRepairDroid 50
#define bProcessor 51
#define bSentinel 52
#define bMothership 53
#define bMothershipDockingBay 54
#define bMothershipEnginesLower 55
#define bSectMoship 56
#define bMothershipSupport 57
#define bCruiseMissile 58
#define bMissileCorvette 59
#define kAdvanceSupportFrigate 60
#define kAttackBomber 61
#define kCarrier 62
#define kCloakGenerator 63
#define kGravWellGenerator 64
#define kHeavyCorvette 65
#define kHeavyCruiser 66
#define kHeavyDefender 67
#define kHeavyInterceptor 68
#define kIonCannonFrigate 69
#define kMultiGunCorvette 70
#define kProximitySensor 71
#define kSalCapCorvette 72
#define kStandardDestroyer 73
#define tAdvanceSupportFrigate 74
#define tAttackBomber 75
#define tCarrier 76
#define tCloakGenerator 77
#define tDFGFrigate 78
#define tGravWellGenerator 79
#define tHeavyCruiser 80
#define tHeavyDefender 81
#define tHeavyInterceptor 82
#define tIonCannonFrigate 83
#define tMinelayerCorvette 84
#define tMissileDestroyer 85
#define tMultiGunCorvette 86
#define tProximitySensor 87
#define tResourceCollector 88
#define tSalCapCorvette 89
#define tStandardDestroyer 90
#define tStandardFrigate 91
#define p1Outpost 92
#define p1Fighter 93
#define p1IonArrayFrigate 94
#define p1MissileCorvette 95
#define P1MissileCorvette      p1MissileCorvette
#define p1Mothership 96
#define p1StandardCorvette 97
#define trFloatingCity 98
#define trCargoBarge 99
#define trMiningBase 100
#define trResearchStation 101
#define trJunkYardDawg 102
#define trJunkYardHQ 103
#define trGhostship 104
#define trJunk_LGun 105
#define trJunk_SGun 106
#define trResearchStationBridge 107
#define trResearchStationTower 108
#define dRKO 109
#define dBigGun 110
#define dBigGunUnit 111
#define dSlipgateSmall 112
#define dSlipgateMedium 113
#define dSlipgateLarge 114
#define dCrate 115
#define dJunk1PartA 116
#define dJunk1Shell 117
#define dJunk1Strut 118
#define dJunk2Shipwreck 119
#define dJunk3Boiler 120
#define dJunk3BoilerCasing 121
#define dMessagePod 122
#define dBuoy 123
#define dFirstPlanet 124
#define dDeadPlanet 125
#define dCrystal0 126
#define dCrystal1 127
#define dBeastDockingBay 128
#define dNISBuoy 129
#define dBeastMothership 130
#define dNomadMoon 131
#define dNomadFieldEmitter 132
#define dAcolyte 133
#define dPrisonShip 134
#define dPrisonShipInfected 135
#define dSlipgateNode 136
#define dKushanCarrier 137
#define dIonArrayFrigate 138
#define dIonCannonFrigate 139
#define dEcclesiam 140
#define dEcclesiamInfected 141
#define nisLightSource1 142
#define nisLightSource2 143
#define nisLensflareSource1 144

#define SECT_FIRST_SHIP sRecon
#define SECT_LAST_SHIP sMothershipOreCanArmour
#define BEAST_FIRST_SHIP bRecon
#define BEAST_LAST_SHIP bMissileCorvette

#define STD_FIRST_SHIP SECT_FIRST_SHIP
#define STD_LAST_SHIP BEAST_LAST_SHIP

#define TOTAL_NUM_SHIPS 145

#define TOTAL_STD_SHIPS (STD_LAST_SHIP - STD_FIRST_SHIP + 1)
#define TOTAL_SECT_SHIPS (SECT_LAST_SHIP - SECT_FIRST_SHIP + 1)
#define TOTAL_BEAST_SHIPS (BEAST_LAST_SHIP - BEAST_FIRST_SHIP + 1)

// Ship range defines for non-standard races
// Note: P1 = Turanic Raiders (race 4), ships have k prefix (Kushan HW1 origin)  
// P3 = Kushan (race 2), ships have p1 prefix (P1 HW1 origin)
// The naming is historical from HW1 and does not match Cataclysm's race names
#define P1_FIRST_SHIP    kAdvanceSupportFrigate
#define P1_LAST_SHIP     kStandardDestroyer
#define P2_FIRST_SHIP    tAdvanceSupportFrigate
#define P2_LAST_SHIP     tStandardFrigate
#define P3_FIRST_SHIP    p1Outpost
#define P3_LAST_SHIP     p1StandardCorvette
#define TRADERS_FIRST_SHIP trFloatingCity
#define TRADERS_LAST_SHIP  trResearchStationTower
#define DERELICT_FIRST_SHIP dRKO
#define DERELICT_LAST_SHIP  nisLensflareSource1

#define TOTAL_P1_SHIPS        (P1_LAST_SHIP - P1_FIRST_SHIP + 1)
#define TOTAL_P2_SHIPS        (P2_LAST_SHIP - P2_FIRST_SHIP + 1)
#define TOTAL_P3_SHIPS        (P3_LAST_SHIP - P3_FIRST_SHIP + 1)
#define TOTAL_TRADERS_SHIPS   (TRADERS_LAST_SHIP - TRADERS_FIRST_SHIP + 1)
#define TOTAL_DERELICT_SHIPS  (DERELICT_LAST_SHIP - DERELICT_FIRST_SHIP + 1)
#define TOTAL_OTHER_SHIPS     (TOTAL_P1_SHIPS + TOTAL_P2_SHIPS + TOTAL_P3_SHIPS + TOTAL_TRADERS_SHIPS)

static __inline bool32 ShipTypeIsWorker(ShipType shiptype)
{
    return (shiptype == sWorker) || (shiptype == bWorker);
}

static __inline bool32 ShipTypeIsMothershipType(ShipType shiptype)
{
    return (shiptype == sMothership) || (shiptype == bMothership);
}

static __inline bool32 ShipTypeIsStandardMothership(ShipType shiptype)
{
    return ShipTypeIsMothershipType(shiptype);
}

static __inline bool32 ShipTypeIsCarrierType(ShipType shiptype)
{
    return (shiptype == sCarrier) || (shiptype == bCarrier) || (shiptype == kCarrier) || (shiptype == tCarrier) || (shiptype == dKushanCarrier);
}

static __inline bool32 ShipTypeIsMothershipOrCarrierType(ShipType shiptype)
{
    return ShipTypeIsMothershipType(shiptype) || ShipTypeIsCarrierType(shiptype);
}

static __inline bool32 ShipTypeIsProximitySensorType(ShipType shiptype)
{
    return (shiptype == kProximitySensor) || (shiptype == tProximitySensor);
}

static __inline bool32 ShipTypeIsRepairShipType(ShipType shiptype)
{
    return (shiptype == sRepairDroid) || (shiptype == bRepairDroid) || (shiptype == kAdvanceSupportFrigate) || (shiptype == tAdvanceSupportFrigate);
}

static __inline bool32 ShipTypeIsProcessorType(ShipType shiptype)
{
    return (shiptype == sProcessor) || (shiptype == bProcessor);
}

static __inline bool32 ShipTypeIsDefenseFieldFrigateType(ShipType shiptype)
{
    return (shiptype == bDFGFrigate) || (shiptype == tDFGFrigate);
}

static __inline bool32 ShipTypeIsCloakGeneratorType(ShipType shiptype)
{
    return (shiptype == kCloakGenerator) || (shiptype == tCloakGenerator);
}

static __inline bool32 ShipTypeIsGravWellGeneratorType(ShipType shiptype)
{
    return (shiptype == kGravWellGenerator) || (shiptype == tGravWellGenerator);
}

static __inline bool32 ShipTypeIsSalCapCorvetteType(ShipType shiptype)
{
    return (shiptype == kSalCapCorvette) || (shiptype == tSalCapCorvette);
}

static __inline bool32 ShipTypeIsHomeworldAttackBomberType(ShipType shiptype)
{
    return (shiptype == kAttackBomber) || (shiptype == tAttackBomber);
}

static __inline bool32 ShipTypeIsHomeworldHeavyInterceptorType(ShipType shiptype)
{
    return (shiptype == kHeavyInterceptor) || (shiptype == tHeavyInterceptor);
}

static __inline bool32 ShipTypeIsHomeworldHeavyDefenderType(ShipType shiptype)
{
    return (shiptype == kHeavyDefender) || (shiptype == tHeavyDefender);
}

static __inline bool32 ShipTypeIsHomeworldMultiGunCorvetteType(ShipType shiptype)
{
    return (shiptype == kMultiGunCorvette) || (shiptype == tMultiGunCorvette);
}

static __inline bool32 ShipTypeIsHomeworldSalCapCorvetteType(ShipType shiptype)
{
    return (shiptype == kSalCapCorvette) || (shiptype == tSalCapCorvette);
}

static __inline bool32 ShipTypeIsHomeworldIonCannonFrigateType(ShipType shiptype)
{
    return (shiptype == kIonCannonFrigate) || (shiptype == tIonCannonFrigate);
}

static __inline bool32 ShipTypeIsHomeworldStandardDestroyerType(ShipType shiptype)
{
    return (shiptype == kStandardDestroyer) || (shiptype == tStandardDestroyer);
}

static __inline bool32 ShipTypeIsHomeworldStandardFrigateType(ShipType shiptype)
{
    return shiptype == tStandardFrigate;
}

static __inline bool32 ShipTypeIsHomeworldMissileDestroyerType(ShipType shiptype)
{
    return shiptype == tMissileDestroyer;
}

static __inline bool32 ShipTypeIsTargetDroneType(ShipType shiptype)
{
    return shiptype == sSwarmer;
}

static __inline ShipType WorkerShipTypeForRace(ShipRace race)
{
    return (race == RACE_Beast) ? bWorker : sWorker;
}

static __inline ShipType ProcessorShipTypeForRace(ShipRace race)
{
    return (race == RACE_Beast) ? bProcessor : sProcessor;
}

static __inline ShipType RepairShipTypeForRace(ShipRace race)
{
    return (race == RACE_Beast) ? bRepairDroid : sRepairDroid;
}

static __inline ShipType HomeworldShipTypeForRace(ShipRace race, ShipType kshiptype, ShipType tshiptype)
{
    return (race == RACE_Taiidan) ? tshiptype : kshiptype;
}

static __inline ShipType HomeworldAttackBomberTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kAttackBomber, tAttackBomber);
}

static __inline ShipType HomeworldHeavyInterceptorTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kHeavyInterceptor, tHeavyInterceptor);
}

static __inline ShipType HomeworldHeavyDefenderTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kHeavyDefender, tHeavyDefender);
}

static __inline ShipType HomeworldMultiGunCorvetteTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kMultiGunCorvette, tMultiGunCorvette);
}

static __inline ShipType HomeworldSalCapCorvetteTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kSalCapCorvette, tSalCapCorvette);
}

static __inline ShipType HomeworldIonCannonFrigateTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kIonCannonFrigate, tIonCannonFrigate);
}

static __inline ShipType HomeworldStandardDestroyerTypeForRace(ShipRace race)
{
    return HomeworldShipTypeForRace(race, kStandardDestroyer, tStandardDestroyer);
}

static __inline ShipType HomeworldStandardFrigateTypeForRace(ShipRace race)
{
    return tStandardFrigate;
}

static __inline ShipType HomeworldMissileDestroyerTypeForRace(ShipRace race)
{
    return tMissileDestroyer;
}

// Traders - direct mapping
#define FloatingCity           trFloatingCity
#define CargoBarge             trCargoBarge
#define MiningBase             trMiningBase
#define ResearchStation        trResearchStation
#define JunkYardDawg           trJunkYardDawg
#define JunkYardHQ             trJunkYardHQ
#define Ghostship              trGhostship
#define Junk_LGun              trJunk_LGun
#define Junk_SGun              trJunk_SGun
#define ResearchStationBridge  trResearchStationBridge
#define ResearchStationTower   trResearchStationTower

#define TargetDrone               sSwarmer
// Other
#define HeadShotAsteroid    dRKO
#define CryoTray               dCrate
#define DefaultShip            TOTAL_NUM_SHIPS

#endif // ___SHIPDEFS_H