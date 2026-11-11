#include "serial_display_adapter.h"

#include "adapters/displays/i_telemetry_display.h"

#include <string>

namespace hrs::adapters::displays
{

#ifdef ARDUINO
SerialDisplayAdapter::SerialDisplayAdapter(Print& out, const float danger_distance_m, const float caution_distance_m)
    : ITelemetryDisplay(danger_distance_m, caution_distance_m)
    , out_(out)
{
}

bool SerialDisplayAdapter::init()
{
    out_.println(F("{\"type\":\"radar_status\",\"status\":\"online\"}"));
    return true;
}

void SerialDisplayAdapter::send_line(const std::string& line)
{
    out_.print(line.c_str());
}

#else

SerialDisplayAdapter::SerialDisplayAdapter(const float danger_distance_m, const float caution_distance_m)
    : ITelemetryDisplay(danger_distance_m, caution_distance_m)
{
}

bool SerialDisplayAdapter::init()
{
    return true;
}

void SerialDisplayAdapter::send_line(const std::string& /*line*/)
{
}

#endif

} // namespace hrs::adapters::displays
