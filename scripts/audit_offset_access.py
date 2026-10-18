#!/usr/bin/env python3
"""
audit_offset_access.py -- Raw struct-field offset access audit for Cataclysm.

Finds raw pointer arithmetic used to access struct fields by byte offset
(e.g. *(udword *)((ubyte *)ship + 0x1C8)) that indicate missing or unnamed
struct members.  These should be converted to proper named field accesses.

Detection strategies
--------------------
  cast_hex       (ubyte*)ptr + 0xNN  -- canonical IDA decompile pattern, high confidence
  cast_dec       (ubyte*)ptr + NNN   -- decimal offset >= --min-decimal (default 4)
  index_bytevar  byteVar[N]          -- array-index access through a variable that was
                                         itself declared/assigned as a raw byte pointer
                                         (ubyte*/char*/uint8_t*/BYTE*/sbyte*)
  cast_bytevar   (T *)(byteVar + N)  -- offset arithmetic on a raw byte-pointer local,
                                         then cast to some other type at the point of
                                         dereference (the cast is on the *result*, not
                                         the base pointer, so cast_hex/cast_dec miss it)
  broad_hex      any line with + 0xNN -- catches edge cases; use --broad to enable;
                                         has false positives (bitmask ops, etc.)

Note: index_bytevar/cast_bytevar require the base variable to have been declared (or
assigned via `x = (ubyte *)y;`) as a byte-pointer type somewhere in the same file, so
they stay high-confidence without needing --broad.

Type attribution
----------------
  Each file is scanned for typed-pointer declarations (parameters and locals).
  The variable name in each offset expression is looked up in that map.
  Falls back to the raw var_expr when the type cannot be determined.

Usage
-----
  python scripts/audit_offset_access.py                   # struct-grouped summary
  python scripts/audit_offset_access.py --struct Ship     # detail for one struct
  python scripts/audit_offset_access.py --lines           # show source lines
  python scripts/audit_offset_access.py --files           # group by file
  python scripts/audit_offset_access.py --broad           # include broad hits
  python scripts/audit_offset_access.py --min-decimal 0   # catch all decimal offsets
  python scripts/audit_offset_access.py --csv             # CSV for further processing
"""

import os
import re
import sys
import csv
import glob
import argparse
from collections import defaultdict


# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

DEFAULT_MIN_DECIMAL = 4     # Minimum decimal offset to flag as struct access.
                             # Values < 4 are almost always pointer ++ steps.

# Raw byte-pointer types -- when a variable is declared/assigned as one of these,
# any offset arithmetic or indexing through it is raw memory access, regardless of
# what the *result* of that arithmetic is later cast to.
BYTE_PTR_TYPES = ('ubyte', 'char', 'uint8_t', 'BYTE', 'sbyte')

# Types that are never struct names -- skip attribution for these.
PRIMITIVE_TYPES = frozenset({
    'int', 'char', 'void', 'float', 'double', 'short', 'long',
    'unsigned', 'signed', 'bool', 'byte', 'ubyte', 'sbyte',
    'sdword', 'udword', 'sword', 'uword', 'real32', 'real64',
    'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t',
    'int8_t', 'int16_t', 'int32_t', 'int64_t',
    'size_t', 'ptrdiff_t', 'uintptr_t', 'intptr_t',
    'BYTE', 'WORD', 'DWORD', 'QWORD', 'BOOL', 'HANDLE',
    'LPVOID', 'PVOID', 'LPBYTE', 'LPDWORD',
    'bool32', 'bitflag32', 'memsize', 'const', 'FILE', 'va_list',
})


# ---------------------------------------------------------------------------
# Regexes
# ---------------------------------------------------------------------------

# (ubyte *)ptr + 0xNN  or  (ubyte *)(ptr) + 0xNN
# Also matches (char *), (uint8_t *), (BYTE *), and const-qualified variants.
# Group 1: variable expression (may include -> and . for member chains)
# Group 2: hex offset string including 0x prefix
CAST_HEX_RE = re.compile(
    r'\(\s*(?:const\s+)?(?:ubyte|char|uint8_t|BYTE)\s*\*\s*\)'   # (ubyte *) / (const ubyte *)
    r'\s*(\(\s*[\w>.\-]+\s*\)|[\w>.\-]+)'                         # var or (var), allows -> and .
    r'\s*\+\s*(0x[0-9a-fA-F]+)',                                   # + 0xNN
)

