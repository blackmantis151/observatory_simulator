import json
from pathlib import Path
import sys
import tempfile
import threading
import time
from types import SimpleNamespace
import unittest
import uuid

import zmq

SIM_DIR = Path(__file__).resolve().parents[1] / "tcs_sim"
sys.path.insert(0, str(SIM_DIR))

from alt_sim import ALTSubsystem
from amn_sim import AMNSubsystem
from comms.message import SimMessage
from comms.zmq_comms import ZmqComms


class FakeComms:
    def __init__(self):
        self.results = []
        self.statuses = []

    def send_result(self, message: SimMessage):
        self.results.append(message)

    def send_status(self, message: SimMessage):
        self.statuses.append(message)

    def send_heartbeat(self, message: SimMessage):
        pass


def request(command, first, value, cmd_id=1):
    return {
        "msg_type": "CMD",
        "cmd_id": cmd_id,
        "command": command,
        "parameters": [first, 0x10000, 4, 0, value, 0, 0, 0],
    }


class WordCommandTests(unittest.TestCase):
    def test_amn_commands_validate_and_publish_common_results(self):
        with tempfile.TemporaryDirectory() as directory:
            comms = FakeComms()
            amn = AMNSubsystem(comms, log_dir=directory)
            amn.sim_speed = 1e9
            vectors = [
                ("FOCUS", 0x00350007, 25000, "focus_pos_mm", 25.0),
                ("MIRROR_COVER", 0x27, 1, "mirror_cover", "OPEN"),
                ("MOVE_FOLD", 0x00340007, 4, "fold_position", "4"),
                ("DARKSLIDE", 0x00310011, 2, "darkslide", "OUT"),
                ("AGFILTER", 0x0031000E, 2, "agfilter", "IN"),
                ("AGFOCUS", 0x00320007, 1235, "agfocus_mm", 1.235),
                ("AGRADIAL", 0x00310007, 2500, "agradial_mm", 2.5),
            ]
            for command, first, value, attr, expected in vectors:
                comms.results.clear()
                words = request(command, first, value)["parameters"]
                amn.handle_command(request(command, first, value))
                self.assertEqual(
                    [(item.topic, item.body["class"], item.body["service"])
                     for item in comms.results],
                    [(f"result.{command}", 3, 251),
                     (f"result.{command}", 2, 251)],
                )
                self.assertEqual(comms.results[-1].body["payload_words"], words)
                self.assertEqual(getattr(amn, attr), expected)

    def test_bad_reserved_word_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            comms = FakeComms()
            amn = AMNSubsystem(comms, log_dir=directory)
            message = request("FOCUS", 0x00350007, 25000)
            message["parameters"][5] = 1
            amn.handle_command(message)
            self.assertEqual(comms.results[-1].topic, "result.FOCUS")
            self.assertEqual(comms.results[-1].body["class"], 6)

    def test_altitude_words_convert_mas_to_degrees(self):
        with tempfile.TemporaryDirectory() as directory:
            comms = FakeComms()
            alt = ALTSubsystem(comms, log_dir=directory)
            alt.current_pos_deg = 45.5
            alt.current_pos = 45.5
            alt.handle_command(request("ALTITUDE", 0x002A0007, 163800000))
            self.assertEqual(
                [(item.topic, item.body["class"], item.body["service"])
                 for item in comms.results],
                [("result.ALTITUDE", 3, 9), ("result.ALTITUDE", 2, 9)],
            )
            self.assertEqual(alt.target_pos_deg, 45.5)
            alt.running = False

    def test_command_specific_subscriptions_preserve_legacy_topics(self):
        self.assertIn("cmd.AMN", ZmqComms.command_topics("AMN"))
        self.assertIn("cmd.FOCUS", ZmqComms.command_topics("AMN"))
        self.assertIn("cmd.ALT", ZmqComms.command_topics("ALT"))
        self.assertIn("cmd.ALTITUDE", ZmqComms.command_topics("ALT"))


class DirectZmqEndToEndTests(unittest.TestCase):
    def test_focus_request_and_terminal_reply_without_gateway(self):
        context = zmq.Context.instance()
        prefix = f"inproc://word-contract-{uuid.uuid4()}"
        bus = SimpleNamespace(
            cmd_endpoint=prefix + "-cmd",
            result_endpoint=prefix + "-result",
            status_endpoint=prefix + "-status",
            heartbeat_endpoint=prefix + "-heartbeat",
        )
        command_pub = context.socket(zmq.PUB)
        result_sub = context.socket(zmq.SUB)
        status_sub = context.socket(zmq.SUB)
        heartbeat_sub = context.socket(zmq.SUB)
        command_pub.bind(bus.cmd_endpoint)
        result_sub.bind(bus.result_endpoint)
        result_sub.setsockopt_string(zmq.SUBSCRIBE, "result.FOCUS")
        status_sub.bind(bus.status_endpoint)
        status_sub.setsockopt_string(zmq.SUBSCRIBE, "")
        heartbeat_sub.bind(bus.heartbeat_endpoint)
        heartbeat_sub.setsockopt_string(zmq.SUBSCRIBE, "")

        backend = ZmqComms(bus, owner="AMN", role="subsystem")
        backend.start()
        with tempfile.TemporaryDirectory() as directory:
            amn = AMNSubsystem(backend, log_dir=directory)
            amn.sim_speed = 1e9
            thread = threading.Thread(target=amn.run, daemon=True)
            thread.start()
            try:
                time.sleep(0.2)
                message = request("FOCUS", 0x00350007, 0, cmd_id=101)
                command_pub.send_string("cmd.FOCUS", zmq.SNDMORE)
                command_pub.send_string(json.dumps(message))
                replies = []
                poller = zmq.Poller()
                poller.register(result_sub, zmq.POLLIN)
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline and len(replies) < 2:
                    if result_sub in dict(poller.poll(200)):
                        self.assertEqual(result_sub.recv_string(), "result.FOCUS")
                        replies.append(json.loads(result_sub.recv_string()))
                self.assertEqual([reply["class"] for reply in replies], [3, 2])
                self.assertEqual(replies[-1]["cmd_id"], "101")
                self.assertEqual(replies[-1]["service"], 251)
                self.assertEqual(replies[-1]["payload_words"],
                                 message["parameters"])
            finally:
                amn.running = False
                thread.join(2)
                backend.stop()
                command_pub.close(0)
                result_sub.close(0)
                status_sub.close(0)
                heartbeat_sub.close(0)


if __name__ == "__main__":
    unittest.main()
