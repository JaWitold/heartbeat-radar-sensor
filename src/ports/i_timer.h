#pragma once

#include <cstdint>

namespace hrs::ports
{

class ITimer
{
public:
    virtual ~ITimer() = default;

    [[nodiscard]] virtual uint32_t millis() const = 0;
    virtual void delay_ms(const uint32_t ms)      = 0;
};

} // namespace hrs::ports
