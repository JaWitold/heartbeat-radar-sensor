#include "adapters/buzzers/esp32_buzzer_adapter.h"

#include <catch2/catch_test_macros.hpp>

using namespace hrs::adapters::buzzers;

SCENARIO("ESP32BuzzerAdapter operations", "[buzzer][adapters]")
{
    GIVEN("A buzzer adapter wired to GPIO 25")
    {
        ESP32BuzzerAdapter buzzer(25);

        THEN("The pin is recorded and buzzer is idle")
        {
            REQUIRE(25 == buzzer.pin());
            REQUIRE(false == buzzer.is_beeping());
            REQUIRE(0 == buzzer.last_freq_hz());
            REQUIRE(0 == buzzer.last_duration_ms());
        }

        WHEN("beep() is triggered")
        {
            buzzer.beep(2500, 50);

            THEN("Buzzer state transitions to beeping with recorded parameters")
            {
                REQUIRE(true == buzzer.is_beeping());
                REQUIRE(2500 == buzzer.last_freq_hz());
                REQUIRE(50 == buzzer.last_duration_ms());
            }
        }

        WHEN("stop() is triggered after beeping")
        {
            buzzer.beep(1500, 20);
            buzzer.stop();

            THEN("Buzzer stops beeping")
            {
                REQUIRE(false == buzzer.is_beeping());
            }
        }
    }

    GIVEN("A buzzer adapter configured with custom GPIO 14")
    {
        ESP32BuzzerAdapter buzzer(14);

        THEN("Custom pin is recorded")
        {
            REQUIRE(14 == buzzer.pin());
        }
    }
}
