"""
Non-blocking threaded serial reader for physical UART, virtual PTY, or demo mode.
"""

import math
import threading
import time
from typing import Callable, List, Optional
from .protocol_decoders import FrameDecoder, RadarContact


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
        import serial

        buffer = bytearray()
        try:
            self._serial = serial.Serial(
                port=self._port_name,
                baudrate=self._baud_rate,
                timeout=0.1,
            )
            print(f"[GUI] Connected to {self._port_name} @ {self._baud_rate}")

            while self._running:
                data = self._serial.read(64)
                if data:
                    buffer.extend(data)
                    contacts, buffer = FrameDecoder.parse_stream(buffer)
                    if contacts:
                        self._callback(contacts)
                else:
                    time.sleep(0.01)

        except serial.SerialException as e:
            if "Permission denied" in str(e):
                print(f"[GUI Error] Permission denied accessing '{self._port_name}'.")
                print(
                    "[GUI Tip] Add your Linux user to the 'dialout' group: "
                    "sudo usermod -a -G dialout $USER (then re-login or run 'newgrp dialout')."
                )
            else:
                print(f"[GUI] Serial error on {self._port_name}: {e}")
        except PermissionError:
            print(f"[GUI Error] Permission denied accessing '{self._port_name}'.")
            print(
                "[GUI Tip] Add your Linux user to the 'dialout' group: "
                "sudo usermod -a -G dialout $USER (then re-login or run 'newgrp dialout')."
            )
        except Exception as e:
            print(f"[GUI] Serial error on {self._port_name}: {e}")
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
        sim_time = 0.0
        while self._running:
            sim_time += 0.05
            now = time.monotonic()

            # Target 1: Approaching center (patrol)
            y1 = 2.8 + 1.8 * math.cos(0.8 * sim_time)
            s1 = -1.8 * 0.8 * math.sin(0.8 * sim_time)
            d1 = math.sqrt(0.1 * 0.1 + y1 * y1)
            t1 = RadarContact(
                id=1, x=0.1, y=y1, speed=s1, distance=d1,
                angle_deg=math.degrees(math.atan2(0.1, y1)), timestamp=now
            )

            # Target 2: Walking left to right
            x2 = 2.0 * math.sin(0.5 * sim_time)
            s2 = 0.3 * math.cos(0.5 * sim_time)
            d2 = math.sqrt(x2 * x2 + 3.2 * 3.2)
            t2 = RadarContact(
                id=2, x=x2, y=3.2, speed=s2, distance=d2,
                angle_deg=math.degrees(math.atan2(x2, 3.2)), timestamp=now
            )

            # Target 3: Wandering flank
            x3 = -1.4 + 0.3 * math.sin(0.3 * sim_time)
            y3 = 2.0 + 0.3 * math.cos(0.3 * sim_time)
            d3 = math.sqrt(x3 * x3 + y3 * y3)
            t3 = RadarContact(
                id=3, x=x3, y=y3, speed=0.1, distance=d3,
                angle_deg=math.degrees(math.atan2(x3, y3)), timestamp=now
            )

            self._callback([t1, t2, t3])
            time.sleep(0.05)
