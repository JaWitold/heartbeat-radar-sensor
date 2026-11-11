#pragma once

#include "target_data.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hrs::core
{

class LD2450Parser
{
public:
    static constexpr size_t FRAME_SIZE         = 30;
    static constexpr size_t HEADER_SIZE        = 4;
    static constexpr size_t TAIL_SIZE          = 2;
    static constexpr size_t TARGET_SLOT_SIZE   = 8;
    static constexpr uint8_t MAX_FRAME_TARGETS = 3;

    static constexpr uint8_t HEADER_BYTE_0 = 0xAA;
    static constexpr uint8_t HEADER_BYTE_1 = 0xFF;
    static constexpr uint8_t HEADER_BYTE_2 = 0x03;
    static constexpr uint8_t HEADER_BYTE_3 = 0x00;
    static constexpr uint8_t TAIL_BYTE_0   = 0x55;
    static constexpr uint8_t TAIL_BYTE_1   = 0xCC;

    LD2450Parser() = default;

    bool parse_byte(const uint8_t byte);

    [[nodiscard]] const std::vector<TargetData>& get_latest_targets() const;
    void reset();

private:
    void decode_frame(const uint8_t* frame);
    void search_and_align_header();

    std::vector<uint8_t> buffer_{};
    std::vector<TargetData> latest_targets_{};
};

} // namespace hrs::core
