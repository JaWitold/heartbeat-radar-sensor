---
name: run-tests
description: Run the test suite for Heartbeat Radar Sensor across host Catch2 unit tests and on-device tests.
---

# Skill: Run Tests

## Purpose
Executes native unit tests and embedded integration tests using the isolated Docker environment or physical hardware.

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

### All Tests (Native + Embedded)
To run the full suite:
```bash
./scripts/test.sh
```

### Debugging Failing Tests
To run Catch2 directly with specific filter tags:
```bash
docker compose -f docker/docker-compose.yml run --rm test-native \
    bash -c "cd /workspace/build && ./test_native/unit_tests '[core]'"
```
