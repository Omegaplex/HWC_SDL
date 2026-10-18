"""
Find raw decompiler symbol names that still appear in the Cataclysm tree.

This scanner inventories:
  - repo text references in source/tooling artifacts
  - unnamed functions still present in .ida-snapshot.json
  - optional live IDA/MCP globals, functions, and structs

The output is designed to seed SQL-driven audit work: every occurrence is emitted
as a finding, and every distinct symbol also gets an aggregate summary record.

Usage:
    python src/Scripts/find_raw_decompiler_names.py
    python src/Scripts/find_raw_decompiler_names.py --json-out temp\\raw-name-audit.json
    python src/Scripts/find_raw_decompiler_names.py --sqlite-out temp\\raw-name-audit.sqlite
    python src/Scripts/find_raw_decompiler_names.py --include-mcp --prefix sub_ --prefix dword_
"""

from __future__ import annotations

import argparse
import json
import re
import sqlite3
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Iterable

import requests


REPO_ROOT = Path(__file__).resolve().parents[2]
VCXPROJ_PATH = REPO_ROOT / "Cataclysm.vcxproj"
MCP_URL = "http://localhost:13337/mcp"
DEFAULT_SCAN_PATHS = (
    "src",
    "scripts",
    ".ida-typing-progress.json",
)
DEFAULT_TEXT_SUFFIXES = {".c", ".h", ".cpp", ".hpp", ".py", ".ps1", ".json", ".txt"}
COMPILED_SOURCE_SUFFIXES = {".c", ".cpp"}
EXCLUDED_DIR_NAMES = {
    ".git",
    ".vs",
    "Debug",
    "Release",
    "hw_rgl",
    "lib",
    "temp",
    "vcpkg_installed",
    "worktrees",
}
DEFAULT_PREFIX_KINDS = {
    "dword_": "global",
    "flt_": "global",
    "stru_": "struct",
    "sub_": "function",
}
MCP_SANITY_CHECKS = (
    ("univFreeAttackTargets", "0x541df0"),
    ("SaveLinkedListOfSpaceObjs", "0x5019c0"),
    ("aimPatrolMoveClose", "0x407a10"),
)


@dataclass(frozen=True)
class PrefixSpec:
    prefix: str
    symbol_kind: str


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def normalize_relpath(path: Path) -> str:
    return str(path.relative_to(REPO_ROOT)).replace("/", "\\")


def normalize_project_relpath(value: str) -> str:
    return value.replace("/", "\\").strip().lower()


def build_prefix_specs(prefixes: list[str] | None) -> list[PrefixSpec]:
    if not prefixes:
        return [PrefixSpec(prefix, kind) for prefix, kind in DEFAULT_PREFIX_KINDS.items()]

    specs: list[PrefixSpec] = []
    for prefix in prefixes:
        if prefix not in DEFAULT_PREFIX_KINDS:
            raise SystemExit(
                f"Unsupported prefix '{prefix}'. Supported prefixes: "
                + ", ".join(sorted(DEFAULT_PREFIX_KINDS))
            )
        specs.append(PrefixSpec(prefix, DEFAULT_PREFIX_KINDS[prefix]))
    return specs


def build_symbol_regex(specs: Iterable[PrefixSpec]) -> re.Pattern[str]:
    parts = [re.escape(spec.prefix) + r"[0-9A-Fa-f]+" for spec in specs]
    return re.compile(r"\b(?:" + "|".join(parts) + r")\b")


def symbol_kind_for(specs: Iterable[PrefixSpec], symbol: str) -> str:
    for spec in specs:
        if symbol.startswith(spec.prefix):
            return spec.symbol_kind
    return "unknown"


def default_context_for(path: Path) -> str:
    suffix = path.suffix.lower()
    if suffix in {".c", ".h", ".cpp", ".hpp"}:
        return "active_code"
    if suffix in {".py", ".ps1"}:
        return "tooling"
    return "artifact"


