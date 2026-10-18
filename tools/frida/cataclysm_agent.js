'use strict';

// Frida agent for Cataclysm parity instrumentation.
//
// Ported and trimmed from the QFG5 runtime agent. Differences:
//   - Targets the Frida 17 module API (Process.getModuleByName(...).getExportByName).
//   - Adds a precise memory-writer watch built on hardware watchpoints, plus the
//     page-granular MemoryAccessMonitor write watch as a fallback.
//   - Window-only input record/replay: messages via DispatchMessageA/PostMessageA,
//     mouse position via GetCursorPos capture/override (never moves the real cursor).
//   - All addresses are absolute and already rebased by the host (IDA map -> runtime).
//
// The host (cataclysm_runtime.py) owns symbol resolution from the IDA function map
// and turns the structured `send()` events below into ParityLog-style lines:
//   [frame=N] channel: key=value key=value ...

const hooks = new Map();

let frameSource = null;       // { address: NativePointer } -> univ.univUpdateCounter
const memWatches = new Map(); // id -> watch descriptor
let exceptionHandlerInstalled = false;

function tryOrError(callback) {
    try {
        return { ok: true, value: callback() };
    } catch (error) {
        return { ok: false, error: String(error) };
    }
}

function coerce(payload) {
    return typeof payload === 'string' ? JSON.parse(payload) : payload;
}

function parseIntLoose(value, fallback) {
    if (value === undefined || value === null) {
        return fallback;
    }
    return typeof value === 'string' ? parseInt(value, 0) : value;
}

function toHex(buffer) {
    const bytes = new Uint8Array(buffer);
    let hex = '';
    for (const value of bytes) {
        hex += value.toString(16).padStart(2, '0');
    }
    return hex;
}

function fromHex(text) {
    if (text.length % 2 !== 0) {
        throw new Error('Hex payload must contain an even number of characters');
    }
    const bytes = [];
    for (let index = 0; index < text.length; index += 2) {
        bytes.push(parseInt(text.substring(index, index + 2), 16));
    }
    return bytes;
}

function readFrame() {
    if (frameSource === null) {
        return null;
    }
    const result = tryOrError(() => frameSource.address.readU32());
    return result.ok ? result.value : null;
}

function getTargetModule(moduleName) {
    if (moduleName && moduleName.length > 0) {
        return Process.getModuleByName(moduleName);
    }
    return Process.mainModule;
}

function captureArgs(args, count) {
    const values = [];
    for (let index = 0; index < count; index += 1) {
        values.push(args[index].toString());
    }
    return values;
}

function parseDecodeU16Specs(specsPayload) {
    const specs = coerce(specsPayload) || [];
    return specs.map((spec, index) => {
        const parts = String(spec).split(':');
        if (parts.length !== 3) {
            throw new Error(`decode-u16 spec must be LABEL:ARG_INDEX:OFFSET (got '${spec}')`);
        }
        return {
            label: parts[0] && parts[0].length > 0 ? parts[0] : `u16_${index}`,
            argIndex: parseIntLoose(parts[1], 0),
            offset: parseIntLoose(parts[2], 0)
        };
    });
}

function parseFilterU16Specs(specsPayload) {
    const specs = coerce(specsPayload) || [];
    return specs.map((spec) => {
        const parts = String(spec).split(':');
        if (parts.length !== 3) {
            throw new Error(`filter-u16 spec must be ARG_INDEX:OFFSET:VALUE (got '${spec}')`);
        }
        return {
            argIndex: parseIntLoose(parts[0], 0),
            offset: parseIntLoose(parts[1], 0),
            value: parseIntLoose(parts[2], 0)
        };
    });
}

function parseDecodeU32Specs(specsPayload) {
    const specs = coerce(specsPayload) || [];
    return specs.map((spec, index) => {
        const parts = String(spec).split(':');
        if (parts.length !== 3) {
            throw new Error(`decode-u32 spec must be LABEL:ARG_INDEX:OFFSET (got '${spec}')`);
        }
        return {
            label: parts[0] && parts[0].length > 0 ? parts[0] : `u32_${index}`,
            argIndex: parseIntLoose(parts[1], 0),
            offset: parseIntLoose(parts[2], 0)
        };
    });
}

function parseDecodeF32Specs(specsPayload) {
    const specs = coerce(specsPayload) || [];
    return specs.map((spec, index) => {
        const parts = String(spec).split(':');
        if (parts.length !== 3) {
            throw new Error(`decode-f32 spec must be LABEL:ARG_INDEX:OFFSET (got '${spec}')`);
        }
        return {
            label: parts[0] && parts[0].length > 0 ? parts[0] : `f32_${index}`,
            argIndex: parseIntLoose(parts[1], 0),
            offset: parseIntLoose(parts[2], 0)
        };
    });
}

function parseFilterU32Specs(specsPayload) {
    const specs = coerce(specsPayload) || [];
    return specs.map((spec) => {
        const parts = String(spec).split(':');
        if (parts.length !== 3) {
            throw new Error(`filter-u32 spec must be ARG_INDEX:OFFSET:VALUE[,VALUE...] (got '${spec}')`);
        }
        return {
            argIndex: parseIntLoose(parts[0], 0),
            offset: parseIntLoose(parts[1], 0),
            values: String(parts[2]).split(',').map((value) => parseIntLoose(value, 0))
        };
    });
}

