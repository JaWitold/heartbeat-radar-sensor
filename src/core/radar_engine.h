#pragma once

#include "core/attitude_estimator.h"
#include "core/coordinate_transformer.h"
#include "core/imu_data.h"
#include "core/sensor_specs.h"
#include "core/target_data.h"
#include "core/threat_assessment.h"
#include "ports/i_buzzer.h"
#include "ports/i_display.h"
#include "ports/i_imu.h"
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
    RadarEngine(ports::ISensor& sensor,
                ports::IDisplay& display,
                ports::ITimer& timer,
                ports::IThreatEvaluator& threat_evaluator,
                ports::IBuzzer* buzzer = nullptr,
                ports::IImu* imu       = nullptr);

    bool init();
    void update();

    [[nodiscard]] size_t active_target_count() const;
    [[nodiscard]] ThreatLevel current_threat_level() const;
    [[nodiscard]] const ThreatAssessment& current_threat() const;
    [[nodiscard]] float current_sweep_angle() const;
    [[nodiscard]] float fov_min_deg() const;
    [[nodiscard]] float fov_max_deg() const;
    [[nodiscard]] SensorSpecs sensor_specs() const;

    [[nodiscard]] bool has_imu() const;
    [[nodiscard]] const ImuData& latest_imu() const;
    [[nodiscard]] const Orientation& current_orientation() const;

    [[nodiscard]] const std::vector<TargetData>& targets() const;
    [[nodiscard]] const std::vector<TargetData>& raw_targets() const;

    void set_attitude_compensation(const bool enabled);
    [[nodiscard]] bool is_attitude_compensated() const;

    void set_transform_config(const TransformConfig& config);
    [[nodiscard]] const TransformConfig& transform_config() const;

private:
    float update_sweep();
    void update_buzzer();

    ports::ISensor& sensor_;
    ports::IDisplay& display_;
    ports::ITimer& timer_;
    ports::IThreatEvaluator& threat_evaluator_;
    ports::IBuzzer* buzzer_;
    ports::IImu* imu_;
    AttitudeEstimator attitude_estimator_{};

    float fov_min_deg_{0.0f};
    float fov_max_deg_{0.0f};

    std::vector<TargetData> targets_{};
    std::vector<TargetData> raw_targets_{};
    ThreatAssessment threat_{};
    ImuData latest_imu_{};
    bool has_imu_data_{false};
    bool compensate_attitude_{true};
    TransformConfig transform_config_{};

    float sweep_angle_{0.0f};
    bool sweep_forward_{true};
    bool has_last_update_{false};
    uint32_t last_update_ms_{0};
    bool has_beeped_{false};
    uint32_t last_beep_ms_{0};
};

} // namespace hrs::core
