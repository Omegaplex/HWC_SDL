#ifndef ___KASFUNC_H
#define ___KASFUNC_H

#include "AIUtilities.h"
#include "ShipSelect.h"
#include "Volume.h"

//
//  --------------------------------
//  these are for the placeholder for a real fleet intelligence message window
//
//
//  PETER:  we can nuke all this now, right?
//

// seconds
#define POPUPTEXT_DURATION_PER_LINE  4

// chars
#define POPUPTEXT_MAX_LINE_LENGTH  128
#define POPUPTEXT_MAX_LINES  16

// pixels
#define POPUPTEXT_BOX_WIDTH  200

void kasfPopupTextDraw(void);
void kasfPopupText(char *line);
void kasfPopupTextClear(void);

//
//  --------------------------------
//


//
//  new function prototypes also need to go into KAS2C.C so the
//  translator will recognize them
//

void kasfMissionCompleted(void);
void kasfMissionFailed(void);

void kasfFadeToWhite(void);

void kasfVarCreate(char *name);
void kasfVarValueSet(char *name, sdword value);
void kasfVarCreateSet(char *name, sdword value);
void kasfVarValueInc(char *name);
void kasfVarValueDec(char *name);
sdword kasfVarValueGet(char *name);
void kasfVarDestroy(char *name);
sdword kasfEntryTableInit(void);
void kasfEntryTableSave(void);
void kasfEntryTableLoad(void);
sdword *kasfEntryFindByScopeAndId(sdword scope, sdword id);
sdword kasfEntryScopeName(sdword scopeName);

void kasfCommandMessage(char *message);

void kasfTimerCreate(char *name);
void kasfTimerSet(char *name, sdword duration);
void kasfTimerStart(char *name);
void kasfTimerCreateSetStart(char *name, sdword duration);
void kasfTimerStop(char *name);
void kasfHarvest(void);
sdword kasfTimerRemaining(char *name);
sdword kasfTimerExpired(char *name);
sdword kasfTimerExpiredDestroy(char *name);
void kasfTimerDestroy(char *name);

sdword kasfMsgReceived(char *msg);

void kasfAttackMothership(void);
void kasfAttack(GrowSelection *targets);
void kasfAttackFlank(GrowSelection *targets);
void kasfAttackHarass(void);
void kasfAttackSpecial(GrowSelection *targets);
void kasfCloak(GrowSelection *ships);
void kasfMoveAttack(GrowSelection *targets);
void kasfForceAttack(GrowSelection *ships, hvector *destination, sdword a3);
sdword kasfBulgeAttack(GrowSelection *targets, GrowSelection *bulgetargets, GrowSelection *attackers, sdword radius);
void kasfIntercept(GrowSelection *targets);
void kasfTargetDrop(void);
void kasfSetSwarmerTargets(GrowSelection *targets);
void kasfSwarmMoveTo(GrowSelection *targets);
void kasfShipsAttack(GrowSelection *targets, GrowSelection *attackers);
void kasfCurrentTeamAttack(GrowSelection *targets);

void kasfFormationDelta(void);
void kasfFormationBroad(void);
void kasfFormationDelta3D(void);
void kasfFormationClaw(void);
void kasfFormationWall(void);
void kasfFormationSphere(void);
void kasfFormationCustom(GrowSelection *shipsg);

sdword kasfRandom(sdword lowestNum, sdword highestNum);

void kasfGuardMothership(void);
void kasfGuardShips(GrowSelection *ships);

void kasfPatrolPath(Path *path);
void kasfPatrolActive(void);
void kasfTeamSpecialToggle(void);

void kasfLog(char *string);
void kasfLogInteger(char *string, sdword integer);

sdword kasfTeamHealthAverage(void);
sdword kasfTeamHealthLowest(void);
sdword kasfTeamFuelAverage(void);
sdword kasfTeamFuelLowest(void);
sdword kasfTeamCount(void);
sdword kasfTeamCountOriginal(void);
sdword kasfNewShipsAdded(void);
struct AITeamMove *kasfTeamSetShips(struct AITeam *team, GrowSelection *ships);

void kasfGrowSelectionClear(GrowSelection *ships);

