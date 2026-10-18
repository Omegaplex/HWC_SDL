// SPDX-License-Identifier: MIT
/*=============================================================================
    Name    : RepairDroid.c
    Purpose : Repair droid helpers
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "glinc.h"

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "memory.h"
#include "RepairDroid.h"
#include "RepairCorvette.h"
#include "AIShip.h"
#include "AITrack.h"
#include "CommandLayer.h"
#include "ClassDefs.h"
#include "collision.h"
#include "Ship.h"
#include "universe.h"
#include "etg.h"
#include "matrix.h"
#include "statscript.h"
#include "dock.h"
#include "Randy.h"
#include "RammingFrigate.h"
#include "UnivUpdate.h"
#include "mesh.h"
#include "LOD.h"
#include "objtypes.h"
#include "ShipDefs.h"
#include "FastMath.h"
#include "ACV.h"
#include <math.h>
#include <string.h>

void telldir(void);

typedef struct RepairDroidEntry
{
    SpaceObj     *repairDroid;    // 0
    ubyte         state;          // 4 (byte 0 of DWORD)
    ubyte         pad[3];
    vector        targetPos;      // 8
    sdword        jointIndexA;    // 20
    sdword        jointIndexB;    // 24
    sdword        prevJointIndexA;// 28
    real32        timestamp1;     // 32
    real32        timestamp2;     // 36
    real32        unk10;          // 40
} RepairDroidEntry;

static_assert(sizeof(RepairDroidEntry) == 44);
static_assert(offsetof(RepairDroidEntry, repairDroid) == 0);
static_assert(offsetof(RepairDroidEntry, state) == 4);
static_assert(offsetof(RepairDroidEntry, targetPos) == 8);
static_assert(offsetof(RepairDroidEntry, jointIndexA) == 20);
static_assert(offsetof(RepairDroidEntry, jointIndexB) == 24);
static_assert(offsetof(RepairDroidEntry, prevJointIndexA) == 28);
static_assert(offsetof(RepairDroidEntry, timestamp1) == 32);
static_assert(offsetof(RepairDroidEntry, timestamp2) == 36);
static_assert(offsetof(RepairDroidEntry, unk10) == 40);

struct RepairDroidController
{
    RepairDroidEntry *entries;    // 0
    sdword            count;      // 4
    Ship             *parentShip; // 8
    sdword            cycleIndex; // 12
    real32            nextEventTime; // 16
    real32            outputHealth;  // 20
};

RepairDroidController *CreateRepairDroid(Ship *parentShip, sdword count)
{
    vector position = { 0.0f, 0.0f, 0.0f };
    ShipRace parentShipRace = parentShip->shiprace;
    ShipStaticInfo *repairDroidStatic;
    RepairDroidController *controller;
    sdword i;

    dbgAssert(parentShip != NULL);
    dbgAssert(count > 0);

    if (parentShip == NULL || count <= 0)
    {
        return NULL;
    }

    switch (parentShipRace)
    {
        case RACE_Sect:
        case RACE_Kushan:
        case RACE_Taiidan:
        case RACE_P1:
        case RACE_Traders:
        case RACE_Derelict:
            repairDroidStatic = GetShipStaticInfo(sRepairDroid, parentShipRace);
            break;
        case RACE_Beast:
            telldir();
            return NULL;
        default:
            telldir();
            return NULL;
    }

    controller = memAlloc(sizeof(*controller), "RepairDroidCtrl", NonVolatile);
    if (controller == NULL)
    {
        return NULL;
    }

    controller->entries = NULL;
    controller->count = 0;
    controller->parentShip = NULL;
    controller->cycleIndex = -1;
    controller->nextEventTime = -1.0f;
    controller->outputHealth = parentShip->health;

    controller->entries = memAlloc(sizeof(*controller->entries) * count, "RepairDroidEntries", NonVolatile);
    if (controller->entries == NULL)
    {
        memFree(controller);
        return NULL;
    }

    controller->count = count;
    controller->parentShip = parentShip;

    for (i = 0; i < count; i++)
    {
        controller->entries[i].repairDroid = NULL;
        controller->entries[i].state = 0;
        controller->entries[i].targetPos = position;
        controller->entries[i].jointIndexA = 0;
        controller->entries[i].jointIndexB = 0;
        controller->entries[i].timestamp1 = -1.0f;
        controller->entries[i].timestamp2 = -1.0f;
    }

    controller->entries[0].repairDroid = memAlloc(sizeof(Ship) * count, "RepairDroidShips", NonVolatile);
    if (controller->entries[0].repairDroid == NULL)
    {
        memFree(controller->entries);
        memFree(controller);
        return NULL;
    }

    for (i = 1; i < count; i++)
    {
        controller->entries[i].repairDroid =
            (SpaceObj *)((ubyte *)controller->entries[0].repairDroid + sizeof(Ship) * i);
    }

    for (i = 0; i < count; i++)
    {
        Ship *repairDroid = (Ship *)controller->entries[i].repairDroid;
        repairDroid->currentLOD = 0;
        repairDroid->staticinfo = (StaticInfo *)repairDroidStatic;
        repairDroid->flags |= SOF_Rotatable;
        repairDroid->objtype = OBJ_ShipType;
        repairDroid->shiptype = sRepairDroid;
        repairDroid->shiprace = parentShipRace;
        repairDroid->tacticstype = Neutral;
        repairDroid->tactics_ordertype = 0;
        repairDroid->collMyBlob = parentShip->collMyBlob;
        univInitSpaceObjPosRot((SpaceObj *)repairDroid, &position, FALSE);
    }

    return controller;
}

static hmatrix repairDroidRenderHMatrix;

etglod *repairDroidHitEffects = NULL;

static real32 repairDroidBlendWeight    = 0.2f;
static real32 repairDroidCurrentWeight  = 1.0f;

#define BeastSelfDamageTick  20.0f
#define ZERO_POINT_FIVE      0.5f
#define RADIAN_PER_DEG       0.017453f
#define repairAlignDotMin    0.86602f
#define repairVelocityDamp   0.7f
#define repairTimingSmall    0.01f
#define repairTimingBig      2.0f
#define repairRangeFar       90.0f
#define repairRangeMid       50.0f
#define repairRangeNear      25.0f
#define repairSpeedThreshold 0.1f
#define repairEffectInterval 0.02f
#define REPAIR_SPEECH_DELAY  2.5f

void startRepairEffect(Ship *ship, SpaceObjRotImpTarg *target, vector *trajectory, real32 distance);
void ModifyRepairEffect(Effect *effect, Ship *ship, vector *trajectory, real32 distance, SpaceObjRotImpTarg *target);

static void repairDroidStopBeamEffect(Ship *ship)
{
    if (ship->rceffect != NULL)
    {
        stopRepairEffect(ship);
    }
}

static bool32 repairDroidTargetsAllHealthy(SelectAnyCommand *targets)
{
    sdword i;

    for (i = 0; i < targets->numTargets; i++)
    {
        Ship *target = (Ship *)targets->TargetPtr[i];

        if (target->health != target->staticinfo->maxhealth)
        {
            return FALSE;
        }
    }

    return TRUE;
}

bool32 repairDroidAIUpdate(Ship *ship, SelectAnyCommand *targets, real32 rangeToRefuel)
{
    SelectCommand selectOne;
    sdword i;
    Ship *targetShip;
    SpaceObjRotImpTarg *target;
    SpaceObjRotImpTarg *targetTemp;
    vector trajectory;
    vector curheading;
    real32 range;
    real32 dotprod;
    real32 repairStartRange;
    real32 repairStopRange;
    CommandToDo *targetCommand;

    for (i = 0; i < targets->numTargets;)
    {
        target = targets->TargetPtr[i];
        if (target->objtype != OBJ_ShipType)
        {
            targets->numTargets--;
            targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            continue;
        }

        targetShip = (Ship *)target;
        targetCommand = getShipAndItsCommand(&universe.mainCommandLayer, targetShip);
        if (targetCommand != NULL &&
            targetCommand->ordertype.order == COMMAND_DOCK &&
            shipDockingCollisionTarget(targetShip) != NULL)
        {
            targets->numTargets--;
            targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
            continue;
        }

        if ((((targetShip->staticinfo->shipclass == CLASS_Fighter) ||
              (targetShip->staticinfo->shipclass == CLASS_Corvette)) &&
             (targetShip->playerowner == ship->playerowner)) ||
            ((ship->staticinfo->repairTime != 0.0f) &&
             (targetShip->playerowner == ship->playerowner)))
        {
            i++;
            continue;
        }

        targets->numTargets--;
        targets->TargetPtr[i] = targets->TargetPtr[targets->numTargets];
    }

    if (targets->numTargets <= 0)
    {
        return TRUE;
    }

    target = targets->TargetPtr[0];
    targetShip = (Ship *)target;
    if (targetShip == ship)
    {
        return FALSE;
    }

    if ((ship->staticinfo->repairTime == 0.0f) ||
        (targetShip->staticinfo->shipclass == CLASS_Fighter))
    {
        aishipGetTrajectory(ship, target, &trajectory);
        aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position,
                                         AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible | AISHIP_FirstPointInDirectionFlying,
                                         0.0f, NULL);
        if (RangeToTarget(ship, target, &trajectory) <= rangeToRefuel)
        {
            selectOne.numShips = 1;
            selectOne.ShipPtr[0] = targetShip;
            if (target->objtype == OBJ_ShipType)
            {
                targetShip->fuel += 300.0f;
                if (targetShip->fuel > targetShip->staticinfo->maxfuel)
                {
                    targetShip->fuel = targetShip->staticinfo->maxfuel;
                }
                clDock(&universe.mainCommandLayer, &selectOne, DOCK_AT_SPECIFIC_SHIP, ship);
            }
            targets->numTargets--;
            if (targets->numTargets <= 0)
            {
                return TRUE;
            }
            targets->TargetPtr[0] = targets->TargetPtr[targets->numTargets];
        }
        return FALSE;
    }

    aishipGetTrajectory(ship, target, &trajectory);
    range = RangeToTarget(ship, target, &trajectory);
    vecNormalize(&trajectory);
    aitrackHeadingFunc((SpaceObjRotImpTargGuidance *)ship, &trajectory, 0.999f, 0.999f, 0.0f, 0);

    repairStartRange = ship->staticinfo->repairCombatDamage;
    repairStopRange = ship->staticinfo->repairCombatTime;
    if (targetShip->staticinfo->shipclass == CLASS_Mothership)
    {
        repairStartRange *= 2.0f;
        repairStopRange *= 2.0f;
    }

    if (range <= repairStartRange)
    {
        aitrackZeroVelocity(ship);
    }
    else
    {
        aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position,
                                         AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible,
                                         0.0f, NULL);
        if (range > repairStopRange)
        {
            repairDroidStopBeamEffect(ship);
            return FALSE;
        }
    }

    matGetVectFromMatrixCol3(curheading, ship->rotinfo.coordsys);
    dotprod = vecDotProduct(trajectory, curheading);
    if (repairDroidTargetsAllHealthy(targets) ||
        dotprod <= ship->staticinfo->pumpFuelRate)
    {
        repairDroidStopBeamEffect(ship);
        return FALSE;
    }

    if (ship->rceffect != NULL)
    {
        ModifyRepairEffect(ship->rceffect, ship, &trajectory, range, target);
    }
    else
    {
        startRepairEffect(ship, target, &trajectory, range);
    }

    if (!bitTest(targetShip->flags, SOF_NISShip))
    {
        targetShip->health += universe.unknown1 * ship->staticinfo->repairDamage;
    }
    if (targetShip->health <= targetShip->staticinfo->maxhealth)
    {
        return FALSE;
    }

    repairDroidStopBeamEffect(ship);
    targetShip->health = targetShip->staticinfo->maxhealth;
    targetTemp = targets->TargetPtr[0];
    for (i = 0; i < targets->numTargets - 1; i++)
    {
        targets->TargetPtr[i] = targets->TargetPtr[i + 1];
    }
    targets->TargetPtr[targets->numTargets - 1] = targetTemp;
    return FALSE;
}

void deleteRepairDroidData(RepairDroidController *controller)
{
    if (controller != NULL)
    {
        if (controller->entries != NULL)
        {
            if (controller->entries[0].repairDroid != NULL)
            {
                memFree(controller->entries[0].repairDroid);
            }

            memFree(controller->entries);
        }

        memFree(controller);
    }
}


// Decompiled from cataclysm.exe @ 0x5C29E0
static sdword repairDroidAnimStep(udword *a1, sdword *a2, udword *a3)
{
    sdword nVertices = (sdword)a1[2];
    sdword nPolygons = (sdword)a1[5];
    sdword result;
    sdword v13 = 0;
    uword *polyEntry;

    if ((sdword)*a3 >= nVertices)
        *a3 = (sdword)*a3 % nVertices;

    result = nPolygons;
    if (result < 0)
    {
    reset:
        *a2 = 0;
        *a3 = 0;
        return result;
    }

    while (1)
    {
        sdword v6 = *a2 + 1;
        *a2 = v6;
        sdword v7 = (sdword)a1[5];
        if (v6 >= v7)
            *a2 = v6 % v7;
        polyEntry = (uword *)(a1[9] + 40 * *a2);
        udword rnd = ranRandom(0) & 3;
        if (!rnd)
            break;
        udword r1 = rnd - 1;
        if (r1)
        {
            if (r1 == 1)
            {
                result = *a3;
                if ((uword)*a3 == polyEntry[2])
                    goto set_v4;
                if ((uword)result == polyEntry[3])
                {
                    *a3 = polyEntry[2];
                    return result;
                }
                if ((uword)result == polyEntry[4])
                {
                    *a3 = polyEntry[3];
                    return result;
                }
            }
            else if (nVertices > 0)
            {
                *a3 = ranRandom(0) % (udword)nVertices;
            }
            else
            {
                *a3 = 0;
            }
        }
        else
        {
            result = *a3;
            if ((uword)*a3 == polyEntry[2])
            {
            set_v4:
                result = polyEntry[4];
                *a3 = result;
                return result;
            }
            if ((uword)result == polyEntry[3])
            {
                *a3 = polyEntry[3];
                return result;
            }
            if ((uword)result == polyEntry[4])
            {
            set_v2:
                *a3 = polyEntry[2];
                return result;
            }
        }
    next:
        result = ++v13;
        if (v13 > (sdword)a1[5])
            goto reset;
    }
    result = *a3;
    if ((uword)*a3 == polyEntry[2])
    {
        result = polyEntry[3];
        *a3 = result;
        return result;
    }
    if ((uword)result != polyEntry[3])
    {
        if ((uword)result == polyEntry[4])
            goto set_v2;
        goto next;
    }
    *a3 = polyEntry[4];
    return result;
}

#if 0  // Cat2: repairDroidAnimStep live code differs from binary or raw IDA offsets
/*=== repairDroidAnimStep (0x5c29e0) ===*/
int __cdecl repairDroidAnimStep(udword *a1, int *a2, udword *a3)
{
  int v4; // ecx
  int result; // eax
  int v6; // esi
  int v7; // ecx
  unsigned __int16 *v8; // esi
  udword v9; // eax
  udword v10; // eax
  char *v11; // [esp+0h] [ebp-10h]
  sdword v12; // [esp+4h] [ebp-Ch]
  int v13; // [esp+14h] [ebp+4h]

  v4 = a1[2];
  if ( (int)*a3 >= v4 )
    *a3 = (int)*a3 % v4;
  result = a1[5];
  v13 = 0;
  if ( result < 0 )
  {
LABEL_24:
    *a2 = 0;
    *a3 = 0;
    return result;
  }
  while ( 1 )
  {
    v6 = *a2 + 1;
    *a2 = v6;
    v7 = a1[5];
    if ( v6 >= v7 )
      *a2 = v6 % v7;
    v8 = (unsigned __int16 *)(a1[9] + 40 * *a2);
    v9 = ranRandom(0) & 3;
    if ( !v9 )
      break;
    v10 = v9 - 1;
    if ( v10 )
    {
      if ( v10 == 1 )
      {
        result = *a3;
        if ( *a3 == v8[2] )
          goto LABEL_25;
        if ( result == v8[3] )
        {
          *a3 = v8[2];
          return result;
        }
        if ( result == v8[4] )
        {
          *a3 = v8[3];
          return result;
        }
      }
      else if ( (int)a1[2] > 0 )
      {
        *a3 = ranRandom(0) % a1[2];
      }
      else
      {
        *a3 = 0;
      }
    }
    else
    {
      result = *a3;
      if ( *a3 == v8[2] )
      {
LABEL_25:
        result = v8[4];
        *a3 = result;
        return result;
      }
      if ( result == v8[3] )
      {
        *a3 = v8[3];
        return result;
      }
      if ( result == v8[4] )
      {
LABEL_29:
        *a3 = v8[2];
        return result;
      }
    }
LABEL_23:
    result = ++v13;
    if ( v13 > a1[5] )
      goto LABEL_24;
  }
  result = *a3;
  if ( *a3 == v8[2] )
  {
    result = v8[3];
    *a3 = result;
    return result;
  }
  if ( result != v8[3] )
  {
    if ( result == v8[4] )
      goto LABEL_29;
    goto LABEL_23;
  }
  *a3 = v8[4];
  return result;
}
#endif

