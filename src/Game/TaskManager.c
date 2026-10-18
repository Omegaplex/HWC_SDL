/*=============================================================================
    Name    : TaskManager.c
    Purpose : Decompiled functions from cataclysm.exe (TaskManager module)
=============================================================================*/

#include "types.h"
#include "objtypes.h"
#include "region.h"

extern regionhandle svShipViewRegion;
extern ShipType svShipType;


// ============================================================================
// Decompiled from cataclysm.exe @ 0x514C70
void mgResetShipListFilter(void)
{
    svShipViewRegion = NULL;
    svShipType = DefaultShip;
}
