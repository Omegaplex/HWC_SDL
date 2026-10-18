"""
Audit KAS remap table coverage for one or more missions.

The script walks the mission callback graph from Init_MissionNN / Watch_MissionNN,
finds every function that can be installed into an AITeam watch slot through KAS
callback APIs, and compares that set against src/Game/KASRemap.c.

It can also resolve original cataclysm.exe offsets for missing entries from
.ida-snapshot.json or the live IDA MCP server and emit ready-to-paste table rows.

Usage:
    python src/Scripts/audit_kas_remap.py 1
    python src/Scripts/audit_kas_remap.py 1 2 --check
    python src/Scripts/audit_kas_remap.py 1 --emit-missing-rows --use-snapshot
    python src/Scripts/audit_kas_remap.py 1 --emit-missing-rows --live-ida
"""

from __future__ import annotations

import argparse
import json
import re
import sys
import time
from dataclasses import dataclass
from pathlib import Path

import requests

REPO_ROOT = Path(__file__).resolve().parents[2]
GAME_DIR = REPO_ROOT / "src" / "Game"
KAS_REMAP = GAME_DIR / "KASRemap.c"
SNAPSHOT = REPO_ROOT / ".ida-snapshot.json"
MCP_URL = "http://localhost:13337/mcp"

CALLBACK_CALLS = {
    "kasFSMCreate": (1, 2),
    "kasJump": (1, 2),
    "kasJumpTeam": (1, 2),
    "kasfSpeechEvent": (1, 2),
}
DIRECT_ASSIGN_FIELDS = ("kasFSMWatchFunction", "kasStateWatchFunction")
NON_CALLBACK_TOKENS = {"NULL", "0", "-1"}
TABLE_START_RE = re.compile(
    r"static\s+const\s+KASFuncOffsetEntry\s+kasOffsetTable_Mission(?P<mission>\d{2})\[\]\s*=\s*\{",
    re.MULTILINE,
)
TABLE_ROW_RE = re.compile(r"\{\s*(?P<offset>-?\d+|NOOP_OFFSET)\s*,\s*(?P<name>[A-Za-z_]\w*)\s*\}")


@dataclass
class FunctionDef:
    name: str
    signature: str
    body: str
    path: Path


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*?$", "", text, flags=re.MULTILINE)
    return text


def strip_preprocessor_disabled(text: str) -> str:
    return re.sub(r"#if\s+0.*?#endif", "", text, flags=re.DOTALL)


def find_matching(text: str, open_index: int, open_char: str, close_char: str) -> int | None:
    depth = 0
    in_string = False
    in_char = False
    escape = False
    for index in range(open_index, len(text)):
        ch = text[index]
        if escape:
            escape = False
            continue
        if ch == "\\" and (in_string or in_char):
            escape = True
            continue
        if ch == '"' and not in_char:
            in_string = not in_string
            continue
        if ch == "'" and not in_string:
            in_char = not in_char
            continue
        if in_string or in_char:
            continue
        if ch == open_char:
            depth += 1
        elif ch == close_char:
            depth -= 1
            if depth == 0:
                return index
    return None


def split_top_level_args(arg_text: str) -> list[str]:
    args: list[str] = []
    current: list[str] = []
    paren_depth = 0
    bracket_depth = 0
    brace_depth = 0
    in_string = False
    in_char = False
    escape = False

    for ch in arg_text:
        if escape:
            current.append(ch)
            escape = False
            continue
        if ch == "\\" and (in_string or in_char):
            current.append(ch)
            escape = True
            continue
        if ch == '"' and not in_char:
            current.append(ch)
            in_string = not in_string
            continue
        if ch == "'" and not in_string:
            current.append(ch)
            in_char = not in_char
            continue
        if in_string or in_char:
            current.append(ch)
            continue
        if ch == "(":
            paren_depth += 1
        elif ch == ")":
            paren_depth -= 1
        elif ch == "[":
            bracket_depth += 1
        elif ch == "]":
            bracket_depth -= 1
        elif ch == "{":
            brace_depth += 1
        elif ch == "}":
            brace_depth -= 1
        elif ch == "," and paren_depth == 0 and bracket_depth == 0 and brace_depth == 0:
            args.append("".join(current).strip())
            current = []
            continue
        current.append(ch)

    tail = "".join(current).strip()
    if tail:
        args.append(tail)
    return args


