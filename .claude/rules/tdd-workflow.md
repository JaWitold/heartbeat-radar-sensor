# Rule: Test-Driven Development (TDD) Workflow

1. **Red Phase First:**
   - Always write a failing unit test in `test_native/` before writing any production domain code.
   - Run `./scripts/test-native.sh` and verify the test fails for the expected reason.
2. **Green Phase:**
   - Write the simplest, most direct code in `src/core/` to make the test pass.
   - Do not add speculative features or premature abstractions.
3. **Refactor Phase:**
   - Refactor for readability, SOLID principles, Yoda comparisons, and const-correctness.
   - Ensure all unit tests remain green after refactoring.
4. **No Direct Hardware in Tests:**
   - Core domain logic must be testable 100% on the host machine using mocks.
   - Never introduce dependencies on `<Arduino.h>` or ESP-IDF in tests or core domain code.
