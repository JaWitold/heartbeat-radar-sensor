#pragma once

#include "ports/i_timer.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <chrono>
#include <thread>
#endif

namespace hrs::adapters::timers
{

class ESP32TimerAdapter : public ports::ITimer
{
public:
    ESP32TimerAdapter()           = default;
    ~ESP32TimerAdapter() override = default;

    [[nodiscard]] uint32_t millis() const override
    {
#ifdef ARDUINO
        return ::millis();
#else
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
#endif
    }

    void delay_ms(const uint32_t ms) override
    {
#ifdef ARDUINO
        ::delay(ms);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
    }
};

} // namespace hrs::adapters::timers