void kasfTeamAttributesBitSet(sdword attributes);
void kasfTeamAttributesBitClear(sdword attributes);
void kasfTeamAttributesSet(sdword attributes);

void kasfShipsAttributesBitSet(GrowSelection *ships,sdword attributes);
void kasfShipsAttributesBitClear(GrowSelection *ships,sdword attributes);
void kasfShipsAttributesSet(GrowSelection *ships,sdword attributes);

sdword kasfShipsDisabled(GrowSelection *ships);
void kasfDisableShips(GrowSelection *ships);
sdword kasfShipsAnyIdle(GrowSelection *ships);

sdword kasfShipsCount(GrowSelection *ships);

sdword kasfPointInside(Volume *volume, hvector *location);

sdword kasfFindShipsInside(Volume *volume, GrowSelection *ships);
sdword kasfFindMissilesInside(Volume *volume, GrowSelection *ships);
sdword kasfFindMissilesNearTeam(GrowSelection *ships, sdword radius);
sdword kasfFindEnemyShipsInsideVolume(Volume *volume, GrowSelection *ships);
sdword kasfFindEnemiesInside(Volume *volume, GrowSelection *ships, sdword neighborRadius);
sdword kasfFindEnemiesNearby(GrowSelection *ships, sdword radius);
sdword kasfFindEnemiesNearTeam(GrowSelection *ships, sdword radius);
sdword kasfFindEnemiesNearCurrentTeam(GrowSelection *ships, sdword radius);
sdword kasfModuleSlotOrderSetShipType(sdword orderGroup, char *shipType);
sdword kasfModuleSlotOrderSetCount(sdword orderGroup, sdword count);
sdword kasfActivateTech(char *techName);
sdword kasfFindEnemyMothership(GrowSelection *result);
sdword kasfFindEnemyShipsOfType(GrowSelection *ships, char *shipType);
sdword kasfFindFriendlyShipsOfType(GrowSelection *ships, char *shipType);
sdword kasfFindEnemyShipsOfClass(GrowSelection *ships, char *shipClass);
sdword kasfFindFriendlyShipsOfClass(GrowSelection *ships, char *shipClass);
void kasfFindBeastShipsInside(Volume *volume, GrowSelection *result);
sdword kasfFindShipsNearPoint(GrowSelection *ships, hvector *location, sdword radius);

void kasfTeamSkillSet(sdword skillLevel);
sdword kasfTeamSkillGet(void);

void kasfDisablePlayerHyperspace(void);
void kasfHoldHyperspaceWindow(bool32 hold);
void kasfShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation);
sdword kasfGetShipTypeCount(char *shipType, sdword playerIndex);
sdword kasfGetReplacementShipTypeCount(char *shipType, sdword playerIndex);
sdword kasfWatchTutorial1Tutorial10CameraMoveDescription(char *techName);
void kasfTeamHyperspaceIn(hvector *destination);
void kasfTeamHyperspaceInNear(hvector *destination, sdword distance);
void kasfTeamHyperspaceOut(void);
void kasfHyperspaceDelay(sdword milliseconds);
void kasfShipsHyperspaceIn(GrowSelection *ships, hvector *destination);
void kasfShipsSetHyperspaceDestination(GrowSelection *ships, hvector *destination);

void kasfGateDestroy(hvector *gatePoint);
void kasfGateShipsIn(GrowSelection *ships, hvector *gatePoint);
void kasfGateShipsOut(GrowSelection *ships, hvector *gatePoint);
void kasfGateMoveToNearest(void);
sdword kasfGateShipsOutNearest(GrowSelection *ships);

void kasfMissionSkillSet(sdword skillLevel);
sdword kasfMissionSkillGet(void);

void kasfRequestShips(char *shipType, sdword numShips);
void kasfRequestShipsOriginal(sdword percentOriginal);

void kasfReinforce(struct AITeam *team);

void kasfTeamGiveToAI(void);

void kasfDisableAIFeature(sdword feature, sdword type);
void kasfEnableAIFeature(sdword feature, sdword type);
void kasfDisableAllAIFeatures(void);
void kasfEnableAllAIFeatures(void);

