#pragma once

#include "ports/i_display.h"

#include <vector>

namespace hrs::mocks
{

class MockDisplay : public ports::IDisplay
{
public:
    MockDisplay()           = default;
    ~MockDisplay() override = default;

    bool init() override
    {
        init_called_ = true;
        return init_return_;
    }

    void clear() override
    {
        ++clear_call_count_;
    }

    void draw_sweep(const float angle_deg) override
    {
        ++draw_sweep_call_count_;
        last_sweep_angle_ = angle_deg;
    }

    void draw_radar(const std::vector<core::TargetData>& targets) override
    {
        ++draw_radar_call_count_;
        last_targets_ = targets;
    }

    void draw_alert(const core::ThreatAssessment& threat) override
    {
        ++draw_alert_call_count_;
        last_threat_ = threat;
    }

    void draw_attitude(const core::Orientation& orientation, const core::ImuData* imu) override
    {
        ++draw_attitude_call_count_;
        last_orientation_ = orientation;
        if (nullptr != imu)
        {
            last_imu_ = *imu;
            has_imu_  = true;
        }
        else
        {
            has_imu_ = false;
        }
    }

    void flush() override
    {
        ++flush_call_count_;
    }

    void set_init_return(const bool success)
    {
        init_return_ = success;
    }

    [[nodiscard]] bool init_called() const
    {
        return init_called_;
    }

    [[nodiscard]] uint32_t clear_call_count() const
    {
        return clear_call_count_;
    }

    [[nodiscard]] uint32_t draw_sweep_call_count() const
    {
        return draw_sweep_call_count_;
    }

    [[nodiscard]] float last_sweep_angle() const
    {
        return last_sweep_angle_;
    }

    [[nodiscard]] uint32_t draw_radar_call_count() const
    {
        return draw_radar_call_count_;
    }

    [[nodiscard]] const std::vector<core::TargetData>& last_targets() const
    {
        return last_targets_;
    }

    [[nodiscard]] uint32_t draw_alert_call_count() const
    {
        return draw_alert_call_count_;
    }

    [[nodiscard]] uint32_t draw_attitude_call_count() const
    {
        return draw_attitude_call_count_;
    }

    [[nodiscard]] const core::Orientation& last_orientation() const
    {
        return last_orientation_;
    }

    [[nodiscard]] const core::ImuData& last_imu() const
    {
        return last_imu_;
    }

    [[nodiscard]] bool has_imu() const
    {
        return has_imu_;
    }

    [[nodiscard]] const core::ThreatAssessment& last_threat() const
    {
        return last_threat_;
    }

    [[nodiscard]] uint32_t flush_call_count() const
    {
        return flush_call_count_;
    }

private:
    bool init_called_{false};
    bool init_return_{true};
    uint32_t clear_call_count_{0};
    uint32_t draw_sweep_call_count_{0};
    float last_sweep_angle_{0.0f};
    uint32_t draw_radar_call_count_{0};
    std::vector<core::TargetData> last_targets_{};
    uint32_t draw_alert_call_count_{0};
    core::ThreatAssessment last_threat_{};
    uint32_t draw_attitude_call_count_{0};
    core::Orientation last_orientation_{};
    core::ImuData last_imu_{};
    bool has_imu_{false};
    uint32_t flush_call_count_{0};
};

} // namespace hrs::mocks