// Decompiled from cataclysm.exe @ 0x5C2B80
static sdword repairDroidFindBestJoint(udword *a1, float *a2, sdword *a3, sdword *a4)
{
    udword *v4 = a1;
    sdword nVertices = (sdword)a1[2];
    sdword nPolygons = (sdword)a1[5];
    real32 bestDot = -2.0f;
    sdword result;
    sdword v9;
    sdword v32 = 0;
    uword *polyEntry;
    float *vertList = (float *)a1[6];

    if (*a4 >= nVertices)
        *a4 %= nVertices;
    if (*a3 >= nPolygons)
        *a3 %= nPolygons;

    result = *a3;
    v9 = *a4;
    --*a3;
    sdword v34 = result;

    if (nPolygons < 0)
    {
        *a4 = v9;
        *a3 = result;
    }
    else
    {
        do
        {
            sdword v10 = *a3 + 1;
            *a3 = v10;
            sdword v11 = (sdword)v4[5];
            if (v10 >= v11)
                *a3 = v10 % v11;
            polyEntry = (uword *)(v4[9] + 40 * *a3);

            if (*a4 == polyEntry[2])
            {
                float *v14 = (float *)(vertList + 4 * (sdword)*a4);
                vector edge;
                edge.x = *(float *)(vertList + 4 * (sdword)polyEntry[3]) - *v14;
                edge.y = *(float *)(vertList + 4 * (sdword)polyEntry[3] + 1) - v14[1];
                edge.z = *(float *)(vertList + 4 * (sdword)polyEntry[3] + 2) - v14[2];
                vecNormalize(&edge);
                real32 dot = edge.y * a2[1] + edge.x * a2[0] + edge.z * a2[2];
                if (dot >= bestDot)
                {
                    bestDot = dot;
                    v9 = polyEntry[3];
                    v34 = *a3;
                }
                v14 = (float *)(vertList + 4 * (sdword)*a4);
                edge.x = *(float *)(vertList + 4 * (sdword)polyEntry[4]) - *v14;
                edge.y = *(float *)(vertList + 4 * (sdword)polyEntry[4] + 1) - v14[1];
                edge.z = *(float *)(vertList + 4 * (sdword)polyEntry[4] + 2) - v14[2];
                vecNormalize(&edge);
                dot = edge.y * a2[1] + edge.x * a2[0] + edge.z * a2[2];
                if (dot >= bestDot)
                {
                    bestDot = dot;
                    v9 = polyEntry[4];
                    v34 = *a3;
                }
                v4 = a1;
            }
            if (*a4 == polyEntry[3])
            {
                float *v20 = (float *)(vertList + 4 * (sdword)*a4);
                vector edge;
                edge.x = *(float *)(vertList + 4 * (sdword)polyEntry[2]) - *v20;
                edge.y = *(float *)(vertList + 4 * (sdword)polyEntry[2] + 1) - v20[1];
                edge.z = *(float *)(vertList + 4 * (sdword)polyEntry[2] + 2) - v20[2];
                vecNormalize(&edge);
                real32 dot = edge.y * a2[1] + edge.x * a2[0] + edge.z * a2[2];
                if (dot >= bestDot)
                {
                    bestDot = dot;
                    v9 = polyEntry[2];
                    v34 = *a3;
                }
                float *v23 = (float *)(vertList + 4 * (sdword)*a4);
                edge.x = *(float *)(vertList + 4 * (sdword)polyEntry[4]) - *v23;
                edge.y = *(float *)(vertList + 4 * (sdword)polyEntry[4] + 1) - v23[1];
                edge.z = *(float *)(vertList + 4 * (sdword)polyEntry[4] + 2) - v23[2];
                vecNormalize(&edge);
                dot = edge.y * a2[1] + edge.x * a2[0] + edge.z * a2[2];
                if (dot >= bestDot)
                {
                    bestDot = dot;
                    v9 = polyEntry[4];
                    v34 = *a3;
                }
                v4 = a1;
            }
            if (*a4 == polyEntry[4])
            {
                float *v26 = (float *)(vertList + 4 * (sdword)*a4);
                vector edge;
                edge.x = *(float *)(vertList + 4 * (sdword)polyEntry[2]) - *v26;
                edge.y = *(float *)(vertList + 4 * (sdword)polyEntry[2] + 1) - v26[1];
                edge.z = *(float *)(vertList + 4 * (sdword)polyEntry[2] + 2) - v26[2];
                vecNormalize(&edge);
                real32 dot = edge.y * a2[1] + edge.x * a2[0] + edge.z * a2[2];
                if (dot >= bestDot)
                {
                    bestDot = dot;
                    v9 = polyEntry[2];
                    v34 = *a3;
                }
                float *v29 = (float *)(vertList + 4 * (sdword)*a4);
                edge.x = *(float *)(vertList + 4 * (sdword)polyEntry[3]) - *v29;
                edge.y = *(float *)(vertList + 4 * (sdword)polyEntry[3] + 1) - v29[1];
                edge.z = *(float *)(vertList + 4 * (sdword)polyEntry[3] + 2) - v29[2];
                vecNormalize(&edge);
                dot = edge.y * a2[1] + edge.x * a2[0] + edge.z * a2[2];
                if (dot >= bestDot)
                {
                    bestDot = dot;
                    v9 = polyEntry[3];
                    v34 = *a3;
                }
                v4 = a1;
            }
            ++v32;
        }
        while (v32 <= (sdword)v4[5]);
        result = v34;
        *a4 = v9;
        *a3 = v34;
    }
    return result;
}

