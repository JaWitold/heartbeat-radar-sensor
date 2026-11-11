#pragma once

#include "core/imu_data.h"

namespace hrs::core
{

/// Complementary filter fusing accelerometer tilt angles and gyroscope angular rate.
class AttitudeEstimator
{
public:
    static constexpr float DEFAULT_ALPHA = 0.98f;

    explicit AttitudeEstimator(const float alpha = DEFAULT_ALPHA);

    void reset();
    void update(const ImuData& imu, const float dt_s);

    [[nodiscard]] const Orientation& orientation() const;
    [[nodiscard]] float pitch_deg() const;
    [[nodiscard]] float roll_deg() const;
    [[nodiscard]] float yaw_deg() const;

private:
    float alpha_{DEFAULT_ALPHA};
    Orientation orientation_{};
    bool is_initialized_{false};
};

} // namespace hrs::core
