/*=============================================================================
    Name    : ConsMgr.c
    Purpose : Logic for the Construction Manager

    Created 7/18/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "types.h"
#include "linkedlist.h"
#include "universe.h"
#include "region.h"
#include "uicontrols.h"
#include "feflow.h"
#include "font.h"
#include "fontreg.h"
#include "objtypes.h"
#include "task.h"
#include "mouse.h"
#include "MultiplayerGame.h"
#include "CommandLayer.h"
#include "piePlate.h"
#include "ConsMgr.h"
#include "globals.h"
#include "CommandWrap.h"
#include "scroller.h"
#include "soundevent.h"
#include "Battle.h"
#include "randy.h"
#include "strings.h"
#include "researchapi.h"
#include "BeastMothership.h"
#include "mainrgn.h"
#include "taskbar.h"
#include "shipview.h"
#include "glinc.h"
#include "glcaps.h"
#include "render.h"
#include "mainrgn.h"

#include "sensors.h"
#include "singleplayer.h"
#include "ping.h"
#include "texreg.h"
#include "fereg.h"
#include "shipdefs.h"
#include "Ship.h"
#include "nis.h"
#include "tutor.h"
#include "options.h"
#include "FEColour.h"
#include "glcompat.h"
#include "infooverlay.h"
#include "SaveGame.h"
#include "dock.h"
#include "Subtitle.h"
#include "camera.h"

char *shipNameGet(Ship *ship);

typedef struct cmDetermProgress_t
{
    Node     node;
    sdword   numShips;
    ShipType shipType;
    ShipRace shipRace;
    sdword   costPerShip;
    real32   costPerTick;
    real32   costSoFar;
    sdword   ticksForOne;
    sdword   ticksLeft;
    sdword   playerIndex;
    ShipPtr  creator;
    bool32   paused;
    bool32   extBuildStarted;
    bool32   extBuildCreated;
    Ship    *extBuildShip;
} cmDetermProgress_t;

int cmBuildNarratorUpdate(void)
{
    subMessageEnded = 2;
    if (subRegion[STR_CloseCaptioned].bEnabled && subRegion[STR_CloseCaptioned].cardIndex > 0)
    {
        return subTitleUpdate(&subRegion[STR_CloseCaptioned], smSkipFadeoutTime);
    }

    speechEventActorStop(ACTOR_ALL_ACTORS, smSkipFadeoutTime);
    return subTitleUpdate(&subRegion[STR_LetterboxBar], smSkipFadeoutTime);
}

static bool32 cmBuildStateChanged(Ship *factoryShip, ShipClass shipclass, ShipType shiptype, bool32 isAcolyte, uword shiprace)
{
    DockPointActivity *dockPointActivity;
    DockStaticInfo *dockStaticInfo;
    sdword buildState;

    if (factoryShip == NULL || factoryShip->staticinfo == NULL)
    {
        return FALSE;
    }

    dockPointActivity = factoryShip->dockvars.dockPointActivity;
    dockStaticInfo = factoryShip->staticinfo->dockStaticInfo;
    if (dockPointActivity == NULL || dockStaticInfo == NULL)
    {
        return FALSE;
    }

    buildState = cmGetBuildState(shipclass, shiptype, isAcolyte, shiprace, 2, factoryShip);
    if (buildState < 0)
    {
        return FALSE;
    }

    return dockPointActivity->numDockedOfType[buildState] != dockStaticInfo->numPointsOfType[buildState];
}

extern LinkedList listofShipsInProgress;
extern LinkedList cmDetermProgress;
extern regionhandle cmSUBarRegion;
void mgResetShipListFilter(void);
void cmDirtyShipInfo(void);
void cmUpdateShipAvailable(void);
void univReallyDeleteThisShipRightNow(Ship *ship);

static void cmDirtyCarrierCount(void)
{
    if (cmSUBarRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmSUBarRegion);
#endif
        cmSUBarRegion->status |= RSF_DrawThisFrame;
    }
}

static sdword cmShipBuiltSpeechEvent(sdword shipclass, sdword shiptype)
{
    sdword handle;

    if (shipclass > CLASS_Frigate)
    {
        return shipclass;
    }

    handle = battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, 69, SOUND_EVENT_DEFAULT, NULL);
    if (handle == ERROR)
    {
        return speechEventQueue(NULL, STAT_F_Const_CapShipComplete, shiptype, -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
    }

    return battleChatterFleetAttempt(handle, 70, SOUND_EVENT_DEFAULT, NULL);
}

static void cmShipBuildComplete(shipinprogress *progress, Ship *factoryship)
{
    bool32 externalBuild;

    externalBuild = (LOBYTE(progress->info->externalBuild) == 1);
    if (externalBuild)
    {
        if (progress->cataSaveField1 == 1 &&
            !multiPlayerGame &&
            (!recordingActive || !multiPlayerRecording))
        {
            if (progress->cataSaveField2 != 0)
            {
                clWrapExternalConstruction(&universe.mainCommandLayer,
                                           universe.curPlayerIndex,
                                           factoryship,
                                           (void *)progress->cataSaveField2,
                                           1.0f,
                                           0);
            }
            progress->cataSaveField1 = 0;
            progress->cataSaveField2 = 0;
        }
    }
    else if (!multiPlayerGame && (!recordingActive || !multiPlayerRecording))
    {
        clWrapCreateShip(&universe.mainCommandLayer,
                         progress->info->shiptype,
                         universe.curPlayerPtr->race,
                         universe.curPlayerIndex,
                         factoryship);
    }

    if (multiPlayerGame)
    {
        shiplagtotals[progress->info->shiptype]++;
    }

    cmDirtyCarrierCount();

    if (progress->nJobs > 0)
    {
        progress->nJobs--;
        progress->costSoFar -= singlePlayerGame ?
            (progress->info->buildTime / GameDifficultyProductionModifier) :
            progress->info->buildTime;

        if (progress->nJobs <= 0)
        {
            progress->nJobsTotal = 0;
            progress->timeStart = 0;
            pingNewShipPingCreate(&factoryship->posinfo.position);
            cmShipBuiltSpeechEvent(progress->info->shipclass, progress->info->shiptype);
        }
        else
        {
            cmShipBuiltSpeechEvent(progress->info->shipclass, progress->info->shiptype);
            progress->selected = 0;
            progress->timeFraction = 0;
            progress->timeStart = singlePlayerGame ?
                (progress->info->buildTime / GameDifficultyBuildTimeModifier) :
                progress->info->buildTime;
            progress->timeLeft = progress->timeStart;
        }
    }
}

static real32 cmExternalBuildProgress(sdword timeLeft, sdword timeStart)
{
    real32 progress;

    if (timeStart <= 0)
    {
        return 1.0f;
    }

    progress = 1.0f - ((real32)timeLeft / (real32)timeStart);
    if (progress < 0.0f)
    {
        return 0.0f;
    }
    if (progress > 1.0f)
    {
        return 1.0f;
    }
    return progress;
}

static shipinprogress *cmFindProgressForFactory(Ship *factoryShip, ShipType shipType)
{
    Node *node;

    if (factoryShip == NULL || (udword)shipType >= TOTAL_NUM_SHIPS)
    {
        return NULL;
    }

    for (node = listofShipsInProgress.head; node != NULL; node = node->next)
    {
        shipsinprogress *factory = listGetStructOfNode(node);

        if (factory->ship == factoryShip)
        {
            return &factory->progress[shipType];
        }
    }

    return NULL;
}

void cmUpdateExternalBuildShip(Ship *ship, real32 progress, sdword operation)
{
    real32 delta;
    real32 maxHealth;

    if (ship == NULL || ship->staticinfo == NULL)
    {
        dbgWarningf(DBG_Loc, "cmUpdateExternalBuildShip called with invalid ship");
        return;
    }

    switch (operation)
    {
        case 0:
            if (ship->buildTimerElapsed < 1.0f)
            {
                delta = progress - ship->buildTimerElapsed;
                ship->buildTimerElapsed = progress;
                maxHealth = ship->staticinfo->maxhealth;
                ship->health += delta * maxHealth;
                if (ship->health > maxHealth)
                {
                    ship->health = maxHealth;
                }
            }

            ship->specialFlags2 &= ~SPECIAL_2_DockFlag;
            if (ship->shiptype >= sMothershipSpecial &&
                ship->shiptype <= sMothershipBigGun)
            {
                ModuleCompleted(ship->playerowner, ship->shiptype, TRUE);
            }
            cmDirtyShipInfo();
            break;

        case 1:
            ship->specialFlags2 &= ~SPECIAL_2_DockFlag;
            univReallyDeleteThisShipRightNow(ship);
            cmDirtyShipInfo();
            break;

        case 2:
            delta = progress - ship->buildTimerElapsed;
            ship->buildTimerElapsed = progress;
            maxHealth = ship->staticinfo->maxhealth;
            ship->health += delta * maxHealth;
            if (ship->health > maxHealth)
            {
                ship->health = maxHealth;
            }
            break;
    }
}

void cmUntrackExternalBuildShip(Ship *ship)
{
    Ship *creator;
    shipinprogress *progress;
    Node *node;
    shipsinprogress *factory;

    if (ship->clampInfo == NULL)
    {
        return;
    }

    // Retail reads the creator ship from dockvars.dockstaticpoint (Ship+0x4AC), not
    // customdockinfo (Ship+0x4B0) - confirmed via raw disasm of cmUntrackExternalBuildShip
    // (IDA 0x45599f/0x455a3b: `mov edx, [esi+4ACh]`).
    creator = (Ship *)ship->dockvars.dockstaticpoint;
    if (!playPackets)
    {
        Player *playerOwner = ship->playerowner;

        if (playerOwner == universe.curPlayerPtr && !playerOwner->aiPlayer)
        {
            factory = NULL;
            for (node = listofShipsInProgress.head; node != NULL; node = node->next)
            {
                shipsinprogress *candidate = listGetStructOfNode(node);

                if (candidate->ship == creator)
                {
                    factory = candidate;
                    break;
                }
            }

            if (factory != NULL)
            {
                progress = &factory->progress[ship->shiptype];
                if (progress->nJobs > 0 &&
                    (Ship *)progress->cataSaveField2 == ship)
                {
                    progress->nJobsTotal--;
                    progress->nJobs--;
                    progress->timeFraction = 0;
                    progress->timeLeft = 0;
                    progress->timeStart = 0;
                    if (!multiPlayerGame && (!recordingActive || !multiPlayerRecording))
                    {
                        progress->cataSaveField1 = 0;
                        progress->cataSaveField0 = 0;
                    }
                    progress->cataSaveField2 = 0;
                    cmDirtyShipInfo();
                }
            }
        }
    }

    if (multiPlayerGame || (recordingActive && multiPlayerRecording))
    {
        for (node = cmDetermProgress.head; node != NULL; node = node->next)
        {
            cmDetermProgress_t *progress = listGetStructOfNode(node);

            if (progress->creator == creator &&
                progress->extBuildShip == ship)
            {
                shipinprogress *factoryProgress = NULL;

                progress->numShips--;
                progress->ticksLeft = progress->ticksForOne;
                progress->extBuildCreated = FALSE;
                progress->extBuildStarted = FALSE;

                for (node = listofShipsInProgress.head; node != NULL; node = node->next)
                {
                    shipsinprogress *candidate = listGetStructOfNode(node);

                    if (candidate->ship == progress->creator)
                    {
                        factoryProgress = &candidate->progress[progress->shipType];
                        break;
                    }
                }

                dbgAssert(factoryProgress != NULL);
                factoryProgress->cataSaveField0 = 0;
                factoryProgress->cataSaveField1 = 0;
                progress->extBuildShip = NULL;
                return;
            }
        }
    }
}

void cmTrackExternalBuildShip(Ship *ship)
{
    // Retail reads the creator ship from dockvars.dockstaticpoint (Ship+0x4AC), not
    // customdockinfo (Ship+0x4B0) - confirmed via raw disasm of cmTrackExternalBuildShip
    // (IDA 0x455b23/0x455bad: `mov edx, [edi+4ACh]`).
    Ship *creator = (Ship *)ship->dockvars.dockstaticpoint;
    shipinprogress *factoryProgress;
    Node *node;
    shipsinprogress *factory;
    factoryProgress = NULL;

    if (!playPackets)
    {
        Player *playerOwner = ship->playerowner;

        if (playerOwner == universe.curPlayerPtr && !playerOwner->aiPlayer)
        {
            factory = NULL;
            for (node = listofShipsInProgress.head; node != NULL; node = node->next)
            {
                shipsinprogress *candidate = listGetStructOfNode(node);

                if (candidate->ship == creator)
                {
                    factory = candidate;
                    break;
                }
            }

            dbgAssert(factory != NULL);
            factoryProgress = &factory->progress[ship->shiptype];
            if (!multiPlayerGame && (!recordingActive || !multiPlayerRecording))
            {
                factoryProgress->cataSaveField2 = (sdword)ship;
            }

            if (!multiPlayerGame && (!recordingActive || !multiPlayerRecording))
            {
                factoryProgress->cataSaveField0 = 1;
            }
        }
    }

    if (!multiPlayerGame && (!recordingActive || !multiPlayerRecording))
    {
        return;
    }

    for (node = cmDetermProgress.head; node != NULL; node = node->next)
    {
        cmDetermProgress_t *progress = listGetStructOfNode(node);

        if (progress->creator == creator &&
            progress->shipType == ship->shiptype)
        {
            shipsinprogress *factoryCandidate;

            progress->extBuildStarted = TRUE;
            factoryProgress = NULL;
            for (factoryCandidate = NULL, node = listofShipsInProgress.head; node != NULL; node = node->next)
            {
                shipsinprogress *candidate = listGetStructOfNode(node);

                if (candidate->ship == progress->creator)
                {
                    factoryCandidate = candidate;
                    break;
                }
            }

            dbgAssert(factoryCandidate != NULL);
            factoryProgress = &factoryCandidate->progress[progress->shipType];
            dbgAssert(factoryProgress != NULL);
            factoryProgress->cataSaveField0 = 1;
            return;
        }
    }
}

/*=============================================================================
    Definitions:
=============================================================================*/
#define CM_FIBFile              "FEMan\\Construction_Manager.fib"
#define CM_BeastFIBFile         "FEMan\\Beast_Construction_Manager.fib"
#define CM_ConstructionScreen   "Construction_manager"
#define CM_BeastConstructionScreen "Beast_Construction_manager"
//#define CM_BuildScreen          "Build_screen"
#define CM_DefaultFont          "default.hff"
#define CM_ShipListFont         "HW_EuroseCond_11.hff"
#define CM_TitleFont            "EuroseWideHeavy_i13.hff"
#define CM_FontNameLength       64
#define CM_HeadingColorFactor   1.5f

//margins for printing RU's
#define CM_RUMarginRight        2
#define CM_RUMarginTop          2

//margins for available ships
#define CM_ASHeadingMarginLeft  3
#define CM_ASMarginLeft         9
#define CM_ASMarginTop          2
#define CM_ASMarginRight        2
#define CM_ASInterSpacing       2

#define CM_NumberJobsPerShip    128
#define CM_NumberJobsPerClass   10


#define CM_PausedColor          colRGB(80,120,80)
#define CM_SelectedColorSolid   colRGB(60,030,0)
#define CM_SelectedColorOutline colRGB(120,070,0)
#define CM_BackgroundColor      colRGB(0,35,79)
#define CM_SelectBoxX0          0
#define CM_SelectBoxX1          190
#define CM_SelectBoxY0          0
#define CM_SelectBoxY1          12

sdword cmSelectTopAdjust = 0;
sdword cmSelectBottomAdjust = 0;
sdword cmSelectLeftAdjust = 0;
sdword cmSelectRightAdjust = 0;

#define CM_FIVEKEY              CONTROLKEY      //hold to increase/decrease by 5

//task parameters
#define CM_TaskStackSize        4096
#define CM_TaskServicePeriod      UNIVERSE_UPDATE_PERIOD

//parameter defaults
#define CM_SelectionTextColor     colRGB(255, 255, 105)
#define CM_BuildingTextColor      colRGB(0, 255, 0)
#define CM_SelectionRectColor     colRGB(50, 120, 20)
#define CM_ProgressShipColor      colRGB(100, 100, 55)
#define CM_ProgressTotalColor     colRGB(180, 180, 80)
#define CM_StandardTextColor      colRGB(0,110,172)
#define CM_ClassHeadingTextColor  colRGB(255, 200, 0)
#define CM_CapsExceededTextColor  colRGB(255,25,25)
#define CM_RefundRatio          39321

#define CM_FloatRefundRatio     1.0f

// defines for shipavailable status word

#define ITEM_CLASS                  0
#define ITEM_SHIP                   1
#define ITEM_SHIP_REQUIRES_COUNT    2

#define STAT_CANBUILD       0
#define STAT_CANTBUILD      1
#define STAT_PRINT          2
#define STAT_DONTPRINT      3
#define STAT_INACTIVE       4

// frame count for red flash when ship limits exceeded
#define CAPS_REDFRAMECOUNT 5;

//margins for printing construction progress
#define CM_ProgressMarginLeft   2
#define CM_ProgressMarginRight  90
#define CM_ProgressMarginTop    2
#define CM_ProgressInterSpacing 2
#define CM_SHIP_IMAGE_INSET     2

#define NUM_CMTEXTURES          6
#define NUM_CMSUPPORTTEXTURES   5

#define NUM_CMCARRIERS          2

sdword cmCurrentSupportUnitsUsed(Player *player);

#define UNIVERSE_TURBO_REPEAT       19

#define CM_MaxJobsMothership    2
#define CM_MaxJobsHeavyCruiser  2
#define CM_MaxJobsCarrier       2
#define CM_MaxJobsDestroyer     2
#define CM_MaxJobsFrigate       2
#define CM_MaxJobsCorvette      2
#define CM_MaxJobsFighter       3
#define CM_MaxJobsResource      2
#define CM_MaxJobsNonCombat     2

/*=============================================================================
    Data:
=============================================================================*/
static sdword cmRenderEverythingCounter;

bool32 cmPaletted;

#if CM_CHEAP_SHIPS
bool32 cmCheapShips = FALSE;
#endif

void cmDeterministicStartup(void);
void cmDeterministicShutdown(void);

//callback tables for this screen
void cmClose(char *string, featom *atom);
void cmResearch(char *string, featom *atom);
void cmLaunch(char *string, featom *atom);
void cmCancelJobs(char *string, featom *atom);
void cmPauseJobs(char *string, featom *atom);
void cmBuildShips(char *string, featom *atom);
void cmNumberRUsDraw(featom *atom, regionhandle region);
void cmShipCostsDraw(featom *atom, regionhandle region);
void cmShipInfoDraw(featom *atom, regionhandle region);
void cmShipNumbersDraw(featom *atom, regionhandle region);
void cmTotalRUsDraw(featom *atom, regionhandle region);
void cmTotalShipsDraw(featom *atom, regionhandle region);
void cmConnectorDraw(featom *atom, regionhandle region);
void cmFactoryNameDraw(featom *atom, regionhandle region);
void cmMotherShipDraw(featom *atom, regionhandle region);
void cmCarrier1Draw(featom *atom, regionhandle region);
void cmCarrier2Draw(featom *atom, regionhandle region);
void cmLeftArrowDraw(featom *atom, regionhandle region);
void cmRightArrowDraw(featom *atom, regionhandle region);
void cmSUBarDraw(featom *atom, regionhandle region);

void cmScroller(char *string, featom *atom);
void cmExtendedInfo(char *string, featom *atom);

void cmFillInCarrierXs(void);

ShipType cmKeyToShipType(uword key);

LinkedList listofShipsInProgress;

fecallback cmCallback[] =
{
    {cmClose,               "CM_Close"      },
    {cmBuildShips,          "CM_BuildShips" },
    {cmCancelJobs,          "CM_CancelJobs" },
    {cmPauseJobs,           "CM_PauseJobs"  },
    {cmScroller,            "CM_Scroller"   },
    {cmExtendedInfo,        "CM_ExtendedInfo"},
    {cmResearch,            "CM_Research"   },
    {cmLaunch,              "CM_Launch"     },
    {NULL,                  NULL            }
};
fedrawcallback cmDrawCallback[] =
{
    {cmNumberRUsDraw,           "CM_RUs"                },
    {cmShipCostsDraw,           "CM_ShipCost"           },
    {cmShipInfoDraw,            "CM_Ships"              },
    {cmShipNumbersDraw,         "CM_ShipNumbers"        },
    {cmTotalRUsDraw,            "CM_TotalCost"          },
    {cmTotalShipsDraw,          "CM_TotalShips"         },
    {cmConnectorDraw,           "CM_ConnectorDraw"      },
    {cmFactoryNameDraw,         "CM_FactoryNameDraw"    },
    {cmMotherShipDraw,          "CM_MotherShipDraw"     },
    {cmCarrier1Draw,            "CM_Carrier1Draw"       },
    {cmCarrier2Draw,            "CM_Carrier2Draw"       },
    {cmLeftArrowDraw,           "CM_LeftArrowDraw"      },
    {cmRightArrowDraw,          "CM_RightArrowDraw"     },
    {cmSUBarDraw,               "CM_SUBarDraw"          },
    {NULL,                      NULL                    }
};

