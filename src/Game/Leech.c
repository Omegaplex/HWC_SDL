#include "types.h"
#include "spaceobj.h"
#include "Leech.h"

#include "AIShip.h"
#include "AITrack.h"
#include "Clamp.h"
#include "ClassDefs.h"
#include "CommandDefs.h"
#include "CommandWrap.h"
#include "dock.h"
#include "etg.h"
#include "FastMath.h"
#include "Memory.h"
#include "meshanim.h"
#include "objtypes.h"
#include "Randy.h"
#include "SaveGame.h"
#include "select.h"
#include "SoundEvent.h"
#include "Strings.h"
#include "StringsOnly.h"
#include "UnivUpdate.h"
#include "universe.h"

#include <string.h>

#define LEECH_MODE_RESOURCE             7
#define LEECH_MODE_DAMAGE               8

#define LEECH_STATE_IDLE                0
#define LEECH_STATE_SELECT_TARGET       1
#define LEECH_STATE_APPROACH            2
#define LEECH_STATE_CLAMP               3
#define LEECH_STATE_RESOURCE            7
#define LEECH_STATE_DAMAGE              8
#define LEECH_STATE_RETURN_EMPTY        9
#define LEECH_STATE_RETURN_FULL         10
#define LEECH_STATE_SHUTDOWN            11
#define LEECH_STATE_SHUTDOWN_2          12
#define LEECH_STATE_RETURN_DEPLETED     13
#define LEECH_STATE_DONE                14

#define LEECH_CLAIM_FLAG_MASK           0x40u
#define LEECH_ATTACHED_FLAG_MASK        0x80000000u
#define LEECH_MESSAGE_DELAY             0.5f
#define LEECH_MESSAGE_COOLDOWN          2.0f
#define LEECH_MODE_EFFECT_RETRY_DELAY   3.0f
#define LEECH_MODE_EFFECT_RANDOM_SCALE  4.656612873077393e-10f
#define LEECH_MODE_EFFECT_FLAGS         24
#define LEECH_MODE_EFFECT_PARAMS        2
#define LEECH_ATTACH_OFFSET_SCALE       6.0f
#define LEECH_ATTACH_TRACK_ACCURACY     0.99989998f
#define LEECH_APPROACH_SLOWDOWN         -100.0f
#define LEECH_CLAMP_SLOWDOWN            -35.0f
#define LEECH_OPEN_ANIM_DISTANCE        50.0f
#define LEECH_CLAMP_COMPLETE_DISTANCE   2.0f
#define LEECH_FLYAWAY_COMPLETE_SCALE    0.20000000298023224f
#define LEECH_DRAIN_INTERVAL            5.0f
#define LEECH_DAMAGE_HOW                666
#define LEECH_SPECIAL_UPGRADE_MASK      RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY
#define LEECH_AUTO_ATTACK_RANGE         7000.0f
#define LEECH_RESOURCE_DOCK_TYPE        DOCK_MODE_DEPOSIT_RESOURCES
#define LEECH_RESOURCE_START_SPEECH     33556658
#define LEECH_DAMAGE_START_SPEECH       33556659
#define LEECH_AUTO_ATTACK_MASK          15
#define LEECH_AUTO_ATTACK_FRAME         14
#define LEECH_MODE_SPEECH_STEALTH       67111089
#define LEECH_MODE_SPEECH_RESOURCE      67111087
#define LEECH_MODE_SPEECH_ATTACK        67111088

typedef struct
{
    udword type;
    sdword dockIndex;
    vector position;
    vector approachVector;
    real32 approachDot;
    real32 approachOffset;
    real32 clampMinDistance;
    real32 clampMaxDistance;
    udword headingDirection;
    udword upDirection;
    char name[LIGHTNAME_LENGTH];
} LeechDockPoint;

static_assert(sizeof(LeechDockPoint) == 76);

static udword leechStaticBlob[1];
static real32 leechFeedbackSuppressUntil = 0.0f;
static real32 leechFeedbackDisplayUntil = 0.0f;
static ubyte leechFeedbackHadTarget = 0;
static ubyte leechFeedbackNoTarget = 0;

void leechResetFeedbackState(void)
{
    leechFeedbackSuppressUntil = 0.0f;
    leechFeedbackDisplayUntil = 0.0f;
    leechFeedbackHadTarget = 0;
    leechFeedbackNoTarget = 0;
}

real32 ClampToWithin(real32 x, real32 low, real32 high);
void GetDirectionVectorOfShip(vector *dest, udword direction, matrix *coordsys);

static LeechSpecifics *leechSpecifics(Ship *ship)
{
    return (LeechSpecifics *)ship->ShipSpecifics;
}

