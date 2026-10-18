---
name: semantic-naming-batches
description: Run the Cataclysm semantic naming workflow. Use this when asked to continue naming batches, dispatch reverse-engineering subagents, verify IDA/MCP state, update .ida-snapshot.json, save cataclysm.i64, validate the build, or commit a semantic naming batch.
---

# Cataclysm semantic naming batches

Use this skill for the established multi-agent workflow that identifies unnamed Cataclysm functions, records the names, appends cleaned reference pseudocode, validates the build, and commits batch results.

This skill is for **orchestration plus worker guidance**. The detailed prompt copies live here:

- `.github/prompts/semantic-naming-batch.prompt.md`
- `.github/prompts/semantic-naming-worker.prompt.md`

Treat those files as the checked-in prompt templates. This skill summarizes when to use them and the rules that must not be skipped.

## When to use this skill

Use this skill when the task is any of:

- continue the Cataclysm semantic naming campaign
- launch multiple reverse-engineering agents in one batch
- mine named functions for unnamed callees
- rename functions in IDA and sync the repo-local artifacts
- validate and commit a naming batch
- restart or correct a naming batch after MCP verification confusion

## Core repo artifacts

- repo root: `$env:HWC_ROOT`
- IDA database to persist: `$env:HWC_ROOT\cataclysm.i64`
- IDA snapshot: `$env:HWC_ROOT\.ida-snapshot.json`
- build command:

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

## MCP verification rule

Verify the live MCP against the **current repo-local state**, not against stale addresses from older runs.

The stabilized repo-local checks from this session are:

- `univFreeAttackTargets` -> `0x541DF0`
- `SaveLinkedListOfSpaceObjs` -> `0x5019C0`
- `aimPatrolMoveClose` -> `0x407A10`

Use `get_function_by_name` for those three names. If those addresses match, proceed.

Do **not** reject the MCP session because `get_metadata.path` shows `$env:CATACLYSM_PATH\cataclysm.exe`. In this repo that reflects the original loaded executable path, not proof that the wrong `.i64` is open.

## Orchestrator workflow

### 1. Pick non-overlapping module clusters

Dispatch a multi-agent batch using the number of workers the user asked for. Choose module groups that do not overlap heavily in addresses or output files.

Recent productive clusters included:

- `Universe`, `univupdate`, `consMgr`, `trails`, `statscript`
- `UIcontrols`, `FeFlow`, `Ship`, `dock`, `render`, `CommandLayer`
- `Missions`, `KAS`, `KASFunc`, `singleplayer`, `Subtitle`
- `SaveGame`, `Ship`, `dock`, `CommandNetwork`, `render`

### 2. Seed explicit worker queues

Before launching workers, seed an explicit SQL queue of target `sub_XXXXXX` addresses for each worker in the session database.

Use:

```sql
CREATE TABLE IF NOT EXISTS naming_queue (
  worker_id TEXT,
  address TEXT,
  module TEXT,
  source_file TEXT,
  current_name TEXT,
  order_index INTEGER,
  status TEXT DEFAULT 'pending',
  notes TEXT,
  PRIMARY KEY (worker_id, address)
);
```

Each worker should receive:

- a unique `worker_id`
- an explicit ordered list of target `sub_XXXXXX` addresses
- enough queue items to keep it busy for the full batch, with a hard minimum of **40 reviewed items per worker queue** and a normal target of **60-80 reviewed items per worker**

The queue being empty is the worker's only normal stopping condition.

Queue seeding must be validated against **live IDA function starts** at launch time. Do not build queues from stale candidate lists or raw address ranges alone.

For every queued address, require all of:

- `get_function_by_address` or `list_functions` shows that exact address is a current function start
- the current live IDA name at that start still begins with `sub_`
- the address is not already named in live IDA or `.ida-snapshot.json`

Reject candidates that resolve to:

- a mid-function offset inside a larger named or unnamed function
- an already named function
- an address with no current function start

Prefer **bounded seams** over loose neighborhood queues. Before seeding a worker queue, identify a local seam bounded by already-named functions from the **same source file / module family** and then map the unnamed starts inside that seam against a concrete source-backed candidate list.

