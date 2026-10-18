"""
Generic source-backed prototype typer for any named function group.
Parses C headers, applies prototype substitutions for types not in IDA's namespace,
and applies via set_function_prototype.

Usage:
    python src/Scripts/type_from_headers.py --module cl [--dry-run] [--resume]
    python src/Scripts/type_from_headers.py --module uni [--dry-run] [--resume]
    python src/Scripts/type_from_headers.py --module ship [--dry-run] [--resume]
"""

import json
import os
import re
import time
import argparse
from pathlib import Path

import requests

REPO_ROOT = Path(__file__).resolve().parents[2]
HW_ROOT   = Path(os.environ.get("HW_ROOT", ""))
HW1_ROOT  = HW_ROOT / "src" / "Game"
MCP_URL   = "http://localhost:13337/mcp"

# ── Module configuration ────────────────────────────────────────────────────
MODULES = {
    "cl": {
        "prefix": "cl",
        "headers": [
            HW1_ROOT / "CommandLayer.h",
            REPO_ROOT / "src/Game/CommandLayer.h",
        ],
        "checkpoint": ".ida-typing-cl-utils-progress.json",
    },
    "uni": {
        "prefix": "uni",
        "headers": [
            HW1_ROOT / "Universe.h",
            REPO_ROOT / "src/Game/Universe.h",
        ],
        "checkpoint": ".ida-typing-uni-utils-progress.json",
    },
    "ship": {
        "prefix": "ship",
        "headers": [
            HW1_ROOT / "Ships.h",
            HW1_ROOT / "ShipSelect.h",
            REPO_ROOT / "src/Game/Ships.h",
            REPO_ROOT / "src/Game/ShipSelect.h",
        ],
        "checkpoint": ".ida-typing-ship-utils-progress.json",
    },
    "sel": {
        "prefix": "sel",
        "headers": [
            HW1_ROOT / "Select.h",
            REPO_ROOT / "src/Game/Select.h",
        ],
        "checkpoint": ".ida-typing-sel-utils-progress.json",
    },
    "task": {
        "prefix": "task",
        "headers": [
            HW1_ROOT / "Task.h",
            REPO_ROOT / "src/Game/Task.h",
        ],
        "checkpoint": ".ida-typing-task-utils-progress.json",
    },
    "sound": {
        "prefix": "sound",
        "headers": [
            HW1_ROOT / "SoundEvent.h",
            REPO_ROOT / "src/Game/SoundEvent.h",
        ],
        "checkpoint": ".ida-typing-sound-utils-progress.json",
    },
    "tr": {
        "prefix": "tr",
        "headers": [
            HW1_ROOT / "Trails.h",
            REPO_ROOT / "src/Game/Trails.h",
        ],
        "checkpoint": ".ida-typing-tr-utils-progress.json",
    },
    "rnd": {
        "prefix": "rnd",
        "headers": [
            HW_ROOT / "src" / "Win32" / "render.h",
            REPO_ROOT / "src/Win32/render.h",
        ],
        "checkpoint": ".ida-typing-rnd-utils-progress.json",
    },
    "fe": {
        "prefix": "fe",
        "headers": [
            HW1_ROOT / "FeFlow.h",
            REPO_ROOT / "src/Game/FEFlow.h",
        ],
        "checkpoint": ".ida-typing-fe-utils-progress.json",
    },
    "dock": {
        "prefix": "dock",
        "headers": [
            HW1_ROOT / "dock.h",
            REPO_ROOT / "src/Game/Dock.h",
        ],
        "checkpoint": ".ida-typing-dock-utils-progress.json",
    },
    "gun": {
        "prefix": "gun",
        "headers": [
            HW1_ROOT / "gun.h",
            REPO_ROOT / "src/Game/Gun.h",
        ],
        "checkpoint": ".ida-typing-gun-utils-progress.json",
    },
    "sp": {
        "prefix": "sp",
        "headers": [
            HW1_ROOT / "SinglePlayer.h",
            REPO_ROOT / "src/Game/SinglePlayer.h",
        ],
        "checkpoint": ".ida-typing-sp-utils-progress.json",
    },
    "tut": {
        "prefix": "tut",
        "headers": [
            HW1_ROOT / "Tutorial.h",
            REPO_ROOT / "src/Game/Tutorial.h",
        ],
        "checkpoint": ".ida-typing-tut-utils-progress.json",
    },
    "op": {
        "prefix": "op",
        "headers": [
            HW1_ROOT / "Options.h",
            REPO_ROOT / "src/Game/Options.h",
        ],
        "checkpoint": ".ida-typing-op-utils-progress.json",
    },
    "etg": {
        "prefix": "etg",
        "headers": [
            HW1_ROOT / "ETG.h",
            REPO_ROOT / "src/Game/ETG.h",
        ],
        "checkpoint": ".ida-typing-etg-utils-progress.json",
    },
    "univ": {
        "prefix": "univ",
        "headers": [
            HW1_ROOT / "Universe.h",
            REPO_ROOT / "src/Game/Universe.h",
            REPO_ROOT / "src/Game/UnivUpdate.h",
        ],
        "checkpoint": ".ida-typing-univ-utils-progress.json",
    },
    "nis": {
        "prefix": "nis",
        "headers": [
            HW1_ROOT / "NIS.h",
            REPO_ROOT / "src/Game/NIS.h",
        ],
        "checkpoint": ".ida-typing-nis-utils-progress.json",
    },
    "mesh": {
        "prefix": "mesh",
        "headers": [
            HW1_ROOT / "Mesh.h",
            REPO_ROOT / "src/Game/Mesh.h",
        ],
        "checkpoint": ".ida-typing-mesh-utils-progress.json",
    },
    "ping": {
        "prefix": "ping",
        "headers": [
            HW1_ROOT / "ping.h",
            REPO_ROOT / "src/Game/ping.h",
        ],
        "checkpoint": ".ida-typing-ping-utils-progress.json",
    },
    "cc": {
        "prefix": "cc",
        "headers": [
            HW1_ROOT / "CameraCommand.h",
            REPO_ROOT / "src/Game/CameraCommand.h",
        ],
        "checkpoint": ".ida-typing-cc-utils-progress.json",
    },
}

