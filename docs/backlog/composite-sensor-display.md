# [Feature]: Multi-sensor fusion and multi-display output (CompositeSensor / CompositeDisplay)

**Labels:** `type:feature`, `component:core`, `component:sensor`, `component:display`, `status:idea`

> Backlog item. Fields mirror `.github/ISSUE_TEMPLATE/feature_request.yml`; paste into GitHub issue as-is.

## Description

Firmware drives one `ISensor`, one `IDisplay`. `scripts/build.sh` already emits
`USE_COMPOSITE_SENSOR_FUSION` / `USE_COMPOSITE_DISPLAY_BROADCAST` for multi-value `--sensors` / `--displays`;
`src/main.cpp` rejects them with `#error` until composites exist.

Composite pattern. `RadarEngine` (core) unchanged, still sees single port:

```
RadarEngine ──> ISensor  <── CompositeSensor  ──> [LD2450 #1, LD2450 #2, ...]
            ──> IDisplay <── CompositeDisplay ──> [ST7789, Serial, ...]
```

### CompositeDisplay (small, do first)

- `src/adapters/displays/composite_display.h`: holds `IDisplay&` children; each of 7 methods (`init`, `clear`, `draw_sweep`, `draw_radar`, `draw_alert`, `draw_attitude`, `flush`) loops over them.
- No vendor code → host-testable with two `MockDisplay`s.
- `main.cpp`: build each display adapter, inject composite into `RadarEngine`.

### CompositeSensor (design decision first)

1. **Mounting pose:** new core value type, e.g. `SensorMount{x_mm, y_mm, yaw_deg}`. Transform each child's targets into common frame (pure math, `src/core/`, native tests).
2. **Merge strategy — pick one:**
   - *Coverage*: sensors face different directions. Union of targets, wider FoV. Simple.
   - *Accuracy*: overlapping sensors. Associate targets within gate (~300 mm), average
     (optionally weighted by `distance_resolution_mm`). Needs association logic + tests.
3. **`specs()`:** FoV union in common frame, max range, summed target capacity.
4. **`is_connected()`:** decide: one dead child → `NoSignal`, or degraded mode.

## Motivation

- Wider coverage (two LD2450s back to back) or better position estimate from overlapping sensors.
- Several outputs at once (on-device TFT + JSON telemetry to PC HUD).

## Acceptance Criteria

- [ ] `CompositeDisplay` fans out every `IDisplay` call; native test with 2 mocks.
- [ ] `./scripts/build.sh --displays st7789,serial` builds, drives both outputs.
- [ ] Merge strategy chosen and documented before `CompositeSensor` work.
- [ ] `CompositeSensor` transforms child targets via `SensorMount`; native tests for transform and merge.
- [ ] `./scripts/build.sh --sensors ld2450,ld2450` (or second sensor type) builds; `#error` guard removed.
- [ ] Simulator feeds two sensors at once (two `sim-radar.sh` instances, separate ports) for bench tests.
- [ ] Core (`src/core/radar_engine.*`) unchanged.

## Component

Core, Sensor, Display

## Additional Context — hardware limits to check before buying

- **UARTs:** ESP32 has 3. UART0 = USB/serial telemetry → max 2 UART radars. Second sensor needs own
  pin set (`SENSOR2_UART_RX_PIN`, ...).
- **Radar interference:** two co-located 24 GHz FMCW radars facing same way may interfere (LD2450 has no
  chirp sync). Coverage fusion = safe win. Validate accuracy fusion on bench first.
- **Two TFTs:** TFT_eSPI configured for one panel via global build flags (`ST7789_DRIVER`, pins). Two panels need
  separate CS pins + library workaround. Hardware/library problem, not architecture.
