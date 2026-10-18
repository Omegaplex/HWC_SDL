/*=============================================================================
    Name    : Dock.c
    Purpose : Contains code for docking, and refueling/repairing

    Created 11/25/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <string.h>
#include <math.h>

#include "ShipDefs.h"
#include "types.h"
#include "memory.h"
#include "debug.h"
#include "vector.h"
#include "fastmath.h"
#include "linkedlist.h"
#include "objtypes.h"
#include "spaceobj.h"
#include "universe.h"
#include "aitrack.h"
#include "aiship.h"
#include "commandlayer.h"
#include "aiplayer.h"
#include "prim3d.h"
#include "render.h"
#include "select.h"
#include "HiveFrigate.h"
#include "soundevent.h"
#include "dock.h"
#include "Ship.h"
#include "univupdate.h"
#include "drone.h"
#include "physics.h"
#include "collision.h"

#include "launchMgr.h"
#include "tactics.h"
#include "singleplayer.h"
#include "Dreadnought.h"
#include "ACV.h"

#include "madlinkin.h"

#include "madlinkindefs.h"
#include "mothership.h"
#include "clamp.h"
#include "salcapcorvette.h"
#include "tutor.h"
#include "randy.h"
#include "aivar.h"
#include "alliance.h"
#include "battle.h"
#include "DDDFrigate.h"
#include "Carrier.h"
#include "HeavyCruiser.h"
#include "consMgr.h"
#include "CommandWrap.h"
#include "CameraCommand.h"
#include "InfoOverlay.h"
#include "researchapi.h"

static bool32 shipIsBeastShip(Ship *ship);


//#define DEBUG_DOCKING

void telldir(void);

#define NOT_FINISHED_DOCKING 0
#define FINISHED_DOCKING_FLYING_BACK 1
#define FINISHED_DOCKING 2
#define PERMANENTLY_DOCKED 3
#define FINISHED_LAUNCHING_FROM_DOCKING 4
#define FINISHED_RETIREMENT             5

bool32 isCapitalShipStaticOrBig(ShipStaticInfo *shipstatic);
void selSelectionAddSingleShipNoCheck(MaxSelection *dest, Ship *ship);
void acolyteStartLinking(Ship *ship);
void acolyteStopLinking(Ship *ship);

typedef struct ShipSwarmerOverlay
{
    udword unk_638;
    udword resetState;
    Ship *swarmers[6];
} ShipSwarmerOverlay;

static ShipSwarmerOverlay *shipSwarmerOverlay(Ship *ship);

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 LATCH_TOLERANCE = 25.0f;

real32 CONEORIGIN_MINFLYSPEED = -40.0f;
real32 CONEORIGIN_TOLERANCE = 50.0f;

real32 DRONE_CONEORIGIN_MINFLYSPEED = -40.0f;
real32 DRONE_CONEORIGIN_TOLERANCE = 25.0f;

real32 BACKOUT_MINFLYSPEED = -80.0f;
real32 BACKOUT_TOLERANCE = 50.0f;

real32 FLYOUTCONE_MINFLYSPEED = -120.0f;
real32 FLYOUTCONE_TOLERANCE = 65.0f;

real32 CONEINSIDE_MINFLYSPEED = -80.0f;
real32 CONEINSIDE_TOLERANCE = 50.0f;

real32 DOCK_SLIPGATE_TRANSIT_DELAY = 1.5f;    /* retail constant at 0x72A7EC */

real32 BACKTODESTINATION_MINFLYSPEED = -40.0f;
real32 BACKTODESTINATION_TOLERANCE = 50.0f;

real32 BUSY_ADD_TO_DISTANCE = 2000.0f;
real32 BUSY_ADD_TO_DISTANCE_DEPOSITRU = 5000.0f;

real32 R1RESCONTROLLER_FIGHTERLATCHDIST = 0.0f;
real32 R1RESCONTROLLER_CORVETTELATCHDIST = 0.0f;
real32 R1RESCONTROLLER_RESOURCERLATCHDIST = 0.0f;
real32 R1RESCONTROLLER_LATCHMINTIME = 1.0f;

real32 R2RESCONTROLLER_FIGHTERLATCHDIST = 0.0f;
real32 R2RESCONTROLLER_CORVETTELATCHDIST = 0.0f;
real32 R2RESCONTROLLER_RESOURCERLATCHDIST = 0.0f;
real32 R2RESCONTROLLER_LATCHMINTIME = 1.0f;

real32 P2FUELPOD_FIGHTERLATCHDIST = 0.0f;
real32 P2FUELPOD_LATCHMINTIME = 1.0f;

real32 R1ASF_FIGHTERLATCHDIST = 0.0f;
real32 R1ASF_CORVETTELATCHDIST = 0.0f;
real32 R1ASF_LATCHMINTIME = 1.0f;

real32 R2ASF_FIGHTERLATCHDIST = 0.0f;
real32 R2ASF_CORVETTELATCHDIST = 0.0f;
real32 R2ASF_LATCHMINTIME = 1.0f;

real32 WAIT_FOR_LATCH_DISTANCE_ROUGH_GUESS_REALLY_RIGHT_NOW = 3333.0f;

real32 R1CARRIER_LATCHMINTIME = 1.0f;
real32 R1CARRIER_REFUELREPAIRMINTIME = 1.0f;

real32 R2CARRIER_LATCHMINTIME = 1.0f;
real32 R2CARRIER_REFUELREPAIRMINTIME = 1.0f;

real32 R1MOTHERSHIP_LATCHMINTIME = 1.0f;
real32 R1MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 R2MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 P1MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 P2MOTHERSHIP_REFUELREPAIRMINTIME = 1.0f;

real32 R1REPCORV_FIGHTERLATCHDIST = 150.0f;
real32 R1REPCORV_CORVETTELATCHDIST = 300.0f;
real32 R1REPCORV_LATCHMINTIME = 3.0f;

real32 R2REPCORV_FIGHTERLATCHDIST = 150.0f;
real32 R2REPCORV_CORVETTELATCHDIST = 300.0f;
real32 R2REPCORV_LATCHMINTIME = 3.0f;

real32 R1RESCOL_FIGHTERLATCHDIST = 150.0f;
real32 R1RESCOL_LATCHDIST = 300.0f;
real32 R1RESCOL_LATCHMINTIME = 3.0f;

real32 R2RESCOL_FIGHTERLATCHDIST = 150.0f;
real32 R2RESCOL_LATCHDIST = 300.0f;
real32 R2RESCOL_LATCHMINTIME = 3.0f;

real32 JUNKYARDHQ_LATCHDIST = 300.0f;
real32 JUNKYARDHQ_LATCHMINTIME = 3.0f;

real32 JUNKYARDHQ_MUSTBENEAR_TO_DOCK = 12000.0f;

real32 DRONE_LAUNCH_VELOCITY = 300.0f;

real32 CARRIERMOTHERDOCK_DELAY_FLYINSIDE = 10.0f;
real32 CARRIERMOTHERDOCK_DELAY_FLYINSIDE_CORVETTE = 15.0f;

real32 SHIP_NEED_REFUEL_PERCENT = 0.7f;
real32 SHIP_NEED_REPAIR_PERCENT = 0.7f;

/*=============================================================================
    Private functions:
=============================================================================*/

static void dockBusyThisShip(Ship *ship,Ship *dockwith);
static void dockUnbusyThisShip(Ship *ship);
static void dockSetAvoidanceIgnoreTarget(Ship *ship, Ship *target);
sdword dockNeedRotationOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused);
sdword dockNeedOffsetOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused);

static void dockApplyLaunchOrientationRotation(Ship *ship, real32 angle, void (*makeRotation)(matrix *mat, real32 costheta, real32 sintheta))
{
    matrix rotation;
    matrix current;

    makeRotation(&rotation, cosf(angle), sinf(angle));
    current = ship->rotinfo.coordsys;
    matMultiplyMatByMat(&ship->rotinfo.coordsys, &current, &rotation);
}

CommandToDo *findFormationGuardingShip(Ship *ship);

extern void clSetAttribute(sdword ship, sdword value, sdword attribute);
extern Ship *shipAddSupportCapacity(Ship *ship);
extern sdword SpaceObjRegistryGetID(SpaceObj *obj);
extern Ship *SpaceObjRegistryGetShip(sdword ID);
extern sdword univIncrementShipTypeCount(sdword shipType, sdword playerIndex);
extern void univReallyDeleteThisShipRightNow(Ship *ship);

extern bool32 lmShipAutoLaunchEnabled(Ship *ship);
extern void shipApplyPlayerUpgrades(Ship *ship);
extern void acolyteStartLinking(Ship *ship);
extern void acvInitSpecial(Ship *ship);
extern void seekdir(void);

sdword cmGetBuildState(unsigned int shipclass, unsigned int shiptype, int isAcolyte, udword shipFlags, int checkMode, Ship *factoryShip)
{
    Ship *dockwith = factoryShip;
    ShipStaticInfo *staticinfo;
    DockStaticInfo *dockStaticInfo;

    if (dockwith == NULL || dockwith->staticinfo == NULL)
    {
        return -1;
    }

    staticinfo = dockwith->staticinfo;
    dockStaticInfo = staticinfo->dockStaticInfo;
    if (dockStaticInfo == NULL)
    {
        return -1;
    }

    switch (checkMode)
    {
        case 0:
        case 8:
            switch (shipclass)
            {
                case CLASS_Mothership:
                case CLASS_HeavyCruiser:
                case CLASS_Carrier:
                case CLASS_Destroyer:
                    return DPT_Large_In;
                case CLASS_Frigate:
                    return DPT_Med_In;
                case CLASS_Corvette:
                    return dockStaticInfo->firstPointOfType[DPT_Small_In] == 0xFF ? DPT_Corvette_Latch : DPT_Small_In;
                case CLASS_Fighter:
                    return dockStaticInfo->firstPointOfType[DPT_Small_In] == 0xFF ? DPT_Fighter_Latch : DPT_Small_In;
                case CLASS_Resource:
                    switch (shiptype)
                    {
                        case sWorker:
                            if (dockwith->shiptype == dEcclesiamInfected && singlePlayerGame && singlePlayerGameInfo.currentMission == 16)
                            {
                                return DPT_Tow;
                            }
                            return DPT_Med_In;
                        case bWorker:
                            return DPT_Med_In;
                        case tResourceCollector:
                            return DPT_Resourcer_RU_Transfer;
                        default:
                            return DPT_Large_In;
                    }
                case CLASS_NonCombat:
                    if (shiptype == dCrystal0 || shiptype == dCrystal1)
                    {
                        return DPT_Crystal_Process;
                    }
                    return -1;
                case CLASS_Micro:
                    if (shiptype == sSwarmer || shiptype == bSwarmer)
                    {
                        return DPT_Swarmer_In;
                    }
                    return DPT_Small_In;
                default:
                    return -1;
            }

        case 1:
        case 9:
            if (shipclass == CLASS_Micro)
            {
                if (shiptype == sSwarmer || shiptype == bSwarmer)
                {
                    return DPT_Swarmer_In;
                }
                return DPT_Small_In;
            }
            if (shipclass == CLASS_NonCombat)
            {
                return DPT_Med_In;
            }
            return DPT_Small_In;

        case 2:
            if ((staticinfo->shipFlags & SSTATF_BeastShip) != 0)
            {
                switch (shipclass)
                {
                    case CLASS_Mothership:
                        return DPT_Mothership_Out;
                    case CLASS_Frigate:
                        return DPT_Med_Out;
                    case CLASS_Corvette:
                    case CLASS_Fighter:
                    case CLASS_NonCombat:
                    case CLASS_Component:
                    case CLASS_Micro:
                        return DPT_Small_Out;
                    case CLASS_Resource:
                        if (shiptype != sProcessor && shiptype != bProcessor)
                        {
                            return DPT_Med_Out;
                        }
                        return DPT_Large_Out;
                    default:
                        return DPT_Large_Out;
                }
            }
            if (isAcolyte)
            {
                switch (shiptype)
                {
                    case sDreadnought:
                    case sCarrier:
                    case sProcessor:
                    case sDestroyer:
                    case bHeavyCruiser:
                    case bCarrier:
                    case bProcessor:
                        return dockwith->shiptype == sMothership || dockwith->shiptype == bMothership ? DPT_Destroyer_EB : DPT_SalvageProcess;
                    case sCarrierSupport:
                    case sMothershipSupport:
                    case bMothershipSupport:
                        return DPT_Support_EB;
                    case sMothershipSpecial:
                    case sMothershipEngineering:
                    case sMothershipArmour:
                    case sMothershipMicro:
                    case sMothershipWeapons:
                    case sMothershipDockingBay:
                    case sMothershipBigGun:
                        return DPT_Module_EB;
                    default:
                        break;
                }
            }
            switch (shipclass)
            {
                case CLASS_Mothership:
                case CLASS_HeavyCruiser:
                case CLASS_Carrier:
                case CLASS_Destroyer:
                    return DPT_Large_Out;
                case CLASS_Frigate:
                case CLASS_NonCombat:
                    return DPT_Med_Out;
                case CLASS_Corvette:
                case CLASS_Fighter:
                    return DPT_Small_Out;
                case CLASS_Resource:
                    if (shiptype == sProcessor || shiptype == bProcessor)
                    {
                        return DPT_Large_Out;
                    }
                    return DPT_Med_Out;
                case CLASS_Micro:
                    if (shiptype == sSwarmer || shiptype == bSwarmer)
                    {
                        return DPT_Swarmer_Out;
                    }
                    return DPT_Small_Out;
                default:
                    return -1;
            }

        case 3:
            if (shiptype == tResourceCollector)
            {
                return DPT_Resourcer_RU_Transfer;
            }
            if ((shipFlags & 1) != 0)
            {
                return DPT_Worker_RU_Transfer;
            }
            return (shipFlags & 0x200) != 0 ? DPT_Leech_RU_Transfer : -1;

        case 4:
            return DPT_Leech;

        case 5:
            return DPT_Tow;

        case 7:
            switch (shiptype)
            {
                case dBigGunUnit:
                    return dockwith->shiptype == sMothership || dockwith->shiptype == bMothership ? DPT_Destroyer_EB : DPT_SalvageProcess;
                case dMessagePod:
                    return DPT_Med_In;
                case dCrystal0:
                case dCrystal1:
                    return DPT_Crystal_Process;
                default:
                    if (shipclass >= CLASS_Carrier &&
                        (shipclass <= CLASS_Destroyer || (shipclass == CLASS_Resource && (shiptype == sProcessor || shiptype == bProcessor))))
                    {
                        return DPT_Large_In;
                    }
                    return DPT_Med_In;
            }

        case 10:
            if ((staticinfo->shipFlags & SSTATF_BeastShip) == 0)
            {
                return -1;
            }
            switch (shipclass)
            {
                case CLASS_Mothership:
                    return DPT_Mothership_In;
                case CLASS_Frigate:
                    return DPT_Med_In;
                case CLASS_Corvette:
                case CLASS_Fighter:
                case CLASS_NonCombat:
                case CLASS_Component:
                case CLASS_Micro:
                    return DPT_Small_In;
                case CLASS_Resource:
                    if (shiptype == sProcessor || shiptype == bProcessor)
                    {
                        return DPT_Large_In;
                    }
                    return DPT_Med_In;
                default:
                    return DPT_Large_In;
            }

        default:
            return -1;
    }
}

static DockType dockModeFromRequest(DockType dockType)
{
    switch (dockType)
    {
        case DOCK_MODE_BEAST:
        case DOCK_MODE_RETIRE:
        case DOCK_MODE_DEPOSIT_RESOURCES:
        case DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH:
        case DOCK_MODE_INSTANT_REFUELREPAIR:
        case DOCK_MODE_STAY_TILL_EXPLICITLAUNCH:
        case DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP:
            return dockType;
        default:
            break;
    }

    if (dockType & DOCK_AT_SPECIFIC_SHIP)
    {
        return DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP;
    }
    if (dockType & DOCK_FOR_RETIRE)
    {
        return DOCK_MODE_RETIRE;
    }
    if (dockType & DOCK_TO_DEPOSIT_RESOURCES)
    {
        return DOCK_MODE_DEPOSIT_RESOURCES;
    }
    if (dockType & (DOCK_PERMANENTLY | DOCK_STAY_TILL_EXPLICITLAUNCH))
    {
        return (dockType & DOCK_INSTANTANEOUSLY) != 0
            ? DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH
            : DOCK_MODE_STAY_TILL_EXPLICITLAUNCH;
    }
    if (dockType & DOCK_INSTANTANEOUSLY)
    {
        return DOCK_MODE_INSTANT_REFUELREPAIR;
    }
    return DOCK_MODE_REFUELREPAIR;
}

static udword dockProcessSingleShipToDo(CommandToDo *docktodo, Ship *ship, Ship *dockwith)
{
    if (ShipStaticInfoGetDockProcessCallback(dockwith->staticinfo) != NULL)
    {
        return ShipStaticInfoGetDockProcessCallback(dockwith->staticinfo)(docktodo, ship, dockwith);
    }

    return processSingleLaunchShipToDo(ship, dockwith, dockCommandGetMode(docktodo)) ? FINISHED_DOCKING : NOT_FINISHED_DOCKING;
}

