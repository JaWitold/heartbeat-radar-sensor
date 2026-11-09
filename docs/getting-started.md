# Getting Started

## Prerequisites

The project is designed with a **Docker-first philosophy**: you do not need to install local compilers, ESP-IDF toolchains, or PlatformIO on your host machine to build or test the project.

- **Required:**
  - [Docker](https://docs.docker.com/get-docker/) (v24+ recommended with Docker Compose v2)
  - [Git](https://git-scm.com/)
- **Optional (for direct hardware flashing outside containers):**
  - [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/installation/index.html) or VS Code with PlatformIO IDE extension

---

## Quick Start

### 1. Clone Repository
```bash
git clone git@github.com:JaWitold/heartbeat-radar-sensor.git
cd heartbeat-radar-sensor
```

### 2. Verify Code Quality & Format
All QA tools run in isolated containers:
```bash
# Check formatting
./scripts/format-check.sh

# Run static analysis (clang-tidy & cppcheck)
./scripts/lint.sh
```

### 3. Run Native Unit Tests (Catch2)
Host-side tests compile with GCC and run in a Docker container without physical hardware:
```bash
./scripts/test-native.sh
```

### 4. Build Firmware
Compile the ESP32 binary using the containerized PlatformIO build environment:
```bash
./scripts/build.sh
```

### 5. Flash to Hardware
Connect your ESP32 via USB and upload:
```bash
# Default uses /dev/ttyUSB0
./scripts/upload.sh

# Or specify a custom serial port:
./scripts/upload.sh /dev/ttyACM0
```

### 6. Monitor Serial Output
View real-time logs and debug telemetry from the device:
```bash
./scripts/monitor.sh /dev/ttyUSB0
```

---

## Directory Overview

| Path | Purpose |
|------|---------|
| `src/core/` | Platform-independent radar tracking domain logic |
| `src/ports/` | Abstract C++ interfaces (`ISensor`, `IDisplay`, `ITimer`, `IBuzzer`) |
| `src/adapters/` | Hardware-specific drivers (HLK-LD2450 UART, ST7789 SPI) |
| `test_native/` | Catch2 host-side unit tests |
| `scripts/` | Shell script entry points for Docker operations |
| `docker/` | Dockerfiles (`dev`, `test`, `qa`) and Compose specification |
| `docs/` | Architecture guides, hardware wiring, and diagrams |
