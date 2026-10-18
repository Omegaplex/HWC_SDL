/*=============================================================================
    Name    : MissionScriptSupport.c
    Purpose : Shared single-player mission-script support and campaign glue
=============================================================================*/

#include "MissionScriptSupport.h"
#include "..\Generated\mission01.h"
#include "..\Generated\mission02.h"
#include "..\Generated\mission03.h"
#include "..\Generated\mission04.h"
#ifdef OEM
#include "..\Generated\mission05_OEM.h"
#else
#include "..\Generated\mission05.h"
#include "..\Generated\mission06.h"
#include "..\Generated\mission07.h"
#include "..\Generated\mission08.h"
#include "..\Generated\mission09.h"
#include "..\Generated\mission10.h"
#include "..\Generated\mission11.h"
#include "..\Generated\mission12.h"
#include "..\Generated\mission13.h"
#include "..\Generated\mission14.h"
#include "..\Generated\mission15.h"
#include "..\Generated\mission16.h"
#include "..\Generated\mission17.h"
#endif
#include "..\Generated\tutorial1.h"

#undef CATACLYSM_DIAG
#define CATACLYSM_DIAG 1


void kasfTutSetPointerTargetAIVolumeNoLine(char *label, Volume *volume)
{
    tutSetPointerTargetAIVolume(label, volume, 0);
}
void kasfTutSetPointerTargetAIVolumeWithLine(char *label, Volume *volume)
{
    tutSetPointerTargetAIVolume(label, volume, 1);
}

int kasCheckShipDockFlag(int a1)
{
    return ((((Ship *)(*(int *)(*(int *)(a1 + 4) + 8)))->specialFlags2 & SPECIAL_2_DockFlag) != 0);
}

sdword kasShipSelectPredicate(Ship *ship, sdword arg1, sdword arg2)
{
    ShipType shiptype = ship->shiptype;
    if (arg1)
    {
        if (ship->playerowner->playerIndex)
        {
            if (shiptype != sSwarmer && shiptype != dCrystal0 && shiptype != dCrystal1)
                return 1;
        }
    }
    else
    {
        if (!ship->playerowner->playerIndex && shiptype != sSwarmer)
            return 1;
    }
    return 0;
}

sdword kasSimpleShipSelectPredicate(Ship *ship, sdword arg1, sdword arg2)
{
    ShipType shiptype = ship->shiptype;
    if (arg1)
    {
        if (!ship->playerowner->playerIndex)
            return 0;
        if (shiptype == sSwarmer || shiptype == dCrystal0 || shiptype == dCrystal1)
            return 0;
        return ship->playerowner->playerIndex;
    }
    return !ship->playerowner->playerIndex && shiptype != sSwarmer;
}

int kasSelectEnemyShips(int *result, int source)
{
    return kasfShipsSelectByPredicate((GrowSelection *)result, (GrowSelection *)source, kasShipSelectPredicate, 1, 0);
}

int kasSelectPlayerShips(int *result, int source)
{
    return kasfShipsSelectByPredicate((GrowSelection *)result, (GrowSelection *)source, kasShipSelectPredicate, 0, 0);
}

int kasSelectPlayerShipsSimple(int *result, int source)
{
    return kasfShipsSelectByPredicate((GrowSelection *)result, (GrowSelection *)source, kasSimpleShipSelectPredicate, 0, 0);
}

void MaintainTeamContext(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
}

sdword Mission02ShipSelectPredicate(Ship *ship, sdword arg1)
{
    ShipType shiptype;

    if (arg1)
    {
        if (ship->playerowner->playerIndex && (ship->specialFlags4 & 0x2000000) == 0)
        {
            shiptype = ship->shiptype;
            if (shiptype != sSwarmer && shiptype != dCrystal0 && shiptype != dCrystal1)
                return 1;
        }
    }
    else if (!ship->playerowner->playerIndex &&
             shipGetSlotStatus(ship, 1) &&
             !shipIsModuleSlotActive(ship, universe.curPlayerPtr) &&
             (ship->specialFlags4 & 0x2000000) == 0 &&
             ship->shiptype != sSwarmer)
    {
        return 1;
    }
    return 0;
}

