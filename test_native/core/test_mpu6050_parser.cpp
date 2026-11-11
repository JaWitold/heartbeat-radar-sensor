#include "core/mpu6050_parser.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <vector>

using namespace hrs::core;
using Catch::Approx;

SCENARIO("MPU6050 burst packet parsing", "[mpu6050][core]")
{
    GIVEN("A nullptr or undersized buffer")
    {
        ImuData out{};

        WHEN("Buffer pointer is null")
        {
            const bool ok = parse_mpu6050_burst(nullptr, 14, out);

            THEN("Parsing fails and data is invalid")
            {
                REQUIRE(false == ok);
                REQUIRE(false == out.is_valid);
            }
        }

        WHEN("Buffer has fewer than 14 bytes")
        {
            const uint8_t short_buf[13] = {0};
            const bool ok               = parse_mpu6050_burst(short_buf, sizeof(short_buf), out);

            THEN("Parsing fails and data is invalid")
            {
                REQUIRE(false == ok);
                REQUIRE(false == out.is_valid);
            }
        }
    }

    GIVEN("A valid 14-byte packet representing a level resting sensor (1g along Z)")
    {
        // Big-endian 16-bit values:
        // Accel X = 0       -> 0x00, 0x00
        // Accel Y = 0       -> 0x00, 0x00
        // Accel Z = 16384   -> 0x40, 0x00
        // Temp    = 0       -> 0x00, 0x00  (36.53 °C)
        // Gyro X  = 0       -> 0x00, 0x00
        // Gyro Y  = 0       -> 0x00, 0x00
        // Gyro Z  = 0       -> 0x00, 0x00
        const uint8_t packet[14] = {
            0x00,
            0x00, // AX
            0x00,
            0x00, // AY
            0x40,
            0x00, // AZ (16384 = 1.0g @ ±2g)
            0x00,
            0x00, // Temp
            0x00,
            0x00, // GX
            0x00,
            0x00, // GY
            0x00,
            0x00 // GZ
        };

        ImuData out{};

        WHEN("Parsed with default ±2g and ±250 deg/s scales")
        {
            const bool ok = parse_mpu6050_burst(packet, sizeof(packet), out);

            THEN("Parsing succeeds and physical units match")
            {
                REQUIRE(true == ok);
                REQUIRE(true == out.is_valid);
                REQUIRE(0.0f == Approx(out.accel.x).margin(0.001f));
                REQUIRE(0.0f == Approx(out.accel.y).margin(0.001f));
                REQUIRE(1.0f == Approx(out.accel.z).margin(0.001f));
                REQUIRE(1.0f == Approx(out.accel.magnitude_g()).margin(0.001f));
                REQUIRE(36.53f == Approx(out.temp_c).margin(0.01f));
                REQUIRE(0.0f == Approx(out.gyro.x).margin(0.001f));
                REQUIRE(0.0f == Approx(out.gyro.y).margin(0.001f));
                REQUIRE(0.0f == Approx(out.gyro.z).margin(0.001f));
            }
        }
    }

    GIVEN("A packet with negative two's-complement values and active rotation")
    {
        // Accel X = -8192   -> 0xE0, 0x00  (-0.5g @ ±2g)
        // Accel Y = +4096   -> 0x10, 0x00  (+0.25g @ ±2g)
        // Accel Z = -16384  -> 0xC0, 0x00  (-1.0g @ ±2g)
        // Temp    = -1000   -> 0xFC, 0x18  ((-1000 / 340.0) + 36.53 = ~33.588 °C)
        // Gyro X  = -1310   -> 0xFA, 0xE2  (-10.0 deg/s @ ±250 deg/s)
        // Gyro Y  = +2620   -> 0x0A, 0x3C  (+20.0 deg/s @ ±250 deg/s)
        // Gyro Z  = -655    -> 0xFD, 0x71  (-5.0 deg/s @ ±250 deg/s)
        const uint8_t packet[14] = {
            0xE0,
            0x00, // AX: -8192
            0x10,
            0x00, // AY: +4096
            0xC0,
            0x00, // AZ: -16384
            0xFC,
            0x18, // Temp: -1000
            0xFA,
            0xE2, // GX: -1310
            0x0A,
            0x3C, // GY: +2620
            0xFD,
            0x71 // GZ: -655
        };

        ImuData out{};

        WHEN("Parsed")
        {
            const bool ok = parse_mpu6050_burst(packet, sizeof(packet), out);

            THEN("Negative signs and values are preserved")
            {
                REQUIRE(true == ok);
                REQUIRE(true == out.is_valid);
                REQUIRE(-0.5f == Approx(out.accel.x).margin(0.01f));
                REQUIRE(0.25f == Approx(out.accel.y).margin(0.01f));
                REQUIRE(-1.0f == Approx(out.accel.z).margin(0.01f));
                REQUIRE(33.59f == Approx(out.temp_c).margin(0.02f));
                REQUIRE(-10.0f == Approx(out.gyro.x).margin(0.02f));
                REQUIRE(20.0f == Approx(out.gyro.y).margin(0.02f));
                REQUIRE(-5.0f == Approx(out.gyro.z).margin(0.02f));
            }
        }
    }

    GIVEN("Custom full-scale sensitivity ranges (±4g and ±500 deg/s)")
    {
        // At ±4g, 8192 LSB/g. Raw 8192 -> 1.0g
        // At ±500 deg/s, 65.5 LSB/(deg/s). Raw 655 -> 10.0 deg/s
        const uint8_t packet[14] = {
            0x20,
            0x00, // AX: 8192
            0x00,
            0x00, // AY: 0
            0x00,
            0x00, // AZ: 0
            0x00,
            0x00, // Temp
            0x02,
            0x8F, // GX: 655
            0x00,
            0x00, // GY: 0
            0x00,
            0x00 // GZ: 0
        };

        ImuData out{};

        WHEN("Parsed with G4 and Deg500")
        {
            const bool ok = parse_mpu6050_burst(packet, sizeof(packet), out, AccelScale::G4, GyroScale::Deg500);

            THEN("Sensitivity divides correctly")
            {
                REQUIRE(true == ok);
                REQUIRE(1.0f == Approx(out.accel.x).margin(0.01f));
                REQUIRE(10.0f == Approx(out.gyro.x).margin(0.05f));
            }
        }
    }
}
