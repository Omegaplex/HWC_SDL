"""
Fix parse-fail functions from type_from_decomp passes.

These functions failed because their decompiled prototype spans multiple lines.
This script decompiles them, reassembles the prototype across lines, and applies it.

Usage:
    python src/Scripts/fix_parse_fails.py [--dry-run]

Or with explicit address/name pairs passed as a JSON file:
    python src/Scripts/fix_parse_fails.py --targets targets.json [--dry-run]
"""

import json
import re
import sys
import time
import argparse
from pathlib import Path
from datetime import datetime, timezone

import requests

REPO_ROOT = Path(__file__).resolve().parents[2]
MCP_URL   = "http://localhost:13337/mcp"

# All parse-fail addresses and names from the lg/cp/uty/neb/trail/prim run
PARSE_FAILS = [
    # lg
    (0x4c6df0, "lgProcessCallBacksTask"),
    (0x4c7150, "lgStartMultiPlayerLANGameScreens"),
    (0x4c7270, "lgShutdownMultiPlayerGameScreens"),
    (0x4c72e0, "lgStartup"),
    (0x4c7590, "lgUpdateGameInfo"),
    (0x4c76b0, "lgJoinGameConfirmed"),
    (0x4c7760, "lgJoinGameDenied"),
    (0x4c77d0, "lgNotifyGameDisolved"),
    (0x534300, "lgUpdateChatRegions"),
    # cp
    (0x43e330, "cpValueTextureCreate"),
    (0x43e4c0, "cpPreviewTexturePrepare"),
    (0x43efb0, "cpPreviewImageDelete"),
    (0x43f160, "cpDefault"),
    (0x43f310, "cpResetRegions"),
    (0x440cb0, "cpTexturesPurge"),
    (0x5d92d0, "cpHueSatImageDelete"),
    # uty
    (0x488e70, "utyGameStatsInitLayout"),
    (0x5a83a0, "utyRegistryOptionsRead"),
    (0x5a84f0, "utyRegistryOptionsWrite"),
    (0x5a9190, "utyHideMenuOrGotoQuitGame"),
    (0x5a91b0, "utyCloseOK"),
    (0x5aa160, "utyShowCredits"),
    (0x5ab150, "utyBackFromSinglePlayer"),
    (0x5ab570, "utyGameQuit"),
    (0x5ab5b0, "utyGameQuitToMain"),
    (0x5ab6b0, "utyGameQuitToMain_0"),
    (0x5ab810, "utyGameSystemsPreInit"),
    (0x5abe70, "utyGameSystemsInit"),
    (0x5ac690, "utyGameSystemsPreShutdown"),
    (0x5ac710, "utyGameSystemsShutdown"),
    (0x5acd90, "utyTaskTimerClear"),
    (0x5acf70, "utyMouseButtonsClear"),
    (0x5acfd0, "utyCaretBlinkRate"),
    (0x5ad040, "utyToggleKeyStatesRestore"),
    (0x70bfe0, "utyLoadSinglePlayerGameGivenFilename"),
    # neb
    (0x4c7b30, "nebStartup"),
    (0x4ca0d0, "nebCreateCylinder"),
    (0x4cad80, "nebRender"),
    (0x4cb210, "nebSetFog"),
    (0x4cb7f0, "nebLoad_Nebula"),
    (0x5bdc80, "nebRemoveTendril"),
    # trail
    (0x5a49d0, "trailStartup"),
    # prim (decomp-fail skipped at top, rest multiline)
    (0x568f00, "primSolidTexture3_multi"),
    (0x56fdf0, "primTriSolid2"),
    (0x56fef0, "primTriOutline2"),
    (0x570010, "primRectSolidTextured2"),
    (0x570ac0, "primGLRectOutline2"),
    (0x570c30, "primRectShaded2"),
    (0x570f00, "primOvalArcOutline2"),
    (0x5711b0, "primGLCircleOutline2"),
    (0x571270, "primLine2"),
    (0x571500, "primLineLoopStart2"),
    (0x5715e0, "primBeveledRectSolid"),
    (0x572400, "primCircleSolid3"),
    (0x572a50, "primPoint3"),
    (0x572af0, "primSolidTexture3"),
    # nis
    (0x4cc600, "nisUpdateTask"),
    (0x57cac0, "nisLightStatePush"),
    (0x692750, "nisSMPTEOff"),
    # mesh
    (0x4b0970, "meshObjectRenderLit"),
    (0x4b1610, "meshObjectRenderLitRGLvx"),
    (0x4b4970, "meshHierarchyWalkR"),
    (0x4b6400, "meshRenderShipHierarchyCore"),
    (0x5f8070, "meshEnableVertexArrays"),
    # ping
    (0x497db0, "pingEvalAlwaysTrue"),
    (0x4e8540, "pingUpdateTask"),
    (0x4e8840, "pingListGetHead"),
    (0x4e9790, "pingLoad"),
    # cc
    (0x42c2e0, "ccIsInPilotMode"),
]

