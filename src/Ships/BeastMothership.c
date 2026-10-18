/*=============================================================================
    Name    : BeastMothership.c
    Purpose : Beast mothership-specific Cataclysm code and reference blocks
=============================================================================*/

#include "types.h"
#include "BigGun.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "BeastMothership.h"
#include "Mothership.h"
#include "SoundEvent.h"

typedef struct
{
    real32 mothershipGunRange[NUM_TACTICS_TYPES];
    real32 mothershipTooCloseRange[NUM_TACTICS_TYPES];
} BeastMothershipAttackStatics;

static BeastMothershipAttackStatics beastMothershipAttackStatics;

static_assert(offsetof(BeastMothershipSpec, modifierType) == 0x28, "BeastMothershipSpec modifierType offset mismatch");
static_assert(offsetof(BeastMothershipSpec, infectionLevel) == 0x2C, "BeastMothershipSpec infectionLevel offset mismatch");
static_assert(offsetof(BeastMothershipSpec, bigGunSpecifics) == 0x30, "BeastMothershipSpec bigGunSpecifics offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionState) == 0xAC, "BeastMothershipSpec destructionState offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionStateTime) == 0xB0, "BeastMothershipSpec destructionStateTime offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionCleanupTime) == 0xB4, "BeastMothershipSpec destructionCleanupTime offset mismatch");
static_assert(offsetof(BeastMothershipSpec, destructionDamageType) == 0xB8, "BeastMothershipSpec destructionDamageType offset mismatch");

static void BeastMothershipStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    (void)directory;
    (void)filename;

    statinfo->custstatinfo = &beastMothershipAttackStatics;
    if (shipStaticCanHaveBigGun(statinfo))
    {
        beastWeaponInfoInit();
    }
}

static void BeastMothershipInit(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoInit(ship);
    }
    if (ship->shiptype == bMothership)
    {
        real32 nextHyperspaceSpeechTime = universe.totaltimeelapsed + 1.0f;

        ship->statusWindow.phaseScale = -1.0f;
        ship->statusWindow.state = 1;
        ship->statusWindow.phaseStartTime = nextHyperspaceSpeechTime;
        ship->statusWindow.phaseEndTime = nextHyperspaceSpeechTime + SPEECH_HYPERSPACE_DELAY;
    }
}

static void BeastMothershipRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearTargetPointers(ship, shiptoremove);
    }
}

static void BeastMothershipBigGunNotifyObjectDeleted(Ship *ship, void *custom)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearQueuedPointers(ship, NULL, custom);
    }
}

static void BeastMothershipHousekeep(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        bigGunProcessTargetingAndFiring(ship);
    }
}

static void BeastMothershipClose(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClose(ship);
    }
}

static void BeastMothership_Save(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        SaveShipBigGunInfo(ship);
    }
}

static void BeastMothership_Load(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        LoadShipBigGunInfo(ship);
    }
}

static void BeastMothership_Fix(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        FixShipBigGunInfo(ship);
    }
}

CustShipHeader BeastMothershipHeader =
{
    sizeof(BeastMothershipSpec),
    BeastMothershipStaticInit,
    NULL,
    BeastMothershipInit,
    NULL,
    shipShootAllGunsAtTarget,
    NULL,
    NULL,
    shipBigGunSpecialTarget,
    MothershipSpecialOrderCleanup,
    BeastMothershipHousekeep,
    BeastMothershipRemoveShipReferences,
    BeastMothershipRemoveShipReferences,
    NULL,
    BeastMothershipBigGunNotifyObjectDeleted,
    BeastMothershipClose,
    NULL,
    BeastMothership_Save,
    BeastMothership_Load,
    BeastMothership_Fix
};