function readPointerArgU16(args, argIndex, offset) {
    const arg = args[argIndex];
    if (arg === undefined || arg === null) {
        return { ok: false, error: `arg${argIndex} missing`, value: null };
    }
    const base = ptr(arg);
    if (base.isNull()) {
        return { ok: true, value: null };
    }
    return tryOrError(() => base.add(offset).readU16());
}

function readPointerArgU32(args, argIndex, offset) {
    const arg = args[argIndex];
    if (arg === undefined || arg === null) {
        return { ok: false, error: `arg${argIndex} missing`, value: null };
    }
    const base = ptr(arg);
    if (base.isNull()) {
        return { ok: true, value: null };
    }
    return tryOrError(() => base.add(offset).readU32());
}

function readPointerArgF32(args, argIndex, offset) {
    const arg = args[argIndex];
    if (arg === undefined || arg === null) {
        return { ok: false, error: `arg${argIndex} missing`, value: null };
    }
    const base = ptr(arg);
    if (base.isNull()) {
        return { ok: true, value: null };
    }
    return tryOrError(() => base.add(offset).readFloat());
}

function captureDecodedU16Args(args, specs) {
    const decoded = [];
    for (const spec of specs) {
        const result = readPointerArgU16(args, spec.argIndex, spec.offset);
        decoded.push({
            label: spec.label,
            argIndex: spec.argIndex,
            offset: spec.offset,
            value: result.ok ? result.value : null,
            error: result.ok ? null : result.error
        });
    }
    return decoded;
}

function filtersMatchU16Args(args, specs) {
    for (const spec of specs) {
        const result = readPointerArgU16(args, spec.argIndex, spec.offset);
        if (!result.ok || result.value !== spec.value) {
            return false;
        }
    }
    return true;
}

function captureDecodedU32Args(args, specs) {
    const decoded = [];
    for (const spec of specs) {
        const result = readPointerArgU32(args, spec.argIndex, spec.offset);
        decoded.push({
            label: spec.label,
            argIndex: spec.argIndex,
            offset: spec.offset,
            value: result.ok ? result.value : null,
            error: result.ok ? null : result.error
        });
    }
    return decoded;
}

function captureDecodedF32Args(args, specs) {
    const decoded = [];
    for (const spec of specs) {
        const result = readPointerArgF32(args, spec.argIndex, spec.offset);
        decoded.push({
            label: spec.label,
            argIndex: spec.argIndex,
            offset: spec.offset,
            value: result.ok ? result.value : null,
            error: result.ok ? null : result.error
        });
    }
    return decoded;
}

function filtersMatchU32Args(args, specs) {
    for (const spec of specs) {
        const result = readPointerArgU32(args, spec.argIndex, spec.offset);
        if (!result.ok || result.value === null || spec.values.indexOf(result.value) === -1) {
            return false;
        }
    }
    return true;
}

function captureContext(context) {
    return {
        eax: context.eax ? context.eax.toString() : null,
        ebx: context.ebx ? context.ebx.toString() : null,
        ecx: context.ecx ? context.ecx.toString() : null,
        edx: context.edx ? context.edx.toString() : null,
        esi: context.esi ? context.esi.toString() : null,
        edi: context.edi ? context.edi.toString() : null,
        ebp: context.ebp ? context.ebp.toString() : null,
        esp: context.esp ? context.esp.toString() : null,
        eip: context.eip ? context.eip.toString() : null
    };
}

function backtrace(context) {
    return Thread.backtrace(context, Backtracer.ACCURATE).map((address) => address.toString());
}

// ---------------------------------------------------------------------------
// Memory monitoring helpers (shared by function-trace world diff / access set).
// ---------------------------------------------------------------------------

function normalizeRanges(payload) {
    const ranges = coerce(payload) || [];
    return ranges.map((range, index) => ({
        name: range.name || `range_${index}`,
        index,
        base: ptr(range.address || range.base),
        size: parseIntLoose(range.size, 0)
    }));
}

function snapshotRanges(ranges) {
    return ranges.map((range) => {
        const buffer = range.base.readByteArray(range.size);
        return {
            name: range.name,
            rangeIndex: range.index,
            address: range.base.toString(),
            size: range.size,
            bytes: new Uint8Array(buffer === null ? 0 : buffer)
        };
    });
}

function serializeBefore(snapshots) {
    return snapshots.map((snapshot) => ({
        name: snapshot.name,
        range_index: snapshot.rangeIndex,
        address: snapshot.address,
        size: snapshot.size,
        hex: toHex(snapshot.bytes)
    }));
}

function diffSnapshots(before, after) {
    const writeSet = [];
    for (let i = 0; i < before.length; i += 1) {
        const b = before[i].bytes;
        const a = after[i].bytes;
        let start = -1;
        for (let offset = 0; offset <= b.length; offset += 1) {
            const differs = offset < b.length && b[offset] !== a[offset];
            if (differs && start === -1) {
                start = offset;
            } else if (!differs && start !== -1) {
                writeSet.push({
                    range_name: before[i].name,
                    range_index: before[i].rangeIndex,
                    address: ptr(before[i].address).add(start).toString(),
                    offset: start,
                    size: offset - start,
                    before_hex: toHex(b.slice(start, offset)),
                    after_hex: toHex(a.slice(start, offset))
                });
                start = -1;
            }
        }
    }
    return writeSet;
}

