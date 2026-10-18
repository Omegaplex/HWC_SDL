/*=============================================================================
    Name    : researchgui.c
    Purpose : logic for the research manager

    Created 5/27/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include "glinc.h"
#include "main.h"
#include "types.h"
#include "universe.h"
#include "region.h"
#include "uicontrols.h"
#include "feflow.h"
#include "font.h"
#include "fontreg.h"
#include "objtypes.h"
#include "texreg.h"
#include "task.h"
#include "mouse.h"
#include "CommandLayer.h"
#include "piePlate.h"
#include "globals.h"
#include "CommandWrap.h"
#include "scroller.h"
#include "soundevent.h"
#include "randy.h"
#include "mainrgn.h"
#include "shipdefs.h"
#include "prim2d.h"
#include "researchapi.h"
#include "researchgui.h"
#include "strings.h"
#include "fereg.h"
#include "taskbar.h"
#include "render.h"
#define FAST_TO_INT(X) ((int)(X))
#include "singleplayer.h"
#include "tutor.h"
#include "trademgr.h"
#include "multiplayergame.h"
#include "FEColour.h"
#include "glcompat.h"
#include "InfoOverlay.h"
#include "commandnetwork.h"
#include "camera.h"
#include "Sensors.h"

extern Camera smCamera;

/*=============================================================================
    Defines:
=============================================================================*/

#define RM_LabIcon              "Research_icon_weapons_on.lif"


// defines for spacing
#define RM_VertSpacing          (fontHeight(" ") >> 1)
#define RM_HorzSpacing          (fontWidth(" "))
#define RM_InfoWidth            (region->rect.x1 - region->rect.x0)

// defines for the headings in the tech list box
#define WeaponHeading           0
#define ConstructionHeading     1
#define InterdictionHeading     2
#define SupportHeading          3


// defines for the buttons sizeing
#define LB_BUTTONSIZE           45
#define LB_HALFBUTTON           (LB_BUTTONSIZE/2)
#define LB_BUTTONSPACE          6
#define LB_BUTTONWIDTH          25

// defines for the pregeneration of the circle data
#define CIRCLE_POINTS           150
#define CIRCLE_XRADIUS          0.06875
#define CIRCLE_YRADIUS          0.09166666666667

// defines for the pulse effect
#define PULSE_INC               2048
#define FIXED16_ONE             65536
#define PULSE_COLOR             colRGB(200, 200, 200)

// defines for the pictures LRM caching

#define RM_DSINIT               0
#define RM_DSNORMAL             1
#define RM_DSDRAW               2
#define RM_DSERASE              3

#define RM_MARQUEERAD           3
#define RM_NUMMARQUEE           3
#define RM_MARQUEESPACE         7
#define RM_MARQUEE_Y_SPACE      18
#define RM_MARQUEE_HEIGHT       (LB_BUTTONSIZE - (RM_MARQUEE_Y_SPACE * 2))
#define RM_MARQUEE_HALF_HEIGHT  (RM_MARQUEE_HEIGHT / 2)
#define RM_MARQUEERATE          3.0
#define RM_TEXTURE_INSET        3
#define RM_LAB_INSET            5

#define NUMTECHSINLIST          190
#define RM_NumVisibleLabButtons 5
#define RM_NumLabButtonTextures 15

/*=============================================================================
    Data:
=============================================================================*/

sdword rmRenderEverythingCounter;
static bool32 rmOpenedFromSensors = FALSE;

static featom* _rmTechAtom;
static regionhandle _rmTechRegion;
static bool32 _rmTechRender = FALSE;

double chop_temp;

bool32 rmPaletted;

fonthandle rmTechListFont=0;
fonthandle rmTechInfoFont=0;
char rmTechListFontName [RM_FontNameLength] = RM_TechListFont;
char rmTechInfoFontName [RM_FontNameLength] = RM_TechInfoFont;

static const strGamesMessages rmTechHeadingStrings[] =
{
    strWeaponHeading,
    strConstuctionHeading,
    strInterdictionHeading,
    strSupportHeading,
    strHANGAR_HEADING,
    strARMOUR_HEADING,
    strENGINEERING_HEADING,
    strADV_ENGINEERING_HEADING,
    strWEAPON_HEADING,
    strGENETICS_HEADING,
    strRECON_HEADING,
    strWORKER_HEADING,
    strPROCESSOR_HEADING,
    strACOLYTE_HEADING,
    strACV_HEADING,
    strMULTIBEAMFRIGATE_HEADING,
    strRAMMINGFRIGATE_HEADING,
    strHIVEFRIGATE_HEADING,
    strINTERCEPTOR_HEADING,
    strCLOAKEDFIGHTER_HEADING,
    strATTACKBOMBER_HEADING,
    strCRUISEMISSILE_HEADING,
    strMISSILECORVETTE_HEADING,
    strMULTIGUNCORVETTE_HEADING,
    strHEAVYCORVETTE_HEADING,
    strDFGFRIGATE_HEADING,
    strIONARRAYFRIGATE_HEADING,
    strHEAVYCRUISER_HEADING,
    strCARRIER_HEADING,
    strSENTINEL_HEADING
};

//flag indicating loaded status of this screen
fibfileheader *rmScreensHandle = NULL;
sdword rmScreensRace = -1;

regionhandle  rmTechBriefRegion = NULL;
regionhandle  rmTechImageRegion = NULL;
regionhandle  rmTechListRegion  = NULL;
regionhandle  rmBaseRegion      = NULL;

regionhandle rmLab1Region = NULL;
regionhandle rmLab2Region = NULL;
regionhandle rmLab3Region = NULL;
regionhandle rmLab4Region = NULL;
regionhandle rmLab5Region = NULL;
regionhandle rmLab6Region = NULL;

regionhandle rmConnectorRegion = NULL;

regionhandle rmLabConnector1Region = NULL;
regionhandle rmLabConnector2Region = NULL;
regionhandle rmLabConnector3Region = NULL;
regionhandle rmLabConnector4Region = NULL;
regionhandle rmLabConnector5Region = NULL;
regionhandle rmLabConnector6Region = NULL;

listwindowhandle rmTechListWindowHandle = NULL;

// callbacks for ui elements

void rmSelectAll(char *string, featom *atom);
void rmClearCurrentTechSelection(char *string, featom *atom);
void rmClearAllLabs(char *string, featom *atom);
void rmClearSelectedLab(char *string, featom *atom);
void rmResearchItem(char *string, featom *atom);
void rmExitMenu(char *string, featom *atom);
void rmTechListWindow(char *string, featom *atom);
void rmExtendedInfo(char *string, featom *atom);
sdword rmResearchCurrentSelection(void);

// callbacks for user draw areas

void rmLab1Draw(featom *atom, regionhandle region);
void rmLab2Draw(featom *atom, regionhandle region);
void rmLab3Draw(featom *atom, regionhandle region);
void rmLab4Draw(featom *atom, regionhandle region);
void rmLab5Draw(featom *atom, regionhandle region);
void rmLab6Draw(featom *atom, regionhandle region);
void rmLabConnector1Draw(featom *atom, regionhandle region);
void rmLabConnector2Draw(featom *atom, regionhandle region);
void rmLabConnector3Draw(featom *atom, regionhandle region);
void rmLabConnector4Draw(featom *atom, regionhandle region);
void rmLabConnector5Draw(featom *atom, regionhandle region);
void rmLabConnector6Draw(featom *atom, regionhandle region);
void rmConnectorDraw(featom *atom, regionhandle region);
void rmTechBriefDraw(featom *atom, regionhandle region);
void rmCurrentSelectionLabelDraw(featom *atom, regionhandle region);
void rmTechImageDraw(featom *atom, regionhandle region);
void rmDrawLabIcon(featom *atom, regionhandle region);

fecallback rmCallback[] =
{
    {rmClearCurrentTechSelection, "RM_ClearAllLabs"     },
    {rmResearchItem         ,   "RM_ClearLab"           },
    {(fefunction)rmResearchCurrentSelection, "RM_ResearchItem" },
    {rmExitMenu             ,   "RM_ExitMenu"           },
    {rmTechListWindow       ,   "RM_TechListWindow"     },
    {rmExtendedInfo         ,   "RM_ExtendedInfo"       },
    {NULL,                      NULL                    }
};

fedrawcallback rmDrawCallback[] =
{
    {rmLab1Draw,                "RM_Lab1Draw"           },
    {rmLab2Draw,                "RM_Lab2Draw"           },
    {rmLab3Draw,                "RM_Lab3Draw"           },
    {rmLab4Draw,                "RM_Lab4Draw"           },
    {rmLab5Draw,                "RM_Lab5Draw"           },
    {rmLab6Draw,                "RM_Lab6Draw"           },
    {rmLabConnector1Draw,       "RM_LabConnector1Draw"  },
    {rmLabConnector2Draw,       "RM_LabConnector2Draw"  },
    {rmLabConnector3Draw,       "RM_LabConnector3Draw"  },
    {rmLabConnector4Draw,       "RM_LabConnector4Draw"  },
    {rmLabConnector5Draw,       "RM_LabConnector5Draw"  },
    {rmLabConnector6Draw,       "RM_LabConnector6Draw"  },
    {rmConnectorDraw,           "RM_ConnectorDraw"      },
    {rmTechBriefDraw,           "RM_TechBriefDraw"       },
    {rmCurrentSelectionLabelDraw, "RM_ModuleNameDraw"   },
    {rmTechImageDraw,           "RM_TechImageDraw"      },
    {rmDrawLabIcon,             "RM_PussyBlob"          },
    {NULL,                      NULL                    }
};


