/*=============================================================================
    Name    : HiveFrigate.c
    Purpose : Cataclysm-specific Hive Frigate special-action wiring
=============================================================================*/
#include "types.h"
#include "spaceobj.h"
#include "HiveFrigate.h"
#include <math.h>

#include "AIShip.h"
#include "Attack.h"
#include "Battle.h"
#include "Collision.h"
#include "CommandLayer.h"
#include "DefaultShip.h"
#include "dock.h"
#include "Gun.h"
#include "MadLinkIn.h"
#include "Memory.h"
#include "Randy.h"
#include "SaveGame.h"
#include "Ship.h"
#include "SoundEvent.h"
#include "UnivUpdate.h"
#include "CommandWrap.h"
#include "shipselect.h"
#include "universe.h"

typedef struct
{
    real32 swarmerRebuildDelay;                  // +0x00
    real32 unk4;                                 // +0x04
    real32 swarmerAutoTargetRange;               // +0x08
    real32 swarmAttackRange[NUM_TACTICS_TYPES];  // +0x0C
    real32 swarmTooCloseRange[NUM_TACTICS_TYPES];// +0x18
    ubyte unk24[0x1C];                           // +0x24
} HiveFrigateStatics;

typedef struct
{
    /*
     * Retail overlays the hive-launch state onto the existing swarmer tail
     * instead of starting at ShipSpecifics[0]. The first 8 bytes remain the
     * swarmer's own runtime state, and the sal-cap captureRef slot at +0x10
     * doubles as the launch anim-state field.
     */
    ubyte reserved_00_07[0x08];
    sdword slotIndex;
    Ship *parentHive;
    sdword animState;
    real32 readyAt;
    real32 reserved_18;
    real32 launchDelay;
    real32 healthUpdateTime;
} HiveSwarmerOverlay;

typedef struct
{
    real32 rebuildSwarmerAt;                     // swarmer slot rebuild timer at ShipSpecifics+0x00
    HiveFrigateSpecifics spec;
} HiveFrigateRuntimeOverlay;

static HiveFrigateStatics HiveFrigateStaticsSect;
static HiveFrigateStatics HiveFrigateStaticsBeast;

bool32 fogObjectVisibleToPlayer(Player *player, SpaceObj *object, bool32 ignoreNebula);

static_assert(offsetof(HiveSwarmerOverlay, slotIndex) == 0x08);
static_assert(offsetof(HiveSwarmerOverlay, parentHive) == 0x0C);
static_assert(offsetof(HiveSwarmerOverlay, animState) == 0x10);
static_assert(offsetof(HiveSwarmerOverlay, readyAt) == 0x14);
static_assert(offsetof(HiveSwarmerOverlay, launchDelay) == 0x1C);
static_assert(offsetof(HiveSwarmerOverlay, healthUpdateTime) == 0x20);

static HiveFrigateStatics *hiveFrigateStatics(Ship *ship)
{
    return (HiveFrigateStatics *)ship->staticinfo->custstatinfo;
}

HiveFrigateSpecifics *hiveFrigateSpec(Ship *ship)
{
    return &((HiveFrigateRuntimeOverlay *)ship->ShipSpecifics)->spec;
}

static real32 *hiveFrigateRebuildTime(Ship *ship)
{
    return &((HiveFrigateRuntimeOverlay *)ship->ShipSpecifics)->rebuildSwarmerAt;
}

static HiveSwarmerOverlay *hiveSwarmerOverlay(Ship *ship)
{
    return (HiveSwarmerOverlay *)ship->ShipSpecifics;
}

static ubyte *hiveFrigateMadState(Ship *ship)
{
    return (ubyte *)&ship->attackvars.attackevents;
}

static ShipType hiveFrigateSwarmerType(Ship *ship)
{
    return (ship->shiprace == RACE_Beast) ? bSwarmer : sSwarmer;
}

static ubyte hiveFrigateBusyFlag(Ship *ship)
{
    return (ubyte)ship->specialFlags1C8;
}

static real32 hiveFrigateLaunchDelay(void)
{
    static real64 const hiveLaunchDelayScale = -4.6566128730773926e-10;
    udword rnd = ranRandom(1);
    real64 t = (real64)rnd * hiveLaunchDelayScale;
    return (real32)(2.0 * (1.0 - pow(2.0, t)));
}

