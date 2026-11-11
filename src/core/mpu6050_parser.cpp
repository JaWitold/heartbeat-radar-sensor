#include "mpu6050_parser.h"

#include "imu_data.h"

#include <cstddef>
#include <cstdint>

namespace hrs::core
{

namespace
{

constexpr size_t MPU6050_BURST_LEN = 14;

int16_t read_be16(const uint8_t* const buffer, const size_t offset)
{
    const auto raw = static_cast<uint16_t>((static_cast<uint16_t>(buffer[offset]) << 8U) |
                                           static_cast<uint16_t>(buffer[offset + 1U]));
    return static_cast<int16_t>(raw);
}

float get_accel_sensitivity(const AccelScale scale)
{
    switch (scale)
    {
        case AccelScale::G4:
            return 8192.0f;
        case AccelScale::G8:
            return 4096.0f;
        case AccelScale::G16:
            return 2048.0f;
        case AccelScale::G2:
        default:
            return 16384.0f;
    }
}

float get_gyro_sensitivity(const GyroScale scale)
{
    switch (scale)
    {
        case GyroScale::Deg500:
            return 65.5f;
        case GyroScale::Deg1000:
            return 32.8f;
        case GyroScale::Deg2000:
            return 16.4f;
        case GyroScale::Deg250:
        default:
            return 131.0f;
    }
}

} // namespace

bool parse_mpu6050_burst(const uint8_t* const buffer,
                         const size_t length,
                         ImuData& out_data,
                         const AccelScale accel_scale,
                         const GyroScale gyro_scale)
{
    if (nullptr == buffer || MPU6050_BURST_LEN > length)
    {
        out_data.is_valid = false;
        return false;
    }

    const float accel_sensitivity = get_accel_sensitivity(accel_scale);
    const float gyro_sensitivity  = get_gyro_sensitivity(gyro_scale);

    const int16_t raw_ax = read_be16(buffer, 0);
    const int16_t raw_ay = read_be16(buffer, 2);
    const int16_t raw_az = read_be16(buffer, 4);

    const int16_t raw_temp = read_be16(buffer, 6);

    const int16_t raw_gx = read_be16(buffer, 8);
    const int16_t raw_gy = read_be16(buffer, 10);
    const int16_t raw_gz = read_be16(buffer, 12);

    out_data.accel.x = static_cast<float>(raw_ax) / accel_sensitivity;
    out_data.accel.y = static_cast<float>(raw_ay) / accel_sensitivity;
    out_data.accel.z = static_cast<float>(raw_az) / accel_sensitivity;

    // InvenSense MPU-6000/MPU-6050 Register Map Revision 4.2 Section 4.18:
    // Temperature in degrees C = (TEMP_OUT Register Value as signed 16-bit / 340) + 36.53
    out_data.temp_c = (static_cast<float>(raw_temp) / 340.0f) + 36.53f;

    out_data.gyro.x = static_cast<float>(raw_gx) / gyro_sensitivity;
    out_data.gyro.y = static_cast<float>(raw_gy) / gyro_sensitivity;
    out_data.gyro.z = static_cast<float>(raw_gz) / gyro_sensitivity;

    out_data.is_valid = true;
    return true;
}

} // namespace hrs::core
