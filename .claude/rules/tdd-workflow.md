# Rule: Test-Driven Development (TDD) Workflow

1. **Red first:**
   - Failing unit test in `test_native/` before any production domain code.
   - Run `./scripts/test-native.sh`; verify test fails for expected reason.
2. **Green:**
   - Simplest direct code in `src/core/` to pass.
   - No speculative features, no premature abstractions.
3. **Refactor:**
   - Readability, SOLID, Yoda comparisons, const-correctness.
   - All unit tests stay green.
4. **No hardware in tests:**
   - Core logic 100% host-testable with mocks.
   - Never `<Arduino.h>` or ESP-IDF in tests or core.