void kasfTeamSwitchPlayerOwner(bool32 fullTransferToPlayer);
void kasfShipsSwitchPlayerOwner(GrowSelection *ships, bool32 fullTransferToPlayer);

void kasfReinforceTeamWithShips(struct AITeam *teamtoreinforce,GrowSelection *shipstoadd);
void kasfTeamMakeCrazy(sdword makecrazy);
void kasfShipsMakeCrazy(GrowSelection *ships, sdword makecrazy);

void kasfForceCombatStatus(GrowSelection *ships, sdword on);

sdword kasfThisTeamIs(struct AITeam *team);

void kasfMoveTo(hvector *destination);
void kasfShipsMoveTo(GrowSelection *ships, hvector *destination);
void kasfProtect(GrowSelection *targets);

void kasfTacticsAggressive(void);
void kasfTacticsNeutral(void);
void kasfTacticsEvasive(void);

sdword kasfNearby(hvector *location, sdword distance);
sdword kasfFindDistance(hvector *location1, hvector *location2);

sdword kasfUnderAttack(GrowSelection *attackers);
sdword kasfUnderAttackElsewhere(struct AITeam *otherTeam, GrowSelection *attackers);

sdword kasfShipsCountType(GrowSelection *ships, char *shipType);

void kasfDock(struct AITeam *withTeam);
void kasfDockSupport(void);
void kasfDockSupportWith(struct AITeam * withTeam);
void kasfShipsDockSupportWith(GrowSelection *ships, GrowSelection *withShips);
void kasfDockStay(struct AITeam * withTeam);
void kasfShipsDockStay(GrowSelection *ships, GrowSelection *withShips);
void kasfDockStayMothership(void);
void kasfDockInstant(struct AITeam * withTeam);
struct AITeamMove *kasfDockStayWithTeam(struct AITeam *withTeam);
struct AITeamMove *kasfDockInstantWithTeam(struct AITeam *withTeam);
bool32 kasfShipsDockWithSelection(GrowSelection *ships, GrowSelection *withShips);
void kasfShipsProtect(GrowSelection *ships, GrowSelection *targets);
void kasfShipsSetFormation(GrowSelection *ships, sdword formation);
void kasfShipsSetPlayerMothershipParade(GrowSelection *ships);
void kasfShipsClearPercentFueled(GrowSelection *ships);
sdword kasfSelectionIsInsideVolume(Volume *volume, GrowSelection *ships);
bool32 kasfTacticsSet(TacticsType tactics);
sdword kasfShipsAnyHyperspaceReady(GrowSelection *ships);
sdword kasfShipsAnyBigGunReady(GrowSelection *ships);
void kasfTDSalvageTrialStartWaitingForShips(GrowSelection *shipSelection, vector *position);
sdword kasfTDSalvageTrialWaitingForShipsDone(GrowSelection *shipSelection);
sdword kasfTDSalvageTrialGetWaitingForShipsPoints(GrowSelection *shipSelection, vector *fromPoint, vector *toPoint);
sdword kasfTDSalvageTrialWaitingForShipsActive(GrowSelection *shipSelection);
void kasfDisableMothershipBigGunLaunch(void);
void kasfLaunch(void);

sdword kasfTeamDocking(void);
sdword kasfTeamDockedReadyForLaunch(void);
sdword kasfTeamFinishedLaunching(void);
sdword kasfShipsFinishedLaunching(GrowSelection *ships);
bool32 kasfTeamIsIdle(struct AITeam *team);
bool32 std_ctype_char_Term(void);

void kasfMsgSend(struct AITeam *team, char *msg);
void kasfMsgSendAll(char *msg);

sdword kasfRUsEnemyCollected(void);

sdword kasfPathNextPoint(void);

sdword kasfWideScreenIsActive(void);

