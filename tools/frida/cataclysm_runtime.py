"""Frida runtime host for Cataclysm parity instrumentation.

Ported from the QFG5 runtime, adapted for the retail Homeworld: Cataclysm binary.

Key differences from QFG5:
  - The function-name/address map is sourced from IDA (default: the repo's
    `.ida-snapshot.json`, which is the IDA-derived 12,160-function dump). Use
    `build_address_map.py` to refresh it directly from the IDA MCP server.
  - Window-only input record/replay: capture keyboard/mouse from the retail
    window and replay it back into the game window only (PostMessage + GetCursorPos
    override), without moving the real desktop cursor.
  - Captured agent events are rendered as ParityLog-style lines so they diff
    against CET / reconstructed-build logs:  [frame=N] channel: key=value ...
  - The headline feature is `memwrite-watch`: a precise "who writes this field"
    trace built on Frida hardware watchpoints, with the writing instruction and
    backtrace resolved back to IDA function names.

Examples:
    py runtime ping
    py runtime resolve-symbol --symbol univUpdate
    py runtime memwrite-watch --symbol universe --offset 0x1ac --size 4 --duration 20
    py runtime function-trace --symbol aitrackHeadingAndUp --arg-count 3 --backtrace --duration 20

where `py runtime` == tools\frida\.venv\Scripts\python.exe tools\frida\cataclysm_runtime.py
"""

from __future__ import annotations

import argparse
import bisect
import json
import os
import pathlib
import sys
import threading
import time
from typing import Any

import frida


REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
DEFAULT_AGENT_PATH = pathlib.Path(__file__).resolve().parent / "cataclysm_agent.js"
DEFAULT_MAP_PATH = REPO_ROOT / ".ida-snapshot.json"
CATACLYSM_ROOT = pathlib.Path(os.environ["CATACLYSM_PATH"]) if os.environ.get("CATACLYSM_PATH") else None
DEFAULT_EXE_PATH = CATACLYSM_ROOT / "cataclysm.exe" if CATACLYSM_ROOT else None
DEFAULT_CWD = CATACLYSM_ROOT
DEFAULT_IMAGE_BASE = 0x400000

# Resolved retail address of universe.univUpdateCounter (see build_address_map.py /
# project notes). Used as the default ParityLog frame source.
DEFAULT_FRAME_VA = 0xB62BAC


def parse_addr(value: Any) -> int:
    if isinstance(value, int):
        return value
    return int(str(value), 0)


class SymbolMap:
    """Bidirectional IDA symbol map with rebasing to the live module base."""

    def __init__(self, entries: list[dict], image_base: int):
        self.image_base = image_base
        self.by_name: dict[str, int] = {}
        starts: list[int] = []
        records: list[tuple[int, int, str]] = []
        for entry in entries:
            name = entry.get("name")
            addr = entry.get("address")
            if not name or addr in (None, ""):
                continue
            va = parse_addr(addr)
            size = parse_addr(entry.get("size", 0)) if entry.get("size") not in (None, "") else 0
            self.by_name[name] = va
            records.append((va, va + max(size, 1), name))
        records.sort()
        self._starts = [r[0] for r in records]
        self._records = records
        self.module_base = image_base  # updated once attached

    @classmethod
    def load(cls, path: pathlib.Path) -> "SymbolMap":
        data = json.loads(path.read_text(encoding="utf-8"))
        if isinstance(data, dict):
            image_base = parse_addr(data.get("image_base", DEFAULT_IMAGE_BASE))
            entries = data.get("functions", [])
        else:
            image_base = DEFAULT_IMAGE_BASE
            entries = data
        return cls(entries, image_base)

    def set_module_base(self, base: int) -> None:
        self.module_base = base

    def name_to_runtime(self, name: str) -> int:
        if name not in self.by_name:
            raise KeyError(f"symbol not found in map: {name}")
        return self.va_to_runtime(self.by_name[name])

    def va_to_runtime(self, va: int) -> int:
        return va - self.image_base + self.module_base

    def runtime_to_va(self, runtime: int) -> int:
        return runtime - self.module_base + self.image_base

    def resolve_runtime(self, runtime: int | None) -> str:
        if runtime is None:
            return "?"
        return self.resolve_va(self.runtime_to_va(runtime))

    def resolve_va(self, va: int) -> str:
        idx = bisect.bisect_right(self._starts, va) - 1
        if 0 <= idx < len(self._records):
            start, end, name = self._records[idx]
            if start <= va < end:
                off = va - start
                return f"{name}+0x{off:x}" if off else name
        return f"0x{va:x}"


# ---------------------------------------------------------------------------
# ParityLog-style rendering of agent events.
# ---------------------------------------------------------------------------

