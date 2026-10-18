"""Seed the parity-audit work queue from IDA + source identity (not BinDiff).

Regenerates the carried-over-HW1-function work queue that the
`parity-audit-function` skill consumes, and writes it into the checked-in
`parity_audit.sqlite` database at the repo root.

WHY NOT BINDIFF
---------------
This seeder used to derive the queue from a two-hop BinDiff chain
(debug -> retail -> cata) and use BinDiff similarity as a divergence signal.
That was abandoned because BinDiff is unreliable here:

  * It mispairs constantly across clusters of near-identical wrappers, so the
    queue was polluted and the skill wasted time auditing source A against
    binary B.
  * It is LOSSY: of the carried-over functions, BinDiff failed to produce any
    verified pairing for the large majority (e.g. RemoveShipFromBeingTargeted
    at 0x4453C0 was a real divergence that BinDiff never paired, so it never
    entered the queue and was only found by hand). Using BinDiff as the gate
    creates blind spots that are exactly where latent HW1 bugs hide.

IDENTITY MODEL (the reliable one)
---------------------------------
A function is in scope iff ALL THREE are true:

  1. It is DEFINED in a Cataclysm `.c` file that the vcxproj actually compiles
     (`build_source_inventory`)            -> we have source to audit.
  2. It is NAMED in `.ida-snapshot.json`, the authoritative address->name map of
     the current, heavily-named Cataclysm IDB (`load_snapshot`)
                                            -> we have a concrete cata address to
                                               decompile, and identity is certain
                                               (the snapshot name == the source
                                               name, modulo CRT decoration).
  3. A function of the same name is DEFINED in the original HW1 C source tree
     (`build_hw1_inventory`)               -> it is genuinely carried over, not a
                                               Cataclysm-only addition. This grep
                                               of HW1's own source is what excludes
                                               Beast/Acolyte/Auth/etc. additions
                                               without trusting BinDiff at all.

The cata address comes straight from the snapshot, so there is no pairing step
and no mispairing. Anything carried over but not yet named in IDA is honestly
out of scope until it is named (snapshot freshness bounds coverage).

BINDIFF IS NOW A HINT ONLY
--------------------------
If the optional retail->cata BinDiff file is present, its similarity is attached
to a row ONLY when BinDiff's own name agrees with the snapshot name at that
address. It is used purely to order the queue (provably-changed first), never to
include or exclude a function. Missing/!=name BinDiff data simply leaves
`similarity` NULL.

The script is idempotent: re-running refreshes queue rows (similarity, tier,
hw1 source ref, current cata name) without clobbering completed `audit_results`.

Intentional policy exclusions also apply. In particular, software/glide-only
renderer-management functions that this project has deliberately removed from
scope must stay out of the queue even if they still exist in carried-over HW1
source form.
"""
import argparse
import json
import os
import re
import sqlite3
import time
import xml.etree.ElementTree as ET

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

DEFAULT_RETAIL_CATA = os.environ.get("CATACLYSM_RETAIL_CATA_BINDIFF")
DEFAULT_DB = os.path.join(REPO_ROOT, "parity_audit.sqlite")
DEFAULT_VCXPROJ = os.path.join(REPO_ROOT, "Cataclysm.vcxproj")
DEFAULT_SNAPSHOT = os.path.join(REPO_ROOT, ".ida-snapshot.json")

# Matches a C/C++ function DEFINITION (a prototype/return spec followed by `(...) {`).
FUNC_DEF_RE = re.compile(
    r'(?m)^\s*(static\s+)?(?:[A-Za-z_][\w]*[\t\r\n *]+)+([A-Za-z_]\w*)\s*\([^;{}]*\)\s*{'
)

# Names that are not real carried-over game functions (compiler internals, STL, CRT,
# HW1-only mission FSM handlers).
SKIP_PATTERNS = [
    re.compile(r'^\$'),
    re.compile(r'^std::'),
    re.compile(r'^operator'),
    re.compile(r'^\?\?'),
    re.compile(r'::`scalar deleting'),
    re.compile(r'::`vector deleting'),
    re.compile(r'^_CRT'),
    re.compile(r'^__'),
    re.compile(r'^_allmul'),
    re.compile(r'^_alldiv'),
    re.compile(r'^_aulldiv'),
    re.compile(r'^_aullrem'),
    re.compile(r'^_ftol'),
    re.compile(r'^\w+::\w+'),
    re.compile(r'^Watch_Mission'),
    re.compile(r'^Init_Mission'),
    re.compile(r'^Watch_Tutorial'),
    re.compile(r'^Init_Tutorial'),
]

