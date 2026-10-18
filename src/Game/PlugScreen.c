/*=============================================================================
    Name    : PlugScreen.c
    Purpose : Code to handle screens with sales info and web links.

    Created 3/31/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "glinc.h"
#include "task.h"
#include "region.h"
#include "color.h"
#include "prim2d.h"
#include "interfce.h"
#include "texreg.h"
#include "fontreg.h"
#include "statscript.h"
#include "feflow.h"
#include "mouse.h"
#include "utility.h"
#include "soundevent.h"
#include "strings.h"
#include "PlugScreen.h"
#include "render.h"
#include "glcompat.h"
#include "subtitle.h"

/*=============================================================================
    Data:
=============================================================================*/
char *psLanguageString[] = {FR_English, FR_French, FR_German, FR_Spanish, FR_Italian};
udword psGlobalFlags;
regionhandle psBaseRegion = NULL;               //base region, the actual plug screen bitmap
taskhandle psRenderTask;                        //rendering task which replaces standard render task when here
char psDirectory[80];
psimage psScreenImage;

//whether we have to reactivate the glcompat module on psModeEnd
static bool32 psGLCompat = FALSE;

//fading stuff
psimage psFadeImage;
sdword psFadeLevel = UBYTE_Max;
sdword psLastFadeLevel;
//bool8 psFadingToBlack;
//bool8 psFadingFromBlack;
sdword psFadeState;
pluglink *psFadeLink = NULL;
real32 psFadeStartTime;
real32 psFadeTime = PS_FadeTime;

void psImageSet(char *directory,char *field,void *dataToFillIn);
void psPlugLinkSet(char *directory,char *field,void *dataToFillIn);
void psTimeoutSet(char *directory,char *field,void *dataToFillIn);
void psCrossFadeLinkSet(char *directory,char *field,void *dataToFillIn);

//for timing out a screen
real32 psScreenCreationTime;
real32 psScreenTimeout;                         //duration of screen or 0 if no timout
char psTimeoutName[80];                         //name of screen to link to or "exit" if it's an exit screen

//is mouse enabled for current screen?
bool32 psMouseFlag;

scriptEntry psScreenTweaks[] =
{
    { "Image",           psImageSet,    NULL },
    { "CrossFadeLink",   psCrossFadeLinkSet, NULL },
    { "ScreenLink",      psPlugLinkSet, (void *)PLT_Screen },
    { "WebLink",         psPlugLinkSet, (void *)PLT_URL },
    { "ExitLink",        psPlugLinkSet, (void *)PLT_Exit },
    { "GameLink",        psPlugLinkSet, (void *)PLT_GameOn },
    { "TimeOut",         psTimeoutSet,  NULL },
    { "Mouse",           scriptSetBool, &psMouseFlag },
    endEntry
};

