#include "adapters/sensors/ld2450_adapter.h"
#include "core/radar_engine.h"
#include "core/sensor_specs.h"
#include "mocks/mock_buzzer.h"
#include "mocks/mock_display.h"
#include "mocks/mock_sensor.h"
#include "mocks/mock_threat_evaluator.h"
#include "mocks/mock_timer.h"

#include <catch2/catch_test_macros.hpp>

using namespace hrs::core;
using namespace hrs::mocks;

namespace
{
// Candidate sensors without adapters yet; values move into their adapters once written.
constexpr SensorSpecs RD03D_SPECS{8.0f, -75.0f, 75.0f, 3, 2.0f, 4.5f};
constexpr SensorSpecs RCWL0516_SPECS{7.0f, -90.0f, 90.0f, 1, 2.0f, 4.0f};
} // namespace

SCENARIO("Sensor adapters self-describe their specifications", "[radar_config][sensors][core]")
{
    GIVEN("An HLK-LD2450 adapter")
    {
        const hrs::adapters::sensors::LD2450Adapter adapter;

        THEN("It reports the LD2450 specifications")
        {
            const SensorSpecs specs = adapter.specs();
            REQUIRE(6.0f == specs.max_distance_m);
            REQUIRE(-60.0f == specs.fov_min_deg);
            REQUIRE(60.0f == specs.fov_max_deg);
            REQUIRE(3 == specs.max_tracked_targets);
            REQUIRE(1.5f == specs.danger_distance_m);
            REQUIRE(3.5f == specs.caution_distance_m);
        }

        THEN("specs() matches the compile-time SPECS constant")
        {
            constexpr SensorSpecs SPECS = hrs::adapters::sensors::LD2450Adapter::SPECS;
            REQUIRE(SPECS.max_distance_m == adapter.specs().max_distance_m);
            REQUIRE(SPECS.danger_distance_m == adapter.specs().danger_distance_m);
        }
    }
}

SCENARIO("RadarEngine auto-configuration from connected sensor specs", "[radar_config][radar_engine][core]")
{
    GIVEN("A MockSensor configured with RD-03D wide-angle specifications")
    {
        MockSensor sensor;
        sensor.set_specs(RD03D_SPECS);

        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;

        WHEN("RadarEngine is constructed without explicit FOV arguments")
        {
            RadarEngine engine(sensor, display, timer, evaluator);

            timer.set_millis(0);
            REQUIRE(true == engine.init());

            THEN("Engine automatically inherits the connected sensor's FOV and specs")
            {
                REQUIRE(-75.0f == engine.fov_min_deg());
                REQUIRE(75.0f == engine.fov_max_deg());
                REQUIRE(-75.0f == engine.current_sweep_angle());
                REQUIRE(8.0f == engine.sensor_specs().max_distance_m);
                REQUIRE(3 == engine.sensor_specs().max_tracked_targets);
            }
        }
    }

    GIVEN("A MockSensor configured with RCWL-0516 omni microwave specifications")
    {
        MockSensor sensor;
        sensor.set_specs(RCWL0516_SPECS);

        MockDisplay display;
        MockTimer timer;
        MockThreatEvaluator evaluator;

        WHEN("RadarEngine is constructed without explicit FOV arguments")
        {
            RadarEngine engine(sensor, display, timer, evaluator);

            timer.set_millis(0);
            REQUIRE(true == engine.init());

            THEN("Engine automatically inherits RCWL-0516 FOV and target limit")
            {
                REQUIRE(-90.0f == engine.fov_min_deg());
                REQUIRE(90.0f == engine.fov_max_deg());
                REQUIRE(-90.0f == engine.current_sweep_angle());
                REQUIRE(7.0f == engine.sensor_specs().max_distance_m);
                REQUIRE(1 == engine.sensor_specs().max_tracked_targets);
            }
        }
    }
}
