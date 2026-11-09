---
name: simulate-hardware
description: Guide and run multi-tier simulation for radar sensors, displays, and microcontroller boards without requiring physical hardware.
---

# Skill: Simulate Hardware

## Purpose
Enforces testing radar tracking, HUD display graphics, and peripheral protocols completely in software, via virtual PTY loopbacks, browser WebSockets, or benchtop USB-to-UART hardware-in-the-loop emulation.

---

## Simulation Modes

### Mode 1: USB-to-UART Bench Simulation (Hardware-in-the-Loop)
Simulate radar modules into a physical ESP32 using a USB-UART dongle (CP2102, FTDI, CH340):
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
./scripts/sim-radar.sh --sensor ld2450 --interactive --port /dev/ttyUSB0
```

---

### Mode 2: Headless Virtual PTY Simulation (No Hardware)
Create a virtual serial loopback to feed simulated radar data into native test applications or Wokwi:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
```

---

### Mode 3: Tactical Desktop HUD (Tkinter GUI)
Run desktop HUD visualization connecting to live radar feeds or synthetic demo streams:
```bash
./scripts/gui.sh --demo                  # Standalone synthetic demo
./scripts/gui.sh --port /dev/ttyUSB0 --raw  # Direct sensor feed @ 256k baud
./scripts/gui.sh --port /dev/ttyUSB0     # ESP32 JSON telemetry @ 115.2k baud
```

---

### Mode 4: Tactical Web HUD (Browser Display)
FastAPI and WebSocket server serving an HTML5 Canvas tactical radar display:
```bash
./scripts/web.sh --demo                  # Synthetic demo stream on http://127.0.0.1:8899
./scripts/web.sh --port /dev/ttyUSB0 --raw  # Raw radar stream on browser HUD
./scripts/web.sh --port /dev/ttyUSB0     # ESP32 telemetry on browser HUD
```

---

### Mode 5: All-in-One Simulator & HUD Runner (tmux)
Launch sensor simulation and HUD in a synchronized multi-pane tmux session:
```bash
./scripts/sim-tmux.sh                    # Simulator + Desktop HUD
./scripts/sim-tmux.sh --web              # Simulator + Web HUD server
./scripts/sim-tmux.sh --scenario multi   # Multi-target scenario
```

---

### Mode 6: Wokwi MCU & Display Emulation
Simulate the ESP32 CPU, SPI TFT display, and custom LD2450 chip in browser:
1. Compile firmware:
   ```bash
   ./scripts/build.sh
   ```
2. Open Wokwi in VS Code or in browser via `diagram.json` and `wokwi.toml`.

---

## Pre-Purchase Sensor Evaluation
Before ordering a new radar sensor model:
1. Add its protocol encoder to `scripts/tools/sensor_sim/protocols/`.
2. Verify signed integer format (sign-magnitude vs two's complement) against official datasheet byte examples.
3. Run automated protocol verification tests:
   ```bash
   ./scripts/test-python.sh
   ```
4. Stream synthetic frames into the firmware parser to verify parsing speed, memory footprint, and edge-case handling.