class ParityWriter:
    def __init__(self, symbols: SymbolMap, out_path: pathlib.Path | None):
        self.symbols = symbols
        self.file = open(out_path, "w", encoding="utf-8") if out_path else None
        self.event_count = 0
        if self.file:
            self.file.write("# Parity Log v1 (frida)\n")
            self.file.write("# Format: [frame=N] channel: key=value ...\n#\n")
            self.file.flush()

    def emit(self, channel: str, frame: Any, message: str) -> None:
        frame_text = frame if frame is not None else "?"
        line = f"[frame={frame_text}] {channel}: {message}"
        print(line, flush=True)
        if self.file:
            self.file.write(line + "\n")
            self.file.flush()
        self.event_count += 1

    def _resolve(self, runtime_str: str | None) -> str:
        if runtime_str in (None, "0x0", ""):
            return "?"
        return self.symbols.resolve_runtime(parse_addr(runtime_str))

    def _u32_from_hex(self, value_hex: str | None) -> str:
        if not value_hex or len(value_hex) < 8:
            return ""
        raw = bytes.fromhex(value_hex[:8])
        return f" u32={int.from_bytes(raw, 'little')}"

    def handle(self, payload: dict) -> None:
        etype = payload.get("type")
        if etype == "memwrite-hit":
            data_addr = payload.get("data_address")
            data_text = self._resolve(data_addr) if data_addr else "?"
            value_hex = payload.get("value_hex")
            msg = (
                f"name={payload.get('watchId')} "
                f"field={data_text} addr={data_addr or payload.get('address')} "
                f"from={self._resolve(payload.get('from'))} "
                f"value=0x{value_hex or ''}{self._u32_from_hex(value_hex)} "
                f"thread={payload.get('threadId')} hits={payload.get('hits')}"
            )
            self.emit(payload.get("channel", "memwrite"), payload.get("frame"), msg)
            bt = payload.get("backtrace")
            if bt:
                frames = " <- ".join(self._resolve(f) for f in bt[:8])
                self.emit(payload.get("channel", "memwrite"), payload.get("frame"), f"  stack={frames}")

        elif etype == "memwrite-debug":
            self.emit(
                "memwrite", payload.get("frame"),
                f"  [debug] exc={payload.get('exc_type')} has_memory={payload.get('has_memory')} "
                f"mem_op={payload.get('memory_op')} mem_addr={payload.get('memory_address')} "
                f"pc={payload.get('pc')}",
            )

        elif etype == "function-trace-capture":
            fn = self.symbols.resolve_runtime(parse_addr(payload["address"]))
            entry = payload.get("entry", {})
            args = entry.get("args", [])
            writes = payload.get("write_set", [])
            decoded = entry.get("decoded_u16", [])
            decoded += entry.get("decoded_u32", [])
            decoded += entry.get("decoded_f32", [])
            decoded_text = ""
            if decoded:
                parts = []
                for item in decoded:
                    label = item.get("label", "u16")
                    value = item.get("value")
                    if value is None:
                        parts.append(f"{label}=?")
                    elif isinstance(value, float):
                        parts.append(f"{label}={value:.3f}")
                    else:
                        parts.append(f"{label}={value}")
                decoded_text = " " + " ".join(parts)
            msg = (
                f"fn={fn} args=[{','.join(args)}]{decoded_text} "
                f"retval={payload.get('leave', {}).get('retval')} writes={len(writes)}"
            )
            self.emit(payload.get("channel", "fntrace"), payload.get("frame"), msg)
            for write in writes:
                self.emit(
                    payload.get("channel", "fntrace"),
                    payload.get("frame"),
                    f"  write {self.symbols.resolve_runtime(parse_addr(write['address']))} "
                    f"@{write['address']} +{write['size']}B "
                    f"{write['before_hex']}->{write['after_hex']}",
                )
            bt = entry.get("backtrace")
            if bt:
                frames = " <- ".join(self.symbols.resolve_runtime(parse_addr(f)) for f in bt[:8])
                self.emit(payload.get("channel", "fntrace"), payload.get("frame"), f"  stack={frames}")

        elif etype in ("call-enter", "call-leave"):
            fn = self.symbols.resolve_runtime(parse_addr(payload["address"]))
            if etype == "call-enter":
                args = payload.get("args", [])
                decoded = payload.get("decoded_u16", [])
                decoded += payload.get("decoded_u32", [])
                decoded += payload.get("decoded_f32", [])
                decoded_text = ""
                if decoded:
                    parts = []
                    for item in decoded:
                        label = item.get("label", "u16")
                        value = item.get("value")
                        if value is None:
                            parts.append(f"{label}=?")
                        elif isinstance(value, float):
                            parts.append(f"{label}={value:.3f}")
                        else:
                            parts.append(f"{label}={value}")
                    decoded_text = " " + " ".join(parts)
                msg = f"enter {fn} args=[{','.join(args)}]{decoded_text} thread={payload.get('threadId')}"
            else:
                msg = f"leave {fn} retval={payload.get('retval')} thread={payload.get('threadId')}"
            self.emit(payload.get("channel", "call"), payload.get("frame"), msg)
            bt = payload.get("backtrace")
            if bt:
                frames = " <- ".join(self.symbols.resolve_runtime(parse_addr(f)) for f in bt[:8])
                self.emit(payload.get("channel", "call"), payload.get("frame"), f"  stack={frames}")

        elif etype in ("function-trace-error",):
            self.emit("error", None, f"{payload.get('hookId')} {payload.get('stage')}: {payload.get('error')}")
        else:
            self.emit("event", payload.get("frame"), json.dumps(payload))


# ---------------------------------------------------------------------------
# Session management.
# ---------------------------------------------------------------------------

