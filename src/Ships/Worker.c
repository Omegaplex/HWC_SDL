/*=============================================================================
    Name    : Worker.c
    Purpose : Worker ship static/runtime initialization

    Created by Copilot based on Cataclysm binary reconstruction
=============================================================================*/

#include "types.h"
#include "AIShip.h"
#include "AITrack.h"
#include "Battle.h"
#include "CommandLayer.h"
#include "debug.h"
#include "dock.h"
#include "etg.h"
#include "MissionScriptSupport.h"
#include "matrix.h"
#include "meshanim.h"
#include "objtypes.h"
#include "rescollect.h"
#include "SalCapCorvette.h"
#include "Ship.h"
#include "spaceobj.h"
#include "statscript.h"
#include "tweak.h"
#include "UnivUpdate.h"
#include "universe.h"
#include "Worker.h"
#include <math.h>

#define WORKER_JACK_RETRACT_TIME_DEFAULT 6.0f
#define WORKER_EFFECT_START_TIME_INACTIVE 1.0e21f
#define WORKER_SPECIAL_MODE_REPAIR_BEAM 0
#define WORKER_SPECIAL_MODE_SALVAGE 1
#define WORKER_SPECIAL_MODE_IDLE 2
#define WORKER_REPAIR_BEAM_STATE_IDLE 0
#define WORKER_REPAIR_BEAM_STATE_FLY_TO_TARGET 1
/* State 2 does not exist in retail: salCapFlyToTarget (0x5C8740) combines flying and
   heading alignment, so the state machine goes directly from FLY_TO_TARGET to
   START_EFFECT once salCapFlyToTarget reports in-range-and-facing. */
#define WORKER_REPAIR_BEAM_STATE_START_EFFECT 3
#define WORKER_REPAIR_BEAM_STATE_REPAIRING 4
#define WORKER_REPAIR_BEAM_STATE_FINISHED 5
#define WORKER_REPAIR_BEAM_PRIORITY_COUNTER_OFFSET 1536
#define WORKER_REPAIR_BEAM_ACTIVE_COUNTER_OFFSET 1540
#define WORKER_REPAIR_BEAM_MIN_RANGE 800.0f
#define WORKER_REPAIR_BEAM_MAX_RANGE 1500.0f
#define WORKER_REPAIR_BEAM_RANGE_SCALE 2.0f
#define WORKER_REPAIR_BEAM_ALIGN_DOT 0.99f
/* Retail repairBeamAIUpdate (0x5C89A0) scales the per-tick repair amount by this
   factor when the game is in tutorial mode; shared/confirmed via the identical
   duplicate reconstruction in RKO.c (rkoTutorialEatRateScale), which decompiles the
   exact same function address (repairBeamAIUpdate has exactly one caller in the
   binary: Worker's WorkerTargetSpecial). */
#define WORKER_TUTORIAL_REPAIR_RATE_SCALE 10.0f

static_assert(offsetof(WorkerHarvestStaticInfo, jackRetractTime) == 0x00);
static_assert(offsetof(WorkerHarvestStaticInfo, grinderDockPoint) == 0x04);
static_assert(offsetof(WorkerHarvestStaticInfo, jackhammerDockPoint) == 0x08);
static_assert(offsetof(WorkerHarvestStaticInfo, hostQueueScriptData) == 0x0C);
static_assert(offsetof(WorkerHarvestStaticInfo, hostQueueDockPoint) == 0x10);
static_assert(offsetof(WorkerHarvestStaticInfo, repairDistance) == 0x14);
static_assert(offsetof(WorkerHarvestStaticInfo, repairCone) == 0x18);
static_assert(offsetof(WorkerHarvestStaticInfo, repairBeamWidth) == 0x1C);
static_assert(offsetof(WorkerHarvestStaticInfo, repairConeCosHalfAngle) == 0x20);
static_assert(offsetof(WorkerHarvestStaticInfo, jackhammerPosition) == 0x24);
static_assert(sizeof(WorkerHarvestStaticInfo) == 0x30);
static_assert(offsetof(WorkerShipSpecifics, effectStartTime) == 0x04);
static_assert(sizeof(WorkerShipSpecifics) == 0x48);

static WorkerHarvestStaticInfo WorkerStaticRace1;
static WorkerHarvestStaticInfo WorkerStaticRace2;

static scriptStructEntry WorkerStaticScriptTable[] =
{
    {
        "jackRetractTime",
        scriptSetReal32CB,
        (udword)&(WorkerStaticRace1.jackRetractTime),
        (udword)&WorkerStaticRace1
    },
    { NULL, NULL, 0, 0 }
};

