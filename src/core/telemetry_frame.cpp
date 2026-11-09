#include "telemetry_frame.h"

#include "target_data.h"
#include "threat_assessment.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <string>
#include <vector>

namespace hrs::core
{

namespace
{

constexpr float MM_PER_M    = 1000.0f;
constexpr float CMS_PER_MPS = 100.0f;

constexpr int SWEEP_DECIMALS    = 1;
constexpr int SEVERITY_DECIMALS = 2;
constexpr int METERS_DECIMALS   = 3;
constexpr int SPEED_DECIMALS    = 2;

// Fits any int16-derived metre/speed value or a float sweep angle with its decimals.
constexpr std::size_t NUMBER_BUFFER_SIZE = 32;

void append_fixed(std::string& out, const float value, const int decimals)
{
    std::array<char, NUMBER_BUFFER_SIZE> buffer{};
    const auto result =
        std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, std::chars_format::fixed, decimals);
    out.append(buffer.data(), result.ptr);
}

const char* level_name(const ThreatLevel level)
{
    switch (level)
    {
        case ThreatLevel::NoSignal:
            return "NoSignal";
        case ThreatLevel::Critical:
            return "Critical";
        case ThreatLevel::Danger:
            return "Danger";
        case ThreatLevel::Caution:
            return "Caution";
        case ThreatLevel::Safe:
        default:
            return "Safe";
    }
}

const char* zone_name(const TargetZone zone)
{
    switch (zone)
    {
        case TargetZone::Danger:
            return "Danger";
        case TargetZone::Caution:
            return "Caution";
        case TargetZone::Safe:
        default:
            return "Safe";
    }
}

} // namespace

std::string encode_telemetry_frame(const float sweep_deg,
                                   const ThreatAssessment& threat,
                                   const std::vector<TargetData>& targets,
                                   const float danger_distance_m,
                                   const float caution_distance_m)
{
    std::string line = R"({"type":"radar_frame","sweep_deg":)";
    append_fixed(line, sweep_deg, SWEEP_DECIMALS);
    line += R"(,"alert":")";
    line += level_name(threat.level);
    line += R"(","severity":)";
    append_fixed(line, threat.severity, SEVERITY_DECIMALS);
    line += R"(,"targets":[)";

    bool first = true;
    for (const auto& target : targets)
    {
        if (false == target.is_valid)
        {
            continue;
        }
        if (false == first)
        {
            line += ',';
        }
        first = false;

        line += R"({"id":)";
        line += std::to_string(static_cast<unsigned>(target.id));
        line += R"(,"x":)";
        append_fixed(line, static_cast<float>(target.x_mm) / MM_PER_M, METERS_DECIMALS);
        line += R"(,"y":)";
        append_fixed(line, static_cast<float>(target.y_mm) / MM_PER_M, METERS_DECIMALS);
        line += R"(,"speed":)";
        append_fixed(line, static_cast<float>(target.speed_cms) / CMS_PER_MPS, SPEED_DECIMALS);
        line += R"(,"distance":)";
        append_fixed(line, target.distance_m(), METERS_DECIMALS);
        line += R"(,"threat_zone":")";
        line += zone_name(target.zone(danger_distance_m, caution_distance_m));
        line += R"("})";
    }

    line += "]}\n";
    return line;
}

} // namespace hrs::core