sdword Mission02ShipsSelect(GrowSelection *dest, GrowSelection *src)
{
    return kasfShipsSelectByPredicate(dest, src,
        (sdword (*)(Ship *, sdword, sdword))Mission02ShipSelectPredicate, 0, 0);
}

void Init_QueueStandardFrigateBatch(void)
{
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    shipsToCreate = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(shipsToCreate);
    kasfShipsCreateInside(
        shipsToCreate,
        "tStandardFrigate",
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(45804546),
        1);
}

void Watch_StandardFrigateBatchComplete(void)
{
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    shipsToCreate = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(shipsToCreate) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, shipsToCreate);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission09_MissionFlow_GhostIntro);
    }
}

void Init_QueueMultigunCorvetteOrHeavyInterceptorBatch(void)
{
    char *shipType;
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    shipsToCreate = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(shipsToCreate);
    if (kasfThisTeamIs(kasAITeamPtrIfExists(204990129)))
    {
        shipType = "tMultiGunCorvette";
    }
    else
    {
        shipType = "tHeavyInterceptor";
    }
    kasfShipsCreateInside(
        shipsToCreate,
        shipType,
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(196001810),
        1);
}

void Watch_MultigunCorvetteOrHeavyInterceptorBatchComplete(void)
{
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    shipsToCreate = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(shipsToCreate) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, shipsToCreate);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission10_MissionFlow_FleetCommandIntro);
    }
}

void Init_SelectAndFocusProximitySensors(void)
{
    GrowSelection *currentTeamShips;
    GrowSelection *proximitySensors;
    GrowSelection *focusSelection;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    currentTeamShips = &((AITeam *)kasCurrentTeamPtr)->shipList;
    proximitySensors = kasGrowSelectionPtr(193452755);
    focusSelection = kasGrowSelectionPtr(126432768);
    kasfUnhideShips(currentTeamShips);
    kasfShipsSelectType(proximitySensors, currentTeamShips, "kProximitySensor");
    kasfShipsSelectIndex(focusSelection, proximitySensors, 0);
    kasfMoveTo(kasShipsVectorPtrIfExists(126432768));
    kasTimerCreateSetStartById(150120898, 6);
}

void Watch_ProximitySensorContactLoop(void)
{
    GrowSelection *nearbyShips;
    GrowSelection *nearbySensors;
    GrowSelection *proximitySensors;
    GrowSelection *focusSelection;
    GrowSelection *currentTeamShips;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredDestroyById(150120898))
    {
        nearbyShips = kasGrowSelectionPtr(86599289);
        currentTeamShips = &((AITeam *)kasCurrentTeamPtr)->shipList;
        kasfFindShipsNearPoint(nearbyShips, kasThisTeamsVectorPtr(), 10000);
        if (kasfShipsCount(nearbyShips) > 0)
        {
            if (!kasfEntryValueGet(71552180))
            {
                nearbySensors = kasGrowSelectionPtr(342540);
                if (kasfShipsSelectType(nearbySensors, nearbyShips, "kProximitySensor"))
                {
                    kasfEntryGetOrCreateSetValue(71552180, 1);
                    kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)253349863);
                }
            }
            tutDeallocateRootRegion();
        }
        else
        {
            proximitySensors = kasGrowSelectionPtr(193452755);
            if (kasfShipsSelectType(proximitySensors, currentTeamShips, "kProximitySensor"))
            {
                focusSelection = kasGrowSelectionPtr(126432768);
                kasfShipsSelectIndex(focusSelection, proximitySensors, 0);
                kasfMoveTo(kasShipsVectorPtrIfExists(126432768));
            }
        }
        kasTimerSetById(150120898, 6);
        kasTimerStartById(150120898);
    }
}