sdword hiveSwarmerGetState(Ship *swarmer)
{
    sdword state = hiveSwarmerOverlay(swarmer)->animState;
    CommandToDo *command;

    if (swarmer->shiplink.belongto == NULL)
    {
        return HiveSwarmer_Docked;
    }

    command = swarmer->command;
    if (command == NULL)
    {
        if (state == HiveSwarmer_Launching ||
            state == HiveSwarmer_Returning ||
            state == HiveSwarmer_Attacking)
        {
            return HiveSwarmer_Deployed;
        }
        return state;
    }

    if (command->ordertype.order == COMMAND_ATTACK)
    {
        return HiveSwarmer_Attacking;
    }
    if (command->ordertype.order != COMMAND_DOCK)
    {
        return HiveSwarmer_Deployed;
    }
    if (state == HiveSwarmer_Docked)
    {
        return HiveSwarmer_Launching;
    }
    if (state == HiveSwarmer_Deployed || state == HiveSwarmer_Attacking)
    {
        return HiveSwarmer_Returning;
    }
    return state;
}

void hiveSwarmerSetState(Ship *swarmer, sdword state)
{
    hiveSwarmerOverlay(swarmer)->animState = state;
}

static real32 *hiveSwarmerHealthUpdateTime(Ship *swarmer)
{
    return &hiveSwarmerOverlay(swarmer)->healthUpdateTime;
}

static SelectAnyCommand *hiveFrigateAllocTargetCommand(sdword numTargets)
{
    SelectAnyCommand *command;

    dbgAssert(numTargets > 0);
    command = memAlloc(sizeof(SelectAnyCommand) + sizeof(TargetPtr) * (numTargets - 1), "HiveTargets", 0);
    command->numTargets = numTargets;
    command->timeLastStatus = 0.0f;
    return command;
}

static void hiveFrigateIssueNearbyTargets(Ship *ship, sdword maxTargets, bool32 issueSpecial)
{
    SelectAnyCommand *nearbyTargets;
    SelectAnyCommand *commandCopy;
    SelectCommand selection;
    sdword i;

    nearbyTargets = univFindNearbyTargets(ship, 7000.0f);
    if (nearbyTargets == NULL || nearbyTargets->numTargets <= 0)
    {
        return;
    }

    commandCopy = hiveFrigateAllocTargetCommand(nearbyTargets->numTargets);
    for (i = 0; i < nearbyTargets->numTargets; i++)
    {
        commandCopy->TargetPtr[i] = nearbyTargets->TargetPtr[i];
    }

    selection.numShips = 1;
    selection.timeLastStatus = 0.0f;
    selection.ShipPtr[0] = ship;
    if (issueSpecial)
    {
        clSpecial(&universe.mainCommandLayer, &selection, (SpecialCommand *)commandCopy);
    }
    else
    {
        clAttack(&universe.mainCommandLayer, &selection, (AttackCommand *)commandCopy);
    }
    memFree(commandCopy);
}

static Ship *hiveFrigateCreateDockedSwarmer(Ship *ship, sdword slotIndex)
{
    HiveFrigateSpecifics *spec;
    HiveSwarmerOverlay *overlay;
    Ship *swarmer;

    spec = hiveFrigateSpec(ship);
    swarmer = univCreateShip(hiveFrigateSwarmerType(ship), ship->shiprace, &ship->posinfo.position, ship->playerowner, 5);
    gameStatsAddShip(swarmer, ship->playerowner->playerIndex);
    swarmer->flags &= ~0x2000u;

    overlay = hiveSwarmerOverlay(swarmer);
    overlay->slotIndex = slotIndex;
    overlay->parentHive = ship;

    spec->swarmerCount++;
    dockPutShipInside(swarmer, ship);
    spec->swarmers[slotIndex] = swarmer;
    return swarmer;
}

static void hiveFrigateStartReturnSpeech(Ship *ship, HiveFrigateSpecifics *spec)
{
    if (spec->launchState != HiveLaunch_Idle && spec->launchState != HiveLaunch_Recalling)
    {
        speechEventQueue(ship, 67111082, 0, -1, -1, -1, -1, 0.0f, -1);
        spec->launchState = HiveLaunch_Canceling;
    }
}

static void hiveFrigateClearTargets(Ship *ship, bool32 clearCurrentTarget)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    for (i = 0; i < spec->targetCount; i++)
    {
        spec->targetList[i] = NULL;
    }
    spec->targetCount = 0;
    if (clearCurrentTarget)
    {
        spec->currentTarget = NULL;
    }
}

