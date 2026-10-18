/*=============================================================================
    Name    : Font.c
    Purpose : Functions for loading and drawing fonts.

    Created 7/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "glinc.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "color.h"
#include "prim2d.h"
#include "memory.h"
#include "file.h"
#include "debug.h"
#include "utility.h"
#include "render.h"
#include "font.h"
#include "fontreg.h"
#include "main.h"
#include "glcaps.h"
#include "texreg.h"
#include "glcompat.h"
#include "twiddle.h"
#include "devstats.h"


/*=============================================================================
    Defines:
=============================================================================*/

//spacing between characters within a glfont page
#define GLFONT_Y_SPACING     4
#define GLFONT_X_SPACING     4
//output targas for the glfont pages if !0
#define GLFONT_OUTPUT_TARGAS 0


/*=============================================================================
    Data:
=============================================================================*/

extern udword gDevcaps;

udword fontRedBrightFactor =    FONT_RedBrightFactor;
udword fontGreenBrightFactor =  FONT_GreenBrightFactor;
udword fontBlueBrightFactor =   FONT_BlueBrightFactor;

//current font
fontheader *fontCurrentFont = NULL;
fonthandle  fontCurrent=0;

FontShadowType fontCurrentShadowType = FS_NONE;
color fontCurrentShadowColor;


/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : fontIndexToBitPlaneUpsideDown
    Description : Convert an index-color image to an upside-down bit plane (1-bit image)
    Inputs      : dest - destination buffer to recieve buffer.
                    Must be of size (width + 7) / 8 * height.
                  source - source buffer of color indeces for creating bit plane
                  width,height - size of source buffer
                  colorKey - index of value to be 0 in bit plane.  All other
                    values will create 1.
    Outputs     : Converts source to a 1-bit image in dest
    Return      : void
----------------------------------------------------------------------------*/
void fontIndexToBitPlaneUpsideDown(ubyte *dest, ubyte *source, sdword width, sdword height, ubyte colorKey)
{
    sdword x, y;
    ubyte *pDest;

    for (y = 0; y < height; y++)
    {                                                       //for all rows
        pDest = dest + (height - 1 - y) * (width + 7);      //create pointer to current line
        for (x = 0; x < width; x++, source++)
        {                                                   //for all columns
            if (*source == colorKey)
            {                                               //if key color
                pDest[x] = 0;
            }
            else
            {
                pDest[x] = 255;
            }
        }
    }
}

