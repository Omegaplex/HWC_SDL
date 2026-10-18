# Cataclysm Source Reconstruction - Agent Guidelines

## Project Overview

This project reconstructs the lost source code for Homeworld: Cataclysm, a sequel to Homeworld 1 sharing the same engine. The approach combines:
- Copying identical HW1 source code (the engine is shared)
- Adapting modified functions using IDA Hex-Rays decompilation of `cataclysm.exe`

## Specialized references

Load these only when the task requires them — do not pull them in by default.

- Load `docs\frida.md` only when you need the detailed Frida / `run_frida.ps1` workflow for
  instrumenting the **retail** binary: memory-writer traces, function-trace/call-log,
  auto-injection, or the IDA symbol-map and retail-launch caveats.
- Load `docs\KASRemap.md` when working on KAS save-file watch-function pointer remapping.

## Critical Rules

- *ALWAYS* prioritize parity with the Cataclysm binary over quick fixes. Do not implement workarounds, shortcuts, or other non-parity fixes to solve a problem. They will be rejected by the user.
- Always prefer to let the game fail loudly if something doesn't match the original Cataclysm binary behavior. If it doesn't work, it's a clue that something is missing or wrong in our reconstruction.
- When asking the user for debugger state, give exact watch expressions in a short copy-pasteable list (one per line) and keep the request focused to the current frame/path.

### Hex-Rays `ship[n]` warning

Do **not** take Hex-Rays `ship[n]` accesses literally for Cataclysm runtime overlays. On this codebase they often mean "read past `Ship` into ship-specific tail storage / overlay data," not "index a real array of `Ship`." Always convert the rendered access back into a concrete offset and check whether it lands in `ShipSpecifics`, module-slot runtime storage, or another post-`Ship` overlay before naming fields or wiring behavior.

### 0. Hard stop on missing parity evidence

If you do **not** have parity evidence for a behavioral change, do **not** implement the fix. Investigate instead.

Any behavioral change must cite at least one authoritative basis:
1. **Cataclysm binary** — IDA decompilation / xrefs / live function signatures
2. **`cataclysm.big` or extracted game data** — actual filenames, scripts, tables, formats
3. **Confirmed unchanged HW1 source parity** — shared engine code with evidence that Cataclysm did not change it

Do **not** substitute fallbacks, clamps, silent null guards, default-success returns, skip-the-broken-path branches, or other "make it work" logic for missing parity evidence.

Before committing a behavior-changing diff, run the repo-local `parity-review-gate` skill. It is the final check for workaround-shaped changes and missing evidence.

### 1. Do NOT modify HW1 code without evidence

The HW1 source code is **tested, working code** from the original game. If imported HW1 code crashes or behaves incorrectly, the bug is almost certainly in our project — not in the HW1 code. Before modifying any HW1-imported file:

1. **Trace the actual cause.** The crash is likely due to missing initialization, a conflicting stub, a wrong struct layout, or a missing subsystem — not a bug in the original code.
2. **Check the IDA disassembly** to confirm whether Cataclysm actually changed the function before modifying it.
3. **Common causes of HW1 code "breaking":**
   - A stub returning 0/NULL where a real function should return valid data
   - A `sdword X = 0;` variable stub being called as a function (jumps to address 0x0)
   - Missing header declarations causing implicit function definitions (generates direct call instead of indirect call for function pointers)
   - Headers with block-commented (`/* */`) or `#if 0`-disabled declarations
   - Tentative definitions without initializers (MSVC treats `type X;` after `extern type X;` as UNDEF)
   - Wrong struct sizes (e.g., `time_t` is 8 bytes on MSVC 2022 vs 4 bytes on original MSVC 5.0)
   - Subsystems not initialized in the correct order

### 2. Check headers for disabled declarations

**Always check for `/* */` blocks or `#if 0` around function declarations** when adding a .c file to the build.

Without proper declarations, the compiler generates implicit `int ()` declarations, causing:
- Function pointers called as direct functions (crashes on the variable address)
- Wrong return types silently corrupting data

### 3. Stub management

Stubs in `stubs.c` are temporary placeholders. They cause problems when:

- **`sdword X = 0;` used as a function**: The linker resolves function calls to this variable's address (containing 0), causing a jump to NULL. Fix: change to `void X() { }` or better, remove the stub when the real implementation is available.
- **Stubs shadow real implementations**: When adding a .c file to the build, its functions may conflict with stubs. Always search stubs.c for conflicting names and remove them.
- **Variable stubs conflict with real globals**: Variables like `RGL`, `RGLtype`, `taskFrequency` defined in stubs.c can shadow the real definitions in their owning .c files. The linker may pick the wrong copy.

**Before adding any .c file to the build:**
1. Check what functions/variables it defines
2. Search stubs.c for ALL matching names
3. Remove conflicting stubs
4. Check if the .h file has commented-out declarations

