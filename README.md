# 🎯 Heartbeat Radar Sensor

<!-- Badges -->
[![CI Status](https://img.shields.io/badge/CI-passing-brightgreen?style=flat-square)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange?style=flat-square)](https://platformio.org)

---

**A real-time human presence and movement detection system** inspired by tactical heartbeat sensors from games like Call of Duty and Rainbow Six Siege — built on real hardware.

The system uses an **ESP32** microcontroller paired with an **HLK-LD2450** millimeter-wave radar module to detect human presence, distance, and movement angle. Detection results are rendered in real time on an **ST7789 TFT display** as a radar sweep visualization.

## ✨ Features (v1.0 MVP)

- 🔍 Real-time human target detection via HLK-LD2450 mmWave radar
- 📡 Distance and angle measurement for up to 3 simultaneous targets
- 🖥️ Radar sweep visualization on ST7789 TFT display
- ⚙️ Configurable detection zones and sensitivity thresholds
- 🏗️ Clean hexagonal (ports & adapters) architecture
- 🧪 Comprehensive unit and integration test suite
- 🐳 Fully containerized development environment

## 🛠️ Tech Stack

| Layer            | Technology                        |
| ---------------- | --------------------------------- |
| MCU              | ESP32 (Xtensa dual-core, 240MHz) |
| Radar Sensor     | HLK-LD2450 (24 GHz mmWave)       |
| Display          | ST7789 240×240 TFT               |
| Language         | C++17                             |
| Build System     | PlatformIO                        |
| Architecture     | Hexagonal (Ports & Adapters)      |
| Testing          | Unity (PlatformIO native)         |
| Containerization | Docker                            |
| CI/CD            | GitLab CI                         |

## 🚀 Quick Start

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) (recommended) **or** [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation.html)
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

# Or select composable targets (fusion / multi-display)
./scripts/build.sh --sensors ld2450,rd03d --displays st7789,serial
```

### Test & Simulate

```bash
# Run host-native Catch2 unit tests (in Docker)
./scripts/test-native.sh

# Run swappable radar simulator (USB-UART, PTY, or socket)
./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0
```

### Flash

```bash
# Flash to connected ESP32
./scripts/upload.sh /dev/ttyUSB0
```

## 🏛️ Architecture

This project follows a **hexagonal (ports & adapters) architecture** to keep business logic decoupled from hardware drivers and framework specifics. This enables comprehensive testing on the host machine without physical hardware.

For a detailed architecture overview, diagrams, and design decisions, see:

📖 **[Architecture Documentation](docs/architecture.md)**

## 📁 Project Structure

```
heartbeat-radar-sensor/
├── src/                        # Application source code
│   ├── domain/                 #   Core business logic (pure C++)
│   ├── ports/                  #   Interface definitions
│   ├── adapters/               #   Hardware & framework implementations
│   └── main.cpp                #   Entry point
├── include/                    # Public headers
├── lib/                        # Project-specific libraries
├── test/                       # Test suites
│   ├── unit/                   #   Unit tests (native)
│   └── integration/            #   Integration tests
├── docs/                       # Documentation
│   ├── architecture.md         #   Architecture overview
│   ├── hardware-wiring.md      #   Wiring diagrams & BOM
│   └── contributing.md         #   Contribution guidelines
├── scripts/                    # Automation scripts
├── hardware/                   # Fritzing schematics & diagrams
├── platformio.ini              # PlatformIO configuration
├── Dockerfile                  # Development container
├── docker-compose.yml          # Container orchestration
└── .gitlab-ci.yml              # CI/CD pipeline
```

## 📜 Scripts Reference

| Script | Command | Description |
|---|---|---|
| `scripts/build.sh` | `./scripts/build.sh [--board B] [--sensor S] [--display D]` | Build firmware with orthogonal targets (Docker) |
| `scripts/sim-radar.sh` | `./scripts/sim-radar.sh [--sensor S] [--scenario SC] [--port P \| --pty]` | Run swappable radar sensor simulator (uv) |
| `scripts/gui.sh` | `./scripts/gui.sh [--demo \| --port P]` | Tactical desktop radar HUD display (uv) |
| `scripts/test-native.sh` | `./scripts/test-native.sh` | Run host Catch2 unit tests (Docker) |
| `scripts/test.sh` | `./scripts/test.sh` | Run all unit & integration tests |
| `scripts/upload.sh` | `./scripts/upload.sh [port]` | Flash firmware to connected ESP32 |
| `scripts/monitor.sh` | `./scripts/monitor.sh [port]` | Open serial console monitor |
| `scripts/lint.sh` | `./scripts/lint.sh` | Run clang-tidy and cppcheck static analysis |
| `scripts/format.sh` | `./scripts/format.sh` | Auto-format C++ code with clang-format |
| `scripts/format-check.sh` | `./scripts/format-check.sh` | Check formatting in CI (exits non-zero if dirty) |
| `scripts/docs.sh` | `./scripts/docs.sh` | Generate Doxygen HTML documentation |

## 🔧 Hardware

### Components

| Component        | Model / Spec               | Qty |
| ---------------- | -------------------------- | --- |
| Microcontroller  | ESP32 DevKit V1            | 1   |
| Radar Module     | HLK-LD2450 (24 GHz)       | 1   |
| TFT Display      | ST7789 240×240 1.3" IPS    | 1   |
| Misc             | Jumper wires, breadboard   | —   |

For complete wiring diagrams, pin mappings, and assembly instructions, see:

🔌 **[Hardware Wiring Guide](docs/hardware-wiring.md)**

## 🗺️ Roadmap

Planned features and enhancements are tracked as issues:

📋 **[GitHub Issues](https://github.com/JaWitold/heartbeat-radar-sensor/issues)**

## 🤝 Contributing

Contributions are welcome! Please read the contribution guidelines before submitting a pull request:

📖 **[Contributing Guide](docs/contributing.md)**

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

<p align="center">
  <sub>Built with ❤️ by <a href="https://github.com/JaWitold">JaWitold</a></sub>
</p>