void Init_QueueHeavyInterceptorBatch(void)
{
    GrowSelection *heavyInterceptorBatch;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    heavyInterceptorBatch = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(heavyInterceptorBatch);
    kasfShipsCreateInside(
        heavyInterceptorBatch,
        "kHeavyInterceptor",
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(202030339),
        0);
}

void Watch_HeavyInterceptorBatchComplete(void)
{
    GrowSelection *heavyInterceptorBatch;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    heavyInterceptorBatch = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(heavyInterceptorBatch) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, heavyInterceptorBatch);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_Mission10_AsteroidPocketAttackTeam_Intro);
    }
}

void Init_ApplyUpgradesThenDockForLaunch(void)
{
    AITeam *team;
    sdword i;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    if (team->shipList.selection != NULL)
    {
        for (i = 0; i < team->shipList.selection->numShips; i++)
        {
            shipApplyPlayerUpgrades(team->shipList.selection->ShipPtr[i]);
        }
    }
    kasfSpeechEvent(5597651, (sdword)Init_DockCurrentTeamForLaunch, (sdword)Watch_Mission10_MissionFlow_FirstWarning);
}

void Init_DockCurrentTeamForLaunch(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(196001810));
}

void Init_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor(void)
{
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfStop();
    shipsToCreate = kasGrowSelectionPtr(21159);
    kasfGrowSelectionClear(shipsToCreate);
    kasfShipsCreateInside(
        shipsToCreate,
        "tHeavyInterceptor",
        kasfTeamCountOriginal(),
        kasAITeamShipsPtrIfExists(114551810),
        1);
}

void Watch_AsteroidPocketEncounter_ReplacementHeavyInterceptorReady(void)
{
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    shipsToCreate = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(shipsToCreate) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, shipsToCreate);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_AsteroidPocketEncounter_ResumeAttackers);
    }
}

void Watch_AsteroidPocketEncounter_ResumeAttackers(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        kasfSpeechEvent(
            139967671,
            (sdword)Watch_Mission10_AsteroidPocketEncounter_SelectAttackers,
            (sdword)Watch_Mission10_AsteroidPocketEncounter_UpdateAttackers);
    }
}

void Init_AsteroidPocketEncounter_FocusThreatWarning(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfDockInstantWithTeam(kasAITeamPtrIfExists(114551810));
    kasfEntryGetOrCreateSetValue(-1970110140, 1);
#if !defined(HW_Release) && CATACLYSM_DIAG
    dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission support focus threat warning team=%p total=%.2f",
        team,
        universe.totaltimeelapsed);
#endif
    kasfSpeechEvent(204400306, (sdword)MaintainTeamContext, (sdword)Watch_AsteroidPocketEncounter_ThreatWarningAcknowledged);
}

void Watch_AsteroidPocketEncounter_ThreatWarningAcknowledged(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 missionSupportDiagLastThreatAckLogTime = -1000.0f;
#endif
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)213247342))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission support threat warning acknowledged total=%.2f",
            universe.totaltimeelapsed);
#endif
        kasfLaunch();
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_AsteroidPocketEncounter_BeginDefenseLoop);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    else if (universe.totaltimeelapsed - missionSupportDiagLastThreatAckLogTime >= 10.0f)
    {
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission support threat warning waiting total=%.2f",
            universe.totaltimeelapsed);
        missionSupportDiagLastThreatAckLogTime = universe.totaltimeelapsed;
    }
#endif
}

sdword singlePlayerShipsAnyIdle(GrowSelection *ships)
{
    SelectCommand *selection;
    sdword i;

    if (ships == NULL || ships->selection == NULL)
    {
        return 0;
    }
    selection = ships->selection;
    for (i = 0; i < selection->numShips; i++)
    {
        if (selection->ShipPtr[i]->command == NULL
         || selection->ShipPtr[i]->command->ordertype.order == COMMAND_NULL)
        {
            return 1;
        }
    }
    return 0;
}

