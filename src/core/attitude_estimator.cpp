#include "attitude_estimator.h"

#include "imu_data.h"

#include <cmath>

namespace hrs::core
{

namespace
{

constexpr float RAD_TO_DEG = 57.29577951308232f;

} // namespace

AttitudeEstimator::AttitudeEstimator(const float alpha)
    : alpha_(alpha)
{
}

void AttitudeEstimator::reset()
{
    orientation_    = Orientation{};
    is_initialized_ = false;
}

void AttitudeEstimator::update(const ImuData& imu, const float dt_s)
{
    if (false == imu.is_valid || 0.0f >= dt_s)
    {
        return;
    }

    const float ax = imu.accel.x;
    const float ay = imu.accel.y;
    const float az = imu.accel.z;

    // Pitch: elevation angle around lateral axis (positive = nose up)
    // Roll:  bank angle around longitudinal axis (positive = bank right)
    const float roll_accel  = std::atan2(ay, az) * RAD_TO_DEG;
    const float pitch_accel = std::atan2(-ax, std::sqrt(ay * ay + az * az)) * RAD_TO_DEG;

    if (false == is_initialized_)
    {
        orientation_.pitch_deg = pitch_accel;
        orientation_.roll_deg  = roll_accel;
        orientation_.yaw_deg   = 0.0f;
        is_initialized_        = true;
        return;
    }

    // Complementary filter: high-pass gyro for responsiveness, low-pass accel for drift compensation
    orientation_.roll_deg  = alpha_ * (orientation_.roll_deg + imu.gyro.x * dt_s) + (1.0f - alpha_) * roll_accel;
    orientation_.pitch_deg = alpha_ * (orientation_.pitch_deg + imu.gyro.y * dt_s) + (1.0f - alpha_) * pitch_accel;
    orientation_.yaw_deg += imu.gyro.z * dt_s;

    // Normalize yaw to [-180, +180]
    while (180.0f < orientation_.yaw_deg)
    {
        orientation_.yaw_deg -= 360.0f;
    }
    while (-180.0f > orientation_.yaw_deg)
    {
        orientation_.yaw_deg += 360.0f;
    }
}

const Orientation& AttitudeEstimator::orientation() const
{
    return orientation_;
}

float AttitudeEstimator::pitch_deg() const
{
    return orientation_.pitch_deg;
}

float AttitudeEstimator::roll_deg() const
{
    return orientation_.roll_deg;
}

float AttitudeEstimator::yaw_deg() const
{
    return orientation_.yaw_deg;
}

} // namespace hrs::core