extern ShipType svShipType;

KeysToShips cmShipTypes[]=
{
    {F5KEY,          bAttackBomber},
    {F4KEY,          bCloakedFighter},
    {F7KEY,          bHeavyCorvette},
    {F11KEY,         bMultiGunCorvette},
    {F12KEY,         bWorker},
    {F12KEY,         sWorker},
    {-1,             0}
};

bool32 cmPrintHotKey = FALSE;
bool32 cmExtendedInfoActive = FALSE;



//index of first displayed element
sword cmUpperIndex = 0;
//maximum number of displayable elements in our list
uword cmMaxIndex = 18;

//For drawing accelerator arrows
sdword cmArrowIndex = -1;

static scrollbarhandle g_shandle;

//flag indicating loaded status of this screen
fibfileheader *cmScreensHandle = NULL;
static sdword cmScreensRace = -1;
static bool32 cmOpenedFromSensors = FALSE;
extern Camera smCamera;

static char *cmScreenNameForRace(sdword race)
{
    return (race == RACE_Sect) ? CM_ConstructionScreen : CM_BeastConstructionScreen;
}

static char *cmFIBFileForRace(sdword race)
{
    return (race == RACE_Sect) ? CM_FIBFile : CM_BeastFIBFile;
}

//handle to construction task
taskhandle cmBuildTask = -1;

#define NSHIPS_AVAILABLE_SECT_REG  19
#define NSHIPS_AVAILABLE_SECT_BIG  30

#define NSHIPS_AVAILABLE_BEAST_REG  22
#define NSHIPS_AVAILABLE_BEAST_BIG  28

#define NSHIPS_AVAILABLE_P1     4 + 3
#define NSHIPS_AVAILABLE_P2     1 + 1

regionhandle cmBaseRegion        = NULL;
regionhandle cmShipInfoRegion    = NULL;
regionhandle cmShipNumbersRegion = NULL;
regionhandle cmNumberRUsRegion   = NULL;
regionhandle cmTotalRUsRegion    = NULL;
regionhandle cmTotalShipsRegion  = NULL;
regionhandle cmShipCostsRegion   = NULL;
regionhandle cmConnectorRegion   = NULL;
regionhandle cmFactoryRegion     = NULL;
regionhandle cmMotherShipRegion  = NULL;
regionhandle cmSUBarRegion       = NULL;
regionhandle cmCarrierRegions[NUM_CMCARRIERS] = { NULL, NULL };

/*
typedef struct
{
    sdword   disabled;
    ShipType type;
} shipdisabled;

//use [0,1], ![FALSE,TRUE] for disabling
shipdisabled cmShipsDisabled[] =
{
    {0, AdvanceSupportFrigate},
    {1, AttackBomber},
    {0, Carrier},
    {0, CloakedFighter},
    {1, CloakGenerator},
    {1, DDDFrigate},
    {0, DefenseFighter},
    {1, DFGFrigate},
    {1, GravWellGenerator},
    {0, HeavyCorvette},
    {1, HeavyCruiser},
    {0, HeavyDefender},
    {0, HeavyInterceptor},
    {0, IonCannonFrigate},
    {0, LightCorvette},
    {0, LightDefender},
    {0, LightInterceptor},
    {1, MinelayerCorvette},
    {1, MissileDestroyer},
    {1, MultiGunCorvette},
    {0, Probe},
    {0, ProximitySensor},
    {0, RepairCorvette},
    {0, ResearchShip},
    {0, ResourceCollector},
    {0, ResourceController},
    {1, SalCapCorvette},
    {0, SensorArray},
    {0, StandardDestroyer},
    {0, StandardFrigate},
    {0, Drone},
    {-1, 0},
};
*/

//ships available for construction
shipavailable cmShipsAvailableSectReg[NSHIPS_AVAILABLE_SECT_REG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  sRecon              ,  NULL},
    {0,ITEM_SHIP ,0,  sAcolyte            ,  NULL},
    {0,ITEM_SHIP ,0,  dAcolyte            ,  NULL},
    {0,ITEM_SHIP ,0,  sMimic              ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  sACV                ,  NULL},
    {0,ITEM_SHIP ,0,  sMCV                ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  sRammingFrigate     ,  NULL},
    {0,ITEM_SHIP ,0,  sMultiBeamFrigate   ,  NULL},
    {0,ITEM_SHIP ,0,  sHiveFrigate        ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Micro         ,  NULL},
    {0,ITEM_SHIP ,0,  sLeech              ,  NULL},
    {0,ITEM_SHIP ,0,  sSentinel           ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  sWorker             ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Component     ,  NULL},
    {0,ITEM_SHIP ,0,  sCarrierSupport     ,  NULL},

    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableSectBig[NSHIPS_AVAILABLE_SECT_BIG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  sRecon              ,  NULL},
    {0,ITEM_SHIP ,0,  sAcolyte            ,  NULL},
    {0,ITEM_SHIP ,0,  dAcolyte            ,  NULL},
    {0,ITEM_SHIP ,0,  sMimic              ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  sACV                ,  NULL},
    {0,ITEM_SHIP ,0,  sMCV                ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  sRammingFrigate     ,  NULL},
    {0,ITEM_SHIP ,0,  sMultiBeamFrigate   ,  NULL},
    {0,ITEM_SHIP ,0,  sHiveFrigate        ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Destroyer     ,  NULL},
    {0,ITEM_SHIP ,0,  sDestroyer          ,  NULL},
    {0,ITEM_SHIP ,0,  sDreadnought        ,  NULL},
    {0,ITEM_SHIP ,0,  sCarrier            ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Micro         ,  NULL},
    {0,ITEM_SHIP ,0,  sLeech              ,  NULL},
    {0,ITEM_SHIP ,0,  sSentinel           ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  sWorker             ,  NULL},
    {0,ITEM_SHIP ,0,  sProcessor          ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Component     ,  NULL},
    {0,ITEM_SHIP ,0,  sMothershipSupport  ,  NULL},
    {0,ITEM_SHIP ,0,  sMothershipDockingBay, NULL},
    {0,ITEM_SHIP ,0,  sMothershipMicro    ,  NULL},
    {0,ITEM_SHIP ,0,  sMothershipSpecial  ,  NULL},
    {0,ITEM_SHIP ,0,  sMothershipArmour   ,  NULL},
    {0,ITEM_SHIP ,0,  sMothershipWeapons  ,  NULL},
    {0,ITEM_SHIP ,0,  sMothershipBigGun   ,  NULL},


    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableBeastReg[NSHIPS_AVAILABLE_BEAST_REG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  bRecon              ,  NULL},
    {0,ITEM_SHIP ,0,  bInterceptor        ,  NULL},
    {0,ITEM_SHIP ,0,  bCloakedFighter     ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bAcolyte, NULL},
    {0,ITEM_SHIP ,0,  bAttackBomber       ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  bMultiGunCorvette   ,  NULL},
    {0,ITEM_SHIP ,0,  bHeavyCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  bMissileCorvette    ,  NULL},
    {0,ITEM_SHIP ,0,  bCruiseMissile      ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bACV   ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  bIonArrayFrigate    ,  NULL},
    {0,ITEM_SHIP ,0,  bDFGFrigate         ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bRammingFrigate, NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bMultiBeamFrigate, NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bHiveFrigate, NULL},

    {0,ITEM_CLASS,0,  CLASS_Micro         ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bSentinel, NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  bWorker             ,  NULL},

    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableBeastBig[NSHIPS_AVAILABLE_BEAST_BIG + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  bRecon              ,  NULL},
    {0,ITEM_SHIP ,0,  bInterceptor        ,  NULL},
    {0,ITEM_SHIP ,0,  bCloakedFighter     ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bAcolyte, NULL},
    {0,ITEM_SHIP ,0,  bAttackBomber       ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  bMultiGunCorvette   ,  NULL},
    {0,ITEM_SHIP ,0,  bHeavyCorvette      ,  NULL},
    {0,ITEM_SHIP ,0,  bMissileCorvette    ,  NULL},
    {0,ITEM_SHIP ,0,  bCruiseMissile      ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bACV   ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  bIonArrayFrigate    ,  NULL},
    {0,ITEM_SHIP ,0,  bDFGFrigate         ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bRammingFrigate, NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bMultiBeamFrigate, NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bHiveFrigate, NULL},

    {0,ITEM_CLASS,0,  CLASS_Destroyer     ,  NULL},
    {0,ITEM_SHIP ,0,  bCarrier            ,  NULL},
    {0,ITEM_SHIP ,0,  bHeavyCruiser       ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Micro         ,  NULL},
    {0,ITEM_SHIP_REQUIRES_COUNT,0, bSentinel, NULL},

    {0,ITEM_CLASS,0,  CLASS_NonCombat     ,  NULL},
    {0,ITEM_SHIP ,0,  bWorker             ,  NULL},
    {0,ITEM_SHIP ,0,  bProcessor          ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Component     ,  NULL},
    {0,ITEM_SHIP ,0,  bMothershipSupport  ,  NULL},


    {-1,0,0, 0                   ,  NULL}
};

shipavailable cmShipsAvailableP1[NSHIPS_AVAILABLE_P1 + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Fighter       ,  NULL},
    {0,ITEM_SHIP ,0,  P1Fighter           ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Corvette      ,  NULL},
    {0,ITEM_SHIP ,0,  P1StandardCorvette  ,  NULL},
    {0,ITEM_SHIP ,0,  P1MissileCorvette   ,  NULL},

    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  P1IonArrayFrigate   ,  NULL},

    {-1,0,0,0,NULL}
};


shipavailable cmShipsAvailableP2[NSHIPS_AVAILABLE_P2 + 1] =
{
    {0,ITEM_CLASS,0,  CLASS_Frigate       ,  NULL},
    {0,ITEM_SHIP ,0,  tStandardFrigate    ,  NULL},

    {-1,0,0,0,NULL}
};

shipsinprogress *curshipsInProgress;
sdword NUMBER_SHIPS_AVAILABLE = 0;
shipavailable *cmShipsAvailable;

CarrierInfo *cmCarriers = NULL;
sdword cmNumCarriers = 0;
sdword cmNumPlayersCarriers = 0;

static Ship *cmCarrierX[NUM_CMCARRIERS] = { NULL, NULL };     // don't export these for sync reasons

//sdword cmSelectedShip = -1;

//colors for printing selections and stuff
color cmSelectionTextColor = CM_SelectionTextColor;
color cmBuildingTextColor = CM_BuildingTextColor;
color cmProgressShipColor = CM_ProgressShipColor;
color cmProgressTotalColor = CM_ProgressTotalColor;
color cmStandardTextColor = CM_StandardTextColor;
color cmClassHeadingTextColor = CM_ClassHeadingTextColor;
color cmCapsExceededTextColor = CM_CapsExceededTextColor;
udword cmRefundRatio = CM_RefundRatio;
real32 cmFloatRefundRatio = CM_FloatRefundRatio;

bool32 cmIoSaveState;
int lgUpdateChatRegions(void);

udword cmShipTexture[MAX_RACES][NUM_CMTEXTURES] =
{
    {TR_InvalidInternalHandle, TR_InvalidInternalHandle},
    {TR_InvalidInternalHandle, TR_InvalidInternalHandle}
};

lifheader *cmShipImage[MAX_RACES][NUM_CMTEXTURES] =
{
    { NULL, NULL },
    { NULL, NULL }
};


char *ShipImagePaths[] =
{
    {"FEMan\\Construction_Manager\\Build_race1_"},
    {"FEMan\\Construction_Manager\\Build_race2_"}
};

char *ShipIcons[] =
{
    {"icon1_disabled.lif"},
    {"icon1_off.lif"},
    {"icon1_on.lif"},
    {"icon2_disabled.lif"},
    {"icon2_off.lif"},
    {"icon2_on.lif"},
};

udword cmArrowTexture[6] =
{
    TR_InvalidInternalHandle, TR_InvalidInternalHandle,TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle,TR_InvalidInternalHandle
};

lifheader *cmArrowIcon[6] =
{
    NULL, NULL, NULL, NULL, NULL, NULL
};


#define ARROW_ICON_PATH "FEMan\\Textures\\"

char *ArrowIcons[] =
{
    {"Build_arrow_left_mouse.lif"},
    {"Build_Arrow_Left_on.lif"},
    {"Build_Arrow_Left_off.lif"},
    {"Build_Arrow_right_mouse.lif"},
    {"Build_Arrow_right_on.lif"},
    {"Build_Arrow_right_off.lif"},
};

udword cmSupportTexture[NUM_CMSUPPORTTEXTURES] =
{
    TR_InvalidInternalHandle,
    TR_InvalidInternalHandle,
    TR_InvalidInternalHandle,
    TR_InvalidInternalHandle,
    TR_InvalidInternalHandle
};

lifheader *cmSupportIcon[NUM_CMSUPPORTTEXTURES] =
{
    NULL, NULL, NULL, NULL, NULL
};

#define CM_SUPPORT_ICON_PATH "Feman\\texDecorative\\"

char *cmSupportIcons[] =
{
    {"b_Build_SupportMask_Top.lif"},
    {"b_Build_SupportMask_Mid.lif"},
    {"b_Build_SupportMask_Low.lif"},
    {"b_Build_SupportIcon_Top.lif"},
    {"b_Build_SupportIcon_Low.lif"},
};




sdword cmCurrentSelect = 0;



//font for the ship listing
fonthandle cmDefaultFont = 0;
fonthandle cmShipListFont = 0;
fonthandle cmTitleFont = 0;
char cmDefaultFontName[CM_FontNameLength]  = CM_DefaultFont;
char cmShipListFontName[CM_FontNameLength] = CM_ShipListFont;
char cmTitleFontName[CM_FontNameLength]    = CM_TitleFont;

// frame count for red flash when ship limits exceeded
#define CAPS_REDFRAMECOUNT 5;

// Ship Caps exceeded falgs
real32 ExceededCapsFrames = 0.0;
uword ShipNumberExceeded = 0;

bool8 cmLeftArrowActive  = FALSE;
bool8 cmRightArrowActive = FALSE;

// Variable for storing the last viewed ship
ShipType curshipview=DefaultShip;

// Hack to fix Johan's hack ...
#define CM_WAIT_ACC 0.25f
extern real32 mgAccTime;

// Fix for unit caps in internet game ...
sdword shiplagtotals[TOTAL_NUM_SHIPS];

bool32 cmReallyPausedAll = FALSE;

static sdword cmBeastBuildFactor(Ship *ship)
{
    const BeastMothershipRuntimeState *runtimeState = bMothershipRuntimeStateConst(ship);

    if (runtimeState->infectionLevel != 0 &&
        runtimeState->systemType == BS_Build)
    {
        return 2;
    }
    return 1;
}

bool32 cmActive=FALSE;

shipsinprogress *cmFindFactory(struct Ship *ship);
LinkedList cmDetermProgress;

void cmCancelFactoryExternalBuilds(Ship *factoryShip)
{
    shipsinprogress *factory;
    Node *node;
    Node *nextNode;
    sdword shipType;

    if (factoryShip == NULL)
    {
        return;
    }

    factory = cmFindFactory(factoryShip);
    if (factory != NULL)
    {
        for (shipType = 0; shipType < TOTAL_NUM_SHIPS; shipType++)
        {
            shipinprogress *progress = &factory->progress[shipType];

            if (progress->info == NULL || !progress->info->externalBuild)
            {
                continue;
            }

            progress->timeFraction = 0;
            progress->cataSaveField0 = 0;
            progress->cataSaveField1 = 0;
            progress->cataSaveField2 = 0;
            progress->timeStart = 0;
            progress->timeLeft = 0;
            progress->nJobsTotal = 0;
            progress->nJobs = 0;
            progress->costSoFar = 0;
        }
    }

    for (node = cmDetermProgress.head; node != NULL; node = nextNode)
    {
        cmDetermProgress_t *progress = listGetStructOfNode(node);
        ShipStaticInfo *shipStaticInfo;

        nextNode = node->next;
        if (progress->creator != factoryShip)
        {
            continue;
        }

        shipStaticInfo = GetShipStaticInfo(progress->shipType, progress->shipRace);
        if (shipStaticInfo != NULL && shipStaticInfo->externalBuild)
        {
            listDeleteNode(node);
        }
    }

    cmDirtyShipInfo();
}

//max number of ship types per class that can be built
sdword cmClassCapRemap[NUM_CLASSES]=
{
    CLASS_Mothership,
    CLASS_Destroyer,
    CLASS_Destroyer,
    CLASS_Destroyer,
    CLASS_Frigate,
    CLASS_Corvette,
    CLASS_Fighter,
    CLASS_Resource,
    CLASS_NonCombat,
    CLASS_Destroyer,
    CLASS_Component,
    CLASS_Micro,
    CLASS_Corvette
};
sdword cmMaxJobsPerClass[NUM_CLASSES] =
{
    CM_MaxJobsMothership,
    CM_MaxJobsHeavyCruiser,
    CM_MaxJobsCarrier,
    CM_MaxJobsDestroyer,
    CM_MaxJobsFrigate,
    CM_MaxJobsCorvette,
    CM_MaxJobsFighter,
    CM_MaxJobsResource,
    CM_MaxJobsNonCombat,
    CM_MaxJobsDestroyer,
    CM_MaxJobsNonCombat,
    CM_MaxJobsFighter,
    CM_MaxJobsCorvette,
};

/*=============================================================================
    Private functions:
=============================================================================*/

void updateShipView(void)
{
    udword index;
    bool32   set=TRUE;

    if (curshipview == DefaultShip)
    {
        for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
        {                                                       //for all available ships
            if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
            {
                if(cmShipsAvailable[index].itemstat==STAT_CANBUILD)
                {
                    svSelectShip(cmShipsAvailable[index].info->shiptype);
                    curshipview = cmShipsAvailable[index].info->shiptype;
                    break;
                }
            }
        }
    }
    else
    {
        for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
        {                                                       //for all available ships
            if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
                 (cmShipsAvailable[index].info->shiptype==curshipview) )
            {
                if (cmShipsAvailable[index].itemstat!=STAT_CANBUILD)
                {
                    break;
                }
                else
                {
                    svSelectShip(curshipview);
                    set = FALSE;
                }
            }
        }
        if (set)
        {
            for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
            {                                                       //for all available ships
                if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
                {
                    if(cmShipsAvailable[index].itemstat==STAT_CANBUILD)
                    {
                        svSelectShip(cmShipsAvailable[index].info->shiptype);
                        curshipview = cmShipsAvailable[index].info->shiptype;
                        break;
                    }
                }

            }
        }
    }
}

//return the progress pointed to by index
shipinprogress *cmSIP(udword index)
{
    //return &curshipsInProgress->progress[index];
    shipsinprogress *factory;
    factory = curshipsInProgress;

    return &factory->progress[index];
}


void cmWheelNegative()
{
    sdword ui = (sdword)cmUpperIndex-3;

    if (ui < 0)
        ui = 0;
    cmUpperIndex = (uword)ui;
}

void cmWheelPositive()
{
    sdword ui = (sdword)cmUpperIndex + 3;

    while (ui + cmMaxIndex > NUMBER_SHIPS_AVAILABLE)
    {
        ui--;
        if (ui < 0)
        {
            ui = 0;
            break;
        }
    }
    cmUpperIndex = (sword)ui;
}

void cmDirtyShipInfo(void)
{
    sdword i;
    if (cmBaseRegion == NULL)
    {
        return;
    }

    cmUpdateShipAvailable();

    if (cmShipInfoRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmShipInfoRegion);
#endif

        cmShipInfoRegion->status |= RSF_DrawThisFrame;
    }
    if (cmNumberRUsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmNumberRUsRegion);
#endif
        cmNumberRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalRUsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmTotalRUsRegion);
#endif
        cmTotalRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalShipsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmTotalShipsRegion);
#endif
        cmTotalShipsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipCostsRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmShipCostsRegion);
#endif
        cmShipCostsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipNumbersRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmShipNumbersRegion);
#endif
        cmShipNumbersRegion->status |= RSF_DrawThisFrame;
    }
    if (cmConnectorRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmConnectorRegion);
#endif
        cmConnectorRegion->status |= RSF_DrawThisFrame;
    }
    if (cmSUBarRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmSUBarRegion);
#endif
        cmSUBarRegion->status |= RSF_DrawThisFrame;
    }

    if (cmMotherShipRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(cmMotherShipRegion);
#endif
        cmMotherShipRegion->status |= RSF_DrawThisFrame;
    }
    for (i=0;i<NUM_CMCARRIERS;i++)
    {
        if (cmCarrierRegions[i] != NULL)
        {
    #ifdef DEBUG_STOMP
            regVerify(cmCarrierRegions[i]);
    #endif
            cmCarrierRegions[i]->status |= RSF_DrawThisFrame;
        }
    }
}

