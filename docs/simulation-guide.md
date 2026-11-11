# Simulation Guide: Test Before Buying Hardware

Test every component without physical hardware.

---

## 1. Native Unit Tests (Catch2 in Docker)
Logic, coordinate math, state transitions; milliseconds:
```bash
./scripts/test-native.sh
```
Core verified with mocks (`MockSensor`, `MockDisplay`, ...) from `test_native/mocks/`.

---

## 2. USB-UART Hardware-in-the-Loop
Have ESP32 + display, no radar (or need reproducible target scenarios):

### Wiring
USB-UART adapter (**PL2303HX**, **CP2102**, **FT232**, **CH340**) → ESP32:
- Adapter `TX` $\to$ ESP32 `GPIO 16 (RX2)`
- Adapter `GND` $\to$ ESP32 `GND`
- **Jumper:** **3.3V logic**. Never 5V logic on ESP32 GPIOs.

### Run
```bash
# Simulate HLK-LD2450 patrolling contact (1m to 5m)
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0

# Simulate 3 crossing targets
./scripts/sim-radar.sh --sensor ld2450 --scenario multi --port /dev/ttyUSB0

# Interactive keyboard control
./scripts/sim-radar.sh --sensor ld2450 --scenario interactive --port /dev/ttyUSB0
```

---

## 3. Virtual Serial PTY (Linux, no hardware)
Test parsers and desktop apps without USB dongle:
```bash
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --pty
```
Output:
```
[PtySink] Virtual Serial Port ready!
          Connect client/tests to: /dev/pts/X
```
Point test harness or serial monitor at `/dev/pts/X` → real LD2450 frames.

---

## 4. tmux: Simulator + PTY + HUD
Simulator and desktop HUD in split `tmux` session, one command:

```bash
# Launch multi-target simulation (3 crossing targets):
./scripts/sim-tmux.sh

# Or with a patrolling target:
./scripts/sim-tmux.sh --scenario patrol

# Web HUD instead of desktop GUI:
./scripts/sim-tmux.sh --web

# Stop the simulation session:
./scripts/sim-tmux.sh --kill
```

- **Pane 0 (left):** `sim-radar`, 30-byte LD2450 frames into allocated virtual PTY.
- **Pane 1 (right):** `radar-gui` on that PTY, 60 FPS HUD.
- **Keys:** `Ctrl+b` then `d` → detach. Re-attach: `tmux attach -t hrs-sim`.

---

## 5. Wokwi (ESP32 + Display Emulation)
Emulates ESP32, FreeRTOS, SPI display, UART radar:
1. Build firmware (`wokwi.toml` loads `.pio/build/esp32dev/firmware.bin`):
   ```bash
   ./scripts/build.sh
   ```
2. Open `diagram.json` in VS Code Wokwi extension or Wokwi web simulator.
3. Custom chip `simulation/wokwi/chip-ld2450.c` streams LD2450 frames into ESP32 UART2.