// Decompiled from cataclysm.exe @ 0x5B57C0
bool32 HiveFrigateCancelSpecial(Ship *ship)
{
    HiveFrigateSpecifics *spec;

    spec = hiveFrigateSpec(ship);
    hiveFrigateStartReturnSpeech(ship, spec);
    hiveFrigateClearTargets(ship, TRUE);
    spec->specialState = 0;
    return TRUE;
}

static void hiveFrigateUpdateDockedShipHealth(Ship *ship)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
    {
        Ship *dockedShip = spec->swarmers[i];
        if (dockedShip == NULL)
        {
            continue;
        }

        if (hiveSwarmerGetState(dockedShip) == HiveSwarmer_Docked &&
            dockedShip->health < dockedShip->staticinfo->maxhealth)
        {
            real32 *lastUpdateTime = hiveSwarmerHealthUpdateTime(dockedShip);
            if (*lastUpdateTime > 0.0f)
            {
                real32 elapsed = universe.totaltimeelapsed - *lastUpdateTime;
                if (elapsed > 1.0f)
                {
                    dockedShip->health += elapsed;
                    if (dockedShip->health > dockedShip->staticinfo->maxhealth)
                    {
                        dockedShip->health = dockedShip->staticinfo->maxhealth;
                    }
                    *lastUpdateTime = universe.totaltimeelapsed;
                }
            }
            else
            {
                *lastUpdateTime = universe.totaltimeelapsed;
            }
        }
    }
}

// Decompiled from cataclysm.exe @ 0x5B54B0
static void HiveFrigateStaticInit(char *directory, char *field, ShipStaticInfo *statinfo)
{
    HiveFrigateStatics *statics;
    sdword i;

    (void)directory;
    (void)field;

    statics = (statinfo->shiprace == RACE_Beast) ? &HiveFrigateStaticsBeast : &HiveFrigateStaticsSect;
    statics->swarmerRebuildDelay = 15.0f;
    statics->unk4 = 0.0f;
    statics->swarmerAutoTargetRange = 7000.0f;
    for (i = 0; i < NUM_TACTICS_TYPES; i++)
    {
        statics->swarmAttackRange[i] = statinfo->bulletRange[i] * 0.8f;
        statics->swarmTooCloseRange[i] = statinfo->minBulletRange[i] * 0.7f;
    }
    statinfo->custstatinfo = statics;
    statinfo->unk4 = TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B5520
static void HiveFrigateInit(Ship *ship)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    spec->launchState = HiveLaunch_Idle;
    *hiveFrigateRebuildTime(ship) = universe.totaltimeelapsed;
    spec->currentTarget = NULL;
    spec->specialState = 0;
    spec->dockReceiver = NULL;
    spec->swarmerCount = 0;
    spec->targetCount = 0;
    *hiveFrigateMadState(ship) = 0;

    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
    {
        hiveFrigateCreateDockedSwarmer(ship, i);
    }
}

// Decompiled from cataclysm.exe @ 0x5B5950
static void HiveFrigateFire(Ship *ship, SpaceObjRotImpTarg *target, sdword rotateFlags)
{
    DefaultShipFire(ship, target, rotateFlags);
    hiveFrigateSpec(ship)->specialState = 0;
}

// Decompiled from cataclysm.exe @ 0x5B5770
static void HiveFrigateAttackPassive(Ship *ship, Ship *target, bool32 rotate)
{
    if ((ship->specialFlags3 & rotate) != 0)
    {
        attackPassive(ship, target);
    }
    else
    {
        attackSimple(ship, (SpaceObjRotImpTarg *)target);
    }
    hiveFrigateSpec(ship)->specialState = 0;
}

// Decompiled from cataclysm.exe @ 0x5B5920
static bool32 HiveFrigateSpecialActivate(Ship *ship)
{
    hiveFrigateSpec(ship)->specialState = 0;
    return speechEventQueue(ship, 67111081, 0, -1, -1, -1, -1, 0.0f, -1);
}

