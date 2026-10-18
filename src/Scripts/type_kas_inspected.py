"""
Applies LLM-inspected prototypes to the 128 kas* functions not covered by source headers.
All signatures derived from IDA decompilation analysis.

Usage:
    python src/Scripts/type_kas_inspected.py [--dry-run] [--resume]
"""

import json
import time
import argparse
from pathlib import Path

import requests

REPO_ROOT  = Path(__file__).resolve().parents[2]
SNAPSHOT   = REPO_ROOT / ".ida-snapshot.json"
CHECKPOINT = REPO_ROOT / ".ida-typing-kas-inspected-progress.json"
MCP_URL    = "http://localhost:13337/mcp"

# Signatures derived from IDA decompilation inspection.
# Types used: Ship*, GrowSelection*, AITeam*, vector*, hvector* (as struct hvector*),
# Volume* (as struct Volume*), bool32, sdword, udword, real32, BOOL
SIGNATURES = {
    # kasTimer family: identify timers by (AITeam *team, sdword timerId) key pair
    "kasTimerCreate":              "sdword __cdecl kasTimerCreate(AITeam *team, sdword timerId)",
    "kasTimerSet":                 "void __cdecl kasTimerSet(AITeam *team, sdword timerId, sdword duration)",
    "kasTimerCreateSetStart":      "void __cdecl kasTimerCreateSetStart(AITeam *team, sdword timerId, sdword duration)",
    "kasTimerDestroy":             "void __cdecl kasTimerDestroy(AITeam *team, sdword timerId)",
    "kasTimerExpiredDestroy":      "sdword __cdecl kasTimerExpiredDestroy(AITeam *team, sdword timerId)",
    "kasTimerExpiredDestroyCheck": "sdword __cdecl kasTimerExpiredDestroyCheck(AITeam *team, sdword timerId)",
    "kasTimerPauseAllNotScoped":   "void __cdecl kasTimerPauseAllNotScoped(sdword scope)",
    "kasTimerRemaining":           "sdword __cdecl kasTimerRemaining(AITeam *team, sdword timerId)",
    "kasTimerStart":               "void __cdecl kasTimerStart(AITeam *team, sdword timerId)",
    "kasTimerStop":                "void __cdecl kasTimerStop(AITeam *team, sdword timerId)",

    # ById variants: use current KAS scope, take only timerId
    "kasTimerSetById":             "void __cdecl kasTimerSetById(sdword timerId, sdword duration)",
    "kasTimerStartById":           "void __cdecl kasTimerStartById(sdword timerId)",
    "kasTimerExpiredDestroyById":  "sdword __cdecl kasTimerExpiredDestroyById(sdword timerId)",

    # kasShipIs* predicates — (Ship*, optional extra) → BOOL/sdword
    "kasShipIsAttackingSelection": "sdword __cdecl kasShipIsAttackingSelection(Ship *ship, GrowSelection *attackers)",
    "kasShipIsCapital":            "sdword __cdecl kasShipIsCapital(Ship *ship, sdword invert)",
    "kasShipIsEnemyPlayerOwned":   "BOOL __cdecl kasShipIsEnemyPlayerOwned(Ship *ship)",
    "kasShipIsFartherThanDistanceFromPoint": "BOOL __cdecl kasShipIsFartherThanDistanceFromPoint(Ship *ship, vector *point, real32 distance)",
    "kasShipIsFriendlyPlayerOwned":"BOOL __cdecl kasShipIsFriendlyPlayerOwned(Ship *ship)",
    "kasShipIsInsideVolume":       "sdword __cdecl kasShipIsInsideVolume(Ship *ship, struct Volume *volume)",
    "kasShipIsMemberOfSelection":  "sdword __cdecl kasShipIsMemberOfSelection(Ship *ship, GrowSelection *sel)",
    "kasShipIsMoving":             "BOOL __cdecl kasShipIsMoving(Ship *ship)",
    "kasShipIsWithinDistanceOfPoint": "BOOL __cdecl kasShipIsWithinDistanceOfPoint(Ship *ship, vector *point, real32 maxDist)",

    # kasShipMatches* / kasShip operaions
    "kasShipMatchesClass":         "BOOL __cdecl kasShipMatchesClass(Ship *ship, sdword shipClass)",
    "kasShipMatchesNotDockedFilter":"BOOL __cdecl kasShipMatchesNotDockedFilter(Ship *ship, sdword filter)",
    "kasShipMatchesRace":          "BOOL __cdecl kasShipMatchesRace(Ship *ship, sdword race)",
    "kasShipMatchesSpecialSelectionFlag": "sdword __cdecl kasShipMatchesSpecialSelectionFlag(Ship *ship, sdword flag)",
    "kasShipMatchesType":          "BOOL __cdecl kasShipMatchesType(Ship *ship, sdword shipType)",
    "kasShipClearRowGettingOutOfWayFlag":    "sdword __cdecl kasShipClearRowGettingOutOfWayFlag(Ship *ship)",
    "kasShipClearSinglePlayerInParadeFlag":  "sdword __cdecl kasShipClearSinglePlayerInParadeFlag(Ship *ship)",
    "kasShipFuelIsBelowPercent":   "BOOL __cdecl kasShipFuelIsBelowPercent(Ship *ship, real32 percent)",
    "kasShipHasAvailableUpgradeChanges": "sdword __cdecl kasShipHasAvailableUpgradeChanges(Ship *ship)",
    "kasSelectionIndexIsBelowLimit": "BOOL __cdecl kasSelectionIndexIsBelowLimit(GrowSelection *ships, udword limit)",

    # kasLabelled* utility
    "kasLabelledTimerDestroyAll":  "void __cdecl kasLabelledTimerDestroyAll(void)",
    "kasLabelledVectorSet":        "struct hvector *__cdecl kasLabelledVectorSet(char *label, struct hvector *vec)",
    "kasFlagsPtrIfExists":         "void *__cdecl kasFlagsPtrIfExists(char *label)",

    # kasfShipsSelect* filter family: (GrowSelection *result, GrowSelection *source, ...)
    "kasfShipsSelectAttackingSelection":    "sdword __cdecl kasfShipsSelectAttackingSelection(GrowSelection *result, GrowSelection *source, GrowSelection *attackers)",
    "kasfShipsSelectAvailableUpgradeChanges": "sdword __cdecl kasfShipsSelectAvailableUpgradeChanges(GrowSelection *result, GrowSelection *source)",
    "kasfShipsSelectFartherThanDistanceFromPoint": "sdword __cdecl kasfShipsSelectFartherThanDistanceFromPoint(GrowSelection *result, GrowSelection *source, vector *point, real32 distance)",
    "kasfShipsSelectFirstN":       "sdword __cdecl kasfShipsSelectFirstN(GrowSelection *result, GrowSelection *source, sdword count)",
    "kasfShipsSelectFuelBelowPercent": "sdword __cdecl kasfShipsSelectFuelBelowPercent(GrowSelection *result, GrowSelection *source, real32 percent)",
    "kasfShipsSelectHealthAtLeastPercent": "sdword __cdecl kasfShipsSelectHealthAtLeastPercent(GrowSelection *result, GrowSelection *source, real32 percent)",
    "kasfShipsSelectInsideVolume": "sdword __cdecl kasfShipsSelectInsideVolume(GrowSelection *result, GrowSelection *source, struct Volume *volume)",
    "kasfShipsSelectLinkedShips":  "sdword __cdecl kasfShipsSelectLinkedShips(GrowSelection *result, GrowSelection *source, Ship *leadShip)",
    "kasfShipsSelectLinkedWith":   "sdword __cdecl kasfShipsSelectLinkedWith(GrowSelection *result, GrowSelection *source, GrowSelection *linkedTo)",
    "kasfShipsSelectMembersOfSelection": "sdword __cdecl kasfShipsSelectMembersOfSelection(GrowSelection *result, GrowSelection *source, GrowSelection *members)",
    "kasfShipsSelectRace":         "sdword __cdecl kasfShipsSelectRace(GrowSelection *result, GrowSelection *source, sdword race)",
    "kasfShipsSelectRowGettingOutOfWay":   "sdword __cdecl kasfShipsSelectRowGettingOutOfWay(GrowSelection *result, GrowSelection *source)",
    "kasfShipsSelectSinglePlayerInParade": "sdword __cdecl kasfShipsSelectSinglePlayerInParade(GrowSelection *result, GrowSelection *source)",
    "kasfShipsSelectSpecialTargets": "sdword __cdecl kasfShipsSelectSpecialTargets(GrowSelection *result, GrowSelection *source, GrowSelection *targets)",
    "kasfShipsSelectTargets":      "sdword __cdecl kasfShipsSelectTargets(GrowSelection *result, GrowSelection *source, GrowSelection *targets)",

    # kasfShipsSet* setters: (GrowSelection *ships, ...)
    "kasfShipsSetAttachVelocityFlag": "void __cdecl kasfShipsSetAttachVelocityFlag(GrowSelection *ships, bool32 flag)",
    "kasfShipsSetClamped":         "void __cdecl kasfShipsSetClamped(GrowSelection *ships, bool32 clamped)",
    "kasfShipsSetDelayedDeath":    "void __cdecl kasfShipsSetDelayedDeath(GrowSelection *ships, sdword delay)",
    "kasfShipsSetHyperspaceDestination": "void __cdecl kasfShipsSetHyperspaceDestination(GrowSelection *ships, vector *dest)",
    "kasfShipsSetHyperspaceDestinationToCurrentPosition": "void __cdecl kasfShipsSetHyperspaceDestinationToCurrentPosition(GrowSelection *ships, vector *currentPos)",
    "kasfShipsSetMilitaryParade":  "void __cdecl kasfShipsSetMilitaryParade(GrowSelection *ships, GrowSelection *formation)",
    "kasfShipsSetNotBumpable":     "void __cdecl kasfShipsSetNotBumpable(GrowSelection *ships, bool32 notBumpable)",
    "kasfShipsSetPosition":        "void __cdecl kasfShipsSetPosition(GrowSelection *ships, real32 x, real32 y, real32 z)",
    "kasfShipsSetPositionFromVector": "void __cdecl kasfShipsSetPositionFromVector(GrowSelection *ships, vector *pos)",
    "kasfShipsSetPositionNearVector": "void __cdecl kasfShipsSetPositionNearVector(GrowSelection *ships, vector *pos, real32 radius)",
    "kasfShipsSetRotatableFlag":   "void __cdecl kasfShipsSetRotatableFlag(GrowSelection *ships, bool32 flag)",
    "kasfShipsSetSlaveableFlag":   "void __cdecl kasfShipsSetSlaveableFlag(GrowSelection *ships, bool32 slaveable)",
    "kasfShipsProcessModuleSlotOrder": "sdword __cdecl kasfShipsProcessModuleSlotOrder(GrowSelection *ships, GrowSelection *source, sdword slotType)",

    # kasfShips* collection operations
    "kasfShipsApplyPlayerUpgrades":"void __cdecl kasfShipsApplyPlayerUpgrades(GrowSelection *ships)",
    "kasfShipsAnyDying":           "sdword __cdecl kasfShipsAnyDying(GrowSelection *ships)",
    "kasfShipsAnyHyperspaceReady": "sdword __cdecl kasfShipsAnyHyperspaceReady(GrowSelection *ships)",
    "kasfShipsCreate":             "Ship *__cdecl kasfShipsCreate(GrowSelection *result, char *shipType, sdword playerIndex, sdword count)",
    "kasfShipsCreateHyperspaceIn": "sdword __cdecl kasfShipsCreateHyperspaceIn(GrowSelection *result, char *shipType, sdword playerIndex, struct hvector *destination, sdword count)",
    "kasfShipsCreateInside":       "sdword __cdecl kasfShipsCreateInside(GrowSelection *result, char *shipType, char *raceName, sdword playerIndex, sdword count)",
    "kasfShipsDockInstant":        "void __cdecl kasfShipsDockInstant(GrowSelection *ships, GrowSelection *dockTarget)",
    "kasfShipsHealthAverage":      "sdword __cdecl kasfShipsHealthAverage(GrowSelection *ships)",
    "kasfShipsHyperspaceIn":       "void __cdecl kasfShipsHyperspaceIn(GrowSelection *ships, vector *destination)",
    "kasfShipsMakeCrazy":          "sdword __cdecl kasfShipsMakeCrazy(GrowSelection *ships, sdword crazyLevel)",
    "kasfShipsShuffle":            "void __cdecl kasfShipsShuffle(GrowSelection *ships)",
    "kasfShipsSpecialToggle":      "void __cdecl kasfShipsSpecialToggle(GrowSelection *ships, sdword toggle)",

    # Single ship / per-ship operations
    "kasfCloak":                   "void __cdecl kasfCloak(GrowSelection *ships)",
    "kasfDisableShips":            "void __cdecl kasfDisableShips(GrowSelection *ships)",
    "kasfEnableShip":              "sdword __cdecl kasfEnableShip(char *shipTypeName, char *raceName, sdword playerIndex)",
    "kasfShipDecrementModuleSlotTimer": "void __cdecl kasfShipDecrementModuleSlotTimer(Ship *ship)",
    "kasfShipDelayedDeathPending": "sdword __cdecl kasfShipDelayedDeathPending(Ship *ship)",
    "kasfShipGetModuleSlotTimer":  "sdword __cdecl kasfShipGetModuleSlotTimer(Ship *ship)",
    "kasfShipSetTargetableFlag":   "void __cdecl kasfShipSetTargetableFlag(Ship *ship, bool32 targetable)",
    "kasfShipTypeBuildCost":       "sdword __cdecl kasfShipTypeBuildCost(sdword shipType)",
    "kasfShipTypeBuildTime":       "sdword __cdecl kasfShipTypeBuildTime(sdword shipType)",

    # Camera / pilot mode
    "kasfCameraSetAngleDeg":       "void __cdecl kasfCameraSetAngleDeg(sdword angleDeg)",
    "kasfCameraSetDeclinationDeg": "void __cdecl kasfCameraSetDeclinationDeg(sdword declinationDeg)",
    "kasfCameraSetDistance":       "void __cdecl kasfCameraSetDistance(sdword distance)",
    "kasfSetPilotMode":            "sdword __cdecl kasfSetPilotMode(sdword mode)",

    # Tutorial helpers
    "kasfTutCameraFocusClose":     "void __cdecl kasfTutCameraFocusClose(GrowSelection *ships)",
    "kasfTutSetSelectedShips":     "void __cdecl kasfTutSetSelectedShips(GrowSelection *ships)",
    "kasfWatchTutorial1Tutorial10CameraMoveDescription": "void __cdecl kasfWatchTutorial1Tutorial10CameraMoveDescription(void)",

    # Misc utility
    "kasfActivateTech":            "sdword __cdecl kasfActivateTech(char *techName)",
    "kasfDisableMothershipBigGunLaunch": "void __cdecl kasfDisableMothershipBigGunLaunch(void)",
    "kasfFindBeastShipsInside":    "sdword __cdecl kasfFindBeastShipsInside(struct Volume *volume, GrowSelection *result)",
    "kasfFindEnemyMothership":     "sdword __cdecl kasfFindEnemyMothership(GrowSelection *result)",
    "kasfLabelledVectorSet":       "struct hvector *__cdecl kasfLabelledVectorSet(char *label, struct hvector *vec)",
    "kasfPingAddShipsIndexed":     "void __cdecl kasfPingAddShipsIndexed(GrowSelection *ships, sdword pingIndex)",
    "kasfResearchGUIActive":       "sdword __cdecl kasfResearchGUIActive(void)",
    "kasfSensorsSetLookatPoint":   "void __cdecl kasfSensorsSetLookatPoint(vector *point)",
    "kasfSetSalCapProcessingShip": "void __cdecl kasfSetSalCapProcessingShip(Ship *ship)",
    "kasfSpeechEventQueueCurrentPlayer": "void __cdecl kasfSpeechEventQueueCurrentPlayer(sdword event, sdword variable)",
    "kasfSubtitleCleanup":         "void __cdecl kasfSubtitleCleanup(void)",

    # TDSalvage trial
    "kasfTDSalvageTrialStartWaitingForShips":      "void __cdecl kasfTDSalvageTrialStartWaitingForShips(GrowSelection *ships, vector *position)",
    "kasfTDSalvageTrialWaitingForShipsActive":     "sdword __cdecl kasfTDSalvageTrialWaitingForShipsActive(GrowSelection *ships)",
    "kasfTDSalvageTrialWaitingForShipsDone":       "sdword __cdecl kasfTDSalvageTrialWaitingForShipsDone(GrowSelection *ships)",
    "kasfTDSalvageTrialGetWaitingForShipsPoints":  "sdword __cdecl kasfTDSalvageTrialGetWaitingForShipsPoints(GrowSelection *ships, vector *outPosition)",

    # Vector operations on hvector* (4D float)
    "kasfVectorAddClampToUniverse":        "void __cdecl kasfVectorAddClampToUniverse(struct hvector *dest, struct hvector *src)",
    "kasfVectorBlendClampToUniverse":      "void __cdecl kasfVectorBlendClampToUniverse(struct hvector *result, struct hvector *a, struct hvector *b, real32 t)",
    "kasfVectorCrossNormalize":            "void __cdecl kasfVectorCrossNormalize(struct hvector *result, struct hvector *a, struct hvector *b)",
    "kasfVectorDotProductScaled100":       "sdword __cdecl kasfVectorDotProductScaled100(struct hvector *a, struct hvector *b)",
    "kasfVectorRandomizeInRange":          "void __cdecl kasfVectorRandomizeInRange(struct hvector *result, vector *center, sdword range)",
    "kasfVectorRoundW":                    "void __cdecl kasfVectorRoundW(struct hvector *v)",
    "kasfVectorRoundX":                    "void __cdecl kasfVectorRoundX(struct hvector *v)",
    "kasfVectorRoundY":                    "void __cdecl kasfVectorRoundY(struct hvector *v)",
    "kasfVectorRoundZ":                    "void __cdecl kasfVectorRoundZ(struct hvector *v)",
    "kasfVectorScaleByRatio":              "struct hvector *__cdecl kasfVectorScaleByRatio(struct hvector *v, sdword numerator, sdword denominator)",
    "kasfVectorSubtractClampToUniverse":   "void __cdecl kasfVectorSubtractClampToUniverse(struct hvector *dest, struct hvector *src)",
}


