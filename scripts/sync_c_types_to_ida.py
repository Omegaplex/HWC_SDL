"""
sync_c_types_to_ida.py

Synchronize C type declarations from the Cataclysm source tree into IDA Local
Types. By default this script updates existing Local Types and imports any
missing extracted types.

Primary use:
    1. Open cataclysm.i64 in IDA.
    2. File -> Script file... -> scripts\\sync_c_types_to_ida.py

Optional command-line usage from IDA:
    idat.exe -A -S"scripts\\sync_c_types_to_ida.py --header src\\Game\\spaceobj.h --type ShipStaticInfo" cataclysm.exe

Notes:
    - By default this script updates existing Local Types in place and also
      imports missing extracted types. Pass --existing-only if you need the old
      conservative behavior for a particular run.
    - Do not delete and recreate existing Local Types in a live IDB. IDA binds
      many type references by ordinal, so deletion can orphan existing
      signatures and variable types as anonymous #ordinal references.
    - Type extraction prefers full struct/union/enum definitions over forward
      declarations and plain aliases.
    - Preprocessing is intentionally small and local: #include lines are
      ignored, object-like #defines are tracked, and simple #if/#ifdef/#ifndef
      blocks are evaluated. If you need build-specific guards, pass
      --define NAME=VALUE.
"""

from __future__ import annotations

import argparse
import dataclasses
import pathlib
import re
import sys
import warnings
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

try:
    import ida_kernwin
    import ida_typeinf
    import idc

    IN_IDA = True
except ImportError:
    ida_kernwin = None
    ida_typeinf = None
    idc = None
    IN_IDA = False


TYPE_START_RE = re.compile(r"^\s*(typedef|struct|union|enum)\b", re.S)
IDENT_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.S)
LINE_COMMENT_RE = re.compile(r"//.*?$", re.M)
DEFINED_CALL_RE = re.compile(r"defined\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)")
MACRO_CALL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(([^()]*)\)")
ARRAY_BOUNDS_RE = re.compile(r"\[([^\[\]]+)\]")
BITFIELD_BOUNDS_RE = re.compile(r"(:\s*)([A-Za-z_][A-Za-z0-9_]*)(\b)")
TAG_REF_RE = re.compile(r"\b(struct|union|enum)\s+([A-Za-z_][A-Za-z0-9_]*)\b")
FUNCTION_PTR_TYPEDEF_RE = re.compile(
    r"^\s*typedef\b.*?\(\s*(?:[A-Za-z_][A-Za-z0-9_]*\s+)*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)\s*\(",
    re.S,
)
WINDOWS_QUALIFIER_RE = re.compile(r"\b(?:FAR|far|NEAR|near|HUGE|huge)\b")
STATIC_ASSERT_TYPEDEF_RE = re.compile(r"^\s*typedef\s+char\s+_[A-Za-z0-9_]+\s*\[[^;\]]+\]\s*;\s*$", re.S)
SUFFIXED_ALIAS_RE_TEMPLATE = r"^{base}\d+$"
DEBUG_ENABLED = False
LOG_FILE_PATH: Optional[pathlib.Path] = None


def log(message: str) -> None:
    if IN_IDA:
        ida_kernwin.msg(message + "\n")
    else:
        print(message)
    if LOG_FILE_PATH is not None:
        with LOG_FILE_PATH.open("a", encoding="utf-8") as handle:
            handle.write(message)
            handle.write("\n")


def debug_log(message: str) -> None:
    if DEBUG_ENABLED:
        log(f"[debug] {message}")


@dataclasses.dataclass(frozen=True)
class DeclCandidate:
    header_path: pathlib.Path
    decl: str
    names: Tuple[str, ...]
    has_body: bool
    score: int
    order: int


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Sync C types into IDA Local Types, updating existing mappings and importing missing ones by default."
    )
    parser.add_argument(
        "--root",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parents[1],
        help="Repository root. Defaults to the parent of this script directory.",
    )
    parser.add_argument(
        "--header",
        action="append",
        default=[],
        help="File or directory path relative to repo root. May be passed multiple times.",
    )
    parser.add_argument(
        "--type",
        action="append",
        default=[],
        dest="type_names",
        help="Restrict sync to specific type names. May be passed multiple times.",
    )
    parser.add_argument(
        "--define",
        action="append",
        default=[],
        dest="defines",
        help="Predefine a simple preprocessor symbol, e.g. HW_BUILD_FOR_DEBUGGING=1.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report what would be updated without changing Local Types.",
    )
    parser.add_argument(
        "--scan-only",
        action="store_true",
        help="Only scan and report extracted declarations. Useful outside IDA.",
    )
    parser.add_argument(
        "--include-missing",
        action="store_true",
        help="Deprecated no-op kept for compatibility; missing extracted types are included by default.",
    )
    parser.add_argument(
        "--existing-only",
        action="store_true",
        help="Restrict sync to types that already exist in IDA Local Types.",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="Emit detailed extraction, dependency, and name-resolution diagnostics.",
    )
    parser.add_argument(
        "--log-file",
        type=pathlib.Path,
        help="Append log output to this file in addition to stdout/IDA output.",
    )
    return parser.parse_args(list(argv))


