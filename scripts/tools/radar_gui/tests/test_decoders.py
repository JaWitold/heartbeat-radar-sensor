"""
Unit tests for protocol_decoders in radar_gui.
"""

import struct
import unittest
from radar_gui.protocol_decoders import FrameDecoder, RadarContact


class TestFrameDecoder(unittest.TestCase):

    def test_decode_ld2450_valid_frame(self):
        # Build 30-byte frame: header + T1 + T2 + T3 + tail
        header = bytes([0xAA, 0xFF, 0x03, 0x00])
        tail = bytes([0x55, 0xCC])

        # Target 1: x=1000mm (1.0m), y=2000mm (2.0m), speed=50cm/s (0.5m/s), res=200mm
        t1 = struct.pack("<hhhH", 1000, 2000, 50, 200)
        # Target 2 & 3: empty
        t2 = b"\x00" * 8
        t3 = b"\x00" * 8

        payload = bytearray(header + t1 + t2 + t3 + tail)
        contacts, residual = FrameDecoder.parse_stream(payload)

        self.assertEqual(len(residual), 0)
        self.assertEqual(len(contacts), 1)
        c = contacts[0]
        self.assertEqual(c.id, 1)
        self.assertAlmostEqual(c.x, 1.0, places=2)
        self.assertAlmostEqual(c.y, 2.0, places=2)
        self.assertAlmostEqual(c.speed, 0.5, places=2)

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
