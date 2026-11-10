#include "adapters/timers/esp32_timer_adapter.h"
#include "core/distance_threat_evaluator.h"
#include "core/radar_engine.h"

#include <Arduino.h>

#if defined(USE_COMPOSITE_SENSOR_FUSION) || defined(USE_COMPOSITE_DISPLAY_BROADCAST)
#error "Multiple sensors/displays need CompositeSensor/CompositeDisplay, which are not implemented yet"
#endif

// Configuration contract (see .claude/rules/adapter-config.md):
//   board wiring  -> platformio.ini [env:<board>] (SERIAL_BAUD_RATE, LD2450_*_PIN, BUZZER_PIN, TFT_*)
//   tunables      -> adapter constexpr DEFAULT_*, overridable here only via ./scripts/build.sh -f NAME=value
//   selection     -> ENABLE_SENSOR_* / ENABLE_DISPLAY_* from ./scripts/build.sh

#if defined(ENABLE_SENSOR_LD2450)
#include "adapters/sensors/ld2450_adapter.h"
#ifndef LD2450_BAUD_RATE
#define LD2450_BAUD_RATE hrs::adapters::sensors::LD2450Adapter::DEFAULT_BAUD_RATE
#endif
#endif

#if defined(ENABLE_DISPLAY_SERIAL)
#include "adapters/displays/serial_display_adapter.h"
#endif

#if defined(ENABLE_DISPLAY_ST7789)
#include "adapters/displays/st7789_adapter.h"
#endif

#if defined(ENABLE_DISPLAY_WIFI)
// WiFi board and WIFI_SSID/WIFI_PASSWORD are validated by scripts/build.sh.
#include "adapters/displays/wifi_display_adapter.h"
#ifndef WIFI_TCP_PORT
#define WIFI_TCP_PORT hrs::adapters::displays::WifiDisplayAdapter::DEFAULT_TCP_PORT
#endif
#ifndef WIFI_HOSTNAME
#define WIFI_HOSTNAME hrs::adapters::displays::WifiDisplayAdapter::DEFAULT_HOSTNAME
#endif
#endif

#if defined(ENABLE_BUZZER)
#include "adapters/buzzers/esp32_buzzer_adapter.h"
#endif

namespace
{

hrs::adapters::timers::ESP32TimerAdapter timer_adapter;

#if defined(ENABLE_SENSOR_LD2450)
using SelectedSensor = hrs::adapters::sensors::LD2450Adapter;
SelectedSensor sensor_adapter(Serial2, LD2450_RX_PIN, LD2450_TX_PIN, LD2450_BAUD_RATE);
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
#elif defined(ENABLE_DISPLAY_WIFI)
hrs::adapters::displays::WifiDisplayAdapter display_adapter(
    WIFI_SSID, WIFI_PASSWORD, WIFI_TCP_PORT, WIFI_HOSTNAME, SPECS.danger_distance_m, SPECS.caution_distance_m);
#else
#error "No display selected. Build with ./scripts/build.sh (defaults: --sensors ld2450 --displays serial)"
#endif

#if defined(ENABLE_BUZZER)
hrs::adapters::buzzers::ESP32BuzzerAdapter buzzer_adapter(BUZZER_PIN);
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
