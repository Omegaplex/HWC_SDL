#include "AIUtilities.h"
#include "vector.h"
#include "volume.h"

//
//  returns true if the point is in the volume
//
sdword volPointInside(Volume *vol, vector *point)
{
    real32 radiusSquared;
    switch (vol->type)
    {
        case VOLUME_AA_BOX:
            return (point->x >= vol->attribs.aaBox.x0 &&
                    point->x <= vol->attribs.aaBox.x1 &&
                    point->y >= vol->attribs.aaBox.y0 &&
                    point->y <= vol->attribs.aaBox.y1 &&
                    point->z >= vol->attribs.aaBox.z0 &&
                    point->z <= vol->attribs.aaBox.z1);

        case VOLUME_SPHERE:
            radiusSquared = vol->attribs.sphere.radius * vol->attribs.sphere.radius;
            return aiuFindDistanceSquared(*point, vol->attribs.sphere.center) <= radiusSquared;

        default:
            return 0;
    }
}


/*-----------------------------------------------------------------------------
    Name        : volSphereIntersection
    Description : Returns true if the volume intersects the sphere
    Inputs      : volume - the volume,
                  center, radius - parameters defining the sphere
    Outputs     :
    Return      : TRUE if the volume and the sphere intersect
----------------------------------------------------------------------------*/
#define SQR(x) (x)*(x)
sdword volSphereIntersection(Volume vol, vector center, real32 radius)
{
    real32 volDistSq, dmin = 0;

    switch (vol.type)
    {
        case VOLUME_AA_BOX:
            if (center.x < min(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1))
                dmin += SQR(center.x - min(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1));
            else if (center.x > max(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1))
                dmin += SQR(center.x - max(vol.attribs.aaBox.x0, vol.attribs.aaBox.x1));

            if (center.y < min(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1))
                dmin += SQR(center.y - min(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1));
            else if (center.y > max(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1))
                dmin += SQR(center.y - max(vol.attribs.aaBox.y0, vol.attribs.aaBox.y1));

            if (center.z < min(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1))
                dmin += SQR(center.z - min(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1));
            else if (center.z > max(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1))
                dmin += SQR(center.z - max(vol.attribs.aaBox.z0, vol.attribs.aaBox.z1));

            return dmin <= radius*radius;
            break;
        case VOLUME_SPHERE:
            volDistSq = aiuFindDistanceSquared(center, vol.attribs.sphere.center);
            return (volDistSq < ((radius+vol.attribs.sphere.radius) * (radius+vol.attribs.sphere.radius)));
            break;
        default:
            return 0;
            break;
    }
}

#if 0  // Cat2: volFindCenter HW1 version — Cataclysm uses if/else and fixes z0+z0 bug (0x551140)
vector volFindCenter(Volume *vol)
{
    vector returnvec;

    switch (vol->type)
    {
        case VOLUME_AA_BOX:
            returnvec.x = (vol->attribs.aaBox.x0 + vol->attribs.aaBox.x1) / 2;
            returnvec.y = (vol->attribs.aaBox.y0 + vol->attribs.aaBox.y1) / 2;
            returnvec.z = (vol->attribs.aaBox.z0 + vol->attribs.aaBox.z0) / 2;
            return returnvec;
            break;
        case VOLUME_SPHERE:
            return vol->attribs.sphere.center;
            break;
        default:
            vecZeroVector(returnvec);
            return returnvec;
            break;
    }
}
#endif

#if 0  // Cat2: volFindRadius HW1 version — Cataclysm uses early-return if/else and * 0.5f (0x5511E0)
real32 volFindRadius(Volume *vol)
{
    real32 radius = 0;

    switch (vol->type)
    {
        case VOLUME_AA_BOX:
            //divides the average side length by two
            radius = (((vol->attribs.aaBox.x1 - vol->attribs.aaBox.x0) + (vol->attribs.aaBox.y1 - vol->attribs.aaBox.y0) + (vol->attribs.aaBox.z1 - vol->attribs.aaBox.z0))/3)/2;
            break;
        case VOLUME_SPHERE:
            radius = vol->attribs.sphere.radius;
            break;
        default:
            break;
    }
    return radius;
}
#endif





