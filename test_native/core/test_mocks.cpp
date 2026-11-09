#include "mocks/mock_buzzer.h"
#include "mocks/mock_display.h"
#include "mocks/mock_sensor.h"
#include "mocks/mock_timer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace hrs;
using Catch::Approx;

SCENARIO("MockSensor implements ISensor port contract", "[mocks][sensor]")
{
    GIVEN("A MockSensor accessed through ISensor interface")
    {
        mocks::MockSensor mock{};
        ports::ISensor& sensor = mock;

        THEN("Default initialization succeeds and reports connected")
        {
            REQUIRE(true == sensor.init());
            REQUIRE(true == mock.init_called());
            REQUIRE(true == sensor.is_connected());
        }

        WHEN("Target contacts are injected")
        {
            core::TargetData t1{};
            t1.id       = 1;
            t1.x_mm     = 500;
            t1.y_mm     = 2000;
            t1.is_valid = true;

            mock.set_targets({t1});

            THEN("scan() returns injected targets and increments call counter")
            {
                const auto contacts = sensor.scan();
                REQUIRE(1 == contacts.size());
                REQUIRE(1 == contacts[0].id);
                REQUIRE(1 == mock.scan_call_count());
            }
        }
    }
}

SCENARIO("MockDisplay implements IDisplay port contract", "[mocks][display]")
{
    GIVEN("A MockDisplay accessed through IDisplay interface")
    {
        mocks::MockDisplay mock{};
        ports::IDisplay& display = mock;

        THEN("Display operations record state and invocation counts")
        {
            REQUIRE(true == display.init());
            REQUIRE(true == mock.init_called());

            display.clear();
            REQUIRE(1 == mock.clear_call_count());

            display.draw_sweep(45.5f);
            REQUIRE(1 == mock.draw_sweep_call_count());
            REQUIRE(Approx(45.5f) == mock.last_sweep_angle());

            core::TargetData t1{};
            t1.id       = 1;
            t1.x_mm     = -800;
            t1.y_mm     = 1200;
            t1.is_valid = true;

            display.draw_radar({t1});
            REQUIRE(1 == mock.draw_radar_call_count());
            REQUIRE(1 == mock.last_targets().size());

            core::ThreatAssessment threat{};
            threat.level             = core::ThreatLevel::Danger;
            threat.severity          = 0.85f;
            threat.primary_target_id = 1;

            display.draw_alert(threat);
            REQUIRE(1 == mock.draw_alert_call_count());
            REQUIRE(core::ThreatLevel::Danger == mock.last_threat().level);
            REQUIRE(0.85f == mock.last_threat().severity);

            display.flush();
            REQUIRE(1 == mock.flush_call_count());
        }
    }
}

SCENARIO("MockTimer implements ITimer port contract", "[mocks][timer]")
{
    GIVEN("A MockTimer accessed through ITimer interface")
    {
        mocks::MockTimer mock{};
        ports::ITimer& timer = mock;

        THEN("Initial time is zero")
        {
            REQUIRE(0 == timer.millis());
        }

        WHEN("Time is advanced synthetically")
        {
            mock.advance_ms(500);

            THEN("millis() reports advanced time")
            {
                REQUIRE(500 == timer.millis());
            }
        }

        WHEN("delay_ms is called")
        {
            timer.delay_ms(150);

            THEN("Time increases and call count increments")
            {
                REQUIRE(150 == timer.millis());
                REQUIRE(1 == mock.delay_call_count());
            }
        }
    }
}

SCENARIO("MockBuzzer implements IBuzzer port contract", "[mocks][buzzer]")
{
    GIVEN("A MockBuzzer accessed through IBuzzer interface")
    {
        mocks::MockBuzzer mock{};
        ports::IBuzzer& buzzer = mock;

        THEN("Default state is silent")
        {
            REQUIRE(false == mock.is_beeping());
            REQUIRE(0 == mock.beep_call_count());
        }

        WHEN("beep() is triggered")
        {
            buzzer.beep(2400, 100);

            THEN("Beep parameters and state are recorded")
            {
                REQUIRE(true == mock.is_beeping());
                REQUIRE(1 == mock.beep_call_count());
                REQUIRE(2400 == mock.last_freq_hz());
                REQUIRE(100 == mock.last_duration_ms());
            }

            AND_WHEN("stop() is called")
            {
                buzzer.stop();

                THEN("State transitions back to silent")
                {
                    REQUIRE(false == mock.is_beeping());
                    REQUIRE(1 == mock.stop_call_count());
                }
            }
        }
    }
}
