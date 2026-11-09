# Hardware Wiring Guide

## Bill of Materials (BOM)

| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| Microcontroller | **ESP32 DevKit V1** (30 or 38 pin) | 1 | Standard dual-core with hardware UART2 |
| Radar Sensor | **HLK-LD2450** (24GHz mmWave FMCW) | 1 | 3-target tracking, UART interface |
| Display | **ST7789 TFT LCD** (240x320) | 1 | SPI interface, IPS panel recommended |
| Prototyping | Breadboard + Dupont jumper wires | 1 set | High quality power jumpers |
| Power Supply | 5V 2A microUSB / external 5V regulator | 1 | mmWave sensor requires clean current |

---

## Pin Connections

### 1. HLK-LD2450 Radar Sensor (UART2)
The HLK-LD2450 communicates via high-speed serial (default 256000 baud). Connect to ESP32 Hardware Serial 2:

| HLK-LD2450 Pin | ESP32 Pin | Function | Notes |
|:--------------:|:---------:|:--------:|:------|
| **VCC** | **VIN (5V)** | Power | Requires 5V; do NOT connect to 3.3V |
| **GND** | **GND** | Ground | Common system ground |
| **TX** | **GPIO 16 (RX2)** | Sensor Data Out $\to$ ESP32 In | Hardware Serial 2 RX |
| **RX** | **GPIO 17 (TX2)** | ESP32 Out $\to$ Sensor Command | Hardware Serial 2 TX |

> [!WARNING]
> The HLK-LD2450 peak transmit power draws high transient current. If the ESP32 browns out or reboots when radar starts, add a 100µF–470µF electrolytic capacitor across sensor `VCC` and `GND`.

---

### 2. ST7789 TFT Display (SPI)

| ST7789 Pin | ESP32 Pin | Function | Description |
|:----------:|:---------:|:--------:|:------------|
| **VCC** | **3.3V** | Power | Display logic power |
| **GND** | **GND** | Ground | Common system ground |
| **SCL / SCK** | **GPIO 18** | SPI Clock | Hardware VSPI SCK |
| **SDA / MOSI**| **GPIO 23** | SPI Data | Hardware VSPI MOSI |
| **RES / RST** | **GPIO 4** | Reset | Display hardware reset |
| **DC** | **GPIO 2** | Data/Command | Register select |
| **CS** | **GPIO 15** | Chip Select | SPI device selection |
| **BLK / LED** | **GPIO 32** (or 3.3V)| Backlight | PWM-capable for brightness dimming |

---

### 3. USB-to-UART Adapters (PL2303HX, CP2102, CH340)

External USB-to-UART converters (such as **PL2303HX**, **CP2102**, or **CH340**) are supported across testing, simulation, and direct sensor benchtop workflows.

> [!CAUTION]
> **CRITICAL VOLTAGE RULE: SET JUMPER TO 3.3V LOGIC LEVEL**
> - The ESP32 GPIO pins and the HLK-LD2450 RX/TX pins operate strictly at **3.3V logic level** and are **NOT 5V-tolerant**.
> - If your adapter features a 3.3V/5V selector jumper or switch, **it MUST be configured to 3.3V**. Connecting 5V logic signals will permanently burn ESP32 or radar input pins.
> - **Power supply distinction:** The HLK-LD2450 sensor requires 5V power on its `VCC` pin (>150mA). You may use the adapter's **5V power output** (derived from the PC's USB VBUS) to power `VCC`, while keeping data lines (`TXD`/`RXD`) at 3.3V logic.

#### A. Direct Radar Sensor Testing on PC (Tactical HUD @ 256000 baud)
Inspect raw 30-byte FMCW radar frames on PC with `./scripts/gui.sh --port /dev/ttyUSB0 --raw` without needing an ESP32:

| USB-UART Adapter Pin | HLK-LD2450 Pin | Function | Notes |
|:--------------------:|:--------------:|:--------:|:------|
| **5V (Power Out)**   | **VCC**        | Power    | Powers radar module via USB 5V |
| **GND**              | **GND**        | Ground   | Common reference |
| **RXD**              | **TX**         | Inbound  | Sensor data to PC |
| **TXD**              | **RX**         | Outbound | PC commands to sensor |

#### B. PC Simulator Injection into ESP32 (Hardware-in-the-Loop Simulation)
Stream simulated patrolling targets from PC into the ESP32 firmware running on physical hardware:

| USB-UART Adapter Pin | ESP32 DevKit Pin | Function | Notes |
|:--------------------:|:----------------:|:--------:|:------|
| **TXD**              | **GPIO 16 (RX2)**| Inbound  | Simulated sensor data from PC |
| **GND**              | **GND**          | Ground   | Common reference ground |

*(Run with: `./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0`)*

#### C. Linux Permissions Configuration
Under Linux (e.g. Fedora, Ubuntu), USB serial converters enumerate as `/dev/ttyUSB0` owned by `root:dialout`. To grant permission without `sudo`:
```bash
sudo usermod -a -G dialout $USER
```
*Note: Log out and log back in, or run `newgrp dialout` in your current terminal to apply group membership.*

#### D. Flashing Microcontroller Firmware Notes
Standard ESP32 DevKit boards feature an integrated USB-UART converter with automatic DTR/RTS auto-reset circuits. If you are using an external PL2303HX module to flash bare ESP32 chips (lacking DTR/RTS lines):
1. Connect `TXD` $\to$ `RX0 (GPIO 3)`, `RXD` $\to$ `TX0 (GPIO 1)`, `GND` $\to$ `GND`.
2. When PlatformIO displays `Connecting...`, press and hold the **BOOT (GPIO 0)** button, tap **EN (Reset)**, then release **BOOT** to force bootloader download mode.

---

## Schematic & Breadboard Layout

Detailed Fritzing `.fzz` project files and exported vector graphics (`.svg`) are tracked in `docs/diagrams/hardware/`:
- `docs/diagrams/hardware/esp32-ld2450-wiring.svg`
- `docs/diagrams/hardware/esp32-st7789-wiring.svg`
- `docs/diagrams/hardware/full-assembly.svg`

---

## Power Distribution Summary

```
                      +-------------------+
                      |   5V USB Power    |
                      +---------+---------+
                                |
             +------------------+------------------+
             |                                     |
             v                                     v
     +---------------+                     +---------------+
     |  ESP32 DevKit |                     |  HLK-LD2450   |
     |  (VIN pin)    |                     |  (VCC pin)    |
     +-------+-------+                     +---------------+
             | (3.3V On-board LDO)
             v
     +---------------+
     |  ST7789 TFT   |
     |  (VCC pin)    |
     +---------------+
```