void kasfTeamSetMaxVelocity(sdword maxVelocity);
void kasfTeamClearMaxVelocity(void);
void kasfShipsSetMaxVelocity(GrowSelection *ships, sdword maxVelocity);
sdword kasfSetHyperspaceWindowHold(sdword hold);
void kasfShipsClearMaxVelocity(GrowSelection *ships, sdword maxVelocity);
void kasfShipsSetDamageFactor(GrowSelection *ships, sdword damagePercent);
void kasfShipsClearDamageFactor(GrowSelection *ships);
void kasfTeamSetPercentDamaged(sdword percentDamaged);
void kasfTeamSetPercentFueled(sdword percentFueled);
void kasfShipsSetPercentFueled(GrowSelection *ships, real32 percentFueled);
void kasfShipsSortByDistanceToPoint(GrowSelection *ships, vector *point);
GrowSelection *kasfShipsSpecialToggle(GrowSelection *ships);
sdword kasfShipsSpecial(GrowSelection *ships, GrowSelection *targets);
sdword kasfSpeechEventQueueCurrentPlayerUnlinked(sdword event, sdword shipType);
void kasfHaltAllNotDockedShips(void);
sdword kasfEnableShip(char *shipTypeName, char *shipRaceName, sdword bEnabled);
void kasfShipsSetPosition(GrowSelection *ships, sdword x, sdword y, sdword z);
void kasfShipsSetPositionFromVector(GrowSelection *ships, hvector *position);
void kasfShipsSetPositionNearVector(GrowSelection *ships, hvector *position, sdword distance);
sdword kasfShipSetTargetableFlag(GrowSelection *ships, sdword targetable);
GrowSelection *kasfShipsSetAttachVelocityFlag(GrowSelection *ships, sdword enable);
GrowSelection *kasfShipsSetAttachCoordsysFlag(GrowSelection *ships, sdword enable);
hvector *kasfVectorCrossNormalize(hvector *result, hvector *lhs, hvector *rhs);
sdword kasfVectorDotProductScaled100(hvector *lhs, hvector *rhs);
void kasfVectorBlendClampToUniverse(hvector *result, hvector *from, hvector *to, sdword numerator, sdword denominator);
void kasfVectorAddClampToUniverse(hvector *result, hvector *delta);
void kasfVectorSubtractClampToUniverse(hvector *result, hvector *delta);
hvector *kasfVectorScaleByRatio(hvector *vectorToScale, sdword numerator, sdword denominator);
sdword kasfVectorRoundX(hvector *value);
sdword kasfVectorRoundY(hvector *value);
sdword kasfVectorRoundZ(hvector *value);
sdword kasfVectorRoundW(hvector *value);

