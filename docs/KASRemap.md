# KAS Watch Function Remap Table

## Why this exists

Save files store `AITeam` FSM watch function pointers as **signed integer offsets** relative
to the `Watch_MissionNN` function for the current mission.  The offsets come from the original
`cataclysm.exe` binary and are **permanently fixed** — they never change because the original
binary never changes.

Our reconstructed binary has different function addresses on every rebuild.  Without a translation
layer, saves written by the original game would crash when loaded by our binary (and vice versa).

`KASRemap.c` solves this with a per-mission lookup table that maps between the original binary's
fixed offsets and our current build's function pointers.

```
Original cataclysm.exe save  ───┐
                                 │  original offset (e.g. -20720)
Our binary save ─────────────────┤
                                 ▼
                         kasConvertOffsetToFuncPtr
                                 │
                         table lookup
                                 │
                                 ▼
                    our build's Watch_Mission02_MissionFlow pointer
```

Because we always read *and write* the original binary's offsets (never our own addresses), saves
are **fully interchangeable** across all builds of our binary and with the original game, for any
mission that has a complete table.

## How the table works

`KASRemap.c` contains one `KASFuncOffsetEntry[]` array per implemented mission.  Each entry is:

```c
{ originalOffset, ourFuncPtr }
```

- `originalOffset` — the signed difference `(ubyte*)func - (ubyte*)Watch_MissionNN` measured
  inside the original `cataclysm.exe`.  Comes from IDA; never changes.
- `ourFuncPtr` — the address of the matching function in our binary.  Resolved at link time;
  changes on every rebuild.

At load time, `kasConvertOffsetToFuncPtr` searches the table for the saved `originalOffset` and
returns the matching `ourFuncPtr`.  At save time, `kasConvertFuncPtrToOffset` does the reverse.

`kasGetOffsetTable()` selects the right table for the current mission.  If no table exists for
a mission (i.e., the `default:` branch) the code falls back to a build-specific offset — this
**breaks cross-build save compatibility** and a `dbgFatalf` fires in the table path if any
function is missing.  Always complete the table before shipping saves.

## How to create a table for a new mission

### 1. Find the mission's `Watch_MissionNN` anchor address in IDA

```
get_function_by_name("Watch_Mission03")
→ { "address": "0x...", "name": "Watch_Mission03" }
```

Record the hex address — this is your anchor.  All offsets are `func_address - anchor_address`.

### 2. List every FSM function that is a callee of `Watch_MissionNN` or its helpers

Use `get_callees` starting from `Watch_MissionNN`, then recursively expand until you have every
Init_/Watch_ helper used by the mission.  Also check `kasFSMCreate` call sites inside the mission
to catch any FSM watch functions registered there.

Alternatively, use the session database if the mission's functions have already been named:

```sql
SELECT address, name FROM functions
WHERE name LIKE 'Watch_Mission03%' OR name LIKE 'Init_Mission03%'
ORDER BY CAST(address AS INTEGER);
```

### 3. Compute the offset for each function

For each function at address `F` with anchor at address `A`:

```
originalOffset = (signed 32-bit) (F - A)
```

IDA example:
```
Watch_Mission03            = 0x5F1000   (anchor, offset = 0)
Init_Mission03_FirstState  = 0x5F0C00   (offset = 0x5F0C00 - 0x5F1000 = -1024)
Watch_Mission03_FirstState = 0x5F0D00   (offset = 0x5F0D00 - 0x5F1000 = -768)
```

Alternatively: `get_function_by_name("Watch_Mission03")` gives the anchor; subtract from each
target address.  Keep the sign — functions before the anchor have negative offsets.

**Important:** `Watch_NoOpTeamContext` appears as a special sentinel in every mission table.
Its offset from `Watch_Mission02` in the original binary is `310992` (0x4BBC90).  For a new
mission, compute its offset from that mission's `Watch_MissionNN` anchor using IDA.

