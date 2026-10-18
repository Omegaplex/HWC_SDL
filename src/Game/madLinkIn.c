/*=============================================================================
    Name    : madLinkIn.c
    Purpose : contains all code used to bind mesh animations to
              actual events.  Esentially a hard coded nightmare.

    Created 31/08/1998 by bryce pasechnik
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

//#include "meshanim.h"
#include <stdlib.h>

#include "debug.h"
#include "commandlayer.h"
#include "universe.h"
#include "spaceobj.h"
#include "madLinkIn.h"
#include "madLinkInDefs.h"
#include "types.h"
#include "mothership.h"
#include "randy.h"
#include "soundevent.h"

#define shaneyPooSoundEventAnimationTypeFlag soundEventAnimationTypeFlag
#define Mothership sMothership
#define HeavyDefender kHeavyDefender
#define HeavyCorvette kHeavyCorvette
#define Probe kProximitySensor

#ifdef bpasechn
    #define DEBUG_MESH_ANIMATIONS
#endif

int madOpenSpecialShip(Ship *ship);
int madCloseSpecialShip(Ship *ship);


//Variables in ship structure to support mesh animations (amongst others)
//
//    udword madAnimationFlags;        //general purpose flags for mad animations
//    real32 madAnimationTime;        //general purpose variable for counting
//    udword cuedAnimation[MAD_MAX_CUEABLE];            //Animation index waiting in the wings to be played

/*-----------------------------------------------------------------------------
    Name        : madLinkInInit
    Description : Loads in mad script file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInInit()
{

}


//    udword madAnimationFlags;        //general purpose flags for mad animations
//    real32 madAnimationTime;        //general purpose variable for counting
//    udword cuedAnimation[MAD_MAX_CUEABLE];            //Animation index waiting in the wings to be played


/*-----------------------------------------------------------------------------
    Name        : madLinkInUpdateMeshAnimations
    Description : starts,stops and does whatever needs to be done
                  for mesh animations based on code given cues.
    Inputs      : Ship to have animation updated
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInUpdateMeshAnimations(Ship *ship)
{
    if(bitTest(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION))
    {
        madAnimationStart(ship, ship->cuedAnimationIndex);
        soundEventPlay(ship, ship->soundEventAnimationTypeFlag, NULL);
        bitClear(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
    else if (madAnimationUpdate(ship, universe.unknown1))
    {
#ifdef DEBUG_MESH_ANIMATIONS
        dbgMessagef("\nAnimation Finished.");
#endif
        switch(ship->cuedAnimationType)
        {
        case MAD_ANIMATION_GUN_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madGunStatus = MAD_STATUS_GUNS_OPEN;
            break;
        case MAD_ANIMATION_GUN_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madGunStatus = MAD_STATUS_GUNS_CLOSED;
            //setup closed gun hack...
            madAnimationStart(ship, ship->staticinfo->madStatic->gunOpenIndexes[0]);    //start 0th animation
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        case MAD_ANIMATION_WINGS_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madWingStatus = MAD_STATUS_WINGS_OPEN;
            break;
        case MAD_ANIMATION_WINGS_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madWingStatus = MAD_STATUS_WINGS_CLOSED;
            //setup closed gun hack...
            madAnimationStart(ship, ship->staticinfo->madStatic->PostDockIndexes[0]);   //start 0th animation
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        case MAD_ANIMATION_DOOR_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madDoorStatus = MAD_STATUS_DOOR_OPEN;
            madAnimationStart(ship, ship->staticinfo->madStatic->DoorCloseIndexes[0]);  //start 0th animaiton
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        case MAD_ANIMATION_DOOR_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madDoorStatus = MAD_STATUS_DOOR_CLOSED;
            break;
        case MAD_ANIMATION_SPECIAL_OPENING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madSpecialStatus = MAD_STATUS_SPECIAL_OPEN;
            break;
        case MAD_ANIMATION_SPECIAL_CLOSING:
            ship->cuedAnimationType = MAD_ANIMATION_NOTHING;
            ship->madSpecialStatus = MAD_STATUS_SPECIAL_CLOSED;
            madAnimationStart(ship, ship->staticinfo->madStatic->specialOpenIndexes[0]);    //start 0th animaiton
            madAnimationPause(ship,!ship->madBindings->bPaused);
            break;
        }

        bitClear(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->nextAnim != 0)
        {
            switch(ship->nextAnim)
            {
            case MAD_ANIMATION_GUN_OPENING:
                madOpenGunsShip(ship);
                ship->nextAnim = 0;
                break;
            case MAD_ANIMATION_GUN_CLOSING:
                madLinkInCloseGunsShip(ship);
                ship->nextAnim = 0;
                break;
            case 3:
                madLinkInOpenDoor(ship);
                ship->nextAnim = 0;
                break;
            case 4:
                madLinkInCloseDoor(ship);
                ship->nextAnim = 0;
                break;
            case 5:
                madOpenSpecialShip(ship);
                ship->nextAnim = 0;
                break;
            case 6:
                madCloseSpecialShip(ship);
                ship->nextAnim = 0;
                break;
            case 7:
                madLinkInOpenSpecialShip(ship);
                ship->nextAnim = 0;
                break;
            case 8:
                madLinkInCloseSpecialShip(ship);
                ship->nextAnim = 0;
                break;
            default:
                dbgFatalf(DBG_Loc,"\nUnknown animation in nextAnim variable.");
                ship->nextAnim = 0;
                break;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInSetUpInitialBuiltMadState
    Description : For ships like the defender, probe, etc...
                  they need to have animations started then paused
                  upon creation.  This function does that.
    Inputs      : Ship to have animation set up
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInSetUpInitialBuiltMadState(Ship *ship)
{
#ifdef DEBUG_MESH_ANIMATIONS
    dbgAssert(ship->madBindings != NULL);
#endif

    if(ship->staticinfo->madStatic->needStartBuiltAnimation)
    {
        //if ship has need to be set to a specific starting animation
        madAnimationStart(ship, ship->staticinfo->madStatic->startBuiltAnimationIndex);
        madAnimationPause(ship, !ship->madBindings->bPaused);
        madLinkInOpenSpecialShip(ship);
    }

}
/*-----------------------------------------------------------------------------
    Name        : madLinkInSetUpInitialBuiltMadState
    Description : For ships like the defender, probe, etc...
                  they need to have animations started then paused
                  upon creation.  This function does that.
    Inputs      : Ship to have animation set up
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInSetUpInitialPlacedMadState(Ship *ship)
{
#ifdef DEBUG_MESH_ANIMATIONS
    dbgAssert(ship->madBindings != NULL);
#endif

    if(ship->staticinfo->madStatic->needStartPlacedAnimation)
    {
        //if ship has need to be set to a specific starting animation
        madAnimationStart(ship, ship->staticinfo->madStatic->startPlacedAnimationIndex);
        madAnimationPause(ship, !ship->madBindings->bPaused);
        switch(ship->shiprace)
        {
        case RACE_Sect:
            switch(ship->shiptype)
            {
            case HeavyDefender:
                ship->madGunStatus = MAD_STATUS_GUNS_CLOSED;
                break;
            }
            break;
        case RACE_Beast:
            switch(ship->shiptype)
            {
            }
            break;
        case P1:
        case RACE_Taiidan:
        case RACE_Kushan:
        case Traders:
            break;
        default:
            dbgFatalf(DBG_Loc,"\nUnknown race type with a start up mesh animation (1).");
        }
    }

}

//General Comment:
//
//priority below:  1 - standard  (opens based on standard timings)
//                 2 - immediate (cues animation to play immediatly)

void madOpenGunsShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numGunOpenIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madGunStatus == MAD_STATUS_GUNS_OPEN ||
           ship->madGunStatus == MAD_STATUS_GUNS_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madGunStatus == MAD_STATUS_GUNS_CLOSING)
        {
            ship->nextAnim = MAD_ANIMATION_GUN_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health * ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numGunOpenDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunOpenDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunOpenDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedOpen;

        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunOpenIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunOpenIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
        }
        ship->cuedAnimationType = MAD_ANIMATION_GUN_OPENING;
        ship->madGunStatus = MAD_STATUS_GUNS_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInOpenGuns
    Description : Sets up ships with gun animations to open their guns imediatly (regardless of certain other things).
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInOpenGuns(CommandToDo *command,udword priority)
{
    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madOpenGunsShip(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInCloseGuns
    Description : Sets the ship to imediatly begin a gun closing animation
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInCloseGuns(CommandToDo *command,udword priority)
{
    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        if (ship->shiptype != bIonArrayFrigate &&
            ship->shiptype != p1IonArrayFrigate)
        {
            madLinkInCloseGunsShip(ship);
        }
    }
}

void madLinkInCloseGunsShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numGunCloseIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }
        if(ship->madGunStatus == MAD_STATUS_GUNS_CLOSED ||
           ship->madGunStatus == MAD_STATUS_GUNS_CLOSING)
        {
            //guns are currently open/opening...so lets ferget about it
            return;
        }
        else if(ship->madGunStatus == MAD_STATUS_GUNS_OPENING)
        {
            ship->nextAnim = MAD_ANIMATION_GUN_CLOSING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health * ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numGunCloseDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunCloseDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunCloseDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedClose;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numGunCloseIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->gunCloseIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
		}
        ship->cuedAnimationType = MAD_ANIMATION_GUN_CLOSING;
        ship->madGunStatus = MAD_STATUS_GUNS_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInPostDocking
    Description : Cues up animations for ships that have finished
                  docking (opens fins, etc...)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPostDocking(CommandToDo *command,udword priority)
{
    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madLinkInPostDockingShip(ship);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInPostDockingShip
    Description : utility funciton for above function
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPostDockingShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numDockIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madWingStatus == MAD_STATUS_WINGS_OPEN ||
           ship->madWingStatus == MAD_STATUS_WINGS_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madWingStatus == MAD_STATUS_WINGS_CLOSING)
        {
            ship->nextAnim = MAD_ANIMATION_WINGS_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health * ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madWingOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numPostDockDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numPostDockDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->PostDockDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedOpen;
		}
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numPostDockIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->PostDockIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
        }
        ship->cuedAnimationType = MAD_ANIMATION_WINGS_OPENING;
        ship->madWingStatus = MAD_STATUS_WINGS_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInPreDocking
    Description : Cues up animations for ships that are docking (closes fins, etc...)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPreDocking(CommandToDo *command,udword priority)
{

    sdword i;
    Ship *ship;

    for(i=0;i<command->selection->numShips;i++)
    {
        ship = command->selection->ShipPtr[i];
        madLinkInPreDockingShip(ship);
    }
}
/*-----------------------------------------------------------------------------
    Name        : madLinkInPreDockingShip
    Description : utility function for above function
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInPreDockingShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numPostDockIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madWingStatus == MAD_STATUS_WINGS_CLOSED ||
           ship->madWingStatus == MAD_STATUS_WINGS_CLOSING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madWingStatus == MAD_STATUS_WINGS_OPENING)
        {
            ship->nextAnim = MAD_ANIMATION_WINGS_CLOSING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health * ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madWingOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numDockDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numDockDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->DockDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedClose;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numDockIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->DockIndexes[animIndex];
            ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
        }
        ship->cuedAnimationType = MAD_ANIMATION_WINGS_CLOSING;
        ship->madWingStatus = MAD_STATUS_WINGS_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInOpenDoor
    Description : opens door on R1 mothership (for now)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInOpenDoor(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numDoorOpenIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madDoorStatus == MAD_STATUS_DOOR_OPEN ||
           ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madDoorStatus == MAD_STATUS_DOOR_CLOSING)
        {
            //ALWAYS return;
            ship->nextAnim = MAD_ANIMATION_DOOR_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        animIndex = randombetween(0,(ship->staticinfo->madStatic->numDoorOpenIndexes-1));
        ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorOpenIndexes[animIndex];

        ship->cuedAnimationType = MAD_ANIMATION_DOOR_OPENING;
        ship->madDoorStatus = MAD_STATUS_DOOR_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
		ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInCloseDoor
    Description : opens door on R1 mothership (for now)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInCloseDoor(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numDoorCloseIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madDoorStatus == MAD_STATUS_DOOR_CLOSED ||
           ship->madDoorStatus == MAD_STATUS_DOOR_CLOSING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
        {
            //ALWAYS return;
            ship->nextAnim = MAD_ANIMATION_DOOR_CLOSING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        animIndex = randombetween(0,(ship->staticinfo->madStatic->numDoorCloseIndexes-1));
        ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorCloseIndexes[animIndex];

        ship->cuedAnimationType = MAD_ANIMATION_DOOR_CLOSING;
        ship->madDoorStatus = MAD_STATUS_DOOR_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
		ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
    }
}

int madOpenSpecialShip(Ship *ship)
{
    sdword animIndex;

    if (ship->madBindings == NULL || ship->staticinfo->madStatic == NULL)
    {
        return FALSE;
    }

    if (ship->staticinfo->madStatic->numDoorOpenIndexes < 1)
    {
        return TRUE;
    }

    if (ship->madDoorStatus == MAD_STATUS_DOOR_OPEN ||
        ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
    {
        return TRUE;
    }
    else if (ship->madDoorStatus == MAD_STATUS_DOOR_CLOSING)
    {
        ship->nextAnim = MAD_ANIMATION_DOOR_OPENING;
        return TRUE;
    }

    bitSet(ship->madAnimationFlags, MAD_ANIMATION_NEED_PROC);
    animIndex = randombetween(0, ship->staticinfo->madStatic->numDoorOpenIndexes - 1);
    ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorOpenIndexes[animIndex];
    ship->cuedAnimationType = MAD_ANIMATION_DOOR_OPENING;
    ship->madDoorStatus = MAD_STATUS_DOOR_OPENING;
    bitSet(ship->madAnimationFlags, MAD_NEED_TO_START_NEW_ANIMATION);
    ship->soundEventAnimationTypeFlag = ShipCmn_AnimOpening;
    return TRUE;
}

int madCloseSpecialShip(Ship *ship)
{
    sdword animIndex;

    if (ship->madBindings == NULL || ship->staticinfo->madStatic == NULL)
    {
        return FALSE;
    }

    if (ship->staticinfo->madStatic->numDoorCloseIndexes < 1)
    {
        return TRUE;
    }

    if (ship->madDoorStatus == MAD_STATUS_DOOR_CLOSED ||
        ship->madDoorStatus == MAD_STATUS_DOOR_CLOSING)
    {
        return TRUE;
    }
    else if (ship->madDoorStatus == MAD_STATUS_DOOR_OPENING)
    {
        ship->nextAnim = MAD_ANIMATION_DOOR_CLOSING;
        return TRUE;
    }

    bitSet(ship->madAnimationFlags, MAD_ANIMATION_NEED_PROC);
    animIndex = randombetween(0, ship->staticinfo->madStatic->numDoorCloseIndexes - 1);
    ship->cuedAnimationIndex = ship->staticinfo->madStatic->DoorCloseIndexes[animIndex];
    ship->cuedAnimationType = MAD_ANIMATION_DOOR_CLOSING;
    ship->madDoorStatus = MAD_STATUS_DOOR_CLOSING;
    bitSet(ship->madAnimationFlags, MAD_NEED_TO_START_NEW_ANIMATION);
    ship->soundEventAnimationTypeFlag = ShipCmn_AnimClosing;
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : getDoorInfo
    Description : fills in coordsys and position with the doors information for this ship
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

bool32 madLinkInGetDoorInfo(Ship *ship, matrix *coordsys, vector *position)
{
    sdword madIndex, gunIndex;
    vector positiont;
    matrix coordsyst;

    madIndex = madBindingIndexFindByName(ship->staticinfo->madStatic->header, "Hangardoor");
    gunIndex = madGunBindingIndexFindByName(ship->staticinfo, "Hangardoor");
    dbgAssert(madIndex != -1);
    dbgAssert(gunIndex != -1);
    madAnimBindingMatrix(&coordsyst,&positiont,ship,gunIndex, madIndex);

    matMultiplyMatByMat(coordsys,&ship->rotinfo.coordsys,&coordsyst);
    //matMultiplyVecByMat(position,&positionSS,&ship->rotinfo.coordsys);
    matMultiplyMatByVec(position,&ship->rotinfo.coordsys,&positiont);
    vecAddTo(*position,ship->posinfo.position);
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInOpenSpecialShip
    Description : runs the 'open' special animation of the ships.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInOpenSpecialShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numSpecialOpenIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }

        if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPEN ||
           ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPENING)
        {
            //guns are currently open...so lets ferget about it
            return;
        }
        else if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_CLOSING)
        {
            //toggle Reverse here!
            //Luke...Make it so.
            //for now, overwrite animation
            ship->nextAnim = MAD_ANIMATION_SPECIAL_OPENING;
            return;
        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health * ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numSpecialOpenDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialOpenDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialOpenDamagedIndexes[animIndex];
            ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedOpen;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialOpenIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialOpenIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimOpening;
        }
        ship->cuedAnimationType = MAD_ANIMATION_SPECIAL_OPENING;
        ship->madSpecialStatus = MAD_STATUS_SPECIAL_OPENING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}

/*-----------------------------------------------------------------------------
    Name        : madLinkInCloseSpecialShip
    Description : sets the 'close' special animation to play
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void madLinkInCloseSpecialShip(Ship *ship)
{
    sdword animIndex;
    if (ship->madBindings != NULL && ship->staticinfo->madStatic != NULL)
    {
        //if ship has mesh animations associated with it

        if(ship->staticinfo->madStatic->numSpecialCloseIndexes < 1)
        {
            //ship doesn't have gun opening animations
            return;
        }
        if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_CLOSED ||
           ship->madSpecialStatus == MAD_STATUS_SPECIAL_CLOSING)
        {
            //guns are currently open/opening...so lets ferget about it
            return;
        }
        else if(ship->madSpecialStatus == MAD_STATUS_SPECIAL_OPENING)

        {
            //cue this anim for play right after the current one finishings
            ship->nextAnim = MAD_ANIMATION_SPECIAL_CLOSING;
            return;

        }

        //signal that an animation has been added to the ships cue
        bitSet(ship->madAnimationFlags,MAD_ANIMATION_NEED_PROC);

        if(ship->health * ship->staticinfo->oneOverMaxHealth < ship->staticinfo->madGunOpenDamagedHealthThreshold && ship->staticinfo->madStatic->numSpecialCloseDamagedIndexes > 0)
        {
            //ships health is below threshold
            //and ship has damaged animations as well
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialCloseDamagedIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialCloseDamagedIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimDamagedClose;
        }
        else
        {
            //ships health is normalish..use normal gun anims
            animIndex = randombetween(0,(ship->staticinfo->madStatic->numSpecialCloseIndexes-1));
            ship->cuedAnimationIndex = ship->staticinfo->madStatic->specialCloseIndexes[animIndex];
			ship->shaneyPooSoundEventAnimationTypeFlag = ShipCmn_AnimClosing;
        }
        ship->cuedAnimationType = MAD_ANIMATION_SPECIAL_CLOSING;
        ship->madSpecialStatus = MAD_STATUS_SPECIAL_CLOSING;
        bitSet(ship->madAnimationFlags,MAD_NEED_TO_START_NEW_ANIMATION);
    }
}



#if 0  // Cat2: Binary madOpenGunsShip; live implementation at L295 uses typed Ship* and CustShipHeader (0x4A5560)
int __cdecl madOpenGunsShip(int ship)
{
    int result;
    int staticinfo;
    int madStatic;

    result = *(int *)(ship + 1212);
    if (!result)
        return result;

    staticinfo = *(int *)(ship + 32);
    result = *(int *)(staticinfo + 788);
    if (!result)
        return result;

    madStatic = result;
    if (*(int *)(madStatic + 20) < 1)
        return result;

    result = *(int *)(ship + 772);
    if (result == 1 || result == 3)
        return result;

    if (result == 4)
    {
        *(int *)(ship + 796) = 1;
        return result;
    }

    /* Set flags and select dodge maneuver */
    *(int *)(ship + 768) |= 1u;

    int dodgeTable;
    int dodgeCount;
    float healthThreshold = *(float *)(staticinfo + 168) * *(float *)(ship + 368);

    if (healthThreshold >= *(float *)(staticinfo + 792)
        || *(int *)(*(int *)(staticinfo + 788) + 24) <= 0)
    {
        madStatic = *(int *)(*(int *)(ship + 32) + 788);
        dodgeCount = ranRandomFn(10) % *(int *)(madStatic + 20);
        dodgeTable = *(int *)(madStatic + 36);
    }
    else
    {
        madStatic = *(int *)(*(int *)(ship + 32) + 788);
        dodgeCount = ranRandomFn(10) % *(int *)(madStatic + 24);
        dodgeTable = *(int *)(madStatic + 40);
    }

    int selectedDodge = *(int *)(dodgeTable + 4 * dodgeCount);
    result = *(int *)(ship + 768) | 2;
    *(int *)(ship + 788) = selectedDodge;
    *(int *)(ship + 800) = 1029;
    *(int *)(ship + 792) = 1;
    *(int *)(ship + 772) = 3;
    *(int *)(ship + 768) = result;

    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A57C0
