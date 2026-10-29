from pathlib import Path
import queue
import struct
import sys
import threading
import unittest

import numpy as np


SIM_DIR = Path(__file__).resolve().parents[1] / "tcs_sim"
sys.path.insert(0, str(SIM_DIR))

from ags_sim import (  # noqa: E402
    AGS_BAD_CMD,
    AGSSubsystem,
    SYS_NOMINAL,
    format_guide_packet,
)
from comms.cil_codec import CIL_CMD, CilPacket  # noqa: E402
from comms.cil_comms import CilComms  # noqa: E402
from disturbance import TrackingDisturbance  # noqa: E402


class FakeComms:
    def __init__(self):
        self.results = []
        self.status = []
        self.heartbeats = []

    def send_result(self, message):
        self.results.append(message.body)

    def send_status(self, message):
        self.status.append(message.body)

    def send_heartbeat(self, message):
        self.heartbeats.append(message.body)


class Axis:
    tracking_active = True
    current_pos_deg = 45.0
    target_pos_deg = 45.0


def make_ags(**overrides):
    config = {
        "guide_udp_host": "127.0.0.1",
        "guide_udp_port": 9,
        "cadence_s": 60,
        "pixel_scale_arcsec": 1.0,
        "camera": {
            "width": 1024,
            "height": 1024,
            "guide_window": 100,
            "star_count": 3,
            "magnitude_range": [12.0, 12.0],
            "magnitude_zero_point": 24.4,
            "background_adu": 100.0,
            "read_noise_adu": 1.0,
            "saturation_adu": 10000000.0,
        },
        "focus": {
            "minimum_mm": 0.0,
            "maximum_mm": 25.0,
            "default_mm": 12.5,
            "optimum_mm": 12.5,
            "speed_mm_s": 1000.0,
            "in_focus_fwhm_px": 2.0,
            "defocus_px_per_mm": 0.8,
        },
        "disturbance": {"seed": 7},
    }
    config.update(overrides)
    disturbance = TrackingDisturbance({
        "seed": 7,
        "fixed_initial_offset_arcsec": [1.0, -0.5],
        "drift_rms_arcsec": 0.0,
        "measurement_noise_arcsec": 0.0,
        "vibrations": [],
    })
    return AGSSubsystem(FakeComms(), config, Axis(), Axis(), disturbance)


class GuidePacketTests(unittest.TestCase):
    def test_exact_legacy_packet(self):
        packet = format_guide_packet(512.34, 510.20, 2.0, "0")
        self.assertEqual(len(packet), 34)
        self.assertEqual(packet[:29], b"00512.34 00510.20 00002.00 0 ")
        self.assertEqual(int(packet[29:33]), sum(packet[:29]))
        self.assertEqual(packet[-1:], b"\r")

    def test_values_that_do_not_fit_are_rejected(self):
        with self.assertRaises(ValueError):
            format_guide_packet(10000.0, 1.0, 2.0, "0")


class DisturbanceTests(unittest.TestCase):
    def test_seeded_disturbance_is_repeatable_and_bounded(self):
        config = {
            "seed": 42,
            "initial_offset_radius_arcsec": 1.5,
            "drift_rms_arcsec": 0.2,
            "measurement_noise_arcsec": 0.0,
            "vibrations": [],
        }
        first = TrackingDisturbance(config)
        second = TrackingDisturbance(config)
        self.assertLessEqual(float(np.linalg.norm(first.initial)), 1.5)
        times = [first.started + value for value in (0.1, 0.2, 0.4)]
        self.assertEqual(
            [first.sample(True, value) for value in times],
            [second.sample(True, second.started + value) for value in (0.1, 0.2, 0.4)],
        )
        self.assertEqual(first.sample(False), (0.0, 0.0))


