---
name: run-tests
description: Run Heartbeat Radar Sensor tests — host Catch2 unit tests, Python tool tests (pytest), format and static analysis — in Docker.
---

# Skill: Run Tests

## Purpose
Native C++ unit tests, Python tool tests, static analysis. Isolated Docker containers.

## Workflow

### Native Host Unit Tests (Catch2)
Docker. No ESP32, no cross-compiler:
```bash
./scripts/test-native.sh
```

Steps inside:
1. CMake configure, `-GNinja`
2. Catch2 compile, GCC 13
3. `ctest --output-on-failure`

### Python Tool Tests (pytest)
Sensor simulators, protocol encoders/decoders, web HUD server:
```bash
./scripts/test-python.sh
```

### Full Suite (Native C++ + Python)
Python suite skipped if `uv` missing:
```bash
./scripts/test.sh
```

### Format & Static Analysis
```bash
./scripts/format-check.sh
./scripts/lint.sh
```

### Debug Failing Tests
Catch2 directly with tag filter:
```bash
docker compose -f docker/docker-compose.yml run --rm test-native \
    bash -c "cd /workspace/build && ./test_native/unit_tests '[core]'"
```
