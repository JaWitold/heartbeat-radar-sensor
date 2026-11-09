#!/usr/bin/env bash
# =============================================================================
# monitor.sh — Serial monitor for ESP32
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Opens a serial monitor to view real-time output from the ESP32.
# Runs OUTSIDE Docker because it needs direct USB/serial device access.
#
# Usage:
#   ./scripts/monitor.sh [PORT]
#
# Arguments:
#   PORT  Serial port device (default: /dev/ttyUSB0)
#
# Press Ctrl+] to exit the monitor.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# --- Configuration ---
PORT="${1:-/dev/ttyUSB0}"
BAUD=115200

# --- Check for PlatformIO ---
if ! command -v pio &>/dev/null; then
    error "PlatformIO CLI (pio) is not installed or not in PATH."
    echo ""
    echo "  Serial monitor requires direct USB access and must run outside Docker."
    echo "  Install PlatformIO CLI locally:"
    echo ""
    echo "    pip install platformio"
    echo "    # or"
    echo "    curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py"
    echo "    python3 get-platformio.py"
    echo ""
    exit 1
fi

# --- Check serial port ---
if [[ ! -e "$PORT" ]]; then
    error "Serial port '$PORT' does not exist."
    echo "  Available serial ports:"
    ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "    (none found)"
    echo ""
    echo "  Usage: $0 [PORT]"
    exit 1
fi

# --- Start monitor ---
info "Opening serial monitor on ${PORT} at ${BAUD} baud..."
info "Press Ctrl+] to exit."
echo ""

cd "$PROJECT_ROOT"
pio device monitor --port "$PORT" --baud "$BAUD"
