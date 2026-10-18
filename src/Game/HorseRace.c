/*=============================================================================
    Name    : HorseRace.c
    Purpose : Logic for the horse race and loading chat screens.

    Created 6/16/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <math.h>
#include "types.h"
#include "globals.h"
#include "glinc.h"
#include "types.h"
#include "Debug.h"
#include "prim2d.h"
#include "universe.h"
#include "render.h"
#include "CommandNetwork.h"
#include "horserace.h"
#include "region.h"
#include "uicontrols.h"
#include "feflow.h"
#include "font.h"
#include "fontreg.h"
#include "prim2d.h"
#include "utility.h"
#include "task.h"
#include "demo.h"
#include "mouse.h"
#include "linkedlist.h"
#include "chatting.h"
#include "glcaps.h"
#include "nis.h"
#include "multiplayergame.h"
#include "etg.h"
#include "file.h"
#include "jpginterfce.h"
#include "feReg.h"
#include "interfce.h"
#include "teams.h"
#include "TimeoutTimer.h"
#include "strings.h"
#include "singleplayer.h"
#include "autodownloadmap.h"
#include "glcompat.h"
#include "titan.h"
#include "devstats.h"

#define ShouldHaveMousePtr (FALSE)

extern LONGLONG utyTimerLast;
extern udword gDevcaps2;
extern sdword g_GameDifficulty;

/*=============================================================================
    defines:
=============================================================================*/

#define HR_PlayerNameFont   "Arial_12.hff"
#define MAX_CHAT_TEXT       64
#define NUM_CHAT_LINES      10

real32 HorseRacePlayerDropoutTime = 10.0f;     // tweakable
color HorseRaceDropoutColor = colRGB(75,75,75);

/*=============================================================================
    data:
=============================================================================*/

extern HDC hGLDeviceContext;

static rectangle hrSinglePlayerPos;
static color hrSinglePlayerColor;

static sdword JustInit;
static sdword localbar;

// Pixels and info about the background image chosen
static bool32 hrBackgroundInitFrame = 0;
static DWORD *hrBackgroundImage = NULL;
static long hrBackgroundDirty = 0;
bool32 hrBackgroundReinit = FALSE;
static long hrBackXSize, hrBackYSize;

static regionhandle hrDecRegion;

HorseStatus horseracestatus;

TTimer hrPlayerDropoutTimers[MAX_MULTIPLAYER_PLAYERS];

fibfileheader *hrScreensHandle = NULL;

fonthandle      playernamefont=0;
color           hrBackBarColor=HR_BackBarColor;
color           hrChatTextColor=HR_ChatTextColor;

bool32            PlayersAlreadyDrawnDropped[MAX_MULTIPLAYER_PLAYERS];

textentryhandle ChatTextEntryBox = NULL;

regionhandle   hrBaseRegion = NULL;
regionhandle   hrProgressRegion = NULL;
regionhandle   hrChatBoxRegion = NULL;

regionhandle   hrAbortLoadConfirm = NULL;

region horseCrapRegion =
{
    {-1,-1,-1,-1},          //rectangle
    NULL,                   //draw function
    NULL,                   //process function
    NULL, NULL,             //parent, child
    NULL, NULL,             //previous, next
    0, 0,                   //flags, status,
    0,                      //nKeys
    {0},                    //keys
    0,
#if REG_ERROR_CHECKING      //userID
    REG_ValidationKey,       //validation key
#endif

};

bool32 hrRunning=FALSE;

ChatPacket chathistory[NUM_CHAT_LINES];

udword      chatline=0;

void horseRaceRender(void);
void hrDrawPlayersProgress(featom *atom, regionhandle region);
void hrDrawChatBox(featom *atom, regionhandle region);
void hrChatTextEntry(char *name, featom *atom);
//void hrAbortNetworkGame(char *name, featom *atom);
//void hrAbortNonNetworkGame(char *name, featom *atom);

void hrAbortLoadingYes(char *name, featom *atom);
void hrAbortLoadingNo(char *name, featom *atom);

real32 horseRaceGetPacketPercent(real32 barPercent);

//HR_ChatTextEntry
//extern nisheader *utyTeaserHeader;

/*=============================================================================
    Functions
=============================================================================*/
fedrawcallback hrDrawCallback[] =
{
    {hrDrawPlayersProgress,     "HR_DrawPlayersProgress"    },
    {hrDrawChatBox,             "HR_DrawChatBox"            },
    {NULL,          NULL}
};

fecallback hrCallBack[] =
{
    {hrChatTextEntry,           "HR_ChatTextEntry"          },
    //{hrAbortNetworkGame,        "HR_AbortNetworkGame"       },
    //{hrAbortNonNetworkGame,     "HR_AbortNonNetworkGame"    },
    {hrAbortLoadingYes,         "HR_AbortLoadingYes"        },
    {hrAbortLoadingNo,          "HR_AbortLoadingNo"         },
    {NULL,          NULL}
};

void hrDirtyProgressBar()
{
    if (hrRunning)
    {
        if (hrProgressRegion != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(hrProgressRegion);
#endif
            hrProgressRegion->status |= RSF_DrawThisFrame;
        }
    }
}

void hrDirtyChatBox()
{
    if (hrRunning)
    {
        if (hrChatBoxRegion != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(hrChatBoxRegion);
#endif
            hrChatBoxRegion->status |= RSF_DrawThisFrame;
        }
    }
}

void hrBarDraw(rectangle *rect, color back, color fore, real32 percent)
{
    rectangle temp;
//  primRectSolid2(rect, back);

    if (percent > 1.0)
    {
        percent = 1.0;
    }

    temp.x0 = rect->x0;
    temp.y0 = rect->y0;
    temp.x1 = rect->x0 + (sdword)((rect->x1-rect->x0)*percent);
    temp.y1 = rect->y1;

    primRectSolid2(&temp, fore);
}

