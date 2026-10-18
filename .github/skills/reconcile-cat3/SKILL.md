---
name: reconcile-cat3
description: Reconcile Category 3 functions — Cataclysm-only code with no HW1 equivalent. Clean the IDA pseudocode into compilable C, applying project conventions and proper types.
---

# Reconcile Category 3: Cataclysm-Only Functions

Use this skill when processing functions that have NO HW1 equivalent. These are entirely new Cataclysm additions — beast mechanics, new UI, mission scripting, etc. The only source is IDA decompilation.

## When to use

- A function exists in the Cataclysm binary but has no matching name in HW1 source
- The `#if 0` pseudocode block is the only reference for its implementation
- The function implements Cataclysm-specific gameplay, UI, or system logic

## Identification criteria

A function is Cat 3 if:

1. No function with the same name exists under `$env:HW_ROOT\src\`
2. OR the HW1 function with the same name is completely different (shared name only, <30% flow match)
3. The function implements Cata-specific features: beast infection, Somtaaw/Beast ships, new mission scripting, worker/resource UI, etc.

## Prerequisites

1. **File is in the build** (`Cataclysm.vcxproj`)
2. **Header declarations active** — function declared in appropriate `.h` file
3. **No stub conflicts** in `stubs.c`
4. **Dependencies resolved** — functions called by this code must either be implemented, stubbed, or declared

## Workflow per function

Do *NOT* delegate this work to subagents.

```
1. Read the #if 0 pseudocode block carefully
2. (Optional) Re-decompile from IDA for freshest output:
   - get_function_by_name → address
   - decompile_function → current pseudocode
3. If the pseudocode already resolves to an existing live compiled function in the Cataclysm tree and is functionally identical, stop here. Delete the pseudocode and move on to the next function.
   - Do **not** delete a decompiled reference block merely because our current source tree has no callers for it.
   - Decompiled Cataclysm binary code is authoritative evidence that the function existed in the shipped game. If our tree does not currently reach it, that is a parity gap, not proof that the function is dead.
   - If our tree has no callers yet, first clean the function into compilable C when practical, then use IDA xrefs/callers to identify who should be reaching it. Missing reachability usually means a Cat 2/other parity gap elsewhere.
4. Understand the function's purpose:
   - What does it do? (name + context from callers)
   - What types does it operate on? (Ship*, Player*, etc.)
   - What's its role in the system? (init, update, callback, etc.)
5. Clean the pseudocode into real C:
   - Replace decompiler types with project types
   - Replace numbered locals (v1, v2) with meaningful names where purpose is clear
   - Fix control flow (remove goto where structured flow is obvious)
   - Apply project coding conventions
   - Resolve sub_XXXXXX calls to their real names (from .ida-snapshot.json)
6. Handle unknowns conservatively:
   - If a local's purpose is unclear, keep a generic name (don't invent)
   - If a struct offset is unknown, use a comment: /* offset 0x1C - unknown field */
   - If a sub_ call is unresolved, call it by sub_ name with a TODO comment
   - If cleanup exposes a missing shared helper, prefer restoring/exporting it from its original module (or the closest plausible shared module) rather than adding a file-local copy
7. Delete the #if 0 block only after the behavior exists in live compiled C (either because you reconstructed it or because a verified equivalent live implementation already exists). If the function has no live callers in our tree yet, preserve the reference until you clean it up and investigate the missing callers via IDA.
8. Build and fix errors
```

## Cleaning rules

### Type replacements

| IDA type | Project type | Notes |
|----------|-------------|-------|
| `int` | `sdword` | Signed 32-bit |
| `unsigned int` | `udword` | Unsigned 32-bit |
| `__int16` | `sword` | Signed 16-bit |
| `unsigned __int16` | `uword` | Unsigned 16-bit |
| `char` | `sbyte` | Signed 8-bit |
| `unsigned __int8` | `ubyte` | Unsigned 8-bit |
| `_BYTE` | `ubyte` | |
| `_WORD` | `uword` | |
| `_DWORD` | `udword` | |
| `BOOL` | `bool32` | 4-byte boolean |
| `float` | `real32` | 32-bit float |
| `double` | `real64` | 64-bit float |
| `void *` | Context-dependent | Use actual struct pointer if known |

### Control flow cleaning

IDA produces `goto`-heavy code. Clean patterns:

**IDA goto loop → while loop:**
```c
// IDA:
LABEL_5:
  if (node == NULL) goto LABEL_10;
  // ... body ...
  node = node->next;
  goto LABEL_5;
LABEL_10:

// Clean:
while (node != NULL) {
    // ... body ...
    node = node->next;
}
```

**IDA goto early-return → if/return:**
```c
// IDA:
if (ship == NULL) goto LABEL_END;
// ... body ...
LABEL_END:
return result;

// Clean:
if (ship == NULL)
    return result;
