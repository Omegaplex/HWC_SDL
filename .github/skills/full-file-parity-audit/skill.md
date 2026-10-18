---
name: full-file-parity-audit
description: Audit an entire reconstructed Cataclysm source file function-by-function against the retail binary, maintaining a per-function parity ledger instead of reviewing only the current diff.
---

# Cataclysm full-file parity audit

Use this skill when a reconstructed source file appears to contain multiple decompilation mistakes and needs a **systematic function-by-function audit** against the shipped Cataclysm retail binary.

Do **not** use subagents. The calling agent should perform the audit directly so the ledger, IDA evidence, and code changes stay in one context.

## Purpose

This skill is for **whole-file parity auditing**, not final diff review.

Its job is to:

1. enumerate the functions in a target source file
2. compare each function's behavior-relevant control flow with the retail binary
3. record a per-function status ledger
4. identify confirmed mismatches and missing prerequisites
5. drive iterative reconstruction until the file is substantially audited

This skill complements `parity-review-gate`:

- `parity-review-gate` answers: **"Is this diff safe to commit?"**
- this skill answers: **"Which functions in this file still diverge from retail?"**

## When to use

Run this skill when any of the following are true:

- a file has multiple confirmed decompilation artifacts
- call targets appear to point into other missions or unrelated systems
- symbol names suggest cross-mission leakage (`Mission02*`, `Mission10*`, etc.)
- the file was reconstructed from pseudocode and needs a broad parity sweep
- repeated bug fixing suggests the remaining unseen mismatches are dense enough that ad hoc repair is no longer efficient

## Authoritative evidence

Every function status must be justified by the **Cataclysm retail binary**:

1. **Cataclysm binary** — IDA decompilation, disassembly, xrefs, constants, strings, or control flow

`cataclysm.big`, extracted data, and HW1 source can help orient investigation, but they do **not** establish parity status for this audit unless the binary evidence also supports the conclusion.

## Audit ledger

Maintain a per-function ledger with one of these statuses:

- `PASS` — inspected and behavior matches retail evidence closely enough for now
- `FAIL` — confirmed divergence from retail
- `NEEDS_INVESTIGATION` — suspicious but not yet proven
- `BLOCKED_PREREQ` — parity depends on reconstructing another missing helper/callee/translation unit first
- `SKIP` — intentionally deferred with reason

Each ledger entry should include:

- file path
- function name
- status
- evidence source(s)
- short note on the observed behavior or mismatch

## Hard fail patterns

Mark a function `FAIL` immediately if retail evidence shows any of these:

1. wrong callee or wrong caller/callee wiring
2. cross-mission call targets in a self-contained KAS script
3. substituted helper that changes semantics or selection criteria
4. missing side effect, branch, timer, message send, or cleanup call
5. wrong predicate or filter function
6. incorrect objective/intel flow transitions
7. attack/order logic inserted where retail only computes or selects
8. cleanup logic inserted where retail issues commands, or vice versa

## Missing prerequisite rule

If a function diverges because the current tree is missing the real retail helper/callee:

- do **not** bless a local substitute as "close enough"
- mark the function `BLOCKED_PREREQ` or `FAIL`
- name the missing prerequisite explicitly
- reconstruct that prerequisite rather than preserving the substitute path

## Workflow

1. Identify the target file and enumerate its function definitions.
2. Create or update a per-function audit ledger.
3. Triage obvious high-risk functions first:
   - cross-mission symbols
   - odd unnamed helper names
   - suspicious predicates/selectors
   - functions already implicated by bugs
4. For each audited function:
   - inspect the source implementation
   - inspect the corresponding retail function in IDA
   - compare control flow, callee set, and behavior-relevant side effects
   - record `PASS`, `FAIL`, `NEEDS_INVESTIGATION`, `BLOCKED_PREREQ`, or `SKIP`
5. If you confirm a mismatch and can repair it with strong binary evidence, patch it immediately and update the ledger.
6. If a mismatch needs deeper IDA work, continue investigating the next suspicious function rather than stopping at the first issue.
7. After the pass, summarize:
   - audited functions
   - confirmed mismatches fixed
   - confirmed mismatches remaining
   - next highest-value investigation targets

## Review heuristics

Treat these as suspicious until proven otherwise:

- references to another mission's functions in a mission script
- generic-looking helper names copied from another file
- functions that select ships using a predicate from another mission
- command/attack calls appearing in places where retail only builds selections
- missing `tutDeallocateRootRegion`, message send, timer, or speech-event calls
- wrong `kasfSpeechEvent` init/watch targets
- watch/init callbacks whose local names disagree with retail call structure

## Output requirements

The audit result must be explicit.

Report:

- `AUDIT RESULT`
- the file audited
- the functions reviewed this pass
- ledger entries added or updated
- confirmed `FAIL` items and retail evidence
- any fixes applied
- the next functions that should be audited

If the audit reveals a strong next mismatch, assume the user wants that investigation to continue immediately unless they redirect you.

## Repo-specific reminders

- KAS mission scripts are expected to be self-contained unless retail evidence proves otherwise.
- Prefer loud failure over hidden divergence.
- A wrong but plausible helper is still a parity bug.
- Missing reachability in the current tree does **not** prove the retail code path is dead.
- Use `parity-review-gate` later as the final pre-commit check after the full-file audit work produces a diff.