static const LeechSpecifics *leechSpecificsConst(const Ship *ship)
{
    return (const LeechSpecifics *)ship->ShipSpecifics;
}

static LeechDockPoint *leechDockPoint(Ship *target, sdword index)
{
    DockStaticInfo *dockStaticInfo;

    dbgAssert(target != NULL);
    dockStaticInfo = target->staticinfo->dockStaticInfo;
    dbgAssert(dockStaticInfo != NULL);
    if (index < 0 || index >= dockStaticInfo->numDockPoints)
    {
        return NULL;
    }

    return (LeechDockPoint *)&dockStaticInfo->dockstaticpoints[index];
}

static void leechReleaseClaimedHardpoint(LeechSpecifics *specifics)
{
    if (specifics->claimedHardpointIndex != -1)
    {
        if (specifics->currentTarget != NULL && specifics->currentTarget->dockvars.dockPointActivity != NULL)
        {
            specifics->currentTarget->dockvars.dockPointActivity->reserved[specifics->claimedHardpointIndex] = 0;
        }
        specifics->claimedHardpointIndex = -1;
    }
}

static bool32 leechTrySetModeFromRequested(LeechSpecifics *specifics)
{
    switch (specifics->requestedMode)
    {
        case 6:
        case 7:
        case 8:
            specifics->activeMode = specifics->requestedMode;
            specifics->preserveDockingState = 0;
            return TRUE;
        default:
            return FALSE;
    }
}

static void leechCreateModeEffect(Ship *ship, ubyte mode)
{
    LeechSpecifics *specifics = leechSpecifics(ship);
    etglod *effectLOD;
    etgeffectstatic *effectStatic;
    sdword lodIndex;

    if (mode != LEECH_MODE_DAMAGE)
    {
        return;
    }

    effectLOD = etgSpecialPurposeEffectTable[EGT_LEECH_VENT_EFFECT];
    if (effectLOD == NULL)
    {
        return;
    }

    lodIndex = ship->currentLOD;
    if (specifics->currentTarget != NULL && lodIndex > specifics->currentTarget->currentLOD)
    {
        lodIndex = specifics->currentTarget->currentLOD;
    }

    effectStatic = (lodIndex < effectLOD->nLevels) ? effectLOD->level[lodIndex] : NULL;
    if (effectStatic != NULL && !etgFrequencyExceeded(effectStatic))
    {
        if (univSpaceObjInRenderList((SpaceObj *)ship))
        {
            etgEffectCreate(effectStatic, ship, NULL, NULL, NULL, 0.0f, LEECH_MODE_EFFECT_FLAGS, LEECH_MODE_EFFECT_PARAMS);
        }
        specifics->nextModeEffectTime = universe.totaltimeelapsed +
            LEECH_MODE_EFFECT_RETRY_DELAY +
            (real32)ranRandom(RAN_ParticleStream) * LEECH_MODE_EFFECT_RANDOM_SCALE;
    }
}

static MaxSelection *leechBuildTargetList(SpecialCommand *targets)
{
    MaxSelection *selection;
    sdword i;
    sdword count;
    bool32 usedDockParent = FALSE;

    if (targets == NULL)
    {
        return NULL;
    }

    selection = memAlloc(sizeof(MaxSelection), "LeechTargets", NonVolatile);
    selection->numShips = 0;
    selection->timeLastStatus = 0.0f;

    count = targets->numTargets;
    for (i = 0; i < count; i++)
    {
        Ship *target = (Ship *)targets->TargetPtr[i];

        switch (target->shiptype)
        {
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
            case 0x1F:
            case 0x20:
            case 0x36:
            case 0x37:
            case 0x38:
            case 0x39:
                if (!usedDockParent && target->dockvars.dockship != NULL)
                {
                    selection->ShipPtr[selection->numShips++] = target->dockvars.dockship;
                    usedDockParent = TRUE;
                }
                break;

            default:
                switch (target->staticinfo->shipclass)
                {
                    case CLASS_Corvette:
                    case CLASS_Fighter:
                    case CLASS_Component:
                    case CLASS_Micro:
                        break;

                    default:
                        selection->ShipPtr[selection->numShips++] = target;
                        break;
                }
                break;
        }
    }

    return selection;
}

