#pragma once

#include <cmath>

namespace hrs::core
{

/// 3-axis linear acceleration expressed in g (1g = 9.80665 m/s^2).
struct AccelData
{
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};

    [[nodiscard]] float magnitude_g() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }
};

/// 3-axis angular velocity expressed in degrees per second (deg/s).
struct GyroData
{
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

/// Aggregate container for 6-DOF inertial measurement unit reading and on-die temperature.
struct ImuData
{
    AccelData accel{};
    GyroData gyro{};
    float temp_c{0.0f};
    bool is_valid{false};
};

/// 3-DOF device orientation / attitude angles in degrees.
struct Orientation
{
    float pitch_deg{0.0f}; // Elevation tilt: positive = nose up, negative = nose down
    float roll_deg{0.0f};  // Bank tilt: positive = right bank, negative = left bank
    float yaw_deg{0.0f};   // Azimuth heading: [-180, +180] relative to boot
};

} // namespace hrs::core
