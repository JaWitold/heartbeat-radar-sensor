#!/usr/bin/env bash
# =============================================================================
# upload.sh — Upload firmware to ESP32 via serial
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Uploads the compiled firmware to the ESP32 over a serial port.
# Runs OUTSIDE Docker because it needs direct USB/serial device access.
#
# Usage:
#   ./scripts/upload.sh [PORT]
#
# Arguments:
#   PORT  Serial port device (default: /dev/ttyUSB0)
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

# --- Check for PlatformIO ---
if ! command -v pio &>/dev/null; then
    error "PlatformIO CLI (pio) is not installed or not in PATH."
    echo ""
    echo "  Upload requires direct USB access and must run outside Docker."
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

# --- Upload ---
info "Uploading firmware to ${PORT}..."

cd "$PROJECT_ROOT"
pio run -t upload --upload-port "$PORT"

success "Firmware uploaded to ${PORT}."