TechPrintList PlayerTechListStatic[NUMTECHSINLIST] =
{
    {ITEM_CLASSHEADER   ,   0               ,   WeaponHeading           },

    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive1Kt            },
    {ITEM_TECHNOLOGY    ,   0               ,   Chassis1                },
    {ITEM_TECHNOLOGY    ,   0               ,   FireControl             },
    {ITEM_TECHNOLOGY    ,   0               ,   CoolingSystems          },
    {ITEM_TECHNOLOGY    ,   0               ,   PlasmaWeapons           },
    {ITEM_TECHNOLOGY    ,   0               ,   AdvancedCoolingSystems  },
    {ITEM_TECHNOLOGY    ,   0               ,   AdvancedFireControl     },
    {ITEM_TECHNOLOGY    ,   0               ,   CloakDefenseFighter     },

    {ITEM_CLASSHEADER   ,   0               ,   ConstructionHeading     },

    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive10Kt           },
    {ITEM_TECHNOLOGY    ,   0               ,   Chassis2                },
    {ITEM_TECHNOLOGY    ,   0               ,   MediumGuns              },
    {ITEM_TECHNOLOGY    ,   0               ,   TargetingSystems        },
    {ITEM_TECHNOLOGY    ,   0               ,   MineLayerTech           },
    {ITEM_TECHNOLOGY    ,   0               ,   NewAlloys               },

    {ITEM_CLASSHEADER   ,   0               ,   InterdictionHeading     },

    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive100Kt          },
    {ITEM_TECHNOLOGY    ,   0               ,   Chassis3                },
    {ITEM_TECHNOLOGY    ,   0               ,   IonWeapons              },
    {ITEM_TECHNOLOGY    ,   0               ,   MissileWeapons          },

    {ITEM_TECHNOLOGY    ,   0               ,   DDDFDFGFTech            },
    {ITEM_TECHNOLOGY    ,   0               ,   MassDrive1Mt            },
    {ITEM_TECHNOLOGY    ,   0               ,   RepairTech              },

    {ITEM_TECHNOLOGY    ,   0               ,   HeavyGuns               },

    {ITEM_CLASSHEADER   ,   0               ,   SupportHeading          },
    {ITEM_TECHNOLOGY    ,   0               ,   ProximityDetector       },
    {ITEM_TECHNOLOGY    ,   0               ,   SupportRefuelTech       },
    {ITEM_TECHNOLOGY    ,   0               ,   AdvanceTacticalSupport  },
    {ITEM_TECHNOLOGY    ,   0               ,   ConstructionTech        },
    {ITEM_TECHNOLOGY    ,   0               ,   SensorsArrayTech        },
    {ITEM_TECHNOLOGY    ,   0               ,   GravityWellGeneratorTech},
    {ITEM_TECHNOLOGY    ,   0               ,   CloakGeneratorTech      },

    {ITEM_ENDLIST       ,   0               ,   0                       }
};


static TechPrintList PlayerTechListStaticR1[] =
{
    { ITEM_CLASSHEADER, 0, 4, 0 },
    { ITEM_MODULETECH, 0, TechDriveFighter, 0 },
    { ITEM_MODULETECH, 0, TechDriveFrigate, 0 },
    { ITEM_MODULETECH, 0, TechDriveDestroyer, 0 },
    { ITEM_MODULETECH, 0, TechLink, 0 },
    { ITEM_SHIPTECH,   0, TechADriveFighter, 0 },
    { ITEM_SHIPTECH,   0, TechADriveFrigate, 0 },
    { ITEM_SHIPTECH,   0, TechADriveDestroyer, 0 },
    { ITEM_CLASSHEADER, 0, 5, 1 },
    { ITEM_SHIPTECH,   0, TechArmour2, 1 },
    { ITEM_SHIPTECH,   0, TechArmour3, 1 },
    { ITEM_SHIPTECH,   0, TechForceField1, 1 },
    { ITEM_SHIPTECH,   0, TechForceField2, 1 },
    { ITEM_SHIPTECH,   0, TechForceField3, 1 },
    { ITEM_CLASSHEADER, 0, 6, 2 },
    { ITEM_MODULETECH, 0, TechMimic, 2 },
    { ITEM_MODULETECH, 0, TechSupportSystems, 2 },
    { ITEM_MODULETECH, 0, TechAfterburner, 2 },
    { ITEM_SHIPTECH,   0, TechRepair, 2 },
    { ITEM_SHIPTECH,   0, TechSalvage, 2 },
    { ITEM_SHIPTECH,   0, TechResearchCleesanAttack, 2 },
    { ITEM_CLASSHEADER, 0, 7, 3 },
    { ITEM_MODULETECH, 0, TechMicroShip, 3 },
    { ITEM_MODULETECH, 0, TechPhasedTelemetry, 3 },
    { ITEM_SHIPTECH,   0, TechAdvancedSensors, 3 },
    { ITEM_SHIPTECH,   0, TechRepairBots, 3 },
    { ITEM_SHIPTECH,   0, TechBeastVaccine, 3 },
    { ITEM_SHIPTECH,   0, TechCrystalHarvest, 3 },
    { ITEM_SHIPTECH,   0, TechInertialessDrive, 3 },
    { ITEM_CLASSHEADER, 0, 8, 4 },
    { ITEM_MODULETECH, 0, TechIon, 4 },
    { ITEM_MODULETECH, 0, TechMissile, 4 },
    { ITEM_SHIPTECH,   0, TechEnergy, 4 },
    { ITEM_SHIPTECH,   0, TechEMP, 4 },
    { ITEM_SHIPTECH,   0, TechExplosiveCharge, 4 },
    { ITEM_SHIPTECH,   0, TechRepulse, 4 },
    { ITEM_ENDLIST,    0, 0, 0 }
};

static TechPrintList PlayerTechListStaticBeast[] =
{
    { ITEM_CLASSHEADER, 0, 9, 0 },
    { ITEM_SHIPTECH, 0, 16, 0 },
    { ITEM_SHIPTECH, 0, 32, 0 },
    { ITEM_SHIPTECH, 0, 34, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 10, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 9, 0 },
    { ITEM_SHIPTECH, 0, 28, 0 },
    { ITEM_CLASSHEADER, 0, 11, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, 6, 0 },
    { ITEM_SHIPTECH, 0, 7, 0 },
    { ITEM_CLASSHEADER, 0, 12, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 6, 0 },
    { ITEM_SHIPTECH, 0, 34, 0 },
    { ITEM_CLASSHEADER, 0, 13, 0 },
    { ITEM_TECHHIDDEN, 0, 8, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 9, 0 },
    { ITEM_TECHHIDDEN, 0, 24, 0 },
    { ITEM_TECHHIDDEN, 0, 20, 0 },
    { ITEM_CLASSHEADER, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 8, 0 },
    { ITEM_TECHHIDDEN, 0, 24, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 9, 0 },
    { ITEM_TECHHIDDEN, 0, 22, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 29, 0 },
    { ITEM_TECHHIDDEN, 0, 26, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 19, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_CLASSHEADER, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, 10, 0 },
    { ITEM_TECHHIDDEN, 0, 19, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 11, 0 },
    { ITEM_CLASSHEADER, 0, 16, 0 },
    { ITEM_TECHHIDDEN, 0, 10, 0 },
    { ITEM_TECHHIDDEN, 0, 17, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 11, 0 },
    { ITEM_CLASSHEADER, 0, 17, 0 },
    { ITEM_TECHHIDDEN, 0, 17, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_TECHHIDDEN, 0, 29, 0 },
    { ITEM_TECHHIDDEN, 0, 30, 0 },
    { ITEM_TECHHIDDEN, 0, 31, 0 },
    { ITEM_TECHHIDDEN, 0, 28, 0 },
    { ITEM_CLASSHEADER, 0, 18, 0 },
    { ITEM_MODULETECH, 0, 8, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 9, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 19, 0 },
    { ITEM_MODULETECH, 0, 8, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 0, 0 },
    { ITEM_TECHHIDDEN, 0, 9, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 20, 0 },
    { ITEM_MODULETECH, 0, 8, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 2, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 9, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 21, 0 },
    { ITEM_MODULETECH, 0, 40, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 39, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_SHIPTECH, 0, 32, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 22, 0 },
    { ITEM_MODULETECH, 0, 40, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 20, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 23, 0 },
    { ITEM_MODULETECH, 0, 40, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 24, 0 },
    { ITEM_MODULETECH, 0, 40, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 3, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_CLASSHEADER, 0, 25, 0 },
    { ITEM_MODULETECH, 0, 10, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 4, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 11, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_CLASSHEADER, 0, 26, 0 },
    { ITEM_MODULETECH, 0, 10, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_MODULETECH, 0, 19, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_SHIPTECH, 0, 0, 0 },
    { ITEM_TECHHIDDEN, 0, 11, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_CLASSHEADER, 0, 27, 0 },
    { ITEM_MODULETECH, 0, 12, 0 },
    { ITEM_MODULETECH, 0, 5, 0 },
    { ITEM_MODULETECH, 0, 19, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_SHIPTECH, 0, 32, 0 },
    { ITEM_CLASSHEADER, 0, 28, 0 },
    { ITEM_MODULETECH, 0, 12, 0 },
    { ITEM_MODULETECH, 0, 16, 0 },
    { ITEM_TECHHIDDEN, 0, -1, 0 },
    { ITEM_TECHHIDDEN, 0, 18, 0 },
    { ITEM_TECHHIDDEN, 0, 14, 0 },
    { ITEM_TECHHIDDEN, 0, 15, 0 },
    { ITEM_SHIPTECH, 0, 32, 0 },
    { ITEM_SHIPTECH, 0, 34, 0 },
    { ITEM_ENDLIST, 0, 0, 0 }
};

TechPrintList SinglePlayerTechList[NUMTECHSINLIST];

TechPrintList MultiPlayerSectTechList[NUMTECHSINLIST];
TechPrintList MultiPlayerBeastTechList[NUMTECHSINLIST];

TechPrintList MultiPlayerCarrierSectTechList[NUMTECHSINLIST];
TechPrintList MultiPlayerCarrierBeastTechList[NUMTECHSINLIST];

TechNames TechImageNames[]=
{
    {"NewAlloys.lif"                },
    {"MassDrive1Kt.lif"             },
    {"CoolingSystems.lif"           },
    {"CloakDefenseFighter.lif"      },
    {"TargetingSystems.lif"         },
    {"PlasmaWeapons.lif"            },
    {"Chassis1.lif"                 },
    {"MassDrive10Kt.lif"            },
    {"MediumGuns.lif"               },
    {"MineLayerTech.lif"            },
    {"Chassis2.lif"                 },
    {"AdvancedCoolingSystems.lif"   },
    {"MassDrive100Kt.lif"           },
    {"FireControl.lif"              },
    {"SupportRefuelTech.lif"        },
    {"AdvanceTacticalSupport.lif"   },
    {"IonWeapons.lif"               },
    {"DDDFDFGFTech.lif"             },
    {"Chassis3.lif"                 },
    {"MassDrive1Mt.lif"             },
    {"AdvancedFireControl.lif"      },
    {"MissileWeapons.lif"           },
    {"ConstructionTech.lif"         },
    {"HeavyGuns.lif"                },
    {"ProximityDetector.lif"        },
    {"SensorsArrayTech.lif"         },
    {"GravityWellGeneratorTech.lif" },
    {"CloakGeneratorTech.lif"       },
    {"RepairTech.lif"               },
    {"SalvageTech.lif"              },
    {NULL                           }
};

