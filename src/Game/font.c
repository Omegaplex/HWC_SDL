/*=============================================================================
    Name    : font.c
    Purpose : Decompiled functions from cataclysm.exe (font module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x557030
void fontCharacterCreate(sdword x, sdword y, sdword width, sdword height, ubyte *source, ubyte *dest, sdword sourceWidth, sdword sourceHeight, ubyte *antialias)
{
    int rows;
    signed int cols;
    char *srcRow;
    char *dstRow;
    char *result;
    char *pixel;
    signed int remaining;
    int srcStride;

    rows = height;
    cols = width;
    memset(dest, 0, height * width);
    if ( height > 0 )
    {
        srcRow = (char *)(source + x + sourceWidth * y);
        dstRow = &dest[width * (height - 1)];  // start at bottom row (vertical flip)
        srcStride = height;
        while ( 1 )
        {
            qmemcpy(dstRow, srcRow, cols);
            srcRow += sourceWidth;
            dstRow -= width;
            if ( !--srcStride )
                break;
            cols = width;
        }
        rows = height;
        cols = width;
    }
    result = antialias;
    if ( !antialias && rows > 0 )
    {
        pixel = dest;
        do
        {
            result = pixel;
            if ( cols > 0 )
            {
                remaining = cols;
                do
                {
                    *result = (unsigned char)*result < 6u ? 0 : 15;
                    ++result;
                    --remaining;
                }
                while ( remaining );
            }
            pixel += cols;
            --rows;
        }
        while ( rows );
    }
}
