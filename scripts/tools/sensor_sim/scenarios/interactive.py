"""
Interactive scenario: Target position driven by user input.
"""

from typing import List
from ..interfaces import IScenario, SimulatedTarget


class InteractiveScenario(IScenario):
    """
    Maintains a target whose coordinates can be nudged programmatically
    or via keyboard hooks.
    """

    def __init__(self, start_x: float = 0.0, start_y: float = 2.5):
        self.x = start_x
        self.y = start_y
        self.speed = 0.0

    def move(self, dx: float, dy: float) -> None:
        self.x = max(-3.0, min(3.0, self.x + dx))
        self.y = max(0.5, min(6.0, self.y + dy))

    def update(self, dt: float) -> List[SimulatedTarget]:
        return [
            SimulatedTarget(
                id=1,
                x=self.x,
                y=self.y,
                speed=self.speed,
                distance_resolution=0.1,
                is_valid=True,
            )
        ]