class Session:
    def __init__(self, args):
        self.args = args
        self.symbols = SymbolMap.load(pathlib.Path(args.map))
        self.writer: ParityWriter | None = None
        self.input_recorder: "InputRecorder | None" = None
        self.script = None
        self.session = None
        self.pid = None
        self.resume_wall: float | None = None
        # cataclysm.exe pids that already existed before we spawned ours; used to
        # clean up any process the run relaunches without touching pre-existing ones.
        self._spawn_baseline_pids: set[int] = set()
        self._exe_name: str | None = None
        self._target_pid: int | None = None

    def _on_message(self, message, data):
        if message.get("type") == "send":
            payload = message.get("payload", {})
            ptype = payload.get("type") if isinstance(payload, dict) else None
            if ptype in ("input-record-event", "input-record-meta") and self.input_recorder:
                self.input_recorder.handle(payload)
            elif self.writer:
                self.writer.handle(payload)
            else:
                print(json.dumps(payload), flush=True)
        elif message.get("type") == "error":
            print(f"[agent-error] {message.get('description')}", file=sys.stderr, flush=True)

    def start(self) -> None:
        device = frida.get_local_device()
        if self.args.attach:
            attach_target = self.args.attach
            if isinstance(attach_target, str) and attach_target.isdigit():
                attach_target = int(attach_target, 10)
            self.session = device.attach(attach_target)
        else:
            program = [str(self.args.exe)] + list(getattr(self.args, "game_arg", None) or [])
            self._exe_name = pathlib.Path(self.args.exe).name.lower()
            try:
                self._spawn_baseline_pids = {
                    p.pid for p in device.enumerate_processes()
                    if p.name.lower() == self._exe_name
                }
            except Exception:
                self._spawn_baseline_pids = set()
            self.pid = device.spawn(program, cwd=str(self.args.cwd))
            self.session = device.attach(self.pid)

        agent_source = pathlib.Path(self.args.agent).read_text(encoding="utf-8")
        self.script = self.session.create_script(agent_source)
        self.script.on("message", self._on_message)
        self.script.load()

        info = self.script.exports_sync.ping()
        self._target_pid = int(info["pid"])
        module_base = parse_addr(info["mainModule"]["base"])
        self.symbols.set_module_base(module_base)
        print(
            f"[runtime] pid={info['pid']} arch={info['arch']} "
            f"module={info['mainModule']['name']} base=0x{module_base:x} "
            f"imagebase=0x{self.symbols.image_base:x}",
            file=sys.stderr,
            flush=True,
        )

        out_path = pathlib.Path(self.args.output) if getattr(self.args, "output", None) else None
        # input-record streams its own trace file to args.output; keep the
        # ParityWriter away from that path so it doesn't clobber or lock it.
        if self.input_recorder is not None:
            out_path = None
        self.writer = ParityWriter(self.symbols, out_path)

        if not getattr(self.args, "no_frame", False):
            frame_va = parse_addr(self.args.frame_addr) if self.args.frame_addr else DEFAULT_FRAME_VA
            self.script.exports_sync.setframesource(hex(self.symbols.va_to_runtime(frame_va)))

        # Replicate cataclysm_loader.exe: inject CET so retail behaves on modern OSes.
        if self.pid is not None and not getattr(self.args, "no_cet", False):
            cet_path = getattr(self.args, "cet_dll", None) or str(
                pathlib.Path(self.args.cwd) / "CataclysmEngineTweaks.dll"
            )
            if pathlib.Path(cet_path).exists():
                info = self.script.exports_sync.loadlibrary(cet_path)
                print(f"[runtime] loaded CET {info['name']} base={info['base']}",
                      file=sys.stderr, flush=True)
            else:
                print(f"[runtime] WARNING: CET DLL not found at {cet_path}; "
                      f"retail may misbehave. Use --no-cet to silence.",
                      file=sys.stderr, flush=True)

    def resume(self) -> None:
        if self.pid is not None:
            frida.get_local_device().resume(self.pid)
        # Record-time t is measured from spawn/resume; remember when resume happened
        # so input replay can anchor to the same reference regardless of how long
        # window discovery takes afterwards.
        self.resume_wall = time.time()

    def runtime_addr_for(self, args) -> int:
        if getattr(args, "field_addr", None):
            return self.symbols.va_to_runtime(parse_addr(args.field_addr))
        base = self.symbols.name_to_runtime(args.symbol)
        return base + parse_addr(getattr(args, "offset", 0) or 0)

    def pump(self, duration: float | None) -> None:
        try:
            deadline = None if duration is None else time.time() + duration
            if duration is None:
                while True:
                    if not self._target_alive():
                        self._emit_target_exit()
                        break
                    time.sleep(0.2)
            else:
                while time.time() < deadline:
                    if not self._target_alive():
                        self._emit_target_exit()
                        break
                    time.sleep(0.1)
        except KeyboardInterrupt:
            pass

    def _target_alive(self) -> bool:
        if self._target_pid is None:
            return True
        try:
            frida.get_local_device().get_process(self._target_pid)
            return True
        except frida.ProcessNotFoundError:
            return False
        except Exception:
            return True

    def _emit_target_exit(self) -> None:
        if self._target_pid is None:
            return
        print(f"[runtime] target pid {self._target_pid} exited", file=sys.stderr, flush=True)
        self._target_pid = None

    def detach(self) -> None:
        try:
            if self.script:
                self.script.unload()
        except Exception:
            pass
        try:
            if self.session:
                self.session.detach()
        except Exception:
            pass

    def kill(self) -> None:
        """Detach instrumentation and terminate the spawned game process.

        Frida leaves a spawned+resumed process running on plain detach, so
        callers that want the game gone (e.g. unattended replay) must kill it
        explicitly. Only kills processes this runtime spawned, never an
        attached-to one.
        """
        self.detach()
        if self.pid is None:
            return
        device = frida.get_local_device()
        try:
            device.kill(self.pid)
        except Exception:
            pass
        # The game can relaunch itself (a fresh cataclysm.exe outlives the pid we
        # spawned). Kill any same-named process that appeared during this run so an
        # unattended replay never leaves a window behind; leave pre-existing
        # instances alone.
        if not self._exe_name:
            return
        try:
            strays = [
                p.pid for p in device.enumerate_processes()
                if p.name.lower() == self._exe_name
                and p.pid != self.pid
                and p.pid not in self._spawn_baseline_pids
            ]
        except Exception:
            strays = []
        for spid in strays:
            try:
                device.kill(spid)
                print(f"[runtime] killed relaunched {self._exe_name} pid={spid}",
                      file=sys.stderr, flush=True)
            except Exception:
                pass


