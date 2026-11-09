---
name: flash-firmware
description: Guide and execute the build, verification, and flashing process for the Heartbeat Radar Sensor ESP32 target.
---

# Skill: Flash Firmware

## Purpose
This skill coordinates the compilation of firmware via the Dockerized PlatformIO environment and guides flashing to a connected ESP32 target device.

## Flashing Procedure

### Step 1: Pre-flight Verification
Run local static analysis, test suite, and format check to prevent flashing non-compliant firmware:
```bash
./scripts/format-check.sh
./scripts/lint.sh
./scripts/test.sh
```

### Step 2: Build Firmware in Docker
Compile the project binary inside the container:
```bash
# Default build: LD2450 sensor + Serial JSON telemetry output
./scripts/build.sh

# With custom alert threshold override
./scripts/build.sh -f RADAR_DANGER_DISTANCE_M=2.0f

# With SPI TFT display output (renderer is currently a stub)
./scripts/build.sh --displays st7789
```
Verify that the output `.pio/build/esp32dev/firmware.bin` was created.

### Step 3: Flash to ESP32
Connect the ESP32 via USB and run:
```bash
# Default serial device /dev/ttyUSB0
./scripts/upload.sh

# Or supply the target serial port explicitly:
./scripts/upload.sh /dev/ttyACM0
```

### Step 4: Verify Device Boot
Open the serial monitor to view startup logs and verify sensor/display initialization:
```bash
./scripts/monitor.sh /dev/ttyUSB0
```
Telemetry will output JSON frames:
```json
{"threat":"Safe","targets":[{"id":1,"x":0.5,"y":2.0,"speed":-0.2,"distance":2.06,"threat_zone":"Safe"}]}
```
