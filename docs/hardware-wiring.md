# Hardware Wiring Guide

Source of truth for pins: `platformio.ini` `[env:esp32dev]`.

## Bill of Materials (BOM)

| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| Microcontroller | **ESP32 DevKit V1** (30 or 38 pin) | 1 | Dual-core, hardware UART2 |
| Radar Sensor | **HLK-LD2450** (24GHz mmWave FMCW) | 1 | 3 targets, UART |
| IMU | **MPU-6050** | 1 | I2C, optional (`--imu mpu6050`) |
| Display | **ST7789 TFT LCD** (240x320) | 1 | SPI, IPS panel recommended |
| Buzzer | PWM-driven buzzer | 1 | `BUZZER_PIN` |
| Prototyping | Breadboard + Dupont jumper wires | 1 set | Good-quality power jumpers |
| Power Supply | 5V 2A microUSB / external 5V regulator | 1 | Radar needs stable 5V supply |

---

## Pin Connections

### 1. HLK-LD2450 Radar Sensor (UART2)
Default 256000 baud. ESP32 Hardware Serial 2:

| HLK-LD2450 Pin | ESP32 Pin | Function | Notes |
|:--------------:|:---------:|:--------:|:------|
| **VCC** | **VIN (5V)** | Power | 5V only; not 3.3V |
| **GND** | **GND** | Ground | Common ground |
| **TX** | **GPIO 16 (RX2)** | Sensor Data Out $\to$ ESP32 In | `LD2450_RX_PIN` |
| **RX** | **GPIO 17 (TX2)** | ESP32 Out $\to$ Sensor Command | `LD2450_TX_PIN` |

> [!WARNING]
> LD2450 transmit bursts draw high transient current. ESP32 brownout/reboot at radar start → add 100µF–470µF electrolytic capacitor across sensor `VCC`/`GND`.

---

### 2. ST7789 TFT Display (SPI)

| ST7789 Pin | ESP32 Pin | Function | Description |
|:----------:|:---------:|:--------:|:------------|
| **VCC** | **3.3V** | Power | Logic power |
| **GND** | **GND** | Ground | Common ground |
| **SCL / SCK** | **GPIO 18** | SPI Clock | VSPI SCK (`TFT_SCLK`) |
| **SDA / MOSI**| **GPIO 23** | SPI Data | VSPI MOSI (`TFT_MOSI`) |
| **RES / RST** | **GPIO 4** | Reset | `TFT_RST` |
| **DC** | **GPIO 2** | Data/Command | `TFT_DC` |
| **CS** | **GPIO 15** | Chip Select | `TFT_CS` |
| **BLK / LED** | **GPIO 32** (or 3.3V)| Backlight | PWM-capable for dimming |

---

### 3. MPU-6050 IMU (I2C) and Buzzer

| Device Pin | ESP32 Pin | Macro |
|:----------:|:---------:|:-----:|
| MPU-6050 **SDA** | **GPIO 21** | `MPU6050_SDA_PIN` |
| MPU-6050 **SCL** | **GPIO 22** | `MPU6050_SCL_PIN` |
| Buzzer **+** | **GPIO 25** | `BUZZER_PIN` |

---

### 4. USB-to-UART Adapters (PL2303HX, CP2102, CH340)

Used for testing, simulation, benchtop sensor work.

> [!CAUTION]
> **SET JUMPER TO 3.3V LOGIC LEVEL**
> - ESP32 GPIOs and HLK-LD2450 RX/TX: **3.3V logic**, **not 5V-tolerant**.
> - Adapter with 3.3V/5V jumper or switch: **must be 3.3V**. 5V logic permanently damages ESP32 or radar input pins.
> - **Power vs logic:** LD2450 `VCC` needs 5V (>150mA). Adapter **5V output** (USB VBUS) may power `VCC`; data lines (`TXD`/`RXD`) stay 3.3V.

#### A. Raw Sensor on PC (HUD @ 256000 baud)
View raw 30-byte FMCW frames, no ESP32: `./scripts/gui.sh --port /dev/ttyUSB0 --raw`

| USB-UART Adapter Pin | HLK-LD2450 Pin | Function | Notes |
|:--------------------:|:--------------:|:--------:|:------|
| **5V (Power Out)**   | **VCC**        | Power    | Radar from USB 5V |
| **GND**              | **GND**        | Ground   | Common reference |
| **RXD**              | **TX**         | Inbound  | Sensor → PC |
| **TXD**              | **RX**         | Outbound | PC → sensor |

#### B. Simulator Injection into ESP32 (Hardware-in-the-Loop)
Simulated targets from PC into ESP32 firmware:

| USB-UART Adapter Pin | ESP32 DevKit Pin | Function | Notes |
|:--------------------:|:----------------:|:--------:|:------|
| **TXD**              | **GPIO 16 (RX2)**| Inbound  | Simulated sensor data |
| **GND**              | **GND**          | Ground   | Common reference |

*(Run: `./scripts/sim-radar.sh --sensor ld2450 --scenario patrol --port /dev/ttyUSB0`)*

#### C. Linux Permissions
USB serial converters enumerate as `/dev/ttyUSB0`, owner `root:dialout`. Access without `sudo`:
```bash
sudo usermod -a -G dialout $USER
```
*Apply: log out/in, or `newgrp dialout` in current shell.*

#### D. Flashing via External Adapter
DevKit boards have on-board USB-UART with DTR/RTS auto-reset. Bare ESP32 + external PL2303HX (no DTR/RTS):
1. Connect `TXD` $\to$ `RX0 (GPIO 3)`, `RXD` $\to$ `TX0 (GPIO 1)`, `GND` $\to$ `GND`.
2. At PlatformIO `Connecting...`: hold **BOOT (GPIO 0)**, tap **EN (Reset)**, release **BOOT** → bootloader mode.

---

## Schematic & Breadboard Layout

Fritzing sources (`.fzz`) + `.svg` exports go in `docs/diagrams/hardware/`. Planned files (not yet committed):
- `docs/diagrams/hardware/esp32-ld2450-wiring.svg`
- `docs/diagrams/hardware/esp32-st7789-wiring.svg`
- `docs/diagrams/hardware/full-assembly.svg`

---

## Power Distribution

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