### 4. Struct binary compatibility

Any struct read from binary files (big files, save games, etc.) must exactly match the original layout:
- `time_t` is 8 bytes on MSVC 2022 x86 vs 4 bytes on original MSVC 5.0. Use `unsigned long` instead.
- Check `sizeof()` and field offsets match the original 32-byte/etc. expectations.
- Padding may differ between compilers.

## Investigation Workflow

### Comparing HW1 vs Cataclysm code

- **BinDiff data** is in the session SQLite database (`bindiff_matches` table, 3809 rows)
- **Function database** is in `functions` table (11,937 entries with addresses, names, HW1 module mapping)
- Functions with BinDiff similarity ≥ 95% are likely identical; use HW1 source directly
- Functions with 50-95% similarity need IDA decompilation to identify changes
- Cataclysm-specific additions (no HW1 match) need full decompilation from IDA

### IDA interaction

- IDA Pro MCP server on port 13337, endpoint: `POST http://localhost:13337/mcp` (JSON-RPC 2.0)
- IDA database: `cataclysm.i64` at the project root

**Important**: IDA MCP serializes requests (~2-5s each). Avoid running parallel workers against the same IDA instance.

## Build Configuration

- **Force-include**: `compat.h` (via /FI) — contains Titan globals, ship aliases, common includes
- **Include paths**: src\Game; src\Win32; src\Ships; src\Generated; hw_rgl; hw_rgl\rgl_common
- **Link libraries**: winmm.lib, dsound.lib, ddraw.lib, dxguid.lib, wsock32.lib, advapi32.lib
- **Renderer**: hw_rgl (D3D9 backend via rgld3d.dll), loaded at runtime via GetProcAddress
- **Debug**: HW_BUILD_FOR_DEBUGGING defined; runtime debug/fatal/ASan output goes to `cataclysm.log` in the game's current working directory (`$env:CATACLYSM_PATH`) and to stdout. Worktree builds use `cataclysm_<worktree>.log`, with non-`[A-Za-z0-9._-]` characters replaced by `_`.

## File Locations

Set `HWC_ROOT` to the Cataclysm repository root.
Set `HW_ROOT` to the root of the original Homeworld source checkout; source files are under `$env:HW_ROOT\src`.

| Item | Path |
|------|------|
| Project | `$env:HWC_ROOT` |
| Original HW1 source | `$env:HW_ROOT\src` |
| Game data | `$env:CATACLYSM_PATH` |
| Extracted big data | `$env:CATACLYSM_PATH\big` — all files from `cataclysm.big` are pre-extracted here; read ship scripts (`.shp`), ETG scripts, NIS files, etc. directly from this directory tree |
| Runtime log | `cataclysm.log` in `$env:CATACLYSM_PATH` (or `cataclysm_<worktree>.log` from a worktree) |
| MSVC build log | `$env:HWC_ROOT\Debug\Cataclysm.log` |

## Building and Running

To build: `msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32 /m /nologo /v:m`

You MUST use the helper script to run the game — do not launch `cataclysm.exe` directly. The executable must run from the build output directory with the working directory set to the Cataclysm install path so logs, config files, and game data resolve correctly.

Set `CATACLYSM_PATH` to your Cataclysm install root. Runtime logs then appear in `$env:CATACLYSM_PATH\cataclysm.log` on main, or `$env:CATACLYSM_PATH\cataclysm_<worktree>.log` from a worktree.

```powershell
.\scripts\run_game.ps1 -Wait 15
.\scripts\run_game.ps1 -Configuration Release -Wait 15 /load "$env:CATACLYSM_PATH\Save\profile\savegame.gam"
```

`scripts\run_game.ps1` enables `/background` by default so agent launches stay hidden, non-activating, and do not steal focus. Pass `-NoBackground` if you need to inspect the window interactively. For worktrees, run the script from the worktree root; it auto-detects `Debug\` or `Release\` relative to itself.

### Demo recording and playback

Use the built-in demo system for non-interactive repros instead of ad-hoc input injection when possible.

- **Record a deterministic demo (Mode A):**
  ```powershell
  .\scripts\run_game.ps1 -Wait 20 -RecordDemo "$env:CATACLYSM_PATH\repros\issue-a.dem" /new 2
  ```
- **Record an exact client/input demo (Mode B):**
  ```powershell
  .\scripts\run_game.ps1 -Wait 20 -RecordDemo "$env:CATACLYSM_PATH\repros\issue-b.dem" /demoModeB /new 2
  ```
- **Play back a demo:**
  ```powershell
  .\scripts\run_game.ps1 -Wait 20 -PlayDemo "$env:CATACLYSM_PATH\repros\issue-b.dem"
  ```

Rules:
- Pass an exact `.dem` path. Do **not** use prefix-only names and do **not** expect `<width>x<height>` suffixes to be appended.
- Mode A keeps the original deterministic simulation-focused capture.
- Mode B extends the file format and records exact client state needed for UI / camera / front-end investigations. Mode B playback also restores the recorded resolution from the demo header before launch.
- Old pre-fix Mode B demos are intentionally unsupported. Re-record them if playback fails.
- Combine `/demoPlay` or `/demoRecord` with `/new`, `/load`, and `/quitAfter` to build unattended repro loops for agents.
- Read `cataclysm.log` (or the active `cataclysm_<worktree>.log`) after each run; the wrapper already launches hidden by default, so it is safe for background agent workflows.

## Temporary Diagnostic Tracing

Wrap all throwaway instrumentation in `#if !defined(HW_Release) && CATACLYSM_DIAG ... #endif` blocks so Release builds always compile the tracing out and `scripts\check_diag.py` can still find it before it lands permanently.