function summarizeAccesses(accessEvents, ranges) {
    const pageSize = Process.pageSize;
    const perPage = new Map();
    for (const event of accessEvents) {
        const key = `${event.rangeIndex}:${event.pageIndex}`;
        if (!perPage.has(key)) {
            const range = ranges[event.rangeIndex];
            perPage.set(key, {
                range_name: range.name,
                range_index: event.rangeIndex,
                page_index: event.pageIndex,
                page_address: range.base.add(event.pageIndex * pageSize).toString(),
                first_access_address: event.address,
                from: event.from,
                operations: []
            });
        }
        const summary = perPage.get(key);
        if (summary.operations.indexOf(event.operation) === -1) {
            summary.operations.push(event.operation);
        }
    }
    return Array.from(perPage.values());
}

// ---------------------------------------------------------------------------
// Hardware-watchpoint based memory-writer trace ("who writes this field").
// ---------------------------------------------------------------------------

function ensureExceptionHandler() {
    if (exceptionHandlerInstalled) {
        return;
    }
    Process.setExceptionHandler((details) => {
        const pc = details.context && details.context.pc ? details.context.pc : null;
        const threadId = tryOrError(() => Process.getCurrentThreadId());
        let handled = false;
        for (const watch of memWatches.values()) {
            // Hardware watchpoints report the faulting instruction in pc; the data
            // address is in details.memory.address when the OS/Frida supplies it.
            const dataAddress = details.memory ? details.memory.address : null;
            const matchesData = dataAddress !== null && dataAddress !== undefined && !dataAddress.isNull()
                && dataAddress.compare(watch.base) >= 0
                && dataAddress.compare(watch.base.add(watch.size)) < 0;
            const isWatchException = details.type === 'breakpoint' || details.type === 'single-step';
            if (!isWatchException) {
                continue;
            }
            if (dataAddress !== null && dataAddress !== undefined && !matchesData) {
                continue;
            }

            // First few exceptions: emit raw shape so the host can confirm
            // attribution semantics for this Frida/OS build.
            if (watch.debugLeft > 0) {
                watch.debugLeft -= 1;
                send({
                    type: 'memwrite-debug',
                    watchId: watch.id,
                    exc_type: details.type,
                    has_memory: !!details.memory,
                    memory_op: details.memory ? details.memory.operation : null,
                    memory_address: dataAddress ? dataAddress.toString() : null,
                    pc: pc ? pc.toString() : null
                });
            }

            handled = true;
            watch.hits += 1;
            const valueResult = tryOrError(() => watch.base.readByteArray(watch.size));
            send({
                type: 'memwrite-hit',
                watchId: watch.id,
                channel: watch.channel,
                frame: readFrame(),
                address: watch.base.toString(),
                size: watch.size,
                data_address: dataAddress ? dataAddress.toString() : watch.base.toString(),
                from: pc ? pc.toString() : null,
                value_hex: valueResult.ok && valueResult.value ? toHex(valueResult.value) : null,
                threadId: threadId.ok ? threadId.value : null,
                context: details.context ? captureContext(details.context) : null,
                backtrace: watch.backtrace && details.context ? backtrace(details.context) : null,
                hits: watch.hits
            });
        }
        return handled;
    });
    exceptionHandlerInstalled = true;
}

function installMemWriteWatch(watchId, optionsPayload) {
    if (memWatches.has(watchId)) {
        throw new Error(`Memory watch '${watchId}' already exists`);
    }
    const options = coerce(optionsPayload) || {};
    const base = ptr(options.address);
    const size = parseIntLoose(options.size, 4);
    const conditions = options.conditions || 'w';
    const slot = parseIntLoose(options.slot, 0);

    ensureExceptionHandler();

    const armedThreads = [];
    for (const thread of Process.enumerateThreads()) {
        const result = tryOrError(() => thread.setHardwareWatchpoint(slot, base, size, conditions));
        armedThreads.push({ id: thread.id, ok: result.ok, error: result.ok ? null : result.error });
    }

    const watch = {
        id: watchId,
        channel: options.channel || 'memwrite',
        base,
        size,
        conditions,
        slot,
        backtrace: options.backtrace !== false,
        hits: 0,
        debugLeft: parseIntLoose(options.debug, 3),
        armedThreads
    };
    memWatches.set(watchId, watch);

    return {
        watchId,
        address: base.toString(),
        size,
        conditions,
        slot,
        armed_threads: armedThreads,
        thread_count: armedThreads.length
    };
}

function removeMemWriteWatch(watchId) {
    const watch = memWatches.get(watchId);
    if (!watch) {
        return { removed: false, watchId };
    }
    for (const thread of Process.enumerateThreads()) {
        tryOrError(() => thread.unsetHardwareWatchpoint(watch.slot));
    }
    memWatches.delete(watchId);
    return { removed: true, watchId, hits: watch.hits };
}

