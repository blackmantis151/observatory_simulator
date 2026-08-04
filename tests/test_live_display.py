from pathlib import Path
import sys
import threading
import unittest
from unittest.mock import patch

import matplotlib
matplotlib.use("Agg")
import numpy as np


SIM_DIR = Path(__file__).resolve().parents[1] / "tcs_sim"
sys.path.insert(0, str(SIM_DIR))

import run_all  # noqa: E402


class FakeAxis:
    def __init__(self, actual, target):
        self.actual = actual
        self.target_pos_deg = target
        self.report_calls = 0

    def reported_pos_deg(self):
        self.report_calls += 1
        return self.actual


class FakeAGS:
    def __init__(self):
        self.window = 10
        self.frame_lock = threading.Lock()
        self.latest_frame = np.arange(100, dtype=float).reshape(10, 10)
        self.latest = {
            "window_tlx": 100,
            "window_tly": 200,
            "centroid_x": 104.5,
            "centroid_y": 205.5,
            "fwhm": 2.1,
            "guide_status": "0",
        }
        self.mode = "GUIDING"
        self.focus_mm = 12.5
        self.cfg = {}


class LiveDisplayTests(unittest.TestCase):
    def test_polar_camera_and_unwrapped_3d_history_share_one_sample(self):
        alt = FakeAxis(actual=45.25, target=46.0)
        az = FakeAxis(actual=401.5, target=405.0)
        ags = FakeAGS()

        with (
            patch.object(run_all.plt, "show"),
            patch.object(run_all.plt, "close"),
            patch.object(run_all.time, "sleep", side_effect=KeyboardInterrupt),
        ):
            run_all.live_sky_plot(alt, az, ags)

        figure = run_all.plt.gcf()
        polar, camera, history = figure.axes[:3]
        self.assertEqual((alt.report_calls, az.report_calls), (1, 1))
        self.assertEqual(camera.images[0].get_array().shape, (10, 10))
        waiting = next(text for text in camera.texts
                       if text.get_text() == "Waiting for guide acquisition")
        self.assertFalse(waiting.get_visible())

        demanded = history.lines[0].get_data_3d()
        achieved = history.lines[1].get_data_3d()
        self.assertEqual(list(demanded[1]), [46.0])
        self.assertEqual(list(demanded[2]), [405.0])
        self.assertEqual(list(achieved[1]), [45.25])
        self.assertEqual(list(achieved[2]), [401.5])

        polar_theta, polar_radius = polar.lines[1].get_data()
        self.assertAlmostEqual(float(polar_theta[0]), np.radians(401.5))
        self.assertAlmostEqual(float(polar_radius[0]), 90.0 - 45.25)


if __name__ == "__main__":
    unittest.main()
