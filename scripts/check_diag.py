#!/usr/bin/env python3
"""
Identify temporary diagnostic logging in Cataclysm source files.

Patterns detected:

0. Preprocessor-guarded CATACLYSM_DIAG blocks (preferred convention):
    #undef CATACLYSM_DIAG
    #define CATACLYSM_DIAG 1
    #if !defined(HW_Release) && CATACLYSM_DIAG
    ...any code...
    #endif

    #else / #elif inside a CATACLYSM_DIAG block is treated as an error.
    Use the HW_Release guard (not #ifdef) so Release builds compile the tracing
    out even when a file enables CATACLYSM_DIAG locally.

1. Lines tagged with // DIAG (case-insensitive, anywhere in line):
    dbgMessagef("\\n" CATACLYSM_DIAG_LABEL "x=%d", x);  // DIAG

2. Comment-only lines starting with // DIAG: or // DIAGNOSTIC:
    // DIAG: tracing alliance setup

3. Multi-line blocks between // DIAG-BEGIN and // DIAG-END:
    // DIAG-BEGIN
    dbgMessagef("\\n" CATACLYSM_DIAG_LABEL "x=%d", x);
    // DIAG-END

4. Temporary log statements that explicitly identify themselves as diagnostic:
    dbgMessagef("\\n" CATACLYSM_DIAG_LABEL "...");
    dbgWarningf(DBG_Loc, CATACLYSM_DIAG_LABEL "...");
    logfileLogf("foo.log", CATACLYSM_DIAG_LABEL "...");
    fprintf(file, CATACLYSM_DIAG_LABEL "...");

Lines tagged with // diag-ok are always preserved.

Usage:
    python scripts/check_diag.py             # verify staged + modified files
    python scripts/check_diag.py --all       # verify all source files under src/
    python scripts/check_diag.py src/Foo.c   # verify specific files
"""

from __future__ import annotations

import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

SOURCE_EXTENSIONS = {".c", ".h", ".cpp", ".hpp"}
DIAG_OK_RE = re.compile(r"//\s*diag-ok\b", re.IGNORECASE)
DIAG_DEFINE_RE = re.compile(r"^\s*#\s*define\s+CATACLYSM_DIAG\s+[1-9]\d*\s*$")
DIAG_IF_RE = re.compile(r"^\s*#\s*if\b.*\bCATACLYSM_DIAG\b")
PP_IF_RE = re.compile(r"^\s*#\s*(?:if|ifdef|ifndef)\b")
PP_ELIF_ELSE_RE = re.compile(r"^\s*#\s*(?:elif|else)\b")
PP_ENDIF_RE = re.compile(r"^\s*#\s*endif\b")
DIAG_TAG_RE = re.compile(r"//\s*DIAG\b", re.IGNORECASE)
DIAG_BEGIN_RE = re.compile(r"//\s*DIAG-BEGIN\s*$", re.IGNORECASE)
DIAG_END_RE = re.compile(r"//\s*DIAG-END\s*$", re.IGNORECASE)
DIAG_COMMENT_RE = re.compile(r"^\s*//\s*DIAG(?:NOSTIC)?\s*:", re.IGNORECASE)
TRACE_LABEL_RE = re.compile(r"\[[^]\n\"]*Trace[^]\n\"]*\]", re.IGNORECASE)
DIAG_LABEL_RE = re.compile(r"CATACLYSM_DIAG_LABEL|CATACLYSM_DIAG|\bDIAG\b", re.IGNORECASE)
LOG_CALL_RE = re.compile(
    r"\b(?:dbgMessagef|dbgWarningf|dbgFatalf|logfileLogf|logfileLog|fprintf|printf)\s*\(",
    re.IGNORECASE,
)


@dataclass
class Finding:
    line: int
    kind: str
    detail: str


def is_source_file(path: Path) -> bool:
    return path.suffix.lower() in SOURCE_EXTENSIONS and path.exists()


def get_changed_files() -> list[Path]:
    files: set[Path] = set()
    commands = [
        ["git", "diff", "--cached", "--name-only", "--diff-filter=ACM"],
        ["git", "diff", "--name-only", "--diff-filter=ACM"],
    ]
    for command in commands:
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode == 0:
            files.update(Path(line) for line in result.stdout.splitlines() if line.strip())
    return sorted(path for path in files if is_source_file(path))


