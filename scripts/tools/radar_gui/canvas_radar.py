"""
Tactical Radar CRT Canvas rendering COD MW2 / R6 Siege style HUD.
"""

import math
import time
import tkinter as tk
from typing import Dict, List
from .protocol_decoders import RadarContact, zone_of


class RadarCanvas(tk.Canvas):
    """60 FPS double-buffered CRT tactical radar display."""

    COLOR_BG = "#070D0A"
    COLOR_GRID_DARK = "#0C2318"
    COLOR_GRID_LIGHT = "#154228"
    COLOR_GRID_TEXT = "#2B6842"
    COLOR_SWEEP_LINE = "#00FF66"
    COLOR_BLIP_NEAR = "#FF3333"   # Danger (<1.5m)
    COLOR_BLIP_MID = "#FFAA00"    # Caution (1.5 - 3.5m)
    COLOR_BLIP_FAR = "#00FF88"    # Clear (>3.5m)

    MAX_RANGE_M = 6.0  # Max radar display distance in meters

    def __init__(self, parent, size: int = 560):
        super().__init__(
            parent,
            width=size,
            height=size,
            bg=self.COLOR_BG,
            highlightthickness=1,
            highlightbackground="#154228",
        )
        self.size = size
        self.center_x = size // 2
        self.center_y = int(size * 0.85)  # Offset center downward for forward-facing 140° FoV
        self.max_radius = int(size * 0.75)
        self.sweep_angle_deg = 20.0       # Current sweep angle in degrees
        self.sweep_dir = 1.0              # Oscillate or rotate
        self.contacts: Dict[int, RadarContact] = {}
        self.last_seen: Dict[int, float] = {}

    def update_contacts(self, new_contacts: List[RadarContact]) -> None:
        now = time.monotonic()
        for contact in new_contacts:
            self.contacts[contact.id] = contact
            self.last_seen[contact.id] = now

    def advance_sweep(self, dt: float) -> None:
        """Advance radar sweep beam (sweeps -70 deg to +70 deg forward sector)."""
        speed_deg_per_sec = 80.0  # 1 full sweep cycle every ~3.5 seconds
        self.sweep_angle_deg += self.sweep_dir * speed_deg_per_sec * dt

        if self.sweep_angle_deg >= 70.0:
            self.sweep_angle_deg = 70.0
            self.sweep_dir = -1.0
        elif self.sweep_angle_deg <= -70.0:
            self.sweep_angle_deg = -70.0
            self.sweep_dir = 1.0

    def redraw(self) -> None:
        """Render complete radar HUD frame."""
        self.delete("all")
        self._draw_grid()
        self._draw_sweep_beam()
        self._draw_contacts()
        self._draw_alert_perimeter()

    def _meters_to_pixels(self, x_m: float, y_m: float) -> tuple:
        scale = self.max_radius / self.MAX_RANGE_M
        px = self.center_x + (x_m * scale)
        py = self.center_y - (y_m * scale)
        return px, py

    def _draw_grid(self) -> None:
        cx, cy = self.center_x, self.center_y

        # Draw concentric distance rings (1m to 6m)
        for r_m in range(1, int(self.MAX_RANGE_M) + 1):
            radius = (r_m / self.MAX_RANGE_M) * self.max_radius
            color = self.COLOR_GRID_LIGHT if r_m % 2 == 0 else self.COLOR_GRID_DARK
            self.create_oval(
                cx - radius, cy - radius, cx + radius, cy + radius,
                outline=color, width=1, dash=(2, 4) if r_m % 2 != 0 else ()
            )
            # Distance text labels along centerline
            self.create_text(
                cx + 4, cy - radius - 6,
                text=f"{r_m}m", fill=self.COLOR_GRID_TEXT, font=("Courier", 8, "bold")
            )

        # Azimuth ray lines (-60°, -30°, 0°, +30°, +60°)
        for deg in [-60, -30, 0, 30, 60]:
            rad = math.radians(deg)
            ex = cx + self.max_radius * math.sin(rad)
            ey = cy - self.max_radius * math.cos(rad)
            color = self.COLOR_GRID_LIGHT if deg == 0 else self.COLOR_GRID_DARK
            self.create_line(cx, cy, ex, ey, fill=color, width=1)
            self.create_text(
                ex + math.sin(rad) * 12, ey - math.cos(rad) * 12,
                text=f"{deg}°", fill=self.COLOR_GRID_TEXT, font=("Courier", 8)
            )

    def _draw_sweep_beam(self) -> None:
        cx, cy = self.center_x, self.center_y
        rad = math.radians(self.sweep_angle_deg)
        ex = cx + self.max_radius * math.sin(rad)
        ey = cy - self.max_radius * math.cos(rad)

        # Draw main sweep vector line
        self.create_line(cx, cy, ex, ey, fill=self.COLOR_SWEEP_LINE, width=2)

        # Trailing phosphor glow line
        trail_rad = math.radians(self.sweep_angle_deg - self.sweep_dir * 6.0)
        tx = cx + self.max_radius * math.sin(trail_rad)
        ty = cy - self.max_radius * math.cos(trail_rad)
        self.create_line(cx, cy, tx, ty, fill="#0D4424", width=4)

    def _draw_contacts(self) -> None:
        now = time.monotonic()
        decay_window_sec = 2.0
        stale_keys = []

        for cid, contact in self.contacts.items():
            age = now - self.last_seen.get(cid, 0.0)
            if age > decay_window_sec:
                stale_keys.append(cid)
                continue

            px, py = self._meters_to_pixels(contact.x, contact.y)

            # Determine threat color by zone
            color = {
                "Danger": self.COLOR_BLIP_NEAR, "Caution": self.COLOR_BLIP_MID
            }.get(zone_of(contact), self.COLOR_BLIP_FAR)

            # Blip size and fading opacity proxy
            radius = 6 if age < 0.5 else 4
            self.create_oval(
                px - radius, py - radius, px + radius, py + radius,
                fill=color, outline="#FFFFFF" if age < 0.2 else color, width=1
            )
            # Pulsing outer ring
            if age < 0.3:
                self.create_oval(
                    px - radius - 4, py - radius - 4, px + radius + 4, py + radius + 4,
                    outline=color, width=1
                )

            # Target ID label
            self.create_text(
                px + 10, py - 4,
                text=f"T{cid} [{contact.distance:.1f}m]",
                fill=color, font=("Courier", 9, "bold")
            )

        # Cleanup stale
        for k in stale_keys:
            del self.contacts[k]
            del self.last_seen[k]

    def _draw_alert_perimeter(self) -> None:
        """Flash warning border if any contact is in near/danger zone (<1.5m)."""
        has_near_threat = any(zone_of(c) == "Danger" for c in self.contacts.values())
        if has_near_threat and int(time.monotonic() * 4) % 2 == 0:
            self.create_rectangle(
                4, 4, self.size - 4, self.size - 4,
                outline=self.COLOR_BLIP_NEAR, width=3
            )
            self.create_text(
                self.center_x, 24,
                text="!! PROXIMITY ALERT — DANGER ZONE !!",
                fill=self.COLOR_BLIP_NEAR, font=("Courier", 12, "bold")
            )
