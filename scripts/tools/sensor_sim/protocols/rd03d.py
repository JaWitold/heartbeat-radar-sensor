"""
Ai-Thinker RD-03D mmWave Radar Protocol Encoder.
Spec: 24GHz FMCW, 120 degree azimuth FoV, up to 3 targets, 256000 bps UART.
"""

import struct
from typing import List
from ..interfaces import ISensorProtocol, SimulatedTarget


class RD03DEncoder(ISensorProtocol):
    """Encodes targets into Ai-Thinker RD-03D trajectory reporting frames."""

    HEADER = bytes([0xAA, 0xA5])
    TAIL = bytes([0x55, 0x5A])

    @property
    def name(self) -> str:
        return "Ai-Thinker-RD-03D"

    @property
    def default_baud_rate(self) -> int:
        return 256000

    @property
    def max_targets(self) -> int:
        return 3

    def encode_frame(self, targets: List[SimulatedTarget]) -> bytes:
        payload = bytearray(self.HEADER)
        # Frame length placeholder (1 byte target count + 3 * 8 bytes targets)
        valid_targets = [t for t in targets if t.is_valid][:self.max_targets]
        payload.append(len(valid_targets))

        for target in valid_targets:
            # RD-03D formats: Target ID (1B), X_cm (int16), Y_cm (int16), Speed_cms (int16), Angle_deg (int8)
            x_cm = int(round(target.x * 100.0))
            y_cm = int(round(target.y * 100.0))
            speed_cms = int(round(target.speed * 100.0))
            # Calculate angle in degrees
            import math
            angle_deg = int(round(math.degrees(math.atan2(target.x, max(0.001, target.y)))))
            angle_deg = max(-90, min(90, angle_deg))

            payload.extend(struct.pack("<Bhhhb", target.id & 0xFF, x_cm, y_cm, speed_cms, angle_deg))

        # Checksum: simple XOR of all preceding payload bytes
        checksum = 0
        for b in payload:
            checksum ^= b
        payload.append(checksum)
        payload.extend(self.TAIL)

        return bytes(payload)