#if 0  // Cat2: repairDroidFindBestJoint live code differs from binary or raw IDA offsets
/*=== repairDroidFindBestJoint (0x5c2b80) ===*/
int __cdecl repairDroidFindBestJoint(udword *a1, float *a2, int *a3, int *a4)
{
  _DWORD *v4; // ecx
  int v5; // edi
  int v7; // edi
  int result; // eax
  int v9; // ebx
  int v10; // eax
  int v11; // esi
  unsigned __int16 *v12; // esi
  int v13; // ecx
  float *v14; // eax
  double v15; // st7
  int v16; // eax
  float *v17; // ecx
  double v18; // st7
  int v19; // ecx
  float *v20; // eax
  double v21; // st7
  int v22; // eax
  float *v23; // ecx
  double v24; // st7
  int v25; // ecx
  float *v26; // eax
  double v27; // st7
  int v28; // eax
  float *v29; // ecx
  double v30; // st7
  float v31; // [esp+10h] [ebp-14h]
  int v32; // [esp+14h] [ebp-10h]
  vector a; // [esp+18h] [ebp-Ch] BYREF
  int v34; // [esp+30h] [ebp+Ch]

  v4 = a1;
  v5 = a1[2];
  v31 = -2.0;
  if ( *a4 >= v5 )
    *a4 %= v5;
  v7 = a1[5];
  if ( *a3 >= v7 )
    *a3 %= v7;
  result = *a3;
  v9 = *a4;
  --*a3;
  v34 = result;
  v32 = 0;
  if ( (int)a1[5] < 0 )
  {
    *a4 = v9;
    *a3 = result;
  }
  else
  {
    do
    {
      v10 = *a3 + 1;
      *a3 = v10;
      v11 = v4[5];
      if ( v10 >= v11 )
        *a3 = v10 % v11;
      v12 = (unsigned __int16 *)(v4[9] + 40 * *a3);
      if ( *a4 == v12[2] )
      {
        v13 = v4[6];
        v14 = (float *)(v13 + 16 * *a4);
        a.x = *(float *)(16 * v12[3] + v13) - *v14;
        a.y = *(float *)(16 * v12[3] + v13 + 4) - v14[1];
        a.z = *(float *)(16 * v12[3] + v13 + 8) - v14[2];
        vecNormalize(&a);
        v15 = a.y * a2[1] + a.x * *a2 + a.z * a2[2];
        if ( v15 >= v31 )
        {
          v31 = v15;
          v9 = v12[3];
          v34 = *a3;
        }
        v16 = a1[6];
        v17 = (float *)(16 * *a4 + v16);
        a.x = *(float *)(16 * v12[4] + v16) - *v17;
        a.y = *(float *)(16 * v12[4] + v16 + 4) - v17[1];
        a.z = *(float *)(16 * v12[4] + v16 + 8) - v17[2];
        vecNormalize(&a);
        v18 = a.y * a2[1] + a.x * *a2 + a.z * a2[2];
        if ( v18 >= v31 )
        {
          v31 = v18;
          v9 = v12[4];
          v34 = *a3;
        }
        v4 = a1;
      }
      if ( *a4 == v12[3] )
      {
        v19 = v4[6];
        v20 = (float *)(v19 + 16 * *a4);
        a.x = *(float *)(16 * v12[2] + v19) - *v20;
        a.y = *(float *)(16 * v12[2] + v19 + 4) - v20[1];
        a.z = *(float *)(16 * v12[2] + v19 + 8) - v20[2];
        vecNormalize(&a);
        v21 = a.y * a2[1] + a.x * *a2 + a.z * a2[2];
        if ( v21 >= v31 )
        {
          v31 = v21;
          v9 = v12[2];
          v34 = *a3;
        }
        v22 = a1[6];
        v23 = (float *)(16 * *a4 + v22);
        a.x = *(float *)(16 * v12[4] + v22) - *v23;
        a.y = *(float *)(16 * v12[4] + v22 + 4) - v23[1];
        a.z = *(float *)(16 * v12[4] + v22 + 8) - v23[2];
        vecNormalize(&a);
        v24 = a.y * a2[1] + a.x * *a2 + a.z * a2[2];
        if ( v24 >= v31 )
        {
          v31 = v24;
          v9 = v12[4];
          v34 = *a3;
        }
        v4 = a1;
      }
      if ( *a4 == v12[4] )
      {
        v25 = v4[6];
        v26 = (float *)(v25 + 16 * *a4);
        a.x = *(float *)(16 * v12[2] + v25) - *v26;
        a.y = *(float *)(16 * v12[2] + v25 + 4) - v26[1];
        a.z = *(float *)(16 * v12[2] + v25 + 8) - v26[2];
        vecNormalize(&a);
        v27 = a.y * a2[1] + a.x * *a2 + a.z * a2[2];
        if ( v27 >= v31 )
        {
          v31 = v27;
          v9 = v12[2];
          v34 = *a3;
        }
        v28 = a1[6];
        v29 = (float *)(16 * *a4 + v28);
        a.x = *(float *)(16 * v12[3] + v28) - *v29;
        a.y = *(float *)(16 * v12[3] + v28 + 4) - v29[1];
        a.z = *(float *)(16 * v12[3] + v28 + 8) - v29[2];
        vecNormalize(&a);
        v30 = a.y * a2[1] + a.x * *a2 + a.z * a2[2];
        if ( v30 >= v31 )
        {
          v31 = v30;
          v9 = v12[3];
          v34 = *a3;
        }
        v4 = a1;
      }
      ++v32;
    }
    while ( v32 <= v4[5] );
    result = v34;
    *a4 = v9;
    *a3 = v34;
  }
  return result;
}
#endif

