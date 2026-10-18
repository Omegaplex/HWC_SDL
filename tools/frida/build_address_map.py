"""Build a Cataclysm symbol map for the Frida runtime, sourced from IDA.

The Frida runtime (`cataclysm_runtime.py`) defaults to reading the repo's
`.ida-snapshot.json` directly, which is already the IDA-derived function dump.
This script exists to (a) refresh that data straight from the live IDA MCP
server, and (b) optionally fold in data globals so the map can resolve named
globals (e.g. `universe`) in addition to functions.

Output JSON shape (consumed by SymbolMap.load):
    {
      "image_base": "0x400000",
      "functions": [ {"address": "0x401000", "name": "...", "size": "0x114"}, ... ]
    }

Usage:
    python build_address_map.py --output cataclysm_addresses.json
    python build_address_map.py --include-globals --output cataclysm_addresses.json
"""

from __future__ import annotations

import argparse
import json
import sys
import urllib.request
from pathlib import Path


def ida_call(uri: str, method: str, params: dict, timeout: int = 60):
    body = json.dumps({"jsonrpc": "2.0", "id": 1, "method": method, "params": params}).encode()
    req = urllib.request.Request(uri, data=body, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=timeout) as response:
        payload = json.loads(response.read())
    if payload.get("error"):
        raise RuntimeError(f"IDA error for {method}: {payload['error']}")
    return payload.get("result")


def paginate(uri: str, method: str, batch_size: int):
    offset = 0
    while True:
        result = ida_call(uri, method, {"offset": offset, "count": batch_size})
        if not result:
            break
        data = result.get("data", [])
        if not data:
            break
        for entry in data:
            yield entry
        if len(data) < batch_size:
            break
        offset = result.get("next_offset") or (offset + len(data))


def main() -> None:
    parser = argparse.ArgumentParser(description="Build Cataclysm IDA symbol map")
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=13337)
    parser.add_argument("--batch-size", type=int, default=1000)
    parser.add_argument("--image-base", default="0x400000")
    parser.add_argument("--include-globals", action="store_true",
                        help="Also include data globals from IDA list_globals")
    parser.add_argument("--output", default="cataclysm_addresses.json")
    args = parser.parse_args()

    uri = f"http://{args.host}:{args.port}/mcp"

    try:
        functions = [
            {"address": e.get("address", ""), "name": e.get("name", ""), "size": e.get("size", "")}
            for e in paginate(uri, "list_functions", args.batch_size)
            if e.get("name")
        ]
    except Exception as exc:  # noqa: BLE001
        print(f"Cannot query IDA MCP at {uri}: {exc}", file=sys.stderr)
        sys.exit(1)

    if args.include_globals:
        for e in paginate(uri, "list_globals", args.batch_size):
            if e.get("name") and e.get("address") not in (None, ""):
                functions.append({"address": e["address"], "name": e["name"], "size": e.get("size", "")})

    payload = {"image_base": args.image_base, "functions": functions}
    out = Path(args.output)
    out.write_text(json.dumps(payload, indent=1), encoding="utf-8")
    print(f"Wrote {len(functions)} symbols to {out} (image_base {args.image_base})")


if __name__ == "__main__":
    main()
