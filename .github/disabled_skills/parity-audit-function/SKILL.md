---
name: parity-audit-function
description: Audit a carried-over HW1 function against the retail Cataclysm binary in IDA. Confirm identity against the IDA snapshot (source name == snapshot name == HW1 definition), enumerate every retail control-flow branch, call, and data access, map each to the current source bidirectionally, verify struct offsets against raw disassembly, and record a three-valued verdict with a mandatory per-construct correspondence table in parity_audit.sqlite. The queue is seeded from C-source + IDA-snapshot + HW1-source identity (not BinDiff, which is lossy and mispairs). Designed to catch latent, unreconciled HW1 divergences that superficial "looks the same" reviews miss.
---

# Parity audit: control flow & data access vs the retail binary

Use this skill to **prove** that a function in the current source tree behaves
identically to its counterpart in the shipped Cataclysm retail binary — not to
decide whether it "looks reasonable."

The target population is **latent, unreconciled HW1 code**: functions copied from
Homeworld 1 source on the assumption they were unchanged, where Cataclysm in fact
made small modifications that have not yet been reconciled. These divergences
compile cleanly, run without crashing, and surface much later as gameplay bugs.

This skill exists because a looser version of it failed on another project: many
divergent functions "passed" review and were caught later in debugging. The fix is
structural — make a superficial pass require *more* effort than a real one, and
make "I could not find a difference" a non-passing result.

## First warning: never delete debug-only source code

The shipped Cataclysm binary is a **release** build. `dbgAssert*`, `dbgMessage*`,
`dbgWarning*`, `dbgFatal*`, `logfileLog`, `if (logEnable)`, `#if HW_BUILD_FOR_DEBUGGING`,
`#ifndef HW_Release`, and `CATACLYSM_DIAG` blocks can be absent from retail **by
construction**. Their absence is **never** evidence of a divergence and **never** a
reason to delete them from source. When retail lacks a debug-only block, classify
that source construct as an expected release-build difference and leave it intact.

## Second warning: do not reintroduce intentionally removed software/glide paths

Some carried-over HW1 functions exist only to manage the old software or Glide
renderer paths. In this repository those paths were removed intentionally. Do
**not** treat their absence from current behavior as a parity bug and do **not**
reconstruct them just because retail still has them.

When a queued function is strictly software/glide-only renderer management
(currently `mainSetupSoftware` / `mainRestoreSoftware`, and any future rows of
the same kind), mark it `skipped` / keep it out of scope rather than auditing it
as a live parity target.

## Third warning: preserve approved Mode B automation extensions

This repository intentionally carries **non-retail Mode B recording/playback and
background-automation support** in parts of `demo.c` and the Win32 utility/input
path (`utility.c`, `mouse.c`, `main.c`, and nearby helpers). These branches are
approved project extensions, not latent HW1 parity bugs.

When a queued function in those files differs from retail **only** because of:

- Mode B demo recording/playback plumbing,
- game-only cursor/input forwarding used to avoid moving the desktop cursor, or
- hidden/background automation behavior needed by `run_game.ps1`,

do **not** record that delta as a parity divergence and do **not** reconcile it
away. Audit the underlying retail path normally, but classify the extension-only
source constructs as expected repository-specific differences and leave them intact.

If a demo/utility function also has a **separate** retail-backed gameplay or UI
divergence unrelated to the approved Mode B / automation extension, audit and
reconcile that real divergence as usual.

## Core principles (read before every audit)

1. **The retail binary is the only authority.** HW1 source is the *suspect*, not
   the standard. `cataclysm.big` is an investigation aid, not an approval basis.
2. **IDA-first.** Enumerate the retail function's constructs first, then find each
   one in source. Never start from source and skim IDA for confirmation.
3. **Bidirectional coverage.** Every retail construct must map to source, AND every
   source construct must map to retail. An unmatched item on either side is a finding.
4. **Three-valued verdict.** Each function and each region is `MATCH`,
   `DIVERGENT`, or `INDETERMINATE`. `INDETERMINATE` **never** counts as a pass.
   "No difference found" is not "proven identical."
