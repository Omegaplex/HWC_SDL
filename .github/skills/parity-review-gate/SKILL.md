---
name: parity-review-gate
description: Final parity review before commit. Review the diff for workaround-shaped changes, require authoritative parity evidence for behavior changes, and block commit when evidence is missing.
---

# Cataclysm parity review gate

Use this skill as the **final review gate before committing** behavior-changing work in this repository.

Do **not** use subagents. The calling agent already has the relevant task context, diff context, and investigation history. Perform the review directly in the current agent context.

## Purpose

This skill exists to prevent non-parity fixes from slipping in late in a session when the immediate bug is understood better than the original Cataclysm behavior.

The goal is **not** to ask whether the change seems reasonable. The goal is to ask whether the change is justified by authoritative evidence from the shipped Cataclysm retail binary.

## When to use

Run this skill before commit when the diff does any of the following:

- changes runtime behavior
- modifies imported HW1 code
- adds or removes guards, fallbacks, retries, or default values
- changes data tables, filenames, enums, or constants
- rewires a caller/callee path that could affect reachability
- cleans up decompiled code in a way that might alter behavior rather than only structure

This skill is optional for purely non-behavioral changes such as comments, documentation, or formatting-only edits.

## Authoritative evidence

Every behavior-changing edit must be justified by the **Cataclysm retail binary**:

1. **Cataclysm binary** — IDA decompilation, xrefs, signatures, constants, strings, or control flow

`cataclysm.big` / extracted game data can help with investigation, but they do **not** replace binary evidence for review-gate approval. HW1 source is **not** authoritative for this gate.

## Hard fail conditions

Block the commit if the diff contains any of these without explicit authoritative support:

1. fallback behavior added to avoid a crash or error path
2. clamps, caps, sanitization, or bounds checks added because the observed value "looks wrong"
3. silent null checks, early returns, or default-success/error-return changes
4. "temporary" workaround branches that preserve forward progress instead of exposing the mismatch
5. edits to imported HW1 code without Cataclysm binary evidence that the function changed
6. guessed filenames, enum values, table entries, or constants
7. deletion or disabling of decompiled Cataclysm code because the current tree does not yet reach it
8. structural cleanup of decompiled code that also changes branch behavior, call order, or side effects without evidence
9. replacing a missing retail helper, subsystem, or translation-unit dependency with a local equivalent implementation instead of reconstructing the missing prerequisite properly

## Missing prerequisite rule

If a parity-correct implementation is blocked because the current tree is missing a retail function, helper, declaration, source file, or other prerequisite from another translation unit, the correct fix is to reconstruct and wire that prerequisite.

Do **not** approve a diff that:

- swaps in a nearby helper because it is "functionally equivalent"
- inlines the behavior locally to avoid restoring the real callee
- reads underlying state directly instead of reconstructing the missing API/helper
- substitutes a different draw primitive, data accessor, or utility because the original symbol is not currently linked

Treat these as workaround-shaped deviations even when the immediate behavior appears to match. The gate should require the missing prerequisite to be implemented and called through the correct path before passing.

## Review workflow

1. Read the current diff and identify every behavior-changing hunk.
2. For each hunk, write down:
   - file/function
   - behavioral effect
   - cited evidence source
   - why the edit is parity-safe
3. Flag any hunk whose evidence is missing, weak, or based only on "this fixes the immediate problem."
4. Check especially for the common shortcut pattern: a crash disappears because the new code avoids the original path rather than reconstructing the missing original behavior.
5. If every behavior-changing hunk has solid evidence, approve the diff.
6. If any hunk fails, stop and report the specific blocker instead of approving the commit.

## Output requirements

The review result must be explicit and concise.

If the diff passes, report:

- `PASS`
- a short list of the behavior-changing hunks reviewed
- the evidence basis for each

If the diff fails, report:

- `FAIL`
- the exact hunk(s) that are not parity-safe
- what evidence is missing
- whether the correct next step is IDA investigation or reverting the workaround-shaped edit

If the next step is IDA investigation, assume that is what the user wants and start immediately, do not wait for confirmation.

## Review heuristics

Treat these patterns as suspicious until proven otherwise:

- substituted default values
- range clamping on load/deserialize paths
- added "safe" guards around function-pointer, callback, or table lookups
- replacing hard failure with warning/log-only behavior
- skipping a path because "it only happens in bad data"
- replacing missing initialization with ad hoc local initialization
- changing struct usage to match the current tree instead of the binary layout
- replacing a raw offset/bit test from the binary with a plausible named field or flag without proving the mapping

## Repo-specific reminders

- Prefer loud failure over hidden divergence.
- Missing callers in the current tree are usually a reconstruction gap, not proof that binary code is dead.
- Imported HW1 source is not review authority for Cataclysm behavior changes.
- The shipped Cataclysm retail binary is the approval standard; extracted data does not override it.
