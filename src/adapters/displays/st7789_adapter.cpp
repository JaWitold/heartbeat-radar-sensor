#include "st7789_adapter.h"

namespace hrs::adapters::displays
{

#ifdef ARDUINO

bool ST7789Adapter::init()
{
    tft_.init();
    tft_.setRotation(0);
    tft_.fillScreen(TFT_BLACK);
    return true;
}

void ST7789Adapter::clear()
{
    targets_.clear();
}

void ST7789Adapter::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void ST7789Adapter::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void ST7789Adapter::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void ST7789Adapter::flush()
{
    // Basic HUD rendering stub: clear and draw crosshair / sweep indicator
    // Full graphical renderer can be expanded in TFT feature phase
}

#else

bool ST7789Adapter::init()
{
    return true;
}

void ST7789Adapter::clear()
{
}

void ST7789Adapter::draw_sweep(const float /*angle_deg*/)
{
}

void ST7789Adapter::draw_radar(const std::vector<core::TargetData>& /*targets*/)
{
}

void ST7789Adapter::draw_alert(const core::ThreatAssessment& /*threat*/)
{
}

void ST7789Adapter::flush()
{
}

#endif

} // namespace hrs::adapters::displays
