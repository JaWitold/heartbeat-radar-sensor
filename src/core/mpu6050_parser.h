#pragma once

#include "core/imu_data.h"

#include <cstddef>
#include <cstdint>

namespace hrs::core
{

enum class AccelScale : uint8_t
{
    G2  = 0, // ±2g  -> 16384 LSB/g
    G4  = 1, // ±4g  -> 8192 LSB/g
    G8  = 2, // ±8g  -> 4096 LSB/g
    G16 = 3  // ±16g -> 2048 LSB/g
};

enum class GyroScale : uint8_t
{
    Deg250  = 0, // ±250 °/s  -> 131.0 LSB/(°/s)
    Deg500  = 1, // ±500 °/s  -> 65.5 LSB/(°/s)
    Deg1000 = 2, // ±1000 °/s -> 32.8 LSB/(°/s)
    Deg2000 = 3  // ±2000 °/s -> 16.4 LSB/(°/s)
};

/// Parses 14-byte contiguous burst read from MPU-6050 starting at register 0x3B (ACCEL_XOUT_H):
/// [0-1] ACCEL_X, [2-3] ACCEL_Y, [4-5] ACCEL_Z,
/// [6-7] TEMP,
/// [8-9] GYRO_X,  [10-11] GYRO_Y, [12-13] GYRO_Z
/// Signed fields are big-endian two's complement.
bool parse_mpu6050_burst(const uint8_t* buffer,
                         size_t length,
                         ImuData& out_data,
                         AccelScale accel_scale = AccelScale::G2,
                         GyroScale gyro_scale   = GyroScale::Deg250);

} // namespace hrs::core
