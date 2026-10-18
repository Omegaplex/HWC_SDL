#!/usr/bin/env python3
"""
Rank Cat3 pseudocode blocks by file.

By default this scans for unresolved Cat3-style pseudocode blocks only:

    #if 0  // raw IDA pseudocode - needs cleanup before compiling
    #if 0  // Decompiled pseudocode — raw IDA symbols need proper names

Reference-only blocks such as:

    #if 0  // IDA pseudocode reference

are counted by default so the report reflects the full pseudocode backlog. Use
--only-unresolved to limit the report to the raw/non-reference cleanup queue.

Each matching block is counted once for its containing file. Files are then ranked by
fix priority using a weighted score:

* raw pseudocode blocks: 100 points each
* symbol/name-cleanup pseudocode blocks: 20 points each
* other pseudocode blocks: 5 points each
* reference-only pseudocode blocks: 1 point each

Usage:
    python scripts/rank_cat3_pseudocode.py
    python scripts/rank_cat3_pseudocode.py src/Game src/Win32/Auth1.cpp
    python scripts/rank_cat3_pseudocode.py --lines
    python scripts/rank_cat3_pseudocode.py --json
    python scripts/rank_cat3_pseudocode.py --only-unresolved
"""

from __future__ import annotations

import argparse
import fnmatch
import json
import sys
from dataclasses import dataclass
from pathlib import Path
import re

SOURCE_EXTENSIONS = {".c", ".cpp", ".h", ".hpp"}

# Directories (matched against each path component) to exclude from scanning.
# Supports glob-style wildcards via fnmatch.
IGNORED_DIR_PATTERNS = [
    "JPG*",          # third-party JPEG library — real source available
    "titannet*",     # Titan network SDK stubs — not part of this reconciliation pass
    "TitanInterface*",
    "bmp.c",         # hybrid game+ImageMagick file — ImageMagick pseudocode blocks are 3rd-party library
]

PSEUDOCODE_START_RE = re.compile(
    r"^\s*#\s*if\s+0\b(?P<comment>[^\r\n]*)$",
    re.IGNORECASE,
)
PP_IF_RE = re.compile(r"^\s*#\s*(?:if|ifdef|ifndef)\b")
PP_ENDIF_RE = re.compile(r"^\s*#\s*endif\b")

PRIORITY_WEIGHTS = {
    "raw": 100,
    "symbols": 20,
    "other": 5,
    "reference": 1,
}


@dataclass(frozen=True)
class BlockFinding:
    line: int
    end_line: int
    category: str
    comment: str


@dataclass
class FileSummary:
    path: Path
    findings: list[BlockFinding]

    @property
    def counts(self) -> dict[str, int]:
        counts = {key: 0 for key in PRIORITY_WEIGHTS}
        for finding in self.findings:
            counts[finding.category] += 1
        return counts

    @property
    def total(self) -> int:
        return len(self.findings)

    @property
    def score(self) -> int:
        counts = self.counts
        return sum(counts[key] * weight for key, weight in PRIORITY_WEIGHTS.items())


def normalize_comment(comment: str) -> str:
    return (
        comment.casefold()
        .replace("—", "-")
        .replace("–", "-")
        .replace("\t", " ")
        .strip()
    )


def classify_comment(comment: str, include_reference: bool) -> str | None:
    normalized = normalize_comment(comment)
    if "pseudocode" not in normalized:
        return None
    if "raw ida pseudocode" in normalized:
        return "raw"
    if "raw ida symbols" in normalized or "need proper names" in normalized:
        return "symbols"
    if not include_reference:
        return None
    if "pseudocode reference" in normalized:
        return "reference"
    return "other"


def find_matching_endif(lines: list[str], start_index: int) -> int:
    depth = 1
    for index in range(start_index + 1, len(lines)):
        line = lines[index]
        if PP_IF_RE.match(line):
            depth += 1
        elif PP_ENDIF_RE.match(line):
            depth -= 1
            if depth == 0:
                return index
    return start_index


def find_blocks(path: Path, include_reference: bool) -> list[BlockFinding]:
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines(keepends=True)
    except OSError as exc:
        print(f"warning: cannot read {path}: {exc}", file=sys.stderr)
        return []

    findings: list[BlockFinding] = []
    index = 0
    while index < len(lines):
        match = PSEUDOCODE_START_RE.match(lines[index])
        if not match:
            index += 1
            continue

        comment = match.group("comment") or ""
        category = classify_comment(comment, include_reference)
        if category is None:
            index += 1
            continue

        end_index = find_matching_endif(lines, index)
        findings.append(
            BlockFinding(
                line=index + 1,
                end_line=end_index + 1,
                category=category,
                comment=comment.strip(),
            )
        )
        index = end_index + 1

    return findings


