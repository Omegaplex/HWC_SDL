#include <string.h>
#include <math.h>
#include "glinc.h"
#include "types.h"
#include "debug.h"
#include "memory.h"
#include "vector.h"
#include "matrix.h"
#include "linkedlist.h"
#include "objtypes.h"
#include "spaceobj.h"
#include "Randy.h"
#include "FastMath.h"
#include "Sensors.h"
#include "etg.h"
#include "render.h"
#include "SaveGame.h"
#include "soundevent.h"
#include "singleplayer.h"
#include "Ship.h"
#include "UnivUpdate.h"
#include "universe.h"
#include "physics.h"
#include "shockwave.h"

sdword univIsObjFriendly(SpaceObj *obj, Player *player);
bool32 shipIsBeastShip(Ship *ship);
void telldir(void);
void seekdir(void);
sdword soundEventUpdateShockwaveLoop(Shockwave *shockwave);

#define SHOCKWAVE_RENDER_SAMPLE_COUNT          3000
#define SHOCKWAVE_RENDER_SAMPLE_ORDER_COUNT    50
#define SHOCKWAVE_SPOKE_SEGMENTS               80
#define SHOCKWAVE_SPOKE_JITTER_RADIUS          3.0f
#define SHOCKWAVE_SOUND_EQ_ROW                 42
#define SHOCKWAVE_SOUND_SIZE                   350.0f
#define SHOCKWAVE_SOUND_PRIORITY               50
#define SHOCKWAVE_SOUND_NIS_DISTANCE_SCALE     0.25f
#define SHOCKWAVE_RENDER_POINT_BASE            600
#define SHOCKWAVE_RENDER_POINT_GROWTH_SCALE    0.4f
#define SHOCKWAVE_RENDER_POINT_FADE_START      0.9f
#define SHOCKWAVE_RENDER_POINT_MIN_COLOR       0.7f
#define SHOCKWAVE_RENDER_POINT_ROTATION_SCALE  0.017453292f
#define SHOCKWAVE_SPOKE_MIN_AZIMUTH_STEP       0.34906587f
#define SHOCKWAVE_SPOKE_MAX_AZIMUTH_STEP       1.04719758f
#define SHOCKWAVE_SPOKE_MIN_POLAR_STEP         0.34906587f
#define SHOCKWAVE_SPOKE_MAX_POLAR_STEP         0.69813174f
#define SHOCKWAVE_RANDOM_POLAR_MAX_FACTOR      10.0f
#define SHOCKWAVE_RENDER_POINT_RANDOM_SCALE    1.4629181e-09f
#define SHOCKWAVE_RANDOM_TO_UNIT_SCALE         2.3283064e-10f
#define SHOCKWAVE_RANDOM_TO_SIGNED_UNIT_SCALE  4.6566129e-10f
#define SHOCKWAVE_HALF_PI                      1.57079637f
#define SHOCKWAVE_RENDER_POINT_COUNT_ROUND     0.5f
#define SHOCKWAVE_PROJECTILE_SP_FALLOFF_MAX    60000.0f
#define SHOCKWAVE_PROJECTILE_SP_FALLOFF_MIN    35000.0f
#define SHOCKWAVE_PROJECTILE_MP_FALLOFF_MAX    90000.0f
#define SHOCKWAVE_PROJECTILE_MP_FALLOFF_MIN    30000.0f
#define SHOCKWAVE_PROJECTILE_MIN_DAMAGE_SCALE  0.01f
#define SHOCKWAVE_PROJECTILE_NEARBY_SCALE      0.5f

typedef struct ShockwaveState
{
    Node link;
    Shockwave shockwave;
} ShockwaveState;

typedef struct ShockwaveProjectile
{
    Node node;
    SpaceObjRotImpTarg *projectile;
} ShockwaveProjectile;

typedef struct ShockwaveRenderSample
{
    real32 radius;
    real32 azimuth;
    real32 polarAngle;
} ShockwaveRenderSample;