# (ubyte *)ptr + NNN  -- decimal, no 0x prefix
# Group 1: variable expression
# Group 2: decimal string
CAST_DEC_RE = re.compile(
    r'\(\s*(?:const\s+)?(?:ubyte|char|uint8_t|BYTE)\s*\*\s*\)'   # (ubyte *) / (const ubyte *)
    r'\s*(\(\s*[\w>.\-]+\s*\)|[\w>.\-]+)'                         # var or (var)
    r'\s*\+\s*(\d+)\b',                                            # + NNN (no 0x)
)

# Broad: any + 0xNN anywhere on a line (many false positives).
BROAD_HEX_RE = re.compile(r'\+\s*(0x[0-9a-fA-F]+)')

# Declaration of a raw byte-pointer variable, e.g. "ubyte *commandBytes" as a
# parameter or local (with or without an initializer).
# Group 1: variable name.
BYTE_PTR_DECL_RE = re.compile(
    r'\b(?:const\s+)?(?:' + '|'.join(BYTE_PTR_TYPES) + r')\s*\*\s*(\w+)\b'
)

# Assignment of a byte-pointer local from a cast of another (typed) variable, e.g.
# "commandBytes = (ubyte *)command;" -- lets us attribute the byte-pointer variable
# back to the original struct type for reporting purposes.
# Group 1: byte-pointer variable name.  Group 2: source variable expression.
BYTE_PTR_ASSIGN_RE = re.compile(
    r'\b(\w+)\s*=\s*\(\s*(?:const\s+)?(?:' + '|'.join(BYTE_PTR_TYPES) + r')\s*\*\s*\)'
    r'\s*(\(?\s*[\w>.\-]+\s*\)?)\s*;'
)

# byteVar[N] -- array-index access through a variable already known to be a raw
# byte pointer (see build_byte_ptr_vars).  Only meaningful once the caller filters
# by byte_ptr_vars membership.
# Group 1: variable name.  Group 2: offset (decimal or hex).
INDEX_BYTEVAR_RE = re.compile(r'\b(\w+)\s*\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]')

# (SomeType *)(byteVar + N)  or  (SomeType *)byteVar + N -- offset arithmetic on a
# raw byte-pointer local where the cast is on the *result*, not the base pointer.
# Only meaningful once the caller filters by byte_ptr_vars membership.
# Group 1: variable name.  Group 2: offset (decimal or hex).
CAST_BYTEVAR_RE = re.compile(
    r'\(\s*(?:const\s+)?[A-Za-z_]\w*\s*\*+\s*\)'
    r'\s*\(?\s*(\w+)\s*\+\s*(0x[0-9a-fA-F]+|\d+)\s*\)?'
)

# Typed pointer: "TypeName *varname" or "TypeName* varname"
# Used to build variable-name -> struct-type map.
TYPED_PTR_RE = re.compile(r'\b([A-Z]\w*)\s*\*+\s*(\w+)\b')

# Strip C++ and C line comments (leaves the rest of the line intact).
LINE_COMMENT_RE = re.compile(r'//.*$')


# ---------------------------------------------------------------------------
# Type-map building
# ---------------------------------------------------------------------------

def build_type_map(content):
    """
    Scan file content and return {var_name: struct_type} for all typed pointer
    declarations where the type starts with an uppercase letter and is not a
    known primitive.

    Covers both function parameters and local variable declarations.
    """
    type_map = {}
    for m in TYPED_PTR_RE.finditer(content):
        type_name = m.group(1)
        var_name  = m.group(2)
        if type_name not in PRIMITIVE_TYPES:
            type_map[var_name] = type_name
    return type_map


def attribute_type(var_expr, type_map):
    """
    Map a variable expression to a struct type name.

    Handles:
      - Simple variable:       "ship"           -> type_map["ship"]
      - Parenthesised:         "(ship)"         -> type_map["ship"]
      - Single arrow:          "ship->member"   -> type_map["ship"]  (root only)
      - Multi-level arrows:    "a->b->c"        -> type_map["a"]     (root only)

    For arrow-chains the root variable's type is returned.  This may not be
    perfectly accurate for multi-hop chains (e.g. ship->staticinfo->custstatinfo)
    but still identifies the owning struct tree.

    Returns the struct type name string, or None if unknown.
    """
    expr = var_expr.strip().strip('()')

    # Simple variable name
    if re.match(r'^\w+$', expr):
        return type_map.get(expr)

    # Arrow/dot chain -- look up the root variable
    root = re.split(r'->|\.', expr)[0].strip()
    return type_map.get(root)


