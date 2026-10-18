import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


DEFAULT_PROGRESS_PATH = Path(__file__).resolve().parents[2] / ".ida-typing-progress.json"


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def normalize_address(address: str) -> str:
    value = address.strip().lower()
    if not value.startswith("0x"):
        value = f"0x{value}"
    return value


def load_progress(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    if "functions" not in data or not isinstance(data["functions"], list):
        raise ValueError(f"{path} does not contain a top-level 'functions' list")
    return data


def save_progress(path: Path, data: dict[str, Any]) -> None:
    data["last_updated"] = utc_now()
    data["functions"].sort(
        key=lambda item: (
            str(item.get("worker_id", "")),
            int(item.get("order_index", 1 << 30)),
            str(item.get("address", "")),
        )
    )
    with path.open("w", encoding="utf-8", newline="\n") as f:
        json.dump(data, f, indent=2)
        f.write("\n")


def find_record(data: dict[str, Any], address: str) -> dict[str, Any] | None:
    normalized = normalize_address(address)
    for item in data["functions"]:
        if normalize_address(str(item.get("address", ""))) == normalized:
            return item
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description="Update .ida-typing-progress.json for a typed function.")
    parser.add_argument("--path", default=str(DEFAULT_PROGRESS_PATH), help="Path to the typing progress JSON file.")
    parser.add_argument("--address", required=True, help="Function address, for example 0x490660.")
    parser.add_argument("--current-name")
    parser.add_argument("--module")
    parser.add_argument("--source-file")
    parser.add_argument("--worker-id")
    parser.add_argument("--order-index", type=int)
    parser.add_argument("--priority", type=int)
    parser.add_argument("--queue-status")
    parser.add_argument("--typing-status")
    parser.add_argument("--typing-reason")
    parser.add_argument("--evidence")
    parser.add_argument("--notes")
    parser.add_argument("--baseline-header")
    parser.add_argument("--current-header")
    parser.add_argument("--change", action="append", default=[], help="Append an applied change entry.")
    parser.add_argument("--clear-changes", action="store_true", help="Clear applied_changes before adding any new --change entries.")
    parser.add_argument("--create-missing", action="store_true", help="Create a new record if the address is not present.")
    args = parser.parse_args()

    path = Path(args.path)
    data = load_progress(path)
    record = find_record(data, args.address)

    if record is None:
        if not args.create_missing:
            raise SystemExit(f"Address {args.address} was not found in {path}. Use --create-missing to add it.")
        record = {
            "address": normalize_address(args.address),
            "current_name": args.current_name or f"sub_{normalize_address(args.address)[2:].upper()}",
            "module": args.module or "",
            "source_file": args.source_file or "",
            "worker_id": args.worker_id or "",
            "order_index": args.order_index if args.order_index is not None else 0,
            "priority": args.priority if args.priority is not None else 0,
            "queue_status": args.queue_status or "pending",
            "typing_status": args.typing_status or "pending",
            "typing_reason": args.typing_reason or "",
            "evidence": args.evidence or "",
            "notes": args.notes or "",
            "baseline_header": args.baseline_header or "",
            "current_header": args.current_header or args.baseline_header or "",
            "applied_changes": [],
            "last_updated": utc_now(),
        }
        data["functions"].append(record)

    updates = {
        "address": normalize_address(args.address),
        "current_name": args.current_name,
        "module": args.module,
        "source_file": args.source_file,
        "worker_id": args.worker_id,
        "order_index": args.order_index,
        "priority": args.priority,
        "queue_status": args.queue_status,
        "typing_status": args.typing_status,
        "typing_reason": args.typing_reason,
        "evidence": args.evidence,
        "notes": args.notes,
        "baseline_header": args.baseline_header,
        "current_header": args.current_header,
    }

    for key, value in updates.items():
        if value is not None:
            record[key] = value

    if args.clear_changes or "applied_changes" not in record or not isinstance(record["applied_changes"], list):
        record["applied_changes"] = []
    if args.change:
        record["applied_changes"].extend(args.change)

    record["last_updated"] = utc_now()
    save_progress(path, data)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