def iter_source_files(path: pathlib.Path) -> Iterable[pathlib.Path]:
    if path.is_file():
        if path.suffix.lower() in {".h", ".c"}:
            yield path
        return

    if not path.exists():
        return

    for child in sorted(path.rglob("*")):
        if child.is_file() and child.suffix.lower() in {".h", ".c"}:
            yield child


def repo_source_paths(root: pathlib.Path, header_args: Sequence[str]) -> List[pathlib.Path]:
    if header_args:
        files: List[pathlib.Path] = []
        for arg in header_args:
            files.extend(iter_source_files(root / arg))
        return files

    files: List[pathlib.Path] = []
    for rel_dir in ("src", "hw_rgl"):
        files.extend(iter_source_files(root / rel_dir))
    return files


def strip_comments(text: str) -> str:
    text = LINE_COMMENT_RE.sub("", text)
    # Remove // comments first so banner lines like //******* do not look like
    # a block comment starting at the second slash.
    text = BLOCK_COMMENT_RE.sub("", text)
    return text


def parse_define_value(value: str, macros: Dict[str, int]) -> Optional[int]:
    expr = value.strip()
    if not expr:
        return 1
    evaluated = eval_pp_value(expr, macros)
    return evaluated


def rewrite_pp_expr(expr: str, macros: Dict[str, int]) -> str:
    expr = DEFINED_CALL_RE.sub(lambda m: "1" if m.group(1) in macros else "0", expr)
    while True:
        rewritten = MACRO_CALL_RE.sub(lambda m: str(macros.get(m.group(1), 0)), expr)
        if rewritten == expr:
            break
        expr = rewritten
    expr = expr.replace("&&", " and ")
    expr = expr.replace("||", " or ")
    expr = re.sub(r"(?<![=!<>])!(?!=)", " not ", expr)

    def replace_token(match: re.Match[str]) -> str:
        token = match.group(0)
        if token in {"and", "or", "not"}:
            return token
        if token in macros:
            return str(macros[token])
        return "0"

    return IDENT_RE.sub(replace_token, expr)


def eval_pp_value(expr: str, macros: Dict[str, int]) -> Optional[int]:
    expr = rewrite_pp_expr(expr, macros)
    try:
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", SyntaxWarning)
            return int(eval(expr, {"__builtins__": {}}, {}))
    except Exception:
        return None


def eval_pp_expr(expr: str, macros: Dict[str, int]) -> Optional[int]:
    expr = rewrite_pp_expr(expr, macros)
    try:
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", SyntaxWarning)
            return int(bool(eval(expr, {"__builtins__": {}}, {})))
    except Exception:
        return None


def preprocess_text(text: str, initial_macros: Dict[str, int]) -> str:
    macros = dict(initial_macros)
    active_stack: List[bool] = []
    taken_stack: List[bool] = []
    output: List[str] = []

    for raw_line in text.splitlines():
        stripped = raw_line.strip()
        if not stripped.startswith("#"):
            if all(active_stack or [True]):
                output.append(raw_line)
            continue

        directive = stripped[1:].strip()
        parts = directive.split(None, 1)
        keyword = parts[0] if parts else ""
        arg = parts[1] if len(parts) > 1 else ""
        parent_active = all(active_stack[:-1] or [True])

        if keyword == "define":
            if all(active_stack or [True]) and arg:
                define_parts = arg.split(None, 1)
                name = define_parts[0]
                if "(" not in name:
                    value = define_parts[1] if len(define_parts) > 1 else "1"
                    parsed = parse_define_value(value, macros)
                    if parsed is not None:
                        macros[name] = parsed
            continue
        if keyword == "undef":
            if all(active_stack or [True]) and arg:
                macros.pop(arg.strip(), None)
            continue
        if keyword == "ifdef":
            cond = arg.strip() in macros
            active_stack.append(parent_active and cond)
            taken_stack.append(cond)
            continue
        if keyword == "ifndef":
            cond = arg.strip() not in macros
            active_stack.append(parent_active and cond)
            taken_stack.append(cond)
            continue
        if keyword == "if":
            value = eval_pp_expr(arg, macros)
            cond = bool(value)
            active_stack.append(parent_active and cond)
            taken_stack.append(cond)
            continue
        if keyword == "elif":
            if not active_stack:
                continue
            already_taken = taken_stack[-1]
            if already_taken:
                active_stack[-1] = False
                continue
            value = eval_pp_expr(arg, macros)
            cond = bool(value)
            active_stack[-1] = parent_active and cond
            taken_stack[-1] = cond
            continue
        if keyword == "else":
            if not active_stack:
                continue
            already_taken = taken_stack[-1]
            active_stack[-1] = parent_active and not already_taken
            taken_stack[-1] = True
            continue
        if keyword == "endif":
            if active_stack:
                active_stack.pop()
            if taken_stack:
                taken_stack.pop()
            continue

    return "\n".join(output) + "\n"