def strip_outer_parens(text: str) -> str:
    text = text.strip()
    while text.startswith("(") and text.endswith(")"):
        close_index = find_matching(text, 0, "(", ")")
        if close_index != len(text) - 1:
            break
        text = text[1:-1].strip()
    return text


def strip_leading_casts(text: str) -> str:
    text = text.strip()
    while text.startswith("("):
        close_index = find_matching(text, 0, "(", ")")
        if close_index is None:
            break
        cast = text[1:close_index].strip()
        if not cast:
            break
        if any(token in cast for token in ("+", "-", "/", "%", "&", "|", "?", ":")):
            break
        if re.search(r"[A-Za-z_]", cast) is None:
            break
        text = text[close_index + 1 :].strip()
    return text


def normalize_callback_expr(expr: str) -> str | None:
    expr = strip_outer_parens(expr)
    expr = strip_leading_casts(expr)
    expr = strip_outer_parens(expr)
    while expr.startswith("&"):
        expr = expr[1:].strip()
    expr = strip_outer_parens(expr)
    if not expr or expr in NON_CALLBACK_TOKENS:
        return None
    if re.fullmatch(r"[A-Za-z_]\w*", expr):
        return expr
    return None


def iter_named_calls(body: str, call_name: str):
    pattern = re.compile(rf"\b{re.escape(call_name)}\s*\(")
    start = 0
    while True:
        match = pattern.search(body, start)
        if not match:
            return
        open_index = body.find("(", match.start())
        close_index = find_matching(body, open_index, "(", ")")
        if close_index is None:
            return
        yield split_top_level_args(body[open_index + 1 : close_index])
        start = close_index + 1


def function_priority(defn: FunctionDef, mission: int, definition_mode: str) -> tuple[int, int]:
    path_name = defn.path.name.lower()
    mission_file = f"mission{mission:02d}.c"
    score = 0
    if definition_mode == "runtime":
        if path_name == "missions.c":
            score += 180
        elif path_name == "singleplayermissionscripts.c":
            score += 40
        elif path_name == "stubs.c":
            score -= 200
        elif path_name.startswith("mission") and path_name.endswith(".c"):
            score -= 40
        else:
            score += 120
        if body_looks_like_stub(defn.body):
            score -= 180
    elif definition_mode == "extracted":
        if path_name == mission_file:
            score += 100
        elif path_name.startswith("mission") and path_name.endswith(".c"):
            score += 80
        elif path_name == "singleplayermissionscripts.c":
            score += 20
        elif path_name == "missions.c":
            score += 10
        elif path_name == "stubs.c":
            score -= 100
    else:
        if path_name == mission_file:
            score += 100
        elif path_name.startswith("mission") and path_name.endswith(".c"):
            score += 80
        elif path_name == "singleplayermissionscripts.c":
            score += 20
        elif path_name == "missions.c":
            score += 10
        elif path_name == "stubs.c":
            score -= 100
    if not body_looks_like_stub(defn.body):
        score += 25
    current_mission_refs = len(re.findall(rf"\b(?:Init|Watch)_Mission{mission:02d}(?:_|\b)", defn.body))
    other_mission_refs = len(
        [
            match
            for match in re.findall(r"\b(?:Init|Watch)_Mission(\d{2})(?:_|\b)", defn.body)
            if int(match) != mission
        ]
    )
    score += current_mission_refs * 5
    score -= other_mission_refs * 5
    return score, len(defn.body)


