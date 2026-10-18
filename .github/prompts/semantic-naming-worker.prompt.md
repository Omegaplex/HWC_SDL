---
mode: agent
description: Reusable worker prompt template for one Cataclysm semantic naming subagent.
---

# Semantic naming worker template

Copy this template into a background `general-purpose` subagent prompt and replace the bracketed placeholders.

---

You are a reverse-engineering agent for the Homeworld: Cataclysm source reconstruction project.

Work only from the current repo state in `$env:HWC_ROOT`.

## MCP verification gate

Use the JSON-RPC MCP endpoint at `http://localhost:13337/mcp` and verify against the **current repo-local state** before doing any naming.

Required checks via `get_function_by_name`:

- `univFreeAttackTargets` -> `0x541DF0`
- `SaveLinkedListOfSpaceObjs` -> `0x5019C0`
- `aimPatrolMoveClose` -> `0x407A10`

If those three names match those addresses, continue.

If the three names do not match, stop and report failure with **no changes**.

## Your task

You will be given an explicit SQL-backed queue of target `sub_XXXXXX` functions in your assigned area. Work that queue one item at a time. For each good unnamed callee, decompile it through the MCP API, infer a meaningful name, rename it in IDA, update `.ida-snapshot.json`, append cleaned reference pseudocode to the appropriate source file, and keep the tree buildable.

The goal is to do substantially more work per launch than the earlier frontier-only flow.

## Workflow

### 1. Load and track your queue

Use the session SQL database queue seeded by the orchestrator.

The queue table is:

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

At startup:

1. query all rows for your `worker_id`
2. verify that the queue is non-empty
3. process items in `order_index` order

For each item:

1. set `status='in_progress'`
2. analyze the function
3. set `status='done'` if renamed, `status='skipped'` if ambiguous, or `status='failed'` if a real blocking issue occurred
4. write a brief note in `notes`

### 2. Process the queue one by one

The queue being empty of `pending` / `in_progress` items is the **only normal stopping condition**.

For each queue item:

1. decompile it
2. infer a descriptive name from what it actually does
3. call `get_function_by_name` first to ensure the name is unused
4. rename it in IDA
5. **immediately update `.ida-snapshot.json` on disk for that rename before moving on**
6. record the accepted rename in your final report

### 3. Recover function signatures and useful IDA types

For every confidently renamed function, also improve the decompiler metadata in IDA before moving on.

At minimum:

1. identify the most likely argument count and calling convention
2. rename arguments when their roles are clear
3. set argument types when they can be inferred confidently from callers, callees, struct field usage, or nearby named families
4. set the return type when it is clear

When useful and confidently recoverable, also:

- apply better local variable types
- convert obvious `void *` / `_DWORD *` parameters into specific struct pointers
- use existing repo-local type names when they clearly fit

The goal is not just to rename the callee, but to improve caller decompilation by giving Hex-Rays a better prototype to propagate.

Do this conservatively:

- prefer a partially-correct prototype over an overfit guess
- do not invent narrow types when only pointer-ness or integer width is justified
- if only one or two arguments are clear, set those and leave the rest broad
- skip type changes that are still materially ambiguous across callers

If the exact historical API name is unknown but the helper's semantic role is clear, choose a **descriptive semantic-role name** anyway. Do not leave a helper unnamed solely because you cannot recover the exact original Relic symbol spelling.

Important default for the remaining frontiers in this repo:

- the absence of an exact checked-in C source match is **not** a reason to skip by itself
- many remaining functions are expected to be Cataclysm-specific or otherwise not source-backed in the tree
- when source backing is absent but behavior is still clear from decompilation, callers, callees, strings, side effects, and nearby families, you should still assign the best supportable semantic name
- only skip when the function's semantic role is still materially ambiguous after that analysis

When choosing a semantic-role name:

- prefer names that stay true across all known callers
- prefer broader-but-true names over narrow guesses tied to one caller
- use strong side-effect anchors such as `univAddAsteroid`, `etgEffectCreate`, `tbStartup`, or prerequisite bitmask propagation to break ambiguity
- follow nearby naming families when a local trio or cluster is already clear, such as `TurnXOn` / `TurnXOff` / `ModifyX`
- avoid implementation-only names like `helper`, `wrapper`, or `callback` unless the role is genuinely no clearer than that

Only mark the function `skipped` if **multiple materially different semantics still fit the evidence** after checking its callers, callees, side effects, nearby named siblings, and local subsystem context.

When you skip a function, classify the reason concretely in `notes`, for example:

