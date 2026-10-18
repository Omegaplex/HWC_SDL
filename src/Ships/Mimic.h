#ifndef ___MIMIC_H
#define ___MIMIC_H

#include "spaceobj.h"

extern CustShipHeader MimicHeader;

/*
 * Mimic morph state machine (MimicSpecifics::state).
 * Idle: module disabled, shoots normally via shipRotateAndShootAtTarget.
 * Morphed: module active, mimic is locked to sphere formation and activates link flags.
 */
#define MIMIC_STATE_IDLE    0  // not morphed; default fire behavior
#define MIMIC_STATE_MORPHED 1  // morphed into active target ship type

/*
 * Bits ORed into ship->specialFlags1C8 to signal that the mimic link is active
 * (bits 1 and 2; checked externally to enable coordinated fire).
 */
#define MIMIC_LINK_ACTIVE_FLAGS 0x6u

typedef struct
{
    LinkableSpecifics linkable;               // 0x00 - shared gun-state / relink-timer head
    struct Ship *linkPartner;            // 0x18 - current link partner during link/relink state
    sdword linkState;                    // 0x1C - link command state machine state
    sdword linkStateUnk20;               // 0x20 - preserved slot in the link command block
    sdword linkAnimationIndex;           // 0x24 - active link animation selector
    ubyte  reserved_28[0xA4];           // 0x28..0xCB

    // Active morph target: the ship configuration the mimic is currently imitating.
    sdword activeType;                   // 0xCC - ShipType of the active morph target (-1 = none)
    sdword activeRace;                   // 0xD0 - ShipRace of the active morph target (-1 = none)
    ubyte  reserved_D4[4];              // 0xD4..0xD7
    sdword activeBindings;              // 0xD8 - weapon binding count for the active configuration
    ubyte  reserved_DC[8];              // 0xDC..0xE3
    sdword activeSlotIndex;             // 0xE4 - module slot index of the active morph (-1 = none)
    sdword activeRadius;                 // 0xE8 - collision radius for the active ship type (-1 = none)
    ubyte  reserved_EC[12];             // 0xEC..0xF7

    // Pending morph target: the ship configuration being morphed to next.
    sdword pendingType;                  // 0xF8 - ShipType being morphed to (-1 = none)
    sdword pendingRace;                  // 0xFC - ShipRace being morphed to (-1 = none)
    ubyte  reserved_100[4];             // 0x100..0x103
    sdword pendingBindings;             // 0x104 - weapon binding count for the pending configuration
    ubyte  reserved_108[8];             // 0x108..0x10F
    sdword pendingSlotIndex;            // 0x110 - module slot index of the pending morph (-1 = none)
    sdword pendingRadius;               // 0x114 - collision radius for the pending ship type (-1 = none)
    ubyte  reserved_118[12];            // 0x118..0x123

    sdword morphProgress;               // 0x124 - progress counter for the current morph transition
    ubyte  state;                        // 0x128 - MIMIC_STATE_* morph state machine
    ubyte  reserved_129[11];            // 0x129..0x133
    sdword activeEffect;                 // 0x134 - active morph visual effect handle (0 = none)
    udword unk_138;                      // 0x138 - initialized to 0x40000000
} MimicSpecifics;

static_assert(offsetof(MimicSpecifics, linkable) == 0x00, "MimicSpecifics linkable offset mismatch");
static_assert(offsetof(MimicSpecifics, linkPartner) == 0x18, "MimicSpecifics linkPartner offset mismatch");
static_assert(offsetof(MimicSpecifics, linkState) == 0x1C, "MimicSpecifics linkState offset mismatch");
static_assert(offsetof(MimicSpecifics, linkStateUnk20) == 0x20, "MimicSpecifics linkStateUnk20 offset mismatch");
static_assert(offsetof(MimicSpecifics, linkAnimationIndex) == 0x24, "MimicSpecifics linkAnimationIndex offset mismatch");
static_assert(offsetof(MimicSpecifics, activeType) == 0xCC, "MimicSpecifics activeType offset mismatch");
static_assert(offsetof(MimicSpecifics, activeRace) == 0xD0, "MimicSpecifics activeRace offset mismatch");
static_assert(offsetof(MimicSpecifics, activeBindings) == 0xD8, "MimicSpecifics activeBindings offset mismatch");
static_assert(offsetof(MimicSpecifics, activeSlotIndex) == 0xE4, "MimicSpecifics activeSlotIndex offset mismatch");
static_assert(offsetof(MimicSpecifics, activeRadius) == 0xE8, "MimicSpecifics activeRadius offset mismatch");
static_assert(offsetof(MimicSpecifics, pendingType) == 0xF8, "MimicSpecifics pendingType offset mismatch");
static_assert(offsetof(MimicSpecifics, pendingRace) == 0xFC, "MimicSpecifics pendingRace offset mismatch");
static_assert(offsetof(MimicSpecifics, pendingBindings) == 0x104, "MimicSpecifics pendingBindings offset mismatch");
static_assert(offsetof(MimicSpecifics, pendingSlotIndex) == 0x110, "MimicSpecifics pendingSlotIndex offset mismatch");
static_assert(offsetof(MimicSpecifics, pendingRadius) == 0x114, "MimicSpecifics pendingRadius offset mismatch");
static_assert(offsetof(MimicSpecifics, morphProgress) == 0x124, "MimicSpecifics morphProgress offset mismatch");
static_assert(offsetof(MimicSpecifics, state) == 0x128, "MimicSpecifics state offset mismatch");
static_assert(offsetof(MimicSpecifics, activeEffect) == 0x134, "MimicSpecifics activeEffect offset mismatch");
static_assert(offsetof(MimicSpecifics, unk_138) == 0x138, "MimicSpecifics unk_138 offset mismatch");
static_assert(sizeof(MimicSpecifics) == 0x13C, "MimicSpecifics size mismatch");

static inline MimicSpecifics *mimicShipSpecifics(Ship *ship)
{
    return (MimicSpecifics *)ship->ShipSpecifics;
}

static inline const MimicSpecifics *mimicShipSpecificsConst(const Ship *ship)
{
    return (const MimicSpecifics *)ship->ShipSpecifics;
}

#endif
