# Heartbeat Radar Sensor — Developer & Agent Guide

Human presence + movement detection.
ESP32 + HLK-LD2450 24GHz FMCW mmWave radar + ST7789 240x320 SPI TFT.

---

## Architecture: Hexagonal (Ports & Adapters)

- **Strict boundary:** core (`src/core/`) = pure C++17, **ZERO** platform/hardware deps. Forbidden in `src/core/`: `<Arduino.h>`, ESP-IDF, FreeRTOS, driver headers. Physical capabilities = pure value types (`SensorSpecs`).
- **Ports (`src/ports/`):** abstract interfaces, pure virtual methods:
  - `ISensor`: `scan()` → `std::vector<TargetData>`, `specs()`, health `is_connected()`.
  - `IDisplay`: HUD render, radar sweep, blip plot, alert borders.
  - `ITimer`: timing + delays (synthetic time acceleration in unit tests).
  - `IBuzzer`: proximity audio alert contract.
  - `IThreatEvaluator`: target vectors → `ThreatAssessment`.
  - `IImu`: `init()`, `is_connected()`, `read_imu(ImuData&)`.
- **Adapters (`src/adapters/`):** per subsystem (`sensors`, `displays`, `imu`, `timers`, `buzzers`):
  - `LD2450Adapter`: UART2 binary protocol parser (256000 baud), owns `SPECS`.
  - `SerialDisplayAdapter`: serial JSON telemetry for desktop/web HUD.
  - `ST7789Adapter`: SPI display via `TFT_eSPI`.
  - `WifiDisplayAdapter`: same JSON telemetry over TCP server on WiFi boards (`socket://heartbeat-radar.local:3333`). One client at a time; newer replaces older. Many viewers: connect only `web.sh`, open it in many browsers.
  - `MPU6050Adapter`: MPU-6050 IMU over I2C.
  - `ESP32BuzzerAdapter`: PWM audio alert driver.
  - `ESP32TimerAdapter`: hardware timer.
- **Mocks (`test_native/mocks/`):** port test doubles for deterministic host tests.

- **Writing style** (`.claude/rules/writing-style.md`): docs, comments, rules, logs, UI text = short, concrete, technical. No marketing words, game references, emoji.

- **Configuration** (`.claude/rules/adapter-config.md`): board wiring (pins, console baud) only in `platformio.ini` `[env:<board>]`. Adapter tunables = `static constexpr DEFAULT_*`, override via `./scripts/build.sh -f NAME=value`. Secrets from env via `build.sh`. Only `src/main.cpp` reads config macros.

---

## Common Commands

Build, test, QA run in isolated Docker containers:

```bash
# Build firmware binary (orthogonal options: --board, --sensors, --displays)
./scripts/build.sh                             # ld2450 + serial telemetry (default)
./scripts/build.sh --displays st7789           # TFT (renderer still a stub)
WIFI_SSID=… WIFI_PASSWORD=… ./scripts/build.sh --displays wifi   # TCP telemetry (esp32dev; creds from env only)
./scripts/build.sh -f RADAR_DANGER_DISTANCE_M=2.0f   # Override alert threshold
# Planned (rejected until RD-03D adapter and CompositeSensor/CompositeDisplay exist):
#   --sensors ld2450,rd03d   --displays st7789,serial

# Run swappable radar sensor simulator (USB-UART, PTY, or socket via uv)
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0
./scripts/sim-radar.sh --sensor rd03d --scenario multi --pty

# Run desktop radar HUD (Tkinter GUI via uv)
./scripts/gui.sh --demo                         # Standalone demo mode
./scripts/gui.sh --port /dev/ttyUSB0 --raw      # Live raw radar feed @ 256k
./scripts/gui.sh --port /dev/ttyUSB0            # ESP32 telemetry feed @ 921k
./scripts/gui.sh --port socket://heartbeat-radar.local:3333   # ESP32 WiFi telemetry (or simulator --socket)

# Run web radar HUD (FastAPI + HTML5 Canvas via uv)
./scripts/web.sh --demo                         # Standalone web demo on http://127.0.0.1:8088
./scripts/web.sh --serial /dev/ttyUSB0 --raw    # Live raw radar feed @ 256k
./scripts/web.sh --serial /dev/ttyUSB0          # ESP32 telemetry feed @ 921k
./scripts/web.sh --serial socket://heartbeat-radar.local:3333 # ESP32 WiFi telemetry (or simulator --socket)

# Run all-in-one synchronized sensor simulator + HUD (tmux)
./scripts/sim-tmux.sh                           # Simulator + Desktop HUD
./scripts/sim-tmux.sh --web                     # Simulator + Web HUD

# Run native Catch2 unit tests (host-side in Docker)
./scripts/test-native.sh

# All tests: native Catch2 + Python (Python skipped if no `uv`)
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

# Serial monitor (runs locally, 921600 baud)
./scripts/monitor.sh [port]  # Default: /dev/ttyUSB0
```