static scriptStructEntry WorkerRepairStaticScriptTable[] =
{
    {
        "repairDistance",
        scriptSetReal32CB,
        (udword)&(WorkerStaticRace1.repairDistance),
        (udword)&WorkerStaticRace1
    },
    {
        "repairCone",
        scriptSetReal32CB,
        (udword)&(WorkerStaticRace1.repairCone),
        (udword)&WorkerStaticRace1
    },
    {
        "repairBeamWidth",
        scriptSetReal32CB,
        (udword)&(WorkerStaticRace1.repairBeamWidth),
        (udword)&WorkerStaticRace1
    },
    { NULL, NULL, 0, 0 }
};

static Ship *workerRepairBeamSelectClosestPriorityShip(Ship *ship, SelectAnyCommand *priorityTargets);
static Ship *workerRepairBeamSelectTarget(Ship *ship, SpecialCommand *targets, bool32 *usedPriorityTarget);
static void workerDeleteShowDamageEffect(Ship *ship);
static void workerDeleteRepairBeamEffect(WorkerShipSpecifics *spec);
static void workerCreateRepairBeamEffect(Ship *ship);
static Ship *workerFindMostDamagedShip(Ship *target);
static bool32 workerRepairBeamIsInRange(Ship *ship);
static bool32 workerRepairBeamFlyToTarget(Ship *ship);
static void repairBeamResetState(Ship *ship);
static bool32 repairBeamAIUpdate(Ship *ship, SpecialCommand *targets);
static bool32 WorkerTargetSpecial(Ship *ship, void *custom);
static void WorkerSpecialOrderCleanup(void *context);
static void WorkerHousekeep(Ship *ship);
static void WorkerRemoveShipReferences(Ship *ship, Ship *shipToRemove);
static void WorkerClose(Ship *ship);
static void WorkerPreFix(Ship *ship);
static void WorkerFix(Ship *ship);
static bool32 WorkerCanHalt(Ship *ship);
void SalCapInitHostQueueScriptData(char *directory, char *filename, ShipStaticInfo *statinfo);
void telldir(void);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
sdword pingEvalAlwaysTrue(void);

static WorkerHarvestStaticInfo *workerStaticsForType(ShipType type)
{
    switch (type)
    {
        case sWorker:
            return &WorkerStaticRace1;

        case bWorker:
            return &WorkerStaticRace2;

        default:
            dbgFatalf(DBG_Loc, "WORKER: Static info for %s cannot be initialized by worker code.",
                      ShipTypeToStr(type));
            return NULL;
    }
}

static udword *workerRepairBeamPriorityCounter(Ship *ship)
{
    return (udword *)((ubyte *)ship + WORKER_REPAIR_BEAM_PRIORITY_COUNTER_OFFSET);
}

static udword *workerRepairBeamActiveCounter(Ship *ship)
{
    return (udword *)((ubyte *)ship + WORKER_REPAIR_BEAM_ACTIVE_COUNTER_OFFSET);
}

static Ship *workerRepairBeamTarget(const WorkerShipSpecifics *spec)
{
    return (Ship *)spec->unk_38;
}

static Effect *workerRepairBeamEffect(const WorkerShipSpecifics *spec)
{
    return (Effect *)spec->unk_3C;
}

static real32 workerRepairBeamStandOffDistance(const WorkerShipSpecifics *spec)
{
    return TreatAsReal32(spec->unk_44);
}

static void workerSetRepairBeamTarget(WorkerShipSpecifics *spec, Ship *target)
{
    spec->unk_38 = (udword)target;
}

static void workerSetRepairBeamEffect(WorkerShipSpecifics *spec, Effect *effect)
{
    spec->unk_3C = (udword)effect;
}

static void workerSetRepairBeamStandOffDistance(WorkerShipSpecifics *spec, real32 distance)
{
    spec->unk_44 = TreatAsUdword(distance);
}

static real32 workerRepairRateForShip(const Ship *ship)
{
    dbgAssert(ship->staticinfo->shipclass >= 0 && ship->staticinfo->shipclass < NUM_CLASSES);
    return twRepairRate[ship->staticinfo->shipclass];
}

static void workerDeleteShowDamageEffect(Ship *ship)
{
    Effect *effect = ship->showingDamage[0][0];

    if (effect == NULL)
    {
        return;
    }
    if ((((ubyte *)effect->staticinfo)[104] & 1) != 0)
    {
        *(udword *)(((udword *)effect)[72] + 8) = ((udword *)effect)[61];
    }
    else
    {
        etgEffectDelete(effect);
        univRemoveObjFromRenderList((SpaceObj *)effect);
        listDeleteNode(&effect->objlink);
    }
    ship->showingDamage[0][0] = NULL;
}

static void workerDeleteRepairBeamEffect(WorkerShipSpecifics *spec)
{
    Effect *effect = workerRepairBeamEffect(spec);

    if (effect != NULL)
    {
        etgEffectDelete(effect);
        univRemoveObjFromRenderList((SpaceObj *)effect);
        listDeleteNode(&effect->objlink);
    }
    workerSetRepairBeamEffect(spec, NULL);
    spec->unk_40 = 0;
    workerSetRepairBeamStandOffDistance(spec, 0.0f);
}

