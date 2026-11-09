"""
Unit tests for protocol_decoders in radar_gui.
"""

import unittest
from radar_gui.protocol_decoders import FrameDecoder, RadarContact, zone_of
from sensor_sim.interfaces import SimulatedTarget
from sensor_sim.protocols.ld2450 import LD2450Encoder


class TestFrameDecoder(unittest.TestCase):

    def test_decode_ld2450_valid_frame(self):
        frame = LD2450Encoder().encode_frame(
            [SimulatedTarget(id=1, x=1.0, y=2.0, speed=0.5, distance_resolution=0.2, is_valid=True)]
        )
        contacts, residual = FrameDecoder.parse_stream(bytearray(frame))

        self.assertEqual(len(residual), 0)
        self.assertEqual(len(contacts), 1)
        c = contacts[0]
        self.assertEqual(c.id, 1)
        self.assertAlmostEqual(c.x, 1.0, places=2)
        self.assertAlmostEqual(c.y, 2.0, places=2)
        self.assertAlmostEqual(c.speed, 0.5, places=2)

    def test_decode_ld2450_datasheet_example(self):
        contacts, _ = FrameDecoder.parse_stream(bytearray(bytes([0xAA, 0xFF, 0x03, 0x00, 0x0E, 0x03, 0xB1, 0x86, 0x10, 0x00, 0x40, 0x01] + [0x00] * 16 + [0x55, 0xCC])))
        self.assertEqual(len(contacts), 1)
        self.assertAlmostEqual(contacts[0].x, -0.782, places=3)
        self.assertAlmostEqual(contacts[0].y, 1.713, places=3)
        self.assertAlmostEqual(contacts[0].speed, -0.16, places=3)

    def test_binary_frame_containing_newline_byte_is_not_dropped(self):
        # x=10mm encodes its low byte as 0x0A, which must not be mistaken for a JSON line end
        frame = LD2450Encoder().encode_frame(
            [SimulatedTarget(id=1, x=0.010, y=2.0, speed=0.0, is_valid=True)]
        )
        self.assertIn(0x0A, frame)
        contacts, _ = FrameDecoder.parse_stream(bytearray(frame + frame))
        self.assertEqual(len(contacts), 1)
        self.assertAlmostEqual(contacts[0].x, 0.010, places=3)

    def test_firmware_threat_zone_wins_over_local_thresholds(self):
        line = b'{"targets": [{"id": 1, "x": 0.0, "y": 1.8, "speed": 0.0, "threat_zone": "Danger"}]}\n'
        contacts, _ = FrameDecoder.parse_stream(bytearray(line))
        self.assertEqual(contacts[0].threat_zone, "Danger")
        self.assertEqual(zone_of(contacts[0]), "Danger")  # local 1.5m rule alone would say Caution

    def test_decode_json_line(self):
        line = b'{"targets": [{"id": 2, "x": -1.5, "y": 3.0, "speed": -0.2}]}\n'
        buffer = bytearray(line)
        contacts, residual = FrameDecoder.parse_stream(buffer)

        self.assertEqual(len(residual), 0)
        self.assertEqual(len(contacts), 1)
        c = contacts[0]
        self.assertEqual(c.id, 2)
        self.assertAlmostEqual(c.x, -1.5, places=2)
        self.assertAlmostEqual(c.y, 3.0, places=2)
        self.assertAlmostEqual(c.speed, -0.2, places=2)


if __name__ == "__main__":
    unittest.main()
