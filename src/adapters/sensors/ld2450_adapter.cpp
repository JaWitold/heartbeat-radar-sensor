#include "ld2450_adapter.h"

#include "core/sensor_specs.h"
#include "core/target_data.h"

#include <vector>

namespace hrs::adapters::sensors
{

#ifdef ARDUINO
LD2450Adapter::LD2450Adapter(HardwareSerial& serial, const int rx_pin, const int tx_pin, const uint32_t baud)
    : serial_(serial)
    , rx_pin_(rx_pin)
    , tx_pin_(tx_pin)
    , baud_rate_(baud)
{
}

bool LD2450Adapter::init()
{
    serial_.begin(baud_rate_, SERIAL_8N1, rx_pin_, tx_pin_);
    parser_.reset();
    has_frame_      = false;
    is_initialized_ = true;
    return true;
}

std::vector<core::TargetData> LD2450Adapter::scan()
{
    if (false == is_initialized_)
    {
        return {};
    }

    while (serial_.available() > 0)
    {
        const int byte = serial_.read();
        if (byte >= 0 && true == parser_.parse_byte(static_cast<uint8_t>(byte)))
        {
            has_frame_     = true;
            last_frame_ms_ = millis();
        }
    }

    if (false == is_connected())
    {
        return {};
    }
    return parser_.get_latest_targets();
}

bool LD2450Adapter::is_connected() const
{
    return true == is_initialized_ && true == has_frame_ && (millis() - last_frame_ms_) < SIGNAL_TIMEOUT_MS;
}

#else

bool LD2450Adapter::init()
{
    is_initialized_ = true;
    return true;
}

std::vector<core::TargetData> LD2450Adapter::scan()
{
    return parser_.get_latest_targets();
}

bool LD2450Adapter::is_connected() const
{
    return is_initialized_;
}

#endif

core::SensorSpecs LD2450Adapter::specs() const
{
    return SPECS;
}

} // namespace hrs::adapters::sensors
