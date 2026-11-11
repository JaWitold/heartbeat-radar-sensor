#include "core/telemetry_frame.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace hrs::core;

namespace
{

constexpr float DANGER_M  = 1.5f;
constexpr float CAUTION_M = 3.5f;

TargetData make_target(const uint8_t id, const int16_t x_mm, const int16_t y_mm, const int16_t speed_cms)
{
    TargetData target{};
    target.id        = id;
    target.x_mm      = x_mm;
    target.y_mm      = y_mm;
    target.speed_cms = speed_cms;
    target.is_valid  = true;
    return target;
}

ThreatAssessment make_threat(const ThreatLevel level, const float severity)
{
    ThreatAssessment threat{};
    threat.level    = level;
    threat.severity = severity;
    return threat;
}

} // namespace

SCENARIO("Telemetry frame encodes the JSON line the HUDs decode", "[core][telemetry]")
{
    GIVEN("One valid target inside the danger zone")
    {
        const std::vector<TargetData> targets{make_target(1, 500, 1000, -25)};
        const ThreatAssessment threat = make_threat(ThreatLevel::Danger, 0.8f);

        WHEN("The frame is encoded")
        {
            const std::string line = encode_telemetry_frame(12.5f, threat, targets, DANGER_M, CAUTION_M);

            THEN("It matches the serial telemetry wire format exactly")
            {
                REQUIRE(std::string("{\"type\":\"radar_frame\",\"sweep_deg\":12.5,\"alert\":\"Danger\","
                                    "\"severity\":0.80,\"targets\":[{\"id\":1,\"x\":0.500,\"y\":1.000,"
                                    "\"speed\":-0.25,\"distance\":1.118,\"threat_zone\":\"Danger\"}]}\n") == line);
            }
        }
    }

    GIVEN("No targets")
    {
        const std::vector<TargetData> targets{};

        THEN("The targets array is empty")
        {
            REQUIRE(std::string("{\"type\":\"radar_frame\",\"sweep_deg\":0.0,\"alert\":\"Safe\","
                                "\"severity\":0.00,\"targets\":[]}\n") ==
                    encode_telemetry_frame(0.0f, ThreatAssessment{}, targets, DANGER_M, CAUTION_M));
        }
    }

    GIVEN("A mix of valid and invalid targets")
    {
        TargetData invalid = make_target(2, 100, 100, 0);
        invalid.is_valid   = false;
        const std::vector<TargetData> targets{invalid, make_target(3, -2000, 2000, 10)};

        WHEN("The frame is encoded")
        {
            const std::string line = encode_telemetry_frame(0.0f, ThreatAssessment{}, targets, DANGER_M, CAUTION_M);

            THEN("Invalid targets are skipped and negative coordinates keep their sign")
            {
                REQUIRE(std::string::npos == line.find("\"id\":2"));
                REQUIRE(std::string::npos != line.find("{\"id\":3,\"x\":-2.000,\"y\":2.000,\"speed\":0.10,"
                                                       "\"distance\":2.828,\"threat_zone\":\"Caution\"}"));
            }
        }
    }

    GIVEN("A target 3 m away")
    {
        const std::vector<TargetData> targets{make_target(1, 0, 3000, 0)};

        THEN("Its zone follows the thresholds passed in, not global constants")
        {
            REQUIRE(std::string::npos != encode_telemetry_frame(0.0f, ThreatAssessment{}, targets, 1.5f, 3.5f)
                                             .find("\"threat_zone\":\"Caution\""));
            REQUIRE(
                std::string::npos !=
                encode_telemetry_frame(0.0f, ThreatAssessment{}, targets, 1.0f, 2.0f).find("\"threat_zone\":\"Safe\""));
            REQUIRE(std::string::npos != encode_telemetry_frame(0.0f, ThreatAssessment{}, targets, 4.0f, 5.0f)
                                             .find("\"threat_zone\":\"Danger\""));
        }
    }

    GIVEN("Every threat level")
    {
        const std::vector<TargetData> targets{};

        THEN("Each level is named as the HUD expects")
        {
            const auto alert_of = [&targets](const ThreatLevel level)
            { return encode_telemetry_frame(0.0f, make_threat(level, 0.0f), targets, DANGER_M, CAUTION_M); };

            REQUIRE(std::string::npos != alert_of(ThreatLevel::Safe).find("\"alert\":\"Safe\""));
            REQUIRE(std::string::npos != alert_of(ThreatLevel::Caution).find("\"alert\":\"Caution\""));
            REQUIRE(std::string::npos != alert_of(ThreatLevel::Danger).find("\"alert\":\"Danger\""));
            REQUIRE(std::string::npos != alert_of(ThreatLevel::NoSignal).find("\"alert\":\"NoSignal\""));
        }
    }

    GIVEN("A valid IMU reading and calculated device orientation")
    {
        const std::vector<TargetData> targets{};
        const ThreatAssessment threat = make_threat(ThreatLevel::Safe, 0.0f);

        ImuData imu{};
        imu.is_valid = true;
        imu.accel    = {0.02f, 0.15f, 0.98f};
        imu.gyro     = {0.1f, -0.2f, 0.0f};
        imu.temp_c   = 24.5f;

        Orientation orientation{1.2f, -3.4f, 45.0f};

        WHEN("The frame is encoded with IMU telemetry")
        {
            const std::string line =
                encode_telemetry_frame(0.0f, threat, targets, DANGER_M, CAUTION_M, &imu, &orientation);

            THEN("The imu object is serialized with fixed-point accuracy")
            {
                REQUIRE(std::string::npos != line.find(",\"imu\":{\"pitch\":1.2,\"roll\":-3.4,\"yaw\":45.0,"
                                                       "\"ax\":0.02,\"ay\":0.15,\"az\":0.98,"
                                                       "\"gx\":0.1,\"gy\":-0.2,\"gz\":0.0,\"temp\":24.5}"));
            }
        }

        WHEN("IMU is invalid")
        {
            imu.is_valid = false;
            const std::string line =
                encode_telemetry_frame(0.0f, threat, targets, DANGER_M, CAUTION_M, &imu, &orientation);

            THEN("The imu object is omitted")
            {
                REQUIRE(std::string::npos == line.find("\"imu\":"));
            }
        }
    }
}
