#pragma once

#include "ports/i_sensor.h"

#include <vector>

namespace hrs::mocks
{

class MockSensor : public ports::ISensor
{
public:
    MockSensor()           = default;
    ~MockSensor() override = default;

    bool init() override
    {
        init_called_ = true;
        return init_return_;
    }

    std::vector<core::TargetData> scan() override
    {
        ++scan_call_count_;
        return targets_;
    }

    [[nodiscard]] bool is_connected() const override
    {
        return is_connected_;
    }

    [[nodiscard]] core::SensorSpecs specs() const override
    {
        return specs_;
    }

    void set_specs(const core::SensorSpecs& specs)
    {
        specs_ = specs;
    }

    void set_init_return(const bool success)
    {
        init_return_ = success;
    }

    void set_connected(const bool connected)
    {
        is_connected_ = connected;
    }

    void set_targets(const std::vector<core::TargetData>& targets)
    {
        targets_ = targets;
    }

    [[nodiscard]] bool init_called() const
    {
        return init_called_;
    }

    [[nodiscard]] uint32_t scan_call_count() const
    {
        return scan_call_count_;
    }

private:
    bool init_called_{false};
    bool init_return_{true};
    bool is_connected_{true};
    uint32_t scan_call_count_{0};
    std::vector<core::TargetData> targets_{};
    core::SensorSpecs specs_{6.0f, -60.0f, 60.0f, 3, 1.5f, 3.5f}; // LD2450-like defaults
};

} // namespace hrs::mocks
