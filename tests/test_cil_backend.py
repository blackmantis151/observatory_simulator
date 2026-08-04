import os
from pathlib import Path
import queue
import socket
import struct
import sys
import tempfile
import threading
import time
import unittest


SIM_DIR = Path(__file__).resolve().parents[1] / "tcs_sim"
sys.path.insert(0, str(SIM_DIR))

from comms.cil_codec import (  # noqa: E402
    CIL_ACT,
    CIL_CMD,
    CIL_COM,
    CIL_ERR,
    FRAME_RX,
    CilPacket,
    decode_fields,
    encode_fields,
    load_cil_map,
    pack_frame,
    unpack_frame,
)
from comms.cil_comms import BridgeProcess, CilComms, Transaction  # noqa: E402
from comms.message import SimMessage  # noqa: E402


class FakeBridge:
    identity = "ELC"

    def __init__(self):
        self.sent = []

    def send(self, packet):
        self.sent.append(packet)


SERVICE_SPEC = {
    "command": "ALTITUDE",
    "request_fields": [{"name": "angle", "type": "f64"}],
    "responses": {
        "ACK": {"class": CIL_ACT, "fields": []},
        "COMPLETED": {
            "class": CIL_COM,
            "fields": [
                {"name": "achieved", "source": "current_pos_deg", "type": "f64"},
                {"name": "state", "type": "i32", "const": 7},
            ],
        },
        "ERROR": {"class": CIL_ERR, "fields": []},
    },
}


def make_backend():
    backend = object.__new__(CilComms)
    backend.owner = "ALT"
    backend.role = "subsystem"
    backend.timeout_s = 30
    backend.command_queue = queue.Queue()
    backend.transactions = {}
    backend.lock = __import__("threading").Lock()
    backend.latest_heartbeat = {"state": "TRACKING"}
    backend.failure = None
    backend.running = True
    backend.map_ids = {"TCS": 1, "ELC": 2, "SDB": 3}
    backend.owner_cfg = {}
    backend.cfg = {}
    bridge = FakeBridge()
    backend.bridges = [bridge]
    return backend, bridge


