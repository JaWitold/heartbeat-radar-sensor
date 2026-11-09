# Simulation Guide: Testing Before Ordering Hardware

This guide explains how to test every component of the Heartbeat Radar Sensor without needing physical hardware connected.

---

## 1. Fast Native Unit Tests (Catch2 in Docker)
Run logic tests, coordinate math, and state machine transitions in milliseconds:
```bash
./scripts/test-native.sh
```
Uses `MockSensor` and `MockDisplay` to verify the core domain deterministically.

---

## 2. USB-to-UART Benchtop Hardware-in-the-Loop Simulation
If you have an ESP32 and display, but **no physical radar sensor** (or want to test reproducible target movement scenarios):

### Wiring
Connect a standard USB-UART adapter (**PL2303HX**, **CP2102**, **FT232**, **CH340**) to the ESP32:
- Adapter `TX` $\to$ ESP32 `GPIO 16 (RX2)`
- Adapter `GND` $\to$ ESP32 `GND`
- **Jumper:** Ensure adapter voltage jumper is set to **3.3V logic** (do NOT use 5V logic on ESP32 GPIOs).

### Execution
Run the modular sensor simulator:
```bash
# Simulate HLK-LD2450 patrolling contact (1m to 5m)
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0

# Simulate 3 crossing targets
./scripts/sim-radar.sh --sensor ld2450 --scenario multi --port /dev/ttyUSB0

# Interactive keyboard control
./scripts/sim-radar.sh --sensor ld2450 --interactive --port /dev/ttyUSB0
```

---

## 3. Headless Linux Virtual Serial (No Hardware)
Test protocol parsers and native desktop apps on your Linux workstation without any physical dongle:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
```
The script will output:
```
[PtySink] Virtual Serial Port ready!
          Connect client/tests to: /dev/pts/X
```
Point any test harness or serial monitor to `/dev/pts/X` to receive real radar packets.

---

## 4. All-in-One tmux Simulation (Sensor + Virtual PTY + Tactical HUD)
Run both the radar sensor simulator and the Tactical Desktop HUD simultaneously in a split `tmux` session with a single command:

```bash
# Launch multi-target simulation (3 crossing targets):
./scripts/sim-tmux.sh

# Or with a patrolling target:
./scripts/sim-tmux.sh --scenario patrol

# Stop the simulation session:
./scripts/sim-tmux.sh --kill
```

- **Pane 0 (Left):** Runs `sim-radar` streaming 30-byte FMCW radar frames into a dynamically allocated virtual PTY.
- **Pane 1 (Right):** Runs `radar-gui` connected to the virtual PTY, rendering the 60 FPS CRT phosphor green HUD.
- **Keybindings:** Press `Ctrl+b` then `d` to detach. Re-attach at any time with `tmux attach -t hrs-sim`.

---

## 5. Wokwi Virtual Microcontroller & Display Emulation
Simulate the full ESP32 microcontroller, FreeRTOS, SPI display, and UART radar sensor:
1. Build firmware:
   ```bash
   ./scripts/build.sh --display ili9341
   ```
2. Open `diagram.json` in VS Code with the Wokwi extension or load into Wokwi web simulator.
3. The custom chip `simulation/wokwi/chip-ld2450.c` will automatically stream radar frames into the emulated ESP32 UART2 registers.