def trim_line_text(text: str) -> str:
    return text.rstrip("\r\n")[:400]


def load_compiled_source_paths(vcxproj_path: Path) -> set[str]:
    tree = ET.parse(vcxproj_path)
    compiled_paths: set[str] = set()
    for element in tree.getroot().iter():
        if not element.tag.endswith("ClCompile"):
            continue
        include_path = element.attrib.get("Include")
        if not include_path:
            continue
        compiled_paths.add(normalize_project_relpath(include_path))
    return compiled_paths


def should_include_path(path: Path, compiled_source_paths: set[str], include_noncompiled_source: bool) -> bool:
    relative_parts = path.relative_to(REPO_ROOT).parts
    if any(part in EXCLUDED_DIR_NAMES for part in relative_parts):
        return False
    if path.suffix.lower() not in DEFAULT_TEXT_SUFFIXES:
        return False
    if include_noncompiled_source:
        return True
    if path.suffix.lower() not in COMPILED_SOURCE_SUFFIXES:
        return True
    return normalize_relpath(path).lower() in compiled_source_paths


def iter_text_files(
    scan_paths: Iterable[str],
    compiled_source_paths: set[str],
    include_noncompiled_source: bool,
) -> list[Path]:
    files: list[Path] = []
    seen: set[Path] = set()

    for rel in scan_paths:
        candidate = REPO_ROOT / rel
        if not candidate.exists():
            continue
        if candidate.is_file():
            if not should_include_path(candidate, compiled_source_paths, include_noncompiled_source):
                continue
            resolved = candidate.resolve()
            if resolved not in seen:
                files.append(candidate)
                seen.add(resolved)
            continue

        for path in candidate.rglob("*"):
            if not path.is_file():
                continue
            if not should_include_path(path, compiled_source_paths, include_noncompiled_source):
                continue
            resolved = path.resolve()
            if resolved in seen:
                continue
            files.append(path)
            seen.add(resolved)

    return sorted(files)


def new_finding(
    *,
    symbol: str,
    symbol_kind: str,
    source_kind: str,
    context_kind: str,
    file_path: str,
    occurrence_count: int = 1,
    line_number: int | None = None,
    line_text: str | None = None,
    address: str | None = None,
    size: str | None = None,
) -> dict[str, Any]:
    return {
        "symbol": symbol,
        "symbol_kind": symbol_kind,
        "source_kind": source_kind,
        "context_kind": context_kind,
        "file_path": file_path,
        "line_number": line_number,
        "line_text": line_text,
        "occurrence_count": occurrence_count,
        "address": address,
        "size": size,
    }


def scan_text_file(path: Path, symbol_re: re.Pattern[str], specs: list[PrefixSpec]) -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    context_stack: list[str] = []
    relpath = normalize_relpath(path)
    default_context = default_context_for(path)

    with path.open("r", encoding="utf-8", errors="replace") as handle:
        for line_number, line in enumerate(handle, start=1):
            stripped = line.strip()
            upper = stripped.upper()

            if upper.startswith("#IF 0"):
                if "CAT2:" in upper or "DECOMPILED" in upper:
                    context_stack.append("disabled_reference")
                else:
                    context_stack.append("disabled_block")
            elif upper.startswith("#ENDIF") and context_stack:
                context_stack.pop()

            matches = symbol_re.findall(line)
            if not matches:
                continue

            if context_stack:
                context_kind = context_stack[-1]
            else:
                context_kind = default_context

            for symbol, count in Counter(matches).items():
                findings.append(
                    new_finding(
                        symbol=symbol,
                        symbol_kind=symbol_kind_for(specs, symbol),
                        source_kind="repo_text",
                        context_kind=context_kind,
                        file_path=relpath,
                        line_number=line_number,
                        line_text=trim_line_text(line),
                        occurrence_count=count,
                    )
                )

    return findings