def iter_source_files(paths: list[Path]) -> list[Path]:
    seen: set[Path] = set()
    files: list[Path] = []
    for raw_path in paths:
        path = raw_path.resolve()
        if not path.exists():
            print(f"warning: path does not exist: {raw_path}", file=sys.stderr)
            continue
        if path.is_file():
            if path.suffix.lower() in SOURCE_EXTENSIONS and path not in seen:
                if not any(
                    fnmatch.fnmatch(part, pat)
                    for part in path.parts
                    for pat in IGNORED_DIR_PATTERNS
                ):
                    seen.add(path)
                    files.append(path)
            continue
        for candidate in sorted(path.rglob("*")):
            if not candidate.is_file():
                continue
            if candidate.suffix.lower() not in SOURCE_EXTENSIONS:
                continue
            if any(
                fnmatch.fnmatch(part, pat)
                for part in candidate.parts
                for pat in IGNORED_DIR_PATTERNS
            ):
                continue
            if candidate not in seen:
                seen.add(candidate)
                files.append(candidate)
    return files


def format_path(path: Path, root: Path) -> str:
    try:
        relative = path.relative_to(root)
    except ValueError:
        relative = path
    return str(relative).replace("/", "\\")


def summarize(paths: list[Path], include_reference: bool) -> tuple[list[FileSummary], int]:
    summaries: list[FileSummary] = []
    total_blocks = 0
    for path in paths:
        findings = find_blocks(path, include_reference)
        if not findings:
            continue
        summaries.append(FileSummary(path=path, findings=findings))
        total_blocks += len(findings)

    summaries.sort(
        key=lambda summary: (
            -summary.score,
            -summary.counts["raw"],
            -summary.counts["symbols"],
            -summary.total,
            format_path(summary.path, Path.cwd()).lower(),
        )
    )
    return summaries, total_blocks


def print_table(
    summaries: list[FileSummary],
    root: Path,
    show_lines: bool,
    top: int,
    include_reference: bool,
) -> None:
    selected = summaries[:top] if top > 0 else summaries
    if include_reference:
        header = f"{'Rank':>4} {'Score':>5} {'Raw':>3} {'Sym':>3} {'Oth':>3} {'Ref':>3} {'Tot':>3} File"
    else:
        header = f"{'Rank':>4} {'Score':>5} {'Raw':>3} {'Sym':>3} {'Tot':>3} File"
    print(header)
    print("-" * len(header))
    for index, summary in enumerate(selected, start=1):
        counts = summary.counts
        if include_reference:
            print(
                f"{index:>4} {summary.score:>5} "
                f"{counts['raw']:>3} {counts['symbols']:>3} {counts['other']:>3} {counts['reference']:>3} "
                f"{summary.total:>3} {format_path(summary.path, root)}"
            )
        else:
            print(
                f"{index:>4} {summary.score:>5} "
                f"{counts['raw']:>3} {counts['symbols']:>3} {summary.total:>3} {format_path(summary.path, root)}"
            )
        if show_lines:
            lines = ", ".join(f"L{finding.line}" for finding in summary.findings)
            print(f"                 lines: {lines}")


def print_json(summaries: list[FileSummary], root: Path, top: int) -> None:
    selected = summaries[:top] if top > 0 else summaries
    payload = []
    for index, summary in enumerate(selected, start=1):
        counts = summary.counts
        payload.append(
            {
                "rank": index,
                "file": format_path(summary.path, root),
                "score": summary.score,
                "counts": counts,
                "total": summary.total,
                "blocks": [
                    {
                        "line": finding.line,
                        "end_line": finding.end_line,
                        "category": finding.category,
                        "comment": finding.comment,
                    }
                    for finding in summary.findings
                ],
            }
        )
    json.dump(payload, sys.stdout, indent=2)
    sys.stdout.write("\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Count Cat3 pseudocode blocks per file and rank fix priority."
    )
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="Files or directories to scan. Defaults to src\\.",
    )
    parser.add_argument(
        "--lines",
        action="store_true",
        help="Show the starting line for each pseudocode block.",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON instead of a table.",
    )
    parser.add_argument(
        "--top",
        type=int,
        default=0,
        help="Limit output to the top N ranked files.",
    )
    parser.add_argument(
        "--only-unresolved",
        action="store_true",
        help="Exclude lower-priority reference-only pseudocode blocks.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = Path.cwd().resolve()
    scan_roots = args.paths if args.paths else [root / "src"]
    files = iter_source_files(scan_roots)
    if not files:
        print("No source files to scan.")
        return 0

    include_reference = not args.only_unresolved
    summaries, total_blocks = summarize(files, include_reference)
    if args.json:
        print_json(summaries, root, args.top)
        return 0

    print("Priority score: raw=100, symbol/name cleanup=20, other=5, reference=1")
    print(
        f"Scanned {len(files)} source files. Found {total_blocks} Cat3 pseudocode block(s) in {len(summaries)} file(s).\n"
    )

    if not summaries:
        print("No Cat3 pseudocode blocks found.")
        return 0

    print_table(summaries, root, args.lines, args.top, include_reference)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
