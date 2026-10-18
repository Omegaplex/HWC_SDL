/*=============================================================================
    Name    : color.c
    Purpose : Decompiled functions from cataclysm.exe (color module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x554E30
void colHSVToRGB(real32 *R, real32 *G, real32 *B, real32 H, real32 S, real32 V)
{
    real32 integer, fraction, p, q, t;
    if (S == 0.0f)
    {
        *R = *G = *B = V;
    }
    else
    {
        if (H == 1.0f) H = 0.0f;
        H *= 6.0f;
        integer = (real32)floor((double)H);
        fraction = H - integer;
        p = V * (1.0f - S);
        q = V * (1.0f - S * fraction);
        t = V * (1.0f - (S * (1.0f - fraction)));
        switch ((sdword)integer)
        {
            case 0: *R = V; *G = t; *B = p; break;
            case 1: *R = q; *G = V; *B = p; break;
            case 2: *R = p; *G = V; *B = t; break;
            case 3: *R = p; *G = q; *B = V; break;
            case 4: *R = t; *G = p; *B = V; break;
            case 5: *R = V; *G = p; *B = q; break;
        }
    }
}
