"""
Bulk-applies void __cdecl Name(void) to all Init_/Watch_ KAS mission callbacks
recorded in .ida-snapshot.json.

Checkpoints progress to .ida-typing-callbacks-progress.json so the run can be
resumed if interrupted.

Usage:
    python src/Scripts/type_kas_callbacks.py [--resume] [--dry-run]
"""

import json
import time
import argparse
import sys
from pathlib import Path
from datetime import datetime, timezone

import requests

REPO_ROOT = Path(__file__).resolve().parents[2]
SNAPSHOT   = REPO_ROOT / ".ida-snapshot.json"
CHECKPOINT = REPO_ROOT / ".ida-typing-callbacks-progress.json"
MCP_URL    = "http://localhost:13337/mcp"
PREFIXES   = ("Init_", "Watch_")


def mcp(method, params, retries=3):
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


def load_checkpoint():
    if CHECKPOINT.exists():
        return json.loads(CHECKPOINT.read_text(encoding="utf-8"))
    return {"done": {}, "failed": {}, "skipped": {}}


def save_checkpoint(cp):
    CHECKPOINT.write_text(json.dumps(cp, indent=2), encoding="utf-8")


def load_candidates():
    data = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    callbacks = [e for e in data if e["name"].startswith(PREFIXES)]
    callbacks.sort(key=lambda e: int(e["address"], 16))
    return callbacks


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--resume", action="store_true", help="Skip already-done addresses")
    parser.add_argument("--dry-run", action="store_true", help="Print what would be applied, don't call IDA")
    args = parser.parse_args()

    candidates = load_candidates()
    cp = load_checkpoint() if args.resume else {"done": {}, "failed": {}, "skipped": {}}

    total  = len(candidates)
    done   = set(cp["done"])
    failed = set(cp["failed"])

    pending = [c for c in candidates if c["address"] not in done and c["address"] not in failed]

    print(f"Candidates: {total}  |  Already done: {len(done)}  |  Failed: {len(failed)}  |  Pending: {len(pending)}")
    if args.dry_run:
        print("DRY RUN — first 10 pending:")
        for c in pending[:10]:
            print(f"  {c['address']}  void __cdecl {c['name']}(void)")
        return

    start = time.time()
    for i, c in enumerate(pending):
        addr_int = int(c["address"], 16)
        proto    = f"void __cdecl {c['name']}(void)"

        result, err = mcp("set_function_prototype", {"function_address": addr_int, "prototype": proto})

        if err:
            print(f"  FAIL  {c['address']}  {c['name']}  -- {err}")
            cp["failed"][c["address"]] = {"name": c["name"], "error": err}
        else:
            cp["done"][c["address"]] = {"name": c["name"], "prototype": proto}

        # Save checkpoint every 25 entries
        if (i + 1) % 25 == 0:
            save_checkpoint(cp)
            elapsed = time.time() - start
            rate    = (i + 1) / elapsed
            remaining = (len(pending) - i - 1) / rate if rate > 0 else 0
            pct = 100 * (len(done) + i + 1) / total
            print(f"  [{pct:.0f}%] {len(done)+i+1}/{total}  {elapsed:.0f}s elapsed  ~{remaining:.0f}s remaining")

        time.sleep(0.1)  # small back-off; IDA MCP serializes anyway

    save_checkpoint(cp)
    elapsed = time.time() - start
    print(f"\nDone. Applied: {len(cp['done'])}  Failed: {len(cp['failed'])}  in {elapsed:.0f}s")
    print(f"Checkpoint: {CHECKPOINT}")


if __name__ == "__main__":
    main()
