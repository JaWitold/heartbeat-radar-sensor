"""
HLK-LD2450 mmWave Radar Protocol Encoder.
Spec: 24GHz FMCW, up to 3 targets, 256000 bps UART.
Packet size: 30 bytes.
"""

import struct
from typing import List
from ..interfaces import ISensorProtocol, SimulatedTarget


def _sign_magnitude(value: int) -> int:
    """LD2450 signed fields: bit 15 set = positive, clear = negative; magnitude in bits 0..14."""
    magnitude = min(abs(value), 0x7FFF)
    return magnitude | 0x8000 if value > 0 else magnitude


class LD2450Encoder(ISensorProtocol):
    """Encodes targets into 30-byte HLK-LD2450 periodic reporting frames."""

    HEADER = bytes([0xAA, 0xFF, 0x03, 0x00])
    TAIL = bytes([0x55, 0xCC])

    @property
    def name(self) -> str:
        return "HLK-LD2450"

    @property
    def default_baud_rate(self) -> int:
        return 256000

    @property
    def max_targets(self) -> int:
        return 3

    def encode_frame(self, targets: List[SimulatedTarget]) -> bytes:
        payload = bytearray(self.HEADER)

        # Encode up to 3 targets (8 bytes each = 24 bytes total)
        for i in range(self.max_targets):
            if i < len(targets) and targets[i].is_valid:
                target = targets[i]
                # Convert meters to signed millimeters (int16)
                x_mm = int(round(target.x * 1000.0))
                y_mm = int(round(target.y * 1000.0))
                # Convert m/s to signed cm/s (int16)
                speed_cms = int(round(target.speed * 100.0))
                # Distance resolution in mm (uint16)
                dist_res_mm = int(round(target.distance_resolution * 1000.0))

                dist_res_mm = max(0, min(65535, dist_res_mm))

                payload.extend(struct.pack(
                    "<HHHH", _sign_magnitude(x_mm), _sign_magnitude(y_mm), _sign_magnitude(speed_cms), dist_res_mm
                ))
            else:
                # 8 bytes of zeroes for empty target slot
                payload.extend(b"\x00" * 8)

        payload.extend(self.TAIL)
        assert len(payload) == 30, f"LD2450 frame must be exactly 30 bytes, got {len(payload)}"
        return bytes(payload)
