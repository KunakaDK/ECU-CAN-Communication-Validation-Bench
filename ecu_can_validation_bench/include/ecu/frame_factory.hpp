#pragma once

#include "can/frame.hpp"
#include "validation/frame_validator.hpp"

#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace ecu_sim {

class FrameFactory {
public:
    static can_sim::CanFrame vehicle_speed(const double speed_kmh,
                                           const std::uint64_t timestamp_ms = 0) {
        if (!std::isfinite(speed_kmh) || speed_kmh < 0.0 || speed_kmh > 250.0) {
            throw std::out_of_range("Speed must be within 0-250 km/h");
        }
        const auto raw = static_cast<std::uint16_t>(std::lround(speed_kmh * 10.0));
        can_sim::CanFrame frame;
        frame.id = validation::FrameValidator::kSpeedCanId;
        frame.dlc = 2;
        frame.data[0] = static_cast<std::uint8_t>((raw >> 8U) & 0xFFU);
        frame.data[1] = static_cast<std::uint8_t>(raw & 0xFFU);
        frame.timestamp_ms = timestamp_ms;
        return frame;
    }

    static can_sim::CanFrame coolant_temperature(const double temperature_c,
                                                  const std::uint64_t timestamp_ms = 0) {
        if (!std::isfinite(temperature_c) || temperature_c < -40.0 || temperature_c > 150.0) {
            throw std::out_of_range("Coolant temperature must be within -40-150 C");
        }
        const auto raw = static_cast<int>(std::lround(temperature_c + 40.0));
        can_sim::CanFrame frame;
        frame.id = validation::FrameValidator::kCoolantCanId;
        frame.dlc = 1;
        frame.data[0] = static_cast<std::uint8_t>(raw);
        frame.timestamp_ms = timestamp_ms;
        return frame;
    }

    static can_sim::CanFrame accelerator_pedal(const double pedal_percent,
                                                const std::uint64_t timestamp_ms = 0) {
        if (!std::isfinite(pedal_percent) || pedal_percent < 0.0 || pedal_percent > 100.0) {
            throw std::out_of_range("Pedal value must be within 0-100 percent");
        }
        can_sim::CanFrame frame;
        frame.id = validation::FrameValidator::kPedalCanId;
        frame.dlc = 1;
        frame.data[0] = static_cast<std::uint8_t>(std::lround(pedal_percent));
        frame.timestamp_ms = timestamp_ms;
        return frame;
    }
};

} // namespace ecu_sim