static Ship *workerFindMostDamagedShip(Ship *target)
{
    Ship *mostDamagedShip = target;
    real32 lowestHealthFraction = target->health / target->staticinfo->maxhealth;

    if (target->shipsInsideMe != NULL)
    {
        Node *node;

        for (node = target->shipsInsideMe->insideList.head; node != NULL; node = node->next)
        {
            InsideShip *insideShip = listGetStructOfNode(node);
            Ship *inside = insideShip->ship;
            real32 insideHealthFraction = inside->health / inside->staticinfo->maxhealth;

            if (insideHealthFraction < lowestHealthFraction)
            {
                lowestHealthFraction = insideHealthFraction;
                mostDamagedShip = inside;
            }
        }
    }
    return mostDamagedShip;
}

static Ship *workerRepairBeamSelectClosestPriorityShip(Ship *ship, SelectAnyCommand *priorityTargets)
{
    sdword minimumPriorityCount = SDWORD_Max;
    sdword bestIndex = -1;
    sdword fallbackIndex = -1;
    real64 bestDistanceSquared = REALlyBig;
    sdword index;

    for (index = 0; index < priorityTargets->numTargets; index++)
    {
        Ship *candidate = (Ship *)priorityTargets->TargetPtr[index];
        sdword priorityCount = (sdword)*workerRepairBeamPriorityCounter(candidate);

        if (priorityCount < minimumPriorityCount)
        {
            minimumPriorityCount = priorityCount;
        }
    }

    for (index = 0; index < priorityTargets->numTargets; index++)
    {
        Ship *candidate = (Ship *)priorityTargets->TargetPtr[index];
        vector delta;
        real64 distanceSquared;

        delta.x = candidate->posinfo.position.x - ship->posinfo.position.x;
        delta.y = candidate->posinfo.position.y - ship->posinfo.position.y;
        delta.z = candidate->posinfo.position.z - ship->posinfo.position.z;
        distanceSquared = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
        if (distanceSquared < bestDistanceSquared)
        {
            if ((sdword)*workerRepairBeamPriorityCounter(candidate) == minimumPriorityCount)
            {
                bestDistanceSquared = distanceSquared;
                bestIndex = index;
            }
            else if (bestIndex == -1 && fallbackIndex == -1)
            {
                fallbackIndex = index;
            }
        }
    }

    if (bestIndex != -1)
    {
        return (Ship *)priorityTargets->TargetPtr[bestIndex];
    }
    if (fallbackIndex != -1)
    {
        return (Ship *)priorityTargets->TargetPtr[fallbackIndex];
    }
    return NULL;
}

static Ship *workerRepairBeamSelectTarget(Ship *ship, SpecialCommand *targets, bool32 *usedPriorityTarget)
{
    sdword index = 0;
    sdword closestAvailableIndex = -1;
    sdword closestBusyIndex = -1;
    real64 closestDistanceSquared = REALlyBig;
    MaxAnySelection priorityTargets;

    memset(&priorityTargets, 0, sizeof(priorityTargets));
    *usedPriorityTarget = FALSE;
    while (index < targets->numTargets)
    {
        Ship *candidate = (Ship *)targets->TargetPtr[index];
        if (candidate->objtype != OBJ_ShipType ||
            !univIsObjFriendly((SpaceObj *)candidate, ship->playerowner) ||
            workerRepairRateForShip(candidate) <= 0.0f)
        {
            targets->numTargets--;
            targets->TargetPtr[index] = targets->TargetPtr[targets->numTargets];
            continue;
        }
        if ((candidate->specialFlags2 & SPECIAL_2_DockFlag) == 0)
        {
            Ship *mostDamagedShip = workerFindMostDamagedShip(candidate);

            if (mostDamagedShip->health >= mostDamagedShip->staticinfo->maxhealth)
            {
                priorityTargets.TargetPtr[priorityTargets.numTargets++] = (TargetPtr)candidate;
            }
            else
            {
                vector delta;
                real64 distanceSquared;

                delta.x = candidate->posinfo.position.x - ship->posinfo.position.x;
                delta.y = candidate->posinfo.position.y - ship->posinfo.position.y;
                delta.z = candidate->posinfo.position.z - ship->posinfo.position.z;
                distanceSquared = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
                if (distanceSquared < closestDistanceSquared)
                {
                    if (*workerRepairBeamActiveCounter(candidate) != 0)
                    {
                        if (closestAvailableIndex == -1 && closestBusyIndex == -1)
                        {
                            closestBusyIndex = index;
                        }
                    }
                    else
                    {
                        closestDistanceSquared = distanceSquared;
                        closestAvailableIndex = index;
                    }
                }
            }
        }
        index++;
    }

    if (closestAvailableIndex != -1)
    {
        return (Ship *)targets->TargetPtr[closestAvailableIndex];
    }
    if (closestBusyIndex != -1)
    {
        return (Ship *)targets->TargetPtr[closestBusyIndex];
    }
    if (priorityTargets.numTargets <= 0)
    {
        return NULL;
    }

    *usedPriorityTarget = TRUE;
    return workerRepairBeamSelectClosestPriorityShip(ship, (SelectAnyCommand *)&priorityTargets);
}

