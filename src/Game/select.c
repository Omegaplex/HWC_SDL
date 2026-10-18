/*=============================================================================
    Name    : select.c
    Purpose : Logic for selecting ships and groups of ships.

    Created 7/2/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef SW_Render
#include <windows.h>
#endif

#include <stdlib.h>
#include "types.h"
#include "spaceobj.h"
#include "matrix.h"
#include "vector.h"
#include "prim2d.h"
#include "font.h"
#include "fontreg.h"
#include "ACV.h"
#include "Dreadnought.h"
#include "BigGun.h"
#include "utility.h"
#include "camera.h"
#include "debug.h"
#include "spaceobj.h"
#include "Ship.h"
#include "ClassDefs.h"
#include "universe.h"
#include "cloakgenerator.h"
#include "gravwellgenerator.h"
#include "clamp.h"
#include "univupdate.h"
#include "ShipDefs.h"
#include "fastmath.h"
#include "pieplate.h"
#include "mouse.h"
#include "options.h"
#include "glinc.h"
#include "MissionScriptSupport.h"
#include "main.h"
#include "InfoOverlay.h"
#include "GamePick.h"
#include "mainrgn.h"
#include "render.h"
#include "alliance.h"
#include "sensors.h"
#include "select.h"
#include "salcapcorvette.h"
#include "proximitysensor.h"
#include "strings.h"
#include "Timer.h"
#include "tweak.h"
#include "tutor.h"
#include "mainrgn.h"

extern sdword nisWideScreenActive;

static bool32 selIsCrystalClickTarget(Ship *ship)
{
    return ship != NULL && (ship->shiptype == dCrystal0 || ship->shiptype == dCrystal1);
}

void selCircleComputeRect(float *screenCircle, sdword lod, sdword *rect);
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
sdword shipGetSlotStatus(Ship *ship, sdword slotIndex);
void opOverlayIconDraw(sdword iconIndex, sdword top, sdword left, sdword width, sdword height, color c);
bool32 acolyteCanLink(Ship *ship);
sdword acolyteGetLinkState(Ship *ship);
void telldir(void);
void selStatusDrawCommon(Ship *ship, sdword lod);
bool32 selStatusDrawSpecialStatePulse(Ship *ship, sdword lod);
void selStatusDrawEffectIcon(Ship *ship, sdword lod);
sdword selStatusDrawPlayerSlotDots(Ship *ship, sdword lod);
sdword selStatusDrawCounterPips(Ship *ship, sdword lod);
void selStatusDrawBadgeText(Ship *ship, sdword lod);
void selStatusDrawAuxText(Ship *ship, sdword lod, sdword row);
static void selStatusCircleComputeRect(Ship *ship, sdword lod, sdword *rect);
void selStatusBarDraw(Ship *ship, sdword lod, sdword row, real32 *factor, color *fillColor, color *emptyColor);
void selHealthColorPairGet(real32 *factor, color *fillColor, color *emptyColor);
void selHealthFactorClamp(Ship *ship, real32 *factor);
bool32 shipIsBeastShip(Ship *ship);

void selSelectionAddSingleShipNoCheck(MaxSelection *dest, Ship *ship)
{
    if (dest->numShips >= SEL_NumberSelections)
    {
        telldir();
    }
    else
    {
        dest->ShipPtr[dest->numShips++] = ship;
    }
}


#ifdef gshaw
//#define DEBUG_COLLSPHERES
#endif

/*=============================================================================
    Data:
=============================================================================*/
MaxSelection selSelected;
MaxAnySelection selSelecting;
MaxSelection selHotKeyGroup[SEL_NumberHotKeyGroups];
SelectCommand protectcom;
static const color selBandboxPreviewColor = colRGB(192, 192, 0);

//script-adjustable parameters
sdword selClickBoxWidth = SEL_ClickBoxWidth;    //size of box inside which a drag is a click
sdword selClickBoxHeight = SEL_ClickBoxHeight;
sdword selClickMargin = SEL_ClickMargin;        //additional 'margin' around selection for 'fuzzy' 'logic'
sdword selDragMargin = SEL_DragMargin;          //additional 'margin' around selection for 'fuzzy' 'logic'
real32 selBandBoxInsideIn = SEL_BandBoxInsideIn;//selection hysteresis factors
real32 selBandBoxInsideOut = SEL_BandBoxInsideOut;
color selShipResourceColor = colRGB(151, 71, 23);
color selShipDarkResourceColor = colRGB(58, 27, 8);
color selSelectingColor = SEL_SelectedColor;    //color of ships being selected
color selShipHealthGreen = SEL_ShipHealthGreen; //ship health in the green
color selShipHealthYellow = SEL_ShipHealthYellow;//ship health color in the yellow
color selShipHealthRed = SEL_ShipHealthRed;     //ship health color in the yellow
color selShipHealthDarkGreen = SEL_ShipHealthDarkGreen; //ship health in the green
color selShipHealthDarkYellow = SEL_ShipHealthDarkYellow;//ship health color in the yellow
color selShipHealthDarkRed = SEL_ShipHealthDarkRed;     //ship health color in the yellow
color selShipHealthSolidGreen = SEL_ShipHealthSolidGreen; //ship health in the green at LOD3+
color selShipHealthSolidYellow = SEL_ShipHealthSolidYellow;//ship health color in the yellow at LOD3+
color selShipHealthSolidRed = SEL_ShipHealthSolidRed;     //ship health color in the red at LOD3+
color selFuelColorGreen = SEL_FuelGreen;        //color of fuel bar 'in the green'
color selFuelColorRed = SEL_ShipHealthRed;
color selFuelColorDarkGreen = SEL_FuelDarkGreen;        //color of fuel bar 'in the green'
color selFuelColorDarkRed = SEL_ShipHealthDarkRed;
real32 selFuelGreenFactor = SEL_FuelGreenFactor;//amount of fuel required for it to be 'in the green'
real32 selDepthSelectMultiplier = SEL_DepthSelectMultiplier;
//real32 selShipHealthGreenFactor = SEL_ShipHealthGreenFactor;//min for green health
//real32 selShipHealthYellowFactor = SEL_ShipHealthYellowFactor;//min for yellow health
real32 selSelectionWidthFactor0 = SEL_SelectionWidthFactor0;
real32 selAsteroidMoveNeartoSize = SEL_AsteroidMoveNeartoSize;
//centre point of current selection
vector selCentrePoint;
real32 selAverageSize;

