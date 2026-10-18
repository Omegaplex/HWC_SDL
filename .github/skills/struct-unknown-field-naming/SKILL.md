---
name: struct-unknown-field-naming
description: Identify semantic names for unknown struct fields using Cataclysm parity evidence. Audit source xrefs, raw offset accesses, unions, and retail usage before renaming.
---

# Cataclysm struct unknown-field naming

Use this skill when a struct still has `unk_*` members and you need to replace them with semantic names **without guessing past the retail binary**.

This skill is intentionally conservative. The goal is to improve struct readability while preserving exact Cataclysm layout and avoiding source-shaped inventions.

## When to use

Use this skill when the task is any of:

- naming `unk_*` fields in `spaceobj.h` or other runtime structs
- replacing stale HW1 names with Cataclysm-correct overlays
- auditing a union-heavy struct where different ship types reuse the same offsets
- checking whether current source references still match any retail function before trusting a field name

## Hard rules

1. **Do not rename by vibes.** Every accepted name needs an authoritative basis from the Cataclysm binary, extracted game data, or confirmed unchanged HW1 parity.
2. **Preserve layout exactly.** Do not change struct size, field offsets, packing, or binary save/load behavior.
3. **Prefer strongly typed members over raw placeholders.** When evidence is sufficient, replace `unk_*` dwords and byte arrays with typed members directly instead of preserving extra raw aliases for convenience.
4. **Respect unions and overlays only where reuse is real.** If one offset truly serves multiple systems, use the narrowest typed overlay or union needed for the proven overload instead of forcing one name onto every use.
5. **Fix timer-shaped storage when proven.** If a field is used through `Timer*` helpers, type it as `Timer` rather than `ubyte[16]` or raw dwords.
6. **Audit raw offset uses.** Naming work is incomplete if code still uses `*(type *)((ubyte *)obj + 0xNNN)`, `offsetof(...)`, or save/fix helpers against the same region.
7. **Do not delete source paths during naming-only work.** If a stale field name suggests dead HW1 behavior, record it as a follow-up parity audit instead of deleting it in the same pass unless the user explicitly requested that lane too.

## Workflow

### 1. Inventory the target struct

For the chosen struct:

- list every `unk_*` field
- note offset, size, and surrounding known fields
- separate **active** unknowns with source xrefs from pure padding / untouched placeholders

Start with the active set. They give the highest-leverage names and the best evidence.

### 2. Collect source evidence

For each candidate offset, search all of:

- direct field uses like `ship->unk_570`
- overlay uses like `ship->statusEffectOverlay.unk_4F0`
- raw offset casts like `*(udword *)((ubyte *)ship + 0x570)`
- `offsetof(Struct, field)` save/load references
- helper macros that already encode semantics

Record:

- file/function
- access type
- observed value type
- whether the field behaves like a pointer, timer, flags word, counter, owner ref, or state enum

### 3. Collect retail evidence

Use IDA for the same offset and its callers:

1. identify the owning function(s) that read/write the offset
2. inspect decompilation and xrefs
3. confirm width and interpretation
4. confirm whether the same offset is overloaded by ship type or code path

If the source and retail disagree, trust retail.

### 4. Choose the naming shape

Pick the narrowest safe representation:

- **single semantic field** when one meaning clearly owns the offset
- **typed overlay struct** when a contiguous region has a coherent subsystem meaning
- **typed union/overlay only for proven overloads** when the same bytes are genuinely reused by multiple systems
- **leave as `unk_*`** when the evidence is still weak

Prefer names that describe the runtime role, not the current caller's guess.

### 5. Apply the rename safely

When editing:

- rename the field or add a typed overlay in the header
- update source call sites to use the semantic alias where confidence is high
- delete obsolete raw aliases once all proven users can move to typed members
- add or preserve `static_assert`s for overlay sizes and critical offsets

### 6. Record unresolved follow-ups

For each offset you do **not** rename yet, capture why:

- no retail xrefs yet
- overloaded region still ambiguous
- source-only naming evidence
- likely stale HW1 path that needs separate retail-presence audit

## Output expectations

A good pass should produce:

1. a concise list of accepted semantic names with evidence
2. a short list of deferred offsets with the blocker for each
3. any follow-up parity audits required for stale HW1 names or source-only call sites

## Repo-specific reminders

- `Ship` and related runtime structs are heavily overloaded by ship type; do not flatten those overlays into one misleading name.
- Save/load offsets matter as much as normal call sites.
- Raw offset users in decompiled files are often the fastest clue that a field should become a typed overlay.
- A stale HW1 name can survive in current source even when Cataclysm repurposed the same bytes.
