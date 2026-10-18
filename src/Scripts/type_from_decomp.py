"""
Batch-applies the current HexRays inferred prototype for untyped named functions.

After typing 1300+ callers/callees, HexRays has already propagated better types
into many functions. This script locks that inference in by reading the decompiled
first line and applying it as the explicit prototype.

For functions where HexRays only infers (int, int, ...) with no improvement,
these are flagged for manual review.

Usage:
    python src/Scripts/type_from_decomp.py --module tr [--dry-run]
    python src/Scripts/type_from_decomp.py --modules tr sp rm lm sm tut [--dry-run]
"""

import json
import re
import time
import argparse
from pathlib import Path
from datetime import datetime, timezone

import requests

REPO_ROOT = Path(__file__).resolve().parents[2]
MCP_URL   = "http://localhost:13337/mcp"

# Prototype line pattern: extracts return type + name + params from decompiler output
PROTO_RE = re.compile(
    r"^((?:(?:unsigned|signed|struct|const|__int64)\s+)?"
    r"[A-Za-z_][A-Za-z0-9_ ]*?\*?)"          # return type (greedy-ish)
    r"\s+(__(?:cdecl|stdcall|fastcall))\s+"   # calling convention
    r"([A-Za-z_][A-Za-z0-9_?]*)"             # function name
    r"\s*(\([^)]*(?:\([^)]*\)[^)]*)*\))"     # params (nested-paren tolerant)
    r"\s*$"
)

# Heuristic: prototype is "unimproved" if every param is just int/unsigned/void * with no name info
WEAK_PARAM_RE = re.compile(r"^\s*(int|unsigned int|__int64|void \*|char \*)\s*$")


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


def parse_proto_from_decomp(text: str, expected_name: str | None = None) -> str | None:
    """Extract the function prototype from the first non-blank line of decompiler output."""
    # Strip IDA line annotations: /* line: N, address: 0xXXX */
    line_annot = re.compile(r"/\*\s*line:\s*\d+(?:,\s*address:\s*0x[0-9a-fA-F]+)?\s*\*/\s*")

    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("//") or line.startswith("#"):
            continue
        line = line_annot.sub("", line).strip()
        if not line:
            continue
        if "__cdecl" in line or "__stdcall" in line or "__fastcall" in line:
            proto = line.rstrip("{").strip()
            # Replace IDA's (possibly mangled/sub_*) name with the known snapshot name
            if expected_name:
                proto = re.sub(r"\b[A-Za-z_][A-Za-z0-9_?]*\b(?=\s*\()", expected_name, proto, count=1)
            return proto
        break
    return None


def is_weak_proto(proto: str) -> bool:
    """Return True if prototype has no type information beyond bare ints."""
    m = re.search(r"\((.+)\)", proto)
    if not m:
        return False
    params_str = m.group(1).strip()
    if params_str in ("void", ""):
        return False
    params = [p.strip() for p in params_str.split(",")]
    # Weak if ALL params are bare int/unsigned/pointer with no named struct types
    for p in params:
        # Remove parameter name (last word)
        p_type = re.sub(r"\b[a-z][a-zA-Z0-9_]*\s*$", "", p).strip()
        if not WEAK_PARAM_RE.match(p_type):
            return False
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--module",  help="Single module prefix")
    parser.add_argument("--modules", nargs="+", help="Multiple module prefixes")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    prefixes = []
    if args.modules:
        prefixes = args.modules
    elif args.module:
        prefixes = [args.module]
    else:
        parser.error("Provide --module or --modules")

    snap   = json.loads((REPO_ROOT / ".ida-snapshot.json").read_text(encoding="utf-8"))
    master_path = REPO_ROOT / ".ida-typing-progress.json"
    master = json.loads(master_path.read_text(encoding="utf-8"))
    typed_addrs = {e["address"] for e in master["functions"]
                   if e.get("typing_status") in ("typed", "improved")}
    master_by_addr = {e["address"]: e for e in master["functions"]}

    now = datetime.now(timezone.utc).isoformat()
    total_applied = 0
    total_weak    = 0
    total_failed  = 0
    weak_list     = []

    for prefix in prefixes:
        pat = re.compile(r"^" + re.escape(prefix) + r"[A-Z_]")
        targets = [e for e in snap
                   if pat.match(e["name"]) and e["address"] not in typed_addrs]
        print(f"\n=== {prefix}: {len(targets)} untyped ===")

        for entry in targets:
            addr_hex = entry["address"]
            addr_int = int(addr_hex, 16)
            name     = entry["name"]

            decomp, err = mcp("decompile_function", {"address": addr_int})
            if err or not decomp:
                print(f"  DECOMP-FAIL {addr_hex} {name}: {err}")
                total_failed += 1
                continue

            text = decomp if isinstance(decomp, str) else decomp.get("code", "")
            proto = parse_proto_from_decomp(text, name.rstrip("?"))
            if not proto:
                print(f"  PARSE-FAIL  {addr_hex} {name}")
                total_failed += 1
                continue

            # Normalise name in prototype (IDA may use mangled name with '?')
            proto = re.sub(r"\b" + re.escape(name.rstrip("?")) + r"\??\b", name.rstrip("?"), proto)

            weak = is_weak_proto(proto)
            if weak:
                weak_list.append((addr_hex, name, proto))
                total_weak += 1
                if not args.dry_run:
                    # Still apply — locking in void params is better than nothing
                    pass

            if args.dry_run:
                flag = " [WEAK]" if weak else ""
                print(f"  {addr_hex}  {proto}{flag}")
                continue

            result, err = mcp("set_function_prototype",
                              {"function_address": addr_int, "prototype": proto})
            if err:
                print(f"  TYPE-FAIL   {addr_hex} {name}: {err[:80]}")
                total_failed += 1
                continue

            # Update progress
            e = master_by_addr.get(addr_hex)
            if e:
                e["typing_status"] = "typed"
                e["prototype"]     = proto
                e["last_updated"]  = now
            else:
                new_e = {
                    "address":      addr_hex,
                    "current_name": name,
                    "module":       prefix,
                    "source_file":  "decomp:auto",
                    "queue_status": "done",
                    "typing_status": "typed",
                    "prototype":    proto,
                    "last_updated": now,
                }
                master["functions"].append(new_e)
                master_by_addr[addr_hex] = new_e

            status = "[WEAK]" if weak else "OK"
            print(f"  {status:6} {addr_hex}  {proto[:90]}")
            total_applied += 1
            time.sleep(0.05)

        if not args.dry_run:
            master_path.write_text(json.dumps(master, indent=2), encoding="utf-8")

    print(f"\n{'DRY RUN — ' if args.dry_run else ''}Summary:")
    print(f"  Applied : {total_applied}")
    print(f"  Weak    : {total_weak} (params still int/void*)")
    print(f"  Failed  : {total_failed}")
    if weak_list:
        print(f"\nWeak prototypes (review recommended):")
        for addr, name, proto in weak_list[:20]:
            print(f"  {addr}  {name}")
        if len(weak_list) > 20:
            print(f"  ... and {len(weak_list) - 20} more")


if __name__ == "__main__":
    main()
