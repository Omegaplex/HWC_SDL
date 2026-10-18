/*=============================================================================
    Name    : clipper.c
    Purpose : out-of-GL line clipping / projection code

    Created 4/10/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#include <windows.h>
#endif
#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "glinc.h"
#include "clipper.h"


#define CORRECT_BBOX_CLIP 0


#ifndef DEPTH_SCALE
#define DEPTH_SCALE 65535.0f
#define CLIP_RIGHT_BIT 1
#define CLIP_LEFT_BIT 2
#define CLIP_TOP_BIT 4
#define CLIP_BOTTOM_BIT 8
#define CLIP_NEAR_BIT 16
#define CLIP_FAR_BIT 32
#define CLIP_ALL_BITS 0x3f
#endif

/*-----------------------------------------------------------------------------
    Name        : clipViewclipLine
    Description :
    Inputs      : vectors - packed array of 4 tuples, expected to have extra space
                  i - address of 1st vertex index
                  j - address of 2nd vertex index
    Outputs     : i, j - vertex indices
    Return      : 0 = line completely outside of view
                  1 = line inside view
----------------------------------------------------------------------------*/
sdword clipViewclipLine(real32* vectors, udword* i, udword* j)
{
    real32 t, dx, dy, dz, dw;
    udword ii, jj, Free;
    real32* coord = vectors;

    ii = *i;
    jj = *j;
    Free = 2;

#define GENERAL_CLIP \
    if (OUTSIDE(ii)) \
    { \
        if (OUTSIDE(jj)) \
        { \
            return 0; \
        } \
        else \
        { \
            COMPUTE_INTERSECTION(Free, jj, ii) \
            ii = Free; \
            Free++; \
        } \
    } \
    else \
    { \
        if (OUTSIDE(jj)) \
        { \
            COMPUTE_INTERSECTION(Free, ii, jj) \
            jj = Free; \
            Free++; \
        } \
    }

#define X(I) coord[4*I + 0]
#define Y(I) coord[4*I + 1]
#define Z(I) coord[4*I + 2]
#define W(I) coord[4*I + 3]

    /* clip against +X side */
#define OUTSIDE(K) (X(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dx = X(out) - X(in); \
    dw = W(out) - W(in); \
    t = (X(in) - W(in)) / (dw - dx); \
    X(new) = X(in) + t * dx; \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw; \

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against -X side */
#define OUTSIDE(K) (X(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dx = X(out) - X(in); \
    dw = W(out) - W(in); \
    t = -(X(in) + W(in)) / (dw + dx); \
    X(new) = X(in) + t * dx; \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against +Y side */
#define OUTSIDE(K) (Y(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = (Y(in) - W(in)) / (dw - dy); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * dy; \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against -Y side */
#define OUTSIDE(K) (Y(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dy = Y(out) - Y(in); \
    dw = W(out) - W(in); \
    t = -(Y(in) + W(in)) / (dw + dy); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * dy; \
    Z(new) = Z(in) + t * (Z(out) - Z(in)); \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against +Z side */
#define OUTSIDE(K) (Z(K) > W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = (Z(in) - W(in)) / (dw - dz); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * dz; \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

    /* clip against -Z side */
#define OUTSIDE(K) (Z(K) < -W(K))
#define COMPUTE_INTERSECTION(new, in, out) \
    dz = Z(out) - Z(in); \
    dw = W(out) - W(in); \
    t = -(Z(in) + W(in)) / (dw + dz); \
    X(new) = X(in) + t * (X(out) - X(in)); \
    Y(new) = Y(in) + t * (Y(out) - Y(in)); \
    Z(new) = Z(in) + t * dz; \
    W(new) = W(in) + t * dw;

    GENERAL_CLIP
#undef OUTSIDE
#undef COMPUTE_INTERSECTION

#undef GENERAL_CLIP

    *i = ii;
    *j = jj;
    return 1;
}

