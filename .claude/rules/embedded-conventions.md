# Rule: Embedded C++ Conventions

1. **Language Standard:** C++17.
2. **Yoda Comparisons:** Always place constants or literals on the left-hand side of comparison operators:
   - `if (nullptr == pointer)` instead of `if (pointer == nullptr)`
   - `if (0 == count)` instead of `if (count == 0)`
   - `if (true == is_valid)` instead of `if (is_valid == true)`
3. **Memory Safety & RAII:**
   - Never use raw `new` or `delete`.
   - Prefer standard containers (`std::vector`, `std::array`) and smart pointers (`std::unique_ptr`).
4. **Const-Correctness:**
   - Mark local variables `const` by default unless mutation is needed.
   - Pass non-primitive parameters by `const &`.
   - Mark non-mutating member functions `const`.
5. **Brace Style:** Allman style (opening brace on a new line).
6. **No Magic Numbers:** Define named constants using `constexpr` or typed enums (`enum class`).
7. **Naming Rules:**
   - Classes & Structs: `PascalCase`
   - Abstract Interfaces: `I` prefix + `PascalCase` (`ISensor`, `IDisplay`)
   - Functions & Methods: `snake_case()`
   - Variables & Parameters: `snake_case`
   - Private Member Variables: `snake_case_` (ending with an underscore)
   - Constants & Enums: `UPPER_SNAKE_CASE`
