---
name: add-feature
description: Guide TDD-driven implementation of a new feature for the Heartbeat Radar Sensor while adhering to hexagonal architecture and coding standards.
---

# Skill: Add Feature

## Purpose
Enforces the professional development standard when implementing a new capability or sensor/display adapter.

## Step-by-Step Procedure

### 1. Identify Layer and Interfaces
- **Domain Logic:** (e.g. contact persistence, sweep calculations, threat assessment) -> add to `src/core/`. Keep domain models pure (using `SensorSpecs` from `src/core/sensor_specs.h`).
- **External Hardware (Sensors):**
  1. Verify the abstract contract in `src/ports/i_sensor.h` (`scan()`, `specs()`, `is_connected()`).
  2. Implement/update mock in `test_native/mocks/mock_sensor.h` with `set_specs()` and `set_connected()`.
  3. Verify the wire-format representation in the sensor datasheet (check whether signed numbers are sign-magnitude or two's complement).
  4. Implement software encoder in `scripts/tools/sensor_sim/protocols/` and decoder in `scripts/tools/radar_gui/protocol_decoders.py`.
  5. Implement concrete adapter in `src/adapters/sensors/` with `static constexpr core::SensorSpecs SPECS{...}`.
  6. Register sensor in `scripts/build.sh` and `src/main.cpp`.
- **External Hardware (Displays):**
  1. Implement contract in `src/ports/i_display.h`.
  2. Implement concrete adapter in `src/adapters/displays/`.
  3. Register display in `scripts/build.sh` and `src/main.cpp`.

### 2. Red Phase (Write Failing Test)
Create or update tests in `test_native/core/` or `test_native/adapters/`:
- Use Catch2 BDD syntax (`SCENARIO`, `GIVEN`, `WHEN`, `THEN`).
- Ensure comparisons follow Yoda style (`REQUIRE(0 == target_count)`).
- Include exact datasheet byte vectors for binary protocol frames.
- Run:
  ```bash
  ./scripts/test-native.sh
  ```
- Confirm the test fails.

### 3. Green Phase (Pass Minimal Implementation)
- Write minimal code in `src/core/` or `src/adapters/` to pass tests.
- Run `./scripts/test-native.sh` until all tests pass.

### 4. Refactor & Verification Phase
- Check const-correctness, Yoda comparisons, and memory safety.
- Run format and lint checks:
  ```bash
  ./scripts/format.sh
  ./scripts/format-check.sh
  ./scripts/lint.sh
  ./scripts/test.sh
  ```

### 5. Firmware Build Verification
- Verify that firmware compiles cleanly inside Docker:
  ```bash
  ./scripts/build.sh
  ```

### 6. Commit
- Commit with conventional format:
  ```
  [Type] Short summary (max 72 chars)

  Detailed explanation wrapped at 72 chars.

  Signed-off-by: JaWitold <witekkaras00@gmail.com>
  ```
