---
mode: agent
description: Run a multi-agent semantic naming batch for Cataclysm, validate the results, save the IDA database, and commit the batch.
---

# Semantic naming batch orchestrator

Use this prompt when you want Copilot to run the established Cataclysm semantic naming workflow with **multiple background subagents** and then land the results safely.

## Goal

Identify additional unnamed Cataclysm functions by assigning explicit `sub_XXXXXX` queues to workers, renaming the best candidates in IDA, recording them in repo-local artifacts, appending cleaned reference pseudocode, validating the build, saving the IDA database, and committing the batch.

## Current workflow baseline

### Repository and core artifacts

- Repository root: `$env:HWC_ROOT`
- IDA database file to persist: `$env:HWC_ROOT\cataclysm.i64`
- IDA snapshot file: `$env:HWC_ROOT\.ida-snapshot.json`
- Build command:

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

### Proven MCP verification gate

Before dispatching or retrying worker agents, verify the live MCP state against the **current repo-local state**, not against stale hardcoded addresses from older runs.

At the time this workflow was stabilized, these checks matched both the repo-local metadata and the live MCP:

- `univFreeAttackTargets` -> `0x541DF0`
- `SaveLinkedListOfSpaceObjs` -> `0x5019C0`
- `aimPatrolMoveClose` -> `0x407A10`

Use `get_function_by_name` for those names. If they match those addresses, the MCP session is acceptable for this workflow.

**Do not** reject the MCP session just because `get_metadata.path` reports `$env:CATACLYSM_PATH\cataclysm.exe`. In this setup that path reflects the original loaded executable path, not proof that the wrong `.i64` is open.

### Hard-won safety rules

1. All appended pseudocode must be inside `#if 0` / `#endif`.
2. Appended pseudocode is **reference only**. Do not try to make raw IDA output compile.
3. Append only. Do not alter existing live code unless you are fixing a regression introduced by the batch itself.
4. Strip all `/* line: N */` and `/* line: N, address: 0x... */` prefixes from decompiler output before writing.
5. Replace `sub_XXXXXX` in the function signature with the chosen final name.
6. Always run the Win32 Debug build after agent work and before committing.
7. **Save the IDA database through MCP before committing.**
8. Commit `cataclysm.i64` when IDA renames were made.

## Orchestrator procedure

### 1. Pick non-overlapping module clusters

Choose fresh, non-overlapping module groups so the agents do not compete on the same functions or files. Honor the number of workers the user requested rather than assuming exactly two.

Prefer areas with:

- many existing named functions
- nearby unnamed callees
- obvious behavior from callers, strings, or adjacent confirmed code

Recent good clusters included:

- `Universe`, `univupdate`, `consMgr`, `trails`, `statscript`
- `UIcontrols`, `FeFlow`, `Ship`, `dock`, `render`, `CommandLayer`
- `Missions`, `KAS`, `KASFunc`, `singleplayer`, `Subtitle`
- `SaveGame`, `Ship`, `dock`, `CommandNetwork`, `render`

### 2. Seed explicit worker queues

Before launching workers, create and populate an explicit SQL queue of `sub_XXXXXX` candidate addresses for each worker.

Use the session SQL database. Create this table if it does not already exist:

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

For each worker:

1. generate a concrete list of target `sub_XXXXXX` addresses in the assigned area
2. insert them into `naming_queue` with a stable `order_index`
3. tell the worker its `worker_id`

The orchestrator should prefer giving each worker an explicit queue up to the target size instead of asking it to discover its own frontier from scratch.

Target queue size:

- enforce a hard minimum of **40 reviewed items per worker queue**
- prefer roughly **60-80 items per worker** when the frontier can support it
- do not launch tiny tail batches; if one seam is too small, combine **multiple non-overlapping seams** into the same worker queue
- if the current frontier itself is thin, extend the queue with previously skipped-but-still-unresolved items from adjacent or evidence-compatible families until it reaches at least 40 items

