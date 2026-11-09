"""
Non-blocking threaded serial reader for physical UART, virtual PTY, or demo mode.
"""

import threading
import time
from typing import Callable, List, Optional

from sensor_sim.protocols.ld2450 import LD2450Encoder
from sensor_sim.scenarios import get_scenario
from .protocol_decoders import FrameDecoder, RadarContact


def open_serial(port: str, baud: int, timeout: float):
    """Opens a UART, PTY or pyserial URL (e.g. socket://heartbeat-radar.local:3333).

    PTYs get 115200 because they reject non-standard rates like 256000; sockets ignore baud.
    """
    import serial

    if port.startswith("/dev/pts/"):
        baud = 115200
    return serial.serial_for_url(port, baudrate=baud, timeout=timeout)


def print_serial_error(tag: str, port: str, exc: Exception) -> None:
    if isinstance(exc, PermissionError) or "Permission denied" in str(exc):
        print(f"[{tag} Error] Permission denied accessing '{port}'.")
        print(
            f"[{tag} Tip] Add your Linux user to the 'dialout' group: "
            "sudo usermod -a -G dialout $USER (then re-login or run 'newgrp dialout')."
        )
    else:
        print(f"[{tag}] Serial error on {port}: {exc}")


def simulated_contacts(scenario, dt: float) -> List[RadarContact]:
    """Runs a sensor_sim scenario through the real LD2450 encoder and decoder."""
    frame = LD2450Encoder().encode_frame(scenario.update(dt))
    return FrameDecoder.parse_stream(bytearray(frame))[0]


class SerialReaderThread:
    """Background reader thread reading from serial port or demo generator."""

    def __init__(self, on_contacts_received: Callable[[List[RadarContact]], None]):
        self._callback = on_contacts_received
        self._running = False
        self._thread: Optional[threading.Thread] = None
        self._port_name: Optional[str] = None
        self._baud_rate: int = 115200
        self._demo_mode: bool = False
        self._serial = None

    def start_serial(self, port: str, baud_rate: int) -> None:
        self.stop()
        self._port_name = port
        self._baud_rate = baud_rate
        self._demo_mode = False
        self._running = True
        self._thread = threading.Thread(target=self._serial_loop, daemon=True)
        self._thread.start()

    def start_demo(self) -> None:
        self.stop()
        self._demo_mode = True
        self._running = True
        self._thread = threading.Thread(target=self._demo_loop, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._running = False
        if self._thread and self._thread.is_alive():
            self._thread.join(timeout=1.0)
        self._thread = None
        if self._serial:
            try:
                self._serial.close()
            except Exception:
                pass
            self._serial = None

    @property
    def is_connected(self) -> bool:
        if self._demo_mode:
            return True
        return self._serial is not None and getattr(self._serial, "is_open", False)

    def _serial_loop(self) -> None:
        buffer = bytearray()
        try:
            self._serial = open_serial(self._port_name, self._baud_rate, timeout=0.1)
            print(f"[GUI] Connected to {self._port_name} @ {self._serial.baudrate}")

            while self._running:
                data = self._serial.read(64)
                if data:
                    buffer.extend(data)
                    contacts, buffer = FrameDecoder.parse_stream(buffer)
                    if contacts:
                        self._callback(contacts)
                else:
                    time.sleep(0.01)

        except Exception as e:
            print_serial_error("GUI", self._port_name, e)
        finally:
            if self._serial:
                try:
                    self._serial.close()
                except Exception:
                    pass
                self._serial = None

    def _demo_loop(self) -> None:
        """Synthetic demo contact generator for hardware-free testing."""
        print("[GUI] Demo mode started.")
        scenario = get_scenario("multi")
        while self._running:
            self._callback(simulated_contacts(scenario, 0.05))
            time.sleep(0.05)
