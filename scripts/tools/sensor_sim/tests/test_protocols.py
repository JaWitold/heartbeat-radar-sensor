"""
Unit tests for sensor protocol encoders and scenario generators.
"""

import unittest
from sensor_sim.interfaces import SimulatedTarget
from sensor_sim.protocols import get_protocol
from sensor_sim.scenarios import get_scenario


class TestSensorProtocols(unittest.TestCase):

    def test_ld2450_frame_structure(self):
        encoder = get_protocol("ld2450")
        self.assertEqual(encoder.name, "HLK-LD2450")
        self.assertEqual(encoder.default_baud_rate, 256000)

        # 1 valid target + 2 empty slots
        targets = [
            SimulatedTarget(id=1, x=1.25, y=3.50, speed=-0.45, distance_resolution=0.2, is_valid=True)
        ]
        frame = encoder.encode_frame(targets)

        # Verify exact byte length
        self.assertEqual(len(frame), 30)

        # Verify header (0xAA 0xFF 0x03 0x00)
        self.assertEqual(frame[0:4], bytes([0xAA, 0xFF, 0x03, 0x00]))

        # Verify tail (0x55 0xCC)
        self.assertEqual(frame[28:30], bytes([0x55, 0xCC]))

        # Verify empty slots (targets 2 and 3 should be 0x00)
        self.assertEqual(frame[12:20], b"\x00" * 8)
        self.assertEqual(frame[20:28], b"\x00" * 8)

    def test_ld2450_matches_datasheet_example(self):
        # Datasheet: x=-782mm, y=1713mm, speed=-16cm/s, resolution=320mm (sign-magnitude, bit 15 = positive)
        targets = [SimulatedTarget(id=1, x=-0.782, y=1.713, speed=-0.16, distance_resolution=0.32, is_valid=True)]
        frame = get_protocol("ld2450").encode_frame(targets)
        self.assertEqual(frame, bytes([0xAA, 0xFF, 0x03, 0x00, 0x0E, 0x03, 0xB1, 0x86, 0x10, 0x00, 0x40, 0x01] + [0x00] * 16 + [0x55, 0xCC]))

    def test_rd03d_frame_structure(self):
        encoder = get_protocol("rd03d")
        targets = [
            SimulatedTarget(id=1, x=-0.5, y=2.0, speed=0.1, is_valid=True)
        ]
        frame = encoder.encode_frame(targets)
        # Header 0xAA 0xA5
        self.assertEqual(frame[0:2], bytes([0xAA, 0xA5]))
        # Tail 0x55 0x5A
        self.assertEqual(frame[-2:], bytes([0x55, 0x5A]))

    def test_rcwl0516_frame_structure(self):
        encoder = get_protocol("rcwl0516")
        targets = [
            SimulatedTarget(id=1, x=0.0, y=1.5, speed=0.2, is_valid=True)
        ]
        frame = encoder.encode_frame(targets)
        self.assertTrue(frame.startswith(b"$RC"))
        self.assertTrue(frame.endswith(b"\r\n"))

    def test_telemetry_encoder_with_imu(self):
        encoder = get_protocol("telemetry")
        self.assertEqual(encoder.default_baud_rate, 921600)
        targets = [
            SimulatedTarget(id=1, x=0.5, y=1.2, speed=-0.2, is_valid=True)
        ]
        frame = encoder.encode_frame(targets)
        self.assertTrue(frame.endswith(b"\n"))
        import json
        data = json.loads(frame.decode("utf-8"))
        self.assertEqual(data["type"], "radar_frame")
        self.assertIn("targets", data)
        self.assertEqual(len(data["targets"]), 1)
        self.assertEqual(data["targets"][0]["threat_zone"], "Danger")
        self.assertIn("imu", data)
        self.assertIn("pitch", data["imu"])
        self.assertIn("roll", data["imu"])
        self.assertIn("yaw", data["imu"])
        self.assertIn("az", data["imu"])

    def test_telemetry_threat_matches_firmware_evaluator(self):
        # DistanceThreatEvaluator: nearest target sets the alert, severity = 1 - d / max_distance (6 m)
        import json
        frame = get_protocol("telemetry").encode_frame([
            SimulatedTarget(id=1, x=0.0, y=4.0, speed=0.0, is_valid=True),
            SimulatedTarget(id=2, x=0.0, y=1.0, speed=0.0, is_valid=True),
        ])
        data = json.loads(frame)
        self.assertEqual(data["alert"], "Danger")
        self.assertAlmostEqual(data["severity"], 0.83, places=2)

    def test_patrol_scenario_bounds(self):
        scenario = get_scenario("patrol")
        # Step through 10 seconds of updates
        for _ in range(100):
            targets = scenario.update(0.1)
            self.assertEqual(len(targets), 1)
            self.assertGreaterEqual(targets[0].y, 1.0)
            self.assertLessEqual(targets[0].y, 5.0)


if __name__ == "__main__":
    unittest.main()
