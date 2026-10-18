"""
Bulk-applies source-backed prototypes to kas* utility functions using
declarations from KAS.h, KASFunc.h, and KASFunc.c native definitions.

Only applies signatures that exist in source — no inference.
Functions not covered by source are skipped (they need manual inspection).

Usage:
    python src/Scripts/type_kas_utilities.py [--dry-run] [--resume]
"""

import json
import re
import time
import argparse
from pathlib import Path

import requests

REPO_ROOT  = Path(__file__).resolve().parents[2]
SNAPSHOT   = REPO_ROOT / ".ida-snapshot.json"
CHECKPOINT = REPO_ROOT / ".ida-typing-kas-utils-progress.json"
MCP_URL    = "http://localhost:13337/mcp"

SOURCE_FILES = [
    REPO_ROOT / "src" / "Game" / "KAS.h",
    REPO_ROOT / "src" / "Game" / "KASFunc.h",
    REPO_ROOT / "src" / "Game" / "KASFunc.c",
]


def parse_source_sigs(text):
    """Extract function signatures from source text, strip decompile blocks."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"#if 0.*?#endif", "", text, flags=re.DOTALL)
    matches = re.findall(
        r"^([^\n#][^;{\n]*\b(kas\w+)\s*\([^)]*\))\s*[;{]",
        text, re.MULTILINE
    )
    result = {}
    for sig, name in matches:
        sig = sig.strip().rstrip("{").strip()
        # KAS.h takes priority — process in order, later files don't override
        if name not in result:
            result[name] = sig
    return result


def build_prototype(sig):
    """Convert a C source signature to an IDA-friendly prototype string."""
    sig = re.sub(r"\bstatic\b", "", sig).strip()
    # Ensure __cdecl is present
    # Match: return_type name(params)
    m = re.match(r"^(.*?\s)(\w+)\s*(\(.*\))$", sig, re.DOTALL)
    if not m:
        return None
    ret, name, params = m.group(1).strip(), m.group(2), m.group(3)
    # Insert __cdecl if not already there
    if "__cdecl" not in ret and "__stdcall" not in ret:
        return f"{ret} __cdecl {name}{params}"
    return f"{ret} {name}{params}"


def mcp_call(method, params, retries=3):
    payload = {"jsonrpc": "2.0", "id": 1, "method": method, "params": params}
    for attempt in range(retries):
        try:
            r = requests.post(MCP_URL, json=payload, timeout=30)
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

    # Build source signature map (KAS.h first for priority)
    all_sigs = {}
    for path in reversed(SOURCE_FILES):  # reversed so KAS.h wins
        text = path.read_text(encoding="utf-8", errors="replace")
        sigs = parse_source_sigs(text)
        all_sigs.update(sigs)

    # Load snapshot kas* functions
    data = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    kas_snap = {e["name"]: e for e in data if e["name"].startswith("kas")}

    # Candidates: snapshot entries that have a source signature
    candidates = []
    skipped_no_sig = []
    for name, entry in sorted(kas_snap.items(), key=lambda x: int(x[1]["address"], 16)):
        if name in all_sigs:
            proto = build_prototype(all_sigs[name])
            if proto:
                candidates.append((entry["address"], name, proto))
        else:
            skipped_no_sig.append(name)

    print(f"Source-backed candidates: {len(candidates)}")
    print(f"Skipped (no source sig):  {len(skipped_no_sig)}")

    if args.dry_run:
        print("\nDRY RUN — first 15 candidates:")
        for addr, name, proto in candidates[:15]:
            print(f"  {addr}  {proto}")
        print(f"\nSkipped functions ({len(skipped_no_sig)}) need manual inspection.")
        return

    # Load checkpoint
    cp = {"done": {}, "failed": {}, "skipped": {}}
    if args.resume and CHECKPOINT.exists():
        cp = json.loads(CHECKPOINT.read_text(encoding="utf-8"))

    done_set   = set(cp["done"])
    failed_set = set(cp["failed"])
    pending    = [(a, n, p) for a, n, p in candidates
                  if a not in done_set and a not in failed_set]

    print(f"Already done: {len(done_set)}  Pending: {len(pending)}")

    start = time.time()
    for i, (addr, name, proto) in enumerate(pending):
        addr_int = int(addr, 16)
        result, err = mcp_call("set_function_prototype",
                               {"function_address": addr_int, "prototype": proto})
        if err:
            print(f"  FAIL  {addr}  {name}  -- {err}")
            cp["failed"][addr] = {"name": name, "error": err}
        else:
            cp["done"][addr] = {"name": name, "prototype": proto}

        if (i + 1) % 25 == 0:
            CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")
            elapsed = time.time() - start
            rate = (i + 1) / elapsed
            remaining = (len(pending) - i - 1) / rate if rate > 0 else 0
            pct = 100 * (len(done_set) + i + 1) / len(candidates)
            print(f"  [{pct:.0f}%] {len(done_set)+i+1}/{len(candidates)}"
                  f"  {elapsed:.0f}s elapsed  ~{remaining:.0f}s remaining")

        time.sleep(0.1)

    CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")
    elapsed = time.time() - start
    print(f"\nDone. Applied: {len(cp['done'])}  Failed: {len(cp['failed'])}  in {elapsed:.0f}s")
    print(f"Functions needing manual inspection: {len(skipped_no_sig)}")


if __name__ == "__main__":
    main()