void cmScroller(char *string, featom *atom)
{
    scrollbarhandle shandle;
    sword ind;

    if (FEFIRSTCALL(atom))
    {
        g_shandle = scSetupThumbwheel(cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE, atom);
        return;
    }

    shandle = (scrollbarhandle)atom->pData;

    if (shandle->mouseMoved != 0)
    {
        sword divs, up;
        real32 mm;

        cmDirtyShipInfo();

        mm = (real32)shandle->mouseMoved;
        divs = (sword)(mm / shandle->divSize);
        up = (sword)(cmUpperIndex + divs);
        if (up < 0)
            up = 0;
        else if (up > NUMBER_SHIPS_AVAILABLE - cmMaxIndex)
            up = (sword)(NUMBER_SHIPS_AVAILABLE - cmMaxIndex);

        if (up <0)
            up = 0;

        cmUpperIndex = (uword)up;

        scAdjustThumbwheel(shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
        shandle->mouseMoved = 0;    //clear flag
        return;
    }

    switch (shandle->event)
    {
        case SC_Other:
        ind = (sword)cmUpperIndex;
        if (mouseCursorY() < shandle->thumb.y0)
        {
            ind = (sword)(ind - cmMaxIndex);
            if (ind < 0) ind = 0;
            cmUpperIndex = (uword)ind;
        }
        else
        {
            ind = (sword)(ind + cmMaxIndex);
            if (ind > NUMBER_SHIPS_AVAILABLE - cmMaxIndex)
                ind = (sword)(NUMBER_SHIPS_AVAILABLE - cmMaxIndex);

                        if (ind <0)
                                ind = 0;

            cmUpperIndex = (uword)ind;
        }
        cmDirtyShipInfo();
        break;
    case SC_Thumb:
        break;
    case SC_Negative:
        cmWheelNegative();
        cmDirtyShipInfo();
        break;
    case SC_Positive:
        cmWheelPositive();
        cmDirtyShipInfo();
        break;
    default:
        dbgMessagef("\nunhandled consMgr scroll event %d", shandle->clickType);
    }

    scAdjustThumbwheel(shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
}

void cmExtendedInfo(char *string, featom *atom)
{
    cmExtendedInfoActive = !cmExtendedInfoActive;
    cmDirtyShipInfo();
}

shipsinprogress *cmFindFactory(struct Ship *ship)
{
    Node *node = listofShipsInProgress.head;
    shipsinprogress *sinprogress;

    while (node != NULL)
    {
        sinprogress = (shipsinprogress *)listGetStructOfNode(node);
        if (sinprogress->ship == ship)
        {
            return sinprogress;
        }
        node = node->next;
    }
    return NULL;
}

void cmAddFactory(struct Ship *ship,bool32 canBuildBigShips)
{
    shipsinprogress *sinprogress;

    if (singlePlayerGame && ship->specialFlags3 != 0)
    {
        return;
    }

    sinprogress = memAlloc(sizeof(shipsinprogress),"sinprogress",NonVolatile);
    memset(sinprogress,0,sizeof(shipsinprogress));

    if (ShipTypeIsCarrierType(ship->shiptype))       // add even if not for the player, because we want cmCarriers to be
                                            // deterministic for all machines to prevent sync problems
    {
        cmNumCarriers++;
        cmCarriers = memRealloc(cmCarriers, sizeof(CarrierInfo) * cmNumCarriers, "cmCarriers", NonVolatile);
        cmCarriers[cmNumCarriers - 1].ship  = ship;
        cmCarriers[cmNumCarriers - 1].owner = ship->playerowner;
    }
    sinprogress->ship = ship;
    sinprogress->canBuildBigShips = canBuildBigShips;
    listAddNode(&listofShipsInProgress,&sinprogress->node,sinprogress);
    cmDirtyShipInfo();

    cmFillInCarrierXs();
}

void cmReset(void)
{
    if (!singlePlayerGameLoadNewLevelFlag)      // preserve this stuff if inbetween single player levels
    {
        listDeleteAll(&listofShipsInProgress);
        if (cmCarriers != NULL)
        {
            sdword i;
            memFree(cmCarriers);
            cmCarriers = NULL;
            cmNumCarriers = 0;
            for (i=0;i<NUM_CMCARRIERS;i++)
            {
                cmCarrierX[i] = NULL;
            }
        }
    }

    cmFillInCarrierXs();
}

void cmCloseIfOpen(void)
{
    if (cmBaseRegion != NULL)
    {
        cmClose(NULL,NULL);
    }
}

void cmResearch(char *string, featom *atom)
{
    cmClose(string, atom);
    mrResearch(string, atom);
}

void cmLaunch(char *string, featom *atom)
{
    cmClose(string, atom);
    mrLaunch(string, atom);
}

void cmPauseAllJbos(void)
{
    cmReallyPausedAll = TRUE;
}

void cmUnPauseAllJobs(void)
{
    cmReallyPausedAll = FALSE;
}

void cmRemoveFactory(struct Ship *ship)
{
    sdword i, j, k;
    Node *node;
    Node *nextnode;
    cmDetermProgress_t *dprog;
    shipsinprogress *sinprogress = cmFindFactory(ship);
    if (sinprogress != NULL)
    {
        listDeleteNode(&sinprogress->node);
        if (curshipsInProgress == sinprogress)
        {
            curshipsInProgress = NULL;
            cmCloseIfOpen();
        }
    }

    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t *)listGetStructOfNode(node);
        nextnode = node->next;

        if (dprog->creator == ship)
        {
            listDeleteNode(node);
        }

        node = nextnode;
    }

    for (i = 0; i < cmNumCarriers; i++)
    {
        if (cmCarriers[i].ship == ship)
        {
            for (k=0;k<NUM_CMCARRIERS;k++)
            {
                if (cmCarriers[i].ship == cmCarrierX[k])
                    cmCarrierX[k] = NULL;
            }

            // Resize the cmCarriers array
            for (j = i + 1; j < cmNumCarriers; j++)
            {
                cmCarriers[j - 1] = cmCarriers[j];
            }
            cmNumCarriers--;

            if (cmNumCarriers > 0)
            {
                cmCarriers = memRealloc(cmCarriers, sizeof(CarrierInfo) * cmNumCarriers, "cmCarriers", NonVolatile);
            }
            else
            {
                memFree(cmCarriers);
                cmCarriers = NULL;
            }

            break;
        }
    }
    cmDirtyShipInfo();

    cmFillInCarrierXs();
}

static bool32 resourcesHaveBeenAboveBefore = TRUE;

