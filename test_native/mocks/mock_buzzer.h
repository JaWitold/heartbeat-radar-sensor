#pragma once

#include "ports/i_buzzer.h"

#include <cstdint>

namespace hrs::mocks
{

class MockBuzzer : public ports::IBuzzer
{
public:
    MockBuzzer()           = default;
    ~MockBuzzer() override = default;

    void beep(const uint16_t freq_hz, const uint16_t duration_ms) override
    {
        ++beep_call_count_;
        last_freq_hz_     = freq_hz;
        last_duration_ms_ = duration_ms;
        is_beeping_       = true;
    }

    void stop() override
    {
        ++stop_call_count_;
        is_beeping_ = false;
    }

    [[nodiscard]] uint32_t beep_call_count() const
    {
        return beep_call_count_;
    }

    [[nodiscard]] uint32_t stop_call_count() const
    {
        return stop_call_count_;
    }

    [[nodiscard]] uint16_t last_freq_hz() const
    {
        return last_freq_hz_;
    }

    [[nodiscard]] uint16_t last_duration_ms() const
    {
        return last_duration_ms_;
    }

    [[nodiscard]] bool is_beeping() const
    {
        return is_beeping_;
    }

private:
    uint32_t beep_call_count_{0};
    uint32_t stop_call_count_{0};
    uint16_t last_freq_hz_{0};
    uint16_t last_duration_ms_{0};
    bool is_beeping_{false};
};

} // namespace hrs::mocks
