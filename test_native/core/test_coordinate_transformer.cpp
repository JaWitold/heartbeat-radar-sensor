#include "core/coordinate_transformer.h"
#include "core/imu_data.h"
#include "core/target_data.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace hrs::core;
using Catch::Approx;

SCENARIO("CoordinateTransformer attitude and mounting compensation", "[core][transformer]")
{
    GIVEN("A valid target directly ahead at 2.0m (x=0, y=2000mm)")
    {
        TargetData target{};
        target.id                     = 1;
        target.x_mm                   = 0;
        target.y_mm                   = 2000;
        target.speed_cms              = -50;
        target.distance_resolution_mm = 750;
        target.is_valid               = true;

        WHEN("Orientation is zero (flat, level, forward)")
        {
            const Orientation orientation{0.0f, 0.0f, 0.0f};
            const TargetData result = CoordinateTransformer::transform(target, orientation);

            THEN("Coordinates and metadata are completely unchanged")
            {
                REQUIRE(1 == result.id);
                REQUIRE(true == result.is_valid);
                REQUIRE(0 == result.x_mm);
                REQUIRE(2000 == result.y_mm);
                REQUIRE(-50 == result.speed_cms);
                REQUIRE(Approx(2.0f) == result.distance_m());
                REQUIRE(Approx(0.0f) == result.angle_deg());
            }
        }

        WHEN("Target is invalid")
        {
            target.is_valid = false;
            const Orientation orientation{45.0f, 0.0f, 45.0f};
            const TargetData result = CoordinateTransformer::transform(target, orientation);

            THEN("Invalid target is returned unchanged")
            {
                REQUIRE(false == result.is_valid);
                REQUIRE(0 == result.x_mm);
                REQUIRE(2000 == result.y_mm);
            }
        }
    }

    GIVEN("Pitch tilt compensation (user problem: radar tilted at 45 deg reads distance off by 1.414)")
    {
        // When radar is tilted down at 45 deg, an obstacle at 1.0m horizontal ground distance
        // is read along the tilted slant-range beam at: 1.0m * sqrt(2) = 1.4142m (1414mm).
        TargetData target{};
        target.id       = 2;
        target.x_mm     = 0;
        target.y_mm     = 1414;
        target.is_valid = true;

        WHEN("Sensor pitch is +45 deg (tilted up) or -45 deg (tilted down)")
        {
            const Orientation tilt_up{45.0f, 0.0f, 0.0f};
            const TargetData res_up = CoordinateTransformer::transform(target, tilt_up);

            const Orientation tilt_down{-45.0f, 0.0f, 0.0f};
            const TargetData res_down = CoordinateTransformer::transform(target, tilt_down);

            THEN("Slant range 1.414m is projected back to exactly 1.0m horizontal ground distance")
            {
                REQUIRE(0 == res_up.x_mm);
                REQUIRE(1000 == Approx(res_up.y_mm).margin(2));
                REQUIRE(Approx(1.0f).margin(0.01f) == res_up.distance_m());

                REQUIRE(0 == res_down.x_mm);
                REQUIRE(1000 == Approx(res_down.y_mm).margin(2));
                REQUIRE(Approx(1.0f).margin(0.01f) == res_down.distance_m());
            }
        }

        WHEN("Pitch compensation is explicitly disabled in config")
        {
            TransformConfig cfg{};
            cfg.compensate_pitch = false;

            const Orientation tilt{45.0f, 0.0f, 0.0f};
            const TargetData res = CoordinateTransformer::transform(target, tilt, cfg);

            THEN("Slant range is left uncompensated")
            {
                REQUIRE(1414 == res.y_mm);
            }
        }
    }

    GIVEN("Yaw heading rotation (radar turned by 45 deg)")
    {
        // Object is straight ahead of operator at (x=0, y=2000mm).
        // Device/radar is turned 45 deg to the right (+45 deg yaw).
        // Sensor sees object at 45 deg to the left:
        // x_sensor = -2000 * sin(45 deg) = -1414 mm
        // y_sensor =  2000 * cos(45 deg) =  1414 mm
        TargetData target{};
        target.id       = 3;
        target.x_mm     = -1414;
        target.y_mm     = 1414;
        target.is_valid = true;

        WHEN("Yaw is +45 deg")
        {
            const Orientation yaw_turn{0.0f, 0.0f, 45.0f};
            const TargetData result = CoordinateTransformer::transform(target, yaw_turn);

            THEN("Target is rotated back to (x=0, y=2000mm) in operator forward frame")
            {
                REQUIRE(0 == Approx(result.x_mm).margin(2));
                REQUIRE(2000 == Approx(result.y_mm).margin(2));
                REQUIRE(Approx(2.0f).margin(0.01f) == result.distance_m());
                REQUIRE(Approx(0.0f).margin(0.5f) == result.angle_deg());
            }
        }

        WHEN("Yaw is -45 deg (turned 45 deg left, object at 45 deg right)")
        {
            TargetData right_target{};
            right_target.id       = 4;
            right_target.x_mm     = 1414;
            right_target.y_mm     = 1414;
            right_target.is_valid = true;

            const Orientation yaw_turn{0.0f, 0.0f, -45.0f};
            const TargetData result = CoordinateTransformer::transform(right_target, yaw_turn);

            THEN("Target is rotated back to (x=0, y=2000mm)")
            {
                REQUIRE(0 == Approx(result.x_mm).margin(2));
                REQUIRE(2000 == Approx(result.y_mm).margin(2));
            }
        }

        WHEN("Yaw compensation is explicitly disabled in config")
        {
            TransformConfig cfg{};
            cfg.compensate_yaw = false;

            const Orientation yaw_turn{0.0f, 0.0f, 45.0f};
            const TargetData result = CoordinateTransformer::transform(target, yaw_turn, cfg);

            THEN("Sensor coordinates remain unrotated")
            {
                REQUIRE(-1414 == result.x_mm);
                REQUIRE(1414 == result.y_mm);
            }
        }
    }

    GIVEN("Roll bank compensation")
    {
        // Target at lateral offset x=1414mm, forward y=1000mm.
        // Device is rolled right by 45 deg.
        TargetData target{};
        target.id       = 5;
        target.x_mm     = 1414;
        target.y_mm     = 1000;
        target.is_valid = true;

        WHEN("Roll is 45 deg")
        {
            const Orientation rolled{0.0f, 45.0f, 0.0f};
            const TargetData result = CoordinateTransformer::transform(target, rolled);

            THEN("Lateral offset is projected to horizontal plane (1414 * cos(45) = 1000mm)")
            {
                REQUIRE(1000 == Approx(result.x_mm).margin(2));
                REQUIRE(1000 == result.y_mm);
            }
        }
    }

    GIVEN("Physical mounting offset compensation")
    {
        // Radar is mounted with a static 45 deg yaw offset on the device.
        // IMU reads 0 deg yaw. Target in sensor frame is at (-1414, 1414).
        TargetData target{};
        target.id       = 6;
        target.x_mm     = -1414;
        target.y_mm     = 1414;
        target.is_valid = true;

        TransformConfig cfg{};
        cfg.mounting_yaw_deg = 45.0f;

        WHEN("IMU orientation is zero")
        {
            const Orientation zero{};
            const TargetData result = CoordinateTransformer::transform(target, zero, cfg);

            THEN("Static mounting offset rotates target to forward axis")
            {
                REQUIRE(0 == Approx(result.x_mm).margin(2));
                REQUIRE(2000 == Approx(result.y_mm).margin(2));
            }
        }
    }

    GIVEN("Batch transformation of multiple targets")
    {
        std::vector<TargetData> targets(3);
        targets[0].id       = 1;
        targets[0].x_mm     = 0;
        targets[0].y_mm     = 1414;
        targets[0].is_valid = true;

        targets[1].id       = 2;
        targets[1].x_mm     = 1000;
        targets[1].y_mm     = 1414;
        targets[1].is_valid = true;

        targets[2].id       = 3;
        targets[2].is_valid = false;

        WHEN("Transformed with 45 deg pitch")
        {
            const Orientation tilt{45.0f, 0.0f, 0.0f};
            const auto results = CoordinateTransformer::transform(targets, tilt);

            THEN("All valid targets are projected and invalid targets remain untouched")
            {
                REQUIRE(3 == results.size());
                REQUIRE(true == results[0].is_valid);
                REQUIRE(1000 == Approx(results[0].y_mm).margin(2));

                REQUIRE(true == results[1].is_valid);
                REQUIRE(1000 == results[1].x_mm);
                REQUIRE(1000 == Approx(results[1].y_mm).margin(2));

                REQUIRE(false == results[2].is_valid);
            }
        }
    }
}