char* TechImagePaths[]=
{
    {"ResearchGUI\\Race1\\" },
    {"ResearchGUI\\Race2\\" },
    {NULL                   }
};

char* LabImagePaths[]=
{
    {"ResearchGUI\\Race1\\" },
    {"ResearchGUI\\Race1\\" },
    {NULL                   }
};

static const char *rmLabButtonIconNames[RM_NumLabButtonTextures] =
{
    "research_icon_hangar_disabled.lif",
    "research_icon_armour_disabled.lif",
    "research_icon_engineering_disabled.lif",
    "research_icon_special_disabled.lif",
    "research_icon_weapons_disabled.lif",
    "research_icon_hangar_off.lif",
    "research_icon_armour_off.lif",
    "research_icon_engineering_off.lif",
    "research_icon_special_off.lif",
    "research_icon_weapons_off.lif",
    "research_icon_hangar_on.lif",
    "research_icon_armour_on.lif",
    "research_icon_engineering_on.lif",
    "research_icon_special_on.lif",
    "research_icon_weapons_on.lif"
};

static const char *rmBeastBlobImagePaths[] =
{
    "ResearchGUI\\Race2\\bresearch_ani_01.lif",
    "ResearchGUI\\Race2\\bresearch_ani_02.lif",
    "ResearchGUI\\Race2\\bresearch_ani_03.lif",
    "ResearchGUI\\Race2\\bresearch_ani_04.lif",
    "ResearchGUI\\Race2\\bresearch_ani_05.lif",
    "ResearchGUI\\Race2\\bresearch_ani_06.lif"
};

static const uword rmBeastBlobFrameSequence[] = { 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };
static const char *rmDefaultResearchImageName = "default_research_image.lif";

TechPrintList *PlayerTechList;

//colors for printing selections and stuff

color rmSelectionTextColor     = RM_SelectionTextColor;
color rmResearchingTextColor   = RM_ResearchingTextColor;
color rmCantResearchTextColor  = RM_CantResearchTextColor;
color rmStandardTextColor      = RM_StandardTextColor;
color rmClassHeadingTextColor  = RM_ClassHeadingTextColor;
color rmTechInfoTextColor      = colRGB(121, 255, 238);
color rmCanResearchShipTextColor = colRGB(0, 215, 155);
color rmAlreadyHaveShipTextColor = colRGB(0, 130, 94);
color rmProgressToGoColor      = RM_ProgressToGoColor;
color rmProgressDoneColor0     = RM_ProgressDoneColor0;
color rmProgressDoneColor1     = RM_ProgressDoneColor1;
color rmLabActiveColor         = RM_LabActiveColor;
color rmPulseColor             = RM_PulseColor;
color rmNoResearchItemColor    = RM_NoResearchItemColor;
color rmMarqueeOnColor         = RM_MarqueeOnColor;
color rmMarqueeSemiOnColor     = RM_MarqueeSemiOnColor;
color rmMarqueeOffColor        = RM_MarqueeOffColor;

sdword rmCurIndex=-1;
TechnologyType  techinfo=-1;

// is TRUE if GUI is active FALSE otherwise.
bool32 rmGUIActive=FALSE;

// List of lab buttons
LabPrintList labbuttons[RM_NumVisibleLabButtons];

// image variables for technology bitmap
LRUPicture      pictures[RM_TOTALPICS];
udword          rmCurTexture   = TR_InvalidInternalHandle;
lifheader      *rmCurTechImage = NULL;
TechnologyType  rmCurTechTexture = -1;
sdword          drawstate;
udword          rmLabTexture[MAX_RACES] = { TR_InvalidInternalHandle, TR_InvalidInternalHandle };
lifheader      *rmLabImage[MAX_RACES] = { NULL, NULL };
udword          rmLabButtonTexture[RM_NumLabButtonTextures] =
{
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle
};
lifheader      *rmLabButtonImage[RM_NumLabButtonTextures] =
{
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL
};
udword          rmBeastBlobTexture[sizeof(rmBeastBlobImagePaths) / sizeof(rmBeastBlobImagePaths[0])] =
{
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle,
    TR_InvalidInternalHandle, TR_InvalidInternalHandle, TR_InvalidInternalHandle
};
lifheader      *rmBeastBlobImage[sizeof(rmBeastBlobImagePaths) / sizeof(rmBeastBlobImagePaths[0])] =
{
    NULL, NULL, NULL, NULL, NULL, NULL
};
real32          rmBeastBlobAnimTime = 0.0f;

static const char *rmTechModuleImageNames[] =
{
    "MODULE_Weapon",
    "MODULE_Advanced_Engineering",
    "MODULE_Engineering",
    "MODULE_Armour",
    "MODULE_Hangar"
};

static const char *rmTechImageFileStem(sdword techIndex)
{
    sdword moduleIndex;

    if ((udword)techIndex <= TechDriveCorvette)
    {
        return TechTypeToString((TechnologyType)techIndex);
    }

    moduleIndex = techIndex - (TechDriveCorvette + 1);
    if ((udword)moduleIndex < (sizeof(rmTechModuleImageNames) / sizeof(rmTechModuleImageNames[0])))
    {
        return rmTechModuleImageNames[moduleIndex];
    }

    dbgFatalf(DBG_Loc, "Unknown research image index %d", techIndex);
    return "";
}
real32          rmBeastBlobPrevTime = 0.0f;
sdword          rmBeastBlobFrame = 0;

// variable for marque light position
real32          marqueetime=0.0;
real32          marqueepos=0;

bool32 rmExtendedInfoActive = FALSE;

uword researchingthistopic;
bool32 multipleresearchersselected;
ResearchTopic *multipleresearchtopic;
static sdword rmCurrentSelectionLab = -1;
/*=============================================================================
    Function Prototypes:
=============================================================================*/

void   rmUpdateTechList(void);
sdword rmSelectAvailableTech(regionhandle region, sdword ID, udword event, udword data);
sdword rmSelectTechType(regionhandle region, sdword yClicked);
void   SelectTechWindow(void);
int lgUpdateChatRegions(void);

/*=============================================================================
    Functions:
=============================================================================*/

void rmCurrentSelectionLabelDraw(featom *atom, regionhandle region)
{
    fonthandle oldFont;
    char label[32];

    if (rmCurrentSelectionLab < 0 || rmCurrentSelectionLab >= RM_NumVisibleLabButtons)
    {
        return;
    }

    sprintf(label, "Lab %d", rmCurrentSelectionLab + 1);
    oldFont = fontMakeCurrent(rmTechInfoFont);
    fontPrint(region->rect.x0 + 2, region->rect.y0 + 2, colRGB(0, 0, 0), label);
    fontPrint(region->rect.x0, region->rect.y0, colRGB(255, 255, 255), label);
    fontMakeCurrent(oldFont);
}

sdword rmResearchCurrentSelection(void)
{
    TechPrintList *tech;
    ResearchLab *lab;
    uword playerIndex;

    if (!rmTechListWindowHandle->CurLineSelected)
        return 0;

    tech = (TechPrintList *)rmTechListWindowHandle->CurLineSelected->data;

    if (tech->itemstat != STAT_ALREADYHAVE)
    {
        if (tutorial != TUTORIAL_ONLY || (((udword *)&tutEnable)[1] & 0x800u))
        {
            lab = labbuttons[tech->labnum].lab;

            if (lab->labstatus == LS_NORESEARCHITEM || (tutorial && !lab->labstatus))
            {
                lab->labstatus = LS_RESEARCHITEMSOON;
                lab->topic = NULL;
                clWrapResearch(RESEARCH_SUBCOMMAND_START, universe.curPlayerIndex, tech->labnum, tech->itemID);
                playerIndex = (uword)universe.curPlayerIndex;
                labbuttons[tech->labnum].pulsepos = 0;
                speechEventQueue(0, 67113009, -1, -1, 0, playerIndex, -1, 5.0f, -1);
            }

            rmCurrentSelectionLab = tech->labnum;

            if (rmBaseRegion)
                rmBaseRegion->status |= RSF_ReallyDirty;

            rmTechListWindowHandle->reg.status |= RSF_DrawThisFrame;
        }
    }

    return (sdword)rmTechListWindowHandle;
}

void rmDirtyTechInfo()
{
    if (rmTechBriefRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechBriefRegion);
#endif
        rmTechBriefRegion->status |= RSF_DrawThisFrame;
    }
    if (rmTechImageRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechImageRegion);
#endif
        rmTechImageRegion->status |= RSF_DrawThisFrame;
    }
    if (rmConnectorRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmConnectorRegion);
#endif
        rmConnectorRegion->status |= RSF_DrawThisFrame;
    }
}

void rmDirtyTechList()
{
    if (rmTechListRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechListRegion);
#endif
        rmTechListRegion->status |= RSF_DrawThisFrame;
    }

    if (rmLab1Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab1Region);
#endif
        rmLab1Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab2Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab2Region);
#endif
        rmLab2Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab3Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab3Region);
#endif
        rmLab3Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab4Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab4Region);
#endif
        rmLab4Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab5Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab5Region);
#endif
        rmLab5Region->status |= RSF_DrawThisFrame;
    }
    if (rmLab6Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLab6Region);
#endif
        rmLab6Region->status |= RSF_DrawThisFrame;
    }

    if (rmLabConnector1Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector1Region);
#endif
        rmLabConnector1Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector2Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector2Region);
#endif
        rmLabConnector2Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector3Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector3Region);
#endif
        rmLabConnector3Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector4Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector4Region);
#endif
        rmLabConnector4Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector5Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector5Region);
#endif
        rmLabConnector5Region->status |= RSF_DrawThisFrame;
    }
    if (rmLabConnector6Region != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmLabConnector6Region);
#endif
        rmLabConnector6Region->status |= RSF_DrawThisFrame;
    }
}

void rmSelectAll(char *string, featom *atom)
{
    sdword index;

    for (index=0; index<RM_NumVisibleLabButtons; index++)
    {
        if (labbuttons[index].lab->labstatus==LS_NORESEARCHITEM)
        {
            labbuttons[index].selected = TRUE;
        }
    }
}

void rmClearCurrentTechSelection(char *string, featom *atom)
{
    rmCurrentSelectionLab = -1;
}