//hot-key number stuff
color selHotKeyNumberColor = SEL_HotKeyNumberColor;
udword selNumberMargin = SEL_NumberMargin;
char *selHotKeyString[10] = {"10", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
static color selStatusChargeColor = colRGB(50, 39, 252);
static color selStatusChargeDarkColor = colRGB(10, 2, 157);
static color selStatusBigGunColor = colRGB(255, 40, 255);
static color selStatusBigGunDarkColor = colRGB(100, 10, 100);
static color selStatusCannonColor = colRGB(252, 138, 63);
static color selStatusCannonDarkColor = colRGB(213, 87, 4);
static color selStatusBeastBigGunColor = colRGB(50, 39, 252);
static color selStatusBeastBigGunDarkColor = colRGB(10, 2, 127);
static color selStatusBuildProgressColor = colRGB(250, 253, 181);
static color selStatusBuildProgressDarkColor = colRGB(119, 125, 4);

//camera-space location of centre point of ship most recently considered for selection
hvector selCameraSpace;

//fonts used for printing group numbers
char *selFontName0 = NULL;
char *selFontName1 = NULL;
char *selFontName2 = NULL;
char *selFontName3 = NULL;
fonthandle selGroupFont0;
fonthandle selGroupFont1;
fonthandle selGroupFont2;
fonthandle selGroupFont3;
fonthandle selDefaultFont;

real32 SEL_MinSelectionRadius;
real32 selMinSelectionRadius;

#if PIE_MOVE_NEARTO
//closest ship this frame, used for the movement mechanis's 'move near to' command
SpaceObjRotImpTarg *selClosestTarget = NULL;
sdword selClosestDistance = SDWORD_Max;
#endif //PIE_MOVE_NEARTO

#if RND_VISUALIZATION
extern bool8 RENDER_BOXES;
#endif

/*=============================================================================
    Functions:
=============================================================================*/

void selReset(void)
{
    sdword index;

    selSelected.numShips = 0;
    selSelecting.numTargets = 0;

    for (index = 0; index < SEL_NumberHotKeyGroups; index++)
    {
        selHotKeyGroup[index].numShips = 0;
        selHotKeyGroup[index].timeLastStatus = 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : selStartup
    Description : Start up the selection module
    Inputs      : void
    Outputs     : clears out the selection lists
    Return      : void
----------------------------------------------------------------------------*/
void selStartup(void)
{
    sdword index;

    SEL_MinSelectionRadius = primScreenToGLX(6);
    selMinSelectionRadius = SEL_MinSelectionRadius;
    selSelected.numShips = 0;
    attackcom.numTargets = 0;
    for (index = 0; index < SEL_NumberHotKeyGroups; index++)
    {
        selHotKeyGroup[index].numShips = 0;
        selHotKeyGroup[index].timeLastStatus = 0.0f;
    }
    selGroupFont0 = frFontRegister(selFontName0);
    selGroupFont1 = frFontRegister(selFontName1);
    selGroupFont2 = frFontRegister(selFontName2);
    selGroupFont3 = frFontRegister(selFontName3);
    selDefaultFont = frFontRegister("default.hff");
}

/*-----------------------------------------------------------------------------
    Name        : selShutdown
    Description : Shut down the selection module
    Inputs      : void
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
void selShutdown(void)
{
    ;
}

/*-----------------------------------------------------------------------------
    Name        : selRectDragFunction
    Description : Update a list of ships being selected based upon
                    specified rectangle and camera.
    Inputs      : startNode - starting of the list to search through
                  screenRect - rectangle in viewport space which the user is dragging.
                  camera - which viewport to select in
                  destList - pointer to an array of ships to put selected list into
                  destCount - pointer to integer to recieve number of selected ships
                  playerSpecific - only select ships of current player if TRUE
                  selectAnything - if TRUE, anything, including missiles, asteroids, etc. can be selected
                  bAttack - TRUE if this is an attack selection operation.  Some objects only respond to attack selections.
    Outputs     : Updates the selected list and count.
                  Also updates the selCentrePoint vector.
    Return      : void
----------------------------------------------------------------------------*/
void selRectDragFunction(Node *startNode, Camera *camera, rectangle *screenRect, SpaceObjRotImpTarg **destList, sdword *destCount, sdword playerSpecific, bool32 selectAnything, bool32 bAttack)
{
    Node *targetnode;
    SpaceObjRotImpTarg *target;
    realrectangle selectRect, unionRect, polyRect, rect;
    real32 areaUnion, areaTotal, val;
    vector *point;
    ubyte *pIndex;
    sdword index, base;
    real32 dragMargin = primScreenToGLScaleX(selDragMargin);

    dbgAssert(camera != NULL);                              //verify parameters

    *destCount = 0;                                         //start off with nothing selected

    rect.x0 = primScreenToGLX(screenRect->x0);
    rect.x1 = primScreenToGLX(screenRect->x1);
    rect.y0 = primScreenToGLY(screenRect->y1);
    rect.y1 = primScreenToGLY(screenRect->y0);

    //this system can later be optimised by only walking through sorted porions
    //of the object list.  However, for now we will walk the entire list.
    selSelecting.numTargets = 0;
//    targetnode = universe.RenderList.head;                   //get first node in list
    targetnode = startNode;
    while (targetnode != NULL)
    {
        target = (SpaceObjRotImpTarg *)listGetStructOfNode(targetnode);
        if ((target->flags & (SOF_Selectable|SOF_Targetable)) == 0)
        {
            goto nexttarget;
        }

        if (target->collInfo.selCircleRadius <= 0.0f)
        {
            goto nexttarget;
        }

        if (target->flags & SOF_Dead)
        {
            goto nexttarget;
        }

        if ((target->flags2 & BIT4) != 0 &&
            (target->objtype != OBJ_ShipType || ((Ship *)target)->shiptype != dBigGunUnit))
        {
            goto nexttarget;
        }

        if (!selectAnything)
        {
            if (target->objtype == OBJ_ShipType)
            {
                if (playerSpecific)
                {
                    if (!(target->flags & SOF_Selectable))
                    {
                        goto nexttarget;
                    }
                    // only own player should be able to select it
                    if (((Ship *)target)->playerowner != universe.curPlayerPtr)
                    {
                        goto nexttarget;
                    }
                }
            }
            else if (target->objtype == OBJ_DerelictType)
            {
                if (playerSpecific)
                {
                    goto nexttarget;
                }
            }
            else
            {
                goto nexttarget;
            }
        }
        if(target->objtype == OBJ_ShipType)     //later optimize if ONLY ships are cloaked!
        {
            if (univIsShipCloaked(target) &&
                ((Ship *)target)->playerowner != universe.curPlayerPtr &&
                !shipGetSlotStatus((Ship *)target, universe.curPlayerPtr->playerIndex))
            {
                goto nexttarget;
            }
            if(bAttack && ((Ship *)target)->staticinfo->cannotForceAttackIfOwnShip)
            {
                if(((Ship *)target)->playerowner == universe.curPlayerPtr)
                {
                    //don't allow anysort of attacking of own cryotrays
                    goto nexttarget;
                }
            }
        }
        else if (target->objtype == OBJ_AsteroidType)
        {
            if (playerSpecific)
            {
                goto nexttarget;
            }

            if (selectAnything || (bAttack && (target->attributes & (ATTRIBUTES_KillerCollDamage|ATTRIBUTES_HeadShotKillerCollDamage))))
            {
                ;  // allow this target
            }
            else
            {
                goto nexttarget;
            }
        }


        //here we do a test of the overlap of the selection rect
        //and a rectangle representing the target's selection sphere.
        selectRect.x0 = target->collInfo.selCircleX - target->collInfo.selCircleRadius - dragMargin;
        selectRect.x1 = target->collInfo.selCircleX + target->collInfo.selCircleRadius + dragMargin;
        selectRect.y0 = (target->collInfo.selCircleY - target->collInfo.selCircleRadius) - dragMargin;
        selectRect.y1 = (target->collInfo.selCircleY + target->collInfo.selCircleRadius) + dragMargin;
        if (target->collInfo.precise != NULL &&
            target->currentLOD <= target->staticinfo->staticheader.staticCollInfo.preciseSelection)
        {                                               //if we should perform precise collision checking
            point = target->collInfo.precise->worldRectPos;//list of points
            pIndex = target->collInfo.precise->corner;    //list of vertices
            areaUnion = areaTotal = 0.0f;
            for (index = base = 0; index < target->collInfo.precise->nPolys; index++, base += 4)
            {                                           //for each poly
                val = min(point[pIndex[base + 0]].x, min(point[pIndex[base + 1]].x, min(point[pIndex[base + 2]].x, point[pIndex[base + 3]].x)));
                polyRect.x0 = val;
                val = max(point[pIndex[base + 0]].x, max(point[pIndex[base + 1]].x, max(point[pIndex[base + 2]].x, point[pIndex[base + 3]].x)));
                polyRect.x1 = val;
                val = max(point[pIndex[base + 0]].y, max(point[pIndex[base + 1]].y, max(point[pIndex[base + 2]].y, point[pIndex[base + 3]].y)));
                polyRect.y1 = val;
                val = min(point[pIndex[base + 0]].y, min(point[pIndex[base + 1]].y, min(point[pIndex[base + 2]].y, point[pIndex[base + 3]].y)));
                polyRect.y0 = val;
                areaTotal += (polyRect.x1 - polyRect.x0) * (polyRect.y1 - polyRect.y0);
                primRealRectUnion2(&unionRect, &polyRect, &rect);
                areaUnion += (unionRect.x1 - unionRect.x0) * (unionRect.y1 - unionRect.y0);
            }
        }
        else
        {
            primRealRectUnion2(&unionRect, &selectRect, &rect);
            areaTotal = ((selectRect.x1 - selectRect.x0) *
                                 (selectRect.y1 - selectRect.y0));
            areaUnion = ((unionRect.x1 - unionRect.x0) *
                                 (unionRect.y1 - unionRect.y0));
        }
        if (areaUnion / areaTotal >= selBandBoxInsideIn)
        {                                           //if enough of target inside selection box
            sdword existingIndex;
            SpaceObjRotImpTarg *selectedTarget = target;

            if (selectedTarget->objtype == OBJ_ShipType)
            {
                Ship *selectedShip = (Ship *)selectedTarget;

                if (((ShipStaticInfo *)selectedShip->staticinfo)->shipclass == CLASS_Component &&
                    selectedShip->clampInfo != NULL &&
                    selectedShip->clampInfo->host != NULL)
                {
                    selectedTarget = selectedShip->clampInfo->host;
                }
            }

            for (existingIndex = 0; existingIndex < *destCount; existingIndex++)
            {
                if (destList[existingIndex] == selectedTarget)
                {
                    goto nexttarget;
                }
            }

//            dbgAssert(*destCount < COMMAND_MAX_SHIPS);
            if (*destCount < COMMAND_MAX_SHIPS)
            {
                destList[*destCount] = selectedTarget;    //add it to selected list
                (*destCount)++;
            }
            else
            {
                dbgMessage("\nWarning: Tried to select too many ships");
            }
        }

nexttarget:
        targetnode = targetnode->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : selRectDragAddFunction
    Description : Update a list of ships being selected based upon
                    specified rectangle and camera.
    Inputs      : rect - rectangle in viewport space which the user is dragging.
                  camera - which viewport to select in
    Outputs     : Adds all ships in rectangle to current selection.
                  Also updates the selCentrePoint vector.
    Return      : void
----------------------------------------------------------------------------*/
void selRectDragAddFunction(Node *startNode, Camera *camera, rectangle *rect)
{
    sdword index, selectCount;
    MaxSelection selection;

    //do a selection into a temporary selection
    selRectDragFunction(startNode, camera, rect, (SpaceObjRotImpTarg **)selection.ShipPtr, &selection.numShips, TRUE, FALSE, FALSE);
    //see if we should be adding or removing these ships
    for (selectCount = index = 0; index < selection.numShips; index++)
    {
        if (selShipInSelection(selSelected.ShipPtr,
                selSelected.numShips, selection.ShipPtr[index]) != FALSE)
        {
            selectCount++;
        }
    }
    //add in or remove the ships from selection.  Warning!!! this is a order n * m search
    if (selectCount == selection.numShips)
    {                                                       //if all ships selected are already selected, remove them
        for (index = 0; index < selection.numShips; index++)
        {
            selSelectionRemoveSingleShip(&selSelected, selection.ShipPtr[index]);
        }
    }
    else
    {                                                       //not all selected, add them
        for (index = 0; index < selection.numShips; index++)
        {
            selSelectionAddSingleShip(&selSelected, selection.ShipPtr[index]);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : selShipInSelection
    Description : Search for a ship in a particular selection
    Inputs      : shipList - list of ships to search
                  nShips - number of ships in list to search
                  ship - ship to search for a match of
    Outputs     : ..
    Return      : TRUE of ship found, false otherwise
----------------------------------------------------------------------------*/
sdword selShipInSelection(ShipPtr *shipList, sdword nShips, ShipPtr ship)
{
    sdword index;

    for (index = 0; index  < nShips; index++, shipList++)
    {
        if (ship == *shipList)
            return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionSetSingleShip
    Description : Select a single ship
    Inputs      : ship - ship to select
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void selSelectionSetSingleShip(Ship *ship)
{
    dbgAssert(ship->playerowner == universe.curPlayerPtr);
    selSelected.numShips = 1;
    selSelected.ShipPtr[0] = ship;
    selCentrePoint = ship->posinfo.position;
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionAddSingleShip
    Description : Add a single ship to current selection
    Inputs      : ship - ship to add to selection
    Outputs     : adds to the selSelected list if not already there
    Return      : void
----------------------------------------------------------------------------*/
void selSelectionAddSingleShip(MaxSelection *dest, Ship *ship)
{
    sdword index;

//    dbgAssert(dest->numShips < COMMAND_MAX_SHIPS - 1);
    if (dest->numShips >= COMMAND_MAX_SHIPS)
    {
        dbgAssert(dest->numShips <= COMMAND_MAX_SHIPS);
        dbgMessage("\nWarning: Tried to add to many ships to selection");
        return;
    }

    for (index = 0; index < dest->numShips; index++)
    {
        if (dest->ShipPtr[index] == ship)
            return;
    }
    dest->ShipPtr[dest->numShips] = ship;
    dest->numShips++;
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionRemoveSingleShip
    Description : Remove a selected ship from selSelected, if it is even there
    Inputs      :
    Outputs     : Decrements selSelected.numTargets and may move entire rest of
                    list back one to fill in the hole.
    Return      :
----------------------------------------------------------------------------*/
void selSelectionRemoveSingleShip(MaxSelection *dest, Ship *ship)
{
    sdword index;

    for (index = 0; index < dest->numShips; index++)
    {
        if (dest->ShipPtr[index] == ship)              //if ship in selected list
        {
            for (; index < dest->numShips - 1; index++)
            {                                               //move rest of list back 1
                dest->ShipPtr[index] = dest->ShipPtr[index + 1];
            }
            dest->numShips--;                       //one less ship
            return;                                         //all done
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionClick
    Description : Select the closest ship the player has clicked on.
    Inputs      : listHead - where to start scanning from
                  camera - specific camera/mission sphere
                  x, y - screen pixel location of the click
                  bIncludeDerelicts - TRUE to include derelicts in click selection possibilities
                  bIncludeResources - TRUE to include resources in click selection possibilities
    Outputs     : ..
    Return      : closest ship clicked on or NULL if none under cursor
    Note        : A margin around each ship's selection sphere is added to
                    permit clicking very distant ships.
----------------------------------------------------------------------------*/
Ship *selSelectionClick(Node *listHead, Camera *camera, sdword x, sdword y, bool32 bIncludeDerelicts, bool32 bIncludeResources)
{
    Node *shipnode;
    Ship *ship;
    rectangle selectRect;
    sdword closestDistance = SDWORD_Max, distance;
    Ship *closestShip = NULL;
    vector *point;
    ubyte *pIndex;
    sdword index, base;
    real32 xReal, yReal;
    real32 p0, p1, p2, p3;

    dbgAssert(camera != NULL);                              //verify parameters

    //this system can later be optimised by only walking through sorted porions
    //of the object list.  However, for now we will walk the entire list.
    shipnode = listHead;
    while (shipnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(shipnode);
        if (ship->objtype == OBJ_ShipType)
        {
            if ((ship->flags & SOF_Selectable) == 0 &&
                ship->shiprace != RACE_Beast &&
                !selIsCrystalClickTarget(ship) &&
                ship->shiptype != dEcclesiamInfected)
            {
                goto nextship;
            }
        }
        else if (bIncludeResources && (ship->objtype == OBJ_AsteroidType || ship->objtype == OBJ_DustType))
        {
            ;                                               //good
        }
        else if (bIncludeDerelicts && ship->objtype == OBJ_DerelictType)
        {
            ;                                               //good
        }
        else
        {                                                   //not anything we want
            goto nextship;                                  //skip this one
        }
        if (univIsShipCloaked((SpaceObj *)ship) &&
            ship->playerowner != universe.curPlayerPtr &&
            !shipGetSlotStatus(ship, universe.curPlayerPtr->playerIndex))
        {
            goto nextship;
        }
        if ((ship->collInfo.selCircleRadius <= 0.0f) || bitTest(ship->flags, SOF_Dead))
        {
            if ((primGLToScreenX(ship->collInfo.selCircleX) == x) &&
                (primGLToScreenY(ship->collInfo.selCircleY) == y))
            {
                closestShip = ship;
            }
        }
        else   //and it is in front of the camera
        {
            //here we do a test of the overlap of the selection rect
            //and a rectangle representing the ship's selection sphere.
            if (ship->collInfo.precise != NULL &&
                ship->currentLOD <= ship->staticinfo->staticheader.staticCollInfo.preciseSelection)
            {                                               //if we should perform precise collision checking
                point = ship->collInfo.precise->worldRectPos;//list of points
                pIndex = ship->collInfo.precise->corner;    //list of vertices
                xReal = primScreenToGLX(x);                 //floating-point location of points
                yReal = primScreenToGLY(y);
                for (index = base = 0; index < ship->collInfo.precise->nPolys; index++, base += 4)
                {                                           //for each poly
                    p0 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 0]].x, point[pIndex[base + 0]].y, point[pIndex[base + 1]].x, point[pIndex[base + 1]].y);
                    p1 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 1]].x, point[pIndex[base + 1]].y, point[pIndex[base + 2]].x, point[pIndex[base + 2]].y);
                    p2 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 2]].x, point[pIndex[base + 2]].y, point[pIndex[base + 3]].x, point[pIndex[base + 3]].y);
                    p3 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 3]].x, point[pIndex[base + 3]].y, point[pIndex[base + 0]].x, point[pIndex[base + 0]].y);
                    if (p0 > 0.0f && p1 > 0.0f && p2 > 0.0f && p3 > 0.0f)
                    {                                       //if point inside this poly
                        distance = abs(primGLToScreenX(ship->collInfo.selCircleX) - x) +
                            abs(primGLToScreenY(ship->collInfo.selCircleY) - y) +
                            primGLToScreenScaleX(ship->collInfo.selCircleDepth);
                        if (distance < closestDistance)
                        {
                            closestDistance = distance;
                            closestShip = ship;
                        }
                        break;
                    }
                }
            }
            else
            {
                selectRect.x0 = primGLToScreenX(ship->collInfo.selCircleX - ship->collInfo.selCircleRadius) - selClickMargin;
                selectRect.x1 = primGLToScreenX(ship->collInfo.selCircleX + ship->collInfo.selCircleRadius) + selClickMargin;
                selectRect.y0 = primGLToScreenY(ship->collInfo.selCircleY + ship->collInfo.selCircleRadius) - selClickMargin;
                selectRect.y1 = primGLToScreenY(ship->collInfo.selCircleY - ship->collInfo.selCircleRadius) + selClickMargin;

                //note that this is a rectangular intersection test
                if ((selectRect.x0 <= x) && (x <= selectRect.x1) &&
                    (selectRect.y0 <= y) && (y <= selectRect.y1))
                {                                           //found a matching object, return
                    distance = abs(primGLToScreenX(ship->collInfo.selCircleX) - x) +
                        abs(primGLToScreenY(ship->collInfo.selCircleY) - y) +
                            primGLToScreenScaleX(ship->collInfo.selCircleDepth);
                   if (distance < closestDistance)
                   {
                       closestDistance = distance;
                       closestShip = ship;
                   }
    //                return(ship);
               }
           }
       }
nextship:
        shipnode = shipnode->next;
    }

    if (closestShip != NULL &&
        closestShip->objtype == OBJ_ShipType &&
        closestShip->staticinfo->shipclass == CLASS_Component &&
        closestShip->clampInfo != NULL &&
        closestShip->clampInfo->host != NULL)
    {
        return (Ship *)closestShip->clampInfo->host;
    }

    return(closestShip);
}

/*-----------------------------------------------------------------------------
    Name        : selClickFromArray
    Description : Perform a mouse click from an array of spaceobj pointers
                    instead of a linked list.
    Inputs      : list - list of object pointers
                  length - number of pointers in the array
                  x, y - location of mouse click
    Outputs     :
    Return      : selected object, or NULL
----------------------------------------------------------------------------*/
SpaceObj *selClickFromArray(SpaceObj **list, sdword length, sdword x, sdword y)
{
    SpaceObj *object;
    rectangle selectRect;
    sdword closestDistance = SDWORD_Max, distance;
    SpaceObj *closestObject = NULL;

    while (length > 0)
    {
        object = (SpaceObj *)(*list);
        if ((object->flags & (SOF_Selectable | SOF_Targetable)) == 0)
        {
            goto nextship;
        }
        if (object->objtype == OBJ_ShipType &&
            univIsShipCloaked(object) &&
            ((Ship *)object)->playerowner != universe.curPlayerPtr &&
            !shipGetSlotStatus((Ship *)object, universe.curPlayerPtr->playerIndex))
        {
            goto nextship;
        }
        if ((((SpaceObjRotImpTarg *)object)->collInfo.selCircleRadius > 0) &&
            ((((SpaceObjRotImpTarg *)object)->flags & SOF_Dead) == 0))   //and it is in front of the camera
        {
            //here we do a test of the overlap of the selection rect
            //and a rectangle representing the object's selection sphere.
            selectRect.x0 = primGLToScreenX(((SpaceObjRotImpTarg *)object)->collInfo.selCircleX - ((SpaceObjRotImpTarg *)object)->collInfo.selCircleRadius) - selClickMargin;
            selectRect.x1 = primGLToScreenX(((SpaceObjRotImpTarg *)object)->collInfo.selCircleX + ((SpaceObjRotImpTarg *)object)->collInfo.selCircleRadius) + selClickMargin;
            selectRect.y0 = primGLToScreenY(((SpaceObjRotImpTarg *)object)->collInfo.selCircleY + ((SpaceObjRotImpTarg *)object)->collInfo.selCircleRadius) - selClickMargin;
            selectRect.y1 = primGLToScreenY(((SpaceObjRotImpTarg *)object)->collInfo.selCircleY - ((SpaceObjRotImpTarg *)object)->collInfo.selCircleRadius) + selClickMargin;
            //!!! note that this is a rectangular intersection test
            if ((selectRect.x0 <= x) && (x <= selectRect.x1) &&
                (selectRect.y0 <= y) && (y <= selectRect.y1))
            {                                           //found a matching object, return
                distance = abs(primGLToScreenX(((SpaceObjRotImpTarg *)object)->collInfo.selCircleX) - x) +
                    abs(primGLToScreenY(((SpaceObjRotImpTarg *)object)->collInfo.selCircleY) - y);//manhattan distance
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    closestObject = object;
                }
//                return(object);
            }
        }
nextship:
        list++;
        length--;
    }
    return(closestObject);
}

//I know I shouldn't be making external references like this but hey, Falko put
//these in mouse.c where they shouldn't be in the first place.  And hey, 2
//wrongs make a right, right? (Luke)
// um... sorry... (Falko)
extern mouseCursor  mouseCursorType;
extern udword       mouseCursorMode;
extern MaxSelection mouseCursorSelect;
extern real32       mouseDoubleClickTime;
extern real32       mouseDoubleClickExpire;
extern real32      mouseCursorOverDistance;
extern sdword      mouseCursorOverShipType;
extern real32      mouseCursorTextExpire;
extern SpaceObj    *mouseCursorObjPtr;
extern ubyte        mouseCursorOverLODs;
extern sdword       mouseCursorEnemyType;
extern sdword       mouseCursorEnemyPlayer;

/*-----------------------------------------------------------------------------
    Name        : selShipUnderMouse
    Description : Handle the case of an object being under the mouse
    Inputs      : target - what object is under the mouse
                  distance - distance to centre of selection circle
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void selShipUnderMouse(SpaceObjRotImpTarg *target, sdword distance)
{
    SpaceObjRotImpTarg *mouseoverTarget;
    Ship *hoveredShip;

#if PIE_MOVE_NEARTO
    //process object for move near to command
    if (piePointSpecMode == PSM_XY || piePointSpecMode == PSM_Z)
    {                                                       //if in point spec mode
        if ((target->objtype == OBJ_ShipType && ((Ship*)target)->shiptype != Mothership)  ||
            (target->objtype == OBJ_AsteroidType/* &&
             target->staticinfo->staticheader.staticCollInfo.collspheresize > selAsteroidMoveNeartoSize*/))
        {                                                   //if it's a valid object type
            if (distance < smClosestDistance)
            {
                selClosestDistance = distance;
                selClosestTarget = target;
            }
        }
    }