# Intentional non-parity policy exclusions. These functions only manage the old
# software/glide renderer paths that the Cataclysm tree intentionally no longer
# supports, so auditing them would only pressure the queue toward restoring
# removed renderer behavior.
EXCLUDED_FUNCTIONS = {
    "mainSetupSoftware",
    "mainRestoreSoftware",
}


def should_skip(name):
    if name in EXCLUDED_FUNCTIONS:
        return True
    for p in SKIP_PATTERNS:
        if p.search(name):
            return True
    if len(name) < 3:
        return True
    if any(c in name for c in "`[]{}()<>:;,!@#$%^&*+= '\"\\"):
        return True
    return False


def tier_for(source_lines):
    """Priority hint based on source size (BinDiff-independent).

    Bigger carried-over functions have more control-flow / data-access surface for a
    latent HW1 divergence to hide in, and a higher blast radius when one does. BinDiff
    similarity is too noisy here to drive ordering (it scores clearly-intact functions
    like LoadUniverse at ~0.003), so it is stored as an annotation only.

    A: >= 120 source lines (large)   B: 30..120 (medium)   C: < 30 (small)
    """
    if source_lines is None:
        return "B"
    if source_lines >= 120:
        return "A"
    if source_lines >= 30:
        return "B"
    return "C"


def norm_name(name):
    """Normalize for identity comparison: drop CRT decoration (leading underscores)."""
    if not name:
        return ""
    return name.lstrip("_").lower()