#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void cmBuildTaskFunction(void)
{
    static sdword index;
    static shipinprogress *progress;
    static Node *node;
    static shipsinprogress *sinprogress;
    static Ship *factoryship;
    static sdword decrement;

    if ((!gameIsRunning) || (cmReallyPausedAll))
    {
        return;
    }

    if (playPackets)
    {
        return;     // don't want to execute this routine which messes up resourceUnits, etc in record playback
    }

    if (keyIsHit(BACKSPACEKEY))
    {
        cmBuildNarratorUpdate();
    }

/*    taskYield(0);

    while (1)
    {
        if ((!gameIsRunning) || (cmReallyPausedAll))
        {
            taskYield(0);
            continue;
        }
        taskStackSaveCond(0);*/

        if (universe.curPlayerPtr->resourceUnits >= 300)
        {
            resourcesHaveBeenAboveBefore = TRUE;
        }

        node = listofShipsInProgress.head;
        while (node != NULL)
        {
            sinprogress = (shipsinprogress *)listGetStructOfNode(node);
            factoryship = sinprogress->ship;

            //this is a patch to avoid a very intermittent crash.
            if (!factoryship)
            {
                node = node->next;
                continue;
            }

            if (factoryship->playerowner == universe.curPlayerPtr)
            {
                for (index = 0, progress = &sinprogress->progress[0]; index < TOTAL_NUM_SHIPS; index++, progress++)
                {                                                   //for all classes of ships
                    if (universe.curPlayerPtr->resourceUnits <= 0)
                    {
                        break;
                    }

                    if (!progress->paused)
                    {
                        if (progress->nJobs > 0)
                        {
                            bool32 externalBuild;

                            dbgAssert(progress->info != NULL);
                            externalBuild = (LOBYTE(progress->info->externalBuild) == 1);

                            if (progress->timeLeft > 0 &&
                                (!externalBuild ||
                                 (progress->cataSaveField1 != 0 &&
                                  progress->cataSaveField0 != 0)))
                            {                                               //for each construction job
                            // Simulating fixed point because we need to save fractional bits across frames
                            // RUs -= 0.5 cannot be simulated properly with integer
    #if CM_CHEAP_SHIPS
                            if (cmCheapShips && !multiPlayerGame)
                            {
                                decrement = 1;
                            }
                            else
    #endif
                            {
                                if (singlePlayerGame)
                                {
                                    decrement = ((progress->info->buildCost << (17 - GameDifficultyProductionModifier)) / progress->timeStart) +
                                        progress->timeFraction;
                                }
                                else
                                {
                                    decrement = ((progress->info->buildCost << 16) / progress->timeStart) +
                                        progress->timeFraction;
                                }
                                progress->timeFraction = decrement & 0x0000FFFF; // save fraction bits
                                decrement >>= 16; // Convert back to integer
                            }
/*#if (!UNIVERSE_TURBORECORD_ONLY)
#if UNIVERSE_TURBOPAUSE_DEBUG
                            if (universeTurbo)
                            {
                                decrement *= (UNIVERSE_TURBO_REPEAT+1);
                            }
#endif


#endif*/
                            if (!universePause)
                            {
                                if (factoryship->shiptype == bMothership)
                                {
                                    decrement *= cmBeastBuildFactor(factoryship);
                                }

                                progress->costSoFar += decrement;
                                if (!multiPlayerGame)
                                {
                                    //xxx
                                    universe.curPlayerPtr->resourceUnits -= decrement;// decrement RU's
                                }
                                if(universe.curPlayerPtr->resourceUnits <= 0)
                                {
                                    if (resourcesHaveBeenAboveBefore)
                                    {
//                                        speechEventFleet(STAT_F_ResLevels_Nil, 0, universe.curPlayerIndex);
                                        speechEventFleet(STAT_F_ConstInProgs_ResInsufficient,
                                                         0,
                                                         universe.curPlayerIndex);
                                        resourcesHaveBeenAboveBefore = FALSE;
                                    }
                                    if (!multiPlayerGame)
                                    {
                                        //xxx
                                        universe.curPlayerPtr->resourceUnits = 0;
                                    }
                                }

        #if CM_CHEAP_SHIPS
                                if (cmCheapShips && !multiPlayerGame)
                                {
                                    progress->timeLeft = 0;
                                }
                                else
        #endif
                                {
                                    if (factoryship->shiptype == bMothership)
                                    {
                                        progress->timeLeft -= cmBeastBuildFactor(factoryship);
                                    }
                                    else
                                    {
                                        progress->timeLeft--;                   //decrement the time
                                    }
                                }

                                if (externalBuild &&
                                    progress->cataSaveField2 != 0 &&
                                    progress->timeLeft > 0 &&
                                    !multiPlayerGame)
                                {
                                    clWrapExternalConstruction(&universe.mainCommandLayer,
                                                               universe.curPlayerIndex,
                                                               factoryship,
                                                               (void *)progress->cataSaveField2,
                                                               cmExternalBuildProgress(progress->timeLeft, progress->timeStart),
                                                               2);
                                }
                            }

                            if (progress->timeLeft <= 0)
                            {                                           //if job has ended
                                if(progress->timeFraction > 32767) // > 0.5, round up and reduce the resourceUnits by 1
                                {
                                    if (!multiPlayerGame)
                                    {
                                        if (factoryship->shiptype == bMothership)
                                        {
                                            universe.curPlayerPtr->resourceUnits -= cmBeastBuildFactor(factoryship);
                                        }
                                        else
                                        {
                                            universe.curPlayerPtr->resourceUnits--;
                                        }
                                    }
                                    if(universe.curPlayerPtr->resourceUnits <= 0)
                                    {
                                        if (resourcesHaveBeenAboveBefore)
                                        {
                                            speechEventFleet(STAT_F_ResLevels_Nil, 0, universe.curPlayerIndex);
                                            resourcesHaveBeenAboveBefore = FALSE;
                                        }
                                        if (!multiPlayerGame)
                                        {
                                            //xxx
                                            universe.curPlayerPtr->resourceUnits = 0;
                                        }
                                    }
                                }

#if CM_VERBOSE_LEVEL >= 1
                                dbgMessagef("\nConstruction job #%d has finished a %s",
                                            index, ShipTypeToNiceStr(progress->info->shiptype));
#endif
//yyy                                clWrapRUTransfer(&universe.mainCommandLayer,0,universe.curPlayerPtr->playerIndex,progress->info->buildCost,RUTRANS_BUILTSHIP);
                                cmShipBuildComplete(progress, factoryship);
                            }
                            }

                            if (progress->nJobs > 0 &&
                                (progress->timeLeft == progress->timeStart ||
                                 progress->timeLeft <= 0) &&
                                externalBuild &&
                                !progress->cataSaveField1)
                            {
                                if (cmBuildStateChanged(factoryship,
                                                        progress->info->shipclass,
                                                        progress->info->shiptype,
                                                        TRUE,
                                                        0) &&
                                    (progress->info->shipclass == CLASS_Component ||
                                     factoryship->shiptype == bMothership ||
                                     univPlayerHasShipType(sMothershipDockingBay, universe.curPlayerPtr)))
                                {
                                    if (!multiPlayerGame)
                                    {
                                        progress->cataSaveField2 = (sdword)clWrapCreateShip(
                                            &universe.mainCommandLayer,
                                            progress->info->shiptype,
                                            universe.curPlayerPtr->race,
                                            universe.curPlayerIndex,
                                            factoryship);
                                        cmDirtyCarrierCount();
                                    }

                                    progress->timeFraction = 0;
                                    progress->timeStart = singlePlayerGame ?
                                        (progress->info->buildTime / GameDifficultyBuildTimeModifier) :
                                        progress->info->buildTime;
                                    progress->timeLeft = progress->timeStart;

                                    if (!multiPlayerGame)
                                    {
                                        progress->cataSaveField1 = 1;
                                        progress->cataSaveField0 = 0;
                                    }
                                }
                                else
                                {
                                    progress->timeFraction = 0;
                                    progress->timeStart = singlePlayerGame ?
                                        (progress->info->buildTime / GameDifficultyBuildTimeModifier) :
                                        progress->info->buildTime;
                                    progress->timeLeft = progress->timeStart;
                                }
                            }
                        }
                    }
                }
            }
            node = node->next;
        }

/*        taskStackRestoreCond();
        taskYield(0);
    }*/
}
#pragma optimize("", on)

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : cmLoadTextures
    Description : load textures used by the construction manager
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmLoadTextures(void)
{
    sdword race, index;
    char filename[128];

    for (race = 0; race < MAX_RACES; race++)
    {
        for (index = 0; index < NUM_CMTEXTURES; index++)
        {
            // build filename for loading texture from file
            strcpy(filename, ShipImagePaths[race]);
            strcat(filename, ShipIcons[index]);

            // load the image
            cmShipImage[race][index] = trLIFFileLoad(filename, NonVolatile);

            if (bitTest(cmShipImage[race][index]->flags, TRF_Paletted))
            {
                cmPaletted = TRUE;
            }
            else
            {
                cmPaletted = FALSE;
            }

            if (cmPaletted)
            {
                cmShipTexture[race][index] = trPalettedTextureCreate(
                    cmShipImage[race][index]->data,
                    cmShipImage[race][index]->palette,
                    cmShipImage[race][index]->width,
                    cmShipImage[race][index]->height);
            }
            else
            {
                cmShipTexture[race][index] = trRGBTextureCreate(
                    (color*)cmShipImage[race][index]->data,
                    cmShipImage[race][index]->width,
                    cmShipImage[race][index]->height,
                    TRUE);
            }
        }
    }

    for (index = 0; index < 6; index++)
    {
#if 1
        cmArrowIcon[index] = NULL;
        cmArrowTexture[index] = TR_InvalidInternalHandle;
#else
        strcpy(filename, ARROW_ICON_PATH);
        strcat(filename, ArrowIcons[index]);

        cmArrowIcon[index] = trLIFFileLoad(filename, NonVolatile);
        dbgAssert(cmArrowIcon[index]);

        cmArrowTexture[index] = trRGBTextureCreate(
            (color*)cmArrowIcon[index]->data,
            cmArrowIcon[index]->width,
            cmArrowIcon[index]->height,
            TRUE);
#endif
    }

    for (index = 0; index < NUM_CMSUPPORTTEXTURES; index++)
    {
        strcpy(filename, CM_SUPPORT_ICON_PATH);
        strcat(filename, cmSupportIcons[index]);

        cmSupportIcon[index] = trLIFFileLoad(filename, NonVolatile);
        dbgAssert(cmSupportIcon[index] != NULL);

        cmSupportTexture[index] = trRGBTextureCreate(
            (color *)cmSupportIcon[index]->data,
            cmSupportIcon[index]->width,
            cmSupportIcon[index]->height,
            TRUE);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmCloseTextures
    Description : free memory and handles associated with any textures we might be using
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmCloseTextures(void)
{
    sdword race, index;

    for (race = 0; race < MAX_RACES; race++)
    {
        for (index = 0; index < NUM_CMTEXTURES; index++)
        {
            if (cmShipImage[race][index] != NULL)
            {
                memFree(cmShipImage[race][index]);
                cmShipImage[race][index] = NULL;
            }
            if (cmShipTexture[race][index] != TR_InvalidInternalHandle)
            {
                trRGBTextureDelete(cmShipTexture[race][index]);
                cmShipTexture[race][index] = TR_InvalidInternalHandle;
            }
        }
    }

    for (index = 0; index < 6; index++)
    {
        if (cmArrowIcon[index] != NULL)
        {
            memFree(cmArrowIcon[index]);
            cmArrowIcon[index] = NULL;
        }
        if (cmArrowTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(cmArrowTexture[index]);
            cmArrowTexture[index] = TR_InvalidInternalHandle;
        }
    }

    for (index = 0; index < NUM_CMSUPPORTTEXTURES; index++)
    {
        if (cmSupportIcon[index] != NULL)
        {
            memFree(cmSupportIcon[index]);
            cmSupportIcon[index] = NULL;
        }
        if (cmSupportTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(cmSupportTexture[index]);
            cmSupportTexture[index] = TR_InvalidInternalHandle;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmStartup
    Description : Start the construction manager module.
    Inputs      : void
    Outputs     : Starts a task to take care of construction.
    Return      : void
----------------------------------------------------------------------------*/
void cmStartup(void)
{
    sdword index;

    cmDeterministicStartup();

    listInit(&listofShipsInProgress);
//    cmBuildTask = taskStart(cmBuildTaskFunction, CM_TaskServicePeriod, 0);
    cmDefaultFont = frFontRegister(cmDefaultFontName);
    cmShipListFont = frFontRegister(cmShipListFontName);
    cmTitleFont = frFontRegister(cmTitleFontName);

    cmStandardTextColor = FEC_ListItemStandard;

    cmLoadTextures();

    for (index=0;index<TOTAL_NUM_SHIPS;index++)
    {
        shiplagtotals[index] = 0;
    }

    cmRenderEverythingCounter = 0;

    /*
    textureRect.x0 = rect.x0+SP_TEXTURE_INSET;
    textureRect.y0 = rect.y0+SP_TEXTURE_INSET;
    textureRect.x1 = rect.x1-SP_TEXTURE_INSET;
    textureRect.y1 = rect.y1-SP_TEXTURE_INSET;

    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&textureRect);
    */
}

/*-----------------------------------------------------------------------------
    Name        : cmShutdown
    Description : Shut down the task manager
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void cmShutdown(void)
{
    cmDeterministicShutdown();

    if (cmScreensHandle)
    {
        feScreensDelete(cmScreensHandle);
        cmScreensHandle = NULL;
    }

    if (cmBuildTask != -1)
    {
        taskStop(cmBuildTask);
    }

    cmCloseTextures();

    cmReset();
}

/*-----------------------------------------------------------------------------
    Name        : cmConnectorDraw
    Description : Draws the connector between the image of the ship and
                  the info. text
    Inputs      : region
    Outputs     : connector image
    Return      : void
----------------------------------------------------------------------------*/
void cmConnectorDraw(featom *atom, regionhandle region)
{
    sdword centerX  = region->rect.x0 + ((region->rect.x1 - region->rect.x0) / 2);

    cmConnectorRegion = region;

    primLine2(centerX, region->rect.y0, centerX, region->rect.y1, colRGB(255, 0, 0));
    primCircleSolid2(centerX, region->rect.y1, 3, 16, colRGB(255, 0, 0));
}

void cmFactoryNameDraw(featom *atom, regionhandle region)
{
    fonthandle oldfont;
    Ship *shipfactory;
    sdword y;

    cmFactoryRegion = region;
    oldfont = fontMakeCurrent(cmTitleFont);

    if (cmCurrentSelect != 0)
    {
        shipfactory = cmCarrierX[cmCurrentSelect - 1];
    }
    else
    {
        shipfactory = universe.curPlayerPtr->PlayerMothership;
    }

    if (shipfactory != NULL)
    {
        y = region->rect.y1 - fontHeight(" ");
        fontPrintf(region->rect.x0, y, colRGB(255, 255, 255), "%s", shipNameGet(shipfactory));
    }

    primLineThick2(region->rect.x0, region->rect.y1, region->rect.x1, region->rect.y1, 2, colRGB(255, 255, 255));
    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : cmShipDisabled
    Description : determines whether a given shiptype has been disabled for building or not
    Inputs      : type - ship type
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
/*
bool32 cmShipDisabled(ShipType type)
{
    sdword index;

    for (index = 0; cmShipsDisabled[index].disabled != -1; index++)
    {
        if (cmShipsDisabled[index].type == type &&
            cmShipsDisabled[index].disabled)
        {
            return TRUE;
        }
    }

    return FALSE;
}
*/
/*-----------------------------------------------------------------------------
    Name        : cmUpdateShipAvailable
    Description : updates the ship build list for technology state.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void cmUpdateShipAvailable(void)
{
    sdword index;
    sdword isclass=STAT_DONTPRINT;
    Ship *shipfactory = NULL;

    NUMBER_SHIPS_AVAILABLE=0;

    if (cmCurrentSelect != 0)
    {
        shipfactory = cmCarrierX[cmCurrentSelect - 1];
    }

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {
        if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
        {
            if (((cmShipsAvailable[index].itemtype != ITEM_SHIP_REQUIRES_COUNT) ||
                 (cmGetShipTypeCount(universe.curPlayerPtr, cmShipsAvailable[index].type) != 0)) &&
                rmCanBuildShip(universe.curPlayerPtr, cmShipsAvailable[index].type) &&
                bitTest(cmShipsAvailable[index].info->staticheader.infoFlags, IF_InfoLoaded))
            {
                cmShipsAvailable[index].itemstat = STAT_CANBUILD;
                NUMBER_SHIPS_AVAILABLE++;

                if ((cdLimitCaps[cmShipsAvailable[index].type] > 0) &&
                    (cmShipsAvailable[index].type == sCarrierSupport) &&
                    (shipfactory != NULL) &&
                    (shipfactory->shiptype == sCarrier))
                {
                    if (cmCountAvailableCarrierSupport(shipfactory) >= cdLimitCaps[cmShipsAvailable[index].type])
                    {
                        cmShipsAvailable[index].itemstat = STAT_INACTIVE;
                    }
                }
                else if ((cdLimitCaps[cmShipsAvailable[index].type] > 0) &&
                         (universe.curPlayerPtr->shiptotals[cmShipsAvailable[index].type] >= cdLimitCaps[cmShipsAvailable[index].type]))
                {
                    cmShipsAvailable[index].itemstat = STAT_INACTIVE;
                }

                if ((cmShipsAvailable[index].itemstat == STAT_CANBUILD) &&
                    (cdLimitCaps[cmShipsAvailable[index].type] == -1))
                {
                    if ((universe.curPlayerPtr->playerIndex == 0) || (!singlePlayerGame))
                    {
                        udword requiredSupportUnits = ShipStaticInfoGetSupportUnits(cmShipsAvailable[index].info);

                        if (requiredSupportUnits != 0)
                        {
                            if ((universe.curPlayerPtr->supportUnitsUsed >= universe.curPlayerPtr->supportUnitsMax) ||
                                ((udword)(universe.curPlayerPtr->supportUnitsMax - universe.curPlayerPtr->supportUnitsUsed) <
                                 (cmCurrentSupportUnitsUsed(universe.curPlayerPtr) + requiredSupportUnits)))
                            {
                                cmShipsAvailable[index].itemstat = STAT_INACTIVE;
                            }
                        }
                    }
                }
            }
            else
            {
                cmShipsAvailable[index].itemstat = STAT_CANTBUILD;
            }
        }
    }

    for (; index >= 0; index--)
    {
        if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
        {
            if (cmShipsAvailable[index].itemstat != STAT_CANTBUILD)
                isclass=STAT_PRINT;
        }
        else if (cmShipsAvailable[index].itemtype==ITEM_CLASS)
        {
            cmShipsAvailable[index].itemstat = (sword)isclass;
            if (isclass==STAT_PRINT) NUMBER_SHIPS_AVAILABLE++;
            isclass=STAT_DONTPRINT;
        }
    }
}

sdword cmCountAvailableCarrierSupport(Ship *carrier)
{
    ClampInfo *clampInfo;
    SelectCommand *selection;
    sdword numSupportShips = 0;
    sdword i;

    if (carrier == NULL)
    {
        return 0;
    }

    // Cataclysm: sCarrierSupport modules are externally clamped to the carrier
    // once built (dockPutShipOutside removes them from the inside-hangar list),
    // so they must be counted via clampInfo->selection, not the inside-hangar list.
    clampInfo = carrier->clampInfo;
    if (clampInfo == NULL)
    {
        return 0;
    }

    selection = (SelectCommand *)clampInfo->selection;
    if (selection == NULL)
    {
        return 0;
    }

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *supportShip = selection->ShipPtr[i];

        if ((supportShip->shiptype == sCarrierSupport) &&
            !bitTest(supportShip->specialFlags2, SPECIAL_2_DockFlag))
        {
            numSupportShips++;
        }
    }

    return numSupportShips;
}


/*-----------------------------------------------------------------------------
    Name        : cmClose
    Description : Callback function to close the construction manager
    Inputs      :
    Outputs     : Deletes all regions associated with construction manager
    Return      :
----------------------------------------------------------------------------*/
void cmClose(char *string, featom *atom)
{                                                           //close the construction manager
    sdword i;
    regionhandle baseRegion = cmBaseRegion;
    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildClose)
        return;

    cmRenderEverythingCounter = 0;

#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nClose construction manager.");
#endif
    curshipsInProgress = NULL;
    cmUpperIndex=0;
    feScreenDeleteFlags(baseRegion,FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND);
    cmBaseRegion = NULL;

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    spRestoreManagerScreen();

    for (i = 0; i < NUM_CMCARRIERS; i++)
    {
        cmCarrierRegions[i] = NULL;
    }
    cmShipInfoRegion    = NULL;
    cmNumberRUsRegion   = NULL;
    cmTotalRUsRegion    = NULL;
    cmTotalShipsRegion  = NULL;
    cmShipCostsRegion   = NULL;
    cmShipNumbersRegion = NULL;
    cmMotherShipRegion  = NULL;
    cmSUBarRegion       = NULL;
    cmFactoryRegion     = NULL;
    cmConnectorRegion   = NULL;

    opKeyDetour = 0; //no key detours

    cmPrintHotKey = FALSE;
    cmActive = FALSE;
    mgResetShipListFilter();

    if (cmOpenedFromSensors)
    {
        smSensorsForceOpen = TRUE;
        cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
        smSensorsBegin(NULL, NULL);
    }

    lgUpdateChatRegions();

}
/*-----------------------------------------------------------------------------
    Name        : cmBuildShips
    Description : Callback function to build selected ships.
    Inputs      :
    Outputs     : Start building all the selected ships
    Return      :
----------------------------------------------------------------------------*/
void cmBuildShips(char *string, featom *atom)
{
    sdword index;
    bool32 builtSomething = FALSE;
    bool32 capShipBuilt = FALSE;
    bool32 mixedCapShipTypes = FALSE;
    ShipType capShipType = 0;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildBuildShips)
        return;

#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\nBuild selected ships..");
#endif

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
/*        if (cmSIP(index)->paused)
        {
            cmSIP(index)->paused = FALSE;
        }*/
        if (cmShipsAvailable[index].nJobs > 0)
        {
            builtSomething = TRUE;
            cmBuildJobsAdd(curshipsInProgress,
                           cmShipsAvailable[index].info,
                           cmShipsAvailable[index].nJobs);
            if (cmShipsAvailable[index].info->shipclass <= CLASS_Frigate)
            {
                if (capShipBuilt)
                {
                    if (capShipType != cmShipsAvailable[index].info->shiptype)
                    {
                        mixedCapShipTypes = TRUE;
                    }
                }
                else
                {
                    capShipType = cmShipsAvailable[index].info->shiptype;
                    capShipBuilt = TRUE;
                }
            }
        }
    }

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
        cmShipsAvailable[index].nJobs = 0;                    //no jobs queued yet
    }

    if (builtSomething)
    {
        if (!capShipBuilt || mixedCapShipTypes)
        {
            speechEventQueue(NULL, 67113013, 0, -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
        }
        else
        {
            speechEventQueue(NULL, 67113022, capShipType, -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
        }
    }

    if (cmShipInfoRegion != NULL)
    {
        cmShipInfoRegion->status |= RSF_DrawThisFrame;
    }
    if (cmNumberRUsRegion != NULL)
    {
        cmNumberRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalRUsRegion != NULL)
    {
        cmTotalRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalShipsRegion != NULL)
    {
        cmTotalShipsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipCostsRegion != NULL)
    {
        cmShipCostsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipNumbersRegion != NULL)
    {
        cmShipNumbersRegion->status |= RSF_DrawThisFrame;
    }
    if (cmMotherShipRegion != NULL)
    {
        cmMotherShipRegion->status |= RSF_DrawThisFrame;
    }
    for (index = 0; index < NUM_CMCARRIERS; index++)
    {
        if (cmCarrierRegions[index] != NULL)
        {
            cmCarrierRegions[index]->status |= RSF_DrawThisFrame;
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : cmCancelShipType
    Description : Cancels all jobs that refer to a certain shiptype
    Inputs      : type - the shiptype to cancel
    Outputs     : Cancels the build requests of a shiptype
    Return      : void
----------------------------------------------------------------------------*/
void cmForceBuildShipType(ShipType type)
{
//    udword i;

    curshipsInProgress = cmFindFactory(universe.curPlayerPtr->PlayerMothership);

    curshipsInProgress->progress[type].timeLeft = 0;
}


/*-----------------------------------------------------------------------------
    Name        : cmCancelJobs
    Description : Callback to cancel all current jobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmCancelJobs(char *string, featom *atom)
{
    sdword index, jobCount;
    shipinprogress *progress;
    sdword refund/*,totalSpent*/;
    bool32   canceled=FALSE;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildCancelJobs)
        return;

    progress = &curshipsInProgress->progress[0];

    for (index = jobCount = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (progress[cmShipsAvailable[index].type].selected) )
        {
            cmShipsAvailable[index].nJobs = 0;
            canceled = TRUE;

            if (multiPlayerGame || (multiPlayerGameUnderWay && playPackets))
            {
                if (cmShipsAvailable[index].info)
                {
                    //xxx
                    clWrapDeterministicBuild(CMD_CANCEL, &universe.mainCommandLayer,
                                             0, /*numShips == as.many.as.there.are*/
                                             cmShipsAvailable[index].type,
                                             cmShipsAvailable[index].info->shiprace,
                                             universe.curPlayerIndex,
                                             curshipsInProgress->ship);
                }
            }
        }
    }
    //speechEventFleet(COMM_F_Const_CancelAll, 0, universe.curPlayerIndex);

    for (index = 0; index < TOTAL_NUM_SHIPS; index++, progress++)
    {                                                   //for all classes of ships
        if (progress->selected)
        {
            if (progress->nJobsTotal != 0)
            {
                refund = progress->costSoFar;
                refund = (sdword)((real32)refund * cmFloatRefundRatio);
//yyy
/*
                clWrapRUTransfer(&universe.mainCommandLayer,
                                 0,
                                 universe.curPlayerPtr->playerIndex,
                                 refund, RUTRANS_BUILDCANCEL);
*/
                if (!multiPlayerGame)
                {
                    //xxx
                    universe.curPlayerPtr->resourceUnits += refund;
                }
            }
            progress->timeFraction = 0; // No fixed point fraction bits (used for decrementing RUs)
            if (progress->info != NULL &&
                LOBYTE(progress->info->externalBuild) == 1 &&
                progress->cataSaveField1 == 1)
            {
                if (progress->cataSaveField2 != 0)
                {
                    clWrapExternalConstruction(&universe.mainCommandLayer,
                                               universe.curPlayerIndex,
                                               curshipsInProgress->ship,
                                               (void *)progress->cataSaveField2,
                                               0.0f,
                                               1);
                }
                progress->cataSaveField0 = 0;
                progress->cataSaveField1 = 0;
                progress->cataSaveField2 = 0;
            }
            progress->timeStart = progress->timeLeft = 0;   //job all done
            progress->nJobsTotal = progress->nJobs = 0;     //cancel building jobs
            progress->costSoFar = 0;
            canceled = TRUE;

            if (multiPlayerGame || (multiPlayerGameUnderWay && playPackets))
            {
                if (progress->info)
                {
                    //xxx
                    clWrapDeterministicBuild(CMD_CANCEL, &universe.mainCommandLayer,
                                             0, /*numShips == as.many.as.there.are*/
                                             progress->info->shiptype,
                                             progress->info->shiprace,
                                             universe.curPlayerIndex,
                                             curshipsInProgress->ship);
                }
            }
        }
    }

    if (canceled == TRUE) speechEventFleet(COMM_F_Const_CancelBatch, 0, universe.curPlayerIndex);

    cmDirtyShipInfo();
}


/*-----------------------------------------------------------------------------
    Name        : cmPauseJobs
    Description : Callback to pause/unpause all current jobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmPauseJobs(char *string, featom *atom)
{
    sdword index;
    shipinprogress *progress;
    bool32 allpaused = TRUE, changed=FALSE;

    //speechEventFleet(COMM_F_Const_CancelAll, 0, universe.curPlayerIndex);   // later change to ConstructionCancel

    if((tutorial==TUTORIAL_ONLY) & !tutEnable.bBuildPauseJobs)
        return;

    //all paused?
    progress = &curshipsInProgress->progress[0];
    for (index = 0; (index < TOTAL_NUM_SHIPS) && allpaused; index++, progress++)
    {                                                   //for all classes of ships
        if (progress->selected)
        {
            if (!progress->paused)
            {
                allpaused = FALSE;
            }
        }
    }

    //set all jobs to opposite of allpaused
    progress = &curshipsInProgress->progress[0];
    for (index = 0; index < TOTAL_NUM_SHIPS; index++, progress++)
    {                                                   //for all classes of ships
        if (progress->selected)
        {
            udword command;

            progress->paused = !allpaused;
            if (progress->nJobs>0) changed = TRUE;

            command = progress->paused ? CMD_PAUSE : CMD_UNPAUSE;
            if (multiPlayerGame)
            {
                if (progress->info)
                {
                    //xxx
                    clWrapDeterministicBuild(command, &universe.mainCommandLayer,
                                             0, /*numShips doesn't matter here*/
                                             progress->info->shiptype,
                                             progress->info->shiprace,
                                             universe.curPlayerIndex,
                                             curshipsInProgress->ship);
                }
            }
        }
    }

    if (changed)
    {
        if (!allpaused)
        {
            speechEventFleet(COMM_F_Const_Pause, 0, universe.curPlayerIndex);
        }
        else
        {
            speechEventFleet(COMM_F_Const_Resume, 0, universe.curPlayerIndex);
        }
    }
    if (cmShipInfoRegion != NULL)
    {
        cmShipInfoRegion->status |= RSF_DrawThisFrame;
    }
    if (cmNumberRUsRegion != NULL)
    {
        cmNumberRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalRUsRegion != NULL)
    {
        cmTotalRUsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmTotalShipsRegion != NULL)
    {
        cmTotalShipsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipCostsRegion != NULL)
    {
        cmShipCostsRegion->status |= RSF_DrawThisFrame;
    }
    if (cmShipNumbersRegion != NULL)
    {
        cmShipNumbersRegion->status |= RSF_DrawThisFrame;
    }
    if (cmMotherShipRegion != NULL)
    {
        cmMotherShipRegion->status |= RSF_DrawThisFrame;
    }
    for (index = 0; index < NUM_CMCARRIERS; index++)
    {
        if (cmCarrierRegions[index] != NULL)
        {
            cmCarrierRegions[index]->status |= RSF_DrawThisFrame;
        }
    }
}

#if(0)
/*-----------------------------------------------------------------------------
    Name        : cmPauseJobs
    Description : Callback to pause/unpause all current jobs
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmPauseJobsOld(char *string, featom *atom)
{
    udword index;
    bool32 allpaused = TRUE;
    shipinprogress *sip;

    if((tutorial==TUTORIAL_ONLY) & !tutEnable.bBuildPauseJobs)
        return;

    cmDirtyShipInfo();

    /*
    if (bitTest(taskData[cmBuildTask]->flags, TF_Paused))
    {
        speechEventFleet(COMM_F_Const_Resume, 0, universe.curPlayerIndex);
        taskResume(cmBuildTask);
    }
    else
    {
        speechEventFleet(COMM_F_Const_Pause, 0, universe.curPlayerIndex);
        taskPause(cmBuildTask);
    }
    */

    //check if all selected ships paused

    for (index=0; (cmShipsAvailable[index].nJobs!=-1) && allpaused; index++)
    {
        //if (cmShipsAvailable[index].selected)
        sip = cmSIP(index);

        if (sip->selected)
        {
            if (!sip->paused)
            {
                allpaused = FALSE;
            }
        }
    }

    if (allpaused)  //unpause them all
    {
        for (index=0; (cmShipsAvailable[index].nJobs!=-1) && allpaused; index++)
        {
            sip = cmSIP(index);
            if (sip->selected)
            {
                sip->paused = FALSE;
            }
        }

    }
    else    //pause them all
    {
        for (index=0; (cmShipsAvailable[index].nJobs!=-1) && allpaused; index++)
        {
            sip = cmSIP(index);
            if (sip->selected)
            {
                sip->paused = TRUE;
            }        }

    }
}
#endif

