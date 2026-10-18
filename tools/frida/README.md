# Cataclysm Frida runtime

Runtime instrumentation for the **retail** `cataclysm.exe`, ported from the QFG5
Frida infra. It complements the CET MinHook parity DLL, an external dependency
maintained outside this repository and used to test the retail executable. Use
the DLL for durable, typed, committed parity checks, and use this for fast
exploratory questions the DLL is bad at — above all **"who writes this memory?"**.

## What it does
- Spawns retail `cataclysm.exe` under Frida (or attaches to a running one).
- **Injects `CataclysmEngineTweaks.dll` (CET) on spawn**, replicating
  `cataclysm_loader.exe`. Retail misbehaves on modern Windows without it.
- Resolves symbols from the **IDA** function map (default: repo `.ida-snapshot.json`,
  12,160 functions). Rebases IDA VAs to the live module automatically.
- Emits **ParityLog-style** lines so captures diff against CET / reconstructed logs:
  `[frame=N] channel: key=value ...`  (frame = `universe.univUpdateCounter`).

### Headline feature: memory-writer traces
`memwrite-watch` arms a **hardware watchpoint** on a field/global and reports every
write — the writing instruction (resolved to an IDA function), value, thread, and
backtrace. This needs no recompile and pinpoints *who mutates a struct field*.

## Setup
```powershell
py -3.11 -m venv tools\frida\.venv
tools\frida\.venv\Scripts\python.exe -m pip install -r tools\frida\requirements.txt
```
Requires the IDA Pro MCP server on `localhost:13337` only if you rebuild the map.

## Usage
`$py = tools\frida\.venv\Scripts\python.exe`

```powershell
# Sanity: spawn retail (+CET) and report module info
& $py tools\frida\cataclysm_runtime.py ping

# Resolve an IDA symbol to its runtime address
& $py tools\frida\cataclysm_runtime.py resolve-symbol --symbol univUpdate

# WHO WRITES THIS FIELD: watch univUpdateCounter (universe + 0x1ac)
& $py tools\frida\cataclysm_runtime.py memwrite-watch --field-addr 0xB62BAC --size 4 --duration 30

# Watch a field by symbol+offset (symbol must be in the map; see globals note)
& $py tools\frida\cataclysm_runtime.py memwrite-watch --symbol someGlobal --offset 0x10 --size 4

# Trace a function: args, return, and the .data writes it performed
& $py tools\frida\cataclysm_runtime.py function-trace --symbol aitrackHeadingAndUp --arg-count 3 --backtrace `
    --world '[{\"name\":\"counter\",\"address\":\"0xB62BAC\",\"size\":4}]'

# Log every call to one or more functions
& $py tools\frida\cataclysm_runtime.py call-log --symbol taskExecuteAllPending --no-leave
& $py tools\frida\cataclysm_runtime.py call-log --symbol Watch_Mission01_MissionFlow_FirstWarning `
    --symbol Watch_Mission01_MissionFlow_FirstWarningDelay --no-leave

# Follow only heavy interceptor 119 in retail dreadnoughtAttack
& $py tools\frida\cataclysm_runtime.py call-log --symbol dreadnoughtAttack --arg-count 2 `
    --decode-u16 ship:0:0x488 --decode-u16 target:1:0x488 --filter-u16 0:0x488:119

# Follow heavy interceptors by ship type instead of transient ship number
& $py tools\frida\cataclysm_runtime.py function-trace --symbol isThisShipThreateningMe --arg-count 2 --repeat `
    --decode-u32 meType:1:0x2B0 --decode-u32 targetType:0:0x2B0 `
    --decode-u16 targetShip:0:0x488 --filter-u32 1:0x2B0:68,82
```

Common options: `--output <file>` (also write the ParityLog), `--attach <pid|name>`
(instead of spawning), `--no-cet`, `--no-frame`, `--frame-addr <va>`, `--symbol <name>`
(for `call-log`, repeat to install multiple hooks in one session),
`--decode-u16 <label:arg:offset>`, `--filter-u16 <arg:offset:value>`,
`--decode-u32 <label:arg:offset>`, `--decode-f32 <label:arg:offset>`,
`--filter-u32 <arg:offset:value[,value...]>`.

For `call-log`, omitting `--duration` now means "stay attached until Ctrl-C or the hooked process exits."

## Input record / replay (window-only)
Capture keyboard/mouse from the retail window and replay it later **into the game
window only** — the real desktop cursor is never moved and no global synthetic
input is generated, so you can replay in the background.

```powershell
# Record: play in the game window; Ctrl-C or --duration stops it. The trace is
# written continuously, so a crash/exit mid-record still leaves a usable file.
& $py tools\frida\cataclysm_runtime.py input-record --output temp\login.json --duration 20

