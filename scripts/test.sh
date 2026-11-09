#!/usr/bin/env bash
# =============================================================================
# test.sh — Run ALL tests
# Heartbeat Radar Sensor (ESP32 + HLK-LD2450)
# =============================================================================
# Orchestrates all test suites and prints a summary. Currently runs:
#   1. Native (host-side) Catch2 unit tests
#
# Future test targets (e.g., integration, hardware-in-the-loop) can be added
# as additional steps below.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# --- Colored output helpers ---
info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }
header()  { echo -e "\n\033[1;36m════════════════════════════════════════════\033[0m"; echo -e "\033[1;36m  $*\033[0m"; echo -e "\033[1;36m════════════════════════════════════════════\033[0m\n"; }

PASSED=0
FAILED=0

# --- Helper: run a test suite and track results ---
run_suite() {
    local name="$1"
    local script="$2"

    header "$name"

    if bash "$script"; then
        success "$name — PASSED"
        ((PASSED++))
    else
        error "$name — FAILED"
        ((FAILED++))
    fi
}

# --- Run test suites ---
info "Running all test suites..."

run_suite "Native Unit Tests (Catch2)" "${SCRIPT_DIR}/test-native.sh"

# Future: add more suites here
# run_suite "Integration Tests" "${SCRIPT_DIR}/test-integration.sh"

# --- Summary ---
header "Test Summary"

echo "  Passed: ${PASSED}"
echo "  Failed: ${FAILED}"
echo ""

if [[ "$FAILED" -gt 0 ]]; then
    error "${FAILED} test suite(s) failed."
    exit 1
fi

success "All ${PASSED} test suite(s) passed."