def collect_macros_from_text(text: str, initial_macros: Dict[str, int]) -> Dict[str, int]:
    macros = dict(initial_macros)
    active_stack: List[bool] = []
    taken_stack: List[bool] = []

    for raw_line in text.splitlines():
        stripped = raw_line.strip()
        if not stripped.startswith("#"):
            continue

        directive = stripped[1:].strip()
        parts = directive.split(None, 1)
        keyword = parts[0] if parts else ""
        arg = parts[1] if len(parts) > 1 else ""
        parent_active = all(active_stack[:-1] or [True])

        if keyword == "define":
            if all(active_stack or [True]) and arg:
                define_parts = arg.split(None, 1)
                name = define_parts[0]
                if "(" not in name:
                    value = define_parts[1] if len(define_parts) > 1 else "1"
                    parsed = parse_define_value(value, macros)
                    if parsed is not None:
                        macros[name] = parsed
            continue
        if keyword == "undef":
            if all(active_stack or [True]) and arg:
                macros.pop(arg.strip(), None)
            continue
        if keyword == "ifdef":
            cond = arg.strip() in macros
            active_stack.append(parent_active and cond)
            taken_stack.append(cond)
            continue
        if keyword == "ifndef":
            cond = arg.strip() not in macros
            active_stack.append(parent_active and cond)
            taken_stack.append(cond)
            continue
        if keyword == "if":
            value = eval_pp_expr(arg, macros)
            cond = bool(value)
            active_stack.append(parent_active and cond)
            taken_stack.append(cond)
            continue
        if keyword == "elif":
            if not active_stack:
                continue
            already_taken = taken_stack[-1]
            if already_taken:
                active_stack[-1] = False
                continue
            value = eval_pp_expr(arg, macros)
            cond = bool(value)
            active_stack[-1] = parent_active and cond
            taken_stack[-1] = cond
            continue
        if keyword == "else":
            if not active_stack:
                continue
            already_taken = taken_stack[-1]
            active_stack[-1] = parent_active and not already_taken
            taken_stack[-1] = True
            continue
        if keyword == "endif":
            if active_stack:
                active_stack.pop()
            if taken_stack:
                taken_stack.pop()
            continue

    return macros


def detect_header_guard(text: str) -> Optional[str]:
    text = strip_comments(text)
    lines = [line.strip() for line in text.splitlines()[:16] if line.strip()]
    for index, line in enumerate(lines):
        if not line.startswith("#ifndef "):
            continue
        guard = line[len("#ifndef ") :].strip()
        for next_line in lines[index + 1 :]:
            if next_line == f"#define {guard}":
                return guard
            if next_line.startswith("#if") or next_line.startswith("#include"):
                break
    return None


def collect_repo_macros(header_paths: Sequence[pathlib.Path], initial_macros: Dict[str, int]) -> Dict[str, int]:
    macros = dict(initial_macros)
    header_guards = set()
    texts: Dict[pathlib.Path, str] = {}
    for header_path in header_paths:
        text = header_path.read_text(encoding="utf-8", errors="replace")
        texts[header_path] = text
        guard = detect_header_guard(text)
        if guard:
            header_guards.add(guard)
    for header_path in header_paths:
        text = texts[header_path]
        text = strip_comments(text)
        macros = collect_macros_from_text(text, macros)
    for header_path in header_paths:
        text = texts[header_path]
        local_macros = dict(macros)
        guard = detect_header_guard(text)
        if guard:
            local_macros.pop(guard, None)
        text = preprocess_text(text, local_macros)
        macros = collect_enum_constants_from_text(text, macros)
    for guard in header_guards:
        macros.pop(guard, None)
    for header_path in header_paths:
        text = texts[header_path]
        text = strip_comments(text)
        macros = collect_macros_from_text(text, macros)
    for guard in header_guards:
        macros.pop(guard, None)
    return macros


def split_top_level_decls(text: str) -> List[str]:
    decls: List[str] = []
    current: List[str] = []
    brace_depth = 0

    for ch in text:
        current.append(ch)
        if ch == "{":
            brace_depth += 1
        elif ch == "}":
            if brace_depth > 0:
                brace_depth -= 1
        elif ch == ";" and brace_depth == 0:
            decl = "".join(current).strip()
            if decl:
                decls.append(decl)
            current = []

    return decls


def split_top_level_items(text: str, separator: str = ",") -> List[str]:
    items: List[str] = []
    current: List[str] = []
    brace_depth = 0
    paren_depth = 0
    bracket_depth = 0

    for ch in text:
        current.append(ch)
        if ch == "{":
            brace_depth += 1
        elif ch == "}":
            if brace_depth > 0:
                brace_depth -= 1
        elif ch == "(":
            paren_depth += 1
        elif ch == ")":
            if paren_depth > 0:
                paren_depth -= 1
        elif ch == "[":
            bracket_depth += 1
        elif ch == "]":
            if bracket_depth > 0:
                bracket_depth -= 1
        elif ch == separator and brace_depth == 0 and paren_depth == 0 and bracket_depth == 0:
            item = "".join(current[:-1]).strip()
            if item:
                items.append(item)
            current = []

    tail = "".join(current).strip()
    if tail:
        items.append(tail)
    return items