def build_byte_ptr_vars(content, type_map):
    """
    Scan file content and return {var_name: struct_type_or_None} for every
    variable declared (as a parameter or local) with a raw byte-pointer type
    (ubyte*/char*/uint8_t*/BYTE*/sbyte*).

    If the variable is later assigned from a cast of another typed variable
    (e.g. "commandBytes = (ubyte *)command;"), the value is the attributed
    struct type of that source variable (via *type_map*) so offset hits
    through the byte pointer can still be grouped under the real struct.
    Otherwise the value is None (unknown origin).
    """
    byte_vars = {}
    for m in BYTE_PTR_DECL_RE.finditer(content):
        byte_vars.setdefault(m.group(1), None)
    for m in BYTE_PTR_ASSIGN_RE.finditer(content):
        var_name = m.group(1)
        origin   = m.group(2)
        origin_type = attribute_type(origin, type_map)
        if origin_type:
            byte_vars[var_name] = origin_type
        else:
            byte_vars.setdefault(var_name, None)
    return byte_vars


# ---------------------------------------------------------------------------
# Hit collection
# ---------------------------------------------------------------------------

IF0_OPEN_RE  = re.compile(r'^\s*#\s*if\s+0\b')
IF0_CLOSE_RE = re.compile(r'^\s*#\s*endif\b')
IFNN_OPEN_RE = re.compile(r'^\s*#\s*if\b(?!\s+0\b)')   # any #if that is NOT #if 0


