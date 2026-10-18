
#ifndef ___LEVELLOAD_H
#define ___LEVELLOAD_H

#include "types.h"
#include "objtypes.h"

/*=============================================================================
    Functions:
=============================================================================*/

void levelPreInit(char *directory,char *pickedMission);
void levelInit(char *directory,char *pickedMission);
signed int levelInitResourceVolumes(void);

void levelStartNext(char *directory,char *pickedMission);        // for single player game only

// utility functions
ShipType GetAppropriateShipTypeForRace(ShipType request,ShipRace shiprace);
void TryToFindMothershipsForPlayers();
ShipRace GetSinglePlayerRaceEquivalent(ShipRace race);
void llResetMissionSpherePlayerRemap(void);
void llRandomizeMissionSpherePlayerRemap(sdword maxPlayers);

bool32 AddResourceToSphere(struct ResourceVolume *sphere,bool32 regrowing);
bool32 AddResourceToCylinder(struct ResourceVolume *cylinder,bool32 regrowing);
bool32 AddResourceToRectangle(struct ResourceVolume *cylinder,bool32 regrowing);

void SetInfoNeededForShipAndRelatedStaticInfo(ShipType type,ShipRace race,bool8 dataToFillIn);

/*=============================================================================
    Global variables:
=============================================================================*/

extern udword nebAttributes;
extern sdword SongNumber;

#endif
