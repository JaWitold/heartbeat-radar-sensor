#pragma once

#include "ports/i_timer.h"

#include <cstdint>

namespace hrs::mocks
{

class MockTimer : public ports::ITimer
{
public:
    MockTimer()           = default;
    ~MockTimer() override = default;

    [[nodiscard]] uint32_t millis() const override
    {
        return current_millis_;
    }

    void delay_ms(const uint32_t ms) override
    {
        current_millis_ += ms;
        ++delay_call_count_;
    }

    void advance_ms(const uint32_t ms)
    {
        current_millis_ += ms;
    }

    void set_millis(const uint32_t ms)
    {
        current_millis_ = ms;
    }

    [[nodiscard]] uint32_t delay_call_count() const
    {
        return delay_call_count_;
    }

private:
    uint32_t current_millis_{0};
    uint32_t delay_call_count_{0};
};

} // namespace hrs::mocks