void __cdecl madLinkInOpenDoors(Ship *ship)
{
int *result; // eax
int v2; // esi
int v3; // edi

result = *(int **)(a1 + 16);
v2 = 0;
if ( *result > 0 )
{
v3 = 2;
do
{
sub_4A5800(result[v3]);
result = *(int **)(a1 + 16);
++v2;
++v3;
}
while ( v2 < *result );
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A5800
int __cdecl madLinkInOpenDoor(Ship *ship)
{
int result; // eax
int v2; // ecx
int v3; // edi
udword v4; // edx
int v5; // eax
int v6; // edi
int v7; // ecx

result = *(_DWORD *)(a1 + 1212);
if ( result )
{
v2 = *(_DWORD *)(a1 + 32);
result = *(_DWORD *)(v2 + 788);
if ( result )
{
if ( *(int *)(result + 52) >= 1 )
{
result = *(_DWORD *)(a1 + 776);
if ( result != 5 && result != 7 )
{
if ( result == 8 )
{
*(_DWORD *)(a1 + 796) = 3;
}
else
{
*(_DWORD *)(a1 + 768) |= 1u;
if ( *(float *)(v2 + 168) * *(float *)(a1 + 368) >= *(float *)(v2 + 796)
|| *(int *)(*(_DWORD *)(v2 + 788) + 64) <= 0 )
{
v6 = *(_DWORD *)(*(_DWORD *)(a1 + 32) + 788);
v4 = ranRandomFn(10) % *(_DWORD *)(v6 + 60);
v5 = *(_DWORD *)(v6 + 76);
}
else
{
v3 = *(_DWORD *)(*(_DWORD *)(a1 + 32) + 788);
v4 = ranRandomFn(10) % *(_DWORD *)(v3 + 64);
v5 = *(_DWORD *)(v3 + 80);
}
v7 = *(_DWORD *)(v5 + 4 * v4);
result = *(_DWORD *)(a1 + 768) | 2;
*(_DWORD *)(a1 + 788) = v7;
*(_DWORD *)(a1 + 800) = 1029;
*(_DWORD *)(a1 + 792) = 3;
*(_DWORD *)(a1 + 776) = 7;
*(_DWORD *)(a1 + 768) = result;
}
}
}
}
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A5910
int __cdecl madLinkInCloseDoor(Ship *ship)
{
int result; // eax
int v2; // ecx
int v3; // edi
udword v4; // edx
int v5; // eax
int v6; // edi
int v7; // ecx

result = *(_DWORD *)(a1 + 1212);
if ( result )
{
v2 = *(_DWORD *)(a1 + 32);
result = *(_DWORD *)(v2 + 788);
if ( result )
{
if ( *(int *)(result + 60) >= 1 )
{
result = *(_DWORD *)(a1 + 776);
if ( result != 6 && result != 8 )
{
if ( result == 7 )
{
*(_DWORD *)(a1 + 796) = 4;
}
else
{
*(_DWORD *)(a1 + 768) |= 1u;
if ( *(float *)(v2 + 168) * *(float *)(a1 + 368) >= *(float *)(v2 + 796)
|| *(int *)(*(_DWORD *)(v2 + 788) + 56) <= 0 )
{
v6 = *(_DWORD *)(*(_DWORD *)(a1 + 32) + 788);
v4 = ranRandomFn(10) % *(_DWORD *)(v6 + 52);
v5 = *(_DWORD *)(v6 + 68);
}
else
{
v3 = *(_DWORD *)(*(_DWORD *)(a1 + 32) + 788);
v4 = ranRandomFn(10) % *(_DWORD *)(v3 + 56);
v5 = *(_DWORD *)(v3 + 72);
}
v7 = *(_DWORD *)(v5 + 4 * v4);
result = *(_DWORD *)(a1 + 768) | 2;
*(_DWORD *)(a1 + 788) = v7;
*(_DWORD *)(a1 + 800) = 1030;
*(_DWORD *)(a1 + 792) = 4;
*(_DWORD *)(a1 + 776) = 8;
*(_DWORD *)(a1 + 768) = result;
}
}
}
}
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A5A20
int __cdecl madOpenSpecialShip(Ship *ship)
{
int result; // eax
int v2; // ecx
int v3; // edi
int v4; // ecx

result = a1[303];
if ( result )
{
result = *(_DWORD *)(a1[8] + 788);
if ( result )
{
if ( *(int *)(result + 84) >= 1 )
{
result = a1[195];
if ( result != 9 && result != 11 )
{
if ( result == 12 )
{
a1[199] = 5;
}
else
{
v2 = a1[8];
a1[192] |= 1u;
v3 = *(_DWORD *)(v2 + 788);
v4 = *(_DWORD *)(*(_DWORD *)(v3 + 92) + 4 * (ranRandomFn(10) % *(_DWORD *)(v3 + 84)));
result = a1[192] | 2;
a1[197] = v4;
a1[198] = 5;
a1[195] = 11;
a1[192] = result;
a1[200] = 1029;
}
}
}
}
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A5AE0
int __cdecl madCloseSpecialShip(Ship *ship)
{
int result; // eax
int v2; // ecx
int v3; // edi
int v4; // ecx

result = a1[303];
if ( result )
{
result = *(_DWORD *)(a1[8] + 788);
if ( result )
{
if ( *(int *)(result + 88) >= 1 )
{
result = a1[195];
if ( result != 10 && result != 12 )
{
if ( result == 11 )
{
a1[199] = 6;
}
else
{
v2 = a1[8];
a1[192] |= 1u;
v3 = *(_DWORD *)(v2 + 788);
v4 = *(_DWORD *)(*(_DWORD *)(v3 + 96) + 4 * (ranRandomFn(10) % *(_DWORD *)(v3 + 88)));
result = a1[192] | 2;
a1[197] = v4;
a1[198] = 6;
a1[195] = 12;
a1[192] = result;
a1[200] = 1030;
}
}
}
}
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4A5BA0
int __cdecl madLinkInOpenSpecialShip(Ship *ship)
{
int result; // eax
int v2; // ecx
int v3; // edi
udword v4; // edx
int v5; // eax
int v6; // edi
int v7; // ecx

result = *(_DWORD *)(a1 + 1212);
if ( result )
{
v2 = *(_DWORD *)(a1 + 32);
result = *(_DWORD *)(v2 + 788);
if ( result )
{
if ( *(int *)(result + 100) >= 1 )
{
result = *(_DWORD *)(a1 + 784);
if ( result != 13 && result != 15 )
{
if ( result == 16 )
{
*(_DWORD *)(a1 + 796) = 7;
}
else
{
*(_DWORD *)(a1 + 768) |= 1u;
if ( *(float *)(v2 + 168) * *(float *)(a1 + 368) >= *(float *)(v2 + 792)
|| *(int *)(*(_DWORD *)(v2 + 788) + 108) <= 0 )
{
v6 = *(_DWORD *)(*(_DWORD *)(a1 + 32) + 788);
v4 = ranRandomFn(10) % *(_DWORD *)(v6 + 100);
v5 = *(_DWORD *)(v6 + 116);
}
else
{
v3 = *(_DWORD *)(*(_DWORD *)(a1 + 32) + 788);
v4 = ranRandomFn(10) % *(_DWORD *)(v3 + 108);
v5 = *(_DWORD *)(v3 + 124);
}
v7 = *(_DWORD *)(v5 + 4 * v4);
result = *(_DWORD *)(a1 + 768) | 2;
*(_DWORD *)(a1 + 788) = v7;
*(_DWORD *)(a1 + 800) = 1029;
*(_DWORD *)(a1 + 792) = 7;
*(_DWORD *)(a1 + 784) = 15;
*(_DWORD *)(a1 + 768) = result;
}
}
}
}
}
return result;
}
#endif
