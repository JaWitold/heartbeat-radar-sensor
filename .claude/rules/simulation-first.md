# Rule: Simulation-First Validation

1. **Pre-hardware verification:**
   - Simulate + verify every new sensor, display, protocol before buying or testing on hardware.
   - Every candidate sensor needs:
     1. Mock adapter implementing `ISensor` in `test_native/mocks/`.
     2. Protocol encoder in `scripts/tools/sensor_sim/protocols/` with exact datasheet wire-format tests (sign-magnitude vs two's complement).
     3. Automated tests for frame parsing, coordinate scaling, edge cases (incl. binary payload bytes equal to `\n` `0x0A`).

2. **Bench testing via hardware-in-the-loop:**
   - Test ESP32 boards with USB-UART / PTY simulator (`scripts/sim-radar.sh`) before connecting live radar.
   - Gives deterministic scenarios (patrol, multi-target, danger-zone crossing, boundary tracking) without physical movement.

3. **Display & HUD simulation:**
   - Preview + verify visual layouts (radar sweep, blips, HUD, alert banners) with:
     - Desktop HUD (`./scripts/gui.sh --demo` or `--raw` / `--port`)
     - Web HUD (`./scripts/web.sh --demo` or `--port`)
     - All-in-one tmux runner (`./scripts/sim-tmux.sh`)
     - Wokwi virtual display simulation before flashing physical TFT.
