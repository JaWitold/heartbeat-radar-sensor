#!/usr/bin/env bash
# =============================================================================
# build.sh — Build firmware with multi-sensor & multi-display targets
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

info()    { echo -e "\033[1;34m[INFO]\033[0m  $*"; }
success() { echo -e "\033[1;32m[OK]\033[0m    $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }

# Default orthogonal targets
BOARD="esp32dev"

show_help() {
    cat <<EOF
Usage: ./scripts/build.sh [OPTIONS]

Build firmware with composable multi-sensor and multi-display targets.

Options:
  -b, --board BOARD             Target board (default: esp32dev)
  -s, --sensor, --sensors LIST  Sensor(s), comma-separated or repeated:
                                  ld2450 (only one implemented so far)
                                  Default: ld2450
  -d, --display, --displays LIST Display/output sinks, comma-separated or repeated:
                                  serial | st7789 (TFT renderer is still a stub)
                                  wifi (TCP telemetry; WiFi boards only; needs
                                        env WIFI_SSID and WIFI_PASSWORD)
                                  Default: serial
  -i, --imu, --imus LIST        Inertial measurement unit / motion module(s):
                                  none | mpu6050
                                  Default: none
  -f, --flag, --define MACRO    Extra preprocessor definition, repeated:
                                  Example: -f RADAR_DANGER_DISTANCE_M=2.0f
                                  Example: -f RADAR_MOUNTING_YAW_DEG=45.0f
                                  Example: -f WIFI_TCP_PORT=4000
                                  (adapter tunables only; board wiring lives in platformio.ini)
  -h, --help                    Show this help message
EOF
}

# Collect sensor and display arguments (supports repeated flags or comma-separated lists)
COLLECTED_SENSORS=()
COLLECTED_DISPLAYS=()
COLLECTED_IMUS=()
COLLECTED_DEFINES=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--board)
            BOARD="$2"
            shift 2
            ;;
        -s|--sensor|--sensors)
            IFS=',' read -ra ADDR <<< "$2"
            for i in "${ADDR[@]}"; do
                COLLECTED_SENSORS+=("$i")
            done
            shift 2
            ;;
        -d|--display|--displays)
            IFS=',' read -ra ADDR <<< "$2"
            for i in "${ADDR[@]}"; do
                COLLECTED_DISPLAYS+=("$i")
            done
            shift 2
            ;;
        -i|--imu|--imus)
            IFS=',' read -ra ADDR <<< "$2"
            for i in "${ADDR[@]}"; do
                COLLECTED_IMUS+=("$i")
            done
            shift 2
            ;;
        -f|--flag|--define)
            COLLECTED_DEFINES+=("$2")
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            error "Unknown argument: $1"
            show_help
            exit 1
            ;;
    esac
done

# Apply defaults if none provided
if [ ${#COLLECTED_SENSORS[@]} -eq 0 ]; then
    COLLECTED_SENSORS=("ld2450")
fi
if [ ${#COLLECTED_DISPLAYS[@]} -eq 0 ]; then
    COLLECTED_DISPLAYS=("serial")
fi
if [ ${#COLLECTED_IMUS[@]} -eq 0 ]; then
    COLLECTED_IMUS=("none")
fi

# Reject names main.cpp has no adapter for, instead of silently building something else
for s in "${COLLECTED_SENSORS[@]}"; do
    case "$s" in ld2450) ;; *) error "Unsupported sensor: $s"; exit 1 ;; esac
done
for d in "${COLLECTED_DISPLAYS[@]}"; do
    case "$d" in serial|st7789|wifi) ;; *) error "Unsupported display: $d"; exit 1 ;; esac
done
for m in "${COLLECTED_IMUS[@]}"; do
    case "$m" in none|mpu6050) ;; *) error "Unsupported IMU: $m"; exit 1 ;; esac
done

# Build preprocessor flags
BUILD_FLAGS=""

info "Configuring build with composable targets:"
info "  Board:     $BOARD"
info "  Sensor(s): ${COLLECTED_SENSORS[*]}"
for s in "${COLLECTED_SENSORS[@]}"; do
    FLAG="-DENABLE_SENSOR_$(echo "$s" | tr '[:lower:]' '[:upper:]')=1"
    BUILD_FLAGS+="${FLAG} "
done
if [ ${#COLLECTED_SENSORS[@]} -gt 1 ]; then
    BUILD_FLAGS+="-DUSE_COMPOSITE_SENSOR_FUSION=1 "
    info "             -> Multi-sensor fusion active (${#COLLECTED_SENSORS[@]} sensors)"
fi

info "  Display(s): ${COLLECTED_DISPLAYS[*]}"
for d in "${COLLECTED_DISPLAYS[@]}"; do
    FLAG="-DENABLE_DISPLAY_$(echo "$d" | tr '[:lower:]' '[:upper:]')=1"
    BUILD_FLAGS+="${FLAG} "
done
if [ ${#COLLECTED_DISPLAYS[@]} -gt 1 ]; then
    BUILD_FLAGS+="-DUSE_COMPOSITE_DISPLAY_BROADCAST=1 "
    info "             -> Multi-display broadcasting active (${#COLLECTED_DISPLAYS[@]} sinks)"
fi

info "  IMU(s):    ${COLLECTED_IMUS[*]}"
for m in "${COLLECTED_IMUS[@]}"; do
    if [ "$m" != "none" ]; then
        FLAG="-DENABLE_IMU_$(echo "$m" | tr '[:lower:]' '[:upper:]')=1"
        BUILD_FLAGS+="${FLAG} "
    fi
done

if [[ " ${COLLECTED_DISPLAYS[*]} " == *" wifi "* ]]; then
    case "$BOARD" in esp32dev) ;; *) error "Display 'wifi' needs a WiFi-capable board, got: $BOARD"; exit 1 ;; esac
    if [ -z "${WIFI_SSID:-}" ] || [ -z "${WIFI_PASSWORD:-}" ]; then
        error "Display 'wifi' needs WIFI_SSID and WIFI_PASSWORD env vars (kept out of git and logs)"
        exit 1
    fi
    if [[ "${WIFI_SSID}${WIFI_PASSWORD}" == *[\'\"\\]* ]]; then
        error "WIFI_SSID/WIFI_PASSWORD must not contain quotes or backslashes"
        exit 1
    fi
    # Single quotes keep spaces intact when PlatformIO splits PLATFORMIO_BUILD_FLAGS.
    BUILD_FLAGS+="'-DWIFI_SSID=\"${WIFI_SSID}\"' '-DWIFI_PASSWORD=\"${WIFI_PASSWORD}\"' "
    info "  WiFi:       credentials from env (not printed)"
fi

if [ ${#COLLECTED_DEFINES[@]} -gt 0 ]; then
    info "  Define(s):  ${COLLECTED_DEFINES[*]}"
    for def in "${COLLECTED_DEFINES[@]}"; do
        BUILD_FLAGS+="-D${def} "
    done
fi

export PLATFORMIO_BUILD_FLAGS="${BUILD_FLAGS}"

docker compose -f "${PROJECT_ROOT}/docker/docker-compose.yml" run --rm \
    -e PLATFORMIO_BUILD_FLAGS \
    build pio run -e "$BOARD"

success "Firmware build complete."