# ---------------------------------------------------------------------------
# Commands.
# ---------------------------------------------------------------------------

def cmd_ping(session: Session, args) -> None:
    session.start()
    info = session.script.exports_sync.ping()
    print(json.dumps(info, indent=2))
    session.resume()
    session.detach()


def cmd_module_info(session: Session, args) -> None:
    session.start()
    info = session.script.exports_sync.getmoduleinfo(args.module or "")
    print(json.dumps(info, indent=2))
    session.resume()
    session.detach()


def cmd_resolve_symbol(session: Session, args) -> None:
    session.start()
    va = session.symbols.by_name.get(args.symbol)
    if va is None:
        print(f"symbol not found: {args.symbol}", file=sys.stderr)
        sys.exit(1)
    runtime = session.symbols.va_to_runtime(va)
    print(json.dumps({
        "symbol": args.symbol,
        "va": hex(va),
        "runtime": hex(runtime),
        "module_base": hex(session.symbols.module_base),
    }, indent=2))
    session.resume()
    session.detach()


def cmd_memwrite_watch(session: Session, args) -> None:
    session.start()
    runtime = session.runtime_addr_for(args)
    watch_id = args.name or (args.symbol or "field")
    result = session.script.exports_sync.installmemwritewatch(watch_id, {
        "address": hex(runtime),
        "size": args.size,
        "conditions": args.conditions,
        "slot": args.slot,
        "channel": args.channel,
        "backtrace": args.backtrace,
    })
    armed = result.get("armed_threads", [])
    armed_ok = sum(1 for t in armed if t.get("ok"))
    print(
        f"[runtime] watching {session.symbols.resolve_runtime(runtime)} @0x{runtime:x} "
        f"size={args.size} cond={args.conditions} armed={armed_ok}/{result.get('thread_count')}",
        file=sys.stderr, flush=True,
    )
    for t in armed:
        if not t.get("ok"):
            print(f"[runtime]   thread {t.get('id')} arm failed: {t.get('error')}", file=sys.stderr, flush=True)
    session.resume()
    session.pump(args.duration)
    try:
        summary = session.script.exports_sync.removememwritewatch(watch_id)
        hits = summary.get("hits")
    except frida.InvalidOperationError:
        hits = "n/a (session ended)"
    print(f"[runtime] hits={hits} events={session.writer.event_count}",
          file=sys.stderr, flush=True)
    session.detach()


def cmd_function_trace(session: Session, args) -> None:
    session.start()
    runtime = session.symbols.name_to_runtime(args.symbol)
    options = {
        "argCount": args.arg_count,
        "captureBacktrace": args.backtrace,
        "captureContext": args.context,
        "once": not args.repeat,
        "channel": args.channel,
        "decodeU16Args": args.decode_u16 or [],
        "filterU16Args": args.filter_u16 or [],
        "decodeU32Args": args.decode_u32 or [],
        "decodeF32Args": args.decode_f32 or [],
        "filterU32Args": args.filter_u32 or [],
    }
    if args.world:
        options["worldRanges"] = json.loads(args.world)
    if args.access:
        options["accessRanges"] = json.loads(args.access)
    session.script.exports_sync.installfunctiontrace(args.symbol, hex(runtime), options)
    print(f"[runtime] tracing {args.symbol} @0x{runtime:x}", file=sys.stderr, flush=True)
    session.resume()
    session.pump(args.duration)
    session.detach()


def cmd_call_log(session: Session, args) -> None:
    session.start()
    options = {
        "argCount": args.arg_count,
        "captureBacktrace": args.backtrace,
        "captureLeave": not args.no_leave,
        "channel": args.channel,
        "decodeU16Args": args.decode_u16 or [],
        "filterU16Args": args.filter_u16 or [],
        "decodeU32Args": args.decode_u32 or [],
        "decodeF32Args": args.decode_f32 or [],
        "filterU32Args": args.filter_u32 or [],
    }
    for symbol in args.symbol:
        runtime = session.symbols.name_to_runtime(symbol)
        session.script.exports_sync.installcalllog(symbol, hex(runtime), options)
        print(f"[runtime] call-log {symbol} @0x{runtime:x}", file=sys.stderr, flush=True)
    session.resume()
    session.pump(args.duration)
    session.detach()