// Re-arm watchpoints on threads created after the watch was installed.
function rearmMemWriteWatches() {
    const rearmed = [];
    for (const watch of memWatches.values()) {
        const known = new Set(watch.armedThreads.map((entry) => entry.id));
        for (const thread of Process.enumerateThreads()) {
            if (known.has(thread.id)) {
                continue;
            }
            const result = tryOrError(() =>
                thread.setHardwareWatchpoint(watch.slot, watch.base, watch.size, watch.conditions));
            const entry = { id: thread.id, ok: result.ok, error: result.ok ? null : result.error };
            watch.armedThreads.push(entry);
            rearmed.push({ watchId: watch.id, ...entry });
        }
    }
    return { rearmed };
}

// ---------------------------------------------------------------------------
// Function trace (args / backtrace / world diff / page access set).
// ---------------------------------------------------------------------------

function installFunctionTrace(hookId, address, optionsPayload) {
    if (hooks.has(hookId)) {
        throw new Error(`Hook '${hookId}' already exists`);
    }
    const options = coerce(optionsPayload) || {};
    const target = ptr(address);
    const worldRanges = normalizeRanges(options.worldRanges || []);
    const accessRanges = normalizeRanges(options.accessRanges || []);
    const decodeU16Specs = parseDecodeU16Specs(options.decodeU16Args || []);
    const filterU16Specs = parseFilterU16Specs(options.filterU16Args || []);
    const decodeU32Specs = parseDecodeU32Specs(options.decodeU32Args || []);
    const decodeF32Specs = parseDecodeF32Specs(options.decodeF32Args || []);
    const filterU32Specs = parseFilterU32Specs(options.filterU32Args || []);

    const listener = Interceptor.attach(target, {
        onEnter(args) {
            try {
                if ((filterU16Specs.length > 0 && !filtersMatchU16Args(args, filterU16Specs))
                    || (filterU32Specs.length > 0 && !filtersMatchU32Args(args, filterU32Specs))) {
                    this.trace = null;
                    return;
                }
                this.trace = {
                    startedAtMs: Date.now(),
                    frame: readFrame(),
                    args: captureArgs(args, options.argCount || 0),
                    decodedU16: captureDecodedU16Args(args, decodeU16Specs),
                    decodedU32: captureDecodedU32Args(args, decodeU32Specs),
                    decodedF32: captureDecodedF32Args(args, decodeF32Specs),
                    entryContext: options.captureContext ? captureContext(this.context) : null,
                    backtrace: options.captureBacktrace ? backtrace(this.context) : null,
                    worldBefore: snapshotRanges(worldRanges),
                    accessEvents: []
                };
                if (accessRanges.length > 0) {
                    MemoryAccessMonitor.enable(
                        accessRanges.map((range) => ({ base: range.base, size: range.size })),
                        {
                            onAccess: (details) => {
                                this.trace.accessEvents.push({
                                    operation: details.operation,
                                    from: details.from.toString(),
                                    address: details.address.toString(),
                                    rangeIndex: details.rangeIndex,
                                    pageIndex: details.pageIndex
                                });
                            }
                        }
                    );
                }
            } catch (error) {
                MemoryAccessMonitor.disable();
                this.trace = null;
                send({ type: 'function-trace-error', hookId, stage: 'enter', error: String(error) });
            }
        },
        onLeave(retval) {
            try {
                MemoryAccessMonitor.disable();
                const state = this.trace;
                if (!state) {
                    return;
                }
                const worldAfter = snapshotRanges(worldRanges);
                send({
                    type: 'function-trace-capture',
                    hookId,
                    channel: options.channel || 'fntrace',
                    address: target.toString(),
                    frame: state.frame,
                    threadId: this.threadId,
                    entry: {
                        args: state.args,
                        decoded_u16: state.decodedU16,
                        decoded_u32: state.decodedU32,
                        decoded_f32: state.decodedF32,
                        context: state.entryContext,
                        backtrace: state.backtrace
                    },
                    leave: {
                        retval: retval.toString(),
                        context: options.captureContext ? captureContext(this.context) : null
                    },
                    world_before: serializeBefore(state.worldBefore),
                    write_set: diffSnapshots(state.worldBefore, worldAfter),
                    accessed_pages: summarizeAccesses(state.accessEvents, accessRanges)
                });
            } catch (error) {
                send({ type: 'function-trace-error', hookId, stage: 'leave', error: String(error) });
            } finally {
                MemoryAccessMonitor.disable();
                if (options.once !== false) {
                    const active = hooks.get(hookId);
                    if (active) {
                        active.detach();
                        hooks.delete(hookId);
                    }
                }
            }
        }
    });

    hooks.set(hookId, listener);
    return {
        hookId,
        address: target.toString(),
        worldRangeCount: worldRanges.length,
        accessRangeCount: accessRanges.length
    };
}

// ---------------------------------------------------------------------------
// Lightweight call logger (enter/leave with args), stays attached.
// ---------------------------------------------------------------------------

