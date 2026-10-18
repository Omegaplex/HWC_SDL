/*=============================================================================
    Name    : BeastMothership.h
    Purpose : Beast mothership Cataclysm-specific helpers
=============================================================================*/

#ifndef ___BEASTMOTHERSHIP_H
#define ___BEASTMOTHERSHIP_H

#include "Mothership.h"
#include "universe.h"

typedef struct BeastMothershipSpec
{
    ShipBigGunState bigGunState;               // 0x00
    ubyte unknown18_27[0x10];                  // 0x18
    BeastSystemType modifierType;              // 0x28
    sdword infectionLevel;                     // 0x2C
    MothershipBigGunSpecifics bigGunSpecifics; // 0x30
    ubyte unknown7C_AB[0x30];                  // 0x7C
    ubyte destructionState;                    // 0xAC
    ubyte unknownAD_AF[0x03];                  // 0xAD
    real32 destructionStateTime;               // 0xB0
    real32 destructionCleanupTime;             // 0xB4
    GunSoundType destructionDamageType;        // 0xB8
} BeastMothershipSpec;

/*
 * Retail bMothership stores the live beast-system allocation state in the
 * module-slot runtime tail at Ship+0x650/0x654, not in BeastMothershipSpec.
 */
typedef struct BeastMothershipRuntimeState
{
    ubyte unknown00_13[0x14];
    BeastSystemType systemType;                // Ship+0x650
    sdword infectionLevel;                     // Ship+0x654
} BeastMothershipRuntimeState;

static_assert(offsetof(BeastMothershipRuntimeState, systemType) == 0x14, "BeastMothershipRuntimeState systemType offset mismatch");
static_assert(offsetof(BeastMothershipRuntimeState, infectionLevel) == 0x18, "BeastMothershipRuntimeState infectionLevel offset mismatch");

static inline BeastMothershipRuntimeState *bMothershipRuntimeState(Ship *ship)
{
    return (BeastMothershipRuntimeState *)shipModuleSlotRuntime(ship);
}

static inline const BeastMothershipRuntimeState *bMothershipRuntimeStateConst(const Ship *ship)
{
    return (const BeastMothershipRuntimeState *)shipModuleSlotRuntimeConst(ship);
}

extern CustShipHeader BeastMothershipHeader;

BeastSystemType mothershipGetSystemType(Ship *ship);
void mothershipSetSystemType(Ship *ship, BeastSystemType system);
real32 mothershipGetDamageModifier(Ship *ship);
real32 mothershipGetResearchBonus(Ship *ship);
sdword mothershipGetInfectionLevel(Ship *ship);

#endif
