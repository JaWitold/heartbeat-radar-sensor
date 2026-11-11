# Getting Started

## Prerequisites

Docker-first. No local compiler, ESP-IDF, or PlatformIO needed to build or test.

- **Required:**
  - [Docker](https://docs.docker.com/get-docker/) (v24+, Docker Compose v2)
  - [Git](https://git-scm.com/)
- **Optional (flash outside containers):**
  - [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/installation/index.html) or VS Code + PlatformIO IDE extension

---

## Quick Start

### 1. Clone
```bash
git clone git@github.com:JaWitold/heartbeat-radar-sensor.git
cd heartbeat-radar-sensor
```

### 2. Format & Static Analysis
Runs in containers:
```bash
# Check formatting
./scripts/format-check.sh

# Run static analysis (clang-tidy & cppcheck)
./scripts/lint.sh
```

### 3. Native Unit Tests (Catch2)
GCC host build in Docker, no hardware:
```bash
./scripts/test-native.sh
```

### 4. Build Firmware
ESP32 binary, containerized PlatformIO:
```bash
./scripts/build.sh
```

### 5. Flash
ESP32 via USB:
```bash
# Default uses /dev/ttyUSB0
./scripts/upload.sh

# Or specify a custom serial port:
./scripts/upload.sh /dev/ttyACM0
```

### 6. Serial Monitor
Logs + telemetry, 921600 baud:
```bash
./scripts/monitor.sh /dev/ttyUSB0
```

---

## Directory Overview

| Path | Purpose |
|------|---------|
| `src/core/` | Platform-independent domain logic |
| `src/ports/` | Abstract C++ interfaces (`ISensor`, `IDisplay`, `ITimer`, `IBuzzer`, `IImu`, `IThreatEvaluator`) |
| `src/adapters/` | Hardware drivers (HLK-LD2450 UART, serial/WiFi telemetry, ST7789 SPI, MPU-6050 I2C, buzzer PWM) |
| `test_native/` | Catch2 host unit tests + mocks |
| `scripts/` | Shell entry points for Docker operations |
| `docker/` | Dockerfiles (`dev`, `test`, `qa`) + Compose file |
| `docs/` | Architecture, wiring, diagrams |