function installCallLog(hookId, address, optionsPayload) {
    if (hooks.has(hookId)) {
        throw new Error(`Hook '${hookId}' already exists`);
    }
    const options = coerce(optionsPayload) || {};
    const target = ptr(address);
    const decodeU16Specs = parseDecodeU16Specs(options.decodeU16Args || []);
    const filterU16Specs = parseFilterU16Specs(options.filterU16Args || []);
    const decodeU32Specs = parseDecodeU32Specs(options.decodeU32Args || []);
    const decodeF32Specs = parseDecodeF32Specs(options.decodeF32Args || []);
    const filterU32Specs = parseFilterU32Specs(options.filterU32Args || []);
    const listener = Interceptor.attach(target, {
        onEnter(args) {
            if ((filterU16Specs.length > 0 && !filtersMatchU16Args(args, filterU16Specs))
                || (filterU32Specs.length > 0 && !filtersMatchU32Args(args, filterU32Specs))) {
                this.skip = true;
                return;
            }
            this.skip = false;
            const payload = {
                type: 'call-enter',
                hookId,
                channel: options.channel || 'call',
                address: target.toString(),
                frame: readFrame(),
                threadId: this.threadId,
                args: captureArgs(args, options.argCount || 0),
                decoded_u16: captureDecodedU16Args(args, decodeU16Specs),
                decoded_u32: captureDecodedU32Args(args, decodeU32Specs),
                decoded_f32: captureDecodedF32Args(args, decodeF32Specs)
            };
            if (options.captureBacktrace) {
                payload.backtrace = backtrace(this.context);
            }
            send(payload);
        },
        onLeave(retval) {
            if (this.skip) {
                return;
            }
            if (options.captureLeave === false) {
                return;
            }
            send({
                type: 'call-leave',
                hookId,
                channel: options.channel || 'call',
                address: target.toString(),
                frame: readFrame(),
                threadId: this.threadId,
                retval: retval.toString()
            });
            if (options.once) {
                const active = hooks.get(hookId);
                if (active) {
                    active.detach();
                    hooks.delete(hookId);
                }
            }
        }
    });
    hooks.set(hookId, listener);
    return { hookId, address: target.toString() };
}

// ---------------------------------------------------------------------------
// Input record / replay (window-only; never disturbs the desktop).
//
// Cataclysm's input model (src/Win32/main.c, mouse.c):
//   - Keyboard / mouse-button / wheel arrive as WM_* messages dispatched to the
//     game's WindowProc, which updates internal key state via keyPress*().
//   - Mouse POSITION is polled with GetCursorPos(); mousePoll()/utyClientRectGet()
//     derive the in-game position as (GetCursorPos - client-origin). There is no
//     WM_MOUSEMOVE handler.
//
// So we capture/inject at exactly those two seams:
//   - messages  -> recorded from DispatchMessageA, replayed with PostMessageA to
//                  the game HWND (queue delivery; no foreground needed, no global
//                  synthetic input, so other windows and the real cursor are
//                  untouched).
//   - cursor    -> recorded as the RAW screen POINT GetCursorPos returns to the
//                  game, replayed by overriding the GetCursorPos return value with
//                  that same raw POINT. Because the game subtracts its own client
//                  origin exactly as it did at record time, the in-game position
//                  matches with no coordinate conversion on our side (record and
//                  replay must use the same resolution and window position). The
//                  real desktop cursor is never moved.
// ---------------------------------------------------------------------------

const INPUT_MESSAGE_IDS = new Set([
    0x0100, // WM_KEYDOWN
    0x0101, // WM_KEYUP
    0x0102, // WM_CHAR
    0x0104, // WM_SYSKEYDOWN
    0x0105, // WM_SYSKEYUP
    0x0201, // WM_LBUTTONDOWN
    0x0202, // WM_LBUTTONUP
    0x0203, // WM_LBUTTONDBLCLK
    0x0204, // WM_RBUTTONDOWN
    0x0205, // WM_RBUTTONUP
    0x0206, // WM_RBUTTONDBLCLK
    0x0207, // WM_MBUTTONDOWN
    0x0208, // WM_MBUTTONUP
    0x0209, // WM_MBUTTONDBLCLK
    0x020A  // WM_MOUSEWHEEL
]);

let user32 = null;
let inputRecorder = null;
let inputReplay = null;

function getUser32() {
    if (user32 !== null) {
        return user32;
    }
    const mod = Process.getModuleByName('user32.dll');
    const fn = (name, ret, argv) => new NativeFunction(mod.getExportByName(name), ret, argv);
    user32 = {
        FindWindowA: fn('FindWindowA', 'pointer', ['pointer', 'pointer']),
        PostMessageA: fn('PostMessageA', 'int', ['pointer', 'uint', 'uint', 'long']),
        GetClientRect: fn('GetClientRect', 'int', ['pointer', 'pointer']),
        EnumWindows: fn('EnumWindows', 'int', ['pointer', 'pointer']),
        GetWindowThreadProcessId: fn('GetWindowThreadProcessId', 'uint', ['pointer', 'pointer']),
        GetClassNameA: fn('GetClassNameA', 'int', ['pointer', 'pointer', 'int']),
        IsWindowVisible: fn('IsWindowVisible', 'int', ['pointer'])
    };
    return user32;
}