LINE_ANNOT  = re.compile(r"/\*\s*line:\s*\d+(?:,\s*address:\s*0x[0-9a-fA-F]+)?\s*\*/\s*")
CC_RE       = re.compile(r"__(?:cdecl|stdcall|fastcall|thiscall|usercall)")
# Regex to parse a no-CC prototype line: ReturnType [__noreturn] name(params)
NO_CC_RE    = re.compile(
    r"^((?:(?:unsigned|signed|const|_BOOL|BOOL|LONG|HWND|LPVOID|HANDLE)\s+)?"
    r"(?:void\s+|int\s+|char\s+|_DWORD\s+|\w+\s+\*?\s*)?"
    r"(?:__noreturn\s+)?)"           # optional __noreturn attribute
    r"\s*([A-Za-z_][A-Za-z0-9_?]*)" # function name
    r"\s*(\(.*\))\s*$"               # params (must be single-line for no-CC)
)

# struct/typedef names that need the struct keyword in IDA prototypes
TYPE_SUBS = {
    "Ship": "struct Ship",
    "Player": "struct Player",
    "SpaceObj": "struct SpaceObj",
    "SpaceObjRotImpTarg": "struct SpaceObjRotImpTarg",
    "Resource": "struct Resource",
    "Derelict": "struct Derelict",
    "Missile": "struct Missile",
    "Gun": "struct Gun",
    "GunStatic": "struct GunStatic",
    "AITeam": "struct AITeam",
    "AITeamMove": "struct AITeamMove",
    "AIPlayer": "struct AIPlayer",
    "CommandLayer": "struct CommandLayer",
    "AttackCommand": "struct AttackCommand",
    "BabyCallBack": "struct BabyCallBack",
    "rndinitdata": "struct rndinitdata",
    "hvector": "struct hvector",
    "region": "struct region",
    "fescreen": "struct fescreen",
    "featom": "struct featom",
    "fecallback": "struct fecallback",
    "fedrawcallback": "struct fedrawcallback",
    "fibfileheader": "struct fibfileheader",
    "tagRegion": "struct tagRegion",
    "uiclistwindow": "struct uiclistwindow",
    "uiclistitem": "struct uiclistitem",
    "chatlist": "struct chatlist",
    "lgqueueuserhere": "struct lgqueueuserhere",
    "lgqueuegamehere": "struct lgqueuegamehere",
    "lgqueuechatmsg": "struct lgqueuechatmsg",
    "lgqueuegameplayerinfo": "struct lgqueuegameplayerinfo",
    "lgqueuechatlist": "struct lgqueuechatlist",
    "nisplaying": "struct nisplaying",
    "cameramotion": "struct cameramotion",
    "nisheader": "struct nisheader",
    "nisSMPTE": "struct nisSMPTE",
    "nisstatic": "struct nisstatic",
    "mhbinding": "struct mhbinding",
    "materialentry": "struct materialentry",
    "hmatrix": "struct hmatrix",
    "meshdata": "struct meshdata",
    "polygonobject": "struct polygonobject",
    "shipbindings": "struct shipbindings",
    "CameraCommand": "struct CameraCommand",
    "FocusCommand": "struct FocusCommand",
    "Camera": "struct Camera",
    "nebulae": "struct nebulae",
    "nebchunk": "struct nebchunk",
    "nebtendril": "struct nebtendril",
}


