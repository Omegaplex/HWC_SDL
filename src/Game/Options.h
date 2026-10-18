
/*=============================================================================
    Name    : options.h
    Purpose : Definitons for the options

    Created  11/05/1998 by yo
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#ifndef ___OPTIONS_H
#define ___OPTIONS_H

#include "types.h"
#include "liloptions.h"
#include "region.h"

typedef struct tagfeatom featom;

/*=============================================================================
    Definitions:
=============================================================================*/
#define OP_KEYMAPSIZE 80
    //4 chars needed per key

#define OP_TroubleShootPage "http://www.sierrastudios.com/games/homeworld/noflash/c-support.html"

#define OP_ResolutionFont "Arial_12.hff"
#define OP_RenderFont "Arial_12.hff"

#define OP_ResolutionSelectedColor  colRGB(255,255,105)
#define OP_ResolutionColor          colRGB(240,240,240)

#define OP_RenderSelectedColor      colRGB(255,255,105)
#define OP_RenderColor              colRGB(240,240,240)

#define OP_DimRenderColor     colRGB(192,192,192)
#define OP_RenderColorOutline colRGB(120,70,0)

extern uword opKeyDetour;
extern char opKeymap[OP_KEYMAPSIZE];

extern bool32 opReloading;
extern sdword opNoPalMB;
extern udword opAutoSaveInterval;
extern sdword opMusicVol;
extern sdword opSFXVol;
extern sdword opSpeechVol;
extern sdword opSpeakerSe;
extern sdword opEqualizer;
extern sdword opSpeakerSetting;
extern sdword opVoice0On;
extern sdword opVoice1On;
extern sdword opVoice2On;
extern sdword opVoiceComm;
extern sdword opVoiceStat;
extern sdword opVoiceChat;
extern sdword opMouseSens;
extern sdword opInfoOverlayVar;
extern sdword opBattleChatter;
extern sdword opEffectsVal;
extern sdword opBrightnessVal;
extern udword opDetailThresholdVal;
extern bool32 opCustomEffectsToggled;
extern sdword opNoLODVal;
extern sdword opNumChannels;
extern sdword opAutoChannel;
extern sdword opSoundQuality;
extern sdword opHardwareTransformLighting;
extern sdword DetailQuality;
extern sdword GameFramerateOption;
extern sdword largePopupMenuFonts;
extern sdword homeworldCSA;
extern sdword rightClickCSA;
extern sdword showTaskbarBackdrop;
extern sdword showTaskbarButtons;
extern sdword experienceMessages;
extern sdword showDust;
extern sdword showResources;
extern sdword showFOWBlobs;
extern sdword homeworldTaskbar;
extern sdword showSensorBackgrounds;
extern sdword showAreasVisited;
extern sdword showCapitalShipOverlay;
extern sdword showEnemyShips;
extern sdword showFriendlyShips;
extern sdword showMeteors;
extern sdword autoFocusOnWaypoints;
extern sdword showAreasOccupied;
extern sdword showUniverseExtents;
extern sdword showCompass;
extern sdword showObjectivePointers;
extern sdword showSlipgatePaths;

extern bool32   opTimerActive;
extern real32 opTimerStart;
extern real32 opTimerLength;

void opOptionsSaveCustomEffectsSettings(void);


#define NUM_SMOOTHIES 32


typedef void (*smoothieFunc)(real32 data, struct SmoothieX *smoo);    //callback function

typedef struct SmoothieX
{
    real32 feedback;
    real32 buffer;
    real32 *source;
    real32 *dest;
    sdword *destint;
    real32 threshold;
    sdword *user;
    smoothieFunc callback;
}
Smoothie;

