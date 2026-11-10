#pragma once

#include "core/ld2450_parser.h"
#include "ports/i_sensor.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <HardwareSerial.h>
#endif

namespace hrs::adapters::sensors
{

class LD2450Adapter : public ports::ISensor
{
public:
    /// LD2450 factory UART rate. Override with -DLD2450_BAUD_RATE (read in main.cpp only).
    static constexpr uint32_t DEFAULT_BAUD_RATE = 256000;

    /// HLK-LD2450: 24GHz FMCW mmWave, 120° FoV (±60°), 6m range, 3 targets.
    static constexpr core::SensorSpecs SPECS{6.0f, -60.0f, 60.0f, 3, 1.5f, 3.5f};

#ifdef ARDUINO
    /// Pins are board wiring: the composition root passes them from the board env.
    LD2450Adapter(HardwareSerial& serial, const int rx_pin, const int tx_pin, const uint32_t baud = DEFAULT_BAUD_RATE);
#else
    LD2450Adapter() = default;
#endif

    ~LD2450Adapter() override = default;

    bool init() override;
    std::vector<core::TargetData> scan() override;
    [[nodiscard]] core::SensorSpecs specs() const override;
    [[nodiscard]] bool is_connected() const override;

private:
#ifdef ARDUINO
    HardwareSerial& serial_;
    int rx_pin_;
    int tx_pin_;
    uint32_t baud_rate_;
#endif
    core::LD2450Parser parser_{};
    bool is_initialized_{false};
#ifdef ARDUINO
    static constexpr uint32_t SIGNAL_TIMEOUT_MS = 500; // ~5 missed frames at the LD2450's ~10 Hz rate
    bool has_frame_{false};
    uint32_t last_frame_ms_{0};
#endif
};

} // namespace hrs::adapters::sensors
