/*=============================================================================
    Name    : shipselect.c
    Purpose : Routines for manipulating SelectCommand structures

    Created 10/19/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "spaceobj.h"
#include "shipselect.h"
#include "universe.h"
#include "alliance.h"
#include "attributes.h"
#include "ProximitySensor.h"
#include "SinglePlayer.h"
#include "soundevent.h"
#include "speechevent.h"
#include "battle.h"
#include "InfoOverlay.h"
#include "consmgr.h"
#include "select.h"
#include "MissionScriptSupport.h"
#include "MultiplayerGame.h"
#include "BigGun.h"

sdword univIsObjFriendly(SpaceObj *obj, Player *player);
bool32 univObjIsDying(SpaceObj *obj);
bool32 SalCapCanTargetShipWithReasonCounts(Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target, uword *reasonCounts);
bool32 SalCapHasHostQueue(Ship *ship);
ubyte *SalCapGetHostQueue(Ship *ship);
BOOL salCapIsActiveOrBeaming(Ship *ship);

static bool32 shipselectIsShipInfected(Ship *ship)
{
    // Retail `IsShipInfected` checks byte 0 of Ship+0x638, which is simply the first byte of
    // the ship-specific payload. Its meaning depends on the active ShipSpecifics layout.
    return *(ubyte *)ship->ShipSpecifics != 0;
}

BOOL IsShipInfected(Ship *ship)
{
    return shipselectIsShipInfected(ship);
}

static bool32 shipselectIsAttackCapableByHeader(const ShipStaticInfo *shipstatic)
{
    // Retail attack-capable filters check the active attack-command callback slot.
    return shipstatic->custshipheader.CustShipActiveAttackCommand != NULL;
}

static bool32 shipselectIsCrystal(Ship *ship)
{
    return (ship != NULL) && (ship->shiptype == dBigGunUnit);
}

static bool32 shipselectHasDockFlaggedAttachedShip(Ship *ship)
{
    SelectCommand *attachedSelection;
    sdword attachedIndex;

    if (ship->clampInfo == NULL || ship->clampInfo->selection == NULL)
    {
        return FALSE;
    }

    attachedSelection = (SelectCommand *)ship->clampInfo->selection;
    for (attachedIndex = 0; attachedIndex < attachedSelection->numShips; attachedIndex++)
    {
        if ((attachedSelection->ShipPtr[attachedIndex]->specialFlags2 & SPECIAL_2_DockFlag) != 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static bool32 shipselectHostQueueHasCrystal(Ship *ship)
{
    Ship **hostQueue;
    sdword queueIndex;

    if (ship->flags2 != 0 || !SalCapHasHostQueue(ship))
    {
        return FALSE;
    }

    hostQueue = (Ship **)SalCapGetHostQueue(ship);
    for (queueIndex = 0; queueIndex < 16; queueIndex++, hostQueue += 5)
    {
        if (*hostQueue != NULL && shipselectIsCrystal(*hostQueue))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static bool32 shipselectPlayerHasShipWithUpgrade(Player *player, udword upgradeMask, ShipType excludedShipType)
{
    Node *node;

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ownedShip = node->structptr;

        if ((ownedShip->playerowner == player) &&
            ShipHasUpgrade(ownedShip, upgradeMask) &&
            ((excludedShipType == DefaultShip) || (ownedShip->shiptype != excludedShipType)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : AddSpaceObjToSelectionBeforeIndex
    Description : Adds a SpaceObj to selection before index
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddSpaceObjToSelectionBeforeIndex(SpaceObj *obj,SpaceObjSelection *selection,sdword index)
{
    sdword i;

    dbgAssert(index >= 0);
    dbgAssert(index < selection->numSpaceObjs);

    for (i=selection->numSpaceObjs;i>index;i--)
    {
        selection->SpaceObjPtr[i] = selection->SpaceObjPtr[i-1];
    }
    selection->SpaceObjPtr[index] = obj;
    selection->numSpaceObjs++;
}

/*-----------------------------------------------------------------------------
    Name        : AddSpaceObjToSelectionAfterIndex
    Description : Adds a SpaceObj to selection after index
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddSpaceObjToSelectionAfterIndex(SpaceObj *obj,SpaceObjSelection *selection,sdword index)
{
    sdword i;

    dbgAssert(index >= 0);
    dbgAssert(index < selection->numSpaceObjs);

    index++;

    for (i=selection->numSpaceObjs;i>index;i--)
    {
        selection->SpaceObjPtr[i] = selection->SpaceObjPtr[i-1];
    }
    selection->SpaceObjPtr[index] = obj;
    selection->numSpaceObjs++;
}

/*-----------------------------------------------------------------------------
    Name        : RemoveSpaceObjFromSelectionPreserveOrder
    Description : removes a space obj from the selection, while preserving order
    Inputs      : selection, obj
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 RemoveSpaceObjFromSelectionPreserveOrder(SpaceObjSelection *selection,SpaceObj *obj)
{
    sdword i;
    sdword numSpaceObjs = selection->numSpaceObjs;

    for (i=0;i<numSpaceObjs;i++)
    {
        if (obj == selection->SpaceObjPtr[i])
        {
            for ( ;i<numSpaceObjs-1;i++)
            {                                           //for the rest of the list
                selection->SpaceObjPtr[i] = selection->SpaceObjPtr[i+1];
            }
            selection->numSpaceObjs--;

            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : RemoveTargetFromSelection
    Description : A useful utility to remove targets from selection
                  Does not delete entry - just removes reference to it
    Inputs      : removeTargetPtr
    Outputs     :
    Return      : Returns TRUE if any target was removed
----------------------------------------------------------------------------*/
bool32 clRemoveTargetFromSelection(SelectAnyCommand *selection,TargetPtr removeTargetPtr)
{
    sdword i;
    bool32 removedAny = FALSE;

    for (i=0;i<selection->numTargets; )
    {
        if (removeTargetPtr == selection->TargetPtr[i])
        {
            removedAny = TRUE;
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
                      // warning: leave this as continue because sometimes removeTargetPtr may be set to NULL to remove all null's
        }
        i++;
    }

    return removedAny;
}

/*-----------------------------------------------------------------------------
    Name        : ShipInSelection
    Description : returns TRUE if ship is in selection
    Inputs      : selection, ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 ShipInSelection(SelectCommand *selection,Ship *ship)
{
    sdword i;

    for (i=0;i<selection->numShips;i++)
    {
        if (selection->ShipPtr[i] == ship)
        {
            return TRUE;
        }
    }
    return FALSE;
}


/*-----------------------------------------------------------------------------
    Name        : ShiptypeInSelection
    Description : Returns TRUE if the selection contains a ship of shiptype "Type"
    Inputs      : selection - the selection to check, type - the type to check for
    Outputs     :
    Return      : TRUE if a ship of the specified type is found
----------------------------------------------------------------------------*/
udword ShiptypeInSelection(SelectCommand *selection, ShipType type)
{
    udword i, count;

    for (i=0,count=0;i<selection->numShips;i++)
    {
        if (selection->ShipPtr[i]->shiptype == type)
        {
            count++;
        }
    }
    return count;
}


/*-----------------------------------------------------------------------------
    Name        : FindFirstInstanceOfShipType
    Description : Returns the first ship in the selection of the specified type
    Inputs      : selection - the selection to check, type - the type to check for
    Outputs     :
    Return      : A pointer to the first instance of the specified type
----------------------------------------------------------------------------*/
ShipPtr FindFirstInstanceOfShipType(SelectCommand *selection, ShipType type)
{
    udword i;

    for (i = 0; i<selection->numShips; i++)
    {
        if (selection->ShipPtr[i]->shiptype == type)
        {
            return selection->ShipPtr[i];
        }
    }
    return NULL;
}