/*-----------------------------------------------------------------------------
    Name        : cmShipCostsDraw
    Description : Draw the cost of building selected ships.
    Inputs      : feflow callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void cmShipCostsDraw(featom *atom, regionhandle region)
{
    sdword x, y, index;
    rectangle *rect = &region->rect;
    bool32        newline=FALSE;
    color c;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    sdword  numlines,startind=0;

    currentFont = fontCurrentGet();
    fontMakeCurrent(cmDefaultFont);

    feStaticRectangleDraw(region);                          //draw standard rectangle
    primRectSolid2(rect, CM_BackgroundColor);
    primLine2(rect->x0, rect->y0, rect->x0, rect->y1, colRGB(0, 100, 160));

    cmShipCostsRegion = region;

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if (cmShipsAvailable[index].itemstat == STAT_INACTIVE)
            {
                c = FEC_ListItemInactive;
            }
            else if (cmShipsAvailable[index].nJobs > 0)
            {                                                   //if this ship already selected
                c = cmSelectionTextColor;
            }
            else
            {
                c = cmStandardTextColor;
            }
            if (progress->nJobs > 0)
            {                                                   //if working on this class
                c = cmBuildingTextColor;
            }

#if CM_CHEAP_SHIPS
            if (cmCheapShips && !multiPlayerGame)
            {
                x = rect->x1 - CM_ASMarginLeft - fontWidth("1");
                fontPrint(x, y, c, "1");
            }
            else
#endif
            {
                x = rect->x1 - CM_ASMarginLeft - fontWidthf("%d", cmShipsAvailable[index].info->buildCost);
                fontPrintf(x, y, c, "%d", cmShipsAvailable[index].info->buildCost);
            }

            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }


        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

}

/*-----------------------------------------------------------------------------
    Name        : cmSelectShipType
    Description : Find what type of ship was selected
    Inputs      : reg - region clicked on
                  yClicked - vertical location clicked on.
    Outputs     :
    Return      : type of ship clicked on or -1 if none
----------------------------------------------------------------------------*/
sdword cmSelectShipType(regionhandle region, sdword yClicked)
{
    sdword index, y, numlines, startind;
    fonthandle currentFont;
    bool32 newline = FALSE;

    currentFont = fontMakeCurrent(cmDefaultFont);

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
        {
            if (cmShipsAvailable[index].itemstat != STAT_CANTBUILD)
                newline=TRUE;
        }
        else if (cmShipsAvailable[index].itemtype==ITEM_CLASS)
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    y = region->rect.y0 + CM_ASMarginTop;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
        {
            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }

        if ((yClicked < y + fontHeight(" ") + CM_ASInterSpacing)&&(newline))
        {
            if ((cmShipsAvailable[index].itemtype != ITEM_CLASS)&&
                (cmShipsAvailable[index].itemstat != STAT_CANTBUILD))
            {
                if( (tutorial==TUTORIAL_ONLY) && tutIsBuildShipRestricted((sdword)cmShipsAvailable[index].type) )
                    return(-1);
                else
                    return(index);
            }
            else
                return(-1);
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

    return (-1);
}

/*-----------------------------------------------------------------------------
    Name        : cmClassCapClassRemap
    Description : Get the build manager class for a given ship
    Inputs      : shipType - type of a ship
                  shipClass - class of said ship
    Outputs     :
    Return      : build manager class
----------------------------------------------------------------------------*/
ShipClass cmClassCapClassRemap(ShipType shipType, ShipClass shipClass)
{
    if (ShipTypeIsProcessorType(shipType))
    {                                                       //resource controller is a special case
        return(CLASS_Resource);
    }
    return(cmClassCapRemap[shipClass]);
}

/*-----------------------------------------------------------------------------
    Name        : cmClassCapCanCreateShip
    Description : Can we build a ship of this class or are all the berths in use?
    Inputs      : shipType - what type of ship we want to build.
                  factory - where we want to build it from.
                  cmShipsAvail - what ships are in the works.
    Outputs     :
    Return      : TRUE if we can build it, false otherwise
----------------------------------------------------------------------------*/
bool32 cmClassCapCanCreateShip(ShipType shipType, shipsinprogress *factory, shipavailable *cmShipsAvail)
{
    ShipStaticInfo *shipStaticInfo;
    ShipClass shipClass;
    ShipRace shipRace;
    sdword index, nInThisClass;
    bool32 shipsQueuedUp[TOTAL_NUM_SHIPS];

    shipStaticInfo = GetShipStaticInfo(shipType, factory->ship->shiprace);//get static info
    shipClass = cmClassCapClassRemap(shipType, shipStaticInfo->shipclass); //get ship class
    shipRace = factory->ship->playerowner->race;            //get race of player who owns ship, not race of ship

    //clear out list of queued ships
    memset(shipsQueuedUp, 0, TOTAL_NUM_SHIPS * sizeof(bool32));

    //find all ships of same class queued up for construction
    for (index = nInThisClass = 0; cmShipsAvail[index].nJobs != -1; index++)
    {
        shipStaticInfo = GetShipStaticInfo(cmShipsAvail[index].type, shipRace);
        if (cmShipsAvail[index].nJobs > 0)
        {                                                   //if this ship being built
            if (cmClassCapClassRemap(shipStaticInfo->shiptype, shipStaticInfo->shipclass) == shipClass)
            {                                               //if this job the same class as the one we want to build
                shipsQueuedUp[shipStaticInfo->shiptype] = TRUE;
                if (shipStaticInfo->shiptype != shipType)
                {                                           //don't count this one if it's the one we're clicking on
                    nInThisClass++;
                }
            }
        }
    }
    //find all ships of same class already under construction in this factory
    for (index = 0; index < TOTAL_NUM_SHIPS; index++)
    {
        shipStaticInfo = GetShipStaticInfo(index, shipRace);
        if (factory->progress[index].nJobs > 0)
        {
            if (cmClassCapClassRemap(shipStaticInfo->shiptype, shipStaticInfo->shipclass) == shipClass)
            {                                               //if this job the same class as the one we want to build
                if (!shipsQueuedUp[index])
                {
                    shipsQueuedUp[index] = TRUE;
                    if (shipStaticInfo->shiptype != shipType)
                    {                                       //don't count this one if it's the one we're clicking on
                        nInThisClass++;
                    }
                }
            }
        }
    }
    //find all ships not yet started due to network lag.  This does not seem to know of it's factory.
    //...or would that be covered by the first check?
    for (index = 0; index < TOTAL_NUM_SHIPS; index++)
    {
        shipStaticInfo = GetShipStaticInfo(index, shipRace);
        if (shiplagtotals[index] > 0)
        {
            if (cmClassCapClassRemap(shipStaticInfo->shiptype, shipStaticInfo->shipclass) == shipClass)
            {                                               //if this job the same class as the one we want to build
                if (!shipsQueuedUp[index])
                {
                    shipsQueuedUp[index] = TRUE;
                    if (shipStaticInfo->shiptype != shipType)
                    {                                       //don't count this one if it's the one we're clicking on
                        nInThisClass++;
                    }
                }
            }
        }
    }

    return(nInThisClass < cmMaxJobsPerClass[shipClass]);
}

bool32 cmIncrement(shipsinprogress *factory, uword index)
{
    shipinprogress *progress;
    Player *player;
    ShipStaticInfo *shipStaticInfo;
    bool32 flag = FALSE;
    bool32 canCreateByUnitCap;
    bool32 supportGatePass;
    sdword requiredSupportUnits;
    sdword queuedSupportUnits;
    sdword availableSupportUnits;

    progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
    player = factory->ship->playerowner;
    shipStaticInfo = GetShipStaticInfo(cmShipsAvailable[index].type, player->race);
    requiredSupportUnits = (sdword)ShipStaticInfoGetSupportUnits(shipStaticInfo);
    queuedSupportUnits = cmCurrentSupportUnitsUsed(player);
    availableSupportUnits = player->supportUnitsMax - player->supportUnitsUsed;
    canCreateByUnitCap = unitCapCanCreateShip((uword)cmShipsAvailable[index].info->shiptype, factory, cmShipsAvailable);
    supportGatePass = (((player->playerIndex != 0) && singlePlayerGame) ||
                       (requiredSupportUnits == 0) ||
                       ((player->supportUnitsUsed < player->supportUnitsMax) &&
                        (availableSupportUnits >= (requiredSupportUnits + queuedSupportUnits))));

    if (canCreateByUnitCap &&
        supportGatePass)
    {
        flag = TRUE;
        if (progress->nJobsTotal > 0)
        {
            progress->nJobsTotal++;
            progress->nJobs++;

            if (multiPlayerGame || (multiPlayerGameUnderWay && playPackets))
            {
                clWrapDeterministicBuild(CMD_START, &universe.mainCommandLayer,
                                         1,
                                         progress->info->shiptype,
                                         universe.curPlayerPtr->race,
                                         universe.curPlayerIndex,
                                         factory->ship);
            }
        }
        else
        {
            cmShipsAvailable[index].nJobs++;
        }
    }
    else
    {
        ShipNumberExceeded = (uword)index;
        ExceededCapsFrames = taskTimeElapsed;
    }
    svSelectShip(cmShipsAvailable[index].info->shiptype);
    curshipview = cmShipsAvailable[index].info->shiptype;
    cmUpdateShipAvailable();

    return flag;
}



bool32 cmDecrement(shipsinprogress *factory, uword index)
{
    shipinprogress *progress;
    bool32 flag = TRUE, skip = FALSE;

    //ctrl = add 5?

    progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
    //inactive jobs
    if (cmShipsAvailable[index].nJobs > 0)
    {
        cmShipsAvailable[index].nJobs--;
        skip = TRUE;
    }
    //active jobs
    if (!skip && progress->nJobs > 1)
    {
        progress->nJobs--;
        progress->nJobsTotal--;
        dbgMessagef("\n!! removed a job, jobs %d total %d !!",
                    progress->nJobs, progress->nJobsTotal);

        if (multiPlayerGame || (multiPlayerGameUnderWay && playPackets))
        {
            clWrapDeterministicBuild(CMD_START, &universe.mainCommandLayer,
                                     1 | 0x4000,
                                     progress->info->shiptype,
                                     universe.curPlayerPtr->race,
                                     universe.curPlayerIndex,
                                     factory->ship);
        }
    }
    svSelectShip(cmShipsAvailable[index].info->shiptype);
    curshipview = cmShipsAvailable[index].info->shiptype;
    cmUpdateShipAvailable();

    return flag;
}



/*-----------------------------------------------------------------------------
    Name        : cmRightArrowProcess
    Description :
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword cmRightArrowProcess(regionhandle region, sdword ID, udword event, udword data)
{
    shipsinprogress *factory;
    //shipinprogress *progress;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    if (event == RPE_PressLeft)
    {
        mgAccTime = taskTimeElapsed - CM_WAIT_ACC;
        return (0);
    }

    if (event == RPE_ReleaseLeft)
    {
        cmArrowIndex = -1;
//        mgAcceleratorRelease();
        return (0);
    }
    //if(!mouseInRect(&region->rect))
    //{
    //    return (0);
    //}

    cmArrowIndex = cmSelectShipType(region, mouseCursorY());

    factory = curshipsInProgress;

    if ( (event == RPE_HoldLeft) && (cmArrowIndex!=-1)  && mgAccelerator() )
    {
        if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildArrows)
            return(0);

        if( (tutorial==TUTORIAL_ONLY) && tutIsBuildShipRestricted(cmShipsAvailable[cmArrowIndex].info->shiptype) )
            return(0);

        if (keyIsHit(CM_FIVEKEY))
        {
            uword j;
            for (j=0; j<5; j++)
            {
                if (cmIncrement(factory,(uword)cmArrowIndex))
                    cmRightArrowActive = TRUE;
                else
                    break;
            }
        }
        else
        {
            if (cmIncrement(factory,(uword)cmArrowIndex))
                cmRightArrowActive = TRUE;
        }

    }
    else if (event == RPE_WheelUp)
    {
        cmWheelNegative();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    else if (event == RPE_WheelDown)
    {
        cmWheelPositive();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    cmDirtyShipInfo();
    return(0);
}


/*-----------------------------------------------------------------------------
    Name        : cmLeftArrowProcess
    Description :
    Inputs      : standard region callback
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword cmLeftArrowProcess(regionhandle region, sdword ID, udword event, udword data)
{
    shipsinprogress *factory;
    //shipinprogress *progress;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    if (event == RPE_PressLeft)
    {
        mgAccTime = taskTimeElapsed - CM_WAIT_ACC;
        return (0);
    }

    if (event == RPE_ReleaseLeft)
    {
        cmArrowIndex = -1;
//        mgAcceleratorRelease();
        return (0);
    }
    //if(!mouseInRect(&region->rect))
    //{
    //    return (0);
    //}

    cmArrowIndex = cmSelectShipType(region, mouseCursorY());

    factory = curshipsInProgress;

    if ( (event == RPE_HoldLeft) && (cmArrowIndex!=-1) && mgAccelerator())
    {
        bool8 skip = FALSE;

        if ((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildArrows)
            return(0);

        cmLeftArrowActive = TRUE;


        if (keyIsHit(CM_FIVEKEY))
        {
            uword j;
            for (j=0; j<5; j++)
            {
                if (cmDecrement(factory,(uword)cmArrowIndex))
                    cmLeftArrowActive = TRUE;
            }
        }
        else
        {
            if (cmDecrement(factory,(uword)cmArrowIndex))
                cmLeftArrowActive = TRUE;
        }
    }
    else if (event == RPE_WheelUp)
    {
        cmWheelNegative();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    else if (event == RPE_WheelDown)
    {
        cmWheelPositive();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    cmDirtyShipInfo();
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : cmDrawArrow
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void new_cmDrawArrow(regionhandle region, bool32 leftArrow)
{
    sdword y, index, numlines, startind=0;
    bool32 newline = FALSE;
    rectangle *rect = &region->rect;
    rectangle arrowrect;

    triangle tri;
    sdword width = rect->x1 - rect->x0 - 4;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    sdword leftoffset = 0;

    numlines = 0;

    currentFont = fontMakeCurrent(cmDefaultFont);

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    #define YOFFSET 200

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    if(leftArrow)
    {
        tri.x0 = rect->x1 - 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }
    else
    {
        tri.x0 = rect->x0 + 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if(leftArrow)
            {
                tri.y0 = YOFFSET + y;
                tri.y1 = YOFFSET + y + (width / 2);
                tri.y2 = YOFFSET + y + width;
            }
            else
            {
                tri.y0 = YOFFSET + y;
                tri.y1 = YOFFSET + y + width;
                tri.y2 = YOFFSET + y + (width / 2);
            }

            if(((leftArrow && cmLeftArrowActive) || (!leftArrow && cmRightArrowActive)) && (index==cmArrowIndex))
            {
                cmLeftArrowActive  = FALSE;
                cmRightArrowActive = FALSE;
                cmArrowIndex = -1;
                primTriSolid2(&tri, colRGB(200, 0, 0));
            }


            primTriOutline2(&tri, 1, colRGB(200, 200, 0));

            if (leftArrow)
                leftoffset = 3;
            else
                leftoffset = 0;


            arrowrect.x0 = rect->x0;
            arrowrect.x1 = rect->x0 + 20;
            arrowrect.y0 = y;
            arrowrect.y1 = y + 20;

            //trPalettedTextureMakeCurrent(cmArrowTexture[0 + leftoffset], cmArrowIcon[0 + leftoffset]->palette);
            trRGBTextureMakeCurrent(cmArrowTexture[0 + leftoffset]);

            rndPerspectiveCorrection(FALSE);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            primRectSolidTextured2(&arrowrect);






            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }
    fontMakeCurrent(currentFont);
}



/*-----------------------------------------------------------------------------
    Name        : cmDrawArrow
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmDrawArrow(regionhandle region, bool32 leftArrow)
{
    sdword y, index, numlines, startind=0;
    bool32 newline = FALSE;
    rectangle *rect = &region->rect;
    triangle tri;
    sdword width = rect->x1 - rect->x0 - 4;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;

    numlines = 0;

    currentFont = fontMakeCurrent(cmDefaultFont);

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    if(leftArrow)
    {
        tri.x0 = rect->x1 - 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }
    else
    {
        tri.x0 = rect->x0 + 2;
        tri.x1 = rect->x0 + 2;
        tri.x2 = rect->x1 - 2;
    }

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if(leftArrow)
            {
                tri.y0 = y;
                tri.y1 = y + (width / 2);
                tri.y2 = y + width;
            }
            else
            {
                tri.y0 = y;
                tri.y1 = y + width;
                tri.y2 = y + (width / 2);
            }

            if(((leftArrow && cmLeftArrowActive) || (!leftArrow && cmRightArrowActive)) && (index==cmArrowIndex))
            {
                cmLeftArrowActive  = FALSE;
                cmRightArrowActive = FALSE;
                cmArrowIndex = -1;
                primTriSolid2(&tri, colRGB(200, 0, 0));
            }

            primTriOutline2(&tri, 1, colRGB(200, 200, 0));

            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }
    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : cmLeftArrowDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmLeftArrowDraw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)cmLeftArrowProcess);         //set new region handler function
        regSiblingMoveToFront(region);
    }
    cmDrawArrow(region, TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : cmRightArrowDraw
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmRightArrowDraw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_ReleaseLeft | RPE_HoldLeft |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)cmRightArrowProcess);        //set new region handler function
        regSiblingMoveToFront(region);
    }
    cmDrawArrow(region, FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : cmShipNumbersDraw
    Description : Draw the number of ships being built.
    Inputs      : feflow callback
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void cmShipNumbersDraw(featom *atom, regionhandle region)
{
    sdword x, y, index, numShips;
    rectangle *rect = &region->rect;
    bool32 newline = FALSE;
    color c;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    sdword  numlines,startind=0;

    currentFont = fontCurrentGet();
    fontMakeCurrent(cmDefaultFont);

    //if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    //{                                                       //if region not processed yet
    //    region->flags = RPE_PressLeft | RPE_PressRight |
    //                    RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
    //    regFunctionSet(region, cmSelectNumber);             //set new region handler function
    //}
    feStaticRectangleDraw(region);                          //draw standard rectangle
    primRectSolid2(rect, CM_BackgroundColor);
    primLine2(rect->x0, rect->y0, rect->x0, rect->y1, colRGB(0, 100, 160));

    cmShipNumbersRegion = region;

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    newline=FALSE;
    numlines=0;

    y = rect->y0 + CM_ASMarginTop;
    factory = curshipsInProgress;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= rect->y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

            if (cmShipsAvailable[index].itemstat == STAT_INACTIVE)
            {
                c = FEC_ListItemInactive;
            }
            else if (cmShipsAvailable[index].nJobs > 0)
            {                                                   //if this ship already selected
                c = cmSelectionTextColor;
            }
            else
            {
                c = cmStandardTextColor;
            }
            if (progress->nJobs > 0)
            {                                                   //if working on this class
                c = cmBuildingTextColor;
            }

            numShips = cmShipsAvailable[index].nJobs;
            numShips += progress->nJobs;  //active jobs, too

            x = rect->x1 - CM_ASMarginLeft - fontWidthf("%d", numShips) - 5;
            fontPrintf(x, y, c, "%d", numShips);

            newline=TRUE;
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            newline=TRUE;
        }


        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
        }
    }

    fontMakeCurrent(currentFont);

}


/*-----------------------------------------------------------------------------
    Name        : cmSelectAvailable
    Description : Callback for selecting/deselecting ships
    Inputs      : standard region callback
    Outputs     : selects or deselects a ship for construction
    Return      :
----------------------------------------------------------------------------*/
sdword cmSelectAvailable(regionhandle region, sdword ID, udword event, udword data)
{
    sdword index;
    shipsinprogress *factory;
    shipinprogress *progress, *kprogress;

    if (multiPlayerGame)
    {
        if (!multiPlayerGameUnderWay)
        {
            return (0);
        }
    }

    index = cmSelectShipType(region, mouseCursorY());

    factory = curshipsInProgress;

    if ( (event == RPE_PressLeft || event == RPE_DoubleLeft) && (index!=-1) )
    {
        progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

        if (progress->selected && !keyIsHit(SHIFTKEY))
        {
                        //left press (select/add job)
            if((tutorial==TUTORIAL_ONLY) && tutIsBuildShipRestricted(cmShipsAvailable[index].info->shiptype) )
                return(0);

            if (keyIsHit(CM_FIVEKEY))
            {
                uword j;
                for (j=0; j<5; j++)
                {
                    if (cmIncrement(factory,(uword)index))
                        cmRightArrowActive = TRUE;
                }
            }
            else
            {
                if (cmIncrement(factory,(uword)index))
                    cmRightArrowActive = TRUE;
            }
        }
        else
        {
            if (keyIsHit(SHIFTKEY))
            {
                progress->selected = !progress->selected;
            }
            else
            {
                udword i;

                kprogress = progress;
                progress = &curshipsInProgress->progress[0];
                for (i = 0; i < TOTAL_NUM_SHIPS; i++, progress++)
                {                                                   //for all classes of ships
                    progress->selected = FALSE;
                }

                kprogress->selected = TRUE;

                /*
                for (i=0; cmShipsAvailable[i].nJobs!=-1; i++)
                {

                    sip = cmSIP(index);
                    if (i == index)
                        sip->selected = TRUE;
                    else
                        sip->selected = FALSE;*/
            }
        }

        svSelectShip(cmShipsAvailable[index].info->shiptype);
        curshipview = cmShipsAvailable[index].info->shiptype;
    }
    else if ( (event == RPE_PressRight || event == RPE_DoubleRight) && (index!=-1) )
    {
        progress = &factory->progress[cmShipsAvailable[index].info->shiptype];

        //cmDecrement(factory, (uword)index);
        if (keyIsHit(CM_FIVEKEY))
        {
            uword j;
            for (j=0; j<5; j++)
            {
                if (cmDecrement(factory,(uword)index))
                    cmLeftArrowActive = TRUE;
            }
        }
        else
        {
            if (cmDecrement(factory,(uword)index))
                cmLeftArrowActive = TRUE;

            if (keyIsHit(SHIFTKEY))
            {
                progress->selected = !progress->selected;
            }
            else
            {
                udword i;

                kprogress = progress;
                progress = &curshipsInProgress->progress[0];
                for (i = 0; i < TOTAL_NUM_SHIPS; i++, progress++)
                {                                                   //for all classes of ships
                    progress->selected = FALSE;
                }

                kprogress->selected = TRUE;
            }
        }

        svSelectShip(cmShipsAvailable[index].info->shiptype);
        curshipview = cmShipsAvailable[index].info->shiptype;
    }
    else if (event == RPE_WheelUp)
    {
        cmWheelNegative();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    else if (event == RPE_WheelDown)
    {
        cmWheelPositive();
        scAdjustThumbwheel(g_shandle, cmUpperIndex, cmMaxIndex, (uword)NUMBER_SHIPS_AVAILABLE);
    }
    cmDirtyShipInfo();
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : cmShipInfoDraw
    Description : Draw the available ships and construction progress
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmShipInfoDraw(featom *atom, regionhandle region)
{
    sdword x, y, index, percent;
    rectangle rect = region->rect;
    color c;
    shipsinprogress *factory;
    shipinprogress *progress;
    fonthandle currentFont;
    bool32       newline;
    sdword     numlines, startind;

    if (cmRenderEverythingCounter > 0)
    {
        cmRenderEverythingCounter--;
        feRenderEverything = TRUE;
    }

    cmShipInfoRegion = region;

    currentFont = fontMakeCurrent(cmShipListFont);

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                                       //if region not processed yet
        region->flags = RPE_PressLeft | RPE_PressRight |
                        RPE_WheelUp | RPE_WheelDown;        //receive mouse presses from now on
        regFunctionSet(region, (regionfunction)cmSelectAvailable);          //set new region handler function
        regFilterSet(region, region->flags | RPE_DoubleLeft | RPE_DoubleRight);
    }
    feStaticRectangleDraw(region);                          //draw standard rectangle

    rect.x1 -= CM_ProgressMarginRight;
    primRectSolid2(&rect, CM_BackgroundColor);

    numlines = 0;

    for (index=0; cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
            newline=TRUE;
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
            newline = TRUE;

        if (newline)
        {
            newline = FALSE;
            numlines++;
            if (numlines==cmUpperIndex+1)
            {
                startind = index;
                break;
            }
        }
    }

    y = region->rect.y0 + CM_ASMarginTop;

    newline=FALSE;
    numlines=0;

    factory = curshipsInProgress;

    for (index=startind;cmShipsAvailable[index].nJobs!=-1; index++)
    {
        if (y + fontHeight(" ") >= region->rect.y1)
        {
            break;
        }

        if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
             (cmShipsAvailable[index].itemstat != STAT_CANTBUILD) )
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
/*
            if (cmShipDisabled(cmShipsAvailable[index].type))
            {
                c = FEC_ListItemInactive;
            }
            else
*/
            if (cmShipsAvailable[index].itemstat == STAT_INACTIVE)
            {
                c = FEC_ListItemInactive;
            }
            else if (cmShipsAvailable[index].nJobs > 0)
            {                                                   //if this ship already selected
                c = cmSelectionTextColor;
            }
            else
            {
                if (progress->selected)
                {
                    c = FEC_ListItemSelected;
                }
                else
                {
                    c = cmStandardTextColor;
                }
            }

            if (progress->selected)
            {
                rect.x0 = region->rect.x0 + CM_SelectBoxX0 + cmSelectLeftAdjust;
                rect.y0 = y + CM_SelectBoxY0 + cmSelectTopAdjust;
                rect.x1 = region->rect.x0 + CM_SelectBoxX1 + cmSelectRightAdjust;
                rect.y1 = y + CM_SelectBoxY1 + cmSelectBottomAdjust;

                primRectTranslucent2(&rect, FEC_ListItemTranslucent/*CM_SelectedColorSolid*/);
                primGLRectOutline2(&rect, 1, FEC_ListItemTranslucentBorder/*CM_SelectedColorOutline*/);
            }

            if (progress->nJobs > 0)
            {                                                   //if working on this class
                c = cmBuildingTextColor;

                if (progress->paused)
                    c = CM_PausedColor;

                percent = 100 - progress->timeLeft * 100 / progress->timeStart;
                rect.x0 = region->rect.x0 + 2 + cmSelectLeftAdjust; //increment past border of window
                rect.y0 = y + CM_SelectBoxY0 + cmSelectTopAdjust;   //draw top bar (progress of current ship)
                rect.y1 = y + fontHeight(" ") / 2 + cmSelectTopAdjust;
                rect.x1 = rect.x0 - 1 + cmSelectRightAdjust + percent * (region->rect.x1 -
                    region->rect.x0 - CM_ProgressMarginLeft - CM_ProgressMarginRight) / 100;
                primRectSolid2(&rect, cmProgressShipColor);

                percent = 100 - (progress->timeLeft +
                    (progress->nJobs - 1) * progress->timeStart) * 100 /
                    (progress->timeStart * progress->nJobsTotal);
                rect.y0 = rect.y1;                               //draw bottom bar (progress of total job)
                rect.y1 = y + CM_SelectBoxY1 + cmSelectBottomAdjust - 1;
                rect.x1 = rect.x0 - 1 + cmSelectRightAdjust + percent * (region->rect.x1 -
                    region->rect.x0 - CM_ProgressMarginLeft - CM_ProgressMarginRight) / 100;
                primRectSolid2(&rect, cmProgressTotalColor);
            }

            if ((taskTimeElapsed-ExceededCapsFrames < FLASH_TIMER) && (ShipNumberExceeded==index))
            {
                c = FEC_ListItemInvalid;//cmCapsExceededTextColor;
            }


            /*
            if (progress->paused)
            {
                rect.x0 = region->rect.x0 + CM_SelectBoxX0+2;
                rect.y0 = y + CM_SelectBoxY0+2;
                rect.x1 = region->rect.x0 + CM_SelectBoxX1+2;
                rect.y1 = y + CM_SelectBoxY1+2;

                primRectOutline2(&rect, 1, CM_PausedColor);
            }
              */


            //ship fancy name
            x = region->rect.x0 + CM_ASMarginLeft;
            fontPrint(x, y, c, ShipTypeToNiceStr(cmShipsAvailable[index].info->shiptype));

            newline = TRUE;

            cmDirtyShipInfo();
        }
        else if ( (cmShipsAvailable[index].itemtype==ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_PRINT) )
        {
            char* classString;

            c = cmClassHeadingTextColor;
            if (cmShipsAvailable[index].type == CLASS_Destroyer)
            {
                classString = strGetString(strCLASS_SuperCapital);
            }
            else
            {
                classString = ShipClassToNiceStr(cmShipsAvailable[index].type);
            }
            fontPrintf(region->rect.x0 + CM_ASHeadingMarginLeft, y,
                       c, "%s %s", classString, strGetString(strCLASS_Class));
            //next line
            newline = TRUE;
        }

        if (newline)
        {
            newline = FALSE;

            y+= fontHeight(" ") + CM_ASInterSpacing;
            numlines++;
        }
    }

    fontMakeCurrent(currentFont);
//  cmDirtyShipInfo();
}