// Decompiled from cataclysm.exe @ 0x551140
vector volFindCenter(Volume *vol)
{
    vector result;
    if (vol->type == VOLUME_SPHERE)
    {
        result = vol->attribs.sphere.center;
    }
    else if (vol->type == VOLUME_AA_BOX)
    {
        result.x = (vol->attribs.aaBox.x1 + vol->attribs.aaBox.x0) * 0.5f;
        result.y = (vol->attribs.aaBox.y1 + vol->attribs.aaBox.y0) * 0.5f;
        result.z = (vol->attribs.aaBox.z1 + vol->attribs.aaBox.z0) * 0.5f;
    }
    else
    {
        result.x = result.y = result.z = 0.0f;
    }
    return result;
}

// Decompiled from cataclysm.exe @ 0x5511E0
real32 volFindRadius(Volume *vol)
{
    if (vol->type == VOLUME_SPHERE)
        return vol->attribs.sphere.radius;
    if (vol->type == VOLUME_AA_BOX)
        return (vol->attribs.aaBox.z1 - vol->attribs.aaBox.z0
              + vol->attribs.aaBox.y1 - vol->attribs.aaBox.y0
              + vol->attribs.aaBox.x1 - vol->attribs.aaBox.x0)
             * (1.0f / 3.0f) * 0.5f;
    return 0.0f;
}

