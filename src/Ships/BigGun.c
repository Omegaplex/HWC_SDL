/*=============================================================================
    Name    : BigGun.c
    Purpose : Cataclysm big-gun helpers and Beast mothership targeting code
=============================================================================*/

#include <math.h>
#include <string.h>
#include "types.h"
#include "SpaceObj.h"
#include "debug.h"
#include "battlechat.h"
#include "aitrack.h"
#include "attack.h"
#include "AIPlayer.h"
#include "AIShip.h"
#include "AIVar.h"
#include "Collision.h"
#include "etg.h"
#include "linkedlist.h"
#include "Mothership.h"
#include "shipselect.h"
#include "CommandWrap.h"
#include "pieplate.h"
#include "randy.h"
#include "select.h"
#include "sensors.h"
#include "ShipDefs.h"
#include "SoundEvent.h"
#include "statscript.h"
#include "strings.h"
#include "StringsOnly.h"
#include "Timer.h"
#include "meshanim.h"
#include "Gun.h"
#include "SaveGame.h"
#include "KASFunc.h"
#include "singleplayer.h"
#include "universe.h"
#include "univupdate.h"
#include "vector.h"

#include "CommandLayer.h"
#include "BeastMothership.h"
#include "BigGun.h"


#define SHIP_DISABLE_STATE_FLAG (SOF_Crazy | SOF_Disabled)

bool32 TimerExpired(int timer);
void telldir(void);
sdword shipQueueBeastConversion(Ship *ship, Ship *attacker);
bool32 univObjIsDying(SpaceObj *obj);
extern bool32 etgFireEffectsEnabled;

typedef struct
{
    ubyte initialized;
    ubyte pad[3];
    sdword beastAttackCascadePoints;
    real32 beastAttackCascadeDelay;
    real32 beastAttackCascadeDistance;
    real32 beastAttackTimeSpentStunned;
    real32 beastAttackTimeToCompleteAttacks;
    real32 stunnedSpinRate;
} BeastWeaponInfo;

typedef struct ShipBigGunQueueEntry
{
    Node node;
    Bullet *bullet;
    Ship *attacker;
    Ship *sourceShip;
} ShipBigGunQueueEntry;

#define BIG_GUN_FIRE_FLAGS_OFFSET 0x1C8

static_assert(sizeof(ShipBigGunState) == 0x18, "ShipBigGunState size mismatch");
static_assert(sizeof(ShipBigGunQueueEntry) == 0x1C, "ShipBigGunQueueEntry size mismatch");

static const real32 bigGunCascadeDelay = 0.5f;                   /* beastweapon.script beastAttackCascadeDelay */
static const real32 bigGunCascadeDistance = 2000.0f;             /* beastweapon.script beastAttackCascadeDistance */
static const real32 bigGunCascadeMinDamage = 2000.0f;            /* cataclysm.exe 0x72A5AC */
static const real32 bigGunCascadeDamageScale = 0.2f;             /* cataclysm.exe 0x72A510 */
static const real32 bigGunCascadeSoundPitch = 0.2f;              /* cataclysm.exe 0x72A510 */
static const real32 bigGunCascadeStunDuration = 10.0f;           /* beastweapon.script beastAttackTimeSpentStunned */
static const sdword bigGunCascadePoints = 20;                    /* beastweapon.script beastAttackCascadePoints */
static Ship *bigGunChainSourceShip = NULL;
static BeastWeaponInfo beastWeaponInfo;

static scriptStructEntry BeastWeaponInfoScriptTable[] =
{
    { "beastAttackCascadePoints", scriptSetSdwordCB, (udword)&beastWeaponInfo.beastAttackCascadePoints, (udword)&beastWeaponInfo },
    { "beastAttackCascadeDelay", scriptSetReal32CB, (udword)&beastWeaponInfo.beastAttackCascadeDelay, (udword)&beastWeaponInfo },
    { "beastAttackCascadeDistance", scriptSetReal32CB, (udword)&beastWeaponInfo.beastAttackCascadeDistance, (udword)&beastWeaponInfo },
    { "beastAttackTimeSpentStunned", scriptSetReal32CB, (udword)&beastWeaponInfo.beastAttackTimeSpentStunned, (udword)&beastWeaponInfo },
    { "beastAttackTimeToCompleteAttacks", scriptSetReal32CB, (udword)&beastWeaponInfo.beastAttackTimeToCompleteAttacks, (udword)&beastWeaponInfo },
    { "stunnedSpinRate", scriptSetReal32CB, (udword)&beastWeaponInfo.stunnedSpinRate, (udword)&beastWeaponInfo },
    { NULL, NULL, 0, 0 }
};

static_assert(sizeof(CommandToDo) >= COMMAND_SIEGECANNON_TARGET_OFFSET + sizeof(vector), "CommandToDo too small for siege cannon target");