// Decompiled from cataclysm.exe @ 0x5B5850
static bool32 HiveFrigateSpecialTarget(Ship *ship, void *custom)
{
    HiveFrigateSpecifics *spec;
    SelectAnyCommand *targets;
    sdword i;
    sdword targetCount;

    spec = hiveFrigateSpec(ship);
    targets = (SelectAnyCommand *)custom;
    targetCount = targets->numTargets;
    if (targetCount > 1000)
    {
        targetCount = 1000;
    }

    for (i = 0; i < targetCount; i++)
    {
        SpaceObjRotImpTarg *target = targets->TargetPtr[i];
        // Exclude friendly swarmers from the target list.
        if (target != NULL &&
            target->objtype == OBJ_ShipType &&
            ((Ship *)target)->shiprace == ship->shiprace &&
            (((Ship *)target)->shiptype == sSwarmer || ((Ship *)target)->shiptype == bSwarmer))
        {
            spec->targetList[i] = NULL;
        }
        else
        {
            spec->targetList[i] = target;
        }
    }

    spec->targetCount = targetCount;
    // Set currentTarget to the first non-NULL entry.
    for (i = 0; i < targetCount; i++)
    {
        if (spec->targetList[i] != NULL)
        {
            spec->currentTarget = spec->targetList[i];
            break;
        }
    }
    // Keep approach/launch states; only force-transition to Deploying if we're
    // in a state that hasn't committed swarmers yet.
    if (spec->launchState != HiveLaunch_Idle &&
        spec->launchState != HiveLaunch_Approach &&
        spec->launchState != HiveLaunch_Armed &&
        spec->launchState != HiveLaunch_Launching)
    {
        spec->launchState = HiveLaunch_Deploying;
    }
    spec->specialState = 0;
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B5B30
static void HiveFrigateHousekeep(Ship *ship)
{
    HiveFrigateSpecifics *spec;
    HiveFrigateStatics *statics;
    SelectCommandMax6 swarmers;
    SelectCommand commandSelection;
    vector trajectory;
    sdword i;
    sdword selectedCount;
    sdword animState;
    real32 attackRange;

    spec = hiveFrigateSpec(ship);
    statics = hiveFrigateStatics(ship);
    attackRange = statics->swarmAttackRange[ship->tacticstype];

    if ((hiveFrigateBusyFlag(ship) & 0x80) != 0)
    {
        {
            return;
        }
    }

    // If the current target is no longer visible in fog-of-war, cancel the special.
    if (spec->currentTarget != NULL &&
        !fogObjectVisibleToPlayer(ship->playerowner, (SpaceObj *)spec->currentTarget, TRUE))
    {
        commandSelection.numShips = 1;
        commandSelection.timeLastStatus = 0.0f;
        commandSelection.ShipPtr[0] = ship;
        clWrapSpecial(&universe.mainCommandLayer, &commandSelection, NULL);
    }

    switch (spec->launchState)
    {
        case HiveLaunch_Idle:
            // In Aggressive mode, auto-acquire nearby targets when idle.
            if (ship->tacticstype == Aggressive &&
                spec->currentTarget == NULL &&
                (universe.univUpdateCounter & 15) == 14)
            {
                hiveFrigateIssueNearbyTargets(ship, 0, TRUE);
            }
            if (*hiveFrigateMadState(ship) != 0)
            {
                madAnimationStart(ship, 1);
                *hiveFrigateMadState(ship) = 0;
            }
            /* fall through */

        case HiveLaunch_Approach:
            // Handle a pending dock-to-player-base request queued during the special.
            if (spec->specialState == 2)
            {
                swarmers.numShips = 1;
                swarmers.ShipPtr[0] = ship;
                clDock(&universe.mainCommandLayer, (SelectCommand *)&swarmers, 0, spec->dockReceiver);
                spec->specialState = 0;
            }
            // Periodically rebuild any destroyed swarmer slots.
            if (universe.totaltimeelapsed > *hiveFrigateRebuildTime(ship))
            {
                *hiveFrigateRebuildTime(ship) = universe.totaltimeelapsed + statics->swarmerRebuildDelay;
                if (spec->swarmerCount < HIVE_MAX_SWARMERS)
                {
                    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
                    {
                        if (spec->swarmers[i] == NULL)
                        {
                            hiveFrigateCreateDockedSwarmer(ship, i);
                            break;
                        }
                    }
                }
            }
            hiveFrigateUpdateDockedShipHealth(ship);
            if (spec->currentTarget != NULL)
            {
                real32 rangeToTarget;
                aishipGetTrajectory(ship, spec->currentTarget, &trajectory);
                rangeToTarget = RangeToTarget(ship, spec->currentTarget, &trajectory);
                if (rangeToTarget < attackRange)
                {
                    // In range: transition to Armed to begin the launch sequence.
                    spec->launchState = HiveLaunch_Armed;
                    spec->specialState = 0;
                }
                else
                {
                    // Out of range: start approaching. Set Approach if we were Idle.
                    if (spec->launchState == HiveLaunch_Idle)
                    {
                        spec->launchState = HiveLaunch_Approach;
                    }
                    spec->specialState = 1;
                    aishipFlyToPointAvoidingObjsFunc(ship, &spec->currentTarget->posinfo.position, 7u, -400.0f, &spec->currentTarget->posinfo.velocity);
                    ship->shipidle = 0;
                    ship->autostabilizeship = 0;
                }
            }
            return;

        case HiveLaunch_Armed:
            // Assign randomized launch timers to all docked swarmers.
            selectedCount = 0;
            for (i = 0; i < HIVE_MAX_SWARMERS; i++)
            {
                Ship *swarmer = spec->swarmers[i];
                if (swarmer != NULL && hiveSwarmerGetState(swarmer) == HiveSwarmer_Docked)
                {
                    real32 delay = hiveFrigateLaunchDelay();
                    hiveSwarmerOverlay(swarmer)->launchDelay = delay;
                    hiveSwarmerOverlay(swarmer)->readyAt = universe.totaltimeelapsed + delay;
                    selectedCount++;
                }
            }
            if (selectedCount <= 0)
            {
                spec->launchState = HiveLaunch_Idle;
            }
            else
            {
                if (ship->tacticstype == Aggressive)
                {
                    hiveFrigateIssueNearbyTargets(ship, selectedCount, FALSE);
                }
                spec->launchState = HiveLaunch_Launching;
                if (*hiveFrigateMadState(ship) == 0)
                {
                    madAnimationStart(ship, 0);
                    *hiveFrigateMadState(ship) = 1;
                }
            }
            return;

        case HiveLaunch_Launching:
            // Issue clDock launch commands to swarmers whose delay has elapsed.
            swarmers.numShips = 0;
            selectedCount = 1;
            for (i = 0; i < HIVE_MAX_SWARMERS; i++)
            {
                Ship *swarmer = spec->swarmers[i];
                if (swarmer != NULL)
                {
                    animState = hiveSwarmerGetState(swarmer);
                    if (animState == HiveSwarmer_Docked)
                    {
                        if (hiveSwarmerOverlay(swarmer)->readyAt < universe.totaltimeelapsed)
                        {
                            hiveSwarmerSetState(swarmer, HiveSwarmer_Launching);
                            swarmers.ShipPtr[swarmers.numShips++] = swarmer;
                        }
                        selectedCount = 0;  // still waiting on at least one swarmer
                    }
                    else if (animState == HiveSwarmer_Launching)
                    {
                        selectedCount = 0;  // still launching
                    }
                }
            }
            if (swarmers.numShips > 0)
            {
                clDock(
                    &universe.mainCommandLayer,
                    (SelectCommand *)&swarmers,
                    DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP,
                    ship);
            }
            if (selectedCount != 0)
            {
                // All swarmers are out: issue attack orders.
                spec->launchState = HiveLaunch_Deploying;
                if (battleCanChatterAtThisTime(45, ship))
                {
                    battleChatterAttempt(-1, 45, ship, -1);
                }
            }
            return;

        case HiveLaunch_Deploying:
            // Order all deployed swarmers to attack the current target.
            swarmers.numShips = 0;
            for (i = 0; i < HIVE_MAX_SWARMERS; i++)
            {
                Ship *swarmer = spec->swarmers[i];
                if (swarmer != NULL && hiveSwarmerGetState(swarmer) >= HiveSwarmer_Deployed)
                {
                    hiveSwarmerSetState(swarmer, HiveSwarmer_Attacking);
                    swarmers.ShipPtr[swarmers.numShips++] = swarmer;
                }
            }
            if (swarmers.numShips > 0 && spec->currentTarget != NULL)
            {
                AttackCommand attack;
                attack.numTargets = 1;
                attack.timeLastStatus = 0.0f;
                attack.TargetPtr[0] = spec->currentTarget;
                clAttack(&universe.mainCommandLayer, (SelectCommand *)&swarmers, &attack);
            }
            spec->launchState = HiveLaunch_Active;
            // If the hive itself is out of range, close in alongside the swarmers.
            if (spec->currentTarget != NULL)
            {
                aishipGetTrajectory(ship, spec->currentTarget, &trajectory);
                if (RangeToTarget(ship, spec->currentTarget, &trajectory) >= attackRange)
                {
                    spec->specialState = 1;
                    aishipFlyToPointAvoidingObjsFunc(ship, &spec->currentTarget->posinfo.position, 7u, -400.0f, &spec->currentTarget->posinfo.velocity);
                    ship->shipidle = 0;
                    ship->autostabilizeship = 0;
                }
            }
            return;

        case HiveLaunch_Active:
            // While swarmers are attacking, keep the hive in range and re-launch stragglers.
            if (spec->specialState == 1 && spec->currentTarget != NULL)
            {
                aishipGetTrajectory(ship, spec->currentTarget, &trajectory);
                if (RangeToTarget(ship, spec->currentTarget, &trajectory) < attackRange)
                {
                    spec->specialState = 0;
                }
                else
                {
                    spec->specialState = 1;
                    aishipFlyToPointAvoidingObjsFunc(ship, &spec->currentTarget->posinfo.position, 7u, -400.0f, &spec->currentTarget->posinfo.velocity);
                    ship->shipidle = 0;
                    ship->autostabilizeship = 0;
                }
            }

            // Any newly-deployed swarmers without orders get attack commands.
            swarmers.numShips = 0;
            for (i = 0; i < HIVE_MAX_SWARMERS; i++)
            {
                Ship *swarmer = spec->swarmers[i];
                if (swarmer != NULL && hiveSwarmerGetState(swarmer) == HiveSwarmer_Deployed)
                {
                    hiveSwarmerSetState(swarmer, HiveSwarmer_Attacking);
                    swarmers.ShipPtr[swarmers.numShips++] = swarmer;
                }
            }
            if (swarmers.numShips > 0 && spec->currentTarget != NULL)
            {
                AttackCommand attack;
                attack.numTargets = 1;
                attack.timeLastStatus = 0.0f;
                attack.TargetPtr[0] = spec->currentTarget;
                clAttack(&universe.mainCommandLayer, (SelectCommand *)&swarmers, &attack);
            }

            // Within auto-target range, also launch any docked swarmers immediately.
            if (spec->currentTarget != NULL)
            {
                aishipGetTrajectory(ship, spec->currentTarget, &trajectory);
                if (RangeToTarget(ship, spec->currentTarget, &trajectory) < statics->swarmerAutoTargetRange)
                {
                    swarmers.numShips = 0;
                    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
                    {
                        Ship *swarmer = spec->swarmers[i];
                        if (swarmer != NULL && hiveSwarmerGetState(swarmer) == HiveSwarmer_Docked)
                        {
                            hiveSwarmerSetState(swarmer, HiveSwarmer_Launching);
                            swarmers.ShipPtr[swarmers.numShips++] = swarmer;
                        }
                    }
                    if (swarmers.numShips > 0)
                    {
                        clDock(
                            &universe.mainCommandLayer,
                            (SelectCommand *)&swarmers,
                            DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP,
                            ship);
                    }
                }
            }

            if (universe.totaltimeelapsed <= *hiveFrigateRebuildTime(ship))
            {
                return;
            }
            *hiveFrigateRebuildTime(ship) = universe.totaltimeelapsed + statics->swarmerRebuildDelay;
            if (spec->swarmerCount < HIVE_MAX_SWARMERS)
            {
                for (i = 0; i < HIVE_MAX_SWARMERS; i++)
                {
                    if (spec->swarmers[i] == NULL)
                    {
                        hiveFrigateCreateDockedSwarmer(ship, i);
                        break;
                    }
                }
            }
            return;

        case HiveLaunch_Canceling:
            // Wait for all out-swarmers to finish their current attack pass,
            // then delay their return so they land gracefully.
            selectedCount = 0;
            for (i = 0; i < HIVE_MAX_SWARMERS; i++)
            {
                Ship *swarmer = spec->swarmers[i];
                if (swarmer != NULL)
                {
                    animState = hiveSwarmerGetState(swarmer);
                    if (animState != HiveSwarmer_Docked && animState != HiveSwarmer_Returning)
                    {
                        selectedCount++;
                        hiveSwarmerOverlay(swarmer)->readyAt = universe.totaltimeelapsed + hiveSwarmerOverlay(swarmer)->launchDelay;
                    }
                }
            }
            if (selectedCount > 0)
            {
                spec->launchState = HiveLaunch_Recalling;
                return;
            }
            selectedCount = *hiveFrigateMadState(ship);
            spec->launchState = HiveLaunch_Idle;
            if (selectedCount != 0)
            {
                madAnimationStart(ship, 1);
                *hiveFrigateMadState(ship) = 0;
            }
            return;

        case HiveLaunch_Recalling:
            // Issue dock commands to swarmers whose recall delay has elapsed.
            swarmers.numShips = 0;
            selectedCount = 1;
            for (i = 0; i < HIVE_MAX_SWARMERS; i++)
            {
                Ship *swarmer = spec->swarmers[i];
                if (swarmer != NULL)
                {
                    animState = hiveSwarmerGetState(swarmer);
                    if (animState == HiveSwarmer_Deployed || animState == HiveSwarmer_Attacking)
                    {
                        if (hiveSwarmerOverlay(swarmer)->readyAt < universe.totaltimeelapsed)
                        {
                            hiveSwarmerSetState(swarmer, HiveSwarmer_Returning);
                            swarmers.ShipPtr[swarmers.numShips++] = swarmer;
                        }
                        selectedCount = 0;  // still waiting on at least one swarmer
                    }
                    else if (animState != HiveSwarmer_Docked)
                    {
                        selectedCount = 0;  // still in transit
                    }
                }
            }
            if (swarmers.numShips > 0)
            {
                clDock(&universe.mainCommandLayer, (SelectCommand *)&swarmers, 1, ship);
            }
            if (selectedCount != 0)
            {
                // All swarmers are home: stop the launch animation.
                selectedCount = *hiveFrigateMadState(ship);
                spec->launchState = HiveLaunch_Idle;
                if (selectedCount != 0)
                {
                    madAnimationStart(ship, 1);
                    *hiveFrigateMadState(ship) = 0;
                }
            }
            return;

        default:
            return;
    }
}

// Decompiled from cataclysm.exe @ 0x5B5640
static void HiveFrigateRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    if (shipToRemove->shiptype == sSwarmer || shipToRemove->shiptype == bSwarmer)
    {
        for (i = 0; i < HIVE_MAX_SWARMERS; i++)
        {
            if (spec->swarmers[i] == shipToRemove)
            {
                spec->swarmers[i] = NULL;
                spec->swarmerCount--;
                break;
            }
        }
    }

    for (i = 0; i < spec->targetCount; i++)
    {
        if (spec->targetList[i] == (SpaceObjRotImpTarg *)shipToRemove)
        {
            spec->targetList[i] = NULL;
        }
    }

    if (spec->currentTarget == (SpaceObjRotImpTarg *)shipToRemove)
    {
        for (i = 0; i < spec->targetCount; i++)
        {
            if (spec->targetList[i] != NULL)
            {
                spec->currentTarget = spec->targetList[i];
                // Back to Deploying so the swarmers get fresh attack orders on the new target.
                if (spec->launchState == HiveLaunch_Active)
                {
                    spec->launchState = HiveLaunch_Deploying;
                }
                return;
            }
        }
        spec->currentTarget = NULL;
        spec->targetCount = 0;
        if (spec->launchState != HiveLaunch_Idle)
        {
            spec->launchState = HiveLaunch_Canceling;
        }
    }
}