sdword kasfShipsSelectEnemy(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectFriendly(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectRace(GrowSelection *newShips, GrowSelection *originalShips, sdword race);
sdword kasfShipsSelectAttachedShips(GrowSelection *newShips, GrowSelection *source);
sdword kasfShipsSelectAttachedToSelection(GrowSelection *newShips, GrowSelection *originalShips, GrowSelection *attachedTo);
sdword kasfShipsSelectFuelBelowPercent(GrowSelection *newShips, GrowSelection *originalShips, sdword percent);
sdword kasfShipsSelectFirst(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectFirstN(GrowSelection *newShips, GrowSelection *originalShips, sdword count);
sdword kasfShipsSelectClass(GrowSelection *newShips, GrowSelection *originalShips, char *shipClass);
sdword kasfShipsSelectType(GrowSelection *newShips, GrowSelection *originalShips, char *shipType);
sdword kasfShipsSelectSpecialTargets(GrowSelection *result, GrowSelection *source);
sdword kasfShipsSelectLinkedWith(GrowSelection *originalShips, GrowSelection *newShips, GrowSelection *withShips);
sdword kasfShipsSelectDamaged(GrowSelection *newShips, GrowSelection *originalShips, sdword maxHealthPercent);
sdword kasfShipsSelectHealthAtLeastPercent(GrowSelection *newShips, GrowSelection *originalShips, sdword minHealthPercent);
sdword kasfShipsSelectMoving(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectCapital(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectMembersOfSelection(GrowSelection *newShips, GrowSelection *originalShips, GrowSelection *members);
sdword kasfShipsSelectAttackingSelection(GrowSelection *result, GrowSelection *source, GrowSelection *targets);
sdword kasfTeamSpecial(GrowSelection *targets);
sdword kasfShipsSelectNonCapital(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectNotDocked(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectSinglePlayerInParade(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectRowGettingOutOfWay(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectSinglePlayerCombatShips(GrowSelection *newShips, GrowSelection *originalShips);
sdword kasfShipsSelectIndex(GrowSelection *newShips, GrowSelection *originalShips, sdword Index);
sdword kasfShipsSelectNearby(GrowSelection *newShips, GrowSelection *originalShips, hvector *location, sdword distance);
sdword kasfFindEnemiesNearSelection(GrowSelection *source, sdword distance);
sdword kasfShipsDockInstant(GrowSelection *ships, GrowSelection *dockWith);
void kasfPingAddShipsIndexed(GrowSelection *ships, const char *label);

sdword kasfShipsOrder(GrowSelection *ships);
sdword kasfShipsOrderAttributes(GrowSelection *ships);
sdword kasfShipsAnyIdle(GrowSelection *ships);
sdword kasfShipsAnyDying(GrowSelection *ships);
sdword kasfShipsHealthAverage(GrowSelection *ships);
GrowSelection *kasfShipsShuffle(GrowSelection *ships);
GrowSelection *kasfShipsSetNotBumpable(GrowSelection *ships, bool32 enable);
GrowSelection *kasfShipsSetRotatableFlag(GrowSelection *ships, sdword enable);
sdword kasfShipDelayedDeathPending(GrowSelection *ships);
sdword kasfTutSetSelectedShips(GrowSelection *ships);
void kasfTutCameraFocusIgnoreZoom(GrowSelection *ships);

#define kasSpecial_ReturningTechnology  1  //ships are salcapcorvettes returning technology
#define kasSpecial_ShipsDisabled        2  //ships have been disabled (salcapcorvette has locked on)
#define kasSpecial_ShipsAreArmed        3  //ships have arms on them - includes GravWellGenerator as an armed ship
#define kasSpecial_ShipsDefected        4  //ships have defected to the other side
sdword kasfShipsSelectSpecial(GrowSelection *newShips, GrowSelection *originalShips, sdword SpecialFlag);


sdword kasfShipsAdd(GrowSelection *shipsA, GrowSelection *shipsB);
sdword kasfShipsRemove(GrowSelection *shipsA, GrowSelection *shipsB);

void kasfShipsSetResourcingFlag(sdword shipsAddress, bool32 flag);
void kasfShipsSetNonRetaliation(GrowSelection *ships);
void kasfShipsSetRetaliation(GrowSelection *ships);

void kasfPingAddSingleShip(GrowSelection *ships, char *label);
void kasfPingAddShips(GrowSelection *ships, char *label);
void kasfPingAddPoint(hvector *point, char *label);
void kasfPingRemove(char *label);

void kasfBuildControl(sdword on);
void kasfBuildingTeam(struct AITeam *builder);

void kasfStop(void);
void kasfShipsStop(GrowSelection *ships);
void kasfShipsSetSlaveableFlag(GrowSelection *ships, sdword slaveable);
sdword kasfShipTypeBuildTime(char *shipType);
sdword kasfShipTypeBuildCost(char *shipType);
sdword kasfShipsSelectTargets(GrowSelection *newShips, GrowSelection *targets);

void kasfKamikaze(GrowSelection *targets);
void kasfKamikazeEveryone(GrowSelection *targets);

void kasfPopup(char *text);
void kasfPopupInteger(char *text, sdword integer);

void kasfObjectiveCreate(char *label, char *briefText, char *fullText);
void kasfObjectiveCreateSecondary(char *label, char* briefText, char* fullText);
void kasfObjectiveSet(char *label, sdword status);
//void kasfObjectivePopupAll(void);
sdword kasfObjectiveGet(char *label);
sdword kasfObjectiveGetAll(void);
void kasfObjectiveDestroy(char *label);
void kasfObjectiveDestroyAll(void);

sdword kasfSpecialToggle(GrowSelection *ships);

void kasfShipsDamage(GrowSelection *ships, sdword points);

void kasfForceTaskbar(void);

// Tutorial helper functions for kas ------------
void kasfBuilderRestrictShipTypes(char *shipTypes);
void kasfBuilderUnrestrictShipTypes(char *shipTypes);
void kasfBuilderRestrictAll(void);
void kasfBuilderRestrictNone(void);
void kasfBuilderCloseIfOpen(void);
void kasfForceBuildShipType(char *shipType);

sdword kasfCameraGetAngleDeg(void);
sdword kasfCameraGetDeclinationDeg(void);
sdword kasfCameraGetDistance(void);

sdword kasfSelectNumSelected(void);
sdword kasfSelectIsSelectionShipType(sdword Index, char *shipType);
sdword kasfSelectContainsShipTypes(char *shipTypes);
sdword kasfSelectedShipsInFormation(sdword formation);
sdword kasfShipsInFormation(GrowSelection *ships, sdword formation);
sdword kasfShipGetModuleSlotTimer(GrowSelection *shipSelection);
sdword kasfShipDecrementModuleSlotTimer(GrowSelection *shipSelection);
sdword kasfShipGetModuleCount(GrowSelection *shipSelection);
sdword kasfShipCanStartModuleSlotTimer(GrowSelection *shipSelection);
sdword kasfShipTryStartModuleSlotTimer(GrowSelection *shipSelection);
GrowSelection *kasfShipsSetSupportFlag(GrowSelection *ships, sdword enable);
void kasfCreateBasicShockwave(hvector *position);
void kasfWideScreenCleanup(void);

void kasfTutSetPointerTargetXY(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYRight(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYBottomRight(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYTaskbar(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetXYFE(char *name, sdword x, sdword y);
void kasfTutSetPointerTargetShip(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipWithLine(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipSelection(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipHealth(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetShipGroup(char *name, GrowSelection *ships);
void kasfTutSetPointerTargetFERegion(char *name, char *pAtomName);
void kasfTutSetPointerTargetRect(char *name, sdword x0, sdword y0, sdword x1, sdword y1);
void kasfTutSetPointerTargetAIVolume(char *name, Volume *volume);
void kasfTutRemovePointer(char *name);
void kasfTutRemoveAllPointers(void);

void kasfTutSetTextDisplayBoxGame(sdword x, sdword y, sdword width, sdword height);
void kasfTutSetTextDisplayBoxFE(sdword x, sdword y, sdword width, sdword height);
void kasfTutSetTextDisplayBoxToSubtitleRegion(void);
void kasfTutShowText(char *szText);
void kasfTutHideText(void);
void kasfTutShowNextButton(void);
void kasfTutHideNextButton(void);
sdword kasfTutNextButtonClicked(void);
void kasfTutShowBackButton(void);
void kasfTutHideBackButton(void);
void kasfTutShowPrevButton(void);
void kasfTutSaveLesson(sdword Num, char *pName);
void kasfTutShowImages(char *szImages);
void kasfTutHideImages(void);

void kasfTutEnableEverything(void);
void kasfTutDisableEverything(void);
void kasfTutEnableFlags(char *pFlags);
void kasfTutDisableFlags(char *pFlags);
void kasfTutForceUnpaused(void);

sdword kasfTutGameSentMessage(char *commandNames);
void kasfTutResetGameMessageQueue(void);
sdword kasfTutContextMenuDisplayedForShipType(char *shipType);
void  kasfTutResetContextMenuShipTypeTest(void);
void kasfTutRedrawEverything(void);

sdword kasfBuildManagerShipTypeInBatchQueue(char *shipType);
sdword kasfBuildManagerShipTypeInBuildQueue(char *shipType);
sdword kasfBuildManagerShipTypeSelected(char *shipType);

sdword kasfTutCameraFocusedOnShipType(char *shipTypes);
void kasfTutCameraFocus(GrowSelection *ships);
void kasfTutCameraFocusDerelictType(char *derelictType);
void kasfTutCameraFocusFar(GrowSelection *ships);
void kasfTutCameraFocusCancel(void);

void kasfDisablePlayer(bool32 toggle);

sdword kasfTutShipsInView(GrowSelection *ships);
sdword kasfTutShipsTactics(GrowSelection *ships);

sdword kasfTutPieDistance(void);
sdword kasfTutPieHeight(void);

void kasfForceFISensors(void);
void kasfOpenSensors(sdword flag);
void kasfCloseSensors(sdword flag);
sdword kasfSensorsIsOpen(sdword flag);
sdword kasfResearchGUIActive(void);
sdword kasfConstructionManagerActive(void);
void kasfSensorsWeirdness(sdword flag);

void kasfAllowPlayerToResearch(char *name);
void kasfAllowPlayerToPurchase(char *name);
void kasfPlayerAcquiredTechnology(char *name);
sdword kasfCanPlayerResearch(char *name);
sdword kasfCanPlayerPurchase(char *name);
sdword kasfDoesPlayerHave(char *name);
void kasfSetBaseTechnologyCost(char *name, sdword cost);
sdword kasfGetBaseTechnologyCost(char *name);
sdword kasfTechIsResearching(void);

void kasfEnableTraderGUI(void);
sdword kasfTraderGUIActive(void);
void kasfSetTraderDialog(sdword dialogNum, char *text);
void kasfSetTraderPriceScale(sdword percent);
sdword kasfGetTraderPriceScale(void);
void kasfSetTraderDisabled(sdword disable);

sdword kasfRUsGet(sdword player);
void kasfRUsSet(sdword player, sdword RUs);

sdword kasfGetWorldResources(void);

void kasfSoundEvent(sdword event);
void kasfSoundEventShips(GrowSelection *ships, sdword event);

void kasfSpeechEvent(sdword event, sdword initFn, sdword watchFn);
void kasfSpeechEventQueueCurrentPlayer(sdword event, sdword variable);
void kasfSpeechEventQueueCurrentPlayerLinked(sdword event, sdword variable, sdword linkedEvent);
void kasfSpeechEventShips(GrowSelection *ships, sdword event, sdword variable);

void kasfToggleActor(sdword Actor, sdword on);

void kasfMusicPlay(sdword trackNum);
void kasfMusicStop(sdword fadeTime);

// Returns 1 if at least one ship in the selection has been rendered with the given LOD or lower
// (that is, closer to the camera / with higher details). 0 otherwise
sdword kasfRenderedShips(GrowSelection *ships, sdword LOD);
void kasfResetShipRenderFlags(GrowSelection *ships);
sdword kasfRenderedDerelicts(char *derelictType, sdword LOD);
void kasfResetDerelictRenderFlags(char *derelicttype);

udword kasfFindSelectedShips(GrowSelection *ships);
sdword kasfShipsSelectAllAttachedShips(GrowSelection *result, GrowSelection *source);

void kasfSaveLevel(sdword num, char *name);

void kasfClearScreen(void);

void kasfWideScreenIn(sdword frames);
void kasfWideScreenOut(sdword frames);
sdword kasfSubtitleCleanup(void);
void kasfSubtitleSimulate(sdword actor, sdword milliseconds, char *speech);
sdword kasfSubtitleDraw(void);
void kasfLocationCard(sdword milliseconds, char *location);
sdword kasfSetPilotMode(sdword enable);
void kasfHideShips(GrowSelection *ships);
void kasfUnhideShips(GrowSelection *ships);
void kasfDisableShips(GrowSelection *ships);
void kasfShipsSetPercentDamaged(GrowSelection *ships, sdword percentDamaged);
void kasfSetSalCapProcessingShip(GrowSelection *ships);

void kasfDeleteShips(GrowSelection *ships);

void kasfRotateDerelictType(char *derelictType, sdword rot_x, sdword rot_y, sdword rot_z, sdword variation);

sdword kasfRaceOfHuman(void);
void kasfUniversePause(void);
void kasfUniverseUnpause(void);
void kasfPauseBuildShips(void);
void kasfUnpauseBuildShips(void);
void kasfOtherKASPause(void);
void kasfOtherKASUnpause(void);

sdword kasfIntelEventEnded(void);
void kasfIntelEventNotEnded(void);
void kasfForceIntelEventEnded(void);

//turn sensors manager static on/off
void kasfSensorsStaticOn(void);
void kasfSensorsStaticOff(void);

//end the game right now and start plugscreens
void kasfGameEnd(void);

void kasfSpawnEffect(GrowSelection *ships, char *effectName, sdword parameter);

//
//  Mission02 / Cataclysm KAS runtime additions
//

//  Integer-hash entry store (mission flags persisted in the savegame)
sdword *kasfEntryFindOrCreateByScopeAndId(sdword scope, sdword id);
void kasfEntryGetOrCreate(sdword id);
void kasfEntryGetOrCreateSetValue(sdword id, sdword value);
void kasfEntrySetValue(sdword id, sdword value);
void kasfEntryIncrement(sdword id);
void kasfEntryDecrement(sdword id);
sdword kasfEntryDelete(sdword *entry);
sdword kasfEntryValueGet(sdword id);

//  Integer-hash timer engine
void kasTimerCreate(sdword scope, sdword id);
void kasTimerCreateSetStart(sdword scope, sdword id, sdword duration);
void kasTimerStart(sdword scope, sdword id);
void kasTimerSet(sdword scope, sdword id, sdword duration);
sdword kasTimerRemaining(sdword scope, sdword id);
sdword kasTimerExpiredDestroyCheck(sdword scope, sdword id);
sdword kasTimerExpiredDestroy(sdword scope, sdword id);
void kasTimerDestroy(sdword scope, sdword id);
void kasTimerCreateById(sdword id);
void kasTimerCreateSetStartById(sdword id, sdword duration);
void kasTimerStartById(sdword id);
void kasTimerSetById(sdword id, sdword duration);
sdword kasTimerRemainingById(sdword id);
sdword kasTimerExpiredDestroyById(sdword id);
sdword kasTimerExpiredById(sdword id);
void kasTimerDestroyById(sdword id);

//  Camera control helpers
void kasfCameraSetAngleDeg(sdword degrees);
void kasfCameraSetDeclinationDeg(sdword degrees);
void kasfCameraSetDistance(sdword distance);
void kasfTutCameraAdjustDistanceTowardMinimumZoom(sdword scalePercent);
void kasfTutCameraSetDistanceIgnoreZoom(sdword distance);
void kasfTutCameraFocusClose(GrowSelection *ships);
sdword kasfTutSetCataclysmInterfaceStyle(void);
void kasfTutSetPointerTargetFirstShip(char *name, GrowSelection *ships);

//  Ship-selection helpers
sdword kasfShipsSelectByPredicate(GrowSelection *result, GrowSelection *source,
                                  sdword (*predicate)(Ship *ship, sdword arg1, sdword arg2),
                                  sdword arg1, sdword arg2);
sdword kasShipIsNotSalvager(Ship *ship, sdword arg1, sdword arg2);
sdword kasfShipsSelectNotSalvagers(GrowSelection *result, GrowSelection *source);
sdword kasfShipsSelectLinkedShips(GrowSelection *result, GrowSelection *source);
sdword kasfFindCloakedShips(GrowSelection *ships);
void kasfShipsSetDontApplyPhysicsFlag(GrowSelection *ships, sdword set);
void kasfShipsSetDelayedDeath(GrowSelection *ships, bool32 delayedDeath);
sdword kasfShipsProcessModuleSlotOrder(GrowSelection *ships, char *shipTypeName, sdword orderValue);
GrowSelection *kasfShipsApplyPlayerUpgrades(GrowSelection *ships);
sdword kasfShipsAnyInSensorRange(GrowSelection *ships);
void kasfShipsEnableAttachNLips(GrowSelection *ships);
Ship *kasfShipsCreate(GrowSelection *result, char *shipTypeStr, sdword count, sdword playerIndex);
sdword kasfShipsCreateInside(GrowSelection *result, char *shipTypeStr, sdword count,
                             GrowSelection *target, sdword playerIndex);

//  Vector / sensor helpers
void kasfLabelledVectorSet(sdword label, sdword x, sdword y, sdword z);
hvector *kasfIntVectorPtrIfExists(sdword label);
void kasfVectorRandomizeInRange(hvector *out, hvector *center, sdword radius);
void kasfSensorsSetLookatPoint(hvector *point);

#endif