def substitute_array_bounds(decl: str, macros: Dict[str, int]) -> str:
    def replace(match: re.Match[str]) -> str:
        expr = match.group(1).strip()
        value = eval_pp_value(expr, macros)
        if value is None:
            return match.group(0)
        return f"[{value}]"

    return ARRAY_BOUNDS_RE.sub(replace, decl)


def substitute_bitfield_bounds(decl: str, macros: Dict[str, int]) -> str:
    def replace(match: re.Match[str]) -> str:
        expr = match.group(2).strip()
        value = eval_pp_value(expr, macros)
        if value is None:
            return match.group(0)
        return f"{match.group(1)}{value}{match.group(3)}"

    return BITFIELD_BOUNDS_RE.sub(replace, decl)


def normalize_typedef_spacing(decl: str) -> str:
    return re.sub(r"\}\s*([A-Za-z_][A-Za-z0-9_]*)\s*;", r"} \1;", decl)


def strip_windows_type_qualifiers(decl: str) -> str:
    decl = WINDOWS_QUALIFIER_RE.sub("", decl)
    decl = re.sub(r"\s{2,}", " ", decl)
    decl = re.sub(r"\s+\*", " *", decl)
    return decl


def replace_dummy_aggregate_macros(decl: str) -> str:
    def replace(match: re.Match[str]) -> str:
        value = re.sub(r"\W+", "_", match.group(1).strip()).strip("_")
        if not value:
            value = "field"
        return f"u{value}"

    decl = re.sub(r"\bDUMMY(?:UNION|STRUCT)NAMEN\s*\(\s*([^)]+?)\s*\)", replace, decl)
    decl = re.sub(r"\bDUMMY(?:UNION|STRUCT)NAME\b", "u", decl)
    return decl


def split_typedef_body_aliases(decl: str) -> str:
    if not decl.lstrip().startswith("typedef") or "{" not in decl or "}" not in decl:
        return decl

    head, tail = decl.rsplit("}", 1)
    tail = tail.rsplit(";", 1)[0].strip()
    aliases = split_top_level_items(tail)
    if len(aliases) <= 1:
        return decl

    primary = aliases[0].strip()
    primary_name_match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\]\s*)?$", primary)
    if not primary_name_match:
        return decl

    primary_name = primary_name_match.group(1)
    rewritten = [f"{head}}} {primary};"]
    for alias in aliases[1:]:
        alias = alias.strip()
        if alias:
            rewritten.append(f"typedef {primary_name} {alias};")
    return "\n".join(rewritten)


def tag_top_level_anonymous_typedef(decl: str) -> str:
    match = re.match(r"^(\s*typedef\s+)(struct|union|enum)(\s*)\{", decl)
    if not match:
        return decl

    names = extract_declared_names(decl)
    if not names:
        return decl

    tag = names[0]
    insert_at = match.start(3)
    return decl[:insert_at] + f" {tag}" + decl[insert_at:]


def strip_top_level_typedef_tag_if_safe(decl: str) -> str:
    match = re.match(r"^(\s*typedef\s+)(struct|union|enum)(\s+)([A-Za-z_][A-Za-z0-9_]*)(\s*\{)", decl)
    if not match:
        return decl
    kind = match.group(2)
    tag = match.group(4)
    if tag in extract_declared_names(decl):
        return decl
    body = decl[match.end() :]
    if re.search(rf"\b{kind}\s+{re.escape(tag)}\b", body):
        return decl
    return decl[: match.start(4)] + decl[match.end(4) :]


def name_anonymous_aggregates(decl: str) -> str:
    result: List[str] = []
    stack: List[Dict[str, object]] = []
    anon_counter = 0
    i = 0
    n = len(decl)

    while i < n:
        matched = False
        for kind in ("struct", "union", "enum"):
            if decl.startswith(kind, i):
                prev_ok = i == 0 or not (decl[i - 1].isalnum() or decl[i - 1] == "_")
                next_index = i + len(kind)
                next_ok = next_index >= n or not (decl[next_index].isalnum() or decl[next_index] == "_")
                if not (prev_ok and next_ok):
                    continue

                j = next_index
                while j < n and decl[j].isspace():
                    j += 1
                anonymous = j < n and decl[j] == "{"
                if anonymous:
                    definition = True
                else:
                    k = j
                    if k < n and (decl[k].isalpha() or decl[k] == "_"):
                        k += 1
                        while k < n and (decl[k].isalnum() or decl[k] == "_"):
                            k += 1
                        while k < n and decl[k].isspace():
                            k += 1
                        definition = k < n and decl[k] == "{"
                    else:
                        definition = False
                if not definition:
                    continue
                top_level = anonymous and not stack and decl[:i].lstrip().startswith("typedef")
                anon_tag = None
                if anonymous and not top_level:
                    anon_counter += 1
                    anon_tag = f"__anon_{kind}_{anon_counter}"
                    result.append(f"{kind} {anon_tag}")
                else:
                    result.append(kind)
                stack.append(
                    {
                        "kind": kind,
                        "anonymous": anonymous,
                        "top_level": top_level,
                        "anon_tag": anon_tag,
                    }
                )
                i = next_index
                matched = True
                break
        if matched:
            continue

        ch = decl[i]
        if ch == "{":
            result.append(ch)
            i += 1
            continue
        if ch == "}":
            result.append(ch)
            aggregate = stack.pop() if stack else None
            if aggregate and aggregate["anonymous"] and not aggregate["top_level"]:
                j = i + 1
                while j < n and decl[j].isspace():
                    result.append(decl[j])
                    j += 1
                if j < n and decl[j] == ";":
                    result.append(f" {aggregate['anon_tag']}")
            i += 1
            continue

        result.append(ch)
        i += 1

    return "".join(result)