def scan_snapshot(path: Path, symbol_re: re.Pattern[str], specs: list[PrefixSpec]) -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    data = json.loads(path.read_text(encoding="utf-8"))
    relpath = normalize_relpath(path)

    for entry in data:
        name = str(entry.get("name", ""))
        if not symbol_re.fullmatch(name):
            continue
        findings.append(
            new_finding(
                symbol=name,
                symbol_kind=symbol_kind_for(specs, name),
                source_kind="ida_snapshot",
                context_kind="ida_snapshot",
                file_path=relpath,
                address=str(entry.get("address", "")),
                size=str(entry.get("size", "")),
            )
        )

    return findings


def mcp_call(method: str, params: dict[str, Any], timeout: int = 45) -> Any:
    response = requests.post(
        MCP_URL,
        json={"jsonrpc": "2.0", "id": 1, "method": method, "params": params},
        timeout=timeout,
    )
    response.raise_for_status()
    payload = response.json()
    if "error" in payload:
        raise RuntimeError(f"{method} failed: {payload['error']}")
    return payload.get("result")


def verify_mcp_sanity() -> None:
    for name, expected_address in MCP_SANITY_CHECKS:
        result = mcp_call("get_function_by_name", {"name": name})
        actual = str(result.get("address", "")).lower()
        if actual != expected_address:
            raise RuntimeError(f"MCP sanity check failed for {name}: got {actual}, expected {expected_address}")


def scan_live_ida(symbol_re: re.Pattern[str], specs: list[PrefixSpec], page_size: int) -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    verify_mcp_sanity()

    for method, source_kind in (
        ("list_functions", "live_ida_function"),
        ("list_globals", "live_ida_global"),
    ):
        offset = 0
        while True:
            result = mcp_call(method, {"offset": offset, "count": page_size})
            items = result.get("data", [])
            for item in items:
                name = str(item.get("name", ""))
                if not symbol_re.fullmatch(name):
                    continue
                findings.append(
                    new_finding(
                        symbol=name,
                        symbol_kind=symbol_kind_for(specs, name),
                        source_kind=source_kind,
                        context_kind="live_ida",
                        file_path="<live-ida>",
                        address=str(item.get("address", "")),
                        size=str(item.get("size", "")),
                    )
                )

            next_offset = result.get("next_offset")
            if next_offset is None or not items:
                break
            offset = int(next_offset)

    if any(spec.symbol_kind == "struct" for spec in specs):
        for spec in specs:
            if spec.symbol_kind != "struct":
                continue
            items = mcp_call("search_structures", {"filter": spec.prefix}, timeout=90)
            for item in items:
                name = str(item.get("name", ""))
                if not symbol_re.fullmatch(name):
                    continue
                findings.append(
                    new_finding(
                        symbol=name,
                        symbol_kind=spec.symbol_kind,
                        source_kind="live_ida_struct",
                        context_kind="live_ida",
                        file_path="<live-ida>",
                        address=None,
                        size=str(item.get("size", "")),
                    )
                )

    return findings


