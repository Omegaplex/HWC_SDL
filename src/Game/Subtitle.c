/*=============================================================================
    Name    : Subtitle.c
    Purpose : Cat3-specific subtitle functions

    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include "font.h"
#include "universe.h"
#include "subtitle.h"

/*=============================================================================
    Functions:
=============================================================================*/

ubyte subRedrawFlags = 0;

/*-----------------------------------------------------------------------------
    Name        : subStringToBool
    Description : Parses a boolean string, skipping leading whitespace/commas.
    Inputs      : boolString - string to parse
    Outputs     :
    Return      : TRUE if string represents a true value
----------------------------------------------------------------------------*/
// Decompiled from cataclysm.exe @ 0x525C40
bool32 subStringToBool(char *boolString)
{
    char *p = boolString;
    char c = *boolString;
    while (c)
    {
        if (!strchr(" \t,\n", c))
            break;
        c = *++p;
    }
    return (*p >= '1' && *p <= '9')
        || !_strcmpi(p, "TRUE")
        || !_strcmpi(p, "YES");
}

/*-----------------------------------------------------------------------------
    Name        : subTitleUpdate
    Description : Updates subtitle cards in a region to extend the current
                    subtitle by the given duration. Cards that have not yet
                    started are set to play indefinitely. Sets bAborted.
    Inputs      : region   - subtitle region to update
                  duration - additional duration to apply
    Outputs     :
    Return      : current card count
----------------------------------------------------------------------------*/
// Decompiled from cataclysm.exe @ 0x584B10
sdword subTitleUpdate(subregion *region, real32 duration)
{
    sdword i;
    sdword cardCount;
    subcard *card;

    cardCount = region->cardIndex;
    card = region->card;
    i = 0;
    if (cardCount > 0)
    {
        do
        {
            if (card->creationTime <= *subTimeElapsed)
            {
                if (*subTimeElapsed - card->creationTime + duration < card->duration)
                {
                    card->fadeOut = duration;
                    card->duration = *subTimeElapsed - card->creationTime + duration;
                }
            }
            else
            {
                card->creationTime = *subTimeElapsed;
                card->duration = -1.0f;
            }
            cardCount = region->cardIndex;
            i++;
            card++;
        }
        while (i < cardCount);
    }
    region->bAborted = TRUE;
    return cardCount;
}

// Whitespace character sets for subWrapTextLines
static char aWrapNewlineSkip[] = " \t\r";    // chars to skip after \n/\r
static char aWrapWordBreak[]   = "\t ";      // word-wrap whitespace chars
static char aWrapTokenSkip[]   = " \t\n\r"; // chars to skip after #n token

/*-----------------------------------------------------------------------------
    Name        : subWrapTextLines
    Description : Wraps a long string into lines within the given rectangle
                    width. Handles #n/\n tokens for explicit line breaks and
                    word-wraps on whitespace.
    Inputs      : region     - bounding rectangle (x0/x1 used for width)
                  font       - font to measure with
                  longLength - length of input string
                  longString - source string
                  wrapBuffer - destination buffer for wrapped text
                  lineOffsets - output array of char* pointers into wrapBuffer
    Outputs     : lineOffsets filled with pointers to each wrapped line
    Return      : number of lines produced
----------------------------------------------------------------------------*/
// Decompiled from cataclysm.exe @ 0x583B00
sdword subWrapTextLines(rectangle *region, fonthandle font, sdword longLength, char *longString, char *wrapBuffer, char **lineOffsets)
{
    sdword lineCount;
    sdword writeOffset;
    sdword width;
    sdword savedWidth;
    fonthandle oldFont;
    char *lineStart;
    char *scan;
    char *nextStart;
    bool32 foundWhitespace;
    char *breakScan;
    sdword lineLen;
    char c;

    lineCount = 0;
    writeOffset = 0;
    width = region->x1 - region->x0 - 1;
    savedWidth = width;
    oldFont = fontCurrentGet();
    lineStart = longString;
    scan = longString;
    nextStart = wrapBuffer;

    fontMakeCurrent(font);
    if (*longString)
    {
        do
        {
            c = *scan;
            if ((c == '#' || c == '\\') && scan[1] == 'n')
            {
                // #n or \n token: skip whitespace after it
                nextStart = scan + 2;
                while (*nextStart && strchr(aWrapTokenSkip, *nextStart))
                    nextStart++;
            }
            else if (c != '\n' && c != '\r')
            {
                if (!c)
                    goto emit_line;
                if (fontWidthN(lineStart, (sdword)(scan - lineStart + 1)) >= width)
                {
                    // Line too wide: back-scan for a word break
                    foundWhitespace = FALSE;
                    breakScan = scan;
                    if (scan > lineStart)
                    {
                        while (1)
                        {
                            if (foundWhitespace)
                            {
                                if (!strchr(aWrapWordBreak, *breakScan))
                                {
                                    char *breakEnd = breakScan + 1;
                                    if (breakEnd > lineStart)
                                    {
                                        scan = breakEnd;
                                        goto emit_line;
                                    }
                                    // fall through: emit to original scan pos
                                    goto set_next_at_scan;
                                }
                            }
                            else if (strchr(aWrapWordBreak, *breakScan))
                            {
                                foundWhitespace = TRUE;
                                nextStart = breakScan + 1;
                            }
                            if (--breakScan <= lineStart)
                                goto set_next_at_scan;
                        }
                    }
set_next_at_scan:
                    nextStart = scan;
                    goto emit_line;
                }
                goto next_char;
            }
            else
            {
                // Literal \n or \r: skip trailing whitespace
                nextStart = scan + 1;
                while (*nextStart && strchr(aWrapNewlineSkip, *nextStart))
                    nextStart++;
            }
emit_line:
            lineLen = (sdword)(scan - lineStart);
            memcpy(wrapBuffer + writeOffset, lineStart, lineLen);
            wrapBuffer[writeOffset + lineLen] = '\0';
            lineOffsets[lineCount] = wrapBuffer + writeOffset;
            writeOffset += lineLen + 1;
            lineCount++;
            lineStart = nextStart;
            scan = nextStart - 1;
            width = savedWidth;
next_char:
            scan++;
        }
        while (*lineStart);
    }
    fontMakeCurrent(oldFont);
    return lineCount;
}

