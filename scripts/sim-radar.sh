#!/usr/bin/env bash
# =============================================================================
# sim-radar.sh — Swappable Radar Sensor Simulator CLI (uv-powered)
# =============================================================================
# Streams authentic binary frames for HLK-LD2450, RD-03D, or RCWL-0516
# into a physical USB-UART adapter (/dev/ttyUSB0), a virtual PTY (/dev/pts/*),
# or a network socket.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TOOLS_DIR="$PROJECT_ROOT/scripts/tools"

# Colors for terminal output
CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${CYAN}====================================================${NC}"
echo -e "${CYAN}   Heartbeat Radar — Hexagonal Sensor Simulator  ${NC}"
echo -e "${CYAN}====================================================${NC}"

if command -v uv >/dev/null 2>&1; then
    uv run --project "$TOOLS_DIR" sim-radar "$@"
else
    export PYTHONPATH="$TOOLS_DIR:${PYTHONPATH:-}"
    python3 -m sensor_sim.main "$@"
fi