static void WorkerResetHostQueueScriptData(Ship *ship)
{
    ship->specialFlags3 |= SPECIAL3_Unknown19;
    SalCapDropTarget(ship);
    workerShipSpecifics(ship)->unk_08 = WORKER_SPECIAL_MODE_IDLE;
}

static void WorkerInitRepairStatics(char *directory, char *filename, ShipStaticInfo *statinfo, WorkerHarvestStaticInfo *workerStatic)
{
    DockStaticPoint *gun0;

    workerStatic->repairDistance = 0.0f;
    workerStatic->repairCone = 0.0f;
    workerStatic->repairBeamWidth = 0.0f;
    workerStatic->repairConeCosHalfAngle = 0.0f;
    vecZeroVector(workerStatic->jackhammerPosition);

    scriptSetStruct(directory, filename, WorkerRepairStaticScriptTable, (ubyte *)workerStatic);

    if (workerStatic->repairDistance == 0.0f)
    {
        dbgFatalf(DBG_Loc, "REPAIRING: RepairStatics::repairDistance not set in .shp file for %s.",
                  ShipTypeToStr(statinfo->shiptype));
    }
    if (workerStatic->repairCone == 0.0f)
    {
        dbgFatalf(DBG_Loc, "REPAIRING: RepairStatics::repairCone not set in .shp file for %s.",
                  ShipTypeToStr(statinfo->shiptype));
    }
    if (workerStatic->repairBeamWidth == 0.0f)
    {
        dbgFatalf(DBG_Loc, "REPAIRING: RepairStatics::repairBeamWidth not set in .shp file for %s.",
                  ShipTypeToStr(statinfo->shiptype));
    }

    workerStatic->repairConeCosHalfAngle = (real32)cos(DEG_TO_RAD(workerStatic->repairCone * 0.5f));

    gun0 = dockFindDockStaticPoint("gun0", statinfo->dockStaticInfo);
    if (gun0 == NULL)
    {
        dbgFatalf(DBG_Loc, "\nWorkerStaticInit: No Gun0 (jackhammer)");
    }
    workerStatic->jackhammerPosition = gun0->position;

    statinfo->canSpecialBandBoxFriendlies = TRUE;
    statinfo->unk4 = TRUE;
    statinfo->unk5 = TRUE;
    statinfo->canSingleClickSpecialActivate = TRUE;
}

// Decompiled from cataclysm.exe @ 0x5C9580
static void WorkerStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    WorkerHarvestStaticInfo *workerStatic = workerStaticsForType(statinfo->shiptype);

    if (workerStatic == NULL)
    {
        return;
    }

    workerStatic->jackRetractTime = WORKER_JACK_RETRACT_TIME_DEFAULT;
    workerStatic->grinderDockPoint = dockFindDockStaticPoint("Grinder", statinfo->dockStaticInfo);
    workerStatic->jackhammerDockPoint = dockFindDockStaticPoint("gun0", statinfo->dockStaticInfo);
    /* The shared salvage host-queue loader reads the worker host-queue block via custstatinfo + 0x0C. */
    statinfo->custstatinfo = workerStatic;
    statinfo->workerHarvestStatic = workerStatic;
    scriptSetStruct(directory, filename, WorkerStaticScriptTable, (ubyte *)workerStatic);
    SalCapInitHostQueueScriptData(directory, filename, statinfo);
    WorkerInitRepairStatics(directory, filename, statinfo, workerStatic);
}

// Decompiled from cataclysm.exe @ 0x5C9710
static void WorkerInit(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);

    spec->unk_00 = 0;
    spec->unk_01 = 0;
    spec->effectStartTime = WORKER_EFFECT_START_TIME_INACTIVE;
    spec->unk_08 = 2;
    ship->ShipXHarvestsResourceY = WorkerHarvestsResource;
    WorkerResetHostQueueScriptData(ship);
    spec->unk_34 = 0;
    spec->unk_38 = 0;
    spec->unk_3C = 0;
    spec->unk_40 = 0;
    spec->unk_44 = 0;
}

/* Retail repairBeamAIUpdate/salCapCreateBeamEffect (0x5C8310) clamps the beam's
   rendered width to an angular cone between 10 and 26 degrees (half-angle) as seen
   from the muzzle, rather than passing the target's raw collision-sphere size. The
   two scale constants and the lazy-init pattern are reconstructed verbatim from the
   disassembly (dbl_72C028/dbl_72C020, dword_A78D40, flt_8C6B44/flt_8C6B40). */
