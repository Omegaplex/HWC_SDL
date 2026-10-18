/*=============================================================================
    Name    : Ping.c
    Purpose : Code to update and draw sensors manager pings.

    Created 9/8/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <math.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "debug.h"
#include "task.h"
#include "prim2d.h"
#include "spaceobj.h"
#include "linkedlist.h"
#include "universe.h"
#include "camera.h"
#include "pieplate.h"
#include "select.h"
#include "blobs.h"
#include "strings.h"
#include "tactical.h"
#include "teams.h"
#include "ProximitySensor.h"
#include "sensors.h"
#include "alliance.h"
#include "battle.h"
#include "SaveGame.h"
#include "soundevent.h"
#include "cameraCommand.h"
#include "ping.h"
#include "statscript.h"

/*=============================================================================
    Data:
=============================================================================*/
taskhandle pingTaskHandle;                      //ping evaluation task
LinkedList pingList;                            //list of pings

SpaceObj *pingDyingObject;                      //object that is dying and must be evaluated

extern bool8 smZoomingIn;
extern bool8 smZoomingOut;

//battle-blob stuff
BlobProperties pingBattleBlobProperties;

//anomaly ping tweakables
color  pingAnomalyColor          = PING_AnomalyColor;
real32 pingAnomalySize           = PING_AnomalySize;
real32 pingAnomalyMinSize        = PING_AnomalyMinSize;
real32 pingAnomalyPingDuration   = PING_AnomalyPingDuration;
real32 pingAnomalyInterPingPause = PING_AnomalyInterPingPause;
real32 pingAnomalyEvaluatePeriod = PING_AnomalyEvaluatePeriod;
real32 pingAnomalyMinScreenSize  = PING_AnomalyMinScreenSize;

//battle ping tweakables
color  pingBattleColor           = PING_BattleColor;
//real32 pingBattleSize            = PING_BattleSize;
real32 pingBattleMinSize         = PING_BattleMinSize;
real32 pingBattlePingDuration    = PING_BattlePingDuration;
real32 pingBattleInterPingPause  = PING_BattleInterPingPause;
real32 pingBattleEvaluatePeriod  = PING_BattleEvaluatePeriod;
real32 pingBattleMinScreenSize   = PING_BattleMinScreenSize;

//NewShip ping tweakables
color  pingNewShipColor           = PING_NewShipColor;
real32 pingNewShipSize            = PING_NewShipSize;
real32 pingNewShipMinSize         = PING_NewShipMinSize;
real32 pingNewShipPingDuration    = PING_NewShipPingDuration;
real32 pingNewShipInterPingPause  = PING_NewShipInterPingPause;
real32 pingNewShipEvaluatePeriod  = PING_NewShipEvaluatePeriod;
real32 pingNewShipMinScreenSize   = PING_NewShipMinScreenSize;

//misc tweakables
sdword pingTOMarginX              = PING_TOMarginX;
sdword pingTONSegments            = PING_TONSegments;
real32 pingTOLingerTime           = PING_TOLingerTime;

scriptEntry pingTweaks[] =
{
    { "AnomalyColor",           scriptSetRGBCB,    &pingAnomalyColor },
    { "AnomalySize",            scriptSetReal32CB, &pingAnomalySize },
    { "AnomalyMinSize",         scriptSetReal32CB, &pingAnomalyMinSize },
    { "AnomalyPingDuration",    scriptSetReal32CB, &pingAnomalyPingDuration },
    { "AnomalyInterPingPause",  scriptSetReal32CB, &pingAnomalyInterPingPause },
    { "AnomalyEvaluatePeriod",  scriptSetReal32CB, &pingAnomalyEvaluatePeriod },
    { "AnomalyMinScreenSize",   scriptSetReal32CB, &pingAnomalyMinScreenSize },

    { "BattleColor",            scriptSetRGBCB,    &pingBattleColor },
//    { "BattleSize",             scriptSetReal32CB, &pingBattleSize },
    { "BattleMinSize",          scriptSetReal32CB, &pingBattleMinSize },
    { "BattlePingDuration",     scriptSetReal32CB, &pingBattlePingDuration },
    { "BattleInterPingPause",   scriptSetReal32CB, &pingBattleInterPingPause },
    { "BattleEvaluatePeriod",   scriptSetReal32CB, &pingBattleEvaluatePeriod },
    { "BattleMinScreenSize",    scriptSetReal32CB, &pingBattleMinScreenSize },

    { "NewShipColor",           scriptSetRGBCB,    &pingNewShipColor },
    { "NewShipSize",            scriptSetReal32CB, &pingNewShipSize },
    { "NewShipMinSize",         scriptSetReal32CB, &pingNewShipMinSize },
    { "NewShipPingDuration",    scriptSetReal32CB, &pingNewShipPingDuration },
    { "NewShipInterPingPause",  scriptSetReal32CB, &pingNewShipInterPingPause },
    { "NewShipEvaluatePeriod",  scriptSetReal32CB, &pingNewShipEvaluatePeriod },
    { "NewShipMinScreenSize",   scriptSetReal32CB, &pingNewShipMinScreenSize },

    { "pingBobDensityLow",      scriptSetReal32CB, &pingBattleBlobProperties.bobDensityLow },
    { "pingBobDensityHigh",     scriptSetReal32CB, &pingBattleBlobProperties.bobDensityHigh },
    { "pingBobStartSphereSize", scriptSetReal32CB, &pingBattleBlobProperties.bobStartSphereSize },
    { "pingBobRadiusCombineMargin", scriptSetReal32CB, &pingBattleBlobProperties.bobRadiusCombineMargin },
    { "pingBobOverlapFactor",   scriptSetBlobPropertyOverlap, &pingBattleBlobProperties },
    { "bobSmallestRadius",      scriptSetReal32CB, &pingBattleBlobProperties.bobSmallestRadius },
    { "pingBobBiggestRadius",   scriptSetBlobBiggestRadius, &pingBattleBlobProperties },
//    { "pingBobDoingCollisionBobs", scriptSetBool, &pingBattleBlobProperties.bobDoingCollisionBobs },

    { "pingTOLingerTime",       scriptSetReal32CB, &pingTOLingerTime },
    endEntry
};

//TO stuff
struct
{
    strGamesMessages stringEnum;
    color *c;
    udword bitMask;
    real32 lastTimeDrawn;
}
pingTOList[PTO_NumberTOs] =
{
    {strPingTO0, &ProximitySensorBlipColor, PTOM_Proximity},
    {strPingTO1, &pingNewShipColor, PTOM_NewShips},
    {strPingTO2, &pingAnomalyColor, PTOM_Anomaly},
    {strPingTO3, &pingBattleColor, PTOM_Battle},
    {strPingTO4, &TW_HW_PING_COLOUR_OUT, PTOM_Hyperspace}
};

// pingAnomalyPingTimeout is defined in ping_save.c (extracted in Cataclysm)
extern bool32 pingAnomalyPingTimeout(struct ping *hellaPing, udword userID, char *userData, bool32 bRemoveReferences);

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : pingUpdateTask
    Description : Task to update and evaluate ping tasks
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#pragma optimize("gy", off)                       //turn on stack frame (we need ebp for this function)
void pingUpdateTask(void)
{
    static Node *thisNode, *nextNode;
    static ping *thisPing;

    taskYield(0);
    while (1)
    {
        taskStackSaveCond(0);

        thisNode = pingList.head;

        //... code to go here...

        while (thisNode != NULL)
        {                                                   //scan all pings
            nextNode = thisNode->next;
            thisPing = listGetStructOfNode(thisNode);
            if (universe.totaltimeelapsed - thisPing->lastEvaluateTime >= thisPing->evaluatePeriod)
            {                                               //if time to evaluate a ping
                thisPing->lastEvaluateTime = universe.totaltimeelapsed;
                if (thisPing->evaluate(thisPing, thisPing->userID, (char *)(thisPing + 1), FALSE))
                {                                           //did this ping just expire?
                    listDeleteNode(thisNode);               //kill it if so
                }
            }

            thisNode = nextNode;
        }
        taskStackRestoreCond();
        taskYield(0);
    }
}
#pragma optimize("", on)

