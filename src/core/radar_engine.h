#pragma once

#include "core/sensor_specs.h"
#include "core/target_data.h"
#include "core/threat_assessment.h"
#include "ports/i_buzzer.h"
#include "ports/i_display.h"
#include "ports/i_sensor.h"
#include "ports/i_threat_evaluator.h"
#include "ports/i_timer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hrs::core
{

class RadarEngine
{
public:
    static constexpr float AUTO_FOV = 9999.0f;

    RadarEngine(ports::ISensor& sensor,
                ports::IDisplay& display,
                ports::ITimer& timer,
                ports::IThreatEvaluator& threat_evaluator,
                ports::IBuzzer* buzzer  = nullptr,
                const float fov_min_deg = AUTO_FOV,
                const float fov_max_deg = AUTO_FOV);

    bool init();
    void update();

    [[nodiscard]] size_t active_target_count() const;
    [[nodiscard]] ThreatLevel current_threat_level() const;
    [[nodiscard]] const ThreatAssessment& current_threat() const;
    [[nodiscard]] float current_sweep_angle() const;
    [[nodiscard]] float fov_min_deg() const;
    [[nodiscard]] float fov_max_deg() const;
    [[nodiscard]] SensorSpecs sensor_specs() const;

private:
    void update_sweep();
    void update_buzzer();

    ports::ISensor& sensor_;
    ports::IDisplay& display_;
    ports::ITimer& timer_;
    ports::IThreatEvaluator& threat_evaluator_;
    ports::IBuzzer* buzzer_;

    bool has_custom_fov_{false};
    float fov_min_deg_{0.0f};
    float fov_max_deg_{0.0f};

    std::vector<TargetData> targets_{};
    ThreatAssessment threat_{};
    float sweep_angle_{0.0f};
    bool sweep_forward_{true};
    bool has_last_update_{false};
    uint32_t last_update_ms_{0};
    bool has_beeped_{false};
    uint32_t last_beep_ms_{0};
};

} // namespace hrs::core