void singlePlayerSortShipsByDistanceToPoint(GrowSelection *ships, hvector *point)
{
    vector sortPoint;
    Ship *swapShip;
    sdword i, j;

    if (ships == NULL || ships->selection == NULL || ships->selection->numShips <= 1)
    {
        return;
    }
    vecGrabVecFromHVec(sortPoint, *point);
    for (i = 1; i < ships->selection->numShips; i++)
    {
        for (j = 0; j < i; j++)
        {
            if (aiuFindDistanceSquared(ships->selection->ShipPtr[i]->posinfo.position, sortPoint)
             < aiuFindDistanceSquared(ships->selection->ShipPtr[j]->posinfo.position, sortPoint))
            {
                swapShip = ships->selection->ShipPtr[j];
                ships->selection->ShipPtr[j] = ships->selection->ShipPtr[i];
                ships->selection->ShipPtr[i] = swapShip;
            }
        }
    }
}

sdword singlePlayerShipIsMemberOfSelection(Ship *ship, sdword arg1, sdword arg2)
{
    GrowSelection *selection;
    sdword i;

    selection = (GrowSelection *)arg1;
    if (selection == NULL || selection->selection == NULL || selection->selection->numShips <= 0)
    {
        return 0;
    }
    for (i = 0; i < selection->selection->numShips; i++)
    {
        if (selection->selection->ShipPtr[i] == ship)
        {
            return 1;
        }
    }
    return 0;
}

sdword singlePlayerShipIsMarkedInParade(Ship *ship, sdword arg1, sdword arg2)
{
    if (!(ship->specialFlags & SPECIAL_SinglePlayerInParade))
    {
        return 0;
    }
    bitClear(ship->specialFlags, SPECIAL_SinglePlayerInParade);
    return 1;
}

sdword singlePlayerSelectShipsInParade(GrowSelection *result, GrowSelection *source)
{
    return kasfShipsSelectByPredicate(
        result,
        source,
        singlePlayerShipIsMarkedInParade,
        0,
        0);
}

void singlePlayerTeamSpecialToggle(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    if (team != NULL && team->shipList.selection != NULL && team->shipList.selection->numShips > 0)
    {
        clWrapSpecial(&universe.mainCommandLayer, team->shipList.selection, 0);
    }
}

void sub_496050(void)
{
    singlePlayerTeamSpecialToggle();
}

void sub_652EF0(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
}

void sub_60C010(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfStop();
    kasfEntrySetValue(332667, 1);
}

void singlePlayerShipsSetHyperspaceDestinationToCurrentPosition(GrowSelection *ships)
{
    if (ships == NULL)
    {
        return;
    }
}

void singlePlayerShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation)
{
    SelectCommand *shipsSelection;
    SelectCommand *formationSelection;
    CommandToDo *militaryGroup;
    SelectCommand selectionCommand;
    sdword i;

    if (ships == NULL || formation == NULL || ships->selection == NULL || formation->selection == NULL)
    {
        return;
    }
    shipsSelection = ships->selection;
    formationSelection = formation->selection;
    if (shipsSelection->numShips <= 0 || formationSelection->numShips <= 0)
    {
        return;
    }

    for (i = 0; i < formationSelection->numShips; i++)
    {
        memset(&selectionCommand, 0, sizeof(selectionCommand));
        selectionCommand.numShips = 1;
        selectionCommand.ShipPtr[0] = formationSelection->ShipPtr[i];
        RemoveShipsFromDoingStuff(&universe.mainCommandLayer, &selectionCommand);
        aitrackForceSteadyShip(formationSelection->ShipPtr[i]);
        GroupShipIntoMilitaryParade(
            &universe.mainCommandLayer,
            formationSelection->ShipPtr[i],
            shipsSelection->ShipPtr[0]);
    }

    militaryGroup = GetMilitaryGroupAroundShip(&universe.mainCommandLayer, shipsSelection->ShipPtr[0]);
    setMilitaryParade(militaryGroup);
}

