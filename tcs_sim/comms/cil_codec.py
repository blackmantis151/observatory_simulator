from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import struct
from typing import Any


CIL_CMD = 1
CIL_RSP = 2
CIL_ACK = 3
CIL_ACT = 4
CIL_COM = 5
CIL_ERR = 6

FRAME_VERSION = 1
FRAME_TX = 1
FRAME_RX = 2
FRAME_READY = 3
FRAME_ERROR = 4
FRAME_HEADER = struct.Struct("!10I")
FRAME_LENGTH = struct.Struct("!I")
MAX_FRAME_BYTES = 1024 * 1024

FIELD_TYPES = {
    "i32": "i",
    "u32": "I",
    "f32": "f",
    "f64": "d",
}


@dataclass(frozen=True)
class CilPacket:
    route_id: int
    source: int
    destination: int
    message_class: int
    service: int
    sequence: int
    timestamp_sec: int = 0
    timestamp_nsec: int = 0
    payload: bytes = b""


def load_cil_map(path: str | Path) -> dict[str, int]:
    """Load CIL names using the same sequential IDs as eCilSetup()."""
    names: dict[str, int] = {}
    with Path(path).open("r", encoding="utf-8") as stream:
        for raw_line in stream:
            line = raw_line.split("#", 1)[0].strip()
            if not line:
                continue
            parts = [part.strip() for part in line.split(",")]
            if len(parts) != 3 or not parts[0]:
                raise ValueError(f"Invalid CIL map line: {raw_line.rstrip()}")
            name = parts[0].upper()
            if name in names:
                raise ValueError(f"Duplicate CIL map name: {name}")
            names[name] = len(names) + 1
    if not names:
        raise ValueError(f"CIL map is empty: {path}")
    return names


def pack_frame(kind: int, packet: CilPacket) -> bytes:
    header = FRAME_HEADER.pack(
        FRAME_VERSION,
        kind,
        packet.route_id,
        packet.source,
        packet.destination,
        packet.message_class,
        packet.service,
        packet.sequence,
        packet.timestamp_sec,
        packet.timestamp_nsec,
    )
    body = header + packet.payload
    if len(body) > MAX_FRAME_BYTES:
        raise ValueError("CIL bridge frame is too large")
    return FRAME_LENGTH.pack(len(body)) + body


def unpack_frame(data: bytes) -> tuple[int, CilPacket]:
    if len(data) < FRAME_HEADER.size:
        raise ValueError("Short CIL bridge frame")
    values = FRAME_HEADER.unpack_from(data)
    if values[0] != FRAME_VERSION:
        raise ValueError(f"Unsupported CIL bridge frame version: {values[0]}")
    packet = CilPacket(
        route_id=values[2],
        source=values[3],
        destination=values[4],
        message_class=values[5],
        service=values[6],
        sequence=values[7],
        timestamp_sec=values[8],
        timestamp_nsec=values[9],
        payload=data[FRAME_HEADER.size :],
    )
    return values[1], packet


def decode_fields(field_specs: list[dict], payload: bytes) -> dict[str, Any]:
    fmt, fields = _compile_fields(field_specs)
    codec = struct.Struct("!" + fmt)
    if len(payload) != codec.size:
        raise ValueError(
            f"Invalid payload length {len(payload)}; expected {codec.size}"
        )
    values = codec.unpack(payload)
    decoded: dict[str, Any] = {}
    offset = 0
    for spec, count in fields:
        value: Any = values[offset] if count == 1 else list(values[offset : offset + count])
        offset += count
        enum = spec.get("enum")
        if enum and count == 1:
            reverse = {int(number): name for name, number in enum.items()}
            if int(value) not in reverse:
                raise ValueError(f"Unknown {spec['name']} value: {value}")
            value = reverse[int(value)]
        decoded[spec["name"]] = value
    return decoded


def encode_fields(field_specs: list[dict], values: dict[str, Any]) -> bytes:
    fmt, fields = _compile_fields(field_specs)
    encoded: list[Any] = []
    for spec, count in fields:
        if "const" in spec:
            value = spec["const"]
        else:
            value = _lookup(values, spec.get("source", spec["name"]))
        enum = spec.get("enum")
        if enum:
            if value not in enum:
                raise ValueError(f"Unknown {spec['name']} enum value: {value}")
            value = enum[value]
        items = value if count > 1 else [value]
        if count > 1 and (not isinstance(items, (list, tuple)) or len(items) != count):
            raise ValueError(f"Field {spec['name']} requires {count} values")
        encoded.extend(items)
    return struct.pack("!" + fmt, *encoded)


def _compile_fields(field_specs: list[dict]) -> tuple[str, list[tuple[dict, int]]]:
    fmt = ""
    fields: list[tuple[dict, int]] = []
    for spec in field_specs:
        field_type = spec.get("type")
        if field_type == "pad":
            fmt += f"{int(spec.get('bytes', 1))}x"
            continue
        if field_type not in FIELD_TYPES or not spec.get("name"):
            raise ValueError(f"Invalid CIL field specification: {spec}")
        count = int(spec.get("count", 1))
        if count < 1:
            raise ValueError(f"Invalid count for CIL field {spec['name']}")
        fmt += f"{count}{FIELD_TYPES[field_type]}"
        fields.append((spec, count))
    return fmt, fields


def _lookup(values: dict[str, Any], path: str) -> Any:
    current: Any = values
    for part in path.split("."):
        if not isinstance(current, dict) or part not in current:
            raise ValueError(f"Missing response field: {path}")
        current = current[part]
    return current