// Decompiled from cataclysm.exe @ 0x5C32F0
void repairDroidUpdate(RepairDroidController *controller)
{
    RepairDroidEntry *entries = controller->entries;
    sdword count = controller->count;
    Ship *parentShip = controller->parentShip;
    sdword entryIdx = 0;

    if (count <= 0)
    {
        controller->outputHealth = parentShip->health;
        return;
    }

    RepairDroidEntry *entry = entries;
    while (1)
    {
        Ship *droid = (Ship *)entry->repairDroid;
        udword *meshObj = (udword *)((ubyte *)parentShip->staticinfo->staticheader.LOD->level[0].pData + 32);

        if (!entry->state)
        {
            real32 parentHealth = parentShip->health;
            real32 parentMaxHealth = parentShip->staticinfo->maxhealth;
            if (parentHealth < parentMaxHealth
                && parentShip->currentLOD < 3u
                && (universe.totaltimeelapsed > (double)controller->nextEventTime || !entryIdx))
            {
                controller->nextEventTime = universe.totaltimeelapsed + REPAIR_SPEECH_DELAY;
                DockStaticInfo *dockInfo = parentShip->staticinfo->dockStaticInfo;
                if (dockInfo && dockInfo->numPointsOfType[DPT_Repairbot] > 0)
                {
                    sdword v12 = controller->cycleIndex + 1;
                    controller->cycleIndex = v12;
                    if (v12 >= dockInfo->numPointsOfType[DPT_Repairbot])
                        controller->cycleIndex = 0;
                    DockStaticPoint *dockPt = &dockInfo->dockstaticpoints[controller->cycleIndex + dockInfo->firstPointOfType[DPT_Repairbot]];
                    if (dockPt)
                    {
                        entry->targetPos.x = dockPt->conenormal.x;
                        entry->targetPos.y = dockPt->conenormal.y;
                        entry->targetPos.z = dockPt->conenormal.z;
                        entry->targetPos.x = dockPt->flyawaydist * entry->targetPos.x;
                        entry->targetPos.y = dockPt->flyawaydist * entry->targetPos.y;
                        entry->targetPos.z = dockPt->flyawaydist * entry->targetPos.z;
                        entry->targetPos.x = entry->targetPos.x + dockPt->position.x;
                        entry->targetPos.y = dockPt->position.y + entry->targetPos.y;
                        entry->targetPos.z = dockPt->position.z + entry->targetPos.z;
                        droid->posinfo.position.x = dockPt->position.x;
                        droid->posinfo.position.y = dockPt->position.y;
                        droid->posinfo.position.z = dockPt->position.z;
                        goto init_entry;
                    }
                }
                else
                {
                    ShipTypeToStr(parentShip->shiprace);
                    telldir();
                }
                entry->targetPos.x = 50.0f;
                entry->targetPos.y = 0.0f;
                entry->targetPos.z = 0.0f;
                droid->posinfo.position.x = 0.0f;
                droid->posinfo.position.y = 0.0f;
                droid->posinfo.position.z = 0.0f;
                ShipTypeToStr(parentShip->shiprace);
                telldir();
            init_entry:
                entry->jointIndexA = -1;
                entry->jointIndexB = -1;
                entry->timestamp1 = -1.0f;
                entry->timestamp2 = -1.0f;
                entry->unk10 = -1.0f;
                entry->state = 1;
                goto next_entry;
            }
            goto next_entry;
        }

        sdword jA = entry->jointIndexA;
        float *vertexPos = NULL;
        float *normalPos = NULL;

        if (jA == -1)
        {
            vertexPos = NULL;
        }
        else
        {
            sdword nVerts = (sdword)meshObj[2];
            if (jA >= nVerts)
                entry->jointIndexA = jA % nVerts;
            sdword normalBase = (sdword)meshObj[8];
            sdword vertBase = (sdword)meshObj[6] + 16 * entry->jointIndexA;
            sdword normIdx = 16 * ((vertexentry *)vertBase)->iVertexNormal;
            real32 nx = *(real32 *)(normIdx + normalBase);
            normalPos = (float *)(normalBase + normIdx);
            entry->targetPos.x = nx * BeastSelfDamageTick;
            entry->targetPos.y = normalPos[1] * BeastSelfDamageTick;
            entry->targetPos.z = normalPos[2] * BeastSelfDamageTick;
            entry->targetPos.x = entry->targetPos.x + ((vertexentry *)vertBase)->x;
            entry->targetPos.y = ((vertexentry *)vertBase)->y + entry->targetPos.y;
            entry->targetPos.z = ((vertexentry *)vertBase)->z + entry->targetPos.z;
            vertexPos = (float *)vertBase;
        }

        real32 dx = entry->targetPos.x - droid->posinfo.position.x;
        real32 dy = entry->targetPos.y - droid->posinfo.position.y;
        real32 dz = entry->targetPos.z - droid->posinfo.position.z;
        real32 vx = dx * repairVelocityDamp;
        real32 vy = dy * repairVelocityDamp;
        real32 vz = dz * repairVelocityDamp;
        droid->posinfo.velocity.x = vx;
        droid->posinfo.velocity.y = vy;
        droid->posinfo.velocity.z = vz;

        if (normalPos)
        {
            droid->rotinfo.rotspeed.x = droid->rotinfo.coordsys.m13;
            droid->rotinfo.rotspeed.y = droid->rotinfo.coordsys.m23;
            droid->rotinfo.rotspeed.z = droid->rotinfo.coordsys.m33;
            droid->rotinfo.rotspeed.x = droid->rotinfo.coordsys.m13 - normalPos[0];
            droid->rotinfo.rotspeed.y = droid->rotinfo.coordsys.m23 - normalPos[1];
            droid->rotinfo.rotspeed.z = droid->rotinfo.coordsys.m33 - normalPos[2];
            droid->rotinfo.rotspeed.x *= repairVelocityDamp;
            droid->rotinfo.rotspeed.y *= repairVelocityDamp;
            droid->rotinfo.rotspeed.z *= repairVelocityDamp;
        }

        droid->posinfo.position.x = vx * universe.unknown1 + droid->posinfo.position.x;
        droid->posinfo.position.y = vy * universe.unknown1 + droid->posinfo.position.y;
        droid->posinfo.position.z = vz * universe.unknown1 + droid->posinfo.position.z;

        real32 speedSq = vz * vz + vy * vy + vx * vx;

        switch (entry->state)
        {
            case 1:
                if (parentShip->health >= (double)parentShip->staticinfo->maxhealth
                    || parentShip->currentLOD >= 3u)
                    goto state5;
                if (speedSq <= repairRangeFar)
                {
                    entry->jointIndexA = meshObjectFindNearestVertexIndex((sdword)meshObj, (float *)&droid->posinfo.position);
                    real32 t = (real32)(ranRandom(0) % 0x708) * repairTimingSmall + universe.totaltimeelapsed;
                    entry->state = 2;
                    entry->timestamp1 = t + repairTimingBig;
                }
                break;
            case 2:
                if (speedSq > repairRangeNear)
                    break;
                if (parentShip->health >= (double)parentShip->staticinfo->maxhealth
                    || parentShip->currentLOD > 2u)
                {
                state5:
                    entry->state = 5;
                    break;
                }
                if (universe.totaltimeelapsed <= (double)entry->timestamp1)
                    repairDroidAnimStep(meshObj, &entry->jointIndexB, (udword *)&entry->jointIndexA);
                else
                    entry->state = 3;
                break;
            case 3:
                if (universe.totaltimeelapsed > (double)entry->timestamp1)
                {
                    repairDroidAnimStep(meshObj, &entry->jointIndexB, (udword *)&entry->jointIndexA);
                    entry->state = 4;
                    goto do_hit_effect;
                }
                if (universe.totaltimeelapsed <= (double)entry->timestamp2)
                {
                do_hit_effect:
                    ;
                }
                else
                {
                    if (speedSq > repairRangeMid)
                    {
                        goto do_hit_effect;
                    }
                    if (vertexPos)
                    {
                        vector jointWorld;
                        matrix *parentMat = &parentShip->rotinfo.coordsys;
                        matMultiplyMatByVec(&jointWorld, parentMat, (vector *)vertexPos);
                        jointWorld.x += parentShip->posinfo.position.x;
                        jointWorld.y += parentShip->posinfo.position.y;
                        jointWorld.z += parentShip->posinfo.position.z;
                        vector droidWorld;
                        matMultiplyMatByVec(&droidWorld, parentMat, &droid->posinfo.position);
                        droidWorld.x += parentShip->posinfo.position.x;
                        droidWorld.y += parentShip->posinfo.position.y;
                        droidWorld.z += parentShip->posinfo.position.z;
                        vector beamDir;
                        beamDir.x = droidWorld.x - jointWorld.x;
                        beamDir.y = droidWorld.y - jointWorld.y;
                        real32 bdz = droidWorld.z - jointWorld.z;
                        beamDir.z = bdz;
                        fmathSqrtDouble(bdz * bdz + beamDir.y * beamDir.y + beamDir.x * beamDir.x);
                        vecNormalize(&beamDir);
                        matrix beamMat;
                        matCreateCoordSysFromHeading(&beamMat, &beamDir);
                        if (repairDroidHitEffects)
                        {
                            sdword lodLevel = droid->currentLOD;
                            if ((sdword)(repairDroidHitEffects->nLevels - 1) < lodLevel)
                                lodLevel = repairDroidHitEffects->nLevels - 1;
                            etgeffectstatic *effectStat = repairDroidHitEffects->level[lodLevel];
                            if (effectStat
                                && etgHitEffectsEnabled
                                && !etgFrequencyExceeded(effectStat)
                                && univSpaceObjInRenderList(parentShip))
                            {
                                etgEffectCreate(effectStat, (Ship *)parentShip, &jointWorld, NULL, &beamMat, 1.0f, 8, 3);
                            }
                        }
                    }
                    else
                    {
                        vector zero = {0.0f, 0.0f, 0.0f};
                        telldir();
                        matrix *parentMat = &parentShip->rotinfo.coordsys;
                        vector jointWorld;
                        matMultiplyMatByVec(&jointWorld, parentMat, &zero);
                        jointWorld.x += parentShip->posinfo.position.x;
                        jointWorld.y += parentShip->posinfo.position.y;
                        jointWorld.z += parentShip->posinfo.position.z;
                        vector droidWorld;
                        matMultiplyMatByVec(&droidWorld, parentMat, &droid->posinfo.position);
                        droidWorld.x += parentShip->posinfo.position.x;
                        droidWorld.y += parentShip->posinfo.position.y;
                        droidWorld.z += parentShip->posinfo.position.z;
                        vector beamDir;
                        beamDir.x = droidWorld.x - jointWorld.x;
                        beamDir.y = droidWorld.y - jointWorld.y;
                        real32 bdz2 = droidWorld.z - jointWorld.z;
                        beamDir.z = bdz2;
                        fmathSqrtDouble(bdz2 * bdz2 + beamDir.y * beamDir.y + beamDir.x * beamDir.x);
                        vecNormalize(&beamDir);
                        matrix beamMat;
                        matCreateCoordSysFromHeading(&beamMat, &beamDir);
                        if (repairDroidHitEffects)
                        {
                            sdword lodLevel = droid->currentLOD;
                            if ((sdword)(repairDroidHitEffects->nLevels - 1) < lodLevel)
                                lodLevel = repairDroidHitEffects->nLevels - 1;
                            etgeffectstatic *effectStat = repairDroidHitEffects->level[lodLevel];
                            if (effectStat
                                && etgHitEffectsEnabled
                                && !etgFrequencyExceeded(effectStat)
                                && univSpaceObjInRenderList(parentShip))
                            {
                                etgEffectCreate(effectStat, (Ship *)parentShip, &jointWorld, NULL, &beamMat, 1.0f, 8, 3);
                            }
                        }
                    }
                    sdword rndT = (sdword)(ranRandom(0) % 0x64);
                    entry->timestamp2 = (real32)rndT * repairEffectInterval + universe.totaltimeelapsed + ZERO_POINT_FIVE;
                }
                if (parentShip->health >= (double)parentShip->staticinfo->maxhealth
                    || parentShip->currentLOD > 2u)
                    goto state5;
                break;
            case 4:
                if (speedSq <= repairRangeMid)
                {
                    real32 t4 = (real32)(ranRandom(0) % 0x3E8) * repairTimingSmall + universe.totaltimeelapsed;
                    entry->state = 3;
                    entry->timestamp1 = t4;
                }
                break;
            case 5:
                if (parentShip->health < (double)parentShip->staticinfo->maxhealth
                    && parentShip->currentLOD < 3u)
                {
                    entry->state = 3;
                    break;
                }
                if (speedSq <= repairRangeMid)
                {
                    DockStaticInfo *dockInfo = parentShip->staticinfo->dockStaticInfo;
                    DockStaticPoint *dockPt = NULL;
                    if (dockInfo && dockInfo->numPointsOfType[DPT_Repairbot] > 0)
                    {
                        sdword v38 = controller->cycleIndex + 1;
                        controller->cycleIndex = v38;
                        if (v38 >= dockInfo->numPointsOfType[DPT_Repairbot])
                            controller->cycleIndex = 0;
                        dockPt = &dockInfo->dockstaticpoints[controller->cycleIndex + dockInfo->firstPointOfType[DPT_Repairbot]];
                    }
                    else
                    {
                        ShipTypeToStr(parentShip->shiprace);
                        telldir();
                        dockPt = NULL;
                    }
                    if (clGunIsTargetInCone((float *)dockPt, (float *)&droid->posinfo.position) || entry->jointIndexA == -1)
                    {
                        if (dockPt)
                        {
                            entry->targetPos.x = dockPt->position.x;
                            entry->targetPos.y = dockPt->position.y;
                            entry->targetPos.z = dockPt->position.z;
                        }
                        else
                        {
                            entry->targetPos.x = 0.0f;
                            entry->targetPos.y = 0.0f;
                            entry->targetPos.z = 0.0f;
                        }
                        entry->jointIndexA = -1;
                        entry->jointIndexB = -1;
                        entry->state = 6;
                        break;
                    }
                    if (!dockPt)
                    {
                        entry->targetPos.x = 0.0f;
                        entry->targetPos.y = 0.0f;
                        entry->targetPos.z = 0.0f;
                        entry->jointIndexA = -1;
                        entry->jointIndexB = -1;
                        entry->state = 6;
                        break;
                    }
                    vector dockDir;
                    dockDir.x = dockPt->position.x  - droid->posinfo.position.x;
                    real32 dockY = dockPt->position.y;
                    real32 dockZ = dockPt->position.z;
                    dockDir.y = dockY - droid->posinfo.position.y;
                    dockDir.z = dockZ - droid->posinfo.position.z;
                    vecNormalize(&dockDir);
                    sdword savedJA = entry->jointIndexA;
                    repairDroidFindBestJoint(meshObj, &dockDir.x, &entry->jointIndexB, &entry->jointIndexA);
                    if (entry->prevJointIndexA == entry->jointIndexA)
                    {
                        entry->targetPos.x = dockPt->conenormal.x;
                        entry->targetPos.y = dockPt->conenormal.y;
                        entry->targetPos.z = dockPt->conenormal.z;
                        entry->targetPos.x = dockPt->maxdist * entry->targetPos.x;
                        entry->targetPos.y = dockPt->maxdist * entry->targetPos.y;
                        entry->targetPos.z = dockPt->maxdist * entry->targetPos.z;
                        entry->targetPos.x = entry->targetPos.x + dockPt->position.x;
                        entry->targetPos.y = dockPt->position.y + entry->targetPos.y;
                        entry->targetPos.z = dockPt->position.z + entry->targetPos.z;
                        entry->jointIndexA = -1;
                    }
                    entry->prevJointIndexA = savedJA;
                }
                break;
            case 6:
                entry->jointIndexA = -1;
                entry->jointIndexB = -1;
                if (speedSq <= repairSpeedThreshold)
                    entry->state = 0;
                break;
            default:
                telldir();
                break;
        }

    next_entry:
        entryIdx++;
        if (entryIdx >= count)
            break;
        entry = entry + 1;
    }

    controller->outputHealth = parentShip->health;

}

