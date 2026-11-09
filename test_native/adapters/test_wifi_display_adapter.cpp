#include "adapters/displays/wifi_display_adapter.h"
#include "core/telemetry_frame.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using namespace hrs::adapters::displays;
using namespace hrs::core;

SCENARIO("WifiDisplayAdapter streams telemetry frames", "[display][adapters][wifi]")
{
    GIVEN("A WiFi display adapter with danger 1.5 m and caution 3.5 m")
    {
        WifiDisplayAdapter display(1.5f, 3.5f);

        THEN("Initialization succeeds and nothing is sent yet")
        {
            REQUIRE(true == display.init());
            REQUIRE(display.last_frame().empty());
        }

        WHEN("A full engine draw cycle is flushed")
        {
            TargetData target{};
            target.id       = 1;
            target.x_mm     = 0;
            target.y_mm     = 1000;
            target.is_valid = true;
            const std::vector<TargetData> targets{target};

            ThreatAssessment threat{};
            threat.level    = ThreatLevel::Danger;
            threat.severity = 0.5f;

            display.clear();
            display.draw_sweep(30.0f);
            display.draw_radar(targets);
            display.draw_alert(threat);
            display.flush();

            THEN("The sent line is the shared telemetry encoding with the adapter's thresholds")
            {
                REQUIRE(encode_telemetry_frame(30.0f, threat, targets, 1.5f, 3.5f) == display.last_frame());
            }

            AND_WHEN("The next cycle has no targets")
            {
                display.clear();
                display.flush();

                THEN("The frame reports an empty target list")
                {
                    REQUIRE(std::string::npos != display.last_frame().find("\"targets\":[]"));
                }
            }
        }
    }
}
