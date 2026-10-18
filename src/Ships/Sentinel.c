/*=============================================================================
    Name    : Sentinel.c
    Purpose : Specifics for the Sentinel

    Created for Cataclysm retail parity recovery.
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "attack.h"
#include "FastMath.h"
#include "gun.h"
#include "meshanim.h"
#include "universe.h"
#include "Clamp.h"
#include "CommandDefs.h"
#include "CommandLayer.h"
#include "DefaultShip.h"
#include "SaveGame.h"
#include "formation.h"
#include "Sentinel.h"

sdword pingEvalAlwaysTrue(void);

typedef struct
{
    sdword count;
    Gun guns[1];
} SentinelGunDockInfo;

typedef struct
{
    AttackSideStepParameters sidestepParameters;
    sbyte unknown[64 - sizeof(AttackSideStepParameters)];
} SentinelStatics;

static SentinelStatics SentinelStaticRace1;
static SentinelStatics SentinelStaticRace2;

#define SENTINEL_FORCEFIELD_UNKNOWN_FLAG_OFFSET 0x528

static const real32 sentinelForcefieldArmorByLevel[] = { 4000.0f, 6000.0f, 10000.0f, 4.0f };
static const real32 sentinelForcefieldArmorStepByLevel[] = { 4.0f, 8.0f, 16.0f, 100.0f };
static const real32 sentinelForcefieldPositionTolerance = 50.0f;
static const real32 sentinelForcefieldChargeRate = 0.2f;
static const real32 sentinelForcefieldPulseDecayRate = 0.6f;

static void SentinelStaticInit(char *directory, char *field, ShipStaticInfo *statinfo)
{
    SentinelStatics *statics;

    statics = (statinfo->shiprace == RACE_Beast) ? &SentinelStaticRace2 : &SentinelStaticRace1;
    scriptSetStruct(directory, field, AttackSideStepParametersScriptTable, &statics->sidestepParameters);
    statinfo->custstatinfo = statics;
    statinfo->builtInUpgradeMask |= 0x1000;
    statinfo->canSpecialBandBoxFriendlies = TRUE;
}

static bool32 SentinelFormationFilter(Ship *ship, sdword formation, Ship *leader)
{
    (void)leader;

    if (formation == FORCEFIELD_WALL_FORMATION || formation == FORCEFIELD_SPHERE_FORMATION)
    {
        ship->specialFlags2 |= 0x1000;
        madAnimationStart(ship, 0);
    }
    else
    {
        ship->specialFlags2 &= ~0x1000;
        madAnimationStart(ship, 1);
    }

    return TRUE;
}

static void SentinelInit(Ship *ship)
{
    SentinelSpecifics *spec = sentinelShipSpecifics(ship);

    spec->unk0 = 0;
    spec->forcefieldChargeTimer = 0.0f;
    spec->formationLinkCount = 0;
    spec->forcefieldPulseStartTime = 0.0f;
    spec->unk338 = 0;
    spec->unk344 = 0;
    spec->unk348 = 0;
}

static void SentinelRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    SentinelSpecifics *spec = sentinelShipSpecifics(ship);
    sdword i = 0;

    while (i < spec->formationLinkCount)
    {
        Ship **pair = &spec->formationLinks[i * 2];
        if (pair[0] == shipToRemove || pair[1] == shipToRemove)
        {
            sdword lastIndex = spec->formationLinkCount - 1;
            if (i < lastIndex)
            {
                pair[0] = spec->formationLinks[lastIndex * 2];
                pair[1] = spec->formationLinks[lastIndex * 2 + 1];
            }
            spec->formationLinkCount--;
        }
        else
        {
            i++;
        }
    }
}

static void SentinelPreFix(Ship *ship)
{
    SentinelSpecifics *spec = sentinelShipSpecifics(ship);
    sdword i;

    for (i = 0; i < spec->formationLinkCount; i++)
    {
        spec->formationLinks[i * 2] = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->formationLinks[i * 2]);
        spec->formationLinks[i * 2 + 1] = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->formationLinks[i * 2 + 1]);
    }
}

static void SentinelFix(Ship *ship)
{
    SentinelSpecifics *spec = sentinelShipSpecifics(ship);
    sdword i;

    for (i = 0; i < spec->formationLinkCount; i++)
    {
        spec->formationLinks[i * 2] = SpaceObjRegistryGetShip((sdword)spec->formationLinks[i * 2]);
        spec->formationLinks[i * 2 + 1] = SpaceObjRegistryGetShip((sdword)spec->formationLinks[i * 2 + 1]);
    }
}

static bool32 SentinelSpecialOrder(Ship *ship, void *custom)
{
    attackStraightForward(ship, (SpaceObjRotImpTarg *)custom, ship->staticinfo->bulletRange[ship->tacticstype], 1000.0f);
    return TRUE;
}

static void SentinelAttack(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    SentinelGunDockInfo *customdockinfo = (SentinelGunDockInfo *)ship->dockvars.customdockinfo;
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
            case GUN_MissileLauncher:
                gunShootGunAtTarget(ship, gun, target);
                break;

            case GUN_HiddenGimble:
            case GUN_VisibleGimble:
                if (gun->numMissiles > 0)
                {
                    gunShoot(ship, gun, target);
                }
                break;

            default:
                break;
        }
    }
}

static void SentinelRotateAndShoot(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    CommandToDo *command = ship->command;

    if (command != NULL &&
        (command->ordertype.attributes & COMMAND_IS_FORMATION) != 0 &&
        command->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
    {
        return;
    }

    if ((ship->specialFlags3 & rotateFlags) != 0)
    {
        attackPassive(ship, (Ship *)target);
    }
    else
    {
        attackSimple(ship, target);
    }
}

static void SentinelForcefieldUpdate(Ship *ship)
{
    CommandToDo *command = ship->command;
    SelectCommand *selection;
    Ship *targetShip;
    TypeOfFormation formationType;
    sdword i;

    if (command == NULL)
    {
        sentinelShipSpecifics(ship)->unk0 = 0;
        clDetachShipFromCommand(ship);
        return;
    }

    formationType = command->formation.formationtype;
    if ((command->ordertype.attributes & COMMAND_IS_FORMATION) == 0 ||
        (formationType != FORCEFIELD_WALL_FORMATION && formationType != FORCEFIELD_SPHERE_FORMATION) ||
        command->selection->ShipPtr[0] != ship)
    {
        if (formationType == FORCEFIELD_SPHERE_FORMATION)
        {
            return;
        }
        sentinelShipSpecifics(ship)->unk0 = 0;
        clDetachShipFromCommand(ship);
        return;
    }

    targetShip = ship;
    if (formationType == FORCEFIELD_SPHERE_FORMATION)
    {
        if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            targetShip = command->protect->ShipPtr[0];
        }
        else
        {
            command->formation.formationtype = FORCEFIELD_WALL_FORMATION;
            formationContentHasChanged(command);
        }

        if (targetShip == NULL)
        {
            command->formation.formationtype = FORCEFIELD_WALL_FORMATION;
            formationContentHasChanged(command);
            targetShip = ship;
        }
    }

    selection = command->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        Ship *member = selection->ShipPtr[i];
        SentinelSpecifics *memberSpec = sentinelShipSpecifics(member);
        vector rotatedOffset;
        vector unrotatedOffset;
        vector leaderRotatedOffset;
        real32 distance;
        real32 memberCollDistance;
        real32 positionDistance;
        real32 unrotatedDistance;
        real32 leaderDistance;

        memberSpec->unk0 = 0;
        matMultiplyMatByVec(&rotatedOffset, &targetShip->rotinfo.coordsys, &member->formationOffset);
        unrotatedOffset = member->formationOffset;
        matMultiplyMatByVec(&leaderRotatedOffset, &selection->ShipPtr[0]->rotinfo.coordsys, &member->formationOffset);
        distance = fsqrt(
            (member->posinfo.position.x - (targetShip->collInfo.collPosition.x + rotatedOffset.x)) *
            (member->posinfo.position.x - (targetShip->collInfo.collPosition.x + rotatedOffset.x)) +
            (member->posinfo.position.y - (targetShip->collInfo.collPosition.y + rotatedOffset.y)) *
            (member->posinfo.position.y - (targetShip->collInfo.collPosition.y + rotatedOffset.y)) +
            (member->posinfo.position.z - (targetShip->collInfo.collPosition.z + rotatedOffset.z)) *
            (member->posinfo.position.z - (targetShip->collInfo.collPosition.z + rotatedOffset.z)));
        memberCollDistance = fsqrt(
            (member->collInfo.collPosition.x - (targetShip->collInfo.collPosition.x + rotatedOffset.x)) *
            (member->collInfo.collPosition.x - (targetShip->collInfo.collPosition.x + rotatedOffset.x)) +
            (member->collInfo.collPosition.y - (targetShip->collInfo.collPosition.y + rotatedOffset.y)) *
            (member->collInfo.collPosition.y - (targetShip->collInfo.collPosition.y + rotatedOffset.y)) +
            (member->collInfo.collPosition.z - (targetShip->collInfo.collPosition.z + rotatedOffset.z)) *
            (member->collInfo.collPosition.z - (targetShip->collInfo.collPosition.z + rotatedOffset.z)));
        positionDistance = fsqrt(
            (member->posinfo.position.x - (targetShip->posinfo.position.x + rotatedOffset.x)) *
            (member->posinfo.position.x - (targetShip->posinfo.position.x + rotatedOffset.x)) +
            (member->posinfo.position.y - (targetShip->posinfo.position.y + rotatedOffset.y)) *
            (member->posinfo.position.y - (targetShip->posinfo.position.y + rotatedOffset.y)) +
            (member->posinfo.position.z - (targetShip->posinfo.position.z + rotatedOffset.z)) *
            (member->posinfo.position.z - (targetShip->posinfo.position.z + rotatedOffset.z)));
        unrotatedDistance = fsqrt(
            (member->posinfo.position.x - (targetShip->collInfo.collPosition.x + unrotatedOffset.x)) *
            (member->posinfo.position.x - (targetShip->collInfo.collPosition.x + unrotatedOffset.x)) +
            (member->posinfo.position.y - (targetShip->collInfo.collPosition.y + unrotatedOffset.y)) *
            (member->posinfo.position.y - (targetShip->collInfo.collPosition.y + unrotatedOffset.y)) +
            (member->posinfo.position.z - (targetShip->collInfo.collPosition.z + unrotatedOffset.z)) *
            (member->posinfo.position.z - (targetShip->collInfo.collPosition.z + unrotatedOffset.z)));
        leaderDistance = fsqrt(
            (member->posinfo.position.x - (selection->ShipPtr[0]->posinfo.position.x + leaderRotatedOffset.x)) *
            (member->posinfo.position.x - (selection->ShipPtr[0]->posinfo.position.x + leaderRotatedOffset.x)) +
            (member->posinfo.position.y - (selection->ShipPtr[0]->posinfo.position.y + leaderRotatedOffset.y)) *
            (member->posinfo.position.y - (selection->ShipPtr[0]->posinfo.position.y + leaderRotatedOffset.y)) +
            (member->posinfo.position.z - (selection->ShipPtr[0]->posinfo.position.z + leaderRotatedOffset.z)) *
            (member->posinfo.position.z - (selection->ShipPtr[0]->posinfo.position.z + leaderRotatedOffset.z)));
        memberSpec->forcefieldShipInPosition = (distance <= sentinelForcefieldPositionTolerance);
    }

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *member = selection->ShipPtr[i];
        SentinelSpecifics *memberSpec = sentinelShipSpecifics(member);
        bool32 linksReady = TRUE;
        sdword linkIndex;

        if (!memberSpec->forcefieldShipInPosition)
        {
            linksReady = FALSE;
        }
        else
        {
            for (linkIndex = 0; linkIndex < memberSpec->formationLinkCount; linkIndex++)
            {
                Ship *linkA = memberSpec->formationLinks[linkIndex * 2];
                Ship *linkB = memberSpec->formationLinks[linkIndex * 2 + 1];
                if (linkA == NULL || linkB == NULL ||
                    !sentinelShipSpecifics(linkA)->forcefieldShipInPosition ||
                    !sentinelShipSpecifics(linkB)->forcefieldShipInPosition)
                {
                    linksReady = FALSE;
                    break;
                }
            }
        }

        if (selection->numShips <= 2 ||
            memberSpec->unk340 != 0 ||
            *((ubyte *)member + SENTINEL_FORCEFIELD_UNKNOWN_FLAG_OFFSET) != 0 ||
            memberSpec->unk344 != 0)
        {
            linksReady = FALSE;
        }

        if (linksReady)
        {
            if (memberSpec->forcefieldChargeTimer == 0.0f)
            {
                memberSpec->forcefieldPulseStartTime = universe.totaltimeelapsed;
                memberSpec->forcefieldPulseScale = 1.0f;
            }
            memberSpec->forcefieldChargeTimer += universe.phystimeelapsed * sentinelForcefieldChargeRate;
            if (memberSpec->forcefieldChargeTimer > 1.0f)
            {
                memberSpec->forcefieldChargeTimer = 1.0f;
                if (memberSpec->forcefieldPulseScale > 0.0f)
                {
                    memberSpec->forcefieldPulseScale -= universe.phystimeelapsed * sentinelForcefieldPulseDecayRate;
                    if (memberSpec->forcefieldPulseScale < 0.0f)
                    {
                        memberSpec->forcefieldPulseScale = 0.0f;
                    }
                }
            }
        }
        else
        {
            memberSpec->forcefieldChargeTimer = 0.0f;
        }

        memberSpec->unk0 = linksReady;

        for (linkIndex = 0; linkIndex < memberSpec->formationLinkCount; linkIndex++)
        {
            Ship *linkA = memberSpec->formationLinks[linkIndex * 2];
            Ship *linkB = memberSpec->formationLinks[linkIndex * 2 + 1];
            if (linkA != NULL && sentinelShipSpecifics(linkA)->unk0 == 0)
            {
                sentinelShipSpecifics(linkA)->unk0 = linksReady;
            }
            if (linkB != NULL && sentinelShipSpecifics(linkB)->unk0 == 0)
            {
                sentinelShipSpecifics(linkB)->unk0 = linksReady;
            }
        }
    }

    if (formationType == FORCEFIELD_SPHERE_FORMATION)
    {
        Ship *protectTarget = (command->protect != NULL) ? command->protect->ShipPtr[0] : NULL;

        for (i = 0; i < selection->numShips; i++)
        {
            Ship *member = selection->ShipPtr[i];
            SentinelSpecifics *memberSpec = sentinelShipSpecifics(member);

            if (memberSpec->unk0)
            {
                if ((member->clampInfo == NULL || member->clampInfo->host == NULL) && protectTarget != NULL)
                {
                    clampObjectSetup((SpaceObj *)member, (SpaceObj *)protectTarget);
                }
            }
            else
            {
                clDetachShipFromCommand(member);
            }
        }
    }
}

static void SentinelHousekeep(Ship *ship)
{
    SentinelSpecifics *spec = sentinelShipSpecifics(ship);
    real32 maxArmor = sentinelForcefieldArmorByLevel[spec->formationLevel];
    real32 armorStep = sentinelForcefieldArmorStepByLevel[spec->formationLevel];

    if (singlePlayerGame)
    {
        maxArmor *= 0.5f;
        armorStep *= 0.5f;
    }

    if (maxArmor > spec->forcefieldArmor)
    {
        spec->forcefieldArmor += armorStep;
    }

    if (maxArmor <= spec->forcefieldArmor)
    {
        spec->unk340 = 0;
    }

    if ((universe.univUpdateCounter & 0xC) != 0)
    {
        SentinelForcefieldUpdate(ship);
    }
}

CustShipHeader SentinelHeader =
{
    sizeof(SentinelSpecifics),
    SentinelStaticInit,
    NULL,
    SentinelInit,
    NULL,
    SentinelAttack,
    SentinelRotateAndShoot,
    pingEvalAlwaysTrue,
    pingEvalAlwaysTrue,
    NULL,
    SentinelHousekeep,
    SentinelRemoveShipReferences,
    SentinelRemoveShipReferences,
    NULL,
    NULL,
    NULL,
    SentinelPreFix,
    NULL,
    NULL,
    SentinelFix,
    NULL,
    SentinelSpecialOrder,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SentinelFormationFilter
};
