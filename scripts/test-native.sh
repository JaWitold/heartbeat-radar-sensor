#!/usr/bin/env bash
# =============================================================================
# test-native.sh — Run native Catch2 unit tests in Docker
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Builds and executes host-side Catch2 tests inside the test container.
# This validates domain/core logic without requiring ESP32 hardware.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# --- Main ---
info "Running native Catch2 tests in Docker..."

docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm test-native

success "Native tests complete."
