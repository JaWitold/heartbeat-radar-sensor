#pragma once

#include "ports/i_threat_evaluator.h"

namespace hrs::core
{

class DistanceThreatEvaluator : public ports::IThreatEvaluator
{
public:
    DistanceThreatEvaluator(const float danger_dist_m, const float caution_dist_m, const float max_dist_m);

    ~DistanceThreatEvaluator() override = default;

    [[nodiscard]] ThreatAssessment evaluate(const std::vector<TargetData>& targets) const override;

private:
    float danger_distance_m_;
    float caution_distance_m_;
    float max_distance_m_;
};

} // namespace hrs::core
