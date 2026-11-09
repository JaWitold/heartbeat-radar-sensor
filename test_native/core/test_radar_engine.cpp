#include "core/radar_engine.h"
#include "mocks/mock_buzzer.h"
#include "mocks/mock_display.h"
#include "mocks/mock_sensor.h"
#include "mocks/mock_threat_evaluator.h"
#include "mocks/mock_timer.h"

#include <catch2/catch_test_macros.hpp>

using namespace hrs::core;
using namespace hrs::mocks;

SCENARIO("RadarEngine initialization", "[radar_engine][core]")
{
    GIVEN("A RadarEngine with mocked dependencies")
    {
        MockSensor sensor;
        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;
        MockBuzzer buzzer;

        RadarEngine engine(sensor, display, timer, evaluator, &buzzer);

        WHEN("init() is called successfully")
        {
            sensor.set_init_return(true);
            display.set_init_return(true);

            const bool ok = engine.init();

            THEN("All subsystems are initialized")
            {
                REQUIRE(true == ok);
                REQUIRE(true == sensor.init_called());
                REQUIRE(true == display.init_called());
                REQUIRE(1 == buzzer.stop_call_count());
            }
        }

        WHEN("Sensor initialization fails")
        {
            sensor.set_init_return(false);
            display.set_init_return(true);

            const bool ok = engine.init();

            THEN("init() returns false")
            {
                REQUIRE(false == ok);
            }
        }
    }
}

SCENARIO("RadarEngine update loop and threat delegation", "[radar_engine][core]")
{
    GIVEN("An initialized RadarEngine with mock evaluator")
    {
        MockSensor sensor;
        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;
        MockBuzzer buzzer;

        RadarEngine engine(sensor, display, timer, evaluator, &buzzer);
        engine.init();

        WHEN("No targets are detected and evaluator returns Safe")
        {
            sensor.set_targets({});
            evaluator.set_assessment({ThreatLevel::Safe, 0.0f, 0, 0.0f, 0.0f});

            engine.update();

            THEN("Display renders with Safe alert level")
            {
                REQUIRE(1 == sensor.scan_call_count());
                REQUIRE(1 == evaluator.evaluate_call_count());
                REQUIRE(1 == display.draw_radar_call_count());
                REQUIRE(1 == display.draw_sweep_call_count());
                REQUIRE(1 == display.draw_alert_call_count());
                REQUIRE(1 == display.flush_call_count());
                REQUIRE(ThreatLevel::Safe == display.last_threat().level);
                REQUIRE(0 == buzzer.beep_call_count());
            }
        }

        WHEN("Evaluator assesses a Danger threat")
        {
            TargetData danger_target{};
            danger_target.id       = 1;
            danger_target.x_mm     = 0;
            danger_target.y_mm     = 1000;
            danger_target.is_valid = true;

            sensor.set_targets({danger_target});
            evaluator.set_assessment({ThreatLevel::Danger, 0.85f, 1, 1.0f, -0.5f});

            engine.update();

            THEN("Display and engine receive Danger assessment and buzzer sounds")
            {
                REQUIRE(1 == evaluator.evaluate_call_count());
                REQUIRE(ThreatLevel::Danger == display.last_threat().level);
                REQUIRE(ThreatLevel::Danger == engine.current_threat_level());
                REQUIRE(0.85f == engine.current_threat().severity);
                REQUIRE(1 == buzzer.beep_call_count());
            }
        }

        WHEN("Evaluator assesses a Caution threat")
        {
            TargetData caution_target{};
            caution_target.id       = 2;
            caution_target.x_mm     = 0;
            caution_target.y_mm     = 2500;
            caution_target.is_valid = true;

            sensor.set_targets({caution_target});
            evaluator.set_assessment({ThreatLevel::Caution, 0.5f, 2, 2.5f, -0.1f});

            engine.update();

            THEN("Display receives Caution assessment and buzzer beeps at lower rate")
            {
                REQUIRE(ThreatLevel::Caution == display.last_threat().level);
                REQUIRE(ThreatLevel::Caution == engine.current_threat_level());
                REQUIRE(1 == buzzer.beep_call_count());
            }
        }

        WHEN("Time advances and sweep angle is updated")
        {
            const float angle1 = engine.current_sweep_angle();
            timer.advance_ms(500);
            engine.update();
            const float angle2 = engine.current_sweep_angle();

            THEN("Sweep angle changes smoothly")
            {
                REQUIRE(angle1 != angle2);
                REQUIRE(display.last_sweep_angle() == angle2);
            }
        }
    }
}

SCENARIO("RadarEngine reports lost sensor signal", "[radar_engine][core]")
{
    GIVEN("A sensor that stopped sending frames")
    {
        MockSensor sensor;
        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;
        MockBuzzer buzzer;

        RadarEngine engine(sensor, display, timer, evaluator, &buzzer);
        engine.init();

        TargetData stale_target{};
        stale_target.y_mm     = 1000;
        stale_target.is_valid = true;
        sensor.set_targets({stale_target});
        sensor.set_connected(false);
        evaluator.set_assessment({ThreatLevel::Danger, 0.9f, 1, 1.0f, 0.0f});

        engine.update();

        THEN("Engine drops stale targets and shows NoSignal instead of a threat level")
        {
            REQUIRE(ThreatLevel::NoSignal == engine.current_threat_level());
            REQUIRE(ThreatLevel::NoSignal == display.last_threat().level);
            REQUIRE(0 == engine.active_target_count());
            REQUIRE(0 == evaluator.evaluate_call_count());
            REQUIRE(0 == buzzer.beep_call_count());
        }
    }
}

SCENARIO("RadarEngine paces buzzer beeps like a heartbeat", "[radar_engine][core]")
{
    GIVEN("A Danger threat held for one second with a 20ms main loop")
    {
        MockSensor sensor;
        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;
        MockBuzzer buzzer;

        RadarEngine engine(sensor, display, timer, evaluator, &buzzer);
        engine.init();
        evaluator.set_assessment({ThreatLevel::Danger, 0.9f, 1, 1.0f, 0.0f});

        constexpr uint32_t LOOP_MS    = 20;
        constexpr uint32_t ITERATIONS = 50;
        for (uint32_t i = 0; i < ITERATIONS; ++i)
        {
            timer.set_millis(i * LOOP_MS);
            engine.update();
        }

        THEN("Buzzer beeps every 250ms, not on every loop")
        {
            REQUIRE(4 == buzzer.beep_call_count());
        }
    }
}