static const udword cataShockwaveTemplates[6][25] =
{
    {
        0x00000000, 0x0000001B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x513A43B7, 0x447A0000, 0x00000000, 0x45174000, 0x00000000, 0x00000000, 0x447A0000, 0x00000000,
        0x447A0000, 0x464B2000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x0000083A,
        0x3F800000,
    },
    {
        0x00000001, 0x0000001B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x513A43B7, 0x447A0000, 0x00000000, 0x45174000, 0x00000000, 0x00000000, 0x447A0000, 0x00000000,
        0x447A0000, 0x468CA000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x0000083A,
        0x3F800000,
    },
    {
        0x00000002, 0x0000002A, 0x00000000, 0x00000000, 0x00000000, 0x41200000, 0x447A0000, 0x00000000,
        0x00000000, 0x42C80000, 0x00000000, 0x45174000, 0x00000000, 0x40533333, 0x447A0000, 0x00000000,
        0x447A0000, 0x469C4000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x0000083A,
        0x3F800000,
    },
    {
        0x00000003, 0x00000024, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x520BB2C9, 0x43FA0000, 0x00000000, 0x459C4000, 0x00000000, 0x00000000, 0x447A0000, 0x00000000,
        0x00000000, 0x468CA000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x0000083E,
        0x3F800000,
    },
    {
        0x00000004, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x41200000, 0x447A0000, 0x00000000,
        0x00000000, 0x451C4000, 0x41200000, 0x44BB8000, 0x00000000, 0x00000000, 0x43480000, 0x00000000,
        0x447A0000, 0x461C4000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x0000083D,
        0x3F800000,
    },
    {
        0x00000005, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x459C4000, 0x43160000, 0x451C4000, 0x00000000, 0x00000000, 0x457A0000, 0x00000000,
        0x45866000, 0x46C35000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0x0000083F,
        0x3F800000,
    },
};

static_assert(sizeof(ShockwaveState) == 116, "ShockwaveState size mismatch");