5. **Compiling and not crashing are not evidence.** They prove nothing about parity.
6. **Loud failure over hidden divergence.** A confirmed `DIVERGENT` is a success of
   this skill, not a problem to paper over.
7. **No subagents for the judgment.** Keep the analysis in the calling agent's
   context (a subagent may fetch IDA decompilation/disassembly, but must not render
   the verdict). Mirrors `parity-review-gate`.
8. **The retail binary is a release build.** Debug-only and assert-only code is
   absent from it by construction. That absence is never a divergence and never a
   reason to delete debug code from source (see Step 4).
9. **Proven divergences are immediate reconciliation work.** Once a function is
   recorded `DIVERGENT`, stop queue-draining and reconcile that exact retail-backed
   delta before moving on, unless the user explicitly asks to leave it open or the
   fix is blocked by missing evidence outside the function.

## When to use

- Auditing a queued carried-over function (see Targeting) for parity.
- A bug points at a function that came from HW1 source and may not be reconciled.
- Before trusting any HW1-imported function on a hot path.

Do **not** use this skill to "review the diff I just wrote" — that is
`parity-review-gate`. This skill audits an *existing* function against the binary
regardless of whether you are changing it.

## Targeting (what to audit, in what order)

The work queue lives in the checked-in database `parity_audit.sqlite` at the repo
root. It is seeded from an **IDA + source identity model**, not BinDiff. A function
is in scope iff all three hold:

1. it is **defined in a `.c` file the vcxproj compiles** (we have source to audit),
2. it is **named in `.ida-snapshot.json`** (we have a concrete cata address and the
   name proves identity), and
3. a function of the same name is **defined in the original HW1 C source tree**
   (`$env:HW_ROOT\src`) — i.e. it is genuinely carried over.

- **In scope:** every carried-over function meeting 1–3. There is no similarity
  gate, so there are **no BinDiff blind spots** (e.g. `RemoveShipFromBeingTargeted`
  at `0x4453C0`, which BinDiff never paired, is in the queue).
- **Out of scope (excluded by the seed):**
  - Cataclysm-only functions with **no HW1 C definition** (Beast/Acolyte/Auth/etc.
    additions). Re-audit these only if the *original decompilation itself* is
    suspected wrong — a different task.
  - Functions carried over but **not yet named in IDA** — out of scope until named
    (snapshot freshness bounds coverage).
  - Functions whose sole purpose is the intentionally removed software/glide
    renderer path. These should be skipped, not reconstructed for parity.
- **Priority order:** `priority_tier` is a **source-size** hint (A ≥ 120 lines, B
  30–120, C < 30), not a BinDiff score. Bigger functions have more control-flow and
  data-access surface for a latent divergence to hide in and a larger blast radius,
  so audit Tier A first. `similarity` (BinDiff retail→cata) is stored as an
  **advisory annotation only** — it is noisy here (it scores clearly-intact
  functions like `LoadUniverse` at ~0.003) and must never drive a verdict.

Pick the next function:

```sql
SELECT cata_address, hw1_name, priority_tier, source_lines, hw1_src_ref, similarity
FROM audit_queue
WHERE status = 'pending'
ORDER BY priority_tier, source_lines DESC
LIMIT 1;
```

`hw1_src_ref` points at the carried-over function in the HW1 tree
(`relpath:line` under `$env:HW_ROOT\src`) — read it to see the code our source
descends from. `pairing` is always `verified`: identity is certain by construction
(source name == snapshot name == HW1 definition). You still confirm it in Step 0.

If the function has **no counterpart in our source tree** at audit time (e.g. it was
since removed), mark it `skipped` and move on:

```sql
UPDATE audit_queue SET status = 'skipped' WHERE cata_address = '0x...';
```

### (Re)seeding the queue

If the queue is empty or stale (e.g. after renaming more functions in IDA and
refreshing `.ida-snapshot.json`), regenerate it:

