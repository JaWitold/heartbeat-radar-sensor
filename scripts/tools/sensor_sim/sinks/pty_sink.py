"""
Linux Virtual Pseudo-Terminal (PTY) Sink.
Creates a virtual serial port pair (/dev/pts/X) for hardware-free simulation.
"""

import os
from ..interfaces import IOutputSink


class PtySink(IOutputSink):
    """
    Creates a Linux pseudo-terminal pair.
    The simulator writes to the master side; external test programs or Wokwi
    can read from the printed slave device path (e.g., /dev/pts/3).
    """

    def __init__(self, pty_file: str = "/tmp/hrs_radar_pty"):
        self._master_fd = None
        self._slave_fd = None
        self.slave_name = None
        self._pty_file = pty_file

    def open(self) -> None:
        import pty
        self._master_fd, self._slave_fd = pty.openpty()
        self.slave_name = os.ttyname(self._slave_fd)
        if self._pty_file:
            try:
                with open(self._pty_file, "w") as f:
                    f.write(self.slave_name)
            except OSError:
                pass
        print("=" * 60)
        print(f"[PtySink] Virtual Serial Port ready!")
        print(f"          Connect client/tests to: {self.slave_name}")
        if self._pty_file:
            print(f"          Device path saved to:    {self._pty_file}")
        print("=" * 60)

    def write(self, data: bytes) -> None:
        if self._master_fd is not None:
            os.write(self._master_fd, data)

    def close(self) -> None:
        if self._master_fd is not None:
            try:
                os.close(self._master_fd)
            except OSError:
                pass
            self._master_fd = None

        if self._slave_fd is not None:
            try:
                os.close(self._slave_fd)
            except OSError:
                pass
            self._slave_fd = None

        if self._pty_file:
            try:
                if os.path.exists(self._pty_file):
                    os.remove(self._pty_file)
            except OSError:
                pass

        print("[PtySink] Virtual port closed.")