/*-----------------------------------------------------------------------------
    Name        : clipTransformPoints
    Description : modelview transform stage
    Inputs      : n - number of verts
                  vObj - objectspace (modelview space) coordinates
                  vEye - cameraspace coordinates
                  m - modelview matrix
    Outputs     : fills in vEye with transformed vObj
    Return      :
    Notes       : assumes a general matrix, ie. very inefficient
----------------------------------------------------------------------------*/
void clipTransformPoints(udword n, real32* vObj, real32* vEye, real32* m)
{
    udword i;
    real32 ox, oy, oz;

    for (i = 0; i < n; i++)
    {
        ox = vObj[4*i + 0];
        oy = vObj[4*i + 1];
        oz = vObj[4*i + 2];
        vEye[4*i + 0] = m[0] * ox + m[4] * oy + m[8]  * oz + m[12];
        vEye[4*i + 1] = m[1] * ox + m[5] * oy + m[9]  * oz + m[13];
        vEye[4*i + 2] = m[2] * ox + m[6] * oy + m[10] * oz + m[14];
        vEye[4*i + 3] = m[3] * ox + m[7] * oy + m[11] * oz + m[15];
    }
}

/*-----------------------------------------------------------------------------
    Name        : clipIsPerspective
    Description : determines whether a given matrix is a perspective matrix
    Inputs      : m - projection matrix
    Outputs     :
    Return      : TRUE  = yes, it is a perspective
                  FALSE = no, it's something else
----------------------------------------------------------------------------*/
bool32 clipIsPerspective(real32* m)
{
    if (m[4] == 0.0f && m[12] == 0.0f && m[1] == 0.0f && m[13] == 0.0f &&
        m[2] == 0.0f && m[6] == 0.0f && m[3] == 0.0f && m[7] == 0.0f &&
        m[11] == -1.0f && m[15] == 0.0f)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : clipProjectPoints
    Description : projection transform stage
    Inputs      : n - number of verts
                  vEye  - cameraspace coordinates
                  vClip - canonical (clipspace) coordinates
                  clipmask - vertex clipping flags
                  clipormask, clipandmask - clip masks for non/trivial elimination
                  m - projection matrix
    Outputs     : fills in vClip with transformed vEye
    Return      :
    Notes       : assumes a general matrix, ie. very inefficient
----------------------------------------------------------------------------*/
void clipProjectPoints(
    udword n, real32* vEye, real32* vClip, ubyte* clipmask,
    ubyte* clipormask, ubyte* clipandmask, real32* m)
{
    ubyte tmpormask  = *clipormask;
    ubyte tmpandmask = *clipandmask;
    udword i;

    if (clipIsPerspective(m))
    {
        for (i = 0; i < n; i++)
        {
            real32 ex = vEye[4*i + 0];
            real32 ey = vEye[4*i + 1];
            real32 ez = vEye[4*i + 2];
            real32 ew = vEye[4*i + 3];
            real32 cx = m[0] * ex + m[8] * ez;
            real32 cy = m[5] * ey + m[9] * ez;
            real32 cz = m[10] * ez + m[14] * ew;
            real32 cw = -ez;
            GLubyte mask = 0;
            vClip[4*i + 0] = cx;
            vClip[4*i + 1] = cy;
            vClip[4*i + 2] = cz;
            vClip[4*i + 3] = cw;
            if (cx > cw)        mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy > cw)        mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz > cw)        mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipmask[i] |= mask;
                tmpormask |= mask;
            }
            tmpandmask &= mask;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            real32 ex = vEye[4*i + 0];
            real32 ey = vEye[4*i + 1];
            real32 ez = vEye[4*i + 2];
            real32 ew = vEye[4*i + 3];
            real32 cx = m[0] * ex + m[4] * ey + m[8]  * ez + m[12] * ew;
            real32 cy = m[1] * ex + m[5] * ey + m[9]  * ez + m[13] * ew;
            real32 cz = m[2] * ex + m[6] * ey + m[10] * ez + m[14] * ew;
            real32 cw = m[3] * ex + m[7] * ey + m[11] * ez + m[15] * ew;
            ubyte mask = 0;
            vClip[4*i + 0] = cx;
            vClip[4*i + 1] = cy;
            vClip[4*i + 2] = cz;
            vClip[4*i + 3] = cw;
            if (cx > cw)        mask |= CLIP_RIGHT_BIT;
            else if (cx < -cw)  mask |= CLIP_LEFT_BIT;
            if (cy > cw)        mask |= CLIP_TOP_BIT;
            else if (cy < -cw)  mask |= CLIP_BOTTOM_BIT;
            if (cz > cw)        mask |= CLIP_FAR_BIT;
            else if (cz < -cw)  mask |= CLIP_NEAR_BIT;
            if (mask)
            {
                clipmask[i] |= mask;
                tmpormask |= mask;
            }
            tmpandmask &= mask;
        }
    }

    *clipormask = tmpormask;
    *clipandmask = tmpandmask;
}