def collect_hits(src_root, min_decimal=DEFAULT_MIN_DECIMAL,
                 include_broad=False, include_dead=False):
    """
    Scan every .c and .h file under *src_root* for raw offset accesses.

    Returns a list of hit dicts:
      file        -- relative path from repo root
      lineno      -- 1-based line number
      line        -- original source line (rstripped)
      var_expr    -- extracted variable expression (may be '?' for broad hits)
      offset      -- offset string as it appears in source
      offset_int  -- integer value of the offset
      kind        -- 'cast_hex' | 'cast_dec' | 'index_bytevar' | 'cast_bytevar' | 'broad_hex'
      struct_type -- attributed struct name, or None
      in_dead     -- True if the hit is inside a #if 0 block

    By default hits inside #if 0 blocks (dead code) are excluded.
    Pass include_dead=True to include them.
    """
    hits = []

    c_files  = glob.glob(os.path.join(src_root, '**', '*.c'),  recursive=True)
    h_files  = glob.glob(os.path.join(src_root, '**', '*.h'),  recursive=True)
    all_files = sorted(c_files + h_files)

    repo_root = os.path.dirname(src_root)

    for fpath in all_files:
        try:
            with open(fpath, 'r', encoding='utf-8', errors='ignore') as fh:
                content = fh.read()
        except OSError:
            continue

        rel_path = os.path.relpath(fpath, repo_root)
        type_map = build_type_map(content)
        byte_ptr_vars = build_byte_ptr_vars(content, type_map)
        lines    = content.split('\n')

        # Track #if 0 nesting depth to identify dead code regions.
        # dead_depth > 0  => we are inside at least one #if 0 block.
        dead_depth  = 0   # depth of #if 0 blocks
        total_depth = 0   # total #if depth (for proper #endif matching)

        # (lineno, offset_key) set -- deduplicate across strategies
        seen = set()

        for lineno, raw_line in enumerate(lines, 1):
            # Track preprocessor nesting BEFORE stripping comments so we
            # catch "#if 0" and "#endif" on otherwise-clean lines.
            ls_raw = raw_line.strip()
            if IF0_OPEN_RE.match(ls_raw):
                dead_depth  += 1
                total_depth += 1
            elif IFNN_OPEN_RE.match(ls_raw):
                total_depth += 1
            elif IF0_CLOSE_RE.match(ls_raw) and total_depth > 0:
                if dead_depth > 0 and total_depth == dead_depth:
                    dead_depth -= 1
                total_depth -= 1

            in_dead = dead_depth > 0

            # Skip dead-code hits unless the caller asked for them.
            if in_dead and not include_dead:
                continue

            # Strip line comments before pattern matching.
            stripped = LINE_COMMENT_RE.sub('', raw_line)

            # Skip pure comment lines (Javadoc-style "/** ... */" continuations:
            # " * comment text" or a lone "*/").  Deliberately narrow -- must NOT
            # match code that starts with a bare dereference like "*ptr" or a
            # double-dereference like "**(TYPE **)(ptr + N)", which is exactly
            # the idiom raw offset access uses.
            ls = stripped.strip()
            if ls == '*/' or re.match(r'^\*\s', ls) or ls.startswith('/*'):
                continue

            # -- Strategy 1: cast_hex ----------------------------------
            for m in CAST_HEX_RE.finditer(stripped):
                var_expr   = m.group(1)
                offset_str = m.group(2)
                offset_int = int(offset_str, 16)
                key = (lineno, offset_str.lower())
                if key in seen:
                    continue
                seen.add(key)
                hits.append({
                    'file':        rel_path,
                    'lineno':      lineno,
                    'line':        raw_line.rstrip(),
                    'var_expr':    var_expr.strip(),
                    'offset':      offset_str,
                    'offset_int':  offset_int,
                    'kind':        'cast_hex',
                    'struct_type': attribute_type(var_expr, type_map),
                    'in_dead':     in_dead,
                })

            # -- Strategy 2: cast_dec ----------------------------------
            for m in CAST_DEC_RE.finditer(stripped):
                var_expr   = m.group(1)
                dec_str    = m.group(2)
                offset_int = int(dec_str)
                if offset_int < min_decimal:
                    continue
                # Deduplicate: if the same offset was already caught as hex skip it.
                key_hex = (lineno, '0x%x' % offset_int)
                key_dec = (lineno, dec_str)
                if key_hex in seen or key_dec in seen:
                    continue
                seen.add(key_dec)
                hits.append({
                    'file':        rel_path,
                    'lineno':      lineno,
                    'line':        raw_line.rstrip(),
                    'var_expr':    var_expr.strip(),
                    'offset':      dec_str,
                    'offset_int':  offset_int,
                    'kind':        'cast_dec',
                    'struct_type': attribute_type(var_expr, type_map),
                    'in_dead':     in_dead,
                })

            # -- Strategy 3: index_bytevar ------------------------------
            # var[N] where var was declared/assigned as a raw byte pointer.
            for m in INDEX_BYTEVAR_RE.finditer(stripped):
                var_name = m.group(1)
                if var_name not in byte_ptr_vars:
                    continue
                offset_str = m.group(2)
                offset_int = int(offset_str, 16) if offset_str.lower().startswith('0x') else int(offset_str)
                key_hex = (lineno, '0x%x' % offset_int)
                key_dec = (lineno, str(offset_int))
                if key_hex in seen or key_dec in seen:
                    continue
                seen.add(key_hex if offset_str.lower().startswith('0x') else key_dec)
                hits.append({
                    'file':        rel_path,
                    'lineno':      lineno,
                    'line':        raw_line.rstrip(),
                    'var_expr':    var_name,
                    'offset':      offset_str,
                    'offset_int':  offset_int,
                    'kind':        'index_bytevar',
                    'struct_type': byte_ptr_vars.get(var_name),
                    'in_dead':     in_dead,
                })

            # -- Strategy 4: cast_bytevar --------------------------------
            # (SomeType *)(byteVar + N) -- cast is on the result, not the base
            # pointer, so cast_hex/cast_dec never see it.  Only counts when
            # byteVar is a known raw byte pointer.
            for m in CAST_BYTEVAR_RE.finditer(stripped):
                var_name = m.group(1)
                if var_name not in byte_ptr_vars:
                    continue
                offset_str = m.group(2)
                offset_int = int(offset_str, 16) if offset_str.lower().startswith('0x') else int(offset_str)
                key_hex = (lineno, '0x%x' % offset_int)
                key_dec = (lineno, str(offset_int))
                if key_hex in seen or key_dec in seen:
                    continue
                seen.add(key_hex if offset_str.lower().startswith('0x') else key_dec)
                hits.append({
                    'file':        rel_path,
                    'lineno':      lineno,
                    'line':        raw_line.rstrip(),
                    'var_expr':    var_name,
                    'offset':      offset_str,
                    'offset_int':  offset_int,
                    'kind':        'cast_bytevar',
                    'struct_type': byte_ptr_vars.get(var_name),
                    'in_dead':     in_dead,
                })

            # -- Strategy 5: broad_hex ---------------------------------
            if include_broad:
                for m in BROAD_HEX_RE.finditer(stripped):
                    offset_str = m.group(1)
                    key = (lineno, offset_str.lower())
                    if key in seen:
                        continue
                    seen.add(key)
                    hits.append({
                        'file':        rel_path,
                        'lineno':      lineno,
                        'line':        raw_line.rstrip(),
                        'var_expr':    '?',
                        'offset':      offset_str,
                        'offset_int':  int(offset_str, 16),
                        'kind':        'broad_hex',
                        'struct_type': None,
                        'in_dead':     in_dead,
                    })

    return hits