### Tracing workflow

1. **Trace one concrete symptom at a time.** Log the specific runtime state that should distinguish the working binary from the current build (save chunk type, ship flags, health, effect pointers, mesh handles, etc.), not a broad dump of everything nearby.
2. **Keep the trace bounded.** Use a small static counter, gate on a specific ship type / ship number / mission object when possible, and prefer a few high-signal lines over per-frame spam.
3. **Write to the normal runtime log.** Use `dbgMessagef` / `dbgWarningf` for temporary tracing and read the results from `$env:CATACLYSM_PATH\cataclysm.log` or the active worktree-suffixed log after running `scripts\run_game.ps1`.
4. **Do not hide parity bugs.** Temporary tracing must observe state only; never clamp values, skip branches, or add fallback behavior inside a diagnostic block.
5. **Delete the trace once it answers the question.** If you need a second round, replace the old trace with the next narrower one instead of accumulating stale instrumentation.

### Adding tracing

`src\Game\compat.h` is force-included via `/FI`, so non-Release builds default `CATACLYSM_DIAG` to `0` in every translation unit. To enable temporary tracing in one source file, add a local override near the top of that `.c` file:

```c
#undef CATACLYSM_DIAG
#define CATACLYSM_DIAG 1
```

Then wrap the instrumentation in `#if !defined(HW_Release) && CATACLYSM_DIAG` and give the log a clear `CATACLYSM_DIAG_LABEL` prefix:

```c
#if !defined(HW_Release) && CATACLYSM_DIAG
static sdword fooDiagCount = 0;
if (fooDiagCount < 5)
{
    dbgMessagef("\n" CATACLYSM_DIAG_LABEL "foo=%d state=%d", foo, state);
    fooDiagCount++;
}
#endif
```

Rules:
- Use `#if !defined(HW_Release) && CATACLYSM_DIAG`, never `#ifdef CATACLYSM_DIAG`.
- Because `compat.h` is force-included, use `#undef CATACLYSM_DIAG` before `#define CATACLYSM_DIAG 1` to avoid macro redefinition warnings.
- Do not rely on `CATACLYSM_DIAG` alone to suppress tracing in Release; the `HW_Release` guard is what makes Release builds ignore file-local opt-ins.
- Keep `CATACLYSM_DIAG` blocks side-effect free. Do not put required setup, persistent state changes, or data needed by non-diagnostic code inside them.
- Prefix temporary log strings with `CATACLYSM_DIAG_LABEL` so they are easy to grep and obviously temporary.
- Prefer logging stable identifiers that are easy to correlate across runs: ship type/race, `ship->shipID.shipNumber`, save chunk ordinal/offset, pointer values, and the exact flag word or health value you are validating.
- For render/save-load problems, log before and after the handoff boundary you are testing (for example Save/Load/Fix, then the first runtime consumer) so you can tell whether corruption happened during serialization, pointer fixup, or later gameplay code.
- When tracing per-frame code, guard the log with a static counter or a very specific predicate so the log remains readable and the timing stays close to the original binary.

### Checking for leftover tracing

```powershell
python scripts\check_diag.py
python scripts\check_diag.py --all
```

Default workflow: use the script to identify leftover tracing in staged/modified files. Once the issue is fixed, delete the blocks it reports. Release builds run this check and should fail if temporary tracing is still present.

## Temporary Files

Write any temporary script files to \temp (gitignored). Do not leave loose temp files in the repo root.

## Worktrees

Live under `worktrees\` (gitignored). Resolve conflicts in `main`, not in the worktree.

- Use `scripts\create_worktree.ps1` instead of raw `git worktree add` so the new worktree gets a warmed `Debug\` / `Release\` cache copied from the current tree.
- Do not delete worktrees in `worktrees\` that you do not recognize; other agents may be using them.

```powershell
.\scripts\create_worktree.ps1 naming-pass
.\scripts\create_worktree.ps1 -Name naming-pass -BranchName feature/naming-pass
```