// This function is called when a technology has just finished being researched.  This is so the technology list window
// can be updated.  The lab highlight border is turned off.
void rmClearLab(sdword labindex)
{
/*    rmUpdateTechList();

    // Regenerate list because something has just finished being researched
    uicListCleanUp(rmTechListWindowHandle);
    for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
    {
        if ( (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
             (PlayerTechList[index].itemstat != STAT_CANTRESEARCH) )
        {
            if (PlayerTechList[index].itemtype == ITEM_CLASSHEADER)
                uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], 0, UICLW_AddToTail);
            else
                uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], UICLI_CanSelect, UICLW_AddToTail);
        }
    }
    rmTechListWindowHandle->CurLineSelected = NULL;

    SelectTechWindow();*/

    labbuttons[labindex].selected = FALSE;
}

void rmClearAllLabs(char *string, featom *atom)
{
    sdword index;

    for (index=0; index<RM_NumVisibleLabButtons; index++)
    {
        rmClearResearchlab(universe.curPlayerPtr,index);
        labbuttons[index].selected = FALSE;
    }
}


void rmClearSelectedLab(char *string, featom *atom)
{
    sdword index;
   bool32 halted = FALSE;

    for (index=0; index<RM_NumVisibleLabButtons; index++)
    {
        if ((labbuttons[index].selected) &&
            (labbuttons[index].lab->labstatus==LS_RESEARCHITEM) )
        {
            if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchClearLab)
                return;

            if (!multiPlayerGame)
            {
                rmClearResearchlab(universe.curPlayerPtr,index);
            }
            else
            {
                clWrapResearch(RESEARCH_SUBCOMMAND_STOP, universe.curPlayerIndex, index, 0);
            }
            //labbuttons[index].selected = FALSE;
            halted = TRUE;
        }
    }

   if (halted)
   {
      speechEventFleet(COMM_F_Research_Stop, 0, universe.curPlayerIndex);
   }
}


void rmResearchItem(char *string, featom *atom)
{
    if (rmCurrentSelectionLab != -1 &&
        labbuttons[rmCurrentSelectionLab].lab->labstatus == LS_RESEARCHITEM)
    {
        if ((tutorial != TUTORIAL_ONLY) || tutEnable.bResearchClearLab)
        {
            clWrapResearch(RESEARCH_SUBCOMMAND_STOP, universe.curPlayerIndex, rmCurrentSelectionLab, 0);
            speechEventQueue(NULL, COMM_F_Research_Stop, -1, -1, 0, universe.curPlayerIndex, -1, 5.0f, -1);
        }
    }
}


void rmExitMenu(char *string, featom *atom)
{
    rmRenderEverythingCounter = 0;

    // used to be this
    //feScreenDelete(rmBaseRegion);
    // whoever this caused a memory leak, should use feCurrentScreenDelete to close the current screen
    //
    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchClose)
        return;

    tutGameMessage("RM_Close");
    feCurrentScreenDelete();
    rmBaseRegion = NULL;

    // enable rendering of main game screen
    mrRenderMainScreen = TRUE;

    /* play the exit sound */
    soundEvent(NULL, UI_ManagerExit);
    //restart the sound of space ambient
    soundEvent(NULL, UI_SoundOfSpace);

    spRestoreManagerScreen();

    rmCurTechTexture = -1;
    techinfo = -1;
    rmGUIActive = FALSE;
    rmTechBriefRegion = NULL;
    rmTechImageRegion = NULL;
    rmTechListRegion  = NULL;

    rmLab1Region = NULL;
    rmLab2Region = NULL;
    rmLab3Region = NULL;
    rmLab4Region = NULL;
    rmLab5Region = NULL;
    rmLab6Region = NULL;

    rmConnectorRegion = NULL;

    rmLabConnector1Region = NULL;
    rmLabConnector2Region = NULL;
    rmLabConnector3Region = NULL;
    rmLabConnector4Region = NULL;
    rmLabConnector5Region = NULL;
    rmLabConnector6Region = NULL;

    if (rmOpenedFromSensors)
    {
        smSensorsForceOpen = TRUE;
        cameraCopyPositionInfo(&universe.mainCameraCommand.actualcamera, &smCamera);
        smSensorsBegin(NULL, NULL);
        smSensorsForceOpen = FALSE;
        rmOpenedFromSensors = FALSE;
    }

    lgUpdateChatRegions();
}