Prefer **bounded seams** over rough neighborhood sweeps when generating those queues.

Before you seed a seam, try to establish all of:

1. two already-named anchors on both sides of the seam that belong to the same module / source file family
2. a concrete source-backed list of unresolved candidate functions in source order for that seam
3. a close count match between that source-backed candidate list and the live `sub_` starts between the anchors
4. a mostly uniform prototype family in the block, such as:
   - `void(rectangle *, listitemhandle)` draw callbacks
   - `void(char *, featom *)` init or text-entry callbacks
   - `void(featom *, regionhandle)` region draw callbacks

Do **not** queue a seam just because it is nearby in address space. Deprioritize or split seams when:

- the bounding anchors appear to belong to different modules
- the number of plausible source candidates does not line up with the number of live unnamed starts
- multiple callback/prototype families are intermixed in the same small block
- the seam was inferred from rough address continuity rather than source order and xref evidence

If a seam still looks promising but uncertain, use a **pilot seam**:

1. queue only the first one or two candidates
2. let the worker confirm ownership and exact source alignment
3. seed the remainder only after that confirmation

When a worker reports owner drift or seam bleed, feed that result back into future queue generation instead of treating it as noise. Example: if a queued `mex` seam actually resolves into `MultiplayerGame`, treat that as a corrected local boundary and avoid crossing it in later batches.

Pilot seams are an exception, not the default. If a clean 60-80 item queue cannot be built from one seam, broaden the worker queue with additional non-overlapping seams instead of repeatedly launching tiny pilots. Do not stop broadening once the queue is only 10-20 items; keep going until it reaches at least the 40-item minimum unless there is literally no unresolved pool left.

Before seeding a candidate into a queue, also reject obvious low-value library / RTTI regions so workers do not waste semantic-analysis time on them.

Current blacklist guidance:

- do not queue addresses inside `0x5A3600` - `0x5A5910` unless you are intentionally doing Titan library work
- do not queue addresses inside `0x6B9040` - `0x6BEAF0` unless you are intentionally doing WONCrypt / WONCommon library work
- avoid queueing candidates whose surrounding named family is clearly runtime / STL / EH / RTTI support (`std_*`, `operator_*`, `type_info*`, `exception*`, `Find*TargetTypeInstance`, throw/catch helpers)
- do not treat raw RTTI type-descriptor strings like `.?AV...@WONMsg@@` as direct rename evidence; they are class-family hints, not proof of a specific method identity

### 3. Launch background workers

- Agent type: `general-purpose`
- Model: `gpt-5.4`
- Mode: `background`
- One worker per cluster
- Give each worker a queue containing at least **40** target addresses, and preferably **60-80** when the frontier can support it
- It is acceptable for one worker queue to span multiple non-overlapping seams or adjacent subsystem pockets if that is what it takes to reach an efficient batch size

Name batches consistently, for example:

- `batch26-missions-kas`
- `batch26-save-shipnet`

### 4. Use the worker prompt template

Use `.github/prompts/semantic-naming-worker.prompt.md` as the worker prompt template.

For each worker, fill in:

- batch tag
- module list
- target source files implied by those modules
- remind workers that each accepted rename should also recover the best available IDA function signature and obvious argument/local types so those hints propagate to callers

### 5. Wait and read results

When a worker completes:

1. Read the result in full.
2. If it stopped because of the old metadata-path check or stale address expectations, verify the live MCP state yourself against `.ida-snapshot.json` and the current named functions visible in IDA.
3. If MCP is good, send a corrective follow-up message to the same idle worker instead of launching a brand-new agent.
4. Treat skipped items with nuance: if a worker says the exact historical name is unclear but the semantic role appears stable, spot-check representative examples yourself. Some helpers are still nameable with a broader semantic-role name.
5. Prefer workers that also improved prototypes/types in IDA; better signatures are part of the batch value because they improve surrounding decompilation.