# ── Type substitution map ────────────────────────────────────────────────────
# Applied left-to-right with regex word-boundary matching.
# Enum types become sdword; pointer typedefs expand; struct types get 'struct' prefix.
TYPE_SUBS = [
    # Enums → sdword (these are not in IDA's local type list as usable type names)
    (r"\bShipType\b",          "sdword"),
    (r"\bShipRace\b",          "sdword"),
    (r"\bTypeOfFormation\b",   "sdword"),
    (r"\bFormationType\b",     "sdword"),
    (r"\bDockType\b",          "sdword"),
    (r"\bTeamType\b",          "sdword"),
    (r"\bReconType\b",         "sdword"),
    (r"\bShipClass\b",         "sdword"),
    (r"\bPlayerIndex\b",       "sdword"),
    (r"\bSoundType\b",         "sdword"),
    (r"\bTrailType\b",         "sdword"),
    (r"\bResourceType\b",      "sdword"),
    (r"\bTacticsType\b",       "sdword"),
    # Scalar typedefs
    (r"\buword\b",             "unsigned short"),
    (r"\bsword\b",             "short"),
    (r"\bbool8\b",             "ubyte"),
    (r"\btaskhandle\b",        "sdword"),
    # Pointer typedefs → expand
    (r"\bShipPtr\b",           "Ship *"),
    (r"\bResourcePtr\b",       "struct Resource *"),
    (r"\bPlayerPtr\b",         "struct Player *"),
    (r"\bSpaceObjPtr\b",       "struct SpaceObj *"),
    # Function pointer typedefs → expand inline
    (r"\btaskfunction\b",      "void (__cdecl *)(void)"),
    # Struct types that need the 'struct' keyword in IDA
    (r"(?<!struct )(?<!\*)\bCommandLayer\b(?=\s*\*)",  "struct CommandLayer"),
    (r"(?<!struct )(?<!\*)\bAttackCommand\b(?=\s*\*)", "struct AttackCommand"),
    (r"(?<!struct )(?<!\*)\bSpecialCommand\b(?=\s*\*)", "struct SpecialCommand"),
    (r"(?<!struct )(?<!\*)\bPlayer\b(?=\s*\*)",        "struct Player"),
    (r"(?<!struct )(?<!\*)\bResource\b(?=\s*\*)",      "struct Resource"),
    (r"(?<!struct )(?<!\*)\bSpaceObj\b(?=\s*\*)",      "struct SpaceObj"),
    (r"(?<!struct )(?<!\*)\bSpaceObjRotImpTarg\b(?=\s*\*)", "struct SpaceObjRotImpTarg"),
    (r"(?<!struct )(?<!\*)\bSpaceObjRotImpTargGuidance\b(?=\s*\*)", "struct SpaceObjRotImpTargGuidance"),
    (r"(?<!struct )(?<!\*)\bTrail\b(?=\s*\*)",         "struct Trail"),
    (r"(?<!struct )(?<!\*)\bTask\b(?=\s*\*)",          "struct Task"),
    (r"(?<!struct )(?<!\*)\bTaskTick\b(?=\s*\*)",      "struct TaskTick"),
    (r"(?<!struct )(?<!\*)\bTrailSegment\b(?=\s*\*)",  "struct TrailSegment"),
    # FEFlow types
    (r"\bregionhandle\b",                              "struct region *"),
    (r"(?<!struct )(?<!\*)\bfescreen\b(?=\s*\*)",      "struct fescreen"),
    (r"(?<!struct )(?<!\*)\bfeatom\b(?=\s*\*)",        "struct featom"),
    (r"(?<!struct )(?<!\*)\bfecallback\b(?=\s*\*)",    "struct fecallback"),
    (r"(?<!struct )(?<!\*)\bfedrawcallback\b(?=\s*\*)", "struct fedrawcallback"),
    (r"(?<!struct )(?<!\*)\bfibfileheader\b(?=\s*\*)", "struct fibfileheader"),
    (r"\bfedrawfunction\b",                            "void (__cdecl *)(struct featom *, struct region *)"),
    (r"\bfefunction\b",                                "void (__cdecl *)(char *, struct featom *)"),
    # Additional struct types needing 'struct' prefix in IDA
    (r"(?<!struct )(?<!\*)\bBabyCallBack\b(?=\s*\*)",  "struct BabyCallBack"),
    (r"(?<!struct )(?<!\*)\brndinitdata\b(?=\s*\*)",   "struct rndinitdata"),
    (r"(?<!struct )(?<!\*)\bGun\b(?=\s*\*)",           "struct Gun"),
    (r"(?<!struct )(?<!\*)\bGunStatic\b(?=\s*\*)",     "struct GunStatic"),
    (r"(?<!struct )(?<!\*)\bDerelict\b(?=\s*\*)",      "struct Derelict"),
    (r"(?<!struct )(?<!\*)\bMissile\b(?=\s*\*)",       "struct Missile"),
    (r"(?<!struct )(?<!\*)\bShipStaticInfo\b(?=\s*\*)", "struct ShipStaticInfo"),
    # Already-ok: Ship *, SelectCommand *, GrowSelection *, vector *, sdword, udword, real32, BOOL, bool32
]

