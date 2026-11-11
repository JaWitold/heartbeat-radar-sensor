"""
Sidebar HUD: Connection controls, target status table, and telemetry stats.
"""

import tkinter as tk
from tkinter import ttk
from typing import Callable, Dict, List, Optional
from .protocol_decoders import ImuTelemetry, RadarContact, zone_of


class SidebarHUD(tk.Frame):
    """Tactical telemetry sidebar and serial communication panel."""

    COLOR_BG = "#0B1410"
    COLOR_FG = "#00FF88"
    COLOR_AMBER = "#FFAA00"
    COLOR_RED = "#FF3333"

    def __init__(
        self,
        parent,
        on_connect: Callable[[str, int], None],
        on_disconnect: Callable[[], None],
        on_demo: Callable[[], None],
    ):
        super().__init__(parent, bg=self.COLOR_BG, width=320, padx=12, pady=12)
        self.pack_propagate(False)

        self._on_connect = on_connect
        self._on_disconnect = on_disconnect
        self._on_demo = on_demo

        self._build_header()
        self._build_connection_panel()
        self._build_attitude_panel()
        self._build_target_table()
        self._build_status_bar()

    def _build_header(self) -> None:
        title = tk.Label(
            self,
            text="TACTICAL RADAR HUD",
            font=("Courier", 13, "bold"),
            fg=self.COLOR_FG,
            bg=self.COLOR_BG,
        )
        title.pack(anchor="w", pady=(0, 2))

        subtitle = tk.Label(
            self,
            text="COD MW2 / R6 FMCW mmWave Feed",
            font=("Courier", 8),
            fg="#2B6842",
            bg=self.COLOR_BG,
        )
        subtitle.pack(anchor="w", pady=(0, 12))

    def _build_connection_panel(self) -> None:
        frame = tk.LabelFrame(
            self,
            text=" CONNECTION ",
            font=("Courier", 9, "bold"),
            fg=self.COLOR_FG,
            bg=self.COLOR_BG,
            padx=8,
            pady=8,
        )
        frame.pack(fill="x", pady=(0, 12))

        # Port row
        tk.Label(frame, text="Port:", font=("Courier", 9), fg=self.COLOR_FG, bg=self.COLOR_BG).grid(
            row=0, column=0, sticky="w", pady=2
        )
        self.port_var = tk.StringVar(value="/dev/ttyUSB0")
        self.port_entry = tk.Entry(
            frame, textvariable=self.port_var, font=("Courier", 9), bg="#13241B", fg=self.COLOR_FG, width=16
        )
        self.port_entry.grid(row=0, column=1, sticky="e", pady=2)

        # Baud row
        tk.Label(frame, text="Baud:", font=("Courier", 9), fg=self.COLOR_FG, bg=self.COLOR_BG).grid(
            row=1, column=0, sticky="w", pady=2
        )
        self.baud_var = tk.StringVar(value="256000")
        self.baud_menu = ttk.Combobox(
            frame, textvariable=self.baud_var, values=["115200", "256000", "921600"], width=14, state="readonly"
        )
        self.baud_menu.grid(row=1, column=1, sticky="e", pady=2)

        # Button row
        btn_box = tk.Frame(frame, bg=self.COLOR_BG)
        btn_box.grid(row=2, column=0, columnspan=2, pady=(8, 0), sticky="ew")

        self.btn_connect = tk.Button(
            btn_box,
            text="CONNECT",
            font=("Courier", 8, "bold"),
            bg="#0E3D23",
            fg=self.COLOR_FG,
            command=self._handle_connect,
        )
        self.btn_connect.pack(side="left", expand=True, fill="x", padx=(0, 4))

        self.btn_demo = tk.Button(
            btn_box,
            text="DEMO",
            font=("Courier", 8, "bold"),
            bg="#2A3D12",
            fg=self.COLOR_AMBER,
            command=self._on_demo,
        )
        self.btn_demo.pack(side="right", expand=True, fill="x", padx=(4, 0))

    def _handle_connect(self) -> None:
        port = self.port_var.get().strip()
        baud = int(self.baud_var.get().strip())
        self._on_connect(port, baud)

    def _build_attitude_panel(self) -> None:
        frame = tk.LabelFrame(
            self,
            text=" ATTITUDE / IMU ",
            font=("Courier", 9, "bold"),
            fg=self.COLOR_FG,
            bg=self.COLOR_BG,
            padx=8,
            pady=6,
        )
        frame.pack(fill="x", pady=(0, 10))

        # Row 1: Pitch & Roll
        row1 = tk.Frame(frame, bg=self.COLOR_BG)
        row1.pack(fill="x", pady=1)
        self.lbl_pitch = tk.Label(row1, text="PITCH:  +0.0°", font=("Courier", 8), fg=self.COLOR_FG, bg=self.COLOR_BG)
        self.lbl_pitch.pack(side="left")
        self.lbl_roll = tk.Label(row1, text="ROLL:  +0.0°", font=("Courier", 8), fg=self.COLOR_FG, bg=self.COLOR_BG)
        self.lbl_roll.pack(side="right")

        # Row 2: Yaw & Temp
        row2 = tk.Frame(frame, bg=self.COLOR_BG)
        row2.pack(fill="x", pady=1)
        self.lbl_yaw = tk.Label(row2, text="YAW:    +0.0°", font=("Courier", 8), fg=self.COLOR_FG, bg=self.COLOR_BG)
        self.lbl_yaw.pack(side="left")
        self.lbl_temp = tk.Label(row2, text="TEMP: --.-°C", font=("Courier", 8), fg=self.COLOR_AMBER, bg=self.COLOR_BG)
        self.lbl_temp.pack(side="right")

        # Row 3: Accel G-force
        row3 = tk.Frame(frame, bg=self.COLOR_BG)
        row3.pack(fill="x", pady=1)
        self.lbl_accel = tk.Label(
            row3, text="ACC: 0.00 0.00 1.00 G", font=("Courier", 8), fg="#4E8C6A", bg=self.COLOR_BG
        )
        self.lbl_accel.pack(anchor="w")

    def update_attitude(self, imu: ImuTelemetry) -> None:
        self.lbl_pitch.config(text=f"PITCH: {imu.pitch:+5.1f}°")
        self.lbl_roll.config(text=f"ROLL: {imu.roll:+5.1f}°")
        self.lbl_yaw.config(text=f"YAW:  {imu.yaw:+5.1f}°")
        self.lbl_temp.config(text=f"TEMP: {imu.temp:4.1f}°C")
        self.lbl_accel.config(text=f"ACC: {imu.ax:+.2f} {imu.ay:+.2f} {imu.az:+.2f} G")

    def _build_target_table(self) -> None:
        table_frame = tk.LabelFrame(
            self,
            text=" ACTIVE CONTACTS ",
            font=("Courier", 9, "bold"),
            fg=self.COLOR_FG,
            bg=self.COLOR_BG,
            padx=4,
            pady=4,
        )
        table_frame.pack(fill="both", expand=True, pady=(0, 12))

        columns = ("id", "dist", "azimuth", "speed", "zone")
        self.tree = ttk.Treeview(table_frame, columns=columns, show="headings", height=6)
        self.tree.heading("id", text="ID")
        self.tree.heading("dist", text="Dist")
        self.tree.heading("azimuth", text="Azim")
        self.tree.heading("speed", text="Vel")
        self.tree.heading("zone", text="Zone")

        self.tree.column("id", width=36, anchor="center")
        self.tree.column("dist", width=52, anchor="center")
        self.tree.column("azimuth", width=48, anchor="center")
        self.tree.column("speed", width=52, anchor="center")
        self.tree.column("zone", width=52, anchor="center")

        self.tree.pack(fill="both", expand=True)

    def _build_status_bar(self) -> None:
        status_box = tk.Frame(self, bg=self.COLOR_BG)
        status_box.pack(fill="x", side="bottom")

        self.lbl_status = tk.Label(
            status_box,
            text="● STANDBY",
            font=("Courier", 9, "bold"),
            fg="#666666",
            bg=self.COLOR_BG,
        )
        self.lbl_status.pack(side="left")

        self.lbl_fps = tk.Label(
            status_box,
            text="FPS: --",
            font=("Courier", 9),
            fg="#2B6842",
            bg=self.COLOR_BG,
        )
        self.lbl_fps.pack(side="right")

    def update_status(self, is_online: bool, mode_text: str = "") -> None:
        if is_online:
            self.lbl_status.config(text=f"● {mode_text or 'ONLINE'}", fg=self.COLOR_FG)
            self.btn_connect.config(text="DISCONNECT", bg="#4D1212", fg=self.COLOR_RED, command=self._on_disconnect)
        else:
            self.lbl_status.config(text="● DISCONNECTED", fg="#666666")
            self.btn_connect.config(text="CONNECT", bg="#0E3D23", fg=self.COLOR_FG, command=self._handle_connect)

    def update_targets(self, contacts: List[RadarContact], fps: float) -> None:
        self.lbl_fps.config(text=f"FPS: {fps:.0f}")

        # Clear existing rows
        for item in self.tree.get_children():
            self.tree.delete(item)

        for c in contacts:
            zone = {"Danger": "NEAR", "Caution": "MID"}.get(zone_of(c), "FAR")

            self.tree.insert(
                "",
                "end",
                values=(
                    f"T{c.id}",
                    f"{c.distance:.2f}m",
                    f"{c.angle_deg:+.0f}°",
                    f"{c.speed:+.2f}m/s",
                    zone,
                ),
            )
