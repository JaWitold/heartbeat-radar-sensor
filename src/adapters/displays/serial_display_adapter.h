#pragma once

#include "adapters/displays/i_telemetry_display.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <Print.h>
#endif

#include <string>

namespace hrs::adapters::displays
{

/// Streams the shared JSON telemetry line over the serial console.
class SerialDisplayAdapter : public ITelemetryDisplay
{
public:
#ifdef ARDUINO
    SerialDisplayAdapter(Print& out, const float danger_distance_m, const float caution_distance_m);
#else
    SerialDisplayAdapter(const float danger_distance_m, const float caution_distance_m);
#endif
    ~SerialDisplayAdapter() override = default;

    bool init() override;

protected:
    void send_line(const std::string& line) override;

#ifdef ARDUINO
private:
    Print& out_;
#endif
};

} // namespace hrs::adapters::displays
