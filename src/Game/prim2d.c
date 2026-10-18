/*=============================================================================
    Name    : prim2d.c
    Purpose : Decompiled functions from cataclysm.exe (prim2d module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x5711B0
void primGLCircleOutline2(real32 x, real32 y, real32 radius, sdword nSegments, color c)
{
  int v6; // edx
  int v7; // ecx
  int v8; // esi
  float v10; // [esp+1Ch] [ebp-1Ch]
  float v11; // [esp+20h] [ebp-18h]
  float v12; // [esp+20h] [ebp-18h]
  float v13; // [esp+28h] [ebp-10h]
  __int128 varC; // [esp+2Ch] [ebp-Ch]

  *(double *)((char *)&varC + 4) = dbl_72A500 / (double)a5;
  glColor3ub(a6, a6 >> 8, HIWORD(a6), a1);
  glBegin(3);
  v11 = v13 + a3;
  glVertex2f(LODWORD(a2), LODWORD(v11));
  v6 = HIDWORD(varC);
  v7 = DWORD2(varC);
  *(_QWORD *)&varC = *((_QWORD *)&varC + 1);
  if ( a5 >= 0 )
  {
    v8 = a5 + 1;
    do
    {
      v12 = cos(*(long double *)&varC) * v13 + a3;
      v10 = sin(*(long double *)&varC) * a4 + a2;
      glVertex2f(LODWORD(v10), LODWORD(v12));
      --v8;
      *(double *)&varC = *(double *)&varC + *((double *)&varC + 1);
    }
    while ( v8 );
  }
  return glEnd(v7, v6);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x570F00
void primOvalArcOutline2(oval *o, real32 radStart, real32 radEnd, sdword thickness, sdword segments, color c)
{
  double v7; // st7
  float v8; // eax
  double v9; // st6
  double v10; // st6
  double v11; // st7
  double v12; // st7
  int v14; // edi
  int v15; // esi
  double v16; // st7
  int v17; // edi
  int v18; // esi
  int v19; // edx
  int v20; // ecx
  GLfloat v23; // [esp+14h] [ebp-2Ch]
  float v24; // [esp+28h] [ebp-18h]
  float v25; // [esp+2Ch] [ebp-14h]
  float v26; // [esp+30h] [ebp-10h]
  float v27; // [esp+34h] [ebp-Ch]
  float v28; // [esp+38h] [ebp-8h]
  float v29; // [esp+3Ch] [ebp-4h]
  float retaddr; // [esp+40h] [ebp+0h]
  float v31; // [esp+44h] [ebp+4h]
  float v32; // [esp+44h] [ebp+4h]
  float v33; // [esp+44h] [ebp+4h]
  float v34; // [esp+48h] [ebp+8h]
  float v35; // [esp+48h] [ebp+8h]
  float v36; // [esp+48h] [ebp+8h]
  int v37; // [esp+50h] [ebp+10h]
  int v38; // [esp+54h] [ebp+14h]

  if ( glcActive() )
  {
    v24 = (float)(int)*(_DWORD *)LODWORD(retaddr);
    v26 = (float)*(int *)(LODWORD(retaddr) + 4);
    v7 = (double)*(int *)(LODWORD(retaddr) + 12);
  }
  else
  {
    v8 = retaddr;
    v9 = ((double)(int)*(_DWORD *)LODWORD(retaddr) + flt_72A858) / (double)MAIN_WindowWidth;
    v24 = v9 + v9 - ONE;
    retaddr = (float)MAIN_WindowHeight;
    v10 = ((double)*(int *)(LODWORD(v8) + 4) + flt_72A858) / retaddr;
    v26 = ONE - (v10 + v10);
    v11 = (double)*(int *)(LODWORD(v8) + 12) / retaddr;
    v7 = v11 + v11;
  }
  v25 = v7;
  v12 = (double)a5;
  *(float *)&v37 = v12;
  *(double *)&dbl_C16840 = v12 * a2 * flt_72BE1C + dbl_72A458;
  v14 = *(_DWORD *)&dbl_C16840;
  *(double *)&dbl_C16840 = *(float *)&v37 * a3 * flt_72BE1C - flt_72A680 + dbl_72A458;
  v15 = *(_DWORD *)&dbl_C16840;
  glColor3ub(a6, a6 >> 8, HIWORD(a6), a1);
  glPushAttrib(4u);
  v23 = (float)v37;
  glLineWidth(v23);
  glBegin(3);
  v31 = sin(a3) * v24 + v25;
  v34 = cos(a3) * v26 + v27;
  if ( glcActive() )
  {
    v28 = v31;
    a7 = v34;
  }
  else
  {
    glVertex2f(LODWORD(v31), LODWORD(v34));
  }
  v16 = flt_72A594 / *(float *)&a6;
  v17 = v14 + 1;
  *(float *)&v38 = (double)v17 * v16;
  if ( v17 <= v15 )
  {
    v18 = v15 - v17 + 1;
    do
    {
      v32 = sin(*(float *)&v38) * v24 + v25;
      v35 = cos(*(float *)&v38) * v26 + v27;
      if ( glcActive() )
      {
        glcLine2((__int64)v28, (__int64)a7, (__int64)v32, (__int64)v35, v37, a6);
        v28 = v32;
        a7 = v35;
      }
      else
      {
        glVertex2f(LODWORD(v32), LODWORD(v35));
      }
      --v18;
      v29 = v16;
      *(float *)&v38 = v29 + *(float *)&v38;
    }
    while ( v18 );
  }
  v33 = sin(a4) * v24 + v25;
  v36 = cos(a4) * v26 + v27;
  if ( glcActive() )
    glcLine2((__int64)v28, (__int64)a7, (__int64)v33, (__int64)v36, v37, a6);
  else
    glVertex2f(LODWORD(v33), LODWORD(v36));
  glEnd(v20, v19);
  return glPopAttrib();
}
