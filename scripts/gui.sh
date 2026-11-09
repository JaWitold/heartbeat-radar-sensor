#!/usr/bin/env bash
# =============================================================================
# gui.sh — Tactical Radar Desktop HUD Launcher (uv-powered)
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TOOLS_DIR="$PROJECT_ROOT/scripts/tools"

CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${CYAN}====================================================${NC}"
echo -e "${CYAN}   🎯 Heartbeat Radar — Tactical Desktop HUD        ${NC}"
echo -e "${CYAN}====================================================${NC}"

if command -v uv >/dev/null 2>&1; then
    uv run --project "$TOOLS_DIR" radar-gui "$@"
else
    export PYTHONPATH="$TOOLS_DIR:${PYTHONPATH:-}"
    python3 -m radar_gui "$@"
fi
