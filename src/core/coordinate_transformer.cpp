#include "core/coordinate_transformer.h"

#include "core/imu_data.h"
#include "core/target_data.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <vector>

namespace hrs::core
{

namespace
{
constexpr float PI_CONST          = 3.14159265358979323846f;
constexpr float DEG_TO_RAD_FACTOR = PI_CONST / 180.0f;
} // namespace

TargetData CoordinateTransformer::transform(const TargetData& raw_target,
                                            const Orientation& orientation,
                                            const TransformConfig& config)
{
    if (false == raw_target.is_valid)
    {
        return raw_target;
    }

    float x_m = static_cast<float>(raw_target.x_mm) / 1000.0f;
    float y_m = static_cast<float>(raw_target.y_mm) / 1000.0f;

    // 1. Pitch elevation tilt compensation (slant-to-ground projection)
    if (true == config.compensate_pitch)
    {
        const float total_pitch_deg = orientation.pitch_deg + config.mounting_pitch_deg;
        const float pitch_rad       = total_pitch_deg * DEG_TO_RAD_FACTOR;
        float cos_pitch             = std::cos(pitch_rad);
        if (0.0f > cos_pitch)
        {
            cos_pitch = 0.0f;
        }
        y_m *= cos_pitch;
    }

    // 2. Roll bank tilt compensation (lateral ground projection)
    if (true == config.compensate_roll)
    {
        const float total_roll_deg = orientation.roll_deg + config.mounting_roll_deg;
        const float roll_rad       = total_roll_deg * DEG_TO_RAD_FACTOR;
        float cos_roll             = std::cos(roll_rad);
        if (0.0f > cos_roll)
        {
            cos_roll = 0.0f;
        }
        x_m *= cos_roll;
    }

    // 3. Yaw azimuth heading rotation (stabilize coordinates to reference frame)
    if (true == config.compensate_yaw)
    {
        const float total_yaw_deg = orientation.yaw_deg + config.mounting_yaw_deg;
        const float yaw_rad       = total_yaw_deg * DEG_TO_RAD_FACTOR;
        const float cos_yaw       = std::cos(yaw_rad);
        const float sin_yaw       = std::sin(yaw_rad);

        const float x_rot = (x_m * cos_yaw) + (y_m * sin_yaw);
        const float y_rot = (-x_m * sin_yaw) + (y_m * cos_yaw);

        x_m = x_rot;
        y_m = y_rot;
    }

    TargetData result = raw_target;
    result.x_mm       = static_cast<int16_t>(std::round(x_m * 1000.0f));
    result.y_mm       = static_cast<int16_t>(std::round(y_m * 1000.0f));

    return result;
}

std::vector<TargetData> CoordinateTransformer::transform(const std::vector<TargetData>& raw_targets,
                                                         const Orientation& orientation,
                                                         const TransformConfig& config)
{
    std::vector<TargetData> results;
    results.reserve(raw_targets.size());
    std::transform(raw_targets.begin(),
                   raw_targets.end(),
                   std::back_inserter(results),
                   [&orientation, &config](const TargetData& target)
                   { return transform(target, orientation, config); });
    return results;
}

} // namespace hrs::core