static ShipBigGunState *ShipGetBigGunInfo(Ship *ship)
{
    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return (ShipBigGunState *)ship->ShipSpecifics;
        default:
            return NULL;
    }
}

void shipBigGunInfoInit(Ship *ship)
{
    ShipBigGunState *info;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    info->currentTarget = NULL;
    listInit(&info->queuedTargets);
    info->autostabilizeSuppressed = FALSE;
    ship->specialFlags4 &= ~SPECIAL4_Unknown15;
}

bool32 shipCanHaveBigGun(Ship *ship)
{
    switch (ship->staticinfo->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return TRUE;
        default:
            return FALSE;
    }
}

bool32 beastWeaponInfoInit(void)
{
    if (!beastWeaponInfo.initialized)
    {
        scriptSetStruct("race_beast\\", "beastweapon.script", BeastWeaponInfoScriptTable, (ubyte *)&beastWeaponInfo);
        beastWeaponInfo.initialized = TRUE;
    }
    return beastWeaponInfo.initialized;
}

bool32 shipStaticCanHaveBigGun(ShipStaticInfo *staticinfo)
{
    switch (staticinfo->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return TRUE;
        default:
            return FALSE;
    }
}

bool32 shipCanUseBigGun(Ship *ship)
{
    switch (ship->staticinfo->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT);
        default:
            return FALSE;
    }
}

real32 bigGunChargeDuration(sdword shiptype)
{
    real32 duration = beastWeaponInfo.beastAttackTimeSpentStunned;

    if (shiptype == dEcclesiam)
    {
        duration += duration;
    }
    return duration;
}

sdword shipQueueBeastConversion(Ship *ship, Ship *attacker)
{
    Player *playerowner = attacker->playerowner;
    ShipStaticInfo *staticinfo = ship->staticinfo;
    real32 spinRate = beastWeaponInfo.stunnedSpinRate;
    real32 stunDuration = bigGunChargeDuration(ship->shiptype);
    real32 phaseStartTime = universe.totaltimeelapsed + 1.0f;
    ShipStaticInfo *convertedStaticInfo;
    shipbindings *replacementBindings;

    ship->flags |= SHIP_DISABLE_STATE_FLAG;
    ship->flags2 = (ship->flags2 & 0xFFFFFF7Fu) | (((ranRandom(1) % 0x64) > 0x32) << 7);

    ship->rotinfo.torque.x += spinRate * 0.5f / staticinfo->staticheader.oneOverMomentOfInertiaX;
    ship->rotinfo.torque.y += spinRate * (1.0f / 3.0f) / staticinfo->staticheader.oneOverMomentOfInertiaY;
    ship->rotinfo.torque.z += spinRate * 0.25f / staticinfo->staticheader.oneOverMomentOfInertiaZ;

    ship->beastAttack.sourceShip = attacker;
    ship->beastAttack.sourcePlayer = playerowner;
    ship->statusWindow.state = 2;
    ship->statusWindow.activeUntil = universe.totaltimeelapsed + stunDuration;
    ship->statusWindow.phaseStartTime = phaseStartTime;
    ship->statusWindow.phaseEndTime = phaseStartTime + stunDuration;

    convertedStaticInfo = GetShipStaticInfo(staticinfo->beastAttackConvertTo, playerowner->race);
    replacementBindings = univMeshBindingsStaticDupe(convertedStaticInfo);
    ship->beastAttack.effect = (Effect *)replacementBindings;

    if (singlePlayerGame)
    {
        ship->beastAttack.effectColorScheme = 1;
    }
    else
    {
        Ship *playerMothership = playerowner->PlayerMothership;

        ship->beastAttack.effectColorScheme =
            (playerMothership != NULL) ? playerMothership->colorScheme : 0;
    }

    if (ship->runtimeAnimationOverlay.madAnimationData != 0)
    {
        madAnimationStop(ship);
    }

    speechEventQueue(ship, 16779425, ship->shiptype, -1, -1, -1, -1, 0.0f, -1);
    aiplayerShipDied(ship);
    shipHasJustBeenDisabled(ship);

    if (singlePlayerGame && singlePlayerGameInfo.currentMission == 7 && ship->shiptype == dPrisonShip)
    {
        AIVar *anyFSM = aivarFindAnyFSM("G_TRIGGER_FirstShipInfected");
        sdword *entry = aivarFindEntry(0, anyFSM);

        if (entry != NULL && entry[2] == 0)
        {
            GrowSelection growSelection;
            AIVar *firstInfectedFSM;

            growSelectInit(&growSelection);
            growSelectAddShip(&growSelection, ship);
            firstInfectedFSM = aivarFindAnyFSM("FirstInfectedC");
            kasfShipsAdd(kasGrowSelectionPtr(AIVarToNumber(firstInfectedFSM)), &growSelection);
            growSelectClose(&growSelection);
        }
        return speechEventQueue(NULL, 16779425, ship->shiptype, -1, 0, -1, -1, 5.0f, -1);
    }

    return singlePlayerGame;
}