static bool32 leechChooseTarget(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);
    MaxSelection *targetList = specifics->targetList;
    sdword *candidateIndices;
    sdword candidateCount;
    sdword i;

    specifics->currentTarget = NULL;
    if (targetList == NULL || targetList->numShips <= 0)
    {
        goto done;
    }

    candidateIndices = memAlloc(sizeof(sdword) * targetList->numShips, "LeechChoice", NonVolatile);
    candidateCount = 0;

    for (i = 0; i < targetList->numShips; i++)
    {
        Ship *candidate = targetList->ShipPtr[i];
        DockStaticInfo *dockStaticInfo;
        sdword dockIndex;

        if (specifics->activeMode == LEECH_MODE_RESOURCE &&
            (4.0f * candidate->staticinfo->leechRUsPerSecond) >= candidate->health)
        {
            continue;
        }
        if (candidate->clampInfo != NULL && candidate->clampInfo->host != NULL)
        {
            continue;
        }
        dockStaticInfo = candidate->staticinfo->dockStaticInfo;
        if (candidate->dockvars.dockPointActivity == NULL)
        {
            continue;
        }
        dbgAssert(dockStaticInfo != NULL);
        for (dockIndex = 0; dockIndex < dockStaticInfo->numDockPoints; dockIndex++)
        {
            LeechDockPoint *dockPoint = (LeechDockPoint *)&dockStaticInfo->dockstaticpoints[dockIndex];
            if (dockPoint->type == 12 && candidate->dockvars.dockPointActivity->reserved[dockIndex] == 0)
            {
                candidateIndices[candidateCount++] = i;
                break;
            }
        }
    }

    if (candidateCount > 0)
    {
        specifics->currentTarget = targetList->ShipPtr[candidateIndices[ranRandom(RAN_Game) % candidateCount]];
    }

    memFree(candidateIndices);

done:
    if (ship->playerowner == universe.curPlayerPtr &&
        leechFeedbackSuppressUntil < universeRealtimeElapsed)
    {
        if (specifics->currentTarget != NULL)
        {
            leechFeedbackHadTarget = 1;
        }
        else
        {
            leechFeedbackNoTarget = 1;
        }
        if (leechFeedbackDisplayUntil == 0.0f)
        {
            leechFeedbackDisplayUntil = universeRealtimeElapsed + LEECH_MESSAGE_DELAY;
        }
    }

    return (specifics->currentTarget != NULL);
}

static bool32 leechClaimTargetHardpoint(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);
    Ship *target = specifics->currentTarget;
    DockStaticInfo *dockStaticInfo;
    sdword *candidateIndices;
    sdword candidateCount;
    sdword i;

    if (target == NULL || target->dockvars.dockPointActivity == NULL)
    {
        return FALSE;
    }

    dockStaticInfo = target->staticinfo->dockStaticInfo;
    dbgAssert(dockStaticInfo != NULL);
    if (dockStaticInfo->numDockPoints <= 0)
    {
        return FALSE;
    }

    candidateIndices = memAlloc(sizeof(sdword) * dockStaticInfo->numDockPoints, "LeechClamp", NonVolatile);
    candidateCount = 0;

    for (i = 0; i < dockStaticInfo->numDockPoints; i++)
    {
        LeechDockPoint *dockPoint = (LeechDockPoint *)&dockStaticInfo->dockstaticpoints[i];

        if (dockPoint->type == 12 && target->dockvars.dockPointActivity->reserved[i] == 0)
        {
            candidateIndices[candidateCount++] = i;
        }
    }

    if (candidateCount > 0)
    {
        specifics->claimedHardpointIndex = candidateIndices[ranRandom(RAN_Game) % candidateCount];
        target->dockvars.dockPointActivity->reserved[specifics->claimedHardpointIndex] = 1;
    }

    memFree(candidateIndices);
    return (candidateCount > 0);
}

static void leechAbortCurrentAction(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    specifics->state = LEECH_STATE_IDLE;

    switch (specifics->activeMode)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            if (specifics->activeMode == 7)
            {
                speechEventQueue(ship, LEECH_RESOURCE_START_SPEECH, 0, -1, -1, -1, -1, 0.0f, -1);
            }
            else if (specifics->activeMode == 8)
            {
                speechEventQueue(ship, LEECH_DAMAGE_START_SPEECH, 0, -1, -1, -1, -1, 0.0f, -1);
            }

            leechReleaseClaimedHardpoint(specifics);
            ship->specialFlags2 &= ~LEECH_CLAIM_FLAG_MASK;
            ship->flags &= ~LEECH_ATTACHED_FLAG_MASK;

            if (leechChooseTarget(ship))
            {
                specifics->state = LEECH_STATE_SELECT_TARGET;
            }
            else if (specifics->activeMode == 7)
            {
                if (ship->resources != 0)
                {
                    specifics->dockRequested = 1;
                    specifics->preserveDockingState = 0;
                }
            }
            else
            {
                specifics->state = LEECH_STATE_SHUTDOWN;
            }

            if (specifics->animationState != 0)
            {
                madAnimationStart(ship, 1);
                specifics->animationState = 0;
            }
            clDetachShipFromCommand(ship);
            break;

        case LEECH_STATE_DONE:
            specifics->state = LEECH_STATE_IDLE;
            break;

        default:
            break;
    }
}