/*-----------------------------------------------------------------------------
    Name        : cmNumberRUsDraw
    Description : Callback to draw number of RU's available
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmNumberRUsDraw(featom *atom, regionhandle region)
{
    sdword width;
    fonthandle oldfont;
    rectangle rect = region->rect;

    cmNumberRUsRegion = region;

    oldfont = fontMakeCurrent(cmShipListFont);

    primModeSet2();
    primRectSolid2(&rect, CM_BackgroundColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    width = fontWidthf("%d", universe.curPlayerPtr->resourceUnits);//width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - CM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", universe.curPlayerPtr->resourceUnits);

    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : cmTotalRUsDraw
    Description : Callback to draw total cost of build queue for all ship types
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmTotalRUsDraw(featom *atom, regionhandle region)
{
    sdword width;
    sdword index, RUs;
    shipsinprogress *factory;
    shipinprogress *progress;
    rectangle rect = region->rect;
    fonthandle oldfont;
    real32 RUsAlready;
    Node *node;

    cmTotalRUsRegion = region;

    oldfont = fontMakeCurrent(cmShipListFont);

    factory = curshipsInProgress;

    primModeSet2();
    primRectSolid2(&rect, CM_BackgroundColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    for (index = RUs = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {
        sdword costPerShip;

        if (cmShipsAvailable[index].itemtype == ITEM_CLASS)
        {
            continue;
        }

        progress = &factory->progress[cmShipsAvailable[index].type];

        if (singlePlayerGame)
        {
            costPerShip = cmShipsAvailable[index].info->buildCost / GameDifficultyProductionModifier;
        }
        else
        {
            costPerShip = cmShipsAvailable[index].info->buildCost;
        }

        RUs += cmShipsAvailable[index].nJobs * costPerShip;
        if (progress->nJobs > 0)
        {
            dbgAssert(progress->info != NULL);

            costPerShip = progress->info->buildCost;
            if (singlePlayerGame)
            {
                costPerShip /= GameDifficultyProductionModifier;
            }

            RUs += progress->nJobs * costPerShip;
            if (singlePlayerGame)
            {
                RUsAlready = (real32)(progress->timeStart - progress->timeLeft) * (real32)costPerShip /
                             (real32)(progress->info->buildTime / GameDifficultyBuildTimeModifier);
            }
            else
            {
                RUsAlready = (real32)(progress->timeStart - progress->timeLeft) * (real32)costPerShip /
                             (real32)(progress->info->buildTime);
            }
            RUs -= (sdword)RUsAlready;
        }
    }

    for (node = listofShipsInProgress.head; node != NULL; node = node->next)
    {
        shipsinprogress *otherFactory = listGetStructOfNode(node);

        if (otherFactory->ship != factory->ship &&
            otherFactory->ship->playerowner == universe.curPlayerPtr)
        {
            for (index = 0; index < TOTAL_NUM_SHIPS; index++)
            {
                sdword costPerShip;

                progress = &otherFactory->progress[index];
                if (progress->nJobs <= 0)
                {
                    continue;
                }

                dbgAssert(progress->info != NULL);
                costPerShip = progress->info->buildCost;
                if (singlePlayerGame)
                {
                    costPerShip /= GameDifficultyProductionModifier;
                }

                RUs += progress->nJobs * costPerShip;
                if (singlePlayerGame)
                {
                    RUsAlready = (real32)(progress->timeStart - progress->timeLeft) * (real32)costPerShip /
                                 (real32)(progress->info->buildTime / GameDifficultyBuildTimeModifier);
                }
                else
                {
                    RUsAlready = (real32)(progress->timeStart - progress->timeLeft) * (real32)costPerShip /
                                 (real32)(progress->info->buildTime);
                }
                RUs -= (sdword)RUsAlready;
            }
        }
    }
    width = fontWidthf("%d", RUs);                          //width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - CM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", RUs);
    fontMakeCurrent(oldfont);
}

/*-----------------------------------------------------------------------------
    Name        : cmTotalShipsDraw
    Description : Callback to draw total number of ships queued
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void cmTotalShipsDraw(featom *atom, regionhandle region)
{
    sdword width;
    sdword index, nShips;
    shipsinprogress *factory;
    shipinprogress *progress;
    rectangle rect = region->rect;
    fonthandle oldfont;

    cmTotalShipsRegion = region;

    oldfont = fontMakeCurrent(cmShipListFont);

    factory = curshipsInProgress;

    primModeSet2();
    primRectSolid2(&rect, CM_BackgroundColor);

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x0, MAIN_WindowHeight - rect.y1, rect.x1 - rect.x0, rect.y1 - rect.y0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    for (index = nShips = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {
        if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
        {
            progress = &factory->progress[cmShipsAvailable[index].info->shiptype];
            nShips += cmShipsAvailable[index].nJobs;
            nShips += progress->nJobs;                          //active jobs, too
        }
    }
    width = fontWidthf("%d", nShips);                       //width of number
    feStaticRectangleDraw(region);                          //draw regular rectangle as backdrop
    fontPrintf(region->rect.x1 - width - CM_RUMarginRight,
               (region->rect.y1 - region->rect.y0 - fontHeight(NULL)) / 2 + region->rect.y0,
               atom->borderColor, "%d", nShips);
    fontMakeCurrent(oldfont);
}

sdword cmCurrentSupportUnitsUsed(Player *player)
{
    sdword queuedSupportUnits = 0;
    Node *node;
    sdword index;

    if (player == NULL)
    {
        return 0;
    }

    if (cmShipsAvailable != NULL)
    {
        for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
        {
            ShipStaticInfo *shipStaticInfo;

            if ((cmShipsAvailable[index].itemtype == ITEM_CLASS) || (cmShipsAvailable[index].nJobs <= 0))
            {
                continue;
            }

            shipStaticInfo = GetShipStaticInfo(cmShipsAvailable[index].type, player->race);
            queuedSupportUnits += cmShipsAvailable[index].nJobs * ShipStaticInfoGetSupportUnits(shipStaticInfo);
        }
    }

    //Cataclysm: the committed (post-"Build" click) queue lives in each factory's
    //progress[] array, not in cmDetermProgress (which is only populated for
    //deterministic multiplayer builds). Retail sums nJobs across every factory
    //owned by this player here, matching cmCurrentSupportUnitsUsed @ 0x459c20.
    for (node = listofShipsInProgress.head; node != NULL; node = node->next)
    {
        shipsinprogress *inProgressFactory = listGetStructOfNode(node);
        sdword shipTypeIndex;

        if (inProgressFactory->ship->playerowner != player)
        {
            continue;
        }

        for (shipTypeIndex = 0; shipTypeIndex < TOTAL_NUM_SHIPS; shipTypeIndex++)
        {
            shipinprogress *progress = &inProgressFactory->progress[shipTypeIndex];
            sdword numJobs = progress->nJobs;
            ShipStaticInfo *typeStaticInfo = GetShipStaticInfo((ShipType)shipTypeIndex, player->race);

            if ((numJobs > 0) && progress->info->canReceiveShips && (progress->cataSaveField0 != 0))
            {
                numJobs--;
            }

            queuedSupportUnits += numJobs * ShipStaticInfoGetSupportUnits(typeStaticInfo);
        }
    }

    return queuedSupportUnits;
}

void cmSUBarDraw(featom *atom, regionhandle region)
{
    rectangle rect;
    real32 currentUsage;
    real32 queuedUsage;
    real32 remainingUsage;
    real32 freeRatio;
    real32 barHeight;
    sdword barTop;
    sdword barLeft;
    sdword barRight;

    cmSUBarRegion = region;

    rect = region->rect;
    rect.y1 = region->rect.y0 + (region->rect.y1 - region->rect.y0) / 2 + 1;
    trRGBTextureMakeCurrent(cmSupportTexture[3]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&rect);

    rect = region->rect;
    rect.y0 = region->rect.y0 + (region->rect.y1 - region->rect.y0) / 2 - 1;
    trRGBTextureMakeCurrent(cmSupportTexture[4]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&rect);

    currentUsage = (real32)universe.curPlayerPtr->supportUnitsUsed / (real32)universe.curPlayerPtr->supportUnitsMax;
    currentUsage = max(0.0f, min(1.0f, currentUsage));
    freeRatio = 1.0f - currentUsage;
    barTop = region->rect.y0 + 6;
    barLeft = region->rect.x1 - 20;
    barRight = region->rect.x1 - 4;
    barHeight = (real32)(region->rect.y1 - 4 - (region->rect.y0 + 4) - 14);

    rect.x0 = barLeft;
    rect.y0 = barTop;
    rect.x1 = barRight;
    rect.y1 = barTop + (sdword)(barHeight * freeRatio);
    trRGBTextureMakeCurrent(cmSupportTexture[1]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTexturedFullRectC2(&rect, 0xC0500000);

    rect.y0 = rect.y1 - 1;
    rect.y1 += 7;
    trRGBTextureMakeCurrent(cmSupportTexture[2]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTexturedFullRectC2(&rect, 0xC0500000);

    rect.y0 = region->rect.y0;
    rect.y1 = region->rect.y0 + 7;
    trRGBTextureMakeCurrent(cmSupportTexture[0]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTexturedFullRectC2(&rect, 0xC0500000);

    queuedUsage = (real32)cmCurrentSupportUnitsUsed(universe.curPlayerPtr) / (real32)universe.curPlayerPtr->supportUnitsMax;
    queuedUsage = max(0.0f, min(1.0f, queuedUsage));
    remainingUsage = freeRatio - queuedUsage;
    remainingUsage = max(0.0f, min(1.0f, remainingUsage));

    rect.x0 = barLeft;
    rect.y0 = barTop;
    rect.x1 = barRight;
    rect.y1 = barTop + (sdword)(barHeight * remainingUsage);
    trRGBTextureMakeCurrent(cmSupportTexture[1]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&rect);

    rect.y0 = rect.y1 - 1;
    rect.y1 += 7;
    trRGBTextureMakeCurrent(cmSupportTexture[2]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&rect);

    rect.y0 = region->rect.y0;
    rect.y1 = region->rect.y0 + 7;
    trRGBTextureMakeCurrent(cmSupportTexture[0]);
    rndPerspectiveCorrection(FALSE);
    primRectSolidTextured2(&rect);

    region->status |= RSF_DrawThisFrame;
}

bool32 cmCanBuildShipType(Ship *factoryship,ShipType shiptype,bool32 checkResearch)
{
    ShipStaticInfo *factorystatic;
    ShipStaticInfo *shipstatic;
    shipavailable *shipsavail;
    sdword index;

    if (factoryship == NULL)
    {
        return FALSE;
    }

    factorystatic = factoryship->staticinfo;

    if (factorystatic->shiprace == RACE_Sect)
    {
        if (factorystatic->canBuildBigShips)
        {
            shipsavail = cmShipsAvailableSectBig;
        }
        else
        {
            shipsavail = cmShipsAvailableSectReg;
        }
    }
    else if (factorystatic->shiprace == RACE_Beast)
    {
        if (factorystatic->canBuildBigShips)
        {
            shipsavail = cmShipsAvailableBeastBig;
        }
        else
        {
            shipsavail = cmShipsAvailableBeastReg;
        }
    }
    else if (factorystatic->shiprace == P1)
    {
        shipsavail = cmShipsAvailableP1;
    }
    else if (factorystatic->shiprace == RACE_Taiidan)
    {
        shipsavail = cmShipsAvailableP2;
    }
    else
    {
        dbgAssert(FALSE);           // do we need to deal with Kushan?
        return FALSE;
    }

    for (index = 0; shipsavail[index].nJobs != -1; index++)
    {
        if (shipsavail[index].itemtype != ITEM_CLASS)
        {
            if (shipsavail[index].type == shiptype)
            {
                shipstatic = GetShipStaticInfo(shiptype, factorystatic->shiprace);
                dbgAssert(shipstatic != NULL);

                if (shipstatic->externalBuild)
                {
                    if (factorystatic->shipclass != CLASS_Mothership ||
                        !cmBuildStateChanged(factoryship, shipstatic->shipclass, shiptype, TRUE, 0))
                    {
                        return FALSE;
                    }
                }

                if (!checkResearch)
                {
                    return TRUE;
                }

                {
                    bool32 canResearchBuild = rmCanBuildShip(factoryship->playerowner, shiptype);
                    bool32 countReqPass = ((shipsavail[index].itemtype != ITEM_SHIP_REQUIRES_COUNT) ||
                                           (cmGetShipTypeCount(factoryship->playerowner, shiptype) != 0));
                    if (canResearchBuild && countReqPass)
                    {
                        return TRUE;
                    }
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}

bool32 cmCanSupportShipType(ShipType shiptype, Player *player)
{
    ShipStaticInfo *shipStaticInfo = GetShipStaticInfo(shiptype, player->race);
    sdword queuedSupportUnits = 0;
    sdword availableSupportUnits;
    Node *node;

    if ((player->playerIndex != 0) && singlePlayerGame)
    {
        return TRUE;
    }

    if (ShipStaticInfoGetSupportUnits(shipStaticInfo) == 0)
    {
        return TRUE;
    }

    if (player->supportUnitsUsed >= player->supportUnitsMax)
    {
        return FALSE;
    }

    availableSupportUnits = player->supportUnitsMax - player->supportUnitsUsed;

    for (node = cmDetermProgress.head; node != NULL; node = node->next)
    {
        cmDetermProgress_t *progress = listGetStructOfNode(node);
        sdword numShips;
        ShipStaticInfo *progressStaticInfo;

        if (progress->creator->playerowner != player)
        {
            continue;
        }

        numShips = progress->numShips;
        progressStaticInfo = GetShipStaticInfo(progress->shipType, progress->shipRace);
        if ((numShips > 0) && progressStaticInfo->canReceiveShips && progress->paused)
        {
            numShips--;
        }

        queuedSupportUnits += numShips * ShipStaticInfoGetSupportUnits(progressStaticInfo);
    }

    return (availableSupportUnits >= (queuedSupportUnits + (sdword)ShipStaticInfoGetSupportUnits(shipStaticInfo)));
}

/*-----------------------------------------------------------------------------
    Name        : cmUpdateFactory
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
sdword cmUpdateFactoryNoReset(Ship* shipfactory)
{
    ShipRace race;

    if ( (shipfactory == NULL) || (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips) )
    {
        shipfactory = universe.curPlayerPtr->PlayerMothership;
    }

    if (shipfactory == NULL)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nShipfactory dead.  Can't build anything!");
#endif
        return(0);
    }

    race = shipfactory->playerowner->race;

    if (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nThis ship can't build anything!");
#endif
        return(0);
    }

    //if (bitTest(taskData[cmBuildTask]->flags, TF_Paused))
    //{                                                       //if build task paused
    //    taskResume(cmBuildTask);                            //continue it
    //}

    curshipsInProgress = cmFindFactory(shipfactory);
    dbgAssert(curshipsInProgress != NULL);
    if (race == RACE_Sect)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableSectBig;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableSectReg;
        }
    }
    else if (race == RACE_Beast)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableBeastBig;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableBeastReg;
        }
    }
    else if (race == P1)
    {
        cmShipsAvailable = cmShipsAvailableP1;
    }
    else if (race == RACE_Taiidan)
    {
        cmShipsAvailable = cmShipsAvailableP2;
    }
    else
    {
        dbgAssert(FALSE);
    }

    cmDirtyShipInfo();
    cmUpdateShipAvailable();

    return(1);
}

/*-----------------------------------------------------------------------------
    Name        : cmUpdateShipsAvailable
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmUpdateShipsAvailable(void)
{
    if (cmCurrentSelect==0)
    {
        cmUpdateFactoryNoReset(universe.curPlayerPtr->PlayerMothership);
    }
    else
    {
        cmUpdateFactoryNoReset(cmCarrierX[cmCurrentSelect-1]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmUpdateFactory
    Description :
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
sdword cmUpdateFactory(Ship* shipfactory)
{
    sdword index;
    ShipRace race;
    shipinprogress *sip;

    if ( (shipfactory == NULL) || (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips) )
    {
        shipfactory = universe.curPlayerPtr->PlayerMothership;
    }

    if (shipfactory == NULL)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nShipfactory dead.  Can't build anything!");
#endif
        return(0);
    }

    race = shipfactory->playerowner->race;

    if (!((ShipStaticInfo *)(shipfactory->staticinfo))->canBuildShips)
    {
#if CM_VERBOSE_LEVEL >= 1
        dbgMessagef("\nThis ship can't build anything!");
#endif
        return(0);
    }

    //if (bitTest(taskData[cmBuildTask]->flags, TF_Paused))
    //{                                                       //if build task paused
    //    taskResume(cmBuildTask);                            //continue it
    //}

    curshipsInProgress = cmFindFactory(shipfactory);
    dbgAssert(curshipsInProgress != NULL);
    if (race == RACE_Sect)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableSectBig;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableSectReg;
        }
    }
    else if (race == RACE_Beast)
    {
        if (curshipsInProgress->canBuildBigShips)
        {
            cmShipsAvailable = cmShipsAvailableBeastBig;
        }
        else
        {
            cmShipsAvailable = cmShipsAvailableBeastReg;
        }
    }
    else if (race == P1)
    {
        cmShipsAvailable = cmShipsAvailableP1;
    }
    else if (race == RACE_Taiidan)
    {
        cmShipsAvailable = cmShipsAvailableP2;
    }
    else
    {
        dbgAssert(FALSE);
    }

    for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
    {                                                       //for all available ships
        cmShipsAvailable[index].nJobs = 0;                    //no jobs queued yet

        if (cmShipsAvailable[index].itemtype != ITEM_CLASS)
        {
            cmShipsAvailable[index].info =                        //store the static info
                GetShipStaticInfo(cmShipsAvailable[index].type,
                                  race);
        }
    }


    for (index=0; (cmShipsAvailable[index].nJobs!=-1); index++)
    {
        //if (cmShipsAvailable[index].selected)
        sip = cmSIP(index);

        sip->selected = FALSE;

    }

    cmDirtyShipInfo();
    cmUpdateShipAvailable();

    return(1);
}

/*-----------------------------------------------------------------------------
    Name        : cmConstructionBegin
    Description : Region callback handler for starting the construction manager.
    Inputs      : Standard region callback.
    Outputs     : Starts up the construction manager and returns.
    Return      : void
----------------------------------------------------------------------------*/
sdword cmConstructionBegin(regionhandle region, sdword ID, udword event, udword data)
{
    sdword status = 0;
    ShipPtr bship=NULL;

    if ((playPackets) || (universePause)) return 0;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bBuildManager)
        return 0;

    if (smZoomingIn || smZoomingOut || universe.curPlayerPtr->PlayerMothership == NULL)
    {
        return(status);
    }

    if (singlePlayerGame && universe.curPlayerPtr->playerIndex != 0)
    {
        return(status);
    }

    if (smSensorsActive)
    {
        cmOpenedFromSensors = TRUE;
        smSensorsCloseForGood();
    }
    else
    {
        cmOpenedFromSensors = FALSE;
    }

    cmFillInCarrierXs();

    bship = (Ship *)ID;

    if ((bship != NULL) &&
        (bship->playerowner == NULL ||
         bship->playerowner != universe.curPlayerPtr ||
         bitTest(bship->specialFlags2, SPECIAL_2_DisabledState)))
    {
        bship = NULL;
    }

    if ((bship == NULL) || !ShipTypeIsMothershipOrCarrierType(bship->shiptype))
    {
        if (cmCurrentSelect==0)
        {
            bship = universe.curPlayerPtr->PlayerMothership;
            if (ShipTypeIsCarrierType(universe.curPlayerPtr->PlayerMothership->shiptype))
            {
                cmCurrentSelect = 1;
            }
            else if (ShipTypeIsMothershipType(universe.curPlayerPtr->PlayerMothership->shiptype))
            {
                cmCurrentSelect = 0;
            }
        }
        else if (cmCarrierX[cmCurrentSelect-1]!=NULL)
        {
            bship = cmCarrierX[cmCurrentSelect-1];
        }
        else
        {
            bship = universe.curPlayerPtr->PlayerMothership;
            if (ShipTypeIsCarrierType(universe.curPlayerPtr->PlayerMothership->shiptype))
            {
                cmCurrentSelect = 1;
            }
            else if (ShipTypeIsMothershipType(universe.curPlayerPtr->PlayerMothership->shiptype))
            {
                cmCurrentSelect = 0;
            }
        }
    }
    else if (ShipTypeIsMothershipType(bship->shiptype))
    {
        cmCurrentSelect = 0;
    }
    else
    {
        sdword i;

        for (i=0;i<NUM_CMCARRIERS;i++)
        {
            if (bship == cmCarrierX[i])
            {
                cmCurrentSelect = i+1;
                break;
            }
        }
    }

    if (bship == NULL)
    {
        return status;
    }

    tutGameMessage("Start_BuildManager");

    status = cmUpdateFactory(bship);

    if (!status)
    {
        return(status);
    }

    // disable rendering of main screen
    mrRenderMainScreen = FALSE;

    glcFullscreen(TRUE);

    cmRenderEverythingCounter = (tutorial == TUTORIAL_ONLY) ? 4 : 0;

    // clear the screen
    rndClear();

    // disable taskbar popup window
    tbDisable = TRUE;

    mrManagerScreenBegin(SPLOCKOUT_MR);

    if (piePointSpecMode != PSM_Idle)
    {
        piePointModeOnOff();
    }

    if (!cmScreensHandle || cmScreensRace != universe.curPlayerPtr->race)
    {
        feCallbackAddMultiple(cmCallback);
        feDrawCallbackAddMultiple(cmDrawCallback);
        if (cmScreensHandle != NULL)
        {
            feScreensDelete(cmScreensHandle);
        }
        cmScreensHandle = feScreensLoad(cmFIBFileForRace(universe.curPlayerPtr->race));
        cmScreensRace = universe.curPlayerPtr->race;
    }

    soundEventStopSFX(0.5f);

    /* play the intro sound */
    soundEvent(NULL, UI_ManagerIntro);
    /* play the build manager ambient */
    soundEvent(NULL, UI_BuildManager);

    ioDisable();

    cmBaseRegion = feScreenStart(region,
                                 cmScreenNameForRace(universe.curPlayerPtr->race));//add new regions as siblings of current one

    cmArrowIndex = -1;

    cmLeftArrowActive  = FALSE;
    cmRightArrowActive = FALSE;

    mouseCursorShow();

    opKeyDetour = 0;//2;    //detour keys to cmBuildHotKey
    cmPrintHotKey = FALSE;//TRUE;

    curshipview = DefaultShip;
    updateShipView();

    cmActive = TRUE;

    return(status);
}