static const matrix shockwaveActivationEffectMatrix =
{
    1.0f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

static const udword naggarokProjectileRefOffset = 0x3EC;

static LinkedList shockwaveList = UNINITIALISED_LINKED_LIST;
static LinkedList shockwaveProjectileList = UNINITIALISED_LINKED_LIST;
static ShockwaveRenderSample shockwaveRenderSamples[SHOCKWAVE_RENDER_SAMPLE_COUNT];
static sdword shockwaveRenderSampleOrder[SHOCKWAVE_RENDER_SAMPLE_ORDER_COUNT];
static sdword shockwaveReloadedFromSave = 0;
static real32 shockwaveLastSpokeRadius = 0.0f;
static real32 shockwaveLastSpokeAzimuth = 0.0f;
static real32 shockwaveLastSpokePolarAngle = 0.0f;
static bool32 shockwaveStarted = FALSE;

bool32 bulletIsShockwaveProjectile(SpaceObjRotImpTarg *projectile)
{
    if (projectile->objtype != OBJ_BulletType)
    {
        return FALSE;
    }

    return ((Bullet *)projectile)->soundType == GS_BigGun;
}

static bool32 shockwaveIsPositionInBounds(vector *position)
{
    return position->x <= smUniverseSizeX && position->x >= -smUniverseSizeX &&
           position->y <= smUniverseSizeY && position->y >= -smUniverseSizeY &&
           position->z <= smUniverseSizeZ && position->z >= -smUniverseSizeZ;
}

static real32 shockwaveDistanceSquared(vector *a, vector *b)
{
    real32 dx = a->x - b->x;
    real32 dy = a->y - b->y;
    real32 dz = a->z - b->z;

    return dx * dx + dy * dy + dz * dz;
}

static real32 shockwaveTimerRemaining(Timer *timer)
{
    real32 currentTime = timer->enabled ? taskTimeElapsed : universe.totaltimeelapsed;
    return timer->duration - (currentTime - timer->startTime);
}

static void shockwaveBeginVelocityFreeze(Ship *ship, real32 duration, real32 maxDuration)
{
    if (shockwaveIsPositionInBounds(&ship->posinfo.position))
    {
        if (ship->velocityFreezeActive)
        {
            TimerSetRemaining(&ship->velocityFreezeTimer, duration);
            if (shockwaveTimerRemaining(&ship->velocityFreezeTimer) > maxDuration)
            {
                TimerInitTimerStruct(&ship->velocityFreezeTimer, maxDuration);
            }
        }
        else
        {
            ship->velocityFreezeInitialVelocity = ship->posinfo.velocity;
            ship->velocityFreezeActive = TRUE;
            TimerInitTimerStruct(&ship->velocityFreezeTimer, duration);
        }
    }
}

static etgeffectstatic *shockwaveGetLinkingEffectStat(void)
{
    etglod *effectLOD = etgSpecialPurposeEffectTable[EGT_LINKING_EFFECT];

    if (effectLOD == NULL || effectLOD->nLevels <= 0)
    {
        return NULL;
    }

    return effectLOD->level[0];
}

static bool32 shockwaveSinglePlayerExemptType5Ship(Ship *ship)
{
    return ship->playerowner->playerIndex == 1;
}

static udword *shockwaveShipStatusWord(Ship *ship)
{
    return (udword *)&ship->attributesParam;
}

static ShockwaveState *shockwaveCreateFromTemplate(udword const *templateData)
{
    ShockwaveState *shockwaveState;

    dbgAssert(shockwaveStarted);

    shockwaveState = memAlloc(sizeof(ShockwaveState), "Shockwave", MBF_SmallBlockHeap);
    memcpy(&shockwaveState->shockwave, templateData, sizeof(shockwaveState->shockwave));
    shockwaveState->shockwave.runtimeState = 0;
    shockwaveState->shockwave.activated = FALSE;
    listAddNode(&shockwaveList, &shockwaveState->link, shockwaveState);
    return shockwaveState;
}

static void SaveShockwaveCB(void *shockwaveState)
{
    SaveStructureOfSize(shockwaveState, sizeof(ShockwaveState));
}

static void LoadShockwaveCB(LinkedList *list)
{
    ShockwaveState *shockwaveState = LoadStructureOfSize(sizeof(ShockwaveState));

    shockwaveState->shockwave.runtimeState = 0;
    listAddNode(list, &shockwaveState->link, shockwaveState);
}

static Effect *shockwaveCreateActivationEffect(Shockwave *shockwave)
{
    etglod *effectLOD;
    etgeffectstatic *effectStat;
    vector zeroVelocity = { 0.0f, 0.0f, 0.0f };
    real32 totalShockwaves;
    real32 endRadius;
    real32 totalDuration;

    if (shockwave->activationEffectType < 0)
    {
        return NULL;
    }

    effectLOD = etgSpecialPurposeEffectTable[shockwave->activationEffectType];
    if (effectLOD == NULL || effectLOD->nLevels <= 0)
    {
        return NULL;
    }

    effectStat = effectLOD->level[0];
    if (effectStat == NULL || etgFrequencyExceeded(effectStat))
    {
        return NULL;
    }

    totalDuration = (shockwave->maxRadius - shockwave->currentRadius) / shockwave->expansionRate;
    endRadius = shockwave->expansionRate * 100.0f + shockwave->currentRadius;
    totalShockwaves = (real32)shockwaveList.num;
    if (RGLtype == SWtype)
    {
        totalShockwaves = 100.0f;
    }

    return etgEffectCreate(effectStat,
                           NULL,
                           &shockwave->position,
                           &zeroVelocity,
                           (matrix *)&shockwaveActivationEffectMatrix,
                           1.0f,
                           120,
                           5,
                           TreatAsUdword(totalShockwaves),
                           TreatAsUdword(shockwave->currentRadius),
                           TreatAsUdword(shockwave->expansionRate),
                           TreatAsUdword(endRadius),
                           TreatAsUdword(totalDuration));
}

static void shockwaveApplyToShip(Shockwave *shockwave, Ship *ship)
{
    vector offset;
    vector direction;
    real32 distanceSquared;
    real32 outerRadiusSquared;
    real32 innerRadius;
    real32 innerRadiusSquared;
    real32 distance;
    real32 forceMagnitude;
    real32 extraDamage = 0.0f;
    real32 damage;
    udword *statusWord;
    bool32 inBounds;
    bool32 withinOuterRadius;
    bool32 outsideInnerRadius;

    vecSub(offset, ship->posinfo.position, shockwave->position);
    distanceSquared = vecMagnitudeSquared(offset);
    outerRadiusSquared = shockwave->currentRadius * shockwave->currentRadius;
    innerRadius = shockwave->currentRadius - shockwave->radiusStep;
    innerRadiusSquared = innerRadius * innerRadius;
    inBounds = shockwaveIsPositionInBounds(&ship->posinfo.position);
    withinOuterRadius = distanceSquared < outerRadiusSquared;
    outsideInnerRadius = distanceSquared > innerRadiusSquared;
    if (!withinOuterRadius)
    {
        return;
    }
    if (!outsideInnerRadius)
    {
        return;
    }

    distance = fsqrt(distanceSquared);
    direction = offset;
    vecNormalize(&direction);

    forceMagnitude = shockwave->forceBase * shockwave->expansionRate +
        ship->staticinfo->staticheader.staticCollInfo.collspheresize *
        ship->staticinfo->staticheader.staticCollInfo.collspheresize *
        shockwave->massForceScale * shockwave->expansionRate;

    if (inBounds)
    {
        ship->posinfo.force.x += forceMagnitude * direction.x;
        ship->posinfo.force.y += forceMagnitude * direction.y;
        ship->posinfo.force.z += forceMagnitude * direction.z;

        if (shockwave->type == 5 ||
            (shockwave->type == 4 && ship->staticinfo->staticheader.mass < 700.0f))
        {
            if (shockwave->type == 5 && ship->velocityFreezeTimer.startTime == 0.0f)
            {
                extraDamage = 0.2f * ship->staticinfo->maxhealth;
            }

            shockwaveBeginVelocityFreeze(ship, 1.0f, 1.0f);
            ship->posinfo.force.x = forceMagnitude * direction.x;
            ship->posinfo.force.y = forceMagnitude * direction.y;
            ship->posinfo.force.z = forceMagnitude * direction.z;
            physIntegrateForceToVelocity(ship);
            seekdir();
        }
    }

    ship->rotinfo.torque.x -= 100.0f * direction.x;
    ship->rotinfo.torque.y -= 100.0f * direction.y;
    ship->rotinfo.torque.z -= 100.0f * direction.z;

    damage = extraDamage;
    if (distance > 0.0f)
    {
        damage += shockwave->damageBase +
                  shockwave->damageDistanceScale / distance +
                  shockwave->damageSqrtDistanceScale / fsqrt(distance) +
                  shockwave->damageDistanceSquaredScale / distanceSquared;
    }
    damage *= shockwave->damageScale;

    statusWord = shockwaveShipStatusWord(ship);
    if (shockwave->type == 1)
    {
        *statusWord |= 0x20000u;
    }

    if ((ship->flags & SOF_ForwardDamageToClampHost) == 0 &&
        (shockwave->type != 1 || (*statusWord & 0x10000) == 0))
    {
        ApplyDamageToTarget((SpaceObjRotImpTarg *)ship, damage, 0, 666, shockwave->playerIndex);
    }

    workerCheckResetForHyperspace(ship);
}

static void shockwaveApplyToProjectile(Shockwave *shockwave, Bullet *bullet)
{
    vector offset;
    vector direction;
    real32 distanceSquared;
    real32 innerRadius;
    real32 forceMagnitude;
    vector displacement;

    vecSub(offset, bullet->posinfo.position, shockwave->position);
    distanceSquared = vecMagnitudeSquared(offset);
    if (distanceSquared >= shockwave->currentRadius * shockwave->currentRadius)
    {
        return;
    }

    innerRadius = shockwave->currentRadius - shockwave->radiusStep;
    if (distanceSquared <= innerRadius * innerRadius)
    {
        return;
    }

    direction = offset;
    vecNormalize(&direction);

    forceMagnitude = shockwave->expansionRate * shockwave->forceBase;
    bullet->posinfo.velocity.x += forceMagnitude * direction.x;
    bullet->posinfo.velocity.y += forceMagnitude * direction.y;
    bullet->posinfo.velocity.z += forceMagnitude * direction.z;

    displacement.x = bullet->posinfo.velocity.x * 0.01f * universe.unknown1;
    displacement.y = bullet->posinfo.velocity.y * 0.01f * universe.unknown1;
    displacement.z = bullet->posinfo.velocity.z * 0.01f * universe.unknown1;

    bullet->posinfo.velocity.x = 0.0f;
    bullet->posinfo.velocity.y = 0.0f;
    bullet->posinfo.velocity.z = 0.0f;
    vecAddTo(bullet->posinfo.position, displacement);
}

static real32 shockwaveRandomPolarAngle(void)
{
    real32 radiusFactor;
    real32 angleOffset;

    radiusFactor = (real32)ranRandom(RAN_ParticleStream) * (SHOCKWAVE_RANDOM_POLAR_MAX_FACTOR - 1.0f) * SHOCKWAVE_RANDOM_TO_UNIT_SCALE + 1.0f;
    angleOffset = SHOCKWAVE_HALF_PI / (radiusFactor * radiusFactor);
    if ((real32)ranRandom(RAN_ParticleStream) * SHOCKWAVE_RANDOM_TO_SIGNED_UNIT_SCALE - 1.0f <= 0.0f)
    {
        return SHOCKWAVE_HALF_PI - angleOffset;
    }
    return SHOCKWAVE_HALF_PI + angleOffset;
}

static sdword shockwaveInitRenderSamples(void)
{
    sdword i;

    for (i = 0; i < SHOCKWAVE_RENDER_SAMPLE_COUNT; i++)
    {
        shockwaveRenderSamples[i].radius = 1.0f;
        shockwaveRenderSamples[i].azimuth =
            (real32)ranRandom(RAN_ParticleStream) * SHOCKWAVE_RENDER_POINT_RANDOM_SCALE;
        shockwaveRenderSamples[i].polarAngle = shockwaveRandomPolarAngle();
    }

    for (i = 0; i < SHOCKWAVE_RENDER_SAMPLE_ORDER_COUNT; i++)
    {
        if (SHOCKWAVE_RENDER_POINT_BASE == 1)
        {
            shockwaveRenderSampleOrder[i] = 0;
        }
        else
        {
            shockwaveRenderSampleOrder[i] = ranRandom(RAN_ParticleStream) % (SHOCKWAVE_RENDER_POINT_BASE - 1);
        }
    }

    return 0;
}

static sdword shockwaveRenderSpokes(real32 x, real32 y, real32 z, real32 radius)
{
    sdword i;
    real32 baseAzimuth;
    real32 basePolarAngle;

    if (SHOCKWAVE_SPOKE_SEGMENTS <= 0 || (ranRandom(RAN_ParticleStream) % 10) != 0)
    {
        return 0;
    }

    if (shockwaveLastSpokeRadius == 0.0f)
    {
        baseAzimuth = (real32)ranRandom(RAN_ParticleStream) * SHOCKWAVE_RENDER_POINT_RANDOM_SCALE;
        basePolarAngle = shockwaveRandomPolarAngle();
    }
    else
    {
        baseAzimuth = shockwaveLastSpokeAzimuth;
        basePolarAngle = shockwaveLastSpokePolarAngle;
    }

    for (i = 0; i < 5; i++)
    {
        real32 nextAzimuth = baseAzimuth +
            (real32)ranRandom(RAN_ParticleStream) * (SHOCKWAVE_SPOKE_MAX_AZIMUTH_STEP - SHOCKWAVE_SPOKE_MIN_AZIMUTH_STEP) *
            SHOCKWAVE_RANDOM_TO_UNIT_SCALE + SHOCKWAVE_SPOKE_MIN_AZIMUTH_STEP;
        real32 nextPolarAngle = basePolarAngle +
            (real32)ranRandom(RAN_ParticleStream) * (SHOCKWAVE_SPOKE_MAX_POLAR_STEP - SHOCKWAVE_SPOKE_MIN_POLAR_STEP) *
            SHOCKWAVE_RANDOM_TO_UNIT_SCALE + SHOCKWAVE_SPOKE_MIN_POLAR_STEP;
        real32 sinNextPolar = (real32)sin(nextPolarAngle);
        real32 sinBasePolar = (real32)sin(basePolarAngle);
        real32 x0 = (real32)(cos(baseAzimuth) * radius * sinBasePolar + x);
        real32 y0 = (real32)(sin(baseAzimuth) * radius * sinBasePolar + y);
        real32 z0 = (real32)(cos(basePolarAngle) * radius + z);
        real32 x1 = (real32)(cos(nextAzimuth) * radius * sinNextPolar + x);
        real32 y1 = (real32)(sin(nextAzimuth) * radius * sinNextPolar + y);
        real32 z1 = (real32)(cos(nextPolarAngle) * radius + z);

        rndRenderJaggedLine3(x1, y1, z1, x0, y0, z0, SHOCKWAVE_SPOKE_SEGMENTS, SHOCKWAVE_SPOKE_JITTER_RADIUS);
        baseAzimuth = nextAzimuth;
        basePolarAngle = nextPolarAngle;
    }

    shockwaveLastSpokeRadius = radius;
    shockwaveLastSpokeAzimuth = baseAzimuth;
    shockwaveLastSpokePolarAngle = basePolarAngle;
    return 0;
}

static bool32 shockwaveUpdate(Shockwave *shockwave)
{
    Node *node;
    etgeffectstatic *linkingEffectStat = NULL;
    real32 elapsed;

    if (shockwave->currentRadius >= shockwave->maxRadius)
    {
        if (shockwave->soundHandle != SOUND_NOTINITED)
        {
            soundEventStopSound(shockwave->soundHandle, 0);
            shockwave->soundHandle = SOUND_NOTINITED;
        }
        return FALSE;
    }

    elapsed = universe.totaltimeelapsed - shockwave->activationDelay - shockwave->startTime;
    if (elapsed <= 0.0f)
    {
        return TRUE;
    }

    if (!shockwave->activated)
    {
        shockwave->activated = TRUE;
        shockwave->currentRadius = shockwave->initialRadius;
        shockwaveCreateActivationEffect(shockwave);
    }
    if (shockwaveReloadedFromSave == 1)
    {
        shockwaveCreateActivationEffect(shockwave);
    }

    shockwave->currentRadius = elapsed * shockwave->expansionRate + shockwave->initialRadius;
    telldir();

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = listGetStructOfNode(node);
        bool32 eligible = TRUE;

        if (shockwave->type == 5)
        {
            if (ship->shiprace == 131 ||
                ship->shiprace == 132 ||
                ship->shiprace == 109 ||
                ship->tacticstype == 1 ||
                (singlePlayerGame && shockwaveSinglePlayerExemptType5Ship(ship)))
            {
                eligible = FALSE;
            }
        }
        if (shockwave->type == 4)
        {
            if (univIsObjFriendly((SpaceObj *)ship, &universe.players[shockwave->playerIndex]) ||
                ship->shiptype == 131 ||
                ship->shiptype == 132 ||
                ship->shiptype == 109)
            {
                eligible = FALSE;
            }
        }

        if (!shipIsBeastShip(ship) && eligible)
        {
            shockwaveApplyToShip(shockwave, ship);
        }
    }

    telldir();
    for (node = shockwaveProjectileList.head; node != NULL; node = node->next)
    {
        ShockwaveProjectile *shockwaveProjectile = listGetStructOfNode(node);
        shockwaveApplyToProjectile(shockwave, (Bullet *)shockwaveProjectile->projectile);
    }

    if (shockwave->type == 4 || shockwave->type == 5)
    {
        if (9 == 1 || (9 - 1) <= 0 || (ranRandom(RAN_ParticleStream) % (9 - 1)) == 0)
        {
            real32 azimuth = (real32)ranRandom(RAN_ParticleStream) * SHOCKWAVE_RENDER_POINT_RANDOM_SCALE;
            real32 polarAngle = shockwaveRandomPolarAngle();
            real32 sinPolar = (real32)sin(polarAngle);
            vector position;
            vector velocity;

            position.x = (real32)(cos(azimuth) * shockwave->currentRadius * sinPolar + shockwave->position.x);
            position.y = (real32)(sin(azimuth) * shockwave->currentRadius * sinPolar + shockwave->position.y);
            position.z = (real32)(cos(polarAngle) * shockwave->currentRadius + shockwave->position.z);
            velocity.x = shockwave->expansionRate * (real32)(cos(azimuth) * sinPolar);
            velocity.y = shockwave->expansionRate * (real32)(sin(azimuth) * sinPolar);
            velocity.z = shockwave->expansionRate * (real32)cos(polarAngle);

            linkingEffectStat = shockwaveGetLinkingEffectStat();
            if (!universePause && linkingEffectStat != NULL && !etgFrequencyExceeded(linkingEffectStat))
            {
                matrix effectCoordsys = defaultshipmatrix;
                etgEffectCreate(linkingEffectStat, NULL, &position, &velocity, &effectCoordsys, 1.0f, 120, 0);
            }
        }
    }

    soundEventUpdateShockwaveLoop(shockwave);
    return TRUE;
}

