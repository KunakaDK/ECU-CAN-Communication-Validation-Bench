#pragma once

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace can_sim {

// Classic CAN frame model for an isolated software simulator.
// The structure intentionally permits malformed values so the validator can test them.
struct CanFrame {
    std::uint16_t id{0};
    std::uint8_t dlc{0};
    std::array<std::uint8_t, 8> data{};
    std::uint64_t timestamp_ms{0};
    bool extended_id{false};
};

inline std::string to_hex_id(const std::uint16_t id) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(3)
           << std::setfill('0') << id;
    return stream.str();
}

inline std::string describe(const CanFrame& frame) {
    std::ostringstream stream;
    stream << "ID=" << to_hex_id(frame.id)
           << " DLC=" << static_cast<unsigned int>(frame.dlc)
           << " DATA=";
    const auto count = frame.dlc <= 8U ? frame.dlc : 8U;
    for (std::uint8_t index = 0; index < count; ++index) {
        if (index != 0U) {
            stream << ' ';
        }
        stream << std::uppercase << std::hex << std::setw(2)
               << std::setfill('0')
               << static_cast<unsigned int>(frame.data[index]);
    }
    return stream.str();
}

} // namespace can_sim