#define OP_OVERLAY_ICON_LIST \
    OP_OVERLAY_ICON(PopupMenu_BorderCorner_TopLeft) \
    OP_OVERLAY_ICON(PopupMenu_BorderCorner_TopRight) \
    OP_OVERLAY_ICON(PopupMenu_BorderCorner_BottomLeft) \
    OP_OVERLAY_ICON(PopupMenu_BorderCorner_BottomRight) \
    OP_OVERLAY_ICON(PopupMenu_Background) \
    OP_OVERLAY_ICON(PopupMenu_BorderEdge_Top) \
    OP_OVERLAY_ICON(PopupMenu_BorderEdge_Bottom) \
    OP_OVERLAY_ICON(PopupMenu_BorderEdge_Left) \
    OP_OVERLAY_ICON(PopupMenu_BorderEdge_Right) \
    OP_OVERLAY_ICON(PopupMenu_MouseOverLeftEnd) \
    OP_OVERLAY_ICON(PopupMenu_MouseOverRightEnd) \
    OP_OVERLAY_ICON(PopupMenu_MouseOverMiddle) \
    OP_OVERLAY_ICON(PopupMenu_SelectedIndicator) \
    OP_OVERLAY_ICON(PopupMenu_SubMenuIndicator) \
    OP_OVERLAY_ICON(PopupMenu_SubMenuIndicator_Disabled) \
    OP_OVERLAY_ICON(SectMothership) \
    OP_OVERLAY_ICON(BeastMothership) \
    OP_OVERLAY_ICON(Build_ProgressBar) \
    OP_OVERLAY_ICON(Research_ProgressBar) \
    OP_OVERLAY_ICON(Stats_Exit_Button_Left) \
    OP_OVERLAY_ICON(Stats_Exit_Button_Right) \
    OP_OVERLAY_ICON(Stats_Exit_Button_Mid) \
    OP_OVERLAY_ICON(Stats_Exit_Button_Left_Mouse) \
    OP_OVERLAY_ICON(Stats_Exit_Button_Right_Mouse) \
    OP_OVERLAY_ICON(Stats_Exit_Button_Mid_Mouse) \
    OP_OVERLAY_ICON(Stats_Picker_1) \
    OP_OVERLAY_ICON(Stats_Picker_1_Mouse) \
    OP_OVERLAY_ICON(Stats_Picker_2) \
    OP_OVERLAY_ICON(Stats_Picker_2_Mouse) \
    OP_OVERLAY_ICON(Stats_Picker_Mid) \
    OP_OVERLAY_ICON(Stats_Picker_Mid_Mouse) \
    OP_OVERLAY_ICON(Exp_Star) \
    OP_OVERLAY_ICON(Stats_First) \
    OP_OVERLAY_ICON(Stats_Second) \
    OP_OVERLAY_ICON(Stats_Third) \
    OP_OVERLAY_ICON(Stats_Fourth) \
    OP_OVERLAY_ICON(Stats_Fifth) \
    OP_OVERLAY_ICON(Stats_Sixth) \
    OP_OVERLAY_ICON(Stats_Seventh) \
    OP_OVERLAY_ICON(Stats_Eigth) \
    OP_OVERLAY_ICON(SensorsScroll_Left) \
    OP_OVERLAY_ICON(SensorsScroll_Right) \
    OP_OVERLAY_ICON(SensorsScroll_Up) \
    OP_OVERLAY_ICON(SensorsScroll_Down) \
    OP_OVERLAY_ICON(SensorsScroll_Left_Mouse) \
    OP_OVERLAY_ICON(SensorsScroll_Right_Mouse) \
    OP_OVERLAY_ICON(SensorsScroll_Up_Mouse) \
    OP_OVERLAY_ICON(SensorsScroll_Down_Mouse) \
    OP_OVERLAY_ICON(Buffer_Up_Mouse) \
    OP_OVERLAY_ICON(Buffer_Up) \
    OP_OVERLAY_ICON(Buffer_Down_Mouse) \
    OP_OVERLAY_ICON(Buffer_Down) \
    OP_OVERLAY_ICON(X)

enum
{
#define OP_OVERLAY_ICON(name) OPI_##name,
    OP_OVERLAY_ICON_LIST
#undef OP_OVERLAY_ICON
    OPI_Count
};



/*=============================================================================
    Function Prototypes:
=============================================================================*/

void opUpdateSettings(void);
void opInitKeyTable(void);
char *opKeyToNiceString(keyindex key);
keyindex opKeyTranslate(keyindex key);
void opKeyAssign(uword func, keyindex key);
bool32 opValidKey(keyindex key);
void opDefineThisKey(keyindex key);
void opKeyboardLoad(void);
udword opMusicVolumeProcess(regionhandle reg, sdword ID, udword event, udword data);
udword opSFXVolumeProcess(regionhandle reg, sdword ID, udword event, udword data);
udword opSpeechVolumeProcess(regionhandle reg, sdword ID, udword event, udword data);
udword opNumChannelsProcess(regionhandle reg, sdword ID, udword event, udword data);

void opTimerExpired(void);
void opDetailQuality(char* name, featom* atom);
void opGameFramerateOption(char* name, featom* atom);
void opSetCataclysmInterfaceStyle(char* name, featom* atom);
void opSetHomeworldInterfaceStyle(char* name, featom* atom);
void opUseHomeworldTaskbar(char* name, featom* atom);
void opShowTaskbarButtons(char* name, featom* atom);
void opShowTaskbarBackground(char* name, featom* atom);
void opShowExperienceMeter(char* name, featom* atom);
void opUseHomeworldContextMenuStyle(char* name, featom* atom);
void opUseRightClickContextMenuStyle(char* name, featom* atom);
void opShowAreasOccupied(char* name, featom* atom);
void opShowAreasVisited(char* name, featom* atom);
void opShowSensorSphere(char* name, featom* atom);
void opShowSensorWireframe(char* name, featom* atom);
void opShowResources(char* name, featom* atom);
void opShowDust(char* name, featom* atom);
void opShowBackgrounds(char* name, featom* atom);
void opShowCompass(char* name, featom* atom);
void opShowObjectivePoints(char* name, featom* atom);
void opShowSlipgatePaths(char* name, featom* atom);
void opShowCapitalShipOverlay(char* name, featom* atom);
void opOverlayIconsStartup(void);
void opOverlayIconsShutdown(void);
void opOverlayIconDraw(sdword iconIndex, sdword top, sdword left, sdword width, sdword height, color c);
udword opNoPalMBSliderProcess(regionhandle reg, sdword ID, udword event, udword data);
void opNoPalMBSlider(char* name, featom* atom);
void opSpeechTest(char* name, featom* atom);
void opSoundFXTest(char* name, featom* atom);

void InitSmoothie(Smoothie *smooothie,real32 feedback, real32 *source, real32 *dest,
                  sdword *destint, smoothieFunc callback, real32 threshold);
void AddSmoothie(Smoothie *smoo);
void RemoveSmoothie(Smoothie *smoo);

char ConvertSliderToAIPlayerDifficulty(sdword sliderval);

#endif
