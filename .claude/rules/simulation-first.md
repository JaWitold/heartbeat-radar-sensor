# Rule: Simulation-First Validation

1. **Pre-Hardware Verification:**
   - Before purchasing or testing on physical hardware, all new sensors, displays, and communication protocols must be simulated and verified first.
   - Every candidate sensor must have:
     1. A mock adapter implementing `ISensor` in `test_native/mocks/`.
     2. A software protocol encoder in `scripts/tools/sensor_sim/protocols/`.
     3. An automated test suite verifying frame parsing, coordinate scaling, and edge-case handling against synthetic data.

2. **Bench Testing via Hardware-in-the-Loop:**
   - Physical boards (ESP32) must be tested using the USB-UART / PTY simulator tool (`scripts/sim-radar.sh`) before connecting live radar modules.
   - This ensures deterministic scenarios (e.g. high-speed approach, danger-zone crossing, boundary tracking) can be validated reliably without relying on physical movement in a room.

3. **Display Simulation:**
   - Visual layouts (radar sweeps, blips, HUD, alert banners) must be previewed and verified using either Wokwi virtual displays or native desktop test doubles before flashing to real TFT screens.
