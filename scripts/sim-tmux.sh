#!/usr/bin/env bash
# =============================================================================
# sim-tmux.sh — Run Radar Simulator + Tactical HUD inside tmux
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Spawns the radar sensor simulator on a virtual Linux PTY in one pane and
# automatically launches the Tactical Radar Desktop HUD connected to that PTY
# in a second pane.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SESSION_NAME="hrs-sim"
PTY_FILE="/tmp/hrs_radar_pty"

# Color helpers
CYAN='\033[0;36m'
GREEN='\033[0;32m'
AMBER='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'

# Default arguments
SENSOR="ld2450"
SCENARIO="multi"
NO_ATTACH=false
USE_WEB=false

usage() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS]

Runs the Heartbeat Radar sensor simulator and tactical HUD together
inside a split tmux session connected via a virtual serial PTY.

Options:
  -s, --sensor SENSOR     Sensor to simulate: ld2450 | rd03d | rcwl0516 (default: ld2450)
  -m, --scenario SCENARIO Motion scenario: multi | patrol | interactive (default: multi)
  -w, --web               Launch Web HUD (browser display) instead of desktop GUI
  -d, --detach            Start session in background without attaching
  -k, --kill              Kill any running simulation tmux session and exit
  -h, --help              Show this help message

Examples:
  ./scripts/sim-tmux.sh                       # Desktop Tkinter GUI with 3 targets
  ./scripts/sim-tmux.sh --web                 # Web Browser HUD at http://localhost:8080
  ./scripts/sim-tmux.sh --scenario patrol     # Patrolling target (1m to 5m)
  ./scripts/sim-tmux.sh --sensor rd03d        # Wide-angle RD-03D sensor
  ./scripts/sim-tmux.sh --kill                # Stop the running session
EOF
    exit 0
}

# Parse CLI options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -s|--sensor)
            SENSOR="$2"
            shift 2
            ;;
        -m|--scenario)
            SCENARIO="$2"
            shift 2
            ;;
        -w|--web)
            USE_WEB=true
            shift
            ;;
        -d|--detach)
            NO_ATTACH=true
            shift
            ;;
        -k|--kill)
            echo -e "${AMBER}Stopping simulation tmux session '${SESSION_NAME}'...${NC}"
            tmux kill-session -t "$SESSION_NAME" 2>/dev/null || true
            rm -f "$PTY_FILE"
            echo -e "${GREEN}Session stopped.${NC}"
            exit 0
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo -e "${RED}Error: Unknown argument '$1'${NC}" >&2
            usage
            ;;
    esac
done

# Ensure tmux is available
if ! command -v tmux >/dev/null 2>&1; then
    echo -e "${RED}Error: 'tmux' is required but not installed.${NC}" >&2
    echo -e "Install tmux: sudo dnf install tmux (or sudo apt install tmux)" >&2
    exit 1
fi

echo -e "${CYAN}====================================================${NC}"
echo -e "${CYAN}   🎯 Heartbeat Radar — tmux Simulation Runner     ${NC}"
echo -e "${CYAN}====================================================${NC}"

# Kill any existing session with the same name
if tmux has-session -t "$SESSION_NAME" 2>/dev/null; then
    echo -e "${AMBER}Terminating previous '${SESSION_NAME}' session...${NC}"
    tmux kill-session -t "$SESSION_NAME" 2>/dev/null || true
fi
rm -f "$PTY_FILE"

# Determine host display for GUI
DISPLAY_ENV="${DISPLAY:-:0}"
WAYLAND_ENV="${WAYLAND_DISPLAY:-wayland-0}"

echo -e "Starting tmux session:  ${GREEN}${SESSION_NAME}${NC}"
echo -e "Radar Sensor:           ${GREEN}${SENSOR}${NC}"
echo -e "Target Scenario:        ${GREEN}${SCENARIO}${NC}"

# 1. Create detached session running the simulator in the first pane
tmux new-session -d -s "$SESSION_NAME" -n "Radar-Sim"
tmux send-keys -t "$SESSION_NAME:0.0" "cd '${PROJECT_ROOT}' && ./scripts/sim-radar.sh --sensor '${SENSOR}' --scenario '${SCENARIO}' --pty" C-m

# 2. Wait for simulator to create the virtual PTY device
echo -e "${AMBER}Waiting for virtual PTY allocation...${NC}"
PTY_DEVICE=""
for _ in {1..50}; do
    if [[ -s "$PTY_FILE" ]]; then
        PTY_DEVICE="$(cat "$PTY_FILE")"
        if [[ -c "$PTY_DEVICE" || -e "$PTY_DEVICE" ]]; then
            break
        fi
    fi
    sleep 0.1
done

if [[ -z "$PTY_DEVICE" ]]; then
    echo -e "${RED}Error: Timed out waiting for virtual PTY node.${NC}" >&2
    tmux kill-session -t "$SESSION_NAME" 2>/dev/null || true
    exit 1
fi

echo -e "Virtual Serial Node:    ${GREEN}${PTY_DEVICE}${NC}"

# 3. Split window horizontally: Left = Simulator, Right = Display launcher & monitor
tmux split-window -h -t "$SESSION_NAME:0" -p 50

# Configure the HUD pane (Web Server vs Desktop GUI)
if [[ "$USE_WEB" == true ]]; then
    HUD_CMD="cd '${PROJECT_ROOT}' && ./scripts/web.sh --serial '${PTY_DEVICE}' --raw --http-port 8088"
    HUD_LABEL="Web HUD Server (http://localhost:8088)"
else
    HUD_CMD="cd '${PROJECT_ROOT}' && export DISPLAY='${DISPLAY_ENV}' && export WAYLAND_DISPLAY='${WAYLAND_ENV}' && ./scripts/gui.sh --port '${PTY_DEVICE}' --raw"
    HUD_LABEL="Desktop Tkinter HUD (display: ${DISPLAY_ENV})"
fi
tmux send-keys -t "$SESSION_NAME:0.1" "$HUD_CMD" C-m

echo -e "${GREEN}====================================================${NC}"
echo -e "${GREEN}   ✅ Simulation running in tmux session '${SESSION_NAME}'${NC}"
echo -e "${GREEN}   - Pane 0: Radar Simulator (${SENSOR} / ${SCENARIO})${NC}"
echo -e "${GREEN}   - Pane 1: ${HUD_LABEL}${NC}"
echo -e "${GREEN}====================================================${NC}"
if [[ "$USE_WEB" == true ]]; then
    echo -e "Open Web Browser:       ${CYAN}http://localhost:8088${NC}"
fi
echo -e "To detach from tmux:    Press ${CYAN}Ctrl+b${NC} then ${CYAN}d${NC}"
echo -e "To re-attach later:     ${CYAN}tmux attach -t ${SESSION_NAME}${NC}"
echo -e "To stop simulation:     ${CYAN}./scripts/sim-tmux.sh --kill${NC}"
echo ""

# Attach if running in an interactive terminal and not explicitly detached
if [[ "$NO_ATTACH" == false && -t 0 && -t 1 ]]; then
    tmux attach-session -t "$SESSION_NAME"
fi