def body_looks_like_stub(body: str) -> bool:
    compact = re.sub(r"\s+", "", strip_comments(body))
    if compact == "":
        return True
    if compact in {"return;", "return0;", "returnNULL;"}:
        return True
    if compact.startswith("dbgFatalf("):
        return True
    return False


def build_function_index() -> dict[str, list[FunctionDef]]:
    index: dict[str, list[FunctionDef]] = {}
    for path in sorted(GAME_DIR.glob("*.c")):
        text = path.read_text(encoding="utf-8", errors="replace")
        text = strip_preprocessor_disabled(strip_comments(text))
        for signature, name, body in iter_function_definitions(text):
            index.setdefault(name, []).append(FunctionDef(name=name, signature=signature, body=body, path=path))
    return index


def iter_function_definitions(text: str):
    brace_depth = 0
    last_boundary = 0
    in_string = False
    in_char = False
    escape = False
    index = 0

    while index < len(text):
        ch = text[index]
        if escape:
            escape = False
            index += 1
            continue
        if ch == "\\" and (in_string or in_char):
            escape = True
            index += 1
            continue
        if ch == '"' and not in_char:
            in_string = not in_string
            index += 1
            continue
        if ch == "'" and not in_string:
            in_char = not in_char
            index += 1
            continue
        if in_string or in_char:
            index += 1
            continue

        if ch == ";" and brace_depth == 0:
            last_boundary = index + 1
        elif ch == "{":
            if brace_depth == 0:
                signature = text[last_boundary:index].strip()
                name = extract_function_name(signature)
                if name is not None:
                    close_index = find_matching(text, index, "{", "}")
                    if close_index is None:
                        return
                    body = text[index + 1 : close_index]
                    yield signature, name, body
                    index = close_index + 1
                    last_boundary = index
                    continue
            brace_depth += 1
        elif ch == "}":
            brace_depth = max(0, brace_depth - 1)
            if brace_depth == 0:
                last_boundary = index + 1
        index += 1


def extract_function_name(signature: str) -> str | None:
    signature = signature.strip()
    if not signature or "=" in signature:
        return None
    if signature.startswith(("if ", "for ", "while ", "switch ", "else", "typedef ", "struct ", "enum ", "union ")):
        return None
    match = re.search(r"([A-Za-z_]\w*)\s*\([^;{}]*\)\s*$", signature, flags=re.DOTALL)
    if not match:
        return None
    return match.group(1)


def choose_definition(defs: list[FunctionDef], mission: int, definition_mode: str) -> FunctionDef:
    return max(defs, key=lambda defn: function_priority(defn, mission, definition_mode))


def extract_callback_refs(body: str) -> set[str]:
    refs: set[str] = set()

    for call_name, callback_indexes in CALLBACK_CALLS.items():
        for args in iter_named_calls(body, call_name):
            for index in callback_indexes:
                if index < len(args):
                    name = normalize_callback_expr(args[index])
                    if name:
                        refs.add(name)

    for field_name in DIRECT_ASSIGN_FIELDS:
        pattern = re.compile(rf"(?:->|\.){field_name}\s*=\s*([^;]+);")
        for match in pattern.finditer(body):
            name = normalize_callback_expr(match.group(1))
            if name:
                refs.add(name)

    return refs


def collect_reachable_callbacks(mission: int, definitions: dict[str, list[FunctionDef]], definition_mode: str):
    roots = [f"Init_Mission{mission:02d}", f"Watch_Mission{mission:02d}"]
    queue = list(roots)
    reachable: set[str] = set()
    missing_defs: set[str] = set()
    selected_defs: dict[str, FunctionDef] = {}

    while queue:
        name = queue.pop(0)
        if name in reachable:
            continue
        reachable.add(name)
        defs = definitions.get(name)
        if not defs:
            missing_defs.add(name)
            continue
        selected = choose_definition(defs, mission, definition_mode)
        selected_defs[name] = selected
        for ref in sorted(extract_callback_refs(selected.body)):
            if ref not in reachable:
                queue.append(ref)

    return roots, reachable, selected_defs, missing_defs


