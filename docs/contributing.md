# Contributing Guide

## Process

1. **Issue:** create or pick GitHub issue.
2. **Branch from `main`:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feature/sweep-animation-decay
   ```
3. **TDD (Red → Green → Refactor):**
   - Failing test in `test_native/`
   - Minimal impl in `src/core/`
   - Verify: `./scripts/test-native.sh`
4. **Format & static analysis:**
   ```bash
   ./scripts/format.sh
   ./scripts/format-check.sh
   ./scripts/lint.sh
   ```
5. **Commit:** follow commit convention below.
6. **Pull request:**
   - Base: `main`
   - Fill `.github/PULL_REQUEST_TEMPLATE.md`
   - CI must pass (lint, test, build)
   - Squash merge, linear history.

---

## Commit Format

```
[Type] Short summary description (max 72 characters)

Optional detailed multi-line explanation of changes.
Wrapped at 72 characters per line.

Signed-off-by: Your Name <your.email@example.com>
```

### Types
- `[Feature]` — new functionality
- `[Fix]` — bug fix
- `[Docs]` — documentation
- `[Style]` — formatting, no logic change
- `[Refactor]` — restructure, no behavior change
- `[Test]` — tests
- `[Chore]` — build config, deps, CI
- `[Perf]` — performance

### Example
```
[Feature] Add multi-target aging to RadarEngine

Implements track decay when contacts are no longer seen by ISensor.
Targets fade out after 1500ms of signal loss.

Signed-off-by: JaWitold <jawitold@example.com>
```

---

## Coding Standards

- **Standard:** C++17
- **Braces:** Allman (opening brace on new line)
- **Comparisons:** Yoda (`if (nullptr == ptr)`, `if (0 == count)`)
- **Memory:** smart pointers (`std::unique_ptr`, `std::shared_ptr`) or references; no raw `new`/`delete`
- **Const:** variables, parameters, member functions `const` where possible
- **Architecture:** no `#include <Arduino.h>` or hardware drivers in `src/core/`
- **Naming:**
  - Classes & structs: `PascalCase`
  - Interfaces: `I` + `PascalCase` (`ISensor`, `IDisplay`)
  - Functions & methods: `snake_case()`
  - Variables & members: `snake_case`; private members suffix `_` (`sweep_angle_`)
  - Constants (`constexpr`, global/static): `UPPER_SNAKE_CASE`
  - Enum values: `PascalCase` (`ThreatLevel::Danger`)
