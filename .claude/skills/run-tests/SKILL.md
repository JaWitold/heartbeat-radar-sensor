---
name: run-tests
description: Run the test suite for Heartbeat Radar Sensor across host Catch2 unit tests and on-device tests.
---

# Skill: Run Tests

## Purpose
Executes native C++ unit tests, Python tooling tests, and static analysis verification using isolated Docker containers.

## Test Workflow

### Native Host Unit Tests (Catch2)
Host tests run in Docker without requiring ESP32 hardware or cross-compilers:
```bash
./scripts/test-native.sh
```

Under the hood, this executes:
1. CMake configuration with `-GNinja`
2. Catch2 test compilation with GCC 13
3. `ctest --output-on-failure`

### Python Tools & Simulation Tests (pytest)
Runs pytest on sensor simulators, protocol encoders/decoders, and web HUD server:
```bash
./scripts/test-python.sh
```

### Full Test Suite (Native C++ + Python)
Runs all test suites across the project:
```bash
./scripts/test.sh
```

### Formatting & Static Analysis
```bash
./scripts/format-check.sh
./scripts/lint.sh
```

### Debugging Failing Tests
To run Catch2 directly with specific filter tags:
```bash
docker compose -f docker/docker-compose.yml run --rm test-native \
    bash -c "cd /workspace/build && ./test_native/unit_tests '[core]'"
```
