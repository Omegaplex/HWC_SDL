/*=============================================================================
    Name    : Objtypes.h
    Purpose : Enumerations of object types, ships, race, class, etc.

    Created 7/8/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___OBJTYPES_H
#define ___OBJTYPES_H

#include "RaceDefs.h"
#include "ShipDefs.h"
#include "Types.h"

/*=============================================================================
    Types:
=============================================================================*/

typedef enum
{
    DPT_Small_In,
    DPT_Small_Out,
    DPT_Small_In_Engineering,
    DPT_Small_Out_Engineering,
    DPT_Fighter_Latch,
    DPT_Corvette_Latch,
    DPT_Med_In,
    DPT_Med_Out,
    DPT_Med_In_Engineering,
    DPT_Med_Out_Engineering,
    DPT_Large_In,
    DPT_Large_Out,
    DPT_Leech,
    DPT_Leech_RU_Transfer,
    DPT_Worker_RU_Transfer,
    DPT_Tow,
    DPT_Link,
    DPT_Repairbot,
    DPT_Destroyer_EB,
    DPT_Module_EB,
    DPT_Support_EB,
    DPT_Swarmer_In,
    DPT_Swarmer_Out,
    DPT_Crystal_Process,
    DPT_Crystal_Effect,
    DPT_ForceFieldOrigin,
    DPT_Resourcer_RU_Transfer,
    DPT_Salvage,
    DPT_SalvageLatch,
    DPT_SalvageProcess,
    DPT_RKOWeaponSource,
    DPT_AutoRepair,
    DPT_CleesanBoarding,
    DPT_Mothership_In,
    DPT_Mothership_Out,
    NUM_DOCK_POINT_TYPES
} DockPointType;

typedef enum
{
    Heading,
    Up,
    AttachPoint,
} SalvagePointType;

typedef enum
{
    GUN_Fixed,
    GUN_Gimble,
    GUN_NewGimble,
    GUN_MissileLauncher,
    GUN_MineLauncher,
    GUN_HiddenGimble,
    GUN_VisibleGimble
} GunType;

typedef enum
{
    NAVLIGHT_Default,
    NAVLIGHT_FadeOnOff,
} NAVLightType;

typedef enum
{
    GS_LargeEnergyCannon,
    GS_LargeIonCannon,
    GS_LargePlasmaBomb,
    GS_LargeProjectile,
    GS_MediumEnergyCannon,
    GS_MediumIonCannon,
    GS_MediumPlasmaBomb,
    GS_MediumProjectile,
    GS_MineLauncher,
    GS_MissleLauncher,
    GS_SmallEnergyCannon,
    GS_SmallIonCannon,
    GS_SmallPlasmaBomb,
    GS_SmallProjectile,
    GS_VeryLargeEnergyCannon,
    GS_VeryLargeIonCannon,
    GS_VeryLargePlasmaBomb,
    GS_Laser,
    GS_BigGun,
    GS_BeastWeapon,
    GS_VeryLargeProjectile,
    NUM_GUN_SOUND_TYPES
} GunSoundType;

typedef enum
{
    WEAPON_Projectile,
    WEAPON_Energy,
    WEAPON_Ion,
    WEAPON_PlasmaBomb,
    WEAPON_Laser,
    WEAPON_Missile,
    WEAPON_Mine,
    WEAPON_BigGun,
    WEAPON_BeastInfection
} WeaponType;

typedef enum
{
    GUNSIZE_Small,
    GUNSIZE_Medium,
    GUNSIZE_Large,
    GUNSIZE_VeryLarge
} GunSize;

typedef enum
{
    BULLET_Projectile,
    BULLET_PlasmaBomb,
    BULLET_Beam,
    BULLET_Laser,
    BULLET_SpecialBurst
} BulletType;

typedef enum
{
    MISSILE_Regular,
    MISSILE_Mine
} MissileType;

typedef enum
{
    OBJ_ShipType,
    OBJ_BulletType,
    OBJ_AsteroidType,
    OBJ_NebulaType,
    OBJ_GasType,
    OBJ_DustType,
    OBJ_DerelictType,
    OBJ_EffectType,
    OBJ_MissileType
} ObjType;

// CLASS enumerations   (changed to defines for speech tool in ClassDefs.h)
typedef udword ShipClass;

// RACE enumerations    (changed to defines for speech tool in RaceDefs.h)
typedef udword ShipRace;

