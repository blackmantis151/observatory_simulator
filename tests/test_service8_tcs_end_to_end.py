import ctypes
import importlib.util
from pathlib import Path
import tempfile
import threading
import time
from types import SimpleNamespace
import unittest


SIM_ROOT = Path(__file__).resolve().parents[1]
TCS_ROOT = Path("/home/harshit/TCS_Girawali")

import sys
sys.path.insert(0, str(SIM_ROOT / "tcs_sim"))

from alt_sim import ALTSubsystem
from az_sim import AZSubsystem
from cas_sim import CASSubsystem
from comms.zmq_comms import ZmqComms
from tracking_intake import get_tracking_intake


class TrackingDiagnostics(ctypes.Structure):
    _fields_ = [
        ("accepted", ctypes.c_uint64),
        ("mismatched", ctypes.c_uint64),
        ("ignored", ctypes.c_uint64),
        ("last_generation", ctypes.c_uint64),
        ("last_demand_set_sequence", ctypes.c_uint64),
        ("last_axis", ctypes.c_char * 8),
    ]


class Service8TcsSimulatorEndToEnd(unittest.TestCase):
    def test_production_tracker_to_existing_axis_physics(self):
        spec = importlib.util.spec_from_file_location(
            "goto_verifier", TCS_ROOT / "scripts/test_goto_tracking.py")
        verifier = importlib.util.module_from_spec(spec)
        sys.modules[spec.name] = verifier
        spec.loader.exec_module(verifier)

        with tempfile.TemporaryDirectory(prefix="service8-e2e-") as directory:
            temp = Path(directory)
            ports = verifier.free_ports(5)
            endpoints = [f"tcp://127.0.0.1:{port}" for port in ports]
            build_log = (temp / "build.log").open("w")
            library = verifier.build_library(temp, ports[:4], build_log)
            build_log.close()
            tcs, telescope, source = verifier.write_config(temp)

            import os
            os.environ["TCS_STATE_JSON"] = str(tcs)
            os.environ["TELESCOPE_STATE_JSON"] = str(telescope)
            os.environ["TCS_EDIT_SOURCE_JSON"] = str(source)

            bus = SimpleNamespace(
                cmd_endpoint=endpoints[0], result_endpoint=endpoints[1],
                status_endpoint=endpoints[2], scc_endpoint=endpoints[3],
                heartbeat_endpoint=endpoints[4],
            )
            get_tracking_intake().reset()
            backends = {}
            axes = {}
            threads = []
            lib = verifier.bind_library(library)
            try:
                for name, cls in (("ALT", ALTSubsystem), ("AZ", AZSubsystem),
                                  ("CAS", CASSubsystem)):
                    backend = ZmqComms(bus, owner=name, role="subsystem")
                    backend.start()
                    axis = cls(backend, log_dir=temp / "axis-state")
                    thread = threading.Thread(target=axis.run, daemon=True)
                    thread.start()
                    backends[name] = backend
                    axes[name] = axis
                    threads.append(thread)

                self.assertTrue(lib.comms_init())
                time.sleep(0.5)
                initial = {name: axis.current_pos_deg for name, axis in axes.items()}
                target = ["Moving", "14", "34", "16.81183", "-12", "31",
                          "10.3965", "J2000", "2000", "-0.02363", "0.59535",
                          "0.16499", "8.7"]
                response = verifier.command(lib, "cmd_goto_engine", target)
                self.assertTrue(response.startswith("GOTO accepted"), response)

                deadline = time.monotonic() + 4.0
                while time.monotonic() < deadline:
                    if (all(axis.tracking_active for axis in axes.values()) and
                            all(axis.traj_positions for axis in axes.values()) and
                            any(abs(axis.current_pos_deg - initial[name]) > 1e-5
                                for name, axis in axes.items())):
                        break
                    time.sleep(0.05)
                self.assertTrue(all(axis.tracking_active for axis in axes.values()))
                self.assertTrue(all(axis.traj_positions for axis in axes.values()))
                self.assertTrue(any(abs(axis.current_pos_deg - initial[name]) > 1e-5
                                    for name, axis in axes.items()))

                lib.comms_get_tracking_response_diagnostics.argtypes = [
                    ctypes.POINTER(TrackingDiagnostics)]
                lib.comms_get_tracking_response_diagnostics.restype = ctypes.c_bool
                diagnostics = TrackingDiagnostics()
                deadline = time.monotonic() + 2.0
                while time.monotonic() < deadline:
                    self.assertTrue(lib.comms_get_tracking_response_diagnostics(
                        ctypes.byref(diagnostics)))
                    if diagnostics.accepted >= 3:
                        break
                    time.sleep(0.05)
                self.assertGreaterEqual(diagnostics.accepted, 3)
                self.assertEqual(diagnostics.mismatched, 0)

                lib.tracking_engine_cancel.argtypes = [ctypes.c_char_p]
                lib.tracking_engine_cancel(b"isolated test cancellation")
                deadline = time.monotonic() + 2.0
                while time.monotonic() < deadline and any(
                        axis.tracking_active for axis in axes.values()):
                    time.sleep(0.05)
                self.assertFalse(any(axis.tracking_active for axis in axes.values()))
            finally:
                try:
                    lib.comms_shutdown()
                except Exception:
                    pass
                for axis in axes.values():
                    axis.running = False
                    axis._motion_stop = True
                for thread in threads:
                    thread.join(2.0)
                for backend in backends.values():
                    backend.stop()
                get_tracking_intake().reset()


if __name__ == "__main__":
    unittest.main()
