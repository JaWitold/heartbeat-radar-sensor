# Hardware Diagrams (Fritzing)

This directory stores hardware wiring diagrams and breadboard layouts created with [Fritzing](https://fritzing.org/).

## Files

| Source (`.fzz`) | Vector (`.svg`) | Description |
|-----------------|-----------------|-------------|
| `esp32-ld2450-wiring.fzz` | `esp32-ld2450-wiring.svg` | ESP32 to HLK-LD2450 UART2 interface wiring |
| `esp32-st7789-wiring.fzz` | `esp32-st7789-wiring.svg` | ESP32 to ST7789 SPI display interface wiring |
| `full-assembly.fzz` | `full-assembly.svg` | Complete breadboard assembly combining sensor, display, and power |

## Note on Version Control
`.fzz` files are compressed XML archives. To ensure diffability and GitHub inline rendering, always commit the companion `.svg` export alongside the `.fzz` file.
