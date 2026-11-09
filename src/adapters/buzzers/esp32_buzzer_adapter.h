#pragma once

#include "ports/i_buzzer.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace hrs::adapters::buzzers
{

class ESP32BuzzerAdapter : public ports::IBuzzer
{
public:
#ifndef BUZZER_PIN
    static constexpr int DEFAULT_BUZZER_PIN = 25;
#else
    static constexpr int DEFAULT_BUZZER_PIN = BUZZER_PIN;
#endif

    explicit ESP32BuzzerAdapter(const int pin = DEFAULT_BUZZER_PIN);
    ~ESP32BuzzerAdapter() override;

    // Owns the buzzer pin: a copy's destructor would silence the original.
    ESP32BuzzerAdapter(const ESP32BuzzerAdapter&)            = delete;
    ESP32BuzzerAdapter& operator=(const ESP32BuzzerAdapter&) = delete;
    ESP32BuzzerAdapter(ESP32BuzzerAdapter&&)                 = delete;
    ESP32BuzzerAdapter& operator=(ESP32BuzzerAdapter&&)      = delete;

    void beep(const uint16_t freq_hz, const uint16_t duration_ms) override;
    void stop() override;

    [[nodiscard]] bool is_beeping() const;
    [[nodiscard]] uint16_t last_freq_hz() const;
    [[nodiscard]] uint16_t last_duration_ms() const;
    [[nodiscard]] int pin() const;

private:
    void stop_hardware();

    int pin_{DEFAULT_BUZZER_PIN};
    bool is_beeping_{false};
    uint16_t last_freq_hz_{0};
    uint16_t last_duration_ms_{0};
};

} // namespace hrs::adapters::buzzers