def aggregate_symbols(findings: list[dict[str, Any]]) -> list[dict[str, Any]]:
    aggregates: dict[str, dict[str, Any]] = {}

    for finding in findings:
        symbol = finding["symbol"]
        aggregate = aggregates.setdefault(
            symbol,
            {
                "symbol": symbol,
                "symbol_kind": finding["symbol_kind"],
                "total_occurrences": 0,
                "repo_occurrences": 0,
                "active_code_occurrences": 0,
                "disabled_reference_occurrences": 0,
                "snapshot_occurrences": 0,
                "live_ida_occurrences": 0,
                "tooling_occurrences": 0,
                "artifact_occurrences": 0,
                "files": set(),
                "addresses": set(),
            },
        )

        count = int(finding["occurrence_count"])
        aggregate["total_occurrences"] += count
        aggregate["files"].add(finding["file_path"])
        if finding.get("address"):
            aggregate["addresses"].add(finding["address"])

        source_kind = finding["source_kind"]
        context_kind = finding["context_kind"]
        if source_kind == "repo_text":
            aggregate["repo_occurrences"] += count
        elif source_kind == "ida_snapshot":
            aggregate["snapshot_occurrences"] += count
        elif source_kind.startswith("live_ida"):
            aggregate["live_ida_occurrences"] += count

        if context_kind == "active_code":
            aggregate["active_code_occurrences"] += count
        elif context_kind == "disabled_reference":
            aggregate["disabled_reference_occurrences"] += count
        elif context_kind == "tooling":
            aggregate["tooling_occurrences"] += count
        elif context_kind == "artifact":
            aggregate["artifact_occurrences"] += count

    rows: list[dict[str, Any]] = []
    for aggregate in aggregates.values():
        row = dict(aggregate)
        row["files"] = sorted(row["files"])
        row["addresses"] = sorted(row["addresses"])
        rows.append(row)

    rows.sort(key=lambda item: (-item["total_occurrences"], item["symbol"]))
    return rows


