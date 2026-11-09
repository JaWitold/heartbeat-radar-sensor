#include "esp32_buzzer_adapter.h"

#include <cstdint>

namespace hrs::adapters::buzzers
{

ESP32BuzzerAdapter::ESP32BuzzerAdapter(const int pin)
    : pin_(pin)
{
#ifdef ARDUINO
    pinMode(pin_, OUTPUT);
    noTone(pin_);
#endif
}

ESP32BuzzerAdapter::~ESP32BuzzerAdapter()
{
    stop_hardware();
}

void ESP32BuzzerAdapter::beep(const uint16_t freq_hz, const uint16_t duration_ms)
{
    last_freq_hz_     = freq_hz;
    last_duration_ms_ = duration_ms;
    is_beeping_       = true;

#ifdef ARDUINO
    tone(pin_, freq_hz, duration_ms);
#endif
}

void ESP32BuzzerAdapter::stop()
{
    stop_hardware();
}

void ESP32BuzzerAdapter::stop_hardware()
{
    is_beeping_ = false;

#ifdef ARDUINO
    noTone(pin_);
#endif
}

bool ESP32BuzzerAdapter::is_beeping() const
{
    return is_beeping_;
}

uint16_t ESP32BuzzerAdapter::last_freq_hz() const
{
    return last_freq_hz_;
}

uint16_t ESP32BuzzerAdapter::last_duration_ms() const
{
    return last_duration_ms_;
}

int ESP32BuzzerAdapter::pin() const
{
    return pin_;
}

} // namespace hrs::adapters::buzzers