sdword kasfFindShipsTargetingSelection(GrowSelection *targets, GrowSelection *ships)
{
    Node *node;
    CommandToDo *command;
    sdword i;

    if (targets == NULL || ships == NULL || targets->selection == NULL)
    {
        return 0;
    }

    kasfGrowSelectionClear(ships);
    if (targets->selection->numShips <= 0)
    {
        return 0;
    }

    for (node = universe.mainCommandLayer.todolist.head; node != NULL; node = node->next)
    {
        command = (CommandToDo *)node->structptr;
        if ((command->ordertype.order == COMMAND_ATTACK
          || (command->ordertype.attributes & (COMMAND_IS_PASSIVEATTACKING | COMMAND_IS_ATTACKINGANDMOVING)) != 0)
         && command->attack != NULL
         && AnyOfTheseShipsAreInSelection(targets->selection, (SelectCommand *)command->attack))
        {
            for (i = 0; i < command->selection->numShips; i++)
            {
                growSelectAddShipNoDuplication(&ships->maxNumShips, command->selection->ShipPtr[i]);
            }
        }
    }
    return ships->selection->numShips;
}

sdword singlePlayerFindMissilesInside(Volume *vol, GrowSelection *ships)
{
    Node *node;
    Ship *ship;

    if (ships == NULL)
    {
        return 0;
    }

    kasGrowSelectionClear(ships);
    for (node = universe.MissileList.head; node != NULL; node = node->next)
    {
        ship = (Ship *)node->structptr;
        if (ship->objtype == OBJ_MissileType && volPointInside(vol, &ship->posinfo.position))
        {
            growSelectAddShip(ships, ship);
        }
    }

    return ships->selection->numShips;
}