// ... body ...
return result;
```

**Nested goto → break/continue:**
Identify loop exits (`goto` past loop end → `break`) and loop restarts (`goto` loop top → `continue`).

### Variable naming

- Keep IDA names for genuinely unclear locals (`v12` → leave as `v12` with a `// TODO: identify` if truly opaque)
- Rename when purpose is obvious from usage:
  - `v5` assigned from `ship->shiptype` → `shipType`
  - `v3` used as loop counter → `i` or `count`
  - `v8` is a pointer walked through a linked list → `node` or `cur`
- Use project naming conventions: camelCase for locals, PascalCase for functions

### Struct access

When IDA shows raw offset access like `*(DWORD *)(a1 + 0x1C)`:
1. Check if `a1` type is known (from function signature or callers)
2. If the struct is defined in project headers, find the field at that offset
3. Replace with proper field access: `ship->flags`
4. If offset doesn't match known struct layout expand the struct definition to include the new field. Do not leave raw offset access in the file.

### Unresolved dependencies

For `sub_XXXXXX` calls that have no name in `.ida-snapshot.json`:
```c
// TODO: sub_4A3210 - appears to validate beast infection state
sub_4A3210(ship, target);
```

If the sub_ is critical to the function's correctness and can't be stubbed, note it and move on. The function may need to stay partially disabled until the dependency is resolved.

When a cleaned function starts depending on helpers that are still decompiled-only:
- Prefer implementing them in the original source file already associated with that subsystem
- If that file is not in the build yet, use the closest plausible shared module rather than hiding the helper inside the caller
- Extend an existing header if one already owns that subsystem; avoid creating a shadow header with the same name in another include path

## Quality levels

Not all Cat 3 functions need the same polish level. Use judgment:

### Level A — Full clean (important gameplay functions)
- All types resolved
- All locals meaningfully named
- Control flow fully restructured (no goto)
- Comments explaining non-obvious logic

### Level B — Functional clean (utility/helper functions)
- Types resolved
- Some locals may remain as `v1` etc. if truly opaque
- Simple goto patterns cleaned; complex ones left with a `// TODO: restructure`
- Compiles and works correctly

### Level C — Minimal viable (low-priority or very complex functions)
- Compiles correctly
- Basic type replacements done
- May retain some IDA-isms if cleaning them risks introducing bugs
- Wrapped with a comment noting it needs future cleanup

## Output format

```c
// Cataclysm-specific: [brief description of purpose]
void functionName(Ship *ship, sdword param) {
    // ... cleaned implementation ...
}
```

One-line provenance comment describing what the function does. Omit for self-explanatory names.

## Token efficiency guidelines

- Cat 3 is the most token-intensive category — each function needs careful reading
- Process ONE function at a time for complex ones (50+ lines)
- For simple functions (< 20 lines), batch 3-5 together
- Don't re-decompile if the existing `#if 0` block is already clean and complete
- Prioritize functions that unblock live callers first, but when a decompiled function currently has no callers in our tree, clean it up and then use IDA xrefs/callers to identify the missing reachability. No live callers in our tree means a parity gap, not dead code.

## Batch processing guidance

- Prioritize functions that unblock other reconciliation (callees before callers)
- Large functions (100+ lines) may consume an entire session alone
- If a function is too complex to clean confidently in one pass, do Level C and mark for revisit

## Common pitfalls

1. **Don't invent logic** — if the decompilation is ambiguous, leave it conservative. Wrong code is worse than ugly code.
2. **Watch for IDA type errors** — IDA sometimes gets signedness wrong, or conflates pointers with integers. Cross-reference with callers.
3. **Don't fight the decompiler on bitops** — flag manipulation, bit shifts, and mask operations are often correct as-is from IDA. Don't "simplify" them into something wrong.
4. **Preserve magic numbers from IDA** — constants like `0x1000`, `144`, `0xFF` are often correct values from the binary. Only replace with named constants if you're CERTAIN of the mapping.
5. **Test with real game data** — Cat 3 functions often process game-specific data structures. Compile-time correctness doesn't guarantee runtime correctness.
6. **Don't treat reference-only Cataclysm pseudocode as removable dead code** — linker-optimized shipped binary code was used somewhere. If our source tree lacks callers, clean the function up and use IDA xrefs/callers to find the missing caller path rather than deleting or shelving it as dead.

## IDA interaction

Same as Cat 2:
```
1. get_function_by_name({name: "functionName"}) → address
2. decompile_function({address: "0xADDRESS"}) → pseudocode
```

### MCP sanity checks:
- `univFreeAttackTargets` → `0x541DF0`
- `SaveLinkedListOfSpaceObjs` → `0x5019C0`
- `aimPatrolMoveClose` → `0x407A10`

## Build command

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

## File locations

| Item | Path |
|------|------|
| Cata source | `$env:HWC_ROOT\src\Game\*.c` |
| HW1 source | `$env:HW_ROOT\src\Game\*.c` (for context only) |
| Stubs | `$env:HWC_ROOT\src\Game\stubs.c` |
| Headers | `$env:HWC_ROOT\src\Game\*.h` |
| Build project | `$env:HWC_ROOT\Cataclysm.vcxproj` |
| IDA database | `$env:HWC_ROOT\cataclysm.i64` |
| IDA snapshot | `$env:HWC_ROOT\.ida-snapshot.json` |