Shockwave *shockwaveCreateTyped(sdword shockwaveType, udword x, udword y, udword z, uword playerIndex)
{
    ShockwaveState *shockwaveState = shockwaveCreateFromTemplate(cataShockwaveTemplates[shockwaveType]);

    shockwaveState->shockwave.position.x = *(real32 *)&x;
    shockwaveState->shockwave.position.y = *(real32 *)&y;
    shockwaveState->shockwave.position.z = *(real32 *)&z;
    shockwaveState->shockwave.startTime = universe.totaltimeelapsed;
    shockwaveState->shockwave.playerIndex = playerIndex;
    shockwaveState->shockwave.activated = FALSE;
    return &shockwaveState->shockwave;
}

void shockwaveCreateBasic(real32 x, real32 y, real32 z)
{
    ShockwaveState *shockwaveState = shockwaveCreateFromTemplate(cataShockwaveTemplates[0]);

    shockwaveState->shockwave.position.x = x;
    shockwaveState->shockwave.position.y = y;
    shockwaveState->shockwave.position.z = z;
    shockwaveState->shockwave.startTime = universe.totaltimeelapsed;
    shockwaveState->shockwave.playerIndex = 0;
    shockwaveState->shockwave.activated = FALSE;
}

Shockwave *shockwaveCreateProjectileImpact(real32 x, real32 y, real32 z, uword playerIndex, real32 damageScale)
{
    Node *node;
    ShockwaveState *shockwaveState = shockwaveCreateFromTemplate(singlePlayerGame ? cataShockwaveTemplates[1] : cataShockwaveTemplates[0]);

    shockwaveState->shockwave.position.x = x;
    shockwaveState->shockwave.position.y = y;
    shockwaveState->shockwave.position.z = z;
    shockwaveState->shockwave.startTime = universe.totaltimeelapsed;
    shockwaveState->shockwave.playerIndex = playerIndex;
    shockwaveState->shockwave.activated = FALSE;
    shockwaveState->shockwave.damageScale = damageScale;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = listGetStructOfNode(node);

        if (ship->shiptype == bDFGFrigate &&
            shockwaveDistanceSquared(&shockwaveState->shockwave.position, &ship->posinfo.position) <=
                shockwaveState->shockwave.initialRadius * shockwaveState->shockwave.initialRadius)
        {
            shockwaveState->shockwave.damageScale *= SHOCKWAVE_PROJECTILE_NEARBY_SCALE;
        }
    }

    return &shockwaveState->shockwave;
}

