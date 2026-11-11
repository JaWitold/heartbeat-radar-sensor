#include "telemetry_frame.h"

#include "imu_data.h"
#include "target_data.h"
#include "threat_assessment.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace hrs::core
{

namespace
{

// Fixed-point scales (power of ten = decimals shown). Integer formatting on purpose:
// float std::to_chars/printf pull ~100 KB of Ryu tables into the ESP32 image.
constexpr int32_t MM_PER_M       = 1000; // x, y, distance: 3 decimals
constexpr int32_t CMS_PER_MPS    = 100;  // speed: 2 decimals
constexpr int32_t SWEEP_SCALE    = 10;   // sweep_deg: 1 decimal
constexpr int32_t SEVERITY_SCALE = 100;  // severity: 2 decimals
constexpr int32_t ACCEL_SCALE    = 100;  // ax, ay, az: 2 decimals
constexpr int32_t GYRO_SCALE     = 10;   // gx, gy, gz: 1 decimal
constexpr int32_t ANGLE_SCALE    = 10;   // pitch, roll, yaw: 1 decimal
constexpr int32_t TEMP_SCALE     = 10;   // temp_c: 1 decimal
constexpr int32_t DECIMAL_BASE   = 10;

/// Appends scaled / scale in fixed notation, e.g. (-25, 100) -> "-0.25".
void append_scaled(std::string& out, const int32_t scaled, const int32_t scale)
{
    if (0 > scaled)
    {
        out += '-';
    }
    const int32_t magnitude = (0 > scaled) ? -scaled : scaled;
    out += std::to_string(magnitude / scale);
    out += '.';
    const int32_t fraction = magnitude % scale;
    for (int32_t digit = scale / DECIMAL_BASE; 0 < digit; digit /= DECIMAL_BASE)
    {
        out += static_cast<char>('0' + ((fraction / digit) % DECIMAL_BASE));
    }
}

/// Rounds half away from zero, matching Arduino Print::print(float, digits).
void append_fixed(std::string& out, const float value, const int32_t scale)
{
    append_scaled(out, static_cast<int32_t>(std::lround(value * static_cast<float>(scale))), scale);
}

const char* level_name(const ThreatLevel level)
{
    switch (level)
    {
        case ThreatLevel::NoSignal:
            return "NoSignal";
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
                                   const float caution_distance_m,
                                   const ImuData* const imu,
                                   const Orientation* const orientation)
{
    std::string line = R"({"type":"radar_frame","sweep_deg":)";
    append_fixed(line, sweep_deg, SWEEP_SCALE);
    line += R"(,"alert":")";
    line += level_name(threat.level);
    line += R"(","severity":)";
    append_fixed(line, threat.severity, SEVERITY_SCALE);
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
        append_scaled(line, target.x_mm, MM_PER_M);
        line += R"(,"y":)";
        append_scaled(line, target.y_mm, MM_PER_M);
        line += R"(,"speed":)";
        append_scaled(line, target.speed_cms, CMS_PER_MPS);
        line += R"(,"distance":)";
        append_fixed(line, target.distance_m(), MM_PER_M);
        line += R"(,"threat_zone":")";
        line += zone_name(target.zone(danger_distance_m, caution_distance_m));
        line += R"("})";
    }

    line += "]";

    if (nullptr != imu && true == imu->is_valid && nullptr != orientation)
    {
        line += R"(,"imu":{"pitch":)";
        append_fixed(line, orientation->pitch_deg, ANGLE_SCALE);
        line += R"(,"roll":)";
        append_fixed(line, orientation->roll_deg, ANGLE_SCALE);
        line += R"(,"yaw":)";
        append_fixed(line, orientation->yaw_deg, ANGLE_SCALE);
        line += R"(,"ax":)";
        append_fixed(line, imu->accel.x, ACCEL_SCALE);
        line += R"(,"ay":)";
        append_fixed(line, imu->accel.y, ACCEL_SCALE);
        line += R"(,"az":)";
        append_fixed(line, imu->accel.z, ACCEL_SCALE);
        line += R"(,"gx":)";
        append_fixed(line, imu->gyro.x, GYRO_SCALE);
        line += R"(,"gy":)";
        append_fixed(line, imu->gyro.y, GYRO_SCALE);
        line += R"(,"gz":)";
        append_fixed(line, imu->gyro.z, GYRO_SCALE);
        line += R"(,"temp":)";
        append_fixed(line, imu->temp_c, TEMP_SCALE);
        line += '}';
    }

    line += "}\n";
    return line;
}

} // namespace hrs::core
