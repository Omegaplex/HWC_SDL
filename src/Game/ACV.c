/*=============================================================================
    Name    : ACV.c
    Purpose : Cataclysm-specific EMP behavior for ACVs
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "ACV.h"
#include "Attack.h"
#include "AIShip.h"
#include "Battle.h"
#include "CommandLayer.h"
#include "CommandWrap.h"
#include "DefaultShip.h"
#include "FastMath.h"
#include "formation.h"
#include "Globals.h"
#include "Gun.h"
#include "etg.h"
#include "RaceDefs.h"
#include "SalCapCorvette.h"
#include "SaveGame.h"
#include "Ship.h"
#include "objtypes.h"
#include "SoundEvent.h"
#include "Tactics.h"
#include "Timer.h"
#include "UnivUpdate.h"
#include "universe.h"
#include "vector.h"

#define ACV_EMP_NAGGAROK_RANGE      4000.0f
#define ACV_EMP_RECHARGE_DELAY         2.0f
#define ACV_EMP_RECHARGE_DURATION     75.0f

static ACVStatics acvSectStatics;
static ACVStatics acvBeastStatics;

bool32 fogObjectVisibleToPlayer(Player *player, SpaceObj *object, bool32 ignoreNebula);
void telldir(void);

static void acvStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo);
static void acvInit(Ship *ship);
static void acvAttack(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags);
static void acvFire(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist);
static void acvClose(Ship *ship);
static bool32 acvSpecialActivate(Ship *ship);
static bool32 acvSpecialTarget(Ship *ship, void *custom);
static void acvHousekeep(Ship *ship);
static void acvRemoveShipReferences(Ship *ship, Ship *shipToRemove);
static void acvPreFix(Ship *ship);
static void acvSave(Ship *ship);
static void acvLoad(Ship *ship);
static void acvFix(Ship *ship);
static bool32 acvSpecialOrder(Ship *ship, void *custom);
static bool32 acvMoveOrderCleanup(Ship *ship, void *arg1, void *arg2);
static bool32 acvPreDockCleanup(Ship *ship, sdword dockType, Ship *dockwith);
static bool32 acvCanHalt(Ship *ship);

static ACVStatics *acvStaticInfoByRace(ShipRace shiprace)
{
    return (shiprace == RACE_Beast) ? &acvBeastStatics : &acvSectStatics;
}

static ACVStatics *acvStaticInfo(Ship *ship)
{
    dbgAssert(ship->staticinfo->custstatinfo != NULL);
    return (ACVStatics *)ship->staticinfo->custstatinfo;
}

static SpaceObj **acvUnknown018Pointer(ACVSpecifics *specifics)
{
    return &specifics->unk018;
}

static sdword *acvUnknown040Pointer(ACVSpecifics *specifics)
{
    return &specifics->unk040;
}

static void acvActivateSelection(Ship *ship, bool32 useOnlyShip)
{
    GrowSelection growSelect;
    Ship *centerShip;
    sdword i;

    growSelectInit(&growSelect);
    centerShip = acvSelectEMPCenterShip(ship, useOnlyShip, &growSelect);
    dbgAssert(centerShip != NULL);
    if (centerShip != NULL)
    {
        vector centerPosition = centerShip->posinfo.position;
        real32 combineDistance = acvStaticInfo(centerShip)->EMP_Max_Combine_Distance;

        for (i = 0; i < growSelect.selection->numShips;)
        {
            Ship *candidate = growSelect.selection->ShipPtr[i];

            if (GetDistance(&centerPosition, &candidate->posinfo.position) > combineDistance)
            {
                growSelectRemoveShip(&growSelect, candidate);
            }
            else
            {
                i++;
            }
        }

        if (growSelect.selection->numShips > 1 && ShipInSelection(growSelect.selection, ship))
        {
            acvActivateEMP(&growSelect, centerShip);
        }
        else
        {
            GrowSelection singleShip;

            growSelectInit(&singleShip);
            growSelectAddShip(&singleShip, ship);
            acvActivateEMP(&singleShip, ship);
            growSelectClose(&singleShip);
        }
    }
    growSelectClose(&growSelect);
}

CustShipHeader ACVHeader =
{
    sizeof(ACVSpecifics),
    acvStaticInit,
    NULL,
    acvInit,
    acvClose,
    acvFire,
    acvAttack,
    acvSpecialActivate,
    acvSpecialTarget,
    NULL,
    acvHousekeep,
    acvRemoveShipReferences,
    acvRemoveShipReferences,
    NULL,
    NULL,
    acvClose,
    acvPreFix,
    acvSave,
    acvLoad,
    acvFix,
    (bool32 (*)(Ship *, void *, void *))acvPreDockCleanup,
    acvSpecialOrder,
    acvPreDockCleanup,
    NULL,
    NULL,
    NULL,
    NULL,
    acvCanHalt,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE360
static void acvStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    ACVStatics *statics = acvStaticInfoByRace(statinfo->shiprace);

    statics->EMP_Max_Combine_Distance = statinfo->EMP_Max_Combine_Distance;
    statics->EMP_Range = statinfo->EMP_Range;
    statics->unk0C = 2500.0f;
    statinfo->custstatinfo = statics;
    statinfo->canSpecialBandBoxFriendlies = 0;
    statinfo->unk4 = 1;
    statinfo->unk5 = 0;
    scriptSetStruct(directory, filename, AttackSideStepParametersScriptTable, &statics->attackSideStepParameters);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADC60
bool32 acvIsDeploying(Ship *ship)
{
    return shipIsACV(ship) && acvShipSpecifics(ship)->empState == ACVEMPState_Deploying;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE3D0
static void acvInit(Ship *ship)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    if (!shipIsACV(ship))
    {
        return;
    }

    memset(specifics, 0, sizeof(*specifics));
    growSelectClose(&specifics->empShockedTargets);
    growSelectInit(&specifics->empShockedTargets);
    specifics->empStateStartTime = universe.totaltimeelapsed;
    specifics->empState = ACVEMPState_Active;
    specifics->groupAttackTarget = NULL;
    specifics->groupAttackPending = FALSE;
    specifics->firstSourceLinkEnabled = 0;
    specifics->secondSourceLinkEnabled = 0;
    specifics->firstSourceLinkState = 0;
    specifics->secondSourceLinkState = 0;
    attackSideStepInit(&specifics->attackSideStep);
}

// ============================================================================
// External callers reuse the retail ACV init path when they need to reset the
// runtime EMP state after creating or rearming an ACV instance.
void acvInitSpecial(Ship *ship)
{
    acvInit(ship);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE450
static bool32 acvSpecialActivate(Ship *ship)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    if (specifics->empState != ACVEMPState_Ready)
    {
        specifics->empState = ACVEMPState_Ready;
        specifics->groupAttackTarget = NULL;
        return TRUE;
    }

    if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EMP) || (ship->flags & 0x200000) != 0)
    {
        return TRUE;
    }

    acvActivateSelection(ship, TRUE);
    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE5A0
static bool32 acvSpecialTarget(Ship *ship, void *custom)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);
    SpecialCommand *targets = custom;
    sdword i;

    if (targets == NULL || targets->numTargets <= 0)
    {
        return TRUE;
    }

    if (specifics->empState != ACVEMPState_Ready &&
        specifics->empState != ACVEMPState_Deploying)
    {
        return TRUE;
    }

    for (i = 0; i < targets->numTargets; i++)
    {
        SpaceObjRotImpTarg *targetObject = targets->TargetPtr[i];

        if (((SpaceObj *)targetObject)->objtype != OBJ_ShipType)
        {
            continue;
        }

        if (acvCanTargetShipForEMP((Ship *)targetObject, ship))
        {
            specifics->groupAttackTarget = (Ship *)targetObject;
            specifics->empState = ACVEMPState_Deploying;
            specifics->groupAttackPending = TRUE;
            break;
        }
    }

    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADBA0
bool32 acvTargetIsOwnedOrSalvagedByPlayer(sdword playerIndex, Ship *target)
{
    Ship *attachedShip;
    bool32 ownedOrSalvaged;

    ownedOrSalvaged = FALSE;
    attachedShip = SalCapGetFirstAttachedShip(target);
    if (attachedShip != NULL && (target->specialFlags4 & SPECIAL4_Unknown25) == SPECIAL4_Unknown25)
    {
        ownedOrSalvaged = attachedShip->playerowner->playerIndex == playerIndex;
    }

    if (target->specialTargetShip == NULL)
        return ownedOrSalvaged;

    if (target->specialTargetShip->playerowner->playerIndex == playerIndex)
        return TRUE;

    return ownedOrSalvaged;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADD30
bool32 acvCanTargetShipForEMP(Ship *ship, Ship *target)
{
    bool32 canTarget;

    canTarget = ((ship->staticinfo->TimeShockedByEMP > 0.0f) ||
                 ((ship->specialFlags4 & SPECIAL4_NaggarokEMP) == SPECIAL4_NaggarokEMP)) &&
                (target == NULL || ship->playerowner != target->playerowner);

    if (ship->specialTargetShip != NULL)
        canTarget = FALSE;

    if (target != NULL &&
        acvTargetIsOwnedOrSalvagedByPlayer(target->playerowner->playerIndex, ship))
    {
        canTarget = FALSE;
    }

    return ship->crystalProcessingState.active != 1 && canTarget;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADE10
void acvActivateEMP(GrowSelection *selection, Ship *ship)
{
    GrowSelection growSelect;
    SelectCommand *commandSelection;
    GrowSelection *shipSelection;
    Node *node;
    sdword i;
    bool32 queuedBattleChatter;

    growSelectInit(&growSelect);
    commandSelection = selection->selection;

    for (i = 0; i < commandSelection->numShips; i++)
    {
        Ship *activatingShip;

        activatingShip = commandSelection->ShipPtr[i];
        for (node = universe.ShipList.head; node != NULL; node = node->next)
        {
            Ship *target;
            real32 distance;

            target = node->structptr;
            if (target == activatingShip)
                continue;

            if (!acvCanTargetShipForEMP(target, activatingShip))
                continue;

            distance = GetDistance(&activatingShip->posinfo.position, &target->posinfo.position);
            if (distance < activatingShip->staticinfo->EMP_Range ||
                (shipIsNaggarok(target) && distance < ACV_EMP_NAGGAROK_RANGE))
            {
                growSelectAddShipNoDuplication(&growSelect, target);
            }
        }
    }

    if (growSelect.selection->numShips <= 0)
    {
        speechEventQueue(ship, 67110928, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);
        growSelectClose(&growSelect);
        return;
    }

    soundEventPlay(ship, 2102, NULL);
    speechEventQueue(ship, 67111101, -1, -1, -1, -1, -1, 0.0f, -1);

    for (i = 0; i < commandSelection->numShips; i++)
    {
        Ship *activatingShip;
        ACVSpecifics *specifics;
        etglod *effectLOD;
        etgeffectstatic *effectStat;

        activatingShip = commandSelection->ShipPtr[i];
        specifics = acvShipSpecifics(activatingShip);
        growSelectClose(&specifics->empShockedTargets);
        growSelectInit(&specifics->empShockedTargets);
        specifics->empStateStartTime = universe.totaltimeelapsed;
        specifics->empState = ACVEMPState_Active;
        specifics->groupAttackTarget = NULL;
        effectLOD = etgSpecialPurposeEffectTable[EGT_ELECTRO_MAGNETIC_PULSE];
        effectStat = (effectLOD != NULL && effectLOD->nLevels > 0) ? effectLOD->level[0] : NULL;
        if (effectStat != NULL && !etgFrequencyExceeded(effectStat))
        {
            vector effectPosition = activatingShip->posinfo.position;
            vector effectVelocity;
            matrix effectCoordsys = activatingShip->rotinfo.coordsys;

            memset(&effectVelocity, 0, sizeof(effectVelocity));
            etgEffectCreate(effectStat, activatingShip, &effectPosition, &effectVelocity, &effectCoordsys, 1.0f, EAF_Full, 0);
        }
    }

    shipSelection = acvGetEMPSelection(ship);
    for (i = 0; i < commandSelection->numShips; i++)
    {
        if (commandSelection->ShipPtr[i] != ship)
        {
            growSelectAddShip(shipSelection, commandSelection->ShipPtr[i]);
        }
    }

    queuedBattleChatter = FALSE;
    for (i = 0; i < growSelect.selection->numShips; i++)
    {
        Ship *target;

        target = growSelect.selection->ShipPtr[i];
        shipApplyEMPShock(target, commandSelection->numShips);
        growSelectAddShip(shipSelection, target);
        if (!queuedBattleChatter && target->playerowner->playerIndex == universe.curPlayerIndex)
        {
            battleChatterFleetAttempt(-1, 73, -1, NULL);
            queuedBattleChatter = TRUE;
        }
    }

    growSelectClose(&growSelect);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE1B0
bool32 shipIsACV(Ship *ship)
{
    return ship->shiptype == sACV || ship->shiptype == bACV;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE1D0
Ship *acvSelectEMPCenterShip(Ship *ship, bool32 useOnlyShip, GrowSelection *growSelect)
{
    vector averagePosition;
    Ship *centerShip;
    real32 closestDistance;
    sdword i;

    centerShip = NULL;
    closestDistance = 1.0e21f;
    averagePosition.x = 0.0f;
    averagePosition.y = 0.0f;
    averagePosition.z = 0.0f;

    if (useOnlyShip)
    {
        growSelectAddShip(growSelect, ship);
    }
    else
    {
        CommandToDo *command;

        command = ship->command;
        if (command != NULL &&
            (command->ordertype.attributes & 1) != 0 &&
            command->selection != NULL)
        {
            for (i = 0; i < command->selection->numShips; i++)
            {
                Ship *candidate;
                ACVSpecifics *specifics;

                candidate = command->selection->ShipPtr[i];
                if (!shipIsACV(candidate))
                    continue;

                specifics = acvShipSpecifics(candidate);
                if ((specifics->empState == ACVEMPState_Ready ||
                     specifics->empState == ACVEMPState_Deploying) &&
                    ShipHasUpgrade(candidate, RESEARCH_UPGRADE_MASK_EMP) &&
                    (candidate->flags & 0x200000) == 0)
                {
                    growSelectAddShip(growSelect, candidate);
                }
            }
        }
        else
        {
            growSelectAddShip(growSelect, ship);
        }
    }

    if (growSelect->selection->numShips <= 0)
        return NULL;

    for (i = 0; i < growSelect->selection->numShips; i++)
    {
        Ship *candidate;

        candidate = growSelect->selection->ShipPtr[i];
        averagePosition.x += candidate->posinfo.position.x;
        averagePosition.y += candidate->posinfo.position.y;
        averagePosition.z += candidate->posinfo.position.z;
    }

    averagePosition.x *= 1.0f / growSelect->selection->numShips;
    averagePosition.y *= 1.0f / growSelect->selection->numShips;
    averagePosition.z *= 1.0f / growSelect->selection->numShips;

    for (i = 0; i < growSelect->selection->numShips; i++)
    {
        Ship *candidate;
        real32 distance;

        candidate = growSelect->selection->ShipPtr[i];
        distance = GetDistance(&averagePosition, &candidate->posinfo.position);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            centerShip = candidate;
        }
    }

    return centerShip;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE720
void acvIssueGroupEMPAttack(Ship *ship)
{
    AttackCommand attackcom;
    MaxSelection selectcom;
    ACVSpecifics *shipSpecifics;
    CommandToDo *command;

    shipSpecifics = acvShipSpecifics(ship);
    attackcom.numTargets = 1;
    attackcom.timeLastStatus = 0.0f;
    attackcom.TargetPtr[0] = (TargetPtr)shipSpecifics->groupAttackTarget;
    command = ship->command;

    if (command != NULL &&
        (command->ordertype.attributes & 1) != 0 &&
        command->selection != NULL &&
        command->selection->numShips > 1)
    {
        sdword i;

        selectcom.numShips = 0;
        selectcom.timeLastStatus = 0.0f;
        for (i = 0; i < command->selection->numShips; i++)
        {
            Ship *candidate;
            ACVSpecifics *candidateSpecifics;

            candidate = command->selection->ShipPtr[i];
            if (!shipIsACV(candidate))
                continue;

            candidateSpecifics = acvShipSpecifics(candidate);
            if (candidateSpecifics->groupAttackPending &&
                candidateSpecifics->empState == ACVEMPState_Deploying &&
                candidateSpecifics->groupAttackTarget == shipSpecifics->groupAttackTarget)
            {
                selectcom.ShipPtr[selectcom.numShips++] = candidate;
                candidateSpecifics->groupAttackPending = FALSE;
            }
        }
        clAttack(&universe.mainCommandLayer, (SelectCommand *)&selectcom, &attackcom);
    }
    else
    {
        selectcom.numShips = 1;
        selectcom.timeLastStatus = 0.0f;
        selectcom.ShipPtr[0] = ship;
        clAttack(&universe.mainCommandLayer, (SelectCommand *)&selectcom, &attackcom);
        shipSpecifics->empStateStartTime = 0.0f;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE840
real32 acvGetEMPChargeProgress(Ship *ship)
{
    ACVSpecifics *specifics;
    real32 elapsed;
    real32 progress;

    specifics = acvShipSpecifics(ship);
    elapsed = universe.totaltimeelapsed - specifics->empStateStartTime;
    if (specifics->empState == ACVEMPState_Active)
        return 0.0f;

    if (specifics->empState == ACVEMPState_Recharging)
    {
        progress = (elapsed - ACV_EMP_RECHARGE_DELAY) / ACV_EMP_RECHARGE_DURATION;
        if (progress <= 1.0f)
            return progress;
        return 1.0f;
    }

    if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EMP))
        return 0.0f;

    return 1.0f;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADD10
bool32 acvIsEMPActive(Ship *ship)
{
    return acvShipSpecifics(ship)->empState == ACVEMPState_Active;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AED60
GrowSelection *acvGetEMPSelection(Ship *ship)
{
    return &acvShipSpecifics(ship)->empShockedTargets;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AE8C0
static void acvHousekeep(Ship *ship)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);
    real32 elapsed = universe.totaltimeelapsed - specifics->empStateStartTime;

    if (specifics->empState == ACVEMPState_Active &&
        (elapsed > ACV_EMP_RECHARGE_DELAY || elapsed < 0.0f))
    {
        growSelectClose(&specifics->empShockedTargets);
        growSelectInit(&specifics->empShockedTargets);
        specifics->empState = ACVEMPState_Recharging;
    }

    if (specifics->empState == ACVEMPState_Recharging &&
        (ACV_EMP_RECHARGE_DELAY + ACV_EMP_RECHARGE_DURATION < elapsed || elapsed < 0.0f))
    {
        specifics->empState = ACVEMPState_Ready;
        specifics->empStateStartTime = 0.0f;
    }

    if (specifics->empState == ACVEMPState_Deploying)
    {
        Ship *target = specifics->groupAttackTarget;

        if (specifics->groupAttackPending)
        {
            acvIssueGroupEMPAttack(ship);
            specifics->groupAttackPending = FALSE;
        }

        if (fogObjectVisibleToPlayer(ship->playerowner, (SpaceObj *)target, TRUE))
        {
            vector delta;
            real32 distanceSquared;
            bool32 inRange;

            vecSub(delta, ship->posinfo.position, target->posinfo.position);
            distanceSquared = vecMagnitudeSquared(delta);
            inRange = distanceSquared < ship->staticinfo->EMP_Range * ship->staticinfo->EMP_Range;

            if ((shipIsNaggarok(target) && distanceSquared < ACV_EMP_NAGGAROK_RANGE * ACV_EMP_NAGGAROK_RANGE) || inRange)
            {
                specifics->empState = ACVEMPState_Ready;

                if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EMP) && (ship->flags & 0x200000) == 0)
                {
                    acvActivateSelection(ship, FALSE);
                }

                specifics->groupAttackTarget = NULL;
            }
        }
        else
        {
            MaxSelection selectcom;

            selectcom.numShips = 1;
            selectcom.timeLastStatus = 0.0f;
            selectcom.ShipPtr[0] = ship;
            clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selectcom, NULL);
        }
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AEB90
static void acvFire(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    GunInfo *customDockInfo = (GunInfo *)ship->dockvars.customdockinfo;
    sdword i;

    (void)maxdist;

    if (customDockInfo == NULL)
    {
        return;
    }

    for (i = 0; i < customDockInfo->numGuns; i++)
    {
        Gun *gun = &customDockInfo->guns[i];

        if (!gunCanShootStaggered(ship, gun))
        {
            continue;
        }

        switch (gun->weaponMode)
        {
            case WEAPON_Projectile:
            case WEAPON_Energy:
            case WEAPON_Ion:
            case WEAPON_BigGun:
                gunShootGunAtTarget(ship, gun, target);
                break;

            case WEAPON_Missile:
            case WEAPON_Mine:
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

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADC40
static void acvClose(Ship *ship)
{
    growSelectClose(&acvShipSpecifics(ship)->empShockedTargets);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5ADC90
static void acvRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    if (specifics->empShockedTargets.selection != NULL)
    {
        growSelectRemoveShip(&specifics->empShockedTargets, shipToRemove);
    }
    if (specifics->groupAttackTarget == shipToRemove)
    {
        specifics->groupAttackTarget = NULL;
        if (specifics->empState == ACVEMPState_Deploying)
        {
            specifics->empState = ACVEMPState_Ready;
        }
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AEE20
static void acvPreFix(Ship *ship)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    specifics->groupAttackTarget = (Ship *)SpaceObjRegistryGetID((SpaceObj *)specifics->groupAttackTarget);
    *acvUnknown018Pointer(specifics) = (SpaceObj *)SpaceObjRegistryGetID(*acvUnknown018Pointer(specifics));
    if (*acvUnknown040Pointer(specifics) != 0)
    {
        telldir();
        *acvUnknown040Pointer(specifics) = 0;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AED70
static void acvSave(Ship *ship)
{
    SaveGrowSelection(&acvShipSpecifics(ship)->empShockedTargets);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AED90
static void acvLoad(Ship *ship)
{
    LoadGrowSelection(&acvShipSpecifics(ship)->empShockedTargets);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AEDB0
static void acvFix(Ship *ship)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    FixGrowSelection(&specifics->empShockedTargets);
    specifics->groupAttackTarget = SpaceObjRegistryGetShip((sdword)specifics->groupAttackTarget);
    *acvUnknown018Pointer(specifics) = (SpaceObj *)SpaceObjRegistryGetShip((sdword)*acvUnknown018Pointer(specifics));
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AEC20
static bool32 acvSpecialOrder(Ship *ship, void *custom)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);
    Ship *target = custom;

    if (specifics->empState == ACVEMPState_Deploying && specifics->groupAttackTarget == target)
    {
        CommandToDo *command = ship->command;

        if (command == NULL)
        {
            real32 limitVelocity = (real32)(shipGetMaxSpeed(ship) * tacticsGetShipsMaxVelocity(ship));

            aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 0xAu, limitVelocity, 0);
            ship->aistateattack = 0;
            return TRUE;
        }

        if ((command->ordertype.attributes & 1) == 0 ||
            command->selection->ShipPtr[0] == ship ||
            !shipIsACV(command->selection->ShipPtr[0]) ||
            acvShipSpecifics(command->selection->ShipPtr[0])->groupAttackTarget != specifics->groupAttackTarget)
        {
            if ((command->ordertype.attributes & 1) != 0 &&
                command->selection->ShipPtr[0] == ship)
            {
                aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 0xAu, command->formation.travelvel, 0);
                ship->aistateattack = 0;
                processFormationToDo(command, FALSE, FALSE);
                ship->aistateattack = 0;
                return TRUE;
            }

            {
                real32 limitVelocity = (real32)(shipGetMaxSpeed(ship) * tacticsGetShipsMaxVelocity(ship));

                aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 0xAu, limitVelocity, 0);
            }
            ship->aistateattack = 0;
            return TRUE;
        }
    }

    if (specifics->empState != ACVEMPState_Deploying)
    {
        attackSideStep(ship, (SpaceObjRotImpTarg *)target, &specifics->attackSideStep, &acvStaticInfo(ship)->attackSideStepParameters);
    }

    if (specifics->empState == ACVEMPState_Deploying)
    {
        specifics->empState = ACVEMPState_Ready;
        specifics->groupAttackTarget = NULL;
    }

    return TRUE;
}

// ============================================================================
// Retail slot +0x50 reuses the same ACV EMP cleanup as the dock-start hook.
static bool32 acvMoveOrderCleanup(Ship *ship, void *arg1, void *arg2)
{
    (void)arg1;
    (void)arg2;
    return acvPreDockCleanup(ship, 0, NULL);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AEED0
static bool32 acvPreDockCleanup(Ship *ship, sdword dockType, Ship *dockwith)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    (void)dockType;
    (void)dockwith;

    if (specifics->empState == ACVEMPState_Deploying)
    {
        specifics->empState = ACVEMPState_Ready;
        specifics->groupAttackTarget = NULL;
    }

    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AEE80
static bool32 acvCanHalt(Ship *ship)
{
    ACVSpecifics *specifics = acvShipSpecifics(ship);

    if (specifics->empState == ACVEMPState_Deploying)
    {
        specifics->empState = ACVEMPState_Ready;
        specifics->groupAttackTarget = NULL;
    }

    return (ship->command == NULL || ship->command->ordertype.order != COMMAND_SPECIAL) &&
           specifics->unk018 == NULL;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5BE540
static void acvAttack(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    if ((rotateFlags & ship->staticinfo->rotateToRetaliate) != 0)
    {
        attackPassive(ship, (Ship *)target);
    }
    else
    {
        attackSimple(ship, target);
    }
}