class CilCodecTests(unittest.TestCase):
    def test_frame_round_trip(self):
        packet = CilPacket(1, 2, 3, 4, 5, 6, 7, 8, b"payload")
        frame = pack_frame(FRAME_RX, packet)
        body_size = struct.unpack("!I", frame[:4])[0]
        kind, decoded = unpack_frame(frame[4 : 4 + body_size])
        self.assertEqual(kind, FRAME_RX)
        self.assertEqual(decoded, packet)

    def test_struct_codec_is_network_order_and_strict(self):
        fields = [
            {"name": "command", "type": "u32"},
            {"name": "position", "type": "f64"},
        ]
        payload = encode_fields(fields, {"command": 9, "position": 42.5})
        self.assertEqual(payload, struct.pack("!Id", 9, 42.5))
        self.assertEqual(decode_fields(fields, payload), {"command": 9, "position": 42.5})
        with self.assertRaises(ValueError):
            decode_fields(fields, payload[:-1])

    def test_map_ids_follow_file_order(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as stream:
            stream.write("TCS, localhost, 1001\nELC, localhost, 1002 # axis\n")
            path = stream.name
        try:
            self.assertEqual(load_cil_map(path), {"TCS": 1, "ELC": 2})
        finally:
            os.unlink(path)


class CilResultTests(unittest.TestCase):
    def test_command_runs_before_actual_result_is_encoded(self):
        backend, bridge = make_backend()
        request_payload = struct.pack("!d", 30.0)
        request = CilPacket(
            route_id=1,
            source=1,
            destination=2,
            message_class=CIL_CMD,
            service=0x1234,
            sequence=77,
            payload=request_payload,
        )
        identity = {"name": "ELC", "services": {0x1234: SERVICE_SPEC}}

        backend._handle_packet(bridge, identity, request)
        command = backend.command_queue.get_nowait()
        self.assertEqual(command.body["command"], "ALTITUDE")
        self.assertEqual(command.body["params"]["angle"], 30.0)
        self.assertEqual(bridge.sent, [])

        backend.send_result(
            SimMessage(
                "result.ALT",
                {
                    "msg_type": "ACK",
                    "cmd_id": command.body["cmd_id"],
                    "current_pos_deg": 10.0,
                },
            )
        )
        backend.send_result(
            SimMessage(
                "result.ALT",
                {
                    "msg_type": "COMPLETED",
                    "cmd_id": command.body["cmd_id"],
                    "current_pos_deg": 29.97,
                },
            )
        )

        self.assertEqual([packet.message_class for packet in bridge.sent], [CIL_ACT, CIL_COM])
        self.assertEqual(bridge.sent[0].payload, b"")
        self.assertEqual(bridge.sent[1].payload, struct.pack("!di", 29.97, 7))
        self.assertNotEqual(bridge.sent[1].payload, request_payload)
        self.assertNotIn(command.body["cmd_id"], backend.transactions)

    def test_bad_payload_is_rejected_without_dispatch_or_echo(self):
        backend, bridge = make_backend()
        request = CilPacket(1, 1, 2, CIL_CMD, 0x1234, 1, payload=b"bad")
        identity = {"name": "ELC", "services": {0x1234: SERVICE_SPEC}}
        backend._handle_packet(bridge, identity, request)
        self.assertTrue(backend.command_queue.empty())
        self.assertEqual(len(bridge.sent), 1)
        self.assertEqual(bridge.sent[0].message_class, CIL_ERR)
        self.assertEqual(bridge.sent[0].payload, b"")

    def test_expired_transaction_emits_error_not_request_payload(self):
        backend, bridge = make_backend()
        backend.timeout_s = 0
        request = CilPacket(1, 1, 2, CIL_CMD, 0x1234, 5, payload=b"request")
        backend.transactions["cil:1:5"] = Transaction(
            request, SERVICE_SPEC, time.monotonic() - 1, {"angle": 22.0}
        )
        backend._expire_transactions()
        self.assertEqual(bridge.sent[0].message_class, CIL_ERR)
        self.assertEqual(bridge.sent[0].payload, b"")


class NativeBridgeTests(unittest.TestCase):
    BRIDGE = SIM_DIR / "Cil" / "cil_bridge"
    MAP_TEMPLATE = SIM_DIR / "Cil" / "Cil.test.map"

    @unittest.skipUnless(BRIDGE.is_file(), "build tcs_sim/Cil/cil_bridge first")
    def test_two_identities_exchange_real_result_not_echo(self):
        port_a = self._free_udp_port()
        port_b = self._free_udp_port()
        while port_b == port_a:
            port_b = self._free_udp_port()

        with tempfile.TemporaryDirectory() as directory:
            map_path = Path(directory) / "Cil.map"
            lines = []
            for line in self.MAP_TEMPLATE.read_text(encoding="utf-8").splitlines():
                if line.lstrip().startswith("TU0,"):
                    line = f"TU0, localhost, {port_a}"
                elif line.lstrip().startswith("TU1,"):
                    line = f"TU1, localhost, {port_b}"
                lines.append(line)
            map_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
            ids = load_cil_map(map_path)
            sender_id = ids["TU0"]
            receiver_id = ids["TU1"]

            sender = BridgeProcess(str(self.BRIDGE), str(map_path), "TU0", 10)
            receiver = BridgeProcess(str(self.BRIDGE), str(map_path), "TU1", 10)
            try:
                self.assertEqual(sender.read()[0], 3)
                self.assertEqual(receiver.read()[0], 3)
                sender.send(CilPacket(receiver_id, sender_id, receiver_id,
                                      CIL_CMD, 0x55, 99, payload=b"request"))
                kind, request = self._read_with_timeout(receiver)
                self.assertEqual(kind, FRAME_RX)
                self.assertEqual(request.payload, b"request")

                receiver.send(CilPacket(request.route_id, receiver_id, sender_id,
                                        CIL_COM, 0x55, 99,
                                        payload=b"calculated-result"))
                kind, result = self._read_with_timeout(sender)
                self.assertEqual(kind, FRAME_RX)
                self.assertEqual(result.message_class, CIL_COM)
                self.assertEqual(result.payload, b"calculated-result")
                self.assertNotEqual(result.payload, request.payload)
            finally:
                sender.stop()
                receiver.stop()

    @staticmethod
    def _free_udp_port():
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind(("127.0.0.1", 0))
            return sock.getsockname()[1]

    @staticmethod
    def _read_with_timeout(bridge):
        result = queue.Queue()
        def read():
            try:
                result.put((True, bridge.read()))
            except BaseException as exc:
                result.put((False, exc))

        threading.Thread(target=read, daemon=True).start()
        try:
            successful, value = result.get(timeout=3)
        except queue.Empty as exc:
            raise AssertionError("Timed out waiting for native CIL bridge packet") from exc
        if not successful:
            raise value
        return value


if __name__ == "__main__":
    unittest.main()