# Replay the trace into the game (spawns retail, waits for the window, injects,
# then terminates the game and exits so an agent can continue immediately)
& $py tools\frida\cataclysm_runtime.py input-replay --trace temp\login.json
```

How it maps onto Cataclysm's input model (`src/Win32/main.c`, `mouse.c`):
- **Keyboard / mouse buttons / wheel** arrive as `WM_*` messages dispatched to the
  game's `WindowProc`. Recorded from `DispatchMessageA`; replayed with
  `PostMessageA` to the game window (queue delivery — no foreground
  needed, nothing else on the desktop is touched).
- **Mouse position** is polled via `GetCursorPos` (there is no `WM_MOUSEMOVE`
  handler). Recorded as the **raw screen point** the game receives; on replay the
  `GetCursorPos` return value is overridden with that same raw point so the game
  computes the identical in-game position (it subtracts its own client origin via
  `utyClientRectGet`). `SetCursorPos` (the game's cursor-clip call) is no-op'd
  during replay so the real cursor stays put.

Notes:
- Record and replay at the **same resolution and window position** — cursor samples
  are raw screen pixels and the game subtracts its own client origin. A client-size
  mismatch is reported as a warning.
- Replay options: `--speed <x>`, `--settle-ms`, `--window-timeout-ms`,
  `--allow-set-cursor` (let the game move the real cursor when clipped),
  `--duration` (override pump time), `--no-kill` (leave the game running),
  `--no-sanitize` (replay the raw trace without balancing key/button state).
  Record options: `--no-cursor`, `--no-messages`, `--window-class`. Recording
  streams to disk continuously (throttled atomic rewrite), so a crash/kill
  mid-record still leaves a usable trace.
- Replay **balances key/button state** by default so a key can't get stuck. The
  recorder (hooking `DispatchMessageA`) misses the release of keys that enter
  Windows menu-mode or change focus (notably **ALT** via Alt-Tab); a stuck key
  reinterprets later clicks and crashes the retail front-end in `regRegionProcess`
  (calling a freed region callback). For a press with no recorded release, replay
  **synthesizes a release** ~30 ms after the key's last activity rather than
  dropping the press; auto-repeat key-downs (lParam bit 30) are treated as part of
  an existing hold, so a held key across a modified click (**Alt+click**) replays
  verbatim. `--no-sanitize` replays raw. Replay also kills any cataclysm.exe the
  run relaunches, leaving no stray window.
- By default replay **kills the spawned game and exits** once the trace finishes,
  leaving no stray `cataclysm.exe`; pass `--no-kill` to keep it running.
- Trace JSON: `{ metadata{window_class, client_size}, summary, events[] }` where
  each event is `{type:"cursor", t, cx, cy}` (cx/cy = raw screen point) or
  `{type:"message", t, message, wParam, lParam}`.


## The symbol map
- Default map is `.ida-snapshot.json` — **functions only**. To watch named data
  globals (e.g. `universe`) by name, build a map that includes globals:
  ```powershell
  & $py tools\frida\build_address_map.py --include-globals --output tools\frida\cataclysm_addresses.json
  & $py tools\frida\cataclysm_runtime.py --map tools\frida\cataclysm_addresses.json memwrite-watch --symbol universe --offset 0x1ac
  ```
- Or just pass `--field-addr <absolute VA>` and skip name resolution.

## Notes & limitations
- Retail has **no `/new`-style command line**; it boots to the frontend menu and
  must be driven to a mission by hand. `univUpdateCounter` only advances once a
  mission is running, so `[frame=N]` stays 0 at the menu. Use `input-record` once to
  capture yourself driving into a mission, then `input-replay` to get there unattended.
- This build delivers hardware-watchpoint hits as `single-step` exceptions with no
  data-address detail, so writes are attributed by the faulting instruction (`pc`).
  Up to 4 watchpoints (debug-register slots 0–3) can be armed at once.
- Watchpoints are armed on threads that exist at install time plus any created
  later (auto re-armed). `--slot` selects the debug register.
- The CET DLL is an ASan/debug build; spawn + teardown is slow, and a killed run
  can occasionally leave a lingering `cataclysm.exe` to clean up manually.
- `Ship.shiptype` is retail-backed at `+0x2B0`; heavy interceptors are `68` / `82`.
- The nearby `Ship.shipID.shipNumber` offset is not yet retail-proven, so avoid
  using the old `+0x488` shortcut as authoritative evidence.

## Files
| File | Purpose |
|------|---------|
| `cataclysm_runtime.py` | Host CLI: spawn/attach, IDA map, ParityLog emitter, commands |
| `cataclysm_agent.js`   | Frida agent: watchpoints, function-trace, call-log, input record/replay, CET load |
| `build_address_map.py` | Refresh the symbol map from the IDA MCP server |