/*-----------------------------------------------------------------------------
    Name        : MakeShipsNotIncludeTheseShips
    Description : Makes sure that ships in selection do not include theseships
    Inputs      : selection, theseships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsNotIncludeTheseShips(SelectCommand *selection,SelectCommand *theseships)
{
    sdword numTheseShips = theseships->numShips;
    sdword i;

    for (i=0;i<numTheseShips;i++)
    {
        clRemoveShipFromSelection(selection,theseships->ShipPtr[i]);
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : AnyOfTheseShipsAreInSelection
    Description : returns true if any ships in theseShips are in selection
    Inputs      : theseShips,selection
    Outputs     :
    Return      : returns true if any ships in theseShips are in selection
----------------------------------------------------------------------------*/
bool32 AnyOfTheseShipsAreInSelection(SelectCommand *theseShips,SelectCommand *selection)
{
    sdword i,j;
    ShipPtr thisShip;

    for (i=0;i<theseShips->numShips;i++)
    {
        thisShip = theseShips->ShipPtr[i];

        for (j=0;j<selection->numShips;j++)
        {
            if (thisShip == selection->ShipPtr[j])
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : TheseShipsAreInSelection
    Description : returns true if all of these ships are in selection
    Inputs      : theseShips, selection
    Outputs     :
    Return      : returns true if all of these ships are in selection
----------------------------------------------------------------------------*/
bool32 TheseShipsAreInSelection(SelectCommand *theseShips,SelectCommand *selection)
{
    sdword i;
    sdword numShips = theseShips->numShips;

    if (numShips == 0)
    {
        return FALSE;
    }

    for (i=0;i<numShips;i++)
    {
        if (!ShipInSelection(selection,theseShips->ShipPtr[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : SelectionsAreEquivalent
    Description : tells whether selection1 and selection2 are focusing on the same
                  ships (ignores order of objects in lists).
    Inputs      : selection1, selection2
    Outputs     :
    Return      : true if selection1 and selection2 are focusing on the same ships
----------------------------------------------------------------------------*/
bool32 SelectionsAreEquivalent(SelectCommand *selection1,SelectCommand *selection2)
{
    sdword i,j;
    sdword numShips = selection1->numShips;
    ShipPtr matchship;
    bool32 matched;

    if (numShips != selection2->numShips)
    {
        return FALSE;
    }

    for (i=0;i<numShips;i++)
    {
        matched = FALSE;
        matchship = selection1->ShipPtr[i];

        for (j=0;j<numShips;j++)
        {
            if (selection2->ShipPtr[j] == matchship)
            {
                matched = TRUE;
                break;
            }
        }

        if (!matched)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : SelectionsAreTotallyEquivalent
    Description : tells whether selection1 and selection2 are the same selections
                  (including order).
    Inputs      : selection1, selection2
    Outputs     :
    Return      : true if selection1 and selection2 are the same
----------------------------------------------------------------------------*/
bool32 SelectionsAreTotallyEquivalent(SelectCommand *selection1,SelectCommand *selection2)
{
    sdword numShips = selection1->numShips;
    sdword i;

    if (numShips != selection2->numShips)
    {
        return FALSE;
    }

    for (i=0;i<numShips;i++)
    {
        if (selection1->ShipPtr[i] != selection2->ShipPtr[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : isShipAttackCapable
    Description : returns TRUE if ship is attack capable
    Inputs      : ship
    Outputs     :
    Return      : returns TRUE if ship is attack capable
----------------------------------------------------------------------------*/
bool32 isShipAttackCapable(Ship *ship)
{
    if (!shipselectIsAttackCapableByHeader(ship->staticinfo))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : AreAllShipsAttackCapable
    Description : returns TRUE if all ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if all ships are attack capable
----------------------------------------------------------------------------*/
bool32 AreAllShipsAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (!shipselectIsAttackCapableByHeader(shipstatic))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : AreAnyShipsAttackCapable
    Description : returns TRUE if any ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if any ships are attack capable
----------------------------------------------------------------------------*/
bool32 AreAnyShipsAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipselectIsAttackCapableByHeader(shipstatic))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : isShipPassiveAttackCapable
    Description : returns TRUE if ship is attack capable
    Inputs      : ship
    Outputs     :
    Return      : returns TRUE if ship is attack capable
----------------------------------------------------------------------------*/
bool32 isShipPassiveAttackCapable(Ship *ship)
{
    CommandToDo *command = ship->command;

    if (command != NULL &&
        bitTest(command->ordertype.attributes, COMMAND_IS_FORMATION))
    {
        TypeOfFormation formationtype = command->formation.formationtype;
        if (formationtype == FORCEFIELD_SPHERE_FORMATION ||
            formationtype == FORCEFIELD_WALL_FORMATION)
        {
            return FALSE;
        }
    }

    if (bitTest(ship->specialFlags2, SPECIAL_2_DockFlag))
    {
        return FALSE;
    }

    return ship->staticinfo->custshipheader.CustShipRotateAndShoot != NULL;
}

/*-----------------------------------------------------------------------------
    Name        : AreAllShipsPassiveAttackCapable
    Description : returns TRUE if all ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if all ships are attack capable
----------------------------------------------------------------------------*/
bool32 AreAllShipsPassiveAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipRotateAndShoot == NULL)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : AreAnyShipsPassiveAttackCapable
    Description : returns TRUE if any ships are attack capable
    Inputs      : selection
    Outputs     :
    Return      : returns TRUE if any ships are attack capable
----------------------------------------------------------------------------*/
bool32 AreAnyShipsPassiveAttackCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = selection->ShipPtr[i]->staticinfo;
        if (shipstatic->custshipheader.CustShipRotateAndShoot != NULL)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsAttackCapable
    Description : Makes sure that the ships in this selection are capable of
                  attack.  Removes any ships from this selection that are not
                  capable of attacking.
    Inputs      : source - selection to cull
    Outputs     : dest - culled selection
    Return      : number of ships in culled selection
    Note        : source and dest can safely be the same selection.
----------------------------------------------------------------------------*/
sdword MakeShipsAttackCapable(SelectCommand *dest, SelectCommand *source)
{
    sdword index, numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        Ship *ship = source->ShipPtr[index];
        bool32 attackCapable = shipselectIsAttackCapableByHeader(ship->staticinfo);

        if (attackCapable &&
            !salCapIsActiveOrBeaming(ship) &&
            ship->specialFlags3 == 0)
        {
            dest->ShipPtr[numShips] = ship;
            numShips++;
        }
    }
    dest->numShips = numShips;
    ioUpdateShipTotals();
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : ShipCanHarvest
    Description : returns TRUE if ship can harvest
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship can harvest
----------------------------------------------------------------------------*/
bool32 ShipCanHarvest(ShipStaticInfo *shipstatic)
{
    return (shipstatic->maxresources > 0) && (shipstatic->harvestAmount > 0);
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsHarvestCapable
    Description : Makes a selection of just ships that are harvesters.
    Inputs      : source - selection to search through
    Outputs     : dest - where to store the culled selection
    Return      : number of ships in culled selection
----------------------------------------------------------------------------*/
sdword MakeShipsHarvestCapable(SelectCommand *dest, SelectCommand *source, bool32 requireIdleLightCorvetteOrCargoBarge)
{
    sdword index;
    sdword numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        Ship *ship = source->ShipPtr[index];
        ShipStaticInfo *staticinfo = ship->staticinfo;

        if ((staticinfo->shipclass == CLASS_Resource) &&
            (staticinfo->harvestAmount > 0) &&
            (singlePlayerGame || bitTest(tpGameCreated.flag, MG_HarvestinEnabled)) &&
            ((!requireIdleLightCorvetteOrCargoBarge) ||
             ((ship->specialFlags3 == 0) &&
              ((ship->shiptype == LightCorvette) || (ship->shiptype == CargoBarge)))))
        {
            dest->ShipPtr[numShips] = ship;
            numShips++;
        }
    }
    dest->numShips = numShips;
    ioUpdateShipTotals();
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : ShipCanGuard
    Description : returns TRUE if ship is capable of guarding
    Inputs      :
    Outputs     :
    Return      : returns TRUE if ship is capable of guarding
----------------------------------------------------------------------------*/
bool32 ShipCanGuard(ShipStaticInfo *shipstatic)
{
    if (ShipTypeIsMothershipType(shipstatic->shiptype))
    {
        return FALSE;
    }

    if (ShipTypeIsProximitySensorType(shipstatic->shiptype))
    {
        return FALSE;
    }

    /*
    if (shipstatic->shiptype == ProximitySensor)
    {
        return FALSE;
    }
    */
#if 0
    if (shipstatic->shiptype == ResearchShip)
    {
        return FALSE;
    }

    if (shipstatic->shiptype == SensorArray)
    {
        return FALSE;
    }
#endif
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsGuardCapable
    Description : Makes sure that the ships in this selection are capable of
                  guarding.  Removes any ships from this selection that are not
                  capable of guarding.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsGuardCapable(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (!ShipCanGuard(shipstatic))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsDockCapable
    Description : removes ships that shouldn't be docking from the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsDockCapable(SelectCommand *selection, bool32 requireNotDocked)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        if (ship->staticinfo->shipclass == CLASS_Mothership ||
            ship->staticinfo->shipclass == CLASS_Component ||
            ship->staticinfo->externalBuild ||
            (requireNotDocked && ship->dockvars.dockship != NULL))
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsRetireable
    Description : removes ships that shouldn't be retiring from the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsRetireable(SelectCommand *selection)
{
    sdword i;
    Ship *ship;

    for (i=0;i<selection->numShips;)
    {
        //remove probe from selection..shouldn't dock!
        ship = selection->ShipPtr[i];
        if (ShipTypeIsProximitySensorType(ship->shiptype) || ShipTypeIsTargetDroneType(ship->shiptype))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsNotIncludeSinglePlayerMotherships
    Description : If in single player, removes motherships from the selection
    Inputs      : selection - selection to cull
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsNotIncludeSinglePlayerMotherships(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips;)
    {
        Ship *ship = selection->ShipPtr[i];
        if (ShipTypeIsCarrierType(ship->shiptype) &&
            ((ship->shiprace == RACE_Sect) || (ship->shiprace == RACE_Beast)))
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }

        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsControllable
    Description : Removes ships from the selection such that all ships left in the selction
                  aren't docking and can be freely ordered
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsControllable(SelectCommand *selection,sdword newCommand)
{
    sdword i;
    CommandToDo *command;

    if (playPackets)
    {
        // playing back game, user should be able to do NOTHING
        //selection->numShips = 0;
        return;
    }

    if(newCommand == COMMAND_HALT)
    {
        return;
    }
    for (i=0;i<selection->numShips;)
    {
        command = getShipAndItsCommand(&universe.mainCommandLayer,selection->ShipPtr[i]);
        if(command != NULL)
        {
            if(command->ordertype.order == COMMAND_DOCK ||
                command->ordertype.order == COMMAND_LAUNCHSHIP)
            {
                if (shipDockingCollisionTarget(selection->ShipPtr[i]) != NULL)
                {
                    // to remove ShipPtr from list, decrement numShips and put last member
                    // of array into slot being deleted.

                    ///////////////////////
                    //Cannot Comply speech event!
                    //Use Battle Chatter to filter
                    //if SHOULDn't be called for move's that are cancelled..
                    //then we'll add a flag that gets passed it turning this on or off
                    //event num: COMM_CannotComply
                    //BCE_CannotComply
                    ///////////////////////
                    if (battleCanChatterAtThisTime(BCE_CannotComply, shipDockingCollisionTarget(selection->ShipPtr[i])))
                    {
                        battleChatterAttempt(
                            SOUND_EVENT_DEFAULT,
                            BCE_CannotComply,
                            selection->ShipPtr[i],
                            selection->ShipPtr[i]->shiptype);
                    }

                    selection->numShips--;
                    selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
                    continue; // we must check same index again because we put last array entry here
                }
            }
        }
        i++;
    }
}

void makeSelectionHyperspaceCapable(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips;)
    {
        Ship *ship = selection->ShipPtr[i];

        if (shipselectHasDockFlaggedAttachedShip(ship) ||
            shipselectHostQueueHasCrystal(ship) ||
            (ship->shiptype == sMothership &&
             ship->command != NULL &&
             (ship->command->ordertype.order == COMMAND_SPECIAL ||
              mothershipCanFireBigGun(ship))) ||
            !TW_HyperSpaceCostStruct[ship->shiptype].canMpHyperspace)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }

    ioUpdateShipTotals();
}


/*-----------------------------------------------------------------------------
    Name        : MakeShipsSpecialActivateCapable
    Description : Makes sure that the ships in this selection are capable of
                  the special activate.  Removes any ships from this selection that are not
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsSpecialActivateCapable(SelectCommand *selection, bool32 requireIdleSpecial)
{
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        shipstatic = (ShipStaticInfo *)ship->staticinfo;
        if (shipstatic->custshipheader.CustShipSpecialActivateNoTarget == NULL ||
            (requireIdleSpecial &&
             ship->specialFlags3 != 0 &&
             ship->shiptype != sLeech &&
             ship->shiptype != sSentinel &&
             ship->shiptype != bSentinel))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsSpecialTargetCapable
    Description : Makes sure that the ships in this selection are capable of
                  special targeting.  Removes any ships from this selection that are not
    Inputs      : selection - selection to neuter
                  bFriendlies - if TRUE, only select ships that can z-bandbox
                    friendlies, otherwise only ships that can z-bandbox enemies
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsSpecialTargetCapable(SelectCommand *selection, bool32 bFriendlies)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        Ship *ship = selection->ShipPtr[i];
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
//        if (shipstatic->custshipheader.CustShipSpecialTarget == NULL)
//        {
        if (shipstatic->custshipheader.CustShipSpecialTargetRaw != NULL)
        {
            if (bFriendlies)
            {                                               //if we're targeting friendlies and ship can do so
                if (shipstatic->canSpecialBandBoxFriendlies)
                {
                    i++;                                    //leave this ship as is
                    continue;
                }
            }
            else
            {                                               //if we're targeting enemies and ship can do so
                if (!shipstatic->canSpecialBandBoxFriendlies)
                {
                    i++;                                    //leave this ship as is
                    continue;
                }
            }
        }
        // to remove ShipPtr from list, decrement numShips and put last member
        // of array into slot being deleted.
        selection->numShips--;
        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

        continue; // we must check same index again because we put last array entry here
//        }
//        i++;
    }
    ioUpdateShipTotals();
}

bool32 AreShipsMobile(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;i++)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (shipstatic->staticheader.immobile)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsMobile
    Description : Makes sure that the ships in this selection are mobile
                  Removes any ships from this selection that are not mobile.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsMobile(SelectCommand *selection)
{
    sdword i;
    ShipStaticInfo *shipstatic;

    for (i=0;i<selection->numShips;)
    {
        shipstatic = (ShipStaticInfo *)selection->ShipPtr[i]->staticinfo;
        if (shipstatic->staticheader.immobile || selection->ShipPtr[i]->specialFlags3)
        {
            if ((shipstatic->shipclass == CLASS_Mothership) && (selection->ShipPtr[i]->playerowner == universe.curPlayerPtr))
            {
                speechEventQueue(0, 67112987, 0, -1, 0, -1, -1, 5.0f, -1);
            }

            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsFormationCapable
    Description : removes ships that can't go into formation
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void makeShipsFormationCapable(SelectCommand *selection)
{

    sdword i;
    Ship *ship;
    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];
        if (ShipTypeIsProximitySensorType(ship->shiptype))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsNonCapital
    Description : Makes sure that the ships in this selection are not capital
                  ships.  Removes any capital ships from this selection.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsNonCapital(SelectCommand *selection)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (isCapitalShip(selection->ShipPtr[i]))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }

    ioUpdateShipTotals();
}


/*-----------------------------------------------------------------------------
    Name        : MakeShipsOnlyCapital
    Description : Makes sure that the ships in this selection are only capital ships
    Inputs      : selection - the selection to lookat
    Outputs     : Removes non capital ships from the selection
    Return      : void
----------------------------------------------------------------------------*/
void MakeShipsOnlyCapital(SelectCommand *selection)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (isCapitalShip(selection->ShipPtr[i]))
        {
            i++;
        }
        else
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
        }
    }

    ioUpdateShipTotals();
}


/*-----------------------------------------------------------------------------
    Name        : MakeShipsOnlyFollowConstraints
    Description : General routine that makes all ships in selection follow constraints set
                  out by shipConstraintsCB callback function.
    Inputs      : selection, shipConstraintsCB
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsOnlyFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (!shipConstraintsCB(selection->ShipPtr[i]))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : DoAllShipsFollowConstraints
    Description : Returns TRUE if all ships in selection follow shipConstraintsCB
    Inputs      : selection, shipConstraintsCB
    Outputs     :
    Return      : Returns TRUE if all ships in selection follow shipConstraintsCB
----------------------------------------------------------------------------*/
bool32 DoAllShipsFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB)
{
    sdword i;

    for (i=0;i<selection->numShips;i++)
    {
        if (!shipConstraintsCB(selection->ShipPtr[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : DoAnyShipsFollowConstraints
    Description : Returns TRUE if any ships in selection follow shipConstraintsCB
    Inputs      : selection, shipConstraintsCB
    Outputs     :
    Return      : Returns TRUE if any ships in selection follow shipConstraintsCB
----------------------------------------------------------------------------*/
bool32 DoAnyShipsFollowConstraints(SelectCommand *selection,ShipConstraintsCB shipConstraintsCB)
{
    sdword i;

    for (i=0;i<selection->numShips;i++)
    {
        if (shipConstraintsCB(selection->ShipPtr[i]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsNotIncludeMissiles
    Description : Makes sure that the targets in selection are not missiles.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsNotIncludeMissiles(SelectAnyCommand *selection)
{
    sdword i;

    for (i=0;i<selection->numTargets;)
    {
        if ((selection->TargetPtr[i])->objtype == OBJ_MissileType)
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsOnlyEnemyShips
    Description : Makes sure that the targets in this selection are ships and are enemies (not curplayer's ships)
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsOnlyEnemyShips(SelectAnyCommand *selection,Player *curplayer)
{
    sdword i;
    SpaceObjRotImpTarg *target;

    for (i=0;i<selection->numTargets;)
    {
        target = selection->TargetPtr[i];

        if ((target->objtype != OBJ_ShipType) ||
            (ShipTypeIsTargetDroneType(((Ship *)target)->shiptype)) ||   // Drones not considered ships in this case
            (((Ship *)target)->playerowner == curplayer) ||
            allianceIsShipAlly((Ship *)target, curplayer))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsOnlyNonForceAttackTargets
    Description : Filters out retail force-attack-only targets from the selection.
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsOnlyNonForceAttackTargets(SelectAnyCommand *selection,Player *curplayer)
{
    sdword i = 0;

    while (i < selection->numTargets)
    {
        SpaceObjRotImpTarg *target = selection->TargetPtr[i];
        bool32 removeTarget = FALSE;

        if (target->objtype != OBJ_ShipType)
        {
            removeTarget = (target->objtype != OBJ_DustType);
        }
        else
        {
            Ship *ship = (Ship *)target;
            bool32 removeDyingSentinel = FALSE;

            if (!curplayer && !singlePlayerGame &&
                (ship->shiptype == sSentinel || ship->shiptype == bSentinel))
            {
                removeDyingSentinel = univObjIsDying((SpaceObj *)ship);
            }

            removeTarget =
                bitTest(ship->staticinfo->shipFlags, SSTATF_BeastShip) ||
                ship->crystalProcessingState.active == 1 ||
                bitTest(ship->specialFlags1C8, BIT21) ||
                ship->shiptype == dEcclesiamInfected ||
                removeDyingSentinel;
        }

        if (removeTarget)
        {
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
            continue;
        }

        i++;
    }

    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : MakeTargetsSalvageable
    Description : Filters the targets for any ship or derelict that's salvageable
    Inputs      : selection - the targets to filter,
                  curplayer - determines while player is enemy
    Outputs     : Changes selection
    Return      : void
----------------------------------------------------------------------------*/
void MakeTargetsSalvageable(SelectAnyCommand *selection, Player *curplayer)
{
    sdword i = 0;

    while (i < selection->numTargets)
    {
        SpaceObjRotImpTarg *target = selection->TargetPtr[i];

        if (target->objtype == OBJ_ShipType &&
            ((Ship *)target)->staticinfo->shiprace == RACE_Derelict &&
            ((Ship *)target)->playerowner != curplayer &&
            !shipselectIsCrystal((Ship *)target))
        {
            i++;
            continue;
        }

        if (target->objtype == OBJ_DerelictType &&
            ((Derelict *)target)->derelicttype == HyperspaceGate)
        {
            i++;
            continue;
        }

        if (target->objtype != OBJ_ShipType ||
            univIsObjFriendly((SpaceObj *)target, curplayer) ||
            ((Ship *)target)->shiptype == JunkYardHQ)
        {
            if (target->objtype == OBJ_MissileType &&
                ((Missile *)target)->missileType == MISSILE_Mine)
            {
                i++;
                continue;
            }

            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
            continue;
        }

        if (shipselectIsCrystal((Ship *)target) ||
            bitTest(((Ship *)target)->flags2, BIT4))
        {
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
            continue;
        }

        if (((Ship *)target)->playerowner == universe.curPlayerPtr ||
            !bitTest(((Ship *)target)->flags2, BIT3) ||
            (!shipIsModuleSlotActive((Ship *)target, universe.curPlayerPtr) &&
             !univShipModuleSlotIsEligible((Ship *)target, universe.curPlayerPtr) &&
             !univShipModuleSlotIsEligibleUpgrade((Ship *)target, universe.curPlayerPtr)))
        {
            i++;
            continue;
        }

        selection->numTargets--;
        selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];
    }

    ioUpdateShipTotals();
}



/*-----------------------------------------------------------------------------
    Name        : MakeSelectionNotHaveCloakedEnemyShips
    Description : Removes enemy cloaked ships from the selection
    Inputs      : selection - the selection being filtered,
                  curplayer - the current player
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void MakeSelectionNotHaveCloakedEnemyShips(SelectCommand *selection, Player *curplayer)
{
    sdword i;
    ShipPtr ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if ((ship->playerowner != curplayer) && (bitTest(ship->flags, SOF_Cloaked)))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue;// we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : makeShipsNotHaveNonCombatShipsForGuardAttack
    Description : Removes CloakGen...and other related ships
                    so that selection can be given an ATATCK order
                    while guarding and such that the cloak gen and other
                    ships are left guarding
    Inputs      : selection - the selection being filtered,
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/

void makeShipsNotHaveNonCombatShipsForGuardAttack(SelectCommand *selection)
{
    sdword i;
    ShipPtr ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if (ShipTypeIsCloakGeneratorType(ship->shiptype) ||
            ShipTypeIsGravWellGeneratorType(ship->shiptype) ||
            ShipTypeIsDefenseFieldFrigateType(ship->shiptype))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue;// we must check same index again because we put last array entry here
        }
        i++;
    }
}


/*-----------------------------------------------------------------------------
    Name        : MakeSelectionNotHaveNonVisibleEnemyShips
    Description : Removes non-visible enemy ships from the selection
                  Note: takes into account Proximity Sensors detecting cloaked ships
    Inputs      : selection - the selection being filtered,
                  curplayer - the current player
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void MakeSelectionNotHaveNonVisibleEnemyShips(SelectCommand *selection, Player *curplayer)
{
    sdword i;
    ShipPtr ship;

    for (i=0;i<selection->numShips;)
    {
        ship = selection->ShipPtr[i];

        if ((ship->playerowner != curplayer) &&
            (univIsShipCloaked((SpaceObj *)ship)) &&
            (!shipGetSlotStatus(ship, curplayer->playerIndex)))
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue;// we must check same index again because we put last array entry here
        }
        i++;
    }
    ioUpdateShipTotals();
}



/*-----------------------------------------------------------------------------
    Name        : MakeTargetsOnlyBeWithinRangeAndNotIncludeMe
    Description : Makes targets all be within range of comparewith, but don't include comparewith
    Inputs      : selection, comparewith, range
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeTargetsOnlyBeWithinRangeAndNotIncludeMe(SelectAnyCommand *selection,SpaceObjRotImpTarg *comparewith,real32 range)
{
    sdword i;
    SpaceObjRotImpTarg *target;
    real32 negrange = -range;
    vector diff;

    for (i=0;i<selection->numTargets;)
    {
        target = selection->TargetPtr[i];

        vecSub(diff,target->posinfo.position,comparewith->posinfo.position);
        if ((target == comparewith) ||
            !isBetweenExclusive(diff.x,negrange,range) ||
            !isBetweenExclusive(diff.y,negrange,range) ||
            !isBetweenExclusive(diff.z,negrange,range) )
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numTargets--;
            selection->TargetPtr[i] = selection->TargetPtr[selection->numTargets];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsFriendlyShips
    Description : Makes sure that the ships in this selection are enemies (not curplayer's ships)
                  Removes any friendly ships from the selection
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsFriendlyShips(SelectCommand *selection,Player *curplayer)
{
    sdword i = 0;

    while (i < selection->numShips)
    {
        Ship *ship = selection->ShipPtr[i];

        if (ship->objtype == OBJ_ShipType
            && (ship->playerowner == curplayer || univIsObjFriendly((SpaceObj *)ship, curplayer)))
        {
            i++;
        }
        else
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
        }
    }
    ioUpdateShipTotals();
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsFriendlyAndAlliesShips
    Description : Makes sure that the ships in this selection are curplayer's and allies  ships
                  Removes any enemy ships from the selection
    Inputs      : selection,curplayer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsFriendlyAndAlliesShips(SelectCommand *selection,struct Player *curplayer)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if  ((selection->ShipPtr[i]->objtype != OBJ_ShipType)
            || ( ((selection->ShipPtr[i])->playerowner != curplayer) &&
               (!allianceIsShipAlly(selection->ShipPtr[i], curplayer)) ) )
        {
            // to remove ShipPtr from list, decrement numShips and put last member
            // of array into slot being deleted.
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

            continue; // we must check same index again because we put last array entry here
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : makeshipMastersincludeslaves(SelectCommand *selection)
    Description : Checks the list of selected ships and if it finds a Master in the list,
                  it adds the slaves aswell.
    Inputs      : selection
    Outputs     : none
    Return      : none
----------------------------------------------------------------------------*/
void MakeShipHostsIncludeAttachedShips(SelectCommand *selection)
{
    sdword i,count;

    count = selection->numShips;

    for (i=0;i < count;)
    {
        Ship *ship = selection->ShipPtr[i];

        if (ship->clampInfo != NULL && ship->clampInfo->host == NULL && ship->clampInfo->selection != NULL)
        {
            SelectCommand *attachedSelection = (SelectCommand *)ship->clampInfo->selection;
            sdword attachedIndex;

            for (attachedIndex = 0; attachedIndex < attachedSelection->numShips; attachedIndex++)
            {
                if(selection->numShips < COMMAND_MAX_SHIPS)
                {
                    selection->ShipPtr[selection->numShips] = attachedSelection->ShipPtr[attachedIndex];
                    selection->numShips++;
                }
                else
                {
                    break;
                }
            }
        }
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : selectAllCurrentPlayersNonHyperspacingShips
    Description : Selects all of the current player's non-hyperspacing ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SelectCommand *selectAllCurrentPlayersNonHyperspacingShips(void)
{
    SelectCommand *selection;
    sdword i = 0;
    Node *objnode;
    Ship *ship;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"selectall",0);

    i = 0;
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            if(!(ship->flags & SOF_Disabled))
            {
                if ((!ShipCanHyperspace(ship)) && !ShipTypeIsTargetDroneType(ship->shiptype) && (ship->shiptype != CryoTray) && !ShipTypeIsProximitySensorType(ship->shiptype))
                {
                    selection->ShipPtr[i++] = ship;
                }
            }
        }

        objnode = objnode->next;
    }

    selection->numShips = i;

    return selection;
}

/*-----------------------------------------------------------------------------
    Name        : selectAllCurrentPlayersHyperspacingShips
    Description : Selects all of the current player's hyperspacing ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SelectCommand *selectAllCurrentPlayersHyperspacingShips(void)
{
    SelectCommand *selection;
    sdword i = 0;
    Node *objnode;
    Ship *ship;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"selectall",0);

    i = 0;
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (allianceIsShipAlly(ship,universe.curPlayerPtr))
        {
            if (ship->staticinfo->shipIsCapital ||
                ship->staticinfo->shipclass == CLASS_Component)
            {
                selection->ShipPtr[i++] = ship;
            }
        }

        objnode = objnode->next;
    }

    selection->numShips = i;
    ioUpdateShipTotals();

    return selection;
}

/*-----------------------------------------------------------------------------
    Name        : selectAllPlayersShips
    Description : selects all of a player's ships
    Inputs      : player
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
SelectCommand *selectAllPlayersShips(struct Player *player)
{
    SelectCommand *selection;
    sdword i = 0;
    Node *objnode;
    Ship *ship;

    selection = memAlloc(sizeofSelectCommand(universe.ShipList.num),"selectall",0);

    i = 0;
    objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (ship->playerowner == player)
        {
            selection->ShipPtr[i++] = ship;
        }

        objnode = objnode->next;
    }

    selection->numShips = i;

    return selection;
}

/*-----------------------------------------------------------------------------
    Name        : MakeSelectionNonResearchShips(SelectCommand *selectcom)
    Description : Removes all research ship types from the selection
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

void MakeSelectionNonResearchShips(SelectCommand *selectcom)
{
    ubyte i;

    for (i = 0; i < selectcom->numShips;)
    {
        i++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsSingleClickSpecialCapable
    Description : Makes a selection of just ships that are have the
                    canSingleClickSpecialActivate flag
    Inputs      : source - selection to search through
    Outputs     : dest - where to store the culled selection
    Return      : number of ships in culled selection
----------------------------------------------------------------------------*/
sdword MakeShipsSingleClickSpecialCapable(SelectCommand *dest, SelectCommand *source)
{
    sdword i = 0;

    dbgAssert(dest == source);

    while (i < dest->numShips)
    {
        Ship *ship = dest->ShipPtr[i];

        if (ship->objtype != OBJ_ShipType ||
            !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPAIR) ||
            ship->shiptype == sProcessor ||
            ship->shiptype == bProcessor)
        {
            dest->numShips--;
            dest->ShipPtr[i] = dest->ShipPtr[dest->numShips];
        }
        else
        {
            i++;
        }
    }

    ioUpdateShipTotals();
    return dest->numShips;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsNotDocked
    Description : Makes a selection of ships that are not hiddel
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword MakeShipsNotHidden(SelectCommand *dest, SelectCommand *source)
{
    sdword index, numShips = 0;

    for (index = 0; index < source->numShips; index++)
    {
        if (!bitTest(source->ShipPtr[index]->flags, SOF_Hide))
        {
            dest->ShipPtr[numShips] = source->ShipPtr[index];
            numShips++;
        }
    }
    dest->numShips = numShips;
    ioUpdateShipTotals();
    return(numShips);
}

/*-----------------------------------------------------------------------------
    Name        : selectSelectionInterception
    Description : Returns a selection consisting of ships in both selection1 and selection2
    Inputs      : selection1, selection2
    Outputs     : Creates a new selection
    Return      : the new selection, NULL if there's no intersection
----------------------------------------------------------------------------*/
SelectCommand *selectSelectionIntersection(SelectCommand *selection1, SelectCommand *selection2)
{
    SelectCommand *smallerSelection, *largerSelection;
    SelectCommand *intersection = NULL;
    ShipPtr ship;
    sdword i;

    if (selection1->numShips < selection2->numShips)
    {
        smallerSelection = selection1;
        largerSelection  = selection2;
    }
    else
    {
        smallerSelection = selection2;
        largerSelection  = selection1;
    }

    intersection = selectDupSelection(smallerSelection);

    for (i=intersection->numShips-1; i >= 0; i--)
    {
        ship = intersection->ShipPtr[i];

        if (!ShipInSelection(largerSelection, ship))
        {
            clRemoveShipFromSelection(intersection, ship);
        }
    }

    if (!intersection->numShips)
    {
        memFree(intersection);
        intersection = NULL;
    }

    return intersection;
}




/*-----------------------------------------------------------------------------
    Name        : selectMergeTwoSelections
    Description : Takes two selections and makes one big one. Note that a new selection
                  is allocated in memory.
    Inputs      : selection1, selection2 - the selections to merge
    Outputs     : Allocates memory space for a new selection, frees the other two spaces
    Return      : The new selection
----------------------------------------------------------------------------*/
SelectCommand *selectMergeTwoSelections(SelectCommand *selection1, SelectCommand *selection2, udword dealloc)
{
    SelectCommand *new_selection;
    udword i, j;

    if (selection1 == NULL)
    {
        //if selection1 is NULL, copy selection2 into the new selection
        new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection2->numShips), "merge_sel2", 0);
        new_selection->numShips = selection2->numShips;

        for (i = 0; i < selection2->numShips; i++)
        {
            new_selection->ShipPtr[i] = selection2->ShipPtr[i];
        }
        if ((dealloc == DEALLOC2) || (dealloc == DEALLOC_BOTH))
        {
            memFree(selection2);
        }
    }
    else if (selection2 == NULL)
    {
        //if selection2 is NULL, copy selection1 into the new selection
        new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection1->numShips), "merge_sel1", 0);
        new_selection->numShips = selection1->numShips;

        for (i = 0; i < selection1->numShips; i++)
        {
            new_selection->ShipPtr[i] = selection1->ShipPtr[i];
        }
        if ((dealloc == DEALLOC1) || (dealloc == DEALLOC_BOTH))
        {
            memFree(selection1);
        }
    }
    else
    {
        MakeShipsNotIncludeTheseShips(selection1, selection2);

        new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection1->numShips + selection2->numShips), "merge_sel", 0);

        for (i = 0; i < selection1->numShips; i++)
        {
            new_selection->ShipPtr[i] = selection1->ShipPtr[i];
        }
        for (j = 0; j < selection2->numShips; j++, i++)
        {
            new_selection->ShipPtr[i] = selection2->ShipPtr[j];
        }

        new_selection->numShips = i;

        switch (dealloc)
        {
            case DEALLOC1:
                memFree(selection1);
                break;
            case DEALLOC2:
                memFree(selection2);
                break;
            case DEALLOC_BOTH:
                memFree(selection1);
                memFree(selection2);
                break;
            default:
                break;
        }
    }


    return new_selection;
}


/*-----------------------------------------------------------------------------
    Name        : selectMemDupSelection
    Description : Creates a new selection that is an exact duplicate of another selection
    Inputs      : selection - the selection to duplicate
                  flag - the memory allocation flag
    Outputs     : Allocates memory for the new selection
    Return      : The new selection
----------------------------------------------------------------------------*/
SelectCommand *selectMemDupSelection(SelectCommand *selection, char *str, udword mmflag)
{
    SelectCommand *new_selection;

    new_selection = (SelectCommand *)memAlloc(sizeofSelectCommand(selection->numShips), str, mmflag);

    memcpy(new_selection,selection,sizeofSelectCommand(selection->numShips));

    return new_selection;
}

/*-----------------------------------------------------------------------------
    Name        : MothershipOrCarrierIndexInSelection
    Description : returns index where a mothership or carrier is in selection.  -1 if none found.
                  Mothership has higher priority
    Inputs      :
    Outputs     :
    Return      : returns index where a mothership or carrier is in selection.  -1 if none found
----------------------------------------------------------------------------*/
sdword MothershipOrCarrierIndexInSelection(SelectCommand *selection)
{
    sdword numShips = selection->numShips;
    sdword index = -1;
    sdword i;
    ShipType type;

    // go through selection to find index where a mothership or carrier is.  Give higher priority to motherships.
    for (i=0;i<numShips;i++)
    {
        type = selection->ShipPtr[i]->shiptype;
        if (ShipTypeIsMothershipType(type))
        {
            return i;
        }
        else if (ShipTypeIsCarrierType(type))
        {
            index = i;
        }
    }

    return index;
}

/*=============================================================================
    Grow select functions
=============================================================================*/

void growSelectAddShip(GrowSelection *growSelect,Ship *ship)
{
    // reserve more space if required
    if (growSelect->selection->numShips >= growSelect->maxNumShips)
    {
        growSelect->maxNumShips += GROWSELECT_ADDBATCH;
        growSelect->selection = memRealloc(growSelect->selection, sizeofSelectCommand(growSelect->maxNumShips), "regrowships", 0);
    }

    growSelect->selection->ShipPtr[growSelect->selection->numShips] = ship;
    growSelect->selection->numShips += 1;
}

bool32 growSelectRemoveShip(GrowSelection *growSelect,Ship *ship)
{
    return clRemoveShipFromSelection(growSelect->selection,ship);
}

/*-----------------------------------------------------------------------------
    Name        : growSelectRemoveShipBySettingNULL
    Description : Remove ship references from a grow selection
    Inputs      :
    Outputs     :
    Return      : TRUE if a ship reference was removed
----------------------------------------------------------------------------*/
bool32 growSelectRemoveShipBySettingNULL(GrowSelection *growSelect,Ship *ship)
{
    SelectCommand *selection = growSelect->selection;
    sdword numShips;
    sdword i;

    if (selection == NULL)
    {
        return FALSE;
    }
    numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        if (selection->ShipPtr[i] == ship)
        {
            selection->ShipPtr[i] = NULL;
            return TRUE;
        }
    }

    return FALSE;
}

void growSelectRemoveShipIndex(GrowSelection *growSelect,sdword index)
{
    SelectCommand *selection = growSelect->selection;
    selection->numShips--;
    selection->ShipPtr[index] = selection->ShipPtr[selection->numShips];
}

void growSelectInit(GrowSelection *growSelect)
{
    growSelect->maxNumShips = GROWSELECT_INITIALBATCH;
    growSelect->selection = memAlloc(sizeofSelectCommand(GROWSELECT_INITIALBATCH), "growships", 0);
    growSelect->selection->numShips = 0;
}

void growSelectClose(GrowSelection *growSelect)
{
    if (growSelect->selection)
    {
        memFree(growSelect->selection);
        growSelect->selection = NULL;
    }
    growSelect->maxNumShips = 0;
}

void growSelectReset(GrowSelection *growSelect)
{
    growSelect->selection->numShips = 0;
}

void growSelectAddShipNoDuplication(GrowSelection *growSelect, Ship *ship)
{
    if (!ShipInSelection(growSelect->selection, ship))
    {
        growSelectAddShip(growSelect, ship);
    }
}


bool32 MakeSelectionKamikazeCapable(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        ShipClass shipClass = selection->ShipPtr[i]->staticinfo->shipclass;

        if ((shipClass != CLASS_Fighter) &&
            (shipClass != CLASS_Corvette) &&
            (shipClass != CLASS_Resource))
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            continue;
        }
        i++;
    }
    ioUpdateShipTotals();
    return (selection->numShips > 0);
}




bool32 shipIsBeastShip(Ship *ship)
{
    return ship != NULL &&
           ship->staticinfo != NULL &&
           (ship->staticinfo->shipFlags & SSTATF_BeastShip) != 0;
}

/* selectAllCurrentPlayersHyperspacingShips (0x512f80): live implementation above at ~1596.
   Binary version uses memAllocFunction and raw staticInfo+680/+488 checks instead of
   ShipCanHyperspace API — noted for future Cat2 review. */

// Cataclysm-specific: filters un-docked ships of scuttleable ship types, firing chatter before removal
int *__cdecl makeShipsScuttleable(int *a1, int a2)
{
    sdword count = *a1;
    sdword kept  = 0;
    sdword idx   = 2;

    while (kept < count)
    {
        Ship *ship = (Ship *)a1[idx];
        if (a2 && !ship->specialFlags3)
        {
            ShipType shipType = ship->shiptype;
            switch (shipType)
            {
            case sCarrier:
            case sMothership:
            case bCarrier:
            case bMothership:
                if (battleCanChatterAtThisTime(38, selSelected.ShipPtr[0]))
                    battleChatterAttempt(-1, 38, selSelected.ShipPtr[0],
                                        selSelected.ShipPtr[0]->shiprace);
                break;
            default:
                goto keepShip;
            }
            count--;
            *a1 = count;
            a1[idx] = a1[count + 2];
        }
        else
        {
keepShip:
            kept++;
            idx++;
        }
        count = *a1;
    }
    ioUpdateShipTotals();
    return a1;
}

// Cataclysm-specific: filters a selection to only ships owned by the same player as the first ship
int *__cdecl MakeShipsSameOwnerAsFirstShip(int *a1)
{
    sdword count = *a1;
    sdword kept  = 0;
    sdword idx   = 2;

    while (kept < count)
    {
        if (*(sdword *)((ubyte *)a1[idx] + 812) == *(sdword *)((ubyte *)a1[2] + 812))
        {
            kept++;
            idx++;
        }
        else
        {
            count--;
            *a1 = count;
            a1[idx] = a1[count + 2];
        }
        count = *a1;
    }
    ioUpdateShipTotals();
    return a1;
}

void makeShipsCommandControllable(SelectCommand *selection, sdword requireCommand)
{
    sdword i = 0;

    while (i < selection->numShips)
    {
        Ship *ship = selection->ShipPtr[i];
        bool32 remove = FALSE;

        if (requireCommand && shipHasCataBusyState(ship))
        {
            remove = TRUE;
        }
        else
        {
            struct CommandToDo *command = ship->command;
            if (command != NULL)
            {
                switch (*(uword *)&command->ordertype)
                {
                    case 4:     /* COMMAND_LAUNCHSHIP */
                    case 6:     /* COMMAND_BUILDINGSHIP */
                    case 0xA:
                    case 0xB:
                    case 0xC:
                    case 0xE:
                    case 0xF:
                    case 0x10:
                    case 0x11:
                        remove = TRUE;
                        break;
                    default:
                        break;
                }
            }
        }

        if (remove)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
        }
        else
        {
            i++;
        }
    }
    ioUpdateShipTotals();
}

// Cataclysm-specific: filters a selection down to ships that can use the Beast infection special.
int *__cdecl MakeShipsInfectionCapable(int *selectionData, int requireFreeOrInfected)
{
    sdword i = 0;

    while (i < selectionData[0])
    {
        Ship *ship = (Ship *)selectionData[i + 2];

        if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY) &&
            (!requireFreeOrInfected || !shipHasCataBusyState(ship) || shipselectIsShipInfected(ship)))
        {
            i++;
        }
        else
        {
            selectionData[0]--;
            selectionData[i + 2] = selectionData[selectionData[0] + 2];
        }
    }

    ioUpdateShipTotals();
    return selectionData;
}

// Cataclysm-specific: filters a selection down to ships that have the requested special upgrade.
int *__cdecl MakeShipsOnlyHaveUpgrade(int *selectionData, int upgradeMask)
{
    sdword i = 0;

    while (i < selectionData[0])
    {
        if (ShipHasUpgrade((Ship *)selectionData[i + 2], upgradeMask))
        {
            i++;
        }
        else
        {
            selectionData[0]--;
            selectionData[i + 2] = selectionData[selectionData[0] + 2];
        }
    }

    ioUpdateShipTotals();
    return selectionData;
}

// Cataclysm-specific: filters a selection down to Acolytes with link missiles armed.
int *__cdecl MakeShipsOnlyAcolytesWithLinkMissiles(int *selectionData)
{
    sdword i = 0;

    while (i < selectionData[0])
    {
        Ship *ship = (Ship *)selectionData[i + 2];

        // Retail checks only that this entry is still a ship object plus the Acolyte missile-ready state.
        if (ship->objtype == OBJ_ShipType && acolyteIsLinked(ship))
        {
            i++;
        }
        else
        {
            selectionData[0]--;
            selectionData[i + 2] = selectionData[selectionData[0] + 2];
        }
    }

    ioUpdateShipTotals();
    return selectionData;
}


// Cataclysm-specific: filters a selection down to infected Beast-special ships only.
int *__cdecl MakeShipsOnlyInfected(int *selectionData)
{
    sdword i = 0;

    while (i < selectionData[0])
    {
        Ship *ship = (Ship *)selectionData[i + 2];

        if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY) && IsShipInfected(ship))
        {
            i++;
        }
        else
        {
            selectionData[0]--;
            selectionData[i + 2] = selectionData[selectionData[0] + 2];
        }
    }

    ioUpdateShipTotals();
    return selectionData;
}


int *__cdecl MakeShipsMoveCapable(int *selectionData, int requireCommand)
{
    sdword i = 0;

    while (i < selectionData[0])
    {
        Ship *ship = (Ship *)selectionData[i + 2];
        ShipStaticInfo *shipStatic = (ShipStaticInfo *)ship->staticinfo;

        if (shipStatic->staticheader.immobile ||
            (requireCommand && shipHasCataBusyState(ship)))
        {
            if (!shipStatic->shipclass &&
                ship->playerowner == universe.curPlayerPtr)
            {
                speechEventQueue(0, 67112987, 0, -1, 0, -1, -1, 5.0f, -1);
            }

            selectionData[0]--;
            selectionData[i + 2] = selectionData[selectionData[0] + 2];
        }
        else
        {
            i++;
        }
    }

    ioUpdateShipTotals();
    return selectionData;
}

// Cataclysm-specific: filters a selection to only ships that can be retired or scuttled
int *__cdecl MakeShipsCanBeRetiredOrScuttled(int *selectionData)
{
    sdword count = *selectionData;
    sdword kept  = 0;
    sdword idx   = 2;

    while (kept < count)
    {
        Ship *ship = (Ship *)selectionData[idx];
        if (ship->staticinfo->shipIsCapital)
        {
            count--;
            *selectionData = count;
            selectionData[idx] = selectionData[count + 2];
        }
        else
        {
            kept++;
            idx++;
        }
        count = *selectionData;
    }
    ioUpdateShipTotals();
    return selectionData;
}

// Cataclysm-specific: filters a selection to only enemy ships (removes friendly, docked, or special types)
int *__cdecl MakeShipsOnlyEnemyShips(int *a1, int a2)
{
    sdword count = *a1;
    sdword kept  = 0;
    sdword idx   = 2;

    while (kept < count)
    {
        sdword *ship = (sdword *)a1[idx];
        sdword  shipType = ship[172]; /* offset 688 = shiptype */
        if (ship[4]                         /* offset 16 - flags/docked check */
            || ship[203] == a2              /* offset 812 = playerowner */
            || (ship[114] & 0x40000) != 0  /* offset 456 flags bit */
            || univIsObjFriendly((SpaceObjRotImpTarg *)ship, (Player *)a2)
            || shipType == 126
            || shipType == 127)
        {
            count--;
            *a1 = count;
            a1[idx] = a1[count + 2];
        }
        else
        {
            kept++;
            idx++;
        }
    }
    ioUpdateShipTotals();
    return a1;
}

// Cataclysm-specific: removes crystals (BigGunUnit) from a selection, excluding docked ones
int *__cdecl MakeSelectionNotHaveCrystals(int *a1)
{
    sdword kept = 0;
    sdword idx  = 2;

    MakeTargetsOnlyNonForceAttackTargets(a1, 0);

    while (kept < *a1)
    {
        if (*(sdword *)((ubyte *)a1[idx] + 16) || !shipselectIsCrystal((Ship *)a1[idx]))
        {
            kept++;
            idx++;
        }
        else
        {
            sdword count = *a1 - 1;
            *a1 = count;
            a1[idx] = a1[count + 2];
        }
    }
    ioUpdateShipTotals();
    return a1;
}

// Cataclysm-specific: filters a selection to only ships owned by the specified player
int *__cdecl MakeShipsOwnedByPlayer(int *a1, int a2)
{
    sdword count = *a1;
    sdword kept  = 0;
    sdword idx   = 2;

    while (kept < count)
    {
        sdword ship = a1[idx];
        if (((Ship *)ship)->objtype || (sdword)((Ship *)ship)->playerowner != a2)
        {
            count--;
            *a1 = count;
            a1[idx] = a1[count + 2];
        }
        else
        {
            kept++;
            idx++;
        }
        count = *a1;
    }
    ioUpdateShipTotals();
    return a1;
}


/* selectSelectionIntersection (0x513060) - live implementation at line ~1753 above, using
   selectDupSelection/ShipInSelection/clRemoveShipFromSelection API which matches the binary logic */

bool32 shipIsMoveCapable(Ship *ship, bool32 requireNotDocked)
{
    if (ship->staticinfo->staticheader.immobile)
    {
        return FALSE;
    }

    return shipIsNotDocked(ship, requireNotDocked);
}

bool32 shipCanUseInfectionAbility(Ship *ship, bool32 requireNotDocked)
{
    if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY))
    {
        return FALSE;
    }

    if (!requireNotDocked || !shipHasCataBusyState(ship))
    {
        return TRUE;
    }

    return shipselectIsShipInfected(ship);
}

bool32 shipIsInfected(Ship *ship)
{
    return ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MICROSHIP_PRIMARY) && shipselectIsShipInfected(ship);
}

bool32 acolyteIsLinked(Ship *ship)
{
    GunInfo *gunInfo;

    if (ship->shiptype != sAcolyte && ship->shiptype != bAcolyte)
    {
        return FALSE;
    }
    if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_MISSILE))
    {
        return FALSE;
    }

    gunInfo = ship->cataclysmGunInfo;
    dbgAssert(gunInfo != NULL);
    dbgAssert(gunInfo->numGuns > 3);
    return gunInfo->guns[2].numMissiles > 0 || gunInfo->guns[3].numMissiles > 0;
}

bool32 shipHasModuleSlotState(Ship *ship)
{
    return (ship->flags2 & BIT3) == BIT3 && *(ubyte *)&shipModuleSlotRuntime(ship)->state != 0;
}

bool32 shipModuleSlotIsEligible(Ship *ship, Player *player)
{
    return ship->objtype == OBJ_ShipType &&
           shipTypeSupportsModuleSlots(ship->shiptype) &&
           !shipGetSlotStatus(ship, player->playerIndex) &&
           (ubyte)shipModuleSlotRuntime(ship)->state == 1 &&
           shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetType == -1 &&
           (ship->health / ship->staticinfo->maxhealth) > 0.4f;
}


void MakeShipsOnlyRammingFrigates(SelectCommand *selection, bool32 requireNotDocked)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        Ship *ship = selection->ShipPtr[i];

        if (((ship->shiptype == sRammingFrigate) || (ship->shiptype == bRammingFrigate)) &&
            shipIsNotDocked(ship, requireNotDocked))
        {
            i++;
            continue;
        }

        selection->numShips--;
        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
    }

    ioUpdateShipTotals();
}


void MakeShipsNotDocked(SelectCommand *selection)
{
    sdword i = 0;

    while (i < selection->numShips)
    {
        if (selection->ShipPtr[i]->dockvars.dockship != NULL)
        {
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
        }
        else
        {
            i++;
        }
    }

    ioUpdateShipTotals();
}


bool32 shipCanSingleClickSpecialActivate(Ship *ship)
{
    if ((ship->objtype != OBJ_ShipType) || !ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_REPAIR))
    {
        return FALSE;
    }

    return (ship->shiptype != sProcessor) && (ship->shiptype != bProcessor);
}

bool32 shipCanSpecialTarget(Ship *ship)
{
    return (ship->objtype == OBJ_ShipType) && ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_SALVAGE);
}

bool32 shipCanBeSpecialTargeted(Ship *ship)
{
    uword reasonCounts[9] = { 0 };

    if (singlePlayerGame &&
        singlePlayerGameInfo.currentMission == MISSION_10_SUPER_NOVA_STATION &&
        ship->shiptype == dBigGun)
    {
        return TRUE;
    }

    if (SalCapCanTargetShipWithReasonCounts(
            universe.curPlayerPtr,
            (SpaceObjRotImpTargGuidanceShipDerelict *)ship,
            reasonCounts))
    {
        return TRUE;
    }

    return reasonCounts[0] == 0 &&
           reasonCounts[2] == 0 &&
           reasonCounts[4] == 0 &&
           reasonCounts[5] == 0 &&
           reasonCounts[8] == 0;
}

int *__cdecl MakeShipsUpgradeCapable(int *selectionData, int requireIdle)
{
    sdword i = 0;

    while (i < selectionData[0])
    {
        Ship *ship = (Ship *)selectionData[i + 2];

        if (requireIdle && shipHasCataBusyState(ship) && !ship->upgradeActive)
        {
            selectionData[0]--;
            selectionData[i + 2] = selectionData[selectionData[0] + 2];
        }
        else
        {
            i++;
        }
    }

    ioUpdateShipTotals();
    return selectionData;
}

void MakeShipsNotDockedExceptSentinelLeechTypes(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        ShipType shipType = selection->ShipPtr[i]->shiptype;

        if ((selection->ShipPtr[i]->dockvars.dockship == NULL) ||
            (shipType == sLeech) ||
            (shipType == sSentinel) ||
            (shipType == bSentinel))
        {
            i++;
            continue;
        }

        selection->numShips--;
        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
    }

    ioUpdateShipTotals();
}

bool32 shipIsNotDocked(Ship *ship, bool32 requireNotDocked)
{
    return !requireNotDocked || (ship->dockvars.dockship == NULL);
}

void MakeShipsDockCapableWithShipType(SelectCommand *selection, ShipType dockShipType, bool32 requireNotDocked)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        Ship *ship = selection->ShipPtr[i];

        if (shipIsNotDocked(ship, requireNotDocked) &&
            shipClassCanDockWithShipType(ship->staticinfo->shipclass, dockShipType))
        {
            i++;
            continue;
        }

        selection->numShips--;
        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
    }

    ioUpdateShipTotals();
}

