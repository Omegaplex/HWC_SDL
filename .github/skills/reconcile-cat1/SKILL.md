---
name: reconcile-cat1
description: Reconcile Category 1 functions — HW1 code unchanged in Cataclysm. Adopt the HW1 source directly, remove the pseudocode block, verify the build compiles.
---

# Reconcile Category 1: Unchanged HW1 Functions

Use this skill when processing functions that exist identically in HW1 and Cataclysm. These are the easiest to reconcile — the HW1 source IS the correct implementation.

## When to use

- A `#if 0` pseudocode block's logic matches the HW1 source function exactly (or with only cosmetic differences)
- The function name exists in both HW1 source and the Cataclysm binary at a known address
- No Cataclysm-specific types, calls, or logic appear in the decompiled output

## Identification criteria

A function is Cat 1 if ALL of:

1. The HW1 source file contains a function with the same name
2. The pseudocode block's control flow matches HW1 (same branches, loops, returns)
3. Called functions are the same (no `sub_XXXXXX` calls that resolve to Cata-only functions)
4. Data types are compatible (type aliases like `bool`→`bool32` are cosmetic, not functional)
5. String literals match (or differ only in expected ways like `"Homeworld"`→`"Cataclysm"`)

Cosmetic differences that are STILL Cat 1:
- `bool` → `bool32`, `udword` → `bitflag32`/`memsize`, `ubyte` → `sbyte`
- Variable names differ (decompiler artifacts)
- Whitespace/formatting
- Cast style differences

## Prerequisites

Before adopting ANY Cat 1 function, verify:

1. **The file is in the build** — check `Cataclysm.vcxproj` for the `.c` file
2. **Header declarations are active** — check the `.h` file for `/* */` or `#if 0` around the function declaration; uncomment if needed
3. **No stub conflicts** — search `stubs.c` for the function name; remove if found
4. **Struct compatibility** — if the function accesses `Ship`, `ShipStaticInfo`, `SpaceObj`, or other structs with known Cata differences, confirm field offsets are correct (see Struct Considerations below)

## Workflow per function

```
1. Read the #if 0 pseudocode block in the Cata source file
2. Find the corresponding HW1 function:
   - Same filename in `$env:HW_ROOT\src\Game\<file>.c` (or `Win32\` or `Ships\` under `$env:HW_ROOT\src`)
   - Match by function name
3. Compare:
   - Control flow identical? → proceed
   - Minor type alias differences only? → proceed (Cat 1)
   - Logic differences (extra params, different calls, new branches)? → STOP, this is Cat 2
4. Replace the #if 0 block with the HW1 source code
5. Apply any necessary type alias updates:
   - bool → bool32 (if the project uses bool32)
   - Other alias changes per SpaceObj.h conventions
6. Build: msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
7. Fix any compile errors (usually missing declarations or type mismatches)
8. If errors require non-trivial logic changes → reclassify as Cat 2
```

## Struct considerations

### ShipStaticInfo offset shift

Cataclysm's `ShipStaticInfo` has 16 extra `CustShipUnknownNN` function pointer slots compared to HW1. Any HW1 function that accesses ShipStaticInfo fields AFTER those slots will have incorrect offsets unless the struct is already updated.

**Before adopting functions that access ShipStaticInfo**: confirm the struct in `src/Game/SpaceObj.h` has the 16 extra slots. If not, the struct fix must be applied first.

### Ship struct

The Ship struct has the same field count (144) in both HW1 and Cata, but some fields changed type. Functions that access Ship fields by offset (common in ship command/AI code) are safe as long as field sizes match.

### time_t

Any function using `time_t` directly needs `unsigned long` substitution (MSVC 2022 `time_t` is 8 bytes vs original 4 bytes).

## Output format

After adoption, the function should appear as normal compiled C code — no `#if 0` wrapper, no pseudocode comments. The old pseudocode block is deleted entirely.

If you want to preserve a note about provenance:

```c
// Adopted from HW1 source — identical in Cataclysm binary at 0xADDRESS
void functionName(params) {
    // ... HW1 source code ...
}
```

Keep provenance comments minimal — one line max. Omit entirely for obvious/simple functions.

## Build command

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

## Batch processing guidance

Cat 1 functions are high-volume mechanical work. For efficiency:

- Process all Cat 1 functions in a single file before moving to the next file
- Group files by tier (smallest first): files with 1-3 blocks → files with 10-30 → large files
- A single agent can process 10-20 Cat 1 adoptions per session before context pressure
- No IDA interaction needed for Cat 1 — this is purely source-level work

## Common pitfalls

1. **Don't modify HW1 code to "fix" it** — if HW1 code doesn't compile, the problem is elsewhere (missing include, wrong struct, stub conflict)
2. **Watch for implicit declarations** — if the compiler warns about implicit function declarations, the header needs uncommenting, not the function body changing
3. **Don't adopt without checking calls** — if HW1 code calls functions that don't exist in Cata (different name or removed), it's Cat 2
4. **String literal differences matter** — `"Homeworld"` vs `"Cataclysm"` in user-facing strings means it's Cat 2 (even if trivial to fix)

## File locations

| Item | Path |
|------|------|
| Cata source | `$env:HWC_ROOT\src\Game\*.c` |
| HW1 source | `$env:HW_ROOT\src\Game\*.c` |
| Stubs | `$env:HWC_ROOT\src\Game\stubs.c` |
| Headers | `$env:HWC_ROOT\src\Game\*.h` |
| Build project | `$env:HWC_ROOT\Cataclysm.vcxproj` |
