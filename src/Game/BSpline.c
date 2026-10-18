/*=============================================================================
    Name    : BSpline.c
    Purpose : Decompiled functions from cataclysm.exe (BSpline module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x422EA0
splinecurve *__cdecl bsCurveStart(sdword nPoints, real32 *points, real32 *times, tcb *params, bool bAlloc)
{
  int v5; // ebx
  sdword v6; // eax
  _DWORD *result; // eax
  _DWORD *v8; // ecx
  float *v9; // ecx
  _DWORD *v10; // ecx
  int v11; // esi
  _DWORD *v12; // edi
  int v13; // ecx
  _DWORD *v14; // esi
  int v15; // esi
  _DWORD *v16; // ecx
  _DWORD *v17; // edx

  v5 = a1;
  v6 = 24;
  if ( a5 )
  {
    v5 = a1 + 3;
    v6 = 20 * (a1 + 3) + 24;
  }
  result = memAllocFunction(v6, MBF_Volatile);
  if ( a5 )
  {
    result[3] = result + 6;
    v8 = &result[v5 + 6];
    result[4] = v8;
    result[5] = &v8[v5];
    qmemcpy(result + 7, a2, 4 * v5 - 3);
    *(_DWORD *)result[3] = *(_DWORD *)(result[3] + 4);
    *(_DWORD *)(result[3] + 4 * v5 - 8) = *(_DWORD *)(result[3] + 4 * v5 - 12);
    *(_DWORD *)(result[3] + 4 * v5 - 4) = *(_DWORD *)(result[3] + 4 * v5 - 8);
    qmemcpy((void *)(result[4] + 4), a3, 4 * v5 - 3);
    v9 = (float *)(result[4] + 4 * v5 - 8);
    *v9 = *v9 + ONE;
    *(float *)(result[4] + 4 * v5 - 4) = *(float *)(result[4] + 4 * v5 - 8) + ONE;
    qmemcpy((void *)(result[5] + 12), a4, 12 * v5 - 3);
    v10 = (_DWORD *)result[5];
    *v10 = v10[3];
    v10[1] = v10[4];
    v10[2] = v10[5];
    v11 = result[5];
    v12 = (_DWORD *)(v11 + 4 * (3 * v5 - 9));
    v13 = 4 * (3 * v5 - 6);
    v14 = (_DWORD *)(v13 + v11);
    *v14 = *v12;
    v14[1] = v12[1];
    v14[2] = v12[2];
    v15 = result[5];
    v16 = (_DWORD *)(v15 + v13);
    v17 = (_DWORD *)(v15 + 12 * v5 - 12);
    *v17 = *v16;
    v17[1] = v16[1];
    v17[2] = v16[2];
    result[2] = v5;
  }
  else
  {
    result[3] = a2;
    result[2] = v5;
    result[4] = a3;
    result[5] = a4;
  }
  *result = 0;
  result[1] = 1;
  return result;
}