# ---------------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------------

def _group_by(hits, key_fn):
    groups = defaultdict(list)
    for h in hits:
        groups[key_fn(h)].append(h)
    return groups


def _struct_key(h):
    return h['struct_type'] or '(unknown)'


def _file_key(h):
    return h['file']


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description='Raw struct-field offset access audit for Cataclysm',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument('--src', default='src',
                        help='Source directory (relative to repo root, default: src)')
    parser.add_argument('--struct', metavar='STRUCT',
                        help='Show all hits attributed to a single struct type')
    parser.add_argument('--lines', action='store_true',
                        help='Print the source line for each hit')
    parser.add_argument('--files', action='store_true',
                        help='Group output by file instead of struct type')
    parser.add_argument('--broad', action='store_true',
                        help='Also include any "+ 0x..." line (broad scan, more false positives)')
    parser.add_argument('--min-decimal', type=int, default=DEFAULT_MIN_DECIMAL,
                        help=f'Minimum decimal offset to flag (default: {DEFAULT_MIN_DECIMAL})')
    parser.add_argument('--include-dead', action='store_true',
                        help='Include hits inside #if 0 dead-code blocks (excluded by default)')
    parser.add_argument('--csv', action='store_true',
                        help='Emit CSV to stdout')
    args = parser.parse_args()

    # Resolve --src relative to CWD first (so the script works correctly from worktrees),
    # then fall back to the repo root derived from the script's own location.
    if os.path.isabs(args.src):
        src_dir = args.src
    else:
        cwd_candidate = os.path.join(os.getcwd(), args.src)
        if os.path.isdir(cwd_candidate):
            src_dir = cwd_candidate
        else:
            repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            src_dir   = os.path.join(repo_root, args.src)

    if not os.path.isdir(src_dir):
        print(f'ERROR: source directory not found: {src_dir}', file=sys.stderr)
        sys.exit(1)

    print(f'Scanning {src_dir} ...', file=sys.stderr)
    all_hits = collect_hits(src_dir,
                            min_decimal=args.min_decimal,
                            include_broad=args.broad,
                            include_dead=args.include_dead)

    cast_hits  = [h for h in all_hits
                  if h['kind'] in ('cast_hex', 'cast_dec', 'index_bytevar', 'cast_bytevar')]
    broad_hits = [h for h in all_hits if h['kind'] == 'broad_hex']

    n_attr    = sum(1 for h in cast_hits if h['struct_type'])
    n_unknown = len(cast_hits) - n_attr

    print(f'  cast-offset hits : {len(cast_hits)} '
          f'({n_attr} attributed, {n_unknown} unknown type)',
          file=sys.stderr)
    if args.broad:
        print(f'  broad "+ 0x" hits: {len(broad_hits)} additional', file=sys.stderr)

    # -- CSV --------------------------------------------------------------
    if args.csv:
        w = csv.writer(sys.stdout, lineterminator='\n')
        w.writerow(['file', 'lineno', 'struct_type', 'var_expr',
                    'offset', 'offset_int', 'kind', 'line'])
        sort_key = lambda h: (h['struct_type'] or '\xff', h['file'], h['lineno'])
        for h in sorted(all_hits, key=sort_key):
            w.writerow([h['file'], h['lineno'],
                        h['struct_type'] or '',
                        h['var_expr'], h['offset'], h['offset_int'],
                        h['kind'], h['line'].strip()])
        return

    # -- Single-struct detail ---------------------------------------------
    if args.struct:
        target  = args.struct
        shits   = [h for h in all_hits if h['struct_type'] == target]
        if not shits:
            print(f"\nNo raw offset accesses found attributed to '{target}'.")
            print("(Type attribution uses declared-pointer types; check --lines for context.)")
            return
        shits.sort(key=lambda h: (h['file'], h['lineno']))
        uniq_offsets = sorted({h['offset_int'] for h in shits})
        print(f"\n{'=' * 72}")
        print(f"  Struct '{target}' -- {len(shits)} raw offset hit(s)")
        print(f"{'=' * 72}")
        print(f"  Unique offsets: {', '.join(hex(o) for o in uniq_offsets)}")
        cur_file = None
        for h in shits:
            if h['file'] != cur_file:
                print()
                print(f"  {h['file']}")
                cur_file = h['file']
            print(f"    {h['lineno']:5d}  [{h['kind']:13s}]  {h['offset']}")
            if args.lines:
                print(f"           {h['line'].strip()}")
        return

    # -- File-grouped output ----------------------------------------------
    if args.files:
        by_file = _group_by(cast_hits, _file_key)
        print(f"\n{'=' * 72}")
        print(f"  RAW OFFSET ACCESS BY FILE  ({len(cast_hits)} cast-offset hits)")
        print(f"{'=' * 72}")
        print(f"  {'File':<62s} {'Hits':>4}")
        print(f"  {'-' * 62}  {'-' * 4}")
        for fpath, fhits in sorted(by_file.items(), key=lambda kv: -len(kv[1])):
            print(f"  {fpath:<62s} {len(fhits):>4d}")
            if args.lines:
                for h in sorted(fhits, key=lambda x: x['lineno']):
                    st = f"[{h['struct_type']}]" if h['struct_type'] else '[?]'
                    print(f"    {h['lineno']:5d}  {h['offset']:>8s}  {st}")
                    print(f"         {h['line'].strip()}")
        return

    # -- Default: struct-grouped summary ---------------------------------
    by_struct = _group_by(cast_hits, _struct_key)
    rows = sorted(
        [{'struct': k, 'hits': v,
          'count': len(v),
          'files': len({h['file'] for h in v}),
          'offsets': sorted({h['offset_int'] for h in v})}
         for k, v in by_struct.items()],
        key=lambda r: (-r['count'], r['struct']),
    )

    print(f"\n{'=' * 72}")
    print(f"  RAW STRUCT-FIELD OFFSET ACCESS AUDIT -- CATACLYSM")
    print(f"{'=' * 72}")
    print(f"  Cast-offset hits : {len(cast_hits)}")
    print(f"  Type-attributed  : {n_attr}  |  Unknown type: {n_unknown}")
    n_real_structs = sum(1 for r in rows if r['struct'] != '(unknown)')
    print(f"  Distinct structs : {n_real_structs}  (plus {len(by_struct.get('(unknown)', []))} unattributed)")
    if args.broad:
        print(f"  Broad hits       : {len(broad_hits)} (shown below)")
    print()
    print(f"  {'Struct':<30s}  {'Hits':>4}  {'Files':>5}  Offsets")
    print(f"  {'-' * 30}  {'-' * 4}  {'-' * 5}  {'-' * 30}")
    for row in rows:
        offs = [hex(o) for o in row['offsets']]
        if len(offs) > 5:
            offs_str = ', '.join(offs[:5]) + f', ... (+{len(offs) - 5} more)'
        else:
            offs_str = ', '.join(offs)
        print(f"  {row['struct']:<30s}  {row['count']:>4d}  {row['files']:>5d}  {offs_str}")

    if args.lines:
        for row in rows:
            print(f"\n  {'-' * 72}")
            print(f"  {row['struct']}  ({row['count']} hits in {row['files']} file(s))")
            cur_file = None
            for h in sorted(row['hits'], key=lambda x: (x['file'], x['lineno'])):
                if h['file'] != cur_file:
                    print(f"\n    {h['file']}")
                    cur_file = h['file']
                print(f"      {h['lineno']:5d}  [{h['kind']:13s}]  {h['offset']}")
                print(f"             {h['line'].strip()}")

    if args.broad and broad_hits:
        by_file_broad = _group_by(broad_hits, _file_key)
        print(f"\n{'-' * 72}")
        print(f"  BROAD '+ 0x' hits  ({len(broad_hits)} total, may include false positives)")
        print(f"{'-' * 72}")
        for fpath, fhits in sorted(by_file_broad.items(), key=lambda kv: -len(kv[1])):
            print(f"  {fpath}  ({len(fhits)})")
            if args.lines:
                for h in sorted(fhits, key=lambda x: x['lineno']):
                    print(f"    {h['lineno']:5d}  {h['offset']}")
                    print(f"         {h['line'].strip()}")


if __name__ == '__main__':
    main()

