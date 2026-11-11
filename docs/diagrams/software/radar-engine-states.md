# Radar Engine Update Cycle & Threat Levels

Source: `src/core/radar_engine.cpp`, `src/core/distance_threat_evaluator.cpp`. No boot/idle state machine: engine runs `init()` once, then `update()` per loop tick.

## `update()` per tick

```mermaid
flowchart TD
    Start(["update()"]) --> Sweep["update_sweep()\n120 deg/s, bounce between specs().fov_min_deg / fov_max_deg"]
    Sweep --> HasImu{"imu_ set and\nis_connected()?"}
    HasImu -- yes --> Read{"read_imu() ok?"}
    Read -- yes --> Est["AttitudeEstimator.update(sample, dt_s or 0.02 s)"]
    Read -- no --> NoImu["has_imu_data_ = false"]
    HasImu -- no --> NoImu
    Est --> Scan
    NoImu --> Scan["raw_targets_ = sensor_.scan()"]
    Scan --> Conn{"sensor_.is_connected()?"}
    Conn -- no --> Lost["clear targets\nthreat = NoSignal"]
    Conn -- yes --> Comp{"compensation on\nand IMU data?"}
    Comp -- yes --> Xform["CoordinateTransformer::transform\n(orientation with yaw = 0)"]
    Comp -- no --> Raw["targets_ = raw_targets_"]
    Xform --> Eval["threat_evaluator_.evaluate(targets_)"]
    Raw --> Eval
    Eval --> Draw
    Lost --> Draw["display: clear → draw_sweep → draw_radar\n→ draw_alert → draw_attitude → flush"]
    Draw --> Beep["update_buzzer()"]
    Beep --> End(["return"])
```

Yaw forced to 0: gyro-only yaw drifts, no compass. Only fixed `mounting_yaw_deg` applies.

## Threat level (re-evaluated every tick)

```mermaid
stateDiagram-v2
    [*] --> Safe
    Safe --> Caution: min distance <= caution_m
    Safe --> Danger: min distance < danger_m
    Caution --> Danger: min distance < danger_m
    Caution --> Safe: min distance > caution_m or no valid targets
    Danger --> Caution: danger_m <= min distance <= caution_m
    Danger --> Safe: min distance > caution_m or no valid targets
    Safe --> NoSignal: is_connected() false
    Caution --> NoSignal: is_connected() false
    Danger --> NoSignal: is_connected() false
    NoSignal --> Safe: frames resume, evaluate()
```

- No hysteresis: level is pure function of current closest valid target.
- `LD2450Adapter::SPECS`: danger 1.5 m, caution 3.5 m, max 6.0 m (override `RADAR_DANGER_DISTANCE_M`, `RADAR_CAUTION_DISTANCE_M`).
- `severity` = 1 − clamp(min_distance / max_distance_m, 0, 1).
- `NoSignal`: `LD2450Adapter` reports disconnected after `SIGNAL_TIMEOUT_MS` = 500 ms without frames.
- From `NoSignal`, the first tick with frames re-evaluates straight to whichever level applies.

## Buzzer pacing (`update_buzzer()`)

| Level | Interval | Tone |
|---|---|---|
| Danger | 250 ms | 2500 Hz, 50 ms |
| Caution | 1000 ms | 1500 Hz, 20 ms |
| Safe / NoSignal | none; next threat beeps immediately | — |