When a single clean seam is too small, it is acceptable to build one worker queue from **multiple non-overlapping seams** so long as:

- the seams do not overlap in addresses
- they do not write to the same small hot region in conflicting ways
- the queue notes make the seam boundaries explicit so the worker can report ownership drift per seam
- the combined queue still stays coherent enough for the worker to carry subsystem context

For a seam to be queue-worthy, prefer all of:

- the seam is bounded on both sides by already-named anchors that belong to the same module or source file family
- you can enumerate the unresolved source-backed candidate functions in source order for that seam
- the number of plausible source candidates is close to the number of live `sub_` starts between those anchors
- the candidates mostly share a prototype family or callback shape (`void(rectangle *, listitemhandle)`, `void(char *, featom *)`, `void(featom *, regionhandle)`, etc.)

Avoid seeding seams that fail those checks, especially when:

- neighboring anchors belong to different modules, suggesting an ownership boundary
- the seam was discovered only by rough address proximity
- the source-order function count is unclear or obviously mismatched
- multiple callback/prototype families are intermixed in the same small block

When a seam is still somewhat uncertain, use a **pilot seam**: queue only the first one or two candidates, let the worker classify ownership and exact source alignment, then seed the rest of that seam from the result instead of front-loading the entire block.

Do not overuse tiny pilots. A 1-2 item pilot is for genuinely dangerous seams only. For normal batching, avoid 10-15 item queues that waste worker startup/context cost; prefer broadening the queue with additional non-overlapping seams until it reaches the target size, and do not launch a normal worker queue below **40 items** unless there is literally no larger unresolved pool available.

When a worker reports owner drift such as "not a mex helper; actually MultiplayerGame", treat that as queue-generation feedback. Record the corrected local boundary and avoid re-crossing that seam in future batches.

Batch 33 showed why this matters: loose seeding admitted stale `sub_` candidates that had become mid-function offsets or already-named entries, which wasted most of the queue on guaranteed skips.

Also reject obvious low-value library / RTTI regions from normal semantic queues unless the user explicitly wants that library work:

- `0x5A3600` - `0x5A5910` — Titan / container-style support code around `TitanPacketMsg*`
- `0x6B9040` - `0x6BEAF0` — dense `WONCrypt` / `WONCommon` support code

Treat raw RTTI type-descriptor strings like `.?AV...@WONMsg@@` as class-family hints only. In this repo they have been useful for identifying low-value subsystems, but not as direct evidence for inventing method names when code xrefs are absent.

### 3. Launch workers with the proven settings

- agent type: `general-purpose`
- model: `gpt-5.4`
- mode: `background`
- target at least **40 reviewed candidates** per worker, and preferably **60-80**, unless the entire unresolved pool is genuinely smaller

Batch names should stay consistent, for example:

- `batch26-missions-kas`
- `batch26-save-shipnet`

### 4. Use the worker prompt template

Use `.github/prompts/semantic-naming-worker.prompt.md` as the base prompt. Fill in:

- batch tag
- module list
- intended target files implied by those modules

### 5. Read worker results carefully

When a worker completes:

1. read the full result
2. if the worker stopped because of stale verification assumptions, verify MCP yourself against `.ida-snapshot.json` and the current named functions visible in IDA
3. if the live MCP matches the repo-local state, send a corrective follow-up message to the same idle worker instead of discarding it

### 6. Validate the combined batch

After all workers in the batch finish:

1. inspect `git status --short`
2. inspect `git diff --stat`
3. rebuild the project
4. confirm `Debug\cataclysm_debug.exe` is produced
5. ensure only expected files changed

Expected files usually include:

- `.ida-snapshot.json`
- `cataclysm.i64`
- touched `src\Game\*.c` or `src\Win32\*.c`

### 7. Save IDA before committing

This is mandatory.

Before committing, call MCP `save_database` so `cataclysm.i64` is persisted, and wait for it to finish before staging files:

```json
{"jsonrpc":"2.0","id":1,"method":"save_database","params":{}}
```