#define WORKER_REPAIR_BEAM_NEAR_CONE_ANGLE 0.17453292519943295 /* 10 degrees in radians */
#define WORKER_REPAIR_BEAM_FAR_CONE_ANGLE  0.45378560551852566 /* 26 degrees in radians */

static bool32 workerRepairBeamWidthScaleInitialized = FALSE;
static real32 workerRepairBeamNearWidthScale = 0.0f;
static real32 workerRepairBeamFarWidthScale = 0.0f;

static void workerCreateRepairBeamEffect(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);
    Ship *target = workerRepairBeamTarget(spec);
    WorkerHarvestStaticInfo *workerStatic = ship->staticinfo->workerHarvestStatic;
    etglod *etgLOD = etgSpecialPurposeEffectTable[EGT_REPAIR_BEAM];
    etgeffectstatic *stat;
    vector beamPosition;
    vector trajectory;
    matrix coordsys;
    real32 distance;
    real32 nearWidth;
    real32 farWidth;
    real32 width;
    udword intLength;
    udword intWidth;

    dbgAssert(target != NULL);
    aishipGetTrajectory(ship, (SpaceObjRotImpTarg *)target, &trajectory);
    matCreateCoordSysFromHeading(&coordsys, &trajectory);
    matMultiplyMatByVec(&beamPosition, &ship->rotinfo.coordsys, &workerStatic->jackhammerPosition);
    vecAddTo(beamPosition, ship->posinfo.position);
    distance = (real32)shipDistanceToCompoundAABB(&beamPosition.x, target);
    workerSetRepairBeamStandOffDistance(spec, distance);
    intLength = TreatAsUdword(distance);

    if (!workerRepairBeamWidthScaleInitialized)
    {
        workerRepairBeamWidthScaleInitialized = TRUE;
        workerRepairBeamNearWidthScale = (real32)(tan(WORKER_REPAIR_BEAM_NEAR_CONE_ANGLE) * 2.0);
        workerRepairBeamFarWidthScale = (real32)(tan(WORKER_REPAIR_BEAM_FAR_CONE_ANGLE) * 2.0);
    }
    nearWidth = workerRepairBeamNearWidthScale * distance;
    farWidth = workerRepairBeamFarWidthScale * distance;
    width = target->staticinfo->staticheader.staticCollInfo.collspheresize * 2.0f;
    if (width > farWidth)
    {
        width = farWidth;
    }
    if (width < nearWidth)
    {
        width = nearWidth;
    }
    intWidth = TreatAsUdword(width);

    if (etgLOD != NULL)
    {
        if (ship->currentLOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[ship->currentLOD];
        }
    }
    else
    {
        stat = NULL;
    }
#if ETG_DISABLEABLE
    if (stat != NULL && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
    if (stat != NULL && !etgFrequencyExceeded(stat))
#endif
    {
        /* Retail salCapCreateBeamEffect (0x5C8310, shared by Worker's repairBeamAIUpdate
           and SalCapCorvette's beam-effect equivalent) passes flags=112 (0x70) to
           etgEffectCreate: SOF_AttachCoordsys | SOF_AttachVelocity | SOF_AttachNLips.
           SOF_AttachCoordsys is required here because matCreateCoordSysFromHeading()
           bakes the un-normalized ship->target trajectory length into coordsys column 3
           at creation time; without SOF_AttachCoordsys, etgEffectUpdate() never
           refreshes effect->rotinfo.coordsys from the ship's real (unit-scale)
           orientation, so that huge one-time distance magnitude persists forever and
           the beam geometry renders stretched by it every frame. */
        workerSetRepairBeamEffect(
            spec,
            etgEffectCreate(stat, ship, &beamPosition, &ship->posinfo.velocity, &coordsys,
                            1.0f, EAF_Coordsys | EAF_Velocity | EAF_NLips, 2, intLength, intWidth));
    }
    else
    {
        workerSetRepairBeamEffect(spec, NULL);
    }
    spec->unk_40 = 1;
}

static bool32 workerRepairBeamIsInRange(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);
    Ship *target = workerRepairBeamTarget(spec);
    vector delta;
    vector localPoint;
    real32 compoundAABB[6];
    real32 range;

    dbgAssert(target != NULL);
    range = target->staticinfo->staticheader.staticCollInfo.collspheresize * WORKER_REPAIR_BEAM_RANGE_SCALE;
    if (range > WORKER_REPAIR_BEAM_MAX_RANGE)
    {
        range = WORKER_REPAIR_BEAM_MAX_RANGE;
    }
    else if (range < WORKER_REPAIR_BEAM_MIN_RANGE)
    {
        range = WORKER_REPAIR_BEAM_MIN_RANGE;
    }

    delta.x = ship->posinfo.position.x - target->posinfo.position.x;
    delta.y = ship->posinfo.position.y - target->posinfo.position.y;
    delta.z = ship->posinfo.position.z - target->posinfo.position.z;
    matMultiplyVecByMat(&localPoint, &delta, &target->rotinfo.coordsys);
    shipGetCompoundAABB(target, compoundAABB);
    return compoundAABB[0] - (range + compoundAABB[3]) < localPoint.x &&
           localPoint.x < compoundAABB[0] + (range + compoundAABB[3]) &&
           compoundAABB[1] - (range + compoundAABB[4]) < localPoint.y &&
           localPoint.y < compoundAABB[1] + (range + compoundAABB[4]) &&
           compoundAABB[2] - (range + compoundAABB[5]) < localPoint.z &&
           localPoint.z < compoundAABB[2] + (range + compoundAABB[5]);
}

