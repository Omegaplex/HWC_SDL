/*=============================================================================
    Name    : slipgate.c
    Purpose : Slipgate shared helpers
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "Slipgate.h"
#include "dock.h"
#include "commandlayer.h"
#include "universe.h"
#include "etg.h"

static const real32 slipgateOpeningDuration = 2.25f;  /* cataclysm.exe 0x72C018 */
static const real32 slipgateClosingDuration = 0.625f; /* cataclysm.exe 0x72C014 */

static SlipgateShipSpec *slipgateSpec(Ship *ship)
{
    return (SlipgateShipSpec *)ship->ShipSpecifics;
}

static void slipgateDeleteEffect(SlipgateShipSpec *spec)
{
    SpaceObj *effectObj;

    if (spec->effect == NULL)
    {
        return;
    }

    effectObj = (SpaceObj *)spec->effect;
    etgEffectDelete(spec->effect);
    univRemoveObjFromRenderList(effectObj);
    listDeleteNode(&effectObj->objlink);
    spec->effect = NULL;
}

static etgeffectstatic *slipgateEffectForState(ubyte state)
{
    etglod *effectLOD;

    switch (state)
    {
        case 0:
            effectLOD = etgSpecialPurposeEffectTable[EGT_SLIPGATE_PERMANENT_EFFECT];
            break;
        case 1:
        case 2:
            effectLOD = etgSpecialPurposeEffectTable[EGT_SLIPGATE_OPENING_EFFECT];
            break;
        case 3:
            effectLOD = etgSpecialPurposeEffectTable[EGT_SLIPGATE_CLOSING_EFFECT];
            break;
        default:
            return NULL;
    }

    if (effectLOD == NULL || effectLOD->nLevels <= 0)
    {
        return NULL;
    }

    return effectLOD->level[0];
}

Effect *slipgateSetState(Ship *ship, ubyte state)
{
    SlipgateShipSpec *spec = slipgateSpec(ship);
    etgeffectstatic *effectStat;
    real32 effectRadius;

    slipgateDeleteEffect(spec);

    switch (state)
    {
        case 0:
            spec->stateExpiryTime = 0.0f;
            break;
        case 1:
        case 2:
            spec->stateExpiryTime = universe.totaltimeelapsed + slipgateOpeningDuration;
            break;
        case 3:
            spec->stateExpiryTime = universe.totaltimeelapsed + slipgateClosingDuration;
            break;
        default:
            spec->state = state;
            return NULL;
    }

    spec->state = state;
    effectStat = slipgateEffectForState(state);
    if (effectStat != NULL)
    {
        effectRadius = (state == 0) ? spec->permanentEffectRadius : spec->transientEffectRadius;
        spec->effect = etgEffectCreate(
            effectStat,
            ship,
            NULL,
            NULL,
            NULL,
            0.0f,
            120,
            3,
            *(udword *)&effectRadius,
            0,
            spec->slipgateColor);
    }
    return spec->effect;
}

bool32 slipgateActivateApproach(Ship *ship)
{
    SlipgateShipSpec *spec = slipgateSpec(ship);
    SlipgateShipSpec *partnerSpec = NULL;
    bool32 hasPartner = FALSE;

    if (spec->partner != NULL && (ship->flags & SOF_AvoidanceHidden) == 0)
    {
        partnerSpec = slipgateSpec(spec->partner);
        spec->approachStartTime = universe.totaltimeelapsed;
        partnerSpec->approachStartTime = universe.totaltimeelapsed;
        hasPartner = TRUE;
    }
    else
    {
        spec->approachStartTime = 0.0f;
    }

    if (spec->state == 0)
    {
        slipgateSetState(ship, 1);
    }
    if (hasPartner && partnerSpec->state == 0)
    {
        slipgateSetState(spec->partner, 1);
    }

    return spec->state == 2 && (!hasPartner || partnerSpec->state == 2);
}

bool32 slipgateActivateBeam(Ship *ship)
{
    SlipgateShipSpec *spec = slipgateSpec(ship);

    slipgateSetState(ship, 3);
    if (spec->partner != NULL && (ship->flags & SOF_AvoidanceHidden) == 0)
    {
        slipgateSetState(spec->partner, 3);
    }
    return TRUE;
}

Ship *slipgateSetPartner(Ship *ship, Ship *partner)
{
    slipgateSpec(ship)->partner = partner;
    return partner;
}

Ship *slipgateGetPartner(Ship *ship)
{
    return slipgateSpec(ship)->partner;
}

ubyte slipgateGetState(Ship *ship)
{
    return slipgateSpec(ship)->state;
}

void slipgateMarkPlayerTransit(Ship *ship, sdword playerIndex)
{
    if (playerIndex >= 0 && playerIndex < sizeof(slipgateSpec(ship)->playerTransitState))
    {
        slipgateSpec(ship)->playerTransitState[playerIndex] = 1;
    }
}

void slipgateAddCaptureTarget(Ship *ship, Ship *captureTarget)
{
    growSelectAddShip(&slipgateSpec(ship)->queuedShips, captureTarget);
}

void slipgateRequestDock(Ship *dockwith, Ship *ship)
{
    MaxSelection candidateSelection;
    MaxSelection dockSelection;
    Node *node;
    sdword i;

    candidateSelection.numShips = 0;
    dockSelection.numShips = 0;

    for (node = universe.mainCommandLayer.todolist.head; node != NULL; node = node->next)
    {
        CommandToDo *command = (CommandToDo *)node->structptr;
        bool32 matchesSlipgate = FALSE;

        if ((command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) != 0 &&
            command->attack != NULL &&
            ShipInSelection((SelectCommand *)command->attack, ship))
        {
            matchesSlipgate = TRUE;
        }
        else if (command->ordertype.order == COMMAND_MILITARY_PARADE &&
                 command->launchship.receiverShip == ship)
        {
            matchesSlipgate = TRUE;
        }

        if (!matchesSlipgate)
        {
            continue;
        }

        for (i = 0; i < command->selection->numShips && candidateSelection.numShips < COMMAND_MAX_SHIPS; i++)
        {
            candidateSelection.ShipPtr[candidateSelection.numShips++] = command->selection->ShipPtr[i];
        }
    }

    for (i = 0; i < candidateSelection.numShips; i++)
    {
        Ship *candidate = candidateSelection.ShipPtr[i];

        if (ThisShipCanDockWith(candidate, dockwith, 10, FALSE))
        {
            dockSelection.ShipPtr[dockSelection.numShips++] = candidate;
        }
    }

    if (dockSelection.numShips > 0)
    {
        clDock(&universe.mainCommandLayer, (SelectCommand *)&dockSelection, 10, dockwith);
    }
}