void shipBigGunInfoClose(Ship *ship)
{
    ShipBigGunState *info;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    listRemoveAll(&info->queuedTargets);
    info->currentTarget = NULL;
}

void SaveShipBigGunInfo(Ship *ship)
{
    ShipBigGunState *info;
    Node *node;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    SaveInfoNumber(info->queuedTargets.num);
    SaveInfoNumber(info->savedState);
    SaveInfoNumber(info->currentTarget != NULL ?
                   SpaceObjRegistryGetID((SpaceObj *)info->currentTarget) :
                   -1);

    for (node = info->queuedTargets.head; node != NULL; node = node->next)
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);
        SaveChunk *chunk = CreateChunk(BASIC_STRUCTURE, sizeof(*entry), entry);
        ShipBigGunQueueEntry *savecontents = chunkContents(chunk);

        savecontents->bullet = (Bullet *)SpaceObjRegistryGetID((SpaceObj *)entry->bullet);
        savecontents->attacker = (Ship *)SpaceObjRegistryGetID((SpaceObj *)entry->attacker);
        savecontents->sourceShip = (Ship *)SpaceObjRegistryGetID((SpaceObj *)entry->sourceShip);

        SaveThisChunk(chunk);
        memFree(chunk);
    }
}

void LoadShipBigGunInfo(Ship *ship)
{
    ShipBigGunState *info;
    sdword count;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    count = LoadInfoNumber();
    info->savedState = LoadInfoNumber();
    info->currentTarget = (Ship *)LoadInfoNumber();
    info->autostabilizeSuppressed = FALSE;
    listInit(&info->queuedTargets);

    while (count > 0)
    {
        SaveChunk *chunk = LoadNextChunk();
        ShipBigGunQueueEntry *entry;

        VerifyChunk(chunk, BASIC_STRUCTURE, sizeof(ShipBigGunQueueEntry));
        entry = memAlloc(sizeof(*entry), "ShipBigGunQueueEntry", 0);
        memcpy(entry, chunkContents(chunk), sizeof(*entry));
        memFree(chunk);

        listAddNode(&info->queuedTargets, &entry->node, entry);
        count--;
    }
}

void FixShipBigGunInfo(Ship *ship)
{
    ShipBigGunState *info;
    Node *node;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    info->currentTarget = SpaceObjRegistryGetShip((sdword)info->currentTarget);
    for (node = info->queuedTargets.head; node != NULL; node = node->next)
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);

        entry->bullet = (Bullet *)SpaceObjRegistryGetObj((sdword)entry->bullet);
        entry->attacker = (Ship *)SpaceObjRegistryGetObj((sdword)entry->attacker);
        entry->sourceShip = (Ship *)SpaceObjRegistryGetObj((sdword)entry->sourceShip);
    }
}

static void ShipBigGunFilterCrystals(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        if (selection->ShipPtr[i] != NULL && selection->ShipPtr[i]->shiptype == dBigGunUnit)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }
}

void shipBigGunInfoClearQueuedPointers(Ship *ship, void *objectRef, void *bulletRef)
{
    ShipBigGunState *info;
    Node *node;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    for (node = info->queuedTargets.head; node != NULL; node = node->next)
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);

        if (bulletRef != NULL && entry->bullet == bulletRef)
        {
            entry->bullet = NULL;
        }
        if (objectRef != NULL)
        {
            if (entry->attacker == objectRef)
            {
                entry->attacker = NULL;
            }
            if (entry->sourceShip == objectRef)
            {
                entry->sourceShip = NULL;
            }
        }
    }
}

void shipBigGunInfoClearTargetPointers(Ship *ship, void *targetRef)
{
    ShipBigGunState *info;
    Node *node;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    if (info->currentTarget == targetRef)
    {
        info->currentTarget = NULL;
    }

    for (node = info->queuedTargets.head; node != NULL; node = node->next)
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);

        if (targetRef != NULL)
        {
            if (entry->attacker == targetRef)
            {
                entry->attacker = NULL;
            }
            if (entry->sourceShip == targetRef)
            {
                entry->sourceShip = NULL;
            }
        }
    }
}