/*-----------------------------------------------------------------------------
    Name        : pingStartup
    Description : Startup the ping module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingStartup(void)
{
    scriptSet(NULL, "ping.script", pingTweaks);
    pingAnomalyMinScreenSize = primScreenToGLScaleX(pingAnomalyMinScreenSize);
    pingNewShipMinScreenSize = primScreenToGLScaleX(pingNewShipMinScreenSize);
    pingBattleMinScreenSize = primScreenToGLScaleX(pingBattleMinScreenSize);
    pingTaskHandle = taskStart(pingUpdateTask, PNG_TaskPeriod, 0);
    listInit(&pingList);
    pingReset();
}

/*-----------------------------------------------------------------------------
    Name        : pingReset
    Description : Reset the ping module.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingReset(void)
{
    sdword index;

    for (index = 0; index < PTO_NumberTOs; index++)
    {                                                       //no ping legends yet
        pingTOList[index].lastTimeDrawn = REALlyNegative;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingShutdown
    Description : Shut down the ping module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingShutdown(void)
{
    taskStop(pingTaskHandle);
}

/*-----------------------------------------------------------------------------
    Name        : pingCreate
    Description : Create a ping
    Inputs      : loc - location to create the ping at.  Ping will not move
                  owner - optional object about which to postion a ping.
                        Ping will move with the object.  May be NULL.
                  evaluate - function to evaluate the ping.
                  userDataSize - extra memory to allocate for user data, such
                        as a ship list.
                  userID - number to pass back to evaluation function.
    Outputs     : userData - pointer to extra memory as allocated with the
                    ping structure.
    Return      : Pointer to newly allocated ping structure.
----------------------------------------------------------------------------*/
ping *pingCreate(vector *loc, SpaceObj *owner, pingeval evaluate, ubyte **userData, sdword userDataSize, udword userID)
{
    ping *newPing;

    newPing = memAlloc(sizeof(ping) + userDataSize, "Ping!", Pyrophoric);

    listAddNode(&pingList, &newPing->link, newPing);
    newPing->c = colWhite;
    newPing->size = 1.0f;
    newPing->minScreenSize = 10;
    newPing->minSize = 0.0f;
    if (owner == NULL)
    {
        newPing->centre = *loc;
    }
    else
    {
        newPing->centre = owner->posinfo.position;
    }
    newPing->owner = owner;
    newPing->creationTime = universe.totaltimeelapsed;
    newPing->pingDuration = 1.0f;
    newPing->interPingPause = 0.0f;
    newPing->evaluatePeriod = PNG_TaskPeriod;
    newPing->lastEvaluateTime = universe.totaltimeelapsed;
    newPing->evaluate = evaluate;
    newPing->userDataSize = userDataSize;
    newPing->userID = userID;
    newPing->TOMask = 0;
    if (userData != NULL)
    {
        *userData = (ubyte *)(newPing + 1);
    }
    return(newPing);
}

/*-----------------------------------------------------------------------------
    Name        : pingObjectDied
    Description : Called when an object dies, removes references of said object
                    from all ping structures.
    Inputs      : obj - object that just died
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingObjectDied(SpaceObj *obj)
{
    Node *thisNode, *nextNode;
    ping *thisPing;

    pingDyingObject = obj;

    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);
        if (thisPing->owner == obj)
        {                                                   //if this ping centered on object
            thisPing->centre = obj->posinfo.position;
            thisPing->owner = NULL;                         //remove object from reference
        }
        if (thisPing->userID != 0 || thisPing->userDataSize != 0)
        {                                                   //see if there's anything in the user data
            if (thisPing->evaluate(thisPing, thisPing->userID, (char *)(thisPing + 1), TRUE))
            {                                               //did this ping just expire?
                listDeleteNode(thisNode);                   //kill it if it did
            }
        }

        thisNode = nextNode;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingFindByFunction
    Description : Find a ping in the ping list by function
    Inputs      : evalueate - the evaluation function of the ping to find
    Outputs     :
    Return      : First ping in list with that evaluate function or NULL if none
----------------------------------------------------------------------------*/
ping *pingFindByFunction(pingeval evaluate)
{
    Node *thisNode;
    ping *thisPing;

    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        thisPing = listGetStructOfNode(thisNode);

        if (thisPing->evaluate == evaluate)
        {
            return(thisPing);
        }
        thisNode = thisNode->next;;
    }
    return(NULL);
}

