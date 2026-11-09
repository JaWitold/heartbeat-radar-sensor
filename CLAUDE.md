# Heartbeat Radar Sensor — Developer & Agent Guide

Tactical heartbeat radar sensor inspired by Call of Duty: Modern Warfare 2 and Rainbow Six Siege.
Built for ESP32 with the HLK-LD2450 24GHz FMCW mmWave radar and ST7789 240x320 SPI TFT display.

---

## 🏛️ Architecture: Hexagonal (Ports & Adapters)

- **Strict Boundary:** The Core Domain (`src/core/`) must remain pure C++17 with **ZERO** platform or hardware dependencies (`<Arduino.h>`, ESP-IDF, FreeRTOS, or driver headers are forbidden in `src/core/`). Physical capabilities are described via pure value types (`SensorSpecs`).
- **Ports (`src/ports/`):** Abstract interfaces with pure virtual methods:
  - `ISensor`: Scans and returns `std::vector<TargetData>`, reports `specs()`, and health `is_connected()`.
  - `IDisplay`: HUD rendering, radar sweep drawing, blip plotting, alert borders.
  - `ITimer`: System timing and delays (allows synthetic time acceleration in unit tests).
  - `IBuzzer`: Proximity audio alert contract.
  - `IThreatEvaluator`: Evaluates target vectors into `ThreatAssessment`.
- **Adapters (`src/adapters/`):** Subsystem implementations (`sensors`, `displays`, `timers`, `buzzers`):
  - `LD2450Adapter`: High-speed UART2 binary protocol parser (256000 baud) owning its `SPECS`.
  - `SerialDisplayAdapter`: Serial JSON telemetry output for desktop/web HUD.
  - `ST7789Adapter`: SPI display rendering via `TFT_eSPI`.
  - `ESP32BuzzerAdapter`: PWM audio alert driver.
  - `ESP32TimerAdapter`: Hardware timer implementation.
- **Mocks (`test_native/mocks/`):** Test doubles implementing ports for deterministic host-side testing.

---

## 🛠️ Common Commands

All build, test, and QA operations run inside isolated Docker containers:

```bash
# Build firmware binary (orthogonal options: --board, --sensors, --displays)
./scripts/build.sh                             # ld2450 + serial telemetry (default)
./scripts/build.sh --displays st7789           # TFT (renderer still a stub)
./scripts/build.sh -f RADAR_DANGER_DISTANCE_M=2.0f   # Override alert threshold
# Planned (rejected until RD-03D adapter and CompositeSensor/CompositeDisplay exist):
#   --sensors ld2450,rd03d   --displays st7789,serial

# Run swappable radar sensor simulator (USB-UART, PTY, or socket via uv)
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0
./scripts/sim-radar.sh --sensor rd03d --scenario multi --pty

# Run tactical desktop radar HUD (Tkinter GUI via uv)
./scripts/gui.sh --demo                         # Standalone demo mode
./scripts/gui.sh --port /dev/ttyUSB0 --raw      # Live raw radar feed @ 256k
./scripts/gui.sh --port /dev/ttyUSB0            # ESP32 telemetry feed @ 115k

# Run tactical web radar HUD (FastAPI + HTML5 Canvas via uv)
./scripts/web.sh --demo                         # Standalone web demo on http://127.0.0.1:8899
./scripts/web.sh --port /dev/ttyUSB0 --raw      # Live raw radar feed @ 256k
./scripts/web.sh --port /dev/ttyUSB0            # ESP32 telemetry feed @ 115k

# Run all-in-one synchronized sensor simulator + HUD (tmux)
./scripts/sim-tmux.sh                           # Simulator + Desktop HUD
./scripts/sim-tmux.sh --web                     # Simulator + Web HUD

# Run native Catch2 unit tests (host-side in Docker)
./scripts/test-native.sh

# Run all tests (native + embedded)
./scripts/test.sh

# Code quality & static analysis (clang-tidy + cppcheck)
./scripts/lint.sh

# Verify formatting without modifying files (CI mode)
./scripts/format-check.sh

# Auto-format all C++ files using clang-format
./scripts/format.sh

# Generate Doxygen documentation
./scripts/docs.sh

# Upload firmware to connected ESP32 (runs locally, requires USB)
./scripts/upload.sh [port]   # Default: /dev/ttyUSB0

# Serial monitor (runs locally, 115200 baud)
./scripts/monitor.sh [port]  # Default: /dev/ttyUSB0
```

---

## 📏 Code Style & Conventions

- **Language:** C++17 (`-std=gnu++17` on ESP32, `-std=c++17` for host tests).
- **Yoda Comparisons:** Mandatory for all equality and inequality checks:
  ```cpp
  if (nullptr == target) { ... }
  if (0 == count) { ... }
  if (TargetZone::Near == zone) { ... }
  ```
- **Braces:** Allman style (opening brace on a new line):
  ```cpp
  void RadarEngine::update()
  {
      if (true == is_scanning_)
      {
          // ...
      }
  }
  ```
- **Memory Safety & RAII:**
  - Never use raw `new` or `delete`.
  - Use `std::unique_ptr` for exclusive ownership, references for injected dependencies.
- **Const Correctness:**
  - Mark variables `const` by default.
  - Pass non-primitive types by `const &`.
  - Mark query methods `const`.
- **Naming Conventions:**
  - Types / Classes: `PascalCase` (`RadarEngine`, `TargetData`)
  - Interfaces: `I` prefix + `PascalCase` (`ISensor`, `IDisplay`)
  - Functions / Methods: `snake_case()` (`scan()`, `active_target_count()`)
  - Variables / Parameters: `snake_case`
  - Private Member Variables: trailing underscore `snake_case_` (`sweep_angle_`, `sensor_`)
  - Constants (`constexpr`, global/static): `UPPER_SNAKE_CASE` (`MAX_TARGETS`, `BAUD_RATE`)
  - Enum values: `PascalCase` (`ThreatLevel::Danger`)
  - `const` locals and parameters: plain `snake_case`

---

## 🧪 TDD Workflow (Red → Green → Refactor)

1. **🔴 Red:** Write a failing test in `test_native/` first. Verify failure with `./scripts/test-native.sh`.
2. **🟢 Green:** Implement the minimal code in `src/core/` to make the test pass.
3. **🔵 Refactor:** Clean up code, verify Yoda style, const-correctness, and SOLID principles. Ensure all tests remain green.

---

## 📝 Commit Conventions

Every commit must follow this format with a committer sign-off:

```
[Type] Short summary description (max 72 chars)

Detailed explanation of changes and motivation.
Wrapped at 72 characters per line.

Signed-off-by: JaWitold <witekkaras00@gmail.com>
```

**Allowed Types:**
- `[Feature]` — New functionality
- `[Fix]` — Bug fix
- `[Docs]` — Documentation updates
- `[Style]` — Formatting / whitespace (no logic change)
- `[Refactor]` — Restructuring without behavior change
- `[Test]` — Test suite changes
- `[Chore]` — Build system, CI, Docker, or dependency maintenance
- `[Perf]` — Performance optimization

---

## 📡 Sensor Roadmap

- **v1.0 (Primary):** HLK-LD2450 (24GHz FMCW mmWave, 3 simultaneous targets with X/Y coordinates and speed via UART).
- **Future:**
  - RCWL-0516 multi-sensor triangulation array (`RCWL0516TriangulationAdapter`).
  - Ai-Thinker RD-03D wide-angle mmWave adapter (`RD03DAdapter`, 120° FoV).
