/*=============================================================================
    Name    : RammingFrigate.c
    Purpose : Cataclysm-only Ramming Frigate helpers recovered from retail
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "AIShip.h"
#include "AITrack.h"
#include "Battle.h"
#include "Clamp.h"
#include "Collision.h"
#include "CommandLayer.h"
#include "CommandWrap.h"
#include "dock.h"
#include "fastmath.h"
#include "matrix.h"
#include "meshanim.h"
#include "Physics.h"
#include "Ship.h"
#include "spaceobj.h"
#include "RammingFrigate.h"
#include "SaveGame.h"
#include "Sensors.h"
#include "shipselect.h"
#include "Timer.h"
#include "etg.h"
#include "linkedlist.h"
#include "objtypes.h"
#include "SoundEvent.h"
#include "trails.h"
#include "UnivUpdate.h"
#include "ACV.h"
#include "BigGun.h"
#include "universe.h"

static udword rammingFrigateSpecialParamsDisabled[6];
static udword rammingFrigateSpecialParamsEnabled[6];

static const real32 rammingFrigateState3ResetDelay = 25.0f;
static const real32 rammingFrigateEngineSpeedUpgradeScale = 4.0f;
static const real32 rammingFrigateEngineSpeedBaseScale = 1.3f;
static const real32 rammingFrigateHeadingBlendWeight = 0.07f;
static const real32 rammingFrigateHeadingCurrentWeight = 1.0f;
static const real32 rammingFrigateVelocityDampenCap = 0.1f;
static const real32 rammingFrigateVelocityScale = 1.2f;
static const real32 rammingFrigateVelocityScaleUpgrade = 2.0f;
static const real32 rammingFrigateTransitionDistanceCapital = 2000.0f;
static const real32 rammingFrigateTransitionDistanceAny = 600.0f;
static const real32 rammingFrigateAbortDistanceFighter = 700.0f;
static const real32 rammingFrigateHeadingDotToCharge = 0.5f;
static const real32 rammingFrigateHeadingDotToMaintain = 0.34f;
static const real32 rammingFrigateLineOfSightRetry = 4.0f;
static const real32 rammingFrigateSnapLineLength = 500.0f;
static const real32 rammingFrigateLeadDistance = -1500.0f;
static const real32 rammingFrigateClampImpactDelay = 0.8f;
static const real32 rammingFrigateClampImpactEffectDelay = 0.3f;
static const real32 rammingFrigateClampDamageOffset = 500000.0f;
static const real32 rammingFrigateClampDamageOffsetUpgrade = 800000.0f;
static const real32 rammingFrigateTrackHeadingAccuracy = 0.2f;
static const real32 rammingFrigateTrackHeadingTorque = 400.0f;
static const real32 rammingFrigateApproachSpeed = 99999.0f;
static const vector rammingFrigateClampPoint0 = { 37.0f, -50.0f, -43.0f };
static const vector rammingFrigateClampPoint1 = { -34.0f, -50.0f, -43.0f };
static const vector rammingFrigateClampPoint2 = { 37.0f, 50.0f, -43.0f };
static const vector rammingFrigateClampPoint3 = { -34.0f, 50.0f, -43.0f };
static const vector rammingFrigateTrailEffectOffset = { -75.0f, 0.0f, -25.0f };
static char aSnapcenter[] = "SnapCenter";
static char aSnapstarbft[] = "SnapStarBFT";
static char aSnapstarbfb[] = "SnapStarBFB";
static char aSnapportft[] = "SnapPortFT";
static char aSnapportfb[] = "SnapPortFB";
static char aSnapstarbbt[] = "SnapStarBBT";
static char aSnapstarbbb[] = "SnapStarBBB";
static char aSnapportbt[] = "SnapPortBT";
static char aSnapportbb[] = "SnapPortBB";

bool32 shipIsRammingFrigate(Ship *ship);
bool32 shipIsInRammingMode(Ship *ship);
sdword shipQueueBeastConversion(Ship *ship, Ship *attacker);
sdword fogObjectVisibleToPlayer(Player *player, SpaceObj *obj, sdword treatMultiplayerAsVisible);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
real32 TimerElapsed(Timer *timer);
extern sdword nisWideScreenActive;
void telldir(void);

void spaceobjTransformPointToWorldSpace(real32 *dest, real32 *source, real32 *coordSystem)
{
    vector result;

    matMultiplyMatByVec(&result, (matrix *)(coordSystem + 38), (vector *)source);
    dest[0] = result.x + coordSystem[22];
    dest[1] = result.y + coordSystem[23];
    dest[2] = result.z + coordSystem[24];
}

static bool32 rammingFrigateShipIsSalCapFrigate(Ship *ship)
{
    return ship != NULL && ship->objtype == OBJ_ShipType &&
           (ship->shiptype == sSentinel || ship->shiptype == bSentinel);
}

static bool32 rammingFrigateShipIsBeastRace(Ship *ship)
{
    return ship != NULL && ship->shiptype == sLeech;
}

static sdword rammingFrigateSalCapCantDoFormation(Ship *ship)
{
    if (ship == NULL)
    {
        return FALSE;
    }

    switch (ship->shiptype)
    {
        case sWorker:
        case bWorker:
        case kSalCapCorvette:
        case tSalCapCorvette:
        case p1StandardCorvette:
            return TRUE;

        case trJunkYardDawg:
            telldir();
            return FALSE;

        default:
            return FALSE;
    }
}

static Timer *rammingFrigateCounterTimer(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->counterTimer;
}

static Timer *rammingFrigateClampTimer(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->clampTimer;
}

static Timer *rammingFrigateHeadingResetTimer(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->headingResetTimer;
}

static Timer *rammingFrigateImpactEffectTimer(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->impactEffectTimer;
}

static Timer *rammingFrigateLineOfSightTimer(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->lineOfSightTimer;
}

static Effect **rammingFrigateEffectPtr(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->effect;
}

static sdword *rammingFrigateClampDamageStarted(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->clampDamageStarted;
}

static vector *rammingFrigateClampTrackVector(Ship *ship)
{
    return &rammingFrigateSpecifics(ship)->clampTrackVector;
}

static void rammingFrigateTransformPointToWorldSpace(vector *dest, const vector *source, Ship *ship)
{
    spaceobjTransformPointToWorldSpace((real32 *)dest, (real32 *)source, (real32 *)ship);
}

static bool32 rammingFrigateIsCrystal(Ship *ship)
{
    return IsCrystal(ship);
}

static bool32 rammingFrigateIsSameGroup(SpaceObj *obj1, SpaceObj *obj2)
{
    ClampInfo *clamp1;
    ClampInfo *clamp2;
    SpaceObj *group1;
    SpaceObj *group2;

    if (obj1 == obj2)
    {
        return TRUE;
    }
    if (obj1 == NULL || obj2 == NULL || obj1->objtype != OBJ_ShipType || obj2->objtype != OBJ_ShipType)
    {
        return FALSE;
    }

    clamp1 = ((Ship *)obj1)->clampInfo;
    clamp2 = ((Ship *)obj2)->clampInfo;
    if (clamp1 == NULL || clamp2 == NULL)
    {
        return FALSE;
    }

    group1 = clamp1->host;
    group2 = clamp2->host;
    if (group1 != NULL && group1 == group2)
    {
        return TRUE;
    }

    return group1 == obj2 || group2 == obj1;
}

// Cataclysm-specific: large derelicts that can carry or expose the siege cannon.
bool32 shipIsLargeDerelict(Ship *ship)
{
    return ship->shiptype == dRKO ||
           ship->shiptype == dNomadMoon ||
           ship->shiptype == dNomadFieldEmitter;
}

// Cataclysm-specific: retail static-init callback for the ramming frigate special target data.
void rammingFrigateSelectParams(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    udword *specialParams = rammingFrigateSpecialParamsDisabled;
    const ShipStaticInfoCataclysmTail *cataTail = shipStaticInfoCataclysmTailConst(statinfo);

    if (statinfo->shiprace != 0)
    {
        specialParams = rammingFrigateSpecialParamsEnabled;
    }

    specialParams[1] = cataTail->unk418;
    specialParams[2] = cataTail->bUseRayTriangleIntersectionForHitLocation;
    specialParams[3] = TreatAsUdword(cataTail->linkDistance);
    specialParams[4] = cataTail->supportUnits;
    specialParams[5] = cataTail->UpgradeByDocking;

    statinfo->custstatinfo = specialParams;
    statinfo->specialActivateIsContinuous = 1;
    statinfo->canSpecialBandBoxFriendlies = 1;
    statinfo->unk4 = 0;

    (void)directory;
    (void)filename;
}


static bool32 shipOrDockedCanHaveBigGun(Ship *ship)
{
    ShipsInsideMe *shipsInsideMe;
    Node *node;

    if (ship->objtype == OBJ_ShipType)
    {
        if (shipIsLargeDerelict(ship) || shipCanHaveBigGun(ship))
        {
            return TRUE;
        }
    }

    shipsInsideMe = ship->shipsInsideMe;
    if (shipsInsideMe == NULL)
    {
        return FALSE;
    }

    for (node = shipsInsideMe->insideList.head; node != NULL; node = node->next)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
        Ship *dockedShip = insideShip->ship;

        if (dockedShip != NULL && (shipIsLargeDerelict(dockedShip) || shipCanHaveBigGun(dockedShip)))
        {
            return TRUE;
        }
    }

    return FALSE;
}


static bool32 rammingFrigateInitCounterTimer(Ship *ship)
{
    if (ship->shiptype == sRammingFrigate || ship->shiptype == bRammingFrigate)
    {
        TimerInitTimerStruct(rammingFrigateCounterTimer(ship), ((real32 *)ship->staticinfo->custstatinfo)[5]);
    }
    return TRUE;
}


static bool32 rammingFrigateSetState(Ship *ship, real32 stateValue)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    ubyte newState = (ubyte)stateValue;
    Effect *effect = specifics->effect;
    shiptrail *trail = ship->trail[1];
    bool32 oldUsesSlowEngine;
    bool32 newUsesSlowEngine;

    if (effect != NULL)
    {
        etgEffectDelete(effect);
        univRemoveObjFromRenderList((SpaceObj *)effect);
        listDeleteNode((Node *)effect);
        specifics->effect = NULL;
    }

    if (specifics->ramState == 5 && newState != 5)
    {
        clDetachShipFromCommand(ship);
        madAnimationStart(ship, 1);
        TimerInitTimerStruct(rammingFrigateCounterTimer(ship), ((real32 *)ship->staticinfo->custstatinfo)[5]);
    }

    if (newState == 0)
    {
        ship->aistateattack = 1;
        ship->flightmanState1 = 1;
        specifics->target = NULL;
    }
    else if (newState == 3 && specifics->ramState != 3)
    {
        TimerInitTimerStruct(rammingFrigateHeadingResetTimer(ship), rammingFrigateState3ResetDelay);
    }

    if (specifics->ramState == 3)
    {
        if (newState != 5)
        {
            madAnimationStart(ship, 4);
        }
    }

    if (newState == 5 && specifics->ramState != 5)
    {
        if (ship->soundevent.hyperspaceHandle != SOUND_NOTINITED)
        {
            soundEventStopSound(ship->soundevent.hyperspaceHandle, 0);
            ship->soundevent.hyperspaceHandle = SOUND_NOTINITED;
        }
        specifics->clampDamageStarted = 1;
        TimerInitTimerStruct(rammingFrigateImpactEffectTimer(ship), rammingFrigateClampImpactDelay);
    }

    if (trail != NULL)
    {
        real32 ribbonAdjust = trail->ribbonadjust;

        oldUsesSlowEngine = specifics->ramState == 5 || specifics->ramState == 3 || specifics->ramState == 4;
        newUsesSlowEngine = newState == 5 || newState == 3 || newState == 4;
        if (oldUsesSlowEngine != newUsesSlowEngine)
        {
            real32 stateScale = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER) ? rammingFrigateEngineSpeedUpgradeScale : rammingFrigateEngineSpeedBaseScale;

            if (!newUsesSlowEngine)
            {
                stateScale = 1.0f / stateScale;
            }
            ribbonAdjust *= stateScale;
        }

        trail->ribbonadjust = ribbonAdjust;
    }

    specifics->ramState = newState;
    return TRUE;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5C0AE0
void workerCheckResetForHyperspace(Ship *ship)
{
    if ((ship->shiptype == bRammingFrigate || ship->shiptype == sRammingFrigate) &&
        rammingFrigateSpecifics(ship)->ramState == 5)
    {
        rammingFrigateSetState(ship, 0.0f);
    }
}


static Effect *rammingFrigateCreateEffect(Ship *ship)
{
    etglod *lod = etgSpecialPurposeEffectTable[EGT_BULLET_CUTTING_BEAM];
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    Effect *effect = specifics->effect;

    if (effect != NULL)
    {
        etgEffectDelete(effect);
        univRemoveObjFromRenderList((SpaceObj *)effect);
        listDeleteNode((Node *)effect);
        specifics->effect = NULL;
    }

    if (lod != NULL &&
        ship->currentLOD < lod->nLevels &&
        lod->level[ship->currentLOD] != NULL &&
        etgHitEffectsEnabled &&
        !etgFrequencyExceeded(lod->level[ship->currentLOD]))
    {
        effect = etgEffectCreate(lod->level[ship->currentLOD], ship, NULL, NULL, NULL, 1.0f, 120, 0);
        specifics->effect = effect;
        return effect;
    }

    return NULL;
}



// Cataclysm-specific: retail init callback for the ramming frigate state block.
void rammingFrigateInit(Ship *ship)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);

    specifics->unk_1C = 0x12C;
    specifics->ramState = 0;
    specifics->target = NULL;
    specifics->effect = NULL;
    specifics->unk_40 = 0;
    specifics->targetIsHostile = 0;
    TimerInitTimerStruct(rammingFrigateCounterTimer(ship), 0.01f);
    TimerInitTimerStruct(rammingFrigateHeadingResetTimer(ship), 0.01f);
    TimerInitTimerStruct(rammingFrigateLineOfSightTimer(ship), 0.01f);
    specifics->clampDamageStarted = 0;
    TimerInitTimerStruct(rammingFrigateImpactEffectTimer(ship), 0.01f);
}


bool32 rammingFrigateSetTarget(Ship *ship, void *custom)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    SpaceObj *obj = (SpaceObj *)custom;
    SpaceObj *currentTarget;

    if (obj == NULL || obj->objtype == OBJ_ShipType)
    {
        currentTarget = specifics->target;
        if (currentTarget != obj)
        {
            if (currentTarget == NULL ||
                obj == NULL ||
                currentTarget->objtype != OBJ_ShipType ||
                !rammingFrigateIsSameGroup(currentTarget, obj))
            {
                rammingFrigateSetState(ship, 0.0f);
                rammingFrigateSetState(ship, 1.0f);
                specifics->target = obj;
                specifics->targetIsHostile = !univIsObjFriendly(obj, ship->playerowner);
                if (rammingFrigateIsCrystal((Ship *)obj))
                {
                    specifics->targetIsHostile = FALSE;
                }
            }
        }
    }

    return obj == NULL;
}


bool32 rammingFrigateReset(Ship *ship)
{
    rammingFrigateSpecifics(ship)->target = NULL;
    rammingFrigateSetState(ship, 0.0f);
    ship->aistateattack = 1;
    ship->flightmanState1 = 1;
    return TRUE;
}


bool32 rammingFrigateSelectTarget(Ship *ship, void *custom)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    SelectAnyCommand *targets = (SelectAnyCommand *)custom;
    SpaceObj *selectedTarget = NULL;
    sdword i;

    if (targets->numTargets <= 0)
    {
        return TRUE;
    }

    for (i = 0; i < targets->numTargets; i++)
    {
        SpaceObj *candidate = (SpaceObj *)targets->TargetPtr[i];
        vector *position = &((SpaceObjRotImp *)candidate)->posinfo.position;

        if (candidate->objtype == OBJ_ShipType &&
            position->x <= smUniverseSizeX && position->x >= -smUniverseSizeX &&
            position->y <= smUniverseSizeY && position->y >= -smUniverseSizeY &&
            position->z <= smUniverseSizeZ && position->z >= -smUniverseSizeZ)
        {
            selectedTarget = candidate;
        }
    }

    if (selectedTarget == NULL)
    {
        return TRUE;
    }

    if (selectedTarget != specifics->target &&
        !rammingFrigateIsSameGroup(specifics->target, selectedTarget))
    {
        if (battleCanChatterAtThisTime(BCE_KickingButtRsp, ship))
        {
            battleChatterAttempt(-1, BCE_KickingButtRsp, ship, -1);
        }

        rammingFrigateSetState(ship, 0.0f);
        rammingFrigateSetState(ship, 1.0f);
        specifics->target = selectedTarget;
        specifics->targetIsHostile = !univIsObjFriendly(selectedTarget, ship->playerowner);
        if (rammingFrigateIsCrystal((Ship *)selectedTarget))
        {
            specifics->targetIsHostile = FALSE;
        }
    }

    return FALSE;
}


void rammingFrigateClearTarget(Ship *ship, Ship *shipToRemove)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);

    if ((SpaceObj *)shipToRemove == specifics->target)
    {
        if ((ship->shiptype == sRammingFrigate || ship->shiptype == bRammingFrigate) &&
            specifics->ramState == 5)
        {
            rammingFrigateSetState(ship, 0.0f);
        }
        rammingFrigateSetState(ship, 0.0f);
        specifics->target = NULL;
    }
}



void rammingFrigateBlendHeadingWeighted(SpaceObjRotImp *irobj, vector *heading, real32 weight, real32 currentWeight)
{
    vector blended;
    real32 totalWeight = weight + currentWeight;

    blended.x = (weight * heading->x + currentWeight * irobj->rotinfo.coordsys.m13) / totalWeight;
    blended.y = (weight * heading->y + currentWeight * irobj->rotinfo.coordsys.m23) / totalWeight;
    blended.z = (weight * heading->z + currentWeight * irobj->rotinfo.coordsys.m33) / totalWeight;
    vecNormalize(&blended);
    matCreateCoordSysFromHeading(&irobj->rotinfo.coordsys, &blended);
    collUpdateCollRectangle(irobj);
}


void rammingFrigateSetHeading(SpaceObjRotImp *irobj, vector *heading)
{
    vector normalized = *heading;

    vecNormalize(&normalized);
    matCreateCoordSysFromHeading(&irobj->rotinfo.coordsys, &normalized);
    collUpdateCollRectangle(irobj);
}


void rammingFrigateBlendHeading(SpaceObjRotImp *irobj, vector *heading)
{
    rammingFrigateBlendHeadingWeighted(irobj, heading, rammingFrigateHeadingBlendWeight, rammingFrigateHeadingCurrentWeight);
}



bool32 rammingFrigateResetState(Ship *ship)
{
    rammingFrigateSetState(ship, 0.0f);
    return TRUE;
}

bool32 rammingFrigateMoveConstraint(Ship *ship, vector initialPoint, sdword pointCount, vector *points)
{
    (void)initialPoint;
    (void)pointCount;
    (void)points;
    return rammingFrigateResetState(ship);
}

bool32 rammingFrigateDockStart(Ship *ship, sdword dockType, Ship *dockwith)
{
    (void)dockType;
    (void)dockwith;
    return rammingFrigateResetState(ship);
}

bool32 rammingFrigateCanHalt(Ship *ship)
{
    return rammingFrigateResetState(ship);
}

// Cataclysm-specific: restore saved Ramming Frigate target references after load.
void rammingFrigateFixupPointers(Ship *ship)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);

    specifics->target = (SpaceObj *)SpaceObjRegistryGetShip((sdword)specifics->target);
    specifics->effect = NULL;
}

// Cataclysm-specific: serialize the transient Ramming Frigate target/effect state.
void rammingFrigateSave(Ship *ship)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);

    specifics->target = (SpaceObj *)SpaceObjRegistryGetID(specifics->target);
    if (specifics->effect != NULL)
    {
        etgEffectDelete(specifics->effect);
        univRemoveObjFromRenderList((SpaceObj *)specifics->effect);
        listDeleteNode((Node *)specifics->effect);
        specifics->effect = NULL;
    }
}

// Cataclysm-specific: retail free-contents callback clears the current special target.
void rammingFrigateResetTarget(Ship *ship)
{
    rammingFrigateSpecifics(ship)->target = NULL;
}

// Cataclysm-specific: retail object-delete callback clears tracked transient references.
void rammingFrigateClearDeadRef(Ship *ship, sdword objectID)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);

    if ((sdword)specifics->effect == objectID)
    {
        specifics->effect = NULL;
    }
}

// Cataclysm-specific: retail close callback tears down the clamp effect and target reference.
void rammingFrigateDeleteEffect(Ship *ship)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    SpaceObj *effectObj = (SpaceObj *)specifics->effect;

    if (effectObj != NULL)
    {
        etgEffectDelete((Effect *)effectObj);
        univRemoveObjFromRenderList(effectObj);
        listDeleteNode((Node *)effectObj);
        specifics->effect = NULL;
    }

    specifics->target = NULL;
}








static void rammingFrigateGetClampPoints(Ship *ship, vector *p0, vector *p1, vector *p2, vector *p3, vector *forward, vector *side)
{
    vector axis;

    rammingFrigateTransformPointToWorldSpace(p0, &rammingFrigateClampPoint0, ship);
    rammingFrigateTransformPointToWorldSpace(p1, &rammingFrigateClampPoint1, ship);
    rammingFrigateTransformPointToWorldSpace(p2, &rammingFrigateClampPoint2, ship);
    rammingFrigateTransformPointToWorldSpace(p3, &rammingFrigateClampPoint3, ship);

    axis.x = p2->x - p0->x;
    axis.y = p2->y - p0->y;
    axis.z = p2->z - p0->z;
    vecNormalize(&axis);

    *forward = axis;
    side->x = -axis.x;
    side->y = -axis.y;
    side->z = -axis.z;
}


static Effect *rammingFrigateCreateImpactEffect(Ship *ship, vector *position, vector *heading)
{
    etglod *lod = etgSpecialPurposeEffectTable[EGT_RAMMING_FRIGATE_IMPACT_EFFECT];
    matrix coordsys;
    vector velocity = { 0.0f, 0.0f, 0.0f };

    if (lod == NULL ||
        lod->nLevels <= 0 ||
        lod->level[0] == NULL ||
        !etgHitEffectsEnabled ||
        etgFrequencyExceeded(lod->level[0]) ||
        ship->currentLOD >= 3)
    {
        return NULL;
    }

    matCreateCoordSysFromHeading(&coordsys, heading);
    velocity = ship->posinfo.velocity;
    return etgEffectCreate(lod->level[0], NULL, position, &velocity, &coordsys, 1.0f, 120, 1);
}






bool32 univIsPositionInBounds(vector *position)
{
    return position->x <= smUniverseSizeX && position->x >= -smUniverseSizeX &&
           position->y <= smUniverseSizeY && position->y >= -smUniverseSizeY &&
           position->z <= smUniverseSizeZ && position->z >= -smUniverseSizeZ;
}


bool32 shipIsRammingFrigateActive(Ship *ship)
{
    return (ship->shiptype == sRammingFrigate || ship->shiptype == bRammingFrigate) &&
           rammingFrigateSpecifics(ship)->ramState != 0;
}


static real32 rammingFrigateCheckSnapLine(SpaceObjRotImp *target, vector *direction, vector *sourcePosition, vector *deltaOut)
{
    vector start;
    sdword collSide;
    real32 hitDistance;

    start.x = sourcePosition->x - direction->x * rammingFrigateSnapLineLength;
    start.y = sourcePosition->y - direction->y * rammingFrigateSnapLineLength;
    start.z = sourcePosition->z - direction->z * rammingFrigateSnapLineLength;

    hitDistance = collCheckRectLine(target, &start, direction, rammingFrigateSnapLineLength * 2.0f, &collSide);
    if (hitDistance >= 0.0f)
    {
        vector hitPoint;

        hitPoint.x = start.x + direction->x * hitDistance;
        hitPoint.y = start.y + direction->y * hitDistance;
        hitPoint.z = start.z + direction->z * hitDistance;

        deltaOut->x = hitPoint.x - sourcePosition->x;
        deltaOut->y = hitPoint.y - sourcePosition->y;
        deltaOut->z = hitPoint.z - sourcePosition->z;
    }

    return hitDistance;
}

static void rammingFrigateFindClosestSnapHit(
    Ship *ship,
    SpaceObjRotImp *target,
    DockStaticPoint **snapPoints,
    sdword *foundHit,
    real32 *bestDistance,
    vector *newPosition,
    vector *predictedTargetPosition,
    vector *approachDirection,
    vector *shipPosition,
    SpaceObjRotImp **snappedTarget)
{
    sdword i;

    for (i = 0; i < 9; i++)
    {
        DockStaticPoint *dockPoint = snapPoints[i];
        vector worldPoint;
        vector hitOffset;
        vector snappedPosition;
        real32 hitDistance;

        if (dockPoint == NULL)
        {
            continue;
        }

        rammingFrigateTransformPointToWorldSpace(&worldPoint, &dockPoint->position, ship);
        hitDistance = rammingFrigateCheckSnapLine(target, approachDirection, &worldPoint, &hitOffset);
        if (hitDistance < 0.0f)
        {
            continue;
        }

        snappedPosition.x = shipPosition->x + hitOffset.x;
        snappedPosition.y = shipPosition->y + hitOffset.y;
        snappedPosition.z = shipPosition->z + hitOffset.z;

        if (!*foundHit)
        {
            *foundHit = 1;
            *newPosition = snappedPosition;
            *snappedTarget = target;
            *bestDistance = GetDistance(&snappedPosition, predictedTargetPosition);
            continue;
        }

        hitDistance = GetDistance(&snappedPosition, predictedTargetPosition);
        if (hitDistance < *bestDistance)
        {
            *bestDistance = hitDistance;
            *newPosition = snappedPosition;
            *snappedTarget = target;
        }
    }
}

static sdword rammingFrigateSnapToTarget(Ship *ship, SpaceObjRotImp *target, SpaceObjRotImp **snappedTarget)
{
    vector predictedTargetPosition;
    vector shipPosition;
    vector approachDirection;
    vector newPosition;
    DockStaticPoint *snapPoints[9];
    SpaceObjRotImp *groupTarget;
    SelectCommand *attachedObjects;
    sdword foundHit;
    real32 bestDistance;
    sdword i;

    shipPosition = ship->posinfo.position;
    approachDirection.x = ship->rotinfo.coordsys.m13;
    approachDirection.y = ship->rotinfo.coordsys.m23;
    approachDirection.z = ship->rotinfo.coordsys.m33;
    newPosition.x = approachDirection.x * rammingFrigateLeadDistance;
    newPosition.y = approachDirection.y * rammingFrigateLeadDistance;
    newPosition.z = approachDirection.z * rammingFrigateLeadDistance;
    predictedTargetPosition.x = shipPosition.x + newPosition.x;
    predictedTargetPosition.y = shipPosition.y + newPosition.y;
    predictedTargetPosition.z = shipPosition.z + newPosition.z;

    snapPoints[0] = dockFindDockStaticPoint(aSnapcenter, ship->staticinfo->dockStaticInfo);
    snapPoints[1] = dockFindDockStaticPoint(aSnapstarbft, ship->staticinfo->dockStaticInfo);
    snapPoints[2] = dockFindDockStaticPoint(aSnapstarbfb, ship->staticinfo->dockStaticInfo);
    snapPoints[3] = dockFindDockStaticPoint(aSnapportft, ship->staticinfo->dockStaticInfo);
    snapPoints[4] = dockFindDockStaticPoint(aSnapportfb, ship->staticinfo->dockStaticInfo);
    snapPoints[5] = dockFindDockStaticPoint(aSnapstarbbt, ship->staticinfo->dockStaticInfo);
    snapPoints[6] = dockFindDockStaticPoint(aSnapstarbbb, ship->staticinfo->dockStaticInfo);
    snapPoints[7] = dockFindDockStaticPoint(aSnapportbt, ship->staticinfo->dockStaticInfo);
    snapPoints[8] = dockFindDockStaticPoint(aSnapportbb, ship->staticinfo->dockStaticInfo);

    *snappedTarget = NULL;
    foundHit = 0;
    bestDistance = 0.0f;
    rammingFrigateFindClosestSnapHit(
        ship,
        target,
        snapPoints,
        &foundHit,
        &bestDistance,
        &newPosition,
        &predictedTargetPosition,
        &approachDirection,
        &shipPosition,
        snappedTarget);

    groupTarget = target;
    if (((Ship *)target)->clampInfo != NULL && ((Ship *)target)->clampInfo->host != NULL)
    {
        groupTarget = (SpaceObjRotImp *)((Ship *)target)->clampInfo->host;
        if (groupTarget != target)
        {
            rammingFrigateFindClosestSnapHit(
                ship,
                groupTarget,
                snapPoints,
                &foundHit,
                &bestDistance,
                &newPosition,
                &predictedTargetPosition,
                &approachDirection,
                &shipPosition,
                snappedTarget);
        }
    }

    if (((Ship *)groupTarget)->clampInfo != NULL)
    {
        attachedObjects = (SelectCommand *)((Ship *)groupTarget)->clampInfo->selection;
        if (attachedObjects != NULL)
        {
            for (i = 0; i < attachedObjects->numShips; i++)
            {
                Ship *attachedShip = attachedObjects->ShipPtr[i];

                if (attachedShip == NULL)
                {
                    continue;
                }
                if (rammingFrigateShipIsBeastRace(attachedShip) || rammingFrigateShipIsSalCapFrigate(attachedShip))
                {
                    continue;
                }

                rammingFrigateFindClosestSnapHit(
                    ship,
                    (SpaceObjRotImp *)attachedShip,
                    snapPoints,
                    &foundHit,
                    &bestDistance,
                    &newPosition,
                    &predictedTargetPosition,
                    &approachDirection,
                    &shipPosition,
                    snappedTarget);
            }
        }
    }

    if (foundHit)
    {
        posSetShipPosition(ship, &newPosition);
    }

    if (foundHit == 1 && *snappedTarget != NULL)
    {
        if ((((Ship *)*snappedTarget)->specialFlags2 & SPECIAL_2_DockFlag) != 0)
        {
            return 0;
        }
        if ((*snappedTarget)->objtype == OBJ_ShipType &&
            ((((Ship *)*snappedTarget)->shiptype == sRammingFrigate) || (((Ship *)*snappedTarget)->shiptype == bRammingFrigate)))
        {
            return 0;
        }
    }

    return foundHit;
}

static sdword rammingFrigateProcessImpact(Ship *ship, SpaceObjRotImp *target)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    SpaceObjRotImp *impactTarget;
    SpaceObjRotImp *snappedTarget;
    SelectCommand *attachedObjects;
    SelectCommand removeSelection;
    sdword i;
    sdword canImpact;
    sdword abortImpact;
    ShipStaticInfo *rammingStatic;
    ShipStaticInfo *impactStatic;
    vector approachDirection;

    impactTarget = target;
    if (((Ship *)target)->clampInfo != NULL && ((Ship *)target)->clampInfo->host != NULL)
    {
        impactTarget = (SpaceObjRotImp *)((Ship *)target)->clampInfo->host;
    }

    canImpact = TRUE;
    if (impactTarget == NULL)
    {
        canImpact = FALSE;
    }
    else
    {
        if (((Ship *)impactTarget)->shiptype == sRammingFrigate || ((Ship *)impactTarget)->shiptype == bRammingFrigate)
        {
            canImpact = FALSE;
        }
        if (((Ship *)impactTarget)->clampInfo != NULL && ((Ship *)impactTarget)->clampInfo->host != NULL)
        {
            canImpact = FALSE;
        }
    }

    if (specifics->target != (SpaceObj *)impactTarget)
    {
        if (specifics->target == NULL ||
            ((Ship *)specifics->target)->clampInfo == NULL ||
            ((Ship *)specifics->target)->clampInfo->host != (SpaceObj *)impactTarget)
        {
            canImpact = FALSE;
        }
    }

    if (shipIsInRammingMode(ship))
    {
        canImpact = FALSE;
    }

    if (!univIsPositionInBounds(&ship->posinfo.position) || !canImpact)
    {
        rammingFrigateInitCounterTimer(ship);
        return rammingFrigateSetState(ship, 1.0f);
    }

    if (((Ship *)impactTarget)->clampInfo != NULL)
    {
        attachedObjects = (SelectCommand *)((Ship *)impactTarget)->clampInfo->selection;
        if (attachedObjects != NULL)
        {
            abortImpact = FALSE;
            for (i = 0; i < attachedObjects->numShips; i++)
            {
                Ship *attachedShip = attachedObjects->ShipPtr[i];

                if (attachedShip == NULL)
                {
                    continue;
                }
                if ((attachedShip->shiptype == sRammingFrigate || attachedShip->shiptype == bRammingFrigate) &&
                    collCheckBBoxBumpCollision((SpaceObjRotImp *)ship, (SpaceObjRotImp *)attachedShip))
                {
                    if (shipIsInRammingMode(attachedShip))
                    {
                        rammingFrigateSetState(attachedShip, 0.0f);
                    }
                    abortImpact = TRUE;
                }
            }
            if (abortImpact)
            {
                rammingFrigateInitCounterTimer(ship);
                return rammingFrigateSetState(ship, 1.0f);
            }
        }
    }

    if (shipIsRammingFrigate((Ship *)specifics->target))
    {
        rammingFrigateInitCounterTimer(ship);
        return rammingFrigateSetState(ship, 1.0f);
    }

    abortImpact = TRUE;
    if (ship->clampInfo != NULL)
    {
        attachedObjects = (SelectCommand *)ship->clampInfo->selection;
        if (attachedObjects != NULL)
        {
            for (i = attachedObjects->numShips - 1; i >= 0; i--)
            {
                Ship *attachedShip = attachedObjects->ShipPtr[i];

                if (attachedShip == NULL)
                {
                    continue;
                }
                if (rammingFrigateShipIsBeastRace(attachedShip))
                {
                    removeSelection.numShips = 1;
                    removeSelection.timeLastStatus = 0.0f;
                    removeSelection.ShipPtr[0] = attachedShip;
                    RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &removeSelection);
                }
                else
                {
                    rammingFrigateSalCapCantDoFormation(attachedShip);
                    abortImpact = FALSE;
                }
            }
            if (!abortImpact)
            {
                rammingFrigateInitCounterTimer(ship);
                return rammingFrigateSetState(ship, 1.0f);
            }
        }
    }

    if (singlePlayerGame && nisWideScreenActive == 1)
    {
        rammingFrigateInitCounterTimer(ship);
        return rammingFrigateSetState(ship, 1.0f);
    }

    vecSub(approachDirection, impactTarget->posinfo.position, ship->posinfo.position);
    vecNormalize(&approachDirection);
    rammingFrigateBlendHeadingWeighted(ship, &approachDirection, 1.0f, 0.0f);

    snappedTarget = NULL;
    if (rammingFrigateSnapToTarget(ship, impactTarget, &snappedTarget) != 1)
    {
        if (snappedTarget != NULL &&
            shipIsRammingFrigate((Ship *)snappedTarget) &&
            shipIsInRammingMode((Ship *)snappedTarget))
        {
            rammingFrigateSetState((Ship *)snappedTarget, 0.0f);
        }
        rammingFrigateInitCounterTimer(ship);
        return rammingFrigateSetState(ship, 1.0f);
    }

    rammingStatic = ship->staticinfo;
    impactStatic = ((Ship *)snappedTarget)->staticinfo;
    ApplyDamageToTarget((SpaceObjRotImpTarg *)snappedTarget, rammingStatic->RamDamage, GS_LargeEnergyCannon, 666, ship->playerowner->playerIndex);

    if (impactStatic->shipclass == CLASS_Micro ||
        impactStatic->shipclass == CLASS_Fighter ||
        shipIsACV((Ship *)snappedTarget) ||
        ((Ship *)snappedTarget)->shiptype == sMCV ||
        ((Ship *)snappedTarget)->shiptype == bHeavyCorvette)
    {
        if (((Ship *)snappedTarget)->health >= 0.0f)
        {
            ApplyDamageToTarget((SpaceObjRotImpTarg *)snappedTarget, ((Ship *)snappedTarget)->health + 1.0f, GS_LargeEnergyCannon, 666, ship->playerowner->playerIndex);
        }
    }

    if (((Ship *)snappedTarget)->health < 0.0f)
    {
        TimerInitTimerStruct(rammingFrigateCounterTimer(ship), ship->staticinfo->RamWaitAfterPushTime);
        return rammingFrigateSetState(ship, 1.0f);
    }

    specifics->target = (SpaceObj *)impactTarget;
    clampObjectSetup((SpaceObj *)ship, (SpaceObj *)impactTarget);
    TimerInitTimerStruct(rammingFrigateClampTimer(ship), ship->staticinfo->RamPushTime);
    rammingFrigateSetState(ship, 5.0f);
    rammingFrigateClampTrackVector(ship)->x = impactTarget->rotinfo.coordsys.m13;
    rammingFrigateClampTrackVector(ship)->y = impactTarget->rotinfo.coordsys.m23;
    rammingFrigateClampTrackVector(ship)->z = impactTarget->rotinfo.coordsys.m33;
    madAnimationStart(ship, 0);
    rammingFrigateCreateEffect(ship);
    soundEventPlay(ship, 2076, 0);
    if (battleCanChatterAtThisTime(66, ship))
    {
        return battleChatterAttempt(-1, 66, ship, -1);
    }
    return 0;
}

sdword rammingFrigateCollisionCallback(SpaceObjRotImp *obj1, SpaceObjRotImp *obj2)
{
    Ship *rammingShip = NULL;
    SpaceObjRotImp *otherObj = NULL;

    if (obj1->objtype != OBJ_ShipType || obj2->objtype != OBJ_ShipType)
    {
        return 0;
    }

    if ((((Ship *)obj1)->shiptype == sRammingFrigate || ((Ship *)obj1)->shiptype == bRammingFrigate) &&
        (((Ship *)obj2)->shiptype == sRammingFrigate || ((Ship *)obj2)->shiptype == bRammingFrigate))
    {
        if (rammingFrigateSpecifics((Ship *)obj1)->ramState == 5)
        {
            rammingFrigateSetState((Ship *)obj1, 0.0f);
        }
        if (rammingFrigateSpecifics((Ship *)obj2)->ramState == 5)
        {
            rammingFrigateSetState((Ship *)obj2, 0.0f);
        }
        rammingFrigateInitCounterTimer((Ship *)obj1);
        rammingFrigateInitCounterTimer((Ship *)obj2);
        rammingFrigateSetState((Ship *)obj1, 1.0f);
        return rammingFrigateSetState((Ship *)obj2, 1.0f);
    }

    if ((((Ship *)obj1)->shiptype == sRammingFrigate || ((Ship *)obj1)->shiptype == bRammingFrigate))
    {
        rammingShip = (Ship *)obj1;
        otherObj = obj2;
    }
    else if ((((Ship *)obj2)->shiptype == sRammingFrigate || ((Ship *)obj2)->shiptype == bRammingFrigate))
    {
        rammingShip = (Ship *)obj2;
        otherObj = obj1;
    }

    if (rammingShip == NULL || rammingFrigateSpecifics(rammingShip)->ramState != 3)
    {
        return 0;
    }

    if (otherObj->objtype != OBJ_ShipType ||
        !shipIsLargeDerelict((Ship *)otherObj))
    {
        return rammingFrigateProcessImpact(rammingShip, otherObj);
    }

    if (((Ship *)otherObj)->playerowner == rammingShip->playerowner ||
        ShipHasUpgrade(rammingShip, RESEARCH_UPGRADE_MASK_BEAST_VACCINE) ||
        !shipOrDockedCanHaveBigGun((Ship *)otherObj))
    {
        return rammingFrigateProcessImpact(rammingShip, otherObj);
    }

    shipQueueBeastConversion(rammingShip, (Ship *)otherObj);
    return rammingFrigateSetState(rammingShip, 0.0f);
}

static Effect *rammingFrigateCreateTrailEffect(Ship *ship)
{
    etglod *lod = etgSpecialPurposeEffectTable[EGT_RAM_VENT];
    Effect *trailEffect;
    vector position;
    vector offset;
    vector velocity;
    matrix coordsys;

    if (lod == NULL || lod->nLevels <= 0 || lod->level[0] == NULL || !etgHitEffectsEnabled || etgFrequencyExceeded(lod->level[0]))
    {
        *rammingFrigateEffectPtr(ship) = NULL;
        return NULL;
    }

    offset = rammingFrigateTrailEffectOffset;
    rammingFrigateTransformPointToWorldSpace(&position, &offset, ship);
    velocity = ship->posinfo.velocity;
    coordsys = ship->rotinfo.coordsys;

    trailEffect = etgEffectCreate(lod->level[0], NULL, &position, &velocity, &coordsys, 1.0f, 120, 1);
    *rammingFrigateEffectPtr(ship) = trailEffect;
    return trailEffect;
}

static void rammingFrigateApplyClampDamage(Ship *ship)
{
    Ship *target = (Ship *)rammingFrigateSpecifics(ship)->target;
    Effect *trailEffect = *rammingFrigateEffectPtr(ship);
    vector effectPosition;
    vector offset;
    real32 damage;

    if (target == NULL || *rammingFrigateClampDamageStarted(ship) != 1)
    {
        return;
    }

    if (trailEffect != NULL)
    {
        offset = rammingFrigateTrailEffectOffset;
        rammingFrigateTransformPointToWorldSpace(&effectPosition, &offset, ship);
        trailEffect->posinfo.position = effectPosition;
        trailEffect->rotinfo.coordsys = ship->rotinfo.coordsys;
    }
    else
    {
        rammingFrigateCreateTrailEffect(ship);
    }

    damage = universe.unknown1 * ship->staticinfo->cuttingBeamDamagePerSecond;
    if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_ION))
    {
        damage = universe.unknown1 * ship->staticinfo->cuttingBeamUpgradedDamagePerSecond;
    }
    ApplyDamageToTarget((SpaceObjRotImpTarg *)target, damage, GS_LargeEnergyCannon, 666, ship->playerowner->playerIndex);
}

static sdword rammingFrigateTrackHeading(Ship *ship, vector *desiredHeading, real32 dotThreshold, real32 maxRot)
{
    vector heading;
    vector result;
    real32 dot;
    real32 inversePhysicsTime;
    real32 desiredRot;
    real32 torque;

    if (ship->objtype == OBJ_ShipType && shipDockingTarget(ship) != NULL)
    {
        return TRUE;
    }
    if (ship->staticinfo->shipclass == CLASS_Carrier || ship->staticinfo->shipclass == CLASS_Mothership)
    {
        return FALSE;
    }

    heading.x = ship->rotinfo.coordsys.m13;
    heading.y = ship->rotinfo.coordsys.m23;
    heading.z = ship->rotinfo.coordsys.m33;
    dot = vecDotProduct(heading, *desiredHeading);
    if (dot <= -1.0f)
    {
        physApplyRotToObj((SpaceObjRot *)ship, maxRot, 1u);
        return FALSE;
    }
    if (dot >= dotThreshold)
    {
        aitrackForceZeroRotation(ship);
        return TRUE;
    }

    matMultiplyVecByMat(&result, desiredHeading, &ship->rotinfo.coordsys);
    inversePhysicsTime = 1.0f / universe.unknown1;

    if (result.y != 0.0f)
    {
        desiredRot = -result.y * ship->nonstatvars.turnspeed[TURN_ABOUTZ];
        if (ship->rotinfo.rotspeed.x >= desiredRot)
        {
            torque = (ship->rotinfo.rotspeed.x - desiredRot) * ship->staticinfo->staticheader.momentOfInertiaX * inversePhysicsTime;
            physApplyRotToObj((SpaceObjRot *)ship, (torque <= maxRot) ? torque : maxRot, 1u);
        }
        else
        {
            torque = (desiredRot - ship->rotinfo.rotspeed.x) * ship->staticinfo->staticheader.momentOfInertiaX * inversePhysicsTime;
            physApplyRotToObj((SpaceObjRot *)ship, (torque <= maxRot) ? torque : maxRot, 0u);
        }
    }

    if (result.x != 0.0f)
    {
        desiredRot = result.x * ship->nonstatvars.turnspeed[TURN_ABOUTY];
        if (ship->rotinfo.rotspeed.y >= desiredRot)
        {
            torque = (ship->rotinfo.rotspeed.y - desiredRot) * ship->staticinfo->staticheader.momentOfInertiaY * inversePhysicsTime;
            physApplyRotToObj((SpaceObjRot *)ship, (torque <= maxRot) ? torque : maxRot, 3u);
        }
        else
        {
            torque = (desiredRot - ship->rotinfo.rotspeed.y) * ship->staticinfo->staticheader.momentOfInertiaY * inversePhysicsTime;
            physApplyRotToObj((SpaceObjRot *)ship, (torque <= maxRot) ? torque : maxRot, 2u);
        }
    }

    return FALSE;
}

static void rammingFrigateClampedUpdate(Ship *ship)
{
    Ship *target = (Ship *)rammingFrigateSpecifics(ship)->target;
    real32 scale;

    if (target == NULL)
    {
        return;
    }

    scale = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER) ? rammingFrigateClampDamageOffsetUpgrade : rammingFrigateClampDamageOffset;
    if (!ShipTypeIsMothershipType(target->shiptype) &&
        (target->clampInfo == NULL || target->clampInfo->host == NULL || !ShipTypeIsMothershipType(((Ship *)target->clampInfo->host)->shiptype)))
    {
        // Retail accumulates this into the target's physics force (posinfo.force), letting
        // normal physics integration (mass/deltaTime) scale it down -- not a direct position
        // write. Writing straight to posinfo.position (as an earlier reconstruction pass did)
        // teleports the target across the map in a single frame.
        target->posinfo.force.x += ship->rotinfo.coordsys.m13 * scale;
        target->posinfo.force.y += ship->rotinfo.coordsys.m23 * scale;
        target->posinfo.force.z += ship->rotinfo.coordsys.m33 * scale;
    }

    rammingFrigateTrackHeading(target, rammingFrigateClampTrackVector(ship), rammingFrigateTrackHeadingAccuracy, rammingFrigateTrackHeadingTorque);
    rammingFrigateApplyClampDamage(ship);
}

static void rammingFrigateCheckCollision(Ship *ship, Ship *target)
{
    vector delta;
    real32 collisionRadius;
    real32 distance;
    sdword bboxCollision;

    collisionRadius = ship->staticinfo->staticheader.staticCollInfo.collspheresize +
                      target->staticinfo->staticheader.staticCollInfo.collspheresize;
    vecSub(delta, target->collInfo.collPosition, ship->collInfo.collPosition);
    if (vecMagnitudeSquared(delta) < collisionRadius * collisionRadius)
    {
        distance = GetDistance(&ship->collInfo.collPosition, &target->collInfo.collPosition);
        bboxCollision = collCheckBBoxBumpCollision((SpaceObjRotImp *)ship, (SpaceObjRotImp *)target);
        if (bboxCollision)
        {
            rammingFrigateCollisionCallback((SpaceObjRotImp *)ship, (SpaceObjRotImp *)target);
            if (target->shiptype == sRammingFrigate || target->shiptype == bRammingFrigate)
            {
                univApplyCollisionImpulse(
                    (SpaceObjRotImpTarg *)ship,
                    (SpaceObjRotImpTarg *)target,
                    collisionRadius,
                    &delta,
                    distance);
            }
        }
    }
}

void rammingFrigateAttack(Ship *ship)
{
    RammingFrigateSpecifics *specifics = rammingFrigateSpecifics(ship);
    SpaceObjRotImp *target = (SpaceObjRotImp *)specifics->target;
    vector toTarget;
    vector clampPoint0;
    vector clampPoint2;
    vector clampPoint3;
    vector clampPoint1;
    vector clampForward;
    vector clampSide;
    vector currentHeading;
    vector targetPosition;
    vector shipPosition;
    real32 distance;
    real32 headingDot;
    real32 speedMagnitude;
    ShipStaticInfo *targetStaticInfo;
    bool32 canSteer;

    if (target == NULL)
    {
        return;
    }

    if (ship->specialFlags1C8 & 0x80)
    {
        rammingFrigateSetState(ship, 0.0f);
    }

    if (specifics->clampDamageStarted == 1)
    {
        if (TimerExpired((int)rammingFrigateImpactEffectTimer(ship)))
        {
            specifics->clampDamageStarted = 0;
        }
        else if (TimerElapsed(rammingFrigateImpactEffectTimer(ship)) > rammingFrigateClampImpactEffectDelay)
        {
            rammingFrigateGetClampPoints(ship, &clampPoint0, &clampPoint1, &clampPoint2, &clampPoint3, &clampForward, &clampSide);
            rammingFrigateCreateImpactEffect(ship, &clampPoint0, &clampForward);
            rammingFrigateCreateImpactEffect(ship, &clampPoint1, &clampForward);
            rammingFrigateCreateImpactEffect(ship, &clampPoint2, &clampSide);
            rammingFrigateCreateImpactEffect(ship, &clampPoint3, &clampSide);
        }
    }

    targetPosition = target->posinfo.position;
    shipPosition = ship->posinfo.position;
    vecSub(toTarget, targetPosition, shipPosition);
    distance = GetDistance(&shipPosition, &targetPosition);
    vecNormalize(&toTarget);
    currentHeading.x = ship->rotinfo.coordsys.m13;
    currentHeading.y = ship->rotinfo.coordsys.m23;
    currentHeading.z = ship->rotinfo.coordsys.m33;
    canSteer = singlePlayerGame || (!ship->velocityFreezeActive && (ship->command == NULL || ship->command->ordertype.order != 10));

    switch (specifics->ramState)
    {
        case 1:
            headingDot = vecDotProduct(toTarget, currentHeading);
            if (headingDot > rammingFrigateHeadingDotToCharge && TimerExpired((int)rammingFrigateCounterTimer(ship)))
            {
                rammingFrigateSetState(ship, 2.0f);
            }
            else if (canSteer)
            {
                aitrackHeadingFunc(
                    (SpaceObjRotImpTargGuidance *)ship,
                    &toTarget,
                    0.99f,
                    0.99f,
                    ship->staticinfo->sinbank,
                    0);
            }
            break;

        case 2:
            targetStaticInfo = ((Ship *)target)->staticinfo;
            if ((distance < targetStaticInfo->staticheader.staticCollInfo.collspheresize + rammingFrigateTransitionDistanceCapital ||
                 ((!target->flags || (targetStaticInfo->shipclass != CLASS_Fighter && targetStaticInfo->shipclass != CLASS_Corvette)) &&
                  distance < targetStaticInfo->staticheader.staticCollInfo.collspheresize + rammingFrigateTransitionDistanceAny)) &&
                TimerExpired((int)rammingFrigateLineOfSightTimer(ship)))
            {
                if (collCheckLineOfSight(ship, (Ship *)target, &shipPosition, &toTarget))
                {
                    specifics->unk_10 = 0;
                    rammingFrigateSetState(ship, 3.0f);
                    madAnimationStart(ship, 2);
                }
                else
                {
                    TimerInitTimerStruct(rammingFrigateLineOfSightTimer(ship), rammingFrigateLineOfSightRetry);
                }
            }

            ship->flightmanState1 = 0;
            if (canSteer)
            {
                aishipFlyToPointAvoidingObjsFunc(ship, &targetPosition, 6u, -30.0f, &target->posinfo.velocity);
                aitrackHeadingFunc(
                    (SpaceObjRotImpTargGuidance *)ship,
                    &toTarget,
                    0.99f,
                    0.99f,
                    ship->staticinfo->sinbank,
                    0);
            }
            break;

        case 3:
        {
            vector desiredVelocity;
            vector guidanceVelocity;
            vector currentVelocity;
            vector desiredDirection;
            real32 velocityScale = ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER) ? rammingFrigateVelocityScaleUpgrade : rammingFrigateVelocityScale;

            ship->specialFlags2 |= 0x40;
            ship->aistateattack = 0;
            ship->flightmanState1 = 0;

            guidanceVelocity.x = targetPosition.x + toTarget.x * rammingFrigateLeadDistance;
            guidanceVelocity.y = targetPosition.y + toTarget.y * rammingFrigateLeadDistance;
            guidanceVelocity.z = targetPosition.z + toTarget.z * rammingFrigateLeadDistance;
            desiredVelocity = toTarget;
            speedMagnitude = fsqrt(vecMagnitudeSquared(ship->posinfo.velocity));
            desiredVelocity.x *= speedMagnitude * velocityScale;
            desiredVelocity.y *= speedMagnitude * velocityScale;
            desiredVelocity.z *= speedMagnitude * velocityScale;

            currentVelocity = ship->posinfo.velocity;
            vecNormalize(&currentVelocity);
            desiredDirection = desiredVelocity;
            vecNormalize(&desiredDirection);
            {
                real32 targetHeadingDot = vecDotProduct(toTarget, currentHeading);
                real32 velocityHeadingDot = vecDotProduct(desiredDirection, currentVelocity);

                if (targetHeadingDot < rammingFrigateHeadingDotToMaintain &&
                    velocityHeadingDot < rammingFrigateHeadingDotToMaintain)
                {
                    rammingFrigateSetState(ship, 1.0f);
                }
                else
                {
                    if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER) && speedMagnitude > rammingFrigateVelocityDampenCap)
                    {
                        real32 dampScale = GetDistance(&currentVelocity, &desiredDirection);

                        if (dampScale > rammingFrigateVelocityDampenCap)
                        {
                            dampScale = rammingFrigateVelocityDampenCap;
                        }
                        dampScale = 1.0f - dampScale;
                        desiredVelocity.x *= dampScale;
                        desiredVelocity.y *= dampScale;
                        desiredVelocity.z *= dampScale;
                    }

                    ship->posinfo.velocity = desiredVelocity;
                    if (canSteer)
                    {
                        rammingFrigateBlendHeadingWeighted(
                            (SpaceObjRotImp *)ship,
                            &toTarget,
                            rammingFrigateHeadingBlendWeight,
                            rammingFrigateHeadingCurrentWeight);
                    }
                }
            }

            if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER) && TimerExpired((int)rammingFrigateHeadingResetTimer(ship)))
            {
                rammingFrigateSetState(ship, 1.0f);
                TimerInitTimerStruct(rammingFrigateCounterTimer(ship), ship->staticinfo->RamWaitAfterPushTime);
            }

            if (canSteer)
            {
                aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &toTarget, 0.0f, 0.0f, 0.0f, 0);
                aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)ship, &guidanceVelocity);
            }

            targetStaticInfo = ((Ship *)target)->staticinfo;
            if (targetStaticInfo->shipclass == CLASS_Corvette ||
                targetStaticInfo->shipclass == CLASS_Fighter)
            {
                if (distance > targetStaticInfo->staticheader.staticCollInfo.collspheresize + rammingFrigateAbortDistanceFighter)
                {
                    rammingFrigateSetState(ship, 1.0f);
                }
            }
            break;
        }

        case 5:
            if (TimerExpired((int)rammingFrigateClampTimer(ship)) || !univIsPositionInBounds(&ship->posinfo.position))
            {
                rammingFrigateSetState(ship, 0.0f);
            }
            else
            {
                rammingFrigateClampedUpdate(ship);
            }
            break;

        default:
            break;
    }

    if (specifics->ramState != 5)
    {
        rammingFrigateCheckCollision(ship, (Ship *)target);
    }

    if (!fogObjectVisibleToPlayer(ship->playerowner, (SpaceObj *)target, 1))
    {
        SelectCommandMax6 selection;

        selection.numShips = 1;
        selection.timeLastStatus = 0.0f;
        selection.ShipPtr[0] = ship;
        clWrapSpecial(&universe.mainCommandLayer, (SelectCommand *)&selection, NULL);
    }
}
