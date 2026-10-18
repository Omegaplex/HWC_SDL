/*=============================================================================
    Name    : Acolyte.c
    Purpose : Acolyte-specific retail callbacks
=============================================================================*/

#include <math.h>
#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "DefaultShip.h"
#include "Ship.h"
#include "gun.h"
#include "meshanim.h"
#include "commandlayer.h"
#include "Dreadnought.h"
#include "Randy.h"
#include "SaveGame.h"
#include "universe.h"

static const real32 acolyteLinkFireRangeSquared = 16000000.0f;                 /* cataclysm.exe 0x8C3258 */
static const real32 acolyteLinkDelayScale = -4.6566128730773926e-10f;          /* 0x72A648 * 0x72A64C */
static const real32 acolyteWhiteoutFadeDelay = 3.0f;                           /* matches retail whiteoutFadeDelay */
static const real64 acolyteIdleAnimationExtraDelay = 2.0;                      /* cataclysm.exe 0x72A5C0 */
static const char acolyteLinkEffectSaveMessage[] = "LINKING: Acolyte linking effect not being saved.";
static const sdword acolyteLinkGunBaseIndex = 2;
static const sdword acolyteLinkGunCount = 2;

static bool32 acolyteIsShipType(ShipType shipType)
{
    return (shipType == sAcolyte) || (shipType == bAcolyte);
}

static real32 acolyteRandomAnimationDelay(real64 extraDelay)
{
    return (real32)((1.0 - exp((real64)ranRandom(RAN_ETG) * acolyteLinkDelayScale)) * acolyteWhiteoutFadeDelay + extraDelay);
}

static void AcolyteShootNextGunAtTarget(Ship *ship, SpaceObjRotImpTarg *target)
{
    shipShootNextGunAtTarget(ship, target, 0.0f);
}

static void AcolyteStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    shipAttackStaticInfoScript(directory, filename, statinfo);
    // Retail writes bytes 529-531 of ShipStaticInfo after calling shipAttackStaticInfoScript;
    // these are the canSpecialBandBoxFriendlies/unk4/unk5 bool8 fields, not repairBeamCapable.
    statinfo->canSpecialBandBoxFriendlies = FALSE;
    statinfo->unk4 = TRUE;
    statinfo->unk5 = FALSE;
}

static void AcolyteInit(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    shipResetAttackState(ship);
    gunStateSpecifics(ship)->currentGunIndex = 0;
    specifics->linkPartner = NULL;
    specifics->linkState = 0;
    specifics->linkStateUnk20 = 0;
    specifics->linkAnimationIndex = 0;
    specifics->linkAnimationState = 1;
    specifics->linkAnimationWanted = 0;
    specifics->linkAnimationDelayArmed = 0;
    specifics->linkEffect = NULL;
    specifics->linkTarget = NULL;
    specifics->linkShotPending = 0;
    specifics->linkedGroupAttackPending = 0;
}

void acolyteStopLinking(Ship *ship)
{
    if (!acolyteIsShipType(ship->shiptype))
    {
        return;
    }

    ship->cataclysmGunInfo->guns[acolyteLinkGunBaseIndex + 0].numMissiles = 0;
    ship->cataclysmGunInfo->guns[acolyteLinkGunBaseIndex + 1].numMissiles = 0;
}

bool32 acolyteCanLink(Ship *ship)
{
    return acolyteIsShipType(ship->shiptype) && ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE);
}

void acolyteStartLinking(Ship *ship)
{
    AcolyteSpecifics *specifics;

    if (!acolyteIsShipType(ship->shiptype))
    {
        return;
    }

    specifics = acolyteShipSpecifics(ship);
    specifics->linkTarget = NULL;
    specifics->linkShotPending = 0;
    specifics->linkedGroupAttackPending = 0;
    ship->cataclysmGunInfo->guns[acolyteLinkGunBaseIndex + 0].numMissiles = 1;
    ship->cataclysmGunInfo->guns[acolyteLinkGunBaseIndex + 1].numMissiles = 1;
}

sdword acolyteGetLinkState(Ship *ship)
{
    if (!acolyteCanLink(ship))
    {
        return 0;
    }

    if (ship->cataclysmGunInfo->guns[acolyteLinkGunBaseIndex + 0].numMissiles > 0 ||
        ship->cataclysmGunInfo->guns[acolyteLinkGunBaseIndex + 1].numMissiles > 0)
    {
        return 2;
    }

    return 0;
}