/*-----------------------------------------------------------------------------
    Name        : subBuildScrollingLayout
    Description : Builds subtitle card layout for a scrolling subtitle region.
                    Cards are assigned creation times and durations based on
                    the scroll rate and dwell times.
    Inputs      : region   - subtitle region to fill
                  theme    - theme parameters
                  numLines - number of text lines
                  lines    - array of char* pointing to each line in chopBuffer
                  duration - total subtitle duration in seconds
    Outputs     : region->card[] filled with layout data
    Return      : result of fontMakeCurrent restoring old font
----------------------------------------------------------------------------*/
// Decompiled from cataclysm.exe @ 0x583CD0
sdword subBuildScrollingLayout(subregion *region, subtheme *theme, sdword numLines, char **lines, real32 duration)
{
    sdword lineIndex;
    sdword cardOff;
    sdword i;
    sdword x;
    sdword visibleLines;
    sdword scrollOffLines;
    real32 totalHeight;
    real32 scrollHeight;
    real32 scrollDuration;
    real32 scrollRate;
    real32 creationDelay;
    real32 lineDuration;
    fonthandle oldFont;
    subcard *card;

    oldFont = fontCurrentGet();
    fontMakeCurrent(theme->font);

    visibleLines  = (region->rect.y1 - region->rect.y0) / (fontHeight(" ") + 1);
    scrollOffLines = numLines - visibleLines;
    totalHeight   = (real32)(numLines * (fontHeight(" ") + 1));
    scrollHeight  = (real32)((numLines - visibleLines) * (fontHeight(" ") + 1));
    if (scrollHeight < 0.0f)
        scrollHeight = 0.0f;

    cardOff = 0;
    scrollDuration = duration - subScrollDwellStart - subScrollDwellEnd;
    region->cardIndex = 0;

    if (scrollHeight <= 0.0f)
    {
        scrollRate = 0.0f;
        region->scrollDistance = 0.0f;
        scrollDuration = duration - subScrollDwellStart - subScrollDwellEnd;
    }
    else
    {
        real32 scrollStart = subScrollDwellStart + *subTimeElapsed;
        region->textScrollStart = scrollStart;
        if (scrollDuration >= 0.0f)
            region->textScrollEnd = scrollStart + scrollDuration;
        else
            region->textScrollEnd = scrollStart + subScrollShortest;
        region->scrollDistance = -scrollHeight;
        scrollRate = scrollHeight / (region->textScrollEnd - region->textScrollStart);
    }

    lineIndex = 0;
    i = 0;
    while (lineIndex < numLines)
    {
        if (lineIndex >= visibleLines)
        {
            creationDelay = (real32)((lineIndex - visibleLines) * (fontHeight(" ") + 1)) / scrollRate + subScrollDwellStart;
            if (lineIndex < scrollOffLines)
                lineDuration = (real32)(visibleLines * (fontHeight(" ") + 1)) / scrollRate;
            else
                lineDuration = (totalHeight - (real32)i) / scrollRate + subScrollDwellEnd;
        }
        else
        {
            creationDelay = 0.0f;
            if (lineIndex < scrollOffLines)
                lineDuration = (real32)i / scrollRate + subScrollDwellStart;
            else
                lineDuration = subScrollDwellEnd + subScrollDwellStart + scrollDuration;
        }

        if (theme->bCentred)
            x = (region->rect.x1 - region->rect.x0 - fontWidth(*lines)) / 2;
        else
            x = 0;

        card = (subcard *)((char *)region->card + cardOff);
        card->x            = x;
        card->y            = i;
        card->creationTime = creationDelay + *subTimeElapsed;
        card->duration     = lineDuration;
        card->fadeIn       = theme->fadeIn;
        card->fadeOut      = theme->fadeOut;
        card->c            = theme->textColor;
        card->bDropShadow  = (bool32)theme->bDropShadow;
        card->font         = theme->font;
        card->text         = *lines;

        i += fontHeight(" ") + 1;
        lineIndex++;
        cardOff += sizeof(subcard);
        lines++;
    }

    region->cardIndex = lineIndex;

    if (theme->bPicture)
        region->picture = theme->picture;
    else
        region->picture = (trhandle)-1;

    return fontMakeCurrent(oldFont);
}

