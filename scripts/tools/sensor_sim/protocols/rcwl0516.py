"""
RCWL-0516 Triangulation Array Protocol Encoder.
Simulates a multi-sensor array of Doppler microwave sensors transmitting
triangulated contact events over serial.
"""

import struct
from typing import List
from ..interfaces import ISensorProtocol, SimulatedTarget


class RCWL0516ArrayEncoder(ISensorProtocol):
    """
    Simulates a synthetic multi-channel RCWL-0516 triangulation node.
    Outputs binary packets containing calculated 2D coordinates derived from
    multiple sensors.
    """

    HEADER = bytes([0x24, 0x52, 0x43])  # "$RC"
    TAIL = bytes([0x0D, 0x0A])           # "\r\n"

    @property
    def name(self) -> str:
        return "RCWL-0516-Triangulation"

    @property
    def default_baud_rate(self) -> int:
        return 115200

    @property
    def max_targets(self) -> int:
        return 2  # Simple Doppler arrays typically track 1-2 major reflections

    def encode_frame(self, targets: List[SimulatedTarget]) -> bytes:
        payload = bytearray(self.HEADER)
        valid = [t for t in targets if t.is_valid][:self.max_targets]
        payload.append(len(valid))

        for target in valid:
            x_cm = int(round(target.x * 100.0))
            y_cm = int(round(target.y * 100.0))
            # Signal magnitude proxy for Doppler velocity
            mag = int(min(255, max(0, abs(target.speed) * 100.0)))
            payload.extend(struct.pack("<hhB", x_cm, y_cm, mag))

        payload.extend(self.TAIL)
        return bytes(payload)
