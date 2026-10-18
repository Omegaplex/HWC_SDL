/*=============================================================================
    Name    : DefenseFighter.c
    Purpose : Decompiled functions from cataclysm.exe (DefenseFighter module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5B0470
void DefenseFighterInit(Ship *ship)
{
    DefenseFighterSpec *spec = (DefenseFighterSpec *)ship->ShipSpecifics;
    spec->DefenseFighterCanNowRotate = 1;
    listInit(&spec->DefenseList);
}

#if 0  // Cat3: defenseFighterInterceptBullet — needs implementation; see IDA decompile at 0x5B0940 (team check at bullet+0x14C vs ship+0x32C, intercepted flag at bullet+0x140, DefenseFighterStatics* at staticinfo->custstatinfo) (0x5B0940)
signed int __cdecl defenseFighterInterceptBullet(struct SpaceObj *a1, struct SpaceObj *spaceobj)
{
  int v2;
  struct Effect *v3;
  int v5;
  struct etgeffectstatic *v6;
  double v7;
  long double v8;
  __int64 v9;
  real32 z;
  double v11;
  char *v12;
  sdword v13;
  real32 costheta;
  char *costheta_4;
  real32 costheta_4a;
  char *costheta_8;
  char *costheta_8a;
  sdword costheta_8b;
  sdword costheta_12;
  sdword costheta_12a;
  float v22;
  float v23;
  _BYTE v24[12];
  vector heading;
  float v26;
  float v27;
  vector a;
  matrix result;
  matrix a5;
  matrix first;

  v2 = *(_DWORD *)(*((_DWORD *)a1 + 8) + 404);
  if ( *((_DWORD *)spaceobj + 83) == *((_DWORD *)a1 + 203) || (*((_BYTE *)spaceobj + 320) & 1) != 0 )
    return 0;
  if ( !sub_5BCF80((int)spaceobj) )
  {
    *((_DWORD *)spaceobj + 80) |= 1u;
    if ( ranRandom(10) % 0x65 >= *(_DWORD *)(v2 + 16) )
    {
      v26 = fmathSqrtDouble(*((float *)spaceobj + 25) * *((float *)spaceobj + 25) + *((float *)spaceobj + 26) * *((float *)spaceobj + 26) + *((float *)spaceobj + 27) * *((float *)spaceobj + 27));
      a.x = *((float *)spaceobj + 22) - *((float *)a1 + 22);
      a.y = *((float *)spaceobj + 23) - *((float *)a1 + 23);
      a.z = *((float *)spaceobj + 24) - *((float *)a1 + 24);
      vecNormalize(&a);
      matCreateCoordSysFromHeading(&a5, &a);
      if ( dword_C41CF0 && (v5 = *((unsigned __int8 *)a1 + 52), v5 < *(_DWORD *)dword_C41CF0) )
        v6 = *(struct etgeffectstatic **)(dword_C41CF0 + 4 * v5 + 4);
      else
        v6 = 0;
      if ( univSpaceObjInRenderList(a1) && v6 && etgHitEffectsEnabled && !etgFrequencyExceeded(v6) )
      {
        v7 = univGetDamageMultiplier(*((_DWORD *)spaceobj + 68), 0) * *((float *)spaceobj + 78);
        if ( !RGLtype )
          v7 = v7 * ETG_SoftwareScalarHit;
        v22 = *((float *)a1 + 207);
        v27 = v22 * v7;
        etgEffectCreate((int)v6, (int)a1, (_DWORD *)spaceobj + 22, 0, &a5, v22, 96, 1);
      }
      v23 = (double)(ranRandom(10) % (*(_DWORD *)(v2 + 20) + 1)) * flt_72A4F8;
      v8 = v23;
      if ( (ranRandom(10) & 1) != 0 )
        v8 = v8 * MINUS_ONE;
      costheta_4a = sin(v8);
      costheta = cos(v8);
      matMakeRotAboutZ(&first, costheta, costheta_4a);
      matMultiplyMatByMat(&result, &first, &a5);
      *(_QWORD *)v24 = *(_QWORD *)&result.m11;
      *(real32 *)&v24[8] = result.m31;
      if ( (ranRandom(10) & 1) != 0 )
      {
        *(float *)v24 = *(float *)v24 * MINUS_ONE;
        *(float *)&v24[4] = *(float *)&v24[4] * MINUS_ONE;
        *(float *)&v24[8] = *(float *)&v24[8] * MINUS_ONE;
      }
      heading = *(vector *)v24;
      vecNormalize(&heading);
      matCreateCoordSysFromHeading(&((SpaceObjRotImpTarg *)spaceobj)->rotinfo.coordsys, &heading);
      v9 = *(_QWORD *)&heading.x;
      *((float *)spaceobj + 25) = heading.x * v26;
      *((_DWORD *)spaceobj + 57) = v9;
      z = heading.z;
      v11 = heading.y * v26;
      *((_DWORD *)spaceobj + 58) = HIDWORD(v9);
      *((real32 *)spaceobj + 59) = z;
      *((float *)spaceobj + 26) = v11;
      *((float *)spaceobj + 27) = heading.z * v26;
      *((float *)spaceobj + 60) = *((float *)spaceobj + 63) * heading.x;
      *((float *)spaceobj + 61) = *((float *)spaceobj + 63) * heading.y;
      *((float *)spaceobj + 62) = *((float *)spaceobj + 63) * heading.z;
      *((float *)spaceobj + 70) = *(float *)(v2 + 12) + *((float *)spaceobj + 70);
    }
    return 0;
  }
  v3 = (struct Effect *)*((_DWORD *)spaceobj + 77);
  sub_5BCFA0((int)spaceobj);
  if ( v3 )
  {
    etgEffectDelete(v3);
    univRemoveObjFromRenderList(v3);
    listDeleteNode((Node *)v3);
  }
  pingRemovePingByOwner(spaceobj);
  sub_54B1D0((int)spaceobj);
  bobObjectDied(spaceobj, &universe.collBlobList);
  /* offset 0xD4 = start of CollInfo in SpaceObjRotImp; cast to Node* suggests IDA may have
     the layout inverted (impactablelink before collInfo) — verify with binary before reconciling */
  listRemoveNode((Node *)((char *)spaceobj + 212));
  univRemoveObjFromRenderList(spaceobj);
  listDeleteNode((Node *)spaceobj);
  return 1;
}
#endif

#if 0  // Cat2: dfgUpdateAllFrigates (0x5b0df0) — live equivalent is dfgUpdateAllFrigatesAtPosition(vector*) in physics.c; binary takes vector* arg (IDA missed it); same logic
signed int __cdecl dfgUpdateAllFrigates()
{
  Node *head; // esi
  int v1; // edi
  struct Node *v2; // ecx

  head = universe.ShipList.head;
  v1 = 0;
  if ( !universe.ShipList.head )
    return 0;
  do
  {
    v2 = head[1].prev[86].next;
    if ( v2 == (struct Node *)45 || v2 == (struct Node *)78 )
    {
      if ( vecDistanceLessThanRadiusSq() )
        v1 = 1;
    }
    head = head->next;
  }
  while ( head );
  return v1;
}
#endif