static real32 shockwaveProjectileFalloffScale(real32 travelledDistance)
{
    real32 damageScale = 1.0f;
    real32 falloffMax = singlePlayerGame ? SHOCKWAVE_PROJECTILE_SP_FALLOFF_MAX : SHOCKWAVE_PROJECTILE_MP_FALLOFF_MAX;
    real32 falloffMin = singlePlayerGame ? SHOCKWAVE_PROJECTILE_SP_FALLOFF_MIN : SHOCKWAVE_PROJECTILE_MP_FALLOFF_MIN;

    if (travelledDistance > falloffMin)
    {
        real32 distancePastMin = travelledDistance - falloffMin;
        real32 falloffSpan = falloffMax - falloffMin;

        if (distancePastMin > falloffSpan)
        {
            distancePastMin = falloffSpan;
        }
        if (falloffSpan > 0.0f)
        {
            damageScale = 1.0f - distancePastMin / falloffSpan;
            if (damageScale < 0.0f)
            {
                damageScale = 0.0f;
            }
        }
    }

    if (damageScale < SHOCKWAVE_PROJECTILE_MIN_DAMAGE_SCALE)
    {
        damageScale = SHOCKWAVE_PROJECTILE_MIN_DAMAGE_SCALE;
    }
    return damageScale;
}

