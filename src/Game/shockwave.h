#ifndef ___SHOCKWAVE_H
#define ___SHOCKWAVE_H

#include "types.h"
#include "spaceobj.h"
#include "vector.h"

typedef struct Shockwave
{
    sdword type;
    sdword activationEffectType;
    vector position;
    real32 damageBase;
    real32 damageDistanceScale;
    real32 damageSqrtDistanceScale;
    real32 damageDistanceSquaredScale;
    real32 forceBase;
    real32 massForceScale;
    real32 expansionRate;
    real32 startTime;
    real32 activationDelay;
    real32 radiusStep;
    real32 currentRadius;
    real32 initialRadius;
    real32 maxRadius;
    udword unk48;
    sdword playerIndex;
    ubyte activated;
    ubyte activatedPad[3];
    udword runtimeState;
    sdword soundHandle;
    sdword soundEvent;
    real32 damageScale;
} Shockwave;

static_assert(sizeof(Shockwave) == 100, "Shockwave size mismatch");

sdword shockwaveProjectileHitStationary(Bullet *projectile);
sdword shockwaveProjectileBulletHit(vector *position, Bullet *projectile);

#endif