// Decompiled from cataclysm.exe @ 0x5B55E0
static void HiveFrigateClose(Ship *ship)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
    {
        Ship *swarmer = spec->swarmers[i];
        if (swarmer != NULL && (swarmer->flags & SOF_Dead) == 0 && swarmer->health != 0.0f)
        {
            swarmer->posinfo.position.x = ship->posinfo.position.x;
            swarmer->posinfo.position.y = ship->posinfo.position.y;
            univDeleteDeadShip(swarmer, FALSE);
        }
        spec->swarmers[i] = NULL;
    }
}

// Decompiled from cataclysm.exe @ 0x5B6720
static void HiveFrigatePreFix(Ship *ship)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
    {
        spec->swarmers[i] = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->swarmers[i]);
    }
    for (i = 0; i < spec->targetCount; i++)
    {
        spec->targetList[i] = (SpaceObjRotImpTarg *)SpaceObjRegistryGetID((SpaceObj *)spec->targetList[i]);
    }
    spec->currentTarget = (SpaceObjRotImpTarg *)SpaceObjRegistryGetID((SpaceObj *)spec->currentTarget);
    spec->dockReceiver = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->dockReceiver);
}

// Decompiled from cataclysm.exe @ 0x5B67A0
static void HiveFrigateFix(Ship *ship)
{
    HiveFrigateSpecifics *spec;
    sdword i;

    spec = hiveFrigateSpec(ship);
    for (i = 0; i < HIVE_MAX_SWARMERS; i++)
    {
        spec->swarmers[i] = SpaceObjRegistryGetShip((sdword)spec->swarmers[i]);
    }
    for (i = 0; i < spec->targetCount; i++)
    {
        spec->targetList[i] = (SpaceObjRotImpTarg *)SpaceObjRegistryGetShip((sdword)spec->targetList[i]);
    }
    spec->currentTarget = (SpaceObjRotImpTarg *)SpaceObjRegistryGetShip((sdword)spec->currentTarget);
    spec->dockReceiver = SpaceObjRegistryGetShip((sdword)spec->dockReceiver);
}

