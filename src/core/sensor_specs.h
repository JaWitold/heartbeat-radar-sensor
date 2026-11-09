#pragma once

#include <cstdint>

namespace hrs::core
{

/// Physical limits and alert thresholds reported by a sensor via ISensor::specs().
/// Each sensor adapter owns its own constant; the composition root picks the
/// adapter (and thus the specs) from the build target's ENABLE_SENSOR_* flag.
struct SensorSpecs
{
    float max_distance_m;
    float fov_min_deg;
    float fov_max_deg;
    uint8_t max_tracked_targets;
    float danger_distance_m;
    float caution_distance_m;
};

} // namespace hrs::core
