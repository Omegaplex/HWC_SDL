/*=============================================================================
    Name    : Swarmer.c
    Purpose : Swarmer reconstruction and decompiled pseudocode reference blocks
=============================================================================*/

#include <math.h>
#include <string.h>
#include "types.h"
#include "debug.h"
#include "Attack.h"
#include "AIShip.h"
#include "AITrack.h"
#include "ClassDefs.h"
#include "SaveGame.h"
#include "spaceobj.h"
#include "statscript.h"
#include "Swarmer.h"
#include "universe.h"

void telldir(void);

static AttackSideStepParameters sectSwarmerStaticsA;
static AttackSideStepParameters sectSwarmerStaticsB;

void SharedSwarmerStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    AttackSideStepParameters *statics =
        (statinfo->shiprace == RACE_Sect) ? &sectSwarmerStaticsA : &sectSwarmerStaticsB;

    memset(statics, 0, sizeof(*statics));
    scriptSetStruct(directory, filename, AttackSideStepParametersScriptTable, statics);
    statinfo->custstatinfo = statics;
}

void SharedSwarmerInit(Ship *ship)
{
    SwarmerSpecifics *spec = (SwarmerSpecifics *)ship->ShipSpecifics;

    attackSideStepInit((AttackSideStep *)&spec->attackSideStepTime);
    ship->specialFlags3 |= SPECIAL3_Unknown12;
    spec->unk10 = 0;
}

void SharedSwarmerRemoveShipReferences(Ship *ship, Ship *shipToRemove)
{
    SwarmerSpecifics *spec = (SwarmerSpecifics *)ship->ShipSpecifics;

    if (spec->trackedTarget == shipToRemove)
    {
        spec->trackedTarget = NULL;
    }
}

void SharedSwarmer_PreFix(Ship *ship)
{
    SwarmerSpecifics *spec = (SwarmerSpecifics *)ship->ShipSpecifics;

    spec->trackedTarget = (Ship *)SpaceObjRegistryGetID((SpaceObj *)spec->trackedTarget);
}

void SharedSwarmer_Fix(Ship *ship)
{
    SwarmerSpecifics *spec = (SwarmerSpecifics *)ship->ShipSpecifics;

    spec->trackedTarget = (Ship *)SpaceObjRegistryGetShip((sdword)spec->trackedTarget);
}
