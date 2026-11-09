#include "serial_display_adapter.h"

#include "core/target_data.h"
#include "core/threat_assessment.h"

#ifdef ARDUINO
#include "core/telemetry_frame.h"
#endif

#include <vector>

namespace hrs::adapters::displays
{

#ifdef ARDUINO
SerialDisplayAdapter::SerialDisplayAdapter(Print& out, const float danger_distance_m, const float caution_distance_m)
    : out_(out)
    , danger_distance_m_(danger_distance_m)
    , caution_distance_m_(caution_distance_m)
{
}

bool SerialDisplayAdapter::init()
{
    out_.println(F("{\"type\":\"radar_status\",\"status\":\"online\"}"));
    return true;
}

void SerialDisplayAdapter::clear()
{
    targets_.clear();
}

void SerialDisplayAdapter::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void SerialDisplayAdapter::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void SerialDisplayAdapter::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void SerialDisplayAdapter::flush()
{
    out_.print(
        core::encode_telemetry_frame(sweep_angle_deg_, threat_, targets_, danger_distance_m_, caution_distance_m_)
            .c_str());
}

#else

SerialDisplayAdapter::SerialDisplayAdapter() = default;

bool SerialDisplayAdapter::init()
{
    return true;
}

void SerialDisplayAdapter::clear()
{
    targets_.clear();
}

void SerialDisplayAdapter::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void SerialDisplayAdapter::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void SerialDisplayAdapter::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void SerialDisplayAdapter::flush()
{
}

#endif

} // namespace hrs::adapters::displays