static void leechEnsureAttachedFlags(Ship *ship, LeechSpecifics *specifics)
{
    if (specifics->state == LEECH_STATE_CLAMP ||
        specifics->state == 6 ||
        specifics->state == LEECH_STATE_RESOURCE ||
        specifics->state == LEECH_STATE_DAMAGE)
    {
        ship->specialFlags2 |= LEECH_CLAIM_FLAG_MASK;
        ship->flags |= LEECH_ATTACHED_FLAG_MASK;
    }
}

static bool32 leechUpdateSpecial(Ship *ship, SpecialCommand *targets, ubyte action)
{
    ShipStaticInfo *shipStaticInfo = ship->staticinfo;
    LeechSpecifics *specifics = leechSpecifics(ship);
    Ship *target = specifics->currentTarget;

    if (action != 0)
    {
        if (action == 2 && specifics->state != LEECH_STATE_IDLE)
        {
            specifics->state = LEECH_STATE_SHUTDOWN;
        }
    }
    else
    {
        specifics->state = LEECH_STATE_SELECT_TARGET;
        if (specifics->targetList != NULL)
        {
            memFree(specifics->targetList);
            specifics->targetList = NULL;
        }
        specifics->targetList = leechBuildTargetList(targets);
    }

    if (target != NULL || specifics->state == LEECH_STATE_SELECT_TARGET || specifics->state == LEECH_STATE_SHUTDOWN)
    {
        if (target != NULL)
        {
            leechEnsureAttachedFlags(ship, specifics);
        }

        switch (specifics->state)
        {
            case LEECH_STATE_SELECT_TARGET:
            {
                vector hardpointPosition;
                vector fromHardpoint;
                vector worldApproach;
                LeechDockPoint *dockPoint;
                real32 distance;

                leechReleaseClaimedHardpoint(specifics);

                if (!leechChooseTarget(ship) || !leechClaimTargetHardpoint(ship))
                {
                    specifics->state = LEECH_STATE_SHUTDOWN;
                    break;
                }

                target = specifics->currentTarget;
                dockPoint = leechDockPoint(target, specifics->claimedHardpointIndex);
                matMultiplyMatByVec(&hardpointPosition, &target->rotinfo.coordsys, &dockPoint->position);
                vecAddTo(hardpointPosition, target->posinfo.position);

                vecSub(fromHardpoint, ship->posinfo.position, hardpointPosition);
                distance = (real32)fmathSqrtDouble(vecMagnitudeSquared(fromHardpoint));

                matMultiplyMatByVec(&worldApproach, &target->rotinfo.coordsys, &dockPoint->approachVector);
                if (distance * dockPoint->approachDot <= vecDotProduct(worldApproach, fromHardpoint))
                {
                    specifics->state = LEECH_STATE_CLAMP;
                    ship->specialFlags2 |= LEECH_CLAIM_FLAG_MASK;
                    ship->flags |= LEECH_ATTACHED_FLAG_MASK;
                }
                else
                {
                    specifics->clampDistance = ClampToWithin(distance, dockPoint->clampMinDistance, dockPoint->clampMaxDistance);
                    specifics->state = LEECH_STATE_APPROACH;
                }
                break;
            }

            case LEECH_STATE_APPROACH:
            {
                vector hardpointPosition;
                vector worldApproach;
                vector destination;
                LeechDockPoint *dockPoint = leechDockPoint(target, specifics->claimedHardpointIndex);

                matMultiplyMatByVec(&hardpointPosition, &target->rotinfo.coordsys, &dockPoint->position);
                vecAddTo(hardpointPosition, target->posinfo.position);
                matMultiplyMatByVec(&worldApproach, &target->rotinfo.coordsys, &dockPoint->approachVector);

                vecScalarMultiply(destination, worldApproach, specifics->clampDistance);
                vecAddTo(destination, hardpointPosition);

                ship->shipidle = FALSE;
                aishipFlyToPointAvoidingObjsFunc(ship, &destination, 7u, LEECH_APPROACH_SLOWDOWN, &target->posinfo.velocity);

                if (specifics->animationState != 1 &&
                    MoveReachedDestinationVariable(ship, &destination, LEECH_OPEN_ANIM_DISTANCE))
                {
                    specifics->animationState = 1;
                    madAnimationStart(ship, 0);
                }

                if (MoveReachedDestinationVariable(ship, &destination, dockPoint->clampMinDistance))
                {
                    specifics->state = LEECH_STATE_CLAMP;
                    ship->specialFlags2 |= LEECH_CLAIM_FLAG_MASK;
                    ship->flags |= LEECH_ATTACHED_FLAG_MASK;
                }
                break;
            }

            case LEECH_STATE_CLAMP:
            {
                vector heading;
                vector desiredHeading;
                vector desiredUp;
                vector hardpointPosition;
                vector worldApproach;
                vector destination;
                matrix targetSpace;
                LeechDockPoint *dockPoint = leechDockPoint(target, specifics->claimedHardpointIndex);

                matMultiplyMatByVec(&heading, &target->rotinfo.coordsys, &dockPoint->approachVector);
                matCreateCoordSysFromHeading(&targetSpace, &heading);
                GetDirectionVectorOfShip(&desiredHeading, dockPoint->headingDirection, &targetSpace);
                GetDirectionVectorOfShip(&desiredUp, dockPoint->upDirection, &targetSpace);

                ship->shipidle = FALSE;
                aitrackHeadingAndUp(ship, &desiredHeading, &desiredUp, LEECH_ATTACH_TRACK_ACCURACY);

                matMultiplyMatByVec(&hardpointPosition, &target->rotinfo.coordsys, &dockPoint->position);
                vecAddTo(hardpointPosition, target->posinfo.position);
                matMultiplyMatByVec(&worldApproach, &target->rotinfo.coordsys, &dockPoint->approachVector);
                vecScalarMultiply(destination, worldApproach, LEECH_ATTACH_OFFSET_SCALE);
                vecAddTo(destination, hardpointPosition);

                aishipFlyToPointAvoidingObjsFunc(ship, &destination, 4u, LEECH_CLAMP_SLOWDOWN, &target->posinfo.velocity);

                if (specifics->animationState != 1 &&
                    MoveReachedDestinationVariable(ship, &destination, LEECH_OPEN_ANIM_DISTANCE))
                {
                    specifics->animationState = 1;
                    madAnimationStart(ship, 0);
                }

                if (!MoveReachedDestinationVariable(ship, &destination, LEECH_CLAMP_COMPLETE_DISTANCE))
                {
                    break;
                }

                if (!leechTrySetModeFromRequested(specifics))
                {
                    specifics->state = specifics->activeMode;
                }
                else
                {
                    specifics->state = specifics->activeMode;
                }

                specifics->preserveDockingState = 0;
                specifics->reclampRequested = 1;
                leechCreateModeEffect(ship, 5);
                leechCreateModeEffect(ship, specifics->activeMode);
                break;
            }

            case 6:
            {
                ubyte previousMode = specifics->activeMode;

                leechTrySetModeFromRequested(specifics);
                if (specifics->activeMode != previousMode)
                {
                    leechCreateModeEffect(ship, specifics->activeMode);
                    specifics->state = specifics->activeMode;
                }
                break;
            }

            case LEECH_STATE_RESOURCE:
            {
                if (universe.phystimeelapsed >= specifics->nextDrainTime)
                {
                    real32 drainAmount = target->staticinfo->leechHPsPerSecond;

                    if (drainAmount + drainAmount < target->health)
                    {
                        ship->resources += (sdword)target->staticinfo->leechRUsPerSecond;
                        if (ship->resources > shipStaticInfo->maxresources)
                        {
                            ship->resources = shipStaticInfo->maxresources;
                        }
                    }

                    if (ship->resources >= shipStaticInfo->maxresources)
                    {
                        specifics->state = LEECH_STATE_RETURN_FULL;
                        specifics->savedTarget = target;
                        specifics->feedbackTimer = universe.totaltimeelapsed + LEECH_DRAIN_INTERVAL;
                    }

                    if (drainAmount + drainAmount >= target->health)
                    {
                        if (specifics->state == LEECH_STATE_RESOURCE)
                        {
                            specifics->state = LEECH_STATE_RETURN_DEPLETED;
                            specifics->savedTarget = target;
                            specifics->feedbackTimer = universe.totaltimeelapsed + LEECH_DRAIN_INTERVAL;
                        }
                    }
                    else if (ApplyDamageToTarget((SpaceObjRotImpTarg *)target,
                                                 drainAmount,
                                                 0,
                                                 LEECH_DAMAGE_HOW,
                                                 ship->playerowner->playerIndex))
                    {
                        specifics->state = LEECH_STATE_RETURN_FULL;
                        specifics->savedTarget = target;
                        specifics->feedbackTimer = universe.totaltimeelapsed + LEECH_DRAIN_INTERVAL;
                    }

                    specifics->nextDrainTime = universe.phystimeelapsed + 1.0f;
                }
                break;
            }

            case LEECH_STATE_DAMAGE:
            {
                ubyte previousMode = specifics->activeMode;

                if (universe.phystimeelapsed >= specifics->nextDrainTime)
                {
                    if (ApplyDamageToTarget((SpaceObjRotImpTarg *)target,
                                             target->staticinfo->leechHPsPerSecond,
                                             0,
                                             LEECH_DAMAGE_HOW,
                                             ship->playerowner->playerIndex))
                    {
                        specifics->state = LEECH_STATE_RETURN_FULL;
                        specifics->savedTarget = target;
                        specifics->feedbackTimer = universe.totaltimeelapsed + LEECH_DRAIN_INTERVAL;
                    }

                    leechTrySetModeFromRequested(specifics);
                    if (specifics->activeMode != previousMode)
                    {
                        leechCreateModeEffect(ship, specifics->activeMode);
                        specifics->state = specifics->activeMode;
                    }
                    specifics->nextDrainTime = universe.phystimeelapsed + 1.0f;
                }
                break;
            }

            case LEECH_STATE_RETURN_EMPTY:
            case LEECH_STATE_RETURN_FULL:
            case LEECH_STATE_RETURN_DEPLETED:
            {
                vector hardpointPosition;
                vector worldApproach;
                vector destination;
                vector desiredUp;
                vector desiredHeading;
                real32 completionBounds;
                LeechDockPoint *dockPoint = leechDockPoint(target, specifics->claimedHardpointIndex);

                if (target != NULL)
                {
                    clDetachShipFromCommand(ship);

                    matMultiplyMatByVec(&hardpointPosition, &target->rotinfo.coordsys, &dockPoint->position);
                    vecAddTo(hardpointPosition, target->posinfo.position);
                    matMultiplyMatByVec(&worldApproach, &target->rotinfo.coordsys, &dockPoint->approachVector);

                    vecScalarMultiply(destination, worldApproach, dockPoint->clampMinDistance);
                    vecAddTo(destination, hardpointPosition);

                    GetDirectionVectorOfShip(&desiredUp, dockPoint->headingDirection, &target->rotinfo.coordsys);
                    GetDirectionVectorOfShip(&desiredHeading, dockPoint->upDirection, &target->rotinfo.coordsys);
                    aitrackHeadingAndUp(ship, &desiredUp, &desiredHeading, LEECH_ATTACH_TRACK_ACCURACY);

                    ship->shipidle = FALSE;
                    aishipFlyToPointAvoidingObjsFunc(ship, &destination, 0, dockPoint->clampMaxDistance, &target->posinfo.velocity);
                }

                if (specifics->animationState != 0 &&
                    (target == NULL ||
                     MoveReachedDestinationVariable(ship, &target->posinfo.position, dockPoint->clampMinDistance)))
                {
                    specifics->animationState = 0;
                    madAnimationStart(ship, 1);
                }

                completionBounds = dockPoint->clampMinDistance * LEECH_FLYAWAY_COMPLETE_SCALE;
                if (target != NULL && !MoveReachedDestinationVariable(ship, &destination, completionBounds))
                {
                    break;
                }

                switch (specifics->state)
                {
                    case LEECH_STATE_RETURN_EMPTY:
                        leechReleaseClaimedHardpoint(specifics);
                        specifics->state = LEECH_STATE_SHUTDOWN;
                        break;

                    case LEECH_STATE_RETURN_FULL:
                        specifics->dockRequested = 1;
                        specifics->preserveDockingState = 1;
                        specifics->state = LEECH_STATE_SHUTDOWN;
                        leechReleaseClaimedHardpoint(specifics);
                        break;

                    case LEECH_STATE_RETURN_DEPLETED:
                        leechReleaseClaimedHardpoint(specifics);
                        if (specifics->targetList != NULL)
                        {
                            selSelectionRemoveSingleShip(specifics->targetList, target);
                        }
                        if (leechChooseTarget(ship))
                        {
                            specifics->state = LEECH_STATE_SELECT_TARGET;
                        }
                        else
                        {
                            if (ship->resources != 0)
                            {
                                specifics->dockRequested = 1;
                                specifics->preserveDockingState = 1;
                            }
                            specifics->state = LEECH_STATE_SHUTDOWN;
                        }
                        break;
                }
                break;
            }

            case LEECH_STATE_SHUTDOWN:
            case LEECH_STATE_SHUTDOWN_2:
                specifics->savedTarget = target;
                specifics->feedbackTimer = universe.totaltimeelapsed + LEECH_DRAIN_INTERVAL;

                if (specifics->animationState != 0)
                {
                    specifics->animationState = 0;
                    madAnimationStart(ship, 1);
                }

                leechReleaseClaimedHardpoint(specifics);
                ship->specialFlags2 &= ~LEECH_CLAIM_FLAG_MASK;
                ship->flags &= ~LEECH_ATTACHED_FLAG_MASK;
                clDetachShipFromCommand(ship);

                if (specifics->targetList != NULL)
                {
                    memFree(specifics->targetList);
                    specifics->targetList = NULL;
                }
                specifics->state = LEECH_STATE_IDLE;
                break;

            default:
                break;
        }
    }
    else
    {
        leechAbortCurrentAction(ship);
    }
    return (specifics->state == LEECH_STATE_IDLE);
}

