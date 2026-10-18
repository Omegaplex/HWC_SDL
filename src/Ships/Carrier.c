/*=============================================================================
    Name    : Carrier.c
    Purpose : Specifics for the Carrier

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "Carrier.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "shipselect.h"
#include "repaircorvette.h"
#include "salcapcorvette.h"
#include "camera.h"
#include "BigGun.h"
#include "dock.h"
#include "meshanim.h"
#include "RepairDroid.h"
#include "Randy.h"
#include "SaveGame.h"
#include "Mothership.h"
#include "Dreadnought.h"
#include "universe.h"
#include "commandlayer.h"
#include "consmgr.h"

// Retail CarrierHeader slot 7 points at the imported telldir thunk.
bool32 telldir(Ship *ship);

typedef struct
{
    real32 carrierGunRange[NUM_TACTICS_TYPES];
    real32 carrierTooCloseRange[NUM_TACTICS_TYPES];
    real32 repairApproachDistance;
} CarrierStatics;

typedef struct
{
    ShipBigGunState bigGunState;
    SpaceObjRotImpTargGuidanceShipDerelict *droptarget[MAX_NUM_DROP];
    sdword dropstate[MAX_NUM_DROP];
    sdword dockindex[MAX_NUM_DROP];
    sdword CAP_NumInBuildCue;
    sdword CAPshiptype[CATA_CARRIER_MAX_CAPTURABLE];
    sdword CAPshiprace[CATA_CARRIER_MAX_CAPTURABLE];
    real32 CAPTimeToBuildShip[CATA_CARRIER_MAX_CAPTURABLE];
    sdword dockAnimationState;
    real32 dockAnimationTime;
} CarrierHeaderSpec;

CarrierStatics CarrierStatic;

static CarrierStatics CarrierStaticRace0;
CarrierStatics CarrierStaticRace1;
CarrierStatics CarrierStaticRace2;
static CarrierStatics CarrierStaticRace3;
static CarrierStatics CarrierStaticRace6;

static const real32 carrierHeaderHouseKeepRandomDelayScale = 1.1641532182693481e-9f; /* cataclysm.exe 0x72BF7C */
static const real32 carrierHeaderHouseKeepMinDelay = 5.0f;                            /* cataclysm.exe 0x72A624 */

static_assert(offsetof(CarrierHeaderSpec, dockAnimationState) == 0x160, "CarrierHeaderSpec dockAnimationState offset mismatch");
static_assert(offsetof(CarrierHeaderSpec, dockAnimationTime) == 0x164, "CarrierHeaderSpec dockAnimationTime offset mismatch");
static_assert(sizeof(CarrierHeaderSpec) == sizeof(CarrierSpec), "CarrierHeaderSpec size mismatch");

scriptStructEntry CStaticScriptTable[] =
{
    { "repairApproachDistance",    scriptSetReal32CB, (udword) &(CarrierStatic.repairApproachDistance), (udword) &(CarrierStatic) },

    { NULL,NULL,0,0 }
};

static CarrierHeaderSpec *carrierHeaderSpec(Ship *ship)
{
    return (CarrierHeaderSpec *)ship->ShipSpecifics;
}

typedef struct
{
    sdword count;
    Gun guns[1];
} CarrierHeaderGunDockInfo;

static CarrierStatics *carrierHeaderStaticsForRace(ShipStaticInfo *statinfo)
{
    switch (statinfo->shiprace)
    {
        case RACE_Sect:
            return &CarrierStaticRace0;
        case RACE_Beast:
            return &CarrierStaticRace1;
        case RACE_Kushan:
            return &CarrierStaticRace2;
        case RACE_Taiidan:
            return &CarrierStaticRace3;
        case RACE_Derelict:
            return &CarrierStaticRace6;
        default:
            return (CarrierStatics *)statinfo;
    }
}