void acolyteLinkedGroupAttack(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);
    SelectCommand *groupSelection;
    MaxSelection selection;
    AttackCommand attackCommand;
    sdword i;

    if (ship->command == NULL ||
        (ship->command->ordertype.attributes & COMMAND_IS_FORMATION) == 0 ||
        ship->command->selection == NULL ||
        ship->command->selection->numShips <= 1)
    {
        return;
    }

    groupSelection = ship->command->selection;
    selection.numShips = 0;
    selection.timeLastStatus = 0.0f;

    for (i = 0; i < groupSelection->numShips; i++)
    {
        Ship *groupShip = groupSelection->ShipPtr[i];
        AcolyteSpecifics *groupSpecifics;

        if (!acolyteIsShipType(groupShip->shiptype))
        {
            continue;
        }

        groupSpecifics = acolyteShipSpecifics(groupShip);
        if (groupSpecifics->linkedGroupAttackPending == 0 ||
            groupSpecifics->linkShotPending == 0 ||
            groupSpecifics->linkTarget != specifics->linkTarget)
        {
            continue;
        }

        selection.ShipPtr[selection.numShips++] = groupShip;
        groupSpecifics->linkedGroupAttackPending = 0;
    }

    if (selection.numShips <= 0)
    {
        return;
    }

    attackCommand.numTargets = 1;
    attackCommand.timeLastStatus = 0.0f;
    attackCommand.TargetPtr[0] = specifics->linkTarget;
    clAttack(&universe.mainCommandLayer, (SelectCommand *)&selection, &attackCommand);
}

static void AcolyteAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);
    GunInfo *gunInfo = ship->cataclysmGunInfo;

    if (ship->shiptype != dAcolyte ||
        (specifics->linkAnimationState == 0 && madAnimationCompleted(ship)))
    {
        AcolyteShootNextGunAtTarget(ship, target);
    }

    if (acolyteIsShipType(ship->shiptype) &&
        specifics->linkAnimationState == 0 &&
        madAnimationCompleted(ship) &&
        specifics->linkShotPending == 1 &&
        ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE) &&
        gunInfo != NULL &&
        (gunInfo->guns[acolyteLinkGunBaseIndex + 0].numMissiles > 0 ||
         gunInfo->guns[acolyteLinkGunBaseIndex + 1].numMissiles > 0))
    {
        Gun *gun = &gunInfo->guns[acolyteLinkGunBaseIndex];
        sdword i;

        for (i = 0; i < acolyteLinkGunCount; i++)
        {
            if (gunCanShootStaggered(ship, gun) && gun->numMissiles > 0)
            {
                gunShoot(ship, gun, target);
            }
            gun++;
        }
    }

    ship->shipisattacking = TRUE;
    (void)maxdist;
}

static bool32 AcolyteClearPendingLinkTarget(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    specifics->linkedGroupAttackPending = 0;
    specifics->linkShotPending = 0;
    specifics->linkTarget = NULL;
    return TRUE;
}

static bool32 AcolyteRetargetAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    if ((Ship *)target != specifics->linkPartner)
    {
        specifics->linkStateUnk20 = 0;
        specifics->linkState = 0;
        specifics->linkPartner = NULL;
    }

    return shipSharedFlightManAttackCommand(ship, target, maxdist);
}

static bool32 AcolyteSpecialTarget(Ship *ship, void *custom)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);
    SelectAnyCommand *targets = custom;

    if (targets->numTargets > 0)
    {
        specifics->linkShotPending = 1;
        specifics->linkedGroupAttackPending = 1;
        specifics->linkTarget = targets->TargetPtr[0];
    }

    return TRUE;
}