/*-----------------------------------------------------------------------------
    Name        : subBuildPagedLayout
    Description : Builds subtitle card layout for a paged subtitle region.
                    Lines are grouped into pages; each page has equal duration.
    Inputs      : region   - subtitle region to fill
                  theme    - theme parameters
                  numLines - number of text lines
                  lines    - array of char* pointing to each line in chopBuffer
                  duration - total subtitle duration in seconds
    Outputs     : region->card[] filled with layout data
    Return      : result of fontMakeCurrent restoring old font
----------------------------------------------------------------------------*/
// Decompiled from cataclysm.exe @ 0x583FD0
sdword subBuildPagedLayout(subregion *region, subtheme *theme, sdword numLines, char **lines, real32 duration)
{
    sdword lineIndex;
    sdword cardOff;
    sdword linesPerPage;
    sdword numPages;
    sdword x;
    real32 durationPerPage;
    real32 pageDelay;
    fonthandle oldFont;
    subcard *card;
    sdword result;

    oldFont = fontCurrentGet();
    fontMakeCurrent(theme->font);

    if (duration <= subTitleShortest)
        duration = subTitleShortest;

    lineIndex = 0;
    region->cardIndex = 0;
    region->scrollDistance = 0.0f;

    linesPerPage = (region->rect.y1 - region->rect.y0) / (fontHeight(" ") + 1);
    if (!linesPerPage)
        linesPerPage = 1;

    numPages = numLines / linesPerPage;
    if (numLines % linesPerPage)
        numPages++;

    durationPerPage = duration / (real32)numPages;

    if (numLines > 0)
    {
        cardOff = 0;
        while (1)
        {
            pageDelay = (lineIndex >= linesPerPage)
                ? (real32)(lineIndex / linesPerPage) * durationPerPage
                : 0.0f;

            if (theme->bCentred)
                x = (region->rect.x1 - region->rect.x0 - fontWidth(*lines)) / 2;
            else
                x = 0;

            card = (subcard *)((char *)region->card + cardOff);
            cardOff += sizeof(subcard);

            card->x            = x;
            card->y            = lineIndex % linesPerPage * (fontHeight(" ") + 1);
            lineIndex++;
            card->creationTime = pageDelay + *subTimeElapsed;
            card->duration     = durationPerPage;
            card->fadeIn       = theme->fadeIn;
            card->fadeOut      = theme->fadeOut;
            card->c            = theme->textColor;
            card->bDropShadow  = (bool32)theme->bDropShadow;
            card->font         = theme->font;
            card->text         = *lines++;

            if (lineIndex >= numLines)
                break;
        }
    }

    region->cardIndex = lineIndex;

    if (theme->bPicture)
        region->picture = theme->picture;
    else
        region->picture = (trhandle)-1;

    result = fontMakeCurrent(oldFont);
    region->bAborted = FALSE;
    return result;
}

/*-----------------------------------------------------------------------------
    Name        : subParseInlineTokens
    Description : Parses inline subtitle tokens (#c, #r<n>, #t<n>, #+) from
                    the beginning of text, updating region/theme/continuous.
    Inputs      : text        - current position in subtitle string
                  pRegion     - output: selected subtitle region
                  pTheme      - output: selected subtitle theme
                  pContinuous - output: continuous-event flag
    Outputs     : pRegion, pTheme, pContinuous updated per tokens found
    Return      : pointer past all consumed tokens
----------------------------------------------------------------------------*/
// Decompiled from cataclysm.exe @ 0x5841D0
char *subParseInlineTokens(char *text, subregion **pRegion, subtheme **pTheme, bool8 *pContinuous)
{
    char c;
    char token;

    while (1)
    {
        c = *text;
        if (c != '#' && c != '\\')
            return text;
        if (!c)
            return text;
        token = text[1];
        switch (token)
        {
            case '+':
                *pContinuous = TRUE;
                text += 2;
                break;
            case 'c':
                *pRegion = &subRegion[STR_CloseCaptioned];
                *pTheme  = &subTheme[STT_CloseCaptioned];
                text += 2;
                break;
            case 'r':
                *pRegion = &subRegion[text[2] - '0'];
                text += 3;
                break;
            case 't':
                {
                    char ch = text[2];
                    if (ch < '0' || ch > '9')
                        *pTheme = &subTheme[ch - 87];
                    else
                        *pTheme = &subTheme[ch - '0'];
                    text += 3;
                }
                break;
            default:
                text += 2;
                break;
        }
    }
}