def mcp_call(method, params, retries=3):
    import requests as _r
    payload = {"jsonrpc": "2.0", "id": 1, "method": method, "params": params}
    for attempt in range(retries):
        try:
            r = _r.post(MCP_URL, json=payload, timeout=30)
            r.raise_for_status()
            data = r.json()
            if "error" in data:
                return None, data["error"]["message"]
            return data.get("result"), None
        except Exception as e:
            if attempt == retries - 1:
                return None, str(e)
            time.sleep(2)
    return None, "max retries exceeded"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--resume", action="store_true")
    args = parser.parse_args()

    data = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    snap_by_name = {e["name"]: e for e in data if e["name"].startswith("kas")}

    candidates = []
    for name, proto in SIGNATURES.items():
        entry = snap_by_name.get(name)
        if entry:
            candidates.append((entry["address"], name, proto))
        else:
            print(f"WARN: {name} not found in snapshot")

    print(f"Candidates: {len(candidates)}  (covering {len(SIGNATURES)} inspected signatures)")

    if args.dry_run:
        print("\nDRY RUN — first 15:")
        for addr, name, proto in candidates[:15]:
            print(f"  {addr}  {proto}")
        return

    cp = {"done": {}, "failed": {}}
    if args.resume and CHECKPOINT.exists():
        cp = json.loads(CHECKPOINT.read_text(encoding="utf-8"))

    done_set = set(cp["done"])
    pending  = [(a, n, p) for a, n, p in candidates if a not in done_set and a not in set(cp["failed"])]
    print(f"Already done: {len(done_set)}  Pending: {len(pending)}")

    start = time.time()
    for i, (addr, name, proto) in enumerate(pending):
        result, err = mcp_call("set_function_prototype",
                               {"function_address": int(addr, 16), "prototype": proto})
        if err:
            print(f"  FAIL  {addr}  {name}  -- {err}")
            cp["failed"][addr] = {"name": name, "error": err}
        else:
            cp["done"][addr] = {"name": name, "prototype": proto}

        if (i + 1) % 20 == 0:
            CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")
            elapsed = time.time() - start
            rate = (i + 1) / elapsed
            remaining = (len(pending) - i - 1) / rate if rate > 0 else 0
            print(f"  [{100*(len(done_set)+i+1)/len(candidates):.0f}%]"
                  f" {len(done_set)+i+1}/{len(candidates)}"
                  f"  ~{remaining:.0f}s remaining")
        time.sleep(0.1)

    CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")
    elapsed = time.time() - start
    print(f"\nDone. Applied: {len(cp['done'])}  Failed: {len(cp['failed'])}  in {elapsed:.0f}s")


if __name__ == "__main__":
    main()