// Decompiled from cataclysm.exe @ 0x5C3CF0
void repairDroidRenderAttachments(RepairDroidController *controller)
{
    Ship *parentShip = controller->parentShip;

    if (parentShip->currentLOD >= 2u)
        return;

    sdword count = controller->count;
    RepairDroidEntry *entry = controller->entries;
    for (sdword i = 0; i < count; ++i, ++entry)
    {
        if (!entry->state)
            continue;
        Ship *droid = (Ship *)entry->repairDroid;
        glPushMatrix();
        glTranslatef(
            droid->posinfo.position.x,
            droid->posinfo.position.y,
            droid->posinfo.position.z);
        hmatMakeHMatFromMat(&repairDroidRenderHMatrix, &droid->rotinfo.coordsys);
        glMultMatrixf(&repairDroidRenderHMatrix.m11);
        lodinfo *lod = droid->staticinfo->staticheader.LOD;
        meshRender((meshdata *)lod->level[0].pData, 0);
        glPopMatrix();
    }
}

// Decompiled from cataclysm.exe @ 0x5C3EA0
char repairDroidFlyToTarget(SpaceObjRotImp *ship, vector *targetPos)
{
    char result;
    vector heading;
    heading.x = targetPos->x - ship->posinfo.position.x;
    heading.y = targetPos->y - ship->posinfo.position.y;
    heading.z = targetPos->z - ship->posinfo.position.z;
    vecNormalize(&heading);
    rammingFrigateBlendHeadingWeighted(ship, &heading, repairDroidBlendWeight, repairDroidCurrentWeight);
    vector vel;
    vel.x = ship->rotinfo.coordsys.m13;
    vel.y = ship->rotinfo.coordsys.m23;
    vel.z = ship->rotinfo.coordsys.m33;
    vecNormalize(&vel);
    real32 maxSpeed = ship->staticinfo->staticheader.maxvelocity;
    ship->posinfo.velocity.x = maxSpeed * vel.x;
    ship->posinfo.velocity.y = maxSpeed * vel.y;
    ship->posinfo.velocity.z = maxSpeed * vel.z;
    for (Node *i = universe.ShipList.head; i; i = i->next)
        result = (char)shipIsACV((Ship *)listGetStructOfNode(i));
    return result;
}