def mcp(method, params, retries=3):
    for i in range(retries):
        try:
            r = requests.post(MCP_URL, json={"jsonrpc": "2.0", "id": 1,
                                              "method": method, "params": params},
                              timeout=45)
            r.raise_for_status()
            d = r.json()
            if "error" in d:
                return None, d["error"]["message"]
            return d.get("result"), None
        except Exception as e:
            if i == retries - 1:
                return None, str(e)
            time.sleep(2)
    return None, "max retries"


def apply_type_subs(proto: str) -> str:
    """Replace bare type names with struct-prefixed versions required by IDA.
    Only replaces in type position (not after * which would be a parameter name)."""
    for bare, full in TYPE_SUBS.items():
        # Match as a word NOT immediately preceded by * (parameter name position)
        # and NOT already prefixed with 'struct '
        proto = re.sub(r"(?<!\*\s)(?<!\*)\b(?<!struct )" + re.escape(bare) + r"\b",
                       full, proto)
    return proto


def extract_multiline_proto(text: str, expected_name: str) -> str | None:
    """
    Extract prototype from decompiler output, handling multiline prototypes.

    1. Only looks at lines before the opening brace (prototype section).
    2. If a calling-convention keyword is found, joins lines through the closing ')'.
    3. If no CC keyword (IDA omits __cdecl for default-CC functions), parses the
       first proto line and inserts __cdecl.
    """
    # Build proto section: everything before the first standalone '{'
    proto_lines = []
    for raw in text.splitlines():
        line = LINE_ANNOT.sub("", raw).strip()
        if line in ("{", ""):
            if line == "{":
                break
            continue
        proto_lines.append(line)

    if not proto_lines:
        return None

    # Try to find a line with an explicit calling convention
    cc_idx = None
    for i, line in enumerate(proto_lines):
        if CC_RE.search(line) and not line.startswith("//"):
            cc_idx = i
            break

    if cc_idx is not None:
        # Collect from that line until balanced parens close
        collected = []
        depth = 0
        found_open = False
        for line in proto_lines[cc_idx:]:
            if line.startswith("//"):
                break
            collected.append(line)
            for ch in line:
                if ch == "(":
                    depth += 1
                    found_open = True
                elif ch == ")":
                    depth -= 1
            if found_open and depth == 0:
                break
        proto = " ".join(collected)
        proto = re.sub(r"\s+", " ", proto).strip().rstrip("{").strip()
    else:
        # No explicit CC — parse first proto line and synthesise __cdecl prototype
        first = proto_lines[0]
        # Strip __noreturn if present (not valid in set_function_prototype)
        first = first.replace("__noreturn", "").strip()
        first = re.sub(r"\s+", " ", first)

        # Parse: return_type name(params)
        # The name is the word immediately before '('
        m = re.match(r"^(.*?)\b([A-Za-z_][A-Za-z0-9_?]*)\s*(\([^)]*\))\s*$", first)
        if not m:
            return None
        ret_type = m.group(1).strip() or "void"
        params   = m.group(3).strip()
        if params in ("()", "( )"):
            params = "(void)"
        proto = f"{ret_type} __cdecl {expected_name}{params}"
        proto = re.sub(r"\s+", " ", proto).strip()

    # Normalise the function name in the prototype
    if expected_name:
        proto = re.sub(r"\b[A-Za-z_][A-Za-z0-9_?]*(?=\s*\()", expected_name, proto, count=1)

    return proto if proto else None


