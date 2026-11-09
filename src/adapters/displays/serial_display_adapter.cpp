#include "serial_display_adapter.h"

#ifndef ARDUINO
#include <iostream>
#endif

namespace hrs::adapters::displays
{

#ifdef ARDUINO
SerialDisplayAdapter::SerialDisplayAdapter(Print& out, const float danger_distance_m, const float caution_distance_m)
    : out_(out)
    , danger_distance_m_(danger_distance_m)
    , caution_distance_m_(caution_distance_m)
{
}

bool SerialDisplayAdapter::init()
{
    out_.println(F("{\"type\":\"radar_status\",\"status\":\"online\"}"));
    return true;
}

void SerialDisplayAdapter::clear()
{
    targets_.clear();
}

void SerialDisplayAdapter::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void SerialDisplayAdapter::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void SerialDisplayAdapter::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void SerialDisplayAdapter::flush()
{
    out_.print(F("{\"type\":\"radar_frame\",\"sweep_deg\":"));
    out_.print(sweep_angle_deg_, 1);
    out_.print(F(",\"alert\":\""));

    switch (threat_.level)
    {
        case core::ThreatLevel::NoSignal:
            out_.print(F("NoSignal"));
            break;
        case core::ThreatLevel::Critical:
            out_.print(F("Critical"));
            break;
        case core::ThreatLevel::Danger:
            out_.print(F("Danger"));
            break;
        case core::ThreatLevel::Caution:
            out_.print(F("Caution"));
            break;
        case core::ThreatLevel::Safe:
        default:
            out_.print(F("Safe"));
            break;
    }

    out_.print(F("\",\"severity\":"));
    out_.print(threat_.severity, 2);
    out_.print(F(",\"targets\":["));

    bool first = true;
    for (const auto& t : targets_)
    {
        if (false == t.is_valid)
        {
            continue;
        }

        if (false == first)
        {
            out_.print(',');
        }
        first = false;

        out_.print(F("{\"id\":"));
        out_.print(t.id);
        out_.print(F(",\"x\":"));
        out_.print(static_cast<float>(t.x_mm) / 1000.0f, 3);
        out_.print(F(",\"y\":"));
        out_.print(static_cast<float>(t.y_mm) / 1000.0f, 3);
        out_.print(F(",\"speed\":"));
        out_.print(static_cast<float>(t.speed_cms) / 100.0f, 2);
        out_.print(F(",\"distance\":"));
        out_.print(t.distance_m(), 3);
        out_.print(F(",\"threat_zone\":\""));

        switch (t.zone(danger_distance_m_, caution_distance_m_))
        {
            case core::TargetZone::Danger:
                out_.print(F("Danger"));
                break;
            case core::TargetZone::Caution:
                out_.print(F("Caution"));
                break;
            case core::TargetZone::Safe:
            default:
                out_.print(F("Safe"));
                break;
        }

        out_.print(F("\"}"));
    }

    out_.println(F("]}"));
}

#else

SerialDisplayAdapter::SerialDisplayAdapter()
{
}

bool SerialDisplayAdapter::init()
{
    return true;
}

void SerialDisplayAdapter::clear()
{
    targets_.clear();
}

void SerialDisplayAdapter::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void SerialDisplayAdapter::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void SerialDisplayAdapter::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void SerialDisplayAdapter::flush()
{
}

#endif

} // namespace hrs::adapters::displays