static void leechAttackNearestTarget(Ship *ship)
{
    SelectAnyCommand *targets;
    MaxAnySelection specialCommand;
    SelectCommand selection;

    if (ship->attackvars.attacktarget != NULL)
    {
        return;
    }

    targets = univFindNearbyTargets(ship, LEECH_AUTO_ATTACK_RANGE);
    if (targets == NULL || targets->numTargets <= 0)
    {
        return;
    }

    specialCommand.numTargets = targets->numTargets;
    specialCommand.timeLastStatus = targets->timeLastStatus;
    memcpy(specialCommand.TargetPtr, targets->TargetPtr, sizeof(TargetPtr) * targets->numTargets);

    selection.numShips = 1;
    selection.timeLastStatus = 0.0f;
    selection.ShipPtr[0] = ship;
    clSpecial(&universe.mainCommandLayer, &selection, (SpecialCommand *)&specialCommand);
}

void leechStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    (void)directory;
    (void)filename;

    statinfo->custstatinfo = leechStaticBlob;
    statinfo->rotateToRetaliate = TRUE;
    statinfo->canReceiveTheseShips[0] = TRUE;
    statinfo->canReceiveTheseShips[1] = FALSE;
}

void leechInit(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    memset(specifics, 0, sizeof(*specifics));
    specifics->activeMode = LEECH_MODE_DAMAGE;
    specifics->requestedMode = LEECH_MODE_DAMAGE;
    specifics->claimedHardpointIndex = -1;
}

