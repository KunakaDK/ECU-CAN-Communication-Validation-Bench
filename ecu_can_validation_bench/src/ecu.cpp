#include "ecu/ecu.hpp"

#include "can/frame.hpp"

#include <sstream>

namespace ecu_sim {

void SimulatedEcu::receive(const can_sim::CanFrame& frame) {
    if (state_ == EcuState::SafeState) {
        ++statistics_.ignored_after_safe_state;
        event_log_.push_back(EcuEvent{
            frame.timestamp_ms,
            "FRAME_IGNORED",
            "Frame ignored because the simulated ECU remains in SAFE_STATE: " + can_sim::describe(frame)
        });
        return;
    }

    const validation::ValidationResult result = validator_.validate(frame);
    if (!result.accepted) {
        ++statistics_.rejected_frames;
        last_diagnostic_ = result.code;
        event_log_.push_back(EcuEvent{
            frame.timestamp_ms,
            "FRAME_REJECTED",
            validation::to_string(result.code) + ": " + result.message + " | " + can_sim::describe(frame)
        });
        if (result.safety_relevant) {
            enter_safe_state(frame, result);
        }
        return;
    }

    ++statistics_.accepted_frames;
    switch (result.signal) {
        case validation::SignalType::VehicleSpeed:
            telemetry_.vehicle_speed_kmh = result.physical_value;
            break;
        case validation::SignalType::CoolantTemperature:
            telemetry_.coolant_temperature_c = result.physical_value;
            break;
        case validation::SignalType::AcceleratorPedal:
            telemetry_.accelerator_pedal_percent = result.physical_value;
            break;
        case validation::SignalType::Unknown:
            // The validator never accepts an unknown signal. Keep this branch defensive.
            break;
    }

    std::ostringstream details;
    details << validation::to_string(result.signal) << "=" << result.physical_value
            << " | " << result.message;
    event_log_.push_back(EcuEvent{frame.timestamp_ms, "FRAME_ACCEPTED", details.str()});
}

void SimulatedEcu::enter_safe_state(const can_sim::CanFrame& frame,
                                    const validation::ValidationResult& result) {
    state_ = EcuState::SafeState;
    std::ostringstream details;
    details << "Safety-relevant validation failure; operational processing inhibited. "
            << validation::to_string(result.code) << " | " << can_sim::describe(frame);
    event_log_.push_back(EcuEvent{frame.timestamp_ms, "SAFE_STATE_ENTERED", details.str()});
}

EcuState SimulatedEcu::state() const noexcept {
    return state_;
}

const EcuTelemetry& SimulatedEcu::telemetry() const noexcept {
    return telemetry_;
}

const EcuStatistics& SimulatedEcu::statistics() const noexcept {
    return statistics_;
}

const std::vector<EcuEvent>& SimulatedEcu::event_log() const noexcept {
    return event_log_;
}

validation::DiagnosticCode SimulatedEcu::last_diagnostic() const noexcept {
    return last_diagnostic_;
}

std::string to_string(const EcuState state) {
    switch (state) {
        case EcuState::Operational: return "OPERATIONAL";
        case EcuState::SafeState: return "SAFE_STATE";
    }
    return "UNRECOGNIZED_STATE";
}

} // namespace ecu_sim
