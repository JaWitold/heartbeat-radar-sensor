# Rule: Orthogonal Target Architecture

1. **Strict orthogonality:**
   - Boards, sensors, displays = independent orthogonal axes.
   - Never couple sensor adapter to specific board or display driver.
   - Never couple display adapter to specific sensor or MCU pin layout.

2. **Axis separation:**
   - **Board axis:** MCU platform, toolchain, system clock, pin mappings (`esp32dev`, `native`). All wiring macros in that board's `platformio.ini` env; see `adapter-config.md`.
   - **Sensor axis:** implements pure virtual `ISensor`. Any sensor adapter (`LD2450Adapter`, `RD03DAdapter`, `RCWL0516TriangulationAdapter`, `MockSensor`) swappable via DI.
   - **Display axis:** implements pure virtual `IDisplay`. Any display adapter (`SerialDisplayAdapter`, `WifiDisplayAdapter`, `ST7789Adapter`, `ILI9341Adapter`, `MockDisplay`) swappable via DI.

3. **Factory & composition root:**
   - Adapters instantiated only at composition root (`main.cpp` or test fixtures), via orthogonal factory functions or direct DI.
   - Core (`src/core/`) never references specific board, sensor, or display header.
   - Composition root binds `SelectedSensor::SPECS` with compile-time threshold overrides (`-DRADAR_DANGER_DISTANCE_M=...`, `-DRADAR_CAUTION_DISTANCE_M=...`).
   - `main.cpp` = only reader of config macros (board wiring + `-f` overrides). Adapters get values via constructors. Full contract: `adapter-config.md`.

4. **No combinatorial explosion (cardinality-N composability):**
   - No static build env per combination ($N \times M \times K$).
   - Sensor + display axes multi-selectable:
     - Multiple sensors at once (e.g. `--sensors ld2450,rd03d`) via `CompositeSensor` fusion adapter.
     - Multiple display/output sinks at once (e.g. `--displays st7789,serial,ble`) via `CompositeDisplay` broadcaster.
   - Orthogonal feature flags (`-DENABLE_SENSOR_*`, `-DENABLE_DISPLAY_*`) + CLI params (`./scripts/build.sh --sensors <list> --displays <list> -f <flag>`).
   - **Validation guard:** `scripts/build.sh` and `main.cpp` reject sensor/display options without implemented adapter (e.g. `rd03d`, multi-sensor fusion until adapters written).
