"""
ESP32 JSON Telemetry Protocol Encoder.
Simulates the ESP32 console telemetry stream containing radar targets and MPU-6050 IMU.
"""

import json
import math
import time
from typing import List

from ..interfaces import ISensorProtocol, SimulatedTarget


def demo_imu(t: float) -> dict:
    """Fake MPU-6050 reading at t seconds: gentle wobble plus a slow 8 deg/s turn. Shared by all demo feeds."""
    return {
        "pitch": round(3.5 * math.sin(t * 1.2), 1),
        "roll": round(6.0 * math.cos(t * 0.8), 1),
        "yaw": round((t * 8.0) % 360.0 - 180.0, 1),
        "ax": round(0.02 * math.sin(t), 2),
        "ay": round(0.03 * math.cos(t), 2),
        "az": round(0.98 + 0.02 * math.sin(t * 2.0), 2),
        "gx": round(0.5 * math.sin(t), 1),
        "gy": round(0.4 * math.cos(t), 1),
        "gz": 8.0,
        "temp": 24.5,
    }


class ESP32TelemetryEncoder(ISensorProtocol):
    """Encodes simulated targets and MPU-6050 IMU attitude into ESP32 JSON telemetry lines."""

    def __init__(self, danger_m: float = 1.5, caution_m: float = 3.5, max_m: float = 6.0):
        self._danger_m = danger_m
        self._caution_m = caution_m
        self._max_m = max_m
        self._sweep_deg = -60.0
        self._sweep_direction = 1.0
        self._start_time = time.monotonic()
        self._last_time = time.monotonic()

    @property
    def name(self) -> str:
        return "ESP32 Telemetry (JSON + MPU-6050 IMU)"

    @property
    def default_baud_rate(self) -> int:
        return 921600

    @property
    def max_targets(self) -> int:
        return 3

    def encode_frame(self, targets: List[SimulatedTarget]) -> bytes:
        now = time.monotonic()
        dt = max(0.001, now - self._last_time)
        self._last_time = now
        elapsed = now - self._start_time

        # Advance simulated sweep back and forth across [-60, +60] deg
        sweep_speed_deg_s = 120.0
        self._sweep_deg += self._sweep_direction * sweep_speed_deg_s * dt
        if self._sweep_deg >= 60.0:
            self._sweep_deg = 60.0
            self._sweep_direction = -1.0
        elif self._sweep_deg <= -60.0:
            self._sweep_deg = -60.0
            self._sweep_direction = 1.0

        active_targets = []
        nearest_m = None
        for t in targets[: self.max_targets]:
            if not t.is_valid:
                continue
            dist = math.hypot(t.x, t.y)
            nearest_m = dist if nearest_m is None else min(nearest_m, dist)
            active_targets.append(
                {
                    "id": t.id,
                    "x": round(t.x, 3),
                    "y": round(t.y, 3),
                    "speed": round(t.speed, 2),
                    "distance": round(dist, 3),
                    "threat_zone": self._zone(dist),
                }
            )

        # Same rule as firmware DistanceThreatEvaluator: nearest target sets alert, severity = 1 - d / max
        severity = 0.0 if nearest_m is None else min(1.0, max(0.0, 1.0 - nearest_m / self._max_m))
        frame_dict = {
            "type": "radar_frame",
            "sweep_deg": round(self._sweep_deg, 1),
            "alert": "Safe" if nearest_m is None else self._zone(nearest_m),
            "severity": round(severity, 2),
            "targets": active_targets,
            "imu": demo_imu(elapsed),
        }

        return (json.dumps(frame_dict) + "\n").encode("utf-8")

    def _zone(self, dist: float) -> str:
        if dist < self._danger_m:
            return "Danger"
        if dist <= self._caution_m:
            return "Caution"
        return "Safe"
