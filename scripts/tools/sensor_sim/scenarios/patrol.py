"""
Patrol scenario: A single target moving back and forth in front of the sensor.
"""

from typing import List
from ..interfaces import IScenario, SimulatedTarget


class PatrolScenario(IScenario):
    """
    Simulates a human walking toward the sensor from 5.0m to 1.0m,
    then turning around and walking away back to 5.0m.
    """

    def __init__(self, speed_mps: float = 0.8, min_y: float = 1.0, max_y: float = 5.0):
        self._speed_mps = speed_mps
        self._min_y = min_y
        self._max_y = max_y
        self._current_y = max_y
        self._direction = -1.0  # -1 = approaching, +1 = retreating

    def update(self, dt: float) -> List[SimulatedTarget]:
        self._current_y += self._direction * self._speed_mps * dt

        if self._current_y <= self._min_y:
            self._current_y = self._min_y
            self._direction = 1.0
        elif self._current_y >= self._max_y:
            self._current_y = self._max_y
            self._direction = -1.0

        target = SimulatedTarget(
            id=1,
            x=0.0,
            y=self._current_y,
            speed=self._direction * self._speed_mps,
            distance_resolution=0.2,
            is_valid=True,
        )
        return [target]