static void naggarokClearProjectileRef(void)
{
    Node *node;
    Ship *naggarok = NULL;

    if (!singlePlayerGame)
    {
        return;
    }

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = listGetStructOfNode(node);

        if (shipIsNaggarok(ship))
        {
            naggarok = ship;
        }
    }

    if (naggarok != NULL)
    {
        *(SpaceObjRotImpTarg **)((ubyte *)naggarok->ShipSpecifics + naggarokProjectileRefOffset) = NULL;
    }
}

sdword shockwaveProjectileHitStationary(Bullet *projectile)
{
    naggarokClearProjectileRef();
    singlePlayerUnregisterShockwaveProjectile((SpaceObjRotImpTarg *)projectile);
    shockwaveCreateProjectileImpact(projectile->posinfo.position.x,
                                    projectile->posinfo.position.y,
                                    projectile->posinfo.position.z,
                                    projectile->playerowner->playerIndex,
                                    shockwaveProjectileFalloffScale(TreatAsReal32(projectile->cataUnknown150[1])));
    return soundEventPlay(&projectile->posinfo.position, 4112, 0);
}

sdword shockwaveProjectileBulletHit(vector *position, Bullet *projectile)
{
    naggarokClearProjectileRef();
    singlePlayerUnregisterShockwaveProjectile((SpaceObjRotImpTarg *)projectile);
    shockwaveCreateProjectileImpact(position->x,
                                    position->y,
                                    position->z,
                                    projectile->playerowner->playerIndex,
                                    shockwaveProjectileFalloffScale(TreatAsReal32(projectile->cataUnknown150[1])));
    return soundEventPlay(position, 4112, 0);
}

