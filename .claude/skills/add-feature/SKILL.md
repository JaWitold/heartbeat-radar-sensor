---
name: add-feature
description: Guide TDD-driven implementation of a new feature for the Heartbeat Radar Sensor while adhering to hexagonal architecture and coding standards.
---

# Skill: Add Feature

## Purpose
Enforces the professional development standard when implementing a new capability or sensor/display adapter.

## Step-by-Step Procedure

### 1. Identify Layer and Interfaces
- If adding domain logic (e.g. contact persistence, sweep calculations): add to `src/core/`.
- If introducing external hardware (e.g. new sensor, sound generator):
  1. Define or verify the abstract interface in `src/ports/`.
  2. Implement a mock in `test_native/mocks/`.
  3. Implement the concrete hardware adapter in `src/adapters/`.

### 2. Red Phase (Write Failing Test)
Create or update a test in `test_native/core/`:
- Use Catch2 BDD syntax (`SCENARIO`, `GIVEN`, `WHEN`, `THEN`).
- Ensure comparisons follow Yoda style (`REQUIRE(0 == target_count)`).
- Run:
  ```bash
  ./scripts/test-native.sh
  ```
- Confirm the test fails.

### 3. Green Phase (Pass Minimal Implementation)
- Write the minimal code in `src/core/` to make the test pass.
- Run `./scripts/test-native.sh` until all tests pass.

### 4. Refactor Phase
- Check const-correctness, Yoda comparisons, and memory safety.
- Run format and lint checks:
  ```bash
  ./scripts/format.sh
  ./scripts/lint.sh
  ```

### 5. Commit
- Commit with conventional format:
  ```
  [Feature] Short summary

  Detailed explanation.

  Signed-off-by: JaWitold <email>
  ```
