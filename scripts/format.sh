#!/usr/bin/env bash
# =============================================================================
# format.sh — Auto-format all source code
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Runs clang-format in-place on all .cpp and .h files in src/ and test_native/
# using the QA Docker container.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# --- Main ---
info "Auto-formatting source files..."

docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm format

success "All source files formatted."
