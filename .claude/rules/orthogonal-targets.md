# Rule: Orthogonal Target Architecture

1. **Strict Orthogonality:**
   - Boards, Sensors, and Displays must be treated as independent, orthogonal axes.
   - Never couple a sensor adapter to a specific board or display driver.
   - Never couple a display adapter to a specific sensor or microcontroller pin layout.

2. **Axis Separation:**
   - **Board Axis:** Defines the physical microcontroller platform, platform toolchain, system clock, and pin mappings (`esp32dev`, `native`). All wiring macros live in that board's `platformio.ini` env; see `adapter-config.md`.
   - **Sensor Axis:** Implements the pure virtual interface `ISensor`. Any sensor adapter (`LD2450Adapter`, `RD03DAdapter`, `RCWL0516TriangulationAdapter`, `MockSensor`) must be swappable via dependency injection.
   - **Display Axis:** Implements the pure virtual interface `IDisplay`. Any display adapter (`SerialDisplayAdapter`, `WifiDisplayAdapter`, `ST7789Adapter`, `ILI9341Adapter`, `MockDisplay`) must be swappable via dependency injection.

3. **Factory & Composition Root:**
   - Adapters are instantiated only at the composition root (`main.cpp` or test fixtures) through orthogonal factory functions or direct dependency injection.
   - Core domain logic (`src/core/`) must never reference any specific board, sensor, or display header.
   - The composition root binds `SelectedSensor::SPECS` with compile-time threshold overrides (`-DRADAR_DANGER_DISTANCE_M=...`, `-DRADAR_CAUTION_DISTANCE_M=...`).
   - `main.cpp` is the only reader of config macros (board wiring + `-f` overrides); adapters receive values via constructors. Full contract: `adapter-config.md`.

4. **Avoiding Combinatorial Explosion with Cardinality-N Composability:**
   - Do NOT create static build environments for every combination ($N \times M \times K$).
   - Sensor and Display axes are multi-selectable:
     - Multiple sensors can be active simultaneously (e.g. `--sensors ld2450,rd03d`) using a `CompositeSensor` fusion adapter.
     - Multiple display/output sinks can be active simultaneously (e.g. `--displays st7789,serial,ble`) using a `CompositeDisplay` broadcaster.
   - Use orthogonal feature flags (`-DENABLE_SENSOR_*`, `-DENABLE_DISPLAY_*`) and CLI parameters (`./scripts/build.sh --sensors <list> --displays <list> -f <flag>`).
   - **Validation Guard:** `scripts/build.sh` and `main.cpp` explicitly reject sensor/display options that have no adapter implemented yet (e.g., rejecting `rd03d` or multi-sensor fusion until their adapters are written).