void hrDrawPlayersProgress(featom *atom, regionhandle region)
{
    sdword     index;
    rectangle pos;
    rectangle outline;
    real32 percent;
    fonthandle currentfont;
    bool32 droppedOut;
    char *difficultyLabel;

    hrProgressRegion = region;

    pos = region->rect;

//  primRectSolid2(&pos, colRGBA(0, 0, 0, 64));

    pos.y0+=fontHeight(" ");
    pos.y1=pos.y0+8;

    currentfont = fontMakeCurrent(playernamefont);

    if (multiPlayerGame)
    {
        dbgAssert(sigsNumPlayers == tpGameCreated.numPlayers);
        for (index=0;index<sigsNumPlayers;index++)
        {
            droppedOut = playerHasDroppedOutOrQuit(index);

            outline = pos;
            outline.x0 -= 5;
            outline.x1 += 10;
            outline.y0 -= 3;
            outline.y1 = outline.y0 + fontHeight(" ")*2 - 2;

            if ((hrBackgroundDirty) || (!PlayersAlreadyDrawnDropped[index]))
            {
                PlayersAlreadyDrawnDropped[index] = droppedOut;

#if 0
                if (hrBackgroundDirty)
                {
                    primRectTranslucent2(&outline, colRGBA(0,0,0,64));
                }
#else
                primRectSolid2(&outline, colBlack);
#endif

                if (droppedOut)
                {
                    fontPrintf(pos.x0,pos.y0,colBlack,"%s",tpGameCreated.playerInfo[index].PersonalName);
                    fontPrintf(pos.x0,pos.y0,HorseRaceDropoutColor,"%s",
                               (playersReadyToGo[index] == PLAYER_QUIT) ? strGetString(strQuit) : strGetString(strDroppedOut));
                }
                else
                {
                    fontPrintf(pos.x0,pos.y0,tpGameCreated.playerInfo[index].baseColor,"%s",tpGameCreated.playerInfo[index].PersonalName);

                    if (horseracestatus.hrstatusstr[index][0])
                    {
                        fontPrintf(pos.x0+150,pos.y0,tpGameCreated.playerInfo[index].baseColor,"%s",horseracestatus.hrstatusstr[index]);
                    }
                }
            }

            primGLRectOutline2(&outline, 1, (droppedOut) ? HorseRaceDropoutColor : tpGameCreated.playerInfo[index].stripeColor);

            pos.y0+=fontHeight(" ");
            pos.y1=pos.y0+4;

            percent = horseracestatus.percent[index];

            hrBarDraw(&pos,hrBackBarColor,(droppedOut) ? HorseRaceDropoutColor : tpGameCreated.playerInfo[index].baseColor,percent);

            pos.y0+=fontHeight(" ");
        }
    }
    else if (singlePlayerGame)
    {
        pos = hrSinglePlayerPos;

        if (pos.x0 != 0)
        {
            percent = horseracestatus.percent[0];

            //dbgMessagef("\npercent %f",percent);

            hrBarDraw(&pos, colBlack, hrSinglePlayerColor/*teColorSchemes[0].textureColor.base*/, percent);
        }
    }
    else
    {
        if (hrBackgroundDirty)
        {
            outline = pos;
            outline.x0 -= 5;
            outline.x1 += 10;
            outline.y0 -= 3;
            outline.y1 = outline.y0 + fontHeight(" ")*2 - 2;

            primRectTranslucent2(&outline, colRGBA(0,0,0,64));
            primGLRectOutline2(&outline, 1, teColorSchemes[0].textureColor.detail);

            fontPrintf(pos.x0,pos.y0,teColorSchemes[0].textureColor.base,"%s",playerNames[0]);
        }

        pos.y0+=fontHeight(" ");
        pos.y1=pos.y0+4;

        percent = horseracestatus.percent[0];

        hrBarDraw(&pos,hrBackBarColor,teColorSchemes[0].textureColor.base,percent);
    }
    if (RGL && !RGLtype)
    {
        fontPrint(10, MAIN_WindowHeight - 20, colWhite, strGetString(strRenderingInSoftware));
    }
    if (singlePlayerGame && tutorial != 1)
    {
        difficultyLabel = NULL;
        switch (g_GameDifficulty)
        {
            case 0:
                difficultyLabel = strGetString(strSkillVeryEasy);
                break;
            case 1:
                difficultyLabel = strGetString(strSkillEasy);
                break;
            case 2:
                difficultyLabel = strGetString(strSkillNormal);
                break;
            case 3:
                difficultyLabel = strGetString(strSkillHard);
                break;
            case 4:
                difficultyLabel = strGetString(strSkillVeryHard);
                break;
            default:
                break;
        }
        if (difficultyLabel != NULL)
        {
            fontPrint(10, MAIN_WindowHeight - 40, colWhite, difficultyLabel);
        }
    }
    fontMakeCurrent(currentfont);
}

void hrDrawChatBox(featom *atom, regionhandle region)
{
    fonthandle currentfont;
    sdword     x,y,i;
    char       name[128];

    hrChatBoxRegion = region;

    currentfont = fontMakeCurrent(playernamefont);

    primRectSolid2(&region->rect,colRGB(0,0,0));
    primGLRectOutline2(&region->rect, 1, hrBackBarColor);

    x = region->rect.x0+10;
    y = region->rect.y0;

    for (i=0;i<NUM_CHAT_LINES;i++)
    {
        if (chathistory[i].message[0]!=0)
        {
            x = region->rect.x0;
            sprintf(name,"%s >",tpGameCreated.playerInfo[chathistory[i].packetheader.frame].PersonalName);
            fontPrintf(x,y,tpGameCreated.playerInfo[chathistory[i].packetheader.frame].baseColor,"%s",name);
            x+=fontWidth(name)+12;
            //fontShadowSet(FS_E | FS_SE | FS_S);
            fontPrintf(x,y,hrChatTextColor,"%s",chathistory[i].message);
            //fontShadowSet(FS_NONE);
            y+= fontHeight(" ");
        }
    }

    fontMakeCurrent(currentfont);
}

void hrChooseSinglePlayerBitmap(char* pFilenameBuffer)
{
    static sdword missionToNis[] = { 0, 1, 0, 0, 2, 3, 4, 5, 0, 0, 6, 0, 7, 0, 8, 9, 10, 11 };
    char fname[128], line[64];
    filehandle handle;
    sdword red, green, blue;

    memset(&hrSinglePlayerPos, 0, sizeof(hrSinglePlayerPos));

    if (tutorial == 1)
    {
        strcpy(fname, "tutorials\\tutorial1\\loading.jpg");
    }
    else if (singlePlayerGameInfo.onLoadPlayNIS > 0)
    {
        sprintf(fname,
                "SinglePlayer\\mission%02d_Nis%02d\\loading.jpg",
                singlePlayerGameInfo.currentMission,
                missionToNis[singlePlayerGameInfo.currentMission]);
    }
    else
    {
        sprintf(fname, "SinglePlayer\\mission%02d\\loading.jpg", singlePlayerGameInfo.currentMission);
    }

    if (!fileExists(fname, 0))
    {
        pFilenameBuffer[0] = '\0';
        return;
    }

    strcpy(pFilenameBuffer, fname);
    hrSinglePlayerPos.x0 = feResRepositionX(243);
    hrSinglePlayerPos.y0 = feResRepositionY(234 + (RGLtype == SWtype));
    hrSinglePlayerPos.x1 = hrSinglePlayerPos.x0 + 152;
    hrSinglePlayerPos.y1 = hrSinglePlayerPos.y0 + 3;

    sprintf(fname, "SinglePlayer\\mission%02d\\loading.script", singlePlayerGameInfo.currentMission);
    if (!fileExists(fname, 0))
    {
        hrSinglePlayerColor = colRGB(255,63,63);
        return;
    }
    handle = fileOpen(fname, FF_TextMode);
    fileLineRead(handle, line, 63);
    sscanf(line, "%d %d %d", &red, &green, &blue);
    fileClose(handle);

    hrSinglePlayerColor = colRGB(red, green, blue);
}

