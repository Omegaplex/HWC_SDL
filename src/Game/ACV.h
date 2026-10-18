#ifndef ___ACV_H
#define ___ACV_H

#include "Attack.h"
#include "spaceobj.h"
#include "shipselect.h"

typedef enum
{
    ACVEMPState_Ready = 0,
    ACVEMPState_Active = 1,
    ACVEMPState_Recharging = 2,
    ACVEMPState_Deploying = 3
} ACVEMPState;

typedef struct
{
    ubyte unk000[0x18];
    SpaceObj *unk018;
    ubyte unk01C[0x24];
    sdword unk040;
    AttackSideStep attackSideStep;
    ShipDamageLevelCounters firstSourceDamageLevelCounters;
    ShipDamageLevelCounters secondSourceDamageLevelCounters;
    union
    {
        struct
        {
            GrowSelection empShockedTargets;
            real32 empStateStartTime;
            ubyte empState;
            ubyte pad0B9[3];
            Ship *groupAttackTarget;
            bool32 groupAttackPending;
        };
        struct
        {
            ubyte unkAC_B3[0x08];
            real32 linkedAcolyteStateStartTime;
            sdword linkedAcolyteState;
            ubyte unkBC_C3[0x08];
        };
    };
    sdword firstSourceLinkEnabled;
    sdword secondSourceLinkEnabled;
    sdword firstSourceLinkState;
    sdword secondSourceLinkState;
} ACVSpecifics;

typedef struct
{
    udword unk0;
    real32 EMP_Max_Combine_Distance;
    real32 EMP_Range;
    real32 unk0C;
    AttackSideStepParameters attackSideStepParameters;
} ACVStatics;

static_assert(offsetof(ACVSpecifics, unk018) == 0x18, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, unk040) == 0x40, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, attackSideStep) == 0x44, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, firstSourceDamageLevelCounters) == 0x4C, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, secondSourceDamageLevelCounters) == 0x7C, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, empShockedTargets) == 0xAC, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, empStateStartTime) == 0xB4, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, linkedAcolyteStateStartTime) == 0xB4, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, empState) == 0xB8, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, linkedAcolyteState) == 0xB8, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, groupAttackTarget) == 0xBC, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, groupAttackPending) == 0xC0, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, firstSourceLinkEnabled) == 0xC4, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, secondSourceLinkEnabled) == 0xC8, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, firstSourceLinkState) == 0xCC, "ACVSpecifics layout mismatch");
static_assert(offsetof(ACVSpecifics, secondSourceLinkState) == 0xD0, "ACVSpecifics layout mismatch");
static_assert(sizeof(ACVSpecifics) == 0xD4, "ACVSpecifics size mismatch");

static_assert(offsetof(ACVStatics, EMP_Max_Combine_Distance) == 0x04, "ACVStatics layout mismatch");
static_assert(offsetof(ACVStatics, EMP_Range) == 0x08, "ACVStatics layout mismatch");
static_assert(offsetof(ACVStatics, unk0C) == 0x0C, "ACVStatics layout mismatch");
static_assert(offsetof(ACVStatics, attackSideStepParameters) == 0x10, "ACVStatics layout mismatch");
static_assert(sizeof(ACVStatics) == 0x1C, "ACVStatics size mismatch");

extern CustShipHeader ACVHeader;

#define acvShipSpecifics(ship) ((ACVSpecifics *)((ship)->ShipSpecifics))

bool32 acvCanTargetShipForEMP(Ship *ship, Ship *target);
void acvActivateEMP(GrowSelection *selection, Ship *ship);
bool32 shipIsACV(Ship *ship);
Ship *acvSelectEMPCenterShip(Ship *ship, bool32 useOnlyShip, GrowSelection *growSelect);
void acvIssueGroupEMPAttack(Ship *ship);
real32 acvGetEMPChargeProgress(Ship *ship);
bool32 acvIsEMPActive(Ship *ship);
GrowSelection *acvGetEMPSelection(Ship *ship);
bool32 acvTargetIsOwnedOrSalvagedByPlayer(sdword playerIndex, Ship *target);
bool32 acvIsDeploying(Ship *ship);
void acvInitSpecial(Ship *ship);

#endif