#if 0  // Cat2: volFindCenter and volFindRadius binary uses raw offsets; live code uses struct fields (0x551140, 0x5511E0)
{
  float *result; // eax
  float v3; // [esp+0h] [ebp-Ch]
  float v4; // [esp+4h] [ebp-8h]
  float v5; // [esp+8h] [ebp-4h]

  if ( *(_DWORD *)a2 )
  {
    if ( *(_DWORD *)a2 == 1 )
    {
      v3 = (*(float *)(a2 + 8) + *(float *)(a2 + 4)) * ZERO_POINT_FIVE;
      v4 = (*(float *)(a2 + 16) + *(float *)(a2 + 12)) * ZERO_POINT_FIVE;
      v5 = (*(float *)(a2 + 24) + *(float *)(a2 + 20)) * ZERO_POINT_FIVE;
    }
    else
    {
      v3 = 0.0;
      v4 = 0.0;
      v5 = 0.0;
    }
    result = a1;
    *a1 = v3;
    a1[1] = v4;
    a1[2] = v5;
  }
  else
  {
    result = a1;
    *a1 = *(float *)(a2 + 4);
    a1[1] = *(float *)(a2 + 8);
    a1[2] = *(float *)(a2 + 12);
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5511E0
real32 volFindRadius(Volume *vol)
{
  double result; // st7

  result = ZERO;
  if ( !*(_DWORD *)a1 )
    return *(float *)(a1 + 16);
  if ( *(_DWORD *)a1 == 1 )
    return (*(float *)(a1 + 24)
          - *(float *)(a1 + 20)
          + *(float *)(a1 + 16)
          - *(float *)(a1 + 12)
          + *(float *)(a1 + 8)
          - *(float *)(a1 + 4))
         * flt_72A520
         * ZERO_POINT_FIVE;
  return result;
}
#endif


#if 0  // Cat2: volPointInside live code differs from binary
/*=== volPointInside (0x550da0) ===*/
signed int __cdecl volPointInside(int a1, float *a2)
{
int v3; // ecx
float v4; // edx
float v5; // [esp-Ch] [ebp-Ch]
float v6; // [esp+4h] [ebp+4h]

if ( *(_DWORD *)a1 )
{
if ( *(_DWORD *)a1 == 1
&& *a2 >= (double)*(float *)(a1 + 4)
&& *a2 <= (double)*(float *)(a1 + 8)
&& a2[1] >= (double)*(float *)(a1 + 12)
&& a2[1] <= (double)*(float *)(a1 + 16)
&& a2[2] >= (double)*(float *)(a1 + 20)
&& a2[2] <= (double)*(float *)(a1 + 24) )
{
return 1;
}
}
else
{
v3 = a1 + 4;
v5 = *(float *)(a1 + 4);
v4 = *(float *)(a1 + 8);
v6 = *(float *)(a1 + 16) * *(float *)(a1 + 16);
if ( aiuFindDistanceSquared(*a2, a2[1], a2[2], v5, v4, *(float *)(v3 + 8)) <= v6 )
return 1;
}
return 0;
}
#endif


#if 0  // Cat2: volSphereIntersection live code differs from binary
/*=== volSphereIntersection (0x550e70) ===*/
BOOL __cdecl volSphereIntersection(
int a1,
float a2,
float a3,
float a4,
float a5,
float a6,
float a7,
float a8,
float a9,
float a10,
float a11)
{
double v11; // st7
double v13; // st6
double v14; // st7
double v15; // st6
double v16; // st6
char v17; // fps^1
char v18; // ah
bool v19; // c0
bool v20; // c3
double v21; // st6
double v22; // st5
double v23; // st6
char v24; // fps^1
char v25; // ah
bool v26; // c0
bool v27; // c3
double v28; // st6
double v29; // st5
double v30; // st6
float v31; // [esp+2Ch] [ebp+2Ch]

v11 = ZERO;
if ( !a1 )
{
v31 = a5 + a11;
return v31 * v31 > aiuFindDistanceSquared(a8, a9, a10, a2, a3, a4);
}
if ( a1 != 1 )
return 0;
if ( a2 <= (double)a3 )
v13 = a2;
else
v13 = a3;
if ( a8 < v13 )
{
if ( a2 <= (double)a3 )
v14 = a2;
else
v14 = a3;
if ( a2 > (double)a3 )
{
LABEL_12:
v15 = a3;
LABEL_13:
v11 = (a8 - v15) * (a8 - v14);
goto LABEL_24;
}
LABEL_23:
v15 = a2;
goto LABEL_13;
}
if ( a2 <= (double)a3 )
v16 = a3;
else
v16 = a2;
if ( a8 > v16 )
{
if ( a2 <= (double)a3 )
v14 = a3;
else
v14 = a2;
if ( a2 <= (double)a3 )
goto LABEL_12;
goto LABEL_23;
}
LABEL_24:
v18 = v17;
v19 = a4 < (double)a5;
v20 = a4 == a5;
if ( (v18 & 1) != 0 )
{
if ( v19 || v20 )
v21 = a4;
else
v21 = a5;
if ( a4 > (double)a5 )
{
LABEL_29:
v22 = a5;
LABEL_30:
v11 = v11 + (a9 - v22) * (a9 - v21);
goto LABEL_41;
}
LABEL_40:
v22 = a4;
goto LABEL_30;
}
if ( v19 || v20 )
v23 = a5;
else
v23 = a4;
if ( a9 > v23 )
{
if ( a4 <= (double)a5 )
v21 = a5;
else
v21 = a4;
if ( a4 <= (double)a5 )
goto LABEL_29;
goto LABEL_40;
}
LABEL_41:
v25 = v24;
v26 = a6 < (double)a7;
v27 = a6 == a7;
if ( (v25 & 1) == 0 )
{
if ( v26 || v27 )
v30 = a7;
else
v30 = a6;
if ( a10 <= v30 )
return v11 <= a11 * a11;
if ( a6 <= (double)a7 )
v28 = a7;
else
v28 = a6;
if ( a6 <= (double)a7 )
goto LABEL_46;
LABEL_58:
v29 = a6;
goto LABEL_47;
}
if ( v26 || v27 )
v28 = a6;
else
v28 = a7;
if ( a6 <= (double)a7 )
goto LABEL_58;
LABEL_46:
v29 = a7;
LABEL_47:
v11 = v11 + (a10 - v29) * (a10 - v28);
return v11 <= a11 * a11;
}
#endif
