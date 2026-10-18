/*=============================================================================
    Name    : Research.c
    Purpose : Decompiled functions from cataclysm.exe (Research module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "universe.h"
#include "ResearchAPI.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x4F41B0
bool32 rmTechIsResearched(Player *player, int techIndex)
{
    sdword *techState = &player->classtotals[12];

    return techState[41] == 2 || techState[techIndex] == 1;
}