---

## Code Style & Conventions

- **Language:** C++17 (`-std=gnu++17` on ESP32, `-std=c++17` host tests).
- **Yoda comparisons:** mandatory for all `==` / `!=`:
  ```cpp
  if (nullptr == target) { ... }
  if (0 == count) { ... }
  if (TargetZone::Near == zone) { ... }
  ```
- **Braces:** Allman (opening brace on new line):
  ```cpp
  void RadarEngine::update()
  {
      if (true == is_scanning_)
      {
          // ...
      }
  }
  ```
- **Memory safety & RAII:**
  - Never raw `new` / `delete`.
  - `std::unique_ptr` for exclusive ownership; references for injected deps.
- **Const correctness:**
  - Variables `const` by default.
  - Non-primitive params by `const &`.
  - Query methods `const`.
- **Naming:**
  - Types / classes: `PascalCase` (`RadarEngine`, `TargetData`)
  - Interfaces: `I` prefix + `PascalCase` (`ISensor`, `IDisplay`)
  - Functions / methods: `snake_case()` (`scan()`, `active_target_count()`)
  - Variables / params: `snake_case`
  - Private members: trailing underscore `snake_case_` (`sweep_angle_`, `sensor_`)
  - Constants (`constexpr`, global/static): `UPPER_SNAKE_CASE` (`MAX_TARGETS`, `BAUD_RATE`)
  - Enum values: `PascalCase` (`ThreatLevel::Danger`)
  - `const` locals + params: plain `snake_case`

---

## TDD Workflow (Red → Green → Refactor)

1. **Red:** failing test in `test_native/` first. Verify failure: `./scripts/test-native.sh`.
2. **Green:** minimal code in `src/core/` to pass.
3. **Refactor:** clean up; check Yoda, const-correctness, SOLID. All tests stay green.

---

## Commit Conventions

Every commit: this format + committer sign-off:

```
[Type] Short summary description (max 72 chars)

Detailed explanation of changes and motivation.
Wrapped at 72 characters per line.

Signed-off-by: JaWitold <witekkaras00@gmail.com>
```

**Allowed types:**
- `[Feature]` — new functionality
- `[Fix]` — bug fix
- `[Docs]` — documentation
- `[Style]` — formatting / whitespace, no logic change
- `[Refactor]` — restructure, no behavior change
- `[Test]` — test suite
- `[Chore]` — build system, CI, Docker, deps
- `[Perf]` — performance

---

## Sensor Roadmap

- **v1.0 (primary):** HLK-LD2450 (24GHz FMCW mmWave, 3 simultaneous targets, X/Y + speed via UART).
- **Future:**
  - RCWL-0516 multi-sensor triangulation array (`RCWL0516TriangulationAdapter`).
  - Ai-Thinker RD-03D wide-angle mmWave (`RD03DAdapter`, 120° FoV).
