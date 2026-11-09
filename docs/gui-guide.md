# Tactical Radar Desktop GUI Guide

The **Tactical Radar Desktop GUI** (`./scripts/gui.sh` or `uv run radar-gui`) provides a real-time Call of Duty: Modern Warfare 2 / Rainbow Six Siege style radar HUD for testing, simulation, and hardware telemetry.

---

## 🎨 Features

- **CRT Phosphor Aesthetic:** Dark background (`#070D0A`) with high-contrast phosphor green sweep beam and grid.
- **Range & Azimuth Grid:** 1.0m to 6.0m concentric range rings and azimuth ray lines (-60° to +60°).
- **Smooth 60 FPS Sweep:** Oscillating radar beam with fading phosphor trail.
- **Dynamic Blips & Threat Alert:**
  - `NEAR (<1.5m)`: Red pulsing blip with flashing proximity danger banner.
  - `MID (1.5 - 3.5m)`: Amber caution blip.
  - `FAR (>3.5m)`: Green clear blip.
- **Telemetry Sidebar:** Live coordinates table ($X, Y$ in meters, velocity in $\text{m/s}$, distance, threat zone badge), connection controls, and FPS counter.

---

## 🚀 Running the GUI with `uv`

### 1. Standalone Demo Mode (No Hardware / No Cables)
Instantly preview the radar sweep and moving simulated targets:
```bash
./scripts/gui.sh --demo

# Or directly with uv:
cd scripts/tools && uv run radar-gui --demo
```

---

### 2. Connected to Physical ESP32 Board
When the ESP32 is plugged in via USB and streaming telemetry over UART:
```bash
./scripts/gui.sh --port /dev/ttyUSB0 --baud 115200
```

---

### 3. Connected Directly to Raw HLK-LD2450 Radar Sensor
When the radar sensor is plugged into your PC via a USB-to-UART adapter (e.g. **PL2303HX**, **CP2102**, or **CH340**):
```bash
./scripts/gui.sh --port /dev/ttyUSB0 --raw
```
*(The `--raw` flag configures the parser for 256000 baud 30-byte binary FMCW frames).*

> [!TIP]
> **Hardware Tip for PL2303HX:** Ensure the adapter's voltage jumper is set to **3.3V logic level**. Power the LD2450 sensor from the adapter's 5V pin, connect GND to GND, and cross RX/TX (`PL2303 RX` $\to$ `LD2450 TX`, `PL2303 TX` $\to$ `LD2450 RX`).
>
> If you encounter `Permission denied: '/dev/ttyUSB0'`, grant your Linux user access to the serial device:
> ```bash
> sudo usermod -a -G dialout $USER
> newgrp dialout
> ```

---

### 4. Connected to the Radar Simulator (Virtual PTY Loopback)
In **Terminal 1**, start the sensor simulator on a virtual PTY:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
# Output will display: Connect client/tests to: /dev/pts/X
```

In **Terminal 2**, launch the GUI pointing to the virtual device:
```bash
./scripts/gui.sh --port /dev/pts/X --raw
```
The GUI will render the simulated patrolling targets in real-time!
