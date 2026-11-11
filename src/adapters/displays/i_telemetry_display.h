#pragma once

#include "core/imu_data.h"
#include "core/target_data.h"
#include "core/threat_assessment.h"
#include "ports/i_display.h"

#include <string>
#include <vector>

namespace hrs::adapters::displays
{

/// Collects one frame of draw calls and emits it as the shared JSON telemetry line on flush().
/// Subclasses only decide where the line goes (serial console, TCP socket, ...).
class ITelemetryDisplay : public ports::IDisplay
{
public:
    ITelemetryDisplay(const float danger_distance_m, const float caution_distance_m);
    ~ITelemetryDisplay() override = default;

    void clear() override;
    void draw_sweep(const float angle_deg) override;
    void draw_radar(const std::vector<core::TargetData>& targets) override;
    void draw_alert(const core::ThreatAssessment& threat) override;
    void draw_attitude(const core::Orientation& orientation, const core::ImuData* imu) override;
    void flush() override;

protected:
    virtual void send_line(const std::string& line) = 0;

private:
    float danger_distance_m_;
    float caution_distance_m_;
    float sweep_angle_deg_{0.0f};
    core::ThreatAssessment threat_{};
    std::vector<core::TargetData> targets_{};
    core::Orientation orientation_{};
    core::ImuData imu_{};
    bool has_imu_{false};
};

} // namespace hrs::adapters::displays