static bool32 workerRepairBeamFlyToTarget(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);
    Ship *target = workerRepairBeamTarget(spec);
    vector headingToTarget;
    bool32 inRange;
    bool32 facingTarget;
    vector shipHeading;
    WorkerHarvestStaticInfo *workerStatic = ship->staticinfo->workerHarvestStatic;

    dbgAssert(target != NULL);
    inRange = workerRepairBeamIsInRange(ship);
    headingToTarget.x = target->posinfo.position.x - ship->posinfo.position.x;
    headingToTarget.y = target->posinfo.position.y - ship->posinfo.position.y;
    headingToTarget.z = target->posinfo.position.z - ship->posinfo.position.z;
    vecNormalize(&headingToTarget);
    matGetVectFromMatrixCol3(shipHeading, ship->rotinfo.coordsys);
    facingTarget = vecDotProduct(shipHeading, headingToTarget) >= workerStatic->repairConeCosHalfAngle;
    if (inRange)
    {
        if (spec->unk_40 != 0)
        {
            vector adjustedDestination;
            real64 distanceToTarget;
            real64 scale;

            adjustedDestination.x = ship->posinfo.position.x - target->posinfo.position.x;
            adjustedDestination.y = ship->posinfo.position.y - target->posinfo.position.y;
            adjustedDestination.z = ship->posinfo.position.z - target->posinfo.position.z;
            distanceToTarget = sqrt(adjustedDestination.x * adjustedDestination.x +
                                    adjustedDestination.y * adjustedDestination.y +
                                    adjustedDestination.z * adjustedDestination.z);
            if (distanceToTarget < workerRepairBeamStandOffDistance(spec))
            {
                scale = (workerRepairBeamStandOffDistance(spec) - distanceToTarget) * (1.0 / distanceToTarget);
                adjustedDestination.x = (real32)(scale * adjustedDestination.x) + ship->posinfo.position.x;
                adjustedDestination.y = (real32)(scale * adjustedDestination.y) + ship->posinfo.position.y;
                adjustedDestination.z = (real32)(scale * adjustedDestination.z) + ship->posinfo.position.z;
                aishipFlyToPointAvoidingObjsFunc(ship, &adjustedDestination, 0, 0.0f, NULL);
            }
            else
            {
                aitrackForceGuidanceZeroRotation((SpaceObjRotImpTargGuidance *)ship);
                aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)ship, &target->posinfo.velocity);
            }
        }
        else
        {
            aitrackForceGuidanceZeroRotation((SpaceObjRotImpTargGuidance *)ship);
            aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)ship, &target->posinfo.velocity);
        }
    }
    else
    {
        aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 0, 0.0f, NULL);
    }

    headingToTarget.x = target->posinfo.position.x - ship->posinfo.position.x;
    headingToTarget.y = target->posinfo.position.y - ship->posinfo.position.y;
    headingToTarget.z = target->posinfo.position.z - ship->posinfo.position.z;
    vecNormalize(&headingToTarget);
    if (aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &headingToTarget,
                           WORKER_REPAIR_BEAM_ALIGN_DOT, WORKER_REPAIR_BEAM_ALIGN_DOT, 0.0f, 0))
    {
        aitrackZeroRotationAnywhere(ship);
    }
    return inRange && facingTarget;
}

static void repairBeamResetState(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);
    Ship *target = workerRepairBeamTarget(spec);

    if (target != NULL)
    {
        (*workerRepairBeamActiveCounter(target))--;
        if (ship->salCapDockApproachHost == target)
        {
            ship->salCapDockApproachHost = NULL;
        }
        workerSetRepairBeamTarget(spec, NULL);
    }
    workerDeleteRepairBeamEffect(spec);
    spec->unk_34 = WORKER_REPAIR_BEAM_STATE_IDLE;
}

