# Rule: Adapter Configuration (Macros & Definitions)

Every adapter value lands in exactly one of four buckets. No fifth path.

| Bucket | Lives in | Macro? | Example |
|---|---|---|---|
| **Selection** | `scripts/build.sh` → `-DENABLE_SENSOR_*` / `-DENABLE_DISPLAY_*` | yes, generated | `--displays wifi` → `ENABLE_DISPLAY_WIFI=1` |
| **Board wiring** (pins, buses, console baud) | `platformio.ini` `[env:<board>]` only | yes, required, no fallback | `-DLD2450_RX_PIN=16`, `-DBUZZER_PIN=25`, `-DSERIAL_BAUD_RATE=${this.monitor_speed}` |
| **Adapter tunable** (protocol rate, port, hostname, threshold) | adapter `static constexpr DEFAULT_*` (or `SPECS`) | optional override via `./scripts/build.sh -f NAME=value` | `LD2450Adapter::DEFAULT_BAUD_RATE`, `WifiDisplayAdapter::DEFAULT_TCP_PORT` |
| **Secrets** | env vars read by `scripts/build.sh`, never printed, never in git | yes, generated | `WIFI_SSID`, `WIFI_PASSWORD` |

Third-party library macros (TFT_eSPI `TFT_*`, `LOAD_*`, `SPI_FREQUENCY`) = board wiring owned by library. Keep in board env under library names.

## Rules

1. **Adapters never read config macros.** No `#ifdef`/`#ifndef` on config names in `src/adapters/` or `src/core/` (only `#ifdef ARDUINO` for host stubs). Values arrive via constructor.
2. **`src/main.cpp` = only macro reader.** One block per adapter, inside its `ENABLE_*` guard:
   ```cpp
   #if defined(ENABLE_DISPLAY_WIFI)
   #include "adapters/displays/wifi_display_adapter.h"
   #ifndef WIFI_TCP_PORT
   #define WIFI_TCP_PORT hrs::adapters::displays::WifiDisplayAdapter::DEFAULT_TCP_PORT
   #endif
   #endif
   ```
   Board-wiring macros get **no** fallback. Missing pin must fail build, not silently pick GPIO 16.
3. **One source per value.** Never repeat default in both adapter and `platformio.ini`. Derive, don't copy (`SERIAL_BAUD_RATE=${this.monitor_speed}`; `monitor.sh` reads `monitor_speed`).
4. **Naming:** `<ADAPTER>_<PARAM>`, `UPPER_SNAKE_CASE`, matching adapter (`LD2450_RX_PIN`, `LD2450_BAUD_RATE`, `WIFI_TCP_PORT`, `WIFI_HOSTNAME`, `BUZZER_PIN`). Core threshold overrides keep `RADAR_*` (`RADAR_DANGER_DISTANCE_M`).
5. **Guards in `scripts/build.sh`**, not `#error` in `main.cpp` (cppcheck fails when every config permutation hits `#error`). `build.sh` rejects unsupported board/sensor/display combos + missing secrets.
6. **New board:** add `[env:<board>]` with all wiring macros selected adapters need. No code changes.
7. **New adapter:** constructor takes every value. Tunables get `static constexpr DEFAULT_*`; pins get none. Add `main.cpp` block. Document overridable names in `scripts/build.sh --help` if user-facing.
