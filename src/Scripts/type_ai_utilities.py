"""
Applies source-backed prototypes to named ai* functions in IDA.
Parses HW1 AI headers (higher confidence), then Cataclysm AI headers as override layer.

Usage:
    python src/Scripts/type_ai_utilities.py [--dry-run] [--resume]
"""

import json
import os
import re
import time
import argparse
from pathlib import Path

import requests

REPO_ROOT  = Path(__file__).resolve().parents[2]
HW1_ROOT   = Path(os.environ.get("HW_ROOT", "")) / "src" / "Game"
SNAPSHOT   = REPO_ROOT / ".ida-snapshot.json"
CHECKPOINT = REPO_ROOT / ".ida-typing-ai-utils-progress.json"
MCP_URL    = "http://localhost:13337/mcp"

# AI headers to parse (HW1 authoritative, then Cataclysm overrides)
HW1_HEADERS = [
    HW1_ROOT / "AIMoves.h",
    HW1_ROOT / "AITeam.h",
    HW1_ROOT / "AIUtilities.h",
    HW1_ROOT / "AIPlayer.h",
    HW1_ROOT / "AIEvents.h",
    HW1_ROOT / "AIOrders.h",
    HW1_ROOT / "AIHandler.h",
    HW1_ROOT / "AItrack.h",
    HW1_ROOT / "AIFleetMan.h",
    HW1_ROOT / "AIShip.h",
    HW1_ROOT / "AIVar.h",
    HW1_ROOT / "AIAttackMan.h",
    HW1_ROOT / "AIDefenseMan.h",
    HW1_ROOT / "AIResourceMan.h",
]

CAT_HEADERS = [
    REPO_ROOT / "src" / "Game" / "AIMoves.h",
    REPO_ROOT / "src" / "Game" / "AITeam.h",
    REPO_ROOT / "src" / "Game" / "AIUtilities.h",
    REPO_ROOT / "src" / "Game" / "AIPlayer.h",
    REPO_ROOT / "src" / "Game" / "AIEvents.h",
    REPO_ROOT / "src" / "Game" / "AIOrders.h",
    REPO_ROOT / "src" / "Game" / "AIHandler.h",
    REPO_ROOT / "src" / "Game" / "AITrack.h",
    REPO_ROOT / "src" / "Game" / "AIFleetMan.h",
    REPO_ROOT / "src" / "Game" / "AIShip.h",
    REPO_ROOT / "src" / "Game" / "AIVar.h",
    REPO_ROOT / "src" / "Game" / "AIAttackMan.h",
    REPO_ROOT / "src" / "Game" / "AIDefenseMan.h",
    REPO_ROOT / "src" / "Game" / "AIResourceMan.h",
]

# Regex: match a C function declaration on a single line (no typedef, no block comment)
# Captures return type + name + params up to closing semicolon
DECL_RE = re.compile(
    r"^(?!.*typedef)(?!.*//)"   # not typedef, not line comment
    r"\s*"
    r"((?:(?:unsigned|signed|struct|const)\s+)?"  # optional qualifier
    r"[A-Za-z_][A-Za-z0-9_]*(?:\s*\*+)?)"         # return type (with optional ptr)
    r"\s+"
    r"(ai[A-Za-z_][A-Za-z0-9_]*)"                  # function name starting with 'ai'
    r"\s*"
    r"(\([^)]*\))"                                  # parameter list (single line)
    r"\s*;"
)


def parse_headers(header_list: list[Path]) -> dict[str, str]:
    """Parse C headers for ai* function declarations. Returns {name: prototype}."""
    sigs: dict[str, str] = {}
    for hdr in header_list:
        if not hdr.exists():
            continue
        # Strip block comments before parsing
        text = hdr.read_text(encoding="utf-8", errors="replace")
        # Remove block comments
        text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
        for line in text.splitlines():
            m = DECL_RE.match(line)
            if m:
                ret_type = m.group(1).strip()
                name     = m.group(2).strip()
                params   = m.group(3).strip()
                proto    = f"{ret_type} __cdecl {name}{params}"
                # Normalize whitespace
                proto    = re.sub(r"\s+", " ", proto).strip()
                sigs[name] = proto
    return sigs


def build_proto(raw_proto: str) -> str:
    """Ensure struct keyword for types that IDA needs as forward references."""
    # IDA needs 'struct X *' for types not in its local type list
    for t in ("AITeamMove", "AITeamCommand", "AIVar", "AITeam", "AIPlayer",
              "AIEvents", "AIHandler", "Volume", "hvector"):
        # Replace bare 'TypeName *' not already prefixed with 'struct'
        raw_proto = re.sub(rf"(?<!struct )\b{t}\b(?=\s*[,*\)])", f"struct {t}", raw_proto)
    return raw_proto


def mcp_call(method: str, params: dict, retries: int = 3):
    for attempt in range(retries):
        try:
            r = requests.post(MCP_URL, json={"jsonrpc": "2.0", "id": 1, "method": method, "params": params}, timeout=30)
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
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--resume",  action="store_true")
    args = parser.parse_args()
    if not os.environ.get("HW_ROOT"):
        parser.error("HW_ROOT environment variable must be set to the original Homeworld checkout root.")

    # Parse signatures: HW1 first, then Cataclysm overrides
    sigs = parse_headers(HW1_HEADERS)
    sigs.update(parse_headers(CAT_HEADERS))
    print(f"Signatures parsed from headers: {len(sigs)}")

    # Load snapshot
    snap = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    snap_by_name = {e["name"]: e for e in snap if e["name"].startswith("ai")}

    # Build candidate list: only functions in both sigs and snapshot
    candidates = []
    for name, proto in sorted(sigs.items()):
        entry = snap_by_name.get(name)
        if entry:
            candidates.append((entry["address"], name, build_proto(proto)))
        # else: named in headers but not yet identified in IDA — skip

    not_in_snap = set(sigs) - set(snap_by_name)
    print(f"  In snapshot:     {len(candidates)}")
    print(f"  Not in snapshot: {len(not_in_snap)} (not yet identified in IDA)")
    print(f"  ai* in snapshot not in headers: {len(set(snap_by_name) - set(sigs))} (Cataclysm additions)")

    if args.dry_run:
        print("\nDRY RUN — first 20 candidates:")
        for addr, name, proto in candidates[:20]:
            print(f"  {addr}  {proto}")
        return

    cp = {"done": {}, "failed": {}}
    if args.resume and CHECKPOINT.exists():
        cp = json.loads(CHECKPOINT.read_text(encoding="utf-8"))

    done_set = set(cp["done"])
    pending  = [(a, n, p) for a, n, p in candidates if a not in done_set and a not in set(cp["failed"])]
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

        if (i + 1) % 25 == 0:
            CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")
            elapsed = time.time() - start
            rate    = (i + 1) / elapsed
            rem     = (len(pending) - i - 1) / rate if rate > 0 else 0
            total_done = len(done_set) + i + 1
            print(f"  [{100*total_done/len(candidates):.0f}%]"
                  f" {total_done}/{len(candidates)}"
                  f"  ~{rem:.0f}s remaining")
        time.sleep(0.1)

    CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")
    elapsed = time.time() - start
    print(f"\nDone. Applied: {len(cp['done'])}  Failed: {len(cp['failed'])}  in {elapsed:.0f}s")
    if cp["failed"]:
        print("\nFailed prototypes (need struct/type fixes):")
        for addr, info in cp["failed"].items():
            print(f"  {addr}  {info['name']}  -- {info['error'][:80]}")


if __name__ == "__main__":
    main()
