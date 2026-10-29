from __future__ import annotations

from dataclasses import dataclass
import queue
import subprocess
import threading
import time
from typing import BinaryIO

from .base import CommsBackend
from .cil_codec import (
    CIL_CMD,
    CIL_ERR,
    CIL_RSP,
    FRAME_ERROR,
    FRAME_LENGTH,
    FRAME_READY,
    FRAME_RX,
    FRAME_TX,
    MAX_FRAME_BYTES,
    CilPacket,
    decode_fields,
    encode_fields,
    load_cil_map,
    pack_frame,
    unpack_frame,
)
from .message import SimMessage


_CIL_HEARTBEATS: queue.Queue[SimMessage] = queue.Queue()
_TERMINAL_RESULTS = {"COMPLETED", "DONE", "ERROR"}


def _as_int(value) -> int:
    return int(str(value), 0)


@dataclass
class Transaction:
    packet: CilPacket
    service_spec: dict
    created: float
    request: dict


class BridgeProcess:
    def __init__(self, executable: str, map_file: str, identity: str, timeout_ms: int):
        self.process = subprocess.Popen(
            [
                executable,
                "--map",
                map_file,
                "--name",
                identity,
                "--timeout-ms",
                str(timeout_ms),
            ],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=None,
            bufsize=0,
        )
        self.identity = identity
        self.write_lock = threading.Lock()

    def read(self) -> tuple[int, CilPacket]:
        if self.process.stdout is None:
            raise RuntimeError("CIL bridge stdout is unavailable")
        size_data = _read_exact(self.process.stdout, FRAME_LENGTH.size)
        size = FRAME_LENGTH.unpack(size_data)[0]
        if size < 40 or size > MAX_FRAME_BYTES:
            raise RuntimeError(f"Invalid frame size from CIL bridge: {size}")
        return unpack_frame(_read_exact(self.process.stdout, size))

    def send(self, packet: CilPacket) -> None:
        if self.process.stdin is None or self.process.poll() is not None:
            raise RuntimeError(f"CIL bridge {self.identity} is not running")
        frame = pack_frame(FRAME_TX, packet)
        with self.write_lock:
            self.process.stdin.write(frame)
            self.process.stdin.flush()

    def stop(self) -> None:
        if self.process.stdin is not None:
            try:
                self.process.stdin.close()
            except BrokenPipeError:
                pass
        try:
            self.process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            self.process.terminate()
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        if self.process.stdout is not None:
            self.process.stdout.close()