function detachListeners(listeners) {
    for (const listener of listeners) {
        if (listener && typeof listener.detach === 'function') {
            tryOrError(() => listener.detach());
        }
    }
}

// Find the game's top-level window. The retail and reconstructed builds register
// under different class/title strings (retail's title is "Emergence"), so we
// locate the window by OWNING PROCESS instead: enumerate top-level windows, keep
// the ones owned by this process, skip IME helper windows, and prefer a visible
// one. An optional className filter narrows further when provided.
function findGameWindow(className) {
    const u = getUser32();
    const myPid = Process.id;
    const pidBuf = Memory.alloc(4);
    const nameBuf = Memory.alloc(256);
    let best = null;
    let bestVisible = false;

    const callback = new NativeCallback((hwnd) => {
        pidBuf.writeU32(0);
        u.GetWindowThreadProcessId(hwnd, pidBuf);
        if (pidBuf.readU32() !== myPid) {
            return 1;
        }
        u.GetClassNameA(hwnd, nameBuf, 256);
        const cls = nameBuf.readUtf8String();
        // Skip input-method helper windows that share the process.
        if (cls === 'IME' || cls === 'MSCTFIME UI') {
            return 1;
        }
        if (className && cls !== className) {
            return 1;
        }
        const visible = u.IsWindowVisible(hwnd) !== 0;
        if (best === null || (visible && !bestVisible)) {
            best = hwnd;
            bestVisible = visible;
        }
        return 1;
    }, 'int', ['pointer', 'pointer']);

    u.EnumWindows(callback, NULL);

    if (best === null && className) {
        // Fallback: explicit class lookup if a filter was given.
        const cls = Memory.allocUtf8String(className);
        const hwnd = u.FindWindowA(cls, NULL);
        return hwnd.isNull() ? null : hwnd;
    }
    return best;
}

function clientSize(hwnd) {
    const u = getUser32();
    const rect = Memory.alloc(16);
    if (u.GetClientRect(hwnd, rect) === 0) {
        return null;
    }
    return { width: rect.add(8).readS32(), height: rect.add(12).readS32() };
}

function startInputRecording(optionsPayload) {
    if (inputRecorder !== null) {
        throw new Error('input recording already active');
    }
    const options = coerce(optionsPayload) || {};
    const className = options.windowClass || null;
    const recordCursor = options.recordCursor !== false;
    const recordMessages = options.recordMessages !== false;

    const state = {
        listeners: [],
        startTime: Date.now(),
        className: className,
        hwnd: findGameWindow(className),
        lastCursor: null,
        messageCount: 0,
        cursorCount: 0,
        metaSent: false
    };

    const mod = Process.getModuleByName('user32.dll');

    // Stream the window metadata (client size) once, as soon as the game window is
    // known. This lets the host persist a usable trace header even if the process
    // dies mid-recording.
    function sendMeta() {
        if (state.metaSent || state.hwnd === null) {
            return;
        }
        state.metaSent = true;
        send({
            type: 'input-record-meta',
            windowClass: className,
            clientSize: clientSize(state.hwnd)
        });
    }
    sendMeta();

    if (recordMessages) {
        // MSG layout (ia32): hwnd@0 message@4 wParam@8 lParam@12.
        state.listeners.push(Interceptor.attach(mod.getExportByName('DispatchMessageA'), {
            onEnter(args) {
                const msg = args[0];
                const message = msg.add(4).readU32();
                if (!INPUT_MESSAGE_IDS.has(message)) {
                    return;
                }
                // The window receiving input IS the game window; capture it as a
                // robust fallback if enumeration hasn't resolved one yet.
                if (state.hwnd === null) {
                    const msgHwnd = msg.readPointer();
                    if (!msgHwnd.isNull()) {
                        state.hwnd = msgHwnd;
                        sendMeta();
                    }
                }
                // Stream the event immediately so the host can persist continuously.
                send({
                    type: 'input-record-event',
                    event: {
                        type: 'message',
                        t: Date.now() - state.startTime,
                        message: message,
                        wParam: msg.add(8).readU32(),
                        lParam: msg.add(12).readS32()
                    }
                });
                state.messageCount += 1;
            }
        }));
    }

    if (recordCursor) {
        state.listeners.push(Interceptor.attach(mod.getExportByName('GetCursorPos'), {
            onEnter(args) {
                this.point = args[0];
            },
            onLeave() {
                if (this.point.isNull()) {
                    return;
                }
                // Resolve the game window lazily so the client-size metadata is
                // streamed even when the user only moves the mouse (no messages).
                if (state.hwnd === null) {
                    state.hwnd = findGameWindow(className);
                    sendMeta();
                }
                // Record the RAW screen point the game actually receives, exactly
                // as GetCursorPos returned it. The game derives its internal mouse
                // position as (GetCursorPos - clientOrigin) in mousePoll/
                // utyClientRectGet; replaying this same raw value at the same window
                // position reproduces the identical internal position with zero
                // conversion. Doing our own ScreenToClient/ClientToScreen round-trip
                // instead is lossy whenever our hwnd/origin differs from the game's
                // ghMainWindow (e.g. under the ddraw wrapper), which corrupts coords.
                const sx = this.point.readS32();
                const sy = this.point.add(4).readS32();
                // Collapse runs of identical positions to keep the trace small.
                if (state.lastCursor &&
                    state.lastCursor.x === sx &&
                    state.lastCursor.y === sy) {
                    return;
                }
                state.lastCursor = { x: sx, y: sy };
                // Stream the event immediately so the host can persist continuously.
                send({
                    type: 'input-record-event',
                    event: {
                        type: 'cursor',
                        t: Date.now() - state.startTime,
                        cx: sx,
                        cy: sy
                    }
                });
                state.cursorCount += 1;
            }
        }));
    }

    inputRecorder = state;
    return {
        active: true,
        windowClass: className,
        windowFound: state.hwnd !== null,
        clientSize: state.hwnd !== null ? clientSize(state.hwnd) : null
    };
}