```powershell
python src\Scripts\seed_parity_audit.py            # seed/refresh parity_audit.sqlite
python src\Scripts\seed_parity_audit.py --dry-run  # preview counts, write nothing
python src\Scripts\seed_parity_audit.py --reset    # drop all queue/results/constructs, start over
python src\Scripts\seed_parity_audit.py --validate # run the strictness QA checks (see below)
python src\Scripts\seed_parity_audit.py --no-bindiff # ignore BinDiff entirely (similarity stays NULL)
```

The seeder is idempotent: it refreshes tier/size/name/hw1_src_ref/similarity on
existing rows but never clobbers `status` or completed `audit_results`. Required
inputs: `Cataclysm.vcxproj` (source inventory), `.ida-snapshot.json` (address→name
identity authority), and `$env:HW_ROOT\src` (the carried-over authority).
The retail-to-Cataclysm BinDiff is **optional**. Supply it with
`CATACLYSM_RETAIL_CATA_BINDIFF` or `--retail-cata`; it only annotates `similarity`
where its own name agrees with the snapshot. Commit `parity_audit.sqlite` so audits
are tracked across sessions.

## Procedure

Mark the function in progress before starting:

```sql
UPDATE audit_queue SET status = 'in_progress' WHERE cata_address = '0x...';
```

### Step 0 — Confirm identity (do this before anything else)

Identity is established by construction (snapshot name == source name == HW1
definition), but **verify it yourself** every time — the snapshot can be stale and a
name can be wrong:

1. Get the **live IDA name** of `cata_address` (`get_function_by_address`).
2. Confirm it resolves to the same function as `hw1_name` — same name modulo CRT
   decoration, or clearly the same routine by its strings/callees/role.

If the live cata function is actually a **different** routine (stale snapshot), stop
and record it as a mispairing — **not** a divergence:

```sql
UPDATE audit_queue SET status = 'mispaired' WHERE cata_address = '0x...';
-- and an audit_results row with verdict='INDETERMINATE', divergences noting the mispairing.
```

Only proceed to Step 1 once identity is confirmed.

### Step 1 — Get both retail views

- `decompile_function({address})` — Hex-Rays pseudocode (control flow, calls,
  constants, strings).
- `disassemble_function({start_address})` — raw disassembly. **Required** for every
  data-access check; pseudocode struct offsets are the *least* trustworthy output
  and IDA is frequently out of sync with the C struct definitions.

Also locate the current source: the `.c` definition of `hw1_name` in
`$env:HWC_ROOT\src`. Record it as `source_ref` (file:line). If the callee names
in IDA are still `sub_XXXXXX`, resolve them with `get_function_by_address`; an
unresolved callee whose identity matters makes the relevant construct
`INDETERMINATE`.

### Step 2 — Enumerate retail constructs (IDA-first)

From the retail views, list **every** construct, in order. Do not summarize. Each
construct becomes both a row in your working correspondence table and, at the end,
a persisted `audit_constructs` row (see Recording — this table is mandatory, not
optional):

| # | Kind | Retail ref (addr/block) | Detail |
|---|------|-------------------------|--------|

`Kind` ∈ `conditional` · `loop` · `switch/case` · `return` · `goto` · `call` ·
`field` (struct member read/write) · `global` · `const` (magic number / enum).

For large functions (> ~60–80 lines or multiple distinct blocks), **split into
regions** (one per basic-block cluster / logical section) and audit each region as
its own unit. The function passes only if every region passes.

### Step 3 — Map each retail construct to source (forward pass)

For every row, find the source counterpart and assign a verdict:

| ... | Source ref (file:line) | Verdict | Evidence |

