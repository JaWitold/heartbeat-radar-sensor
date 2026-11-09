"""
Scenario generators registry.
"""

from typing import Dict, Type
from ..interfaces import IScenario
from .patrol import PatrolScenario
from .multi_target import MultiTargetScenario
from .interactive import InteractiveScenario

SCENARIOS: Dict[str, Type[IScenario]] = {
    "patrol": PatrolScenario,
    "multi": MultiTargetScenario,
    "interactive": InteractiveScenario,
}


def get_scenario(name: str) -> IScenario:
    name_lower = name.lower()
    if name_lower not in SCENARIOS:
        raise ValueError(
            f"Unknown scenario: '{name}'. Supported: {list(SCENARIOS.keys())}"
        )
    return SCENARIOS[name_lower]()