Ship *bigGunFindNearestTargetInSelection(Ship *ship, SelectCommand *selection)
{
    Ship *nearestTarget = NULL;
    real32 nearestDistanceSquared = 1.0e21f;
    sdword i;

    if (selection->numShips <= 0)
    {
        return NULL;
    }

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *candidate = selection->ShipPtr[i];
        vector delta;
        real32 distanceSquared;

        if (candidate->shiptype == sLeech ||
            candidate->shiptype == sSwarmer ||
            candidate->shiptype == bSwarmer ||
            (!singlePlayerGame && candidate->playerowner == ship->playerowner))
        {
            continue;
        }

        vecSub(delta, candidate->posinfo.position, ship->posinfo.position);
        distanceSquared = vecMagnitudeSquared(delta);
        if (distanceSquared < nearestDistanceSquared)
        {
            nearestDistanceSquared = distanceSquared;
            nearestTarget = candidate;
        }
    }

    return nearestTarget;
}

bool32 bigGunHasActiveOrSelectableTarget(Ship *ship, SelectCommand *selection)
{
    ShipBigGunState *info;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }
    return info->currentTarget != NULL || bigGunFindNearestTargetInSelection(ship, selection) != NULL;
}

bool32 bigGunChooseTargetFromSelection(Ship *ship, SelectCommand *selection)
{
    ShipBigGunState *info;
    Ship *target;
    Ship *previousTarget;
    Node *node;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }

    ShipBigGunFilterCrystals(selection);
    if (selection->numShips == 0)
    {
        return TRUE;
    }
    if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_INFECT) && !singlePlayerGame)
    {
        return TRUE;
    }

    target = bigGunFindNearestTargetInSelection(ship, selection);
    if (target == NULL)
    {
        return info->currentTarget == NULL;
    }

    previousTarget = info->currentTarget;
    for (node = info->queuedTargets.head; node != NULL; node = node->next)
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);

        if (entry->sourceShip == target)
        {
            return info->currentTarget == NULL;
        }
    }

    if (target->statusWindow.activeUntil == 0.0f)
    {
        info->currentTarget = target;
    }

    if (previousTarget != target)
    {
        speechEventQueue(NULL, 67111095, -1, -1, 0, ship->playerowner->playerIndex, -1, 5.0f, -1);
    }

    return info->currentTarget == NULL;
}

void shipBigGunInfoClearCurrentTarget(Ship *ship)
{
    ShipBigGunState *info;

    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            info = (ShipBigGunState *)ship->ShipSpecifics;
            break;
        default:
            info = NULL;
            break;
    }
    info->currentTarget = NULL;
}

void *shipGetBeastWeaponInfo(Ship *ship)
{
    ShipStaticInfo *staticinfo = ship->staticinfo;

    if ((staticinfo->beastAttacksRequired & 0x80000000) != 0)
    {
        return NULL;
    }

    return &staticinfo->beastAttackCascadeCost;
}

static GunSoundType bigGunExpectedSoundType(Ship *ship)
{
    switch (ship->shiptype)
    {
        case bHeavyCruiser:
        case bCarrier:
        case bMothership:
        case dBeastMothership:
            return GS_BeastWeapon;
        default:
            return GS_BigGun;
    }
}

static etglod *bigGunHitEffect(void)
{
    return etgGunEventTable[RACE_Beast][GS_BeastWeapon][EGT_GunHit];
}


static bool32 bigGunIsValidChainTarget(void *candidatePointer)
{
    Ship *candidate = (Ship *)candidatePointer;
    Node *node;
    Node *shipNode;

    if (candidate == NULL || bigGunChainSourceShip == NULL)
    {
        return FALSE;
    }

    if (candidate->playerowner != bigGunChainSourceShip->playerowner)
    {
        return FALSE;
    }
    if (candidate->statusWindow.activeUntil != 0.0f)
    {
        return FALSE;
    }
    if (candidate->shiptype == sLeech ||
        candidate->shiptype == sSwarmer ||
        candidate->shiptype == bSwarmer)
    {
        return FALSE;
    }

    for (shipNode = universe.ShipList.head; shipNode != NULL; shipNode = shipNode->next)
    {
        Ship *ship = listGetStructOfNode(shipNode);
        ShipBigGunState *info = NULL;

        switch (ship->staticinfo->shiptype)
        {
            case bHeavyCruiser:
            case bCarrier:
            case bMothership:
            case dBeastMothership:
                info = (ShipBigGunState *)ship->ShipSpecifics;
                break;
            default:
                break;
        }
        if (info == NULL)
        {
            continue;
        }
        for (node = info->queuedTargets.head; node != NULL; node = node->next)
        {
            ShipBigGunQueueEntry *entry = listGetStructOfNode(node);

            if (entry->sourceShip == candidate)
            {
                return FALSE;
            }
        }
    }
    return ((bigGunChainSourceShip->specialFlags2 & 0x00008000) != 0) ||
           candidate->staticinfo->beastAttackCascadeCost < bigGunChainSourceShip->beastAttack.raw_4F4;
}

