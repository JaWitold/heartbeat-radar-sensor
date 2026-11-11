---
name: add-feature
description: TDD-driven new feature for Heartbeat Radar Sensor (core logic, sensor or display adapter). Enforces hexagonal architecture, adapter config buckets, coding standards.
---

# Skill: Add Feature

## Purpose
Standard flow for new capability or sensor/display adapter.

## Procedure

### 1. Pick Layer and Interfaces
- **Domain logic** (contact persistence, sweep math, threat assessment) → `src/core/`. Keep models pure; use `SensorSpecs` from `src/core/sensor_specs.h`.
- **Sensor adapter:**
  1. Check contract in `src/ports/i_sensor.h` (`scan()`, `specs()`, `is_connected()`).
  2. Add/update mock `test_native/mocks/mock_sensor.h` with `set_specs()`, `set_connected()`.
  3. Check datasheet wire format: signed fields sign-magnitude or two's complement.
  4. Encoder in `scripts/tools/sensor_sim/protocols/`, decoder in `scripts/tools/radar_gui/protocol_decoders.py`.
  5. Adapter in `src/adapters/sensors/` with `static constexpr core::SensorSpecs SPECS{...}`.
  6. Wire config per `.claude/rules/adapter-config.md` (see 1b).
  7. Register in `scripts/build.sh` and `src/main.cpp`.
- **Display adapter:**
  1. Implement `src/ports/i_display.h`.
  2. Adapter in `src/adapters/displays/`.
  3. Wire config per `.claude/rules/adapter-config.md` (see 1b).
  4. Register in `scripts/build.sh` and `src/main.cpp`.

### 1b. Adapter Configuration (macros & definitions)
Each value → one bucket. Wire exactly:

| Value | Put it in | Wiring |
|---|---|---|
| Pin / bus / console baud | `platformio.ini` `[env:<board>]` as `-D<ADAPTER>_<PARAM>=…` | `main.cpp` passes the macro to the ctor, **no fallback** |
| Tunable (rate, port, hostname, threshold) | adapter `static constexpr DEFAULT_<PARAM>` | `main.cpp`: `#ifndef <ADAPTER>_<PARAM>` → `#define … Adapter::DEFAULT_<PARAM>`, overridable with `./scripts/build.sh -f <ADAPTER>_<PARAM>=…` |
| Secret | env var validated in `scripts/build.sh`, never echoed | `build.sh` appends quoted `-D` |
| Vendor library setting | board env under the library's own names (`TFT_*`) | library reads it |

Checklist:
- [ ] No `#ifdef`/`#ifndef` on config names in `src/adapters/` or `src/core/` (only `#ifdef ARDUINO`).
- [ ] Ctor takes every value; pins: no default arg.
- [ ] Each value defined once (`rg <NAME>` → one definition + `main.cpp` read).
- [ ] Unsupported combos, missing secrets rejected in `scripts/build.sh`, not `#error` in `main.cpp`.
- [ ] User-facing overrides listed in `./scripts/build.sh --help`.

### 2. Red (Failing Test)
Tests in `test_native/core/` or `test_native/adapters/`:
- Catch2 BDD: `SCENARIO`, `GIVEN`, `WHEN`, `THEN`.
- Yoda comparisons (`REQUIRE(0 == target_count)`).
- Exact datasheet byte vectors for binary frames.
- Run:
  ```bash
  ./scripts/test-native.sh
  ```
- Confirm fail.

### 3. Green (Minimal Impl)
- Minimal code in `src/core/` or `src/adapters/`.
- Run `./scripts/test-native.sh` until green.

### 4. Refactor & Verify
- Check const-correctness, Yoda, memory safety.
- Format, lint, test:
  ```bash
  ./scripts/format.sh
  ./scripts/format-check.sh
  ./scripts/lint.sh
  ./scripts/test.sh
  ```

### 5. Firmware Build
- Must compile clean in Docker:
  ```bash
  ./scripts/build.sh
  ```

### 6. Commit
- Format:
  ```
  [Type] Short summary (max 72 chars)

  Detailed explanation wrapped at 72 chars.

  Signed-off-by: JaWitold <witekkaras00@gmail.com>
  ```