/*-----------------------------------------------------------------------------
    Name        : cmBuildJobsAdd
    Description : Create a number of new build jobs on the specified ship
    Inputs      : factory - where the ship is to be built
                  info - information on the ship to build
                  nJobs - number of these ships to build
    Outputs     : adds a new entry into the list of ships in progress
    Return      : void
----------------------------------------------------------------------------*/
void cmBuildJobsAdd(shipsinprogress *factory, ShipStaticInfo *info, sdword nJobs)
{
    shipinprogress *progress;
    sdword prevJobsTotal;
//    sdword index;

#if CM_VERBOSE_LEVEL >= 1
    dbgMessagef("\ncmBuildJobAdd: Building %s", ShipTypeToNiceStr(info->shiptype));
#endif
    dbgAssert((udword)info->shiptype < TOTAL_NUM_SHIPS);                 //make sure we can add a job
    progress = &factory->progress[info->shiptype];         //update the progress
#if CM_CAP_JOBS_PER_CLASS
    if (factory->progress[info->shiptype].nJobs >= CM_NumberJobsPerClass)
    {                                                       //only so much we can do man!
        return;                                             //??? do we need this???
    }
#endif
    if (progress->nJobs == 0)
    {                                                       //if no jobs currently
        progress->timeLeft = progress->timeStart =
            (singlePlayerGame ? (info->buildTime / GameDifficultyBuildTimeModifier) : info->buildTime);
        progress->timeFraction = 0; // no fractional bits in fixed point for RU decrement
        progress->paused = FALSE;
        progress->costSoFar = 0;
        progress->cataSaveField0 = 0;
        progress->cataSaveField1 = 0;
        progress->cataSaveField2 = 0;
    }
    progress->info = info;                                  //init the progress structure
#if CM_CAP_JOBS_PER_CLASS
    progress->nJobs = min(progress->nJobs + nJobs, CM_NumberJobsPerClass);//add the new jobs
#else
    progress->nJobs = progress->nJobs + nJobs;              //add the new jobs
#endif
    prevJobsTotal = progress->nJobsTotal;
    progress->nJobsTotal += nJobs;

    if (multiPlayerGame || (multiPlayerGameUnderWay && playPackets))
    {
        //xxx
        clWrapDeterministicBuild(CMD_START, &universe.mainCommandLayer,
                                 progress->nJobsTotal | 0x8000, info->shiptype,
                                 universe.curPlayerPtr->race,
                                 universe.curPlayerIndex,
                                 factory->ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmSelectMotherShip
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword cmSelectMotherShip(regionhandle region, sdword ID, udword event, udword data)
{
    if ((event == RPE_PressLeft) && (cmCurrentSelect!=0))
    {
        cmCurrentSelect = 0;
        cmUpdateFactory(universe.curPlayerPtr->PlayerMothership);

        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

        updateShipView();
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : cmSelectCarrierX
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword cmSelectCarrierX(regionhandle region, sdword ID, udword event, udword data, udword x)
{
    if ((event == RPE_PressLeft) && (cmCarrierX[x-1] != NULL) && (cmCurrentSelect != x))
    {
        cmCurrentSelect = x;
        cmUpdateFactory(cmCarrierX[x-1]);

        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

        updateShipView();
    }
    return(0);
}

sdword cmSelectCarrier1(regionhandle region, sdword ID, udword event, udword data)
{
    return cmSelectCarrierX(region,ID,event,data,1);
}

sdword cmSelectCarrier2(regionhandle region, sdword ID, udword event, udword data)
{
    return cmSelectCarrierX(region,ID,event,data,2);
}

/*-----------------------------------------------------------------------------
    Name        : cmDrawShipImage
    Description : draws the button image
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmDrawShipImage(regionhandle region, sdword shipID, bool32 active)
{
    rectangle rect;
    udword usetexture;
    sdword centerX;

    usetexture = (shipID == 0) ? 0 : 3;
    if (active)
    {
        usetexture += (shipID == cmCurrentSelect) ? 2 : 1;
    }

    rect.x0 = region->rect.x0 + CM_SHIP_IMAGE_INSET;
    rect.y0 = region->rect.y0 + CM_SHIP_IMAGE_INSET;
    rect.x1 = region->rect.x1 - CM_SHIP_IMAGE_INSET;
    rect.y1 = region->rect.y1 - CM_SHIP_IMAGE_INSET;

    ferDrawImageBorder((sdword *)region, active && shipID == cmCurrentSelect);
    if (active && shipID == cmCurrentSelect && cmFactoryRegion != NULL)
    {
        centerX = rect.x0 + ((rect.x1 - rect.x0) / 2);
        primLineThick2(centerX, cmFactoryRegion->rect.y1, centerX, region->rect.y0, 2, colRGB(255, 255, 255));
        if (cmBaseRegion != NULL)
        {
            bitSet(cmBaseRegion->status, RSF_DrawThisFrame);
        }
    }

    if (glcActive())
    {
        lifheader* lif = cmShipImage[universe.curPlayerPtr->race][usetexture];
        glcRectSolidTexturedScaled2(&rect,
                                    lif->width, lif->height,
                                    lif->data,
                                    cmPaletted ? lif->palette : NULL,
                                    TRUE);
    }
    else
    {
        if (cmPaletted)
        {
            trPalettedTextureMakeCurrent(cmShipTexture[universe.curPlayerPtr->race][usetexture], cmShipImage[universe.curPlayerPtr->race][usetexture]->palette);
        }
        else
        {
            trRGBTextureMakeCurrent(cmShipTexture[universe.curPlayerPtr->race][usetexture]);
        }

        rndPerspectiveCorrection(FALSE);
        primRectSolidTextured2(&rect);
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmMotherShipDraw
    Description : draws the mothership button
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmMotherShipDraw(featom *atom, regionhandle region)
{
    Ship *shipfactory = universe.curPlayerPtr->PlayerMothership;

    cmMotherShipRegion = region;

    if (shipfactory == NULL || shipfactory->staticinfo->shipclass != CLASS_Mothership)
    {
        cmDrawShipImage(region, 0, FALSE);
    }
    else
    {
        if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
        {                                         //if region not processed yet
            region->flags = RPE_PressLeft;        //receive mouse presses from now on
            region->flags |= RPE_DrawEveryFrame;
            regFunctionSet(region, (regionfunction)cmSelectMotherShip); //set new region handler function
        }
        cmDrawShipImage(region, 0, TRUE);
    }
}

void cmFillInCarrierXs(void)
{
    sdword i,insert,index;

    for (i=0;i<NUM_CMCARRIERS;i++)
    {
        cmCarrierX[i] = NULL;
    }

    cmNumPlayersCarriers = 0;

    for (insert=0,index=0;index<cmNumCarriers;index++)
    {
        if (cmCarriers[index].owner == universe.curPlayerPtr)
        {
            if (insert < NUM_CMCARRIERS)
            {
                cmCarrierX[insert++] = cmCarriers[index].ship;

                cmNumPlayersCarriers++;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : cmCarrierXDraw
    Description : draws the carrierX button
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void cmCarrierXDraw(featom *atom, regionhandle region,udword x)
{
    sdword index, count = 0;
    cmCarrierRegions[x-1] = region;
    for (index=0;index<cmNumCarriers;index++)
    {
        if (cmCarriers[index].owner == universe.curPlayerPtr)
        {
            count++;
            if(count > (x-1))
            {
                cmCarrierX[x-1] = cmCarriers[index].ship;
                if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
                {                                         //if region not processed yet
                    region->flags = RPE_PressLeft;        //receive mouse presses from now on
                    region->flags |= RPE_DrawEveryFrame;
                    switch (x)
                    {
                        case 1: regFunctionSet(region, (regionfunction)cmSelectCarrier1); break;
                        case 2: regFunctionSet(region, (regionfunction)cmSelectCarrier2); break;
                        default: break;
                    }
                }
                cmDrawShipImage(region, x, TRUE);
                break;
            }
        }
    }
    if (index == cmNumCarriers)
    {
        cmDrawShipImage(region, x, FALSE);
    }
}

void cmCarrier1Draw(featom *atom, regionhandle region)
{
    cmCarrierXDraw(atom,region,1);
}

void cmCarrier2Draw(featom *atom, regionhandle region)
{
    cmCarrierXDraw(atom,region,2);
}

ShipType cmKeyToShipType(uword key)
{
    uword i;
    ShipType workerType;

    workerType = WorkerShipTypeForRace(universe.curPlayerPtr->race);

    for (i=0; cmShipTypes[i].key!=-1; i++)
    {
        if (cmShipTypes[i].key == key)
        {
            if ((cmShipTypes[i].ship == sWorker || cmShipTypes[i].ship == bWorker) &&
                cmShipTypes[i].ship != workerType)
            {
                continue;
            }
            return (ShipType)cmShipTypes[i].ship;
        }
    }
    return 0;
}


uword cmShipTypeToKey(ShipType ship)
{
    uword i;

    for (i=0; cmShipTypes[i].key!=-1; i++)
    {
        if (cmShipTypes[i].ship == ship)
        {
            return cmShipTypes[i].key;
        }
    }
    return 0;
}


//detoured keypresses:

bool32 cmBuildHotKey(keyindex key, bool32 shift)
{
    uword index;
    uword skey;
    bool32 buildhotkey = FALSE;
    ShipType ship;

    if (((key>=AKEY) && (key<=ZKEY) &&
     (key!=BKEY) &&
     (key!=PKEY) &&
     (key!=CKEY) ) ||
        ((key>=F1KEY) && (key<=F12KEY))
        )
    {
        buildhotkey = TRUE;

    }
    else if ((key>=ONEKEY) && (key<=THREEKEY))
    {
        buildhotkey = TRUE;
    }

    if (buildhotkey)
    {
        ship = 0;
        if (shift)
            skey = key | CM_SHIFT;
        else
            skey = key;

        for (index = 0; cmShipTypes[index].key != -1; index++)
        {
            if (cmShipTypes[index].key == skey)
            {
                ship = cmShipTypes[index].ship;
                break;
            }
        }

        if (ship > 0)
        {
            for (index = 0; cmShipsAvailable[index].nJobs != -1; index++)
            {
                if ( (cmShipsAvailable[index].itemtype != ITEM_CLASS) &&
                  (cmShipsAvailable[index].itemstat==STAT_CANBUILD) )
                {
                    if (cmShipsAvailable[index].type == ship)
                    {
                        if (cmIncrement(curshipsInProgress,index))
                        {
                            cmDecrement(curshipsInProgress,index);
                            //start building now
                            cmBuildJobsAdd(curshipsInProgress,
                              cmShipsAvailable[index].info,
                              1 /*cmShipsAvailable[index].nJobs*/ );

                        }
                        else
                        {
                            //sound effect
                        }
                        break;

                    }

                }
            }
        }
        return TRUE;
    }
    else
        return FALSE;      //don't bypass - it's a standard region key

}

void cmDeterministicStartup(void)
{
    listInit(&cmDetermProgress);
}

void cmDeterministicShutdown(void)
{
    listDeleteAll(&cmDetermProgress);
}

void cmDeterministicReset(void)
{
    listDeleteAll(&cmDetermProgress);
}

crc32 cmDeterministicCRC(void)
{
    Node* node;
    cmDetermProgress_t* block;
    cmDetermProgress_t* ptr;
    cmDetermProgress_t* curNode;
    sdword size;
    crc32 crc;

    if (cmDetermProgress.num == 0)
    {
        return 0;
    }

    size = cmDetermProgress.num * sizeof(cmDetermProgress_t);
    block = (cmDetermProgress_t*)memAlloc(size, "temp dblock", Volatile);
    ptr  = block;
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        curNode = (cmDetermProgress_t*)listGetStructOfNode(node);

        //copy node into block
        memcpy(ptr, curNode, sizeof(cmDetermProgress_t));

        //clear non-deterministic members
        memset(&ptr->node, 0, sizeof(Node));
        ptr->creator = NULL;

        //advance
        ptr++;
        node = node->next;
    }

    crc = crc32Compute((ubyte*)block, size);

    memFree(block);

    return crc;
}

void cmDeterministicBuildDisplay(void)
{
    Node* node;
    cmDetermProgress_t* dprog;
    sdword y, height;

    height = fontHeight(NULL);

    y = 4 * height;
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);

        fontPrintf(
            0, y, colReddish,
            "pl %d, num %d, type %s, race %s, tleft %d, ru %d",
            (sdword)dprog->playerIndex,
            dprog->numShips,
            ShipTypeToStr(dprog->shipType),
            ShipRaceToStr(dprog->shipRace),
            dprog->ticksLeft,
            universe.players[dprog->playerIndex].resourceUnits
            );

        //next line, next node
        y += height;
        if ((y + height) > MAIN_WindowHeight)
        {
            //don't bother w/ off-screen lines
            break;
        }
        node = node->next;
    }
}

bool32 cmDeterministicSanityCheck(cmDetermProgress_t* dprog)
{
    bool32 rval = TRUE;
    Player* player = &universe.players[dprog->playerIndex];

    //check race
    if (dprog->shipRace != player->race)
    {
        rval = FALSE;
    }

    //check type
    if (dprog->shipType < STD_FIRST_SHIP ||
        dprog->shipType > STD_LAST_SHIP)
    {
        rval = FALSE;
    }

    return rval;
}

