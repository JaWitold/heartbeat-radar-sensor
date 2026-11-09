# [Feature]: Multi-sensor fusion and multi-display output (CompositeSensor / CompositeDisplay)

**Labels:** `type:feature`, `component:core`, `component:sensor`, `component:display`, `status:idea`

> Backlog item. Fields mirror `.github/ISSUE_TEMPLATE/feature_request.yml` so this can be pasted into a GitHub issue.

## Description

Today the firmware drives exactly one `ISensor` and one `IDisplay`. `scripts/build.sh` already emits
`USE_COMPOSITE_SENSOR_FUSION` / `USE_COMPOSITE_DISPLAY_BROADCAST` for multi-value `--sensors` / `--displays`,
but `src/main.cpp` rejects them with an `#error` until the composites exist.

Add the Composite pattern so `RadarEngine` (core) stays unchanged and still sees a single port:

```
RadarEngine ──> ISensor  <── CompositeSensor  ──> [LD2450 #1, LD2450 #2, ...]
            ──> IDisplay <── CompositeDisplay ──> [ST7789, Serial, ...]
```

### CompositeDisplay (small, do first)

- `src/adapters/displays/composite_display.h`: holds `IDisplay&` children; each of the 6 methods loops over them.
- No vendor code, so it is host-testable with two `MockDisplay`s.
- `main.cpp`: build each display adapter, inject the composite into `RadarEngine`.

### CompositeSensor (needs a design decision first)

1. **Mounting pose:** new core value type, e.g. `SensorMount{x_mm, y_mm, yaw_deg}`. Each child's targets are
   transformed into a common frame (pure math, lives in `src/core/`, native tests).
2. **Merge strategy — pick one:**
   - *Coverage*: sensors face different directions; union of targets, wider FoV. Simple.
   - *Accuracy*: overlapping sensors; associate targets within a gate (~300 mm) and average
     (optionally weighted by `distance_resolution_mm`). Needs association logic + tests.
3. **`specs()`:** FoV union in the common frame, max range, summed target capacity.
4. **`is_connected()`:** decide whether one dead child means `NoSignal` or a degraded mode.

## Motivation

- Wider coverage (e.g. two LD2450s back to back) or better position estimates from overlapping sensors.
- Several outputs at once (TFT on the device + JSON telemetry to the PC HUD).

## Acceptance Criteria

- [ ] `CompositeDisplay` fans out every `IDisplay` call; native test with 2 mocks.
- [ ] `./scripts/build.sh --displays st7789,serial` builds and drives both outputs.
- [ ] Merge strategy chosen and documented before `CompositeSensor` work starts.
- [ ] `CompositeSensor` transforms child targets via `SensorMount`; native tests for transform and merge.
- [ ] `./scripts/build.sh --sensors ld2450,ld2450` (or with a second sensor type) builds; `#error` guard removed.
- [ ] Simulator can feed two sensors at once (two `sim-radar.sh` instances on separate ports) for bench tests.
- [ ] Core (`src/core/radar_engine.*`) unchanged.

## Component

Core, Sensor, Display

## Additional Context — hardware limits to check before buying

- **UARTs:** ESP32 has 3; UART0 is USB/serial telemetry, so at most 2 UART radars. Second sensor needs its own
  pin set (`SENSOR2_UART_RX_PIN`, ...).
- **Radar interference:** two co-located 24 GHz FMCW radars pointing the same way may interfere (the LD2450 has no
  chirp sync). Coverage fusion is the safe win; validate accuracy fusion on the bench first.
- **Two TFTs:** TFT_eSPI is configured for one panel via global build flags (`ST7789_DRIVER`, pins). Two panels need
  separate CS pins and a library workaround — a hardware/library problem, not an architecture one.
