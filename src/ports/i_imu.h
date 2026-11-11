#pragma once

#include "core/imu_data.h"

namespace hrs::ports
{

/// Abstract contract for 6-DOF Inertial Measurement Units (accelerometer + gyroscope).
class IImu
{
public:
    virtual ~IImu() = default;

    virtual bool init()                             = 0;
    [[nodiscard]] virtual bool is_connected() const = 0;

    /// Reads all 6 axes and temperature in a single atomic burst operation.
    virtual bool read_imu(core::ImuData& out) = 0;
};

} // namespace hrs::ports