void leechFreeShipContents(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    if (specifics->targetList != NULL)
    {
        memFree(specifics->targetList);
        specifics->targetList = NULL;
    }
}

bool32 leechSpecialActivate(Ship *ship, void *custom)
{
    SpecialCommand *targets = custom;
    ubyte action = 1;

    if (targets == NULL || targets->numTargets <= 0)
    {
        return TRUE;
    }
    if (!ShipHasUpgrade(ship, LEECH_SPECIAL_UPGRADE_MASK))
    {
        return TRUE;
    }
    if (leechSpecifics(ship)->state == LEECH_STATE_IDLE || leechSpecifics(ship)->state == LEECH_STATE_DONE)
    {
        action = 0;
    }

    return leechUpdateSpecial(ship, targets, action);
}

void leechSpecialCleanup(void *context)
{
    leechUpdateSpecial((Ship *)context, NULL, 2);
}

void leechHousekeep(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);
    SelectCommand selection;

    if (specifics->feedbackTimer != 0.0f && universe.totaltimeelapsed > specifics->feedbackTimer)
    {
        specifics->feedbackTimer = 0.0f;
        specifics->savedTarget = NULL;
    }

    if (ship->playerowner == universe.curPlayerPtr &&
        leechFeedbackDisplayUntil != 0.0f &&
        leechFeedbackDisplayUntil < universeRealtimeElapsed)
    {
        leechFeedbackDisplayUntil = 0.0f;
        leechFeedbackSuppressUntil = universeRealtimeElapsed + LEECH_MESSAGE_COOLDOWN;

        if (leechFeedbackNoTarget && !leechFeedbackHadTarget)
        {
            clCommandMessage(strGetString(strNoShipToLeechFound), -13746705);
        }
        leechFeedbackNoTarget = 0;
        leechFeedbackHadTarget = 0;
    }

    if (specifics->reclampRequested != 0)
    {
        specifics->reclampRequested = 0;
        if (specifics->currentTarget != NULL)
        {
            clampObjectSetup((SpaceObj *)ship, (SpaceObj *)specifics->currentTarget);
        }
    }

    if (specifics->dockRequested != 0)
    {
        Ship *dockTarget;

        specifics->dockRequested = 0;
        specifics->dockingIssued = 1;

        selection.numShips = 1;
        selection.timeLastStatus = 0.0f;
        selection.ShipPtr[0] = ship;

        dockTarget = FindNearestShipToDockAt(ship, LEECH_RESOURCE_DOCK_TYPE);
        clWrapDock(&universe.mainCommandLayer, &selection, LEECH_RESOURCE_DOCK_TYPE, dockTarget);
    }

    if (ship->command == NULL)
    {
        if (specifics->dockingIssued != 0)
        {
            specifics->dockingIssued = 0;
            if (specifics->preserveDockingState != 0)
            {
                specifics->preserveDockingState = 0;
            }
        }
    }

    if (ship->command != NULL &&
        specifics->state == LEECH_STATE_IDLE &&
        ship->command->ordertype.order == COMMAND_SPECIAL)
    {
        selection.numShips = 1;
        selection.timeLastStatus = 0.0f;
        selection.ShipPtr[0] = ship;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selection);
    }

    if (ship->tacticstype == Aggressive &&
        (universe.univUpdateCounter & LEECH_AUTO_ATTACK_MASK) == LEECH_AUTO_ATTACK_FRAME)
    {
        if (ship->command == NULL || ship->command->ordertype.order != COMMAND_SPECIAL)
        {
            leechAttackNearestTarget(ship);
        }
    }

    if (specifics->state == LEECH_MODE_DAMAGE &&
        universe.totaltimeelapsed > specifics->nextModeEffectTime)
    {
        leechCreateModeEffect(ship, LEECH_MODE_DAMAGE);
    }

    if (specifics->removeDoingStuffRequested != 0)
    {
        specifics->removeDoingStuffRequested = 0;
        selection.numShips = 1;
        selection.timeLastStatus = 0.0f;
        selection.ShipPtr[0] = ship;
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selection);
    }
}

void leechRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    if (specifics->targetList != NULL)
    {
        selSelectionRemoveSingleShip(specifics->targetList, shipToRemove);
    }
    if (specifics->currentTarget == shipToRemove)
    {
        leechAbortCurrentAction(ship);
    }
    if (specifics->savedTarget == shipToRemove)
    {
        specifics->savedTarget = NULL;
        specifics->feedbackTimer = 0.0f;
    }
}

void leechClose(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    leechReleaseClaimedHardpoint(specifics);
    if (specifics->targetList != NULL)
    {
        memFree(specifics->targetList);
        specifics->targetList = NULL;
    }
}

void leechSetMode(Ship *ship, ubyte leechMode)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    specifics->requestedMode = leechMode;
    if (leechMode == 6)
    {
        speechEventQueue(ship, LEECH_MODE_SPEECH_STEALTH, 0, -1, -1, -1, -1, 0.0f, -1);
    }
    else if (leechMode == 7)
    {
        speechEventQueue(ship, LEECH_MODE_SPEECH_RESOURCE, 0, -1, -1, -1, -1, 0.0f, -1);
    }
    else if (leechMode == 8)
    {
        speechEventQueue(ship, LEECH_MODE_SPEECH_ATTACK, 0, -1, -1, -1, -1, 0.0f, -1);
    }
}