bool32 shipClassCanDockWithShipType(ShipClass shipClass, ShipType dockShipType)
{
    sdword dockSize;
    sdword requiredSize;

    switch (dockShipType)
    {
        case dSlipgateSmall:
            dockSize = 1;
            break;
        case dSlipgateMedium:
            dockSize = 2;
            break;
        case dSlipgateLarge:
            dockSize = 3;
            break;
        default:
            dockSize = 0;
            break;
    }

    switch (shipClass)
    {
        case CLASS_Mothership:
        case CLASS_MonsterShip:
            requiredSize = 3;
            break;
        case CLASS_HeavyCruiser:
        case CLASS_Carrier:
        case CLASS_Destroyer:
        case CLASS_Frigate:
        case CLASS_Resource:
            requiredSize = 2;
            break;
        case CLASS_Corvette:
        case CLASS_Fighter:
        case CLASS_NonCombat:
        case CLASS_Component:
        case CLASS_Micro:
            requiredSize = 1;
            break;
        default:
            requiredSize = 4;
            break;
    }

    return dockSize >= requiredSize;
}

void MakeShipsLeechCapable(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; )
    {
        Ship *ship = selection->ShipPtr[i];

        if (ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_1) ||
            ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_2) ||
            ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_3))
        {
            i++;
            continue;
        }

        selection->numShips--;
        selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
    }

    ioUpdateShipTotals();
}