static void AcolyteHouseKeep(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);
    GunInfo *gunInfo = ship->cataclysmGunInfo;

    if (specifics->linkedGroupAttackPending == 1)
    {
        SelectCommand selection;
        AttackCommand attackCommand;

        acolyteLinkedGroupAttack(ship);
        if (specifics->linkedGroupAttackPending == 1)
        {
            selection.numShips = 1;
            selection.timeLastStatus = 0.0f;
            selection.ShipPtr[0] = ship;
            attackCommand.numTargets = 1;
            attackCommand.timeLastStatus = 0.0f;
            attackCommand.TargetPtr[0] = specifics->linkTarget;
            specifics->linkedGroupAttackPending = 0;
            clAttack(&universe.mainCommandLayer, &selection, &attackCommand);
        }
    }

    if (acolyteIsShipType(ship->shiptype) &&
        specifics->linkShotPending != 0 &&
        specifics->linkTarget != NULL &&
        ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE) &&
        gunInfo != NULL &&
        (gunInfo->guns[acolyteLinkGunBaseIndex + 0].numMissiles > 0 ||
         gunInfo->guns[acolyteLinkGunBaseIndex + 1].numMissiles > 0))
    {
        vector delta;

        vecSub(delta, ship->posinfo.position, specifics->linkTarget->posinfo.position);
        if (vecMagnitudeSquared(delta) <= acolyteLinkFireRangeSquared &&
            specifics->linkAnimationState == 0 &&
            madAnimationCompleted(ship) &&
            specifics->linkShotPending == 1)
        {
            Gun *gun = &gunInfo->guns[acolyteLinkGunBaseIndex];
            sdword i;

            for (i = 0; i < acolyteLinkGunCount; i++)
            {
                if (gunCanShootStaggered(ship, gun) && gun->numMissiles > 0)
                {
                    gunShoot(ship, gun, specifics->linkTarget);
                }
                gun++;
            }
        }
    }

    if (ship->shipisattacking)
    {
        if (specifics->linkAnimationState != 0)
        {
            if (!specifics->linkAnimationDelayArmed)
            {
                specifics->linkAnimationDelayArmed = 1;
                specifics->linkAnimationDelayUntil = universe.totaltimeelapsed + acolyteRandomAnimationDelay(0.0);
                return;
            }

            if (specifics->linkAnimationDelayUntil < universe.totaltimeelapsed)
            {
                madAnimationStart(ship, 0); /* drill/link animation */
                specifics->linkAnimationState = 0;
                specifics->linkAnimationWanted = 1;
                specifics->linkAnimationDelayArmed = 0;
            }
            return;
        }

        specifics->linkAnimationDelayArmed = 0;
        return;
    }

    if (!specifics->linkAnimationWanted)
    {
        if (specifics->linkAnimationDelayArmed)
        {
            specifics->linkAnimationDelayArmed = 0;
        }
        return;
    }

    if (specifics->linkAnimationState == 1)
    {
        specifics->linkAnimationDelayArmed = 0;
        return;
    }

    if (!specifics->linkAnimationDelayArmed)
    {
        specifics->linkAnimationDelayArmed = 1;
        specifics->linkAnimationDelayUntil = universe.totaltimeelapsed + acolyteRandomAnimationDelay(acolyteIdleAnimationExtraDelay);
        return;
    }

    if (specifics->linkAnimationDelayUntil < universe.totaltimeelapsed)
    {
        madAnimationStart(ship, 1); /* idle animation */
        specifics->linkAnimationState = 1;
        specifics->linkAnimationWanted = 0;
        specifics->linkAnimationDelayArmed = 0;
    }
}

static void AcolyteRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    if (specifics->linkTarget == (SpaceObjRotImpTarg *)shipToRemove)
    {
        specifics->linkedGroupAttackPending = 0;
        specifics->linkShotPending = 0;
        specifics->linkTarget = NULL;
    }
}

static void Acolyte_PreFix(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    specifics->linkTarget = (SpaceObjRotImpTarg *)SpaceObjRegistryGetID((SpaceObj *)specifics->linkTarget);
    specifics->linkPartner = (Ship *)SpaceObjRegistryGetID((SpaceObj *)specifics->linkPartner);
    if (specifics->linkEffect != NULL)
    {
        dbgMessagef("\n%s", acolyteLinkEffectSaveMessage);
        specifics->linkEffect = NULL;
    }
}

static void Acolyte_Fix(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    specifics->linkTarget = (SpaceObjRotImpTarg *)SpaceObjRegistryGetShip((sdword)specifics->linkTarget);
    specifics->linkPartner = SpaceObjRegistryGetShip((sdword)specifics->linkPartner);
}

static bool32 AcolyteCanHalt(Ship *ship)
{
    AcolyteSpecifics *specifics = acolyteShipSpecifics(ship);

    specifics->linkedGroupAttackPending = 0;
    specifics->linkShotPending = 0;
    specifics->linkTarget = NULL;

    // The Cataclysm link command blocks halting while the merge is in progress.
    if (ship->command != NULL && ship->command->ordertype.order == COMMAND_LINK)
    {
        return FALSE;
    }

    return specifics->linkPartner == NULL;
}

CustShipHeader AcolyteHeader =
{
    sizeof(AcolyteSpecifics),
    AcolyteStaticInit,
    NULL,
    AcolyteInit,
    NULL,
    AcolyteAttack,
    shipRotateAndShootAtTarget,
    AcolyteClearPendingLinkTarget,
    AcolyteSpecialTarget,
    NULL,
    AcolyteHouseKeep,
    AcolyteRemoveShipReferences,
    AcolyteRemoveShipReferences,
    NULL,
    NULL,
    NULL,
    Acolyte_PreFix,
    NULL,
    NULL,
    Acolyte_Fix,
    (bool32 (*)(Ship *, vector *, vector *))AcolyteClearPendingLinkTarget,
    AcolyteRetargetAttack,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    AcolyteCanHalt,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