/*=============================================================================
    Private functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : psImageDraw
    Description : Draw a plugscreen image
    Inputs      : image - image to draw
                  c - color to render image in
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psImageDraw(psimage *image, color c)
{
    sdword x, y, xStart, yStart;
    udword *quiltTexture = image->imageQuilt;
    rectangle rect;

    dbgAssert(image->width != 0);
    dbgAssert(image->imageQuilt != NULL);
    xStart = (MAIN_WindowWidth - image->width) / 2;
    yStart = (MAIN_WindowHeight - image->height) / 2;

    for (y = 0; y < image->height / PS_QuiltPieceHeight; y++)
    {
        rect.y0 = yStart + y * PS_QuiltPieceHeight;
        rect.y1 = rect.y0 + PS_QuiltPieceHeight;
        for (x = 0; x < image->width / PS_QuiltPieceWidth; x++, quiltTexture++)
        {
            rect.x0 = xStart + x * PS_QuiltPieceWidth;
            rect.x1 = rect.x0 + PS_QuiltPieceWidth;
            trRGBTextureMakeCurrent(*quiltTexture);
            primRectSolidTexturedFullRectC2(&rect, c);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : psLinkDraw
    Description : Render callback for plugscreen links.
    Inputs      : region - region we're rendering
    Outputs     : ...user defined...
    Return      : void
----------------------------------------------------------------------------*/
void psLinkDraw(regionhandle region)
{
    pluglink *link = (pluglink *)region;
    udword handle = (bitTest(region->status, RSF_MouseInside)) ? link->onTexture : link->offTexture;
    lifheader *header = (bitTest(region->status, RSF_MouseInside)) ? link->onImage : link->offImage;
    rectangle screenRect = {plugXMargin-1, plugYMargin, plugXMargin + max(psScreenImage.width, region->rect.x1), plugYMargin + max(psScreenImage.height, region->rect.y1)};
    char linkName[80];

    trPalettedTextureMakeCurrent(handle, header->palette);
    primRectSolidTextured2(&region->rect);

    if (bitTest(region->userID, PLF_FadeRegion))
    {                                                       //if this is the fader region
        if (psFadeState == PFS_ToBlack || psFadeState == PFS_CrossFade)
        {                                                   //if we're fading out to black
            //psFadeLevel += PLF_FadeRate;
            psFadeLevel = (sdword)((taskTimeElapsed - psFadeStartTime) / psFadeTime * 255.0f);
            glEnable(GL_BLEND);
            if (psFadeState == PFS_CrossFade)
            {
                psImageDraw(&psFadeImage, colRGBA(UBYTE_Max, UBYTE_Max, UBYTE_Max,  min(psFadeLevel, UBYTE_Max)));
            }
            else
            {
                primRectSolid2(&screenRect, colRGBA(0, 0, 0, min(psFadeLevel, UBYTE_Max)));
            }
            glDisable(GL_BLEND);
            regRecursiveSetDirty(psBaseRegion);
            if (psLastFadeLevel >= UBYTE_Max)
            {                                               //if we've reached black
                switch (psFadeLink->reg.userID & PLM_Type)
                {                                           //process the link we've faded to
                    case PLT_Screen:
                        strcpy(linkName, psFadeLink->linkName);
                        psCurrentScreenDelete();
                        psScreenStart(linkName);
                        break;
#if PS_ERROR_CHECKING
                    case PLT_URL:
                        dbgFatal(DBG_Loc, "Shouldn't have a URL link here");
                        break;
#endif
                    case PLT_Exit:
                        psFadeState = PFS_None;
                        utyBrowserExec(((pluglink *)region)->linkName);
                        utyCloseOK(NULL, 0, 0, 0);
                        break;
                    case PLT_GameOn:
                        psFadeState = PFS_None;
                        psModeEnd();
                        subRedrawFlags |= 8;
                        feRenderEverything = TRUE;
                        regRecursiveSetDirty(ghMainRegion);
                        glEnable(GL_BLEND);
                        psFadeState = PFS_FromBlack;
                        primRectSolid2(&screenRect, colRGBA(0, 0, 0, min(psFadeLevel, UBYTE_Max)));
                        glDisable(GL_BLEND);
                        keyClearAll();
                        taskExit();
                        break;
                }
                psFadeLink = NULL;
            }
            psLastFadeLevel = psFadeLevel;
        }
        else if (psFadeState == PFS_FromBlack)
        {                                                   //if we're fading back in from black
            psFadeLevel = (sdword)(255.0f - ((taskTimeElapsed - psFadeStartTime) / psFadeTime * 255.0f));
            if (psLastFadeLevel < 0)
            {                                               //if finished fading
                psFadeState = PFS_None;
            }
            else
            {
                glEnable(GL_BLEND);
                primRectSolid2(&screenRect, colRGBA(0, 0, 0, max(0, psFadeLevel)));
                glDisable(GL_BLEND);
            }
            if (psFadeState == PFS_FromBlack)
            {
                regRecursiveSetDirty(psBaseRegion);         //force update if still fading
            }
            psLastFadeLevel = psFadeLevel;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : psLinkProcess
    Description : Region processor callback for plugscreen links.
    Inputs      : region - region to handle processing
                  ID - user-assigned ID set when region created
                  event - enumeration of event to be processed
                  data - additional event data (message specific)
    Outputs     : ...user defined...
    Return      : flags indicating further operation:

----------------------------------------------------------------------------*/
udword psLinkProcess(regionhandle region, sdword ID, udword event, udword data)
{
#if PS_VERBOSE_LEVEL >= 2
    dbgMessagef("\nPlug region 0x%x, ID %d, event 0x%x", region, ID, event);
#endif
    regRecursiveSetDirty(region);
    if (event == RPE_PressLeft && (psFadeState == PFS_None))
    {
        switch(ID & PLM_Type)
        {
            case PLT_Screen:
#if PS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nLink to '%s'", ((pluglink *)region)->linkName);
#endif
                psScreenTimeout = 0.0f;                     //hold on this screen
                break;
            case PLT_URL:
#if PS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nBrowse to '%s'", ((pluglink *)region)->linkName);
#endif
                utyBrowserExec(((pluglink *)region)->linkName);
                psScreenTimeout = 0.0f;                     //hold on this screen
                return(0);
            case PLT_Exit:
#if PS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nExit Homeworld.");
#endif
                break;
            case PLT_GameOn:
#if PS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nStart playing Homeworld.");
#endif
                break;
        }
        psFadeLink = (pluglink *)region;
        psFadeState = PFS_ToBlack;
        psFadeLevel = psLastFadeLevel = 0;
        psFadeStartTime = taskTimeElapsed;
        regRecursiveSetDirty(psBaseRegion);
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : psBaseRegionDraw
    Description : Region-draw function for drawing the base region of a plug screen.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psBaseRegionDraw(regionhandle region)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    psImageDraw(&psScreenImage, colWhite);
}

/*-----------------------------------------------------------------------------
    Name        : psBaseRegionProcess
    Description : Region-process function for the base region.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
udword psBaseRegionProcess(regionhandle region, sdword ID, udword event, udword data)
{
#if PS_VERBOSE_LEVEL >= 1
    dbgMessagef("\nBase region 0x%x, ID %d, event 0x%x", region, ID, event);
#endif
    switch (event)
    {
        case RPE_ReleaseLeft:
            //hold on this screen
            if (bitTest(psGlobalFlags, PMF_CanSkip))
            {
                psScreenTimeout = 0.0f;
            }
            break;
        case RPE_KeyDown:
            //skip this screen
            if (psFadeState == PFS_None)
            {
                psFadeTime = PS_FadeTime;
                psScreenTimeout = taskTimeElapsed - psScreenCreationTime;
                if (!bitTest(psGlobalFlags, PMF_CanSkip))
                {                                           //this will only be if ESCAPE is hit during a non-skippable sequence
                    strcpy(psTimeoutName, "gameon");
                }
            }
            else
            {
                psFadeTime = (taskTimeElapsed - psFadeStartTime) + PS_FadeTime;
            }
            break;
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : psImageLoad
    Description : Load in a .jpg image into a psimage structure;
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psImageLoad(psimage *destImage, char *directory, char *imageName)
{
    JPEGDATA    jp;
    sdword x, y, quiltX, quiltY, endX, endY;
    ubyte *pSource, *imageBuffer;
    color *pDest;
    udword *quiltSquare;
    color quiltBuffer[PS_QuiltPieceHeight * PS_QuiltPieceWidth];
    char fileName[80];
    sdword bFilterSave;

    strcpy(fileName, directory);                            //prepare file name
    if (bitTest(psGlobalFlags, PMF_LanguageSpecific))
    {                                                       //optionally append a language directory
        strcat(fileName, psLanguageString[strCurLanguage]);
    }
    strcat(fileName, imageName);

    memset(&jp, 0, sizeof(jp));
    jp.input_file = fileOpen(fileName, 0);                  //open the file
    JpegInfo(&jp);                                          //get info on the file
    fileSeek(jp.input_file, 0, SEEK_SET);                   //reset file pointer
                                                            //alloc a buffer to load the image to
    imageBuffer = memAlloc(jp.width * jp.height * sizeof(color), "PlugScreenImage", 0);
    jp.ptr = (ubyte *)imageBuffer;                          //load in the image
    destImage->width = jp.width;
    destImage->height = jp.height;
    JpegRead(&jp);
    fileClose(jp.input_file);                               //close the file

    bFilterSave = texLinearFiltering;
    texLinearFiltering = FALSE;
    //create a 'quilt' of textures from the single big buffer
    destImage->imageQuilt = memAlloc(sizeof(udword) * (destImage->height / PS_QuiltPieceHeight + 1) * (destImage->width / PS_QuiltPieceWidth + 1), "QuiltTextures", 0);
    quiltSquare = destImage->imageQuilt;
    for (quiltY = 0; quiltY + PS_QuiltPieceHeight <= destImage->height; quiltY += PS_QuiltPieceHeight)
    {
        for (quiltX = 0; quiltX + PS_QuiltPieceWidth <= destImage->width; quiltX += PS_QuiltPieceWidth, quiltSquare++)
        {                                                   //for each square of the quilt
            y = quiltY;
            endY = y + PS_QuiltPieceHeight;
            pDest = quiltBuffer;
            for (; y < min(destImage->height, endY); y++)
            {                                               //for each scan line of the quilt square
                x = quiltX;
                endX = x + PS_QuiltPieceWidth;
                pSource = imageBuffer + (y * destImage->width + x) * 3;
                for (; x < min(destImage->width, endX); x++, pSource += 3, pDest++)
                {                                           //each pixel
                    *pDest = colRGB(pSource[0], pSource[1], pSource[2]);
                }
                for (; x < endX; x++, pDest++)
                {                                           //pixels off the right side of the source image
                    *pDest = colBlack;
                }
            }
            for (; y < endY; y++, pDest += PS_QuiltPieceWidth)
            {                                               //pixels off bottom of source image
                memClearDword(pDest, colBlack, PS_QuiltPieceWidth);
            }
            *quiltSquare = trRGBTextureCreate(quiltBuffer, PS_QuiltPieceWidth, PS_QuiltPieceHeight, FALSE);
        }
    }
    texLinearFiltering = bFilterSave;
    memFree(imageBuffer);
}

/*-----------------------------------------------------------------------------
    Name        : psImageSet
    Description : Load in a jpeg image for the background.  Script parsing callback.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psImageSet(char *directory,char *field,void *dataToFillIn)
{
    if (psFadeImage.imageQuilt != NULL)
    {                                                       //if we just cross-faded from another image
        psScreenImage = psFadeImage;                        //don't need to re-load image
        psFadeImage.imageQuilt = NULL;
        psFadeImage.width = psFadeImage.height = 0;
        return;
    }
    psImageLoad(&psScreenImage, directory, field);
}

/*-----------------------------------------------------------------------------
    Name        : psCrossFadeLinkSet
    Description : Create a cross-fade screen link with a specified image.
    Inputs      : Script parsing callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psCrossFadeLinkSet(char *directory,char *field,void *dataToFillIn)
{
    sdword nScanned;
    char fadeImageName[80];

    //format is <fadeTime> <image> <linkName>
    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%f %s", &psFadeTime, fadeImageName);
    dbgAssert(nScanned == 2);
    psImageLoad(&psFadeImage, directory, fadeImageName);               //load in an image
}

/*-----------------------------------------------------------------------------
    Name        : RemoveCommasButNotQuestionMarksFromString
    Description : copied from statscript.c but modified to not remove question marks
    Inputs      : field
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void RemoveCommasButNotQuestionMarksFromString(char *field)
{
    char *fieldptr = field, *linkPtr;

    if ((linkPtr = strstr(field, "https:")) != NULL)
    {                                                       //Major hack!  The link for "To Order" has every imaginable format character in the URL including double slash and question mark
        linkPtr[6] = (char)'//';
    }
    while (*fieldptr != 0)
    {
        if (*fieldptr == ',')
        {
            *fieldptr = ' ';
        }
        fieldptr++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : psPlugLinkSet
    Description : Create a screen link.  Script parsing callback.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psPlugLinkSet(char *directory,char *field,void *dataToFillIn)
{
    char fileName[80];
    char onName[80];
    char offName[80];
    char linkName[256];
    sdword x, y, nScanned;
    lifheader *header;
    pluglink *link;

    //format is <x>,<y>,<onTexture>,<offTexture>,[<linkName>]
    RemoveCommasButNotQuestionMarksFromString(field);
    nScanned = sscanf(field, "%d %d %s %s %s", &x, &y, onName, offName, linkName);
    dbgAssert(nScanned >= 4);
    if (nScanned == 4)
    {
        dbgAssert((sdword)dataToFillIn == PLT_GameOn);
        linkName[0] = 0;                                    //empty string for game on links
    }
    //create the "on" texture
    strcpy(fileName, directory);                            //prepare file name for on button
    strcat(fileName, onName);
    header = trLIFFileLoad(fileName, NonVolatile);
    dbgAssert(bitTest(header->flags, TRF_Paletted));

    link = (pluglink *)regChildAlloc(psBaseRegion->child, (sdword)dataToFillIn, //create the region
            plugXMargin + x, plugYMargin + y, header->width, header->height, plugLinkExtra(linkName),
            RPE_Enter | RPE_Exit | RPE_EnterHoldLeft | RPE_ExitHoldLeft | RPE_PressLeft);
    regFunctionSet(&link->reg, psLinkProcess);
    regDrawFunctionSet(&link->reg, psLinkDraw);

    link->onTexture = trPalettedTextureCreate(header->data, header->palette, header->width, header->height);
    link->onImage = header;
    //memFree(header);                                        //don't need to keep this image around
    //create the "off" texture
    strcpy(fileName, directory);                            //prepare file name for on button
    strcat(fileName, offName);
    header = trLIFFileLoad(fileName, NonVolatile);
    dbgAssert(bitTest(header->flags, TRF_Paletted));
    link->offTexture = trPalettedTextureCreate(header->data, header->palette, header->width, header->height);
    //memFree(header);
    link->offImage = header;
    strcpy(link->linkName, linkName);                       //store the link name
}

/*-----------------------------------------------------------------------------
    Name        : psTimeoutSet
    Description : Sets a timeout for the screen.  Script parsing callback.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psTimeoutSet(char *directory,char *field,void *dataToFillIn)
{
    sdword nScanned;

    RemoveCommasFromString(field);
    nScanned = sscanf(field, "%f %s", &psScreenTimeout, psTimeoutName);
    dbgAssert(nScanned == 2);
}

/*=============================================================================
    Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : psStartup
    Description : Plugscreen module startup
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psStartup(void)
{
    psGLCompat = FALSE;
    psScreenImage.imageQuilt = NULL;
    psScreenImage.width = 0;
    psScreenImage.height = 0;
    psFadeTime = PS_FadeTime;
    psFadeImage.imageQuilt = NULL;
    psFadeImage.width = 0;
    psFadeImage.height = 0;
}

/*-----------------------------------------------------------------------------
    Name        : psShutdown
    Description : Plugscreen module shutdown
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void psShutdown(void)
{
    if (psScreenImage.imageQuilt != NULL)
    {
        memFree(psScreenImage.imageQuilt);
    }
    if (psFadeImage.imageQuilt != NULL)
    {
        memFree(psFadeImage.imageQuilt);
    }
}

/*-----------------------------------------------------------------------------
    Name        : psRenderTaskFunction
    Description : This function replaces the render task when we're in the
                    plugscreens.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
extern void *hGLDeviceContext;              //void * is really a HDC
void psRenderTaskFunction(void)
{
    static bool32 shouldSwap;
    static sdword index;
    static regionhandle reg;

    taskYield(0);
    while (1)
    {
        primErrorMessagePrint();

        if (psScreenTimeout && taskTimeElapsed >= psScreenCreationTime + psScreenTimeout)
        {                                                   //if the current screen times out
            if (!_stricmp(psTimeoutName, "exit"))
            {
#if PS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nTimed out: Exit Homeworld");
#endif
                for (reg = psBaseRegion->child->child; reg != NULL; reg = reg->next)
                {                                           //find the link that does the same thing
                    if ((reg->userID & PLM_Type) == PLT_Exit)
                    {
                        psFadeLink = (pluglink *)reg;
                        psFadeState = PFS_ToBlack;
                        psFadeLevel = psLastFadeLevel = 0;
                        psFadeStartTime = taskTimeElapsed;
                        regRecursiveSetDirty(psBaseRegion);
                        soundEventPlayMusic(SOUND_FRONTEND_TRACK);
                    }
                }
            }
            else if (!_stricmp(psTimeoutName, "gameon"))
            {
#if PS_VERBOSE_LEVEL >= 1
                dbgMessagef("\nTimed out: Starting Homeworld");
#endif
                for (reg = psBaseRegion->child->child; reg != NULL; reg = reg->next)
                {                                           //find the link that does the same thing
                    if ((reg->userID & PLM_Type) == PLT_GameOn)
                    {
                        psFadeLink = (pluglink *)reg;
                        psFadeState = PFS_ToBlack;
                        psFadeLevel = psLastFadeLevel = 0;
                        psFadeStartTime = taskTimeElapsed;
                        regRecursiveSetDirty(psBaseRegion);
                        if (bitTest(psGlobalFlags, PMF_MusicTrack))
                        {                                   //if there is a music track to stop
                            soundEventStopMusic(psFadeTime);
                            soundEventPlayMusic(SOUND_FRONTEND_TRACK);
                        }
                    }
                }
            }
            else
            {                                               //if it's a screen
                for (reg = psBaseRegion->child->child; reg != NULL; reg = reg->next)
                {                                           //find the link that does the same thing
                    if (!_stricmp(psTimeoutName, ((pluglink *)reg)->linkName))
                    {
#if PS_VERBOSE_LEVEL >= 1
                        dbgMessagef("\nTimed out: Link to '%s'", ((pluglink *)reg)->linkName);
#endif
                        if (psFadeImage.imageQuilt == NULL)
                        {                                  //if there is to be a cross-fade
                            psFadeState = PFS_ToBlack;
                        }
                        else
                        {
                            psFadeState = PFS_CrossFade;
                        }
                        psFadeLink = (pluglink *)reg;
                        psFadeLevel = psLastFadeLevel = 0;
                        psFadeStartTime = taskTimeElapsed;
                        regRecursiveSetDirty(psBaseRegion);
                    }
                }
            }
            psScreenTimeout = 0.0f;                         //don't try to time out any more
        }
        shouldSwap = feSavingMouseCursor;
        if (shouldSwap)
        {
            if (RGL)
                rglFeature(RGL_SAVEBUFFER_ON);
        }
        else
        {
            if (RGL)
                rglFeature(RGL_SAVEBUFFER_OFF);
        }
        primErrorMessagePrint();
        rndPostObjectCallbackExecute();
        primErrorMessagePrint();
        /* need to update audio event layer */
        soundEventUpdate();

        if (shouldSwap)
        {
            mouseStoreCursorUnder();
        }
        mouseDraw();                                        //draw mouse atop everything

        if (!feDontFlush)
        {
            rndFlush();
        }
        feDontFlush = FALSE;
        primErrorMessagePrint();
        if (shouldSwap)
        {
            mouseRestoreCursorUnder();
        }
        primErrorMessagePrint();

        taskYield(0);
    }
}

/*-----------------------------------------------------------------------------
    Name        : psModeBegin
    Description : Starts the plugscreen mode.  A call to psScreenStart should
                    soon follow.
    Inputs      : directory - directory for all the screens and images.
                  modeFlags - global mode flags
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psModeBegin(char *directory, udword modeFlags)
{
    psGlobalFlags = modeFlags;
    if (glcActive())
    {
        glcActivate(FALSE);
        psGLCompat = TRUE;
    }
    else
    {
        psGLCompat = FALSE;
    }
    psStartup();
    strcpy(psDirectory, directory);
    psFadeState = PFS_ToBlack;
    taskPause(utyRenderTask);                               //pause the regular render task
    psRenderTask = taskStart(psRenderTaskFunction, 1.0f, TF_OncePerFrame);//replace it with our own render task
}

/*-----------------------------------------------------------------------------
    Name        : psModeEnd
    Description : Ends the plugscreen mode.  Any remaining plugscreen will be
                    deleted.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void psModeEnd(void)
{
    psDirectory[0] = 0;
    //taskStop(psRenderTask);
    taskResume(utyRenderTask);
    psCurrentScreenDelete();
    mouseEnable();
    if (psGLCompat)
    {
        glcActivate(TRUE);
        psGLCompat = FALSE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : psScreenStart
    Description : Start a plug screen.
    Inputs      : name - name of screen file.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
ubyte psScreenSkipKey[] = {ESCKEY, SPACEKEY, ENTERKEY, 0};
void psScreenStart(char *name)
{
    sdword index;
    regionhandle reg;

    psBaseRegion = regChildAlloc(NULL, 0, plugXMargin, plugYMargin, 640, 480, 0, RPE_ModalBreak);
    regSiblingMoveToFront(psBaseRegion);
    reg = regChildAlloc(psBaseRegion, 0, plugXMargin, plugYMargin, 640, 480, 0, RPE_Enter | RPM_PressRelease);
    regFunctionSet(reg, psBaseRegionProcess);
    regDrawFunctionSet(reg, psBaseRegionDraw);

    psScreenTimeout = 0.0f;
    psScreenCreationTime = taskTimeElapsed;
    psMouseFlag = TRUE;                                     //mouse enabled by default
    psFadeTime = PS_FadeTime;                               //default fade speed
    if (psFadeState == PFS_ToBlack)
    {
        psFadeState = PFS_FromBlack;
    }
    else
    {
        psFadeState = PFS_None;
    }

    scriptSet(psDirectory, name, psScreenTweaks);

    dbgAssert(psScreenImage.imageQuilt != NULL);
    dbgAssert(reg->child != NULL);
    bitSet(reg->child->userID, PLF_FadeRegion);
    for (index = 0; psScreenSkipKey[index] != 0; index++)
    {
        regKeyChildAlloc(reg, 0xffffffff, RPE_KeyDown, psBaseRegionProcess, 1, psScreenSkipKey[index]);
        if (!bitTest(psGlobalFlags, PMF_CanSkip))
        {                                                   //only escape (first key) can be used in a no-skip screen sequence
            break;
        }
    }
    psFadeLevel = psLastFadeLevel = min(UBYTE_Max, psFadeLevel);
    psFadeStartTime = taskTimeElapsed;
    if (psMouseFlag)
    {
        mouseEnable();
    }
    else
    {
        mouseDisable();
    }

    utyForceTopmost(fullScreen);
}

/*-----------------------------------------------------------------------------
    Name        : psCurrentScreenDelete
    Description : Deletes the current screen.
    Inputs      : void
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void psCurrentScreenDelete(void)
{
    regionhandle reg;
    sdword x, y;
    udword *quiltTexture = psScreenImage.imageQuilt;

    for (reg = psBaseRegion->child->child; reg != NULL; reg = reg->next)
    {                                                       //delete the textures from each of the links
        if (reg->drawFunction == psLinkDraw)
        {
            memFree(((pluglink *)reg)->onImage);
            memFree(((pluglink *)reg)->offImage);
            trRGBTextureDelete(((pluglink *)reg)->onTexture);
            trRGBTextureDelete(((pluglink *)reg)->offTexture);
        }
    }
    regRegionDelete(psBaseRegion);
    psBaseRegion = NULL;
    for (y = 0; y < psScreenImage.height / PS_QuiltPieceHeight; y++)
    {
        for (x = 0; x < psScreenImage.width / PS_QuiltPieceWidth; x++, quiltTexture++)
        {
            trRGBTextureDelete(*quiltTexture);
        }
    }
    memFree(psScreenImage.imageQuilt);
    psScreenImage.imageQuilt = NULL;
    psScreenImage.width = 0;
    psScreenImage.height = 0;
}


int v3; // ebx
int v4; // ecx
int v5; // edx
int v6; // ecx
int v7; // eax
int v8; // eax
int v9; // ecx
int *v10; // ebx
int v11; // esi
int v12; // edi
int v13; // ebp
bool v14; // cc
int v15; // eax
int v16; // eax
int v17; // eax
int v19; // [esp+10h] [ebp-80h]
int v20; // [esp+14h] [ebp-7Ch]
int v21; // [esp+18h] [ebp-78h]
int v22; // [esp+1Ch] [ebp-74h]
rectangle rect; // [esp+20h] [ebp-70h] BYREF
signed int v24[4]; // [esp+30h] [ebp-60h] BYREF
char v25[80]; // [esp+40h] [ebp-50h] BYREF
v1 = a1;
if ( (*(_WORD *)(a1 + 44) & 0x800) != 0 )
v2 = *(_DWORD *)(a1 + 96);
else
v2 = *(_DWORD *)(a1 + 100);
if ( (*(_WORD *)(a1 + 44) & 0x800) != 0 )
v3 = *(_DWORD *)(a1 + 88);
else
v3 = *(_DWORD *)(a1 + 92);
v4 = (MAIN_WindowWidth - 640) / 2;
rect.x0 = v4 - 1;
v5 = *(_DWORD *)(a1 + 8);
rect.y0 = (MAIN_WindowHeight - 480) / 2;
if ( dword_C30090 > v5 )
v5 = dword_C30090;
rect.x1 = v5 + v4;
v6 = *(_DWORD *)(a1 + 12);
if ( dword_C30094 > v6 )
v6 = dword_C30094;
rect.y1 = v6 + (MAIN_WindowHeight - 480) / 2;
trPalettedTextureMakeCurrent(v2, *(_DWORD *)(v3 + 36));
primRectSolidTextured2((int *)a1);
LOBYTE(v7) = *(_BYTE *)(a1 + 52);
if ( (v7 & 0x80u) != 0 )
{
LOBYTE(v7) = dword_C30014;
if ( dword_C30014 == 2 || dword_C30014 == 3 )
{
dword_88DCD4 = (__int64)((*(float *)&dword_A6B204 - *(float *)&dword_C30018) / fadetime * FLT_255);
glEnable(3042);
v8 = dword_88DCD4;
if ( dword_C30014 == 3 )
{
v19 = dword_88DCD4;
if ( dword_88DCD4 >= 255 )
v19 = 255;
v9 = dword_C30080;
v10 = (int *)dword_C30088;
v11 = (MAIN_WindowWidth - dword_C30080) / 2;
v12 = (MAIN_WindowHeight - dword_C30084) / 2;
v22 = v11;
v20 = 0;
if ( dword_C30084 / 64 > 0 )
{
v21 = (MAIN_WindowHeight - dword_C30084) / 2;
do
{
v24[3] = v12 + 64;
v13 = 0;
v24[1] = v12;
if ( v9 / 64 > 0 )
{
do
{
v24[0] = v11;
v24[2] = v11 + 64;
trRGBTextureMakeCurrent(*v10);
sub_570190(v24, (v19 << 24) | 0xFFFFFF);
v9 = dword_C30080;
++v13;
v11 += 64;
++v10;
}
while ( v13 < dword_C30080 / 64 );
v11 = v22;
v12 = v21;
}
v12 += 64;
v14 = ++v20 < dword_C30084 / 64;
v21 = v12;
}
while ( v14 );
}
v1 = a1;
}
else
{
if ( dword_88DCD4 >= 255 )
v8 = 255;
primRectSolid2(&rect, v8 << 24);
}
dword_B3A4B0(3042);
regRecursiveSetDirty(dword_A50D18);
LOBYTE(v7) = dword_C2FFA0;
if ( dword_C2FFA0 >= 255 )
{
v15 = *(_DWORD *)(dword_A50D20 + 52) & 7;
if ( v15 )
{
v16 = v15 - 2;
if ( v16 )
{
v7 = v16 - 1;
if ( !v7 )
{
dword_C30014 = 0;
byte_C30020 = 0;
taskResume(dword_AB76C8);
psCurrentScreenDelete();
sub_56F8E0();
if ( dword_A50D1C )
{
glcActivate(1);
dword_A50D1C = 0;
}
byte_88F454 |= 8u;
feRenderEverything = 1;
regRecursiveSetDirty((int)ghMainRegion);
glEnable(3042);
v17 = dword_88DCD4;
dword_C30014 = 1;
if ( dword_88DCD4 >= 255 )
v17 = 255;
primRectSolid2(&rect, v17 << 24);
dword_B3A4B0(3042);
keyClearAll_0();
LOBYTE(v7) = taskFunctionExit();
}
}
else
{
dword_C30014 = 0;
sub_5A8060((LPCSTR)(v1 + 104));
LOBYTE(v7) = utyCloseOK();
}
}
else
{
strcpy(v25, (const char *)(dword_A50D20 + 104));
psCurrentScreenDelete();
LOBYTE(v7) = (unsigned __int8)psScreenStart(v25);
}
dword_A50D20 = 0;
}
goto LABEL_44;
}
if ( dword_C30014 == 1 )
{
dword_88DCD4 = (__int64)(FLT_255 - (*(float *)&dword_A6B204 - *(float *)&dword_C30018) / fadetime * FLT_255);
LOBYTE(v7) = dword_C2FFA0;
if ( dword_C2FFA0 >= 0 )
{
glEnable(3042);
primRectSolid2(&rect, (dword_88DCD4 < 0 ? 0 : dword_88DCD4) << 24);
LOBYTE(v7) = dword_B3A4B0(3042);
if ( dword_C30014 == 1 )
LOBYTE(v7) = regRecursiveSetDirty(dword_A50D18);
}
else
{
dword_C30014 = 0;
}
LABEL_44:
dword_C2FFA0 = dword_88DCD4;
}
}
return v7;
}
#endif

if ( *(float *)&dword_C2FFA4 != ZERO && *(float *)&dword_C2FF9C + *(float *)&dword_C2FFA4 <= *(float *)&dword_A6B204 )
{
if ( _strcmpi(dword_C2FFC0, aExit_0) )
{
v2 = _strcmpi(dword_C2FFC0, dword_88DDA4) == 0;
v3 = *(_DWORD *)(*(_DWORD *)(dword_A50D18 + 28) + 28);
dword_A50D0C = v3;
if ( v2 )
{
for ( ; v3; dword_A50D0C = v3 )
{
if ( (*(_BYTE *)(v3 + 52) & 7) == 3 )
{
dword_A50D20 = v3;
dword_C30014 = 2;
dword_C2FFA0 = 0;
dword_88DCD4 = 0;
dword_C30018 = dword_A6B204;
regRecursiveSetDirty(dword_A50D18);
if ( (dword_C30074 & 4) != 0 )
{
soundEventStopMusic(fadetime);
soundEventPlayMusic(17);
}
v3 = dword_A50D0C;
}
v3 = *(_DWORD *)(v3 + 36);
}
}
else if ( v3 )
{
do
{
if ( !_strcmpi(dword_C2FFC0, (const char *)(v3 + 104)) )
{
dword_A50D20 = dword_A50D0C;
dword_C2FFA0 = 0;
dword_88DCD4 = 0;
dword_C30018 = dword_A6B204;
dword_C30014 = (dword_C30088 != 0) + 2;
regRecursiveSetDirty(dword_A50D18);
}
dword_A50D0C = *(_DWORD *)(dword_A50D0C + 36);
v3 = dword_A50D0C;
}
while ( dword_A50D0C );
}
}
else
{
dword_A50D0C = *(_DWORD *)(*(_DWORD *)(dword_A50D18 + 28) + 28);
for ( i = dword_A50D0C; i; dword_A50D0C = i )
{
if ( (*(_BYTE *)(i + 52) & 7) == 2 )
{
dword_A50D20 = i;
dword_C30014 = 2;
dword_C2FFA0 = 0;
dword_88DCD4 = 0;
dword_C30018 = dword_A6B204;
regRecursiveSetDirty(dword_A50D18);
soundEventPlayMusic(17);
i = dword_A50D0C;
}
i = *(_DWORD *)(i + 36);
}
}
dword_C2FFA4 = 0;
}
dword_A50D10 = dword_C40490;
if ( dword_C40490 )
{
if ( RGL )
rglFeature(17920);
}
else if ( RGL )
{
rglFeature(17921);
}
rndPostObjectCallbackExecute();
soundEventUpdate();
if ( dword_A50D10 )
mouseStoreCursorUnder();
mouseDraw(a1);
if ( !dword_A3E144 )
rndFlush();
dword_A3E144 = 0;
if ( dword_A50D10 )
mouseRestoreCursorUnder();
}
}
#endif
