#pragma once

#include "core/imu_data.h"
#include "core/target_data.h"

#include <vector>

namespace hrs::core
{

/// Configuration parameters and feature flags for IMU attitude compensation.
struct TransformConfig
{
    bool compensate_pitch{true};    ///< Slant-to-ground projection: y' = y * cos(pitch)
    bool compensate_roll{true};     ///< Lateral bank projection: x' = x * cos(roll)
    bool compensate_yaw{true};      ///< Azimuth heading rotation to reference frame
    float mounting_yaw_deg{0.0f};   ///< Fixed static mounting yaw offset
    float mounting_pitch_deg{0.0f}; ///< Fixed static mounting pitch offset
    float mounting_roll_deg{0.0f};  ///< Fixed static mounting roll offset
};

/// Pure domain coordinate transformer projecting radar targets from the local sensor frame
/// to an attitude-compensated reference navigation frame.
class CoordinateTransformer
{
public:
    [[nodiscard]] static TargetData
    transform(const TargetData& raw_target, const Orientation& orientation, const TransformConfig& config = {});

    [[nodiscard]] static std::vector<TargetData> transform(const std::vector<TargetData>& raw_targets,
                                                           const Orientation& orientation,
                                                           const TransformConfig& config = {});
};

} // namespace hrs::core