class InputRecorder:
    """Persists a streamed input trace continuously so it survives a crash/exit.

    The agent streams each captured event (and a one-time window-metadata message)
    over the Frida channel; this sink appends them and rewrites the trace file
    atomically, throttled to keep disk overhead low. Because the file is flushed as
    recording proceeds, killing the game (or it crashing) mid-record still leaves a
    usable trace on disk instead of losing everything buffered until the end.
    """

    FLUSH_INTERVAL_S = 0.5

    def __init__(self, out_path: pathlib.Path, window_class):
        self.out_path = out_path
        self.window_class = window_class
        self.events: list[dict] = []
        self.client_size = None
        self.message_count = 0
        self.cursor_count = 0
        self.start_epoch_ms = int(time.time() * 1000)
        self._last_flush = 0.0
        self._lock = threading.Lock()
        out_path.parent.mkdir(parents=True, exist_ok=True)

    def handle(self, payload: dict) -> None:
        ptype = payload.get("type")
        if ptype == "input-record-meta":
            with self._lock:
                cs = payload.get("clientSize")
                if cs:
                    self.client_size = cs
                if payload.get("windowClass") is not None:
                    self.window_class = payload.get("windowClass")
            self.flush(force=True)
            return
        if ptype == "input-record-event":
            ev = payload.get("event")
            if not isinstance(ev, dict):
                return
            with self._lock:
                self.events.append(ev)
                if ev.get("type") == "message":
                    self.message_count += 1
                elif ev.get("type") == "cursor":
                    self.cursor_count += 1
            self.flush()

    def _trace(self, duration_ms=None) -> dict:
        if duration_ms is None and self.events:
            duration_ms = self.events[-1].get("t")
        return {
            "metadata": {
                "mode": "input-record",
                "recorded_at_epoch_ms": self.start_epoch_ms,
                "duration_ms": duration_ms,
                "window_class": self.window_class,
                "client_size": self.client_size,
            },
            "summary": {
                "message_count": self.message_count,
                "cursor_count": self.cursor_count,
            },
            "events": self.events,
        }

    def flush(self, *, force: bool = False, duration_ms=None) -> None:
        now = time.time()
        with self._lock:
            if not force and (now - self._last_flush) < self.FLUSH_INTERVAL_S:
                return
            self._last_flush = now
            trace = self._trace(duration_ms)
            payload = json.dumps(trace, indent=2)
        tmp = self.out_path.with_name(self.out_path.name + ".tmp")
        tmp.write_text(payload, encoding="utf-8")
        os.replace(tmp, self.out_path)


def cmd_input_record(session: Session, args) -> None:
    out_path = pathlib.Path(args.output)
    session.input_recorder = InputRecorder(out_path, args.window_class)
    session.start()
    session.script.exports_sync.startinputrecording({
        "windowClass": args.window_class,
        "recordCursor": not args.no_cursor,
        "recordMessages": not args.no_messages,
    })
    print(
        f"[runtime] recording input"
        f"{f' (class={args.window_class})' if args.window_class else ''}; "
        f"{'until Ctrl-C' if args.duration is None else f'{args.duration:g}s'}. "
        f"Play in the game window now.",
        file=sys.stderr, flush=True,
    )
    session.resume()
    session.pump(args.duration)

    rec = session.input_recorder
    duration_ms = None
    try:
        result = session.script.exports_sync.stopinputrecording()
        duration_ms = result.get("durationMs")
        if result.get("clientSize") and not rec.client_size:
            rec.client_size = result.get("clientSize")
    except frida.InvalidOperationError:
        print("[runtime] session ended during recording; finalizing streamed trace",
              file=sys.stderr, flush=True)

    rec.flush(force=True, duration_ms=duration_ms)
    print(
        f"[runtime] wrote {out_path} "
        f"messages={rec.message_count} cursor={rec.cursor_count} "
        f"client={rec.client_size}",
        file=sys.stderr, flush=True,
    )
    session.detach()


# Windows message ids used to balance key/button state during replay.
_WM_KEYDOWN, _WM_KEYUP = 0x0100, 0x0101
_WM_SYSKEYDOWN, _WM_SYSKEYUP = 0x0104, 0x0105
_WM_LBUTTONDOWN, _WM_LBUTTONUP, _WM_LBUTTONDBLCLK = 0x0201, 0x0202, 0x0203
_WM_RBUTTONDOWN, _WM_RBUTTONUP, _WM_RBUTTONDBLCLK = 0x0204, 0x0205, 0x0206
_WM_MBUTTONDOWN, _WM_MBUTTONUP, _WM_MBUTTONDBLCLK = 0x0207, 0x0208, 0x0209

_KEY_DOWN_IDS = {_WM_KEYDOWN, _WM_SYSKEYDOWN}
_KEY_UP_IDS = {_WM_KEYUP, _WM_SYSKEYUP}
# Double-click acts as a fresh down that is later matched by a button-up.
_MOUSE_DOWN_IDS = {_WM_LBUTTONDOWN, _WM_LBUTTONDBLCLK, _WM_RBUTTONDOWN,
                   _WM_RBUTTONDBLCLK, _WM_MBUTTONDOWN, _WM_MBUTTONDBLCLK}
_MOUSE_UP_IDS = {_WM_LBUTTONUP, _WM_RBUTTONUP, _WM_MBUTTONUP}
_MOUSE_BUTTON_OF = {
    _WM_LBUTTONDOWN: "L", _WM_LBUTTONUP: "L", _WM_LBUTTONDBLCLK: "L",
    _WM_RBUTTONDOWN: "R", _WM_RBUTTONUP: "R", _WM_RBUTTONDBLCLK: "R",
    _WM_MBUTTONDOWN: "M", _WM_MBUTTONUP: "M", _WM_MBUTTONDBLCLK: "M",
}
_KEY_UP_FOR_DOWN = {_WM_KEYDOWN: _WM_KEYUP, _WM_SYSKEYDOWN: _WM_SYSKEYUP}
_MOUSE_UP_FOR_DOWN = {
    _WM_LBUTTONDOWN: _WM_LBUTTONUP, _WM_LBUTTONDBLCLK: _WM_LBUTTONUP,
    _WM_RBUTTONDOWN: _WM_RBUTTONUP, _WM_RBUTTONDBLCLK: _WM_RBUTTONUP,
    _WM_MBUTTONDOWN: _WM_MBUTTONUP, _WM_MBUTTONDBLCLK: _WM_MBUTTONUP,
}