void hrChooseRandomBitmap(char *pFilenameBuffer)
{
filehandle handle;
long    hFind, FileCount, BigFileCount, WhichFile, Result;
struct _finddata_t  FindData;
char BigName[512], CurDir[512], NewDir[512];

    FileCount = BigFileCount = 0;

    GetCurrentDirectory(511, CurDir);

    // First, find screen shots listed in the BigFile
    handle = fileOpen("ScreenShots\\ShotList.script", FF_ReturnNULLOnFail | FF_TextMode);
    if(handle)
    {
        do {
            Result = fileLineRead(handle, BigName, 512);

            if(Result != FR_EndOfFile && Result > 0)    // Found one!
                BigFileCount++;

        } while(Result != FR_EndOfFile);

        fileClose(handle);
    }

    // Tell the file count how many pics were in the BigFile
    FileCount = BigFileCount;

    NewDir[0] = 0;
    strcpy(NewDir, filePathPrepend("ScreenShots", 0));


    // Switch to the screenshots directory and count the ones in there
    SetCurrentDirectory(NewDir);
    hFind = _findfirst("*.jpg", &FindData);
    if(hFind != -1)
    {
        do {
            if( ((FindData.attrib & _A_SUBDIR) == 0) &&
                ((FindData.attrib & _A_HIDDEN) == 0) )
                FileCount++;
        } while (_findnext(hFind, &FindData) == 0);
        _findclose(hFind);
    }

    // Did we find any at all?
    if(FileCount == 0)
    {
        pFilenameBuffer[0] = 0;
        SetCurrentDirectory(CurDir);
        return;
    }

    WhichFile = (utyTimerLast % 32777) % FileCount;

    if(WhichFile < BigFileCount)
    {
        // The file we want is in the bigfile script
        handle = fileOpen("ScreenShots\\ShotList.script", FF_ReturnNULLOnFail | FF_TextMode);
        if(handle)
        {
            do {
                Result = fileLineRead(handle, BigName, 512);

                if(Result != FR_EndOfFile && Result > 0)    // Found one!
                    WhichFile--;

            } while( (WhichFile >= 0) && (Result != FR_EndOfFile));
            strcpy(pFilenameBuffer, "ScreenShots\\");
            strcat(pFilenameBuffer, BigName);
            SetCurrentDirectory(CurDir);
            return;
        }
    }
    else
    {
        // The file we want is in the screenshots directory, so
        // remove BigFileCount from the file index we're looking for
        WhichFile -= BigFileCount;
        FileCount = 0;
        hFind = _findfirst("*.jpg", &FindData);
        if(hFind != -1)
        {
            do {
                if( ((FindData.attrib & _A_SUBDIR) == 0) &&
                    ((FindData.attrib & _A_HIDDEN) == 0) )
                {
                    if(FileCount == WhichFile)
                    {
                        _findclose(hFind);
                        SetCurrentDirectory(CurDir);
                        strcpy(pFilenameBuffer, "ScreenShots\\");
                        strcat(pFilenameBuffer, FindData.name);
                        return;
                    }
                    else
                        FileCount++;
                }
            } while (_findnext(hFind, &FindData) == 0);
        }
    }
    SetCurrentDirectory(CurDir);
}

typedef struct
{
    unsigned char v[2][2];
} ColorQuad;


//
// Takes X & Y values normalized from 0 to 1
// returns a bilinear interpolated pixel value
//
unsigned char hrBilinear(ColorQuad *pQuad, double X, double Y)
{
float Top, Bot, Final;
long FinalLong;

    Top = (float)pQuad->v[0][0] + ((float)pQuad->v[1][0] - (float)pQuad->v[0][0]) * X;
    Bot = (float)pQuad->v[0][1] + ((float)pQuad->v[1][1] - (float)pQuad->v[0][1]) * X;

    Final = (Top + (Bot - Top) * Y) + 0.5f;

    FinalLong = (long)Final;

    // Clamp it to 0 - 255
    if(FinalLong > 255) FinalLong = (FinalLong >> 31) & 0xff;
    return (unsigned char)FinalLong;
}

unsigned long hrGetInterpPixel(unsigned char *pSrcImg, long XSize, float Xf, float Yf)
{
unsigned long Result, x, y, xx, yy;
unsigned char *pCol;
double xfrac, yfrac, xint, yint;
ColorQuad r, g, b;

    x = (unsigned long)Xf;
    y = (unsigned long)Yf;

    xfrac = modf(Xf, &xint);
    yfrac = modf(Yf, &yint);

    for(yy=0; yy<2; yy++)
    {
        for(xx=0; xx<2; xx++)
        {
            pCol = &pSrcImg[ (((y+yy) * XSize) + x+xx) * 3 ];
            r.v[xx][yy] = pCol[0];
            g.v[xx][yy] = pCol[1];
            b.v[xx][yy] = pCol[2];
        }
    }

    Result = (unsigned long)hrBilinear(&r, xfrac, yfrac);
    Result |= (unsigned long)hrBilinear(&g, xfrac, yfrac) << 8;
    Result |= (unsigned long)hrBilinear(&b, xfrac, yfrac) << 16;
    Result |= 0xff000000;

    return Result;
}


long hrShipsToLoadForRace(ShipRace shiprace)
{
long ShipsToLoad = 0;
ShipType shiptype;
ShipType firstshiptype;
ShipType lastshiptype;
ShipStaticInfo *shipstaticinfo;

    firstshiptype = FirstShipTypeOfRace[shiprace];
    lastshiptype = LastShipTypeOfRace[shiprace];
    for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
    {
        shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);

        if( bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded) && !bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded) )
            ShipsToLoad++;
    }

    return ShipsToLoad;
}