void leech_PreFix(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    specifics->currentTarget = (Ship *)SpaceObjRegistryGetID((SpaceObj *)specifics->currentTarget);
    specifics->savedTarget = (Ship *)SpaceObjRegistryGetID((SpaceObj *)specifics->savedTarget);
}

void leech_Save(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    if (specifics->targetList != NULL)
    {
        SaveSelection((SpaceObjSelection *)specifics->targetList);
    }
}

void leech_Load(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);

    if (specifics->targetList != NULL)
    {
        specifics->targetList = memAlloc(sizeof(MaxSelection), "LeechTargets", NonVolatile);
        {
            SelectCommand *selection = (SelectCommand *)LoadSelection();
            memcpy(specifics->targetList, selection, sizeofSelectCommand(selection->numShips));
            memFree(selection);
        }
    }
}

void leech_Fix(Ship *ship)
{
    LeechSpecifics *specifics = leechSpecifics(ship);
    sdword i;

    specifics->currentTarget = (Ship *)SpaceObjRegistryGetShip((sdword)specifics->currentTarget);
    specifics->savedTarget = (Ship *)SpaceObjRegistryGetShip((sdword)specifics->savedTarget);

    if (specifics->targetList != NULL)
    {
        MaxSelection *targetList = (MaxSelection *)specifics->targetList;

        for (i = 0; i < targetList->numShips; i++)
        {
            targetList->ShipPtr[i] = (Ship *)SpaceObjRegistryGetShip((sdword)targetList->ShipPtr[i]);
        }
    }
}