# lParam bit 30 ("previous key state"): set on an auto-repeat WM_(SYS)KEYDOWN that
# Windows generates while a key is physically held. These are NOT new presses.
_KEY_REPEAT_BIT = 0x40000000
# Gap (ms) after the last recorded activity for a key/button at which a missing
# release is synthesized.
_SYNTH_RELEASE_GAP_MS = 30


def _to_s32(value: int) -> int:
    value &= 0xFFFFFFFF
    return value - 0x100000000 if value & 0x80000000 else value


def _event_state_key(ev: dict):
    """Return (kind, identity, is_repeat) for a key/button message, else None.

    Keyboard identity is the virtual-key (wParam) so a WM_SYSKEYDOWN is matched by
    either a WM_KEYUP or WM_SYSKEYUP for the same vk. Mouse identity is the button.
    is_repeat is True only for auto-repeat key-downs (lParam bit 30 set).
    """
    if ev.get("type") != "message":
        return None
    m = ev.get("message")
    if m in _KEY_DOWN_IDS:
        is_repeat = bool((ev.get("lParam") or 0) & 0xFFFFFFFF & _KEY_REPEAT_BIT)
        return ("down", ("key", ev.get("wParam")), is_repeat)
    if m in _KEY_UP_IDS:
        return ("up", ("key", ev.get("wParam")), False)
    if m in _MOUSE_DOWN_IDS:
        return ("down", ("mouse", _MOUSE_BUTTON_OF[m]), False)
    if m in _MOUSE_UP_IDS:
        return ("up", ("mouse", _MOUSE_BUTTON_OF[m]), False)
    return None


def _make_release_event(down_ev: dict, t: int) -> dict:
    """Build the up message that releases a held down event at time t."""
    m = down_ev.get("message")
    if m in _KEY_UP_FOR_DOWN:
        # Keep wParam (the vk the game releases); set the transition + prev-state
        # bits in lParam like a real WM_(SYS)KEYUP.
        lparam = _to_s32(((down_ev.get("lParam") or 0) & 0x00FFFFFF) | 0xC0000000)
        return {"type": "message", "t": t, "message": _KEY_UP_FOR_DOWN[m],
                "wParam": down_ev.get("wParam"), "lParam": lparam}
    # Mouse: reuse the recorded position; no buttons remain down (wParam 0).
    return {"type": "message", "t": t, "message": _MOUSE_UP_FOR_DOWN[m],
            "wParam": 0, "lParam": down_ev.get("lParam", 0)}


def sanitize_input_events(events: list) -> tuple:
    """Balance key/button state so replay can't leave a key or modifier stuck.

    Recordings are lossy around keys that trigger Windows menu-mode or a focus
    change (notably ALT): the hooked DispatchMessageA sees the down but the up is
    delivered elsewhere (e.g. the desktop after an Alt-Tab). Replaying such an
    orphan down jams the key for the rest of the session, reinterpreting later
    clicks and driving the retail front-end into a stale-region crash
    (regRegionProcess calling a freed region callback).

    Rather than dropping the press (which loses legitimate input), a missing
    release is *synthesized* shortly after the key's last recorded activity, so the
    keypress still replays but cannot stick. Auto-repeat key-downs (lParam bit 30)
    are recognized as part of an existing hold, not new presses, so a held key with
    repeats and a real up is replayed verbatim (e.g. Alt held across an Alt+click).

    Returns (clean_events, notes) where notes describes every synthesized release
    and every dropped orphan up.
    """
    indexed = list(enumerate(events))
    drop = set()
    notes: list[str] = []
    # held[identity] = stack of [down_index, last_activity_t] for unmatched presses.
    held: dict = {}

    for idx, ev in indexed:
        info = _event_state_key(ev)
        if info is None:
            continue
        kind, identity, is_repeat = info
        if kind == "down":
            stack = held.get(identity)
            if is_repeat:
                # Auto-repeat of a physically held key: extend the current hold's
                # last-activity time; if we never saw the initial press, adopt this.
                if stack:
                    stack[-1][1] = ev.get("t")
                else:
                    held[identity] = [[idx, ev.get("t")]]
            else:
                held.setdefault(identity, []).append([idx, ev.get("t")])
        else:  # up
            stack = held.get(identity)
            if stack:
                stack.pop()  # matched with its most recent press
            else:
                drop.add(idx)
                notes.append(f"drop orphan-up {identity} @t={ev.get('t')}")

    # Synthesize a release for anything still held at the end of the trace.
    synth: list[dict] = []
    for identity, stack in held.items():
        for down_idx, last_t in stack:
            up = _make_release_event(events[down_idx], int(last_t) + _SYNTH_RELEASE_GAP_MS)
            synth.append(up)
            notes.append(f"synth release {identity} @t={up['t']} "
                         f"(press @t={events[down_idx].get('t')})")

    clean = [ev for idx, ev in indexed if idx not in drop]
    clean.extend(synth)
    clean.sort(key=lambda e: e.get("t", 0))
    return clean, notes


