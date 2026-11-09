#include "distance_threat_evaluator.h"

#include "core/target_data.h"
#include "core/threat_assessment.h"

#include <algorithm>
#include <limits>
#include <vector>

namespace hrs::core
{

DistanceThreatEvaluator::DistanceThreatEvaluator(const float danger_dist_m,
                                                 const float caution_dist_m,
                                                 const float max_dist_m)
    : danger_distance_m_(danger_dist_m)
    , caution_distance_m_(caution_dist_m)
    , max_distance_m_(max_dist_m)
{
}

ThreatAssessment DistanceThreatEvaluator::evaluate(const std::vector<TargetData>& targets) const
{
    ThreatAssessment assessment{};

    if (true == targets.empty())
    {
        return assessment;
    }

    float min_distance               = std::numeric_limits<float>::max();
    const TargetData* closest_target = nullptr;

    for (const auto& target : targets)
    {
        if (false == target.is_valid)
        {
            continue;
        }

        const float dist = target.distance_m();
        if (dist < min_distance)
        {
            min_distance   = dist;
            closest_target = &target;
        }
    }

    if (nullptr == closest_target)
    {
        return assessment;
    }

    assessment.primary_target_id = closest_target->id;
    assessment.min_distance_m    = min_distance;
    assessment.closing_speed_mps = static_cast<float>(closest_target->speed_cms) / 100.0f;

    // Severity score: 0.0 at max_distance_m_, 1.0 at 0m
    const float normalized_dist = std::clamp(min_distance / max_distance_m_, 0.0f, 1.0f);
    assessment.severity         = 1.0f - normalized_dist;

    // Categorical classification
    if (min_distance < danger_distance_m_)
    {
        assessment.level = ThreatLevel::Danger;
    }
    else if (min_distance <= caution_distance_m_)
    {
        assessment.level = ThreatLevel::Caution;
    }
    else
    {
        assessment.level = ThreatLevel::Safe;
    }

    return assessment;
}

} // namespace hrs::core
