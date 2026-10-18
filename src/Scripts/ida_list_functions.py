"""List IDA functions filtered by prefix, optionally comparing against source.

Usage examples:
    python src/Scripts/ida_list_functions.py --prefix mr
    python src/Scripts/ida_list_functions.py --prefix mr --source-root src --missing-only

The source comparison is identifier-based: a function is considered present if
its exact identifier appears anywhere in a .c/.h file under --source-root.
That makes it useful for finding functions that are missing even as pseudocode
reference blocks, declarations, or callers.
"""

import argparse
import json
import re
import sys
import urllib.request
from pathlib import Path


IDENT_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


def ida_call(uri: str, method: str, params: dict, timeout: int = 60):
    body = json.dumps({
        "jsonrpc": "2.0",
        "id": 1,
        "method": method,
        "params": params,
    }).encode()
    req = urllib.request.Request(
        uri,
        data=body,
        headers={"Content-Type": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=timeout) as response:
        payload = json.loads(response.read())
    if payload.get("error"):
        raise RuntimeError(f"IDA error: {payload['error']}")
    return payload.get("result")


def fetch_functions(uri: str, prefix: str, batch_size: int):
    functions = []
    offset = 0

    while True:
        result = ida_call(uri, "list_functions", {"offset": offset, "count": batch_size})
        if not result:
            break

        data = result.get("data", [])
        if not data:
            break

        for entry in data:
            name = entry.get("name", "")
            if name.startswith(prefix):
                functions.append({
                    "address": entry.get("address", ""),
                    "name": name,
                    "size": entry.get("size", ""),
                })

        next_offset = result.get("next_offset")
        if not next_offset or len(data) < batch_size:
            break
        offset = next_offset

    functions.sort(key=lambda item: item["name"].lower())
    return functions


def collect_source_identifiers(source_root: Path):
    identifiers = set()
    for path in source_root.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix.lower() not in {".c", ".h"}:
            continue

        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue

        identifiers.update(IDENT_RE.findall(text))
    return identifiers


def main():
    parser = argparse.ArgumentParser(description="List IDA functions filtered by prefix")
    parser.add_argument("--host", default="localhost",
                        help="IDA MCP host (default: localhost)")
    parser.add_argument("--port", type=int, default=13337,
                        help="IDA MCP port (default: 13337)")
    parser.add_argument("--prefix", required=True,
                        help="Function-name prefix to match, e.g. mr")
    parser.add_argument("--batch-size", type=int, default=500,
                        help="Functions per MCP request (default: 500)")
    parser.add_argument("--source-root",
                        help="Optional source root to compare against")
    parser.add_argument("--missing-only", action="store_true",
                        help="Only print functions whose name is absent from --source-root")
    parser.add_argument("--json", action="store_true",
                        help="Emit JSON instead of text")
    args = parser.parse_args()

    uri = f"http://{args.host}:{args.port}/mcp"

    try:
        functions = fetch_functions(uri, args.prefix, args.batch_size)
    except Exception as exc:
        print(f"Cannot query IDA MCP at {uri}: {exc}", file=sys.stderr)
        sys.exit(1)

    source_identifiers = None
    if args.source_root:
        source_root = Path(args.source_root)
        if not source_root.exists():
            print(f"Source root does not exist: {source_root}", file=sys.stderr)
            sys.exit(1)
        source_identifiers = collect_source_identifiers(source_root)

    rows = []
    for function in functions:
        row = dict(function)
        if source_identifiers is not None:
            row["present_in_source"] = function["name"] in source_identifiers
        rows.append(row)

    if args.missing_only and source_identifiers is not None:
        rows = [row for row in rows if not row["present_in_source"]]

    if args.json:
        json.dump(rows, sys.stdout, indent=2)
        sys.stdout.write("\n")
        return

    if source_identifiers is None:
        for row in rows:
            print(f"{row['address']:>10}  {row['name']}")
        print(f"\nTotal matches: {len(rows)}")
        return

    for row in rows:
        status = "present" if row["present_in_source"] else "MISSING"
        print(f"{status:>7}  {row['address']:>10}  {row['name']}")

    missing = sum(1 for row in rows if not row["present_in_source"])
    print(f"\nTotal matches: {len(rows)}")
    print(f"Missing in source: {missing}")


if __name__ == "__main__":
    main()
