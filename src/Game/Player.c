/*=============================================================================
    Name    : Player.c
    Purpose : Decompiled functions from cataclysm.exe (Player module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "universe.h"
#include "Globals.h"
#include "Alliance.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x480D10
sdword playerGetSensorCoverage(Player *player)
{
    if (!multiPlayerGame)
        return *(sdword *)((int)player + 25856);
    if (allianceArePlayersAllied(universe.curPlayerPtr, player))
        return *(sdword *)((int)universe.curPlayerPtr + 25856);
    return *(sdword *)((int)player + 25856);
}