void rmCloseIfOpen()
{
    if (rmBaseRegion)
    {
        rmExitMenu(NULL,NULL);
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmMarqueePulse
    Description :
    Inputs      : LabPrintList & region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmMarqueePulse(LabPrintList *labprint, regionhandle region)
{
    sdword count, index;
    sdword x, y;
    rectangle *rect = &region->rect;
    primLine2(rect->x0, rect->y0, rect->x1, rect->y0, rmStandardTextColor);
    primLine2(rect->x0, rect->y1, rect->x1, rect->y1, rmStandardTextColor);

    x = rect->x0 + RM_MARQUEESPACE;
    y = rect->y0 + ((rect->y1 - rect->y0) >> 1);

    if ((labprint->selected) && (labprint->lab->labstatus==LS_RESEARCHITEM))
    {
        for (count=FAST_TO_INT(marqueepos),index=0;index<RM_NUMMARQUEE;index++)
        {
            if (count==0)
            {
                count=2;
                primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeOnColor);
            }else if (count==1)
            {
                primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeSemiOnColor);
                count--;
            }
            else
            {
                primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeOffColor);
                count--;
            }
            x+=RM_MARQUEESPACE;
        }
    }
    else
    {
        for (index=0;index<RM_NUMMARQUEE;index++)
        {
            primCircleSolid2(x,y,RM_MARQUEERAD,20,rmMarqueeOffColor);
            x+=RM_MARQUEESPACE;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmDrawLabButton
    Description : draws lab button
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmDrawLabButton(LabPrintList *labprint, regionhandle region)
{
    color progressColor[4]; // Shaded values
    rectangle rect, progressRect;
    real32 percent, width;
    sdword pos;
    sdword labid = labprint->labid - 1; // zero based
    PlayerResearchInfo *research = &universe.curPlayerPtr->researchinfo;
    sdword iconIndex;
    bool32 labSelected;
    lifheader *lif;
    udword texture;

    rect.x0 = region->rect.x0 + RM_LAB_INSET;
    rect.y0 = region->rect.y0 + RM_LAB_INSET;
    rect.x1 = region->rect.x1 - RM_LAB_INSET;
    rect.y1 = region->rect.y1 - RM_LAB_INSET;

    progressRect.x0 = rect.x0;
    progressRect.y0 = rect.y1 - 10;
    progressRect.x1 = rect.x1;
    progressRect.y1 = rect.y1;

    labSelected = (rmCurrentSelectionLab == labid);
    ferDrawImageBorder((sdword *)region, labSelected);

    switch (labprint->lab->labstatus)
    {
    case LS_NORESEARCHSHIP :
        primRectSolid2(&rect, colRGB(0, 0, 0));                    // clear area
        primGLRectOutline2(&progressRect, 1, rmNoResearchItemColor); // draw empty progress bar
        break;

    case LS_NORESEARCHITEM :
    case LS_RESEARCHITEMSOON :
    case LS_RESEARCHITEM :
        primGLRectOutline2(&progressRect, 1, colRGB(255, 0, 0));

        rect.x0 += 5;
        rect.x1 -= 5;
        rect.y1 = progressRect.y0 - RM_TEXTURE_INSET; // put above the progress bar

        iconIndex = labid + 5;
        if (labSelected)
        {
            iconIndex += 5;
        }
        lif = rmLabButtonImage[iconIndex];
        texture = rmLabButtonTexture[iconIndex];

        if (glcActive())
        {
            glcRectSolidTexturedScaled2(&rect,
                                        lif->width, lif->height,
                                        lif->data,
                                        bitTest(lif->flags, TRF_Paletted) ? lif->palette : NULL,
                                        TRUE);
        }
        else
        {
            if (bitTest(lif->flags, TRF_Paletted))
            {
                trPalettedTextureMakeCurrent(texture, lif->palette);
            }
            else
            {
                trRGBTextureMakeCurrent(texture);
            }

            rndPerspectiveCorrection(FALSE);
            primRectSolidTextured2(&rect);
        }

        if (labprint->lab->labstatus != LS_RESEARCHITEM)
        {
            break;
        }

        percent = 1.0f - (labprint->lab->topic->timeleft /
                          (real32)rmPlayerResearchTimeTable(universe.curPlayerPtr)[labprint->lab->topic->techresearch]);
        if (percent > 0.5f)
        {
            tutGameMessage("Research_ProgressBar");
        }

        // Make the progress rectangle fit within the red border
        progressRect.x0++;
        progressRect.x1--;
        progressRect.y0++;
        progressRect.y1--;

        if (labSelected)
        {
            pos = ((progressRect.x1 - progressRect.x0) * labprint->pulsepos) >> 16;
            pos = progressRect.x0 + pos;

            primLine2(pos, progressRect.y0 + 1, pos, progressRect.y1 - 1, 0);

            if (labprint->pulsepos > FIXED16_ONE)
            {
                labprint->pulsepos = 0;
            }
            else
                labprint->pulsepos += PULSE_INC;

            pos = ((progressRect.x1 - progressRect.x0) * labprint->pulsepos) >> 16;
            pos = progressRect.x0 + pos;

            primLine2(pos, progressRect.y0 + 1, pos, progressRect.y1 - 1, PULSE_COLOR);
        }

        width = ((real32)(progressRect.x1 - progressRect.x0)) * percent;
        progressRect.x1 = progressRect.x0 + FAST_TO_INT(width);

        progressColor[0] = rmProgressDoneColor0;
        progressColor[1] = rmProgressDoneColor0;
        progressColor[2] = rmProgressDoneColor1;
        progressColor[3] = rmProgressDoneColor1;

        primRectShaded2(&progressRect, progressColor);

        break;
    default:
        break;

    }
}

void rmDrawLabIcon(featom *atom, regionhandle region)
{
    rectangle rect;
    Player *player = universe.curPlayerPtr;
    lifheader *lif;
    udword texture;

    rect.x0 = region->rect.x0;
    rect.y0 = region->rect.y0;
    rect.x1 = region->rect.x1;
    rect.y1 = region->rect.y1;

    region->status |= RSF_DrawThisFrame;

    if (player->race == RACE_Beast)
    {
        ResearchLab *lab = rmPlayerResearchLab(player, 0);
        sdword blobFrameIndex = 0;

        rmBeastBlobAnimTime += universe.totaltimeelapsed - rmBeastBlobPrevTime;
        if (rmBeastBlobAnimTime > 0.1f)
        {
            if (rmBeastBlobFrame == 0 && rmResearchingAnything(player))
            {
                soundEventPlay(0, 316, 0);
            }

            rmBeastBlobAnimTime = 0.0f;
            rmBeastBlobFrame++;
            if (rmBeastBlobFrame >= (sdword)(sizeof(rmBeastBlobFrameSequence) / sizeof(rmBeastBlobFrameSequence[0])))
            {
                rmBeastBlobFrame = 0;
            }
        }
        rmBeastBlobPrevTime = universe.totaltimeelapsed;

        if (lab != NULL)
        {
            if (lab->labstatus == LS_RESEARCHITEM)
            {
                blobFrameIndex = rmBeastBlobFrameSequence[rmBeastBlobFrame];
            }
            else if (lab->labstatus == LS_NORESEARCHITEM || lab->labstatus == LS_RESEARCHITEMSOON)
            {
                rmBeastBlobFrame = 0;
            }
        }

        lif = rmBeastBlobImage[blobFrameIndex];
        texture = rmBeastBlobTexture[blobFrameIndex];
    }
    else
    {
        lif = rmLabImage[player->race];
        texture = rmLabTexture[player->race];
    }

    if (lif == NULL)
    {
        return;
    }

    if (glcActive())
    {
        glcRectSolidTexturedScaled2(&rect,
                                    lif->width,
                                    lif->height,
                                    lif->data,
                                    bitTest(lif->flags, TRF_Paletted) ? lif->palette : NULL,
                                    TRUE);
    }
    else
    {
        if (bitTest(lif->flags, TRF_Paletted))
        {
            trPalettedTextureMakeCurrent(texture, lif->palette);
        }
        else
        {
            trRGBTextureMakeCurrent(texture);
        }
        rndPerspectiveCorrection(FALSE);
        primRectSolidTextured2(&rect);
    }
}

//update selection list
void SelectTechWindow(void)
{
    udword         index;
    listitemhandle listitem;

    for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
    {
        if ((PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
            (PlayerTechList[index].itemstat != STAT_CANTRESEARCH))
        {
            if ((            ITEM_VISIBLETECH(PlayerTechList[index].itemtype)) && (PlayerTechList[index].itemID == techinfo))
            {
                listitem = uicListFindItemByData(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index]);
                uicListWindowSetCurItem(rmTechListWindowHandle, listitem);
            }
        }
    }

    /*
    not implemented yet

    listitemhandle  item;

    Node           *walk,*count;
    bool32            found=FALSE;
    listitemhandle  iteminfo, newpos;
    sdword          i, relativecount;

    walk = listwindow->listofitems.head;

    while (walk != NULL)
    {
        listhandle->CurLineSelected->data)->itemID
CurLineSelected == item

        if (item == listGetStructOfNode(walk))
        {
            found = TRUE;
            break;
        }
        walk = walk->next;
    }


    bitClear(rmTechListWindowHandle->CurLineSelected->flags,UICLI_Selected);
    listhandle->CurLineSelected = item;
    bitSet(rmTechListWindowHandle->CurLineSelected->flags,UICLI_Selected);

    */

}

/*-----------------------------------------------------------------------------
    Name        : rmSelectAvailableLab
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSelectAvailableLab(regionhandle region, udword event, sdword labID)
{
    if (event == RPE_PressLeft)
    {                                                       //left press (select/add job)
        dbgAssert(region!=NULL);
#ifdef DEBUG_STOMP
        regVerify(region);
#endif
        bitSet(region->status, RSF_DrawThisFrame);

        if (rmTechImageRegion != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(rmTechImageRegion);
#endif
            rmTechImageRegion->status |= RSF_DrawThisFrame;
        }

        if (labbuttons[labID].lab->labstatus!=LS_NORESEARCHSHIP)
        {
            if(tutorial)
            {
                char gameMessage[128];

                if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchSelectLab)
                    return;

                strcpy(gameMessage, "RM_LabSelect ");
                gameMessage[14] = (char)('0' + labID);
                tutGameMessage(gameMessage);
            }

            rmCurrentSelectionLab = labID;
            techinfo = NumTechnologies + labID;

            if (rmBaseRegion != NULL)
            {
                rmBaseRegion->status |= RSF_ReallyDirty;
            }

            SelectTechWindow();
        }
    }

}

TechPrintList *rmGetCurrentPlayerTechList(void)
{
    if (singlePlayerGame)
    {
        PlayerTechList = &SinglePlayerTechList[0];
    }
    else
    {
        PlayerTechList = (universe.curPlayerPtr->race == RACE_Sect) ? &MultiPlayerSectTechList[0]
                                                                     : &MultiPlayerBeastTechList[0];
    }

    return PlayerTechList;
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab1
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab1(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 0);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab2
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab2(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 1);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab3
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab3(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 2);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab4
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab4(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 3);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab5
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab5(regionhandle region, sdword ID, udword event, udword data)
{
    rmSelectAvailableLab(region, event, 4);
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmSelectLab6
    Description : processes region callback for mouse clicks
    Inputs      : region info
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
sdword rmSelectLab6(regionhandle region, sdword ID, udword event, udword data)
{
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : rmLab1Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab1Draw(featom *atom, regionhandle region)
{
    if (rmRenderEverythingCounter > 0)
    {
        rmRenderEverythingCounter--;
        feRenderEverything = TRUE;
    }

    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab1); //set new region handler function
    }

    rmLab1Region = region;
    rmDrawLabButton(&labbuttons[0], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLab2Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab2Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab2); //set new region handler function
    }

    rmLab2Region = region;
    rmDrawLabButton(&labbuttons[1], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab3Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab3Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab3); //set new region handler function
    }

    rmLab3Region = region;
    rmDrawLabButton(&labbuttons[2], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab4Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab4Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab4); //set new region handler function
    }

    rmLab4Region = region;
    rmDrawLabButton(&labbuttons[3], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab5Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab5Draw(featom *atom, regionhandle region)
{
    if (region->flags == 0 || region->flags == RPE_DrawFunctionAdded)
    {                                         //if region not processed yet
        region->flags = RPE_PressLeft;        //receive mouse presses from now on
        region->flags |= RPE_DrawEveryFrame;
        regFunctionSet(region, (regionfunction)rmSelectLab5); //set new region handler function
    }

    rmLab5Region = region;
    rmDrawLabButton(&labbuttons[4], region);
}


/*-----------------------------------------------------------------------------
    Name        : rmLab6Draw
    Description : draws the lab buttons along with the status of the lab.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLab6Draw(featom *atom, regionhandle region)
{
    rmLab6Region = region;
}


/*-----------------------------------------------------------------------------
    Name        : rmLabConnector1Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector1Draw(featom *atom, regionhandle region)
{
    rmLabConnector1Region = region;
    rmMarqueePulse(&labbuttons[0], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector2Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector2Draw(featom *atom, regionhandle region)
{
    rmLabConnector2Region = region;
    rmMarqueePulse(&labbuttons[1], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector3Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector3Draw(featom *atom, regionhandle region)
{
    rmLabConnector3Region = region;
    rmMarqueePulse(&labbuttons[2], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector4Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector4Draw(featom *atom, regionhandle region)
{
    rmLabConnector4Region = region;
    rmMarqueePulse(&labbuttons[3], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector5Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector5Draw(featom *atom, regionhandle region)
{
    rmLabConnector5Region = region;
    rmMarqueePulse(&labbuttons[4], region);
}

/*-----------------------------------------------------------------------------
    Name        : rmLabConnector6Draw
    Description : draws the lab connector between the status box and diagram.
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmLabConnector6Draw(featom *atom, regionhandle region)
{
    rmLabConnector6Region = region;
    rmMarqueePulse(&labbuttons[5], region);
}

char *getWord(char *dest, char *source)
{
    sdword index=strlen(dest);

    while ( (source[0] == ' ') || (source[0] == '-') )
    {
        dest[index] = source[0];
        dest[index+1] = 0;
        index++;
        source++;
    }

    while ( (source[0] != ' ') && (source[0] != '-') && (source[0] != 0) && (source[0] != '\n') )
    {
        dest[index] = source[0];
        dest[index+1] = 0;
        index++;
        source++;
    }

    return(source);
}

char *getShipBuild(char *dest)
{
    Player               *player = universe.curPlayerPtr;
    PlayerShipPrereqMask *discoveredTable = player->discoveredShipPrereqs;
    sdword               *currentPrereqMask = rmPlayerCurrentPrereqMask(player);
    sdword                result[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    sdword                researchMask[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    ShipType              shipType;
    bool32                first = TRUE;

    memcpy(researchMask, currentPrereqMask, sizeof(researchMask));
    rmActivateTech((udword *)researchMask, techinfo);

    for (shipType = FirstShipTypeOfRace[player->race];
         shipType <= LastShipTypeOfRace[player->race];
         shipType++)
    {
        rmPreRequisitesAnd(result, discoveredTable[shipType].value, researchMask);
        if (rmTechDependenciesEqual(result, discoveredTable[shipType].value))
        {
            rmPreRequisitesAnd(result, currentPrereqMask, discoveredTable[shipType].value);
            if (!rmTechDependenciesEqual(result, discoveredTable[shipType].value) &&
                player->race == RACE_Kushan &&
                shipType != tDFGFrigate)
            {
                if (!first)
                {
                    strcat(dest, ", ");
                }
                else
                {
                    strcat(dest, "\n\n");
                    strcat(dest, strGetString(strCanBuild));
                    strcat(dest, "\n");
                }
                strcat(dest, " ");
                strcat(dest, ShipTypeToNiceStr(shipType));
                first = FALSE;
            }
        }
    }

    return (dest);
}

/*-----------------------------------------------------------------------------
    Name        : rmTechTexturePrepare
    Description : prepares texture for use in the research manager.
    Inputs      : index into pictures structure.
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmTechTexturePrepare(sdword index)
{
    if (pictures[index].techTexture == TR_InvalidInternalHandle)
    {
        pictures[index].techTexture = trPalettedTextureCreate(pictures[index].techImage->data, pictures[index].techImage->palette, pictures[index].techImage->width, pictures[index].techImage->height);
        rmCurTexture = pictures[index].techTexture;
        rmCurIndex = index;
    }
    else
    {
        trPalettedTextureMakeCurrent(pictures[index].techTexture, pictures[index].techImage->palette);
        rmCurTexture = pictures[index].techTexture;
        rmCurIndex = index;
    }
}


/*-----------------------------------------------------------------------------
    Name        : rmExtendedInfo
    Description :
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmExtendedInfo(char *string, featom *atom)
{
    rmExtendedInfoActive = !rmExtendedInfoActive;

    if (rmTechImageRegion != NULL)
    {
#ifdef DEBUG_STOMP
        regVerify(rmTechImageRegion);
#endif
        rmTechImageRegion->status |= RSF_DrawThisFrame;
    }
}


/*-----------------------------------------------------------------------------
    Name        : rmTechInfoDraw
    Description : Draws technology text to the window.
    Inputs      : region
    Outputs     : technology info.
    Return      : void
----------------------------------------------------------------------------*/
void rmTechInfoDraw(regionhandle region)
{
    fonthandle  currentFont;
    sdword      x,y, width, lineHeight, infoWidth;
    char       *pos, *oldpos;
    char        oldline[100], line[100];
    char        stringtoprint[650];
    bool32        justified, done;

    //rmTechInfoRegion = region;

    //feStaticRectangleDraw(region); //draw standard rectangle

    //currentFont = fontMakeCurrent(rmTechListFont);
    currentFont = fontMakeCurrent(rmTechInfoFont);

    x = region->rect.x0 + 15;
    lineHeight = fontHeight(" ");
    y = region->rect.y0 + lineHeight + (lineHeight >> 1) + 16;
    infoWidth = region->rect.x1 - region->rect.x0 - 15;

    if (techinfo != -1)
    {
        if (techinfo < NumTechnologies)
        {
            if (universe.curPlayerPtr->race == RACE_Sect)
            {
                strcpy(stringtoprint, strGetString(techinfo + strTechInfoOffsetR1));
            }
            else
            {
                strcpy(stringtoprint, strGetString(techinfo + strTechInfoOffsetR2));
            }
            getShipBuild(stringtoprint);
        }
        else
        {
            strcpy(stringtoprint, strGetString(techinfo + strTechInfoOffsetR2));
        }

        pos = stringtoprint;

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    if ((width = fontWidth(line)) > infoWidth)
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while (pos[0] == ' ') pos++;
                        justified = TRUE;
                    }
                    else
                    {
                        justified = TRUE;
                        pos++;
                        while ( pos[0] == ' ' ) pos++;
                    }
                }
                else
                {
                    if ((width = fontWidth(line)) > infoWidth)
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while ( pos[0] == ' ' ) pos++;

                        justified = TRUE;
                    }
                    if (pos[0]==0)
                    {
                        justified = TRUE;
                        done      = TRUE;
                    }
                }
            }

            fontPrintf(x,y,rmTechInfoTextColor,"%s",line);
            y += lineHeight;
            if (y > region->rect.y1 + lineHeight) done=TRUE;
        }
    }

    fontMakeCurrent(currentFont);
}