def statement_end(lines: list[str], start_index: int) -> int:
    paren_depth = 0
    saw_paren = False
    for index in range(start_index, len(lines)):
        for char in lines[index]:
            if char == "(":
                paren_depth += 1
                saw_paren = True
            elif char == ")":
                paren_depth -= 1
        if ";" in lines[index] and (not saw_paren or paren_depth <= 0):
            return index
    return min(len(lines) - 1, start_index)


def has_diag_marker(text: str) -> bool:
    return bool(DIAG_LABEL_RE.search(text) or TRACE_LABEL_RE.search(text))


def find_diag_blocks(lines: list[str]) -> list[Finding]:
    findings: list[Finding] = []
    index = 0
    while index < len(lines):
        line = lines[index]

        if DIAG_OK_RE.search(line):
            index += 1
            continue

        if DIAG_DEFINE_RE.match(line):
            findings.append(Finding(index + 1, "define", "temporary CATACLYSM_DIAG enable"))
            index += 1
            continue

        if DIAG_IF_RE.match(line):
            depth = 1
            inner = index + 1
            while inner < len(lines) and depth > 0:
                candidate = lines[inner]
                if PP_IF_RE.match(candidate):
                    depth += 1
                elif PP_ENDIF_RE.match(candidate):
                    depth -= 1
                elif depth == 1 and PP_ELIF_ELSE_RE.match(candidate):
                    findings.append(
                        Finding(index + 1, "diag-block", "#else/#elif inside CATACLYSM_DIAG guard")
                    )
                    break
                inner += 1

            findings.append(Finding(index + 1, "diag-block", "preprocessor-guarded diagnostic block"))
            index += 1
            continue

        if DIAG_BEGIN_RE.search(line.rstrip()):
            findings.append(Finding(index + 1, "diag-block", "DIAG-BEGIN/DIAG-END block"))
            index += 1
            continue

        if DIAG_COMMENT_RE.match(line):
            findings.append(Finding(index + 1, "diag-comment", "diagnostic comment marker"))
            index += 1
            continue

        stripped = line.rstrip()
        if DIAG_TAG_RE.search(stripped) and not DIAG_BEGIN_RE.search(stripped) and not DIAG_END_RE.search(stripped):
            findings.append(Finding(index + 1, "diag-tag", "inline // DIAG marker"))
            index += 1
            continue

        if LOG_CALL_RE.search(line):
            end_index = statement_end(lines, index)
            text = "".join(lines[index:end_index + 1])
            if has_diag_marker(text):
                findings.append(Finding(index + 1, "diag-log", "temporary diagnostic log statement"))
                index = end_index + 1
                continue

        index += 1

    return findings


def process_file(path: Path) -> list[Finding]:
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        print(f"warning: cannot read {path}: {exc}", file=sys.stderr)
        return []
    return find_diag_blocks(text.splitlines(keepends=True))


def gather_files(args: list[str]) -> list[Path]:
    if "--all" in args:
        filtered = [arg for arg in args if arg != "--all"]
        if filtered:
            print("error: --all cannot be combined with explicit paths", file=sys.stderr)
            sys.exit(2)
        return sorted(path for path in Path("src").rglob("*") if is_source_file(path))

    if args:
        return [Path(arg) for arg in args if is_source_file(Path(arg))]

    return get_changed_files()


def main() -> int:
    args = sys.argv[1:]
    if "--clean" in args or "--check" in args:
        print("error: check_diag.py is identify-only; use it without cleanup flags.", file=sys.stderr)
        return 2

    files = gather_files(args)
    if not files:
        print("No source files to process.")
        return 0

    total = 0
    flagged_files = 0
    for path in files:
        findings = process_file(path)
        if not findings:
            continue
        flagged_files += 1
        total += len(findings)
        print(f"{path}:")
        for finding in findings:
            print(f"  L{finding.line}: {finding.kind} - {finding.detail}")

    if total:
        print(f"\nTotal: {total} diagnostic item(s) detected in {flagged_files} file(s).")
        print("Verification only; delete the flagged tracing manually once it is no longer needed.")
        return 1

    print("No diagnostic logging found.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