def cmd_input_replay(session: Session, args) -> None:
    trace = json.loads(pathlib.Path(args.trace).read_text(encoding="utf-8"))
    events = trace.get("events", [])
    if not args.no_sanitize:
        events, notes = sanitize_input_events(events)
        if notes:
            print(f"[runtime] sanitized trace ({len(notes)} fix(es) to avoid stuck "
                  f"input): " + "; ".join(notes[:8])
                  + (" ..." if len(notes) > 8 else ""),
                  file=sys.stderr, flush=True)
    recorded_client = trace.get("metadata", {}).get("client_size")

    session.start()
    session.resume()

    # The window is created shortly after resume; poll for it.
    deadline = time.time() + (args.window_timeout_ms / 1000.0)
    window = {"found": False}
    while time.time() < deadline:
        window = session.script.exports_sync.findwindow(args.window_class)
        if window.get("found"):
            break
        time.sleep(0.1)
    if not window.get("found"):
        print(f"[runtime] game window not found within {args.window_timeout_ms}ms",
              file=sys.stderr, flush=True)
        session.kill() if not args.no_kill else session.detach()
        sys.exit(1)

    live_client = window.get("clientSize")
    if recorded_client and live_client and recorded_client != live_client:
        print(f"[runtime] WARNING: client size differs (recorded={recorded_client} "
              f"live={live_client}); mouse coordinates may not line up.",
              file=sys.stderr, flush=True)

    # How long after resume we are actually starting replay (window discovery,
    # script round-trips). The agent subtracts this so events keep their original
    # resume-relative timing instead of being shifted by the discovery delay.
    origin_elapsed_ms = max(0.0, (time.time() - (session.resume_wall or time.time())) * 1000.0)

    result = session.script.exports_sync.startinputreplay({
        "events": events,
        "speed": args.speed,
        "windowClass": args.window_class,
        "suppressSetCursorPos": not args.allow_set_cursor,
        "originElapsedMs": origin_elapsed_ms,
    })
    est_ms = result.get("estimatedDurationMs") or 0
    print(
        f"[runtime] replaying {result.get('messageCount')} messages / "
        f"{result.get('cursorSampleCount')} cursor samples into hwnd={result.get('hwnd')} "
        f"(~{est_ms / 1000.0:.1f}s @ {args.speed:g}x, origin=+{origin_elapsed_ms / 1000.0:.1f}s)",
        file=sys.stderr, flush=True,
    )

    settle = args.settle_ms / 1000.0
    replay_seconds = est_ms / 1000.0 + settle
    # duration of 0 (or None) means "pump for the trace length plus settle time".
    pump_for = replay_seconds if not args.duration else args.duration
    session.pump(pump_for)
    try:
        summary = session.script.exports_sync.stopinputreplay()
        print(f"[runtime] replay done posted={summary.get('posted')}/{summary.get('total')}",
              file=sys.stderr, flush=True)
    except frida.InvalidOperationError:
        print("[runtime] session ended before replay stopped", file=sys.stderr, flush=True)

    if args.no_kill:
        session.detach()
    else:
        session.kill()
        print("[runtime] game process terminated", file=sys.stderr, flush=True)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Cataclysm Frida parity runtime")
    parser.add_argument(
        "--exe",
        default=str(DEFAULT_EXE_PATH) if DEFAULT_EXE_PATH else None,
        help="Path to cataclysm.exe (spawn mode)",
    )
    parser.add_argument(
        "--cwd",
        default=str(DEFAULT_CWD) if DEFAULT_CWD else None,
        help="Working directory for spawned game",
    )
    parser.add_argument("--attach", help="Attach to a running process (name or pid) instead of spawning")
    parser.add_argument("--game-arg", action="append", help="Extra arg passed to the spawned game (repeatable), e.g. --game-arg /new --game-arg 2")
    parser.add_argument("--agent", default=str(DEFAULT_AGENT_PATH), help="Path to the Frida agent JS")
    parser.add_argument("--map", default=str(DEFAULT_MAP_PATH), help="IDA symbol map (.ida-snapshot.json or built map)")
    parser.add_argument("--output", help="Write ParityLog output to this file (in addition to stdout)")
    parser.add_argument("--frame-addr", help="Override frame-counter VA (default univUpdateCounter 0x%x)" % DEFAULT_FRAME_VA)
    parser.add_argument("--no-frame", action="store_true", help="Do not read a frame counter")
    parser.add_argument("--cet-dll", help="Path to CataclysmEngineTweaks.dll injected on spawn (default: game dir)")
    parser.add_argument("--no-cet", action="store_true", help="Do not inject the CET DLL on spawn")

    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("ping").set_defaults(func=cmd_ping)

    p = sub.add_parser("module-info")
    p.add_argument("--module", help="Module name (default: main module)")
    p.set_defaults(func=cmd_module_info)

    p = sub.add_parser("resolve-symbol")
    p.add_argument("--symbol", required=True)
    p.set_defaults(func=cmd_resolve_symbol)

    p = sub.add_parser("memwrite-watch", help="Hardware-watchpoint trace: who writes a field")
    p.add_argument("--symbol", help="Symbol whose address is the watch base")
    p.add_argument("--offset", default="0", help="Byte offset added to the symbol (e.g. 0x1ac)")
    p.add_argument("--field-addr", help="Absolute VA to watch (overrides --symbol/--offset)")
    p.add_argument("--size", type=int, default=4, help="Watch size in bytes (1/2/4/8)")
    p.add_argument("--conditions", default="w", choices=["r", "w", "rw"], help="Access conditions")
    p.add_argument("--slot", type=int, default=0, help="Hardware debug-register slot (0-3)")
    p.add_argument("--name", help="Watch id / ParityLog name")
    p.add_argument("--channel", default="memwrite")
    p.add_argument("--no-backtrace", dest="backtrace", action="store_false")
    p.add_argument("--duration", type=float, default=20.0, help="Seconds to watch (0 = until Ctrl-C)")
    p.set_defaults(func=cmd_memwrite_watch, backtrace=True)

    p = sub.add_parser("function-trace")
    p.add_argument("--symbol", required=True)
    p.add_argument("--arg-count", type=int, default=0)
    p.add_argument("--decode-u16", action="append", help="Annotate LABEL:ARG_INDEX:OFFSET as a u16 read from a pointer arg")
    p.add_argument("--filter-u16", action="append", help="Only log calls matching ARG_INDEX:OFFSET:VALUE (u16 read from a pointer arg)")
    p.add_argument("--decode-u32", action="append", help="Annotate LABEL:ARG_INDEX:OFFSET as a u32 read from a pointer arg")
    p.add_argument("--decode-f32", action="append", help="Annotate LABEL:ARG_INDEX:OFFSET as an f32 read from a pointer arg")
    p.add_argument("--filter-u32", action="append", help="Only log calls matching ARG_INDEX:OFFSET:VALUE[,VALUE...] (u32 read from a pointer arg)")
    p.add_argument("--backtrace", action="store_true")
    p.add_argument("--context", action="store_true")
    p.add_argument("--repeat", action="store_true", help="Stay attached (default: capture once)")
    p.add_argument("--world", help="JSON array of {name,address,size} ranges to diff")
    p.add_argument("--access", help="JSON array of {name,address,size} ranges to access-monitor")
    p.add_argument("--channel", default="fntrace")
    p.add_argument("--duration", type=float, default=20.0)
    p.set_defaults(func=cmd_function_trace)

    p = sub.add_parser("call-log")
    p.add_argument("--symbol", action="append", required=True,
                   help="Symbol to hook (repeatable)")
    p.add_argument("--arg-count", type=int, default=0)
    p.add_argument("--decode-u16", action="append", help="Annotate LABEL:ARG_INDEX:OFFSET as a u16 read from a pointer arg")
    p.add_argument("--filter-u16", action="append", help="Only log calls matching ARG_INDEX:OFFSET:VALUE (u16 read from a pointer arg)")
    p.add_argument("--decode-u32", action="append", help="Annotate LABEL:ARG_INDEX:OFFSET as a u32 read from a pointer arg")
    p.add_argument("--decode-f32", action="append", help="Annotate LABEL:ARG_INDEX:OFFSET as an f32 read from a pointer arg")
    p.add_argument("--filter-u32", action="append", help="Only log calls matching ARG_INDEX:OFFSET:VALUE[,VALUE...] (u32 read from a pointer arg)")
    p.add_argument("--backtrace", action="store_true")
    p.add_argument("--no-leave", action="store_true")
    p.add_argument("--channel", default="call")
    p.add_argument("--duration", type=float, default=0.0)
    p.set_defaults(func=cmd_call_log)

    p = sub.add_parser("input-record", help="Record window-only keyboard/mouse input to a trace")
    p.add_argument("--output", required=True, help="Trace JSON to write")
    p.add_argument("--window-class", default=None, help="Optional window class filter (default: any window owned by the game process)")
    p.add_argument("--no-cursor", action="store_true", help="Do not record mouse position")
    p.add_argument("--no-messages", action="store_true", help="Do not record keyboard/button/wheel messages")
    p.add_argument("--duration", type=float, default=0.0, help="Seconds to record (0 = until Ctrl-C)")
    p.set_defaults(func=cmd_input_record)

    p = sub.add_parser("input-replay", help="Replay a recorded trace into the game window only")
    p.add_argument("--trace", required=True, help="Trace JSON to replay")
    p.add_argument("--window-class", default=None, help="Optional window class filter (default: any window owned by the game process)")
    p.add_argument("--speed", type=float, default=1.0, help="Playback speed multiplier")
    p.add_argument("--settle-ms", type=int, default=500, help="Extra wait after the last event")
    p.add_argument("--window-timeout-ms", type=int, default=20000, help="How long to wait for the game window")
    p.add_argument("--allow-set-cursor", action="store_true",
                   help="Allow the game's SetCursorPos calls (moves the real cursor when clipped)")
    p.add_argument("--duration", type=float, default=0.0,
                   help="Override pump time in seconds (0 = trace length + settle)")
    p.add_argument("--no-kill", action="store_true",
                   help="Leave the game running after replay instead of terminating it")
    p.add_argument("--no-sanitize", action="store_true",
                   help="Replay raw trace without dropping unbalanced key/button presses (may leave modifiers stuck)")
    p.set_defaults(func=cmd_input_replay)

    return parser


def main() -> None:
    parser = build_parser()
    args = parser.parse_args()
    if not args.attach:
        if not args.exe:
            parser.error("Set CATACLYSM_PATH or pass --exe when spawning the retail executable.")
        if not args.cwd:
            parser.error("Set CATACLYSM_PATH or pass --cwd when spawning the retail executable.")
    args.duration = None if getattr(args, "duration", None) == 0 else getattr(args, "duration", None)
    session = Session(args)
    try:
        args.func(session, args)
    finally:
        session.detach()


if __name__ == "__main__":
    main()