/*-----------------------------------------------------------------------------
    Name        : clipViewportMap
    Description : clip -> screen space projection, with perspective divide
    Inputs      : n - number of verts
                  vClip - clipspace coordinates
                  clipmask - per-vertex clip masks
                  vWin - screen space coordinates
                  viewportS - viewport scale vector
                  viewportT - viewport translation vector
                  force - ignore clip tests
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clipViewportMap(
    udword n, real32* vClip, ubyte* clipmask, real32* vWin,
    vector* viewportS, vector* viewportT, bool32 force)
{
    real32 sx, tx, sy, ty, sz, tz;
    udword i;

    sx = viewportS->x;
    sy = viewportS->y;
    sz = viewportS->z;
    tx = viewportT->x;
    ty = viewportT->y;
    tz = viewportT->z;

    if (clipmask && !force)
    {
        for (i = 0; i < n; i++)
        {
            if (clipmask[i] == 0)
            {
                if (vClip[4*i + 3] != 0.0f)
                {
                    real32 wInv = 1.0f / vClip[4*i + 3];
                    vWin[3*i + 0] = vClip[4*i + 0] * wInv * sx + tx;
                    vWin[3*i + 1] = vClip[4*i + 1] * wInv * sy + ty;
                    vWin[3*i + 2] = vClip[4*i + 2] * wInv * sz + tz;
                }
                else
                {
                    vWin[3*i + 0] = 0.0f;
                    vWin[3*i + 1] = 0.0f;
                    vWin[3*i + 2] = 0.0f;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (vClip[4*i + 3] != 0.0f)
            {
                real32 wInv = 1.0f / vClip[4*i + 3];
                vWin[3*i + 0] = vClip[4*i + 0] * wInv * sx + tx;
                vWin[3*i + 1] = vClip[4*i + 1] * wInv * sy + ty;
                vWin[3*i + 2] = vClip[4*i + 2] * wInv * sz + tz;
            }
            else
            {
                vWin[3*i + 0] = 0.0f;
                vWin[3*i + 1] = 0.0f;
                vWin[3*i + 2] = 0.0f;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : clipPointToScreen
    Description : tries to clip & project a point from modelview -> screen space
    Inputs      : p - the point
                  screen - holder for output screen coord
                  force - if TRUE, project regardless of clip test
    Outputs     : screen will contain screen projection of given point
    Return      : 0 = (failure) point is off screen
                  1 = (success) point is on screen
----------------------------------------------------------------------------*/
bool32 clipPointToScreen(vector* p, vector* screen, bool32 force)
{
    real32 modelview[16], projection[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    return clipPointToScreenWithMatrices(p, screen, modelview, projection, force);
}

/*-----------------------------------------------------------------------------
    Name        : clipPointToScreenWithMatrices
    Description : tries to clip & project a point from modelview -> screen space
    Inputs      : p - the point
                  screen - holder for output screen coord
                  modelview, projection - matrices
                  force - if TRUE, project regardless of clip test
    Outputs     : screen will contain screen projection of given point
    Return      : 0 = (failure) point is off screen
                  1 = (success) point is on screen
----------------------------------------------------------------------------*/
bool32 clipPointToScreenWithMatrices(
    vector* p, vector* screen, real32* modelview, real32* projection, bool32 force)
{
    hvector vbObj[1];
    hvector vbEye[1];
    hvector vbClip[1];
    vector  vbWin[1];
    vector  viewportS, viewportT;
    ubyte   clipmask[1];
    ubyte   clipormask = 0;
    ubyte   clipandmask = CLIP_ALL_BITS;
    GLint   param[4];

    screen->x = 0.0f;
    screen->y = 0.0f;
    screen->z = 0.0f;

    glGetIntegerv(GL_VIEWPORT, param);

    viewportS.x = (real32)param[2] / 2.0f;
    viewportS.y = (real32)param[3] / 2.0f;
    viewportS.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f);
    viewportT.x = viewportS.x + (real32)param[0];
    viewportT.y = viewportS.y + (real32)param[1];
    viewportT.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f + 0.0f);

    vbObj[0].x = p->x;
    vbObj[0].y = p->y;
    vbObj[0].z = p->z;
    vbObj[0].w = 1.0f;

    clipmask[0] = 0;

    clipTransformPoints(1, (real32*)vbObj, (real32*)vbEye, modelview);
    clipProjectPoints(1, (real32*)vbEye, (real32*)vbClip, clipmask,
                      &clipormask, &clipandmask, projection);

    if (clipandmask && !force)
    {
        return 0;
    }

    clipViewportMap(1, (real32*)vbClip,
                    clipormask ? clipmask : NULL,
                    (real32*)vbWin, &viewportS, &viewportT, force);

    if (clipormask && !force)
    {
        return 0;
    }
    else
    {
        screen->x = vbWin[0].x;
        screen->y = vbWin[0].y;
        screen->z = vbWin[0].z;
        return 1;
    }
}

