/*=============================================================================
    Name    : Game.c
    Purpose : Decompiled functions from cataclysm.exe (Game module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A3230
int levelPreInit(const char *directory, const char *filename)
{
  int v2; // ebp
  unsigned int v3; // edx
  char *v4; // edi
  char *v5; // edi
  const char *v6; // esi
  char v7; // cl
  unsigned int v8; // eax
  int v9; // esi
  unsigned int v10; // edi
  BOOL v11; // eax
  struct AIPlayer **p_aiPlayer; // esi
  int v13; // ebx
  float v14; // eax
  int *v15; // ebx
  int v16; // ebp
  ShipTypeEnum v17; // esi
  int v18; // edi
  ShipStaticInfo2 *ShipStaticInfo; // eax
  int v20; // ecx
  int v21; // edx
  int v22; // eax
  ShipTypeEnum v23; // esi
  int v24; // edi
  int v25; // ecx
  ShipStaticInfo2 *v26; // eax
  int v27; // ecx
  int v28; // edx
  int v29; // eax
  int v30; // eax
  _DWORD *v31; // ebp
  _DWORD *v32; // ebx
  int v33; // edi
  ShipTypeEnum v34; // esi
  int v35; // ebp
  ShipStaticInfo2 *v36; // eax
  int v37; // edx
  int v38; // eax
  int v39; // ecx
  ShipTypeEnum v40; // esi
  int v41; // ebp
  int v42; // edx
  ShipStaticInfo2 *v43; // eax
  int v44; // ecx
  int v45; // edx
  int v46; // eax
  int v47; // eax
  int result; // eax
  int *v49; // edi
  int v50; // esi
  int v51; // eax
  ShipStaticInfo2 *v52; // eax
  int v53; // ecx
  int v54; // eax
  _DWORD *v55; // eax
  _DWORD *v56; // ebp
  int v57; // edi
  int v58; // eax
  ShipStaticInfo2 *v59; // eax
  int v60; // ecx
  int v61; // eax
  ShipStaticInfo2 *v62; // eax
  struct AIPlayer **v63; // [esp+10h] [ebp-210h]
  int v64; // [esp+10h] [ebp-210h]
  int v65; // [esp+10h] [ebp-210h]
  int v66; // [esp+14h] [ebp-20Ch]
  int v67; // [esp+18h] [ebp-208h]
  char v68[80]; // [esp+1Ch] [ebp-204h] BYREF
  char v69[52]; // [esp+6Ch] [ebp-1B4h] BYREF
  int v70[96]; // [esp+A0h] [ebp-180h] BYREF

  v2 = 0;
  dword_C362DC = 0;
  if ( a1 )
  {
    strcpy(v68, a1);
    v3 = strlen(a2) + 1;
    v4 = &v68[strlen(v68)];
    qmemcpy(v4, a2, 4 * (v3 >> 2));
    v6 = &a2[4 * (v3 >> 2)];
    v5 = &v4[4 * (v3 >> 2)];
    v7 = v3;
  }
  else
  {
    v8 = strlen(a2) + 1;
    qmemcpy(v68, a2, 4 * (v8 >> 2));
    v6 = &a2[4 * (v8 >> 2)];
    v5 = &v68[4 * (v8 >> 2)];
    v7 = v8;
  }
  qmemcpy(v5, v6, v7 & 3);
  v9 = fileOpen(v68, 1);
  v10 = fileLineRead(v9, v69, 50);
  fileClose(v9);
  v11 = v10 != -12 && strncmp(aMissionman, v69, 0xCu) == 0;
  missionman = v11;
  qmemcpy(v70, &dword_BB3020, sizeof(v70));
  sub_53CCD0();
  sub_52B800();
  sub_57CD20();
  mainFreeDerelictTexture();
  mainFreeNISTexture();
  sub_53C940();
  scriptSet(a1, a2, (const char **)&off_883BE0);
  sub_51C980();
  p_aiPlayer = &universe.players[0].aiPlayer;
  v13 = 0;
  v63 = &universe.players[0].aiPlayer;
  do
  {
    if ( v13 >= (unsigned __int16)word_A43CAC || *p_aiPlayer )
    {
      v14 = *(float *)((char *)&dword_BB3020 + v2 * 4);
      if ( LODWORD(v14) != v70[v2] || *(int *)((char *)&dword_BB3024 + v2 * 4) != v70[v2 + 1] )
        sub_52BA90(v13, v14, *(int *)((char *)&dword_BB3024 + v2 * 4));
    }
    else
    {
      qmemcpy((char *)&dword_BB3020 + v2 * 4, &v70[v2], 0x30u);
      p_aiPlayer = v63;
    }
    p_aiPlayer += 6476;
    ++v13;
    v2 += 12;
    v63 = p_aiPlayer;
  }
  while ( (int)p_aiPlayer < (int)&unk_B99E9C );
  v15 = (int *)dword_C0A5EC;
  v64 = dword_C0A5EC;
  if ( dword_C0A5EC )
  {
    while ( 1 )
    {
      v16 = *(_DWORD *)(v15[3] + 16);
      v17 = *(_DWORD *)(v16 + 688);
      v18 = *(_DWORD *)(v16 + 692);
      ShipStaticInfo = GetShipStaticInfo(v17);
      ShipStaticInfo->staticheader.infoFlags |= 1u;
      if ( v17 == tMissileDestroyer
        || v17 == sDreadnought
        || v17 == p1MissileCorvette
        || v17 == sDestroyer
        || v17 == sAcolyte
        || v17 == bAcolyte )
      {
        byte_BA0F70[300 * v18] |= 1u;
      }
      else if ( v17 == tMinelayerCorvette )
      {
        byte_B3AC10[300 * v18] |= 1u;
      }
      v20 = *(_DWORD *)(v16 + 32);
      v21 = *(_DWORD *)(v20 + 240);
      v22 = *(_DWORD *)(v20 + 244);
      v23 = *(_DWORD *)(v16 + 688);
      v24 = *(_DWORD *)(v16 + 692);
      v25 = *(unsigned __int8 *)(v16 + 857);
      v66 = v21;
      v67 = v22;
      LOWORD(v21) = word_8B1814[v23];
      *((_BYTE *)&v66 + v25) = 1;
      if ( ((unsigned __int16)v21 & (1 << v24)) != 0 )
      {
        v26 = GetShipStaticInfo(v23);
        v27 = v66;
        v28 = v67;
        *(_DWORD *)v26->teamColor = v66;
        *(_DWORD *)&v26->teamColor[4] = v28;
        if ( v23 == sDreadnought
          || v23 == tMissileDestroyer
          || v23 == bMissileCorvette
          || v23 == sAcolyte
          || v23 == bAcolyte
          || v23 == sDestroyer
          || v23 == p1MissileCorvette )
        {
          v30 = 75 * v24;
          dword_BA1030[v30] = v27;
          dword_BA1034[v30] = v28;
        }
        else if ( v23 == tMinelayerCorvette )
        {
          v29 = 75 * v24;
          dword_B3ACD0[v29] = v27;
          dword_B3ACD4[v29] = v28;
        }
      }
      else
      {
        telldir();
      }
      v31 = *(_DWORD **)(v16 + 1204);
      if ( v31 )
      {
        v32 = (_DWORD *)*v31;
        if ( *v31 )
        {
          do
          {
            v33 = *(_DWORD *)(v32[3] + 16);
            v34 = *(_DWORD *)(v33 + 688);
            v35 = *(_DWORD *)(v33 + 692);
            v36 = GetShipStaticInfo(v34);
            v36->staticheader.infoFlags |= 1u;
            if ( v34 == tMissileDestroyer
              || v34 == sDreadnought
              || v34 == p1MissileCorvette
              || v34 == sDestroyer
              || v34 == sAcolyte
              || v34 == bAcolyte )
            {
              byte_BA0F70[300 * v35] |= 1u;
            }
            else if ( v34 == tMinelayerCorvette )
            {
              byte_B3AC10[300 * v35] |= 1u;
            }
            v37 = *(_DWORD *)(v33 + 32);
            v38 = *(_DWORD *)(v37 + 240);
            v39 = *(_DWORD *)(v37 + 244);
            v40 = *(_DWORD *)(v33 + 688);
            v41 = *(_DWORD *)(v33 + 692);
            v42 = *(unsigned __int8 *)(v33 + 857);
            v66 = v38;
            LOWORD(v38) = word_8B1814[v40];
            v67 = v39;
            *((_BYTE *)&v66 + v42) = 1;
            if ( ((unsigned __int16)v38 & (1 << v41)) != 0 )
            {
              v43 = GetShipStaticInfo(v40);
              v44 = v66;
              v45 = v67;
              *(_DWORD *)v43->teamColor = v66;
              *(_DWORD *)&v43->teamColor[4] = v45;
              if ( v40 == sDreadnought
                || v40 == tMissileDestroyer
                || v40 == bMissileCorvette
                || v40 == sAcolyte
                || v40 == bAcolyte
                || v40 == sDestroyer
                || v40 == p1MissileCorvette )
              {
                v47 = 75 * v41;
                dword_BA1030[v47] = v44;
                dword_BA1034[v47] = v45;
              }
              else if ( v40 == tMinelayerCorvette )
              {
                v46 = 75 * v41;
                dword_B3ACD0[v46] = v44;
                dword_B3ACD4[v46] = v45;
              }
            }
            else
            {
              telldir();
            }
            v32 = (_DWORD *)*v32;
          }
          while ( v32 );
        }
        v15 = (int *)v64;
      }
      v64 = *v15;
      if ( !*v15 )
        break;
      v15 = (int *)*v15;
    }
  }
  dword_A447AC = 0;
  dword_A447B0 = 0;
  scriptSet(a1, a2, (const char **)&off_883CB0);
  result = singlePlayerGame;
  if ( singlePlayerGame )
  {
    v49 = (int *)dword_C0A5EC;
    v65 = dword_C0A5EC;
    if ( dword_C0A5EC )
    {
      while ( 1 )
      {
        v50 = v49[3];
        v51 = sub_429B70(*(_DWORD *)(v50 + 16));
        if ( v51 )
        {
          v52 = GetShipStaticInfo(*(ShipTypeEnum *)(v51 + 8));
          v52->staticheader.infoFlags |= 1u;
        }
        *(_BYTE *)(*(_DWORD *)(*(_DWORD *)(v50 + 16) + 32) + 48) |= 1u;
        *(_DWORD *)(*(_DWORD *)(v50 + 16) + 696) = 1;
        *(_DWORD *)(*(_DWORD *)(v50 + 16) + 56) &= ~0x400u;
        *(_DWORD *)(*(_DWORD *)(v50 + 16) + 56) &= ~0x80000u;
        *(_DWORD *)(*(_DWORD *)(v50 + 16) + 832) = 0;
        *(_DWORD *)(*(_DWORD *)(v50 + 16) + 456) &= ~0x80u;
        v53 = *(_DWORD *)(v50 + 16);
        v54 = *(_DWORD *)(v53 + 688);
        if ( v54 == 17 || v54 == 52 )
          *(_DWORD *)(v53 + 2432) = 0;
        v55 = *(_DWORD **)(*(_DWORD *)(v50 + 16) + 1204);
        if ( v55 )
        {
          v56 = (_DWORD *)*v55;
          if ( *v55 )
          {
            do
            {
              v57 = v56[3];
              v58 = sub_429B70(*(_DWORD *)(v57 + 16));
              if ( v58 )
              {
                v59 = GetShipStaticInfo(*(ShipTypeEnum *)(v58 + 8));
                v59->staticheader.infoFlags |= 1u;
              }
              *(_DWORD *)(*(_DWORD *)(v50 + 16) + 696) = 1;
              *(_BYTE *)(*(_DWORD *)(*(_DWORD *)(v57 + 16) + 32) + 48) |= 1u;
              *(_DWORD *)(*(_DWORD *)(v57 + 16) + 832) = 0;
              v60 = *(_DWORD *)(v57 + 16);
              v61 = *(_DWORD *)(v60 + 688);
              if ( v61 == 17 || v61 == 52 )
                *(_DWORD *)(v60 + 2432) = 0;
              *(_DWORD *)(*(_DWORD *)(v57 + 16) + 456) &= ~0x80u;
              v56 = (_DWORD *)*v56;
            }
            while ( v56 );
            v49 = (int *)v65;
          }
        }
        result = *v49;
        v65 = *v49;
        if ( !*v49 )
          break;
        v49 = (int *)*v49;
      }
    }
  }
  else
  {
    if ( (tpGameCreated_flag & 4) != 0 && dword_A447B0 )
    {
      v62 = GetShipStaticInfo(tStandardFrigate);
      v62->staticheader.infoFlags |= 1u;
    }
    result = tpGameCreated_flag;
    if ( (tpGameCreated_flag & 0x1000) != 0 )
      byte_BA0650[0] |= 1u;
  }
  return result;
}


#if 0  // Cat2: scriptSetPlayerColorEntryCB live impl in teams.c — no dbgAsserts, uses colRGBA alpha=0xFF, raw trailColors pointer arithmetic (0x52B6D0)
int __cdecl scriptSetPlayerColorEntryCB(int a1, char *a2)
{
  int result; // eax
  int v3; // [esp+0h] [ebp-10h] BYREF
  int v4; // [esp+4h] [ebp-Ch] BYREF
  int v5; // [esp+8h] [ebp-8h] BYREF
  int v6; // [esp+Ch] [ebp-4h] BYREF

  sscanf(a2, "%d,%d,%d,%d,%d", &v5, &v6, &v4, &v3, &a2);
  result = v6;
  dword_BB3040[12 * v5 + v6] = (unsigned __int8)v4 | (((unsigned __int8)v3 | (((unsigned __int8)a2 | 0xFFFFFF00) << 8)) << 8);
  return result;
}
#endif

#if 0  // Cat2: scriptSetColorEntryCB live impl in teams.c — raw pointer arithmetic (a3 + 48*idx), uses colRGBA alpha=0xFF (0x52B750)
int __cdecl scriptSetColorEntryCB(int a1, char *a2, int a3)
{
  int result; // eax
  int v4; // [esp+0h] [ebp-Ch] BYREF
  int v5; // [esp+4h] [ebp-8h] BYREF
  int v6; // [esp+8h] [ebp-4h] BYREF

  sscanf(a2, "%d,%d,%d,%d", &v6, &v5, &v4, &a2);
  result = 48 * v6;
  *(_DWORD *)(48 * v6 + a3) = (unsigned __int8)v5 | (((unsigned __int8)v4 | (((unsigned __int8)a2 | 0xFFFFFF00) << 8)) << 8);
  return result;
}
#endif

#if 0  // Cat2: colModifyColorHSV live impl in color.c — binary uses x87 FPU bias, raw color byte extraction, sub_554FD0=colRGBToHLS, sub_555170=colHLSToRGB (0x52B890)
unsigned int __cdecl colModifyColorHSV(float a1, float *a2)
{
  __int16 v2; // ax
  double v3; // st7
  double v4; // st7
  double v5; // st7
  char v6; // al
  double v7; // st7
  unsigned __int8 v8; // dl
  double v9; // st7
  unsigned __int8 v10; // cl
  float v12; // [esp+0h] [ebp-20h]
  float v13; // [esp+4h] [ebp-1Ch]
  float v14; // [esp+8h] [ebp-18h]
  float v15; // [esp+Ch] [ebp-14h] BYREF
  float v16; // [esp+10h] [ebp-10h] BYREF
  int v17; // [esp+14h] [ebp-Ch] BYREF
  int v18; // [esp+18h] [ebp-8h] BYREF
  int v19; // [esp+1Ch] [ebp-4h] BYREF

  v2 = LOWORD(a1);
  LODWORD(a1) = BYTE2(a1);
  v14 = (double)SLODWORD(a1) * INV_OF_255;
  LODWORD(a1) = (unsigned __int8)v2;
  v13 = (double)HIBYTE(v2) * INV_OF_255;
  v12 = (double)(unsigned __int8)v2 * INV_OF_255;
  sub_554FD0((int)&a1, (int)&v16, (int)&v15, v12, v13, v14);
  v3 = a1 + *a2;
  a1 = v3;
  if ( v3 < ZERO )
    a1 = a1 + ONE;
  if ( a1 > (double)ONE )
    a1 = a1 - ONE;
  v4 = v16 * a2[1];
  v16 = v4;
  v15 = v15 * a2[2];
  if ( v4 > ONE )
    v16 = 1.0;
  if ( v15 > (double)ONE )
    v15 = 1.0;
  sub_555170((int)&v19, (int)&v18, (int)&v17, a1, v16, v15);
  v5 = *(float *)&v17 * flt_72A6A8 + dbl_72A458;
  *(double *)&dbl_C16840 = v5;
  if ( *(int *)&dbl_C16840 <= 255 )
  {
    *(double *)&dbl_C16840 = v5;
    v6 = dbl_C16840;
  }
  else
  {
    v6 = -1;
  }
  v7 = *(float *)&v18 * flt_72A6A8 + dbl_72A458;
  *(double *)&dbl_C16840 = v7;
  if ( *(int *)&dbl_C16840 <= 255 )
  {
    *(double *)&dbl_C16840 = v7;
    v8 = dbl_C16840;
  }
  else
  {
    v8 = -1;
  }
  v9 = *(float *)&v19 * flt_72A6A8 + dbl_72A458;
  *(double *)&dbl_C16840 = v9;
  if ( *(int *)&dbl_C16840 <= 255 )
  {
    *(double *)&dbl_C16840 = v9;
    v10 = dbl_C16840;
  }
  else
  {
    v10 = -1;
  }
  return v10 | ((v8 | ((*(_DWORD *)&v6 | 0xFFFFFF00) << 8)) << 8);
}
#endif

#if 0  // Cat2: SetPlayerColorScheme live impl in teams.c — uses colModifyColorHSV for trail colors, colRGBA alpha=0xFF for adjusted colors, brightness check via colRGBToHSV (0x52BA90)
int __cdecl SetPlayerColorScheme(int a1, float a2, int a3)
{
  float v3; // ebx
  double v4; // st7
  int v5; // esi
  int v6; // ecx
  int v7; // eax
  float *v8; // esi
  int *v9; // edi
  int v10; // ebx
  int v11; // ebp
  color *v12; // esi
  int v13; // kr00_4
  int v14; // ecx
  int v15; // kr04_4
  int v16; // edi
  int v17; // eax
  int v18; // kr0C_4
  int v19; // ecx
  int v20; // kr10_4
  int v21; // edi
  int v22; // eax
  int v23; // kr18_4
  int v24; // ecx
  int v25; // kr1C_4
  int v26; // edi
  int v27; // eax
  int v28; // kr24_4
  int v29; // ecx
  int v30; // kr28_4
  int v31; // edi
  int v32; // eax
  int v33; // kr30_4
  int v34; // ecx
  int v35; // kr34_4
  int v36; // edi
  int v37; // eax
  int v38; // kr3C_4
  int v39; // ecx
  int v40; // kr40_4
  int v41; // edi
  int result; // eax
  float v43; // [esp+0h] [ebp-24h]
  float v44; // [esp+4h] [ebp-20h]
  float v45; // [esp+8h] [ebp-1Ch]
  int v46; // [esp+1Ch] [ebp-8h] BYREF
  int v47; // [esp+20h] [ebp-4h] BYREF

  v3 = a2;
  if ( multiPlayerGame )
  {
    v45 = (double)BYTE2(a2) * INV_OF_255;
    LODWORD(a2) = LOBYTE(a2);
    v44 = (double)BYTE1(v3) * INV_OF_255;
    v43 = (double)LOBYTE(v3) * INV_OF_255;
    sub_554CD0((float *)&v47, (float *)&v46, &a2, v43, v44, v45);
    v4 = a2 * flt_72A6A8 + dbl_72A458;
    *(double *)&dbl_C16840 = v4;
    if ( *(int *)&dbl_C16840 <= 255 )
    {
      *(double *)&dbl_C16840 = v4;
      v5 = (unsigned __int8)dbl_C16840;
    }
    else
    {
      v5 = 255;
    }
    if ( v5 < 200 * dword_752DDC / 255 )
    {
      v3 = NAN;
      a3 = -6013185;
    }
  }
  v6 = a3;
  v7 = 48 * a1;
  *(float *)((char *)&dword_BB3020 + v7) = v3;
  *(int *)((char *)&dword_BB3024 + v7) = v6;
  v8 = (float *)&unk_BB2FE0;
  v9 = &dword_BB3040[12 * a1];
  do
  {
    *v9 = sub_52B890(v3, v8);
    v8 += 3;
    ++v9;
  }
  while ( (int)v8 < (int)&unk_BB3010 );
  v10 = dword_8AA7B4;
  v11 = dword_8AA7B0;
  v12 = &dword_BB3020;
  do
  {
    v13 = dword_8AA7B8 * (unsigned __int8)BYTE2(*v12);
    if ( v13 / 0x10000 >= 0 )
    {
      LOBYTE(v14) = -1;
      if ( v13 / 0x10000 <= 255 )
        v14 = v13 / 0x10000;
    }
    else
    {
      LOBYTE(v14) = 0;
    }
    v15 = dword_8AA7B8 * (unsigned __int8)BYTE1(*v12);
    if ( v15 / 0x10000 >= 0 )
    {
      LOBYTE(v16) = -1;
      if ( v15 / 0x10000 <= 255 )
        v16 = v15 / 0x10000;
    }
    else
    {
      LOBYTE(v16) = 0;
    }
    v17 = dword_8AA7B8 * *(unsigned __int8 *)v12 / 0x10000;
    if ( v17 >= 0 )
    {
      if ( v17 > 255 )
        LOBYTE(v17) = -1;
    }
    else
    {
      LOBYTE(v17) = 0;
    }
    v12[2] = (unsigned __int8)v17 | (((unsigned __int8)v16 | ((v14 | 0xFFFFFF00) << 8)) << 8);
    v18 = v10 * (unsigned __int8)BYTE2(*v12);
    if ( v18 / 0x10000 >= 0 )
    {
      LOBYTE(v19) = -1;
      if ( v18 / 0x10000 <= 255 )
        v19 = v18 / 0x10000;
    }
    else
    {
      LOBYTE(v19) = 0;
    }
    v20 = v10 * (unsigned __int8)BYTE1(*v12);
    if ( v20 / 0x10000 >= 0 )
    {
      LOBYTE(v21) = -1;
      if ( v20 / 0x10000 <= 255 )
        v21 = v20 / 0x10000;
    }
    else
    {
      LOBYTE(v21) = 0;
    }
    v22 = v10 * *(unsigned __int8 *)v12 / 0x10000;
    if ( v22 >= 0 )
    {
      if ( v22 > 255 )
        LOBYTE(v22) = -1;
    }
    else
    {
      LOBYTE(v22) = 0;
    }
    v12[3] = (unsigned __int8)v22 | (((unsigned __int8)v21 | ((v19 | 0xFFFFFF00) << 8)) << 8);
    v23 = v11 * (unsigned __int8)BYTE2(*v12);
    if ( v23 / 0x10000 >= 0 )
    {
      LOBYTE(v24) = -1;
      if ( v23 / 0x10000 <= 255 )
        v24 = v23 / 0x10000;
    }
    else
    {
      LOBYTE(v24) = 0;
    }
    v25 = v11 * (unsigned __int8)BYTE1(*v12);
    if ( v25 / 0x10000 >= 0 )
    {
      LOBYTE(v26) = -1;
      if ( v25 / 0x10000 <= 255 )
        v26 = v25 / 0x10000;
    }
    else
    {
      LOBYTE(v26) = 0;
    }
    v27 = v11 * *(unsigned __int8 *)v12 / 0x10000;
    if ( v27 >= 0 )
    {
      if ( v27 > 255 )
        LOBYTE(v27) = -1;
    }
    else
    {
      LOBYTE(v27) = 0;
    }
    v12[4] = (unsigned __int8)v27 | (((unsigned __int8)v26 | ((v24 | 0xFFFFFF00) << 8)) << 8);
    v28 = dword_8AA7B8 * (unsigned __int8)BYTE2(*v12);
    if ( v28 / 0x10000 >= 0 )
    {
      LOBYTE(v29) = -1;
      if ( v28 / 0x10000 <= 255 )
        v29 = v28 / 0x10000;
    }
    else
    {
      LOBYTE(v29) = 0;
    }
    v30 = dword_8AA7B8 * (unsigned __int8)BYTE1(*v12);
    if ( v30 / 0x10000 >= 0 )
    {
      LOBYTE(v31) = -1;
      if ( v30 / 0x10000 <= 255 )
        v31 = v30 / 0x10000;
    }
    else
    {
      LOBYTE(v31) = 0;
    }
    v32 = dword_8AA7B8 * *(unsigned __int8 *)v12 / 0x10000;
    if ( v32 >= 0 )
    {
      if ( v32 > 255 )
        LOBYTE(v32) = -1;
    }
    else
    {
      LOBYTE(v32) = 0;
    }
    v12[5] = (unsigned __int8)v32 | (((unsigned __int8)v31 | ((v29 | 0xFFFFFF00) << 8)) << 8);
    v33 = v10 * (unsigned __int8)BYTE2(*v12);
    if ( v33 / 0x10000 >= 0 )
    {
      LOBYTE(v34) = -1;
      if ( v33 / 0x10000 <= 255 )
        v34 = v33 / 0x10000;
    }
    else
    {
      LOBYTE(v34) = 0;
    }
    v35 = v10 * (unsigned __int8)BYTE1(*v12);
    if ( v35 / 0x10000 >= 0 )
    {
      LOBYTE(v36) = -1;
      if ( v35 / 0x10000 <= 255 )
        v36 = v35 / 0x10000;
    }
    else
    {
      LOBYTE(v36) = 0;
    }
    v37 = v10 * *(unsigned __int8 *)v12 / 0x10000;
    if ( v37 >= 0 )
    {
      if ( v37 > 255 )
        LOBYTE(v37) = -1;
    }
    else
    {
      LOBYTE(v37) = 0;
    }
    v12[6] = (unsigned __int8)v37 | (((unsigned __int8)v36 | ((v34 | 0xFFFFFF00) << 8)) << 8);
    v38 = v11 * (unsigned __int8)BYTE2(*v12);
    if ( v38 / 0x10000 >= 0 )
    {
      LOBYTE(v39) = -1;
      if ( v38 / 0x10000 <= 255 )
        v39 = v38 / 0x10000;
    }
    else
    {
      LOBYTE(v39) = 0;
    }
    v40 = v11 * (unsigned __int8)BYTE1(*v12);
    if ( v40 / 0x10000 >= 0 )
    {
      LOBYTE(v41) = -1;
      if ( v40 / 0x10000 <= 255 )
        v41 = v40 / 0x10000;
    }
    else
    {
      LOBYTE(v41) = 0;
    }
    result = v11 * *(unsigned __int8 *)v12 / 0x10000;
    if ( result >= 0 )
    {
      if ( result > 255 )
        LOBYTE(result) = -1;
    }
    else
    {
      LOBYTE(result) = 0;
    }
    result = (unsigned __int8)result;
    v12[7] = (unsigned __int8)result | (((unsigned __int8)v41 | ((v39 | 0xFFFFFF00) << 8)) << 8);
    v12 += 12;
  }
  while ( (int)v12 < (int)&dword_BB31A0 );
  return result;
}
#endif