/*-----------------------------------------------------------------------------
    Name        : pingListSortCallback
    Description : Merge sort callback for sorting the ping list by distance
                    from the camera.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
bool32 pingListSortCallback(void *firststruct,void *secondstruct)
{
    if (((ping *)(firststruct))->cameraDistanceSquared > ((ping *)(secondstruct))->cameraDistanceSquared)
    {
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingListDraw
    Description : Draw all pings from farthest to nearest.
    Inputs      : camera - the camera we're rendering from
                  modelView, projection - current matrices
                  viewPort - rectangle we're rending in, for the TO legend
    Outputs     :
    Return      :
    Note        : The renderer should be in 2D mode at this point.
----------------------------------------------------------------------------*/
void pingListDraw(Camera *camera, hmatrix *modelView, hmatrix *projection, rectangle *viewPort)
{
    real32 pingAge, pingCycle, pingMod, pingSize;
    real32 x, y, radius;
    Node *thisNode, *nextNode;
    ping *thisPing;
    vector distSquared;
    sdword nSegments, index, rowHeight, xScreen, yScreen;
    oval o;
    udword TOFlags = 0;
    fonthandle fhSave;
    toicon *icon;
    color col;
    real32 realMargin;
    ShipClass shipClass;
    static real32 lastProximityPing = REALlyBig;
    static real32 lastAnomolyPing = REALlyBig;
    static real32 lastBattlePing = REALlyBig;
    static real32 lastHyperspacePing = REALlyBig;
    static real32 lastNewshipPing = REALlyBig;
    bool32 pingset;

    //start by sorting the ping list from farthest to nearest
    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);

        if (thisPing->owner != NULL)
        {
            thisPing->centre = thisPing->owner->posinfo.position;
        }
        vecSub(distSquared, camera->eyeposition, thisPing->centre);
        thisPing->cameraDistanceSquared = vecMagnitudeSquared(distSquared);
        TOFlags |= thisPing->TOMask;

        thisNode = nextNode;
    }
    listMergeSortGeneral(&pingList, pingListSortCallback);

    //now the list is sorted; proceed to draw all the pings
    thisNode = pingList.head;

    pingset = FALSE;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);

        pingCycle = thisPing->pingDuration + thisPing->interPingPause;
        pingAge = universe.totaltimeelapsed - thisPing->creationTime;
        pingMod = (real32)fmod((double)pingAge, (double)pingCycle);
        if (pingMod <= thisPing->pingDuration)
        {
            bool32 shouldDrawPing;

            pingSize = (thisPing->size - thisPing->minSize) * pingMod / thisPing->pingDuration + thisPing->minSize;
            selCircleComputeGeneral(modelView, projection, &thisPing->centre, max(thisPing->size,thisPing->minSize), &x, &y, &radius);
            shouldDrawPing = (thisPing->TOMask == PTOM_Hyperspace || !fogCheckPositionVisible(&thisPing->centre, FALSE));
            if (radius > 0.0f && shouldDrawPing)
            {
                radius = max(radius, thisPing->minScreenSize);
                radius *= pingSize / max(thisPing->size,thisPing->minSize);
                o.centreX = primGLToScreenX(x);
                o.centreY = primGLToScreenY(y);
                o.radiusX = o.radiusY = primGLToScreenScaleX(radius);
                nSegments = pieCircleSegmentsCompute(radius);
                primOvalArcOutline2(&o, 0.0f, 2*PI, 1, nSegments, thisPing->c);

                /* starting to draw a new ping so play the sound */
                if (!smZoomingIn && !smZoomingOut && !pingset)
                {
                    switch (thisPing->TOMask)
                    {
                        case PTOM_Anomaly:
                            if (pingSize <=lastAnomolyPing)
                            {
                                soundEvent(NULL, UI_SensorsPing);
                                pingset = TRUE;
                                lastAnomolyPing = pingSize;
                            }
                            break;
                        case PTOM_Battle:
                            if (pingSize <= lastBattlePing)
                            {
                                soundEvent(NULL, UI_PingBattle);
                                pingset = TRUE;
                                lastBattlePing = pingSize;
                            }
                            break;
                        case PTOM_Hyperspace:
                            if (pingSize <=  lastHyperspacePing)
                            {
                                soundEvent(NULL, UI_PingHyperspace);
                                pingset = TRUE;
                                lastHyperspacePing = pingSize;
                            }
                            break;
                        case PTOM_Proximity:
                            if (pingSize <= lastProximityPing)
                            {
                                soundEvent(NULL, UI_PingProximity);
                                pingset = TRUE;
                                lastProximityPing = pingSize;
                            }
                            break;
                        case PTOM_NewShips:
                            if (pingSize <= lastNewshipPing)
                            {
                                soundEvent(NULL, UI_PingNewShips);
                                pingset = TRUE;
                                lastNewshipPing = pingSize;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        thisNode = nextNode;
    }

    //draw the blip TO
    if (smTacticalOverlay)
    {
        realMargin = primScreenToGLScaleX(viewPort->x0);
        fhSave = fontCurrentGet();                          //save the current font
        fontMakeCurrent(selGroupFont2);                     // use a common, fairly small font
        rowHeight = fontHeight("M");                        // used to space the legend
        yScreen = viewPort->y0 + rowHeight;                 //leave some space at the top to start
        radius = primScreenToGLScaleX(rowHeight)/2;
        xScreen = viewPort->x0 + (sdword)(rowHeight * 2.5);

        for (index = 0; index < PTO_NumberTOs; index++)
        {
            if ((TOFlags & pingTOList[index].bitMask))
            {
    //            fontPrint(xScreen, yScreen, *pingTOList[index].c, "O");
                pingTOList[index].lastTimeDrawn = universe.totaltimeelapsed;
            }
            if (universe.totaltimeelapsed - pingTOList[index].lastTimeDrawn <= pingTOLingerTime)
            {
                o.centreX = viewPort->x0 + rowHeight * 3 / 2;
                o.centreY = yScreen + rowHeight / 2;
                o.radiusX = o.radiusY = rowHeight / 2;
                primOvalArcOutline2(&o, 0.0f, TWOPI, 1, pingTONSegments, *pingTOList[index].c);
                fontPrint(xScreen, yScreen, TO_TextColor, strGetString(pingTOList[index].stringEnum));
                yScreen += rowHeight + 1;
            }
        }
        for (shipClass = 0; shipClass < NUM_CLASSES; shipClass++)
        {
            if (!toClassUsed[shipClass][0])
            {
                continue;
            }
            icon = toClassIcon[shipClass];
            fontPrint(xScreen, yScreen + (rowHeight>>2), TO_TextColor, ShipClassToNiceStr(shipClass));
#if TO_STANDARD_COLORS
            col = teFriendlyColor;
#else
            col = teColorSchemes[universe.curPlayerIndex].tacticalColor;
#endif
            col = colRGB(colRed(col)/TO_IconColorFade, colGreen(col)/TO_IconColorFade, colBlue(col)/TO_IconColorFade);
            primLineLoopStart2(1, col);

            for (index = icon->nPoints - 1; index >= 0; index--)
            {
               primLineLoopPoint3F(realMargin + primScreenToGLX(rowHeight*1.5) + icon->loc[index].x * radius,
                                          primScreenToGLY(yScreen + rowHeight/2) + icon->loc[index].y * radius);
            }
            primLineLoopEnd2();
            yScreen += rowHeight + 1;
        }

        fontMakeCurrent(fhSave);
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalySelectionPingAdd
    Description : Create an anomaly ping around a whole bunch of ships
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingAnomalySelectionPingAdd(char *pingName, SelectCommand *selection)
{
    ubyte *userData;
    ping *newPing;
    sdword dataSize;
    vector position;
    real32 dummySize;

    position = selCentrePointComputeGeneral((MaxSelection *)selection, &dummySize);
    dataSize = strlen(pingName) + 1 + selSelectionSize(selection->numShips);
    newPing = pingCreate(&position, NULL, pingAnomalyPingTimeout, &userData, dataSize, 0);
    selSelectionCopy((MaxAnySelection *)userData, (MaxAnySelection *)selection);//copy the ship selection to the ping
    strcpy((char *)userData + selSelectionSize(selection->numShips), pingName);//set the name
    newPing->c              = pingAnomalyColor;
    newPing->size           = pingAnomalySize;
    newPing->minSize        = pingAnomalyMinSize;
    newPing->pingDuration   = pingAnomalyPingDuration;
    newPing->interPingPause = pingAnomalyInterPingPause;
    newPing->evaluatePeriod = pingAnomalyEvaluatePeriod;
    newPing->TOMask         = PTOM_Anomaly;
    newPing->minScreenSize  = pingAnomalyMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalyObjectPingAdd
    Description : Create an anomaly ping about an object
    Inputs      : pingName - name of a ping
                  owner - object about which the ping in centred
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingAnomalyObjectPingAdd(char *pingName, SpaceObj *owner)
{
    ubyte *userData;
    ping *newPing;
    sdword dataSize, zero = 0;

    dataSize = strlen(pingName) + 1 + selSelectionSize(zero);
    newPing = pingCreate(NULL, owner, pingAnomalyPingTimeout, &userData, dataSize, (udword)owner);
    ((SelectCommand *)userData)->numShips = 0;              //dummy selection
    strcpy((char *)userData + selSelectionSize(zero), pingName);       //set the name
    newPing->c              = pingAnomalyColor;
    newPing->size           = pingAnomalySize;
    newPing->minSize        = pingAnomalyMinSize;
    newPing->pingDuration   = pingAnomalyPingDuration;
    newPing->interPingPause = pingAnomalyInterPingPause;
    newPing->evaluatePeriod = pingAnomalyEvaluatePeriod;
    newPing->TOMask         = PTOM_Anomaly;
    newPing->minScreenSize  = pingAnomalyMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingAnolalyPositionPingAdd
    Description : Create an anomaly ping about an arbitrary position
    Inputs      : pingName - name of the ping
                  position - location of the ping
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingAnomalyPositionPingAdd(char *pingName, vector *position)
{
    ubyte *userData;
    ping *newPing;
    sdword dataSize, zero = 0;

    dataSize = strlen(pingName) + 1 + selSelectionSize(zero);
    newPing = pingCreate(position, NULL, pingAnomalyPingTimeout, &userData, dataSize, 0);
    ((SelectCommand *)userData)->numShips = 0;              //dummy selection
    strcpy((char *)userData + selSelectionSize(zero), pingName);       //set the name
    newPing->c              = pingAnomalyColor;
    newPing->size           = pingAnomalySize;
    newPing->minSize        = pingAnomalyMinSize;
    newPing->pingDuration   = pingAnomalyPingDuration;
    newPing->interPingPause = pingAnomalyInterPingPause;
    newPing->evaluatePeriod = pingAnomalyEvaluatePeriod;
    newPing->TOMask         = PTOM_Anomaly;
    newPing->minScreenSize  = pingAnomalyMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingAnomalyPingRemove
    Description : Remove all pings of a given name from the ping list
    Inputs      :
    Outputs     :
    Return      : number of pings removed
----------------------------------------------------------------------------*/
sdword pingAnomalyPingRemove(char *pingName)
{
    Node *thisNode, *nextNode;
    ping *thisPing;
    char *name;
    SelectCommand *selection;
    sdword nRemoved = 0;

    thisNode = pingList.head;

    while (thisNode != NULL)
    {                                                       //scan all pings
        nextNode = thisNode->next;
        thisPing = listGetStructOfNode(thisNode);
        if (thisPing->evaluate == pingAnomalyPingTimeout)
        {
            selection = (SelectCommand *)(thisPing + 1);
            name = (char *)selection + selSelectionSize(selection->numShips );
            if (!strcmp(pingName, name))
            {                                               //if names match
                listDeleteNode(thisNode);                   //kill it if it did
                nRemoved++;                                 //inc number removed
            }
        }

        thisNode = nextNode;
    }
    return(nRemoved);
}

/*-----------------------------------------------------------------------------
    Name        : pingRemovePingByOwner
    Description : removes a ping based on its owner
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingRemovePingByOwner(SpaceObj *owner)
{
    Node *node = pingList.head;
    ping *thisPing;

    while (node != NULL)
    {
        thisPing = listGetStructOfNode(node);

        if (thisPing->owner == owner)
        {
            listDeleteNode(node);
            return;
        }

        node = node->next;
    }
}

/*-----------------------------------------------------------------------------
    Name        : pingAllPingsDelete
    Description : Deletes all pings.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingAllPingsDelete(void)
{
    listDeleteAll(&pingList);
}

/*-----------------------------------------------------------------------------
    Name        : pingBattleBlobCriteria
    Description : sub-blob creation callback to see if an object is a ship in a battle
    Inputs      : superBlob - what blob we're searching through
                  obj - what object we're currently processing
    Outputs     :
    Return      : TRUE if it's a player ship in a battle
----------------------------------------------------------------------------*/
bool32 pingBattleBlobCriteria(blob *superBlob, SpaceObj *obj)
{
    if (obj->objtype != OBJ_ShipType)
    {
        return(FALSE);                                      //not a ship - no good
    }
//    if (!allianceIsShipAlly((Ship *)obj, universe.curPlayerPtr))
//    {
        //!!! should we consider alliances here?
//        return(FALSE);                                      //does not belong to the player
//    }

    if (obj->flags & (SOF_Dead|SOF_Hide|SOF_Crazy|SOF_Disabled))
    {
        return(FALSE);
    }

    if ((((Ship *)obj)->recentlyAttacked && ((Ship *)obj)->recentAttacker->playerowner != universe.curPlayerPtr) ||
        (((Ship *)obj)->shipisattacking))
    {
        if (!bitTest(smShipTypeRenderFlags[((Ship *)obj)->shiptype], SM_Exclude))
        {                                                   //if we should include in battle blobs
            return(TRUE);                                   //it's a player ship in a battle - good!
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingBattlePingEvaluate
    Description : Called when a battle ping times out
    Inputs      : ignored
    Outputs     : Battle sound code will be performed here.
    Return      : TRUE, or maybe false, depending on the mood
----------------------------------------------------------------------------*/
bool32 pingBattlePingEvaluate(struct ping *hellaPing, udword userID, char *userData, bool32 bRemoveReferences)
{
    sdword index;
    Ship **thisShip;
    battleping *battlePing;

    battlePing = (battleping *)(hellaPing + 1);

    if (bRemoveReferences == FALSE)
    {                                                       //ping is being evaluated
        if (battlePing->nEnemyShips == 0)
        {                                                   //no enemies - we must have just won the battle
            if (battlePing->nFriendlyShips == 0)
            {                                               //no friendlies or enemies
                return(TRUE);                               //there's no battle going on anymore, delete this ping
            }
            else
            {
                battlePingEvaluateNoEnemies(hellaPing, battlePing);
            }
        }
        else if (battlePing->nFriendlyShips == 0)
        {                                                   // no friendlies - could be friendly fire
            battlePingEvaluateNoFriendlies(hellaPing, battlePing);
        }
        else
        {
            battlePingEvaluate(hellaPing, battlePing);      //evaluate the ping normally, maybe saying something
        }
        return(TRUE);                                       //time out on first evaluation
        //!!!! try a few times here.  This could be an efficiency issue
    }
                                                            //else an object just died
    if (pingDyingObject->objtype != OBJ_ShipType)
    {
        return(FALSE);                                      //only ships in battle pings
    }
    for (index = 0, thisShip = battlePing->ship; index < battlePing->nShips; index++, thisShip++)
    {
        if (*thisShip == (Ship *)pingDyingObject)
        {                                                   //if this is the object that is dying
            battleAccountForLosses(*thisShip, battlePing);  //account for this damage on computation of future BSN's
            if (allianceIsShipAlly(*thisShip, universe.curPlayerPtr))
            {                                               //we're losin' em here
                battlePing->nFriendlyShips--;
            }
            else
            {
                battlePing->nEnemyShips--;                  //we're beatin' em
            }
            battlePing->nShips--;                            //decrement total number of ships
            if (index < battlePing->nShips)
            {                                               //copy last entry to current entry (fill in this space)
                *thisShip = battlePing->ship[battlePing->nShips];
            }
            break;                                          //done searching for ship
        }
    }
    dbgAssert(battlePing->nFriendlyShips + battlePing->nEnemyShips == battlePing->nShips);
    dbgAssert(battlePing->nShips >= 0);
    if (battlePing->nShips == 0)
    {                                                       //all ships removed, kill the ping
        return(TRUE);
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : pingAttackPingsCreate
    Description : Create a bunch of attack pings in a given blob by creating
                    a list of sub-blobs.
    Inputs      : superBlob - blob to search for battles within.
    Outputs     :
    Return      :
    Note!!!     : no pings will be created if any battle pings are presently going on
----------------------------------------------------------------------------*/
void pingAttackPingsCreate(blob *superBlob)
{
    LinkedList battleBlobs;
    Node *thisNode;
    blob *thisBlob;
    battleping *battlePing;
    ping *newPing;
    static sdword nTotalShips;
    CommandToDo *command;
    Ship **ship, **commandShip, **baseShip;
    sdword index, commandIndex, baseIndex, startShips;

    bobSubBlobListCreate(&pingBattleBlobProperties, &battleBlobs, superBlob, pingBattleBlobCriteria);

    thisNode = battleBlobs.head;

    while (thisNode != NULL)
    {
        thisBlob = listGetStructOfNode(thisNode);

        //create the ping structure
        nTotalShips = superBlob->nCapitalShips + superBlob->nAttackShips + superBlob->nNonCombat + superBlob->nFriendlyShips;
        if (superBlob->blobShips->numShips > nTotalShips)
        {
            nTotalShips = superBlob->blobShips->numShips;
        }
        newPing = pingCreate(&thisBlob->centre, NULL, pingBattlePingEvaluate, (ubyte **)&battlePing, battlePingSize(nTotalShips), 0);
        newPing->c              = pingBattleColor;
//        newPing->size           = pingBattleSize;
        newPing->size           = thisBlob->radius;
        newPing->minSize        = pingBattleMinSize;
        newPing->pingDuration   = pingBattlePingDuration;
        newPing->interPingPause = pingBattleInterPingPause;
        newPing->evaluatePeriod = pingBattleEvaluatePeriod;
        newPing->TOMask         = PTOM_Battle;
        newPing->minScreenSize  = pingBattleMinScreenSize;

        //create the battleping structure
        battlePing->nEnemyShips = thisBlob->nCapitalShips + thisBlob->nAttackShips + thisBlob->nNonCombat;
        battlePing->nFriendlyShips = thisBlob->nFriendlyShips;
        battlePing->nShips = battlePing->nEnemyShips + battlePing->nFriendlyShips;
        battlePing->radius = thisBlob->radius;
        battlePing->friendlyFirepowerRecentlyLost =         //no recent losses just yet
            battlePing->friendlyValueRecentlyLost =
            battlePing->enemyFirepowerRecentlyLost =
            battlePing->enemyValueRecentlyLost = 0.0f;
        battlePing->nShips = thisBlob->blobObjects->numSpaceObjs;
        memcpy(battlePing->ship, thisBlob->blobObjects->SpaceObjPtr, battlePing->nShips * sizeof(SpaceObj *));

        //add all ships in the attack commands of all ships in the selection
        ship = battlePing->ship;
        startShips = battlePing->nShips;
        for (index = 0; index < battlePing->nShips; index++, ship++)
        {                                                   //for each ship in the ping
            dbgAssert((*ship)->objtype == OBJ_ShipType);
            dbgAssert((*ship)->playerowner != NULL);
            command = (*ship)->command;                     //get the ship's command

            if ((command != NULL) &&
                ((command->ordertype.order == COMMAND_ATTACK)))// ||
                 //(command->ordertype.attributes & COMMAND_IS_ATTACKINGANDMOVING|COMMAND_IS_PASSIVEATTACKING)))
            {                                               //if it's a valid attack command
                if (universe.totaltimeelapsed != command->pingUpdateTime || battlePing != command->updatedPing)
                {                                           //if this command has not already been added to this ping
                    commandShip = (Ship **)command->attack->TargetPtr;
                    for (commandIndex = command->attack->numTargets; commandIndex > 0; commandIndex--, commandShip++)
                    {                                       //for each ship in the attack command
                        if (!pingBattleBlobCriteria(NULL, (SpaceObj *)*commandShip))
                        {                                   //if it would be rejected by blob criteria
                            continue;                       //then we don't want it
                        }
                        //see if this ship is already in the ping.  We don't want doubles.
                        baseShip = battlePing->ship;
                        for (baseIndex = 0; baseIndex < battlePing->nShips; baseIndex++, baseShip++)
                        {
                            if (*baseShip == *commandShip)
                            {                               //is this ship already here?
                                goto shipAlreadyInSelection;
                            }
                        }
                        //not found in the selection yet
                        if (battlePing->nShips < nTotalShips)
                        {                                   //if there is room for this ship in the ping
                            battlePing->ship[battlePing->nShips] = *commandShip;
                            if (allianceIsShipAlly(*commandShip, universe.curPlayerPtr))
                            {                               //was that a friendly or an enemy?
                                battlePing->nFriendlyShips++;
                            }
                            else
                            {
                                battlePing->nEnemyShips++;
                            }
                            battlePing->nShips++;           //another ship
                        }
#if 0 && PNG_VERBOSE_LEVEL >= 2
                        else
                        {
                            dbgMessagef("\nShip 0x%x tossed out of ping with %d ships.", commandShip, nTotalShips);
                        }
#endif
shipAlreadyInSelection:;
                    }
                    command->pingUpdateTime = universe.totaltimeelapsed;
                    command->updatedPing = battlePing;      //flag this command as already processed
                }
            }
        }
        dbgAssert(battlePing->nEnemyShips + battlePing->nFriendlyShips == battlePing->nShips);
        thisNode = thisNode->next;
    }
    bobListDelete(&battleBlobs);
//    listDeleteAll(&battleBlobs);
}

/*-----------------------------------------------------------------------------
    Name        : pingNewShipCallback
    Description : Ping evalueation callback for new ship pings
    Inputs      : all ignored so I'm not going to bother explaining it
    Outputs     :
    Return      : TRUE because this function should only be called when the
                    ping times out
----------------------------------------------------------------------------*/
bool32 pingNewShipCallback(struct ping *hellaPing, udword userID, char *userData, bool32 bRemoveReferences)
{
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : pingNewShipPingCreate
    Description : Create a ping at the location of new ship creation.
    Inputs      : position - position of new ship
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void pingNewShipPingCreate(vector *position)
{
    ping *newPing;
    ubyte *userData;

    newPing = pingCreate(position, NULL, pingNewShipCallback, &userData, 0, 0);
    newPing->c              = pingNewShipColor;
    newPing->size           = pingNewShipSize;
    newPing->minSize        = pingNewShipMinSize;
    newPing->pingDuration   = pingNewShipPingDuration;
    newPing->interPingPause = pingNewShipInterPingPause;
    newPing->evaluatePeriod = pingNewShipEvaluatePeriod;
    newPing->TOMask         = PTOM_NewShips;
    newPing->minScreenSize  = pingNewShipMinScreenSize;
}

/*-----------------------------------------------------------------------------
    Name        : pingBattlePingsCreate
    Description : Create a list of attack pings from a blob list
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void pingBattlePingsCreate(LinkedList *blobList)
{
    Node *thisNode;
    blob *thisBlob;

    if (pingFindByFunction(pingBattlePingEvaluate) != NULL)
    {                                                       //if there are already battle pings active
        return;                                             //don't create any new ones
    }
    thisNode = blobList->head;

    while (thisNode != NULL)
    {
        thisBlob = listGetStructOfNode(thisNode);

        if (thisBlob->flags & BTM_PieceOfTheAction)
        {                                                   //if there's action in this blob
            pingAttackPingsCreate(thisBlob);
        }
        thisNode = thisNode->next;
    }
}

#if 0  // Cat2: pingWorldToGridCoords live code differs from binary or raw IDA offsets
// ============================================================================
// Decompiled from cataclysm.exe @ 0x47EFB0
void pingWorldToGridCoords(float worldX, float worldY, float worldZ, float radius)
{
    double invCellSize;
    int gridX;
    int gridY;
    int gridZ;
    int gridRadius;

    if (currentSensorCoverage)
    {
        dword_A3E2E8 = 0;
        invCellSize = ONE / *(float *)(currentSensorCoverage + 28);
        gridX = (int)((worldX - *(float *)(currentSensorCoverage + 16)) * invCellSize);
        gridY = (int)((worldY - *(float *)(currentSensorCoverage + 20)) * invCellSize);
        gridZ = (int)((worldZ - *(float *)(currentSensorCoverage + 24)) * invCellSize);
        gridRadius = (int)(invCellSize * radius) + 1;
        pingFillGridCells(gridX, gridY, gridZ, gridRadius);
    }
    else
    {
        ++dword_A3E2EC;
        dword_A3E2E8 = 1;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47EEB0
int pingFillGridCells(int gridX, int gridY, int gridZ, int gridRadius)
{
    int result;
    int maxX, minX;
    int maxY, minY;
    int maxZ, minZ;
    int clampedMaxX, clampedMaxY, clampedMaxZ;
    int ix, iy, iz;

    dword_A3E2E8 = 0;
    if (currentSensorCoverage)
    {
        maxY = gridY + gridRadius;
        minX = gridX - gridRadius;
        minY = gridY - gridRadius;
        maxZ = gridRadius + gridZ;
        maxX = gridRadius + gridX;
        minZ = gridZ - gridRadius;

        if (minX < 0)
            minX = 0;
        if (maxX >= *(_DWORD *)(currentSensorCoverage + 4))
            maxX = *(_DWORD *)(currentSensorCoverage + 4) - 1;
        if (minY < 0)
            minY = 0;
        clampedMaxY = maxY;
        if (maxY >= *(_DWORD *)(currentSensorCoverage + 8))
            clampedMaxY = *(_DWORD *)(currentSensorCoverage + 8) - 1;
        if (minZ < 0)
            minZ = 0;
        result = *(_DWORD *)(currentSensorCoverage + 12);
        clampedMaxZ = maxZ;
        if (maxZ >= result)
            clampedMaxZ = result - 1;

        for (iz = minZ; iz <= clampedMaxZ; ++iz)
        {
            for (iy = minY; iy <= clampedMaxY; ++iy)
            {
                for (ix = minX; ix <= maxX; ++ix)
                {
                    sub_47EDF0(ix, iy, iz);
                }
            }
            result = iz + 1;
        }
    }
    else
    {
        result = ++dword_A3E2EC;
        dword_A3E2E8 = 1;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47F6B0
int __cdecl pingComputeGridIndices(float worldX, float worldY, float worldZ, float radius, int *gridX, int *gridY, int *gridZ, int *gridRadius)
{
    __int64 v8;
    double invCellSize;

    LODWORD(v8) = currentSensorCoverage;
    dword_A3E2E8 = 0;
    if (currentSensorCoverage)
    {
        invCellSize = ONE / *(float *)(currentSensorCoverage + 28);
        if (gridX)
        {
            *gridX = (__int64)((worldX - *(float *)(currentSensorCoverage + 16)) * invCellSize);
            LODWORD(v8) = currentSensorCoverage;
        }
        if (gridY)
        {
            *gridY = (__int64)((worldY - *(float *)(v8 + 20)) * invCellSize);
            LODWORD(v8) = currentSensorCoverage;
        }
        if (gridZ)
        {
            v8 = (__int64)((worldZ - *(float *)(v8 + 24)) * invCellSize);
            *gridZ = v8;
        }
        if (gridRadius)
        {
            LODWORD(v8) = (__int64)(invCellSize * radius) + 1;
            *gridRadius = v8;
        }
    }
    else
    {
        ++dword_A3E2EC;
        dword_A3E2E8 = 1;
    }
    return v8;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47F070
int __cdecl pingDecrementGridCells(int centerX, int centerY, int centerZ, int radius)
{
    _DWORD *gridPtr;
    int result;
    int maxX;
    int maxY;
    int maxZ;
    int z;
    int minZ;
    int v12;
    int v13;
    int i;
    int *cellPtr;
    int minX, minY;

    gridPtr = (_DWORD *)currentSensorCoverage;
    dword_A3E2E8 = 0;
    if (currentSensorCoverage)
    {
        maxX = centerX + radius;
        maxY = radius + centerY;
        minX = centerX - radius;
        minY = centerY - radius;
        maxZ = radius + centerZ;
        minZ = centerZ - radius;
        // Clamp to grid bounds
        if (minX < 0)
            minX = 0;
        v12 = *(_DWORD *)(currentSensorCoverage + 4);
        if (maxX >= v12)
            maxX = v12 - 1;
        if (minY < 0)
            minY = 0;
        v13 = *(_DWORD *)(currentSensorCoverage + 8);
        if (maxY >= v13)
            maxY = v13 - 1;
        if (minZ < 0)
            minZ = 0;
        result = *(_DWORD *)(currentSensorCoverage + 12);
        if (maxZ >= result)
            maxZ = --result;
        for (; minZ <= maxZ; ++minZ)
        {
            if (minY <= maxY)
            {
                int y = minY;
                do
                {
                    for (i = minX; i <= maxX; ++i)
                    {
                        if (i >= 0 && i < gridPtr[1] && y >= 0 && y < gridPtr[2] && minZ >= 0 && minZ < gridPtr[3])
                        {
                            dword_A3E2E8 = 0;
                            if (gridPtr)
                            {
                                cellPtr = (int *)(*gridPtr + 32 * (i + gridPtr[1] * (y + gridPtr[2] * minZ)));
                                if (*cellPtr > 0)
                                {
                                    --*cellPtr;
                                    sub_47FB80(i, y, minZ, 1);
                                    gridPtr = (_DWORD *)currentSensorCoverage;
                                }
                            }
                            else
                            {
                                ++dword_A3E2EC;
                                dword_A3E2E8 = 1;
                            }
                        }
                    }
                    ++y;
                }
                while (y <= maxY);
            }
            result = maxZ;
        }
    }
    else
    {
        result = ++dword_A3E2EC;
        dword_A3E2E8 = 1;
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47F400
signed int __cdecl pingHasUnseenGridCells(int centerX, int centerY, int centerZ, int radius)
{
    int v6;
    int maxY;
    int v8;
    int maxZ;
    int minZ;
    int v11;
    int v12;
    int v13;
    int i;
    int y;
    int j;
    int minX, maxX;
    int minY;
    int result;

    result = 0;
    dword_A3E2E8 = 0;
    if (currentSensorCoverage)
    {
        v6 = centerX - radius;
        maxX = radius + centerX;
        minX = v6;
        maxY = centerY + radius;
        v8 = centerY - radius;
        maxZ = radius + centerZ;
        minZ = centerZ - radius;
        minY = centerY - radius;
        // Clamp to grid bounds
        if (minX < 0)
            minX = 0;
        v11 = *(_DWORD *)(currentSensorCoverage + 4);
        if (maxX >= v11)
            maxX = v11 - 1;
        if (v8 < 0)
        {
            v8 = 0;
            minY = 0;
        }
        v12 = *(_DWORD *)(currentSensorCoverage + 8);
        if (maxY >= v12)
        {
            maxY = v12 - 1;
        }
        if (minZ < 0)
            minZ = 0;
        v13 = *(_DWORD *)(currentSensorCoverage + 12);
        if (maxZ >= v13)
            maxZ = v13 - 1;
        for (i = minZ; i <= maxZ; ++i)
        {
            y = v8;
            if (v8 <= maxY)
            {
                do
                {
                    for (j = minX; j <= maxX; ++j)
                    {
                        if (j >= 0
                            && j < *(_DWORD *)(currentSensorCoverage + 4)
                            && y >= 0
                            && y < *(_DWORD *)(currentSensorCoverage + 8)
                            && i >= 0
                            && i < *(_DWORD *)(currentSensorCoverage + 12))
                        {
                            dword_A3E2E8 = 0;
                            if (!*(_DWORD *)(32 * (j + *(_DWORD *)(currentSensorCoverage + 4) * (y + *(_DWORD *)(currentSensorCoverage + 8) * i))
                                + *(_DWORD *)currentSensorCoverage + 4))
                                result = 1;
                        }
                    }
                    maxY = maxY;
                    ++y;
                }
                while (y <= maxY);
                v8 = minY;
            }
        }
        return result;
    }
    else
    {
        ++dword_A3E2EC;
        dword_A3E2E8 = 1;
        return 0;
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47F540
signed int __cdecl pingGetGridCellVisibility(int gridX, int gridY, int gridZ)
{
    if (gridX < 0
        || gridX >= *(_DWORD *)(currentSensorCoverage + 4)
        || gridY < 0
        || gridY >= *(_DWORD *)(currentSensorCoverage + 8)
        || gridZ < 0
        || gridZ >= *(_DWORD *)(currentSensorCoverage + 12))
    {
        return -524288;
    }
    dword_A3E2E8 = 0;
    if (!currentSensorCoverage)
    {
        ++dword_A3E2EC;
        dword_A3E2E8 = 1;
        return -524288;
    }
    return *(_DWORD *)(32 * (gridX + *(_DWORD *)(currentSensorCoverage + 4) * (gridY + gridZ * *(_DWORD *)(currentSensorCoverage + 8)))
        + *(_DWORD *)currentSensorCoverage
        + 4);
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4E96A0
_DWORD *__cdecl pingBattlePingsCreate(LinkedList *blobList)
{
_DWORD *result; // eax
int v2; // ecx
_DWORD *v3; // esi

result = (_DWORD *)pingList;
if ( !pingList )
goto LABEL_6;
while ( 1 )
{
v2 = result[3];
if ( *(signed int (__cdecl **)(int, int, int, int))(v2 + 68) == sub_4E9270 )
break;
result = (_DWORD *)*result;
if ( !result )
goto LABEL_6;
}
if ( !v2 )
{
LABEL_6:
result = a1;
v3 = (_DWORD *)*a1;
if ( *a1 )
{
do
{
result = (_DWORD *)v3[3];
if ( (result[26] & 0xC0) != 0 )
result = sub_4E9350(v3[3]);
v3 = (_DWORD *)*v3;
}
while ( v3 );
}
}
return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47EDF0
int __cdecl pingIncrementGridCell(int x, int y, int z)
{
    int result;
    _DWORD *cell;

    if (x >= 0)
    {
        result = currentSensorCoverage;
        if (x < *(int *)(currentSensorCoverage + 4)
            && y >= 0
            && y < *(int *)(currentSensorCoverage + 8)
            && z >= 0
            && z < *(int *)(currentSensorCoverage + 12))
        {
            dword_A3E2E8 = 0;
            if (currentSensorCoverage)
            {
                pingMarkDirtyGridCells(x, y, z, 1);
                cell = (_DWORD *)(32 * (x + *(int *)(currentSensorCoverage + 4)
                    * (y + z * *(int *)(currentSensorCoverage + 8)))
                    + *(int *)currentSensorCoverage);
                ++*cell;           // increment count
                cell[1] = 1;       // set active flag
            }
            else
            {
                result = ++dword_A3E2EC;
                dword_A3E2E8 = 1;
            }
        }
    }
    return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x47FB80
void __cdecl pingMarkDirtyGridCells(int x, int y, int z, int expand)
{
    int curZ;
    _DWORD *cell;

    if (x >= 0)
    {
        curZ = z;
        for (int prevZ = z - 1; ; ++prevZ)
        {
            if (x >= *(int *)(currentSensorCoverage + 4)) break;
            if (y < 0) break;
            if (y >= *(int *)(currentSensorCoverage + 8) || prevZ < -1
                || curZ >= *(int *)(currentSensorCoverage + 12)) break;

            int cellOffset = 32 * (x + *(int *)(currentSensorCoverage + 4)
                * (y + curZ * *(int *)(currentSensorCoverage + 8)));

            // Mark flag array 1 (offset 20)
            if (!*(int *)(cellOffset + *(int *)currentSensorCoverage + 20))
            {
                *(int *)(cellOffset + *(int *)currentSensorCoverage + 20) = 1;
                _DWORD *entry = (_DWORD *)(dword_A3E300 + 12 * dword_A3E304);
                entry[0] = x; entry[1] = y; entry[2] = curZ;
                ++dword_A3E304;
            }
            // Mark flag array 2 (offset 24)
            if (!*(int *)(cellOffset + *(int *)currentSensorCoverage + 24))
            {
                *(int *)(cellOffset + *(int *)currentSensorCoverage + 24) = 1;
                _DWORD *entry = (_DWORD *)(dword_A3E308 + 12 * dword_A3E30C);
                entry[0] = x; entry[1] = y; entry[2] = curZ;
                ++dword_A3E30C;
            }
            // Mark flag array 3 (offset 28)
            _DWORD *flag3 = (_DWORD *)(cellOffset + *(int *)currentSensorCoverage + 28);
            if (!*flag3)
            {
                *flag3 = 1;
                _DWORD *entry = (_DWORD *)(dword_A3E310 + 12 * dword_A3E314);
                entry[0] = x; entry[1] = y; entry[2] = curZ;
                ++dword_A3E314;
            }

            if (!expand) break;
            // Expand to 6 cardinal neighbors
            pingMarkDirtyGridCells(x - 1, y, curZ, 0);
            pingMarkDirtyGridCells(x + 1, y, curZ, 0);
            pingMarkDirtyGridCells(x, y - 1, curZ, 0);
            pingMarkDirtyGridCells(x, y + 1, curZ, 0);
            pingMarkDirtyGridCells(x, y, prevZ, 0);
            ++curZ;
            expand = 0;
        }
    }
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4E9630
real32 * __cdecl pingNewShipBuilt(real32 *position)
{
  real32 *result; // eax

  result = pingCreate(a1, 0, (int)sub_497DB0, &a1, 0, 0);
  *((_DWORD *)result + 4) = dword_88D874;
  *((_DWORD *)result + 5) = dword_88D878;
  *((_DWORD *)result + 6) = dword_88D87C;
  *((_DWORD *)result + 13) = dword_88D880;
  *((_DWORD *)result + 14) = dword_88D884;
  *((_DWORD *)result + 15) = dword_88D888;
  *((_DWORD *)result + 21) = 2;
  *((_DWORD *)result + 7) = dword_A50D08;
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x497DB0
sdword pingEvalAlwaysTrue(void)
{
    return TRUE;
}
#endif

Node *pingListGetHead(void)
{
    return pingList.head;
}

#if 0  // Cat2: pingBattleBlobCriteria live code differs from binary or raw IDA offsets
/*=== pingBattleBlobCriteria (0X4E9210) ===*/
BOOL __cdecl pingBattleBlobCriteria(int a1, int a2)
{
return !*(_DWORD *)(a2 + 16)
&& ((unsigned int)&aisTeams[30].selection[2].sel.ShipPtr[212] & *(_DWORD *)(a2 + 20)) == 0
&& (*(_BYTE *)(a2 + 858) && *(Player **)(*(_DWORD *)(a2 + 836) + 812) != universe.curPlayerPtr
|| *(_BYTE *)(a2 + 856))
&& (byte_892BA8[*(_DWORD *)(a2 + 688)] & 4) == 0;
}
#endif

#if 0  // Cat2: pingBattlePingEvaluate live code differs from binary or raw IDA offsets
/*=== pingBattlePingEvaluate (0X4E9270) ===*/
BOOL __cdecl pingBattlePingEvaluate(char *voidPing, int a2, int a3, int a4)
{
_DWORD *v4; // esi
int v5; // ecx
int v7; // eax
int v8; // ebx
Ship **v9; // edi
int v10; // edx
v4 = voidPing + 88;
if ( a4 )
{
if ( dword_C3009C->objtype )
{
return 0;
}
else
{
v7 = *((_DWORD *)voidPing + 25);
v8 = 0;
v9 = (Ship **)(voidPing + 132);
if ( v7 > 0 )
{
while ( *v9 != dword_C3009C )
{
++v8;
++v9;
if ( v8 >= v7 )
return *((_DWORD *)voidPing + 25) == 0;
}
battleAccountForLosses(*v9, (float *)voidPing + 22);
if ( univIsObjFriendly((struct SpaceObj *)*v9, universe.curPlayerPtr) )
--*((_DWORD *)voidPing + 23);
else
--*v4;
v10 = *((_DWORD *)voidPing + 25) - 1;
*((_DWORD *)voidPing + 25) = v10;
if ( v8 < v10 )
*v9 = (Ship *)v4[v10 + 11];
}
return *((_DWORD *)voidPing + 25) == 0;
}
}
else
{
v5 = *((_DWORD *)voidPing + 23);
if ( *v4 )
{
if ( v5 )
battlePingEvaluate(voidPing, (float *)voidPing + 22);
else
seekdir();
return 1;
}
else
{
if ( v5 )
battlePingEvaluateNoEnemies(voidPing, (float *)voidPing + 22);
return 1;
}
}
}
#endif

#if 0  // Cat2: pingAttackPingsCreate live code differs from binary or raw IDA offsets
/*=== pingAttackPingsCreate (0X4E9350) ===*/
_DWORD *__cdecl pingAttackPingsCreate(_DWORD *a1)
{
_DWORD *v1; // ebx
int v2; // esi
int v3; // eax
int *v4; // edx
real32 *v5; // eax
_DWORD *v6; // edi
int v7; // eax
int v8; // ebx
_DWORD *v9; // eax
struct SpaceObj **v10; // ebp
int v11; // eax
struct SpaceObj *v12; // edx
int v13; // eax
int v14; // ecx
struct SpaceObj **v15; // esi
bool v16; // cc
_DWORD *v18; // [esp+8h] [ebp-20h] BYREF
_DWORD *v19; // [esp+Ch] [ebp-1Ch]
int *i; // [esp+10h] [ebp-18h]
int v21; // [esp+14h] [ebp-14h]
int v22; // [esp+18h] [ebp-10h]
_DWORD *v23[3]; // [esp+1Ch] [ebp-Ch] BYREF
v1 = a1;
bobSubBlobListCreate((int)&unk_C300C0, (int *)v23, (int)a1, (int (__cdecl *)(int, float *))pingBattleBlobCriteria);
for ( i = v23[0]; i; i = (int *)*i )
{
v2 = i[3];
v3 = v1[31] + v1[29] + v1[27] + v1[28];
dword_A50CF8 = v3;
v4 = (int *)v1[34];
if ( *v4 > v3 )
{
v3 = *v4;
dword_A50CF8 = *v4;
}
v5 = pingCreate((real32 *)(v2 + 32), 0, (int)pingBattlePingEvaluate, &v18, 4 * v3 + 44, 0);
*((_DWORD *)v5 + 4) = dword_88D860;
v5[5] = *(real32 *)(v2 + 44);
*((_DWORD *)v5 + 6) = dword_88D864;
*((_DWORD *)v5 + 13) = dword_88D868;
*((_DWORD *)v5 + 14) = dword_88D86C;
*((_DWORD *)v5 + 15) = dword_88D870;
*((_DWORD *)v5 + 21) = 8;
*((_DWORD *)v5 + 7) = dword_A50D04;
*v18 = *(_DWORD *)(v2 + 108) + *(_DWORD *)(v2 + 112) + *(_DWORD *)(v2 + 116);
v18[1] = *(_DWORD *)(v2 + 124);
v18[3] = *v18 + v18[1];
v18[2] = *(_DWORD *)(v2 + 44);
v18[5] = 0;
v18[4] = 0;
v18[8] = 0;
v18[7] = 0;
v18[3] = **(_DWORD **)(v2 + 248);
v18[10] = 0;
qmemcpy(v18 + 11, (const void *)(*(_DWORD *)(v2 + 248) + 8), 4 * v18[3]);
v6 = v18;
v7 = v18[3];
v19 = v18 + 11;
v22 = 0;
if ( v7 > 0 )
{
do
{
v8 = *(_DWORD *)(*v19 + 880);
if ( v8
&& *(_WORD *)(v8 + 20) == 2
&& (universe.totaltimeelapsed != *(float *)(v8 + 808) || v6 != *(_DWORD **)(v8 + 812)) )
{
v9 = *(_DWORD **)(v8 + 556);
v10 = (struct SpaceObj **)(v9 + 2);
v11 = *v9;
if ( v11 > 0 )
{
v21 = v11;
do
{
v12 = *v10;
if ( !*((_DWORD *)*v10 + 4)
&& ((unsigned int)&aisTeams[30].selection[2].sel.ShipPtr[212] & *((_DWORD *)v12 + 5)) == 0
&& (*((_BYTE *)v12 + 858) && *(Player **)(*((_DWORD *)v12 + 209) + 812) != universe.curPlayerPtr
|| *((_BYTE *)v12 + 856))
&& (byte_892BA8[*((_DWORD *)v12 + 172)] & 4) == 0 )
{
v13 = v6[3];
v14 = 0;
v15 = (struct SpaceObj **)(v6 + 11);
if ( v13 <= 0 )
{
LABEL_20:
if ( v13 < dword_A50CF8 )
{
v6[v13 + 11] = v12;
if ( univIsObjFriendly(*v10, universe.curPlayerPtr) )
++v18[1];
else
++*v18;
++v18[3];
v6 = v18;
}
}
else
{
while ( *v15 != v12 )
{
++v14;
++v15;
if ( v14 >= v13 )
goto LABEL_20;
}
}
}
++v10;
--v21;
}
while ( v21 );
}
*(real32 *)(v8 + 808) = universe.totaltimeelapsed;
*(_DWORD *)(v8 + 812) = v18;
v6 = v18;
}
v16 = ++v22 < v6[3];
++v19;
}
while ( v16 );
v1 = a1;
}
}
return bobListDelete(v23);
}
#endif

#if 0  // Cat2: pingLoad live code differs from binary or raw IDA offsets
/*=== pingLoad (0X4E9790) ===*/
int pingLoad()
{
int result; // eax
ubyte *NextChunk; // ebp
int v2; // esi
int *v3; // ebx
ShipPtr Ship; // eax
int v5; // eax
_DWORD *SelectionAndFix; // eax
int v7; // [esp-24h] [ebp-28h]
int v8; // [esp+0h] [ebp-4h]
result = LoadInfoNumber();
if ( result > 0 )
{
v8 = result;
do
{
NextChunk = (ubyte *)LoadNextChunk();
v2 = *((_DWORD *)NextChunk + 20) + 88;
v3 = (int *)memAllocFunction(v2, MBF_Volatile);
qmemcpy(v3, NextChunk + 8, v2);
memFree(NextChunk);
Ship = SpaceObjRegistryGetShip(v3[11]);
v7 = v3[19];
v3[11] = (int)Ship;
v3[19] = (int)SpaceObjRegistryGetShip(v7);
v5 = v3[18];
v3[17] = (int)pingAnomalyPingTimeout;
if ( v5 > 0 )
{
SelectionAndFix = LoadSelectionAndFix();
qmemcpy(v3 + 22, SelectionAndFix, 4 * *SelectionAndFix + 8);
memFree((ubyte *)SelectionAndFix);
}
listAddNode(&pingList, v3, (int)v3);
result = --v8;
}
while ( v8 );
}
return result;
}
#endif
