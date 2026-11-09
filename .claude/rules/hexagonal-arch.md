# Rule: Hexagonal Architecture Boundaries

1. **Core Domain Isolation (`src/core/`):**
   - Must contain pure C++17 logic only.
   - Strictly forbidden: `#include <Arduino.h>`, FreeRTOS, ESP-IDF headers, hardware peripheral registers.
   - Core communicates solely through port interfaces in `src/ports/`.
2. **Ports (`src/ports/`):**
   - Must be abstract base classes (interfaces) with pure virtual methods (`= 0`) and virtual destructors (`virtual ~IPort() = default;`).
   - Define contracts for external dependencies: `ISensor`, `IDisplay`, `ITimer`, `IBuzzer`.
3. **Adapters (`src/adapters/`):**
   - Concrete implementations that bridge hardware SDKs or third-party libraries (e.g. `TFT_eSPI`, ESP32 HardwareSerial) to port interfaces.
   - Adapters may include vendor headers, but those headers must never leak into port interfaces or core domain.
4. **Dependency Inversion:**
   - High-level modules (Core) must not depend on low-level modules (Adapters). Both must depend on abstractions (Ports).
   - Wire dependencies in the composition root (`main.cpp`) or in unit test setups.
5. **Composite Ports for Multi-Module Composition (Cardinality-N):**
   - Never assume ports are 1-to-1 singletons.
   - **Multi-Sensor Fusion:** When multiple sensors are configured (e.g. dual LD2450s, LD2450 + RD-03D, or RCWL-0516 array), use a `CompositeSensor` implementing `ISensor`. It polls all child sensors and merges/triangulates coordinates before passing them to the core domain.
   - **Multi-Display Broadcasting:** When multiple outputs are active (e.g. physical TFT + Serial Telemetry to PC + BLE broadcast, or dual TFT displays), use a `CompositeDisplay` implementing `IDisplay`. It fans out draw calls to all registered sinks.
   - The Core Domain remains completely unchanged and oblivious to whether 1 or 5 sensors/displays are active.
