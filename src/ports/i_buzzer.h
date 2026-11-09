#pragma once

#include <cstdint>

namespace hrs::ports
{

class IBuzzer
{
public:
    virtual ~IBuzzer() = default;

    virtual void beep(const uint16_t freq_hz, const uint16_t duration_ms) = 0;
    virtual void stop()                                                   = 0;
};

} // namespace hrs::ports
