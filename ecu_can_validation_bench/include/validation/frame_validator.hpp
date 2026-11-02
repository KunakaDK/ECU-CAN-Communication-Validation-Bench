#pragma once

#include "can/frame.hpp"

#include <cstdint>
#include <string>

namespace validation {

enum class SignalType {
    Unknown,
    VehicleSpeed,
    CoolantTemperature,
    AcceleratorPedal
};

enum class DiagnosticCode {
    None,
    InvalidStandardId,
    UnsupportedExtendedId,
    DlcExceedsClassicCan,
    UnknownCanId,
    UnexpectedDlc,
    SignalOutOfRange
};

struct ValidationResult {
    bool accepted{false};
    bool safety_relevant{false};
    DiagnosticCode code{DiagnosticCode::None};
    SignalType signal{SignalType::Unknown};
    double physical_value{0.0};
    std::string message;
};

class FrameValidator {
public:
    ValidationResult validate(const can_sim::CanFrame& frame) const;

    static constexpr std::uint16_t kSpeedCanId = 0x100;
    static constexpr std::uint16_t kCoolantCanId = 0x101;
    static constexpr std::uint16_t kPedalCanId = 0x102;
};

std::string to_string(DiagnosticCode code);
std::string to_string(SignalType signal);

} // namespace validation
