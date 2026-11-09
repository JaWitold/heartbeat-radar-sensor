"""
Protocol decoders for raw LD2450 binary packets and ESP32 telemetry JSON streams.
"""

import json
import math
import struct
import time
from dataclasses import dataclass
from typing import List, Optional, Tuple


@dataclass
class RadarContact:
    id: int
    x: float             # Meters (left < 0, right > 0)
    y: float             # Meters (distance forward)
    speed: float         # m/s
    distance: float      # Euclidean distance in meters
    angle_deg: float     # Azimuth angle in degrees (-90 to +90)
    timestamp: float     # Local reception timestamp
    threat_zone: Optional[str] = None  # From firmware telemetry; None for raw sensor frames


# Fallback thresholds for raw LD2450 frames (mirror LD2450Adapter::SPECS); firmware JSON carries its own zone.
DANGER_DISTANCE_M = 1.5
CAUTION_DISTANCE_M = 3.5


def zone_of(contact: RadarContact) -> str:
    """Danger / Caution / Safe, preferring the zone the firmware computed."""
    if contact.threat_zone:
        return contact.threat_zone
    if contact.distance < DANGER_DISTANCE_M:
        return "Danger"
    if contact.distance <= CAUTION_DISTANCE_M:
        return "Caution"
    return "Safe"


def _ld2450_signed(raw: int) -> int:
    """LD2450 sign-magnitude field: bit 15 set = positive, clear = negative."""
    return raw - 0x8000 if raw & 0x8000 else -raw


class FrameDecoder:
    """Auto-detects and decodes radar frames from raw byte streams."""

    LD2450_HEADER = bytes([0xAA, 0xFF, 0x03, 0x00])
    LD2450_TAIL = bytes([0x55, 0xCC])
    LD2450_FRAME_LEN = 30

    @classmethod
    def parse_stream(cls, buffer: bytearray) -> Tuple[List[RadarContact], bytearray]:
        """
        Scans buffer for complete frames (binary LD2450 or JSON newline lines).
        Returns extracted contacts from the newest valid frame and the residual buffer.
        """
        contacts: List[RadarContact] = []

        # JSON line only if a newline comes before any binary header: 0x0A is also a valid byte inside LD2450 frames
        newline_idx = buffer.find(b"\n")
        header_idx = buffer.find(cls.LD2450_HEADER)
        if newline_idx != -1 and (header_idx == -1 or newline_idx < header_idx):
            line_bytes = bytes(buffer[:newline_idx]).strip()
            del buffer[:newline_idx + 1]
            try:
                line_str = line_bytes.decode("utf-8", errors="ignore")
                if line_str.startswith("{") and line_str.endswith("}"):
                    contacts = cls._decode_json(line_str)
                    return contacts, buffer
            except Exception:
                pass

        # Try to find LD2450 binary frames
        while len(buffer) >= cls.LD2450_FRAME_LEN:
            idx = buffer.find(cls.LD2450_HEADER)
            if idx == -1:
                # If no binary header, but buffer contains readable text, don't discard
                if b"\n" in buffer or any(b in buffer for b in (b"{", b"}", b'"')):
                    break
                buffer = buffer[-3:]
                break

            if idx > 0:
                del buffer[:idx]

            if len(buffer) < cls.LD2450_FRAME_LEN:
                break

            if buffer[28:30] == cls.LD2450_TAIL:
                frame = bytes(buffer[:cls.LD2450_FRAME_LEN])
                del buffer[:cls.LD2450_FRAME_LEN]
                contacts = cls._decode_ld2450(frame)
            else:
                del buffer[:1]

        return contacts, buffer

    @classmethod
    def _decode_ld2450(cls, frame: bytes) -> List[RadarContact]:
        contacts: List[RadarContact] = []
        now = time.monotonic()

        for slot in range(3):
            offset = 4 + slot * 8
            raw_x, raw_y, raw_speed, _ = struct.unpack_from("<HHHH", frame, offset)
            x_mm, y_mm, speed_cms = _ld2450_signed(raw_x), _ld2450_signed(raw_y), _ld2450_signed(raw_speed)

            # Ignore empty/inactive slots
            if x_mm == 0 and y_mm == 0 and speed_cms == 0:
                continue

            x_m = x_mm / 1000.0
            y_m = y_mm / 1000.0
            speed_mps = speed_cms / 100.0
            distance = math.sqrt(x_m * x_m + y_m * y_m)
            angle_deg = math.degrees(math.atan2(x_m, max(0.001, y_m)))

            contacts.append(
                RadarContact(
                    id=slot + 1,
                    x=x_m,
                    y=y_m,
                    speed=speed_mps,
                    distance=distance,
                    angle_deg=angle_deg,
                    timestamp=now,
                )
            )

        return contacts

    @classmethod
    def _decode_json(cls, line: str) -> List[RadarContact]:
        contacts: List[RadarContact] = []
        now = time.monotonic()
        try:
            data = json.loads(line)
            targets = data.get("targets", [])
            for idx, t in enumerate(targets):
                x = float(t.get("x", 0.0))
                y = float(t.get("y", 0.0))
                speed = float(t.get("speed", 0.0))
                dist = math.sqrt(x * x + y * y)
                angle = math.degrees(math.atan2(x, max(0.001, y)))
                contacts.append(
                    RadarContact(
                        id=t.get("id", idx + 1),
                        x=x,
                        y=y,
                        speed=speed,
                        distance=dist,
                        angle_deg=angle,
                        timestamp=now,
                        threat_zone=t.get("threat_zone"),
                    )
                )
        except (json.JSONDecodeError, ValueError, KeyError):
            pass
        return contacts