// Decompiled from cataclysm.exe @ 0x5C89A0
static bool32 repairBeamAIUpdate(Ship *ship, SpecialCommand *targets)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);
    WorkerHarvestStaticInfo *workerStatic = ship->staticinfo->workerHarvestStatic;

    /* Retail tears down any in-progress beam acquisition/effect and resets the state
       machine to IDLE whenever the ship is dying/disabled, before even looking at
       the current state. */
    if ((ship->flags & 0x808800) != 0 || ship->disableStateStarted)
    {
        repairBeamResetState(ship);
        return FALSE;
    }

    switch (spec->unk_34)
    {
        case WORKER_REPAIR_BEAM_STATE_IDLE:
        {
            Ship *target;
            bool32 usedPriorityTarget;

            if (targets == NULL || targets->numTargets <= 0)
            {
                spec->unk_34 = WORKER_REPAIR_BEAM_STATE_FINISHED;
                return FALSE;
            }

            target = workerRepairBeamSelectTarget(ship, targets, &usedPriorityTarget);
            if (target == NULL)
            {
                return FALSE;
            }

            if (!usedPriorityTarget)
            {
                (*workerRepairBeamActiveCounter(target))++;
                ship->salCapDockApproachHost = target;
                workerSetRepairBeamTarget(spec, target);
                spec->unk_34 = WORKER_REPAIR_BEAM_STATE_FLY_TO_TARGET;
            }
            else
            {
                (*workerRepairBeamPriorityCounter(target))++;
                ship->salCapDockApproachHost = target;
                workerSetRepairBeamTarget(spec, target);
                workerRepairBeamFlyToTarget(ship);
                repairBeamResetState(ship);
            }
            return FALSE;
        }

        case WORKER_REPAIR_BEAM_STATE_FLY_TO_TARGET:
            if (workerRepairBeamFlyToTarget(ship))
            {
                spec->unk_34 = WORKER_REPAIR_BEAM_STATE_START_EFFECT;
            }
            return FALSE;

        case WORKER_REPAIR_BEAM_STATE_START_EFFECT:
        {
            /* Gates on the ship's mad-animation binding (the "connect" animation,
               index 0), not madStatic: keep flying/facing the target, (re)start the
               connect animation if it isn't running, and once it has played through
               to its paused end frame, create the beam effect and start repairing. */
            madanim *anim = ship->madBindings;
            sdword currentAnim = anim->nCurrentAnim;

            workerRepairBeamFlyToTarget(ship);

            if (currentAnim != 0)
            {
                if (currentAnim == MAD_NoAnimation || anim->bPaused)
                {
                    madAnimationStart(ship, 0);
                    spec->effectStartTime = universe.phystimeelapsed + workerStatic->jackRetractTime;
                }
            }
            else
            {
                spec->effectStartTime = universe.phystimeelapsed + workerStatic->jackRetractTime;
                if (anim->bPaused)
                {
                    workerCreateRepairBeamEffect(ship);
                    spec->unk_34 = WORKER_REPAIR_BEAM_STATE_REPAIRING;
                }
            }
            return FALSE;
        }

        case WORKER_REPAIR_BEAM_STATE_REPAIRING:
        {
            Ship *target = workerRepairBeamTarget(spec);
            Effect *effect = workerRepairBeamEffect(spec);
            Ship *mostDamagedShip;
            real32 repairAmount;
            madanim *anim;
            sdword currentAnim;

            dbgAssert(target != NULL);

            if (effect != NULL)
            {
                vector muzzleWorld;

                matMultiplyMatByVec(&muzzleWorld, &ship->rotinfo.coordsys, &workerStatic->jackhammerPosition);
                vecAddTo(muzzleWorld, ship->posinfo.position);
                effect->posinfo.position = muzzleWorld;
            }

            workerRepairBeamFlyToTarget(ship);

            /* Keep the connect animation looping/restarted while repairing, exactly
               as in START_EFFECT above. */
            anim = ship->madBindings;
            currentAnim = anim->nCurrentAnim;
            if (currentAnim == 0)
            {
                spec->effectStartTime = universe.phystimeelapsed + workerStatic->jackRetractTime;
            }
            else if (currentAnim == MAD_NoAnimation || anim->bPaused)
            {
                madAnimationStart(ship, 0);
                spec->effectStartTime = universe.phystimeelapsed + workerStatic->jackRetractTime;
            }

            /* The repair rate is looked up using the outer target's class, not the
               (possibly-inside) most-damaged ship's own class. */
            mostDamagedShip = workerFindMostDamagedShip(target);
            repairAmount = universe.phystimeelapsed * workerRepairRateForShip(target);
            if (tutorial == 1)
            {
                repairAmount *= WORKER_TUTORIAL_REPAIR_RATE_SCALE;
            }
            mostDamagedShip->health += repairAmount;
            if (mostDamagedShip->health > mostDamagedShip->staticinfo->maxhealth)
            {
                mostDamagedShip->health = mostDamagedShip->staticinfo->maxhealth;
            }
            if (mostDamagedShip->health == mostDamagedShip->staticinfo->maxhealth)
            {
                if (battleCanChatterAtThisTime(82, ship))
                {
                    battleChatterAttempt(-1, 82, ship, ship->shiptype);
                }
                workerDeleteRepairBeamEffect(spec);
                repairBeamResetState(ship);
            }
            return FALSE;
        }

        case WORKER_REPAIR_BEAM_STATE_FINISHED:
            repairBeamResetState(ship);
            return TRUE;

        default:
            return TRUE;
    }
}

