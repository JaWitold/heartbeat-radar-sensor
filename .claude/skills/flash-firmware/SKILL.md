---
name: flash-firmware
description: Guide and execute the build, verification, and flashing process for the Heartbeat Radar Sensor ESP32 target.
---

# Skill: Flash Firmware

## Purpose
This skill coordinates the compilation of firmware via the Dockerized PlatformIO environment and guides flashing to a connected ESP32 target device.

## Flashing Procedure

### Step 1: Pre-flight Verification
Run local static analysis and format check to prevent flashing non-compliant firmware:
```bash
./scripts/format-check.sh
./scripts/lint.sh
```

### Step 2: Build Firmware in Docker
Compile the project binary inside the container:
```bash
./scripts/build.sh
```
Verify that the output `.pio/build/esp32dev/firmware.bin` was created.

### Step 3: Flash to ESP32
Connect the ESP32 via USB and run:
```bash
# If using the default serial device /dev/ttyUSB0
./scripts/upload.sh

# Or supply the target serial port explicitly:
./scripts/upload.sh /dev/ttyACM0
```

### Step 4: Verify Device Boot
Open the serial monitor to view startup logs and verify sensor/display initialization:
```bash
./scripts/monitor.sh /dev/ttyUSB0
```
