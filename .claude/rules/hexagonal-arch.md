# Rule: Hexagonal Architecture Boundaries

1. **Core Domain Isolation (`src/core/`):**
   - Must contain pure C++17 logic only.
   - Strictly forbidden: `#include <Arduino.h>`, FreeRTOS, ESP-IDF headers, hardware peripheral registers.
   - Core communicates solely through port interfaces in `src/ports/`.
   - Keep domain models pure: physical capabilities are described via value types (e.g., `SensorSpecs` in `src/core/sensor_specs.h`). Target evaluations (such as `TargetData::zone(danger_m, caution_m)`) must be pure calculations that receive thresholds rather than hardcoding global static constants.
2. **Ports (`src/ports/`):**
   - Must be abstract base classes (interfaces) with pure virtual methods (`= 0`) and virtual destructors (`virtual ~IPort() = default;`).
   - Define contracts for external dependencies: `ISensor`, `IDisplay`, `ITimer`, `IBuzzer`, `IThreatEvaluator`.
   - Sensors must self-describe their capabilities via `virtual SensorSpecs specs() const = 0;` and report live connection health via `virtual bool is_connected() const = 0;`.
3. **Adapters (`src/adapters/`):**
   - Concrete implementations organized by subsystem namespace: `hrs::adapters::sensors`, `hrs::adapters::displays`, `hrs::adapters::timers`, `hrs::adapters::buzzers`.
   - Hardware sensor adapters own their own physical specifications constant (e.g. `LD2450Adapter::SPECS`) and return it via `specs()`.
   - Adapters bridge hardware SDKs or third-party libraries (e.g. `TFT_eSPI`, ESP32 `HardwareSerial`, `tone()`) to port interfaces.
   - Adapters may include vendor headers, but those headers must never leak into port interfaces or core domain.
   - Adapters never read config macros; they expose `static constexpr DEFAULT_*` tunables and take every value (incl. pins) through the constructor. See `adapter-config.md`.
4. **Dependency Inversion & Composition Root:**
   - High-level modules (Core) must not depend on low-level modules (Adapters). Both depend on abstractions (Ports).
   - Wire dependencies in the composition root (`main.cpp`) or in unit test setups.
   - The composition root inspects the active sensor adapter's `SPECS`, applies optional compile-time overrides (`RADAR_DANGER_DISTANCE_M`, etc.), and passes the resolved thresholds to the display and threat evaluator.
   - `RadarEngine` auto-configures its sweep limits and tracking scale from `sensor_.specs()`, eliminating manual environment variable plumbing.
5. **Composite Ports for Multi-Module Composition (Cardinality-N):**
   - Never assume ports are 1-to-1 singletons.
   - **Multi-Sensor Fusion:** When multiple sensors are configured (e.g. dual LD2450s, LD2450 + RD-03D, or RCWL-0516 array), use a `CompositeSensor` implementing `ISensor`. It polls all child sensors, merges/triangulates coordinates, and reports synthesized `specs()` (fused FOV union, max range, sum of target capacity).
   - **Multi-Display Broadcasting:** When multiple outputs are active (e.g. physical TFT + Serial Telemetry to PC + BLE broadcast, or dual TFT displays), use a `CompositeDisplay` implementing `IDisplay`. It fans out draw calls to all registered sinks.
   - Until `CompositeSensor` and `CompositeDisplay` are implemented, `main.cpp` and `scripts/build.sh` reject multi-sensor/multi-display combinations with an explicit error to prevent silent miscompilation.