/*-----------------------------------------------------------------------------
    Name        : rmTechBriefDraw
    Description :
    Inputs      : region
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmTechBriefDraw(featom *atom, regionhandle region)
{
    fonthandle  currentFont;
    sdword      x,y;
    rectangle   rect;
    //char       *pos, *oldpos;
    //char        oldline[100], line[100];
    //char        stringtoprint[650];
    //bool32        justified, done;

    rmTechBriefRegion = region;
    rect = region->rect;

    feStaticRectangleDraw(region); //draw standard rectangle
    rect.x0 += 2;
    rect.y0 += 2;
    rect.x1 -= 2;
    rect.y1 -= 2;
    primRectSolid2(&rect, atom->contentColor);

    currentFont = fontMakeCurrent(rmTechListFont);
    //currentFont = fontMakeCurrent(rmTechInfoFont);

    x = region->rect.x0 + 15;
    y = region->rect.y0 + 5;

    if (techinfo != -1)
    {
        fontPrintf(x,y,FEC_ListItemStandard/*rmResearchingTextColor*/,"%s",TechTypeToNiceString(techinfo));

        /*
        y += RM_VertSpacing + fontHeight(" ");

        //fontMakeCurrent(rmTechInfoFont);

        if (universe.curPlayerPtr->race==RACE_Sect)
        {
            dbgAssert(strGetString(techinfo+strTechInfoOffsetR1)!=NULL);
            strcpy(stringtoprint,strGetString(techinfo+strTechInfoOffsetR1));
        }
        else
        {
            dbgAssert(strGetString(techinfo+strTechInfoOffsetR2)!=NULL);
            strcpy(stringtoprint,strGetString(techinfo+strTechInfoOffsetR2));
        }

        getShipBuild(stringtoprint);

        pos = stringtoprint;

        done = FALSE;
        while (!done)
        {
            justified = FALSE;
            line[0]=0;
            while (!justified)
            {
                strcpy(oldline, line);
                oldpos = pos;
                pos = getWord(line, pos);

                if (pos[0] == '\n')
                {
                    justified = TRUE;
                    pos++;
                    while ( pos[0] == ' ' ) pos++;
                }
                else
                {
                    if ( (width=fontWidth(line)) > RM_InfoWidth - 15)
                    {
                        strcpy(line, oldline);
                        pos = oldpos;
                        while ( pos[0] == ' ' ) pos++;

                        justified = TRUE;
                    }
                    if (pos[0]==0)
                    {
                        justified = TRUE;
                        done      = TRUE;
                    }
                }
            }

            fontPrintf(x,y,rmStandardTextColor,"%s",line);
            y += fontHeight(" ");
            if (y > region->rect.y1 + fontHeight(" ")) done=TRUE;
        }
        */
    }

    fontMakeCurrent(currentFont);
}

/*-----------------------------------------------------------------------------
    Name        : rmTechImageDraw
    Description : Loads in, decompresses and draws the tech picture.
    Inputs      : none
    Outputs     : loads in rmTechImage and creates rmTechTexture
    Return      : void
----------------------------------------------------------------------------*/
void rmTechImageDraw(featom *atom, regionhandle region)
{
    char      filename[128];
    sdword    index, lru;
    real32    time=(real32)1.0e22;
    rectangle textureRect;
    fonthandle oldFont;

    rmTechImageRegion = region;

    feStaticRectangleDraw(region); //draw standard rectangle

    if (techinfo != -1)
    {
        if (techinfo != rmCurTechTexture)
        {
            for (index=0;index<RM_TOTALPICS;index++) // RM_TOTALPICS is the size of the cache
            {
                // Find least recently used texture
                if (pictures[index].timestamp < time)
                {
                    time = pictures[index].timestamp;
                    lru  = index;
                }
                // If already cached, use it and exit this routine
                if ( (pictures[index].tech==techinfo) &&
                     (pictures[index].race==universe.curPlayerPtr->race) )
                {
                    rmTechTexturePrepare(index);
                    rmCurTechTexture = techinfo;
                    rmCurTechImage = pictures[index].techImage;
                    rmCurIndex = index;
                    pictures[index].timestamp = universe.totaltimeelapsed;
                    goto rmDrawTechImage;
                }
            }

            // Build filename for loading texture from file
            strcpy(filename, TechImagePaths[universe.curPlayerPtr->race]);
            strcat(filename, rmTechImageFileStem(techinfo));
            strcat(filename,".lif");

            // Remove oldest (least recently used) texture from memory
            if (pictures[lru].techImage != NULL)
            {
                memFree(pictures[lru].techImage);
                pictures[lru].techImage = NULL;
            }
            if (pictures[lru].techTexture != TR_InvalidInternalHandle)
            {
                trRGBTextureDelete(pictures[lru].techTexture);
                pictures[lru].techTexture = TR_InvalidInternalHandle;
            }

            // Load the image into LRU cache
            pictures[lru].techImage = trLIFFileLoad(filename, NonVolatile);
            dbgAssert(pictures[lru].techImage->flags & TRF_Paletted);

            rmTechTexturePrepare(lru);
            rmCurTechTexture = techinfo;
            rmCurTechImage = pictures[lru].techImage;
            pictures[lru].tech = techinfo;
            pictures[lru].race = universe.curPlayerPtr->race;
            pictures[lru].timestamp = universe.totaltimeelapsed;
        }
        else
        {
            trPalettedTextureMakeCurrent(pictures[rmCurIndex].techTexture,
                                         pictures[rmCurIndex].techImage->palette);
        }

rmDrawTechImage:
        textureRect.x0 = region->rect.x0 + RM_TEXTURE_INSET;
        textureRect.y0 = region->rect.y0 + RM_TEXTURE_INSET;
        textureRect.x1 = region->rect.x1 - RM_TEXTURE_INSET;
        textureRect.y1 = region->rect.y1 - RM_TEXTURE_INSET;

        rndPerspectiveCorrection(FALSE);
        if (glcActive())
        {
            glcRectSolidTextured2(&textureRect,
                                  pictures[rmCurIndex].techImage->width,
                                  pictures[rmCurIndex].techImage->height,
                                  pictures[rmCurIndex].techImage->data,
                                  pictures[rmCurIndex].techImage->palette,
                                  TRUE);
        }
        else
        {
            primRectSolidTextured2(&textureRect);
        }

        if (rmExtendedInfoActive)
        {
            primRectTranslucent2(&textureRect, colRGBA(0, 0, 0, 128));
            rmTechInfoDraw(region);
        }

        if ((udword)techinfo <= TechDriveCorvette)
        {
            oldFont = fontMakeCurrent(rmTechInfoFont);
            fontPrint(region->rect.x0 + 15,
                      region->rect.y0 + 11,
                      rmClassHeadingTextColor,
                      RaceSpecificTechTypeToNiceString((TechnologyType)techinfo, universe.curPlayerPtr->race));
            fontMakeCurrent(oldFont);
        }

    }
}