def load_snapshot(snapshot_path):
    """Return (addr_int -> name, normalized_name -> addr_int) from .ida-snapshot.json."""
    with open(snapshot_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    addr_to_name = {}
    name_to_addr = {}
    for fn in data:
        name = fn.get("name")
        addr = fn.get("address")
        if name is None or addr is None:
            continue
        addr_i = int(addr, 16) if isinstance(addr, str) else int(addr)
        addr_to_name[addr_i] = name
        if not name.startswith("sub_"):
            name_to_addr.setdefault(norm_name(name), addr_i)
    return addr_to_name, name_to_addr


def build_hw1_inventory(hw1_src_root):
    """Map norm_name -> 'relpath:line' for every function DEFINED in the HW1 C source.

    Membership in this set is the authority for "carried over from HW1" and is what
    excludes Cataclysm-only additions (Beast/Acolyte/Auth/etc.) without any BinDiff.
    """
    inv = {}
    for root, _dirs, files in os.walk(hw1_src_root):
        for fn in files:
            if not fn.lower().endswith((".c", ".cpp")):
                continue
            path = os.path.join(root, fn)
            try:
                text = open(path, encoding="latin-1").read()
            except OSError:
                continue
            for m in FUNC_DEF_RE.finditer(text):
                key = norm_name(m.group(2))
                if key and key not in inv:
                    line = text.count("\n", 0, m.start()) + 1
                    rel = os.path.relpath(path, hw1_src_root)
                    inv[key] = f"{rel}:{line}"
    return inv


def compiled_c_files(vcxproj_path):
    tree = ET.parse(vcxproj_path)
    root = tree.getroot()
    ns = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
    files = []
    for node in root.findall(".//msb:ClCompile", ns):
        include = node.get("Include")
        if not include:
            continue
        path = os.path.normpath(os.path.join(REPO_ROOT, include))
        if path.lower().endswith(".c") and os.path.exists(path):
            files.append(path)
    return files


def build_source_inventory(vcxproj_path):
    inventory = {}
    for path in compiled_c_files(vcxproj_path):
        with open(path, "r", encoding="latin-1") as f:
            text = f.read()
        matches = list(FUNC_DEF_RE.finditer(text))
        for i, match in enumerate(matches):
            name = match.group(2)
            start = text.count("\n", 0, match.start()) + 1
            next_start = matches[i + 1].start() if i + 1 < len(matches) else len(text)
            end = text.count("\n", 0, next_start)
            inventory[name] = {
                "path": path,
                "line_start": start,
                "line_end": max(start, end),
                "line_count": max(1, end - start + 1),
                "is_static": bool(match.group(1)),
            }
    return inventory


SCHEMA = """
CREATE TABLE IF NOT EXISTS audit_queue (
    cata_address      TEXT PRIMARY KEY,   -- "0x........" Cataclysm function address (decompile target)
    hw1_name          TEXT NOT NULL,      -- function name (== source name == snapshot name, CRT-decoration stripped)
    cata_name         TEXT,               -- current IDA name at cata_address (authoritative snapshot)
    similarity        REAL,               -- OPTIONAL retail->cata BinDiff hint (NULL unless BinDiff name agrees); ordering only
    sim_debug_retail  REAL,               -- unused (legacy column; always NULL under the IDA+source identity model)
    priority_tier     TEXT,               -- 'A' large(>=120 lines) | 'B' medium | 'C' small: source-size divergence-surface hint
    pairing           TEXT NOT NULL DEFAULT 'verified', -- always 'verified': identity is C-def == snapshot-name == HW1-def
    source_lines      INTEGER,            -- size of the Cataclysm source definition (drives priority_tier/order)
    hw1_src_ref       TEXT,               -- 'relpath:line' of the carried-over function in the HW1 source tree
    status            TEXT NOT NULL DEFAULT 'pending', -- pending|in_progress|match|divergent|indeterminate|mispaired|skipped
    last_result_id    INTEGER,            -- FK -> audit_results.id (latest audit)
    seeded_at         TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS audit_results (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    cata_address       TEXT NOT NULL,
    hw1_name           TEXT,
    source_ref         TEXT,              -- file:line of the audited source function
    verdict            TEXT NOT NULL,     -- MATCH | DIVERGENT | INDETERMINATE
    control_flow       TEXT,              -- MATCH | DIVERGENT | INDETERMINATE
    data_access        TEXT,              -- MATCH | DIVERGENT | INDETERMINATE
    semantics          TEXT,              -- MATCH | DIVERGENT | INDETERMINATE
    constructs_total   INTEGER,           -- # constructs enumerated from IDA
    constructs_matched INTEGER,
    divergences        TEXT,              -- summary of DIVERGENT/INDETERMINATE findings
    evidence           TEXT,              -- where the correspondence table / notes live
    auditor            TEXT,              -- agent / session id
    commit_sha         TEXT,
    audited_at         TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS audit_constructs (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    result_id   INTEGER NOT NULL,        -- FK -> audit_results.id
    kind        TEXT,                    -- conditional|loop|switch|return|goto|call|field|global|const
    retail_ref  TEXT,                    -- IDA address / basic-block ref (disassembly for offsets)
    source_ref  TEXT,                    -- file:line
    verdict     TEXT,                    -- MATCH | DIVERGENT | INDETERMINATE
    note        TEXT
);

CREATE INDEX IF NOT EXISTS idx_results_addr ON audit_results(cata_address);
CREATE INDEX IF NOT EXISTS idx_constructs_result ON audit_constructs(result_id);
CREATE INDEX IF NOT EXISTS idx_queue_status ON audit_queue(status);
"""


def build_universe(source_inventory, hw1_inventory, addr_to_name, name_to_addr,
                   retail_cata_path=None):
    """Build the audit queue from the IDA+source identity model.

    Universe = vcxproj-compiled C definition  (source_inventory)
             INTERSECT named in .ida-snapshot.json  (name_to_addr)
             INTERSECT defined in the HW1 C source  (hw1_inventory).

    BinDiff (retail_cata_path) is optional and contributes only an advisory
    `similarity` used for ordering; it never includes or excludes a function.
    """
    # Optional BinDiff sim, attached only where BinDiff's own name agrees with the
    # authoritative snapshot name at that cata address (otherwise it is mispaired noise).
    cata_sim = {}
    if retail_cata_path and os.path.exists(retail_cata_path):
        bd2 = sqlite3.connect(retail_cata_path)
        for cata_addr, cata_name, sim in bd2.execute(
            "SELECT address2, name2, similarity FROM function"
        ):
            live = addr_to_name.get(cata_addr)
            if (live and not live.startswith("sub_") and cata_name
                    and norm_name(cata_name) == norm_name(live)):
                if cata_addr not in cata_sim or sim > cata_sim[cata_addr]:
                    cata_sim[cata_addr] = sim
        bd2.close()

    rows = []
    filtered = {
        "skipped_name": 0,
        "intentional_renderer_removal": 0,
        "not_named_in_ida": 0,
        "cataclysm_only": 0,
    }
    by_addr = {}
    for name, meta in source_inventory.items():
        if name in EXCLUDED_FUNCTIONS:
            filtered["intentional_renderer_removal"] += 1
            continue
        if should_skip(name):
            filtered["skipped_name"] += 1
            continue
        key = norm_name(name)
        addr = name_to_addr.get(key)
        if addr is None:
            filtered["not_named_in_ida"] += 1  # carried over but not yet named in IDA -> out of scope
            continue
        hw1_ref = hw1_inventory.get(key)
        if hw1_ref is None:
            filtered["cataclysm_only"] += 1     # no HW1 C definition -> Cataclysm-only addition
            continue
        lines = meta.get("line_count") if meta else None
        row = {
            "hw1_name": name,
            "cata_address": f"0x{addr:x}",
            "cata_name": addr_to_name.get(addr),
            "similarity": cata_sim.get(addr),   # advisory annotation only
            "sim_debug_retail": None,
            "source_lines": lines,
            "tier": tier_for(lines),
            "pairing": "verified",   # identity certain by construction
            "hw1_src_ref": hw1_ref,
        }
        # Two source names can normalize to the same address; keep the larger one.
        prev = by_addr.get(row["cata_address"])
        if prev is None or (row["source_lines"] or 0) > (prev["source_lines"] or 0):
            by_addr[row["cata_address"]] = row

    rows = sorted(
        by_addr.values(),
        key=lambda r: (r["tier"], -(r["source_lines"] or 0), r["hw1_name"]),
    )
    return rows, filtered


def seed(db_path, rows, prune_pending=False):
    now = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    db = sqlite3.connect(db_path)
    db.executescript(SCHEMA)
    inserted = updated = pruned = 0
    row_addrs = {r["cata_address"] for r in rows}
    for r in rows:
        existing = db.execute(
            "SELECT status FROM audit_queue WHERE cata_address = ?", (r["cata_address"],)
        ).fetchone()
        if existing is None:
            db.execute(
                "INSERT INTO audit_queue "
                "(cata_address, hw1_name, cata_name, similarity, sim_debug_retail, "
                " priority_tier, pairing, source_lines, hw1_src_ref, status, seeded_at) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 'pending', ?)",
                (r["cata_address"], r["hw1_name"], r["cata_name"], r["similarity"],
                 r["sim_debug_retail"], r["tier"], r["pairing"], r["source_lines"],
                 r["hw1_src_ref"], now),
            )
            inserted += 1
        else:
            # Refresh signal fields only; never clobber audit status / results.
            db.execute(
                "UPDATE audit_queue SET hw1_name=?, cata_name=?, similarity=?, "
                "sim_debug_retail=?, priority_tier=?, pairing=?, source_lines=?, "
                "hw1_src_ref=? WHERE cata_address=?",
                (r["hw1_name"], r["cata_name"], r["similarity"],
                 r["sim_debug_retail"], r["tier"], r["pairing"], r["source_lines"],
                 r["hw1_src_ref"], r["cata_address"]),
            )
            updated += 1
    if prune_pending:
        pruned = db.execute(
            "UPDATE audit_queue SET status='skipped' "
            "WHERE status='pending' AND cata_address NOT IN ({})".format(
                ",".join("?" for _ in row_addrs) if row_addrs else "''"
            ),
            tuple(sorted(row_addrs)),
        ).rowcount
    db.commit()
    return inserted, updated, pruned, db


def validate(db_path):
    """Run QA checks that enforce the skill's strictness rules. Returns issue count."""
    db = sqlite3.connect(db_path)
    db.row_factory = sqlite3.Row
    issues = 0

    def report(label, rows, fmt):
        nonlocal issues
        if rows:
            issues += len(rows)
            print(f"\n[{len(rows)}] {label}")
            for r in rows[:15]:
                print("   " + fmt(r))
            if len(rows) > 15:
                print(f"   ... and {len(rows) - 15} more")

    # 1. MATCH recorded without a per-construct correspondence table.
    report(
        "MATCH results with zero audit_constructs rows (correspondence table skipped)",
        db.execute(
            "SELECT r.id, r.hw1_name FROM audit_results r WHERE r.verdict='MATCH' "
            "AND NOT EXISTS (SELECT 1 FROM audit_constructs ac WHERE ac.result_id=r.id)"
        ).fetchall(),
        lambda r: f"result {r['id']}: {r['hw1_name']}",
    )
    # 2. MATCH where enumerated/matched construct counts do not reconcile.
    report(
        "MATCH results where constructs_total/matched do not reconcile or are NULL/0",
        db.execute(
            "SELECT id, hw1_name, constructs_total t, constructs_matched m FROM audit_results "
            "WHERE verdict='MATCH' AND (constructs_total IS NULL OR constructs_total=0 "
            "OR constructs_matched IS NULL OR constructs_matched<>constructs_total)"
        ).fetchall(),
        lambda r: f"result {r['id']}: {r['hw1_name']} total={r['t']} matched={r['m']}",
    )
    # 3. data_access=MATCH but no disassembly offset evidence anywhere in the result.
    report(
        "data_access=MATCH with no cited '+0x' disassembly offset (Step 5 unverified)",
        db.execute(
            "SELECT r.id, r.hw1_name FROM audit_results r WHERE r.data_access='MATCH' "
            "AND r.evidence NOT LIKE '%+0x%' AND coalesce(r.divergences,'') NOT LIKE '%+0x%' "
            "AND NOT EXISTS (SELECT 1 FROM audit_constructs ac WHERE ac.result_id=r.id "
            "AND ac.kind IN ('field','global') AND ac.retail_ref LIKE '%0x%')"
        ).fetchall(),
        lambda r: f"result {r['id']}: {r['hw1_name']}",
    )
    # 4. Mispairing-shaped DIVERGENT: reasoning says the retail func "is <other>".
    report(
        "DIVERGENT results whose reasoning looks like a mispairing, not a source bug",
        db.execute(
            "SELECT id, hw1_name FROM audit_results WHERE verdict='DIVERGENT' AND "
            "(coalesce(divergences,'') LIKE '% is %' OR coalesce(evidence,'') LIKE '%pairing%' "
            "OR coalesce(divergences,'') LIKE '%pairing%')"
        ).fetchall(),
        lambda r: f"result {r['id']}: {r['hw1_name']}",
    )
    # 5. Closed queue rows whose status disagrees with their latest result verdict.
    report(
        "queue rows whose status does not match their latest result verdict",
        db.execute(
            "SELECT q.cata_address, q.hw1_name, q.status, r.verdict FROM audit_queue q "
            "JOIN audit_results r ON r.id=q.last_result_id "
            "WHERE lower(q.status)<>lower(r.verdict) "
            "AND q.status NOT IN ('pending','in_progress','skipped','mispaired')"
        ).fetchall(),
        lambda r: f"{r['cata_address']} {r['hw1_name']}: status={r['status']} verdict={r['verdict']}",
    )

    if issues == 0:
        print("validate: OK — no strictness violations found.")
    else:
        print(f"\nvalidate: {issues} issue(s) found. These rows are not valid passes.")
    return issues


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--retail-cata", default=DEFAULT_RETAIL_CATA,
                    help="Optional retail->Cataclysm BinDiff ordering hint "
                         "(defaults to CATACLYSM_RETAIL_CATA_BINDIFF)")
    ap.add_argument("--no-bindiff", action="store_true",
                    help="ignore BinDiff entirely (similarity stays NULL, tier defaults to B)")
    ap.add_argument("--db", default=DEFAULT_DB)
    ap.add_argument("--vcxproj", default=DEFAULT_VCXPROJ)
    ap.add_argument("--snapshot", default=DEFAULT_SNAPSHOT,
                    help="IDA address->name snapshot used as the identity authority")
    ap.add_argument("--hw1-src", default=None,
                    help="original HW1 C source tree; its function definitions are the carried-over authority")
    ap.add_argument("--dry-run", action="store_true", help="report counts, do not write")
    ap.add_argument("--prune-pending", action="store_true",
                    help="mark currently-pending rows that no longer qualify under the seed policy as skipped")
    ap.add_argument("--reset", action="store_true",
                    help="drop all queue/results/constructs and start over before seeding")
    ap.add_argument("--validate", action="store_true",
                    help="run strictness QA checks against the DB and exit")
    args = ap.parse_args()

    if args.validate:
        raise SystemExit(1 if validate(args.db) else 0)

    if args.hw1_src is None:
        hw_root = os.environ.get("HW_ROOT")
        if not hw_root:
            raise SystemExit("Set HW_ROOT to the original Homeworld checkout root or pass --hw1-src.")
        args.hw1_src = os.path.join(hw_root, "src")

    for p in (args.vcxproj, args.snapshot, args.hw1_src):
        if not os.path.exists(p):
            raise SystemExit(f"Required input missing: {p}")

    if args.reset:
        db = sqlite3.connect(args.db)
        db.executescript(
            "DROP TABLE IF EXISTS audit_constructs;"
            "DROP TABLE IF EXISTS audit_results;"
            "DROP TABLE IF EXISTS audit_queue;"
            "DELETE FROM sqlite_sequence WHERE name IN ('audit_results','audit_constructs');"
        )
        db.commit()
        db.close()
        print("reset: dropped audit_queue / audit_results / audit_constructs")

    addr_to_name, name_to_addr = load_snapshot(args.snapshot)
    source_inventory = build_source_inventory(args.vcxproj)
    hw1_inventory = build_hw1_inventory(args.hw1_src)
    retail_cata = None if args.no_bindiff else args.retail_cata
    rows, filtered = build_universe(source_inventory, hw1_inventory,
                                    addr_to_name, name_to_addr, retail_cata)
    tiers = {"A": 0, "B": 0, "C": 0}
    for r in rows:
        tiers[r["tier"]] += 1
    print(f"Snapshot named funcs: {len(name_to_addr)}  vcxproj C defs: {len(source_inventory)}  "
          f"HW1 src defs: {len(hw1_inventory)}")
    print(f"Carried-over functions in scope: {len(rows)}")
    print(f"  Tier A (large >=120 lines): {tiers['A']}   "
          f"Tier B (medium 30-120): {tiers['B']}   "
          f"Tier C (small <30): {tiers['C']}")
    print("  Excluded:")
    print(f"    non-function / decorated names skipped:          {filtered['skipped_name']}")
    print(f"    intentional sw/glide renderer removals:         {filtered['intentional_renderer_removal']}")
    print(f"    carried over but not yet named in IDA:           {filtered['not_named_in_ida']}")
    print(f"    Cataclysm-only (no HW1 C definition):            {filtered['cataclysm_only']}")

    if args.dry_run:
        print("\n--dry-run: no database written. Sample (Tier A, largest first):")
        for r in [x for x in rows if x["tier"] == "A"][:15]:
            sim = f"{r['similarity']:.3f}" if r["similarity"] is not None else "  -  "
            print(f"  {r['cata_address']:>10}  {r['source_lines']:>4}L  sim={sim}  "
                  f"{r['hw1_name']:<32} {r['hw1_src_ref']}")
        return

    inserted, updated, pruned, db = seed(args.db, rows, prune_pending=args.prune_pending)
    pending = db.execute("SELECT COUNT(*) FROM audit_queue WHERE status='pending'").fetchone()[0]
    done = db.execute("SELECT COUNT(*) FROM audit_queue WHERE status NOT IN ('pending','in_progress')").fetchone()[0]
    print(f"\nSeeded {args.db}")
    print(f"  inserted: {inserted}, refreshed: {updated}")
    if args.prune_pending:
        print(f"  pruned pending -> skipped: {pruned}")
    print(f"  queue: {pending} pending, {done} audited/closed")


if __name__ == "__main__":
    main()
