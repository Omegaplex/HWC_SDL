---
name: reconcile-cat2
description: Reconcile Category 2 functions — HW1 code modified in Cataclysm. Start from HW1 source, apply targeted edits identified by comparing against IDA decompilation of the Cataclysm binary.
---

# Reconcile Category 2: Modified HW1 Functions

Use this skill when processing functions that exist in both HW1 and Cataclysm but have meaningful differences. The HW1 source is the starting point, and IDA decompilation reveals the exact changes Cataclysm made.

## When to use

- A function exists in HW1 source AND the Cataclysm binary (named in IDA)
- The pseudocode block shows logic differences vs HW1: extra parameters, different calls, additional branches, changed constants, new enum values
- The core structure/purpose is recognizably the same function, just modified

## Identification criteria

A function is Cat 2 if:

1. HW1 source exists for this function name
2. AND one or more of:
   - Extra parameters added in Cata version
   - Calls to Cata-specific functions (ship types, beast mechanics, etc.)
   - Additional branches or cases (e.g., new switch cases for Cata ship types)
   - Changed constants or enum values
   - Modified data table entries
   - String literal changes (`"Homeworld"` → `"Cataclysm"`, new NIS names, etc.)

NOT Cat 2 (reclassify as Cat 3) if:
- The function is completely rewritten with only the name in common
- Less than ~30% of the control flow is shared with HW1
- The parameter signature is entirely different (not just extra params)

## Prerequisites

Same as Cat 1:
1. File is in the build (`Cataclysm.vcxproj`)
2. Header declarations active (no `/* */` or `#if 0` around decl)
3. No stub conflicts in `stubs.c`
4. Struct compatibility verified (ShipStaticInfo offset shift)

Additionally:
5. **IDA MCP available** — Cat 2 requires decompiling from the binary
6. **Function is named in IDA** — must have a real name (not `sub_XXXXXX`)

## Workflow per function

```
1. Read the #if 0 pseudocode block in the Cata source file
2. Find the HW1 source for the same function name
3. Decompile from IDA (if pseudocode block is stale or incomplete):
   - Use get_function_by_name to get the address
   - Use decompile_function with that address
4. Diff the HW1 source against the IDA decompilation:
   - Identify WHAT changed (new params? extra cases? different calls?)
   - Identify WHERE in the function the changes occur
   - Identify WHY (Cata ship types? beast mechanics? UI changes?)
5. Start from the HW1 source code
6. Apply ONLY the identified changes:
   - Add new parameters to the signature
   - Add new switch cases / if branches
   - Replace HW1-specific constants with Cata equivalents
   - Update function calls that changed
7. Preserve HW1 code style and variable naming where unchanged
8. Delete the #if 0 pseudocode block
9. Build and fix errors
```

## Diff strategy

The goal is **minimal surgical edits** to HW1 source, not a rewrite from the decompilation. IDA decompilation has artifacts (goto, weird casts, wrong types) that make it unsuitable as direct source. Instead:

### What to trust from IDA decompilation:
- Control flow structure (which branches exist)
- Function calls made (what's called and in what order)
- Constants and enum values used
- Parameter count and rough types
- String literals

### What to trust from HW1 source:
- Variable names and types
- Code style and idioms
- Comment-worthy logic
- Correct operator usage (IDA sometimes gets `<` vs `<=` wrong)
- Proper use of project macros and types

### Merge principle:
> "HW1 source + Cata-specific deltas identified from IDA = correct Cata source"

## Common modification patterns

### Pattern: New ship type cases

HW1:
```c
switch (ship->shiptype) {
    case HeavyCruiser:
    case Carrier:
        // ...
}
```

Cata (add new Cata ship types):
```c
switch (ship->shiptype) {
    case HeavyCruiser:
    case Carrier:
    case sDreadnought:  // Cata addition
    case bCarrier:      // Cata addition
        // ...
}
```

### Pattern: Extra parameter

HW1: `void foo(Ship *ship, sdword count)`
Cata: `void foo(Ship *ship, sdword count, bool32 beastMode)`

Add the parameter and any conditional logic that uses it.

### Pattern: Changed string/filename

HW1: `bigFileOpen("Homeworld.big", ...)`
Cata: `bigFileOpen("cataclysm.big", ...)`

### Pattern: Extended data tables

HW1 tables have fewer entries. Cata tables have entries for new ship types, races, or game modes. Adopt HW1 table structure, extend with Cata entries identified from IDA.

### Pattern: New helper calls

Cata version calls functions that don't exist in HW1. These may be:
- Already implemented elsewhere in the Cata build → just call them
- Still stubbed → add a stub and note it for later Cat 3 processing
- Inlined in the decompilation → extract and name appropriately

## Output format

The function appears as normal compiled C — no `#if 0`, no pseudocode. Optionally:

```c
// Based on HW1 source, modified for Cataclysm (extra beast ship cases, new callback param)
void functionName(params) {
    // ... merged source ...
}
```

Keep the provenance comment to one line describing what changed. Omit for trivial changes (string swap, one extra enum case).

## IDA interaction

### Decompile a named function:
```
1. get_function_by_name({name: "functionName"}) → get address
2. decompile_function({address: "0xADDRESS"}) → get pseudocode
```

### Check what a sub_ call resolves to:
```
1. get_function_by_address({address: "0xADDRESS"}) → get current name
   - If named: use that name
   - If still sub_: note as unresolved dependency
```

### MCP sanity checks (verify before first use):
- `univFreeAttackTargets` → `0x541DF0`
- `SaveLinkedListOfSpaceObjs` → `0x5019C0`
- `aimPatrolMoveClose` → `0x407A10`

## Token efficiency guidelines

- Do NOT paste entire decompilations into your working context if they're huge
- Focus on the DIFF — what's different between HW1 and Cata
- For large functions (100+ lines), identify changed regions by line ranges rather than holding the full text
- Process one function at a time, commit the mental diff, then move on
- If a function has >50% new code, reclassify as Cat 3 — it's cheaper to work from pseudocode than to try merging

## Batch processing guidance

- Cat 2 is slower than Cat 1 (one IDA decompile per function)
- Expect 5-10 Cat 2 functions per session depending on complexity
- Group by file — processing all Cat 2 in one file maintains context about local types/patterns
- After finishing a file's Cat 2 functions, build immediately to catch issues while context is fresh

## Common pitfalls

1. **Don't blindly trust IDA's types** — IDA guesses types; HW1 source has the real types. Only adopt IDA-specific changes that represent genuine Cata logic, not decompiler artifacts.
2. **Watch for inlined functions** — IDA may show code from inlined callees mixed into the function body. Compare against HW1 to distinguish real additions from inlined noise.
3. **Don't over-modify** — if a change isn't clearly intentional (could be compiler optimization artifact), leave the HW1 version. Conservative is correct.
4. **Propagate signature changes** — if you add a parameter, update all call sites too (or at minimum, ensure they compile with a default).

## Build command

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

## File locations

| Item | Path |
|------|------|
| Cata source | `$env:HWC_ROOT\src\Game\*.c` |
| HW1 source | `$env:HW_ROOT\src\Game\*.c` |
| Stubs | `$env:HWC_ROOT\src\Game\stubs.c` |
| Headers | `$env:HWC_ROOT\src\Game\*.h` |
| Build project | `$env:HWC_ROOT\Cataclysm.vcxproj` |
| IDA database | `$env:HWC_ROOT\cataclysm.i64` |