function stopInputRecording() {
    if (inputRecorder === null) {
        return { active: false };
    }
    const state = inputRecorder;
    detachListeners(state.listeners);
    const result = {
        active: false,
        durationMs: Date.now() - state.startTime,
        messageCount: state.messageCount,
        cursorCount: state.cursorCount,
        windowClass: state.className,
        clientSize: state.hwnd !== null ? clientSize(state.hwnd) : null
    };
    inputRecorder = null;
    return result;
}

function startInputReplay(payload) {
    if (inputReplay !== null) {
        throw new Error('input replay already active');
    }
    const config = coerce(payload) || {};
    const className = config.windowClass || null;
    const speed = config.speed && config.speed > 0 ? config.speed : 1.0;
    const events = Array.isArray(config.events) ? config.events : [];
    const suppressSetCursorPos = config.suppressSetCursorPos !== false;

    const hwnd = findGameWindow(className);
    if (hwnd === null) {
        throw new Error('game window not found' + (className ? ' for class ' + className : ''));
    }

    // Event timestamps are recorded relative to spawn/resume (startInputRecording
    // runs just before the process is resumed). Replay must use the SAME reference
    // so input lands when the game is in the matching state. originElapsedMs is how
    // long after resume this replay is starting (window discovery takes time), so an
    // event recorded at t fires (t - originElapsedMs) from now. Do NOT rebase to the
    // first event: that anchors to "window appeared" instead of resume and fires
    // input seconds before the menu is interactive, desyncing the whole sequence.
    const originElapsedMs = (typeof config.originElapsedMs === 'number' && config.originElapsedMs > 0)
        ? config.originElapsedMs
        : 0;

    // Cursor samples, sorted by recorded time, consumed by the GetCursorPos hook.
    const cursorSamples = events
        .filter((e) => e.type === 'cursor')
        .map((e) => ({ t: e.t, cx: e.cx, cy: e.cy }))
        .sort((a, b) => a.t - b.t);

    const messages = events
        .filter((e) => e.type === 'message')
        .map((e) => ({ t: e.t, message: e.message, wParam: e.wParam, lParam: e.lParam }))
        .sort((a, b) => a.t - b.t);

    // Start the cursor at the sample matching the current (origin) record-time.
    let initialCursorIndex = 0;
    while (initialCursorIndex + 1 < cursorSamples.length &&
           cursorSamples[initialCursorIndex + 1].t <= originElapsedMs) {
        initialCursorIndex += 1;
    }

    const state = {
        listeners: [],
        timers: [],
        hwnd: hwnd,
        speed: speed,
        startTime: Date.now(),
        originElapsedMs: originElapsedMs,
        cursorSamples: cursorSamples,
        cursorIndex: initialCursorIndex,
        currentClient: cursorSamples.length > 0 ? cursorSamples[initialCursorIndex] : null,
        postedMessages: 0,
        totalMessages: messages.length,
        done: false
    };

    const u = getUser32();
    const mod = Process.getModuleByName('user32.dll');

    // Override mouse position: return the scheduled RAW screen point exactly as it
    // was recorded. The game subtracts its own client origin (utyClientRectGet) the
    // same way it did at record time, so the internal mouse position matches without
    // any conversion on our side.
    if (cursorSamples.length > 0) {
        state.listeners.push(Interceptor.attach(mod.getExportByName('GetCursorPos'), {
            onEnter(args) {
                this.point = args[0];
            },
            onLeave(retval) {
                if (this.point.isNull()) {
                    return;
                }
                // Record-time elapsed since resume == wall time since replay start
                // (scaled by speed) plus however long after resume we began.
                const recordedElapsed = (Date.now() - state.startTime) * state.speed + state.originElapsedMs;
                while (state.cursorIndex + 1 < state.cursorSamples.length &&
                       state.cursorSamples[state.cursorIndex + 1].t <= recordedElapsed) {
                    state.cursorIndex += 1;
                }
                state.currentClient = state.cursorSamples[state.cursorIndex];
                this.point.writeS32(state.currentClient.cx);
                this.point.add(4).writeS32(state.currentClient.cy);
                retval.replace(ptr(1));
            }
        }));
    }

    // Keep the real desktop cursor still: the game calls SetCursorPos when the
    // cursor is clipped to the window; no-op it during replay.
    if (suppressSetCursorPos) {
        const setCursorPosAddr = mod.getExportByName('SetCursorPos');
        const setCursorPos = new NativeCallback(function () {
            return 1;
        }, 'int', ['int', 'int']);
        Interceptor.replace(setCursorPosAddr, setCursorPos);
        state.setCursorPosReplaced = setCursorPosAddr;
        state.setCursorPosCallback = setCursorPos;
    }

    // Schedule each input message via PostMessageA at its resume-relative time.
    // Events already in the past (t < originElapsedMs) fire as soon as possible.
    let maxDelay = 0;
    for (const event of messages) {
        const delay = Math.max(0, (event.t - state.originElapsedMs) / state.speed);
        maxDelay = Math.max(maxDelay, delay);
        const timer = setTimeout(() => {
            u.PostMessageA(state.hwnd, event.message, event.wParam, event.lParam);
            state.postedMessages += 1;
        }, delay);
        state.timers.push(timer);
    }

    // Make sure the pump runs long enough to replay the trailing cursor motion too,
    // not just the last message.
    if (cursorSamples.length > 0) {
        const lastCursorDelay = Math.max(0,
            (cursorSamples[cursorSamples.length - 1].t - state.originElapsedMs) / state.speed);
        maxDelay = Math.max(maxDelay, lastCursorDelay);
    }

    state.timers.push(setTimeout(() => {
        state.done = true;
        send({ type: 'input-replay-done', posted: state.postedMessages, total: state.totalMessages });
    }, maxDelay + 1));

    inputReplay = state;
    return {
        active: true,
        windowClass: className,
        hwnd: hwnd.toString(),
        clientSize: clientSize(hwnd),
        messageCount: messages.length,
        cursorSampleCount: cursorSamples.length,
        estimatedDurationMs: maxDelay
    };
}