- `MATCH` only with concrete evidence (cited line, and for offsets the disassembly
  `[reg+0xNN]` matched against the source field's byte offset).
- `INDETERMINATE` if you cannot confirm it (unknown type, unresolved callee,
  offset you could not pin down). **Do not upgrade to MATCH by assumption.**
- `DIVERGENT` if the source does the wrong thing or the construct is missing.

Paste the **raw IDA block** beside the source for any non-trivial or suspicious
region. Paraphrasing is where divergence hides. For trivial regions, a line-range
reference is enough (keep token cost bounded, per the cat2 guidance).

### Step 4 — Reverse pass (catch latent HW1 code)

Walk the **source** function and confirm every branch, call, field access, global,
and constant it contains is justified by a retail construct from Step 2. Anything in
source with **no** retail counterpart is the classic latent-HW1 signature
(Cataclysm removed or changed it) — mark it `DIVERGENT` (or `INDETERMINATE` if you
genuinely cannot tell).

**Exception — release-build-only code (do NOT reconcile this away).** The shipped
Cataclysm binary is a **release** build, so debug-only and assert-only code is
absent *by construction*. Its absence from the decompilation is **not** evidence of
a Cataclysm source change. When a source construct with no retail counterpart is
debug/release-stripped code, classify it `MATCH` (expected release-build difference)
and **leave the source exactly as it is** — never delete it to "make the diff match."
Treat the following as release-stripped, not divergent:

- debug logging: `dbgMessage`, `dbgMessagef`, `dbgWarning`, `dbgWarningf`,
  `dbgFatal`, `dbgNonFatal` (and `dbgMessagef`-style trace calls)
- assertions: `dbgAssert`, `dbgAssertOrIgnore`, `dbgAssertAlwaysDo` — these expand
  to `((void)0)` in release
- logging guarded by debug/log flags (e.g. `logfileLog`, `if (logEnable) { ... }`)
- code inside `#if HW_BUILD_FOR_DEBUGGING` / debug-only macro blocks
- `CATACLYSM_DIAG` temporary instrumentation

Concrete example: in `captaincyLog` (`src\Game\Captaincy.c`) the release retail
function only contains the `clCommandMessage` call; the `dbgMessage` echo and the
`logEnable`/`logfileLog` block compiled out. The correct audit result is `MATCH` on
those constructs and **no source edit** — the real Cataclysm change in that function
was only the `clCommandMessage` argument-count delta. Removing the debug block would
have been a parity regression, not a fix.

If you are *unsure* whether a missing construct is release-stripped debug code or a
genuine removed behavior, leave it `INDETERMINATE` and keep the source — never
delete working code to close a diff.

**Exception — approved repository extensions in demo/utility code.** Certain
source-only branches in `demo.c` and the Win32 input/utility path exist to support
Mode B recording/playback and background automation in this repository. When the
only unmatched source construct is one of those approved extensions, classify it
`MATCH` (expected repository-specific difference) and leave it intact. Do not use
this exception to excuse unrelated retail divergences in the same function.

### Step 5 — Data-access / offset verification (disassembly is the tie-breaker)

For every `field`/`global` construct:

1. Take the source access and compute its **byte offset** into the struct.
2. Take the retail access from the **raw disassembly** (`[reg+0xNN]`), not the
   pseudocode's named field.
3. They must match. A mismatch is `DIVERGENT` (struct layout drift). An offset you
   cannot resolve is `INDETERMINATE`.

Apply the `ship[n]` overlay rule (AGENTS.md): Hex-Rays `ship[n].field` usually means
"read past `Ship` into ship-specific tail/overlay storage," not a real `Ship` array.
Convert it to a concrete offset and confirm whether it lands in `ShipSpecifics`,
module-slot runtime storage, or another post-`Ship` overlay before calling it a match.

### Step 6 — Semantic-equivalence checklist (identical CFG can still diverge)

Even when control flow matches construct-for-construct, explicitly check each region
for these, because they pass a casual read but bite in debugging:

- comparison operator `<` vs `<=`, `>` vs `>=` (Hex-Rays gets these wrong)
- **signed vs unsigned** comparisons / casts
- loop bounds and start index (off-by-one)
- integer width / truncation / sign extension
- argument **order and count** at each call site
- order of side effects between calls

Record these as the `semantics` sub-verdict.

### Step 7 — Decide the verdict

- `MATCH` — every region and every construct is `MATCH`, including the Step 5 offset
  checks and the Step 6 checklist. No `INDETERMINATE` rows remain.
- `DIVERGENT` — at least one confirmed divergence. This is a real, actionable finding.
- `INDETERMINATE` — coverage has gaps you could not close (unresolved callees,
  unknown offsets, ran out of evidence). **Not a pass.** Note exactly what blocked it.

### Step 8 — Reconcile immediately after a proven divergence

If Step 7 yields `DIVERGENT`, reconciliation is part of the same workflow by
default:

1. Record the divergent audit result and keep the evidence precise.
2. Edit the source to match the retail behavior you just proved.
3. Rebuild and re-run any relevant validation already used by the repo.
4. Re-audit the function against the retail evidence and record a new `MATCH`
   result only when the source now lines up construct-for-construct.

Do **not** leave a known-fixable divergence in the queue just because it was
successfully identified. The queue may retain `divergent` rows only for cases that
are still blocked by missing evidence, broader prerequisite reconciliation, or an
explicit user decision to defer the fix.

## Recording the result

A recorded `MATCH` is only valid if the per-construct correspondence table was
actually built and persisted. **Insert one `audit_constructs` row for every
construct you enumerated in Step 2** (not a sample — all of them), then one
`audit_results` row, then update the queue. `constructs_total` must equal the
number of `audit_constructs` rows, and `constructs_matched` must equal the number
of those whose verdict is `MATCH`.

```sql
-- 1. the correspondence table (one row per enumerated construct)
INSERT INTO audit_constructs (result_id, kind, retail_ref, source_ref, verdict, note)
VALUES (:rid, 'field', '0x5402a1 [esi+0x54]', 'src\\Game\\univupdate.c:3812', 'MATCH',
        'collision-sphere offset, offset confirmed against disassembly');
-- ... repeat for every construct ...

-- 2. the result header (counts must reconcile with the rows above)
INSERT INTO audit_results
  (cata_address, hw1_name, source_ref, verdict, control_flow, data_access,
   semantics, constructs_total, constructs_matched, divergences, evidence,
   auditor, commit_sha, audited_at)
VALUES
  ('0x...', 'funcName', 'src\\Game\\foo.c:123', 'DIVERGENT', 'MATCH', 'DIVERGENT',
   'MATCH', 42, 41, 'Field at +0x1C8: source reads bit 0x20000000, retail tests '
   || '0x40000000 (disasm 0x4xxxxx).', 'correspondence table in audit_constructs',
   'session/agent-id', '<git sha or NULL>', strftime('%Y-%m-%dT%H:%M:%SZ','now'));

-- 3. point the queue row at the result and set status to the verdict
UPDATE audit_queue
SET status = 'divergent',           -- match | divergent | indeterminate | mispaired
    last_result_id = last_insert_rowid()
WHERE cata_address = '0x...';
```

`status` values: `match`, `divergent`, `indeterminate`, `mispaired`, `skipped`.
Keep `divergent`, `indeterminate`, and `mispaired` visible — they are the backlog
that finding incomplete analysis later depends on. Under this workflow, `divergent`
should be temporary: record it first, then reconcile immediately and replace it with
follow-up `match` evidence once the source is corrected.

### Validate before you trust a batch of passes

After recording (and before committing `parity_audit.sqlite`), run the strictness
QA gate and fix anything it flags:

```powershell
python src\Scripts\seed_parity_audit.py --validate
```

It fails the batch if any `MATCH` has no correspondence rows, if construct counts
do not reconcile, if a `data_access=MATCH` cites no disassembly offset, if a
`DIVERGENT` looks like a mispairing, or if a queue status disagrees with its
result. A `MATCH` that cannot pass `--validate` is not a pass — re-open it.

## Output to the user

Report concisely, like `parity-review-gate`:

- the verdict (`MATCH` / `DIVERGENT` / `INDETERMINATE`)
- counts: constructs enumerated vs matched; regions passed vs total
- for `DIVERGENT`: each divergence with retail evidence (addr/offset) and the exact
  source location, what reconciliation you are applying now, and what evidence
  would be needed only if the fix is blocked
- for `INDETERMINATE`: precisely what could not be verified and what evidence is needed

If the verdict is `DIVERGENT`, reconciling the source is the expected same-turn next
step unless the fix is genuinely blocked. Treat the change itself as subject to
`parity-review-gate` before commit. Do not "fix" anything you could not first prove
divergent against the binary.

## Hard rules (cannot pass)

A function may **not** be recorded `match` if any of these is true:

1. Identity was not confirmed in Step 0 (the cata function might be a different routine).
2. A retail construct has no source counterpart (forward-pass gap).
3. A source construct has no retail counterpart (reverse-pass gap / latent HW1 code).
4. The `audit_constructs` correspondence table was not persisted, or its row count
   does not reconcile with `constructs_total` / `constructs_matched`.
5. Any `field`/`global` offset was not confirmed against raw disassembly.
6. Any callee whose behavior matters is still `sub_` / unidentified.
7. Any Step 6 semantic item (operator, signedness, bound, arg order) is unverified.
8. The verdict rests on "it looks equivalent" rather than per-construct evidence.
9. Only the pseudocode was consulted for offsets (disassembly is mandatory there).
10. `python src\Scripts\seed_parity_audit.py --validate` flags the row.

Any of the above is `INDETERMINATE` at best. Investigate until it is `MATCH` or
`DIVERGENT`, or leave it `in_progress`/`indeterminate` with a note — never inflate
it to a pass.

**Never delete release-stripped debug code.** Do not remove `dbg*` logging,
`dbgAssert*`, or debug/log-flag-guarded blocks (`logfileLog`, `if (logEnable)`,
`#if HW_BUILD_FOR_DEBUGGING`, `CATACLYSM_DIAG`) just because the release-compiled
retail binary does not contain them. Their absence is a build-mode artifact, not a
divergence; reconcile the actual Cataclysm logic delta only, and leave the debug
code in place (see Step 4).

**Never "fix" approved Mode B / automation extensions back to retail.** In
`demo.c` and the Win32 demo/input/utility helpers, extension-only branches for
Mode B recording/playback or background automation are intentional repository
behavior. Leave them in place and audit only unrelated real retail divergences.

## Database reference (`parity_audit.sqlite`)

- `audit_queue` — seeded work list: `cata_address` (decompile target), `hw1_name`
  (== source == HW1 name), `cata_name` (authoritative IDA name), `priority_tier`
  (A/B/C by `source_lines`), `source_lines`, `hw1_src_ref` (`relpath:line` in the
  HW1 tree), `similarity` (advisory BinDiff annotation, often NULL — never gate on
  it), `sim_debug_retail` (legacy, NULL), `pairing` (always `verified`), `status`,
  `last_result_id`, `seeded_at`.
- `audit_results` — one row per completed audit: verdicts (`verdict`,
  `control_flow`, `data_access`, `semantics`), construct counts, `divergences`,
  `evidence`, `source_ref`, `auditor`, `commit_sha`, `audited_at`.
- `audit_constructs` — **mandatory** per-construct correspondence rows linked by
  `result_id` (`kind`, `retail_ref`, `source_ref`, `verdict`, `note`). One row per
  construct enumerated in Step 2; required for a valid `MATCH`.

## File locations

| Item | Path |
|------|------|
| Audit DB (checked in) | `$env:HWC_ROOT\parity_audit.sqlite` |
| Seeder + validator script | `$env:HWC_ROOT\src\Scripts\seed_parity_audit.py` |
| Identity authority (address→name) | `$env:HWC_ROOT\.ida-snapshot.json` |
| HW1 source (carried-over authority) | `$env:HW_ROOT\src` |
| BinDiff: retail→cata (optional sim hint) | `$env:CATACLYSM_RETAIL_CATA_BINDIFF` |
| Cata source | `$env:HWC_ROOT\src\Game\*.c`, `$env:HWC_ROOT\src\Ships\*.c` |
| IDA database | `$env:HWC_ROOT\cataclysm.i64` |
