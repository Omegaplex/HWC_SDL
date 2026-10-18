// =============================================================================
//  GenericInterceptor.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 6/30/1997 by gshaw
// =============================================================================

#ifndef ___GENERICINTERCEPTOR_H
#define ___GENERICINTERCEPTOR_H

#include "ClassDefs.h"
#include "FlightMan.h"
#include "ObjTypes.h"
#include "Types.h"

#define STATE_INIT              0
#define STATE_APPROACH          1
#define STATE_KILL              2
#define STATE_BREAK             3
#define STATE_TURNAROUND        4
#define STATE_REAPPROACH        5

#define STATE_WHIPSTRAFE1       10
#define STATE_WHIPSTRAFE2       11
#define STATE_BARRELROLL_OUT    12
#define STATE_START_BOMBING_RUN 13
#define STATE_BOMBING_DIVE      14

static inline bool32 shipTypeUsesInterceptorCombat(ShipType shipType)
{
    switch (shipType)
    {
        case sRecon:
        case bRecon:
        case bInterceptor:
        case bCloakedFighter:
        case bAttackBomber:
        case kAttackBomber:
        case kHeavyInterceptor:
        case tAttackBomber:
        case tHeavyInterceptor:
        case P1Fighter:
        case TargetDrone:
            return TRUE;
        default:
            return FALSE;
    }
}

#define isShipInterceptor(ship) shipTypeUsesInterceptorCombat((ship)->shiptype)
#define isShipStaticInterceptor(shipstatic) shipTypeUsesInterceptorCombat((shipstatic)->shiptype)

typedef struct
{
    real32 lasttimefired;
    real32 aitimekill;
    vector aivec;
    bool16 aiAIPflightman;
    uword activeGun;
} GenericInterceptorSpec;

typedef struct
{
    real32 lasttimefired;
    real32 aitimekill;
    vector aivec;
    bool16 aiAIPflightman;
    uword activeGun;
    real32 CloakingStatus;
    bool8 CloakLowWarning;
    bool8 ReCloak;
    real32 ReCloakTime;
} CloakedFighterSpec;

typedef enum
{
    BeastCloakMode_Recharging = 0,
    BeastCloakMode_Cloaked = 1,
    BeastCloakMode_Decloaking = 2,
    BeastCloakMode_Cloaking = 3
} BeastCloakMode;

typedef struct
{
    BeastCloakMode mode;
    real32 charge;
    real32 cloakDrainPerTick;
    real32 decloakDrainPerTick;
    real32 nextTransitionTime;
    real32 blend;
} BeastCloakState;

typedef struct
{
    real32 lasttimefired;
    real32 aitimekill;
    vector aivec;
    bool16 aiAIPflightman;
    uword activeGun;
    BeastCloakState cloakState;
    sdword slotStatus[MAX_MULTIPLAYER_PLAYERS];
} BeastCloakedFighterSpec;

typedef struct
{
    real32 lasttimefired;
    real32 aitimekill;
    vector aivec;
    bool16 aiAIPflightman;
    uword activeGun;
    vector rallypoint;
    vector orig_point;
    real32 angle;
    sdword bombingdelay;
} AttackBomberSpec;

typedef struct
{
    real32 firetime;
    real32 maxFlyAwayDist[NUM_TACTICS_TYPES][NUM_CLASSES];
    real32 maxFlyAwayDistSquared[NUM_TACTICS_TYPES][NUM_CLASSES];
    real32 breakRange[NUM_TACTICS_TYPES][NUM_CLASSES];
    real32 flyPastDist[NUM_TACTICS_TYPES][NUM_CLASSES];
    real32 triggerHappy[NUM_TACTICS_TYPES][NUM_CLASSES];
    real32 faceTargetAccuracy[NUM_TACTICS_TYPES][NUM_CLASSES];
    real32 maxAttackTime[NUM_TACTICS_TYPES][NUM_CLASSES];
    FlightManProb flightmanProb[NUM_TACTICS_TYPES][NUM_CLASSES];
} GenericInterceptorStatics;

extern CustShipHeader AttackBomberHeader;
extern CustShipHeader BeastCloakedFighterHeader;
extern CustShipHeader CloakedFighterHeader;
extern CustShipHeader GenericInterceptorHeader;
extern CustShipHeader TargetDroneHeader;

void GenericInterceptorAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
bool32 GenericInterceptorCanFire(Ship *ship, SpaceObjRotImpTarg *target, vector *trajectory, real32 triggerHappy);
FlightManProb *GenericInterceptorGetFlightManProb(Ship *ship, SpaceObjRotImpTarg *target);
bool32 InterceptorInRange(Ship *ship, SpaceObjRotImpTarg *target);
void SpawnCloakingEffect(Ship *ship, etglod *etgLOD);
bool32 CloakedShipSpecialActivate(Ship *ship);
void CloakedShipHousekeep(Ship *ship);

#endif
