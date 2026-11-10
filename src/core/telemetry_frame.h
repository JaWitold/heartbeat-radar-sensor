#pragma once

#include "target_data.h"
#include "threat_assessment.h"

#include <string>
#include <vector>

namespace hrs::core
{

/// Encodes one radar frame as the newline-terminated JSON line the desktop/web HUDs decode.
/// Invalid targets are skipped; each target's zone is evaluated against the given thresholds.
[[nodiscard]] std::string encode_telemetry_frame(const float sweep_deg,
                                                 const ThreatAssessment& threat,
                                                 const std::vector<TargetData>& targets,
                                                 const float danger_distance_m,
                                                 const float caution_distance_m);

} // namespace hrs::core
