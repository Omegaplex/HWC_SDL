#ifndef ___LEECH_H
#define ___LEECH_H

#include "types.h"
#include "spaceobj.h"
#include "CommandLayer.h"

void leechStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo);
void leechInit(Ship *ship);
void leechFreeShipContents(Ship *ship);
bool32 leechSpecialActivate(Ship *ship, void *custom);
void leechSpecialCleanup(void *context);
void leechHousekeep(Ship *ship);
void leechResetFeedbackState(void);
void leechRemoveShipReferences(Ship *ship, Ship *shipToRemove);
void leechClose(Ship *ship);
void leechSetMode(Ship *ship, ubyte leechMode);
void leech_PreFix(Ship *ship);
void leech_Save(Ship *ship);
void leech_Load(Ship *ship);
void leech_Fix(Ship *ship);

#endif