#if CORRECT_BBOX_CLIP
//helper, determines whether a line is inside the frustum or not
static bool32 _lineinside(
    hvector rectpos[], udword v1, udword v2, ubyte clipmask[], hvector vbClip[])
{
    if (clipmask[v1] | clipmask[v2])
    {
        hvector vectors[8];
        vectors[0].x = vbClip[v1].x;
        vectors[0].y = vbClip[v1].y;
        vectors[0].z = vbClip[v1].z;
        vectors[0].w = vbClip[v1].w;
        vectors[1].x = vbClip[v2].x;
        vectors[1].y = vbClip[v2].y;
        vectors[1].z = vbClip[v2].z;
        vectors[1].w = vbClip[v2].w;
        return clipViewclipLine((real32*)vectors, &v1, &v2);
    }
    else
    {
        return TRUE;
    }
}
#endif //CORRECT_BBOX_CLIP

sdword clipTotallyIn = FALSE;

/*-----------------------------------------------------------------------------
    Name        : clipBBoxIsClipped
    Description : determines whether a given bbox is visible given the current frustum
    Inputs      : collrectoffset - gshaw's bbox array
                  uplength, rightlength, forwardlength - dimensions
    Outputs     :
    Return      : TRUE if totally outside of frustum, FALSE otherwise
----------------------------------------------------------------------------*/
bool32 clipBBoxIsClipped(
    real32* collrectoffset, real32 uplength, real32 rightlength, real32 forwardlength)
{
    vector upvector = {1.0f, 0.0f, 0.0f};
    vector rightvector = {0.0f, 1.0f, 0.0f};
    vector forwardvector = {0.0f, 0.0f, 1.0f};

    hvector rectpos[8]; //vbObj
    hvector vbEye[8];
    hvector vbClip[8];
    ubyte clipmask[8];
    ubyte clipormask = 0;
    ubyte clipandmask = CLIP_ALL_BITS;

    real32 modelview[16], projection[16];

    sdword i;
    bool32 result;

    clipTotallyIn = FALSE;

    rectpos[0].x = collrectoffset[0];
    rectpos[0].y = collrectoffset[1];
    rectpos[0].z = collrectoffset[2];

    vecMultiplyByScalar(upvector, uplength);
    vecMultiplyByScalar(rightvector, rightlength);
    vecMultiplyByScalar(forwardvector, forwardlength);

    for (i = 0; i < 8; i++)
    {
        rectpos[i].w = 1.0f;
        clipmask[i] = 0;
    }

    vecAdd(rectpos[1], rectpos[0], rightvector);
    vecAdd(rectpos[2], rectpos[1], forwardvector);
    vecAdd(rectpos[3], rectpos[0], forwardvector);

    vecAdd(rectpos[4], rectpos[0], upvector);
    vecAdd(rectpos[5], rectpos[1], upvector);
    vecAdd(rectpos[6], rectpos[2], upvector);
    vecAdd(rectpos[7], rectpos[3], upvector);

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    clipTransformPoints(8, (real32*)rectpos, (real32*)vbEye, modelview);
    clipProjectPoints(8, (real32*)vbEye, (real32*)vbClip, clipmask,
                      &clipormask, &clipandmask, projection);

#if CORRECT_BBOX_CLIP
    result = TRUE;
    if (clipormask)
    {
#define LINETEST(v1,v2) if (_lineinside(rectpos, v1, v2, clipmask, vbClip)) { result = FALSE; goto PASSED; }
        LINETEST(0,1);
        LINETEST(1,2);
        LINETEST(2,3);
        LINETEST(3,0);
        LINETEST(4,5);
        LINETEST(5,6);
        LINETEST(6,7);
        LINETEST(7,4);
        LINETEST(0,4);
        LINETEST(1,5);
        LINETEST(2,6);
        LINETEST(3,7);
#undef LINETEST
    }
    else
    {
        result = FALSE;
    }

PASSED:
#else  //inCORRECT_BBOX_CLIP
    result = clipandmask ? TRUE : FALSE;
    if (clipormask == 0)
    {
        clipTotallyIn = TRUE;
    }
#endif //CORRECT_BBOX_CLIP
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : clipLineToScreen
    Description : tries to clip & project vertices from modelview -> screen space
    Inputs      : pa, pb - endpoints of the line (3D)
                  modelview  - current modelview matrix
                  projection - current projection matrix
                  screenA, screenB - final window coordinates (2D)
    Outputs     :
    Return      : 0 = (failure) line is off screen
                  1 = (success) line is on screen
----------------------------------------------------------------------------*/
bool32 clipLineToScreen(
    vector* pa, vector* pb, real32* modelview, real32* projection,
    vector* screenA, vector* screenB)
{
    hvector vbObj[8];
    hvector vbEye[8];
    hvector vbClip[8];
    vector  vbWin[8];
    vector  viewportS, viewportT;
    ubyte clipmask[8];
    ubyte clipormask = 0;
    ubyte clipandmask = CLIP_ALL_BITS;
    udword i;

    GLint param[4];

    glGetIntegerv(GL_VIEWPORT, param);
    viewportS.x = (real32)param[2] / 2.0f;
    viewportS.y = (real32)param[3] / 2.0f;
    viewportS.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f);
    viewportT.x = viewportS.x + (real32)param[0];
    viewportT.y = viewportS.y + (real32)param[1];
    viewportT.z = DEPTH_SCALE * ((1.0f - 0.0f) / 2.0f + 0.0f);

    vbObj[0].x = pa->x;
    vbObj[0].y = pa->y;
    vbObj[0].z = pa->z;
    vbObj[0].w = 1.0f;
    vbObj[1].x = pb->x;
    vbObj[1].y = pb->y;
    vbObj[1].z = pb->z;
    vbObj[1].w = 1.0f;

    for (i = 0; i < 8; i++)
    {
        clipmask[i] = 0;
    }

    clipTransformPoints(2, (real32*)vbObj, (real32*)vbEye, modelview);
    clipProjectPoints(2, (real32*)vbEye, (real32*)vbClip, clipmask,
                      &clipormask, &clipandmask, projection);

    if (clipandmask)
    {
        return 0;
    }

    clipViewportMap(2, (real32*)vbClip,
                    clipormask ? clipmask : NULL,
                    (real32*)vbWin, &viewportS, &viewportT, 0);

    if (clipormask)
    {
        if (clipmask[0] | clipmask[1])
        {
            udword v0, v1;
            real32 ndcX, ndcY, ndcZ;

            v0 = 0;
            v1 = 1;
            if (clipViewclipLine((real32*)vbClip, &v0, &v1) == 0)
            {
                return 0;
            }

            if (vbClip[v0].w != 0.0f)
            {
                real32 wInv = 1.0f / vbClip[v0].w;
                ndcX = vbClip[v0].x * wInv;
                ndcY = vbClip[v0].y * wInv;
                ndcZ = vbClip[v0].z * wInv;
            }
            else
            {
                ndcX = ndcY = ndcZ = 0.0f;
            }

            screenA->x = vbWin[v0].x = ndcX * viewportS.x + viewportT.x;
            screenA->y = vbWin[v0].y = ndcY * viewportS.y + viewportT.y;
            screenA->z = vbWin[v0].z = ndcZ * viewportS.z + viewportT.z;

            if (vbClip[v1].w != 0.0f)
            {
                real32 wInv = 1.0f / vbClip[v1].w;
                ndcX = vbClip[v1].x * wInv;
                ndcY = vbClip[v1].y * wInv;
                ndcZ = vbClip[v1].z * wInv;
            }
            else
            {
                ndcX = ndcY = ndcZ = 0.0f;
            }

            screenB->x = vbWin[v1].x = ndcX * viewportS.x + viewportT.x;
            screenB->y = vbWin[v1].y = ndcY * viewportS.y + viewportT.y;
            screenB->z = vbWin[v1].z = ndcZ * viewportS.z + viewportT.z;
        }
        else
        {
            screenA->x = vbWin[0].x;
            screenA->y = vbWin[0].y;
            screenA->z = vbWin[0].z;
            screenB->x = vbWin[1].x;
            screenB->y = vbWin[1].y;
            screenB->z = vbWin[1].z;
        }
    }
    else
    {
        screenA->x = vbWin[0].x;
        screenA->y = vbWin[0].y;
        screenA->z = vbWin[0].z;
        screenB->x = vbWin[1].x;
        screenB->y = vbWin[1].y;
        screenB->z = vbWin[1].z;
    }

    return 1;
}






