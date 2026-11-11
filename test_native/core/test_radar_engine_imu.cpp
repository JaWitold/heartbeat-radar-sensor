#include "core/radar_engine.h"
#include "mocks/mock_buzzer.h"
#include "mocks/mock_display.h"
#include "mocks/mock_imu.h"
#include "mocks/mock_sensor.h"
#include "mocks/mock_threat_evaluator.h"
#include "mocks/mock_timer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace hrs::core;
using namespace hrs::mocks;
using Catch::Approx;

SCENARIO("RadarEngine with optional IMU dependency", "[radar_engine][imu]")
{
    GIVEN("A RadarEngine wired with MockSensor, MockDisplay, MockTimer, and MockImu")
    {
        MockSensor sensor;
        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;
        MockBuzzer buzzer;
        MockImu imu;

        RadarEngine engine(sensor, display, timer, evaluator, &buzzer, &imu);

        WHEN("init() is called")
        {
            const bool ok = engine.init();

            THEN("Sensor, display, and IMU are all initialized")
            {
                REQUIRE(true == ok);
                REQUIRE(true == sensor.init_called());
                REQUIRE(true == display.init_called());
                REQUIRE(true == imu.init_called());
                REQUIRE(false == engine.has_imu());
            }
        }

        WHEN("update() is called with valid IMU data")
        {
            engine.init();

            ImuData sample{};
            sample.is_valid = true;
            sample.accel    = {0.0f, 0.7071f, 0.7071f}; // 45 deg right roll
            sample.gyro     = {0.0f, 0.0f, 0.0f};
            sample.temp_c   = 26.5f;
            imu.set_imu_data(sample);

            timer.advance_ms(20);
            engine.update();

            THEN("IMU is polled and attitude is passed to display")
            {
                REQUIRE(1 == imu.read_imu_call_count());
                REQUIRE(true == engine.has_imu());
                REQUIRE(45.0f == Approx(engine.current_orientation().roll_deg).margin(0.5f));
                REQUIRE(1 == display.draw_attitude_call_count());
                REQUIRE(true == display.has_imu());
                REQUIRE(45.0f == Approx(display.last_orientation().roll_deg).margin(0.5f));
                REQUIRE(26.5f == Approx(display.last_imu().temp_c).margin(0.1f));
            }
        }

        WHEN("IMU is disconnected")
        {
            engine.init();
            imu.set_connected(false);

            timer.advance_ms(20);
            engine.update();

            THEN("Radar still scans and display receives no IMU data")
            {
                REQUIRE(1 == sensor.scan_call_count());
                REQUIRE(false == engine.has_imu());
                REQUIRE(1 == display.draw_attitude_call_count());
                REQUIRE(false == display.has_imu());
            }
        }

        WHEN("Engine is constructed without IMU (nullptr)")
        {
            RadarEngine engine_no_imu(sensor, display, timer, evaluator, &buzzer, nullptr);
            engine_no_imu.init();

            timer.advance_ms(20);
            engine_no_imu.update();

            THEN("Engine operates normally without IMU")
            {
                REQUIRE(false == engine_no_imu.has_imu());
                REQUIRE(0 == imu.read_imu_call_count());
                REQUIRE(1 == display.draw_attitude_call_count());
                REQUIRE(false == display.has_imu());
            }
        }

        WHEN("Sensor detects a target while radar is tilted at 45 deg pitch")
        {
            engine.init();

            // Set up a target at 1414mm slant range (1.0m horizontal at 45 deg pitch)
            TargetData slant_target{};
            slant_target.id       = 1;
            slant_target.x_mm     = 0;
            slant_target.y_mm     = 1414;
            slant_target.is_valid = true;
            sensor.set_targets({slant_target});

            // Set up 45 deg pitch in IMU (ax = -sin(45) = -0.7071g, az = cos(45) = 0.7071g)
            ImuData sample{};
            sample.is_valid = true;
            sample.accel    = {-0.7071f, 0.0f, 0.7071f};
            sample.gyro     = {0.0f, 0.0f, 0.0f};
            imu.set_imu_data(sample);

            timer.advance_ms(20);
            engine.update();

            THEN("Engine compensates coordinates: raw is 1414mm, active targets are 1000mm")
            {
                REQUIRE(true == engine.is_attitude_compensated());
                REQUIRE(1 == engine.raw_targets().size());
                REQUIRE(1414 == engine.raw_targets()[0].y_mm);

                REQUIRE(1 == engine.targets().size());
                REQUIRE(1000 == Approx(engine.targets()[0].y_mm).margin(2));
                REQUIRE(0 == engine.targets()[0].x_mm);

                // Display and evaluator receive the compensated targets
                REQUIRE(1 == display.last_targets().size());
                REQUIRE(1000 == Approx(display.last_targets()[0].y_mm).margin(2));
            }

            AND_WHEN("Attitude compensation is disabled")
            {
                engine.set_attitude_compensation(false);
                timer.advance_ms(20);
                engine.update();

                THEN("Active targets match uncompensated raw targets (1414mm)")
                {
                    REQUIRE(false == engine.is_attitude_compensated());
                    REQUIRE(1 == engine.targets().size());
                    REQUIRE(1414 == engine.targets()[0].y_mm);
                    REQUIRE(1414 == display.last_targets()[0].y_mm);
                }
            }
        }

        WHEN("A level, still radar has gyro z bias of 10 deg/s for 10 s")
        {
            engine.init();

            TargetData ahead{};
            ahead.id       = 1;
            ahead.y_mm     = 2000;
            ahead.is_valid = true;
            sensor.set_targets({ahead});

            ImuData sample{};
            sample.is_valid = true;
            sample.accel    = {0.0f, 0.0f, 1.0f};
            sample.gyro     = {0.0f, 0.0f, 10.0f};
            imu.set_imu_data(sample);

            for (int i = 0; 500 > i; ++i)
            {
                timer.advance_ms(20);
                engine.update();
            }

            THEN("Gyro yaw drift does not rotate the target off straight ahead")
            {
                REQUIRE(1 == engine.targets().size());
                REQUIRE(0 == Approx(engine.targets()[0].x_mm).margin(2));
                REQUIRE(2000 == Approx(engine.targets()[0].y_mm).margin(2));
            }
        }
    }
}
