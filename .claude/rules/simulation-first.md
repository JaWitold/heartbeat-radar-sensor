# Rule: Simulation-First Validation

1. **Pre-Hardware Verification:**
   - Before purchasing or testing on physical hardware, all new sensors, displays, and communication protocols must be simulated and verified first.
   - Every candidate sensor must have:
     1. A mock adapter implementing `ISensor` in `test_native/mocks/`.
     2. A software protocol encoder in `scripts/tools/sensor_sim/protocols/` with exact datasheet wire-format tests (verifying sign-magnitude vs two's complement).
     3. An automated test suite verifying frame parsing, coordinate scaling, and edge-case handling (including binary payload bytes matching `\n` `0x0A`).

2. **Bench Testing via Hardware-in-the-Loop:**
   - Physical boards (ESP32) must be tested using the USB-UART / PTY simulator tool (`scripts/sim-radar.sh`) before connecting live radar modules.
   - This ensures deterministic scenarios (e.g. patrol, multi-target, danger-zone crossing, boundary tracking) can be validated reliably without physical movement in a room.

3. **Display & HUD Simulation:**
   - Visual layouts (radar sweeps, blips, HUD, alert banners) must be previewed and verified using:
     - Tactical Desktop HUD (`./scripts/gui.sh --demo` or `--raw` / `--port`)
     - Tactical Web HUD (`./scripts/web.sh --demo` or `--port`)
     - All-in-one tmux runner (`./scripts/sim-tmux.sh`)
     - Wokwi virtual display simulation before flashing to physical TFT screens.