class CilComms(CommsBackend):
    """Strict CIL backend: packets are dispatched, never echoed."""

    def __init__(self, bus_config, owner: str, role: str):
        super().__init__(bus_config, owner, role)
        self.cfg = bus_config.cil
        self.owner_cfg = self.cfg["endpoints"][self.owner]
        self.timeout_s = int(self.cfg.get("transaction_timeout_ms", 30000)) / 1000.0
        self.command_queue: queue.Queue[SimMessage] = queue.Queue()
        self.bridges: list[BridgeProcess] = []
        self.bridge_specs: dict[str, dict] = {}
        self.transactions: dict[str, Transaction] = {}
        self.latest_heartbeat: dict = {"state": "STARTING"}
        self.lock = threading.Lock()
        self.running = False
        self.failure: BaseException | None = None
        self.map_ids = load_cil_map(self.cfg["map_file"])

    def start(self) -> None:
        identities = self.owner_cfg.get("identities", [])
        if not identities:
            raise RuntimeError(f"No CIL identities configured for {self.owner}")
        if self.role == "subsystem" and not any(item.get("services") for item in identities):
            raise RuntimeError(
                f"No official CIL service profile configured for {self.owner}; "
                "refusing to start an echo/incomplete backend"
            )
        if self.role == "subsystem" and not any(
            service.get("kind") == "heartbeat"
            for identity in identities
            for service in identity.get("services", {}).values()
        ):
            raise RuntimeError(f"No official CIL heartbeat profile configured for {self.owner}")
        if self.role == "subsystem" and "status" not in self.owner_cfg.get("outputs", {}):
            raise RuntimeError(f"No official CIL status profile configured for {self.owner}")
        if self.role == "scc" and "summary" not in self.owner_cfg.get("outputs", {}):
            raise RuntimeError("No official CIL summary profile configured for SCC")
        self._validate_profiles(identities)
        self.running = True
        try:
            for spec in identities:
                name = spec["name"].upper()
                if name not in self.map_ids:
                    raise RuntimeError(f"CIL identity {name} is absent from the map")
                bridge = BridgeProcess(
                    self.cfg["bridge_executable"],
                    self.cfg["map_file"],
                    name,
                    int(self.cfg.get("receive_timeout_ms", 20)),
                )
                kind, ready = bridge.read()
                if kind != FRAME_READY or ready.source != self.map_ids[name]:
                    bridge.stop()
                    raise RuntimeError(f"CIL bridge {name} did not become ready")
                self.bridges.append(bridge)
                self.bridge_specs[name] = spec
                threading.Thread(
                    target=self._reader_loop, args=(bridge, spec), daemon=True
                ).start()
        except BaseException:
            self.stop()
            raise

    def stop(self) -> None:
        self.running = False
        for bridge in self.bridges:
            bridge.stop()
        self.bridges.clear()

    def send_result(self, message: SimMessage) -> None:
        result_type = str(message.body.get("msg_type", "")).upper()
        command_id = str(message.body.get("cmd_id", ""))
        with self.lock:
            transaction = self.transactions.get(command_id)
        if transaction is None:
            raise RuntimeError(f"No pending CIL transaction for result {command_id}")
        response = transaction.service_spec.get("responses", {}).get(result_type)
        if response is None:
            raise RuntimeError(
                f"No official {result_type} response profile for service "
                f"0x{transaction.packet.service:08x}"
            )
        values = dict(message.body)
        values["request"] = transaction.request
        payload = encode_fields(response.get("fields", []), values)
        self._send_transaction(transaction, _as_int(response["class"]), payload)
        if result_type in _TERMINAL_RESULTS:
            with self.lock:
                self.transactions.pop(command_id, None)

    def send_status(self, message: SimMessage) -> None:
        self._send_output("status", message.body)

    def send_heartbeat(self, message: SimMessage) -> None:
        self.latest_heartbeat = dict(message.body)
        _CIL_HEARTBEATS.put(message)

    def send_summary(self, message: SimMessage) -> None:
        self._send_output("summary", message.body)

    def receive_command(self, timeout_ms: int | None = None) -> SimMessage | None:
        self._raise_if_failed()
        self._expire_transactions()
        try:
            timeout = None if timeout_ms is None else timeout_ms / 1000.0
            return self.command_queue.get(timeout=timeout)
        except queue.Empty:
            self._raise_if_failed()
            return None

    def receive_heartbeat(self, timeout_ms: int | None = None) -> SimMessage | None:
        self._raise_if_failed()
        try:
            timeout = None if timeout_ms is None else timeout_ms / 1000.0
            return _CIL_HEARTBEATS.get(timeout=timeout)
        except queue.Empty:
            return None

    def _reader_loop(self, bridge: BridgeProcess, identity_spec: dict) -> None:
        try:
            while self.running:
                kind, packet = bridge.read()
                if kind == FRAME_ERROR:
                    raise RuntimeError(packet.payload.decode("utf-8", errors="replace"))
                if kind != FRAME_RX:
                    raise RuntimeError(f"Unexpected CIL bridge frame kind: {kind}")
                self._handle_packet(bridge, identity_spec, packet)
        except BaseException as exc:
            if self.running:
                self.failure = exc

    def _validate_profiles(self, identities: list[dict]) -> None:
        local_names = {identity["name"].upper() for identity in identities}
        for identity in identities:
            for raw_service, service in identity.get("services", {}).items():
                _as_int(raw_service)
                if (
                    service.get("kind") != "heartbeat"
                    and not service.get("command")
                    and not service.get("commands")
                ):
                    raise RuntimeError(
                        f"CIL service {raw_service} for {self.owner} has no command"
                    )
                response_sets = [service.get("responses", {})]
                response_sets.extend(
                    command.get("responses", {})
                    for command in service.get("commands", {}).values()
                )
                for responses in response_sets:
                    for response in responses.values():
                        message_class = _as_int(response["class"])
                        if (
                            message_class == CIL_CMD
                            or message_class < 2
                            or message_class > 6
                        ):
                            raise RuntimeError(
                                f"CIL service {raw_service} has invalid response class "
                                f"{message_class}"
                            )
        for output_name, output in self.owner_cfg.get("outputs", {}).items():
            identity = output.get("identity", "").upper()
            destination = output.get("destination", "").upper()
            send_to = output.get("send_to", destination).upper()
            if identity not in local_names:
                raise RuntimeError(
                    f"CIL {output_name} output uses inactive identity {identity}"
                )
            for name in (destination, send_to):
                if name not in self.map_ids:
                    raise RuntimeError(
                        f"CIL {output_name} destination {name} is absent from the map"
                    )
            _as_int(output["class"])
            _as_int(output["service"])

    def _handle_packet(self, bridge: BridgeProcess, identity_spec: dict, packet: CilPacket) -> None:
        local_id = self.map_ids[identity_spec["name"].upper()]
        if packet.destination != local_id or packet.message_class != CIL_CMD:
            self._send_error(bridge, packet)
            return
        services = identity_spec.get("services", {})
        service_spec = services.get(f"0x{packet.service:08x}", services.get(packet.service))
        if service_spec is None:
            self._send_error(bridge, packet)
            return
        if service_spec.get("kind") == "heartbeat":
            payload = encode_fields(service_spec.get("response_fields", []), self.latest_heartbeat)
            self._reply(bridge, packet, CIL_RSP, payload)
            return
        try:
            params = decode_fields(service_spec.get("request_fields", []), packet.payload)
        except ValueError:
            self._send_error(bridge, packet)
            return
        commands = service_spec.get("commands")
        if commands:
            discriminator = str(
                params.get(service_spec.get("command_field", "command"))
            )
            command_spec = commands.get(discriminator)
            if command_spec is None:
                self._send_error(bridge, packet)
                return
            service_spec = command_spec
        command_id = f"cil:{packet.source}:{packet.sequence}"
        transaction = Transaction(packet, service_spec, time.monotonic(), params)
        with self.lock:
            if command_id in self.transactions:
                self._send_error(bridge, packet)
                return
            self.transactions[command_id] = transaction
        self.command_queue.put(
            SimMessage(
                topic=f"cmd.{self.owner}",
                body={
                    "msg_type": "CMD",
                    "command": service_spec["command"],
                    "cmd_id": command_id,
                    "params": params,
                },
            )
        )

    def _send_transaction(self, transaction: Transaction, message_class: int, payload: bytes) -> None:
        bridge = self._bridge_for_local_id(transaction.packet.destination)
        self._reply(bridge, transaction.packet, message_class, payload)

    def _reply(self, bridge: BridgeProcess, request: CilPacket, message_class: int, payload: bytes) -> None:
        bridge.send(
            CilPacket(
                route_id=request.route_id,
                source=request.destination,
                destination=request.source,
                message_class=message_class,
                service=request.service,
                sequence=request.sequence,
                payload=payload,
            )
        )

    def _send_error(self, bridge: BridgeProcess, request: CilPacket) -> None:
        # Empty CIL-level error is deliberate: unknown payloads are never mirrored.
        self._reply(bridge, request, CIL_ERR, b"")

    def _send_output(self, output_name: str, values: dict) -> None:
        output = self.owner_cfg.get("outputs", {}).get(output_name)
        if output is None:
            return
        bridge = self._bridge_for_name(output["identity"])
        destination = self.map_ids[output["destination"].upper()]
        send_to = self.map_ids[output.get("send_to", output["destination"]).upper()]
        payload = encode_fields(output.get("fields", []), values)
        sequence = int(time.time_ns() & 0xFFFFFFFF) or 1
        bridge.send(
            CilPacket(
                route_id=send_to,
                source=self.map_ids[output["identity"].upper()],
                destination=destination,
                message_class=_as_int(output["class"]),
                service=_as_int(output["service"]),
                sequence=sequence,
                payload=payload,
            )
        )

    def _bridge_for_name(self, name: str) -> BridgeProcess:
        wanted = name.upper()
        for bridge in self.bridges:
            if bridge.identity == wanted:
                return bridge
        raise RuntimeError(f"No running CIL bridge for identity {wanted}")

    def _bridge_for_local_id(self, local_id: int) -> BridgeProcess:
        for bridge in self.bridges:
            if self.map_ids[bridge.identity] == local_id:
                return bridge
        raise RuntimeError(f"No running CIL bridge for local ID {local_id}")

    def _expire_transactions(self) -> None:
        cutoff = time.monotonic() - self.timeout_s
        with self.lock:
            expired = [
                (key, value)
                for key, value in self.transactions.items()
                if value.created < cutoff
            ]
            for key, _ in expired:
                self.transactions.pop(key, None)
        for _, transaction in expired:
            response = transaction.service_spec.get("responses", {}).get("TIMEOUT")
            if response is None:
                self._send_transaction(transaction, CIL_ERR, b"")
                continue
            values = {"error_code": "TIMEDOUT", "request": transaction.request}
            payload = encode_fields(response.get("fields", []), values)
            self._send_transaction(transaction, _as_int(response["class"]), payload)

    def _raise_if_failed(self) -> None:
        if self.failure is not None:
            raise RuntimeError(f"CIL bridge failed for {self.owner}") from self.failure


def _read_exact(stream: BinaryIO, size: int) -> bytes:
    chunks = bytearray()
    while len(chunks) < size:
        chunk = stream.read(size - len(chunks))
        if not chunk:
            raise EOFError("CIL bridge closed its output")
        chunks.extend(chunk)
    return bytes(chunks)