void Init_AsteroidPocketEncounter_DefenseLoop(void)
{
    AITeam *team;
    GrowSelection *workers;
    GrowSelection *candidateAttackers;
    hvector *attackPoint;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfFormationDelta();
    workers = kasGrowSelectionPtr(205978147);
    candidateAttackers = kasGrowSelectionPtr(57245931);
    kasfShipsSelectType(workers, kasGrowSelectionPtr((sdword)&loc_4A2A29), "sWorker");
    if (kasfShipsCount(workers))
    {
        kasfCurrentTeamAttack(workers);
    }
    else
    {
        kasfGrowSelectionClear(candidateAttackers);
        kasfShipsAdd(candidateAttackers, kasGrowSelectionPtr((sdword)&loc_4A2A29));
        kasfShipsSelectNotSalvagers(candidateAttackers, candidateAttackers);
        attackPoint = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
        kasfShipsSortByDistanceToPoint(candidateAttackers, (vector *)attackPoint);
        kasfCurrentTeamAttack(candidateAttackers);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    if (kasfShipsCount(workers))
    {
        attackPoint = kasShipsVectorPtrIfExists(205978147);
    }
    else
    {
        attackPoint = ((hvector *(__cdecl *)(void))kasShipsVectorPtr)();
    }
    dbgMessagef(
        "\n" CATACLYSM_DIAG_LABEL "mission support defense loop init team=%p live=%d workers=%d targets=%d dist=%d total=%.2f",
        team,
        kasfTeamCount(),
        kasfShipsCount(workers),
        kasfShipsCount(candidateAttackers),
        kasfFindDistance(kasThisTeamsVectorPtr(), attackPoint),
        universe.totaltimeelapsed);
#endif
    kasTimerCreateSetStartById(139352562, 7);
}

void Watch_AsteroidPocketEncounter_DefenseLoop(void)
{
    AITeam *team;
    GrowSelection *workers;
    GrowSelection *candidateAttackers;
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 missionSupportDiagLastDefenseActiveLogTime = -1000.0f;
#endif

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    if (kasfTeamCount())
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        if (universe.totaltimeelapsed - missionSupportDiagLastDefenseActiveLogTime >= 10.0f)
        {
            dbgMessagef(
                "\n" CATACLYSM_DIAG_LABEL "mission support defense loop active team=%p live=%d idle=%d total=%.2f",
                team,
                kasfTeamCount(),
                kasfShipsAnyIdle(&team->shipList),
                universe.totaltimeelapsed);
            missionSupportDiagLastDefenseActiveLogTime = universe.totaltimeelapsed;
        }
#endif
        if (kasfShipsAnyIdle(&team->shipList))
        {
            workers = kasGrowSelectionPtr(205978147);
            candidateAttackers = kasGrowSelectionPtr(57245931);
            kasfShipsSelectType(workers, kasGrowSelectionPtr((sdword)&loc_4A2A29), "sWorker");
            if (kasfShipsCount(workers))
            {
                kasfCurrentTeamAttack(workers);
            }
            else
            {
                kasfGrowSelectionClear(candidateAttackers);
                kasfShipsAdd(candidateAttackers, kasGrowSelectionPtr((sdword)&loc_4A2A29));
                kasfShipsSelectNotSalvagers(candidateAttackers, candidateAttackers);
                kasfShipsSortByDistanceToPoint(candidateAttackers, (vector *)((hvector *(__cdecl *)(void))kasShipsVectorPtr)());
                kasfCurrentTeamAttack(candidateAttackers);
            }
            kasTimerSetById(139352562, 7);
            kasTimerStartById(139352562);
        }
    }
    else if (kasfEntryValueGet(-1970110140) >= 3)
    {
        kasfSpeechEvent(342540, (sdword)MaintainTeamContext, (sdword)MaintainTeamContext);
    }
    else
    {
        kasfEntryIncrement(-1970110140);
        kasfSpeechEvent(
            142812005,
            (sdword)Init_AsteroidPocketEncounter_ReinforcementDelay,
            (sdword)Watch_AsteroidPocketEncounter_ReinforcementDelay);
    }
}

void Init_AsteroidPocketEncounter_ReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasTimerCreateSetStartById(148949186, 45);
}

void Watch_AsteroidPocketEncounter_ReinforcementDelay(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(148949186))
    {
        kasfSpeechEvent(
            75929221,
            (sdword)Init_AsteroidPocketEncounter_QueueReplacementHeavyInterceptor,
            (sdword)Watch_AsteroidPocketEncounter_ReinforcementReady);
    }
}

void Watch_AsteroidPocketEncounter_ReinforcementReady(void)
{
    GrowSelection *shipsToCreate;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    shipsToCreate = kasGrowSelectionPtr(21159);
    if (kasfShipsCount(shipsToCreate) >= kasfTeamCountOriginal())
    {
        kasfReinforceTeamWithShips((AITeam *)kasCurrentTeamPtr, shipsToCreate);
        kasfSpeechEvent(84320888, (sdword)MaintainTeamContext, (sdword)Watch_AsteroidPocketEncounter_BeginDefenseLoop);
    }
}

void Watch_AsteroidPocketEncounter_BeginDefenseLoop(void)
{
#if !defined(HW_Release) && CATACLYSM_DIAG
    static real32 missionSupportDiagLastDefenseLoopLogTime = -1000.0f;
#endif
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfTeamFinishedLaunching())
    {
        if (!kasfEntryValueGet(71552181))
        {
            kasfEntryGetOrCreateSetValue(71552181, 1);
            kasfMsgSend(kasAITeamPtrIfExists(176850956), (char *)10664497);
        }
        kasfSpeechEvent(
            139967671,
            (sdword)Init_AsteroidPocketEncounter_DefenseLoop,
            (sdword)Watch_AsteroidPocketEncounter_DefenseLoop);
    }
