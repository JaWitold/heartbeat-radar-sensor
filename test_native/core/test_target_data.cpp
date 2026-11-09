#include "core/target_data.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace hrs::core;
using Catch::Approx;

SCENARIO("TargetData default construction and validity", "[core][target]")
{
    GIVEN("A default-initialized TargetData instance")
    {
        const TargetData target{};

        THEN("All coordinates and properties are zero or invalid")
        {
            REQUIRE(0 == target.id);
            REQUIRE(0 == target.x_mm);
            REQUIRE(0 == target.y_mm);
            REQUIRE(0 == target.speed_cms);
            REQUIRE(0 == target.distance_resolution_mm);
            REQUIRE(false == target.is_valid);
            REQUIRE(0.0f == target.distance_m());
            REQUIRE(0.0f == target.angle_deg());
            REQUIRE(TargetZone::Safe == target.zone(1.5f, 3.5f));
        }
    }
}

SCENARIO("TargetData Euclidean distance calculation", "[core][target]")
{
    GIVEN("A valid target directly ahead on the Y axis")
    {
        TargetData target{};
        target.id       = 1;
        target.x_mm     = 0;
        target.y_mm     = 3000;
        target.is_valid = true;

        THEN("The distance is exactly 3.0 meters")
        {
            REQUIRE(Approx(3.0f) == target.distance_m());
        }
    }

    GIVEN("A valid target with 3-4-5 Pythagorean coordinates")
    {
        TargetData target{};
        target.id       = 2;
        target.x_mm     = 1200;
        target.y_mm     = 1600;
        target.is_valid = true;

        THEN("The distance is exactly 2.0 meters")
        {
            REQUIRE(Approx(2.0f) == target.distance_m());
        }
    }

    GIVEN("A target marked invalid despite non-zero coordinates")
    {
        TargetData target{};
        target.id       = 3;
        target.x_mm     = 1200;
        target.y_mm     = 1600;
        target.is_valid = false;

        THEN("The calculated distance returns 0.0 meters")
        {
            REQUIRE(0.0f == target.distance_m());
        }
    }
}

SCENARIO("TargetData Azimuth angle calculation", "[core][target]")
{
    GIVEN("A target located straight ahead")
    {
        TargetData target{};
        target.x_mm     = 0;
        target.y_mm     = 2500;
        target.is_valid = true;

        THEN("Azimuth angle is 0.0 degrees")
        {
            REQUIRE(Approx(0.0f) == target.angle_deg());
        }
    }

    GIVEN("A target located 45 degrees to the right (positive X)")
    {
        TargetData target{};
        target.x_mm     = 2000;
        target.y_mm     = 2000;
        target.is_valid = true;

        THEN("Azimuth angle is +45.0 degrees")
        {
            REQUIRE(Approx(45.0f) == target.angle_deg());
        }
    }

    GIVEN("A target located 45 degrees to the left (negative X)")
    {
        TargetData target{};
        target.x_mm     = -2000;
        target.y_mm     = 2000;
        target.is_valid = true;

        THEN("Azimuth angle is -45.0 degrees")
        {
            REQUIRE(Approx(-45.0f) == target.angle_deg());
        }
    }
}

SCENARIO("TargetData Threat Zone classification", "[core][target]")
{
    constexpr float DANGER_M  = 1.5f;
    constexpr float CAUTION_M = 3.5f;

    GIVEN("A target closer than 1.5 meters")
    {
        TargetData target{};
        target.x_mm     = 0;
        target.y_mm     = 1200; // 1.2m
        target.is_valid = true;

        THEN("Zone is Danger (hazard border & buzzer alert trigger)")
        {
            REQUIRE(TargetZone::Danger == target.zone(DANGER_M, CAUTION_M));
        }
    }

    GIVEN("A target at caution distance between 1.5m and 3.5m")
    {
        TargetData target{};
        target.x_mm     = 1000;
        target.y_mm     = 2000; // ~2.23m
        target.is_valid = true;

        THEN("Zone is Caution (amber blip)")
        {
            REQUIRE(TargetZone::Caution == target.zone(DANGER_M, CAUTION_M));
        }
    }

    GIVEN("A target beyond 3.5 meters")
    {
        TargetData target{};
        target.x_mm     = 0;
        target.y_mm     = 4500; // 4.5m
        target.is_valid = true;

        THEN("Zone is Safe (green blip)")
        {
            REQUIRE(TargetZone::Safe == target.zone(DANGER_M, CAUTION_M));
        }
    }

    GIVEN("A target at exact boundary conditions")
    {
        TargetData danger_boundary{};
        danger_boundary.x_mm     = 0;
        danger_boundary.y_mm     = 1499; // 1.499m < 1.5m
        danger_boundary.is_valid = true;
        REQUIRE(TargetZone::Danger == danger_boundary.zone(DANGER_M, CAUTION_M));

        TargetData caution_boundary_low{};
        caution_boundary_low.x_mm     = 0;
        caution_boundary_low.y_mm     = 1500; // 1.5m
        caution_boundary_low.is_valid = true;
        REQUIRE(TargetZone::Caution == caution_boundary_low.zone(DANGER_M, CAUTION_M));

        TargetData caution_boundary_high{};
        caution_boundary_high.x_mm     = 0;
        caution_boundary_high.y_mm     = 3500; // 3.5m
        caution_boundary_high.is_valid = true;
        REQUIRE(TargetZone::Caution == caution_boundary_high.zone(DANGER_M, CAUTION_M));

        TargetData safe_boundary{};
        safe_boundary.x_mm     = 0;
        safe_boundary.y_mm     = 3501; // 3.501m
        safe_boundary.is_valid = true;
        REQUIRE(TargetZone::Safe == safe_boundary.zone(DANGER_M, CAUTION_M));
    }
}

SCENARIO("TargetData zone uses the thresholds it is given", "[core][target]")
{
    GIVEN("A valid target at 1.8m")
    {
        TargetData target{};
        target.y_mm     = 1800;
        target.is_valid = true;

        THEN("It is Caution with 1.5m/3.5m thresholds")
        {
            REQUIRE(TargetZone::Caution == target.zone(1.5f, 3.5f));
        }

        THEN("It is Danger with 2.0m/4.5m thresholds")
        {
            REQUIRE(TargetZone::Danger == target.zone(2.0f, 4.5f));
        }
    }
}
