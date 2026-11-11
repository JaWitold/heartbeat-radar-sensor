# Rule: Embedded C++ Conventions

1. **Language standard:** C++17 (`-std=gnu++17` on ESP32, `-std=c++17` host tests).
2. **Yoda comparisons:** constant/literal on left of comparison:
   - `if (nullptr == pointer)` not `if (pointer == nullptr)`
   - `if (0 == count)` not `if (count == 0)`
   - `if (true == is_valid)` not `if (is_valid == true)`
3. **Memory safety & RAII:**
   - Never raw `new` / `delete`.
   - Prefer standard containers (`std::vector`, `std::array`) + smart pointers (`std::unique_ptr`).
4. **Const-correctness:**
   - Locals `const` by default unless mutated.
   - Non-primitive params by `const &`.
   - Non-mutating member functions `const`.
5. **Brace style:** Allman (opening brace on new line).
6. **No magic numbers:** named constants via `constexpr` or `enum class`. Pins + buses = board wiring macros in `platformio.ini`, never literals in adapters (`adapter-config.md`).
7. **Naming:**
   - Classes & structs: `PascalCase`
   - Abstract interfaces: `I` prefix + `PascalCase` (`ISensor`, `IDisplay`)
   - Functions & methods: `snake_case()`
   - Variables & params: `snake_case`
   - Private members: `snake_case_` (trailing underscore)
   - Constants (`constexpr`, global/static): `UPPER_SNAKE_CASE`
   - Enum values: `PascalCase` (`ThreatLevel::Danger`, `ThreatLevel::NoSignal`)
   - `const` locals + params: `snake_case`
8. **Binary wire format pitfalls:**
   - Always verify signed integer format in radar datasheets. Many mmWave sensors (e.g. HLK-LD2450) use **sign-magnitude** (bit 15 set = positive, clear = negative; magnitude in bits 0..14), not two's complement.
   - Serial streams: never assume newline `0x0A` = line terminator without checking for preceding binary frame headers. Payload bytes often contain `0x0A`.
9. **Audio alert cadence:**
   - Never trigger buzzer/tones synchronously every engine tick.
   - Pace alert pulses with explicit interval timers (e.g. 250ms Danger heartbeat, 1000ms Caution). Avoids re-triggering tone generator every 20ms frame.
10. **Signal loss & timeouts:**
    - Sensor adapters implement connection timeout (`SIGNAL_TIMEOUT_MS`).
    - Frames stop → report `is_connected() == false`, clear stale targets, go to `ThreatLevel::NoSignal`.