def write_sqlite(path: Path, findings: list[dict[str, Any]], symbols: list[dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with sqlite3.connect(path) as conn:
        conn.executescript(
            """
            DROP TABLE IF EXISTS findings;
            DROP TABLE IF EXISTS symbols;

            CREATE TABLE findings (
                id INTEGER PRIMARY KEY,
                symbol TEXT NOT NULL,
                symbol_kind TEXT NOT NULL,
                source_kind TEXT NOT NULL,
                context_kind TEXT NOT NULL,
                file_path TEXT NOT NULL,
                line_number INTEGER,
                line_text TEXT,
                occurrence_count INTEGER NOT NULL,
                address TEXT,
                size TEXT
            );

            CREATE TABLE symbols (
                symbol TEXT PRIMARY KEY,
                symbol_kind TEXT NOT NULL,
                total_occurrences INTEGER NOT NULL,
                repo_occurrences INTEGER NOT NULL,
                active_code_occurrences INTEGER NOT NULL,
                disabled_reference_occurrences INTEGER NOT NULL,
                snapshot_occurrences INTEGER NOT NULL,
                live_ida_occurrences INTEGER NOT NULL,
                tooling_occurrences INTEGER NOT NULL,
                artifact_occurrences INTEGER NOT NULL,
                files_json TEXT NOT NULL,
                addresses_json TEXT NOT NULL
            );
            """
        )

        conn.executemany(
            """
            INSERT INTO findings (
                symbol, symbol_kind, source_kind, context_kind, file_path,
                line_number, line_text, occurrence_count, address, size
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            [
                (
                    finding["symbol"],
                    finding["symbol_kind"],
                    finding["source_kind"],
                    finding["context_kind"],
                    finding["file_path"],
                    finding["line_number"],
                    finding["line_text"],
                    finding["occurrence_count"],
                    finding["address"],
                    finding["size"],
                )
                for finding in findings
            ],
        )

        conn.executemany(
            """
            INSERT INTO symbols (
                symbol, symbol_kind, total_occurrences, repo_occurrences,
                active_code_occurrences, disabled_reference_occurrences,
                snapshot_occurrences, live_ida_occurrences, tooling_occurrences,
                artifact_occurrences, files_json, addresses_json
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            [
                (
                    symbol["symbol"],
                    symbol["symbol_kind"],
                    symbol["total_occurrences"],
                    symbol["repo_occurrences"],
                    symbol["active_code_occurrences"],
                    symbol["disabled_reference_occurrences"],
                    symbol["snapshot_occurrences"],
                    symbol["live_ida_occurrences"],
                    symbol["tooling_occurrences"],
                    symbol["artifact_occurrences"],
                    json.dumps(symbol["files"]),
                    json.dumps(symbol["addresses"]),
                )
                for symbol in symbols
            ],
        )


def summarize(findings: list[dict[str, Any]], symbols: list[dict[str, Any]]) -> str:
    by_kind: dict[str, int] = defaultdict(int)
    by_source: dict[str, int] = defaultdict(int)
    for finding in findings:
        by_kind[finding["symbol_kind"]] += int(finding["occurrence_count"])
        by_source[finding["source_kind"]] += int(finding["occurrence_count"])

    lines = [
        f"Distinct symbols: {len(symbols)}",
        f"Total occurrences: {sum(symbol['total_occurrences'] for symbol in symbols)}",
    ]
    for kind in sorted(by_kind):
        lines.append(f"  {kind}: {by_kind[kind]}")
    for source in sorted(by_source):
        lines.append(f"  {source}: {by_source[source]}")
    lines.append("Top symbols:")
    for symbol in symbols[:10]:
        lines.append(
            f"  {symbol['symbol']} ({symbol['symbol_kind']}): "
            f"{symbol['total_occurrences']} occurrences, "
            f"{len(symbol['files'])} files"
        )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Find raw decompiler names still present in the Cataclysm tree.")
    parser.add_argument(
        "--prefix",
        action="append",
        choices=sorted(DEFAULT_PREFIX_KINDS),
        help="Restrict the scan to one or more raw-name prefixes.",
    )
    parser.add_argument(
        "--scan-path",
        action="append",
        default=[],
        help="Additional repo-relative file or directory to scan as text.",
    )
    parser.add_argument("--include-mcp", action="store_true", help="Also scan live IDA globals/functions/structs via MCP.")
    parser.add_argument(
        "--include-noncompiled-source",
        action="store_true",
        help="Include .c/.cpp files not compiled by Cataclysm.vcxproj.",
    )
    parser.add_argument("--mcp-page-size", type=int, default=500, help="Page size for list_functions/list_globals MCP calls.")
    parser.add_argument("--json-out", help="Write the full machine-readable report to this path.")
    parser.add_argument("--sqlite-out", help="Write findings and symbol summaries to this SQLite database.")
    args = parser.parse_args()

    specs = build_prefix_specs(args.prefix)
    symbol_re = build_symbol_regex(specs)
    scan_paths = list(DEFAULT_SCAN_PATHS) + list(args.scan_path)
    compiled_source_paths = load_compiled_source_paths(VCXPROJ_PATH)

    findings: list[dict[str, Any]] = []
    findings.extend(scan_snapshot(REPO_ROOT / ".ida-snapshot.json", symbol_re, specs))
    for path in iter_text_files(scan_paths, compiled_source_paths, args.include_noncompiled_source):
        findings.extend(scan_text_file(path, symbol_re, specs))
    if args.include_mcp:
        findings.extend(scan_live_ida(symbol_re, specs, args.mcp_page_size))

    findings.sort(
        key=lambda item: (
            item["symbol"],
            item["source_kind"],
            item["file_path"],
            item["line_number"] or 0,
            item["address"] or "",
        )
    )
    symbols = aggregate_symbols(findings)

    report = {
        "generated_at": utc_now(),
        "repo_root": str(REPO_ROOT),
        "vcxproj_path": str(VCXPROJ_PATH),
        "compiled_source_filter": not args.include_noncompiled_source,
        "prefixes": [{"prefix": spec.prefix, "symbol_kind": spec.symbol_kind} for spec in specs],
        "findings": findings,
        "symbols": symbols,
    }

    if args.json_out:
        json_path = Path(args.json_out)
        if not json_path.is_absolute():
            json_path = REPO_ROOT / json_path
        json_path.parent.mkdir(parents=True, exist_ok=True)
        json_path.write_text(json.dumps(report, indent=2), encoding="utf-8", newline="\n")

    if args.sqlite_out:
        sqlite_path = Path(args.sqlite_out)
        if not sqlite_path.is_absolute():
            sqlite_path = REPO_ROOT / sqlite_path
        write_sqlite(sqlite_path, findings, symbols)

    print(summarize(findings, symbols))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