// Decompiled from cataclysm.exe @ 0x516D40
bool32 shockwaveHasActiveShockwaves(void)
{
    return shockwaveList.num > 0;
}

void SaveShockwaves(void)
{
    SaveLinkedListOfStuff(&shockwaveList, SaveShockwaveCB);
}

void LoadShockwaves(void)
{
    Node *node;

    shockwaveShutdown();
    shockwaveStartup();
    LoadLinkedListOfStuff(&shockwaveList, LoadShockwaveCB);
    shockwaveReloadedFromSave = 1;

    for (node = universe.BulletList.head; node != NULL; node = node->next)
    {
        SpaceObjRotImpTarg *projectile = listGetStructOfNode(node);
        if (bulletIsShockwaveProjectile(projectile))
        {
            ShockwaveProjectile *shockwaveProjectile = memAlloc(sizeof(ShockwaveProjectile), "ShockwaveProjectile", MBF_SmallBlockHeap);
            shockwaveProjectile->projectile = projectile;
            listAddNode(&shockwaveProjectileList, &shockwaveProjectile->node, shockwaveProjectile);
        }
    }
}

void shockwaveRegisterProjectile(SpaceObjRotImpTarg *projectile)
{
    ShockwaveProjectile *shockwaveProjectile;

    if (!bulletIsShockwaveProjectile(projectile))
    {
        return;
    }

    dbgAssert(shockwaveStarted);

    shockwaveProjectile = memAlloc(sizeof(ShockwaveProjectile), "ShockwaveProjectile", MBF_SmallBlockHeap);
    shockwaveProjectile->projectile = projectile;
    listAddNode(&shockwaveProjectileList, &shockwaveProjectile->node, shockwaveProjectile);
}

