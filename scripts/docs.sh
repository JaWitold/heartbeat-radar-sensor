#!/usr/bin/env bash
# =============================================================================
# docs.sh — Generate Doxygen documentation
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Runs Doxygen inside the QA Docker container to generate API documentation
# from annotated source code. Output is written to docs/html/.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# --- Main ---
info "Generating Doxygen documentation..."

docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm docs

success "Documentation generated. Open docs/html/index.html in your browser."
