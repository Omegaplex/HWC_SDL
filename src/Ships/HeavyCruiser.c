#include "types.h"
#include "SpaceObj.h"
#include "Ship.h"
#include "HeavyCruiser.h"
#include "attack.h"
#include "camera.h"
#include "Carrier.h"
#include "DefaultShip.h"
#include "BigGun.h"
#include "dock.h"
#include "Mothership.h"
#include "P1Mothership.h"
#include "statscript.h"
#include "universe.h"
#include "Dreadnought.h"

typedef struct BeastHeavyCruiserStatics
{
    real32 gunRange[NUM_TACTICS_TYPES];
    real32 tooCloseRange[NUM_TACTICS_TYPES];
} BeastHeavyCruiserStatics;

static BeastHeavyCruiserStatics beastHeavyCruiserStaticsBeast;
static BeastHeavyCruiserStatics beastHeavyCruiserStaticsKushan;
static BeastHeavyCruiserStatics beastHeavyCruiserStaticsTaiidan;

void BeastHeavyCruiserStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo)
{
    BeastHeavyCruiserStatics *statics;
    udword i;

    (void)directory;
    (void)filename;

    switch (statinfo->shiprace)
    {
        case RACE_Beast:
            statics = &beastHeavyCruiserStaticsBeast;
            break;
        case RACE_Kushan:
            statics = &beastHeavyCruiserStaticsKushan;
            break;
        case RACE_Taiidan:
            statics = &beastHeavyCruiserStaticsTaiidan;
            break;
        default:
            statics = &beastHeavyCruiserStaticsBeast;
            break;
    }

    statinfo->custstatinfo = statics;
    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        statics->gunRange[i] = statinfo->bulletRange[i];
        statics->tooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    if (shipStaticCanHaveBigGun(statinfo))
    {
        beastWeaponInfoInit();
    }
}

bool32 shipBigGunSpecialTarget(Ship *ship, void *custom)
{
    if (shipCanHaveBigGun(ship))
    {
        return bigGunChooseTargetFromSelection(ship, (SelectCommand *)custom);
    }
    return TRUE;
}

static void HeavyCruiserRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    if (ship->shiptype == bHeavyCruiser)
    {
        RemoveShipFromCrystalDockSlots(ship, shiptoremove);
    }
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearTargetPointers(ship, shiptoremove);
    }
}

static void BeastHeavyCruiserInit(Ship *ship)
{
    if (ship->shiptype == bHeavyCruiser)
    {
        InitCrystalDockSlots(ship);
    }
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoInit(ship);
    }
}

static void BeastHeavyCruiserHouseKeep(Ship *ship)
{
    if (ship->shiptype == bHeavyCruiser)
    {
        ProcessCrystalDockSlots(ship);
    }
    if (shipCanHaveBigGun(ship))
    {
        bigGunProcessTargetingAndFiring(ship);
    }
}

static void BeastHeavyCruiserClose(Ship *ship)
{
    if (ship->shiptype == bHeavyCruiser)
    {
        ClearCrystalDockSlots(ship);
    }
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClose(ship);
    }
}

static void HeavyCruiser_PreFix(Ship *ship)
{
    if (ship->shiptype == bHeavyCruiser)
    {
        PreFix_CrystalDockSlots(ship);
    }
}

static void BeastHeavyCruiser_Fix(Ship *ship)
{
    if (ship->shiptype == bHeavyCruiser)
    {
        Fix_CrystalDockSlots(ship);
    }
    if (shipCanHaveBigGun(ship))
    {
        FixShipBigGunInfo(ship);
    }
}

bool32 HeavyCruiserAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;

    attackStraightForward(
        ship,
        target,
        shipstaticinfo->bulletRange[ship->tacticstype],
        shipstaticinfo->minBulletRange[ship->tacticstype] * 0.9f);
    (void)maxdist;
    return TRUE;
}

CustShipHeader HeavyCruiserHeader =
{
    sizeof(HeavyCruiserSpec),
    BeastHeavyCruiserStaticInit,
    NULL,
    BeastHeavyCruiserInit,
    NULL,
    shipShootAllGunsAtTarget,
    P1MothershipRotateAndShoot,
    NULL,
    shipBigGunSpecialTarget,
    MothershipSpecialOrderCleanup,
    BeastHeavyCruiserHouseKeep,
    HeavyCruiserRemoveShipReferences,
    HeavyCruiserRemoveShipReferences,
    NULL,
    shipBigGunNotifyObjectDeleted,
    BeastHeavyCruiserClose,
    HeavyCruiser_PreFix,
    Mothership_Save,
    Mothership_Load,
    BeastHeavyCruiser_Fix,
    NULL,
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

CustShipHeader BeastHeavyCruiserHeader =
{
    sizeof(HeavyCruiserSpec),
    BeastHeavyCruiserStaticInit,
    NULL,
    BeastHeavyCruiserInit,
    NULL,
    shipShootAllGunsAtTarget,
    P1MothershipRotateAndShoot,
    NULL,
    shipBigGunSpecialTarget,
    MothershipSpecialOrderCleanup,
    BeastHeavyCruiserHouseKeep,
    HeavyCruiserRemoveShipReferences,
    HeavyCruiserRemoveShipReferences,
    NULL,
    shipBigGunNotifyObjectDeleted,
    BeastHeavyCruiserClose,
    HeavyCruiser_PreFix,
    Mothership_Save,
    Mothership_Load,
    BeastHeavyCruiser_Fix,
    NULL,
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