long hrIsBackgroundWorthLoading(void)
{
long            ToLoad = 0;
ShipRace        shiprace;
AsteroidType    asteroidtype;
DustCloudType   dustcloudtype;
GasCloudType    gascloudtype;
NebulaType      nebulatype;
DerelictType    derelicttype;

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
        ToLoad += hrShipsToLoadForRace(shiprace);

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        if(bitTest(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        if(bitTest(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        if(bitTest(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        if(bitTest(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        if(bitTest(derelictStaticInfos[derelicttype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(derelictStaticInfos[derelicttype].staticheader.infoFlags, IF_InfoNeeded))
            ToLoad++;
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        if(bitTest(missileStaticInfos[shiprace].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(missileStaticInfos[shiprace].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    return(ToLoad > 5);
}

static bool32 hrDrawPixelsSupported(void)
{
    extern bool32 mainNoDrawPixels;

    if (mainNoDrawPixels)
    {
        //commandline option
        return FALSE;
    }
    if (mainSafeGL && (RGLtype == GLtype))
    {
        //running in "safe mode"
        return FALSE;
    }
    if ((RGLtype == GLtype) &&
        bitTest(gDevcaps2, DEVSTAT2_NO_DRAWPIXELS))
    {
        //bit in devcaps2
        return FALSE;
    }
    else if (_stricmp(GLC_VENDOR, "ati") == 0 &&
             strstr(GLC_RENDERER, "128") != NULL)
    {
        //ATI Rage 128
        return FALSE;
    }
    else if (strstr(GLC_VENDOR, "Matrox") != NULL ||
             strstr(GLC_VENDOR, "atrox ") != NULL)
    {
        //Matrox cards
        return FALSE;
    }
    else
    {
        //assume support
        return TRUE;
    }
}

void hrInitBackground(void)
{
char hrImageName[512];
filehandle handle;
long XSize, YSize, i;
unsigned long *pDest;
unsigned char *pTempImage, *pTempLine, *pRGB;

float SrcX, SrcY, Scale;
float IncX, IncY;
long DestX, DestY;
long DestXSize, DestYSize, Size, Top, Bot;
JPEGDATA    jp;
char CurDir[512], NewDir[512];

    GetCurrentDirectory(511, CurDir);

    hrImageName[0] = 0;
    if (singlePlayerGame)
    {
        hrChooseSinglePlayerBitmap(hrImageName);
    }
    else
    {
//        if(hrIsBackgroundWorthLoading())
            hrChooseRandomBitmap( hrImageName );
    }

    GetCurrentDirectory(511, NewDir);

    dbgAssert(stricmp(CurDir,NewDir) == 0);

    // Load the bitmap image
    handle = fileOpen(hrImageName, FF_ReturnNULLOnFail);
    if(handle)
    {
        memset(&jp, 0, sizeof(jp));
        jp.input_file = handle;
        JpegInfo(&jp);

        fileSeek(handle, 0, SEEK_SET);

        XSize = jp.width;
        YSize = jp.height;

        pTempImage = (unsigned char *)memAllocAttempt((XSize+1) * (YSize+1) * 3, "BackgroundTemp", NonVolatile);
        if(pTempImage == NULL)
            return;

        jp.ptr = pTempImage;
        JpegRead(&jp);

        fileClose(handle);

        Size = XSize*3;
        pTempLine = (unsigned char *)malloc(Size);
        for(i=0; i<(YSize/2); i++)
        {
            Top = i;
            Bot = (YSize-1)-i;

            memcpy(pTempLine, pTempImage + (Size * Top), Size);
            memcpy(pTempImage + (Size * Top), pTempImage + (Size * Bot), Size);
            memcpy(pTempImage + (Size * Bot), pTempLine, Size);
        }
        free(pTempLine);

        // Replicate the last line to appease the filter algorithm
        memcpy(&pTempImage[YSize * XSize * 3], &pTempImage[(YSize-1) * XSize * 3], XSize*3);

        if (singlePlayerGame)
        {
            //fixed width for singleplayer game images
            DestXSize = 640;
            DestYSize = 480;
            hrBackgroundImage = (unsigned long*)malloc(DestXSize * DestYSize * 4);
        }
        else
        {
            if (!hrDrawPixelsSupported())
            {
                //no DrawPixels support, must use glcompat 640x480 quilting
                DestXSize = 640;
                DestYSize = 480;
                hrBackgroundImage = (unsigned long*)malloc(DestXSize * DestYSize * 4);
            }
            else
            {
                Scale = 1.1f;
                do {
                    Scale -= 0.1f;
                    DestXSize = (long)((float)MAIN_WindowWidth * Scale);
                    DestYSize = (long)((float)MAIN_WindowHeight * Scale);

                    hrBackgroundImage = (unsigned long *)malloc(DestXSize * DestYSize * 4);
                } while((hrBackgroundImage == NULL) && (Scale > 0.4f));
            }
        }

        //if the memory was not succesfully allocated
        if (hrBackgroundImage == NULL)
        {
            memFree(pTempImage);
            return;
        }

        // Stretch the image to fit the current display size
        IncX = (float)XSize / (float)DestXSize;
        IncY = (float)YSize / (float)DestYSize;

        pDest = hrBackgroundImage;
        if((IncX < 1.0f) && (IncY < 1.0f) && (pDest != NULL))
        {
            SrcY = 0.0f;
            for(DestY = 0; DestY < DestYSize; DestY++)
            {
                SrcX = 0.0f;
                for(DestX = 0; DestX < DestXSize; DestX++)
                {
                    pDest[DestX] = hrGetInterpPixel(pTempImage, XSize, SrcX, SrcY);
                    SrcX += IncX;
                }
                pDest += DestXSize;
                SrcY += IncY;
            }
        }
        else if (pDest != NULL)
        {
            SrcY = 0.0f;
            for(DestY = 0; DestY < DestYSize; DestY++)
            {
                SrcX = 0.0f;
                for(DestX = 0; DestX < DestXSize; DestX++)
                {
                    pRGB = &pTempImage[ (((unsigned long)SrcY * XSize) + (unsigned long)SrcX) * 3 ];
                    pDest[DestX] = 0xff000000 + ((unsigned long)pRGB[0]) + ((unsigned long)pRGB[1] << 8) + ((unsigned long)pRGB[2] << 16);

                    SrcX += IncX;
                }
                SrcY += IncY;
                pDest += DestXSize;
            }
        }

        hrBackXSize = DestXSize;
        hrBackYSize = DestYSize;
        memFree(pTempImage);
    }
}

void hrDrawBackground(void)
{
    real32 x, y;

    rndClearToBlack();

    // Draw the cached background bitmap using glDrawPixels
    if (hrBackgroundImage)
    {
        x = -((real32)hrBackXSize / (real32)MAIN_WindowWidth);
        y = -((real32)hrBackYSize / (real32)MAIN_WindowHeight);

        rndTextureEnable(FALSE);
        rndLightingEnable(FALSE);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glRasterPos2f(x, y);
        if (hrDrawPixelsSupported())
        {
            glDrawPixels(hrBackXSize, hrBackYSize, GL_RGBA, GL_UNSIGNED_BYTE, hrBackgroundImage);
        }
        else
        {
            glcMatrixSetup(TRUE);
            if (singlePlayerGame)
            {
                glcDisplayRGBABackgroundWithoutScaling((ubyte*)hrBackgroundImage);
            }
            else
            {
                glcDisplayRGBABackgroundScaled((ubyte*)hrBackgroundImage);
            }
            glcMatrixSetup(FALSE);
        }
    }
}

void hrShutdownBackground(void)
{
    // free the memory required by the cached background bitmap

    if (hrBackgroundImage != NULL)
    {
        free(hrBackgroundImage);
        hrBackgroundImage = NULL;
    }
    hrBackgroundInitFrame = 0;
    hrBackgroundReinit = FALSE;
}

void hrAbortLoadingYes(char *name, featom *atom)
{
    hrAbortLoadingGame = TRUE;

    if (multiPlayerGame)
        SendDroppingOutOfLoad(sigsPlayerIndex);

    if (hrAbortLoadConfirm)
    {
        feScreenDelete(hrAbortLoadConfirm);
        hrAbortLoadConfirm = NULL;
    }
}

void hrAbortLoadingNo(char *name, featom *atom)
{
    if (hrAbortLoadConfirm)
    {
        feScreenDelete(hrAbortLoadConfirm);
        hrAbortLoadConfirm = NULL;
    }
}

void hrChatTextEntry(char *name, featom *atom)
{
    char *string;
    ChatPacket temp;
    sdword     width;
    fonthandle fhsave;
    char  testwidth[MAX_CHATSTRING_LENGTH+40];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        ChatTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(ChatTextEntryBox,UICTE_NoLossOfFocus|UICTE_ChatTextEntry);
        uicTextBufferResize(ChatTextEntryBox,MAX_CHATSTRING_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_AcceptText :
            string = ((textentryhandle)atom->pData)->textBuffer;
            sendChatMessage(ALL_PLAYER_MASK^PLAYER_MASK(sigsPlayerIndex),string,(uword)sigsPlayerIndex);
            dbgMessagef("text entry: %s\n",string);
            strcpy(temp.message,string);
            temp.packetheader.frame = (uword)sigsPlayerIndex;
            hrProcessPacket((struct ChatPacket *)&temp);
            uicTextEntrySet(ChatTextEntryBox,"",0);
        break;
        case CM_KeyPressed :
            fhsave = fontMakeCurrent(((textentryhandle)atom->pData)->currentFont); //select the appropriate font
            sprintf(testwidth, "%s >  %s", playerNames[sigsPlayerIndex], ((textentryhandle)atom->pData)->textBuffer);
            width = fontWidth(testwidth);
            fontMakeCurrent(fhsave);
            if (width > (atom->width-30))
            {
                uicBackspaceCharacter((textentryhandle)atom->pData);
            }
        break;
    }
}

#if 0
void hrAbortNetworkGame(char *name, featom *atom)
{
    hrAbortLoadingGame = TRUE;
    dbgAssert(multiPlayerGame);
    SendDroppingOutOfLoad(sigsPlayerIndex);
}

void hrAbortNonNetworkGame(char *name, featom *atom)
{
    hrAbortLoadingGame = TRUE;
}
#endif

HorseRaceBars horseBarInfo;
extern bool8 etgHasBeenStarted;

void horseGetNumBars(HorseRaceBars *horsebars)
{
    sdword i;
    bool32 enablebar[MAX_POSSIBLE_NUM_BARS];
    real32 totalperc;

    dbgAssert(horseTotalNumBars.numBars > 0);
    for (i=0;i<horseTotalNumBars.numBars;i++)
    {
        switch (i)
        {
            case DOWNLOADMAP_BAR:
                enablebar[i] = autodownloadmapRequired();
                break;

            case UNIVERSE_BAR:
                enablebar[i] = TRUE;
                break;

            case ETG_BAR:
                enablebar[i] = etgHasBeenStarted ? FALSE : TRUE;
                break;

            case TEXTURE1_BAR:
            case TEXTURE2_BAR:
                enablebar[i] = GLOBAL_NO_TEXTURES ? FALSE : TRUE;
                break;

            default:
                dbgFatalf(DBG_Loc,"You must specify whether or not to load bar %i here",i);
                break;
        }
    }

    horsebars->numBars = 0;

    // now, based on enablebar[i], selectively add bars that will be loaded:

    for (i=0;i<horseTotalNumBars.numBars;i++)
    {
        if (enablebar[i])
        {
            horsebars->perc[horsebars->numBars++] = horseTotalNumBars.perc[i];
        }
    }

    dbgAssert(horsebars->numBars > 0);

    // now renormalize percentages to 1.0
    for (totalperc = 0.0f,i=0;i<horsebars->numBars;i++)
    {
        totalperc += horsebars->perc[i];
    }

    dbgAssert(totalperc > 0.0f);
    if (totalperc != 1.0f)
    {
        for (i=0;i<horsebars->numBars;i++)
        {
            horsebars->perc[i] /= totalperc;
        }
    }
}

void horseRaceInit()
{
    udword i;
    horseGetNumBars(&horseBarInfo);

    if (glcActive())
    {
        (void)glcActivate(FALSE);
    }

    //initialize current bar to the 0th bar
    localbar=0;

    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        horseracestatus.barnum[i] = 0;
        horseracestatus.percent[i] = 0;
        horseracestatus.hrstatusstr[i][0] = 0;
        if (autodownloadmapRequired())
            TTimerStart(&hrPlayerDropoutTimers[i],HorseRacePlayerDropoutTime*2.0f);     // give double time for autodownloads
        else
            TTimerStart(&hrPlayerDropoutTimers[i],HorseRacePlayerDropoutTime);
        PlayersAlreadyDrawnDropped[i] = FALSE;
    }

    for (i=0;i<NUM_CHAT_LINES;i++)
    {
        chathistory[i].message[0] = 0;
    }

    listInit(&horseCrapRegion.cutouts);

    JustInit = TRUE;

    if (!hrScreensHandle)
    {
        feCallbackAddMultiple(hrCallBack);
        feDrawCallbackAddMultiple(hrDrawCallback);
        hrScreensHandle = feScreensLoad(HR_FIBFile);
    }

    if (!ShouldHaveMousePtr) mouseCursorHide();

    if (singlePlayerGame)
    {
        hrBaseRegion = feScreenStart(&horseCrapRegion, HR_SingleRaceScreen);
    }
    else
    {
        hrBaseRegion = feScreenStart(&horseCrapRegion, (multiPlayerGame) ? HR_RaceScreen : HR_RaceScreenNotNetwork);
    }

    playernamefont = frFontRegister(HR_PlayerNameFont);

    hrRunning=TRUE;
    if (RGLtype == SWtype && feShouldSaveMouseCursor())
    {
        rglFeature(RGL_SAVEBUFFER_ON);
    }
    hrBackgroundDirty = (RGLtype == SWtype) ? 10000 : 3;
    hrBackgroundReinit = FALSE;
}

void horseRaceShutdown()
{
    sdword i;

    if (!ShouldHaveMousePtr) mouseCursorShow();

    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        TTimerInit(&hrPlayerDropoutTimers[i]);
    }

    hrRunning=FALSE;
    if (hrAbortLoadConfirm)
    {
        feScreenDeleteFlags(hrAbortLoadConfirm, 0);
        hrAbortLoadConfirm = NULL;
    }

    feScreenDeleteFlags(hrBaseRegion, 0);

    hrShutdownBackground();

    hrBaseRegion = NULL;
    hrProgressRegion = NULL;
    hrChatBoxRegion = NULL;
    ChatTextEntryBox = NULL;

    if (RGL)
    {
        if (RGLtype == SWtype)
        {
            rglFeature(RGL_SAVEBUFFER_OFF);
        }
        ((void (*)(void))rglEnable)();
    }
}

void horseRaceWaitForNetworkGameStartShutdown()
{
    if (multiPlayerGame)
    {
        while (!multiPlayerGameUnderWay) ;
    }
}

real32 lasttime;

void HorseRaceBeginBar(uword barnum)
{
    //udword i;
    HorsePacket packet;
    lasttime = 0.0f;
//    if(utyTeaserHeader) return;

    if(JustInit)
    {
        JustInit = FALSE;
    }
    else
    {
        localbar++;
    }

    //send packet
    packet.packetheader.type = PACKETTYPE_HORSERACE;
    packet.playerindex = (uword) sigsPlayerIndex;
    packet.packetheader.numberOfCommands = 0;                    //don't need probably
    packet.barnum = (uword) localbar;
    packet.percent = horseRaceGetPacketPercent(0.0f);

    if (multiPlayerGame)
    {
        SendHorseRacePacket((ubyte *)&packet,sizeof(HorsePacket));
    }
    else
    {
        recievedHorsePacketCB((ubyte *)&packet,sizeof(HorsePacket));
    }
}

extern void mousePoll();
extern udword regRegionProcess(regionhandle reg, udword mask);
extern sdword regProcessingRegions;
extern sdword regRenderEventIndex;

void hrUncleanDecorative(void)
{
    if (hrDecRegion != NULL)
    {
        hrDecRegion->drawFunction = ferDrawBitmap;
    }
}

void horseRaceRender()
{
    MSG message;

    _asm xor eax,eax
    _asm mov regRenderEventIndex, eax

    //special-case code for double-clicks
/*    if (keyIsHit(LMOUSE_DOUBLE))
    {
        keyPressUp(LMOUSE_DOUBLE);
        utyDoubleClick();
    }*/
    if (hrBackgroundReinit)
    {
        if (hrBackgroundImage != NULL)
        {
            free(hrBackgroundImage);
            hrBackgroundImage = NULL;
        }
        hrBackgroundReinit = FALSE;
        hrBackgroundDirty = (RGLtype == SWtype) ? 10000 : 3;
        hrBackgroundInitFrame = 0;
    }

    // Make sure the Homeworld text gets drawn on the correct frames
    if (hrBackgroundDirty || RGLtype == SWtype)
    {
        regRecursiveSetDirty(&horseCrapRegion);
        hrDecRegion = NULL;
    }
    else
    {
        regionhandle reg;

        // "Clean" the region with the homeworld logo
        // in it so it doesn't re-draw all the time

        reg = horseCrapRegion.child;
        if (reg)
        {
            reg = reg->child;
        }

        while (reg && reg->drawFunction != &ferDrawBitmap)
        {
            reg = reg->next;
        }

        if (reg)
        {
            void regNULLRenderFunction(regionhandle region);
            hrDecRegion = reg;
            reg->drawFunction = regNULLRenderFunction;
        }
    }

    if (TitanActive)
        titanPumpEngine();

    if (!PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE)!=0)
    {
        regProcessingRegions = TRUE;
        regRegionProcess(horseCrapRegion.child, 0xffffffff);
        regProcessingRegions = FALSE;
        if (ChatTextEntryBox!=NULL)
        {
            bitSet(ChatTextEntryBox->reg.status,RSF_KeyCapture);
            keyBufferClear();
        }
    }
    else
    {
        regRegionProcess(horseCrapRegion.child, 0xffffffff);
        while (PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE)!=0)
        {
            if(GetMessage(&message, NULL, 0, 0))
            {
                TranslateMessage(&message);
                DispatchMessage(&message);

                if (multiPlayerGame)
                {
                    if (keyIsStuck(ESCKEY))
                    {
                        keyClearSticky(ESCKEY);                      //clear the sticky bit
                        if (!hrAbortLoadConfirm)
                        {
                            if (!hrAbortLoadingGame)    // if not already aborting
                            {
                                hrAbortLoadConfirm = feScreenStart(&horseCrapRegion, "AbortLoadConfirm");
                            }
                        }
                        else
                        {
                            feScreenDelete(hrAbortLoadConfirm);
                            hrAbortLoadConfirm = NULL;
                        }
                    }
                }

                regProcessingRegions = TRUE;
                if (hrAbortLoadConfirm!=NULL)
                {
                    ;
                }
                else if (ChatTextEntryBox!=NULL)
                {
                    regRegionProcess(&ChatTextEntryBox->reg, 0xffffffff);
                }
                regProcessingRegions = FALSE;
            }
            if (ChatTextEntryBox!=NULL)
            {
                bitSet(ChatTextEntryBox->reg.status,RSF_KeyCapture);
                keyBufferClear();
            }

            if (TitanActive)
                titanPumpEngine();
        }
    }

    // All of the hacked stuff from the render task

    //glColor3ub(colRed(RND_StarColor), colGreen(RND_StarColor), colBlue(RND_StarColor));

    if (ShouldHaveMousePtr)
    {
        if (feShouldSaveMouseCursor())
        {
            if (RGLtype == SWtype) rglFeature(RGL_SAVEBUFFER_ON);
        }
        else
        {
            if (RGLtype == SWtype) rglFeature(RGL_SAVEBUFFER_OFF);
            rndClearToBlack();
            glClear(GL_DEPTH_BUFFER_BIT);
        }
    }

//    primErrorMessagePrint();
    //default rendering scheme is primitives on any
    //functions which want it off should set it back on when done


    // I know this looks weird, but it's correct
    if(hrBackgroundInitFrame == 1)
        hrInitBackground();


    // When there's no background loaded yet, it fills the screen with black
    if (RGLtype == SWtype)
    {
        hrDrawBackground();
    }
    else
    {
        if (hrBackgroundDirty)
        {
            hrDrawBackground();
        }
        else
        {
            if (hrBackgroundImage != NULL)
            {
                free(hrBackgroundImage);
                hrBackgroundImage = NULL;
            }
        }
    }

    regFunctionsDraw();                                //render all regions
    primErrorMessagePrint();

    hrUncleanDecorative();

    // We want the init code to be called on the 2nd pass.  That way, the screen erases,
    // Then we incur the delay of loading the background.
    // For two frames -after that- we'll draw the background,
    // then just draw the progress bars.
    if(hrBackgroundDirty && hrBackgroundInitFrame)
    {
        if (hrBackgroundDirty > 0) hrBackgroundDirty--;
    }

    hrBackgroundInitFrame++;

    if (ShouldHaveMousePtr)
    {
        mousePoll();
        mouseDraw();                                        //draw mouse atop everything

        if (demDemoPlaying)
        {
            rndShamelessPlug();
        }
    }

    rndFlush();
    if (ShouldHaveMousePtr)
    {
        if (feShouldSaveMouseCursor())
        {
            mouseRestoreCursorUnder();
        }
    }
    primErrorMessagePrint();
}

//function that returns the doctored percentage for conversion of
//multiple bars to a single bar.

real32 horseRaceGetPacketPercent(real32 barPercent)
{
    real32 percent=0.0f;
    sdword i;
    for(i=0;i<localbar;i++)
    {
        //add previous bar percentiles...
        percent += horseBarInfo.perc[i];
    }
    //add on current bars contribution scaled by appropriate ammount
    percent += barPercent*horseBarInfo.perc[localbar];
    return percent;
}

bool32 HorseRaceNext(real32 percent)
{
    HorsePacket packet;
    real32 temptime;
    udword i;
    sdword dontrenderhack = FALSE;
    bool32 sendhrpackethack = FALSE;
//    static real32 lastper = 0.0f;
    SYSTEMTIME lp;
    static sdword modulusCounter = 0;

    if (percent > 1.0f) percent = 1.0f;
//    if(utyTeaserHeader)  return TRUE;

    /*glClear(GL_COLOR_BUFFER_BIT);
    regFunctionsDraw();                                 //render all regions
    glFlush();                                          //ensure all draws complete*/

    if (TitanActive)
        titanPumpEngine();

    packet.packetheader.type = PACKETTYPE_HORSERACE;
    packet.playerindex = (uword) sigsPlayerIndex;
    packet.packetheader.numberOfCommands = 0;                    //don't need probably
    packet.barnum = (uword) localbar;
    //packet.current = (uword) localcurrent;
    //packet.max = (uword) localmax;

    packet.percent = horseRaceGetPacketPercent(percent);

    //render first
    if (lasttime == 0.0f)
    {
        horseRaceRender();
        dontrenderhack = TRUE;
    }

    if (multiPlayerGame && (startingGameState == AUTODOWNLOAD_MAP) && (sigsPlayerIndex == 0))
    {
        sendhrpackethack = TRUE;
        SendHorseRacePacket((ubyte *)&packet,sizeof(HorsePacket));      // always send horse race packet when autouploading map
    }

    GetSystemTime(&lp);
    temptime = (lp.wMilliseconds+lp.wSecond*1000.0f);
    if(temptime - lasttime < horseRaceRenderTime)
    {
        if(lasttime > temptime)
            lasttime = temptime;
        return FALSE;     //don't return
    }
    lasttime = temptime;

//    if(lastper != percent)
//    {

    if (multiPlayerGame)
    {
        if (!sendhrpackethack)
        {
            if (modulusCounter <= 0)
            {
                modulusCounter = 6;
            }
            modulusCounter--;
            if (modulusCounter == 0)
            {
                SendHorseRacePacket((ubyte *)&packet,sizeof(HorsePacket));
            }
        }
    }
    else
    {
        recievedHorsePacketCB((ubyte *)&packet,sizeof(HorsePacket));
    }

//    }
//    lastper = percent;

    if (!dontrenderhack)
    {
        horseRaceRender();
    }

    if(multiPlayerGame)
    {
        for(i=0;i<sigsNumPlayers;i++)
        {
            if (i != sigsPlayerIndex)       // don't check myself
            {
                if (!playerHasDroppedOutOrQuit(i))
                {
                    if (TTimerUpdate(&hrPlayerDropoutTimers[i]))
                    {
                        PlayerDroppedOut(i,TRUE);
                    }
                }
            }
        }

        if(localbar == (horseBarInfo.numBars - 1))
        {
            for(i=0;i<sigsNumPlayers;i++)
            {
                if (!playerHasDroppedOutOrQuit(i))
                {
                    if(horseracestatus.percent[i] < 0.999f)
                    {
                        return FALSE;
                    }
                }
            }
        }
        return TRUE;
    }

    return TRUE;
}

void hrProcessPacket(struct ChatPacket *packet)
{
    sdword  i;
    bool32    done=FALSE;

    for (i=0;i<NUM_CHAT_LINES;i++)
    {
        if (chathistory[i].message[0]==0)
        {
            strcpy(chathistory[i].message,(*((ChatPacket *)packet)).message);
            chathistory[i].packetheader.frame = (*((ChatPacket *)packet)).packetheader.frame;
            done = TRUE;
            break;
        }
    }

    if (!done)
    {
        for (i=0;i<NUM_CHAT_LINES-1;i++)
        {
            chathistory[i] = chathistory[i+1];
            strcpy(chathistory[i].message,chathistory[i+1].message);
        }
        strcpy(chathistory[i].message,(*((ChatPacket *)packet)).message);
        chathistory[i].packetheader.frame = (*((ChatPacket *)packet)).packetheader.frame;
    }

    hrDirtyChatBox();
}

void recievedHorsePacketCB(ubyte *packet,udword sizeofpacket)
{
    //captain should have recieved a packet here...
    HorsePacket *hp = (HorsePacket *)packet;
    udword i;

    i = hp->playerindex;
    dbgAssert(i < MAX_MULTIPLAYER_PLAYERS);

    if (multiPlayerGame)
    {
        titanDebug("hr: recv hrpacket from %d status %d",i,playersReadyToGo[i]);
        if (!playerHasDroppedOutOrQuit(i))
        {
            TTimerReset(&hrPlayerDropoutTimers[i]);
        }
    }

    hrDirtyProgressBar();

    horseracestatus.barnum[i] = hp->barnum;
    horseracestatus.percent[i] = hp->percent;
}



#if 0  // Cat2: Binary hrInitBackground; live implementation at HorseRace.c L697 and L1426 (0x48DC00)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x48DC00
void hrInitBackground(void)
{
  char *result; // eax
  char *v1; // ebx
  int v2; // esi
  char *v3; // edi
  ubyte *v4; // ebp
  unsigned int v5; // ebx
  ubyte *v6; // esi
  ubyte *v7; // edx
  int v8; // eax
  int v9; // ebx
  char *v10; // ecx
  unsigned int *v11; // esi
  int v12; // edi
  bool v13; // zf
  double v14; // st7
  double v15; // st6
  char *v16; // esi
  __int64 v17; // rax
  ubyte *v18; // [esp+10h] [ebp-654h]
  float v19; // [esp+10h] [ebp-654h]
  unsigned int *v20; // [esp+10h] [ebp-654h]
  ubyte *v21; // [esp+14h] [ebp-650h]
  int v22; // [esp+14h] [ebp-650h]
  int v23; // [esp+18h] [ebp-64Ch]
  float v24; // [esp+18h] [ebp-64Ch]
  int v25; // [esp+1Ch] [ebp-648h]
  int v26; // [esp+1Ch] [ebp-648h]
  char *v27; // [esp+20h] [ebp-644h]
  float v28; // [esp+20h] [ebp-644h]
  int v29; // [esp+24h] [ebp-640h]
  float v30; // [esp+24h] [ebp-640h]
  int v31; // [esp+28h] [ebp-63Ch]
  float v32; // [esp+2Ch] [ebp-638h]
  char *v33[13]; // [esp+30h] [ebp-634h] BYREF
  char v34[512]; // [esp+64h] [ebp-600h] BYREF
  CHAR Buffer[512]; // [esp+264h] [ebp-400h] BYREF
  CHAR v36[512]; // [esp+464h] [ebp-200h] BYREF

  GetCurrentDirectoryA(0x1FFu, Buffer);
  v34[0] = 0;
  if ( singlePlayerGame )
    sub_48D500(v34);
  else
    hrChooseRandomBitmap(v34);
  GetCurrentDirectoryA(0x1FFu, v36);
  result = (char *)fileOpen(v34, 64);
  v1 = result;
  if ( !result )
    return result;
  memset(v33, 0, sizeof(v33));
  v33[4] = result;
  sub_5C9CD0(v33);
  fileSeek((int)v1, 0, 0);
  v2 = (int)v33[1];
  v3 = v33[2];
  v31 = (int)v33[1];
  v27 = v33[2];
  result = (char *)memAlloc(3 * (int)(v33[1] + 1) * (int)(v33[2] + 1), 8);
  v4 = (ubyte *)result;
  if ( !result )
    return result;
  v33[0] = result;
  sub_5C9F80(v33);
  fileClose((int)v1);
  v5 = 3 * v2;
  v18 = (ubyte *)malloc_34(3 * v2);
  if ( (int)v3 / 2 > 0 )
  {
    v6 = v4;
    v21 = v4;
    v7 = &v4[v5 * (_DWORD)(v3 - 1)];
    v29 = (int)v3 / 2;
    do
    {
      qmemcpy(v18, v6, v5);
      qmemcpy(v21, v7, v5);
      qmemcpy(v7, v18, v5);
      v8 = v29;
      v7 -= v5;
      v6 = &v21[v5];
      v21 += v5;
      --v29;
    }
    while ( v8 != 1 );
    v3 = v27;
    v2 = v31;
  }
  free(v18);
  qmemcpy(&v4[2 * v31 * (_DWORD)v3 + v31 * (_DWORD)v3], &v4[2 * v2 * (_DWORD)(v3 - 1) + v2 * (_DWORD)(v3 - 1)], v5);
  if ( singlePlayerGame )
  {
    v9 = 640;
    v23 = 640;
    v22 = 480;
    v10 = (char *)malloc_34(0x12C000u);
    dword_A43E4C = v10;
    goto LABEL_12;
  }
  if ( dword_A74D6C )
  {
LABEL_25:
    v23 = 640;
    v22 = 480;
    v9 = 640;
    v10 = (char *)malloc_34(0x12C000u);
    dword_A43E4C = v10;
LABEL_12:
    if ( !v10 )
      return (char *)memFree(v4);
    goto LABEL_31;
  }
  if ( mainSafeGL )
  {
    if ( RGLtype == 3 )
      goto LABEL_25;
  }
  else if ( RGLtype == 3 && (gDevcaps2 & 8) != 0 )
  {
    goto LABEL_25;
  }
  if ( bitTest(gDevcaps2, DEVSTAT2_BLACKLOADSCREENS)
    || !_strcmpi(dword_B3A6BC, off_881818) && strstr(GLC_RENDERER, a128)
    || strstr(dword_B3A6BC, aMatrox)
    || strstr(dword_B3A6BC, aAtrox) )
  {
    goto LABEL_25;
  }
  v19 = 1.1;
  while ( 1 )
  {
    v19 = v19 - flt_72A4D4;
    v23 = (__int64)((double)MAIN_WindowWidth * v19);
    v22 = (__int64)((double)MAIN_WindowHeight * v19);
    v10 = (char *)malloc_34(4 * v23 * v22);
    dword_A43E4C = v10;
    if ( v10 )
      break;
    if ( v19 <= (double)flt_72A674 )
      return (char *)memFree(v4);
  }
  v9 = v23;
LABEL_31:
  v20 = (unsigned int *)v10;
  v32 = (double)v31 / (double)v23;
  v30 = (double)(int)v27 / (double)v22;
  if ( v32 >= (double)ONE || v30 >= (double)ONE )
  {
    v14 = ZERO;
    if ( v22 > 0 )
    {
      v26 = v22;
      do
      {
        v15 = ZERO;
        if ( v9 > 0 )
        {
          v16 = (char *)v20;
          do
          {
            v17 = (__int64)v15;
            v15 = v15 + v32;
            LODWORD(v17) = v31 * (__int64)v14 + v17;
            v16 += 4;
            --v9;
            *((_DWORD *)v16 - 1) = v4[2 * (int)v17 + v17]
                                 + ((v4[2 * (_DWORD)v17 + 1 + (_DWORD)v17]
                                   + ((v4[2 * (_DWORD)v17 + 2 + (_DWORD)v17] + 65280) << 8)) << 8);
          }
          while ( v9 );
          v9 = v23;
        }
        v14 = v14 + v30;
        v20 += v9;
        --v26;
      }
      while ( v26 );
    }
  }
  else
  {
    v28 = 0.0;
    if ( v22 > 0 )
    {
      v25 = v22;
      do
      {
        v24 = 0.0;
        if ( v9 > 0 )
        {
          v11 = v20;
          v12 = v9;
          do
          {
            *v11++ = sub_48D9E0((int)v4, v31, v24, v28);
            --v12;
            v24 = v24 + v32;
          }
          while ( v12 );
        }
        v13 = v25 == 1;
        v28 = v28 + v30;
        v20 += v9;
        --v25;
      }
      while ( !v13 );
    }
  }
  dword_A43E44 = v9;
  dword_A43E30 = v22;
  return (char *)memFree(v4);
}
// ============================================================================
// Decompiled from cataclysm.exe @ 0x48E3C0
sdword horseRaceInit(void)
{
    _DWORD *timerPtr;
    _BYTE *barPtr;
    int i;
    char *v3;
    _DWORD *screenPtr;
    const char *screenName;
    int v6;
    sdword result;
    float timerDuration;

    hrResetProgressBars(&hrProgressState);

    if (glcActive())
        glcActivate(0);

    hrAbortLoadingGame = 0;
    timerPtr = &hrTimers;
    barPtr = &hrBarStates;
    i = 0;

    do
    {
        hrBarProgress[i] = 0;
        hrBarTargets[i] = 0;
        *barPtr = 0;

        if (isNetworkGame())
        {
            timerDuration = hrBarTimerDuration + hrBarTimerDuration;
            TTimerStart(timerPtr, timerDuration);
        }
        else
        {
            TTimerStart(timerPtr, hrBarTimerDuration);
        }

        hrBarComplete[i] = 0;
        barPtr += 100;
        timerPtr += 5;
        ++i;
    }
    while (barPtr < hrBarStatesEnd);

    // Clear player name slots
    v3 = hrPlayerNames;
    do
    {
        *v3 = 0;
        v3 += 552;
    }
    while (v3 < hrPlayerNamesEnd);

    listInit(hrChatList);
    hrInitialized = 1;

    if (!hrScreensHandle)
    {
        feCallbackAddMultiple((int *)&hrCallbacks);
        feDrawCallbackAddMultiple((int *)&hrDrawCallbacks);
        hrScreensHandle = (int)feScreensLoad("feman\horse_race.fib");
    }

    mouseCursorHide();

    if (singlePlayerGame)
    {
        screenPtr = feScreenStart(hrBaseRegion, "HorseRace_SinglePlayer");
    }
    else
    {
        screenName = "HorseRace";
        if (!multiPlayerGame)
            screenName = "HorseRace_NonNetwork";
        screenPtr = feScreenStart(hrBaseRegion, screenName);
    }

    hrCurrentScreen = (int)screenPtr;
    hrPlayerNameFont = frFontRegister("Arial_12.hff");
    v6 = RGLtype;
    hrRunning = 1;

    if (!RGLtype)
    {
        if (feShouldSaveMouseCursor())
            rglFeature(17920);  // RGL_SAVEBUFFER_ON
        v6 = RGLtype;
    }

    result = v6 != 0 ? 3 : 10000;
    hrFrameCount = 0;
    hrTickInterval = result;

    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x48E530
void horseRaceShutdown(void)
{
  _DWORD *v0;

  mouseCursorShow();
  v0 = &unk_C38E00;
  do
  {
    TTimerInit(v0);
    v0 += 5;
  }
  while ( (int)v0 < (int)&dword_C38EA0 );
  hrRunning = 0;
  if ( dword_A43E70 )
  {
    feScreenDeleteFlags(dword_A43E70, 0);
    dword_A43E70 = 0;
  }
  feScreenDeleteFlags(dword_A43E64, 0);
  if ( dword_A43E4C )
  {
    free(dword_A43E4C);
    dword_A43E4C = 0;
  }
  dword_A43E48 = 0;
  dword_A43E54 = 0;
  dword_A43E64 = 0;
  dword_A43E68 = 0;
  dword_A43E6C = 0;
  dword_A43E60 = 0;
  if ( RGL )
  {
    if ( !RGLtype )
      rglFeature(17921);
    rglEnable();
  }
}
#endif

#if 0  // Cat2: hrDrawChatBox live code differs from binary
/*=== hrDrawChatBox (0x48d410) ===*/
udword __cdecl hrDrawChatBox(int a1, rectangle *rect)
{
sdword y0; // ebx
const char *v4; // esi
sdword x0; // edi
sdword v6; // eax
udword font; // [esp+10h] [ebp-84h]
char string[128]; // [esp+14h] [ebp-80h] BYREF
dword_A43E6C = (int)rect;
font = fontMakeCurrent(dword_A43E5C);
primRectSolid2(rect, 0xFF000000);
primGLRectOutline2(rect, 1, dword_881618);
y0 = rect->y0;
v4 = byte_C37888;
do
{
if ( *v4 )
{
x0 = rect->x0;
sprintf(string, "%s >", (const char *)(60 * *((_DWORD *)v4 - 9) + 8937760));
fontPrintf(x0, y0, *(&dword_886108 + 15 * *((_DWORD *)v4 - 9)), "%s", string);
v6 = fontWidth(string);
fontPrintf(x0 + v6 + 12, y0, dword_88161C, "%s", v4);
y0 += fontHeight(location);
}
v4 += 552;
}
while ( (int)v4 < (int)byte_C38E18 );
return fontMakeCurrent(font);
}
#endif

#if 0  // Cat2: hrGetInterpPixel live code differs from binary
/*=== hrGetInterpPixel (0x48d9e0) ===*/
unsigned int __cdecl hrGetInterpPixel(unsigned __int8 *pSrcImg, int XSize, float Xf, float Yf)
{
double v4; // st7
unsigned int v5; // edi
unsigned __int8 *v6; // ebp
unsigned __int8 *v7; // eax
unsigned int v8; // ecx
int v9; // esi
double v10; // st4
__int64 v11; // rax
int v12; // esi
double v13; // st4
__int64 v14; // rax
unsigned __int16 v15; // cx
int v16; // esi
__int64 v17; // rax
char v19[4]; // [esp+1Ch] [ebp-20h]
double v20; // [esp+24h] [ebp-18h] BYREF
double v21; // [esp+2Ch] [ebp-10h] BYREF
double v22; // [esp+34h] [ebp-8h]
float Yfa; // [esp+4Ch] [ebp+10h]
v22 = modf(Xf, &v21);
v4 = modf(Yf, &v20);
v5 = 0;
v6 = &pSrcImg[2 * (__int64)Xf + 2 + 2 * XSize * (__int64)Yf + (__int64)Xf + XSize * (__int64)Yf];
do
{
v7 = v6;
v8 = v5;
v9 = 2;
do
{
v19[v8] = *(v7 - 2);
*((_BYTE *)&v20 + v8) = *(v7 - 1);
*((_BYTE *)&v21 + v8) = *v7;
v7 += 3;
v8 += 2;
--v9;
}
while ( v9 );
++v5;
v6 += 3 * XSize;
}
while ( v5 < 2 );
v10 = ((double)(unsigned __int8)v19[2] - (double)(unsigned __int8)v19[0]) * v22 + (double)(unsigned __int8)v19[0];
v11 = (__int64)((((double)(unsigned __int8)v19[3] - (double)(unsigned __int8)v19[1]) * v22
+ (double)(unsigned __int8)v19[1]
- v10)
* v4
+ v10
+ dbl_72A480);
if ( (int)v11 > 255 )
LOBYTE(v11) = (int)v11 >> 31;
v12 = (unsigned __int8)v11;
v13 = ((double)BYTE2(v20) - (double)LOBYTE(v20)) * v22 + (double)LOBYTE(v20);
v14 = (__int64)((((double)BYTE3(v20) - (double)BYTE1(v20)) * v22 + (double)BYTE1(v20) - v13) * v4 + v13 + dbl_72A480);
if ( (int)v14 > 255 )
LOBYTE(v14) = (int)v14 >> 31;
LOBYTE(v15) = 0;
HIBYTE(v15) = v14;
v16 = v15 | v12;
Yfa = ((double)BYTE2(v21) - (double)LOBYTE(v21)) * v22 + (double)LOBYTE(v21);
v17 = (__int64)(v4 * (v22 * ((double)BYTE3(v21) - (double)BYTE1(v21)) + (double)BYTE1(v21) - Yfa) + Yfa + dbl_72A480);
if ( (int)v17 > 255 )
LOBYTE(v17) = (int)v17 >> 31;
return v16 | (((unsigned int)v17 | 0xFFFFFF00) << 16);
}
#endif
