#pragma once

#include "core/target_data.h"
#include "core/threat_assessment.h"
#include "ports/i_display.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <Print.h>
#else
#include <iosfwd>
#endif

#include <vector>

namespace hrs::adapters::displays
{

class SerialDisplayAdapter : public ports::IDisplay
{
public:
#ifdef ARDUINO
    SerialDisplayAdapter(Print& out, const float danger_distance_m, const float caution_distance_m);
#else
    SerialDisplayAdapter();
#endif
    ~SerialDisplayAdapter() override = default;

    bool init() override;
    void clear() override;
    void draw_sweep(const float angle_deg) override;
    void draw_radar(const std::vector<core::TargetData>& targets) override;
    void draw_alert(const core::ThreatAssessment& threat) override;
    void flush() override;

private:
#ifdef ARDUINO
    Print& out_;
    float danger_distance_m_;
    float caution_distance_m_;
#endif
    float sweep_angle_deg_{0.0f};
    core::ThreatAssessment threat_{};
    std::vector<core::TargetData> targets_{};
};

} // namespace hrs::adapters::displays