static bool32 shipIsBeastShip(Ship *ship)
{
    return ship != NULL &&
           ship->staticinfo != NULL &&
           (ship->staticinfo->shipFlags & SSTATF_BeastShip) != 0;
}

bool32 dockIsSlipgateShip(Ship *ship)
{
    return ship != NULL &&
           ship->shiptype >= dSlipgateSmall &&
           ship->shiptype <= dSlipgateLarge;
}

static bool32 univIsObjFriendly(SpaceObj *obj, Player *player)
{
    Player *ownerPlayer;

    if (obj == NULL || player == NULL || obj->objtype != OBJ_ShipType)
    {
        return FALSE;
    }
    if (((Ship *)obj)->shiptype == 111)
    {
        return FALSE;
    }

    ownerPlayer = ((Ship *)obj)->playerowner;
    if (ownerPlayer == NULL)
    {
        return FALSE;
    }
    if (ownerPlayer == player || (obj->attributes & 0x100) != 0)
    {
        return TRUE;
    }

    return (player->Allies & (1 << ownerPlayer->playerIndex)) != 0;
}

bool32 IsCrystal(Ship *ship)
{
    return ship != NULL &&
           (ship->shiptype == dCrystal0 || ship->shiptype == dCrystal1);
}

void dockWorkerClampFunc(Ship *ship)
{
    sdword dockPointIndex = ship->dockvars.dockPointIndex;

    if (dockPointIndex != -1)
    {
        Ship *dockShip = ship->dockvars.dockship;

        // Non-parity assert: retail (dockWorkerClampFunc @ 0x465360) also
        // dereferences dockShip unconditionally here once dockPointIndex != -1,
        // and univRemoveAllShipReferencesFromSpecialShips (0x5476e0) nulls a
        // ship's dockvars.dockship without resetting dockPointIndex when its
        // dock target dies, so retail can reach this exact state too. Catch it
        // loudly with full context instead of a raw null-pointer crash so we
        // can gather timing evidence (same-frame chain death vs. stale state
        // from an earlier frame) on repro.
        if (dockShip == NULL)
        {
            dbgFatalf(DBG_Loc, "dockWorkerClampFunc saw ship=%p (shiptype=%d) with dockPointIndex=%d but NULL dockvars.dockship",
                      ship, ship->shiptype, dockPointIndex);
        }

        DockStaticInfo *dockStaticInfo = dockShip->staticinfo->dockStaticInfo;
        DockPointActivity *dockPointActivity = dockShip->dockvars.dockPointActivity;
        DockPointType dockPointType = dockStaticInfo->dockstaticpoints[dockPointIndex].type;

        dockPointActivity->reserved[dockPointIndex] = 0;
        dockPointActivity->numDockedOfType[dockPointType]--;
        ship->dockvars.dockPointIndex = -1;
    }

    ship->specialFlags1C8 &= ~BIT12;
    ship->flags &= ~SOF_AvoidanceHidden;
    ship->specialFlags2 &= ~0x40u;

    if (ship->clampInfo != NULL && ship->clampInfo->host != NULL)
    {
        if ((Ship *)ship->clampInfo->host != ship->dockvars.dockship)
        {
            ShipTypeToStr(ship->shiptype);
            telldir();
        }
        clDetachShipFromCommand(ship);
    }

    dockSetAvoidanceIgnoreTarget(ship, NULL);
    ship->dockvars.dockship = NULL;

    switch (ship->shiptype)
    {
        case sWorker:
        case bWorker:
            if (*(udword *)(ship->runtimeAnimationOverlay.madAnimationData + 12) == 3)
            {
                madAnimationStart(ship, 2);
            }
            break;

        default:
            break;
    }
}

static void dockPrepareContainedShipsForSlipgateTransit(Ship *ship)
{
    ShipsInsideMe *shipsInsideMe = ship->shipsInsideMe;
    Node *insideNode;

    if (shipsInsideMe == NULL)
    {
        return;
    }

    for (insideNode = shipsInsideMe->insideList.tail; insideNode != NULL; insideNode = insideNode->prev)
    {
        InsideShip *insideShip = listGetStructOfNode(insideNode);
        Ship *containedShip = insideShip->ship;

        workerCheckResetForHyperspace(containedShip);
        if (shipIsBeastRace(containedShip))
        {
            *(ubyte *)containedShip->ShipSpecifics = 1;
        }
        SalCapNotifyProcessingShipRemoved(containedShip);
    }
}

static ShipSwarmerOverlay *shipSwarmerOverlay(Ship *ship)
{
    return (ShipSwarmerOverlay *)ship->ShipSpecifics;
}

static void dockCompleteSingleShip(Ship *ship, Ship *dockwith, CommandToDo *docktodo)
{
    SelectCommand *selection = docktodo->selection;

    dockWorkerClampFunc(ship);
    clRemoveTargetFromSelection((SelectAnyCommand *)selection, (TargetPtr)ship);
    ship->command = NULL;

    if (docktodo->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        RemoveShipFromFormation(ship);
        if (selection->numShips > 0)
        {
            formationContentHasChanged(docktodo);
        }
    }

    if (docktodo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        RemoveShipFromAttacking(ship);
        RemoveShipReferencesFromExtraAttackInfo(ship, docktodo);
    }

    if (dockCommandGetMode(docktodo) == DOCK_MODE_RETIRE)
    {
        sdword refundedRU = ship->resources + (sdword)(TW_RETIRE_MONEYBACK_RATIO * ship->staticinfo->buildCost);

        if (IsCrystal(ship))
        {
            refundedRU += ship->ShipSpecifics[0];
        }
        playerAddResourceUnits(ship->playerowner, refundedRU);
        ship->howDidIDie = DEATH_BY_RETIREMENT;
        aiplayerShipDied(ship);
        univDeleteWipeInsideShipOutOfExistence(ship);
    }
    else if (ship->dockvars.dockLaunchState == 12 && !dockCommandGetHaveFlyedFarEnoughAway(docktodo))
    {
        if (ship->shiptype != sSwarmer && ship->shiptype != bSwarmer)
        {
            GroupShip(&universe.mainCommandLayer, ship, dockwith);
        }
    }
}

Ship *shipClearSwarmers(Ship *ship)
{
    ShipSwarmerOverlay *overlay;
    Ship *result;
    sdword i;

    overlay = shipSwarmerOverlay(ship);
    overlay->resetState = 0;
    result = NULL;
    for (i = 0; i < 6; i++)
    {
        if (overlay->swarmers[i] != NULL)
        {
            hiveSwarmerSetState(overlay->swarmers[i], HiveSwarmer_Docked);
            result = NULL;
        }
    }
    return result;
}

static bool32 dockCheckAnimStateForCone(Ship *ship, sdword dockPointType)
{
    CarrierSpec *carrierSpec = (CarrierSpec *)ship->ShipSpecifics;

    if (dockPointType != 6 && dockPointType != 7)
    {
        return TRUE;
    }

    if (carrierSpec->dockAnimationState == 0 && madAnimationCompleted(ship))
    {
        return TRUE;
    }

    if (carrierSpec->dockAnimationState == 1 && madAnimationCompleted(ship))
    {
        madAnimationStart(ship, 0);
        carrierSpec->dockAnimationState = 0;
        carrierSpec->dockAnimationTime = -1.0f;
    }

    return FALSE;
}

static bool32 dockMatricesAlignedWithinThreshold(matrix *source, matrix *target, real32 threshold)
{
    return source->m11 * target->m11 + source->m21 * target->m21 + source->m31 * target->m31 >= threshold &&
           source->m13 * target->m13 + source->m23 * target->m23 + source->m33 * target->m33 >= threshold &&
           source->m12 * target->m12 + source->m22 * target->m22 + source->m32 * target->m32 >= threshold;
}


