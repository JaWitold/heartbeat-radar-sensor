#include "core/ld2450_parser.h"

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <vector>

using namespace hrs::core;

namespace
{

// LD2450 wire format: bit 15 set = positive, clear = negative; magnitude in bits 0..14.
void put_signed(std::vector<uint8_t>& frame, const size_t offset, const int16_t value)
{
    const uint16_t raw = (0 < value) ? static_cast<uint16_t>(value | 0x8000) : static_cast<uint16_t>(-value);
    frame[offset]      = static_cast<uint8_t>(raw & 0xFF);
    frame[offset + 1]  = static_cast<uint8_t>(raw >> 8);
}

std::vector<uint8_t> make_test_frame(const int16_t x1,
                                     const int16_t y1,
                                     const int16_t speed1,
                                     const uint16_t res1,
                                     const int16_t x2     = 0,
                                     const int16_t y2     = 0,
                                     const int16_t speed2 = 0,
                                     const uint16_t res2  = 0,
                                     const int16_t x3     = 0,
                                     const int16_t y3     = 0,
                                     const int16_t speed3 = 0,
                                     const uint16_t res3  = 0)
{
    std::vector<uint8_t> frame(30, 0);

    // Header: 0xAA 0xFF 0x03 0x00
    frame[0] = 0xAA;
    frame[1] = 0xFF;
    frame[2] = 0x03;
    frame[3] = 0x00;

    // Target 1 (bytes 4..11)
    put_signed(frame, 4, x1);
    put_signed(frame, 6, y1);
    put_signed(frame, 8, speed1);
    std::memcpy(&frame[10], &res1, sizeof(uint16_t));

    // Target 2 (bytes 12..19)
    put_signed(frame, 12, x2);
    put_signed(frame, 14, y2);
    put_signed(frame, 16, speed2);
    std::memcpy(&frame[18], &res2, sizeof(uint16_t));

    // Target 3 (bytes 20..27)
    put_signed(frame, 20, x3);
    put_signed(frame, 22, y3);
    put_signed(frame, 24, speed3);
    std::memcpy(&frame[26], &res3, sizeof(uint16_t));

    // Tail: 0x55 0xCC
    frame[28] = 0x55;
    frame[29] = 0xCC;

    return frame;
}

} // namespace

SCENARIO("LD2450Parser decodes the datasheet example frame", "[ld2450][core]")
{
    // HLK-LD2450 datasheet example: target 1 at x=-782mm, y=1713mm, speed=-16cm/s, resolution=320mm
    const std::vector<uint8_t> frame = {0xAA, 0xFF, 0x03, 0x00, 0x0E, 0x03, 0xB1, 0x86, 0x10, 0x00,
                                        0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xCC};
    LD2450Parser parser;
    const auto targets = parser.parse_buffer(frame.data(), frame.size());

    REQUIRE(1 == targets.size());
    REQUIRE(-782 == targets[0].x_mm);
    REQUIRE(1713 == targets[0].y_mm);
    REQUIRE(-16 == targets[0].speed_cms);
    REQUIRE(320 == targets[0].distance_resolution_mm);
}

SCENARIO("LD2450Parser decodes valid binary frames", "[ld2450][core]")
{
    GIVEN("A default initialized LD2450Parser")
    {
        LD2450Parser parser;

        WHEN("A frame with one active target is parsed")
        {
            // Target 1: x = 500mm, y = 2000mm, speed = -20 cm/s, res = 100mm
            const auto frame   = make_test_frame(500, 2000, -20, 100);
            const auto targets = parser.parse_buffer(frame.data(), frame.size());

            THEN("One valid target is returned")
            {
                REQUIRE(1 == targets.size());
                REQUIRE(1 == targets[0].id);
                REQUIRE(500 == targets[0].x_mm);
                REQUIRE(2000 == targets[0].y_mm);
                REQUIRE(-20 == targets[0].speed_cms);
                REQUIRE(100 == targets[0].distance_resolution_mm);
                REQUIRE(true == targets[0].is_valid);
            }
        }

        WHEN("A frame with three active targets is parsed")
        {
            const auto frame   = make_test_frame(-300, 1500, 10, 50, 800, 2500, -40, 75, 0, 4000, 0, 100);
            const auto targets = parser.parse_buffer(frame.data(), frame.size());

            THEN("Three valid targets are returned with matching slot IDs")
            {
                REQUIRE(3 == targets.size());
                REQUIRE(1 == targets[0].id);
                REQUIRE(-300 == targets[0].x_mm);
                REQUIRE(2 == targets[1].id);
                REQUIRE(800 == targets[1].x_mm);
                REQUIRE(3 == targets[2].id);
                REQUIRE(4000 == targets[2].y_mm);
            }
        }

        WHEN("A frame has all zero slots")
        {
            const auto frame   = make_test_frame(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            const auto targets = parser.parse_buffer(frame.data(), frame.size());

            THEN("Zero targets are returned")
            {
                REQUIRE(0 == targets.size());
            }
        }
    }
}

SCENARIO("LD2450Parser handles noisy streams and fragmented bytes", "[ld2450][core]")
{
    GIVEN("An LD2450Parser")
    {
        LD2450Parser parser;

        WHEN("Garbage noise precedes a valid frame")
        {
            std::vector<uint8_t> stream = {0x12, 0x34, 0xAA, 0x00, 0xFF, 0xAA};
            const auto valid_frame      = make_test_frame(250, 1200, -15, 60);
            stream.insert(stream.end(), valid_frame.begin(), valid_frame.end());

            const auto targets = parser.parse_buffer(stream.data(), stream.size());

            THEN("The noise is discarded and the valid frame is parsed")
            {
                REQUIRE(1 == targets.size());
                REQUIRE(250 == targets[0].x_mm);
                REQUIRE(1200 == targets[0].y_mm);
            }
        }

        WHEN("A valid frame is fed byte-by-byte")
        {
            const auto frame = make_test_frame(400, 1800, 5, 80);
            bool completed   = false;

            for (size_t i = 0; i < frame.size(); ++i)
            {
                if (true == parser.parse_byte(frame[i]))
                {
                    completed = true;
                }
            }

            THEN("parse_byte signals completion on the final tail byte")
            {
                REQUIRE(true == completed);
                const auto& targets = parser.get_latest_targets();
                REQUIRE(1 == targets.size());
                REQUIRE(400 == targets[0].x_mm);
            }
        }

        WHEN("A frame has an invalid tail")
        {
            auto frame = make_test_frame(400, 1800, 5, 80);
            frame[29]  = 0xAA; // Corrupt tail (expected 0xCC)

            const auto targets = parser.parse_buffer(frame.data(), frame.size());

            THEN("The corrupted frame is discarded")
            {
                REQUIRE(0 == targets.size());
            }
        }
    }
}
