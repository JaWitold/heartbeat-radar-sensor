# Class Hierarchy & Interfaces

Source: `src/ports/`, `src/core/`, `src/adapters/`, `test_native/mocks/`. Public API only.

## Ports + implementations

```mermaid
classDiagram
    class ISensor {
        <<interface>>
        +init() bool
        +scan() vector~TargetData~
        +specs() SensorSpecs
        +is_connected() bool
    }

    class IDisplay {
        <<interface>>
        +init() bool
        +clear() void
        +draw_sweep(float angle_deg) void
        +draw_radar(vector~TargetData~ targets) void
        +draw_alert(ThreatAssessment threat) void
        +draw_attitude(Orientation orientation, ImuData* imu) void
        +flush() void
    }

    class ITimer {
        <<interface>>
        +millis() uint32_t
        +delay_ms(uint32_t ms) void
    }

    class IBuzzer {
        <<interface>>
        +beep(uint16_t freq_hz, uint16_t duration_ms) void
        +stop() void
    }

    class IImu {
        <<interface>>
        +init() bool
        +is_connected() bool
        +read_imu(ImuData& out) bool
    }

    class IThreatEvaluator {
        <<interface>>
        +evaluate(vector~TargetData~ targets) ThreatAssessment
    }

    class LD2450Adapter {
        +SPECS$ SensorSpecs
        +DEFAULT_BAUD_RATE$ 256000
        -LD2450Parser parser_
    }

    class ITelemetryDisplay {
        <<abstract>>
        #send_line(string line) void*
    }
    class SerialDisplayAdapter
    class WifiDisplayAdapter
    class ST7789Adapter {
        -TFT_eSPI tft_
    }

    class ESP32TimerAdapter
    class ESP32BuzzerAdapter
    class MPU6050Adapter {
        -TwoWire& wire_
    }
    class DistanceThreatEvaluator {
        +DistanceThreatEvaluator(float danger_dist_m, float caution_dist_m, float max_dist_m)
    }

    ISensor <|.. LD2450Adapter
    ISensor <|.. MockSensor
    IDisplay <|.. ITelemetryDisplay
    ITelemetryDisplay <|-- SerialDisplayAdapter
    ITelemetryDisplay <|-- WifiDisplayAdapter
    IDisplay <|.. ST7789Adapter
    IDisplay <|.. MockDisplay
    ITimer <|.. ESP32TimerAdapter
    ITimer <|.. MockTimer
    IBuzzer <|.. ESP32BuzzerAdapter
    IBuzzer <|.. MockBuzzer
    IImu <|.. MPU6050Adapter
    IImu <|.. MockImu
    IThreatEvaluator <|.. DistanceThreatEvaluator
    IThreatEvaluator <|.. MockThreatEvaluator
```

`ITelemetryDisplay` (`src/adapters/displays/`): buffers draw calls, `flush()` sends one `encode_telemetry_frame()` JSON line via `send_line()`. Serial → UART, WiFi → TCP client.

## Core

```mermaid
classDiagram
    class RadarEngine {
        +RadarEngine(ISensor&, IDisplay&, ITimer&, IThreatEvaluator&, IBuzzer* = nullptr, IImu* = nullptr)
        +init() bool
        +update() void
        +active_target_count() size_t
        +current_threat_level() ThreatLevel
        +current_threat() ThreatAssessment
        +current_sweep_angle() float
        +fov_min_deg() float
        +fov_max_deg() float
        +sensor_specs() SensorSpecs
        +has_imu() bool
        +latest_imu() ImuData
        +current_orientation() Orientation
        +targets() vector~TargetData~
        +raw_targets() vector~TargetData~
        +set_attitude_compensation(bool) void
        +is_attitude_compensated() bool
        +set_transform_config(TransformConfig) void
        +transform_config() TransformConfig
    }

    class AttitudeEstimator {
        +DEFAULT_ALPHA$ 0.98
        +reset() void
        +update(ImuData imu, float dt_s) void
        +orientation() Orientation
    }

    class CoordinateTransformer {
        +transform(TargetData, Orientation, TransformConfig)$ TargetData
        +transform(vector~TargetData~, Orientation, TransformConfig)$ vector~TargetData~
    }

    class LD2450Parser {
        +FRAME_SIZE$ 30
        +parse_byte(uint8_t byte) bool
        +get_latest_targets() vector~TargetData~
        +reset() void
    }

    class TargetData {
        +uint8_t id
        +int16_t x_mm
        +int16_t y_mm
        +int16_t speed_cms
        +uint16_t distance_resolution_mm
        +bool is_valid
        +distance_m() float
        +angle_deg() float
        +zone(float danger_m, float caution_m) TargetZone
    }

    class TargetZone {
        <<enumeration>>
        Safe
        Caution
        Danger
    }

    class ThreatLevel {
        <<enumeration>>
        Safe
        Caution
        Danger
        NoSignal
    }

    class ThreatAssessment {
        +ThreatLevel level
        +float severity
        +uint8_t primary_target_id
        +float min_distance_m
        +float closing_speed_mps
    }

    class SensorSpecs {
        +float max_distance_m
        +float fov_min_deg
        +float fov_max_deg
        +uint8_t max_tracked_targets
        +float danger_distance_m
        +float caution_distance_m
    }

    class ImuData {
        +AccelData accel
        +GyroData gyro
        +float temp_c
        +bool is_valid
    }

    class Orientation {
        +float pitch_deg
        +float roll_deg
        +float yaw_deg
    }

    class TransformConfig {
        +bool compensate_pitch
        +bool compensate_roll
        +bool compensate_yaw
        +float mounting_yaw_deg
        +float mounting_pitch_deg
        +float mounting_roll_deg
    }

    RadarEngine o-- ISensor
    RadarEngine o-- IDisplay
    RadarEngine o-- ITimer
    RadarEngine o-- IThreatEvaluator
    RadarEngine o-- IBuzzer : optional
    RadarEngine o-- IImu : optional
    RadarEngine *-- AttitudeEstimator
    RadarEngine *-- TransformConfig
    RadarEngine ..> CoordinateTransformer
    RadarEngine ..> ThreatAssessment
    ThreatAssessment --> ThreatLevel
    TargetData ..> TargetZone
    AttitudeEstimator ..> ImuData
    AttitudeEstimator --> Orientation
```

Free functions in core: `parse_mpu6050_burst()` (`mpu6050_parser.h`, used by `MPU6050Adapter`), `encode_telemetry_frame()` (`telemetry_frame.h`, used by `ITelemetryDisplay`).
