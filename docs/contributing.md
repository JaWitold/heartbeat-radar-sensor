# Contributing Guide

## Development Process

1. **Create an Issue:** Start by creating or picking an issue on GitHub.
2. **Branch from `main`:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feature/sweep-animation-decay
   ```
3. **Follow TDD (Red-Green-Refactor):**
   - Write a failing test in `test_native/`
   - Implement minimal functionality in `src/core/`
   - Verify tests pass: `./scripts/test-native.sh`
4. **Format & Static Analysis:**
   ```bash
   ./scripts/format.sh
   ./scripts/format-check.sh
   ./scripts/lint.sh
   ```
5. **Commit Following the Commit Convention:**
   Ensure each commit follows the required template.
6. **Open Pull Request:**
   - Base branch: `main`
   - Fill out `.github/PULL_REQUEST_TEMPLATE.md`
   - All CI checks must pass (Lint, Test, Build)
   - PR will be squash merged with linear history.

---

## Commit Guidelines

All commits must follow this structure:

```
[Type] Short summary description (max 72 characters)

Optional detailed multi-line explanation of changes.
Wrapped at 72 characters per line.

Signed-off-by: Your Name <your.email@example.com>
```

### Commit Types:
- `[Feature]` — New capability or functionality
- `[Fix]` — Bug fix
- `[Docs]` — Documentation changes
- `[Style]` — Code styling or formatting (no logic change)
- `[Refactor]` — Restructuring code without changing behavior
- `[Test]` — Adding or updating test suites
- `[Chore]` — Build configuration, dependencies, CI tooling
- `[Perf]` — Performance optimization

### Example Commit:
```
[Feature] Add multi-target aging to RadarEngine

Implements track decay when contacts are no longer seen by ISensor.
Targets fade out after 1500ms of signal loss.

Signed-off-by: JaWitold <jawitold@example.com>
```

---

## Coding Standards

- **Language Standard:** C++17
- **Braces:** Allman style (opening brace on a new line)
- **Comparisons:** Yoda style (`if (nullptr == ptr)`, `if (0 == count)`)
- **Memory Safety:** Smart pointers (`std::unique_ptr`, `std::shared_ptr`) or references; zero raw `new`/`delete`
- **Const Correctness:** Variables, parameters, and member functions must be `const` whenever possible
- **Architecture Integrity:** Zero `#include <Arduino.h>` or hardware drivers inside `src/core/`
- **Naming Conventions:**
  - Classes & Structs: `PascalCase`
  - Interfaces: `I` prefix + `PascalCase` (`ISensor`, `IDisplay`)
  - Functions & Methods: `snake_case()`
  - Variables & Members: `snake_case`, private member variables suffix `_` (e.g. `sweep_angle_`)
  - Constants & Enums: `UPPER_SNAKE_CASE`