void cmDeterministicBuildStart(
    sdword numShips, ShipType shipType, ShipRace shipRace, sdword playerIndex, ShipPtr creator)
{
    Node* node;
    cmDetermProgress_t* dprog;
    ShipStaticInfo* info;

    //look for existing match in cmDetermProgress
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        if (dprog->shipType == shipType &&
            dprog->shipRace == shipRace &&
            dprog->playerIndex == playerIndex &&
            dprog->creator == creator)
        {
            if (bitTest(numShips, 0x8000))
            {
                //hi-bit (of uword) set, we're an absolute number
                bitClear(numShips, 0x8000);
                dprog->numShips = numShips;
                if (numShips <= 0)
                {
                    //we've run out of jobs in the batch
                    listDeleteNode(node);
                }
                return;
            }
            else
            {
                //match found, increment count & exit
                if (bitTest(numShips, 0x4000))
                {
                    bitClear(numShips, 0x4000);
                    if (dprog->numShips > 1)
                    {
                        dprog->numShips -= numShips;
                    }
                }
                else
                {
                    dprog->numShips += numShips;
                }
                return;
            }
        }

        //move on
        node = node->next;
    }

    //not found, create new node
    info = GetShipStaticInfo(shipType, shipRace);
    dprog = (cmDetermProgress_t*)memAlloc(sizeof(cmDetermProgress_t), "dprog_t", NonVolatile);
    if (bitTest(numShips, 0x8000))
    {
        bitClear(numShips, 0x8000);
    }
    if (bitTest(numShips, 0x4000))
    {
        //we're subtracting from nothing
        return;
    }
    dprog->numShips = numShips;
    dprog->shipType = shipType;
    dprog->shipRace = shipRace;

    dprog->ticksForOne = (sdword)info->RKOEatMeTime;
    if (singlePlayerGame)
    {
        dprog->ticksForOne /= GameDifficultyBuildTimeModifier;
    }
    dprog->ticksLeft = dprog->ticksForOne;

    if (singlePlayerGame)
    {
        dprog->costPerShip = (sdword)((real32)info->svFirePower / GameDifficultyProductionModifier);
    }
    else
    {
        dprog->costPerShip = info->svFirePower;
    }
    dprog->costPerTick = (real32)dprog->costPerShip / (real32)dprog->ticksForOne;
    dprog->costSoFar = 0.0f;

    dprog->playerIndex = playerIndex;
    dprog->creator = creator;
    dprog->paused = FALSE;
    dprog->extBuildStarted = FALSE;
    dprog->extBuildCreated = FALSE;
    dprog->extBuildShip = NULL;

    {
        shipinprogress *progress = cmFindProgressForFactory(creator, shipType);
        if (progress != NULL)
        {
            progress->cataSaveField1 = 0;
            progress->cataSaveField2 = 0;
        }
    }

    if (cmDeterministicSanityCheck(dprog))
    {
        listAddNode(&cmDetermProgress, &dprog->node, dprog);
    }
}

void cmAdjustRU(cmDetermProgress_t* dprog, sdword amount)
{
    Player* player = &universe.players[dprog->playerIndex];
    player->resourceUnits += amount;

    universe.gameStats.playerStats[player->playerIndex].totalResourceUnitsSpent += min(player->resourceUnits, abs(amount));

    if (player->resourceUnits < 0)
    {
        player->resourceUnits = 0;
    }
}

void cmDeterministicBuildCancel(ShipType shipType, ShipRace shipRace, sdword playerIndex, ShipPtr creator)
{
    Node* node;
    cmDetermProgress_t* dprog;
    shipinprogress *progress;

    //look for match
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        if (dprog->shipType == shipType &&
            dprog->shipRace == shipRace &&
            dprog->playerIndex == playerIndex &&
            dprog->creator == creator)
        {
            progress = cmFindProgressForFactory(creator, shipType);
            if (progress != NULL && progress->cataSaveField2 != 0)
            {
                cmUpdateExternalBuildShip((Ship *)progress->cataSaveField2, 0.0f, 1);
                progress->cataSaveField1 = 0;
                progress->cataSaveField2 = 0;
            }
            //match found, cancel & refund RUs
            cmAdjustRU(dprog, (sdword)(dprog->costSoFar * cmFloatRefundRatio));
            listDeleteNode(node);
            return;
        }

        //move on
        node = node->next;
    }
}

void cmDeterministicBuildPause(
    bool32 pause, ShipType shipType, ShipRace shipRace, sdword playerIndex, ShipPtr creator)
{
    Node* node;
    cmDetermProgress_t* dprog;

    //look for match
    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        if (dprog->shipType == shipType &&
            dprog->shipRace == shipRace &&
            dprog->playerIndex == playerIndex &&
            dprog->creator == creator)
        {
            //match found, adjust pause state
            dprog->paused = pause;
        }

        //next node
        node = node->next;
    }
}

void cmDeterministicBuild(udword command, sdword numShips,
                          ShipType shipType, ShipRace shipRace,
                          uword playerIndex, ShipPtr creator)
{
    switch (command)
    {
    case CMD_START:
        cmDeterministicBuildStart(numShips, shipType, shipRace, playerIndex, creator);
        break;

    case CMD_CANCEL:
        cmDeterministicBuildCancel(shipType, shipRace, playerIndex, creator);
        break;

    case CMD_PAUSE:
        cmDeterministicBuildPause(TRUE, shipType, shipRace, playerIndex, creator);
        break;

    case CMD_UNPAUSE:
        cmDeterministicBuildPause(FALSE, shipType, shipRace, playerIndex, creator);
        break;

    default:
        dbgFatalf(DBG_Loc, "unknown dbuild command: %u", command);
    }
}

void cmDeterministicBuildProcess(void)
{
    Node* node;
    Node* curNode;
    cmDetermProgress_t* dprog;
    Player* player;
    ShipStaticInfo *info;
    shipinprogress *progress;
    Ship *createdShip;

    if (universePause)
    {
        //universe is paused, we won't be building anything
        return;
    }

    node = cmDetermProgress.head;
    while (node != NULL)
    {
        dprog = (cmDetermProgress_t*)listGetStructOfNode(node);
        player = &universe.players[dprog->playerIndex];
        if (player->playerState != PLAYER_ALIVE)
        {
            //dead player, clear batch
            curNode = node;
            node = node->next;
            listDeleteNode(curNode);
            continue;
        }

        //advance
        curNode = node;
        node = node->next;

        if ((sdword)dprog->numShips <= 0)
        {
            listDeleteNode(curNode);
            continue;
        }

        if (!dprog->paused)
        {
            info = GetShipStaticInfo(dprog->shipType, dprog->shipRace);
            progress = cmFindProgressForFactory(dprog->creator, dprog->shipType);
            dbgAssert(info != NULL);

            if (info->externalBuild != 0 &&
                dprog->ticksLeft == dprog->ticksForOne &&
                !dprog->extBuildCreated &&
                cmBuildStateChanged(dprog->creator, info->shipclass, dprog->shipType, TRUE, 0))
            {
                if (info->shipclass == CLASS_Component ||
                    dprog->creator->shiptype == bMothership ||
                    univPlayerHasShipType(sMothershipDockingBay, dprog->creator->playerowner))
                {
                    createdShip = clCreateShip(&universe.mainCommandLayer,
                                               dprog->shipType,
                                               dprog->shipRace,
                                               (uword)dprog->playerIndex,
                                               dprog->creator);
                    dprog->extBuildCreated = TRUE;
                    dprog->extBuildStarted = FALSE;
                    dprog->extBuildShip = createdShip;
                    dbgAssert(progress != NULL);
                    progress->cataSaveField0 = 0;
                    progress->cataSaveField1 = 1;
                }
            }

            if (info->externalBuild == 0 ||
                dprog->extBuildStarted)
            {
                sdword lastCost, newCost;
                real32 costPerTick;

                costPerTick = (dprog->creator->shiptype == bMothership) ?
                    (real32)cmBeastBuildFactor(dprog->creator) * dprog->costPerTick :
                    dprog->costPerTick;
                lastCost = (sdword)dprog->costSoFar;
                newCost = (sdword)(dprog->costSoFar + costPerTick);

                if (player->resourceUnits == 0)
                {
                    continue;
                }

                if (newCost != lastCost)
                {
                    sdword amount = newCost - lastCost;
                    cmAdjustRU(dprog, -amount);
                }

                dprog->costSoFar += costPerTick;
                if (dprog->creator->shiptype == bMothership)
                {
                    dprog->ticksLeft -= cmBeastBuildFactor(dprog->creator);
                }
                else
                {
                    dprog->ticksLeft--;
                }

                if (dprog->ticksLeft > 0)
                {
                    if (info->externalBuild != 0 &&
                        dprog->extBuildStarted)
                    {
                        processUpdateExternalConstructionToDo(&universe.mainCommandLayer,
                                                              (uword)dprog->playerIndex,
                                                              dprog->creator,
                                                              dprog->extBuildShip,
                                                              cmExternalBuildProgress(dprog->ticksLeft, dprog->ticksForOne),
                                                              2);
                    }
                }
                else
                {
                    if (info->externalBuild != 0)
                    {
                        if (dprog->extBuildCreated)
                        {
                            processUpdateExternalConstructionToDo(&universe.mainCommandLayer,
                                                                  (uword)dprog->playerIndex,
                                                                  dprog->creator,
                                                                  dprog->extBuildShip,
                                                                  1.0f,
                                                                  0);
                        }
                        dprog->extBuildCreated = FALSE;
                        dprog->extBuildStarted = FALSE;
                        dprog->extBuildShip = NULL;
                        dbgAssert(progress != NULL);
                        progress->cataSaveField1 = 0;
                        progress->cataSaveField2 = 0;
                    }
                    else
                    {
                        clCreateShip(&universe.mainCommandLayer,
                                     dprog->shipType,
                                     dprog->shipRace,
                                     (uword)dprog->playerIndex,
                                     dprog->creator);
                    }

                    if (player->resourceUnits > 0)
                    {
                        cmAdjustRU(dprog,
                                   -(dprog->creator->shiptype == bMothership ?
                                         cmBeastBuildFactor(dprog->creator) :
                                         1));
                    }

                    dprog->costSoFar = 0.0f;
                    dprog->numShips--;
                    if (dprog->numShips == 0)
                    {
                        listDeleteNode(curNode);
                    }
                    else
                    {
                        dprog->ticksLeft = dprog->ticksForOne;
                    }
                }
            }
        }
    }
}

/*=============================================================================
    Save Game Stuff:
=============================================================================*/

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

#define RaceAndTypeToRaceType(race,type) ( ((race)<<16) + (type) )
#define GetRaceFromRaceType(racetype) ( ((racetype)>>16) & 0x0000ffff )
#define GetTypeFromRaceType(racetype) ( (racetype) & 0x0000ffff )

sdword ConvertShipStaticInfoToRaceType(ShipStaticInfo *shipstatic)
{
    if (shipstatic == NULL)
    {
        return -1;
    }
    else
    {
        return RaceAndTypeToRaceType(shipstatic->shiprace,shipstatic->shiptype);
    }
}

ShipStaticInfo *ConvertRaceTypeToShipStaticInfo(sdword racetype)
{
    if (racetype == -1)
    {
        return NULL;
    }
    else
    {
        return GetShipStaticInfo(GetTypeFromRaceType(racetype),GetRaceFromRaceType(racetype));
    }
}

void Save_shipsinprogress(shipsinprogress *sinprogress)
{
    SaveChunk *chunk;
    shipsinprogress *savecontents;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SHIPSINPROGRESS,sizeof(shipsinprogress),sinprogress);
    savecontents = (shipsinprogress *)chunkContents(chunk);

    savecontents->ship = (Ship *)SpaceObjRegistryGetID((SpaceObj *)savecontents->ship);
    dbgAssert((sdword)savecontents->ship != -1);
    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        savecontents->progress[i].info = (ShipStaticInfo *)ConvertShipStaticInfoToRaceType(savecontents->progress[i].info);
        savecontents->progress[i].cataSaveField2 =
            (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->progress[i].cataSaveField2);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveConsMgr()
{
    sdword num = listofShipsInProgress.num;
    sdword cur = 0;
    Node *node = listofShipsInProgress.head;

    SaveInfoNumber(num);

    while (node != NULL)
    {
        cur++;
        Save_shipsinprogress(listGetStructOfNode(node));

        node = node->next;
    }

    dbgAssert(cur == num);

    SaveInfoNumber(cmNumCarriers);
    if (cmNumCarriers > 0)
    {
        SaveChunk *chunk;
        CarrierInfo *savecontents;
        sdword i;

        chunk = CreateChunk(BASIC_STRUCTURE,sizeof(CarrierInfo) * cmNumCarriers,cmCarriers);
        savecontents = (CarrierInfo *)chunkContents(chunk);

        for (i=0;i<cmNumCarriers;i++)
        {
            savecontents[i].ship = (Ship *)SpaceObjRegistryGetID((SpaceObj *)savecontents[i].ship);
            dbgAssert((sdword)savecontents[i].ship != -1);
            savecontents[i].owner = (Player *)SavePlayerToPlayerIndex(savecontents[i].owner);
        }

        SaveThisChunk(chunk);
        memFree(chunk);
    }
}

shipsinprogress *Load_shipsinprogress()
{
    SaveChunk *chunk;
    shipsinprogress *sinprogress;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SHIPSINPROGRESS,sizeof(shipsinprogress));
    sinprogress = memAlloc(sizeof(shipsinprogress),"sinprogress",NonVolatile);
    memcpy(sinprogress,chunkContents(chunk),sizeof(shipsinprogress));

    memFree(chunk);

    sinprogress->ship = SpaceObjRegistryGetShip((sdword)sinprogress->ship);
    dbgAssert(sinprogress->ship != NULL);

    for (i=0;i<TOTAL_NUM_SHIPS;i++)
    {
        sinprogress->progress[i].info = ConvertRaceTypeToShipStaticInfo((sdword)sinprogress->progress[i].info);
        sinprogress->progress[i].cataSaveField2 =
            (sdword)SpaceObjRegistryGetShip(sinprogress->progress[i].cataSaveField2);
    }

    return sinprogress;
}

void LoadConsMgr()
{
    sdword num;
    sdword i;
    shipsinprogress *sinprogress;

    num = LoadInfoNumber();

    dbgAssert(listofShipsInProgress.num == 0);
    listInit(&listofShipsInProgress);

    for (i=0;i<num;i++)
    {
        sinprogress = Load_shipsinprogress();
        listAddNode(&listofShipsInProgress,&sinprogress->node,sinprogress);
    }

    cmNumCarriers = LoadInfoNumber();
    if (cmNumCarriers > 0)
    {
        SaveChunk *chunk;
        sdword size = sizeof(CarrierInfo) * cmNumCarriers;

        chunk = LoadNextChunk();
        VerifyChunk(chunk,BASIC_STRUCTURE,size);

        cmCarriers = memAlloc(size, "cmCarriers", NonVolatile);
        memcpy(cmCarriers,chunkContents(chunk),size);

        for (i=0;i<cmNumCarriers;i++)
        {
            cmCarriers[i].ship = SpaceObjRegistryGetShip((sdword)cmCarriers[i].ship);
            cmCarriers[i].owner = SavePlayerIndexToPlayer(cmCarriers[i].owner);
        }

        memFree(chunk);
    }

    cmFillInCarrierXs();
}

void SaveConsMgrDetermListCB(void *stuff)
{
    cmDetermProgress_t copy = *((cmDetermProgress_t *)stuff);

    copy.creator = SpaceObjRegistryGetID((SpaceObj *)copy.creator);
    copy.extBuildShip = SpaceObjRegistryGetID((SpaceObj *)copy.extBuildShip);

    SaveStructureOfSize(&copy,sizeof(copy));
}

void LoadConsMgrDetermListCB(LinkedList *list)
{
    cmDetermProgress_t *dp = LoadStructureOfSize(sizeof(cmDetermProgress_t));
    dp->creator = SpaceObjRegistryGetShip((sdword)dp->creator);
    dp->extBuildShip = SpaceObjRegistryGetShip((sdword)dp->extBuildShip);

    listAddNode(list,&dp->node,dp);
}

#define BUILD_OPTIONAL_FLAG     0xb11db11d

void SaveConsMgrDetermOptional()
{
    if (cmDetermProgress.num == 0)
    {
        return;         // don't save anything - optional block
    }

    SaveInfoNumber(BUILD_OPTIONAL_FLAG);

    SaveLinkedListOfStuff(&cmDetermProgress,SaveConsMgrDetermListCB);
}

void LoadConsMgrDetermOptional()
{
    sdword optional;

    if (LoadInfoNumberOptional(&optional) && (optional == BUILD_OPTIONAL_FLAG))
    {
        LoadLinkedListOfStuff(&cmDetermProgress,LoadConsMgrDetermListCB);
    }
    else
    {
        listInit(&cmDetermProgress);
    }
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"

void shipAddSupportUsage(Ship *ship, Player *player)
{
    if ((ship->specialFlags2 & SPECIAL_2_DisabledState) != 0)
    {
        return;
    }

    player->supportUnitsUsed += ShipStaticInfoGetSupportUnits(ship->staticinfo);
    if ((ship->specialFlags2 & SPECIAL_2_DockFlag) == 0)
    {
        player->supportUnitsMax += ShipStaticInfoGetProvidesSU(ship->staticinfo);
        ship->specialFlags2 |= SPECIAL_2_SupportCapacityAccounted;
    }
}

void shipRemoveSupportUsage(Ship *ship)
{
    Player *player = ship->playerowner;

    if ((ship->specialFlags2 & SPECIAL_2_DisabledState) != 0)
    {
        return;
    }

    player->supportUnitsUsed -= ShipStaticInfoGetSupportUnits(ship->staticinfo);
    if ((ship->specialFlags2 & SPECIAL_2_SupportCapacityAccounted) != 0)
    {
        player->supportUnitsMax -= ShipStaticInfoGetProvidesSU(ship->staticinfo);
        ship->specialFlags2 &= ~SPECIAL_2_SupportCapacityAccounted;
    }
}
sdword cmCurrentPlayerShipInProgressByType(char *shipTypeName)
{
    Node *node;
    ShipType shipType = StrToShipType(shipTypeName);

    for (node = listofShipsInProgress.head; node != NULL; node = node->next)
    {
        sdword i;
        shipsinprogress *factory = listGetStructOfNode(node);

        if (factory->ship->playerowner != universe.curPlayerPtr)
        {
            continue;
        }

        for (i = 0; i < TOTAL_NUM_SHIPS; i++)
        {
            shipinprogress *progress = &factory->progress[i];

            if (progress->info != NULL && progress->info->shiptype == shipType)
            {
                return progress->nJobs;
            }
        }
    }

    return 0;
}

Ship *shipAddSupportCapacity(Ship *ship)
{
    if ((ship->specialFlags2 & SPECIAL_2_DockFlag) == 0)
    {
        ship->playerowner->supportUnitsMax += ShipStaticInfoGetProvidesSU(ship->staticinfo);
        ship->specialFlags2 |= SPECIAL_2_SupportCapacityAccounted;
    }
    return ship;
}

sdword cmCollectPlayerBuildTypesInProgress(Player *player, sdword *buildSummary)
{
    sdword buildTypeCount = 0;
    sdword buildIndexByType[TOTAL_NUM_SHIPS];
    sdword nextSecondarySlot = 1;
    Node *node;
    sdword i;

    if (player == NULL || buildSummary == NULL)
    {
        return 0;
    }

    for (i = 0; i < TOTAL_NUM_SHIPS; i++)
    {
        buildIndexByType[i] = -1;
    }

    for (node = listofShipsInProgress.head; node != NULL; node = node->next)
    {
        shipsinprogress *factory = listGetStructOfNode(node);
        sdword buildSlot;

        if (factory->ship->playerowner != player)
        {
            continue;
        }

        if (factory->ship->shiptype == sMothership ||
            factory->ship->shiptype == bMothership)
        {
            buildSlot = 0;
        }
        else
        {
            buildSlot = nextSecondarySlot++;
        }

        if (buildSlot >= 4)
        {
            continue;
        }

        for (i = 0; i < TOTAL_NUM_SHIPS; i++)
        {
            shipinprogress *progress = &factory->progress[i];
            sdword buildSummaryIndex;
            sdword *summary;

            if (progress->nJobs <= 0 || progress->info == NULL)
            {
                continue;
            }

            buildSummaryIndex = buildIndexByType[progress->info->shiptype];
            if (buildSummaryIndex == -1)
            {
                summary = &buildSummary[10 * buildTypeCount];
                summary[0] = progress->info->shiptype;
                summary[1] = 0;
                summary[2] = 0;
                summary[3] = 0;
                summary[4] = 0;
                summary[5] = singlePlayerGame ? (progress->info->buildTime / GameDifficultyBuildTimeModifier) : progress->info->buildTime;
                summary[6] = progress->timeLeft;
                summary[7] = 0;
                summary[8] = 0;
                summary[9] = 0;

                buildIndexByType[progress->info->shiptype] = buildTypeCount;
                buildSummaryIndex = buildTypeCount;
                buildTypeCount++;
            }

            summary = &buildSummary[10 * buildSummaryIndex];
            summary[1 + buildSlot] = progress->nJobs;
            summary[6 + buildSlot] = progress->timeLeft;

        }
    }

    return buildTypeCount;
}

bool32 cmConstructionManagerActive(void)
{
    return (cmBaseRegion != NULL);
}
