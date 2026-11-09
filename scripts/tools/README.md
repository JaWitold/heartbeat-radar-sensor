# Heartbeat Radar Sensor — Python Tools (`hrs-tools`)

Simulation, testing, and desktop telemetry tools managed with [`uv`](https://github.com/astral-sh/uv).

## Tools Included

1. **`sim-radar` (`sensor_sim`):** Hexagonal multi-sensor protocol simulator (HLK-LD2450, RD-03D, RCWL-0516) streaming over USB-UART, virtual PTY, or TCP sockets.
2. **`radar-gui` (`radar_gui`):** Tactical COD MW2 / R6 Siege radar HUD visualizer for serial telemetry.

## Quickstart with `uv`

```bash
# Sync dependencies and create virtualenv
uv sync

# Run the radar simulator
uv run sim-radar --sensor ld2450 --scenario patrol --pty

# Run the desktop radar GUI in demo mode
uv run radar-gui --demo

# Connect GUI to real hardware or PTY
uv run radar-gui --port /dev/ttyUSB0 --baud 115200

# Run all unit tests
uv run pytest
```