def sanitize_decl_for_ida(decl: str) -> str:
    decl = strip_windows_type_qualifiers(decl)
    decl = normalize_typedef_spacing(decl)
    decl = replace_dummy_aggregate_macros(decl)
    decl = tag_top_level_anonymous_typedef(decl)
    decl = strip_top_level_typedef_tag_if_safe(decl)
    decl = name_anonymous_aggregates(decl)
    decl = split_typedef_body_aliases(decl)
    return decl


def sanitize_decl_for_numbered_type(decl: str) -> str:
    decl = strip_windows_type_qualifiers(decl)
    decl = normalize_typedef_spacing(decl)
    decl = replace_dummy_aggregate_macros(decl)
    decl = tag_top_level_anonymous_typedef(decl)
    decl = strip_top_level_typedef_tag_if_safe(decl)
    decl = name_anonymous_aggregates(decl)
    decl = split_typedef_body_aliases(decl)
    return decl


def prepend_forward_tag_decls(decl: str) -> str:
    defined_tags = set()
    for kind, name in TAG_REF_RE.findall(decl):
        if f"{kind} {name}" in decl and f"{kind} {name}\n" in decl and "{" in decl:
            pass

    defined_tags.update(re.findall(r"\b(?:typedef\s+)?(struct|union|enum)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{", decl))
    defined_tag_pairs = {(kind, name) for kind, name in defined_tags}

    forward_lines: List[str] = []
    seen = set()
    for kind, name in TAG_REF_RE.findall(decl):
        pair = (kind, name)
        if pair in defined_tag_pairs or pair in seen:
            continue
        seen.add(pair)
        forward_lines.append(f"{kind} {name};")

    if not forward_lines:
        return decl
    return "\n".join(forward_lines) + "\n" + decl


def collect_enum_constants_from_text(text: str, initial_values: Dict[str, int]) -> Dict[str, int]:
    values = dict(initial_values)
    text = strip_comments(text)
    for decl in split_top_level_decls(text):
        decl = decl.strip()
        if not decl.startswith("enum") and not decl.startswith("typedef enum"):
            continue
        if "{" not in decl or "}" not in decl:
            continue
        body = decl.split("{", 1)[1].rsplit("}", 1)[0]
        next_value = 0
        for item in split_top_level_items(body):
            item = item.strip()
            if not item:
                continue
            if "=" in item:
                name, expr = item.split("=", 1)
                name = name.strip()
                parsed = eval_pp_value(expr.strip(), values)
                if parsed is None:
                    continue
                values[name] = parsed
                next_value = parsed + 1
            else:
                name = item.strip()
                if not IDENT_RE.fullmatch(name):
                    continue
                values[name] = next_value
                next_value += 1
    return values


def extract_declared_names(decl: str) -> Tuple[str, ...]:
    decl = decl.strip()
    if not TYPE_START_RE.match(decl):
        return ()

    if decl.startswith("typedef"):
        if "}" in decl:
            tail = decl.rsplit("}", 1)[1].rsplit(";", 1)[0]
            names = []
            for part in tail.split(","):
                match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\]\s*)?$", part.strip())
                if match:
                    name = match.group(1)
                    if name not in names:
                        names.append(name)
            return tuple(names)

        func_ptr_match = FUNCTION_PTR_TYPEDEF_RE.match(decl)
        if func_ptr_match:
            return (func_ptr_match.group(1),)

        tail = decl.rsplit(";", 1)[0]
        match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\]\s*)?$", tail)
        return (match.group(1),) if match else ()

    match = re.match(r"\s*(struct|union|enum)\s+([A-Za-z_][A-Za-z0-9_]*)", decl)
    if match and "{" in decl:
        return (match.group(2),)

    return ()


def decl_score(decl: str) -> int:
    score = 0
    if "{" in decl and "}" in decl:
        score += 100
    if decl.startswith("typedef"):
        score += 10
    return score