#if !defined(HW_Release) && CATACLYSM_DIAG
    else if (universe.totaltimeelapsed - missionSupportDiagLastDefenseLoopLogTime >= 10.0f)
    {
        dbgMessagef(
            "\n" CATACLYSM_DIAG_LABEL "mission support defense loop waiting team=%p live=%d total=%.2f",
            kasCurrentTeamPtr,
            kasfTeamCount(),
            universe.totaltimeelapsed);
        missionSupportDiagLastDefenseLoopLogTime = universe.totaltimeelapsed;
    }
#endif
}

void Init_AsteroidPocketEncounter_InitialAttackerWarning(void)
{
    AITeam *team;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 1;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsApplyPlayerUpgrades(&team->shipList);
    kasfSpeechEvent(244895509, (sdword)MaintainTeamContext, (sdword)Watch_AsteroidPocketEncounter_InitialAttackerWarningAcknowledged);
}

void Watch_AsteroidPocketEncounter_InitialAttackerWarningAcknowledged(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasfMsgReceived((char *)237703397))
    {
#if !defined(HW_Release) && CATACLYSM_DIAG
        dbgMessagef("\n" CATACLYSM_DIAG_LABEL "mission01 initial attacker warning acknowledged total=%.2f", universe.totaltimeelapsed);
#endif
        kasfSpeechEvent(57056661, (sdword)Init_AsteroidPocketEncounter_SelectInitialAttackers, (sdword)MaintainTeamContext);
    }
}

void Init_AsteroidPocketEncounter_SelectInitialAttackers(void)
{
    AITeam *team;
    GrowSelection *attackersInPocket;
    GrowSelection *candidateAttackers;

    team = (AITeam *)kasCurrentTeamPtr;
    kasCurrentStep = 2;
    kasCurrentScopeHash = team->kasLabelHash;
    kasfShipsSetDamageFactor(&team->shipList, 10);
    attackersInPocket = kasGrowSelectionPtr(22816);
    candidateAttackers = kasGrowSelectionPtr(57245931);
    kasfGrowSelectionClear(attackersInPocket);
    kasfGrowSelectionClear(candidateAttackers);
    kasfFindShipsInside(kasVolumePtr(151951029), attackersInPocket);
    kasfShipsSelectMissionVolumeInactivePlayerModules(
        (int *)&candidateAttackers->maxNumShips,
        (int)attackersInPocket);
    kasfCurrentTeamAttack(kasGrowSelectionPtr(57245931));
}

void Init_AsteroidPocketEncounter_ClearCurrentSelectionHighlight(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    kasfShipsClearDamageFactor(&((AITeam *)kasCurrentTeamPtr)->shipList);
}

void FailMissionIfTrackedSelectionAlive(void)
{
  kasCurrentStep = 2;
  kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
  if ( kasfShipsCount((sdword)kasGrowSelectionPtr(211966048)) )
    kasfMissionFailed();
}

void Begin_BeastIonArrayFrigateCombatLoop(void)
{
    GrowSelection *capitalTargets;
    hvector *moveTarget;

    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    capitalTargets = kasGrowSelectionPtr(124053532);
    kasfShipsSelectCapital(capitalTargets, &((AITeam *)kasCurrentTeamPtr)->shipList);
    singlePlayerSortShipsByDistanceToPoint(capitalTargets, kasThisTeamsVectorPtr());
    kasfFormationWall();
    if (kasfShipsSelectIndex(kasGrowSelectionPtr(21541), capitalTargets, 0))
    {
        moveTarget = kasShipsVectorPtrIfExists(21541);
    }
    else
    {
        moveTarget = kasShipsVectorPtrIfExists(218060896);
    }
    kasfMoveTo(moveTarget);
    kasTimerDestroyById(337061);
    kasTimerCreateSetStartById(337061, kasfRandom(10, 20));
}
