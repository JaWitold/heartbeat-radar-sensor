"""
Abstract interfaces and data structures for hexagonal sensor simulator.
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Optional


@dataclass
class SimulatedTarget:
    """Represents a tracked radar contact in Cartesian space."""
    id: int
    x: float          # Meters (positive = right, negative = left)
    y: float          # Meters (distance in front of sensor)
    speed: float      # Meters per second (positive = moving away, negative = approaching)
    distance_resolution: float = 0.2  # Measurement resolution in meters
    is_valid: bool = True


class ISensorProtocol(ABC):
    """Port interface for radar sensor protocol encoders."""

    @property
    @abstractmethod
    def name(self) -> str:
        """Name of the sensor model."""
        pass

    @property
    @abstractmethod
    def default_baud_rate(self) -> int:
        """Default baud rate for UART communication."""
        pass

    @property
    @abstractmethod
    def max_targets(self) -> int:
        """Maximum number of simultaneous targets supported."""
        pass

    @abstractmethod
    def encode_frame(self, targets: List[SimulatedTarget]) -> bytes:
        """
        Encode target positions into sensor-specific binary transmission packet.
        """
        pass


class IOutputSink(ABC):
    """Port interface for data sinks (physical UART, virtual PTY, socket)."""

    @abstractmethod
    def open(self) -> None:
        """Open or initialize the communication channel."""
        pass

    @abstractmethod
    def write(self, data: bytes) -> None:
        """Send raw binary frame to the destination."""
        pass

    @abstractmethod
    def close(self) -> None:
        """Close and clean up the communication channel."""
        pass


class IScenario(ABC):
    """Port interface for target motion scenario generators."""

    @abstractmethod
    def update(self, dt: float) -> List[SimulatedTarget]:
        """
        Advance simulation time by dt seconds and return current target states.
        """
        pass
