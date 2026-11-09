#pragma once

#include "ports/i_threat_evaluator.h"

#include <vector>

namespace hrs::mocks
{

class MockThreatEvaluator : public ports::IThreatEvaluator
{
public:
    MockThreatEvaluator()           = default;
    ~MockThreatEvaluator() override = default;

    core::ThreatAssessment evaluate(const std::vector<core::TargetData>& targets) const override
    {
        ++evaluate_call_count_;
        last_targets_ = targets;
        return stubbed_assessment_;
    }

    void set_assessment(const core::ThreatAssessment& assessment)
    {
        stubbed_assessment_ = assessment;
    }

    [[nodiscard]] uint32_t evaluate_call_count() const
    {
        return evaluate_call_count_;
    }

    [[nodiscard]] const std::vector<core::TargetData>& last_targets() const
    {
        return last_targets_;
    }

private:
    mutable uint32_t evaluate_call_count_{0};
    mutable std::vector<core::TargetData> last_targets_{};
    core::ThreatAssessment stubbed_assessment_{};
};

} // namespace hrs::mocks