def collect_decl_candidates(header_path: pathlib.Path, macros: Dict[str, int]) -> List[DeclCandidate]:
    text = header_path.read_text(encoding="utf-8", errors="replace")
    text = preprocess_text(text, macros)
    text = strip_comments(text)

    candidates: List[DeclCandidate] = []
    for order, decl in enumerate(split_top_level_decls(text)):
        decl = decl.strip()
        if not TYPE_START_RE.match(decl):
            continue
        if "<" in decl and ">" in decl:
            continue
        if decl.startswith("struct __extensions__"):
            continue
        if STATIC_ASSERT_TYPEDEF_RE.match(decl) and ("sizeof(" in decl or "offsetof(" in decl):
            continue
        decl = substitute_array_bounds(decl, macros)
        decl = substitute_bitfield_bounds(decl, macros)

        names = extract_declared_names(decl)
        if not names:
            continue

        candidates.append(
            DeclCandidate(
                header_path=header_path,
                decl=decl,
                names=names,
                has_body=("{" in decl and "}" in decl),
                score=decl_score(decl),
                order=order,
            )
        )

    return candidates


def choose_best_decl_per_name(candidates: Iterable[DeclCandidate]) -> Dict[str, DeclCandidate]:
    best: Dict[str, DeclCandidate] = {}
    for candidate in candidates:
        for name in candidate.names:
            previous = best.get(name)
            if previous is None or candidate.score > previous.score or (
                candidate.score == previous.score and candidate.order < previous.order
            ):
                best[name] = candidate
    return best


def parse_cli_defines(entries: Sequence[str]) -> Dict[str, int]:
    macros: Dict[str, int] = {}
    for entry in entries:
        if "=" in entry:
            key, value = entry.split("=", 1)
            parsed = parse_define_value(value, macros)
            macros[key] = 1 if parsed is None else parsed
        else:
            macros[entry] = 1
    return macros


def get_existing_ordinal(name: str) -> Optional[int]:
    resolved_name = resolve_existing_name(name)
    if resolved_name is None:
        return None

    tif = ida_typeinf.tinfo_t()
    try:
        found = tif.get_named_type(ida_typeinf.get_idati(), resolved_name)
    except TypeError:
        found = tif.get_named_type(None, resolved_name)

    if not found:
        return None

    ordinal = tif.get_ordinal()
    if ordinal in (-1, 0):
        return None
    return ordinal


def iter_existing_local_type_names() -> Iterable[str]:
    if not IN_IDA:
        return []

    qty = idc.get_ordinal_qty()
    names: List[str] = []
    for ordinal in range(1, qty + 1):
        name = idc.get_numbered_type_name(ordinal)
        if name:
            names.append(name)
    return names


def resolve_existing_name(name: str) -> Optional[str]:
    if not IN_IDA:
        return None

    tif = ida_typeinf.tinfo_t()
    try:
        found = tif.get_named_type(ida_typeinf.get_idati(), name)
    except TypeError:
        found = tif.get_named_type(None, name)
    if found:
        ordinal = tif.get_ordinal()
        debug_log(f"resolve_existing_name({name}) -> exact match (ordinal {ordinal})")
        return name

    alias_re = re.compile(SUFFIXED_ALIAS_RE_TEMPLATE.format(base=re.escape(name)))
    matches = [candidate for candidate in iter_existing_local_type_names() if alias_re.match(candidate)]
    if len(matches) == 1:
        debug_log(f"resolve_existing_name({name}) -> unique suffixed alias {matches[0]}")
        return matches[0]
    if matches:
        debug_log(f"resolve_existing_name({name}) -> ambiguous suffixed aliases {matches}")
    else:
        debug_log(f"resolve_existing_name({name}) -> no existing local type")
    return None


def retarget_decl_name(decl: str, old_name: str, new_name: str) -> str:
    if old_name == new_name:
        return decl

    return re.sub(rf"\b{re.escape(old_name)}\b", new_name, decl)


def parser_flag_silent() -> int:
    if not IN_IDA:
        return 0
    for attr in ("PT_SIL", "PT_SILENT"):
        if hasattr(ida_typeinf, attr):
            return int(getattr(ida_typeinf, attr))
    return 0


def parser_flag_unsilent(flags: int) -> int:
    for attr in ("PT_SIL", "PT_SILENT"):
        if hasattr(ida_typeinf, attr):
            return flags & ~int(getattr(ida_typeinf, attr))
    return flags


def parser_flag_replace() -> int:
    if not IN_IDA or not hasattr(ida_typeinf, "PT_REPLACE"):
        return 0
    return int(ida_typeinf.PT_REPLACE)


def import_decl_flags() -> int:
    return parser_flag_silent() | parser_flag_replace()


def get_named_type_size(name: str) -> Optional[int]:
    if not IN_IDA:
        return None

    tif = ida_typeinf.tinfo_t()
    try:
        found = tif.get_named_type(ida_typeinf.get_idati(), name)
    except TypeError:
        found = tif.get_named_type(None, name)
    if not found:
        return None

    for attr in ("get_size",):
        getter = getattr(tif, attr, None)
        if getter is None:
            continue
        try:
            size = getter()
        except Exception:
            continue
        if isinstance(size, int) and size >= 0:
            return size
    return None


