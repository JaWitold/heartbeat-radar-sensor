#pragma once

#include <cmath>
#include <cstdint>

namespace hrs::core
{

enum class TargetZone : uint8_t
{
    Safe    = 0,
    Caution = 1,
    Danger  = 2
};

struct TargetData
{
    uint8_t id{0};
    int16_t x_mm{0};
    int16_t y_mm{0};
    int16_t speed_cms{0};
    uint16_t distance_resolution_mm{0};
    bool is_valid{false};

    [[nodiscard]] float distance_m() const
    {
        if (false == is_valid)
        {
            return 0.0f;
        }
        const float x_m = static_cast<float>(x_mm) / 1000.0f;
        const float y_m = static_cast<float>(y_mm) / 1000.0f;
        return std::sqrt(x_m * x_m + y_m * y_m);
    }

    [[nodiscard]] float angle_deg() const
    {
        if (false == is_valid || (0 == x_mm && 0 == y_mm))
        {
            return 0.0f;
        }
        constexpr float RAD_TO_DEG_FACTOR = 180.0f / 3.14159265358979323846f;
        return std::atan2(static_cast<float>(x_mm), static_cast<float>(y_mm)) * RAD_TO_DEG_FACTOR;
    }

    [[nodiscard]] TargetZone zone(const float danger_distance_m, const float caution_distance_m) const
    {
        if (false == is_valid)
        {
            return TargetZone::Safe;
        }
        const float dist = distance_m();
        if (dist < danger_distance_m)
        {
            return TargetZone::Danger;
        }
        if (dist <= caution_distance_m)
        {
            return TargetZone::Caution;
        }
        return TargetZone::Safe;
    }
};

} // namespace hrs::core
