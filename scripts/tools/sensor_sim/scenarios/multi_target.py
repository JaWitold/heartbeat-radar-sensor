"""
Multi-target scenario: 3 simultaneous contacts crossing the field of view.
"""

import math
from typing import List
from ..interfaces import IScenario, SimulatedTarget


class MultiTargetScenario(IScenario):
    """
    Simulates 3 simultaneous targets:
    - Target 1: Approaching center (X=0.0m, Y=4.5m -> 1.2m)
    - Target 2: Walking from left to right (X=-2.0m -> +2.0m, Y=3.0m)
    - Target 3: Stationary contact breathing at (X=1.5m, Y=2.0m)
    """

    def __init__(self):
        self._time = 0.0

    def update(self, dt: float) -> List[SimulatedTarget]:
        self._time += dt

        # Target 1: Oscillating depth
        t1_y = 2.8 + 1.8 * math.cos(0.8 * self._time)
        t1_speed = -1.8 * 0.8 * math.sin(0.8 * self._time)
        target1 = SimulatedTarget(id=1, x=0.1, y=t1_y, speed=t1_speed, is_valid=True)

        # Target 2: Crossing left to right
        t2_x = 2.0 * math.sin(0.5 * self._time)
        t2_speed = 0.3 * math.cos(0.5 * self._time)
        target2 = SimulatedTarget(id=2, x=t2_x, y=3.2, speed=t2_speed, is_valid=True)

        # Target 3: Slow wander near right edge
        t3_x = 1.4 + 0.3 * math.sin(0.2 * self._time)
        t3_y = 1.8 + 0.2 * math.cos(0.2 * self._time)
        target3 = SimulatedTarget(id=3, x=t3_x, y=t3_y, speed=0.05, is_valid=True)

        return [target1, target2, target3]