/*-----------------------------------------------------------------------------
    Name        : rmConnectorDraw
    Description : Draws the connector between the picture of the technology and
                  the info. text
    Inputs      : region
    Outputs     : connector image
    Return      : void
----------------------------------------------------------------------------*/
void rmConnectorDraw(featom *atom, regionhandle region)
{
    rmConnectorRegion = region;

    if (techinfo != -1)
    {
        sdword centerX  = region->rect.x0 + ((region->rect.x1 - region->rect.x0) / 2);

        primLine2(centerX, region->rect.y0, centerX, region->rect.y1, colRGB(255, 0, 0));
        primCircleSolid2(centerX, region->rect.y1, 3, 16, colRGB(255, 0, 0));
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmDrawTechListItem
    Description : This function draws each item in the technology list.
    Inputs      : rectangle, and a pointer to the item handle.
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void rmDrawTechListItem(rectangle *rect, listitemhandle data)
{
    TechPrintList *techprint=(TechPrintList *)data->data;
    sdword              x, y;
    color               c;
    Player             *player=universe.curPlayerPtr;
    PlayerResearchInfo *research=&universe.curPlayerPtr->researchinfo;
    ResearchTopic      *topic;
    TechnologyType      tech;
    char                temp[64];
    real32              percent;
    sdword             *timeTable;
    rectangle           bar;
    fonthandle          oldfont;
    color               progressColor[4];

    x = rect->x0;
    y = rect->y0;

    oldfont = fontMakeCurrent(rmTechListFont);

    if (ITEM_VISIBLETECH(techprint->itemtype))
    {
        tech = techprint->itemID;
        timeTable = rmPlayerResearchTimeTable(player);

        sprintf(temp, "%s", RaceSpecificTechTypeToNiceString(tech, player->race));

        switch (techprint->itemstat)
        {
            case STAT_CANRESEARCH :
                x += RM_HorzSpacing<<2;
                c = (techprint->itemtype == ITEM_MODULETECH) ? FEC_ListItemStandard : rmCanResearchShipTextColor;
            break;
            case STAT_CANTRESEARCH :
                x += RM_HorzSpacing<<2;
                c = rmCantResearchTextColor;
            break;
            case STAT_RESEARCHING :
                x += RM_HorzSpacing<<2;
                if ((topic=Researching(player,tech))!=NULL)
                {
                    percent = (real32)((real32)topic->timeleft / (real32)timeTable[tech]);
                    bar.x0 = x-2;
                    bar.y0 = y/*-2*/;
                    bar.x1 = rect->x1-2;
                    bar.y1 = y+fontHeight(" ") + 2;
                    primRectSolid2(&bar, rmProgressToGoColor);
                    bar.x1 -= (udword)((real32)( (rect->x1-2) - (x-2))*percent);

                    progressColor[0] = rmProgressDoneColor0;
                    progressColor[1] = rmProgressDoneColor0;
                    progressColor[2] = rmProgressDoneColor1;
                    progressColor[3] = rmProgressDoneColor1;

                    primRectShaded2(&bar, progressColor);

#ifdef DEBUG_STOMP
                    regVerify((regionhandle)&rmTechListWindowHandle->reg);
#endif
                    rmTechListWindowHandle->reg.status |= RSF_DrawThisFrame;
                }
                c = rmResearchingTextColor;
            break;
            case STAT_ALREADYHAVE :
                x += RM_HorzSpacing<<2;
                c = (techprint->itemtype == ITEM_MODULETECH) ? FEC_ListItemInactive : rmAlreadyHaveShipTextColor;
                primCircleSolid2(x-7,y+5,3,20,rmResearchingTextColor);
            break;
            default:
                x += RM_HorzSpacing<<2;
                c = FEC_ListItemStandard;
            break;
        }
        if (bitTest(data->flags,UICLI_Selected))
        {
        if (techprint->itemstat == STAT_RESEARCHING)
            {
            c = rmResearchingTextColor;//rmSelectionTextColor;
            }
        else if (techprint->itemtype == ITEM_MODULETECH)
        {
            c = FEC_ListItemStandard;
        }
        else
        {
            c = rmCanResearchShipTextColor;
        }
        }
    }
    else if ( (techprint->itemtype==ITEM_CLASSHEADER) &&
              (techprint->itemstat==STAT_CANPRINT) )
    {
        dbgAssert(techprint->itemID >= 0);
        dbgAssert(techprint->itemID < (sdword)(sizeof(rmTechHeadingStrings) / sizeof(rmTechHeadingStrings[0])));
        sprintf(temp, "%s", strGetString(rmTechHeadingStrings[techprint->itemID]));
        c = rmClassHeadingTextColor;
    }

    fontPrintf(x,y,c,"%s",temp);

    fontMakeCurrent(oldfont);

    marqueepos += (real32)((taskTimeElapsed - marqueetime) * RM_MARQUEERATE);
    if (marqueepos > (real32)(RM_NUMMARQUEE / 3) + 1.0)
    {
        marqueepos -= (real32)(RM_NUMMARQUEE / 3) + 1.0;
    }
    marqueetime = taskTimeElapsed;
}


/*-----------------------------------------------------------------------------
    Name        : rmTechListWindow
    Description : This function handle messages from the technology listwindow.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmTechListWindow(char *string, featom *atom)
{
    fonthandle oldfont;
    sdword     index;

    if (FEFIRSTCALL(atom))
    {
        oldfont = fontMakeCurrent(rmTechListFont);
        rmTechListWindowHandle = (listwindowhandle)atom->pData;

        uicListWindowInit(rmTechListWindowHandle,
                          NULL,         //  title draw, no title
                          NULL,         //  title click process, no title
                          0,            //  title height, no title
                          rmDrawTechListItem,           // item draw funtcion
                          fontHeight(" ")+RM_VertSpacing, // item height
                          UICLW_CanSelect);

        for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
        {
            if ((PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
                (PlayerTechList[index].itemstat != STAT_CANTRESEARCH))
            {
                if (PlayerTechList[index].itemtype == ITEM_CLASSHEADER)
                {
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], 0, UICLW_AddToTail);
                }
                else if (ITEM_VISIBLETECH(PlayerTechList[index].itemtype))
                {
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], UICLI_CanSelect, UICLW_AddToTail);
                }
            }
        }
        return;
    } else if (FELASTCALL(atom))
    {
        rmTechListWindowHandle = NULL;
        return;
    }


    switch (rmTechListWindowHandle->message)
    {
        case CM_NewItemSelected:
        {
            TechPrintList *selectedItem;

            if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchSelectTech)
                return;

            selectedItem = (TechPrintList *)rmTechListWindowHandle->CurLineSelected->data;
            techinfo = selectedItem->itemID;
            rmCurrentSelectionLab = selectedItem->labnum;
            rmDirtyTechInfo();
            if (rmBaseRegion != NULL)
            {
                rmBaseRegion->status |= RSF_ReallyDirty;
            }

            if(tutorial)
            {
                char gameMessage[128];

                strcpy(gameMessage, "RM_TechSelect_");
                strcat(gameMessage, rmTechImageFileStem(techinfo));
                tutGameMessage(gameMessage);
            }
        }
        break;

        case CM_DoubleClick:

            if (tutorial)
            {
                tutGameMessage("RM_DoubleClick");
            }
            rmResearchCurrentSelection();
        break;



    }
}

/*-----------------------------------------------------------------------------
    Name        : rmUpdateTechList
    Description : updates the status of the technology list.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmUpdateTechList(void)
{
    PlayerResearchInfo *research=&universe.curPlayerPtr->researchinfo;
    PlayerShipPrereqMask *discoveredTable = universe.curPlayerPtr->discoveredShipPrereqs;
    sdword *currentPrereqMask = rmPlayerCurrentPrereqMask(universe.curPlayerPtr);
    sdword prereqResult[PLAYER_SHIP_PREREQ_ENTRY_COUNT];
    TechnologyType tech;
    sdword index, canprint=STAT_CANTPRINT, count=0;
    bool32 labAvailable;
//    ubyte *data=NULL;

    for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
    {
        if (ITEM_VISIBLETECH(PlayerTechList[index].itemtype))
        {
            tech    = PlayerTechList[index].itemID;
            labAvailable = (PlayerTechList[index].labnum >= 0) &&
                           (PlayerTechList[index].labnum < RM_NumVisibleLabButtons) &&
                           (labbuttons[PlayerTechList[index].labnum].lab->labstatus != LS_NORESEARCHSHIP);

            if (currentPrereqMask[PLAYER_SHIP_PREREQ_STATE_INDEX] == TECH_ALLTECHNOLOGY || currentPrereqMask[tech] == 1)
            {
                PlayerTechList[index].itemstat = STAT_ALREADYHAVE;
            }
            else if (Researching(universe.curPlayerPtr,tech)!=NULL)
            {
                PlayerTechList[index].itemstat = STAT_RESEARCHING;
            }
            else if (tutorial == TUTORIAL_ONLY)
            {
                PlayerTechList[index].itemstat = tmTechResearchable[tech] ? STAT_CANRESEARCH : STAT_CANTRESEARCH;
            }
            else if (singlePlayerGame)
            {
                if (tmTechResearchable[tech] && labAvailable)
                {
                    PlayerTechList[index].itemstat = STAT_CANRESEARCH;
                }
                else
                {
                    PlayerTechList[index].itemstat = STAT_CANTRESEARCH;
                }
            }
            else
            {
                if (labAvailable &&
                    discoveredTable != NULL &&
                    rmTechDependenciesEqual(
                        rmPreRequisitesAnd(
                            prereqResult,
                            currentPrereqMask,
                            discoveredTable[TOTAL_NUM_SHIPS + tech].value),
                        discoveredTable[TOTAL_NUM_SHIPS + tech].value) &&
                    !(tech == TechCrystalHarvest && !bitTest(tpGameCreated.flag, MG_HarvestinEnabled)))
                {
                    PlayerTechList[index].itemstat = STAT_CANRESEARCH;
                }
                else
                {
                    PlayerTechList[index].itemstat = STAT_CANTRESEARCH;
                }
            }
        }
        else if (PlayerTechList[index].itemtype == ITEM_TECHHIDDEN)
        {
            PlayerTechList[index].itemstat = STAT_CANTRESEARCH;
        }
    }

    for (; index >= 0; index--)
    {
        if (ITEM_VISIBLETECH(PlayerTechList[index].itemtype) &&
            (PlayerTechList[index].itemstat != STAT_CANTPRINT) &&
            (PlayerTechList[index].itemstat != STAT_CANTRESEARCH))
        {
            canprint=STAT_CANPRINT;
        }
        else if (PlayerTechList[index].itemtype==ITEM_CLASSHEADER)
        {
            PlayerTechList[index].itemstat = (sword)canprint;

            canprint=STAT_CANTPRINT;
        }
        if (PlayerTechList[index].itemstat != STAT_CANTPRINT &&
            PlayerTechList[index].itemstat != STAT_CANTRESEARCH)
        {
            count++;
        }
    }
    count--;

    // Regenerate list because something has just finished being researched

    if ( (rmTechListWindowHandle!=NULL) &&
         (count != rmTechListWindowHandle->ListTotal))
    {
        uicListCleanUp(rmTechListWindowHandle);
        for (index=0;PlayerTechList[index].itemtype!=ITEM_ENDLIST; index++)
        {
            if (PlayerTechList[index].itemstat != STAT_CANTPRINT &&
                PlayerTechList[index].itemstat != STAT_CANTRESEARCH)
            {
                if (PlayerTechList[index].itemtype == ITEM_CLASSHEADER)
                {
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], 0, UICLW_AddToTail);
                }
                else if (ITEM_VISIBLETECH(PlayerTechList[index].itemtype))
                {
                    uicListAddItem(rmTechListWindowHandle, (ubyte *)&PlayerTechList[index], UICLI_CanSelect, UICLW_AddToTail);
                }
            }
        }

        SelectTechWindow();

        rmDirtyTechList();
    }

    if (rmTechListWindowHandle != NULL)
    {
    #ifdef DEBUG_STOMP
        regVerify((regionhandle)&rmTechListWindowHandle->reg);
    #endif
        rmTechListWindowHandle->reg.status |= RSF_DrawThisFrame;
    }

    rmDirtyTechList();
}


/*-----------------------------------------------------------------------------
    Name        : rmResearchBegin
    Description : brings up research manager GUI
    Inputs      : standard region event handles
    Outputs     : success fail
    Return      :
----------------------------------------------------------------------------*/
sdword rmResearchGUIBegin(regionhandle region, sdword ID, udword event, udword data)
{
    sdword index;
    sdword lru;
    real32 time;
    char filename[128];

    if ((playPackets) || (universePause)) return 0;

    if ((universe.curPlayerPtr == NULL) ||
        (universe.curPlayerPtr->PlayerMothership == NULL) ||
        (!universe.curPlayerPtr->PlayerMothership->staticinfo->canBuildShips) ||
        (singlePlayerGame && universe.curPlayerPtr->playerIndex != 0))
    {
        return 0;
    }

    if (smSensorsActive)
    {
        rmOpenedFromSensors = TRUE;
        smSensorsCloseForGood();
    }
    else
    {
        rmOpenedFromSensors = FALSE;
    }

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bResearchManager)
        return (0);

    // disable rendering of main screen
    mrRenderMainScreen = FALSE;

    rmRenderEverythingCounter = (tutorial == TUTORIAL_ONLY) ? 4 : 0;

    // clear the screen
    rndClear();

    mrManagerScreenBegin(SPLOCKOUT_MR);

    //mouseEnable();
    //mouseCursorShow();

    if (!rmScreensHandle)
    {
        feCallbackAddMultiple(rmCallback);                  //add in the callbacks
        feDrawCallbackAddMultiple(rmDrawCallback);
        rmScreensRace = -1;
    }

    if (rmScreensRace != universe.curPlayerPtr->race)
    {
        if (rmScreensHandle)
        {
            feScreensDelete(rmScreensHandle);
        }

        rmScreensHandle = feScreensLoad((universe.curPlayerPtr->race == RACE_Sect) ? RM_FIBFile : RM_BeastFIBFile);
        rmScreensRace = universe.curPlayerPtr->race;
    }

    PlayerTechList = rmGetCurrentPlayerTechList();

    for (index=0;index<RM_NumVisibleLabButtons;index++)
    {
        labbuttons[index].lab = rmPlayerResearchLab(universe.curPlayerPtr, index);
        labbuttons[index].labid = (uword) (index+1);
        labbuttons[index].pulsepos = 0;
    }

    soundEventStopSFX(0.5f);

    /* play the intro sound */
    soundEvent(NULL, UI_ManagerIntro);
    /* start the research ambient sound */
    soundEvent(NULL, UI_ResearchManager);

    rmUpdateTechList();

    rmBaseRegion = feScreenStart(region,
                                 (universe.curPlayerPtr->race == RACE_Sect) ? RM_ResearchScreen : RM_BeastResearchScreen);  //add new regions as siblings of current one

    time = (real32)1.0e22;
    lru = 0;
    strcpy(filename, TechImagePaths[universe.curPlayerPtr->race]);
    strcat(filename, rmDefaultResearchImageName);

    for (index=0; index<RM_TOTALPICS; index++)
    {
        if (pictures[index].timestamp < time)
        {
            time = pictures[index].timestamp;
            lru = index;
        }
        if ((pictures[index].tech == techinfo) &&
            (pictures[index].race == universe.curPlayerPtr->race))
        {
            if (pictures[index].techTexture == TR_InvalidInternalHandle)
            {
                pictures[index].techTexture = trPalettedTextureCreate(
                    pictures[index].techImage->data,
                    pictures[index].techImage->palette,
                    pictures[index].techImage->width,
                    pictures[index].techImage->height);
            }
            else
            {
                trPalettedTextureMakeCurrent(pictures[index].techTexture,
                                             pictures[index].techImage->palette);
            }
            rmCurTexture = pictures[index].techTexture;
            rmCurTechTexture = techinfo;
            rmCurTechImage = pictures[index].techImage;
            rmCurIndex = index;
            pictures[index].timestamp = universe.phystimeelapsed;
            goto rmDefaultImageReady;
        }
    }

    if (pictures[lru].techImage != NULL)
    {
        memFree(pictures[lru].techImage);
        pictures[lru].techImage = NULL;
    }
    if (pictures[lru].techTexture != TR_InvalidInternalHandle)
    {
        trRGBTextureDelete(pictures[lru].techTexture);
        pictures[lru].techTexture = TR_InvalidInternalHandle;
    }

    pictures[lru].techImage = trLIFFileLoad(filename, NonVolatile);
    if (pictures[lru].techTexture == TR_InvalidInternalHandle)
    {
        pictures[lru].techTexture = trPalettedTextureCreate(
            pictures[lru].techImage->data,
            pictures[lru].techImage->palette,
            pictures[lru].techImage->width,
            pictures[lru].techImage->height);
    }
    else
    {
        trPalettedTextureMakeCurrent(pictures[lru].techTexture,
                                     pictures[lru].techImage->palette);
    }
    rmCurTexture = pictures[lru].techTexture;
    rmCurTechTexture = techinfo;
    rmCurTechImage = pictures[lru].techImage;
    rmCurIndex = lru;
    pictures[lru].tech = techinfo;
    pictures[lru].race = universe.curPlayerPtr->race;
    pictures[lru].timestamp = universe.phystimeelapsed;

rmDefaultImageReady:
    rmGUIActive = TRUE;

    tutGameMessage("Start_ResearchManager");

    mouseCursorShow();

//    rmExtendedInfoActive = FALSE;

    mouseCursorShow();

    ioDisable();

    return (0);
}

/*-----------------------------------------------------------------------------
    Name        : rmGUIStartup
    Description : Initializes the Research Manager GUI resources.
    Inputs      : none
    Outputs     : none
    Return      : void
-----------------------------------------------------------------------------*/
void rmGUIStartup(void)
{
    mainTexturesInit();
}

/*-----------------------------------------------------------------------------
    Name        : ShipsNeedTech
    Description : rerturns true if a ship needs the technology specified.
    Inputs      : techstatics
    Outputs     : TRUE/FALSE
    Return      : bool32
----------------------------------------------------------------------------*/
bool32 ShipsNeedTech(TechStatics *techstat, TechnologyType tech)
{
    sdword index;

    for (index=0; index<TOTAL_NUM_SHIPS; index++)
    {
        if (techstat->TechNeededToBuildShip[index]&TechToBit(tech))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : rmSetPrintList
    Description : Sets the print lists for single and multiplayer after the dependancies are loaded;
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmSetPrintList(udword whichlist, TechStatics *techstat)
{
    sdword index, copycount;
    TechPrintList *outputList = NULL;
    TechPrintList *sourceList = NULL;

    (void)techstat;

    switch (whichlist)
    {
        case RM_SP:
            outputList = SinglePlayerTechList;
            sourceList = PlayerTechListStaticR1;
            break;
        case RM_MPS:
            outputList = MultiPlayerSectTechList;
            sourceList = PlayerTechListStaticR1;
            break;
        case RM_MPB:
            outputList = MultiPlayerBeastTechList;
            sourceList = PlayerTechListStaticBeast;
            break;
    }

    dbgAssert(outputList != NULL);
    dbgAssert(sourceList != NULL);

    for (copycount = 0, index = 0; index < NUMTECHSINLIST; index++)
    {
        outputList[copycount] = sourceList[index];
        copycount++;
        if (sourceList[index].itemtype == ITEM_ENDLIST)
        {
            break;
        }
    }

    if (copycount == 0 || outputList[copycount - 1].itemtype != ITEM_ENDLIST)
    {
        outputList[copycount].itemtype = ITEM_ENDLIST;
        outputList[copycount].itemstat = 0;
        outputList[copycount].itemID = 0;
        outputList[copycount].labnum = 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : rmGUIShutdown
    Description : shuts down the research managers GUI
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void rmGUIShutdown(void)
{
    sdword index;

    if (rmScreensHandle)
    {
        feScreensDelete(rmScreensHandle);
        rmScreensHandle = NULL;
    }

    for (index=0;index<RM_TOTALPICS;index++)
    {
        if (pictures[index].techImage != NULL)
        {
            memFree(pictures[index].techImage);
            pictures[index].techImage = NULL;
        }

        if (pictures[index].techTexture != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(pictures[index].techTexture);
            pictures[index].techTexture = TR_InvalidInternalHandle;
        }
    }

    for(index=0;index<MAX_RACES;index++)
    {
        if (rmLabImage[index] != NULL)
        {
            memFree(rmLabImage[index]);
            rmLabImage[index] = NULL;
        }
        if (rmLabTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(rmLabTexture[index]);
            rmLabTexture[index] = TR_InvalidInternalHandle;
        }
    }

    for (index = 0; index < RM_NumLabButtonTextures; index++)
    {
        if (rmLabButtonImage[index] != NULL)
        {
            memFree(rmLabButtonImage[index]);
            rmLabButtonImage[index] = NULL;
        }
        if (rmLabButtonTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(rmLabButtonTexture[index]);
            rmLabButtonTexture[index] = TR_InvalidInternalHandle;
        }
    }

    for (index = 0; index < (sdword)(sizeof(rmBeastBlobImagePaths) / sizeof(rmBeastBlobImagePaths[0])); index++)
    {
        if (rmBeastBlobImage[index] != NULL)
        {
            memFree(rmBeastBlobImage[index]);
            rmBeastBlobImage[index] = NULL;
        }
        if (rmBeastBlobTexture[index] != TR_InvalidInternalHandle)
        {
            trRGBTextureDelete(rmBeastBlobTexture[index]);
            rmBeastBlobTexture[index] = TR_InvalidInternalHandle;
        }
    }
}
