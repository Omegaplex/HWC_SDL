/*=============================================================================
    Name    : SalCapCorvetteHelpers.c
    Purpose : Shared attached-salvager iteration helpers
=============================================================================*/

#include "types.h"
#include "SalCapCorvette.h"
#include "universe.h"

static Ship *salCapAttachedTarget = NULL;
static Node *salCapAttachedIterator = NULL;

static Ship *SalCapFindNextAttachedShip(void)
{
    while (salCapAttachedIterator != NULL)
    {
        Ship *ship;

        ship = salCapAttachedIterator->structptr;
        salCapAttachedIterator = salCapAttachedIterator->next;
        if ((ship->specialFlags & SPECIAL_IsASalvager) &&
            shipDockingTarget(ship) == salCapAttachedTarget)
        {
            return ship;
        }
    }

    salCapAttachedTarget = NULL;
    return NULL;
}

Ship *SalCapGetFirstAttachedShip(Ship *target)
{
    salCapAttachedTarget = target;
    salCapAttachedIterator = (target != NULL) ? universe.ShipList.head : NULL;
    return SalCapFindNextAttachedShip();
}

Ship *SalCapGetNextAttachedShip(void)
{
    if (salCapAttachedTarget == NULL)
        return NULL;

    return SalCapFindNextAttachedShip();
}
