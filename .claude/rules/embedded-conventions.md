# Rule: Embedded C++ Conventions

1. **Language Standard:** C++17 (`-std=gnu++17` on ESP32, `-std=c++17` for host tests).
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
   - Constants (`constexpr`, global/static): `UPPER_SNAKE_CASE`
   - Enum values: `PascalCase` (`ThreatLevel::Danger`, `ThreatLevel::NoSignal`)
   - `const` locals and parameters: `snake_case`
8. **Binary Wire Format Pitfalls:**
   - Always verify signed integer formats in radar datasheets: many mmWave sensors (like HLK-LD2450) use **sign-magnitude** (bit 15 set = positive, clear = negative; magnitude in bits 0..14) rather than standard two's complement.
   - When streaming over serial, never assume newline `0x0A` is a line terminator without checking for preceding binary frame headers; payload bytes frequently contain `0x0A`.
9. **Audio Alert Cadence:**
   - Never trigger buzzer hardware or audio tones synchronously on every engine update tick.
   - Always pace alert pulses with explicit interval timers (e.g. 250ms heartbeat for Danger, 1000ms for Caution) to avoid re-triggering tone generators every 20ms frame.
10. **Signal Loss & Timeouts:**
    - Sensor adapters must implement connection timeouts (`SIGNAL_TIMEOUT_MS`).
    - If incoming frames cease, report `is_connected() == false`, clear stale targets, and transition to `ThreatLevel::NoSignal`.
