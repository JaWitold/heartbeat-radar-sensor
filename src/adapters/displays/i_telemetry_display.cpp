#include "i_telemetry_display.h"

#include "core/imu_data.h"
#include "core/target_data.h"
#include "core/telemetry_frame.h"
#include "core/threat_assessment.h"

#include <vector>

namespace hrs::adapters::displays
{

ITelemetryDisplay::ITelemetryDisplay(const float danger_distance_m, const float caution_distance_m)
    : danger_distance_m_(danger_distance_m)
    , caution_distance_m_(caution_distance_m)
{
}

void ITelemetryDisplay::clear()
{
    targets_.clear();
}

void ITelemetryDisplay::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void ITelemetryDisplay::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void ITelemetryDisplay::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void ITelemetryDisplay::draw_attitude(const core::Orientation& orientation, const core::ImuData* const imu)
{
    orientation_ = orientation;
    has_imu_     = (nullptr != imu && true == imu->is_valid);
    if (true == has_imu_)
    {
        imu_ = *imu;
    }
}

void ITelemetryDisplay::flush()
{
    send_line(core::encode_telemetry_frame(sweep_angle_deg_,
                                           threat_,
                                           targets_,
                                           danger_distance_m_,
                                           caution_distance_m_,
                                           has_imu_ ? &imu_ : nullptr,
                                           has_imu_ ? &orientation_ : nullptr));
}

} // namespace hrs::adapters::displays
