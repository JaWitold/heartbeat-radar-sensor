#include "ld2450_parser.h"

#include "core/target_data.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace hrs::core
{

namespace
{

// LD2450 signed fields are sign-magnitude, not two's complement:
// bit 15 set = positive, clear = negative; magnitude in bits 0..14.
inline int16_t read_signed_le(const uint8_t* ptr)
{
    constexpr uint8_t SIGN_BIT       = 0x80;
    constexpr uint8_t MAGNITUDE_MASK = 0x7F;
    const auto magnitude             = static_cast<int16_t>(((ptr[1] & MAGNITUDE_MASK) << 8) | ptr[0]);
    return (0 != (ptr[1] & SIGN_BIT)) ? magnitude : static_cast<int16_t>(-magnitude);
}

inline uint16_t read_u16_le(const uint8_t* ptr)
{
    const auto lo = static_cast<uint16_t>(ptr[0]);
    const auto hi = static_cast<uint16_t>(ptr[1]);
    return static_cast<uint16_t>(lo | (hi << 8));
}

} // namespace

bool LD2450Parser::parse_byte(const uint8_t byte)
{
    buffer_.push_back(byte);
    search_and_align_header();

    if (FRAME_SIZE <= buffer_.size())
    {
        if (TAIL_BYTE_0 == buffer_[28] && TAIL_BYTE_1 == buffer_[29])
        {
            decode_frame(buffer_.data());
            buffer_.erase(buffer_.begin(), buffer_.begin() + FRAME_SIZE);
            return true;
        }

        // Invalid tail, drop first byte to advance search
        buffer_.erase(buffer_.begin());
        search_and_align_header();
    }

    return false;
}

const std::vector<TargetData>& LD2450Parser::get_latest_targets() const
{
    return latest_targets_;
}

void LD2450Parser::reset()
{
    buffer_.clear();
    latest_targets_.clear();
}

void LD2450Parser::search_and_align_header()
{
    while (false == buffer_.empty())
    {
        // Check if header matches up to available bytes
        const size_t check_len = std::min(buffer_.size(), HEADER_SIZE);
        bool match             = true;

        if (check_len >= 1 && HEADER_BYTE_0 != buffer_[0])
        {
            match = false;
        }
        if (check_len >= 2 && HEADER_BYTE_1 != buffer_[1])
        {
            match = false;
        }
        if (check_len >= 3 && HEADER_BYTE_2 != buffer_[2])
        {
            match = false;
        }
        if (check_len >= 4 && HEADER_BYTE_3 != buffer_[3])
        {
            match = false;
        }

        if (true == match)
        {
            break;
        }

        buffer_.erase(buffer_.begin());
    }
}

void LD2450Parser::decode_frame(const uint8_t* frame)
{
    latest_targets_.clear();

    for (uint8_t slot = 0; slot < MAX_FRAME_TARGETS; ++slot)
    {
        const size_t offset        = HEADER_SIZE + (slot * TARGET_SLOT_SIZE);
        const int16_t x_mm         = read_signed_le(&frame[offset]);
        const int16_t y_mm         = read_signed_le(&frame[offset + 2]);
        const int16_t speed_cms    = read_signed_le(&frame[offset + 4]);
        const uint16_t dist_res_mm = read_u16_le(&frame[offset + 6]);

        // Inactive slot check
        if (0 == x_mm && 0 == y_mm && 0 == speed_cms)
        {
            continue;
        }

        TargetData target{};
        target.id                     = static_cast<uint8_t>(slot + 1);
        target.x_mm                   = x_mm;
        target.y_mm                   = y_mm;
        target.speed_cms              = speed_cms;
        target.distance_resolution_mm = dist_res_mm;
        target.is_valid               = true;

        latest_targets_.push_back(target);
    }
}

} // namespace hrs::core