class CameraAndCommandTests(unittest.TestCase):
    def test_no_state_machine_blocks_guiding(self):
        ags = make_ags()
        try:
            ags.handle_command({"cmd_id": "end", "command": "END_SESSION", "params": {}})
            ags.handle_command({"cmd_id": "guide", "command": "GUIDE_ON_BRIGHTEST", "params": {}})
            self.assertTrue(ags.guiding)
            self.assertEqual(ags.comms.results[-1]["status"], SYS_NOMINAL)
        finally:
            ags.stop()

    def test_mcp_commands_ack_complete_and_stop_safely(self):
        ags = make_ags()
        try:
            ags._select(ags.stars[0])
            ags.handle_command({"cmd_id": "safe", "command": "SAFESTATE",
                                "params": {}})
            self.assertFalse(ags.guiding)
            self.assertEqual(
                [item["msg_type"] for item in ags.comms.results[-2:]],
                ["ACK", "COMPLETED"],
            )
            self.assertEqual(ags.comms.results[-1]["mcp_state"], 6)
            ags.handle_command({"cmd_id": "activate", "command": "ACTIVATE",
                                "params": {}})
            self.assertEqual(ags.comms.results[-1]["mcp_state"], 1)
        finally:
            ags.stop()

    def test_focus_changes_rendered_fwhm_and_centroid_is_calculated(self):
        ags = make_ags()
        try:
            ags._select(ags.stars[0])
            ags.focus_mm = 12.5
            _, sharp = ags._render()
            ags.focus_mm = 20.0
            _, blurred = ags._render()
            self.assertGreater(blurred["fwhm"], sharp["fwhm"])
            self.assertNotEqual(sharp["centroid_x"], ags.stars[0]["x"])
            self.assertAlmostEqual(
                sharp["centroid_x"] - ags.stars[0]["x"], 1.0, delta=0.25
            )
        finally:
            ags.stop()

    def test_invalid_focus_and_range_return_errors(self):
        ags = make_ags()
        try:
            ags.handle_command({"cmd_id": "focus", "command": "SET_FOCUS",
                                "params": {"position_mm": 30}})
            ags.handle_command({"cmd_id": "range", "command": "GUIDE_ON_RANGE",
                                "params": {}})
            self.assertEqual([item["status"] for item in ags.comms.results[-2:]],
                             [AGS_BAD_CMD, AGS_BAD_CMD])
        finally:
            ags.stop()


class FakeBridge:
    identity = "AGS"
    def __init__(self):
        self.sent = []
    def send(self, packet):
        self.sent.append(packet)


class CilDispatchTests(unittest.TestCase):
    def test_ags_service_dispatches_by_payload_command(self):
        backend = object.__new__(CilComms)
        backend.owner = "AGS"
        backend.role = "subsystem"
        backend.command_queue = queue.Queue()
        backend.transactions = {}
        backend.lock = threading.Lock()
        backend.latest_heartbeat = {}
        backend.failure = None
        backend.running = True
        backend.map_ids = {"TCS": 1, "AGS": 2}
        bridge = FakeBridge()
        backend.bridges = [bridge]
        replies = {"COMPLETED": {"class": 2, "fields": []}}
        service = {
            "command_field": "command",
            "request_fields": [
                {"name": "command", "type": "i32"},
                {"name": "status", "type": "i32"},
                {"name": "param1", "type": "i32"},
                {"name": "param2", "type": "i32"},
            ],
            "commands": {"10": {"command": "START_SESSION", "responses": replies}},
        }
        packet = CilPacket(1, 1, 2, CIL_CMD, 0x004F0000, 55,
                           payload=struct.pack("!4i", 10, SYS_NOMINAL, 0, 0))
        backend._handle_packet(bridge, {"name": "AGS", "services": {0x004F0000: service}}, packet)
        command = backend.command_queue.get_nowait()
        self.assertEqual(command.body["command"], "START_SESSION")
        self.assertEqual(command.body["params"]["command"], 10)
        self.assertEqual(bridge.sent, [])


if __name__ == "__main__":
    unittest.main()
