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
#ifndef SENSOR_UART_RX_PIN
    static constexpr int DEFAULT_RX_PIN = 16;
#else
    static constexpr int DEFAULT_RX_PIN = SENSOR_UART_RX_PIN;
#endif

#ifndef SENSOR_UART_TX_PIN
    static constexpr int DEFAULT_TX_PIN = 17;
#else
    static constexpr int DEFAULT_TX_PIN = SENSOR_UART_TX_PIN;
#endif

#ifndef SENSOR_UART_BAUD_RATE
    static constexpr uint32_t DEFAULT_BAUD_RATE = 256000;
#else
    static constexpr uint32_t DEFAULT_BAUD_RATE = SENSOR_UART_BAUD_RATE;
#endif

    /// HLK-LD2450: 24GHz FMCW mmWave, 120° FoV (±60°), 6m range, 3 targets.
    static constexpr core::SensorSpecs SPECS{6.0f, -60.0f, 60.0f, 3, 1.5f, 3.5f};

#ifdef ARDUINO
    explicit LD2450Adapter(HardwareSerial& serial = Serial2,
                           const int rx_pin       = DEFAULT_RX_PIN,
                           const int tx_pin       = DEFAULT_TX_PIN,
                           const uint32_t baud    = DEFAULT_BAUD_RATE);
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
    int rx_pin_{DEFAULT_RX_PIN};
    int tx_pin_{DEFAULT_TX_PIN};
    uint32_t baud_rate_{DEFAULT_BAUD_RATE};
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