### 4. Write the table in `KASRemap.c`

Copy the Mission 02 block as a template:

```c
/* ===========================================================================
   Mission 03 remap table
   All offsets are relative to Watch_Mission03 = 0xADDRESS in cataclysm.exe.
   =========================================================================== */

extern void Init_Mission03_FirstState(void);
extern void Watch_Mission03_FirstState(void);
// ... one extern per function ...

static const KASFuncOffsetEntry kasOffsetTable_Mission03[] =
{
    { -1024, Init_Mission03_FirstState  },
    {  -768, Watch_Mission03_FirstState },
    // ...
    { NOOP_OFFSET, Watch_NoOpTeamContext },
    {          0, NULL }
};
```

Then add the new mission to `kasGetOffsetTable()`:

```c
static const KASFuncOffsetEntry *kasGetOffsetTable(void)
{
    switch (singlePlayerGameInfo.currentMission)
    {
        case 2:  return kasOffsetTable_Mission02;
        case 3:  return kasOffsetTable_Mission03;   // add this line
        default: return NULL;
    }
}
```

### 5. Verify completeness

Run the game and load (or create) a save for the new mission.  If any function is missing from
the table, `kasConvertFuncPtrToOffset` will `dbgFatalf` with a message like:

```
kasConvertFuncPtrToOffset: function 0x... not in table for mission 3
— add it to kasOffsetTable_Mission03
```

Add the missing function and its IDA-derived offset, rebuild, and retry.

### 6. Audit the mission callback closure with the script

Use `src\Scripts\audit_kas_remap.py` as the default gate while reconciling a mission:

```powershell
python src\Scripts\audit_kas_remap.py 1
python src\Scripts\audit_kas_remap.py 1 2 --check
python src\Scripts\audit_kas_remap.py 1 --emit-missing-rows --use-snapshot
python src\Scripts\audit_kas_remap.py 1 --emit-missing-rows --live-ida
python src\Scripts\audit_kas_remap.py 3 --definition-mode extracted
```

The script starts from `Init_MissionNN` and `Watch_MissionNN`, walks reachable callback installs
(`kasFSMCreate`, `kasJump`, `kasJumpTeam`, `kasfSpeechEvent`, and direct watch-function assignments),
and compares that reachable set against `kasOffsetTable_MissionNN`.

It flags three important gaps:

1. reachable callbacks that are missing from `KASRemap.c`
2. mapped callbacks that still resolve to stub bodies
3. reachable callbacks that still have no source definition

With `--emit-missing-rows`, it also resolves offsets from `.ida-snapshot.json` or the live IDA MCP
server and prints ready-to-paste `KASRemap.c` rows.  If `--check` fails, the mission's remap
coverage is not complete enough for old binary save compatibility.

By default duplicate symbol names prefer the integrated runtime sources (`Missions.c`,
`SinglePlayerMissionScripts.c`, support files) so the audit matches the current build.  Use
`--definition-mode extracted` when you explicitly want to audit the standalone `missionNN.c`
reconstruction instead.

## Troubleshooting

| Symptom | Likely cause |
|---------|--------------|
| `dbgFatalf` on save | A new Init_/Watch_ function was added to singleplayer.c but not to the table |
| `dbgFatalf` on load | A table entry's `originalOffset` doesn't match what's in the save file |
| Teams stuck after load | `kasConvertOffsetToFuncPtr` returned NULL (already logged) or the wrong pointer |
| Crashes only after rebuild | Using the build-specific fallback (no table for that mission) — add the table |

## Files

| File | Purpose |
|------|---------|
| `src/Game/KASRemap.c` | Table data and conversion functions |
| `src/Game/KAS.h` | Public declarations (`kasConvertFuncPtrToOffset`, `kasConvertOffsetToFuncPtr`) |
| `src/Game/AITeam.c` | Calls the two public functions during `SaveThisAITeam` and `FixThisAITeam` |