#endif
    //process object for mouse-cursor action
    // only the closest ship gets named
    if (!mrRightMouseCameraMode)
    {                                                       //if no mouse action going down
        if (bitTest(target->flags, SOF_DockMarker))
        {                                                   //if closer than previous close object
            return;
        }

        if (target->objtype == OBJ_ShipType)
        {
            if (smSensorsActive)
            {
                if (!univIsObjFriendly((SpaceObj *)target, universe.curPlayerPtr) &&
                    !fogObjectVisibleToPlayer(universe.curPlayerPtr, (SpaceObj *)target, FALSE))
                {
                    return;
                }
            }
            else if (target->cameraDistanceSquared > mouseCursorOverDistance)
            {
                return;
            }
        }

        mouseCursorOverDistance = target->cameraDistanceSquared;

        if (target->objtype == OBJ_ShipType)
        {
            hoveredShip = (Ship *)target;
            mouseoverTarget = target;
            if (((ShipStaticInfo *)hoveredShip->staticinfo)->shipclass == CLASS_Component &&
                hoveredShip->clampInfo != NULL &&
                hoveredShip->clampInfo->host != NULL)
            {
                mouseoverTarget = hoveredShip->clampInfo->host;
            }

            // enemy ships are only identified as "enemy ship"
            if ((((Ship *)mouseoverTarget)->playerowner == universe.curPlayerPtr) ||
                bitTest(((Ship *)mouseoverTarget)->specialFlags1C8, 0x00040000) &&
                (tutorial != TUTORIAL_ONLY))
            {
                mouseCursorOverShipType  = ((Ship *)mouseoverTarget)->shiptype;
                mouseCursorObjPtr        = (SpaceObj *)mouseoverTarget;
                bitSet(mouseCursorMode, MCM_CursorOverObj);
            }
            else
            {
                //the trader ship refuses not to be buggy in these "general" cases,
                //so I'll just put special case code for the trader, seeing as it's
                //a special case anyway.
                if (((Ship *)mouseoverTarget)->shiptype == FloatingCity)
                {
                    mouseCursorOverShipType = FloatingCity;
                    mouseCursorObjPtr       = NULL;
                    bitClear(mouseCursorMode, MCM_CursorOverObj);
                }
                else if (univIsObjFriendly((SpaceObj *)mouseoverTarget, universe.curPlayerPtr))
                {
                    mouseCursorOverShipType = MC_ALLIED_SHIP;
                    mouseCursorObjPtr       = (SpaceObj *)mouseoverTarget;
                    bitClear(mouseCursorMode, MCM_CursorOverObj);
                }
                else
                {
                    mouseCursorOverShipType = MC_ENEMY_SHIP;
                    mouseCursorObjPtr       = (SpaceObj *)mouseoverTarget;
                    bitClear(mouseCursorMode, MCM_CursorOverObj);
                }
            }
            mouseCursorOverLODs     = mouseoverTarget->renderedLODs;
            mouseCursorEnemyType    = ((Ship *)mouseoverTarget)->shiptype;
            mouseCursorEnemyPlayer  = ((Ship *)mouseoverTarget)->playerowner->playerIndex;
        }
        else if (target->objtype == OBJ_DerelictType)
        {
            if ((((Derelict *)target)->derelicttype != HyperspaceGate) &&
                !(((DerelictStaticInfo *)target->staticinfo)->salvageable))
            {
                return;
            }
            mouseCursorOverShipType = MC_DERELICT;
            mouseCursorObjPtr       = (SpaceObj *)target;
            bitSet(mouseCursorMode, MCM_CursorOverObj);
        }
        else
        {
            // only consider dustclouds as cursor objects if there's a
            // resource collector selected
            if ((target->objtype != OBJ_DustType) ||
                ShiptypeInSelection((SelectCommand *)&selSelected, sWorker) ||
                ShiptypeInSelection((SelectCommand *)&selSelected, bWorker))
            {
                mouseCursorOverShipType = MC_RESOURCE;
                mouseCursorObjPtr = (SpaceObj *)target;
                bitSet(mouseCursorMode, MCM_CursorOverObj);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : selCircleCompute
    Description : Compute on-screen size and location of the selection circle
                    for specified ship.
    Inputs      : modelView, projection - matrices specifying the model/view
                    and projection matrices ship is drawn with.
                  target - target (ship, asteroid, etc.) to compute selection circle for.
    Outputs     : stores computed circle in Ship structure
    Return      : void
----------------------------------------------------------------------------*/
void selCircleCompute(hmatrix *modelView, hmatrix *projection, SpaceObjRotImpTarg *target)
{
    hvector centre, cameraSpace, screenSpace, radius, radiusProjected, preciseCorner[8];
    sdword distance;
    rectangle selectRect;
    PreciseSelection *precise;
    CollInfo *collision;
    vector v0, v1, cross;
    vector *point;
    ubyte *pIndex;
    sdword index, base;
    real32 xReal, yReal;
    real32 p0, p1, p2, p3;

    collision = &target->collInfo;

    centre.x = collision->collPosition.x;                    //vector at centre of target
    centre.y = collision->collPosition.y;
    centre.z = collision->collPosition.z;
    centre.w = 1.0f;
    hmatMultiplyHMatByHVec(&selCameraSpace, modelView, &centre);//in camera space
    radius = selCameraSpace;                                   //project another point to determine selection sphere size
    radius.x += shipGetCollSphereSize((SpaceObj *)target);
    hmatMultiplyHMatByHVec(&screenSpace, projection, &selCameraSpace);//in screen space
    hmatMultiplyHMatByHVec(&radiusProjected, projection, &radius);
    collision->selCircleX = screenSpace.x / screenSpace.w;
    collision->selCircleY = screenSpace.y / screenSpace.w;
    collision->selCircleDepth = (-selCameraSpace.z) / CAMERA_MAX_ZOOMOUT_DISTANCE * selDepthSelectMultiplier;
    collision->selCircleRadius = (radiusProjected.x - screenSpace.x) / screenSpace.w;

    //compute the precise selection if the ship type demands it
    if (collision->precise != NULL)
    {
        if (target->currentLOD <= target->staticinfo->staticheader.staticCollInfo.preciseSelection)
        {                                                   //if we should do precise selection
            if (target->objtype == OBJ_ShipType)
            {
                Ship *ship = (Ship *)target;
                dbgMessagef("selCircleCompute precise: ship=%x reg=%d type=%d(%s) race=%d precise=%x sizeSpec=%u lod=%d preciseSelection=%d",
                            ship,
                            ship->shipID.shipNumber,
                            ship->shiptype,
                            ShipTypeToStr(ship->shiptype),
                            ship->shiprace,
                            collision->precise,
                            ship->staticinfo->custshipheader.sizeofShipSpecifics,
                            target->currentLOD,
                            target->staticinfo->staticheader.staticCollInfo.preciseSelection);
            }
            else
            {
                dbgMessagef("selCircleCompute precise: target=%x objtype=%d precise=%x lod=%d preciseSelection=%d",
                            target,
                            target->objtype,
                            collision->precise,
                            target->currentLOD,
                            target->staticinfo->staticheader.staticCollInfo.preciseSelection);
            }
            if (((udword)collision->precise & 3) != 0)
            {
                dbgMessagef("selCircleCompute: bad precise target=%x objtype=%d static=%x precise=%x lod=%d preciseSelection=%d",
                            target,
                            target->objtype,
                            target->staticinfo,
                            collision->precise,
                            target->currentLOD,
                            target->staticinfo->staticheader.staticCollInfo.preciseSelection);
                if (target->objtype == OBJ_ShipType)
                {
                    Ship *ship = (Ship *)target;
                    dbgMessagef("selCircleCompute: bad precise ship=%x reg=%d type=%d(%s) race=%d",
                                ship,
                                ship->shipID.shipNumber,
                                ship->shiptype,
                                ShipTypeToNiceStr(ship->shiptype),
                                ship->shiprace);
                }
            }
            precise = collision->precise;
            point = precise->worldRectPos;

            //transform the corners into camera space and project into screen space
            for (index = 0; index < 8; index++)
            {
                centre.x = collision->rectpos[index].x + target->posinfo.position.x;//make a hvector (w is already 1.0f)
                centre.y = collision->rectpos[index].y + target->posinfo.position.y;
                centre.z = collision->rectpos[index].z + target->posinfo.position.z;
                hmatMultiplyHMatByHVec(&cameraSpace, modelView, &centre);//transform collision rectangle into world space
                hmatMultiplyHMatByHVec(&screenSpace, projection, &cameraSpace);//transform into screen space
                point[index].x = screenSpace.x / screenSpace.w;
                point[index].y = screenSpace.y / screenSpace.w;
                preciseCorner[index].x = screenSpace.x / screenSpace.w;
                preciseCorner[index].y = screenSpace.y / screenSpace.w;
                preciseCorner[index].z = screenSpace.z / screenSpace.w;
            }

            //now we have all points in screen space, let's find what polys point toward camera
            index = 0;                                      //init the corner index to zero
            precise->nPolys = 0;
            pIndex = precise->corner;
            //top poly (4567)
            if (preciseCorner[4].z > 0.0f && preciseCorner[5].z > 0.0f
                && preciseCorner[6].z > 0.0f && preciseCorner[7].z > 0.0f)
            {
                vecSub(v0, preciseCorner[5], preciseCorner[4]);
                vecSub(v1, preciseCorner[7], preciseCorner[4]);
                vecCrossProduct(cross, v0, v1);             //point up from ship
                if (cross.z > 0.0f)
                {                                           //if cross product camera facing
                    pIndex[index + 0] = 4;
                    pIndex[index + 1] = 5;
                    pIndex[index + 2] = 6;
                    pIndex[index + 3] = 7;
                    index += 4;
                    precise->nPolys++;
                }
            }
            //bottom poly (0321)
            if (preciseCorner[0].z > 0.0f && preciseCorner[3].z > 0.0f
                && preciseCorner[2].z > 0.0f && preciseCorner[1].z > 0.0f)
            {
                vecSub(v0, preciseCorner[1], preciseCorner[0]);
                vecSub(v1, preciseCorner[3], preciseCorner[0]);
                vecCrossProduct(cross, v1, v0);             //point down from ship
                if (cross.z > 0.0f)
                {                                           //if cross product camera facing
                    pIndex[index + 0] = 0;
                    pIndex[index + 1] = 3;
                    pIndex[index + 2] = 2;
                    pIndex[index + 3] = 1;
                    index += 4;
                    precise->nPolys++;
                }
            }
            //front poly (7623)
            if (preciseCorner[7].z > 0.0f && preciseCorner[6].z > 0.0f
                && preciseCorner[2].z > 0.0f && preciseCorner[3].z > 0.0f)
            {
                vecSub(v0, preciseCorner[6], preciseCorner[7]);
                vecSub(v1, preciseCorner[3], preciseCorner[7]);
                vecCrossProduct(cross, v0, v1);             //point forward from ship
                if (cross.z > 0.0f)
                {                                           //if cross product camera facing
                    pIndex[index + 0] = 7;
                    pIndex[index + 1] = 6;
                    pIndex[index + 2] = 2;
                    pIndex[index + 3] = 3;
                    index += 4;
                    precise->nPolys++;
                }
            }
            //back poly (4015)
            if (preciseCorner[4].z > 0.0f && preciseCorner[0].z > 0.0f
                && preciseCorner[1].z > 0.0f && preciseCorner[5].z > 0.0f)
            {
                vecSub(v0, preciseCorner[5], preciseCorner[4]);
                vecSub(v1, preciseCorner[0], preciseCorner[4]);
                vecCrossProduct(cross, v1, v0);             //point backward from ship
                if (cross.z > 0.0f)
                {                                           //if cross product camera facing
                    pIndex[index + 0] = 4;
                    pIndex[index + 1] = 0;
                    pIndex[index + 2] = 1;
                    pIndex[index + 3] = 5;
                    index += 4;
                    precise->nPolys++;
                }
            }
            //left poly (7304)
            if (preciseCorner[7].z > 0.0f && preciseCorner[3].z > 0.0f
                && preciseCorner[0].z > 0.0f && preciseCorner[4].z > 0.0f)
            {
                vecSub(v0, preciseCorner[4], preciseCorner[7]);
                vecSub(v1, preciseCorner[3], preciseCorner[7]);
                vecCrossProduct(cross, v1, v0);             //point left from ship
                if (cross.z > 0.0f)
                {                                           //if cross product camera facing
                    pIndex[index + 0] = 7;
                    pIndex[index + 1] = 3;
                    pIndex[index + 2] = 0;
                    pIndex[index + 3] = 4;
                    index += 4;
                    precise->nPolys++;
                }
            }
            //right poly (6512)
            if (preciseCorner[6].z > 0.0f && preciseCorner[5].z > 0.0f
                && preciseCorner[1].z > 0.0f && preciseCorner[2].z > 0.0f)
            {
                vecSub(v0, preciseCorner[5], preciseCorner[6]);
                vecSub(v1, preciseCorner[2], preciseCorner[6]);
                vecCrossProduct(cross, v0, v1);             //point right from ship
                if (cross.z > 0.0f)
                {                                           //if cross product camera facing
                    pIndex[index + 0] = 6;
                    pIndex[index + 1] = 5;
                    pIndex[index + 2] = 1;
                    pIndex[index + 3] = 2;
                    index += 4;
                    precise->nPolys++;
                }
            }
            precise->nPolys = (ubyte)min(precise->nPolys, 3);
            //find the target under the mouse cursor for the move near to command/mouse cursor text
            xReal = primScreenToGLX(mouseCursorX());        //floating-point location of mouse
            yReal = primScreenToGLY(mouseCursorY());
            for (index = base = 0; index < target->collInfo.precise->nPolys; index++, base += 4)
            {                                               //for each poly
                p0 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 0]].x, point[pIndex[base + 0]].y, point[pIndex[base + 1]].x, point[pIndex[base + 1]].y);
                p1 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 1]].x, point[pIndex[base + 1]].y, point[pIndex[base + 2]].x, point[pIndex[base + 2]].y);
                p2 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 2]].x, point[pIndex[base + 2]].y, point[pIndex[base + 3]].x, point[pIndex[base + 3]].y);
                p3 = primPointLineIntersection(xReal, yReal, point[pIndex[base + 3]].x, point[pIndex[base + 3]].y, point[pIndex[base + 0]].x, point[pIndex[base + 0]].y);
                if (p0 > 0.0f && p1 > 0.0f && p2 > 0.0f && p3 > 0.0f)
                {                                       //if mouse inside this poly
                    if (target->collInfo.selCircleRadius > 0.0f)
                    {
                        distance = abs(primGLToScreenX(target->collInfo.selCircleX) - mouseCursorX()) +
                            abs(primGLToScreenY(target->collInfo.selCircleY) - mouseCursorY());
                        selShipUnderMouse(target, distance);
                    }
                    break;
                }
            }
        }
    }
    else
    {
        selectRect.x0 = primGLToScreenX(collision->selCircleX - collision->selCircleRadius) - selClickMargin;
        selectRect.x1 = primGLToScreenX(collision->selCircleX + collision->selCircleRadius) + selClickMargin;
        selectRect.y0 = primGLToScreenY(collision->selCircleY + collision->selCircleRadius) - selClickMargin;
        selectRect.y1 = primGLToScreenY(collision->selCircleY - collision->selCircleRadius) + selClickMargin;

        if ((selectRect.x0 <= mouseCursorX()) && (mouseCursorX() <= selectRect.x1) &&
            (selectRect.y0 <= mouseCursorY()) && (mouseCursorY() <= selectRect.y1))
        {                                               //if under mouse
            distance = abs(primGLToScreenX(collision->selCircleX) - mouseCursorX()) +
                abs(primGLToScreenY(collision->selCircleY) - mouseCursorY());
            selShipUnderMouse(target, distance);

        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : selDrawBoxes
    Description : Draw the projected selection boxes if the target has a precise selection
    Inputs      : target - target to draw boxes for
    Outputs     :
    Return      :
    Note        : draws in 2d
----------------------------------------------------------------------------*/
#if SEL_DRAW_BOXES
void selDrawBoxes(SpaceObjRotImpTarg *target)
{
    vector *point;
    ubyte *pIndex;
    CollInfo *collision;

    collision = &target->collInfo;
    if (collision->precise != NULL)
    {
        if (target->currentLOD <= target->staticinfo->staticheader.staticCollInfo.preciseSelection)
        {                                                   //if we should do precise selection
            point = collision->precise->worldRectPos;
            pIndex = collision->precise->corner;

            if (collision->precise->nPolys >= 1)
            {
                fontPrint(primGLToScreenX((point[pIndex[0]].x + point[pIndex[2]].x) / 2.0f),
                           primGLToScreenY((point[pIndex[0]].y + point[pIndex[2]].y) / 2.0f),
                           colWhite, "1");
                primLine2(primGLToScreenX(point[pIndex[0]].x), primGLToScreenY(point[pIndex[0]].y),
                          primGLToScreenX(point[pIndex[1]].x), primGLToScreenY(point[pIndex[1]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[1]].x), primGLToScreenY(point[pIndex[1]].y),
                          primGLToScreenX(point[pIndex[2]].x), primGLToScreenY(point[pIndex[2]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[2]].x), primGLToScreenY(point[pIndex[2]].y),
                          primGLToScreenX(point[pIndex[3]].x), primGLToScreenY(point[pIndex[3]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[3]].x), primGLToScreenY(point[pIndex[3]].y),
                          primGLToScreenX(point[pIndex[0]].x), primGLToScreenY(point[pIndex[0]].y), colWhite);
            }
/*
            dbgMessagef("\n%3.2f, %3.2f, %3.2f, %3.2f",
                       primPointLineIntersection(primScreenToGLX(mouseCursorX()), primScreenToGLY(mouseCursorY()),
                            point[pIndex[0]].x, point[pIndex[0]].y, point[pIndex[1]].x, point[pIndex[1]].y),
                       primPointLineIntersection(primScreenToGLX(mouseCursorX()), primScreenToGLY(mouseCursorY()),
                            point[pIndex[1]].x, point[pIndex[1]].y, point[pIndex[2]].x, point[pIndex[2]].y),
                       primPointLineIntersection(primScreenToGLX(mouseCursorX()), primScreenToGLY(mouseCursorY()),
                            point[pIndex[2]].x, point[pIndex[2]].y, point[pIndex[3]].x, point[pIndex[3]].y),
                       primPointLineIntersection(primScreenToGLX(mouseCursorX()), primScreenToGLY(mouseCursorY()),
                            point[pIndex[3]].x, point[pIndex[3]].y, point[pIndex[0]].x, point[pIndex[0]].y)
                       );
*/
            if (collision->precise->nPolys >= 2)
            {
                fontPrint(primGLToScreenX((point[pIndex[4]].x + point[pIndex[6]].x) / 2.0f),
                           primGLToScreenY((point[pIndex[4]].y + point[pIndex[6]].y) / 2.0f),
                           colWhite, "2");
                primLine2(primGLToScreenX(point[pIndex[4]].x), primGLToScreenY(point[pIndex[4]].y),
                          primGLToScreenX(point[pIndex[5]].x), primGLToScreenY(point[pIndex[5]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[5]].x), primGLToScreenY(point[pIndex[5]].y),
                          primGLToScreenX(point[pIndex[6]].x), primGLToScreenY(point[pIndex[6]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[6]].x), primGLToScreenY(point[pIndex[6]].y),
                          primGLToScreenX(point[pIndex[7]].x), primGLToScreenY(point[pIndex[7]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[7]].x), primGLToScreenY(point[pIndex[7]].y),
                          primGLToScreenX(point[pIndex[4]].x), primGLToScreenY(point[pIndex[4]].y), colWhite);
            }

            if (collision->precise->nPolys >= 3)
            {
                fontPrint(primGLToScreenX((point[pIndex[8]].x + point[pIndex[10]].x) / 2.0f),
                           primGLToScreenY((point[pIndex[8]].y + point[pIndex[10]].y) / 2.0f),
                           colWhite, "3");
                primLine2(primGLToScreenX(point[pIndex[8]].x), primGLToScreenY(point[pIndex[8]].y),
                          primGLToScreenX(point[pIndex[9]].x), primGLToScreenY(point[pIndex[9]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[9]].x), primGLToScreenY(point[pIndex[9]].y),
                          primGLToScreenX(point[pIndex[10]].x), primGLToScreenY(point[pIndex[10]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[10]].x), primGLToScreenY(point[pIndex[10]].y),
                          primGLToScreenX(point[pIndex[11]].x), primGLToScreenY(point[pIndex[11]].y), colWhite);
                primLine2(primGLToScreenX(point[pIndex[11]].x), primGLToScreenY(point[pIndex[11]].y),
                          primGLToScreenX(point[pIndex[8]].x), primGLToScreenY(point[pIndex[8]].y), colWhite);
            }
        }
    }
}
#endif //SEL_DRAW_BOXES

/*-----------------------------------------------------------------------------
    Name        : selCircleComputeGeneral
    Description : Compute a selection circle like the previous function, just
                    without being speific to ships.
    Inputs      : modelView, projection - camera/modellig matrices
                  location - position of the sphere to project.
                  worldRadius - radius of sphere to project.
    Outputs     : destX,Y - screen location of projected circle.
                  destRadius - screen size of the projected circle.
    Return      : void
----------------------------------------------------------------------------*/
void selCircleComputeGeneral(hmatrix *modelView, hmatrix *projection, vector *location, real32 worldRadius, real32 *destX, real32 *destY, real32 *destRadius)
{
    hvector centre, screenSpace, radius, radiusProjected;
    centre.x = location->x;                                 //vector at centre of ship
    centre.y = location->y;
    centre.z = location->z;
    centre.w = 1.0f;

    hmatMultiplyHMatByHVec(&selCameraSpace, modelView, &centre);//in camera space
    radius = selCameraSpace;                                //project another point to determine selection sphere size
    radius.x += worldRadius;
    hmatMultiplyHMatByHVec(&screenSpace, projection, &selCameraSpace);//in screen space
    hmatMultiplyHMatByHVec(&radiusProjected, projection, &radius);
    *destX = screenSpace.x / screenSpace.w;
    *destY = screenSpace.y / screenSpace.w;
    *destRadius = (radiusProjected.x - screenSpace.x) / screenSpace.w;
}

#ifdef DEBUG_COLLSPHERES
/*-----------------------------------------------------------------------------
    Name        : selSelectionDraw0..5
    Description : Draw a particular ship's selection circle at a given
                    level-of-detail.
    Inputs      : ship - pointer to ship to draw about
                  radius - pre-converted integer radius
                  segments - number of segments to draw circles with
                  c - color to render health bar in
                  angle - angle of end of health bar
    Outputs     : ..
    Return      : void
----------------------------------------------------------------------------*/
#define fixedSin45    ((udword)(0.7071067811865 * 65536.0))
void selSelectionDraw5(Ship *ship)      // PLEASE DON'T COMMENT THIS FUNCTION OUT - I need it for testing/debugging - Gary
{   //0 outline circles 1-pixel dim full health circle
    oval outline;
    sdword x, y;//, offset;
    sdword radius;
    sdword segments = 20;
    color c = selShipHealthGreen;

    x = primGLToScreenX(ship->collInfo.selCircleX);
    y = primGLToScreenY(ship->collInfo.selCircleY);
    radius = primGLToScreenScaleX(ship->collInfo.selCircleRadius);

    outline.centreX = x;                                    //draw inner outline circle
    outline.centreY = y;
    outline.radiusX = outline.radiusY = radius;
    primOvalArcOutline2(&outline, 0.0f, TWOPI, 1, segments, c);
#if 0
    if (ship->hotKeyGroup & SEL_HotKeyBit)
    {                                                       //if member of a hot-key group
        offset = fixedSin45 * radius / 65536 + selNumberMargin;
        fontPrint(x + offset, y + offset, selHotKeyNumberColor,
                  selHotKeyString[ship->hotKeyGroup & SEL_HotKeyMask]);
    }
#endif
}
#endif

//lod0: health/fuel with outline and hollow centre
void selStatusDraw0(Ship *ship)
{
    sdword rect[4];
    sdword x;
    sdword halfWidth;
    real32 radius;

    selCircleComputeRect((float *)ship, 0, rect);
    x = primGLToScreenX(ship->collInfo.selCircleX);
    radius = max(ship->collInfo.selCircleRadius, selMinSelectionRadius);
    halfWidth = primGLToScreenScaleX(radius * selSelectionWidthFactor0);

    selStatusDrawCommon(ship, 0);

    if (tutorial && tutPointerShip == ship && tutPointerShipHealthRect)
    {
        tutPointerShipHealthRect->x0 = x - halfWidth - 2;
        tutPointerShipHealthRect->x1 = x + halfWidth + 2;
        tutPointerShipHealthRect->y0 = rect[1] - 2;
        tutPointerShipHealthRect->y1 = rect[3] + 2;
        if (ship->staticinfo->maxresources != 0)
        {
            tutPointerShipHealthRect->y1 += 5;
        }
        if (shipCanUseBigGun(ship) || dreadnoughtHasCannon(ship))
        {
            tutPointerShipHealthRect->y1 += 5;
        }
    }
}
//lod1: 4-pixel health/fuel bar
void selStatusDraw1(Ship *ship)
{
    sdword rect[4];
    sdword x;
    sdword halfWidth;
    real32 radius;

    selCircleComputeRect((float *)ship, 1, rect);
    x = primGLToScreenX(ship->collInfo.selCircleX);
    radius = max(ship->collInfo.selCircleRadius, selMinSelectionRadius);
    halfWidth = primGLToScreenScaleX(radius * selSelectionWidthFactor0);

    selStatusDrawCommon(ship, 1);

    if (tutorial && tutPointerShip == ship && tutPointerShipHealthRect)
    {
        tutPointerShipHealthRect->x0 = x - halfWidth - 2;
        tutPointerShipHealthRect->x1 = x + halfWidth + 2;
        tutPointerShipHealthRect->y0 = rect[1] - 2;
        tutPointerShipHealthRect->y1 = rect[3] + 2;
        if (ship->staticinfo->maxresources != 0)
        {
            tutPointerShipHealthRect->y1 += 4;
        }
        if (shipCanUseBigGun(ship) || dreadnoughtHasCannon(ship))
        {
            tutPointerShipHealthRect->y1 += 4;
        }
    }
}
//lod2: just health and damage only 2 pixels tall
void selStatusDraw2(Ship *ship)
{
    sdword rect[4];
    sdword x;
    sdword halfWidth;
    real32 radius;

    selCircleComputeRect((float *)ship, 2, rect);
    x = primGLToScreenX(ship->collInfo.selCircleX);
    radius = max(ship->collInfo.selCircleRadius, selMinSelectionRadius);
    halfWidth = primGLToScreenScaleX(radius * selSelectionWidthFactor0);

    selStatusDrawCommon(ship, 2);

    if (tutorial && tutPointerShip == ship && tutPointerShipHealthRect)
    {
        tutPointerShipHealthRect->x0 = x - halfWidth - 2;
        tutPointerShipHealthRect->x1 = x + halfWidth + 2;
        tutPointerShipHealthRect->y0 = rect[1] - 2;
        tutPointerShipHealthRect->y1 = rect[3] + 2;
    }
    if (tutorial && tutPointerShip == ship && tutPointerShipGroupRect)
    {
        if (ship->staticinfo->maxresources != 0)
        {
            tutPointerShipGroupRect->y1 += 3;
        }
        if (shipCanUseBigGun(ship) || dreadnoughtHasCannon(ship))
        {
            tutPointerShipGroupRect->y1 += 3;
        }
    }
}

//lod3: health and fuel 1 pixel tall and solid
void selStatusDraw3(Ship *ship)
{
    real32 radius, factor;
    //sdword health, maxHealth;
    sdword x, y, halfWidth;
    color healthColor;

    x = primGLToScreenX(ship->collInfo.selCircleX);
    y = primGLToScreenY(ship->collInfo.selCircleY);
    radius = max(ship->collInfo.selCircleRadius, selMinSelectionRadius);//radius of selection circle
    halfWidth = primGLToScreenScaleX(radius * selSelectionWidthFactor0);//width of bars
    //health = (sdword) ship->health;                                  //get ship health color and size
    //maxHealth = (sdword) ((ShipStaticInfo *)ship->staticinfo)->maxhealth;
    //factor = (real32)health / (real32)maxHealth;            //amount percentage of full health
    factor = ship->health * ((ShipStaticInfo *)ship->staticinfo)->oneOverMaxHealth;
    if (factor > selShipHealthGreenFactor)
    {
        healthColor = selShipHealthSolidGreen;
    }
    else if (factor > selShipHealthYellowFactor)
    {
        healthColor = selShipHealthSolidYellow;
    }
    else
    {
        healthColor = selShipHealthSolidRed;
    }
    //draw health
    y = y - primGLToScreenScaleX(radius);
    primNonAALine2(x - halfWidth, y, x + halfWidth, y, healthColor);

    if(tutorial && tutPointerShip == ship && tutPointerShipHealthRect)
    {
        tutPointerShipHealthRect->x0 = x - halfWidth - 2;
        tutPointerShipHealthRect->x1 = x + halfWidth + 2;
        tutPointerShipHealthRect->y0 = y - 2;
        tutPointerShipHealthRect->y1 = y + 2;
    }

}

//lod4: health 1 pixel tall and solid
void selStatusDraw4(Ship *ship)
{
    real32 radius;
    sdword x, y, halfWidth;
    x = primGLToScreenX(ship->collInfo.selCircleX);
    y = primGLToScreenY(ship->collInfo.selCircleY);
    radius = max(ship->collInfo.selCircleRadius, selMinSelectionRadius);
    halfWidth = primGLToScreenScaleX(radius * selSelectionWidthFactor0);
    y = y - primGLToScreenScaleX(radius);
    selStatusDrawCommon(ship, 4);

    if (tutorial && tutPointerShip == ship && tutPointerShipHealthRect)
    {
        tutPointerShipHealthRect->x0 = x - halfWidth - 2;
        tutPointerShipHealthRect->x1 = x + halfWidth + 2;
        tutPointerShipHealthRect->y0 = y - 2;
        tutPointerShipHealthRect->y1 = y + 2;
    }
}


void selStatusDrawNULL(Ship *ship)
{
    ;//!!! do nothing
}
typedef void (*seldrawfunction)(Ship *ship);
seldrawfunction selStatusDraw[SEL_NumberLOD] =
{
#ifdef DEBUG_COLLSPHERES
    selSelectionDraw5,
    selSelectionDraw5,
    selSelectionDraw5,
    selSelectionDraw5,
    selSelectionDraw5,
#else
    selStatusDraw0,
    selStatusDraw1,
    selStatusDraw2,
    selStatusDraw3,
    selStatusDraw4,
#endif
    selStatusDrawNULL,
};
/*-----------------------------------------------------------------------------
    Name        : selSelectedDraw/selSelectingDraw
    Description : Draw overlays for the list of selected/selecting ships
    Inputs      : void
    Outputs     : updates the selCentrePoint vector
    Return      : void
    Note        : rendering system must be in 2D primitive mode when this is
                    called.
----------------------------------------------------------------------------*/
void selSelectedDraw(void)
{
    sdword index;
    Ship *ship;
    Ship *focusedShip;
    fonthandle currentFont = fontCurrentGet();
    sdword currentLOD;
    real32 factor;
    color fillColor;
    color emptyColor;
    sdword rectCoords[4];
    rectangle outlineRect;

    rndTextureEnable(FALSE);
    rndLightingEnable(FALSE);

    for (index = 0; index < selSelected.numShips; index++)
    {
        ship = selSelected.ShipPtr[index];
        dbgAssert(ship->objtype == OBJ_ShipType);

        if (univSpaceObjInRenderList((SpaceObj *)ship))
        {
            if (ship->collInfo.selCircleRadius > 0.0f && !ccIsPilotingShip(ship))
            {
                currentLOD = ship->currentLOD;
                dbgAssert(currentLOD < SEL_NumberLOD);
                if (ShipTypeIsStandardMothership(ship->shiptype) && (currentLOD > 0))
                {
                    currentLOD--;
                }
                glShadeModel(GL_FLAT);
                selStatusDrawCommon(ship, currentLOD);
#if SEL_DRAW_BOXES
#if RND_VISUALIZATION
                if (RENDER_BOXES)
                {
                    selDrawBoxes((SpaceObjRotImpTarg *)ship);
                }
#endif
#endif
            }
        }
    }

    if (!ioOverlayPageIsNonPrimary() &&
        universe.mainCameraCommand.currentCameraStack->focus.numShips == 1)
    {
        focusedShip = universe.mainCameraCommand.currentCameraStack->focus.ShipPtr[0];
        if (focusedShip->objtype == OBJ_ShipType &&
            focusedShip->playerowner != universe.curPlayerPtr &&
            rmTechIsResearched(universe.curPlayerPtr, TechAdvancedSensors) &&
            !nisWideScreenActive &&
            (focusedShip->flags2 & 0x10) == 0 &&
            !bitTest(focusedShip->flags, SOF_Dead) &&
            !shipIsBeastShip(focusedShip))
        {
            currentLOD = focusedShip->currentLOD;
            selHealthFactorClamp(focusedShip, &factor);
            selHealthColorPairGet(&factor, &fillColor, &emptyColor);
            selStatusBarDraw(focusedShip, currentLOD, 0, &factor, &fillColor, &emptyColor);
            selCircleComputeRect((float *)focusedShip, currentLOD, rectCoords);

            outlineRect.x0 = rectCoords[0] - 1;
            outlineRect.x1 = rectCoords[2] + 1;
            outlineRect.y0 = rectCoords[1] - 1;
            outlineRect.y1 = rectCoords[3] + 1;
            primGLRectOutline2(&outlineRect, 1, colRGB(255, 0, 0));
        }
    }

    fontMakeCurrent(currentFont);
}

void selSelectingDraw(void)
{
    sdword index, j;
    SpaceObjRotImpTarg *target;
    rectangle rect;
    real32 radius;
    real32 x0, y0, x1, y1;
    vector *point;

    for (index = 0; index < selSelecting.numTargets; index++)
    {
        target = selSelecting.TargetPtr[index];

        if (univSpaceObjInRenderList((SpaceObj *)target))
        {
            if (target->collInfo.selCircleRadius > 0.0f)
            {
                if (target->collInfo.precise != NULL &&
                    target->currentLOD <= target->staticinfo->staticheader.staticCollInfo.preciseSelection)
                {                                           //if we should perform precise collision checking
                    point = target->collInfo.precise->worldRectPos;//list of points
                    x0 = y1 = REALlyBig;
                    x1 = y0 = REALlyNegative;
                    for (j = 0; j < 8; j++)
                    {                                       //find bounds of selection
                        x0 = min(x0, point[j].x);
                        y0 = max(y0, point[j].y);
                        x1 = max(x1, point[j].x);
                        y1 = min(y1, point[j].y);
                    }
                    rect.x0 = primGLToScreenX(x0);
                    rect.y0 = primGLToScreenY(y0);
                    rect.x1 = primGLToScreenX(x1);
                    rect.y1 = primGLToScreenY(y1);
                }
                else
                {
                    if (target->objtype == OBJ_ShipType)
                        radius = max(target->collInfo.selCircleRadius * ((Ship *)target)->staticinfo->tacticalSelectionScale, selMinSelectionRadius);
                    else
                        radius = max(target->collInfo.selCircleRadius, selMinSelectionRadius);
                    rect.x0 = primGLToScreenX(target->collInfo.selCircleX - radius);
                    rect.x1 = primGLToScreenX(target->collInfo.selCircleX + radius);
                    rect.y0 = primGLToScreenY(target->collInfo.selCircleY + radius);
                    rect.y1 = primGLToScreenY(target->collInfo.selCircleY - radius);
                }
                primRectOutline2(&rect, 1, selSelectingColor);//!!! note the hard-coded numbers
                if (tutorial == TUTORIAL_ONLY)
                {
                    tutGameMessage("Game_SelectingRect");
                }
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : selCentrePointComputeGeneral
    Description : Compute the centre point of any selection
    Inputs      : selection - the selection to compute the center of
                  average_size - optional return variable - average size of the selection
    Outputs     :
    Return      : vector
----------------------------------------------------------------------------*/
vector selCentrePointComputeGeneral(MaxSelection *selection, real32 *average_size)
{
    sdword index;
    real32 realIndex;
    Ship *ship;
    vector minVector = {REALlyBig, REALlyBig, REALlyBig};
    vector maxVector = {REALlyNegative, REALlyNegative, REALlyNegative};
    vector returnVector;

    returnVector.x = returnVector.y = returnVector.z = *average_size = 0.0f;
    for (index = 0; index < selection->numShips; index++)
    {
        ship = selection->ShipPtr[index];
        dbgAssert(ship->objtype == OBJ_ShipType);

        minVector.x = min(minVector.x, ship->posinfo.position.x);
        minVector.y = min(minVector.y, ship->posinfo.position.y);
        minVector.z = min(minVector.z, ship->posinfo.position.z);
        maxVector.x = max(maxVector.x, ship->posinfo.position.x);
        maxVector.y = max(maxVector.y, ship->posinfo.position.y);
        maxVector.z = max(maxVector.z, ship->posinfo.position.z);
        *average_size += ship->staticinfo->staticheader.staticCollInfo.collspheresize * ship->magnitudeSquared;
//        returnVector.x += ship->posinfo.position.x;
//        returnVector.y += ship->posinfo.position.y;
//        returnVector.z += ship->posinfo.position.z;
    }
    realIndex = (real32)index;
    *average_size /= realIndex;
//    returnVector.x /= realIndex;
//    returnVector.y /= realIndex;
//    returnVector.z /= realIndex;
    returnVector.x = (minVector.x + maxVector.x) / 2.0f;
    returnVector.y = (minVector.y + maxVector.y) / 2.0f;
    returnVector.z = (minVector.z + maxVector.z) / 2.0f;

    return returnVector;
}



/*-----------------------------------------------------------------------------
    Name        : selCentrePointCompute
    Description : Compute the centre point of current selection
    Inputs      : void
    Outputs     : updates the centre point of the current selection.
    Return      : void
----------------------------------------------------------------------------*/
void selCentrePointCompute(void)
{
    selCentrePoint = selCentrePointComputeGeneral(&selSelected, &selAverageSize);
}


/*-----------------------------------------------------------------------------
    Name        : selSelectionCopy
    Description : Copy a MaxSelection structure from one place to another.
    Inputs      : source - source MaxSelection structure
                  dest - dest MaxSelection strucure
    Outputs     : guess
    Return      : number of entries copied
----------------------------------------------------------------------------*/
sdword selSelectionCopy(MaxAnySelection *dest, MaxAnySelection *source)
{
    sdword index;
    TargetPtr *sourceP, *destP;

    sourceP = source->TargetPtr;                            //pointers to the base of lists
    destP =   dest->TargetPtr;

    dbgAssert(source->numTargets < COMMAND_MAX_SHIPS);

    for (index = source->numTargets; index > 0; index--)
    {                                                       //for each ship in source
        *destP = *sourceP;                                  //copy a single ShipPtr
        sourceP++;                                          //inc source/dest pointers
        destP++;
    }
    dest->numTargets = source->numTargets;                  //copy number of ships

    return(source->numTargets);                             //return number of ships copied
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionIsReinforced
    Description : See if a group will be is being reinforced by a new selection.
    Inputs      : dest - selection to be reinforced
                  source - selection that will replace this new selection
    Outputs     :
    Return      : TRUE if it's a reinforcement.
----------------------------------------------------------------------------*/
bool32 selSelectionIsReinforced(MaxAnySelection *dest, MaxAnySelection *source)
{
    sdword index;
    sdword nAlreadyInSelection = 0;
    sdword nNeededAlready;

    if (source->numTargets <= dest->numTargets)
    {                                                       //if we're replacing no more ships
        return(FALSE);                                      //it's not reinforcing at all!
    }
    else if (dest->numTargets == 0)
    {                                                       //if starting selection is empty
        return(FALSE);                                      //that's not a reinforcement
    }
    else if (dest->numTargets == 1)
    {                                                       //only reinforcing 1 guy
        nNeededAlready = 1;
    }
    else
    {
        nNeededAlready = dest->numTargets * SEL_ReinforceFactor;
    }

    for (index = 0; index < dest->numTargets; index++)
    {
        if (selShipInSelection((ShipPtr *)source->TargetPtr, source->numTargets, (ShipPtr)dest->TargetPtr[index]))
        {                                                   //if this ship is in the replacement selection
            nAlreadyInSelection++;
        }
    }
    if (nAlreadyInSelection >= nNeededAlready)
    {                                                       //if there is enough reinforcment here to count
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionCopyByClass
    Description : Copy a MaxSelection structure from one place to another, copying
                    only the ships which match a specified class.
    Inputs      : source - source MaxSelection structure
                  dest - dest MaxSelection strucure
                  classMask - class to match
    Outputs     : guess
    Return      : number of entries copied
----------------------------------------------------------------------------*/
sdword selSelectionCopyByClass(MaxSelection *dest, MaxSelection *source, ShipClass classMask)
{
    sdword index;
    ShipPtr *sourceP, *destP;

    sourceP = source->ShipPtr;                              //pointers to the base of lists
    destP =   dest->ShipPtr;

    dbgAssert(source->numShips < COMMAND_MAX_SHIPS);

    dest->numShips = 0;

    for (index = source->numShips; index > 0; index--)
    {                                                       //for each ship in source
        dbgAssert((*sourceP)->objtype == OBJ_ShipType);
        if ((*sourceP)->staticinfo->shipclass == classMask)
        {                                                   //if classes match
            *destP = *sourceP;                              //copy a single ShipPtr
            destP++;
            dest->numShips++;
        }
        sourceP++;                                          //inc source/dest pointers
    }
    return(dest->numShips);                                 //return number of ships copied
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionCopyByType
    Description : Copy a MaxSelection structure from one place to another, copying
                    only the ships which match a specified class.
    Inputs      : source - source MaxSelection structure
                  dest - dest MaxSelection strucure
                  classMask - class to match
    Outputs     : guess
    Return      : number of entries copied
----------------------------------------------------------------------------*/
sdword selSelectionCopyByType(MaxSelection *dest, MaxSelection *source, ShipType typeMask)
{
    sdword index;
    ShipPtr *sourceP, *destP;

    sourceP = source->ShipPtr;                              //pointers to the base of lists
    destP =   dest->ShipPtr;

    dbgAssert(source->numShips < COMMAND_MAX_SHIPS);

    dest->numShips = 0;

    for (index = source->numShips; index > 0; index--)
    {                                                       //for each ship in source
        dbgAssert((*sourceP)->objtype == OBJ_ShipType);
        if ((*sourceP)->shiptype == typeMask)
        {                                                   //if classes match
            *destP = *sourceP;                              //copy a single ShipPtr
            destP++;
            dest->numShips++;
        }
        sourceP++;                                          //inc source/dest pointers
    }
    return(dest->numShips);                                 //return number of ships copied
}

/*-----------------------------------------------------------------------------
    Name        : selSortCompare
    Description : Sort a selection by ship pointer, as a callback from qsort.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
int selSortCompare(const void *p0, const void *p1)
{
    if (*((TargetPtr *)p0) > *((TargetPtr *)p1))
    {
        return(1);
    }
    else if (*((TargetPtr *)p0) < *((TargetPtr *)p1))
    {
        return(-1);
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : selSelectionCompare
    Description : Compare 2 selections
    Inputs      : s0, s1 - pointers to selections to compare
    Outputs     : sorts both selections before comparing them
    Return      : 0 if identical, nonzero otherwise
----------------------------------------------------------------------------*/
sdword selSelectionCompare(MaxAnySelection *s0, MaxAnySelection *s1)
{
    sdword index;
    TargetPtr *sourceP, *destP;

    sourceP = s0->TargetPtr;                               //pointers to the base of lists
    destP =   s1->TargetPtr;

    dbgAssert(s0->numTargets < COMMAND_MAX_SHIPS);
    dbgAssert(s1->numTargets < COMMAND_MAX_SHIPS);

    qsort(s0->TargetPtr, s0->numTargets, sizeof(TargetPtr), selSortCompare);
    qsort(s1->TargetPtr, s1->numTargets, sizeof(TargetPtr), selSortCompare);
    if (s0->numTargets != s1->numTargets)
    {
        return(1);
    }
    for (index = s0->numTargets; index > 0; index--)
    {                                                       //for each ship in both lists
        if ((sdword)(*destP - *sourceP) != 0)
        {
            return(1);
        }
        sourceP++;                                          //inc source/dest pointers
        destP++;
    }
    return(0);                                              //return identical
}

/*-----------------------------------------------------------------------------
    Name        : selShipsInSelection
    Description : Search for a set of ships in a particular selection
    Inputs      : dest - where to search for matching ships
                  list - what ships to match
    Outputs     : sorts both lists for better searching
    Return      : TRUE of ship found, false otherwise
----------------------------------------------------------------------------*/
bool32 selShipsInSelection(MaxSelection *dest, MaxSelection *list)
{
    sdword numShips;
    sdword listNumShips;
    ShipPtr *destShipPtr;
    sdword requiredMatches;
    sdword matches;
    sdword destIndex;
    sdword listIndex;

    numShips = dest->numShips;
    listNumShips = list->numShips;
    matches = 0;

    if (listNumShips <= numShips)
    {
        return FALSE;
    }
    if (numShips == 0)
    {
        return FALSE;
    }
    if (numShips == 1)
    {
        requiredMatches = 1;
    }
    else
    {
        requiredMatches = 215 * numShips / 256;
    }
    if (numShips > 0)
    {
        destShipPtr = dest->ShipPtr;
        for (destIndex = dest->numShips; destIndex > 0; destIndex--, destShipPtr++)
        {
            for (listIndex = 0; listIndex < listNumShips; listIndex++)
            {
                if (*destShipPtr == list->ShipPtr[listIndex])
                {
                    matches++;
                    break;
                }
            }
        }
    }
    return (matches >= requiredMatches);
}

/*-----------------------------------------------------------------------------
    Name        : selSelectHotKeyGroup
    Description : selects a given hotkeygroup
    Inputs      : hotkeygroup to select
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void selSelectHotKeyGroup(MaxSelection *hotkeygroup)
{
    sdword i;
    Ship *ship;

    selSelected.numShips = 0;
    for (i=0;i<hotkeygroup->numShips;i++)
    {
        ship = hotkeygroup->ShipPtr[i];
        if ((ship->flags & (SOF_Hide|SOF_Disabled|SOF_Hyperspace|SOF_Crazy)) == 0 && bitTest(ship->flags, SOF_Selectable))
        {
            selSelected.ShipPtr[selSelected.numShips++] = ship;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : selHotKeyNumbersSet
    Description : Set the hot key group number for all ships in a specified
                    selection.
    Inputs      : group - selection to set numbers for
    Outputs     : Sets the hotKeyGroup of all ships in selection to number.
                    Also removes them from other hot-key groups they may be in.
    Return      : OKAY
----------------------------------------------------------------------------*/
sdword selHotKeyNumbersSet(sdword group)
{
    sdword index;
    ShipPtr *destP;

    destP = selHotKeyGroup[group].ShipPtr;

    for (index = selHotKeyGroup[group].numShips; index > 0; index--)
    {                                                       //for each ship in source
        selHotKeySet(*destP, group);
//        if (selHotKeyGroupNumberTest(*destP) == SEL_InvalidHotKey)
//        {                                                   //if this one belongs to no hot key group
            selHotKeyGroupNumberSet(*destP, group);         //set it to this group
//        }
        destP++;
    }
    return(OKAY);
}

/*-----------------------------------------------------------------------------
    Name        : selHotKeyGroupRemoveReferences
    Description : Remove references to this hot-key group from the ships in this group.
    Inputs      : group - group to remove references from
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void selHotKeyGroupRemoveReferences(sdword group)
{
    sdword index, j;
    ShipPtr *sourceP;
    MaxSelection *destSelection, *thisSelection;

    thisSelection = &selHotKeyGroup[group];
    sourceP = thisSelection->ShipPtr;

    for (index = thisSelection->numShips; index > 0; index--)
    {                                                       //for each ship in source
        if (selHotKeyGroupNumberTest(*sourceP) == group)
        {                                                   //if ship in this hot-key group
            destSelection = &selHotKeyGroup[group];
            if (destSelection != thisSelection)
            {                                               //don't remove it from this selection because this selection is about to be overwritten anyhow
                clRemoveShipFromSelection((SelectAnyCommand *)destSelection, *sourceP);
            }
            selHotKeyClear(*sourceP, group);
            selHotKeyGroupNumberClear(*sourceP);
            for (j = 0; j < SEL_NumberHotKeyGroups; j++)
            {                                               //search for another hotkey group this guy belongs to
                if (selHotKeyTest(*sourceP, j))
                {
//                    selHotKeySet(*sourceP, j);
                    selHotKeyGroupNumberSet(*sourceP, j);
                    goto foundOne;
                }
            }
            //no other hot-key group found, set hot key group number
            selHotKeyGroupNumberClear(*sourceP);
foundOne:;
        }

        sourceP++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : selHotKeyGroupRemoveReferencesFromAllGroups
    Description : Remove references to all hot-key groups from the ships in selSelected.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void selHotKeyGroupRemoveReferencesFromAllGroups(void)
{
    sdword index, j;
    ShipPtr *sourceP;
    MaxSelection *destSelection;

    sourceP = selSelected.ShipPtr;

    for (index = selSelected.numShips; index > 0; index--)
    {                                                       //for each ship in source
        selHotKeyGroupNumberClear(*sourceP);
        for (j = 0; j < SEL_NumberHotKeyGroups; j++)
        {                                                   //see what groups this ship is in
            if (selHotKeyTest(*sourceP, j))
            {                                               //if it's in this group
                destSelection = &selHotKeyGroup[j];         //remove it from this group
                clRemoveShipFromSelection((SelectAnyCommand *)destSelection, *sourceP);
            }
        }
        selHotKeyClearAll(*sourceP);                        //not in any group
//        (*sourceP)->hotKeyGroup = SEL_InvalidHotKey;

        sourceP++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : selHotKeyGroupsVerify
    Description : Verify integrity of hot-key groups
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if SEL_ERROR_CHECKING
void selHotKeyGroupsVerify(void)
{
    sdword index, j;

    for (index = 0; index < SEL_NumberHotKeyGroups; index++)
    {                                                       //for all hot-key groups
        for (j = 0; j < selHotKeyGroup[index].numShips; j++)
        {
            dbgAssert(selHotKeyGroup[index].ShipPtr[j]->objtype == OBJ_ShipType);
            if (selAnyHotKeyTest(selHotKeyGroup[index].ShipPtr[j]))
            {
                dbgAssert(selHotKeyGroupNumberTest(selHotKeyGroup[index].ShipPtr[j]) != SEL_InvalidHotKey);
                dbgAssert(selHotKeyTest(selHotKeyGroup[index].ShipPtr[j], index));
                dbgAssert(selHotKeyTest(selHotKeyGroup[index].ShipPtr[j], selHotKeyGroupNumberTest(selHotKeyGroup[index].ShipPtr[j])));
            }
            else
            {
                dbgAssert(selHotKeyGroupNumberTest(selHotKeyGroup[index].ShipPtr[j]) == SEL_InvalidHotKey);
            }
        }
    }
}
#endif //SEL_ERROR_CHECKING

/*-----------------------------------------------------------------------------
    Name        : selSelectionDimensions
    Description : Compute the on-screen selection circle for a group of ships
    Inputs      : selection - selection to compute on-screen coordinates of
                  modelView, projection - matrices for camera and projection
    Outputs     : destX, destY, destRad - on-screen positon of the selection of ships
    Return      :
----------------------------------------------------------------------------*/
void selSelectionDimensions(hmatrix *modelView, hmatrix *projection, SelectCommand *selection, real32 *destX, real32 *destY, real32 *destRad)
{
    sdword index;
    real32 averageSize;
    Ship *ship;
    real32 radius, maxRadius = 0;
    vector centre = {0, 0, 0}, distance;

    /*
    if (smSensorsActive)
    {
    }
    else
    */
    {                                                       //not in the SM: plain-on-screen position of ships
        //run through all ships to compute median position.
        centre = selCentrePointComputeGeneral((MaxSelection *)selection, &averageSize);
        /*
        for (index = 0; index < selection->numShips; index++)
        {
            ship = selection->ShipPtr[index];
            vecAddTo(centre, ship->posinfo.position);
        }
        radius = 1.0f / (real32)index;
        vecMultiplyByScalar(centre, radius);                //compute mean position
        */
        //run through again to find max radius
        for (index = 0; index < selection->numShips; index++)
        {
            ship = selection->ShipPtr[index];
            vecSub(distance, centre, ship->posinfo.position);//distance to mean
            if (bitTest(ship->flags2, BIT3))
            {
                radius = shipGetCollSphereSize((SpaceObj *)ship);
            }
            else
            {
                radius = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
            }
            radius = vecMagnitudeSquared(distance) + radius * radius;//square of distance
            if (radius > maxRadius)
            {
                maxRadius = radius;
            }
        }
        maxRadius = fsqrt(maxRadius);
        selCircleComputeGeneral(modelView, projection, &centre, maxRadius, destX, destY, destRad);
    }
}
BOOL shipIsRammingFrigate(Ship *ship)
{
    return ship->shiptype == sRammingFrigate || ship->shiptype == bRammingFrigate;
}

BOOL shipIsRammingFrigateType(Ship *ship)
{
    return ship->objtype == OBJ_ShipType && shipIsRammingFrigate(ship);
}

BOOL shipIsInRammingMode(Ship *ship)
{
    return *((ubyte *)ship->ShipSpecifics) == 5;
}

static void selStatusCircleComputeRect(Ship *ship, sdword lod, sdword *rect)
{
    selCircleComputeRect((float *)ship, lod, rect);
}

void selCircleComputeRect(float *screenCircle, sdword lod, sdword *rect)
{
    Ship *ship = (Ship *)screenCircle;
    sdword x;
    sdword y;
    sdword screenRadius;
    real32 barWidth;
    real32 barHeight;
    sdword barHeightPixels;

    x = primGLToScreenX(ship->collInfo.selCircleX);
    y = primGLToScreenY(ship->collInfo.selCircleY);
    screenRadius = primGLToScreenScaleX(max(ship->collInfo.selCircleRadius, selMinSelectionRadius));

    barWidth = 1.6f * (real32)screenRadius;
    if (barWidth < 8.0f)
    {
        barWidth = 8.0f;
    }

    barHeight = 0.03125f * (real32)screenRadius;
    if (barHeight < 2.0f)
    {
        barHeight = 2.0f;
    }

    barHeightPixels = (sdword)barHeight;
    rect[0] = x - ((sdword)(barWidth * 0.5f)) - 1;
    rect[1] = y - (screenRadius + barHeightPixels);
    rect[2] = rect[0] + 4 + (sdword)(barWidth - (barHeight + barHeight));
    rect[3] = rect[1] + barHeightPixels;
}

static color selStatusPulseBlendColor(color from, color to, real32 factor)
{
    sdword factor255;
    sdword invFactor;
    sdword red;
    sdword green;
    sdword blue;

    if (factor < 0.0f)
    {
        factor = 0.0f;
    }
    else if (factor > 1.0f)
    {
        factor = 1.0f;
    }

    factor255 = (sdword)(factor * 255.0f);
    invFactor = 255 - factor255;
    red = (colRed(from) * invFactor + colRed(to) * factor255) / 255;
    green = (colGreen(from) * invFactor + colGreen(to) * factor255) / 255;
    blue = (colBlue(from) * invFactor + colBlue(to) * factor255) / 255;
    return colRGB(red, green, blue);
}

static void selStatusSetFont(sdword lod)
{
    switch (lod)
    {
        case 0:
            fontMakeCurrent(selGroupFont0);
            break;
        case 1:
            fontMakeCurrent(selGroupFont1);
            break;
        case 2:
            fontMakeCurrent(selGroupFont2);
            break;
        default:
            fontMakeCurrent(selGroupFont3);
            break;
    }
}

void selStatusBarDraw(Ship *ship, sdword lod, sdword row, real32 *factor, color *fillColor, color *emptyColor)
{
    sdword rectCoords[4];
    rectangle fillRect, emptyRect;
    real32 clamped;
    sdword yOffset;

    clamped = *factor;
    if (clamped < 0.0f)
    {
        clamped = 0.0f;
    }
    else if (clamped > 1.0f)
    {
        clamped = 1.0f;
    }

    selCircleComputeRect((float *)ship, lod, rectCoords);
    yOffset = ((rectCoords[3] - rectCoords[1]) + 1) * row;

    fillRect.x0 = rectCoords[0];
    fillRect.x1 = rectCoords[0] + (sdword)((rectCoords[2] - rectCoords[0]) * clamped);
    fillRect.y0 = rectCoords[1] + yOffset;
    fillRect.y1 = rectCoords[3] + yOffset;

    emptyRect.x0 = fillRect.x1;
    emptyRect.x1 = rectCoords[2];
    emptyRect.y0 = fillRect.y0;
    emptyRect.y1 = fillRect.y1;

    primRectSolid2(&fillRect, *fillColor);
    primRectSolid2(&emptyRect, *emptyColor);
}

void selHealthColorPairGet(real32 *factor, color *fillColor, color *emptyColor)
{
    if (*factor > selShipHealthGreenFactor)
    {
        *fillColor = selShipHealthGreen;
        *emptyColor = selShipHealthDarkGreen;
    }
    else if (*factor > selShipHealthYellowFactor)
    {
        *fillColor = selShipHealthYellow;
        *emptyColor = selShipHealthDarkYellow;
    }
    else
    {
        *fillColor = selShipHealthRed;
        *emptyColor = selShipHealthDarkRed;
    }
}

void selHealthFactorClamp(Ship *ship, real32 *factor)
{
    *factor = ship->health * ship->staticinfo->oneOverMaxHealth;
    if (*factor < 0.0f)
    {
        *factor = 0.0f;
    }
    else if (*factor > 1.0f)
    {
        *factor = 1.0f;
    }

}

static real32 selGravWellRemainingFraction(Ship *ship)
{
    GravWellGeneratorStatics *statics;
    GravWellGeneratorSpec *spec;
    real32 factor;

    statics = (GravWellGeneratorStatics *)ship->staticinfo->custstatinfo;
    spec = (GravWellGeneratorSpec *)ship->ShipSpecifics;
    if (statics == NULL || statics->OperationTime <= 0.0f)
    {
        return 0.0f;
    }

    factor = (statics->OperationTime - spec->TimeOn) / statics->OperationTime;
    if (factor < 0.0f)
    {
        return 0.0f;
    }
    if (factor > 1.0f)
    {
        return 1.0f;
    }
    return factor;
}

void selStatusDrawCommon(Ship *ship, sdword lod)
{
    real32 factor;
    color fillColor;
    color emptyColor;
    sdword row;

    selHealthFactorClamp(ship, &factor);
    selHealthColorPairGet(&factor, &fillColor, &emptyColor);
    selStatusBarDraw(ship, lod, 0, &factor, &fillColor, &emptyColor);

    row = 1;
    if ((ship->shiptype == sACV || ship->shiptype == bACV) &&
        ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_EMP))
    {
        factor = acvGetEMPChargeProgress(ship);
        selStatusBarDraw(ship, lod, row, &factor, &selStatusChargeColor, &selStatusChargeDarkColor);
        row++;
    }

    if (ship->staticinfo->maxresources != 0)
    {
        factor = (real32)ship->resources / (real32)ship->staticinfo->maxresources;
        selStatusBarDraw(ship, lod, row, &factor, &selShipResourceColor, &selShipDarkResourceColor);
        row++;
    }

    if (shipCanUseBigGun(ship))
    {
        factor = bigGunChargeFraction(ship);
        selStatusBarDraw(ship, lod, row, &factor, &selStatusBigGunColor, &selStatusBigGunDarkColor);
        row++;
    }

    if (dreadnoughtHasCannon(ship))
    {
        factor = dreadnoughtGetCannonTimerRemaining(ship);
        selStatusBarDraw(ship, lod, row, &factor, &selStatusCannonColor, &selStatusCannonDarkColor);
        row++;
    }

    if (mothershipHasReadyBigGun(ship))
    {
        factor = mothershipGetBigGunProgress(ship);
        selStatusBarDraw(ship, lod, row, &factor, &selStatusBeastBigGunColor, &selStatusBeastBigGunDarkColor);
        row++;
    }

    if (shipGetCloakChargeFraction(ship, &factor))
    {
        selStatusBarDraw(ship, lod, row, &factor, &selShipResourceColor, &selShipDarkResourceColor);
        row++;
    }

    if (ShipTypeIsGravWellGeneratorType(ship->shiptype))
    {
        factor = selGravWellRemainingFraction(ship);
        selStatusBarDraw(ship, lod, row, &factor, &selShipResourceColor, &selShipDarkResourceColor);
        row++;
    }

    if (ship->upgradeActive &&
        ship->upgradeEndTime > ship->upgradeStartTime)
    {
        factor = (universe.totaltimeelapsed - ship->upgradeStartTime) /
                 (ship->upgradeEndTime - ship->upgradeStartTime);
        if (factor < 0.0f)
        {
            factor = 0.0f;
        }
        else if (factor > 1.0f)
        {
            factor = 1.0f;
        }
        selStatusBarDraw(ship, lod, row, &factor, &selStatusBuildProgressColor, &selStatusBuildProgressDarkColor);
        row++;
    }

    selStatusDrawSpecialStatePulse(ship, lod);
    selStatusDrawEffectIcon(ship, lod);
    selStatusDrawPlayerSlotDots(ship, lod);
    selStatusDrawCounterPips(ship, lod);
    selStatusDrawBadgeText(ship, lod);
    selStatusDrawAuxText(ship, lod, row);
}

sdword selProtectDraw(void)
{
    sdword index;

    for (index = 0; index < attackcom.numTargets; index++)
    {
        SpaceObjRotImpTarg *target = attackcom.TargetPtr[index];
        rectangle rect;
        vector *point;
        real32 x0, y0, x1, y1;
        real32 radius;
        sdword pointIndex;

        if (!univSpaceObjInRenderList((SpaceObj *)target) || target->collInfo.selCircleRadius <= 0.0f)
        {
            continue;
        }

        if (target->collInfo.precise != NULL &&
            target->currentLOD <= target->staticinfo->staticheader.staticCollInfo.preciseSelection)
        {
            point = target->collInfo.precise->worldRectPos;
            x0 = y1 = REALlyBig;
            x1 = y0 = REALlyNegative;
            for (pointIndex = 0; pointIndex < 8; pointIndex++)
            {
                x0 = min(x0, point[pointIndex].x);
                y0 = max(y0, point[pointIndex].y);
                x1 = max(x1, point[pointIndex].x);
                y1 = min(y1, point[pointIndex].y);
            }
            rect.x0 = primGLToScreenX(x0);
            rect.y0 = primGLToScreenY(y0);
            rect.x1 = primGLToScreenX(x1);
            rect.y1 = primGLToScreenY(y1);
        }
        else
        {
            if (target->objtype == OBJ_ShipType)
            {
                radius = max(
                    target->collInfo.selCircleRadius * ((Ship *)target)->staticinfo->tacticalSelectionScale,
                    selMinSelectionRadius);
            }
            else
            {
                radius = max(target->collInfo.selCircleRadius, selMinSelectionRadius);
            }
            rect.x0 = primGLToScreenX(target->collInfo.selCircleX - radius);
            rect.x1 = primGLToScreenX(target->collInfo.selCircleX + radius);
            rect.y0 = primGLToScreenY(target->collInfo.selCircleY + radius);
            rect.y1 = primGLToScreenY(target->collInfo.selCircleY - radius);
        }

        primGLRectOutline2(&rect, 1, selBandboxPreviewColor);
    }

    return attackcom.numTargets;
}

static real32 selStatusPulseStartTime = 0.0f;

bool32 selStatusDrawSpecialStatePulse(Ship *ship, sdword lod)
{
    sdword rect[4];
    sdword radius;
    sdword centerX;
    sdword centerY;
    real32 phase;
    real32 blendFactor;
    color pulseColor;

    if (!univIsObjFriendly((SpaceObj *)ship, universe.curPlayerPtr))
    {
        return FALSE;
    }
    if (!shipHasAvailableUpgradeChanges(ship))
    {
        return FALSE;
    }

    selStatusCircleComputeRect(ship, lod, rect);
    radius = (rect[3] - rect[1]) / 2;
    centerX = rect[2] + radius;
    centerY = rect[1] + radius;

    if (selStatusPulseStartTime + 0.5f < taskTimeElapsed || selStatusPulseStartTime > taskTimeElapsed)
    {
        selStatusPulseStartTime = taskTimeElapsed;
    }

    phase = (taskTimeElapsed - selStatusPulseStartTime) / 0.5f;
    if (phase > 0.5f)
    {
        blendFactor = (phase - 1.0f) * -2.0f;
    }
    else
    {
        blendFactor = phase + phase;
    }
    if (blendFactor <= 0.5f)
    {
        blendFactor += blendFactor;
    }
    else
    {
        blendFactor = 1.0f;
    }

    pulseColor = selStatusPulseBlendColor(colRGB(255, 0, 0), colWhite, blendFactor);
    primCircleSolid2(centerX, centerY, radius, 20, pulseColor);
    return TRUE;
}

void selStatusDrawEffectIcon(Ship *ship, sdword lod)
{
    sdword rect[4];
    sdword iconSize;

    /* Retail gates this X icon on a Cataclysm runtime flag at ship+0x210. */
    if (ship->specialFlags3 == 0 ||
        !univIsObjFriendly((SpaceObj *)ship, universe.curPlayerPtr))
    {
        return;
    }

    selStatusCircleComputeRect(ship, lod, rect);
    iconSize = 3 * (rect[3] - rect[1]);
    opOverlayIconDraw(OPI_X, rect[1], rect[0] - iconSize - 1, iconSize, iconSize, colWhite);
}

sdword selStatusDrawCounterPips(Ship *ship, sdword lod)
{
    sdword rect[4];
    sdword pipSize;
    sdword left;
    sdword top;
    sdword i;

    selStatusCircleComputeRect(ship, lod, rect);
    left = rect[0];
    pipSize = 2 * (rect[3] - rect[1]);
    top = rect[1] - pipSize;
    for (i = 0; i < ship->experienceLevel; i++)
    {
        opOverlayIconDraw(OPI_Exp_Star, top, left, pipSize, pipSize, colWhite);
        left += pipSize;
    }
    return ship->experienceLevel;
}

void selStatusDrawBadgeText(Ship *ship, sdword lod)
{
    sdword rect[4];
    sdword groupIndex;

    if (!selAnyHotKeyTest(ship))
    {
        return;
    }

    groupIndex = selHotKeyGroupNumberTest(ship);
    if (groupIndex < 0 || groupIndex >= SEL_NumberHotKeyGroups)
    {
        return;
    }

    selStatusCircleComputeRect(ship, lod, rect);
    selStatusSetFont(lod);
    fontPrint(rect[2], rect[1] - fontHeight(" "), selHotKeyNumberColor, selHotKeyString[groupIndex]);
}

static sdword selStatusMissileCount(Ship *ship)
{
    GunInfo *gunInfo = ship->cataclysmGunInfo;
    sdword missileCount = -1;
    sdword i;

    for (i = 0; i < gunInfo->numGuns; i++)
    {
        GunStatic *gunStatic = &((ShipStaticInfo *)ship->staticinfo)->gunStaticInfo->gunstatics[i];

        if (gunStatic->guntype == GUN_MissileLauncher ||
            gunStatic->guntype == GUN_MineLauncher)
        {
            missileCount += gunInfo->guns[i].numMissiles;
        }
    }

    return missileCount;
}

void selStatusDrawAuxText(Ship *ship, sdword lod, sdword row)
{
    sdword rect[4];
    sdword y;
    sdword auxValue;
    ShipsInsideMe *shipsInsideMe;

    if (lod > 2)
    {
        return;
    }

    selStatusCircleComputeRect(ship, lod, rect);
    y = rect[3] + (row - 1) * (rect[3] - rect[1]);
    fontMakeCurrent(selGroupFont3);

    if (ship->shiptype == sDreadnought ||
        ship->shiptype == sDestroyer ||
        ship->shiptype == tMissileDestroyer ||
        ship->shiptype == tMinelayerCorvette ||
        acolyteCanLink(ship))
    {
        auxValue = -1;
        if (ship->shiptype == sDreadnought ||
            ship->shiptype == sDestroyer ||
            ship->shiptype == tMissileDestroyer ||
            ship->shiptype == tMinelayerCorvette)
        {
            auxValue = selStatusMissileCount(ship);
        }
        if (acolyteCanLink(ship))
        {
            auxValue = acolyteGetLinkState(ship);
            if (auxValue == 0)
            {
                auxValue = -1;
            }
        }
        if (auxValue != -1)
        {
            fontPrintf(rect[0], y, selHotKeyNumberColor,
                       strGetString(strMissilesLeft), auxValue);
        }
        return;
    }

    shipsInsideMe = ship->shipsInsideMe;
    if (shipsInsideMe != NULL && !ShipTypeIsDefenseFieldFrigateType(ship->shiptype))
    {
        fontPrintf(rect[0], y, selHotKeyNumberColor,
                   strGetString(strFightersCorvettesDocked),
                   shipsInsideMe->FightersInsideme,
                   shipsInsideMe->CorvettesInsideme);
    }
}

sdword selStatusDrawPlayerSlotDots(Ship *ship, sdword lod)
{
    sdword rect[4];
    sdword radius;
    sdword left;
    sdword centerY;
    sdword playerIndex;
    Player *player;
    color dotColor;

    if ((ship->staticinfo->shipFlags & 0x20000) == 0 ||
        !univIsObjFriendly((SpaceObj *)ship, universe.curPlayerPtr))
    {
        return 0;
    }

    selStatusCircleComputeRect(ship, lod, rect);
    radius = (rect[3] - rect[1]) / 2;
    left = rect[0] + radius;
    centerY = rect[3] + radius + 2;
    for (playerIndex = 0, player = universe.players;
         playerIndex < universe.numPlayers;
         playerIndex++, player++)
    {
        if (allianceArePlayersAllied(ship->playerowner, player) || !shipGetSlotStatus(ship, playerIndex))
        {
            dotColor = colRGB(0, 255, 0);
        }
        else
        {
            dotColor = colRGB(255, 0, 0);
        }
        primCircleSolid2(left, centerY, radius, 20, dotColor);
        left += radius * 2;
    }
    return radius * 2;
}

sdword selSelectionCopyBounded(MaxSelection *dest, MaxSelection *source)
{
    sdword index;

    for (index = 0; index < source->numShips && index < 255; index++)
    {
        dest->ShipPtr[index] = source->ShipPtr[index];
    }
    dest->numShips = source->numShips;

    return source->numShips;
}

void selSelectedHotKeyNumbersDraw(void)
{
    sdword groupIndex;
    sdword xPositions[SEL_NumberSelections];
    sdword yPositions[SEL_NumberSelections];

    fontMakeCurrent(selGroupFont3);
    for (groupIndex = 0; groupIndex < SEL_NumberHotKeyGroups; groupIndex++)
    {
        MaxSelection *group = &selHotKeyGroup[groupIndex];
        const char *label = selHotKeyString[groupIndex];
        sdword labelMargin = 5 * fontWidth(label);
        sdword drawnCount = 0;
        sdword shipIndex;

        for (shipIndex = 0; shipIndex < group->numShips; shipIndex++)
        {
            Ship *ship = group->ShipPtr[shipIndex];
            sdword x;
            sdword y;
            sdword radiusX;
            sdword radiusY;
            sdword compareIndex;
            bool32 canDraw;

            if (!selShipInSelection(selSelected.ShipPtr, selSelected.numShips, ship))
            {
                continue;
            }

            x = primGLToScreenX(ship->collInfo.selCircleX);
            y = primGLToScreenY(ship->collInfo.selCircleY);
            radiusX = primGLToScreenScaleX(max(ship->collInfo.selCircleRadius, selMinSelectionRadius));
            radiusY = primGLToScreenScaleY(max(ship->collInfo.selCircleRadius, selMinSelectionRadius));
            canDraw = TRUE;

            for (compareIndex = 0; compareIndex < drawnCount; compareIndex++)
            {
                if (abs(xPositions[compareIndex] - x) < radiusX + labelMargin &&
                    abs(yPositions[compareIndex] - y) < radiusY + labelMargin)
                {
                    canDraw = FALSE;
                    break;
                }
            }

            if (canDraw)
            {
                xPositions[drawnCount] = x;
                yPositions[drawnCount] = y;
                fontPrint(x, y - fontHeight(" "), selHotKeyNumberColor, label);
                drawnCount++;
            }
        }
    }
}