DECL_RE = re.compile(
    r"^(?!.*typedef)(?!.*//)"
    r"\s*"
    r"((?:(?:unsigned|signed|struct|const)\s+)?"
    r"[A-Za-z_][A-Za-z0-9_]*(?:\s*\*+)?)"           # return type
    r"\s+"
    r"({prefix}[A-Za-z_][A-Za-z0-9_]*)"              # function name
    r"\s*"
    r"(\([^)]*\))"                                    # params (single-line)
    r"\s*;"
)


def make_decl_re(prefix: str) -> re.Pattern:
    src = DECL_RE.pattern.replace("{prefix}", prefix)
    return re.compile(src)


def apply_type_subs(proto: str) -> str:
    for pattern, replacement in TYPE_SUBS:
        proto = re.sub(pattern, replacement, proto)
    proto = re.sub(r"\s+", " ", proto).strip()
    return proto


def parse_headers(header_list: list, prefix: str) -> dict:
    decl_re = make_decl_re(prefix)
    sigs: dict = {}
    for hdr in header_list:
        hdr = Path(hdr)
        if not hdr.exists():
            continue
        text = hdr.read_text(encoding="utf-8", errors="replace")
        text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
        for line in text.splitlines():
            m = decl_re.match(line)
            if m:
                ret    = m.group(1).strip()
                name   = m.group(2).strip()
                params = m.group(3).strip()
                proto  = apply_type_subs(f"{ret} __cdecl {name}{params}")
                sigs[name] = proto
    return sigs


