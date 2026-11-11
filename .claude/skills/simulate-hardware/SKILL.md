---
name: simulate-hardware
description: Run multi-tier simulation of radar sensors, displays, and ESP32 board without physical hardware — USB-UART HIL, virtual PTY, desktop/web HUD, tmux runner, Wokwi.
---

# Skill: Simulate Hardware

## Purpose
Test radar tracking, HUD graphics, peripheral protocols in software: virtual PTY loopback, browser WebSocket, or bench USB-UART hardware-in-the-loop.

---

## Simulation Modes

### Mode 1: USB-UART Bench (Hardware-in-the-Loop)
Feed simulated radar into real ESP32 via USB-UART dongle (CP2102, FTDI, CH340):
```
PC USB Port ---> USB-UART TX Pin ---> ESP32 GPIO 16 (RX2)
                 USB-UART GND    ---> ESP32 GND
```

Commands:
```bash
# Simulate HLK-LD2450 patrolling contact
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0

# Simulate Ai-Thinker RD-03D (wide angle)
./scripts/sim-radar.sh --sensor rd03d --scenario multi --port /dev/ttyUSB0

# Interactive mode (control target with keyboard arrow keys):
./scripts/sim-radar.sh --sensor ld2450 --scenario interactive --port /dev/ttyUSB0
```

---

### Mode 2: Headless Virtual PTY (No Hardware)
Virtual serial loopback. Feeds native apps or Wokwi:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
```

---

### Mode 3: Desktop HUD (Tkinter)
Live radar feed or synthetic demo:
```bash
./scripts/gui.sh --demo                  # Standalone synthetic demo
./scripts/gui.sh --port /dev/ttyUSB0 --raw  # Direct sensor feed @ 256k baud
./scripts/gui.sh --port /dev/ttyUSB0     # ESP32 JSON telemetry @ 921.6k baud
```

---

### Mode 4: Web HUD (Browser)
aiohttp + WebSocket server, HTML5 Canvas display:
```bash
./scripts/web.sh --demo                    # Synthetic demo stream on http://127.0.0.1:8088
./scripts/web.sh --serial /dev/ttyUSB0 --raw  # Raw radar stream on browser HUD
./scripts/web.sh --serial /dev/ttyUSB0     # ESP32 telemetry on browser HUD
```

---

### Mode 5: Simulator + HUD in tmux
Simulator and HUD in multi-pane tmux session:
```bash
./scripts/sim-tmux.sh                    # Simulator + Desktop HUD
./scripts/sim-tmux.sh --web              # Simulator + Web HUD server
./scripts/sim-tmux.sh --scenario multi   # Multi-target scenario
```

---

### Mode 6: Wokwi MCU & Display Emulation
ESP32 CPU, SPI TFT, custom LD2450 chip (`simulation/wokwi/`) in browser:
1. Build:
   ```bash
   ./scripts/build.sh
   ```
2. Open Wokwi (VS Code or browser) with `diagram.json`, `wokwi.toml`.

---

## Pre-Purchase Sensor Evaluation
Before ordering new radar model:
1. Add protocol encoder to `scripts/tools/sensor_sim/protocols/`.
2. Check signed int format (sign-magnitude vs two's complement) against datasheet byte examples.
3. Run protocol tests:
   ```bash
   ./scripts/test-python.sh
   ```
4. Stream synthetic frames into firmware parser. Check parse speed, memory footprint, edge cases.