// Decompiled from cataclysm.exe @ 0x5C4050
BOOL repairDroidIsValidTarget(Ship *parentShip, Ship *target, sdword allowAllied)
{
    BOOL notDisabled = (parentShip->specialFlags2 & 0x400000) == 0;
    if (parentShip->playerowner == target->playerowner
        && (target->tacticstype != 3 || !allowAllied))
        return FALSE;
    if (!notDisabled)
        return notDisabled;
    sdword shipClass = target->staticinfo->shipclass;
    if (shipClass != 1 && shipClass != 2 && shipClass != 3 && shipClass != 4)
        return FALSE;
    if ((target->specialFlags2 & 8) == 8)
        return FALSE;
    return ((udword)shipModuleSlotRuntime(parentShip)->iconState[4].iconId != 1) && univIsPositionInBounds(&target->posinfo.position) && notDisabled;
}


// ============================================================================
// Decompiled from cataclysm.exe @ 0x5C4130
void repairDroidHyperspaceReset(Ship *ship)
{
    ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;
    Node *insideNode;

    if (shipsInsideMe == NULL)
    {
        return;
    }

    for (insideNode = shipsInsideMe->insideList.tail; insideNode != NULL; insideNode = insideNode->prev)
    {
        InsideShip *insideShip = listGetStructOfNode(insideNode);
        Ship *containedShip = insideShip->ship;

        workerCheckResetForHyperspace(containedShip);
        if (shipIsBeastRace(containedShip))
        {
            *(ubyte *)containedShip->ShipSpecifics = 1;
        }
        SalCapNotifyProcessingShipRemoved(containedShip);
    }
}

