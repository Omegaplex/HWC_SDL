---
name: cleanup-decompiled-readability
description: Improve readability of an already-compiling decompiled/reconstructed source file without changing behavior. Convert magic numbers to named constants/enums, name raw offsets and weak struct fields, add explanatory comments, and strip decompiler artifacts while preserving control flow and binary layout.
---

# Cleanup decompiled file for readability

Use this skill when a file is **already in the build and behaves correctly**, but still reads like
decompiler output: magic-number state machines, raw offset access, `unk_*` struct fields, missing
comments, and leftover decompiler artifacts. The goal is a readability pass that a future maintainer
can follow, **without changing observable behavior, control flow, or binary layout**.

This is a *polish* pass, not a reconstruction pass. If the file does not yet compile or is still a
`#if 0` pseudocode block, use `reconcile-cat1/2/3` first, then return here.

The canonical worked example is the `src/Ships/HiveFrigate.c` + `HiveFrigate.h` cleanup (commit
`df274bb`, "Restore retail Hive Frigate overlays"). It was an entirely new Cataclysm file with the
worst readability problems in the tree, and every pattern below comes from that pass.

## When to use

- A reconstructed/decompiled `.c` file compiles and runs, but is hard to read.
- A function or struct is dominated by bare integer literals (`spec->unk0 = 5;`, `case 3:`, `for (i = 0; i < 6; i++)`).
- Code still uses raw pointer arithmetic: `*(real32 *)((ubyte *)swarmer->ShipSpecifics + 28)`.
- A struct still carries `unk0`, `unk4`, `unkFD7`, or other weak placeholder names that are actually understood.
- The logic is correct but uncommented, so the *why* is lost.

## Hard rules (do not break parity)

1. **Behavior must not change.** This is a pure readability pass. Control flow, operator semantics,
   evaluation order, and side effects stay byte-for-byte equivalent. If a "cleanup" would alter what
   the CPU does, it is out of scope for this skill — route it through the normal parity workflow and
   the `parity-review-gate` instead.
2. **Preserve binary layout exactly.** Renaming a struct field or splitting a `union` must not change
   any offset, size, or padding. Add `static_assert(offsetof(...) == 0xNN)` to lock layout when you
   touch a struct that is read from save games or overlaid on runtime memory.
3. **Every name must have evidence.** A constant/enum/field name is a claim about the binary. Back it
   with IDA decompilation, xrefs, extracted game data, or confirmed-unchanged HW1 parity — exactly the
   same standard as any other change. Do **not** invent semantic names by vibes. If you cannot justify
   a name, leave the placeholder and move on.
4. **Keep magic numbers that you cannot map.** A literal you don't understand stays a literal. Naming
   it wrong is worse than leaving it bare. Only promote a literal to a constant when you are certain of
   the mapping.
5. **Do not delete code paths.** A readability pass never removes a branch, a case, or a field "because
   it looks unused." Unused-looking storage may be required for layout or for a parity gap elsewhere;
   document it instead (see "Unused / overlay fields").
6. **Verify with a build.** The file compiled before your pass; it must still compile after. Renames
   that miss a call site, or `static_assert`s that fail, are caught here.

## What to change

### 1. Magic numbers → named constants / enums

This is the highest-value change. Group related literals into a named set and document the state
machine they encode.

- Identify literal *families*: a `switch (spec->unk0)` with cases `0..7`, an anim-state compared
  against `0,1,2,3,4`, a fixed loop bound `6` repeated everywhere.
- Define the set in the owning header with a short doc block. Prefer `#define` groups (or an `enum`)
  that mirror the binary's numeric values exactly. Keep the original integers — only the spelling changes.
- Document the state machine transitions in the header comment so the reader can follow the flow
  without reverse-engineering it again.

```c
// HiveFrigate.h
#define HIVE_MAX_SWARMERS 6

/*
 * Hive frigate swarmer-launch state machine (HiveFrigateSpecifics::launchState).
 * Idle ──(in range)──► Armed ──► Launching ──► Deploying ──► Active
 * Active ──(target lost)──► Canceling ──► Recalling ──► Idle
 */
#define HiveLaunch_Idle      0  // no active special
#define HiveLaunch_Armed     1  // in range; assigning launch timers
#define HiveLaunch_Launching 2  // issuing dock-to-launch commands
/* ... */
#define HiveSwarmer_Docked    0  // docked and idle inside the hive
#define HiveSwarmer_Launching 1
/* ... */
```

Then sweep the `.c`: `spec->unk0 = 5;` → `spec->launchState = HiveLaunch_Canceling;`,
`case 3:` → `case HiveLaunch_Deploying:`, `for (i = 0; i < 6; i++)` → `for (i = 0; i < HIVE_MAX_SWARMERS; i++)`,
`salCapGetAnimState(swarmer) == 0` → `... == HiveSwarmer_Docked`.

Repeated bounds like `6` that drive parallel loops and a `swarmers[6]` array should all reference the
same constant, and the array declaration should carry a comment tying it to that constant.

### 2. Raw offset access → named fields

A readability pass is **incomplete while any `*(type *)((ubyte *)obj + 0xNN)` or `offsetof`-style raw
access remains** for a field you understand.

- Resolve the offset against the real struct layout. Confirm where it lands (`ShipSpecifics`, a runtime
  overlay tail, `attackvars`, etc.) — do not take Hex-Rays `ship[n]` array accesses literally.
- Replace with a typed accessor or a named field. If the offset is reached through a documented
  overlay, model the overlay as a struct with named fields and `static_assert` the offsets.

```c
// before
return (real32 *)((ubyte *)swarmer->ShipSpecifics + 28);
// after
return &hiveSwarmerOverlay(swarmer)->healthUpdateTime;
```

