"""
Physical USB-UART serial port sink.
"""

from ..interfaces import IOutputSink


class SerialPortSink(IOutputSink):
    """Writes frames to a physical USB-UART adapter (e.g., /dev/ttyUSB0)."""

    def __init__(self, port: str, baud_rate: int):
        self._port_name = port
        self._baud_rate = baud_rate
        self._serial = None

    def open(self) -> None:
        try:
            import serial
        except ImportError:
            raise RuntimeError(
                "pyserial is required for physical UART output. Run: pip install pyserial"
            )

        try:
            self._serial = serial.Serial(
                port=self._port_name,
                baudrate=self._baud_rate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=1.0,
            )
            print(f"[SerialSink] Opened {self._port_name} @ {self._baud_rate} baud")
        except serial.SerialException as e:
            if "Permission denied" in str(e):
                raise PermissionError(
                    f"Permission denied accessing '{self._port_name}'. "
                    "Add your Linux user to the 'dialout' group: sudo usermod -a -G dialout $USER "
                    "(then log out and log back in, or run 'newgrp dialout' in your terminal)."
                ) from e
            raise

    def write(self, data: bytes) -> None:
        if self._serial and self._serial.is_open:
            self._serial.write(data)
            self._serial.flush()

    def close(self) -> None:
        if self._serial and self._serial.is_open:
            self._serial.close()
            print(f"[SerialSink] Closed {self._port_name}")
