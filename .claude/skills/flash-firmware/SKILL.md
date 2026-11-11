---
name: flash-firmware
description: Build, verify, flash Heartbeat Radar Sensor firmware to ESP32 (Docker PlatformIO build, upload, serial monitor check).
---

# Skill: Flash Firmware

## Purpose
Build firmware in Docker PlatformIO env. Flash to USB-connected ESP32.

## Procedure

### Step 1: Pre-flight
Format check, lint, tests. Block flashing non-compliant firmware:
```bash
./scripts/format-check.sh
./scripts/lint.sh
./scripts/test.sh
```

### Step 2: Build in Docker
```bash
# Default build: LD2450 sensor + Serial JSON telemetry output
./scripts/build.sh

# With custom alert threshold override
./scripts/build.sh -f RADAR_DANGER_DISTANCE_M=2.0f

# Adapter tunables use the same -f path (names: .claude/rules/adapter-config.md)
./scripts/build.sh -f LD2450_BAUD_RATE=115200

# WiFi telemetry display (secrets from env, never committed)
WIFI_SSID=… WIFI_PASSWORD=… ./scripts/build.sh --displays wifi -f WIFI_TCP_PORT=4000

# With SPI TFT display output (renderer is currently a stub)
./scripts/build.sh --displays st7789
```
Check output exists: `.pio/build/esp32dev/firmware.bin`.

### Step 3: Flash
ESP32 on USB:
```bash
# Default serial device /dev/ttyUSB0
./scripts/upload.sh

# Or supply the target serial port explicitly:
./scripts/upload.sh /dev/ttyACM0
```

### Step 4: Verify Boot
Serial monitor; check sensor/display init logs:
```bash
./scripts/monitor.sh /dev/ttyUSB0
```
Telemetry: one JSON frame per line (`src/core/telemetry_frame.cpp`):
```json
{"type":"radar_frame","sweep_deg":12.5,"alert":"Safe","severity":0.00,"targets":[{"id":1,"x":0.5,"y":2.0,"speed":-0.2,"distance":2.06,"threat_zone":"Safe"}]}
```
`imu` object appended when MPU-6050 data valid.
