/*=============================================================================
    Name    : prim3d.c
    Purpose : Decompiled functions from cataclysm.exe (prim3d module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x5730E0
void prim3dStartup(void)
{
  return listInit(&dword_B37BA0);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x572400
void primCircleSolid3(vector *centre, real32 radius, sdword nSlices, color c)
{
  float v4; // ecx
  int v5; // edx
  long double v6; // st7
  int v7; // edi
  double v8; // st7
  int v9; // edx
  int v10; // ecx
  int v12; // [esp+8h] [ebp-2Ch]
  double i; // [esp+14h] [ebp-20h]
  float v14; // [esp+28h] [ebp-Ch] BYREF
  float v15; // [esp+2Ch] [ebp-8h]
  int v16; // [esp+30h] [ebp-4h]

  glColor3ub(a4, a4 >> 8, HIWORD(a4), v12);
  v4 = *((float *)a1 + 1);
  v5 = a1[2];
  v14 = *(float *)a1;
  v15 = v4;
  v16 = v5;
  glBegin(6);
  glVertex3fv(&v14);
  v6 = ZERO_2;
  v7 = a3;
  if ( a3 > 0 )
  {
    for ( i = dbl_72A500 / (double)a3; ; v6 = i + v6 )
    {
      v14 = sin(v6) * a2 + *(float *)a1;
      v15 = cos(v6) * a2 + *((float *)a1 + 1);
      glVertex3fv(&v14);
      if ( !--v7 )
        break;
    }
  }
  v8 = a2 + *((float *)a1 + 1);
  v14 = *(float *)a1;
  v15 = v8;
  glVertex3fv(&v14);
  return glEnd(v10, v9);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x572af0
void primSolidTexture3(vector *p1, real32 size, color c, trhandle tex)
{
  int v2; // eax
  double v3; // st7
  int v4; // edx
  int v5; // ecx
  int v6; // eax
  int v7; // edx
  int v8; // ecx
  float v10; // [esp+A8h] [ebp-24h]
  float v11; // [esp+A8h] [ebp-24h]
  float v12; // [esp+A8h] [ebp-24h]
  float v13; // [esp+A8h] [ebp-24h]
  float v14; // [esp+ACh] [ebp-20h]
  float v15; // [esp+ACh] [ebp-20h]
  float v16; // [esp+ACh] [ebp-20h]
  float v17; // [esp+ACh] [ebp-20h]
  float v18; // [esp+B4h] [ebp-18h]
  float v19; // [esp+B4h] [ebp-18h]
  float v20; // [esp+B4h] [ebp-18h]
  float v21; // [esp+B4h] [ebp-18h]
  float v22; // [esp+B8h] [ebp-14h]
  float v23; // [esp+B8h] [ebp-14h]
  float v24; // [esp+B8h] [ebp-14h]
  float v25; // [esp+B8h] [ebp-14h]
  float v26; // [esp+B8h] [ebp-14h]
  float v27; // [esp+B8h] [ebp-14h]
  float v28; // [esp+B8h] [ebp-14h]
  float v29; // [esp+B8h] [ebp-14h]
  int v30; // [esp+BCh] [ebp-10h]
  float v31; // [esp+BCh] [ebp-10h]
  float v32; // [esp+BCh] [ebp-10h]
  float v33; // [esp+BCh] [ebp-10h]
  float v34; // [esp+BCh] [ebp-10h]
  int v35; // [esp+C0h] [ebp-Ch]
  float v36; // [esp+C0h] [ebp-Ch]
  float *green; // [esp+C8h] [ebp-4h]
  float retaddr; // [esp+CCh] [ebp+0h]
  int v39; // [esp+D0h] [ebp+4h]
  int v40; // [esp+D4h] [ebp+8h]
  float v41; // [esp+D4h] [ebp+8h]

  v2 = glCapFeatureExists(0x9004u);
  v3 = retaddr * ZERO_POINT_FIVE;
  if ( v2 )
  {
    retaddr = v3;
    rndTextureEnable(1);
    trMakeCurrent(a2);
    if ( (*(_BYTE *)(trStructureGet(a2) + 12) & 8) != 0 )
    {
      glEnable(3042);
      dword_B3A4B0(3008);
      rndAdditiveBlends(1);
    }
    v40 = BYTE1(a1);
    v6 = BYTE2(a1);
    *(float *)&v39 = (double)(unsigned __int8)a1 * flt_72A6B0;
    v36 = (double)v40 * flt_72A6B0;
    v41 = (double)v6 * flt_72A6B0;
    if ( RGL )
    {
      glPixelTransferf(0xD15u, *(GLfloat *)&v39);
      glPixelTransferf(0xD19u, v36);
      glPixelTransferf(0xD1Bu, v41);
    }
    glColor3f(*(GLfloat *)&v39, v36, v41);
    glBegin(7);
    glTexCoord2f(0, 0);
    v14 = green[1] - retaddr;
    v10 = *green - retaddr;
    glVertex3f(LODWORD(v10), LODWORD(v14), 0);
    glTexCoord2f(1065353216, 0);
    v15 = green[1] - retaddr;
    v11 = retaddr + *green;
    glVertex3f(LODWORD(v11), LODWORD(v15), 0);
    glTexCoord2f(1065353216, 1065353216);
    v16 = retaddr + green[1];
    v12 = retaddr + *green;
    glVertex3f(LODWORD(v12), LODWORD(v16), 0);
    glTexCoord2f(0, 1065353216);
    v17 = retaddr + green[1];
    v13 = *green - retaddr;
    glVertex3f(LODWORD(v13), LODWORD(v17), 0);
    glEnd(v8, v7);
    if ( RGL )
    {
      glPixelTransferf(0xD15u, 0.0);
      glPixelTransferf(0xD19u, 0.0);
      glPixelTransferf(0xD1Bu, 0.0);
    }
  }
  else
  {
    retaddr = v3;
    rndTextureEnable(1);
    trMakeCurrent(a2);
    if ( (*(_BYTE *)(trStructureGet(a2) + 12) & 8) != 0 )
    {
      glEnable(3042);
      dword_B3A4B0(3008);
      rndAdditiveBlends(1);
    }
    glBegin(7);
    glColor3ub(a1, a1 >> 8, HIWORD(a1), v30);
    glTexCoord2f(0, 0);
    v22 = *(float *)(LODWORD(retaddr) + 4) - *(float *)&a1;
    v18 = *(float *)LODWORD(retaddr) - *(float *)&a1;
    glVertex3f(LODWORD(v18), LODWORD(v22), 0);
    glTexCoord2f(1065353216, 0);
    v23 = *(float *)(LODWORD(retaddr) + 4) - *(float *)&a1;
    v19 = *(float *)&a1 + *(float *)LODWORD(retaddr);
    glVertex3f(LODWORD(v19), LODWORD(v23), 0);
    glTexCoord2f(1065353216, 1065353216);
    v24 = *(float *)&a1 + *(float *)(LODWORD(retaddr) + 4);
    v20 = *(float *)&a1 + *(float *)LODWORD(retaddr);
    glVertex3f(LODWORD(v20), LODWORD(v24), 0);
    glTexCoord2f(0, 1065353216);
    v25 = *(float *)&a1 + *(float *)(LODWORD(retaddr) + 4);
    v21 = *(float *)LODWORD(retaddr) - *(float *)&a1;
    glVertex3f(LODWORD(v21), LODWORD(v25), 0);
    if ( (gDevcaps & 0x80000) == 0 )
    {
      glColor3ub(172, 172, 172, v35);
      glTexCoord2f(0, 0);
      v31 = *(float *)(LODWORD(retaddr) + 4) - *(float *)&a2;
      v26 = *(float *)LODWORD(retaddr) - *(float *)&a2;
      glVertex3f(LODWORD(v26), LODWORD(v31), 0);
      glTexCoord2f(1065353216, 0);
      v32 = *(float *)(LODWORD(retaddr) + 4) - *(float *)&a2;
      v27 = *(float *)&a2 + *(float *)LODWORD(retaddr);
      glVertex3f(LODWORD(v27), LODWORD(v32), 0);
      glTexCoord2f(1065353216, 1065353216);
      v33 = *(float *)&a2 + *(float *)(LODWORD(retaddr) + 4);
      v28 = *(float *)&a2 + *(float *)LODWORD(retaddr);
      glVertex3f(LODWORD(v28), LODWORD(v33), 0);
      glTexCoord2f(0, 1065353216);
      v34 = *(float *)&a2 + *(float *)(LODWORD(retaddr) + 4);
      v29 = *(float *)LODWORD(retaddr) - *(float *)&a2;
      glVertex3f(LODWORD(v29), LODWORD(v34), 0);
    }
    glEnd(v5, v4);
  }
  dword_B3A4B0(3042);
  return rndAdditiveBlends(0);
}