// Decompiled from cataclysm.exe @ 0x5B5730
static bool32 HiveFrigateSpecialOrder(Ship *ship, void *custom)
{
    HiveFrigateStatics *statics;

    statics = hiveFrigateStatics(ship);
    attackStraightForward(
        ship,
        (SpaceObjRotImpTarg *)custom,
        statics->swarmAttackRange[ship->tacticstype],
        statics->swarmTooCloseRange[ship->tacticstype]);
    hiveFrigateSpec(ship)->specialState = 0;
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B65C0
static bool32 HiveFrigateMoveConstraint(Ship *ship, vector *fromPoint, vector *toPoint)
{
    HiveFrigateSpecifics *spec;

    spec = hiveFrigateSpec(ship);
    // A manual move order cancels the special entirely if we're idle or just approaching.
    if (spec->launchState == HiveLaunch_Idle || spec->launchState == HiveLaunch_Approach)
    {
        HiveFrigateCancelSpecial(ship);
    }
    else
    {
        spec->specialState = 0;
    }
    (void)fromPoint;
    (void)toPoint;
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B6650
static bool32 HiveFrigateCanHalt(Ship *ship)
{
    hiveFrigateSpec(ship)->specialState = 0;
    return TRUE;
}

// Decompiled from cataclysm.exe @ 0x5B6670
static bool32 HiveFrigateDockStart(Ship *ship, sdword dockType, Ship *dockwith)
{
    HiveFrigateSpecifics *spec;

    spec = hiveFrigateSpec(ship);
    if (spec->launchState == HiveLaunch_Idle)
    {
        return TRUE;
    }

    HiveFrigateCancelSpecial(ship);
    spec->specialState = 2;
    spec->dockReceiver = dockwith;
    (void)dockType;
    return FALSE;
}

CustShipHeader HiveFrigateHeader =
{
    sizeof(HiveFrigateRuntimeOverlay),
    HiveFrigateStaticInit,
    NULL,
    HiveFrigateInit,
    NULL,
    HiveFrigateFire,
    HiveFrigateAttackPassive,
    HiveFrigateCancelSpecial,
    HiveFrigateSpecialTarget,
    HiveFrigateSpecialActivate,
    HiveFrigateHousekeep,
    HiveFrigateRemoveShipReferences,
    HiveFrigateRemoveShipReferences,
    NULL,
    NULL,
    HiveFrigateClose,
    HiveFrigatePreFix,
    NULL,
    NULL,
    HiveFrigateFix,
    HiveFrigateMoveConstraint,
    HiveFrigateSpecialOrder,
    HiveFrigateDockStart,
    NULL,
    NULL,
    NULL,
    NULL,
    HiveFrigateCanHalt,
    NULL
};

#if 0  // Cat3: hiveFrigateHousekeep (0x5B5B30) — full retail decomp in temp\hive_5b5b30.c.txt; pending live port (0x5B5B30)
#endif

#if 0  // Cat2: Live HiveFrigateCanHalt at L867 uses hiveFrigateSpecifics(ship)->specialState = 0; return TRUE (0x5B6650)
int __cdecl hiveFrigateCanHalt(int object)
{
  *(_DWORD *)(object + 5636) = 0;
  return 1;
}
#endif