def parse_kas_remap_tables() -> dict[int, dict[str, str]]:
    text = KAS_REMAP.read_text(encoding="utf-8", errors="replace")
    tables: dict[int, dict[str, str]] = {}
    for match in TABLE_START_RE.finditer(text):
        mission = int(match.group("mission"))
        open_index = text.find("{", match.end() - 1)
        close_index = find_matching(text, open_index, "{", "}")
        if close_index is None:
            continue
        table_body = text[open_index + 1 : close_index]
        entries: dict[str, str] = {}
        for row in TABLE_ROW_RE.finditer(table_body):
            name = row.group("name")
            if name != "NULL":
                entries[name] = row.group("offset")
        tables[mission] = entries
    return tables


def signature_to_extern(signature: str) -> str:
    signature = re.sub(r"\bstatic\b", "", signature).strip()
    signature = re.sub(r"\s+", " ", signature)
    if not signature.startswith("extern "):
        signature = f"extern {signature}"
    return signature.rstrip(";") + ";"


def load_snapshot_addresses() -> dict[str, int]:
    if not SNAPSHOT.exists():
        raise FileNotFoundError(f"Snapshot not found: {SNAPSHOT}")
    data = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    return {entry["name"]: int(entry["address"], 16) for entry in data if "name" in entry and "address" in entry}


def mcp_call(method: str, params: dict, retries: int = 3):
    payload = {"jsonrpc": "2.0", "id": 1, "method": method, "params": params}
    for attempt in range(retries):
        try:
            response = requests.post(MCP_URL, json=payload, timeout=30)
            response.raise_for_status()
            data = response.json()
            if "error" in data:
                return None, data["error"]["message"]
            return data.get("result"), None
        except Exception as exc:
            if attempt == retries - 1:
                return None, str(exc)
            time.sleep(2)
    return None, "max retries exceeded"


def lookup_ida_addresses_live(names: set[str]) -> dict[str, int]:
    resolved: dict[str, int] = {}
    for name in sorted(names):
        result, err = mcp_call("get_function_by_name", {"name": name})
        if err or not result or "address" not in result:
            continue
        address = result["address"]
        if isinstance(address, str):
            resolved[name] = int(address, 16)
        else:
            resolved[name] = int(address)
    return resolved


def resolve_offsets(
    mission: int,
    names: set[str],
    use_snapshot: bool,
    live_ida: bool,
) -> tuple[dict[str, int], list[str]]:
    resolved: dict[str, int] = {}
    unresolved_sources: list[str] = []
    anchor_name = f"Watch_Mission{mission:02d}"
    addresses: dict[str, int] = {}

    if use_snapshot:
        try:
            addresses.update(load_snapshot_addresses())
        except FileNotFoundError as exc:
            unresolved_sources.append(str(exc))

    needed = set(names) | {anchor_name}
    unresolved = {name for name in needed if name not in addresses}

    if live_ida and unresolved:
        addresses.update(lookup_ida_addresses_live(unresolved))

    anchor_address = addresses.get(anchor_name)
    if anchor_address is None:
        unresolved_sources.append(f"Could not resolve IDA anchor {anchor_name}")
        return resolved, unresolved_sources

    for name in sorted(names):
        address = addresses.get(name)
        if address is None:
            continue
        resolved[name] = address - anchor_address

    return resolved, unresolved_sources


def format_offset(offset: int) -> str:
    return f"{offset:d}"


