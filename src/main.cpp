#include "adapters/timers/esp32_timer_adapter.h"
#include "core/distance_threat_evaluator.h"
#include "core/radar_engine.h"

#include <Arduino.h>

#if defined(USE_COMPOSITE_SENSOR_FUSION) || defined(USE_COMPOSITE_DISPLAY_BROADCAST)
#error "Multiple sensors/displays need CompositeSensor/CompositeDisplay, which are not implemented yet"
#endif

#if defined(ENABLE_SENSOR_LD2450)
#include "adapters/sensors/ld2450_adapter.h"
#endif

#if defined(ENABLE_DISPLAY_SERIAL)
#include "adapters/displays/serial_display_adapter.h"
#endif

#if defined(ENABLE_DISPLAY_ST7789)
#include "adapters/displays/st7789_adapter.h"
#endif

#if defined(ENABLE_BUZZER)
#include "adapters/buzzers/esp32_buzzer_adapter.h"
#endif

#ifndef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE 115200
#endif

namespace
{

hrs::adapters::timers::ESP32TimerAdapter timer_adapter;

#if defined(ENABLE_SENSOR_LD2450)
using SelectedSensor = hrs::adapters::sensors::LD2450Adapter;
SelectedSensor sensor_adapter(Serial2,
                              hrs::adapters::sensors::LD2450Adapter::DEFAULT_RX_PIN,
                              hrs::adapters::sensors::LD2450Adapter::DEFAULT_TX_PIN,
                              hrs::adapters::sensors::LD2450Adapter::DEFAULT_BAUD_RATE);
#else
#error "No sensor selected. Build with ./scripts/build.sh (defaults: --sensors ld2450 --displays serial)"
#endif

// Specs come from the sensor selected by the build target; RADAR_* flags override alert thresholds.
constexpr hrs::core::SensorSpecs with_overrides(const hrs::core::SensorSpecs& base)
{
    hrs::core::SensorSpecs specs = base;
#ifdef RADAR_DANGER_DISTANCE_M
    specs.danger_distance_m = RADAR_DANGER_DISTANCE_M;
#endif
#ifdef RADAR_CAUTION_DISTANCE_M
    specs.caution_distance_m = RADAR_CAUTION_DISTANCE_M;
#endif
    return specs;
}

constexpr hrs::core::SensorSpecs SPECS = with_overrides(SelectedSensor::SPECS);

#if defined(ENABLE_DISPLAY_SERIAL)
hrs::adapters::displays::SerialDisplayAdapter
    display_adapter(Serial, SPECS.danger_distance_m, SPECS.caution_distance_m);
#elif defined(ENABLE_DISPLAY_ST7789)
hrs::adapters::displays::ST7789Adapter display_adapter;
#else
#error "No display selected. Build with ./scripts/build.sh (defaults: --sensors ld2450 --displays serial)"
#endif

#if defined(ENABLE_BUZZER)
hrs::adapters::buzzers::ESP32BuzzerAdapter buzzer_adapter;
#endif

hrs::core::DistanceThreatEvaluator
    threat_evaluator(SPECS.danger_distance_m, SPECS.caution_distance_m, SPECS.max_distance_m);

hrs::core::RadarEngine engine(sensor_adapter,
                              display_adapter,
                              timer_adapter,
                              threat_evaluator,
#if defined(ENABLE_BUZZER)
                              &buzzer_adapter
#else
                              nullptr
#endif
);

} // namespace

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    Serial.println(F("[HeartbeatRadar] Booting Tactical Radar System..."));

    if (false == engine.init())
    {
        Serial.println(F("[HeartbeatRadar Error] Subsystem initialization failed!"));
    }
    else
    {
        Serial.println(F("[HeartbeatRadar] System online. Radar active."));
    }
}

void loop()
{
    engine.update();
    delay(20);
}
