# Heartbeat Radar Sensor

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)

Human presence + movement detection. **ESP32** + **HLK-LD2450** 24 GHz FMCW mmWave radar. Targets (X/Y position, speed) drawn as radar plot on **ST7789** TFT, or streamed as JSON telemetry over serial / WiFi TCP to desktop or web HUD.

## Features (v1.0)

- HLK-LD2450 UART2 binary protocol parser (256000 baud), up to 3 simultaneous targets, X/Y + speed
- Distance-based threat zones (danger / caution), compile-time threshold overrides
- MPU-6050 IMU attitude estimation; targets rotated into device forward frame
- Output adapters: serial JSON telemetry, WiFi TCP telemetry, ST7789 TFT (renderer = stub)
- PWM buzzer alert paced by threat level
- Hexagonal (ports & adapters); core unit-tested on host
- Docker build, test, lint

## Tech Stack

| Layer            | Technology                          |
| ---------------- | ----------------------------------- |
| MCU              | ESP32 (Xtensa dual-core, 240 MHz)   |
| Radar            | HLK-LD2450 (24 GHz FMCW mmWave)     |
| IMU              | MPU-6050 (I2C)                      |
| Display          | ST7789 240×320 SPI TFT (TFT_eSPI)   |
| Language         | C++17                               |
| Build system     | PlatformIO                          |
| Host tests       | Catch2 (CMake)                      |
| Tooling          | Python via `uv` (simulator, HUDs)   |
| Containerization | Docker                              |
| CI/CD            | GitHub Actions                      |

## Quick Start

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) **or** [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation.html)
- [`uv`](https://github.com/astral-sh/uv) for Python tools
- Git

### Clone

```bash
git clone https://github.com/JaWitold/heartbeat-radar-sensor.git
cd heartbeat-radar-sensor
```

### Build

```bash
# Compile firmware (Docker)
./scripts/build.sh

# Or pick the display (serial is default; st7789 renderer is still a stub)
./scripts/build.sh --displays st7789

# Stream telemetry over WiFi (esp32dev); HUDs connect to socket://heartbeat-radar.local:3333
WIFI_SSID=… WIFI_PASSWORD=… ./scripts/build.sh --displays wifi
```

### Test & Simulate

```bash
# Run host-native Catch2 unit tests (in Docker)
./scripts/test-native.sh

# Run radar simulator (USB-UART, PTY, or socket)
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0
```

### Flash

```bash
./scripts/upload.sh /dev/ttyUSB0
```

## Architecture

Hexagonal (ports & adapters). `src/core/` = pure C++17, no Arduino/ESP-IDF deps. Hardware access only via interfaces in `src/ports/`. Host-tested with mocks.

Details: **[Architecture Documentation](docs/architecture.md)**.

## Project Structure

```
heartbeat-radar-sensor/
├── src/
│   ├── core/                   # Domain logic (pure C++17)
│   ├── ports/                  # Abstract interfaces (ISensor, IDisplay, IImu, ...)
│   ├── adapters/               # sensors, displays, imu, buzzers, timers
│   └── main.cpp                # Composition root, only reader of config macros
├── test_native/                # Host Catch2 tests + mocks
├── test/test_embedded/         # On-device tests (empty)
├── scripts/                    # Build/test/flash wrappers
│   └── tools/                  # Python: sensor_sim, radar_gui, radar_web
├── simulation/wokwi/           # Wokwi custom LD2450 chip
├── docker/                     # Dev / test / QA Dockerfiles, compose
├── docs/                       # Architecture, wiring, guides, diagrams
├── platformio.ini              # Board envs and wiring macros
└── .github/workflows/          # CI and release
```

## Scripts Reference

| Script | Command | Description |
|---|---|---|
| `scripts/build.sh` | `./scripts/build.sh [--board B] [--sensors S] [--displays D] [-f NAME=value]` | Build firmware (Docker) |
| `scripts/sim-radar.sh` | `./scripts/sim-radar.sh [--sensor S] [--scenario SC] [--port P \| --pty]` | Radar sensor simulator (uv) |
| `scripts/gui.sh` | `./scripts/gui.sh [--demo \| --port P]` | Desktop radar HUD, Tkinter (uv) |
| `scripts/web.sh` | `./scripts/web.sh [--demo \| --serial P]` | Web radar HUD on http://127.0.0.1:8088 (uv) |
| `scripts/sim-tmux.sh` | `./scripts/sim-tmux.sh [--web]` | Simulator + HUD in tmux |
| `scripts/test-native.sh` | `./scripts/test-native.sh` | Host Catch2 unit tests (Docker) |
| `scripts/test-python.sh` | `./scripts/test-python.sh` | Python tool tests |
| `scripts/test.sh` | `./scripts/test.sh` | All tests: native + Python (if `uv`) |
| `scripts/upload.sh` | `./scripts/upload.sh [port]` | Flash firmware to ESP32 |
| `scripts/monitor.sh` | `./scripts/monitor.sh [port]` | Serial monitor (921600 baud) |
| `scripts/lint.sh` | `./scripts/lint.sh` | clang-tidy + cppcheck |
| `scripts/format.sh` | `./scripts/format.sh` | clang-format in place |
| `scripts/format-check.sh` | `./scripts/format-check.sh` | Check formatting (non-zero exit if dirty) |
| `scripts/docs.sh` | `./scripts/docs.sh` | Doxygen HTML docs |

## Hardware

| Component        | Model / Spec                | Qty |
| ---------------- | --------------------------- | --- |
| Microcontroller  | ESP32 DevKit V1             | 1   |
| Radar module     | HLK-LD2450 (24 GHz)         | 1   |
| IMU              | MPU-6050                    | 1   |
| TFT display      | ST7789 240×320 SPI          | 1   |
| Misc             | Jumper wires, breadboard    | —   |

Pins + wiring: **[Hardware Wiring Guide](docs/hardware-wiring.md)**. Pins defined in `platformio.ini` `[env:esp32dev]`.

## Roadmap

[GitHub Issues](https://github.com/JaWitold/heartbeat-radar-sensor/issues).

## Contributing

**[Contributing Guide](docs/contributing.md)**.

## License

MIT — [LICENSE](LICENSE).