#define RaceToRaceBits(race) (1<<(race))

// RACE bits
#define R1_VALID            (1<<RACE_Sect)
#define R2_VALID            (1<<RACE_Beast)
#define P1_VALID            (1<<P1)
#define TAIIDAN_VALID       (1<<RACE_Taiidan)
#define KUSHAN_VALID        (1<<RACE_Kushan)
#define Traders_VALID       (1<<Traders)

//special 'races' for different types of objects besides ships
#define NSR_Asteroid                (NUM_RACES + 0)
#define NSR_DustCloud               (NUM_RACES + 1)
#define NSR_GasCloud                (NUM_RACES + 2)
#define NSR_Nebula                  (NUM_RACES + 3)
#define NSR_Derelict                (NUM_RACES + 4)
#define NSR_Effect                  (NUM_RACES + 5)
#define NSR_Generic                 (NUM_RACES + 6)
#define NSR_Missile                 (NUM_RACES + 7)
#define NSR_LastNSR                 (NSR_Missile + 1)

// moved ship defines to ShipDefs.h

typedef enum
{
    Asteroid0,
    Asteroid1,
    Asteroid2,
    Asteroid3,
    Asteroid4,
    NUM_ASTEROIDTYPES
} AsteroidType;

typedef enum
{
    DustCloud0,
    DustCloud1,
    DustCloud2,
    DustCloud3,
    NUM_DUSTCLOUDTYPES
} DustCloudType;

typedef enum
{
    GasCloud0,
    GasCloud1,
    NUM_GASCLOUDTYPES
} GasCloudType;

typedef enum
{
    Nebula0,
    NUM_NEBULATYPES
} NebulaType;

typedef enum
{
    // Cataclysm derelict types (8 entries, decompiled from IDA 0x889B68)
    Crate,
    Homeworld,
    PlanetOfOrigin,
    PlanetOfOrigin_scarred,
    Galaxy,
    LavaBall,
    MattWorld,
    HyperspaceGate,
    NUM_DERELICTTYPES
} DerelictType;

typedef enum
{
    ET_Explosion = 0,           //generic explosion
    ET_LightLaserHit,           //weapon impacts
    ET_MedLaserHit,
    ET_HeavyLaserHit,
    ET_PlasmaBombHit,

    ET_InterceptorExplosion,    //ship explosions
    ET_CorvetteExplosion,
    ET_FrigateExplosion,
    ET_CarrierExplosion,
    ET_ResourcerExplosion,

    NUM_EFFECT_TYPES            // must be last
}
EffectType;

typedef enum
{
    Evasive,
    Neutral,
    Aggressive,

    NUM_TACTICS_TYPES
}
TacticsType;

typedef enum
{
    Firepower = 0,
    Coverage,
    Maneuver,
    Armor,
    TopSpeed,
    Mass,
    VeryLow,
    Low,
    Medium,
    High,
    VeryHigh,
    CoverageUnits,

    NUM_SHIP_STATS
}
ShipStatsType;

typedef struct
{
    uword number;
    char *str;
} NumStrXlate;

typedef udword SpecialEffectType;
//special effect var defines
#define EGT_MINE_WALL_EFFECT        0
#define EGT_BURST_CHARING_EFFECT    1
#define EGT_BURST_MUZZLE_EFFECT     2
#define EGT_BURST_BULLET_EFFECT     3
#define EGT_BURST_HIT_EFFECT        4
#define EGT_BURST_EXPLOSION_EFFECT  5   //bullet explosion
#define EGT_CLOAK_ON                6
#define EGT_CLOAK_OFF               7
#define EGT_GRAVWELL_ON             8
#define EGT_GRAVWELL_OFF            9
#define EGT_LIGHTNING_GLOW         10
#define EGT_BULLET_DEFLECT         12
#define EGT_CRATE_GENERATED        13
#define EGT_CRATE_IS_FOUND_SHIP    14
#define EGT_CRATE_IS_FOUND_MONEY   15
#define EGT_CRATE_IS_FOUND_RESEARCH 16
#define EGT_CRATE_TIME_OUT         17
#define EGT_REPAIR_BEAM            18
#define EGT_CAUGHT_GRAVWELL        19
// Cataclysm additions - binary-backed special-purpose ETG indices
#define EGT_WELDING_EFFECT         20
#define EGT_RESOURCE_SOLID         21
#define EGT_VORTEX_EFFECT          22
#define EGT_ELECTRO_MAGNETIC_PULSE 23
#define EGT_LEECH_COLLECT_EFFECT   24
#define EGT_LEECH_VENT_EFFECT      25
#define EGT_LINKING_EFFECT         26
#define EGT_BIG_GUN_BULLET_EXPLOSION 27
#define EGT_BIG_GUN_CHARGING       28
#define EGT_SLIPGATE_OPENING_EFFECT 29
#define EGT_SLIPGATE_CLOSING_EFFECT 30
#define EGT_SLIPGATE_PERMANENT_EFFECT 31
#define EGT_RAMMING_FRIGATE_IMPACT_EFFECT 32
#define EGT_SHOCKWAVE_STAR         33
#define EGT_SUCK_SHIP              34
#define EGT_RESOURCE_SHIP          35
#define EGT_CRYSTAL_SHOCK_WAVE     36
#define EGT_BULLET_CUTTING_BEAM    37
#define EGT_WORKER_RESOURCE_SOLID  38
#define EGT_DISTORTION_FIELD       39
#define EGT_AFTER_BURNER           40
#define EGT_RAM_VENT               41
#define EGT_MOTHER_EXPLOSION_SHOCKWAVE 42
#define EGT_NumberOfSpecialEffects 43