void fontCharacterCreate(sdword x, sdword y, sdword width, sdword height, ubyte *source, ubyte *dest, sdword sourceWidth, sdword sourceHeight, bool32 antialias)
{
    sdword line;

    memset(dest, 0, width * height);
    for (line = 0; line < height; line++)
    {
        memcpy(dest + ((height-1) - line) * width, source + (y + line) * sourceWidth + x, width);
    }

    if (!antialias)
    {
        ubyte* dp;
        sdword i;

        for (line = 0; line < height; line++)
        {
            dp = dest + width * line;

            for (i = 0; i < width; i++, dp++)
            {
                *dp = (ubyte)((*dp < 6) ? 0 : 15);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glfontCopyChar
    Description : copy an 8bit region of a font image -> 32bit (RGBA)
    Inputs      : dest - destination RGBA image
                  du, dv - upper left coordinates in destination image
                  src - source 8bit image
                  su, sv - upper left coordinates in source image
                  width, height - dimensions of region to copy
                  dPitch, sPitch - pitch of dest, src buffers
    Outputs     : dest region has src region blitted into it
    Return      :
----------------------------------------------------------------------------*/
static void glfontCopyChar(color* dest, sdword du, sdword dv,
                           ubyte* src,  sdword su, sdword sv,
                           sdword width, sdword height,
                           sdword dPitch, sdword sPitch)
{
    sdword y, x;
    color* dp;
    ubyte* sp;

    for (y = 0; y < height; y++)
    {
        dp = (color*)((ubyte*)dest + dPitch*(y+dv) + 4*du);
        sp = src + sPitch*(y+sv) + su;
        for (x = 0; x < width; x++)
        {
            dp[x] = colRGBA(255,255,255, sp[x] * 16);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : glfontPackOntoPage
    Description : attempt to pack a font's characters onto a GL font page.  can
                  fail, in which case page dimensions should be adjusted
    Inputs      : header - fontheader type to convert from
                  glfont - fresh GL font type
                  page - the GL font page the characters are going onto
                  usedHeight - output for bottom of used space
                  lastChar - output for last character packed
    Outputs     : on success, usedHeight will contain height of used space.
                  on failure, usedHeight will contain -1
    Return      : new page on success, partial page on failure
----------------------------------------------------------------------------*/
color* glfontPackOntoPage(fontheader* header, glfontheader* glfont, glfontpage* page, sdword* usedHeight, sdword* lastChar)
{
    sdword index;
    charheader* character;
    glfontcharacter* glchar;
    sdword xIndex, yIndex;
    color* data;
    static sdword charIndex;

    if (header == NULL)
    {
        //reset starting character index
        charIndex = TreatAsUdword(lastChar);
        return NULL;
    }

    //allocate temporary memory
    data = (color*)memAlloc(sizeof(color) * page->width * page->height, "temp glfont page", NonVolatile);
    memset(data, 0, sizeof(color) * page->width * page->height);

    //set indices to upperleft
    xIndex = 0;
    yIndex = 0;

    //create texture from individual character bitmaps
    for (index = charIndex; index < 256; index++)
    {
        if (header->character[index] != NULL)
        {
            //get character pointer
            character = (charheader*)header->character[index];

            //create the GL character

            glchar = &glfont->character[index];
            glchar->page = page;
            if ((xIndex + character->width) >= page->width)
            {
                //eol, advance in y
                xIndex = 0;
                yIndex += header->imageHeight + GLFONT_Y_SPACING;
                if ((yIndex + header->imageHeight) >= page->height)
                {
                    //out of space, return NULL
                    if (usedHeight != NULL)
                    {
                        *usedHeight = -1;
                    }
                    if (lastChar != NULL)
                    {
                        *lastChar = index;
                    }
                    return data;
                }
            }

            //create character
            glchar->u = xIndex;
            glchar->v = yIndex;

            //copy character data
            glfontCopyChar(data, glchar->u, glchar->v,
                           character->bitmap, 0, 0,
                           character->width, character->height,
                           4*page->width, character->width);

            //update x location on font page
            xIndex += character->width + GLFONT_X_SPACING;
        }
    }

    if (usedHeight != NULL)
    {
        *usedHeight = yIndex + header->imageHeight;
    }
    if (lastChar != NULL)
    {
        *lastChar = index;
    }
    return data;
}

/*-----------------------------------------------------------------------------
    Name        : glfontRecreate
    Description : recreate the GL font pages for the given font
    Inputs      : header - fontheader type for the font
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glfontRecreate(fontheader* newHeader)
{
    glfontheader* glfont;
    sdword index;
    glfontpage* page;
    sdword dims[4] = {64, 128, 256, 0};
    sdword usedHeight, lastChar, lastPageChar;
    color* data;
    GLuint handle;

    glfont = (glfontheader*)newHeader->glFont;

    lastPageChar = 0;

    for (glfont->numPages = 1;;)
    {
    PACK_PAGE:
        //reset starting character index
        glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
        page = &glfont->page[glfont->numPages - 1];

        for (index = 0;; index++)
        {
            if (dims[index] == 0)
            {
                //failure, pack at largest size then create new page
                page->width  = 256;
                page->height = 256;
                data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //update starting character index
                lastPageChar = lastChar;

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                glfont->numPages++;
                goto PACK_PAGE;
            }

            page->width  = dims[index];
            page->height = dims[index];

            //attempt to pack onto page
            data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
            if (usedHeight == -1)
            {
                //failure, repack at larger dim

                //reset starting character index
                glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
                memFree(data);
            }
            else
            {
                //success

                //trim bottom of font page if not RIVA 128 (poor square only cap handling)
                if (!bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    page->height = bitHighExponent2(usedHeight);
                }
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                goto PACK_DONE;
            }
        }
    }

PACK_DONE:

    //1 / page.dims
    for (index = 0; index < glfont->numPages; index++)
    {
        glfont->page[index].oneOverWidth  = 1.0f / (real32)glfont->page[index].width;
        glfont->page[index].oneOverHeight = 1.0f / (real32)glfont->page[index].height;
    }
}

#if GLFONT_OUTPUT_TARGAS
#include "tga.h"
static void glfontReverse(ubyte* surf, sdword height, sdword pitch)
{
    ubyte  line[4*256];
    sdword y, top, bot;

    for (y = 0; y < (height/2); y++)
    {
        top = y;
        bot = (height - 1) - y;

        memcpy(line, surf + pitch*top, pitch);
        memcpy(surf + pitch*top, surf + pitch*bot, pitch);
        memcpy(surf + pitch*bot, line, pitch);
    }
}
static void glfontWritePage(fontheader* header, sdword nPage, glfontpage* page, color* data)
{
    char filename[32];

    glfontReverse((ubyte*)data, page->height, 4*page->width);
    sprintf(filename, "font%08X_%d.tga", (udword)header, nPage);

    //note: write must calc alphablend of fontdata
    tgaHeaderWriteRGB(filename, NULL, page->width, page->height);
    tgaBodyWriteRGB(data, page->width, page->height);
    tgaWriteClose();
}
#endif //GLFONT_OUTPUT_TARGAS

/*-----------------------------------------------------------------------------
    Name        : glfontCreate
    Description : create the GL rep of a font (texture pages for entire font)
    Inputs      : header - fontheader type to convert from
                  newHeader - fontheader type in progress of conversion
    Outputs     :
    Return      : a fresh glfontheader, fully loaded
----------------------------------------------------------------------------*/
glfontheader* glfontCreate(fontheader* header, fontheader* newHeader)
{
    glfontheader* glfont;
    sdword index;
    glfontpage* page;
    sdword dims[4] = {64, 128, 256, 0};
    sdword usedHeight, lastChar, lastPageChar;
    color* data;
    GLuint handle;

    glfont = memAlloc(sizeof(glfontheader), "glfont", NonVolatile);
    memset(glfont, 0, sizeof(glfontheader));

    lastPageChar = 0;

    for (glfont->numPages = 1;;)
    {
    PACK_PAGE:
        //reset starting character index
        glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
        page = &glfont->page[glfont->numPages - 1];

        for (index = 0;; index++)
        {
            if (dims[index] == 0)
            {
                //failure, pack at largest size then create new page
                page->width  = 256;
                page->height = 256;
                data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //update starting character index
                lastPageChar = lastChar;

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
#if GLFONT_OUTPUT_TARGAS
                glfontWritePage(header, glfont->numPages, page, data);
#endif
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                glfont->numPages++;
                goto PACK_PAGE;
            }

            page->width  = dims[index];
            page->height = dims[index];

            //attempt to pack onto page
            data = glfontPackOntoPage(newHeader, glfont, page, &usedHeight, &lastChar);
            if (usedHeight == -1)
            {
                //failure, repack at larger dim

                //reset starting character index
                glfontPackOntoPage(NULL, NULL, NULL, NULL, (sdword*)lastPageChar);
                memFree(data);
            }
            else
            {
                //success

                //trim bottom of font page if not RIVA 128 (poor square only cap handling)
                if (!bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    page->height = bitHighExponent2(usedHeight);
                }
                dbgMessagef("\nglfontCreate: page %d [%d.%d]",
                            glfont->numPages, page->width, page->height);

                //create the GL font page
                glGenTextures(1, &handle);
                page->glhandle = handle;
                glBindTexture(GL_TEXTURE_2D, handle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             page->width, page->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
#if GLFONT_OUTPUT_TARGAS
                glfontWritePage(header, glfont->numPages, page, data);
#endif
                if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                memFree(data);

                goto PACK_DONE;
            }
        }
    }

PACK_DONE:

    //1 / page.dims
    for (index = 0; index < glfont->numPages; index++)
    {
        glfont->page[index].oneOverWidth  = 1.0f / (real32)glfont->page[index].width;
        glfont->page[index].oneOverHeight = 1.0f / (real32)glfont->page[index].height;
    }

    return glfont;
}

/*-----------------------------------------------------------------------------
    Name        : glfontDiscard
    Description : free all texobjs in a font, but no memFree of the glfontheader
    Inputs      : glfont - the GL font to work with
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void glfontDiscard(glfontheader* glfont)
{
    sdword index;
    glfontpage* page;

    for (index = 0; index < glfont->numPages; index++)
    {
        page = &glfont->page[index];
        if (page->glhandle != 0)
        {
            glDeleteTextures(1, (GLuint*)&page->glhandle);
            page->glhandle = 0;
        }
    }
}

static GLuint lastGLHandle;

/*-----------------------------------------------------------------------------
    Name        : glfontDisplayCharacter
    Description : display a single character from a GL font
    Inputs      : font - the font
                  ch - the character
                  x, y - location in screenspace
                  c - colour to display the font with
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
bool32 glfontDisplayCharacter(fontheader* font, char ch, sdword x, sdword y, color c)
{
#define VERT(S,T,X,Y) \
    glTexCoord2f((real32)(S), (real32)(T)); \
    glVertex2f(primScreenToGLX(X), primScreenToGLY(Y));

    glfontheader* glfont;
    sdword charIndex;
    glfontcharacter* character;
    charheader* fcharacter;
    glfontpage* page;
    real32 sBegin, sEnd;
    real32 tBegin, tEnd;
    real32 xFrac, yFrac;

    glfont = (glfontheader*)font->glFont;
    dbgAssert(glfont != NULL);

    charIndex = (sdword)((ubyte)ch);
    fcharacter = font->character[charIndex];
    character = &glfont->character[charIndex];
    page = character->page;
    if (page == NULL)
    {
        //no page, no character
        return FALSE;
    }

    //texture coordinates in font page
    sBegin = (real32)character->u * page->oneOverWidth;
    sEnd   = (real32)(character->u + fcharacter->width) * page->oneOverWidth;
    tBegin = (real32)character->v * page->oneOverHeight;
    tEnd   = (real32)(character->v + fcharacter->height) * page->oneOverHeight;

    if (bitTest(gDevcaps, DEVSTAT_YADJUST))
    {
        yFrac = 0.1f * page->oneOverHeight;
        tBegin += yFrac;
        tEnd   -= yFrac;
    }
    if (bitTest(gDevcaps, DEVSTAT_XADJUST))
    {
        xFrac = 0.1f * page->oneOverWidth;
        sBegin += xFrac;
        sEnd   -= xFrac;
    }
    if (bitTest(gDevcaps, DEVSTAT_NEGXADJUST))
    {
        xFrac = 0.5f * page->oneOverWidth;
        sEnd += xFrac;
    }

    //only switch textures if new page differs from last
    if (lastGLHandle != page->glhandle)
    {
        glBindTexture(GL_TEXTURE_2D, page->glhandle);
    }
    glColor4ub(colRed(c), colGreen(c), colBlue(c), colAlpha(c));
    glBegin(GL_QUADS);

    VERT(sBegin, tEnd,
         x + fcharacter->offsetX,
         y + fcharacter->offsetY);
    VERT(sBegin, tBegin,
         x + fcharacter->offsetX,
         y + fcharacter->offsetY + fcharacter->height);
    VERT(sEnd, tBegin,
         x + fcharacter->offsetX + fcharacter->width,
         y + fcharacter->offsetY + fcharacter->height);
    VERT(sEnd, tEnd,
         x + fcharacter->offsetX + fcharacter->width,
         y + fcharacter->offsetY);

    glEnd();

    //set last used texobj
    lastGLHandle = page->glhandle;

    return TRUE;
#undef VERT
}

/*-----------------------------------------------------------------------------
    Name        : glfontDisplayString
    Description : display a string of characters from a GL font
    Inputs      : font - the font
                  string - the string to display
                  x, y - screenspace location to display at
                  c - colour
    Outputs     :
    Return      : TRUE or FALSE (success or failure)
----------------------------------------------------------------------------*/
bool32 glfontDisplayString(fontheader* font, char* string, sdword x, sdword y, color c)
{
    char* charp;
    glfontheader* glfont;
    glfontcharacter* character;
    charheader* fcharacter;
    sdword sx, sy;
    bool32 texOn, blendOn, alphatestOn;
    bool32 rval;
    sdword brights[3];
    sdword additiveOn;
    color bright, colour;

    glfont = (glfontheader*)font->glFont;

    //is there actually a font to display ?
    if (glfont == NULL || glfont->numPages == 0)
    {
        return FALSE;
    }

    //highlighted font colour
    brights[0] = colRed(c) * fontRedBrightFactor / 256;
    brights[1] = colGreen(c) * fontGreenBrightFactor / 256;
    brights[2] = colBlue(c) * fontBlueBrightFactor / 256;
    //clamp
    if (brights[0] > 255) brights[0] = 255;
    if (brights[1] > 255) brights[1] = 255;
    if (brights[2] > 255) brights[2] = 255;
    //set bright color
    bright = colRGB(brights[0], brights[1], brights[2]);

    //set up GL state
    trClearCurrent();
    lastGLHandle = 0;
    texOn = rndTextureEnable(TRUE);
    blendOn = (bool32)glIsEnabled(GL_BLEND);
    alphatestOn = (bool32)glIsEnabled(GL_ALPHA_TEST);
    if (!blendOn) glEnable(GL_BLEND);
    if (alphatestOn) glDisable(GL_ALPHA_TEST);
    additiveOn = rndAdditiveBlends(FALSE);
    rndPerspectiveCorrection(FALSE);
    glShadeModel(GL_FLAT);
    rndTextureEnvironment(RTE_Modulate);

    //initial screen location
    sx = x;
    sy = y;

    rval = TRUE;

    for (charp = string; *charp != '\0'; charp++)
    {
        character = &glfont->character[(ubyte)*charp];
        if (character->page == NULL)
        {
            //this character doesn't exist
            sx += fontCurrentFont->spacing;
            continue;
        }
        //test for highlight escape sequence
        if (*charp == '&')
        {
            charp++;    //advance to next character
            if (*charp == '\0')
            {
                //eos
                break;
            }
            character = &glfont->character[(ubyte)*charp];
            if (*charp == '&')
            {
                //must escape & to get an &
                colour = c;
            }
            else
            {
                //highlight colour
                colour = bright;
            }
        }
        else
        {
            //normal colour
            colour = c;
        }
        fcharacter = font->character[(ubyte)*charp];
        //display the character
        (void)glfontDisplayCharacter(font, *charp, sx, sy, colour);
        //advance screen location
        sx += fcharacter->width + fontCurrentFont->spacing;
    }

    //reset state
    rndAdditiveBlends(additiveOn);
    rndTextureEnable(texOn);
    if (!blendOn) glDisable(GL_BLEND);
    if (alphatestOn) glEnable(GL_ALPHA_TEST);

    return rval;
}

/*-----------------------------------------------------------------------------
    Name        : fontLoad
    Description : Load in a font.
    Inputs      : fileName - name of font file
    Outputs     : allocates memory for font
    Return      : handle to font
----------------------------------------------------------------------------*/
fontheader *fontLoad(char *fileName)
{
    fontfileheader *fileHeader;
    fontheader *header, *newHeader;
    sdword length, index;
    sdword iCharacter;
    charheader *pCharacter;
    charfileheader *pFileCharacter;
    sdword sizeTotal, size, sizeUsed;
    ubyte *bitmapBase;
    bool32 antialias;

    antialias = TRUE;

    length = fileLoadAlloc(fileName, (void **)(&fileHeader), NonVolatile);

#if FONT_ERROR_CHECKING
    if (strcmp(fileHeader->identification, FONT_Identification))
    {                                                       //if wrong header
        dbgFatalf(DBG_Loc, "fontLoad: Invalid header '%s' in file '%s'", fileHeader->identification, fileName);
    }
    if (fileHeader->version != FONT_Version)
    {                                                       //if wrong version
        dbgFatalf(DBG_Loc, "fontLoad: expected version 0x%x, found version 0x%x", FONT_Version, fileHeader->version);
    }
#endif
    header = &fileHeader->header;
    dbgAssert(fileHeader->flags == 0);                          //!!! don't yet support color or anti-aliased fonts
#if FONT_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfontLoad: loaded %d bytes and %d characters from file '%s'", length, header->nCharacters, fileName);
#endif

    header->image += (udword)fileHeader;                    //fix-up image pointer

    header->reserved[0] = (ubyte)antialias;

    //compute size required for 8-bit images of each character
    sizeTotal = 0;
    for (index = iCharacter = 0; index < 256; index++)
    {                                                       //for all possible characters
        if (header->character[index] != NULL)
        {
            (ubyte *)header->character[index] += (udword)fileHeader;//fix up character pointer
            pFileCharacter = (charfileheader *)header->character[index];//get reference to data loaded from disk
            size = (pFileCharacter->width - pFileCharacter->offsetX) *
                   (pFileCharacter->height - pFileCharacter->offsetY);
            sizeTotal += size;
        }
    }
    //allocate data for new header, character indices and 8-bit bitmaps
    newHeader = memAlloc(fontHeaderSize(header->nCharacters) +
                         header->nCharacters * sizeof(charheader) +
                         sizeTotal,
                         "Font header", NonVolatile);

    *newHeader = *header;                                   //duplicate the header
    pCharacter = (charheader *)((ubyte *)newHeader +
                 fontHeaderSize(header->nCharacters));
    bitmapBase = (ubyte *)pCharacter + header->nCharacters * sizeof(charheader);
    sizeUsed = 0;
    for (index = iCharacter = 0; index < 256; index++)
    {                                                       //for all possible characters
        if (header->character[index] != NULL)
        {
            newHeader->character[index] = pCharacter;       //set character * for this character
            pFileCharacter = (charfileheader *)header->character[index];//get reference to data loaded from disk
            pCharacter->width = pFileCharacter->width;      //duplicate attributes
            pCharacter->height = pFileCharacter->height;
            pCharacter->offsetX = pFileCharacter->offsetX;
            pCharacter->offsetY = pFileCharacter->offsetY;
            pCharacter->bitmap = bitmapBase + sizeUsed;
            fontCharacterCreate(pFileCharacter->u + pFileCharacter->offsetX,//create an 8-bit character map
                                pFileCharacter->v + pFileCharacter->offsetY,
                                pFileCharacter->width - pFileCharacter->offsetX,
                                pFileCharacter->height - pFileCharacter->offsetY,
                                header->image, pCharacter->bitmap,
                                header->imageWidth, header->imageHeight,
                                antialias);
            iCharacter++;                                   //increment character index and pointer
            pCharacter++;
            size = (pFileCharacter->width - pFileCharacter->offsetX) *
                   (pFileCharacter->height - pFileCharacter->offsetY);
            sizeUsed += size;

            dbgAssert(sizeUsed <= sizeTotal);                //make sure not too much RAM used
            dbgAssert(iCharacter <= header->nCharacters);    //make sure not too many characters
        }
    }

    newHeader->glFont = (void*)glfontCreate(&fileHeader->header, newHeader);

    memFree(fileHeader);
    return(newHeader);
}

/*-----------------------------------------------------------------------------
    Name        : fontDiscardGL
    Description : discard (free) a font's GL texture handles
    Inputs      : font - font to free
    Outputs     : frees all texture names associated with font
    Return      :
----------------------------------------------------------------------------*/
void fontDiscardGL(fontheader* font)
{
    sdword i;

    for (i = 0; i < 256; i++)
    {
        if (font->glFont != NULL)
        {
            glfontDiscard((glfontheader*)font->glFont);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : fontDiscard
    Description : Discard (free) a font
    Inputs      : font - handle of font to free
    Outputs     : Frees all memory associated with font
    Return      : void
----------------------------------------------------------------------------*/
void fontDiscard(fonthandle font)
{
#if FONT_VERBOSE_LEVEL >= 2
    dbgMessagef("\nfontDiscard: Freeing font 0x%x", font);
#endif
    fontDiscardGL(frFontRegistry[font].fontdat);
    if (frFontRegistry[font].fontdat->glFont != NULL)
    {
        glfontDiscard((glfontheader*)frFontRegistry[font].fontdat->glFont);
        memFree(frFontRegistry[font].fontdat->glFont);
    }
    memFree(frFontRegistry[font].fontdat);  //!!! is this enough???
    frFontRegistry[font].fontdat = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : fontMakeCurrent
    Description : Select font for future drawing operations.
    Inputs      : font - handle of font to make current
    Outputs     : copies font to fontCurrentFont
    Return      :
----------------------------------------------------------------------------*/
fonthandle fontMakeCurrent(fonthandle font)
{
    fonthandle old;
    old = fontCurrent;
    fontCurrent = font;
    fontCurrentFont = frFontRegistry[font].fontdat;
    return(old);
}

/*-----------------------------------------------------------------------------
    Name        : fontCurrentGet
    Description : Get handle of current font
    Inputs      : void
    Outputs     : none
    Return      : handle of current font
----------------------------------------------------------------------------*/
fonthandle fontCurrentGet(void)
{
    return(fontCurrent);
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintCentreCentreRectangle
    Description : prints a text string totally centred using current font in rect
    Inputs      : rect, c, string
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintCentreCentreRectangle(rectangle *rect, color c, char *string)
{
    sdword x;
    sdword width = fontWidth(string);
    sdword y;
    sdword height = fontHeight(NULL);
    sdword rectwidth = rect->x1 - rect->x0;
    sdword rectheight = rect->y1 - rect->y0;

    if (width >= rectwidth)
    {
        x = rect->x0;
    }
    else
    {
        x = ((rectwidth - width) >> 1) + rect->x0;
    }

    if (height >= rectheight)
    {
        y = rect->y0;
    }
    else
    {
        y = ((rectheight - height) >> 1) + rect->y0;
    }

    return fontPrint(x,y,c,string);
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintCentre
    Description : prints a text string using current font, centred
    Inputs      : y, c, string
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintCentre(sdword y, color c, char *string)
{
    sdword x;
    sdword width = fontWidth(string);

    if (width >= MAIN_WindowWidth)
    {
        x = 0;
    }
    else
    {
        x = (MAIN_WindowWidth - width) >> 1;
    }

    return fontPrint(x,y,c,string);
}

#define SHADOW_COLOR_ADJUST 2

/*-----------------------------------------------------------------------------
    Name        : fontPrint
    Description : Print a text string using current font
    Inputs      : x, y - location of top-left corner of font
                  c - color to draw text in
                  string - string to render
    Outputs     : Creates a quadrangle texture for each texture.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrint(sdword x, sdword y, color c, char *string)
{
    return fontPrintN(x, y, c, string, SDWORD_Max);
}

/*-----------------------------------------------------------------------------
//
//  set font shadow type for subsequent text output
//  (you may OR multiple shadow directions together)
//
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    Name        : fontShadowSet
    Description : Set current dropshadow properties
    Inputs      : s - shadow cardinal direction or FS_NONE for none
                  c - color to draw drop shadow in
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void fontShadowSet(FontShadowType s, color c)
{
    fontCurrentShadowType = s;
    fontCurrentShadowColor = c;
}

/*-----------------------------------------------------------------------------
//
//  returns current shadow type
//
----------------------------------------------------------------------------*/
FontShadowType fontShadowGet(void)
{
    return fontCurrentShadowType;
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintN
    Description : Print a text string using current font
    Inputs      : x, y - location of top-left corner of font
                  c - color to draw text in
                  string - string to render
                  maxCharacters - max number of characters to print
    Outputs     : Creates a quadrangle texture for each texture.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintN(sdword x, sdword y, color c, char *string, sdword maxCharacters)
{
    charheader *character;
    real32 imageWidth, imageHeight, clipHeight = 0.0f;
    sdword clip = 0;
    ubyte ch;
    udword brightRed, brightGreen, brightBlue;
    udword red, green, blue;
    sdword xOffset, yOffset;
    FontShadowType saveType;

    if (fontCurrentShadowType != FS_NONE)
    {
        // shadow the text as requested
        xOffset = yOffset = 0;
        switch (fontCurrentShadowType)
        {
            case FS_N :
                yOffset = -1;
                break;
            case FS_S :
                yOffset = 1;
                break;
            case FS_E :
                xOffset = 1;
                break;
            case FS_W :
                xOffset = -1;
                break;
            case FS_NE:
                yOffset = -1;
                xOffset = 1;
                break;
            case FS_NW:
                yOffset = -1;
                xOffset = -1;
                break;
            case FS_SE:
                yOffset = 1;
                xOffset = 1;
                break;
            case FS_SW:
                yOffset = 1;
                xOffset = -1;
                break;
        }
        saveType = fontCurrentShadowType;
        fontCurrentShadowType = FS_NONE;
        fontPrintN(x + xOffset, y + yOffset, fontCurrentShadowColor, string, SDWORD_Max);
        fontCurrentShadowType = saveType;
    }

    //display the GL font-page version if we "should" and "can"
    if (RGLtype != SWtype && !glcActive() &&
        glfontDisplayString(fontCurrentFont, string, x, y, c))
    {
        return TRUE;
    }

#if FONT_MANUAL_CLIP
/*
    if (x <= (-MAIN_WindowWidth) || x >= MAIN_WindowWidth || y <= (-MAIN_WindowHeight) || y >= MAIN_WindowHeight)
    {                                                       //if coordinates not reasonable
        return(ERROR);
    }
*/
    if (y <= -fontCurrentFont->fullHeight)
    {
        return(ERROR);
    }
    if (MAIN_WindowHeight - y <= fontCurrentFont->fullHeight)
    {                                                       //if partly off bottom
        if (y >= MAIN_WindowHeight)
        {                                                   //if fully off bottom
            return(ERROR);
        }
/*
        else
        {
            clipHeight = (real32)(MAIN_WindowHeight - y);
            clip = MAIN_WindowHeight - y;
        }
*/
    }
#endif

    dbgAssert(fontCurrentFont != NULL);
    red = brightRed = colRed(c);
    green = brightGreen = colGreen(c);
    blue = brightBlue = colBlue(c);
    glColor3ub((ubyte)brightRed, (ubyte)brightGreen, (ubyte)brightBlue);
    brightRed = brightRed * fontRedBrightFactor / 256;
    if (brightRed > 255)
    {
        brightRed = 255;
    }
    brightGreen = brightGreen * fontGreenBrightFactor / 256;
    if (brightGreen > 255)
    {
        brightGreen = 255;
    }
    brightBlue = brightBlue * fontBlueBrightFactor / 256;
    if (brightBlue > 255)
    {
        brightBlue = 255;
    }
#if FONT_COLOR_HACK
    glBegin(GL_POINTS);
    glVertex2f(primScreenToGLX(-1), primScreenToGLY(-1));   //work around color bug in 3DFxGL
    glEnd();
#endif

    imageWidth = (real32)fontCurrentFont->imageWidth;       //get floating-point image dimesions
    imageHeight = (real32)fontCurrentFont->imageHeight;
    while (*string && maxCharacters > 0)
    {                                                       //while not at end of string
#if FONT_ALL_CAPS
        ch = toupper(*string);
#else
        ch = *string;
#endif
        if (ch == '&')
        {
            string++;
#if FONT_ALL_CAPS
            ch = toupper(*string);
#else
            ch = *string;
#endif
            if (ch == 0)
            {
                break;
            }
            else if (ch != '&')
            {
                glColor3ub((ubyte)brightRed, (ubyte)brightGreen, (ubyte)brightBlue);
            }
        }
        else
        {
            glColor3ub((ubyte)red, (ubyte)green, (ubyte)blue);
        }
        character = fontCurrentFont->character[(ubyte)ch]; //get character header
        if (character == NULL)
        {                                                   //if character not supported
            x += fontCurrentFont->spacing;                  //put a space instead of a character
            string++;                                       //and character pointer
            maxCharacters--;
            continue;
        }
        if (x < 0)
        {                                                   //if off left
            goto noDraw;
        }
        if (x > MAIN_WindowWidth - character->width)
        {                                                   //if off right
            goto noDraw;
        }

        glRasterPos2f(primScreenToGLX(x + character->offsetX),
                      primScreenToGLY(y + fontCurrentFont->fullHeight + character->offsetY - clip));

        glDrawPixels(character->width, character->height - clip, GL_RGB8, GL_UNSIGNED_BYTE, character->bitmap);

noDraw:
        x += character->width + fontCurrentFont->spacing;   //update screen location
        string++;                                           //and character pointer
        maxCharacters--;
    }

    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : fontPrintf
    Description : Print a formatted text string using current font
    Inputs      : Same as font print except for format string and format params
    Outputs     :
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword fontPrintf(sdword x, sdword y, color c, char *format, ...)
{
    char buffer[DBG_BufferLength];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);
    return(fontPrint(x, y, c, buffer));                     //actually print
}

/*-----------------------------------------------------------------------------
    Name        : fontWidthN
    Description : Get the width, in pixels of a string up to a maximum number of letters
    Inputs      : string - string to measure
                  maxCharacters - maximum number of characters
    Outputs     :
    Return      : Width in pixels of string.  This will be the same number of
                    pixels used if the string is printed.
----------------------------------------------------------------------------*/
sdword fontWidthN(char *string, sdword maxCharacters)
{
    charheader *character;
    ubyte ch;
    sdword width = 0;

    while (*string && maxCharacters > 0)
    {                                                       //while not at end of string or exceeding a number of characters
#if FONT_ALL_CAPS
        ch = toupper(*string);
#else
        ch = *string;
#endif
        if (ch == '&')
        {
            string++;
#if FONT_ALL_CAPS
            ch = toupper(*string);
#else
            ch = *string;
#endif
            if (ch == 0)
            {
                break;
            }
        }
//        dbgAssert((sdword)ch < fontCurrentFont->nCharacters);
        character = fontCurrentFont->character[(ubyte)ch];  //get character header
        if (character == NULL)
        {                                                   //if character not supported
            width += fontCurrentFont->spacing;              //put a space instead of a character
            string++;                                       //and character pointer
            maxCharacters--;
            continue;
        }
        width += character->width + fontCurrentFont->spacing;//update screen location
        string++;                                           //and character pointer
        maxCharacters--;
    }
    return(width);
}

/*-----------------------------------------------------------------------------
    Name        : fontWidth
    Description : Get the width, in pixels of a string
    Inputs      :

    Outputs     :
    Return      : Width in pixels of string.  This will be the same number of
                    pixels used if the string is printed.
----------------------------------------------------------------------------*/
sdword fontWidth(char *string)
{
    return(fontWidthN(string, SDWORD_Max));
}

/*-----------------------------------------------------------------------------
    Name        : fontWidthf
    Description : Width of a formatted string
    Inputs      :
    Outputs     :
    Return      : Windth, in pixels required to print this formatted string.
----------------------------------------------------------------------------*/
sdword fontWidthf(char *format, ...)
{
    char buffer[DBG_BufferLength];
    va_list argList;
    va_start(argList, format);                              //get first arg
    vsprintf(buffer, format, argList);                      //prepare output string
    va_end(argList);
    return(fontWidth(buffer));                              //actual width
}

/*-----------------------------------------------------------------------------
    Name        : fontHeight/fontHeightf
    Description : Height of a printed font (same regardless of string)
    Inputs      :
    Outputs     :
    Return      : Height of currently selected font.
----------------------------------------------------------------------------*/
sdword fontHeight(char *string)
{
    return(fontCurrentFont->fullHeight);
}
sdword fontHeightf(char *format, ...)
{
    return(fontCurrentFont->fullHeight);
}


#if 0  // Cat2: glfontDisplayString live code differs from binary or raw IDA offsets
/*=== glfontDisplayString (0x557be0) ===*/
signed int __cdecl glfontDisplayString(int a1, unsigned __int8 *a2, int a3, int a4, unsigned int a5, int a6)
{
  _DWORD *v6; // esi
  unsigned int v7; // edx
  unsigned int v8; // ecx
  unsigned int v9; // eax
  int IsEnabled; // ebp
  unsigned __int8 *v11; // ebx
  int v13; // ecx
  unsigned __int8 v14; // al
  unsigned int v15; // ecx
  int v16; // esi
  int v17; // eax
  unsigned int v20; // [esp+14h] [ebp-14h]
  _DWORD *v21; // [esp+18h] [ebp-10h]
  int v22; // [esp+20h] [ebp-8h]
  int v23; // [esp+24h] [ebp-4h]
  int v24; // [esp+40h] [ebp+18h]

  v6 = *(_DWORD **)(a1 + 40);
  v21 = v6;
  if ( !v6 || !*v6 )
    return 0;
  v7 = (dword_8B7808 * (unsigned int)(unsigned __int8)a5) >> 8;
  v8 = (dword_8B780C * (unsigned int)BYTE1(a5)) >> 8;
  v9 = (dword_8B7810 * (unsigned int)BYTE2(a5)) >> 8;
  if ( v7 > 0xFF )
    LOBYTE(v7) = -1;
  if ( (dword_8B780C * (unsigned int)BYTE1(a5)) >> 8 > 0xFF )
    LOBYTE(v8) = -1;
  if ( (dword_8B7810 * (unsigned int)BYTE2(a5)) >> 8 > 0xFF )
    LOBYTE(v9) = -1;
  v20 = (unsigned __int8)v7 | (((unsigned __int8)v8 | ((v9 | 0xFFFFFF00) << 8)) << 8);
  trClearCurrent();
  lastGLHandle = 0;
  v22 = rndTextureEnable(1);
  IsEnabled = (unsigned __int8)glIsEnabled(3042);
  v23 = (unsigned __int8)glIsEnabled(3008);
  if ( v23 )
    dword_B3A4B0(3008);
  rndPerspectiveCorrection(0);
  glShadeModel(7424);
  rndTextureEnvironment(0);
  if ( !IsEnabled )
    glEnable(3042);
  v11 = a2;
  v24 = rndAdditiveBlends(a6 != 0);
  if ( *a2 )
  {
    v13 = dword_A70FE8;
    do
    {
      v14 = *v11;
      if ( v6[2 * *v11 + 25] )
      {
        if ( v14 != 38 )
          goto LABEL_20;
        v14 = *++v11;
        if ( !v14 )
          break;
        v15 = v20;
        if ( v14 == 38 )
LABEL_20:
          v15 = a5;
        v16 = *(_DWORD *)(a1 + 4 * v14 + 56);
        glfontDisplayCharacter(a1, v14, a3, a4, v15);
        v13 = dword_A70FE8;
        v17 = *(_DWORD *)(dword_A70FE8 + 4) + *(__int16 *)(v16 + 4);
        v6 = v21;
        a3 += v17;
      }
      else
      {
        a3 += *(_DWORD *)(v13 + 4);
      }
    }
    while ( *++v11 );
  }
  rndAdditiveBlends(v24);
  rndTextureEnable(v22);
  if ( !IsEnabled )
    dword_B3A4B0(3042);
  if ( v23 )
    glEnable(3008);
  return 1;
}
#endif

#if 0  // Cat3: fontLoad
/*=== fontLoad (0x557e00) ===*/
char *__cdecl fontLoad(unsigned int a1)
{
  int *v1; // ebx
  __int16 **v2; // ebp
  int v3; // edx
  __int16 **v4; // ecx
  __int16 *v5; // eax
  char *v6; // eax
  char *v7; // edi
  int v8; // ecx
  __int16 *v9; // esi
  int v11; // [esp+10h] [ebp-14h]
  int v12; // [esp+10h] [ebp-14h]
  int v13; // [esp+14h] [ebp-10h]
  char *v14; // [esp+18h] [ebp-Ch]
  int v15; // [esp+1Ch] [ebp-8h]
  int v16; // [esp+20h] [ebp-4h]

  fileLoadAlloc((char *)a1, (void **)&a1, 8);
  v1 = (int *)(a1 + 12);
  v2 = (__int16 **)(a1 + 68);
  *(_DWORD *)(a1 + 48) += a1;
  *((_BYTE *)v1 + 44) = 1;
  v3 = 0;
  v4 = v2;
  v11 = 256;
  do
  {
    if ( *v4 )
    {
      v5 = (__int16 *)((char *)*v4 + a1);
      *v4 = v5;
      v3 += (v5[2] - v5[4]) * (v5[3] - v5[5]);
    }
    ++v4;
    --v11;
  }
  while ( v11 );
  v6 = (char *)memAllocFunction(v3 + 12 * (*v1 + 90), MBF_NonVolatile);
  qmemcpy(v6, v1, 0x438u);
  v7 = v6 + 1080;
  v8 = v6 - (char *)v1;
  v14 = v6;
  v15 = (int)&v6[12 * *v1 + 1080];
  v12 = 0;
  v16 = v6 - (char *)v1;
  v13 = 256;
  do
  {
    if ( *v2 )
    {
      *(__int16 **)((char *)v2 + v8) = (__int16 *)v7;
      v9 = *v2;
      *((_WORD *)v7 + 2) = (*v2)[2];
      *((_WORD *)v7 + 3) = v9[3];
      *((_WORD *)v7 + 4) = v9[4];
      *((_WORD *)v7 + 5) = v9[5];
      *(_DWORD *)v7 = v12 + v15;
      fontCharacterCreate(
        v9[4] + *v9,
        v9[5] + v9[1],
        v9[2] - v9[4],
        v9[3] - v9[5],
        v1[9],
        (char *)(v12 + v15),
        v1[5],
        v1[6],
        (_BYTE *)1);
      v8 = v16;
      v7 += 12;
      v12 += (v9[2] - v9[4]) * (v9[3] - v9[5]);
      v6 = v14;
    }
    ++v2;
    --v13;
  }
  while ( v13 );
  *((_DWORD *)v14 + 10) = glfontCreate(a1 + 12, (int)v6);
  memFree((ubyte *)a1);
  return v14;
}
#endif

#if 0  // Cat2: fontDiscardGL live code differs from binary or raw IDA offsets
/*=== fontDiscardGL (0x557fb0) ===*/
int __cdecl fontDiscardGL(int a1)
{
  int *v2; // edi
  int result; // eax
  int v4; // ebx
  _DWORD *v5; // esi
  int v6; // [esp+14h] [ebp+4h]

  v6 = 256;
  do
  {
    v2 = *(int **)(a1 + 40);
    if ( v2 )
    {
      result = *v2;
      v4 = 0;
      if ( *v2 > 0 )
      {
        v5 = v2 + 5;
        do
        {
          if ( *v5 )
          {
            glDeleteTextures(1, v5);
            *v5 = 0;
          }
          result = *v2;
          ++v4;
          v5 += 6;
        }
        while ( v4 < *v2 );
      }
    }
    --v6;
  }
  while ( v6 );
  return result;
}
#endif

#if 0  // Cat2: fontShadowSet live code differs from binary or raw IDA offsets
/*=== fontShadowSet (0x558270) ===*/
int __cdecl fontShadowSet(int a1, int a2, unsigned __int8 *a3, int a4)
{
  return fontPrintN(a1, a2, a3, (char *)1, a4, 0x7FFFFFFF);
}
#endif

#if 0  // Cat2: fontPrintN live code differs from binary or raw IDA offsets
/*=== fontPrintN (0x5582c0) ===*/
// positive sp value has been detected, the output may be wrong!
int __cdecl fontPrintN(signed int a1, int a2, unsigned __int8 *a3, char *a4, int a5, int a6)
{
  char v6; // bl
  int v7; // eax
  int v8; // ecx
  int v9; // esi
  unsigned int v11; // ebp
  unsigned int v12; // edx
  char *v13; // ebx
  char v14; // cl
  int v15; // eax
  char v16; // cl
  int v17; // esi
  double v18; // st7
  double v19; // st7
  int v20; // [esp+18h] [ebp-24h]
  float v21; // [esp+18h] [ebp-24h]
  int v22; // [esp+1Ch] [ebp-20h]
  float v23; // [esp+1Ch] [ebp-20h]
  int v24; // [esp+2Ch] [ebp-10h]
  int v25; // [esp+30h] [ebp-Ch]
  int v26; // [esp+34h] [ebp-8h]
  int v27; // [esp+38h] [ebp-4h]
  char *retaddr; // [esp+3Ch] [ebp+0h]
  unsigned int v29; // [esp+44h] [ebp+8h]
  int v30; // [esp+48h] [ebp+Ch]

  v6 = dword_A70FF0;
  if ( (_BYTE)dword_A70FF0 )
  {
    v7 = 0;
    v8 = 0;
    switch ( (unsigned __int8)dword_A70FF0 )
    {
      case 1u:
        v7 = -1;
        break;
      case 2u:
        v7 = 1;
        break;
      case 4u:
        goto LABEL_8;
      case 8u:
        goto LABEL_10;
      case 0x10u:
        v7 = -1;
        v8 = 1;
        break;
      case 0x20u:
        v7 = -1;
        goto LABEL_10;
      case 0x40u:
        v7 = 1;
LABEL_8:
        v8 = 1;
        break;
      case 0x80u:
        v7 = 1;
LABEL_10:
        v8 = -1;
        break;
      default:
        break;
    }
    v9 = (int)retaddr;
    LOBYTE(dword_A70FF0) = 0;
    fontPrintN(v27 + v8, &retaddr[v7], dword_B3A72C, a2, a3, 0x7FFFFFFF);
    LOBYTE(dword_A70FF0) = v6;
  }
  else
  {
    v9 = (int)retaddr;
  }
  if ( RGLtype && !glcActive() && glfontDisplayString(dword_A70FE8, a3, v27, v9, a1, a2) )
    return 1;
  if ( v9 > -*(_DWORD *)(dword_A70FE8 + 8)
    && (MAIN_WindowHeight - v9 > *(_DWORD *)(dword_A70FE8 + 8) || v9 < MAIN_WindowHeight) )
  {
    v25 = (unsigned __int8)a1;
    v24 = BYTE1(a1);
    glColor3ub((unsigned __int8)a1, BYTE1(a1), BYTE2(a1), v20);
    v11 = ((unsigned int)(unsigned __int8)a1 * dword_8B7808) >> 8;
    if ( v11 > 0xFF )
      v11 = 255;
    v29 = ((unsigned int)BYTE1(a1) * dword_8B780C) >> 8;
    if ( v29 > 0xFF )
      v29 = 255;
    v12 = ((unsigned int)BYTE2(a1) * dword_8B7810) >> 8;
    v30 = v12;
    if ( v12 > 0xFF )
    {
      v30 = 255;
      v12 = 255;
    }
    v13 = a4;
    v14 = *a4;
    if ( *a4 )
    {
      v15 = dword_A70FE8;
      while ( 1 )
      {
        if ( a5 <= 0 )
          return 0;
        if ( v14 != 38 )
          break;
        v16 = *++v13;
        if ( !v16 )
          return 0;
        if ( v16 != 38 )
        {
          glColor3ub(v11, v29, v12, v22);
LABEL_35:
          v15 = dword_A70FE8;
        }
        v17 = *(_DWORD *)(v15 + 4 * (unsigned __int8)a5 + 56);
        if ( v17 )
        {
          if ( a1 >= 0 && a1 <= MAIN_WindowWidth - *(__int16 *)(v17 + 4) )
          {
            v18 = (double)(int)(v29 + *(_DWORD *)(v15 + 8) + *(__int16 *)(v17 + 10)) + flt_72A858;
            v23 = ONE - (v18 / (double)(int)MAIN_WindowHeight + v18 / (double)(int)MAIN_WindowHeight);
            v19 = ((double)(a1 + *(__int16 *)(v17 + 8)) + flt_72A858) / (double)(int)MAIN_WindowWidth;
            v21 = v19 + v19 - ONE;
            glRasterPos2f(LODWORD(v21), LODWORD(v23));
            glDrawPixels(*(__int16 *)(v17 + 4), *(__int16 *)(v17 + 6), 32849, 5121, *(_DWORD *)v17);
            v15 = dword_A70FE8;
          }
          a1 += *(_DWORD *)(v15 + 4) + *(__int16 *)(v17 + 4);
        }
        else
        {
          a1 += *(_DWORD *)(v15 + 4);
        }
        ++v13;
        --a6;
        v14 = *v13;
        if ( !*v13 )
          return 0;
        v12 = v30;
      }
      glColor3ub(v26, v25, v24, v22);
      goto LABEL_35;
    }
  }
  return 0;
}
#endif
