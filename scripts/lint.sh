#!/usr/bin/env bash
# =============================================================================
# lint.sh — Run all linters in Docker
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Performs static analysis using clang-tidy and cppcheck inside the QA
# container. Exits non-zero if any issues are found.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# --- Main ---
info "Running linters (clang-tidy + cppcheck)..."

# clang-tidy needs build/compile_commands.json; only the test image has cmake + Catch2
docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm test-native \
    bash -c "cmake -S /workspace -B /workspace/build -G Ninja"
docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm lint

success "Linting complete — no issues found."