static void dockMarkPointFreed(Ship *dockwith, sdword dockpointindex)
{
    DockStaticInfo *dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    DockStaticPoint *dockstaticpoint;
    DockPointActivity *dockPointActivity = dockwith->dockvars.dockPointActivity;
    DockPointType dockPointType;

    dbgAssert(dockStaticInfo != NULL);
    dbgAssert(dockpointindex >= 0);
    dbgAssert(dockpointindex < dockStaticInfo->numDockPoints);

    dockstaticpoint = &dockStaticInfo->dockstaticpoints[dockpointindex];
    dockPointType = dockstaticpoint->type;

    if (dockPointActivity != NULL)
    {
        if (dockPointActivity->reserved[dockpointindex])
        {
            dockPointActivity->reserved[dockpointindex] = 0;
            if (dockPointActivity->numDockedOfType[dockPointType] > 0)
            {
                dockPointActivity->numDockedOfType[dockPointType]--;
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : GetDirectionVectorOfShip
    Description : gets a direction vector of ship, given a direction (0=up,1=right,2=forward,4=down,5=left,6=back)
    Inputs      : direction, ship
    Outputs     : result
    Return      :
----------------------------------------------------------------------------*/
void GetDirectionVectorOfShip(vector *result,udword direction,matrix *coordsys)
{
    switch (direction & 3)
    {
        case 0:
            matGetVectFromMatrixCol1(*result,*coordsys);
            break;

        case 1:
            matGetVectFromMatrixCol2(*result,*coordsys);
            break;

        case 2:
        case 3:
            matGetVectFromMatrixCol3(*result,*coordsys);
            break;
    }

    if (direction & 4)
    {
        vecNegate(*result);
    }
}

/*-----------------------------------------------------------------------------
    Name        : dockFindDockIndex
    Description : finds a dock point with given name
    Inputs      : name, dockstaticinfo
    Outputs     :
    Return      : returns an index to the found dockpoint
----------------------------------------------------------------------------*/
sdword dockFindDockIndex(char *name,DockStaticInfo *dockstaticinfo)
{
    sdword numDockPoints = dockstaticinfo->numDockPoints;
    DockStaticPoint *curdockpoint;
    sdword i;

    for (i=0,curdockpoint=&dockstaticinfo->dockstaticpoints[0];i<numDockPoints;i++,curdockpoint++)
    {
        if (_stricmp(name,curdockpoint->name) == 0)
        {
            return i;
        }
    }
    dbgFatalf(DBG_Loc,"Dock name %s not found",name);
    return -1;
}

/*-----------------------------------------------------------------------------
    Name        : dockFindDockIndex
    Description : finds a dock point with given name
    Inputs      : name, dockstaticinfo
    Outputs     :
    Return      : returns an index to the found dockstaticpoint
----------------------------------------------------------------------------*/
DockStaticPoint *dockFindDockStaticPoint(char *name,DockStaticInfo *dockstaticinfo)
{
    sdword numDockPoints = dockstaticinfo->numDockPoints;
    DockStaticPoint *curdockpoint;
    sdword i;

    for (i=0,curdockpoint=&dockstaticinfo->dockstaticpoints[0];i<numDockPoints;i++,curdockpoint++)
    {
        if (_stricmp(name,curdockpoint->name) == 0)
        {
            return curdockpoint;
        }
    }
    dbgAssert(FALSE);
    return NULL;
}

/*-----------------------------------------------------------------------------
    Name        : ShipWithinDockRange
    Description : returns TRUE if ship is within docking range of target
    Inputs      : ship, target
    Outputs     :
    Return      : returns TRUE if ship is within docking range of target
----------------------------------------------------------------------------*/
bool32 ShipWithinDockRange(Ship *ship,Ship *target)
{
    vector trajectory;
    ShipStaticInfo *targetStaticInfo = (ShipStaticInfo *)target->staticinfo;

    aishipGetTrajectory(ship, (SpaceObjRotImpTarg *)target, &trajectory);
    return RangeToTarget(ship, (SpaceObjRotImpTarg *)target, &trajectory) <
           targetStaticInfo->dockShipRange +
           targetStaticInfo->staticheader.staticCollInfo.avoidcollspheresize;
}

bool32 ThisShipCanDockWith(Ship *ship, Ship *dockwith, DockType dockType, bool32 checkCapacity)
{
    DockStaticInfo *dockStaticInfo;
    DockStaticInfo *shipDockStaticInfo;
    DockPointActivity *dockPointActivity;
    sdword dockPointType = -1;
    bool32 friendly;
    dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    shipDockStaticInfo = ship->staticinfo->dockStaticInfo;
    dockPointActivity = dockwith->dockvars.dockPointActivity;

    if (dockwith->shiptype == dRKO && ship->shiptype != sMimic && ship->shiptype != sMCV && ship->shiptype != bCruiseMissile)
    {
        return FALSE;
    }
    if (dockwith->shiptype == dBeastMothership && ship->playerowner->playerIndex == 0)
    {
        return FALSE;
    }
    if (dockStaticInfo == NULL)
    {
        return FALSE;
    }

    dockPointType = cmGetBuildState(ship->staticinfo->shipclass,
                                    ship->staticinfo->shiptype,
                                    0,
                                    ship->staticinfo->shipFlags,
                                    dockModeFromRequest(dockType),
                                    dockwith);
    if (dockPointType < 0 || dockPointType >= CATA_DOCK_TYPE_TABLE_SIZE)
    {
        return FALSE;
    }
    if ((dockPointType == 4 || dockPointType == 5) &&
        (ship->staticinfo->dockStaticInfo == NULL || ship->staticinfo->dockStaticInfo->pad_4C == 0))
    {
        return FALSE;
    }
    if (dockStaticInfo->firstPointOfType[dockPointType] == 0xFF)
    {
        return FALSE;
    }
    if (ship->shiptype == dockwith->shiptype)
    {
        return FALSE;
    }
    if ((dockwith->flags & SOF_DeCloaking) != 0 || (ship->flags & SOF_DeCloaking) != 0)
    {
        return FALSE;
    }
    if ((dockwith->attributesParam & 0x400) != 0 && !dockNeedsSpecialDocking(ship, dockType))
    {
        return FALSE;
    }

    if (dockType != 10)
    {
        if (shipIsBeastShip(dockwith))
        {
            return FALSE;
        }
        if (singlePlayerGame &&
            dockwith->shipSinglePlayerGameInfo != NULL &&
            dockwith->shipSinglePlayerGameInfo->shipHyperspaceState != SHIPHYPERSPACE_NONE)
        {
            return FALSE;
        }
    }

    if ((ship->specialFlags2 & SPECIAL_2_DisabledState) == 0 && (dockwith->specialFlags2 & SPECIAL_2_DisabledState) != 0)
    {
        return FALSE;
    }

    friendly = univIsObjFriendly((SpaceObj *)ship, dockwith->playerowner)
            || (ship->specialFlags & SPECIAL_FriendlyStatus) != 0
            || (dockwith->specialFlags2 & SPECIAL_2_FriendlyDockingOverride) != 0
            || (IsCrystal(ship) && (ship->specialTargetShip != NULL || SalCapGetFirstAttachedShip(ship) != NULL));
    if (!friendly)
    {
        return FALSE;
    }
    if ((dockwith->specialFlags2 & SPECIAL_2_DockFlag) != 0)
    {
        return FALSE;
    }
    if (checkCapacity && dockPointActivity != NULL)
    {
        if (dockPointActivity->numDockedOfType[dockPointType] >= dockStaticInfo->numPointsOfType[dockPointType])
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : MakeShipsAbleToDockWithThisShip
    Description : Makes sure that the ships in this selection are able to dock with
                  the ship dockwith.
    Inputs      : selection
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void MakeShipsAbleToDockWithThisShip(SelectCommand *selection,Ship *dockwith,DockType dockType)
{
    sdword i;

    for (i=0;i<selection->numShips;)
    {
        if (!ThisShipCanDockWith(selection->ShipPtr[i],dockwith,dockType,FALSE))
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
    Name        : FindNearestShipToDockAt
    Description : finds nearest ship to dock at
    Inputs      : ship (which wants to dock), dockType (type of docking desired)
    Outputs     :
    Return      : returns nearest ship to dock at
----------------------------------------------------------------------------*/
Ship *FindNearestShipToDockAt(Ship *ship,DockType dockType)
{
    Node *objnode;
    Ship *dockat;
    real32 mindist = REALlyBig;
    Ship *closestDockat = NULL;

    if (dockType == DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP)
    {
        for (objnode = universe.ShipList.head; objnode != NULL; objnode = objnode->next)
        {
            ShipsInsideMe *shipsInsideMe;
            Node *insideNode;

            dockat = (Ship *)listGetStructOfNode(objnode);
            dbgAssert(dockat->objtype == OBJ_ShipType);
            shipsInsideMe = dockat->shipsInsideMe;
            if (shipsInsideMe == NULL)
            {
                continue;
            }

            for (insideNode = shipsInsideMe->insideList.head; insideNode != NULL; insideNode = insideNode->next)
            {
                InsideShip *insideShip = (InsideShip *)listGetStructOfNode(insideNode);

                if (insideShip->ship == ship)
                {
                    return dockat;
                }
            }
        }

        return NULL;
    }

    for (objnode = universe.ShipList.head; objnode != NULL; objnode = objnode->next)
    {
        vector diff;
        real32 dist;

        dockat = (Ship *)listGetStructOfNode(objnode);
        dbgAssert(dockat->objtype == OBJ_ShipType);
        if (!ThisShipCanDockWith(ship, dockat, dockType, FALSE))
        {
            continue;
        }

        vecSub(diff, dockat->posinfo.position, ship->posinfo.position);
        dist = fsqrt(vecMagnitudeSquared(diff));
        if (dist < mindist)
        {
            mindist = dist;
            closestDockat = dockat;
        }
    }

    return closestDockat;
}

/*-----------------------------------------------------------------------------
    Name        : allShipsDockingWithSameShip
    Description : returns TRUE if all ships in selection are docking with same ship
    Inputs      : selectcom
    Outputs     :
    Return      : returns TRUE if all ships in selection are docking with same ship
----------------------------------------------------------------------------*/
bool32 allShipsDockingWithSameShip(SelectCommand *selectcom)
{
    sdword numShips = selectcom->numShips;
    Ship *firstship;
    Ship *ship;
    sdword i;

    dbgAssert(numShips > 0);

    firstship = (selectcom->ShipPtr[0])->dockvars.dockship;

    for (i=1;i<numShips;i++)
    {
        ship = (selectcom->ShipPtr[i])->dockvars.dockship;

        if (ship != firstship)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static void dockSetAvoidanceIgnoreTarget(Ship *ship, Ship *target)
{
    ship->unk_618 = (udword)target;
}

void dockSetDockingTarget(Ship *ship, Ship *target)
{
    dbgAssert(ship != NULL);
    ship->dockingCollisionTarget = target;
    ship->cataBusyStateWindow.dockingship = target;
}

void dockClearDockingTarget(Ship *ship)
{
    dbgAssert(ship != NULL);
    ship->dockingCollisionTarget = NULL;
    ship->cataBusyStateWindow.dockingship = NULL;
    dockSetAvoidanceIgnoreTarget(ship, NULL);
}

/*-----------------------------------------------------------------------------
    Name        : dockGetAppropriateTypeOfDocking
    Description : returns type of dockType based on repair/refuel state of selection of ships
    Inputs      : selectcom
    Outputs     :
    Return      : dockType to use
----------------------------------------------------------------------------*/
DockType dockGetAppropriateTypeOfDocking(SelectCommand *selectcom)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    DockType dockType = DOCK_FOR_BOTHREFUELANDREPAIR;
    sdword shipsConsidered = 0;
    sdword shipsFuelConsidered = 0;
    bool32 shipsNeedRepair = FALSE;
    bool32 shipsNeedRefuel = FALSE;
    real32 avgfuel = 0.0f;
    real32 avghealth = 0.0f;

    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[0];
        shipstatic = ship->staticinfo;
        if (ShipTypeIsWorker(ship->shiptype))
        {
            return DOCK_FOR_REFUELREPAIR;
        }
        else if (!isCapitalShipStatic(shipstatic))
        {
            shipsConsidered++;
            avghealth += (ship->health / shipstatic->maxhealth);

            if (shipstatic->maxfuel != 0.0f)
            {
                avgfuel += (ship->fuel / shipstatic->maxfuel);
                shipsFuelConsidered++;
            }
        }
    }

    if (shipsConsidered > 0)
    {
        avghealth /= shipsConsidered;
        if (avghealth < SHIP_NEED_REPAIR_PERCENT)
        {
            shipsNeedRepair = TRUE;
        }
        if (shipsFuelConsidered > 0)
        {
            avgfuel /= shipsFuelConsidered;
            if (avgfuel < SHIP_NEED_REFUEL_PERCENT)
            {
                shipsNeedRefuel = TRUE;
            }
        }

        if (shipsNeedRepair && shipsNeedRefuel)
        {
            dockType = DOCK_FOR_BOTHREFUELANDREPAIR;
        }
        else if (shipsNeedRepair)
        {
            dockType = DOCK_FOR_REPAIR;
        }
        else if (shipsNeedRefuel)
        {
            dockType = DOCK_FOR_REFUEL;
        }
        else
        {
            dockType = DOCK_FOR_REFUELREPAIR;
        }
    }

    return dockType;
}

/*-----------------------------------------------------------------------------
    Name        : clDock
    Description : Command to dock
    Inputs      : selectcom, dockType
    Outputs     :
    Return      : the number of ships that are actually docking
----------------------------------------------------------------------------*/
static void dockSetupShipForCommand(Ship *ship, Ship *dockTarget, DockType dockType)
{
    if (dockType == DOCK_MODE_STAY_TILL_EXPLICITLAUNCH ||
        dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH)
    {
        ship->specialFlags1C8 |= BIT12;
        if (dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH &&
            (dockTarget->shiptype == sHiveFrigate || dockTarget->shiptype == bHiveFrigate))
        {
            shipClearSwarmers(dockTarget);
        }
    }

    dockSetAvoidanceIgnoreTarget(ship, dockTarget);
    ship->dockvars.dockship = dockTarget;
    if (dockType == DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP)
    {
        ship->flags2 |= SOF2_BayDocked;
        ship->dockvars.dockLaunchState = 10;
    }
    else
    {
        if (ship->shiptype == bIonArrayFrigate || ship->shiptype == p1IonArrayFrigate)
        {
            if (ship->madGunStatus == 3 || ship->madGunStatus == 1)
            {
                madLinkInCloseGunsShip(ship);
            }
        }
        ship->dockvars.dockLaunchState = 0;
    }
    ship->dockvars.dockPointIndex = -1;
}

sdword clDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith)
{
    CommandToDo *newcommand;
    CommandToDo *alreadycommand;
    SelectCommand *selection;
    udword sizeofselection;
    sdword numShips;
    sdword i;
    Ship *ship;
    DockType dockMode = dockModeFromRequest(dockType);
    bool8 wasHarvesting = FALSE;

    numShips = selectcom->numShips;
    if (numShips == 0)
    {
        return 0;
    }

    sizeofselection = sizeofSelectCommand(numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",0);
    selection->numShips = 0;
    for (i=0;i<numShips;i++)
    {
        bool32 (*dockStart)(Ship *ship, sdword dockType, Ship *dockwith);

        ship = selectcom->ShipPtr[i];
        if (dockwith != NULL && !ThisShipCanDockWith(ship, dockwith, dockMode, FALSE))
        {
            continue;
        }

        dockStart = ship->staticinfo->custshipheader.CustShip_DockStart;
        if (dockStart != NULL && !dockStart(ship, dockMode, dockwith))
        {
            continue;
        }

        selection->ShipPtr[selection->numShips++] = ship;
    }

    if (selection->numShips == 0)
    {
        memFree(selection);
        return 0;
    }

    alreadycommand = IsSelectionAlreadyDoingSomething(comlayer,selection);
    if (alreadycommand != NULL)
    {
        if (alreadycommand->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            ClearProtecting(alreadycommand);
        }
        if (alreadycommand->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
        {
            ClearPassiveAttacking(alreadycommand);
        }
    }

    if (alreadycommand == NULL || alreadycommand->ordertype.order == COMMAND_COLLECTRESOURCE)
    {
        if (alreadycommand != NULL &&
            (dockMode == DOCK_MODE_REFUELREPAIR ||
             dockMode == DOCK_MODE_DEPOSIT_RESOURCES ||
             dockMode == DOCK_MODE_BEAST))
        {
            wasHarvesting = TRUE;
        }

        RemoveShipsFromDoingStuff(comlayer,selection);

        newcommand = memAlloc(sizeof(CommandToDo),"ToDo",0);
        InitCommandToDo(newcommand);
        newcommand->selection = selection;
        newcommand->ordertype.order = COMMAND_DOCK;

        dockCommandSetWasHarvesting(newcommand, wasHarvesting);
        dockCommandClearLegacyState(newcommand);
        dockCommandSetMode(newcommand, dockMode);
        dockCommandSetAllNearShip(newcommand, FALSE);
        dockCommandSetHaveFlyedFarEnoughAway(newcommand, FALSE);

        for (i=0;i<selection->numShips;i++)
        {
            Ship *dockTarget;

            ship = selection->ShipPtr[i];
            dockTarget = dockwith != NULL ? dockwith : FindNearestShipToDockAt(ship, dockMode);
            dockSetupShipForCommand(ship, dockTarget, dockMode);
        }

        dockCommandSetAllDockingWithSameShip(newcommand, allShipsDockingWithSameShip(selection));
        resetShipsAIStates(selection, TRUE);
        PrepareShipsForCommand(newcommand, TRUE);
        listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);
        return selection->numShips;
    }

    FreeLastOrder(alreadycommand);
    alreadycommand->ordertype.order = COMMAND_DOCK;
    dockCommandSetWasHarvesting(alreadycommand, FALSE);
    dockCommandClearLegacyState(alreadycommand);
    dockCommandSetMode(alreadycommand, dockMode);
    dockCommandSetAllNearShip(alreadycommand, FALSE);
    dockCommandSetHaveFlyedFarEnoughAway(alreadycommand, FALSE);

    for (i=0;i<selection->numShips;i++)
    {
        Ship *dockTarget;

        ship = selection->ShipPtr[i];
        dockTarget = dockwith != NULL ? dockwith : FindNearestShipToDockAt(ship, dockMode);
        dockSetupShipForCommand(ship, dockTarget, dockMode);
    }

    dockCommandSetAllDockingWithSameShip(alreadycommand, allShipsDockingWithSameShip(selection));
    resetShipsAIStates(alreadycommand->selection, TRUE);
    PrepareShipsForCommand(alreadycommand, TRUE);
    numShips = selection->numShips;
    memFree(selection);
    return numShips;
}

/*-----------------------------------------------------------------------------
    Name        : dockChangeSingleShipToDock
    Description : changes a command for a single ship to COMMAND_DOCK
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword dockChangeSingleShipToDock(struct CommandToDo *command, Ship *ship, Ship *dockship, bool32 wasHarvesting, DockType dockType)
{
    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        ClearProtecting(command);
    }
    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        ClearPassiveAttacking(command);
    }

    command->ordertype.order = COMMAND_DOCK;
    dockCommandSetWasHarvesting(command, wasHarvesting);
    dockCommandClearLegacyState(command);
    dockCommandSetMode(command, dockType);
    dockCommandSetAllDockingWithSameShip(command, TRUE);
    dockCommandSetAllNearShip(command, FALSE);
    dockCommandSetHaveFlyedFarEnoughAway(command, FALSE);

    if (dockType == DOCK_MODE_STAY_TILL_EXPLICITLAUNCH ||
        dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH)
    {
        ship->specialFlags1C8 |= BIT12;
        if (dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH &&
            (dockship->shiptype == sHiveFrigate || dockship->shiptype == bHiveFrigate))
        {
            shipClearSwarmers(dockship);
        }
    }

    dockSetAvoidanceIgnoreTarget(ship, dockship);
    ship->dockvars.dockship = dockship;
    if (dockType == DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP)
    {
        ship->flags2 |= SOF2_BayDocked;
        ship->dockvars.dockLaunchState = 10;
    }
    else
    {
        if (ship->shiptype == bIonArrayFrigate || ship->shiptype == p1IonArrayFrigate)
        {
            if (ship->madGunStatus == 3 || ship->madGunStatus == 1)
            {
                madLinkInCloseGunsShip(ship);
            }
        }
        ship->dockvars.dockLaunchState = 0;
    }
    ship->dockvars.dockPointIndex = -1;

    PrepareShipsForCommand(command, TRUE);

    return resetShipsAIStates(command->selection, TRUE);
}

sdword dockPrepareSingleShipForLaunch(Ship *ship, Ship *dockship, DockType dockType)
{
    if (dockType == DOCK_MODE_STAY_TILL_EXPLICITLAUNCH ||
        dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH)
    {
        ship->specialFlags1C8 |= BIT12;
        if (dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH &&
            (dockship->shiptype == sHiveFrigate || dockship->shiptype == bHiveFrigate))
        {
            shipClearSwarmers(dockship);
        }
    }

    dockSetAvoidanceIgnoreTarget(ship, dockship);
    ship->dockvars.dockship = dockship;
    if (dockType == DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP)
    {
        ship->flags2 |= SOF2_BayDocked;
        ship->dockvars.dockLaunchState = 10;
    }
    else
    {
        if (ship->shiptype == bIonArrayFrigate || ship->shiptype == p1IonArrayFrigate)
        {
            if (ship->madGunStatus == 3 || ship->madGunStatus == 1)
            {
                madLinkInCloseGunsShip(ship);
            }
        }
        ship->dockvars.dockLaunchState = 0;
    }
    ship->dockvars.dockPointIndex = -1;
    return 0;
}

void RemoveShipFromDocking(Ship *ship)
{
    bitClear(ship->specialFlags,SPECIAL_STAY_TILL_EXPLICITLAUNCH);
    ship->specialFlags1C8 &= ~BIT24;
    if(ship->clampInfo != NULL)
    {
        if(shipDockingTarget(ship) == (Ship *)ship->clampInfo->host)
        {
            //host is docking ships...so unclamp
            //won't unclamp unless it is the right ship
            unClampObj((SpaceObjRotImpTargGuidanceShipDerelict *)ship);
        }
    }

    if (ship->dockvars.dockship != NULL)
    {
        dockUnbusyThisShip(ship);
    }
    dockClearDockingTarget(ship);
    dockFreeDockPoint(ship);
    dockSetAvoidanceIgnoreTarget(ship, NULL);
    ship->dockvars.dockship = NULL;
    ship->dockvars.dockstaticpoint = NULL;
    if (ship->dockvars.customdockinfo)
    {
        ship->dockvars.customdockinfo = NULL;
    }
    madLinkInPostDockingShip(ship);
}

void RemoveShipFromLaunching(Ship *ship)
{
    bitClear(ship->specialFlags,SPECIAL_STAY_TILL_EXPLICITLAUNCH);
    ship->specialFlags1C8 &= ~CATA_RUNTIME_KAS_CHECK_DONE_LAUNCHING;
    bitClear(ship->specialFlags,SPECIAL_Launching);
    dockClearDockingTarget(ship);
    dockFreeDockPoint(ship);
    dockSetAvoidanceIgnoreTarget(ship, NULL);
    ship->dockvars.dockship = NULL;
    ship->dockvars.dockstaticpoint = NULL;
    if (ship->dockvars.customdockinfo != NULL)
    {
        ship->dockvars.customdockinfo = NULL;
    }
    madLinkInPostDockingShip(ship);

    battleLaunchWelcomeExchange(ship);
}

void LaunchCleanup(struct CommandToDo *launchtodo)
{
    Ship *ship;

    if (launchtodo->selection->numShips == 0)
    {
        return;     // don't need to do cleanup if there are no ships (e.g. they have been removed)
    }

    ship = launchtodo->selection->ShipPtr[0];

    dbgAssert(launchtodo->selection->numShips == 1);
    RemoveShipFromLaunching(ship);
}

sdword dockNeedRotationOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused)
{
    sdword i;
    if(dockwith->staticinfo->dockOverideInfo != NULL)
    {
        for(i=0;i<dockwith->staticinfo->dockOverideInfo->numDockOverides;i++)
        {
            if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiptype == ship->shiptype)
            {
                //this is the overide slot for the ship
                if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiprace == ship->shiprace)
                {
                    //correct race?
                    if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].useNewOrientation==1)
                    {
                        //use the offset overide for this one
                        if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightNameUsed)
                        {
                            //light specific
                            if(strcmp(dockpointused->name,dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightName)==0)
                            {
                                return i;
                            }
                        }
                        else
                        {
                            //light name NOT used so this is good enough
                            return i;
                        }
                    }
                }
            }
        }
    }
    return(-1);
}
sdword dockNeedOffsetOveride(Ship *dockwith,Ship *ship,DockStaticPoint *dockpointused)
{
    sdword i;
    if(dockwith->staticinfo->dockOverideInfo != NULL)
    {
        for(i=0;i<dockwith->staticinfo->dockOverideInfo->numDockOverides;i++)
        {
            if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiptype == ship->shiptype)
            {
                //this is the overide slot for the ship
                if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].shiprace == ship->shiprace)
                {
                    //correct race
                    if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].useNewOffset==1)
                    {
                        //use overide slot
                        if(dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightNameUsed)
                        {
                            //light specific
                            if(strcmp(dockpointused->name,dockwith->staticinfo->dockOverideInfo->dockOverides[i].lightName)==0)
                            {
                                //light name is used and this is the one
                                return i;
                            }
                        }
                        else
                        {
                            //don't need light name
                            return i;
                        }
                    }
                }
            }
        }
    }
    return(-1);
}


/*-----------------------------------------------------------------------------
    Name        : DockCleanup
    Description : cleans up refueling state information
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void DockCleanup(struct CommandToDo *docktodo)
{
    sdword i;
    SelectCommand *selection = docktodo->selection;
    sdword numShips = selection->numShips;

    for (i=0;i<numShips;i++)
    {
        dockWorkerClampFunc(selection->ShipPtr[i]);
    }
    for (i=0;i<numShips;i++)
    {
        madLinkInOpenDoor(selection->ShipPtr[i]);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ClampToWithin
    Description : returns number clamped to within min and max
    Inputs      : number, min, max
    Outputs     :
    Return      : returns number clamped to within min and max
----------------------------------------------------------------------------*/
real32 ClampToWithin(real32 number,real32 min,real32 max)
{
    if (number < min)
    {
        return min;
    }
    else if (number > max)
    {
        return max;
    }
    else
    {
        return number;
    }
}

void dockUnbusyThisShip(Ship *ship)
{
    Ship *busyShip = ship->dockvars.dockship;

    if (busyShip != NULL && busyShip->dockInfo != NULL && busyShip->dockInfo->busyness > 0)
    {
        busyShip->dockInfo->busyness--;
        dbgAssert(busyShip->dockInfo->busyness >= 0);
    }
}

void dockBusyThisShip(Ship *ship,Ship *dockwith)
{
    (void)ship;
    if (dockwith != NULL && dockwith->dockInfo != NULL)
    {
        dockwith->dockInfo->busyness++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : dockReserveDockPoint
    Description : reserves a docking point for use
    Inputs      : ship, dockwith, dockpointindex
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void dockReserveDockPoint(Ship *ship, Ship *dockwith, sdword dockpointindex)
{
    DockStaticInfo *dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    DockPointActivity *activity = dockwith->dockvars.dockPointActivity;
    sdword pointType;

    dbgAssert(dockpointindex >= 0);
    dbgAssert(dockStaticInfo != NULL);
    dbgAssert(dockpointindex < dockStaticInfo->numDockPoints);

    pointType = (sdword)dockStaticInfo->dockstaticpoints[dockpointindex].type;

    ship->dockvars.dockPointIndex = dockpointindex;
    activity->reserved[dockpointindex] = 1;
    activity->numDockedOfType[pointType]++;
}

void dockPutShipInside(Ship *ship,Ship *dockwith)
{
    ShipsInsideMe *shipsInsideMe = dockwith->shipsInsideMe;
    Node *node;
    InsideShip *insideShip;

    dbgAssert(shipsInsideMe);

    for (node = shipsInsideMe->insideList.head; node != NULL; node = node->next)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        if (insideShip->ship == ship)
        {
            return;
        }
    }

    insideShip = memAlloc(sizeof(InsideShip),"InsideShip",0);
    insideShip->ship = ship;
    listAddNode(&shipsInsideMe->insideList,&insideShip->node,insideShip);
    lmUpdateShipsInside();
    if (ship->staticinfo->shipclass == CLASS_Fighter ||
        ship->staticinfo->shipclass == CLASS_Micro)
    {
        shipsInsideMe->FightersInsideme++;
    }
    else if (ship->staticinfo->shipclass == CLASS_Corvette ||
             ship->staticinfo->shipclass == CLASS_Resource)
    {
        shipsInsideMe->CorvettesInsideme++;
    }
}

void dockInitShipForLaunch(Ship *ship)
{
    ShipLaunchInitSpecificsOverlay *launchSpecifics = shipLaunchInitSpecifics(ship);
    sdword shiptype = ship->shiptype;

    launchSpecifics->receiverShip = ship;
    launchSpecifics->state = 0;
    launchSpecifics->unk_0C = 0;

    if (shiptype == sACV || shiptype == bACV)
    {
        speechEventQueue(0, 67111069, shiptype, -1, 0, ship->playerowner->playerIndex, -1, 5.0f, -1);
        GetDirectionVectorOfShip(&launchSpecifics->primaryDirection, 2, &ship->rotinfo.coordsys);
        GetDirectionVectorOfShip(&launchSpecifics->secondaryDirection, 0, &ship->rotinfo.coordsys);
        return;
    }

    speechEventQueue(ship, 67111069, shiptype, -1, -1, -1, -1, 0.0f, -1);
    GetDirectionVectorOfShip(&launchSpecifics->primaryDirection, 2, &ship->rotinfo.coordsys);
    GetDirectionVectorOfShip(&launchSpecifics->secondaryDirection, 5, &ship->rotinfo.coordsys);
    dockDisableShipForLaunch(ship, 1);
}

void dockRemoveShipFromInside(Ship *ship,Ship *dockwith)
{
    ShipsInsideMe *shipsInsideMe = dockwith->shipsInsideMe;
    Node *node = shipsInsideMe->insideList.head;
    InsideShip *insideShip;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);

        if (insideShip->ship == ship)
        {
            listDeleteNode(node);
            lmUpdateShipsInside();
            if (ship->staticinfo->shipclass == CLASS_Fighter ||
                ship->staticinfo->shipclass == CLASS_Micro)
            {
                shipsInsideMe->FightersInsideme--;
            }
            else if (ship->staticinfo->shipclass == CLASS_Corvette ||
                     ship->staticinfo->shipclass == CLASS_Resource)
            {
                shipsInsideMe->CorvettesInsideme--;
            }

            return;
        }

        node = node->next;
    }
    //dbgAssert(FALSE);
    dbgMessagef("\nWarning: ship %d not inside ship %d but expected",ship->shipID.shipNumber,dockwith->shipID.shipNumber);
}

// Handles the launch state machine for a single ship leaving a carrier.
// State progression for a standard launch (launchType == 2):
//   8 (docked, waiting) -> 10 (find exit point, put outside) -> 12 (fly away) -> TRUE (done)
// Returns TRUE when the ship has fully launched, FALSE to be called again next frame.

bool32 processSingleLaunchShipToDo(Ship *ship, Ship *receiverShip, sdword launchType)
{
    vector createat;
    ShipsInsideMe *shipsInsideMe;
    sdword state = ship->dockvars.dockLaunchState;
restart:
    switch (state)
    {
    case 0:
        {
            if (launchType == DOCK_MODE_INSTANT_REFUELREPAIR ||
                launchType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH)
            {
                univRemoveShipFromOutside(ship);
                dockPutShipInside(ship, receiverShip);
                if (receiverShip->shiptype != dRKO && (ship->flags2 & 8) != 0)
                {
                    shipResetModuleSlots(ship);
                }
                return TRUE;
            }

            ShipsInsideMe *insideMe = receiverShip->shipsInsideMe;
            bool32 withinDockRange;
            bool32 foundDockPoint = FALSE;
            vector dockTrajectory;
            real32 dockRange;
            real32 dockThreshold;
            if (insideMe != NULL)
            {
                Node *insideNode = insideMe->insideList.head;
                while (insideNode != NULL)
                {
                    InsideShip *insideEntry = (InsideShip *)listGetStructOfNode(insideNode);
                    if (insideEntry->ship == ship)
                    {
                        state = 8;
                        ship->dockvars.dockLaunchState = 8;
                        ship->dockvars.dockStateDeadline = universe.totaltimeelapsed;
                        goto restart;
                    }
                    insideNode = insideNode->next;
                }
            }

            aishipGetTrajectory(ship, (SpaceObjRotImpTarg *)receiverShip, &dockTrajectory);
            dockRange = RangeToTarget(ship, (SpaceObjRotImpTarg *)receiverShip, &dockTrajectory);
            dockThreshold = receiverShip->staticinfo->dockShipRange +
                            receiverShip->staticinfo->staticheader.staticCollInfo.avoidcollspheresize;
            withinDockRange = ShipWithinDockRange(ship, receiverShip);
            if (withinDockRange)
            {
                foundDockPoint = dockFindNearestDockPoint(ship, receiverShip, launchType);
                if (foundDockPoint &&
                    launchType == DOCK_MODE_BEAST &&
                    dockIsSlipgateShip(receiverShip))
                {
                    slipgateAddCaptureTarget(receiverShip, ship);
                }
            }
            if (withinDockRange && foundDockPoint)
            {
                ship->dockvars.dockLaunchState = 2;
                return FALSE;
            }

            if (!withinDockRange)
            {
                aishipFlyToPointAvoidingObjsFunc(ship, &receiverShip->posinfo.position, 58, 0.0f, NULL);
            }
            else
            {
                aitrackForceSteadyShip(ship);
            }
        }
        return FALSE;

    case 1:
        if (!dockFlyToDockPoint(ship, receiverShip))
            return FALSE;
        ship->dockvars.dockLaunchState = 2;
        return FALSE;

    case 2:
        {
            DockStaticInfo *dockStaticInfo = receiverShip->staticinfo->dockStaticInfo;
            DockStaticPoint *dockstaticpoint;
            bool32 coneReached;

            dbgAssert(dockStaticInfo != NULL);
            dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];
            if (dockIsSlipgateShip(receiverShip))
            {
                slipgateActivateApproach(receiverShip);
            }
            coneReached = dockFlyToConeInside(ship, receiverShip, dockstaticpoint);
            if (!coneReached)
            {
                return FALSE;
            }
            ship->flags |= SOF_AvoidanceHidden;
            ship->specialFlags2 |= 0x40u;

            if (dockNeedsSpecialDocking(ship, launchType))
            {
                ship->dockvars.dockLaunchState = 6;
            }
            else
            {
                ship->dockvars.dockLaunchState = 5;
                matMultiplyMatByVec(&ship->lastDockApproachHeading,
                                    &receiverShip->rotinfo.coordsys,
                                    &dockstaticpoint->conenormal);
            }
            return FALSE;
        }

    case 5:
    {
        DockStaticInfo *dockStaticInfo = receiverShip->staticinfo->dockStaticInfo;
        DockStaticPoint *dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];

        if (dockIsSlipgateShip(receiverShip))
        {
            slipgateActivateApproach(receiverShip);
        }
        if (!dockFlyToFinalDockPoint(ship, receiverShip, 4.0f))
        {
            return FALSE;
        }

        if (dockIsSlipgateShip(receiverShip))
        {
            if (!slipgateActivateApproach(receiverShip))
            {
                aitrackSteadyShip(ship);
                return FALSE;
            }

            seekdir();
            ship->dockvars.dockLaunchState = 16;
            ship->dockStateStartTime = universe.totaltimeelapsed;
            dockPrepareContainedShipsForSlipgateTransit(ship);
            univNotifyShipDied(ship);
            ship->specialFlags |= 0x100;
            if (ship->dockvars.dockPointIndex != -1)
            {
                dockMarkPointFreed(receiverShip, ship->dockvars.dockPointIndex);
                ship->dockvars.dockPointIndex = -1;
            }
            return FALSE;
        }

        univRemoveShipFromOutside(ship);
        if (launchType == 7)
        {
            // Retail does not actually play speech on this retire path; this raw status ID
            // queues an unplayable event in our build (event 64 with shiptype as variable).
            // Keep the retire-side bookkeeping, but do not enqueue the dead speech event.
            /* speechEventQueue(0, 33558592, ship->shiptype, -1, 0, ship->playerowner->playerIndex, -1, 5.0f, -1); */
            if (ship->shiprace == 1)
            {
                rmRecomputeShipPrereqMask(ship);
            }
        }
        else
        {
            dockPutShipInside(ship, receiverShip);
            ship->dockvars.dockLaunchState = 8;
            ship->dockvars.dockStateDeadline = universe.totaltimeelapsed;
        }
        if (!(receiverShip->shiptype == sCarrier &&
              (dockstaticpoint->type == DPT_Med_In || dockstaticpoint->type == DPT_Med_Out)))
        {
            dockMarkPointFreed(receiverShip, ship->dockvars.dockPointIndex);
            ship->dockvars.dockPointIndex = -1;
        }
        if (launchType == DOCK_MODE_RETIRE)
        {
            dockCommandSetWasHarvesting(ship->command, FALSE);
            return TRUE;
        }
        return FALSE;
    }

    case 6:
    {
        DockStaticInfo *dockStaticInfo = receiverShip->staticinfo->dockStaticInfo;
        DockStaticPoint *dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];
        if (!dockFlyToClampedConePosition(ship, receiverShip, dockstaticpoint))
            return FALSE;
        switch (ship->shiptype)
        {
        case sWorker:
        case bWorker:
            if (*(udword *)(ship->runtimeAnimationOverlay.madAnimationData + 12) != 3)
            {
                madAnimationStart(ship, 3);
                madAnimationPause(ship, 1);
            }
            break;
        default:
            break;
        }
        ship->dockvars.dockLaunchState = 7;
        return FALSE;
    }

    case 7:
    {
        DockStaticInfo *shipDockInfo = ship->staticinfo->dockStaticInfo;
        DockStaticPoint *dockOffsetPoint = shipDockInfo ? (DockStaticPoint *)(uintptr_t)shipDockInfo->pad_4C : NULL;
        real32 *dockOffset = (real32 *)dockOffsetPoint;
        DockStaticInfo *dockStaticInfo = receiverShip->staticinfo->dockStaticInfo;
        DockStaticPoint *dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];
        if (!dockFlyToOffsetDockPoint(ship, dockOffset, receiverShip, dockstaticpoint))
            return FALSE;
        ship->dockvars.dockStateDeadline = universe.totaltimeelapsed;
        if ((double)ship->resources > 0.0)
            ship->dockvars.dockStateDeadline += 4.0f;
        if ((double)ship->health < (double)ship->staticinfo->maxhealth)
            ship->dockvars.dockStateDeadline += 6.0f;
        clampObjectSetup((SpaceObj *)ship, (SpaceObj *)receiverShip);
        ship->dockvars.dockLaunchState = 9;
        if (battleCanChatterAtThisTime(74, ship))
        {
            if ((double)ship->resources <= 0.0)
                battleChatterAttempt(-1, 74, ship, *(sdword *)&ship->staticinfo->forceFieldProtectionDistanceScale);
            else
                battleChatterAttempt(-1, 83, ship, *(sdword *)&ship->staticinfo->forceFieldProtectionDistanceScale);
        }
        if (ship->shiptype == bWorker || ship->shiptype == sWorker)
            soundEventPlay(ship, 2055, 0);
        return FALSE;
    }

    case 8:
        // Fully docked, waiting for timer/launch trigger
        if (universe.totaltimeelapsed <= (double)ship->dockvars.dockStateDeadline)
            return FALSE;
        {
            bool32 autoLaunchEnabled = lmShipAutoLaunchEnabled(receiverShip);
            bool32 forceLaunch = ShipHasToLaunch(receiverShip, ship);
            if ((autoLaunchEnabled || forceLaunch)
            && !(ship->specialFlags1C8 & BIT12))
            {
                if (launchType != DOCK_MODE_DEPOSIT_RESOURCES)
                {
                    dbgAssert(ship->command != NULL);
                    dockCommandSetWasHarvesting(ship->command, FALSE);
                }
                state = 10;
                ship->dockvars.dockLaunchState = 10;
                goto restart;
            }
            else
            {
                dockCommandSetWasHarvesting(ship->command, FALSE);
                return TRUE;  // no launch needed, stay docked
            }
        }

    case 9:
        if (universe.totaltimeelapsed <= (double)ship->dockvars.dockStateDeadline
            || (ship->specialFlags3 & SPECIAL3_Unknown0) != 0)
            return FALSE;
        switch (launchType)
        {
        case 0: case 1: case 2: case 3: case 7: case 8: case 9:
            switch (ship->shiptype)
            {
            case sLeech:
            case sWorker:
            case bWorker:
            case tResourceCollector:
            {
                sdword resources = ship->resources;
                ship->resources = 0;
                soundEventPlay(ship, 2052, 0);
                {
                    Player *playerowner = ship->playerowner;
                    if (resources > 0)
                    {
                        playerowner->resourceUnits += resources;
                        universe.gameStats.playerStats[playerowner->playerIndex].totalResourceUnitsCollected += resources;
                        universe.gameStats.playerStats[playerowner->playerIndex].totalResourceUnits += resources;
                        universe.gameStats.totalResourceUnitsCollected += resources;
                        if (singlePlayerGame && !playerowner->playerIndex)
                            singlePlayerGameInfo.resourceUnitsCollected += resources;
                    }
                }
                break;
            }
            default:
                break;
            }
            break;
        default:
            break;
        }
        clDetachShipFromCommand(ship);
        ship->dockvars.dockLaunchState = 12;
        return FALSE;

    case 10:
        // Find an exit dock point and put ship outside
        {
        DockStaticInfo *dockStaticInfo;
        DockStaticPoint *dockstaticpoint;

        if (receiverShip->specialFlags1C8 & BIT21)
        {
            return FALSE;
        }
        if (!dockFindAndReserveDockPoint(ship, receiverShip))
        {
            return FALSE;
        }

        dockStaticInfo = receiverShip->staticinfo->dockStaticInfo;
        dbgAssert(dockStaticInfo != NULL);
        dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];

        ship->flags |= SOF_AvoidanceHidden;
        ship->specialFlags2 |= 0x40u;

        // Transform dock exit position to world space
        matMultiplyMatByVec(&createat, &receiverShip->rotinfo.coordsys, &dockstaticpoint->position);
        vecAddTo(createat, receiverShip->posinfo.position);

        // Remove from insideList before putting outside
        shipsInsideMe = receiverShip->shipsInsideMe;
        if (shipsInsideMe != NULL)
            dockRemoveShipFromInside(ship, receiverShip);

        dockPutShipOutside(ship, receiverShip, &createat, dockstaticpoint->headingdirection, dockstaticpoint->updirection);
        if (receiverShip->shiptype == dSlipgateSmall
            || receiverShip->shiptype == dSlipgateMedium
            || receiverShip->shiptype == dSlipgateLarge)
        {
            // Slipgate exit -> drift wait state
            ship->dockvars.dockLaunchState = 17;
            ship->dockStateStartTime = universe.totaltimeelapsed;
        }
        else if (ship->specialFlags2 & SPECIAL_2_DockFlag)
        {
            // Clamped component launch (module being built)
            SelectCommand selectOne;

            dbgAssert(ship->shiptype != bCruiseMissile);
            ship->dockvars.dockLaunchState = 11;
            clampObjectSetup((SpaceObj *)ship, (SpaceObj *)ship->dockvars.dockstaticpoint);
            clSetAttribute((sdword)ship, 1, 4);
            if (ship->playerowner->aiPlayer != NULL)
            {
                selectOne.numShips = 1;
                selectOne.ShipPtr[0] = ship;
                clWrapSetShipDisabled(&universe.mainCommandLayer, &selectOne);
            }
            else
            {
                cmTrackExternalBuildShip(ship);
            }
        }
        else
        {
            ship->dockvars.dockLaunchState = 12;
            if (ship->staticinfo->providesSU || shipIsAliveCarrierType(ship))
                shipApplyPlayerUpgrades(ship);
            acolyteStartLinking(ship);
            acvInitSpecial(ship);
            ship->health = ship->staticinfo->maxhealth;
            ship->specialFlags1C8 &= ~BIT7;
        }
        return FALSE;
        }

    case 11:
        if (ship->specialFlags2 & SPECIAL_2_DockFlag)
        {
            dbgAssert(ship->shiptype != bCruiseMissile);
            return FALSE;
        }

        shipAddSupportCapacity(ship);
        if (ship->staticinfo->shipclass == CLASS_Component)
        {
            return TRUE;
        }

        clDetachShipFromCommand(ship);
        clSetAttribute((sdword)ship, 0, 4);
        if (ship->shiptype == sDreadnought)
        {
            dreadnoughtReattachCannon(ship);
        }
        dbgAssert(ship->shiptype != bCruiseMissile);
        ship->dockvars.dockLaunchState = 12;
        if (ship->staticinfo->canBuildShips || ship->staticinfo->canBuildBigShips)
        {
            cmAddFactory(ship, ship->staticinfo->canBuildBigShips);
        }
        return FALSE;

    case 12:
        // Fly away from carrier's cone exit point
        if (!dockFlyToConeOrigin(ship, receiverShip))
        {
            return FALSE;
        }
        if (dockNeedsSpecialDocking(ship, launchType))
        {
            switch (ship->shiptype)
            {
            case sWorker:
            case bWorker:
                if (*(udword *)(ship->runtimeAnimationOverlay.madAnimationData + 12) != 2)
                    madAnimationStart(ship, 2);
                break;
            default:
                break;
            }
        }
        return TRUE;

    case 13:
        aitrackSteadyShipDriftOnly(ship);
        slipgateActivateApproach(receiverShip);
        if (universe.totaltimeelapsed <= (double)ship->dockvars.dockStateDeadline)
        {
            return FALSE;
        }
        univRemoveShipFromOutside(ship);
        dockPutShipInside(ship, slipgateGetPartner(receiverShip));
        dockSetAvoidanceIgnoreTarget(ship, slipgateGetPartner(receiverShip));
        ship->dockvars.dockship = slipgateGetPartner(receiverShip);
        dbgAssert(ship->dockvars.dockship != NULL);
        ship->dockvars.dockLaunchState = 10;
        slipgateMarkPlayerTransit(receiverShip, ship->playerowner->playerIndex);
        slipgateMarkPlayerTransit(ship->dockvars.dockship, ship->playerowner->playerIndex);
        return FALSE;

    case 14:
        aitrackSteadyShipDriftOnly(ship);
        slipgateActivateApproach(receiverShip);
        if (universe.totaltimeelapsed <= (double)ship->dockvars.dockStateDeadline)
        {
            return FALSE;
        }
        seekdir();
        ship->dockvars.dockLaunchState = 12;
        return FALSE;

    case 16:
    case 17:
        aitrackSteadyShipDriftOnly(ship);
        slipgateActivateApproach(receiverShip);
        if (ship->dockStateStartTime + DOCK_SLIPGATE_TRANSIT_DELAY >= universe.totaltimeelapsed)
        {
            return FALSE;
        }
        ship->dockvars.dockLaunchState = ship->dockvars.dockLaunchState != 16 ? 14 : 13;
        return FALSE;

    default:
        return FALSE;
    }
}

void dockPutShipOutside(Ship *ship,Ship *creator,vector *createat,udword headingdirection,udword updirection)
{
    vector heading;
    vector up;
    vector right;
    DockStaticInfo *dockStaticInfo;
    sdword dockIndex;
    sdword launchSlotIndex;
    sdword i;
    Node *insideNode;
    InsideShip *insideShip;

    dbgAssert(creator);
    if (creator->collMyBlob != NULL)
    {
        bobCalcObjDistAndAdd(creator->collMyBlob, (SpaceObj *)ship);
    }
    else
    {
        collAddSpaceObjToCollBlobs((SpaceObj *)ship);
    }
    listAddNode(&universe.SpaceObjList,&(ship->objlink),ship);
    listAddNode(&universe.ShipList,&(ship->shiplink),ship);
    listAddNode(&universe.ImpactableList,&(ship->impactablelink),ship);
    bitClear(ship->flags, SOF_DockMarker);

    bitClear(ship->flags, SOF_Hide);

    univAddObjToRenderListIf((SpaceObj *)ship,(SpaceObj *)creator);     // add to render list if parent ship is in render list
    if (ship->shipsInsideMe != NULL)
    {
        insideNode = ship->shipsInsideMe->insideList.head;
        while (insideNode != NULL)
        {
            insideShip = (InsideShip *)listGetStructOfNode(insideNode);
            univAddObjToRenderListIf((SpaceObj *)insideShip->ship, (SpaceObj *)creator);
            bitClear(insideShip->ship->flags, SOF_DockMarker);
            insideNode = insideNode->next;
        }
    }
    ship->posinfo.position = *createat;
    ship->posinfo.velocity = creator->posinfo.velocity;

    dbgAssert(headingdirection != updirection);
    GetDirectionVectorOfShip(&heading,headingdirection,&creator->rotinfo.coordsys);
    GetDirectionVectorOfShip(&up,updirection,&creator->rotinfo.coordsys);
    vecCrossProduct(right,heading,up);   // should not have to normalize because heading, up are unit vectors and angle between them is 90 degrees.
    matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&right,&heading);

    univUpdateShipProximity(ship);

    dockStaticInfo = creator->staticinfo->dockStaticInfo;
    dockIndex = ship->dockvars.dockPointIndex;
    if (dockStaticInfo != NULL &&
        dockIndex >= 0 &&
        dockIndex < dockStaticInfo->numDockPoints)
    {
        DockPointType pointType = dockStaticInfo->dockstaticpoints[dockIndex].type;

        launchSlotIndex = dockIndex - dockStaticInfo->firstPointOfType[pointType];
        if (ship->shiptype == bMothershipSupport)
        {
            if ((launchSlotIndex & 2) != 0)
            {
                dockApplyLaunchOrientationRotation(ship, 3.14159274101257f, matMakeRotAboutY);
                dockApplyLaunchOrientationRotation(
                    ship,
                    (launchSlotIndex & 1) != 0 ? -0.122173f : 0.122173f,
                    matMakeRotAboutZ);
            }
            if ((launchSlotIndex & 1) != 0)
            {
                dockApplyLaunchOrientationRotation(ship, 3.14159274101257f, matMakeRotAboutX);
            }
        }
        else if (ship->shiptype == sCarrierSupport || ship->shiptype == sMothershipSupport)
        {
            if ((launchSlotIndex & 1) != 0)
            {
                dockApplyLaunchOrientationRotation(ship, 3.14159274101257f, matMakeRotAboutX);
            }
        }
    }

    univUpdateObjRotInfo((SpaceObjRot *)ship);

    for (i = 0; i < MAX_NUM_TRAILS; i++)
    {
        if (ship->trail[i])
        {
            trailZeroLength(ship->trail[i]);
        }
    }
}

void dockMakeNewStaticInfoForMaster(Ship *master)
{
    dbgAssert(!(master->flags & SOF_StaticInfoIsDynamicCata));


    master->staticinfo = memAlloc(sizeof(ShipStaticInfo),"ShipStaticInfo",0);
    *((ShipStaticInfo *)(master->staticinfo)) = *GetShipStaticInfo(master->shiptype,master->shiprace);
        //asteroidStaticInfos[asteroid->asteroidtype];  // structure copy
    master->flags |= SOF_StaticInfoIsDynamicCata;

}

void dockFreeMasterStaticMem(Ship *master)
{
    if (master->flags & SOF_StaticInfoIsDynamicCata)
    {
        memFree(master->staticinfo);
        bitClear(master->flags, SOF_StaticInfoIsDynamicCata);
    }
    //here regenerate static info...not yet though
    master->staticinfo = GetShipStaticInfo(master->shiptype,master->shiprace);
    //should probably disperse damages to all slaves...but not now...
    //don't envision the need to break ships apart
}


/*-----------------------------------------------------------------------------
    Name        : processLinkedLaunchToDo
    Description : processes a COMMAND_LINKED_LAUNCH command (retail order 12);
                  handles Beast ACV/MCV fission (splitting into two acolytes)
                  and the post-fission mirrored position/rotation sync between
                  the two halves until they separate far enough / are ready.
                  Retail dispatches this from clLaunchShipSelection, which is
                  distinct from a normal COMMAND_DOCK order.
    Inputs      :
    Outputs     :
    Return      : returns TRUE if done
----------------------------------------------------------------------------*/
static bool32 dockShipUsesLinkedFission(const Ship *ship)
{
    return ship->shiptype == sACV || ship->shiptype == sMCV || ship->shiptype == bACV;
}

static void dockInitLinkedFissionChildren(Ship *firstChild, Ship *secondChild)
{
    AcolyteSpecifics *firstSpecifics = acolyteShipSpecifics(firstChild);
    AcolyteSpecifics *secondSpecifics = acolyteShipSpecifics(secondChild);

    firstSpecifics->linkPartner = secondChild;
    firstSpecifics->linkAnimationIndex = -1;
    secondSpecifics->linkPartner = firstChild;
    secondSpecifics->linkState = 1;
    secondSpecifics->linkAnimationIndex = -1;
    // retail (0x468650) initializes Ship::linkTransformTimer, not a ShipSpecifics-overlay
    // timer; confirmed via disassembly (lea ecx/edx,[reg+624h] ahead of TimerInitTimerStruct)
    TimerInitTimerStruct(&firstChild->linkTransformTimer, 3.0f);
    TimerInitTimerStruct(&secondChild->linkTransformTimer, 3.0f);
}

static bool32 dockProcessLinkedFission(CommandToDo *docktodo, sdword selectionIndex, bool32 *allDone)
{
    SelectCommand *selection = docktodo->selection;
    Ship *linkedShip = selection->ShipPtr[selectionIndex];
    ShipLaunchInitSpecificsOverlay *launchSpecifics;
    ACVSpecifics *linkedSpecifics;
    ShipType childType;
    sdword animationIndex;
    Ship *firstChild;
    Ship *secondChild;
    vector heading;
    vector right;
    vector up;
    vector firstPosition;
    vector secondPosition;
    matrix childMatrix;
    real32 splitDistance;
    real32 sharedHealthScale;
    sdword i;

    if (!dockShipUsesLinkedFission(linkedShip))
    {
        return FALSE;
    }

    launchSpecifics = shipLaunchInitSpecifics(linkedShip);
    if (launchSpecifics->receiverShip == NULL)
    {
        return FALSE;
    }

    if (!aitrackHeadingAndUp(linkedShip, &launchSpecifics->primaryDirection, &launchSpecifics->secondaryDirection, 0.99959999f))
    {
        *allDone = FALSE;
        return TRUE;
    }

    switch (linkedShip->shiptype)
    {
        case sACV:
            childType = sAcolyte;
            animationIndex = 3;
            break;

        case sMCV:
            childType = sMimic;
            animationIndex = 1;
            if (shipModuleSlotRuntime(linkedShip)->state != ShipModuleSlotState_Disabled &&
                shipModuleSlotRuntime(linkedShip)->state != ShipModuleSlotState_Destroyed)
            {
                *allDone = FALSE;
                return TRUE;
            }
            break;

        case bACV:
            childType = bAcolyte;
            animationIndex = 3;
            break;

        default:
            return FALSE;
    }

    firstChild = univCreateShip(childType, linkedShip->shiprace, &linkedShip->posinfo.position, linkedShip->playerowner, TRUE);
    cat_unitCapCreateShip(firstChild, firstChild->playerowner);
    memcpy(&firstChild->posinfo, &linkedShip->posinfo, 0x28);

    secondChild = univCreateShip(childType, linkedShip->shiprace, &linkedShip->posinfo.position, linkedShip->playerowner, TRUE);
    cat_unitCapCreateShip(secondChild, secondChild->playerowner);
    memcpy(&secondChild->posinfo, &linkedShip->posinfo, 0x28);

    heading.x = linkedShip->rotinfo.coordsys.m13;
    heading.y = linkedShip->rotinfo.coordsys.m23;
    heading.z = linkedShip->rotinfo.coordsys.m33;
    matCreateCoordSysFromHeading(&childMatrix, &heading);

    right.x = childMatrix.m11;
    right.y = childMatrix.m21;
    right.z = childMatrix.m31;
    up.x = childMatrix.m12;
    up.y = childMatrix.m22;
    up.z = childMatrix.m32;

    splitDistance = ShipStaticInfoGetLinkDistanceValue(firstChild->staticinfo);

    matCreateMatFromVecs(&firstChild->rotinfo.coordsys, &right, &up, &heading);
    firstPosition.x = linkedShip->posinfo.position.x + splitDistance * right.x;
    firstPosition.y = linkedShip->posinfo.position.y + splitDistance * right.y;
    firstPosition.z = linkedShip->posinfo.position.z + splitDistance * right.z;
    posSetShipPosition(firstChild, &firstPosition);

    right.x = -right.x;
    right.y = -right.y;
    right.z = -right.z;
    up.x = -up.x;
    up.y = -up.y;
    up.z = -up.z;
    matCreateMatFromVecs(&secondChild->rotinfo.coordsys, &right, &up, &heading);
    secondPosition.x = linkedShip->posinfo.position.x - splitDistance * childMatrix.m11;
    secondPosition.y = linkedShip->posinfo.position.y - splitDistance * childMatrix.m21;
    secondPosition.z = linkedShip->posinfo.position.z - splitDistance * childMatrix.m31;
    posSetShipPosition(secondChild, &secondPosition);

    univUpdateObjRotInfo((SpaceObjRot *)firstChild);
    univAddObjToRenderListIf((SpaceObj *)firstChild, (SpaceObj *)linkedShip);
    univUpdateObjRotInfo((SpaceObjRot *)secondChild);
    univAddObjToRenderListIf((SpaceObj *)secondChild, (SpaceObj *)linkedShip);

    if (ShipInSelection((SelectCommand *)&selSelected, linkedShip))
    {
        selSelectionAddSingleShipNoCheck((MaxSelection *)&selSelected, firstChild);
        selSelectionAddSingleShipNoCheck((MaxSelection *)&selSelected, secondChild);
        ioUpdateShipTotals();
    }

    if (ShipInSelection(&universe.mainCameraCommand.currentCameraStack->focus, linkedShip))
    {
        MaxSelection focusSelection;
        sdword focusIndex;

        focusSelection.numShips = universe.mainCameraCommand.currentCameraStack->focus.numShips;
        for (focusIndex = 0; focusIndex < focusSelection.numShips; focusIndex++)
        {
            focusSelection.ShipPtr[focusIndex] = universe.mainCameraCommand.currentCameraStack->focus.ShipPtr[focusIndex];
        }
        selSelectionAddSingleShipNoCheck(&focusSelection, firstChild);
        selSelectionAddSingleShipNoCheck(&focusSelection, secondChild);
        ccFocus(&universe.mainCameraCommand, (FocusCommand *)&focusSelection);
    }

    sharedHealthScale = (linkedShip->health - linkedShip->linkCombinedHealth) / linkedShip->staticinfo->maxhealth;
    firstChild->health = sharedHealthScale * firstChild->staticinfo->maxhealth + linkedShip->linkSourceShipHealth0;
    secondChild->health = sharedHealthScale * secondChild->staticinfo->maxhealth + linkedShip->linkSourceShipHealth1;

    if (firstChild->health < 1.0f)
    {
        firstChild->health = 1.0f;
    }
    else if (firstChild->health > firstChild->staticinfo->maxhealth)
    {
        firstChild->health = firstChild->staticinfo->maxhealth;
    }

    if (secondChild->health < 1.0f)
    {
        secondChild->health = 1.0f;
    }
    else if (secondChild->health > secondChild->staticinfo->maxhealth)
    {
        secondChild->health = secondChild->staticinfo->maxhealth;
    }

    firstChild->tacticstype = linkedShip->tacticstype;
    secondChild->tacticstype = linkedShip->tacticstype;

    if ((((linkedShip->hotKeyGroup & 0xFF) - 1) & 0xF) != 0xF)
    {
        sdword hotKeyIndex = (((linkedShip->hotKeyGroup & 0xFF) - 1) & 0xF);
        udword hotKeyMask = (1 << (hotKeyIndex + 5));

        firstChild->hotKeyGroup = (uword)(((firstChild->hotKeyGroup | hotKeyMask) & 0xFFF0) + hotKeyIndex + 1);
        selSelectionAddSingleShip(&selHotKeyGroup[hotKeyIndex], firstChild);
        secondChild->hotKeyGroup = (uword)(((secondChild->hotKeyGroup | hotKeyMask) & 0xFFF0) + hotKeyIndex + 1);
        selSelectionAddSingleShip(&selHotKeyGroup[hotKeyIndex], secondChild);
    }

    linkedSpecifics = acvShipSpecifics(linkedShip);
    for (i = 0; i < (sdword)(sizeof(linkedSpecifics->firstSourceDamageLevelCounters.words) / sizeof(linkedSpecifics->firstSourceDamageLevelCounters.words[0])); i++)
    {
        sdword sharedRuntime = (sdword)linkedShip->damageLevelCounters.words[i]
                             - (sdword)linkedSpecifics->firstSourceDamageLevelCounters.words[i]
                             - (sdword)linkedSpecifics->secondSourceDamageLevelCounters.words[i];

        firstChild->damageLevelCounters.words[i] = (udword)(sharedRuntime / 2 + (sdword)linkedSpecifics->firstSourceDamageLevelCounters.words[i]);
        secondChild->damageLevelCounters.words[i] = (udword)(sharedRuntime / 2 + (sdword)linkedSpecifics->secondSourceDamageLevelCounters.words[i]);
    }

    if (linkedShip->shiptype == sACV || linkedShip->shiptype == bACV)
    {
        if (linkedSpecifics->firstSourceLinkEnabled)
            acolyteStartLinking(firstChild);
        else
            acolyteStopLinking(firstChild);

        if (linkedSpecifics->secondSourceLinkEnabled)
            acolyteStartLinking(secondChild);
        else
            acolyteStopLinking(secondChild);
    }

    dockInitLinkedFissionChildren(firstChild, secondChild);
    madAnimationStart(firstChild, animationIndex);
    madAnimationStart(secondChild, animationIndex);

    linkedShip->command = NULL;
    selection->ShipPtr[selectionIndex] = secondChild;
    secondChild->command = docktodo;
    univReallyDeleteThisShipRightNow(linkedShip);
    soundEventPlay(firstChild, 2101, 0);
    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : processLinkedLaunchToDo
    Description : see header comment above dockShipUsesLinkedFission; per-ship
                  loop mirrors retail's real 0x468650, dispatched under the new
                  COMMAND_LINKED_LAUNCH order (12) instead of COMMAND_DOCK.
    Inputs      :
    Outputs     :
    Return      : returns TRUE if done
----------------------------------------------------------------------------*/
bool32 processLinkedLaunchToDo(CommandToDo *docktodo)
{
    SelectCommand *selection = docktodo->selection;
    bool32 allDone = TRUE;
    sdword i;

    // retail (0x468650) always advances exactly one selection index per call, whether a
    // ship is mid-fission, mid mirror-sync, done, or swap-removed; never reprocesses the
    // same index (or a ship swapped into it) within the same tick, so this loop always
    // increments once per pass instead of using an early continue.
    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];
        ShipLaunchInitSpecificsOverlay *launchSpecifics = shipLaunchInitSpecifics(ship);
        Ship *partner = launchSpecifics->receiverShip;

        if (partner == NULL)
        {
            continue;
        }

        if (launchSpecifics->state == 0)
        {
            dockProcessLinkedFission(docktodo, i, &allDone);
            continue;
        }

        if (launchSpecifics->state == 1)
        {
            matrix mirrored;
            vector newPosition;
            real32 separation;

            // mirror partner's coordsys (negate right/up columns, keep heading), matching
            // the reflection used when the two halves were first split in dockProcessLinkedFission
            mirrored.m11 = -partner->rotinfo.coordsys.m11;
            mirrored.m21 = -partner->rotinfo.coordsys.m21;
            mirrored.m31 = -partner->rotinfo.coordsys.m31;
            mirrored.m12 = -partner->rotinfo.coordsys.m12;
            mirrored.m22 = -partner->rotinfo.coordsys.m22;
            mirrored.m32 = -partner->rotinfo.coordsys.m32;
            mirrored.m13 = partner->rotinfo.coordsys.m13;
            mirrored.m23 = partner->rotinfo.coordsys.m23;
            mirrored.m33 = partner->rotinfo.coordsys.m33;
            ship->rotinfo.coordsys = mirrored;

            separation = ShipStaticInfoGetLinkDistanceValue(ship->staticinfo) * 2.0f;
            newPosition.x = partner->posinfo.position.x + separation * mirrored.m11;
            newPosition.y = partner->posinfo.position.y + separation * mirrored.m21;
            newPosition.z = partner->posinfo.position.z + separation * mirrored.m31;
            posSetShipPosition(ship, &newPosition);

            if ((ship->madBindings->bPaused && partner->madBindings->bPaused) ||
                TimerExpired((sdword)&ship->linkTransformTimer) ||
                TimerExpired((sdword)&partner->linkTransformTimer))
            {
                real32 torque;

                if (ship->shiptype == sMimic && shipModuleSlotRuntime(ship)->state == ShipModuleSlotState_Destroyed)
                {
                    shipModuleSlotResetState(ship);
                    shipModuleSlotResetState(partner);
                }

                shipLaunchInitSpecifics(ship)->receiverShip = NULL;
                shipLaunchInitSpecifics(partner)->receiverShip = NULL;
                ship->command = NULL;

                selection->numShips--;
                selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];

                madAnimationStop(ship);
                madAnimationStop(partner);

                torque = ship->nonstatvars.rotstrength[ROT_ROLLRIGHT];
                if (torque > 0.5f)
                {
                    torque = 0.5f;
                }
                physApplyRotToObj((SpaceObjRot *)ship, torque, ROT_ROLLRIGHT);
            }
            else
            {
                allDone = FALSE;
            }
        }
    }

    return allDone;
}