def _set_numbered_type_with_flags(
    ordinal: int,
    flags: int,
    name: str,
    type_bytes: bytes,
    field_bytes: Optional[bytes],
) -> object:
    try:
        return ida_typeinf.set_numbered_type(
            ida_typeinf.get_idati(),
            ordinal,
            flags,
            name,
            type_bytes,
            field_bytes or None,
            None,
            None,
            None,
        )
    except TypeError:
        return ida_typeinf.set_numbered_type(
            ida_typeinf.get_idati(),
            ordinal,
            flags,
            name,
            type_bytes,
            field_bytes or None,
        )


def _replace_existing_numbered_type(name: str, decl: str) -> Optional[bool]:
    if not IN_IDA or not hasattr(ida_typeinf, "idc_parse_decl") or not hasattr(ida_typeinf, "set_numbered_type"):
        return None

    ordinal = get_existing_ordinal(name)
    if ordinal is None:
        return None

    decl = sanitize_decl_for_numbered_type(decl)
    parse_flags = parser_flag_silent()
    try:
        parsed = ida_typeinf.idc_parse_decl(ida_typeinf.get_idati(), decl, parse_flags)
    except Exception as exc:
        debug_log(f"idc_parse_decl failed for {name}: {exc!r}")
        return None

    if not parsed:
        debug_log(f"idc_parse_decl returned no result for {name}")
        return None

    parsed_name, type_bytes, field_bytes = parsed
    if parsed_name != name:
        debug_log(f"idc_parse_decl returned mismatched parsed_name for {name}: {parsed_name!r}")
        return None

    if not type_bytes:
        debug_log(f"idc_parse_decl returned no type bytes for {name} (parsed_name={parsed_name!r})")
        return None

    terr_ok = getattr(ida_typeinf, "TERR_OK", 0)
    parsed_size = ida_typeinf.calc_type_size(ida_typeinf.get_idati(), type_bytes)

    flag_attempts = []
    base_flags = 0
    for attr in ("NTF_TYPE", "NTF_REPLACE", "NTF_CHKSYNC"):
        if hasattr(ida_typeinf, attr):
            base_flags |= int(getattr(ida_typeinf, attr))
    flag_attempts.append(("replace", base_flags))
    if hasattr(ida_typeinf, "NTF_COPY"):
        flag_attempts.append(("replace_copy", base_flags | int(getattr(ida_typeinf, "NTF_COPY"))))

    for label, ntf_flags in flag_attempts:
        try:
            result = _set_numbered_type_with_flags(ordinal, ntf_flags, name, type_bytes, field_bytes)
        except Exception as exc:
            debug_log(f"set_numbered_type failed for {name} [{label}]: {exc!r}")
            continue

        live_size = get_named_type_size(name)
        debug_log(
            f"set_numbered_type({name}, ordinal={ordinal}, mode={label}) -> {result!r}, parsed_name={parsed_name!r}, "
            f"parsed_size={parsed_size!r}, live_size={live_size!r}"
        )
        if result == terr_ok and (parsed_size in (-1, None) or live_size == parsed_size):
            return True

    return False


def import_type_decl(decl: str, target_name: Optional[str] = None) -> bool:
    raw_decl = decl
    decl = sanitize_decl_for_ida(decl)
    if target_name:
        replaced = _replace_existing_numbered_type(target_name, raw_decl)
        if replaced is not None:
            return replaced
        before_size = get_named_type_size(target_name)
    else:
        before_size = None
    decl = prepend_forward_tag_decls(decl)
    flags = import_decl_flags()
    if hasattr(idc, "parse_decls"):
        result = idc.parse_decls(decl, flags)
        if result != 0:
            log("[debug] parse_decls failed; retrying unsilenced for diagnostics")
            log(decl)
            idc.parse_decls(decl, parser_flag_unsilent(flags))
    elif hasattr(ida_typeinf, "idc_parse_types"):
        result = ida_typeinf.idc_parse_types(decl, flags)
        if result != 0:
            log("[debug] idc_parse_types failed; retrying unsilenced for diagnostics")
            log(decl)
            ida_typeinf.idc_parse_types(decl, parser_flag_unsilent(flags))
    else:
        raise AttributeError("No supported type import function found in this IDA build")
    if target_name:
        after_size = get_named_type_size(target_name)
        debug_log(
            f"parse_decls fallback for {target_name}: result={result!r}, before_size={before_size!r}, after_size={after_size!r}"
        )
    return result == 0


def candidate_dependencies(candidate: DeclCandidate, best_by_name: Dict[str, DeclCandidate]) -> List[str]:
    deps = []
    declared_names = set(candidate.names)
    for match in IDENT_RE.finditer(candidate.decl):
        token = match.group(0)
        if token in declared_names or token not in best_by_name:
            continue
        prefix = candidate.decl[:match.start()].rstrip()
        if prefix.endswith("struct") or prefix.endswith("union") or prefix.endswith("enum"):
            continue
        if token not in deps:
            deps.append(token)
    return deps


