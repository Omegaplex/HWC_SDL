#!/usr/bin/env python3
"""
Generate offset/size static_assert lines for a struct using a real MSVC layout probe.

Unlike comment-based generators, this script compiles a tiny C probe with the same
include paths, defines, and forced include used by the Cataclysm Win32 build. That
lets offsetof()/sizeof() report the authoritative layout even when the header has
anonymous structs/unions, typedef indirection, or no inline offset comments.

Usage:
    python scripts/generate_struct_static_asserts.py src\\Game\\spaceobj.h AcolyteSpecifics

Example output:
    static_assert(offsetof(AcolyteSpecifics, relinkDelayTimer) == 0x0, "AcolyteSpecifics relinkDelayTimer offset mismatch");
    ...
    static_assert(sizeof(AcolyteSpecifics) == 0x5C, "AcolyteSpecifics size mismatch");
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
import textwrap
import uuid
from dataclasses import dataclass
from pathlib import Path

STRUCT_START_RE = re.compile(r"^\s*typedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*$")
TYPEDEF_END_RE = re.compile(r"^\s*}\s*(?P<name>[A-Za-z_]\w*)\s*;\s*$")
FIELD_RE = re.compile(
    r"""
    ^\s*
    (?P<prefix>.+?)
    \b(?P<name>[A-Za-z_]\w*)
    (?P<suffix>\s*(?:\[[^;\]]+\]\s*)*)
    ;
    \s*(?://.*)?$
    """,
    re.VERBOSE,
)

PROJECT_DEFINES = [
    "_CRT_SECURE_NO_WARNINGS",
    "_CRT_NONSTDC_NO_WARNINGS",
    "WIN32",
    "_DEBUG",
    "_WINDOWS",
    "HW_BUILD_FOR_DEBUGGING",
]

PROJECT_INCLUDE_DIRS = [
    r"src\Game",
    r"src\Win32",
    r"src\Ships",
    r"src\Generated",
    r"hw_rgl",
    r"hw_rgl\rgl_common",
]


@dataclass
class ProbeResult:
    fields: dict[str, int]
    size: int


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("header", type=Path, help="Header containing the target typedef struct")
    parser.add_argument("struct_name", help="Struct name to inspect")
    parser.add_argument(
        "--header-include",
        help="Header name/path to #include in the probe. Defaults to the basename of <header>.",
    )
    parser.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep generated probe files in temp\\ for debugging.",
    )
    return parser.parse_args()


def read_lines(path: Path) -> list[str]:
    try:
        return path.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        raise SystemExit(f"error: failed to read {path}: {exc}") from exc


def collect_struct_fields(lines: list[str], struct_name: str, header_path: Path) -> list[str]:
    in_typedef = False
    body_lines: list[str] = []

    for line in lines:
        if not in_typedef and STRUCT_START_RE.match(line):
            in_typedef = True
            body_lines.clear()
            continue

        if not in_typedef:
            continue

        end_match = TYPEDEF_END_RE.match(line)
        if end_match:
            if end_match.group("name") == struct_name:
                return parse_fields(body_lines, struct_name)
            in_typedef = False
            body_lines.clear()
            continue

        body_lines.append(line)

    raise SystemExit(f"error: typedef struct for {struct_name} not found in {header_path}")


def parse_fields(body_lines: list[str], struct_name: str) -> list[str]:
    fields: list[str] = []
    nesting = 0

    for raw_line in body_lines:
        line = raw_line.strip()
        if not line:
            continue
        if line == "{":
            continue
        if line.startswith("}"):
            nesting = max(0, nesting - raw_line.count("}"))
            continue
        if "{" in raw_line:
            nesting += raw_line.count("{")
            continue
        if nesting != 0:
            continue
        if line.startswith("/*") or line.startswith("//"):
            continue
        if line.startswith("static_assert"):
            continue
        if line.startswith("#"):
            continue

        field_match = FIELD_RE.match(raw_line)
        if field_match:
            fields.append(field_match.group("name"))

    if not fields:
        raise SystemExit(f"error: no direct fields found for {struct_name}")
    return fields


def find_vcvars32() -> Path:
    env_vswhere = os.environ.get("VSWHERE")
    candidates: list[Path] = []
    if env_vswhere:
        candidates.append(Path(env_vswhere))

    program_files_x86 = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))
    candidates.append(program_files_x86 / r"Microsoft Visual Studio\Installer\vswhere.exe")

    for vswhere in candidates:
        if not vswhere.exists():
            continue
        command = [
            str(vswhere),
            "-latest",
            "-products",
            "*",
            "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-find",
            r"VC\Auxiliary\Build\vcvars32.bat",
        ]
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode == 0:
            path_text = result.stdout.strip()
            if path_text:
                return Path(path_text)

    fallback_paths = [
        Path(r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"),
        Path(r"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat"),
        Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"),
        Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"),
        Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat"),
        Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"),
    ]
    for candidate in fallback_paths:
        if candidate.exists():
            return candidate

    raise SystemExit("error: could not locate vcvars32.bat")


def format_include(header_path: Path, header_include: str | None) -> str:
    if header_include:
        return header_include.replace("\\", "/")
    return header_path.name


def make_probe_source(struct_name: str, include_name: str, fields: list[str]) -> str:
    field_lines = "\n".join(
        f'    printf("FIELD {field} %zu\\n", offsetof({struct_name}, {field}));' for field in fields
    )
    return textwrap.dedent(
        f"""\
        #include <stddef.h>
        #include <stdio.h>
        #include "{include_name}"

        int main(void)
        {{
            printf("SIZE %zu\\n", sizeof({struct_name}));
        {field_lines}
            return 0;
        }}
        """
    )


def run_probe(repo_root: Path, vcvars32: Path, probe_c: Path, probe_exe: Path, probe_bat: Path) -> str:
    define_flags = " ".join(f'/D{define}' for define in PROJECT_DEFINES)
    include_flags = " ".join(f'/I"{repo_root / include_dir}"' for include_dir in PROJECT_INCLUDE_DIRS)
    batch_text = (
        "@echo off\n"
        f'call "{vcvars32}" >nul || exit /b 1\n'
        f'cd /d "{repo_root}" || exit /b 1\n'
        f'cl /nologo /TC {define_flags} {include_flags} /FIcompat.h /Fe:"{probe_exe}" "{probe_c}" >nul || exit /b 1\n'
        f'"{probe_exe}"\n'
    )
    probe_bat.write_text(batch_text, encoding="ascii")
    result = subprocess.run(
        [os.environ.get("ComSpec", r"C:\Windows\System32\cmd.exe"), "/c", str(probe_bat)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        stderr = result.stderr.strip()
        stdout = result.stdout.strip()
        details = "\n".join(part for part in [stdout, stderr] if part)
        raise SystemExit(f"error: layout probe failed\n{details}")
    return result.stdout


def parse_probe_output(output: str, fields: list[str]) -> ProbeResult:
    size: int | None = None
    field_offsets: dict[str, int] = {}

    for raw_line in output.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) == 2 and parts[0] == "SIZE":
            size = int(parts[1])
        elif len(parts) == 3 and parts[0] == "FIELD":
            _, name, value = parts
            field_offsets[name] = int(value)

    missing = [field for field in fields if field not in field_offsets]
    if size is None or missing:
        problems = []
        if size is None:
            problems.append("size")
        if missing:
            problems.append("fields: " + ", ".join(missing))
        raise SystemExit("error: incomplete probe output (" + "; ".join(problems) + ")")

    return ProbeResult(fields=field_offsets, size=size)


def emit_asserts(struct_name: str, result: ProbeResult, fields: list[str]) -> None:
    for field in fields:
        offset = result.fields[field]
        print(
            f'static_assert(offsetof({struct_name}, {field}) == 0x{offset:X}, '
            f'"{struct_name} {field} offset mismatch");'
        )
    print(f'static_assert(sizeof({struct_name}) == 0x{result.size:X}, "{struct_name} size mismatch");')


def cleanup_temp(paths: list[Path]) -> None:
    for path in paths:
        try:
            if path.exists():
                path.unlink()
        except OSError:
            pass


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent.parent
    header_path = (repo_root / args.header).resolve() if not args.header.is_absolute() else args.header.resolve()
    lines = read_lines(header_path)
    fields = collect_struct_fields(lines, args.struct_name, header_path)
    include_name = format_include(header_path, args.header_include)
    vcvars32 = find_vcvars32()

    temp_dir = repo_root / "temp"
    temp_dir.mkdir(parents=True, exist_ok=True)
    token = uuid.uuid4().hex[:8]
    probe_c = temp_dir / f"layout_probe_{args.struct_name}_{token}.c"
    probe_exe = temp_dir / f"layout_probe_{args.struct_name}_{token}.exe"
    probe_obj = temp_dir / f"layout_probe_{args.struct_name}_{token}.obj"
    probe_bat = temp_dir / f"layout_probe_{args.struct_name}_{token}.bat"

    probe_c.write_text(make_probe_source(args.struct_name, include_name, fields), encoding="ascii")

    try:
        output = run_probe(repo_root, vcvars32, probe_c, probe_exe, probe_bat)
        result = parse_probe_output(output, fields)
        emit_asserts(args.struct_name, result, fields)
    finally:
        if not args.keep_temp:
            cleanup_temp([probe_c, probe_exe, probe_obj, probe_bat])

    return 0


if __name__ == "__main__":
    sys.exit(main())
