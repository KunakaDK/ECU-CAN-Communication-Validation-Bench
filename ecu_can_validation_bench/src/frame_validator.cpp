#include "validation/frame_validator.hpp"

#include <cmath>
#include <sstream>

namespace validation {
namespace {

ValidationResult reject(const DiagnosticCode code,
                        const SignalType signal,
                        const bool safety_relevant,
                        const std::string& message) {
    return ValidationResult{false, safety_relevant, code, signal, 0.0, message};
}

ValidationResult accept(const SignalType signal,
                        const double value,
                        const std::string& message) {
    return ValidationResult{true, false, DiagnosticCode::None, signal, value, message};
}

} // namespace

ValidationResult FrameValidator::validate(const can_sim::CanFrame& frame) const {
    if (frame.extended_id) {
        return reject(DiagnosticCode::UnsupportedExtendedId, SignalType::Unknown, false,
                      "Extended identifiers are outside this classic 11-bit simulation profile");
    }
    if (frame.id > 0x7FFU) {
        return reject(DiagnosticCode::InvalidStandardId, SignalType::Unknown, false,
                      "Standard CAN identifier is greater than 0x7FF");
    }
    if (frame.dlc > 8U) {
        const bool known_id = frame.id == kSpeedCanId ||
                              frame.id == kCoolantCanId ||
                              frame.id == kPedalCanId;
        SignalType signal = SignalType::Unknown;
        if (frame.id == kSpeedCanId) signal = SignalType::VehicleSpeed;
        if (frame.id == kCoolantCanId) signal = SignalType::CoolantTemperature;
        if (frame.id == kPedalCanId) signal = SignalType::AcceleratorPedal;
        return reject(DiagnosticCode::DlcExceedsClassicCan, signal, known_id,
                      "DLC exceeds the 8-byte classic CAN payload limit");
    }

    if (frame.id == kSpeedCanId) {
        if (frame.dlc != 2U) {
            return reject(DiagnosticCode::UnexpectedDlc, SignalType::VehicleSpeed, true,
                          "Vehicle-speed frame requires exactly 2 data bytes");
        }
        const auto raw = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(frame.data[0]) << 8U) |
            static_cast<std::uint16_t>(frame.data[1]));
        const double speed_kmh = static_cast<double>(raw) * 0.1;
        if (speed_kmh < 0.0 || speed_kmh > 250.0) {
            return reject(DiagnosticCode::SignalOutOfRange, SignalType::VehicleSpeed, true,
                          "Decoded vehicle speed is outside the configured 0-250 km/h test range");
        }
        return accept(SignalType::VehicleSpeed, speed_kmh, "Vehicle-speed signal is valid");
    }

    if (frame.id == kCoolantCanId) {
        if (frame.dlc != 1U) {
            return reject(DiagnosticCode::UnexpectedDlc, SignalType::CoolantTemperature, true,
                          "Coolant-temperature frame requires exactly 1 data byte");
        }
        const double temperature_c = static_cast<double>(frame.data[0]) - 40.0;
        if (temperature_c < -40.0 || temperature_c > 150.0) {
            return reject(DiagnosticCode::SignalOutOfRange, SignalType::CoolantTemperature, true,
                          "Decoded coolant temperature is outside the configured -40-150 C test range");
        }
        return accept(SignalType::CoolantTemperature, temperature_c,
                      "Coolant-temperature signal is valid");
    }

    if (frame.id == kPedalCanId) {
        if (frame.dlc != 1U) {
            return reject(DiagnosticCode::UnexpectedDlc, SignalType::AcceleratorPedal, true,
                          "Accelerator-pedal frame requires exactly 1 data byte");
        }
        const double pedal_percent = static_cast<double>(frame.data[0]);
        if (pedal_percent < 0.0 || pedal_percent > 100.0) {
            return reject(DiagnosticCode::SignalOutOfRange, SignalType::AcceleratorPedal, true,
                          "Decoded accelerator-pedal value is outside the configured 0-100% test range");
        }
        return accept(SignalType::AcceleratorPedal, pedal_percent,
                      "Accelerator-pedal signal is valid");
    }

    std::ostringstream message;
    message << "CAN identifier " << can_sim::to_hex_id(frame.id)
            << " is not present in the simulator signal map";
    return reject(DiagnosticCode::UnknownCanId, SignalType::Unknown, false, message.str());
}

std::string to_string(const DiagnosticCode code) {
    switch (code) {
        case DiagnosticCode::None: return "NONE";
        case DiagnosticCode::InvalidStandardId: return "INVALID_STANDARD_ID";
        case DiagnosticCode::UnsupportedExtendedId: return "UNSUPPORTED_EXTENDED_ID";
        case DiagnosticCode::DlcExceedsClassicCan: return "DLC_EXCEEDS_CLASSIC_CAN";
        case DiagnosticCode::UnknownCanId: return "UNKNOWN_CAN_ID";
        case DiagnosticCode::UnexpectedDlc: return "UNEXPECTED_DLC";
        case DiagnosticCode::SignalOutOfRange: return "SIGNAL_OUT_OF_RANGE";
    }
    return "UNRECOGNIZED_DIAGNOSTIC";
}

std::string to_string(const SignalType signal) {
    switch (signal) {
        case SignalType::Unknown: return "UNKNOWN";
        case SignalType::VehicleSpeed: return "VEHICLE_SPEED";
        case SignalType::CoolantTemperature: return "COOLANT_TEMPERATURE";
        case SignalType::AcceleratorPedal: return "ACCELERATOR_PEDAL";
    }
    return "UNRECOGNIZED_SIGNAL";
}

} // namespace validation
