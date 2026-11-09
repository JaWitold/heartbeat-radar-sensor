// =============================================================================
// chip-ld2450.c — Wokwi Custom Chip: HLK-LD2450 Radar Sensor Emulator
// =============================================================================
// Emits 30-byte binary reporting frames at 256000 baud every 100ms into the
// ESP32 UART2 RX pin (GPIO 16).
// =============================================================================

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    uart_dev_t uart;
    timer_t timer;
    uint32_t step;
} chip_state_t;

// LD2450 signed fields: bit 15 set = positive, clear = negative; magnitude in bits 0..14.
static uint16_t sign_magnitude(int16_t value)
{
    return value > 0 ? (uint16_t)(value | 0x8000) : (uint16_t)(-value);
}

static void on_timer_tick(void *user_data)
{
    chip_state_t *chip = (chip_state_t *)user_data;

    // Build 30-byte HLK-LD2450 frame
    uint8_t frame[30];
    memset(frame, 0, sizeof(frame));

    // Header: 0xAA 0xFF 0x03 0x00
    frame[0] = 0xAA;
    frame[1] = 0xFF;
    frame[2] = 0x03;
    frame[3] = 0x00;

    // Simulate moving Target 1 (approaching from 4.5m to 1.0m, oscillating)
    chip->step = (chip->step + 1) % 100;
    int16_t x_mm = (int16_t)((chip->step > 50 ? (100 - chip->step) : chip->step) * 20 - 500);
    int16_t y_mm = (int16_t)(1500 + (chip->step > 50 ? (100 - chip->step) : chip->step) * 60);
    int16_t speed_cms = 30;
    uint16_t dist_res = 200;
    const uint16_t x_raw = sign_magnitude(x_mm);
    const uint16_t y_raw = sign_magnitude(y_mm);
    const uint16_t speed_raw = sign_magnitude(speed_cms);

    // Target 1 (Bytes 4..11)
    frame[4] = (uint8_t)(x_raw & 0xFF);
    frame[5] = (uint8_t)(x_raw >> 8);
    frame[6] = (uint8_t)(y_raw & 0xFF);
    frame[7] = (uint8_t)(y_raw >> 8);
    frame[8] = (uint8_t)(speed_raw & 0xFF);
    frame[9] = (uint8_t)(speed_raw >> 8);
    frame[10] = (uint8_t)(dist_res & 0xFF);
    frame[11] = (uint8_t)((dist_res >> 8) & 0xFF);

    // Targets 2 & 3: inactive (all 0x00)

    // Tail: 0x55 0xCC (Bytes 28..29)
    frame[28] = 0x55;
    frame[29] = 0xCC;

    // Transmit over UART at 256000 baud
    uart_write(chip->uart, frame, sizeof(frame));
}

void chip_init(void)
{
    chip_state_t *chip = (chip_state_t *)malloc(sizeof(chip_state_t));
    chip->step = 0;

    const uart_config_t uart_config = {
        .tx = pin_init("TX", INPUT),
        .rx = pin_init("RX", INPUT),
        .baud_rate = 256000,
    };
    chip->uart = uart_init(&uart_config);

    const timer_config_t timer_config = {
        .callback = on_timer_tick,
        .user_data = chip,
    };
    chip->timer = timer_init(&timer_config);

    // Periodic 100ms timer (10Hz radar stream)
    timer_start(chip->timer, 100000, true);
    printf("[chip-ld2450] Wokwi HLK-LD2450 radar emulator started.\n");
}
