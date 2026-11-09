#include "core/distance_threat_evaluator.h"
#include "core/target_data.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace hrs::core;
using Catch::Matchers::WithinAbs;

SCENARIO("DistanceThreatEvaluator assesses target threats", "[threat_evaluator][core]")
{
    GIVEN("A default DistanceThreatEvaluator (Danger < 1.5m, Caution <= 3.5m)")
    {
        DistanceThreatEvaluator evaluator(1.5f, 3.5f, 6.0f);

        WHEN("Target list is empty")
        {
            const auto assessment = evaluator.evaluate({});

            THEN("Threat assessment is Safe with 0 severity")
            {
                REQUIRE(ThreatLevel::Safe == assessment.level);
                REQUIRE_THAT(assessment.severity, WithinAbs(0.0f, 0.001f));
                REQUIRE(0 == assessment.primary_target_id);
            }
        }

        WHEN("A distant target at 4.5m is evaluated")
        {
            TargetData target{};
            target.id        = 1;
            target.x_mm      = 0;
            target.y_mm      = 4500;
            target.speed_cms = -10;
            target.is_valid  = true;

            const auto assessment = evaluator.evaluate({target});

            THEN("Threat level is Safe with low severity")
            {
                REQUIRE(ThreatLevel::Safe == assessment.level);
                REQUIRE(assessment.severity < 0.35f);
                REQUIRE(1 == assessment.primary_target_id);
                REQUIRE_THAT(assessment.min_distance_m, WithinAbs(4.5f, 0.01f));
            }
        }

        WHEN("A caution target at 2.5m is evaluated")
        {
            TargetData target{};
            target.id        = 2;
            target.x_mm      = 0;
            target.y_mm      = 2500;
            target.speed_cms = -20;
            target.is_valid  = true;

            const auto assessment = evaluator.evaluate({target});

            THEN("Threat level is Caution")
            {
                REQUIRE(ThreatLevel::Caution == assessment.level);
                REQUIRE(2 == assessment.primary_target_id);
                REQUIRE(assessment.severity >= 0.35f);
                REQUIRE(assessment.severity < 0.75f);
            }
        }

        WHEN("A danger target at 1.2m is evaluated")
        {
            TargetData target{};
            target.id        = 3;
            target.x_mm      = 0;
            target.y_mm      = 1200;
            target.speed_cms = -50;
            target.is_valid  = true;

            const auto assessment = evaluator.evaluate({target});

            THEN("Threat level is Danger with high severity")
            {
                REQUIRE(ThreatLevel::Danger == assessment.level);
                REQUIRE(3 == assessment.primary_target_id);
                REQUIRE(assessment.severity >= 0.75f);
            }
        }

        WHEN("Multiple targets are present, closest dominates")
        {
            TargetData far_target{};
            far_target.id       = 1;
            far_target.x_mm     = 0;
            far_target.y_mm     = 4000;
            far_target.is_valid = true;

            TargetData close_target{};
            close_target.id       = 2;
            close_target.x_mm     = 0;
            close_target.y_mm     = 1100;
            close_target.is_valid = true;

            const auto assessment = evaluator.evaluate({far_target, close_target});

            THEN("Evaluation prioritizes the danger target")
            {
                REQUIRE(ThreatLevel::Danger == assessment.level);
                REQUIRE(2 == assessment.primary_target_id);
                REQUIRE_THAT(assessment.min_distance_m, WithinAbs(1.1f, 0.01f));
            }
        }
    }

    GIVEN("A custom DistanceThreatEvaluator with custom thresholds")
    {
        // Custom scale: danger at 2.0m, caution at 5.0m
        DistanceThreatEvaluator custom_evaluator(2.0f, 5.0f, 8.0f);

        WHEN("Target is at 1.8m")
        {
            TargetData target{};
            target.id       = 1;
            target.x_mm     = 0;
            target.y_mm     = 1800;
            target.is_valid = true;

            const auto assessment = custom_evaluator.evaluate({target});

            THEN("Classified as Danger under custom threshold")
            {
                REQUIRE(ThreatLevel::Danger == assessment.level);
            }
        }
    }
}
