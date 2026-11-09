#pragma once

#include <cstdint>

namespace hrs::core
{

enum class ThreatLevel : uint8_t
{
    Safe     = 0,
    Caution  = 1,
    Danger   = 2,
    Critical = 3,
    NoSignal = 4 // sensor stopped sending frames; no assessment possible
};

struct ThreatAssessment
{
    ThreatLevel level{ThreatLevel::Safe};
    float severity{0.0f};
    uint8_t primary_target_id{0};
    float min_distance_m{0.0f};
    float closing_speed_mps{0.0f};

    [[nodiscard]] bool operator==(const ThreatAssessment& other) const
    {
        return level == other.level && severity == other.severity && primary_target_id == other.primary_target_id;
    }
};

} // namespace hrs::core
