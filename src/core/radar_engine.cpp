#include "radar_engine.h"

#include "core/sensor_specs.h"
#include "core/threat_assessment.h"
#include "ports/i_buzzer.h"
#include "ports/i_display.h"
#include "ports/i_sensor.h"
#include "ports/i_threat_evaluator.h"
#include "ports/i_timer.h"

#include <cstddef>
#include <cstdint>

namespace hrs::core
{

namespace
{
constexpr float SWEEP_SPEED_DEG_PER_SEC = 120.0f;

constexpr uint32_t DANGER_BEEP_INTERVAL_MS  = 250;
constexpr uint32_t CAUTION_BEEP_INTERVAL_MS = 1000;
constexpr uint16_t DANGER_BEEP_FREQ_HZ      = 2500;
constexpr uint16_t DANGER_BEEP_DURATION_MS  = 50;
constexpr uint16_t CAUTION_BEEP_FREQ_HZ     = 1500;
constexpr uint16_t CAUTION_BEEP_DURATION_MS = 20;
} // namespace

RadarEngine::RadarEngine(ports::ISensor& sensor,
                         ports::IDisplay& display,
                         ports::ITimer& timer,
                         ports::IThreatEvaluator& threat_evaluator,
                         ports::IBuzzer* buzzer,
                         const float fov_min_deg,
                         const float fov_max_deg)
    : sensor_(sensor)
    , display_(display)
    , timer_(timer)
    , threat_evaluator_(threat_evaluator)
    , buzzer_(buzzer)
    , has_custom_fov_(AUTO_FOV != fov_min_deg && AUTO_FOV != fov_max_deg)
{
    if (true == has_custom_fov_)
    {
        fov_min_deg_ = fov_min_deg;
        fov_max_deg_ = fov_max_deg;
    }
    else
    {
        const SensorSpecs specs = sensor_.specs();
        fov_min_deg_            = specs.fov_min_deg;
        fov_max_deg_            = specs.fov_max_deg;
    }
    sweep_angle_ = fov_min_deg_;
}

bool RadarEngine::init()
{
    const bool sensor_ok  = sensor_.init();
    const bool display_ok = display_.init();

    if (nullptr != buzzer_)
    {
        buzzer_->stop();
    }

    last_update_ms_  = timer_.millis();
    has_last_update_ = true;

    return sensor_ok && display_ok;
}

void RadarEngine::update()
{
    update_sweep();

    targets_ = sensor_.scan();
    if (true == sensor_.is_connected())
    {
        threat_ = threat_evaluator_.evaluate(targets_);
    }
    else
    {
        targets_.clear();
        threat_ = ThreatAssessment{ThreatLevel::NoSignal};
    }

    display_.clear();
    display_.draw_sweep(sweep_angle_);
    display_.draw_radar(targets_);
    display_.draw_alert(threat_);
    display_.flush();

    update_buzzer();
}

void RadarEngine::update_buzzer()
{
    if (nullptr == buzzer_)
    {
        return;
    }

    const bool is_danger  = (ThreatLevel::Danger == threat_.level || ThreatLevel::Critical == threat_.level);
    const bool is_caution = (ThreatLevel::Caution == threat_.level);
    if (false == is_danger && false == is_caution)
    {
        has_beeped_ = false; // next threat beeps immediately
        return;
    }

    const uint32_t now      = timer_.millis();
    const uint32_t interval = (true == is_danger) ? DANGER_BEEP_INTERVAL_MS : CAUTION_BEEP_INTERVAL_MS;
    if (true == has_beeped_ && (now - last_beep_ms_) < interval)
    {
        return;
    }

    if (true == is_danger)
    {
        buzzer_->beep(DANGER_BEEP_FREQ_HZ, DANGER_BEEP_DURATION_MS);
    }
    else
    {
        buzzer_->beep(CAUTION_BEEP_FREQ_HZ, CAUTION_BEEP_DURATION_MS);
    }
    last_beep_ms_ = now;
    has_beeped_   = true;
}

size_t RadarEngine::active_target_count() const
{
    return targets_.size();
}

ThreatLevel RadarEngine::current_threat_level() const
{
    return threat_.level;
}

const ThreatAssessment& RadarEngine::current_threat() const
{
    return threat_;
}

float RadarEngine::current_sweep_angle() const
{
    return sweep_angle_;
}

float RadarEngine::fov_min_deg() const
{
    return fov_min_deg_;
}

float RadarEngine::fov_max_deg() const
{
    return fov_max_deg_;
}

SensorSpecs RadarEngine::sensor_specs() const
{
    return sensor_.specs();
}

void RadarEngine::update_sweep()
{
    const uint32_t now = timer_.millis();

    if (false == has_last_update_)
    {
        has_last_update_ = true;
        last_update_ms_  = now;
        return;
    }

    const uint32_t dt_ms = now - last_update_ms_;
    last_update_ms_      = now;

    const float dt_s      = static_cast<float>(dt_ms) / 1000.0f;
    const float delta_deg = dt_s * SWEEP_SPEED_DEG_PER_SEC;

    if (true == sweep_forward_)
    {
        sweep_angle_ += delta_deg;
        if (sweep_angle_ >= fov_max_deg_)
        {
            sweep_angle_   = fov_max_deg_;
            sweep_forward_ = false;
        }
    }
    else
    {
        sweep_angle_ -= delta_deg;
        if (sweep_angle_ <= fov_min_deg_)
        {
            sweep_angle_   = fov_min_deg_;
            sweep_forward_ = true;
        }
    }
}

} // namespace hrs::core
