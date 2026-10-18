/*=============================================================================
    Name    : Swarmer.h
    Purpose : Swarmer declarations
=============================================================================*/

#ifndef ___SWARMER_H
#define ___SWARMER_H

#include "SpaceObj.h"

// Retail LoadShip builds sSwarmer's main chunk size as 1592 + 0x20, and the
// identified SharedSwarmer callbacks only touch ShipSpecifics through +0x10.
#define SWARMER_RETAIL_SPECIFICS_SIZE 0x20

void SharedSwarmerStaticInit(char *directory, char *filename, struct ShipStaticInfo *statinfo);
void SharedSwarmerInit(Ship *ship);
void SharedSwarmerRemoveShipReferences(Ship *ship, Ship *shipToRemove);
void SharedSwarmer_PreFix(Ship *ship);
void SharedSwarmer_Fix(Ship *ship);

#endif
