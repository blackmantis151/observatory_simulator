import math
from pathlib import Path
import sys
import tempfile
import time
import unittest


SIM_DIR = Path(__file__).resolve().parents[1] / "tcs_sim"
sys.path.insert(0, str(SIM_DIR))

from alt_sim import ALTSubsystem
from az_sim import AZSubsystem
from cas_sim import CASSubsystem
from tracking_intake import get_tracking_intake, signed_mas_degrees


class FakeComms:
    def __init__(self):
        self.results = []
        self.statuses = []

    def send_result(self, message):
        self.results.append(message)

    def send_status(self, message):
        self.statuses.append(message)

    def send_heartbeat(self, message):
        pass


DEST = {"ALT": 40, "AZ": 34, "CAS": 46}


def angle_word(degrees):
    return int(round(degrees * 3_600_000.0)) & 0xFFFFFFFF


def service8(axis, degrees, sequence=1, generation=1, session="test-session",
             seconds=None, nanoseconds=123456789):
    if seconds is None:
        seconds = int(time.time()) + 1
    return {
        "schema": "tcs.tracking.service8.v1",
        "session": session,
        "process_id": 1234,
        "generation": generation,
        "demand_set_sequence": sequence,
        "axis": axis,
        "DestId": DEST[axis],
        "Class": 1,
        "Service": 8,
        "param0": 0,
        "param1": 0,
        "param2": 4,
        "param3": 0,
        "param4": angle_word(degrees),
        "param5": seconds,
        "param6": nanoseconds,
        "param7": 0,
    }


def service9(destination, degrees):
    return {
        "DestId": destination,
        "Class": 1,
        "Service": 9,
        "param0": 0x002A0007,
        "param1": 0x00010000,
        "param2": 4,
        "param3": 0,
        "param4": angle_word(degrees),
        "param5": 0,
        "param6": 0,
        "param7": 0,
    }


class Service8TrackingTests(unittest.TestCase):
    def setUp(self):
        get_tracking_intake().reset()
        self.temp = tempfile.TemporaryDirectory()
        self.comms = FakeComms()
        self.axes = {
            "ALT": ALTSubsystem(self.comms, log_dir=self.temp.name),
            "AZ": AZSubsystem(self.comms, log_dir=self.temp.name),
            "CAS": CASSubsystem(self.comms, log_dir=self.temp.name),
        }

    def tearDown(self):
        for axis in self.axes.values():
            axis.running = False
            axis._motion_stop = True
        for axis in self.axes.values():
            axis.physics_thread.join(1.0)
        self.temp.cleanup()
        get_tracking_intake().reset()

    def send_set(self, alt=45.0, az=-10.0, cas=-10.0, **metadata):
        intake = get_tracking_intake()
        packets = {
            "ALT": service8("ALT", alt, **metadata),
            "AZ": service8("AZ", az, **metadata),
            "CAS": service8("CAS", cas, **metadata),
        }
        for name in ("ALT", "AZ", "CAS"):
            self.assertTrue(intake.ingest(
                self.axes[name], f"tracking.{name}", packets[name]))
        return packets

    def test_signed_cas_round_trip_and_complete_set_commit(self):
        self.assertEqual(angle_word(-10.0), 4_258_967_296)
        self.assertEqual(angle_word(-10.0), 0xFDDAAF00)
        self.assertEqual(signed_mas_degrees(angle_word(-10.0)), -10.0)

        timestamp = int(time.time()) + 1
        packets = self.send_set(seconds=timestamp)
        self.assertTrue(all(axis.tracking_active for axis in self.axes.values()))
        self.assertEqual(self.axes["ALT"].traj_positions, [45.0])
        self.assertEqual(self.axes["AZ"].traj_positions, [-10.0])
        self.assertEqual(self.axes["CAS"].traj_positions, [-10.0])

        for axis in self.axes.values():
            axis._drain_tracking_responses()
        self.assertEqual(len(self.comms.results), 3)
        for response in self.comms.results:
            body = response.body
            request = packets[body["axis"]]
            self.assertEqual((body["Class"], body["Service"]), (2, 8))
            self.assertEqual((body["param0"], body["param1"], body["param7"]),
                             (0x002A0008, 0x00010000, 0x18))
            self.assertEqual(body["param4"], request["param4"])
            self.assertEqual(body["param5"], request["param5"])
            self.assertEqual(body["param6"], request["param6"])

    def test_incomplete_duplicate_stale_and_cancellation(self):
        intake = get_tracking_intake()
        packet = service8("ALT", 45.0)
        self.assertTrue(intake.ingest(self.axes["ALT"], "tracking.ALT", packet))
        self.assertTrue(intake.ingest(self.axes["ALT"], "tracking.ALT", packet))
        self.assertFalse(any(axis.tracking_active for axis in self.axes.values()))

        self.send_set(sequence=1, generation=2)
        before = list(self.axes["ALT"].traj_positions)
        stale = service8("ALT", 46.0, sequence=0, generation=2)
        self.assertFalse(intake.ingest(self.axes["ALT"], "tracking.ALT", stale))
        self.assertEqual(self.axes["ALT"].traj_positions, before)

        self.assertTrue(intake.cancel({
            "schema": "tcs.tracking.cancel.v1",
            "session": "test-session",
            "retire_before_generation": 3,
        }))
        self.assertFalse(any(axis.tracking_active for axis in self.axes.values()))
        delayed = service8("ALT", 46.0, generation=2, sequence=2)
        self.assertFalse(intake.ingest(self.axes["ALT"], "tracking.ALT", delayed))

    def test_update_does_not_reset_pid_or_start_threads(self):
        self.send_set(sequence=1)
        for axis in self.axes.values():
            axis.integral_error = 1.25
            axis.prev_error = -0.5
            axis.current_vel = 0.2
        thread_ids = {name: id(axis.physics_thread) for name, axis in self.axes.items()}
        self.send_set(alt=45.1, az=-9.9, cas=-9.8, sequence=2)
        for name, axis in self.axes.items():
            self.assertEqual(id(axis.physics_thread), thread_ids[name])
            self.assertTrue(math.isclose(axis.integral_error, 1.25))
            self.assertTrue(math.isclose(axis.prev_error, -0.5))
            self.assertTrue(math.isclose(axis.current_vel, 0.2))

    def test_flat_service9_manual_packet_preserves_signed_word(self):
        self.send_set(sequence=1)
        self.axes["CAS"].sim_speed = 1e9
        request = service9(46, -10.0)
        self.axes["CAS"].handle_command(request)
        responses = [item for item in self.comms.results
                     if item.topic == "result.CAS" and item.body.get("Service") == 9]
        self.assertEqual(len(responses), 1)
        self.assertEqual(responses[0].body["param4"], 0xFDDAAF00)
        self.assertEqual(self.axes["CAS"].target_pos_deg, -10.0)
        self.assertFalse(any(axis.tracking_active for axis in self.axes.values()))

    def test_rejects_bad_timestamp_and_fractional_word(self):
        intake = get_tracking_intake()
        packet = service8("ALT", 45.0)
        packet["param6"] = 1_000_000_000
        self.assertFalse(intake.ingest(self.axes["ALT"], "tracking.ALT", packet))
        packet = service8("ALT", 45.0)
        packet["param4"] = 1.5
        self.assertFalse(intake.ingest(self.axes["ALT"], "tracking.ALT", packet))


if __name__ == "__main__":
    unittest.main()