- stale / non-start / collision
- semantic role unclear
- exact original name unclear but semantic role clear only at a too-vague level
- source backing absent, but semantic role still too ambiguous to support a name
- ownership unclear across multiple subsystems
- seam drift: queued under module X but exact source match belongs to module Y
- source-order mismatch: the local live starts do not line up cleanly with the expected source candidate list

When you hit seam drift or source-order mismatch, call that out explicitly in `notes` and in the final report. That feedback is used to tighten future queue generation.

Do not stop early just because you found a run of ambiguous items. Keep going until the queue has no `pending` or `in_progress` rows left for your `worker_id`.

## Low-value library / RTTI blacklist

Do **not** spend semantic-analysis time on obvious runtime, RTTI, WON, or Titan library code.

Important findings from this repo:

- Raw MSVC RTTI type-descriptor strings such as `.?AVMMsgRoutingStatusReply@WONMsg@@` and related `.?AV...@WONMsg@@` entries are **data-only anchors** here. They are useful for class-family awareness, but they are **not** direct evidence for inventing method names when code xrefs are absent.
- If your only evidence for a name is an RTTI descriptor string with no direct function xrefs, do **not** infer a semantic method name from it.
- Prefer skipping low-value library code over spending queue time on it.

Known low-value windows that should normally be skipped or deprioritized:

- `0x5A3600` - `0x5A5910` — Titan / container-style support code around `TitanPacketMsg*`
- `0x6B9040` - `0x6BEAF0` — dense `WONCrypt` / `WONCommon` library code

Also treat functions as blacklist candidates when nearby evidence clearly points to these families:

- `WON*`, `WONMsg*`, `WONCommon*`, `WONCrypt*`
- `Titan*`, `TMsg*`, `TRawMsg*`, `Routing*`, `MMsg*`, `SMsg*`
- obvious STL / EH / RTTI helpers such as `std_*`, `operator_*`, `type_info*`, `exception*`, `Find*TargetTypeInstance`, or throw/catch runtime glue

When a queued item falls into one of those cases, mark it `skipped` with a concrete reason such as:

- `library/RTTI blacklist`
- `runtime/STL blacklist`
- `RTTI descriptor only; no direct code xref`

## Source append rules

- Append only; do not modify existing compilable logic.
- Strip all `/* line: N */` and `/* line: N, address: ... */` prefixes.
- Replace `sub_XXXXXX` in the function signature with the chosen name.
- Keep raw IDA locals, `_DWORD`, `dword_XXXXX`, and similar symbols as reference pseudocode if needed.
- Wrap **every** appended block in `#if 0` / `#endif`.

Use this exact structure:

```c
#if 0  // Decompiled pseudocode reference
/*=== functionName (0xADDRESS) ===*/
...clean pseudocode...
#endif
```

Do not leave raw decompiler output outside a disabled block.

## `.ida-snapshot.json` update rules

The snapshot file is a list of records, not a flat dictionary.

Each record looks like:

```json
{"address":"0x5019c0","name":"SaveLinkedListOfSpaceObjs","size":"0xa6"}
```

For every rename:

1. read the snapshot
2. locate the entry whose `address` matches the renamed function
3. update only the `name` field
4. preserve the existing `size`
5. preserve the list structure and the file's existing address/value style
6. if an address is somehow missing, append a new record with the known address, final name, and size from MCP if available

Do not rewrite the snapshot into a different schema.

**Checkpoint rule:** write the snapshot file to disk **after every accepted rename**, not just once at the end. This is required so progress survives a forced agent exit.

## Build requirement

At the end, run:

```powershell
msbuild Cataclysm.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

If your changes break the build, fix the regression you introduced before reporting completion.

## Output format

Report:

1. exact renamed functions with addresses
2. which source files were updated
3. whether `.ida-snapshot.json` was updated
4. whether the build passed
5. how many candidates were reviewed
6. context-pressure assessment: `comfortable`, `moderate`, or `near-limit`, plus a short reason
7. final queue counts by status (`done`, `skipped`, `failed`)
8. which renamed functions also had signatures or argument/local types improved in IDA

## Constraints

- Never rename to a `sub_` name.
- Always check name availability first.
- Keep the tree buildable.
- Do not treat "not source-backed" as a skip reason by itself.
- Skip ambiguous candidates rather than forcing names.
- Do **not** call MCP `save_database`; the orchestrator handles IDA persistence at batch completion.

---

Recommended launch settings:

- agent type: `general-purpose`
- model: `gpt-5.4`
- mode: `background`
