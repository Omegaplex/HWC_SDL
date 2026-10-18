r"""Audit C struct members against IDA struct metadata.

This script is meant to find places where the source headers still carry weak
typing or placeholder names while IDA already has stronger information.

It does not edit source. It:
  1. Parses typedef struct blocks from one or more headers.
  2. Builds a small probe with MSVC so C offsets/sizes come from the real
     compiled layout rather than text parsing assumptions.
  3. Queries IDA's struct metadata.
  4. Reports likely upgrade opportunities such as:
       - C unknown/padding member vs named IDA member at same offset
       - named IDA members landing inside C unk/pad byte arrays
       - C void * / raw byte arrays where IDA has a stronger type

Usage:
    python src\Scripts\audit_struct_type_gaps.py
    python src\Scripts\audit_struct_type_gaps.py --struct Ship ShipStaticInfo
    python src\Scripts\audit_struct_type_gaps.py --json-out .struct-gaps.json
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import requests


REPO_ROOT = Path(__file__).resolve().parents[2]
MCP_URL = "http://localhost:13337/mcp"
DEFAULT_HEADERS = [REPO_ROOT / "src" / "Game" / "spaceobj.h"]
DEFAULT_STRUCTS = ["Ship", "ShipStaticInfo"]
DEFAULT_ALIAS_MAP = {
    "ShipStaticInfo": "ShipStaticInfo2",
}
PRIMITIVE_IDA_TYPES = {
    "bool",
    "bool8",
    "bool32",
    "byte",
    "char",
    "int",
    "long",
    "real32",
    "sbyte",
    "sdword",
    "short",
    "ubyte",
    "udword",
    "uint",
    "ulong",
    "unsigned int",
    "unsigned long",
    "unsigned short",
    "uword",
    "void *",
}
UNKNOWN_NAME_RE = re.compile(
    r"^(?:unk\d*|unk_[0-9a-f]+|pad\d*|padding\d*|unknown\d*|unused\d*|spare\d*)$",
    re.IGNORECASE,
)
COMMENT_RE = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
TYPEDEF_STRUCT_RE = re.compile(r"typedef\s+struct(?:\s+[A-Za-z_]\w*)?\s*\{", re.MULTILINE)


@dataclass
class ParsedField:
    name: str
    declaration: str


@dataclass
class ParsedStruct:
    name: str
    header: Path
    fields: list[ParsedField]


@dataclass
class FieldLayout:
    name: str
    declaration: str
    offset: int
    size: int


def strip_comments(text: str) -> str:
    return COMMENT_RE.sub("", text)


def split_top_level(text: str, separator: str) -> list[str]:
    parts: list[str] = []
    start = 0
    brace = 0
    paren = 0
    bracket = 0
    for index, char in enumerate(text):
        if char == "{":
            brace += 1
        elif char == "}":
            brace -= 1
        elif char == "(":
            paren += 1
        elif char == ")":
            paren -= 1
        elif char == "[":
            bracket += 1
        elif char == "]":
            bracket -= 1
        elif char == separator and brace == 0 and paren == 0 and bracket == 0:
            chunk = text[start:index].strip()
            if chunk:
                parts.append(chunk)
            start = index + 1
    tail = text[start:].strip()
    if tail:
        parts.append(tail)
    return parts


def find_matching_brace(text: str, open_index: int) -> int:
    depth = 0
    for index in range(open_index, len(text)):
        char = text[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    raise ValueError("Unmatched brace in struct definition")


def extract_structs_from_header(header: Path) -> dict[str, ParsedStruct]:
    text = strip_comments(header.read_text(encoding="utf-8", errors="replace"))
    structs: dict[str, ParsedStruct] = {}
    for match in TYPEDEF_STRUCT_RE.finditer(text):
        open_brace = text.find("{", match.start())
        close_brace = find_matching_brace(text, open_brace)
        suffix = text[close_brace + 1 :]
        name_match = re.match(r"\s*([A-Za-z_]\w*)\s*;", suffix)
        if not name_match:
            continue
        struct_name = name_match.group(1)
        body = text[open_brace + 1 : close_brace]
        fields = parse_struct_fields(body)
        structs[struct_name] = ParsedStruct(struct_name, header, fields)
    return structs


def parse_struct_fields(body: str) -> list[ParsedField]:
    fields: list[ParsedField] = []
    for statement in split_top_level(body, ";"):
        statement = statement.strip()
        if not statement or statement.startswith("#"):
            continue

        anonymous_body = extract_anonymous_aggregate_body(statement)
        if anonymous_body is not None:
            fields.extend(parse_struct_fields(anonymous_body))
            continue

        for field_name in extract_declarator_names(statement):
            fields.append(ParsedField(field_name, normalize_space(statement)))
    return fields


def extract_anonymous_aggregate_body(statement: str) -> str | None:
    if not statement.startswith(("struct", "union")) or "{" not in statement:
        return None
    open_brace = statement.find("{")
    close_brace = find_matching_brace(statement, open_brace)
    tail = statement[close_brace + 1 :].strip()
    if tail:
        return None
    return statement[open_brace + 1 : close_brace]


def extract_declarator_names(statement: str) -> list[str]:
    names: list[str] = []
    for part in split_top_level(statement, ","):
        function_pointer = re.search(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", part)
        if function_pointer:
            names.append(function_pointer.group(1))
            continue

        part_without_arrays = re.sub(r"\[[^\]]*\]", "[]", part)
        part_without_bitfield = re.sub(r":[^,]+$", "", part_without_arrays)
        tokens = re.findall(r"[A-Za-z_]\w*", part_without_bitfield)
        if not tokens:
            continue
        names.append(tokens[-1])
    return names


def normalize_space(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def parse_aliases(values: list[str]) -> dict[str, str]:
    aliases = dict(DEFAULT_ALIAS_MAP)
    for value in values:
        if "=" not in value:
            raise ValueError(f"Invalid alias '{value}'; expected CName=IdaName")
        c_name, ida_name = value.split("=", 1)
        aliases[c_name.strip()] = ida_name.strip()
    return aliases


def find_vsdevcmd() -> Path | None:
    env_path = os.environ.get("VSDEVCMD")
    if env_path:
        candidate = Path(env_path)
        if candidate.exists():
            return candidate

    vswhere = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if vswhere.exists():
        try:
            result = subprocess.run(
                [
                    str(vswhere),
                    "-latest",
                    "-products",
                    "*",
                    "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-find",
                    r"Common7\Tools\VsDevCmd.bat",
                ],
                capture_output=True,
                text=True,
                check=True,
            )
            path = result.stdout.strip()
            if path:
                candidate = Path(path)
                if candidate.exists():
                    return candidate
        except Exception:
            pass

    base = Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Microsoft Visual Studio" / "2022"
    for edition in ("Enterprise", "Professional", "Community", "BuildTools"):
        candidate = base / edition / "Common7" / "Tools" / "VsDevCmd.bat"
        if candidate.exists():
            return candidate
    return None


def generate_probe_source(parsed_structs: list[ParsedStruct], output_path: Path) -> None:
    lines = [
        "#include <stddef.h>",
        "#include <stdio.h>",
    ]
    for header in sorted({struct_info.header for struct_info in parsed_structs}):
        lines.append(f'#include "{str(header)}"')
    lines.extend(
        [
            "",
            "int main(void)",
            "{",
        ]
    )
    for struct_info in parsed_structs:
        lines.append(f'    printf("STRUCT|{struct_info.name}|%zu\\n", sizeof({struct_info.name}));')
        for field in struct_info.fields:
            lines.append(
                f'    printf("FIELD|{struct_info.name}|{field.name}|%zu|%zu\\n", '
                f"offsetof({struct_info.name}, {field.name}), sizeof((({struct_info.name} *)0)->{field.name}));"
            )
    lines.append("    return 0;")
    lines.append("}")
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def compile_probe(source_path: Path, exe_path: Path) -> None:
    vsdevcmd = find_vsdevcmd()
    if vsdevcmd is None:
        raise RuntimeError("Could not locate VsDevCmd.bat")

    include_paths = [
        REPO_ROOT / "src" / "Game",
        REPO_ROOT / "src" / "Win32",
        REPO_ROOT / "src" / "Ships",
        REPO_ROOT / "src" / "Generated",
        REPO_ROOT / "hw_rgl",
        REPO_ROOT / "hw_rgl" / "rgl_common",
    ]
    include_args = " ".join(f'/I "{path}"' for path in include_paths)
    compile_command = (
        f'cl /nologo /TC /DWIN32 /D_DEBUG /D_WINDOWS /DHW_BUILD_FOR_DEBUGGING '
        f'{include_args} /FI compat.h "{source_path}" /Fe:"{exe_path}"'
    )
    batch_path = source_path.with_suffix(".bat")
    batch_path.write_text(
        "@echo off\n"
        f'call "{vsdevcmd}" -arch=x86 >nul\n'
        "if errorlevel 1 exit /b %errorlevel%\n"
        f"{compile_command}\n",
        encoding="utf-8",
    )
    result = subprocess.run(
        ["cmd.exe", "/d", "/c", str(batch_path)],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "Probe compile failed:\n"
            f"STDOUT:\n{result.stdout}\n"
            f"STDERR:\n{result.stderr}"
        )


def run_probe(exe_path: Path) -> dict[str, dict[str, FieldLayout] | int]:
    result = subprocess.run([str(exe_path)], capture_output=True, text=True, check=True)
    layouts: dict[str, dict[str, FieldLayout] | int] = {}
    for line in result.stdout.splitlines():
        parts = line.strip().split("|")
        if len(parts) == 3 and parts[0] == "STRUCT":
            layouts.setdefault(parts[1], {})
            layouts[f"{parts[1]}.__size__"] = int(parts[2])
        elif len(parts) == 5 and parts[0] == "FIELD":
            struct_name = parts[1]
            fields = layouts.setdefault(struct_name, {})
            assert isinstance(fields, dict)
            fields[parts[2]] = FieldLayout(
                name=parts[2],
                declaration="",
                offset=int(parts[3]),
                size=int(parts[4]),
            )
    return layouts


def mcp_call(method: str, params: dict, timeout: int = 60):
    response = requests.post(
        MCP_URL,
        json={"jsonrpc": "2.0", "id": 1, "method": method, "params": params},
        timeout=timeout,
    )
    response.raise_for_status()
    payload = response.json()
    if "error" in payload:
        raise RuntimeError(payload["error"]["message"])
    return payload.get("result")


def resolve_ida_struct_name(c_name: str, aliases: dict[str, str]) -> str | None:
    if c_name in aliases:
        return aliases[c_name]

    try:
        info = mcp_call("get_struct_info_simple", {"name": c_name})
        if info:
            return str(info["name"])
    except Exception:
        pass

    try:
        results = mcp_call("search_structures", {"filter": c_name}) or []
    except Exception:
        return None

    lowered = c_name.lower()
    exact = [entry["name"] for entry in results if str(entry["name"]).lower() == lowered]
    if len(exact) == 1:
        return str(exact[0])

    suffixed = [
        entry["name"]
        for entry in results
        if str(entry["name"]).lower() in {lowered + "2", lowered + "_t", lowered + "info2"}
    ]
    if len(suffixed) == 1:
        return str(suffixed[0])

    prefix = [entry["name"] for entry in results if str(entry["name"]).lower().startswith(lowered)]
    if len(prefix) == 1:
        return str(prefix[0])

    return None


def type_is_primitive(ida_type: str) -> bool:
    lowered = ida_type.strip().lower()
    if "[" in lowered and "]" in lowered:
        base = lowered.split("[", 1)[0].strip()
        return base in PRIMITIVE_IDA_TYPES
    return lowered in PRIMITIVE_IDA_TYPES


def c_decl_is_generic(field: FieldLayout) -> bool:
    decl = normalize_space(field.declaration).lower()
    if "(*" in decl:
        return False
    if "void *" in decl or "void*" in decl:
        return True
    if re.search(r"\b(?:ubyte|byte)\b\s+\w+\s*\[", decl):
        return True
    if UNKNOWN_NAME_RE.match(field.name):
        return True
    return False


def same_name(c_name: str, ida_name: str) -> bool:
    return c_name.lower() == ida_name.lower()


def attach_declarations(
    parsed_structs: dict[str, ParsedStruct],
    probe_layouts: dict[str, dict[str, FieldLayout] | int],
) -> dict[str, dict[str, FieldLayout]]:
    merged: dict[str, dict[str, FieldLayout]] = {}
    for struct_name, struct_info in parsed_structs.items():
        probe_fields = probe_layouts.get(struct_name)
        if not isinstance(probe_fields, dict):
            continue
        with_decls: dict[str, FieldLayout] = {}
        declaration_by_name = {field.name: field.declaration for field in struct_info.fields}
        for field_name, layout in probe_fields.items():
            with_decls[field_name] = FieldLayout(
                name=layout.name,
                declaration=declaration_by_name.get(field_name, ""),
                offset=layout.offset,
                size=layout.size,
            )
        merged[struct_name] = with_decls
    return merged


def compare_struct(
    c_name: str,
    ida_name: str,
    c_fields: dict[str, FieldLayout],
    c_size: int,
    ida_info: dict,
) -> dict:
    issues: list[dict] = []
    c_by_offset = {field.offset: field for field in c_fields.values()}
    c_ranges = sorted(c_fields.values(), key=lambda field: (field.offset, field.name))

    for ida_member in ida_info["members"]:
        ida_offset = int(ida_member["offset"])
        ida_size = int(ida_member["size"])
        ida_field_name = str(ida_member["name"])
        ida_field_type = str(ida_member["type"])
        c_field = c_by_offset.get(ida_offset)

        if c_field is not None:
            if UNKNOWN_NAME_RE.match(c_field.name) and not UNKNOWN_NAME_RE.match(ida_field_name):
                issues.append(
                    {
                        "kind": "semantic_name_gap",
                        "offset": ida_offset,
                        "c_field": c_field.name,
                        "c_decl": c_field.declaration,
                        "ida_field": ida_field_name,
                        "ida_type": ida_field_type,
                    }
                )
            elif c_decl_is_generic(c_field) and not type_is_primitive(ida_field_type) and not same_name(c_field.name, ida_field_name):
                issues.append(
                    {
                        "kind": "stronger_ida_type",
                        "offset": ida_offset,
                        "c_field": c_field.name,
                        "c_decl": c_field.declaration,
                        "ida_field": ida_field_name,
                        "ida_type": ida_field_type,
                    }
                )
            elif c_decl_is_generic(c_field) and not type_is_primitive(ida_field_type):
                issues.append(
                    {
                        "kind": "stronger_ida_type",
                        "offset": ida_offset,
                        "c_field": c_field.name,
                        "c_decl": c_field.declaration,
                        "ida_field": ida_field_name,
                        "ida_type": ida_field_type,
                    }
                )
            continue

        covering = next(
            (
                field
                for field in c_ranges
                if field.offset < ida_offset < field.offset + field.size
                or (field.offset == ida_offset and field.size > ida_size and c_decl_is_generic(field))
            ),
            None,
        )
        if covering is not None and c_decl_is_generic(covering) and not UNKNOWN_NAME_RE.match(ida_field_name):
            issues.append(
                {
                    "kind": "ida_inside_unknown_c_range",
                    "offset": ida_offset,
                    "c_field": covering.name,
                    "c_decl": covering.declaration,
                    "c_range": [covering.offset, covering.offset + covering.size],
                    "ida_field": ida_field_name,
                    "ida_type": ida_field_type,
                    "ida_size": ida_size,
                }
            )

    issues.sort(key=lambda issue: (issue["offset"], issue["kind"], issue.get("ida_field", "")))
    return {
        "c_struct": c_name,
        "ida_struct": ida_name,
        "c_size": c_size,
        "ida_size": int(ida_info["size"]),
        "issue_count": len(issues),
        "issues": issues,
    }


def print_report(report: dict, limit: int) -> None:
    unresolved = report.get("unresolved_structs", [])
    if unresolved:
        print("Unresolved IDA structs:")
        for entry in unresolved:
            print(f"  {entry['c_struct']}: {entry['reason']}")
        print()

    for struct_report in report["structs"]:
        print(
            f"{struct_report['c_struct']} -> {struct_report['ida_struct']}: "
            f"{struct_report['issue_count']} issue(s)"
        )
        for issue in struct_report["issues"][:limit]:
            summary = format_issue(issue)
            print(f"  0x{issue['offset']:03X}: {issue['kind']}: {summary}")
        hidden = len(struct_report["issues"]) - min(len(struct_report["issues"]), limit)
        if hidden > 0:
            print(f"  ... {hidden} more")
        print()


def format_issue(issue: dict) -> str:
    if issue["kind"] == "semantic_name_gap":
        return f"C {issue['c_field']} ({issue['c_decl']}) vs IDA {issue['ida_field']} : {issue['ida_type']}"
    if issue["kind"] == "stronger_ida_type":
        return f"C {issue['c_field']} ({issue['c_decl']}) vs IDA {issue['ida_field']} : {issue['ida_type']}"
    if issue["kind"] == "ida_inside_unknown_c_range":
        start, end = issue["c_range"]
        return (
            f"IDA {issue['ida_field']} : {issue['ida_type']} lands inside "
            f"C {issue['c_field']} ({issue['c_decl']}) range 0x{start:X}-0x{end:X}"
        )
    return json.dumps(issue, sort_keys=True)


def select_structs(
    all_structs: dict[str, ParsedStruct],
    selected_names: Iterable[str] | None,
    all_parsed: bool,
) -> dict[str, ParsedStruct]:
    if all_parsed:
        return dict(sorted(all_structs.items()))

    names = list(selected_names) if selected_names else list(DEFAULT_STRUCTS)
    selected: dict[str, ParsedStruct] = {}
    missing = [name for name in names if name not in all_structs]
    if missing:
        raise KeyError(f"Structs not found in parsed headers: {', '.join(missing)}")
    for name in names:
        selected[name] = all_structs[name]
    return selected


def main() -> int:
    parser = argparse.ArgumentParser(description="Audit C structs against IDA struct metadata")
    parser.add_argument(
        "--header",
        action="append",
        default=[],
        help="Header to parse (default: src\\Game\\spaceobj.h). Repeatable.",
    )
    parser.add_argument("--struct", nargs="+", help="Specific C structs to audit")
    parser.add_argument("--all-parsed", action="store_true", help="Audit every parsed typedef struct")
    parser.add_argument(
        "--alias",
        action="append",
        default=[],
        help="Override struct name mapping as CName=IdaName. Repeatable.",
    )
    parser.add_argument("--json-out", help="Optional JSON report path")
    parser.add_argument("--limit", type=int, default=20, help="Max issues to print per struct")
    parser.add_argument("--keep-probe", action="store_true", help="Keep generated probe artifacts")
    args = parser.parse_args()

    headers = [Path(path) for path in args.header] if args.header else list(DEFAULT_HEADERS)
    aliases = parse_aliases(args.alias)

    parsed_structs: dict[str, ParsedStruct] = {}
    for header in headers:
        if not header.exists():
            raise FileNotFoundError(f"Header not found: {header}")
        parsed_structs.update(extract_structs_from_header(header))

    selected = select_structs(parsed_structs, args.struct, args.all_parsed)

    with tempfile.TemporaryDirectory(prefix="struct-gap-audit-") as temp_dir_name:
        temp_dir = Path(temp_dir_name)
        probe_c = temp_dir / "struct_gap_probe.c"
        probe_exe = temp_dir / "struct_gap_probe.exe"

        generate_probe_source(list(selected.values()), probe_c)
        compile_probe(probe_c, probe_exe)
        probe_layouts = run_probe(probe_exe)

        c_size_map = {
            struct_name: int(probe_layouts[f"{struct_name}.__size__"])
            for struct_name in selected
            if f"{struct_name}.__size__" in probe_layouts
        }
        merged_fields = attach_declarations(selected, probe_layouts)

        if args.keep_probe:
            kept_probe = REPO_ROOT / ".struct-gap-probe"
            kept_probe.mkdir(parents=True, exist_ok=True)
            probe_copy = kept_probe / "struct_gap_probe.c"
            exe_copy = kept_probe / "struct_gap_probe.exe"
            probe_copy.write_text(probe_c.read_text(encoding="utf-8"), encoding="utf-8")
            exe_copy.write_bytes(probe_exe.read_bytes())

    report = {"structs": [], "unresolved_structs": []}

    for c_name in selected:
        ida_name = resolve_ida_struct_name(c_name, aliases)
        if ida_name is None:
            report["unresolved_structs"].append(
                {"c_struct": c_name, "reason": "No matching IDA struct found"}
            )
            continue

        ida_info = mcp_call("get_struct_info_simple", {"name": ida_name})
        c_fields = merged_fields.get(c_name)
        c_size = c_size_map.get(c_name)
        if c_fields is None or c_size is None:
            report["unresolved_structs"].append(
                {"c_struct": c_name, "reason": "Probe output missing"}
            )
            continue

        report["structs"].append(compare_struct(c_name, ida_name, c_fields, c_size, ida_info))

    print_report(report, args.limit)

    if args.json_out:
        output_path = Path(args.json_out)
        output_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