// Decompiled from cataclysm.exe @ 0x5C41A0
void repairDroidAlignToTarget(SpaceObjRotImp *ship, float *a2)
{
    vector toTarget;
    toTarget.x = a2[22] - ship->posinfo.position.x;
    toTarget.y = a2[23] - ship->posinfo.position.y;
    toTarget.z = a2[24] - ship->posinfo.position.z;
    vector heading = toTarget;
    vecNormalize(&heading);
    vector curHeading;
    curHeading.x = ship->rotinfo.coordsys.m13;
    curHeading.y = ship->rotinfo.coordsys.m23;
    curHeading.z = ship->rotinfo.coordsys.m33;
    vecNormalize(&curHeading);
    if (heading.x * curHeading.x + heading.y * curHeading.y + heading.z * curHeading.z <= repairAlignDotMin)
        rammingFrigateSetHeading(ship, &heading);
}

typedef struct
{
    real32 repairDistance;
    real32 repairCone;
    real32 repairBeamWidth;
} RepairNozzleScriptFields;

static RepairNozzleScriptFields repairNozzleScriptDummy;

static scriptStructEntry repairNozzleScriptTable[] =
{
    {"repairdistance",  scriptSetReal32CB,
     (udword)&repairNozzleScriptDummy.repairDistance,  (udword)&repairNozzleScriptDummy},
    {"repaircone",      scriptSetReal32CB,
     (udword)&repairNozzleScriptDummy.repairCone,      (udword)&repairNozzleScriptDummy},
    {"repairbeamwidth", scriptSetReal32CB,
     (udword)&repairNozzleScriptDummy.repairBeamWidth,  (udword)&repairNozzleScriptDummy},
    END_SCRIPT_STRUCT_ENTRY
};