When the binary overlays state onto an existing tail (rather than starting at offset 0), make the
overlay explicit with reserved gaps and lock it down:

```c
typedef struct
{
    ubyte  reserved_00_07[0x08]; // swarmer's own runtime state
    sdword slotIndex;
    Ship  *parentHive;
    sdword animState;
    real32 readyAt;
    real32 reserved_18;
    real32 launchDelay;
    real32 healthUpdateTime;
} HiveSwarmerOverlay;

static_assert(offsetof(HiveSwarmerOverlay, animState) == 0x10);
static_assert(offsetof(HiveSwarmerOverlay, healthUpdateTime) == 0x20);
```

### 3. Strengthen weak struct field names

Replace `unk0`, `unk4`, `unkFD7`-style members with semantic names **when you have evidence** for what
they hold. This is the conservative naming work covered in depth by the `struct-unknown-field-naming`
skill — defer to it for the audit methodology (xrefs, unions, overlays, save/fix helpers).

- Promote understood fields to typed, named members; add a trailing `// ...` comment describing units,
  valid ranges, or the consuming system.
- Collapse a decompiler `union { udword unk4; real32 rebuildSwarmerAt; }` to the single proven member
  when only one arm is real.
- Keep layout identical: a renamed `ubyte unkFD7` that is genuinely padding stays the same size and
  offset, just better documented.

```c
// before                           after
udword unk0;                        udword launchState;   // HiveLaunch_* state machine
ubyte  unkFD7;                      ubyte  reserved_FD7;  // unused padding (kept for layout)
```

### 4. Add documentation where it helps

Add comments that explain *why*, not *what*. Target:

- Each non-obvious state-machine `case`: one line on what the state is doing and what it transitions to.
- Loops/branches whose intent is not obvious from the code (e.g. "Exclude friendly swarmers from the
  target list", "still waiting on at least one swarmer").
- Overlay/struct doc blocks explaining how the layout maps onto retail memory.
- Keep retail-provenance markers already in the file (`// Decompiled from cataclysm.exe @ 0x5B5770`).

Do **not** add comments that restate the code, narrate the cleanup, or reference earlier attempts in
this session.

### 5. Strip decompiler artifacts (without touching control flow)

Safe cosmetic cleanups only:

- Replace decompiler types with project types (`int`→`sdword`, `float`→`real32`, etc. — see the table
  in `reconcile-cat3`).
- Give meaningful names to numbered locals (`v5` from `ship->shiptype` → `shipType`) where the purpose
  is unambiguous; leave genuinely opaque locals alone.
- Tidy whitespace/blank lines around the banner and declarations.
- Do **not** restructure `goto`/loops in this pass unless the file still has raw `goto` artifacts; if it
  does and converting them is non-trivial, that is reconstruction work — keep it separate so the
  readability diff stays reviewable and obviously behavior-preserving.

## Unused / overlay fields

If a field looks unused after you wire up the real storage (e.g. the old `madState`/`busyFlags` now
live on `ship->attackvars.attackevents` / `ship->cataRuntimeFlags1C8`), **do not delete it**. Keep it
for layout and annotate where the live value actually lives:

```c
ubyte madState;   // unused padding (was madAnimationStart tracker; now ship->attackvars.attackevents)
ubyte busyFlags;  // unused padding (now ship->cataRuntimeFlags1C8)
```

## Workflow

```
1. Confirm scope: file already compiles and runs. If not, reconcile first.
2. Read the whole file. Inventory:
   - literal families (state values, loop bounds, anim states)
   - raw offset accesses
   - weak field names (unk*) and decompiler-typed locals
   - undocumented non-obvious branches
3. Gather evidence for any name you intend to assign (IDA decomp/xrefs, big data, HW1 parity).
   No evidence → leave the placeholder.
4. Add named constants/enums + state-machine doc to the owning header.
5. Sweep the .c to use the new names; replace raw offsets with typed fields; add static_asserts
   for any touched binary-layout struct.
6. Rename understood struct fields; document the rest as reserved/padding.
7. Add "why" comments to non-obvious logic.
8. Build. Fix any missed call sites or failing static_asserts.
9. Re-read the diff: confirm it is rename/comment/constant-only with zero control-flow changes.
```

## Verification

- The build must succeed and all `static_assert`s must pass.
- Diff self-review: the change set should be limited to identifiers, comments, constants, and struct
  field names. If a line changes what the program *does*, it does not belong in this skill's diff.
- Run the `parity-review-gate` skill before committing if the pass touched anything beyond pure
  renames/comments (e.g. overlay remodeling), so workaround-shaped changes are caught.

## Common pitfalls

1. **Promoting a literal you don't actually understand.** Wrong constant names mislead future readers
   worse than bare numbers. Map it or leave it.
2. **Changing a numeric value while renaming it.** `#define HiveLaunch_Canceling 5` must keep the value
   the binary uses. Verify the integer, not just the spelling.
3. **Renaming a struct field and breaking layout.** Always `static_assert` offsets on save-game/overlay
   structs after edits.
4. **Missing a call site during a sweep.** A renamed field with a leftover `spec->unk0` reference fails
   to compile — good — but a renamed *accessor* used in another TU may not. Grep the tree, then build.
5. **Deleting "unused" fields or branches.** Out of scope. Document them instead.
6. **Smuggling in a behavior fix.** If you spot a real bug during cleanup, stop and handle it as a
   separate parity-evidenced change; do not bury it inside a readability diff.

## Related skills

- `struct-unknown-field-naming` — the conservative methodology for naming `unk_*` fields.
- `parity-review-gate` — final pre-commit check for workaround-shaped or behavior-changing diffs.