/*=============================================================================
    Data:
=============================================================================*/

extern ShipType FirstShipTypeOfRace[NUM_RACES];
extern ShipType LastShipTypeOfRace[NUM_RACES];
extern uword NumShipTypesInRace[NUM_RACES];

/*=============================================================================
    Support Functions:
=============================================================================*/

char *NumToStr(NumStrXlate numstrtab[],uword num);
sdword StrToNum(NumStrXlate numstrtab[],char *str);

ShipRace RaceBitsToRace(uword raceBits);

char *ShipClassToStr(ShipClass shipclass);
char *ShipClassToNiceStr(ShipClass shipclass);
ShipClass StrToShipClass(char *str);

char *ShipRaceToStr(ShipRace shiprace);
ShipRace StrToShipRace(char *str);

char *TacticsTypeToStr(TacticsType tactics);
TacticsType StrToTacticsType(char *str);

SpecialEffectType StrToEffectNum(char *str);

char *ShipTypeToStr(ShipType shiptype);
char *ShipTypeToNiceStr(ShipType shiptype);
ShipType StrToShipType(char *str);

char *ShipStatToNiceStr(ShipStatsType stattype);
char *ShipTypeStatToNiceStr(ShipType shiptype, ShipStatsType stattype);

char *AsteroidTypeToStr(AsteroidType asteroidtype);
AsteroidType StrToAsteroidType(char *str);

char *DustCloudTypeToStr(DustCloudType dustcloudtype);
DustCloudType StrToDustCloudType(char *str);

char *GasCloudTypeToStr(GasCloudType gascloudtype);
GasCloudType StrToGasCloudType(char *str);

char *NebulaTypeToStr(NebulaType nebulatype);
NebulaType StrToNebulaType(char *str);

char *DerelictTypeToStr(DerelictType derelicttype);
DerelictType StrToDerelictType(char *str);

char *ExplosionTypeToStr(EffectType type);
EffectType StrToExplosionType(char *str);

char *GunTypeToStr(GunType guntype);
GunType StrToGunType(char *str);

char *WeaponTypeToStr(WeaponType weaponType);
WeaponType StrToWeaponType(char *str);

char *GunSizeToStr(GunSize gunSize);
GunSize StrToGunSize(char *str);

DockPointType StrToDockPointType(char *str);
char *DockPointTypeToStr(DockPointType dockpointtype);

SalvagePointType StrToSalvagePointType(char *str);
char *SalvagePointTypeToStr(SalvagePointType salvagepointtype);

char *GunSoundTypeToStr(GunSoundType gunsoundtype);
GunSoundType StrToGunSoundType(char *str);

char *BulletTypeToStr(BulletType bulletType);
BulletType StrToBulletType(char *str);

char *MissileTypeToStr(MissileType missileType);
MissileType StrToMissileType(char *str);

char *NAVLightTypeToStr(NAVLightType navlightType);
NAVLightType StrToNAVLightType(char *str);

char *ObjTypeToStr(ObjType objtype);
ObjType StrToObjType(char *str);

char *NisRaceToStr(ShipRace race);
ShipRace StrToNisRace(char *string);

#endif
