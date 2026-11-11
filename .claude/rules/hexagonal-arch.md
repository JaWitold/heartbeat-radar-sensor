# Rule: Hexagonal Architecture Boundaries

1. **Core isolation (`src/core/`):**
   - Pure C++17 logic only.
   - Forbidden: `#include <Arduino.h>`, FreeRTOS, ESP-IDF headers, peripheral registers.
   - Core talks only via port interfaces in `src/ports/`.
   - Domain models pure. Physical capabilities = value types (e.g. `SensorSpecs` in `src/core/sensor_specs.h`). Target evaluations (e.g. `TargetData::zone(danger_m, caution_m)`) = pure calculations taking thresholds, no hardcoded global static constants.
2. **Ports (`src/ports/`):**
   - Abstract base classes, pure virtual methods (`= 0`), virtual destructor (`virtual ~IPort() = default;`).
   - Contracts for external deps: `ISensor`, `IDisplay`, `ITimer`, `IBuzzer`, `IThreatEvaluator`, `IImu`.
   - Sensors self-describe via `virtual SensorSpecs specs() const = 0;`, report live health via `virtual bool is_connected() const = 0;`.
3. **Adapters (`src/adapters/`):**
   - Concrete impls by subsystem namespace: `hrs::adapters::sensors`, `hrs::adapters::displays`, `hrs::adapters::timers`, `hrs::adapters::buzzers`, `hrs::adapters::imu`.
   - Hardware sensor adapters own physical specs constant (e.g. `LD2450Adapter::SPECS`), return it via `specs()`.
   - Adapters bridge hardware SDKs / third-party libs (e.g. `TFT_eSPI`, ESP32 `HardwareSerial`, `tone()`) to ports.
   - Adapters may include vendor headers; those never leak into ports or core.
   - Adapters never read config macros. Expose `static constexpr DEFAULT_*` tunables, take every value (incl. pins) via constructor. See `adapter-config.md`.
4. **Dependency inversion & composition root:**
   - Core never depends on adapters. Both depend on ports.
   - Wire deps in composition root (`main.cpp`) or unit test setup.
   - Composition root reads active sensor adapter `SPECS`, applies optional compile-time overrides (`RADAR_DANGER_DISTANCE_M`, etc.), passes resolved thresholds to display + threat evaluator.
   - `RadarEngine` auto-configures sweep limits + tracking scale from `sensor_.specs()`. No manual env var plumbing.
5. **Composite ports for multi-module composition (cardinality-N):**
   - Never assume ports = 1-to-1 singletons.
   - **Multi-sensor fusion:** multiple sensors (e.g. dual LD2450, LD2450 + RD-03D, RCWL-0516 array) → `CompositeSensor` implementing `ISensor`. Polls all children, merges/triangulates coordinates, reports synthesized `specs()` (fused FOV union, max range, sum of target capacity).
   - **Multi-display broadcast:** multiple outputs (e.g. TFT + serial telemetry to PC + BLE broadcast, or dual TFT) → `CompositeDisplay` implementing `IDisplay`. Fans out draw calls to all sinks.
   - Until `CompositeSensor` + `CompositeDisplay` exist, `main.cpp` and `scripts/build.sh` reject multi-sensor/multi-display combos with explicit error. Prevents silent miscompilation.
