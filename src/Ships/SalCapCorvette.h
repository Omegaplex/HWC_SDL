// =============================================================================
//  SalCapCorvette.h
// =============================================================================
//  Copyright Relic Entertainment, Inc. All rights reserved.
//  Created 11/5/1997 by khent
// =============================================================================

#ifndef ___SALCAPCORVETTE_H
#define ___SALCAPCORVETTE_H

#include "Attack.h"
#include "ShipSelect.h"
#include "Slipgate.h"
#include "render.h"

#define SALVAGE_AT_GET_TECH       0x01

#define SAL_WL_FLYTOCONEORIGIN    7

typedef struct
{
    AttackSideStep attacksidestep;
    sdword salvageState;               //sal/cap corv aistate for salvaging
    SpaceObjRotImpTargGuidanceShipDerelict *target;
    Ship *dockwith;
    sdword dockindex;
    sdword salvageIndex;
    sdword salvageAttributes;
    real32 timeCounter;
    real32 getTechTime;
    sdword tractorBeam;
    sdword noAvoid;
    real32 noDamageTargetTime;
    sdword groupme;
	SpaceObjRotImpTargGuidanceShipDerelict *noDamageTarget;
    real32 beamRange;
} SalCapCorvetteSpec;

typedef struct
{
    AttackSideStepParameters sidestepParameters;
    real32 HealthThreshold;
    real32 healthRemovedPerSecond;
    real32 getTechTime;
    real32 flyToDistance;
    real32 maxPushingVelocitySingle;
    real32 noLightClampingDistance;
} SalCapCorvetteStatics;

extern CustShipHeader JunkYardDawgHeader;
extern CustShipHeader SalCapCorvetteHeader;

ubyte *SalvagerFunc(Ship *ship);
bool32 salCapCantDoFormation(Ship *ship);
bool32 salCapIsActiveOrBeaming(Ship *ship);
bool32 SalCapCanTargetShip(struct Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target);
bool32 SalCapCanTargetShipWithReasonCounts(struct Player *player, SpaceObjRotImpTargGuidanceShipDerelict *target, uword *reasonCounts);
bool32 SalCapTargetSupportsSalvageAttachment(SpaceObjRotImpTargGuidanceShipDerelict *target);
bool32 SalCapExecuteSpecialOrder(Ship *ship, void *custom);
bool32 DropTargetInShip(Ship *dockwith,sdword *targetDepotState, SpaceObjRotImpTargGuidanceShipDerelict *target,sdword *dockindex);
bool32 isThereAnotherTargetForMe(Ship *ship,SelectAnyCommand *targets);
sdword Fix_SalvagerSpecifics(Ship *ship);
sdword PreFix_SalvagerSpecifics(Ship *ship);
sdword SalCapDropTargetAndRememberPreviousTarget(Ship *ship);
sdword SalCapHandleProcessingShipRemoved(Ship *ship, Ship *removedShip);
ubyte SalCapProcessHostQueueScriptData(Ship *ship);
void salCapCleanUpCloakingTarget(Ship *ship, Ship *shiptoremove);
void salCapClearTechBool();
void SalCapDropTargetCallback(Ship *ship);
void SalCapDropTarget(Ship *ship);
void SalCapCorvette_RegisterExtraSpaceObjs(Ship *ship);
void salCapExtraSpecialOrderCleanUp(SelectCommand *selection,udword ordertype,Ship *dockwith,SelectCommand *targets);
void salCapHarvestTarget(SpaceObjRotImpTargGuidanceShipDerelict *target,Ship *dockwith);
sdword salCapNeedBig(Ship *ship,SpaceObjRotImpTargGuidanceShipDerelict *target);
void SalCapOrderChangedCleanUp(Ship *ship);
void salCapRemoveDerelictReferences(Ship *ship,Derelict *d);
Ship *SalCapGetFirstAttachedShip(Ship *target);
Ship *SalCapGetNextAttachedShip(void);
Ship *SalCapGetProcessingShip(void);
Ship *SalCapSetProcessingShip(Ship *ship);
Ship *SalCapNotifyProcessingShipRemoved(Ship *ship);
Ship *salCapSetCaptureRef(Ship *ship, Ship *captureRef);
bool32 salCapIsCapturing(Ship *ship);
sdword SalCapCleanupHostQueueEntry(Ship *hostShip, sdword entryIndex);
udword CrystalDestroyProcessingEffect(Ship *crystal);
real32 *CrystalBuildProcessingSlicePlane(Ship *ship, real32 *planeEquation);
void SalCapProcessCrystalQueueEntry(Ship *hostShip, sdword entryIndex);
void SalCapProcessBigGunUnitQueueEntry(Ship *hostShip, sdword entryIndex);
void SalCapProcessMessagePodOrBuoyQueueEntry(Ship *hostShip, sdword entryIndex);
void SalCapProcessCapturedShipQueueEntryR2(Ship *hostShip, sdword entryIndex);
void SalCapProcessCapturedShipQueueEntryR1(Ship *hostShip, sdword entryIndex);

#if RND_VISUALIZATION
void dockDrawSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *obj);
#endif

#endif
