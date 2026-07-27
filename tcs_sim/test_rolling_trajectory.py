import os
import tempfile
import time
import unittest

from subsystem_base import SubsystemBase


class RollingTrajectoryTest(unittest.TestCase):
    def test_future_replacement_and_stale_revision(self):
        previous = os.getcwd()
        with tempfile.TemporaryDirectory() as directory:
            os.chdir(directory)
            try:
                axis = SubsystemBase("ALT", object())
                start = time.time() + 1
                first = {
                    "trajectory_id": 7,
                    "revision": 1,
                    "mode": "START",
                    "effective_from": start,
                    "timestamp": [start, start + 1, start + 2],
                    "position": [10, 11, 12],
                }
                self.assertTrue(axis.update_trajectory_data(first))
                replacement = {
                    "trajectory_id": 7,
                    "revision": 2,
                    "mode": "REPLACE_FUTURE",
                    "effective_from": start + 1,
                    "timestamp": [start + 1, start + 2, start + 3],
                    "position": [21, 22, 23],
                }
                self.assertTrue(axis.update_trajectory_data(replacement))
                self.assertEqual(axis.traj_positions, [10.0, 21.0, 22.0, 23.0])
                self.assertTrue(axis.update_trajectory_data(first))
                self.assertEqual(axis.traj_positions, [10.0, 21.0, 22.0, 23.0])
                self.assertAlmostEqual(axis.get_rolling_target(start + 1.5), 21.5)
                azimuth = SubsystemBase("AZ", object())
                wrapped = dict(first, trajectory_id=8, position=[359, 0, 1])
                self.assertTrue(azimuth.update_trajectory_data(wrapped))
                self.assertEqual(azimuth.traj_positions, [359.0, 360.0, 361.0])
            finally:
                os.chdir(previous)


if __name__ == "__main__":
    unittest.main()