# Manual prototype overrides for functions whose decompiled proto can't be parsed
# automatically (function-pointer returns, complex signatures, etc.)
MANUAL_OVERRIDES = {
    0x5acf70: "void __cdecl utyMouseButtonsClear(void)",  # returns fn-ptr, simplify to void
    # Returns int(__cdecl*)(int,int,int) — simplify fn-ptr return to int *
    0x4b6400: "int * __cdecl meshRenderShipHierarchyCore(int a1, int a2, int a3, int a4, float a5, int a6, int a7, int a8)",
}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--show-decomp", action="store_true",
                        help="Print first 5 decompiler lines for review")
    args = parser.parse_args()

    master_path = REPO_ROOT / ".ida-typing-progress.json"
    master = json.loads(master_path.read_text(encoding="utf-8"))
    typed_addrs = {e["address"] for e in master["functions"]
                   if e.get("typing_status") in ("typed", "improved")}
    master_by_addr = {e["address"]: e for e in master["functions"]}
    now = datetime.now(timezone.utc).isoformat()

    applied = 0
    failed  = 0
    skipped = 0

    for addr_int, name in PARSE_FAILS:
        addr_hex = hex(addr_int)

        if addr_hex in typed_addrs:
            skipped += 1
            continue

        # Resolve prototype — manual override or decompile
        if addr_int in MANUAL_OVERRIDES:
            proto = MANUAL_OVERRIDES[addr_int]
        else:
            decomp, err = mcp("decompile_function", {"address": addr_int})
            if err or not decomp:
                print(f"  DECOMP-FAIL {addr_hex} {name}: {err}")
                failed += 1
                continue

            text = decomp if isinstance(decomp, str) else decomp.get("code", "")

            if args.show_decomp:
                preview = "\n    ".join(text.splitlines()[:6])
                print(f"\n--- {name} @ {addr_hex} ---\n    {preview}")

            proto = extract_multiline_proto(text, name)
            if not proto:
                print(f"  STILL-FAIL  {addr_hex} {name}")
                failed += 1
                continue

        # Filter out __usercall and __thiscall — can't apply these as-is
        if "__usercall" in proto or "__thiscall" in proto:
            print(f"  SKIP-CC     {addr_hex} {name}: {proto[:80]}")
            skipped += 1
            continue

        proto = apply_type_subs(proto)

        if args.dry_run:
            print(f"  DRY  {addr_hex}  {proto[:100]}")
            continue

        result, err = mcp("set_function_prototype",
                          {"function_address": addr_int, "prototype": proto})
        if err:
            print(f"  TYPE-FAIL   {addr_hex} {name}: {err[:80]}")
            print(f"              proto was: {proto[:100]}")
            failed += 1
            continue

        # Update progress
        e = master_by_addr.get(addr_hex)
        module = name[:3] if len(name) >= 3 else "?"
        # Infer module from prefix
        for pfx in ("lgS","lgP","lgU","lgB","lgJ","lgN","lgA","lgC","lgD","lgG","lgL","lgV","lgY",
                    "cp","uty","neb","trail","prim","lg"):
            if name.startswith(pfx):
                module = pfx.rstrip("ABCDEFGHIJKLMNOPQRSTUVWXYZ").lower()
                if module == "lg":
                    module = "lg"
                elif module == "cp":
                    module = "cp"
                elif module == "uty":
                    module = "uty"
                elif module == "neb":
                    module = "neb"
                elif module == "trail":
                    module = "trail"
                elif module == "prim":
                    module = "prim"
                break

        if e:
            e["typing_status"] = "typed"
            e["prototype"]     = proto
            e["last_updated"]  = now
        else:
            new_e = {
                "address":      addr_hex,
                "current_name": name,
                "module":       module,
                "source_file":  "decomp:multiline-fix",
                "queue_status": "done",
                "typing_status": "typed",
                "prototype":    proto,
                "last_updated": now,
            }
            master["functions"].append(new_e)
            master_by_addr[addr_hex] = new_e

        print(f"  OK  {addr_hex}  {proto[:100]}")
        applied += 1
        time.sleep(0.05)

    if not args.dry_run:
        master_path.write_text(json.dumps(master, indent=2), encoding="utf-8")

    print(f"\n{'DRY RUN — ' if args.dry_run else ''}Summary:")
    print(f"  Applied : {applied}")
    print(f"  Skipped : {skipped}")
    print(f"  Failed  : {failed}")


if __name__ == "__main__":
    main()