static bool32 bigGunApplyHitAndQueueChain(Ship *ship, Ship *target, Bullet *bullet)
{
    Ship *sourceShip = target->beastAttack.sourceShip;
    bool32 canCascade = target->staticinfo->beastAttackCascadeCost >= 0.0f;
    ShipBigGunState *sourceInfo;
    ShipBigGunQueueEntry *entry;
    union
    {
        real32 realValue;
        udword rawValue;
    } rawCascadeTime;

    dbgAssert(sourceShip != NULL);
    if (sourceShip == NULL)
    {
        return FALSE;
    }
    if (canCascade)
    {
        target->statusWindow.chainHitCount++;
        if ((target->shiptype == 17 || target->shiptype == 52) &&
            univObjIsDying((SpaceObj *)target))
        {
            return TRUE;
        }

        if (target->statusWindow.chainHitCount < target->staticinfo->beastAttacksRequired)
        {
            target->statusWindow.phaseScale = -1.0f;
            target->statusWindow.state = 1;
            target->statusWindow.phaseStartTime = universe.totaltimeelapsed + 1.0f;
            target->statusWindow.phaseEndTime = target->statusWindow.phaseStartTime + bigGunCascadeStunDuration;
        }
        else
        {
            shipQueueBeastConversion(target, sourceShip);
        }

        rawCascadeTime.realValue = universe.totaltimeelapsed;
        target->beastAttack.raw_4F0 = rawCascadeTime.rawValue;
        target->beastAttack.nextChainTime = universe.totaltimeelapsed + bigGunCascadeDelay;
        target->beastAttack.raw_4F4 = bigGunCascadePoints;
    }
    else
    {
        real32 damage = target->health * bigGunCascadeDamageScale;

        if (damage < bigGunCascadeMinDamage)
        {
            damage = target->health - 1.0f;
        }

        ApplyDamageToTarget((SpaceObjRotImpTarg *)target,
                            damage,
                            bullet->soundType,
                            DEATH_Killed_By_Player,
                            sourceShip->playerowner->playerIndex);

        // Retail (bigGunApplyHitAndQueueChain, IDA 0x42b0c0-0x42b0e1) writes this
        // per-shot scratch value on `ship` (the live shooter parameter that just fired
        // this bullet: `mov eax,[ebp+attacker]; mov [eax+4B0h],ebx; cmp [eax+2B0h],2Eh;
        // jnz skip; mov [eax+4B0h],esi`), not on `sourceShip`
        // (target->beastAttack.sourceShip, a potentially stale historical reference to
        // whichever ship originally started this beast-conversion chain, which may since
        // have been destroyed and its ship slot reused by an unrelated ship).
        ship->dockvars.customdockinfo = NULL;
        if (ship->shiptype == bHeavyCruiser && !singlePlayerGame)
        {
            ship->dockvars.customdockinfo = (DockStaticPoint *)target;
        }

        rawCascadeTime.realValue = universe.totaltimeelapsed;
        target->beastAttack.raw_4F0 = rawCascadeTime.rawValue;
        target->beastAttack.nextChainTime = universe.totaltimeelapsed + bigGunCascadeDelay;
        target->beastAttack.raw_4F4 = 0;
    }

    sourceInfo = ShipGetBigGunInfo(sourceShip);
    dbgAssert(sourceInfo != NULL);

    entry = memAlloc(sizeof(*entry), "ShipBigGunQueueEntry", NonVolatile);
    entry->bullet = bullet;
    entry->attacker = ship;
    entry->sourceShip = target;
    listAddNode(&sourceInfo->queuedTargets, &entry->node, entry);

    return TRUE;
}