void shockwaveUnregisterProjectile(SpaceObjRotImpTarg *projectile)
{
    Node *node;
    Node *next;

    if (!bulletIsShockwaveProjectile(projectile) || !shockwaveStarted)
    {
        return;
    }

    for (node = shockwaveProjectileList.head; node != NULL; node = next)
    {
        ShockwaveProjectile *shockwaveProjectile = listGetStructOfNode(node);

        next = node->next;
        if (shockwaveProjectile->projectile == projectile)
        {
            listDeleteNode(node);
        }
    }
}

void shockwaveUpdateAll(void)
{
    Node *node;
    Node *next;

    for (node = shockwaveList.head; node != NULL; node = next)
    {
        ShockwaveState *shockwaveState = listGetStructOfNode(node);

        next = node->next;
        if (!shockwaveUpdate(&shockwaveState->shockwave))
        {
            listRemoveNode(node);
        }
    }

    if (shockwaveReloadedFromSave == 1)
    {
        shockwaveReloadedFromSave = 0;
    }
}

sdword shockwaveStartup(void)
{
    listInit(&shockwaveList);
    listInit(&shockwaveProjectileList);
    shockwaveReloadedFromSave = 0;
    shockwaveLastSpokeRadius = 0.0f;
    shockwaveLastSpokeAzimuth = 0.0f;
    shockwaveLastSpokePolarAngle = 0.0f;
    shockwaveStarted = TRUE;
    return shockwaveInitRenderSamples();
}

void shockwaveShutdown(void)
{
    listDeleteAll(&shockwaveProjectileList);
    listDeleteAll(&shockwaveList);
    shockwaveReloadedFromSave = 0;
    shockwaveStarted = FALSE;
}

sdword shockwaveRenderAll(void)
{
    Node *node;

    for (node = shockwaveList.head; node != NULL; node = node->next)
    {
        ShockwaveState *shockwaveState = listGetStructOfNode(node);
        Shockwave *shockwave = &shockwaveState->shockwave;

        if (!shockwave->activated)
        {
            continue;
        }
        if (shockwave->type == 4 || shockwave->type == 5)
        {
            sdword textureEnabled = rndTextureEnable(FALSE);
            sdword lightingEnabled = rndLightingEnable(FALSE);
            real32 radiusFraction = shockwave->currentRadius / shockwave->maxRadius;
            sdword pointCount = SHOCKWAVE_RENDER_POINT_BASE +
                (sdword)(SHOCKWAVE_RENDER_POINT_BASE * SHOCKWAVE_RENDER_POINT_GROWTH_SCALE * radiusFraction +
                         SHOCKWAVE_RENDER_POINT_COUNT_ROUND);
            sdword i;

            glColor3f(1.0f, 1.0f, 1.0f);
            shockwaveRenderSpokes(shockwave->position.x, shockwave->position.y, shockwave->position.z, shockwave->currentRadius);

            if (pointCount > SHOCKWAVE_RENDER_SAMPLE_COUNT)
            {
                pointCount = SHOCKWAVE_RENDER_SAMPLE_COUNT;
            }
            if (radiusFraction > SHOCKWAVE_RENDER_POINT_FADE_START)
            {
                pointCount = (sdword)((1.0f - (radiusFraction - SHOCKWAVE_RENDER_POINT_FADE_START) /
                    (1.0f - SHOCKWAVE_RENDER_POINT_FADE_START)) * pointCount);
            }

            for (i = 0; i < pointCount; i++)
            {
                ShockwaveRenderSample *sample = &shockwaveRenderSamples[i];
                real32 sinPolarAngle;
                real32 colorValue;
                real32 pointX;
                real32 pointY;
                real32 pointZ;

                sample->radius = shockwave->currentRadius;
                sample->azimuth += SHOCKWAVE_RENDER_POINT_ROTATION_SCALE * sample->polarAngle;
                sinPolarAngle = (real32)sin(sample->polarAngle);
                pointX = (real32)(cos(sample->azimuth) * sample->radius * sinPolarAngle + shockwave->position.x);
                pointY = (real32)(sin(sample->azimuth) * sample->radius * sinPolarAngle + shockwave->position.y);
                pointZ = (real32)(cos(sample->polarAngle) * sample->radius + shockwave->position.z);
                colorValue = (real32)ranRandom(RAN_ParticleStream) *
                    (1.0f - SHOCKWAVE_RENDER_POINT_MIN_COLOR) * SHOCKWAVE_RANDOM_TO_UNIT_SCALE +
                    SHOCKWAVE_RENDER_POINT_MIN_COLOR;

                glBegin(GL_POINTS);
                glColor3f(colorValue, 1.0f, colorValue);
                glVertex3f(pointX, pointY, pointZ);
                glEnd();
            }

            rndTextureEnable(textureEnabled);
            rndLightingEnable(lightingEnabled);
        }
    }

    return 0;
}
