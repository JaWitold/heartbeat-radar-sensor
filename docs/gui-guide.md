# Radar Desktop GUI Guide

Desktop GUI: `./scripts/gui.sh` or `uv run radar-gui`. Tkinter radar HUD for testing, simulation, hardware telemetry.

---

## Features

- **CRT style:** background `#070D0A`, green sweep beam + grid.
- **Grid:** range rings 1.0 m to 6.0 m; azimuth rays -60° to +60°.
- **Sweep:** 60 FPS, oscillating beam, fading trail.
- **Blips by zone:**
  - `NEAR (<1.5m)`: red pulsing blip + flashing danger banner.
  - `MID (1.5 - 3.5m)`: amber.
  - `FAR (>3.5m)`: green.
- **Sidebar:** per-target $X, Y$ (m), velocity ($\text{m/s}$), distance, zone badge; connection controls; FPS counter.

---

## Running with `uv`

### 1. Demo Mode (no hardware)
Simulated moving targets:
```bash
./scripts/gui.sh --demo

# Or directly with uv:
cd scripts/tools && uv run radar-gui --demo
```

---

### 2. ESP32 Telemetry
ESP32 on USB, streaming JSON telemetry over UART (921600 baud, default):
```bash
./scripts/gui.sh --port /dev/ttyUSB0 --baud 921600
```

---

### 3. Raw HLK-LD2450 via USB-UART
Sensor on PC via USB-UART adapter (**PL2303HX**, **CP2102**, **CH340**):
```bash
./scripts/gui.sh --port /dev/ttyUSB0 --raw
```
*`--raw`: parse 256000 baud, 30-byte binary frames.*

> [!TIP]
> **PL2303HX:** voltage jumper → **3.3V logic**. LD2450 power from adapter 5V pin, GND to GND, cross RX/TX (`PL2303 RX` $\to$ `LD2450 TX`, `PL2303 TX` $\to$ `LD2450 RX`).
>
> `Permission denied: '/dev/ttyUSB0'` → add user to `dialout`:
> ```bash
> sudo usermod -a -G dialout $USER
> newgrp dialout
> ```

---

### 4. Simulator via Virtual PTY
**Terminal 1**, simulator on PTY:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
# Output will display: Connect client/tests to: /dev/pts/X
```

**Terminal 2**, GUI on that device:
```bash
./scripts/gui.sh --port /dev/pts/X --raw
```
GUI shows simulated patrol targets.

---

## Alternative: Web HUD

Use for browser, phone/tablet, or headless host without Tkinter/X11:

```bash
# Standalone Demo Mode (opens at http://localhost:8088):
./scripts/web.sh --demo

# Live physical radar sensor feed:
./scripts/web.sh --serial /dev/ttyUSB0 --raw

# Live ESP32 telemetry feed:
./scripts/web.sh --serial /dev/ttyUSB0 --baud 921600

# Full tmux simulation with Web HUD:
./scripts/sim-tmux.sh --web
```

### Web HUD
- HTML5 Canvas + WebSocket (`/ws`), 60 FPS, no build step.
- LAN: `http://<your-ip>:8088` from any device on same network.
- Audio: toggleable Web Audio beeps when target $<1.5$ m.