### 6. Validate the combined batch

After all workers in the batch finish successfully:

1. Inspect `git status --short`.
2. Inspect `git diff --stat`.
3. Rebuild:

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32 /nologo /verbosity:minimal
```

4. Confirm the build still produces `Debug\cataclysm_debug.exe`.
5. Confirm only expected files changed:
   - `.ida-snapshot.json`
   - `cataclysm.i64`
   - touched `src\Game\*.c` or `src\Win32\*.c` files
   - never unrelated files unless required for a batch-caused fix

### 7. Save IDA before commit

Before committing, persist the IDA database through MCP and wait for that save to finish before staging files.

Use the HTTP JSON-RPC endpoint and call:

```json
{"jsonrpc":"2.0","id":1,"method":"save_database","params":{}}
```

If the save fails, stop and report it instead of committing a possibly stale `cataclysm.i64`.

Do **not** split this into separate commits. The batch commit should include the already-saved `cataclysm.i64`.

### 8. Commit the batch

Stage only the intended files. In particular:

- include `cataclysm.i64` if any renames were made
- include `.ida-snapshot.json`
- include the touched source files

Use concise batch-oriented commit messages, for example:

```text
Batch 26: 9 functions named (KAS helpers and save/network serialization)
```

Include the renamed functions or clusters in the body when the batch is large enough to justify it.

### 9. Report clearly

When done, report:

- batch number
- renamed function count
- key renamed functions
- files updated
- commit SHA
- build result
- worker-reported context-pressure assessment(s)
- queue completion stats

## Snapshot update notes for the orchestrator

`.ida-snapshot.json` is a **list of function records**, not a simple name map.

Each entry looks like:

```json
{"address":"0x5019c0","name":"SaveLinkedListOfSpaceObjs","size":"0xa6"}
```

When verifying worker output, confirm that:

1. each renamed address exists in the snapshot
2. the `name` field changed to the final function name
3. the `size` field was preserved
4. address casing/style stays consistent with the existing file format

Also verify that each worker drained its queue:

```sql
SELECT worker_id, status, COUNT(*)
FROM naming_queue
GROUP BY worker_id, status
ORDER BY worker_id, status;
```

## Useful verification snippet

### Verify MCP name lookup in PowerShell

```powershell
$body = @{
  jsonrpc = "2.0"
  id = 1
  method = "get_function_by_name"
  params = @{ name = "SaveLinkedListOfSpaceObjs" }
} | ConvertTo-Json -Depth 5

Invoke-RestMethod -Uri "http://localhost:13337/mcp" -Method Post -Body $body -ContentType "application/json"
```

## What this workflow has already taught us

- The strongest guardrail is build preservation, not raw rename count.
- The best agents are conservative: they skip ambiguous candidates and name only clearly-understood callees.
- Workers should still assign semantic-role names when exact original symbol spelling is unrecoverable but subsystem ownership, side effects, and helper role are clear.
- Strong anchors like authoritative source matches, local naming families, or side-effect calls (`univAddAsteroid`, `etgEffectCreate`, prerequisite propagation, etc.) often break apparent ambiguity.
- Stale verification assumptions can waste a whole batch; compare live MCP against repo-local artifacts first.
- Wrapping appended pseudocode in `#if 0` is mandatory; earlier runs broke the build by writing raw IDA output directly into live files.
- Saving IDA before commit prevents losing renames between the snapshot and checked-in `.i64`.
- Per-rename snapshot writes are the safest checkpoint when workers cannot compact their own context.
- Explicit SQL queues prevent workers from stopping early just because the first frontier ran dry.
- Bounded same-owner seams with source-order candidate lists are safer than loose address-neighborhood queues.
- Prototype-family clustering is a useful queue-generation heuristic in UI-heavy regions.
- Pilot seams are the right fallback when a block looks promising but still has module-boundary risk.