// Decompiled from cataclysm.exe @ 0x5C9770
static bool32 WorkerTargetSpecial(Ship *ship, void *custom)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);
    bool32 result;

    if (spec->unk_08 == WORKER_SPECIAL_MODE_REPAIR_BEAM)
    {
        result = repairBeamAIUpdate(ship, (SpecialCommand *)custom);
    }
    else if (spec->unk_08 == WORKER_SPECIAL_MODE_SALVAGE)
    {
        result = SalCapExecuteSpecialOrder(ship, custom);
    }
    else
    {
        telldir();
        spec->unk_08 = WORKER_SPECIAL_MODE_IDLE;
        return TRUE;
    }
    if (result)
    {
        spec->unk_08 = WORKER_SPECIAL_MODE_IDLE;
    }
    return result;
}

// Decompiled from cataclysm.exe @ 0x5C97D0
static void WorkerSpecialOrderCleanup(void *context)
{
    Ship *ship = (Ship *)context;
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);

    SalCapDropTarget(ship);
    repairBeamResetState(ship);
    spec->unk_08 = WORKER_SPECIAL_MODE_IDLE;
}

// Decompiled from cataclysm.exe @ 0x5C9880
static void WorkerHousekeep(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);

    if ((((unsigned int)&aisTeams[30].selection[2].sel.ShipPtr[212]) & ship->flags) != 0)
    {
        workerDeleteShowDamageEffect(ship);
        ship->rowState = 0;
    }
    if (spec->effectStartTime < universe.phystimeelapsed)
    {
        madanim *anim = ship->madBindings;

        if (anim->nCurrentAnim != 1)
        {
            if (anim->bPaused)
            {
                madAnimationStart(ship, 1);
                spec->effectStartTime = WORKER_EFFECT_START_TIME_INACTIVE;
            }
        }
    }
    (void)SalCapProcessHostQueueScriptData(ship);
    if ((ship->flags & 0x808800) != 0 || ship->cuedAnimationIndex != 0)
    {
        repairBeamResetState(ship);
    }
    else if (workerRepairBeamTarget(spec) != NULL && workerRepairBeamEffect(spec) == NULL)
    {
        workerCreateRepairBeamEffect(ship);
    }
}

// Decompiled from cataclysm.exe @ 0x5C99F0
static void WorkerRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    if (shipToRemove == NULL)
    {
        return;
    }
    if (shipToRemove == workerRepairBeamTarget(workerShipSpecifics(ship)))
    {
        repairBeamResetState(ship);
    }
}

// Decompiled from cataclysm.exe @ 0x5C9AA0
static void WorkerClose(Ship *ship)
{
    workerDeleteShowDamageEffect(ship);
    ship->rowState = 0;
    SalCapDropTarget(ship);
    repairBeamResetState(ship);
}

// Decompiled from cataclysm.exe @ 0x5C9B90
static void WorkerPreFix(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);

    (void)PreFix_SalvagerSpecifics(ship);
    spec->unk_38 = SpaceObjRegistryGetID((SpaceObj *)workerRepairBeamTarget(spec));
    spec->unk_3C = 0;
}

// Decompiled from cataclysm.exe @ 0x5C9BC0
static void WorkerFix(Ship *ship)
{
    WorkerShipSpecifics *spec = workerShipSpecifics(ship);

    (void)Fix_SalvagerSpecifics(ship);
    workerSetRepairBeamTarget(spec, SpaceObjRegistryGetShip((sdword)spec->unk_38));
}

// Decompiled from cataclysm.exe @ 0x5C9BF0
static bool32 WorkerCanHalt(Ship *ship)
{
    workerDeleteShowDamageEffect(ship);
    ship->rowState = 0;
    return TRUE;
}

CustShipHeader WorkerHeader =
{
    sizeof(WorkerShipSpecifics),
    WorkerStaticInit,                 /* 0x5C9580 */
    NULL,
    WorkerInit,                       /* 0x5C9710 */
    NULL,
    NULL,
    NULL,
    NULL,
    WorkerTargetSpecial,              /* 0x5C9770 */
    WorkerSpecialOrderCleanup,        /* 0x5C97D0 */
    WorkerHousekeep,                  /* 0x5C9880 */
    WorkerRemoveShipReferences,       /* 0x5C99F0 */
    WorkerRemoveShipReferences,       /* 0x5C99F0 */
    NULL,
    NULL,
    WorkerClose,                      /* 0x5C9AA0 */
    WorkerPreFix,                     /* 0x5C9B90 */
    NULL,
    NULL,
    WorkerFix,                        /* 0x5C9BC0 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    pingEvalAlwaysTrue,
    WorkerCanHalt,                    /* 0x5C9BF0 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
