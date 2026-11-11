"""
Main application window coordinating RadarCanvas, SidebarHUD, and SerialReaderThread.
"""

import argparse
import sys
import time
import tkinter as tk
from typing import List, Optional

from .canvas_radar import RadarCanvas
from .protocol_decoders import ImuTelemetry, RadarContact
from .serial_reader import SerialReaderThread
from .sidebar_hud import SidebarHUD


class RadarGuiApp:
    """Tactical Radar Desktop Telemetry Window."""

    def __init__(self, port: str = None, baud: int = 921600, demo: bool = False):
        self.root = tk.Tk()
        self.root.title("Tactical Heartbeat Radar HUD")
        self.root.geometry("920x620")
        self.root.configure(bg="#070D0A")
        self.root.minsize(800, 560)

        self._last_frame_time = time.monotonic()
        self._fps = 0.0
        self._fps_counter = 0
        self._fps_timer = time.monotonic()
        # Written by the reader thread, applied in _render_loop: Tk widgets are main-thread only.
        self._latest_contacts: Optional[List[RadarContact]] = None
        self._latest_imu: Optional[ImuTelemetry] = None

        # Build subcomponents
        self._build_layout()

        # Serial worker thread
        self.reader = SerialReaderThread(self._on_contacts_received, self._on_imu_received)

        # Handle auto-start flags
        if demo:
            self._start_demo()
        elif port:
            self.sidebar.port_var.set(port)
            self.sidebar.baud_var.set(str(baud))
            self._connect_serial(port, baud)

        # Start 60 FPS render loop
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        self._render_loop()

    def _build_layout(self) -> None:
        container = tk.Frame(self.root, bg="#070D0A")
        container.pack(fill="both", expand=True, padx=8, pady=8)

        # Left side: Radar Sweep CRT
        self.canvas = RadarCanvas(container, size=580)
        self.canvas.pack(side="left", fill="both", expand=True)

        # Right side: Tactical HUD & Controls
        self.sidebar = SidebarHUD(
            container,
            on_connect=self._connect_serial,
            on_disconnect=self._disconnect,
            on_demo=self._start_demo,
        )
        self.sidebar.pack(side="right", fill="y", padx=(8, 0))

    def _on_contacts_received(self, contacts: List[RadarContact]) -> None:
        self._latest_contacts = contacts

    def _on_imu_received(self, imu: ImuTelemetry) -> None:
        self._latest_imu = imu

    def _connect_serial(self, port: str, baud: int) -> None:
        self.reader.start_serial(port, baud)
        self.sidebar.update_status(True, f"UART {baud}")

    def _start_demo(self) -> None:
        self.reader.start_demo()
        self.sidebar.update_status(True, "DEMO MODE")

    def _disconnect(self) -> None:
        self.reader.stop()
        self.sidebar.update_status(False)

    def _render_loop(self) -> None:
        now = time.monotonic()
        dt = now - self._last_frame_time
        self._last_frame_time = now

        # Compute FPS
        self._fps_counter += 1
        if now - self._fps_timer >= 1.0:
            self._fps = self._fps_counter / (now - self._fps_timer)
            self._fps_counter = 0
            self._fps_timer = now

        # Update radar sweep and contacts
        self.canvas.advance_sweep(dt)
        contacts, self._latest_contacts = self._latest_contacts, None
        if contacts is not None:  # [] clears the table; stale blips fade out on the canvas
            self.canvas.update_contacts(contacts)
            self.sidebar.update_targets(contacts, self._fps)
        imu, self._latest_imu = self._latest_imu, None
        if imu is not None:
            self.sidebar.update_attitude(imu)

        self.canvas.redraw()

        # Schedule next frame at ~60 FPS (16ms)
        self.root.after(16, self._render_loop)

    def _on_close(self) -> None:
        self.reader.stop()
        self.root.destroy()

    def run(self) -> None:
        self.root.mainloop()


def parse_args():
    parser = argparse.ArgumentParser(description="Tactical Heartbeat Radar Desktop HUD")
    parser.add_argument("--port", type=str, help="Serial port (e.g. /dev/ttyUSB0 or /dev/pts/15)")
    parser.add_argument("--baud", type=int, default=921600, help="Baud rate (default: 921600)")
    parser.add_argument("--demo", action="store_true", help="Launch in standalone demo mode")
    parser.add_argument("--raw", action="store_true", help="Expect raw 256000 baud LD2450 sensor packets")
    return parser.parse_args()


def main():
    args = parse_args()
    baud = 256000 if args.raw else args.baud
    app = RadarGuiApp(port=args.port, baud=baud, demo=args.demo)
    app.run()


if __name__ == "__main__":
    main()