bool32 processDockToDo(CommandToDo *docktodo)
{
    sdword i;
    SelectCommand *selection = docktodo->selection;
    Ship *ship;
    Ship *dockwith;
    bool32 allDone = TRUE;
    udword returnValue;
    bool32 shipRemovedFromSelection;

    for (i = 0; i < selection->numShips; )
    {
        shipRemovedFromSelection = FALSE;
        ship = selection->ShipPtr[i];

        dockwith = ship->dockvars.dockship;


        if (dockwith == NULL)
        {
            DockType dockType = dockCommandGetMode(docktodo);

            // Retail (processDockToDo @0x466950, disasm 0x466989-0x4669a7) calls
            // FindNearestShipToDockAt unconditionally whenever dockwith is NULL, passing
            // the raw mode value straight through with no mode-based gate or translation.
            // There is no retail equivalent of a "does this mode need a host search" check.
            dockwith = FindNearestShipToDockAt(ship, dockType);

            if (dockwith != NULL)
            {
                // Retail processDockToDo (0x466950, disasm 0x4669b5-0x466a4a) inlines this
                // launch-setup sequence directly; there is no call to a shared helper function.
                // The STAY_TILL_EXPLICITLAUNCH-equivalent bit is written to ship->specialFlags1C8
                // (BIT12) at this call site, not ship->specialFlags (the HW1
                // SPECIAL_STAY_TILL_EXPLICITLAUNCH field, a different bit array entirely) nor
                // ship->specialFlags3. Retail also does not touch dockingCollisionTarget,
                // cataBusyStateWindow.dockingship, unk_618, dockstaticpoint, or customdockinfo
                // in this function.
                if (dockType == DOCK_MODE_STAY_TILL_EXPLICITLAUNCH ||
                    dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH)
                {
                    ship->specialFlags1C8 |= BIT12;
                    if (dockType == DOCK_MODE_INSTANT_STAY_TILL_EXPLICITLAUNCH)
                    {
                        if (dockwith->shiptype == sHiveFrigate || dockwith->shiptype == bHiveFrigate)
                        {
                            shipClearSwarmers(dockwith);
                        }
                    }
                }

                ship->dockvars.dockship = dockwith;

                if (dockType == DOCK_MODE_LAUNCH_AT_SPECIFIC_SHIP)
                {
                    ship->flags2 |= SOF2_BayDocked;
                    ship->dockvars.dockLaunchState = 10;
                }
                else
                {
                    // Binary processLaunchShipsToDo: close MAD guns on IonArrayFrigate ships
                    if (ship->shiptype == bIonArrayFrigate || ship->shiptype == p1IonArrayFrigate)
                    {
                        if (ship->madGunStatus == MAD_STATUS_GUNS_OPEN ||
                            ship->madGunStatus == MAD_STATUS_GUNS_OPENING)
                        {
                            madLinkInCloseGunsShip(ship);
                        }
                    }
                    ship->dockvars.dockLaunchState = 0;
                }
                ship->dockvars.dockPointIndex = -1;

                InitShipAI(ship, TRUE);
                allDone = FALSE;
            }
        }
        else
        {
            returnValue = dockProcessSingleShipToDo(docktodo, ship, dockwith);
            ship->shipidle = FALSE;
            if (returnValue == NOT_FINISHED_DOCKING ||
                (returnValue == FINISHED_DOCKING_FLYING_BACK && !dockCommandGetHaveFlyedFarEnoughAway(docktodo)))
            {
                allDone = FALSE;
            }
            else
            {
                if (docktodo->ordertype.attributes & COMMAND_IS_FORMATION)
                {
                    RemoveShipFromFormation(ship);
                    if (selection->numShips > 0)
                    {
                        formationContentHasChanged(docktodo);
                    }
                }

                if (docktodo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
                {
                    RemoveShipFromAttacking(ship);
                    RemoveShipReferencesFromExtraAttackInfo(ship, docktodo);
                }

                if (!dockCommandGetWasHarvesting(docktodo) || dockCommandGetMode(docktodo) == DOCK_MODE_RETIRE)
                {
                    // Retail (processDockToDo @0x466950, disasm at 0x466ab9-0x466ad4): calling
                    // dockCompleteSingleShip goes to LABEL_31, skipping the index increment below.
                    // dockCompleteSingleShip -> clRemoveTargetFromSelection swap-removes this ship
                    // (last array entry moves into slot i, numShips--), so the loop must revisit
                    // index i rather than advancing past the ship that just moved into this slot.
                    dockCompleteSingleShip(ship, dockwith, docktodo);
                    shipRemovedFromSelection = TRUE;
                }
            }
        }


        if (!shipRemovedFromSelection)
        {
            i++;
        }
    }

    return allDone;
}

/*-----------------------------------------------------------------------------
    Name        : dockDrawDockInfo
    Description : draws docking info for debugging
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if RND_VISUALIZATION
void dockDrawDockInfo(Ship *ship)
{
    sdword i;
    DockStaticInfo *dockStaticInfo = ((ShipStaticInfo *)(ship->staticinfo))->dockStaticInfo;
    sdword numDockPoints;
    DockStaticPoint *dockstaticpoint;
    vector to;

    if (dockStaticInfo == NULL)
    {
        return;
    }

    numDockPoints = dockStaticInfo->numDockPoints;

    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    for (i=0,dockstaticpoint=&dockStaticInfo->dockstaticpoints[0];i<numDockPoints;i++,dockstaticpoint++)
    {
        vecScalarMultiply(to,dockstaticpoint->conenormal,1000.0f);
        vecAddTo(to,dockstaticpoint->position);
        primLine3(&dockstaticpoint->position,&to,colWhite);
    }

    rndLightingEnable(TRUE);
}
#endif

bool32 ShipIsRefuelingAtCarrierMother(struct Ship *ship)
{
    if (ship->dockvars.dockship)
    {
        sdword dockState = ship->dockvars.dockLaunchState;
        return (dockState == 8 || dockState == 9);
    }
    return FALSE;
}

bool32 ShipIsWaitingForSoftLaunch(struct Ship *ship)
{
    if (ship->dockvars.dockship)
    {
        ShipType dockshiptype = ship->dockvars.dockship->shiptype;
        if (dockshiptype == sCarrier || dockshiptype == bCarrier ||
            dockshiptype == tCarrier || dockshiptype == kCarrier)
        {
            sdword dockState = ship->dockvars.dockLaunchState;
            return (dockState == 8 || dockState == 9);
        }
    }

    return FALSE;
}

CommandToDo *findFormationGuardingShip(Ship *ship)
{
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);

        if ((command->ordertype.attributes & (COMMAND_IS_PROTECTING|COMMAND_IS_FORMATION)) == (COMMAND_IS_PROTECTING|COMMAND_IS_FORMATION))
        {
            if (ShipInSelection(command->protect,ship))
            {
                return command;
            }
        }

        curnode = curnode->next;
    }

    return NULL;
}


CrystalDockState *dockGetCrystalDockState(Ship *ship)
{
    switch (ship->shiptype)
    {
        case sCarrier:
        case bCarrier:
        case kCarrier:
        case tCarrier:
        case dKushanCarrier:
            return (CrystalDockState *)&((CarrierSpec *)ship->ShipSpecifics)->droptarget[1];
        case sProcessor:
        case bProcessor:
        case p1Mothership:
            return (CrystalDockState *)ship->ShipSpecifics;
        case sMothership:
        case bMothership:
        case trGhostship:
            return (CrystalDockState *)shipModuleSlotRuntime(ship)->slotAvailability;
        case bHeavyCruiser:
            return (CrystalDockState *)((HeavyCruiserSpec *)ship->ShipSpecifics)->unk_18;
        default:
            return NULL;
    }
}

static sbyte *dockGetCrystalDockSlotIndexPtr(Ship *ship)
{
    return &ship->crystalDockSlotIndex;
}

static Ship **dockGetCrystalDockHostPtr(Ship *ship)
{
    return &ship->specialTargetShip;
}

static sdword *dockGetCrystalQueueIndexPtr(Ship *ship)
{
    return &ship->dockvars.dockPointIndex;
}

static void dockClearCrystalSlot(CrystalDockSlot *slot)
{
    memset(slot, 0, sizeof(*slot));
}

void dockFreeDockPoint(Ship *ship)
{
    sdword dockIndex = ship->dockvars.dockPointIndex;

    if (dockIndex != -1 && ship->staticinfo->shipclass != CLASS_Component)
    {
        if (ship->dockvars.dockship != NULL)
        {
            dockMarkPointFreed(ship->dockvars.dockship, dockIndex);
        }
        ship->dockvars.dockPointIndex = -1;
        dockSetAvoidanceIgnoreTarget(ship, NULL);
        ship->dockvars.dockship = NULL;
    }

    ship->flags2 &= ~SOF2_BayDocked;
    ship->specialFlags1C8 &= ~BIT12;
    ship->flags &= ~SOF_AvoidanceHidden;
    ship->specialFlags2 &= ~0x40u;
}

void dockComputeOverideOrientation(Ship *ship, real32 *dockOffset, Ship *dockwith, DockStaticPoint *dockpoint, matrix *result)
{
    matrix dockPointMatrix;
    matrix dockOffsetMatrix;
    vector heading;
    vector up;
    vector right;
    vector offsetCol1;
    vector offsetCol2;
    vector offsetCol3;

    (void)ship;

    matCreateCoordSysFromHeading(&dockPointMatrix, &dockpoint->conenormal);
    GetDirectionVectorOfShip(&heading, dockpoint->headingdirection, &dockPointMatrix);
    GetDirectionVectorOfShip(&up, dockpoint->updirection, &dockPointMatrix);
    // Retail (0x4636dc): a.x = up.z*heading.y - up.y*heading.z, which expands to
    // cross(heading, up), not cross(up, heading).
    vecCrossProduct(right, heading, up);
    vecNormalize(&right);
    matCreateMatFromVecs(&dockPointMatrix, &up, &right, &heading);
    matMultiplyMatByMat(result, &dockwith->rotinfo.coordsys, &dockPointMatrix);

    offsetCol1.x = -dockOffset[5];
    offsetCol1.y = -dockOffset[6];
    offsetCol1.z = -dockOffset[7];
    offsetCol2.x = -offsetCol1.y;
    offsetCol2.y = offsetCol1.x;
    offsetCol2.z = 0.0f;
    vecNormalize(&offsetCol2);
    // Retail (0x4637cf): v11.x = col2.z*v7.y - col2.y*v7.z, which expands to
    // cross(v7, col2) == cross(offsetCol1, offsetCol2), not cross(offsetCol2, offsetCol1).
    // (Re-verified directly against 0x463680 decompilation; a prior pass had this backwards.)
    vecCrossProduct(offsetCol3, offsetCol1, offsetCol2);
    vecNormalize(&offsetCol3);
    matCreateMatFromVecs(&dockOffsetMatrix, &offsetCol1, &offsetCol2, &offsetCol3);
    matTranspose(&dockOffsetMatrix);
    matMultiplyMatByMat(result, result, &dockOffsetMatrix);
}

bool32 dockFlyToDockPoint(Ship *ship, Ship *dockwith)
{
    vector direction = { 1.0f, 0.0f, 0.0f };
    vector destination;
    real32 scale;

    matMultiplyMatByVec(&destination, &dockwith->rotinfo.coordsys, &direction);
    scale = dockwith->staticinfo->staticheader.staticCollInfo.collspheresize;
    if (dockwith->shiptype == sMothership || dockwith->shiptype == kCarrier)
    {
        scale *= 2.0f;
    }
    vecScalarMultiply(destination, destination, scale);
    vecAddTo(destination, dockwith->posinfo.position);

    if (MoveReachedDestinationVariable(ship, &destination, 100.0f))
    {
        return TRUE;
    }

    aishipFlyToPointAvoidingObjsFunc(ship, &destination, 6, -80.0f, &dockwith->posinfo.velocity);
    return FALSE;
}

void playerAddResourceUnits(Player *player, sdword amount)
{
    if (amount > 0)
    {
        player->resourceUnits += amount;
        universe.gameStats.playerStats[player->playerIndex].totalRegeneratedResourceUnits += amount;
        universe.gameStats.playerStats[player->playerIndex].totalResourceUnits += amount;
        universe.gameStats.totalResourceUnitsCollected += amount;
        if (singlePlayerGame && player->playerIndex == 0)
        {
        }
    }
}

bool32 dockFlyToConeOrigin(Ship *ship, Ship *dockwith)
{
    DockStaticInfo *dockStaticInfo;
    DockStaticPoint *dockstaticpoint;
    vector coneHeading;
    vector conePosition;
    vector coneToShip;
    vector destination;
    vector projected;
    real32 projectionFactor;
    real32 coneDistance;

    dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];
    coneDistance = dockstaticpoint->flyawaydist;

    matMultiplyMatByVec(&coneHeading, &dockwith->rotinfo.coordsys, &dockstaticpoint->conenormal);
    matMultiplyMatByVec(&conePosition, &dockwith->rotinfo.coordsys, &dockstaticpoint->position);
    vecAddTo(conePosition, dockwith->posinfo.position);

    destination.x = coneDistance * 4.0f * coneHeading.x + conePosition.x;
    destination.y = coneDistance * 4.0f * coneHeading.y + conePosition.y;
    destination.z = coneDistance * 4.0f * coneHeading.z + conePosition.z;

    vecSub(coneToShip, ship->posinfo.position, conePosition);
    projectionFactor = vecDotProduct(coneToShip, coneHeading) / vecMagnitudeSquared(coneHeading);
    projected.x = coneHeading.x * projectionFactor + conePosition.x;
    projected.y = coneHeading.y * projectionFactor + conePosition.y;
    projected.z = coneHeading.z * projectionFactor + conePosition.z;
    posSetShipPosition(ship, &projected);

    if (dockwith->shiptype != 12 || dockCheckAnimStateForCone(dockwith, dockstaticpoint->type))
    {
        real32 distanceSq = vecMagnitudeSquared(coneToShip);
        if (coneDistance * coneDistance <= distanceSq)
        {
            ship->specialFlags1C8 &= ~BIT24;
            return TRUE;
        }
        aishipFlyToPointAvoidingObjsFunc(ship, &destination, 0, 0.0f, &dockwith->posinfo.velocity);
    }
    return FALSE;
}

bool32 dockFlyToConeInside(Ship *ship, Ship *dockwith, DockStaticPoint *dockstaticpoint)
{
    vector conePosition;
    vector coneToShip;
    vector destination;
    vector coneHeading;
    vector shipHeading;
    ShipClass shipclass;
    real32 distance;
    real32 clampedDistance;
    real32 dotProduct;
    real32 coneThreshold;

    matMultiplyMatByVec(&conePosition, &dockwith->rotinfo.coordsys, &dockstaticpoint->position);
    vecAddTo(conePosition, dockwith->posinfo.position);
    vecSub(coneToShip, ship->posinfo.position, conePosition);
    distance = fsqrt(vecMagnitudeSquared(coneToShip));
    clampedDistance = ClampToWithin(distance, dockstaticpoint->mindist, dockstaticpoint->maxdist);

    matMultiplyMatByVec(&coneHeading, &dockwith->rotinfo.coordsys, &dockstaticpoint->conenormal);
    destination.x = dockstaticpoint->mindist * coneHeading.x + conePosition.x;
    destination.y = dockstaticpoint->mindist * coneHeading.y + conePosition.y;
    destination.z = dockstaticpoint->mindist * coneHeading.z + conePosition.z;

    dotProduct = vecDotProduct(coneHeading, coneToShip);
    coneThreshold = distance * dockstaticpoint->coneangle;
    matGetVectFromMatrixCol3(shipHeading, ship->rotinfo.coordsys);

    if (coneThreshold >= dotProduct + 5.0f)
    {
        if (clampedDistance + 500.0f >= distance)
        {
            aishipFlyToPointAvoidingObjsFunc(ship, &destination, 3, -10.0f, &dockwith->posinfo.velocity);
        }
        else
        {
            aishipFlyToPointAvoidingObjsFunc(ship, &destination, 7, -80.0f, &dockwith->posinfo.velocity);
        }
        return FALSE;
    }

    shipclass = ship->staticinfo->shipclass;
    if (!shipIsBeastShip(dockwith)
     || (shipclass != CLASS_Fighter
      && shipclass != CLASS_Corvette
      && shipclass != CLASS_Micro))
    {
        aitrackForceSteadyShip(ship);
    }
    return TRUE;
}

sdword dockFindNearestDockPoint(Ship *ship, Ship *dockwith, sdword dockType)
{
    DockStaticInfo *dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    DockPointActivity *dockPointActivity = dockwith->dockvars.dockPointActivity;
    sdword dockPointType;
    sdword firstPoint;
    sdword count;
    sdword bestIndex = -1;
    real32 minDistSqr = REALlyBig;
    sdword i;

    dockPointType = cmGetBuildState(ship->staticinfo->shipclass,
                                    ship->staticinfo->shiptype,
                                    0,
                                    ship->staticinfo->shipFlags,
                                    dockType,
                                    dockwith);
    if ((dockwith->shiptype == sMothership || dockwith->shiptype == kCarrier)
     && (!univPlayerHasShipType(sMothershipDockingBay, dockwith->playerowner)
      || dockPointActivity->numDockedOfType[dockPointType] == dockStaticInfo->numPointsOfType[dockPointType]))
    {
        switch (dockPointType)
        {
            case DPT_Small_In:
                dockPointType = DPT_Small_In_Engineering;
                break;
            case DPT_Small_Out:
                dockPointType = DPT_Small_Out_Engineering;
                break;
            case DPT_Med_In:
                dockPointType = DPT_Med_In_Engineering;
                if (dockPointActivity->numDockedOfType[DPT_Med_Out] != 0) return FALSE;
                break;
            case DPT_Med_Out:
                dockPointType = DPT_Med_Out_Engineering;
                if (dockPointActivity->numDockedOfType[DPT_Med_In] == 0) return FALSE;
                break;
        }
    }
    if (dockwith->shiptype == sCarrier && (dockPointType == DPT_Med_In || dockPointType == DPT_Med_Out))
    {
        dockPointType = DPT_Med_In;
        if (ship->dockvars.dockPointIndex != -1)
        {
            return TRUE;
        }
    }
    if (dockPointActivity->numDockedOfType[dockPointType] >= dockStaticInfo->numPointsOfType[dockPointType])
    {
        return FALSE;
    }

    firstPoint = dockStaticInfo->firstPointOfType[dockPointType];
    count = dockStaticInfo->numPointsOfType[dockPointType];
    for (i = 0; i < count; i++)
    {
        sdword dockPointIndex = firstPoint + i;
        vector worldPosition;
        vector worldCone;
        vector worldConeInside;
        vector diff;
        real32 distSqr;

        if (dockPointActivity->reserved[dockPointIndex] != 0)
        {
            continue;
        }

        matMultiplyMatByVec(&worldPosition, &dockwith->rotinfo.coordsys, &dockStaticInfo->dockstaticpoints[dockPointIndex].position);
        vecAddTo(worldPosition, dockwith->posinfo.position);
        vecSub(diff, ship->posinfo.position, worldPosition);
        distSqr = vecMagnitudeSquared(diff);
        matMultiplyMatByVec(&worldCone, &dockwith->rotinfo.coordsys, &dockStaticInfo->dockstaticpoints[dockPointIndex].conenormal);
        worldConeInside.x = dockStaticInfo->dockstaticpoints[dockPointIndex].mindist * worldCone.x + worldPosition.x;
        worldConeInside.y = dockStaticInfo->dockstaticpoints[dockPointIndex].mindist * worldCone.y + worldPosition.y;
        worldConeInside.z = dockStaticInfo->dockstaticpoints[dockPointIndex].mindist * worldCone.z + worldPosition.z;
        vecSub(diff, ship->posinfo.position, worldPosition);
        distSqr = vecMagnitudeSquared(diff);
        if (distSqr < minDistSqr)
        {
            minDistSqr = distSqr;
            bestIndex = dockPointIndex;
        }
    }

    if (bestIndex == -1)
    {
        return FALSE;
    }

    ship->dockvars.coneDistance = ClampToWithin(fsqrt(minDistSqr),
                                                dockStaticInfo->dockstaticpoints[bestIndex].mindist,
                                                dockStaticInfo->dockstaticpoints[bestIndex].maxdist);
    ship->dockvars.dockPointIndex = bestIndex;
    dockPointActivity->reserved[bestIndex] = 1;
    dockPointActivity->numDockedOfType[dockStaticInfo->dockstaticpoints[bestIndex].type]++;
    return TRUE;
}

bool32 dockFindAndReserveDockPoint(Ship *ship, Ship *dockwith)
{
    DockStaticInfo *dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    DockPointActivity *dockPointActivity = dockwith->dockvars.dockPointActivity;
    sdword dockPointType;
    sdword firstPoint;
    sdword dockPointIndex;

    if (dockStaticInfo == NULL || dockPointActivity == NULL)
    {
        return FALSE;
    }

    dockPointType = cmGetBuildState(ship->staticinfo->shipclass,
                                    ship->staticinfo->shiptype,
                                    ship->staticinfo->externalBuild,
                                    ship->staticinfo->shipFlags,
                                    2,
                                    dockwith);
    if ((dockwith->shiptype == sMothership && !univPlayerHasShipType(sMothershipDockingBay, dockwith->playerowner))
     || dockwith->shiptype == trGhostship)
    {
        switch (dockPointType)
        {
            case DPT_Small_In: dockPointType = DPT_Small_In_Engineering; break;
            case DPT_Small_Out: dockPointType = DPT_Small_Out_Engineering; break;
            case DPT_Med_In: dockPointType = DPT_Med_In_Engineering; break;
            case DPT_Med_Out: dockPointType = DPT_Med_Out_Engineering; break;
        }
    }
    if (dockwith->shiptype == sCarrier && (dockPointType == DPT_Med_In || dockPointType == DPT_Med_Out))
    {
        dockPointType = DPT_Med_In;
        if (ship->dockvars.dockPointIndex != -1)
        {
            return TRUE;
        }
    }

    firstPoint = dockStaticInfo->firstPointOfType[dockPointType];
    if (firstPoint == 0xFF)
    {
        return FALSE;
    }

    dockPointIndex = firstPoint;
    while (dockPointIndex < dockStaticInfo->numDockPoints &&
           dockStaticInfo->dockstaticpoints[dockPointIndex].type == dockPointType)
    {
        if (dockPointActivity->reserved[dockPointIndex] == 0)
        {
            ship->dockvars.coneDistance = dockStaticInfo->dockstaticpoints[dockPointIndex].flyawaydist;
            ship->dockvars.dockPointIndex = dockPointIndex;
            dockPointActivity->reserved[dockPointIndex] = 1;
            dockPointActivity->numDockedOfType[dockPointType]++;
            return TRUE;
        }
        dockPointIndex++;
    }

    return FALSE;
}

sdword dockAllocDockPointActivity(Ship *ship)
{
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    DockPointActivity *activity;

    if (dockStaticInfo == NULL)
    {
        ship->dockvars.dockPointActivity = NULL;
        return 0;
    }

    activity = memAlloc(36 + 4 * dockStaticInfo->numDockPoints, "dockPointActivity", NonVolatile);
    memset(activity, 0, 36 + 4 * dockStaticInfo->numDockPoints);
    ship->dockvars.dockPointActivity = activity;
    return dockStaticInfo->numDockPoints;
}

bool32 dockNeedsSpecialDocking(Ship *ship, sdword dockState)
{
    Ship *dockwith;
    DockStaticInfo *dockStaticInfo;
    sdword dockIndex;

    if (singlePlayerGame && ship->shiptype == sWorker)
    {
        dockwith = ship->dockvars.dockship;
    if (dockwith != NULL && dockwith->shiptype == dEcclesiamInfected)
        {
            return TRUE;
        }
    }

    dockwith = ship->dockvars.dockship;
    dockIndex = ship->dockvars.dockPointIndex;
    if (dockwith != NULL && dockIndex >= 0)
    {
        dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
        if (dockStaticInfo != NULL)
        {
            DockPointType pointType = dockStaticInfo->dockstaticpoints[dockIndex].type;
            if (pointType >= 4 && pointType <= 5)
            {
                return TRUE;
            }
        }
    }

    return dockState >= 3 && dockState <= 5;
}

sdword dockPrepareSingleShipForLaunch(Ship *ship, Ship *dockship, DockType dockType);

bool32 dockFlyToFinalDockPoint(Ship *ship, Ship *dockwith, real32 tolerance)
{
    DockStaticInfo *dockStaticInfo = dockwith->staticinfo->dockStaticInfo;
    DockStaticPoint *dockstaticpoint = &dockStaticInfo->dockstaticpoints[ship->dockvars.dockPointIndex];
    vector destination;
    vector desiredHeading;
    vector desiredUp;
    vector coneHeading;
    vector toDestination;
    vector adjustedPosition;
    real32 distance;
    real32 headingDot;
    bool32 aligned;

    if (shipIsBeastShip(dockwith)
     && (ship->staticinfo->shipclass == CLASS_Fighter
      || ship->staticinfo->shipclass == CLASS_Corvette
      || ship->staticinfo->shipclass == CLASS_Micro))
    {
        matMultiplyMatByVec(&destination, &dockwith->rotinfo.coordsys, &dockstaticpoint->position);
        vecAddTo(destination, dockwith->posinfo.position);
        if (MoveReachedDestinationVariable(ship, &destination, ship->staticinfo->staticheader.staticCollInfo.collspheresize))
        {
            return TRUE;
        }
        aishipFlyToPointAvoidingObjsFunc(ship, &destination, 7, -100.0f, &dockwith->posinfo.velocity);
        return FALSE;
    }

    matMultiplyMatByVec(&destination, &dockwith->rotinfo.coordsys, &dockstaticpoint->position);
    vecAddTo(destination, dockwith->posinfo.position);
    if (MoveReachedDestinationVariable(ship, &destination, tolerance))
    {
        return TRUE;
    }

    matMultiplyMatByVec(&coneHeading, &dockwith->rotinfo.coordsys, &dockstaticpoint->conenormal);
    GetDirectionVectorOfShip(&desiredUp, dockstaticpoint->updirection, &dockwith->rotinfo.coordsys);

    if (ship->shiptype == dEcclesiam)
    {
        desiredHeading.x = -coneHeading.x;
        desiredHeading.y = -coneHeading.y;
        desiredHeading.z = -coneHeading.z;
        vecCrossProduct(desiredUp, desiredUp, desiredHeading);
    }
    else
    {
        GetDirectionVectorOfShip(&desiredHeading, dockstaticpoint->headingdirection, &dockwith->rotinfo.coordsys);
    }

    vecSub(toDestination, ship->posinfo.position, destination);
    distance = fsqrt(vecMagnitudeSquared(toDestination));
    if (distance <= 0.0f)
    {
        return TRUE;
    }
    vecNormalize(&toDestination);
    headingDot = vecDotProduct(coneHeading, toDestination);

    adjustedPosition.x = (coneHeading.x - ship->lastDockApproachHeading.x) * distance + ship->posinfo.position.x;
    adjustedPosition.y = (coneHeading.y - ship->lastDockApproachHeading.y) * distance + ship->posinfo.position.y;
    adjustedPosition.z = (coneHeading.z - ship->lastDockApproachHeading.z) * distance + ship->posinfo.position.z;
    posSetShipPosition(ship, &adjustedPosition);

    aligned = aitrackHeadingAndUp(ship, &desiredHeading, &desiredUp, 0.995f);
    if (headingDot < 0.0f)
    {
        return TRUE;
    }

    if (headingDot < 0.995f && distance > 100.0f)
    {
        vector projected;
        real32 projection = vecDotProduct(coneHeading, toDestination) * distance;
        projected.x = projection * coneHeading.x + destination.x;
        projected.y = projection * coneHeading.y + destination.y;
        projected.z = projection * coneHeading.z + destination.z;
        aishipFlyToPointAvoidingObjsFunc(ship, &projected, 0, -20.0f, &dockwith->posinfo.velocity);
    }

    if (aligned)
    {
        udword flags = shipIsBeastShip(dockwith) ? 4 : 0;
        real32 limitVel = shipIsBeastShip(dockwith) ? -100.0f : -120.0f;

        aishipFlyToPointAvoidingObjsFunc(ship, &destination, flags, limitVel, &dockwith->posinfo.velocity);
        if (shipIsBeastShip(dockwith))
        {
            real32 snapTolerance = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
            if (snapTolerance <= 25.0f)
            {
                snapTolerance = 1000.0f;
            }
            if (MoveReachedDestinationVariable(ship, &destination, snapTolerance))
            {
                return TRUE;
            }
        }
    }

    ship->lastDockApproachHeading = coneHeading;
    return FALSE;
}

bool32 dockFlyToClampedConePosition(Ship *ship, Ship *dockwith, DockStaticPoint *dockstaticpoint)
{
    vector conePosition;
    vector coneHeading;
    vector destination;
    real32 distance;
    real32 tolerance;
    udword flags = 179;
    real32 limitVel;

    if (dockwith->objtype == OBJ_ShipType && shipIsBeastShip(dockwith))
    {
        ShipClass shipclass = ship->staticinfo->shipclass;
        if (shipclass == CLASS_Fighter || shipclass == CLASS_Corvette || shipclass == CLASS_Micro)
        {
            return TRUE;
        }
    }

    matMultiplyMatByVec(&conePosition, &dockwith->rotinfo.coordsys, &dockstaticpoint->position);
    vecAddTo(conePosition, dockwith->posinfo.position);
    {
        vector coneToShip;
        vecSub(coneToShip, ship->posinfo.position, conePosition);
        distance = fsqrt(vecMagnitudeSquared(coneToShip));
    }
    distance = ClampToWithin(distance, dockstaticpoint->mindist, dockstaticpoint->maxdist);

    matMultiplyMatByVec(&coneHeading, &dockwith->rotinfo.coordsys, &dockstaticpoint->conenormal);
    vecScalarMultiply(destination, coneHeading, distance);
    vecAddTo(destination, conePosition);
    if (dockwith->objtype != OBJ_ShipType || !shipIsBeastShip(dockwith))
    {
        tolerance = 10.0f;
    }
    else
    {
        tolerance = ship->staticinfo->staticheader.staticCollInfo.collspheresize * 0.125f + 25.0f;
    }

    if (MoveReachedDestinationVariable(ship, &destination, tolerance))
    {
        return TRUE;
    }

    if (dockwith->objtype == OBJ_ShipType && shipIsBeastShip(dockwith))
    {
        flags = 183;
        limitVel = -100.0f;
    }
    else
    {
        limitVel = -80.0f;
    }
    aishipFlyToPointAvoidingObjsFunc(ship, &destination, flags, limitVel, &dockwith->posinfo.velocity);
    return FALSE;
}

bool32 dockFlyToOffsetDockPoint(Ship *ship, real32 *dockOffset, Ship *dockwith, DockStaticPoint *dockstaticpoint)
{
    matrix result;
    vector destination;
    vector offsetWorld;
    vector desiredHeading;
    vector desiredUp;
    real32 avoidRadiusSqr;
    real32 collisionRadiusSqr;
    real32 distSqr;
    bool32 closeEnough;
    bool32 aligned;
    bool32 shipReady = TRUE;
    bool32 dockwithReady = TRUE;
    bool32 dockingReady;
    madanim *shipAnimState;
    madanim *dockAnimState = NULL;

    avoidRadiusSqr = ship->staticinfo->staticheader.staticCollInfo.avoidcollspheresize;
    avoidRadiusSqr *= avoidRadiusSqr;
    collisionRadiusSqr = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    collisionRadiusSqr *= collisionRadiusSqr;

    dockComputeOverideOrientation(ship, dockOffset, dockwith, dockstaticpoint, &result);
    matMultiplyMatByVec(&destination, &dockwith->rotinfo.coordsys, &dockstaticpoint->position);
    vecAddTo(destination, dockwith->posinfo.position);
    matMultiplyMatByVec(&offsetWorld, &ship->rotinfo.coordsys, (vector *)(dockOffset + 2));
    vecSubFrom(destination, offsetWorld);

    if (dockwith->objtype == OBJ_ShipType && shipIsBeastShip(dockwith))
    {
        // Retail dockFlyToOffsetDockPoint (0x4675c0) always uses collspheresize*0.25f for
        // the beast-dockwith tolerance and the SAME 0.996f alignment threshold as the
        // non-beast branch below (single dockMatricesAlignedWithinThreshold call site at
        // 0x467751, constant 0x3F7EF9DB == 0.996f) -- there is no separate 0.985f/0.125f
        // beast-specific alignment threshold.
        real32 beastTolerance = ship->staticinfo->staticheader.staticCollInfo.collspheresize * 0.25f;
        closeEnough = beastTolerance > fabs(ship->posinfo.position.x - destination.x)
                   && beastTolerance > fabs(ship->posinfo.position.y - destination.y)
                   && beastTolerance > fabs(ship->posinfo.position.z - destination.z);
        aligned = dockMatricesAlignedWithinThreshold(&ship->rotinfo.coordsys, &result, 0.996f);
    }
    else
    {
        closeEnough = fabs(ship->posinfo.position.x - destination.x) < 5.0f
                   && fabs(ship->posinfo.position.y - destination.y) < 5.0f
                   && fabs(ship->posinfo.position.z - destination.z) < 5.0f;
        aligned = dockMatricesAlignedWithinThreshold(&ship->rotinfo.coordsys, &result, 0.996f);
    }

    distSqr = (destination.x - ship->posinfo.position.x) * (destination.x - ship->posinfo.position.x)
            + (destination.y - ship->posinfo.position.y) * (destination.y - ship->posinfo.position.y)
            + (destination.z - ship->posinfo.position.z) * (destination.z - ship->posinfo.position.z);

    shipAnimState = ship->madBindings;
    if (shipAnimState != NULL &&
        shipAnimState->nCurrentAnim != MAD_NoAnimation &&
        shipAnimState->flags == 0 &&
        (distSqr >= avoidRadiusSqr || !shipAnimState->bPaused))
    {
        shipReady = FALSE;
    }

    if (dockwith->objtype == OBJ_ShipType)
    {
        dockAnimState = dockwith->madBindings;
        if (dockAnimState != NULL &&
            dockAnimState->nCurrentAnim != MAD_NoAnimation &&
            dockAnimState->flags == 0)
        {
            dockwithReady = (distSqr < avoidRadiusSqr) && dockAnimState->bPaused;
        }
    }
    dockingReady = shipReady && dockwithReady;

    if (aligned && closeEnough && dockingReady)
    {
        ship->rotinfo.coordsys = result;
        ship->posinfo.velocity = dockwith->posinfo.velocity;
        posSetShipPosition(ship, &destination);
        return TRUE;
    }

    if (distSqr < avoidRadiusSqr)
    {
        if (shipAnimState != NULL &&
            shipAnimState->nCurrentAnim != MAD_NoAnimation &&
            shipAnimState->flags == 0)
        {
            madAnimationPause(ship, FALSE);
        }

        if (dockwith->objtype == OBJ_ShipType)
        {
            if (dockAnimState != NULL &&
                dockAnimState->nCurrentAnim != MAD_NoAnimation &&
                dockAnimState->flags == 0)
            {
                madAnimationPause(dockwith, FALSE);
            }
        }
    }

    if (!aligned)
    {
        matGetVectFromMatrixCol3(desiredHeading, result);
        matGetVectFromMatrixCol1(desiredUp, result);
        aitrackHeadingAndUp(ship, &desiredHeading, &desiredUp, 0.999f);
    }

    if (closeEnough)
    {
        return FALSE;
    }

    if ((aligned && dockingReady) || distSqr > collisionRadiusSqr)
    {
        aishipFlyToPointAvoidingObjsFunc(ship, &destination, 0, -120.0f, &dockwith->posinfo.velocity);
    }
    else
    {
        aitrackVelocityVectorGuidance((SpaceObjRotImpTargGuidance *)ship, &dockwith->posinfo.velocity);
    }

    return FALSE;
}

sdword PreFix_CrystalDockSlots(Ship *ship)
{
    CrystalDockState *state = dockGetCrystalDockState(ship);
    sdword i;
    sdword result = 0;

    if (state == NULL)
    {
        return 0;
    }
    for (i = 0; i < 16; i++)
    {
        result = SpaceObjRegistryGetID((SpaceObj *)state->slots[i].ship);
        state->slots[i].ship = (Ship *)result;
    }
    return result;
}

Ship *Fix_CrystalDockSlots(Ship *ship)
{
    CrystalDockState *state = dockGetCrystalDockState(ship);
    sdword i;
    Ship *result = NULL;

    if (state == NULL)
    {
        return NULL;
    }
    for (i = 0; i < 16; i++)
    {
        result = SpaceObjRegistryGetShip((sdword)state->slots[i].ship);
        state->slots[i].ship = result;
    }
    return result;
}

sdword salCapGetWorkerOpenAnimIndex(sdword shiptype)
{
    switch (shiptype)
    {
        case sWorker:
        case bWorker:
            return 3;
        default:
            return -1;
    }
}

sdword salCapGetWorkerCloseAnimIndex(sdword shiptype)
{
    switch (shiptype)
    {
        case sWorker:
        case bWorker:
            return 2;
        default:
            return -1;
    }
}

sdword InitCrystalDockSlots(Ship *ship)
{
    CrystalDockState *state = dockGetCrystalDockState(ship);

    ship->specialFlags2 |= SPECIAL_2_CrystalDockSlot;
    if (state != NULL)
    {
        memset(state, 0, sizeof(*state));
        state->assignedSlot = -1;
    }
    return 0;
}

sdword CrystalGetReplacementShipType(Ship *crystalShip, Ship *hostShip)
{
    sdword shipType;

    if (crystalShip->shiptype == dBigGunUnit)
    {
        return dBigGunUnit;
    }

    switch (hostShip->playerowner->race)
    {
        case RACE_Sect:
            break;
        case RACE_Beast:
            if (hostShip->shiptype < bRecon || hostShip->shiptype > bMissileCorvette)
            {
                break;
            }
            if (crystalShip->staticinfo->beastAttacksRequired == (udword)-1)
            {
                return -1;
            }
            return crystalShip->staticinfo->beastAttackConvertTo;
        case RACE_Kushan:
        case RACE_Taiidan:
        case RACE_P1:
        case RACE_Traders:
        case RACE_Derelict:
            break;
        default:
            return -1;
    }

    if (crystalShip->shiptype < bRecon || crystalShip->shiptype > bMissileCorvette)
    {
        return crystalShip->shiptype;
    }

    for (shipType = 0; shipType < TOTAL_NUM_SHIPS; shipType++)
    {
        ShipStaticInfo *shipStaticInfo = GetShipStaticInfo((ShipType)shipType, hostShip->shiprace);
        if (shipStaticInfo->beastAttacksRequired != (udword)-1
         && shipStaticInfo->beastAttackConvertTo == crystalShip->shiptype
         && (shipType < bRecon || shipType > bMissileCorvette))
        {
            return shipType;
        }
    }

    return -1;
}

void ProcessCrystalDockSlots(Ship *ship)
{
    CrystalDockState *state = dockGetCrystalDockState(ship);
    sdword i;

    if (state == NULL)
    {
        return;
    }

    for (i = 0; i < 16; i++)
    {
        CrystalDockSlot *slot = &state->slots[i];
        if (slot->ship == NULL)
        {
            continue;
        }
        if (slot->cleanupPending != 0)
        {
            SalCapCleanupHostQueueEntry(ship, i);
            slot->cleanupPending = 0;
            continue;
        }
        if (IsCrystal(slot->ship))
        {
            SalCapProcessCrystalQueueEntry(ship, i);
        }
        else if (slot->ship->shiptype == dMessagePod || slot->ship->shiptype == dBuoy)
        {
            SalCapProcessMessagePodOrBuoyQueueEntry(ship, i);
        }
        else if (slot->ship->shiptype == dBigGunUnit)
        {
            SalCapProcessBigGunUnitQueueEntry(ship, i);
        }
        else if (ship->playerowner->race == RACE_Beast)
        {
            SalCapProcessCapturedShipQueueEntryR1(ship, i);
        }
        else
        {
            SalCapProcessCapturedShipQueueEntryR2(ship, i);
        }
    }
}

void ClearCrystalDockSlots(Ship *ship)
{
    CrystalDockState *state = dockGetCrystalDockState(ship);
    sdword i;

    if (state == NULL)
    {
        return;
    }

    for (i = 0; i < 16; i++)
    {
        CrystalDockSlot *slot = &state->slots[i];
        Ship *slotShip = slot->ship;
        void (*processFunc)(Ship *, sdword);

        if (slotShip == NULL)
        {
            continue;
        }

        if (*dockGetCrystalDockSlotIndexPtr(slotShip) != -1)
        {
            dockFreeDockPoint(slotShip);
            *dockGetCrystalDockSlotIndexPtr(slotShip) = -1;
        }

        if (IsCrystal(slotShip))
        {
            CrystalDestroyProcessingEffect(slotShip);
        }
        else
        {
            if (slotShip->shiptype == dMessagePod || slotShip->shiptype == dBuoy)
            {
                processFunc = SalCapProcessMessagePodOrBuoyQueueEntry;
            }
            else if (slotShip->shiptype == dBigGunUnit)
            {
                processFunc = SalCapProcessBigGunUnitQueueEntry;
            }
            else if (ship->playerowner->race == RACE_Beast)
            {
                processFunc = SalCapProcessCapturedShipQueueEntryR1;
            }
            else
            {
                processFunc = SalCapProcessCapturedShipQueueEntryR2;
            }

            if (processFunc == SalCapProcessMessagePodOrBuoyQueueEntry || processFunc == SalCapProcessBigGunUnitQueueEntry)
            {
                *dockGetCrystalQueueIndexPtr(slotShip) = -1;
            }
            else if (processFunc == SalCapProcessCapturedShipQueueEntryR2)
            {
                slotShip->replacementStateActive = 0;
                slotShip->replacementStateEndTime = universe.totaltimeelapsed - 1.0f;
            }
            else
            {
                slotShip->flags = (slotShip->flags & 0xFF3FDFFF) | 0x2000;
            }
        }

        slotShip->deathtime = universe.totaltimeelapsed + 1.0f;
        *dockGetCrystalDockHostPtr(slotShip) = NULL;
        dockClearCrystalSlot(slot);
    }
}

sdword RemoveShipFromCrystalDockSlots(Ship *hostShip, Ship *crystalShip)
{
    CrystalDockState *state = dockGetCrystalDockState(hostShip);
    sdword i;

    if (state == NULL)
    {
        return 0;
    }

    if (state->assignedSlot != -1 && state->slots[state->assignedSlot].ship == crystalShip)
    {
        state->assignedSlot = -1;
    }

    for (i = 0; i < 16; i++)
    {
        CrystalDockSlot *slot = &state->slots[i];
        if (slot->ship == crystalShip)
        {
            if (*dockGetCrystalDockSlotIndexPtr(crystalShip) != -1)
            {
                dockFreeDockPoint(crystalShip);
                *dockGetCrystalDockSlotIndexPtr(crystalShip) = -1;
            }
            dockClearCrystalSlot(slot);
        }
    }
    return 0;
}

bool32 CrystalHasAssignedDockSlot(Ship *crystalShip)
{
    Ship *hostShip;
    CrystalDockState *state;

    if (crystalShip->shiptype != dBigGunUnit)
    {
        return FALSE;
    }

    hostShip = *dockGetCrystalDockHostPtr(crystalShip);
    if (hostShip == NULL)
    {
        return FALSE;
    }

    state = dockGetCrystalDockState(hostShip);
    return state != NULL && state->assignedSlot != -1;
}

real32 CrystalGetDockSlotProgress(Ship *crystalShip)
{
    Ship *hostShip = *dockGetCrystalDockHostPtr(crystalShip);
    CrystalDockState *state;
    CrystalDockSlot *slot;
    real32 result;

    if (hostShip == NULL)
    {
        return 0.0f;
    }

    state = dockGetCrystalDockState(hostShip);
    if (state == NULL || state->assignedSlot < 0 || state->assignedSlot >= 16)
    {
        return 0.0f;
    }

    slot = &state->slots[state->assignedSlot];
    if (slot->endTime <= slot->startTime)
    {
        return 0.0f;
    }

    result = (universe.totaltimeelapsed - slot->startTime) / (slot->endTime - slot->startTime);
    if (result >= 1.0f)
    {
        return 1.0f;
    }
    if (result <= 0.0f)
    {
        return 0.0f;
    }
    return result;
}

sdword dockDisableShipForLaunch(Ship *ship, sdword allowDelay)
{
    return shipDisable(ship, allowDelay);
}
