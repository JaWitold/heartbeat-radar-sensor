#pragma once

#include "core/imu_data.h"
#include "core/target_data.h"
#include "core/threat_assessment.h"
#include "ports/i_display.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <TFT_eSPI.h>
#endif

#include <vector>

namespace hrs::adapters::displays
{

class ST7789Adapter : public ports::IDisplay
{
public:
    ST7789Adapter()           = default;
    ~ST7789Adapter() override = default;

    bool init() override;
    void clear() override;
    void draw_sweep(const float angle_deg) override;
    void draw_radar(const std::vector<core::TargetData>& targets) override;
    void draw_alert(const core::ThreatAssessment& threat) override;
    void draw_attitude(const core::Orientation& orientation, const core::ImuData* imu) override;
    void flush() override;

private:
#ifdef ARDUINO
    TFT_eSPI tft_{};
#endif
    float sweep_angle_deg_{0.0f};
    core::ThreatAssessment threat_{};
    std::vector<core::TargetData> targets_{};
};

} // namespace hrs::adapters::displays