function stopInputReplay() {
    if (inputReplay === null) {
        return { active: false };
    }
    const state = inputReplay;
    for (const timer of state.timers) {
        clearTimeout(timer);
    }
    detachListeners(state.listeners);
    if (state.setCursorPosReplaced) {
        tryOrError(() => Interceptor.revert(state.setCursorPosReplaced));
    }
    const result = {
        active: false,
        posted: state.postedMessages,
        total: state.totalMessages,
        done: state.done
    };
    inputReplay = null;
    return result;
}

// ---------------------------------------------------------------------------
// RPC surface.
// ---------------------------------------------------------------------------

rpc.exports = {
    ping() {
        return {
            ok: true,
            pid: Process.id,
            arch: Process.arch,
            platform: Process.platform,
            pageSize: Process.pageSize,
            mainModule: {
                name: Process.mainModule.name,
                base: Process.mainModule.base.toString(),
                size: Process.mainModule.size
            }
        };
    },

    getmoduleinfo(moduleName) {
        const module = getTargetModule(moduleName);
        return {
            name: module.name,
            path: module.path,
            base: module.base.toString(),
            size: module.size
        };
    },

    loadlibrary(path) {
        const module = Module.load(path);
        return {
            name: module.name,
            path: module.path,
            base: module.base.toString(),
            size: module.size
        };
    },

    setframesource(address) {
        if (address === null || address === undefined) {
            frameSource = null;
            return { enabled: false };
        }
        frameSource = { address: ptr(address) };
        return { enabled: true, address: frameSource.address.toString(), frame: readFrame() };
    },

    getframe() {
        return { frame: readFrame() };
    },

    readmemory(address, size) {
        const buffer = ptr(address).readByteArray(size);
        return { address: ptr(address).toString(), size, hex: buffer === null ? '' : toHex(buffer) };
    },

    writememory(address, hex) {
        const bytes = fromHex(hex);
        ptr(address).writeByteArray(bytes);
        return { address: ptr(address).toString(), size: bytes.length };
    },

    installmemwritewatch(watchId, options) {
        return installMemWriteWatch(watchId, options);
    },

    removememwritewatch(watchId) {
        return removeMemWriteWatch(watchId);
    },

    rearmmemwritewatches() {
        return rearmMemWriteWatches();
    },

    listmemwritewatches() {
        return Array.from(memWatches.values()).map((watch) => ({
            watchId: watch.id,
            address: watch.base.toString(),
            size: watch.size,
            conditions: watch.conditions,
            slot: watch.slot,
            hits: watch.hits
        }));
    },

    installfunctiontrace(hookId, address, options) {
        return installFunctionTrace(hookId, address, options);
    },

    installcalllog(hookId, address, options) {
        return installCallLog(hookId, address, options);
    },

    startinputrecording(options) {
        return startInputRecording(options);
    },

    stopinputrecording() {
        return stopInputRecording();
    },

    startinputreplay(payload) {
        return startInputReplay(payload);
    },

    stopinputreplay() {
        return stopInputReplay();
    },

    findwindow(className) {
        const hwnd = findGameWindow(className || null);
        if (hwnd === null) {
            return { found: false };
        }
        return { found: true, hwnd: hwnd.toString(), clientSize: clientSize(hwnd) };
    },

    removehook(hookId) {
        const listener = hooks.get(hookId);
        if (!listener) {
            return { removed: false, hookId };
        }
        listener.detach();
        hooks.delete(hookId);
        return { removed: true, hookId };
    },

    listhooks() {
        return Array.from(hooks.keys());
    }
};
