---
name: simulate-hardware
description: Guide and run multi-tier simulation for radar sensors, displays, and microcontroller boards without requiring physical hardware.
---

# Skill: Simulate Hardware

## Purpose
Enables testing radar tracking, HUD display graphics, and peripheral protocols completely in software or via benchtop USB-to-UART hardware-in-the-loop emulation.

---

## Simulation Modes

### Mode 1: USB-to-UART Bench Simulation (Hardware-in-the-Loop)
Simulate radar modules into a physical ESP32 using a cheap USB-UART dongle (CP2102, FTDI, CH340):
```
PC USB Port ---> USB-UART TX Pin ---> ESP32 GPIO 16 (RX2)
                 USB-UART GND    ---> ESP32 GND
```

Commands:
```bash
# Simulate HLK-LD2450 patrolling contact
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0

# Simulate Ai-Thinker RD-03D (120° wide angle)
./scripts/sim-radar.sh --sensor rd03d --scenario multi --port /dev/ttyUSB0

# Interactive mode (control target with keyboard arrow keys):
./scripts/sim-radar.sh --sensor ld2450 --interactive --port /dev/ttyUSB0
```

---

### Mode 2: Headless Virtual PTY Simulation (No Hardware)
On Linux, create a virtual serial loopback to feed simulated radar data into native test applications or Wokwi:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
```

---

### Mode 3: Wokwi MCU & Display Emulation
Simulate the ESP32 CPU, SPI TFT display, and radar sensor in browser or CI:
1. Compile firmware:
   ```bash
   ./scripts/build.sh
   ```
2. Open Wokwi in VS Code or in browser via `diagram.json` and `wokwi.toml`.

---

### Mode 4: Multi-Sensor & Dual-Display Bench Simulation
Test complex configurations (e.g. dual radar sensors or local TFT + live PC Serial telemetry):
1. **Build with dual display sinks (TFT + Serial PC logging):**
   ```bash
   ./scripts/build.sh --displays st7789,serial
   ```
2. **Build with multi-sensor fusion (LD2450 + RD-03D):**
   ```bash
   ./scripts/build.sh --sensors ld2450,rd03d --displays st7789,serial
   ```
3. **Simulate multiple sensors via USB-UART or PTY:**
   Run multiple simulator instances on separate ports or channels to feed the fusion engine.

---

## Pre-Purchase Sensor Evaluation
Before ordering a new radar sensor model:
1. Add its protocol encoder to `scripts/tools/sensor_sim/protocols/`.
2. Run automated protocol verification tests:
   ```bash
   python3 -m unittest discover -s scripts/tools/sensor_sim/tests
   ```
3. Stream synthetic frames into the firmware parser to verify parsing speed, memory footprint, and edge-case handling.
