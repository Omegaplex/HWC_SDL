"""
batch_prefix_rename.py
Batch-renames unnamed (sub_XXXXXX) functions in known library regions
to `jpg_XXXXXX`, `won_XXXXXX`, `titan_XXXXXX`, or `stl_XXXXXX` so they are
clearly marked as library code without requiring individual semantic analysis.

Usage:
    python src/Scripts/batch_prefix_rename.py [--dry-run] [--jpg-only] [--won-only] [--titan-only] [--stl-only]
"""

import json
import sys
import argparse
import requests
import time

MCP_URL = "http://localhost:13337/mcp"
SNAPSHOT_PATH = ".ida-snapshot.json"

# Confirmed library regions
REGIONS = [
    # (start_addr, end_addr_exclusive, prefix)
    (0x580000, 0x5A3600, "won"),   # WON auth C++ thiscall infrastructure (pre-Titan)
    (0x5A3600, 0x5D6EF0, "titan"), # WON/Titan service layer (TitanFTP, TitanMatch, TitanAd, etc.)
    (0x5D6EF0, 0x640000, "jpg"),   # libjpeg v6 (IJG)
    (0x680000, 0x6B9040, "won"),   # WONMsg routing classes
    (0x6B9040, 0x6BEAF0, "won"),   # WONCrypt / WONCommon
    (0x6BEF00, 0x6D0000, "won"),   # WON crypto extensions
    (0x6D0000, 0x70A060, "won"),   # WON/Magick/EasySocket C++ boilerplate (named functions skipped)
    (0x70A060, 0x730000, "stl"),   # MSVC 6.0 Dinkumware STL (locale, iostream, string instantiations)
    (0x890000, 0x89A000, "crt"),   # CRT/runtime helpers (high address section)
    (0xC50000, 0xC70000, "crt"),   # CRT/runtime helpers (high address section)
]

def mcp_call(method, params, req_id=1):
    body = {"jsonrpc": "2.0", "id": req_id, "method": method, "params": params}
    r = requests.post(MCP_URL, json=body, timeout=15)
    r.raise_for_status()
    return r.json()

def verify_sanity():
    checks = [
        ("univFreeAttackTargets", "0x541df0"),
        ("SaveLinkedListOfSpaceObjs", "0x5019c0"),
        ("aimPatrolMoveClose", "0x407a10"),
    ]
    print("Verifying MCP sanity anchors...")
    for name, expected_addr in checks:
        result = mcp_call("get_function_by_name", {"name": name})
        actual = result.get("result", {})
        if not actual:
            print(f"  FAIL: {name} not found")
            return False
        if actual.get("address", "").lower() != expected_addr:
            print(f"  FAIL: {name} -> {actual.get('address')} (expected {expected_addr})")
            return False
        print(f"  OK: {name} -> {actual['address']}")
    return True

def in_region(addr_int, prefix_filter):
    for start, end, prefix in REGIONS:
        if start <= addr_int < end:
            if prefix_filter is None or prefix == prefix_filter:
                return prefix
    return None

def load_snapshot():
    with open(SNAPSHOT_PATH, "r") as f:
        return json.load(f)

def save_snapshot(data):
    with open(SNAPSHOT_PATH, "w") as f:
        json.dump(data, f, indent=2)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true", help="Print renames without applying")
    parser.add_argument("--jpg-only", action="store_true")
    parser.add_argument("--won-only", action="store_true")
    parser.add_argument("--titan-only", action="store_true")
    parser.add_argument("--stl-only", action="store_true")
    parser.add_argument("--crt-only", action="store_true")
    args = parser.parse_args()

    prefix_filter = None
    if args.jpg_only:
        prefix_filter = "jpg"
    elif args.won_only:
        prefix_filter = "won"
    elif args.titan_only:
        prefix_filter = "titan"
    elif args.stl_only:
        prefix_filter = "stl"
    elif args.crt_only:
        prefix_filter = "crt"

    if not verify_sanity():
        print("Sanity check failed — aborting.")
        sys.exit(1)

    snapshot = load_snapshot()
    addr_to_idx = {}
    for i, entry in enumerate(snapshot):
        addr_to_idx[entry["address"].lower()] = i

    # Collect targets
    targets = []
    for entry in snapshot:
        if not entry["name"].startswith("sub_"):
            continue
        try:
            addr_int = int(entry["address"], 16)
        except ValueError:
            continue
        prefix = in_region(addr_int, prefix_filter)
        if prefix is None:
            continue
        # New name: e.g. jpg_5D7050 (strip 0x, uppercase hex)
        hex_part = entry["address"][2:].upper()
        new_name = f"{prefix}_{hex_part}"
        targets.append((entry["address"], addr_int, new_name, prefix))

    print(f"Found {len(targets)} targets to rename ({sum(1 for t in targets if t[3]=='jpg')} jpg, {sum(1 for t in targets if t[3]=='won')} won, {sum(1 for t in targets if t[3]=='titan')} titan, {sum(1 for t in targets if t[3]=='stl')} stl, {sum(1 for t in targets if t[3]=='crt')} crt)")

    if args.dry_run:
        for addr, _, new_name, _ in targets:
            print(f"  {addr} -> {new_name}")
        return

    accepted = 0
    failed = 0
    for i, (addr, addr_int, new_name, prefix) in enumerate(targets):
        result = mcp_call("rename_function", {"function_address": addr_int, "new_name": new_name}, req_id=i+1)
        if result.get("error"):
            print(f"  FAIL {addr}: {result['error']}")
            failed += 1
        else:
            idx = addr_to_idx.get(addr.lower())
            if idx is not None:
                snapshot[idx]["name"] = new_name
            accepted += 1
            if accepted % 100 == 0:
                print(f"  Progress: {accepted} renamed, {failed} failed ({i+1}/{len(targets)})")
                save_snapshot(snapshot)

    # Final snapshot save
    save_snapshot(snapshot)
    print(f"\nDone. Renamed: {accepted}, Failed: {failed}")

if __name__ == "__main__":
    main()