def audit_mission(
    mission: int,
    definitions: dict[str, list[FunctionDef]],
    remap_tables: dict[int, dict[str, str]],
    args: argparse.Namespace,
) -> int:
    roots, reachable, selected_defs, missing_defs = collect_reachable_callbacks(
        mission, definitions, args.definition_mode
    )
    remap_entries = remap_tables.get(mission, {})

    missing_from_table = sorted(name for name in reachable if name not in remap_entries)
    mapped_but_stub = sorted(
        name
        for name in reachable
        if name in remap_entries and name in selected_defs and body_looks_like_stub(selected_defs[name].body)
    )

    print(f"Mission {mission:02d}")
    print(f"  Roots: {', '.join(roots)}")
    print(f"  Reachable callbacks: {len(reachable)}")
    print(f"  Table entries present: {len(remap_entries)}")
    print(f"  Missing table entries: {len(missing_from_table)}")
    print(f"  Stub-backed mapped entries: {len(mapped_but_stub)}")
    print(f"  Missing source definitions: {len(missing_defs)}")

    if missing_from_table:
        print("  Missing from table:")
        for name in missing_from_table:
            suffix = ""
            if name in selected_defs:
                suffix = f"  [{selected_defs[name].path.name}]"
            elif name in missing_defs:
                suffix = "  [no source definition]"
            print(f"    - {name}{suffix}")

    if mapped_but_stub:
        print("  Mapped but currently stub-backed:")
        for name in mapped_but_stub:
            print(f"    - {name}  [{selected_defs[name].path.name}]")

    if missing_defs:
        unresolved_defs = sorted(name for name in missing_defs if name not in missing_from_table)
        if unresolved_defs:
            print("  Reachable callbacks with no source definition:")
            for name in unresolved_defs:
                print(f"    - {name}")

    if args.emit_missing_rows and missing_from_table:
        offsets, offset_errors = resolve_offsets(
            mission,
            set(missing_from_table),
            use_snapshot=args.use_snapshot,
            live_ida=args.live_ida,
        )
        for message in offset_errors:
            print(f"  IDA note: {message}")

        if offsets:
            print("  Suggested externs:")
            for name in sorted(offsets, key=lambda item: offsets[item]):
                if name in selected_defs:
                    print(f"    {signature_to_extern(selected_defs[name].signature)}")
                else:
                    print(f"    extern void {name}(void);")

            print("  Suggested table rows:")
            for name in sorted(offsets, key=lambda item: offsets[item]):
                print(f"    {{ {format_offset(offsets[name]):>7}, {name} }},")

            unresolved_offsets = sorted(name for name in missing_from_table if name not in offsets)
            if unresolved_offsets:
                print("  Missing rows still needing IDA resolution:")
                for name in unresolved_offsets:
                    print(f"    - {name}")
        else:
            print("  No offsets resolved for missing entries.")

    print()

    if args.check and (missing_from_table or mapped_but_stub or missing_defs):
        return 1
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("missions", nargs="+", type=int, help="Mission numbers to audit, e.g. 1 2")
    parser.add_argument("--check", action="store_true", help="Exit non-zero when gaps remain")
    parser.add_argument(
        "--emit-missing-rows",
        action="store_true",
        help="Print externs and KASRemap.c rows for missing entries when IDA names resolve",
    )
    parser.add_argument(
        "--use-snapshot",
        action="store_true",
        help="Resolve offsets from .ida-snapshot.json",
    )
    parser.add_argument(
        "--live-ida",
        action="store_true",
        help="Resolve offsets through the live IDA MCP server",
    )
    parser.add_argument(
        "--definition-mode",
        choices=("runtime", "extracted", "mixed"),
        default="runtime",
        help="Choose whether duplicate function names prefer integrated runtime sources or extracted mission files",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    definitions = build_function_index()
    remap_tables = parse_kas_remap_tables()

    exit_code = 0
    for mission in args.missions:
        if mission < 1 or mission > 99:
            print(f"Invalid mission number: {mission}", file=sys.stderr)
            return 2
        exit_code |= audit_mission(mission, definitions, remap_tables, args)
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
