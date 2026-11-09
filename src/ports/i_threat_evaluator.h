#pragma once

#include "core/target_data.h"
#include "core/threat_assessment.h"

#include <vector>

namespace hrs::ports
{

class IThreatEvaluator
{
public:
    virtual ~IThreatEvaluator() = default;

    [[nodiscard]] virtual core::ThreatAssessment evaluate(const std::vector<core::TargetData>& targets) const = 0;
};

} // namespace hrs::ports