static bool32 CarrierHeaderHasReservedMediumDockPoint(Ship *ship)
{
    DockPointActivity *dockPointActivity = ship->dockvars.dockPointActivity;
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    sdword i;

    if (dockPointActivity == NULL || dockStaticInfo == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < dockStaticInfo->numDockPoints; i++)
    {
        DockPointType dockPointType = dockStaticInfo->dockstaticpoints[i].type;

        if ((dockPointType == DPT_Med_In || dockPointType == DPT_Med_Out) &&
            dockPointActivity->reserved[i] != 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CarrierStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    udword i;
    CarrierStatics *carrstat = (statinfo->shiprace == RACE_Sect) ? &CarrierStaticRace1 : &CarrierStaticRace2;

    statinfo->custstatinfo = carrstat;

    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        carrstat->carrierGunRange[i] = statinfo->bulletRange[i];
        carrstat->carrierTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    scriptSetStruct(directory, filename, CStaticScriptTable, (ubyte *)carrstat);
}

void CarrierInit(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;

    spec->CAP_NumInBuildCue = 0;

    for (i = 0; i < MAX_NUM_DROP; i++)
    {
        spec->dropstate[i] = 0;
        spec->droptarget[i] = NULL;
        spec->dockindex[i] = 0;
    }
}

void CarrierAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;
    CarrierStatics *carrstat = (CarrierStatics *)shipstaticinfo->custstatinfo;

    attackStraightForward(ship, target, carrstat->carrierGunRange[ship->tacticstype], carrstat->carrierTooCloseRange[ship->tacticstype]);
    (void)maxdist;
}

void CarrierAttackPassive(Ship *ship, Ship *target, bool32 rotate)
{
#if 0
    if ((rotate) & ((bool)((ShipStaticInfo *)(ship->staticinfo))->rotateToRetaliate))
    {
        attackPassiveRotate(ship,target);
    }
    else
#endif
    attackPassive(ship, target);
}

bool32 CarrierSpecialTarget(Ship *ship, void *custom)
{
    (void)ship;
    (void)custom;
    return FALSE;
}

static void CarrierHeaderStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    CarrierStatics *carrstat = carrierHeaderStaticsForRace(statinfo);
    udword i;

    statinfo->canSpecialBandBoxFriendlies = FALSE;
    statinfo->unk4 = TRUE;
    statinfo->unk5 = FALSE;
    if (statinfo->shiprace == RACE_Beast)
    {
        statinfo->canSpecialBandBoxFriendlies = TRUE;
    }

    statinfo->custstatinfo = carrstat;
    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        carrstat->carrierGunRange[i] = statinfo->bulletRange[i];
        carrstat->carrierTooCloseRange[i] = statinfo->minBulletRange[i] * 0.9f;
    }
    scriptSetStruct(directory, filename, CStaticScriptTable, (ubyte *)carrstat);
    if (shipStaticCanHaveBigGun(statinfo))
    {
        dbgAssert(beastWeaponInfoInit());
    }
}

static void CarrierHeaderInit(Ship *ship)
{
    CarrierHeaderSpec *spec = carrierHeaderSpec(ship);

    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoInit(ship);
    }
    spec->dockAnimationState = 1;
    spec->dockAnimationTime = -1.0f;
    InitCrystalDockSlots(ship);
}

static void CarrierHeaderAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    attackSimple(ship, target);
    (void)maxdist;
}

static bool32 CarrierHeaderSpecialTarget(Ship *ship, void *custom)
{
    CarrierStatics *carrstat = (CarrierStatics *)ship->staticinfo->custstatinfo;
    SelectAnyCommand *targets = custom;
    SelectCommand *selection = custom;

    if (shipCanHaveBigGun(ship) &&
        bigGunHasActiveOrSelectableTarget(ship, selection))
    {
        return bigGunChooseTargetFromSelection(ship, selection);
    }

    return repairDroidAIUpdate(ship, targets, carrstat->repairApproachDistance);
}

static void CarrierHeaderNotifySpecialTarget(Ship *ship, void *custom)
{
    (void)custom;

    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearCurrentTarget(ship);
    }
}

void CarrierHouseKeep(Ship *ship)
{
    (void)ship;
}

static void CarrierHeaderHouseKeep(Ship *ship)
{
    CarrierHeaderSpec *spec = carrierHeaderSpec(ship);

    if (ship->shiptype == sCarrier)
    {
        if (CarrierHeaderHasReservedMediumDockPoint(ship))
        {
            if (spec->dockAnimationState == 1 && madAnimationCompleted(ship))
            {
                madAnimationStart(ship, 0);
                spec->dockAnimationState = 0;
            }
            spec->dockAnimationTime = -1.0f;
        }
        else if (spec->dockAnimationState != 0)
        {
            spec->dockAnimationTime = -1.0f;
        }
        else if (madAnimationCompleted(ship))
        {
            if (spec->dockAnimationTime < 0.0f)
            {
                spec->dockAnimationTime =
                    ((real32)ranRandom(10) * carrierHeaderHouseKeepRandomDelayScale) +
                    carrierHeaderHouseKeepMinDelay +
                    universe.totaltimeelapsed;
            }
            else if (spec->dockAnimationTime <= universe.totaltimeelapsed)
            {
                madAnimationStart(ship, 1);
                spec->dockAnimationState = 1;
                spec->dockAnimationTime = -1.0f;
            }
        }
    }

    if (shipCanHaveBigGun(ship))
    {
        bigGunProcessTargetingAndFiring(ship);
    }
    ProcessCrystalDockSlots(ship);
}

static void CarrierHeaderRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearTargetPointers(ship, shiptoremove);
    }
    RemoveShipFromCrystalDockSlots(ship, shiptoremove);
}

void shipBigGunNotifyObjectDeleted(Ship *ship, void *deadObject)
{
    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClearQueuedPointers(ship, NULL, deadObject);
    }
}