void bigGunProcessTargetingAndFiring(Ship *ship)
{
    ShipBigGunState *info;
    Ship *currentTarget;
    Ship *selectedTarget;
    GunInfo *gunInfo;
    GunSoundType expectedSoundType;
    Gun *gun;
    Bullet *bullet;
    Ship *target;
    Node *node;

    info = ShipGetBigGunInfo(ship);
    dbgAssert(info != NULL);
    if (info == NULL)
    {
        return;
    }

    currentTarget = info->currentTarget;
    if (currentTarget != NULL)
    {
        selectedTarget = currentTarget;
        if (currentTarget->objtype == OBJ_ShipType && currentTarget->health > 0.0f)
        {
            vector heading;
            real32 nearestDistanceSquared;
            Node *todoNode;

            vecSub(heading, currentTarget->posinfo.position, ship->posinfo.position);
            nearestDistanceSquared = vecMagnitudeSquared(heading);
            vecNormalize(&heading);

            for (todoNode = universe.mainCommandLayer.todolist.head; todoNode != NULL; todoNode = todoNode->next)
            {
                CommandToDo *command = listGetStructOfNode(todoNode);

                if ((command->ordertype.attributes & COMMAND_MASK_PROTECTING) == 0)
                {
                    continue;
                }
                if (command->protect != NULL && command->protect->numShips != 0)
                {
                    SelectCommand *commandSelection = command->selection;
                    sdword i;

                    if (commandSelection != NULL &&
                        commandSelection->numShips > 0 &&
                        commandSelection->ShipPtr[0]->playerowner != ship->playerowner)
                    {
                        bool32 foundBlockedTarget = FALSE;

                        for (i = 0; i < commandSelection->numShips; i++)
                        {
                            Ship *candidate = commandSelection->ShipPtr[i];

                            if ((candidate->shiptype == dEcclesiam || candidate->shiptype == dPrisonShip) &&
                                univObjIsDying((SpaceObj *)candidate) &&
                                collCheckRayShipCollPolygons(candidate,
                                                             &ship->posinfo.position,
                                                             &heading,
                                                             NULL) != -1)
                            {
                                foundBlockedTarget = TRUE;
                            }
                        }

                        if (foundBlockedTarget)
                        {
                            for (i = 0; i < commandSelection->numShips; i++)
                            {
                                Ship *candidate = commandSelection->ShipPtr[i];

                                if (candidate->shiptype == dEcclesiam || candidate->shiptype == dPrisonShip)
                                {
                                    vector delta;
                                    real32 distanceSquared;

                                    vecSub(delta, ship->posinfo.position, candidate->posinfo.position);
                                    distanceSquared = vecMagnitudeSquared(delta);
                                    if (distanceSquared < nearestDistanceSquared)
                                    {
                                        nearestDistanceSquared = distanceSquared;
                                        selectedTarget = candidate;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            info->currentTarget = selectedTarget;
            ship->autostabilizeship = FALSE;
            aishipFlyToPointAvoidingObjsFunc(ship, &selectedTarget->posinfo.position, 0xFu, 0.0f, NULL);

            if (bigGunChargeFraction(ship) == 1.0f)
            {
                vector toTarget;
                real32 targetDistanceSquared;
                Ship *previousSourceShip;
                struct Player *previousSourcePlayer;
                SelectCommand selection;
                sdword gunIndex;

                gunInfo = ship->cataclysmGunInfo;
                expectedSoundType = bigGunExpectedSoundType(ship);
                if (gunInfo != NULL && gunInfo->numGuns > 0)
                {
                    vecSub(toTarget, selectedTarget->collInfo.collPosition, ship->posinfo.position);
                    targetDistanceSquared = vecMagnitudeSquared(toTarget);
                    bullet = NULL;
                    for (gunIndex = 0; gunIndex < gunInfo->numGuns && bullet == NULL; gunIndex++)
                    {
                        real32 rangeSquared;
                        bool32 inRange;
                        bool32 oriented;
                        bool32 modeReady;

                        gun = &gunInfo->guns[gunIndex];
                        if (gun->gunstatic == NULL ||
                            gun->gunstatic->gunsoundtype != expectedSoundType)
                        {
                            continue;
                        }

                        rangeSquared = gun->gunstatic->bulletrange * gun->gunstatic->bulletrange;
                        inRange = targetDistanceSquared < rangeSquared;
                        oriented = FALSE;
                        if (inRange)
                        {
                            oriented = gunOrientGimbleGun(ship, gun, (SpaceObjRotImpTarg *)selectedTarget);
                        }
                        modeReady = *(udword *)gun == WEAPON_BeastInfection;
                        if (inRange && oriented && modeReady)
                        {
                            bullet = gunShootGunAtTarget(ship, gun, (SpaceObjRotImpTarg *)selectedTarget);
                        }
                    }
                    if (bullet != NULL)
                    {
                        bullet->hitEffect = bigGunHitEffect();

                        previousSourceShip = selectedTarget->beastAttack.sourceShip;
                        previousSourcePlayer = selectedTarget->beastAttack.sourcePlayer;
                        selectedTarget->beastAttack.sourceShip = ship;
                        selectedTarget->beastAttack.sourcePlayer = ship->playerowner;
                        ship->beastAttack.sourceShip = ship;
                        ship->beastAttack.sourcePlayer = ship->playerowner;

                        if (bigGunApplyHitAndQueueChain(ship, selectedTarget, bullet))
                        {
                            soundEventPlay(ship, 513, bullet->gunowner);
                        }
                        else
                        {
                            selectedTarget->beastAttack.sourceShip = previousSourceShip;
                            selectedTarget->beastAttack.sourcePlayer = previousSourcePlayer;
                        }
                        info->currentTarget = NULL;
                        memset(&selection, 0, sizeof(selection));
                        selection.numShips = 1;
                        selection.ShipPtr[0] = ship;
                        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selection);
                    }
                }
            }
            info->autostabilizeSuppressed = TRUE;
        }
    }
    else if (info->autostabilizeSuppressed)
    {
        ship->autostabilizeship = TRUE;
        info->autostabilizeSuppressed = FALSE;
    }

    for (node = info->queuedTargets.head; node != NULL; )
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);
        Bullet *entryBullet = entry->bullet;
        Ship *entryTarget = entry->sourceShip;
        Node *next = node->next;

        // Retail (bigGunProcessTargetingAndFiring, IDA 0x42acc6-0x42ad0f) gates this
        // chain-processing branch on exactly four conditions: entryTarget != NULL,
        // entry->bullet != NULL, nextChainTime != 0, and totaltimeelapsed > nextChainTime.
        // It never dereferences entry->bullet beyond the NULL check (no flags/SOF_Dead
        // read), and does not test entryTarget->health or ->beastAttack.sourceShip here.
        // Those three extra guards were non-parity additions; the entryBullet->flags
        // read in particular was a use-after-free once the queued bullet had already
        // been freed by univUpdateAllPosVelBullets.
        if (entryTarget != NULL &&
            entryBullet != NULL &&
            entryTarget->beastAttack.nextChainTime != 0.0f &&
            universe.totaltimeelapsed > entryTarget->beastAttack.nextChainTime)
        {
            bigGunChainSourceShip = entryTarget;
            target = univFindNearestShipWithConstraints(entryTarget, bigGunIsValidChainTarget);
            if (target != NULL)
            {
                vector toTarget;

                vecSub(toTarget, target->collInfo.collPosition, entryTarget->collInfo.collPosition);
                if (vecMagnitudeSquared(toTarget) <= bigGunCascadeDistance * bigGunCascadeDistance)
                {
                    Ship *previousSourceShip = target->beastAttack.sourceShip;
                    struct Player *previousSourcePlayer = target->beastAttack.sourcePlayer;

                    if (previousSourceShip == NULL ||
                        previousSourceShip->playerowner != ship->playerowner ||
                        (target->specialFlags2 & SPECIAL_2_DisabledState) != 0)
                    {
                        target->beastAttack.sourceShip = ship->beastAttack.sourceShip;
                        target->beastAttack.sourcePlayer = ship->beastAttack.sourcePlayer;

                        bullet = gunCreateBullet(ship,
                                                 entryBullet->gunowner,
                                                 (SpaceObjRotImpTarg *)entryTarget,
                                                 (SpaceObjRotImpTarg *)target,
                                                 1.0f);
                        if (bullet != NULL)
                        {
                            bullet->hitEffect = bigGunHitEffect();
                            if (bigGunApplyHitAndQueueChain(entryTarget, target, bullet))
                            {
                                target->beastAttack.raw_4F4 = (udword)(entryTarget->beastAttack.raw_4F4 -
                                                                              target->staticinfo->beastAttackCascadeCost);
                                entryTarget->beastAttack.raw_4F4 = 0;
                                soundEventPlay(entryTarget, 2103, 0);
                            }
                            else
                            {
                                entryTarget->beastAttack.raw_4F4 = 0;
                                target->beastAttack.sourceShip = previousSourceShip;
                                target->beastAttack.sourcePlayer = previousSourcePlayer;
                            }
                        }
                    }
                }
                else
                {
                    entryTarget->beastAttack.raw_4F4 = 0;
                }
            }
            else
            {
                entryTarget->beastAttack.raw_4F4 = 0;
            }
            bigGunChainSourceShip = NULL;
            entryTarget->beastAttack.nextChainTime = 0.0f;
        }
        node = next;
    }

    for (node = info->queuedTargets.head; node != NULL; )
    {
        ShipBigGunQueueEntry *entry = listGetStructOfNode(node);
        Node *next = node->next;

        if (entry->bullet == NULL)
        {
            listRemoveNode(node);
        }
        node = next;
    }
}

// Cat2: live implementation in Mothership.c uses mothershipBigGunExpectedSoundType helper (0x5bb790)

// Cat2: live implementation in Mothership.c uses universe.phystimeelapsed not totaltimeelapsed,
//       and mothershipFindBigGunEntry instead of inline loop (0x5bd440)

// Cat2: live implementation in Mothership.c missing clCommandMessage call (0x5BBBD0)
#if 0  // Cat2: MothershipMoveOrderCleanup binary inlines logic; live code delegates (0x5BBBD0)
/*=== MothershipMoveOrderCleanup (0x5BBBD0) ===*/
int __cdecl MothershipMoveOrderCleanup(int a1)
{
    char v1;
    int v2;

    if ( *(_DWORD *)(a1 + 688) == 18 )
    {
        v1 = *(_BYTE *)(a1 + 1652);
        if ( v1 != 3 )
        {
            if ( v1 )
            {
                if ( v1 == 4 )
                {
                    v2 = *(_DWORD *)(a1 + 1664);
                    *(_BYTE *)(a1 + 1668) = 2;
                    if ( v2 )
                        **(_DWORD **)(v2 + 288) = 1065353216;
                }
                *(_BYTE *)(a1 + 1652) = 3;
                if ( aitrackStabilizeGuidance(a1) )
                    *(_BYTE *)(a1 + 1652) = 0;
                if ( *(Player **)(a1 + 812) == universe.numPlayers )
                    clCommandMessage(dword_BB501C);
            }
        }
    }
    return 1;
}
#endif

// Cat2: live implementation in Mothership.c uses MothershipMoveOrderCleanup call instead of inlining (0x5BBC50)
#if 0  // Cat2: MothershipAttackCommand binary inlines cleanup; live code delegates (0x5BBC50)
/*=== MothershipAttackCommand (0x5BBC50) ===*/
int __cdecl MothershipAttackCommand(struct Ship *ship, struct SpaceObjRotImpTarg *target)
{
    char x_low;
    real32 collOptimizeDist;

    if ( ship->shiptype == 18 )
    {
        x_low = LOBYTE(ship[1].currentLOD.x);
        if ( x_low != 3 )
        {
            if ( x_low )
            {
                if ( x_low == 4 )
                {
                    collOptimizeDist = ship[1].cameraDistanceSquared;
                    LOBYTE(ship[1].cameraDistanceVector) = 2;
                    if ( collOptimizeDist != 0.0 )
                        **(_DWORD **)(LODWORD(collOptimizeDist) + 288) = 1065353216;
                }
                LOBYTE(ship[1].currentLOD.x) = 3;
                if ( aitrackStabilizeGuidance((int)ship) )
                    LOBYTE(ship[1].currentLOD.x) = 0;
                if ( *(Player **)&ship->playerowner == universe.numPlayers )
                    clCommandMessage(dword_BB501C);
            }
        }
    }
    attackSimple(ship, target);
    return 1;
}
#endif

// Cat2: live implementation in Mothership.c uses mothershipFindReadyBigGun instead of inline clampInfo loop (0x5BC050)
#if 0  // Cat2: MothershipSpecialActivateNoTarget live code uses mothershipFindReadyBigGun (0x5BC050)
/*=== MothershipSpecialActivateNoTarget (0x5BC050) ===*/
int __cdecl MothershipSpecialActivateNoTarget(int a1)
{
    int v1;
    int v2;
    int *v3;
    int v4;
    _DWORD *v5;

    v1 = 0;
    if ( *(_DWORD *)(a1 + 688) == 18 )
    {
        v2 = *(_DWORD *)(a1 + 444);
        if ( v2 )
        {
            v3 = *(int **)(v2 + 52);
            if ( v3 )
            {
                v4 = *v3;
                if ( *v3 > 0 )
                {
                    v5 = v3 + 2;
                    do
                    {
                        if ( *(_DWORD *)(*v5 + 688) == 31 && (*(_BYTE *)(*v5 + 460) & 8) == 0 )
                            v1 = 1;
                        ++v5;
                        --v4;
                    }
                    while ( v4 );
                    if ( v1 && *(_BYTE *)(a1 + 1652) )
                        *(_BYTE *)(a1 + 1652) = 0;
                }
            }
        }
    }
    return 1;
}
#endif

// Cat2: live implementation in Mothership.c delegates to MothershipMoveOrderCleanup instead of inlining (0x5BCE00)
#if 0  // Cat2: MothershipCanHalt binary inlines logic; live code delegates (0x5BCE00)
/*=== MothershipCanHalt (0x5BCE00) ===*/
int __cdecl MothershipCanHalt(int a1)
{
    char v1;
    int v2;

    if ( *(_DWORD *)(a1 + 688) == 18 )
    {
        v1 = *(_BYTE *)(a1 + 1652);
        if ( v1 != 3 )
        {
            if ( v1 )
            {
                if ( v1 == 4 )
                {
                    v2 = *(_DWORD *)(a1 + 1664);
                    *(_BYTE *)(a1 + 1668) = 2;
                    if ( v2 )
                        **(_DWORD **)(v2 + 288) = 1065353216;
                }
                *(_BYTE *)(a1 + 1652) = 3;
                if ( aitrackStabilizeGuidance(a1) )
                    *(_BYTE *)(a1 + 1652) = 0;
                if ( *(Player **)(a1 + 812) == universe.numPlayers )
                    clCommandMessage(dword_BB501C);
            }
        }
    }
    return 1;
}
#endif
