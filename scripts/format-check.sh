#!/usr/bin/env bash
# =============================================================================
# format-check.sh — Check code formatting (CI mode)
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Verifies that all source files conform to the project's clang-format style.
# Exits non-zero if any file would be reformatted — suitable for CI pipelines.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# --- Main ---
info "Checking code formatting..."

if docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm format-check; then
    success "All files are properly formatted."
else
    error "Formatting issues detected. Run './scripts/format.sh' to fix."
    exit 1
fi