def required_candidates_for_targets(
    best_by_name: Dict[str, DeclCandidate], target_names: Iterable[str]
) -> List[DeclCandidate]:
    ordered: List[DeclCandidate] = []
    seen_decl_keys = set()
    visiting = set()

    def visit(name: str) -> None:
        candidate = best_by_name.get(name)
        if candidate is None:
            return

        decl_key = (candidate.header_path, candidate.decl)
        if decl_key in seen_decl_keys or name in visiting:
            return

        visiting.add(name)
        for dep_name in candidate_dependencies(candidate, best_by_name):
            visit(dep_name)
        visiting.remove(name)

        if decl_key not in seen_decl_keys:
            seen_decl_keys.add(decl_key)
            ordered.append(candidate)

    for name in target_names:
        visit(name)
    return ordered


def main(argv: Sequence[str]) -> int:
    global DEBUG_ENABLED, LOG_FILE_PATH
    args = parse_args(argv)
    DEBUG_ENABLED = args.debug
    LOG_FILE_PATH = args.log_file.resolve() if args.log_file else None
    if LOG_FILE_PATH is not None:
        LOG_FILE_PATH.parent.mkdir(parents=True, exist_ok=True)
        LOG_FILE_PATH.write_text("", encoding="utf-8")
    root = args.root.resolve()
    macros = parse_cli_defines(args.defines)
    headers = repo_source_paths(root, args.header)
    macro_source_paths = repo_source_paths(root, [])

    if not headers:
        log("No headers found.")
        return 1

    repo_macros = collect_repo_macros(macro_source_paths, macros)
    debug_log(f"collected {len(repo_macros)} macros from {len(macro_source_paths)} source files")
    all_candidates: List[DeclCandidate] = []
    for header_path in headers:
        if not header_path.exists():
            log(f"[skip] Missing source file: {header_path}")
            continue
        all_candidates.extend(collect_decl_candidates(header_path, repo_macros))

    if not all_candidates:
        log("No type declarations extracted.")
        return 1

    best_by_name = choose_best_decl_per_name(all_candidates)
    debug_log(f"extracted {len(all_candidates)} candidates, {len(best_by_name)} unique type names")

    requested_names = set(args.type_names)
    if requested_names:
        target_names = []
        for name in args.type_names:
            if name in best_by_name and name not in target_names:
                target_names.append(name)
            elif name not in best_by_name:
                debug_log(f"requested type {name} was not extracted from source")
    elif args.existing_only:
        target_names = [name for name in sorted(best_by_name) if get_existing_ordinal(name) is not None]
    else:
        target_names = sorted(best_by_name)

    if not requested_names:
        target_names.sort(key=lambda name: (best_by_name[name].header_path.as_posix(), best_by_name[name].order, name))
    import_candidates = required_candidates_for_targets(best_by_name, target_names)
    debug_log(f"target names: {target_names}")
    for name in target_names:
        candidate = best_by_name[name]
        debug_log(
            f"target {name}: deps={candidate_dependencies(candidate, best_by_name)} "
            f"header={candidate.header_path.relative_to(root)}"
        )
    debug_log(f"import order: {[candidate.names[0] for candidate in import_candidates]}")

    if args.scan_only or not IN_IDA:
        for name in target_names:
            candidate = best_by_name[name]
            body_note = "definition" if candidate.has_body else "forward/alias"
            existing_name = resolve_existing_name(name) if IN_IDA else None
            existing = existing_name is not None
            action = "update" if existing else "add"
            display_name = existing_name or name
            alias_note = f" (source {name})" if existing_name and existing_name != name else ""
            log(f"{display_name}: {action} {body_note} from {candidate.header_path.relative_to(root)}{alias_note}")
        if not IN_IDA and not args.scan_only:
            log("Not running under IDA; scan complete only.")
        return 0

    applied = 0
    skipped_missing = 0
    failed = 0
    if requested_names:
        skipped_missing = len(requested_names - set(target_names))

    for candidate in import_candidates:
        representative_name = candidate.names[0]
        existing_name = resolve_existing_name(representative_name)
        existing = existing_name is not None
        decl_to_import = candidate.decl
        if existing_name and existing_name != representative_name:
            decl_to_import = retarget_decl_name(candidate.decl, representative_name, existing_name)
        debug_log(
            f"importing {representative_name} as {existing_name or representative_name} "
            f"from {candidate.header_path.relative_to(root)}"
        )

        if args.dry_run:
            log(
                f"[dry-run] {'update' if existing else 'add'} {existing_name or representative_name} <- {candidate.header_path.relative_to(root)}"
            )
            applied += 1
            continue

        if import_type_decl(decl_to_import, existing_name or representative_name):
            applied += 1
            log(
                f"[ok] {existing_name or representative_name} <- {candidate.header_path.relative_to(root)}"
            )
        else:
            failed += 1
            log(
                f"[fail] {existing_name or representative_name} <- {candidate.header_path.relative_to(root)}"
            )

    log(
        f"Done. applied={applied} skipped_missing={skipped_missing} failed={failed}"
    )
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    exit_code = main(sys.argv[1:])
    if not IN_IDA:
        raise SystemExit(exit_code)