// Decompiled from cataclysm.exe @ 0x5C7AC0
char repairDroidStaticScriptLoad(const char *directory, const char *filename, ShipStaticInfo *info)
{
    real32 *fields = &((WorkerHarvestStaticInfo *)info->custstatinfo)->repairDistance;

    memset(fields, 0, 28);
    scriptSetStruct((char *)directory, (char *)filename, repairNozzleScriptTable, (ubyte *)fields);

    if (fields[0] == 0.0f)
    {
        ShipTypeToStr(info->shiptype);
        telldir();
    }
    if (fields[1] == 0.0f)
    {
        ShipTypeToStr(info->shiptype);
        telldir();
    }
    if (fields[2] == 0.0f)
    {
        ShipTypeToStr(info->shiptype);
        telldir();
    }

    fields[3] = cosf(fields[1] * ZERO_POINT_FIVE * RADIAN_PER_DEG);

    DockStaticPoint *gun0 = dockFindDockStaticPoint("gun0_0", info->dockStaticInfo);
    fields[4] = gun0->position.x;
    fields[5] = gun0->position.y;
    fields[6] = gun0->position.z;

    info->canSpecialBandBoxFriendlies = 1;
    info->unk4 = 1;
    info->unk5 = 1;
    info->canSingleClickSpecialActivate = 1;
    return 1;
}

#if 0  // Cat2: repairDroidAlignToTarget live code differs from binary or raw IDA offsets
/*=== repairDroidAlignToTarget (0x5c41a0) ===*/
void __cdecl repairDroidAlignToTarget(int a1, float *a2)
{
  double v2; // st7
  real32 v3; // edx
  real32 v4; // eax
  vector a; // [esp+4h] [ebp-24h] BYREF
  vector v6; // [esp+10h] [ebp-18h] BYREF
  real32 v7; // [esp+1Ch] [ebp-Ch]
  real32 v8; // [esp+20h] [ebp-8h]
  real32 v9; // [esp+24h] [ebp-4h]

  v6.x = a2[22] - *(float *)(a1 + 88);
  v6.y = a2[23] - *(float *)(a1 + 92);
  v2 = a2[24] - *(float *)(a1 + 96);
  a = v6;
  v6.z = v2;
  vecNormalize(&a);
  v3 = *(float *)(a1 + 180);
  v4 = *(float *)(a1 + 184);
  v7 = *(float *)(a1 + 176);
  v6.x = v7;
  v8 = v3;
  v9 = v4;
  v6.y = v3;
  v6.z = v4;
  vecNormalize(&v6);
  if ( a.z * v6.z + a.y * v6.y + a.x * v6.x <= flt_72C004 )
    rammingFrigateSetHeading((struct SpaceObjRotImp *)a1, &a.x);
}
#endif

#if 0  // Cat2: repairDroidStaticScriptLoad live code differs from binary or raw IDA offsets
/*=== repairDroidStaticScriptLoad (0x5c7ac0) ===*/
char __cdecl Watch_Mission08_Events_EnemyGoingHere(const char *a1, const char *a2, int a3)
{
  int v3; // esi
  float *v4; // esi
  _DWORD *v5; // eax
  _DWORD *v6; // esi
  char result; // al
  int v8; // [esp+10h] [ebp-5Ch] BYREF
  char v9[24]; // [esp+14h] [ebp-58h] BYREF
  const char *v10[16]; // [esp+2Ch] [ebp-40h] BYREF

  v3 = *(udword *)(a3 + 404);
  v8 = 0;
  memset(v9, 0, sizeof(v9));
  v10[2] = (const char *)&v8;
  v10[1] = (const char *)scriptSetReal32CB;
  v10[5] = (const char *)scriptSetReal32CB;
  v10[9] = (const char *)scriptSetReal32CB;
  v10[6] = v9;
  v10[3] = (const char *)&v8;
  v10[10] = &v9[4];
  v10[11] = (const char *)&v8;
  v4 = (float *)(v3 + 20);
  v10[7] = (const char *)&v8;
  v10[0] = aRepairdistance;
  v10[4] = aRepaircone;
  v10[8] = aRepairbeamwidt;
  memset(&v10[12], 0, 16);
  memset(v4, 0, 0x1Cu);
  scriptSetStruct(a1, a2, v10, (int)v4);
  if ( *v4 == ZERO )
  {
    ShipTypeToStr(*(udword *)(a3 + 480));
    telldir();
  }
  if ( v4[1] == ZERO )
  {
    ShipTypeToStr(*(udword *)(a3 + 480));
    telldir();
  }
  if ( v4[2] == ZERO )
  {
    ShipTypeToStr(*(udword *)(a3 + 480));
    telldir();
  }
  v4[3] = cos(v4[1] * ZERO_POINT_FIVE * flt_72A4F8);
  v5 = (_DWORD *)(dockFindDockStaticPoint(aGun0_0, *(udword *)(a3 + 1008)) + 8);
  v6 = v4 + 4;
  *v6 = *v5;
  v6[1] = v5[1];
  v6[2] = v5[2];
  result = 1;
  *(_BYTE *)(a3 + 529) = 1;
  *(_BYTE *)(a3 + 530) = 1;
  *(_BYTE *)(a3 + 531) = 1;
  *(_BYTE *)(a3 + 532) = 1;
  return result;
}
#endif
