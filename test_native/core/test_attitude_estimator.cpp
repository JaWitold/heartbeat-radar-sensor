#include "core/attitude_estimator.h"
#include "core/imu_data.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace hrs::core;
using Catch::Approx;

SCENARIO("AttitudeEstimator orientation tracking and complementary filter", "[attitude][core]")
{
    GIVEN("A newly constructed AttitudeEstimator")
    {
        AttitudeEstimator estimator;

        THEN("Initial orientation is zero")
        {
            REQUIRE(0.0f == Approx(estimator.pitch_deg()).margin(0.001f));
            REQUIRE(0.0f == Approx(estimator.roll_deg()).margin(0.001f));
            REQUIRE(0.0f == Approx(estimator.yaw_deg()).margin(0.001f));
        }

        WHEN("First reading is level (1g along Z)")
        {
            ImuData imu{};
            imu.is_valid = true;
            imu.accel    = {0.0f, 0.0f, 1.0f};
            imu.gyro     = {0.0f, 0.0f, 0.0f};

            estimator.update(imu, 0.02f);

            THEN("Pitch and roll initialize to zero degrees")
            {
                REQUIRE(0.0f == Approx(estimator.pitch_deg()).margin(0.1f));
                REQUIRE(0.0f == Approx(estimator.roll_deg()).margin(0.1f));
                REQUIRE(0.0f == Approx(estimator.yaw_deg()).margin(0.1f));
            }
        }

        WHEN("First reading is tilted 45 degrees right (roll = +45 deg)")
        {
            // ay = sin(45) * 1g = 0.7071g, az = cos(45) * 1g = 0.7071g
            ImuData imu{};
            imu.is_valid = true;
            imu.accel    = {0.0f, 0.7071f, 0.7071f};
            imu.gyro     = {0.0f, 0.0f, 0.0f};

            estimator.update(imu, 0.02f);

            THEN("Roll initializes immediately to approximately +45 degrees")
            {
                REQUIRE(45.0f == Approx(estimator.roll_deg()).margin(0.5f));
                REQUIRE(0.0f == Approx(estimator.pitch_deg()).margin(0.5f));
            }
        }

        WHEN("First reading is tilted 30 degrees nose-up (pitch = +30 deg)")
        {
            // ax = -sin(30) = -0.5g, az = cos(30) = 0.866g
            ImuData imu{};
            imu.is_valid = true;
            imu.accel    = {-0.5f, 0.0f, 0.866f};
            imu.gyro     = {0.0f, 0.0f, 0.0f};

            estimator.update(imu, 0.02f);

            THEN("Pitch initializes immediately to approximately +30 degrees")
            {
                REQUIRE(30.0f == Approx(estimator.pitch_deg()).margin(0.5f));
                REQUIRE(0.0f == Approx(estimator.roll_deg()).margin(0.5f));
            }
        }

        WHEN("Device rotates in yaw at 45 deg/s for 2 seconds")
        {
            ImuData imu{};
            imu.is_valid = true;
            imu.accel    = {0.0f, 0.0f, 1.0f};
            imu.gyro     = {0.0f, 0.0f, 45.0f}; // 45 deg/s right turn

            // First step initializes
            estimator.update(imu, 0.02f);

            // 100 steps of 0.02s = 2.0 seconds total (45 deg/s * 2s = 90 deg)
            for (int i = 0; 100 > i; ++i)
            {
                estimator.update(imu, 0.02f);
            }

            THEN("Yaw integrates to approximately 90 degrees")
            {
                REQUIRE(90.0f == Approx(estimator.yaw_deg()).margin(0.5f));
            }
        }

        WHEN("Yaw exceeds 180 degrees")
        {
            ImuData imu{};
            imu.is_valid = true;
            imu.accel    = {0.0f, 0.0f, 1.0f};
            imu.gyro     = {0.0f, 0.0f, 200.0f};

            estimator.update(imu, 0.02f);
            estimator.update(imu, 1.0f); // +200 deg -> should wrap to -160 deg

            THEN("Yaw wraps into [-180, +180] range")
            {
                REQUIRE(-160.0f == Approx(estimator.yaw_deg()).margin(0.5f));
            }
        }

        WHEN("Invalid data or non-positive dt is passed")
        {
            ImuData valid{};
            valid.is_valid = true;
            valid.accel    = {0.0f, 0.0f, 1.0f};
            estimator.update(valid, 0.02f);

            ImuData invalid{};
            invalid.is_valid = false;
            invalid.accel    = {100.0f, 100.0f, 100.0f};

            estimator.update(invalid, 0.02f);
            estimator.update(valid, 0.0f);
            estimator.update(valid, -0.01f);

            THEN("State remains uncorrupted")
            {
                REQUIRE(0.0f == Approx(estimator.pitch_deg()).margin(0.1f));
                REQUIRE(0.0f == Approx(estimator.roll_deg()).margin(0.1f));
            }
        }

        WHEN("Reset is called after movement")
        {
            ImuData imu{};
            imu.is_valid = true;
            imu.accel    = {0.0f, 0.0f, 1.0f};
            imu.gyro     = {0.0f, 0.0f, 50.0f};

            estimator.update(imu, 0.02f);
            estimator.update(imu, 1.0f);

            estimator.reset();

            THEN("All angles return to zero and estimator awaits fresh initialization")
            {
                REQUIRE(0.0f == Approx(estimator.pitch_deg()).margin(0.001f));
                REQUIRE(0.0f == Approx(estimator.roll_deg()).margin(0.001f));
                REQUIRE(0.0f == Approx(estimator.yaw_deg()).margin(0.001f));
            }
        }
    }
}