static void CarrierHeaderClose(Ship *ship)
{
    SelectCommand *selection;
    sdword i;

    if (shipCanHaveBigGun(ship))
    {
        shipBigGunInfoClose(ship);
    }
    ClearCrystalDockSlots(ship);

    if (ship->clampInfo == NULL || ship->clampInfo->selection == NULL)
    {
        return;
    }

    selection = (SelectCommand *)ship->clampInfo->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        Ship *clampedShip = selection->ShipPtr[i];

        if (clampedShip->staticinfo->shiptype == sDreadnought)
        {
            bitSet(clampedShip->flags2, SOF2_Unknown4);
        }
    }
}

static void CarrierHeader_PreFix(Ship *ship)
{
    PreFix_CrystalDockSlots(ship);
}

static void CarrierHeader_Save(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        SaveShipBigGunInfo(ship);
    }
}

static void CarrierHeader_Load(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        LoadShipBigGunInfo(ship);
    }
}

static void CarrierHeader_Fix(Ship *ship)
{
    if (shipCanHaveBigGun(ship))
    {
        FixShipBigGunInfo(ship);
    }
    Fix_CrystalDockSlots(ship);
}

void shipShootAllGunsAtTarget(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    CarrierHeaderGunDockInfo *customdockinfo = ship->dockvars.customdockinfo;
    Gun *gun;
    sdword i;
    (void)maxdist;

    if (customdockinfo == NULL || customdockinfo->count <= 0)
    {
        return;
    }

    gun = &customdockinfo->guns[0];
    for (i = 0; i < customdockinfo->count; i++, gun++)
    {
        if (!gunCanShootStaggered(ship, gun))
        {
            continue;
        }

        switch (gun->gunstatic->guntype)
        {
            case GUN_Fixed:
            case GUN_Gimble:
            case GUN_NewGimble:
                gunShootGunAtTarget(ship, gun, target);
                break;

            case GUN_MissileLauncher:
            case GUN_MineLauncher:
                if (gun->numMissiles > 0)
                {
                    missileShoot(ship, gun, target);
                }
                break;

            default:
                break;
        }
    }
}

void CarrierRemoveShipReferences(Ship *ship, Ship *shiptoremove)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for (i = 0; i < MAX_NUM_DROP; i++)
    {
        if (shiptoremove == (Ship *)spec->droptarget[i])
        {
            if (ship->dockInfo != NULL)
            {
                if (ship->dockInfo->dockpoints[spec->dockindex[i]].thisDockBusy)
                {
                    ship->dockInfo->dockpoints[spec->dockindex[i]].thisDockBusy--;
                }
            }
            spec->droptarget[i] = NULL;
            break;
        }
    }
}

#pragma warning( 4 : 4047 )
void Carrier_PreFix(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for (i = 0; i < MAX_NUM_DROP; i++)
    {
        spec->droptarget[i] = SpaceObjRegistryGetID((SpaceObj *)spec->droptarget[i]);
    }
}

void Carrier_Fix(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for (i = 0; i < MAX_NUM_DROP; i++)
    {
        spec->droptarget[i] = (SpaceObjRotImpTargGuidanceShipDerelict *)SpaceObjRegistryGetObj((sdword)spec->droptarget[i]);
    }
    FixShipBigGunInfo(ship);
}
#pragma warning( 2 : 4047 )

void CarrierDied(Ship *ship)
{
    CarrierSpec *spec = (CarrierSpec *)ship->ShipSpecifics;
    sdword i;
    for (i = 0; i < MAX_NUM_DROP; i++)
    {
        if (spec->droptarget[i] != NULL)
        {
            if (spec->droptarget[i]->objtype == OBJ_ShipType)
            {
                if (!bitTest(((SpaceObjRotImpTargGuidanceShipDerelict *)spec->droptarget[i])->specialFlags2, SPECIAL_2_BIT7_UNAUDITED))
                {
                    bitClear(spec->droptarget[i]->flags, SOF_Disabled);
                    bitClear(spec->droptarget[i]->specialFlags, SPECIAL_SalvagedTargetGoingIntoDockWithShip);
                    bitSet(spec->droptarget[i]->flags, SOF_Selectable);
                }
            }
        }
    }
}

CustShipHeader CarrierHeader =
{
    sizeof(CarrierHeaderSpec),
    CarrierHeaderStaticInit,
    NULL,
    CarrierHeaderInit,
    NULL,
    shipShootAllGunsAtTarget,
    CarrierHeaderAttack,
    telldir,
    CarrierHeaderSpecialTarget,
    CarrierHeaderNotifySpecialTarget,
    CarrierHeaderHouseKeep,
    CarrierHeaderRemoveShipReferences,
    CarrierHeaderRemoveShipReferences,
    NULL,
    shipBigGunNotifyObjectDeleted,
    CarrierHeaderClose,
    Mothership_PreFix,
    Mothership_Save,
    Mothership_Load,
    CarrierHeader_Fix,
    NULL,
    shipStraightForwardAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
