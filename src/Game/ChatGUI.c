/*=============================================================================
    Name    : ChatGUI.c
    Purpose : Decompiled functions from cataclysm.exe (ChatGUI module)
=============================================================================*/

#include "types.h"
#include "Tutor.h"

extern sdword tutNextVisible;
extern sdword tutPrevButtonVisible;
extern sdword tutQuitVisible;
extern sdword ioSavedNextVisible;
extern sdword ioSavedPrevButtonVisible;
extern sdword ioSavedQuitVisible;
void tutShowQuitButton(void);

// ============================================================================
// Decompiled from cataclysm.exe @ 0x534300
int lgUpdateChatRegions(void)
{
    int result = ioSavedNextVisible;

    if (ioSavedNextVisible)
    {
        tutShowNextButton();
    }
    if (ioSavedPrevButtonVisible)
    {
        tutShowPrevButtonRegion();
    }
    if (ioSavedQuitVisible)
    {
        tutShowQuitButton();
        result = ioSavedQuitVisible;
    }

    return result;
}
