#pragma once

#include "core/imu_data.h"
#include "core/target_data.h"
#include "core/threat_assessment.h"

#include <vector>

namespace hrs::ports
{

class IDisplay
{
public:
    virtual ~IDisplay() = default;

    virtual bool init()                                                                        = 0;
    virtual void clear()                                                                       = 0;
    virtual void draw_sweep(const float angle_deg)                                             = 0;
    virtual void draw_radar(const std::vector<core::TargetData>& targets)                      = 0;
    virtual void draw_alert(const core::ThreatAssessment& threat)                              = 0;
    virtual void draw_attitude(const core::Orientation& orientation, const core::ImuData* imu) = 0;
    virtual void flush()                                                                       = 0;
};

} // namespace hrs::ports