#if 0  // Cat2: clipViewclipLine live code differs from binary
/*=== clipViewclipLine (0x434C10) ===*/
signed int __cdecl clipViewclipLine(float *a1, signed int *a2, signed int *a3)
{
  signed int v3;
  signed int v5;
  float *v6;
  int v7;
  float *v8;
  double v10;
  double v11;
  float *v12;
  double v13;
  double v14;
  float *v15;
  float *v16;
  double v17;
  float *v18;
  double v19;
  float *v20;
  double v21;
  float *v22;
  float *v23;
  float *v24;
  double v25;
  float *v26;
  double v27;
  float *v28;
  double v29;
  float *v30;
  float *v31;
  float *v32;
  double v33;
  float *v34;
  double v35;
  float *v36;
  double v37;
  float *v38;
  float *v39;
  float *v40;
  double v41;
  float *v42;
  double v43;
  float *v44;
  double v45;
  float *v46;
  float *v47;
  float *v48;
  double v49;
  float *v50;
  double v51;
  float *v52;
  double v53;
  float *v54;
  float v55;
  float v56;
  float v57;
  float v58;
  float v59;
  float v60;
  float v61;
  float v62;
  float v63;
  float v64;
  float v65;
  float v66;
  float v67;
  float v68;
  float v69;
  float v70;
  float v71;
  float v72;
  float v73;
  float v74;
  float v75;
  float v76;
  float v77;
  float v78;

  v3 = *a3;
  v5 = *a2;
  v6 = &a1[4 * *a2];
  v7 = 2;
  if ( *v6 <= (double)v6[3] )
  {
    v12 = &a1[4 * v3];
    if ( *v12 <= (double)v12[3] )
      goto LABEL_8;
    v3 = 2;
    v13 = *v12 - *v6;
    v56 = v12[3] - v6[3];
    v14 = (*v6 - v6[3]) / (v56 - v13);
    v68 = v14;
    a1[8] = v14 * v13 + *v6;
    a1[9] = (v12[1] - v6[1]) * v68 + v6[1];
    a1[10] = (v12[2] - v6[2]) * v68 + v6[2];
    a1[11] = v68 * v56 + v6[3];
  }
  else
  {
    v8 = &a1[4 * v3];
    if ( *v8 > (double)v8[3] )
      return 0;
    v5 = 2;
    v10 = *v6 - *v8;
    v55 = v6[3] - v8[3];
    v11 = (*v8 - v8[3]) / (v55 - v10);
    v67 = v11;
    a1[8] = v11 * v10 + *v8;
    a1[9] = (v6[1] - v8[1]) * v67 + v8[1];
    a1[10] = (v6[2] - v8[2]) * v67 + v8[2];
    a1[11] = v67 * v55 + v8[3];
  }
  v7 = 3;
LABEL_8:
  v15 = &a1[4 * v5];
  if ( -v15[3] <= *v15 )
  {
    v20 = &a1[4 * v3];
    if ( -v20[3] <= *v20 )
      goto LABEL_15;
    v21 = *v20 - *v15;
    v22 = &a1[4 * v7];
    v3 = v7;
    v58 = v20[3] - v15[3];
    v70 = -((v15[3] + *v15) / (v58 + v21));
    *v22 = v70 * v21 + *v15;
    v22[1] = (v20[1] - v15[1]) * v70 + v15[1];
    v22[2] = (v20[2] - v15[2]) * v70 + v15[2];
    v22[3] = v70 * v58 + v15[3];
  }
  else
  {
    v16 = &a1[4 * v3];
    if ( -v16[3] > *v16 )
      return 0;
    v17 = *v15 - *v16;
    v18 = &a1[4 * v7];
    v57 = v15[3] - v16[3];
    v69 = -((v16[3] + *v16) / (v57 + v17));
    *v18 = v69 * v17 + *v16;
    v18[1] = (v15[1] - v16[1]) * v69 + v16[1];
    v18[2] = (v15[2] - v16[2]) * v69 + v16[2];
    v19 = v69 * v57 + v16[3];
    v5 = v7;
    v18[3] = v19;
  }
  ++v7;
LABEL_15:
  v23 = &a1[4 * v5];
  if ( v23[1] <= (double)v23[3] )
  {
    v28 = &a1[4 * v3];
    if ( v28[1] <= (double)v28[3] )
      goto LABEL_22;
    v29 = v28[1] - v23[1];
    v30 = &a1[4 * v7];
    v3 = v7;
    v60 = v28[3] - v23[3];
    v72 = (v23[1] - v23[3]) / (v60 - v29);
    *v30 = (*v28 - *v23) * v72 + *v23;
    v30[1] = v29 * v72 + v23[1];
    v30[2] = (v28[2] - v23[2]) * v72 + v23[2];
    v30[3] = v72 * v60 + v23[3];
  }
  else
  {
    v24 = &a1[4 * v3];
    if ( v24[1] > (double)v24[3] )
      return 0;
    v25 = v23[1] - v24[1];
    v26 = &a1[4 * v7];
    v59 = v23[3] - v24[3];
    v71 = (v24[1] - v24[3]) / (v59 - v25);
    *v26 = (*v23 - *v24) * v71 + *v24;
    v26[1] = v25 * v71 + v24[1];
    v26[2] = (v23[2] - v24[2]) * v71 + v24[2];
    v27 = v71 * v59 + v24[3];
    v5 = v7;
    v26[3] = v27;
  }
  ++v7;
LABEL_22:
  v31 = &a1[4 * v5];
  if ( -v31[3] <= v31[1] )
  {
    v36 = &a1[4 * v3];
    if ( -v36[3] <= v36[1] )
      goto LABEL_29;
    v37 = v36[1] - v31[1];
    v38 = &a1[4 * v7];
    v3 = v7;
    v62 = v36[3] - v31[3];
    v74 = -((v31[3] + v31[1]) / (v37 + v62));
    *v38 = (*v36 - *v31) * v74 + *v31;
    v38[1] = v37 * v74 + v31[1];
    v38[2] = (v36[2] - v31[2]) * v74 + v31[2];
    v38[3] = v74 * v62 + v31[3];
  }
  else
  {
    v32 = &a1[4 * v3];
    if ( -v32[3] > v32[1] )
      return 0;
    v33 = v31[1] - v32[1];
    v34 = &a1[4 * v7];
    v61 = v31[3] - v32[3];
    v73 = -((v32[3] + v32[1]) / (v33 + v61));
    *v34 = (*v31 - *v32) * v73 + *v32;
    v34[1] = v33 * v73 + v32[1];
    v34[2] = (v31[2] - v32[2]) * v73 + v32[2];
    v35 = v73 * v61 + v32[3];
    v5 = v7;
    v34[3] = v35;
  }
  ++v7;
LABEL_29:
  v39 = &a1[4 * v5];
  if ( v39[2] <= (double)v39[3] )
  {
    v44 = &a1[4 * v3];
    if ( v44[2] <= (double)v44[3] )
      goto LABEL_36;
    v45 = v44[2] - v39[2];
    v46 = &a1[4 * v7];
    v3 = v7;
    v64 = v44[3] - v39[3];
    v76 = (v39[2] - v39[3]) / (v64 - v45);
    *v46 = (*v44 - *v39) * v76 + *v39;
    v46[1] = (v44[1] - v39[1]) * v76 + v39[1];
    v46[2] = v45 * v76 + v39[2];
    v46[3] = v76 * v64 + v39[3];
  }
  else
  {
    v40 = &a1[4 * v3];
    if ( v40[2] > (double)v40[3] )
      return 0;
    v41 = v39[2] - v40[2];
    v42 = &a1[4 * v7];
    v63 = v39[3] - v40[3];
    v75 = (v40[2] - v40[3]) / (v63 - v41);
    *v42 = (*v39 - *v40) * v75 + *v40;
    v42[1] = (v39[1] - v40[1]) * v75 + v40[1];
    v42[2] = v41 * v75 + v40[2];
    v43 = v75 * v63 + v40[3];
    v5 = v7;
    v42[3] = v43;
  }
  ++v7;
LABEL_36:
  v47 = &a1[4 * v5];
  if ( -v47[3] <= v47[2] )
  {
    v52 = &a1[4 * v3];
    if ( -v52[3] > v52[2] )
    {
      v53 = v52[2] - v47[2];
      v54 = &a1[4 * v7];
      v3 = v7;
      v66 = v52[3] - v47[3];
      v78 = -((v47[3] + v47[2]) / (v53 + v66));
      *v54 = (*v52 - *v47) * v78 + *v47;
      v54[1] = (v52[1] - v47[1]) * v78 + v47[1];
      v54[2] = v53 * v78 + v47[2];
      v54[3] = v78 * v66 + v47[3];
    }
  }
  else
  {
    v48 = &a1[4 * v3];
    if ( -v48[3] > v48[2] )
      return 0;
    v49 = v47[2] - v48[2];
    v50 = &a1[4 * v7];
    v65 = v47[3] - v48[3];
    v77 = -((v48[2] + v48[3]) / (v49 + v65));
    *v50 = (*v47 - *v48) * v77 + *v48;
    v50[1] = (v47[1] - v48[1]) * v77 + v48[1];
    v50[2] = v49 * v77 + v48[2];
    v51 = v77 * v65 + v48[3];
    v5 = v7;
    v50[3] = v51;
  }
  *a2 = v5;
  *a3 = v3;
  return 1;
}
#endif