If `save_database` fails, stop and report the failure instead of committing.

Do **not** split the save into a follow-up commit. The batch commit should include the already-saved `cataclysm.i64`.

### 8. Commit the batch

Stage only the intended files:

- `.ida-snapshot.json`
- `cataclysm.i64` when renames were made
- touched source files

Use a concise batch-oriented commit message, for example:

```text
Batch 26: 9 functions named (KAS helpers and save/network serialization)
```

## Worker responsibilities

Each worker should:

1. load its explicit SQL queue
2. process items one by one in order
3. call `get_function_by_name` before every rename
4. rename in IDA when confident
5. recover the best available function signature in IDA for each accepted rename
6. apply confident argument / return / obvious local types when they improve caller-side decompilation
7. update `.ida-snapshot.json` immediately after every accepted rename
8. mark queue items `done`, `skipped`, or `failed`
9. append cleaned reference pseudocode to the owning source file
10. run the Win32 Debug build and leave the tree buildable

Workers should not call MCP `save_database`; the orchestrator saves IDA once at batch completion.

## `.ida-snapshot.json` rules

This file is a **list of records**, not a flat map.

Each entry looks like:

```json
{"address":"0x5019c0","name":"SaveLinkedListOfSpaceObjs","size":"0xa6"}
```

For each renamed function:

1. find the entry with the matching `address`
2. update only the `name`
3. preserve `size`
4. preserve the list structure and formatting style
5. if the address is missing, append a new record using the known address, final name, and size if available

Do not rewrite the snapshot into another schema.

## Source append rules

All appended pseudocode is **reference only**.

Rules:

1. append only
2. strip `/* line: N */` and `/* line: N, address: ... */`
3. replace `sub_XXXXXX` in the signature with the final chosen name
4. keep raw IDA locals and types if necessary
5. wrap every appended block in `#if 0` / `#endif`

Required format:

```c
#if 0  // Decompiled pseudocode reference
/*=== functionName (0xADDRESS) ===*/
...clean pseudocode...
#endif
```

Do not leave raw decompiler output outside disabled blocks. Earlier runs broke the build that way.

## Useful session lessons

- build preservation matters more than raw rename count
- conservative agents outperform aggressive ones
- stale MCP validation assumptions can waste a whole batch
- the fastest recovery is usually to correct and reuse the idle worker
- the worker prompt should explicitly mention `.ida-snapshot.json` update semantics
- saving IDA before commit prevents divergence between `cataclysm.i64` and the snapshot
- per-rename snapshot writes are the safest checkpoint when workers cannot compact their own context
- workers should report qualitative context pressure (`comfortable`, `moderate`, `near-limit`) instead of guessing a fake token percentage
- explicit SQL queues prevent workers from declaring victory too early when the first local frontier runs dry
- queue generation must normalize candidates against live IDA starts first; otherwise stale `sub_` addresses turn into mid-function or already-named skips
- queue generation should start from bounded, same-owner seams with a source-ordered candidate list, not from rough nearby address blocks
- prototype-family clustering reduces borderline seam mistakes in UI-heavy regions where draw/init/text-entry callbacks interleave
- uncertain seams are better handled as 1-2 item pilot queues first, then expanded after the worker confirms ownership
- recovering function prototypes and confident argument types in IDA is worth doing during naming because Hex-Rays propagates those hints outward to callers
- raw RTTI type descriptors with no direct code xrefs are usually better used for blacklist/deprioritization than for naming inference
- dense Titan / WONCrypt / runtime-support pockets should be kept out of ordinary semantic queues unless the batch is intentionally targeting that library code

## Recommended usage pattern

If asked to continue the campaign:

1. verify MCP against the three repo-local sanity-check names
2. choose as many fresh clusters as the user requested
3. if a frontier tail is too small, extend it with previously skipped-but-still-unresolved items from adjacent/evidence-compatible families until each worker queue is at least 40 items
4. launch background workers with the worker template
5. read all worker results
6. rebuild
7. call MCP `save_database`
8. commit only the intended files