def mcp_call(method: str, params: dict, retries: int = 3):
    for attempt in range(retries):
        try:
            r = requests.post(MCP_URL, json={"jsonrpc": "2.0", "id": 1,
                                              "method": method, "params": params},
                              timeout=30)
            r.raise_for_status()
            data = r.json()
            if "error" in data:
                return None, data["error"]["message"]
            return data.get("result"), None
        except Exception as e:
            if attempt == retries - 1:
                return None, str(e)
            time.sleep(2)
    return None, "max retries"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--module",  required=True, choices=list(MODULES.keys()))
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--resume",  action="store_true")
    args = parser.parse_args()
    if not os.environ.get("HW_ROOT"):
        parser.error("HW_ROOT environment variable must be set to the original Homeworld checkout root.")

    cfg    = MODULES[args.module]
    prefix = cfg["prefix"]
    snap   = json.loads((REPO_ROOT / ".ida-snapshot.json").read_text(encoding="utf-8"))
    snap_by_name = {e["name"]: e for e in snap if e["name"].startswith(prefix)}

    sigs = parse_headers(cfg["headers"], prefix)
    print(f"Signatures from headers: {len(sigs)}")

    candidates = []
    for name, proto in sorted(sigs.items()):
        entry = snap_by_name.get(name)
        if entry:
            candidates.append((entry["address"], name, proto))

    uncovered = set(snap_by_name) - set(sigs)
    print(f"  Candidates (in snapshot): {len(candidates)}")
    print(f"  Not in snapshot:          {len(set(sigs) - set(snap_by_name))}")
    print(f"  Snapshot not in headers:  {len(uncovered)} (need LLM inspection)")

    if args.dry_run:
        print(f"\nDRY RUN — first 20 of {len(candidates)}:")
        for addr, name, proto in candidates[:20]:
            print(f"  {addr}  {proto}")
        return

    cp_path = REPO_ROOT / cfg["checkpoint"]
    cp = {"done": {}, "failed": {}}
    if args.resume and cp_path.exists():
        cp = json.loads(cp_path.read_text(encoding="utf-8"))

    done_set = set(cp["done"])
    pending  = [(a, n, p) for a, n, p in candidates
                if a not in done_set and a not in set(cp["failed"])]
    print(f"\nAlready done: {len(done_set)}  Pending: {len(pending)}")

    start = time.time()
    for i, (addr, name, proto) in enumerate(pending):
        result, err = mcp_call("set_function_prototype",
                               {"function_address": int(addr, 16), "prototype": proto})
        if err:
            print(f"  FAIL  {addr}  {name}  -- {err}")
            cp["failed"][addr] = {"name": name, "prototype": proto, "error": err}
        else:
            cp["done"][addr] = {"name": name, "prototype": proto}

        if (i + 1) % 20 == 0:
            cp_path.write_text(json.dumps(cp, indent=2), encoding="utf-8")
            elapsed = time.time() - start
            rate    = (i + 1) / elapsed
            rem     = (len(pending) - i - 1) / rate if rate > 0 else 0
            total   = len(done_set) + i + 1
            print(f"  [{100*total/len(candidates):.0f}%] {total}/{len(candidates)}  ~{rem:.0f}s remaining")
        time.sleep(0.1)

    cp_path.write_text(json.dumps(cp, indent=2), encoding="utf-8")
    elapsed = time.time() - start
    print(f"\nDone. Applied: {len(cp['done'])}  Failed: {len(cp['failed'])}  in {elapsed:.0f}s")
    if cp["failed"]:
        print("\nFailed (type fixes needed):")
        for addr, info in cp["failed"].items():
            print(f"  {addr}  {info['name']}  -- {info['error'][:80]}")

    # Merge results into consolidated progress file
    master_path = REPO_ROOT / ".ida-typing-progress.json"
    if master_path.exists():
        master = json.loads(master_path.read_text(encoding="utf-8"))
    else:
        master = {"schema_version": 2, "functions": []}

    master_by_addr = {e["address"]: e for e in master["functions"]}
    added = 0
    updated = 0
    from datetime import datetime, timezone
    now = datetime.now(timezone.utc).isoformat()
    for addr, info in cp["done"].items():
        hex_addr = addr if addr.startswith("0x") else hex(int(addr, 16))
        entry = master_by_addr.get(hex_addr)
        if entry:
            entry["typing_status"] = "typed"
            entry["prototype"] = info["prototype"]
            entry["last_updated"] = now
            updated += 1
        else:
            master["functions"].append({
                "address": hex_addr,
                "current_name": info["name"],
                "module": prefix,
                "source_file": f"headers:{args.module}",
                "queue_status": "done",
                "typing_status": "typed",
                "prototype": info["prototype"],
                "last_updated": now,
            })
            added += 1

    master_path.write_text(json.dumps(master, indent=2), encoding="utf-8")
    print(f"Progress file: +{added} added, {updated} updated -> {len(master['functions'])} total")


if __name__ == "__main__":
    main()
